/* Capability failure must not replace a host's existing backend callbacks. */
#ifndef XRT_MODULE_ALL
#define XRT_MODULE_ALL
#endif
#if defined(_WIN32) && !defined(XRT_USE_SHARED)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct Symbol {const char* name;xge_host_xrt_proc proc;} Symbol;
static const Symbol symbols[]={
#define XGE_HOST_XRT_SLOT(Name) {#Name,(xge_host_xrt_proc)Name},
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
};
static xge_host_xrt_proc resolve(void* user,const char* name)
{(void)user;for(size_t n=0;n<sizeof(symbols)/sizeof(*symbols);++n)if(!strcmp(symbols[n].name,name))return symbols[n].proc;return NULL;}
static int inert(void* user){return user ? XGE_OK:XGE_ERROR_INVALID_ARGUMENT;}
static void inertVoid(void* user){assert(user);}
static void unchanged(const xge_platform_backend_t* platform,const xge_graphics_backend_t* graphics)
{
 xge_platform_backend_t p=xgePlatformBackendGet();xge_graphics_backend_t g=xgeGraphicsBackendGet();
 assert(p.iType==platform->iType && p.sName==platform->sName && p.init==platform->init && p.unit==platform->unit && p.poll==platform->poll && p.present==platform->present && p.pUser==platform->pUser);
 assert(g.iType==graphics->iType && g.sName==graphics->sName && g.init==graphics->init && g.unit==graphics->unit && g.begin==graphics->begin && g.end==graphics->end && g.pUser==graphics->pUser);
}
int main(void)
{
 static const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
 assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
 xge_egl_caps_t caps={0};assert(xgeEGLCapsGet(&caps)==XGE_OK && !caps.bCompiled);
 int marker=1;
 xge_platform_backend_t platform={XGE_PLATFORM_BACKEND_CUSTOM,"sentinel-platform",inert,inertVoid,inert,inertVoid,&marker};
 xge_graphics_backend_t graphics={XGE_GPU_BACKEND_CUSTOM,"sentinel-graphics",inert,inertVoid,inert,inert,&marker};
 assert(xgePlatformBackendSet(&platform)==XGE_OK && xgeGraphicsBackendSet(&graphics)==XGE_OK);
 for(int mode=0;mode<5;++mode){
  xge_egl_desc_t desc={0};desc.iWidth=desc.iHeight=1;
  if(mode==0)desc.bPBuffer=1;
  else if(mode==1)desc.bSurfaceless=1;
  else if(mode==2)desc.pNativeWindow=&marker;
  else if(mode==4)desc.iWidth=0;
  xge_egl_context_t context={0};int result=xgeEGLInit(&context,&desc);
  assert(result==(mode==4 ? XGE_ERROR_INVALID_ARGUMENT:XGE_ERROR_UNSUPPORTED));
  assert(!context.bInitialized && !xrtGetError());unchanged(&platform,&graphics);
  xgeEGLUnit(&context);unchanged(&platform,&graphics);
 }
 xge_platform_backend_t p=xgePlatformBackendDefault();xge_graphics_backend_t g=xgeGraphicsBackendDefault();
 assert(xgePlatformBackendSet(&p)==XGE_OK && xgeGraphicsBackendSet(&g)==XGE_OK);
 puts("Disabled EGL: five failed initialization modes preserve exact host backend tables and cleanup");return 0;
}
