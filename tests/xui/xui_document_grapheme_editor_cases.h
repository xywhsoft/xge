#include <math.h>

static xui_doc_position_t doc_grapheme_edit_position(xui_document document,
    uint64_t node, uint64_t offset)
{
    xui_doc_position_t position = {0};
    position.iSize = sizeof(position); position.iKind = XUI_DOC_POSITION_TEXT;
    position.iDocumentId = xuiDocumentGetIdentity(document);
    position.iRevision = xuiDocumentGetRevision(document);
    position.iNodeId = node; position.iOffset = offset;
    return position;
}
static void doc_grapheme_edit_plain(xui_document document, const char* expected)
{
    xui_document_snapshot snapshot; char* text; uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotCopyPlainText(snapshot, &text, &bytes) == XUI_OK &&
        bytes == strlen(expected) && !memcmp(text, expected, (size_t)bytes));
    xuiDocumentFreeBuffer(text); xuiDocumentSnapshotRelease(snapshot);
}
static void document_cross_node_grapheme_editor(xui_context context,
    xui_test_proxy_state_t* proxy)
{
    unsigned sample;
    for (sample = 0; sample < 2; sample++) {
        xui_document document; xui_document_transaction transaction;
        xui_doc_node_desc_t node = {0}; xui_doc_editor_desc_t desc = {0};
        xui_widget editor; uint64_t paragraph, nodes[4] = {0}; unsigned part;
        xui_doc_range_t selection, moved, range; xui_doc_position_t hit;
        xui_event_t key = {0}, pointer = {0}; xui_rect_t content;
        xui_font_metrics_t metrics = {0};
        const char* full = sample ? "\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x92\xbb" : "e\xcc\x81";
        const char* split[4] = {sample ? "\xf0\x9f\x91\xa9" : "e",
            sample ? "\xe2\x80\x8d" : "\xcc\x81", "\xf0\x9f\x92\xbb", "X"};
        unsigned parts = sample ? 4 : 3;
        if (!sample) split[2] = "X";
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
        CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND,
            &node, &paragraph) == XUI_OK);
        for (part = 0; part < parts; part++) {
            node.iKind = XUI_DOC_TEXT; node.sText = split[part]; node.iTextBytes = strlen(node.sText);
            node.tAttributes.iTextColor = part & 1 ? XUI_COLOR_RGBA(180, 20, 40, 255) : 0;
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph,
                XUI_DOCUMENT_APPEND, &node, &nodes[part]) == XUI_OK);
        }
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = document;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 200}) == XUI_OK &&
            xuiInputViewport(context, 320, 200) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        selection.tAnchor = selection.tCaret = doc_grapheme_edit_position(document, nodes[0], 0);
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor; key.iKey = XUI_KEY_RIGHT;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK && xuiDocumentViewGetSelection(editor, &moved) == XUI_OK);
        range.tAnchor = selection.tCaret; range.tCaret = moved.tCaret;
        {
            xui_document_snapshot snapshot; char* text; uint64_t bytes;
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotCopyRange(snapshot, &range, &text, &bytes) == XUI_OK &&
                bytes == strlen(full) && !memcmp(text, full, (size_t)bytes));
            xuiDocumentFreeBuffer(text); xuiDocumentSnapshotRelease(snapshot);
        }
        key.iKey = XUI_KEY_LEFT;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK && xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
            selection.tCaret.iNodeId == nodes[0] && selection.tCaret.iOffset == 0);
        key.iKey = XUI_KEY_RIGHT;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK);
        doc_grapheme_edit_plain(document, "X\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        doc_grapheme_edit_plain(document, sample ?
            "\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x92\xbbX\n" : "e\xcc\x81X\n");
        /* Pointer hit, editor selection and deletion share the renderer's
         * whole-grapheme endpoint rather than the middle style boundary. */
        content = xuiWidgetGetContentRect(editor);
        CHECK(proxy->tProxy.fontGetMetrics(&proxy->tProxy, xuiGetDefaultFont(context), &metrics) == XUI_OK);
        pointer.iSize = sizeof(pointer); pointer.pTarget = editor;
        pointer.iPointerId = 9; pointer.iPointerType = XUI_POINTER_TYPE_MOUSE;
        pointer.iButton = XUI_POINTER_BUTTON_LEFT;
        pointer.fX = content.fX + metrics.fSize * .5f * (sample ? 3 : 2) * .75f;
        pointer.fY = content.fY + metrics.fLineHeight * .5f;
        CHECK(xuiDocumentViewHitTest(editor, pointer.fX, pointer.fY, &hit) == XUI_OK &&
            hit.iNodeId == nodes[parts - 2] && hit.iOffset == strlen(split[parts - 2]));
        pointer.iType = XUI_EVENT_POINTER_DOWN; pointer.iButtons = XUI_POINTER_BUTTON_LEFT;
        CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
        pointer.iType = XUI_EVENT_POINTER_UP; pointer.iButtons = 0;
        CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK &&
            xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
            selection.tCaret.iNodeId == hit.iNodeId && selection.tCaret.iOffset == hit.iOffset &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK);
        doc_grapheme_edit_plain(document, "X\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        selection.tAnchor = selection.tCaret = doc_grapheme_edit_position(document, nodes[0], 0);
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_DELETE) == XUI_OK);
        doc_grapheme_edit_plain(document, "X\n");
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("Cross-node grapheme editor: arrow, pointer hit, backspace/delete and one-step undo passed");
}
static void document_unicode_control_editor(xui_context context, xui_test_proxy_state_t* proxy)
{
    static const char* const controls[] = {"\n", "\r", "\r\n", "\v", "\f",
        "\xc2\x85", "\xe2\x80\xa8", "\xe2\x80\xa9"};
    unsigned sample, source;
    xui_font_metrics_t metrics = {0};
    CHECK(proxy->tProxy.fontGetMetrics(&proxy->tProxy, xuiGetDefaultFont(context), &metrics) == XUI_OK);
    for (sample = 0; sample < sizeof(controls) / sizeof(*controls); sample++) for (source = 0; source < 2; source++) {
        xui_document document; xui_doc_desc_t profile = {0}; xui_doc_editor_desc_t desc = {0};
        xui_doc_range_t selection, moved; xui_doc_position_t hit; xui_widget editor;
        xui_event_t key = {0}; uint64_t node_id = 1; char text[16], expected[20]; xui_rect_t content;
        size_t length = strlen(controls[sample]);
        snprintf(text, sizeof(text), "A%sV", controls[sample]);
        profile.iSize = sizeof(profile); profile.iProfile = source ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK);
        if (source) CHECK(xuiDocumentLoadMarkdown(document, text, strlen(text)) == XUI_OK);
        else {
            xui_document_transaction transaction; xui_doc_node_desc_t node = {0}; uint64_t paragraph;
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
            CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
            node.iKind = XUI_DOC_TEXT; node.sText = text; node.iTextBytes = strlen(text);
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &node_id) == XUI_OK &&
                xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        }
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = document;
        desc.tView.tRenderer.iSize = sizeof(desc.tView.tRenderer); desc.tView.tRenderer.fLineGap = 4;
        desc.iMode = source ? XUI_DOC_SOURCE_TEXT : XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 200}) == XUI_OK &&
            xuiInputViewport(context, 320, 200) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        selection.tAnchor = selection.tCaret = doc_grapheme_edit_position(document, node_id, 1);
        if (source) selection.tAnchor.iKind = selection.tCaret.iKind = XUI_DOC_POSITION_SOURCE;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor; key.iKey = XUI_KEY_DOWN;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK && xuiDocumentViewGetSelection(editor, &moved) == XUI_OK &&
            moved.tCaret.iNodeId == node_id && moved.tCaret.iOffset == 2 + length);
        key.iKey = XUI_KEY_UP;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK && xuiDocumentViewGetSelection(editor, &moved) == XUI_OK &&
            moved.tCaret.iNodeId == node_id && moved.tCaret.iOffset == 1);
        content = xuiWidgetGetContentRect(editor);
        CHECK(xuiDocumentViewHitTest(editor, content.fX + metrics.fSize * .375f,
            content.fY + metrics.fLineHeight * 1.5f + (source ? 0 : 4), &hit) == XUI_OK &&
            hit.iNodeId == node_id && hit.iOffset == 2 + length);
        selection.tAnchor.iOffset = selection.tCaret.iOffset = 1 + length;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK);
        if (source) {
            xui_document_snapshot snapshot; uint64_t bytes;
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotCopySource(snapshot, expected, sizeof(expected), &bytes) == XUI_OK &&
                bytes == 2 && !memcmp(expected, "AV", 2)); xuiDocumentSnapshotRelease(snapshot);
        } else doc_grapheme_edit_plain(document, "AV\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        if (source) {
            xui_document_snapshot snapshot; uint64_t bytes;
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotCopySource(snapshot, expected, sizeof(expected), &bytes) == XUI_OK &&
                bytes == strlen(text) && !memcmp(expected, text, (size_t)bytes)); xuiDocumentSnapshotRelease(snapshot);
        } else { snprintf(expected, sizeof(expected), "%s\n", text); doc_grapheme_edit_plain(document, expected); }
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("Unicode mandatory editor controls: Rich/SOURCE vertical navigation, second-line hit, whole-control backspace and exact undo passed");
}

static void document_trailing_line_editor(xui_context context, xui_test_proxy_state_t* proxy)
{
    static const char* const controls[] = {"\n", "\r", "\r\n", "\v", "\f",
        "\xc2\x85", "\xe2\x80\xa8", "\xe2\x80\xa9"};
    unsigned sample, mode; xui_font_metrics_t metrics = {0};
    CHECK(proxy->tProxy.fontGetMetrics(&proxy->tProxy, xuiGetDefaultFont(context), &metrics) == XUI_OK);
    for (sample = 0; sample < sizeof(controls) / sizeof(*controls); sample++) for (mode = 0; mode < 5; mode++) {
        xui_document document; xui_doc_desc_t profile = {0}; xui_doc_editor_desc_t desc = {0}; xui_widget editor;
        xui_doc_range_t selection, moved; xui_doc_position_t hit; xui_event_t key = {0}; xui_rect_t content;
        uint64_t node_id = 1, paragraph = 0; char text[16], expected[24]; size_t length = strlen(controls[sample]);
        int source = mode == 1 || mode == 2;
        if (mode == 4 && sample) continue;
        snprintf(text, sizeof(text), "A%s", controls[sample]);
        profile.iSize = sizeof(profile); profile.iProfile = source ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK);
        if (source) CHECK(xuiDocumentLoadMarkdown(document, text, strlen(text)) == XUI_OK);
        else {
            xui_document_transaction transaction; xui_doc_node_desc_t node = {0}; uint64_t hard;
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            node.iSize = sizeof(node); node.iKind = mode == 3 ? XUI_DOC_CODE_BLOCK : XUI_DOC_PARAGRAPH;
            if (mode == 3) { node.sText = text; node.iTextBytes = strlen(text); }
            CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
            if (mode == 3) node_id = paragraph;
            else {
                node.iKind = XUI_DOC_TEXT; node.sText = mode == 4 ? "A" : text; node.iTextBytes = strlen(node.sText);
                CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &node_id) == XUI_OK);
                if (mode == 4) {
                    node.iKind = XUI_DOC_HARD_BREAK; node.sText = NULL; node.iTextBytes = 0;
                    CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &hard) == XUI_OK);
                }
            }
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        }
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = document;
        desc.tView.tRenderer.iSize = sizeof(desc.tView.tRenderer); desc.tView.tRenderer.fLineGap = 4;
        desc.iMode = mode == 1 ? XUI_DOC_SOURCE_TEXT : mode == 2 ? XUI_DOC_LIVE_MARKDOWN : XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 200}) == XUI_OK &&
            xuiInputViewport(context, 320, 200) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        selection.tAnchor = selection.tCaret = doc_grapheme_edit_position(document, node_id, 1);
        if (source) selection.tAnchor.iKind = selection.tCaret.iKind = XUI_DOC_POSITION_SOURCE;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor; key.iKey = XUI_KEY_DOWN;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK && xuiDocumentViewGetSelection(editor, &moved) == XUI_OK &&
            moved.tCaret.iNodeId == (mode == 4 ? paragraph : node_id) && moved.tCaret.iOffset == (mode == 4 ? 2 : 1 + length));
        key.iKey = XUI_KEY_UP;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK && xuiDocumentViewGetSelection(editor, &moved) == XUI_OK &&
            moved.tCaret.iNodeId == node_id && moved.tCaret.iOffset == 1);
        content = xuiWidgetGetContentRect(editor);
        CHECK(xuiDocumentViewHitTest(editor, content.fX + (mode == 3 ? 8 : 0) + 5,
            content.fY + metrics.fLineHeight * 1.5f + (source ? 0 : 4), &hit) == XUI_OK &&
            hit.iNodeId == (mode == 4 ? paragraph : node_id) && hit.iOffset == (mode == 4 ? 2 : 1 + length) &&
            hit.iKind == (source ? XUI_DOC_POSITION_SOURCE : mode == 4 ? XUI_DOC_POSITION_GAP : XUI_DOC_POSITION_TEXT));
        selection.tAnchor = selection.tCaret = hit;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
            xuiDocumentEditorInsertText(editor, "V", 1) == XUI_OK);
        if (source) {
            xui_document_snapshot snapshot; uint64_t bytes;
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotCopySource(snapshot, expected, sizeof(expected), &bytes) == XUI_OK &&
                bytes == strlen(text) + 1 && !memcmp(expected, text, strlen(text)) && expected[bytes - 1] == 'V');
            xuiDocumentSnapshotRelease(snapshot);
        } else { snprintf(expected, sizeof(expected), "%sV\n", text); doc_grapheme_edit_plain(document, expected); }
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK &&
            xuiUpdate(context, .016f) == XUI_OK);
        /* Rebuild the endpoint using the new revision, then remove the whole
         * terminal control. Structural HardBreak uses its editable parent gap. */
        selection.tAnchor = selection.tCaret = doc_grapheme_edit_position(document, mode == 4 ? paragraph : node_id,
            mode == 4 ? 2 : 1 + length);
        if (source) selection.tAnchor.iKind = selection.tCaret.iKind = XUI_DOC_POSITION_SOURCE;
        if (mode == 4) selection.tAnchor.iKind = selection.tCaret.iKind = XUI_DOC_POSITION_GAP;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK);
        if (source) {
            xui_document_snapshot snapshot; uint64_t bytes;
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotCopySource(snapshot, expected, sizeof(expected), &bytes) == XUI_OK &&
                bytes == 1 && expected[0] == 'A'); xuiDocumentSnapshotRelease(snapshot);
        } else doc_grapheme_edit_plain(document, "A\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        if (source) {
            xui_document_snapshot snapshot; uint64_t bytes;
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotCopySource(snapshot, expected, sizeof(expected), &bytes) == XUI_OK &&
                bytes == strlen(text) && !memcmp(expected, text, strlen(text))); xuiDocumentSnapshotRelease(snapshot);
        } else { snprintf(expected, sizeof(expected), "%s\n", text); doc_grapheme_edit_plain(document, expected); }
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("Document trailing-line editor: Rich/code/SOURCE/LIVE and HardBreak gap, vertical arrows, blank-row hit, input, whole-control backspace and undo passed");
}

static void document_blank_row_editor(xui_context context, xui_test_proxy_state_t* proxy)
{
    static const char* const controls[] = {"\n", "\r", "\r\n", "\v", "\f",
        "\xc2\x85", "\xe2\x80\xa8", "\xe2\x80\xa9"};
    unsigned sample, mode, row; xui_font_metrics_t metrics = {0};
    CHECK(proxy->tProxy.fontGetMetrics(&proxy->tProxy, xuiGetDefaultFont(context), &metrics) == XUI_OK);
    for (sample = 0; sample < sizeof(controls) / sizeof(*controls); sample++) for (mode = 0; mode < 5; mode++) {
        xui_document document; xui_doc_desc_t profile = {0}; xui_doc_editor_desc_t desc = {0}; xui_widget editor;
        uint64_t node_id = 1, paragraph = 0; char original[24], expected[32], buffer[32];
        size_t length = strlen(controls[sample]); int source = mode == 2 || mode == 3;
        if (mode == 4 && sample) continue;
        snprintf(original, sizeof(original), "%s%sAV", controls[sample], controls[sample]);
        profile.iSize = sizeof(profile); profile.iProfile = source ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK);
        if (source) CHECK(xuiDocumentLoadMarkdown(document, original, strlen(original)) == XUI_OK);
        else {
            xui_document_transaction transaction; xui_doc_node_desc_t node = {0}; uint64_t hard;
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            node.iSize = sizeof(node); node.iKind = mode == 1 ? XUI_DOC_CODE_BLOCK : XUI_DOC_PARAGRAPH;
            if (mode == 1) { node.sText = original; node.iTextBytes = strlen(original); }
            CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
            if (mode == 1) node_id = paragraph;
            else {
                if (mode == 4) {
                    node.iKind = XUI_DOC_HARD_BREAK;
                    CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &hard) == XUI_OK &&
                        xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &hard) == XUI_OK);
                }
                node.iKind = XUI_DOC_TEXT; node.sText = mode == 4 ? "AV" : original; node.iTextBytes = strlen(node.sText);
                CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &node_id) == XUI_OK);
            }
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        }
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = document;
        desc.tView.tRenderer.iSize = sizeof(desc.tView.tRenderer); desc.tView.tRenderer.fLineGap = 4;
        desc.iMode = mode == 2 ? XUI_DOC_SOURCE_TEXT : mode == 3 ? XUI_DOC_LIVE_MARKDOWN : XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 200}) == XUI_OK &&
            xuiInputViewport(context, 320, 200) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        for (row = 0; row < 2; row++) {
            xui_rect_t content = xuiWidgetGetContentRect(editor); xui_doc_position_t hit; xui_doc_range_t selection;
            xui_event_t pointer = {0}; xui_document_snapshot snapshot; uint64_t bytes;
            size_t at = row * length;
            /* Activate the source syntax row in LIVE before querying its hit. */
            if (source) {
                selection.tAnchor = selection.tCaret = doc_grapheme_edit_position(document, 1, at);
                selection.tAnchor.iKind = selection.tCaret.iKind = XUI_DOC_POSITION_SOURCE;
                CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
            }
            pointer.iSize = sizeof(pointer); pointer.pTarget = editor; pointer.iPointerId = 9;
            pointer.iPointerType = XUI_POINTER_TYPE_MOUSE; pointer.iButton = XUI_POINTER_BUTTON_LEFT;
            pointer.fX = content.fX + (mode == 1 ? 8 : 0) + 5;
            pointer.fY = content.fY + (row + .5f) * metrics.fLineHeight + (source ? 0 : row * 4);
            CHECK(xuiDocumentViewHitTest(editor, pointer.fX, pointer.fY, &hit) == XUI_OK &&
                hit.iKind == (source ? XUI_DOC_POSITION_SOURCE : mode == 4 ? XUI_DOC_POSITION_GAP : XUI_DOC_POSITION_TEXT) &&
                hit.iNodeId == (mode == 4 ? paragraph : node_id) && hit.iOffset == (mode == 4 ? row : at));
            pointer.iType = XUI_EVENT_POINTER_DOWN; pointer.iButtons = XUI_POINTER_BUTTON_LEFT;
            CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
            pointer.iType = XUI_EVENT_POINTER_UP; pointer.iButtons = 0;
            CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK && xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
                selection.tCaret.iNodeId == hit.iNodeId && selection.tCaret.iOffset == hit.iOffset &&
                xuiDocumentEditorInsertText(editor, "X", 1) == XUI_OK);
            memcpy(expected, original, at); expected[at] = 'X'; strcpy(expected + at + 1, original + at);
            if (source) {
                CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                    xuiDocumentSnapshotCopySource(snapshot, buffer, sizeof(buffer), &bytes) == XUI_OK &&
                    bytes == strlen(expected) && !memcmp(buffer, expected, (size_t)bytes)); xuiDocumentSnapshotRelease(snapshot);
            } else { strcat(expected, "\n"); doc_grapheme_edit_plain(document, expected); }
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
            if (source) {
                CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                    xuiDocumentSnapshotCopySource(snapshot, buffer, sizeof(buffer), &bytes) == XUI_OK &&
                    bytes == strlen(original) && !memcmp(buffer, original, (size_t)bytes)); xuiDocumentSnapshotRelease(snapshot);
            } else { snprintf(expected, sizeof(expected), "%s\n", original); doc_grapheme_edit_plain(document, expected); }
        }
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("Document blank-row editor: eight controls in Rich/code/SOURCE/LIVE and HardBreak gaps, pointer input on both empty rows and exact undo passed");
}

static void document_format_editor(xui_context context, xui_test_proxy_state_t* proxy)
{
    static const char* const formats[] = {"\xc2\xad", "\xe2\x80\x8b", "\xe2\x81\xa0", "\xef\xbb\xbf"};
    unsigned sample, mode; (void)proxy;
    for (sample = 0; sample < 4; sample++) for (mode = 0; mode < 4; mode++) {
        xui_document document; xui_document_snapshot snapshot; xui_doc_desc_t profile = {0};
        xui_doc_editor_desc_t desc = {0}; xui_widget editor; uint64_t paragraph, text_id = 1;
        xui_doc_range_t selection; char text[16], expected[24], copied[24]; size_t length = strlen(formats[sample]); uint64_t bytes;
        int source = mode == 1 || mode == 3;
        snprintf(text, sizeof(text), "A%sV", formats[sample]);
        profile.iSize = sizeof(profile); profile.iProfile = mode ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK);
        if (mode) {
            CHECK(xuiDocumentLoadMarkdown(document, text, strlen(text)) == XUI_OK && xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
            if (!source) CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &text_id) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
        } else {
            xui_document_transaction transaction; xui_doc_node_desc_t node = {0};
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
            CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
            node.iKind = XUI_DOC_TEXT; node.sText = text; node.iTextBytes = strlen(text);
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &text_id) == XUI_OK &&
                xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        }
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = document;
        desc.iMode = mode == 1 ? XUI_DOC_SOURCE_TEXT : mode == 3 ? XUI_DOC_LIVE_MARKDOWN : XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 200}) == XUI_OK && xuiInputViewport(context, 320, 200) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        selection.tAnchor = selection.tCaret = doc_grapheme_edit_position(document, text_id, 1 + length);
        if (source) selection.tAnchor.iKind = selection.tCaret.iKind = XUI_DOC_POSITION_SOURCE;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK && xuiDocumentEditorInsertText(editor, "X", 1) == XUI_OK);
        snprintf(expected, sizeof(expected), "A%sXV", formats[sample]);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        if (mode) CHECK(xuiDocumentSnapshotCopySource(snapshot, copied, sizeof(copied), &bytes) == XUI_OK && bytes == strlen(expected) && !memcmp(copied, expected, (size_t)bytes));
        xuiDocumentSnapshotRelease(snapshot);
        if (!mode) { strcat(expected, "\n"); doc_grapheme_edit_plain(document, expected); }
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        if (mode && !source) {
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK && xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &text_id) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
        }
        selection.tAnchor = selection.tCaret = doc_grapheme_edit_position(document, text_id, 1 + length);
        if (source) selection.tAnchor.iKind = selection.tCaret.iKind = XUI_DOC_POSITION_SOURCE;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK);
        if (mode) {
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK && xuiDocumentSnapshotCopySource(snapshot, copied, sizeof(copied), &bytes) == XUI_OK &&
                bytes == 2 && !memcmp(copied, "AV", 2)); xuiDocumentSnapshotRelease(snapshot);
        } else doc_grapheme_edit_plain(document, "AV\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        if (mode) {
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK && xuiDocumentSnapshotCopySource(snapshot, copied, sizeof(copied), &bytes) == XUI_OK &&
                bytes == strlen(text) && !memcmp(copied, text, (size_t)bytes)); xuiDocumentSnapshotRelease(snapshot);
        } else { snprintf(expected, sizeof(expected), "%s\n", text); doc_grapheme_edit_plain(document, expected); }
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("Document format editor: SHY/ZWSP/WJ/FEFF in Rich and Markdown VISUAL/SOURCE/LIVE, raw-offset input, whole-format backspace and byte-exact undo passed");
}
static void document_soft_wrap_editor(xui_context context, xui_test_proxy_state_t* proxy)
{
    unsigned variant; xui_font_metrics_t metrics = {0};
    CHECK(proxy->tProxy.fontGetMetrics(&proxy->tProxy, xuiGetDefaultFont(context), &metrics) == XUI_OK);
    for (variant = 0; variant < 3; variant++) {
        xui_document document; xui_doc_desc_t profile = {0}; xui_doc_editor_desc_t desc = {0}; xui_widget editor;
        xui_doc_range_t selection, moved; xui_document_snapshot snapshot; uint64_t first, paragraph;
        xui_event_t pointer = {0}, key = {0}; xui_rect_t content, caret;
        xui_widget_ime_rect_proc caret_rect; void* caret_user;
        profile.iSize = sizeof(profile); profile.iProfile = variant == 2 ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK);
        if (variant == 2) {
            CHECK(xuiDocumentLoadMarkdown(document, "AV AV AV", 8) == XUI_OK &&
                xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &first) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
        } else {
            xui_document_transaction transaction; xui_doc_node_desc_t node = {0}; uint64_t next; unsigned part;
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH; node.tAttributes.iFlags = XUI_DOC_SPACING_EXPLICIT;
            CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
            for (part = 0; part < (variant ? 3u : 1u); part++) {
                node.iKind = XUI_DOC_TEXT; node.sText = variant ? (part == 2 ? "AV" : "AV ") : "AV AV AV";
                node.iTextBytes = strlen(node.sText); node.tAttributes.iTextColor = part & 1 ? XUI_COLOR_RGBA(180, 20, 40, 255) : 0;
                CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &next) == XUI_OK);
                if (!part) first = next;
            }
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        }
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = document;
        desc.tView.tRenderer.iSize = sizeof(desc.tView.tRenderer); desc.tView.tRenderer.fLineGap = 4;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 200}) == XUI_OK &&
            xuiInputViewport(context, 320, 200) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        content = xuiWidgetGetContentRect(editor);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320 - content.fW + metrics.fLineHeight * 1.5f + .5f, 200}) == XUI_OK &&
            xuiUpdate(context, .016f) == XUI_OK &&
            xuiWidgetGetImeCandidateRect(editor, &caret_rect, &caret_user) == XUI_OK && caret_rect);
        content = xuiWidgetGetContentRect(editor);
        pointer.iSize = sizeof(pointer); pointer.pTarget = editor; pointer.iPointerId = 9;
        pointer.iPointerType = XUI_POINTER_TYPE_MOUSE; pointer.iButton = XUI_POINTER_BUTTON_LEFT;
        pointer.fX = content.fX + content.fW - .2f; pointer.fY = content.fY + metrics.fLineHeight * .5f;
        pointer.iType = XUI_EVENT_POINTER_DOWN; pointer.iButtons = XUI_POINTER_BUTTON_LEFT;
        CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK); pointer.iType = XUI_EVENT_POINTER_UP; pointer.iButtons = 0;
        CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK && xuiDocumentViewGetSelection(editor, &moved) == XUI_OK &&
            moved.tCaret.iNodeId == first && moved.tCaret.iOffset == 3 && moved.tCaret.iAffinity == XUI_DOC_BEFORE);
        caret = caret_rect(editor, caret_user); CHECK(caret.fY == floor(content.fY));
        CHECK(xuiDocumentEditorInsertText(editor, "X", 1) == XUI_OK); doc_grapheme_edit_plain(document, "AV XAV AV\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        doc_grapheme_edit_plain(document, "AV AV AV\n");
        selection.tAnchor = selection.tCaret = doc_grapheme_edit_position(document, first, 0);
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor; key.iKey = XUI_KEY_END;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK && xuiDocumentViewGetSelection(editor, &moved) == XUI_OK &&
            moved.tCaret.iNodeId == first && moved.tCaret.iOffset == 3 && moved.tCaret.iAffinity == XUI_DOC_BEFORE);
        caret = caret_rect(editor, caret_user); CHECK(caret.fY == floor(content.fY));
        key.iKey = XUI_KEY_DOWN; CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        caret = caret_rect(editor, caret_user); CHECK(caret.fY == floor(content.fY + metrics.fLineHeight + 4));
        key.iKey = XUI_KEY_UP; CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        caret = caret_rect(editor, caret_user); CHECK(caret.fY == floor(content.fY));
        key.iKey = XUI_KEY_PAGE_DOWN; CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        caret = caret_rect(editor, caret_user); CHECK(caret.fY == floor(content.fY + 2 * (metrics.fLineHeight + 4)));
        key.iKey = XUI_KEY_PAGE_UP;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK && xuiDocumentViewGetSelection(editor, &moved) == XUI_OK &&
            moved.tCaret.iOffset == 3 && moved.tCaret.iAffinity == XUI_DOC_BEFORE);
        caret = caret_rect(editor, caret_user); CHECK(caret.fY == floor(content.fY));
        key.iKey = XUI_KEY_DOWN; CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        key.iKey = XUI_KEY_HOME; CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        caret = caret_rect(editor, caret_user); CHECK(caret.fY == floor(content.fY + metrics.fLineHeight + 4) && caret.fX == floor(content.fX));
        key.iKey = XUI_KEY_END; CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        caret = caret_rect(editor, caret_user); CHECK(caret.fY == floor(content.fY + metrics.fLineHeight + 4));
        key.iKey = XUI_KEY_HOME; key.iModifiers = XUI_MOD_CTRL;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK && xuiDocumentViewGetSelection(editor, &moved) == XUI_OK &&
            moved.tCaret.iNodeId == first && moved.tCaret.iOffset == 0 && moved.tCaret.iAffinity == XUI_DOC_AFTER);
        key.iKey = XUI_KEY_END; CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        caret = caret_rect(editor, caret_user); CHECK(caret.fY == floor(content.fY + 2 * (metrics.fLineHeight + 4)));
        key.iKey = XUI_KEY_END; key.iModifiers = XUI_MOD_SHIFT;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK && xuiDispatchEvent(context, &key) == XUI_OK &&
            xuiDocumentViewGetSelection(editor, &moved) == XUI_OK &&
            moved.tAnchor.iOffset == 0 && moved.tCaret.iOffset == 3 && moved.tCaret.iAffinity == XUI_DOC_BEFORE);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("Document soft-wrap editor: Rich joined/split and Markdown visual, row-end pointer/input/undo, Home/End/Shift-End, Up/Down/Page and Ctrl document edges retain visual row passed");
}
