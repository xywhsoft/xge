/* Public API oracle: source-index roles drive an actual source transaction. */
#define main existing_source_index_main
#include "xui_document_source_index_test.c"
#undef main
static const char* cst_index_cases[] = {
    "head\n\n[^u]: **bold** [a](<uri> \"title\") &amp;\n",
    "head[^u]\n\n[^u]: **bold** [a](<uri> \"title\") &amp;\n",
    "head[^u]\n\n[^u]: shown\n[^u]: **bold** [a](<uri> \"title\") &amp;\n",
    "[^u]: > \ttext\n",
    "[^u]: - \titem\n",
    "[^u]:\n\n        code\n",
    "[^u]:\n\n    ```lang extra\n    code\n    ```\n",
    "[^u]: a | b\n    :-: | ---\n    x | y\n",
    "[^u]: a  \n    b\\\n    c\n",
    "head\n\n[^u]: aa <!-- raw --> <b>text</b>\n",
    "[^u]: [missing] [^absent] ![image][none]\n",
    "[^u]: **中文** &amp; \\*\n"
};
static char* cst_index_variant(const char* input, unsigned eol, unsigned bom, uint64_t* bytes)
{
    size_t size = strlen(input), at = 0, i; char* source = malloc(size * 2 + 4); CHECK(source);
    if (bom) { memcpy(source, "\xef\xbb\xbf", 3); at = 3; }
    for (i = 0; i < size; i++) {
        if (input[i] == '\n') { if (eol) source[at++] = '\r'; if (eol != 2) source[at++] = '\n'; }
        else source[at++] = input[i];
    }
    source[at] = 0; *bytes = at; return source;
}
static xui_doc_source_record_t cst_index_role(xui_document_source_index index,
    uint32_t role, uint32_t token_kind, uint32_t detail, uint32_t required_flags,
    uint64_t start, uint64_t end, xui_doc_source_binding_t* found)
{
    xui_doc_source_index_info_t info = {0}; xui_doc_source_record_t record = {0}; uint64_t i, matches = 0;
    info.iSize = sizeof(info); CHECK(xuiDocumentSourceIndexGetInfo(index, &info) == XUI_OK);
    for (i = 0; i < info.iBindingCount; i++) {
        xui_doc_source_binding_t b = {0}; xui_doc_source_record_t r = {0}; b.iSize = sizeof(b); r.iSize = sizeof(r);
        CHECK(xuiDocumentSourceIndexGetBinding(index, i, &b) == XUI_OK);
        if (b.iOriginKind != XUI_DOC_SOURCE_BINDING_DEFINITION_SYNTAX ||
            b.tRange.iKind != token_kind || (detail && b.tRange.iDetail != detail) ||
            b.tRange.iSourceStart != start || b.tRange.iSourceEnd != end) continue;
        CHECK(xuiDocumentSourceIndexGetRecord(index, b.tRange.iRecordIndex, &r) == XUI_OK);
        if (r.iKind != XUI_DOC_SOURCE_RECORD_SYNTAX || r.iDetail != role || (r.iFlags & required_flags) != required_flags) continue;
        CHECK(!r.iNodeId && !b.tRange.iNodeId && (b.iMetadata & XUI_DOC_SOURCE_BINDING_SOURCE_ONLY));
        CHECK(b.tRange.iTextStart == UINT64_MAX && b.tRange.iTextEnd == UINT64_MAX &&
            b.tRange.iOriginalSourceStart <= start && end <= b.tRange.iOriginalSourceEnd);
        record = r; if (found) *found = b; matches++;
    }
    if (matches != 1) fprintf(stderr, "CST role %u kind %u detail %u flags %u range %llu..%llu matches %llu\n",
        role, token_kind, detail, required_flags, (unsigned long long)start, (unsigned long long)end, (unsigned long long)matches);
    CHECK(matches == 1); return record;
}
static uint64_t cst_index_count(xui_document_source_index index)
{
    xui_doc_source_index_info_t info = {0}; uint64_t i, total = 0; info.iSize = sizeof(info);
    CHECK(xuiDocumentSourceIndexGetInfo(index, &info) == XUI_OK);
    for (i = 0; i < info.iRecordCount; i++) { xui_doc_source_record_t r = {0}; r.iSize = sizeof(r);
        CHECK(xuiDocumentSourceIndexGetRecord(index, i, &r) == XUI_OK);
        if (r.iKind == XUI_DOC_SOURCE_RECORD_SYNTAX && !(r.iFlags & XUI_DOC_SOURCE_RECORD_SYNTAX_DOCUMENT)) total++; }
    return total;
}
static void cst_index_goldens(xui_document_source_index index, const char* source, unsigned fixture)
{
    const char* p; xui_doc_source_binding_t b = {0}; xui_doc_source_record_t r;
    if (fixture <= 2) {
        p = strstr(source, "**bold**"); CHECK(p);
        cst_index_role(index, XUI_DOC_SOURCE_SYNTAX_INLINE, XUI_DOC_SOURCE_TOKEN_SCOPE, XUI_DOC_SYNTAX_STRONG, 0, p-source, p-source+8, NULL);
        p = strstr(source, "<uri>"); CHECK(p);
        r = cst_index_role(index, XUI_DOC_SOURCE_SYNTAX_FIELD, XUI_DOC_SOURCE_TOKEN_FIELD_CONTENT, XUI_DOC_SYNTAX_DESTINATION, 0, p-source+1, p-source+4, &b);
        CHECK(r.iSourceStart == (uint64_t)(p-source) && r.iSourceEnd == (uint64_t)(p-source+5) &&
            r.iContentStart == (uint64_t)(p-source+1) && r.iContentEnd == (uint64_t)(p-source+4));
        p = strstr(source, "\"title\""); CHECK(p);
        r = cst_index_role(index, XUI_DOC_SOURCE_SYNTAX_FIELD, XUI_DOC_SOURCE_TOKEN_FIELD_CONTENT, XUI_DOC_SYNTAX_TITLE, 0, p-source+1, p-source+6, NULL);
        CHECK(r.iSourceStart == (uint64_t)(p-source) && r.iSourceEnd == (uint64_t)(p-source+7));
        p = strstr(source, "&amp;"); CHECK(p);
        cst_index_role(index, XUI_DOC_SOURCE_SYNTAX_TEXT, XUI_DOC_SOURCE_TOKEN_CONTENT, XUI_DOC_TEXT, 0, p-source, p-source+5, &b);
        CHECK(b.tRange.iSegmentKind == XUI_DOC_SOURCE_ENTITY);
    } else if (fixture == 3 || fixture == 4) {
        p = strstr(source, fixture == 3 ? "> \t" : "- \t"); CHECK(p);
        cst_index_role(index, XUI_DOC_SOURCE_SYNTAX_INDENT, XUI_DOC_SOURCE_TOKEN_CONTAINER_INDENT, 0,
            fixture == 3 ? XUI_DOC_SOURCE_RECORD_SYNTAX_QUOTE_INDENT : XUI_DOC_SOURCE_RECORD_SYNTAX_LIST_FIRST_INDENT,
            p-source, p-source+3, &b);
        CHECK((b.iMetadata & XUI_DOC_SOURCE_BINDING_COLUMNS) && !b.iStartColumn && b.iEndColumn == 4);
        if (fixture == 3) CHECK(!b.iMarkerColumn && b.iContentColumn == 2);
        else CHECK(b.iMarkerColumn == UINT64_MAX && b.iContentColumn == 4);
    } else if (fixture == 5) {
        p = strstr(source, "        code"); CHECK(p);
        cst_index_role(index, XUI_DOC_SOURCE_SYNTAX_INDENT, XUI_DOC_SOURCE_TOKEN_CODE_INDENT, 0,
            XUI_DOC_SOURCE_RECORD_SYNTAX_CODE_INDENT, p-source+4, p-source+8, &b);
        CHECK((b.iMetadata & XUI_DOC_SOURCE_BINDING_COLUMNS) && !b.iStartColumn && b.iContentColumn == 4 && b.iEndColumn == 4);
    } else if (fixture == 6) {
        p = strstr(source, "lang extra"); CHECK(p);
        cst_index_role(index, XUI_DOC_SOURCE_SYNTAX_FIELD, XUI_DOC_SOURCE_TOKEN_FENCE_INFO, XUI_DOC_BLOCK_SYNTAX_FENCED_CODE,
            XUI_DOC_SOURCE_RECORD_SYNTAX_FENCE_INFO, p-source, p-source+10, NULL);
        cst_index_role(index, XUI_DOC_SOURCE_SYNTAX_FIELD, XUI_DOC_SOURCE_TOKEN_FENCE_INFO, XUI_DOC_BLOCK_SYNTAX_FENCED_CODE,
            XUI_DOC_SOURCE_RECORD_SYNTAX_FENCE_LANGUAGE, p-source, p-source+4, NULL);
    } else if (fixture == 7) {
        p = strstr(source, ":-:"); CHECK(p);
        cst_index_role(index, XUI_DOC_SOURCE_SYNTAX_MARKER, XUI_DOC_SOURCE_TOKEN_TABLE,
            XUI_DOC_TABLE_TOKEN_UNDERLINE, XUI_DOC_SOURCE_RECORD_SYNTAX_TABLE, p-source, p-source+3, &b);
        CHECK((b.iMetadata & XUI_DOC_SOURCE_BINDING_TABLE) && b.iRow == 1 && !b.iOrdinal &&
            b.iMarkerFlags == (XUI_DOC_TABLE_TOKEN_LEFT_COLON | XUI_DOC_TABLE_TOKEN_RIGHT_COLON));
    } else if (fixture == 8) {
        p = strstr(source, "b\\"); CHECK(p);
        cst_index_role(index, XUI_DOC_SOURCE_SYNTAX_BREAK, XUI_DOC_SOURCE_TOKEN_BREAK_MARKER,
            XUI_DOC_BREAK_HARD_BACKSLASH, XUI_DOC_SOURCE_RECORD_SYNTAX_BREAK_MARKER, p-source+1, p-source+2, NULL);
    } else if (fixture == 9) {
        p = strstr(source, "<!-- raw -->"); CHECK(p);
        cst_index_role(index, XUI_DOC_SOURCE_SYNTAX_OPAQUE, XUI_DOC_SOURCE_TOKEN_HTML_COMMENT, XUI_DOC_HTML,
            XUI_DOC_SOURCE_RECORD_OPAQUE, p-source, p-source+12, &b);
        CHECK(b.tRange.iFlags & XUI_DOC_SOURCE_TOKEN_OPAQUE_PAYLOAD);
        expected_token(index, p-source, XUI_DOC_SOURCE_TOKEN_HTML_COMMENT, "<!-- raw -->");
    }
}
static void cst_index_edit(xui_document d, xui_document_snapshot before,
    xui_document_source_index original, const char* source, uint64_t bytes, unsigned prepare)
{
    const char* p = strstr(source, "<uri>"); char* expected = malloc((size_t)bytes + 8);
    xui_doc_source_record_t r; xui_document_transaction txn; xui_doc_txn_desc_t td = {0};
    xui_document_snapshot after, undo; xui_document fresh; xui_document_source_index changed, full, old;
    CHECK(p && expected); r = cst_index_role(original, XUI_DOC_SOURCE_SYNTAX_FIELD,
        XUI_DOC_SOURCE_TOKEN_FIELD_CONTENT, XUI_DOC_SYNTAX_DESTINATION, 0, p-source+1, p-source+4, NULL);
    memcpy(expected, source, (size_t)r.iContentStart); memcpy(expected+r.iContentStart, "new-uri", 7);
    memcpy(expected+r.iContentStart+7, source+r.iContentEnd, (size_t)(bytes-r.iContentEnd+1));
    td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentMarkSaved(d, before) == XUI_OK);
    if (!prepare) {
        CHECK(xuiDocumentBeginTransaction(d, &td, &txn) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceSource(txn, r.iContentStart, r.iContentEnd, "new-uri", 7) == XUI_OK && xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
        xuiDocumentTxnRelease(txn);
    } else {
        xui_document_prepare first, second; xui_doc_source_patch_t patch = {0}; xui_doc_prepare_info_t info = {0};
        patch.iSize = sizeof(patch); patch.iStart = r.iContentStart; patch.iEnd = r.iContentEnd; patch.sText = "new-"; patch.iTextBytes = 4;
        CHECK(xuiDocumentPrepareSource(d, &td, &patch, 1, &first) == XUI_OK && xuiDocumentPrepareRun(first) == XUI_OK);
        index_check(original, source, bytes, 1);
        patch.iStart = patch.iEnd = r.iContentStart+4; patch.sText = "uri"; patch.iTextBytes = 3;
        CHECK(xuiDocumentPrepareContinueSource(d, first, &patch, 1, &second) == XUI_OK);
        info.iSize = sizeof(info); CHECK(xuiDocumentPrepareGetInfo(first, &info) == XUI_OK && info.iResult == XUI_DOC_ERROR_STALE);
        CHECK(xuiDocumentPrepareRun(second) == XUI_OK && xuiDocumentPreparePublish(d, second, NULL) == XUI_OK);
        xuiDocumentPrepareRelease(first); xuiDocumentPrepareRelease(second);
    }
    CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); changed = make_index(after);
    index_check(changed, expected, bytes+4, 1); fresh = open_source(expected, bytes+4, 3, NULL);
    CHECK(xuiDocumentAcquireSnapshot(fresh, &undo) == XUI_OK); full = make_index(undo); compare_indices(changed, full);
    xuiDocumentSnapshotRelease(undo); CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d));
    CHECK(xuiDocumentAcquireSnapshot(d, &undo) == XUI_OK); old = make_index(undo); compare_indices(original, old);
    xuiDocumentSnapshotRelease(undo); CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
    index_check(original, source, bytes, 1); index_check(changed, expected, bytes+4, 1);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d));
    xuiDocumentSourceIndexRelease(changed); xuiDocumentSourceIndexRelease(full); xuiDocumentSourceIndexRelease(old);
    xuiDocumentSnapshotRelease(after); xuiDocumentRelease(fresh); free(expected);
}
static void cst_index_stream(xui_document d, xui_document_source_index original,
    const char* source, uint64_t bytes, unsigned eol)
{
    uint64_t suffix_bytes, at; char* suffix = cst_index_variant("\n[^next]: **中文** [a](x)\n", eol, 0, &suffix_bytes);
    char* expected = malloc((size_t)(bytes+suffix_bytes+1)); xui_document_prepare previous = NULL, next;
    xui_document_snapshot after, full_snapshot; xui_document fresh; xui_document_source_index changed, full;
    xui_doc_stats_t stats = {0}, before = {0}; before.iSize = sizeof(before);
    CHECK(xuiDocumentGetStats(d, &before) == XUI_OK && expected);
    memcpy(expected, source, (size_t)bytes); memcpy(expected+bytes, suffix, (size_t)suffix_bytes+1);
    for (at = 0; at < suffix_bytes; at++) {
        xui_doc_prepare_info_t info = {0}; info.iSize = sizeof(info);
        CHECK(xuiDocumentPrepareStreamSource(d, previous, NULL, suffix+at, 1, at+1 == suffix_bytes, &next) == XUI_OK);
        if (previous) xuiDocumentPrepareRelease(previous);
        previous = next;
        CHECK(xuiDocumentPrepareGetInfo(previous, &info) == XUI_OK);
        if (info.iBufferedUtf8Bytes) CHECK(xuiDocumentPrepareRun(previous) == XUI_DOC_ERROR_BUSY);
        else CHECK(xuiDocumentPrepareRun(previous) == XUI_OK);
        index_check(original, source, bytes, 0);
    }
    CHECK(xuiDocumentPreparePublish(d, previous, NULL) == XUI_OK); xuiDocumentPrepareRelease(previous);
    CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); changed = make_index(after); index_check(changed, expected, bytes+suffix_bytes, 1);
    fresh = open_source(expected, bytes+suffix_bytes, 3, NULL); CHECK(xuiDocumentAcquireSnapshot(fresh, &full_snapshot) == XUI_OK);
    full = make_index(full_snapshot); compare_indices(changed, full);
    stats.iSize = sizeof(stats); CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK && stats.iUndoCount == before.iUndoCount+1);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d));
    CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); index_check(changed, expected, bytes+suffix_bytes, 1);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
    xuiDocumentSourceIndexRelease(changed); xuiDocumentSourceIndexRelease(full);
    xuiDocumentSnapshotRelease(after); xuiDocumentSnapshotRelease(full_snapshot); xuiDocumentRelease(fresh);
    free(suffix); free(expected);
}
static void cst_index_matrix(void)
{
    unsigned fixture, dialect, eol, bom, total = 0, edits = 0, streams = 0;
    for (fixture = 0; fixture < sizeof(cst_index_cases)/sizeof(*cst_index_cases); fixture++)
    for (dialect = 1; dialect <= 3; dialect++) for (eol = 0; eol < 3; eol++) for (bom = 0; bom < 2; bom++) {
        uint64_t bytes; char* source = cst_index_variant(cst_index_cases[fixture], eol, bom, &bytes);
        fail_allocator a = {-1, 0}; xui_document d = open_source(source, bytes, dialect, &a); xui_document_snapshot before;
        xui_document_source_index index; unsigned live;
        fprintf(stderr, "CST index fixture=%u dialect=%u eol=%u bom=%u load\n", fixture, dialect, eol, bom);
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK); index = make_index(before); live = a.live;
        a.remaining = 0; index_check(index, source, bytes, 1);
        if (dialect == 3) { CHECK(cst_index_count(index)); cst_index_goldens(index, source, fixture); }
        else {
            /* CommonMark/GFM can parse '[^u]: shown' as a regular link
             * definition. Link CST now exists in all dialects; footnote
             * grammar remains exclusive to the extended dialect. */
            uint64_t ordinal = 0; xui_doc_reference_definition_t definition = {0}; int result;
            for (;;) {
                definition.iSize = sizeof(definition);
                result = xuiDocumentSnapshotGetReferenceDefinition(before, ordinal, &definition);
                if (result == XUI_ERROR_NOT_FOUND) break;
                CHECK(result == XUI_OK && definition.iKind == XUI_DOC_REFERENCE_LINK); ordinal++;
            }
            CHECK((cst_index_count(index) != 0) == (ordinal != 0));
        }
        CHECK(a.live == live); a.remaining = -1;
        if (dialect == 3 && fixture <= 2) for (unsigned mode = 0; mode < 2; mode++) {
            fprintf(stderr, "CST index edit mode=%u\n", mode); cst_index_edit(d, before, index, source, bytes, mode); edits++;
        }
        if (dialect == 3) { CHECK(xuiDocumentMarkSaved(d, before) == XUI_OK);
            fprintf(stderr, "CST index stream\n"); cst_index_stream(d, index, source, bytes, eol); streams++; }
        xuiDocumentRelease(d); xuiDocumentSnapshotRelease(before); index_check(index, source, bytes, 1);
        xuiDocumentSourceIndexRelease(index); CHECK(!a.live); free(source); total++;
    }
    printf("Definition CST source index: %u public-query/dialect/ending/BOM/lifetime cases, %u query-driven SOURCE/Prepare/Continue URI edits, %u byte-stream UTF8/new-definition cases; full parse/Undo/Redo, no query allocations or semantic nodes\n", total, edits, streams);
}
int main(void) { cst_index_matrix(); return 0; }
