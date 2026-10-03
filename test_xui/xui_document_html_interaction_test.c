#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xge.h"
#include "xui_document_ui.h"
#include "src/xui_webview_internal.h"

static const char first_html[] =
    "<input id='v' value='idle'><script>"
    "document.getElementById('v').value='unsafe'</script>";
static const char second_html[] =
    "<input id=\"v\" value=\"new &amp; safe\">";
static const char markdown_html[] =
    "<div>\n<meta http-equiv=\"refresh\" content=\"0;url=about:blank#escaped\">"
    "<form id=\"form\" action=\"about:blank#submitted\">"
    "<input id=\"v\" value=\"markdown\"></form>\n"
    "<a id=\"jump\" href=\"#bottom\">Jump</a>\n"
    "<a id=\"external\" href=\"https://example.org/path?x=1&amp;y=2\">External</a>\n"
    "<a id=\"blocked\" href=\"javascript:alert(1)\">Blocked</a>\n"
    "<img id=\"remote\" src=\"http://127.0.0.1:38941/pixel.png\">\n"
    "<img id=\"inline\" src=\"data:image/svg+xml,%3Csvg%20xmlns%3D'http://www.w3.org/2000/svg'%20width%3D'1'%20height%3D'1'%3E%3C/svg%3E\">\n"
    "<div style=\"height:1200px\"></div>\n"
    "<p id=\"bottom\">Bottom</p>\n</div>\n";
static xui_context context;
static xui_widget root;
static xui_document document;
static xui_doc_node_id html_node;
static xui_doc_html_interaction panel;
static xui_web_request probe;
static int phase, frames, phase_frames, result = 1;
static int link_calls;
static char link_url[256];
static uint64_t markdown_revision;
static SOCKET resource_listener = INVALID_SOCKET;
static int resource_requests;
static int resource_winsock_started;

static int start_resource_listener(void)
{
    WSADATA wsa;
    struct sockaddr_in address = {0};
    u_long nonblocking = 1;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 0;
    resource_winsock_started = 1;
    resource_listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (resource_listener == INVALID_SOCKET) return 0;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(38941);
    if (bind(resource_listener, (const struct sockaddr*)&address,
            sizeof(address)) != 0 ||
        listen(resource_listener, 4) != 0 ||
        ioctlsocket(resource_listener, FIONBIO, &nonblocking) != 0)
        return 0;
    return 1;
}

static int collect_resource_requests(void)
{
    SOCKET connection;
    if (resource_listener == INVALID_SOCKET) return 1;
    while ((connection = accept(resource_listener, NULL, NULL)) != INVALID_SOCKET) {
        resource_requests++;
        closesocket(connection);
    }
    return WSAGetLastError() == WSAEWOULDBLOCK;
}

static void on_open_link(const char* url, void* user)
{
    (void)user;
    link_calls++;
    snprintf(link_url, sizeof(link_url), "%s", url);
}

static void fail(const char* step)
{
    fprintf(stderr, "HTML interaction failed: %s (phase %d)\n", step, phase);
    result = 2;
    xgeQuit();
}

static int setup(void)
{
    xui_doc_node_desc_t node = {0};
    xui_doc_html_interaction_desc_t desc = {0};
    xui_document_transaction transaction = NULL;
#define SETUP_OK(step, expression) do { int setup_status = (expression); \
    if (setup_status != XUI_OK) { fprintf(stderr, "setup %s: %d\n", step, setup_status); return 0; } \
} while (0)
    SETUP_OK("context", xuiCreate(&context));
    SETUP_OK("root", xuiWidgetCreate(context, &root));
    SETUP_OK("set root", xuiSetRootWidget(context, root));
    SETUP_OK("root rect", xuiWidgetSetRect(root, (xui_rect_t){0, 0, 800, 600}));
    SETUP_OK("viewport", xuiInputViewport(context, 800, 600));
    SETUP_OK("document", xuiDocumentCreate(NULL, &document));
    node.iSize = sizeof(node); node.iKind = XUI_DOC_HTML;
    node.tAttributes.iFlags = XUI_DOC_BLOCK;
    node.sText = first_html; node.iTextBytes = sizeof(first_html) - 1;
    SETUP_OK("begin", xuiDocumentBeginTransaction(document, NULL, &transaction));
    SETUP_OK("insert", xuiDocumentTxnInsertNode(transaction, 1, 0, &node, &html_node));
    SETUP_OK("commit", xuiDocumentTxnCommit(transaction, NULL));
    xuiDocumentTxnRelease(transaction);
    desc.iSize = sizeof(desc); desc.pContext = context;
    desc.pDocument = document; desc.iNodeId = html_node;
    desc.tBounds = (xui_rect_t){40, 40, 680, 460};
    SETUP_OK("panel", xuiDocumentHtmlInteractionCreate(&desc, &panel));
    if (!xuiDocumentHtmlInteractionGetWindow(panel) ||
        !xuiDocumentHtmlInteractionGetWebView(panel)) return 0;
#undef SETUP_OK
    return 1;
}

static int html_source_is(const char* expected)
{
    xui_document_snapshot snapshot = NULL;
    char buffer[256]; uint64_t bytes = 0;
    int ok = xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotCopyText(snapshot, html_node, buffer,
            sizeof(buffer), &bytes) == XUI_OK &&
        bytes == strlen(expected) && !memcmp(buffer, expected, (size_t)bytes);
    xuiDocumentSnapshotRelease(snapshot);
    return ok;
}

static int setup_markdown(void)
{
    xui_doc_desc_t document_desc = {0};
    xui_doc_html_interaction_desc_t panel_desc = {0};
    xui_document_snapshot snapshot = NULL;
    xui_doc_node_info_t info = {0};
    document_desc.iSize = sizeof(document_desc);
    document_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    if (getenv("XUI_DOCUMENT_HTML_RESOURCE_TEST") &&
        !start_resource_listener()) goto fail;
    if (xuiDocumentCreate(&document_desc, &document) != XUI_OK ||
        xuiDocumentLoadMarkdown(document, markdown_html,
            sizeof(markdown_html) - 1) != XUI_OK ||
        xuiDocumentAcquireSnapshot(document, &snapshot) != XUI_OK ||
        xuiDocumentSnapshotGetChild(snapshot, XUI_DOCUMENT_ROOT, 0,
            &html_node) != XUI_OK)
        goto fail;
    info.iSize = sizeof(info);
    if (xuiDocumentSnapshotGetNode(snapshot, html_node, &info) != XUI_OK ||
        info.iKind != XUI_DOC_HTML)
        goto fail;
    xuiDocumentSnapshotRelease(snapshot);
    panel_desc.iSize = sizeof(panel_desc);
    panel_desc.pContext = context;
    panel_desc.pDocument = document;
    panel_desc.iNodeId = html_node;
    panel_desc.tBounds = (xui_rect_t){40, 40, 680, 460};
    panel_desc.onOpenLink = on_open_link;
    markdown_revision = xuiDocumentGetRevision(document);
    return xuiDocumentHtmlInteractionCreate(&panel_desc, &panel) == XUI_OK;
fail:
    xuiDocumentSnapshotRelease(snapshot);
    return 0;
}

static int panel_has_clipped_host(void)
{
    HWND window = (HWND)xgePlatformNativeHandle();
    HWND host = FindWindowExW(window, NULL, L"Static", NULL);
    RECT rect;
    if (!window || !host || !GetWindowRect(host, &rect)) {
        return 0;
    }
    MapWindowPoints(NULL, window, (POINT*)&rect, 2);
    return rect.left >= 40 && rect.top >= 68 &&
        rect.right <= 720 && rect.bottom <= 500 &&
        rect.right - rect.left > 500 && rect.bottom - rect.top > 300 &&
        IsWindowVisible(host);
}

static int send_unicode_to_panel(void)
{
    static const WORD characters[] = {0x4e2d, 0x6587, 'A'};
    HWND window = (HWND)xgePlatformNativeHandle();
    HWND host = FindWindowExW(window, NULL, L"Static", NULL);
    HWND focused = GetFocus();
    INPUT input[sizeof(characters) / sizeof(characters[0]) * 2] = {{0}};
    size_t i;
    if (!window || !host || GetForegroundWindow() != window || !focused ||
        (focused != host && !IsChild(host, focused))) return 0;
    for (i = 0; i < sizeof(characters) / sizeof(characters[0]); i++) {
        input[i * 2].type = INPUT_KEYBOARD;
        input[i * 2].ki.wScan = characters[i];
        input[i * 2].ki.dwFlags = KEYEVENTF_UNICODE;
        input[i * 2 + 1] = input[i * 2];
        input[i * 2 + 1].ki.dwFlags |= KEYEVENTF_KEYUP;
    }
    return SendInput((UINT)(sizeof(input) / sizeof(input[0])), input,
        sizeof(INPUT)) == (UINT)(sizeof(input) / sizeof(input[0]));
}

static int add_unrelated_paragraph(void)
{
    xui_doc_node_desc_t node = {0};
    xui_document_transaction t = NULL;
    xui_doc_node_id id = 0;
    int status;
    node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
    status = xuiDocumentBeginTransaction(document, NULL, &t);
    if (status == XUI_OK) status = xuiDocumentTxnInsertNode(t, 1, 1, &node, &id);
    if (status == XUI_OK) status = xuiDocumentTxnCommit(t, NULL);
    xuiDocumentTxnRelease(t);
    return status == XUI_OK && id != 0;
}

static int replace_html(void)
{
    xui_document_transaction t = NULL;
    int status = xuiDocumentBeginTransaction(document, NULL, &t);
    if (status == XUI_OK) status = xuiDocumentTxnReplaceText(t, html_node,
        0, sizeof(first_html) - 1, second_html, sizeof(second_html) - 1);
    if (status == XUI_OK) status = xuiDocumentTxnCommit(t, NULL);
    xuiDocumentTxnRelease(t);
    return status == XUI_OK;
}

static int probe_text(const char* script, const char* expected)
{
    xui_widget browser = xuiDocumentHtmlInteractionGetWebView(panel);
    const char* json = NULL; size_t bytes = 0;
    if (!probe) {
        int status = xuiWebViewEvalScriptAsync(browser, script, strlen(script),
            &probe);
        if (status != XUI_OK) {
            fprintf(stderr, "probe start: %d, web state=%d native=0x%08x\n",
                status, xuiWebViewGetState(browser),
                (unsigned)xuiWebViewGetNativeError(browser));
            return -1;
        }
        return 0;
    }
    if (xuiWebRequestGetState(probe) == XUI_WEB_REQUEST_PENDING) return 0;
    if (xuiWebRequestGetState(probe) == XUI_WEB_REQUEST_CANCELLED) {
        xuiWebRequestRelease(probe); probe = NULL;
        return 0;
    }
    if (xuiWebRequestGetState(probe) != XUI_WEB_REQUEST_COMPLETED ||
        xuiWebRequestGetResultJson(probe, &json, &bytes) != XUI_OK) {
        fprintf(stderr, "probe result state=%d native=0x%08x\n",
            xuiWebRequestGetState(probe),
            (unsigned)xuiWebRequestGetNativeError(probe));
        return -1;
    }
    {
        int match = json && bytes == strlen(json) && strstr(json, expected) != NULL;
        xuiWebRequestRelease(probe); probe = NULL;
        return match ? 1 : 0;
    }
}

static int frame(void* user)
{
    static const char read_value[] =
        "document.querySelector('iframe')?.contentDocument.getElementById('v')?.value || ''";
    static const char read_refresh_value[] =
        "(()=>{const d=document.querySelector('iframe')?.contentDocument;"
        "return d?.querySelector('meta[http-equiv=refresh]')?.content"
        ".includes('about:blank#escaped')?d.getElementById('v')?.value:'missing meta'})()";
    static const char edit_dom[] =
        "const v=document.querySelector('iframe').contentDocument.getElementById('v');"
        "v.value='clicked';v.value";
    static const char click_anchor[] =
        "document.querySelector('iframe').contentDocument.getElementById('jump').click();'clicked'";
    static const char check_anchor_scroll[] =
        "(()=>{const f=document.querySelector('iframe');"
        "return f.contentDocument?.getElementById('bottom') &&"
        "f.contentWindow.scrollY>0?'scrolled':'waiting'})()";
    static const char click_external[] =
        "document.querySelector('iframe').contentDocument.getElementById('external').click();'clicked'";
    static const char click_blocked[] =
        "document.querySelector('iframe').contentDocument.getElementById('blocked').click();'clicked'";
    static const char stale_message[] =
        "chrome.webview.postMessage({kind:'externalLink',"
        "url:'https://stale.example/',generation:0});'sent'";
    static const char invalid_message[] =
        "chrome.webview.postMessage({kind:'externalLink',"
        "url:'javascript:alert(1)',"
        "generation:Number(document.querySelector('iframe').dataset.generation)});'sent'";
    static const char submit_form[] =
        "document.querySelector('iframe').contentDocument.getElementById('form').submit();'submitted'";
    static const char focus_input[] =
        "const v=document.querySelector('iframe').contentDocument.getElementById('v');"
        "v.value='';v.focus();'focused'";
    static const char check_input[] =
        "document.querySelector('iframe').contentDocument.getElementById('v').value"
        "==='\\u4e2d\\u6587A'";
    static const char check_resources[] =
        "(()=>{const d=document.querySelector('iframe').contentDocument;"
        "const r=d.getElementById('remote'),i=d.getElementById('inline');"
        "return r.complete&&r.naturalWidth===0&&i.complete&&i.naturalWidth===1"
        "?'resource-blocked':'waiting'})()";
    int checked;
    (void)user;
    if (!context && !setup()) { fail("setup"); return XGE_ERROR; }
    if (++frames > 900) { fail("timeout"); return XGE_ERROR; }
    if (xuiDispatchPendingEvents(context) != XUI_OK ||
        xuiLayout(context) != XUI_OK ||
        xuiUpdate(context, .016f) != XUI_OK ||
        xuiDocumentHtmlInteractionUpdate(panel) != XUI_OK ||
        !collect_resource_requests()) {
        fail("update"); return XGE_ERROR;
    }
    phase_frames++;
    if (xuiWebViewGetState(xuiDocumentHtmlInteractionGetWebView(panel)) !=
        XUI_WEBVIEW_READY) goto render;
    if (phase == 21 && getenv("XUI_DOCUMENT_HTML_SYNTHETIC_TEXT")) {
        if (!send_unicode_to_panel()) {
            fail("system Unicode input into HTML form"); return XGE_ERROR;
        }
        phase = 22; phase_frames = 0;
        goto render;
    }
    if (phase == 22 && phase_frames < 12) goto render;
    if ((phase == 11 && !link_calls) ||
            (phase == 19 && phase_frames < 30)) checked = 0;
    else if (phase == 19 && getenv("XUI_DOCUMENT_HTML_RESOURCE_TEST"))
        checked = probe_text(check_resources, "resource-blocked");
    else if (phase == 0 || phase == 2 || phase == 3 || phase == 4 ||
            phase == 5 || phase == 7 || phase == 11 || phase == 13 ||
            phase == 15 || phase == 17 || phase == 19) {
        const char* expected = phase == 2 ? "clicked" :
            phase == 3 ? "new & safe" :
            phase >= 7 ? "markdown" : "idle";
        checked = probe_text(phase == 7 ? read_refresh_value : read_value,
            expected);
    } else if (phase == 8) checked = probe_text(click_anchor, "clicked");
    else if (phase == 9) checked = probe_text(check_anchor_scroll, "scrolled");
    else if (phase == 10) checked = probe_text(click_external, "clicked");
    else if (phase == 12) checked = probe_text(click_blocked, "clicked");
    else if (phase == 14) checked = probe_text(stale_message, "sent");
    else if (phase == 16) checked = probe_text(invalid_message, "sent");
    else if (phase == 18) checked = probe_text(submit_form, "submitted");
    else if (phase == 20 && getenv("XUI_DOCUMENT_HTML_SYNTHETIC_TEXT")) {
        HWND window = (HWND)xgePlatformNativeHandle();
        SetForegroundWindow(window);
        if (GetForegroundWindow() != window) checked = 0;
        else if (xuiWebViewFocus(xuiDocumentHtmlInteractionGetWebView(panel)) != XUI_OK) {
            fail("HTML input focus setup"); return XGE_ERROR;
        } else checked = probe_text(focus_input, "focused");
    } else if (phase == 20) checked = 1;
    else if (phase == 22) checked = probe_text(check_input, "true");
    else checked = probe_text(edit_dom, "clicked");
    if (checked < 0) { fail("DOM probe"); return XGE_ERROR; }
    if (checked == 1) {
        if (probe) { fail("probe lifetime"); return XGE_ERROR; }
        phase++; phase_frames = 0;
        if (phase == 1 && (!html_source_is(first_html) ||
                !panel_has_clipped_host())) {
            fail("sandboxed HTML or clipped host"); return XGE_ERROR;
        }
        if (phase == 2 && (!add_unrelated_paragraph() ||
                xuiDocumentHtmlInteractionUpdate(panel) != XUI_OK)) {
            fail("unrelated edit"); return XGE_ERROR;
        }
        if (phase == 3 && (!replace_html() ||
                xuiDocumentHtmlInteractionUpdate(panel) != XUI_OK)) {
            fail("HTML source edit"); return XGE_ERROR;
        }
        if (phase == 4 && (xuiDocumentUndo(document, NULL) != XUI_OK ||
                xuiDocumentHtmlInteractionUpdate(panel) != XUI_OK)) {
            fail("HTML Undo"); return XGE_ERROR;
        }
        if (phase == 5) {
            int close_status = xuiWindowSetOpen(xuiDocumentHtmlInteractionGetWindow(panel), 0);
            int show_status = xuiDocumentHtmlInteractionShow(panel);
            if (close_status != XUI_OK || show_status != XUI_OK) {
                fprintf(stderr, "panel close=%d show=%d web=%d native=0x%08x\n",
                    close_status, show_status,
                    xuiWebViewGetState(xuiDocumentHtmlInteractionGetWebView(panel)),
                    (unsigned)xuiWebViewGetNativeError(xuiDocumentHtmlInteractionGetWebView(panel)));
                fail("reopen panel"); return XGE_ERROR;
            }
        }
        if (phase == 6) {
            if (!html_source_is(first_html) ||
                xuiGetFocusWidget(context) !=
                    xuiDocumentHtmlInteractionGetWebView(panel)) {
                fail("source or focus after interaction"); return XGE_ERROR;
            }
            xuiDocumentHtmlInteractionRelease(panel); panel = NULL;
            xuiDocumentRelease(document); document = NULL;
            if (!setup_markdown()) {
                fail("Markdown HTML panel"); return XGE_ERROR;
            }
            phase = 7;
        }
        if ((phase == 21 && !getenv("XUI_DOCUMENT_HTML_SYNTHETIC_TEXT")) ||
            phase == 23) {
            if (link_calls != 1 || strcmp(link_url,
                    "https://example.org/path?x=1&y=2") != 0 ||
                (phase == 23 && xuiDocumentGetRevision(document) != markdown_revision) ||
                resource_requests != 0 ||
                (getenv("XUI_DOCUMENT_HTML_RESOURCE_TEST") &&
                    xuiWebViewGetBlockedResourceCount(
                        xuiDocumentHtmlInteractionGetWebView(panel)) == 0)) {
                fail("external link policy"); return XGE_ERROR;
            }
            if (getenv("XUI_DOCUMENT_HTML_RESOURCE_TEST"))
                printf("Document HTML interaction: external iframe resources blocked=%u, listener requests=%d; inline data image retained\n",
                    (unsigned)xuiWebViewGetBlockedResourceCount(
                        xuiDocumentHtmlInteractionGetWebView(panel)), resource_requests);
            else puts(phase == 23 ?
                "Document HTML interaction: native Unicode form input and source isolation passed" :
                "Document HTML interaction: Rich/Markdown, sandboxed script/form, clipping, DOM/source isolation, reload, Undo, focus, anchors, scroll, links and stale-message policy passed");
            result = 0; xgeQuit();
        }
    }
    if (phase_frames > 300) { fail("phase timeout"); return XGE_ERROR; }
render:
    if (xgeBegin() != XGE_OK) { fail("begin"); return XGE_ERROR; }
    xgeClear(0xffeef1f5);
    if (xgeEnd() != XGE_OK) { fail("end"); return XGE_ERROR; }
    return XGE_OK;
}

int main(void)
{
    xge_desc_t desc = {0};
    desc.iWidth = 800; desc.iHeight = 600;
    desc.sTitle = "Document HTML interaction";
    desc.iRunMode = XGE_RUN_GAME_LOOP; desc.iTargetFPS = 60;
    if (xgeInit(&desc) != XGE_OK) return 2;
    xgeRun(frame, NULL);
    if (probe) xuiWebRequestRelease(probe);
    if (resource_listener != INVALID_SOCKET) closesocket(resource_listener);
    if (resource_winsock_started) WSACleanup();
    xuiDocumentHtmlInteractionRelease(panel);
    xuiDocumentRelease(document);
    if (context) xuiDestroy(context);
    xgeUnit();
    return result;
}
