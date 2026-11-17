#include "xui_config.h"
#if XUI_ENABLE_WORKFLOW
#include "xui_internal.h"
#include "xui_flow_graph_widget_internal.h"

#include <string.h>

typedef struct xui_workflow_widget_data_t {
	xui_workflow pWorkflow;
	xui_workflow pRetiredWorkflow; /* One actual owning ref awaiting checked cleanup. */
	int bOwnWorkflow;
	int bWorkflowMutationActive;
	xui_widget pCanvas;
	xui_flow_context_proc onContext;
	void* pContextUser;
} xui_workflow_widget_data_t;

/* Typed native snapshot, retained through the callback's physical return.
 * Scalar arguments and caller-owned hit snapshots precede reentrant Hold. */
static int __xuiOwnedInvoke_xuiWorkflowWidgetSetContextMenu(xui_widget widget,xui_workflow_widget_data_t* data,xui_flow_context_proc selected,void* user,const xui_flow_hit_t* pHit,float fX,float fY)
{
    if(!widget->pEventOwners || !widget->pEventOwners->pBindings)return selected(widget,pHit,fX,fY,user);
    xui_callback_snapshot_t snapshot={0};
    int status=xuiInternalCallbackSnapshotBeginReported(widget,&data->onContext,&user,&snapshot);
    if(status!=XUI_OK)return status;
    int value=selected(widget,pHit,fX,fY,user);
    status=value<0 ? value:XUI_OK;
    xerror* first=status!=XUI_OK ? xrtTakeError():NULL;
    int ended=xuiInternalCallbackSnapshotEndOutcome(&snapshot,status,first);
    return ended!=XUI_OK ? ended:value;
}


static xui_workflow_widget_data_t* __xuiWorkflowWidgetGetData(xui_widget pWidget);

static int __xuiWorkflowWidgetModelReady(xui_workflow workflow)
{
    if(!workflow)return XUI_OK;
    if(xuiWorkflowIsOwned(workflow)){
        xrtownershipref ref=xuiWorkflowOwnership(workflow);
        const xrtownershipquiescencev1* quiet=xuiWorkflowOwnershipQuiescenceV1(ref);
        return !quiet || quiet->Busy(ref.Data) ? XUI_ERROR_BUSY:XUI_OK;
    }
    return xuiWorkflowGetNodeCount(workflow)==XUI_ERROR_BUSY ? XUI_ERROR_BUSY:XUI_OK;
}

static int __xuiWorkflowWidgetDestroyReady(xui_widget widget,const void* raw)
{
    (void)widget;
    const xui_workflow_widget_data_t* data=raw;
    if(!data)return XUI_OK;
    if(data->bWorkflowMutationActive)return XUI_ERROR_BUSY;
    int status=__xuiWorkflowWidgetModelReady(data->pWorkflow);
    return status!=XUI_OK ? status:__xuiWorkflowWidgetModelReady(data->pRetiredWorkflow);
}

/* A slot is cleared only after its actual reference was consumed. This cold
 * slot allows a committed replacement to retain failed cleanup safely. */
static int __xuiWorkflowWidgetReleaseSlot(xui_workflow* slot)
{
    if(!*slot)return XUI_OK;
    bool consumed=false;int status;
    if(xuiWorkflowIsOwned(*slot))status=xuiWorkflowReleaseOwnerChecked(*slot,&consumed);
    else{status=xuiWorkflowDestroyChecked(*slot);consumed=status==XUI_OK;}
    if(consumed)*slot=NULL;
    if(!consumed && status==XUI_OK)status=XUI_ERROR_INVALID_STATE;
    if(!consumed && status==XUI_STATUS_DEFERRED)status=XUI_ERROR_BUSY;
    return status;
}

static int __xuiWorkflowWidgetDestroyPrepare(xui_widget widget,void* raw,bool* complete)
{
    xui_workflow_widget_data_t* data=raw;
    *complete=false;
    if(!data){*complete=true;return XUI_OK;}
    data->bWorkflowMutationActive=1;
    xui_cleanup_receipt_t outcome={0};
    bool canvasReady=false;
    int status=__xuiWorkflowWidgetReleaseSlot(&data->pRetiredWorkflow);
    xuiInternalCleanupAdd(&outcome,status);
    if(!data->pRetiredWorkflow){
        status=xuiInternalFlowGraphWidgetPrepareModelRelease(data->pCanvas,widget);
        xuiInternalCleanupAdd(&outcome,status);
        canvasReady=status==XUI_OK;
        if(status==XUI_OK && data->bOwnWorkflow){
            xuiInternalContextRecordOutcome(widget->pContext,outcome.status,outcome.error);
            status=__xuiWorkflowWidgetReleaseSlot(&data->pWorkflow);
            xuiInternalCleanupAdd(&outcome,status);
        }
    }
    data->bWorkflowMutationActive=0;
    /* The part no longer borrows its Graph before checked release. A refused
     * release retains the actual owner slot and prevents physical destruction. */
    *complete=canvasReady && !data->pRetiredWorkflow && (!data->bOwnWorkflow || !data->pWorkflow);
    xrtSetErrorTake(outcome.error);return outcome.status;
}

static int __xuiWorkflowWidgetContext(xui_widget pCanvas, const xui_flow_hit_t* pHit,
	float fX, float fY, void* pUser)
{
	xui_widget pWidget = (xui_widget)pUser;
	xui_workflow_widget_data_t* pData = __xuiWorkflowWidgetGetData(pWidget);
	(void)pCanvas;
	if ( pData == NULL || pData->onContext == NULL ) return XUI_OK;
	return __xuiOwnedInvoke_xuiWorkflowWidgetSetContextMenu(pWidget,pData,pData->onContext,pData->pContextUser ,pHit,fX,fY);
}

static int __xuiWorkflowWidgetDescValid(const xui_workflow_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( pDesc->iSize != 0u && pDesc->iSize < sizeof(*pDesc) ) {
		return 0;
	}
	return (pDesc->tGraph.iSize == 0u || pDesc->tGraph.iSize >= sizeof(pDesc->tGraph));
}

static xui_workflow_widget_data_t* __xuiWorkflowWidgetGetData(xui_widget pWidget)
{
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return NULL;
	}
	pType = xuiWorkflowGetType(xuiWidgetGetContext(pWidget));
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_workflow_widget_data_t*)xuiWidgetGetTypeData(pWidget);
}

static void __xuiWorkflowWidgetDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_DOCK;
	pLayout->iWidthMode = XUI_SIZE_FILL;
	pLayout->iHeightMode = XUI_SIZE_FILL;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_CLIP;
	pLayout->iAlignX = XUI_ALIGN_STRETCH;
	pLayout->iAlignY = XUI_ALIGN_STRETCH;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static int __xuiWorkflowWidgetInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_workflow_widget_data_t* pData;
	const xui_workflow_desc_t* pDesc;
	xui_flow_graph_desc_t tGraphDesc;
	int bCreatedWorkflow;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pTypeData == NULL) || !__xuiWorkflowWidgetDescValid((const xui_workflow_desc_t*)pCreateData) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = (xui_workflow_widget_data_t*)pTypeData;
	pDesc = (const xui_workflow_desc_t*)pCreateData;
	if(pDesc && pDesc->pWorkflow && xuiWorkflowGetNodeCount(pDesc->pWorkflow)==XUI_ERROR_BUSY)return XUI_ERROR_BUSY;
	memset(pData, 0, sizeof(*pData));
	bCreatedWorkflow = 0;
	if ( pDesc != NULL ) {
		pData->pWorkflow = pDesc->pWorkflow;
		pData->bOwnWorkflow = pDesc->bOwnWorkflow ? 1 : 0;
	}
	if ( pData->pWorkflow == NULL ) {
		iRet = xuiWorkflowCreate(&pData->pWorkflow);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
		pData->bOwnWorkflow = 1;
		bCreatedWorkflow = 1;
	}
	memset(&tGraphDesc, 0, sizeof(tGraphDesc));
	tGraphDesc.iSize = sizeof(tGraphDesc);
	if ( pDesc != NULL && pDesc->tGraph.iSize >= sizeof(pDesc->tGraph) ) {
		tGraphDesc = pDesc->tGraph;
		tGraphDesc.iSize = sizeof(tGraphDesc);
	}
	tGraphDesc.pGraph = xuiWorkflowGetGraph(pData->pWorkflow);
	tGraphDesc.bOwnGraph = 0;
	iRet = xuiInternalWidgetCreatePart(pWidget,xuiFlowGraphGetType(xuiWidgetGetContext(pWidget)),&pData->pCanvas,&tGraphDesc);
	if ( iRet != XUI_OK ) {
		if ( bCreatedWorkflow && pData->pWorkflow != NULL ) {
			xuiWorkflowDestroy(pData->pWorkflow);
		}
		memset(pData, 0, sizeof(*pData));
		return iRet;
	}
	iRet = xuiFlowGraphWidgetSetContextMenu(pData->pCanvas, __xuiWorkflowWidgetContext, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pData->pCanvas, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetDock(pData->pCanvas, XUI_DOCK_FILL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAlign(pData->pCanvas, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	if ( iRet == XUI_OK ) iRet = xuiInternalWidgetInsertPart(pWidget,pWidget,pData->pCanvas,NULL);
	if ( iRet != XUI_OK ) {
		(void)xuiInternalWidgetDestroyPart(pWidget,pData->pCanvas);
		pData->pCanvas = NULL;
		if ( bCreatedWorkflow && pData->pWorkflow != NULL ) {
			xuiWorkflowDestroy(pData->pWorkflow);
		}
		memset(pData, 0, sizeof(*pData));
		return iRet;
	}
	return XUI_OK;
}

static void __xuiWorkflowWidgetDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_workflow_widget_data_t* pData;

	(void)pWidget;
	(void)pUser;
	pData = (xui_workflow_widget_data_t*)pTypeData;
	if ( pData != NULL ) {
		if ( pData->bOwnWorkflow && pData->pWorkflow != NULL ) {
			int status=__xuiWorkflowWidgetReleaseSlot(&pData->pWorkflow);
            xuiInternalContextRecordOutcome(pWidget->pContext,status,xrtGetError());
		}
		memset(pData, 0, sizeof(*pData));
	}
}

static bool __xuiWorkflowWidgetTraceSlot(xui_workflow workflow,xrtownershipvisitor visitor,void* user)
{
    if(!workflow)return true;
    if(xuiWorkflowIsOwned(workflow)){
        xrtownershipref ref=xuiWorkflowOwnership(workflow);
        return ref.Data && visitor(ref,user);
    }
    return xuiWorkflowTraceOwnedValues(workflow,visitor,user);
}
static bool __xuiWorkflowWidgetTraceOwnership(xui_widget pWidget, const void* pTypeData,
	xrtownershipvisitor onVisit, void* pUser)
{
	(void)pWidget;
	const xui_workflow_widget_data_t* pData = pTypeData;
	/* The canvas borrows the Graph; each actual Workflow slot is one edge. */
	return !pData ||
        ((!pData->bOwnWorkflow || __xuiWorkflowWidgetTraceSlot(pData->pWorkflow,onVisit,pUser)) &&
         __xuiWorkflowWidgetTraceSlot(pData->pRetiredWorkflow,onVisit,pUser));
}

XUI_API xui_widget_type xuiWorkflowGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "workflow");
	if ( pType != NULL ) {
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "workflow";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT;
	tDesc.iTypeDataSize = sizeof(xui_workflow_widget_data_t);
	tDesc.onInit = __xuiWorkflowWidgetInit;
	tDesc.onDestroy = __xuiWorkflowWidgetDestroy;
	__xuiWorkflowWidgetDefaultLayout(&tDesc.tLayout);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet == XUI_OK ) {
		pType->onTraceOwnership = __xuiWorkflowWidgetTraceOwnership;
		pType->onDestroyReady = __xuiWorkflowWidgetDestroyReady;
        pType->onDestroyPrepare = __xuiWorkflowWidgetDestroyPrepare;
	}
	return (iRet == XUI_OK) ? pType : NULL;
}

XUI_API int xuiWorkflowWidgetCreate(xui_context pContext, xui_widget* ppWidget, const xui_workflow_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( (ppWidget == NULL) || !__xuiWorkflowWidgetDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if(pDesc && pDesc->pWorkflow && xuiWorkflowGetNodeCount(pDesc->pWorkflow)==XUI_ERROR_BUSY)return XUI_ERROR_BUSY;
	*ppWidget = NULL;
	pType = xuiWorkflowGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API xui_workflow xuiWorkflowWidgetGetWorkflow(xui_widget pWidget)
{
	xui_workflow_widget_data_t* pData;

	pData = __xuiWorkflowWidgetGetData(pWidget);
	if(pData && __xuiWorkflowWidgetDestroyReady(pWidget,pData)!=XUI_OK){
		if(!xrtGetError())xrtSetErrorInfo(XERR_STATE,"xui.flow_graph",XUI_ERROR_BUSY,"Workflow widget model is busy");
		return NULL;
	}
	return (pData != NULL) ? pData->pWorkflow : NULL;
}

static int __xuiWorkflowWidgetSetWorkflow(xui_widget pWidget,xui_workflow pWorkflow,int bOwnWorkflow,bool* installed)
{
	xui_workflow_widget_data_t* pData;
	int iRet;
    xui_cleanup_receipt_t outcome={0};
    if(installed)*installed=false;

	pData = __xuiWorkflowWidgetGetData(pWidget);
	if ( (pData == NULL) || (pWorkflow == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if(__xuiWorkflowWidgetDestroyReady(pWidget,pData)!=XUI_OK ||
       __xuiWorkflowWidgetModelReady(pWorkflow)!=XUI_OK)return XUI_ERROR_BUSY;
    if(xuiWorkflowIsOwned(pWorkflow) && !xuiWorkflowOwnershipReadReady(xuiWorkflowOwnership(pWorkflow)))return XUI_ERROR_INVALID_STATE;
	xui_flow_graph graph=xuiWorkflowGetGraph(pWorkflow);
	if(!graph)return XUI_ERROR_INVALID_ARGUMENT;
	xui_call_scope_t scope={0};iRet=xuiCallScopeBegin(pWidget->pContext,&scope);
	if(iRet!=XUI_OK)return iRet;
	pData->bWorkflowMutationActive=1;
    iRet=__xuiWorkflowWidgetReleaseSlot(&pData->pRetiredWorkflow);
    xuiInternalContextCleanupOutcome(pWidget->pContext,iRet,&outcome.status,&outcome.error);
    if(outcome.status!=XUI_OK)goto finish;
    bool graphPublished=false;
	iRet=xuiInternalFlowGraphWidgetSetGraphPublished(pData->pCanvas,graph,0,pWidget,&graphPublished);
    xuiInternalCleanupAdd(&outcome,iRet);
    if(!graphPublished)goto finish;
    /* The actual Graph slot is now authoritative, even if a later native
     * invalidate reported an error or requested logical widget retirement. */
    if(pData->bOwnWorkflow && pData->pWorkflow && pData->pWorkflow!=pWorkflow)
        pData->pRetiredWorkflow=pData->pWorkflow;
    if(installed && bOwnWorkflow && (pData->pWorkflow!=pWorkflow || !pData->bOwnWorkflow))*installed=true;
	pData->pWorkflow = pWorkflow;
	pData->bOwnWorkflow = bOwnWorkflow ? 1 : 0;
    xuiInternalCallScopeRecordOutcome(&scope,outcome.status,outcome.error);
    iRet=__xuiWorkflowWidgetReleaseSlot(&pData->pRetiredWorkflow);
    xuiInternalContextCleanupOutcome(pWidget->pContext,iRet,&outcome.status,&outcome.error);
    iRet=xuiInternalWidgetIsValid(pWidget) ?
        xuiWidgetInvalidate(pWidget,XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER):XUI_ERROR_INVALID_STATE;
    xuiInternalCleanupAdd(&outcome,iRet);
finish:
	pData->bWorkflowMutationActive=0;
    xerror* first=outcome.error;
	iRet=xuiInternalCallScopeEndOutcome(&scope,outcome.status,&first);
	xrtSetErrorTake(first);return iRet;
}

XUI_API int xuiWorkflowWidgetSetWorkflow(xui_widget widget,xui_workflow workflow,int ownWorkflow)
{return __xuiWorkflowWidgetSetWorkflow(widget,workflow,ownWorkflow,NULL);}

XUI_API int xuiWorkflowWidgetGetWorkflowOwned(xui_widget widget,xui_workflow* output)
{
    if(!output)return XUI_ERROR_INVALID_ARGUMENT;
    xui_workflow_widget_data_t* data=__xuiWorkflowWidgetGetData(widget);
    if(!data || !data->pWorkflow)return XUI_ERROR_INVALID_ARGUMENT;
    if(__xuiWorkflowWidgetDestroyReady(widget,data)!=XUI_OK)return XUI_ERROR_BUSY;
    if(!data->bOwnWorkflow && !xuiWorkflowIsOwned(data->pWorkflow))return XUI_ERROR_INVALID_STATE;
    xui_call_scope_t scope={0};int status=xuiCallScopeBegin(widget->pContext,&scope);
    if(status!=XUI_OK)return status;
    data->bWorkflowMutationActive=1;
    xui_workflow held=NULL;
    status=xuiInternalWorkflowPromoteOwned(data->pWorkflow);
    if(status==XUI_OK)status=xuiWorkflowShareOwned(data->pWorkflow,&held);
    data->bWorkflowMutationActive=0;
    xerror* first=status!=XUI_OK ? xrtTakeError():NULL;
    status=xuiInternalCallScopeEndOutcome(&scope,status,&first);
    if(status==XUI_OK)*output=held;
    else if(held){(void)xuiWorkflowReleaseChecked(held);xrtClearError();}
    xrtSetErrorTake(first);return status;
}

XUI_API int xuiWorkflowWidgetSetWorkflowShared(xui_widget widget,xui_workflow workflow)
{
    xui_workflow_widget_data_t* data=__xuiWorkflowWidgetGetData(widget);
    if(!data || !workflow || !xuiWorkflowIsOwned(workflow))return XUI_ERROR_INVALID_ARGUMENT;
    if(__xuiWorkflowWidgetDestroyReady(widget,data)!=XUI_OK ||
       __xuiWorkflowWidgetModelReady(workflow)!=XUI_OK)return XUI_ERROR_BUSY;
    if(!xuiWorkflowOwnershipReadReady(xuiWorkflowOwnership(workflow)))return XUI_ERROR_INVALID_STATE;
    if(data->pWorkflow==workflow && data->bOwnWorkflow && !data->pRetiredWorkflow)return XUI_OK;
    xui_call_scope_t scope={0};int status=xuiCallScopeBegin(widget->pContext,&scope);
    if(status!=XUI_OK)return status;
    xui_workflow held=NULL;bool installed=false;
    if(data->bOwnWorkflow && data->pWorkflow && !xuiWorkflowIsOwned(data->pWorkflow)){
        status=xuiInternalWorkflowPromoteOwned(data->pWorkflow);
        if(status!=XUI_OK)goto finish;
    }
    status=xuiWorkflowShareOwned(workflow,&held);
    if(status!=XUI_OK)goto finish;
    status=__xuiWorkflowWidgetSetWorkflow(widget,held,1,&installed);
finish:
    if(!installed && held){
        xerror* first=xrtTakeError();
        int dropped=xuiWorkflowReleaseChecked(held);xerror* secondary=xrtTakeError();
        if(status==XUI_OK && (dropped!=XUI_OK || secondary)){
            status=dropped!=XUI_OK ? dropped:XUI_ERROR_CALLBACK_FAILED;
            xrtErrorFree(first);first=secondary;secondary=NULL;
        }
        xrtErrorFree(secondary);xrtSetErrorTake(first);
    }
    xerror* first=status!=XUI_OK ? xrtTakeError():NULL;
    status=xuiInternalCallScopeEndOutcome(&scope,status,&first);
    xrtSetErrorTake(first);return status;
}

XUI_API int xuiWorkflowWidgetSetContextMenuOwned(xui_widget pWidget, xui_flow_context_proc onContext, void* pUser, const xui_callback_owner_t* pOwner)
{
	xui_workflow_widget_data_t* pData = __xuiWorkflowWidgetGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	xui_callback_update_t ownedUpdate={0};
    int ownedStatus=xuiInternalCallbackUpdateBegin(pWidget,&pData->onContext,onContext!=NULL,pUser,pOwner,&ownedUpdate);
    if(ownedStatus!=XUI_OK)return ownedStatus;
    pData->onContext = onContext;
	pData->pContextUser = pUser;
    xuiInternalCallbackUpdatePublish(&ownedUpdate);
	return xuiInternalCallbackUpdateEnd(&ownedUpdate,XUI_OK);
}

XUI_API int xuiWorkflowWidgetSetContextMenu(xui_widget pWidget, xui_flow_context_proc onContext, void* pUser)
{
	xui_workflow_widget_data_t* pData = __xuiWorkflowWidgetGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if(xuiInternalCallbackSlotOwned(pWidget,&pData->onContext))return xuiWorkflowWidgetSetContextMenuOwned(pWidget,onContext,pUser,NULL);
    pData->onContext = onContext;
	pData->pContextUser = pUser;
	return XUI_OK;
}

XUI_API xui_widget xuiWorkflowWidgetGetCanvas(xui_widget pWidget)
{
	xui_workflow_widget_data_t* pData;

	pData = __xuiWorkflowWidgetGetData(pWidget);
	return (pData != NULL) ? pData->pCanvas : NULL;
}

#endif
