/* The actual private GL loader against a real EGL/GLES3 context. This is not
 * acceptance of an owning XGE renderer session or hardware performance. */
#include <EGL/egl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define XGE_GL_IMPL
#include "xge_gl.h"
static void* resolve(const char* name,void* missing)
{return missing && !strcmp(name,missing) ? NULL:(void*)eglGetProcAddress(name);}
int main(int argc,char** argv)
{
    assert(argc==2);int surfaceless=atoi(argv[1]);
    EGLDisplay display=eglGetDisplay(EGL_DEFAULT_DISPLAY);assert(display!=EGL_NO_DISPLAY);
    assert(eglInitialize(display,NULL,NULL) && eglBindAPI(EGL_OPENGL_ES_API));
    const EGLint configAttrs[]={EGL_SURFACE_TYPE,EGL_PBUFFER_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_ES3_BIT,
        EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_NONE};
    EGLConfig config;EGLint count=0;assert(eglChooseConfig(display,configAttrs,&config,1,&count) && count);
    const EGLint surfaceAttrs[]={EGL_WIDTH,2,EGL_HEIGHT,2,EGL_NONE};
    EGLSurface surface=surfaceless ? EGL_NO_SURFACE:eglCreatePbufferSurface(display,config,surfaceAttrs);
    assert(surfaceless || surface!=EGL_NO_SURFACE);
    const EGLint contextAttrs[]={EGL_CONTEXT_CLIENT_VERSION,3,EGL_NONE};
    EGLContext context=eglCreateContext(display,config,EGL_NO_CONTEXT,contextAttrs);assert(context!=EGL_NO_CONTEXT);
    assert(eglMakeCurrent(display,surface,surface,context));
    assert(xge_gl_load_ex(resolve,NULL));
    PFNGLBINDFRAMEBUFFERPROC committed=xge_glBindFramebuffer;
    assert(!xge_gl_load_ex(resolve,"glBindFramebuffer") && xge_glBindFramebuffer==committed);
    GLuint texture=0,fbo=0;unsigned char pixel[4]={0};
    xge_glGenTextures(1,&texture);xge_glBindTexture(GL_TEXTURE_2D,texture);
    xge_glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
    xge_glGenFramebuffers(1,&fbo);xge_glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    xge_glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);
    assert(xge_glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);
    xge_glViewport(0,0,1,1);xge_glClearColor(17.0f/255,34.0f/255,51.0f/255,1);
    xge_glClear(GL_COLOR_BUFFER_BIT);xge_glReadPixels(0,0,1,1,GL_RGBA,GL_UNSIGNED_BYTE,pixel);
    assert(pixel[0]==17 && pixel[1]==34 && pixel[2]==51 && pixel[3]==255);
    xge_glBindFramebuffer(GL_FRAMEBUFFER,0);xge_glDeleteFramebuffers(1,&fbo);xge_glDeleteTextures(1,&texture);
    assert(xge_glGetError()==GL_NO_ERROR);
    printf("Actual GL transaction/pixels: %s; %s; surfaceless=%d\n",xge_glGetString(GL_VERSION),xge_glGetString(GL_RENDERER),surfaceless);
    xge_gl_unload();assert(!xge_glBindFramebuffer && !xge_glCreateShader);
    assert(eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT));
    assert(eglDestroyContext(display,context));
    if(surface!=EGL_NO_SURFACE)assert(eglDestroySurface(display,surface));
    assert(eglTerminate(display));return 0;
}
