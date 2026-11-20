#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include "xui.h"
#if !defined(XUI_TEST_STATIC)
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#endif
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <string.h>

#if !defined(XUI_TEST_STATIC)
static xge_host_xrt_proc resolve(void* context,const char* name)
{
    (void)context;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif

static void compareRawAndOwned(void)
{
    const char text[]="A\0tag_12 tag tag_3";
    const char pattern[]="tag(_([0-9]+))?";
    xui_find_result_t raw[3]={0};
    int total=0;
    size_t count=0;
    char diagnostic[48];
    xui_find_matches_t* matches=NULL;
    const xui_find_result_t* items=NULL;
    uint32_t flags=XUI_FIND_REGEX | XUI_FIND_BACKWARD | XUI_FIND_WRAP;
    assert(xuiFindCollectText(text,sizeof(text)-1,pattern,0,-1,flags,raw,3,&total,NULL,0)==XUI_OK);
    assert(total==3);
    assert(xuiFindCollectMatches(text,sizeof(text)-1,pattern,0,-1,flags,2,
        &matches,diagnostic,sizeof(diagnostic))==XUI_OK);
    assert(!diagnostic[0] && xuiFindMatchesCount(matches)==3);
    assert(xuiFindMatchesAddRef(matches)==XUI_OK);
    assert(xuiFindMatchesGet(matches,&items,&count)==XUI_OK && count==2);
    xuiFindMatchesRelease(matches);
    for(size_t i=0;i<count;++i){
        assert(items[i].iStart==raw[i].iStart && items[i].iEnd==raw[i].iEnd);
        assert(items[i].arrCaptures[2].iStart==raw[i].arrCaptures[2].iStart);
    }
    assert(items[0].arrCaptures[2].iEnd==8 && items[1].arrCaptures[2].iStart==-1);
    xuiFindMatchesRelease(matches);

    assert(xuiFindCollectMatches(NULL,0,"^",0,-1,XUI_FIND_REGEX,-1,&matches,NULL,0)==XUI_OK);
    assert(xuiFindMatchesCount(matches)==1);
    assert(xuiFindMatchesGet(matches,&items,&count)==XUI_OK && count==1);
    assert(items[0].iStart==0 && items[0].iEnd==0);
    xuiFindMatchesRelease(matches);
    assert(xuiFindCollectMatches(text,sizeof(text)-1,pattern,0,-1,flags,0,&matches,NULL,0)==XUI_OK);
    assert(xuiFindMatchesCount(matches)==3);
    assert(xuiFindMatchesGet(matches,&items,&count)==XUI_OK && !items && count==0);
    xuiFindMatchesRelease(matches);
    assert(xuiFindCollectMatches(text,sizeof(text)-1,pattern,0,-1,flags,-2,&matches,
        diagnostic,sizeof(diagnostic))==XUI_ERROR_INVALID_ARGUMENT);
    assert(!matches && diagnostic[0]);
}

static void rollbackAndAllocationShape(void)
{
    char text[35];
    memset(text,'x',sizeof(text));
    xmemdebugsnapshot before,after;
    int failed=0;
    for(int i=0;i<20;++i){
        xui_find_matches_t* matches=NULL;
        const xui_find_result_t* items=NULL;
        size_t count=0;
        xrtMemDebugSnapshot(&before);
        assert(xrtMemDebugFailAfter(i));
        int status=xuiFindCollectMatches(text,sizeof(text),"x",0,-1,0,-1,&matches,NULL,0);
        bool triggered=xrtMemDebugFailTriggered();
        xrtMemDebugFailClear();
        xrtClearError();
        if(triggered){
            ++failed;
            assert(status==XUI_ERROR_OUT_OF_MEMORY && !matches);
        } else {
            assert(status==XUI_OK && xuiFindMatchesCount(matches)==35);
            assert(xuiFindMatchesGet(matches,&items,&count)==XUI_OK && count==35);
            assert(items[34].iStart==34 && items[34].iEnd==35);
            xuiFindMatchesRelease(matches);
        }
        xrtMemDebugSnapshot(&after);
        assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
        assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
        if(!triggered)break;
    }
    assert(failed>=4 && failed<20);
}

static void replacementBytes(void)
{
    const char text[]="X\0tag_12 Z";
    const char pattern[]="tag_([0-9]+)";
    uint32_t flags=XUI_FIND_REGEX;
    xui_find_result_t match;
    char* output=NULL;
    int length=0;
    int count=0;
    assert(xuiFindLocateText(text,sizeof(text)-1,pattern,0,0,-1,flags,&match,NULL,0)==1);
    assert(xuiFindBuildReplacement(text,sizeof(text)-1,pattern,"[$1]",flags,&match,
        &output,&length,NULL,0)==XUI_OK);
    assert(length==4 && !memcmp(output,"[12]",length));
    xuiFindFreeText(output);
    assert(xuiFindReplaceAllText(text,sizeof(text)-1,pattern,"[$1]",0,-1,flags,
        &output,&length,&count,NULL,0)==XUI_OK);
    assert(length==8 && count==1 && !memcmp(output,"X\0[12] Z",length));
    xrtFree(output);
    assert(xuiFindLocateText(NULL,0,"^",0,0,-1,flags,&match,NULL,0)==1);
    assert(xuiFindBuildReplacement(NULL,0,"^","!",flags,&match,&output,&length,NULL,0)==XUI_OK);
    assert(length==1 && output[0]=='!');
    xuiFindFreeText(output);
}

int main(void)
{
#if !defined(XUI_TEST_STATIC)
    const xge_host_xrt_contract_t abi=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&abi,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    compareRawAndOwned();
    rollbackAndAllocationShape();
    replacementBytes();
    puts("XUI_FIND_MATCHES_NATIVE_OK");
    return 0;
}
