/* The real static engine calls the same instrumented platform entries. */
#define _POSIX_C_SOURCE 200809L
#define XRT_MODULE_ALL
#include <xge.h>
#include <xge_static_xrt_contract.h>
#include <X11/Xlib.h>
#include "linux_startup_faults.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned frames,application_errors;
static int frame(void* unused){(void)unused;++frames;return 1;}
static int app_error(Display* display,XErrorEvent* error){(void)display;(void)error;++application_errors;return 0;}
int main(int argc,char** argv)
{
    assert(argc==2);int mode=atoi(argv[1]);assert(mode>0 && mode<=STARTUP_SECOND_CONFIGS);
    const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;
    assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
    assert(xrtMemDebugEnabled()||xrtMemDebugEnable(true));
    xge_desc_t desc={0};desc.iWidth=128;desc.iHeight=96;desc.sTitle="Checked XGE GLX startup";
    assert(xgeInit(&desc)==XGE_OK);XErrorHandler original=XSetErrorHandler(app_error);
    for(unsigned attempt=0;attempt<3;++attempt){
        startup_fault_reset(mode);xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
        if(mode==STARTUP_ALLOCATION)assert(xrtMemDebugFailAfter(2));
        int status=xgeRun(frame,NULL);xrtMemDebugFailClear();xrtClearError();xrtMemDebugSnapshot(&after);
        int expected=(mode==STARTUP_ALLOCATION || mode==STARTUP_HINTS)?XGE_ERROR_OUT_OF_MEMORY:XGE_ERROR_BACKEND_FAILED;
        assert(status==expected && !frames && !application_errors);
        assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
        assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
        startup_fault_balanced();assert(XSetErrorHandler(app_error)==app_error);
    }
    startup_fault_reset(STARTUP_HEALTHY);
    for(unsigned attempt=0;attempt<3;++attempt){
        int status=xgeRun(frame,NULL);
        if(status!=XGE_OK || frames!=attempt+1 || application_errors)fprintf(stderr,"Healthy engine attempt=%u status=%d frames=%u applicationErrors=%u\n",attempt,status,frames,application_errors);
        assert(status==XGE_OK && frames==attempt+1 && !application_errors);
        /* XGE retains its resolver until checked Unit. Sokol has already
         * retired its independent owner after physical window teardown. */
        startup_fault_balanced_libraries(1);
        assert(xgeSetFrameCallbackOwned(NULL,NULL,NULL)==XGE_OK && xgeUnitChecked()==XGE_OK && !xrtGetError());
        startup_fault_balanced();
        /* A completed graphics lifetime is ENDED; the next healthy run needs
         * fresh initialization. Pre-startup failures still permit direct retry. */
        if(attempt<2)assert(xgeInit(&desc)==XGE_OK);
    }
    startup_fault_balanced();assert(XSetErrorHandler(original)==app_error);
    printf("XGE GLX stage %d: three checked balanced failures and same-process actual engine retry passed\n",mode);
    return 0;
}
