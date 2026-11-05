#ifndef XGE_TRANSFER_INTERNAL_H
#define XGE_TRANSFER_INTERNAL_H
#include "xge_transfer_internal_api.h"
/* The engine has one synchronous Run. Its caller owns services/code for that
 * complete C invocation; no weak FV or permanent service lease is stored.
 * A short lock only protects publishing/copying the stack record. Never hold
 * it across a factory, OS call, allocation or user callback. */
typedef struct xge_transfer_run_scope_t {
    const void* pSelf;
    uint64_t iThread;
    xn_transfer_services_v1 tServices;
} xge_transfer_run_scope_t;
static xspinlock g_xgeTransferSpin=XRT_SPIN_INIT;
static xge_transfer_run_scope_t* g_xgeTransferRun;
static xatomic64 g_xgeTransferUiThread;
uint64_t xgeInternalTransferThreadId(void)
{return xrtAtomic64Load(&g_xgeTransferUiThread,XMEMORY_ACQUIRE);}
static int __xgeTransferValidate(const xn_transfer_services_v1* services)
{
    if(!services || services->iSize!=sizeof(*services) || services->iAbiTag!=XN_TRANSFER_SERVICES_V1_TAG ||
       (services->iFlags&~XN_TRANSFER_PROCESS_RESIDENT) || !services->wrapDataObject ||
       !services->wrapDropSource || !services->wrapDropTarget)return XGE_ERROR_INVALID_ARGUMENT;
    if((services->iFlags&XN_TRANSFER_PROCESS_RESIDENT) ? services->pCodeCapability!=NULL:services->pCodeCapability==NULL)return XGE_ERROR_INVALID_ARGUMENT;
    return XGE_OK;
}
static int __xgeTransferRunBegin(xge_transfer_run_scope_t* scope,const xn_transfer_services_v1* services)
{
    memset(scope,0,sizeof(*scope));if(!services)return XGE_OK;
    int status=__xgeTransferValidate(services);if(status!=XGE_OK)return status;
    scope->iThread=xrtThreadCurrentId();scope->tServices=*services;
    (void)xrtSpinLock(&g_xgeTransferSpin);
    if(g_xgeTransferRun){(void)xrtSpinUnlock(&g_xgeTransferSpin);return XGE_ERROR_INVALID_STATE;}
    scope->pSelf=scope;g_xgeTransferRun=scope;(void)xrtSpinUnlock(&g_xgeTransferSpin);return XGE_OK;
}
static void __xgeTransferRunEnd(xge_transfer_run_scope_t* scope)
{
    if(!scope->pSelf)return;
    uint64_t thread=xrtThreadCurrentId();
    (void)xrtSpinLock(&g_xgeTransferSpin);
    if(scope->pSelf!=scope || g_xgeTransferRun!=scope || scope->iThread!=thread)abort();
    g_xgeTransferRun=NULL;scope->pSelf=NULL;(void)xrtSpinUnlock(&g_xgeTransferSpin);
}
static bool __xgeTransferCurrent(xn_transfer_services_v1* output)
{
    uint64_t thread=xrtThreadCurrentId();bool found=false;
    (void)xrtSpinLock(&g_xgeTransferSpin);
    if(g_xgeTransferRun && g_xgeTransferRun->iThread==thread){*output=g_xgeTransferRun->tServices;found=true;}
    (void)xrtSpinUnlock(&g_xgeTransferSpin);return found;
}
#endif
