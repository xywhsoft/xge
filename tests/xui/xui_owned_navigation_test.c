/* Plain native C publishers; no language call scope encloses these APIs. */
#define main xui_owned_navigation_base_main
#include "xui_owned_callback_test.c"
#undef main

enum {NAV_KEEP=600,NAV_CLEAR,NAV_CLOSE,NAV_NEST};
typedef struct NavigationUser {unsigned marker;Owner* owner;int kind;xui_widget peer;} NavigationUser;
static int clear_navigation(xui_widget w,int k)
{
    switch(k){case 0:return xuiHyperlinkSetClick(w,NULL,NULL);case 1:return xuiPageSetChange(w,NULL,NULL);
    case 2:return xuiCarouselSetChange(w,NULL,NULL);case 3:return xuiBreadcrumbSetClick(w,NULL,NULL);
    default:return xuiBreadcrumbSetContextMenu(w,NULL,NULL);}
}
static int fire_navigation(xui_widget w,int k,unsigned serial)
{
    if(k==0)return xuiWidgetPerformAccessibleAction(w,1,XUI_ACCESSIBLE_ACTION_ACTIVATE,NULL);
    if(k==1)return xuiPageSetCurrent(w,serial%2 ? 1:2,1);
    if(k==2)return xuiCarouselSetCurrent(w,serial%2 ? 0:1,1);
    if(k==3)return xuiWidgetPerformAccessibleAction(w,2,XUI_ACCESSIBLE_ACTION_ACTIVATE,NULL);
    xui_event_t event={0};event.iSize=sizeof(event);event.iType=XUI_EVENT_CONTEXT_MENU;event.iKey=XUI_KEY_CONTEXT_MENU;event.pTarget=w;
    return xuiDispatchEvent(w->pContext,&event);
}
static void navigation_body(xui_widget w,void* raw)
{
    NavigationUser* user=raw;assert(user->marker==0xCE12U);Owner* owner=user->owner;
    assert(owner->refs>=2 && !owner->released);bool nested=owner->inCall;owner->inCall=true;++owner->calls;
    if(owner->action==NAV_CLEAR)assert(clear_navigation(w,user->kind)==XUI_OK);
    else if(owner->action==NAV_CLOSE)xuiDestroy(owner->context);
    else if(owner->action==NAV_NEST && !nested)assert(fire_navigation(user->peer,user->kind,0)==XUI_OK);
    assert(owner->refs && !owner->released);owner->inCall=nested;
}
static void navigation_click(xui_widget w,void* u){navigation_body(w,u);}
static void navigation_change(xui_widget w,int oldPage,int page,void* u){assert(oldPage!=page);navigation_body(w,u);}
static void navigation_breadcrumb(xui_widget w,int index,int value,void* u){assert(index==0 && value==71);navigation_body(w,u);}
static int navigation_context(xui_widget w,int index,int value,float x,float y,void* u)
{(void)index;(void)value;(void)x;(void)y;navigation_body(w,u);return XUI_OK;}
static int install_navigation(xui_widget w,int k,NavigationUser* user,const xui_callback_owner_t* ref)
{
    switch(k){case 0:return xuiHyperlinkSetClickOwned(w,navigation_click,user,ref);
    case 1:return xuiPageSetChangeOwned(w,navigation_change,user,ref);case 2:return xuiCarouselSetChangeOwned(w,navigation_change,user,ref);
    case 3:return xuiBreadcrumbSetClickOwned(w,navigation_breadcrumb,user,ref);
    default:return xuiBreadcrumbSetContextMenuOwned(w,navigation_context,user,ref);}
}
static xui_widget make_navigation(xui_context c,int k)
{
    xui_widget w=NULL;int result;
    switch(k){case 0:result=xuiHyperlinkCreate(c,&w,NULL);break;case 1:result=xuiPageCreate(c,&w,NULL);break;
    case 2:result=xuiCarouselCreate(c,&w,NULL);break;default:result=xuiBreadcrumbCreate(c,&w,NULL);break;}
    assert(result==XUI_OK && w);
    assert(xuiWidgetSetRect(w,(xui_rect_t){0,0,240,48})==XUI_OK);
    if(k==1)assert(xuiPageSetPageCount(w,2)==XUI_OK);
    else if(k==2)assert(xuiCarouselSetPageCount(w,2)==XUI_OK);
    else if(k>=3){xui_breadcrumb_item_t item={"root",1,71};assert(xuiBreadcrumbSetItems(w,&item,1)==XUI_OK);}
    return w;
}
static void navigation_lifetimes(void)
{
    for(int k=0;k<5;++k)for(int behavior=NAV_KEEP;behavior<=NAV_NEST;++behavior){
        xui_context c=NULL;assert(xuiCreate(&c)==XUI_OK);xui_widget w=make_navigation(c,k);
        Owner owner={.refs=1,.context=c,.action=behavior};NavigationUser user={0xCE12U,&owner,k,NULL};xui_callback_owner_t ref=descriptor(&owner);
        assert(install_navigation(w,k,&user,&ref)==XUI_OK);
        if(behavior==NAV_NEST){user.peer=make_navigation(c,k);assert(install_navigation(user.peer,k,&user,&ref)==XUI_OK);}
        assert(drop(&owner));assert(fire_navigation(w,k,0)==XUI_OK);assert(owner.calls==(behavior==NAV_NEST?2u:1u));
        if(behavior!=NAV_CLOSE)xuiDestroy(c);
        assert(!owner.refs && owner.released==1);
    }
}
static void navigation_warm(void)
{
    xui_context c=NULL;assert(xuiCreate(&c)==XUI_OK);Owner owner={.refs=1,.context=c,.action=NAV_KEEP};NavigationUser users[5];xui_widget widgets[5];xui_callback_owner_t ref=descriptor(&owner);
    for(int k=0;k<5;++k){widgets[k]=make_navigation(c,k);users[k]=(NavigationUser){0xCE12U,&owner,k,NULL};assert(install_navigation(widgets[k],k,&users[k],&ref)==XUI_OK);}
    xrtownershipscope freeze={0};unsigned edges=0;assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceOwnership(c,visit,&edges) && edges==5);assert(xrtOwnershipScopeEnd(&freeze));
    for(int k=0;k<5;++k){assert(fire_navigation(widgets[k],k,0)==XUI_OK);assert(fire_navigation(widgets[k],k,1)==XUI_OK);}
    unsigned calls=owner.calls;xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);
    for(unsigned n=0;n<200;++n)for(int k=0;k<5;++k)assert(fire_navigation(widgets[k],k,n)==XUI_OK);
    xrtMemDebugSnapshot(&b);assert(owner.calls==calls+1000 && a.AllocCount==b.AllocCount && a.FreeCount==b.FreeCount && a.ReallocCount==b.ReallocCount);
    assert(drop(&owner));xuiDestroy(c);assert(owner.released==1);
}
static void navigation_failures(void)
{
    for(int k=0;k<5;++k){
        xui_context c=NULL;assert(xuiCreate(&c)==XUI_OK);xui_widget w=make_navigation(c,k);Owner owner={.refs=1,.context=c,.action=NAV_KEEP};NavigationUser user={0xCE12U,&owner,k,NULL};xui_callback_owner_t ref=descriptor(&owner);
        for(int point=0;point<2;++point){assert(xrtMemDebugFailAfter(point));assert(install_navigation(w,k,&user,&ref)==XUI_ERROR_OUT_OF_MEMORY);assert(xrtMemDebugFailAfter(-1));xrtClearError();assert(owner.refs==1 && !w->pEventOwners);}
        assert(install_navigation(w,k,&user,&ref)==XUI_OK);owner.failHold=true;
        xui_call_scope_t scope={0};assert(xuiCallScopeBegin(c,&scope)==XUI_OK);int result=fire_navigation(w,k,0);assert(result==XUI_OK || result==XUI_ERROR_CALLBACK_FAILED);
        assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED && !owner.calls && !xrtGetError());owner.failHold=false;
        owner.action=NAV_CLEAR;owner.failDrop=true;assert(drop(&owner));assert(xuiCallScopeBegin(c,&scope)==XUI_OK);
        result=fire_navigation(w,k,1);assert(result==XUI_OK || result==XUI_ERROR_CALLBACK_FAILED);assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED && owner.calls==1 && !xrtGetError());
        assert(!owner.refs && owner.released==1);xuiDestroy(c);
    }
}
int main(void)
{
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);
    navigation_lifetimes();navigation_warm();navigation_failures();xrtClearError();xrtMemDebugSnapshot(&b);
    assert(a.LiveCount==b.LiveCount && a.LiveBytes==b.LiveBytes && a.InvalidFreeCount==b.InvalidFreeCount && a.DoubleFreeCount==b.DoubleFreeCount);
    puts("five owned navigation callbacks: real native publishers, close/replace/reentry, graph, OOM, checked failure and warm allocation checks passed");return 0;
}
