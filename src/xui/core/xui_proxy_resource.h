#ifndef XUI_PROXY_RESOURCE_INTERNAL_H
#define XUI_PROXY_RESOURCE_INTERNAL_H
#include "xui.h"
#include <stddef.h>

/* Shared native storage. Backends embed this prefix, then their payload in
 * the same allocation. No method dispatch or language object is added here. */
enum { XUI_OWNED_PROXY=1, XUI_OWNED_FONT=2, XUI_OWNED_SURFACE=3, XUI_OWNED_DRAW=4 };
typedef int (*xui_owned_payload_finalize_proc)(xui_proxy,void*);
typedef struct xui_owned_resource_t {
    _Alignas(max_align_t) uint32_t iMagic;
    uint32_t iKind;
    size_t iReferences,iOwners,iPins;
    const void* pClaim;
    /* Clear quarantines this exact owning slot; Finish consumes it. */
    xui_proxy pProxy;
    xui_owned_payload_finalize_proc onFinalize;
    uint64_t iCreatorThread;
    uint32_t iFlags;
    /* Mutable fields above are protected by Mutation/Freeze then tLock.
     * Readiness uses the published atomic state without taking the lock. */
    xatomic32 tState,tLock;
} xui_owned_resource_t;
/* Draw alone owns an additional target slot. Other resource headers keep
 * their existing size, and backend payloads remain in this allocation. */
typedef struct xui_owned_draw_t {
    xui_owned_resource_t tResource;
    xui_surface pTarget;
} xui_owned_draw_t;

int xuiInternalProxyCreateOwned(const xui_proxy_t*,const xui_callback_owner_t*,bool,uint32_t,xui_proxy*);
bool xuiInternalProxyResourceManaged(const void*,unsigned);
bool xuiInternalProxyResourceReady(const void*,unsigned);
int xuiInternalProxyResourcePin(void*,unsigned);
int xuiInternalProxyResourceUnpin(void*,unsigned);
static inline void* xuiInternalProxyResourcePayload(void* resource)
{
    xui_owned_resource_t* value=resource;
    return value->iKind==XUI_OWNED_DRAW ? (void*)((xui_owned_draw_t*)resource+1):(void*)(value+1);
}
#endif
