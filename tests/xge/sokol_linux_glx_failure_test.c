/* Actual vendored Sokol; only platform entry points are failed by the linker. */
#define _POSIX_C_SOURCE 200809L
#define SOKOL_IMPL
#define SOKOL_NO_ENTRY
#ifndef SOKOL_GLES3
#define SOKOL_GLCORE
#endif
#ifdef XGE_TEST_EGL
#define SOKOL_FORCE_EGL
#endif
#include "extlibs/sokol/sokol_app.h"
#include "linux_startup_faults.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static unsigned allocations,frees,panics,inits,frames,cleanups,application_errors;
static int allowance=-1;
static uint32_t expected;
static void* allocate(size_t size,void* user)
{(void)user;if(allowance==0)return NULL;if(allowance>0)--allowance;void* p=malloc(size);assert(p);++allocations;return p;}
static void release(void* p,void* user){(void)user;if(p){++frees;free(p);}}
static int app_error(Display* display,XErrorEvent* error){(void)display;(void)error;++application_errors;return 0;}
static void log_message(const char* tag,uint32_t level,uint32_t item,const char* message,uint32_t line,const char* file,void* user)
{(void)tag;(void)line;(void)file;(void)user;if(!level){
    if(item!=expected)fprintf(stderr,"Unexpected fatal: stage=%d item=%u expected=%u message=%s panics=%u frames=%u\n",startup_fault,item,expected,message?message:"",panics,frames);
    assert(item==expected);++panics;
}}
static void initialize(void)
{
    ++inits;uint32_t pixels[16];for(unsigned i=0;i<16;++i)pixels[i]=0xFF336699;
    sapp_image_desc image;memset(&image,0,sizeof(image));image.width=image.height=4;
    image.pixels.ptr=pixels;image.pixels.size=sizeof(pixels);
    assert(sapp_bind_mouse_cursor_image(SAPP_MOUSECURSOR_CROSSHAIR,&image)==SAPP_MOUSECURSOR_CROSSHAIR);
    /* Leave it bound: platform teardown must release it before XCloseDisplay. */
}
static void frame(void){++frames;sapp_quit();}
static void cleanup(void){++cleanups;}
static uint32_t log_item(int mode)
{
    switch(mode){
    case STARTUP_EGL_BIND:
#ifdef SOKOL_GLES3
        return SAPP_LOGITEM_LINUX_EGL_BIND_OPENGL_ES_API_FAILED;
#else
        return SAPP_LOGITEM_LINUX_EGL_BIND_OPENGL_API_FAILED;
#endif
    case STARTUP_EGL_DISPLAY:return SAPP_LOGITEM_LINUX_EGL_GET_DISPLAY_FAILED;
    case STARTUP_EGL_INITIALIZE:return SAPP_LOGITEM_LINUX_EGL_INITIALIZE_FAILED;
    case STARTUP_EGL_CONFIG:case STARTUP_EGL_EMPTY_CONFIG:case STARTUP_EGL_CONFIG_OVERFLOW:return SAPP_LOGITEM_LINUX_EGL_NO_CONFIGS;
    case STARTUP_EGL_NATIVE_VISUAL:return SAPP_LOGITEM_LINUX_EGL_NO_NATIVE_VISUAL;
    case STARTUP_EGL_VISUAL:case STARTUP_EGL_EMPTY_VISUAL:return SAPP_LOGITEM_LINUX_EGL_GET_VISUAL_INFO_FAILED;
    case STARTUP_EGL_SURFACE:return SAPP_LOGITEM_LINUX_EGL_CREATE_WINDOW_SURFACE_FAILED;
    case STARTUP_EGL_CONTEXT:return SAPP_LOGITEM_LINUX_EGL_CREATE_CONTEXT_FAILED;
    case STARTUP_EGL_CURRENT:return SAPP_LOGITEM_LINUX_EGL_MAKE_CURRENT_FAILED;
    case STARTUP_LIBGL:return SAPP_LOGITEM_LINUX_GLX_LOAD_LIBGL_FAILED;
    case STARTUP_ENTRY:return SAPP_LOGITEM_LINUX_GLX_LOAD_ENTRY_POINTS_FAILED;
    case STARTUP_EXTENSION:return SAPP_LOGITEM_LINUX_GLX_EXTENSION_NOT_FOUND;
    case STARTUP_QUERY:return SAPP_LOGITEM_LINUX_GLX_QUERY_VERSION_FAILED;
    case STARTUP_VERSION:return SAPP_LOGITEM_LINUX_GLX_VERSION_TOO_LOW;
    case STARTUP_CONFIGS:case STARTUP_SECOND_CONFIGS:return SAPP_LOGITEM_LINUX_GLX_NO_GLXFBCONFIGS;
    case STARTUP_ALLOCATION:case STARTUP_HINTS:return SAPP_LOGITEM_MALLOC_FAILED;
    case STARTUP_MATCH:return SAPP_LOGITEM_LINUX_GLX_NO_SUITABLE_GLXFBCONFIG;
    case STARTUP_VISUAL:return SAPP_LOGITEM_LINUX_GLX_GET_VISUAL_FROM_FBCONFIG_FAILED;
    case STARTUP_REQUIRED:return SAPP_LOGITEM_LINUX_GLX_REQUIRED_EXTENSIONS_MISSING;
    case STARTUP_CONTEXT:return SAPP_LOGITEM_LINUX_GLX_CREATE_CONTEXT_FAILED;
    case STARTUP_DRAWABLE:return SAPP_LOGITEM_LINUX_GLX_CREATE_WINDOW_FAILED;
    case STARTUP_CURRENT:
#ifdef XGE_TEST_LEGACY_SOKOL
        return SAPP_LOGITEM_LINUX_GLX_CREATE_CONTEXT_FAILED;
#else
        return SAPP_LOGITEM_LINUX_GLX_MAKE_CURRENT_FAILED;
#endif
    default:return SAPP_LOGITEM_LINUX_X11_CREATE_WINDOW_FAILED;
    }
}
int main(int argc,char** argv)
{
    assert(argc==2);int mode=atoi(argv[1]);assert((mode>0 && mode<=STARTUP_SECOND_CONFIGS)||(mode>=STARTUP_EGL_BIND && mode<=STARTUP_EGL_EMPTY_VISUAL));
    sapp_desc desc;memset(&desc,0,sizeof(desc));desc.width=128;desc.height=96;desc.window_title="Sokol Linux graphics startup";
    desc.enable_clipboard=true;desc.enable_dragndrop=true;desc.allocator.alloc_fn=allocate;desc.allocator.free_fn=release;
    desc.logger.func=log_message;desc.init_cb=initialize;desc.frame_cb=frame;desc.cleanup_cb=cleanup;
    XErrorHandler original=XSetErrorHandler(app_error);expected=log_item(mode);
    for(unsigned attempt=0;attempt<3;++attempt){
        startup_fault_reset(mode);allowance=mode==STARTUP_ALLOCATION?2:-1;sapp_run(&desc);allowance=-1;
        assert(!sapp_isvalid() && !inits && !frames && !cleanups && panics==attempt+1);
        assert(allocations==frees && !application_errors);startup_fault_balanced();
#ifdef XGE_TEST_EGL
        startup_egl_balanced();
#endif
        XErrorHandler prior=XSetErrorHandler(app_error);assert(prior==app_error);
    }
    startup_fault_reset(STARTUP_HEALTHY);sapp_run(&desc);
    assert(inits==1 && frames==1 && cleanups==1 && panics==3 && !application_errors);
    assert(allocations==frees && !sapp_isvalid());startup_fault_balanced();
#ifdef XGE_TEST_EGL
    startup_egl_balanced();
#endif
    assert(XSetErrorHandler(original)==app_error);
    printf("Sokol Linux graphics stage %d: three balanced failures, handler restoration and same-process window retry passed\n",mode);
    return 0;
}
