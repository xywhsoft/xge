/* Integrated with the public core suite. No renderer or OS dependency. */
static void language_remove_empty_fields(char* json, uint64_t* bytes)
{
    const char field[] = ",\"language\":\"\"";
    char* at;
    while ((at = strstr(json, field)) != NULL) {
        size_t n = sizeof(field) - 1;
        memmove(at, at + n, strlen(at + n) + 1); *bytes -= n;
    }
}
static void language_set(xui_document document, uint64_t node, const char* language)
{
    xui_document_snapshot snapshot;
    xui_document_transaction transaction;
    xui_doc_node_info_t info = {0}; info.iSize = sizeof(info);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, node, &info) == XUI_OK);
    info.tAttributes.sLanguage = language;
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    CHECK(xuiDocumentTxnSetAttributes(transaction, node, &info.tAttributes) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction); xuiDocumentSnapshotRelease(snapshot);
}
static void language_expect_root(xui_document document, const char* expected)
{
    xui_document_snapshot snapshot; xui_doc_node_info_t info = {0};
    info.iSize = sizeof(info);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, 1, &info) == XUI_OK &&
        !strcmp(info.tAttributes.sLanguage ? info.tAttributes.sLanguage : "", expected));
    xuiDocumentSnapshotRelease(snapshot);
}
static void language_query(xui_document document, const char* text,
    const char* expected, uint32_t mixed)
{
    xui_doc_range_t range = test_find(document, text);
    xui_document_snapshot snapshot; xui_doc_text_style_query_t query = {0};
    query.iSize = sizeof(query);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotQueryTextStyle(snapshot, &range, &query) == XUI_OK &&
        query.bHasText && !strcmp(query.tStyle.sLanguage, expected) &&
        (query.iMixedFields & XUI_DOC_TEXT_STYLE_LANGUAGE) == mixed);
    xuiDocumentSnapshotRelease(snapshot);
}
static void document_language_contract(void)
{
    xui_document document, loaded = NULL, markdown;
    xui_document_transaction transaction;
    xui_document_snapshot snapshot, kept;
    xui_doc_node_info_t info = {0}; xui_doc_text_style_t style = {0};
    xui_doc_range_t range; xui_document_fragment fragment;
    uint64_t paragraph, text, bytes, html_bytes, total;
    char *native = NULL, *html = NULL, *borrowed = malloc(16), tag[257];
    xui_doc_markdown_loss_t loss[8];
    CHECK(borrowed); strcpy(borrowed, "EN-us");
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    paragraph = insert(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
    text = insert(transaction, paragraph, XUI_DOC_TEXT, "iii");
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction);
    language_set(document, 1, borrowed);
    memset(borrowed, 'z', 15); free(borrowed);
    language_query(document, "iii", "en-us", 0);
    language_set(document, paragraph, "TR-tr");
    language_query(document, "iii", "tr-tr", 0);
    CHECK(xuiDocumentAcquireSnapshot(document, &kept) == XUI_OK);
    language_set(document, text, "und"); language_query(document, "iii", "und", 0);
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK); language_query(document, "iii", "tr-tr", 0);
    CHECK(xuiDocumentRedo(document, NULL) == XUI_OK); language_query(document, "iii", "und", 0);
    language_set(document, text, ""); language_query(document, "iii", "tr-tr", 0);
    range = test_find(document, "iii"); range.tCaret.iOffset = 1;
    style.iSize = sizeof(style); strcpy(style.sLanguage, "EN");
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
        xuiDocumentTxnSetTextStyle(transaction, &range, XUI_DOC_TEXT_STYLE_LANGUAGE, &style) == XUI_OK &&
        xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction);
    memset(style.sLanguage, 'x', sizeof(style.sLanguage)); /* Input is not retained. */
    language_query(document, "iii", "en", XUI_DOC_TEXT_STYLE_LANGUAGE);
    range = test_find(document, "iii");
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
        xuiDocumentTxnClearFormatting(transaction, &range) == XUI_OK &&
        xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction);
    language_query(document, "iii", "en", XUI_DOC_TEXT_STYLE_LANGUAGE); /* Language is semantic. */
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSerialize(snapshot, &native, &bytes) == XUI_OK &&
        strstr(native, "\"schemaVersion\":7") && strstr(native, "\"language\":\"tr-tr\"") &&
        xuiDocumentDeserialize(NULL, native, bytes, &loaded) == XUI_OK);
    language_query(loaded, "iii", "en", XUI_DOC_TEXT_STYLE_LANGUAGE);
    xuiDocumentRelease(loaded); loaded = NULL;
    CHECK(xuiDocumentExportHtml(snapshot, &html, &html_bytes) == XUI_OK &&
        strstr(html, "data-xui-document=\"true\" lang=\"en-us\"") &&
        strstr(html, "lang=\"tr-tr\"") && strstr(html, "lang=\"en\"") &&
        xuiDocumentFragmentImportHtml(html, html_bytes, &fragment) == XUI_OK);
    {
        uint64_t copied, count, target;
        CHECK(xuiDocumentCreate(NULL, &loaded) == XUI_OK);
        language_set(loaded, 1, "de");
        CHECK(xuiDocumentBeginTransaction(loaded, NULL, &transaction) == XUI_OK &&
            xuiDocumentTxnInsertFragment(transaction, fragment, 1, XUI_DOCUMENT_APPEND, &copied, &count) == XUI_OK &&
            xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        language_query(loaded, "iii", "en", XUI_DOC_TEXT_STYLE_LANGUAGE);
        xuiDocumentRelease(loaded); loaded = NULL;
        xuiDocumentFragmentRelease(fragment); xuiDocumentFreeBuffer(html);
        range = test_find(document, "iii");
        CHECK(xuiDocumentFragmentCreateRange(snapshot, &range, &fragment) == XUI_OK &&
            xuiDocumentCreate(NULL, &loaded) == XUI_OK);
        language_set(loaded, 1, "de");
        CHECK(xuiDocumentBeginTransaction(loaded, NULL, &transaction) == XUI_OK);
        target = insert(transaction, 1, XUI_DOC_PARAGRAPH, NULL);
        CHECK(xuiDocumentTxnInsertFragment(transaction, fragment, target, XUI_DOCUMENT_APPEND, &copied, &count) == XUI_OK &&
            xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        language_query(loaded, "iii", "en", XUI_DOC_TEXT_STYLE_LANGUAGE);
        xuiDocumentRelease(loaded); loaded = NULL; xuiDocumentFragmentRelease(fragment);
    }
    CHECK(xuiDocumentSnapshotAnalyzeMarkdownConversion(snapshot, XUI_MD_GFM,
        loss, 8, &total) == XUI_OK && total >= 3);
    for (uint64_t i = 0; i < total; i++) CHECK(loss[i].iReasons == XUI_DOC_MD_LOSS_LANGUAGE);
    CHECK(xuiDocumentSnapshotConvertToMarkdownWithPolicy(snapshot, XUI_MD_GFM,
        XUI_DOC_MD_LOSS_TEXT_STYLE, &loaded) == XUI_DOC_ERROR_UNREPRESENTABLE && !loaded);
    CHECK(xuiDocumentSnapshotConvertToMarkdownWithPolicy(snapshot, XUI_MD_GFM,
        XUI_DOC_MD_LOSS_LANGUAGE, &loaded) == XUI_OK);
    language_query(loaded, "iii", "", 0); xuiDocumentRelease(loaded); loaded = NULL;
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentFreeBuffer(native);
    /* RFC 5646 private use reaches the accepted 255-byte boundary exactly. */
    strcpy(tag, "x-"); for (unsigned i = 0; i < 28; i++) strcat(tag, "abcdefgh-"); strcat(tag, "a");
    CHECK(strlen(tag) == 255); language_set(document, 1, tag);
    tag[255] = 'b'; tag[256] = 0;
    info.iSize = sizeof(info);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotGetNode(snapshot, 1, &info) == XUI_OK);
    info.tAttributes.sLanguage = tag;
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
        xuiDocumentTxnSetAttributes(transaction, 1, &info.tAttributes) == XUI_DOC_ERROR_SCHEMA &&
        xuiDocumentTxnCommit(transaction, NULL) == XUI_DOC_ERROR_SCHEMA);
    xuiDocumentTxnRelease(transaction); xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
    CHECK(xuiDocumentSnapshotGetNode(kept, paragraph, &info) == XUI_OK &&
        !strcmp(info.tAttributes.sLanguage, "tr-tr"));
    xuiDocumentSnapshotRelease(kept);
    markdown = test_markdown_open("iii\n"); language_set(markdown, 1, "tr");
    test_source(markdown, "iii\n", 1); language_query(markdown, "iii", "tr", 0);
    CHECK(xuiDocumentLoadMarkdown(markdown, "# iii\n\nnew\n", 11) == XUI_OK);
    language_expect_root(markdown, "tr"); language_query(markdown, "iii", "tr", 0);
    CHECK(xuiDocumentAcquireSnapshot(markdown, &snapshot) == XUI_OK &&
        xuiDocumentSerialize(snapshot, &native, &bytes) == XUI_OK &&
        xuiDocumentDeserialize(NULL, native, bytes, &loaded) == XUI_OK);
    language_expect_root(loaded, "tr"); language_query(loaded, "iii", "tr", 0);
    xuiDocumentRelease(loaded); loaded = NULL;
    strstr(native, "\"schemaVersion\":7")[16] = '6';
    CHECK(xuiDocumentDeserialize(NULL, native, bytes, &loaded) == XUI_DOC_ERROR_FORMAT && !loaded);
    xuiDocumentFreeBuffer(native); xuiDocumentSnapshotRelease(snapshot);
    language_set(markdown, 1, "en"); CHECK(xuiDocumentUndo(markdown, NULL) == XUI_OK);
    language_expect_root(markdown, "tr"); CHECK(xuiDocumentRedo(markdown, NULL) == XUI_OK);
    language_expect_root(markdown, "en");
    {
        xui_doc_txn_desc_t desc = {0}; xui_document_prepare prepare;
        xui_doc_source_patch_t patch = {0};
        desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE;
        CHECK(xuiDocumentBeginTransaction(markdown, &desc, &transaction) == XUI_OK &&
            xuiDocumentTxnReplaceSource(transaction, 7, 10, "other", 5) == XUI_OK &&
            xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
        xuiDocumentTxnRelease(transaction); language_query(markdown, "other", "en", 0);
        patch.iSize = sizeof(patch); patch.iStart = 7; patch.iEnd = 12; patch.sText = "last"; patch.iTextBytes = 4;
        CHECK(xuiDocumentPrepareSource(markdown, &desc, &patch, 1, &prepare) == XUI_OK &&
            xuiDocumentPrepareRun(prepare) == XUI_OK && xuiDocumentPreparePublish(markdown, prepare, NULL) == XUI_OK);
        xuiDocumentPrepareRelease(prepare); language_query(markdown, "last", "en", 0);
        CHECK(xuiDocumentUndo(markdown, NULL) == XUI_OK); language_query(markdown, "other", "en", 0);
        CHECK(xuiDocumentRedo(markdown, NULL) == XUI_OK); language_query(markdown, "last", "en", 0);
    }
    xuiDocumentRelease(markdown);
    puts("Document language: ownership, inheritance/und, range/mixed/clear, Undo/Redo, native/HTML, Markdown metadata and 255-byte bound passed");
}
static void document_language_failures(void)
{
    unsigned failures = 0, successes = 0;
    for (long limit = 0; limit < 48; limit++) {
        fail_allocator allocator = {-1, 0}; xui_doc_desc_t desc = {0};
        xui_document document; xui_document_transaction transaction = NULL;
        xui_document_snapshot snapshot; xui_doc_node_info_t info = {0};
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_RICH; desc.onAlloc = failing_alloc;
        desc.onFree = failing_free; desc.pAllocatorUser = &allocator;
        CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
        language_set(document, 1, "en");
        info.iSize = sizeof(info);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotGetNode(snapshot, 1, &info) == XUI_OK);
        info.tAttributes.sLanguage = "zh-Hant-TW";
        allocator.remaining = limit;
        int result = xuiDocumentBeginTransaction(document, NULL, &transaction);
        if (result == XUI_OK) result = xuiDocumentTxnSetAttributes(transaction, 1, &info.tAttributes);
        if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, NULL);
        CHECK(result == XUI_OK || result == XUI_ERROR_OUT_OF_MEMORY);
        allocator.remaining = -1;
        xuiDocumentTxnRelease(transaction); xuiDocumentSnapshotRelease(snapshot);
        language_expect_root(document, result == XUI_OK ? "zh-hant-tw" : "en");
        if (result == XUI_OK) successes++; else failures++;
        xuiDocumentRelease(document); CHECK(!allocator.live);
    }
    CHECK(failures && successes);
    printf("Document language allocation failures: %u failures, %u successful budgets, atomic and leak-free\n", failures, successes);
}
static void document_language_html(void)
{
    const char input[] = "<div lang=EN><p><span lang=TR>i</span><span lang=''>j</span><span lang=en--US>k</span></p></div>"
        "<pre lang=EN><code lang=TR data-language=c>code</code></pre><p xml:lang=TR>xml</p>";
    xui_document document; xui_document_fragment fragment; xui_document_transaction transaction;
    xui_document_snapshot snapshot; xui_doc_node_info_t info = {0}; uint64_t copied, count, code;
    CHECK(xuiDocumentFragmentImportHtml(input, strlen(input), &fragment) == XUI_OK && xuiDocumentCreate(NULL, &document) == XUI_OK);
    language_set(document, 1, "de");
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
        xuiDocumentTxnInsertFragment(transaction, fragment, 1, XUI_DOCUMENT_APPEND, &copied, &count) == XUI_OK &&
        xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction); xuiDocumentFragmentRelease(fragment);
    language_query(document, "i", "tr", 0); language_query(document, "j", "und", 0);
    language_query(document, "k", "en", 0); language_query(document, "xml", "tr", 0);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK); code = find_kind(snapshot, 1, XUI_DOC_CODE_BLOCK);
    info.iSize = sizeof(info);
    CHECK(code && xuiDocumentSnapshotGetNode(snapshot, code, &info) == XUI_OK &&
        !strcmp(info.sInfo, "c") && !strcmp(info.tAttributes.sLanguage, "tr"));
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(document);
    puts("Document HTML language: lang/xml:lang, empty und, invalid inheritance and separate code natural/programming languages passed");
}
