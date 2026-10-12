/* Deterministic context-sensitive advances expose both a two-state line-cut
 * cycle and a paragraph needing more than eight global replans. */
static xui_text_shape_proc joint_convergence_base_shape;
static xui_draw_text_spans_proc joint_convergence_base_spans;
static unsigned joint_convergence_mode, joint_convergence_long_shapes;
static unsigned joint_convergence_four_seen, joint_convergence_fail_four;
static unsigned joint_convergence_fail_two_after_four, joint_convergence_draw_rows;
static int joint_convergence_capture;
static int joint_convergence_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    int result, i; double total = 0;
    float advance = !joint_convergence_mode && (bytes == 3 || bytes == 4) ? 10.f : 5.f;
    if (joint_convergence_mode && bytes > 8) joint_convergence_long_shapes++;
    if (joint_convergence_mode && bytes == 4 && !memcmp(text, "A A ", 4)) {
        joint_convergence_four_seen++;
        if (joint_convergence_fail_four && !--joint_convergence_fail_four) return XUI_ERROR_OUT_OF_MEMORY;
    }
    if (joint_convergence_mode && joint_convergence_four_seen && bytes == 2 &&
        joint_convergence_fail_two_after_four && !--joint_convergence_fail_two_after_four)
        return XUI_ERROR_OUT_OF_MEMORY;
    result = joint_convergence_base_shape(proxy, pTextItem, shape);
    if (result == XUI_OK) {
        for (i = 0; i < shape->iClusterCount; i++) {
            float measured = joint_convergence_mode && bytes >= 4 ?
                (shape->pClusters[i].iTextStart >= bytes - 2 ? 11.f : 0.f) : advance;
            shape->pClusters[i].fAdvance = measured; total += measured;
        }
        shape->fWidth = (float)total;
    }
    return result;
}
static int joint_convergence_spans(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    if (joint_convergence_capture) {
        unsigned words = joint_convergence_mode ? 21 : 3;
        CHECK(joint_convergence_draw_rows < words && bytes == (joint_convergence_draw_rows + 1 == words ? 1 : 2));
        CHECK(text[0] == 'A' && (bytes == 1 || text[1] == ' ') &&
            fabs((double)rect.fX) < .01 && fabs((double)rect.fY - joint_convergence_draw_rows * 24) < .01 &&
            fabs((double)rect.fW - bytes * 5) < .01);
        joint_convergence_draw_rows++;
    }
    return joint_convergence_base_spans(proxy, draw, pTextItem, rect, color, flags, spans, count);
}
static void joint_convergence_check(xui_document document, xui_document_renderer renderer,
    uint64_t text_id, unsigned split, unsigned words,
    xui_test_proxy_state_t* proxy, xui_surface surface)
{
    unsigned row; xui_draw_context draw;
    for (row = 1; row < words; row++) {
        xui_doc_position_t at = decoration_position(document, text_id, row * 2 - split);
        xui_doc_rect_t before, after; xui_doc_position_t hit;
        at.iAffinity = XUI_DOC_BEFORE;
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &before) == XUI_OK &&
            fabs(before.x - 10) < .01 && fabs(before.y - (row - 1) * 24) < .01);
        at.iAffinity = XUI_DOC_AFTER;
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &after) == XUI_OK &&
            fabs(after.x) < .01 && fabs(after.y - row * 24) < .01);
        CHECK(xuiDocumentRendererHitTest(renderer, 10, (row - 1) * 24 + 10, &hit) == XUI_OK &&
            hit.iNodeId == text_id && hit.iOffset == row * 2 - split && hit.iAffinity == XUI_DOC_BEFORE);
    }
    {
        xui_doc_position_t at = decoration_position(document, text_id, words * 2 - 1 - split);
        xui_doc_rect_t caret;
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
            fabs(caret.x - 5) < .01 && fabs(caret.y - (words - 1) * 24) < .01);
    }
    joint_convergence_draw_rows = 0; joint_convergence_capture = 1;
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, surface) == XUI_OK &&
        xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 320, 600}, NULL, 0) == XUI_OK &&
        proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && joint_convergence_draw_rows == words);
    joint_convergence_capture = 0;
}
static void document_joint_convergence(xui_test_proxy_state_t* proxy)
{
    xui_context context; xui_font font; xui_surface surface; unsigned sample, variant;
    xui_text_shape_proc saved = proxy->tProxy.textShape;
    joint_convergence_base_shape = saved; proxy->tProxy.textShape = joint_convergence_shape;
    joint_convergence_base_spans = proxy->tProxy.drawTextSpans; proxy->tProxy.drawTextSpans = joint_convergence_spans;
    CHECK(proxy->tProxy.fontLoadFile(&proxy->tProxy, &font, "joint-convergence.ttf", 20, 0) == XUI_OK &&
        xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK &&
        xuiTestSurfaceCreate(proxy, &surface, 320, 600, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    for (sample = 0; sample < 2; sample++) for (variant = 0; variant < 3; variant++) {
        unsigned words = sample ? 21 : 3, pass;
        size_t bytes = words * 2 - 1, i; char text[42], markdown[46];
        uint64_t paragraph, text_id; xui_document document; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_desc_t profile = {0}; xui_doc_renderer_desc_t desc = {0};
        joint_convergence_mode = sample;
        for (i = 0; i < bytes; i += 2) memcpy(text + i, "A ", 2);
        text[bytes] = 0;
        memcpy(markdown, "**A**", 5); memcpy(markdown + 5, text + 1, bytes);
        profile.iSize = sizeof(profile); profile.iProfile = variant == 2 ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK);
        if (variant == 2) CHECK(xuiDocumentLoadMarkdown(document, markdown, bytes + 4) == XUI_OK);
        else {
            xui_document_transaction transaction;
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
            if (variant) {
                decoration_insert(transaction, paragraph, "A", 0, 0xff3366ffu);
                text_id = decoration_insert(transaction, paragraph, text + 1, 0, 0xffee6633u);
            } else text_id = decoration_insert(transaction, paragraph, text, 0, 0xff3366ffu);
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        }
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        if (variant == 2) CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, paragraph, 1, &text_id) == XUI_OK);
        desc.iSize = sizeof(desc); desc.fLineGap = 4; desc.tFonts = (xui_doc_font_set_t){font, font, font, font, font};
        CHECK(xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        for (pass = 0; pass < 3; pass++) {
            CHECK(xuiDocumentRendererLayout(renderer, 1000, 0, 600) == XUI_OK);
            CHECK(xuiDocumentRendererLayout(renderer, sample ? 10 : 20, 0, 600) == XUI_OK);
            joint_convergence_check(document, renderer, text_id, !!variant, words, proxy, surface);
        }
        if (sample && variant == 1) for (pass = 0; pass < 2; pass++) {
            uint64_t revision = xuiDocumentGetRevision(document);
            int failure;
            CHECK(xuiDocumentRendererLayout(renderer, 1000, 0, 600) == XUI_OK);
            joint_convergence_four_seen = 0; joint_convergence_long_shapes = 0;
            joint_convergence_fail_four = pass ? 0 : 1;
            /* The exact planner measures twenty "A " candidates. The first
             * precedes "A A "; the next nineteen follow it. The twentieth
             * subsequent two-byte shape publishes the first chosen row. */
            joint_convergence_fail_two_after_four = pass ? 20 : 0;
            failure = xuiDocumentRendererLayout(renderer, 10, 0, 600);
            printf("Joint exact failure #%u: result=%d large=%u four=%u four-left=%u two-left=%u\n",
                pass + 1, failure, joint_convergence_long_shapes, joint_convergence_four_seen,
                joint_convergence_fail_four, joint_convergence_fail_two_after_four);
            CHECK(failure == XUI_ERROR_OUT_OF_MEMORY &&
                !joint_convergence_fail_four && !joint_convergence_fail_two_after_four &&
                joint_convergence_long_shapes >= 8 && xuiDocumentGetRevision(document) == revision);
            if (pass) CHECK(joint_convergence_four_seen == 19);
            CHECK(xuiDocumentRendererLayout(renderer, 10, 0, 600) == XUI_OK);
            joint_convergence_check(document, renderer, text_id, 1, words, proxy, surface);
        }
        if (variant == 2) {
            char copy[46]; uint64_t source_bytes;
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotCopySource(snapshot, copy, sizeof(copy), &source_bytes) == XUI_OK &&
                source_bytes == bytes + 4 && !memcmp(copy, markdown, bytes + 4));
            xuiDocumentSnapshotRelease(snapshot);
        }
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    proxy->tProxy.textShape = saved; proxy->tProxy.drawTextSpans = joint_convergence_base_spans;
    proxy->tProxy.surfaceDestroy(&proxy->tProxy, surface); xuiDestroy(context);
    proxy->tProxy.fontDestroy(&proxy->tProxy, font);
    puts("Document joint convergence: cyclic/slow contexts in Rich joined/split and Markdown retain exact row strings, carets, hits, reflow and candidate/publish failure retries passed");
}
