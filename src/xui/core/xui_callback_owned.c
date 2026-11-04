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
int xuiInternalCallbackGroupUpdateBegin(xui_widget widget,xui_callback_group_entry_t* entries,
    size_t count,xui_callback_group_update_t* update)
{
    if(!update || update->pContext || !entries || !count || count>16 ||
       !xuiInternalWidgetIsValid(widget))return XUI_ERROR_INVALID_ARGUMENT;
    for(size_t i=0;i<count;++i){
        xui_callback_group_entry_t* entry=&entries[i];
        if(!entry->pSlot || entry->bHeld || entry->pAllocated || entry->pRetired ||
           !xuiInternalCallbackOwnerValid(entry->pOwner) ||
           (!entry->bHasCallback && entry->pOwner && entry->pOwner->onHold))return XUI_ERROR_INVALID_ARGUMENT;
        for(size_t j=0;j<i;++j)if(entries[j].pSlot==entry->pSlot)return XUI_ERROR_INVALID_ARGUMENT;
        entry->tOld=(xui_callback_owner_t){0};
        entry->tCandidate=entry->pOwner ? *entry->pOwner:(xui_callback_owner_t){0};
    }
    xui_context context=widget->pContext;
    int result=xuiCallScopeBegin(context,&update->tScope);
    if(result!=XUI_OK)return result;
    xui_event_owners_t* header=NULL;
    for(size_t i=0;i<count;++i){
        result=xuiInternalCallbackOwnerHold(&entries[i].tCandidate);
        if(result!=XUI_OK)goto fail;
        entries[i].bHeld=true;
        if(!xuiInternalWidgetIsValid(widget) || context->bDestroyPending || context->bDestroying){result=XUI_ERROR_INVALID_STATE;goto fail;}
    }
    /* Hold has finished, including all reentry. Read the resulting directory;
     * no user callback runs during allocation or under the mutation gate. */
    for(size_t i=0;i<count;++i){
        xui_callback_group_entry_t* entry=&entries[i];
        xui_callback_binding_t** current=__xuiCallbackFind(widget,entry->pSlot);
        if(entry->tCandidate.onHold && (!current || !*current)){
            if(!widget->pEventOwners && !header){
                header=xrtCalloc(1,sizeof(*header));
                if(!header){result=XUI_ERROR_OUT_OF_MEMORY;goto fail;}
            }
            entry->pAllocated=xrtCalloc(1,sizeof(*entry->pAllocated));
            if(!entry->pAllocated){result=XUI_ERROR_OUT_OF_MEMORY;goto fail;}
            entry->pAllocated->pSlot=entry->pSlot;
            entry->pAllocated->pUser=entry->pUser;
            entry->pAllocated->tOwner=entry->tCandidate;
        }
    }
    if(!xrtOwnershipMutationBegin(&update->tMutation)){result=XUI_ERROR_INVALID_STATE;goto fail;}
    update->pWidget=widget;update->pContext=context;update->pEntries=entries;update->iCount=count;
    if(header){widget->pEventOwners=header;header=NULL;}
    for(size_t i=0;i<count;++i){
        xui_callback_group_entry_t* entry=&entries[i];
        xui_callback_binding_t** current=__xuiCallbackFind(widget,entry->pSlot);
        if(current && *current){
            xui_callback_binding_t* old=*current;entry->tOld=old->tOwner;
            if(entry->tCandidate.onHold){old->tOwner=entry->tCandidate;old->pUser=entry->pUser;}
            else{*current=old->pNext;entry->pRetired=old;}
        }else if(entry->pAllocated){
            entry->pAllocated->pNext=widget->pEventOwners->pBindings;
            widget->pEventOwners->pBindings=entry->pAllocated;entry->pAllocated=NULL;
        }
    }
    return XUI_OK;
fail:
    xrtFree(header);
    for(size_t i=count;i>0;--i){
        xui_callback_group_entry_t* entry=&entries[i-1];
        xrtFree(entry->pAllocated);entry->pAllocated=NULL;
        if(entry->bHeld){entry->bHeld=false;(void)xuiInternalCallbackOwnerDrop(context,&entry->tCandidate);}
    }
    (void)xuiCallScopeEnd(&update->tScope);
    return result;
}
void xuiInternalCallbackGroupUpdatePublish(xui_callback_group_update_t* update)
{
    if(!update || !update->pContext || update->bPublished || !xrtOwnershipScopeEnd(&update->tMutation))abort();
    update->bPublished=true;
}
int xuiInternalCallbackGroupUpdateEnd(xui_callback_group_update_t* update,int result)
{
    if(!update || !update->pContext || !update->bPublished)abort();
    xui_context context=update->pContext;
    for(size_t i=0;i<update->iCount;++i){
        xui_callback_group_entry_t* entry=&update->pEntries[i];
        xrtFree(entry->pRetired);entry->pRetired=NULL;entry->bHeld=false;
        int dropped=xuiInternalCallbackOwnerDrop(context,&entry->tOld);
        if(result==XUI_OK)result=dropped;
    }
    int ended=xuiCallScopeEnd(&update->tScope);
    memset(update,0,sizeof(*update));
    return result!=XUI_OK ? result:ended;
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
