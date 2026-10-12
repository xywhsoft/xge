#define XUI_QUOTE_BOUNDARY_NO_MAIN
#include "xui_document_quote_boundary_test.c"

static const char* admonition_tags[] = {"nOtE", "TIP", "Important", "warning", "CaUtIoN"};
static const unwrap_source_case admonition_cases[] = {
    { "before &amp; raw\n\n> [!~TYPE~]\n> take &amp; raw\n>\n> ~~~~ lang opaque &amp; info\n> code\\raw\n> ~~~~\n\nafter &amp; raw\n",
      "before &amp; raw\n\ntake &amp; raw\n\n~~~~ lang opaque &amp; info\ncode\\raw\n~~~~\n\nafter &amp; raw\n" },
    { "before &amp; raw\n> [!~TYPE~]\n> take &amp; raw\n>\n> <div>raw &amp;</div>\nafter &amp; raw\n",
      "before &amp; raw\n\ntake &amp; raw\n\n<div>raw &amp;</div>\n\nafter &amp; raw\n" },
    { "> before &amp; raw\n> > [!~TYPE~]\n> > take &amp; raw\n>\n> after &amp; raw\n",
      "> before &amp; raw\n> \n> take &amp; raw\n>\n> after &amp; raw\n" },
    { "- before &amp; raw\n  > [!~TYPE~]\n  > take &amp; raw\n- last &amp; raw\n",
      "- before &amp; raw\n  \n  take &amp; raw\n- last &amp; raw\n" },
    { "- > [!~TYPE~]\n  > take &amp; raw\n  >\n  > ~~~~ lang opaque\n  > code\\raw\n  > ~~~~\n- last &amp; raw\n",
      "- take &amp; raw\n  \n  ~~~~ lang opaque\n  code\\raw\n  ~~~~\n- last &amp; raw\n" },
    { "- > [!~TYPE~]\n  > take &amp; raw\n  >\n  > <div>raw &amp;</div>\n  after &amp; raw\n- last &amp; raw\n",
      "- take &amp; raw\n  \n  <div>raw &amp;</div>\n  \n  after &amp; raw\n- last &amp; raw\n" },
    { "100. > [!~TYPE~]\n     > take &amp; raw\n     >\n     > <div>raw &amp;</div>\n     after &amp; raw\n101. last &amp; raw\n",
      "100. take &amp; raw\n     \n     <div>raw &amp;</div>\n     \n     after &amp; raw\n101. last &amp; raw\n" },
    { "> - > [!~TYPE~]\n>   > take &amp; raw\n> - last &amp; raw\n", "> - take &amp; raw\n> - last &amp; raw\n" },
    { "> [!nOtE]\n> > [!~TYPE~]\n> > take &amp; raw\n> >\n> > ~~~~ lang opaque\n> > [!NOTE] literal\n> > ~~~~\n",
      "> [!nOtE]\n> take &amp; raw\n> \n> ~~~~ lang opaque\n> [!NOTE] literal\n> ~~~~\n" },
    { "  > [!~TYPE~]  \t\n > take 中文 🧡 &amp; raw\n >\n   > ~~~~ lang opaque\n > code\\raw\n > ~~~~\n",
      "take 中文 🧡 &amp; raw\n\n~~~~ lang opaque\ncode\\raw\n~~~~\n" },
    { "> [!~TYPE~]\n> take &amp; raw", "take &amp; raw" },
    { "> [!~TYPE~]\n> take &amp; raw\n>\n>     [!NOTE] code\n", "take &amp; raw\n\n    [!NOTE] code\n" },
    { "-\t> [!~TYPE~]\n\t> take &amp; raw\n\t>\n\t> ~~~~ lang opaque\n\t>\tcode\t<&> raw\n\t> ~~~~\n",
      "-\ttake &amp; raw\n\t\n\t~~~~ lang opaque\n\t  code\t<&> raw\n\t~~~~\n" },
    { "> [!~TYPE~]\n> take [a]\n>\n> [a]: /first 'raw &amp;'\n>\n> [a]: /duplicate 'one\n> two'\n>\n> | a | b |\n> | :-- | --: |\n> | [a] | &amp; |\n",
      "take [a]\n\n[a]: /first 'raw &amp;'\n\n[a]: /duplicate 'one\ntwo'\n\n| a | b |\n| :-- | --: |\n| [a] | &amp; |\n" },
    { "> [!~TYPE~]\n> take [^n]\n>\n> [^n]: foot &amp; body\n>     continuation &amp;\n>\n> [^unused]: raw unused\n>     continuation &amp;\n",
      "take [^n]\n\n[^n]: foot &amp; body\n    continuation &amp;\n\n[^unused]: raw unused\n    continuation &amp;\n" },
    { "before &amp; raw\r\n> [!~TYPE~]\n> take &amp; raw\r\n>\n> <div>raw &amp;</div>\r\nafter &amp; raw\n",
      "before &amp; raw\r\n\r\ntake &amp; raw\r\n\n<div>raw &amp;</div>\r\n\r\nafter &amp; raw\n" },
    { "> [!~TYPE~]\r> take &amp; raw\r>\r> <div>raw &amp;</div>\rafter &amp; raw\r",
      "take &amp; raw\r\r<div>raw &amp;</div>\r\rafter &amp; raw\r" }
};
static void admonition_source(unsigned ci, unsigned tag, unsigned mode, char* source, char* expected, size_t capacity)
{
    char raw[8192]; const char* at = strstr(admonition_cases[ci].source, "~TYPE~"); size_t lead;
    CHECK(at); lead = (size_t)(at - admonition_cases[ci].source);
    CHECK(lead + strlen(admonition_tags[tag]) + strlen(at + 6) < sizeof(raw));
    memcpy(raw, admonition_cases[ci].source, lead); strcpy(raw + lead, admonition_tags[tag]);
    strcat(raw, at + 6); boundary_variant(raw, mode, source, capacity);
    boundary_variant(admonition_cases[ci].expected, mode, expected, capacity);
}
static uint64_t admonition_patch_proof(const char* source, const char* output,
    xui_document_snapshot before, xui_document_change_set change, xui_doc_block_syntax_t* header)
{
    xui_doc_change_info_t info = {0}; uint64_t i, at, last = UINT64_MAX, removed = 0, added = 0, count = 0;
    info.iSize = sizeof(info); CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
    for (i = 0; i < info.iOperationCount; i++) {
        const xui_doc_operation_t* op = &info.pOperations[i];
        if (op->iKind != XUI_DOC_OP_SOURCE) continue;
        CHECK(op->iOffset <= last && op->iOldLength <= 2048 && op->iNewLength <= 2048);
        for (at = op->iOffset; at < op->iOffset + op->iOldLength; at++)
            CHECK((at >= header->iSecondaryStart && at < header->iSecondaryEnd) ||
                source[at] == '>' || source[at] == ' ' || source[at] == '\t' || source[at] == '\r' || source[at] == '\n');
        removed += op->iOldLength; added += op->iNewLength; last = op->iOffset; count++;
    }
    CHECK(count && strlen(source) - removed + added == strlen(output));
    for (at = 0; at <= strlen(source); at++) {
        unsigned affinity;
        if (((unsigned char)source[at] & 0xc0) == 0x80) continue;
        for (affinity = 0; affinity < 2; affinity++) {
            xui_doc_position_t p = {0}, mapped; uint64_t expected = at; int mapping;
            p.iSize = sizeof(p); p.iKind = XUI_DOC_POSITION_SOURCE; p.iNodeId = 1;
            p.iDocumentId = xuiDocumentSnapshotGetIdentity(before); p.iRevision = xuiDocumentSnapshotGetRevision(before);
            p.iOffset = at; p.iAffinity = affinity;
            for (i = 0; i < info.iOperationCount; i++) {
                const xui_doc_operation_t* op = &info.pOperations[i];
                if (op->iKind != XUI_DOC_OP_SOURCE || expected < op->iOffset ||
                    (expected == op->iOffset && affinity == XUI_DOC_BEFORE)) continue;
                if (expected >= op->iOffset + op->iOldLength) expected = expected - op->iOldLength + op->iNewLength;
                else expected = op->iOffset + (affinity == XUI_DOC_AFTER ? op->iNewLength : 0);
            }
            CHECK(xuiDocumentMapPosition(change, &p, &mapped, &mapping) == XUI_OK && mapped.iOffset == expected);
        }
    }
    return removed;
}
static void admonition_matrix(void)
{
    unsigned ci, tag, mode, gap, affinity, count = 0;
    for (ci = 0; ci < sizeof(admonition_cases) / sizeof(admonition_cases[0]); ci++)
    for (tag = 0; tag < sizeof(admonition_tags) / sizeof(admonition_tags[0]); tag++)
    for (mode = 0; mode < 3; mode++)
    for (gap = 0; gap < 3; gap++)
    for (affinity = 0; affinity < 2; affinity++) {
        char source[8192], expected[8192], actual[8192], spelling[32]; xui_document d, reload;
        xui_document_snapshot before, after, full; xui_document_transaction t; xui_document_change_set change;
        xui_doc_range_t found; xui_doc_position_t at, caret; xui_doc_node_info_t quote; xui_doc_block_syntax_t header = {0};
        uint64_t i, removed; int result;
        if (mode == 1 && strchr(admonition_cases[ci].source, '\r')) continue;
        admonition_source(ci, tag, mode, source, expected, sizeof(source)); d = open_md(source); found = find_text(d, "take");
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        at = unwrap_position(before, found.tAnchor, gap); at.iAffinity = affinity; if (!gap) at.iOffset += 2;
        quote = unwrap_quote_node(before, at); header.iSize = sizeof(header);
        CHECK(xuiDocumentSnapshotGetBlockSyntax(before, quote.iId, &header) == XUI_OK &&
            header.iKind == XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN && header.iSecondaryEnd > header.iSecondaryStart &&
            header.iSecondaryEnd - header.iSecondaryStart < sizeof(spelling));
        CHECK(xuiDocumentSnapshotReadSource(before, header.iSecondaryStart, spelling, header.iSecondaryEnd - header.iSecondaryStart) == XUI_OK);
        spelling[header.iSecondaryEnd - header.iSecondaryStart] = 0;
        CHECK(spelling[0] == '[' && spelling[1] == '!' && !strncmp(spelling + 2, admonition_tags[tag], strlen(admonition_tags[tag])));
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        result = xuiDocumentTxnUnwrapQuote(t, &at, &caret);
        if (result) fprintf(stderr, "admonition %u/%u/%u/%u/%u: %d\n", ci, tag, mode, gap, affinity, result);
        CHECK(result == XUI_OK && xuiDocumentTxnCommit(t, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, actual, sizeof(actual));
        if (strcmp(actual, expected)) fprintf(stderr, "admonition %u/%u/%u/%u/%u source:\n%s\nexpected:\n%s\n", ci, tag, mode, gap, affinity, actual, expected);
        CHECK(!strcmp(actual, expected)); removed = admonition_patch_proof(source, actual, before, change, &header);
        xuiDocumentChangeSetRelease(change);
        for (i = 0; i < quote.iChildCount; i++) {
            uint64_t child; xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
            CHECK(xuiDocumentSnapshotGetChild(before, quote.iId, i, &child) == XUI_OK &&
                xuiDocumentSnapshotGetNode(after, child, &node) == XUI_OK && node.iParentId == quote.iParentId);
            same_tree(before, child, after, child); boundary_child_ids(before, after, child);
        }
#ifndef XUI_DLL
        CHECK(unwrap_shared_bytes(before->state->source, after->state->source) == strlen(source) - removed &&
            before->state->reference_values == after->state->reference_values);
        reference_cache_snapshot_equal(before, after);
#else
        (void)removed;
#endif
        reload = open_md(actual); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full); xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); count++;
    }
    printf("Admonition source: %u exact five-type/case TEXT/GAP/affinity/EOL cases, owned header, stable children, sparse copies and history\n", count);
}
static uint64_t admonition_first(xui_document_snapshot snapshot, uint64_t id)
{
    xui_doc_node_info_t node = {0}; uint64_t i, child, found; node.iSize = sizeof(node);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &node) == XUI_OK);
    if (node.iKind == XUI_DOC_QUOTE && node.sInfo && *node.sInfo) return id;
    for (i = 0; i < node.iChildCount; i++) {
        CHECK(xuiDocumentSnapshotGetChild(snapshot, id, i, &child) == XUI_OK);
        found = admonition_first(snapshot, child); if (found) return found;
    }
    return 0;
}
static void admonition_empty_and_dialects(void)
{
    const unwrap_source_case cases[] = {
        {"> [!NOTE]\n", ""}, {"> [!NOTE]", ""}, {"- > [!NOTE]\n", "- \n"},
        {"> > [!NOTE]\n", "> \n"},
        {"before &amp; raw\n> [!NOTE]\nafter &amp; raw\n", "before &amp; raw\n\nafter &amp; raw\n"},
        {"- before &amp; raw\n  > [!NOTE]\n  after &amp; raw\n- last\n", "- before &amp; raw\n  \n  after &amp; raw\n- last\n"}
    };
    unsigned ci, mode, affinity, count = 0;
    for (ci = 0; ci < sizeof(cases) / sizeof(cases[0]); ci++)
    for (mode = 0; mode < 3; mode++)
    for (affinity = 0; affinity < 2; affinity++) {
        char source[8192], expected[8192], actual[8192]; xui_document d, reload; uint64_t id;
        xui_document_snapshot before, after, full; xui_document_transaction t; xui_document_change_set change;
        xui_doc_node_info_t node = {0}; xui_doc_position_t at = {0}, caret; xui_doc_block_syntax_t header = {0};
        boundary_variant(cases[ci].source, mode, source, sizeof(source)); boundary_variant(cases[ci].expected, mode, expected, sizeof(expected));
        d = open_md(source); CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        id = admonition_first(before, 1); CHECK(id); node.iSize = sizeof(node);
        CHECK(xuiDocumentSnapshotGetNode(before, id, &node) == XUI_OK && !node.iChildCount);
        at.iSize = sizeof(at); at.iKind = XUI_DOC_POSITION_GAP; at.iNodeId = id; at.iAffinity = affinity;
        at.iDocumentId = xuiDocumentSnapshotGetIdentity(before); at.iRevision = xuiDocumentSnapshotGetRevision(before);
        header.iSize = sizeof(header); CHECK(xuiDocumentSnapshotGetBlockSyntax(before, id, &header) == XUI_OK);
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnUnwrapQuote(t, &at, &caret) == XUI_OK &&
            xuiDocumentTxnCommit(t, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, actual, sizeof(actual));
        if (strcmp(actual, expected)) fprintf(stderr, "empty admonition %u/%u/%u:\n%s\nexpected:\n%s\n", ci, mode, affinity, actual, expected);
        CHECK(!strcmp(actual, expected)); admonition_patch_proof(source, actual, before, change, &header); xuiDocumentChangeSetRelease(change);
        reload = open_md(actual); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK); same_tree(after, 1, full, 1); same_syntax(after, full);
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); count++;
    }
    for (ci = XUI_MD_COMMONMARK; ci <= XUI_MD_GFM; ci++) {
        const char* source = "> [!NOTE]\n> take &amp; raw\n"; xui_doc_desc_t desc = {0}; xui_document d;
        xui_document_snapshot snapshot; xui_doc_range_t found; xui_doc_node_info_t quote;
        xui_doc_block_syntax_t header = {0}; xui_document_transaction t; xui_doc_position_t caret; char actual[256];
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = ci;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
            xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
        found = find_text(d, "take"); quote = unwrap_quote_node(snapshot, found.tAnchor); header.iSize = sizeof(header);
        CHECK(!*quote.sInfo && xuiDocumentSnapshotGetBlockSyntax(snapshot, quote.iId, &header) == XUI_OK && header.iSecondaryStart == UINT64_MAX);
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnUnwrapQuote(t, &found.tAnchor, &caret) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, "[!NOTE]\ntake &amp; raw\n"));
        xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(d);
    }
    printf("Admonition empty: %u root/list/parent/adjacent/BOM/EOF cases; CommonMark and GFM literal tags preserved\n", count);
}
static void admonition_shift_and_native(void)
{
    char source[8192], expected[8192], shifted[8192], actual[8192]; const char* prefix = "prefix &amp; raw\n\n";
    xui_document d, loaded; xui_document_snapshot before, after, full; xui_document_transaction t;
    xui_doc_txn_desc_t td = {0};
    xui_doc_range_t found; xui_doc_node_info_t quote; xui_doc_block_syntax_t a = {0}, b = {0}; char* data; uint64_t bytes;
    admonition_source(0, 0, 1, source, expected, sizeof(source)); d = open_md(source); found = find_text(d, "take");
    CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK); quote = unwrap_quote_node(before, found.tAnchor);
    a.iSize = b.iSize = sizeof(a); CHECK(xuiDocumentSnapshotGetBlockSyntax(before, quote.iId, &a) == XUI_OK);
    td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK && xuiDocumentTxnReplaceSource(t, 0, 0, prefix, strlen(prefix)) == XUI_OK &&
        xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentSnapshotGetBlockSyntax(after, quote.iId, &b) == XUI_OK && b.iSecondaryStart == a.iSecondaryStart + strlen(prefix) &&
        b.iSecondaryEnd == a.iSecondaryEnd + strlen(prefix));
    strcpy(shifted, prefix); strcat(shifted, source); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, shifted));
    CHECK(xuiDocumentSerialize(after, &data, &bytes) == XUI_OK && xuiDocumentDeserialize(NULL, data, bytes, &loaded) == XUI_OK &&
        xuiDocumentAcquireSnapshot(loaded, &full) == XUI_OK); xuiDocumentFreeBuffer(data);
    same_tree(after, 1, full, 1); same_syntax(after, full);
    found = find_text(loaded, "take");
    {
        xui_doc_node_info_t loaded_quote = unwrap_quote_node(full, found.tAnchor);
        b.iSize = sizeof(b); CHECK(xuiDocumentSnapshotGetBlockSyntax(full, loaded_quote.iId, &b) == XUI_OK &&
            b.iSecondaryStart == a.iSecondaryStart + strlen(prefix));
    }
    xuiDocumentSnapshotRelease(full); xuiDocumentRelease(loaded);
    CHECK(xuiDocumentSnapshotGetBlockSyntax(before, quote.iId, &b) == XUI_OK && b.iSecondaryStart == a.iSecondaryStart);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
    xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    puts("Admonition metadata: original case, old snapshot, incremental source shift and native roundtrip passed");
}
static void admonition_depth_and_compound(void)
{
    const unsigned depths[] = {0, 1, 8, 32, 64, 123}; unsigned di;
    for (di = 0; di < sizeof(depths) / sizeof(depths[0]); di++) {
        const char* lines[] = {"[!nOtE]", "take &amp; raw", "", "~~~~ lang opaque", "code\\raw", "~~~~"};
        char source[8192], expected[8192], actual[8192]; size_t a = 0, b = 0; unsigned line, layer;
        xui_document d; xui_document_snapshot before, after; xui_document_transaction t;
        xui_doc_range_t found; xui_doc_position_t caret;
        for (line = 0; line < sizeof(lines) / sizeof(lines[0]); line++) {
            for (layer = 0; layer <= depths[di]; layer++) a += (size_t)sprintf(source + a, "> ");
            a += (size_t)sprintf(source + a, "%s\n", lines[line]);
            if (!line) continue;
            for (layer = 0; layer < depths[di]; layer++) b += (size_t)sprintf(expected + b, "> ");
            b += (size_t)sprintf(expected + b, "%s\n", lines[line]);
        }
        CHECK(a < sizeof(source) && b < sizeof(expected)); d = open_md(source); found = find_text(d, "take");
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnUnwrapQuote(t, &found.tAnchor, &caret) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        same_tree(before, 1, after, 1); same_syntax(before, after); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    }
    {
        const char* source = "> [!nOtE]\n> > [!TIP]\n> > take &amp; raw\n> >\n> > ~~~~ lang opaque\n> > code\\raw\n> > ~~~~\n";
        const char* expected = "take &amp; raw\n\n~~~~ lang opaque\ncode\\raw\n~~~~\n";
        xui_document d = open_md(source); xui_document_transaction t; xui_doc_range_t found = find_text(d, "take");
        xui_doc_position_t caret; char actual[8192]; uint64_t revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnUnwrapQuote(t, &found.tAnchor, &caret) == XUI_OK && xuiDocumentTxnUnwrapQuote(t, &caret, &caret) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source) && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentGetRevision(d) == revision + 1); xuiDocumentTxnRelease(t);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d)); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        xuiDocumentRelease(d);
    }
    puts("Admonition source: root/1/8/32/64/123 parent depths, nested two-header transaction and single Undo passed");
}
static void admonition_footnote_metadata(void)
{
    const char* sources[] = {
        "use[^n]\n\n[^n]:\n    > [!nOtE]\n    > take &amp; raw\n",
        "use[^n]\r\n\r\n[^n]:\r\n    > [!TIP]\r\n    > take &amp; raw\r\n",
        "use[^n]\n\n[^n]:\n    - > [!Important]\n      > take &amp; raw\n",
        "use[^n]\n\n[^n]:\n\t> [!warning]\n\t> take &amp; raw\n"
    }; unsigned ci;
    for (ci = 0; ci < sizeof(sources) / sizeof(sources[0]); ci++) {
        xui_document d = open_md(sources[ci]), loaded; xui_document_snapshot snapshot, full;
        xui_doc_range_t found = find_text(d, "take"); xui_doc_node_info_t quote; xui_doc_block_syntax_t header = {0};
        char spelling[32], *data; uint64_t bytes;
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK); quote = unwrap_quote_node(snapshot, found.tAnchor);
        header.iSize = sizeof(header); CHECK(xuiDocumentSnapshotGetBlockSyntax(snapshot, quote.iId, &header) == XUI_OK &&
            header.iSecondaryEnd > header.iSecondaryStart && header.iSecondaryEnd - header.iSecondaryStart < sizeof(spelling));
        CHECK(xuiDocumentSnapshotReadSource(snapshot, header.iSecondaryStart, spelling, header.iSecondaryEnd - header.iSecondaryStart) == XUI_OK);
        spelling[header.iSecondaryEnd - header.iSecondaryStart] = 0; CHECK(strstr(sources[ci], spelling) && spelling[0] == '[' && spelling[1] == '!');
        CHECK(xuiDocumentSerialize(snapshot, &data, &bytes) == XUI_OK && xuiDocumentDeserialize(NULL, data, bytes, &loaded) == XUI_OK &&
            xuiDocumentAcquireSnapshot(loaded, &full) == XUI_OK); xuiDocumentFreeBuffer(data);
        same_tree(snapshot, 1, full, 1); same_syntax(snapshot, full);
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(loaded); xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(d);
    }
    puts("Admonition footnote metadata: four dedented/list/Tab/mixed-ending loads and native roundtrips passed; inner-footnote structure editing remains open");
}
static void admonition_large_source(void)
{
    const unsigned sizes[] = {4096, 131072, 1048576}; unsigned si; uint64_t history = 0;
    for (si = 0; si < sizeof(sizes) / sizeof(sizes[0]); si++) {
        const char* head = "before &amp; raw\n> [!nOtE]\n> take\n>\n> [^unused]: ";
        const char* expected_head = "before &amp; raw\n\ntake\n\n[^unused]: ";
        const char* tail = "\n>\n> <div>raw &amp;</div>\nafter &amp; raw\n";
        const char* expected_tail = "\n\n<div>raw &amp;</div>\n\nafter &amp; raw\n";
        size_t capacity = sizes[si] + 256, length; char* source = malloc(capacity), *expected = malloc(capacity), *actual = malloc(capacity);
        xui_document d; xui_document_snapshot before, after; xui_document_transaction t; xui_document_change_set change;
        xui_doc_range_t at; xui_doc_position_t caret; xui_doc_node_info_t quote; xui_doc_block_syntax_t header = {0};
        xui_doc_memory_stats_t stats = {0}; xui_doc_change_info_t info = {0}; uint64_t i, removed = 0;
        CHECK(source && expected && actual); strcpy(source, head); length = strlen(head); memset(source + length, 'q', sizes[si]);
        strcpy(source + length + sizes[si], tail); strcpy(expected, expected_head); length = strlen(expected_head);
        memset(expected + length, 'q', sizes[si]); strcpy(expected + length + sizes[si], expected_tail);
        d = open_md(source); at = find_text(d, "take");
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        quote = unwrap_quote_node(before, at.tAnchor); header.iSize = sizeof(header);
        CHECK(xuiDocumentSnapshotGetBlockSyntax(before, quote.iId, &header) == XUI_OK &&
            xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnUnwrapQuote(t, &at.tAnchor, &caret) == XUI_OK &&
            xuiDocumentTxnCommit(t, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, actual, capacity); CHECK(!strcmp(actual, expected));
        info.iSize = sizeof(info); CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
        for (i = 0; i < info.iOperationCount; i++) if (info.pOperations[i].iKind == XUI_DOC_OP_SOURCE) {
            CHECK(info.pOperations[i].iOldLength < 64 && info.pOperations[i].iNewLength < 4);
            removed += info.pOperations[i].iOldLength;
        }
        xuiDocumentChangeSetRelease(change);
#ifndef XUI_DLL
        CHECK(unwrap_shared_bytes(before->state->source, after->state->source) == strlen(source) - removed &&
            before->state->reference_values == after->state->reference_values);
        reference_cache_snapshot_equal(before, after);
#else
        (void)removed;
#endif
        stats.iSize = sizeof(stats); CHECK(xuiDocumentGetMemoryStats(d, &stats) == XUI_OK && stats.iHistoryBytes < 65536);
        if (history) CHECK(stats.iHistoryBytes == history);
        history = stats.iHistoryBytes;
        printf("Admonition source %u-byte hidden body: HistoryBytes=%llu\n", sizes[si], (unsigned long long)history);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, capacity); CHECK(!strcmp(actual, source));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); free(source); free(expected); free(actual);
    }
}
static void admonition_failures(void)
{
    const unsigned samples[] = {0, 1, 3, 4, 6, 8, 13, 14}; unsigned sample; long total = 0;
    for (sample = 0; sample < sizeof(samples) / sizeof(samples[0]); sample++) {
        unsigned ci = samples[sample]; long point; int success = 0;
        for (point = 0; point < 10000 && !success; point++) {
            failing_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_transaction t = NULL; xui_document_snapshot before, after; xui_doc_range_t found;
            xui_doc_position_t caret; char actual[8192], source[8192], expected[8192]; unsigned live; uint64_t revision; int result;
            admonition_source(ci, 0, 0, source, expected, sizeof(source));
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = fail_alloc; desc.onFree = fail_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            found = find_text(d, "take"); live = a.live; revision = xuiDocumentGetRevision(d); a.remaining = point;
            result = xuiDocumentBeginTransaction(d, NULL, &t);
            if (!result) result = xuiDocumentTxnUnwrapQuote(t, &found.tAnchor, &caret);
            if (!result) result = xuiDocumentTxnCommit(t, NULL);
            xuiDocumentTxnRelease(t); a.remaining = -1;
            if (result) CHECK(result == XUI_ERROR_OUT_OF_MEMORY && a.live == live &&
                xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) {
                success = 1; copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
            } else { same_tree(before, 1, after, 1); same_syntax(before, after); }
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success); total += point - 1; printf("Admonition source OOM %u: %ld points, all failed attempts unpublished and no leaks\n", ci, point - 1);
    }
    printf("Admonition source OOM total: %ld\n", total);
}
#ifndef XUI_DLL
static void admonition_cancellation(void)
{
    const unsigned samples[] = {0, 1, 3, 4, 6, 8, 13, 14}; unsigned sample; long total = 0;
    for (sample = 0; sample < sizeof(samples) / sizeof(samples[0]); sample++) {
        unsigned ci = samples[sample]; long point; int success = 0;
        for (point = 0; point < 10000 && !success; point++) {
            prefix_cancel_allocator a = {{-1, 0}, -1, ATOMIC_VAR_INIT(0)}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_transaction t; xui_document_snapshot before, after; xui_doc_range_t found;
            xui_doc_position_t caret; char actual[8192], source[8192], expected[8192]; unsigned live; uint64_t revision; int result;
            admonition_source(ci, 0, 0, source, expected, sizeof(source));
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = prefix_cancel_alloc; desc.onFree = prefix_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            found = find_text(d, "take"); live = a.allocation.live; revision = xuiDocumentGetRevision(d);
            CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); t->cancellation = &a.cancellation; a.remaining = point;
            result = xuiDocumentTxnUnwrapQuote(t, &found.tAnchor, &caret);
            if (!result) result = xuiDocumentTxnCommit(t, NULL);
            a.remaining = -1; xuiDocumentTxnRelease(t); atomic_store(&a.cancellation, 0);
            if (result) CHECK(result == XUI_DOC_ERROR_CANCELLED && a.allocation.live == live &&
                xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) {
                success = 1; copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
            } else { same_tree(before, 1, after, 1); same_syntax(before, after); reference_cache_snapshot_equal(before, after); }
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.allocation.live);
        }
        CHECK(success); total += point - 1; printf("Admonition source cancellation %u: %ld checkpoints, atomic state and no leaks\n", ci, point - 1);
    }
    printf("Admonition source cancellation total: %ld\n", total);
}
#endif
static void admonition_rich(void)
{
    const unsigned samples[] = {0, 2, 3, 7, 8, 10, 14}; unsigned si, tag;
    for (si = 0; si < sizeof(samples) / sizeof(samples[0]); si++)
    for (tag = 0; tag < sizeof(admonition_tags) / sizeof(admonition_tags[0]); tag++) {
        unsigned ci = samples[si]; xui_document md, rich, expected_md, expected_rich;
        xui_document_snapshot source, before, after, full; xui_doc_rich_conversion_report_t report = {0};
        xui_document_transaction t; xui_doc_range_t at; xui_doc_position_t caret;
        char raw_source[8192], raw_expected[8192];
        admonition_source(ci, tag, 0, raw_source, raw_expected, sizeof(raw_source));
        report.iSize = sizeof(report); md = open_md(raw_source);
        CHECK(xuiDocumentAcquireSnapshot(md, &source) == XUI_OK && xuiDocumentSnapshotAnalyzeRichConversion(source, &report) == XUI_OK &&
            xuiDocumentSnapshotConvertToRich(source, report.iReasons, &rich) == XUI_OK);
        xuiDocumentSnapshotRelease(source); xuiDocumentRelease(md);
        expected_md = open_md(raw_expected);
        CHECK(xuiDocumentAcquireSnapshot(expected_md, &source) == XUI_OK && xuiDocumentSnapshotAnalyzeRichConversion(source, &report) == XUI_OK &&
            xuiDocumentSnapshotConvertToRich(source, report.iReasons, &expected_rich) == XUI_OK);
        xuiDocumentSnapshotRelease(source); xuiDocumentRelease(expected_md); at = find_text(rich, "take");
        CHECK(xuiDocumentClearHistory(rich) == XUI_OK && xuiDocumentAcquireSnapshot(rich, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(rich, NULL, &t) == XUI_OK && xuiDocumentTxnUnwrapQuote(t, &at.tAnchor, &caret) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(rich, &after) == XUI_OK &&
            xuiDocumentAcquireSnapshot(expected_rich, &full) == XUI_OK);
        xuiDocumentTxnRelease(t); same_tree(after, 1, full, 1);
        xuiDocumentSnapshotRelease(after); xuiDocumentSnapshotRelease(full);
        CHECK(xuiDocumentUndo(rich, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(rich, &after) == XUI_OK);
        same_tree(before, 1, after, 1); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after);
        CHECK(!xuiDocumentCanUndo(rich) && xuiDocumentCanRedo(rich));
        xuiDocumentRelease(rich); xuiDocumentRelease(expected_rich);
    }
    puts("Admonition source: 35 Rich equivalents, paragraph seams, ordered children and shared Undo");
}
#ifdef XUI_QUOTE_TEST_EDITOR
static void admonition_editor(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    unsigned ci, tag, gap, mode, affinity, total = 0, refused = 0;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    for (ci = 0; ci < sizeof(admonition_cases) / sizeof(admonition_cases[0]); ci++)
    for (tag = 0; tag < sizeof(admonition_tags) / sizeof(admonition_tags[0]); tag++)
    for (gap = 0; gap < 3; gap++)
    for (mode = 0; mode < 3; mode++)
    for (affinity = 0; affinity < 2; affinity++) {
        char raw_source[8192], raw_expected[8192]; xui_document d;
        admonition_source(ci, tag, 0, raw_source, raw_expected, sizeof(raw_source)); d = open_md(raw_source); xui_document_snapshot before;
        xui_doc_range_t range = find_text(d, "take"), selected; xui_widget editor;
        xui_doc_editor_desc_t desc = {0}; xui_doc_command_state_t state = {0}; char actual[8192]; uint64_t revision;
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        range.tAnchor = unwrap_position(before, range.tAnchor, gap);
        range.tAnchor.iAffinity = affinity; if (!gap) range.tAnchor.iOffset += 2;
        range.tCaret = range.tAnchor; xuiDocumentSnapshotRelease(before); revision = xuiDocumentGetRevision(d);
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d;
        desc.iMode = mode == 2 ? XUI_DOC_SOURCE_TEXT : mode == 1 ? XUI_DOC_LIVE_MARKDOWN : XUI_DOC_VISUAL;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK && xuiWidgetSetRect(editor, (xui_rect_t){0,0,640,480}) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        if (mode) {
            state.iSize = sizeof(state);
            CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK && !state.bEnabled &&
                state.iDisabledReason == XUI_ERROR_UNSUPPORTED && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_ERROR_UNSUPPORTED &&
                xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, raw_source)); refused++;
            CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        }
        state.iSize = sizeof(state); CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK && state.bEnabled && state.bActive);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK && xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &selected) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, raw_expected));
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK && xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
            xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, raw_source));
        CHECK(!xuiDocumentCanUndo(d) && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, raw_expected));
        xuiWidgetDestroy(editor); xuiDocumentRelease(d); total++;
    }
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Admonition source Editor: %u actual DLL VISUAL Query/Execute/Caret cases, %u LIVE/SOURCE atomic refusals and three-mode history\n", total, refused);
}
#endif
int main(void)
{
    admonition_matrix(); admonition_empty_and_dialects(); admonition_shift_and_native();
    admonition_depth_and_compound(); admonition_footnote_metadata(); admonition_large_source(); admonition_rich(); admonition_failures();
#ifndef XUI_DLL
    admonition_cancellation();
#endif
#ifdef XUI_QUOTE_TEST_EDITOR
    admonition_editor();
#endif
    return 0;
}
