#ifdef XGE_XRT_HOST
#define XRT_MODULE_ALL
#ifdef _WIN32
#define XRT_USE_SHARED
#endif
#endif
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef XGE_XRT_HOST
#include <xge_host_xrt_contract.h>
static xge_host_xrt_proc resolve(void* user, const char* name)
{
    (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if (!strcmp(name, #symbol)) return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif

static void check_snapshot_storage(void)
{
    xui_code_token_buffer buffer = NULL;
    const xui_code_token_t values[] = {
        {.iStartOffset=0, .iEndOffset=3}, {.iStartOffset=2, .iEndOffset=5},
        {.iStartOffset=5, .iEndOffset=9}, {.iStartOffset=6, .iEndOffset=10}
    };
    const xui_code_token_t replacement = {.iStartOffset=3, .iEndOffset=9};
    assert(xuiCodeTokenBufferCreate(&buffer) == XUI_OK);
    assert(xuiCodeTokenBufferSetRange(buffer, values, 4, UINT32_MAX, 0, 10) == XUI_OK);
    xmemdebugsnapshot begin, after;
    xrtMemDebugSnapshot(&begin);
    assert(xrtMemDebugFailAfter(0));
    for ( int i = 0; i < 1000; ++i )
        assert(xuiCodeTokenBufferSetRange(buffer, values, 4, UINT32_MAX, 0, 10) == XUI_OK);
    xrtMemDebugFailClear();
    xrtMemDebugSnapshot(&after);
    assert(after.AllocCount == begin.AllocCount && after.FreeCount == begin.FreeCount && after.ReallocCount == begin.ReallocCount);
    puts("token warmed writes: 1000 publications, heap 0/0/0");

    xui_code_token_snapshot all = NULL, slice = NULL, empty = NULL;
    assert(xuiCodeTokenBufferCapture(buffer, UINT32_MAX, &all) == XUI_OK);
    assert(xuiCodeTokenBufferCaptureInRange(buffer, UINT32_MAX, 3, 6, &slice) == XUI_OK);
    const xui_code_token_t* fullData = NULL;
    const xui_code_token_t* sliceData = NULL;
    size_t count = 0;
    assert(xuiCodeTokenSnapshotGetTokens(all, &fullData, &count) == XUI_OK && count == 4);
    assert(xuiCodeTokenSnapshotGetTokens(slice, &sliceData, &count) == XUI_OK && count == 2);
    assert(sliceData == fullData + 1 && sliceData[1].iEndOffset == 9);
    int copied;
    xui_code_token_t formerlyCopied[2];
    assert(xuiCodeTokenBufferGetTokensInRange(buffer, UINT32_MAX, 3, 6, formerlyCopied, 2, &copied) == XUI_OK);
    assert(copied == 2 && !memcmp(formerlyCopied, sliceData, sizeof(formerlyCopied)));
    assert(xuiCodeTokenBufferCaptureInRange(buffer, UINT32_MAX, 3, 3, &empty) == XUI_OK);
    const xui_code_token_t* emptyData = fullData;
    assert(xuiCodeTokenSnapshotGetTokens(empty, &emptyData, &count) == XUI_OK && count == 0 && emptyData == NULL);
    xuiCodeTokenSnapshotDestroy(empty);
    empty = NULL;
    assert(xuiCodeTokenBufferCaptureInRange(buffer, UINT32_MAX, 8, 11, &empty) == XUI_ERROR_UNSUPPORTED && empty == NULL);

    xrtMemDebugSnapshot(&begin);
    for ( int i = 0; i < 1000; ++i ) {
        xui_code_token_snapshot copy = NULL;
        assert(xuiCodeTokenBufferCaptureInRange(buffer, UINT32_MAX, 3, 6, &copy) == XUI_OK);
        const xui_code_token_t* data;
        assert(xuiCodeTokenSnapshotGetTokens(copy, &data, &count) == XUI_OK && data == sliceData && count == 2);
        xuiCodeTokenSnapshotDestroy(copy);
    }
    xrtMemDebugSnapshot(&after);
    assert(after.AllocCount - begin.AllocCount == 1000 && after.FreeCount - begin.FreeCount == 1000);
    assert(after.ReallocCount == begin.ReallocCount && after.LiveBytes == begin.LiveBytes);
    puts("token repeated captures: 1000 shared slices, heap 1000/1000/0");

    xrtMemDebugSnapshot(&begin);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiCodeTokenBufferCapture(buffer, UINT32_MAX, &empty) == XUI_ERROR_OUT_OF_MEMORY && empty == NULL);
    xrtMemDebugFailClear();
    xrtClearError();
    assert(xrtMemDebugFailAfter(0));
    assert(xuiCodeTokenBufferSetRange(buffer, &replacement, 1, 42, 0, 10) == XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();
    xrtClearError();
    xrtMemDebugSnapshot(&after);
    assert(after.LiveBytes == begin.LiveBytes && after.LiveCount == begin.LiveCount);
    uint32_t version = 0;
    assert(xuiCodeTokenBufferGetVersion(buffer, &version) == XUI_OK && version == UINT32_MAX);
    assert(xuiCodeTokenBufferGetCount(buffer) == 4 && fullData[0].iStartOffset == 0);
    assert(xuiCodeTokenBufferSetRange(buffer, &replacement, 1, 42, 0, 10) == XUI_OK);
    xui_code_token_snapshot newer = NULL;
    const xui_code_token_t* newerData = NULL;
    assert(xuiCodeTokenBufferCapture(buffer, 42, &newer) == XUI_OK);
    assert(xuiCodeTokenSnapshotGetTokens(newer, &newerData, &count) == XUI_OK && count == 1);
    assert(newerData != fullData && newerData[0].iStartOffset == replacement.iStartOffset);
    xuiCodeTokenSnapshotDestroy(newer);
    assert(xuiCodeTokenSnapshotAddRef(slice) == XUI_OK);
    xuiCodeTokenSnapshotDestroy(slice);
    xuiCodeTokenBufferClear(buffer);
    xuiCodeTokenBufferDestroy(buffer);
    xuiCodeTokenSnapshotDestroy(all);
    assert(sliceData[0].iStartOffset == 2 && sliceData[1].iEndOffset == 9);
    xuiCodeTokenSnapshotDestroy(slice);
}

static void invalidated(void* user)
{
    ++*(int*)user;
}

static int wrong_thread(void* user)
{
    xui_code_token_buffer buffer = user;
    xui_code_token_snapshot snapshot = NULL;
    assert(xuiCodeTokenBufferCapture(buffer, 0, &snapshot) == XUI_ERROR_BUSY && snapshot == NULL);
    assert(xuiCodeTokenBufferClearChecked(buffer) == XUI_ERROR_BUSY);
    return 0;
}

typedef struct style_probe_t {
    xui_code_token_buffer buffer;
    int hits;
} style_probe_t;

static int token_style(xui_code_document document, const xui_code_token_t* token, int* style, void* user)
{
    (void)document;
    style_probe_t* probe = user;
    assert(xuiCodeTokenBufferClearChecked(probe->buffer) == XUI_ERROR_BUSY);
    if ( token->iStartOffset == 0 && token->iEndOffset == 4 && token->iStyle == XUI_CODE_STYLE_COMMENT )
        ++probe->hits;
    *style = token->iStyle;
    return XUI_OK;
}

static void check_editor_cache(void)
{
    xui_test_proxy_state_t proxy;
    xuiTestProxyInit(&proxy);
    xui_context_desc_t desc;
    xuiContextDescDefault(&desc);
    desc.iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED;
    xui_context context = NULL;
    assert(xuiCreateEx(&desc, &context) == XUI_OK);
    assert(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    xui_font font = NULL;
    assert(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "tokens.ttf", 14, 0) == XUI_OK);
    assert(xuiSetDefaultFont(context, font) == XUI_OK);
    xui_widget editor = NULL;
    assert(xuiCodeEditCreate(context, &editor, NULL) == XUI_OK);
    assert(xuiSetRootWidget(context, editor) == XUI_OK);
    assert(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 160}) == XUI_OK);
    assert(xuiInputViewport(context, 320, 160) == XUI_OK);
    xui_surface target = NULL;
    assert(xuiTestSurfaceCreate(&proxy, &target, 320, 160, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    assert(xuiCodeEditSetText(editor, "memo 42") == XUI_OK);
    assert(xuiCodeEditSetLanguage(editor, "c") == XUI_OK);
    uint32_t version, cached;
    assert(xuiCodeEditGetDocumentVersion(editor, &version) == XUI_OK);
    xui_code_token_buffer buffer = xuiCodeEditGetTokenBuffer(editor);
    assert(xuiCodeTokenBufferDestroyChecked(buffer) == XUI_ERROR_INVALID_STATE);
    style_probe_t probe = {buffer, 0};
    assert(xuiCodeEditSetTokenStyleResolver(editor, token_style, &probe) == XUI_OK);

    const uint32_t dirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
    xuiWidgetClearDirty(editor, dirty);
    const xui_code_token_t token = {
        .iStartOffset=0, .iEndOffset=4, .iKind=XUI_CODE_TOKEN_COMMENT, .iStyle=XUI_CODE_STYLE_COMMENT
    };
    assert(xuiCodeTokenBufferSetRange(buffer, &token, 1, version, 0, 8) == XUI_ERROR_INVALID_ARGUMENT);
    assert(!(xuiWidgetGetDirtyFlags(editor) & dirty));
    assert(xuiCodeTokenBufferSet(buffer, &token, 1, version) == XUI_OK);
    assert((xuiWidgetGetDirtyFlags(editor) & dirty) == dirty);
    int start, end;
    assert(xuiCodeTokenBufferGetRange(buffer, &cached, &start, &end) == XUI_OK);
    assert(cached == version && start == 0 && end == 7);
    assert(xuiLayout(context) == XUI_OK && xuiRender(context, target, NULL, 0) == XUI_OK);
    assert(probe.hits > 0);

    xui_code_token_snapshot snapshot = NULL;
    assert(xuiCodeTokenBufferCapture(buffer, version, &snapshot) == XUI_OK);
    assert(xuiCodeDocumentInsert(xuiCodeEditGetDocument(editor), 0, "int ") == XUI_OK);
    assert(xuiCodeTokenBufferSet(buffer, &token, 1, version) == XUI_ERROR_INVALID_STATE);
    assert(xuiCodeEditGetDocumentVersion(editor, &version) == XUI_OK && version != cached);
    assert(xuiLayout(context) == XUI_OK && xuiRender(context, target, NULL, 0) == XUI_OK);
    assert(xuiCodeTokenBufferGetVersion(buffer, &cached) == XUI_OK && cached == version);

    xthread* worker = xrtThreadCreate(wrong_thread, buffer, 0);
    assert(worker && xrtThreadWait(worker) == XWAIT_OK);
    xrtThreadDestroy(worker);
    xui_lifetime_link_t watch = {0};
    int calls = 0;
    assert(xuiCodeTokenBufferEditorLifetimeWatch(buffer, &watch, invalidated, &calls) == XUI_OK);
    xui_call_scope_t scope = {0};
    assert(xuiCallScopeBegin(context, &scope) == XUI_OK);
    assert(xuiWidgetDestroyChecked(editor) == XUI_OK && calls == 1);
    assert(xuiCodeTokenBufferClearChecked(buffer) == XUI_ERROR_INVALID_STATE);
    assert(xuiCallScopeEnd(&scope) == XUI_OK);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
    xuiDestroy(context);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    const xui_code_token_t* data = NULL;
    size_t count = 0;
    assert(xuiCodeTokenSnapshotGetTokens(snapshot, &data, &count) == XUI_OK && count == 1);
    assert(data[0].iStyle == XUI_CODE_STYLE_COMMENT && data[0].iEndOffset == 4);
    xuiCodeTokenSnapshotDestroy(snapshot);
}

int main(void)
{
#ifdef XGE_XRT_HOST
    const xge_host_xrt_contract_t binding = XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&binding, resolve, NULL, NULL) == XGE_HOST_XRT_OK);
#endif
    assert(xrtMemDebugEnable(true));
    check_snapshot_storage();
    check_editor_cache();
    puts("code_edit_token_snapshot: shared arrays, atomic writes, render consumption and lifetime passed");
    return 0;
}
