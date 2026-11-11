/* Actual host/static public API, also compiled as C++11/C++17. CPU only. */
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
static xatomic32 ready=XRT_ATOMIC32_INIT(0),go=XRT_ATOMIC32_INIT(0);
static int32_t refWorker(void* raw)
{
    xge_emoji_pack pack=(xge_emoji_pack)raw;
    xrtAtomic32FetchAdd(&ready,1,XMEMORY_RELEASE);
    while(!xrtAtomic32Load(&go,XMEMORY_ACQUIRE))xrtThreadYield();
    for(unsigned i=0;i<10000;++i){assert(xgeEmojiPackAddRef(pack)>1);xgeEmojiPackFree(pack);}return 0;
}
static void reenterError(const xge_error_info_t* error,void* raw)
{
    assert(error && error->iCode==XGE_ERROR_OUT_OF_MEMORY);++*(int*)raw;
    xgeEmojiPackClearDefault();
}
int main(void)
{
#if defined(XGE_TEST_STATIC)
    const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;
    assert(contract.revision==xgeStaticXrtContractV1.revision && contract.dataModel==xgeStaticXrtContractV1.dataModel &&
        contract.coreLayout==xgeStaticXrtContractV1.coreLayout && !strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    xge_desc_t desc;memset(&desc,0,sizeof(desc));desc.iWidth=desc.iHeight=16;desc.iRunMode=XGE_RUN_MANUAL;
    desc.sTitle="CPU Emoji public API";assert(xgeInit(&desc)==XGE_OK);
    const char svg[]="<svg width='8' height='8'><rect width='8' height='8'/></svg>";
    xge_emoji_pack pack=NULL;assert(xgeEmojiPackCreate(&pack)==XGE_OK && pack);
    uint32_t id=0;assert(xgeEmojiPackAddSvgMemory(pack,"A",svg,sizeof(svg)-1,NULL,&id)==XGE_OK && id==1);
    assert(xgeEmojiPackAddSvgMemory(pack,"AB",svg,sizeof(svg)-1,NULL,&id)==XGE_OK && id==2);
    xge_emoji_match_t match;memset(&match,0,sizeof(match));
    assert(xgeEmojiPackMatch(pack,"ABtail",6,&match)==XGE_OK && match.iEmojiId==2 && match.iTextSize==2);
    assert(xrtMemDebugFailAfter(0));
    assert(xgeEmojiPackAddSvgMemory(pack,"Z\xff",svg,sizeof(svg)-1,NULL,NULL)==XGE_ERROR_RESOURCE_FAILED);
    assert(!xrtMemDebugFailTriggered());
    assert(xgeEmojiPackAddSvgMemory(pack,"new",svg,sizeof(svg)-1,NULL,NULL)==XGE_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();xrtClearError();
    assert(xgeEmojiPackMatch(pack,"A",1,&match)==XGE_OK && match.iEmojiId==1);
    assert(xgeEmojiPackSetDefault(pack)==XGE_OK);xge_emoji_pack shared=NULL;
    assert(xgeEmojiPackGetDefault(&shared)==XGE_OK && shared==pack);xgeEmojiPackFree(shared);
    xthread* jobs[3];
    for(unsigned i=0;i<3;++i){jobs[i]=xrtThreadCreate(refWorker,pack,0);assert(jobs[i]);}
    while(xrtAtomic32Load(&ready,XMEMORY_ACQUIRE)!=3)xrtThreadYield();
    xrtAtomic32Store(&go,1,XMEMORY_RELEASE);
    for(unsigned i=0;i<3;++i){assert(xrtThreadWaitFor(jobs[i],30000)==XWAIT_OK && !xrtThreadExitCode(jobs[i]));xrtThreadDestroy(jobs[i]);}
    assert(xgeEmojiPackAddRef(pack)==3);xgeEmojiPackFree(pack);
    int callbacks=0;assert(xgeSetErrorCallback(reenterError,&callbacks)==XGE_OK);
    assert(xrtMemDebugFailAfter(0));xge_emoji_pack builtin=NULL;int status=xgeEmojiPackLoadBuiltin(&builtin);
    xrtMemDebugFailClear();xrtClearError();assert(status==XGE_ERROR_OUT_OF_MEMORY && !builtin && callbacks==1);
    assert(xgeSetErrorCallback(NULL,NULL)==XGE_OK);
    assert(xgeEmojiPackAddRef(pack)==2);xgeEmojiPackFree(pack);
    assert(xgeEmojiPackLoadBuiltin(&builtin)==XGE_OK && builtin);
    const char grin[]="\xf0\x9f\x98\x80";xmemdebugsnapshot hot;xrtMemDebugSnapshot(&hot);assert(xrtMemDebugFailAfter(0));
    for(unsigned i=0;i<1000;++i){
        shared=NULL;assert(xgeEmojiPackLoadBuiltin(&shared)==XGE_OK && shared==builtin);
        assert(xgeEmojiPackMatch(shared,grin,sizeof(grin)-1,&match)==XGE_OK && match.iTextSize==4);
        xgeEmojiPackFree(shared);
    }
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtMemDebugSnapshot(&after);
    assert(hot.AllocCount==after.AllocCount && hot.FreeCount==after.FreeCount && hot.ReallocCount==after.ReallocCount);
    assert(xgeEmojiPackSetDefault(builtin)==XGE_OK);xgeEmojiPackFree(pack);
    assert(xgeUnitChecked()==XGE_OK && !xgeGraphicsOwnedResourceCount());
    /* The caller's independent CPU owner survives both global slot drops. */
    assert(xgeEmojiPackAddRef(builtin)==2);xgeEmojiPackFree(builtin);
    assert(xgeEmojiPackMatch(builtin,grin,sizeof(grin)-1,&match)==XGE_OK);xgeEmojiPackFree(builtin);
    xrtMemDebugSnapshot(&after);assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("Public Emoji host/static ABI, shared refs, callback reentry, Unit ownership and 1000 hot hits passed");return 0;
}
