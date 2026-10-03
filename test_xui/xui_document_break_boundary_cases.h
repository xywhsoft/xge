/* Explicit affinity belongs to a visual boundary, independent of run splits. */
static void document_break_boundary_carets(xui_test_proxy_state_t* proxy)
{
    xui_context context; xui_font font; unsigned sample, variant, resize;
    xui_text_shape_proc saved_shape = proxy->tProxy.textShape;
    decoration_base_shape = saved_shape; proxy->tProxy.textShape = decoration_shape;
    CHECK(proxy->tProxy.fontLoadFile(&proxy->tProxy, &font, "boundary.ttf", 20, 0) == XUI_OK &&
        xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    for (sample = 0; sample < sizeof(document_mandatory_breaks) / sizeof(*document_mandatory_breaks); sample++)
    for (variant = 0; variant < 4; variant++) {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0}; xui_doc_node_desc_t node = {0};
        uint64_t paragraph, first, control, following, empty = 0; char joined[24];
        size_t length = strlen(document_mandatory_breaks[sample]), control_end = length;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH; node.tAttributes.iFlags = XUI_DOC_SPACING_EXPLICIT;
        CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
        if (!variant) {
            snprintf(joined, sizeof(joined), "AV%sAV", document_mandatory_breaks[sample]);
            first = control = following = decoration_insert(transaction, paragraph, joined, 0, 0);
            control_end += 2;
        } else {
            first = decoration_insert(transaction, paragraph, "AV", 0, 0);
            if (sample == 2) decoration_insert(transaction, paragraph, "\r", 0, 0);
            control = decoration_insert(transaction, paragraph, sample == 2 ? "\n" : document_mandatory_breaks[sample], 0, 0);
            if (sample == 2) control_end = 1;
            if (variant >= 2) {
                node.iKind = XUI_DOC_TEXT; node.sText = ""; node.iTextBytes = 0;
                node.tAttributes.fFontSize = 40;
                CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &empty) == XUI_OK);
            }
            following = variant == 3 ? empty : decoration_insert(transaction, paragraph, "AV", 0, 0);
        }
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        for (resize = 0; resize < 3; resize++) {
            xui_doc_position_t at, reference; xui_doc_rect_t before, after, expected_before, expected_after;
            CHECK(xuiDocumentRendererLayout(renderer, resize == 1 ? 12 : 200, 0, 200) == XUI_OK);
            at = decoration_position(document, control, control_end); at.iAffinity = XUI_DOC_BEFORE;
            reference = decoration_position(document, first, 2); reference.iAffinity = XUI_DOC_BEFORE;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &before) == XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer, &reference, &expected_before) == XUI_OK &&
                fabs(before.x - expected_before.x) < .01 && fabs(before.y - expected_before.y) < .01 &&
                fabs(before.height - expected_before.height) < .01);
            at.iAffinity = XUI_DOC_AFTER;
            reference = decoration_position(document, empty ? empty : following, variant ? 0 : control_end);
            reference.iAffinity = XUI_DOC_AFTER;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &after) == XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer, &reference, &expected_after) == XUI_OK &&
                fabs(after.x - expected_after.x) < .01 && fabs(after.y - expected_after.y) < .01 &&
                fabs(after.height - expected_after.height) < .01 && after.y > before.y);
            if (resize != 1) CHECK(fabs(before.x - 17) < .01 && fabs(before.y) < .01 &&
                fabs(after.x) < .01 && fabs(after.y - 24) < .01 && fabs(after.height - (empty ? 40 : 20)) < .01);
        }
        if (empty) {
            xui_document_change_set changes; xui_doc_position_t at; xui_doc_rect_t caret;
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
                xuiDocumentTxnDeleteNode(transaction, empty) == XUI_OK &&
                xuiDocumentTxnCommit(transaction, &changes) == XUI_OK); xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK &&
                xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
            at = decoration_position(document, control, control_end); at.iAffinity = XUI_DOC_AFTER;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK &&
                fabs(caret.x) < .01 && fabs(caret.y - 24) < .01 && fabs(caret.height - 20) < .01);
            CHECK(xuiDocumentUndo(document, &changes) == XUI_OK && xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK &&
                xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
            at = decoration_position(document, control, control_end); at.iAffinity = XUI_DOC_AFTER;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &caret) == XUI_OK && fabs(caret.height - 40) < .01);
        }
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    /* Blank rows have no glyph to win a hit-distance tie against the control.
     * Hit-before-control must remain on the row actually clicked. */
    for (sample = 0; sample < sizeof(document_mandatory_breaks) / sizeof(*document_mandatory_breaks); sample++)
    for (variant = 0; variant < 3; variant++) {
        xui_document document; xui_document_snapshot snapshot; xui_document_renderer renderer;
        xui_doc_desc_t profile = {0}; xui_doc_renderer_desc_t desc = {0}; uint64_t node_id = 1;
        size_t length = strlen(document_mandatory_breaks[sample]); char text[24]; unsigned row;
        profile.iSize = sizeof(profile); profile.iProfile = variant == 2 ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK);
        snprintf(text, sizeof(text), "%s%sAV", document_mandatory_breaks[sample], document_mandatory_breaks[sample]);
        if (variant == 2) CHECK(xuiDocumentLoadMarkdown(document, text, strlen(text)) == XUI_OK);
        else {
            xui_document_transaction transaction; xui_doc_node_desc_t node = {0}; uint64_t paragraph;
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            node.iSize = sizeof(node); node.iKind = variant ? XUI_DOC_CODE_BLOCK : XUI_DOC_PARAGRAPH;
            if (variant) { node.sText = text; node.iTextBytes = strlen(text); }
            CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
            node_id = variant ? paragraph : decoration_insert(transaction, paragraph, text, 0, 0);
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        }
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            (variant != 2 || xuiDocumentRendererSetMode(renderer, XUI_DOC_SOURCE_TEXT) == XUI_OK) &&
            xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        for (row = 0; row < 2; row++) {
            xui_doc_position_t hit; xui_doc_rect_t caret; double top = row * (variant == 2 ? 20 : 24);
            CHECK(xuiDocumentRendererHitTest(renderer, variant == 1 ? 13 : 5, top + 10, &hit) == XUI_OK &&
                hit.iNodeId == node_id && hit.iOffset == row * length &&
                xuiDocumentRendererGetCaretRect(renderer, &hit, &caret) == XUI_OK && fabs(caret.y - top) < .01);
        }
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    /* Structural blank rows must hit an input-capable parent gap. */
    {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_node_desc_t node = {0}; xui_doc_renderer_desc_t desc = {0};
        uint64_t paragraph, hard; unsigned row;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
        CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
        node.iKind = XUI_DOC_HARD_BREAK;
        CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &hard) == XUI_OK &&
            xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &hard) == XUI_OK &&
            xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
        for (row = 0; row < 3; row++) {
            xui_doc_position_t hit; xui_doc_rect_t caret;
            CHECK(xuiDocumentRendererHitTest(renderer, 5, row * 24 + 10, &hit) == XUI_OK &&
                hit.iKind == XUI_DOC_POSITION_GAP && hit.iNodeId == paragraph && hit.iOffset == row &&
                xuiDocumentRendererGetCaretRect(renderer, &hit, &caret) == XUI_OK && fabs(caret.y - row * 24) < .01);
        }
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    /* The downstream carrier on a prefix's uncommitted last row must be
     * extended before publishing a caret for the preceding control. */
    {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer sparse, full; xui_doc_renderer_desc_t desc = {0};
        xui_doc_renderer_stats_t initial = {0}, queried = {0}; xui_doc_position_t at;
        xui_doc_rect_t caret, expected; uint64_t paragraph, control; char* text = malloc(65537); size_t i;
        CHECK(text); for (i = 0; i < 65535; i += 3) memcpy(text + i, "AV ", 3); text[65535] = 0;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
        text[8190] = 0; decoration_insert(transaction, paragraph, text, 0, 0); text[8190] = 'A';
        control = decoration_insert(transaction, paragraph, "\n", 0, 0);
        decoration_insert(transaction, paragraph, "A ", 0, 0);
        decoration_insert(transaction, paragraph, text, 0, 0);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &sparse) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &full) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(sparse, snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(full, snapshot, NULL) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
        initial.iSize = queried.iSize = sizeof(initial);
        CHECK(xuiDocumentRendererLayout(sparse, 200, 0, 60) == XUI_OK &&
            xuiDocumentRendererGetStats(sparse, &initial) == XUI_OK && initial.iFragments < 10000 &&
            initial.iShapedBytes < initial.iFragments * 4);
        at = decoration_position(document, control, 1); at.iAffinity = XUI_DOC_AFTER;
        CHECK(xuiDocumentRendererGetCaretRect(sparse, &at, &caret) == XUI_OK &&
            xuiDocumentRendererGetStats(sparse, &queried) == XUI_OK && queried.iShapedBytes > initial.iShapedBytes &&
            queried.iFragments < 32768 && queried.iShapedBytes < queried.iFragments * 4 &&
            xuiDocumentRendererLayout(full, 200, 0, 1000000) == XUI_OK &&
            xuiDocumentRendererGetCaretRect(full, &at, &expected) == XUI_OK &&
            fabs(caret.x - expected.x) < .01 && fabs(caret.y - expected.y) < .01 && fabs(caret.height - expected.height) < .01);
        xuiDocumentRendererRelease(sparse); xuiDocumentRendererRelease(full); xuiDocumentRelease(document); free(text);
    }
    xuiDestroy(context); proxy->tProxy.fontDestroy(&proxy->tProxy, font); proxy->tProxy.textShape = saved_shape;
    puts("Mandatory boundary carets: cross-node affinity, CRLF, empty fonts/mutation/undo, blank-row hit identity and structural input gaps passed");
}
