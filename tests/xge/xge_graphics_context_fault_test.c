/* Real owning renderer failures via typed ELF interposition. No replacement
 * engine TU. Every unfinished OS/code obligation must remain recoverable. */
#define XRT_MODULE_ALL
#include <xge.h>
#if defined(XGE_TEST_STATIC)
#include <xge_static_xrt_contract.h>
#else
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#endif
#include <EGL/egl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "xge_egl_fault_preload.h"
typedef struct Owner {unsigned refs,holds,drops;} Owner;
static bool count(const void* data,size_t* out){*out=((const Owner*)data)->refs;return true;}
static bool trace(const void* data,xrtownershipvisitor visit,void* user){(void)data;(void)visit;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static bool hold(const void* data){Owner* o=(Owner*)data;++o->refs;++o->holds;return true;}
static bool drop(const void* data){Owner* o=(Owner*)data;assert(o->refs==2);--o->refs;++o->drops;return true;}
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
typedef struct Reentry {xge_graphics_context context;unsigned calls;} Reentry;
static void onError(const xge_error_info_t* error,void* user)
{
    assert(error);Reentry* reentry=user;++reentry->calls;bool consumed=true;
    assert(xgeGraphicsContextReleaseOwnerChecked(reentry->context,&consumed)==XGE_STATUS_DEFERRED && !consumed);
    xge_graphics_context_info_t info={0};info.size=sizeof(info);
    assert(xgeGraphicsContextInfoGet(reentry->context,&info)==XGE_OK && !info.graphicsReady);
}
static void dispatchError(void* user)
{
    (void)user;
    /* EGL itself does not invoke the XGE error callback. Explicitly inject
     * synchronous user reentry at the real failed-driver call boundary. */
    xge_error_info_t error={0};error.iSize=sizeof(error);error.iCode=XGE_ERROR_BACKEND_FAILED;
    error.bRecoverable=true;error.sSubsystem="fixture";error.sOperation="eglMakeCurrent";
    assert(xgeReportError(&error)==XGE_OK);
}
int main(int argc,char** argv)
{
    assert(argc==2);unsigned mode=(unsigned)atoi(argv[1]);assert(mode<=17);
#if !defined(XGE_TEST_STATIC)
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
#endif
    assert(xrtMemDebugEnable(true));assert(xgeInit(NULL)==XGE_OK && xgeUnitChecked()==XGE_OK);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    xge_desc_t desc={0};desc.iWidth=80;desc.iHeight=60;desc.iRunMode=XGE_RUN_MANUAL;assert(xgeInit(&desc)==XGE_OK);
    xge_platform_backend_t platform=xgePlatformBackendGet();xge_graphics_backend_t graphics=xgeGraphicsBackendGet();
    Owner owner={1,0,0};xge_callback_owner_t code={{&owner,&ops},hold,drop};xge_graphics_context context=NULL;
    EGLenum previous=eglQueryAPI();xgeEglFaultSet(0);
    if(mode<=6){
        xgeEglFaultSet((int)mode+1);
        assert(xgeGraphicsContextCreateEglOwned(&context,4,4,false,&code)==XGE_ERROR_BACKEND_FAILED && !context);
    }else if(mode==11){
        /* A failed attach must not discard another owner's preload queue. */
        xge_texture preload=NULL;assert(xgeTextureCreateRGBAOwned(&preload,1,1,NULL,0)==XGE_OK);
        uint32_t flags=preload->iFlags;
        xgeEglFaultSet(6);xgeEglCleanupFaultRepeat(3,UINT_MAX);
        assert(xgeGraphicsContextCreateEglOwned(&context,4,4,false,&code)==XGE_ERROR_BACKEND_FAILED && context);
        assert(owner.refs==2 && !owner.drops && xgeGraphicsOwnedResourceCount()==2 && preload->iFlags==flags);
        xgeEglCleanupFaultClear();bool consumed=false;
        assert(xgeGraphicsContextReleaseOwnerChecked(context,&consumed)==XGE_OK && consumed);
        assert(preload->iFlags==flags && xgeGraphicsOwnedResourceCount()==1);xgeTextureFree(preload);
    }else if(mode==14 || mode==15 || mode==16){
        if(mode==16)xgeEglQuerySurfaceFaultArm();
        else xgeEglProcFault(mode==14 ? "glBindFramebuffer":"glGetIntegerv");
        int result=xgeGraphicsContextCreateEglOwned(&context,4,4,false,&code);
        assert(result==(mode==15 ? XGE_ERROR_UNSUPPORTED:XGE_ERROR_BACKEND_FAILED) && !context);
    }else if(mode==17){
        xge_offscreen_t offscreen={0};assert(xgeOffscreenInit(&offscreen,2,2)==XGE_OK && offscreen.bActive);
        xgeEglCleanupFaultArm(4);xgeOffscreenUnit(&offscreen);
        assert(!offscreen.bActive && offscreen.tEGL.bCleanupPending && !xgeOffscreenRenderTarget(&offscreen));
        xgeOffscreenUnit(&offscreen);assert(!offscreen.tEGL.bCleanupPending);
        assert(xgePlatformBackendSet(&platform)==XGE_OK && xgeGraphicsBackendSet(&graphics)==XGE_OK);
    }else{
        if(mode==12)assert(eglBindAPI(EGL_OPENGL_API));
        EGLenum expectedApi=eglQueryAPI();
        assert(xgeGraphicsContextCreateEglOwned(&context,4,4,false,&code)==XGE_OK && context);
        if(mode==13){
            xge_render_target target=NULL;assert(xgeRenderTargetCreateOwned(&target,2,2)==XGE_OK);
            xge_texture texture=NULL;const unsigned char first[4]={255,255,255,255},next[4]={1,2,3,255};
            assert(xgeTextureCreateRGBAOwned(&texture,1,1,first,sizeof(first))==XGE_OK);
            assert(eglMakeCurrent(eglGetCurrentDisplay(),EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT));
            Reentry reentry={context,0};assert(xgeSetErrorCallback(onError,&reentry)==XGE_OK);xgeEglFaultModeOnly(7);
            xgeEglMakeCurrentFaultHook(dispatchError,NULL);
            assert(xgeGraphicsContextMakeCurrent(context)==XGE_ERROR_BACKEND_FAILED && reentry.calls);
            xgeEglMakeCurrentFaultHook(NULL,NULL);
            assert(xgeSetErrorCallback(NULL,NULL)==XGE_OK);
            unsigned char output[16];assert(xgeRenderTargetReadPixelsSized(target,output,sizeof(output),8)==XGE_ERROR_INVALID_STATE);
            assert(xgeTextureUpdateRGBASized(texture,0,0,1,1,next,sizeof(next),4)==XGE_ERROR_NOT_INITIALIZED);
            xge_sampler_t sampler=xgeTextureGetSampler(texture);sampler.iWrapS=XGE_WRAP_REPEAT;
            assert(xgeTextureSetSampler(texture,&sampler)==XGE_ERROR_NOT_INITIALIZED);
            xgeEglFaultModeOnly(0);assert(xgeGraphicsContextMakeCurrent(context)==XGE_OK);
            xgeRenderTargetFree(target);xgeTextureFree(texture);
        }else if(mode==12)xgeEglRestoreApiFaultArm();
        else xgeEglCleanupFaultArm((int)mode-6);
        bool consumed=false;int result=xgeGraphicsContextReleaseOwnerChecked(context,&consumed);
        if(mode==13)assert(result==XGE_OK && consumed);
        else{
            assert(result==XGE_ERROR_BACKEND_FAILED && !consumed && owner.refs==2 && !owner.drops && xgeGraphicsOwnedResourceCount()==1);
            assert(xgeUnitChecked()==XGE_ERROR_BUSY);
            assert(xgeGraphicsContextReleaseOwnerChecked(context,&consumed)==XGE_OK && consumed);
        }
        assert(eglQueryAPI()==expectedApi);if(mode==12)assert(eglBindAPI(previous));
    }
    assert(owner.refs==1 && owner.holds==owner.drops && !xgeGraphicsOwnedResourceCount());
    assert(eglGetCurrentContext()==EGL_NO_CONTEXT && eglQueryAPI()==previous);
    if(mode!=17)assert(xgePlatformBackendGet().iType==platform.iType && xgeGraphicsBackendGet().iType==graphics.iType);
    assert(xgeUnitChecked()==XGE_OK && !xrtGetError());xrtMemDebugSnapshot(&after);
    assert(after.LiveBytes==before.LiveBytes && after.LiveCount==before.LiveCount);
    printf("Owning context fault mode %u: rollback/retry/reentry balanced\n",mode);return 0;
}
