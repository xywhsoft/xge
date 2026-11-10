/* Real native CPU storage and checked shutdown. No graphics context/device. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xui.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static xge_host_xrt_proc resolve(void* user,const char* name)
{
    (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
static int frame(void* user){++*(int*)user;return 17;}
static int32 off_thread(ptr unused)
{
    (void)unused;xge_texture texture=(void*)(uintptr_t)1;xui_proxy proxy=(void*)(uintptr_t)1;
    xge_render_target window=(void*)(uintptr_t)1;assert(xrtMemDebugFailAfter(0));
    assert(xgeTextureCreateRGBAOwned(&texture,2,2,NULL,0)==XGE_ERROR_INVALID_STATE && !texture);
    assert(xgeRenderTargetWindowOwned(&window)==XGE_ERROR_INVALID_STATE && !window);
    assert(xuiProxyXgeCreate(&proxy)==XUI_ERROR_INVALID_STATE && !proxy);
    assert(xgeUnitChecked()==XGE_ERROR_INVALID_STATE && xgeInit(NULL)==XGE_ERROR_INVALID_STATE);
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
    return 0;
}
static void shutdown_and_slots(void)
{
    xge_texture texture=NULL;assert(xgeTextureCreateRGBAOwned(&texture,2,2,NULL,0)==XGE_OK);
    assert(xrtMemDebugFailAfter(0));assert(xgeInit(NULL)==XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailAfter(-1));xrtClearError();
    assert(xgeGraphicsOwnedResourceCount()==1);
    assert(xgeInit(NULL)==XGE_OK);
    assert(xrtMemDebugFailAfter(0));
    assert(xgeTextureUploadQueue(texture)==XGE_OK && !xrtMemDebugFailTriggered());
    assert(xrtMemDebugFailAfter(-1));
    int calls=0,result=0;assert(xgeSetFrameCallbackOwned(frame,&calls,NULL)==XGE_OK);
    assert(texture && xgeTextureIsOwned(texture) && xgeGraphicsOwnedResourceCount()==1);
    xthread* worker=xrtThreadCreate(off_thread,NULL,0);assert(worker);
    assert(xrtThreadWaitFor(worker,10000)==XWAIT_OK && xrtThreadExitCode(worker)==0);xrtThreadDestroy(worker);
    xge_material_t material; xgeMaterialInit(&material);
    assert(xgeMaterialSetTextureChecked(&material,texture)==XGE_OK);
    xgeTextureFree(texture); /* Material now owns the only reference. */
    assert(xgeUnitChecked()==XGE_ERROR_BUSY);
    assert(xgeInit(NULL)==XGE_ERROR_ALREADY_INITIALIZED);
    assert(xgeFrameCallbackDispatchChecked(&result)==XGE_OK && result==17 && calls==1);
    xgeMaterialFree(&material);assert(!xgeGraphicsOwnedResourceCount());
    assert(xgeUnitChecked()==XGE_OK);
    result=-1;assert(xgeFrameCallbackDispatchChecked(&result)==XGE_OK && result==0 && calls==1);
}
static void target_children(void)
{
    xge_render_target target=NULL;assert(xgeRenderTargetCreateOwned(&target,2,2)==XGE_OK);
    xge_texture child=xgeRenderTargetTexture(target);assert(child && xgeTextureIsOwned(child));
    assert(xgeTextureAddRef(child)==2 && target->_iOwnerReferences==2);
    assert(xgeRenderTargetResize(target,3,3)==XGE_ERROR_BUSY && target->iWidth==2);
    xgeRenderTargetFree(target); /* The child keeps this target's storage. */
    assert(child->iWidth==2 && xgeGraphicsOwnedResourceCount()==1);
    assert(xgeUnitChecked()==XGE_ERROR_BUSY);xgeTextureFree(child);
    assert(!xgeGraphicsOwnedResourceCount());
    assert(xgeRenderTargetCreateOwned(&target,2,2)==XGE_OK);
    void* storage=target->tTexture.pBackend;
    assert(xrtMemDebugFailAfter(0));assert(xgeRenderTargetResize(target,4,4)==XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailAfter(-1));xrtClearError();
    assert(target->iWidth==2 && target->tTexture.pBackend==storage && target->_iOwnerReferences==1);
    assert(xgeRenderTargetResize(target,3,3)==XGE_OK && target->iWidth==3);
    assert(xrtMemDebugFailAfter(0));
    assert(xgeTextureUploadQueue(&target->tTexture)==XGE_OK && !xrtMemDebugFailTriggered());
    assert(xrtMemDebugFailAfter(-1));
    unsigned char bytes[40];memset(bytes,0xA5,sizeof(bytes));
    assert(xgeRenderTargetReadPixelsSized(target,bytes,35,12)==XGE_ERROR_INVALID_ARGUMENT);
    for(size_t i=0;i<sizeof(bytes);++i)assert(bytes[i]==0xA5);
    /* No current renderer: a render target's CPU shadow is not a screenshot. */
    assert(xgeRenderTargetReadPixelsSized(target,bytes,36,12)==XGE_ERROR_INVALID_STATE);
    for(size_t i=0;i<sizeof(bytes);++i)assert(bytes[i]==0xA5);
    xge_pass pass=NULL;assert(xgePassCreateOwned(&pass,target,0,0)==XGE_OK);
    assert(target->_iOwnerReferences==2 && xgeGraphicsOwnedResourceCount()==1);
    xgeRenderTargetFree(target);assert(xgePassBegin(pass)==XGE_ERROR_NOT_INITIALIZED);
    assert(!pass->bActive && !target->_iActivePasses && xgePassIsIdle());
    bool consumed=false;assert(xgePassReleaseOwnerChecked(pass,&consumed)==XGE_OK && consumed);
    assert(!xgeGraphicsOwnedResourceCount());
}
static void shaders(void)
{
    xge_shader shader=(xge_shader)(uintptr_t)1;
    assert(xgeShaderCreateOwned(&shader,"void main(){}","void main(){}")==XGE_ERROR_NOT_INITIALIZED && !shader);
    assert(!xgeGraphicsOwnedResourceCount());
    xge_shader_t raw={0},overflow={0};raw.iRefCount=1;overflow.iRefCount=INT_MAX;
    xge_material_t material;xgeMaterialInit(&material);
    assert(xgeMaterialSetShaderChecked(&material,&raw)==XGE_OK && raw.iRefCount==2);
    assert(xgeMaterialSetShaderChecked(&material,&overflow)==XGE_ERROR_INVALID_STATE && material.pShader==&raw);
    assert(xgeMaterialSetShaderChecked(&material,&raw)==XGE_OK && raw.iRefCount==2);
    xgeShaderFree(&raw);assert(raw.iRefCount==1);
    xgeMaterialSetShader(&material,&raw);assert(material._bOwnsShader && raw.iRefCount==1);
    xgeMaterialFree(&material);assert(!raw.iRefCount);
    xge_shader_variant_set_t set={0};assert(xgeShaderVariantSetInit(&set,"void main(){}","void main(){}")==XGE_OK);
    assert(xgeShaderVariantGetOwned(&set,0,NULL,0,&shader)==XGE_ERROR_NOT_INITIALIZED && !shader && !set.iCount);
    assert(!xgeGraphicsOwnedResourceCount());xgeShaderVariantSetFree(&set);
}
static void proxy_storage(void)
{
    xui_proxy proxy=NULL;assert(xuiProxyXgeCreate(&proxy)==XUI_OK);
    assert(xgeGraphicsOwnedResourceCount()==1);
    xui_surface surface=NULL;assert(proxy->surfaceCreateRGBA(proxy,&surface,2,2,NULL,0,0)==XUI_OK);
    bool consumed=false;assert(xuiProxyReleaseOwnerChecked(proxy,&consumed)==XUI_OK && consumed);
    assert(xgeGraphicsOwnedResourceCount()==1 && xgeUnitChecked()==XGE_ERROR_BUSY);
    assert(xuiSurfaceReleaseOwnerChecked(surface,&consumed)==XUI_OK && consumed);
    assert(!xgeGraphicsOwnedResourceCount());
}
static void allocation_failures(void)
{
    for(int budget=0;budget<12;++budget){
        xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
        assert(xrtMemDebugFailAfter(budget));xge_texture texture=NULL;
        int code=xgeTextureCreateRGBAOwned(&texture,2,2,NULL,0);
        if(code==XGE_OK)assert(!xrtMemDebugFailTriggered());
        assert(xrtMemDebugFailAfter(-1));xrtClearError();
        assert((code==XGE_OK)==(texture!=NULL));xgeTextureFree(texture);
        assert(!xgeGraphicsOwnedResourceCount());
        assert(xrtMemDebugFailAfter(budget));xge_render_target target=NULL;
        code=xgeRenderTargetCreateOwned(&target,2,2);
        assert(xrtMemDebugFailAfter(-1));xrtClearError();
        assert((code==XGE_OK)==(target!=NULL));xgeRenderTargetFree(target);
        assert(!xgeGraphicsOwnedResourceCount());
        assert(xrtMemDebugFailAfter(budget));xge_buffer buffer=NULL;
        code=xgeBufferCreateOwned(&buffer,XGE_BUFFER_VERTEX,XGE_BUFFER_STATIC,"abc",3);
        assert(xrtMemDebugFailAfter(-1));xrtClearError();
        assert((code==XGE_OK)==(buffer!=NULL));xgeBufferFree(buffer);
        assert(!xgeGraphicsOwnedResourceCount());
        assert(xrtMemDebugFailAfter(budget));xui_proxy proxy=NULL;code=xuiProxyXgeCreate(&proxy);
        assert(xrtMemDebugFailAfter(-1));xrtClearError();
        assert((code==XUI_OK)==(proxy!=NULL));
        if(proxy){bool consumed=false;assert(xuiProxyReleaseOwnerChecked(proxy,&consumed)==XUI_OK && consumed);}
        xrtClearError();assert(!xgeGraphicsOwnedResourceCount());xrtMemDebugSnapshot(&after);
        assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
        assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    }
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    assert(xgeHostXrtRuntimeIdentity()==(xge_host_xrt_proc)xrtVersion);
    assert(xgeCheckBuildCompatibility(XGE_BUILD_ABI_FINGERPRINT,XGE_FEATURES)==XGE_OK);
    assert(xrtMemDebugEnable(true));shutdown_and_slots();
    xmemdebugsnapshot before,after;xrtClearError();xrtMemDebugSnapshot(&before);
    target_children();shaders();proxy_storage();allocation_failures();
    assert(xgeUnitChecked()==XGE_OK);xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("native owned graphics: Busy preserves engine/callbacks; target children/pass; checked slots; proxy physical storage; OOM rollback: PASS");
    return 0;
}
