/* CPU-only, real native publishers. Every installed callback owns distinct
 * userdata and ownership data; no function pointer is erased or cast. */
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xui.h"
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



#include "../../src/xui/core/xui_xrt_port.h"
static void match(const char* text,const char* format,int year,int month,int day,int hour,int minute,int second)
{
    xtime expected=0,actual=123;assert(xrtDateTime(year,month,day,hour,minute,second,0,&expected));
    assert(xuiXrtTryTimeParse(text,format,&actual) && actual==expected && !xrtGetError());
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot a,b;xrtMemDebugSnapshot(&a);
    match("2026-05-18","yyyy-mm-dd",2026,5,18,0,0,0);
    match("2026-05-18 14:27:36","yyyy-mm-dd hh:nn:ss",2026,5,18,14,27,36);
    match("18/05/2026","dd/mm/yyyy",2026,5,18,0,0,0);
    match("0001-01-01","yyyy-mm-dd",1,1,1,0,0,0);
    match("2026-05-18 %","yyyy-mm-dd %",2026,5,18,0,0,0);
    xtime value=123;assert(!xuiXrtTryTimeParse("bad","yyyy-mm-dd",&value) && value==123 && !xrtGetError());
    assert(!xuiXrtTryTimeParse(NULL,"yyyy-mm-dd",&value) && value==123 && !xrtGetError());
    xrtSetErrorInfo(XERR_VALUE,"date-port-test",123,"keep the caller diagnostic");const xerror* prior=xrtGetError();assert(prior);
    assert(xuiXrtTryTimeParse("2026-05-18","yyyy-mm-dd",&value) && xrtGetError()==prior);
    assert(!xuiXrtTryTimeParse("bad","yyyy-mm-dd",&value) && xrtGetError()==prior);xrtClearError();
    assert(xrtMemDebugFailAfter(0));assert(!xuiXrtTryTimeParse("bad","yyyy-mm-dd",&value));assert(xrtMemDebugFailAfter(-1));assert(!xrtGetError());
    xrtMemDebugSnapshot(&b);assert(a.LiveCount==b.LiveCount && a.LiveBytes==b.LiveBytes && a.InvalidFreeCount==b.InvalidFreeCount && a.DoubleFreeCount==b.DoubleFreeCount);
    puts("date port: exact VB token conversion, real XRT parse, epoch value, literal percent, mismatch isolation, prior diagnostic and OOM passed");return 0;
}
