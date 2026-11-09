/* Exact byte capacities, unchanged failures and native owner rollback; no GPU. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
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

static const unsigned char original[16]={1,2,3,255, 4,5,6,255, 7,8,9,255, 10,11,12,255};

static void check_pixels(xge_texture texture,const unsigned char* expected)
{
    unsigned char pixels[16];
    assert(xgeTextureReadPixelsSized(texture,pixels,sizeof(pixels),0)==XGE_OK);
    assert(!memcmp(pixels,expected,sizeof(pixels)));
}

static void rows(void)
{
    xge_texture texture=NULL;
    assert(xgeTextureCreateRGBAExOwned(&texture,2,2,original,sizeof(original),XGE_TEXTURE_COMPRESS_NONE)==XGE_OK);
    unsigned char output[24];memset(output,0xA5,sizeof(output));
    assert(xgeTextureReadPixelsSized(texture,output,19,12)==XGE_ERROR_INVALID_ARGUMENT);
    for(size_t i=0;i<sizeof(output);++i)assert(output[i]==0xA5);
    assert(xgeTextureReadPixelsSized(texture,output,20,12)==XGE_OK);
    assert(!memcmp(output,original,8) && !memcmp(output+12,original+8,8));
    for(int i=8;i<12;++i)assert(output[i]==0xA5);
    for(int i=20;i<24;++i)assert(output[i]==0xA5);

    const unsigned char patch[12]={21,22,23,255, 0,0,0,0, 24,25,26,255};
    assert(xgeTextureUpdateRGBASized(texture,1,0,1,2,patch,11,8)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeTextureUpdateRGBASized(texture,0,0,INT_MAX,2,patch,sizeof(patch),0)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeTextureUpdateRGBASized(texture,INT_MAX,0,1,1,patch,sizeof(patch),0)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeTextureUpdateRGBASized(texture,0,0,1,2,patch,sizeof(patch),3)==XGE_ERROR_INVALID_ARGUMENT);
    check_pixels(texture,original);
    assert(xgeTextureUpdateRGBASized(texture,1,0,1,2,patch,sizeof(patch),8)==XGE_OK);
    unsigned char expected[16];memcpy(expected,original,sizeof(expected));
    memcpy(expected+4,patch,4);memcpy(expected+12,patch+8,4);check_pixels(texture,expected);

    xge_sampler_t sampler=xgeSamplerDefault(),before=xgeTextureGetSampler(texture);
    sampler.iWrapS=INT_MAX;
    assert(xgeTextureSetSampler(texture,&sampler)==XGE_ERROR_INVALID_ARGUMENT);
    sampler=xgeTextureGetSampler(texture);assert(!memcmp(&sampler,&before,sizeof(sampler)));
    xge_texture_storage_info_t info={0};info.iSize=sizeof(info);
    assert(xgeTextureGetStorageInfo(texture,&info)==XGE_OK && info.iCompression==XGE_TEXTURE_COMPRESS_NONE && info.iCpuBytes==16);
    xgeTextureFree(texture);

    assert(xgeTextureCreateYUV420POwned(&texture,3,3)==XGE_OK);
    const unsigned char y[9]={0},uv[4]={128,128,128,128};
    assert(xgeTextureUpdateYUV420PSized(texture,y,9,3,uv,4,2,uv,3,2)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeTextureUpdateYUV420PSized(texture,y,9,3,uv,4,2,uv,4,2)==XGE_OK);
    xgeTextureFree(texture);
}

static void factories(const char* path)
{
    void* png=NULL;size_t pngSize=0;
    assert(xgeImageEncodePNG(2,2,original,8,&png,&pngSize)==XGE_OK && pngSize<=INT_MAX);
    FILE* file=fopen(path,"wb");assert(file);
    assert(fwrite(png,1,pngSize,file)==pngSize && !fclose(file));
    xge_texture texture=NULL;
    assert(xgeTextureLoadMemoryExOwned(&texture,png,(int)pngSize,0)==XGE_OK);check_pixels(texture,original);xgeTextureFree(texture);
    assert(xgeTextureLoadExOwned(&texture,path,0)==XGE_OK);check_pixels(texture,original);xgeTextureFree(texture);
    xge_image_t image={0};assert(xgeImageLoadMemory(&image,png,(int)pngSize)==XGE_OK);
    assert(xgeTextureCreateFromImageOwned(&texture,&image,XGE_TEXTURE_COMPRESS_NONE)==XGE_OK);
    xgeImageFree(&image);check_pixels(texture,original);xgeTextureFree(texture);

    for(int budget=0;budget<12;++budget){
        xmemdebugsnapshot before,after;xrtClearError();xrtMemDebugSnapshot(&before);
        assert(xrtMemDebugFailAfter(budget));texture=(void*)(uintptr_t)1;
        int result=xgeTextureLoadMemoryExOwned(&texture,png,(int)pngSize,0);
        assert(xrtMemDebugFailAfter(-1));xrtClearError();
        assert((result==XGE_OK)==(texture!=NULL));xgeTextureFree(texture);
        assert(!xgeGraphicsOwnedResourceCount());xrtMemDebugSnapshot(&after);
        assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    }
    xrtFree(png);assert(!remove(path));
}

static void fallback(void)
{
    assert(xgeTextureFallbackSetRGBASized(2,2,original,sizeof(original))==XGE_OK);
    for(int budget=0;budget<5;++budget){
        const unsigned char replacement[4]={100,110,120,255};
        assert(xrtMemDebugFailAfter(budget));
        int result=xgeTextureFallbackSetRGBASized(1,1,replacement,sizeof(replacement));
        assert(xrtMemDebugFailAfter(-1));xrtClearError();
        xge_texture copy=NULL;assert(xgeTextureFallbackGetOwned(&copy)==XGE_OK);
        if(result==XGE_OK){assert(copy->iWidth==1);unsigned char pixels[4];assert(xgeTextureReadPixelsSized(copy,pixels,4,0)==XGE_OK && !memcmp(pixels,replacement,4));}
        else{assert(copy->iWidth==2);check_pixels(copy,original);}
        xgeTextureFree(copy);
        assert(xgeTextureFallbackSetRGBASized(2,2,original,sizeof(original))==XGE_OK);
    }
    assert(xgeTextureFallbackSetRGBASized(2,2,original,15)==XGE_ERROR_INVALID_ARGUMENT);
    xge_texture copy=NULL;assert(xgeTextureFallbackGetOwned(&copy)==XGE_OK);check_pixels(copy,original);xgeTextureFree(copy);
    xgeTextureFallbackClear();copy=(void*)(uintptr_t)1;
    assert(xgeTextureFallbackGetOwned(&copy)==XGE_ERROR_RESOURCE_FAILED && !copy);
}

int main(int argc,char** argv)
{
    assert(argc==2);const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    rows();factories(argv[1]);fallback();assert(!xgeGraphicsOwnedResourceCount());
    assert(xgeUnitChecked()==XGE_OK);xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("Texture: exact/padded rows, canaries, unchanged failures, owned flags/image/path/memory/fallback and OOM passed");return 0;
}
