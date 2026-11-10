/* Matched SDK feature/capability ABI and actual raw EGL/GLES pixel readback.
 * This does not assert that the XGE renderer is attached to the raw context. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#if defined(XGE_TEST_STATIC)
#include <xge_static_xrt_contract.h>
#else
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if XGE_ENABLE_EGL
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "xge_egl_fault_preload.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
static Display* nativeDisplay;
static Window nativeWindow;
static Colormap nativeColormap;
static void prepareWindow(xge_egl_desc_t* desc)
{
    nativeDisplay=XOpenDisplay(NULL);assert(nativeDisplay);
    EGLDisplay display=eglGetDisplay((EGLNativeDisplayType)nativeDisplay);assert(display!=EGL_NO_DISPLAY);
    assert(eglInitialize(display,NULL,NULL)==EGL_TRUE);
    const EGLint attributes[]={EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_ES3_BIT,
        EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_DEPTH_SIZE,16,EGL_STENCIL_SIZE,0,EGL_NONE};
    EGLConfig config;EGLint count=0,visualId=0;
    assert(eglChooseConfig(display,attributes,&config,1,&count)==EGL_TRUE && count>0);
    assert(eglGetConfigAttrib(display,config,EGL_NATIVE_VISUAL_ID,&visualId)==EGL_TRUE);
    XVisualInfo query={0};query.visualid=(VisualID)visualId;int visuals=0;
    XVisualInfo* visual=XGetVisualInfo(nativeDisplay,VisualIDMask,&query,&visuals);assert(visual && visuals>0);
    Window root=RootWindow(nativeDisplay,visual->screen);
    nativeColormap=XCreateColormap(nativeDisplay,root,visual->visual,AllocNone);assert(nativeColormap);
    XSetWindowAttributes options={0};options.colormap=nativeColormap;
    nativeWindow=XCreateWindow(nativeDisplay,root,0,0,16,16,0,visual->depth,InputOutput,visual->visual,
        CWColormap|CWBorderPixel|CWBackPixel,&options);assert(nativeWindow);
    XFree(visual);XMapWindow(nativeDisplay,nativeWindow);XSync(nativeDisplay,False);
    desc->pNativeDisplay=nativeDisplay;desc->pNativeWindow=(void*)(uintptr_t)nativeWindow;desc->bPBuffer=0;
}
static void closeWindow(void)
{
    if(!nativeDisplay)return;
    XDestroyWindow(nativeDisplay,nativeWindow);XFreeColormap(nativeDisplay,nativeColormap);XCloseDisplay(nativeDisplay);
}
static void pixels(void)
{
    /* Surfaceless has no default framebuffer. Use a real typed GLES FBO in
     * both modes so the same clear/readback workload is verified. */
    GLuint texture=0,framebuffer=0;unsigned char pixel[4]={0};
    glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
    glGenFramebuffers(1,&framebuffer);glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);
    glViewport(0,0,1,1);glClearColor(17.0f/255,34.0f/255,51.0f/255,1);glClear(GL_COLOR_BUFFER_BIT);
    glReadPixels(0,0,1,1,GL_RGBA,GL_UNSIGNED_BYTE,pixel);
    assert(pixel[0]==17 && pixel[1]==34 && pixel[2]==51 && pixel[3]==255);
    glBindFramebuffer(GL_FRAMEBUFFER,0);glDeleteFramebuffers(1,&framebuffer);glDeleteTextures(1,&texture);
    assert(glGetError()==GL_NO_ERROR);
}
#endif
#if !defined(XGE_TEST_STATIC)
static xge_host_xrt_proc resolve(void* unused,const char* name)
{
    (void)unused;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif
static int inert(void* user){return user ? XGE_OK:XGE_ERROR_INVALID_ARGUMENT;}
static void inertVoid(void* user){assert(user);}
int main(int argc,char** argv)
{
    assert(argc==2);int mode=atoi(argv[1]);assert(mode>=0 && mode<=10);
#if !defined(XGE_TEST_STATIC)
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
#else
    const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;
    assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#endif
    xge_build_info_t build={0};build.size=sizeof(build);
    assert(xgeGetBuildInfo(&build)==XGE_OK && build.features==XGE_FEATURES);
    assert(xgeGetBuildFeatures()==XGE_FEATURES && ((build.features>>10)&1)==XGE_ENABLE_EGL);
    assert(xgeCheckBuildCompatibility(XGE_BUILD_ABI_FINGERPRINT,XGE_FEATURES)==XGE_OK);
    assert(xgeCheckBuildCompatibility(XGE_BUILD_ABI_FINGERPRINT,XGE_FEATURES^(1u<<10))==XGE_ERROR_UNSUPPORTED);
    xge_egl_caps_t caps={0};assert(xgeEGLCapsGet(&caps)==XGE_OK);
    assert(caps.bCompiled==XGE_ENABLE_EGL && caps.bPBuffer==XGE_ENABLE_EGL &&
        caps.bSurfaceless==XGE_ENABLE_EGL && caps.bNativeWindow==XGE_ENABLE_EGL);
    int marker=1;
    xge_platform_backend_t platform={XGE_PLATFORM_BACKEND_CUSTOM,"sentinel-platform",inert,inertVoid,inert,inertVoid,&marker};
    xge_graphics_backend_t graphics={XGE_GPU_BACKEND_CUSTOM,"sentinel-graphics",inert,inertVoid,inert,inert,&marker};
    assert(xgePlatformBackendSet(&platform)==XGE_OK && xgeGraphicsBackendSet(&graphics)==XGE_OK);
    xge_egl_desc_t desc={0};desc.iWidth=desc.iHeight=1;desc.bPBuffer=mode!=8;desc.bSurfaceless=mode==8;
#if XGE_ENABLE_EGL
    if(mode>=9)prepareWindow(&desc);
    xgeEglFaultSet(mode==8 || mode==9 ? 0:mode);
#endif
    xge_egl_context_t context={0};int result=xgeEGLInit(&context,&desc);
    xge_platform_backend_t p=xgePlatformBackendGet();xge_graphics_backend_t g=xgeGraphicsBackendGet();
#if XGE_ENABLE_EGL
    int contexts=0,surfaces=0,terminated=0;xgeEglFaultCounts(&contexts,&surfaces,&terminated);
    if(mode==0 || mode==8 || mode==9){
        assert(result==XGE_OK && context.bInitialized && context.bCleanupPending && p.iType==XGE_PLATFORM_BACKEND_EGL && g.iType==XGE_GPU_BACKEND_GLES30);
        assert(eglGetCurrentContext()==(EGLContext)context.pContext && !contexts && !surfaces && !terminated);
        printf("Actual EGL vendor/version: %s / %s; GLES: %s\n",eglQueryString((EGLDisplay)context.pDisplay,EGL_VENDOR),eglQueryString((EGLDisplay)context.pDisplay,EGL_VERSION),glGetString(GL_VERSION));
        pixels();assert(xgeEGLMakeCurrent(&context)==XGE_OK);
        if(mode==9)assert(eglSwapBuffers((EGLDisplay)context.pDisplay,(EGLSurface)context.pSurface)==EGL_TRUE);
        xgeEGLUnit(&context);
        xgeEglFaultCounts(&contexts,&surfaces,&terminated);
        assert(eglGetCurrentContext()==EGL_NO_CONTEXT && contexts==1 && surfaces==(mode!=8) && terminated==1);
    }else{
        assert(result==XGE_ERROR_BACKEND_FAILED && !context.bInitialized && !context.bCleanupPending);
        assert(terminated==(mode>=3) && surfaces==(mode>=6 && mode!=10) && contexts==(mode==7));
#else
    assert(result==XGE_ERROR_UNSUPPORTED && !context.bInitialized);
    {
#endif
        assert(p.iType==platform.iType && p.sName==platform.sName && p.init==platform.init && p.unit==platform.unit && p.poll==platform.poll && p.present==platform.present && p.pUser==platform.pUser);
        assert(g.iType==graphics.iType && g.sName==graphics.sName && g.init==graphics.init && g.unit==graphics.unit && g.begin==graphics.begin && g.end==graphics.end && g.pUser==graphics.pUser);
        xgeEGLUnit(&context);
#if XGE_ENABLE_EGL
        xgeEglFaultCounts(&contexts,&surfaces,&terminated);assert(terminated==(mode>=3) && surfaces==(mode>=6 && mode!=10) && contexts==(mode==7));
#endif
    }
#if XGE_ENABLE_EGL
    closeWindow();
#endif
    assert(!context.bInitialized && !context.bCleanupPending && !xrtGetError());puts("Matched EGL SDK capabilities, raw context transaction and pixel checks passed");return 0;
}
