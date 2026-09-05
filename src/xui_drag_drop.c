#include "xui_internal.h"

#include <limits.h>
#include <string.h>

#define XUI_DATA_OBJECT_MAGIC 0x5855444Fu
#define XUI_DRAG_EFFECT_MASK \
	(XUI_DRAG_EFFECT_COPY | XUI_DRAG_EFFECT_MOVE | XUI_DRAG_EFFECT_LINK)

typedef struct xui_data_slot_t {
	char* sFormat;
	void* pValue;
	size_t iSize;
	xui_data_provider_proc onRead;
	xui_data_provider_free_proc onFree;
	void* pUser;
} xui_data_slot_t;

struct xui_data_object_t {
	uint32_t iMagic;
	int iRefCount;
	xui_data_slot_t* pSlots;
	int iCount;
	int iCapacity;
};

static int __xuiDataObjectValid(xui_data_object pData)
{
	return pData != NULL && pData->iMagic == XUI_DATA_OBJECT_MAGIC &&
		pData->iRefCount > 0;
}

static char* __xuiDataStringCopy(const char* sText)
{
	char* sCopy;
	size_t iLength;

	if ( sText == NULL || sText[0] == '\0' ) return NULL;
	iLength = strlen(sText);
	sCopy = (char*)xrtMalloc(iLength + 1u);
	if ( sCopy == NULL ) return NULL;
	memcpy(sCopy, sText, iLength + 1u);
	return sCopy;
}

static void __xuiDataSlotClear(xui_data_slot_t* pSlot)
{
	if ( pSlot == NULL ) return;
	if ( pSlot->onFree != NULL ) pSlot->onFree(pSlot->pUser);
	if ( pSlot->pValue != NULL ) xrtFree(pSlot->pValue);
	if ( pSlot->sFormat != NULL ) xrtFree(pSlot->sFormat);
	memset(pSlot, 0, sizeof(*pSlot));
}

static int __xuiDataSlotFind(xui_data_object pData, const char* sFormat)
{
	int i;

	if ( !__xuiDataObjectValid(pData) || sFormat == NULL ) return -1;
	for ( i = 0; i < pData->iCount; i++ ) {
		if ( strcmp(pData->pSlots[i].sFormat, sFormat) == 0 ) return i;
	}
	return -1;
}

static int __xuiDataSlotReserve(xui_data_object pData, int iCapacity)
{
	xui_data_slot_t* pNew;
	int iNewCapacity;

	if ( iCapacity <= pData->iCapacity ) return XUI_OK;
	iNewCapacity = pData->iCapacity > 0 ? pData->iCapacity * 2 : 4;
	while ( iNewCapacity < iCapacity ) iNewCapacity *= 2;
	pNew = (xui_data_slot_t*)xrtRealloc(pData->pSlots,
		sizeof(*pNew) * (size_t)iNewCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pNew + pData->iCapacity, 0,
		sizeof(*pNew) * (size_t)(iNewCapacity - pData->iCapacity));
	pData->pSlots = pNew;
	pData->iCapacity = iNewCapacity;
	return XUI_OK;
}

static int __xuiDataSlotPrepare(xui_data_object pData, const char* sFormat,
	xui_data_slot_t** ppSlot)
{
	xui_data_slot_t* pSlot;
	char* sCopy;
	int iIndex;
	int iRet;

	if ( !__xuiDataObjectValid(pData) || sFormat == NULL ||
		sFormat[0] == '\0' || ppSlot == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xuiDataSlotFind(pData, sFormat);
	if ( iIndex >= 0 ) {
		pSlot = &pData->pSlots[iIndex];
		if ( pSlot->onFree != NULL ) pSlot->onFree(pSlot->pUser);
		if ( pSlot->pValue != NULL ) xrtFree(pSlot->pValue);
		pSlot->pValue = NULL;
		pSlot->iSize = 0u;
		pSlot->onRead = NULL;
		pSlot->onFree = NULL;
		pSlot->pUser = NULL;
		*ppSlot = pSlot;
		return XUI_OK;
	}
	iRet = __xuiDataSlotReserve(pData, pData->iCount + 1);
	if ( iRet != XUI_OK ) return iRet;
	sCopy = __xuiDataStringCopy(sFormat);
	if ( sCopy == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pSlot = &pData->pSlots[pData->iCount++];
	pSlot->sFormat = sCopy;
	*ppSlot = pSlot;
	return XUI_OK;
}

XUI_API int xuiDataObjectCreate(xui_data_object* ppData)
{
	xui_data_object pData;

	if ( ppData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppData = NULL;
	pData = (xui_data_object)xrtMalloc(sizeof(*pData));
	if ( pData == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pData, 0, sizeof(*pData));
	pData->iMagic = XUI_DATA_OBJECT_MAGIC;
	pData->iRefCount = 1;
	*ppData = pData;
	return XUI_OK;
}

XUI_API int xuiDataObjectAddRef(xui_data_object pData)
{
	if ( !__xuiDataObjectValid(pData) || pData->iRefCount == INT_MAX ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData->iRefCount++;
	return XUI_OK;
}

XUI_API void xuiDataObjectRelease(xui_data_object pData)
{
	int i;

	if ( !__xuiDataObjectValid(pData) ) return;
	pData->iRefCount--;
	if ( pData->iRefCount > 0 ) return;
	for ( i = 0; i < pData->iCount; i++ ) __xuiDataSlotClear(&pData->pSlots[i]);
	if ( pData->pSlots != NULL ) xrtFree(pData->pSlots);
	pData->iMagic = 0u;
	xrtFree(pData);
}

XUI_API int xuiDataObjectSet(xui_data_object pData, const char* sFormat,
	const void* pValue, size_t iSize)
{
	xui_data_slot_t* pSlot;
	void* pCopy = NULL;
	int iRet;

	if ( iSize > 0u && pValue == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iSize > 0u ) {
		pCopy = xrtMalloc(iSize);
		if ( pCopy == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		memcpy(pCopy, pValue, iSize);
	}
	iRet = __xuiDataSlotPrepare(pData, sFormat, &pSlot);
	if ( iRet != XUI_OK ) {
		if ( pCopy != NULL ) xrtFree(pCopy);
		return iRet;
	}
	pSlot->pValue = pCopy;
	pSlot->iSize = iSize;
	return XUI_OK;
}

XUI_API int xuiDataObjectSetProvider(xui_data_object pData,
	const char* sFormat, xui_data_provider_proc onRead,
	xui_data_provider_free_proc onFree, void* pUser)
{
	xui_data_slot_t* pSlot;
	int iRet;

	if ( onRead == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiDataSlotPrepare(pData, sFormat, &pSlot);
	if ( iRet != XUI_OK ) return iRet;
	pSlot->onRead = onRead;
	pSlot->onFree = onFree;
	pSlot->pUser = pUser;
	return XUI_OK;
}

XUI_API int xuiDataObjectFormatCount(xui_data_object pData)
{
	return __xuiDataObjectValid(pData) ? pData->iCount : 0;
}

XUI_API const char* xuiDataObjectFormatAt(xui_data_object pData, int iIndex)
{
	if ( !__xuiDataObjectValid(pData) || iIndex < 0 ||
		iIndex >= pData->iCount ) return NULL;
	return pData->pSlots[iIndex].sFormat;
}

XUI_API int xuiDataObjectHas(xui_data_object pData, const char* sFormat)
{
	return __xuiDataSlotFind(pData, sFormat) >= 0;
}

XUI_API int xuiDataObjectGet(xui_data_object pData, const char* sFormat,
	void* pOutput, size_t iCapacity, size_t* pOutputSize)
{
	xui_data_slot_t* pSlot;
	int iIndex;

	if ( pOutputSize == NULL || (pOutput == NULL && iCapacity != 0u) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pOutputSize = 0u;
	iIndex = __xuiDataSlotFind(pData, sFormat);
	if ( iIndex < 0 ) return XUI_ERROR_NOT_FOUND;
	pSlot = &pData->pSlots[iIndex];
	if ( pSlot->onRead != NULL ) {
		return pSlot->onRead(sFormat, pOutput, iCapacity,
			pOutputSize, pSlot->pUser);
	}
	*pOutputSize = pSlot->iSize;
	if ( pOutput == NULL ) return XUI_OK;
	if ( iCapacity < pSlot->iSize ) return XUI_ERROR_BUFFER_TOO_SMALL;
	if ( pSlot->iSize > 0u ) memcpy(pOutput, pSlot->pValue, pSlot->iSize);
	return XUI_OK;
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
