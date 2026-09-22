#include "../xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(test) do { if (!(test)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #test); exit(1); } } while (0)
static unsigned notifications;
static void changed(xui_document d, xui_document_change_set c, void* user)
{
    xui_document_transaction t = NULL;
    (void)c; (void)user;
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_DOC_ERROR_BUSY);
    CHECK(t == NULL); notifications++;
}
static uint64_t insert(xui_document_transaction t, uint64_t parent, unsigned kind, const char* text)
{
    xui_doc_node_desc_t desc = {0}; uint64_t id = 0;
    desc.iSize = sizeof(desc); desc.iKind = kind; desc.sText = text;
    desc.iTextBytes = text ? strlen(text) : 0;
    CHECK(xuiDocumentTxnInsertNode(t, parent, UINT64_MAX, &desc, &id) == XUI_OK);
    return id;
}
static void expect_text(xui_document_snapshot s, uint64_t id, const char* expected)
{
    char buffer[256]; uint64_t n;
    CHECK(xuiDocumentSnapshotCopyText(s, id, buffer, sizeof(buffer), &n) == XUI_OK);
    CHECK(n == strlen(expected) && strcmp(buffer, expected) == 0);
}
static void rich(void)
{
    xui_document d; xui_document_transaction t;
    xui_document_snapshot original, edited, restored;
    xui_document_change_set changes;
    xui_doc_change_info_t ci = {0};
    xui_doc_position_t position = {0}, mapped;
    uint64_t paragraph, text, table, row, cell, cp, ct, token;
    int mapping;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK);
    CHECK(xuiDocumentSubscribe(d, changed, NULL, &token) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    paragraph = insert(t, 1, XUI_DOC_PARAGRAPH, NULL);
    text = insert(t, paragraph, XUI_DOC_TEXT, "hello");
    table = insert(t, 1, XUI_DOC_TABLE, NULL); row = insert(t, table, XUI_DOC_ROW, NULL);
    cell = insert(t, row, XUI_DOC_CELL, NULL); cp = insert(t, cell, XUI_DOC_PARAGRAPH, NULL);
    ct = insert(t, cp, XUI_DOC_TEXT, "cell");
    CHECK(xuiDocumentTxnCommit(t, &changes) == XUI_OK); xuiDocumentTxnRelease(t);
    ci.iSize = sizeof(ci); CHECK(xuiDocumentChangeSetGetInfo(changes, &ci) == XUI_OK && ci.iOperationCount == 7);
    xuiDocumentChangeSetRelease(changes);
    CHECK(notifications == 1);
    CHECK(xuiDocumentAcquireSnapshot(d, &original) == XUI_OK);
    CHECK(xuiDocumentMarkSaved(d, original) == XUI_OK && !xuiDocumentIsDirty(d));
    position.iSize = sizeof(position); position.iKind = XUI_DOC_POSITION_TEXT;
    position.iDocumentId = xuiDocumentGetIdentity(d); position.iRevision = xuiDocumentGetRevision(d);
    position.iNodeId = text; position.iOffset = 5; position.iAffinity = XUI_DOC_AFTER;
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(t, text, 5, 5, " world", 6) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(t, ct, 0, 4, "new", 3) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, &changes) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentMapPosition(changes, &position, &mapped, &mapping) == XUI_OK);
    CHECK(mapped.iOffset == 11 && mapping == XUI_DOC_MAP_EXACT);
    xuiDocumentChangeSetRelease(changes);
    CHECK(notifications == 2 && xuiDocumentIsDirty(d));
    CHECK(xuiDocumentAcquireSnapshot(d, &edited) == XUI_OK);
    expect_text(original, text, "hello"); expect_text(original, ct, "cell");
    expect_text(edited, text, "hello world"); expect_text(edited, ct, "new");
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d));
    CHECK(xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
    expect_text(restored, text, "hello"); expect_text(restored, ct, "cell");
    xuiDocumentSnapshotRelease(restored);
    CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(t, text, 0, 1, "H", 1) == XUI_OK);
    CHECK(xuiDocumentTxnMoveNode(t, table, cell, 0) == XUI_DOC_ERROR_SCHEMA);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_DOC_ERROR_SCHEMA);
    xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); expect_text(restored, text, "hello world");
    xuiDocumentSnapshotRelease(restored); xuiDocumentUnsubscribe(d, token); xuiDocumentRelease(d);
    expect_text(original, text, "hello"); expect_text(edited, ct, "new");
    xuiDocumentSnapshotRelease(original); xuiDocumentSnapshotRelease(edited);
}
static uint64_t find_kind(xui_document_snapshot s, uint64_t parent, unsigned kind)
{
    xui_doc_node_info_t info = {0}; uint64_t i, child, result;
    info.iSize = sizeof(info); CHECK(xuiDocumentSnapshotGetNode(s, parent, &info) == XUI_OK);
    if (info.iKind == kind) return parent;
    for (i = 0; i < info.iChildCount; i++) {
        CHECK(xuiDocumentSnapshotGetChild(s, parent, i, &child) == XUI_OK);
        result = find_kind(s, child, kind); if (result) return result;
    }
    return 0;
}
static void markdown(void)
{
    const char* input = "\xef\xbb\xbf# Title\r\n\r\n- [ ] **bold** &amp; \\*literal\\*\r\n  - nested\r\n\r\n| A | B |\r\n| - | - |\r\n| x | y |\r\n\r\n```c\r\n*literal*\r\n```\r\n\r\nInline $x^2$ and note[^a].\r\n\r\n[^a]: Footnote\r\n";
    xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot s;
    xui_document_transaction t; xui_doc_node_info_t info = {0};
    char source[1024]; uint64_t n, heading, text;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(d, input, strlen(input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(s, source, sizeof(source), &n) == XUI_OK && strcmp(source, input) == 0);
    CHECK(find_kind(s, 1, XUI_DOC_TABLE) && find_kind(s, 1, XUI_DOC_LIST) && find_kind(s, 1, XUI_DOC_FOOTNOTE));
    CHECK(find_kind(s, 1, XUI_DOC_MATH));
    heading = find_kind(s, 1, XUI_DOC_HEADING); CHECK(heading != 0);
    CHECK(xuiDocumentSnapshotGetChild(s, heading, 0, &text) == XUI_OK);
    info.iSize = sizeof(info); CHECK(xuiDocumentSnapshotGetNode(s, text, &info) == XUI_OK);
    expect_text(s, text, "Title"); CHECK(info.bSourceExact);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(t, text, 0, 5, "New *title*", 11) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t); xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(s, source, sizeof(source), &n) == XUI_OK);
    CHECK(strstr(source, "# New \\*title\\*\r\n") != NULL);
    xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(s, source, sizeof(source), &n) == XUI_OK && strcmp(source, input) == 0);
    xuiDocumentRelease(d); xuiDocumentSnapshotRelease(s);
}
typedef struct fail_allocator { long remaining; unsigned live; } fail_allocator;
static void* failing_alloc(void* user, size_t bytes)
{
    fail_allocator* a = user; void* p;
    if (a->remaining == 0) return NULL;
    if (a->remaining > 0) a->remaining--;
    p = malloc(bytes); if (p) a->live++; return p;
}
static void failing_free(void* user, void* pointer)
{
    fail_allocator* a = user; CHECK(a->live > 0); a->live--; free(pointer);
}
static void failures(void)
{
    long fail;
    int saw_success = 0;
    for (fail = 0; fail < 512 && !saw_success; fail++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0};
        xui_document d; xui_document_transaction t = NULL; xui_document_snapshot s;
        uint64_t p, text, revision; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_RICH;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        p = insert(t, 1, XUI_DOC_PARAGRAPH, NULL); text = insert(t, p, XUI_DOC_TEXT, "unchanged");
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t); t = NULL;
        revision = xuiDocumentGetRevision(d); a.remaining = fail;
        result = xuiDocumentBeginTransaction(d, NULL, &t);
        if (result == XUI_OK) result = xuiDocumentTxnReplaceText(t, text, 0, 9, "changed", 7);
        if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
        xuiDocumentTxnRelease(t); a.remaining = -1;
        CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
        if (result != XUI_OK) { CHECK(result == XUI_ERROR_OUT_OF_MEMORY); CHECK(xuiDocumentGetRevision(d) == revision); expect_text(s, text, "unchanged"); }
        else { saw_success = 1; expect_text(s, text, "changed"); }
        xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d); CHECK(a.live == 0);
    }
    CHECK(saw_success); printf("Document allocation-failure sweep: %ld points passed\n", fail);
}
static void roundtrip(xui_document d)
{
    xui_document loaded = NULL; xui_document_snapshot s, l;
    char *json = NULL, *second = NULL, *html = NULL; uint64_t n, m, h;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSerialize(s, &json, &n) == XUI_OK && n > 0);
    CHECK(xuiDocumentDeserialize(NULL, json, n, &loaded) == XUI_OK && loaded);
    CHECK(!xuiDocumentIsDirty(loaded) && !xuiDocumentCanUndo(loaded));
    CHECK(xuiDocumentAcquireSnapshot(loaded, &l) == XUI_OK);
    CHECK(xuiDocumentSerialize(l, &second, &m) == XUI_OK && m == n && memcmp(json, second, (size_t)n) == 0);
    CHECK(xuiDocumentExportHtml(l, &html, &h) == XUI_OK && html && h > 0);
    xuiDocumentFreeBuffer(html); xuiDocumentFreeBuffer(second);
    xuiDocumentSnapshotRelease(l); xuiDocumentRelease(loaded); loaded = NULL;
    CHECK(xuiDocumentDeserialize(NULL, json, n - 1, &loaded) != XUI_OK && !loaded);
    { char* version = strstr(json, "\"schemaVersion\":1"); CHECK(version); version[16] = '2'; }
    CHECK(xuiDocumentDeserialize(NULL, json, n, &loaded) == XUI_DOC_ERROR_FORMAT && !loaded);
    xuiDocumentFreeBuffer(json); xuiDocumentSnapshotRelease(s);
}
static void persistence_and_schema(void)
{
    xui_document d; xui_document_transaction t; xui_doc_node_info_t info = {0};
    uint64_t p, a, table, r1, r2, c1, c2, c3; xui_document_snapshot s;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    p = insert(t, 1, XUI_DOC_QUOTE, NULL); p = insert(t, p, XUI_DOC_PARAGRAPH, NULL);
    a = insert(t, p, XUI_DOC_TEXT, "Unicode \xe4\xb8\xad\xe6\x96\x87 & < > \" '");
    info.iSize = sizeof(info); CHECK(xuiDocumentTxnGetNode(t, a, &info) == XUI_OK);
    info.tAttributes.iMarks = XUI_DOC_BOLD | XUI_DOC_ITALIC | XUI_DOC_LINK;
    CHECK(xuiDocumentTxnSetAttributes(t, a, &info.tAttributes) == XUI_OK);
    CHECK(xuiDocumentTxnSetResource(t, a, "https://example.org/?a=1&b=2", "", "link") == XUI_OK);
    table = insert(t, 1, XUI_DOC_TABLE, NULL); r1 = insert(t, table, XUI_DOC_ROW, NULL); r2 = insert(t, table, XUI_DOC_ROW, NULL);
    c1 = insert(t, r1, XUI_DOC_CELL, NULL); c2 = insert(t, r1, XUI_DOC_CELL, NULL); c3 = insert(t, r2, XUI_DOC_CELL, NULL);
    CHECK(xuiDocumentTxnGetNode(t, c1, &info) == XUI_OK); info.tAttributes.iRowSpan = 2;
    CHECK(xuiDocumentTxnSetAttributes(t, c1, &info.tAttributes) == XUI_OK);
    p = insert(t, c1, XUI_DOC_PARAGRAPH, NULL); insert(t, p, XUI_DOC_TEXT, "span");
    p = insert(t, c2, XUI_DOC_PARAGRAPH, NULL); insert(t, p, XUI_DOC_TEXT, "top");
    p = insert(t, c3, XUI_DOC_PARAGRAPH, NULL); insert(t, p, XUI_DOC_TEXT, "bottom");
    insert(t, 1, XUI_DOC_EXTENSION, "{\"unknown-extension\":true}");
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    roundtrip(d);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnDeleteNode(t, c3) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_DOC_ERROR_SCHEMA); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(s, c3, &info) == XUI_OK);
    xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    { xui_doc_desc_t desc = {0}; const char* md = "\xef\xbb\xbf# Heading\r\n\r\n- **bold** &amp; text\r\n\r\n```c\r\ncode\r\n```\r\n";
      desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
      CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK); CHECK(xuiDocumentLoadMarkdown(d, md, strlen(md)) == XUI_OK);
      roundtrip(d); xuiDocumentRelease(d);
    }
}
static unsigned random_value = 7;
static unsigned next_random(void) { random_value ^= random_value << 13; random_value ^= random_value >> 17; random_value ^= random_value << 5; return random_value; }
static void random_edits(void)
{
    xui_document d; xui_document_transaction t; xui_document_snapshot old = NULL, s;
    char reference[8192] = "begin", saved[8192], actual[8192], replacement[32];
    uint64_t p, text, size = 5, bytes; unsigned i;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    p = insert(t, 1, XUI_DOC_PARAGRAPH, NULL); text = insert(t, p, XUI_DOC_TEXT, reference);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    for (i = 0; i < 3000; i++) {
        uint64_t start = next_random() % (size + 1), end = start + next_random() % (size - start + 1), add = next_random() % 32, j;
        uint64_t revision = xuiDocumentGetRevision(d);
        for (j = 0; j < add; j++) replacement[j] = (char)('a' + next_random() % 26);
        if (i % 97 == 0) {
            xuiDocumentSnapshotRelease(old); CHECK(xuiDocumentAcquireSnapshot(d, &old) == XUI_OK); strcpy(saved, reference);
        }
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceText(t, text, start, end, replacement, add) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        if (start == end && !add) CHECK(xuiDocumentGetRevision(d) == revision);
        memmove(reference + start + add, reference + end, (size_t)(size - end + 1)); memcpy(reference + start, replacement, (size_t)add);
        size = size - (end - start) + add; CHECK(size < sizeof(reference));
        CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
        CHECK(xuiDocumentSnapshotCopyText(s, text, actual, sizeof(actual), &bytes) == XUI_OK && bytes == size && strcmp(actual, reference) == 0);
        CHECK(xuiDocumentSnapshotCopyText(old, text, actual, sizeof(actual), &bytes) == XUI_OK && strcmp(actual, saved) == 0);
        xuiDocumentSnapshotRelease(s);
    }
    xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d);
}
static void markdown_identity(void)
{
    xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot before, after;
    xui_document_transaction t; xui_doc_txn_desc_t td = {0}; xui_doc_node_info_t info = {0};
    uint64_t heading, list, text, revision;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(d, "# A\n\n- one\n- two\n", 17) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
    heading = find_kind(before, 1, XUI_DOC_HEADING); list = find_kind(before, 1, XUI_DOC_LIST);
    CHECK(xuiDocumentSnapshotGetChild(before, heading, 0, &text) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, 2, 3, "Heading", 7) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(after, heading, &info) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(after, list, &info) == XUI_OK);
    expect_text(before, text, "A"); expect_text(after, text, "Heading");
    revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, 2, 9, "Heading", 7) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentGetRevision(d) == revision); xuiDocumentTxnRelease(t);
    xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
}
static void markdown_failures(void)
{
    const char* initial = "# Old\n\n- item\n";
    const char* replacement = "**strong** [link &amp;](https://example.org/?a=1&amp;b=2)\n\n| A | B |\n| - | - |\n| a | b |\n\n- item\n";
    long fail; int success = 0;
    for (fail = 0; fail < 4096 && !success; fail++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot before, after;
        char *old_json, *new_json; uint64_t old_size, new_size, revision; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(d, initial, strlen(initial)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        CHECK(xuiDocumentSerialize(before, &old_json, &old_size) == XUI_OK);
        revision = xuiDocumentGetRevision(d); a.remaining = fail;
        result = xuiDocumentLoadMarkdown(d, replacement, strlen(replacement)); a.remaining = -1;
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        if (result != XUI_OK) {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision);
            CHECK(xuiDocumentSerialize(after, &new_json, &new_size) == XUI_OK);
            CHECK(old_size == new_size && memcmp(old_json, new_json, (size_t)old_size) == 0); xuiDocumentFreeBuffer(new_json);
        } else success = 1;
        xuiDocumentFreeBuffer(old_json); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
        if (a.live) fprintf(stderr, "Markdown fault point %ld leaked %u allocations\n", fail, a.live);
        CHECK(a.live == 0);
    }
    CHECK(success); printf("Markdown/parser allocation-failure sweep: %ld points passed\n", fail);
}
typedef struct reader_test { xui_document_snapshot snapshot; uint64_t text; } reader_test;
static int32 snapshot_reader(ptr user)
{
    reader_test* r = user; unsigned i;
    for (i = 0; i < 10000; i++) {
        xuiDocumentSnapshotRetain(r->snapshot);
        expect_text(r->snapshot, r->text, "immutable");
        xuiDocumentSnapshotRelease(r->snapshot);
    }
    return 0;
}
static void concurrent_snapshots(void)
{
    xui_document d; xui_document_transaction t; reader_test reader; xthread* workers[4]; unsigned i;
    uint64_t p;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    p = insert(t, 1, XUI_DOC_PARAGRAPH, NULL); reader.text = insert(t, p, XUI_DOC_TEXT, "immutable");
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &reader.snapshot) == XUI_OK);
    for (i = 0; i < 4; i++) { workers[i] = xrtThreadCreate(snapshot_reader, &reader, 0); CHECK(workers[i]); }
    for (i = 0; i < 500; i++) {
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceText(t, reader.text, 0, 1, i & 1 ? "I" : "i", 1) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    }
    xuiDocumentRelease(d);
    for (i = 0; i < 4; i++) { (void)xrtThreadWait(workers[i]); CHECK(xrtThreadExitCode(workers[i]) == 0); xrtThreadDestroy(workers[i]); }
    xuiDocumentSnapshotRelease(reader.snapshot);
}
static void mark_range(void)
{
    xui_document d; xui_document_transaction t; xui_document_snapshot s;
    xui_doc_range_t range = {0}; xui_doc_node_info_t info = {0}; uint64_t p, a, b, tail;
    char* plain; uint64_t bytes;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    p = insert(t, 1, XUI_DOC_PARAGRAPH, NULL); a = insert(t, p, XUI_DOC_TEXT, "first"); b = insert(t, p, XUI_DOC_TEXT, "second");
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    range.tAnchor.iSize = range.tCaret.iSize = sizeof(range.tAnchor); range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_TEXT;
    range.tAnchor.iDocumentId = range.tCaret.iDocumentId = xuiDocumentGetIdentity(d);
    range.tAnchor.iRevision = range.tCaret.iRevision = xuiDocumentGetRevision(d);
    range.tAnchor.iNodeId = a; range.tAnchor.iOffset = 2; range.tCaret.iNodeId = b; range.tCaret.iOffset = 3;
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnSetMarks(t, &range, XUI_DOC_BOLD, 0) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetChild(s, p, 1, &tail) == XUI_OK); expect_text(s, tail, "rst");
    CHECK(xuiDocumentSnapshotGetNode(s, tail, &info) == XUI_OK && info.tAttributes.iMarks == XUI_DOC_BOLD);
    expect_text(s, b, "sec");
    CHECK(xuiDocumentSnapshotGetNode(s, b, &info) == XUI_OK && info.tAttributes.iMarks == XUI_DOC_BOLD);
    CHECK(xuiDocumentSnapshotCopyPlainText(s, &plain, &bytes) == XUI_OK && !strcmp(plain, "firstsecond\n")); xuiDocumentFreeBuffer(plain);
    xuiDocumentSnapshotRelease(s); CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); expect_text(s, a, "first"); expect_text(s, b, "second");
    xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
}
static void markdown_semantic_validation(void)
{
    xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot s; xui_document_transaction t;
    xui_doc_range_t range = {0}; uint64_t text, revision; char source[128]; uint64_t n;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK); CHECK(xuiDocumentLoadMarkdown(d, "word", 4) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); text = find_kind(s, 1, XUI_DOC_TEXT); xuiDocumentSnapshotRelease(s);
    range.tAnchor.iSize = range.tCaret.iSize = sizeof(range.tAnchor); range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_TEXT;
    range.tAnchor.iDocumentId = range.tCaret.iDocumentId = xuiDocumentGetIdentity(d);
    range.tAnchor.iRevision = range.tCaret.iRevision = xuiDocumentGetRevision(d);
    range.tAnchor.iNodeId = range.tCaret.iNodeId = text; range.tCaret.iOffset = 4;
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnSetMarks(t, &range, XUI_DOC_BOLD, 0) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(s, source, sizeof(source), &n) == XUI_OK && !strcmp(source, "**word**")); xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentLoadMarkdown(d, "`code`", 6) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); text = find_kind(s, 1, XUI_DOC_TEXT); xuiDocumentSnapshotRelease(s);
    revision = xuiDocumentGetRevision(d); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(t, text, 0, 4, "a`b", 3) == XUI_DOC_ERROR_UNREPRESENTABLE);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_DOC_ERROR_UNREPRESENTABLE); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentGetRevision(d) == revision); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(s, source, sizeof(source), &n) == XUI_OK && !strcmp(source, "`code`"));
    xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
}
static void structural_commands(void)
{
    xui_document d; xui_document_transaction t; xui_document_snapshot s; xui_doc_range_t range = {0}; xui_doc_position_t caret;
    uint64_t p, a, b, last, bytes; char* text;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    p = insert(t, 1, XUI_DOC_PARAGRAPH, NULL); a = insert(t, p, XUI_DOC_TEXT, "first"); insert(t, p, XUI_DOC_TEXT, " middle");
    p = insert(t, 1, XUI_DOC_PARAGRAPH, NULL); b = insert(t, p, XUI_DOC_TEXT, "second"); last = insert(t, p, XUI_DOC_TEXT, " tail");
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d);
    range.tAnchor.iRevision = xuiDocumentGetRevision(d); range.tAnchor.iKind = XUI_DOC_POSITION_TEXT;
    range.tCaret = range.tAnchor; range.tAnchor.iNodeId = a; range.tAnchor.iOffset = 2; range.tCaret.iNodeId = b; range.tCaret.iOffset = 3;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopyRange(s, &range, &text, &bytes) == XUI_OK && !strcmp(text, "rst middle\nsec"));
    xuiDocumentFreeBuffer(text); xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceRange(t, &range, "NEW\r\nLINE", 9, &caret) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopyPlainText(s, &text, &bytes) == XUI_OK && !strcmp(text, "fiNEW\nLINEond tail\n"));
    xuiDocumentFreeBuffer(text); expect_text(s, last, " tail"); xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopyPlainText(s, &text, &bytes) == XUI_OK && !strcmp(text, "first middle\nsecond tail\n"));
    xuiDocumentFreeBuffer(text); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
}
static void table_commands(void)
{
    xui_document d, reloaded; xui_document_transaction t; xui_document_snapshot s;
    xui_doc_node_info_t info = {0}; uint64_t table, merged, bytes, row, cell, paragraph, text, i; char* json;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(t, 1, 0, 3, 3, 1, &table) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(s, table, 1, &row) == XUI_OK); CHECK(xuiDocumentSnapshotGetChild(s, row, 1, &cell) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(s, cell, 0, &paragraph) == XUI_OK); CHECK(xuiDocumentSnapshotGetChild(s, paragraph, 0, &text) == XUI_OK); xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); CHECK(xuiDocumentTxnReplaceText(t, text, 0, 0, "center", 6) == XUI_OK);
    CHECK(xuiDocumentTxnMergeCells(t, table, 0, 0, 2, 2, &merged) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    info.iSize = sizeof(info); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(s, merged, &info) == XUI_OK && info.tAttributes.iRowSpan == 2 && info.tAttributes.iColumnSpan == 2);
    expect_text(s, text, "center"); xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTableRow(t, table, 1) == XUI_OK); CHECK(xuiDocumentTxnInsertTableColumn(t, table, 1) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(s, merged, &info) == XUI_OK && info.tAttributes.iRowSpan == 3 && info.tAttributes.iColumnSpan == 3); xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnDeleteTableRow(t, table, 0) == XUI_OK); CHECK(xuiDocumentTxnDeleteTableColumn(t, table, 0) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); expect_text(s, text, "center"); xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); CHECK(xuiDocumentTxnSplitCell(t, merged) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    for (i = 0; i < 3; i++) { CHECK(xuiDocumentSnapshotGetChild(s, table, i, &row) == XUI_OK); CHECK(xuiDocumentSnapshotGetNode(s, row, &info) == XUI_OK && info.iChildCount == 3); }
    CHECK(xuiDocumentSerialize(s, &json, &bytes) == XUI_OK); CHECK(xuiDocumentDeserialize(NULL, json, bytes, &reloaded) == XUI_OK);
    xuiDocumentFreeBuffer(json); xuiDocumentRelease(reloaded); xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); CHECK(xuiDocumentSnapshotGetNode(s, cell, &info) == XUI_OK);
    expect_text(s, text, ""); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
}
static void markdown_structural_input(void)
{
    xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t; xui_document_snapshot s;
    xui_doc_range_t range = {0}; xui_doc_position_t caret; uint64_t id, bytes; char source[512]; char* plain;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(d, "abcd\n\n```c\nkeep(  );\n```\n", sizeof("abcd\n\n```c\nkeep(  );\n```\n") - 1) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); id = find_kind(s, 1, XUI_DOC_TEXT); xuiDocumentSnapshotRelease(s);
    range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iKind = XUI_DOC_POSITION_TEXT; range.tAnchor.iNodeId = id;
    range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d); range.tAnchor.iOffset = 2;
    range.tCaret = range.tAnchor;
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceRange(t, &range, "\n", 1, &caret) == XUI_OK); CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(s, source, sizeof(source), &bytes) == XUI_OK && strstr(source, "```c\nkeep(  );\n```\n"));
    CHECK(xuiDocumentSnapshotCopyPlainText(s, &plain, &bytes) == XUI_OK && !strncmp(plain, "ab\ncd\n", 6)); xuiDocumentFreeBuffer(plain);
    xuiDocumentSnapshotRelease(s); caret.iRevision = xuiDocumentGetRevision(d); range.tAnchor = range.tCaret = caret;
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); CHECK(xuiDocumentTxnReplaceRange(t, &range, "X", 1, &caret) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); CHECK(xuiDocumentSnapshotCopyPlainText(s, &plain, &bytes) == XUI_OK && !strncmp(plain, "ab\nXcd\n", 7));
    xuiDocumentFreeBuffer(plain); xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentLoadMarkdown(d, "", 0) == XUI_OK);
    range.tAnchor.iRevision = xuiDocumentGetRevision(d); range.tAnchor.iNodeId = 1; range.tAnchor.iOffset = 0; range.tAnchor.iKind = XUI_DOC_POSITION_GAP; range.tCaret = range.tAnchor;
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); CHECK(xuiDocumentTxnReplaceRange(t, &range, "hello", 5, &caret) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); CHECK(xuiDocumentSnapshotCopyPlainText(s, &plain, &bytes) == XUI_OK && !strcmp(plain, "hello\n"));
    xuiDocumentFreeBuffer(plain); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
}
static void structural_failures(void)
{
    int mode;
    for (mode = 0; mode < 2; mode++) {
        long fail; int passed = 0;
        for (fail = 0; fail < 4096 && !passed; fail++) {
            fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t = NULL;
            xui_document_snapshot s; xui_doc_range_t range = {0}; xui_doc_position_t caret; uint64_t id, revision, bytes; char* before; char* after; int result;
            desc.iSize = sizeof(desc); desc.iProfile = mode ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
            desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
            if (mode) CHECK(xuiDocumentLoadMarkdown(d, "abcd\n", 5) == XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); id = mode ? find_kind(s, 1, XUI_DOC_TEXT) : 1;
            CHECK(xuiDocumentSerialize(s, &before, &bytes) == XUI_OK); xuiDocumentSnapshotRelease(s);
            range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d);
            range.tAnchor.iKind = mode ? XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP; range.tAnchor.iNodeId = id; range.tAnchor.iOffset = mode ? 2 : 0; range.tCaret = range.tAnchor;
            revision = xuiDocumentGetRevision(d); a.remaining = fail;
            result = xuiDocumentBeginTransaction(d, NULL, &t);
            if (result == XUI_OK) result = mode ? xuiDocumentTxnReplaceRange(t, &range, "\n", 1, &caret) : xuiDocumentTxnInsertTable(t, 1, 0, 2, 2, 1, &id);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            xuiDocumentTxnRelease(t); a.remaining = -1;
            if (result == XUI_OK) passed = 1;
            else {
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY); CHECK(xuiDocumentGetRevision(d) == revision);
                CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); CHECK(xuiDocumentSerialize(s, &after, &bytes) == XUI_OK && !strcmp(after, before));
                xuiDocumentFreeBuffer(after); xuiDocumentSnapshotRelease(s);
            }
            xuiDocumentFreeBuffer(before); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(passed); printf("%s structural allocation-failure sweep: %ld points passed\n", mode ? "Markdown" : "Table", fail);
    }
}
static void dialects(void)
{
    const char* source = "~~strike~~ $math$\n\n| A | B |\n| - | - |\n| C | D |\n"; unsigned dialect;
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) {
        xui_doc_desc_t desc = {0}; xui_document d, loaded; xui_document_snapshot s; xui_document_transaction t;
        xui_doc_txn_desc_t td = {0}; char* json; uint64_t bytes;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK); CHECK(xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE; CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceSource(t, 0, 0, "prefix ", 7) == XUI_OK); CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentGetMarkdownDialect(d) == (int)dialect); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
        CHECK((find_kind(s, 1, XUI_DOC_TABLE) != 0) == (dialect != XUI_MD_COMMONMARK));
        CHECK((find_kind(s, 1, XUI_DOC_MATH) != 0) == (dialect == XUI_MD_EXTENDED));
        CHECK(xuiDocumentSerialize(s, &json, &bytes) == XUI_OK); CHECK(xuiDocumentDeserialize(NULL, json, bytes, &loaded) == XUI_OK);
        CHECK(xuiDocumentGetMarkdownDialect(loaded) == (int)dialect); xuiDocumentFreeBuffer(json); xuiDocumentRelease(loaded); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    }
}
static void markdown_combined_marks(void)
{
    xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot s; xui_document_transaction t;
    xui_doc_range_t range = {0}; uint64_t p, a, b, size; char source[256]; xui_doc_node_info_t info = {0};
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; info.iSize = sizeof(info);
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK); CHECK(xuiDocumentLoadMarkdown(d, "a*b*c\n\n~~~c\nx;\n~~~\n", sizeof("a*b*c\n\n~~~c\nx;\n~~~\n") - 1) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); CHECK(xuiDocumentSnapshotGetChild(s, 1, 0, &p) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(s, p, 0, &a) == XUI_OK); CHECK(xuiDocumentSnapshotGetChild(s, p, 2, &b) == XUI_OK); xuiDocumentSnapshotRelease(s);
    range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iKind = XUI_DOC_POSITION_TEXT;
    range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d);
    range.tAnchor.iNodeId = a; range.tCaret = range.tAnchor; range.tCaret.iNodeId = b; range.tCaret.iOffset = 1;
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); CHECK(xuiDocumentTxnSetMarks(t, &range, XUI_DOC_BOLD, 0) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); CHECK(xuiDocumentSnapshotGetChild(s, 1, 0, &p) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(s, p, 1, &a) == XUI_OK); CHECK(xuiDocumentSnapshotGetNode(s, a, &info) == XUI_OK);
    CHECK(info.tAttributes.iMarks == (XUI_DOC_BOLD | XUI_DOC_ITALIC));
    CHECK(xuiDocumentSnapshotCopySource(s, source, sizeof(source), &size) == XUI_OK && strstr(source, "~~~c\nx;\n~~~\n")); xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(d, "**abcd**", 8) == XUI_OK); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    a = find_kind(s, 1, XUI_DOC_TEXT); xuiDocumentSnapshotRelease(s);
    range.tAnchor.iRevision = xuiDocumentGetRevision(d); range.tAnchor.iNodeId = a; range.tAnchor.iOffset = 1;
    range.tCaret = range.tAnchor; range.tCaret.iOffset = 3;
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); CHECK(xuiDocumentTxnSetMarks(t, &range, 0, XUI_DOC_BOLD) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); CHECK(xuiDocumentSnapshotGetChild(s, 1, 0, &p) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(s, p, 1, &a) == XUI_OK); CHECK(xuiDocumentSnapshotGetNode(s, a, &info) == XUI_OK && !info.tAttributes.iMarks);
    expect_text(s, a, "bc"); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
}
static void search_and_replace(void)
{
    xui_document d; xui_document_snapshot s; xui_document_transaction t; xui_doc_range_t matches[4], whole = {0};
    uint64_t p, a, b, total, bytes; uint32_t common, mixed; char* text;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    p = insert(t, 1, XUI_DOC_PARAGRAPH, NULL); a = insert(t, p, XUI_DOC_TEXT, "hel"); b = insert(t, p, XUI_DOC_TEXT, "lo world");
    p = insert(t, 1, XUI_DOC_PARAGRAPH, NULL); insert(t, p, XUI_DOC_TEXT, "hello world");
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(s, XUI_DOC_SEMANTIC, "hello", 5, NULL, matches, 4, &total) == XUI_OK && total == 2);
    CHECK(matches[0].tAnchor.iNodeId == a && matches[0].tCaret.iNodeId == b && matches[0].tCaret.iOffset == 2);
    CHECK(xuiDocumentSnapshotFind(s, XUI_DOC_SEMANTIC, "world\nhello", 11, NULL, NULL, 0, &total) == XUI_OK && total == 1);
    whole.tAnchor = whole.tCaret = matches[0].tAnchor; whole.tAnchor.iKind = whole.tCaret.iKind = XUI_DOC_POSITION_GAP;
    whole.tAnchor.iNodeId = whole.tCaret.iNodeId = 1; whole.tAnchor.iOffset = 0; whole.tCaret.iOffset = 2;
    xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); CHECK(xuiDocumentTxnSetMarks(t, &whole, XUI_DOC_BOLD, 0) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    whole.tAnchor.iRevision = whole.tCaret.iRevision = xuiDocumentGetRevision(d); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotQueryMarks(s, &whole, &common, &mixed) == XUI_OK && common == XUI_DOC_BOLD && !mixed);
    xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); CHECK(xuiDocumentTxnReplaceAll(t, "hello", 5, "hi", 2, NULL, &total) == XUI_OK && total == 2);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); CHECK(xuiDocumentSnapshotCopyPlainText(s, &text, &bytes) == XUI_OK && !strcmp(text, "hi world\nhi world\n"));
    xuiDocumentFreeBuffer(text); xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    expect_text(s, a, "hel"); expect_text(s, b, "lo world"); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    { xui_doc_desc_t desc = {0}; char source[128];
      desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
      CHECK(xuiDocumentLoadMarkdown(d, "**hello** hello\n", 16) == XUI_OK); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
      CHECK(xuiDocumentTxnReplaceAll(t, "hello", 5, "bye", 3, NULL, &total) == XUI_OK && total == 2);
      CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
      CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); CHECK(xuiDocumentSnapshotCopySource(s, source, sizeof(source), &bytes) == XUI_OK && strstr(source, "**bye** bye"));
      xuiDocumentSnapshotRelease(s); CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
      CHECK(xuiDocumentSnapshotCopySource(s, source, sizeof(source), &bytes) == XUI_OK && !strcmp(source, "**hello** hello\n")); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d); }
}
static void file_roundtrip(void)
{
    const char* md_path = "build/document/roundtrip.md"; const char* native_path = "build/document/roundtrip.xdoc";
    const char* source = "\xef\xbb\xbf# Title\r\n\r\nA &amp; B\\*\n";
    xui_doc_desc_t desc = {0}; xui_document d, loaded; xui_document_snapshot snapshot; xui_document_transaction t;
    xui_doc_txn_desc_t td = {0}; char output[128]; uint64_t bytes; unsigned char* file; size_t file_bytes;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK); CHECK(xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
    CHECK(xuiDocumentSaveFile(d, md_path, XUI_DOC_FILE_MARKDOWN) == XUI_OK && !xuiDocumentIsDirty(d));
    file = xrtFileReadAll(md_path, &file_bytes); CHECK(file && file_bytes == strlen(source) && !memcmp(file, source, file_bytes)); xrtFree(file);
    CHECK(xuiDocumentOpenFile(md_path, XUI_DOC_FILE_MARKDOWN, NULL, 1024, &loaded) == XUI_OK);
    CHECK(!xuiDocumentIsDirty(loaded) && !xuiDocumentCanUndo(loaded)); xuiDocumentRelease(loaded);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE; CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(t, 3, 3, "new ", 4) == XUI_OK); CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentSnapshotExportFile(snapshot, md_path, XUI_DOC_FILE_MARKDOWN) == XUI_OK);
    CHECK(xuiDocumentMarkSaved(d, snapshot) == XUI_OK && xuiDocumentIsDirty(d)); xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentSaveFile(d, "build/document", XUI_DOC_FILE_MARKDOWN) == XUI_DOC_ERROR_IO && xuiDocumentIsDirty(d));
    CHECK(xuiDocumentSaveFile(d, native_path, XUI_DOC_FILE_NATIVE) == XUI_OK && !xuiDocumentIsDirty(d));
    CHECK(xuiDocumentOpenFile(native_path, XUI_DOC_FILE_NATIVE, NULL, 4096, &loaded) == XUI_OK);
    CHECK(!xuiDocumentIsDirty(loaded)); CHECK(xuiDocumentAcquireSnapshot(loaded, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(snapshot, output, sizeof(output), &bytes) == XUI_OK && !strcmp(output, "\xef\xbb\xbfnew # Title\r\n\r\nA &amp; B\\*\n"));
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(loaded); xuiDocumentRelease(d);
    CHECK(xrtFileDelete(md_path)); CHECK(xrtFileDelete(native_path));
}
static void replace_all_failures(void)
{
    uint32_t domain;
    for (domain = XUI_DOC_SEMANTIC; domain <= XUI_DOC_SOURCE; domain++) {
    long fail; int passed = 0;
    for (fail = 0; fail < 8192 && !passed; fail++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t = NULL;
        xui_document_snapshot s; xui_doc_txn_desc_t td = {0}; uint64_t revision, bytes, replaced; char* before; char* after; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK); CHECK(xuiDocumentLoadMarkdown(d, "**hello** hello\n", 16) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); CHECK(xuiDocumentSerialize(s, &before, &bytes) == XUI_OK); xuiDocumentSnapshotRelease(s);
        revision = xuiDocumentGetRevision(d); a.remaining = fail;
        td.iSize = sizeof(td); td.iDomain = domain;
        result = xuiDocumentBeginTransaction(d, &td, &t);
        if (result == XUI_OK) result = xuiDocumentTxnReplaceAll(t, "hello", 5, "bye", 3, NULL, &replaced);
        if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
        xuiDocumentTxnRelease(t); a.remaining = -1;
        if (result == XUI_OK) passed = 1;
        else {
            CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision);
            CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); CHECK(xuiDocumentSerialize(s, &after, &bytes) == XUI_OK && !strcmp(before, after));
            xuiDocumentFreeBuffer(after); xuiDocumentSnapshotRelease(s);
        }
        xuiDocumentFreeBuffer(before); xuiDocumentRelease(d); CHECK(!a.live);
    }
    CHECK(passed); printf("%s replace-all allocation-failure sweep: %ld points passed\n", domain == XUI_DOC_SOURCE ? "Source" : "Semantic", fail);
    }
}
static void batch_source_replace(void)
{
    xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t; xui_doc_txn_desc_t td = {0};
    xui_doc_stats_t before = {0}, after = {0}; uint64_t count, total, tail; xui_document_snapshot s;
    xui_doc_range_t found[1]; const char* source = "one one one\n\none two\n\nkeep\n";
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(s, XUI_DOC_SEMANTIC, "keep", 4, NULL, found, 1, &count) == XUI_OK && count == 1);
    tail = found[0].tAnchor.iNodeId; xuiDocumentSnapshotRelease(s);
    before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
    td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE; CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceAll(t, "one", 3, "three", 5, NULL, &total) == XUI_OK && total == 4);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentGetStats(d, &after) == XUI_OK && after.iMarkdownParses == before.iMarkdownParses + 1);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); expect_text(s, tail, "keep");
    CHECK(xuiDocumentSnapshotFind(s, XUI_DOC_SOURCE, "three", 5, NULL, NULL, 0, &count) == XUI_OK && count == 4);
    xuiDocumentSnapshotRelease(s); CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    { char text[128]; CHECK(xuiDocumentSnapshotCopySource(s, text, sizeof(text), &count) == XUI_OK && !strcmp(text, source)); }
    xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
}
static void deletion_position_map(void)
{
    xui_document d; xui_document_transaction t; xui_document_change_set changes;
    xui_doc_position_t position = {0}, mapped; uint64_t p[5], text[5], i; int mapping;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    for (i = 0; i < 5; i++) { p[i] = insert(t, 1, XUI_DOC_PARAGRAPH, NULL); text[i] = insert(t, p[i], XUI_DOC_TEXT, "text"); }
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    position.iSize = sizeof(position); position.iDocumentId = xuiDocumentGetIdentity(d); position.iRevision = xuiDocumentGetRevision(d);
    position.iKind = XUI_DOC_POSITION_TEXT; position.iNodeId = text[2]; position.iOffset = 2; position.iAffinity = XUI_DOC_AFTER;
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnDeleteNode(t, p[0]) == XUI_OK); CHECK(xuiDocumentTxnDeleteNode(t, p[2]) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, &changes) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentMapPosition(changes, &position, &mapped, &mapping) == XUI_OK && mapping == XUI_DOC_MAP_DELETED);
    CHECK(mapped.iKind == XUI_DOC_POSITION_GAP && mapped.iNodeId == 1 && mapped.iOffset == 1);
    xuiDocumentChangeSetRelease(changes); CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
    position.iRevision = xuiDocumentGetRevision(d); position.iNodeId = text[4];
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnMoveNode(t, text[4], p[1], XUI_DOCUMENT_APPEND) == XUI_OK);
    CHECK(xuiDocumentTxnDeleteNode(t, p[1]) == XUI_OK); CHECK(xuiDocumentTxnDeleteNode(t, p[0]) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, &changes) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentMapPosition(changes, &position, &mapped, &mapping) == XUI_OK && mapping == XUI_DOC_MAP_DELETED);
    CHECK(mapped.iKind == XUI_DOC_POSITION_GAP && mapped.iNodeId == 1 && mapped.iOffset == 0);
    xuiDocumentChangeSetRelease(changes); xuiDocumentRelease(d);
}
int main(void)
{
    rich(); markdown(); failures(); persistence_and_schema(); random_edits(); markdown_identity();
    markdown_failures(); concurrent_snapshots(); mark_range();
    markdown_semantic_validation();
    structural_commands();
    table_commands();
    markdown_structural_input();
    structural_failures(); dialects(); markdown_combined_marks(); search_and_replace();
    file_roundtrip(); replace_all_failures();
    deletion_position_map();
    batch_source_replace();
    puts("Unified Document core tests passed"); return 0;
}
