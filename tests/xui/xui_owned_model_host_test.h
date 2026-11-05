#ifndef XUI_OWNED_MODEL_HOST_TEST_H
#define XUI_OWNED_MODEL_HOST_TEST_H
/* The same model fixtures can link either the independent 29-TU document
 * subset or the exact normally-built full host engine object graph. */
#if defined(XGE_XRT_HOST) && XGE_XRT_HOST
#define XRT_MODULE_ALL
#include "extlibs/xrt/xrt.h"
#include "xge_host_xrt.h"
#include <xge_host_xrt_contract.h>
#include <assert.h>
#include <string.h>
typedef struct ModelHostEntry {const char* name;xge_host_xrt_proc address;} ModelHostEntry;
static const ModelHostEntry modelHostEntries[]={
#define XGE_HOST_XRT_SLOT(name) {#name,(xge_host_xrt_proc)name},
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
};
static xge_host_xrt_proc modelHostResolve(void* user,const char* name)
{
    (void)user;size_t first=0,last=sizeof(modelHostEntries)/sizeof(*modelHostEntries);
    while(first<last){size_t mid=first+(last-first)/2;int order=strcmp(name,modelHostEntries[mid].name);if(order<0)last=mid;else if(order>0)first=mid+1;else return modelHostEntries[mid].address;}
    return NULL;
}
static void modelHostBind(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,modelHostResolve,NULL,NULL)==XGE_HOST_XRT_OK);
}
#else
static void modelHostBind(void){}
#endif
#endif
