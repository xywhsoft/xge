#define main xui_document_core_uninvoked
#include "xui_document_test.c"
#undef main

static void unlink_baseline(void)
{
    const char* source = "before `literal`\n\n[__A__ &#38; \\\\*B\\\\*]( \told 'title' \t)\n\n[keep]: target 'hidden'\n\nfollowing &amp;  \n";
    const char* expected = "before `literal`\n\n__A__ &#38; \\\\*B\\\\*\n\n[keep]: target 'hidden'\n\nfollowing &amp;  \n";
    char actual[2048]; uint64_t paragraph, bytes; xui_doc_desc_t desc = {0}; xui_document d;
    xui_document_snapshot before, after; xui_document_transaction t; xui_doc_range_t range = {0}; xui_doc_node_info_t info = {0};
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_COMMONMARK;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSnapshotGetChild(before, 1, 1, &paragraph) == XUI_OK);
    info.iSize = sizeof(info); CHECK(xuiDocumentSnapshotGetNode(before, paragraph, &info) == XUI_OK && info.iKind == XUI_DOC_PARAGRAPH);
    range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d);
    range.tAnchor.iNodeId = paragraph; range.tAnchor.iKind = XUI_DOC_POSITION_GAP; range.tCaret = range.tAnchor; range.tCaret.iOffset = info.iChildCount;
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnSetLink(t, &range, "", "") == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
    xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK && xuiDocumentSnapshotCopySource(after, actual, sizeof(actual), &bytes) == XUI_OK);
    if (strcmp(actual, expected)) fprintf(stderr, "unlink spelling\nactual:%s\nexpected:%s\n", actual, expected);
    CHECK(bytes == strlen(expected) && !memcmp(actual, expected, (size_t)bytes));
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    puts("Whole-link removal retained label spelling and surrounding source passed");
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

typedef struct unlink_fixture { const char *source, *expected; } unlink_fixture;
static const unlink_fixture unlink_fixtures[] = {
    {"[__A__ &#38; \\*B\\*]( \told 'title' \t)", "__A__ &#38; \\*B\\*"},
    {"[alpha]( oldA 'one' )[**beta**]( \t<oldB> (two) )", "alpha**beta**"},
    {"[__A__][ONE][`B`][TWO]", "__A__`B`"},
    {"[![![inner](hidden)](asset 'keep')](old)[![](other)][TWO]", "![![inner](hidden)](asset 'keep')![](other)"},
    {"<https://example.tests/xge/a*b*c>", "https\\:\\/\\/example\\.tests\\/xge\\/a\\*b\\*c"},
    {"<user+tag@example.test>", "user\\+tag\\@example\\.test"},
    {"[foo\nbar](old 'one')", "foo\nbar"},
    {"[**a** <i>b</i>](old)", "**a** <i>b</i>"},
    {"[![](asset)](old)", "![](asset)"}
};
static void unlink_source(char* out, const char* raw, unsigned eol, unsigned container)
{
    const char* endings[] = {"\n", "\r\n", "\r"}; const char* p = raw; unsigned line = 0;
    strcpy(out, "before `literal` &amp;  \n\n");
    do {
        const char* start = p; strcat(out, container == 1 ? "> " : container == 2 ? (line ? "  " : "- ") : "");
        while (*p && *p != '\n') p++;
        strncat(out, start, (size_t)(p - start)); strcat(out, endings[eol]); if (*p) p++; line++;
    } while (*p);
    strcat(out, endings[eol]); strcat(out, "[ONE]: oldA 'one'\n[TWO]: oldB \"two\"\n\nfollowing &amp;  \n");
}
static void unlink_operations(xui_document_snapshot before, xui_document_change_set changes)
{
    typedef struct unlink_allowed { uint64_t start, end; int replacement; } unlink_allowed;
    xui_doc_source_info_t source = {0}; xui_doc_change_info_t info = {0}; unlink_allowed* allowed;
    uint64_t capacity, count = 0, i, j, actual = 0, previous = UINT64_MAX;
    source.iSize = sizeof(source); info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetSourceInfo(before, &source) == XUI_OK && xuiDocumentChangeSetGetInfo(changes, &info) == XUI_OK);
    CHECK(source.iInlineSyntaxCount <= SIZE_MAX / (2 * sizeof(*allowed)));
    capacity = source.iInlineSyntaxCount ? source.iInlineSyntaxCount * 2 : 1;
    allowed = malloc((size_t)capacity * sizeof(*allowed)); CHECK(allowed);
    for (i = 0; i < source.iInlineSyntaxCount; i++) {
        xui_doc_inline_syntax_t owner = {0}; uint64_t parent, steps = 0; int visible = 1; owner.iSize = sizeof(owner);
        CHECK(xuiDocumentSnapshotGetInlineSyntax(before, i, &owner) == XUI_OK);
        if (owner.iKind != XUI_DOC_SYNTAX_LINK) continue;
        parent = owner.iParentIndex;
        while (parent != UINT64_MAX) {
            xui_doc_inline_syntax_t ancestor = {0}; ancestor.iSize = sizeof(ancestor);
            CHECK(parent < source.iInlineSyntaxCount && steps++ < source.iInlineSyntaxCount && xuiDocumentSnapshotGetInlineSyntax(before, parent, &ancestor) == XUI_OK);
            if (ancestor.iKind == XUI_DOC_SYNTAX_IMAGE) visible = 0;
            parent = ancestor.iParentIndex;
        }
        if (!visible) continue;
        CHECK(count + 2 <= capacity);
        if (owner.iFlags & XUI_DOC_SYNTAX_AUTOLINK) allowed[count++] = (unlink_allowed){owner.iSourceStart, owner.iSourceEnd, 1};
        else {
            allowed[count++] = (unlink_allowed){owner.iSourceStart, owner.iContentStart, 0};
            allowed[count++] = (unlink_allowed){owner.iContentEnd, owner.iSourceEnd, 0};
        }
    }
    for (i = 0; i < info.iOperationCount; i++) if (info.pOperations[i].iKind == XUI_DOC_OP_SOURCE) {
        const xui_doc_operation_t* op = &info.pOperations[i]; unsigned found = 0;
        for (j = 0; j < count; j++) if (op->iOffset == allowed[j].start && op->iOldLength == allowed[j].end - allowed[j].start &&
            (allowed[j].replacement || !op->iNewLength)) found++;
        CHECK(found == 1 && op->iOffset <= previous); previous = op->iOffset; actual++;
    }
    CHECK(actual == count); free(allowed);
}
static void unlink_matrix(void)
{
    unsigned fixture, eol, container, dialect, reverse, native, shift, cases = 0;
    for (fixture = 0; fixture < sizeof(unlink_fixtures)/sizeof(*unlink_fixtures); fixture++) for (eol = 0; eol < 3; eol++) for (container = 0; container < 3; container++)
        for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) for (reverse = 0; reverse < 2; reverse++) for (native = 0; native < 2; native++) for (shift = 0; shift < 3; shift++) {
        char source[4096], expected[4096], actual[4096]; uint64_t block, paragraph, bytes; xui_doc_desc_t desc = {0}; xui_document d, fresh;
        xui_document_snapshot before, after, restored, parsed; xui_document_transaction t; xui_document_change_set changes; xui_doc_range_t range = {0}; xui_doc_node_info_t info = {0};
        unlink_source(source, unlink_fixtures[fixture].source, eol, container); unlink_source(expected, unlink_fixtures[fixture].expected, eol, container);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        if (native) { char* wire; xui_document loaded;
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSerialize(before, &wire, &bytes) == XUI_OK && xuiDocumentDeserialize(NULL, wire, bytes, &loaded) == XUI_OK);
            xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); d = loaded;
        }
        if (shift) {
            if (shift == 1) {
                xui_doc_txn_desc_t td = {0}; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
                CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK && xuiDocumentTxnReplaceSource(t, 0, 0, "prefix\n\n", 8) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
            } else {
                xui_doc_source_patch_t patch = {0}; xui_document_prepare prepare; patch.iSize = sizeof(patch); patch.sText = "prefix\n\n"; patch.iTextBytes = 8;
                CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &prepare) == XUI_OK && xuiDocumentPrepareRun(prepare) == XUI_OK && xuiDocumentPreparePublish(d, prepare, NULL) == XUI_OK); xuiDocumentPrepareRelease(prepare);
            }
            memmove(expected + 8, expected, strlen(expected) + 1); memcpy(expected, "prefix\n\n", 8);
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSnapshotGetChild(before, 1, shift ? 2 : 1, &block) == XUI_OK);
        paragraph = find_kind(before, block, XUI_DOC_PARAGRAPH); CHECK(paragraph);
        info.iSize = sizeof(info); CHECK(xuiDocumentSnapshotGetNode(before, paragraph, &info) == XUI_OK);
        range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d);
        range.tAnchor.iNodeId = paragraph; range.tAnchor.iKind = XUI_DOC_POSITION_GAP; range.tCaret = range.tAnchor; range.tCaret.iOffset = info.iChildCount;
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnSetLink(t, &range, "", "") == XUI_OK && xuiDocumentTxnCommit(t, &changes) == XUI_OK); xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK && xuiDocumentSnapshotCopySource(after, actual, sizeof(actual), &bytes) == XUI_OK);
        if (strcmp(actual, expected)) fprintf(stderr, "unlink spelling fixture=%u eol=%u container=%u dialect=%u reverse=%u native=%u shift=%u\nactual:%s\nexpected:%s\n", fixture, eol, container, dialect, reverse, native, shift, actual, expected);
        CHECK(bytes == strlen(expected) && !memcmp(actual, expected, (size_t)bytes));
        unlink_operations(before, changes);
        CHECK(xuiDocumentCreate(&desc, &fresh) == XUI_OK && xuiDocumentLoadMarkdown(fresh, expected, strlen(expected)) == XUI_OK && xuiDocumentAcquireSnapshot(fresh, &parsed) == XUI_OK);
        CHECK(owner_public_equal(parsed, after)); expect_same_syntax(parsed, after); xuiDocumentSnapshotRelease(parsed); xuiDocumentRelease(fresh);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); expect_identity_tree(before, restored, 1); expect_same_syntax(before, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); expect_identity_tree(after, restored, 1); expect_same_syntax(after, restored); xuiDocumentSnapshotRelease(restored);
        xuiDocumentChangeSetRelease(changes); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); cases++;
    }
    printf("Link removal golden: %u independent complete sources, exact descending wrapper-only SOURCE operations, semantic/public origins/CST, native/SOURCE/Prepare/container/EOL/dialect/reverse/history passed\n", cases);
}
static void unlink_faults(void)
{
    unsigned fixture; long failures = 0; const unsigned samples[] = {0, 3, 4};
    for (fixture = 0; fixture < 3; fixture++) {
        long point; int succeeded = 0; char source[4096], expected[4096];
        unlink_source(source, unlink_fixtures[samples[fixture]].source, fixture, fixture);
        unlink_source(expected, unlink_fixtures[samples[fixture]].expected, fixture, fixture);
        for (point = 0; point < 4096 && !succeeded; point++) {
            fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t = NULL; xui_document_snapshot before, after;
            xui_doc_range_t range = {0}; xui_doc_node_info_t info = {0}; uint64_t block, paragraph, revision, bytes; int result; char actual[4096];
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK &&
                xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK && xuiDocumentSnapshotGetChild(before, 1, 1, &block) == XUI_OK);
            paragraph = find_kind(before, block, XUI_DOC_PARAGRAPH); info.iSize = sizeof(info); CHECK(paragraph && xuiDocumentSnapshotGetNode(before, paragraph, &info) == XUI_OK);
            range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d);
            range.tAnchor.iNodeId = paragraph; range.tAnchor.iKind = XUI_DOC_POSITION_GAP; range.tCaret = range.tAnchor; range.tCaret.iOffset = info.iChildCount;
            revision = xuiDocumentGetRevision(d); a.remaining = point; result = xuiDocumentBeginTransaction(d, NULL, &t);
            if (result == XUI_OK) result = xuiDocumentTxnSetLink(t, &range, "", "");
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL); else if (t) CHECK(xuiDocumentTxnCommit(t, NULL) == result);
            a.remaining = -1; xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result != XUI_OK) {
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
                inc_snapshot_equal(before, after); expect_same_syntax(before, after); expect_identity_tree(before, after, 1); failures++;
            } else {
                xui_document fresh; xui_document_snapshot parsed; xui_doc_desc_t plain = {0}; plain.iSize = sizeof(plain); plain.iProfile = XUI_DOCUMENT_MARKDOWN;
                CHECK(xuiDocumentSnapshotCopySource(after, actual, sizeof(actual), &bytes) == XUI_OK && bytes == strlen(expected) && !memcmp(actual, expected, (size_t)bytes));
                CHECK(xuiDocumentCreate(&plain, &fresh) == XUI_OK && xuiDocumentLoadMarkdown(fresh, expected, bytes) == XUI_OK && xuiDocumentAcquireSnapshot(fresh, &parsed) == XUI_OK);
                CHECK(owner_public_equal(parsed, after)); expect_same_syntax(parsed, after); xuiDocumentSnapshotRelease(parsed); xuiDocumentRelease(fresh); succeeded = 1;
            }
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(succeeded);
    }
    printf("Link removal faults: %ld OOM checkpoints; retained-label plan/autolink/publication source, identity/CST/revision/history/savepoint atomic and leak-free\n", failures);
}
static int unlink_semantic_intent(xui_document_snapshot before, xui_document_snapshot after)
{
    owner_stream a = {0}, b = {0}; uint64_t i; int equal = 1;
    owner_public_stream(before, 1, &a); owner_public_stream(after, 1, &b);
    if (a.count != b.count) equal = 0;
    for (i = 0; i < a.count && equal; i++) {
        xui_doc_node_info_t x = a.events[i].info; const xui_doc_node_info_t* y = &b.events[i].info;
        if (x.tAttributes.iMarks & XUI_DOC_LINK) {
            x.tAttributes.iMarks &= ~XUI_DOC_LINK;
            if (x.iKind == XUI_DOC_IMAGE) x.sLinkTarget = x.sLinkTitle = "";
            else x.sResource = x.sTitle = "";
        }
        equal = a.events[i].type == b.events[i].type && a.events[i].byte == b.events[i].byte &&
            !memcmp(&x.tAttributes, &y->tAttributes, sizeof(x.tAttributes)) &&
            !strcmp(x.sResource, y->sResource) && !strcmp(x.sInfo, y->sInfo) && !strcmp(x.sTitle, y->sTitle) &&
            !strcmp(x.sLinkTarget, y->sLinkTarget) && !strcmp(x.sLinkTitle, y->sLinkTitle) &&
            x.iExtensionVersion == y->iExtensionVersion && x.bExtensionRequired == y->bExtensionRequired;
    }
    free(a.events); free(b.events); free(a.segments); free(b.segments); return equal;
}
static void unlink_boundaries(void)
{
    const char* fixtures[] = {"[# head](old)", "[1. item](old)", "[> text](old)", "[---](old)", "[[ONE]](old)",
        "[a](old)[__beta__](other)", "alpha[__beta__](old)", "[foo](old)*bar*", "[www.example.test](old)", "[user@example.test](old)"};
    unsigned fixture, eol, container, dialect, reverse, cases = 0;
    for (fixture = 0; fixture < sizeof(fixtures)/sizeof(*fixtures); fixture++) for (eol = 0; eol < 3; eol++) for (container = 0; container < 3; container++)
        for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) for (reverse = 0; reverse < 2; reverse++) {
        char source[4096], actual[4096]; uint64_t block, paragraph, bytes; xui_doc_desc_t desc = {0}; xui_document d, fresh;
        xui_document_snapshot before, after, parsed, restored; xui_document_transaction t; xui_doc_range_t range = {0}; xui_doc_node_info_t info = {0}; int result;
        unlink_source(source, fixtures[fixture], eol, container); desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSnapshotGetChild(before, 1, 1, &block) == XUI_OK);
        paragraph = find_kind(before, block, XUI_DOC_PARAGRAPH); info.iSize = sizeof(info); CHECK(paragraph && xuiDocumentSnapshotGetNode(before, paragraph, &info) == XUI_OK);
        range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d);
        range.tAnchor.iNodeId = paragraph; range.tAnchor.iKind = XUI_DOC_POSITION_GAP; range.tCaret = range.tAnchor; range.tCaret.iOffset = info.iChildCount;
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); result = xuiDocumentTxnSetLink(t, &range, "", "");
        if (result != XUI_OK) fprintf(stderr, "unlink boundary command fixture=%u eol=%u container=%u dialect=%u reverse=%u result=%d\n", fixture, eol, container, dialect, reverse, result);
        CHECK(result == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK && xuiDocumentSnapshotCopySource(after, actual, sizeof(actual), &bytes) == XUI_OK);
        if (!unlink_semantic_intent(before, after)) fprintf(stderr, "unlink boundary semantic fixture=%u eol=%u container=%u dialect=%u reverse=%u\nsource:%s\n", fixture, eol, container, dialect, reverse, actual);
        CHECK(unlink_semantic_intent(before, after));
        CHECK(xuiDocumentCreate(&desc, &fresh) == XUI_OK && xuiDocumentLoadMarkdown(fresh, actual, bytes) == XUI_OK && xuiDocumentAcquireSnapshot(fresh, &parsed) == XUI_OK);
        CHECK(owner_public_equal(parsed, after)); expect_same_syntax(parsed, after); xuiDocumentSnapshotRelease(parsed); xuiDocumentRelease(fresh);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); expect_identity_tree(before, restored, 1); expect_same_syntax(before, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); expect_identity_tree(after, restored, 1); expect_same_syntax(after, restored); xuiDocumentSnapshotRelease(restored);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); cases++;
    }
    printf("Link removal boundaries: %u block/list/quote/rule/reference/delimiter/GFM-auto contexts; independent semantic intent, actual-source full parse/origins/CST and history passed\n", cases);
}
static void unlink_compound(void)
{
    const char* source = "before `literal`\n\n[__A__](old)[`B`][TWO]\n\n[TWO]: other 'keep'\n\nfollowing &amp;  \n";
    const char* expected = "before `literal`\n\n__A__`B`\n\n[TWO]: other 'keep'\n\nfollowing &amp;  \n";
    unsigned dialect, reverse, cases = 0;
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) for (reverse = 0; reverse < 2; reverse++) {
        xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t; xui_document_snapshot before, after, restored;
        xui_doc_range_t range = {0}; xui_doc_node_info_t info = {0}; uint64_t paragraph, revision, bytes; char actual[2048];
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(before, 1, 1, &paragraph) == XUI_OK); info.iSize = sizeof(info); CHECK(xuiDocumentSnapshotGetNode(before, paragraph, &info) == XUI_OK);
        range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d);
        range.tAnchor.iNodeId = paragraph; range.tAnchor.iKind = XUI_DOC_POSITION_GAP; range.tCaret = range.tAnchor; range.tCaret.iOffset = info.iChildCount;
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnSetLink(t, &range, "new", "next") == XUI_OK &&
            xuiDocumentTxnSetLink(t, &range, "", "") == XUI_OK && xuiDocumentTxnSetLink(t, &range, "", "") == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK && xuiDocumentSnapshotCopySource(after, actual, sizeof(actual), &bytes) == XUI_OK && bytes == strlen(expected) && !memcmp(actual, expected, (size_t)bytes));
        CHECK(unlink_semantic_intent(before, after));
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); expect_identity_tree(before, restored, 1); expect_same_syntax(before, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); expect_identity_tree(after, restored, 1); expect_same_syntax(after, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentMarkSaved(d, after) == XUI_OK); revision = xuiDocumentGetRevision(d);
        range.tAnchor.iRevision = range.tCaret.iRevision = revision;
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnSetLink(t, &range, "", "") == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        expect_identity_tree(after, restored, 1); expect_same_syntax(after, restored); xuiDocumentSnapshotRelease(restored);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); cases++;
    }
    printf("Link removal compound: %u update/clear/clear transactions, one-step Undo/Redo and unchanged clear revision/history/savepoint passed\n", cases);
}
int main(void) { unlink_baseline(); unlink_matrix(); unlink_faults(); unlink_boundaries(); unlink_compound(); return 0; }
