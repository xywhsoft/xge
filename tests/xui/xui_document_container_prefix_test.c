#include "xui_document_container_move_cases.h"

static const container_move_case container_complex_cases[] = {
    {"root-to-quote-multiline", "alpha &amp; __one__\n\n>anchor &lt;\n>\n> tail __same__\n\nBETA &#60;  \nnext `code`\n",
        "alpha &amp; __one__\n\n>anchor &lt;\n>\n> BETA &#60;  \n> next `code`\n>\n> tail __same__\n\n", "BETA", "anchor", 1},
    {"quote-to-root-multiline", "alpha &amp; __one__\n\n>anchor &lt;\n>\n> BETA &#60;  \n>next `code`\n>\n> tail __same__\n\n",
        "alpha &amp; __one__\n\nBETA &#60;  \nnext `code`\n\n>anchor &lt;\n>\n> tail __same__\n\n", "BETA", "alpha", 1},
    {"root-to-nested-quote", "alpha &amp; __one__\n\n>>anchor &lt;\n>>\n>> tail __same__\n\nBETA &#60;\n",
        "alpha &amp; __one__\n\n>>anchor &lt;\n>>\n>> BETA &#60;\n>>\n>> tail __same__\n\n", "BETA", "anchor", 1},
    {"nested-task-first-paragraph", ">+ [X] alpha &amp; __one__\n>\n>   BETA &#60;\n>\n>   gamma\n",
        ">+ [X] BETA &#60;\n>\n>   alpha &amp; __one__\n>\n>   gamma\n", "BETA", "alpha", 0},
    {"inline-openers-first-paragraph", "+ __alpha__ &amp;\n\n  __BETA__ &#60;\n\n  gamma\n",
        "+ __BETA__ &#60;\n\n  __alpha__ &amp;\n\n  gamma\n", "BETA", "alpha", 0},
    {"list-tab-to-ordered-item", "+ alpha &amp; __one__\n\n  BETA &#60;\n\t  next `code`\n\n  tail __same__\n\n123) anchor &lt;\n\n     end __other__\n",
        "+ alpha &amp; __one__\n\n  tail __same__\n\n123) anchor &lt;\n\n     BETA &#60;\n         next `code`\n\n     end __other__\n", "BETA", "anchor", 1}
};
#define container_cases container_complex_cases
static char* container_variant(const char* raw, const char* eol, unsigned bom, unsigned eof)
{
    size_t count = strlen(raw), size = bom ? 3 : 0, at = 0;
    if (eof && count && raw[count - 1] == '\n') count--;
    for (size_t i = 0; i < count; i++) size += raw[i] == '\n' ? strlen(eol) : 1;
    char* text = malloc(size + 1);
    if (!text) return NULL;
    if (bom) { memcpy(text, "\xef\xbb\xbf", 3); at = 3; }
    for (size_t i = 0; i < count; i++) {
        if (raw[i] == '\n') { memcpy(text + at, eol, strlen(eol)); at += strlen(eol); }
        else text[at++] = raw[i];
    }
    text[at] = 0; return text;
}
static int container_all_ids(xui_document_snapshot old, xui_document_snapshot now,
    uint64_t id, uint64_t moving, uint64_t new_parent)
{
    xui_doc_node_info_t a = {0}, b = {0}; a.iSize = b.iSize = sizeof(a);
    if (xuiDocumentSnapshotGetNode(old, id, &a) != XUI_OK ||
        xuiDocumentSnapshotGetNode(now, id, &b) != XUI_OK || a.iKind != b.iKind ||
        a.iTextBytes != b.iTextBytes || b.iParentId != (id == moving ? new_parent : a.iParentId)) return 0;
    for (uint64_t i = 0; i < a.iChildCount; i++) {
        uint64_t child;
        if (xuiDocumentSnapshotGetChild(old, id, i, &child) != XUI_OK ||
            !container_all_ids(old, now, child, moving, new_parent)) return 0;
    }
    return 1;
}
#ifndef CONTAINER_MOVE_MATRIX_NO_MAIN
int main(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"};
    unsigned total = 0, source_failures = 0, semantic_failures = 0, rejections = 0, history_failures = 0;
    for (unsigned fixture = 0; fixture < 6; fixture++)
    for (unsigned eol = 0; eol < 3; eol++) for (unsigned bom = 0; bom < 2; bom++)
    for (unsigned eof = 0; eof < 2; eof++)
    for (unsigned reverse = 0; reverse < 1u; reverse++) {
        const container_move_case* c = &container_cases[fixture];
        char *before = container_variant(reverse ? c->after : c->before, endings[eol], bom, eof),
            *wanted = container_variant(reverse ? c->before : c->after, endings[eol], bom, (fixture == 0 || fixture == 2) ? 0 : eof);
        xui_document d = NULL, oracle = NULL;
        xui_document_snapshot old = NULL, now = NULL, expected = NULL, restored = NULL;
        xui_document_transaction t = NULL;
        xui_doc_node_info_t parent = {0}; parent.iSize = sizeof(parent);
        uint64_t moving = 0, target = 0, moved = 0, na = 0, nb = 0;
        char *a = NULL, *b = NULL;
        unsigned index = reverse ? 1 : (unsigned)c->index;
        int source = 0, semantic = 0, history = 0, result = before && wanted ? container_open(before, &d) : -1;
        if (result == XUI_OK) result = xuiDocumentAcquireSnapshot(d, &old);
        if (result == XUI_OK) result = container_paragraph(old, before, c->moving, &moving);
        if (result == XUI_OK) result = container_paragraph(old, before, c->target, &target);
        if (result == XUI_OK) result = xuiDocumentSnapshotGetNode(old, target, &parent);
        if (result == XUI_OK) result = xuiDocumentBeginTransaction(d, NULL, &t);
        if (result == XUI_OK) result = xuiDocumentTxnMoveNode(t, moving, parent.iParentId, reverse ? 2 : c->index);
        if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
        xuiDocumentTxnRelease(t);
        if (result == XUI_OK) result = xuiDocumentAcquireSnapshot(d, &now);
        if (result == XUI_OK) {
            source = container_source(now, wanted);
            if (container_open(wanted, &oracle) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK &&
                xuiDocumentExportHtml(now, &a, &na) == XUI_OK && xuiDocumentExportHtml(expected, &b, &nb) == XUI_OK)
                semantic = na == nb && !memcmp(a, b, (size_t)na);
            history = container_source(old, before) && container_all_ids(old, now, 1, moving, parent.iParentId) &&
                xuiDocumentSnapshotGetChild(now, parent.iParentId, index, &moved) == XUI_OK && moved == moving;
            if (xuiDocumentUndo(d, NULL) != XUI_OK || xuiDocumentAcquireSnapshot(d, &restored) != XUI_OK ||
                !container_source(restored, before) || !container_all_ids(old, restored, 1, 0, 0)) history = 0;
            xuiDocumentSnapshotRelease(restored); restored = NULL;
            if (xuiDocumentRedo(d, NULL) != XUI_OK || xuiDocumentAcquireSnapshot(d, &restored) != XUI_OK ||
                !container_source(restored, wanted) || !container_all_ids(old, restored, 1, moving, parent.iParentId)) history = 0;
        }
        source_failures += result == XUI_OK && !source; semantic_failures += result == XUI_OK && !semantic;
        history_failures += result == XUI_OK && !history; rejections += result != XUI_OK; total++;
        if (result != XUI_OK || !source || !semantic || !history) {
            printf("Container matrix fixture=%u eol=%u bom=%u eof=%u reverse=%u result=%d source=%d semantic=%d history=%d\n",
                fixture, eol, bom, eof, reverse, result, source, semantic, history);
            if (result == XUI_OK && !source) {
                char raw[4096]; uint64_t bytes;
                if (xuiDocumentSnapshotCopySource(now, raw, sizeof(raw) - 1, &bytes) == XUI_OK) {
                    raw[bytes] = 0; printf("Actual=[%s]\nWanted=[%s]\n", raw, wanted);
                }
            }
        }
        free(before); free(wanted); xuiDocumentFreeBuffer(a); xuiDocumentFreeBuffer(b);
        xuiDocumentSnapshotRelease(restored); xuiDocumentSnapshotRelease(expected); xuiDocumentSnapshotRelease(now);
        xuiDocumentSnapshotRelease(old); xuiDocumentRelease(oracle); xuiDocumentRelease(d);
    }
    printf("Container matrix: %u independent source/HTML/all-NodeId/snapshot/Undo/Redo cases; %u source differences, %u semantic differences, %u rejections, %u history differences\n",
        total, source_failures, semantic_failures, rejections, history_failures);
    return total != 72 || source_failures || semantic_failures || rejections || history_failures ? 1 : 0;
}
#endif
