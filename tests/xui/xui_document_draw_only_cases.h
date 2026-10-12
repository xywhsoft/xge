static xui_text_shape_proc draw_only_base_shape;
static xui_draw_text_proc draw_only_base_text;
static xui_draw_clip_set_proc draw_only_base_clip_set;
static xui_draw_clip_get_proc draw_only_base_clip_get;
static xui_draw_clip_clear_proc draw_only_base_clip_clear;
static unsigned draw_only_clip_error, draw_only_get_error, draw_only_clear_error;
static unsigned draw_only_shape_error;
static unsigned draw_only_sample, draw_only_capture, draw_only_calls, draw_only_error;
static struct { char text[32]; xui_rect_t rect, clip; uint32_t color; int clipped; } draw_only_journal[16];
static const char* const draw_only_sources[] = {"AVX", "fiX", "e\xcc\x81iX"};
static int draw_only_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    int result = draw_only_base_shape(proxy, pTextItem, shape);
    if (result == XUI_OK && bytes == 2 && !memcmp(text, "AV", 2) &&
        draw_only_shape_error && !--draw_only_shape_error) return XUI_ERROR_OUT_OF_MEMORY;
    if (result == XUI_OK) {
        int i, count = 0; double width = 0;
        for (i = 0; i < shape->iClusterCount; i++) {
            xui_text_cluster_t cluster = shape->pClusters[i];
            if (bytes - cluster.iTextStart >= 2 && !memcmp(text + cluster.iTextStart, "AV", 2)) cluster.fAdvance -= 3;
            if (bytes - cluster.iTextStart >= 2 && !memcmp(text + cluster.iTextStart, "fi", 2)) {
                cluster.iTextEnd = cluster.iTextStart + 2; cluster.fAdvance = 12;
                while (i + 1 < shape->iClusterCount && shape->pClusters[i + 1].iTextStart < cluster.iTextEnd) i++;
            }
            if (bytes - cluster.iTextStart >= 3 && !memcmp(text + cluster.iTextStart, "e\xcc\x81", 3)) {
                cluster.iTextEnd = cluster.iTextStart + 3; cluster.fAdvance = 11;
                while (i + 1 < shape->iClusterCount && shape->pClusters[i + 1].iTextStart < cluster.iTextEnd) i++;
            }
            shape->pClusters[count++] = cluster; width += cluster.fAdvance;
        }
        shape->iClusterCount = count; shape->fWidth = (float)width;
    }
    return result;
}
static int draw_only_text(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;

    if (draw_only_capture) {
        unsigned at = draw_only_calls++;
        CHECK(at < 16 && strlen(text) < sizeof(draw_only_journal[at].text));
        snprintf(draw_only_journal[at].text, sizeof(draw_only_journal[at].text), "%s", text);
        draw_only_journal[at].rect = rect; draw_only_journal[at].color = color;
        draw_only_journal[at].clipped = 0;
        if (proxy->drawClipGet) CHECK(proxy->drawClipGet(proxy, draw, &draw_only_journal[at].clip, &draw_only_journal[at].clipped) == XUI_OK);
        if (draw_only_error && !--draw_only_error) return XUI_ERROR_INVALID_STATE;
    }
    return draw_only_base_text(proxy, draw, pTextItem, rect, color, flags);
}
static int draw_only_clip_set(xui_proxy proxy, xui_draw_context draw, xui_rect_t rect)
{
    int result = draw_only_base_clip_set(proxy, draw, rect);
    if (result == XUI_OK && draw_only_clip_error && !--draw_only_clip_error) return XUI_ERROR_INVALID_STATE;
    return result;
}
static int draw_only_clip_get(xui_proxy proxy, xui_draw_context draw, xui_rect_t* rect, int* clipped)
{
    if (draw_only_get_error && !--draw_only_get_error) return XUI_ERROR_INVALID_STATE;
    return draw_only_base_clip_get(proxy, draw, rect, clipped);
}
static int draw_only_clip_clear(xui_proxy proxy, xui_draw_context draw)
{
    int result = draw_only_base_clip_clear(proxy, draw);
    if (result == XUI_OK && draw_only_clear_error && !--draw_only_clear_error) return XUI_ERROR_INVALID_STATE;
    return result;
}
static void document_draw_only(xui_test_proxy_state_t* proxy)
{
    const uint32_t blue = 0xff3366ffu, red = 0xffdd4433u;
    xui_proxy_t saved = proxy->tProxy; unsigned clipping, variant, pass;
    draw_only_base_shape = saved.textShape; draw_only_base_text = saved.drawText; draw_only_base_clip_set = saved.drawClipSet;
    draw_only_base_clip_get = saved.drawClipGet; draw_only_base_clip_clear = saved.drawClipClear;
    for (clipping = 0; clipping < 2; clipping++) {
        xui_context context; xui_font font; xui_surface surface; xui_surface_desc_t target = {0};
        proxy->tProxy.textShape = draw_only_shape; proxy->tProxy.drawText = draw_only_text;
        proxy->tProxy.drawTextSpans = NULL;
        proxy->tProxy.drawClipSet = clipping ? draw_only_clip_set : NULL;
        proxy->tProxy.drawClipGet = draw_only_clip_get; proxy->tProxy.drawClipClear = draw_only_clip_clear;
        CHECK(proxy->tProxy.fontLoadFile(&proxy->tProxy, &font, "draw-only.ttf", 20, 0) == XUI_OK &&
            xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
        target.iKind = XUI_SURFACE_KIND_TEXTURE; target.iFormat = XUI_SURFACE_FORMAT_RGBA8;
        target.iWidth = 320; target.iHeight = 200; target.iFlags = XUI_SURFACE_USAGE_TARGET;
        CHECK(proxy->tProxy.surfaceCreate(&proxy->tProxy, &surface, &target) == XUI_OK);
        for (draw_only_sample = 0; draw_only_sample < 3; draw_only_sample++) for (variant = 0; variant < 6; variant++) {
            const char* body = draw_only_sources[draw_only_sample]; size_t bytes = strlen(body), middle = draw_only_sample == 2 ? 3 : 1;
            xui_document document; xui_document_snapshot snapshot; xui_document_renderer renderer;
            xui_doc_renderer_desc_t desc = {0}; uint64_t paragraph, first, tail;
            xui_doc_position_t head, at, eof; xui_doc_rect_t origin, caret;
            if (variant >= 3) {
                xui_doc_desc_t profile = {0}; char markdown[40];
                profile.iSize = sizeof(profile); profile.iProfile = XUI_DOCUMENT_MARKDOWN;
                if (variant == 3) snprintf(markdown, sizeof(markdown), "**%.*s**%s", (int)middle, body, body + middle);
                else snprintf(markdown, sizeof(markdown), "%s", body);
                CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK && xuiDocumentLoadMarkdown(document, markdown, strlen(markdown)) == XUI_OK &&
                    xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK && xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK &&
                    xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &first) == XUI_OK);
                if (variant == 3) CHECK(xuiDocumentSnapshotGetChild(snapshot, paragraph, 1, &tail) == XUI_OK); else tail = first;
                xuiDocumentSnapshotRelease(snapshot);
            } else {
                xui_document_transaction transaction; char prefix[2] = {body[0], 0};
                CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
                paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
                first = decoration_insert(transaction, paragraph, variant ? prefix : body, XUI_DOC_UNDERLINE, blue);
                tail = variant ? decoration_insert(transaction, paragraph, body + 1, XUI_DOC_STRIKE, variant == 2 ? red : blue) : first;
                CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
            }
            head = decoration_position(document, first, 0);
            at = decoration_position(document, draw_only_sample == 2 && (variant == 1 || variant == 2) ? tail : first,
                draw_only_sample == 2 && (variant == 1 || variant == 2) ? 2 : middle);
            eof = decoration_position(document, tail, bytes - (variant == 1 || variant == 2 ? 1 : variant == 3 ? middle : 0));
            if (variant >= 4) { head.iKind = at.iKind = eof.iKind = XUI_DOC_POSITION_SOURCE;
                head.iNodeId = at.iNodeId = eof.iNodeId = 1; eof.iOffset = bytes; }
            desc.iSize = sizeof(desc); desc.fLineGap = 4; desc.iTextColor = blue;
            desc.tFonts = (xui_doc_font_set_t){font, font, font, font, font};
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK && xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
            if (variant >= 4) CHECK(xuiDocumentRendererSetMode(renderer, variant == 4 ? XUI_DOC_SOURCE_TEXT : XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
            if (variant == 5) CHECK(xuiDocumentRendererSetActivePosition(renderer, &at) == XUI_OK);
            for (pass = 0; pass < 3; pass++) {
                xui_draw_context draw; xui_rect_t clip = {0, 0, 300, 100}, restored; int has_clip;
                double advance = draw_only_sample == 0 ? 7 : draw_only_sample == 1 ? 6 : 11;
                int wrapped = pass == 1 && variant < 4, result;
                CHECK(xuiDocumentRendererLayout(renderer, pass == 1 ? draw_only_sample == 2 ? 22 : 20 : 200, 0, 200) == XUI_OK &&
                    xuiDocumentRendererGetCaretRect(renderer, &head, &origin) == XUI_OK && xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK);
                if (fabs(caret.x - origin.x - advance) >= .01) fprintf(stderr, "Draw-only caret clip=%u sample=%u variant=%u pass=%u got=%g want=%g\n", clipping, draw_only_sample, variant, pass, caret.x - origin.x, advance);
                CHECK(fabs(caret.x - origin.x - advance) < .01);
                {
                    xui_doc_position_t hit; xui_doc_range_t range = {head, at}; xui_doc_rect_t rects[2];
                    uint64_t count; int order;
                    CHECK(xuiDocumentRendererGetRangeRects(renderer, &range, rects, 2, &count) == XUI_OK && count == 1 &&
                        fabs(rects[0].width - advance) < .01 &&
                        xuiDocumentRendererHitTest(renderer, origin.x + advance - 1, origin.y + 10, &hit) == XUI_OK &&
                        xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                        xuiDocumentSnapshotComparePositions(snapshot, &hit, &at, &order) == XUI_OK && !order);
                    xuiDocumentSnapshotRelease(snapshot);
                }
                CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, surface) == XUI_OK);
                if (clipping) CHECK(proxy->tProxy.drawClipSet(&proxy->tProxy, draw, clip) == XUI_OK);
                draw_only_calls = 0; draw_only_capture = 1;
                result = xuiDocumentRendererDraw(renderer, draw, 0, 0, clip, NULL, 0); draw_only_capture = 0;
                if (!clipping && variant == 2) CHECK(result == XUI_ERROR_UNSUPPORTED && !draw_only_calls);
                else {
                    unsigned call; CHECK(result == XUI_OK && draw_only_calls == (wrapped ? 2u : 1u) + (variant == 2 ? 1u : 0u));
                    for (call = 0; call < draw_only_calls; call++) {
                        unsigned row = wrapped && call + 1 == draw_only_calls;
                        size_t length = row ? 1 : bytes - (wrapped ? 1 : 0);
                        const char* expected = row ? "X" : body;
                        double width = row ? 10 : (draw_only_sample == 0 ? 27 : draw_only_sample == 1 ? 22 : 31) - (wrapped ? 10 : 0);
                        CHECK(strlen(draw_only_journal[call].text) == length && !memcmp(draw_only_journal[call].text, expected, length) &&
                            fabs((double)draw_only_journal[call].rect.fX) < .01 && fabs((double)draw_only_journal[call].rect.fY - row * 24) < .01 &&
                            fabs((double)draw_only_journal[call].rect.fW - width) < .01);
                    }
                    if (variant == 2) CHECK(draw_only_journal[0].color == blue && draw_only_journal[1].color == red &&
                        draw_only_journal[0].clipped && draw_only_journal[1].clipped &&
                        draw_only_journal[0].clip.fW == (draw_only_sample == 2 ? 11 : advance) &&
                        draw_only_journal[1].clip.fX == (draw_only_sample == 2 ? 11 : advance));
                    if (clipping) CHECK(proxy->tProxy.drawClipGet(&proxy->tProxy, draw, &restored, &has_clip) == XUI_OK && has_clip && !memcmp(&clip, &restored, sizeof(clip)));
                }
                if (clipping && !pass && !draw_only_sample && variant == 2) {
                    static const unsigned errors[] = {1, 2, 3, 4, 6}; unsigned failure;
                    xui_rect_t original = {2, 3, 200, 80};
                    for (failure = 0; failure < sizeof(errors) / sizeof(*errors) + 1; failure++) {
                        CHECK(proxy->tProxy.drawClipSet(&proxy->tProxy, draw, original) == XUI_OK);
                        draw_only_calls = 0; draw_only_capture = 1;
                        if (failure < sizeof(errors) / sizeof(*errors)) draw_only_clip_error = errors[failure];
                        else draw_only_error = 2;
                        result = xuiDocumentRendererDraw(renderer, draw, 0, 0, clip, NULL, 0);
                        draw_only_capture = 0;
                        if (result != XUI_ERROR_INVALID_STATE) fprintf(stderr, "Draw-only failure=%u result=%d clip-left=%u text-left=%u\n", failure, result, draw_only_clip_error, draw_only_error);
                        CHECK(result == XUI_ERROR_INVALID_STATE && !draw_only_clip_error && !draw_only_error &&
                            proxy->tProxy.drawClipGet(&proxy->tProxy, draw, &restored, &has_clip) == XUI_OK && has_clip && !memcmp(&original, &restored, sizeof(original)));
                        CHECK(xuiDocumentRendererDraw(renderer, draw, 0, 0, clip, NULL, 0) == XUI_OK &&
                            proxy->tProxy.drawClipGet(&proxy->tProxy, draw, &restored, &has_clip) == XUI_OK && has_clip && !memcmp(&original, &restored, sizeof(original)));
                    }
                    draw_only_calls = 0; draw_only_capture = 1; draw_only_get_error = 1;
                    CHECK(xuiDocumentRendererDraw(renderer, draw, 0, 0, clip, NULL, 0) == XUI_ERROR_INVALID_STATE &&
                        !draw_only_get_error && !draw_only_calls);
                    draw_only_capture = 0;
                    CHECK(proxy->tProxy.drawClipGet(&proxy->tProxy, draw, &restored, &has_clip) == XUI_OK && has_clip && !memcmp(&original, &restored, sizeof(original)));
                    CHECK(proxy->tProxy.drawClipClear(&proxy->tProxy, draw) == XUI_OK);
                    draw_only_clear_error = 1;
                    CHECK(xuiDocumentRendererDraw(renderer, draw, 0, 0, clip, NULL, 0) == XUI_ERROR_INVALID_STATE && !draw_only_clear_error &&
                        proxy->tProxy.drawClipGet(&proxy->tProxy, draw, &restored, &has_clip) == XUI_OK && !has_clip);
                    CHECK(xuiDocumentRendererDraw(renderer, draw, 0, 0, clip, NULL, 0) == XUI_OK &&
                        proxy->tProxy.drawClipGet(&proxy->tProxy, draw, &restored, &has_clip) == XUI_OK && !has_clip);
                }
                CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
            }
            if (clipping && !draw_only_sample && variant == 1) {
                uint64_t revision = xuiDocumentGetRevision(document);
                draw_only_shape_error = 1;
                CHECK(xuiDocumentRendererInvalidateFonts(renderer) == XUI_OK &&
                    xuiDocumentRendererLayout(renderer, 20, 0, 200) == XUI_ERROR_OUT_OF_MEMORY &&
                    !draw_only_shape_error && xuiDocumentGetRevision(document) == revision);
                CHECK(xuiDocumentRendererLayout(renderer, 20, 0, 200) == XUI_OK &&
                    xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK && fabs(caret.x - 7) < .01 &&
                    xuiDocumentRendererGetCaretRect(renderer, &eof, &caret) == XUI_OK && fabs(caret.x - 10) < .01 && fabs(caret.y - 24) < .01);
            }
            xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
        }
        proxy->tProxy.surfaceDestroy(&proxy->tProxy, surface); xuiDestroy(context); proxy->tProxy.fontDestroy(&proxy->tProxy, font);
        proxy->tProxy = saved;
    }
    puts("DrawText-only Document: full-line kerning/ligatures/grapheme ownership, color clipping, reflow and no-clip capability passed");
}
