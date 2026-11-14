/* Actual direct-XRT static engine; Sokol pre-window buffers fail in sequence. */
#define XRT_MODULE_ALL
#include <xge.h>
#include <xge_static_xrt_contract.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int frames;
static int frame(void* unused){(void)unused;++frames;return 1;}
int main(int argc,char** argv)
{
    assert(argc==1 || (argc==2 && !strcmp(argv[1],"recover")));
    const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;
    assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xge_desc_t desc={0};desc.iWidth=128;desc.iHeight=96;desc.sTitle="XGE pre-window OOM";
    assert(xgeInit(&desc)==XGE_OK);
#ifdef _WIN32
    const int stages=1; /* XGE disables Sokol's native Win32 drop buffer. */
#else
    const int stages=2;
#endif
    for(int cycle=0;cycle<4;++cycle)for(int budget=0;budget<stages;++budget){
        xmemdebugsnapshot before,after;xrtClearError();xrtMemDebugSnapshot(&before);
        assert(xrtMemDebugFailAfter(budget));int status=xgeRun(frame,NULL);xrtMemDebugFailClear();
        xrtClearError();xrtMemDebugSnapshot(&after);
        assert(status==XGE_ERROR_OUT_OF_MEMORY && !frames);
        assert(after.AllocCount-before.AllocCount==(uint64_t)budget && after.FreeCount-before.FreeCount==(uint64_t)budget);
        assert(after.LiveCount==before.LiveCount && after.LiveBytes==before.LiveBytes);
        assert(after.InvalidFreeCount==before.InvalidFreeCount && after.DoubleFreeCount==before.DoubleFreeCount);
    }
    if(argc==2)assert(xgeRun(frame,NULL)==XGE_OK && frames==1);
    assert(xgeSetFrameCallbackOwned(NULL,NULL,NULL)==XGE_OK && xgeUnitChecked()==XGE_OK);
    desc.iRunMode=XGE_RUN_MANUAL;
    int previous=frames;assert(xgeInit(&desc)==XGE_OK && xgeRun(frame,NULL)==XGE_OK && frames==previous);
    assert(xgeSetFrameCallbackOwned(NULL,NULL,NULL)==XGE_OK && xgeUnitChecked()==XGE_OK && !xrtGetError());
    printf("XGE state OOM: %d native stages, balanced buffers, no frames, manual mode, recovery=%d\n",stages,previous);
    return 0;
}
