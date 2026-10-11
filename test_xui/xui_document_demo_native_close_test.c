/* Native WM_CHAR/WM_CLOSE integration; no physical IME or UIA automation. */
#define main document_demo_original_main
#include "../examples/xui_document/main.c"
#undef main

static demo app;
static const char* destination;
static unsigned phase, checks, native_frames;
static uint64_t before_revision;
static int succeeded;
#define REQUIRE(e) do { checks++; if (!(e)) { fprintf(stderr,"native close line %d: %s\n",__LINE__,#e); app.result = XUI_ERROR_INVALID_STATE; xgeQuit(); return app.result; } } while (0)

static int button_click(xui_widget button)
{
    xui_rect_t rect; int r = xuiLayout(app.context);
    if (r != XUI_OK || !button) return XUI_ERROR_INVALID_ARGUMENT;
    rect = xuiWidgetGetWorldRect(button);
    r = xuiInputPointerMove(app.context,rect.fX+rect.fW/2,rect.fY+rect.fH/2,0);
    if (r == XUI_OK) r = xuiInputPointerDown(app.context,rect.fX+rect.fW/2,rect.fY+rect.fH/2,XUI_POINTER_BUTTON_LEFT,XUI_POINTER_BUTTON_LEFT);
    if (r == XUI_OK) r = xuiInputPointerUp(app.context,rect.fX+rect.fW/2,rect.fY+rect.fH/2,XUI_POINTER_BUTTON_LEFT,0);
    if (r == XUI_OK) r = xuiDispatchPendingEvents(app.context);
    return r;
}
static int mark_saved(xui_document document)
{
    xui_document_snapshot snapshot = NULL; int r = xuiDocumentAcquireSnapshot(document,&snapshot);
    if (r == XUI_OK) r = xuiDocumentMarkSaved(document,snapshot);
    xuiDocumentSnapshotRelease(snapshot); return r;
}
static int native_frame(void* unused)
{
    HWND window; int r; (void)unused;
    REQUIRE(++native_frames < 300);
    r = frame(&app); REQUIRE(r == XGE_OK);
    window = (HWND)xgePlatformNativeHandle(); REQUIRE(window && IsWindow(window));
    if (!phase) {
        ShowWindow(window,SW_HIDE);
        REQUIRE(xuiProxyXgePumpInputRect(app.context,(xui_rect_t){0,0,W,H}) == XUI_OK);
        REQUIRE(mark_saved(app.rich_doc) == XUI_OK && mark_saved(app.md_doc) == XUI_OK);
        REQUIRE(xuiSetFocusWidget(app.context,app.markdown) == XUI_OK);
        before_revision = xuiDocumentGetRevision(app.md_doc);
        REQUIRE(PostMessageW(window,WM_CHAR,0x4e2d,1));
        REQUIRE(PostMessageW(window,WM_CHAR,0x6587,1));
        REQUIRE(xuiDocumentGetRevision(app.md_doc) == before_revision);
        REQUIRE(PostMessageW(window,WM_CLOSE,0,0));
        REQUIRE(IsWindow(window));
        phase = 1;
    } else if (phase == 1 && app.close_box && xuiMsgBoxIsOpen(app.close_box)) {
        char text[2048], image_path[1024]; uint64_t bytes, source, preview; xui_document_snapshot snapshot = NULL;
        REQUIRE(xuiDocumentIsDirty(app.md_doc) && xuiDocumentGetRevision(app.md_doc) == before_revision + 2);
        REQUIRE(xuiDocumentAcquireSnapshot(app.md_doc,&snapshot) == XUI_OK);
        REQUIRE(xuiDocumentSnapshotCopySource(snapshot,text,sizeof(text),&bytes) == XUI_OK);
        REQUIRE(bytes == strlen(md_text) + strlen("中文") && !strncmp(text,"中文",strlen("中文")) && !strcmp(text+strlen("中文"),md_text));
        xuiDocumentSnapshotRelease(snapshot);
        REQUIRE(xuiDocumentViewGetDocument(app.preview) == app.md_doc);
        snprintf(image_path,sizeof(image_path),"%s/native-close-prompt.png",destination);
        REQUIRE(capture_async(&app,image_path,&source,&preview) == XUI_OK);
        REQUIRE(button_click(xuiMsgBoxGetButtonWidget(app.close_box,2)) == XUI_OK);
        REQUIRE(!app.close_requested && !app.close_saving && !xuiMsgBoxIsOpen(app.close_box) && IsWindow(window));
        REQUIRE(xuiDocumentIsDirty(app.md_doc));
        REQUIRE(xuiGetFocusWidget(app.context) == app.markdown);
        REQUIRE(PostMessageW(window,WM_CLOSE,0,0));
        phase = 2;
    } else if (phase == 2 && app.close_box && xuiMsgBoxIsOpen(app.close_box)) {
        REQUIRE(button_click(xuiMsgBoxGetButtonWidget(app.close_box,1)) == XUI_OK);
        REQUIRE(!app.close_requested && !app.close_saving && xuiDocumentIsDirty(app.md_doc));
        succeeded = 1;
    }
    return XGE_OK;
}
int main(int argc,char** argv)
{
    xge_desc_t desc = {0}; int r;
    if (argc != 2) return 2;
    destination = argv[1]; desc.iWidth = W; desc.iHeight = H; desc.sTitle = "Native Document close";
    desc.iRunMode = XGE_RUN_GAME_LOOP; desc.iTargetFPS = 60; desc.iFlags = XGE_INIT_VSYNC;
    if (xgeInit(&desc) != XGE_OK) return 2;
    r = xgeRun(native_frame,NULL);
    fprintf(stderr,"native close result: run=%d demo=%d phase=%u frames=%u checks=%u succeeded=%d requested=%d saving=%d prompt=%d\n",
        r,app.result,phase,native_frames,checks,succeeded,app.close_requested,app.close_saving,
        app.close_box ? xuiMsgBoxIsOpen(app.close_box) : 0);
    xgeSetQuitRequestCallback(NULL,NULL);
    if (app.close_box) xuiMsgBoxDestroy(app.close_box);
    if (app.file_dialog) xuiFileDialogDestroy(app.file_dialog);
    if (app.discard_box) xuiMsgBoxDestroy(app.discard_box);
    xuiDocumentRelease(app.pending_open);
    xuiDestroy(app.context); xuiDocumentRelease(app.rich_doc); xuiDocumentRelease(app.md_doc);
    if (app.proxy.surfaceDestroy) app.proxy.surfaceDestroy(&app.proxy,app.target);
    if (app.proxy.fontDestroy) { app.proxy.fontDestroy(&app.proxy,app.mono); app.proxy.fontDestroy(&app.proxy,app.italic); app.proxy.fontDestroy(&app.proxy,app.bold); app.proxy.fontDestroy(&app.proxy,app.font); }
    xgeFontFallbackClear(); xgeUnit();
    if (r == XGE_OK && succeeded && app.result == XUI_OK) {
        printf("Native WM_CHAR/WM_CLOSE: Unicode before close, saved-state recheck, cancel/reopen/discard, shared preview and native window lifetime passed (%u checks).\n",checks);
        return 0;
    }
    return 1;
}
