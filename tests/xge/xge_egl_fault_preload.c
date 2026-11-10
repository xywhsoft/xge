/* Typed, explicitly linked ELF interposition for the real engine shared image.
 * Test-only controls; no substitute engine translation unit or graphics mocks. */
#include <EGL/egl.h>
#include <assert.h>
#include <dlfcn.h>
#include <string.h>
#include "xge_egl_fault_preload.h"
static int mode,contexts,surfaces,terminated,cleanupStage;
static unsigned cleanupAttempts;
static int restoreApiFault,querySurfaceFault;
static char missingProc[64];
static EGLint injectedError;
static void (*makeCurrentHook)(void*);
static void* makeCurrentUser;
void xgeEglFaultSet(int value){mode=value;contexts=surfaces=terminated=cleanupStage=0;cleanupAttempts=0;restoreApiFault=querySurfaceFault=0;missingProc[0]=0;injectedError=EGL_SUCCESS;}
void xgeEglFaultCounts(int* c,int* s,int* t){*c=contexts;*s=surfaces;*t=terminated;}
void xgeEglCleanupFaultRepeat(int stage,unsigned attempts){assert(stage>=1 && stage<=4 && attempts);cleanupStage=stage;cleanupAttempts=attempts;}
void xgeEglCleanupFaultArm(int stage){xgeEglCleanupFaultRepeat(stage,1);}
void xgeEglCleanupFaultClear(void){cleanupStage=0;cleanupAttempts=0;}
void xgeEglFaultModeOnly(int value){mode=value;}
void xgeEglMakeCurrentFaultHook(void (*callback)(void*),void* user){makeCurrentHook=callback;makeCurrentUser=user;}
void xgeEglRestoreApiFaultArm(void){restoreApiFault=1;}
void xgeEglProcFault(const char* name){assert(!name || strlen(name)<sizeof(missingProc));if(name)strcpy(missingProc,name);else missingProc[0]=0;}
void xgeEglQuerySurfaceFaultArm(void){querySurfaceFault=1;}
static int cleanupFail(int stage)
{if(cleanupStage!=stage)return 0;if(!--cleanupAttempts)cleanupStage=0;injectedError=EGL_BAD_ACCESS;return 1;}
#define REAL(Name,Type) Type real;do{void* p=dlsym(RTLD_NEXT,#Name);assert(p && sizeof(real)==sizeof(p));memcpy(&real,&p,sizeof(real));}while(0)
EGLint eglGetError(void)
{typedef EGLint (*Fn)(void);REAL(eglGetError,Fn);if(injectedError!=EGL_SUCCESS){EGLint e=injectedError;injectedError=EGL_SUCCESS;return e;}return real();}
EGLDisplay eglGetDisplay(EGLNativeDisplayType d)
{typedef EGLDisplay (*Fn)(EGLNativeDisplayType);REAL(eglGetDisplay,Fn);return mode==1 ? EGL_NO_DISPLAY:real(d);}
EGLBoolean eglInitialize(EGLDisplay d,EGLint* a,EGLint* b)
{typedef EGLBoolean (*Fn)(EGLDisplay,EGLint*,EGLint*);REAL(eglInitialize,Fn);return mode==2 ? EGL_FALSE:real(d,a,b);}
EGLBoolean eglBindAPI(EGLenum api)
{typedef EGLBoolean (*Fn)(EGLenum);REAL(eglBindAPI,Fn);if(restoreApiFault && api==EGL_OPENGL_API){restoreApiFault=0;injectedError=EGL_BAD_ACCESS;return EGL_FALSE;}return mode==3 ? EGL_FALSE:real(api);}
EGLBoolean eglChooseConfig(EGLDisplay d,const EGLint* a,EGLConfig* c,EGLint count,EGLint* n)
{typedef EGLBoolean (*Fn)(EGLDisplay,const EGLint*,EGLConfig*,EGLint,EGLint*);REAL(eglChooseConfig,Fn);return mode==4 ? EGL_FALSE:real(d,a,c,count,n);}
EGLSurface eglCreatePbufferSurface(EGLDisplay d,EGLConfig c,const EGLint* a)
{typedef EGLSurface (*Fn)(EGLDisplay,EGLConfig,const EGLint*);REAL(eglCreatePbufferSurface,Fn);return mode==5 ? EGL_NO_SURFACE:real(d,c,a);}
EGLSurface eglCreateWindowSurface(EGLDisplay d,EGLConfig c,EGLNativeWindowType w,const EGLint* a)
{typedef EGLSurface (*Fn)(EGLDisplay,EGLConfig,EGLNativeWindowType,const EGLint*);REAL(eglCreateWindowSurface,Fn);return mode==10 ? EGL_NO_SURFACE:real(d,c,w,a);}
EGLContext eglCreateContext(EGLDisplay d,EGLConfig c,EGLContext shared,const EGLint* a)
{typedef EGLContext (*Fn)(EGLDisplay,EGLConfig,EGLContext,const EGLint*);REAL(eglCreateContext,Fn);return mode==6 ? EGL_NO_CONTEXT:real(d,c,shared,a);}
EGLBoolean eglMakeCurrent(EGLDisplay d,EGLSurface a,EGLSurface b,EGLContext c)
{typedef EGLBoolean (*Fn)(EGLDisplay,EGLSurface,EGLSurface,EGLContext);REAL(eglMakeCurrent,Fn);
 if(mode==7 && c!=EGL_NO_CONTEXT){if(makeCurrentHook)makeCurrentHook(makeCurrentUser);injectedError=EGL_BAD_ACCESS;return EGL_FALSE;}
 return c==EGL_NO_CONTEXT && cleanupFail(1) ? EGL_FALSE:real(d,a,b,c);}
EGLBoolean eglDestroyContext(EGLDisplay d,EGLContext c)
{typedef EGLBoolean (*Fn)(EGLDisplay,EGLContext);REAL(eglDestroyContext,Fn);if(cleanupFail(2))return EGL_FALSE;++contexts;EGLBoolean ok=real(d,c);assert(ok==EGL_TRUE);return ok;}
EGLBoolean eglDestroySurface(EGLDisplay d,EGLSurface s)
{typedef EGLBoolean (*Fn)(EGLDisplay,EGLSurface);REAL(eglDestroySurface,Fn);if(cleanupFail(3))return EGL_FALSE;++surfaces;EGLBoolean ok=real(d,s);assert(ok==EGL_TRUE);return ok;}
EGLBoolean eglTerminate(EGLDisplay d)
{typedef EGLBoolean (*Fn)(EGLDisplay);REAL(eglTerminate,Fn);if(cleanupFail(4))return EGL_FALSE;++terminated;EGLBoolean ok=real(d);assert(ok==EGL_TRUE);return ok;}
__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char* name)
{typedef __eglMustCastToProperFunctionPointerType (*Fn)(const char*);REAL(eglGetProcAddress,Fn);return missingProc[0] && !strcmp(missingProc,name) ? NULL:real(name);}
EGLBoolean eglQuerySurface(EGLDisplay d,EGLSurface s,EGLint attribute,EGLint* value)
{typedef EGLBoolean (*Fn)(EGLDisplay,EGLSurface,EGLint,EGLint*);REAL(eglQuerySurface,Fn);if(querySurfaceFault){querySurfaceFault=0;injectedError=EGL_BAD_ACCESS;return EGL_FALSE;}return real(d,s,attribute,value);}
#undef REAL
