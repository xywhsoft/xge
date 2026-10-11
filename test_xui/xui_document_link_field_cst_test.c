#define main xui_document_core_uninvoked
#include "xui_document_test.c"
#undef main

typedef struct link_field_case {
    const char* destination;
    unsigned content_start, content_end;
    const char* gap;
    const char* title;
} link_field_case;
static const link_field_case field_cases[] = {
    {"old", 0, 3, "", ""}, {"<old>", 1, 4, " ", "\"title\""},
    {"old(1)", 0, 6, "\t", "'title'"}, {"old\\(x\\)", 0, 8, " ", "(a\\)b)"},
    {"old&amp;x", 0, 9, "  ", "\"a &amp; b\""},
    {"<>", 1, 1, " ", "\"\""}, {"", 0, 0, "", ""},
    {"old", 0, 3, " \t ", "'a\\'b'"},
    {"<old>", 1, 4, "\n ", "\"first\n second\""},
    {"old", 0, 3, " ", "()"}, {"old", 0, 3, " ", "''"},
    {"<路径>", 1, 7, " ", "\"标题\""}
};
static const char* field_labels[] = {
    "[**x**]", "![**x**]", "[**x ![y](inner) z**]", "![**x ![y](inner) z**]"
};
static void field_check(xui_document_snapshot s, uint64_t parent, uint32_t kind,
    uint64_t start, uint64_t content_start, uint64_t content_end, uint64_t end)
{
    xui_doc_source_info_t info = {0}; uint64_t i, found = 0;
    info.iSize = sizeof(info); CHECK(xuiDocumentSnapshotGetSourceInfo(s, &info) == XUI_OK);
    for (i = 0; i < info.iInlineSyntaxCount; i++) {
        xui_doc_inline_syntax_t syntax = {0}; syntax.iSize = sizeof(syntax);
        CHECK(xuiDocumentSnapshotGetInlineSyntax(s, i, &syntax) == XUI_OK);
        if (syntax.iKind != kind || syntax.iParentIndex != parent) continue;
        expect_inline_syntax(s, i, kind, parent, start, content_start, content_end, end, UINT64_MAX);
        CHECK(!syntax.iFlags && !syntax.iReserved); found++;
    }
    CHECK(found == 1);
}
static uint64_t field_owner(xui_document_snapshot s, uint64_t start, uint32_t kind)
{
    xui_doc_source_info_t info = {0}; uint64_t i;
    info.iSize = sizeof(info); CHECK(xuiDocumentSnapshotGetSourceInfo(s, &info) == XUI_OK);
    for (i = 0; i < info.iInlineSyntaxCount; i++) {
        xui_doc_inline_syntax_t syntax = {0}; syntax.iSize = sizeof(syntax);
        CHECK(xuiDocumentSnapshotGetInlineSyntax(s, i, &syntax) == XUI_OK);
        if (syntax.iKind == kind && syntax.iSourceStart == start) return i;
    }
    CHECK(0); return UINT64_MAX;
}
static void field_invariants(xui_document_snapshot s)
{
    xui_doc_source_info_t info = {0}; uint64_t i, previous = 0;
    info.iSize = sizeof(info); CHECK(xuiDocumentSnapshotGetSourceInfo(s, &info) == XUI_OK);
    for (i = 0; i < info.iInlineSyntaxCount; i++) {
        xui_doc_inline_syntax_t syntax = {0}, parent = {0}; syntax.iSize = parent.iSize = sizeof(syntax);
        CHECK(xuiDocumentSnapshotGetInlineSyntax(s, i, &syntax) == XUI_OK);
        CHECK(previous <= syntax.iSourceStart && syntax.iSourceStart <= syntax.iContentStart &&
            syntax.iContentStart <= syntax.iContentEnd && syntax.iContentEnd <= syntax.iSourceEnd);
        previous = syntax.iSourceStart;
        if (syntax.iKind == XUI_DOC_SYNTAX_DESTINATION || syntax.iKind == XUI_DOC_SYNTAX_TITLE) {
            CHECK(syntax.iParentIndex < i && xuiDocumentSnapshotGetInlineSyntax(s, syntax.iParentIndex, &parent) == XUI_OK);
            CHECK((parent.iKind == XUI_DOC_SYNTAX_LINK || parent.iKind == XUI_DOC_SYNTAX_IMAGE) &&
                parent.iDefinitionIndex == UINT64_MAX && !(parent.iFlags & XUI_DOC_SYNTAX_AUTOLINK));
            CHECK(parent.iContentEnd < syntax.iSourceStart && syntax.iSourceEnd < parent.iSourceEnd);
        }
    }
}
static void field_full_oracle(xui_document d, xui_document_snapshot actual)
{
    char source[8192]; uint64_t bytes; xui_document fresh; xui_document_snapshot expected;
    xui_doc_desc_t desc = {0}; desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.iMarkdownDialect = xuiDocumentGetMarkdownDialect(d);
    CHECK(xuiDocumentSnapshotCopySource(actual, source, sizeof(source), &bytes) == XUI_OK);
    CHECK(xuiDocumentCreate(&desc, &fresh) == XUI_OK && xuiDocumentLoadMarkdown(fresh, source, bytes) == XUI_OK &&
        xuiDocumentAcquireSnapshot(fresh, &expected) == XUI_OK);
    inc_snapshot_equal(actual, expected); expect_same_syntax(actual, expected); field_invariants(actual);
    xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(fresh);
}
static void field_matrix(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"};
    unsigned fixture, label, eol, container, dialect, native, mode, cases = 0;
    for (fixture = 0; fixture < sizeof(field_cases)/sizeof(*field_cases); fixture++)
    for (label = 0; label < 4; label++) for (eol = 0; eol < 3; eol++) for (container = 0; container < 3; container++)
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) for (native = 0; native < 2; native++)
    for (mode = 0; mode < 3; mode++) {
        const link_field_case* f = &field_cases[fixture]; const char* prefix = container == 1 ? "> " : container == 2 ? "  " : "";
        char raw[1024], source[4096]; uint64_t map[1024], used, j, dst, title, owner, fields = 0;
        xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot s, held, restored;
        xui_doc_source_info_t info = {0};
        strcpy(raw, field_labels[label]); strcat(raw, "( \t"); dst = strlen(raw); strcat(raw, f->destination);
        strcat(raw, f->gap); title = strlen(raw); strcat(raw, f->title); strcat(raw, " \t)");
        if (!*f->destination) dst = strlen(raw) - 1;
        strcpy(source, native ? "\xef\xbb\xbfprior `span`\n\n" : "prior `span`\n\n"); used = strlen(source);
        if (container == 2) { memcpy(source + used, "- ", 2); used += 2; }
        else { memcpy(source + used, prefix, strlen(prefix)); used += strlen(prefix); }
        for (j = 0; raw[j]; j++) {
            map[j] = used;
            if (raw[j] == '\n') {
                memcpy(source + used, endings[eol], strlen(endings[eol])); used += strlen(endings[eol]);
                memcpy(source + used, prefix, strlen(prefix)); used += strlen(prefix);
            } else source[used++] = raw[j];
        }
        map[j] = used; strcpy(source + used, "\n\nfollowing `span`\n");
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        {
            int result = xuiDocumentLoadMarkdown(d, source, strlen(source));
            if (result) fprintf(stderr, "field CST load fixture=%u label=%u eol=%u container=%u dialect=%u native=%u mode=%u result=%d\n",
                fixture, label, eol, container, dialect, native, mode, result);
            CHECK(result == XUI_OK);
        }
        if (native) {
            char* wire; uint64_t bytes; xui_document loaded;
            CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && xuiDocumentSerialize(s, &wire, &bytes) == XUI_OK &&
                xuiDocumentDeserialize(NULL, wire, bytes, &loaded) == XUI_OK);
            xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d); d = loaded;
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &held) == XUI_OK);
        if (mode) {
            const char* shift = "earlier **span**\n\n"; uint64_t shift_bytes = strlen(shift);
            if (mode == 1) {
                xui_document_transaction t; xui_doc_txn_desc_t td = {0}; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
                CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK && xuiDocumentTxnReplaceSource(t, 0, 0, shift, shift_bytes) == XUI_OK &&
                    xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
            } else {
                xui_doc_source_patch_t p = {0}; xui_document_prepare prepare; p.iSize = sizeof(p); p.sText = shift; p.iTextBytes = shift_bytes;
                CHECK(xuiDocumentPrepareSource(d, NULL, &p, 1, &prepare) == XUI_OK && xuiDocumentPrepareRun(prepare) == XUI_OK &&
                    xuiDocumentPreparePublish(d, prepare, NULL) == XUI_OK); xuiDocumentPrepareRelease(prepare);
            }
            for (j = 0; j <= strlen(raw); j++) map[j] += shift_bytes;
        }
        CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
        owner = field_owner(s, map[0], (label & 1) ? XUI_DOC_SYNTAX_IMAGE : XUI_DOC_SYNTAX_LINK);
        field_check(s, owner, XUI_DOC_SYNTAX_DESTINATION, map[dst], map[dst + f->content_start],
            map[dst + f->content_end], map[dst + strlen(f->destination)]);
        if (*f->title) field_check(s, owner, XUI_DOC_SYNTAX_TITLE, map[title], map[title + 1],
            map[title + strlen(f->title) - 1], map[title + strlen(f->title)]);
        if (label >= 2) {
            uint64_t inner = (uint64_t)(strstr(raw, "![y]") - raw), inner_dst = (uint64_t)(strstr(raw, "inner") - raw);
            field_check(s, field_owner(s, map[inner], XUI_DOC_SYNTAX_IMAGE), XUI_DOC_SYNTAX_DESTINATION,
                map[inner_dst], map[inner_dst], map[inner_dst + 5], map[inner_dst + 5]);
        }
        info.iSize = sizeof(info); CHECK(xuiDocumentSnapshotGetSourceInfo(s, &info) == XUI_OK);
        for (j = 0; j < info.iInlineSyntaxCount; j++) {
            xui_doc_inline_syntax_t token = {0}; token.iSize = sizeof(token);
            CHECK(xuiDocumentSnapshotGetInlineSyntax(s, j, &token) == XUI_OK);
            if (token.iKind == XUI_DOC_SYNTAX_DESTINATION || token.iKind == XUI_DOC_SYNTAX_TITLE) fields++;
        }
        CHECK(fields == 1u + !!*f->title + (label >= 2)); field_full_oracle(d, s);
        if (mode) {
            CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            expect_same_syntax(held, restored); expect_identity_tree(held, restored, 1); xuiDocumentSnapshotRelease(restored);
            CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            expect_same_syntax(s, restored); expect_identity_tree(s, restored, 1); xuiDocumentSnapshotRelease(restored);
        }
        xuiDocumentSnapshotRelease(held); xuiDocumentRelease(d); field_invariants(s); xuiDocumentSnapshotRelease(s); cases++;
    }
    printf("Link field CST golden: %u exact independent field-token/value ranges, empty/escaped/entity/Unicode/multiline/container/nested, native, lazy SOURCE/Prepare shifts and history cases passed\n", cases);
}
static void field_differential(void)
{
    const char* source = "before **a**\n\n> [**x ![y](inner) z**](<old>\r\n> \"first\r\n> second\")\r\n\n![y][ref]\n\n[ref]: other 'title'\n\nafter\n";
    uint64_t offset; unsigned kind, cases = 0;
    for (offset = 0; offset <= strlen(source); offset++) for (kind = 0; kind < 3; kind++) {
        xui_document d = test_markdown_open(source); xui_document_transaction t; xui_document_snapshot s, held, restored;
        xui_doc_txn_desc_t td = {0}; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &held) == XUI_OK);
        CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK && xuiDocumentTxnReplaceSource(t, offset,
            offset < strlen(source) && kind == 2 ? offset + 1 : offset, kind == 2 ? "" : kind == 1 ? "\n" : "x", kind == 2 ? 0 : 1) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); field_full_oracle(d, s);
        if (!(kind == 2 && offset == strlen(source))) {
            CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            expect_same_syntax(held, restored); expect_identity_tree(held, restored, 1); xuiDocumentSnapshotRelease(restored);
        }
        xuiDocumentSnapshotRelease(held); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d); cases++;
    }
    printf("Link field CST differential: %u every-byte delimiter/line/reference/nested changes, full syntax/semantic oracle and history cases passed\n", cases);
}
static uint64_t field_image(xui_document_snapshot s, uint64_t id)
{
    xui_doc_node_info_t node = {0}; uint64_t i; node.iSize = sizeof(node);
    CHECK(xuiDocumentSnapshotGetNode(s, id, &node) == XUI_OK);
    if (node.iKind == XUI_DOC_IMAGE) return id;
    for (i = 0; i < node.iChildCount; i++) {
        uint64_t child, found; CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK);
        found = field_image(s, child); if (found) return found;
    }
    return 0;
}
static void field_container(char* output, const char* raw, unsigned eol, unsigned container)
{
    const char* endings[] = {"\n", "\r\n", "\r"};
    const char* prefix = container == 1 ? "> " : container == 2 ? "  " : "";
    uint64_t used, i;
    strcpy(output, "prior `span`\n\n"); used = strlen(output);
    strcpy(output + used, container == 2 ? "- " : prefix); used = strlen(output);
    for (i = 0; raw[i]; i++) {
        if (raw[i] == '\n') {
            strcpy(output + used, endings[eol]); used = strlen(output);
            strcpy(output + used, prefix); used = strlen(output);
        } else output[used++] = raw[i];
    }
    strcpy(output + used, "\n\nfollowing  &amp; untouched\n");
}
static void field_commands(void)
{
    unsigned fixture, label, eol, container, operation, mode, cases = 0;
    for (fixture = 0; fixture < sizeof(field_cases)/sizeof(*field_cases); fixture++)
    for (label = 1; label < 4; label += 2) for (eol = 0; eol < 3; eol++) for (container = 0; container < 3; container++)
    for (operation = 0; operation < 5; operation++) for (mode = 0; mode < 3; mode++) {
        const link_field_case* f = &field_cases[fixture]; char raw[1024], source[4096], expected_raw[1024], expected[4096], alt[512];
        const char* uri; const char* title; uint64_t id, bytes, i, source_ops = 0, revision;
        int uri_changed, title_changed;
        xui_document d; xui_document_transaction t; xui_document_snapshot before, after, restored;
        xui_document_change_set change; xui_doc_node_info_t node = {0}; xui_doc_image_desc_t image = {0}; xui_doc_change_info_t info = {0};
        strcpy(raw, field_labels[label]); strcat(raw, "( \t"); strcat(raw, f->destination); strcat(raw, f->gap); strcat(raw, f->title); strcat(raw, " \t)");
        field_container(source, raw, eol, container); d = test_markdown_open(source);
        if (mode) {
            const char* shift = "earlier **span**\n\n";
            if (mode == 1) {
                xui_doc_txn_desc_t td = {0}; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
                CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK && xuiDocumentTxnReplaceSource(t, 0, 0, shift, strlen(shift)) == XUI_OK &&
                    xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
            } else {
                xui_doc_source_patch_t edit = {0}; xui_document_prepare p; edit.iSize = sizeof(edit); edit.sText = shift; edit.iTextBytes = strlen(shift);
                CHECK(xuiDocumentPrepareSource(d, NULL, &edit, 1, &p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_OK &&
                    xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); xuiDocumentPrepareRelease(p);
            }
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
        id = field_image(before, 1); node.iSize = sizeof(node);
        CHECK(id && xuiDocumentSnapshotGetNode(before, id, &node) == XUI_OK && xuiDocumentSnapshotCopyText(before, id, alt, sizeof(alt), &bytes) == XUI_OK);
        uri = operation == 1 || operation == 2 || operation == 4 ? node.sResource : "new";
        title = operation == 0 ? node.sTitle : operation == 2 ? "" : operation == 4 ? "next\nline\rend" : "next";
        uri_changed = strcmp(uri, node.sResource) != 0;
        title_changed = strcmp(title ? title : "", node.sTitle ? node.sTitle : "") != 0;
        strcpy(expected_raw, field_labels[label]); strcat(expected_raw, "( \t");
        if (uri_changed) strcat(expected_raw, "<new>");
        else if (!*f->destination && title_changed && title && *title) strcat(expected_raw, "<>");
        else strcat(expected_raw, f->destination);
        strcat(expected_raw, f->gap);
        if (title_changed) {
            if (title && *title) { if (!*f->title) strcat(expected_raw, " "); strcat(expected_raw, operation == 4 ? "\"next&#10;line&#13;end\"" : "\"next\""); }
        } else strcat(expected_raw, f->title);
        strcat(expected_raw, " \t)"); field_container(expected, expected_raw, eol, container);
        if (!*f->destination) {
            /* The omitted destination is at the closing ')': all preceding
             * whitespace remains before the newly inserted token. */
            strcpy(expected_raw, field_labels[label]); strcat(expected_raw, "( \t \t");
            if (uri_changed) strcat(expected_raw, "<new>");
            else if (title_changed && title && *title) strcat(expected_raw, "<>");
            if (title_changed && title && *title) strcat(expected_raw, operation == 4 ? " \"next&#10;line&#13;end\"" : " \"next\"");
            strcat(expected_raw, ")"); field_container(expected, expected_raw, eol, container);
        }
        if (mode) { uint64_t shift = strlen("earlier **span**\n\n"); memmove(expected + shift, expected, strlen(expected) + 1); memcpy(expected, "earlier **span**\n\n", shift); }
        image.iSize = sizeof(image); image.sResource = uri; image.sTitle = title; image.sAlt = alt; image.iAltBytes = bytes;
        revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        {
            int result = xuiDocumentTxnUpdateImage(t, id, &image);
            if (result) fprintf(stderr, "field command fixture=%u label=%u eol=%u container=%u operation=%u mode=%u result=%d\n",
                fixture, label, eol, container, operation, mode, result);
            CHECK(result == XUI_OK && xuiDocumentTxnCommit(t, &change) == XUI_OK);
        }
        xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        {
            char actual[8192]; uint64_t actual_bytes;
            CHECK(xuiDocumentSnapshotCopySource(after, actual, sizeof(actual), &actual_bytes) == XUI_OK);
            if (strcmp(actual, expected)) fprintf(stderr, "field command spelling fixture=%u label=%u eol=%u container=%u operation=%u mode=%u\nactual: %s\nexpected: %s\n",
                fixture, label, eol, container, operation, mode, actual, expected);
            CHECK(actual_bytes == strlen(expected) && !memcmp(actual, expected, actual_bytes));
        }
        if (uri_changed || title_changed) {
            info.iSize = sizeof(info); CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
            for (i = 0; i < info.iOperationCount; i++) if (info.pOperations[i].iKind == XUI_DOC_OP_SOURCE) source_ops++;
        } else CHECK(!change);
        CHECK(source_ops == (uint64_t)((uri_changed && title_changed && *f->title) ? 2 : (uri_changed || title_changed)));
        field_full_oracle(d, after); expect_text(after, id, alt);
        {
            xui_doc_node_info_t updated = {0}; updated.iSize = sizeof(updated);
            CHECK(xuiDocumentSnapshotGetNode(after, id, &updated) == XUI_OK && !strcmp(updated.sResource, uri) &&
                !strcmp(updated.sTitle ? updated.sTitle : "", title ? title : ""));
        }
        if (uri_changed || title_changed) {
            CHECK(xuiDocumentIsDirty(d) && xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            expect_same_syntax(before, restored); expect_identity_tree(before, restored, 1); xuiDocumentSnapshotRelease(restored);
            CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            expect_same_syntax(after, restored); expect_identity_tree(after, restored, 1); xuiDocumentSnapshotRelease(restored);
        } else CHECK(xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
        xuiDocumentChangeSetRelease(change); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d);
        field_invariants(after); xuiDocumentSnapshotRelease(after); cases++;
    }
    printf("Link field image commands: %u independent exact whole-source URI-only/title-only/removal/joint/no-op spellings, trivia/alt/nested/Unicode/EOL, minimal source operations and full oracle/history cases passed\n", cases);
}
static void field_faults(void)
{
    const char* inputs[] = {"![**x**](<old> 'title')\n", "> ![**x ![y](inner) z**](<old>\r\n> \"first\r\n> second\")\r\n", "![x](<> \"\")\n"};
    unsigned sample; long oom = 0, cancelled = 0;
    for (sample = 0; sample < sizeof(inputs)/sizeof(*inputs); sample++) {
        long point; int succeeded = 0;
        for (point = 0; point < 4096 && !succeeded; point++) {
            fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t = NULL;
            xui_document_snapshot before, after; xui_doc_node_info_t node = {0}; xui_doc_image_desc_t image = {0};
            char alt[512]; uint64_t id, bytes, revision; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, inputs[sample], strlen(inputs[sample])) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
            id = field_image(before, 1); node.iSize = sizeof(node);
            CHECK(xuiDocumentSnapshotGetNode(before, id, &node) == XUI_OK && xuiDocumentSnapshotCopyText(before, id, alt, sizeof(alt), &bytes) == XUI_OK);
            image.iSize = sizeof(image); image.sResource = "new"; image.sTitle = "next"; image.sAlt = alt; image.iAltBytes = bytes;
            revision = xuiDocumentGetRevision(d); a.remaining = point;
            result = xuiDocumentBeginTransaction(d, NULL, &t);
            if (result == XUI_OK) result = xuiDocumentTxnUpdateImage(t, id, &image);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            else if (t) CHECK(xuiDocumentTxnCommit(t, NULL) == result);
            a.remaining = -1; xuiDocumentTxnRelease(t);
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result != XUI_OK) {
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
                inc_snapshot_equal(before, after); expect_same_syntax(before, after); expect_identity_tree(before, after, 1); oom++;
            } else { field_full_oracle(d, after); succeeded = 1; }
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(succeeded); succeeded = 0;
        for (point = 0; point < 4096 && !succeeded; point++) {
            prepare_cancel_allocator a = {0, -1, 0, NULL}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_prepare p; xui_document_snapshot before, after; xui_doc_source_patch_t edit = {0};
            uint64_t revision; int result;
            const char* replacement = "[a](<one> \"\") [b](two 'title') [c](three) [d](four) [e](five)\n\n![**x ![y](inner) z**](<>\n \"multi\n line\")\n";
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.onAlloc = prepare_cancel_alloc; desc.onFree = prepare_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, inputs[sample], strlen(inputs[sample])) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
            edit.iSize = sizeof(edit); edit.iEnd = strlen(inputs[sample]); edit.sText = replacement; edit.iTextBytes = strlen(replacement);
            revision = xuiDocumentGetRevision(d); CHECK(xuiDocumentPrepareSource(d, NULL, &edit, 1, &p) == XUI_OK);
            a.prepare = p; a.remaining = point; result = xuiDocumentPrepareRun(p); a.prepare = NULL;
            if (a.fired) {
                CHECK(result == XUI_DOC_ERROR_CANCELLED && xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_CANCELLED &&
                    xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
                CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); inc_snapshot_equal(before, after); expect_same_syntax(before, after);
                expect_identity_tree(before, after, 1); cancelled++;
            } else {
                CHECK(result == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
                field_full_oracle(d, after); succeeded = 1;
            }
            xuiDocumentPrepareRelease(p); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(succeeded);
    }
    printf("Link field faults: %ld image-command/publication OOM, %ld Prepare cancellation checkpoints, complete source/syntax/identity/history/savepoint state atomic and leak-free\n", oom, cancelled);
}
int main(void)
{
    field_matrix(); field_differential(); field_commands(); field_faults(); inline_source_syntax();
    puts("Link field CST tests passed"); return 0;
}
