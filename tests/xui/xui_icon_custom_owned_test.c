/* Immutable callback owners, transactional publication and physical tails.
 * The explicitly legacy CPU renderer is used only for native draw callbacks. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct custom_state custom_state;
typedef struct custom_owner {custom_state* state;unsigned refs,slot;int holdMode,dropMode;} custom_owner;
typedef struct custom_user {custom_state* state;unsigned slot;} custom_user;
struct custom_state {
    xui_test_proxy_state_t backend;
    xui_context context;xui_icon_category category;xui_icon icon;
    xui_widget stateWidget;
    xui_lifetime_link_t contextLink,iconLink;
    custom_owner owners[4];custom_user users[4];
    unsigned calls[4],gone,iconGone,entered;
    int operation,action,failure;bool shared;
    xerror* expected;
};
static void contextGone(void* raw){++((custom_state*)raw)->gone;}
static void iconGone(void* raw){++((custom_state*)raw)->iconGone;}
static bool hold(const void* raw)
{
    custom_owner* owner=(custom_owner*)raw;custom_state* s=owner->state;
    if(owner->holdMode==1)return false;
    ++owner->refs;
    if(owner->holdMode==2){xrtSetErrorKind(XERR_TYPE);s->expected=xrtErrorRef(xrtGetError());assert(s->expected);}
    else if(owner->holdMode==3)xuiDestroy(s->context);
    else if(owner->holdMode==4)assert(xuiIconCategoryRemove(s->context,"custom")==XUI_OK);
    else if(owner->holdMode>=5)assert(xuiWidgetSetEnabled(s->stateWidget,0)==XUI_OK);
    return true;
}
static bool drop(const void* raw)
{
    custom_owner* owner=(custom_owner*)raw;assert(owner->refs>1);--owner->refs;
    if(owner->dropMode==2)xrtSetErrorKind(XERR_RANGE);
    return !owner->dropMode;
}
static bool count(const void* raw,size_t* output){*output=((const custom_owner*)raw)->refs;return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user)
{(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops operations={count,trace};
static int enter(xui_icon icon,void* raw,unsigned slot)
{
    custom_user* user=raw;custom_state* s=user->state;assert(user->slot==slot);
    if(s->shared)assert(s->owners[0].refs==5);
    else for(unsigned i=0;i<4;++i)assert(s->owners[i].refs>=2);
    ++s->calls[slot];assert(!strcmp(xuiIconGetName(icon),"icon"));
    if((s->operation==0 && slot==1) || (s->operation==1 && slot==2) || (s->operation==2 && slot==0)){
        if(!s->entered++){
            if(s->action==1)assert(xuiIconRemoveById(s->category,xuiIconGetId(icon))==XUI_OK);
            else if(s->action==2)xuiDestroy(s->context);
        }
        assert(!s->gone && !s->iconGone);
        if(s->failure==2){xrtSetErrorKind(XERR_TYPE);s->expected=xrtErrorRef(xrtGetError());assert(s->expected);}
        if(s->failure)return XUI_ERROR_BACKEND_FAILED;
    }
    return XUI_OK;
}
static int draw(xui_icon icon,xui_painter painter,xui_rect_t rect,const xui_icon_draw_desc_t* style,void* raw)
{(void)style;int status=enter(icon,raw,0);return status==XUI_OK?xuiPainterFillRect(painter,rect,XUI_COLOR_WHITE):status;}
static int measure(xui_icon icon,xui_vec2_t* size,void* raw)
{*size=(xui_vec2_t){16,16};return enter(icon,raw,1);}
static int prepare(xui_icon icon,int width,int height,void* raw)
{assert(width>0 && height>0);return enter(icon,raw,2);}
static void destroy(xui_icon icon,void* raw)
{assert(enter(icon,raw,3)==XUI_OK);}
static void setup(custom_state* s)
{
    s->operation=-1;
    assert(xuiCreate(&s->context)==XUI_OK && xuiIconCategoryCreate(s->context,"custom",NULL,&s->category)==XUI_OK);
    assert(xuiContextLifetimeWatch(s->context,&s->contextLink,contextGone,s)==XUI_OK);
    for(unsigned i=0;i<4;++i){s->owners[i]=(custom_owner){s,1,i,0,0};s->users[i]=(custom_user){s,i};}
}
static int create(custom_state* s)
{
    xui_callback_owner_t owners[4];
    for(unsigned i=0;i<4;++i)owners[i]=(xui_callback_owner_t){{&s->owners[s->shared?0:i],&operations},hold,drop};
    return xuiIconAddCustomOwned(s->category,"icon",draw,&s->users[0],&owners[0],
        measure,&s->users[1],&owners[1],prepare,&s->users[2],&owners[2],destroy,&s->users[3],&owners[3],NULL,&s->icon);
}
static void finish(custom_state* s)
{
    for(unsigned i=0;i<4;++i)s->owners[i].dropMode=0;
    if(!s->gone)xuiDestroy(s->context);
    assert(s->gone==1);
    for(unsigned i=0;i<4;++i)assert(s->owners[i].refs==1);
    xrtClearError();xrtErrorFree(s->expected);
}
static bool visit(xrtownershipref ref,void* raw)
{if(ref.Ops==&operations){assert(ref.Data);++*(unsigned*)raw;}return true;}
static int queued(xui_widget widget,const xui_event_t* event,void* raw)
{
    (void)widget;(void)event;custom_state* s=raw;
    xui_icon icon=xuiIconFind(s->category,"icon");assert(icon);
    if(s->owners[0].holdMode==5)assert(xuiIconRemoveById(s->category,xuiIconGetId(icon))==XUI_OK);
    else xuiDestroy(s->context);
    return XUI_OK;
}
static void check(int operation,int action,int failure,int ambientPresent,int alias)
{
    custom_state s={0};setup(&s);assert(create(&s)==XUI_OK);
    assert(xuiIconLifetimeWatch(s.icon,&s.iconLink,iconGone,&s)==XUI_OK);
    xrtownershipscope frozen={0};unsigned edges=0;assert(xrtOwnershipFreezeTryBegin(&frozen));
    assert(xuiContextTraceOwnership(s.context,visit,&edges) && edges==4);assert(xrtOwnershipScopeEnd(&frozen));
    xui_icon root=s.icon;if(alias)assert(xuiIconAddAlias(s.category,"alias",s.icon,NULL,&root)==XUI_OK);
    xui_surface target=NULL;xui_painter painter=NULL;xui_call_scope_t drawing={0};
    if(operation==2){
        xuiTestProxyInit(&s.backend);assert(xuiSetProxy(s.context,&s.backend.tProxy)==XUI_OK);
        assert(xuiTestSurfaceCreate(&s.backend,&target,32,32,0)==XUI_OK);
        assert(xuiCallScopeBegin(s.context,&drawing)==XUI_OK && xuiPainterBegin(s.context,target,&painter)==XUI_OK);
    }
    s.operation=operation;s.action=action;s.failure=failure;
    xerror* ambient=NULL;if(ambientPresent){xrtSetErrorKind(XERR_ARGUMENT);ambient=xrtErrorRef(xrtGetError());assert(ambient);}
    int status;
    if(operation==0){xui_vec2_t size={0};status=xuiIconGetIntrinsicSize(root,&size);}
    else if(operation==1)status=xuiIconPrepare(root,16,16);
    else status=xuiIconDraw(painter,root,(xui_rect_t){0,0,16,16},NULL);
    xerror* result=xrtTakeError();assert(s.entered>=1 && status==(failure?XUI_ERROR_BACKEND_FAILED:XUI_OK));
    assert(result==(failure?s.expected:ambient));
    if(operation==2){
        /* The deliberately installed diagnostic is new to the enclosing
         * drawing receipt. It remains sticky there even when the returned
         * child operation outcome has already been handled above. */
        assert(xuiPainterEnd(painter)==XUI_OK);
        assert(xuiCallScopeEnd(&drawing)==(ambientPresent?XUI_ERROR_CALLBACK_FAILED:XUI_OK));
        xerror* restored=xrtTakeError();assert(restored==ambient);xrtErrorFree(restored);
        s.backend.tProxy.surfaceDestroy(&s.backend.tProxy,target);
    }
    s.operation=-1;s.failure=0;s.action=0;
    finish(&s);assert(s.calls[3]==1);
    xrtErrorFree(result);xrtErrorFree(ambient);
}
static void registration(void)
{
    for(int slot=0;slot<4;++slot)for(int mode=1;mode<=4;++mode){
        custom_state s={0};setup(&s);s.owners[slot].holdMode=mode;
        int status=create(&s);xerror* result=xrtTakeError();
        assert(status==(mode<=2?XUI_ERROR_CALLBACK_FAILED:XUI_ERROR_INVALID_STATE) && !s.icon && !s.calls[3]);
        assert(result==(mode==2?s.expected:NULL));
        if(!s.gone)assert(!xuiIconCategoryFind(s.context,"custom") || xuiIconCategoryGetIconCount(s.category)==0);
        finish(&s);xrtErrorFree(result);
    }
    for(int allowance=0;allowance<32;++allowance){
        custom_state s={0};setup(&s);assert(xrtMemDebugFailAfter(allowance));int status=create(&s);
        bool failed=xrtMemDebugFailTriggered();xrtMemDebugFailClear();xrtClearError();
        if(status!=XUI_OK){assert(failed && status==XUI_ERROR_OUT_OF_MEMORY && !s.icon && !s.calls[3]);assert(!xuiIconFind(s.category,"icon"));}
        else assert(s.icon && xuiIconCategoryGetIconCount(s.category)==1);
        finish(&s);assert(s.calls[3]==(status==XUI_OK?1u:0u));
    }
    custom_state shared={0};setup(&shared);shared.shared=true;assert(create(&shared)==XUI_OK && shared.owners[0].refs==5);finish(&shared);
    for(int mode=5;mode<=6;++mode){
        custom_state s={0};setup(&s);assert(xuiWidgetCreate(s.context,&s.stateWidget)==XUI_OK);
        assert(xuiWidgetSetEventHandler(s.stateWidget,XUI_EVENT_ENABLED_CHANGED,queued,&s)==XUI_OK);
        s.owners[0].holdMode=mode;assert(create(&s)==XUI_ERROR_INVALID_STATE && !s.icon && s.calls[3]==1);
        finish(&s);
    }
}
static void dropFailures(void)
{
    for(int slot=0;slot<4;++slot)for(int diagnostic=0;diagnostic<2;++diagnostic){
        custom_state s={0};setup(&s);assert(create(&s)==XUI_OK);s.owners[slot].dropMode=diagnostic?2:1;
        int status=xuiIconRemoveById(s.category,xuiIconGetId(s.icon));xerror* error=xrtTakeError();
        assert(status==XUI_ERROR_CALLBACK_FAILED && (error!=NULL)==(diagnostic!=0) && s.calls[3]==1);
        for(unsigned i=0;i<4;++i)assert(s.owners[i].refs==1);
        finish(&s);xrtErrorFree(error);
    }
}
static void heap(void)
{
    custom_state s={0};setup(&s);assert(create(&s)==XUI_OK);xuiTestProxyInit(&s.backend);
    assert(xuiSetProxy(s.context,&s.backend.tProxy)==XUI_OK);xui_surface target=NULL;xui_painter painter=NULL;xui_call_scope_t scope={0};
    assert(xuiTestSurfaceCreate(&s.backend,&target,32,32,0)==XUI_OK);
    assert(xuiCallScopeBegin(s.context,&scope)==XUI_OK && xuiPainterBegin(s.context,target,&painter)==XUI_OK);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);assert(xrtMemDebugFailAfter(0));
    for(int i=0;i<1000;++i){xui_vec2_t size={0};assert(xuiIconGetIntrinsicSize(s.icon,&size)==XUI_OK && xuiIconPrepare(s.icon,16,16)==XUI_OK);assert(xuiIconCategoryPreload(s.category,16,16)==XUI_OK && xuiIconDraw(painter,s.icon,(xui_rect_t){0,0,16,16},NULL)==XUI_OK);}
    xrtMemDebugFailClear();xrtMemDebugSnapshot(&after);assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount);
    assert(xuiPainterEnd(painter)==XUI_OK && xuiCallScopeEnd(&scope)==XUI_OK);finish(&s);s.backend.tProxy.surfaceDestroy(&s.backend.tProxy,target);
}
int main(void)
{
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    for(int operation=0;operation<3;++operation)for(int action=0;action<3;++action)for(int failure=0;failure<3;++failure)
        for(int ambient=0;ambient<2;++ambient)for(int alias=0;alias<2;++alias)check(operation,action,failure,ambient,alias);
    registration();dropFailures();heap();puts("Owned custom Icon: 108 invocation cases, 16 Hold failures/reentry, 32 OOM, 8 consuming Drop failures and 4000 zero-heap calls passed");return 0;
}
