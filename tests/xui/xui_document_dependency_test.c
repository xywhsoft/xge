#define main xui_document_core_uninvoked
#include "xui_document_test.c"
#undef main

typedef struct dependency_case {
    const char *source, *needle, *replacement;
} dependency_case;
static const dependency_case dependency_cases[] = {
    {"use [^a]\n\n[^a]: alpha [^b] and [r]\n\n[^b]: beta [^c]\n\n[^c]: gamma\n\n[r]: /before\n",
     "beta", "BETA longer"},
    {"use [^a]\n\n[^a]: alpha [^b] and [r]\n\n[^b]: beta [^a] and [r]\n\n[r]: /before\n",
     "/before", "/after-longer"},
    {"use [^a] then [^b]\n\n[^a]: alpha [^b]\n\n[^b]: beta\n",
     "use [^a] then [^b]", "use [^b] then [^a]"},
    {"use [^a]\n\n[^a]: alpha [^b]\n\n[^b]: beta\n",
     "[^b]:", "[^c]:"},
    {"use [r]\n\n> [r]: /before\n>\n> text [r]\n\n[r]: /duplicate\n",
     "/before", "/after-longer"},
    {"use [r] and [^a]\n\n- [r]: /before\n\n- text [r]\n\n[^a]: alpha [r] [^b]\n\n[^b]: beta\n",
     "/before", "/after-longer"},
    {"use [^later]\n\n[r]: /before\n",
     "[r]: /before", "[^later]: activated [^a]\n\n[^a]: alpha"},
    {"use [^a] then [^b]\n\n[^a]: alpha [^b]\n\n[^b]: beta [^a]\n",
     "use [^a] then [^b]", "use [^b]"},
    {"use [^a]\n\n> [^a]: alpha [^b]\n>\n> quoted body\n\n[^b]: beta\n",
     "beta", "BETA longer"},
    {"use [^new]\n\nmarker\n",
     "marker", "[^new]: first definition [^next]\n\n[^next]: transitive body"},
    {"use [r]\n\nmarker\n",
     "marker", "[r]: /first-definition \"first title\""},
};
static void dependency_snapshot(xui_document_snapshot actual, const char* text)
{
    xui_document oracle = test_markdown_open(text); xui_document_snapshot expected;
    char* copied = malloc(strlen(text) + 1); uint64_t bytes; CHECK(copied);
    CHECK(xuiDocumentSnapshotCopySource(actual, copied, strlen(text) + 1, &bytes) == XUI_OK &&
        bytes == strlen(text) && !memcmp(copied, text, (size_t)bytes + 1));
    CHECK(xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK); inc_snapshot_equal(actual, expected);
    free(copied); xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
}
static char* dependency_lines(const char* source, unsigned ending)
{
    const char* newline = ending == 0 ? "\n" : ending == 1 ? "\r\n" : "\r";
    size_t i, used = 0; char* text = malloc(strlen(source) * 2 + 1); CHECK(text);
    for (i = 0; source[i]; i++) {
        if (source[i] == '\n') { memcpy(text + used, newline, strlen(newline)); used += strlen(newline); }
        else text[used++] = source[i];
    }
    text[used] = 0; return text;
}
static char* dependency_source(unsigned fixture, unsigned paragraphs, unsigned header)
{
    char* source = malloc(65536); size_t used = 0; unsigned p; CHECK(source);
    source[0] = 0;
    if (header) { strcpy(source, "\xef\xbb\xbf---\nname: cohort\n---\n\n"); used = strlen(source); }
    for (p = 0; p < paragraphs; p++) {
        int bytes = snprintf(source + used, 65536 - used,
            p % 3 ? "untouched %u *emphasis* &amp; **strong** `code`\n\n" : "## untouched heading %u\n\n", p);
        CHECK(bytes > 0 && (size_t)bytes < 65536 - used); used += (size_t)bytes;
    }
    CHECK(used + strlen(dependency_cases[fixture].source) + 64 < 65536);
    strcpy(source + used, dependency_cases[fixture].source);
    strcat(source, "\nuntouched suffix *one*\n\n## untouched suffix two\n");
    return source;
}
static void dependency_matrix(void)
{
    unsigned fixture, mode, ending, header, cases = 0;
    for (fixture = 0; fixture < sizeof(dependency_cases) / sizeof(dependency_cases[0]); fixture++)
    for (ending = 0; ending < 3; ending++) for (header = 0; header < 2; header++) for (mode = 0; mode < 4; mode++) {
        char *plain = dependency_source(fixture, 32, header), *source = dependency_lines(plain, ending);
        char *replacement = dependency_lines(dependency_cases[fixture].replacement, ending);
        const char* extra = ending == 0 ? "\n[extra]: /unused\n" : ending == 1 ? "\r\n[extra]: /unused\r\n" : "\r[extra]: /unused\r";
        char* edited = malloc(strlen(source) + strlen(replacement) + strlen(extra) + 1);
        char* found = strstr(source, dependency_cases[fixture].needle);
        size_t at, removed = strlen(dependency_cases[fixture].needle), added = strlen(replacement), length;
        xui_document d = test_markdown_open(source); xui_document_snapshot before, after, restored;
        xui_doc_stats_t stats; xui_doc_source_patch_t patches[2]; unsigned i;
        CHECK(edited && found); at = (size_t)(found - source);
        memcpy(edited, source, at); memcpy(edited + at, replacement, added);
        strcpy(edited + at + added, source + at + removed); length = strlen(edited);
        if (mode >= 2) strcat(edited, extra);
        if (header) {
            char* wire; uint64_t bytes; xui_document loaded;
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSerialize(before, &wire, &bytes) == XUI_OK &&
                xuiDocumentDeserialize(NULL, wire, bytes, &loaded) == XUI_OK);
            xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); d = loaded;
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentMarkSaved(d, before) == XUI_OK); stats = inc_stats(d);
        patches[0] = prepare_patch(at, at + removed, replacement);
        patches[1] = prepare_patch(length, length, extra);
        if (!mode) {
            xui_doc_txn_desc_t desc = {0}; xui_document_transaction t;
            desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE;
            CHECK(xuiDocumentBeginTransaction(d, &desc, &t) == XUI_OK &&
                xuiDocumentTxnReplaceSource(t, at, at + removed, replacement, added) == XUI_OK &&
                xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        } else {
            xui_document_prepare p, next;
            CHECK(xuiDocumentPrepareSource(d, NULL, patches, 1, &p) == XUI_OK);
            if (mode >= 2) {
                if (mode == 3) CHECK(xuiDocumentPrepareRun(p) == XUI_OK);
                CHECK(xuiDocumentPrepareContinueSource(d, p, patches + 1, 1, &next) == XUI_OK);
                xuiDocumentPrepareRelease(p); p = next;
            }
            CHECK(xuiDocumentPrepareRun(p) == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
            xuiDocumentPrepareRelease(p);
        }
        CHECK(inc_stats(d).iMarkdownIncrementalParses > stats.iMarkdownIncrementalParses);
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); dependency_snapshot(after, edited);
        for (i = 0; i < 32; i++) {
            uint64_t old_id, new_id;
            CHECK(xuiDocumentSnapshotGetChild(before, 1, i, &old_id) == XUI_OK &&
                xuiDocumentSnapshotGetChild(after, 1, i, &new_id) == XUI_OK && old_id == new_id);
            expect_identity_tree(before, after, old_id);
        }
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d) &&
            xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(before, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentIsDirty(d) &&
            xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(after, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK);
#ifndef XUI_DLL
        memory_oracle_check(d);
#endif
        xuiDocumentRelease(d); dependency_snapshot(before, source); dependency_snapshot(after, edited);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after);
        printf("Dependency fixture=%u ending=%u header/reload=%u mode=%u passed\n", fixture, ending, header, mode); fflush(stdout);
        free(plain); free(source); free(replacement); free(edited); cases++;
    }
    printf("Global dependency cohort: %u SOURCE/Prepare/queued/ready Continue, LF/CRLF/CR, BOM/frontmatter/native reload, full-load/source/CST/cache, IDs/Undo/ownership/retained snapshots passed\n", cases);
}
#include "xui_document_dependency_cases.h"
int main(void)
{
    dependency_matrix();
    dependency_unresolved_caret();
    dependency_differential();
    dependency_faults();
    dependency_continue_running();
    return 0;
}
