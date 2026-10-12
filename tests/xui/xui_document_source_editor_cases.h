static void source_editor_set_offset(xui_widget editor, xui_doc_range_t origin, size_t offset)
{
    xui_doc_range_t range = origin;
    range.tAnchor.iOffset = range.tCaret.iOffset = offset;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
}
static void source_editor_same_source(xui_document document, const char* expected, size_t bytes)
{
    xui_document_snapshot snapshot; char* actual = malloc(bytes + 1); uint64_t length = 0;
    CHECK(actual && xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(snapshot, actual, bytes + 1, &length) == XUI_OK);
    CHECK(length == bytes && !memcmp(actual, expected, bytes));
    xuiDocumentSnapshotRelease(snapshot); free(actual);
}
static void source_editor_navigation_cases(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font; xui_widget editor;
    xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0}; xui_document document;
    xui_doc_range_t origin, selected; char sample[4096], expected[4096]; size_t bytes = 0, i;
    const char* prefix = "alpha beta\r\nflag \xf0\x9f\x87\xba\xf0\x9f\x87\xb8 and emoji \xf0\x9f\x91\xa8\xe2\x80\x8d\xf0\x9f\x91\xa9\n";
    const char* tail = "e\xcc\x81\r\nmore words!\n";
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    bytes = strlen(prefix); memcpy(sample, prefix, bytes);
    while (bytes < 1022) sample[bytes++] = 'a';
    memcpy(sample + bytes, tail, strlen(tail)); bytes += strlen(tail);
    while (bytes < 2050) sample[bytes++] = 'b';
    memcpy(sample + bytes, "\nlast\n", 6); bytes += 6; sample[bytes] = 0;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, sample, bytes) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView);
    ed.tView.pDocument = document; ed.iMode = XUI_DOC_SOURCE_TEXT;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 240}) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &origin) == XUI_OK);
    CHECK(origin.tCaret.iKind == XUI_DOC_POSITION_SOURCE);
    {
        int first = -1, last = -1;
        CHECK(xuiEditSetSelection(editor, 1040, 1010) == XUI_OK);
        CHECK(xuiEditGetSelection(editor, &first, &last) == XUI_OK && first == 1040 && last == 1010);
        CHECK(xuiEditSetSelection(editor, 1024, 1024) == XUI_DOC_ERROR_UTF8);
        CHECK(xuiEditGetSelection(editor, &first, &last) == XUI_OK && first == 1040 && last == 1010);
        CHECK(xuiEditSetSelection(editor, (int)bytes + 1, (int)bytes + 1) == XUI_ERROR_INVALID_ARGUMENT);
        CHECK(xuiEditGetText(editor) && !memcmp(xuiEditGetText(editor), sample, bytes + 1));
    }
    for (i = 0; i <= bytes; i++) {
        unsigned operation;
        if (i < bytes && ((unsigned char)sample[i] & 0xc0) == 0x80) continue;
        if (i % 157 && (i < 1018 || i > 1041) && i < bytes - 24) continue;
        for (operation = 0; operation < 4; operation++) {
            int right = operation & 1, word = operation & 2;
            int oracle = word ? (right ? xuiInternalTextWordNext(sample, (int)bytes, (int)i, XUI_INTERNAL_WORD_NATURAL) :
                xuiInternalTextWordPrev(sample, (int)bytes, (int)i, XUI_INTERNAL_WORD_NATURAL)) :
                (right ? xuiInternalTextGraphemeNext(sample, (int)bytes, (int)i) :
                xuiInternalTextGraphemePrev(sample, (int)bytes, (int)i));
            source_editor_set_offset(editor, origin, i);
            ordered_key(context, editor, right ? XUI_KEY_RIGHT : XUI_KEY_LEFT, word ? XUI_MOD_CTRL : 0);
            CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK);
            if (selected.tCaret.iOffset != (uint64_t)oracle)
                fprintf(stderr, "SOURCE navigation: at=%llu op=%u actual=%llu expected=%d\n",
                    (unsigned long long)i, operation, (unsigned long long)selected.tCaret.iOffset, oracle);
            CHECK(selected.tCaret.iKind == XUI_DOC_POSITION_SOURCE && selected.tCaret.iOffset == (uint64_t)oracle);
        }
    }
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
    {
        int first = -1, last = -1;
        CHECK(xuiEditSetSelection(editor, 1031, 1019) == XUI_OK);
        CHECK(xuiEditGetSelection(editor, &first, &last) == XUI_OK && first == 1031 && last == 1019);
    }
    for (i = 1019; i <= 1038; i++) {
        int oracle;
        if (((unsigned char)sample[i] & 0xc0) == 0x80) continue;
        source_editor_set_offset(editor, origin, i);
        oracle = xuiInternalTextGraphemePrev(sample, (int)bytes, (int)i);
        ordered_key(context, editor, XUI_KEY_LEFT, 0);
        CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK && selected.tCaret.iOffset == (uint64_t)oracle);
    }
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source_editor_set_offset(editor, origin, 1025);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK);
    memcpy(expected, sample, 1022); memcpy(expected + 1022, sample + 1025, bytes - 1025);
    bytes -= 3; expected[bytes] = 0; source_editor_same_source(document, expected, bytes);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source_editor_same_source(document, sample, bytes + 3);
    CHECK(xuiDocumentViewGetSelection(editor, &origin) == XUI_OK);
    {
        xui_event_t event = {0};
        event.iSize = sizeof(event); event.iType = XUI_EVENT_IME_COMPOSITION; event.pTarget = editor;
        event.bCompositionReplacementRange = 1;
        event.iCompositionReplacementStart = 1; event.iCompositionReplacementEnd = 2;
        event.iTextSize = 1; strcpy(event.sText, "Z");
        CHECK(xuiDispatchEvent(context, &event) == XUI_OK);
        memcpy(expected, sample, bytes + 4); expected[1] = 'Z';
        source_editor_same_source(document, expected, bytes + 3);
    }
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("SOURCE Editor: grapheme/word navigation matches full-text Unicode oracle across cache, CRLF and emoji; backspace and IME replacement passed.");
}
