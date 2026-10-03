#include "xui_document_ui.h"
#include "xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define W 1280
#define H 780
typedef struct demo demo;
typedef struct action { demo* app; int command; } action;
struct demo {
    xui_context context; xui_proxy_t proxy; xui_surface target;
    xui_font font, bold, italic, mono; xui_widget root, rich, markdown, preview, status, active_editor;
    xui_document rich_doc, md_doc; action actions[7];
    int verify, frames, result;
    uint64_t source_pixels;
    uint64_t async_revision, async_source_pixels, async_preview_pixels, async_pending_pixels;
};
static const char* md_text =
    "# Shared Markdown document\n\n"
    "This **preview** and the editor use the *same Document*.\n\n"
    "![Native image](demo.surface)\n\n"
    "> A real nested block quote.\n> - Keep source and content together\n> - One shared undo history\n\n"
    "- [x] Source preserved\n- [ ] Visual structural commands\n\n"
    "| Feature | Status |\n| :--- | ---: |\n| Snapshot | Shared |\n| Tables | Native |\n\n"
    "```c\n// A code fence\nxuiDocumentUndo(document, NULL);\n```\n";
static const char* md_image_test_text =
    "# Missing image\n\n![Preview alt text](missing.surface)\n";
static int root_draw(xui_widget w, xui_draw_context draw, uint32_t state, void* user)
{
    demo* d = user; (void)w; (void)state;
    d->proxy.drawRectFill(&d->proxy, draw, (xui_rect_t){0,0,W,H}, XUI_COLOR_RGBA(239,242,247,255));
    d->proxy.drawRectFill(&d->proxy, draw, (xui_rect_t){14,94,388,H-142}, XUI_COLOR_WHITE);
    d->proxy.drawRectFill(&d->proxy, draw, (xui_rect_t){414,94,410,H-142}, XUI_COLOR_WHITE);
    return d->proxy.drawRectFill(&d->proxy, draw, (xui_rect_t){836,94,W-850,H-142}, XUI_COLOR_WHITE);
}
static void show_error(xui_widget editor, int error, void* user)
{
    demo* d = user; char text[140]; (void)editor;
    snprintf(text, sizeof(text), "Operation returned %d. The document was not partially committed.", error);
    xuiLabelSetText(d->status, text);
}
static void image_destroy(xui_context context, void* handle, void* user)
{
    demo* d = user; (void)context;
    d->proxy.surfaceDestroy(&d->proxy, (xui_surface)handle);
}
static void click(xui_widget button, void* user)
{
    action* a = user; demo* d = a->app; xui_widget editor = xuiGetFocusWidget(d->context); int result;
    (void)button;
    if (editor != d->rich && editor != d->markdown) editor = d->active_editor ? d->active_editor : d->markdown;
    if (a->command >= 100) { editor = d->markdown; result = xuiDocumentViewSetMode(editor,
        a->command == 100 ? XUI_DOC_SOURCE_TEXT : a->command == 101 ? XUI_DOC_VISUAL : XUI_DOC_LIVE_MARKDOWN); }
    else result = xuiDocumentEditorExecute(editor, (uint32_t)a->command);
    if (result != XUI_OK) show_error(editor, result, d);
    else xuiLabelSetText(d->status, "Ctrl+Z undo | Ctrl+B/I format | Ctrl+wheel zoom | Source and preview share one document.");
    d->active_editor = editor; xuiSetFocusWidget(d->context, editor);
}
static int label(demo* d, const char* text, xui_rect_t rect, xui_widget* out)
{
    xui_label_desc_t desc = {0}; int r; desc.iSize = sizeof(desc); desc.sText = text; desc.pFont = d->font; desc.iTextColor = XUI_COLOR_RGBA(39,51,70,255);
    r = xuiLabelCreate(d->context, out, &desc); if (r != XUI_OK) return r;
    xuiWidgetSetRect(*out, rect); return xuiWidgetAddChild(d->root, *out);
}
static int setup(demo* d)
{
    xui_doc_desc_t md = {0}; xui_doc_editor_desc_t editor = {0}; xui_doc_view_desc_t view = {0}; xui_widget caption;
    const char* labels[] = {"Undo", "Redo", "Bold", "Italic", "Source", "Visual", "Live MD"};
    const int commands[] = {XUI_DOC_EDIT_UNDO,XUI_DOC_EDIT_REDO,XUI_DOC_EDIT_BOLD,XUI_DOC_EDIT_ITALIC,100,101,102};
    int i, result;
#define OK(expr) do { result = (expr); if (result != XUI_OK) { fprintf(stderr, "line %d: %s returned %d\n", __LINE__, #expr, result); return result; } } while (0)
    OK(xuiWidgetCreate(d->context, &d->root)); OK(xuiSetRootWidget(d->context, d->root));
    xuiWidgetSetRect(d->root, (xui_rect_t){0,0,W,H}); xuiWidgetSetCacheRenderCallback(d->root, root_draw, d);
    OK(label(d, "Unified Document / Native Rich Text + Markdown", (xui_rect_t){18,8,W-36,28}, &caption));
    OK(label(d, "Rich text editor", (xui_rect_t){20,65,380,24}, &caption));
    OK(label(d, "Markdown editor", (xui_rect_t){420,65,400,24}, &caption));
    OK(label(d, "Shared Markdown preview", (xui_rect_t){842,65,410,24}, &caption));
    OK(label(d, "Ctrl+Z undo | Ctrl+B/I format | Ctrl+wheel zoom | Source and preview share one document.", (xui_rect_t){18,H-36,W-36,26}, &d->status));
    OK(xuiDocumentCreate(NULL, &d->rich_doc)); md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    OK(xuiDocumentCreate(&md, &d->md_doc));
    {
        const char* source = d->verify == 3 ? md_image_test_text : md_text;
        OK(xuiDocumentLoadMarkdown(d->md_doc, source, strlen(source)));
    }
    editor.iSize = sizeof(editor); editor.tView.iSize = sizeof(editor.tView); editor.tView.pDocument = d->rich_doc;
    editor.tView.tRenderer.iSize = sizeof(editor.tView.tRenderer); editor.tView.tRenderer.tFonts = (xui_doc_font_set_t){d->font,d->bold,d->italic,d->bold,d->mono};
    editor.onError = show_error; editor.pUser = d;
    OK(xuiDocumentEditorCreate(d->context, &editor, &d->rich));
    OK(xuiDocumentEditorInsertText(d->rich, "A new document kernel\n\nRich paragraphs and Markdown share storage, transactions, snapshots and undo.\n\nTry selecting text, applying bold, pasting paragraphs, or typing Chinese.\n\n", strlen("A new document kernel\n\nRich paragraphs and Markdown share storage, transactions, snapshots and undo.\n\nTry selecting text, applying bold, pasting paragraphs, or typing Chinese.\n\n")));
    { xui_document_transaction t; uint64_t table;
      OK(xuiDocumentBeginTransaction(d->rich_doc, NULL, &t));
      result = xuiDocumentTxnInsertTable(t, 1, XUI_DOCUMENT_APPEND, 3, 2, 1, &table);
      if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
      xuiDocumentTxnRelease(t); if (result != XUI_OK) return result; }
    xuiWidgetSetRect(d->rich, (xui_rect_t){24,106,368,H-166}); OK(xuiWidgetAddChild(d->root, d->rich));
    OK(xuiDocumentViewSetScroll(d->rich, 0, 0));
    editor.tView.pDocument = d->md_doc; editor.iMode = XUI_DOC_SOURCE_TEXT;
    if (d->verify == 2) editor.iAsyncSourceThresholdBytes = 1;
    OK(xuiDocumentEditorCreate(d->context, &editor, &d->markdown));
    xuiWidgetSetRect(d->markdown, (xui_rect_t){424,106,390,H-166}); OK(xuiWidgetAddChild(d->root, d->markdown));
    view = editor.tView; view.pDocument = d->md_doc;
    OK(xuiDocumentViewCreate(d->context, &view, &d->preview));
    xuiWidgetSetRect(d->preview, (xui_rect_t){846,106,W-870,H-166}); OK(xuiWidgetAddChild(d->root, d->preview));
    for (i = 0; i < 7; i++) {
        xui_widget button; xui_button_desc_t desc = {0}; desc.iSize = sizeof(desc); desc.sText = labels[i]; desc.pFont = d->font;
        OK(xuiButtonCreate(d->context, &button, &desc)); d->actions[i] = (action){d,commands[i]};
        xuiButtonSetClick(button, click, &d->actions[i]); xuiWidgetSetRect(button, (xui_rect_t){20+i*96,37,88,26}); xuiWidgetSetFocusable(button, 0);
        OK(xuiWidgetAddChild(d->root, button));
    }
    if (!d->verify) OK(xuiSetFocusWidget(d->context, d->markdown));
    return XUI_OK;
#undef OK
}
static int resources(demo* d)
{
    xui_surface_desc_t surface = {0}; xui_resource_desc_t image_resource = {0};
    xui_surface image = NULL; xui_resource decoded = NULL; int r;
    d->proxy = xuiProxyXge(); if (xuiCreate(&d->context) != XUI_OK || xuiSetProxy(d->context, &d->proxy) != XUI_OK) return XUI_ERROR;
    if (d->proxy.fontLoadFile(&d->proxy, &d->font, "C:\\Windows\\Fonts\\segoeui.ttf", 16, XUI_FONT_FORMAT_TTF) != XUI_OK) return XUI_ERROR;
    d->proxy.fontLoadFile(&d->proxy, &d->bold, "C:\\Windows\\Fonts\\segoeuib.ttf", 16, XUI_FONT_FORMAT_TTF);
    d->proxy.fontLoadFile(&d->proxy, &d->italic, "C:\\Windows\\Fonts\\segoeuii.ttf", 16, XUI_FONT_FORMAT_TTF);
    d->proxy.fontLoadFile(&d->proxy, &d->mono, "C:\\Windows\\Fonts\\consola.ttf", 15, XUI_FONT_FORMAT_TTF);
    xuiSetDefaultFont(d->context, d->font); xuiInputViewport(d->context, W, H);
    surface.iKind = XUI_SURFACE_KIND_TEXTURE; surface.iFormat = XUI_SURFACE_FORMAT_RGBA8; surface.iWidth = W; surface.iHeight = H;
    surface.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
    r = d->proxy.surfaceCreate(&d->proxy, &d->target, &surface);
    if (r != XUI_OK) return r;
    surface.iWidth = 96; surface.iHeight = 56;
    r = d->proxy.surfaceCreate(&d->proxy, &image, &surface);
    if (r != XUI_OK) return r;
    r = d->proxy.surfaceClear(&d->proxy, image, XUI_COLOR_RGBA(42,190,126,255));
    if (r != XUI_OK) { d->proxy.surfaceDestroy(&d->proxy, image); return r; }
    image_resource.iSize = sizeof(image_resource); image_resource.sName = "demo.surface";
    image_resource.iKind = XUI_RESOURCE_SURFACE; image_resource.pHandle = image;
    image_resource.pUser = d; image_resource.onDestroy = image_destroy;
    r = xuiResourceSet(d->context, NULL, &image_resource);
    if (r != XUI_OK) { d->proxy.surfaceDestroy(&d->proxy, image); return r; }
    if (d->verify) {
        r = xuiDocumentImageResourceLoadFile(d->context, "demo.decoded.probe", "res/msgbox_info.png", NULL, &decoded);
        if (r != XUI_OK) return r;
        r = d->proxy.surfaceGetDesc(&d->proxy, (xui_surface)xuiResourceGetHandle(decoded), &surface);
        if (r != XUI_OK || surface.iWidth <= 0 || surface.iHeight <= 0) return XUI_ERROR_BACKEND_FAILED;
        r = xuiResourceRemove(decoded);
        if (r != XUI_OK) return r;
        {
            xui_doc_image_request request = NULL; unsigned attempt;
            r = xuiDocumentImageResourceLoadFileAsync(d->context, "demo.async.probe", "res/msgbox_info.png", NULL, &request);
            if (r != XUI_OK) return r;
            for (attempt = 0; attempt < 5000; attempt++) {
                r = xuiDocumentImageResourcePoll(request, &decoded);
                if (r != XUI_DOC_ERROR_BUSY) break;
#ifdef _WIN32
                Sleep(1);
#endif
            }
            if (r == XUI_OK && decoded) {
                r = d->proxy.surfaceGetDesc(&d->proxy, (xui_surface)xuiResourceGetHandle(decoded), &surface);
                if (r == XUI_OK && surface.iWidth > 0 && surface.iHeight > 0)
                    r = xuiResourceRemove(decoded);
                else r = XUI_ERROR_BACKEND_FAILED;
            } else if (r == XUI_OK) r = XUI_ERROR_BACKEND_FAILED;
            xuiDocumentImageResourceRelease(request);
            if (r != XUI_OK) return r;
        }
    }
    return setup(d);
}
static int capture_async(demo* d, const char* path, uint64_t* source, uint64_t* preview)
{
    unsigned char* pixels = malloc((size_t)W * H * 4); int x, y, result;
    if (!pixels) return XUI_ERROR_OUT_OF_MEMORY;
    *source = *preview = UINT64_C(14695981039346656037);
    result = d->proxy.surfaceReadRGBA(&d->proxy, d->target, pixels, W * 4);
    if (result == XUI_OK) for (y = 110; y < H - 80; y++) for (x = 424; x < W - 30; x++) {
        uint64_t* hash = x < 814 ? source : x >= 846 ? preview : NULL;
        if (hash) { *hash ^= pixels[((size_t)y * W + x) * 4]; *hash *= UINT64_C(1099511628211); }
    }
    if (result == XUI_OK && path) result = xgeImageSavePNG(path, W, H, pixels, W * 4);
    free(pixels); return result;
}
static int verify_async(demo* d)
{
    static const char prefix[] = "Pending source input; preview updates after publication.\n\n";
    uint64_t source_hash = 0, preview_hash = 0; int result = XUI_OK;
    if (d->frames == 1) {
        result = capture_async(d, NULL, &d->async_source_pixels, &d->async_preview_pixels);
        d->async_revision = xuiDocumentGetRevision(d->md_doc);
        if (result == XUI_OK) result = xuiDocumentEditorInsertText(d->markdown, prefix, sizeof(prefix) - 1);
    } else if (d->frames == 2) {
        xui_doc_prepare_info_t info = {0}; xui_doc_range_t range; info.iSize = sizeof(info);
        result = xuiDocumentEditorGetPendingInput(d->markdown, &info);
        if (result == XUI_OK) result = xuiDocumentViewGetSelection(d->markdown, &range);
        if (result == XUI_OK && (xuiDocumentGetRevision(d->md_doc) != d->async_revision || !range.tCaret.iInputGeneration)) result = XUI_ERROR_INVALID_STATE;
        if (result == XUI_OK) result = capture_async(d, "artifacts/xui-document-rebuild/native-async-pending.png", &source_hash, &preview_hash);
        if (result == XUI_OK && (source_hash == d->async_source_pixels || preview_hash != d->async_preview_pixels)) result = XUI_ERROR_BACKEND_FAILED;
        d->async_pending_pixels = source_hash;
    } else if (d->frames > 2 && !xuiDocumentHasPrepare(d->md_doc)) {
        xui_document_snapshot snapshot = NULL; char buffer[2048]; uint64_t bytes;
        result = capture_async(d, "artifacts/xui-document-rebuild/native-async-committed.png", &source_hash, &preview_hash);
        if (result == XUI_OK && (source_hash != d->async_pending_pixels || preview_hash == d->async_preview_pixels ||
            xuiDocumentGetRevision(d->md_doc) != d->async_revision + 1)) result = XUI_ERROR_BACKEND_FAILED;
        if (result == XUI_OK) result = xuiDocumentAcquireSnapshot(d->md_doc, &snapshot);
        if (result == XUI_OK) result = xuiDocumentSnapshotCopySource(snapshot, buffer, sizeof(buffer), &bytes);
        if (result == XUI_OK && (strncmp(buffer, prefix, sizeof(prefix) - 1) || strcmp(buffer + sizeof(prefix) - 1, md_text))) result = XUI_ERROR_INVALID_STATE;
        xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
        if (result == XUI_OK) result = xuiDocumentEditorExecute(d->markdown, XUI_DOC_EDIT_UNDO);
        if (result == XUI_OK) result = xuiDocumentAcquireSnapshot(d->md_doc, &snapshot);
        if (result == XUI_OK) result = xuiDocumentSnapshotCopySource(snapshot, buffer, sizeof(buffer), &bytes);
        if (result == XUI_OK && strcmp(buffer, md_text)) result = XUI_ERROR_INVALID_STATE;
        xuiDocumentSnapshotRelease(snapshot); d->result = result; xgeQuit();
    } else if (d->frames > 600) result = XUI_DOC_ERROR_BUSY;
    return result;
}
static int frame(void* user)
{
    demo* d = user; xui_rect_i_t damage = {0,0,W,H}; int r;
#ifdef _WIN32
    if (d->verify && !d->frames) ShowWindow((HWND)xgePlatformNativeHandle(), SW_HIDE);
#endif
    if (!d->target) { r = resources(d); if (r != XUI_OK) { fprintf(stderr, "Document resources failed: %d\n", r); goto failed; } }
    r = xgeBegin(); if (r != XGE_OK) goto failed;
    if (!d->verify) {
        xui_widget focused = xuiGetFocusWidget(d->context);
        if (focused == d->rich || focused == d->markdown) d->active_editor = focused;
        xuiProxyXgePumpInputRect(d->context, (xui_rect_t){0,0,W,H}); xuiDispatchPendingEvents(d->context);
    }
    r = xuiLayout(d->context); if (r != XUI_OK) goto failed;
    r = xuiUpdate(d->context, .016f); if (r != XUI_OK) goto failed;
    d->proxy.surfaceClear(&d->proxy, d->target, XUI_COLOR_WHITE);
    r = xuiRender(d->context, d->target, &damage, 1); if (r != XUI_OK) goto failed;
    if (!d->verify) { xgeClear(XUI_COLOR_WHITE); d->proxy.surfaceDraw(&d->proxy, d->target, (xui_rect_t){0,0,W,H}, (xui_rect_t){0,0,W,H}, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE); }
    r = xgeEnd();
    if (r != XGE_OK) goto failed;
    d->frames++;
    if (d->verify == 2) { r = verify_async(d); if (r != XUI_OK) goto failed; return r; }
    if (d->verify == 3 && d->frames == 3) {
        unsigned char* pixels = malloc((size_t)W * H * 4);
        unsigned background = 0, border = 0; int x, y;
        if (!pixels) r = XUI_ERROR_OUT_OF_MEMORY;
        else {
            r = d->proxy.surfaceReadRGBA(&d->proxy, d->target, pixels, W * 4);
            if (r == XUI_OK) {
                for (y = 110; y < H - 80; y++) for (x = 846; x < W - 30; x++) {
                    const unsigned char* p = pixels + ((size_t)y * W + x) * 4;
                    if (p[3] > 200 && p[0] == 238 && p[1] == 241 && p[2] == 245) background++;
                    if (p[3] > 200 && p[0] == 170 && p[1] == 180 && p[2] == 192) border++;
                }
                if (background < 200 || border < 10) {
                    fprintf(stderr, "Missing image placeholder pixels: background=%u border=%u\n",
                        background, border);
                    r = XUI_ERROR_BACKEND_FAILED;
                }
            }
            if (r == XUI_OK) r = xgeImageSavePNG(
                "artifacts/xui-document-rebuild/native-image-placeholder.png",
                W, H, pixels, W * 4);
            free(pixels);
        }
        d->result = r; xgeQuit(); return r;
    }
    if ((d->frames == 3 || d->frames == 6) && d->verify) {
        unsigned char* pixels = malloc((size_t)W*H*4); unsigned counts[3] = {0}, image_pixels = 0; int x, y; uint64_t hash = UINT64_C(14695981039346656037);
        if (!pixels) r = XUI_ERROR_OUT_OF_MEMORY;
        else {
            if (r == XUI_OK) r = d->proxy.surfaceReadRGBA(&d->proxy, d->target, pixels, W*4);
            if (r == XUI_OK) {
                for (y = 110; y < H-80; y++) for (x = 24; x < W-30; x++) {
                    unsigned char* p = pixels + ((size_t)y*W+x)*4;
                    if (p[3] > 200 && p[0] < 140 && p[1] < 140 && p[2] < 140) counts[x < 410 ? 0 : x < 836 ? 1 : 2]++;
                    if (x >= 846 && p[0] < 80 && p[1] > 150 && p[2] > 90 && p[2] < 170) image_pixels++;
                    if (x >= 424 && x < 814) { hash ^= p[0]; hash *= UINT64_C(1099511628211); }
                }
                if (counts[0] < 200 || counts[1] < 200 || counts[2] < 200) { fprintf(stderr, "Missing text pixels: %u, %u, %u\n", counts[0], counts[1], counts[2]); r = XUI_ERROR_BACKEND_FAILED; }
                if (image_pixels < 1000) { fprintf(stderr, "Missing native image pixels: %u\n", image_pixels); r = XUI_ERROR_BACKEND_FAILED; }
            }
            if (r == XUI_OK) r = xgeImageSavePNG(d->frames == 3 ? "artifacts/xui-document-rebuild/native-smoke.png" :
                "artifacts/xui-document-rebuild/native-live-smoke.png", W, H, pixels, W*4);
            free(pixels);
        }
        if (r == XUI_OK && d->frames == 3) {
            d->source_pixels = hash; r = xuiDocumentViewSetMode(d->markdown, XUI_DOC_LIVE_MARKDOWN);
        } else if (r == XUI_OK && hash == d->source_pixels) { fprintf(stderr, "Live projection did not change the source editor pixels.\n"); r = XUI_ERROR_BACKEND_FAILED; }
        if (r != XUI_OK || d->frames == 6) { d->result = r; xgeQuit(); }
    }
    return r;
failed:
    d->result = r; xgeQuit(); return r;
}
int main(int argc, char** argv)
{
    demo d = {0}; xge_desc_t desc = {0}; int r;
    d.verify = argc > 1 && !strcmp(argv[1], "--verify");
    if (argc > 1 && !strcmp(argv[1], "--verify-async")) d.verify = 2;
    if (argc > 1 && !strcmp(argv[1], "--verify-image-placeholder")) d.verify = 3;
    desc.iWidth = W; desc.iHeight = H; desc.sTitle = "XUI Unified Document"; desc.iFlags = d.verify ? XGE_INIT_OFFSCREEN : XGE_INIT_VSYNC;
    desc.iRunMode = XGE_RUN_GAME_LOOP; desc.iTargetFPS = 60;
    if (xgeInit(&desc) != XGE_OK) return 1;
    r = xgeRun(frame, &d); if (r == XUI_OK) r = d.result;
    xuiDestroy(d.context); xuiDocumentRelease(d.rich_doc); xuiDocumentRelease(d.md_doc);
    if (d.proxy.surfaceDestroy) d.proxy.surfaceDestroy(&d.proxy, d.target);
    if (d.proxy.fontDestroy) { d.proxy.fontDestroy(&d.proxy, d.mono); d.proxy.fontDestroy(&d.proxy, d.italic); d.proxy.fontDestroy(&d.proxy, d.bold); d.proxy.fontDestroy(&d.proxy, d.font); }
    xgeUnit();
    if (r != XUI_OK) fprintf(stderr, "Document demo failed: %d\n", r);
    else if (d.verify == 2) puts("Native XGE async Document: source pixels change before publication, preview pixels change only after publication, shared Undo passed.");
    else if (d.verify == 3) puts("Native XGE missing-image placeholder: background and border pixels passed.");
    else if (d.verify) puts("Native XGE Document rendering passed; captured native-smoke.png and native-live-smoke.png.");
    return r == XUI_OK ? 0 : 1;
}
