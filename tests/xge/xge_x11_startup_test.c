/* Actual direct-XRT static XGE: failure returns through Run, balances Sokol
 * buffers, dispatches no frame, and permits cleanup/retry in the same process. */
#define _POSIX_C_SOURCE 200809L
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
    const char* display=getenv("DISPLAY");char* saved=display?strdup(display):NULL;
    assert(!display || saved);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xge_desc_t desc={0};desc.iWidth=128;desc.iHeight=96;desc.sTitle="XGE X11 checked startup";
    assert(xgeInit(&desc)==XGE_OK);
    for(unsigned i=0;i<12;++i){
        if(i%3==0)assert(unsetenv("DISPLAY")==0);
        else assert(setenv("DISPLAY",i%3==1?"":"not-a-valid-display-name",1)==0);
        xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
        int status=xgeRun(frame,NULL);xrtMemDebugSnapshot(&after);
        assert(status==XGE_ERROR_BACKEND_FAILED && !frames && !xrtGetError());
        assert(after.LiveCount==before.LiveCount && after.LiveBytes==before.LiveBytes);
        assert(after.InvalidFreeCount==before.InvalidFreeCount && after.DoubleFreeCount==before.DoubleFreeCount);
        assert(after.AllocCount-before.AllocCount==2 && after.FreeCount-before.FreeCount==2);
    }
    if(saved)assert(setenv("DISPLAY",saved,1)==0);else assert(unsetenv("DISPLAY")==0);
    free(saved);
    if(argc==2){
        assert(getenv("DISPLAY") && *getenv("DISPLAY"));
        assert(xgeRun(frame,NULL)==XGE_OK && frames==1);
    }
    assert(xgeSetFrameCallbackOwned(NULL,NULL,NULL)==XGE_OK && xgeUnitChecked()==XGE_OK && !xrtGetError());
    /* Manual/CPU mode needs no X11 display and must not acquire one. */
    assert(unsetenv("DISPLAY")==0);desc.iRunMode=XGE_RUN_MANUAL;
    int previous=frames;
    assert(xgeInit(&desc)==XGE_OK && xgeRun(frame,NULL)==XGE_OK && frames==previous);
    assert(xgeSetFrameCallbackOwned(NULL,NULL,NULL)==XGE_OK && xgeUnitChecked()==XGE_OK && !xrtGetError());
    printf("XGE X11: 12 checked failures, balanced buffers, manual mode and recovery=%d\n",previous);
    return 0;
}
