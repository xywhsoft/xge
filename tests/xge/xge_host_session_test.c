/* Real private WGL/EGL context and XGE pixels. Local bounded tests only. */
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
#include <math.h>
#include <limits.h>
#if defined(_WIN32)
#include <windows.h>
#include <GL/gl.h>
#else
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif
typedef struct Test {
    unsigned refs,holds,drops,pinit,ginit,punit,gunit,polls,begins,ends,presents,releases,binds;
    unsigned mode;
    xge_host_session host;
    bool releaseDeferred;
#if defined(_WIN32)
    HWND window;HDC dc;HGLRC context;
#else
    EGLDisplay display;EGLSurface surface;EGLContext context;
#endif
} Test;
static bool count(const void* raw,size_t* out){*out=((const Test*)raw)->refs;return true;}
static bool trace(const void* raw,xrtownershipvisitor visitor,void* user)
{(void)raw;(void)visitor;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static bool hold(const void* raw)
{
    Test* t=(Test*)raw;++t->holds;
    if(t->mode==22 && t->holds==3)return false;
    ++t->refs;
    if(t->mode==23 && t->holds==3)xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"host-test",41,"first hold failure"));
    return true;
}
static bool drop(const void* raw)
{
    Test* t=(Test*)raw;assert(t->refs>1);--t->refs;++t->drops;
    if(t->mode==24 && t->drops==3){xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"host-test",42,"consuming drop failure"));return false;}
    return true;
}
static xge_host_session_info_t info(Test* t)
{xge_host_session_info_t i={0};i.size=sizeof(i);assert(xgeHostSessionInfoGet(t->host,&i)==XGE_OK);return i;}
static void reentry(Test* t,bool poll)
{
    if(!t->host)return;
    bool consumed=true;assert(xgeHostSessionReleaseOwnerChecked(t->host,&consumed)==XGE_STATUS_DEFERRED && !consumed);
    assert(xgeHostSessionBegin(t->host)==XGE_ERROR_BUSY);
    if(poll){assert(info(t).width==4);assert(xgeHostSessionResize(t->host,4,4,2,0)==XGE_OK);}
    else assert(xgeHostSessionResize(t->host,4,4,1,0)==XGE_ERROR_BUSY);
}
static int platformInit(void* raw)
{
    Test* t=raw;++t->pinit;
    if(t->mode==5)return XGE_ERROR_BACKEND_FAILED;
#if defined(_WIN32)
    WNDCLASSW cls={0};cls.style=CS_OWNDC;cls.lpfnWndProc=DefWindowProcW;cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=L"xge-host-local-test";
    if(!RegisterClassW(&cls) && GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)return XGE_ERROR_BACKEND_FAILED;
    t->window=CreateWindowW(cls.lpszClassName,L"XGE private test",WS_OVERLAPPEDWINDOW,0,0,64,64,NULL,NULL,cls.hInstance,NULL);
    if(!t->window)return XGE_ERROR_BACKEND_FAILED;
    t->dc=GetDC(t->window);if(!t->dc)return XGE_ERROR_BACKEND_FAILED;
    PIXELFORMATDESCRIPTOR p={0};p.nSize=sizeof(p);p.nVersion=1;p.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;p.iPixelType=PFD_TYPE_RGBA;p.cColorBits=32;p.cAlphaBits=8;
    int format=ChoosePixelFormat(t->dc,&p);if(!format || !SetPixelFormat(t->dc,format,&p))return XGE_ERROR_BACKEND_FAILED;
#else
    PFNEGLGETPLATFORMDISPLAYEXTPROC get=(PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
    if(!get)return XGE_ERROR_UNSUPPORTED;
    t->display=get(EGL_PLATFORM_SURFACELESS_MESA,EGL_DEFAULT_DISPLAY,NULL);
    if(t->display==EGL_NO_DISPLAY || !eglInitialize(t->display,NULL,NULL))return XGE_ERROR_BACKEND_FAILED;
#endif
    return XGE_OK;
}
static int graphicsInit(void* raw)
{
    Test* t=raw;++t->ginit;if(t->mode==6 || t->mode==25)return XGE_ERROR_BACKEND_FAILED;
#if defined(_WIN32)
    HGLRC temporary=wglCreateContext(t->dc);if(!temporary || !wglMakeCurrent(t->dc,temporary))return XGE_ERROR_BACKEND_FAILED;
    typedef HGLRC (WINAPI *Create)(HDC,HGLRC,const int*);
    Create create=(Create)wglGetProcAddress("wglCreateContextAttribsARB");
    const int attributes[]={0x2091,3,0x2092,3,0x9126,1,0};
    if(create)t->context=create(t->dc,NULL,attributes);
    assert(wglMakeCurrent(NULL,NULL));assert(wglDeleteContext(temporary));
    if(!t->context)return XGE_ERROR_UNSUPPORTED;
#else
    if(!eglBindAPI(EGL_OPENGL_API))return XGE_ERROR_BACKEND_FAILED;
    EGLint attrs[]={EGL_SURFACE_TYPE,EGL_PBUFFER_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_NONE};
    EGLConfig config=NULL;EGLint n=0;
    if(!eglChooseConfig(t->display,attrs,&config,1,&n) || n!=1)return XGE_ERROR_BACKEND_FAILED;
    EGLint surface[]={EGL_WIDTH,4,EGL_HEIGHT,4,EGL_NONE};t->surface=eglCreatePbufferSurface(t->display,config,surface);
    EGLint context[]={EGL_CONTEXT_MAJOR_VERSION,3,EGL_CONTEXT_MINOR_VERSION,3,EGL_CONTEXT_OPENGL_PROFILE_MASK,EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,EGL_NONE};
    t->context=eglCreateContext(t->display,config,EGL_NO_CONTEXT,context);
    if(t->surface==EGL_NO_SURFACE || t->context==EGL_NO_CONTEXT)return XGE_ERROR_BACKEND_FAILED;
#endif
    return XGE_OK;
}
static void platformUnit(void* raw){Test* t=raw;++t->punit;assert(t->releases && t->gunit<=1);}
static void graphicsUnit(void* raw){Test* t=raw;++t->gunit;assert(!t->releases);}
static int makeCurrent(void* raw)
{
    Test* t=raw;++t->binds;if(t->mode==7)return XGE_ERROR_BACKEND_FAILED;
#if defined(_WIN32)
    return wglMakeCurrent(t->dc,t->context) ? XGE_OK:XGE_ERROR_BACKEND_FAILED;
#else
    return eglMakeCurrent(t->display,t->surface,t->surface,t->context) ? XGE_OK:XGE_ERROR_BACKEND_FAILED;
#endif
}
static int isCurrent(void* raw)
{
    Test* t=raw;
#if defined(_WIN32)
    return t->context && wglGetCurrentContext()==t->context;
#else
    return t->context!=EGL_NO_CONTEXT && eglGetCurrentContext()==t->context;
#endif
}
static void displace(void)
{
#if defined(_WIN32)
    assert(wglMakeCurrent(NULL,NULL));
#else
    assert(eglMakeCurrent(eglGetCurrentDisplay(),EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT));
#endif
}
static void* load(const char* name,void* raw)
{
    Test* t=raw;if(t->mode==16 && !strcmp(name,"glViewport"))return NULL;
#if defined(_WIN32)
    PROC value=wglGetProcAddress(name);
    if(value==(PROC)1 || value==(PROC)2 || value==(PROC)3 || value==(PROC)(intptr_t)-1)value=NULL;
    if(!value)value=GetProcAddress(GetModuleHandleW(L"opengl32.dll"),name);
    return (void*)value;
#else
    return (void*)eglGetProcAddress(name);
#endif
}
static xge_host_release_result_t release(void* raw)
{
    Test* t=raw;++t->releases;
    if((t->mode==3 || t->mode==7 || t->mode==16 || t->mode==25) && !t->releaseDeferred){t->releaseDeferred=true;return (xge_host_release_result_t){XGE_OK,false};}
#if defined(_WIN32)
    if(t->context){if(wglGetCurrentContext()==t->context)assert(wglMakeCurrent(NULL,NULL));assert(wglDeleteContext(t->context));t->context=NULL;}
    if(t->dc){assert(ReleaseDC(t->window,t->dc));t->dc=NULL;}
    if(t->window){assert(DestroyWindow(t->window));t->window=NULL;}
#else
    if(t->context!=EGL_NO_CONTEXT){
        if(eglGetCurrentContext()==t->context)assert(eglMakeCurrent(t->display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT));
        assert(eglDestroyContext(t->display,t->context));t->context=EGL_NO_CONTEXT;
    }
    if(t->surface!=EGL_NO_SURFACE){assert(eglDestroySurface(t->display,t->surface));t->surface=EGL_NO_SURFACE;}
    if(t->display!=EGL_NO_DISPLAY){assert(eglTerminate(t->display));t->display=EGL_NO_DISPLAY;}
#endif
    if(t->mode==4)return (xge_host_release_result_t){XGE_ERROR_BACKEND_FAILED,true};
    if(t->mode==28)return (xge_host_release_result_t){XGE_STATUS_DEFERRED,true};
    return (xge_host_release_result_t){XGE_OK,true};
}
static int poll(void* raw){Test* t=raw;++t->polls;reentry(t,true);return t->mode==11 ? XGE_ERROR_BACKEND_FAILED:9;}
static int begin(void* raw)
{
    Test* t=raw;++t->begins;reentry(t,false);
    if(t->mode==26)xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"host-test",44,"accepted begin failure"));
    return t->mode==9 ? XGE_ERROR_BACKEND_FAILED:XGE_OK;
}
static int end(void* raw)
{
    Test* t=raw;++t->ends;reentry(t,false);
    if(t->mode==27)xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"host-test",45,"end failure"));
    return t->mode==10 ? XGE_ERROR_BACKEND_FAILED:XGE_OK;
}
static void present(void* raw)
{
    Test* t=raw;++t->presents;reentry(t,false);
    if(t->mode==12)xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"host-test",43,"present failure"));
}
#if !defined(XGE_TEST_STATIC)
static xge_host_xrt_proc resolve(void* user,const char* name)
{
    (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif
static void pixels(void)
{
    xge_render_target target=NULL;xge_pass pass=NULL;
    assert(xgeRenderTargetCreateOwned(&target,2,2)==XGE_OK);
    assert(xgePassCreateOwned(&pass,target,XGE_PASS_CLEAR_COLOR,XGE_COLOR_RGBA(10,20,30,255))==XGE_OK);
    assert(xgePassBegin(pass)==XGE_OK);
    xge_rect_t rect={0,0,2,2};xgeShapeRectFill(rect,XGE_COLOR_RGBA(80,90,100,255));
    assert(xgePassEnd(pass)==XGE_OK);unsigned char bytes[16]={0};
    assert(xgeRenderTargetReadPixelsSized(target,bytes,sizeof(bytes),8)==XGE_OK);
    for(unsigned i=0;i<4;++i)assert(bytes[i*4]==80 && bytes[i*4+1]==90 && bytes[i*4+2]==100 && bytes[i*4+3]==255);
    xgePassFree(pass);xgeRenderTargetFree(target);
}
static int32 foreign(void* raw)
{
    Test* t=raw;bool consumed=true,current=true;xge_host_session copy=NULL;xge_host_session_info_t i={0};i.size=sizeof(i);
    assert(xrtMemDebugFailAfter(0));
    assert(xgeHostSessionInfoGet(t->host,&i)==XGE_ERROR_BUSY);
    assert(xgeHostSessionBegin(t->host)==XGE_ERROR_BUSY);
    assert(xgeHostSessionPoll(t->host)==XGE_ERROR_BUSY);
    assert(xgeHostSessionMakeCurrent(t->host)==XGE_ERROR_BUSY);
    assert(xgeHostSessionIsCurrent(t->host,&current)==XGE_ERROR_BUSY && !current);
    assert(xgeHostSessionResize(t->host,2,2,1,0)==XGE_ERROR_BUSY);
    assert(xgeHostSessionShareOwned(t->host,&copy)==XGE_ERROR_BUSY && !copy);
    assert(xgeHostSessionReleaseOwnerChecked(t->host,&consumed)==XGE_ERROR_BUSY && !consumed);
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();return 0;
}
static bool visit(xrtownershipref ref,void* raw)
{unsigned* n=raw;size_t count=0;assert(ref.Ops && ref.Ops->Count(ref.Data,&count) && count);++*n;return true;}
static void graph(Test* t)
{
    xrtownershipref ref=xgeHostSessionOwnership(t->host);const xrtownershipadapterv1* a=xgeHostSessionOwnershipAdapterV1(ref);
    const xrtownershippreparationv1* p=xgeHostSessionOwnershipPreparationV1(ref);
    const xrtownershipfinishv2* f=xgeHostSessionOwnershipCompletionV2(ref);
    const xrtownershipfinalizationv1* z=xgeHostSessionOwnershipFinalizationV1(ref);
    assert(a && p && f && z);unsigned n=0;assert(ref.Ops->Trace(ref.Data,visit,&n) && n==3);
    int token=0;assert(a->Hold(ref.Data) && a->Claim(ref.Data,&token));
    assert(p->Prepare(ref.Data,&token)==XRT_OWNERSHIP_PREPARE_READY);
    bool complete=false;assert(z->Step(ref.Data,&token,&complete) && complete);
    a->Clear(ref.Data,&token);assert(f->V1.Step(ref.Data,&token)==XRT_OWNERSHIP_FINISH_READY);
    assert(f->DropChecked(ref.Data));bool consumed=false;
    assert(xgeHostSessionReleaseOwnerChecked(t->host,&consumed)==XGE_OK && consumed);t->host=NULL;
}
static void run(unsigned mode,int64_t failAfter)
{
    Test t={0};t.refs=1;t.mode=mode;
    xge_desc_t init={0};init.iRunMode=XGE_RUN_MANUAL;init.iWidth=80;init.iHeight=60;assert(xgeInit(&init)==XGE_OK);
    xge_callback_owner_t owner={{&t,&ops},hold,drop};
    xge_platform_backend_t pt={XGE_PLATFORM_BACKEND_CUSTOM,"local",platformInit,platformUnit,poll,present,&t};
    xge_graphics_backend_t gt={mode==8 ? XGE_GPU_BACKEND_CUSTOM:XGE_GPU_BACKEND_OPENGL33,"local",graphicsInit,graphicsUnit,begin,end,&t};
    xge_platform_backend_ref platform=NULL;xge_graphics_backend_ref graphics=NULL;
    assert(xgePlatformBackendCreateOwned(&platform,&pt,&owner)==XGE_OK);
    assert(xgeGraphicsBackendCreateOwned(&graphics,&gt,&owner)==XGE_OK);
    xge_host_context_hooks_t hooks={makeCurrent,isCurrent,mode==1 ? NULL:load,release,&t};
    xge_host_session_desc_t desc={sizeof(desc),4,4,1,0};
    if(mode==17){
        assert(xrtMemDebugFailAfter(0));desc.dpiScale=NAN;
        assert(xgeHostSessionCreateOwned(&t.host,&desc,platform,graphics,&hooks,&owner)==XGE_ERROR_INVALID_ARGUMENT && !t.host);
        desc.dpiScale=1e-30f;
        assert(xgeHostSessionCreateOwned(&t.host,&desc,platform,graphics,&hooks,&owner)==XGE_ERROR_INVALID_ARGUMENT && !t.host);
        desc.dpiScale=1;desc.size=0;
        assert(xgeHostSessionCreateOwned(&t.host,&desc,platform,graphics,&hooks,&owner)==XGE_ERROR_INVALID_ARGUMENT && !t.host);
        assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();desc.size=sizeof(desc);
    }
    if(failAfter>=0)assert(xrtMemDebugFailAfter(failAfter));
    int status=xgeHostSessionCreateOwned(&t.host,&desc,platform,graphics,&hooks,&owner);
    if(failAfter>=0)xrtMemDebugFailClear();
    assert(xgePlatformBackendReleaseChecked(platform)==XGE_OK);
    assert(xgeGraphicsBackendReleaseChecked(graphics)==XGE_OK);
    if(status!=XGE_OK){
        assert(failAfter>=0 || (mode>=5 && mode<=8) || mode==16 || mode==22 || mode==23 || mode==25);
        if(mode==23){assert(xrtGetError());xrtErrorFree(xrtTakeError());}
        else xrtErrorFree(xrtTakeError());
        if(t.host){assert(mode==7 || mode==16 || mode==25);bool consumed=false;t.mode=0;
            assert(xgeHostSessionReleaseOwnerChecked(t.host,&consumed)==XGE_OK && consumed);t.host=NULL;}
    }else{
        assert(t.host && info(&t).graphicsReady && info(&t).running);
        assert(xgeUnitChecked()==XGE_ERROR_BUSY && xgeFrame()==XGE_ERROR_BUSY);
        xge_host_session copy=NULL;assert(xgeHostSessionShareOwned(t.host,&copy)==XGE_OK && copy==t.host);
        assert(xgeHostSessionReleaseChecked(copy)==XGE_OK);
        if(mode==13){xthread* thread=xrtThreadCreate(foreign,&t,0);assert(thread);assert(xrtThreadWaitFor(thread,10000)==XWAIT_OK && !xrtThreadExitCode(thread));xrtThreadDestroy(thread);}
        if(mode==18){displace();bool current=true;assert(xgeHostSessionIsCurrent(t.host,&current)==XGE_OK && !current);}
        int result=xgeHostSessionPoll(t.host);
        assert(result==(mode==11 ? XGE_ERROR_BACKEND_FAILED:9));
        if(mode==11)t.mode=0;
        assert(xgeHostSessionBegin(t.host)==(mode==9 ? XGE_ERROR_BACKEND_FAILED:mode==26 ? XGE_ERROR_CALLBACK_FAILED:XGE_OK));
        if(mode==9){t.mode=0;assert(xgeHostSessionBegin(t.host)==XGE_OK);}
        if(mode==26){
            assert(!info(&t).frameOpen && t.ends==1 && xrtGetError());xrtErrorFree(xrtTakeError());
            t.mode=0;assert(xgeHostSessionBegin(t.host)==XGE_OK);
        }
        assert(info(&t).frameOpen && info(&t).width==4);
        bool consumed=true;assert(xgeHostSessionReleaseOwnerChecked(t.host,&consumed)==XGE_STATUS_DEFERRED && !consumed);
        pixels();
        assert(xgeHostSessionEnd(t.host)==(mode==10 ? XGE_ERROR_BACKEND_FAILED:mode==27 ? XGE_ERROR_CALLBACK_FAILED:XGE_OK));
        if(mode==10 || mode==27){assert(!info(&t).presentPending);assert(xgeHostSessionPresent(t.host)==XGE_ERROR_INVALID_STATE);xrtErrorFree(xrtTakeError());t.mode=0;}
        else if(mode==12){assert(xgeHostSessionPresent(t.host)==XGE_ERROR_CALLBACK_FAILED && xrtGetError());xrtErrorFree(xrtTakeError());assert(xgeHostSessionPresent(t.host)==XGE_ERROR_INVALID_STATE);t.mode=0;}
        else{assert(info(&t).presentPending);assert(xgeHostSessionPresent(t.host)==XGE_OK);}
        assert(xgeHostSessionBegin(t.host)==XGE_OK && xgeHostSessionEnd(t.host)==XGE_OK && xgeHostSessionDiscardFrame(t.host)==XGE_OK);
        if(mode==0){
            xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);
            for(unsigned i=0;i<1000;++i)assert(xgeHostSessionBegin(t.host)==XGE_OK && xgeHostSessionEnd(t.host)==XGE_OK && xgeHostSessionPresent(t.host)==XGE_OK);
            xrtMemDebugSnapshot(&b);assert(a.AllocCount==b.AllocCount && a.FreeCount==b.FreeCount && a.ReallocCount==b.ReallocCount);
            puts("HOST_SESSION_WARM_1000_XRT_ALLOCS_0");
        }
        if(mode==19){xgeQuit();assert(!info(&t).running && xgeHostSessionBegin(t.host)==XGE_ERROR_INVALID_STATE);}
        if(mode==20){
            xge_render_target target=NULL;xge_pass pass=NULL;assert(xgeRenderTargetCreateOwned(&target,2,2)==XGE_OK);
            assert(xgePassCreateOwned(&pass,target,XGE_PASS_CLEAR_COLOR,0)==XGE_OK);
            assert(xgeHostSessionBegin(t.host)==XGE_OK && xgePassBegin(pass)==XGE_OK);
            assert(xgeHostSessionEnd(t.host)==XGE_ERROR_BUSY);
            bool consumed=true;assert(xgeHostSessionReleaseOwnerChecked(t.host,&consumed)==XGE_STATUS_DEFERRED && !consumed);
            assert(xgePassEnd(pass)==XGE_OK && xgeHostSessionEnd(t.host)==XGE_OK && xgeHostSessionDiscardFrame(t.host)==XGE_OK);
            xgePassFree(pass);xgeRenderTargetFree(target);
        }
        if(mode==21){
            assert(xgeHostSessionResize(t.host,INT_MAX,4,1,0)==XGE_OK);
            assert(xgeHostSessionBegin(t.host)==XGE_ERROR_UNSUPPORTED && info(&t).width==4);
            assert(xgeHostSessionResize(t.host,4,4,1,0)==XGE_OK);
        }
        if(mode==14)graph(&t);
        else{
            xge_texture child=NULL;
            if(mode==2)assert(xgeTextureCreateRGBAOwned(&child,1,1,NULL,0)==XGE_OK);
            consumed=false;status=xgeHostSessionReleaseOwnerChecked(t.host,&consumed);
            if(mode==2){
                assert(status==XGE_STATUS_DEFERRED && !consumed && info(&t).closing && !t.releases);
                unsigned char bytes[4];assert(xgeTextureReadPixelsSized(child,bytes,sizeof(bytes),4)==XGE_OK);xgeTextureFree(child);
            }
            if(mode==3){assert(status==XGE_STATUS_DEFERRED && !consumed && t.gunit==1 && !t.punit && t.refs==4);}
            if(mode==2 || mode==3){consumed=false;assert(xgeHostSessionReleaseOwnerChecked(t.host,&consumed)==XGE_OK && consumed);}
            else if(mode==4 || mode==24 || mode==28){assert(status==(mode==24 ? XGE_ERROR_INVALID_STATE:XGE_ERROR_BACKEND_FAILED) && consumed);xrtErrorFree(xrtTakeError());}
            else assert(status==XGE_OK && consumed);
            t.host=NULL;
        }
    }
    assert(t.refs==1 && t.holds==t.drops+(mode==22 ? 1:0));
    assert(t.punit<=1 && t.gunit<=1);
    assert(xgeGraphicsOwnedResourceCount()==0 && xgeUnitChecked()==XGE_OK);
}
int main(int argc,char** argv)
{
    assert(argc==2);unsigned mode=(unsigned)atoi(argv[1]);assert(mode<=28);
#if !defined(XGE_TEST_STATIC)
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
#endif
    assert(xrtMemDebugEnable(true));assert(xgeInit(NULL)==XGE_OK && xgeUnitChecked()==XGE_OK);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    if(mode==15){for(int64_t i=0;i<12;++i)run(0,i);}else run(mode,-1);
    xrtMemDebugSnapshot(&after);assert(after.LiveBytes==before.LiveBytes && after.LiveCount==before.LiveCount);
    printf("HOST_SESSION_REAL_GL_OK mode=%u\n",mode);return 0;
}
