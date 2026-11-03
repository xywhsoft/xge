#include "xui_config.h"
#if XGE_ENABLE_XUI
#include "xui_internal.h"
#include <stdlib.h>
#include <string.h>

static xui_callback_binding_t** __xuiCallbackFind(xui_widget widget,const void* slot)
{
    if(!widget->pEventOwners)return NULL;
    xui_callback_binding_t** current=&widget->pEventOwners->pBindings;
    while(*current && (*current)->pSlot!=slot)current=&(*current)->pNext;
    return current;
}
bool xuiInternalCallbackSlotOwned(xui_widget widget,const void* slot)
{
    xui_callback_binding_t** current=__xuiCallbackFind(widget,slot);
    return current && *current;
}
static void __xuiCallbackRecordFailure(xui_context context,int result)
{
    for(xui_call_scope_t* scope=context->pCallScopes;scope;scope=scope->pNext)
        if(scope->iResult==XUI_OK)scope->iResult=result;
}
int xuiInternalCallbackUpdateBegin(xui_widget widget,const void* slot,bool hasCallback,void* user,
    const xui_callback_owner_t* incoming,xui_callback_update_t* update)
{
    if(!update || update->pContext || !slot || !xuiInternalWidgetIsValid(widget) ||
        !xuiInternalCallbackOwnerValid(incoming) || (!hasCallback && incoming && incoming->onHold))return XUI_ERROR_INVALID_ARGUMENT;
    xui_callback_owner_t candidate=incoming ? *incoming : (xui_callback_owner_t){0};
    xui_context context=widget->pContext;
    int result=xuiCallScopeBegin(context,&update->tScope);
    if(result!=XUI_OK)return result;
    result=xuiInternalCallbackOwnerHold(&candidate);
    if(result!=XUI_OK){(void)xuiCallScopeEnd(&update->tScope);return result;}
    xui_event_owners_t* header=NULL;xui_callback_binding_t* binding=NULL;
    if(!xuiInternalWidgetIsValid(widget) || context->bDestroyPending || context->bDestroying){result=XUI_ERROR_INVALID_STATE;goto fail;}
    /* Hold may reenter and replace or remove this exact slot. Allocate against
     * the resulting live directory, outside the ownership mutation gate. */
    xui_callback_binding_t** current=__xuiCallbackFind(widget,slot);
    if(candidate.onHold && (!current || !*current)){
        if(!widget->pEventOwners){header=xrtCalloc(1,sizeof(*header));if(!header){result=XUI_ERROR_OUT_OF_MEMORY;goto fail;}}
        binding=xrtCalloc(1,sizeof(*binding));if(!binding){result=XUI_ERROR_OUT_OF_MEMORY;goto fail;}
        binding->pSlot=slot;binding->pUser=user;binding->tOwner=candidate;
    }
    if(!xrtOwnershipMutationBegin(&update->tMutation)){result=XUI_ERROR_INVALID_STATE;goto fail;}
    update->pWidget=widget;update->pContext=context;
    if(header){widget->pEventOwners=header;header=NULL;}
    current=__xuiCallbackFind(widget,slot);
    if(current && *current){
        xui_callback_binding_t* old=*current;update->tOld=old->tOwner;
        if(candidate.onHold){old->tOwner=candidate;old->pUser=user;}
        else{*current=old->pNext;update->pRetired=old;}
    }else if(binding){
        binding->pNext=widget->pEventOwners->pBindings;
        widget->pEventOwners->pBindings=binding;binding=NULL;
    }
    return XUI_OK;
fail:
    xrtFree(binding);xrtFree(header);
    (void)xuiInternalCallbackOwnerDrop(context,&candidate);
    (void)xuiCallScopeEnd(&update->tScope);
    return result;
}
void xuiInternalCallbackUpdatePublish(xui_callback_update_t* update)
{
    if(!update || !update->pContext || update->bPublished || !xrtOwnershipScopeEnd(&update->tMutation))abort();
    update->bPublished=true;
}
int xuiInternalCallbackUpdateEnd(xui_callback_update_t* update,int result)
{
    if(!update || !update->pContext || !update->bPublished)abort();
    xui_context context=update->pContext;
    xui_callback_owner_t old=update->tOld;
    xrtFree(update->pRetired);update->pRetired=NULL;
    int dropped=xuiInternalCallbackOwnerDrop(context,&old);
    int ended=xuiCallScopeEnd(&update->tScope);
    memset(update,0,sizeof(*update));
    return result!=XUI_OK ? result : dropped!=XUI_OK ? dropped : ended;
}
int xuiInternalCallbackSnapshotBegin(xui_widget widget,const void* slot,void** user,xui_callback_snapshot_t* snapshot)
{
    if(!snapshot || !user || snapshot->pContext || !xuiInternalWidgetIsValid(widget))return XUI_ERROR_INVALID_STATE;
    xui_callback_binding_t** current=__xuiCallbackFind(widget,slot);
    if(!current || !*current)return XUI_OK;
    xui_context context=widget->pContext;
    /* The native public operation, not just the callback, owns physical
     * quiescence. Its Leave must follow all native use of widget/output data. */
    if(context->iOperationDepth<=0 || context->bDestroyPending || context->bDestroying)return XUI_ERROR_INVALID_STATE;
    xui_callback_owner_t owner=(*current)->tOwner;void* selectedUser=(*current)->pUser;
    int result=xuiInternalCallbackOwnerHold(&owner);
    if(result!=XUI_OK){__xuiCallbackRecordFailure(context,result);return result;}
    if(!xuiInternalWidgetIsValid(widget) || context->bDestroyPending || context->bDestroying){
        (void)xuiInternalCallbackOwnerDrop(context,&owner);
        __xuiCallbackRecordFailure(context,XUI_ERROR_INVALID_STATE);return XUI_ERROR_INVALID_STATE;
    }
    snapshot->pContext=context;snapshot->tOwner=owner;*user=selectedUser;return XUI_OK;
}
int xuiInternalCallbackSnapshotEnd(xui_callback_snapshot_t* snapshot,int result)
{
    if(!snapshot || !snapshot->pContext)return result;
    xui_context context=snapshot->pContext;xui_callback_owner_t owner=snapshot->tOwner;
    memset(snapshot,0,sizeof(*snapshot));
    int dropped=xuiInternalCallbackOwnerDrop(context,&owner);
    return result!=XUI_OK ? result : dropped;
}
#endif
