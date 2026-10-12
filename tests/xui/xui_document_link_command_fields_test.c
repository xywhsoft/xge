#define main xui_document_core_uninvoked
#include "xui_document_test.c"
#undef main

typedef struct link_spelling { const char *destination, *gap, *title; int reference; } link_spelling;
static const link_spelling spellings[] = {
    {"old", " \t", "'title'", 0}, {"<old>", "\t ", "\"ti&#116;le\"", 0},
    {"o&#108;d", "\n ", "(title)", 0}, {"old", " \t", "", 0},
    {"<old>", " \t", "''", 0}, {"", "", "", 0},
    {"old", "", "'title'", 1}
};
static const char* labels[] = {
    "alpha", "**alpha**", "foo __bar__ &amp; \\!", "` a  b `", "\\[bracket\\]",
    "\xe4\xb8\xad\xe6\x96\x87 &amp; \\*", "foo\nbar", "foo  \nbar", "foo\\\nbar",
    "![alt](asset 'keep')", "![](asset)", "before ![**alt** ![inner](hidden)](asset 'keep') after",
    "<i title=\"a\">alpha</i>", "**![alt](asset)**", "_a_ __b__"
};
static xui_doc_range_t whole_link(xui_document d, xui_document_snapshot s, uint64_t id, int text_edges)
{
    xui_doc_node_info_t n = {0}; xui_doc_range_t range = {0}; uint64_t i, first = UINT64_MAX, last = 0;
    n.iSize = sizeof(n); CHECK(xuiDocumentSnapshotGetNode(s, id, &n) == XUI_OK);
    for (i = 0; i < n.iChildCount; i++) {
        xui_doc_node_info_t child = {0}; uint64_t child_id; child.iSize = sizeof(child);
        CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child_id) == XUI_OK && xuiDocumentSnapshotGetNode(s, child_id, &child) == XUI_OK);
        if (child.tAttributes.iMarks & XUI_DOC_LINK) { if (first == UINT64_MAX) first = i; last = i; }
    }
    if (first != UINT64_MAX) {
        xui_doc_node_info_t a = {0}, b = {0}; uint64_t first_id, last_id;
        range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d);
        range.tAnchor.iRevision = xuiDocumentGetRevision(d); range.tAnchor.iNodeId = id;
        range.tAnchor.iKind = XUI_DOC_POSITION_GAP; range.tAnchor.iOffset = first;
        range.tCaret = range.tAnchor; range.tCaret.iOffset = last + 1;
        CHECK(xuiDocumentSnapshotGetChild(s, id, first, &first_id) == XUI_OK && xuiDocumentSnapshotGetChild(s, id, last, &last_id) == XUI_OK);
        a.iSize = sizeof(a); b.iSize = sizeof(b);
        CHECK(xuiDocumentSnapshotGetNode(s, first_id, &a) == XUI_OK && xuiDocumentSnapshotGetNode(s, last_id, &b) == XUI_OK);
        if (text_edges && a.iKind != XUI_DOC_SOFT_BREAK && a.iKind != XUI_DOC_HARD_BREAK &&
            b.iKind != XUI_DOC_SOFT_BREAK && b.iKind != XUI_DOC_HARD_BREAK && (first_id != last_id || b.iTextBytes)) {
            range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_TEXT;
            range.tAnchor.iNodeId = first_id; range.tAnchor.iOffset = 0;
            range.tCaret.iNodeId = last_id; range.tCaret.iOffset = b.iTextBytes;
        }
        return range;
    }
    for (i = 0; i < n.iChildCount; i++) {
        uint64_t child; CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK);
        range = whole_link(d, s, child, text_edges); if (range.tAnchor.iSize) return range;
    }
    return range;
}
static void container_source(char* output, const char* raw, unsigned eol, unsigned container)
{
    const char* endings[] = {"\n", "\r\n", "\r"}; const char* p = raw; unsigned line = 0;
    strcpy(output, "before `untouched`\n\n");
    do {
        const char* begin = p;
        strcat(output, container == 1 ? "> " : container == 2 ? (line ? "  " : "- ") : "");
        while (*p && *p != '\n') p++;
        strncat(output, begin, (size_t)(p - begin)); strcat(output, endings[eol]);
        if (*p) p++;
        line++;
    } while (*p);
    strcat(output, endings[eol]);
    strcat(output, "[REF]: old 'title'\n\nfollowing &amp; untouched  \n");
}
static void full_oracle(xui_document d, xui_document_snapshot actual)
{
    char source[8192]; uint64_t bytes; xui_doc_desc_t desc = {0}; xui_document loaded; xui_document_snapshot expected;
    CHECK(xuiDocumentSnapshotCopySource(actual, source, sizeof(source), &bytes) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = xuiDocumentGetMarkdownDialect(d);
    CHECK(xuiDocumentCreate(&desc, &loaded) == XUI_OK && xuiDocumentLoadMarkdown(loaded, source, bytes) == XUI_OK &&
        xuiDocumentAcquireSnapshot(loaded, &expected) == XUI_OK);
    inc_snapshot_equal(expected, actual); expect_same_syntax(expected, actual);
    xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(loaded);
}
static void matrix(void)
{
    unsigned label, spelling, op, eol, container, dialect, edges, cases = 0;
    for (label = 0; label < sizeof(labels)/sizeof(*labels); label++)
    for (spelling = 0; spelling < sizeof(spellings)/sizeof(*spellings); spelling++)
    for (op = 0; op < 6; op++) for (eol = 0; eol < 3; eol++) for (container = 0; container < 3; container++)
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) for (edges = 0; edges < 2; edges++) {
        const link_spelling* f = &spellings[spelling]; int uri_changed = op == 0 || op == 3 || op == 4;
        const char* old_title = spelling == 3 || spelling == 4 || spelling == 5 ? "" : "title";
        const char* uri = uri_changed ? "new" : spelling == 5 ? "" : "old";
        const char* title = op == 1 || op == 3 ? "next" : op == 2 ? "" : op == 4 ? "next\nline\rend" : old_title;
        int title_changed = strcmp(old_title, title) != 0;
        char raw[2048], expected_raw[2048], source[8192], expected_source[8192], actual[8192];
        xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t; xui_document_change_set changes = NULL;
        xui_document_snapshot before, after, restored; xui_doc_range_t range; xui_doc_link_info_t query = {0};
        uint64_t bytes, revision; unsigned shift = (label + spelling + op + eol + container + dialect + edges) % 3;
        if (spelling == 5 && !uri_changed) continue;
        snprintf(raw, sizeof(raw), f->reference ? "[%s][REF]" : "[%s]( \t%s%s%s \t)", labels[label], f->destination, f->gap, f->title);
        snprintf(expected_raw, sizeof(expected_raw), "[%s]", labels[label]);
        if (f->reference && (uri_changed || title_changed)) {
            strcat(expected_raw, uri_changed ? "(<new>" : "(<old>");
            if (*title) strcat(expected_raw, op == 4 ? " \"next&#10;line&#13;end\"" : op == 1 || op == 3 ? " \"next\"" : " \"title\"");
            strcat(expected_raw, ")");
        } else if (f->reference) strcat(expected_raw, "[REF]");
        else if (spelling == 5) {
            strcat(expected_raw, "( \t \t<new>");
            if (*title) strcat(expected_raw, op == 4 ? " \"next&#10;line&#13;end\"" : " \"next\"");
            strcat(expected_raw, ")");
        } else {
            strcat(expected_raw, "( \t"); strcat(expected_raw, uri_changed ? "<new>" : f->destination);
            if (*f->title) strcat(expected_raw, f->gap);
            if (title_changed) {
                if (*title) { if (!*f->title) strcat(expected_raw, " "); strcat(expected_raw, op == 4 ? "\"next&#10;line&#13;end\"" : "\"next\""); }
            } else strcat(expected_raw, f->title);
            if (!*f->title) strcat(expected_raw, f->gap);
            strcat(expected_raw, " \t)");
        }
        container_source(source, raw, eol, container); container_source(expected_source, expected_raw, eol, container);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        if ((label + spelling + op + eol) % 2) {
            char* wire; xui_document loaded;
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSerialize(before, &wire, &bytes) == XUI_OK &&
                xuiDocumentDeserialize(NULL, wire, bytes, &loaded) == XUI_OK);
            xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); d = loaded;
        }
        if (shift) {
            const char* prefix = "earlier **span**\n\n"; uint64_t length = strlen(prefix);
            if (shift == 1) {
                xui_doc_txn_desc_t td = {0}; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
                CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK && xuiDocumentTxnReplaceSource(t, 0, 0, prefix, length) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
                xuiDocumentTxnRelease(t);
            } else {
                xui_doc_source_patch_t patch = {0}; xui_document_prepare prepare;
                patch.iSize = sizeof(patch); patch.sText = prefix; patch.iTextBytes = length;
                CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &prepare) == XUI_OK && xuiDocumentPrepareRun(prepare) == XUI_OK && xuiDocumentPreparePublish(d, prepare, NULL) == XUI_OK);
                xuiDocumentPrepareRelease(prepare);
            }
            memmove(expected_source + length, expected_source, strlen(expected_source) + 1); memcpy(expected_source, prefix, (size_t)length);
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
        range = whole_link(d, before, 1, edges);
        if (!range.tAnchor.iSize) fprintf(stderr, "missing link label=%u spelling=%u op=%u eol=%u container=%u dialect=%u edges=%u source:%s\n", label, spelling, op, eol, container, dialect, edges, source);
        CHECK(range.tAnchor.iSize);
        if ((label + op + eol + dialect) % 2) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        query.iSize = sizeof(query);
        CHECK(xuiDocumentSnapshotQueryLink(before, &range, &query) == XUI_OK && query.bLinked && !query.bMixed &&
            !strcmp(query.sUri, spelling == 5 ? "" : "old") && !strcmp(query.sTitle, old_title));
        revision = xuiDocumentGetRevision(d); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        {
            int result = xuiDocumentTxnSetLink(t, &range, uri, title);
            if (result) fprintf(stderr, "link command label=%u spelling=%u op=%u eol=%u container=%u dialect=%u edges=%u result=%d\n", label, spelling, op, eol, container, dialect, edges, result);
            CHECK(result == XUI_OK && xuiDocumentTxnCommit(t, &changes) == XUI_OK);
        }
        xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK && xuiDocumentSnapshotCopySource(after, actual, sizeof(actual), &bytes) == XUI_OK);
        if (strcmp(actual, expected_source)) fprintf(stderr, "link spelling label=%u spelling=%u op=%u eol=%u container=%u dialect=%u edges=%u\nactual:%s\nexpected:%s\n", label, spelling, op, eol, container, dialect, edges, actual, expected_source);
        CHECK(bytes == strlen(expected_source) && !memcmp(actual, expected_source, (size_t)bytes));
        full_oracle(d, after);
        range = whole_link(d, after, 1, edges); query.iSize = sizeof(query);
        CHECK(xuiDocumentSnapshotQueryLink(after, &range, &query) == XUI_OK && query.bLinked && !query.bMixed && !strcmp(query.sUri, uri) && !strcmp(query.sTitle, title));
        if (uri_changed || title_changed) {
            xui_doc_change_info_t info = {0}; uint64_t i, source_ops = 0; info.iSize = sizeof(info);
            CHECK(xuiDocumentChangeSetGetInfo(changes, &info) == XUI_OK);
            for (i = 0; i < info.iOperationCount; i++) if (info.pOperations[i].iKind == XUI_DOC_OP_SOURCE) source_ops++;
            CHECK(source_ops == (uint64_t)(!f->reference && uri_changed && title_changed && *f->title ? 2 : 1));
            CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            inc_snapshot_equal(before, restored); expect_same_syntax(before, restored); expect_identity_tree(before, restored, 1); xuiDocumentSnapshotRelease(restored);
            CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            inc_snapshot_equal(after, restored); expect_same_syntax(after, restored); expect_identity_tree(after, restored, 1); xuiDocumentSnapshotRelease(restored);
        } else CHECK(!changes && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
        xuiDocumentChangeSetRelease(changes); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); cases++;
    }
    printf("Link command fields golden: %u independent whole-source spellings; formatted/entity/code/Unicode/multinode/break/image/HTML labels, reference definitions, forward/reverse TEXT/GAP, dialect/EOL/container/native/SOURCE/Prepare, minimal SOURCE operations, full oracle and history passed\n", cases);
}
static void faults(void)
{
    const char* inputs[] = {"[**alpha**](old 'title')\n", "> [foo  \r\n> ![alt](asset 'keep') bar](old 'title')\r\n", "[![![](hidden)](asset)][REF]\n\n[REF]: old 'title'\n"};
    unsigned sample; long faults = 0;
    for (sample = 0; sample < sizeof(inputs)/sizeof(*inputs); sample++) {
        long point; int succeeded = 0;
        for (point = 0; point < 4096 && !succeeded; point++) {
            fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t = NULL;
            xui_document_snapshot before, after; xui_doc_range_t range; uint64_t revision; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, inputs[sample], strlen(inputs[sample])) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK &&
                xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
            range = whole_link(d, before, 1, sample % 2); CHECK(range.tAnchor.iSize); revision = xuiDocumentGetRevision(d); a.remaining = point;
            result = xuiDocumentBeginTransaction(d, NULL, &t);
            if (result == XUI_OK) result = xuiDocumentTxnSetLink(t, &range, "new", "next\nline\rend");
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            else if (t) CHECK(xuiDocumentTxnCommit(t, NULL) == result);
            a.remaining = -1; xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result != XUI_OK) {
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
                inc_snapshot_equal(before, after); expect_same_syntax(before, after); expect_identity_tree(before, after, 1); faults++;
            } else { full_oracle(d, after); succeeded = 1; }
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(succeeded);
    }
    printf("Link command fields faults: %ld OOM checkpoints; source, semantic/CST identity, revision/history/savepoint publication atomic and leak-free\n", faults);
}
static void boundary_and_compound(void)
{
    const char* input = "[foo __bar__ ![alt](asset 'keep')]( \told \t 'title' \t)\n";
    const char* expected = "[foo __bar__ ![alt](asset 'keep')]( \t<third> \"next&#10;line&#13;end\" \t  \t)\n";
    xui_document d = test_markdown_open(input); xui_document_snapshot before, after, restored;
    xui_document_transaction t; xui_doc_range_t range;
    CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
    range = whole_link(d, before, 1, 0); CHECK(range.tAnchor.iSize);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
        xuiDocumentTxnSetLink(t, &range, "first", "one") == XUI_OK &&
        xuiDocumentTxnSetLink(t, &range, "second", "") == XUI_OK &&
        xuiDocumentTxnSetLink(t, &range, "third", "next\nline\rend") == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
    xuiDocumentTxnRelease(t); test_source(d, expected, 1);
    CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); full_oracle(d, after);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
    expect_identity_tree(before, restored, 1); expect_same_syntax(before, restored); xuiDocumentSnapshotRelease(restored);
    CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
    expect_identity_tree(after, restored, 1); expect_same_syntax(after, restored); xuiDocumentSnapshotRelease(restored);
    xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    d = test_markdown_open("[alpha](/old 'title') [beta](/other)\n");
    range = test_find(d, "lph");
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnSetLink(t, &range, "/new", "next") == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
    xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); full_oracle(d, after);
    {
        const char* contents[] = {"a", "lph", "a", "beta"}; const char* targets[] = {"/old", "/new", "/old", "/other"};
        uint64_t paragraph, i, index = 0; xui_doc_node_info_t parent = {0}; parent.iSize = sizeof(parent);
        CHECK(xuiDocumentSnapshotGetChild(after, 1, 0, &paragraph) == XUI_OK && xuiDocumentSnapshotGetNode(after, paragraph, &parent) == XUI_OK);
        for (i = 0; i < parent.iChildCount; i++) {
            xui_doc_node_info_t node = {0}; uint64_t child; node.iSize = sizeof(node);
            CHECK(xuiDocumentSnapshotGetChild(after, paragraph, i, &child) == XUI_OK && xuiDocumentSnapshotGetNode(after, child, &node) == XUI_OK);
            if (node.tAttributes.iMarks & XUI_DOC_LINK) {
                CHECK(index < 4 && !strcmp(node.sResource, targets[index])); expect_text(after, child, contents[index]); index++;
            }
        }
        CHECK(index == 4);
    }
    xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    d = test_markdown_open("[![alt](asset 'keep')](old 'title')\n");
    CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK); range = whole_link(d, before, 1, 1); range.tAnchor.iOffset = 1;
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnSetLink(t, &range, "new", "next") == XUI_ERROR_UNSUPPORTED && xuiDocumentTxnCommit(t, NULL) == XUI_ERROR_UNSUPPORTED);
    xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
    inc_snapshot_equal(before, after); expect_same_syntax(before, after); expect_identity_tree(before, after, 1);
    xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    puts("Link command fields boundaries: compound draft shifts/one-step history, partial text semantic guard, neighboring links and partial image atomic rejection passed");
}
static void rich_inline_objects(void)
{
    xui_document d; xui_document_transaction t; xui_document_snapshot before, after, restored;
    xui_doc_node_desc_t image = {0}; xui_doc_range_t range = {0}; xui_doc_link_info_t query = {0};
    uint64_t paragraph, image_id, i; unsigned pass;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    paragraph = insert(t, 1, XUI_DOC_PARAGRAPH, NULL); insert(t, paragraph, XUI_DOC_TEXT, "left");
    image.iSize = sizeof(image); image.iKind = XUI_DOC_IMAGE; image.sResource = "asset"; image.sTitle = "keep"; image.sText = "alt"; image.iTextBytes = 3;
    CHECK(xuiDocumentTxnInsertNode(t, paragraph, 1, &image, &image_id) == XUI_OK);
    insert(t, paragraph, XUI_DOC_SOFT_BREAK, NULL); insert(t, paragraph, XUI_DOC_TEXT, "right");
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
    for (pass = 0; pass < 2; pass++) {
        const char* uri = pass ? "" : "target"; const char* title = pass ? "" : "next\nline\rend";
        range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d);
        range.tAnchor.iNodeId = paragraph; range.tAnchor.iKind = XUI_DOC_POSITION_GAP; range.tAnchor.iOffset = 0;
        range.tCaret = range.tAnchor; range.tCaret.iOffset = 4;
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnSetLink(t, &range, uri, title) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        range.tAnchor.iRevision = range.tCaret.iRevision = xuiDocumentGetRevision(d); query.iSize = sizeof(query);
        CHECK(xuiDocumentSnapshotQueryLink(after, &range, &query) == XUI_OK && query.bLinked == !pass && !query.bMixed);
        if (!pass) CHECK(!strcmp(query.sUri, uri) && !strcmp(query.sTitle, title));
        for (i = 0; i < 4; i++) {
            xui_doc_node_info_t node = {0}; uint64_t child; node.iSize = sizeof(node);
            CHECK(xuiDocumentSnapshotGetChild(after, paragraph, i, &child) == XUI_OK && xuiDocumentSnapshotGetNode(after, child, &node) == XUI_OK && !!(node.tAttributes.iMarks & XUI_DOC_LINK) == !pass);
            if (child == image_id) CHECK(!strcmp(node.sResource, "asset") && !strcmp(node.sTitle, "keep") &&
                !strcmp(node.sLinkTarget ? node.sLinkTarget : "", uri) && !strcmp(node.sLinkTitle ? node.sLinkTitle : "", title));
            else CHECK(!strcmp(node.sResource ? node.sResource : "", uri) && !strcmp(node.sTitle ? node.sTitle : "", title));
        }
        xuiDocumentSnapshotRelease(after);
    }
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
    expect_identity_tree(before, restored, 1); xuiDocumentSnapshotRelease(restored);
    xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d);
    puts("Link command Rich objects: text/image/break common query, multiline title, hyperlink removal preserves image asset/title and Undo identity passed");
}
int main(void)
{
    matrix(); faults(); boundary_and_compound(); rich_inline_objects(); link_range_commands(); link_range_failures();
    puts("Link command fields tests passed"); return 0;
}
