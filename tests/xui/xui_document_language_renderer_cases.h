/* History is disabled so cached draw requests cannot accidentally borrow a
 * language kept alive by Undo. This also runs under Linux ASan/UBSan. */
static xui_text_shape_proc language_lifetime_base_shape;
static xui_draw_text_proc language_lifetime_base_draw;
static xui_draw_text_spans_proc language_lifetime_base_spans;
static unsigned language_lifetime_shapes[3], language_lifetime_draws[3];
static void language_lifetime_check(const xui_text_item_t* item, int draw)
{
    if (!item->iTextSize || !item->sText[0]) return;
    CHECK(item->sLanguage);
    unsigned index = !strcmp(item->sLanguage, "en") ? 0 : !strcmp(item->sLanguage, "tr") ? 1 : 2;
    CHECK(index != 2 || !strcmp(item->sLanguage, "und"));
    (draw ? language_lifetime_draws : language_lifetime_shapes)[index]++;
}
static int language_lifetime_shape(xui_proxy proxy, const xui_text_item_t* item, xui_text_shape_t* shape)
{ language_lifetime_check(item, 0); return language_lifetime_base_shape(proxy, item, shape); }
static int language_lifetime_draw(xui_proxy proxy, xui_draw_context dc, const xui_text_item_t* item,
    xui_rect_t rect, uint32_t color, uint32_t flags)
{ language_lifetime_check(item, 1); return language_lifetime_base_draw(proxy, dc, item, rect, color, flags); }
static int language_lifetime_spans(xui_proxy proxy, xui_draw_context dc, const xui_text_item_t* item,
    xui_rect_t rect, uint32_t color, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{ language_lifetime_check(item, 1); return language_lifetime_base_spans(proxy, dc, item, rect, color, flags, spans, count); }
static void language_lifetime_root(xui_document document, const char* tag, xui_document_renderer renderer)
{
    xui_document_snapshot snapshot; xui_document_transaction transaction; xui_document_change_set changes;
    xui_doc_node_info_t info = {0}; info.iSize = sizeof(info);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotGetNode(snapshot, 1, &info) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    info.tAttributes.sLanguage = tag;
    CHECK(xuiDocumentTxnSetAttributes(transaction, 1, &info.tAttributes) == XUI_OK &&
        xuiDocumentTxnCommit(transaction, &changes) == XUI_OK);
    xuiDocumentTxnRelease(transaction); xuiDocumentSnapshotRelease(snapshot);
    if (renderer) {
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
    }
    xuiDocumentChangeSetRelease(changes);
}
static void document_language_lifetime(xui_test_proxy_state_t* proxy)
{
    xui_text_shape_proc shape = proxy->tProxy.textShape; xui_draw_text_proc draw = proxy->tProxy.drawText;
    xui_draw_text_spans_proc spans = proxy->tProxy.drawTextSpans;
    language_lifetime_base_shape = shape; language_lifetime_base_draw = draw; language_lifetime_base_spans = spans;
    proxy->tProxy.textShape = language_lifetime_shape; proxy->tProxy.drawText = language_lifetime_draw;
    for (unsigned backend = 0; backend < 2; backend++) {
        xui_context context; xui_font font; xui_surface surface; xui_document document;
        xui_document_transaction transaction; xui_document_snapshot snapshot; xui_document_renderer renderer;
        xui_doc_desc_t desc = {0}; xui_doc_node_desc_t node = {0}; uint64_t paragraph, leaf;
        proxy->tProxy.drawTextSpans = backend ? NULL : language_lifetime_spans;
        CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK &&
            proxy->tProxy.fontLoadFile(&proxy->tProxy, &font, "language.ttf", 20, 0) == XUI_OK &&
            xuiSetDefaultFont(context, font) == XUI_OK && xuiTestSurfaceCreate(proxy, &surface, 320, 160, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_RICH; desc.bDisableHistory = 1;
        CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        xui_doc_attributes_t attrs = {0}; attrs.sLanguage = "en";
        CHECK(xuiDocumentTxnSetAttributes(transaction, 1, &attrs) == XUI_OK);
        node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
        CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
        for (unsigned i = 0; i < 3; i++) {
            node.iKind = XUI_DOC_TEXT; node.sText = !i ? "i\xe2\x80\x8d" : "i"; node.iTextBytes = strlen(node.sText);
            node.tAttributes.sLanguage = i == 1 ? "tr" : i == 2 ? "und" : NULL;
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &leaf) == XUI_OK);
        }
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        for (unsigned pass = 0; pass < 4; pass++) {
            xui_draw_context dc;
            if (pass == 1) {
                xui_doc_node_info_t info = {0}; xui_document_change_set changes; info.iSize = sizeof(info);
                CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                    xuiDocumentSnapshotGetNode(snapshot, leaf, &info) == XUI_OK &&
                    xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
                info.tAttributes.iTextColor = 0xabcdefFFu;
                CHECK(xuiDocumentTxnSetAttributes(transaction, leaf, &info.tAttributes) == XUI_OK &&
                    xuiDocumentTxnCommit(transaction, &changes) == XUI_OK);
                xuiDocumentTxnRelease(transaction); xuiDocumentSnapshotRelease(snapshot);
                CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                    xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK);
                xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
            }
            if (pass == 2) { xuiDocumentRelease(document); CHECK(xuiDocumentRendererInvalidateFonts(renderer) == XUI_OK); }
            memset(language_lifetime_shapes, 0, sizeof(language_lifetime_shapes)); memset(language_lifetime_draws, 0, sizeof(language_lifetime_draws));
            CHECK(xuiDocumentRendererLayout(renderer, pass == 3 ? 180 : 320, 0, 160) == XUI_OK &&
                proxy->tProxy.drawBegin(&proxy->tProxy, &dc, surface) == XUI_OK &&
                xuiDocumentRendererDraw(renderer, dc, 0, 0, (xui_rect_t){0, 0, 320, 160}, NULL, 0) == XUI_OK &&
                proxy->tProxy.drawEnd(&proxy->tProxy, dc) == XUI_OK);
            for (unsigned i = 0; i < 3; i++) { CHECK(language_lifetime_draws[i]); if (!pass) CHECK(language_lifetime_shapes[i]); }
        }
        xuiDocumentRendererRelease(renderer);
        /* A no-op preview shares the committed source. A metadata commit
         * must not promote its stale language-dependent cached rows. */
        desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, "i\xe2\x80\x8dii", 6) == XUI_OK);
        language_lifetime_root(document, "en", NULL);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererSetMode(renderer, XUI_DOC_SOURCE_TEXT) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        xui_document_prepare prepare; xui_doc_source_patch_t patch = {0};
        patch.iSize = sizeof(patch); patch.sText = "";
        CHECK(xuiDocumentPrepareSource(document, NULL, &patch, 1, &prepare) == XUI_OK &&
            xuiDocumentRendererSetSourceInput(renderer, prepare) == XUI_OK &&
            xuiDocumentRendererLayout(renderer, 320, 0, 160) == XUI_OK);
        language_lifetime_root(document, "tr", renderer);
        memset(language_lifetime_draws, 0, sizeof(language_lifetime_draws));
        xui_draw_context dc;
        CHECK(xuiDocumentRendererLayout(renderer, 320, 0, 160) == XUI_OK &&
            proxy->tProxy.drawBegin(&proxy->tProxy, &dc, surface) == XUI_OK &&
            xuiDocumentRendererDraw(renderer, dc, 0, 0, (xui_rect_t){0, 0, 320, 160}, NULL, 0) == XUI_OK &&
            proxy->tProxy.drawEnd(&proxy->tProxy, dc) == XUI_OK);
        CHECK(language_lifetime_draws[1] && !language_lifetime_draws[0] && !language_lifetime_draws[2]);
        xuiDocumentPrepareRelease(prepare); xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
        proxy->tProxy.surfaceDestroy(&proxy->tProxy, surface);
        xuiDestroy(context); proxy->tProxy.fontDestroy(&proxy->tProxy, font);
    }
    proxy->tProxy.textShape = shape; proxy->tProxy.drawText = draw; proxy->tProxy.drawTextSpans = spans;
    puts("Document renderer language lifetime: no-history colour-only attr release, both draw backends, released Document, font invalidation and reflow passed");
}
