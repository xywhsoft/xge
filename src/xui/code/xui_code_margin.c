#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui_code_geometry_internal.h"
#include "xui_code_margin_internal.h"
#include <stdlib.h>
#include "../core/xui_callback_hold_internal.h"

#include <float.h>
#include <string.h>

struct xui_code_margin_slot_t {
    struct xui_code_margin_slot_t* next;
    size_t refs;
    xui_callback_owner_t owner;
    void* user;
    union {xui_code_margin_render_proc render;xui_code_margin_event_proc event;} fn;
    int id,role;
};
struct xui_code_margin_model_t {
	xui_code_margin_desc_t* pMargins;
	int iCount;
	int iCapacity;
    uint32_t iMagic;
    bool bDestroyPending,bDestroying;
    uint64_t iCreatorThread;
    size_t iActiveCalls;
    xui_lifetime_link_t* pLifetimeLinks;
    xui_widget pInternalOwner;
    xui_code_margin_slot_t* slots;
};

#define XUI_CODE_MARGIN_MAGIC UINT32_C(0x58434d47)
static int __xuiCodeMarginReadStatus(xui_code_margin_model model)
{
    if(!model)return XUI_ERROR_INVALID_ARGUMENT;
    if(model->iCreatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(model->iMagic!=XUI_CODE_MARGIN_MAGIC || model->bDestroyPending || model->bDestroying ||
        (model->pInternalOwner && !xuiInternalWidgetIsValid(model->pInternalOwner)))return XUI_ERROR_INVALID_STATE;
    return XUI_OK;
}
static void __xuiCodeMarginChanged(xui_code_margin_model model, uint32_t flags)
{
    xui_widget owner = model->pInternalOwner;
    if ( owner == NULL || (owner->iDirtyFlags & flags) == flags ) return;
    xerror* prior = xrtTakeError();
    if ( xuiWidgetInvalidate(owner, flags) != XUI_OK ) {
        xrtClearError();
        (void)xuiInternalContextInvalidateAll(owner->pContext);
    }
    xrtClearError();
    xrtSetErrorTake(prior);
}

static void __xuiCodeMarginGeometryChanged(xui_code_margin_model model)
{
    __xuiCodeMarginChanged(model, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

/* Unlink only while holding the mutation gate. User Drop always runs after
 * the graph update, and a native call scope protects reentrant destruction. */
static xui_code_margin_slot_t* __xuiCodeMarginDetach(xui_code_margin_model model,int id,int role)
{
    xui_code_margin_slot_t* detached=NULL; xui_code_margin_slot_t** at=&model->slots;
    while(*at){
        xui_code_margin_slot_t* node=*at;
        if((!id || node->id==id) && (!role || node->role==role)){
            *at=node->next;node->next=detached;detached=node;
        }else at=&node->next;
    }
    return detached;
}
static void __xuiCodeMarginRelease(xui_context context,xui_code_margin_slot_t* node,xui_cleanup_receipt_t* receipt)
{
    if(!node)return;
    if(!node->refs)abort();
    if(--node->refs)return;
    xui_callback_owner_t owner=node->owner;xrtFree(node);
    xuiInternalCleanupAdd(receipt,xuiInternalCallbackOwnerDropOutcome(context,&owner,receipt->status,receipt->error));
}
static void __xuiCodeMarginReleaseList(xui_context context,xui_code_margin_slot_t* nodes,xui_cleanup_receipt_t* receipt)
{
    while(nodes){xui_code_margin_slot_t* next=nodes->next;nodes->next=NULL;__xuiCodeMarginRelease(context,nodes,receipt);nodes=next;}
}
static int __xuiCodeMarginFinish(xui_code_margin_scope_t*,xui_cleanup_receipt_t*);
static int __xuiCodeMarginDestroyNow(xui_code_margin_model model)
{
    model->bDestroying=true;xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,XUI_OK);
    xuiInternalLifetimeInvalidateAll(&model->pLifetimeLinks);xuiInternalCleanupAdd(&receipt,XUI_OK);
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))abort();
    xui_code_margin_slot_t* nodes=__xuiCodeMarginDetach(model,0,0);
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    __xuiCodeMarginReleaseList(NULL,nodes,&receipt);
    xrtFree(model->pMargins);model->iMagic=0;xrtFree(model);
    return xuiInternalCleanupEnd(NULL,&receipt);
}
static int __xuiCodeMarginDestroyOwnerChecked(xui_code_margin_model model,xui_widget owner,bool* consumed)
{
    if(!consumed)return XUI_ERROR_INVALID_ARGUMENT;
    *consumed=false;
    if(!model || model->iMagic!=XUI_CODE_MARGIN_MAGIC)return XUI_ERROR_INVALID_ARGUMENT;
    if(model->iCreatorThread!=xrtThreadCurrentId() || model->bDestroying)return XUI_ERROR_BUSY;
    if(model->bDestroyPending || model->pInternalOwner!=owner)return XUI_ERROR_INVALID_STATE;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    model->bDestroyPending=true;*consumed=true;if(!xrtOwnershipScopeEnd(&mutation))abort();
    return model->iActiveCalls ? XUI_OK:__xuiCodeMarginDestroyNow(model);
}
XUI_API int xuiCodeMarginModelDestroyOwnerChecked(xui_code_margin_model model,bool* consumed)
{return __xuiCodeMarginDestroyOwnerChecked(model,NULL,consumed);}
XUI_API int xuiCodeMarginModelDestroyChecked(xui_code_margin_model model)
{bool consumed=false;return xuiCodeMarginModelDestroyOwnerChecked(model,&consumed);}
int xuiInternalCodeMarginClaimOwner(xui_code_margin_model model,xui_widget owner)
{
    if(!model || model->iMagic!=XUI_CODE_MARGIN_MAGIC || !xuiInternalWidgetIsValid(owner))return XUI_ERROR_INVALID_ARGUMENT;
    if(model->iCreatorThread!=xrtThreadCurrentId() || model->iActiveCalls)return XUI_ERROR_BUSY;
    if(model->pInternalOwner || model->bDestroyPending)return XUI_ERROR_INVALID_STATE;
    model->pInternalOwner=owner;return XUI_OK;
}
int xuiInternalCodeMarginDestroyOwned(xui_widget owner,xui_code_margin_model model)
{bool consumed=false;return __xuiCodeMarginDestroyOwnerChecked(model,model && model->pInternalOwner ? owner:NULL,&consumed);}
XUI_API bool xuiCodeMarginModelOwnershipBusy(xui_code_margin_model model)
{return !model || model->iCreatorThread!=xrtThreadCurrentId() || model->iMagic!=XUI_CODE_MARGIN_MAGIC || model->iActiveCalls || model->bDestroying;}
XUI_API int xuiCodeMarginModelLifetimeWatch(xui_code_margin_model model,xui_lifetime_link_t* link,xui_lifetime_invalidate_proc invalidated,void* user)
{
    int status=__xuiCodeMarginReadStatus(model);if(status!=XUI_OK)return status;
    if(!link || !invalidated)return XUI_ERROR_INVALID_ARGUMENT;
    if(link->pNext || link->ppPrevious || link->onInvalidate || link->pUser)return XUI_ERROR_INVALID_STATE;
    link->pNext=model->pLifetimeLinks;link->ppPrevious=&model->pLifetimeLinks;link->onInvalidate=invalidated;link->pUser=user;
    if(link->pNext)link->pNext->ppPrevious=&link->pNext;
    model->pLifetimeLinks=link;return XUI_OK;
}
XUI_API int xuiCodeMarginModelEditorLifetimeWatch(xui_code_margin_model model,
    xui_lifetime_link_t* link, xui_lifetime_invalidate_proc invalidated, void* user)
{
    int status = __xuiCodeMarginReadStatus(model);
    if ( status != XUI_OK ) return status;
    if ( model->pInternalOwner == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
    return xuiWidgetLifetimeWatch(model->pInternalOwner, link, invalidated, user);
}

XUI_API int xuiCodeMarginModelCallBegin(xui_code_margin_model model,xui_code_margin_scope_t* scope)
{
    if(!scope || scope->pSelf || scope->pModel || scope->pContext || scope->tContext.pSelf)return XUI_ERROR_INVALID_ARGUMENT;
    int status=__xuiCodeMarginReadStatus(model);if(status!=XUI_OK)return status;
    if(model->iActiveCalls==SIZE_MAX)return XUI_ERROR_INVALID_STATE;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    xui_context context=model->pInternalOwner ? model->pInternalOwner->pContext:NULL;
    if(context)status=xuiCallScopeBegin(context,&scope->tContext);
    else scope->tContext._pFirstError=xrtTakeError();
    if(status==XUI_OK){++model->iActiveCalls;scope->pSelf=scope;scope->pModel=model;scope->pContext=context;}
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    return status;
}
static int __xuiCodeMarginFinish(xui_code_margin_scope_t* scope,xui_cleanup_receipt_t* receipt)
{
    if(!scope || scope->pSelf!=scope || !scope->pModel)return XUI_ERROR_INVALID_ARGUMENT;
    xui_code_margin_model model=scope->pModel;
    if(model->iCreatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    xerror* ambient=scope->pContext ? NULL:scope->tContext._pFirstError;
    if(!scope->pContext)scope->tContext._pFirstError=NULL;
    if(model->bDestroyPending && receipt->status==XUI_OK)receipt->status=XUI_ERROR_INVALID_STATE;
    if(scope->pContext)receipt->status=xuiInternalCallScopeEndOutcome(&scope->tContext,receipt->status,&receipt->error);
    /* Context tail cleanup can consume the unique child after entry above. */
    if(model->bDestroyPending)xuiInternalCleanupAdd(receipt,XUI_ERROR_INVALID_STATE);
    scope->pSelf=NULL;scope->pModel=NULL;scope->pContext=NULL;
    if(!model->iActiveCalls)abort();
    --model->iActiveCalls;
    if(model->bDestroyPending && !model->iActiveCalls)xuiInternalCleanupAdd(receipt,__xuiCodeMarginDestroyNow(model));
    if(ambient){
        if(receipt->status==XUI_OK){xrtErrorFree(receipt->error);receipt->error=ambient;}
        else xrtErrorFree(ambient);
    }
    return xuiInternalCleanupEnd(NULL,receipt);
}
XUI_API int xuiCodeMarginModelCallEnd(xui_code_margin_scope_t* scope)
{
    /* Invalid scope endings must not consume either the pin or ambient TLS. */
    if(!scope || scope->pSelf!=scope || !scope->pModel)return XUI_ERROR_INVALID_ARGUMENT;
    if(scope->pModel->iCreatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,XUI_OK);
    return __xuiCodeMarginFinish(scope,&receipt);
}
XUI_API bool xuiCodeMarginModelTraceOwnership(xui_code_margin_model model,xrtownershipvisitor visit,void* user)
{
    if(!visit || __xuiCodeMarginReadStatus(model)!=XUI_OK || model->iActiveCalls)return false;
    for(xui_code_margin_slot_t* node=model->slots;node;node=node->next)
        if(node->owner.onHold && !visit(node->owner.tReference,user))return false;
    return true;
}


static int __xuiCodeMarginReserve(xui_code_margin_model pModel, int iCapacity)
{
	xui_code_margin_desc_t* pNew;
	int iNewCapacity;

	if ( pModel == NULL || iCapacity < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= pModel->iCapacity ) return XUI_OK;
	iNewCapacity = (pModel->iCapacity > 0) ? pModel->iCapacity : 8;
	while ( iNewCapacity < iCapacity ) {
		if ( iNewCapacity > INT_MAX / 2 ) {
			iNewCapacity = iCapacity;
			break;
		}
		iNewCapacity *= 2;
	}
	if ( (size_t)iNewCapacity > SIZE_MAX / sizeof(*pNew) ) return XUI_ERROR_LIMIT_EXCEEDED;
	pNew = (xui_code_margin_desc_t*)xrtRealloc(pModel->pMargins,
		sizeof(*pNew) * (size_t)iNewCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pModel->pMargins = pNew;
	pModel->iCapacity = iNewCapacity;
	return XUI_OK;
}

static int __xuiCodeMarginRectFromEdges(double fLeft, double fRight,
	xui_rect_t tViewport, xui_rect_t* pRect)
{
	if ( pRect == NULL || fRight < fLeft ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiCodeInternalRectFromDouble(fLeft, tViewport.fY,
		fRight - fLeft, tViewport.fH, pRect);
}

static int __xuiCodeMarginFind(xui_code_margin_model pModel, int iId)
{
	int i;

	for ( i = 0; i < pModel->iCount; i++ ) {
		if ( pModel->pMargins[i].iId == iId ) return i;
	}
	return -1;
}

static int __xuiCodeMarginVisible(const xui_code_margin_desc_t* pDesc)
{
	return (pDesc->iFlags & XUI_CODE_MARGIN_VISIBLE) != 0 && pDesc->fWidth > 0.0f;
}

static void __xuiCodeMarginCopyInfo(xui_code_margin_info_t* pInfo, const xui_code_margin_desc_t* pDesc, xui_rect_t tRect)
{
	memset(pInfo, 0, sizeof(*pInfo));
	pInfo->iSize = sizeof(*pInfo);
	pInfo->iId = pDesc->iId;
	pInfo->iKind = pDesc->iKind;
	pInfo->fWidth = pDesc->fWidth;
	pInfo->iFlags = pDesc->iFlags;
	pInfo->tRect = tRect;
	pInfo->onRender = pDesc->onRender;
	pInfo->onEvent = pDesc->onEvent;
	pInfo->pUser = pDesc->pUser;
}

static xui_code_margin_desc_t __xuiCodeMarginDefault(int iId, int iKind, float fWidth, uint32_t iFlags)
{
	xui_code_margin_desc_t tDesc;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iId = iId;
	tDesc.iKind = iKind;
	tDesc.fWidth = fWidth;
	tDesc.iFlags = iFlags | XUI_CODE_MARGIN_VISIBLE;
	return tDesc;
}

XUI_API int xuiCodeMarginModelCreate(xui_code_margin_model* ppModel)
{
	xui_code_margin_model pModel;

	if ( ppModel == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppModel = NULL;
	pModel = (xui_code_margin_model)xrtMalloc(sizeof(*pModel));
	if ( pModel == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pModel, 0, sizeof(*pModel));
    pModel->iMagic=XUI_CODE_MARGIN_MAGIC;pModel->iCreatorThread=xrtThreadCurrentId();
	*ppModel = pModel;
	return XUI_OK;
}

XUI_API void xuiCodeMarginModelDestroy(xui_code_margin_model pModel)
{if(pModel)(void)xuiCodeMarginModelDestroyChecked(pModel);}
XUI_API int xuiCodeMarginModelClearChecked(xui_code_margin_model pModel)
{
    xui_code_margin_scope_t scope={0};int status=xuiCodeMarginModelCallBegin(pModel,&scope);if(status!=XUI_OK)return status;
    xrtownershipscope mutation={0};xui_code_margin_slot_t* nodes=NULL;
    if(!xrtOwnershipMutationBegin(&mutation))status=XUI_ERROR_INVALID_STATE;
    else {pModel->iCount=0;nodes=__xuiCodeMarginDetach(pModel,0,0);if(!xrtOwnershipScopeEnd(&mutation))abort();}
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,status);
    if ( status == XUI_OK ) __xuiCodeMarginGeometryChanged(pModel);
    __xuiCodeMarginReleaseList(scope.pContext,nodes,&receipt);
    return __xuiCodeMarginFinish(&scope,&receipt);
}
XUI_API void xuiCodeMarginModelClear(xui_code_margin_model pModel)
{if(pModel)(void)xuiCodeMarginModelClearChecked(pModel);}

XUI_API int xuiCodeMarginModelLoadDefaults(xui_code_margin_model pModel, int bLineNumber, int bMarker, int bFold, int bDiagnostic)
{
	xui_code_margin_model_t tNew;
	xui_code_margin_desc_t tDesc;
	int iRet;

	int modelStatus=__xuiCodeMarginReadStatus(pModel);if(modelStatus!=XUI_OK)return modelStatus;
	if ( pModel == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tNew, 0, sizeof(tNew));
    tNew.iMagic=XUI_CODE_MARGIN_MAGIC;tNew.iCreatorThread=pModel->iCreatorThread;
	tDesc = __xuiCodeMarginDefault(1, XUI_CODE_MARGIN_CHANGE, 4.0f, 0);
	iRet = xuiCodeMarginModelAdd(&tNew, &tDesc);
	if ( iRet != XUI_OK ) goto cleanup;
	if ( bMarker ) {
		tDesc = __xuiCodeMarginDefault(2, XUI_CODE_MARGIN_MARKER, 16.0f, XUI_CODE_MARGIN_CLICKABLE);
		iRet = xuiCodeMarginModelAdd(&tNew, &tDesc);
		if ( iRet != XUI_OK ) goto cleanup;
	}
	if ( bFold ) {
		tDesc = __xuiCodeMarginDefault(3, XUI_CODE_MARGIN_FOLD, 14.0f, XUI_CODE_MARGIN_CLICKABLE);
		iRet = xuiCodeMarginModelAdd(&tNew, &tDesc);
		if ( iRet != XUI_OK ) goto cleanup;
	}
	if ( bLineNumber ) {
		tDesc = __xuiCodeMarginDefault(4, XUI_CODE_MARGIN_LINE_NUMBER, 44.0f, 0);
		iRet = xuiCodeMarginModelAdd(&tNew, &tDesc);
		if ( iRet != XUI_OK ) goto cleanup;
	}
	if ( bDiagnostic ) {
		tDesc = __xuiCodeMarginDefault(5, XUI_CODE_MARGIN_DIAGNOSTIC, 14.0f, XUI_CODE_MARGIN_CLICKABLE);
		iRet = xuiCodeMarginModelAdd(&tNew, &tDesc);
		if ( iRet != XUI_OK ) goto cleanup;
	}
    xui_code_margin_scope_t scope={0};iRet=xuiCodeMarginModelCallBegin(pModel,&scope);if(iRet!=XUI_OK)goto cleanup;
    xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation)){
        xui_cleanup_receipt_t failed={0};xuiInternalCleanupStart(&failed,XUI_ERROR_INVALID_STATE);
        xrtFree(tNew.pMargins);return __xuiCodeMarginFinish(&scope,&failed);
    }
    xui_code_margin_slot_t* nodes=__xuiCodeMarginDetach(pModel,0,0);
	xrtFree(pModel->pMargins);
	pModel->pMargins = tNew.pMargins;
	pModel->iCount = tNew.iCount;
	pModel->iCapacity = tNew.iCapacity;
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,XUI_OK);
    __xuiCodeMarginGeometryChanged(pModel);
    __xuiCodeMarginReleaseList(scope.pContext,nodes,&receipt);
    return __xuiCodeMarginFinish(&scope,&receipt);

cleanup:
	xrtFree(tNew.pMargins);
	return iRet;
}

XUI_API int xuiCodeMarginModelAdd(xui_code_margin_model pModel, const xui_code_margin_desc_t* pDesc)
{
	int iIndex;
	int iRet;

	int modelStatus=__xuiCodeMarginReadStatus(pModel);if(modelStatus!=XUI_OK)return modelStatus;
	if ( (pModel == NULL) || (pDesc == NULL) ||
	     (pDesc->iSize < sizeof(*pDesc)) || (pDesc->iId <= 0) ||
	     (pDesc->iKind <= 0) || !isfinite((double)pDesc->fWidth) ||
	     (pDesc->fWidth < 0.0f) ) return XUI_ERROR_INVALID_ARGUMENT;
    /* Input may alias the current array; copy before reserve or user Drop. */
    xui_code_margin_desc_t desc=*pDesc;
    xui_code_margin_scope_t scope={0};iRet=xuiCodeMarginModelCallBegin(pModel,&scope);if(iRet!=XUI_OK)return iRet;
    xui_code_margin_slot_t* nodes=NULL;xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))iRet=XUI_ERROR_INVALID_STATE;
    else {
        iIndex=__xuiCodeMarginFind(pModel,desc.iId);
        if(iIndex<0){
            iRet=pModel->iCount==INT_MAX ? XUI_ERROR_LIMIT_EXCEEDED:__xuiCodeMarginReserve(pModel,pModel->iCount+1);
            if(iRet==XUI_OK)iIndex=pModel->iCount++;
        }
        if(iRet==XUI_OK){
            pModel->pMargins[iIndex]=desc;pModel->pMargins[iIndex].iSize=sizeof(desc);
            nodes=__xuiCodeMarginDetach(pModel,desc.iId,0);
        }
        if(!xrtOwnershipScopeEnd(&mutation))abort();
    }
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,iRet);
    if ( iRet == XUI_OK ) __xuiCodeMarginGeometryChanged(pModel);
    __xuiCodeMarginReleaseList(scope.pContext,nodes,&receipt);
    return __xuiCodeMarginFinish(&scope,&receipt);
}

XUI_API int xuiCodeMarginModelRemove(xui_code_margin_model pModel, int iId)
{
	int iIndex;

	int modelStatus=__xuiCodeMarginReadStatus(pModel);if(modelStatus!=XUI_OK)return modelStatus;
	if ( (pModel == NULL) || (iId <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiCodeMarginFind(pModel, iId);
	if ( iIndex < 0 ) return XUI_ERROR_UNSUPPORTED;
    xui_code_margin_scope_t scope={0};int status=xuiCodeMarginModelCallBegin(pModel,&scope);if(status!=XUI_OK)return status;
    xui_code_margin_slot_t* nodes=NULL;xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))status=XUI_ERROR_INVALID_STATE;
    else {
        memmove(&pModel->pMargins[iIndex],&pModel->pMargins[iIndex+1],sizeof(pModel->pMargins[iIndex])*(size_t)(pModel->iCount-iIndex-1));
        --pModel->iCount;nodes=__xuiCodeMarginDetach(pModel,iId,0);
        if(!xrtOwnershipScopeEnd(&mutation))abort();
    }
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,status);
    if ( status == XUI_OK ) __xuiCodeMarginGeometryChanged(pModel);
    __xuiCodeMarginReleaseList(scope.pContext,nodes,&receipt);
    return __xuiCodeMarginFinish(&scope,&receipt);
}

XUI_API int xuiCodeMarginModelSetWidth(xui_code_margin_model pModel, int iId, float fWidth)
{
	int iIndex;

	int modelStatus=__xuiCodeMarginReadStatus(pModel);if(modelStatus!=XUI_OK)return modelStatus;
	if ( (pModel == NULL) || (iId <= 0) || !isfinite((double)fWidth) ||
	     (fWidth < 0.0f) ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiCodeMarginFind(pModel, iId);
	if ( iIndex < 0 ) return XUI_ERROR_UNSUPPORTED;
	if ( pModel->pMargins[iIndex].fWidth == fWidth ) return XUI_OK;
	if ( pModel->pInternalOwner && pModel->pInternalOwner->pContext->bGuardedCallback )
		return XUI_ERROR_BUSY;
	pModel->pMargins[iIndex].fWidth = fWidth;
	__xuiCodeMarginGeometryChanged(pModel);
	return XUI_OK;
}

XUI_API int xuiCodeMarginModelSetVisible(xui_code_margin_model pModel, int iId, int bVisible)
{
	int iIndex;

	int modelStatus=__xuiCodeMarginReadStatus(pModel);if(modelStatus!=XUI_OK)return modelStatus;
	if ( (pModel == NULL) || (iId <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiCodeMarginFind(pModel, iId);
	if ( iIndex < 0 ) return XUI_ERROR_UNSUPPORTED;
	uint32_t oldFlags = pModel->pMargins[iIndex].iFlags;
	uint32_t flags = bVisible ? oldFlags | XUI_CODE_MARGIN_VISIBLE : oldFlags & ~XUI_CODE_MARGIN_VISIBLE;
	if ( flags == oldFlags ) return XUI_OK;
	if ( pModel->pInternalOwner && pModel->pInternalOwner->pContext->bGuardedCallback ) return XUI_ERROR_BUSY;
	pModel->pMargins[iIndex].iFlags = flags;
	__xuiCodeMarginGeometryChanged(pModel);
	return XUI_OK;
}

XUI_API int xuiCodeMarginModelGetCount(xui_code_margin_model pModel)
{
	return (__xuiCodeMarginReadStatus(pModel)==XUI_OK) ? pModel->iCount : 0;
}

XUI_API int xuiCodeMarginModelGet(xui_code_margin_model pModel, int iIndex, xui_code_margin_info_t* pInfo)
{
	xui_rect_t tRect;

	int modelStatus=__xuiCodeMarginReadStatus(pModel);if(modelStatus!=XUI_OK)return modelStatus;
	if ( (pModel == NULL) || (pInfo == NULL) || (iIndex < 0) || (iIndex >= pModel->iCount) ) return XUI_ERROR_INVALID_ARGUMENT;
    for(xui_code_margin_slot_t* node=pModel->slots;node;node=node->next)
        if(node->id==pModel->pMargins[iIndex].iId)return XUI_ERROR_UNSUPPORTED;
	memset(&tRect, 0, sizeof(tRect));
	__xuiCodeMarginCopyInfo(pInfo, &pModel->pMargins[iIndex], tRect);
	return XUI_OK;
}

XUI_API int xuiCodeMarginModelGetTotalWidth(xui_code_margin_model pModel, float* pWidth)
{
	double fWidth;
	int i;

	int modelStatus=__xuiCodeMarginReadStatus(pModel);if(modelStatus!=XUI_OK)return modelStatus;
	if ( (pModel == NULL) || (pWidth == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	fWidth = 0.0f;
	for ( i = 0; i < pModel->iCount; i++ ) {
		if ( __xuiCodeMarginVisible(&pModel->pMargins[i]) ) {
			fWidth += (double)pModel->pMargins[i].fWidth;
			if ( fWidth > FLT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
		}
	}
	*pWidth = (float)fWidth;
	return XUI_OK;
}

XUI_API int xuiCodeMarginModelGetRect(xui_code_margin_model pModel, int iId, xui_rect_t tViewport, xui_rect_t* pRect)
{
	xui_rect_t tRect;
	double fCurrentX;
	double fNextX;
	int i;
	int iRet;

	int modelStatus=__xuiCodeMarginReadStatus(pModel);if(modelStatus!=XUI_OK)return modelStatus;
	if ( (pModel == NULL) || (iId <= 0) || (pRect == NULL) ||
	     !xuiCodeInternalRectValid(tViewport) ) return XUI_ERROR_INVALID_ARGUMENT;
	fCurrentX = tViewport.fX;
	for ( i = 0; i < pModel->iCount; i++ ) {
		if ( pModel->pMargins[i].iId == iId ) {
			if ( !__xuiCodeMarginVisible(&pModel->pMargins[i]) ) return XUI_ERROR_UNSUPPORTED;
			fNextX = fCurrentX + (double)pModel->pMargins[i].fWidth;
			return __xuiCodeMarginRectFromEdges(fCurrentX, fNextX, tViewport, pRect);
		}
		if ( __xuiCodeMarginVisible(&pModel->pMargins[i]) ) {
			fNextX = fCurrentX + (double)pModel->pMargins[i].fWidth;
			iRet = __xuiCodeMarginRectFromEdges(fCurrentX, fNextX, tViewport, &tRect);
			if ( iRet != XUI_OK ) return iRet;
			fCurrentX = fNextX;
		}
	}
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiCodeMarginLayout(xui_code_margin_model pModel,xui_rect_t tViewport,xui_code_margin_info_t* pInfos,xui_code_margin_entry_t* pEntries,int iInfoCapacity,int* pInfoCount,xui_rect_t* pTextRect)
{
	xui_rect_t tRect;
	xui_rect_t tTextRect;
	double fX;
	double fNextX;
	int iTextLeft;
	int iViewportRight;
	int i;
	int iCount;
	int iRet;

	if ( pInfoCount != NULL ) *pInfoCount = 0;
	int modelStatus=__xuiCodeMarginReadStatus(pModel);if(modelStatus!=XUI_OK)return modelStatus;
	if ( (pModel == NULL) || (pInfoCount == NULL) || (iInfoCapacity < 0) ||
	     (pInfos == NULL && pEntries == NULL && iInfoCapacity > 0) ||
	     !xuiCodeInternalRectValid(tViewport) ) return XUI_ERROR_INVALID_ARGUMENT;
	fX = tViewport.fX;
	iCount = 0;
	for ( i = 0; i < pModel->iCount; i++ ) {
		if ( !__xuiCodeMarginVisible(&pModel->pMargins[i]) ) continue;
		fNextX = fX + (double)pModel->pMargins[i].fWidth;
		iRet = __xuiCodeMarginRectFromEdges(fX, fNextX, tViewport, &tRect);
		if ( iRet != XUI_OK ) return iRet;
		fX = fNextX;
		iCount++;
	}
	iRet = xuiCodeInternalSnapPixel(fX, &iTextLeft);
	if ( iRet != XUI_OK ) return iRet;
	iViewportRight = tViewport.fX + tViewport.fW;
	if ( iTextLeft < tViewport.fX ) iTextLeft = tViewport.fX;
	if ( iTextLeft > iViewportRight ) iTextLeft = iViewportRight;
	tTextRect = (xui_rect_t){ iTextLeft, tViewport.fY,
		iViewportRight - iTextLeft, tViewport.fH };
	if ( iInfoCapacity == 0 ) {
		*pInfoCount = iCount;
		if ( pTextRect != NULL ) *pTextRect = tTextRect;
		return XUI_OK;
	}
	fX = tViewport.fX;
	iCount = 0;
	for ( i = 0; i < pModel->iCount; i++ ) {
		if ( !__xuiCodeMarginVisible(&pModel->pMargins[i]) ) continue;
		fNextX = fX + (double)pModel->pMargins[i].fWidth;
		iRet = __xuiCodeMarginRectFromEdges(fX, fNextX, tViewport, &tRect);
		if ( iRet != XUI_OK ) return iRet;
        if(iCount<iInfoCapacity){
            if(pInfos)__xuiCodeMarginCopyInfo(&pInfos[iCount],&pModel->pMargins[i],tRect);
            if(pEntries){__xuiCodeMarginCopyInfo(&pEntries[iCount].info,&pModel->pMargins[i],tRect);pEntries[iCount].render=NULL;}
        }
		fX = fNextX;
		iCount++;
	}
	*pInfoCount = iCount;
	if ( pTextRect != NULL ) {
		*pTextRect = tTextRect;
	}
	return XUI_OK;
}

XUI_API int xuiCodeMarginModelLayout(xui_code_margin_model model,xui_rect_t viewport,xui_code_margin_info_t* infos,int capacity,int* count,xui_rect_t* text)
{
    int status=__xuiCodeMarginReadStatus(model);if(status!=XUI_OK)return status;
    if(model->slots)return XUI_ERROR_UNSUPPORTED;
    return __xuiCodeMarginLayout(model,viewport,infos,NULL,capacity,count,text);
}

XUI_API int xuiCodeMarginModelMeasureLayout(xui_code_margin_model model, xui_rect_t viewport,
    int* count, xui_rect_t* text)
{
    return __xuiCodeMarginLayout(model, viewport, NULL, NULL, 0, count, text);
}

struct xui_code_margin_layout_t {
    size_t refs, count;
    int total;
    xui_rect_t text;
    xui_code_margin_geometry_t entries[];
};

XUI_API int xuiCodeMarginModelCaptureLayout(xui_code_margin_model model, xui_rect_t viewport,
    int limit, xui_code_margin_layout_t** out)
{
    if (!out) return XUI_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    if (limit < -1) return XUI_ERROR_INVALID_ARGUMENT;

    int total;
    xui_rect_t text;
    int status = xuiCodeMarginModelMeasureLayout(model, viewport, &total, &text);
    if (status != XUI_OK) return status;
    size_t count = (size_t)(limit >= 0 && limit < total ? limit : total);
    if (count > (SIZE_MAX - sizeof(xui_code_margin_layout_t)) / sizeof(xui_code_margin_geometry_t))
        return XUI_ERROR_LIMIT_EXCEEDED;
    xui_code_margin_layout_t* layout = xrtMalloc(sizeof(*layout) + count * sizeof(*layout->entries));
    if (!layout) return XUI_ERROR_OUT_OF_MEMORY;
    layout->refs = 1;
    layout->count = count;
    layout->total = total;
    layout->text = text;

    double left = viewport.fX;
    size_t written = 0;
    for (int i = 0; written < count; ++i) {
        const xui_code_margin_desc_t* desc = &model->pMargins[i];
        if (!__xuiCodeMarginVisible(desc)) continue;
        double right = left + (double)desc->fWidth;
        xui_rect_t rect;
        status = __xuiCodeMarginRectFromEdges(left, right, viewport, &rect);
        if (status != XUI_OK) {
            xrtFree(layout);
            return status;
        }
        layout->entries[written++] = (xui_code_margin_geometry_t){
            {sizeof(xui_code_margin_state_t), desc->iId, desc->iKind, desc->fWidth, desc->iFlags}, rect};
        left = right;
    }
    *out = layout;
    return XUI_OK;
}

XUI_API int xuiCodeMarginLayoutAddRef(xui_code_margin_layout_t* layout)
{
    if (!layout) return XUI_ERROR_INVALID_ARGUMENT;
    if (layout->refs == SIZE_MAX) return XUI_ERROR_LIMIT_EXCEEDED;
    ++layout->refs;
    return XUI_OK;
}

XUI_API void xuiCodeMarginLayoutRelease(xui_code_margin_layout_t* layout)
{
    if (layout && --layout->refs == 0) xrtFree(layout);
}

XUI_API int xuiCodeMarginLayoutGetState(const xui_code_margin_layout_t* layout, int* total, xui_rect_t* text)
{
    if (!layout || !total || !text) return XUI_ERROR_INVALID_ARGUMENT;
    *total = layout->total;
    *text = layout->text;
    return XUI_OK;
}

XUI_API int xuiCodeMarginLayoutGetEntries(const xui_code_margin_layout_t* layout,
    const xui_code_margin_geometry_t** entries, size_t* count)
{
    if (!layout || !entries || !count) return XUI_ERROR_INVALID_ARGUMENT;
    *entries = layout->count ? layout->entries : NULL;
    *count = layout->count;
    return XUI_OK;
}

XUI_API int xuiCodeMarginModelHitTest(xui_code_margin_model pModel, xui_rect_t tViewport, float fX, float fY, float fLineHeight, float fScrollY, xui_code_margin_hit_t* pHit)
{
	xui_rect_t tRect;
	double fCurrentX;
	double fNextX;
	double fLine;
	int i;
	int iRet;

	if ( pHit != NULL ) {
		memset(pHit, 0, sizeof(*pHit));
		pHit->iSize = sizeof(*pHit);
	}
	int modelStatus=__xuiCodeMarginReadStatus(pModel);if(modelStatus!=XUI_OK)return modelStatus;
	if ( (pModel == NULL) || (pHit == NULL) ||
	     !xuiCodeInternalRectValid(tViewport) ||
	     !isfinite((double)fX) || !isfinite((double)fY) ||
	     !isfinite((double)fLineHeight) || fLineHeight <= 0.0f ||
	     !isfinite((double)fScrollY) || fScrollY < 0.0f ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( fY < tViewport.fY || fY >= tViewport.fY + tViewport.fH ) return XUI_ERROR_UNSUPPORTED;
	if ( fX < tViewport.fX || fX >= tViewport.fX + tViewport.fW ) return XUI_ERROR_UNSUPPORTED;
	fCurrentX = tViewport.fX;
	for ( i = 0; i < pModel->iCount; i++ ) {
		if ( !__xuiCodeMarginVisible(&pModel->pMargins[i]) ) continue;
		fNextX = fCurrentX + (double)pModel->pMargins[i].fWidth;
		iRet = __xuiCodeMarginRectFromEdges(fCurrentX, fNextX, tViewport, &tRect);
		if ( iRet != XUI_OK ) return iRet;
		if ( fX >= tRect.fX && fX < tRect.fX + tRect.fW ) {
			pHit->iId = pModel->pMargins[i].iId;
			pHit->iKind = pModel->pMargins[i].iKind;
			pHit->iIndex = i;
			fLine = ((double)fY - tViewport.fY + fScrollY) / fLineHeight;
			if ( fLine < 0.0 || fLine > INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
			pHit->iLine = (int)fLine;
			pHit->tRect = tRect;
			return XUI_OK;
		}
		fCurrentX = fNextX;
	}
	return XUI_ERROR_UNSUPPORTED;
}

XUI_API int xuiCodeMarginModelFindHit(xui_code_margin_model model, xui_rect_t viewport,
    float x, float y, float lineHeight, float scrollY, xui_code_margin_hit_t* hit)
{
    int status = xuiCodeMarginModelHitTest(model, viewport, x, y, lineHeight, scrollY, hit);
    return status == XUI_ERROR_UNSUPPORTED ? XUI_OK : status;
}

#include "xui_code_margin_owned.inl"
#endif
