/* Real public host/static API; no graphics context or window is needed. */
#if !defined(_WIN32) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
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
#include <string.h>
#include <limits.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif
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
static const char outputPath[]="image-png-fixture-output.png";
static void equalHeap(const xmemdebugsnapshot* before)
{
    xmemdebugsnapshot after;xrtMemDebugSnapshot(&after);
    assert(before->LiveCount==after.LiveCount && before->LiveBytes==after.LiveBytes &&
        before->InvalidFreeCount==after.InvalidFreeCount && before->DoubleFreeCount==after.DoubleFreeCount);
}
static void checkImage(const void* data,size_t bytes,unsigned first,unsigned second)
{
    assert(bytes>8 && bytes<=INT_MAX);int width=0,height=0;
    assert(xrtMemDebugFailAfter(0));
    assert(xgeImageInfoMemory(data,(int)bytes,&width,&height)==XGE_OK && width==1 && height==2);
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
    xge_image_t image;memset(&image,0,sizeof(image));
    assert(xgeImageLoadMemoryEx(&image,data,(int)bytes,XGE_IMAGE_STRAIGHT_ALPHA)==XGE_OK);
    const unsigned char* pixels=(const unsigned char*)xgeImageGetPixels(&image);
    assert(pixels && pixels[0]==first && pixels[1]==0 && pixels[3]==128 &&
        pixels[4]==0 && pixels[5]==second && pixels[7]==128);
    xgeImageFree(&image);
}
static void checkFile(unsigned first,unsigned second)
{
    unsigned char encoded[1024];FILE* file=fopen(outputPath,"rb");assert(file);
    size_t size=fread(encoded,1,sizeof(encoded),file);assert(!ferror(file) && !fclose(file));
    checkImage(encoded,size,first,second);assert(!remove(outputPath));
}
static void guardRows(void)
{
    /* Two 4-byte rows, 8-byte stride: the last row ends at the guard page.
     * The four padding bytes between rows are valid; final padding is not. */
    size_t page;
    unsigned char* memory;
#if defined(_WIN32)
    SYSTEM_INFO info;GetSystemInfo(&info);page=info.dwPageSize;
    memory=(unsigned char*)VirtualAlloc(NULL,page*2,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    assert(memory);DWORD previous;
    assert(VirtualProtect(memory+page,page,PAGE_NOACCESS,&previous));
#else
    page=(size_t)sysconf(_SC_PAGESIZE);
    memory=(unsigned char*)mmap(NULL,page*2,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    assert(memory!=MAP_FAILED && !mprotect(memory+page,page,PROT_NONE));
#endif
    unsigned char* pixels=memory+page-12;
    const unsigned char values[12]={64,0,0,128,0,0,0,0,0,64,0,128};
    memcpy(pixels,values,sizeof(values));
    assert(xgeImageSavePNGEx(outputPath,1,2,pixels,8,XGE_IMAGE_PREMULTIPLIED)==XGE_OK);
    checkFile(128,128);
    assert(xgeImageSavePNG(outputPath,1,2,pixels,8)==XGE_OK);checkFile(64,64);
    void* encoded=NULL;size_t size=0;
    assert(xgeImageEncodePNGEx(1,2,pixels,8,XGE_IMAGE_PREMULTIPLIED,&encoded,&size)==XGE_OK);
    checkImage(encoded,size,128,128);xrtFree(encoded);
#if defined(_WIN32)
    assert(VirtualFree(memory,0,MEM_RELEASE));
#else
    assert(!munmap(memory,page*2));
#endif
    puts("PNG final-row guard and alpha roundtrip passed");
}
static void failedDevice(void)
{
#if !defined(_WIN32)
    const unsigned char pixels[8]={64,0,0,128,0,64,0,128};
    assert(xgeImageSavePNGEx("/dev/full",1,2,pixels,4,XGE_IMAGE_STRAIGHT_ALPHA)!=XGE_OK);
    puts("PNG failed device rejects success");
#endif
}
static void invalidInputs(void)
{
    const unsigned char pixels[8]={64,0,0,128,0,64,0,128};
    assert(xgeImageSavePNGEx(outputPath,0,2,pixels,4,0)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeImageSavePNGEx(outputPath,1,2,pixels,3,0)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeImageSavePNGEx(outputPath,1,2,pixels,4,UINT32_MAX)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeImageSavePNGEx(outputPath,1,INT_MAX,pixels,4,0)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeImageSavePNGEx(outputPath,INT_MAX/4,1,pixels,0,0)==XGE_ERROR_INVALID_ARGUMENT);
    void* encoded=(void*)pixels;size_t size=123;
    assert(xgeImageEncodePNGEx(1,INT_MAX,pixels,4,0,&encoded,&size)==XGE_ERROR_INVALID_ARGUMENT);
    assert(encoded==NULL && size==0);
    /* A complete SOF header still takes the real JPEG path after its marker
     * precheck. Dimensions are an info query, not complete-file validation. */
    const unsigned char jpeg[]={255,216,255,192,0,11,8,0,2,0,1,1,1,17,0};
    int width=0,height=0;
    assert(xgeImageInfoMemory(jpeg,sizeof(jpeg),&width,&height)==XGE_OK && width==1 && height==2);
}
static void prefixes(void)
{
    const unsigned char pixels[8]={64,0,0,128,0,64,0,128};
    for(unsigned mode=0;mode<2;++mode){
        unsigned failed=0,passed=0;
        for(int budget=0;budget<64;++budget){
            xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
            assert(xrtMemDebugFailAfter(budget));
            int status=xgeImageSavePNGEx(outputPath,1,2,pixels,4,
                mode?XGE_IMAGE_PREMULTIPLIED:XGE_IMAGE_STRAIGHT_ALPHA);
            xrtMemDebugFailClear();xrtClearError();
            if(status==XGE_OK){++passed;checkFile(mode?128:64,mode?128:64);}
            else {++failed;(void)remove(outputPath);}
            equalHeap(&before);
        }
        assert(failed && passed);printf("PNG save prefixes: %u %u %u\n",mode,failed,passed);
    }
}
int main(int argc,char** argv)
{
#if defined(XGE_TEST_STATIC)
    const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;
    assert(contract.revision==xgeStaticXrtContractV1.revision && !strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
    if(argc>1){
        if(!strcmp(argv[1],"guard"))guardRows();
        else if(!strcmp(argv[1],"device"))failedDevice();
        else assert(!"Unknown PNG fixture mode");
    }else {guardRows();failedDevice();invalidInputs();prefixes();}
    equalHeap(&before);puts("Public PNG host/static bounds, errors and ownership passed");return 0;
}
