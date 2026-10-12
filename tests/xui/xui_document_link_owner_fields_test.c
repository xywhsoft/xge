#define main xui_document_core_uninvoked
#include "xui_document_test.c"
#undef main

typedef struct owner_fixture { const char *source, *expected; } owner_fixture;
static const owner_fixture fixtures[] = {
    {"[alpha]( \toldA 'one' \t)[__beta__]( \t<oldB> \"two\" \t)", "[alpha]( \t<new> \"next\" \t)[__beta__]( \t<new> \"next\" \t)"},
    {"[alpha](oldA)[beta](new \"next\")", "[alpha](<new> \"next\")[beta](new \"next\")"},
    {"[alpha][ONE][beta][TWO]", "[alpha](<new> \"next\")[beta](<new> \"next\")"},
    {"[![alt](asset 'keep')]( \toldA 'one' \t)[**beta**][TWO]", "[![alt](asset 'keep')]( \t<new> \"next\" \t)[**beta**](<new> \"next\")"},
    {"<https://example.tests/xge/a*b*c>", "[https\\:\\/\\/example\\.tests\xge\\/a\\*b\\*c](<new> \"next\")"},
    {"<user+tag@example.test>", "[user\\+tag\\@example\\.test](<new> \"next\")"},
    {"<https://例子.tests/xge/路径?x=1&y=2>", "[https\\:\\/\\/例子\\.tests\xge\\/路径\\?x\\=1\\&y\\=2](<new> \"next\")"},
    {"<https://x.test>[_beta_]( oldB 'two' )", "[https\\:\\/\\/x\\.test](<new> \"next\")[_beta_]( <new> \"next\" )"},
    {"[foo\nbar](oldA 'one')[__beta__]( oldB (two))", "[foo\nbar](<new> \"next\")[__beta__]( <new> \"next\")"},
    {"[![![inner](hidden)](asset)](oldA)[![](other)][TWO]", "[![![inner](hidden)](asset)](<new> \"next\")[![](other)](<new> \"next\")"},
    {"[alpha](new 'next')[__beta__]( <new> (next) )", "[alpha](new 'next')[__beta__]( <new> (next) )"}
};
static xui_doc_range_t owner_range(xui_document d, xui_document_snapshot s, uint64_t id, int text_edges)
{
    xui_doc_range_t range = {0}; xui_doc_node_info_t n = {0}; uint64_t i, first = UINT64_MAX, last = 0;
    n.iSize = sizeof(n); CHECK(xuiDocumentSnapshotGetNode(s, id, &n) == XUI_OK);
    for (i = 0; i < n.iChildCount; i++) {
        xui_doc_node_info_t child = {0}; uint64_t child_id; child.iSize = sizeof(child);
        CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child_id) == XUI_OK && xuiDocumentSnapshotGetNode(s, child_id, &child) == XUI_OK);
        if (child.tAttributes.iMarks & XUI_DOC_LINK) { if (first == UINT64_MAX) first = i; last = i; }
    }
    if (first != UINT64_MAX) {
        xui_doc_node_info_t a = {0}, b = {0}; uint64_t aid, bid;
        range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d);
        range.tAnchor.iNodeId = id; range.tAnchor.iKind = XUI_DOC_POSITION_GAP; range.tAnchor.iOffset = first;
        range.tCaret = range.tAnchor; range.tCaret.iOffset = last + 1;
        CHECK(xuiDocumentSnapshotGetChild(s, id, first, &aid) == XUI_OK && xuiDocumentSnapshotGetChild(s, id, last, &bid) == XUI_OK);
        a.iSize = sizeof(a); b.iSize = sizeof(b);
        CHECK(xuiDocumentSnapshotGetNode(s, aid, &a) == XUI_OK && xuiDocumentSnapshotGetNode(s, bid, &b) == XUI_OK);
        if (text_edges) {
            /* Empty inline atoms have no distinct payload start/end. Keep
             * their container GAP while exercising the other TEXT edge. */
            if (a.iKind != XUI_DOC_SOFT_BREAK && a.iKind != XUI_DOC_HARD_BREAK && a.iTextBytes) {
                range.tAnchor.iKind = XUI_DOC_POSITION_TEXT; range.tAnchor.iNodeId = aid; range.tAnchor.iOffset = 0;
            }
            if (b.iKind != XUI_DOC_SOFT_BREAK && b.iKind != XUI_DOC_HARD_BREAK && b.iTextBytes) {
                range.tCaret.iKind = XUI_DOC_POSITION_TEXT; range.tCaret.iNodeId = bid; range.tCaret.iOffset = b.iTextBytes;
            }
        }
        return range;
    }
    for (i = 0; i < n.iChildCount; i++) {
        uint64_t child; CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK);
        range = owner_range(d, s, child, text_edges); if (range.tAnchor.iSize) return range;
    }
    return range;
}
static void owner_source(char* out, const char* raw, unsigned eol, unsigned container)
{
    const char* endings[] = {"\n", "\r\n", "\r"}; const char* p = raw; unsigned line = 0;
    strcpy(out, "before `literal` &amp;  \n\n");
    do {
        const char* start = p; strcat(out, container == 1 ? "> " : container == 2 ? (line ? "  " : "- ") : "");
        while (*p && *p != '\n') p++;
        strncat(out, start, (size_t)(p - start)); strcat(out, endings[eol]);
        if (*p) p++;
        line++;
    } while (*p);
    strcat(out, endings[eol]); strcat(out, "[ONE]: oldA 'one'\n[TWO]: oldB \"two\"\n\nfollowing &amp;  \n");
}
static unsigned owner_context[7];
static void owner_dump(xui_document_snapshot s, uint64_t id, unsigned depth)
{
    xui_doc_node_info_t n = {0}; uint64_t i; char text[256]; n.iSize = sizeof(n);
    CHECK(xuiDocumentSnapshotGetNode(s, id, &n) == XUI_OK);
    fprintf(stderr, "%*sid=%llu kind=%u children=%llu bytes=%llu marks=%u uri=%s title=%s\n", (int)depth * 2, "", (unsigned long long)id,
        n.iKind, (unsigned long long)n.iChildCount, (unsigned long long)n.iTextBytes, n.tAttributes.iMarks, n.sResource ? n.sResource : "", n.sTitle ? n.sTitle : "");
    if (n.iTextBytes && n.iTextBytes < sizeof(text)) { uint64_t bytes; CHECK(xuiDocumentSnapshotCopyText(s, id, text, sizeof(text), &bytes) == XUI_OK); fprintf(stderr, "%*stext=%s\n", (int)depth * 2, "", text); }
    for (i = 0; i < n.iChildCount; i++) { uint64_t child; CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK); owner_dump(s, child, depth + 1); }
}
static int owner_shape(xui_document_snapshot a, uint64_t aid, xui_document_snapshot b, uint64_t bid)
{
    xui_doc_node_info_t x = {0}, y = {0}; uint64_t i; x.iSize = sizeof(x); y.iSize = sizeof(y);
    CHECK(xuiDocumentSnapshotGetNode(a, aid, &x) == XUI_OK && xuiDocumentSnapshotGetNode(b, bid, &y) == XUI_OK);
    if (x.iKind != y.iKind || x.iChildCount != y.iChildCount || x.iTextBytes != y.iTextBytes) return 0;
    for (i = 0; i < x.iChildCount; i++) { uint64_t xc, yc; CHECK(xuiDocumentSnapshotGetChild(a, aid, i, &xc) == XUI_OK && xuiDocumentSnapshotGetChild(b, bid, i, &yc) == XUI_OK); if (!owner_shape(a, xc, b, yc)) return 0; }
    return 1;
}
typedef struct owner_event { unsigned type, byte; xui_doc_node_info_t info; } owner_event;
typedef struct owner_stream {
    owner_event* events; uint64_t count, capacity;
    xui_doc_source_segment_t* segments; uint64_t segment_count, segment_capacity;
} owner_stream;
static void owner_event_add(owner_stream* stream, unsigned type, unsigned byte, const xui_doc_node_info_t* info)
{
    if (stream->count == stream->capacity) {
        stream->capacity = stream->capacity ? stream->capacity * 2 : 128;
        stream->events = realloc(stream->events, (size_t)stream->capacity * sizeof(*stream->events)); CHECK(stream->events);
    }
    stream->events[stream->count++] = (owner_event){type, byte, *info};
}
static void owner_public_stream(xui_document_snapshot s, uint64_t id, owner_stream* stream)
{
    xui_doc_node_info_t n = {0}; uint64_t i, base; char* text; n.iSize = sizeof(n);
    CHECK(xuiDocumentSnapshotGetNode(s, id, &n) == XUI_OK && !n.iExtensionPayloadBytes);
    if (n.iKind != XUI_DOC_TEXT) owner_event_add(stream, 1000 + n.iKind, 0, &n);
    else CHECK(!n.iChildCount);
    base = stream->count;
    text = malloc((size_t)n.iTextBytes + 1); CHECK(text);
    CHECK(xuiDocumentSnapshotCopyText(s, id, text, n.iTextBytes + 1, &i) == XUI_OK);
    for (i = 0; i < n.iTextBytes; i++) owner_event_add(stream, n.iKind == XUI_DOC_TEXT ? 1 : 2000 + n.iKind, (unsigned char)text[i], &n);
    free(text);
    for (i = 0; i < n.iSourceSegmentCount; i++) {
        xui_doc_source_segment_t segment = {0}; segment.iSize = sizeof(segment);
        CHECK(xuiDocumentSnapshotGetSourceSegment(s, id, i, &segment) == XUI_OK);
        segment.iTextStart += base; segment.iTextEnd += base;
        if (stream->segment_count) {
            xui_doc_source_segment_t* previous = &stream->segments[stream->segment_count - 1];
            if (previous->iKind == segment.iKind && previous->iFlags == segment.iFlags &&
                previous->iTextEnd == segment.iTextStart && previous->iSourceEnd == segment.iSourceStart) {
                previous->iTextEnd = segment.iTextEnd; previous->iSourceEnd = segment.iSourceEnd; continue;
            }
        }
        if (stream->segment_count == stream->segment_capacity) {
            stream->segment_capacity = stream->segment_capacity ? stream->segment_capacity * 2 : 64;
            stream->segments = realloc(stream->segments, (size_t)stream->segment_capacity * sizeof(*stream->segments)); CHECK(stream->segments);
        }
        stream->segments[stream->segment_count++] = segment;
    }
    for (i = 0; i < n.iChildCount; i++) { uint64_t child; CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK); owner_public_stream(s, child, stream); }
    if (n.iKind != XUI_DOC_TEXT) owner_event_add(stream, 3000 + n.iKind, 0, &n);
}
static int owner_public_equal(xui_document_snapshot expected, xui_document_snapshot actual)
{
    owner_stream a = {0}, b = {0}; uint64_t i; int equal = 1;
    owner_public_stream(expected, 1, &a); owner_public_stream(actual, 1, &b);
    if (a.count != b.count || a.segment_count != b.segment_count) equal = 0;
    for (i = 0; i < a.count && equal; i++) {
        const xui_doc_node_info_t *x = &a.events[i].info, *y = &b.events[i].info;
        equal = a.events[i].type == b.events[i].type && a.events[i].byte == b.events[i].byte &&
            !memcmp(&x->tAttributes, &y->tAttributes, sizeof(x->tAttributes)) &&
            !strcmp(x->sResource, y->sResource) && !strcmp(x->sInfo, y->sInfo) && !strcmp(x->sTitle, y->sTitle) &&
            !strcmp(x->sLinkTarget, y->sLinkTarget) && !strcmp(x->sLinkTitle, y->sLinkTitle) &&
            x->iExtensionVersion == y->iExtensionVersion && x->bExtensionRequired == y->bExtensionRequired;
        if (a.events[i].type >= 1000) equal &= x->iSyntaxStart == y->iSyntaxStart && x->iSyntaxEnd == y->iSyntaxEnd;
    }
    for (i = 0; i < a.segment_count && equal; i++) {
        if (memcmp(&a.segments[i], &b.segments[i], sizeof(a.segments[i])))
            fprintf(stderr, "owner canonical origin mismatch index=%llu expected=%llu..%llu/%llu..%llu actual=%llu..%llu/%llu..%llu\n", (unsigned long long)i,
                (unsigned long long)a.segments[i].iTextStart, (unsigned long long)a.segments[i].iTextEnd, (unsigned long long)a.segments[i].iSourceStart, (unsigned long long)a.segments[i].iSourceEnd,
                (unsigned long long)b.segments[i].iTextStart, (unsigned long long)b.segments[i].iTextEnd, (unsigned long long)b.segments[i].iSourceStart, (unsigned long long)b.segments[i].iSourceEnd);
        equal = !memcmp(&a.segments[i], &b.segments[i], sizeof(a.segments[i]));
    }
    free(a.events); free(b.events); free(a.segments); free(b.segments);
    return equal;
}
static void owner_oracle(xui_document d, xui_document_snapshot actual)
{
    char* source; uint64_t bytes; xui_doc_desc_t desc = {0}; xui_document loaded; xui_document_snapshot expected;
    CHECK(xuiDocumentSnapshotCopySource(actual, NULL, 0, &bytes) == XUI_OK && bytes < SIZE_MAX);
    source = malloc((size_t)bytes + 1); CHECK(source);
    CHECK(xuiDocumentSnapshotCopySource(actual, source, bytes + 1, &bytes) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = xuiDocumentGetMarkdownDialect(d);
    CHECK(xuiDocumentCreate(&desc, &loaded) == XUI_OK && xuiDocumentLoadMarkdown(loaded, source, bytes) == XUI_OK && xuiDocumentAcquireSnapshot(loaded, &expected) == XUI_OK);
    if (!owner_shape(expected, 1, actual, 1)) {
        /* Autolink conversion escapes literal punctuation, which introduces
         * parser text callbacks. The transaction preserves its original ID.
         * Compare all public byte/attribute/object events and normalized source
         * segments, rather than requiring identical text leaf boundaries. */
        CHECK(owner_context[0] >= 4 && owner_context[0] <= 7);
        CHECK(owner_public_equal(expected, actual));
    } else inc_snapshot_equal(expected, actual);
    expect_same_syntax(expected, actual); xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(loaded); free(source);
}
static void owner_operations(xui_document_snapshot before, xui_document_change_set changes)
{
    xui_doc_source_info_t source = {0}; xui_doc_change_info_t info = {0}; uint64_t *starts, *ends, capacity, count = 0, i, j, previous = UINT64_MAX;
    source.iSize = sizeof(source); info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetSourceInfo(before, &source) == XUI_OK && xuiDocumentChangeSetGetInfo(changes, &info) == XUI_OK);
    capacity = source.iInlineSyntaxCount ? source.iInlineSyntaxCount : 1;
    CHECK(capacity <= SIZE_MAX / sizeof(*starts));
    starts = malloc((size_t)capacity * sizeof(*starts)); ends = malloc((size_t)capacity * sizeof(*ends)); CHECK(starts && ends);
    for (i = 0; i < source.iInlineSyntaxCount; i++) {
        xui_doc_inline_syntax_t owner = {0}; uint64_t parent; int visible = 1; owner.iSize = sizeof(owner);
        CHECK(xuiDocumentSnapshotGetInlineSyntax(before, i, &owner) == XUI_OK);
        if (owner.iKind != XUI_DOC_SYNTAX_LINK) continue;
        parent = owner.iParentIndex;
        while (parent != UINT64_MAX) {
            xui_doc_inline_syntax_t ancestor = {0}; ancestor.iSize = sizeof(ancestor);
            CHECK(xuiDocumentSnapshotGetInlineSyntax(before, parent, &ancestor) == XUI_OK);
            if (ancestor.iKind == XUI_DOC_SYNTAX_IMAGE) visible = 0;
            parent = ancestor.iParentIndex;
        }
        if (!visible) continue;
        if (owner.iFlags & XUI_DOC_SYNTAX_AUTOLINK) { starts[count] = owner.iSourceStart; ends[count++] = owner.iSourceEnd; }
        else if (owner.iDefinitionIndex != UINT64_MAX) { starts[count] = owner.iContentEnd + 1; ends[count++] = owner.iSourceEnd; }
        else for (j = i + 1; j < source.iInlineSyntaxCount; j++) {
            xui_doc_inline_syntax_t field = {0}; field.iSize = sizeof(field);
            CHECK(xuiDocumentSnapshotGetInlineSyntax(before, j, &field) == XUI_OK);
            if (field.iSourceStart >= owner.iSourceEnd) break;
            if (field.iParentIndex == i && (field.iKind == XUI_DOC_SYNTAX_DESTINATION || field.iKind == XUI_DOC_SYNTAX_TITLE)) {
                CHECK(count < capacity); starts[count] = field.iSourceStart; ends[count++] = field.iSourceEnd;
            }
        }
    }
    for (i = 0; i < info.iOperationCount; i++) if (info.pOperations[i].iKind == XUI_DOC_OP_SOURCE) {
        const xui_doc_operation_t* op = &info.pOperations[i]; int found = 0;
        for (j = 0; j < count; j++) if (op->iOffset == starts[j] && op->iOldLength == ends[j] - starts[j]) found++;
        CHECK(found == 1 && op->iOffset <= previous); previous = op->iOffset;
    }
    free(starts); free(ends);
}
static void owner_matrix(void)
{
    unsigned fixture, eol, container, dialect, edges, reverse, shift, cases = 0;
    for (fixture = 0; fixture < sizeof(fixtures)/sizeof(*fixtures); fixture++) for (eol = 0; eol < 3; eol++) for (container = 0; container < 3; container++)
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) for (edges = 0; edges < 2; edges++) for (reverse = 0; reverse < 2; reverse++) for (shift = 0; shift < 3; shift++) {
        char input[8192], expected[8192], actual[8192]; xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t;
        xui_document_snapshot before, after, restored; xui_document_change_set changes = NULL; xui_doc_range_t range; xui_doc_link_info_t query = {0}; uint64_t bytes, revision;
        owner_context[0] = fixture; owner_context[1] = eol; owner_context[2] = container; owner_context[3] = dialect; owner_context[4] = edges; owner_context[5] = reverse; owner_context[6] = shift;
        owner_source(input, fixtures[fixture].source, eol, container); owner_source(expected, fixtures[fixture].expected, eol, container);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, input, strlen(input)) == XUI_OK);
        if ((fixture + eol + container + edges + reverse + shift) % 2) {
            char* wire; xui_document loaded;
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSerialize(before, &wire, &bytes) == XUI_OK && xuiDocumentDeserialize(NULL, wire, bytes, &loaded) == XUI_OK);
            xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); d = loaded;
        }
        if (shift) {
            const char* prefix = "earlier **span**\n\n"; uint64_t length = strlen(prefix);
            if (shift == 1) {
                xui_doc_txn_desc_t td = {0}; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
                CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK && xuiDocumentTxnReplaceSource(t, 0, 0, prefix, length) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
            } else {
                xui_doc_source_patch_t patch = {0}; xui_document_prepare prepare; patch.iSize = sizeof(patch); patch.sText = prefix; patch.iTextBytes = length;
                CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &prepare) == XUI_OK && xuiDocumentPrepareRun(prepare) == XUI_OK && xuiDocumentPreparePublish(d, prepare, NULL) == XUI_OK); xuiDocumentPrepareRelease(prepare);
            }
            memmove(expected + length, expected, strlen(expected) + 1); memcpy(expected, prefix, (size_t)length);
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
        range = owner_range(d, before, 1, edges); CHECK(range.tAnchor.iSize);
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        revision = xuiDocumentGetRevision(d); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        {
            int result = xuiDocumentTxnSetLink(t, &range, "new", "next");
            if (result) fprintf(stderr, "owner command fixture=%u eol=%u container=%u dialect=%u edges=%u reverse=%u shift=%u result=%d\n", fixture, eol, container, dialect, edges, reverse, shift, result);
            CHECK(result == XUI_OK && xuiDocumentTxnCommit(t, &changes) == XUI_OK);
        }
        xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK && xuiDocumentSnapshotCopySource(after, actual, sizeof(actual), &bytes) == XUI_OK);
        if (strcmp(actual, expected)) fprintf(stderr, "owner spelling fixture=%u eol=%u container=%u dialect=%u edges=%u reverse=%u shift=%u\nactual:%s\nexpected:%s\n", fixture, eol, container, dialect, edges, reverse, shift, actual, expected);
        CHECK(bytes == strlen(expected) && !memcmp(actual, expected, (size_t)bytes)); owner_oracle(d, after);
        range = owner_range(d, after, 1, edges); query.iSize = sizeof(query);
        CHECK(xuiDocumentSnapshotQueryLink(after, &range, &query) == XUI_OK && query.bLinked && !query.bMixed && !strcmp(query.sUri, "new") && !strcmp(query.sTitle, "next"));
        if (fixture == 10) CHECK(!changes && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
        else {
            owner_operations(before, changes);
            CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); expect_identity_tree(before, restored, 1); expect_same_syntax(before, restored); xuiDocumentSnapshotRelease(restored);
            CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); expect_identity_tree(after, restored, 1); expect_same_syntax(after, restored); xuiDocumentSnapshotRelease(restored);
        }
        xuiDocumentChangeSetRelease(changes); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); cases++;
    }
    printf("Link owner golden: %u independent raw-source multi-owner/mixed URI/title/reference/image/auto-link/code punctuation/Unicode/no-op scenarios, exact descending field operations, EOL/container/dialect/TEXT/GAP/reverse/native/SOURCE/Prepare and full oracle/history passed\n", cases);
}
static void owner_faults(void)
{
    unsigned fixture; long oom = 0;
    for (fixture = 0; fixture < 3; fixture++) {
        const unsigned samples[] = {0, 3, 6}; long point; int succeeded = 0; char input[8192];
        owner_context[0] = samples[fixture]; owner_context[1] = owner_context[2] = fixture;
        owner_source(input, fixtures[samples[fixture]].source, fixture, fixture);
        for (point = 0; point < 4096 && !succeeded; point++) {
            fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t = NULL; xui_document_snapshot before, after;
            xui_doc_range_t range; uint64_t revision; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, input, strlen(input)) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK &&
                xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
            range = owner_range(d, before, 1, fixture % 2); revision = xuiDocumentGetRevision(d); a.remaining = point;
            result = xuiDocumentBeginTransaction(d, NULL, &t);
            if (result == XUI_OK) result = xuiDocumentTxnSetLink(t, &range, "new", "next\nline\rend");
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            else if (t) CHECK(xuiDocumentTxnCommit(t, NULL) == result);
            a.remaining = -1; xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result != XUI_OK) { CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
                inc_snapshot_equal(before, after); expect_same_syntax(before, after); expect_identity_tree(before, after, 1); oom++; }
            else { owner_oracle(d, after); succeeded = 1; }
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(succeeded);
    }
    printf("Link owner faults: %ld OOM checkpoints; projection/field-plan/autolink/publication state, identity/history/savepoint atomic and leak-free\n", oom);
}
static void owner_oracle_selftest(void)
{
    xui_document docs[2]; xui_document_snapshot a, b, wrong; uint64_t second = 0; unsigned split;
    for (split = 0; split < 2; split++) {
        xui_document_transaction t; xui_doc_node_desc_t text = {0}; uint64_t paragraph, first;
        CHECK(xuiDocumentCreate(NULL, &docs[split]) == XUI_OK && xuiDocumentBeginTransaction(docs[split], NULL, &t) == XUI_OK);
        paragraph = insert(t, 1, XUI_DOC_PARAGRAPH, NULL); text.iSize = sizeof(text); text.iKind = XUI_DOC_TEXT;
        text.tAttributes.iMarks = XUI_DOC_LINK; text.sResource = "target"; text.sTitle = "title";
        text.sText = split ? "a" : "ab"; text.iTextBytes = split ? 1 : 2;
        CHECK(xuiDocumentTxnInsertNode(t, paragraph, 0, &text, &first) == XUI_OK);
        if (split) { text.sText = "b"; CHECK(xuiDocumentTxnInsertNode(t, paragraph, 1, &text, &second) == XUI_OK); }
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    }
    CHECK(xuiDocumentAcquireSnapshot(docs[0], &a) == XUI_OK && xuiDocumentAcquireSnapshot(docs[1], &b) == XUI_OK &&
        !owner_shape(a, 1, b, 1) && owner_public_equal(a, b));
    {
        xui_document_transaction t;
        CHECK(xuiDocumentBeginTransaction(docs[1], NULL, &t) == XUI_OK && xuiDocumentTxnSetResource(t, second, "wrong", "", "title") == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(docs[1], &wrong) == XUI_OK && !owner_public_equal(a, wrong)); xuiDocumentSnapshotRelease(wrong);
        CHECK(xuiDocumentBeginTransaction(docs[1], NULL, &t) == XUI_OK && xuiDocumentTxnSetResource(t, second, "target", "", "title") == XUI_OK &&
            xuiDocumentTxnReplaceText(t, second, 0, 1, "c", 1) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(docs[1], &wrong) == XUI_OK && !owner_public_equal(a, wrong)); xuiDocumentSnapshotRelease(wrong);
    }
    xuiDocumentSnapshotRelease(a); xuiDocumentSnapshotRelease(b); xuiDocumentRelease(docs[0]); xuiDocumentRelease(docs[1]);
    puts("Link owner public oracle self-test: text fragmentation equivalent, wrong URI and text rejected; all attributes/typed hierarchy and resolved source segments compared");
}
static void owner_edges(xui_document d, xui_document_snapshot s, uint64_t id, xui_doc_range_t* range)
{
    xui_doc_node_info_t n = {0}; uint64_t i; n.iSize = sizeof(n);
    CHECK(xuiDocumentSnapshotGetNode(s, id, &n) == XUI_OK);
    if (n.tAttributes.iMarks & XUI_DOC_LINK) {
        xui_doc_position_t first = {0}, last;
        first.iSize = sizeof(first); first.iDocumentId = xuiDocumentGetIdentity(d); first.iRevision = xuiDocumentGetRevision(d);
        first.iNodeId = id; first.iKind = XUI_DOC_POSITION_TEXT; last = first; last.iOffset = n.iTextBytes;
        if (!n.iTextBytes) {
            xui_doc_node_info_t parent = {0}; uint64_t slot = UINT64_MAX; parent.iSize = sizeof(parent);
            CHECK(xuiDocumentSnapshotGetNode(s, n.iParentId, &parent) == XUI_OK);
            for (i = 0; i < parent.iChildCount; i++) { uint64_t child; CHECK(xuiDocumentSnapshotGetChild(s, n.iParentId, i, &child) == XUI_OK); if (child == id) slot = i; }
            CHECK(slot != UINT64_MAX); first.iKind = XUI_DOC_POSITION_GAP; first.iNodeId = n.iParentId; first.iOffset = slot;
            last = first; last.iOffset++;
        }
        if (!range->tAnchor.iSize) range->tAnchor = first;
        range->tCaret = last;
    }
    for (i = 0; i < n.iChildCount; i++) { uint64_t child; CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK); owner_edges(d, s, child, range); }
}
static void owner_cross_containers(void)
{
    const char* input = "before `literal`\n\n[alpha]( \toldA 'one' \t)\n\n> [__beta__][TWO]\n\n- [![alt](asset 'keep')]( oldB (two) )\n\n[TWO]: oldB 'two'\n\nfollowing &amp;  \n";
    const char* output = "before `literal`\n\n[alpha]( \t<new> \"next\" \t)\n\n> [__beta__](<new> \"next\")\n\n- [![alt](asset 'keep')]( <new> \"next\" )\n\n[TWO]: oldB 'two'\n\nfollowing &amp;  \n";
    unsigned dialect, reverse, native, eol, cases = 0;
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) for (reverse = 0; reverse < 2; reverse++) for (native = 0; native < 2; native++) for (eol = 0; eol < 3; eol++) {
        char source[8192], expected[8192], actual[8192]; const char* endings[] = {"\n", "\r\n", "\r"};
        const char* p; size_t used = 0; xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t;
        xui_document_snapshot before, after, restored; xui_document_change_set changes; xui_doc_range_t range = {0}; uint64_t bytes;
        for (p = input; *p; p++) { if (*p == '\n') { memcpy(source + used, endings[eol], strlen(endings[eol])); used += strlen(endings[eol]); } else source[used++] = *p; } source[used] = 0;
        used = 0; for (p = output; *p; p++) { if (*p == '\n') { memcpy(expected + used, endings[eol], strlen(endings[eol])); used += strlen(endings[eol]); } else expected[used++] = *p; } expected[used] = 0;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        if (native) { char* wire; xui_document loaded;
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSerialize(before, &wire, &bytes) == XUI_OK && xuiDocumentDeserialize(NULL, wire, bytes, &loaded) == XUI_OK);
            xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); d = loaded;
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        owner_edges(d, before, 1, &range); CHECK(range.tAnchor.iSize);
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnSetLink(t, &range, "new", "next") == XUI_OK && xuiDocumentTxnCommit(t, &changes) == XUI_OK);
        xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK && xuiDocumentSnapshotCopySource(after, actual, sizeof(actual), &bytes) == XUI_OK);
        if (strcmp(actual, expected)) fprintf(stderr, "owner cross spelling dialect=%u reverse=%u native=%u eol=%u\nactual:%s\nexpected:%s\n", dialect, reverse, native, eol, actual, expected);
        CHECK(bytes == strlen(expected) && !memcmp(actual, expected, (size_t)bytes)); owner_oracle(d, after); owner_operations(before, changes);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); expect_identity_tree(before, restored, 1); expect_same_syntax(before, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); expect_identity_tree(after, restored, 1); expect_same_syntax(after, restored); xuiDocumentSnapshotRelease(restored);
        xuiDocumentChangeSetRelease(changes); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); cases++;
    }
    printf("Link owner cross-container: %u paragraph/quote/list/reference/image selections; exact original bytes/field operations, full oracle and one-step history passed\n", cases);
}
static void owner_many(void)
{
    unsigned dialect, reverse, native, shift, cases = 0;
    const unsigned samples[] = {0, 2, 4, 9};
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_GFM; dialect++) for (reverse = 0; reverse < 2; reverse++)
        for (native = 0; native < 2; native++) for (shift = 0; shift < 2; shift++) {
        char *source = malloc(65536), *expected = malloc(65536), *actual = malloc(65536);
        unsigned i; xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t;
        xui_document_snapshot before, after, restored; xui_document_change_set changes; xui_doc_range_t range; uint64_t bytes;
        CHECK(source && expected && actual); strcpy(source, "before `literal`\n\n"); strcpy(expected, source);
        for (i = 0; i < 256; i++) {
            const owner_fixture* f = &fixtures[samples[i % 4]];
            CHECK(strlen(source) + strlen(f->source) + 128 < 65536 && strlen(expected) + strlen(f->expected) + 128 < 65536);
            strcat(source, f->source); strcat(expected, f->expected);
        }
        strcat(source, "\n\n[ONE]: oldA 'one'\n[TWO]: oldB \"two\"\n\nfollowing &amp;  \n");
        strcat(expected, "\n\n[ONE]: oldA 'one'\n[TWO]: oldB \"two\"\n\nfollowing &amp;  \n");
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        if (native) { char* wire; xui_document loaded;
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSerialize(before, &wire, &bytes) == XUI_OK && xuiDocumentDeserialize(NULL, wire, bytes, &loaded) == XUI_OK);
            xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); d = loaded;
        }
        if (shift) {
            xui_doc_txn_desc_t td = {0}; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK && xuiDocumentTxnReplaceSource(t, 0, 0, "prefix\n\n", 8) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
            xuiDocumentTxnRelease(t); memmove(expected + 8, expected, strlen(expected) + 1); memcpy(expected, "prefix\n\n", 8);
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        range = owner_range(d, before, 1, 1); CHECK(range.tAnchor.iSize);
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnSetLink(t, &range, "new", "next") == XUI_OK && xuiDocumentTxnCommit(t, &changes) == XUI_OK);
        xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK && xuiDocumentSnapshotCopySource(after, actual, 65536, &bytes) == XUI_OK);
        CHECK(bytes == strlen(expected) && !memcmp(actual, expected, (size_t)bytes)); owner_context[0] = 4; owner_oracle(d, after); owner_operations(before, changes);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); expect_identity_tree(before, restored, 1); expect_same_syntax(before, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); expect_identity_tree(after, restored, 1); expect_same_syntax(after, restored); xuiDocumentSnapshotRelease(restored);
        xuiDocumentChangeSetRelease(changes); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); free(source); free(expected); free(actual); cases++;
    }
    printf("Link owner many: %u selections of 448 mixed explicit/reference/image/autolink owners; fragmented projections, exact field operations, native/SOURCE/history passed\n", cases);
}
int main(void)
{
    owner_oracle_selftest(); owner_matrix(); owner_faults(); owner_cross_containers(); owner_many(); puts("Link owner field tests passed"); return 0;
}
