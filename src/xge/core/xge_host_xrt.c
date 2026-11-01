#include "include/xge_host_xrt.h"
#include "xge_host_xrt_contract.h"
#include <string.h>

/* All slots remain private to this module; SDK clients use XGE's C ABI. */
#define XGE_HOST_XRT_SLOT(name) XgeHostXrtType_##name* XgeHostXrtSlot_##name;
#include "xge_host_xrt_slots.inc"
#undef XGE_HOST_XRT_SLOT

static const char* const xgeHostXrtNames[] = {
#define XGE_HOST_XRT_SLOT(name) #name,
#include "xge_host_xrt_slots.inc"
#undef XGE_HOST_XRT_SLOT
};
static const xge_host_xrt_requirements_t xgeHostXrtRequirements = {
    XGE_HOST_XRT_LOCAL_CONTRACT_INIT,
    sizeof(xgeHostXrtNames)/sizeof(xgeHostXrtNames[0]), xgeHostXrtNames
};
static volatile int xgeHostXrtBinding;
static int xgeHostXrtBound;

XGE_HOST_API const xge_host_xrt_requirements_t* xgeHostXrtDescribe(void)
{
    return &xgeHostXrtRequirements;
}

XGE_HOST_API int xgeHostXrtBind(const xge_host_xrt_contract_t* contract,
    xge_host_xrt_resolve_proc resolve, void* user, const char** missingName)
{
    const xge_host_xrt_contract_t* expected = &xgeHostXrtRequirements.contract;
    xge_host_xrt_proc addresses[sizeof(xgeHostXrtNames)/sizeof(xgeHostXrtNames[0])];
    size_t index;
    int result = XGE_HOST_XRT_OK;
    if (missingName) *missingName = NULL;
    if (!contract || !resolve || contract->size != sizeof(*contract) ||
        contract->revision != expected->revision || contract->dataModel != expected->dataModel ||
        contract->coreLayout != expected->coreLayout || !contract->headerSha256 ||
        strcmp(contract->headerSha256, expected->headerSha256)) return XGE_HOST_XRT_INVALID_CONTRACT;
    /* No XRT synchronization may be used before its imports are bound. */
    if (!__sync_bool_compare_and_swap(&xgeHostXrtBinding, 0, 1)) return XGE_HOST_XRT_BUSY;
    for (index = 0; index < xgeHostXrtRequirements.count; ++index) {
        addresses[index] = resolve(user, xgeHostXrtNames[index]);
        if (!addresses[index]) {
            if (missingName) *missingName = xgeHostXrtNames[index];
            result = XGE_HOST_XRT_MISSING_SYMBOL; goto done;
        }
    }
    index = 0;
    if (xgeHostXrtBound) {
#define XGE_HOST_XRT_SLOT(name) \
        if ((xge_host_xrt_proc)XgeHostXrtSlot_##name != addresses[index++]) { \
            result = XGE_HOST_XRT_DIFFERENT_RUNTIME; goto done; }
#include "xge_host_xrt_slots.inc"
#undef XGE_HOST_XRT_SLOT
        goto done;
    }
#define XGE_HOST_XRT_SLOT(name) XgeHostXrtSlot_##name = (XgeHostXrtType_##name*)addresses[index++];
#include "xge_host_xrt_slots.inc"
#undef XGE_HOST_XRT_SLOT
    xgeHostXrtBound = 1;
done:
    __sync_lock_release(&xgeHostXrtBinding);
    return result;
}

XGE_HOST_API xge_host_xrt_proc xgeHostXrtRuntimeIdentity(void)
{
    /* Reading also requires the documented pre-execution binding discipline. */
    return (xge_host_xrt_proc)XgeHostXrtSlot_xrtVersion;
}
