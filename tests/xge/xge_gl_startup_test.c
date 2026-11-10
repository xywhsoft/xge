/* Linux static engine: intercept its actual symbol lookup, not a replacement
 * engine TU. A failed GL transaction must never dispatch the first frame. */
#define XRT_MODULE_ALL
#include <xge.h>
#include <xge_static_xrt_contract.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int missing,lookups,frames;
void* __real_dlsym(void*,const char*);
void* __wrap_dlsym(void* handle,const char* name)
{
    if(missing && !strcmp(name,"glBindFramebuffer")){++lookups;return NULL;}
    return __real_dlsym(handle,name);
}
static int frame(void* unused){(void)unused;++frames;return 1;}
int main(int argc,char** argv)
{
    assert(argc==2);missing=atoi(argv[1]);assert(missing==0 || missing==1);
    const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;
    assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
    xge_desc_t desc={0};desc.iWidth=128;desc.iHeight=96;desc.sTitle="Bounded GL startup transaction";
    assert(xgeInit(&desc)==XGE_OK);int result=xgeRun(frame,NULL);
    printf("GL startup: missing=%d, lookups=%d, frames=%d, status=%d\n",missing,lookups,frames,result);
    assert(lookups==missing && frames==!missing && result==(missing ? XGE_ERROR_BACKEND_FAILED:XGE_OK));
    assert(xgeUnitChecked()==XGE_OK && !xrtGetError());
    if(missing){
        /* Failure must leave a fully recoverable engine, not an ended backend
         * that poisons the next initialization or retains a half-loaded table. */
        missing=lookups=frames=0;
        assert(xgeInit(&desc)==XGE_OK && xgeRun(frame,NULL)==XGE_OK && frames==1 && !lookups);
        assert(xgeUnitChecked()==XGE_OK && !xrtGetError());
        puts("Healthy reinitialization after failed GL startup passed");
    }
    return 0;
}
