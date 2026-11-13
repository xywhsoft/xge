/* Cache preparation is best-effort, but a checked frame receipt must retain
 * the first failed callback status, including a NULL diagnostic. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdio.h>
typedef struct cache_state {
    xui_context context;xui_lifetime_link_t link;
    int action,diagnostic,tail,calls,reports,gone;
    xerror* expected;
} cache_state;
static void gone(void* raw){++((cache_state*)raw)->gone;}
static int render(xui_widget widget,xui_draw_context draw,uint32_t state,void* raw)
{
    cache_state* s=raw;(void)draw;(void)state;++s->calls;
    if(s->action==1)assert(xuiWidgetDestroyChecked(widget)==XUI_OK);
    if(s->action==2)xuiDestroy(s->context);
    if(s->diagnostic){xrtSetErrorKind(XERR_TYPE);s->expected=xrtErrorRef(xrtGetError());assert(s->expected);}
    return XUI_ERROR_BACKEND_FAILED;
}
static void report(xui_context context,const xui_error_info_t* error,void* raw)
{
    cache_state* s=raw;assert(context==s->context && error);++s->reports;
    if(s->tail)xrtSetErrorKind(XERR_RANGE);
}
static void check(int action,int diagnostic,int ambientPresent,int tail)
{
    cache_state s={0};s.action=action;s.diagnostic=diagnostic;s.tail=tail;
    xui_test_proxy_state_t backend;xuiTestProxyInit(&backend);xui_widget widget=NULL;xui_surface target=NULL;
    assert(xrtGetError()==NULL && xuiCreate(&s.context)==XUI_OK && xuiSetProxy(s.context,&backend.tProxy)==XUI_OK);
    assert(xuiContextLifetimeWatch(s.context,&s.link,gone,&s)==XUI_OK);
    assert(xuiWidgetCreate(s.context,&widget)==XUI_OK && xuiWidgetSetRect(widget,(xui_rect_t){0,0,64,64})==XUI_OK);
    assert(xuiSetRootWidget(s.context,widget)==XUI_OK && xuiWidgetSetCacheRenderCallback(widget,render,&s)==XUI_OK);
    assert(xuiSetErrorCallback(s.context,report,&s)==XUI_OK);
    assert(xuiInputViewport(s.context,64,64)==XUI_OK && xuiTestSurfaceCreate(&backend,&target,64,64,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
    xerror* ambient=NULL;
    if(ambientPresent){xrtSetErrorKind(XERR_RANGE);ambient=xrtErrorRef(xrtGetError());assert(ambient);}
    xui_call_scope_t receipt={0};assert(xuiCallScopeBegin(s.context,&receipt)==XUI_OK);
    xui_rect_i_t rect={0,0,64,64};int raw=xuiRender(s.context,target,&rect,1);
    assert(raw==XUI_OK && s.calls==1);
    assert(xuiCallScopeEnd(&receipt)==XUI_ERROR_BACKEND_FAILED);
    assert(xrtGetError()==s.expected);
    if(!action)assert(s.reports>0);
    xrtClearError();xrtErrorFree(s.expected);xrtErrorFree(ambient);
    if(!s.gone)xuiDestroy(s.context);
    assert(s.gone==1);backend.tProxy.surfaceDestroy(&backend.tProxy,target);xrtClearError();
}
int main(void)
{
    for(int action=0;action<3;++action)for(int diagnostic=0;diagnostic<2;++diagnostic)
        for(int ambient=0;ambient<2;++ambient)for(int tail=0;tail<2;++tail)check(action,diagnostic,ambient,tail);
    puts("Cache render outcomes: 24 status/NULL/diagnostic/ambient/close/tail cases passed");return 0;
}
