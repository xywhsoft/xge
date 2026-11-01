#include "src/xui/core/xui_internal.h"
#include "include/xge_host_xrt.h"

/* Minimal CPU profile lacks the optional popup module. */
xui_widget xuiInternalPopupPointerPassthroughOwner(xui_context context,
    xui_widget hit, float x, float y)
{ (void)context; (void)x; (void)y; return hit; }
int xuiInternalPopupPrepareOwnerPress(xui_context context,
    xui_widget target, float x, float y)
{ (void)context; (void)target; (void)x; (void)y; return XUI_OK; }

XGE_HOST_API void* xgeHostXrtTestAllocate(size_t size) { return xrtMalloc(size); }
XGE_HOST_API void xgeHostXrtTestFree(void* pointer) { xrtFree(pointer); }
XGE_HOST_API void xgeHostXrtTestSnapshot(xmemdebugsnapshot* snapshot) { xrtMemDebugSnapshot(snapshot); }
XGE_HOST_API xge_host_xrt_proc xgeHostXrtTestFunctionAddress(void)
{ return (xge_host_xrt_proc)&xrtMemDebugSnapshot; }
XGE_HOST_API void xgeHostXrtTestSetError(void) { xrtSetErrorKind(XERR_ARGUMENT); }
XGE_HOST_API void xgeHostXrtTestEnter(xui_context context) { xuiInternalOperationEnter(context); }
XGE_HOST_API void xgeHostXrtTestLeave(xui_context context) { xuiInternalOperationLeave(context); }
