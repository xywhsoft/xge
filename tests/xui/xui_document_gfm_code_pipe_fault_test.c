#define XUI_DOC_GFM_CODE_PIPE_HELPERS_ONLY
#include "xui_document_gfm_code_pipe_source_test.c"
#undef XUI_DOC_GFM_CODE_PIPE_HELPERS_ONLY

static int pipe_cancel(void* user)
{
    long* remaining = user;
    if (!*remaining) return 1;
    if (*remaining > 0) (*remaining)--;
    return 0;
}
static void pipe_fault_verify(xui_document_snapshot snapshot, const char* source)
{
    xui_document_source_index index; char word[256]; uint64_t used = 0; unsigned pipes = 0;
    TABLE_CHECK(xuiDocumentSnapshotCreateSourceIndex(snapshot, NULL, &index) == XUI_OK);
    pipe_source_scan(snapshot, index, XUI_DOCUMENT_ROOT, XUI_DOC_LINK | XUI_DOC_CODE, word, &used, &pipes);
    TABLE_CHECK(used == 3 && !memcmp(word, "a|b", 3) && pipes == 1);
    table_source(snapshot, source); integration_equivalent(snapshot, source); xuiDocumentSourceIndexRelease(index);
}
int main(void)
{
    const char* before = "old &amp;\n\n[retain]: <raw> 'Title'\n";
    const char* after = "> | H |\n> | --- |\n> | [``a\\|b``](<raw> 'Title') |\n\nuse [^n].\n\n[^n]:\n    | H |\n    | --- |\n    | `x\\|y` |\n";
    unsigned mode;
    for (mode = 0; mode < 3; mode++) {
        long budget; unsigned failures = 0;
        for (budget = 0; budget < 10000; budget++) {
            table_allocator allocator = {-1, 0}; xui_document document = table_open(mode ? after : before, &allocator);
            xui_document_snapshot old, current; uint64_t revision; unsigned live; int result;
            xui_document_source_index index = NULL;
            TABLE_CHECK(xuiDocumentClearHistory(document) == XUI_OK && xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
                xuiDocumentMarkSaved(document, old) == XUI_OK);
            revision = xuiDocumentGetRevision(document); live = allocator.live;
            if (!mode) {
                allocator.remaining = budget; result = xuiDocumentLoadMarkdown(document, after, strlen(after)); allocator.remaining = -1;
            } else if (mode == 1) {
                allocator.remaining = budget; result = xuiDocumentSnapshotCreateSourceIndex(old, NULL, &index); allocator.remaining = -1;
            } else {
                long remaining = budget; xui_doc_source_index_desc_t desc = {0};
                desc.iSize = sizeof(desc); desc.onCancel = pipe_cancel; desc.pCancelUser = &remaining;
                result = xuiDocumentSnapshotCreateSourceIndex(old, &desc, &index);
            }
            TABLE_CHECK(result == XUI_OK || result == (mode == 2 ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY));
            if (result != XUI_OK) {
                failures++; TABLE_CHECK(!index && allocator.live == live && xuiDocumentGetRevision(document) == revision &&
                    !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document));
                TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
                table_source(current, mode ? after : before); table_equivalent(current, old); xuiDocumentSnapshotRelease(current);
                if (!mode) TABLE_CHECK(xuiDocumentLoadMarkdown(document, after, strlen(after)) == XUI_OK);
                else TABLE_CHECK(xuiDocumentSnapshotCreateSourceIndex(old, NULL, &index) == XUI_OK);
            }
            if (index) xuiDocumentSourceIndexRelease(index);
            TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
            pipe_fault_verify(current, after); table_source(old, mode ? after : before);
            xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(old); xuiDocumentRelease(document); TABLE_CHECK(!allocator.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(budget < 10000 && failures);
        printf("GFM pipe fault mode=%u failures=%u atomic/old snapshot/retry/source/index/zero leaks passed\n", mode, failures);
    }
    return 0;
}
