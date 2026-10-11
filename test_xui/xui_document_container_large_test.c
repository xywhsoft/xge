#include "src/xui_document_internal.h"
#define CONTAINER_MOVE_MATRIX_NO_MAIN
#include "xui_document_container_move_test.c"
#undef CONTAINER_MOVE_MATRIX_NO_MAIN
#define LARGE_CHECK(x) do { if (!(x)) { fprintf(stderr, "Container large line %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static int large_blob_found(doc_sequence* source, doc_blob* blob)
{
    return source && (source->blob == blob || large_blob_found(source->left, blob) || large_blob_found(source->right, blob));
}
static int large_payload_retained(doc_sequence* old, doc_sequence* now)
{
    return !old || ((!old->blob || large_blob_found(now, old->blob)) && large_payload_retained(old->left, now) && large_payload_retained(old->right, now));
}
static char* large_source(const char* first, uint64_t body, const char* last)
{
    uint64_t a = strlen(first), b = strlen(last);
    char* text = malloc((size_t)(a + body + b + 1));
    LARGE_CHECK(text);
    memcpy(text, first, (size_t)a); memset(text + a, 'x', (size_t)body);
    memcpy(text + a + body, last, (size_t)b + 1); return text;
}
static int large_source_equal(xui_document_snapshot snapshot, const char* wanted)
{
    uint64_t bytes = strlen(wanted), actual = 0;
    char* source = malloc((size_t)bytes + 1);
    LARGE_CHECK(source);
    int result = xuiDocumentSnapshotCopySource(snapshot, source, bytes + 1, &actual) == XUI_OK &&
        actual == bytes && !memcmp(source, wanted, (size_t)bytes);
    free(source); return result;
}
int main(void)
{
    static const struct large_case { const char *first, *last, *wanted_first, *wanted_last, *target; unsigned index; } cases[] = {
        {"alpha &amp; __one__\n\n>anchor &lt;\n>\n> tail __same__\n\nBETA ", " &#60;\n",
         "alpha &amp; __one__\n\n>anchor &lt;\n>\n> BETA ", " &#60;\n>\n> tail __same__\n\n", "anchor", 1},
        {"alpha &amp; __one__\n\n>anchor &lt;\n>\n> BETA ", " &#60;\n>\n> tail __same__\n\n",
         "alpha &amp; __one__\n\nBETA ", " &#60;\n\n>anchor &lt;\n>\n> tail __same__\n\n", "alpha", 1},
        {"+ alpha &amp; __one__\n\n  BETA ", " &#60;\n\n  tail __same__\n\n+ anchor &lt;\n\n  end __other__\n",
         "+ alpha &amp; __one__\n\n  tail __same__\n\n+ anchor &lt;\n\n  BETA ", " &#60;\n\n  end __other__\n", "anchor", 1}
    };
    unsigned total = 0;
    for (unsigned scale = 0; scale < 2; scale++) for (unsigned fixture = 0; fixture < 3; fixture++) {
        uint64_t body = (scale ? 10u : 1u) * 1024u * 1024u;
        const struct large_case* c = &cases[fixture];
        char *before = large_source(c->first, body, c->last), *wanted = large_source(c->wanted_first, body, c->wanted_last);
        xui_document d = NULL, oracle = NULL; xui_document_transaction t = NULL;
        xui_document_snapshot old = NULL, now = NULL, expected = NULL, restored = NULL;
        xui_doc_node_info_t parent = {0}; parent.iSize = sizeof(parent);
        uint64_t moving, target, moved, na, nb; char *a = NULL, *b = NULL;
        LARGE_CHECK(container_open(before, &d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
            container_paragraph(old, before, "BETA", &moving) == XUI_OK && container_paragraph(old, before, c->target, &target) == XUI_OK &&
            xuiDocumentSnapshotGetNode(old, target, &parent) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnMoveNode(t, moving, parent.iParentId, c->index) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t);
        LARGE_CHECK(xuiDocumentAcquireSnapshot(d, &now) == XUI_OK && large_source_equal(now, wanted) && large_source_equal(old, before) &&
            container_all_ids(old, now, 1, moving, parent.iParentId) &&
            xuiDocumentSnapshotGetChild(now, parent.iParentId, c->index, &moved) == XUI_OK && moved == moving &&
            large_payload_retained(old->state->source, now->state->source) &&
            now->state->source_storage_bytes >= old->state->source_storage_bytes &&
            now->state->source_storage_bytes - old->state->source_storage_bytes <= 64 &&
            now->state->source_open_brackets == old->state->source_open_brackets);
        LARGE_CHECK(container_open(wanted, &oracle) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK &&
            xuiDocumentExportHtml(now, &a, &na) == XUI_OK && xuiDocumentExportHtml(expected, &b, &nb) == XUI_OK &&
            na == nb && !memcmp(a, b, (size_t)na));
        LARGE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK &&
            large_source_equal(restored, before) && container_all_ids(old, restored, 1, 0, 0));
        xuiDocumentSnapshotRelease(restored); restored = NULL;
        LARGE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK &&
            large_source_equal(restored, wanted) && container_all_ids(old, restored, 1, moving, parent.iParentId));
        printf("Container large fixture=%u body=%llu source-Blob-growth=%llu; exact source/HTML/all-NodeId/snapshot/Undo/Redo/shared payload passed\n",
            fixture, (unsigned long long)body, (unsigned long long)(now->state->source_storage_bytes - old->state->source_storage_bytes));
        fflush(stdout); total++;
        free(before); free(wanted); xuiDocumentFreeBuffer(a); xuiDocumentFreeBuffer(b); xuiDocumentSnapshotRelease(restored);
        xuiDocumentSnapshotRelease(expected); xuiDocumentSnapshotRelease(now); xuiDocumentSnapshotRelease(old);
        xuiDocumentRelease(oracle); xuiDocumentRelease(d);
    }
    LARGE_CHECK(total == 6);
    printf("Container large: six 1/10MiB source payload cases; all invariants passed\n"); return 0;
}
