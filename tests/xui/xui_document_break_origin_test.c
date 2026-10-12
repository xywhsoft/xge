#define main xui_document_core_uninvoked
#include "xui_document_test.c"
#undef main

typedef struct origin_case {
    const char *first, *second;
    unsigned kind, spaces, task;
} origin_case;
static const origin_case origin_cases[] = {
    {"*word* ", "next", XUI_DOC_BREAK_SOFT, 1, 0},
    {"`word`", "next", XUI_DOC_BREAK_SOFT, 0, 0},
    {"_word_\\", "next", XUI_DOC_BREAK_HARD_BACKSLASH, 0, 0},
    {"- [ ] ", "body", XUI_DOC_BREAK_SOFT, 1, 1},
    {"> - [ ] ", ">   body", XUI_DOC_BREAK_SOFT, 1, 1},
    {"*word*  ", "next", XUI_DOC_BREAK_HARD_SPACES, 2, 0}
};
static void origin_golden(xui_document_snapshot s, unsigned fixture, unsigned dialect,
    uint64_t newline, uint64_t ending)
{
    const origin_case* c = &origin_cases[fixture]; xui_doc_node_info_t info = {0};
    uint64_t node = find_kind(s, 1, c->kind == XUI_DOC_BREAK_SOFT ? XUI_DOC_SOFT_BREAK : XUI_DOC_HARD_BREAK);
    uint64_t trailing = newline - (c->task && dialect != XUI_MD_COMMONMARK ? 0 : c->spaces);
    uint64_t marker_start = UINT64_MAX, marker_end = UINT64_MAX, start = trailing;
    CHECK(node); info.iSize = sizeof(info);
    if (c->kind == XUI_DOC_BREAK_HARD_BACKSLASH) { marker_start = newline - 1; marker_end = newline; start = marker_start; }
    if (c->kind == XUI_DOC_BREAK_HARD_SPACES) { marker_start = trailing; marker_end = newline; }
    expect_break_syntax(s, node, c->kind, trailing, newline, newline + ending, marker_start, marker_end);
    CHECK(xuiDocumentSnapshotGetNode(s, node, &info) == XUI_OK &&
        info.iSourceStart == start && info.iSourceEnd == newline + ending && !info.bSourceExact);
}
#include "xui_document_break_origin_boundary_cases.h"
int main(void)
{
    static const char* endings[] = {"\n", "\r\n", "\r"};
    unsigned fixture, dialect, eol, native, mode, cases = 0;
    for (fixture = 0; fixture < sizeof(origin_cases) / sizeof(*origin_cases); fixture++)
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++)
    for (eol = 0; eol < 3; eol++) for (native = 0; native < 2; native++) for (mode = 0; mode < 4; mode++) {
        const char* prefix = native ? "\xef\xbb\xbf---\nname: origin\n---\n\nprior `span`\n\n" : "prior `span`\n\n";
        char source[1024], initial[1024]; size_t at = strlen(prefix) + strlen(origin_cases[fixture].first), ending = strlen(endings[eol]);
        xui_doc_desc_t desc = {0}; xui_document d, oracle; xui_document_snapshot before, after, expected, restored;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        snprintf(source, sizeof(source), "%s%s%s%s%s", prefix, origin_cases[fixture].first, endings[eol], origin_cases[fixture].second, endings[eol]);
        memcpy(initial, source, at); strcpy(initial + at, source + at + ending);
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, mode ? initial : source,
            strlen(mode ? initial : source)) == XUI_OK);
        if (native) {
            char* wire; uint64_t bytes; xui_document loaded;
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSerialize(before, &wire, &bytes) == XUI_OK &&
                xuiDocumentDeserialize(NULL, wire, bytes, &loaded) == XUI_OK);
            xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); d = loaded;
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        if (mode == 1) {
            xui_doc_txn_desc_t td = {0}; xui_document_transaction t; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK &&
                xuiDocumentTxnReplaceSource(t, at, at, endings[eol], ending) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
            xuiDocumentTxnRelease(t);
        } else if (mode >= 2) {
            xui_doc_source_patch_t patch = prepare_patch(at, at, endings[eol]); xui_document_prepare p, next;
            CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_OK);
            if (mode == 3) {
                patch = prepare_patch(strlen(source), strlen(source), "\nfinal\n");
                CHECK(xuiDocumentPrepareContinueSource(d, p, &patch, 1, &next) == XUI_OK); xuiDocumentPrepareRelease(p); p = next;
                strcat(source, "\nfinal\n"); CHECK(xuiDocumentPrepareRun(p) == XUI_OK);
            }
            CHECK(xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); xuiDocumentPrepareRelease(p);
        }
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); origin_golden(after, fixture, dialect, at, ending);
        test_source(d, source, 1);
        CHECK(xuiDocumentCreate(&desc, &oracle) == XUI_OK && xuiDocumentLoadMarkdown(oracle, source, strlen(source)) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK); inc_snapshot_equal(after, expected);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
        if (mode) {
            CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            inc_snapshot_equal(before, restored); xuiDocumentSnapshotRelease(restored);
            CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            inc_snapshot_equal(after, restored); xuiDocumentSnapshotRelease(restored);
        }
        xuiDocumentRelease(d); origin_golden(after, fixture, dialect, at, ending);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); cases++;
    }
    printf("Break origin golden: %u exact raw ranges after marks and empty task lines; three dialects, LF/CRLF/CR, native reload, SOURCE/Prepare/Continue, full oracle, retained snapshots and Undo passed\n", cases);
    origin_boundary_differential();
    return 0;
}
