/* Bounded real OpenGL smoke: two short windows, no input/network/audio test. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#include <assert.h>
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
typedef struct State {int calls;xge_texture texture;xge_shader shader;} State;
static const char vertex[]="#version 330\nin vec2 aPosition;void main(){gl_Position=vec4(aPosition,0.0,1.0);}";
static const char fragment[]="#version 330\nuniform vec4 uTint;out vec4 fragColor;void main(){fragColor=uTint;}";
static int render(void* raw)
{
    State* state=raw;++state->calls;
    xge_render_target first=NULL,second=NULL;
    assert(xgeRenderTargetCreateOwned(&first,2,2)==XGE_OK && xgeRenderTargetCreateOwned(&second,2,2)==XGE_OK);
    xge_pass outer=NULL,inner=NULL;
    assert(xgePassCreateOwned(&outer,first,XGE_PASS_CLEAR_COLOR,XGE_COLOR_RGBA(10,20,30,255))==XGE_OK);
    assert(xgePassCreateOwned(&inner,second,XGE_PASS_CLEAR_COLOR,XGE_COLOR_RGBA(40,50,60,255))==XGE_OK);
    assert(xgePassBegin(outer)==XGE_OK && xgePassBegin(inner)==XGE_OK);
    bool consumed=true;assert(xgePassReleaseOwnerChecked(outer,&consumed)==XGE_ERROR_BUSY && !consumed);
    assert(xgePassIsCurrent(inner) && xgeRenderTargetResize(first,3,3)==XGE_ERROR_BUSY);
    assert(xgePassReleaseOwnerChecked(inner,&consumed)==XGE_OK && consumed && xgePassIsCurrent(outer));
    assert(xgePassEnd(outer)==XGE_OK && xgePassIsIdle());
    unsigned char pixels[16];assert(xgeRenderTargetReadPixelsSized(first,pixels,sizeof(pixels),8)==XGE_OK);
    for(int i=0;i<4;++i)assert(pixels[4*i]==10 && pixels[4*i+1]==20 && pixels[4*i+2]==30 && pixels[4*i+3]==255);
    assert(xgeRenderTargetReadPixelsSized(second,pixels,sizeof(pixels),8)==XGE_OK);
    for(int i=0;i<4;++i)assert(pixels[4*i]==40 && pixels[4*i+1]==50 && pixels[4*i+2]==60 && pixels[4*i+3]==255);
    assert(xgePassReleaseOwnerChecked(outer,&consumed)==XGE_OK && consumed);
    state->texture=xgeRenderTargetTexture(first);assert(xgeTextureAddRef(state->texture)==2);
    xgeRenderTargetFree(first);xgeRenderTargetFree(second);
    xge_shader shader=NULL;assert(xgeShaderCreateOwned(&shader,vertex,fragment)==XGE_OK);xgeShaderFree(shader);
    xge_shader_variant_set_t variants={0};assert(xgeShaderVariantSetInit(&variants,vertex,fragment)==XGE_OK);
    xge_shader_define_t define={0};strcpy(define.sName,"USE_COLOR");define.iValue=1;
    assert(xgeShaderVariantGetOwned(&variants,0,&define,1,&state->shader)==XGE_OK);
    xgeShaderVariantSetFree(&variants);assert(state->shader->iRefCount==1 && state->shader->iProgram);
    xge_material_t material;xgeMaterialInit(&material);
    assert(xgeMaterialSetShaderChecked(&material,state->shader)==XGE_OK && state->shader->iRefCount==2);
    assert(xgeShaderUniform4f(state->shader,"uTint",1,0,0,1)==XGE_OK);
    xgeMaterialFree(&material);assert(state->shader->iRefCount==1);
    return 1;
}
static int empty_frame(void* raw){++*(int*)raw;return 1;}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    xge_desc_t desc={0};desc.iWidth=128;desc.iHeight=96;desc.sTitle="XGE bounded ownership test";
    State state={0};assert(xgeInit(&desc)==XGE_OK && xgeRun(render,&state)==XGE_OK && state.calls==1);
    assert(xgeGraphicsOwnedResourceCount()==2 && xgeUnitChecked()==XGE_ERROR_BUSY);
    assert(xgeInit(&desc)==XGE_ERROR_BUSY && xgeRun(render,&state)==XGE_ERROR_BUSY && state.calls==1);
    /* Physical backend has ended: releasing old handles must not call GL. */
    xgeTextureFree(state.texture);xgeShaderFree(state.shader);
    assert(!xgeGraphicsOwnedResourceCount() && xgeUnitChecked()==XGE_OK);
    int calls=0;assert(xgeInit(&desc)==XGE_OK && xgeRun(empty_frame,&calls)==XGE_OK && calls==1);
    assert(xgeUnitChecked()==XGE_OK && !xrtGetError());
    puts("real OpenGL: nested Pass restore/readback, shader/variant retention, ended-context Busy, release and fresh restart: PASS");
    return 0;
}
