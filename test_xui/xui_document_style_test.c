#include "../xui_document_ui.h"
#include "xui_test_proxy.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(e) do { if (!(e)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #e); exit(1); } } while (0)
static uint32_t seen[4096];
static size_t seen_count;
static xui_surface expected_image_surface;
static size_t image_draw_count;
static xui_rect_t last_image_source;
static uint32_t image_alt_flags;
static xui_draw_text_proc base_text;
static xui_draw_text_spans_proc base_spans;
static int (*base_fill)(xui_proxy, xui_draw_context, xui_rect_t, uint32_t);
static int (*base_stroke)(xui_proxy, xui_draw_context, xui_rect_t, float, uint32_t);
static int (*base_surface)(xui_proxy, xui_draw_context, xui_surface, xui_rect_t, xui_rect_t, uint32_t, uint32_t);
static void record(uint32_t color) { if (seen_count < sizeof(seen) / sizeof(seen[0])) seen[seen_count++] = color; }
static int has(uint32_t color)
{
    size_t i; for (i = 0; i < seen_count; i++) if (seen[i] == color) return 1;
    return 0;
}
static size_t count_color(uint32_t color)
{
    size_t i, count = 0;
    for (i = 0; i < seen_count; i++) count += seen[i] == color;
    return count;
}
static int text_draw(xui_proxy p, xui_draw_context d, const xui_text_item_t* pTextItem, xui_rect_t r, uint32_t c, uint32_t flags)
{
    const char* s = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;

    record(c);
    if (!strcmp(s, "missing")) image_alt_flags = flags;
    return base_text(p, d, pTextItem, r, c, flags);
}
static int spans_draw(xui_proxy p, xui_draw_context d, const xui_text_item_t* pTextItem, xui_rect_t r, uint32_t c, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{

    int i;
    record(c);
    for (i = 0; i < count; i++) record(spans[i].iColor);
    return base_spans(p, d, pTextItem, r, c, flags, spans, count);
}
static int fill_draw(xui_proxy p, xui_draw_context d, xui_rect_t r, uint32_t c)
{ record(c); return base_fill(p, d, r, c); }
static int stroke_draw(xui_proxy p, xui_draw_context d, xui_rect_t r, float w, uint32_t c)
{ record(c); return base_stroke(p, d, r, w, c); }
static int surface_draw(xui_proxy p, xui_draw_context d, xui_surface surface,
    xui_rect_t source, xui_rect_t destination, uint32_t color, uint32_t flags)
{
    if (surface == expected_image_surface) { image_draw_count++; last_image_source = source; }
    return base_surface(p, d, surface, source, destination, color, flags);
}
static void image_destroy(xui_context context, void* handle, void* user)
{
    xui_test_proxy_state_t* proxy = user; (void)context;
    proxy->tProxy.surfaceDestroy(&proxy->tProxy, (xui_surface)handle);
}
static xui_style_property_t color_property(const char* name, uint32_t color)
{
    xui_style_property_t p = {0}; p.iSize = sizeof(p); p.sName = name;
    p.tValue.iSize = sizeof(p.tValue); p.tValue.iType = XUI_STYLE_VALUE_COLOR;
    p.tValue.iColor = color; return p;
}
static void render(xui_context ctx, xui_surface target)
{
    xui_rect_i_t damage = {0, 0, 640, 480}; seen_count = image_draw_count = 0;
    CHECK(xuiRender(ctx, target, &damage, 1) == XUI_OK);
}
static uint64_t insert(xui_document_transaction t, uint64_t parent, uint32_t kind,
    const char* text, uint32_t explicit_color)
{
    xui_doc_node_desc_t desc = {0}; uint64_t id = 0;
    desc.iSize = sizeof(desc); desc.iKind = kind; desc.sText = text;
    desc.iTextBytes = text ? strlen(text) : 0; desc.tAttributes.iTextColor = explicit_color;
    CHECK(xuiDocumentTxnInsertNode(t, parent, XUI_DOCUMENT_APPEND, &desc, &id) == XUI_OK);
    return id;
}
static xui_doc_position_t position(xui_document d, uint64_t id, uint64_t offset)
{
    xui_doc_position_t p = {0}; p.iSize = sizeof(p); p.iKind = XUI_DOC_POSITION_TEXT;
    p.iDocumentId = xuiDocumentGetIdentity(d); p.iRevision = xuiDocumentGetRevision(d);
    p.iNodeId = id; p.iOffset = offset; return p;
}
int main(void)
{
    static const char* const names[] = {
        "document.text.color", "document.background.color", "document.border.color",
        "document.code.background_color", "document.highlight.color", "document.link.color",
        "document.selection.color", "document.caret.color",
        "document.find.result_color", "document.find.active_color",
        "document.border.focus_color", "document.quote.border_color",
        "document.rule.color", "document.paragraph.background_color",
        "document.table.border_color", "document.table.header_color",
        "document.table.cell_color", "document.image.placeholder_color",
        "document.image.border_color", "document.image.text_color"
    };
    enum { TEXT = 0, BACKGROUND, BORDER, CODE, HIGHLIGHT, LINK, SELECTION, CARET,
        FIND_RESULT, FIND_ACTIVE, FOCUS_BORDER, QUOTE_BORDER, RULE_COLOR,
        PARAGRAPH_BACKGROUND, TABLE_BORDER, TABLE_HEADER, TABLE_CELL,
        IMAGE_PLACEHOLDER, IMAGE_BORDER, IMAGE_TEXT };
    const uint32_t explicit_color = 0xc91347ffu, base_color = 0x293547ffu;
    const uint32_t base_background = 0xf3f5f7ffu;
    xui_test_proxy_state_t proxy; xui_context ctx; xui_font font; xui_surface target;
    xui_resource image_resource = NULL; xui_surface image_surface = NULL;
    xui_document d, md, other; xui_document_transaction t; xui_widget editor, view;
    xui_doc_desc_t md_desc = {0};
    xui_doc_editor_desc_t editor_desc = {0}; xui_doc_view_desc_t view_desc = {0};
    xui_doc_range_t selection, found; xui_doc_position_t replacement_caret;
    xui_doc_rect_t before, after;
    xui_style_property_t properties[20], transparent; uint64_t find_count;
    xui_doc_renderer_stats_t shape_before = {0}, shape_after = {0};
    xui_style_desc_t type_style = {0}; uint64_t paragraph, body, image_id, version;
    int exact_before, exact_after, active; size_t i;
    xuiTestProxyInit(&proxy);
    base_text = proxy.tProxy.drawText; proxy.tProxy.drawText = text_draw;
    base_spans = proxy.tProxy.drawTextSpans; proxy.tProxy.drawTextSpans = spans_draw;
    base_fill = proxy.tProxy.drawRectFill; proxy.tProxy.drawRectFill = fill_draw;
    base_stroke = proxy.tProxy.drawRectStroke; proxy.tProxy.drawRectStroke = stroke_draw;
    base_surface = proxy.tProxy.drawSurface; proxy.tProxy.drawSurface = surface_draw;
    CHECK(xuiCreate(&ctx) == XUI_OK && xuiSetProxy(ctx, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(ctx, font) == XUI_OK && xuiInputViewport(ctx, 640, 480) == XUI_OK);
    CHECK(xuiTestSurfaceCreate(&proxy, &target, 640, 480, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    paragraph = insert(t, 1, XUI_DOC_PARAGRAPH, NULL, 0);
    body = insert(t, paragraph, XUI_DOC_TEXT, "default default ", 0);
    insert(t, paragraph, XUI_DOC_TEXT, "explicit", explicit_color);
    insert(t, 1, XUI_DOC_RULE, NULL, 0);
    {
        xui_doc_node_desc_t desc = {0}; uint64_t quote, table, row, cell;
        quote = insert(t, 1, XUI_DOC_QUOTE, NULL, 0);
        paragraph = insert(t, quote, XUI_DOC_PARAGRAPH, NULL, 0);
        insert(t, paragraph, XUI_DOC_TEXT, "quote", 0);
        table = insert(t, 1, XUI_DOC_TABLE, NULL, 0);
        row = insert(t, table, XUI_DOC_ROW, NULL, 0);
        desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_CELL;
        desc.tAttributes.iFlags = XUI_DOC_HEADER;
        CHECK(xuiDocumentTxnInsertNode(t, row, XUI_DOCUMENT_APPEND, &desc, &cell) == XUI_OK);
        paragraph = insert(t, cell, XUI_DOC_PARAGRAPH, NULL, 0);
        insert(t, paragraph, XUI_DOC_TEXT, "head", 0);
        cell = insert(t, row, XUI_DOC_CELL, NULL, 0);
        paragraph = insert(t, cell, XUI_DOC_PARAGRAPH, NULL, 0);
        insert(t, paragraph, XUI_DOC_TEXT, "body", 0);
        memset(&desc, 0, sizeof(desc));
        desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_IMAGE;
        desc.sText = "missing"; desc.iTextBytes = 7;
        desc.sResource = "missing.png";
        desc.tAttributes.fWidth = 80; desc.tAttributes.fHeight = 30;
        CHECK(xuiDocumentTxnInsertNode(t, 1, XUI_DOCUMENT_APPEND, &desc, &image_id) == XUI_OK);
    }
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    editor_desc.iSize = sizeof(editor_desc); editor_desc.tView.iSize = sizeof(editor_desc.tView);
    editor_desc.tView.pDocument = d; editor_desc.tView.iBackgroundColor = base_background;
    editor_desc.tView.tRenderer.iSize = sizeof(editor_desc.tView.tRenderer);
    editor_desc.tView.tRenderer.iTextColor = base_color;
    CHECK(xuiDocumentEditorCreate(ctx, &editor_desc, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(ctx, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 480}) == XUI_OK && xuiLayout(ctx) == XUI_OK);
    render(ctx, target);
    CHECK(has(base_color) && has(base_background) && has(explicit_color) &&
        has(XUI_COLOR_RGBA(238, 241, 245, 255)) &&
        has(XUI_COLOR_RGBA(170, 180, 192, 255)) &&
        has(XUI_COLOR_RGBA(90, 100, 112, 255)) &&
        (image_alt_flags & (XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP)) ==
            (XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP));
    CHECK(xuiDocumentViewGetContentSize(editor, &before, &exact_before) == XUI_OK);
    version = xuiDocumentGetRevision(d);
    for (i = 0; i < 20; i++) {
        CHECK(xuiStyleFindProperty(ctx, names[i]) != 0);
        properties[i] = color_property(names[i], 0x215060ffu + (uint32_t)(i << 16));
    }
    CHECK(xuiStyleSetDefault(ctx, properties, 20) == XUI_OK);
    render(ctx, target);
    CHECK(has(properties[TEXT].tValue.iColor) && has(properties[BACKGROUND].tValue.iColor) &&
        has(properties[BORDER].tValue.iColor) &&
        has(explicit_color) &&
        has(properties[QUOTE_BORDER].tValue.iColor) &&
        has(properties[RULE_COLOR].tValue.iColor) &&
        has(properties[PARAGRAPH_BACKGROUND].tValue.iColor) &&
        has(properties[TABLE_BORDER].tValue.iColor) &&
        has(properties[TABLE_HEADER].tValue.iColor) &&
        has(properties[TABLE_CELL].tValue.iColor) &&
        has(properties[IMAGE_PLACEHOLDER].tValue.iColor) &&
        has(properties[IMAGE_BORDER].tValue.iColor) &&
        has(properties[IMAGE_TEXT].tValue.iColor));
    {
        xui_resource_desc_t resource_desc = {0};
        CHECK(xuiTestSurfaceCreate(&proxy, &image_surface, 80, 30, 0) == XUI_OK);
        resource_desc.iSize = sizeof(resource_desc); resource_desc.sName = "missing.png";
        resource_desc.iKind = XUI_RESOURCE_SURFACE; resource_desc.pHandle = image_surface;
        resource_desc.pUser = &proxy; resource_desc.onDestroy = image_destroy;
        CHECK(xuiResourceSet(ctx, &image_resource, &resource_desc) == XUI_OK);
        expected_image_surface = image_surface;
        CHECK(xuiUpdate(ctx, .016f) == XUI_OK);
        render(ctx, target);
        CHECK(image_draw_count > 0 && !has(properties[IMAGE_PLACEHOLDER].tValue.iColor) &&
            !has(properties[IMAGE_BORDER].tValue.iColor) &&
            !has(properties[IMAGE_TEXT].tValue.iColor) && xuiDocumentGetRevision(d) == version);
        CHECK(xuiDocumentViewSelectObject(editor, image_id) == XUI_OK);
        render(ctx, target);
        CHECK(image_draw_count > 0 && has(properties[SELECTION].tValue.iColor) &&
            !has(properties[IMAGE_PLACEHOLDER].tValue.iColor));
        expected_image_surface = NULL;
        resource_desc.iKind = XUI_RESOURCE_USER;
        resource_desc.pHandle = NULL; resource_desc.onDestroy = NULL;
        CHECK(xuiResourceSet(ctx, &image_resource, &resource_desc) == XUI_OK);
        image_surface = NULL;
        CHECK(xuiUpdate(ctx, .016f) == XUI_OK);
        render(ctx, target);
        CHECK(has(properties[IMAGE_PLACEHOLDER].tValue.iColor) &&
            has(properties[IMAGE_BORDER].tValue.iColor) &&
            has(properties[IMAGE_TEXT].tValue.iColor) &&
            has(properties[SELECTION].tValue.iColor));
        CHECK(xuiTestSurfaceCreate(&proxy, &image_surface, 120, 60, 0) == XUI_OK);
        resource_desc.iKind = XUI_RESOURCE_SURFACE;
        resource_desc.pHandle = image_surface; resource_desc.onDestroy = image_destroy;
        CHECK(xuiResourceSet(ctx, &image_resource, &resource_desc) == XUI_OK);
        expected_image_surface = image_surface;
        CHECK(xuiUpdate(ctx, .016f) == XUI_OK);
        render(ctx, target);
        CHECK(image_draw_count > 0 && last_image_source.fW == 120 &&
            last_image_source.fH == 60 && has(properties[SELECTION].tValue.iColor) &&
            !has(properties[IMAGE_PLACEHOLDER].tValue.iColor) &&
            xuiDocumentGetRevision(d) == version);
        expected_image_surface = NULL;
        CHECK(xuiResourceRemove(image_resource) == XUI_OK);
        image_resource = NULL; image_surface = NULL;
        CHECK(xuiUpdate(ctx, .016f) == XUI_OK);
        render(ctx, target);
        CHECK(has(properties[IMAGE_PLACEHOLDER].tValue.iColor) &&
            has(properties[IMAGE_BORDER].tValue.iColor) &&
            has(properties[IMAGE_TEXT].tValue.iColor) &&
            has(properties[SELECTION].tValue.iColor) &&
            xuiDocumentGetRevision(d) == version);
    }
    CHECK(xuiDocumentViewGetContentSize(editor, &after, &exact_after) == XUI_OK &&
        exact_before == exact_after && fabs(before.height - after.height) < .001 &&
        xuiDocumentGetRevision(d) == version);
    selection.tAnchor = position(d, body, 0); selection.tCaret = position(d, body, 3);
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK && xuiSetFocusWidget(ctx, editor) == XUI_OK);
    render(ctx, target);
    CHECK(has(properties[SELECTION].tValue.iColor) && has(properties[CARET].tValue.iColor) &&
        has(properties[FOCUS_BORDER].tValue.iColor));
    shape_before.iSize = shape_after.iSize = sizeof(shape_before);
    CHECK(xuiDocumentViewGetRenderStats(editor, &shape_before) == XUI_OK);
    properties[QUOTE_BORDER].tValue.iColor = 0x773311ffu;
    properties[TABLE_HEADER].tValue.iColor = 0x337711ffu;
    properties[IMAGE_BORDER].tValue.iColor = 0x113377ffu;
    CHECK(xuiStyleSetDefault(ctx, properties, 20) == XUI_OK);
    render(ctx, target);
    CHECK(has(properties[QUOTE_BORDER].tValue.iColor) &&
        has(properties[TABLE_HEADER].tValue.iColor) &&
        has(properties[IMAGE_BORDER].tValue.iColor));
    CHECK(xuiDocumentViewGetRenderStats(editor, &shape_after) == XUI_OK &&
        shape_after.iShapedBytes == shape_before.iShapedBytes &&
        xuiDocumentGetRevision(d) == version);
    CHECK(xuiDocumentViewSetFindQuery(editor, "default", 7) == XUI_OK);
    CHECK(xuiDocumentViewGetFindResultCount(editor, &find_count) == XUI_OK && find_count == 2);
    CHECK(xuiDocumentViewGetFindResult(editor, 0, &found, &active) == XUI_OK && !active &&
        found.tAnchor.iNodeId == body && found.tAnchor.iOffset == 0);
    render(ctx, target);
    CHECK(count_color(properties[FIND_RESULT].tValue.iColor) >= 2 &&
        !has(properties[FIND_ACTIVE].tValue.iColor));
    selection.tAnchor = selection.tCaret = position(d, body, 0);
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentViewFind(editor, "default", 7, 0, 1, &found) == XUI_OK &&
        found.tAnchor.iOffset == 0);
    render(ctx, target);
    CHECK(has(properties[FIND_ACTIVE].tValue.iColor));
    CHECK(xuiDocumentViewGetFindResult(editor, 0, &found, &active) == XUI_OK && active);
    CHECK(xuiDocumentViewFind(editor, "default", 7, 0, 0, &found) == XUI_OK &&
        found.tAnchor.iOffset == 8);
    CHECK(xuiDocumentViewFind(editor, "default", 7, 0, 0, &found) == XUI_ERROR_NOT_FOUND);
    CHECK(xuiDocumentViewFind(editor, "default", 7, 1, 0, &found) == XUI_OK &&
        found.tAnchor.iOffset == 0);
    transparent = color_property("document.background.color", 0);
    type_style.iSize = sizeof(type_style); type_style.pProperties = &transparent; type_style.iPropertyCount = 1;
    CHECK(xuiStyleSetType(ctx, xuiDocumentEditorGetType(ctx), &type_style) == XUI_OK);
    render(ctx, target); CHECK(!has(properties[BACKGROUND].tValue.iColor));
    CHECK(xuiStyleRemoveType(ctx, xuiDocumentEditorGetType(ctx)) == XUI_OK && xuiStyleClearDefault(ctx) == XUI_OK);
    render(ctx, target); CHECK(has(base_color) && has(base_background) && has(explicit_color) &&
        has(XUI_COLOR_RGBA(255, 235, 128, 150)) && has(XUI_COLOR_RGBA(255, 183, 77, 190)));
    CHECK(xuiDocumentGetRevision(d) == version);
    selection.tAnchor = position(d, body, 0); selection.tCaret = position(d, body, 7);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceRange(t, &selection, "changed", 7, &replacement_caret) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentViewGetFindResultCount(editor, &find_count) == XUI_OK && find_count == 1);
    CHECK(xuiDocumentViewGetFindResult(editor, 0, &found, &active) == XUI_OK &&
        found.tAnchor.iRevision == xuiDocumentGetRevision(d));
    CHECK(xuiDocumentViewSetFindQuery(editor, "absent", 6) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(editor, &find_count) == XUI_OK && !find_count);
    CHECK(xuiDocumentViewClearFind(editor) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(editor, &find_count) == XUI_OK && !find_count);
    CHECK(xuiDocumentViewSetFindQuery(editor, "changed", 7) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &other) == XUI_OK &&
        xuiDocumentViewSetDocument(editor, other) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(editor, &find_count) == XUI_OK && !find_count);
    CHECK(xuiDocumentViewSetDocument(editor, d) == XUI_OK);
    xuiDocumentRelease(other);

    {
        const char* source = "[link](/go) ==mark==\n\n> quote\n\n```\ncode\n```\n\n---\n";
        md_desc.iSize = sizeof(md_desc); md_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        md_desc.iMarkdownDialect = XUI_MD_EXTENDED;
        CHECK(xuiDocumentCreate(&md_desc, &md) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(md, source, strlen(source)) == XUI_OK);
    }
    view_desc.iSize = sizeof(view_desc); view_desc.pDocument = md; view_desc.iBackgroundColor = base_background;
    CHECK(xuiDocumentViewCreate(ctx, &view_desc, &view) == XUI_OK && xuiSetRootWidget(ctx, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 640, 480}) == XUI_OK && xuiLayout(ctx) == XUI_OK);
    xuiWidgetDestroy(editor);
    CHECK(xuiStyleSetDefault(ctx, properties, 20) == XUI_OK);
    version = xuiDocumentGetRevision(md); render(ctx, target);
    CHECK(has(properties[LINK].tValue.iColor) && has(properties[HIGHLIGHT].tValue.iColor) &&
        has(properties[QUOTE_BORDER].tValue.iColor) &&
        has(properties[RULE_COLOR].tValue.iColor) &&
        has(properties[CODE].tValue.iColor) && has(properties[BACKGROUND].tValue.iColor) &&
        has(properties[BORDER].tValue.iColor) &&
        xuiDocumentGetRevision(md) == version);
    CHECK(xuiDocumentViewSetFindQuery(view, "mark", 4) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(view, &find_count) == XUI_OK && find_count == 1);
    render(ctx, target); CHECK(has(properties[FIND_RESULT].tValue.iColor));
    CHECK(xuiDocumentViewSetMode(view, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(view, &find_count) == XUI_OK && find_count == 1);
    render(ctx, target); CHECK(has(properties[FIND_RESULT].tValue.iColor));
    CHECK(xuiDocumentViewSetMode(view, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(view, &find_count) == XUI_OK && find_count == 1);
    render(ctx, target); CHECK(has(properties[FIND_RESULT].tValue.iColor));
    CHECK(xuiDocumentViewSetMode(view, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentViewSetFindQuery(view, "==", 2) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(view, &find_count) == XUI_OK && find_count == 2);
    CHECK(xuiDocumentViewSetMode(view, XUI_DOC_VISUAL) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(view, &find_count) == XUI_OK && !find_count);
    CHECK(xuiDocumentGetRevision(md) == version);
    CHECK(xuiStyleClearDefault(ctx) == XUI_OK);
    xuiDocumentRelease(md); xuiDocumentRelease(d); xuiDestroy(ctx);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Unified Document stylesheet and find highlights: rich/editor and Markdown/view palettes, all/active results, revision/mode refresh, stable geometry passed");
    return 0;
}
