#ifndef XGE_CALLBACK_OWNED_INTERNAL_H
#define XGE_CALLBACK_OWNED_INTERNAL_H
/* Global native publishers are external roots until explicitly cleared.
 * Readers pin the registration node while holding the publisher spin lock,
 * then invoke arbitrary owner code only after unlocking. A separate owner
 * snapshot covers the callback's physical return tail. Raw slots allocate
 * no node and retain their existing caller-managed userdata contract. */
typedef struct xge_callback_node_t {
    xatomic64 tReaders;
    xge_callback_owner_t tOwner;
} xge_callback_node_t;
typedef struct xge_callback_snapshot_t {
    xge_callback_owner_t tOwner;
    bool bHeld;
    bool bActive;
} xge_callback_snapshot_t;
static xatomic32 g_xgeCallbackShutdown;
static xatomic64 g_xgeCallbackActivity;
static bool __xgeCallbackActivityEnter(bool allowShutdown)
{
    if(!allowShutdown && xrtAtomic32Load(&g_xgeCallbackShutdown,XMEMORY_ACQUIRE))return false;
    uint64 expected=xrtAtomic64Load(&g_xgeCallbackActivity,XMEMORY_RELAXED);
    do{if(expected==UINT64_MAX)return false;}
    while(!xrtAtomic64CompareExchange(&g_xgeCallbackActivity,&expected,expected+1,XMEMORY_ACQ_REL,XMEMORY_RELAXED));
    if(!allowShutdown && xrtAtomic32Load(&g_xgeCallbackShutdown,XMEMORY_ACQUIRE)){
        (void)xrtAtomic64FetchSub(&g_xgeCallbackActivity,1,XMEMORY_RELEASE);return false;
    }
    return true;
}
static void __xgeCallbackActivityLeave(void)
{(void)xrtAtomic64FetchSub(&g_xgeCallbackActivity,1,XMEMORY_RELEASE);}
static void __xgeCallbackOutcome(int next,int* result,xerror** first)
{
    xerror* diagnostic=xrtTakeError();
    if(next==XGE_OK && !diagnostic)return;
    if(*result==XGE_OK){*result=next!=XGE_OK ? next:XGE_ERROR_INVALID_STATE;*first=diagnostic;}
    else xrtErrorFree(diagnostic);
}
static int __xgeCallbackOwnerValid(const xge_callback_owner_t* owner)
{
    return !owner || ((!owner->tReference.Data && !owner->tReference.Ops && !owner->onHold && !owner->onDrop) ||
        (owner->tReference.Data && owner->tReference.Ops && owner->onHold && owner->onDrop));
}
static int __xgeCallbackOwnerDrop(xge_callback_owner_t owner)
{return !owner.onDrop || owner.onDrop(owner.tReference.Data) ? XGE_OK:XGE_ERROR_INVALID_STATE;}
static int __xgeCallbackNodePrepare(bool hasCallback,bool allowShutdown,const xge_callback_owner_t* owner,xge_callback_node_t** output)
{
    *output=NULL;
    if(!__xgeCallbackOwnerValid(owner) || (!hasCallback && owner && owner->onHold))return XGE_ERROR_INVALID_ARGUMENT;
    if(!owner || !owner->onHold)return XGE_OK;
    if(!__xgeCallbackActivityEnter(allowShutdown))return XGE_ERROR_INVALID_STATE;
    xge_callback_owner_t incoming=*owner;
    if(!incoming.onHold(incoming.tReference.Data)){__xgeCallbackActivityLeave();return XGE_ERROR_INVALID_STATE;}
    xge_callback_node_t* node=xrtCalloc(1,sizeof(*node));
    if(!node){
        xerror* primary=xrtTakeError();
        (void)__xgeCallbackOwnerDrop(incoming);xrtClearError();xrtSetErrorTake(primary);
        __xgeCallbackActivityLeave();
        return XGE_ERROR_OUT_OF_MEMORY;
    }
    xrtAtomic64Init(&node->tReaders,1);node->tOwner=incoming;*output=node;
    __xgeCallbackActivityLeave();return XGE_OK;
}
/* Publisher lock held; does not invoke an owner or allocate. */
static bool __xgeCallbackNodePin(xge_callback_node_t* node)
{
    if(!node)return true;
    uint64 expected=xrtAtomic64Load(&node->tReaders,XMEMORY_RELAXED);
    do{if(!expected || expected==UINT64_MAX)return false;}
    while(!xrtAtomic64CompareExchange(&node->tReaders,&expected,expected+1,XMEMORY_RELAXED,XMEMORY_RELAXED));
    return true;
}
/* Publisher lock MUST NOT be held. Last reader releases the registration. */
static int __xgeCallbackNodeRelease(xge_callback_node_t* node)
{
    if(!node)return XGE_OK;
    if(xrtAtomic64FetchSub(&node->tReaders,1,XMEMORY_ACQ_REL)!=1)return XGE_OK;
    xge_callback_owner_t owner=node->tOwner;xrtFree(node);return __xgeCallbackOwnerDrop(owner);
}
/* Consumes one pinned node on both outcomes. Owner Hold can reenter and clear
 * or replace its global slot; the pinned node still protects copied metadata. */
static int __xgeCallbackSnapshotBegin(xge_callback_node_t* node,bool allowShutdown,xge_callback_snapshot_t* snapshot)
{
    if(!node)return XGE_OK;
    if(!__xgeCallbackActivityEnter(allowShutdown)){
        (void)__xgeCallbackNodeRelease(node);return XGE_ERROR_INVALID_STATE;
    }
    xge_callback_owner_t owner=node->tOwner;
    bool held=owner.onHold(owner.tReference.Data);
    xerror* primary=held ? NULL:xrtTakeError();
    int released=__xgeCallbackNodeRelease(node);
    if(!held){xrtClearError();xrtSetErrorTake(primary);__xgeCallbackActivityLeave();return XGE_ERROR_INVALID_STATE;}
    if(released!=XGE_OK){
        primary=xrtTakeError();(void)__xgeCallbackOwnerDrop(owner);
        xrtClearError();xrtSetErrorTake(primary);__xgeCallbackActivityLeave();return released;
    }
    snapshot->tOwner=owner;snapshot->bHeld=true;snapshot->bActive=true;return XGE_OK;
}
static int __xgeCallbackSnapshotEnd(xge_callback_snapshot_t* snapshot)
{
    if(!snapshot->bHeld)return XGE_OK;
    xge_callback_owner_t owner=snapshot->tOwner;
    snapshot->bHeld=false;snapshot->tOwner=(xge_callback_owner_t){0};
    int result=__xgeCallbackOwnerDrop(owner);
    if(snapshot->bActive){snapshot->bActive=false;__xgeCallbackActivityLeave();}
    return result;
}
#endif
