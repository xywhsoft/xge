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


typedef struct ContextOwner ContextOwner;
typedef struct ContextUser {unsigned marker;ContextOwner* owner;} ContextUser;
struct ContextOwner {
    unsigned refs,calls,drops,released;int kind,action,holdAction;
    bool inCall,failHold,failDrop,dropError,dropAny;
    int callbackResult;
    xui_context context;xui_widget widget;ContextOwner* other;ContextUser* otherUser;
};
static int install_context(int,xui_widget,ContextUser*,const xui_callback_owner_t*);
static int clear_context(int,xui_widget);
static xui_callback_owner_t context_ref(ContextOwner*);
static bool context_hold(const void* raw)
{
    ContextOwner* o=(ContextOwner*)raw;if(o->failHold)return false;assert(o->refs);++o->refs;
    int action=o->holdAction;o->holdAction=0;
    if(action==1){xui_callback_owner_t ref=context_ref(o->other);assert(install_context(o->kind,o->widget,o->otherUser,&ref)==XUI_OK);}
    else if(action==2)xuiDestroy(o->context);
    return true;
}
static bool context_drop(const void* raw)
{
    ContextOwner* o=(ContextOwner*)raw;assert(o->refs);++o->drops;
    bool final=!--o->refs;
    if(final){assert(!o->inCall);++o->released;}
    if(o->dropError && (final || o->dropAny))xrtSetErrorInfo(XERR_STATE,"context-test",902,"secondary cleanup error");
    return !(o->failDrop && (final || o->dropAny));
}
static bool context_count(const void* raw,size_t* count){*count=((const ContextOwner*)raw)->refs;return *count!=0;}
static bool context_trace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops context_ops={context_count,context_trace};
static xui_callback_owner_t context_ref(ContextOwner* owner){return(xui_callback_owner_t){{owner,&context_ops},context_hold,context_drop};}
static bool context_visit(xrtownershipref ref,void* raw){assert(ref.Data && ref.Ops==&context_ops);++*(unsigned*)raw;return true;}
static int context_body(xui_widget widget,void* raw)
{
    ContextUser* user=raw;assert(user && user->marker==0xCE101u);ContextOwner* o=user->owner;
    assert(o->widget==widget && o->refs && !o->released);++o->calls;o->inCall=true;
    if(o->action==1)assert(clear_context(o->kind,widget)==XUI_OK);
    else if(o->action==2)xuiDestroy(o->context);
    assert(o->refs && !o->released);o->inCall=false;return o->callbackResult;
}
static int context_xui_dock_context_proc(xui_widget_t* pWidget, const xui_dock_hit_t* pHit, float fX, float fY, void* pUser){return context_body(pWidget,pUser);}
static int context_xui_flow_context_proc(xui_widget_t* pWidget, const xui_flow_hit_t* pHit, float fX, float fY, void* pUser){return context_body(pWidget,pUser);}
static int context_xui_list_view_context_proc(xui_widget_t* pWidget, int iIndex, float fX, float fY, void* pUser){return context_body(pWidget,pUser);}
static int context_xui_property_grid_context_proc(xui_widget_t* pWidget, int iPart, int iCategory, int iProperty, float fX, float fY, void* pUser){return context_body(pWidget,pUser);}
static int context_xui_statusbar_context_proc(xui_widget_t* pWidget, int iIndex, int iValue, float fX, float fY, void* pUser){return context_body(pWidget,pUser);}
static int context_xui_table_grid_context_proc(xui_widget_t* pWidget, int iPart, int iRow, int iColumn, float fX, float fY, void* pUser){return context_body(pWidget,pUser);}
static int context_xui_table_view_context_proc(xui_widget_t* pWidget, int iPart, int iRow, int iColumn, float fX, float fY, void* pUser){return context_body(pWidget,pUser);}
static int context_xui_table_view_merge_proc(xui_widget_t* pWidget, int iRow, int iColumn, int* pRowSpan, int* pColSpan, void* pUser){*pRowSpan=2;*pColSpan=1;return context_body(pWidget,pUser);}
static int context_xui_tabs_context_proc(xui_widget_t* pWidget, int iIndex, float fX, float fY, void* pUser){return context_body(pWidget,pUser);}
static int context_xui_tag_input_context_proc(xui_widget_t* pWidget, int iIndex, float fX, float fY, void* pUser){return context_body(pWidget,pUser);}
static int context_xui_toolbar_context_proc(xui_widget_t* pWidget, int iIndex, int iValue, float fX, float fY, void* pUser){return context_body(pWidget,pUser);}
static int context_xui_tree_view_context_proc(xui_widget_t* pWidget, int iNodeId, int iVisibleIndex, float fX, float fY, void* pUser){return context_body(pWidget,pUser);}
static int context_xui_window_context_proc(xui_widget_t* pWidget, int iPart, float fX, float fY, void* pUser){return context_body(pWidget,pUser);}
static int make_context(int kind,xui_context c,xui_widget* w){switch(kind){
case 0:return xuiTagInputCreate(c,w,NULL);
case 1:return xuiTabsCreate(c,w,NULL);
case 2:return xuiWindowCreate(c,w,NULL);
case 3:return xuiDockPanelCreate(c,w,NULL);
case 4:return xuiListViewCreate(c,w,NULL);
case 5:return xuiTreeViewCreate(c,w,NULL);
case 6:return xuiTableViewCreate(c,w,NULL);
case 7:return xuiTableGridCreate(c,w,NULL);
case 8:return xuiPropertyGridCreate(c,w,NULL);
case 9:return xuiToolbarCreate(c,w,NULL);
case 10:return xuiStatusBarCreate(c,w,NULL);
case 11:return xuiFlowGraphWidgetCreate(c,w,NULL);
case 12:return xuiWorkflowWidgetCreate(c,w,NULL);
case 13:return xuiTableViewCreate(c,w,NULL);
default:return XUI_ERROR_INVALID_ARGUMENT;}}
static int install_context(int kind,xui_widget w,ContextUser* user,const xui_callback_owner_t* owner){switch(kind){
case 0:return xuiTagInputSetContextMenuOwned(w,context_xui_tag_input_context_proc,user,owner);
case 1:return xuiTabsSetContextMenuOwned(w,context_xui_tabs_context_proc,user,owner);
case 2:return xuiWindowSetContextMenuOwned(w,context_xui_window_context_proc,user,owner);
case 3:return xuiDockPanelSetContextMenuOwned(w,context_xui_dock_context_proc,user,owner);
case 4:return xuiListViewSetContextMenuOwned(w,context_xui_list_view_context_proc,user,owner);
case 5:return xuiTreeViewSetContextMenuOwned(w,context_xui_tree_view_context_proc,user,owner);
case 6:return xuiTableViewSetContextMenuOwned(w,context_xui_table_view_context_proc,user,owner);
case 7:return xuiTableGridSetContextMenuOwned(w,context_xui_table_grid_context_proc,user,owner);
case 8:return xuiPropertyGridSetContextMenuOwned(w,context_xui_property_grid_context_proc,user,owner);
case 9:return xuiToolbarSetContextMenuOwned(w,context_xui_toolbar_context_proc,user,owner);
case 10:return xuiStatusBarSetContextMenuOwned(w,context_xui_statusbar_context_proc,user,owner);
case 11:return xuiFlowGraphWidgetSetContextMenuOwned(w,context_xui_flow_context_proc,user,owner);
case 12:return xuiWorkflowWidgetSetContextMenuOwned(w,context_xui_flow_context_proc,user,owner);
case 13:return xuiTableViewSetMergeProviderOwned(w,context_xui_table_view_merge_proc,user,owner);
default:return XUI_ERROR_INVALID_ARGUMENT;}}
static int clear_context(int kind,xui_widget w){switch(kind){
case 0:return xuiTagInputSetContextMenu(w,NULL,NULL);
case 1:return xuiTabsSetContextMenu(w,NULL,NULL);
case 2:return xuiWindowSetContextMenu(w,NULL,NULL);
case 3:return xuiDockPanelSetContextMenu(w,NULL,NULL);
case 4:return xuiListViewSetContextMenu(w,NULL,NULL);
case 5:return xuiTreeViewSetContextMenu(w,NULL,NULL);
case 6:return xuiTableViewSetContextMenu(w,NULL,NULL);
case 7:return xuiTableGridSetContextMenu(w,NULL,NULL);
case 8:return xuiPropertyGridSetContextMenu(w,NULL,NULL);
case 9:return xuiToolbarSetContextMenu(w,NULL,NULL);
case 10:return xuiStatusBarSetContextMenu(w,NULL,NULL);
case 11:return xuiFlowGraphWidgetSetContextMenu(w,NULL,NULL);
case 12:return xuiWorkflowWidgetSetContextMenu(w,NULL,NULL);
case 13:return xuiTableViewSetMergeProvider(w,NULL,NULL);
default:return XUI_ERROR_INVALID_ARGUMENT;}}

static void setup_context(int kind,xui_context* c,xui_widget* w)
{
    assert(xuiCreate(c)==XUI_OK);xui_widget root=NULL;
    assert(xuiWidgetCreate(*c,&root)==XUI_OK && xuiSetRootWidget(*c,root)==XUI_OK && xuiWidgetSetRect(root,(xui_rect_t){0,0,400,240})==XUI_OK);
    int made=make_context(kind,*c,w);if(made!=XUI_OK)fprintf(stderr,"setup kind=%d make=%d\n",kind,made);assert(made==XUI_OK);
    if(!xuiWidgetGetParent(*w))assert(xuiWidgetAddChild(root,*w)==XUI_OK);
    assert(xuiInputViewport(*c,400,240)==XUI_OK && xuiWidgetSetRect(*w,(xui_rect_t){0,0,400,240})==XUI_OK);
    if(kind==1){const char* tabs[]={"one"};assert(xuiTabsSetItems(*w,tabs,1)==XUI_OK);}
    if(kind==6 || kind==13){xui_table_view_column_t column={.fWidth=80};assert(xuiTableViewSetColumns(*w,&column,1)==XUI_OK && xuiTableViewSetRows(*w,NULL,3)==XUI_OK);}
    assert(xuiLayout(*c)==XUI_OK);
}
static int fire_context(int kind,xui_context context,xui_widget widget)
{
    if(kind==13){xui_rect_t rect={0};return xuiTableViewGetCellContentRect(widget,0,0,&rect);}
    xui_widget target=widget;
    if(kind==7)target=xuiTableGridGetTableView(widget);
    if(kind==8)target=xuiPropertyGridGetTableView(widget);
    if(kind==12)target=xuiWorkflowWidgetGetCanvas(widget);
    xui_event_t event={.iType=XUI_EVENT_CONTEXT_MENU,.pTarget=target,.iKey=XUI_KEY_CONTEXT_MENU,.fX=20,.fY=20};
    return xuiDispatchEvent(context,&event);
}
static void context_publication(void)
{
    for(int kind=0;kind<14;++kind){
        xui_context c=NULL;xui_widget w=NULL;setup_context(kind,&c,&w);
        ContextOwner owner={.refs=1,.kind=kind,.context=c,.widget=w};ContextUser user={0xCE101,&owner};xui_callback_owner_t ref=context_ref(&owner);
        for(int point=0;point<2;++point){assert(xrtMemDebugFailAfter(point));assert(install_context(kind,w,&user,&ref)==XUI_ERROR_OUT_OF_MEMORY);assert(xrtMemDebugFailAfter(-1));xrtClearError();assert(owner.refs==1);}
        assert(install_context(kind,w,&user,&ref)==XUI_OK && owner.refs==2);
        xrtownershipscope freeze={0};unsigned edges=0;assert(xrtOwnershipFreezeTryBegin(&freeze));assert(xuiContextTraceOwnership(c,context_visit,&edges) && edges==1);assert(xrtOwnershipScopeEnd(&freeze));
        owner.failHold=true;assert(install_context(kind,w,&user,&ref)==XUI_ERROR_CALLBACK_FAILED && owner.refs==2);owner.failHold=false;
        assert(clear_context(kind,w)==XUI_OK && owner.refs==1);assert(context_drop(&owner));xuiDestroy(c);assert(owner.released==1);
    }
}
static void context_publishers(void)
{
    for(int kind=0;kind<14;++kind)for(int action=0;action<3;++action){
        xui_context c=NULL;xui_widget w=NULL;setup_context(kind,&c,&w);
        ContextOwner owner={.refs=1,.kind=kind,.action=action,.callbackResult=kind==13 ? 1:XUI_EVENT_DISPATCH_STOP,.context=c,.widget=w};ContextUser user={0xCE101,&owner};xui_callback_owner_t ref=context_ref(&owner);
        assert(install_context(kind,w,&user,&ref)==XUI_OK && context_drop(&owner));
        xui_call_scope_t scope={0};assert(xuiCallScopeBegin(c,&scope)==XUI_OK);
        int result=fire_context(kind,c,w);int ended=xuiCallScopeEnd(&scope);
        if(!owner.calls)fprintf(stderr,"context not fired kind=%d action=%d result=%d ended=%d\n",kind,action,result,ended);
        assert(owner.calls);
        if(!action){assert(result>=0 && ended==XUI_OK);xuiDestroy(c);}
        else if(action==1){xuiDestroy(c);}
        assert(owner.released==1);
    }
}
static void context_snapshots(void)
{
    for(int kind=0;kind<14;++kind)for(int close=0;close<2;++close){
        xui_context c=NULL;xui_widget w=NULL;setup_context(kind,&c,&w);
        ContextOwner old={.refs=1,.kind=kind,.callbackResult=XUI_EVENT_DISPATCH_STOP,.context=c,.widget=w},other=old;
        ContextUser oldUser={0xCE101,&old},otherUser={0xCE101,&other};xui_callback_owner_t ref=context_ref(&old);
        assert(install_context(kind,w,&oldUser,&ref)==XUI_OK && context_drop(&old));
        old.other=&other;old.otherUser=&otherUser;old.holdAction=close ? 2:1;
        xui_call_scope_t scope={0};assert(xuiCallScopeBegin(c,&scope)==XUI_OK);(void)fire_context(kind,c,w);int ended=xuiCallScopeEnd(&scope);
        assert(old.calls==(close ? 0u:1u) && !other.calls);
        if(close)assert(ended==XUI_ERROR_INVALID_STATE);else xuiDestroy(c);
        assert(old.released==1 && other.refs==1 && context_drop(&other));xrtClearError();
    }
}
static void error_click(xui_widget widget,int layer,int frame,int button,int modifiers,void* user){(void)widget;(void)layer;(void)frame;(void)button;(void)modifiers;(void)user;}
static void grouped_errors(void)
{
    xui_context c=NULL;xui_widget w=NULL;assert(xuiCreate(&c)==XUI_OK && xuiTimeLineViewCreate(c,&w,NULL)==XUI_OK);
    ContextOwner a={.refs=1,.dropAny=true,.dropError=true,.failDrop=true},b={.refs=1,.failHold=true};xui_callback_owner_t ar=context_ref(&a),br=context_ref(&b);
    /* First Hold failure has no diagnostic; rollback Drop adds a secondary one. */
    assert(xuiTimeLineViewSetFrameClickOwned(w,error_click,NULL,&ar,error_click,NULL,&br)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError() && a.refs==1 && b.refs==1);
    a.dropAny=a.dropError=a.failDrop=false;b.failHold=false;
    assert(xuiTimeLineViewSetFrameClickOwned(w,error_click,NULL,&ar,error_click,NULL,&br)==XUI_OK);
    assert(context_drop(&a) && context_drop(&b));a.failDrop=true;b.failDrop=b.dropError=true;
    assert(xuiTimeLineViewSetFrameClickOwned(w,NULL,NULL,NULL,NULL,NULL,NULL)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError());
    assert(a.released==1 && b.released==1);xuiDestroy(c);
}

static int veto_and_clear(xui_widget widget,int oldFrame,int newFrame,void* user)
{
    (void)oldFrame;(void)newFrame;ContextOwner* owner=user;owner->inCall=true;++owner->calls;
    assert(xuiTimeLineViewSetCurrentFrameCallbacksOwned(widget,NULL,NULL,NULL,NULL,NULL,NULL)==XUI_OK);
    assert(owner->refs);owner->inCall=false;return 1;
}
static int negative_count(xui_widget widget,void* user){(void)widget;(void)user;return XUI_ERROR_UNSUPPORTED;}
static int unused_node(xui_widget widget,int index,xui_tree_view_node_output_t* node,void* user)
{(void)widget;(void)index;(void)node;(void)user;assert(0);return XUI_ERROR_CALLBACK_FAILED;}
static void failure_results_and_warm(void)
{
    xui_context c=NULL;xui_widget w=NULL;setup_context(0,&c,&w);
    ContextOwner owner={.refs=1,.kind=0,.action=1,.callbackResult=XUI_ERROR_UNSUPPORTED,.context=c,.widget=w};ContextUser user={0xCE101,&owner};xui_callback_owner_t ref=context_ref(&owner);
    assert(install_context(0,w,&user,&ref)==XUI_OK && context_drop(&owner));owner.failDrop=owner.dropError=true;
    xui_call_scope_t scope={0};assert(xuiCallScopeBegin(c,&scope)==XUI_OK);
    assert(fire_context(0,c,w)==XUI_ERROR_UNSUPPORTED && !xrtGetError());
    assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError() && owner.released==1);xuiDestroy(c);
    assert(xuiCreate(&c)==XUI_OK && xuiTimeLineViewCreate(c,&w,NULL)==XUI_OK && xuiTimeLineViewSetFrameCount(w,16)==XUI_OK);
    owner=(ContextOwner){.refs=1};ref=context_ref(&owner);
    assert(xuiTimeLineViewSetCurrentFrameCallbacksOwned(w,veto_and_clear,&owner,&ref,NULL,NULL,NULL)==XUI_OK && context_drop(&owner));owner.failDrop=true;
    int original=xuiTimeLineViewGetCurrentFrame(w);assert(xuiCallScopeBegin(c,&scope)==XUI_OK);
    assert(xuiTimeLineViewSetCurrentFrame(w,original+1)==XUI_OK && xuiTimeLineViewGetCurrentFrame(w)==original);
    assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED && owner.released==1);xuiDestroy(c);
    assert(xuiCreate(&c)==XUI_OK && xuiTreeViewCreate(c,&w,NULL)==XUI_OK);
    owner=(ContextOwner){.refs=1,.dropAny=true,.dropError=true,.failDrop=true};ref=context_ref(&owner);
    assert(xuiTreeViewSetAdapterOwned(w,negative_count,&owner,&ref,unused_node,NULL,NULL)==XUI_ERROR_UNSUPPORTED && !xrtGetError());
    owner.dropAny=owner.dropError=owner.failDrop=false;xuiDestroy(c);assert(context_drop(&owner) && owner.released==1);
    setup_context(0,&c,&w);owner=(ContextOwner){.refs=1,.kind=0,.callbackResult=XUI_EVENT_DISPATCH_STOP,.context=c,.widget=w};user=(ContextUser){0xCE101,&owner};ref=context_ref(&owner);
    assert(install_context(0,w,&user,&ref)==XUI_OK && xuiCallScopeBegin(c,&scope)==XUI_OK && fire_context(0,c,w)==XUI_OK);
    xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);for(unsigned i=0;i<1000;++i)assert(fire_context(0,c,w)==XUI_OK);xrtMemDebugSnapshot(&b);
    assert(a.AllocCount==b.AllocCount && a.FreeCount==b.FreeCount && a.ReallocCount==b.ReallocCount);
    assert(xuiCallScopeEnd(&scope)==XUI_OK);xuiDestroy(c);assert(context_drop(&owner) && owner.released==1);
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);assert(xrtMemDebugEnable(true));xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);
    context_publication();context_publishers();context_snapshots();grouped_errors();failure_results_and_warm();xrtClearError();xrtMemDebugSnapshot(&b);assert(a.LiveCount==b.LiveCount && a.LiveBytes==b.LiveBytes && a.InvalidFreeCount==b.InvalidFreeCount && a.DoubleFreeCount==b.DoubleFreeCount);
    puts("typed Context/merge publishers: exact callbacks, native dispatch, snapshots, OOM, Hold reentry and close passed");return 0;
}
