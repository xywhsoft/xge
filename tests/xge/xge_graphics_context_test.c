/* Real matched shared/static engine: owning renderer, native graph, creator
 * affinity, deferred children, private queues, OOM and owner callback failures. */
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
#include <limits.h>
#if XGE_ENABLE_EGL
#include <EGL/egl.h>
#endif
typedef struct Owner {unsigned refs,holds,drops,mode;} Owner;
static bool count(const void* raw,size_t* out){*out=((const Owner*)raw)->refs;return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user)
{(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static bool hold(const void* raw)
{
    Owner* owner=(Owner*)raw;++owner->holds;
    if(owner->mode==4)return false;
    ++owner->refs;
    if(owner->mode==5)xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"context-owner",31,"first hold failure"));
    if(owner->mode==0){
        assert(xgeUnitChecked()==XGE_ERROR_BUSY && xgeFrame()==XGE_ERROR_BUSY);
        xge_graphics_context nested=NULL;assert(xgeGraphicsContextCreateEglOwned(&nested,2,2,false,NULL)==XGE_ERROR_BUSY && !nested);
        xge_texture texture=NULL;assert(xgeTextureCreateRGBAOwned(&texture,1,1,NULL,0)==XGE_ERROR_BUSY && !texture);
    }
    return true;
}
static bool drop(const void* raw)
{
    Owner* owner=(Owner*)raw;assert(owner->refs==2);--owner->refs;++owner->drops;
    assert(xgeUnitChecked()==XGE_ERROR_BUSY && xgeFrame()==XGE_ERROR_BUSY);
    if(owner->mode==5)xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"context-owner",32,"secondary drop failure"));
    return owner->mode!=5 && owner->mode!=6;
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
#if XGE_ENABLE_EGL
static bool visitCode(xrtownershipref ref,void* user)
{Owner* owner=user;assert(ref.Data==owner && ref.Ops==&ops && owner->refs==2);return true;}
static xge_graphics_context_info_t info(xge_graphics_context context)
{xge_graphics_context_info_t out={0};out.size=sizeof(out);assert(xgeGraphicsContextInfoGet(context,&out)==XGE_OK);return out;}
static void pixels(void)
{
    xge_render_target target=NULL;xge_pass pass=NULL;
    assert(xgeRenderTargetCreateOwned(&target,2,2)==XGE_OK && target->iFramebufferId);
    assert(xgePassCreateOwned(&pass,target,XGE_PASS_CLEAR_COLOR,XGE_COLOR_RGBA(10,20,30,255))==XGE_OK);
    assert(xgePassBegin(pass)==XGE_OK);
    xge_rect_t rect={0,0,2,2};xgeShapeRectFill(rect,XGE_COLOR_RGBA(80,90,100,255));
    assert(xgePassEnd(pass)==XGE_OK);
    unsigned char data[16]={0};assert(xgeRenderTargetReadPixelsSized(target,data,sizeof(data),8)==XGE_OK);
    for(unsigned i=0;i<4;++i)assert(data[4*i]==80 && data[4*i+1]==90 && data[4*i+2]==100 && data[4*i+3]==255);
    xge_shader shader=NULL;
    assert(xgeShaderCreateOwned(&shader,
        "#version 300 es\nlayout(location=0)in vec4 p;void main(){gl_Position=p;}",
        "#version 300 es\nprecision mediump float;out vec4 c;void main(){c=vec4(1);}")==XGE_OK && shader->iProgram);
    xgeShaderFree(shader);xgePassFree(pass);xgeRenderTargetFree(target);
}
static int32 foreignThread(void* raw)
{
    xge_graphics_context context=raw,copy=NULL;bool consumed=true;
    xge_graphics_context_info_t out={0};out.size=sizeof(out);
    assert(xrtMemDebugFailAfter(0));
    assert(xgeGraphicsContextInfoGet(context,&out)==XGE_ERROR_BUSY);
    assert(xgeGraphicsContextMakeCurrent(context)==XGE_ERROR_BUSY);
    assert(xgeGraphicsContextShareOwned(context,&copy)==XGE_ERROR_BUSY && !copy);
    assert(xgeGraphicsContextReleaseOwnerChecked(context,&consumed)==XGE_ERROR_BUSY && !consumed);
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();return 0;
}
static int frame(void* raw)
{
    bool consumed=true;xge_graphics_context context=raw;
    assert(xgeGraphicsContextReleaseOwnerChecked(context,&consumed)==XGE_STATUS_DEFERRED && !consumed);
    assert(info(context).graphicsReady);return 0;
}
static void graph(xge_graphics_context context,Owner* owner)
{
    xrtownershipref ref=xgeGraphicsContextOwnership(context);size_t refs=0;
    const xrtownershipadapterv1* adapter=xgeGraphicsContextOwnershipAdapterV1(ref);
    const xrtownershippreparationv1* prep=xgeGraphicsContextOwnershipPreparationV1(ref);
    const xrtownershipfinishv2* finish=xgeGraphicsContextOwnershipCompletionV2(ref);
    const xrtownershipfinalizationv1* final=xgeGraphicsContextOwnershipFinalizationV1(ref);
    assert(adapter && prep && finish && final && prep->Adapter==adapter && finish->V1.Adapter==adapter && final->Adapter==adapter);
    assert(ref.Ops->Count(ref.Data,&refs) && refs==1 && ref.Ops->Trace(ref.Data,visitCode,owner));
    assert(adapter->Hold(ref.Data));int token=0;
    assert(adapter->Claim(ref.Data,&token) && prep->Prepare(ref.Data,&token)==XRT_OWNERSHIP_PREPARE_READY);
    xge_texture child=NULL;assert(xgeTextureCreateRGBAOwned(&child,1,1,NULL,0)==XGE_OK);
    bool complete=true;assert(final->CanStep(ref.Data,&token) && final->Step(ref.Data,&token,&complete) && !complete);
    assert(owner->refs==2 && !owner->drops && ref.Ops->Trace(ref.Data,visitCode,owner));
    xgeTextureFree(child);assert(final->Step(ref.Data,&token,&complete) && complete);
    adapter->Clear(ref.Data,&token);assert(finish->V1.Step(ref.Data,&token)==XRT_OWNERSHIP_FINISH_READY);
    assert(owner->refs==1 && owner->drops==1 && !xgeGraphicsOwnedResourceCount());
    assert(finish->DropChecked(ref.Data));bool consumed=false;
    assert(xgeGraphicsContextReleaseOwnerChecked(context,&consumed)==XGE_OK && consumed && owner->drops==1);
}
#endif
int main(int argc,char** argv)
{
    assert(argc==2);unsigned mode=(unsigned)atoi(argv[1]);assert(mode<=13);
#if !defined(XGE_TEST_STATIC)
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
#endif
    assert(xrtMemDebugEnable(true));
    assert(xgeInit(NULL)==XGE_OK && xgeUnitChecked()==XGE_OK);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    xge_desc_t desc={0};desc.iWidth=80;desc.iHeight=60;desc.iRunMode=mode==13 ? XGE_RUN_GAME_LOOP:XGE_RUN_MANUAL;
    assert(xgeInit(&desc)==XGE_OK);xge_platform_backend_t platform=xgePlatformBackendGet();xge_graphics_backend_t graphics=xgeGraphicsBackendGet();
    Owner owner={1,0,0,mode};xge_callback_owner_t code={{&owner,&ops},hold,drop};xge_graphics_context context=NULL;
    assert(xgeGraphicsContextCreateEglOwned(NULL,2,2,false,&code)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeGraphicsContextCreateEglOwned(&context,0,2,false,&code)==XGE_ERROR_INVALID_ARGUMENT && !context);
#if XGE_ENABLE_EGL
    if(mode==7){
        assert(xrtMemDebugFailAfter(0));
        assert(xgeGraphicsContextCreateEglOwned(&context,4,4,false,&code)==XGE_ERROR_OUT_OF_MEMORY && !context);
        assert(xrtMemDebugFailTriggered() && !owner.holds && !xgeGraphicsOwnedResourceCount());
        xrtMemDebugFailClear();xrtClearError();
    }else if(mode==11){
        xge_egl_desc_t rawDesc={0};rawDesc.iWidth=rawDesc.iHeight=2;rawDesc.bPBuffer=1;xge_egl_context_t raw={0};
        assert(xgeEGLInit(&raw,&rawDesc)==XGE_OK);EGLContext current=eglGetCurrentContext();assert(current!=EGL_NO_CONTEXT);
        assert(xgeGraphicsContextCreateEglOwned(&context,4,4,false,&code)==XGE_ERROR_BUSY && !context && !owner.holds);
        assert(eglGetCurrentContext()==current && xgeEGLUnitChecked(&raw)==XGE_OK);
        assert(xgePlatformBackendSet(&platform)==XGE_OK && xgeGraphicsBackendSet(&graphics)==XGE_OK);
    }else if(mode==12){
        assert(xgeGraphicsContextCreateEglOwned(&context,INT_MAX,1,true,&code)==XGE_ERROR_UNSUPPORTED && !context);
        assert(owner.holds==1 && owner.drops==1 && owner.refs==1);
    }else if(mode==13){
        assert(xgeGraphicsContextCreateEglOwned(&context,4,4,false,&code)==XGE_ERROR_UNSUPPORTED && !context && !owner.holds);
    }else{
        EGLenum previous=eglQueryAPI();
        int status=xgeGraphicsContextCreateEglOwned(&context,4,4,mode==1,&code);
        if(mode==4 || mode==5){
            assert(status==XGE_ERROR_INVALID_STATE && !context && owner.holds==1 && owner.refs==1 && owner.drops==(mode==5));
            if(mode==5)assert(xrtGetError() && xrtErrorCode(xrtGetError())==31);
            xrtClearError();
        }else{
            assert(status==XGE_OK && context && owner.refs==2 && owner.holds==1);
            xge_graphics_context_info_t state=info(context);
            assert(state.width==4 && state.height==4 && state.backend==XGE_GPU_BACKEND_GLES30 && state.current && state.graphicsReady && !state.otherResources);
            assert(state.surfaceless==(mode==1) && xgeUnitChecked()==XGE_ERROR_BUSY);
            assert(xgeGraphicsContextMakeCurrent(context)==XGE_OK);
            if(mode==0 || mode==1)pixels();
            if(mode==8){xthread* thread=xrtThreadCreate(foreignThread,context,0);assert(thread);assert(xrtThreadWaitFor(thread,10000)==XWAIT_OK && !xrtThreadExitCode(thread));xrtThreadDestroy(thread);}
            if(mode==9){
                xge_render_target target=NULL;xge_pass pass=NULL;assert(xgeRenderTargetCreateOwned(&target,2,2)==XGE_OK);
                assert(xgePassCreateOwned(&pass,target,0,0)==XGE_OK && xgePassBegin(pass)==XGE_OK);bool consumed=true;
                assert(xgeGraphicsContextReleaseOwnerChecked(context,&consumed)==XGE_STATUS_DEFERRED && !consumed && info(context).graphicsReady);
                assert(xgePassEnd(pass)==XGE_OK);xgePassFree(pass);xgeRenderTargetFree(target);
                assert(xgeSetFrameCallbackOwned(frame,context,NULL)==XGE_OK && xgeFrame()==1 && xgeSetFrameCallbackOwned(NULL,NULL,NULL)==XGE_OK);
            }
            if(mode==10){graph(context,&owner);context=NULL;}
            else{
                xge_graphics_context copy=NULL;assert(xgeGraphicsContextShareOwned(context,&copy)==XGE_OK && copy==context);
                bool consumed=false;assert(xgeGraphicsContextReleaseOwnerChecked(copy,&consumed)==XGE_OK && consumed && !owner.drops);
                xge_texture child=NULL;
                if(mode==2 || mode==3){
                    const unsigned char white[4]={255,255,255,255};assert(xgeTextureCreateRGBAOwned(&child,1,1,white,sizeof(white))==XGE_OK);
                    if(mode==3){xgeDraw(child,0,0);xgeTextureFree(child);child=NULL;}
                }
                consumed=true;status=xgeGraphicsContextReleaseOwnerChecked(context,&consumed);
                if(mode==2){
                    assert(status==XGE_STATUS_DEFERRED && !consumed && !owner.drops && owner.refs==2);
                    xge_gpu_caps_t caps={0};assert(xgeGpuCapsGet(&caps)==XGE_OK && caps.iMaxTextureSize>0 && caps.sRenderer[0]);
                    unsigned char retained[4]={0};assert(xgeTextureReadPixelsSized(child,retained,sizeof(retained),4)==XGE_OK);
                    assert(retained[0]==255 && retained[1]==255 && retained[2]==255 && retained[3]==255);
                    xge_graphics_context next=NULL;assert(xgeGraphicsContextCreateEglOwned(&next,2,2,false,NULL)==XGE_ERROR_BUSY && !next);
                    assert(xgeUnitChecked()==XGE_ERROR_BUSY);xgeTextureFree(child);
                    status=xgeGraphicsContextReleaseOwnerChecked(context,&consumed);
                }
                assert(status==(mode==6 ? XGE_ERROR_INVALID_STATE:XGE_OK) && consumed);context=NULL;
            }
            assert(owner.refs==1 && owner.drops==1 && !xgeGraphicsOwnedResourceCount() && eglQueryAPI()==previous);
        }
    }
#else
    assert(xrtMemDebugFailAfter(0));
    assert(xgeGraphicsContextCreateEglOwned(&context,4,4,false,&code)==XGE_ERROR_UNSUPPORTED && !context);
    assert(!xrtMemDebugFailTriggered() && !owner.holds && !owner.drops);xrtMemDebugFailClear();
#endif
    assert(!xgeGraphicsOwnedResourceCount());
    assert(xgePlatformBackendGet().iType==platform.iType && xgeGraphicsBackendGet().iType==graphics.iType);
    assert(xgeUnitChecked()==XGE_OK && !xrtGetError());
    xrtMemDebugSnapshot(&after);assert(after.LiveBytes==before.LiveBytes && after.LiveCount==before.LiveCount);
    printf("Owning graphics context mode %u: actual renderer/lifetime checks passed (EGL=%d)\n",mode,XGE_ENABLE_EGL);return 0;
}
