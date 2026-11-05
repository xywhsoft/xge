#include "xui_config.h"
#if XGE_ENABLE_XUI
#include "xui_internal.h"
#include "xui_owned_manager.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* Manager records are uniquely owned by their Context. UI-thread operations
 * use the existing Context scope, while graph-visible publications use the
 * XRT mutation gate. No manager RC, global registry or extra allocation. */
static void mutationEnd(xrtownershipscope* scope)
{if(!xrtOwnershipScopeEnd(scope))abort();}
bool xuiInternalManagerAlive(const xui_owned_manager_t* manager)
{return manager && manager->registered && !manager->pending && manager->context;}
int xuiInternalManagerReadStatus(const xui_owned_manager_t* manager)
{
    if(!manager)return XUI_ERROR_INVALID_ARGUMENT;
    /* creatorThread is immutable. Do not inspect mutable manager/context
     * fields on a foreign thread, even when the eventual result is default. */
    if(manager->creatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(!xuiInternalManagerAlive(manager))return XUI_ERROR_INVALID_STATE;
    xui_context context=manager->context;
    if(context->bCloseRequested || context->bDestroyPending || context->bDestroying)return XUI_ERROR_INVALID_STATE;
    return XUI_OK;
}
int xuiInternalManagerRegister(xui_context context,xui_owned_manager_t* manager,const xui_owned_manager_ops_t* ops,void* data)
{
    if(!manager || !ops || !ops->Trace || !ops->Ready || !ops->Prepare || !ops->Close || !ops->Finalize || !data ||
       !xuiInternalContextIsValid(context))return XUI_ERROR_INVALID_ARGUMENT;
    if(context->bCloseRequested || context->bDestroying || context->bDestroyPending || manager->registered || manager->context ||
       manager->previous || manager->next || manager->pendingNext || manager->watches || manager->active ||
       context->iOperationDepth<=0)return XUI_ERROR_INVALID_STATE;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    manager->context=context;manager->ops=ops;manager->data=data;
    manager->creatorThread=xrtThreadCurrentId();manager->registered=true;
    manager->next=context->pOwnedManagers;
    if(manager->next)manager->next->previous=manager;
    context->pOwnedManagers=manager;mutationEnd(&mutation);return XUI_OK;
}
int xuiInternalManagerScopeBegin(xui_owned_manager_t* manager,xui_manager_scope_t* scope)
{
    if(!manager || !scope)return XUI_ERROR_INVALID_ARGUMENT;
    if(manager->creatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(scope->self || scope->manager || !xuiInternalManagerAlive(manager))return XUI_ERROR_INVALID_STATE;
    if(manager->active==UINT_MAX)return XUI_ERROR_INVALID_STATE;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    int status=xuiCallScopeBegin(manager->context,&scope->contextScope);
    if(status==XUI_OK){++manager->active;scope->self=scope;scope->manager=manager;}
    mutationEnd(&mutation);return status;
}
int xuiInternalManagerScopeEnd(xui_manager_scope_t* scope,int status)
{
    if(!scope || scope->self!=scope || !scope->manager)return XUI_ERROR_INVALID_ARGUMENT;
    xui_owned_manager_t* manager=scope->manager;
    if(manager->creatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    xerror* first=xrtTakeError();if(status==XUI_OK && first)status=XUI_ERROR_CALLBACK_FAILED;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation)){xrtSetErrorTake(first);return XUI_ERROR_INVALID_STATE;}
    if(!manager->active)abort();
    --manager->active;scope->self=NULL;scope->manager=NULL;mutationEnd(&mutation);
    /* Context End may destroy this manager and the Context. Nothing touches
     * either record after the existing canonical native tail returns. */
    status=xuiInternalCallScopeEndOutcome(&scope->contextScope,status,&first);
    xrtSetErrorTake(first);return status;
}
int xuiInternalManagerRequestClose(xui_owned_manager_t* manager)
{
    if(!manager)return XUI_ERROR_INVALID_ARGUMENT;
    if(manager->creatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(!manager->registered || !manager->context)return XUI_ERROR_INVALID_ARGUMENT;
    xui_context context=manager->context;
    if(manager->pending)return XUI_OK;
    if(!context->bDestroying && context->iOperationDepth<=0)return XUI_ERROR_INVALID_STATE;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    manager->pending=true;manager->closing=true;manager->queued=true;
    manager->pendingNext=context->pPendingManagers;context->pPendingManagers=manager;mutationEnd(&mutation);
    xuiInternalLifetimeInvalidateAll(&manager->watches);
    /* This only requests logical Widget close. Their deferred physical tails
     * have priority over manager Finalize in Context.OperationLeave. */
    manager->ops->Close(manager->data);
    if(!xrtOwnershipMutationBegin(&mutation))abort();
    manager->closing=false;mutationEnd(&mutation);return XUI_OK;
}
int xuiInternalManagerWatch(xui_owned_manager_t* manager,xui_lifetime_link_t* link,void(*invalidate)(void*),void* data)
{
    if(!manager || !link || !invalidate)return XUI_ERROR_INVALID_ARGUMENT;
    if(manager->creatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(!xuiInternalManagerAlive(manager))return XUI_ERROR_INVALID_ARGUMENT;
    if(link->ppPrevious || link->pNext || link->onInvalidate || link->pUser)return XUI_ERROR_INVALID_STATE;
    link->pNext=manager->watches;link->ppPrevious=&manager->watches;link->onInvalidate=invalidate;link->pUser=data;
    if(link->pNext)link->pNext->ppPrevious=&link->pNext;
    manager->watches=link;return XUI_OK;
}
static void outcome(int next,int* status,xerror** first)
{
    xerror* error=xrtTakeError();
    if(next==XUI_OK && !error)return;
    if(*status==XUI_OK){*status=next!=XUI_OK ? next:XUI_ERROR_CALLBACK_FAILED;*first=error;}
    else xrtErrorFree(error);
}
int xuiInternalManagerCallbackUpdateBegin(xui_owned_manager_t* manager,xui_callback_owner_t* slot,bool hasCallback,const xui_callback_owner_t* incoming,xui_manager_callback_update_t* update)
{
    if(!slot || !update || update->scope.self || update->published ||
       !xuiInternalCallbackOwnerValid(incoming) || (!hasCallback && incoming && incoming->onHold))return XUI_ERROR_INVALID_ARGUMENT;
    xui_callback_owner_t candidate=incoming ? *incoming:(xui_callback_owner_t){0};
    int status=xuiInternalManagerScopeBegin(manager,&update->scope);if(status!=XUI_OK)return status;
    status=xuiInternalCallbackOwnerHold(&candidate);bool held=status==XUI_OK;
    if(status==XUI_OK && xrtGetError())status=XUI_ERROR_CALLBACK_FAILED;
    if(status==XUI_OK && (!xuiInternalManagerAlive(manager) || manager->context->bCloseRequested || manager->context->bDestroyPending || manager->context->bDestroying))status=XUI_ERROR_INVALID_STATE;
    if(status==XUI_OK && !xrtOwnershipMutationBegin(&update->mutation))status=XUI_ERROR_INVALID_STATE;
    if(status==XUI_OK){update->previous=*slot;*slot=candidate;return XUI_OK;}
    xerror* first=xrtTakeError();xui_context context=manager->context;
    xuiInternalCallScopeRecordOutcome(&update->scope.contextScope,status,first);
    if(held)outcome(xuiInternalCallbackOwnerDropOutcome(context,&candidate,status,first),&status,&first);
    xrtSetErrorTake(first);return xuiInternalManagerScopeEnd(&update->scope,status);
}
void xuiInternalManagerCallbackUpdatePublish(xui_manager_callback_update_t* update)
{
    if(!update || !update->scope.self || update->published)abort();
    mutationEnd(&update->mutation);update->published=true;
}
int xuiInternalManagerCallbackUpdateEnd(xui_manager_callback_update_t* update,int status)
{
    if(!update || !update->scope.self || !update->published)abort();
    xui_context context=update->scope.manager->context;
    xerror* first=xrtTakeError();if(status==XUI_OK && first)status=XUI_ERROR_CALLBACK_FAILED;
    xuiInternalCallScopeRecordOutcome(&update->scope.contextScope,status,first);
    outcome(xuiInternalCallbackOwnerDropOutcome(context,&update->previous,status,first),&status,&first);
    update->previous=(xui_callback_owner_t){0};update->published=false;
    xrtSetErrorTake(first);return xuiInternalManagerScopeEnd(&update->scope,status);
}
int xuiInternalManagerCallbackSnapshotBegin(xui_owned_manager_t* manager,const xui_callback_owner_t* incoming,xui_callback_snapshot_t* snapshot,bool report)
{
    if(!manager || !snapshot || snapshot->pContext || !manager->active || !xuiInternalManagerAlive(manager))return XUI_ERROR_INVALID_STATE;
    xui_context admission=manager->context;
    if(admission->bCloseRequested || admission->bDestroyPending || admission->bDestroying){
        if(report)xuiInternalContextRecordOutcome(admission,XUI_ERROR_INVALID_STATE,NULL);
        return XUI_ERROR_INVALID_STATE;
    }
    if(!incoming || !incoming->onHold)return XUI_OK;
    xui_callback_owner_t owner=*incoming;xui_context context=manager->context;
    int status=xuiInternalCallbackOwnerHold(&owner);bool held=status==XUI_OK;
    if(status==XUI_OK && xrtGetError())status=XUI_ERROR_CALLBACK_FAILED;
    if(status==XUI_OK && (!xuiInternalManagerAlive(manager) || context->bCloseRequested || context->bDestroyPending || context->bDestroying))status=XUI_ERROR_INVALID_STATE;
    if(status!=XUI_OK){
        xerror* first=xrtTakeError();
        if(report)xuiInternalContextRecordOutcome(context,status,first);
        if(held)outcome(xuiInternalCallbackOwnerDropOutcome(context,&owner,status,first),&status,&first);
        xrtSetErrorTake(first);return status;
    }
    snapshot->pContext=context;snapshot->tOwner=owner;return XUI_OK;
}
bool xuiInternalContextTraceManagers(xui_context context,xrtownershipvisitor visit,void* data)
{
    if(!context || !visit)return false;
    for(const xui_owned_manager_t* node=context->pOwnedManagers;node;node=node->next)
        if(!node->registered || node->context!=context || !node->ops->Trace(node->data,visit,data))return false;
    return true;
}
bool xuiInternalContextManagersReady(xui_context context)
{
    if(!context)return false;
    for(const xui_owned_manager_t* node=context->pOwnedManagers;node;node=node->next)
        if(node->active || node->closing || node->creatorThread!=xrtThreadCurrentId() || node->ops->Ready(node->data)!=XUI_OK)return false;
    return true;
}
static int prepareManager(xui_owned_manager_t* node,bool* complete)
{
    *complete=false;
    if(node->active || node->closing)return XUI_STATUS_DEFERRED;
    if(node->creatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    ++node->active;mutationEnd(&mutation);
    int status=node->ops->Prepare(node->data,complete);
    if(!xrtOwnershipMutationBegin(&mutation))abort();
    --node->active;mutationEnd(&mutation);
    if(status>=0 && xrtGetError())status=XUI_ERROR_CALLBACK_FAILED;
    if(status==XUI_OK && !*complete)status=XUI_STATUS_DEFERRED;
    return status;
}
int xuiInternalContextPrepareManagers(xui_context context,bool* complete)
{
    if(!complete || !context || context->iOperationDepth<=0)return XUI_ERROR_INVALID_ARGUMENT;
    *complete=true;int result=XUI_OK;xerror* first=NULL;
    for(xui_owned_manager_t* node=context->pOwnedManagers;node;node=node->next){
        bool done=false;int status=prepareManager(node,&done);
        if(!done)*complete=false;
        if(status<0)xuiInternalContextCleanupOutcome(context,status,&result,&first);
    }
    xrtSetErrorTake(first);return result==XUI_OK && !*complete ? XUI_STATUS_DEFERRED:result;
}
static void unlinkManager(xui_owned_manager_t* manager)
{
    xui_context context=manager->context;xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))abort();
    if(manager->previous)manager->previous->next=manager->next;else context->pOwnedManagers=manager->next;
    if(manager->next)manager->next->previous=manager->previous;
    manager->registered=false;manager->previous=manager->next=manager->pendingNext=NULL;
    manager->queued=false;mutationEnd(&mutation);
}
bool xuiInternalContextDrainManagers(xui_context context)
{
    if(!context || (!context->bDestroying && context->iOperationDepth<=0))return false;
    bool progressed=false;
    /* One bounded pass over the initial work list. A deferred first cache
     * must not starve a ready peer which can release its actual dependency. */
    xui_owned_manager_t* node=context->pPendingManagers;
    while(node && !context->pDeferredDestroyHead){
        xui_owned_manager_t* next=node->pendingNext;
        if(!node->active && !node->closing){
            bool complete=false;int prepared=prepareManager(node,&complete);
            if(prepared<0){int result=XUI_OK;xerror* first=NULL;xuiInternalContextCleanupOutcome(context,prepared,&result,&first);xrtSetErrorTake(first);}
            if(complete && !context->pDeferredDestroyHead){
                xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return progressed;
                /* Preparation can close a peer and insert it at the head. */
                xui_owned_manager_t** queued=&context->pPendingManagers;
                while(*queued && *queued!=node)queued=&(*queued)->pendingNext;
                if(*queued!=node)abort();
                *queued=node->pendingNext;mutationEnd(&mutation);
                const xui_owned_manager_ops_t* ops=node->ops;void* data=node->data;
                unlinkManager(node);
                int status=ops->Finalize(data); /* consumed on every result */
                int result=XUI_OK;xerror* first=NULL;
                xuiInternalContextCleanupOutcome(context,status,&result,&first);xrtSetErrorTake(first);
                progressed=true;
            }
        }
        node=next;
    }
    return progressed;
}
void xuiInternalContextDestroyManagers(xui_context context)
{
    if(!context || !context->bDestroying)return;
    /* Close can invalidate and queue a peer. Walk the live directory; no
     * allocation is freed until all Widget destruction tails have drained. */
    for(xui_owned_manager_t* node=context->pOwnedManagers;node;node=node->next)
        if(!node->pending)(void)xuiInternalManagerRequestClose(node);
    xuiInternalWidgetDestroyFlush(context);
    while(context->pPendingManagers){
        if(!xuiInternalContextDrainManagers(context))abort();
        xuiInternalWidgetDestroyFlush(context);
    }
}
#endif
