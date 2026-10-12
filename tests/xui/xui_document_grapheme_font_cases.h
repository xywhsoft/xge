/* The first visible fragment supplies a whole grapheme's font and baseline.
 * Compare a cross-font split against the explicitly styled whole unit. */
#include "xui_document_grapheme_font_model.h"
static xui_draw_text_spans_proc unit_font_base_spans;
static xui_draw_text_proc unit_font_base_text;
static unsigned unit_capture, unit_calls;
typedef struct unit_draw_record { char text[32]; xui_font font; xui_rect_t rect; } unit_draw_record;
static unit_draw_record unit_journal[8];
static void unit_record(xui_font font, const char* text, int bytes, xui_rect_t rect)
{
    if (unit_capture) {
        unit_draw_record* record;
        CHECK(unit_calls < 8 && bytes < 32);
        record = &unit_journal[unit_calls++]; memset(record, 0, sizeof(*record));
        memcpy(record->text, text, (size_t)bytes); record->font = font; record->rect = rect;
    }
}
static int unit_spans(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{
    xui_font font = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->pFont : NULL;
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    unit_record(font, text, bytes, rect);
    return unit_font_base_spans(proxy, draw, pTextItem, rect, color, flags, spans, count);
}
static int unit_text(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags)
{
    xui_font font = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->pFont : NULL;
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;

    unit_record(font, text, (int)strlen(text), rect);
    return unit_font_base_text(proxy, draw, pTextItem, rect, color, flags);
}
static void unit_rect_equal(xui_doc_rect_t a, xui_doc_rect_t b)
{
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01 &&
        fabs(a.width - b.width) < .01 && fabs(a.height - b.height) < .01);
}
static xui_doc_position_t unit_at(xui_document document, uint64_t paragraph, uint64_t offset, unsigned affinity)
{
    xui_document_snapshot snapshot; xui_doc_node_info_t parent = {0}, node = {0};
    uint64_t i, id = 0; xui_doc_position_t at = {0};
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    parent.iSize = sizeof(parent); node.iSize = sizeof(node);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, paragraph, &parent) == XUI_OK);
    for (i = 0; i < parent.iChildCount; i++) {
        CHECK(xuiDocumentSnapshotGetChild(snapshot, paragraph, i, &id) == XUI_OK &&
            xuiDocumentSnapshotGetNode(snapshot, id, &node) == XUI_OK && node.iKind == XUI_DOC_TEXT);
        if (offset <= node.iTextBytes) { at = decoration_position(document, id, offset); break; }
        offset -= node.iTextBytes;
    }
    CHECK(at.iSize); at.iAffinity = affinity; xuiDocumentSnapshotRelease(snapshot); return at;
}
static void document_grapheme_fonts(xui_surface target, xui_test_proxy_state_t* proxy)
{
    static const uint32_t styles[] = {0, XUI_DOC_BOLD, XUI_DOC_SUPERSCRIPT, XUI_DOC_SUBSCRIPT};
    xui_proxy_t saved = proxy->tProxy; unsigned backend, style, form, empty, variant, pass;
    unit_font_base_shape = saved.textShape; unit_font_base_spans = saved.drawTextSpans; unit_font_base_text = saved.drawText;
    CHECK(saved.fontLoadFile(&saved, &unit_fonts[0], "unit-normal.ttf", 20, 0) == XUI_OK &&
        saved.fontLoadFile(&saved, &unit_fonts[1], "unit-bold.ttf", 40, 0) == XUI_OK);
    for (backend = 0; backend < 2; backend++) {
        xui_context context; xui_doc_renderer_desc_t desc = {0};
        proxy->tProxy.textShape = unit_shape; proxy->tProxy.drawTextSpans = backend ? NULL : unit_spans;
        proxy->tProxy.drawText = unit_text;
        CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK &&
            xuiSetDefaultFont(context, unit_fonts[0]) == XUI_OK);
        desc.iSize = sizeof(desc); desc.tFonts = (xui_doc_font_set_t){unit_fonts[0], unit_fonts[1], unit_fonts[0], unit_fonts[1], unit_fonts[0]};
        desc.onFont = unit_font; desc.fLineGap = 4;
        for (unit_sample = 0; unit_sample < 3; unit_sample++) for (style = 0; style < 4; style++)
        for (form = 0; form < 2; form++) for (empty = 0; empty < 2; empty++) {
            xui_document document[2]; xui_document_renderer renderer[2]; uint64_t first[2], tail[2], parents[2];
            unsigned length = (unsigned)strlen(unit_stems[unit_sample]), split = unit_prefix[unit_sample];
            uint32_t next_marks = style == 0 ? XUI_DOC_BOLD : style == 1 ? 0 : styles[style == 2 ? 3 : 2];
            if (form && (style >= 2 || empty)) continue;
            for (variant = 0; variant < 2; variant++) {
                xui_document_snapshot snapshot; uint64_t paragraph;
                if (form) {
                    xui_doc_desc_t profile = {0}; char source[48];
                    profile.iSize = sizeof(profile); profile.iProfile = XUI_DOCUMENT_MARKDOWN;
                    if (style) snprintf(source, sizeof(source), "**%.*s**%s", variant ? (int)split : (int)length,
                        unit_stems[unit_sample], variant ? unit_md_tails[unit_sample] : "&#88;Y");
                    else if (variant) snprintf(source, sizeof(source), "%.*s**%sXY**", (int)split,
                        unit_stems[unit_sample], unit_stems[unit_sample] + split);
                    else snprintf(source, sizeof(source), "%s**XY**", unit_stems[unit_sample]);
                    CHECK(xuiDocumentCreate(&profile, &document[variant]) == XUI_OK &&
                        xuiDocumentLoadMarkdown(document[variant], source, strlen(source)) == XUI_OK &&
                        xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
                        xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK &&
                        xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &first[variant]) == XUI_OK);
                    {
                        int result = xuiDocumentSnapshotGetChild(snapshot, paragraph, 1, &tail[variant]);
                        if (result != XUI_OK) fprintf(stderr, "Markdown grapheme sample=%u style=%u variant=%u source=%s child-result=%d\n",
                            unit_sample, style, variant, source, result);
                        CHECK(result == XUI_OK);
                    }
                    xuiDocumentSnapshotRelease(snapshot);
                } else {
                    xui_document_transaction transaction; char prefix[16], suffix[24];
                    CHECK(xuiDocumentCreate(NULL, &document[variant]) == XUI_OK &&
                        xuiDocumentBeginTransaction(document[variant], NULL, &transaction) == XUI_OK);
                    paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
                    memcpy(prefix, unit_stems[unit_sample], split); prefix[split] = 0;
                    first[variant] = decoration_insert(transaction, paragraph, variant ? prefix : unit_stems[unit_sample], styles[style], 0);
                    if (variant && empty) decoration_insert(transaction, paragraph, "", XUI_DOC_BOLD | XUI_DOC_SUPERSCRIPT, 0);
                    snprintf(suffix, sizeof(suffix), "%sXY", variant ? unit_stems[unit_sample] + split : "");
                    tail[variant] = decoration_insert(transaction, paragraph, suffix, next_marks, 0);
                    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
                }
                parents[variant] = paragraph;
                CHECK(xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
                    xuiDocumentRendererCreate(context, &desc, &renderer[variant]) == XUI_OK &&
                    xuiDocumentRendererSetSnapshot(renderer[variant], snapshot, NULL) == XUI_OK);
                xuiDocumentSnapshotRelease(snapshot);
            }
            for (pass = 0; pass < 3; pass++) {
                xui_doc_rect_t heads[2], ends[2], next[2], rects[2][8]; uint64_t counts[2];
                unit_draw_record records[2][8]; unsigned draws[2];
                for (variant = 0; variant < 2; variant++) {
                    xui_document_snapshot snapshot;
                    xui_doc_position_t head = unit_at(document[variant], parents[variant], 0, XUI_DOC_AFTER);
                    xui_doc_position_t end = unit_at(document[variant], parents[variant], length, XUI_DOC_BEFORE), at, hit;
                    xui_doc_range_t range = {head, end}; xui_draw_context draw; int order;
                    end.iAffinity = XUI_DOC_BEFORE; range.tCaret = end;
                    CHECK(xuiDocumentRendererLayout(renderer[variant], pass == 1 ? 7 : 200, 0, 200) == XUI_OK &&
                        xuiDocumentRendererGetCaretRect(renderer[variant], &head, &heads[variant]) == XUI_OK &&
                        xuiDocumentRendererGetCaretRect(renderer[variant], &end, &ends[variant]) == XUI_OK);
                    if (fabs(ends[variant].x - heads[variant].x - (style == 1 ? 22 : 11)) >= .01)
                        fprintf(stderr, "Grapheme font backend=%u sample=%u style=%u form=%u empty=%u variant=%u pass=%u advance=%g\n",
                            backend, unit_sample, style, form, empty, variant, pass, ends[variant].x - heads[variant].x);
                    CHECK(fabs(ends[variant].x - heads[variant].x - (style == 1 ? 22 : 11)) < .01);
                    at = unit_at(document[variant], parents[variant], length + 1, XUI_DOC_AFTER);
                    CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &next[variant]) == XUI_OK &&
                        xuiDocumentRendererGetRangeRects(renderer[variant], &range, rects[variant], 8, &counts[variant]) == XUI_OK && counts[variant] == 1 &&
                        xuiDocumentRendererHitTest(renderer[variant], heads[variant].x + (ends[variant].x - heads[variant].x) * .75,
                            heads[variant].y + heads[variant].height * .5, &hit) == XUI_OK &&
                        xuiDocumentAcquireSnapshot(document[variant], &snapshot) == XUI_OK &&
                        xuiDocumentSnapshotComparePositions(snapshot, &end, &hit, &order) == XUI_OK);
                    if (order) fprintf(stderr, "Grapheme font hit backend=%u sample=%u style=%u form=%u empty=%u variant=%u pass=%u expected=%llu:%llu hit=%llu:%llu order=%d\n",
                        backend, unit_sample, style, form, empty, variant, pass, (unsigned long long)end.iNodeId,
                        (unsigned long long)end.iOffset, (unsigned long long)hit.iNodeId, (unsigned long long)hit.iOffset, order);
                    CHECK(!order);
                    xuiDocumentSnapshotRelease(snapshot);
                    if (variant) {
                        xui_doc_rect_t before, after;
                        at = decoration_position(document[variant], first[variant], split); at.iAffinity = XUI_DOC_BEFORE;
                        CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &before) == XUI_OK);
                        at.iAffinity = XUI_DOC_AFTER;
                        CHECK(xuiDocumentRendererGetCaretRect(renderer[variant], &at, &after) == XUI_OK);
                        unit_rect_equal(before, heads[variant]); unit_rect_equal(after, ends[variant]);
                    }
                    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
                    unit_capture = 1; unit_calls = 0;
                    CHECK(xuiDocumentRendererDraw(renderer[variant], draw, 0, 0, (xui_rect_t){0, 0, 300, 200}, NULL, 0) == XUI_OK);
                    unit_capture = 0; draws[variant] = unit_calls; memcpy(records[variant], unit_journal, sizeof(unit_journal));
                    CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
                }
                unit_rect_equal(heads[0], heads[1]); unit_rect_equal(ends[0], ends[1]); unit_rect_equal(next[0], next[1]);
                unit_rect_equal(rects[0][0], rects[1][0]); CHECK(draws[0] == draws[1] && draws[1] >= 2);
                for (variant = 0; variant < draws[0]; variant++) CHECK(!strcmp(records[0][variant].text, records[1][variant].text) &&
                    records[0][variant].font == records[1][variant].font &&
                    !memcmp(&records[0][variant].rect, &records[1][variant].rect, sizeof(xui_rect_t)));
            }
            if (!form && !empty && !style) for (pass = 0; pass < 4; pass++) {
                xui_document_snapshot snapshot; xui_document_change_set change;
                xui_doc_position_t at; xui_doc_rect_t caret, eof;
                if (pass < 2) {
                    xui_doc_node_info_t node = {0}; xui_document_transaction transaction;
                    node.iSize = sizeof(node);
                    CHECK(xuiDocumentAcquireSnapshot(document[1], &snapshot) == XUI_OK &&
                        xuiDocumentSnapshotGetNode(snapshot, pass ? first[1] : tail[1], &node) == XUI_OK);
                    xuiDocumentSnapshotRelease(snapshot);
                    node.tAttributes.iMarks = pass ? XUI_DOC_BOLD : 0;
                    CHECK(xuiDocumentBeginTransaction(document[1], NULL, &transaction) == XUI_OK &&
                        xuiDocumentTxnSetAttributes(transaction, node.iId, &node.tAttributes) == XUI_OK &&
                        xuiDocumentTxnCommit(transaction, &change) == XUI_OK); xuiDocumentTxnRelease(transaction);
                } else CHECK(xuiDocumentUndo(document[1], &change) == XUI_OK);
                CHECK(xuiDocumentAcquireSnapshot(document[1], &snapshot) == XUI_OK &&
                    xuiDocumentRendererSetSnapshot(renderer[1], snapshot, change) == XUI_OK);
                xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(change);
                at = unit_at(document[1], parents[1], length, XUI_DOC_BEFORE);
                CHECK(xuiDocumentRendererLayout(renderer[1], 200, 0, 200) == XUI_OK &&
                    xuiDocumentRendererGetCaretRect(renderer[1], &at, &caret) == XUI_OK && fabs(caret.x - (pass == 1 ? 22 : 11)) < .01);
                at = unit_at(document[1], parents[1], length + 2, XUI_DOC_BEFORE);
                CHECK(xuiDocumentRendererGetCaretRect(renderer[1], &at, &eof) == XUI_OK &&
                    fabs(eof.x - (pass == 1 ? 42 : pass == 3 ? 51 : 31)) < .01);
            }
            if (!form && !empty && !style) for (pass = 1; pass <= 2; pass++) {
                xui_document_snapshot snapshot; xui_document_renderer retry; xui_doc_rect_t caret;
                xui_doc_position_t at = decoration_position(document[1], tail[1], length - split);
                uint64_t revision = xuiDocumentGetRevision(document[1]);
                CHECK(xuiDocumentAcquireSnapshot(document[1], &snapshot) == XUI_OK &&
                    xuiDocumentRendererCreate(context, &desc, &retry) == XUI_OK &&
                    xuiDocumentRendererSetSnapshot(retry, snapshot, NULL) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
                unit_fail = pass;
                CHECK(xuiDocumentRendererLayout(retry, 200, 0, 200) == XUI_ERROR_OUT_OF_MEMORY && !unit_fail &&
                    xuiDocumentGetRevision(document[1]) == revision);
                CHECK(xuiDocumentRendererLayout(retry, 200, 0, 200) == XUI_OK &&
                    xuiDocumentRendererGetCaretRect(retry, &at, &caret) == XUI_OK && fabs(caret.x - 11) < .01);
                xuiDocumentRendererRelease(retry);
            }
            for (variant = 0; variant < 2; variant++) { xuiDocumentRendererRelease(renderer[variant]); xuiDocumentRelease(document[variant]); }
        }
        xuiDestroy(context);
    }
    proxy->tProxy = saved; saved.fontDestroy(&saved, unit_fonts[0]); saved.fontDestroy(&saved, unit_fonts[1]);
    puts("Cross-font graphemes: combining/ZWJ/RI, Rich/Markdown, font/script owners, empty carriers, spans/plain draw, wrap/reflow, caret/hit/ranges, font mutation/undo and seed/line OOM retry passed");
}
