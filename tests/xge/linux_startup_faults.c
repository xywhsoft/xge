#include "linux_startup_faults.h"
#include <GL/glx.h>
#include <GL/glxext.h>
#include <X11/Xutil.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
int startup_fault;
static unsigned libraries,displays,windows,colormaps,contexts,drawables,config_calls;
static void* gl_handle;
void* __real_dlopen(const char*,int);
void* __real_dlsym(void*,const char*);
int __real_dlclose(void*);
Display* __real_XOpenDisplay(const char*);
int __real_XCloseDisplay(Display*);
Colormap __real_XCreateColormap(Display*,Window,Visual*,int);
int __real_XFreeColormap(Display*,Colormap);
Window __real_XCreateWindow(Display*,Window,int,int,unsigned,unsigned,unsigned,int,unsigned,Visual*,unsigned long,XSetWindowAttributes*);
int __real_XDestroyWindow(Display*,Window);
XSizeHints* __real_XAllocSizeHints(void);
XVisualInfo* __real_XGetVisualInfo(Display*,long,XVisualInfo*,int*);
static PFNGLXGETFBCONFIGSPROC real_configs;
static PFNGLXGETFBCONFIGATTRIBPROC real_attrib;
static PFNGLXGETVISUALFROMFBCONFIGPROC real_visual;
static PFNGLXCREATECONTEXTATTRIBSARBPROC real_context;
typedef void(*DestroyContextFn)(Display*,GLXContext);
typedef Bool(*MakeCurrentFn)(Display*,GLXDrawable,GLXContext);
static DestroyContextFn real_destroy_context;
static PFNGLXCREATEWINDOWPROC real_drawable;
static PFNGLXDESTROYWINDOWPROC real_destroy_drawable;
static MakeCurrentFn real_current;
static __GLXextFuncPtr (*real_proc)(const GLubyte*);
void startup_fault_balanced_libraries(unsigned expected)
{if(libraries!=expected || displays || windows || colormaps || contexts || drawables)fprintf(stderr,"Startup resources: libraries=%u expected=%u displays=%u windows=%u colormaps=%u contexts=%u drawables=%u\n",libraries,expected,displays,windows,colormaps,contexts,drawables);assert(libraries==expected && !displays && !windows && !colormaps && !contexts && !drawables);}
void startup_fault_balanced(void){startup_fault_balanced_libraries(0);}
void startup_fault_reset(int value)
{startup_fault_balanced();startup_fault=value;config_calls=0;}
void* __wrap_dlopen(const char* path,int flags)
{
    bool gl=path && (!strcmp(path,"libGL.so") || !strcmp(path,"libGL.so.1"));
    if(gl && startup_fault==STARTUP_LIBGL)return NULL;
    void* result=__real_dlopen(path,flags);if(gl && result){gl_handle=result;++libraries;}return result;
}
int __wrap_dlclose(void* handle)
{if(handle==gl_handle){assert(libraries);if(!--libraries)gl_handle=NULL;}return __real_dlclose(handle);}
Display* __wrap_XOpenDisplay(const char* name)
{Display* result=__real_XOpenDisplay(name);if(result)++displays;return result;}
int __wrap_XCloseDisplay(Display* display)
{assert(display && displays);--displays;return __real_XCloseDisplay(display);}
Colormap __wrap_XCreateColormap(Display* d,Window w,Visual* v,int mode)
{if(startup_fault==STARTUP_COLORMAP)return 0;Colormap result=__real_XCreateColormap(d,w,v,mode);if(result)++colormaps;return result;}
int __wrap_XFreeColormap(Display* d,Colormap map)
{assert(map && colormaps);--colormaps;return __real_XFreeColormap(d,map);}
Window __wrap_XCreateWindow(Display* d,Window parent,int x,int y,unsigned width,unsigned height,unsigned border,int depth,unsigned cls,Visual* v,unsigned long mask,XSetWindowAttributes* attributes)
{
    if(startup_fault==STARTUP_WINDOW)return 0;
    if(startup_fault==STARTUP_ASYNC_WINDOW)return __real_XCreateWindow(d,parent,x,y,width,height,border,257,cls,v,mask,attributes);
    Window result=__real_XCreateWindow(d,parent,x,y,width,height,border,depth,cls,v,mask,attributes);if(result)++windows;return result;
}
int __wrap_XDestroyWindow(Display* d,Window w)
{assert(w && windows);--windows;return __real_XDestroyWindow(d,w);}
XSizeHints* __wrap_XAllocSizeHints(void)
{return startup_fault==STARTUP_HINTS ? NULL:__real_XAllocSizeHints();}
XVisualInfo* __wrap_XGetVisualInfo(Display* d,long mask,XVisualInfo* pattern,int* count)
{
    if(startup_fault==STARTUP_EGL_VISUAL){*count=0;return NULL;}
    XVisualInfo* result=__real_XGetVisualInfo(d,mask,pattern,count);
    if(startup_fault==STARTUP_EGL_EMPTY_VISUAL)*count=0;
    return result;
}
static Bool no_extension(Display* d,int* error,int* event){(void)d;(void)error;(void)event;return False;}
static Bool bad_query(Display* d,int* major,int* minor)
{(void)d;if(startup_fault==STARTUP_QUERY)return False;*major=1;*minor=2;return True;}
static GLXFBConfig* configs(Display* d,int screen,int* count)
{
    ++config_calls;
    if(startup_fault==STARTUP_CONFIGS || (startup_fault==STARTUP_SECOND_CONFIGS && config_calls==2)){*count=0;return NULL;}
    return real_configs(d,screen,count);
}
static int attrib(Display* d,GLXFBConfig config,int name,int* value)
{if(startup_fault==STARTUP_MATCH && name==GLX_RENDER_TYPE){*value=0;return 0;}return real_attrib(d,config,name,value);}
static XVisualInfo* visual(Display* d,GLXFBConfig config)
{return startup_fault==STARTUP_VISUAL ? NULL:real_visual(d,config);}
static GLXContext create_context(Display* d,GLXFBConfig config,GLXContext shared,Bool direct,const int* attrs)
{if(startup_fault==STARTUP_CONTEXT)return NULL;GLXContext result=real_context(d,config,shared,direct,attrs);if(result)++contexts;return result;}
static void destroy_context(Display* d,GLXContext context)
{assert(context && contexts);--contexts;real_destroy_context(d,context);}
static GLXWindow create_drawable(Display* d,GLXFBConfig config,Window window,const int* attrs)
{if(startup_fault==STARTUP_DRAWABLE)return 0;GLXWindow result=real_drawable(d,config,window,attrs);if(result)++drawables;return result;}
static void destroy_drawable(Display* d,GLXWindow window)
{assert(window && drawables);--drawables;real_destroy_drawable(d,window);}
static Bool make_current(Display* d,GLXDrawable window,GLXContext context)
{if(startup_fault==STARTUP_CURRENT && window)return False;return real_current(d,window,context);}
static __GLXextFuncPtr proc(const GLubyte* name)
{
    if(!strcmp((const char*)name,"glXCreateContextAttribsARB")){
        if(startup_fault==STARTUP_REQUIRED)return NULL;
        real_context=(PFNGLXCREATECONTEXTATTRIBSARBPROC)real_proc(name);return(__GLXextFuncPtr)create_context;
    }
    return real_proc(name);
}
void* __wrap_dlsym(void* handle,const char* name)
{
    if(startup_fault==STARTUP_ENTRY && !strcmp(name,"glXQueryVersion"))return NULL;
    void* result=__real_dlsym(handle,name);
    if(!strcmp(name,"glXQueryExtension") && startup_fault==STARTUP_EXTENSION)return(void*)no_extension;
    if(!strcmp(name,"glXQueryVersion") && (startup_fault==STARTUP_QUERY || startup_fault==STARTUP_VERSION))return(void*)bad_query;
    if(!strcmp(name,"glXGetFBConfigs")){real_configs=(PFNGLXGETFBCONFIGSPROC)result;return(void*)configs;}
    if(!strcmp(name,"glXGetFBConfigAttrib")){real_attrib=(PFNGLXGETFBCONFIGATTRIBPROC)result;return(void*)attrib;}
    if(!strcmp(name,"glXGetVisualFromFBConfig")){real_visual=(PFNGLXGETVISUALFROMFBCONFIGPROC)result;return(void*)visual;}
    if(!strcmp(name,"glXCreateWindow")){real_drawable=(PFNGLXCREATEWINDOWPROC)result;return(void*)create_drawable;}
    if(!strcmp(name,"glXDestroyWindow")){real_destroy_drawable=(PFNGLXDESTROYWINDOWPROC)result;return(void*)destroy_drawable;}
    if(!strcmp(name,"glXDestroyContext")){real_destroy_context=(DestroyContextFn)result;return(void*)destroy_context;}
    if(!strcmp(name,"glXMakeCurrent")){real_current=(MakeCurrentFn)result;return(void*)make_current;}
    if(!strcmp(name,"glXGetProcAddress") || !strcmp(name,"glXGetProcAddressARB")){real_proc=(__GLXextFuncPtr(*)(const GLubyte*))result;return(void*)proc;}
    return result;
}
