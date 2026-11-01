#include "extlibs/xrt/xrt_config.h"
#include "extlibs/xrt/xrt.h"
#include "include/xge_host_xrt.h"
#include "xge_host_xrt_contract.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

XGE_HOST_API void* xgeHostXrtTestAllocate(size_t size);
XGE_HOST_API void xgeHostXrtTestFree(void* pointer);
XGE_HOST_API void xgeHostXrtTestSnapshot(xmemdebugsnapshot* snapshot);
XGE_HOST_API xge_host_xrt_proc xgeHostXrtTestFunctionAddress(void);
XGE_HOST_API void xgeHostXrtTestSetError(void);
int xuiHostLifetimeMain(void);

static xge_host_xrt_contract_t contract = XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
static const char* omitted;
static int reenter, substitute;
static void other_function(void) {}
static xge_host_xrt_proc resolve(void* user, const char* name)
{
    (void)user;
    if (reenter) {
        reenter = 0;
        assert(xgeHostXrtBind(&contract, resolve, NULL, NULL) == XGE_HOST_XRT_BUSY);
    }
    if (omitted && !strcmp(name, omitted)) return NULL;
    if (substitute && !strcmp(name, "xrtVersion")) return other_function;
#define XGE_HOST_XRT_SLOT(symbol) if (!strcmp(name, #symbol)) return (xge_host_xrt_proc)(symbol);
#include "xge_host_xrt_slots.inc"
#undef XGE_HOST_XRT_SLOT
    return NULL;
}

int main(void)
{
    const xge_host_xrt_requirements_t* needed = xgeHostXrtDescribe();
    const char* missing = (const char*)1;
    assert(needed && needed->count > 1 && !xgeHostXrtRuntimeIdentity());
    xge_host_xrt_contract_t wrong = contract;
    wrong.coreLayout ^= 1;
    assert(xgeHostXrtBind(&wrong, resolve, NULL, &missing) == XGE_HOST_XRT_INVALID_CONTRACT && !missing);
    wrong = contract; wrong.headerSha256 = "wrong header";
    assert(xgeHostXrtBind(&wrong, resolve, NULL, NULL) == XGE_HOST_XRT_INVALID_CONTRACT);
    assert(xgeHostXrtBind(NULL, resolve, NULL, NULL) == XGE_HOST_XRT_INVALID_CONTRACT);
    assert(xgeHostXrtBind(&contract, NULL, NULL, NULL) == XGE_HOST_XRT_INVALID_CONTRACT);
    omitted = needed->names[needed->count-1];
    assert(xgeHostXrtBind(&contract, resolve, NULL, &missing) == XGE_HOST_XRT_MISSING_SYMBOL);
    assert(missing == omitted && !xgeHostXrtRuntimeIdentity());
    omitted = NULL; reenter = 1;
    assert(xgeHostXrtBind(&contract, resolve, NULL, NULL) == XGE_HOST_XRT_OK);
    assert(xgeHostXrtRuntimeIdentity() == (xge_host_xrt_proc)xrtVersion);
    assert(xgeHostXrtTestFunctionAddress() == (xge_host_xrt_proc)&xrtMemDebugSnapshot);
    assert(xgeHostXrtBind(&contract, resolve, NULL, NULL) == XGE_HOST_XRT_OK);
    substitute = 1;
    assert(xgeHostXrtBind(&contract, resolve, NULL, NULL) == XGE_HOST_XRT_DIFFERENT_RUNTIME);
    assert(xgeHostXrtRuntimeIdentity() == (xge_host_xrt_proc)xrtVersion);
    substitute = 0;
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before, native, host, after;
    xrtMemDebugSnapshot(&before);
    void* p = xgeHostXrtTestAllocate(71);
    assert(p); memset(p, 0x42, 71);
    xrtMemDebugSnapshot(&host); xgeHostXrtTestSnapshot(&native);
    assert(host.LiveCount == before.LiveCount+1 && host.LiveBytes == before.LiveBytes+71);
    assert(host.LiveCount == native.LiveCount && host.LiveBytes == native.LiveBytes);
    xrtFree(p);
    p = xrtMalloc(97); assert(p); xgeHostXrtTestFree(p);
    xgeHostXrtTestSetError(); assert(xrtErrorKind(xrtGetError()) == XERR_ARGUMENT);
    xrtClearError();
    xrtMemDebugSnapshot(&after);
    assert(after.LiveCount == before.LiveCount && after.LiveBytes == before.LiveBytes);
    assert(after.InvalidFreeCount == before.InvalidFreeCount && after.DoubleFreeCount == before.DoubleFreeCount);
    assert(xuiHostLifetimeMain() == 0);
    puts("Hosted XRT: transactional bind, exact addresses, one allocator/debug/error state and XUI lifetime passed");
    return 0;
}
