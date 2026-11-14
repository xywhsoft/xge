#include "linux_startup_faults.h"
#include <EGL/egl.h>
#include <assert.h>
static unsigned initialized,surfaces,contexts,current;
EGLBoolean __real_eglBindAPI(EGLenum);
EGLDisplay __real_eglGetDisplay(EGLNativeDisplayType);
EGLBoolean __real_eglInitialize(EGLDisplay,EGLint*,EGLint*);
EGLBoolean __real_eglTerminate(EGLDisplay);
EGLBoolean __real_eglChooseConfig(EGLDisplay,const EGLint*,EGLConfig*,EGLint,EGLint*);
EGLBoolean __real_eglGetConfigAttrib(EGLDisplay,EGLConfig,EGLint,EGLint*);
EGLSurface __real_eglCreateWindowSurface(EGLDisplay,EGLConfig,EGLNativeWindowType,const EGLint*);
EGLBoolean __real_eglDestroySurface(EGLDisplay,EGLSurface);
EGLContext __real_eglCreateContext(EGLDisplay,EGLConfig,EGLContext,const EGLint*);
EGLBoolean __real_eglDestroyContext(EGLDisplay,EGLContext);
EGLBoolean __real_eglMakeCurrent(EGLDisplay,EGLSurface,EGLSurface,EGLContext);
void startup_egl_balanced(void){assert(!initialized && !surfaces && !contexts && !current);}
EGLBoolean __wrap_eglBindAPI(EGLenum api)
{return startup_fault==STARTUP_EGL_BIND?EGL_FALSE:__real_eglBindAPI(api);}
EGLDisplay __wrap_eglGetDisplay(EGLNativeDisplayType native)
{return startup_fault==STARTUP_EGL_DISPLAY?EGL_NO_DISPLAY:__real_eglGetDisplay(native);}
EGLBoolean __wrap_eglInitialize(EGLDisplay d,EGLint* major,EGLint* minor)
{if(startup_fault==STARTUP_EGL_INITIALIZE)return EGL_FALSE;EGLBoolean result=__real_eglInitialize(d,major,minor);if(result)++initialized;return result;}
EGLBoolean __wrap_eglTerminate(EGLDisplay d)
{assert(initialized);--initialized;return __real_eglTerminate(d);}
EGLBoolean __wrap_eglChooseConfig(EGLDisplay d,const EGLint* attrs,EGLConfig* configs,EGLint capacity,EGLint* count)
{
    if(startup_fault==STARTUP_EGL_CONFIG)return EGL_FALSE;
    if(startup_fault==STARTUP_EGL_EMPTY_CONFIG){*count=0;return EGL_TRUE;}
    if(startup_fault==STARTUP_EGL_CONFIG_OVERFLOW){*count=capacity+1;return EGL_TRUE;}
    return __real_eglChooseConfig(d,attrs,configs,capacity,count);
}
EGLBoolean __wrap_eglGetConfigAttrib(EGLDisplay d,EGLConfig config,EGLint name,EGLint* value)
{if(startup_fault==STARTUP_EGL_NATIVE_VISUAL && name==EGL_NATIVE_VISUAL_ID)return EGL_FALSE;return __real_eglGetConfigAttrib(d,config,name,value);}
EGLSurface __wrap_eglCreateWindowSurface(EGLDisplay d,EGLConfig config,EGLNativeWindowType window,const EGLint* attrs)
{if(startup_fault==STARTUP_EGL_SURFACE)return EGL_NO_SURFACE;EGLSurface result=__real_eglCreateWindowSurface(d,config,window,attrs);if(result!=EGL_NO_SURFACE)++surfaces;return result;}
EGLBoolean __wrap_eglDestroySurface(EGLDisplay d,EGLSurface surface)
{assert(surface!=EGL_NO_SURFACE && surfaces);--surfaces;return __real_eglDestroySurface(d,surface);}
EGLContext __wrap_eglCreateContext(EGLDisplay d,EGLConfig config,EGLContext shared,const EGLint* attrs)
{if(startup_fault==STARTUP_EGL_CONTEXT)return EGL_NO_CONTEXT;EGLContext result=__real_eglCreateContext(d,config,shared,attrs);if(result!=EGL_NO_CONTEXT)++contexts;return result;}
EGLBoolean __wrap_eglDestroyContext(EGLDisplay d,EGLContext context)
{assert(context!=EGL_NO_CONTEXT && contexts && !current);--contexts;return __real_eglDestroyContext(d,context);}
EGLBoolean __wrap_eglMakeCurrent(EGLDisplay d,EGLSurface draw,EGLSurface read,EGLContext context)
{if(startup_fault==STARTUP_EGL_CURRENT && context!=EGL_NO_CONTEXT)return EGL_FALSE;EGLBoolean result=__real_eglMakeCurrent(d,draw,read,context);if(result)current=context!=EGL_NO_CONTEXT;return result;}
