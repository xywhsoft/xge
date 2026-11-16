/* Shared model origin. Included after the concrete graph operations so the
 * native owning node traces exactly its actual configuration/history refs. */
static xge_data_lifetime_t* __xuiFlowGraphRoot(xui_flow_graph graph)
{return graph ? __xuiFlowOwnerRoot(graph->pOwned):NULL;}

static bool __xuiFlowModelReadable(const void* data)
{return ((const xui_flow_owned_t*)data)->pGraph!=NULL;}
static int __xuiFlowOwnedExtraReady(const xge_data_lifetime_t* root)
{
    const xui_flow_owned_t* owner=(const xui_flow_owned_t*)root-1;
    return owner->iDepth || __xuiFlowBusy(owner->pGraph) ? XUI_ERROR_BUSY:XUI_OK;
}
static bool __xuiFlowModelTrace(const void*,xrtownershipvisitor,void*);
static int __xuiFlowModelFinalize(void* data,bool* complete)
{
    xui_flow_owned_t* owner=data;
    __xuiFlowDestroyContents(owner->pGraph);
    *complete=true;
    return XUI_OK;
}
static void __xuiFlowOwnedFree(xge_data_lifetime_t* root)
{
    xui_flow_owned_t* owner=(xui_flow_owned_t*)root-1;
    /* Finish can precede the last view/GC reference. Keep this graph address
     * stable until the actual last RC is consumed, including after Clear. */
    xrtFree(owner->pGraph);
    xrtFree(owner);
}
static const xge_data_lifetime_type_t __xuiFlowOwnedType={
    sizeof(xui_flow_owned_t),offsetof(xui_flow_owned_t,iRefs),
    __xuiFlowModelReadable,__xuiFlowModelTrace,__xuiFlowModelFinalize,NULL};

#define DO_FN(name) __xuiFlowOwned##name
#define DO_PUBLIC(name) xuiFlowGraph##name
#define DO_STATUS(name) XUI_##name
#define DO_API XUI_API
#define DO_ROOT(object) __xuiFlowGraphRoot(object)
#define DO_HANDLE xui_flow_graph
#define DO_OBJECT_TYPE __xuiFlowOwnedType
#define DO_GENERATION_TYPE __xuiFlowOwnedType
#define DO_EXTRA_READY(root) __xuiFlowOwnedExtraReady(root)
#define DO_READ_READY(root) (__xuiFlowOwnedAdmit(root)==XUI_OK && __xuiFlowOwnedReady(root)==XUI_OK)
#define DO_FREE_BODY(root) __xuiFlowOwnedFree(root)
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

static bool __xuiFlowModelTrace(const void* data,xrtownershipvisitor visitor,void* user)
{
    xui_flow_owned_t* owner=(xui_flow_owned_t*)data;
    return __xuiFlowOwnedReady(__xuiFlowOwnerRoot(owner))==XUI_OK &&
        __xuiFlowTraceValues(owner->pGraph,visitor,user);
}

/* Only an existing native owner may promote its unique slot. Promotion adds
 * the stable RC node, preserving that owner's reference; it never copies data.
 * The canvas getter subsequently acquires its independent output reference. */
int xuiInternalFlowGraphPromoteOwned(xui_flow_graph graph)
{
    if(__xuiFlowBusy(graph))return XUI_ERROR_BUSY;
    if(!graph || graph->pCallbackOwner)return XUI_ERROR_INVALID_ARGUMENT;
    if(graph->pOwned)return __xuiFlowOwnedAdmit(__xuiFlowGraphRoot(graph));
    xui_flow_owned_t* owner=xrtCalloc(1,sizeof(*owner)+sizeof(xge_data_lifetime_t));
    if(!owner)return XUI_ERROR_OUT_OF_MEMORY;
    owner->pGraph=graph;atomic_init(&owner->iRefs,1u);
    __xuiFlowOwnedInit(__xuiFlowOwnerRoot(owner),&__xuiFlowOwnedType,xrtThreadCurrentId());
    xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation)){xrtFree(owner);return XUI_ERROR_INVALID_STATE;}
    graph->pOwned=owner;
    __xuiFlowOwnedMutationEnd(&mutation);
    return XUI_OK;
}

XUI_API bool xuiFlowGraphIsOwned(xui_flow_graph graph)
{return graph && __xuiFlowOwnedKnown(__xuiFlowGraphRoot(graph));}
XUI_API int xuiFlowGraphCreateOwned(xui_flow_graph* output)
{
    if(!output)return XUI_ERROR_INVALID_ARGUMENT;
    *output=NULL;xui_flow_graph graph=NULL;
    int status=xuiFlowGraphCreate(&graph);
    if(status!=XUI_OK)return status;
    status=xuiInternalFlowGraphPromoteOwned(graph);
    if(status!=XUI_OK){xuiFlowGraphDestroy(graph);return status;}
    *output=graph;return XUI_OK;
}
XUI_API int xuiFlowGraphShareOwned(xui_flow_graph source,xui_flow_graph* output)
{
    if(__xuiFlowBusy(source))return XUI_ERROR_BUSY;
    if(!output || !xuiFlowGraphIsOwned(source))return XUI_ERROR_INVALID_ARGUMENT;
    int status=__xuiFlowOwnedAcquire(__xuiFlowGraphRoot(source),true,false);
    if(status==XUI_OK)*output=source;
    return status;
}
XUI_API int xuiFlowGraphReleaseOwnerChecked(xui_flow_graph graph,bool* consumed)
{
    if(!consumed)return XUI_ERROR_INVALID_ARGUMENT;
    *consumed=false;
    if(!xuiFlowGraphIsOwned(graph))return XUI_ERROR_INVALID_ARGUMENT;
    xge_data_lifetime_t* root=__xuiFlowGraphRoot(graph);
    unsigned state=__xuiFlowOwnedState(root);
    if(!(state&XGE_DATA_FINISHED) && root->iCreator!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(!(state&XGE_DATA_FINISHED) && __xuiFlowBusy(graph))return XUI_ERROR_BUSY;
    return __xuiFlowOwnedRelease(root,consumed);
}
XUI_API int xuiFlowGraphReleaseChecked(xui_flow_graph graph)
{bool consumed=false;return xuiFlowGraphReleaseOwnerChecked(graph,&consumed);}

static int __xuiFlowOwnedOperationBegin(xui_flow_owned_t* owner,xui_flow_operation_t* operation)
{
    operation->pAmbient=NULL;operation->bOuter=false;
    xge_data_lifetime_t* root=__xuiFlowOwnerRoot(owner);
    if(root->iCreator!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(owner->iDepth){
        if(owner->iDepth==UINT_MAX)return XUI_ERROR_INVALID_STATE;
        ++owner->iDepth;return XUI_OK;
    }
    operation->pAmbient=xrtTakeError();
    int status=__xuiFlowOwnedAcquire(root,true,true);
    if(status!=XUI_OK){xrtErrorFree(operation->pAmbient);operation->pAmbient=NULL;return status;}
    owner->iDepth=1u;operation->bOuter=true;return XUI_OK;
}
static int __xuiFlowOwnedOperationEnd(xui_flow_operation_t* operation,int value,int minimum)
{
    xui_flow_owned_t* owner=operation->pOwner;assert(owner->iDepth);
    --owner->iDepth;operation->pOwner=NULL;
    if(!operation->bOuter)return value;
    assert(!owner->iDepth);
    xge_data_outcome_t result={0};
    __xuiFlowOwnedOutcomeBegin(&result,value<minimum ? value:XUI_OK);
    __xuiFlowOwnedOutcomeAdd(&result,__xuiFlowOwnedActivityEnd(__xuiFlowOwnerRoot(owner)));
    int status=__xuiFlowOwnedOutcomeEnd(&result,operation->pAmbient);
    return status==XUI_OK ? value:status;
}
