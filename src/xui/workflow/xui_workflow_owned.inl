/* Shared Workflow identity is independent of its replaceable tables and of
 * the independently shared graph. No model payload is copied by Share. */
static xge_data_lifetime_t* __xuiWorkflowRoot(xui_workflow workflow)
{return workflow ? __xuiWorkflowOwnerRoot(workflow->pOwned):NULL;}

static bool __xuiWorkflowModelReadable(const void* data)
{return ((const xui_workflow_owned_t*)data)->pWorkflow->pGraph!=NULL;}
static int __xuiWorkflowOwnedExtraReady(const xge_data_lifetime_t* root)
{
    const xui_workflow_owned_t* owner=(const xui_workflow_owned_t*)root-1;
    if(owner->iDepth || __xuiWorkflowModelBusy(owner->pWorkflow))return XUI_ERROR_BUSY;
    xui_flow_graph graph=owner->pWorkflow->pGraph;
    if(graph){
        xrtownershipref ref=xuiFlowGraphOwnership(graph);
        const xrtownershipquiescencev1* quiet=xuiFlowGraphOwnershipQuiescenceV1(ref);
        if(!quiet || quiet->Busy(ref.Data))return XUI_ERROR_BUSY;
    }
    return XUI_OK;
}
static bool __xuiWorkflowModelTrace(const void*,xrtownershipvisitor,void*);
static int __xuiWorkflowModelFinalize(void*,bool*);
static void __xuiWorkflowOwnedFree(xge_data_lifetime_t* root)
{
    xui_workflow_owned_t* owner=(xui_workflow_owned_t*)root-1;
    xrtFree(owner->pWorkflow);
    xrtFree(owner);
}
static const xge_data_lifetime_type_t __xuiWorkflowOwnedType={
    sizeof(xui_workflow_owned_t),offsetof(xui_workflow_owned_t,iRefs),
    __xuiWorkflowModelReadable,__xuiWorkflowModelTrace,__xuiWorkflowModelFinalize,NULL};

#define DO_FN(name) __xuiWorkflowOwned##name
#define DO_PUBLIC(name) xuiWorkflow##name
#define DO_STATUS(name) XUI_##name
#define DO_API XUI_API
#define DO_ROOT(object) __xuiWorkflowRoot(object)
#define DO_HANDLE xui_workflow
#define DO_OBJECT_TYPE __xuiWorkflowOwnedType
#define DO_GENERATION_TYPE __xuiWorkflowOwnedType
#define DO_EXTRA_READY(root) __xuiWorkflowOwnedExtraReady(root)
#define DO_READ_READY(root) (__xuiWorkflowOwnedAdmit(root)==XUI_OK && __xuiWorkflowOwnedReady(root)==XUI_OK)
#define DO_FREE_BODY(root) __xuiWorkflowOwnedFree(root)
#include "xge_data_lifetime.inl"
#undef DO_FN
#undef DO_PUBLIC
#undef DO_STATUS
#undef DO_API
#undef DO_ROOT
#undef DO_HANDLE
#undef DO_OBJECT_TYPE
#undef DO_GENERATION_TYPE
#undef DO_EXTRA_READY
#undef DO_READ_READY
#undef DO_FREE_BODY

static bool __xuiWorkflowModelTrace(const void* data,xrtownershipvisitor visitor,void* user)
{
    xui_workflow_owned_t* owner=(xui_workflow_owned_t*)data;
    return __xuiWorkflowOwnedReady(__xuiWorkflowOwnerRoot(owner))==XUI_OK &&
        __xuiWorkflowTraceValues(owner->pWorkflow,visitor,user);
}
static int __xuiWorkflowModelFinalize(void* data,bool* complete)
{
    xui_workflow workflow=((xui_workflow_owned_t*)data)->pWorkflow;
    xge_data_outcome_t outcome={0};
    __xuiWorkflowOwnedOutcomeBegin(&outcome,XUI_OK);
    /* If the child cannot consume its reference, leave the tables intact so
     * the checked owner release can be retried without partial destruction. */
    bool consumed=true;
    if(workflow->pGraph){
        __xuiWorkflowOwnedOutcomeAdd(&outcome,xuiFlowGraphReleaseOwnerChecked(workflow->pGraph,&consumed));
        if(!consumed){*complete=false;return __xuiWorkflowOwnedOutcomeEnd(&outcome,NULL);}
        workflow->pGraph=NULL;
    }
    /* Separate each user Drop boundary so a later error cannot overwrite the
     * first. The lifetime state rejects calls while these tables are cleared. */
    for(size_t i=0;i<workflow->arrTypes.Count;++i){
        __xuiWorkflowFreeType(xrtArrayGet(&workflow->arrTypes,i));
        __xuiWorkflowOwnedOutcomeAdd(&outcome,XUI_OK);
    }
    for(size_t i=0;i<workflow->arrVariables.Count;++i){
        __xuiWorkflowFreeVariable(xrtArrayGet(&workflow->arrVariables,i));
        __xuiWorkflowOwnedOutcomeAdd(&outcome,XUI_OK);
    }
    xrtArrayUnit(&workflow->arrTypes);xrtArrayUnit(&workflow->arrVariables);
    xrtMapUnit(&workflow->mapTypes);xrtMapUnit(&workflow->mapVariables);
    *complete=true;
    return __xuiWorkflowOwnedOutcomeEnd(&outcome,NULL);
}
static bool __xuiWorkflowLegacyReadable(xui_workflow workflow)
{
    if(!workflow)return false;
    if(!workflow->pOwned)return true;
    int status=__xuiWorkflowOwnedAdmit(__xuiWorkflowRoot(workflow));
    if(status!=XUI_OK && !xrtGetError())xrtSetErrorInfo(XERR_STATE,"xui.workflow",status,"Workflow is not readable");
    return status==XUI_OK;
}

/* Promotion is reserved for a native owner of a unique slot. The graph is
 * promoted transactionally first; failure leaves that slot owning its model. */
int xuiInternalWorkflowPromoteOwned(xui_workflow workflow)
{
    if(__xuiWorkflowModelBusy(workflow))return XUI_ERROR_BUSY;
    if(!workflow)return XUI_ERROR_INVALID_ARGUMENT;
    if(workflow->pOwned)return __xuiWorkflowOwnedAdmit(__xuiWorkflowRoot(workflow));
    xui_workflow_owned_t* owner=xrtCalloc(1,sizeof(*owner)+sizeof(xge_data_lifetime_t));
    if(!owner)return XUI_ERROR_OUT_OF_MEMORY;
    int status=xuiInternalFlowGraphPromoteOwned(workflow->pGraph);
    if(status!=XUI_OK){xrtFree(owner);return status;}
    owner->pWorkflow=workflow;atomic_init(&owner->iRefs,1u);
    __xuiWorkflowOwnedInit(__xuiWorkflowOwnerRoot(owner),&__xuiWorkflowOwnedType,xrtThreadCurrentId());
    xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation)){xrtFree(owner);return XUI_ERROR_INVALID_STATE;}
    workflow->pOwned=owner;
    __xuiWorkflowOwnedMutationEnd(&mutation);
    return XUI_OK;
}
XUI_API bool xuiWorkflowIsOwned(xui_workflow workflow)
{return workflow && __xuiWorkflowOwnedKnown(__xuiWorkflowRoot(workflow));}
XUI_API int xuiWorkflowCreateOwned(xui_workflow* output)
{
    if(!output)return XUI_ERROR_INVALID_ARGUMENT;
    *output=NULL;xui_workflow workflow=NULL;
    int status=xuiWorkflowCreate(&workflow);
    if(status!=XUI_OK)return status;
    status=xuiInternalWorkflowPromoteOwned(workflow);
    if(status!=XUI_OK){xuiWorkflowDestroy(workflow);return status;}
    *output=workflow;return XUI_OK;
}
XUI_API int xuiWorkflowShareOwned(xui_workflow source,xui_workflow* output)
{
    if(__xuiWorkflowModelBusy(source))return XUI_ERROR_BUSY;
    if(!output || !xuiWorkflowIsOwned(source))return XUI_ERROR_INVALID_ARGUMENT;
    int status=__xuiWorkflowOwnedAcquire(__xuiWorkflowRoot(source),true,false);
    if(status==XUI_OK)*output=source;
    return status;
}
XUI_API int xuiWorkflowGetGraphOwned(xui_workflow workflow,xui_flow_graph* output)
{
    if(__xuiWorkflowModelBusy(workflow))return XUI_ERROR_BUSY;
    if(!output || !xuiWorkflowIsOwned(workflow))return XUI_ERROR_INVALID_ARGUMENT;
    xui_workflow_operation_t operation={0};
    int status=__xuiWorkflowOperationBegin(workflow,&operation);
    if(status!=XUI_OK)return status;
    xui_flow_graph graph=NULL;
    status=xuiFlowGraphShareOwned(workflow->pGraph,&graph);
    status=__xuiWorkflowOperationEnd(&operation,status,0);
    if(status==XUI_OK)*output=graph;
    else if(graph){
        xerror* first=xrtTakeError();
        (void)xuiFlowGraphReleaseChecked(graph);xrtClearError();xrtSetErrorTake(first);
    }
    return status;
}
XUI_API int xuiWorkflowReleaseOwnerChecked(xui_workflow workflow,bool* consumed)
{
    if(!consumed)return XUI_ERROR_INVALID_ARGUMENT;
    *consumed=false;
    if(!xuiWorkflowIsOwned(workflow))return XUI_ERROR_INVALID_ARGUMENT;
    xge_data_lifetime_t* root=__xuiWorkflowRoot(workflow);
    unsigned state=__xuiWorkflowOwnedState(root);
    if(!(state&XGE_DATA_FINISHED) && root->iCreator!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(!(state&XGE_DATA_FINISHED) && __xuiWorkflowOwnedExtraReady(root)!=XUI_OK)return XUI_ERROR_BUSY;
    return __xuiWorkflowOwnedRelease(root,consumed);
}
XUI_API int xuiWorkflowReleaseChecked(xui_workflow workflow)
{bool consumed=false;return xuiWorkflowReleaseOwnerChecked(workflow,&consumed);}

static int __xuiWorkflowOwnedOperationBegin(xui_workflow_owned_t* owner,xui_workflow_operation_t* operation)
{
    operation->pAmbient=NULL;operation->bOuter=false;
    xge_data_lifetime_t* root=__xuiWorkflowOwnerRoot(owner);
    int status=__xuiWorkflowOwnedAdmit(root);
    if(status!=XUI_OK)return status;
    if(owner->iDepth){
        if(owner->iDepth==UINT_MAX)return XUI_ERROR_INVALID_STATE;
        ++owner->iDepth;return XUI_OK;
    }
    operation->pAmbient=xrtTakeError();
    status=__xuiWorkflowOwnedAcquire(root,true,true);
    if(status!=XUI_OK){xrtErrorFree(operation->pAmbient);operation->pAmbient=NULL;return status;}
    owner->iDepth=1u;operation->bOuter=true;return XUI_OK;
}
static int __xuiWorkflowOwnedOperationEnd(xui_workflow_operation_t* operation,int value,int minimum)
{
    xui_workflow_owned_t* owner=operation->pOwner;assert(owner->iDepth);
    --owner->iDepth;operation->pOwner=NULL;
    if(!operation->bOuter)return value;
    assert(!owner->iDepth);
    xge_data_outcome_t result={0};
    __xuiWorkflowOwnedOutcomeBegin(&result,value<minimum ? value:XUI_OK);
    __xuiWorkflowOwnedOutcomeAdd(&result,__xuiWorkflowOwnedActivityEnd(__xuiWorkflowOwnerRoot(owner)));
    int status=__xuiWorkflowOwnedOutcomeEnd(&result,operation->pAmbient);
    return status==XUI_OK ? value:status;
}
