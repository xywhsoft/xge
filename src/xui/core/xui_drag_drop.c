#include "xui_config.h"
#if XGE_ENABLE_XUI
#include "xui_internal.h"
#include "xui_data_object.c"
#include <limits.h>
#include <string.h>

#define XUI_DRAG_EFFECT_MASK \
	(XUI_DRAG_EFFECT_COPY | XUI_DRAG_EFFECT_MOVE | XUI_DRAG_EFFECT_LINK)

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
		!pContext->bCloseRequested && !pContext->bDestroyPending && !pContext->bDestroying;
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

static int __xuiDragAssign(xui_context context,xui_data_object data)
{
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    context->pTransferData=data;
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    return XUI_OK;
}
static int __xuiDragRelease(xui_context context,xui_data_object data,int primary)
{
    if(!data)return primary;
    xerror* first=xrtTakeError();bool consumed=true;
    int dropped=XUI_OK;
    if(xuiDataObjectIsOwned(data))dropped=xuiDataObjectReleaseOwnerChecked(data,&consumed);
    else xuiDataObjectRelease(data);
    xerror* fresh=xrtTakeError();
    if(dropped==XUI_OK && fresh)dropped=XUI_ERROR_CALLBACK_FAILED;
    if(dropped!=XUI_OK){
        xuiInternalContextRecordOutcome(context,dropped,primary!=XUI_OK ? first:fresh);
        if(primary==XUI_OK){xrtErrorFree(first);first=fresh;fresh=NULL;primary=dropped;}
    }
    xrtErrorFree(fresh);xrtSetErrorTake(first);
    /* Owned Context only admits creator-origin real owners. Active operations
     * own extra refs, so existing slot release consumes even when a callback
     * is active or its semantic Drop fails. Freeze semantic calls are illegal. */
    if(!consumed)abort();
    return primary;
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
		iRet=__xuiDragAssign(pContext,NULL);if(iRet!=XUI_OK)return iRet;
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
	return __xuiDragRelease(pContext,tDrag.pData,iRet);
}

static void __xuiDragClear(xui_context pContext, int bKeepEffect)
{
	xui_data_object pData = pContext->pTransferData;

	int assigned=__xuiDragAssign(pContext,NULL);
	if(assigned!=XUI_OK){xuiInternalContextRecordFailure(pContext,assigned);return;}
	pContext->iDragSessionId++;
	pContext->pDragSource = NULL;
	pContext->pDropTarget = NULL;
	pContext->iDragAllowedEffects = 0u;
	pContext->iDragSuggestedEffect = 0u;
	if ( !bKeepEffect ) pContext->iDragLastEffect = XUI_DRAG_EFFECT_NONE;
	pContext->iDragEffect = XUI_DRAG_EFFECT_NONE;
	pContext->bTransferActive = 0;
	pContext->bTransferExternal = 0;
	pContext->bTransferDropping = 0;
	(void)__xuiDragRelease(pContext,pData,XUI_OK);
}

static int __xuiDragBeginOperation(xui_context pContext, xui_widget pSource,
	xui_data_object pData, uint32_t iAllowedEffects, uint32_t iSuggestedEffect)
{
	if ( !__xuiDragContextReady(pContext) ||
		!__xuiDragWidgetAvailable(pContext, pSource) ||
		!__xuiDataObjectValid(pData) || (pContext->pOwnedProxy && !xuiDataObjectIsOwned(pData)) ||
		(iAllowedEffects & XUI_DRAG_EFFECT_MASK) == 0u ||
		(iAllowedEffects & ~XUI_DRAG_EFFECT_MASK) != 0u ||
		(iSuggestedEffect != 0u &&
		 !__xuiDragEffectValid(iSuggestedEffect, iAllowedEffects)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pContext->bTransferActive ) return XUI_ERROR_INVALID_STATE;
	if ( xuiDataObjectAddRef(pData) != XUI_OK ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext->iDragSessionId++;
	int assigned=__xuiDragAssign(pContext,pData);
	if(assigned!=XUI_OK)return __xuiDragRelease(pContext,pData,assigned);
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
    xui_call_scope_t scope={0};int iRet=xuiCallScopeBegin(pContext,&scope);if(iRet!=XUI_OK)return iRet;
    iRet=__xuiDragBeginOperation(pContext,pSource,pData,iAllowedEffects,iSuggestedEffect);
    xerror* error=xrtTakeError();iRet=xuiInternalCallScopeEndOutcome(&scope,iRet,&error);xrtSetErrorTake(error);return iRet;
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
    xui_call_scope_t scope={0};int status=xuiCallScopeBegin(pContext,&scope);if(status!=XUI_OK)return status;
    xuiInternalDragTransferCancel(pContext);
    xerror* error=xrtTakeError();status=xuiInternalCallScopeEndOutcome(&scope,XUI_OK,&error);xrtSetErrorTake(error);return status;
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
		if ( !__xuiDataObjectValid(pData) || (pContext->pOwnedProxy && !xuiDataObjectIsOwned(pData)) ||
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
			(void)__xuiDragRelease(pContext,pData,XUI_OK);
			if ( pEffect != NULL ) *pEffect = XUI_DRAG_EFFECT_NONE;
			return XUI_OK;
		}
		pContext->iDragSessionId++;
		int assigned=__xuiDragAssign(pContext,pData);
		if(assigned!=XUI_OK)return __xuiDragRelease(pContext,pData,assigned);
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
    xui_call_scope_t scope={0};int iRet=xuiCallScopeBegin(pContext,&scope);if(iRet!=XUI_OK)return iRet;
    iRet=__xuiDragExternalEventOperation(pContext,iType,iX,iY,iModifiers,pData,iAllowedEffects,iSuggestedEffect,pEffect);
    xerror* error=xrtTakeError();iRet=xuiInternalCallScopeEndOutcome(&scope,iRet,&error);xrtSetErrorTake(error);return iRet;
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

typedef struct xui_clipboard_transfer_t {xui_context context;const xn_transfer_services_v1* services;} xui_clipboard_transfer_t;
static bool __xuiTransferValid(const xn_transfer_services_v1* services)
{
    return services && services->iSize==sizeof(*services) && services->iAbiTag==XN_TRANSFER_SERVICES_V1_TAG &&
        !(services->iFlags&~XN_TRANSFER_PROCESS_RESIDENT) && services->wrapDataObject && services->wrapDropSource && services->wrapDropTarget &&
        ((services->iFlags&XN_TRANSFER_PROCESS_RESIDENT) ? services->pCodeCapability==NULL:services->pCodeCapability!=NULL);
}
static int __xuiClipboardPublish(xui_context context,const xui_clipboard_item_t* items,int count,const xn_transfer_services_v1* services)
{
    if(!__xuiTransferValid(services) || !items || count<=0 || count>64)return XUI_ERROR_INVALID_ARGUMENT;
    if(!__xuiDragContextReady(context))return XUI_ERROR_INVALID_STATE;
    if(!context->tProxy.clipboardSetItemsWithTransfer)return XUI_ERROR_UNSUPPORTED;
    return context->tProxy.clipboardSetItemsWithTransfer(&context->tProxy,items,count,services);
}
XUI_API int xuiClipboardSetItemsWithTransfer(xui_context context,const xui_clipboard_item_t* items,int count,const xn_transfer_services_v1* services)
{
    xui_call_scope_t scope={0};int status=xuiCallScopeBegin(context,&scope);
    if(status!=XUI_OK)return status;
    status=__xuiClipboardPublish(context,items,count,services);
    xerror* error=xrtTakeError();status=xuiInternalCallScopeEndOutcome(&scope,status,&error);xrtSetErrorTake(error);return status;
}
static int __xuiDataClipboardConsume(const xui_clipboard_item_t* items,int count,void* opaque)
{xui_clipboard_transfer_t* transfer=opaque;return __xuiClipboardPublish(transfer->context,items,count,transfer->services);}
XUI_API int xuiClipboardCopyDataWithTransfer(xui_context context,xui_data_object data,const xn_transfer_services_v1* services)
{
    if(!__xuiTransferValid(services))return XUI_ERROR_INVALID_ARGUMENT;
    xui_call_scope_t scope={0};int status=xuiCallScopeBegin(context,&scope);
    if(status!=XUI_OK)return status;
    xui_clipboard_transfer_t transfer={context,services};
    status=context->tProxy.clipboardSetItemsWithTransfer ? __xuiDataSnapshotCopy(data,__xuiDataClipboardConsume,&transfer):XUI_ERROR_UNSUPPORTED;
    xerror* error=xrtTakeError();status=xuiInternalCallScopeEndOutcome(&scope,status,&error);xrtSetErrorTake(error);return status;
}
#endif
