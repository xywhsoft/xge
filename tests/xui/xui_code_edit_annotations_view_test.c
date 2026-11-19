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

static void invalidated(void* user)
{
    ++*(int*)user;
}

static int wrong_thread(void* user)
{
    xui_code_annotation_store store = user;
    xui_code_annotation_snapshot view = NULL;
    assert(xuiCodeAnnotationGetDiagnosticCount(store) == XUI_ERROR_BUSY);
    assert(xuiCodeAnnotationStoreClearChecked(store) == XUI_ERROR_BUSY);
    assert(xuiCodeAnnotationCaptureMarkers(store, 0, &view) == XUI_ERROR_BUSY && view == NULL);
    return 0;
}

static void compare_query_allocations(xui_code_annotation_store store)
{
    xmemdebugsnapshot before, borrowed, owned;
    xrtMemDebugSnapshot(&before);
    for ( int i = 0; i < 1000; ++i ) {
        int count;
        xui_code_marker_t marker;
        xui_code_indicator_t indicator;
        xui_code_diagnostic_t diagnostic;
        assert(xuiCodeAnnotationGetMarkers(store, 0, &marker, 1, &count) == XUI_OK && count == 1);
        assert(xuiCodeAnnotationGetIndicatorsAt(store, 1, &indicator, 1, &count) == XUI_OK && count == 1);
        assert(xuiCodeAnnotationGetDiagnosticsAt(store, 1, &diagnostic, 1, &count) == XUI_OK && count == 1);
        assert(!strcmp(marker.sTooltip, "bookmark") && indicator.tRange.iEnd == 4);
        assert(!strcmp(diagnostic.sMessage, "old"));
    }
    xrtMemDebugSnapshot(&borrowed);
    assert(borrowed.AllocCount == before.AllocCount && borrowed.FreeCount == before.FreeCount);
    for ( int i = 0; i < 1000; ++i ) {
        xui_code_annotation_snapshot markers, indicators, diagnostics;
        assert(xuiCodeAnnotationCaptureMarkers(store, 0, &markers) == XUI_OK);
        assert(xuiCodeAnnotationCaptureIndicatorsAt(store, 1, &indicators) == XUI_OK);
        assert(xuiCodeAnnotationCaptureDiagnosticsAt(store, 1, &diagnostics) == XUI_OK);
        assert(xuiCodeAnnotationSnapshotGetCount(markers) == 1);
        assert(xuiCodeAnnotationSnapshotGetCount(indicators) == 1);
        assert(xuiCodeAnnotationSnapshotGetCount(diagnostics) == 1);
        xui_code_marker_t marker;
        xui_code_indicator_t indicator;
        xui_code_diagnostic_capture_t diagnostic;
        assert(xuiCodeAnnotationSnapshotGetMarker(markers, 0, &marker) == XUI_OK);
        assert(xuiCodeAnnotationSnapshotGetIndicator(indicators, 0, &indicator) == XUI_OK);
        assert(xuiCodeAnnotationSnapshotGetDiagnostic(diagnostics, 0, &diagnostic) == XUI_OK);
        assert(!strcmp(marker.sTooltip, "bookmark") && indicator.tRange.iEnd == 4);
        assert(!strcmp(diagnostic.tDiagnostic.sMessage, "old"));
        xuiCodeAnnotationSnapshotDestroy(markers);
        xuiCodeAnnotationSnapshotDestroy(indicators);
        xuiCodeAnnotationSnapshotDestroy(diagnostics);
    }
    xrtMemDebugSnapshot(&owned);
    assert(owned.AllocCount - borrowed.AllocCount == 3000);
    assert(owned.FreeCount - borrowed.FreeCount == 3000);
    assert(owned.ReallocCount == before.ReallocCount && owned.LiveBytes == before.LiveBytes);
    puts("annotation queries allocation counts: borrowed 0/0/0; owned 3000/3000/0");
}

int main(void)
{
#ifdef XGE_XRT_HOST
    const xge_host_xrt_contract_t binding = XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&binding, resolve, NULL, NULL) == XGE_HOST_XRT_OK);
#endif
    xui_test_proxy_state_t proxy;
    xuiTestProxyInit(&proxy);
    xui_context_desc_t desc;
    xuiContextDescDefault(&desc);
    desc.iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED;
    xui_context context = NULL;
    assert(xuiCreateEx(&desc, &context) == XUI_OK);
    assert(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    xui_widget editor = NULL;
    assert(xuiCodeEditCreate(context, &editor, NULL) == XUI_OK);
    assert(xuiSetRootWidget(context, editor) == XUI_OK);
    assert(xuiCodeEditSetText(editor, "TODO one\nTODO two\n") == XUI_OK);
    xui_code_document document = xuiCodeEditGetDocument(editor);
    xui_code_annotation_store store = xuiCodeEditGetAnnotations(editor);
    const uint32_t dirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
    assert(xuiCodeAnnotationStoreDestroyChecked(store) == XUI_ERROR_INVALID_STATE);
    xuiWidgetClearDirty(editor, dirty);
    assert(xuiCodeAnnotationSetMarker(store, 0, XUI_CODE_MARKER_BOOKMARK, 0, "bookmark", 9) == XUI_OK);
    assert((xuiWidgetGetDirtyFlags(editor) & dirty) == dirty);
    xuiWidgetClearDirty(editor, dirty);
    int count;
    assert(xuiCodeEditMarkAllPlain(editor, "TODO", 0, 0, &count) == XUI_OK && count == 2);
    assert((xuiWidgetGetDirtyFlags(editor) & dirty) == dirty);
    xuiWidgetClearDirty(editor, dirty);
    assert(xuiCodeEditMarkAllRegex(editor, "T[O]DO", 0, 0, &count, NULL, 0) == XUI_OK && count == 2);
    assert((xuiWidgetGetDirtyFlags(editor) & dirty) == dirty);
    xui_code_document other = NULL;
    assert(xuiCodeDocumentCreate(&other) == XUI_OK);
    assert(xuiCodeSearchMarkAllPlain(other, store, "x", 0, 0, &count) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiCodeSearchMarkAllRegex(other, store, "x", 0, 0, &count, NULL, 0) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiCodeDocumentDestroyChecked(other) == XUI_OK);

    xui_code_diagnostic_t diagnostics[] = {
        {.tRange = {9,13}, .sMessage = "later"},
        {.tRange = {0,4}, .sCode = "", .sMessage = "old", .iUserData = 17}
    };
    uint32_t version = xuiCodeDocumentGetVersion(document);
    assert(xuiCodeEditApplyDiagnostics(editor, version, diagnostics, 2) == XUI_OK);
    xuiWidgetClearDirty(editor, dirty);
    assert(xuiCodeAnnotationSetDiagnostics(store, diagnostics, 2) == XUI_OK);
    assert(!(xuiWidgetGetDirtyFlags(editor) & dirty));
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    compare_query_allocations(store);
    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);
    xui_code_annotation_snapshot failed = NULL;
    assert(xrtMemDebugFailAfter(0));
    assert(xuiCodeAnnotationCaptureDiagnosticsInRange(store, 0, 13, &failed) == XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();
    xrtClearError();
    xrtMemDebugSnapshot(&after);
    assert(!failed && before.LiveCount == after.LiveCount && before.LiveBytes == after.LiveBytes);
    assert(xuiCodeEditGetDiagnosticCount(editor) == 2 && !(xuiWidgetGetDirtyFlags(editor) & dirty));

    xui_code_annotation_snapshot marker, captured;
    assert(xuiCodeAnnotationCaptureMarkers(store, 0, &marker) == XUI_OK);
    assert(xuiCodeAnnotationCaptureDiagnosticsInRange(store, 0, 13, &captured) == XUI_OK);
    assert(xuiCodeAnnotationSnapshotGetCount(captured) == 2);
    diagnostics[1].sMessage = "replacement";
    assert(xuiCodeEditApplyDiagnostics(editor, version, diagnostics, 2) == XUI_OK);
    assert(xuiCodeAnnotationStoreClearChecked(store) == XUI_OK);
    assert(xuiCodeEditGetDiagnosticCount(editor) == 0);
    xthread* worker = xrtThreadCreate(wrong_thread, store, 0);
    assert(worker && xrtThreadWait(worker) == XWAIT_OK);
    xrtThreadDestroy(worker);
    xui_lifetime_link_t watch = {0};
    int calls = 0;
    assert(xuiCodeAnnotationEditorLifetimeWatch(store, &watch, invalidated, &calls) == XUI_OK);
    xui_call_scope_t scope = {0};
    assert(xuiCallScopeBegin(context, &scope) == XUI_OK);
    assert(xuiWidgetDestroyChecked(editor) == XUI_OK && calls == 1);
    assert(xuiCodeAnnotationStoreClearChecked(store) == XUI_ERROR_INVALID_STATE);
    assert(xuiCallScopeEnd(&scope) == XUI_OK);
    xuiDestroy(context);

    xui_code_marker_t markerInfo;
    xui_code_diagnostic_capture_t info;
    assert(xuiCodeAnnotationSnapshotGetMarker(marker, 0, &markerInfo) == XUI_OK);
    assert(!strcmp(markerInfo.sTooltip, "bookmark") && markerInfo.iUserData == 9);
    assert(xuiCodeAnnotationSnapshotGetDiagnostic(captured, 0, &info) == XUI_OK);
    assert(info.iSourceIndex == 1 && info.tDiagnostic.iUserData == 17);
    assert(!strcmp(info.tDiagnostic.sCode, "") && !strcmp(info.tDiagnostic.sMessage, "old"));
    assert(xuiCodeAnnotationSnapshotGetDiagnostic(marker, 0, &info) == XUI_ERROR_INVALID_ARGUMENT);
    xuiCodeAnnotationSnapshotDestroy(marker);
    xuiCodeAnnotationSnapshotDestroy(captured);
    puts("code_edit_annotations_view: native snapshots, mutation, retirement and rollback passed");
    return 0;
}
