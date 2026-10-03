#include "../xui_document.h"
#include "../src/xui_document_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #c); exit(1); } } while (0)
static xstrview view(const char* s) { xstrview v = {s, strlen(s)}; return v; }
static void prepare_tree_equal(xui_document_snapshot a, uint64_t ai, xui_document_snapshot b, uint64_t bi)
{
    xui_doc_node_info_t left = {0}, right = {0}; uint64_t i; char* lt; char* rt;
    left.iSize = right.iSize = sizeof(left);
    CHECK(xuiDocumentSnapshotGetNode(a, ai, &left) == XUI_OK && xuiDocumentSnapshotGetNode(b, bi, &right) == XUI_OK);
    CHECK(left.iKind == right.iKind && left.iTextBytes == right.iTextBytes && left.iChildCount == right.iChildCount);
    CHECK(!memcmp(&left.tAttributes, &right.tAttributes, sizeof(left.tAttributes)));
    CHECK(!strcmp(left.sResource, right.sResource) && !strcmp(left.sInfo, right.sInfo) && !strcmp(left.sTitle, right.sTitle));
    CHECK(left.iSourceStart == right.iSourceStart && left.iSourceEnd == right.iSourceEnd && left.bSourceExact == right.bSourceExact);
    CHECK(left.iSyntaxStart == right.iSyntaxStart && left.iSyntaxEnd == right.iSyntaxEnd && left.iSourceSegmentCount == right.iSourceSegmentCount);
    lt = malloc((size_t)left.iTextBytes + 1); rt = malloc((size_t)left.iTextBytes + 1); CHECK(lt && rt);
    CHECK(xuiDocumentSnapshotCopyText(a, ai, lt, left.iTextBytes + 1, &i) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopyText(b, bi, rt, left.iTextBytes + 1, &i) == XUI_OK && !memcmp(lt, rt, (size_t)left.iTextBytes));
    free(lt); free(rt);
    if (left.iKind == XUI_DOC_HEADING) {
        xui_doc_block_syntax_t x = {0}, y = {0}; x.iSize = y.iSize = sizeof(x);
        CHECK(xuiDocumentSnapshotGetBlockSyntax(a, ai, &x) == XUI_OK &&
            xuiDocumentSnapshotGetBlockSyntax(b, bi, &y) == XUI_OK && !memcmp(&x, &y, sizeof(x)));
    }
    for (i = 0; i < left.iSourceSegmentCount; i++) {
        xui_doc_source_segment_t x = {0}, y = {0}; x.iSize = y.iSize = sizeof(x);
        CHECK(xuiDocumentSnapshotGetSourceSegment(a, ai, i, &x) == XUI_OK && xuiDocumentSnapshotGetSourceSegment(b, bi, i, &y) == XUI_OK);
        CHECK(!memcmp(&x, &y, sizeof(x)));
    }
    if (left.iKind == XUI_DOC_SOFT_BREAK || left.iKind == XUI_DOC_HARD_BREAK) {
        xui_doc_break_syntax_t x = {0}, y = {0};
        x.iSize = y.iSize = sizeof(x);
        CHECK(xuiDocumentSnapshotGetBreakSyntax(a, ai, &x) == XUI_OK &&
            xuiDocumentSnapshotGetBreakSyntax(b, bi, &y) == XUI_OK &&
            !memcmp(&x, &y, sizeof(x)));
    }
    if (left.iKind == XUI_DOC_LIST_ITEM) {
        xui_doc_block_syntax_t x = {0}, y = {0};
        x.iSize = y.iSize = sizeof(x);
        CHECK(xuiDocumentSnapshotGetBlockSyntax(a, ai, &x) == XUI_OK &&
            xuiDocumentSnapshotGetBlockSyntax(b, bi, &y) == XUI_OK &&
            x.iListIndentCount == y.iListIndentCount);
        for (i = 0; i < x.iListIndentCount; i++) {
            xui_doc_list_indent_t m = {0}, n = {0};
            m.iSize = n.iSize = sizeof(m);
            CHECK(xuiDocumentSnapshotGetListContinuationIndent(a, ai, i, &m) == XUI_OK &&
                xuiDocumentSnapshotGetListContinuationIndent(b, bi, i, &n) == XUI_OK &&
                !memcmp(&m, &n, sizeof(m)));
        }
    }
    if (left.iKind == XUI_DOC_CODE_BLOCK) {
        xui_doc_block_syntax_t x = {0}, y = {0};
        int xs, ys;
        x.iSize = y.iSize = sizeof(x);
        xs = xuiDocumentSnapshotGetBlockSyntax(a, ai, &x);
        ys = xuiDocumentSnapshotGetBlockSyntax(b, bi, &y);
        CHECK(xs == ys);
        if (xs == XUI_OK && x.iKind == XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE) {
            CHECK(y.iKind == x.iKind && x.iCodeIndentCount == y.iCodeIndentCount);
            for (i = 0; i < x.iCodeIndentCount; i++) {
                xui_doc_code_indent_t m = {0}, n = {0};
                m.iSize = n.iSize = sizeof(m);
                CHECK(xuiDocumentSnapshotGetCodeIndent(a, ai, i, &m) == XUI_OK &&
                    xuiDocumentSnapshotGetCodeIndent(b, bi, i, &n) == XUI_OK &&
                    !memcmp(&m, &n, sizeof(m)));
            }
        }
    }
    for (i = 0; i < left.iChildCount; i++) {
        uint64_t ac, bc; CHECK(xuiDocumentSnapshotGetChild(a, ai, i, &ac) == XUI_OK && xuiDocumentSnapshotGetChild(b, bi, i, &bc) == XUI_OK);
        prepare_tree_equal(a, ac, b, bc);
    }
}
#include "xui_document_reference_cache_oracle.h"
static void prepare_snapshot_equal(xui_document_snapshot a, xui_document_snapshot b)
{
    xui_doc_source_info_t x = {0}, y = {0}; uint64_t i;
    reference_cache_snapshot_equal(a, b);
    prepare_tree_equal(a, 1, b, 1); x.iSize = y.iSize = sizeof(x);
    CHECK(xuiDocumentSnapshotGetSourceInfo(a, &x) == XUI_OK && xuiDocumentSnapshotGetSourceInfo(b, &y) == XUI_OK && !memcmp(&x, &y, sizeof(x)));
    for (i = 0; i < x.iInlineSyntaxCount; i++) {
        xui_doc_inline_syntax_t m = {0}, n = {0}; m.iSize = n.iSize = sizeof(m);
        CHECK(xuiDocumentSnapshotGetInlineSyntax(a, i, &m) == XUI_OK && xuiDocumentSnapshotGetInlineSyntax(b, i, &n) == XUI_OK && !memcmp(&m, &n, sizeof(m)));
    }
    for (i = 0; i < x.iReferenceDefinitionCount; i++) {
        xui_doc_reference_definition_t m = {0}, n = {0}; m.iSize = n.iSize = sizeof(m);
        CHECK(xuiDocumentSnapshotGetReferenceDefinition(a, i, &m) == XUI_OK && xuiDocumentSnapshotGetReferenceDefinition(b, i, &n) == XUI_OK && !memcmp(&m, &n, sizeof(m)));
    }
    for (i = 0; i < x.iReferenceCandidateCount; i++) {
        xui_doc_inline_syntax_t m = {0}, n = {0}; m.iSize = n.iSize = sizeof(m);
        CHECK(xuiDocumentSnapshotGetReferenceCandidate(a, i, &m) == XUI_OK &&
            xuiDocumentSnapshotGetReferenceCandidate(b, i, &n) == XUI_OK && !memcmp(&m, &n, sizeof(m)));
    }
}
static void prepare_corpus(xui_doc_desc_t* desc, xui_document_snapshot reference, xstrview markdown)
{
    xui_document d, oracle; xui_document_prepare p, continued; xui_document_snapshot s, next;
    xui_doc_source_patch_t patches[2] = {{0}}; char* edited = malloc(markdown.Size + 19);
    CHECK(edited); memcpy(edited, "prefix\n\n", 8); memcpy(edited + 8, markdown.Data, markdown.Size);
    memcpy(edited + 8 + markdown.Size, "\n\nsuffix\n", 10);
    CHECK(xuiDocumentCreate(desc, &d) == XUI_OK);
    patches[0].iSize = sizeof(patches[0]); patches[0].sText = markdown.Data; patches[0].iTextBytes = markdown.Size;
    CHECK(xuiDocumentPrepareSource(d, NULL, patches, 1, &p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_OK);
    CHECK(xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); xuiDocumentPrepareRelease(p);
    CHECK(d->state->source_open_brackets == doc_source_open_bracket_count(markdown.Data, markdown.Size));
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); prepare_snapshot_equal(reference, s); xuiDocumentSnapshotRelease(s);
    patches[0].sText = "prefix\n\n"; patches[0].iTextBytes = 8;
    patches[1].iSize = sizeof(patches[1]); patches[1].iStart = patches[1].iEnd = markdown.Size + 8;
    patches[1].sText = "\n\nsuffix\n"; patches[1].iTextBytes = 9;
    CHECK(xuiDocumentPrepareSource(d, NULL, patches, 2, &p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_OK);
    CHECK(xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); xuiDocumentPrepareRelease(p);
    CHECK(d->state->source_open_brackets == doc_source_open_bracket_count(edited, markdown.Size + 17));
    CHECK(xuiDocumentCreate(desc, &oracle) == XUI_OK && xuiDocumentLoadMarkdown(oracle, edited, markdown.Size + 17) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &next) == XUI_OK);
    prepare_snapshot_equal(s, next); xuiDocumentSnapshotRelease(s); xuiDocumentSnapshotRelease(next);
    /* Assemble the same input across pending generations as well as a batch. */
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
    CHECK(xuiDocumentPrepareSource(d, NULL, patches, 1, &p) == XUI_OK);
    if (markdown.Size & 1) CHECK(xuiDocumentPrepareRun(p) == XUI_OK);
    CHECK(xuiDocumentPrepareContinueSource(d, p, patches + 1, 1, &continued) == XUI_OK);
    xuiDocumentPrepareRelease(p);
    CHECK(xuiDocumentPrepareRun(continued) == XUI_OK && xuiDocumentPreparePublish(d, continued, NULL) == XUI_OK);
    xuiDocumentPrepareRelease(continued);
    CHECK(d->state->source_open_brackets == doc_source_open_bracket_count(edited, markdown.Size + 17));
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &next) == XUI_OK);
    prepare_snapshot_equal(s, next); xuiDocumentSnapshotRelease(s); xuiDocumentSnapshotRelease(next);
    xuiDocumentRelease(d); xuiDocumentRelease(oracle); free(edited);
}
static void source_syntax(xui_document_snapshot snapshot, uint64_t bytes, uint64_t* spans, uint64_t* definitions, uint64_t* references)
{
    xui_doc_source_info_t info = {0}; uint64_t i, previous_start = 0;
    info.iSize = sizeof(info); CHECK(xuiDocumentSnapshotGetSourceInfo(snapshot, &info) == XUI_OK);
    for (i = 0; i < info.iReferenceDefinitionCount; i++) {
        xui_doc_reference_definition_t def = {0}; def.iSize = sizeof(def);
        CHECK(xuiDocumentSnapshotGetReferenceDefinition(snapshot, i, &def) == XUI_OK); (*definitions)++;
        CHECK(def.iSourceStart <= def.iLabelStart && def.iLabelStart <= def.iLabelEnd);
        CHECK(def.iLabelEnd <= def.iDestinationStart && def.iDestinationStart <= def.iDestinationEnd);
        CHECK(def.iDestinationEnd <= def.iSourceEnd && def.iSourceEnd <= bytes);
        if (def.iTitleStart == UINT64_MAX) CHECK(def.iTitleEnd == UINT64_MAX);
        else CHECK(def.iDestinationEnd <= def.iTitleStart && def.iTitleStart <= def.iTitleEnd && def.iTitleEnd <= def.iSourceEnd);
    }
    for (i = 0; i < info.iInlineSyntaxCount; i++) {
        xui_doc_inline_syntax_t span = {0}; span.iSize = sizeof(span);
        CHECK(xuiDocumentSnapshotGetInlineSyntax(snapshot, i, &span) == XUI_OK); (*spans)++;
        CHECK(span.iSourceStart >= previous_start); previous_start = span.iSourceStart;
        CHECK(span.iKind >= XUI_DOC_SYNTAX_EMPHASIS && span.iKind <= XUI_DOC_SYNTAX_SUPERSCRIPT);
        CHECK(span.iSourceStart <= span.iContentStart && span.iContentStart <= span.iContentEnd);
        CHECK(span.iContentEnd <= span.iSourceEnd && span.iSourceEnd <= bytes);
        if (span.iParentIndex != UINT64_MAX) {
            xui_doc_inline_syntax_t parent = {0}; parent.iSize = sizeof(parent);
            CHECK(span.iParentIndex < i && xuiDocumentSnapshotGetInlineSyntax(snapshot, span.iParentIndex, &parent) == XUI_OK);
            CHECK(parent.iContentStart <= span.iSourceStart && span.iSourceEnd <= parent.iContentEnd);
        }
        if (span.iDefinitionIndex != UINT64_MAX) {
            xui_doc_reference_definition_t def = {0}; def.iSize = sizeof(def); (*references)++;
            CHECK((span.iKind == XUI_DOC_SYNTAX_LINK || span.iKind == XUI_DOC_SYNTAX_IMAGE) && !(span.iFlags & XUI_DOC_SYNTAX_AUTOLINK));
            CHECK(span.iDefinitionIndex < info.iReferenceDefinitionCount);
            CHECK(xuiDocumentSnapshotGetReferenceDefinition(snapshot, span.iDefinitionIndex, &def) == XUI_OK);
            CHECK(def.iSourceEnd <= span.iSourceStart || def.iSourceStart >= span.iSourceEnd);
        }
    }
    previous_start = 0;
    for (i = 0; i < info.iReferenceCandidateCount; i++) {
        xui_doc_inline_syntax_t candidate = {0}; candidate.iSize = sizeof(candidate);
        CHECK(xuiDocumentSnapshotGetReferenceCandidate(snapshot, i, &candidate) == XUI_OK);
        CHECK(candidate.iKind >= XUI_DOC_SYNTAX_CANDIDATE_LINK &&
            candidate.iKind <= XUI_DOC_SYNTAX_CANDIDATE_FOOTNOTE);
        CHECK(candidate.iSourceStart >= previous_start && candidate.iSourceStart <= candidate.iContentStart &&
            candidate.iContentStart <= candidate.iContentEnd && candidate.iContentEnd <= candidate.iSourceEnd &&
            candidate.iSourceEnd <= bytes);
        CHECK(candidate.iParentIndex == UINT64_MAX && candidate.iDefinitionIndex == UINT64_MAX);
        previous_start = candidate.iSourceStart;
    }
}
static void source_segments(xui_document_snapshot snapshot, uint64_t id, const char* source, uint64_t bytes, uint64_t* count)
{
    xui_doc_node_info_t info = {0};
    uint64_t i, previous = 0, previous_block_end = 0, previous_source_start = 0, previous_source_end = 0, child;
    info.iSize = sizeof(info); CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &info) == XUI_OK);
    for (i = 0; i < info.iSourceSegmentCount; i++) {
        xui_doc_source_segment_t segment = {0}; uint64_t at; char text[256]; segment.iSize = sizeof(segment);
        CHECK(xuiDocumentSnapshotGetSourceSegment(snapshot, id, i, &segment) == XUI_OK); (*count)++;
        CHECK(segment.iTextStart == previous && segment.iTextStart < segment.iTextEnd && segment.iTextEnd <= info.iTextBytes);
        previous = segment.iTextEnd;
        if (segment.iSourceStart == UINT64_MAX) { CHECK(segment.iKind == XUI_DOC_SOURCE_SYNTHETIC && segment.iSourceEnd == UINT64_MAX); continue; }
        CHECK(segment.iSourceStart <= segment.iSourceEnd && segment.iSourceEnd <= bytes);
        if (segment.iKind != XUI_DOC_SOURCE_DIRECT) continue;
        CHECK(segment.iSourceEnd - segment.iSourceStart == segment.iTextEnd - segment.iTextStart);
        for (at = 0; at < segment.iTextEnd - segment.iTextStart; ) {
            uint64_t n = segment.iTextEnd - segment.iTextStart - at; if (n > sizeof(text)) n = sizeof(text);
            CHECK(xuiDocumentSnapshotReadText(snapshot, id, segment.iTextStart + at, text, n) == XUI_OK);
            CHECK(!memcmp(text, source + segment.iSourceStart + at, (size_t)n)); at += n;
        }
    }
    if (info.iSourceSegmentCount) CHECK(previous == info.iTextBytes);
    for (i = 0; i < info.iChildCount; i++) {
        CHECK(xuiDocumentSnapshotGetChild(snapshot, id, i, &child) == XUI_OK);
        if (id == 1) {
            xui_doc_node_info_t block = {0}; block.iSize = sizeof(block);
            CHECK(xuiDocumentSnapshotGetNode(snapshot, child, &block) == XUI_OK);
            CHECK(block.iSyntaxEnd != UINT64_MAX && block.iSyntaxEnd >= previous_block_end && block.iSyntaxEnd <= bytes);
            previous_block_end = block.iSyntaxEnd;
            if (block.iKind != XUI_DOC_FRONT_MATTER && block.iSourceStart != UINT64_MAX) {
                CHECK(block.iSourceStart >= previous_source_start && block.iSourceEnd >= previous_source_end);
                previous_source_start = block.iSourceStart; previous_source_end = block.iSourceEnd;
            }
        }
        source_segments(snapshot, child, source, bytes, count);
    }
}
static void source_index_oracle(xui_document_snapshot snapshot, uint64_t offset, unsigned affinity)
{
    uint64_t indexed_id, scanned_id;
    doc_source_hit indexed = {0}, scanned = {0};
    CHECK(snapshot->state->source_blocks_indexed);
    CHECK(doc_source_find_position(snapshot->state, offset, affinity, 0, &indexed_id, &indexed) == XUI_OK);
    CHECK(doc_source_find_position(snapshot->state, offset, affinity, 1, &scanned_id, &scanned) == XUI_OK);
    CHECK(indexed_id == scanned_id);
    if (indexed_id) CHECK(indexed.offset == scanned.offset && indexed.distance == scanned.distance &&
        indexed.span == scanned.span && indexed.rank == scanned.rank && indexed.mapping == scanned.mapping);
}
static void corpus(const char* path)
{
    FILE* file = fopen(path, "rb"); long length; char* json; xvalue* cases; size_t i, count;
    uint64_t segments = 0, mapped_boundaries = 0, indexed_boundaries = 0, indexed_cases = 0;
    uint64_t spans = 0, definitions = 0, references = 0;
    CHECK(file); CHECK(fseek(file, 0, SEEK_END) == 0); length = ftell(file); CHECK(length >= 0);
    rewind(file); json = malloc((size_t)length + 1); CHECK(json);
    CHECK(fread(json, 1, (size_t)length, file) == (size_t)length); fclose(file); json[length] = 0;
    cases = xrtJsonParse(view(json)); free(json); CHECK(cases && xrtValueIs(cases, XVALUE_ARRAY)); count = xrtValueCount(cases);
    for (i = 0; i < count; i++) {
        xstrview md; xui_doc_desc_t desc = {0}; xui_document d, loaded; xui_document_snapshot s, next;
        char *source, *native; uint64_t bytes, native_bytes; int result;
        CHECK(xrtValueGetString(xrtValueObjectGet(xrtValueArrayGet(cases, i), view("markdown")), &md));
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_COMMONMARK;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        result = xuiDocumentLoadMarkdown(d, md.Data, md.Size);
        if (result != XUI_OK) fprintf(stderr, "Corpus example %llu failed: %d\n", (unsigned long long)i + 1, result);
        CHECK(result == XUI_OK); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
        CHECK(d->state->source_open_brackets == doc_source_open_bracket_count(md.Data, md.Size));
        if (s->state->source_blocks_indexed) indexed_cases++;
        prepare_corpus(&desc, s, md);
        source_segments(s, 1, md.Data, md.Size, &segments);
        source_syntax(s, md.Size, &spans, &definitions, &references);
        {
            uint64_t offset; unsigned affinity;
            for (offset = 0; offset <= md.Size; offset++) {
                if (offset < md.Size && ((unsigned char)md.Data[offset] & 0xc0) == 0x80) continue;
                for (affinity = XUI_DOC_BEFORE; affinity <= XUI_DOC_AFTER; affinity++) {
                    xui_doc_position_t position; int quality, order, reverse_quality; uint64_t reverse;
                    CHECK(xuiDocumentSourceToPositionEx(s, offset, affinity, &position, &quality) == XUI_OK);
                    if (s->state->source_blocks_indexed) {
                        source_index_oracle(s, offset, affinity); indexed_boundaries++;
                    }
                    CHECK(xuiDocumentSnapshotComparePositions(s, &position, &position, &order) == XUI_OK && order == 0);
                    if (quality == XUI_DOC_MAP_EXACT) {
                        CHECK(xuiDocumentPositionToSource(s, &position, &reverse, &reverse_quality) == XUI_OK);
                        CHECK(reverse_quality == XUI_DOC_MAP_EXACT && reverse == offset);
                    }
                    mapped_boundaries++;
                }
            }
        }
        {
            xui_doc_node_info_t root = {0}, node = {0}; uint64_t j, id, end = 0;
            root.iSize = node.iSize = sizeof(root);
            CHECK(xuiDocumentSnapshotGetNode(s, 1, &root) == XUI_OK);
            for (j = 0; j < root.iChildCount; j++) {
                CHECK(xuiDocumentSnapshotGetChild(s, 1, j, &id) == XUI_OK);
                CHECK(xuiDocumentSnapshotGetNode(s, id, &node) == XUI_OK);
                if (!(node.iSyntaxStart >= end && node.iSyntaxStart <= node.iSyntaxEnd && node.iSyntaxEnd <= md.Size))
                    fprintf(stderr, "Syntax example %llu, child %llu, kind %u: %llu..%llu (previous %llu, size %llu)\n",
                        (unsigned long long)i + 1, (unsigned long long)j, node.iKind, (unsigned long long)node.iSyntaxStart,
                        (unsigned long long)node.iSyntaxEnd, (unsigned long long)end, (unsigned long long)md.Size);
                CHECK(node.iSyntaxStart >= end && node.iSyntaxStart <= node.iSyntaxEnd && node.iSyntaxEnd <= md.Size);
                end = node.iSyntaxEnd;
            }
        }
        source = malloc(md.Size + 1); CHECK(source);
        CHECK(xuiDocumentSnapshotCopySource(s, source, md.Size + 1, &bytes) == XUI_OK && bytes == md.Size && !memcmp(source, md.Data, md.Size));
        CHECK(xuiDocumentSerialize(s, &native, &native_bytes) == XUI_OK);
        CHECK(xuiDocumentDeserialize(NULL, native, native_bytes, &loaded) == XUI_OK);
        CHECK(loaded->state->source_open_brackets == doc_source_open_bracket_count(md.Data, md.Size));
        CHECK(xuiDocumentAcquireSnapshot(loaded, &next) == XUI_OK);
        CHECK(xuiDocumentSnapshotCopySource(next, source, md.Size + 1, &bytes) == XUI_OK && bytes == md.Size && !memcmp(source, md.Data, md.Size));
        free(source); xuiDocumentFreeBuffer(native); xuiDocumentSnapshotRelease(next); xuiDocumentRelease(loaded);
        xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    }
    xrtValueRelease(cases);
    printf("CommonMark 0.31.2 corpus: %llu parse/source/native-roundtrip cases passed (not HTML conformance).\n", (unsigned long long)count);
    printf("Source provenance: %llu segments, %llu affinity-aware source positions valid; exact mappings round-trip.\n",
        (unsigned long long)segments, (unsigned long long)mapped_boundaries);
    printf("Ordered source index: %llu corpus cases, %llu positions match full DFS.\n",
        (unsigned long long)indexed_cases, (unsigned long long)indexed_boundaries);
    printf("Source syntax: %llu nested spans, %llu definitions, %llu resolved reference uses valid.\n",
        (unsigned long long)spans, (unsigned long long)definitions, (unsigned long long)references);
    printf("Detached prepare differential: %llu full loads, %llu two-patch edits and %llu continued-input chains match complete parsing, including tree/source metadata.\n",
        (unsigned long long)count, (unsigned long long)count, (unsigned long long)count);
}
static uint64_t add(xui_document_transaction t, uint64_t parent, uint32_t kind, const char* text)
{
    xui_doc_node_desc_t n = {0}; uint64_t id;
    n.iSize = sizeof(n); n.iKind = kind; n.sText = text; n.iTextBytes = text ? strlen(text) : 0;
    CHECK(xuiDocumentTxnInsertNode(t, parent, XUI_DOCUMENT_APPEND, &n, &id) == XUI_OK); return id;
}
static int compare_time(const void* a, const void* b)
{
    double x = *(const double*)a, y = *(const double*)b; return x < y ? -1 : x != y;
}
static void percentiles(const char* name, double* samples, size_t count)
{
    qsort(samples, count, sizeof(*samples), compare_time);
    printf("%s, %llu measured edits after warmup: P50 %.3f ms, P95 %.3f ms, max %.3f ms.\n", name,
        (unsigned long long)count, samples[count / 2], samples[(count * 95 + 99) / 100 - 1], samples[count - 1]);
}
static void scale(void)
{
    xui_document d; xui_document_transaction t; xui_doc_stats_t before = {0}, after = {0};
    uint64_t id = 0, i; clock_t begin; double elapsed, samples[64];
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    begin = clock();
    for (i = 0; i < 10000; i++) { uint64_t p = add(t, 1, XUI_DOC_PARAGRAPH, NULL); id = add(t, p, XUI_DOC_TEXT, "paragraph"); }
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    elapsed = 1000.0 * (clock() - begin) / CLOCKS_PER_SEC;
    before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
    begin = clock(); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(t, id, 9, 9, " edited", 7) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentGetStats(d, &after) == XUI_OK);
    CHECK(after.iAllocations - before.iAllocations < 100 && after.iLiveBytes - before.iLiveBytes < 65536);
    printf("Rich 10000 paragraphs: build %.2f ms, local commit %.2f ms, %llu allocations, %llu retained bytes.\n",
        elapsed, 1000.0 * (clock() - begin) / CLOCKS_PER_SEC, (unsigned long long)(after.iAllocations - before.iAllocations),
        (unsigned long long)(after.iLiveBytes - before.iLiveBytes));
    for (i = 0; i < 68; i++) {
        uint64_t started = xrtClock();
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceText(t, id, 0, 1, i & 1 ? "p" : "P", 1) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        if (i >= 4) samples[i - 4] = (double)(xrtClock() - started) / 1000.0;
    }
    percentiles("Rich 10000 paragraphs", samples, 64);
    CHECK(xuiDocumentGetStats(d, &after) == XUI_OK && after.iHistoryBytes <= after.iHistoryMaxBytes);
    printf("Rich retained memory: %llu current bytes, %llu history bytes, %llu Undo steps.\n",
        (unsigned long long)after.iCurrentBytes, (unsigned long long)after.iHistoryBytes, (unsigned long long)after.iUndoCount);
    xuiDocumentRelease(d);
    {
        const char* block = "## Heading\n\nParagraph with **bold**, [a link](https://example.org/) and ordinary text for a Markdown document.\n\n";
        size_t block_size = strlen(block), count = (100 * 1024 + block_size - 1) / block_size, size = block_size * count;
        char* source = malloc(size + 1); xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t td = {0};
        CHECK(source); for (i = 0; i < count; i++) memcpy(source + i * block_size, block, block_size); source[size] = 0;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        begin = clock(); CHECK(xuiDocumentLoadMarkdown(d, source, size) == XUI_OK);
        elapsed = 1000.0 * (clock() - begin) / CLOCKS_PER_SEC;
        td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
        CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
        begin = clock(); CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceSource(t, 3, 4, "h", 1) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentGetStats(d, &after) == XUI_OK);
        printf("Markdown %llu bytes: load %.2f ms, source edit %.2f ms, %llu allocations.\n", (unsigned long long)size,
            elapsed, 1000.0 * (clock() - begin) / CLOCKS_PER_SEC, (unsigned long long)(after.iAllocations - before.iAllocations));
        {
            xui_document_snapshot snapshot; uint64_t offsets[] = {0, 3, 4, 512, 1000, size / 2, size - 1, size};
            size_t at; unsigned affinity;
            CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
            for (at = 0; at < sizeof(offsets) / sizeof(offsets[0]); at++)
                for (affinity = XUI_DOC_BEFORE; affinity <= XUI_DOC_AFTER; affinity++)
                    source_index_oracle(snapshot, offsets[at], affinity);
            xuiDocumentSnapshotRelease(snapshot);
        }
        for (i = 0; i < 68; i++) {
            uint64_t started = xrtClock();
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
            CHECK(xuiDocumentTxnReplaceSource(t, 3, 4, i & 1 ? "h" : "H", 1) == XUI_OK);
            CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
            if (i >= 4) samples[i - 4] = (double)(xrtClock() - started) / 1000.0;
        }
        percentiles("Markdown 102480 bytes (independent-block source edit)", samples, 64);
        CHECK(xuiDocumentGetStats(d, &after) == XUI_OK && after.iHistoryBytes <= after.iHistoryMaxBytes);
        {
            xui_document_snapshot snapshot;
            CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
            source_index_oracle(snapshot, size / 2, XUI_DOC_BEFORE);
            source_index_oracle(snapshot, size, XUI_DOC_AFTER);
            xuiDocumentSnapshotRelease(snapshot);
        }
        printf("Markdown retained memory: %llu current bytes, %llu history bytes, %llu Undo steps (limit %llu bytes).\n",
            (unsigned long long)after.iCurrentBytes, (unsigned long long)after.iHistoryBytes,
            (unsigned long long)after.iUndoCount, (unsigned long long)after.iHistoryMaxBytes);
        free(source); xuiDocumentRelease(d);
    }
}
typedef struct prepare_scale_job { xui_document_prepare prepare; uint64_t elapsed; } prepare_scale_job;
static int32 prepare_scale_worker(void* data)
{
    prepare_scale_job* job = data; uint64_t start = xrtClock(); int result = xuiDocumentPrepareRun(job->prepare);
    job->elapsed = xrtClock() - start; xuiDocumentPrepareRelease(job->prepare); return result;
}
static int32 prepare_scale_cleanup(void* data)
{
    prepare_scale_job* job = data; uint64_t start = xrtClock();
    xuiDocumentPrepareRelease(job->prepare); job->elapsed = xrtClock() - start; return 0;
}
static void prepare_scale(void)
{
    unsigned megabytes, edit;
    for (megabytes = 1; megabytes <= 10; megabytes += 9) {
        const char* prefix = "## Heading\n\nParagraph with **bold**, &amp; and [link](/target). ";
        char block[513], *source; size_t size = (size_t)megabytes * 1024 * 1024, i;
        xui_doc_desc_t desc = {0}; xui_doc_source_patch_t patch = {0}; xui_doc_stats_t stats = {0}, before = {0}, before_run = {0};
        xui_document d; xui_document_snapshot snapshot; prepare_scale_job job; xthread* thread;
        uint64_t begin, create_us, publish_us, prepare_us, bytes; char preview[32],*copy;
        size = (size + 511) / 512 * 512; source = malloc(size); CHECK(source);
        memset(block, 'a', 512); memcpy(block, prefix, strlen(prefix)); block[510] = block[511] = '\n';
        for (i = 0; i < size; i += 512) memcpy(source + i, block, 512);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(d, "old preview\n", 12) == XUI_OK);
        for (edit = 0; edit < 2; edit++) {
            patch.iSize = sizeof(patch); patch.iStart = edit ? 3 : 0; patch.iEnd = edit ? 4 : 12;
            patch.sText = edit ? "h" : source; patch.iTextBytes = edit ? 1 : size;
            before_run.iSize = sizeof(before_run); CHECK(xuiDocumentGetStats(d, &before_run) == XUI_OK);
            begin = xrtClock(); CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &job.prepare) == XUI_OK); create_us = xrtClock() - begin;
            xuiDocumentPrepareRetain(job.prepare); thread = xrtThreadCreate(prepare_scale_worker, &job, 0); CHECK(thread);
            for (i = 0; i < 64; i++) {
                CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
                CHECK(xuiDocumentSnapshotReadSource(snapshot, 0, preview, 12) == XUI_OK && !memcmp(preview, edit ? prefix : "old preview\n", 12));
                xuiDocumentSnapshotRelease(snapshot);
            }
            CHECK(xrtThreadWaitFor(thread, 30000000) == XWAIT_OK && xrtThreadExitCode(thread) == XUI_OK); xrtThreadDestroy(thread); prepare_us = job.elapsed;
            before.iSize = stats.iSize = sizeof(stats); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
            if (edit) CHECK(before.iPreparedAccountingVisits - before_run.iPreparedAccountingVisits < 8192);
            begin = xrtClock(); CHECK(xuiDocumentPreparePublish(d, job.prepare, NULL) == XUI_OK); publish_us = xrtClock() - begin;
            CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK && stats.iAllocations == before.iAllocations && stats.iPreparedPublishes == before.iPreparedPublishes + 1);
            /* Transfer the owner's remaining handle to a cleanup worker. Its
             * potentially linear final Release is explicitly timed separately. */
            thread = xrtThreadCreate(prepare_scale_cleanup, &job, 0); CHECK(thread);
            CHECK(xrtThreadWaitFor(thread, 30000000) == XWAIT_OK && xrtThreadExitCode(thread) == 0); xrtThreadDestroy(thread);
            if (edit) source[3] = 'h';
            CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK); copy = malloc(size + 1); CHECK(copy);
            CHECK(xuiDocumentSnapshotCopySource(snapshot, copy, size + 1, &bytes) == XUI_OK && bytes == size && !memcmp(copy, source, size));
            if (edit) {
                unsigned affinity; uint64_t offsets[] = {0, 3, size / 2, size - 1, size}; size_t at;
                for (at = 0; at < sizeof(offsets) / sizeof(offsets[0]); at++)
                    for (affinity = XUI_DOC_BEFORE; affinity <= XUI_DOC_AFTER; affinity++)
                        source_index_oracle(snapshot, offsets[at], affinity);
                if (megabytes == 10) {
                    uint64_t started, indexed_us, full_us, id; doc_source_hit hit;
                    started = xrtClock();
                    for (i = 0; i < 256; i++)
                        CHECK(doc_source_find_position(snapshot->state, size / 2, XUI_DOC_AFTER, 0, &id, &hit) == XUI_OK);
                    indexed_us = xrtClock() - started;
                    started = xrtClock();
                    CHECK(doc_source_find_position(snapshot->state, size / 2, XUI_DOC_AFTER, 1, &id, &hit) == XUI_OK);
                    full_us = xrtClock() - started;
                    printf("Markdown 10 MiB source position: indexed %.3f us/call (256), full DFS %.3f ms/call.\n",
                        (double)indexed_us / 256, (double)full_us / 1000);
                }
            }
            xuiDocumentSnapshotRelease(snapshot); free(copy);
            printf("Markdown prepare %u MiB %s: owner create %.3f ms, worker prepare %.3f ms, owner publish %.3f ms, worker cleanup %.3f ms; %llu accounting visits, %llu peak bytes, %llu storage updates, %llu nodes.\n",
                megabytes, edit ? "local edit" : "load", (double)create_us / 1000, (double)prepare_us / 1000, (double)publish_us / 1000,
                (double)job.elapsed / 1000, (unsigned long long)(before.iPreparedAccountingVisits - before_run.iPreparedAccountingVisits),
                (unsigned long long)stats.iPeakBytes,
                (unsigned long long)(stats.iPreparedStorageUpdates - before.iPreparedStorageUpdates), (unsigned long long)stats.iNodes);
        }
        free(source); xuiDocumentRelease(d);
    }
}
static void incremental_scale(void)
{
    unsigned mb, position, edit;
    for (mb = 1; mb <= 10; mb += 9) {
        const char* prefix = "## Heading\n\nParagraph with **bold**, &amp; and [link](/target). ";
        char block[512], *source, *expected, *actual; uint64_t size = (uint64_t)mb * 1024 * 1024, i;
        xui_document d; xui_doc_desc_t desc = {0};
        source = malloc((size_t)size + 1); expected = malloc((size_t)size + 2); actual = malloc((size_t)size + 2);
        CHECK(source && expected && actual); memset(block, 'a', sizeof(block)); memcpy(block, prefix, strlen(prefix)); block[510] = block[511] = '\n';
        for (i = 0; i < size; i += 512) memcpy(source + i, block, 512);
        source[size] = 0;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iHistoryMaxBytes = UINT64_C(256) * 1024 * 1024;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, size) == XUI_OK);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK);
        for (position = 0; position < 3; position++) for (edit = 0; edit < 3; edit++) {
            uint64_t at = (position == 0 ? 0 : position == 1 ? size / 2 : size - 512) + 20;
            uint64_t removed = edit == 1 ? 0 : 1, added = edit == 2 ? 0 : 1, bytes, started, published, prepared;
            xui_doc_source_patch_t patch = {0}; xui_doc_stats_t before = {0}, after = {0};
            xui_document_snapshot snapshot; prepare_scale_job job; xthread* thread;
            before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
            patch.iSize = sizeof(patch); patch.iStart = at; patch.iEnd = at + removed; patch.sText = added ? "Q" : ""; patch.iTextBytes = added;
            memcpy(expected, source, (size_t)at); memcpy(expected + at, patch.sText, (size_t)added);
            memcpy(expected + at + added, source + at + removed, (size_t)(size - at - removed + 1));
            CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &job.prepare) == XUI_OK);
            xuiDocumentPrepareRetain(job.prepare); thread = xrtThreadCreate(prepare_scale_worker, &job, 0); CHECK(thread);
            CHECK(xrtThreadWaitFor(thread, 30000000) == XWAIT_OK && xrtThreadExitCode(thread) == XUI_OK); xrtThreadDestroy(thread); prepared = job.elapsed;
            started = xrtClock(); CHECK(xuiDocumentPreparePublish(d, job.prepare, NULL) == XUI_OK); published = xrtClock() - started;
            thread = xrtThreadCreate(prepare_scale_cleanup, &job, 0); CHECK(thread);
            CHECK(xrtThreadWaitFor(thread, 30000000) == XWAIT_OK && xrtThreadExitCode(thread) == 0); xrtThreadDestroy(thread);
            CHECK(xuiDocumentGetStats(d, &after) == XUI_OK && after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1);
            CHECK(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes <= 512 && after.iUndoCount == 1);
            if (mb == 10 && position == 0 && edit == 1) CHECK(after.iHistoryBytes < 256 * 1024);
            if (mb == 10) CHECK(after.iPreparedAccountingVisits - before.iPreparedAccountingVisits < 8192);
            CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
            CHECK(xuiDocumentSnapshotCopySource(snapshot, actual, size + 2, &bytes) == XUI_OK && bytes == size - removed + added && !strcmp(actual, expected));
            xuiDocumentSnapshotRelease(snapshot);
            printf("Markdown incremental %u MiB position %u %s: %llu parsed bytes; worker %.3f ms, publish %.3f ms, cleanup %.3f ms, %llu history bytes, %llu accounting visits, %llu storage updates.\n",
                mb, position, edit == 0 ? "replace" : edit == 1 ? "insert" : "delete",
                (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes), (double)prepared / 1000,
                (double)published / 1000, (double)job.elapsed / 1000, (unsigned long long)after.iHistoryBytes,
                (unsigned long long)(after.iPreparedAccountingVisits - before.iPreparedAccountingVisits),
                (unsigned long long)(after.iPreparedStorageUpdates - before.iPreparedStorageUpdates));
            CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
            CHECK(xuiDocumentSnapshotCopySource(snapshot, actual, size + 2, &bytes) == XUI_OK && bytes == size && !strcmp(actual, source));
            xuiDocumentSnapshotRelease(snapshot); CHECK(xuiDocumentClearHistory(d) == XUI_OK);
        }
        xuiDocumentRelease(d); free(source); free(expected); free(actual);
    }
}
static void no_history_accounting_scale(void)
{
    const char* prefix = "## Heading\n\nParagraph with **bold**, &amp; and [link](/target). ";
    size_t size = 1024 * 1024, i; char block[512], *source = malloc(size + 1), value;
    xui_doc_desc_t desc = {0}; xui_doc_source_patch_t patch = {0}; xui_doc_stats_t before = {0}, after = {0};
    xui_document d; xui_document_snapshot snapshot; prepare_scale_job job; xthread* thread;
    uint64_t started, elapsed;
    CHECK(source); memset(block, 'a', sizeof(block)); memcpy(block, prefix, strlen(prefix)); block[510] = block[511] = '\n';
    for (i = 0; i < size; i += 512) memcpy(source + i, block, 512);
    source[size] = 0;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.bDisableHistory = 1;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, size) == XUI_OK);
    before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
    patch.iSize = sizeof(patch); patch.iStart = 20; patch.iEnd = 21; patch.sText = "Q"; patch.iTextBytes = 1;
    CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &job.prepare) == XUI_OK);
    xuiDocumentPrepareRetain(job.prepare); started = xrtClock();
    thread = xrtThreadCreate(prepare_scale_worker, &job, 0); CHECK(thread);
    CHECK(xrtThreadWaitFor(thread, 30000000) == XWAIT_OK && xrtThreadExitCode(thread) == XUI_OK); xrtThreadDestroy(thread);
    elapsed = xrtClock() - started;
    CHECK(xuiDocumentPreparePublish(d, job.prepare, NULL) == XUI_OK); xuiDocumentPrepareRelease(job.prepare);
    CHECK(xuiDocumentGetStats(d, &after) == XUI_OK && after.iUndoCount == 0 && after.iHistoryBytes == 0);
    CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1);
    CHECK(after.iPreparedAccountingVisits - before.iPreparedAccountingVisits < 8192);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotReadSource(snapshot, 20, &value, 1) == XUI_OK && value == 'Q');
    xuiDocumentSnapshotRelease(snapshot);
    printf("Markdown no-history 1 MiB local edit: %llu accounting visits, %llu storage updates, %.3f ms worker including dispatch.\n",
        (unsigned long long)(after.iPreparedAccountingVisits - before.iPreparedAccountingVisits),
        (unsigned long long)(after.iPreparedStorageUpdates - before.iPreparedStorageUpdates), (double)elapsed / 1000);
    xuiDocumentRelease(d); free(source);
}
static void indexed_locator_scale(void)
{
    const char* prefix = "## Heading\n\nParagraph with **bold**, &amp; and [link](/target). ";
    size_t size = 10 * 1024 * 1024, i; char block[512], *source = malloc(size + 1);
    xui_doc_desc_t desc = {0}; xui_document d; unsigned position;
    CHECK(source); memset(block, 'a', sizeof(block)); memcpy(block, prefix, strlen(prefix)); block[510] = block[511] = '\n';
    for (i = 0; i < size; i += sizeof(block)) memcpy(source + i, block, sizeof(block));
    source[size] = 0;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.bDisableHistory = 1;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, size) == XUI_OK);
    for (position = 0; position < 3; position++) {
        uint64_t at = (position == 0 ? 0 : position == 1 ? size / 2 : size - 512) + 20;
        xui_doc_source_patch_t patch = {0}; xui_doc_stats_t before = {0}, after = {0};
        xui_document_snapshot snapshot; prepare_scale_job job; xthread* thread; char value;
        before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
        patch.iSize = sizeof(patch); patch.iStart = at; patch.iEnd = at + 1; patch.sText = "Q"; patch.iTextBytes = 1;
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &job.prepare) == XUI_OK);
        xuiDocumentPrepareRetain(job.prepare); thread = xrtThreadCreate(prepare_scale_worker, &job, 0); CHECK(thread);
        CHECK(xrtThreadWaitFor(thread, 30000000) == XWAIT_OK && xrtThreadExitCode(thread) == XUI_OK); xrtThreadDestroy(thread);
        CHECK(xuiDocumentPreparePublish(d, job.prepare, NULL) == XUI_OK); xuiDocumentPrepareRelease(job.prepare);
        CHECK(xuiDocumentGetStats(d, &after) == XUI_OK && after.iUndoCount == 0 && after.iHistoryBytes == 0);
        CHECK(after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1);
        CHECK(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes <= 512);
        CHECK(after.iPreparedAccountingVisits - before.iPreparedAccountingVisits < 8192);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotReadSource(snapshot, at, &value, 1) == XUI_OK && value == 'Q');
        xuiDocumentSnapshotRelease(snapshot);
        printf("Markdown indexed locator 10 MiB position %u: %.3f ms worker, %llu parsed bytes, %llu accounting visits.\n",
            position, (double)job.elapsed / 1000,
            (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes),
            (unsigned long long)(after.iPreparedAccountingVisits - before.iPreparedAccountingVisits));
    }
    xuiDocumentRelease(d); free(source);
}
static void reference_budget_scale(void)
{
    const size_t size = 10 * 1024 * 1024, target = 5 * 1024 * 1024;
    const size_t link = target + sizeof("paragraph ") - 1;
    char block[512], *source = malloc(size + 1), value;
    xui_doc_desc_t desc = {0}; xui_document d; unsigned phase;
    CHECK(source); memset(block, 'a', sizeof(block)); block[510] = block[511] = '\n';
    for (size_t at = 0; at < size; at += sizeof(block)) memcpy(source + at, block, sizeof(block));
    memcpy(source, "[ref]: /target\n\n", sizeof("[ref]: /target\n\n") - 1);
    memcpy(source + target, "paragraph [ref] tail", sizeof("paragraph [ref] tail") - 1);
    source[size] = 0;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, size) == XUI_OK);
    CHECK(d->state->source_open_brackets == 2 && d->state->source_blocks_indexed &&
        xuiDocumentClearHistory(d) == XUI_OK);
    for (phase = 0; phase < 3; phase++) {
        xui_doc_source_patch_t patch = {0}; xui_doc_stats_t before = {0}, after = {0};
        prepare_scale_job job; xthread* thread;
        patch.iSize = sizeof(patch); patch.iStart = phase ? link : target + 1;
        patch.iEnd = patch.iStart + 1;
        patch.sText = phase == 0 ? "P" : phase == 1 ? "X" : "["; patch.iTextBytes = 1;
        before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &job.prepare) == XUI_OK);
        xuiDocumentPrepareRetain(job.prepare); thread = xrtThreadCreate(prepare_scale_worker, &job, 0); CHECK(thread);
        CHECK(xrtThreadWaitFor(thread, 30000000) == XWAIT_OK && xrtThreadExitCode(thread) == XUI_OK); xrtThreadDestroy(thread);
        CHECK(xuiDocumentPreparePublish(d, job.prepare, NULL) == XUI_OK); xuiDocumentPrepareRelease(job.prepare);
        CHECK(xuiDocumentGetStats(d, &after) == XUI_OK && after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1);
        CHECK(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 1024);
        CHECK(d->state->source_open_brackets == (phase == 1 ? 1 : 2) &&
            d->state->source_blocks_indexed);
        printf("Markdown reference budget 10 MiB phase %u: worker %.3f ms, %llu parsed bytes, %llu cached brackets.\n",
            phase, (double)job.elapsed / 1000,
            (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes),
            (unsigned long long)d->state->source_open_brackets);
    }
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && d->state->source_open_brackets == 1);
    CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && d->state->source_open_brackets == 2);
    {
        xui_document_snapshot snapshot;
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotReadSource(snapshot, target + 1, &value, 1) == XUI_OK && value == 'P');
        CHECK(xuiDocumentSnapshotReadSource(snapshot, link, &value, 1) == XUI_OK && value == '[');
        xuiDocumentSnapshotRelease(snapshot);
    }
    {
        static const char longer[] = "/a-much-longer-path";
        xui_doc_source_patch_t patch = {0}; xui_doc_stats_t before = {0}, after = {0};
        prepare_scale_job job; xthread* thread;
        patch.iSize = sizeof(patch); patch.iStart = sizeof("[ref]: ") - 1;
        patch.iEnd = patch.iStart + sizeof("/target") - 1;
        patch.sText = longer; patch.iTextBytes = sizeof(longer) - 1;
        before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &job.prepare) == XUI_OK);
        xuiDocumentPrepareRetain(job.prepare); thread = xrtThreadCreate(prepare_scale_worker, &job, 0); CHECK(thread);
        CHECK(xrtThreadWaitFor(thread, 30000000) == XWAIT_OK && xrtThreadExitCode(thread) == XUI_OK); xrtThreadDestroy(thread);
        CHECK(xuiDocumentPreparePublish(d, job.prepare, NULL) == XUI_OK); xuiDocumentPrepareRelease(job.prepare);
        CHECK(xuiDocumentGetStats(d, &after) == XUI_OK && after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1);
        CHECK(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 2048 &&
            d->state->source_open_brackets == 2 && d->state->source_blocks_indexed);
        {
            xui_document_snapshot snapshot; xui_doc_position_t position;
            xui_doc_node_info_t info = {0}; int quality;
            uint64_t moved_link = link + patch.iTextBytes - (patch.iEnd - patch.iStart);
            CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
            CHECK(xuiDocumentSourceToPositionEx(snapshot, moved_link + 1,
                XUI_DOC_AFTER, &position, &quality) == XUI_OK);
            info.iSize = sizeof(info);
            CHECK(xuiDocumentSnapshotGetNode(snapshot, position.iNodeId, &info) == XUI_OK &&
                (info.tAttributes.iMarks & XUI_DOC_LINK) && !strcmp(info.sResource, longer));
            xuiDocumentSnapshotRelease(snapshot);
        }
        printf("Markdown definition length 10 MiB: worker %.3f ms, %llu parsed bytes, %llu cached brackets.\n",
            (double)job.elapsed / 1000,
            (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes),
            (unsigned long long)d->state->source_open_brackets);
    }
    {
        xui_doc_source_patch_t patch = {0}; xui_doc_stats_t before = {0}, after = {0};
        prepare_scale_job job; xthread* thread;
        patch.iSize = sizeof(patch); patch.iStart = sizeof("[ref]: ") - 1;
        patch.iEnd = patch.iStart + sizeof("/a-much-longer-path") - 1;
        patch.sText = "/target"; patch.iTextBytes = sizeof("/target") - 1;
        before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &job.prepare) == XUI_OK);
        xuiDocumentPrepareRetain(job.prepare); thread = xrtThreadCreate(prepare_scale_worker, &job, 0); CHECK(thread);
        CHECK(xrtThreadWaitFor(thread, 30000000) == XWAIT_OK && xrtThreadExitCode(thread) == XUI_OK); xrtThreadDestroy(thread);
        CHECK(xuiDocumentPreparePublish(d, job.prepare, NULL) == XUI_OK); xuiDocumentPrepareRelease(job.prepare);
        CHECK(xuiDocumentGetStats(d, &after) == XUI_OK &&
            after.iMarkdownIncrementalParses == before.iMarkdownIncrementalParses + 1 &&
            after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 2048 &&
            d->state->source_open_brackets == 2 && d->state->source_blocks_indexed);
        {
            xui_document_snapshot snapshot; xui_doc_position_t position;
            xui_doc_node_info_t info = {0}; int quality;
            CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
            CHECK(xuiDocumentSourceToPositionEx(snapshot, link + 1,
                XUI_DOC_AFTER, &position, &quality) == XUI_OK);
            info.iSize = sizeof(info);
            CHECK(xuiDocumentSnapshotGetNode(snapshot, position.iNodeId, &info) == XUI_OK &&
                (info.tAttributes.iMarks & XUI_DOC_LINK) && !strcmp(info.sResource, "/target"));
            xuiDocumentSnapshotRelease(snapshot);
        }
        printf("Markdown definition restore 10 MiB: worker %.3f ms, %llu parsed bytes, indexed.\n",
            (double)job.elapsed / 1000,
            (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes));
    }
    xuiDocumentRelease(d); free(source);
}
typedef struct dependency_growth_allocator {
    size_t request;
    unsigned live, matches;
    int armed, fail, cancel;
    xui_document document;
    xui_document_prepare pending;
    uint64_t parsed_before, first_parsed_delta;
} dependency_growth_allocator;
static void* dependency_growth_alloc(void* user, size_t bytes)
{
    dependency_growth_allocator* a = user;
    void* pointer;
    if (a->armed && bytes == a->request) {
        a->matches++;
        if (a->matches == 1) {
            a->first_parsed_delta = atomic_load(&a->document->state->allocator->markdown_parsed_bytes) - a->parsed_before;
            if (a->fail) return NULL;
            if (a->cancel && a->pending) xuiDocumentPrepareCancel(a->pending);
        }
    }
    pointer = malloc(bytes);
    if (pointer) a->live++;
    return pointer;
}
static void dependency_growth_free(void* user, void* pointer)
{
    dependency_growth_allocator* a = user;
    CHECK(a->live); a->live--; free(pointer);
}
static void dependency_growth_failure(void)
{
    unsigned fixture, attempt;
    for (fixture = 0; fixture < 2; fixture++) {
        char source[2048]; size_t used = 0;
        uint64_t edit_at = fixture ? 1 : 7, removed = fixture ? 3 : 6;
        const char* replacement = fixture ? "new" : "/longer";
        used += (size_t)snprintf(source + used, sizeof(source) - used,
            "%s", fixture ? "[old]: /target\n\n" : "[ref]: /short\n\n");
        for (unsigned block = 0; block < 33; block++)
            used += (size_t)snprintf(source + used, sizeof(source) - used,
                "block %u [%s]\n\n", block, fixture ? "old" : "ref");
        CHECK(used < sizeof(source));
        for (attempt = 0; attempt < 2; attempt++) {
            dependency_growth_allocator allocator = {0};
            xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t tx = {0};
            xui_document d; xui_document_transaction t;
            xui_document_snapshot before, after; uint64_t revision;
            int result;
            allocator.request = sizeof(doc_allocation) + 64 * sizeof(uint64_t);
            allocator.fail = attempt == 1;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.bDisableHistory = 1; desc.onAlloc = dependency_growth_alloc;
            desc.onFree = dependency_growth_free; desc.pAllocatorUser = &allocator;
            tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, source, used) == XUI_OK);
            allocator.document = d;
            allocator.parsed_before = atomic_load(&d->state->allocator->markdown_parsed_bytes);
            revision = xuiDocumentGetRevision(d);
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
            allocator.armed = 1;
            result = xuiDocumentTxnReplaceSource(t, edit_at, edit_at + removed,
                replacement, strlen(replacement));
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            allocator.armed = 0; xuiDocumentTxnRelease(t);
            CHECK(allocator.matches >= 1 &&
                allocator.first_parsed_delta == (fixture ? 15 : 0));
            if (!allocator.fail) {
                CHECK(result == XUI_OK && xuiDocumentGetRevision(d) == revision + 1);
            } else {
                char actual[2048]; uint64_t copied;
                CHECK(allocator.matches == 1 && result == XUI_ERROR_OUT_OF_MEMORY &&
                    xuiDocumentGetRevision(d) == revision);
                CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
                prepare_snapshot_equal(before, after);
                CHECK(xuiDocumentSnapshotCopySource(after, actual, sizeof(actual), &copied) == XUI_OK &&
                    copied == used && !memcmp(actual, source, used + 1));
                xuiDocumentSnapshotRelease(after);
            }
            xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d);
            CHECK(!allocator.live);
        }
        {
            dependency_growth_allocator allocator = {0};
            xui_doc_desc_t desc = {0}; xui_doc_source_patch_t patch = {0};
            xui_document d; xui_document_prepare pending;
            xui_document_snapshot before, after; uint64_t revision;
            allocator.request = sizeof(doc_allocation) + 64 * sizeof(uint64_t);
            allocator.cancel = 1;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.bDisableHistory = 1; desc.onAlloc = dependency_growth_alloc;
            desc.onFree = dependency_growth_free; desc.pAllocatorUser = &allocator;
            patch.iSize = sizeof(patch); patch.iStart = edit_at;
            patch.iEnd = edit_at + removed; patch.sText = replacement;
            patch.iTextBytes = strlen(replacement);
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, source, used) == XUI_OK);
            allocator.document = d;
            allocator.parsed_before = atomic_load(&d->state->allocator->markdown_parsed_bytes);
            revision = xuiDocumentGetRevision(d);
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &pending) == XUI_OK);
            allocator.pending = pending; allocator.armed = 1;
            CHECK(xuiDocumentPrepareRun(pending) == XUI_DOC_ERROR_CANCELLED);
            allocator.armed = 0; allocator.pending = NULL;
            CHECK(allocator.matches == 1 &&
                allocator.first_parsed_delta == (fixture ? 15 : 0) &&
                xuiDocumentPreparePublish(d, pending, NULL) == XUI_DOC_ERROR_CANCELLED &&
                xuiDocumentGetRevision(d) == revision);
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            prepare_snapshot_equal(before, after);
            xuiDocumentSnapshotRelease(after); xuiDocumentSnapshotRelease(before);
            xuiDocumentPrepareRelease(pending); xuiDocumentRelease(d);
            CHECK(!allocator.live);
        }
    }
    puts("Markdown growing definition dependency sets: 33-block value/label allocation failure and cancellation are atomic and leak-free.");
}
int main(int argc, char** argv)
{
    if (argc > 1 && !strcmp(argv[1], "--growth-only")) {
        dependency_growth_failure(); return 0;
    }
    if (argc > 1) corpus(argv[1]);
    scale(); prepare_scale(); incremental_scale(); no_history_accounting_scale(); indexed_locator_scale();
    reference_budget_scale(); dependency_growth_failure(); return 0;
}
