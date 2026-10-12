#define main prefix_baseline_core_main
#include "xui_document_test.c"
#undef main
static const struct { const char* before; const char* after; } prefix_cases[] = {
    {"- > + > abcd\n  >   >\n  >   > [unused]: /raw 'Title'\n",
     "- > + > ab\n  >   >\n  >   > cd\n  >   >\n  >   > [unused]: /raw 'Title'\n"},
    {"- > 7) [x] abcd\n  >    \n  >    [unused]: /raw 'Title'\n",
     "- > 7) [x] ab\n  >\n  >    cd\n  >    \n  >    [unused]: /raw 'Title'\n"}
};
int main(void)
{
    unsigned sample; int failed = 0;
    for (sample = 0; sample < sizeof(prefix_cases) / sizeof(*prefix_cases); sample++) {
        xui_document d = test_markdown_open(prefix_cases[sample].before);
        xui_document oracle = test_markdown_open(prefix_cases[sample].after), reloaded;
        xui_document_snapshot actual, expected, fresh;
        xui_document_transaction transaction; xui_doc_position_t caret;
        xui_doc_range_t range = test_find(d, "abcd");
        char source[2048]; uint64_t bytes, i;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK);
        range.tAnchor.iOffset += 2; range.tCaret = range.tAnchor;
        CHECK(xuiDocumentBeginTransaction(d, NULL, &transaction) == XUI_OK &&
            xuiDocumentTxnReplaceRange(transaction, &range, "\n", 1, &caret) == XUI_OK &&
            xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        nested_prefix_same_semantics(actual, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
        nested_prefix_caret(d, caret);
        CHECK(xuiDocumentSnapshotCopySource(actual, source, sizeof(source), &bytes) == XUI_OK);
        reloaded = test_markdown_open(source);
        CHECK(xuiDocumentAcquireSnapshot(reloaded, &fresh) == XUI_OK);
        nested_prefix_same_semantics(actual, XUI_DOCUMENT_ROOT, fresh, XUI_DOCUMENT_ROOT);
        nested_prefix_same_block_tokens(actual, XUI_DOCUMENT_ROOT, fresh, XUI_DOCUMENT_ROOT);
        if (strcmp(source, prefix_cases[sample].after)) failed = 1;
        printf("PREFIX SAMPLE %u raw_equal=%d actual_hex=", sample, !strcmp(source, prefix_cases[sample].after));
        for (i = 0; i < bytes; i++) printf("%02x", (unsigned char)source[i]);
        puts("");
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d));
        test_source(d, prefix_cases[sample].before, 1);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
        test_source(d, source, 1);
        xuiDocumentSnapshotRelease(fresh); xuiDocumentRelease(reloaded);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
        xuiDocumentSnapshotRelease(actual); xuiDocumentRelease(d);
    }
    return failed;
}
