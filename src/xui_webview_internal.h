#ifndef XUI_WEBVIEW_INTERNAL_H
#define XUI_WEBVIEW_INTERNAL_H

#include "xui.h"

/* Private Document provider channel. The generic public WebView contract is
 * limited to basic page hosting, layout, input and lifecycle. */
#if defined(XUI_WEBVIEW_TEST_EXPORTS)
#define XUI_WEBVIEW_INTERNAL_API XUI_API
#elif !defined(_WIN32) && (defined(__GNUC__) || defined(__clang__))
#define XUI_WEBVIEW_INTERNAL_API __attribute__((visibility("hidden")))
#else
#define XUI_WEBVIEW_INTERNAL_API
#endif
typedef void (*xui_webview_message_proc)(xui_widget pWidget, const char* sSource,
    const char* sJson, size_t iBytes, void* pUser);

typedef struct xui_web_request_t* xui_web_request;
typedef enum xui_web_request_state_t {
    XUI_WEB_REQUEST_PENDING = 0,
    XUI_WEB_REQUEST_COMPLETED = 1,
    XUI_WEB_REQUEST_FAILED = 2,
    XUI_WEB_REQUEST_CANCELLED = 3
} xui_web_request_state_t;
#define XUI_WEBVIEW_MAX_PENDING_SCRIPTS 64u
#define XUI_WEBVIEW_MAX_PENDING_CAPTURES 2u

XUI_WEBVIEW_INTERNAL_API int xuiWebViewSetMessageHandler(xui_widget pWidget, const char* sAllowedOrigin,
    xui_webview_message_proc onMessage, void* pUser);
/* Document-only policy: reject non-inline resource requests, including those
 * initiated by iframe documents. Install before loading untrusted HTML. */
XUI_WEBVIEW_INTERNAL_API int xuiWebViewBlockExternalResources(xui_widget pWidget);
XUI_WEBVIEW_INTERNAL_API uint32_t xuiWebViewGetBlockedResourceCount(xui_widget pWidget);
XUI_WEBVIEW_INTERNAL_API int xuiWebViewPostMessageJson(xui_widget pWidget, const char* sJson,
    size_t iBytes);
XUI_WEBVIEW_INTERNAL_API int xuiWebViewEvalScriptAsync(xui_widget pWidget, const char* sScript,
    size_t iBytes, xui_web_request* ppRequest);
XUI_WEBVIEW_INTERNAL_API int xuiWebViewCapturePngAsync(xui_widget pWidget,
    xui_web_request* ppRequest);
XUI_WEBVIEW_INTERNAL_API void xuiWebRequestRetain(xui_web_request pRequest);
XUI_WEBVIEW_INTERNAL_API void xuiWebRequestRelease(xui_web_request pRequest);
XUI_WEBVIEW_INTERNAL_API int xuiWebRequestGetState(xui_web_request pRequest);
XUI_WEBVIEW_INTERNAL_API int32_t xuiWebRequestGetNativeError(xui_web_request pRequest);
XUI_WEBVIEW_INTERNAL_API int xuiWebRequestGetResultJson(xui_web_request pRequest,
    const char** ppJson, size_t* pBytes);
XUI_WEBVIEW_INTERNAL_API int xuiWebRequestGetResultPng(xui_web_request pRequest,
    const void** ppPng, size_t* pBytes);
XUI_WEBVIEW_INTERNAL_API int xuiWebRequestCancel(xui_web_request pRequest);

#if defined(XUI_WEBVIEW_TEST_EXPORTS)
/* Test-only: query the browser PID for an isolated-profile failure exercise. */
XUI_WEBVIEW_INTERNAL_API int xuiWebViewTestBrowserProcessId(xui_widget pWidget,
    uint32_t* pProcessId);
#endif

#endif /* XUI_WEBVIEW_INTERNAL_H */
