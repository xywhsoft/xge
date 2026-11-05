/* CPU-only, real native publishers. Every installed callback owns distinct
 * userdata and ownership data; no function pointer is erased or cast. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include "xge_host_xrt.h"
#include <xge_host_xrt_contract.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct HostEntry {const char* name;xge_host_xrt_proc address;} HostEntry;
static const HostEntry entries[]={
#define XGE_HOST_XRT_SLOT(name) {#name,(xge_host_xrt_proc)name},
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
};
static xge_host_xrt_proc resolve(void* user,const char* name)
{(void)user;size_t first=0,last=sizeof(entries)/sizeof(*entries);while(first<last){size_t middle=first+(last-first)/2;int order=strcmp(name,entries[middle].name);if(order<0)last=middle;else if(order>0)first=middle+1;else return entries[middle].address;}return NULL;}


#include "xui_test_proxy.h"
#include "src/xui/core/xui_internal.h"

typedef struct State {
    size_t refs,holds,drops;
    xui_toast toast;
    xui_msgtip tip;
    xui_context context;
    int action,dropAction,calls,closes;
    bool failHold,failDrop,dropDiagnostic;
} State;
typedef struct User {unsigned magic;State* owner;} User;
static bool ownerHold(const void* raw)
{
    State* state=(State*)raw;assert(state->refs);++state->holds;
    if(state->failHold)return false;
    ++state->refs;int action=state->action;state->action=0;
    if(action==1)assert(xuiToastClear(state->toast)==XUI_OK);
    if(action==2)assert(xuiToastDestroyChecked(state->toast)==XUI_OK);
    if(action==3)assert(xuiRequestClose(state->context)==XUI_OK);
    if(action==4)assert(xuiMsgTipDestroyChecked(state->tip)==XUI_OK);
    if(action==5)assert(xuiMsgTipSetCloseOwned(state->tip,NULL,NULL,NULL)==XUI_OK);
    return true;
}
static bool ownerDrop(const void* raw)
{
    State* state=(State*)raw;assert(state->refs);--state->refs;++state->drops;
    int action=state->dropAction;state->dropAction=0;if(action==1)assert(xuiRequestClose(state->context)==XUI_OK);
    if(state->dropDiagnostic)xrtSetErrorKind(XERR_STATE);
    return !state->failDrop;
}
static bool ownerCount(const void* raw,size_t* count){*count=((const State*)raw)->refs;return *count!=0;}
static bool ownerTrace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops ownerOps={ownerCount,ownerTrace};
static xui_callback_owner_t ownerRef(State* state){return(xui_callback_owner_t){{state,&ownerOps},ownerHold,ownerDrop};}
static void clicked(xui_toast toast,int id,void* raw)
{
    User* user=raw;assert(user->magic==0xCE44U && id>0 && user->owner->refs>=2);
    ++user->owner->calls;assert(user->owner->toast==toast);
}
static void closed(xui_toast toast,int id,int reason,void* raw)
{User* user=raw;assert(user->magic==0xCE44U && id>0 && reason>0 && user->owner->refs>=2);++user->owner->closes;(void)toast;}
static void invalidated(void* raw){++*(unsigned*)raw;}
static void click(xui_context context,xui_toast toast)
{
    xui_rect_t r=xuiToastGetItemRect(toast,0);float x=r.fX+r.fW*0.5f,y=r.fY+r.fH*0.5f;
    assert(r.fW>0 && r.fH>0);
    assert(xuiInputPointerMove(context,x,y,0)==XUI_OK && xuiDispatchPendingEvents(context)==XUI_OK);
    assert(xuiInputPointerDown(context,x,y,XUI_POINTER_BUTTON_LEFT,XUI_POINTER_BUTTON_LEFT)==XUI_OK && xuiDispatchPendingEvents(context)==XUI_OK);
    assert(xuiInputPointerUp(context,x,y,XUI_POINTER_BUTTON_LEFT,0)==XUI_OK && xuiDispatchPendingEvents(context)==XUI_OK);
    assert(xuiUpdate(context,0.0f)==XUI_OK);
}
typedef struct Fixture {xui_test_proxy_state_t provider;xui_context context;xui_font font;xui_toast toast;} Fixture;
static void setup(Fixture* f)
{
    memset(f,0,sizeof(*f));xuiTestProxyInit(&f->provider);
    assert(xuiCreate(&f->context)==XUI_OK && xuiSetProxy(f->context,&f->provider.tProxy)==XUI_OK);
    assert(xuiInputViewport(f->context,640,420)==XUI_OK);
    assert(f->provider.tProxy.fontLoadMemory(&f->provider.tProxy,&f->font,"body",4,14,XUI_FONT_FORMAT_TTF)==XUI_OK);
    assert(xuiSetDefaultFont(f->context,f->font)==XUI_OK);
    xui_toast_desc_t desc={0};desc.iSize=sizeof(desc);desc.pFont=f->font;desc.iPlacement=XUI_TOAST_PLACEMENT_TOP_RIGHT;
    assert(xuiToastCreate(f->context,&f->toast,&desc)==XUI_OK);
}
static void teardown(Fixture* f)
{
    if(f->toast)assert(xuiToastDestroyChecked(f->toast)==XUI_OK);
    assert(xuiDestroyChecked(f->context)==XUI_OK);
    f->provider.tProxy.fontDestroy(&f->provider.tProxy,f->font);
    xrtClearError();
}
static void snapshot_and_reentry(void)
{
    Fixture f;setup(&f);State state={.refs=1,.toast=f.toast,.context=f.context};User user={0xCE44U,&state};
    xui_callback_owner_t owner=ownerRef(&state);int id=0;
    assert(xuiToastShowOwned(f.toast,XUI_TOAST_TYPE_INFO,"Owned","Click body",3,clicked,&user,&owner,&id)==XUI_OK && id>0 && state.refs==2);
    state.action=1;click(f.context,f.toast);
    assert(state.calls==1 && state.refs==1 && state.holds==2 && state.drops==2 && xuiToastGetActiveCount(f.toast)==0);
    assert(xuiToastSetCloseOwned(f.toast,closed,&user,&owner)==XUI_OK && state.refs==2);
    assert(xuiToastShowOwned(f.toast,XUI_TOAST_TYPE_INFO,"Close","Owned close",3,NULL,NULL,NULL,&id)==XUI_OK);
    assert(xuiToastClear(f.toast)==XUI_OK && state.closes==1 && state.refs==2);
    assert(xuiToastSetCloseOwned(f.toast,NULL,NULL,NULL)==XUI_OK && state.refs==1);
    teardown(&f);assert(state.refs==1);
}
static void admission_failure(void)
{
    Fixture f;setup(&f);State state={.refs=1,.toast=f.toast,.context=f.context,.failHold=true};User user={0xCE44U,&state};
    xui_callback_owner_t owner=ownerRef(&state);int id=99;
    assert(xuiToastShowOwned(f.toast,XUI_TOAST_TYPE_INFO,"Hold","fails",3,clicked,&user,&owner,&id)==XUI_ERROR_CALLBACK_FAILED && id==0 && state.refs==1 && state.drops==0 && !xrtGetError());
    state.failHold=false;state.action=2;unsigned invalid=0;xui_lifetime_link_t watch={0};
    assert(xuiToastLifetimeWatch(f.toast,&watch,invalidated,&invalid)==XUI_OK);
    assert(xuiToastShowOwned(f.toast,XUI_TOAST_TYPE_INFO,"Reentry","close from Hold",3,clicked,&user,&owner,&id)<0 && id==0 && invalid==1 && state.refs==1);
    f.toast=NULL;teardown(&f);
}
static void cleanup_first_null(void)
{
    Fixture f;setup(&f);State a={.refs=1,.toast=f.toast,.context=f.context},b=a;User au={0xCE44U,&a},bu={0xCE44U,&b};
    xui_callback_owner_t ar=ownerRef(&a),br=ownerRef(&b);int id=0;
    assert(xuiToastShowOwned(f.toast,XUI_TOAST_TYPE_INFO,"A","first",3,clicked,&au,&ar,&id)==XUI_OK);
    assert(xuiToastShowOwned(f.toast,XUI_TOAST_TYPE_INFO,"B","second",3,clicked,&bu,&br,&id)==XUI_OK);
    a.failDrop=true;b.failDrop=true;b.dropDiagnostic=true;
    assert(xuiToastClear(f.toast)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError() && a.refs==1 && b.refs==1);
    assert(xuiToastGetActiveCount(f.toast)==0 && xuiToastGetPendingCount(f.toast)==0);
    a.failDrop=b.failDrop=false;b.dropDiagnostic=false;teardown(&f);
}
static void primary_window_and_context(void)
{
    Fixture f;setup(&f);State state={.refs=1,.toast=f.toast,.context=f.context};User user={0xCE44U,&state};
    xui_callback_owner_t owner=ownerRef(&state);int id=0;unsigned invalid=0;xui_lifetime_link_t watch={0};
    assert(xuiToastLifetimeWatch(f.toast,&watch,invalidated,&invalid)==XUI_OK);
    assert(xuiToastShowOwned(f.toast,XUI_TOAST_TYPE_INFO,"A","primary destroy",3,clicked,&user,&owner,&id)==XUI_OK);
    xui_widget primary=xuiToastGetItemWidget(f.toast,0);assert(primary);
    assert(xuiWidgetDestroyChecked(primary)==XUI_OK && invalid==1 && state.refs==1);
    f.toast=NULL;teardown(&f);
    setup(&f);state=(State){.refs=1,.toast=f.toast,.context=f.context};owner=ownerRef(&state);watch=(xui_lifetime_link_t){0};invalid=0;
    assert(xuiToastLifetimeWatch(f.toast,&watch,invalidated,&invalid)==XUI_OK);
    assert(xuiToastShowOwned(f.toast,XUI_TOAST_TYPE_INFO,"B","context destroy",3,clicked,&user,&owner,&id)==XUI_OK);
    f.toast=NULL;assert(xuiDestroyChecked(f.context)==XUI_OK && invalid==1 && state.refs==1);
    f.provider.tProxy.fontDestroy(&f.provider.tProxy,f.font);xrtClearError();
}
static void raw_requested_does_not_publish(void)
{
    Fixture f;setup(&f);State state={.refs=1,.toast=f.toast,.context=f.context};User user={0xCE44U,&state};
    xui_callback_owner_t owner=ownerRef(&state);int id=0;
    assert(xuiToastSetClose(f.toast,closed,&user)==XUI_OK);
    assert(xuiToastShowOwned(f.toast,XUI_TOAST_TYPE_INFO,"A","request from Drop",3,clicked,&user,&owner,&id)==XUI_OK);
    state.dropAction=1;
    assert(xuiToastClear(f.toast)==XUI_ERROR_INVALID_STATE && state.closes==0 && state.refs==1 && !xrtGetError());
    f.toast=NULL;teardown(&f);
}
static void tipClosed(xui_msgtip tip,int expired,void* raw)
{
    User* user=raw;assert(user->magic==0xCE44U && !expired && user->owner->refs>=2);++user->owner->closes;
    assert(xuiMsgTipShow(tip,XUI_MSGTIP_ICON_INFO,"reopened from final close callback",4)==XUI_OK);
}
static void tip_lifecycle_and_snapshot(void)
{
    Fixture f;setup(&f);assert(xuiToastDestroyChecked(f.toast)==XUI_OK);f.toast=NULL;
    xui_msgtip tip=NULL;xui_msgtip_desc_t desc={0};desc.iSize=sizeof(desc);desc.pFont=f.font;desc.sText="tip";desc.iType=XUI_MSGTIP_ICON_INFO;
    assert(xuiMsgTipCreate(f.context,&tip,&desc)==XUI_OK);
    State state={.refs=1,.tip=tip,.context=f.context};User user={0xCE44U,&state};xui_callback_owner_t owner=ownerRef(&state);
    assert(xuiMsgTipSetCloseOwned(tip,tipClosed,&user,&owner)==XUI_OK && state.refs==2);
    assert(xuiMsgTipShow(tip,XUI_MSGTIP_ICON_INFO,"message",5)==XUI_OK);
    state.action=5;
    assert(xuiMsgTipClose(tip)==XUI_OK && state.closes==1 && state.refs==1 && xuiMsgTipIsOpen(tip));
    assert(!strcmp(xuiMsgTipGetText(tip),"reopened from final close callback"));
    unsigned invalid=0;xui_lifetime_link_t watch={0};assert(xuiMsgTipLifetimeWatch(tip,&watch,invalidated,&invalid)==XUI_OK);
    xui_widget primary=xuiMsgTipGetWidget(tip);assert(primary);
    assert(xuiWidgetDestroyChecked(primary)==XUI_OK && invalid==1);
    teardown(&f);
}
static void ambient_is_not_failure(void)
{
    Fixture f;setup(&f);int id=0;xrtSetErrorKind(XERR_STATE);const xerror* old=xrtGetError();assert(old);
    assert(xuiToastShowOwned(f.toast,XUI_TOAST_TYPE_INFO,"A","ambient",3,NULL,NULL,NULL,&id)==XUI_OK && id>0 && xrtGetError()==old);
    assert(xuiToastClear(f.toast)==XUI_OK && xrtGetError()==old);
    xrtClearError();teardown(&f);
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    snapshot_and_reentry();admission_failure();cleanup_first_null();primary_window_and_context();raw_requested_does_not_publish();tip_lifecycle_and_snapshot();ambient_is_not_failure();
    puts("xui_owned_managers_test passed: real item callbacks, reentry, physical tails and ambient diagnostics");return 0;
}
