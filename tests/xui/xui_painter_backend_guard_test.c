/* A foreign owned resource must never reach a backend that expects another
 * capability. The old-engine guard exits before interpreting any payload. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct painter_guard {xui_test_proxy_state_t backend;unsigned refs,calls;} painter_guard;
static bool hold(const void* raw){++((painter_guard*)raw)->refs;return true;}
static bool drop(const void* raw){painter_guard* s=(painter_guard*)raw;assert(s->refs>1);--s->refs;return true;}
static bool count(const void* raw,size_t* output){*output=((const painter_guard*)raw)->refs;return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user)
{(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops operations={count,trace};
static int finish(xui_proxy proxy,void* raw){(void)proxy;(void)raw;return XUI_OK;}
static int begin(xui_proxy proxy,xui_draw_context* output,xui_surface target)
{void* payload=NULL;return xuiDrawCreateOwned(proxy,target,0,finish,output,&payload);}
static int end(xui_proxy proxy,xui_draw_context draw){(void)proxy;return xuiDrawReleaseChecked(draw);}
static void check(xui_proxy proxy,xui_proxy actual)
{
    if(!actual || actual->_pCapability!=proxy->_pCapability){
        puts("FOREIGN_RESOURCE_REACHED_PAINTER_BACKEND");fflush(stdout);_Exit(84);
    }
    ++((painter_guard*)proxy->pUser)->calls;
}
static int surface(xui_proxy proxy,xui_draw_context draw,xui_surface value,xui_rect_t from,xui_rect_t to,uint32_t color,uint32_t flags)
{(void)draw;(void)from;(void)to;(void)color;(void)flags;check(proxy,xuiSurfaceGetProxy(value));return XUI_OK;}
static int quad(xui_proxy proxy,xui_draw_context draw,xui_surface value,const xui_surface_vertex_t* vertices,uint32_t flags)
{(void)draw;(void)vertices;(void)flags;check(proxy,xuiSurfaceGetProxy(value));return XUI_OK;}
static int text(xui_proxy proxy,xui_draw_context draw,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags)
{(void)draw;(void)rect;(void)color;(void)flags;check(proxy,xuiFontGetProxy(item->pFont));return XUI_OK;}
static void run(int operation,bool same)
{
    painter_guard state={0};state.refs=1;xuiTestProxyInit(&state.backend);
    state.backend.tProxy.drawBegin=begin;state.backend.tProxy.drawEnd=end;
    state.backend.tProxy.drawSurface=surface;state.backend.tProxy.drawSurfaceQuad=quad;state.backend.tProxy.drawText=text;
    xui_callback_owner_t owner={{&state,&operations},hold,drop};xui_proxy a=NULL,b=NULL;
    assert(xuiProxyCreateOwned(&state.backend.tProxy,&owner,&a)==XUI_OK);
    assert(xuiProxyCreateOwned(&state.backend.tProxy,&owner,&b)==XUI_OK);
    xui_context_desc_t descriptor;xuiContextDescDefault(&descriptor);descriptor.iSystemFontPolicy=XUI_SYSTEM_FONT_DISABLED;
    xui_context context=NULL;assert(xuiCreateOwnedEx(a,&descriptor,&context)==XUI_OK);
    xui_surface target=NULL,image=NULL;xui_font font=NULL;void* payload=NULL;
    assert(xuiSurfaceCreateOwned(a,0,finish,&target,&payload)==XUI_OK);
    assert(xuiSurfaceCreateOwned(same?a:b,0,finish,&image,&payload)==XUI_OK);
    assert(xuiFontCreateOwned(same?a:b,0,finish,&font,&payload)==XUI_OK);
    xui_call_scope_t scope={0};xui_painter painter=NULL;
    assert(xuiCallScopeBegin(context,&scope)==XUI_OK && xuiPainterBegin(context,target,&painter)==XUI_OK);
    xui_rect_t rect={0,0,32,32};int status;
    if(operation==0)status=xuiPainterDrawSurface(painter,image,rect,rect,XUI_COLOR_WHITE,0);
    else if(operation==1){xui_surface_vertex_t vertices[4]={{0}};for(int i=0;i<4;++i)vertices[i].iColor=XUI_COLOR_WHITE;status=xuiPainterDrawSurfaceQuad(painter,image,vertices,0);}
    else if(operation==2)status=xuiPainterDrawText(painter,font,"text",rect,XUI_COLOR_WHITE,0);
    else status=xuiPainterDrawNinePatch(painter,image,rect,rect,(xui_thickness_t){4,4,4,4},XUI_COLOR_WHITE,0);
    assert(status==(same?XUI_OK:XUI_ERROR_INVALID_ARGUMENT));assert((state.calls>0)==same);
    assert(xuiPainterEnd(painter)==XUI_OK && xuiCallScopeEnd(&scope)==XUI_OK);
    assert(xuiDestroyChecked(context)==XUI_OK && xuiSurfaceReleaseChecked(target)==XUI_OK);
    assert(xuiSurfaceReleaseChecked(image)==XUI_OK && xuiFontReleaseChecked(font)==XUI_OK);
    assert(xuiProxyReleaseChecked(a)==XUI_OK && xuiProxyReleaseChecked(b)==XUI_OK && state.refs==1);
}
int main(int argc,char** argv)
{
    if(argc==2)run(atoi(argv[1]),false);
    else for(int operation=0;operation<4;++operation){run(operation,true);run(operation,false);}
    puts("Painter backend: four valid resources and four foreign capability rejections passed");return 0;
}
