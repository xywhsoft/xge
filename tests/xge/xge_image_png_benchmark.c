/* Same pixel workload for native codec comparison; no language bridge claim.
 * Link to a matched host/static product and its sole XRT runtime. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include "xge.h"
#if defined(XGE_TEST_STATIC)
#include <xge_static_xrt_contract.h>
#else
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#endif
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if !defined(XGE_TEST_STATIC)
static xge_host_xrt_proc resolve(void* unused,const char* name)
{
    (void)unused;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif
static void sample(const char* name,int side,int iterations,int noise)
{
    size_t bytes=(size_t)side*(size_t)side*4;
    unsigned char* pixels=(unsigned char*)xrtMalloc(bytes);assert(pixels);
    uint32_t random=UINT32_C(1234567);
    for(int y=0;y<side;++y)for(int x=0;x<side;++x){
        unsigned char* p=pixels+((size_t)y*(size_t)side+(size_t)x)*4;
        random=random*UINT32_C(1664525)+UINT32_C(1013904223);
        p[0]=(unsigned char)(noise?random:(uint32_t)x);p[1]=(unsigned char)(noise?random>>8:(uint32_t)y);
        p[2]=(unsigned char)(noise?random>>16:(uint32_t)(x+y));p[3]=255;
    }
    for(unsigned round=0;round<3;++round){
        xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
        size_t encodedBytes=0;clock_t start=clock();
        for(int i=0;i<iterations;++i){
            void* encoded=NULL;size_t size=0;
            assert(xgeImageEncodePNGEx(side,side,pixels,side*4,XGE_IMAGE_STRAIGHT_ALPHA,&encoded,&size)==XGE_OK);
            assert(size && encoded);encodedBytes=size;
            if(i==0){
                xge_image_t decoded;memset(&decoded,0,sizeof(decoded));
                assert(size<=INT32_MAX && xgeImageLoadMemoryEx(&decoded,encoded,(int)size,XGE_IMAGE_STRAIGHT_ALPHA)==XGE_OK);
                assert(decoded.iWidth==side && decoded.iHeight==side && !memcmp(decoded.pPixels,pixels,bytes));
                xgeImageFree(&decoded);
            }
            xrtFree(encoded);
        }
        double seconds=(double)(clock()-start)/(double)CLOCKS_PER_SEC;
        xrtMemDebugSnapshot(&after);
        assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes &&
            before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
        printf("{\"workload\":\"%s\",\"round\":%u,\"side\":%d,\"iterations\":%d,\"bytes\":%zu,\"seconds\":%.9f,\"alloc\":%llu,\"realloc\":%llu}\n",
            name,round,side,iterations,encodedBytes,seconds,
            (unsigned long long)(after.AllocCount-before.AllocCount),(unsigned long long)(after.ReallocCount-before.ReallocCount));
    }
    xrtFree(pixels);
}
extern int stbi_write_png_compression_level;
int main(int argc,char** argv)
{
#if !defined(XGE_TEST_STATIC)
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
    assert(xrtMemDebugEnable(true));
    if(argc==2){
        int quality=atoi(argv[1]);assert(quality>=0 && quality<=10);
        stbi_write_png_compression_level=quality;
    }else assert(argc==1);
    sample("icon-gradient",16,100,0);
    sample("image-gradient",256,10,0);
    sample("image-noise",256,10,1);
    return 0;
}
