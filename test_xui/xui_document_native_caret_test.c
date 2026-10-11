/* Reuse the exact accepted native setup; exercise different layout contents. */
#define main accepted_scale_main
#include "xui_document_native_scale_test.c"
#undef main
#include <math.h>

static xui_font large_font;
static unsigned complex_checks;
static unsigned drift_checks;
static double before_x, before_y;

static void complex_caret(xui_document d, xui_widget editor, unsigned mode,
    size_t expected, const char* label, const char* stage)
{
    xui_doc_range_t selection;
    xui_document_snapshot snapshot;
    doc_view_data* view;
    xui_doc_rect_t caret;
    xui_rect_t content;
    double sx, sy;
    uint64_t offset;
    int mapping, outside;
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentPositionToSource(snapshot, &selection.tCaret, &offset, &mapping) == XUI_OK);
    CHECK(offset == expected);
    CHECK(xuiDocumentViewGetScroll(editor, &sx, &sy) == XUI_OK);
    view = (doc_view_data*)xuiWidgetGetTypeData(editor);
    CHECK(view && view->renderer);
    CHECK(xuiDocumentRendererGetCaretRect(view->renderer, &selection.tCaret, &caret) == XUI_OK);
    content = xuiWidgetGetContentRect(editor);
    CHECK(content.fW == 640 && content.fH == 400);
    outside = caret.x-sx < -.5 || caret.x-sx+caret.width > content.fW+.5 ||
        caret.y-sy < -.5 || caret.y-sy+caret.height > content.fH+.5;
    caret_outside += (unsigned)outside;
    if (strstr(stage, "before-draw")) { before_x = caret.x-sx; before_y = caret.y-sy; }
    else {
        double dx = caret.x-sx-before_x, dy = caret.y-sy-before_y;
        int drift = fabs(dx) > .5 || fabs(dy) > .5;
        drift_checks += (unsigned)drift;
        printf("Caret frame stability %s mode=%u stage=%s dx=%.2f dy=%.2f drift=%d\n", label, mode, stage, dx, dy, drift);
    }
    complex_checks++;
    printf("Complex caret %s mode=%u stage=%s source=%llu screen=%.2f,%.2f size=%.2f,%.2f viewport=%.2f,%.2f outside=%d\n",
        label, mode, stage, (unsigned long long)offset, caret.x-sx, caret.y-sy,
        caret.width, caret.height, (double)content.fW, (double)content.fH, outside);
    xuiDocumentSnapshotRelease(snapshot);
}

static char* complex_source(unsigned kind, size_t* bytes, size_t* offset)
{
    const char* phrase = "alpha αβγ שלום العربية **bold** _italic_ `code` ";
    const char* group = "## A heading with **bold**\n\n"
        "> quoted paragraph\n> second quoted line\n\n"
        "- list item\n  - nested item\n\n"
        "```c\nint a;\nint b;\nint c;\nint d;\nint e;\nint f;\n```\n\n"
        "| first | second |\n| :--- | ---: |\n| alpha | beta |\n| gamma | delta |\n\n";
    size_t plen = strlen(phrase), glen = strlen(group);
    size_t capacity = 1024*1024;
    char* source = malloc(capacity);
    CHECK(source);
    *bytes = *offset = 0;
    if (!kind) {
        for (unsigned block = 0; block < 256; block++) {
            CHECK(*bytes + glen + 32*plen + 32 < capacity);
            memcpy(source + *bytes, group, glen); *bytes += glen;
            memcpy(source + *bytes, "plain target ", 13); *bytes += 13;
            if (block == 128) *offset = *bytes;
            for (unsigned word = 0; word < 32; word++) {
                memcpy(source + *bytes, phrase, plen); *bytes += plen;
            }
            source[(*bytes)++] = '\n'; source[(*bytes)++] = '\n';
        }
    } else {
        /* One paragraph with a long mixed-direction UTF-8 line. */
        memcpy(source, "long paragraph ", 15); *bytes = 15;
        for (unsigned word = 0; word < 2048; word++) {
            if (word == 1024) *offset = *bytes;
            CHECK(*bytes + plen + 1 < capacity);
            memcpy(source + *bytes, phrase, plen); *bytes += plen;
        }
    }
    source[*bytes] = 0;
    CHECK(*offset > 0 && *offset < *bytes);
    return source;
}

static void complex_case(unsigned kind, unsigned mode)
{
    const char* label = kind ? "long-unicode-paragraph" : "mixed-blocks";
    size_t bytes, offset;
    char* source = complex_source(kind, &bytes, &offset);
    char* expected = malloc(bytes + 17);
    xui_document d = markdown(source, bytes);
    xui_widget editor;
    xui_doc_editor_desc_t desc = {0};
    xui_doc_range_t range = {0};
    xui_document_snapshot snapshot;
    int mapping;
    CHECK(expected);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = d; desc.iMode = mode;
    desc.tView.tRenderer.iSize = sizeof(desc.tView.tRenderer);
    desc.tView.tRenderer.tFonts = (xui_doc_font_set_t){font,large_font,font,large_font,font};
    desc.tView.tRenderer.iLayoutCacheBudgetBytes = 64*1024;
    desc.iAsyncSourceThresholdBytes = 1;
    desc.iUndoGroupTimeoutMs = 60000; desc.onError = error;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK);
    CHECK(xuiInputViewport(context, 640, 400) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0,0,640,400}) == XUI_OK);
    CHECK(xuiDocumentViewSetZoom(editor, kind ? 1.25f : 1.f) == XUI_OK);
    draw_frame();
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    if (mode == XUI_DOC_VISUAL) {
        CHECK(xuiDocumentSourceToPositionEx(snapshot, offset, XUI_DOC_AFTER, &range.tAnchor, &mapping) == XUI_OK);
        CHECK(range.tAnchor.iKind == XUI_DOC_POSITION_TEXT);
    } else {
        range.tAnchor.iSize = sizeof(range.tAnchor);
        range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d);
        range.tAnchor.iRevision = xuiDocumentGetRevision(d);
        range.tAnchor.iKind = XUI_DOC_POSITION_SOURCE;
        range.tAnchor.iNodeId = 1; range.tAnchor.iOffset = offset;
        range.tAnchor.iAffinity = XUI_DOC_AFTER;
    }
    range.tCaret = range.tAnchor;
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    draw_frame();
    for (unsigned i = 0; i < 16; i++) {
        CHECK(xuiDocumentEditorInsertText(editor, "X", 1) == XUI_OK);
        draw_frame();
    }
    flush(editor);
    memcpy(expected, source, offset); memset(expected + offset, 'X', 16);
    memcpy(expected + offset + 16, source + offset, bytes - offset + 1);
    exact_source(d, expected, bytes + 16);
    complex_caret(d, editor, mode, offset + 16, label, "publish-before-draw");
    draw_frame();
    complex_caret(d, editor, mode, offset + 16, label, "publish-after-draw");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    draw_frame(); exact_source(d, source, bytes);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    complex_caret(d, editor, mode, offset + 16, label, "redo-before-draw");
    draw_frame(); exact_source(d, expected, bytes + 16);
    complex_caret(d, editor, mode, offset + 16, label, "redo-after-draw");
    {
        unsigned char* pixels = malloc(W*H*4);
        char path[1024];
        CHECK(pixels);
        CHECK(proxy.surfaceReadRGBA(&proxy, target, pixels, W*4) == XUI_OK);
        snprintf(path, sizeof(path), "%s/%s-mode-%u.png", destination, label, mode);
        CHECK(xgeImageSavePNG(path, W, H, pixels, W*4) == XGE_OK);
        free(pixels);
    }
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    free(expected); free(source);
}

static int complex_frame(void* user)
{
    xui_surface_desc_t surface = {0}; (void)user;
    printf("Complex actual XGE GL: %s / %s / %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));
    proxy = xuiProxyXge();
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy) == XUI_OK);
    CHECK(proxy.fontLoadFile(&proxy, &font, font_path, 18, 0) == XUI_OK);
    CHECK(proxy.fontLoadFile(&proxy, &large_font, font_path, 32, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK && xuiInputViewport(context, W, H) == XUI_OK);
    surface.iWidth = W; surface.iHeight = H;
    surface.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
    CHECK(proxy.surfaceCreate(&proxy, &target, &surface) == XUI_OK);
    for (unsigned kind = 0; kind < 2; kind++)
        for (unsigned mode = XUI_DOC_VISUAL; mode <= XUI_DOC_LIVE_MARKDOWN; mode++)
            complex_case(kind, mode);
    proxy.surfaceDestroy(&proxy, target); xuiDestroy(context);
    proxy.fontDestroy(&proxy, large_font); proxy.fontDestroy(&proxy, font);
    xgeQuit(); return XGE_OK;
}

int main(int argc, char** argv)
{
    xge_desc_t desc = {0}; CHECK(argc == 3);
    destination = argv[1]; font_path = argv[2];
    CHECK(xgeGetBuildFeatures() == XGE_FEATURES);
    desc.iWidth = W; desc.iHeight = H;
    desc.sTitle = "Document complex caret acceptance";
    desc.iFlags = XGE_INIT_OFFSCREEN; desc.iRunMode = XGE_RUN_GAME_LOOP;
    CHECK(xgeInit(&desc) == XGE_OK && xgeRun(complex_frame, NULL) == XGE_OK);
    xgeUnit();
    printf("Complex caret acceptance: %u checks, %u outside; six cases, exact source and Undo/Redo\n", complex_checks, caret_outside);
    printf("Caret frame stability: %u drifting checks\n", drift_checks);
    return complex_checks == 24 && !caret_outside && !drift_checks ? 0 : 1;
}
