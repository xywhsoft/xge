/* Own typed callbacks, not their arbitrary returned storage or a language ABI. */
#include "xui_code_margin_host.h"
#include "tests/xui/xui_test_proxy.c"
#include <stdio.h>
typedef struct Owner {
    size_t refs;unsigned holds,drops,calls;
    bool holdFalse,holdError,dropFalse,dropError;
    int holdAction,dropAction,callAction;
    xui_code_provider_set model;
    xui_code_provider_scope_t* caller;
    char label[16];xui_code_signature_parameter_t parameter;
} Owner;
static void action(Owner* owner,int code)
{
    if(code==1)assert(xuiCodeProviderSetClearChecked(owner->model)==XUI_OK);
    if(code==2){bool consumed=false;assert(xuiCodeProviderSetDestroyOwnerChecked(owner->model,&consumed)==XUI_OK&&consumed);}
    if(code==3){assert(xuiCodeProviderSetCallEnd(owner->caller)==XUI_OK);assert(xuiCodeProviderSetCallBegin(owner->model,owner->caller)==XUI_OK);}
}
static bool hold(const void* raw)
{
    Owner* owner=(Owner*)raw;++owner->holds;
    if(owner->holdFalse)return false;
    ++owner->refs;int code=owner->holdAction;owner->holdAction=0;action(owner,code);
    if(owner->holdError)xrtSetErrorKind(XERR_IO);
    return true;
}
static bool drop(const void* raw)
{
    Owner* owner=(Owner*)raw;assert(owner->refs>1);--owner->refs;++owner->drops;
    if(owner->refs==1)strcpy(owner->label,"expired");
    int code=owner->dropAction;owner->dropAction=0;action(owner,code);
    if(owner->dropError)xrtSetErrorKind(XERR_IO);
    return !owner->dropFalse;
}
static bool count(const void* raw,size_t* n){*n=((const Owner*)raw)->refs;return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static xui_callback_owner_t capability(Owner* owner){return (xui_callback_owner_t){{owner,&ops},hold,drop};}
static void init(Owner* owner,xui_code_provider_set model)
{
    memset(owner,0,sizeof(*owner));owner->refs=1;owner->model=model;strcpy(owner->label,"owned");
    owner->parameter.iSize=sizeof(owner->parameter);owner->parameter.tLabelRange=(xui_code_range_t){0,1};
}
static void invoked(Owner* owner)
{
    assert(owner->refs>1);++owner->calls;
    int code=owner->callAction;owner->callAction=0;
    if(code==4)xrtSetErrorKind(XERR_IO);else action(owner,code);
    assert(owner->refs>1);
}
static int complete(xui_widget widget,int offset,const char* prefix,xui_code_completion_item_t* items,int capacity,int* n,void* raw)
{
    (void)widget;assert(offset==0&&prefix&&capacity>0);Owner* owner=raw;invoked(owner);
    items[0].sLabel=owner->label;items[0].sInsertText=owner->label;items[0].sDetail=owner->label;*n=1;return XUI_OK;
}
static int hover(xui_widget widget,int offset,xui_code_hover_t* result,void* raw)
{(void)widget;assert(offset==0);Owner* owner=raw;invoked(owner);result->sText=owner->label;return XUI_OK;}
static int signature(xui_widget widget,int offset,xui_code_signature_help_t* result,void* raw)
{
    (void)widget;assert(offset==0);Owner* owner=raw;invoked(owner);result->sLabel=owner->label;
    result->sDocumentation=owner->label;result->pParameters=&owner->parameter;result->iParameterCount=1;result->iActiveParameter=0;return XUI_OK;
}
static int command(xui_widget widget,int id,const void* data,int* result,void* raw)
{(void)widget;(void)data;assert(id>0);invoked(raw);*result=7;return XUI_OK;}
static int enabled(xui_widget widget,int id,int* result,void* raw)
{return command(widget,id,NULL,result,raw);}
static int completeReply(xui_widget widget,int offset,xui_code_completion_reply_t* reply,void* raw)
{
    (void)widget;
    Owner* owner=raw;
    assert(offset==0&&strcmp(xuiCodeCompletionReplyGetPrefix(reply),"pr")==0);
    assert(xuiCodeCompletionReplyGetRemaining(reply)==2);
    char label[]="print";
    xui_code_completion_item_t item={.iSize=sizeof(item),.sLabel=label,.sInsertText=label,
        .sDocumentation="A local reply",.sSortText="",.iUserData=31};
    int status=xuiCodeCompletionReplyAppendItem(reply,&item);
    if(status!=XUI_OK)return status;
    memset(label,'x',sizeof(label)-1);
    assert(xuiCodeCompletionReplyGetRemaining(reply)==1);
    assert(xuiCodeCompletionReplyAppendItems(reply,&item,2)==XUI_ERROR_LIMIT_EXCEEDED);
    invoked(owner);
    return XUI_OK;
}
static int hoverReply(xui_widget widget,int offset,xui_code_hover_reply_t* reply,void* raw)
{
    (void)widget;assert(offset==0);
    char text[]="local hover";
    xui_code_hover_t value={.iSize=sizeof(value),.tRange={0,3},.sText=text,.sContentType=""};
    int status=xuiCodeHoverReplySet(reply,&value);
    if(status!=XUI_OK)return status;
    value.tRange.iStart=4;
    assert(xuiCodeHoverReplySet(reply,&value)==XUI_ERROR_INVALID_ARGUMENT);
    memset(text,'x',sizeof(text)-1);
    invoked(raw);
    return XUI_OK;
}
static int signatureReply(xui_widget widget,int offset,xui_code_signature_reply_t* reply,void* raw)
{
    (void)widget;assert(offset==0);
    char label[]="print(value)",parameter[]="value";
    xui_code_signature_parameter_t argument={.iSize=sizeof(argument),.sLabel=parameter,.tLabelRange={6,11}};
    xui_code_signature_help_t value={.iSize=sizeof(value),.sLabel=label,.pParameters=&argument,
        .iParameterCount=1,.iActiveParameter=0,.iUserData=73};
    int status=xuiCodeSignatureReplySet(reply,&value);
    if(status!=XUI_OK)return status;
    memset(label,'x',sizeof(label)-1);
    memset(parameter,'x',sizeof(parameter)-1);
    argument.tLabelRange=(xui_code_range_t){0,0};
    invoked(raw);
    return XUI_OK;
}
static void replyStorage(void)
{
    xui_code_provider_set model=NULL;
    assert(xuiCodeProviderSetCreate(&model)==XUI_OK);
    Owner o;init(&o,model);
    xui_callback_owner_t cap=capability(&o);
    assert(xuiCodeProviderSetCompletionReplyOwned(model,completeReply,&o,&cap)==XUI_OK);
    assert(xuiCodeProviderSetHoverReplyOwned(model,hoverReply,&o,&cap)==XUI_OK);
    assert(xuiCodeProviderSetSignatureReplyOwned(model,signatureReply,&o,&cap)==XUI_OK&&o.refs==4);
    xui_code_completion_item_t items[2];
    int count=99;
    assert(xuiCodeProviderRequestCompletion(model,NULL,0,"pr",items,2,&count)==XUI_ERROR_INVALID_STATE);
    assert(count==0&&!items[0].sLabel&&o.calls==0);
    xui_code_provider_scope_t outer={0}, inner={0};
    assert(xuiCodeProviderSetCallBegin(model,&outer)==XUI_OK);
    assert(xuiCodeProviderRequestCompletion(model,NULL,0,"pr",items,2,&count)==XUI_OK);
    assert(count==1&&strcmp(items[0].sLabel,"print")==0&&strcmp(items[0].sInsertText,"print")==0);
    assert(strcmp(items[0].sSortText,"")==0&&!items[0].sDetail&&items[0].iUserData==31);
    const char* retainedLabel=items[0].sLabel;
    xui_code_hover_t result={0}, nested={0};
    xui_code_signature_help_t help={0};
    assert(xuiCodeProviderRequestHover(model,NULL,0,&result)==XUI_OK&&strcmp(result.sText,"local hover")==0);
    assert(result.sContentType&&result.sContentType[0]=='\0');
    assert(xuiCodeProviderSetCallBegin(model,&inner)==XUI_OK);
    assert(xuiCodeProviderRequestHover(model,NULL,0,&nested)==XUI_OK&&nested.sText!=result.sText);
    assert(xuiCodeProviderRequestSignature(model,NULL,0,&help)==XUI_OK);
    assert(strcmp(help.sLabel,"print(value)")==0&&help.iUserData==73);
    assert(help.iParameterCount==1&&strcmp(help.pParameters[0].sLabel,"value")==0&&help.pParameters[0].tLabelRange.iStart==6);
    assert(xuiCodeProviderSetCallEnd(&inner)==XUI_OK);
    assert(strcmp(retainedLabel,"print")==0&&strcmp(result.sText,"local hover")==0);

    assert(xrtMemDebugFailAfter(0));
    assert(xuiCodeProviderRequestCompletion(model,NULL,0,"pr",items,2,&count)==XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();xrtClearError();
    assert(count==0&&!items[0].sLabel&&strcmp(retainedLabel,"print")==0);
    o.callAction=4;
    assert(xuiCodeProviderRequestSignature(model,NULL,0,&help)==XUI_ERROR_CALLBACK_FAILED);
    assert(!help.sLabel&&help.iActiveParameter==-1&&xrtGetError());
    xrtClearError();
    assert(xuiCodeProviderRequestCompletion(model,NULL,0,"pr",items,2,&count)==XUI_OK);
    assert(count==1&&strcmp(items[0].sLabel,"print")==0);
    assert(xuiCodeProviderSetCallEnd(&outer)==XUI_OK);
    assert(xuiCodeProviderSetDestroyChecked(model)==XUI_OK&&o.refs==1&&o.drops==3);
    puts("Owned Provider replies: local strings, nested scopes, OOM rollback, diagnostic rollback");
}
static int install(Owner* owner,int role)
{
    xui_callback_owner_t cap=capability(owner);
    if(role==0)return xuiCodeProviderSetCompletionOwned(owner->model,complete,owner,&cap);
    if(role==1)return xuiCodeProviderSetHoverOwned(owner->model,hover,owner,&cap);
    if(role==2)return xuiCodeProviderSetSignatureOwned(owner->model,signature,owner,&cap);
    if(role==3)return xuiCodeProviderSetCommandOwned(owner->model,command,owner,&cap);
    return xuiCodeProviderSetCommandEnabledOwned(owner->model,enabled,owner,&cap);
}
static int invoke(Owner* owner,int role)
{
    if(role==0){xui_code_completion_item_t items[2];int n=77;
        int status=xuiCodeProviderRequestCompletion(owner->model,NULL,0,NULL,items,2,&n);
        assert(status==XUI_OK ? n==1&&!strcmp(items[0].sLabel,"owned"):n==0&&!items[0].sLabel);return status;}
    if(role==1){xui_code_hover_t value;
        int status=xuiCodeProviderRequestHover(owner->model,NULL,0,&value);
        assert(status==XUI_OK ? value.sText&&!strcmp(value.sText,"owned"):!value.sText);return status;}
    if(role==2){xui_code_signature_help_t value;
        int status=xuiCodeProviderRequestSignature(owner->model,NULL,0,&value);
        assert(status==XUI_OK ? value.sLabel&&!strcmp(value.sLabel,"owned")&&value.pParameters==&owner->parameter:!value.sLabel&&!value.pParameters);return status;}
    int value=77;int status=role==3 ? xuiCodeProviderExecuteCommand(owner->model,NULL,1,NULL,&value):xuiCodeProviderCanExecuteCommand(owner->model,NULL,1,&value);
    assert(value==(status==XUI_OK?1:0));return status;
}
static bool edge(xrtownershipref ref,void* raw){assert(ref.Data&&ref.Ops==&ops);++*(unsigned*)raw;return true;}
static void balanced(const xmemdebugsnapshot* a)
{
    xmemdebugsnapshot b;xrtMemDebugSnapshot(&b);
    assert(b.LiveCount==a->LiveCount&&b.LiveBytes==a->LiveBytes&&b.InvalidFreeCount==a->InvalidFreeCount&&b.DoubleFreeCount==a->DoubleFreeCount);
}
static void roles(void)
{
    for(int role=0;role<5;++role)for(int mode=0;mode<4;++mode){
        xui_code_provider_set model=NULL;assert(xuiCodeProviderSetCreate(&model)==XUI_OK);Owner owner;init(&owner,model);
        assert(install(&owner,role)==XUI_OK&&owner.refs==2&&owner.holds==1);
        if(role<3)assert(invoke(&owner,role)==XUI_ERROR_INVALID_STATE&&!owner.calls);
        xui_code_provider_scope_t caller={0};assert(xuiCodeProviderSetCallBegin(model,&caller)==XUI_OK);
        owner.callAction=mode==3?4:mode;
        int status=invoke(&owner,role);assert(status==(mode==2?XUI_ERROR_INVALID_STATE:mode==3?XUI_ERROR_CALLBACK_FAILED:XUI_OK));
        assert(owner.calls==1&&owner.refs==2&&!owner.drops);
        xrtClearError();status=xuiCodeProviderSetCallEnd(&caller);
        if(mode==2){assert(status==XUI_ERROR_INVALID_STATE&&owner.refs==1&&owner.drops==1);}
        else {assert(status==XUI_OK);xrtClearError();assert(xuiCodeProviderSetDestroyChecked(model)==XUI_OK&&owner.refs==1&&owner.drops==1);}
        xrtClearError();
    }
}
static void publication(void)
{
    xui_code_provider_set model=NULL;assert(xuiCodeProviderSetCreate(&model)==XUI_OK);Owner a,b;init(&a,model);init(&b,model);
    assert(install(&a,3)==XUI_OK);
    assert(xrtMemDebugFailAfter(0));assert(install(&b,3)==XUI_ERROR_OUT_OF_MEMORY);xrtMemDebugFailClear();xrtClearError();assert(!b.holds&&a.refs==2);
    b.holdFalse=true;assert(install(&b,3)==XUI_ERROR_CALLBACK_FAILED&&b.refs==1&&!b.drops);xrtClearError();b.holdFalse=false;
    b.holdError=true;assert(install(&b,3)==XUI_ERROR_CALLBACK_FAILED&&b.refs==1&&b.drops==1);xrtClearError();b.holdError=false;strcpy(b.label,"owned");
    a.dropFalse=true;assert(install(&b,3)==XUI_ERROR_CALLBACK_FAILED&&a.refs==1&&b.refs==2);xrtClearError();
    assert(invoke(&b,3)==XUI_OK&&b.calls==1);assert(xuiCodeProviderSetClearChecked(model)==XUI_OK&&b.refs==1);
    init(&b,model);b.holdAction=1;assert(install(&b,3)==XUI_OK&&b.refs==2);assert(xuiCodeProviderSetDestroyChecked(model)==XUI_OK&&b.refs==1);
    assert(xuiCodeProviderSetCreate(&model)==XUI_OK);init(&b,model);b.holdAction=2;
    assert(install(&b,3)==XUI_ERROR_INVALID_STATE&&b.refs==1&&b.drops==1);xrtClearError();
}
static void callerGeneration(void)
{
    xui_code_provider_set model=NULL;assert(xuiCodeProviderSetCreate(&model)==XUI_OK);Owner a,b;init(&a,model);init(&b,model);
    assert(install(&a,0)==XUI_OK);xui_code_provider_scope_t caller={0};assert(xuiCodeProviderSetCallBegin(model,&caller)==XUI_OK);
    uint64_t before=caller._iGeneration;assert(install(&b,0)==XUI_OK&&a.refs==2);
    a.caller=&caller;a.dropAction=3;
    assert(invoke(&b,0)==XUI_ERROR_INVALID_STATE&&!b.calls&&caller._iGeneration!=before);
    assert(xuiCodeProviderSetCallEnd(&caller)==XUI_OK);
    assert(xuiCodeProviderSetDestroyChecked(model)==XUI_OK&&a.refs==1&&b.refs==1);
}
static void diagnostics(void)
{
    xui_code_provider_set model=NULL;assert(xuiCodeProviderSetCreate(&model)==XUI_OK);Owner owner;init(&owner,model);
    assert(install(&owner,3)==XUI_OK);
    xrtSetErrorInfo(XERR_IO,"provider-owned-test",9,"prior diagnostic");const xerror* prior=xrtGetError();assert(prior);
    assert(invoke(&owner,3)==XUI_OK&&xrtGetError()==prior);
    owner.callAction=4;assert(invoke(&owner,3)==XUI_ERROR_CALLBACK_FAILED&&xrtGetError()==prior);
    xrtClearError();owner.dropError=true;
    assert(xuiCodeProviderSetClearChecked(model)==XUI_ERROR_CALLBACK_FAILED&&owner.refs==1&&owner.drops==1&&xrtGetError());
    xrtClearError();assert(xuiCodeProviderSetDestroyChecked(model)==XUI_OK);
}
static void graphAndHot(void)
{
    xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);xui_code_provider_set model=NULL;
    assert(xuiCodeProviderSetCreate(&model)==XUI_OK);xrtMemDebugSnapshot(&b);
    printf("Owned Provider model: alloc=%llu bytes=%llu scope=%zu\n",(unsigned long long)(b.AllocCount-a.AllocCount),(unsigned long long)(b.LiveBytes-a.LiveBytes),sizeof(xui_code_provider_scope_t));
    Owner owners[5];for(int role=0;role<5;++role){
        init(&owners[role],model);xrtMemDebugSnapshot(&a);assert(install(&owners[role],role)==XUI_OK);xrtMemDebugSnapshot(&b);
        assert(b.AllocCount==a.AllocCount+1&&b.LiveCount==a.LiveCount+1);
        if(!role)printf("Owned Provider slot: alloc=1 bytes=%llu\n",(unsigned long long)(b.LiveBytes-a.LiveBytes));
    }
    unsigned edges=0;xrtownershipscope frozen={0};assert(xrtOwnershipFreezeTryBegin(&frozen));
    assert(xuiCodeProviderSetTraceOwnership(model,edge,&edges)&&edges==5&&xrtOwnershipScopeEnd(&frozen));
    xui_code_provider_scope_t caller={0};assert(xuiCodeProviderSetCallBegin(model,&caller)==XUI_OK);
    edges=0;assert(!xuiCodeProviderSetTraceOwnership(model,edge,&edges)&&!edges);
    xrtMemDebugSnapshot(&a);assert(xrtMemDebugFailAfter(0));
    for(int i=0;i<1000;++i)for(int role=0;role<5;++role)assert(invoke(&owners[role],role)==XUI_OK);
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtMemDebugSnapshot(&b);
    assert(a.AllocCount==b.AllocCount&&a.FreeCount==b.FreeCount&&a.ReallocCount==b.ReallocCount);
    puts("Owned Provider hot: 5000 requests,0 heap and no per-call owner Hold");
    for(int role=0;role<5;++role)assert(owners[role].holds==1&&!owners[role].drops&&owners[role].calls==1000);
    assert(xuiCodeProviderSetCallEnd(&caller)==XUI_OK&&xuiCodeProviderSetClearChecked(model)==XUI_OK);
    for(int role=0;role<5;++role)assert(owners[role].refs==1&&owners[role].drops==1);
    edges=0;assert(xuiCodeProviderSetTraceOwnership(model,edge,&edges)&&!edges);
    assert(xuiCodeProviderSetDestroyChecked(model)==XUI_OK);
}
static size_t backendRefs=1;
static bool backendHold(const void* p){assert(p==&backendRefs);++backendRefs;return true;}
static bool backendDrop(const void* p){assert(p==&backendRefs&&backendRefs>1);--backendRefs;return true;}
static bool backendCount(const void* p,size_t* n){assert(p==&backendRefs);*n=backendRefs;return true;}
static const xrtownershipops backendOps={backendCount,trace};
static void consumers(void)
{
    xui_test_proxy_state_t backend;xuiTestProxyInit(&backend);
    for(int role=0;role<3;role+=2){
        xui_proxy proxy=NULL;xui_context context=NULL;xui_widget editor=NULL;
        const xui_callback_owner_t cap={{&backendRefs,&backendOps},backendHold,backendDrop};
        const xui_context_desc_t desc={sizeof(desc),XUI_SYSTEM_FONT_DISABLED,0,0};
        assert(xuiProxyCreateOwned(&backend.tProxy,&cap,&proxy)==XUI_OK&&xuiCreateOwnedEx(proxy,&desc,&context)==XUI_OK);
        assert(xuiCodeEditCreate(context,&editor,NULL)==XUI_OK);Owner owner;init(&owner,xuiCodeEditGetProviders(editor));
        assert(install(&owner,role)==XUI_OK);owner.callAction=1;
        int status=role==0?xuiCodeEditShowCompletion(editor):xuiCodeEditRequestSignatureHelp(editor);
        assert(status==XUI_OK&&owner.refs==1&&owner.drops==1&&!strcmp(owner.label,"expired"));
        if(role==0){
            assert(xuiCodeEditGetCompletionCount(editor)==1&&xuiCodeEditIsCompletionOpen(editor));
            xui_widget list=xuiWidgetGetFirstChild(xuiPopupGetContentWidget(xuiCodeEditGetCompletionPopupWidget(editor)));
            assert(list&&!strcmp(xuiListViewGetItemText(list,0),"owned"));
        }
        else assert(xuiCodeEditIsSignatureHelpOpen(editor));
        assert(xuiDestroyChecked(context)==XUI_OK&&xuiProxyReleaseChecked(proxy)==XUI_OK&&backendRefs==1);
    }
}
static void consumerAllocationPrefixes(void)
{
    xui_test_proxy_state_t backend;xuiTestProxyInit(&backend);
    unsigned total=0;
    for(int role=0;role<3;role+=2){bool success=false;
        for(int budget=0;budget<160;++budget){
            xmemdebugsnapshot initial;xrtMemDebugSnapshot(&initial);
            xui_proxy proxy=NULL;xui_context context=NULL;xui_widget editor=NULL;
            const xui_callback_owner_t cap={{&backendRefs,&backendOps},backendHold,backendDrop};
            const xui_context_desc_t desc={sizeof(desc),XUI_SYSTEM_FONT_DISABLED,0,0};
            assert(xuiProxyCreateOwned(&backend.tProxy,&cap,&proxy)==XUI_OK&&xuiCreateOwnedEx(proxy,&desc,&context)==XUI_OK);
            assert(xuiCodeEditCreate(context,&editor,NULL)==XUI_OK);Owner owner;init(&owner,xuiCodeEditGetProviders(editor));
            assert(install(&owner,role)==XUI_OK);owner.callAction=1;assert(xrtMemDebugFailAfter(budget));
            int status=role==0?xuiCodeEditShowCompletion(editor):xuiCodeEditRequestSignatureHelp(editor);
            bool failed=xrtMemDebugFailTriggered();bool memoryError=xrtErrorKind(xrtGetError())==XERR_MEMORY;
            xrtMemDebugFailClear();xrtClearError();++total;
            if(status!=XUI_OK&&status!=XUI_ERROR_OUT_OF_MEMORY&&!(status==XUI_ERROR_CALLBACK_FAILED&&memoryError))
                fprintf(stderr,"consumer role=%d budget=%d status=%d failed=%d memory=%d calls=%u\n",role,budget,status,failed,memoryError,owner.calls);
            assert(status==XUI_OK||status==XUI_ERROR_OUT_OF_MEMORY||(status==XUI_ERROR_CALLBACK_FAILED&&memoryError));
            assert(xuiDestroyChecked(context)==XUI_OK&&xuiProxyReleaseChecked(proxy)==XUI_OK&&backendRefs==1);
            assert(owner.refs==1&&owner.holds==1&&owner.drops==1);balanced(&initial);
            if(!failed){assert(status==XUI_OK);success=true;break;}
        }
        assert(success);
    }
    printf("Owned Provider consumer allocation prefixes=%u, complete heaps balanced\n",total);
}
typedef struct EditorCommand {
    Owner owner;
    int calls;
    bool enabled;
    bool reenter;
    bool close;
} EditorCommand;

static int executeEditorCommand(xui_widget widget,int command,int* handled,void* user)
{
    EditorCommand* state=user;
    assert(widget&&state->owner.refs>1);
    if(command!=XUI_CODE_COMMAND_USER_BASE)return XUI_ERROR_UNSUPPORTED;
    ++state->calls;
    *handled=1;
    return XUI_OK;
}

static int enableEditorCommand(xui_widget widget,int command,int* enabled,void* user)
{
    EditorCommand* state=user;
    assert(widget&&state->owner.refs>1);
    if(state->close){
        assert(xuiWidgetDestroyChecked(widget)==XUI_OK);
        return XUI_OK;
    }
    if(command!=XUI_CODE_COMMAND_USER_BASE)return XUI_ERROR_UNSUPPORTED;
    if(state->reenter){
        bool decided=false,allowed=false,handled=false;
        assert(xuiCodeEditQueryCommandEnabled(widget,command,&decided,&allowed)==XUI_ERROR_BUSY);
        assert(xuiCodeEditExecuteCommand(widget,command,&handled)==XUI_ERROR_BUSY);
        assert(!decided&&!allowed&&!handled);
    }
    *enabled=state->enabled;
    return XUI_OK;
}

static void commandMapInvalidated(void* user)
{
    ++*(int*)user;
}

static void editorCommands(void)
{
    xui_test_proxy_state_t backend;xuiTestProxyInit(&backend);
    const xui_callback_owner_t backendOwner={{&backendRefs,&backendOps},backendHold,backendDrop};
    const xui_context_desc_t desc={sizeof(desc),XUI_SYSTEM_FONT_DISABLED,0,0};
    xui_proxy proxy=NULL;xui_context context=NULL;xui_widget editor=NULL;
    assert(xuiProxyCreateOwned(&backend.tProxy,&backendOwner,&proxy)==XUI_OK);
    assert(xuiCreateOwnedEx(proxy,&desc,&context)==XUI_OK);
    assert(xuiCodeEditCreate(context,&editor,NULL)==XUI_OK&&xuiSetRootWidget(context,editor)==XUI_OK);
    const int id=XUI_CODE_COMMAND_USER_BASE;
    bool decided=true,allowed=true,handled=true;
    assert(xuiCodeEditQueryCommandEnabled(editor,id,&decided,&allowed)==XUI_OK&&!decided&&!allowed);
    assert(xuiCodeEditExecuteCommand(editor,id,&handled)==XUI_OK&&!handled);
    EditorCommand state={0};init(&state.owner,xuiCodeEditGetProviders(editor));
    const xui_callback_owner_t cap=capability(&state.owner);
    assert(xuiCodeEditSetCommandProviderOwned(editor,executeEditorCommand,&state,&cap)==XUI_OK);
    assert(xuiCodeEditSetCommandEnabledProviderOwned(editor,enableEditorCommand,&state,&cap)==XUI_OK);
    assert(state.owner.refs==3);

    xui_code_command_map map=xuiCodeEditGetCommandMap(editor);
    xui_lifetime_link_t link={0};int invalidated=0;
    assert(xuiCodeCommandMapEditorLifetimeWatch(map,&link,commandMapInvalidated,&invalidated)==XUI_OK);
    assert(xuiCodeCommandMapDestroyChecked(map)==XUI_ERROR_INVALID_STATE);
    assert(xuiCodeCommandMapBind(map,XUI_KEY_F2,XUI_MOD_CTRL,id)==XUI_OK);
    xui_event_t down={.iSize=sizeof(down),.iType=XUI_EVENT_KEY_DOWN,.pTarget=editor,
        .iKey=XUI_KEY_F2,.iModifiers=XUI_MOD_CTRL};
    assert(xuiCodeEditQueryCommandEnabled(editor,id,&decided,&allowed)==XUI_OK&&decided&&!allowed);
    assert(xuiCodeEditExecuteCommand(editor,id,&handled)==XUI_OK&&!handled);
    assert(xuiDispatchEvent(context,&down)==XUI_OK&&state.calls==0);

    state.enabled=true;
    state.reenter=true;
    assert(xuiCodeEditQueryCommandEnabled(editor,id,&decided,&allowed)==XUI_OK&&decided&&allowed);
    assert(xuiDispatchEvent(context,&down)==XUI_OK&&state.calls==1);
    assert(xuiCodeEditExecuteCommand(editor,id,&handled)==XUI_OK&&handled&&state.calls==2);
    assert(xuiCodeEditExecuteCommand(editor,XUI_CODE_COMMAND_SELECT_ALL,&handled)==XUI_OK&&handled);

    assert(xrtMemDebugFailAfter(0));
    assert(xuiCodeCommandMapLoadDefaults(map)==XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();xrtClearError();
    int command=0;
    assert(xuiCodeCommandMapFind(map,XUI_KEY_F2,XUI_MOD_CTRL,&command)==XUI_OK&&command==id);

    state.close=true;
    assert(xuiCodeEditOpenMenu(editor,20,20)==XUI_ERROR_INVALID_STATE);
    assert(invalidated==1&&state.owner.refs==1&&state.owner.drops==2);
    xrtClearError();
    assert(xuiDestroyChecked(context)==XUI_OK&&xuiProxyReleaseChecked(proxy)==XUI_OK&&backendRefs==1);
    puts("Owned Editor commands: guarded dispatch, default fallback, reentry, observed key map and menu close");
}
int main(void)
{
    marginBind();xmemdebugsnapshot initial;xrtMemDebugSnapshot(&initial);
    roles();balanced(&initial);publication();balanced(&initial);callerGeneration();balanced(&initial);diagnostics();balanced(&initial);
    graphAndHot();balanced(&initial);consumers();balanced(&initial);consumerAllocationPrefixes();balanced(&initial);
    replyStorage();balanced(&initial);
    editorCommands();balanced(&initial);
    puts("Owned Provider: all5 roles, immutable pins, scoped borrows, generation, graph, reentry, errors and consumers passed");return 0;
}
