#include "xui_config.h"
#if XGE_ENABLE_XUI
#include "xui_internal.h"

#include <limits.h>
#include <stdatomic.h>
#include <string.h>

#define XUI_DATA_OBJECT_MAGIC 0x5855444Fu
#define XUI_DRAG_EFFECT_MASK \
	(XUI_DRAG_EFFECT_COPY | XUI_DRAG_EFFECT_MOVE | XUI_DRAG_EFFECT_LINK)

/* A slot owns one immutable payload generation. A provider invocation owns a
 * second real reference until its complete C return; replacement never leaves
 * a reader holding a pointer into the movable slot array. Mutable operations
 * retain their existing caller-serialized C contract. Atomic references only
 * protect actual lifetime, not arbitrary concurrent container mutations. */
typedef struct xui_data_generation_t {
	atomic_uint iRefs;
	char* sFormat;
	void* pValue;
	size_t iSize;
	xui_data_provider_proc onRead;
	xui_data_provider_free_proc onFree;
	void* pUser;
} xui_data_generation_t;

struct xui_data_object_t {
	uint32_t iMagic;
	atomic_uint iRefCount;
	xui_data_generation_t** pSlots;
	int iCount;
	int iCapacity;
	atomic_uint iActivity;
	unsigned bDestroying;
};

static int __xuiDataObjectValid(xui_data_object pData)
{
	return pData != NULL && pData->iMagic == XUI_DATA_OBJECT_MAGIC &&
		atomic_load_explicit(&pData->iRefCount,memory_order_acquire)>0u &&
		!pData->bDestroying;
}

static char* __xuiDataStringCopy(const char* sText)
{
	if(!sText || !sText[0])return NULL;
	size_t length=strlen(sText);
	if(length==SIZE_MAX)return NULL;
	char* copy=(char*)xrtMalloc(length+1u);
	if(copy)memcpy(copy,sText,length+1u);
	return copy;
}

static bool __xuiDataGenerationHold(xui_data_generation_t* generation)
{
	unsigned refs=atomic_load_explicit(&generation->iRefs,memory_order_relaxed);
	do {if(!refs || refs==UINT_MAX)return false;}
	while(!atomic_compare_exchange_weak_explicit(&generation->iRefs,&refs,refs+1u,memory_order_relaxed,memory_order_relaxed));
	return true;
}

static void __xuiDataGenerationDrop(xui_data_generation_t* generation)
{
	if(!generation || atomic_fetch_sub_explicit(&generation->iRefs,1u,memory_order_acq_rel)!=1u)return;
	/* The record is detached from its publisher before arbitrary cleanup. Its
	 * fields cannot be reconfigured by that cleanup, including self-reentry. */
	if(generation->onFree)generation->onFree(generation->pUser);
	xrtFree(generation);
}

static int __xuiDataSlotFind(xui_data_object pData,const char* format)
{
	if(!__xuiDataObjectValid(pData) || !format)return -1;
	for(int i=0;i<pData->iCount;++i)
		if(strcmp(pData->pSlots[i]->sFormat,format)==0)return i;
	return -1;
}

static int __xuiDataSlotReserve(xui_data_object pData,int capacity)
{
	if(capacity<=pData->iCapacity)return XUI_OK;
	if(capacity<0 || (size_t)capacity>SIZE_MAX/sizeof(*pData->pSlots))return XUI_ERROR_OUT_OF_MEMORY;
	int next=pData->iCapacity>0 ? pData->iCapacity:4;
	while(next<capacity){if(next>INT_MAX/2){next=capacity;break;}next*=2;}
	if((size_t)next>SIZE_MAX/sizeof(*pData->pSlots))return XUI_ERROR_OUT_OF_MEMORY;
	xui_data_generation_t** slots=(xui_data_generation_t**)xrtRealloc(pData->pSlots,sizeof(*slots)*(size_t)next);
	if(!slots)return XUI_ERROR_OUT_OF_MEMORY;
	pData->pSlots=slots;pData->iCapacity=next;
	return XUI_OK;
}

XUI_API int xuiDataObjectCreate(xui_data_object* output)
{
	if(!output)return XUI_ERROR_INVALID_ARGUMENT;
	*output=NULL;
	xui_data_object object=(xui_data_object)xrtCalloc(1u,sizeof(*object));
	if(!object)return XUI_ERROR_OUT_OF_MEMORY;
	object->iMagic=XUI_DATA_OBJECT_MAGIC;
	atomic_init(&object->iRefCount,1u);atomic_init(&object->iActivity,0u);
	*output=object;return XUI_OK;
}

XUI_API int xuiDataObjectAddRef(xui_data_object object)
{
	if(!__xuiDataObjectValid(object))return XUI_ERROR_INVALID_ARGUMENT;
	unsigned refs=atomic_load_explicit(&object->iRefCount,memory_order_relaxed);
	do {if(!refs || refs>=INT_MAX)return XUI_ERROR_INVALID_ARGUMENT;}
	while(!atomic_compare_exchange_weak_explicit(&object->iRefCount,&refs,refs+1u,memory_order_relaxed,memory_order_relaxed));
	return XUI_OK;
}

XUI_API void xuiDataObjectRelease(xui_data_object object)
{
	if(!__xuiDataObjectValid(object))return;
	if(atomic_fetch_sub_explicit(&object->iRefCount,1u,memory_order_acq_rel)!=1u)return;
	/* No owning field remains attached while provider cleanup can reenter. A
	 * real active operation still owns a reference and cannot reach this tail. */
	object->bDestroying=1u;
	xui_data_generation_t** slots=object->pSlots;
	int count=object->iCount;
	object->pSlots=NULL;object->iCount=0;object->iCapacity=0;
	for(int i=0;i<count;++i)__xuiDataGenerationDrop(slots[i]);
	xrtFree(slots);object->iMagic=0u;xrtFree(object);
}

static int __xuiDataOperationBegin(xui_data_object object)
{
	int status=xuiDataObjectAddRef(object);
	if(status!=XUI_OK)return status;
	unsigned active=atomic_fetch_add_explicit(&object->iActivity,1u,memory_order_relaxed);
	if(active==UINT_MAX){
		atomic_fetch_sub_explicit(&object->iActivity,1u,memory_order_relaxed);
		xuiDataObjectRelease(object);return XUI_ERROR_INVALID_STATE;
	}
	return XUI_OK;
}

static void __xuiDataOperationEnd(xui_data_object object)
{
	atomic_fetch_sub_explicit(&object->iActivity,1u,memory_order_release);
	xuiDataObjectRelease(object);
}

static xui_data_generation_t* __xuiDataGenerationCreate(const char* format,const void* value,size_t size)
{
	size_t length=strlen(format);
	if(length>=SIZE_MAX-sizeof(xui_data_generation_t) ||
		size>SIZE_MAX-sizeof(xui_data_generation_t)-length-1u)return NULL;
	/* Metadata, stable format and optional bytes are one actual allocation.
	 * pValue is byte storage; no typed/aligned payload pointer is exposed. */
	xui_data_generation_t* generation=(xui_data_generation_t*)xrtCalloc(1u,sizeof(*generation)+length+1u+size);
	if(!generation)return NULL;
	atomic_init(&generation->iRefs,1u);
	generation->sFormat=(char*)(generation+1);memcpy(generation->sFormat,format,length+1u);
	if(size){generation->pValue=generation->sFormat+length+1u;memcpy(generation->pValue,value,size);}
	generation->iSize=size;
	return generation;
}

static int __xuiDataPublish(xui_data_object object,xui_data_generation_t* next)
{
	int index=__xuiDataSlotFind(object,next->sFormat);
	xui_data_generation_t* previous=NULL;
	if(index<0){
		if(object->iCount==INT_MAX)return XUI_ERROR_OUT_OF_MEMORY;
		int status=__xuiDataSlotReserve(object,object->iCount+1);
		if(status!=XUI_OK)return status;
		index=object->iCount++;
	}else previous=object->pSlots[index];
	object->pSlots[index]=next;
	__xuiDataGenerationDrop(previous);
	return XUI_OK;
}

XUI_API int xuiDataObjectSet(xui_data_object object,const char* format,const void* value,size_t size)
{
	if(!format || !format[0] || (size && !value))return XUI_ERROR_INVALID_ARGUMENT;
	int status=__xuiDataOperationBegin(object);
	if(status!=XUI_OK)return status;
	xui_data_generation_t* next=__xuiDataGenerationCreate(format,value,size);
	if(!next)status=XUI_ERROR_OUT_OF_MEMORY;
	else {
		status=__xuiDataPublish(object,next);
		if(status!=XUI_OK)__xuiDataGenerationDrop(next);
	}
	__xuiDataOperationEnd(object);return status;
}

XUI_API int xuiDataObjectSetProvider(xui_data_object object,const char* format,
	xui_data_provider_proc read,xui_data_provider_free_proc release,void* user)
{
	if(!format || !format[0] || !read)return XUI_ERROR_INVALID_ARGUMENT;
	int status=__xuiDataOperationBegin(object);
	if(status!=XUI_OK)return status;
	xui_data_generation_t* next=__xuiDataGenerationCreate(format,NULL,0u);
	if(!next)status=XUI_ERROR_OUT_OF_MEMORY;
	else {
		next->onRead=read;next->onFree=release;next->pUser=user;
		status=__xuiDataPublish(object,next);
		if(status!=XUI_OK){next->onFree=NULL;__xuiDataGenerationDrop(next);}
	}
	__xuiDataOperationEnd(object);return status;
}

XUI_API int xuiDataObjectFormatCount(xui_data_object object)
{return __xuiDataObjectValid(object) ? object->iCount:0;}

XUI_API const char* xuiDataObjectFormatAt(xui_data_object object,int index)
{
	return __xuiDataObjectValid(object) && index>=0 && index<object->iCount ? object->pSlots[index]->sFormat:NULL;
}

XUI_API int xuiDataObjectHas(xui_data_object object,const char* format)
{return __xuiDataSlotFind(object,format)>=0;}

XUI_API int xuiDataObjectGet(xui_data_object object,const char* format,void* output,size_t capacity,size_t* outputSize)
{
	if(!outputSize || (capacity && !output))return XUI_ERROR_INVALID_ARGUMENT;
	*outputSize=0u;
	int index=__xuiDataSlotFind(object,format);
	xui_data_generation_t* generation=index<0 ? NULL:object->pSlots[index];
	/* Raw containers are caller-serialized. Pure byte reads do not call user
	 * code and cannot invalidate their slot; keep their direct copy hot path.
	 * Provider reads below pin both actual objects across arbitrary reentry. */
	if(generation && !generation->onRead){
		*outputSize=generation->iSize;
		if(output && capacity<generation->iSize)return XUI_ERROR_BUFFER_TOO_SMALL;
		if(output && generation->iSize)memcpy(output,generation->pValue,generation->iSize);
		return XUI_OK;
	}
	int status=__xuiDataOperationBegin(object);
	if(status!=XUI_OK)return status;
	if(!generation)status=XUI_ERROR_NOT_FOUND;
	else if(!__xuiDataGenerationHold(generation))status=XUI_ERROR_INVALID_STATE;
	else {
		if(generation->onRead)status=generation->onRead(generation->sFormat,output,capacity,outputSize,generation->pUser);
		else {
			*outputSize=generation->iSize;
			if(output && capacity<generation->iSize)status=XUI_ERROR_BUFFER_TOO_SMALL;
			else if(output && generation->iSize)memcpy(output,generation->pValue,generation->iSize);
		}
		/* A failing provider's nullable first diagnostic must survive its final
		 * generation cleanup and a self-released object's final native cleanup. */
		xerror* first=status!=XUI_OK ? xrtTakeError():NULL;
		__xuiDataGenerationDrop(generation);
		__xuiDataOperationEnd(object);
		if(status!=XUI_OK){xrtClearError();xrtSetErrorTake(first);}
		return status;
	}
	__xuiDataOperationEnd(object);return status;
}

static int __xuiDragEffectValid(uint32_t iEffect, uint32_t iAllowed)
{
	return iEffect != XUI_DRAG_EFFECT_NONE &&
		(iEffect & ~XUI_DRAG_EFFECT_MASK) == 0u &&
		(iEffect & (iEffect - 1u)) == 0u &&
		(iEffect & iAllowed) != 0u;
}

typedef struct xui_drag_dispatch_t {
	struct xui_drag_dispatch_t* pPrevious;
	uint64_t iSessionId;
	uint64_t iRevision;
	xui_widget pTarget;
	int bNegotiating;
} xui_drag_dispatch_t;

typedef struct xui_drag_detach_t {
	struct xui_drag_detach_t* pPrevious;
	xui_widget pWidget;
} xui_drag_detach_t;

static int __xuiDragContextReady(xui_context pContext)
{
	return xuiInternalContextIsValid(pContext) &&
		!pContext->bDestroyPending && !pContext->bDestroying;
}

static int __xuiDragCurrent(xui_context pContext, uint64_t iSessionId,
	uint64_t iRevision)
{
	return __xuiDragContextReady(pContext) && pContext->bTransferActive &&
		pContext->iDragSessionId == iSessionId && pContext->iDragRevision == iRevision;
}

static int __xuiDragContains(xui_widget pRoot, xui_widget pWidget)
{
	for ( ; pWidget != NULL; pWidget = pWidget->pParent ) {
		if ( pWidget == pRoot ) return 1;
	}
	return 0;
}

static int __xuiDragWidgetAvailable(xui_context pContext, xui_widget pWidget)
{
	xui_drag_detach_t* pDetach;

	if ( !xuiInternalWidgetIsValid(pWidget) || pWidget->pContext != pContext ) return 0;
	for ( pDetach = pContext->pDragDetach; pDetach != NULL; pDetach = pDetach->pPrevious ) {
		if ( __xuiDragContains(pDetach->pWidget, pWidget) ) return 0;
	}
	return 1;
}

static xui_widget __xuiDragTargetAt(xui_context pContext, int iX, int iY)
{
	xui_widget pTarget;

	pTarget = xuiHitTest(pContext, iX, iY, XUI_WIDGET_HIT_DEFAULT);
	while ( __xuiDragWidgetAvailable(pContext, pTarget) ) {
		if ( pTarget->bDropEnabled ) return pTarget;
		pTarget = pTarget->pParent;
	}
	return NULL;
}

static void __xuiDragClear(xui_context pContext, int bKeepEffect);

static int __xuiDragDispatch(xui_context pContext, int iType,
	xui_widget pTarget, xui_widget pRelated, int bCancel)
{
	xui_drag_dispatch_t tDispatch;
	xui_drag_event_data_t tDrag;
	xui_event_t tEvent;
	int iRet;

	tDispatch.pPrevious = pContext->pDragDispatch;
	tDispatch.iSessionId = pContext->iDragSessionId;
	tDispatch.iRevision = pContext->iDragRevision;
	tDispatch.pTarget = pTarget;
	tDispatch.bNegotiating = (iType != XUI_EVENT_DRAG_LEAVE);
	memset(&tDrag, 0, sizeof(tDrag));
	tDrag.iSize = sizeof(tDrag);
	tDrag.pData = pContext->pTransferData;
	tDrag.pSource = pContext->pDragSource;
	tDrag.pTarget = pTarget;
	tDrag.iAllowedEffects = pContext->iDragAllowedEffects;
	tDrag.iSuggestedEffect = pContext->iDragSuggestedEffect;
	tDrag.iEffect = pContext->iDragEffect;
	tDrag.bExternal = pContext->bTransferExternal;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = iType;
	tEvent.pTarget = pTarget;
	tEvent.pRelated = __xuiDragWidgetAvailable(pContext, pRelated) ? pRelated : NULL;
	tEvent.fX = pContext->fPointerX;
	tEvent.fY = pContext->fPointerY;
	tEvent.iButtons = pContext->iPointerButtons;
	tEvent.iModifiers = pContext->iInputModifiers;
	tEvent.iPointerId = pContext->iInputPointerId;
	tEvent.iPointerType = pContext->iInputPointerType;
	tEvent.pData = &tDrag;
	if ( bCancel ) {
		/* Retire before LEAVE; the dispatch now owns the former session reference. */
		pContext->pTransferData = NULL;
		__xuiDragClear(pContext, 0);
	} else {
		iRet = xuiDataObjectAddRef(tDrag.pData);
		if ( iRet != XUI_OK ) return iRet;
		pContext->iDragEffect = XUI_DRAG_EFFECT_NONE;
	}
	pContext->pDragDispatch = &tDispatch;
	iRet = XUI_OK;
	if ( __xuiDragContextReady(pContext) && xuiInternalWidgetIsValid(pTarget) ) {
		iRet = xuiDispatchEvent(pContext, &tEvent);
	}
	/* Restore the call stack, not the possibly replaced session's negotiation state. */
	pContext->pDragDispatch = tDispatch.pPrevious;
	xuiDataObjectRelease(tDrag.pData);
	return iRet;
}

static void __xuiDragClear(xui_context pContext, int bKeepEffect)
{
	xui_data_object pData = pContext->pTransferData;

	pContext->iDragSessionId++;
	pContext->pTransferData = NULL;
	pContext->pDragSource = NULL;
	pContext->pDropTarget = NULL;
	pContext->iDragAllowedEffects = 0u;
	pContext->iDragSuggestedEffect = 0u;
	if ( !bKeepEffect ) pContext->iDragLastEffect = XUI_DRAG_EFFECT_NONE;
	pContext->iDragEffect = XUI_DRAG_EFFECT_NONE;
	pContext->bTransferActive = 0;
	pContext->bTransferExternal = 0;
	pContext->bTransferDropping = 0;
	if ( pData != NULL ) xuiDataObjectRelease(pData);
}

static int __xuiDragBeginOperation(xui_context pContext, xui_widget pSource,
	xui_data_object pData, uint32_t iAllowedEffects, uint32_t iSuggestedEffect)
{
	if ( !__xuiDragContextReady(pContext) ||
		!__xuiDragWidgetAvailable(pContext, pSource) ||
		!__xuiDataObjectValid(pData) ||
		(iAllowedEffects & XUI_DRAG_EFFECT_MASK) == 0u ||
		(iAllowedEffects & ~XUI_DRAG_EFFECT_MASK) != 0u ||
		(iSuggestedEffect != 0u &&
		 !__xuiDragEffectValid(iSuggestedEffect, iAllowedEffects)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pContext->bTransferActive ) return XUI_ERROR_INVALID_STATE;
	if ( xuiDataObjectAddRef(pData) != XUI_OK ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext->iDragSessionId++;
	pContext->pTransferData = pData;
	pContext->pDragSource = pSource;
	pContext->iDragAllowedEffects = iAllowedEffects;
	pContext->iDragSuggestedEffect = iSuggestedEffect;
	pContext->iDragLastEffect = XUI_DRAG_EFFECT_NONE;
	pContext->bTransferActive = 1;
	pContext->bTransferExternal = 0;
	return xuiInternalDragTransferMove(pContext, pContext->fPointerX,
		pContext->fPointerY, pContext->iInputModifiers);
}

XUI_API int xuiDragBegin(xui_context pContext, xui_widget pSource,
	xui_data_object pData, uint32_t iAllowedEffects, uint32_t iSuggestedEffect)
{
	int iRet;
	xuiInternalOperationEnter(pContext);
	iRet = __xuiDragBeginOperation(pContext, pSource, pData, iAllowedEffects, iSuggestedEffect);
	xuiInternalOperationLeave(pContext);
	return iRet;
}

XUI_API int xuiDragAccept(xui_context pContext, uint32_t iEffect)
{
	xui_drag_dispatch_t* pDispatch;

	if ( !__xuiDragContextReady(pContext) ) return XUI_ERROR_INVALID_STATE;
	pDispatch = pContext->pDragDispatch;
	if ( pDispatch == NULL || !pDispatch->bNegotiating ||
		!__xuiDragCurrent(pContext, pDispatch->iSessionId, pDispatch->iRevision) ||
		pContext->pDropTarget != pDispatch->pTarget ||
		!__xuiDragWidgetAvailable(pContext, pDispatch->pTarget) ) return XUI_ERROR_INVALID_STATE;
	if ( !__xuiDragEffectValid(iEffect, pContext->iDragAllowedEffects) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext->iDragEffect = iEffect;
	return XUI_OK;
}

static int __xuiDragMove(xui_context pContext, int iX, int iY,
	uint32_t iModifiers, uint64_t iSessionId, uint64_t iRevision)
{
	xui_widget pOldTarget;
	xui_widget pTarget;
	int iRet;

	pContext->fPointerX = iX;
	pContext->fPointerY = iY;
	pContext->iInputModifiers = iModifiers;
	pOldTarget = pContext->pDropTarget;
	pTarget = __xuiDragTargetAt(pContext, iX, iY);
	if ( !__xuiDragCurrent(pContext, iSessionId, iRevision) ) return XUI_OK;
	if ( pTarget != pOldTarget ) {
		/* A nested cancel/move must not send LEAVE to this target again. */
		pContext->pDropTarget = NULL;
		if ( pOldTarget != NULL ) {
			iRet = __xuiDragDispatch(pContext, XUI_EVENT_DRAG_LEAVE,
				pOldTarget, pTarget, 0);
			if ( iRet != XUI_OK ) return iRet;
			if ( !__xuiDragCurrent(pContext, iSessionId, iRevision) ) return XUI_OK;
			pTarget = __xuiDragTargetAt(pContext, iX, iY);
			if ( !__xuiDragCurrent(pContext, iSessionId, iRevision) ) return XUI_OK;
		}
		pContext->pDropTarget = pTarget;
		pContext->iDragEffect = XUI_DRAG_EFFECT_NONE;
		if ( pTarget != NULL ) {
			return __xuiDragDispatch(pContext, XUI_EVENT_DRAG_ENTER,
				pTarget, pOldTarget, 0);
		}
		return XUI_OK;
	}
	if ( pTarget != NULL ) {
		return __xuiDragDispatch(pContext, XUI_EVENT_DRAG_OVER, pTarget, NULL, 0);
	}
	pContext->iDragEffect = XUI_DRAG_EFFECT_NONE;
	return XUI_OK;
}

int xuiInternalDragTransferMove(xui_context pContext, int iX, int iY,
	uint32_t iModifiers)
{
	int iRet = XUI_OK;
	xuiInternalOperationEnter(pContext);
	if ( __xuiDragContextReady(pContext) && pContext->bTransferActive ) {
		uint64_t iRevision = ++pContext->iDragRevision;
		iRet = __xuiDragMove(pContext, iX, iY, iModifiers,
			pContext->iDragSessionId, iRevision);
	}
	xuiInternalOperationLeave(pContext);
	return iRet;
}

static int __xuiDragDropOperation(xui_context pContext, int iX, int iY,
	uint32_t iModifiers)
{
	uint64_t iSessionId;
	uint64_t iRevision;
	int iRet;

	if ( !__xuiDragContextReady(pContext) || !pContext->bTransferActive ||
		pContext->bTransferDropping ) {
		return XUI_OK;
	}
	iSessionId = pContext->iDragSessionId;
	iRevision = ++pContext->iDragRevision;
	pContext->bTransferDropping = 1;
	iRet = __xuiDragMove(pContext, iX, iY, iModifiers, iSessionId, iRevision);
	if ( iRet != XUI_OK || !__xuiDragCurrent(pContext, iSessionId, iRevision) ) goto done;
	if ( pContext->pDropTarget != NULL ) {
		iRet = __xuiDragDispatch(pContext, XUI_EVENT_DROP,
			pContext->pDropTarget, NULL, 0);
		if ( iRet != XUI_OK || !__xuiDragCurrent(pContext, iSessionId, iRevision) ) goto done;
	}
	pContext->iDragLastEffect = pContext->iDragEffect;
	__xuiDragClear(pContext, 1);
done:
	if ( pContext->iDragSessionId == iSessionId ) pContext->bTransferDropping = 0;
	return iRet;
}

int xuiInternalDragTransferDrop(xui_context pContext, int iX, int iY,
	uint32_t iModifiers)
{
	int iRet;
	xuiInternalOperationEnter(pContext);
	iRet = __xuiDragDropOperation(pContext, iX, iY, iModifiers);
	xuiInternalOperationLeave(pContext);
	return iRet;
}

void xuiInternalDragTransferCancel(xui_context pContext)
{
	if ( !xuiInternalContextIsValid(pContext) || !pContext->bTransferActive ) return;
	xuiInternalOperationEnter(pContext);
	if ( pContext->pDropTarget != NULL ) {
		(void)__xuiDragDispatch(pContext, XUI_EVENT_DRAG_LEAVE,
			pContext->pDropTarget, NULL, 1);
	} else {
		__xuiDragClear(pContext, 0);
	}
	xuiInternalOperationLeave(pContext);
}

XUI_API int xuiDragCancel(xui_context pContext)
{
	if ( !xuiInternalContextIsValid(pContext) ) return XUI_ERROR_INVALID_ARGUMENT;
	xuiInternalDragTransferCancel(pContext);
	return XUI_OK;
}

XUI_API int xuiDragIsActive(xui_context pContext)
{
	return xuiInternalContextIsValid(pContext) ? pContext->bTransferActive : 0;
}

XUI_API uint32_t xuiDragGetEffect(xui_context pContext)
{
	if ( !xuiInternalContextIsValid(pContext) ) return XUI_DRAG_EFFECT_NONE;
	return pContext->bTransferActive ? pContext->iDragEffect :
		pContext->iDragLastEffect;
}

static int __xuiDragExternalEventOperation(xui_context pContext, int iType,
	int iX, int iY, uint32_t iModifiers, xui_data_object pData,
	uint32_t iAllowedEffects, uint32_t iSuggestedEffect, uint32_t* pEffect)
{
	int iRet = XUI_OK;

	if ( !__xuiDragContextReady(pContext) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iType == XUI_DRAG_EXTERNAL_ENTER ) {
		uint64_t iSessionId = pContext->iDragSessionId;
		/* Pin incoming data before cancellation, including a borrowed current payload. */
		if ( !__xuiDataObjectValid(pData) ||
			(iAllowedEffects & XUI_DRAG_EFFECT_MASK) == 0u ||
			(iAllowedEffects & ~XUI_DRAG_EFFECT_MASK) != 0u ||
			(iSuggestedEffect != 0u &&
			 !__xuiDragEffectValid(iSuggestedEffect, iAllowedEffects)) ) {
			iRet = XUI_ERROR_INVALID_ARGUMENT;
		} else {
			iRet = xuiDataObjectAddRef(pData);
		}
		if ( pContext->bTransferActive ) {
			iSessionId++;
			xuiInternalDragTransferCancel(pContext);
		}
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiDragContextReady(pContext) || pContext->iDragSessionId != iSessionId ) {
			xuiDataObjectRelease(pData);
			if ( pEffect != NULL ) *pEffect = XUI_DRAG_EFFECT_NONE;
			return XUI_OK;
		}
		pContext->iDragSessionId++;
		pContext->pTransferData = pData;
		pContext->iDragAllowedEffects = iAllowedEffects;
		pContext->iDragSuggestedEffect = iSuggestedEffect;
		pContext->iDragLastEffect = XUI_DRAG_EFFECT_NONE;
		pContext->bTransferActive = 1;
		pContext->bTransferExternal = 1;
		iRet = xuiInternalDragTransferMove(pContext, iX, iY, iModifiers);
	} else if ( iType == XUI_DRAG_EXTERNAL_OVER ) {
		if ( !pContext->bTransferActive || !pContext->bTransferExternal ) {
			return XUI_ERROR_INVALID_STATE;
		}
		iRet = xuiInternalDragTransferMove(pContext, iX, iY, iModifiers);
	} else if ( iType == XUI_DRAG_EXTERNAL_DROP ) {
		if ( !pContext->bTransferActive || !pContext->bTransferExternal ) {
			return XUI_ERROR_INVALID_STATE;
		}
		iRet = xuiInternalDragTransferDrop(pContext, iX, iY, iModifiers);
	} else if ( iType == XUI_DRAG_EXTERNAL_LEAVE ||
		iType == XUI_DRAG_EXTERNAL_CANCEL ) {
		xuiInternalDragTransferCancel(pContext);
	} else {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pEffect != NULL ) {
		*pEffect = __xuiDragContextReady(pContext) ? xuiDragGetEffect(pContext) : XUI_DRAG_EFFECT_NONE;
	}
	return iRet;
}

XUI_API int xuiDragExternalEvent(xui_context pContext, int iType,
	int iX, int iY, uint32_t iModifiers, xui_data_object pData,
	uint32_t iAllowedEffects, uint32_t iSuggestedEffect, uint32_t* pEffect)
{
	int iRet;
	xuiInternalOperationEnter(pContext);
	iRet = __xuiDragExternalEventOperation(pContext, iType, iX, iY, iModifiers,
		pData, iAllowedEffects, iSuggestedEffect, pEffect);
	xuiInternalOperationLeave(pContext);
	return iRet;
}

void xuiInternalDragTransferDetachWidget(xui_context pContext, xui_widget pWidget)
{
	xui_drag_detach_t tDetach;

	if ( !xuiInternalContextIsValid(pContext) || !pContext->bTransferActive ||
		pWidget == NULL ) return;
	xuiInternalOperationEnter(pContext);
	tDetach.pPrevious = pContext->pDragDetach;
	tDetach.pWidget = pWidget;
	pContext->pDragDetach = &tDetach;
	if ( __xuiDragContains(pWidget, pContext->pDragSource) ||
		__xuiDragContains(pWidget, pContext->pDropTarget) ) {
		xuiInternalDragTransferCancel(pContext);
	}
	pContext->pDragDetach = tDetach.pPrevious;
	xuiInternalOperationLeave(pContext);
}

void xuiInternalDragTransferShutdown(xui_context pContext)
{
	if ( pContext == NULL ) return;
	__xuiDragClear(pContext, 0);
}

#endif
