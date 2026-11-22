#include "xui_config.h"
#if XGE_ENABLE_XUI
#include "xui_internal.h"
#include "xui_callback_hold_internal.h"
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
{xuiInternalContextRecordFailure(context,result);}
/* A false cleanup can have no diagnostic. Preserve the first status and its
 * exact (possibly NULL) error, never a later error allocated during rollback. */
static void __xuiCallbackOutcome(int next,int* result,xerror** first)
{
    xerror* error=xrtTakeError();
    if(next==XUI_OK && !error)return;
    if(*result==XUI_OK){*result=next!=XUI_OK ? next:XUI_ERROR_CALLBACK_FAILED;*first=error;}
    else xrtErrorFree(error);
}
/* Owned endpoints peek before arbitrary consuming cleanup. A diagnostic from
 * a void callback is still a primary failure; Drop must not replace it. */
static xerror* __xuiCallbackFirstOutcome(xui_context context,int* result)
{
    if(*result==XUI_OK && xrtGetError()!=NULL)*result=XUI_ERROR_CALLBACK_FAILED;
    xerror* first=*result!=XUI_OK ? xrtTakeError():NULL;
    (void)context;
    return first;
}
static int __xuiCallbackUpdateBegin(xui_widget widget,const void* slot,bool hasCallback,void* user,
    const xui_callback_owner_t* incoming,xui_callback_preflight_proc preflight,void* preflightData,
    bool latePrepare,xui_callback_update_t* update)
{
    if(!update || update->pContext || !slot || !xuiInternalWidgetIsValid(widget) ||
        !xuiInternalCallbackOwnerValid(incoming) || (!hasCallback && incoming && incoming->onHold))return XUI_ERROR_INVALID_ARGUMENT;
    xui_callback_owner_t candidate=incoming ? *incoming : (xui_callback_owner_t){0};
    xui_context context=widget->pContext;
    int result=xuiCallScopeBegin(context,&update->tScope);
    if(result!=XUI_OK)return result;
    xui_event_owners_t* header=NULL;xui_callback_binding_t* binding=NULL;
    bool held=false;result=xuiInternalCallbackHoldFresh(&candidate,&held);
    if(result!=XUI_OK){
        if(held)goto fail;
        xerror* first=xrtTakeError();
        result=xuiInternalCallScopeEndOutcome(&update->tScope,result,&first);
        xrtSetErrorTake(first);return result;
    }
    if(!xuiInternalWidgetIsValid(widget) || context->bCloseRequested || context->bDestroyPending || context->bDestroying){result=XUI_ERROR_INVALID_STATE;goto fail;}
    if(preflight && !latePrepare){result=preflight(widget,preflightData);if(result!=XUI_OK)goto fail;}
    /* Hold may reenter and replace or remove this exact slot. Allocate against
     * the resulting live directory, outside the ownership mutation gate. */
    xui_callback_binding_t** current=__xuiCallbackFind(widget,slot);
    if(candidate.onHold && (!current || !*current)){
        if(!widget->pEventOwners){header=xrtCalloc(1,sizeof(*header));if(!header){result=XUI_ERROR_OUT_OF_MEMORY;goto fail;}}
        binding=xrtCalloc(1,sizeof(*binding));if(!binding){result=XUI_ERROR_OUT_OF_MEMORY;goto fail;}
        binding->pSlot=slot;binding->pUser=user;binding->tOwner=candidate;
    }
    if(preflight && latePrepare){
        result=preflight(widget,preflightData);
        if(result!=XUI_OK)goto fail;
        if(!xuiInternalWidgetIsValid(widget) || context->bCloseRequested || context->bDestroyPending || context->bDestroying){result=XUI_ERROR_INVALID_STATE;goto fail;}
        if(header && widget->pEventOwners){xrtFree(header);header=NULL;}
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
fail: {
    xerror* first=xrtTakeError();
    xuiInternalCallScopeRecordOutcome(&update->tScope,result,first);
    xrtFree(binding);xrtFree(header);
    __xuiCallbackOutcome(xuiInternalCallbackOwnerDropOutcome(context,&candidate,result,first),&result,&first);
    result=xuiInternalCallScopeEndOutcome(&update->tScope,result,&first);
    xrtSetErrorTake(first);return result;
}
}
int xuiInternalCallbackUpdateBeginValidated(xui_widget widget,const void* slot,bool hasCallback,void* user,
    const xui_callback_owner_t* incoming,xui_callback_preflight_proc preflight,void* preflightData,xui_callback_update_t* update)
{return __xuiCallbackUpdateBegin(widget,slot,hasCallback,user,incoming,preflight,preflightData,false,update);}
int xuiInternalCallbackUpdateBeginPrepared(xui_widget widget,const void* slot,bool hasCallback,void* user,
    const xui_callback_owner_t* incoming,xui_callback_preflight_proc prepare,void* prepareData,xui_callback_update_t* update)
{return __xuiCallbackUpdateBegin(widget,slot,hasCallback,user,incoming,prepare,prepareData,true,update);}
int xuiInternalCallbackUpdateBegin(xui_widget widget,const void* slot,bool hasCallback,void* user,
    const xui_callback_owner_t* incoming,xui_callback_update_t* update)
{return xuiInternalCallbackUpdateBeginValidated(widget,slot,hasCallback,user,incoming,NULL,NULL,update);}
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
    xerror* first=__xuiCallbackFirstOutcome(context,&result);
    __xuiCallbackOutcome(xuiInternalCallbackOwnerDropOutcome(context,&old,result,first),&result,&first);
    result=xuiInternalCallScopeEndOutcome(&update->tScope,result,&first);
    memset(update,0,sizeof(*update));
    xrtSetErrorTake(first);return result;
}
int xuiInternalCallbackGroupUpdateBegin(xui_widget widget,xui_callback_group_entry_t* entries,
    size_t count,xui_callback_group_update_t* update)
{
    if(!update || update->pContext || !entries || !count ||
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
        result=xuiInternalCallbackHoldFresh(&entries[i].tCandidate,&entries[i].bHeld);
        if(result!=XUI_OK)goto fail;
        if(!xuiInternalWidgetIsValid(widget) || context->bCloseRequested || context->bDestroyPending || context->bDestroying){result=XUI_ERROR_INVALID_STATE;goto fail;}
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
fail: {
    xerror* first=xrtTakeError();xuiInternalCallScopeRecordOutcome(&update->tScope,result,first);xrtFree(header);
    for(size_t i=count;i>0;--i){
        xui_callback_group_entry_t* entry=&entries[i-1];
        xrtFree(entry->pAllocated);entry->pAllocated=NULL;
        if(entry->bHeld){entry->bHeld=false;__xuiCallbackOutcome(xuiInternalCallbackOwnerDropOutcome(context,&entry->tCandidate,result,first),&result,&first);}
    }
    result=xuiInternalCallScopeEndOutcome(&update->tScope,result,&first);
    xrtSetErrorTake(first);return result;
}
}
void xuiInternalCallbackGroupUpdatePublish(xui_callback_group_update_t* update)
{
    if(!update || !update->pContext || update->bPublished || !xrtOwnershipScopeEnd(&update->tMutation))abort();
    update->bPublished=true;
}
int xuiInternalCallbackGroupUpdateEnd(xui_callback_group_update_t* update,int result)
{
    if(!update || !update->pContext || !update->bPublished)abort();
    xui_context context=update->pContext;xerror* first=__xuiCallbackFirstOutcome(context,&result);
    for(size_t i=0;i<update->iCount;++i){
        xui_callback_group_entry_t* entry=&update->pEntries[i];
        xrtFree(entry->pRetired);entry->pRetired=NULL;entry->bHeld=false;
        __xuiCallbackOutcome(xuiInternalCallbackOwnerDropOutcome(context,&entry->tOld,result,first),&result,&first);
    }
    result=xuiInternalCallScopeEndOutcome(&update->tScope,result,&first);
    memset(update,0,sizeof(*update));
    xrtSetErrorTake(first);return result;
}

static int __xuiCallbackSnapshotBegin(xui_widget widget,const void* slot,void** user,xui_callback_snapshot_t* snapshot,bool report)
{
    if(!snapshot || !user || snapshot->pContext || !xuiInternalWidgetIsValid(widget))return XUI_ERROR_INVALID_STATE;
    xui_context context=widget->pContext;
    if(context->bCloseRequested){
        if(report)__xuiCallbackRecordFailure(context,XUI_ERROR_INVALID_STATE);
        return XUI_ERROR_INVALID_STATE;
    }
    xui_callback_binding_t** current=__xuiCallbackFind(widget,slot);
    if(!current || !*current)return XUI_OK;
    /* The native public operation, not just the callback, owns physical
     * quiescence. Its Leave must follow all native use of widget/output data. */
    if(context->iOperationDepth<=0 || context->bCloseRequested || context->bDestroyPending || context->bDestroying)return XUI_ERROR_INVALID_STATE;
    xui_callback_owner_t owner=(*current)->tOwner;void* selectedUser=(*current)->pUser;
    bool held=false;int result=xuiInternalCallbackHoldIsolated(&owner,&held);
    if(result==XUI_OK && (!xuiInternalWidgetIsValid(widget) || context->bCloseRequested || context->bDestroyPending || context->bDestroying))result=XUI_ERROR_INVALID_STATE;
    if(result!=XUI_OK){
        xerror* first=xrtTakeError();
        /* A dispatch that cannot enter its selected callback must reach the
         * enclosing receipt, including native void consumers. */
        if(report)xuiInternalContextRecordOutcome(context,result,first);
        if(held)__xuiCallbackOutcome(xuiInternalCallbackOwnerDropOutcome(context,&owner,result,first),&result,&first);
        xrtSetErrorTake(first);return result;
    }
    snapshot->pContext=context;snapshot->tOwner=owner;*user=selectedUser;return XUI_OK;
}
int xuiInternalCallbackSnapshotBeginReturned(xui_widget widget,const void* slot,void** user,xui_callback_snapshot_t* snapshot)
{return __xuiCallbackSnapshotBegin(widget,slot,user,snapshot,false);}
int xuiInternalCallbackSnapshotBeginReported(xui_widget widget,const void* slot,void** user,xui_callback_snapshot_t* snapshot)
{return __xuiCallbackSnapshotBegin(widget,slot,user,snapshot,true);}
int xuiInternalCallbackSnapshotEndOutcome(xui_callback_snapshot_t* snapshot,int result,xerror* first)
{
    if(!snapshot || !snapshot->pContext){
        if(result!=XUI_OK || first)xrtSetErrorTake(first);
        return result;
    }
    xui_context context=snapshot->pContext;xui_callback_owner_t owner=snapshot->tOwner;
    memset(snapshot,0,sizeof(*snapshot));
    xerror* ambient=xrtTakeError();
    if(result==XUI_OK && (first || ambient)){
        result=XUI_ERROR_CALLBACK_FAILED;
        if(!first){first=ambient;ambient=NULL;}
    }
    xrtErrorFree(ambient);
    __xuiCallbackOutcome(xuiInternalCallbackOwnerDropOutcome(context,&owner,result,first),&result,&first);
    xrtSetErrorTake(first);return result;
}
int xuiInternalCallbackSnapshotEnd(xui_callback_snapshot_t* snapshot,int result)
{
    if(!snapshot || !snapshot->pContext)return result;
    /* The explicit tail takes TLS once on the successful path. */
    xerror* first=result!=XUI_OK ? xrtTakeError():NULL;
    return xuiInternalCallbackSnapshotEndOutcome(snapshot,result,first);
}

int xuiInternalCallbackRecordCreate(xui_context context,bool hasCallback,
    const xui_callback_owner_t* incoming,xui_callback_record_t** output)
{
    if(!output || !xuiInternalCallbackOwnerValid(incoming) ||
       (!hasCallback && incoming && incoming->onHold))return XUI_ERROR_INVALID_ARGUMENT;
    *output=NULL;
    if(!incoming || !incoming->onHold)return XUI_OK;
    xui_callback_record_t* record=xrtCalloc(1,sizeof(*record));
    if(!record)return XUI_ERROR_OUT_OF_MEMORY;
    record->tOwner=*incoming;bool held=false;
    int status=xuiInternalCallbackHoldFresh(&record->tOwner,&held);
    if(status==XUI_OK && (context->bCloseRequested || context->bDestroyPending || context->bDestroying))status=XUI_ERROR_INVALID_STATE;
    if(status!=XUI_OK){
        xerror* first=xrtTakeError();xuiInternalContextRecordOutcome(context,status,first);
        if(held)__xuiCallbackOutcome(xuiInternalCallbackOwnerDropOutcome(context,&record->tOwner,status,first),&status,&first);
        xrtFree(record);xrtSetErrorTake(first);return status;
    }
    record->iReferences=1;*output=record;return XUI_OK;
}
int xuiInternalCallbackRecordRetain(xui_callback_record_t* record)
{
    if(!record)return XUI_OK;
    if(!record->iReferences)return XUI_ERROR_INVALID_STATE;
    if(record->iReferences==SIZE_MAX)return XUI_ERROR_LIMIT_EXCEEDED;
    ++record->iReferences;return XUI_OK;
}
int xuiInternalCallbackRecordRelease(xui_context context,xui_callback_record_t* record,
    int primary,const xerror* error)
{
    if(!record)return XUI_OK;
    /* Private references are UI-thread local; published slots unlink under
     * the graph gate before release. Native operations protect snapshots. */
    if(!record->iReferences)return XUI_ERROR_INVALID_STATE;
    bool consumed=--record->iReferences==0;
    if(!consumed)return XUI_OK;
    xui_callback_owner_t owner=record->tOwner;xrtFree(record);
    return xuiInternalCallbackOwnerDropOutcome(context,&owner,primary,error);
}
#endif
