#include "../xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#else
#include <pthread.h>
#endif
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "source index line %d: %s\n", __LINE__, #c); exit(1); } } while (0)
typedef struct fail_allocator { long remaining; unsigned live; } fail_allocator;
static void* failing_alloc(void* user, size_t bytes)
{
    fail_allocator* a = user; void* p;
    if (!a->remaining) return NULL;
    if (a->remaining > 0) a->remaining--;
    p = malloc(bytes); if (p) a->live++; return p;
}
static void failing_free(void* user, void* pointer)
{ fail_allocator* a = user; CHECK(a->live); a->live--; free(pointer); }
static const char* fixtures[] = {
    "", " \t\r\n\n", "plain words and **bold** &amp; \\*\n",
    "\xef\xbb\xbf---\r\ntitle: value\r\n...\r\n\r\n# end #\r\n",
    "> - [x] **item**\n>   continuation\n>\n> - next\n",
    "- outer\n  - nested *item*\n\n    next paragraph\n",
    "a | b\n:-- | --:\n\\| | `a|b`\n",
    "[**label**]( <u> \t 'title' )\n",
    "![**alt ![hidden](inner)**](outer \"asset\")\n",
    "[![a]( image \"asset\" )]( link (title) )\n",
    "[label][u]\n\n[u]: <target> 'title'\n\n[unused]: other\n",
    "use [^f]\n\n[^f]: first **bold**\n    next\n\n[^unused]: opaque *body*\n",
    "text <!-- inline --> after\n\n<!-- block\ncomment -->\n",
    "<div>\n<b>HTML</b>\n</div>\n",
    "> ```c title\n> code\n> ```\n\n    indented\n\n    code\n",
    "```mermaid\nflowchart LR\nA --> B\n```\n\n$x$ and $$y$$\n",
    "> [!NOTE]\n> a &amp; `b  c`\n",
    "a  \nb\\\nc\r\nd\re\n",
    "[unclosed *literal &amp; <tag\n\ntext",
    "[x](<> '') ![](u) <user@example.com>\n",
    "***nested*** __bold__ ~~strike~~\n",
    "标题 **文字** &amp; `代码`\n",
    "a\tb\n\n---\n\nsetext\n======\n",
    "[^x]: > quote\n    > next\n\nreference [^x]\n",
    "[u]: /url \"first\n second\"\n\nuse [u]\n",
    "- z `aa\n\tbb`\n", "> z `aa\n>\tbb`\n", "1. z `aa\n\tbb`\n",
    "> - z `aa\n>\tbb`\n", "- z <i title=\"a\n\tb\">\n",
    "> z <i title=\"a\n>\tb\">\n", "\tcode\n\n\tmore\n"
};
static xui_document open_source(const char* bytes, uint64_t size, unsigned dialect, fail_allocator* a)
{
    xui_doc_desc_t desc = {0}; xui_document d;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
    if (a) { desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = a; }
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, bytes, size) == XUI_OK);
    return d;
}
typedef struct query_check_t {
    xui_document_source_index index;
    uint64_t start, end, next, count, limit;
} query_check_t;
static int binding_overlaps(const xui_doc_source_binding_t* b, uint64_t start, uint64_t end)
{
    uint64_t a = b->tRange.iSourceStart, z = b->tRange.iSourceEnd;
    if (a == z) return start == end ? a == start : start <= a && a < end;
    return start == end ? a <= start && start < z : a < end && start < z;
}
static int query_callback(void* user, uint64_t ordinal, const xui_doc_source_binding_t* b)
{
    query_check_t* q = user; xui_doc_source_binding_t expected = {0}; xui_doc_source_index_info_t info = {0};
    info.iSize = sizeof(info); CHECK(xuiDocumentSourceIndexGetInfo(q->index, &info) == XUI_OK);
    while (q->next < info.iBindingCount) {
        expected.iSize = sizeof(expected); CHECK(xuiDocumentSourceIndexGetBinding(q->index, q->next, &expected) == XUI_OK);
        if (binding_overlaps(&expected, q->start, q->end)) break;
        q->next++;
    }
    CHECK(ordinal == q->next && !memcmp(&expected, b, sizeof(expected)));
    q->next++; q->count++;
    return q->limit && q->count == q->limit;
}
static void query_check(xui_document_source_index index, uint64_t start, uint64_t end, uint64_t stop)
{
    query_check_t q = {index, start, end, 0, 0, stop}; uint64_t visited = UINT64_MAX;
    xui_doc_source_index_info_t info = {0}; int code;
    info.iSize = sizeof(info); CHECK(xuiDocumentSourceIndexGetInfo(index, &info) == XUI_OK);
    code = xuiDocumentSourceIndexVisitBindings(index, start, end, query_callback, &q, &visited);
    CHECK(visited == q.count);
    if (stop && q.count == stop) CHECK(code == XUI_DOC_ERROR_CANCELLED);
    else {
        CHECK(code == XUI_OK);
        while (q.next < info.iBindingCount) {
            xui_doc_source_binding_t b = {0}; b.iSize = sizeof(b);
            CHECK(xuiDocumentSourceIndexGetBinding(index, q.next++, &b) == XUI_OK && !binding_overlaps(&b, start, end));
        }
    }
}
static void index_check(xui_document_source_index index, const char* source, uint64_t size, int all_bytes)
{
    xui_doc_source_index_info_t info = {0}; uint64_t i, end = 0, unknown = 0, opaque = 0, found;
    char bytes[4096];
    info.iSize = sizeof(info); CHECK(xuiDocumentSourceIndexGetInfo(index, &info) == XUI_OK);
    CHECK(info.iSourceBytes == size && info.iRecordCount && info.iAllocatedBytes <= info.iPeakWorkingBytes);
    for (i = 0; i < info.iRecordCount; i++) {
        xui_doc_source_record_t r = {0}, parent = {0}; r.iSize = parent.iSize = sizeof(r);
        CHECK(xuiDocumentSourceIndexGetRecord(index, i, &r) == XUI_OK);
        if (!i) CHECK(r.iKind == XUI_DOC_SOURCE_RECORD_ROOT && r.iParentIndex == UINT64_MAX && !r.iSourceStart && r.iSourceEnd == size);
        else {
            CHECK(r.iParentIndex < i && xuiDocumentSourceIndexGetRecord(index, r.iParentIndex, &parent) == XUI_OK);
            if (!(r.iFlags & XUI_DOC_SOURCE_RECORD_UNMAPPED)) CHECK(parent.iSourceStart <= r.iSourceStart &&
                r.iSourceStart <= r.iSourceEnd && r.iSourceEnd <= parent.iSourceEnd);
        }
    }
    for (i = 0; i < info.iTokenCount; i++) {
        xui_doc_source_token_t t = {0}; xui_doc_source_record_t r = {0}; uint64_t at;
        t.iSize = sizeof(t); r.iSize = sizeof(r);
        CHECK(xuiDocumentSourceIndexGetToken(index, i, &t) == XUI_OK && t.iSourceStart == end &&
            t.iSourceStart < t.iSourceEnd && t.iSourceEnd <= size && t.iKind >= XUI_DOC_SOURCE_TOKEN_CONTENT &&
            t.iKind <= XUI_DOC_SOURCE_TOKEN_BREAK_MARKER);
        CHECK(xuiDocumentSourceIndexGetRecord(index, t.iRecordIndex, &r) == XUI_OK &&
            r.iSourceStart <= t.iSourceStart && t.iSourceEnd <= r.iSourceEnd);
        for (at = t.iSourceStart; at < t.iSourceEnd;) {
            uint64_t n = t.iSourceEnd - at < sizeof(bytes) ? t.iSourceEnd - at : sizeof(bytes);
            CHECK(xuiDocumentSourceIndexReadSource(index, at, bytes, n) == XUI_OK && !memcmp(bytes, source + at, (size_t)n)); at += n;
        }
        if (t.iKind == XUI_DOC_SOURCE_TOKEN_UNCLASSIFIED) unknown += t.iSourceEnd - t.iSourceStart;
        if (t.iFlags & XUI_DOC_SOURCE_TOKEN_OPAQUE_PAYLOAD) opaque += t.iSourceEnd - t.iSourceStart;
        CHECK(xuiDocumentSourceIndexFindToken(index, t.iSourceStart, &found) == XUI_OK && found == i);
        CHECK(xuiDocumentSourceIndexFindToken(index, t.iSourceEnd - 1, &found) == XUI_OK && found == i);
        if (all_bytes) for (at = t.iSourceStart; at < t.iSourceEnd; at++)
            CHECK(xuiDocumentSourceIndexFindToken(index, at, &found) == XUI_OK && found == i);
        end = t.iSourceEnd;
    }
    CHECK(end == size && unknown == info.iUnclassifiedBytes && opaque == info.iOpaqueBytes);
    CHECK(xuiDocumentSourceIndexFindToken(index, size, &found) == XUI_ERROR_NOT_FOUND && found == UINT64_MAX);
    CHECK(xuiDocumentSourceIndexFindToken(index, size + 1, &found) == XUI_ERROR_INVALID_ARGUMENT && found == UINT64_MAX);
    for (i = 0; i < info.iBindingCount; i++) {
        xui_doc_source_binding_t b = {0}; b.iSize = sizeof(b);
        CHECK(xuiDocumentSourceIndexGetBinding(index, i, &b) == XUI_OK && b.tRange.iSourceStart <= b.tRange.iSourceEnd &&
            b.tRange.iSourceEnd <= size && b.tRange.iRecordIndex < info.iRecordCount &&
            b.tRange.iKind >= XUI_DOC_SOURCE_TOKEN_CONTENT && b.tRange.iKind <= XUI_DOC_SOURCE_TOKEN_SCOPE);
        if (i) {
            xui_doc_source_binding_t previous = {0}; previous.iSize = sizeof(previous);
            CHECK(xuiDocumentSourceIndexGetBinding(index, i - 1, &previous) == XUI_OK &&
                previous.tRange.iSourceStart <= b.tRange.iSourceStart);
        }
    }
    CHECK(info.iBindingCount == info.iAnnotationCount);
    query_check(index, 0, size, 0); query_check(index, 0, 0, 0); query_check(index, size, size, 0);
    query_check(index, 0, size, 1); query_check(index, 0, size, 3);
    if (all_bytes) for (i = 0; i < size; i++) {
        query_check(index, i, i, 0); query_check(index, i, i + 1, 0);
        if (i + 3 <= size) query_check(index, i, i + 3, 0);
    }
    found = UINT64_MAX;
    CHECK(xuiDocumentSourceIndexVisitBindings(index, size + 1, size + 1, query_callback, NULL, &found) == XUI_ERROR_INVALID_ARGUMENT && !found);
    CHECK(xuiDocumentSourceIndexVisitBindings(index, 1, 0, query_callback, NULL, NULL) == XUI_ERROR_INVALID_ARGUMENT);
    CHECK(xuiDocumentSourceIndexVisitBindings(index, 0, size, NULL, NULL, NULL) == XUI_ERROR_INVALID_ARGUMENT);
}
static xui_document_source_index make_index(xui_document_snapshot snapshot)
{
    xui_document_source_index index = NULL; int result = xuiDocumentSnapshotCreateSourceIndex(snapshot, NULL, &index);
    if (result) fprintf(stderr, "source index construction error: %d\n", result);
    CHECK(result == XUI_OK && index); return index;
}
static void compare_indices(xui_document_source_index a, xui_document_source_index b)
{
    xui_doc_source_index_info_t ai = {0}, bi = {0}; uint64_t i;
    ai.iSize = bi.iSize = sizeof(ai); CHECK(xuiDocumentSourceIndexGetInfo(a, &ai) == XUI_OK && xuiDocumentSourceIndexGetInfo(b, &bi) == XUI_OK);
    CHECK(ai.iSourceBytes == bi.iSourceBytes && ai.iRecordCount == bi.iRecordCount && ai.iTokenCount == bi.iTokenCount &&
        ai.iUnclassifiedBytes == bi.iUnclassifiedBytes && ai.iOpaqueBytes == bi.iOpaqueBytes && ai.iBindingCount == bi.iBindingCount);
    for (i = 0; i < ai.iRecordCount; i++) {
        xui_doc_source_record_t ar = {0}, br = {0}; ar.iSize = br.iSize = sizeof(ar);
        CHECK(xuiDocumentSourceIndexGetRecord(a, i, &ar) == XUI_OK && xuiDocumentSourceIndexGetRecord(b, i, &br) == XUI_OK);
        ar.iNodeId = br.iNodeId = 0; CHECK(!memcmp(&ar, &br, sizeof(ar)));
    }
    for (i = 0; i < ai.iTokenCount; i++) {
        xui_doc_source_token_t at = {0}, bt = {0}; at.iSize = bt.iSize = sizeof(at);
        CHECK(xuiDocumentSourceIndexGetToken(a, i, &at) == XUI_OK && xuiDocumentSourceIndexGetToken(b, i, &bt) == XUI_OK);
        at.iNodeId = bt.iNodeId = 0; CHECK(!memcmp(&at, &bt, sizeof(at)));
    }
    for (i = 0; i < ai.iBindingCount; i++) {
        xui_doc_source_binding_t at = {0}, bt = {0}; at.iSize = bt.iSize = sizeof(at);
        CHECK(xuiDocumentSourceIndexGetBinding(a, i, &at) == XUI_OK && xuiDocumentSourceIndexGetBinding(b, i, &bt) == XUI_OK);
        at.tRange.iNodeId = bt.tRange.iNodeId = 0; CHECK(!memcmp(&at, &bt, sizeof(at)));
    }
}
typedef struct provenance_oracle_t {
    uint64_t record, ordinal, matches;
    xui_doc_source_segment_t segment;
} provenance_oracle_t;
static int provenance_callback(void* user, uint64_t ordinal, const xui_doc_source_binding_t* b)
{
    provenance_oracle_t* p = user; const xui_doc_source_segment_t* s = &p->segment; (void)ordinal;
    if (b->iOriginKind != XUI_DOC_SOURCE_BINDING_PROVENANCE || b->tRange.iRecordIndex != p->record || b->iOriginIndex != p->ordinal)
        return 0;
    CHECK(b->tRange.iSourceStart == s->iSourceStart && b->tRange.iSourceEnd == s->iSourceEnd &&
        b->tRange.iOriginalSourceStart == s->iSourceStart && b->tRange.iOriginalSourceEnd == s->iSourceEnd &&
        b->tRange.iTextStart == s->iTextStart && b->tRange.iTextEnd == s->iTextEnd &&
        b->tRange.iSegmentKind == s->iKind && b->tRange.iSegmentFlags == s->iFlags);
    p->matches++; return 0;
}
typedef struct marker_oracle_t {
    uint64_t record, ordinal, start, end, first, content, last, matches;
    unsigned origin, metadata, flags, row, position;
} marker_oracle_t;
static int marker_callback(void* user, uint64_t ordinal, const xui_doc_source_binding_t* b)
{
    marker_oracle_t* p = user; (void)ordinal;
    if (b->tRange.iRecordIndex != p->record || b->iOriginKind != p->origin || b->iOriginIndex != p->ordinal) return 0;
    CHECK(b->tRange.iSourceStart == p->start && b->tRange.iSourceEnd == p->end && b->iMetadata == p->metadata);
    if (p->metadata == XUI_DOC_SOURCE_BINDING_COLUMNS)
        CHECK(b->iStartColumn == p->first && b->iContentColumn == p->content && b->iEndColumn == p->last &&
            b->iMarkerColumn == UINT64_MAX && b->iMarkerFlags == p->flags);
    if (p->metadata == XUI_DOC_SOURCE_BINDING_TABLE)
        CHECK(b->iRow == p->row && b->iOrdinal == p->position && b->iMarkerFlags == p->flags);
    p->matches++; return 0;
}
static void marker_oracle(xui_document_snapshot snapshot, xui_document_source_index index, uint64_t record, uint64_t node)
{
    xui_doc_block_syntax_t block = {0}; uint64_t j; int code;
    block.iSize = sizeof(block); code = xuiDocumentSnapshotGetBlockSyntax(snapshot, node, &block);
    if (code == XUI_ERROR_NOT_FOUND) return;
    CHECK(code == XUI_OK);
    for (j = 0; j < block.iQuotePrefixCount + block.iListIndentCount + block.iCodeIndentCount + block.iTableTokenCount; j++) {
        marker_oracle_t p = {0}; uint64_t k = j; p.record = record;
        if (k < block.iQuotePrefixCount) {
            p.origin = XUI_DOC_SOURCE_BINDING_QUOTE_PREFIX; p.ordinal = k;
            CHECK(xuiDocumentSnapshotGetQuotePrefix(snapshot, node, k, &p.start, &p.end) == XUI_OK);
        } else if ((k -= block.iQuotePrefixCount) < block.iListIndentCount) {
            xui_doc_list_indent_t s = {0}; s.iSize = sizeof(s);
            CHECK(xuiDocumentSnapshotGetListContinuationIndent(snapshot, node, k, &s) == XUI_OK);
            p.origin = XUI_DOC_SOURCE_BINDING_LIST_CONTINUATION_INDENT; p.ordinal = k;
            p.start = s.iSourceStart; p.end = s.iSourceEnd; p.first = s.iIndentStartColumn; p.content = s.iContentColumn;
            p.last = s.iIndentEndColumn; p.metadata = XUI_DOC_SOURCE_BINDING_COLUMNS;
        } else if ((k -= block.iListIndentCount) < block.iCodeIndentCount) {
            xui_doc_code_indent_t s = {0}; s.iSize = sizeof(s);
            CHECK(xuiDocumentSnapshotGetCodeIndent(snapshot, node, k, &s) == XUI_OK);
            p.origin = XUI_DOC_SOURCE_BINDING_CODE_INDENT; p.ordinal = k; p.flags = s.iFlags;
            p.start = s.iSourceStart; p.end = s.iSourceEnd; p.first = s.iIndentStartColumn;
            p.content = s.iContentColumn == UINT32_MAX ? UINT64_MAX : s.iContentColumn;
            p.last = s.iIndentEndColumn; p.metadata = XUI_DOC_SOURCE_BINDING_COLUMNS;
        } else {
            xui_doc_table_token_t s = {0}; s.iSize = sizeof(s); k -= block.iCodeIndentCount;
            CHECK(xuiDocumentSnapshotGetTableToken(snapshot, node, k, &s) == XUI_OK);
            p.origin = XUI_DOC_SOURCE_BINDING_TABLE_TOKEN; p.ordinal = k; p.flags = s.iFlags;
            p.start = s.iSourceStart; p.end = s.iSourceEnd; p.row = s.iRow; p.position = s.iOrdinal;
            p.metadata = XUI_DOC_SOURCE_BINDING_TABLE;
        }
        CHECK(xuiDocumentSourceIndexVisitBindings(index, p.start, p.end, marker_callback, &p, NULL) == XUI_OK && p.matches == 1);
    }
}
static void provenance_oracle(xui_document_snapshot snapshot, xui_document_source_index index)
{
    xui_doc_source_index_info_t info = {0}; uint64_t i, j;
    info.iSize = sizeof(info); CHECK(xuiDocumentSourceIndexGetInfo(index, &info) == XUI_OK);
    for (i = 0; i < info.iRecordCount; i++) {
        xui_doc_source_record_t r = {0}; xui_doc_node_info_t n = {0}; r.iSize = sizeof(r); n.iSize = sizeof(n);
        CHECK(xuiDocumentSourceIndexGetRecord(index, i, &r) == XUI_OK);
        if (r.iKind != XUI_DOC_SOURCE_RECORD_NODE) continue;
        CHECK(xuiDocumentSnapshotGetNode(snapshot, r.iNodeId, &n) == XUI_OK);
        marker_oracle(snapshot, index, i, r.iNodeId);
        for (j = 0; j < n.iSourceSegmentCount; j++) {
            provenance_oracle_t p = {0}; p.record = i; p.ordinal = j; p.segment.iSize = sizeof(p.segment);
            CHECK(xuiDocumentSnapshotGetSourceSegment(snapshot, r.iNodeId, j, &p.segment) == XUI_OK);
            if (p.segment.iSourceStart == UINT64_MAX) continue;
            CHECK(xuiDocumentSourceIndexVisitBindings(index, p.segment.iSourceStart, p.segment.iSourceEnd,
                provenance_callback, &p, NULL) == XUI_OK && p.matches == 1);
        }
    }
}
static void matrix(void)
{
    unsigned fixture, dialect, native, ending, mode, cases = 0;
    const char* endings[] = {"\n", "\r\n", "\r"};
    for (fixture = 0; fixture < sizeof(fixtures) / sizeof(*fixtures); fixture++)
    for (dialect = 1; dialect <= 3; dialect++) for (native = 0; native < 2; native++)
    for (ending = 0; ending < 3; ending++) for (mode = 0; mode < 2; mode++) {
        char source[4096], shifted[8192]; uint64_t at, size = 0, serialized_bytes; xui_document d, fresh;
        xui_document_snapshot before, after, expected; xui_document_source_index original, changed, full;
        xui_document_transaction txn; xui_doc_txn_desc_t td = {0}; char* serialized;
        for (at = 0; fixtures[fixture][at]; at++) {
            if (fixtures[fixture][at] == '\r' && fixtures[fixture][at + 1] == '\n') continue;
            if (fixtures[fixture][at] == '\n' || fixtures[fixture][at] == '\r') {
                memcpy(source + size, endings[ending], strlen(endings[ending])); size += strlen(endings[ending]);
            } else source[size++] = fixtures[fixture][at];
        }
        source[size] = 0; d = open_source(source, size, dialect, NULL);
        if (native) {
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSerialize(before, &serialized, &serialized_bytes) == XUI_OK);
            xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d);
            CHECK(xuiDocumentDeserialize(NULL, serialized, serialized_bytes, &d) == XUI_OK); xuiDocumentFreeBuffer(serialized);
        }
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
        original = make_index(before); index_check(original, source, size, 1); provenance_oracle(before, original);
        if (!mode) {
            td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
            CHECK(xuiDocumentBeginTransaction(d, &td, &txn) == XUI_OK &&
                xuiDocumentTxnReplaceSource(txn, 0, 0, "prior\n\n", 7) == XUI_OK && xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
            xuiDocumentTxnRelease(txn);
        } else {
            xui_doc_source_patch_t patch = {0}; xui_document_prepare prepare;
            patch.iSize = sizeof(patch); patch.sText = "prior\n\n"; patch.iTextBytes = 7;
            CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &prepare) == XUI_OK && xuiDocumentPrepareRun(prepare) == XUI_OK &&
                xuiDocumentPreparePublish(d, prepare, NULL) == XUI_OK); xuiDocumentPrepareRelease(prepare);
        }
        memcpy(shifted, "prior\n\n", 7); memcpy(shifted + 7, source, (size_t)size + 1);
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); changed = make_index(after); index_check(changed, shifted, size + 7, 1);
        provenance_oracle(after, changed);
        fresh = open_source(shifted, size + 7, dialect, NULL); CHECK(xuiDocumentAcquireSnapshot(fresh, &expected) == XUI_OK);
        full = make_index(expected); compare_indices(changed, full);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d));
        xuiDocumentSnapshotRelease(expected); CHECK(xuiDocumentAcquireSnapshot(d, &expected) == XUI_OK);
        xuiDocumentSourceIndexRelease(full); full = make_index(expected); compare_indices(original, full);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); index_check(original, source, size, 1);
        xuiDocumentSourceIndexRelease(original); xuiDocumentSourceIndexRelease(changed); xuiDocumentSourceIndexRelease(full);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentSnapshotRelease(expected);
        xuiDocumentRelease(d); xuiDocumentRelease(fresh); cases++;
    }
    printf("Source index matrix: %u raw-byte/native/source-shift/fresh-parse/Undo cases\n", cases);
}
static void expected_token(xui_document_source_index index, uint64_t offset, unsigned kind, const char* raw)
{
    uint64_t ordinal; xui_doc_source_token_t t = {0}; char bytes[128]; t.iSize = sizeof(t);
    CHECK(xuiDocumentSourceIndexFindToken(index, offset, &ordinal) == XUI_OK && xuiDocumentSourceIndexGetToken(index, ordinal, &t) == XUI_OK);
    if (t.iKind != kind) fprintf(stderr, "token at %llu: expected %u, got %u\n", (unsigned long long)offset, kind, t.iKind);
    CHECK(t.iKind == kind && offset + strlen(raw) <= t.iSourceEnd && strlen(raw) < sizeof(bytes));
    CHECK(xuiDocumentSourceIndexReadSource(index, offset, bytes, strlen(raw)) == XUI_OK && !memcmp(bytes, raw, strlen(raw)));
}
static void goldens(void)
{
    const char* source = "\xef\xbb\xbf# head #\r\n\r\n[**x**]( <u> \t 'title' )\r\n\r\n[u]: <target> 'name'\r\n\r\n<!-- comment -->\r\n";
    xui_document d = open_source(source, strlen(source), 3, NULL); xui_document_snapshot s; xui_document_source_index index;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); index = make_index(s);
    expected_token(index, 0, XUI_DOC_SOURCE_TOKEN_BOM, "\xef\xbb\xbf");
    expected_token(index, 3, XUI_DOC_SOURCE_TOKEN_BLOCK_MARKER, "#");
    expected_token(index, strstr(source, "head") - source, XUI_DOC_SOURCE_TOKEN_CONTENT, "head");
    expected_token(index, strstr(source, "**x") - source, XUI_DOC_SOURCE_TOKEN_INLINE_MARKER, "**");
    expected_token(index, strstr(source, "<u>") - source + 1, XUI_DOC_SOURCE_TOKEN_FIELD_CONTENT, "u");
    expected_token(index, strstr(source, "title") - source, XUI_DOC_SOURCE_TOKEN_FIELD_CONTENT, "title");
    expected_token(index, strstr(source, " \t '") - source, XUI_DOC_SOURCE_TOKEN_HORIZONTAL_SPACE, " \t ");
    expected_token(index, strstr(source, "target") - source, XUI_DOC_SOURCE_TOKEN_REFERENCE_DESTINATION, "target");
    expected_token(index, strstr(source, "name") - source, XUI_DOC_SOURCE_TOKEN_REFERENCE_TITLE, "name");
    expected_token(index, strstr(source, "<!--") - source, XUI_DOC_SOURCE_TOKEN_HTML_COMMENT, "<!-- comment -->");
    index_check(index, source, strlen(source), 1); xuiDocumentSourceIndexRelease(index); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    puts("Source index goldens: explicit BOM/heading/inline/field/definition/comment roles");
}
typedef struct tab_golden_t {
    xui_document_snapshot snapshot;
    unsigned origin, columns, provenance, text_bytes;
    uint64_t start, marker, content, end, tab;
    const char* text;
} tab_golden_t;
static int tab_callback(void* user, uint64_t ordinal, const xui_doc_source_binding_t* b)
{
    tab_golden_t* g = user; (void)ordinal;
    if (b->iOriginKind == g->origin) {
        CHECK(b->iMetadata == XUI_DOC_SOURCE_BINDING_COLUMNS && b->iStartColumn == g->start &&
            b->iMarkerColumn == g->marker && b->iContentColumn == g->content && b->iEndColumn == g->end &&
            (b->tRange.iFlags & XUI_DOC_SOURCE_TOKEN_PARSER_CONFIRMED));
        g->columns++;
    }
    if (g->text && b->iOriginKind == XUI_DOC_SOURCE_BINDING_PROVENANCE && b->tRange.iSourceStart == g->tab && b->tRange.iSourceEnd == g->tab + 1) {
        char text[64]; uint64_t count, at; xui_doc_node_info_t n = {0}; n.iSize = sizeof(n);
        CHECK(xuiDocumentSnapshotCopyText(g->snapshot, b->tRange.iNodeId, text, sizeof(text), &count) == XUI_OK);
        CHECK(b->tRange.iSegmentKind == XUI_DOC_SOURCE_NORMALIZED && b->tRange.iTextEnd - b->tRange.iTextStart == g->text_bytes &&
            xuiDocumentSnapshotGetNode(g->snapshot, b->tRange.iNodeId, &n) == XUI_OK && (n.tAttributes.iMarks & XUI_DOC_CODE));
        for (at = b->tRange.iTextStart; at < b->tRange.iTextEnd; at++) CHECK(at < count && text[at] == ' ');
        g->provenance++; 
    }
    return 0;
}
static void code_projection(xui_document_snapshot snapshot, uint64_t id, char* text, uint64_t* size)
{
    xui_doc_node_info_t n = {0}; uint64_t i; n.iSize = sizeof(n); CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &n) == XUI_OK);
    if (n.iKind == XUI_DOC_TEXT && (n.tAttributes.iMarks & XUI_DOC_CODE)) {
        uint64_t bytes;
        CHECK(xuiDocumentSnapshotCopyText(snapshot, id, text + *size, 64 - *size, &bytes) == XUI_OK);
        *size += bytes;
    }
    for (i = 0; i < n.iChildCount; i++) {
        uint64_t child; CHECK(xuiDocumentSnapshotGetChild(snapshot, id, i, &child) == XUI_OK);
        code_projection(snapshot, child, text, size);
    }
}
static void binding_goldens(void)
{
    const char* source[] = {"- z `aa\n\tbb`\n", "> z `aa\n>\tbb`\n", "1. z `aa\n\tbb`\n", "> - z `aa\n>\tbb`\n"};
    unsigned dialect, i, cases = 0;
    for (dialect = 1; dialect <= 3; dialect++) for (i = 0; i < 4; i++) {
        fail_allocator a = {-1, 0}; xui_document d = open_source(source[i], strlen(source[i]), dialect, &a);
        xui_document_snapshot s; xui_document_source_index index; tab_golden_t g = {0}; unsigned live;
        CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); index = make_index(s); live = a.live;
        g.snapshot = s; g.tab = strchr(source[i], '\t') - source[i]; g.start = 0; g.end = 4;
        g.marker = i == 1 || i == 3 ? 0 : UINT64_MAX;
        g.content = i == 2 ? 3 : 2; g.text_bytes = i == 2 ? 1 : 2;
        g.origin = i == 1 || i == 3 ? XUI_DOC_SOURCE_BINDING_QUOTE_INDENT : XUI_DOC_SOURCE_BINDING_LIST_CONTINUATION_INDENT;
        g.text = i == 3 ? NULL : i == 2 ? "aa  bb" : "aa   bb";
        if (g.text) {
            char text[64] = {0}; uint64_t bytes = 0; code_projection(s, 1, text, &bytes);
            CHECK(bytes == strlen(g.text) && !strcmp(text, g.text));
        }
        a.remaining = 0;
        CHECK(xuiDocumentSourceIndexVisitBindings(index, g.tab, g.tab + 1, tab_callback, &g, NULL) == XUI_OK &&
            g.columns == 1 && (i == 3 || g.provenance == 1));
        if (i == 3) {
            g.columns = 0; g.origin = XUI_DOC_SOURCE_BINDING_LIST_CONTINUATION_INDENT;
            g.start = 2; g.marker = UINT64_MAX; g.content = 4;
            CHECK(xuiDocumentSourceIndexVisitBindings(index, g.tab, g.tab, tab_callback, &g, NULL) == XUI_OK && g.columns == 1);
        }
        CHECK(a.live == live); a.remaining = -1;
        provenance_oracle(s, index); index_check(index, source[i], strlen(source[i]), 1);
        xuiDocumentSourceIndexRelease(index); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d); CHECK(!a.live); cases++;
    }
    {
        const char* raw = "[x](<> '')\n\n    code\n\n    more\n";
        xui_document d = open_source(raw, strlen(raw), 3, NULL); xui_document_snapshot s;
        xui_document_source_index index; xui_doc_source_index_info_t info = {0}; uint64_t j; unsigned fields = 0, blank = 0;
        CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); index = make_index(s);
        info.iSize = sizeof(info); CHECK(xuiDocumentSourceIndexGetInfo(index, &info) == XUI_OK);
        for (j = 0; j < info.iBindingCount; j++) {
            xui_doc_source_binding_t b = {0}; b.iSize = sizeof(b); CHECK(xuiDocumentSourceIndexGetBinding(index, j, &b) == XUI_OK);
            if (b.iOriginKind == XUI_DOC_SOURCE_BINDING_INLINE_FIELD) {
                CHECK(b.tRange.iSourceStart == b.tRange.iSourceEnd && b.tRange.iKind == XUI_DOC_SOURCE_TOKEN_FIELD_CONTENT);
                CHECK(b.tRange.iSourceStart == 5 || b.tRange.iSourceStart == 8); fields++;
            }
            if (b.iOriginKind == XUI_DOC_SOURCE_BINDING_CODE_INDENT && (b.iMarkerFlags & XUI_DOC_CODE_INDENT_BLANK)) {
                CHECK(b.iContentColumn == UINT64_MAX && b.tRange.iSourceStart == b.tRange.iSourceEnd &&
                    b.iStartColumn == b.iEndColumn); blank++;
            }
        }
        CHECK(fields == 2 && blank == 1); index_check(index, raw, strlen(raw), 1);
        xuiDocumentSourceIndexRelease(index); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    }
    printf("Source bindings: %u independent Tab/multiple-container/logical-column goldens, no allocation during queries; empty fields and blank-code anchors\n", cases);
}
typedef struct cancellation { long remaining, calls; } cancellation;
static int cancel_after(void* user)
{
    cancellation* c = user; c->calls++;
    if (!c->remaining) return 1;
    if (c->remaining > 0) c->remaining--;
    return 0;
}
static void faults(void)
{
    unsigned fixture; long total_oom = 0, total_cancel = 0;
    const unsigned chosen[] = {3, 4, 7, 10, 11, 12, 14, 19, 25, 26, 28, 31};
    for (fixture = 0; fixture < sizeof(chosen) / sizeof(*chosen); fixture++) {
        fail_allocator a = {-1, 0}; const char* source = fixtures[chosen[fixture]]; xui_document d;
        xui_document_snapshot s; xui_document_source_index index; unsigned live; uint64_t revision;
        xui_doc_stats_t before = {0}, after = {0}; long point; int success = 0;
        d = open_source(source, strlen(source), 3, &a); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && xuiDocumentMarkSaved(d, s) == XUI_OK);
        live = a.live; revision = xuiDocumentGetRevision(d); before.iSize = after.iSize = sizeof(before);
        CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
        for (point = 0; point < 1000; point++) {
            int result; a.remaining = point; index = (xui_document_source_index)(uintptr_t)1;
            result = xuiDocumentSnapshotCreateSourceIndex(s, NULL, &index); a.remaining = -1;
            if (!result) { CHECK(index); xuiDocumentSourceIndexRelease(index); success = 1; total_oom += point; }
            else CHECK(result == XUI_ERROR_OUT_OF_MEMORY && !index);
            CHECK(a.live == live && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d));
            if (success) break;
        }
        CHECK(success); success = 0;
        for (point = 0; point < 10000; point++) {
            cancellation c = {point, 0}; xui_doc_source_index_desc_t desc = {0}; int result;
            desc.iSize = sizeof(desc); desc.onCancel = cancel_after; desc.pCancelUser = &c;
            index = (xui_document_source_index)(uintptr_t)1; result = xuiDocumentSnapshotCreateSourceIndex(s, &desc, &index);
            if (!result) { CHECK(index); xuiDocumentSourceIndexRelease(index); success = 1; total_cancel += point; }
            else CHECK(result == XUI_DOC_ERROR_CANCELLED && !index);
            CHECK(a.live == live && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d));
            if (success) break;
        }
        CHECK(success && xuiDocumentGetStats(d, &after) == XUI_OK && after.iMarkdownParses == before.iMarkdownParses &&
            after.iUndoCount == before.iUndoCount && after.iRedoCount == before.iRedoCount &&
            after.iHistoryBytes == before.iHistoryBytes && after.iCurrentBytes == before.iCurrentBytes && after.iSnapshotCount == before.iSnapshotCount);
        xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d); CHECK(!a.live);
    }
    printf("Source index faults: %ld allocation points / %ld cancellation points, source/revision/savepoint unchanged and zero leaks\n", total_oom, total_cancel);
}
static void limits(void)
{
    fail_allocator a = {-1, 0}; const char* source = fixtures[11]; xui_document d = open_source(source, strlen(source), 3, &a);
    xui_document_snapshot s; xui_document_source_index index; xui_doc_source_index_info_t info = {0};
    xui_doc_source_index_desc_t desc = {0}; unsigned live;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); live = a.live; index = make_index(s);
    info.iSize = sizeof(info); CHECK(xuiDocumentSourceIndexGetInfo(index, &info) == XUI_OK); xuiDocumentSourceIndexRelease(index); CHECK(a.live == live);
    desc.iSize = sizeof(desc); desc.iMaxWorkingBytes = info.iPeakWorkingBytes;
    CHECK(xuiDocumentSnapshotCreateSourceIndex(s, &desc, &index) == XUI_OK); xuiDocumentSourceIndexRelease(index);
    desc.iMaxWorkingBytes--; CHECK(xuiDocumentSnapshotCreateSourceIndex(s, &desc, &index) == XUI_DOC_ERROR_LIMIT && !index && a.live == live);
    desc.iMaxWorkingBytes = 1; CHECK(xuiDocumentSnapshotCreateSourceIndex(s, &desc, &index) == XUI_DOC_ERROR_LIMIT && !index && a.live == live);
    desc.iMaxWorkingBytes = UINT64_MAX; CHECK(xuiDocumentSnapshotCreateSourceIndex(s, &desc, &index) == XUI_OK); xuiDocumentSourceIndexRelease(index);
    xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d); CHECK(!a.live);
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotCreateSourceIndex(s, NULL, &index) == XUI_ERROR_UNSUPPORTED && !index);
    xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d); puts("Source index limits: actual peak cap, rollback, unlimited and Rich unsupported");
}
static void accounting(void)
{
    xui_document d = open_source(fixtures[11], strlen(fixtures[11]), 3, NULL); xui_document_snapshot s;
    xui_document_source_index index; xui_doc_source_index_info_t info = {0};
    xui_doc_memory_stats_t before = {0}, after = {0}, released = {0};
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    before.iSize = after.iSize = released.iSize = sizeof(before); info.iSize = sizeof(info);
    CHECK(xuiDocumentGetMemoryStats(d, &before) == XUI_OK); index = make_index(s);
    CHECK(xuiDocumentSourceIndexGetInfo(index, &info) == XUI_OK && xuiDocumentGetMemoryStats(d, &after) == XUI_OK);
    CHECK(after.iLiveBytes - before.iLiveBytes == info.iAllocatedBytes && after.iOtherBytes - before.iOtherBytes == info.iAllocatedBytes &&
        before.iCurrentBytes == after.iCurrentBytes && before.iHistoryBytes == after.iHistoryBytes && before.iSnapshotCount == after.iSnapshotCount);
    xuiDocumentSourceIndexRelease(index); CHECK(xuiDocumentGetMemoryStats(d, &released) == XUI_OK && !memcmp(&before, &released, sizeof(before)));
    xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    puts("Source index accounting: exact retained allocations in OtherBytes, no extra snapshot/content/history, complete reclamation");
}
typedef struct reader { xui_document_source_index index; const char* source; uint64_t size; } reader;
#ifdef _WIN32
static DWORD WINAPI read_worker(void* user)
#else
static void* read_worker(void* user)
#endif
{
    reader* r = user; unsigned i;
    for (i = 0; i < 8; i++) index_check(r->index, r->source, r->size, 1);
    return 0;
}
static void lifetime(void)
{
    fail_allocator a = {-1, 0}; const char* source = fixtures[11]; xui_document d = open_source(source, strlen(source), 3, &a);
    xui_document_snapshot s; reader r; unsigned i;
#ifdef _WIN32
    HANDLE threads[4];
#else
    pthread_t threads[4];
#endif
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); r.index = make_index(s); r.source = source; r.size = strlen(source);
    xuiDocumentSourceIndexRetain(r.index); xuiDocumentSourceIndexRelease(r.index);
    xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    for (i = 0; i < 4; i++) {
#ifdef _WIN32
        threads[i] = CreateThread(NULL, 0, read_worker, &r, 0, NULL); CHECK(threads[i]);
#else
        CHECK(pthread_create(&threads[i], NULL, read_worker, &r) == 0);
#endif
    }
    for (i = 0; i < 4; i++) {
#ifdef _WIN32
        CHECK(WaitForSingleObject(threads[i], INFINITE) == WAIT_OBJECT_0 && CloseHandle(threads[i]));
#else
        CHECK(pthread_join(threads[i], NULL) == 0);
#endif
    }
    xuiDocumentSourceIndexRelease(r.index); CHECK(!a.live);
    puts("Source index lifetime: four actual reader threads after caller snapshot/Document release, zero leaks");
}
typedef struct release_query_t { xui_document_source_index index; uint64_t bytes; unsigned released, stop; } release_query_t;
static int release_callback(void* user, uint64_t ordinal, const xui_doc_source_binding_t* b)
{
    release_query_t* q = user; xui_doc_source_binding_t copy = {0}; char byte;
    if (!q->released) { xuiDocumentSourceIndexRelease(q->index); q->released = 1; }
    copy.iSize = sizeof(copy);
    CHECK(xuiDocumentSourceIndexGetBinding(q->index, ordinal, &copy) == XUI_OK && !memcmp(&copy, b, sizeof(copy)));
    CHECK(xuiDocumentSourceIndexReadSource(q->index, 0, &byte, 1) == XUI_OK && byte == '>');
    query_check(q->index, 0, q->bytes, 1);
    return q->stop;
}
static void reentrant_lifetime(void)
{
    unsigned stop;
    for (stop = 0; stop < 2; stop++) {
        fail_allocator a = {-1, 0}; const char* source = fixtures[4]; xui_document d = open_source(source, strlen(source), 3, &a);
        xui_document_snapshot s; release_query_t q = {0}; uint64_t count = 0; int result;
        CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); q.index = make_index(s); q.bytes = strlen(source); q.stop = stop;
        xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d); a.remaining = 0;
        result = xuiDocumentSourceIndexVisitBindings(q.index, 0, q.bytes, release_callback, &q, &count);
        CHECK(result == (stop ? XUI_DOC_ERROR_CANCELLED : XUI_OK) && q.released && count && (!stop || count == 1) && !a.live);
    }
    puts("Source binding reentrancy: nested queries and caller's last-reference release inside callbacks, cancellation/completion both reclaim every allocation");
}
typedef struct builder_reader { xui_document_snapshot snapshot; const char* source; uint64_t size; } builder_reader;
#ifdef _WIN32
static DWORD WINAPI build_worker(void* user)
#else
static void* build_worker(void* user)
#endif
{
    builder_reader* r = user; unsigned i;
    for (i = 0; i < 8; i++) {
        xui_document_source_index index = make_index(r->snapshot);
        index_check(index, r->source, r->size, 1); xuiDocumentSourceIndexRelease(index);
    }
    return 0;
}
static void concurrent_builders(void)
{
    builder_reader r; xui_document d = open_source(fixtures[11], strlen(fixtures[11]), 3, NULL);
    xui_doc_memory_stats_t before = {0}, after = {0}; unsigned i;
#ifdef _WIN32
    HANDLE threads[4];
#else
    pthread_t threads[4];
#endif
    CHECK(xuiDocumentAcquireSnapshot(d, &r.snapshot) == XUI_OK); r.source = fixtures[11]; r.size = strlen(r.source);
    xuiDocumentRelease(d); before.iSize = after.iSize = sizeof(before);
    CHECK(xuiDocumentSnapshotGetMemoryStats(r.snapshot, &before) == XUI_OK);
    for (i = 0; i < 4; i++) {
#ifdef _WIN32
        threads[i] = CreateThread(NULL, 0, build_worker, &r, 0, NULL); CHECK(threads[i]);
#else
        CHECK(pthread_create(&threads[i], NULL, build_worker, &r) == 0);
#endif
    }
    for (i = 0; i < 4; i++) {
#ifdef _WIN32
        CHECK(WaitForSingleObject(threads[i], INFINITE) == WAIT_OBJECT_0 && CloseHandle(threads[i]));
#else
        CHECK(pthread_join(threads[i], NULL) == 0);
#endif
    }
    CHECK(xuiDocumentSnapshotGetMemoryStats(r.snapshot, &after) == XUI_OK && !memcmp(&before, &after, sizeof(before)));
    xuiDocumentSnapshotRelease(r.snapshot);
    puts("Source index concurrent construction: four actual worker threads / 32 indices on one retained snapshot, unchanged memory union");
}
static void scale(void)
{
    uint64_t sizes[] = {65536, 1024 * 1024, 10 * 1024 * 1024}; unsigned i;
    for (i = 0; i < 3; i++) {
        uint64_t size = sizes[i], at; char* source = malloc((size_t)size + 1); xui_document d; xui_document_snapshot s;
        xui_document_source_index index; xui_doc_source_index_info_t info = {0}; CHECK(source);
        memset(source, 'x', (size_t)size); source[size] = 0;
        d = open_source(source, size, 3, NULL); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); index = make_index(s);
        info.iSize = sizeof(info); CHECK(xuiDocumentSourceIndexGetInfo(index, &info) == XUI_OK && info.iSourceBytesRead <= 3 && info.iAllocatedBytes < 32768);
        for (at = 0; at < size; at += 4096) { uint64_t ordinal; CHECK(xuiDocumentSourceIndexFindToken(index, at, &ordinal) == XUI_OK); }
        index_check(index, source, size, 0);
        printf("Source index literal scale: %llu bytes / %llu retained bytes / %llu build steps / %llu raw bytes read\n",
            (unsigned long long)size, (unsigned long long)info.iAllocatedBytes, (unsigned long long)info.iBuildSteps, (unsigned long long)info.iSourceBytesRead);
        xuiDocumentSourceIndexRelease(index); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d); free(source);
    }
    for (i = 0; i < 4; i++) {
        const char* paragraph = "> - [x] **label** [![a](image \"asset\")](url 'title') &amp; `code`\n>   continuation\n>\n\n";
        uint64_t n = UINT64_C(64) << i, bytes = strlen(paragraph), at, size = n * bytes;
        char* source = malloc((size_t)size + 1); xui_document d; xui_document_snapshot s;
        xui_document_source_index index; xui_doc_source_index_info_t info = {0}; unsigned logarithm = 1;
        CHECK(source); for (at = 0; at < n; at++) memcpy(source + at * bytes, paragraph, (size_t)bytes); source[size] = 0;
        d = open_source(source, size, 3, NULL); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); index = make_index(s);
        info.iSize = sizeof(info); CHECK(xuiDocumentSourceIndexGetInfo(index, &info) == XUI_OK);
        for (at = info.iAnnotationCount; at > 1; at >>= 1) logarithm++;
        CHECK(info.iBuildSteps < info.iAnnotationCount * logarithm * 32 && !info.iUnclassifiedBytes);
        index_check(index, source, size, 0);
        printf("Source index structured scale: %llu containers / %llu annotations / %llu steps / %llu retained bytes\n",
            (unsigned long long)n, (unsigned long long)info.iAnnotationCount,
            (unsigned long long)info.iBuildSteps, (unsigned long long)info.iAllocatedBytes);
        xuiDocumentSourceIndexRelease(index); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d); free(source);
    }
}
static int transport(void)
{
    uint64_t size;
#ifdef _WIN32
    CHECK(_setmode(_fileno(stdin), _O_BINARY) != -1 && _setmode(_fileno(stdout), _O_BINARY) != -1);
#endif
    while (fread(&size, sizeof(size), 1, stdin) == 1) {
        uint32_t dialect; char* source; xui_document d; xui_document_snapshot s; xui_document_source_index index;
        xui_doc_source_index_info_t info = {0};
        CHECK(size <= 1024 * 1024 && fread(&dialect, sizeof(dialect), 1, stdin) == 1);
        source = malloc((size_t)size + 1); CHECK(source && fread(source, 1, (size_t)size, stdin) == size); source[size] = 0;
        d = open_source(source, size, dialect, NULL); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); index = make_index(s);
        index_check(index, source, size, 1); info.iSize = sizeof(info); CHECK(xuiDocumentSourceIndexGetInfo(index, &info) == XUI_OK);
        printf("{\"bytes\":%llu,\"records\":%llu,\"tokens\":%llu,\"unclassified\":%llu,\"opaque\":%llu}\n",
            (unsigned long long)size, (unsigned long long)info.iRecordCount, (unsigned long long)info.iTokenCount,
            (unsigned long long)info.iUnclassifiedBytes, (unsigned long long)info.iOpaqueBytes);
        xuiDocumentSourceIndexRelease(index); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d); free(source);
    }
    CHECK(feof(stdin)); return 0;
}
int main(int argc, char** argv)
{
    if (argc == 2 && !strcmp(argv[1], "--transport")) return transport();
    goldens(); binding_goldens(); matrix(); faults(); limits(); accounting(); lifetime(); reentrant_lifetime(); concurrent_builders(); scale();
    puts("Unified source index public API tests passed"); return 0;
}
