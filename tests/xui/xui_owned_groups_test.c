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
typedef struct GroupOwner GroupOwner;
typedef struct GroupUser {unsigned marker;GroupOwner* owner;} GroupUser;
struct GroupOwner {unsigned refs,calls,released;bool inCall,failHold,failDrop;int kind,action,holdAction;xui_context context;xui_widget widget;GroupUser* replacements[2];};
static int install_group(int,xui_widget,GroupUser*,const xui_callback_owner_t*,GroupUser*,const xui_callback_owner_t*);
static int clear_group(int,xui_widget);
static xui_callback_owner_t group_ref(GroupOwner*);
static bool group_hold(const void* raw)
{GroupOwner* o=(GroupOwner*)raw;if(o->failHold)return false;assert(o->refs);++o->refs;int action=o->holdAction;o->holdAction=0;if(action==1){xui_callback_owner_t a=group_ref(o->replacements[0]->owner),b=group_ref(o->replacements[1]->owner);assert(install_group(o->kind,o->widget,o->replacements[0],&a,o->replacements[1],&b)==XUI_OK);}else if(action==2)xuiDestroy(o->context);return true;}
static bool group_drop(const void* raw)
{GroupOwner* o=(GroupOwner*)raw;assert(o->refs);if(--o->refs)return true;assert(!o->inCall);++o->released;return !o->failDrop;}
static bool group_count(const void* raw,size_t* out){const GroupOwner* o=raw;*out=o->refs;return o->refs!=0;}
static bool group_trace(const void* raw,xrtownershipvisitor fn,void* user){(void)raw;(void)fn;(void)user;return true;}
static const xrtownershipops group_ops={group_count,group_trace};
static xui_callback_owner_t group_ref(GroupOwner* o){return(xui_callback_owner_t){{o,&group_ops},group_hold,group_drop};}
static bool group_visit(xrtownershipref ref,void* raw){assert(ref.Ops==&group_ops && ref.Data);++*(unsigned*)raw;return true;}
static int group_body(xui_widget w,void* raw)
{GroupUser* u=raw;assert(u && (u->marker==0xAA01 || u->marker==0xBB02));GroupOwner* o=u->owner;assert(o->refs && !o->released && w==o->widget);++o->calls;o->inCall=true;if(o->action==1)assert(clear_group(o->kind,w)==XUI_OK);else if(o->action==2)xuiDestroy(o->context);assert(o->refs && !o->released);o->inCall=false;return o->action==3 ? 0:1;}
static void group_xui_timeline_context_command_proc(xui_widget_t* pWidget, int iCommand, const xui_timeline_hit_t* pHit, void* pUser) {(void)group_body(pWidget,pUser);}
static int group_xui_timeline_context_opening_proc(xui_widget_t* pWidget, const xui_timeline_hit_t* pHit, void* pUser) {return group_body(pWidget,pUser);}
static void group_xui_timeline_current_frame_changed_proc(xui_widget_t* pWidget, int iOldFrame, int iNewFrame, void* pUser) {(void)group_body(pWidget,pUser);}
static int group_xui_timeline_current_frame_changing_proc(xui_widget_t* pWidget, int iOldFrame, int iNewFrame, void* pUser) {return group_body(pWidget,pUser);}
static void group_xui_timeline_frame_changed_proc(xui_widget_t* pWidget, int iLayer, int iFrame, int iOldType, int iNewType, void* pUser) {(void)group_body(pWidget,pUser);}
static int group_xui_timeline_frame_changing_proc(xui_widget_t* pWidget, int iLayer, int iFrame, int iOldType, int iNewType, void* pUser) {return group_body(pWidget,pUser);}
static void group_xui_timeline_frame_click_proc(xui_widget_t* pWidget, int iLayer, int iFrame, int iButton, int iModifiers, void* pUser) {(void)group_body(pWidget,pUser);}
static void group_xui_timeline_layer_changed_proc(xui_widget_t* pWidget, int iLayer, int iChange, void* pUser) {(void)group_body(pWidget,pUser);}
static int group_xui_timeline_layer_changing_proc(xui_widget_t* pWidget, int iLayer, int iChange, void* pUser) {return group_body(pWidget,pUser);}
static void group_xui_timeline_span_changed_proc(xui_widget_t* pWidget, int iSpanId, int iChange, void* pUser) {(void)group_body(pWidget,pUser);}
static int group_xui_timeline_span_changing_proc(xui_widget_t* pWidget, int iSpanId, int iChange, void* pUser) {return group_body(pWidget,pUser);}
static int install_group(int k,xui_widget w,GroupUser* a,const xui_callback_owner_t* ar,GroupUser* b,const xui_callback_owner_t* br)
{switch(k){
case 0:return xuiTimeLineViewSetCurrentFrameCallbacksOwned(w,group_xui_timeline_current_frame_changing_proc,a,ar,group_xui_timeline_current_frame_changed_proc,b,br);
case 1:return xuiTimeLineViewSetLayerCallbacksOwned(w,group_xui_timeline_layer_changing_proc,a,ar,group_xui_timeline_layer_changed_proc,b,br);
case 2:return xuiTimeLineViewSetFrameCallbacksOwned(w,group_xui_timeline_frame_changing_proc,a,ar,group_xui_timeline_frame_changed_proc,b,br);
case 3:return xuiTimeLineViewSetSpanCallbacksOwned(w,group_xui_timeline_span_changing_proc,a,ar,group_xui_timeline_span_changed_proc,b,br);
case 4:return xuiTimeLineViewSetContextMenuOwned(w,group_xui_timeline_context_opening_proc,a,ar,group_xui_timeline_context_command_proc,b,br);
case 5:return xuiTimeLineViewSetFrameClickOwned(w,group_xui_timeline_frame_click_proc,a,ar,group_xui_timeline_frame_click_proc,b,br);
default:return XUI_ERROR_INVALID_ARGUMENT;}}
static int clear_group(int k,xui_widget w)
{switch(k){
case 0:return xuiTimeLineViewSetCurrentFrameCallbacks(w,NULL,NULL,NULL);
case 1:return xuiTimeLineViewSetLayerCallbacks(w,NULL,NULL,NULL);
case 2:return xuiTimeLineViewSetFrameCallbacks(w,NULL,NULL,NULL);
case 3:return xuiTimeLineViewSetSpanCallbacks(w,NULL,NULL,NULL);
case 4:return xuiTimeLineViewSetContextMenu(w,NULL,NULL,NULL);
case 5:return xuiTimeLineViewSetFrameClick(w,NULL,NULL,NULL);
default:return XUI_ERROR_INVALID_ARGUMENT;}}
static void make_group(xui_context* c,xui_widget* w)
{assert(xuiCreate(c)==XUI_OK && xuiTimeLineViewCreate(*c,w,NULL)==XUI_OK);assert(xuiTimeLineViewSetFrameCount(*w,16)==XUI_OK && xuiTimeLineViewAddLayer(*w,"layer",NULL)==XUI_OK);}
static int fire_group(int k,xui_widget w)
{if(k==0)return xuiTimeLineViewSetCurrentFrame(w,1);if(k==1)return xuiTimeLineViewSetLayerName(w,0,"changed");if(k==2)return xuiTimeLineViewSetFrame(w,0,1,XUI_TIMELINE_FRAME_KEY,NULL);return xuiTimeLineViewAddSpan(w,0,1,3,XUI_TIMELINE_SPAN_MOTION,"span",NULL);}
static void group_publication(void)
{
    for(int k=0;k<6;++k){
        xui_context c=NULL;xui_widget w=NULL;make_group(&c,&w);
        GroupOwner a={.refs=1,.kind=k,.context=c,.widget=w},b={.refs=1,.kind=k,.context=c,.widget=w};GroupUser au={0xAA01,&a},bu={0xBB02,&b};xui_callback_owner_t ar=group_ref(&a),br=group_ref(&b);
        for(int point=0;point<3;++point){assert(xrtMemDebugFailAfter(point));assert(install_group(k,w,&au,&ar,&bu,&br)==XUI_ERROR_OUT_OF_MEMORY);assert(xrtMemDebugFailAfter(-1));xrtClearError();assert(a.refs==1 && b.refs==1);}
        assert(install_group(k,w,&au,&ar,&bu,&br)==XUI_OK && a.refs==2 && b.refs==2);
        xrtownershipscope freeze={0};unsigned edges=0;assert(xrtOwnershipFreezeTryBegin(&freeze));assert(xuiContextTraceOwnership(c,group_visit,&edges) && edges==2);assert(xrtOwnershipScopeEnd(&freeze));
        GroupOwner x={.refs=1,.kind=k,.context=c,.widget=w},y={.refs=1,.kind=k,.context=c,.widget=w,.failHold=true};GroupUser xu={0xAA01,&x},yu={0xBB02,&y};xui_callback_owner_t xr=group_ref(&x),yr=group_ref(&y);
        assert(install_group(k,w,&xu,&xr,&yu,&yr)==XUI_ERROR_CALLBACK_FAILED && a.refs==2 && b.refs==2 && x.refs==1 && y.refs==1);y.failHold=false;
        if(k<4){xui_call_scope_t scope={0};assert(xuiCallScopeBegin(c,&scope)==XUI_OK);assert(fire_group(k,w)==XUI_OK);assert(xuiCallScopeEnd(&scope)==XUI_OK);assert(a.calls==1 && b.calls==1 && !x.calls && !y.calls);}
        assert(clear_group(k,w)==XUI_OK && a.refs==1 && b.refs==1);
        x.replacements[0]=&au;x.replacements[1]=&bu;x.holdAction=1;
        assert(install_group(k,w,&xu,&xr,&yu,&yr)==XUI_OK && x.refs==2 && y.refs==2 && a.refs==1 && b.refs==1);
        assert(group_drop(&a) && group_drop(&b) && group_drop(&x) && group_drop(&y));xuiDestroy(c);assert(a.released==1 && b.released==1 && x.released==1 && y.released==1);
    }
}
static void group_publishers(void)
{
    for(int k=0;k<4;++k)for(int action=0;action<4;++action){
        xui_context c=NULL;xui_widget w=NULL;make_group(&c,&w);
        GroupOwner a={.refs=1,.kind=k,.context=c,.widget=w,.action=action},b={.refs=1,.kind=k,.context=c,.widget=w};GroupUser au={0xAA01,&a},bu={0xBB02,&b};xui_callback_owner_t ar=group_ref(&a),br=group_ref(&b);assert(install_group(k,w,&au,&ar,&bu,&br)==XUI_OK);assert(group_drop(&a) && group_drop(&b));
        xui_call_scope_t scope={0};assert(xuiCallScopeBegin(c,&scope)==XUI_OK);assert(fire_group(k,w)==XUI_OK);assert(xuiCallScopeEnd(&scope)==XUI_OK);assert(a.calls==1 && b.calls==(action==0 ? 1u:0u));if(action!=2)xuiDestroy(c);assert(a.released==1 && b.released==1);
    }
    for(int k=0;k<6;++k){
        xui_context c=NULL;xui_widget w=NULL;make_group(&c,&w);GroupOwner a={.refs=1,.kind=k,.context=c,.widget=w,.holdAction=2},b={.refs=1,.kind=k,.context=c,.widget=w};GroupUser au={0xAA01,&a},bu={0xBB02,&b};xui_callback_owner_t ar=group_ref(&a),br=group_ref(&b);assert(install_group(k,w,&au,&ar,&bu,&br)==XUI_ERROR_INVALID_STATE && a.refs==1 && b.refs==1);assert(group_drop(&a) && group_drop(&b));
    }
}
static void group_snapshot_reentry(void)
{
    for(int k=0;k<4;++k)for(int close=0;close<2;++close){
        xui_context c=NULL;xui_widget w=NULL;make_group(&c,&w);
        GroupOwner a={.refs=1,.kind=k,.context=c,.widget=w},b={.refs=1,.kind=k,.context=c,.widget=w},x={.refs=1,.kind=k,.context=c,.widget=w},y={.refs=1,.kind=k,.context=c,.widget=w};
        GroupUser au={0xAA01,&a},bu={0xBB02,&b},xu={0xAA01,&x},yu={0xBB02,&y};xui_callback_owner_t ar=group_ref(&a),br=group_ref(&b);assert(install_group(k,w,&au,&ar,&bu,&br)==XUI_OK);assert(group_drop(&a) && group_drop(&b));
        a.replacements[0]=&xu;a.replacements[1]=&yu;a.holdAction=close ? 2:1;
        xui_call_scope_t scope={0};assert(xuiCallScopeBegin(c,&scope)==XUI_OK);assert(fire_group(k,w)==XUI_OK);assert(xuiCallScopeEnd(&scope)==(close ? XUI_ERROR_INVALID_STATE:XUI_OK));
        assert(a.calls==(close ? 0u:1u) && !b.calls && !x.calls && y.calls==(close ? 0u:1u));
        if(!close)xuiDestroy(c);
        assert(a.released==1 && b.released==1);assert(group_drop(&x) && group_drop(&y) && x.released==1 && y.released==1);
    }
}
static void group_warm_and_failures(void)
{
    xui_context c=NULL;xui_widget w=NULL;make_group(&c,&w);GroupOwner a={.refs=1,.kind=0,.context=c,.widget=w},b={.refs=1,.kind=0,.context=c,.widget=w};GroupUser au={0xAA01,&a},bu={0xBB02,&b};xui_callback_owner_t ar=group_ref(&a),br=group_ref(&b);assert(install_group(0,w,&au,&ar,&bu,&br)==XUI_OK);
    xui_call_scope_t scope={0};assert(xuiCallScopeBegin(c,&scope)==XUI_OK);assert(fire_group(0,w)==XUI_OK);
    xmemdebugsnapshot x,y;xrtMemDebugSnapshot(&x);for(unsigned i=0;i<1000;++i)assert(xuiTimeLineViewSetCurrentFrame(w,i%2 ? 1:2)==XUI_OK);xrtMemDebugSnapshot(&y);assert(x.AllocCount==y.AllocCount && x.FreeCount==y.FreeCount && x.ReallocCount==y.ReallocCount);
    a.failHold=true;unsigned calls=a.calls;assert(xuiTimeLineViewSetCurrentFrame(w,3)==XUI_OK && a.calls==calls);assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED);a.failHold=false;
    assert(group_drop(&a) && group_drop(&b));b.action=1;b.failDrop=true;assert(xuiCallScopeBegin(c,&scope)==XUI_OK);assert(xuiTimeLineViewSetCurrentFrame(w,4)==XUI_OK);assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED);assert(a.released==1 && b.released==1);xuiDestroy(c);
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);assert(xrtMemDebugEnable(true));xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);group_publication();group_publishers();group_snapshot_reentry();group_warm_and_failures();xrtClearError();xrtMemDebugSnapshot(&b);assert(a.LiveCount==b.LiveCount && a.LiveBytes==b.LiveBytes && a.InvalidFreeCount==b.InvalidFreeCount && a.DoubleFreeCount==b.DoubleFreeCount);puts("six independent typed callback groups: atomic OOM/second-Hold rollback, real veto/change, Hold reentry, close, self-clear, graph and zero-allocation warm notifications passed");return 0;
}
