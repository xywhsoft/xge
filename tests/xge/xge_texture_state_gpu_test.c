/* Bounded real OpenGL smoke: two short windows, no input/network/audio test. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#if defined(_WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>
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

static int testMode;
static xge_texture yuv,guardTexture;
static const unsigned char y[9]={235,235,235,235,235,235,235,235,235};
static const unsigned char uv[4]={128,128,128,128};
static int frame(void* user)
{
    (void)user;assert(yuv->iBackendId && guardTexture->iBackendId);
    if(testMode==1){
        glBindTexture(GL_TEXTURE_2D,guardTexture->iBackendId);
        xge_sampler_t sampler=xgeSamplerDefault();sampler.iMinFilter=XGE_FILTER_NEAREST;
        assert(xgeTextureSetSampler(yuv,&sampler)==XGE_OK);
        GLint binding=0;glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding);
        assert((GLuint)binding==guardTexture->iBackendId);
    }else if(testMode==2){
        glBindTexture(GL_TEXTURE_2D,guardTexture->iBackendId);glPixelStorei(GL_UNPACK_ALIGNMENT,8);
        assert(xgeTextureUpdateYUV420P(yuv,y,3,uv,2,uv,2)==XGE_OK);
        GLint binding=0,alignment=0;glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding);glGetIntegerv(GL_UNPACK_ALIGNMENT,&alignment);
        assert((GLuint)binding==guardTexture->iBackendId && alignment==8);
        glPixelStorei(GL_UNPACK_ALIGNMENT,4);
    }else{
        xge_render_target target=NULL;xge_pass pass=NULL;
        assert(xgeRenderTargetCreateOwned(&target,2,2)==XGE_OK);
        assert(xgePassCreateOwned(&pass,target,XGE_PASS_CLEAR_COLOR,0xff00ffff)==XGE_OK);
        assert(xgePassBegin(pass)==XGE_OK);
        xge_draw_t draw={0};draw.pTexture=yuv;draw.tDst.fW=draw.tDst.fH=2;draw.iColor=0xffffffff;draw.iFlags=XGE_DRAW_SCREEN_SPACE;
        xgeDrawEx(&draw);assert(xgeFlush()==XGE_OK);assert(xgePassEnd(pass)==XGE_OK);
        unsigned char pixels[16];assert(xgeRenderTargetReadPixelsSized(target,pixels,sizeof(pixels),8)==XGE_OK);
        printf("YUV sample: %u %u %u %u\n",pixels[0],pixels[1],pixels[2],pixels[3]);fflush(stdout);
        for(int i=0;i<4;++i)assert(pixels[4*i]<=2 && pixels[4*i+1]<=2 && pixels[4*i+2]<=2 && pixels[4*i+3]==255);
        xgePassFree(pass);xgeRenderTargetFree(target);
    }
    glBindTexture(GL_TEXTURE_2D,0);xgeTextureFree(yuv);xgeTextureFree(guardTexture);return 1;
}
int main(int argc,char** argv)
{
    assert(argc==2);testMode=argv[1][0]-'0';assert(testMode>=1 && testMode<=3);
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    assert(xgeTextureCreateYUV420POwned(&yuv,3,3)==XGE_OK);
    const unsigned char pixel[4]={255,255,255,255};assert(xgeTextureCreateRGBAOwned(&guardTexture,1,1,pixel,4)==XGE_OK);
    if(testMode==3)assert(xgeTextureUpdateYUV420P(yuv,y,3,uv,2,uv,1)==XGE_ERROR_INVALID_ARGUMENT);
    xge_desc_t desc={0};desc.iWidth=64;desc.iHeight=64;desc.sTitle="XGE bounded texture state";
    assert(xgeInit(&desc)==XGE_OK && xgeRun(frame,NULL)==XGE_OK);
    assert(xgeUnitChecked()==XGE_OK && !xgeGraphicsOwnedResourceCount());
    printf("texture state %d passed\n",testMode);return 0;
}
