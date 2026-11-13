/* Stop before duplicate physical destruction or returning to retired carriers
 * on an old engine. No intentional double-free or use-after-free is executed. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct cache_clear_state {
    xui_test_proxy_state_t backend;
    xui_context context;xui_icon_category category;xui_icon icon;
    xui_lifetime_link_t contextLink,categoryLink,iconLink;
    xui_surface_destroy_proc originalDestroy;
    xui_surface activeSurface;
    int source,action,armed,entered,activeDestroy,contextGone,categoryGone,iconGone,writeError;
    xerror* expected;
} cache_clear_state;
static void contextGone(void* raw){++((cache_clear_state*)raw)->contextGone;}
static void categoryGone(void* raw){++((cache_clear_state*)raw)->categoryGone;}
static void iconGone(void* raw){++((cache_clear_state*)raw)->iconGone;}
static void destroy(xui_proxy proxy,xui_surface surface)
{
    cache_clear_state* s=proxy->pUser;
    if(s->activeDestroy && surface==s->activeSurface){
        puts("ICON_CACHE_SURFACE_DESTROYED_TWICE");fflush(stdout);_Exit(81);
    }
    if(s->armed){
        s->armed=0;++s->entered;s->activeDestroy=1;s->activeSurface=surface;
        if(s->action==0)assert(xuiIconTouch(s->icon)==XUI_OK);
        else if(s->action==1)assert(xuiIconRemoveById(s->category,xuiIconGetId(s->icon))==XUI_OK);
        else if(s->action==2)assert(xuiIconCategoryRemove(s->context,"icons")==XUI_OK);
        else if(s->action==3)xuiDestroy(s->context);
        else{
            assert(xuiIconTouch(s->icon)==XUI_OK);
            assert(xuiIconPrepare(s->icon,16,16)==XUI_OK);
        }
        if(s->iconGone || (s->source==2 && s->categoryGone) || s->contextGone){
            puts("ICON_CACHE_CARRIER_RETIRED_INSIDE_CLEAR");fflush(stdout);_Exit(82);
        }
        assert(!strcmp(xuiIconGetName(s->icon),"image"));
        if(s->writeError){xrtSetErrorKind(XERR_TYPE);s->expected=xrtErrorRef(xrtGetError());assert(s->expected);}
        s->activeDestroy=0;s->activeSurface=NULL;
    }
    s->originalDestroy(proxy,surface);
}
static void check(int source,int action,int ambientPresent,int writeError)
{
    cache_clear_state s={0};s.source=source;s.action=action;s.writeError=writeError;
    xuiTestProxyInit(&s.backend);s.originalDestroy=s.backend.tProxy.surfaceDestroy;s.backend.tProxy.surfaceDestroy=destroy;
    assert(xuiCreate(&s.context)==XUI_OK && xuiSetProxy(s.context,&s.backend.tProxy)==XUI_OK);
    assert(xuiIconCategoryCreate(s.context,"icons",NULL,&s.category)==XUI_OK);
    const char* data="<svg width=\"16\" height=\"16\"></svg>";
    if(source==1)assert(xuiIconAddRasterMemory(s.category,"image",data,(int)strlen(data),NULL,&s.icon)==XUI_OK);
    else assert(xuiIconAddSvgMemory(s.category,"image",data,(int)strlen(data),NULL,&s.icon)==XUI_OK);
    assert(xuiContextLifetimeWatch(s.context,&s.contextLink,contextGone,&s)==XUI_OK);
    assert(xuiIconCategoryLifetimeWatch(s.category,&s.categoryLink,categoryGone,&s)==XUI_OK);
    assert(xuiIconLifetimeWatch(s.icon,&s.iconLink,iconGone,&s)==XUI_OK);
    assert(xuiIconPrepare(s.icon,16,16)==XUI_OK);
    if(source!=1)assert(xuiIconPrepare(s.icon,32,32)==XUI_OK);
    int loaded=source==1?s.backend.iSurfaceLoadCount:s.backend.iSvgSurfaceLoadCount;
    xerror* ambient=NULL;if(ambientPresent){xrtSetErrorKind(XERR_RANGE);ambient=xrtErrorRef(xrtGetError());assert(ambient);}
    s.armed=1;int status;
    if(source==2){xui_icon_category_desc_t desc;xuiIconCategoryDescDefault(&desc);desc.iCacheCapacity=3;status=xuiIconCategorySetDesc(s.category,&desc);}
    else status=xuiIconTouch(s.icon);
    xerror* result=xrtTakeError();assert(status==(writeError?XUI_ERROR_CALLBACK_FAILED:XUI_OK));
    assert(s.entered==1 && result==(writeError?s.expected:ambient));
    if(action==4){
        int before=source==1?s.backend.iSurfaceLoadCount:s.backend.iSvgSurfaceLoadCount;
        assert(before==loaded+1);
        assert(xuiIconPrepare(s.icon,16,16)==XUI_OK);
        assert((source==1?s.backend.iSurfaceLoadCount:s.backend.iSvgSurfaceLoadCount)==before);
    }
    if(!s.contextGone)xuiDestroy(s.context);
    assert(s.iconGone==1 && s.categoryGone==1 && s.contextGone==1);
    assert(s.backend.iSurfaceDestroyCount==(source==1?s.backend.iSurfaceLoadCount:s.backend.iSvgSurfaceLoadCount));
    assert(!s.contextLink.ppPrevious && !s.categoryLink.ppPrevious && !s.iconLink.ppPrevious && !xrtGetError());
    xrtErrorFree(result);xrtErrorFree(s.expected);xrtErrorFree(ambient);
}
static void checkHeap(void)
{
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xui_context context=NULL;xui_icon_category category=NULL;xui_icon icon=NULL;
    assert(xuiCreate(&context)==XUI_OK && xuiIconCategoryCreate(context,"empty",NULL,&category)==XUI_OK);
    assert(xuiIconAddSvgPath(category,"path","M0 0 H16 V16 H0 Z",(xui_rect_t){0,0,16,16},NULL,NULL,&icon)==XUI_OK);
    xui_icon_category_desc_t desc;xuiIconCategoryDescDefault(&desc);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    for(int i=0;i<1000;++i){
        assert(xuiIconTouch(icon)==XUI_OK);
        desc.fWidth=(float)(16+(i&1));
        assert(xuiIconCategorySetDesc(category,&desc)==XUI_OK);
    }
    xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount);
    xuiDestroy(context);
}
int main(int argc,char** argv)
{
    if(argc==2)check(atoi(argv[1]),0,0,0);
    else for(int source=0;source<3;++source)for(int action=0;action<5;++action)
        for(int ambient=0;ambient<2;++ambient)for(int error=0;error<2;++error)check(source,action,ambient,error);
    checkHeap();puts("Icon cache clear: 60 reentry/lifetime/error cases and 2000 zero-heap empty mutations passed");return 0;
}
