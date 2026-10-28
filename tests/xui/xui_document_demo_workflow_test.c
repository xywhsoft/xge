/* Exercise the actual demo callbacks through XUI pointer/key dispatch and the
 * real XGE renderer. This is synthetic UI input, not physical IME acceptance. */
#define main document_demo_original_main
#include "../examples/xui/xui_document/main.c"
#undef main
#include "xui_test_time.h"

static demo app;
static const char* destination;
static uint32_t markdown_mode;
static const char* close_mode;
static unsigned workflow_checks;
#define REQUIRE(e) do { workflow_checks++; if (!(e)) { fprintf(stderr, "workflow line %d: %s\n", __LINE__, #e); app.result = XUI_ERROR_INVALID_STATE; xgeQuit(); return app.result; } } while (0)

static int ui_click(xui_widget widget)
{
    xui_rect_t rect; int r;
    if (!widget) return XUI_ERROR_INVALID_ARGUMENT;
    r = xuiLayout(app.context); if (r != XUI_OK) return r;
    rect = xuiWidgetGetWorldRect(widget);
    if (rect.fW <= 0 || rect.fH <= 0) return XUI_ERROR_INVALID_STATE;
    r = xuiInputPointerMove(app.context, rect.fX + rect.fW / 2, rect.fY + rect.fH / 2, 0);
    if (r == XUI_OK) r = xuiInputPointerDown(app.context, rect.fX + rect.fW / 2, rect.fY + rect.fH / 2, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
    if (r == XUI_OK) r = xuiInputPointerUp(app.context, rect.fX + rect.fW / 2, rect.fY + rect.fH / 2, XUI_POINTER_BUTTON_LEFT, 0);
    if (r == XUI_OK) r = xuiDispatchPendingEvents(app.context);
    return r;
}
static int ui_key(int key)
{
    int r = xuiInputKeyDown(app.context, key, 0);
    if (r == XUI_OK) r = xuiInputKeyUp(app.context, key, 0);
    if (r == XUI_OK) r = xuiDispatchPendingEvents(app.context);
    return r;
}
static int ui_wait_edit(xui_widget editor)
{
    xui_test_deadline_t deadline = xuiTestDeadlineAfterUs(5000000);
    do {
        xui_doc_editor_pending_work_t pending = {0}; int r;
        r = xuiUpdate(app.context, .016f);
        if (r != XUI_OK) return r;
        pending.iSize = sizeof(pending);
        r = xuiDocumentEditorGetPendingWork(editor, &pending);
        if (r != XUI_OK) return r;
        if (!pending.bHasInput && !pending.iQueuedEvents &&
            !xuiDocumentHasPrepare(xuiDocumentViewGetDocument(editor))) return pending.iResult;
        xrtThreadYield();
    } while (!xuiTestDeadlineExpired(deadline));
    return XUI_DOC_ERROR_BUSY;
}
static int ui_shortcut(int key)
{
    int r = xuiInputKeyDown(app.context, key, XUI_MOD_CTRL);
    /* Release the shortcut modifiers too; text events use the last modifier
     * snapshot and correctly suppress ordinary typing while Ctrl is held. */
    if (r == XUI_OK) r = xuiInputKeyUp(app.context, key, 0);
    if (r == XUI_OK) r = xuiDispatchPendingEvents(app.context);
    if (r == XUI_OK) r = ui_wait_edit(xuiGetFocusWidget(app.context));
    return r;
}
static xui_widget toolbar(const char* name)
{
    xui_widget widget;
    for (widget = xuiWidgetGetFirstChild(app.root); widget; widget = xuiWidgetGetNextSibling(widget)) {
        const char* text = xuiButtonGetText(widget);
        if (text && !strcmp(text, name)) return widget;
    }
    return NULL;
}
static int capture_dialog(const char* name)
{
    xui_rect_i_t damage = {0,0,W,H}; uint64_t source, preview; char path[1024]; int r;
    r = xuiLayout(app.context);
    if (r == XUI_OK) r = xuiUpdate(app.context, .016f);
    if (r == XUI_OK) r = app.proxy.surfaceClear(&app.proxy, app.target, XUI_COLOR_WHITE);
    if (r == XUI_OK) r = xuiRender(app.context, app.target, &damage, 1);
    snprintf(path, sizeof(path), "%s/%s", destination, name);
    if (r == XUI_OK) r = capture_async(&app, path, &source, &preview);
    return r;
}
static int prepare_dialog_name(const char* name)
{
    int r = xuiFileDialogSetDirectory(app.file_dialog, destination);
    if (r == XUI_OK) r = xuiInputSetText(xuiFileDialogGetNameInputWidget(app.file_dialog), name);
    return r;
}
static int file_workflow(int rich)
{
    xui_widget editor = rich ? app.rich : app.markdown;
    xui_document current, expected = NULL, retained;
    char path[1024]; uint64_t revision;
    const char* filename = rich ? "rich.xdoc" : "markdown.md";
    REQUIRE(xuiSetFocusWidget(app.context, editor) == XUI_OK);
    app.active_editor = editor;
    current = xuiDocumentViewGetDocument(editor);
    REQUIRE(xuiDocumentIsDirty(current));
    revision = xuiDocumentGetRevision(current);

    /* Cancel the actual Save dialog; it must not mark a document saved. */
    REQUIRE(ui_click(toolbar("Save as")) == XUI_OK);
    REQUIRE(app.file_dialog && xuiFileDialogIsOpen(app.file_dialog) && app.file_save);
    REQUIRE(ui_click(xuiFileDialogGetCancelButtonWidget(app.file_dialog)) == XUI_OK);
    REQUIRE(!xuiFileDialogIsOpen(app.file_dialog) && xuiDocumentIsDirty(current));
    REQUIRE(xuiDocumentGetRevision(current) == revision);
    REQUIRE(xuiGetFocusWidget(app.context) == editor);

    REQUIRE(ui_click(toolbar("Save as")) == XUI_OK);
    REQUIRE(prepare_dialog_name(filename) == XUI_OK);
    REQUIRE(capture_dialog(rich ? "rich-save.png" : "markdown-save.png") == XUI_OK);
    REQUIRE(ui_click(xuiFileDialogGetOkButtonWidget(app.file_dialog)) == XUI_OK);
    REQUIRE(!xuiFileDialogIsOpen(app.file_dialog) && !xuiDocumentIsDirty(current));
    REQUIRE(xuiGetFocusWidget(app.context) == editor);
    snprintf(path, sizeof(path), "%s/%s", destination, filename);
    REQUIRE(xuiDocumentOpenFile(path, rich ? XUI_DOC_FILE_NATIVE : XUI_DOC_FILE_MARKDOWN, NULL, 0, &expected) == XUI_OK);
    REQUIRE(same_document_content(current, expected) == XUI_OK);
    /* Resume through the focused input route, rather than calling InsertText
     * on an editor that might have lost focus when the dialog closed. */
    revision = xuiDocumentGetRevision(current);
    REQUIRE(xuiInputText(app.context, 0x4e2d) == XUI_OK);
    REQUIRE(xuiDispatchPendingEvents(app.context) == XUI_OK);
    REQUIRE(ui_wait_edit(editor) == XUI_OK);
    REQUIRE(xuiDocumentGetRevision(current) == revision + 1 && xuiDocumentIsDirty(current));
    REQUIRE(ui_shortcut('Z') == XUI_OK);
    REQUIRE(same_document_content(current, expected) == XUI_OK && !xuiDocumentIsDirty(current));

    REQUIRE(xuiDocumentEditorInsertText(editor, "Unsaved 中文\n", strlen("Unsaved 中文\n")) == XUI_OK);
    REQUIRE(xuiDocumentIsDirty(current));
    retained = current; xuiDocumentRetain(retained); revision = xuiDocumentGetRevision(current);
    REQUIRE(ui_click(toolbar("Open")) == XUI_OK);
    REQUIRE(prepare_dialog_name(filename) == XUI_OK);
    REQUIRE(ui_click(xuiFileDialogGetOkButtonWidget(app.file_dialog)) == XUI_OK);
    REQUIRE(app.pending_open && app.discard_box && xuiMsgBoxIsOpen(app.discard_box));
    REQUIRE(xuiDocumentViewGetDocument(editor) == retained);
    REQUIRE(capture_dialog(rich ? "rich-discard.png" : "markdown-discard.png") == XUI_OK);
    REQUIRE(ui_click(xuiMsgBoxGetButtonWidget(app.discard_box, 1)) == XUI_OK);
    REQUIRE(!app.pending_open && !xuiMsgBoxIsOpen(app.discard_box));
    REQUIRE(xuiDocumentViewGetDocument(editor) == retained && xuiDocumentGetRevision(retained) == revision && xuiDocumentIsDirty(retained));
    REQUIRE(xuiGetFocusWidget(app.context) == editor);
    if (!rich) REQUIRE(xuiDocumentViewGetDocument(app.preview) == retained);

    /* Escape from the file dialog keeps both document bindings. */
    REQUIRE(ui_click(toolbar("Open")) == XUI_OK);
    REQUIRE(ui_key(XUI_KEY_ESCAPE) == XUI_OK);
    REQUIRE(!xuiFileDialogIsOpen(app.file_dialog) && !app.pending_open);
    REQUIRE(xuiDocumentViewGetDocument(editor) == retained && xuiDocumentGetRevision(retained) == revision);
    REQUIRE(xuiGetFocusWidget(app.context) == editor);

    REQUIRE(ui_click(toolbar("Open")) == XUI_OK);
    REQUIRE(prepare_dialog_name(filename) == XUI_OK);
    REQUIRE(ui_click(xuiFileDialogGetOkButtonWidget(app.file_dialog)) == XUI_OK);
    REQUIRE(app.pending_open && xuiMsgBoxIsOpen(app.discard_box));
    REQUIRE(ui_click(xuiMsgBoxGetButtonWidget(app.discard_box, 0)) == XUI_OK);
    REQUIRE(!app.pending_open && !xuiMsgBoxIsOpen(app.discard_box));
    current = xuiDocumentViewGetDocument(editor);
    REQUIRE(current != retained && !xuiDocumentIsDirty(current));
    REQUIRE(same_document_content(current, expected) == XUI_OK);
    REQUIRE(xuiGetFocusWidget(app.context) == editor);
    if (!rich) REQUIRE(xuiDocumentViewGetDocument(app.preview) == current);
    REQUIRE(xuiInputText(app.context, 0x6587) == XUI_OK);
    REQUIRE(xuiDispatchPendingEvents(app.context) == XUI_OK);
    REQUIRE(ui_wait_edit(editor) == XUI_OK);
    REQUIRE(xuiDocumentIsDirty(current));
    REQUIRE(ui_shortcut('Z') == XUI_OK);
    REQUIRE(same_document_content(current, expected) == XUI_OK && !xuiDocumentIsDirty(current));
    xuiDocumentRelease(expected); xuiDocumentRelease(retained);
    return XUI_OK;
}
static int pending_discard_workflow(void)
{
    xui_document current = app.md_doc, expected = NULL;
    xui_document_prepare pending = NULL;
    xui_doc_source_patch_t patch = {.iSize = sizeof(patch), .iStart = 0, .iEnd = 0, .sText = "pending\n", .iTextBytes = 8};
    char path[1024], busy_message[64]; uint64_t revision;
    snprintf(path, sizeof(path), "%s/discard-base.md", destination);
    REQUIRE(xuiDocumentSaveFile(current, path, XUI_DOC_FILE_MARKDOWN) == XUI_OK);
    REQUIRE(xuiDocumentOpenFile(path, XUI_DOC_FILE_MARKDOWN, NULL, 0, &expected) == XUI_OK);
    REQUIRE(xuiDocumentEditorInsertText(app.markdown, "Guarded edit\n", strlen("Guarded edit\n")) == XUI_OK);
    revision = xuiDocumentGetRevision(current);
    REQUIRE(xuiSetFocusWidget(app.context, app.markdown) == XUI_OK);
    app.active_editor = app.markdown;
    REQUIRE(ui_click(toolbar("Open")) == XUI_OK);
    REQUIRE(prepare_dialog_name("discard-base.md") == XUI_OK);
    REQUIRE(ui_click(xuiFileDialogGetOkButtonWidget(app.file_dialog)) == XUI_OK);
    REQUIRE(app.pending_open && app.discard_box && xuiMsgBoxIsOpen(app.discard_box));
    /* A host candidate starts after the discard prompt opened. Its unpublished
     * input must not be detached by the eventual confirmation. */
    REQUIRE(xuiDocumentPrepareSource(current, NULL, &patch, 1, &pending) == XUI_OK);
    REQUIRE(capture_dialog("pending-discard.png") == XUI_OK);
    REQUIRE(ui_click(xuiMsgBoxGetButtonWidget(app.discard_box, 0)) == XUI_OK);
    REQUIRE(!app.pending_open && !xuiMsgBoxIsOpen(app.discard_box));
    REQUIRE(xuiDocumentViewGetDocument(app.markdown) == current && app.md_doc == current);
    REQUIRE(xuiDocumentViewGetDocument(app.preview) == current);
    REQUIRE(xuiDocumentHasPrepare(current) && xuiDocumentGetRevision(current) == revision && xuiDocumentIsDirty(current));
    snprintf(busy_message, sizeof(busy_message), "Operation returned %d.", XUI_DOC_ERROR_BUSY);
    REQUIRE(strstr(xuiLabelGetText(app.status), busy_message) != NULL);
    REQUIRE(xuiGetFocusWidget(app.context) == app.markdown);
    REQUIRE(xuiDocumentCancelPrepare(current) == XUI_OK);
    xuiDocumentPrepareRelease(pending);
    REQUIRE(ui_shortcut('Z') == XUI_OK);
    REQUIRE(same_document_content(current, expected) == XUI_OK && !xuiDocumentIsDirty(current));
    xuiDocumentRelease(expected);
    return XUI_OK;
}
static int close_workflow(void)
{
    xui_document_prepare pending = NULL;
    uint64_t revision = xuiDocumentGetRevision(app.rich_doc);
    REQUIRE(xuiProxyXgePumpInputRect(app.context, (xui_rect_t){0,0,W,H}) == XUI_OK);
    if (!strcmp(close_mode, "clean")) {
        REQUIRE(!xuiDocumentIsDirty(app.rich_doc) && !xuiDocumentIsDirty(app.md_doc));
        REQUIRE(xgeRequestQuit() == 0 && app.close_requested);
        REQUIRE(service_close(&app) == XUI_OK && !app.close_requested && !app.close_box);
        return XUI_OK;
    }
    if (!strcmp(close_mode, "pending")) {
        xui_doc_source_patch_t patch = {.iSize = sizeof(patch), .iStart = 0, .iEnd = 0, .sText = "pending\n", .iTextBytes = 8};
        REQUIRE(xuiDocumentPrepareSource(app.md_doc, NULL, &patch, 1, &pending) == XUI_OK);
        REQUIRE(xuiDocumentHasPrepare(app.md_doc) && !xuiDocumentIsDirty(app.md_doc));
    } else {
        xge_input_event_t text = {0}; text.iSize = sizeof(text);
        text.iType = XGE_EVENT_TEXT; text.iFlags = XGE_INPUT_EVENT_FLAG_SYNTHETIC; text.iCodepoint = 'Z';
        /* A text event queued just before close must be dispatched before the
         * app decides whether committed content is dirty. */
        REQUIRE(xuiSetFocusWidget(app.context, app.rich) == XUI_OK);
        REQUIRE(xgeInputEventPost(&text) == XGE_OK);
        REQUIRE(xuiDocumentGetRevision(app.rich_doc) == revision);
    }
    REQUIRE(xgeRequestQuit() == 0 && app.close_requested);
    REQUIRE(!app.close_box);
    REQUIRE(xuiProxyXgePumpInputRect(app.context, (xui_rect_t){0,0,W,H}) == XUI_OK);
    REQUIRE(xuiDispatchPendingEvents(app.context) == XUI_OK);
    if (!pending) REQUIRE(xuiDocumentIsDirty(app.rich_doc) && xuiDocumentGetRevision(app.rich_doc) == revision + 1);
    REQUIRE(service_close(&app) == XUI_OK && app.close_box && xuiMsgBoxIsOpen(app.close_box));
    REQUIRE(capture_dialog("close-prompt.png") == XUI_OK);
    if (!strcmp(close_mode, "discard")) {
        REQUIRE(ui_click(xuiMsgBoxGetButtonWidget(app.close_box, 1)) == XUI_OK);
        REQUIRE(!app.close_requested && !app.close_saving && xuiDocumentIsDirty(app.rich_doc));
        return XUI_OK;
    }
    if (!strcmp(close_mode, "cancel")) {
        REQUIRE(ui_click(xuiMsgBoxGetButtonWidget(app.close_box, 2)) == XUI_OK);
        REQUIRE(!app.close_requested && !app.close_saving && xuiDocumentIsDirty(app.rich_doc));
        REQUIRE(xgeRequestQuit() == 0 && service_close(&app) == XUI_OK);
    }
    REQUIRE(ui_click(xuiMsgBoxGetButtonWidget(app.close_box, 0)) == XUI_OK && app.close_saving);
    if (pending) {
        REQUIRE(service_close(&app) == XUI_OK && app.close_saving);
        REQUIRE(!xuiFileDialogIsOpen(app.file_dialog) && xuiDocumentHasPrepare(app.md_doc));
        REQUIRE(xgeRequestQuit() == 0 && service_close(&app) == XUI_OK);
        REQUIRE(xuiMsgBoxIsOpen(app.close_box));
        REQUIRE(ui_click(xuiMsgBoxGetButtonWidget(app.close_box, 2)) == XUI_OK);
        REQUIRE(!app.close_requested && !app.close_saving && xuiDocumentHasPrepare(app.md_doc));
        REQUIRE(xuiDocumentCancelPrepare(app.md_doc) == XUI_OK);
        xuiDocumentPrepareRelease(pending);
        REQUIRE(!xuiDocumentHasPrepare(app.md_doc) && !xuiDocumentIsDirty(app.md_doc));
        return XUI_OK;
    }
    if (!strcmp(close_mode, "save")) {
        REQUIRE(xuiDocumentEditorInsertText(app.markdown, "Close save\n", strlen("Close save\n")) == XUI_OK);
    }
    REQUIRE(service_close(&app) == XUI_OK && app.file_save && xuiFileDialogIsOpen(app.file_dialog));
    REQUIRE(app.file_editor == app.rich);
    if (!strcmp(close_mode, "cancel")) {
        REQUIRE(ui_click(xuiFileDialogGetCancelButtonWidget(app.file_dialog)) == XUI_OK);
        REQUIRE(!app.close_saving && !app.close_requested && xuiDocumentIsDirty(app.rich_doc));
        REQUIRE(xgeRequestQuit() == 0 && service_close(&app) == XUI_OK);
        REQUIRE(ui_key(XUI_KEY_ESCAPE) == XUI_OK);
        REQUIRE(!app.close_requested && !app.close_saving && !xuiMsgBoxIsOpen(app.close_box));
    } else {
        xui_document saved = NULL; char path[1024];
        REQUIRE(prepare_dialog_name("closing-rich.xdoc") == XUI_OK);
        REQUIRE(ui_click(xuiFileDialogGetOkButtonWidget(app.file_dialog)) == XUI_OK);
        REQUIRE(!xuiDocumentIsDirty(app.rich_doc) && app.close_saving);
        REQUIRE(service_close(&app) == XUI_OK && xuiFileDialogIsOpen(app.file_dialog) && app.file_editor == app.markdown);
        REQUIRE(prepare_dialog_name("closing-markdown.md") == XUI_OK);
        REQUIRE(ui_click(xuiFileDialogGetOkButtonWidget(app.file_dialog)) == XUI_OK);
        REQUIRE(!xuiDocumentIsDirty(app.md_doc));
        snprintf(path,sizeof(path),"%s/closing-rich.xdoc",destination);
        REQUIRE(xuiDocumentOpenFile(path,XUI_DOC_FILE_NATIVE,NULL,0,&saved) == XUI_OK);
        REQUIRE(same_document_content(saved,app.rich_doc) == XUI_OK); xuiDocumentRelease(saved); saved = NULL;
        snprintf(path,sizeof(path),"%s/closing-markdown.md",destination);
        REQUIRE(xuiDocumentOpenFile(path,XUI_DOC_FILE_MARKDOWN,NULL,0,&saved) == XUI_OK);
        REQUIRE(same_document_content(saved,app.md_doc) == XUI_OK); xuiDocumentRelease(saved);
        REQUIRE(service_close(&app) == XUI_OK && !app.close_saving && !app.close_requested);
    }
    return XUI_OK;
}
static int pending_file_workflow(void)
{
    xui_document current = app.md_doc; xui_document_prepare pending = NULL;
    xui_doc_source_patch_t patch = {.iSize = sizeof(patch), .iStart = 0, .iEnd = 0, .sText = "pending\n", .iTextBytes = 8};
    uint64_t revision = xuiDocumentGetRevision(current);
    for (int save = 0; save < 2; save++) {
        REQUIRE(xuiSetFocusWidget(app.context, app.markdown) == XUI_OK);
        app.active_editor = app.markdown;
        REQUIRE(ui_click(toolbar(save ? "Save as" : "Open")) == XUI_OK);
        /* A host edit can become pending while the file dialog is open. */
        REQUIRE(xuiDocumentPrepareSource(current, NULL, &patch, 1, &pending) == XUI_OK);
        REQUIRE(prepare_dialog_name(save ? "pending-save.md" : "markdown.md") == XUI_OK);
        REQUIRE(ui_click(xuiFileDialogGetOkButtonWidget(app.file_dialog)) == XUI_OK);
        REQUIRE(!xuiFileDialogIsOpen(app.file_dialog));
        REQUIRE(xuiDocumentViewGetDocument(app.markdown) == current && app.md_doc == current);
        REQUIRE(xuiDocumentViewGetDocument(app.preview) == current);
        REQUIRE(xuiDocumentHasPrepare(current) && xuiDocumentGetRevision(current) == revision && !xuiDocumentIsDirty(current));
        REQUIRE(xuiGetFocusWidget(app.context) == app.markdown);
        REQUIRE(xuiDocumentCancelPrepare(current) == XUI_OK);
        xuiDocumentPrepareRelease(pending); pending = NULL;
    }
    return XUI_OK;
}
static int workflow_frame(void* unused)
{
    int r; (void)unused;
#ifdef _WIN32
    ShowWindow((HWND)xgePlatformNativeHandle(), SW_HIDE);
#endif
    REQUIRE(resources(&app) == XUI_OK);
    REQUIRE(xgeBegin() == XGE_OK);
    REQUIRE(file_workflow(1) == XUI_OK);
    REQUIRE(xuiDocumentViewSetMode(app.markdown, markdown_mode) == XUI_OK);
    REQUIRE(file_workflow(0) == XUI_OK);
    REQUIRE(pending_discard_workflow() == XUI_OK);
    REQUIRE(pending_file_workflow() == XUI_OK);
    REQUIRE(capture_dialog("after-open.png") == XUI_OK);
    REQUIRE(close_workflow() == XUI_OK);
    r = xgeEnd(); REQUIRE(r == XGE_OK);
    app.result = XUI_OK; xgeQuit();
    return XGE_OK;
}
int main(int argc, char** argv)
{
    xge_desc_t desc = {0}; int r;
    if (argc != 4) return 2;
    destination = argv[1]; markdown_mode = !strcmp(argv[2], "source") ? XUI_DOC_SOURCE_TEXT :
        !strcmp(argv[2], "visual") ? XUI_DOC_VISUAL : XUI_DOC_LIVE_MARKDOWN;
    close_mode = argv[3];
    desc.iWidth = W; desc.iHeight = H; desc.sTitle = "Document file workflows";
    desc.iFlags = XGE_INIT_OFFSCREEN; desc.iRunMode = XGE_RUN_GAME_LOOP;
    if (xgeInit(&desc) != XGE_OK) return 2;
    r = xgeRun(workflow_frame, NULL); if (r == XGE_OK) r = app.result;
    xgeSetQuitRequestCallback(NULL, NULL);
    if (app.file_dialog) xuiFileDialogDestroy(app.file_dialog);
    if (app.discard_box) xuiMsgBoxDestroy(app.discard_box);
    if (app.close_box) xuiMsgBoxDestroy(app.close_box);
    xuiDocumentRelease(app.pending_open);
    xuiDestroy(app.context); xuiDocumentRelease(app.rich_doc); xuiDocumentRelease(app.md_doc);
    if (app.proxy.surfaceDestroy) app.proxy.surfaceDestroy(&app.proxy, app.target);
    if (app.proxy.fontDestroy) { app.proxy.fontDestroy(&app.proxy, app.mono); app.proxy.fontDestroy(&app.proxy, app.italic); app.proxy.fontDestroy(&app.proxy, app.bold); app.proxy.fontDestroy(&app.proxy, app.font); }
    xgeFontFallbackClear(); xgeUnit();
    if (r == XUI_OK) printf("Native demo workflow %s/%s: %u pointer/key/callback, file/dialog/close and shared-preview checks passed.\n", argv[2], close_mode, workflow_checks);
    return r == XUI_OK ? 0 : 1;
}
