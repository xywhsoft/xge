/* Isolated trees: siblings cannot accidentally force another prepare pass. */
/* The constructor chain destroys only successful init stages. Radio must
 * clean its own partial text when init fails. Exercise real constructors. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include "xui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "tests/xui/xui_test_proxy.c"
#if defined(XGE_TEST_STATIC)
#include <xge_static_xrt_contract.h>
#else
#include <xge_host_xrt_contract.h>
static xge_host_xrt_proc resolve(void* user,const char* name)
{
 (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
 return NULL;
}
#endif

#include <stdlib.h>
static xui_draw_surface_proc originalDraw;
static xui_surface atlas;
static int indicatorDraws;
static int counted(xui_proxy proxy,xui_draw_context draw,xui_surface image,xui_rect_t src,xui_rect_t dst,uint32_t color,uint32_t flags)
{if(image==atlas)++indicatorDraws;return originalDraw(proxy,draw,image,src,dst,color,flags);}
int main(int argc,char**argv)
{
#if defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
 assert(xrtMemDebugEnable(true));
 assert(argc==2);int kind=atoi(argv[1]);assert(kind>=0&&kind<4);
 static xui_test_proxy_state_t state;xuiTestProxyInit(&state);originalDraw=state.tProxy.drawSurface;state.tProxy.drawSurface=counted;
 xui_context ui=NULL;xui_widget root=NULL,child=NULL;xui_surface target=NULL;xui_font font=NULL;const xui_rect_t bounds={0,0,220,80},src={0,0,16,16};const xui_rect_i_t dirty={0,0,220,80};
 assert(xuiCreate(&ui)==XUI_OK&&xuiSetProxy(ui,&state.tProxy)==XUI_OK);
 assert(state.tProxy.fontLoadMemory(&state.tProxy,&font,"font",4,16,XUI_FONT_FORMAT_TTF)==XUI_OK&&xuiSetDefaultFont(ui,font)==XUI_OK);
 assert(xuiWidgetCreate(ui,&root)==XUI_OK&&xuiWidgetSetRect(root,bounds)==XUI_OK&&xuiSetRootWidget(ui,root)==XUI_OK);
 int status=kind==0?xuiButtonCreate(ui,&child,NULL):kind==1?xuiCheckBoxCreate(ui,&child,NULL):kind==2?xuiToggleCreate(ui,&child,NULL):xuiRadioCreate(ui,&child,NULL);
 assert(status==XUI_OK&&xuiWidgetAddChild(root,child)==XUI_OK&&xuiWidgetSetRect(child,(xui_rect_t){0,0,160,32})==XUI_OK);
 assert(xuiTestSurfaceCreate(&state,&atlas,32,32,0)==XUI_OK&&xuiTestSurfaceCreate(&state,&target,220,80,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
 status=kind==0?xuiButtonSetIcon(child,atlas,src):kind==1?xuiCheckBoxSetIndicatorSurface(child,atlas,src,atlas,src):kind==2?xuiToggleSetIndicatorSurface(child,atlas,src,atlas,src):xuiRadioSetIndicatorSurface(child,atlas,src,atlas,src);assert(status==XUI_OK);
 assert(xuiLayout(ui)==XUI_OK&&xuiRender(ui,target,&dirty,1)==XUI_OK&&xuiRenderPrepare(ui)==XUI_OK&&xuiRender(ui,target,&dirty,1)==XUI_OK);
 xuiWidgetClearDirty(child,XUI_WIDGET_DIRTY_ALL);xuiWidgetClearDirty(root,XUI_WIDGET_DIRTY_ALL);indicatorDraws=0;
 assert(xuiRenderPrepare(ui)==XUI_OK&&indicatorDraws==0);
 assert(state.tProxy.surfaceUpdateRGBA(&state.tProxy,atlas,(xui_rect_i_t){0,0,1,1},NULL,0)==XUI_OK&&xuiRenderPrepare(ui)==XUI_OK);
 int refreshed=indicatorDraws>0;printf("kind=%d clean-cache generation refresh=%d draws=%d\n",kind,refreshed,indicatorDraws);
 assert(xuiDestroyChecked(ui)==XUI_OK);state.tProxy.surfaceDestroy(&state.tProxy,target);state.tProxy.surfaceDestroy(&state.tProxy,atlas);state.tProxy.fontDestroy(&state.tProxy,font);xrtClearError();xmemdebugsnapshot remaining;xrtMemDebugSnapshot(&remaining);assert(!remaining.LiveCount&&!remaining.InvalidFreeCount&&!remaining.DoubleFreeCount);return refreshed?0:3;
}
