static void language_editor_query(xui_document document, const char* text, const char* expected,
    xui_doc_range_t* range)
{
    xui_document_snapshot snapshot; xui_doc_text_style_query_t query = {0}; uint64_t count;
    query.iSize = sizeof(query);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, text, strlen(text), NULL, range, 1, &count) == XUI_OK && count == 1 &&
        xuiDocumentSnapshotQueryTextStyle(snapshot, range, &query) == XUI_OK &&
        !strcmp(query.tStyle.sLanguage, expected) && !(query.iMixedFields & XUI_DOC_TEXT_STYLE_LANGUAGE));
    xuiDocumentSnapshotRelease(snapshot);
}
static void language_editor_cases(xui_context context)
{
    xui_document document; xui_document_snapshot snapshot; xui_document_transaction transaction;
    xui_widget editor; xui_doc_editor_desc_t desc = {0}; xui_doc_node_info_t root = {0};
    xui_doc_text_style_t style = {0}; xui_doc_editor_text_style_state_t state = {0}; xui_doc_range_t range;
    root.iSize = sizeof(root); style.iSize = sizeof(style); state.iSize = sizeof(state);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotGetNode(snapshot, 1, &root) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    root.tAttributes.sLanguage = "en";
    CHECK(xuiDocumentTxnSetAttributes(transaction, 1, &root.tAttributes) == XUI_OK && xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction); xuiDocumentSnapshotRelease(snapshot);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = document; desc.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK && xuiDocumentEditorInsertText(editor, "abc", 3) == XUI_OK);
    language_editor_query(document, "b", "en", &range);
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    strcpy(style.sLanguage, "TR-tr");
    CHECK(xuiDocumentEditorSetTextStyle(editor, XUI_DOC_TEXT_STYLE_LANGUAGE, &style) == XUI_OK);
    memset(style.sLanguage, 'x', sizeof(style.sLanguage));
    CHECK(xuiDocumentEditorQueryTextStyle(editor, &state) == XUI_OK && state.bEnabled &&
        !strcmp(state.tQuery.tStyle.sLanguage, "tr-tr"));
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    language_editor_query(document, "b", "en", &range);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    language_editor_query(document, "b", "tr-tr", &range);
    range.tAnchor = range.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    strcpy(style.sLanguage, "und");
    CHECK(xuiDocumentEditorSetTextStyle(editor, XUI_DOC_TEXT_STYLE_LANGUAGE, &style) == XUI_OK);
    memset(style.sLanguage, 'z', sizeof(style.sLanguage));
    CHECK(xuiDocumentEditorQueryTextStyle(editor, &state) == XUI_OK && !strcmp(state.tQuery.tStyle.sLanguage, "und") &&
        xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CLEAR_FORMATTING) == XUI_OK &&
        xuiDocumentEditorQueryTextStyle(editor, &state) == XUI_OK && !strcmp(state.tQuery.tStyle.sLanguage, "und") &&
        xuiDocumentEditorInsertText(editor, "X", 1) == XUI_OK);
    language_editor_query(document, "X", "und", &range);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CLEAR_FORMATTING) == XUI_OK &&
        xuiDocumentEditorQueryTextStyle(editor, &state) == XUI_OK && !strcmp(state.tQuery.tStyle.sLanguage, "und") &&
        xuiDocumentEditorInsertText(editor, "Y", 1) == XUI_OK);
    language_editor_query(document, "Y", "und", &range);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK &&
        xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    language_editor_query(document, "Y", "und", &range);
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    memset(style.sLanguage, 'q', sizeof(style.sLanguage));
    CHECK(xuiDocumentEditorSetTextStyle(editor, XUI_DOC_TEXT_STYLE_LANGUAGE, &style) == XUI_ERROR_INVALID_ARGUMENT);
    strcpy(style.sLanguage, "en--US");
    CHECK(xuiDocumentEditorSetTextStyle(editor, XUI_DOC_TEXT_STYLE_LANGUAGE, &style) == XUI_ERROR_INVALID_ARGUMENT);
    style.sLanguage[0] = 0;
    CHECK(xuiDocumentEditorSetTextStyle(editor, XUI_DOC_TEXT_STYLE_LANGUAGE, &style) == XUI_OK);
    language_editor_query(document, "Y", "en", &range);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    puts("DocumentEditor language: selection/pending ownership, inheritance reset, und, Undo/Redo, semantic ClearFormatting and invalid tags passed");
}
