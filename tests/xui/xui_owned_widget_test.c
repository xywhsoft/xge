/* Exact base-widget publishers. No window, input injection or GPU is used. */
#define main xui_owned_event_regression_main
#include "xui_owned_callback_test.c"
#undef main

enum { BASE_KEEP=200,BASE_CLEAR,BASE_CLOSE,BASE_NEST };
typedef struct User {unsigned marker;Owner* owner;int slot;} User;
static int clear_slot(xui_widget w,int slot)
{
    switch(slot){
    case 0:return xuiWidgetSetDefaultAction(w,NULL,NULL);
    case 1:return xuiWidgetSetCancelAction(w,NULL,NULL);
    case 2:return xuiWidgetSetCursorQueryCallback(w,NULL,NULL);
    case 3:return xuiWidgetSetImeCandidateRect(w,NULL,NULL);
    case 4:return xuiWidgetSetCacheRenderCallback(w,NULL,NULL);
    default:return xuiWidgetSetUpdateCallback(w,NULL,NULL);
    }
}
static void fire(xui_context,xui_widget,int);
static void body(xui_widget widget,void* raw)
{
    User* user=raw;assert(user->marker==0xCE11U);Owner* owner=user->owner;
    assert(owner->refs>=2 && !owner->released);bool nested=owner->inCall;
    owner->inCall=true;++owner->calls;
    if(owner->action==BASE_CLEAR)assert(clear_slot(widget,user->slot)==XUI_OK);
    else if(owner->action==BASE_CLOSE)xuiDestroy(owner->context);
    else if(owner->action==BASE_NEST && !nested)fire(owner->context,widget,user->slot);
    assert(owner->refs && !owner->released);owner->inCall=nested;
}
static void action(xui_widget w,void* u){body(w,u);}
static int cursor(xui_widget w,int x,int y,void* u)
{assert(x==12 && y==13);body(w,u);return XUI_CURSOR_HAND;}
static xui_rect_t ime(xui_widget w,void* u)
{body(w,u);return (xui_rect_t){1,2,3,4};}
static int render(xui_widget w,xui_draw_context d,uint32_t s,void* u)
{assert(d==(xui_draw_context)(uintptr_t)0x1234 && s==9);body(w,u);return XUI_OK;}
static int update(xui_widget w,float delta,void* u)
{assert(delta==0.25f);body(w,u);return XUI_OK;}
static int install(xui_widget w,int slot,void* user,const xui_callback_owner_t* ref)
{
    switch(slot){
    case 0:return xuiWidgetSetDefaultActionOwned(w,action,user,ref);
    case 1:return xuiWidgetSetCancelActionOwned(w,action,user,ref);
    case 2:return xuiWidgetSetCursorQueryCallbackOwned(w,cursor,user,ref);
    case 3:return xuiWidgetSetImeCandidateRectOwned(w,ime,user,ref);
    case 4:return xuiWidgetSetCacheRenderCallbackOwned(w,render,user,ref);
    default:return xuiWidgetSetUpdateCallbackOwned(w,update,user,ref);
    }
}
static void fire(xui_context c,xui_widget w,int slot)
{
    /* Layout/cache draw needs a backend; its typed publisher is invoked under
     * the same enclosing native operation here, with a non-dereferenced token.
     * Public input/cursor/update/IME entry points are covered separately. */
    xuiInternalOperationEnter(c);
    switch(slot){
    case 0:assert(xuiInternalInvokeDefaultAction(w)==XUI_OK);break;
    case 1:assert(xuiInternalInvokeCancelAction(w)==XUI_OK);break;
    case 2:(void)xuiInternalInvokeCursorQueryCallback(w,12,13);break;
    case 3:(void)xuiInternalInvokeImeCandidateRect(w);break;
    case 4:assert(xuiInternalInvokeCacheRenderCallback(w,(xui_draw_context)(uintptr_t)0x1234,9)==XUI_OK);break;
    default:assert(xuiInternalInvokeUpdateCallback(w,0.25f)==XUI_OK);break;
    }
    xuiInternalOperationLeave(c);
}
static void six_lifetimes(void)
{
    for(int slot=0;slot<6;++slot)for(int behavior=BASE_KEEP;behavior<=BASE_NEST;++behavior){
        xui_context c=NULL;xui_widget w=NULL;create(&c,&w);
        Owner owner={.refs=1,.context=c,.action=behavior};User user={0xCE11U,&owner,slot};
        xui_callback_owner_t ref=descriptor(&owner);assert(install(w,slot,&user,&ref)==XUI_OK && drop(&owner));
        fire(c,w,slot);assert(owner.calls==(behavior==BASE_NEST ? 2u : 1u));
        if(behavior!=BASE_CLOSE)xuiDestroy(c);
        assert(owner.refs==0 && owner.released==1);
    }
}
static void setup_root(xui_context c,xui_widget w)
{
    assert(xuiSetRootWidget(c,w)==XUI_OK && xuiSetViewportSize(c,100,100)==XUI_OK);
    assert(xuiWidgetSetFocusable(w,1)==XUI_OK && xuiSetFocusWidget(c,w)==XUI_OK);
    c->pActiveWidget=w;
}
static void public_publishers(void)
{
    for(int slot=0;slot<6;++slot){
        if(slot==4)continue;
        xui_context c=NULL;xui_widget w=NULL;create(&c,&w);setup_root(c,w);
        Owner owner={.refs=1,.context=c,.action=BASE_KEEP};User user={0xCE11U,&owner,slot};
        xui_callback_owner_t ref=descriptor(&owner);assert(install(w,slot,&user,&ref)==XUI_OK);
        unsigned before=owner.calls;
        if(slot<=1)assert(xuiInputKeyDown(c,slot ? XUI_KEY_ESCAPE:XUI_KEY_ENTER,0)==XUI_OK);
        else if(slot==2)assert(xuiQueryCursor(c,12,13)==XUI_CURSOR_HAND);
        else if(slot==3)assert(xuiWidgetSetImeMode(w,XUI_IME_ENABLED)==XUI_OK);
        else assert(xuiUpdate(c,0.25f)==XUI_OK);
        assert(owner.calls>before);owner.action=BASE_CLOSE;
        if(slot<=1)assert(xuiInputKeyDown(c,slot ? XUI_KEY_ESCAPE:XUI_KEY_ENTER,0)==XUI_OK);
        else if(slot==2)assert(xuiQueryCursor(c,12,13)==XUI_CURSOR_ARROW);
        else if(slot==3)assert(xuiWidgetSetImeMode(w,XUI_IME_ENABLED)==XUI_OK);
        else assert(xuiUpdate(c,0.25f)==XUI_OK);
        assert(owner.refs==1 && drop(&owner));
    }
}
static void graph_and_warm(void)
{
    xui_context c=NULL;xui_widget w=NULL;create(&c,&w);
    Owner owner={.refs=1,.context=c,.action=BASE_KEEP};User users[6];xui_callback_owner_t ref=descriptor(&owner);
    for(int slot=0;slot<6;++slot){users[slot]=(User){0xCE11U,&owner,slot};assert(install(w,slot,&users[slot],&ref)==XUI_OK);}
    xrtownershipscope freeze={0};unsigned edges=0;assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceOwnership(c,visit,&edges) && edges==6);assert(xrtOwnershipScopeEnd(&freeze));
    xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);
    for(unsigned i=0;i<200;++i)for(int slot=0;slot<6;++slot)fire(c,w,slot);
    xrtMemDebugSnapshot(&b);assert(a.AllocCount==b.AllocCount && a.FreeCount==b.FreeCount && a.ReallocCount==b.ReallocCount);
    assert(drop(&owner));xuiDestroy(c);assert(owner.released==1);
}
static void base_failures(void)
{
    for(int slot=0;slot<6;++slot){
        xui_context c=NULL;xui_widget w=NULL;create(&c,&w);
        Owner owner={.refs=1,.context=c,.action=BASE_KEEP};User user={0xCE11U,&owner,slot};xui_callback_owner_t ref=descriptor(&owner);
        for(int fail=0;fail<2;++fail){assert(xrtMemDebugFailAfter(fail));
            assert(install(w,slot,&user,&ref)==XUI_ERROR_OUT_OF_MEMORY);assert(xrtMemDebugFailAfter(-1));xrtClearError();
            assert(owner.refs==1 && !w->pEventOwners);}
        assert(install(w,slot,&user,&ref)==XUI_OK);owner.failHold=true;
        xui_call_scope_t scope={0};assert(xuiCallScopeBegin(c,&scope)==XUI_OK);
        /* Errors surface through checked call scopes even for void / record /
         * enum-returning callbacks that cannot return a status to the caller. */
        xuiInternalOperationEnter(c);
        switch(slot){case 0:(void)xuiInternalInvokeDefaultAction(w);break;case 1:(void)xuiInternalInvokeCancelAction(w);break;
        case 2:(void)xuiInternalInvokeCursorQueryCallback(w,12,13);break;case 3:(void)xuiInternalInvokeImeCandidateRect(w);break;
        case 4:(void)xuiInternalInvokeCacheRenderCallback(w,(xui_draw_context)(uintptr_t)0x1234,9);break;
        default:(void)xuiInternalInvokeUpdateCallback(w,0.25f);break;}
        xuiInternalOperationLeave(c);assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED && !owner.calls && !xrtGetError());
        owner.failHold=false;assert(clear_slot(w,slot)==XUI_OK && owner.refs==1);xuiDestroy(c);assert(drop(&owner));
    }
}
typedef struct Reentry {Owner base;User* replacement;bool pending;} Reentry;
static bool replace_during_hold(const void* raw)
{
    Reentry* reentry=(Reentry*)raw;if(!hold(&reentry->base))return false;
    if(reentry->pending){
        reentry->pending=false;User* next=reentry->replacement;
        xui_callback_owner_t ref=descriptor(next->owner);
        assert(install(reentry->base.widget,next->slot,next,&ref)==XUI_OK);
    }
    return true;
}
static void base_registration_reentry(void)
{
    for(int slot=0;slot<6;++slot){
        xui_context c=NULL;xui_widget w=NULL;create(&c,&w);
        Owner nested={.refs=1,.context=c,.action=BASE_KEEP};User next={0xCE11U,&nested,slot};
        Reentry outer={{.refs=1,.context=c,.widget=w,.action=BASE_KEEP},&next,true};
        User current={0xCE11U,&outer.base,slot};xui_callback_owner_t ref=descriptor(&outer.base);ref.onHold=replace_during_hold;
        assert(install(w,slot,&current,&ref)==XUI_OK && outer.base.refs==2 && nested.refs==1);
        fire(c,w,slot);assert(outer.base.calls==1 && !nested.calls);
        xuiDestroy(c);assert(drop(&outer.base) && drop(&nested));
        for(int close=3;close<=4;++close){
            create(&c,&w);Owner owner={.refs=1,.context=c,.widget=w,.holdAction=close};
            User user={0xCE11U,&owner,slot};ref=descriptor(&owner);
            assert(install(w,slot,&user,&ref)==XUI_ERROR_INVALID_STATE && owner.refs==1);
            assert(drop(&owner));if(close==3)xuiDestroy(c);
        }
    }
}
int main(void)
{
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);
    six_lifetimes();public_publishers();graph_and_warm();base_failures();base_registration_reentry();xrtClearError();xrtMemDebugSnapshot(&b);
    assert(a.LiveCount==b.LiveCount && a.LiveBytes==b.LiveBytes && a.InvalidFreeCount==b.InvalidFreeCount && a.DoubleFreeCount==b.DoubleFreeCount);
    puts("six base widget owned callbacks: typed publishers, public entry points, close/reentry, graph, OOM and 200 warm calls passed");return 0;
}
