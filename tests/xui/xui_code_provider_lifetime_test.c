/* Native prerequisite only: no language closure or owning output publication. */
#include "xui_code_margin_host.h"
#include "tests/xui/xui_test_proxy.c"
#include <stdio.h>

typedef struct Watch {xui_code_provider_set providers;unsigned calls;bool fail;} Watch;
static void invalidated(void* raw)
{
    Watch* watch=raw;++watch->calls;bool consumed=true;
    assert(xuiCodeProviderSetDestroyOwnerChecked(watch->providers,&consumed)==XUI_ERROR_BUSY&&!consumed);
    assert(xuiCodeProviderSetClearChecked(watch->providers)==XUI_ERROR_INVALID_STATE);
    xui_code_provider_scope_t scope={0};assert(xuiCodeProviderSetCallBegin(watch->providers,&scope)==XUI_ERROR_INVALID_STATE);
    if(watch->fail)xrtSetErrorKind(XERR_STATE);
}
static bool visit(xrtownershipref edge,void* raw){(void)edge;++*(int*)raw;return true;}
static void balanced(const xmemdebugsnapshot* before)
{
    xmemdebugsnapshot after;xrtMemDebugSnapshot(&after);
    assert(after.LiveCount==before->LiveCount&&after.LiveBytes==before->LiveBytes&&after.InvalidFreeCount==before->InvalidFreeCount&&after.DoubleFreeCount==before->DoubleFreeCount);
}
static void standalone(void)
{
    xui_code_provider_set providers=NULL;assert(xuiCodeProviderSetCreate(&providers)==XUI_OK);
    Watch watch={providers,0,false};xui_lifetime_link_t link={0};
    assert(xuiCodeProviderSetLifetimeWatch(providers,&link,invalidated,&watch)==XUI_OK);
    assert(xuiCodeProviderSetLifetimeWatch(providers,&link,invalidated,&watch)==XUI_ERROR_INVALID_STATE);
    xui_code_provider_scope_t outer={0},inner={0};
    assert(xuiCodeProviderSetCallBegin(providers,&outer)==XUI_OK&&xuiCodeProviderSetCallBegin(providers,&inner)==XUI_OK);
    assert(xuiCodeProviderSetCallBegin(providers,&outer)==XUI_ERROR_INVALID_ARGUMENT);
    xui_code_provider_scope_t moved=inner;assert(xuiCodeProviderSetCallEnd(&moved)==XUI_ERROR_INVALID_ARGUMENT);
    int edges=0;assert(xuiCodeProviderSetOwnershipBusy(providers)&&!xuiCodeProviderSetTraceOwnership(providers,visit,&edges)&&edges==0);
    bool consumed=false;assert(xuiCodeProviderSetDestroyOwnerChecked(providers,&consumed)==XUI_OK&&consumed&&!watch.calls);
    consumed=true;assert(xuiCodeProviderSetDestroyOwnerChecked(providers,&consumed)==XUI_ERROR_INVALID_STATE&&!consumed);
    assert(xuiCodeProviderSetClearChecked(providers)==XUI_ERROR_INVALID_STATE);
    assert(xuiCodeProviderSetCallEnd(&inner)==XUI_ERROR_INVALID_STATE&&!watch.calls);
    assert(xuiCodeProviderSetCallEnd(&outer)==XUI_ERROR_INVALID_STATE&&watch.calls==1&&!link.ppPrevious);
    assert(xuiCodeProviderSetCallEnd(&outer)==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiCodeProviderSetCreate(&providers)==XUI_OK);watch=(Watch){providers,0,true};link=(xui_lifetime_link_t){0};
    assert(xuiCodeProviderSetLifetimeWatch(providers,&link,invalidated,&watch)==XUI_OK);
    consumed=false;assert(xuiCodeProviderSetDestroyOwnerChecked(providers,&consumed)==XUI_ERROR_CALLBACK_FAILED&&consumed&&watch.calls==1);
    xrtClearError();
    providers=(xui_code_provider_set)(uintptr_t)1;assert(xrtMemDebugFailAfter(0));
    assert(xuiCodeProviderSetCreate(&providers)==XUI_ERROR_OUT_OF_MEMORY&&!providers);
    xrtMemDebugFailClear();xrtClearError();
}
typedef struct Call {xui_code_provider_set providers;int action,calls;uint64_t thread;const xerror* ambient;} Call;
static void action(Call* call)
{
    ++call->calls;call->thread=xrtThreadCurrentId();assert(xuiCodeProviderSetOwnershipBusy(call->providers));
    if(call->ambient)assert(xrtGetError()==call->ambient);
    if(call->action==1)assert(xuiCodeProviderSetClearChecked(call->providers)==XUI_OK);
    if(call->action==2){bool consumed=false;assert(xuiCodeProviderSetDestroyOwnerChecked(call->providers,&consumed)==XUI_OK&&consumed);}
    if(call->action==3)xrtSetErrorKind(XERR_IO);
}
static int complete(xui_widget widget,int offset,const char* prefix,xui_code_completion_item_t* items,int capacity,int* count,void* raw)
{
    (void)widget;assert(offset==0&&prefix&&!strcmp(prefix,"")&&capacity==2&&items[0].iSize==sizeof(items[0]));
    action(raw);items[0].sLabel="item";*count=1;return XUI_OK;
}
static int hover(xui_widget widget,int offset,xui_code_hover_t* value,void* raw)
{(void)widget;assert(offset==0);action(raw);value->sText="hover";return XUI_OK;}
static int signature(xui_widget widget,int offset,xui_code_signature_help_t* value,void* raw)
{(void)widget;assert(offset==0);action(raw);value->sLabel="fn()";return XUI_OK;}
static int command(xui_widget widget,int id,const void* data,int* handled,void* raw)
{(void)widget;assert(id==1&&!data);action(raw);*handled=7;return XUI_OK;}
static int enabled(xui_widget widget,int id,int* value,void* raw)
{(void)widget;assert(id==1);action(raw);*value=7;return XUI_OK;}
static void install(Call* call)
{
    assert(xuiCodeProviderSetCompletion(call->providers,complete,call)==XUI_OK);
    assert(xuiCodeProviderSetHover(call->providers,hover,call)==XUI_OK);
    assert(xuiCodeProviderSetSignature(call->providers,signature,call)==XUI_OK);
    assert(xuiCodeProviderSetCommand(call->providers,command,call)==XUI_OK);
    assert(xuiCodeProviderSetCommandEnabled(call->providers,enabled,call)==XUI_OK);
}
static void invoke(Call* call,int role,int expected)
{
    int status;
    if(role==0){
        xui_code_completion_item_t items[2];memset(items,0xa5,sizeof(items));int count=77;
        status=xuiCodeProviderRequestCompletion(call->providers,NULL,0,NULL,items,2,&count);
        assert(count==(expected==XUI_OK?1:0)&&items[0].iSize==sizeof(items[0]));
        assert(expected==XUI_OK?items[0].sLabel&&!strcmp(items[0].sLabel,"item"):items[0].sLabel==NULL);
    }else if(role==1){
        xui_code_hover_t value;memset(&value,0xa5,sizeof(value));
        status=xuiCodeProviderRequestHover(call->providers,NULL,0,&value);
        assert(value.iSize==sizeof(value)&&(expected==XUI_OK?value.sText&&!strcmp(value.sText,"hover"):!value.sText));
    }else if(role==2){
        xui_code_signature_help_t value;memset(&value,0xa5,sizeof(value));
        status=xuiCodeProviderRequestSignature(call->providers,NULL,0,&value);
        assert(value.iSize==sizeof(value)&&value.iActiveParameter==-1&&(expected==XUI_OK?value.sLabel&&!strcmp(value.sLabel,"fn()"):!value.sLabel));
    }else {
        int value=77;status=role==3?xuiCodeProviderExecuteCommand(call->providers,NULL,1,NULL,&value):xuiCodeProviderCanExecuteCommand(call->providers,NULL,1,&value);
        assert(value==(expected==XUI_OK?1:0));
    }
    assert(status==expected);
}
static void requests(void)
{
    for(int role=0;role<5;++role)for(int mode=0;mode<4;++mode){
        Call call={0};assert(xuiCodeProviderSetCreate(&call.providers)==XUI_OK);call.action=mode;install(&call);
        Watch watch={call.providers,0,false};xui_lifetime_link_t link={0};
        assert(xuiCodeProviderSetLifetimeWatch(call.providers,&link,invalidated,&watch)==XUI_OK);
        invoke(&call,role,mode==2?XUI_ERROR_INVALID_STATE:mode==3?XUI_ERROR_CALLBACK_FAILED:XUI_OK);
        assert(call.calls==1);
        if(mode==2)assert(watch.calls==1);
        else {if(mode==1)invoke(&call,role,XUI_ERROR_UNSUPPORTED);xrtClearError();assert(xuiCodeProviderSetDestroyChecked(call.providers)==XUI_OK&&watch.calls==1);}
        xrtClearError();
    }
    Call call={0};assert(xuiCodeProviderSetCreate(&call.providers)==XUI_OK);install(&call);
    xrtSetErrorInfo(XERR_IO,"provider-test",7,"prior business diagnostic");call.ambient=xrtGetError();assert(call.ambient);
    invoke(&call,3,XUI_OK);assert(xrtGetError()==call.ambient);
    call.action=3;invoke(&call,3,XUI_ERROR_CALLBACK_FAILED);assert(xrtGetError()==call.ambient);
    xrtClearError();assert(xuiCodeProviderSetDestroyChecked(call.providers)==XUI_OK);
}
typedef struct Worker {Call* call;xui_code_provider_scope_t* active;} Worker;
static int32 transfer(void* raw)
{
    Worker* worker=raw;
    if(worker->active){
        bool consumed=true;xui_code_provider_scope_t scope={0};
        assert(xuiCodeProviderSetCallBegin(worker->call->providers,&scope)==XUI_ERROR_BUSY&&!scope.pSelf);
        assert(xuiCodeProviderSetCallEnd(worker->active)==XUI_ERROR_BUSY&&worker->active->pSelf==worker->active);
        assert(xuiCodeProviderSetClearChecked(worker->call->providers)==XUI_ERROR_BUSY);
        assert(xuiCodeProviderSetDestroyOwnerChecked(worker->call->providers,&consumed)==XUI_ERROR_BUSY&&!consumed);
    }else {install(worker->call);invoke(worker->call,3,XUI_OK);assert(worker->call->thread==xrtThreadCurrentId());}
    return 0;
}
static void threads(void)
{
    Call call={0};assert(xuiCodeProviderSetCreate(&call.providers)==XUI_OK);Worker data={&call,NULL};
    xthread* worker=xrtThreadCreate(transfer,&data,0);assert(worker&&xrtThreadWait(worker)==XWAIT_OK);xrtThreadDestroy(worker);
    assert(call.calls==1&&call.thread!=xrtThreadCurrentId());
    xui_code_provider_scope_t scope={0};assert(xuiCodeProviderSetCallBegin(call.providers,&scope)==XUI_OK);data.active=&scope;
    worker=xrtThreadCreate(transfer,&data,0);assert(worker&&xrtThreadWait(worker)==XWAIT_OK);xrtThreadDestroy(worker);
    assert(xuiCodeProviderSetCallEnd(&scope)==XUI_OK);
    xrtownershipscope frozen={0};assert(xrtOwnershipFreezeTryBegin(&frozen));int edges=0;
    assert(xuiCodeProviderSetTraceOwnership(call.providers,visit,&edges)&&edges==0);
    assert(xuiCodeProviderSetCallBegin(call.providers,&scope)==XUI_OK&&xuiCodeProviderSetOwnershipBusy(call.providers));
    assert(xuiCodeProviderSetCallEnd(&scope)==XUI_OK&&xrtOwnershipScopeEnd(&frozen));
    assert(xuiCodeProviderSetDestroyChecked(call.providers)==XUI_OK);
}
static size_t refs=1;
static bool hold(const void* raw){assert(raw==&refs);++refs;return true;}
static bool drop(const void* raw){assert(raw==&refs&&refs>1);--refs;return true;}
static bool count(const void* raw,size_t* n){assert(raw==&refs);*n=refs;return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static xui_test_proxy_state_t backend;
static void embedded(int closeContext)
{
    const xui_callback_owner_t owner={{&refs,&ops},hold,drop};xui_proxy proxy=NULL;xui_context context=NULL;
    const xui_context_desc_t desc={sizeof(desc),XUI_SYSTEM_FONT_DISABLED,0,0};xui_widget editor=NULL;
    assert(xuiProxyCreateOwned(&backend.tProxy,&owner,&proxy)==XUI_OK&&xuiCreateOwnedEx(proxy,&desc,&context)==XUI_OK);
    assert(xuiCodeEditCreate(context,&editor,NULL)==XUI_OK);xui_code_provider_set providers=xuiCodeEditGetProviders(editor);assert(providers);
    bool consumed=true;assert(xuiCodeProviderSetDestroyOwnerChecked(providers,&consumed)==XUI_ERROR_INVALID_STATE&&!consumed);
    xuiCodeProviderSetDestroy(providers);assert(xuiCodeEditGetProviders(editor)==providers);
    Watch watch={providers,0,false};xui_lifetime_link_t link={0};
    assert(xuiCodeProviderSetLifetimeWatch(providers,&link,invalidated,&watch)==XUI_OK);
    xui_code_provider_scope_t scope={0};assert(xuiCodeProviderSetCallBegin(providers,&scope)==XUI_OK&&xuiContextOwnershipBusy(context));
    int status=closeContext?xuiDestroyChecked(context):xuiWidgetDestroyChecked(editor);
    assert(status==(closeContext?XUI_STATUS_DEFERRED:XUI_OK)&&!watch.calls);
    status=xuiCodeProviderSetCallEnd(&scope);
    if(closeContext){
        assert(status==XUI_OK&&!watch.calls);
        int edges=0;assert(xuiCodeProviderSetTraceOwnership(providers,visit,&edges)&&edges==0);
        assert(xuiCodeProviderSetCallBegin(providers,&scope)==XUI_ERROR_INVALID_STATE&&!scope.pSelf);
        assert(xuiDestroyChecked(context)==XUI_OK&&watch.calls==1);
    }
    else {assert(status==XUI_ERROR_INVALID_STATE&&watch.calls==1);xrtClearError();assert(xuiDestroyChecked(context)==XUI_OK);}
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK&&refs==1);
}
typedef struct Consumer {xui_context context;xui_widget editor;int closeContext,calls;} Consumer;
static void consumerClose(Consumer* call)
{
    ++call->calls;
    assert(xuiContextOwnershipBusy(call->context));
    int status=call->closeContext ? xuiDestroyChecked(call->context):xuiWidgetDestroyChecked(call->editor);
    assert(status==(call->closeContext ? XUI_STATUS_DEFERRED:XUI_OK));
}
static int consumerComplete(xui_widget editor,int offset,const char* prefix,xui_code_completion_item_t* items,int capacity,int* count,void* raw)
{
    Consumer* call=raw;assert(editor==call->editor&&offset==0&&prefix&&capacity>0);
    consumerClose(call);items[0].sLabel="completion";*count=1;return XUI_OK;
}
static int consumerSignature(xui_widget editor,int offset,xui_code_signature_help_t* help,void* raw)
{
    Consumer* call=raw;assert(editor==call->editor&&offset==0);
    consumerClose(call);help->sLabel="function()";return XUI_OK;
}
static int consumerEnabled(xui_widget editor,int command,int* enabled,void* raw)
{
    Consumer* call=raw;assert(editor==call->editor&&command==XUI_CODE_COMMAND_GOTO_LINE);
    consumerClose(call);*enabled=1;return XUI_OK;
}
static void consumers(void)
{
    for(int role=0;role<3;++role)for(int closeContext=0;closeContext<2;++closeContext){
        const xui_callback_owner_t owner={{&refs,&ops},hold,drop};xui_proxy proxy=NULL;
        const xui_context_desc_t desc={sizeof(desc),XUI_SYSTEM_FONT_DISABLED,0,0};
        Consumer call={0};call.closeContext=closeContext;
        assert(xuiProxyCreateOwned(&backend.tProxy,&owner,&proxy)==XUI_OK&&xuiCreateOwnedEx(proxy,&desc,&call.context)==XUI_OK);
        assert(xuiCodeEditCreate(call.context,&call.editor,NULL)==XUI_OK);
        xui_code_provider_set providers=xuiCodeEditGetProviders(call.editor);
        Watch watch={providers,0,false};xui_lifetime_link_t link={0};
        assert(xuiCodeProviderSetLifetimeWatch(providers,&link,invalidated,&watch)==XUI_OK);
        int status;
        if(role==0){assert(xuiCodeProviderSetCompletion(providers,consumerComplete,&call)==XUI_OK);status=xuiCodeEditShowCompletion(call.editor);}
        else if(role==1){assert(xuiCodeProviderSetSignature(providers,consumerSignature,&call)==XUI_OK);status=xuiCodeEditRequestSignatureHelp(call.editor);}
        else {assert(xuiCodeProviderSetCommandEnabled(providers,consumerEnabled,&call)==XUI_OK);status=xuiCodeEditSetMenuTitle(call.editor,XUI_CODE_COMMAND_GOTO_LINE,"Go");}
        if(call.calls!=1 || status!=XUI_ERROR_INVALID_STATE)
            fprintf(stderr,"consumer role=%d contextClose=%d calls=%d status=%d watches=%u\n",role,closeContext,call.calls,status,watch.calls);
        assert(call.calls==1&&status==XUI_ERROR_INVALID_STATE);
        if(closeContext)assert(!watch.calls);
        else assert(watch.calls==1);
        xrtClearError();assert(xuiDestroyChecked(call.context)==XUI_OK&&watch.calls==1);
        assert(xuiProxyReleaseChecked(proxy)==XUI_OK&&refs==1);
    }
    puts("Provider consumers: completion, signature and menu close tails passed");
}
static void hot(void)
{
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);Call call={0};
    assert(xuiCodeProviderSetCreate(&call.providers)==XUI_OK);xrtMemDebugSnapshot(&after);
    printf("Provider model:alloc=%llu bytes=%llu; scope=%zu\n",(unsigned long long)(after.AllocCount-before.AllocCount),(unsigned long long)(after.LiveBytes-before.LiveBytes),sizeof(xui_code_provider_scope_t));
    install(&call);invoke(&call,3,XUI_OK);xrtMemDebugSnapshot(&before);assert(xrtMemDebugFailAfter(0));
    for(int i=0;i<1000;++i){xui_code_provider_scope_t scope={0};
        assert(xuiCodeProviderSetCallBegin(call.providers,&scope)==XUI_OK);invoke(&call,3,XUI_OK);assert(xuiCodeProviderSetCallEnd(&scope)==XUI_OK);
    }
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtMemDebugSnapshot(&after);
    assert(after.AllocCount==before.AllocCount&&after.FreeCount==before.FreeCount&&after.ReallocCount==before.ReallocCount);
    assert(xuiCodeProviderSetDestroyChecked(call.providers)==XUI_OK);
    puts("Provider hot:1000 scopes/requests,0 heap");
}
int main(void)
{
    marginBind();xuiTestProxyInit(&backend);threads();xrtClearError();
    xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
    standalone();balanced(&before);requests();balanced(&before);threads();balanced(&before);
    embedded(0);balanced(&before);embedded(1);balanced(&before);consumers();balanced(&before);hot();balanced(&before);
    puts("Provider lifetime: unique owner, watches, all5 callback reentry, deferred close, ambient errors, thread transfer, freeze and cleanup passed");return 0;
}
