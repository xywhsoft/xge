/* Callback-lifetime floor for size, prepare, draw and category preload.
 * The baseline guard exits inside the callback before an old caller can
 * continue with freed native memory. No window or graphics device is used. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct operation_state {
    xui_context context;xui_icon_category category;xui_icon icon;
    xui_lifetime_link_t iconLink,contextLink;
    int operation,action,iconGone,contextGone,entered,destroyed,writeError;
    xerror* expected;
} operation_state;
static void iconGone(void* raw){++((operation_state*)raw)->iconGone;}
static void contextGone(void* raw){++((operation_state*)raw)->contextGone;}
static void invoke(operation_state* s,xui_icon icon)
{
    assert(!s->entered);++s->entered;
    if(s->action)xuiDestroy(s->context);
    else if(s->operation==3)assert(xuiIconCategoryRemove(s->context,"category")==XUI_OK);
    else assert(xuiIconRemoveById(s->category,xuiIconGetId(icon))==XUI_OK);
    if(s->iconGone || s->contextGone){
        puts("ICON_DESTROYED_INSIDE_ACTIVE_CALLBACK");fflush(stdout);_Exit(75);
    }
    assert(!strcmp(xuiIconGetName(icon),"custom"));
    if(s->writeError){xrtSetErrorKind(XERR_TYPE);s->expected=xrtErrorRef(xrtGetError());assert(s->expected);}
}
static int measure(xui_icon icon,xui_vec2_t* size,void* raw)
{
    operation_state* s=raw;*size=(xui_vec2_t){16,16};
    if(s->operation==0)invoke(s,icon);
    return XUI_OK;
}
static int prepare(xui_icon icon,int width,int height,void* raw)
{
    operation_state* s=raw;assert(width>0 && height>0);
    if(s->operation==1 || s->operation==3)invoke(s,icon);
    return XUI_OK;
}
static int draw(xui_icon icon,xui_painter painter,xui_rect_t rect,const xui_icon_draw_desc_t* desc,void* raw)
{
    operation_state* s=raw;(void)painter;(void)rect;(void)desc;
    if(s->operation==2)invoke(s,icon);
    return XUI_OK;
}
static void destroy(xui_icon icon,void* raw)
{operation_state* s=raw;assert(!strcmp(xuiIconGetName(icon),"custom"));++s->destroyed;}
static void check(int operation,int action,int alias,int ambientPresent,int writeError)
{
    operation_state s={0};s.operation=operation;s.action=action;s.writeError=writeError;
    assert(xrtGetError()==NULL && xuiCreate(&s.context)==XUI_OK);
    assert(xuiIconCategoryCreate(s.context,"category",NULL,&s.category)==XUI_OK);
    xui_icon_custom_desc_t desc={0};desc.iSize=sizeof(desc);desc.onMeasure=measure;desc.onPrepare=prepare;desc.onDraw=draw;desc.onDestroy=destroy;desc.pUser=&s;
    assert(xuiIconAddCustom(s.category,"custom",&desc,NULL,&s.icon)==XUI_OK);
    assert(xuiIconLifetimeWatch(s.icon,&s.iconLink,iconGone,&s)==XUI_OK);
    assert(xuiContextLifetimeWatch(s.context,&s.contextLink,contextGone,&s)==XUI_OK);
    xui_icon root=s.icon;
    if(alias)assert(xuiIconAddAlias(s.category,"alias",s.icon,NULL,&root)==XUI_OK);
    xerror* ambient=NULL;
    if(ambientPresent){xrtSetErrorKind(XERR_RANGE);ambient=xrtErrorRef(xrtGetError());assert(ambient);}
    xui_test_proxy_state_t backend;xui_surface target=NULL;xui_painter painter=NULL;xui_call_scope_t drawing={0};
    if(operation==2){
        xuiTestProxyInit(&backend);assert(xuiSetProxy(s.context,&backend.tProxy)==XUI_OK);
        assert(xuiTestSurfaceCreate(&backend,&target,32,32,0)==XUI_OK);
        /* Raw Painter clients explicitly hold their Context for the session.
         * Its public Begin/End do not promise a managed lifetime lease. */
        assert(xuiCallScopeBegin(s.context,&drawing)==XUI_OK);
        assert(xuiPainterBegin(s.context,target,&painter)==XUI_OK);
        if(ambient)xrtSetErrorTake(xrtErrorRef(ambient));
    }
    int status;
    if(operation==0){xui_vec2_t size={0};status=xuiIconGetIntrinsicSize(root,&size);assert(size.fX==16 && size.fY==16);}
    else if(operation==1)status=xuiIconPrepare(root,16,16);
    else if(operation==2)status=xuiIconDraw(painter,root,(xui_rect_t){0,0,16,16},NULL);
    else status=xuiIconCategoryPreload(s.category,16,16);
    xerror* result=xrtTakeError();
    assert(s.entered==1 && status==(writeError ? XUI_ERROR_CALLBACK_FAILED:XUI_OK));
    assert(result==(writeError ? s.expected:ambient));
    if(operation==2){
        assert(xuiPainterEnd(painter)==XUI_OK);
        int ending=xuiCallScopeEnd(&drawing);xerror* tail=xrtTakeError();
        /* The operation's returned diagnostic was handled above. It must not
         * poison the surrounding painter session's ordinary return boundary. */
        assert(ending==XUI_OK && tail==ambient);xrtErrorFree(tail);
        backend.tProxy.surfaceDestroy(&backend.tProxy,target);
    }
    if(!s.contextGone)xuiDestroy(s.context);
    assert(s.iconGone==1 && s.contextGone==1 && s.destroyed==1);
    assert(!s.iconLink.ppPrevious && !s.contextLink.ppPrevious && xrtGetError()==NULL);
    xrtErrorFree(result);xrtErrorFree(s.expected);xrtErrorFree(ambient);
}
static void checkHeap(void)
{
    operation_state s={0};s.operation=-1;
    xui_test_proxy_state_t backend;xuiTestProxyInit(&backend);
    assert(xuiCreate(&s.context)==XUI_OK && xuiSetProxy(s.context,&backend.tProxy)==XUI_OK);
    assert(xuiIconCategoryCreate(s.context,"category",NULL,&s.category)==XUI_OK);
    xui_icon_custom_desc_t desc={0};desc.iSize=sizeof(desc);desc.onMeasure=measure;desc.onPrepare=prepare;desc.onDraw=draw;desc.onDestroy=destroy;desc.pUser=&s;
    assert(xuiIconAddCustom(s.category,"custom",&desc,NULL,&s.icon)==XUI_OK);
    xui_surface target=NULL;xui_painter painter=NULL;
    assert(xuiTestSurfaceCreate(&backend,&target,32,32,0)==XUI_OK);
    assert(xuiPainterBegin(s.context,target,&painter)==XUI_OK);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    for(int i=0;i<1000;++i){
        xui_vec2_t size={0};assert(xuiIconGetIntrinsicSize(s.icon,&size)==XUI_OK);
        assert(xuiIconPrepare(s.icon,16,16)==XUI_OK);
        assert(xuiIconDraw(painter,s.icon,(xui_rect_t){0,0,16,16},NULL)==XUI_OK);
        assert(xuiIconCategoryPreload(s.category,16,16)==XUI_OK);
    }
    xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount);
    assert(xuiPainterEnd(painter)==XUI_OK);backend.tProxy.surfaceDestroy(&backend.tProxy,target);
    xuiDestroy(s.context);assert(s.destroyed==1 && !s.entered && xrtGetError()==NULL);
}
int main(int argc,char** argv)
{
    if(argc==2){int route=atoi(argv[1]);assert(route>=0 && route<4);check(route/2,route%2,0,0,0);}
    else for(int operation=0;operation<4;++operation)
        for(int action=0;action<2;++action)
            for(int alias=0;alias<(operation==3 ? 1:2);++alias)
                for(int ambient=0;ambient<2;++ambient)
                    for(int error=0;error<2;++error)check(operation,action,alias,ambient,error);
    checkHeap();
    puts("Icon operations: 56 callback lifetime/error cases and 4000 zero-heap operations passed");return 0;
}
