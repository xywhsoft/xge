/* Exact native publisher/dispatch admission after Hold reenters RequestClose.
 * Offline/CPU-only. A fake language wrapper is not used. */
#define XUI_OWNED_LAYOUT_HELPERS
#include "xui_owned_layout_test.c"
typedef struct RequestedOwner {size_t refs;unsigned holds,drops;bool request,failNext,secondary;xui_context context;} RequestedOwner;
static bool reqHold(const void* raw)
{
    RequestedOwner* owner=(RequestedOwner*)raw;assert(owner->refs);++owner->refs;++owner->holds;
    if(owner->request)assert(xuiRequestClose(owner->context)==XUI_OK);
    return true;
}
static bool reqDrop(const void* raw)
{
    RequestedOwner* owner=(RequestedOwner*)raw;assert(owner->refs);--owner->refs;++owner->drops;
    bool failure=owner->failNext;owner->failNext=false;
    if(failure && owner->secondary)xrtSetErrorKind(XERR_STATE);
    return !failure;
}
static bool reqCount(const void* raw,size_t* count){*count=((const RequestedOwner*)raw)->refs;return *count!=0;}
static const xrtownershipops reqOps={reqCount,trace};
static xui_callback_owner_t reqRef(RequestedOwner* owner){return(xui_callback_owner_t){{owner,&reqOps},reqHold,reqDrop};}
static void noCall(void){}
static void updateRejected(bool group,bool failedDrop)
{
    xui_context context=NULL;xui_widget widget=NULL;assert(xuiCreate(&context)==XUI_OK && xuiWidgetCreate(context,&widget)==XUI_OK);
    RequestedOwner a={.refs=1,.request=true,.context=context,.failNext=failedDrop,.secondary=failedDrop && !group},b={.refs=1,.context=context};
    xui_callback_owner_t ar=reqRef(&a),br=reqRef(&b);void(*one)(void)=NULL;void(*two)(void)=NULL;
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(context,&outer)==XUI_OK);
    int status;
    if(group){
        xui_callback_group_entry_t entries[]={ {.pSlot=&one,.bHasCallback=true,.pOwner=&ar},{.pSlot=&two,.bHasCallback=true,.pOwner=&br} };
        xui_callback_group_update_t update={0};status=xuiInternalCallbackGroupUpdateBegin(widget,entries,2,&update);
        assert(!update.pContext && !entries[0].bHeld && !entries[1].bHeld && !b.holds && !b.drops);
    }else{xui_callback_update_t update={0};status=xuiInternalCallbackUpdateBegin(widget,&one,true,NULL,&ar,&update);assert(!update.pContext);}
    assert(status==XUI_ERROR_INVALID_STATE && one==NULL && two==NULL && !widget->pEventOwners && a.refs==1 && a.holds==1 && a.drops==1 && !xrtGetError());
    assert(xuiCallScopeEnd(&outer)==(failedDrop ? XUI_ERROR_CALLBACK_FAILED:XUI_OK) && !xrtGetError());
    assert(xuiDestroyChecked(context)==XUI_OK);assert(reqDrop(&a) && reqDrop(&b));
}
static void snapshotRejected(bool reported,bool failedDrop)
{
    xui_context context=NULL;xui_widget widget=NULL;assert(xuiCreate(&context)==XUI_OK && xuiWidgetCreate(context,&widget)==XUI_OK);
    RequestedOwner owner={.refs=1,.context=context};xui_callback_owner_t ref=reqRef(&owner);void(*callback)(void)=NULL;
    xui_callback_update_t update={0};assert(xuiInternalCallbackUpdateBegin(widget,&callback,true,&owner,&ref,&update)==XUI_OK);
    callback=noCall;xuiInternalCallbackUpdatePublish(&update);assert(xuiInternalCallbackUpdateEnd(&update,XUI_OK)==XUI_OK && owner.refs==2);
    owner.request=true;owner.failNext=failedDrop;owner.secondary=failedDrop && reported;
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(context,&outer)==XUI_OK);
    xui_callback_snapshot_t snapshot={0};void* user=(void*)(uintptr_t)0x12;
    int status=reported ? xuiInternalCallbackSnapshotBeginReported(widget,&callback,&user,&snapshot):xuiInternalCallbackSnapshotBeginReturned(widget,&callback,&user,&snapshot);
    assert(status==XUI_ERROR_INVALID_STATE && !snapshot.pContext && user==(void*)(uintptr_t)0x12 && owner.refs==2 && owner.holds==2 && owner.drops==1 && !xrtGetError());
    int expected=reported ? XUI_ERROR_INVALID_STATE:failedDrop ? XUI_ERROR_CALLBACK_FAILED:XUI_OK;
    assert(xuiCallScopeEnd(&outer)==expected && !xrtGetError());
    assert(xuiDestroyChecked(context)==XUI_OK && owner.refs==1);assert(reqDrop(&owner));
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    updateRejected(false,false);updateRejected(false,true);updateRejected(true,false);updateRejected(true,true);
    snapshotRejected(false,false);snapshotRejected(false,true);snapshotRejected(true,false);snapshotRejected(true,true);
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("requested callback admission: Hold reentry rejects publication/dispatch; next group Hold skipped; Returned local vs Reported/cleanup sticky, first NULL preserved");return 0;
}
