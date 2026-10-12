/* Former byte/run/line cutoffs must not make a color boundary a shaping
 * boundary. Use the same content and font in the joined and split views. */
static unsigned joint_scale_fail_large, joint_scale_capture_calls;
static int joint_scale_capture;
static uint64_t joint_scale_digest;
static xui_draw_text_spans_proc joint_scale_base_spans;
static int joint_scale_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    if (bytes >= 8190 && joint_scale_fail_large && !--joint_scale_fail_large)
        return XUI_ERROR_OUT_OF_MEMORY;
    return decoration_shape(proxy, pTextItem, shape);
}
static void joint_scale_hash(const void* data, size_t bytes)
{
    const unsigned char* text = data; size_t i;
    for (i = 0; i < bytes; i++) { joint_scale_digest ^= text[i]; joint_scale_digest *= UINT64_C(1099511628211); }
}
static int joint_scale_spans(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    if (joint_scale_capture) {
        int i; joint_scale_capture_calls++;
        joint_scale_hash(text, (size_t)bytes); joint_scale_hash(&rect, sizeof(rect));
        for (i = 0; i < count; i++) {
            joint_scale_hash(&spans[i].iStart, sizeof(spans[i].iStart));
            joint_scale_hash(&spans[i].iEnd, sizeof(spans[i].iEnd));
            joint_scale_hash(&spans[i].iColor, sizeof(spans[i].iColor));
        }
    }
    return joint_scale_base_spans(proxy, draw, pTextItem, rect, color, flags, spans, count);
}
static uint64_t joint_scale_draw_digest(xui_document_renderer renderer,
    xui_test_proxy_state_t* proxy, xui_surface surface)
{
    xui_draw_context draw;
    joint_scale_digest = UINT64_C(1469598103934665603); joint_scale_capture_calls = 0; joint_scale_capture = 1;
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, surface) == XUI_OK &&
        xuiDocumentRendererDraw(renderer, draw, 0, 0, (xui_rect_t){0, 0, 320, 200}, NULL, 0) == XUI_OK &&
        proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK && joint_scale_capture_calls >= 8);
    joint_scale_capture = 0;
    return joint_scale_digest;
}
static void document_joint_scale(xui_test_proxy_state_t* proxy)
{
    xui_context context; xui_font font; xui_surface surface;
    xui_text_shape_proc saved = proxy->tProxy.textShape;
    unsigned sample, variant, pass;
    static const size_t word_counts[] = {3000, 35, 600, 30000};
    decoration_base_shape = saved; proxy->tProxy.textShape = joint_scale_shape;
    joint_scale_base_spans = proxy->tProxy.drawTextSpans; proxy->tProxy.drawTextSpans = joint_scale_spans;
    CHECK(proxy->tProxy.fontLoadFile(&proxy->tProxy, &font, "joint-scale.ttf", 20, 0) == XUI_OK &&
        xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK &&
        xuiTestSurfaceCreate(proxy, &surface, 320, 200, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    for (sample = 0; sample < 4; sample++) {
        size_t bytes = word_counts[sample] * 3 - (sample == 1 ? 0 : 1), i;
        char* text = malloc(bytes + 1);
        xui_document documents[3]; xui_document_renderer renderers[3];
        uint64_t* ids[3]; size_t nodes[3]; unsigned variants = sample == 1 ? 2u : 3u;
        char* markdown = malloc(bytes + 5);
        xui_doc_renderer_desc_t desc = {0};
        CHECK(text && markdown); for (i = 0; i < bytes; i += 3) memcpy(text + i, "AV ", 3); text[bytes] = 0;
        memcpy(markdown, "**A**", 5); memcpy(markdown + 5, text + 1, bytes); /* Include the source terminator. */
        desc.iSize = sizeof(desc); desc.fLineGap = 4; desc.tFonts = (xui_doc_font_set_t){font, font, font, font, font};
        for (variant = 0; variant < variants; variant++) {
            xui_document_transaction transaction; xui_document_snapshot snapshot;
            uint64_t paragraph;
            xui_doc_desc_t profile = {0};
            nodes[variant] = !variant ? 1 : sample == 1 ? bytes : 2;
            ids[variant] = calloc(nodes[variant], sizeof(*ids[variant])); CHECK(ids[variant]);
            profile.iSize = sizeof(profile); profile.iProfile = variant == 2 ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
            CHECK(xuiDocumentCreate(&profile, &documents[variant]) == XUI_OK);
            if (variant == 2) CHECK(xuiDocumentLoadMarkdown(documents[variant], markdown, bytes + 4) == XUI_OK);
            else {
                CHECK(xuiDocumentBeginTransaction(documents[variant], NULL, &transaction) == XUI_OK);
                paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
                if (!variant) ids[variant][0] = decoration_insert(transaction, paragraph, text, 0, 0);
                else if (sample == 1) {
                    for (i = 0; i < bytes; i++) {
                        char scalar[2] = {text[i], 0};
                        ids[variant][i] = decoration_insert(transaction, paragraph, scalar, 0,
                            i & 1 ? XUI_COLOR_RGBA(180, 20, 40, 255) : XUI_COLOR_RGBA(20, 40, 180, 255));
                    }
                } else {
                    ids[variant][0] = decoration_insert(transaction, paragraph, "A", 0, 0);
                    ids[variant][1] = decoration_insert(transaction, paragraph, text + 1, 0, XUI_COLOR_RGBA(180, 20, 40, 255));
                }
                CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
            }
            CHECK(xuiDocumentAcquireSnapshot(documents[variant], &snapshot) == XUI_OK);
            if (variant == 2) CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &ids[variant][0]) == XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot, paragraph, 1, &ids[variant][1]) == XUI_OK);
            CHECK(xuiDocumentRendererCreate(context, &desc, &renderers[variant]) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderers[variant], snapshot, NULL) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
        }
        for (pass = 0; pass < 3; pass++) {
            double width = pass == 1 ? 200 : sample == 1 ? 2000 : 27;
            xui_doc_rect_t first[3], last[3];
            for (variant = 0; variant < variants; variant++) {
                xui_doc_position_t at;
                xui_draw_context draw; xui_doc_renderer_stats_t stats = {0}, baseline = {0};
                baseline.iSize = sizeof(baseline);
                CHECK(xuiDocumentRendererGetStats(renderers[variant], &baseline) == XUI_OK);
                CHECK(xuiDocumentRendererLayout(renderers[variant], width, 0, sample == 3 ? 80 : 1000000) == XUI_OK);
                at = decoration_position(documents[variant], ids[variant][variant ? 1 : 0], variant ? 1 : 2);
                CHECK(xuiDocumentRendererGetCaretRect(renderers[variant], &at, &first[variant]) == XUI_OK &&
                    fabs(first[variant].x - 17) < .01 && fabs(first[variant].y) < .01);
                if (sample == 3) {
                    /* Query the unfinished line at the first shaping cut.
                     * It must extend the prefix while leaving the far tail cold. */
                    uint64_t offsets[] = {8192, 16385, 32768}; unsigned query;
                    stats.iSize = sizeof(stats);
                    CHECK(xuiDocumentRendererGetStats(renderers[variant], &stats) == XUI_OK);
                    if (!pass) CHECK(stats.iFragments - baseline.iFragments < bytes / 3 &&
                        stats.iTextRunBytes - baseline.iTextRunBytes < 10000 &&
                        stats.iShapedBytes - baseline.iShapedBytes < (stats.iTextRunBytes - baseline.iTextRunBytes) * 4);
                    else CHECK(stats.iFragments - baseline.iFragments < bytes &&
                        stats.iTextRunBytes == baseline.iTextRunBytes &&
                        stats.iShapedBytes - baseline.iShapedBytes < baseline.iTextRunBytes * 4);
                    for (query = 0; query < 3; query++) {
                        xui_doc_rect_t caret;
                        at = decoration_position(documents[variant], ids[variant][variant ? 1 : 0], offsets[query] - (variant ? 1 : 0));
                        CHECK(xuiDocumentRendererGetCaretRect(renderers[variant], &at, &caret) == XUI_OK);
                        if (pass != 1) CHECK(fabs(caret.x - 17) < .01 && fabs(caret.y - (offsets[query] / 3) * 24) < .01);
                        last[variant] = caret;
                    }
                    CHECK(xuiDocumentRendererGetStats(renderers[variant], &stats) == XUI_OK &&
                        stats.iFragments - baseline.iFragments < bytes);
                    if (pass) CHECK(stats.iTextRunBytes == baseline.iTextRunBytes);
                } else {
                    at = decoration_position(documents[variant], ids[variant][nodes[variant] - 1],
                        variant ? sample == 1 ? 1 : bytes - 1 : bytes);
                    CHECK(xuiDocumentRendererGetCaretRect(renderers[variant], &at, &last[variant]) == XUI_OK);
                    if (pass != 1 && sample != 1) CHECK(fabs(last[variant].x - 17) < .01 &&
                        fabs(last[variant].y - (word_counts[sample] - 1) * 24) < .01);
                    if (sample == 1 && pass != 1) CHECK(fabs(last[variant].x - word_counts[sample] * 27) < .01 && fabs(last[variant].y) < .01);
                }
                CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, surface) == XUI_OK &&
                    xuiDocumentRendererDraw(renderers[variant], draw, 0, 0, (xui_rect_t){0, 0, 320, 200}, NULL, 0) == XUI_OK &&
                    proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
                if (variant == 2) {
                    xui_document_snapshot snapshot; char* copied = malloc(bytes + 5); uint64_t source_bytes;
                    CHECK(copied && xuiDocumentAcquireSnapshot(documents[variant], &snapshot) == XUI_OK &&
                        xuiDocumentSnapshotCopySource(snapshot, copied, bytes + 5, &source_bytes) == XUI_OK &&
                        source_bytes == bytes + 4 && !memcmp(copied, markdown, bytes + 4));
                    xuiDocumentSnapshotRelease(snapshot); free(copied);
                }
            }
            for (variant = 1; variant < variants; variant++) CHECK(fabs(first[0].x - first[variant].x) < .01 && fabs(first[0].y - first[variant].y) < .01 &&
                fabs(last[0].x - last[variant].x) < .01 && fabs(last[0].y - last[variant].y) < .01);
        }
        for (variant = 0; variant < variants; variant++) {
            xuiDocumentRendererRelease(renderers[variant]); xuiDocumentRelease(documents[variant]); free(ids[variant]);
        }
        free(text); free(markdown);
    }
    /* Fail before the first seed, inside its joint measurement, and in a
     * later growth step after an earlier provisional tail was reshaped.
     * Both committed geometry and draw strings must survive unchanged. */
    {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_doc_renderer_desc_t desc = {0}; uint64_t paragraph, tail;
        char* text = malloc(90001); size_t i;
        CHECK(text); for (i = 0; i < 90000; i += 3) memcpy(text + i, "AV ", 3); text[90000] = 0;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
        decoration_insert(transaction, paragraph, "A", 0, 0);
        tail = decoration_insert(transaction, paragraph, text + 1, 0, XUI_COLOR_RGBA(180, 20, 40, 255));
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        for (sample = 1; sample <= 3; sample++) {
            xui_document_renderer renderer; xui_doc_position_t at = decoration_position(document, tail, 8188);
            xui_doc_rect_t before, after; uint64_t digest;
            CHECK(xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
                xuiDocumentRendererLayout(renderer, 27, 0, 80) == XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer, &at, &before) == XUI_OK);
            digest = joint_scale_draw_digest(renderer, proxy, surface);
            joint_scale_fail_large = sample;
            CHECK(xuiDocumentRendererLayout(renderer, 27, 250000, 80) == XUI_ERROR_OUT_OF_MEMORY && !joint_scale_fail_large &&
                xuiDocumentRendererGetCaretRect(renderer, &at, &after) == XUI_OK &&
                fabs(before.x - after.x) < .01 && fabs(before.y - after.y) < .01 &&
                joint_scale_draw_digest(renderer, proxy, surface) == digest &&
                xuiDocumentRendererLayout(renderer, 27, 250000, 80) == XUI_OK &&
                joint_scale_draw_digest(renderer, proxy, surface) == digest);
            xuiDocumentRendererRelease(renderer);
        }
        xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(document); free(text);
    }
    proxy->tProxy.surfaceDestroy(&proxy->tProxy, surface); xuiDestroy(context); proxy->tProxy.fontDestroy(&proxy->tProxy, font);
    proxy->tProxy.textShape = saved;
    proxy->tProxy.drawTextSpans = joint_scale_base_spans;
    puts("Document joint scale: over 8 KiB, 105 color nodes, 600 rows and ~90 KiB lazy continuation retain Rich joined/split and Markdown kerning, caret, reflow/draw/source; 3 extension failures preserve geometry and paint then retry");
}
