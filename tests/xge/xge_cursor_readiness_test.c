/* CPU-only, real native publishers. Every installed callback owns distinct
 * userdata and ownership data; no function pointer is erased or cast. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xge.h"
#include "xge_host_xrt.h"
#include <xge_host_xrt_contract.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct HostEntry {const char* name;xge_host_xrt_proc address;} HostEntry;
static const HostEntry entries[]={
#define XGE_HOST_XRT_SLOT(name) {#name,(xge_host_xrt_proc)name},
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
};
static xge_host_xrt_proc resolve(void* user,const char* name)
{(void)user;size_t first=0,last=sizeof(entries)/sizeof(*entries);while(first<last){size_t middle=first+(last-first)/2;int order=strcmp(name,entries[middle].name);if(order<0)last=middle;else if(order>0)first=middle+1;else return entries[middle].address;}return NULL;}



static void change_cursor(void)
{
    int old=xgeGetCursor();assert(xgeSetCursor(-1)==XGE_ERROR_INVALID_ARGUMENT && xgeGetCursor()==old);
    for(int cycle=0;cycle<5;++cycle)for(int cursor=XGE_CURSOR_ARROW;cursor<=XGE_CURSOR_NOT_ALLOWED;++cursor){
        assert(xgeSetCursor(cursor)==XGE_OK && xgeGetCursor()==cursor);
        assert(xgeSetCursor(cursor)==XGE_OK && xgeGetCursor()==cursor);
    }
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    change_cursor();
    xge_desc_t desc={0};desc.iWidth=320;desc.iHeight=240;desc.iRunMode=XGE_RUN_MANUAL;
    assert(xgeInit(&desc)==XGE_OK);change_cursor();
    assert(xgeUnitChecked()==XGE_OK);change_cursor();xrtClearError();
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes && before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("cursor readiness: pre-init/manual logical init/pre-window shutdown changes preserve state without OS window, GPU or audio devices");
    return 0;
}
