/* Typed, explicitly linked ELF interposition for the real engine shared image.
 * Test-only controls; no substitute engine translation unit or graphics mocks. */
#include <EGL/egl.h>
#include <assert.h>
#include <dlfcn.h>
#include <string.h>
#include "xge_egl_fault_preload.h"
static int mode,contexts,surfaces,terminated;
void xgeEglFaultSet(int value){mode=value;contexts=surfaces=terminated=0;}
void xgeEglFaultCounts(int* c,int* s,int* t){*c=contexts;*s=surfaces;*t=terminated;}
#define REAL(Name,Type) Type real;do{void* p=dlsym(RTLD_NEXT,#Name);assert(p && sizeof(real)==sizeof(p));memcpy(&real,&p,sizeof(real));}while(0)
EGLDisplay eglGetDisplay(EGLNativeDisplayType d)
{typedef EGLDisplay (*Fn)(EGLNativeDisplayType);REAL(eglGetDisplay,Fn);return mode==1 ? EGL_NO_DISPLAY:real(d);}
EGLBoolean eglInitialize(EGLDisplay d,EGLint* a,EGLint* b)
{typedef EGLBoolean (*Fn)(EGLDisplay,EGLint*,EGLint*);REAL(eglInitialize,Fn);return mode==2 ? EGL_FALSE:real(d,a,b);}
EGLBoolean eglBindAPI(EGLenum api)
{typedef EGLBoolean (*Fn)(EGLenum);REAL(eglBindAPI,Fn);return mode==3 ? EGL_FALSE:real(api);}
EGLBoolean eglChooseConfig(EGLDisplay d,const EGLint* a,EGLConfig* c,EGLint count,EGLint* n)
{typedef EGLBoolean (*Fn)(EGLDisplay,const EGLint*,EGLConfig*,EGLint,EGLint*);REAL(eglChooseConfig,Fn);return mode==4 ? EGL_FALSE:real(d,a,c,count,n);}
EGLSurface eglCreatePbufferSurface(EGLDisplay d,EGLConfig c,const EGLint* a)
{typedef EGLSurface (*Fn)(EGLDisplay,EGLConfig,const EGLint*);REAL(eglCreatePbufferSurface,Fn);return mode==5 ? EGL_NO_SURFACE:real(d,c,a);}
EGLSurface eglCreateWindowSurface(EGLDisplay d,EGLConfig c,EGLNativeWindowType w,const EGLint* a)
{typedef EGLSurface (*Fn)(EGLDisplay,EGLConfig,EGLNativeWindowType,const EGLint*);REAL(eglCreateWindowSurface,Fn);return mode==10 ? EGL_NO_SURFACE:real(d,c,w,a);}
EGLContext eglCreateContext(EGLDisplay d,EGLConfig c,EGLContext shared,const EGLint* a)
{typedef EGLContext (*Fn)(EGLDisplay,EGLConfig,EGLContext,const EGLint*);REAL(eglCreateContext,Fn);return mode==6 ? EGL_NO_CONTEXT:real(d,c,shared,a);}
EGLBoolean eglMakeCurrent(EGLDisplay d,EGLSurface a,EGLSurface b,EGLContext c)
{typedef EGLBoolean (*Fn)(EGLDisplay,EGLSurface,EGLSurface,EGLContext);REAL(eglMakeCurrent,Fn);return mode==7 && c!=EGL_NO_CONTEXT ? EGL_FALSE:real(d,a,b,c);}
EGLBoolean eglDestroyContext(EGLDisplay d,EGLContext c)
{typedef EGLBoolean (*Fn)(EGLDisplay,EGLContext);REAL(eglDestroyContext,Fn);++contexts;EGLBoolean ok=real(d,c);assert(ok==EGL_TRUE);return ok;}
EGLBoolean eglDestroySurface(EGLDisplay d,EGLSurface s)
{typedef EGLBoolean (*Fn)(EGLDisplay,EGLSurface);REAL(eglDestroySurface,Fn);++surfaces;EGLBoolean ok=real(d,s);assert(ok==EGL_TRUE);return ok;}
EGLBoolean eglTerminate(EGLDisplay d)
{typedef EGLBoolean (*Fn)(EGLDisplay);REAL(eglTerminate,Fn);++terminated;EGLBoolean ok=real(d);assert(ok==EGL_TRUE);return ok;}
#undef REAL
