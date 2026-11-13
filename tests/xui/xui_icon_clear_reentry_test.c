/* Safe failure guard exits inside the finalizer, before an old engine returns
 * to its freed Category array. No intentional use-after-free is executed. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct clear_state {
    xui_context context;xui_icon_category category;
    xui_lifetime_link_t categoryLink,contextLink;
    int route,categoryGone,contextGone,destroyed,writeError;
    xerror* expected;
} clear_state;
static void categoryGone(void* raw){++((clear_state*)raw)->categoryGone;}
static void contextGone(void* raw){++((clear_state*)raw)->contextGone;}
static int draw(xui_icon icon,xui_painter painter,xui_rect_t rect,const xui_icon_draw_desc_t* desc,void* raw)
{(void)icon;(void)painter;(void)rect;(void)desc;(void)raw;return XUI_OK;}
static void destroy(xui_icon icon,void* raw)
{
    clear_state* s=raw;(void)icon;bool first=++s->destroyed==1;
    if(first){
        if(s->route==1)assert(xuiIconCategoryRemove(s->context,"category")==XUI_OK);
        else if(s->route==2)xuiIconCategoryClear(s->context);
        else if(s->route==3)xuiDestroy(s->context);
        else xuiIconClear(s->category);
    }
    if(s->categoryGone || s->contextGone){
        puts("CATEGORY_DESTROYED_INSIDE_ICON_CLEAR");fflush(stdout);_Exit(74);
    }
    assert(!strcmp(xuiIconCategoryGetName(s->category),"category"));
    if(first && s->writeError){
        xrtSetErrorKind(XERR_TYPE);s->expected=xrtErrorRef(xrtGetError());assert(s->expected);
    }
}
static void check(int route,int ambientPresent,int writeError)
{
    clear_state s={0};s.route=route;s.writeError=writeError;assert(xrtGetError()==NULL);
    assert(xuiCreate(&s.context)==XUI_OK);
    assert(xuiIconCategoryCreate(s.context,"category",NULL,&s.category)==XUI_OK);
    assert(xuiContextLifetimeWatch(s.context,&s.contextLink,contextGone,&s)==XUI_OK);
    assert(xuiIconCategoryLifetimeWatch(s.category,&s.categoryLink,categoryGone,&s)==XUI_OK);
    xui_icon_custom_desc_t desc={0};desc.iSize=sizeof(desc);desc.onDraw=draw;desc.onDestroy=destroy;desc.pUser=&s;
    xui_icon first=NULL,second=NULL;
    assert(xuiIconAddCustom(s.category,"first",&desc,NULL,&first)==XUI_OK);
    assert(xuiIconAddCustom(s.category,"second",&desc,NULL,&second)==XUI_OK);
    xerror* ambient=NULL;
    if(ambientPresent){xrtSetErrorKind(XERR_RANGE);ambient=xrtErrorRef(xrtGetError());assert(ambient);}
    xuiIconClear(s.category);
    xerror* result=xrtTakeError();assert(result==(writeError ? s.expected:ambient));
    assert(s.destroyed==2);
    if(route<=2){
        assert(s.categoryGone==1 && !s.contextGone);
        assert(xuiIconCategoryFind(s.context,"category")==NULL);
        xuiDestroy(s.context);
    }else if(route==3)assert(s.categoryGone==1 && s.contextGone==1);
    else{
        assert(!s.categoryGone && !s.contextGone);
        assert(xuiIconCategoryGetIconCount(s.category)==0);
        xuiDestroy(s.context);
    }
    assert(s.categoryGone==1 && s.contextGone==1);
    assert(!s.categoryLink.ppPrevious && !s.contextLink.ppPrevious);
    assert(xrtGetError()==NULL);xrtErrorFree(result);xrtErrorFree(s.expected);xrtErrorFree(ambient);
}
int main(int argc,char** argv)
{
    if(argc==2){int route=atoi(argv[1]);assert(route>=1 && route<=4);check(route,0,0);}
    else for(int route=1;route<=4;++route)
        for(int ambient=0;ambient<2;++ambient)
            for(int error=0;error<2;++error)check(route,ambient,error);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xui_context context=NULL;xui_icon_category category=NULL;
    assert(xuiCreate(&context)==XUI_OK && xuiIconCategoryCreate(context,"empty",NULL,&category)==XUI_OK);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    for(int i=0;i<1000;++i)xuiIconClear(category);
    xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount);
    xuiDestroy(context);
    puts("Icon clear: four reentry routes, 16 diagnostics cases and 1000 zero-heap empty clears passed");return 0;
}
