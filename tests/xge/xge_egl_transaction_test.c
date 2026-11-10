/* Real EGL pbuffer on an isolated display; typed link faults cover rollback. */
#ifndef XRT_MODULE_ALL
#define XRT_MODULE_ALL
#endif
#include <xge.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#include <EGL/egl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct Symbol {const char* name;xge_host_xrt_proc proc;} Symbol;
static const Symbol symbols[]={
#define XGE_HOST_XRT_SLOT(Name) {#Name,(xge_host_xrt_proc)Name},
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
};
static xge_host_xrt_proc resolve(void* user,const char* name)
{(void)user;for(size_t n=0;n<sizeof(symbols)/sizeof(*symbols);++n)if(!strcmp(symbols[n].name,name))return symbols[n].proc;return NULL;}
static int mode,terminated,surfaces,contexts;
EGLDisplay __real_eglGetDisplay(EGLNativeDisplayType);
EGLBoolean __real_eglInitialize(EGLDisplay,EGLint*,EGLint*);
EGLBoolean __real_eglBindAPI(EGLenum);
EGLBoolean __real_eglChooseConfig(EGLDisplay,const EGLint*,EGLConfig*,EGLint,EGLint*);
EGLSurface __real_eglCreatePbufferSurface(EGLDisplay,EGLConfig,const EGLint*);
EGLContext __real_eglCreateContext(EGLDisplay,EGLConfig,EGLContext,const EGLint*);
EGLBoolean __real_eglMakeCurrent(EGLDisplay,EGLSurface,EGLSurface,EGLContext);
EGLBoolean __real_eglDestroyContext(EGLDisplay,EGLContext);
EGLBoolean __real_eglDestroySurface(EGLDisplay,EGLSurface);
EGLBoolean __real_eglTerminate(EGLDisplay);
EGLDisplay __wrap_eglGetDisplay(EGLNativeDisplayType d)
{return mode==1 ? EGL_NO_DISPLAY:__real_eglGetDisplay(d);}
EGLBoolean __wrap_eglInitialize(EGLDisplay d,EGLint* a,EGLint* b)
{return mode==2 ? EGL_FALSE:__real_eglInitialize(d,a,b);}
EGLBoolean __wrap_eglBindAPI(EGLenum api)
{return mode==3 ? EGL_FALSE:__real_eglBindAPI(api);}
EGLBoolean __wrap_eglChooseConfig(EGLDisplay d,const EGLint* a,EGLConfig* c,EGLint count,EGLint* n)
{return mode==4 ? EGL_FALSE:__real_eglChooseConfig(d,a,c,count,n);}
EGLSurface __wrap_eglCreatePbufferSurface(EGLDisplay d,EGLConfig c,const EGLint* a)
{return mode==5 ? EGL_NO_SURFACE:__real_eglCreatePbufferSurface(d,c,a);}
EGLContext __wrap_eglCreateContext(EGLDisplay d,EGLConfig c,EGLContext shared,const EGLint* a)
{return mode==6 ? EGL_NO_CONTEXT:__real_eglCreateContext(d,c,shared,a);}
EGLBoolean __wrap_eglMakeCurrent(EGLDisplay d,EGLSurface a,EGLSurface b,EGLContext c)
{return mode==7 && c!=EGL_NO_CONTEXT ? EGL_FALSE:__real_eglMakeCurrent(d,a,b,c);}
EGLBoolean __wrap_eglDestroyContext(EGLDisplay d,EGLContext c)
{++contexts;EGLBoolean result=__real_eglDestroyContext(d,c);assert(result==EGL_TRUE);return result;}
EGLBoolean __wrap_eglDestroySurface(EGLDisplay d,EGLSurface s)
{++surfaces;EGLBoolean result=__real_eglDestroySurface(d,s);assert(result==EGL_TRUE);return result;}
EGLBoolean __wrap_eglTerminate(EGLDisplay d)
{++terminated;EGLBoolean result=__real_eglTerminate(d);assert(result==EGL_TRUE);return result;}
static int inert(void* user){return user ? XGE_OK:XGE_ERROR_INVALID_ARGUMENT;}
static void inertVoid(void* user){assert(user);}
int main(int argc,char** argv)
{
 assert(argc==2);mode=atoi(argv[1]);assert(mode>=0 && mode<=7);
 static const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
 assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
 xge_egl_caps_t caps={0};assert(xgeEGLCapsGet(&caps)==XGE_OK && caps.bCompiled && caps.bPBuffer);
 int marker=1;
 xge_platform_backend_t platform={XGE_PLATFORM_BACKEND_CUSTOM,"sentinel-platform",inert,inertVoid,inert,inertVoid,&marker};
 xge_graphics_backend_t graphics={XGE_GPU_BACKEND_CUSTOM,"sentinel-graphics",inert,inertVoid,inert,inert,&marker};
 assert(xgePlatformBackendSet(&platform)==XGE_OK && xgeGraphicsBackendSet(&graphics)==XGE_OK);
 xge_egl_desc_t desc={0};desc.iWidth=desc.iHeight=1;desc.bPBuffer=1;
 xge_egl_context_t context={0};int result=xgeEGLInit(&context,&desc);
 xge_platform_backend_t p=xgePlatformBackendGet();xge_graphics_backend_t g=xgeGraphicsBackendGet();
 if(!mode){
  assert(result==XGE_OK && context.bInitialized && p.iType==XGE_PLATFORM_BACKEND_EGL && g.iType==XGE_GPU_BACKEND_GLES30);
  assert(eglGetCurrentContext()==(EGLContext)context.pContext && !contexts && !surfaces && !terminated);
  printf("Actual EGL vendor/version: %s / %s\n",eglQueryString((EGLDisplay)context.pDisplay,EGL_VENDOR),eglQueryString((EGLDisplay)context.pDisplay,EGL_VERSION));
  xgeEGLUnit(&context);assert(eglGetCurrentContext()==EGL_NO_CONTEXT && contexts==1 && surfaces==1 && terminated==1);
 }else{
  assert(result==XGE_ERROR_BACKEND_FAILED && !context.bInitialized);
  assert(p.iType==platform.iType && p.sName==platform.sName && p.init==platform.init && p.unit==platform.unit && p.poll==platform.poll && p.present==platform.present && p.pUser==platform.pUser);
  assert(g.iType==graphics.iType && g.sName==graphics.sName && g.init==graphics.init && g.unit==graphics.unit && g.begin==graphics.begin && g.end==graphics.end && g.pUser==graphics.pUser);
  assert(terminated==(mode>=3) && surfaces==(mode>=6) && contexts==(mode>=7));
  xgeEGLUnit(&context);
  assert(terminated==(mode>=3) && surfaces==(mode>=6) && contexts==(mode>=7));
 }
 assert(!context.bInitialized && !xrtGetError());
 puts("EGL transaction: exact backend commit/rollback and real context/surface/display cleanup passed");return 0;
}
