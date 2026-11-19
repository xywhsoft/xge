/* Included after the numeric model implementation. No language-specific
 * callback bridge: native consumers call the original typed function once. */
static int __xuiCodeMarginSetCallback(xui_code_margin_model model,int id,int role,
    xui_code_margin_render_proc render,xui_code_margin_event_proc event,
    void* user,const xui_callback_owner_t* incoming)
{
    bool has=role==1 ? render!=NULL:event!=NULL;
    if(!xuiInternalCallbackOwnerRawValid(incoming) ||
       (!has && incoming && incoming->onHold))return XUI_ERROR_INVALID_ARGUMENT;
    xui_callback_owner_t owner={0};if(incoming)owner=*incoming;
    xui_code_margin_scope_t scope={0};
    int status=xuiCodeMarginModelCallBegin(model,&scope);if(status!=XUI_OK)return status;
    xui_code_margin_slot_t* node=NULL;
    bool held=false;
    if(id<=0)status=XUI_ERROR_INVALID_ARGUMENT;
    else if(__xuiCodeMarginFind(model,id)<0)status=XUI_ERROR_UNSUPPORTED;
    if(status==XUI_OK && has){
        node=xrtCalloc(1,sizeof(*node));
        if(!node)status=XUI_ERROR_OUT_OF_MEMORY;
        else {
            node->owner=owner;node->user=user;node->id=id;node->role=role;
            if(role==1)node->fn.render=render;else node->fn.event=event;
            status=xuiInternalCallbackHoldFresh(&owner,&held);
        }
    }
    /* Hold may reenter: reread admission and the id, never retain an array
     * index or publish into a model consumed by that reentrant call. */
    if(status==XUI_OK)status=__xuiCodeMarginReadStatus(model);
    if(status==XUI_OK && scope.pContext &&
       (scope.pContext->bCloseRequested || scope.pContext->bDestroyPending))status=XUI_ERROR_INVALID_STATE;
    int index=status==XUI_OK ? __xuiCodeMarginFind(model,id):-1;
    if(status==XUI_OK && index<0)status=XUI_ERROR_UNSUPPORTED;
    xui_code_margin_slot_t* old=NULL;
    if(status==XUI_OK){
        xrtownershipscope mutation={0};
        if(!xrtOwnershipMutationBegin(&mutation))status=XUI_ERROR_INVALID_STATE;
        else {
            old=__xuiCodeMarginDetach(model,id,role);
            if(node){node->refs=1;node->next=model->slots;model->slots=node;node=NULL;held=false;}
            /* Clearing also clears a legacy borrowed callback of that role. */
            if(role==1)model->pMargins[index].onRender=NULL;
            else model->pMargins[index].onEvent=NULL;
            if(!xrtOwnershipScopeEnd(&mutation))abort();
        }
    }
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,status);
    if(node){
        if(held)xuiInternalCleanupAdd(&receipt,xuiInternalCallbackOwnerDropOutcome(scope.pContext,&node->owner,receipt.status,receipt.error));
        xrtFree(node);
    }
    if (status == XUI_OK && role == 1) __xuiCodeMarginChanged(model, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
    __xuiCodeMarginReleaseList(scope.pContext,old,&receipt);
    return __xuiCodeMarginFinish(&scope,&receipt);
}
XUI_API int xuiCodeMarginModelSetRenderCallbackOwned(xui_code_margin_model model,
    int id,xui_code_margin_render_proc callback,void* user,const xui_callback_owner_t* owner)
{return __xuiCodeMarginSetCallback(model,id,1,callback,NULL,user,owner);}
XUI_API int xuiCodeMarginModelSetEventCallbackOwned(xui_code_margin_model model,
    int id,xui_code_margin_event_proc callback,void* user,const xui_callback_owner_t* owner)
{return __xuiCodeMarginSetCallback(model,id,2,NULL,callback,user,owner);}
XUI_API int xuiCodeMarginModelGetState(xui_code_margin_model model,int index,xui_code_margin_state_t* output)
{
    int status=__xuiCodeMarginReadStatus(model);if(status!=XUI_OK)return status;
    if(!output || index<0 || index>=model->iCount)return XUI_ERROR_INVALID_ARGUMENT;
    const xui_code_margin_desc_t* d=&model->pMargins[index];
    *output=(xui_code_margin_state_t){sizeof(*output),d->iId,d->iKind,d->fWidth,d->iFlags};
    return XUI_OK;
}
static xui_code_margin_slot_t* __xuiCodeMarginSlot(xui_code_margin_model model,int id,int role)
{
    for(xui_code_margin_slot_t* n=model->slots;n;n=n->next)if(n->id==id && n->role==role)return n;
    return NULL;
}
static int __xuiCodeMarginRetain(xui_code_margin_slot_t* node)
{
    if(!node)return XUI_OK;
    if(!node->refs)return XUI_ERROR_INVALID_STATE;
    if(node->refs==SIZE_MAX)return XUI_ERROR_LIMIT_EXCEEDED;
    ++node->refs;return XUI_OK;
}
int xuiInternalCodeMarginSnapshotBegin(xui_code_margin_model model,xui_rect_t viewport,
    xui_code_margin_entry_t* entries,int capacity,xui_code_margin_snapshot_t* frame,xui_rect_t* text)
{
    if(!frame || frame->self || frame->scope.pSelf || frame->entries || frame->count ||
       frame->ending || frame->invoking || capacity<0 || (!entries && capacity))return XUI_ERROR_INVALID_ARGUMENT;
    int status=xuiCodeMarginModelCallBegin(model,&frame->scope);if(status!=XUI_OK)return status;
    int count=0;
    status=__xuiCodeMarginLayout(model,viewport,NULL,entries,capacity,&count,text);
    if(status==XUI_OK && count>capacity)status=XUI_ERROR_LIMIT_EXCEEDED;
    int pinned=0;
    if(status==XUI_OK){
        xrtownershipscope mutation={0};
        if(!xrtOwnershipMutationBegin(&mutation))status=XUI_ERROR_INVALID_STATE;
        else {
            for(int i=0;i<count;++i){
                xui_code_margin_slot_t* node=__xuiCodeMarginSlot(model,entries[i].info.iId,1);
                status=__xuiCodeMarginRetain(node);if(status!=XUI_OK)break;
                entries[i].render=node;++pinned;
            }
            if(!xrtOwnershipScopeEnd(&mutation))abort();
        }
    }
    frame->self=frame;frame->entries=entries;frame->count=pinned;
    if(status!=XUI_OK)return xuiInternalCodeMarginSnapshotEnd(frame,status);
    return XUI_OK;
}
int xuiInternalCodeMarginSnapshotEnd(xui_code_margin_snapshot_t* frame,int primary)
{
    if(!frame || frame->self!=frame || !frame->scope.pModel)return XUI_ERROR_INVALID_ARGUMENT;
    if(frame->scope.pModel->iCreatorThread!=xrtThreadCurrentId() || frame->invoking || frame->ending)return XUI_ERROR_BUSY;
    frame->ending=true;
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,primary);
    for(int i=0;i<frame->count;++i){
        xui_code_margin_slot_t* node=frame->entries[i].render;frame->entries[i].render=NULL;
        __xuiCodeMarginRelease(frame->scope.pContext,node,&receipt);
    }
    frame->entries=NULL;frame->count=0;
    int status=__xuiCodeMarginFinish(&frame->scope,&receipt);
    frame->self=NULL;frame->ending=false;return status;
}
static int __xuiCodeMarginWidgetBegin(xui_code_margin_model model,xui_widget widget,xui_call_scope_t* scope)
{
    if(!xuiInternalWidgetIsValid(widget) ||
       (model->pInternalOwner && model->pInternalOwner!=widget))return XUI_ERROR_INVALID_ARGUMENT;
    return xuiCallScopeBegin(widget->pContext,scope);
}
static int __xuiCodeMarginInvokeEnd(xui_code_margin_model model,xui_widget widget,
    xui_call_scope_t* scope,xui_code_margin_slot_t* node,int status)
{
    if(status==XUI_OK && (!xuiInternalWidgetIsValid(widget) ||
       widget->pContext->bCloseRequested || widget->pContext->bDestroyPending ||
       __xuiCodeMarginReadStatus(model)!=XUI_OK))status=XUI_ERROR_INVALID_STATE;
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,status);
    __xuiCodeMarginRelease(scope->pContext,node,&receipt);
    receipt.status=xuiInternalCallScopeEndOutcome(scope,receipt.status,&receipt.error);
    return xuiInternalCleanupEnd(NULL,&receipt);
}
int xuiInternalCodeMarginSnapshotRender(xui_code_margin_snapshot_t* frame,int index,
    xui_widget widget,int line,xui_draw_context draw,xui_rect_t rect,bool* called)
{
    if(!called)return XUI_ERROR_INVALID_ARGUMENT;
    *called=false;
    if(!frame || frame->self!=frame || !frame->scope.pModel || index<0 || index>=frame->count)return XUI_ERROR_INVALID_ARGUMENT;
    xui_code_margin_model model=frame->scope.pModel;
    int status=__xuiCodeMarginReadStatus(model);if(status!=XUI_OK)return status;
    if(frame->ending || frame->invoking)return XUI_ERROR_BUSY;
    xui_code_margin_entry_t* entry=&frame->entries[index];
    xui_code_margin_slot_t* node=entry->render;
    xui_code_margin_render_proc fn=node ? node->fn.render:entry->info.onRender;
    void* user=node ? node->user:entry->info.pUser;int id=entry->info.iId;
    if(!fn)return XUI_OK;
    xui_call_scope_t scope={0};status=__xuiCodeMarginWidgetBegin(model,widget,&scope);if(status!=XUI_OK)return status;
    status=__xuiCodeMarginRetain(node);
    if(status==XUI_OK){
        frame->invoking=true;*called=true;status=fn(widget,id,line,draw,rect,user);frame->invoking=false;
    }else node=NULL;
    return __xuiCodeMarginInvokeEnd(model,widget,&scope,node,status);
}
int xuiInternalCodeMarginInvokeEvent(xui_code_margin_model model,int index,
    xui_widget widget,int line,int event,xui_rect_t rect,bool* called)
{
    if(!called)return XUI_ERROR_INVALID_ARGUMENT;
    *called=false;xui_code_margin_scope_t modelScope={0};
    int status=xuiCodeMarginModelCallBegin(model,&modelScope);if(status!=XUI_OK)return status;
    xui_call_scope_t scope={0};xui_code_margin_slot_t* node=NULL;
    if(index<0 || index>=model->iCount)status=XUI_ERROR_INVALID_ARGUMENT;
    if(status==XUI_OK)status=__xuiCodeMarginWidgetBegin(model,widget,&scope);
    if(status==XUI_OK){
        int id=model->pMargins[index].iId;node=__xuiCodeMarginSlot(model,id,2);
        xui_code_margin_event_proc fn=node ? node->fn.event:model->pMargins[index].onEvent;
        void* user=node ? node->user:model->pMargins[index].pUser;
        status=__xuiCodeMarginRetain(node);
        if(status==XUI_OK){if(fn){*called=true;status=fn(widget,id,line,event,rect,user);}}
        else node=NULL;
        status=__xuiCodeMarginInvokeEnd(model,widget,&scope,node,status);
    }
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,status);
    return __xuiCodeMarginFinish(&modelScope,&receipt);
}
