static void document_soft_wrap_affinity(xui_test_proxy_state_t* proxy)
{
    xui_context context; xui_font font; xui_text_shape_proc saved_shape = proxy->tProxy.textShape;
    unsigned variant, pass;
    decoration_base_shape = saved_shape; proxy->tProxy.textShape = decoration_shape;
    CHECK(proxy->tProxy.fontLoadFile(&proxy->tProxy, &font, "soft-affinity.ttf", 20, 0) == XUI_OK &&
        xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    for (variant = 0; variant < 4; variant++) {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0}; xui_doc_node_desc_t node = {0};
        uint64_t paragraph, first, next, empty = 0;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH; node.tAttributes.iFlags = XUI_DOC_SPACING_EXPLICIT;
        CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
        first = decoration_insert(transaction, paragraph, variant ? "AV " : "AV AV", 0, XUI_COLOR_RGBA(20, 40, 180, 255));
        if (variant == 3) {
            node.iKind = XUI_DOC_TEXT; node.sText = ""; node.tAttributes.fFontSize = 40;
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &empty) == XUI_OK);
        }
        if (variant) {
            node.iKind = XUI_DOC_TEXT; node.sText = "AV"; node.iTextBytes = 2;
            node.tAttributes.fFontSize = variant == 2 ? 40 : 0;
            node.tAttributes.iTextColor = XUI_COLOR_RGBA(180, 20, 40, 255);
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &next) == XUI_OK);
        } else next = first;
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
        for (pass = 0; pass < 4; pass++) {
            xui_doc_position_t at = decoration_position(document, first, 3), target, hit;
            xui_doc_rect_t before, after, reference, clicked, empty_before = {0}; double width = pass == 1 ? 27 : pass == 2 ? 12 : 200;
            CHECK(xuiDocumentRendererLayout(renderer, width, 0, 200) == XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer, &at, &before) == XUI_OK);
            target = decoration_position(document, next, variant ? 0 : 3); target.iAffinity = XUI_DOC_AFTER;
            at.iAffinity = XUI_DOC_AFTER;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &after) == XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer, &target, &reference) == XUI_OK);
            if (empty) {
                xui_doc_position_t zero = decoration_position(document, empty, 0);
                CHECK(xuiDocumentRendererGetCaretRect(renderer, &zero, &empty_before) == XUI_OK);
                /* An empty Text already on a new row is itself the next
                 * insertion carrier. Only skip empties still on the old row. */
                if (empty_before.y + 32 > before.y + 16 + .01) reference = empty_before;
            }
            if (pass == 1 || pass == 2) CHECK(after.y > before.y && fabs(after.x - reference.x) < .01 &&
                fabs(after.y - reference.y) < .01 && fabs(after.height - reference.height) < .01);
            else CHECK(fabs(after.x - before.x) < .01 && fabs(after.y - before.y) < .01 && fabs(after.height - before.height) < .01);
            /* Row-end pointer results must survive GetCaretRect without
             * jumping down due to the default downstream affinity. */
            if (pass == 1 || pass == 2) CHECK(xuiDocumentRendererHitTest(renderer, before.x + 100, before.y + before.height * .5, &hit) == XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer, &hit, &clicked) == XUI_OK &&
                fabs(clicked.y - before.y) < .01);
            if (empty && (pass == 1 || pass == 2)) {
                xui_doc_position_t zero = decoration_position(document, empty, 0);
                xui_doc_rect_t upstream, downstream;
                CHECK(xuiDocumentRendererGetCaretRect(renderer, &zero, &upstream) == XUI_OK && fabs(upstream.height - 40) < .01);
                zero.iAffinity = XUI_DOC_AFTER;
                CHECK(xuiDocumentRendererGetCaretRect(renderer, &zero, &downstream) == XUI_OK && fabs(downstream.height - 40) < .01);
                if (pass == 1) CHECK(fabs(downstream.x - reference.x) < .01 &&
                    fabs(downstream.y - (reference.y + 16 - 32)) < .01 && downstream.y > upstream.y);
                else CHECK(fabs(downstream.x - upstream.x) < .01 && fabs(downstream.y - upstream.y) < .01 &&
                    upstream.y > before.y);
            }
        }
        if (empty) {
            xui_document_change_set changes; xui_doc_position_t at; xui_doc_rect_t original, removed, restored;
            CHECK(xuiDocumentRendererLayout(renderer, 27, 0, 200) == XUI_OK);
            at = decoration_position(document, first, 3); at.iAffinity = XUI_DOC_AFTER;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &original) == XUI_OK &&
                xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
                xuiDocumentTxnDeleteNode(transaction, empty) == XUI_OK && xuiDocumentTxnCommit(transaction, &changes) == XUI_OK);
            xuiDocumentTxnRelease(transaction);
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK &&
                xuiDocumentRendererLayout(renderer, 27, 0, 200) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
            at = decoration_position(document, first, 3); at.iAffinity = XUI_DOC_AFTER;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &removed) == XUI_OK &&
                fabs(removed.x) < .01 && fabs(removed.y - 24) < .01 && fabs(removed.height - 20) < .01 && removed.y < original.y &&
                xuiDocumentUndo(document, &changes) == XUI_OK && xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK &&
                xuiDocumentRendererLayout(renderer, 27, 0, 200) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
            at = decoration_position(document, first, 3); at.iAffinity = XUI_DOC_AFTER;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &restored) == XUI_OK &&
                fabs(restored.x - original.x) < .01 && fabs(restored.y - original.y) < .01 && fabs(restored.height - original.height) < .01);
        }
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    /* A table is one renderer block, but each paragraph/cell owns a separate
     * inline flow. Its following row must never capture the previous EOF. */
    {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer renderer; xui_doc_node_desc_t node = {0}; xui_doc_renderer_desc_t desc = {0};
        uint64_t table, row, cell, paragraph, ids[3]; unsigned part;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        node.iSize = sizeof(node); node.iKind = XUI_DOC_TABLE;
        CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &table) == XUI_OK);
        node.iKind = XUI_DOC_ROW;
        CHECK(xuiDocumentTxnInsertNode(transaction, table, XUI_DOCUMENT_APPEND, &node, &row) == XUI_OK);
        for (part = 0; part < 3; part++) {
            if (part != 1) {
                node.iKind = XUI_DOC_CELL;
                CHECK(xuiDocumentTxnInsertNode(transaction, row, XUI_DOCUMENT_APPEND, &node, &cell) == XUI_OK);
            }
            paragraph = decoration_insert(transaction, cell, NULL, 0, 0);
            ids[part] = decoration_insert(transaction, paragraph, "AV", 0, 0);
        }
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.fLineGap = 4;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererLayout(renderer, 200, 0, 200) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
        for (part = 0; part < 3; part++) {
            xui_doc_position_t at = decoration_position(document, ids[part], 2); xui_doc_rect_t before, after;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &before) == XUI_OK); at.iAffinity = XUI_DOC_AFTER;
            CHECK(xuiDocumentRendererGetCaretRect(renderer, &at, &after) == XUI_OK &&
                fabs(before.x - after.x) < .01 && fabs(before.y - after.y) < .01 && fabs(before.height - after.height) < .01);
        }
        xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    }
    /* The next soft-wrapped row at the projection's byte cut is provisional.
     * A cold downstream query must extend it before returning its geometry. */
    {
        xui_document document; xui_document_transaction transaction; xui_document_snapshot snapshot;
        xui_document_renderer sparse, full; xui_doc_renderer_desc_t desc = {0};
        xui_doc_renderer_stats_t initial = {0}, queried = {0}; xui_doc_position_t at, target;
        xui_doc_rect_t caret, expected; uint64_t paragraph, first, next; char* text = malloc(65537); size_t i;
        CHECK(text); for (i = 0; i < 65535; i += 3) memcpy(text + i, "AV ", 3); text[65535] = 0;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        paragraph = decoration_insert(transaction, 1, NULL, 0, 0);
        text[8190] = 0; first = decoration_insert(transaction, paragraph, text, 0, 0); text[8190] = 'A';
        next = decoration_insert(transaction, paragraph, text, 0, XUI_COLOR_RGBA(180, 20, 40, 255));
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
        at = decoration_position(document, first, 8190); at.iAffinity = XUI_DOC_AFTER;
        target = decoration_position(document, next, 0); target.iAffinity = XUI_DOC_AFTER;
        CHECK(xuiDocumentRendererGetCaretRect(sparse, &at, &caret) == XUI_OK &&
            xuiDocumentRendererGetStats(sparse, &queried) == XUI_OK && queried.iShapedBytes > initial.iShapedBytes &&
            queried.iFragments < 32768 && queried.iShapedBytes < queried.iFragments * 4 &&
            xuiDocumentRendererLayout(full, 200, 0, 1000000) == XUI_OK &&
            xuiDocumentRendererGetCaretRect(full, &target, &expected) == XUI_OK &&
            fabs(caret.x - expected.x) < .01 && fabs(caret.y - expected.y) < .01 && fabs(caret.height - expected.height) < .01);
        printf("Soft-wrap joint prefix: initial=%llu shaped/%llu fragments, extended=%llu shaped/%llu fragments, total source=73725\n",
            (unsigned long long)initial.iShapedBytes, (unsigned long long)initial.iFragments,
            (unsigned long long)queried.iShapedBytes, (unsigned long long)queried.iFragments);
        xuiDocumentRendererRelease(sparse); xuiDocumentRendererRelease(full); xuiDocumentRelease(document); free(text);
    }
    xuiDestroy(context); proxy->tProxy.fontDestroy(&proxy->tProxy, font); proxy->tProxy.textShape = saved_shape;
    puts("Soft-wrap affinity: same/split colors, mixed fonts, empty styled carriers/mutation/undo, row-end hit identity, paragraph/cell scope and cold prefix continuation passed");
}
