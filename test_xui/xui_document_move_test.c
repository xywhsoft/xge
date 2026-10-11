#include "xui_document_move_cases.h"
int main(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"};
    unsigned total = 0, source_failures = 0, rejections = 0, semantic_failures = 0, invariant_failures = 0;
    for (unsigned fixture = 0; fixture < 14; fixture++)
    for (unsigned eol = 0; eol < 3; eol++) for (unsigned bom = 0; bom < 2; bom++)
    for (unsigned api = 0; api < 4; api++) {
        const sample* c = fixture < 8 ? &samples[fixture] : (fixture < 11 ? &eof_samples[fixture - 8] : &definition_eof_samples[fixture - 11]);
        char *before = move_source_variant(c->before, endings[eol], bom);
        char *wanted = move_source_variant(c->after, endings[eol], bom);
        xui_document d = NULL, oracle = NULL;
        xui_document_snapshot old = NULL, now = NULL, expected = NULL, restored = NULL;
        xui_document_transaction t = NULL;
        xui_doc_range_t range = {0}, after = {0};
        uint64_t block = 0, parent = 0, index = 0, moved = 0;
        char *html = NULL, *wanted_html = NULL;
        uint64_t html_bytes = 0, wanted_bytes = 0;
        int quality = 0, raw_equal = 0, semantics = 0, invariants = 0;
        int result = before && wanted ? open_document(before, &d) : -1;
        if (result == XUI_OK) result = xuiDocumentAcquireSnapshot(d, &old);
        if (result == XUI_OK) result = xuiDocumentSourceToPosition(old,
            (uint64_t)(strstr(before, c->needle) - before), &range.tAnchor, &quality);
        range.tCaret = range.tAnchor;
        if (result == XUI_OK) result = move_target(old, c, &range.tAnchor, &block, &parent, &index);
        if (api == 1 || api == 2 || c->root_gap) {
            range.tAnchor.iKind = XUI_DOC_POSITION_GAP; range.tAnchor.iNodeId = parent;
            range.tAnchor.iOffset = api == 2 ? index + 1 : index;
            range.tCaret = range.tAnchor; range.tCaret.iOffset = api == 2 ? index : index + 1;
        }
        if (result == XUI_OK) result = xuiDocumentBeginTransaction(d, NULL, &t);
        if (result == XUI_OK) result = api == 3 ? xuiDocumentTxnMoveNode(t, block, parent,
            c->down ? index + 2 : index - 1) : xuiDocumentTxnMoveBlockRange(t, &range, c->down, &after);
        if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
        xuiDocumentTxnRelease(t);
        if (result == XUI_OK) result = xuiDocumentAcquireSnapshot(d, &now);
        if (result == XUI_OK) {
            raw_equal = move_source_equal(now, wanted);
            if (open_document(wanted, &oracle) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK &&
                xuiDocumentExportHtml(now, &html, &html_bytes) == XUI_OK &&
                xuiDocumentExportHtml(expected, &wanted_html, &wanted_bytes) == XUI_OK)
                semantics = html_bytes == wanted_bytes && !memcmp(html, wanted_html, (size_t)html_bytes);
            invariants = move_source_equal(old, before) &&
                xuiDocumentSnapshotGetChild(now, parent, c->down ? index + 1 : index - 1, &moved) == XUI_OK && moved == block;
            if (xuiDocumentUndo(d, NULL) != XUI_OK || xuiDocumentAcquireSnapshot(d, &restored) != XUI_OK ||
                !move_source_equal(restored, before) || xuiDocumentSnapshotGetChild(restored, parent, index, &moved) != XUI_OK || moved != block)
                invariants = 0;
            xuiDocumentSnapshotRelease(restored); restored = NULL;
            if (xuiDocumentRedo(d, NULL) != XUI_OK || xuiDocumentAcquireSnapshot(d, &restored) != XUI_OK ||
                !move_source_equal(restored, wanted) || !move_source_equal(old, before)) invariants = 0;
        }
        total++; rejections += result != XUI_OK;
        source_failures += result == XUI_OK && !raw_equal;
        semantic_failures += result == XUI_OK && !semantics;
        invariant_failures += result == XUI_OK && !invariants;
        if (result != XUI_OK || !raw_equal || !semantics || !invariants)
            printf("Move matrix fixture=%u name=%s eol=%u bom=%u api=%u result=%d raw_equal=%d semantics=%d invariants=%d\n",
                fixture, c->name, eol, bom, api, result, raw_equal, semantics, invariants);
        xuiDocumentFreeBuffer(html); xuiDocumentFreeBuffer(wanted_html);
        xuiDocumentSnapshotRelease(restored); xuiDocumentSnapshotRelease(expected);
        xuiDocumentSnapshotRelease(now); xuiDocumentSnapshotRelease(old);
        xuiDocumentRelease(oracle); xuiDocumentRelease(d); free(before); free(wanted);
    }
    printf("Move matrix: %u independent source/HTML/identity/snapshot/Undo/Redo cases; %u source differences, %u rejections, %u semantic failures, %u invariant failures\n",
        total, source_failures, rejections, semantic_failures, invariant_failures);
    return source_failures || rejections || semantic_failures || invariant_failures ? 1 : 0;
}
