/* A partial or complete fence on a line outside an ancestor container must
 * be parsed as an outer line. These expected trees are independent goldens;
 * differential checks alone would share the parser's previous lost bytes. */
#define main document_core_uninvoked
#include "xui_document_test.c"
#undef main

typedef struct fence_node { unsigned kind, depth; const char* text; } fence_node;
#define F(k,d,t) {XUI_DOC_##k,d,t}
static const fence_node quote_literal[] = {F(ROOT,0,""),F(QUOTE,1,""),F(CODE_BLOCK,2,""),F(PARAGRAPH,1,""),F(TEXT,2,"`> text")};
static const fence_node quote_outer_code[] = {F(ROOT,0,""),F(QUOTE,1,""),F(CODE_BLOCK,2,""),F(CODE_BLOCK,1,"out\n")};
static const fence_node list_literal[] = {F(ROOT,0,""),F(LIST,1,""),F(LIST_ITEM,2,""),F(CODE_BLOCK,3,""),F(PARAGRAPH,1,""),F(TEXT,2,"`- text")};
static const fence_node list_outer_code[] = {F(ROOT,0,""),F(LIST,1,""),F(LIST_ITEM,2,""),F(CODE_BLOCK,3,""),F(CODE_BLOCK,1,"out\n")};
static const fence_node nested_literal[] = {F(ROOT,0,""),F(QUOTE,1,""),F(LIST,2,""),F(LIST_ITEM,3,""),F(CODE_BLOCK,4,""),F(PARAGRAPH,2,""),F(TEXT,3,"`> text")};
static const fence_node inside_code[] = {F(ROOT,0,""),F(QUOTE,1,""),F(CODE_BLOCK,2,"`> text\n")};
static const fence_node tilde_literal[] = {F(ROOT,0,""),F(QUOTE,1,""),F(CODE_BLOCK,2,""),F(PARAGRAPH,1,""),F(TEXT,2,"~> text")};
static const fence_node ordered_literal[] = {F(ROOT,0,""),F(LIST,1,""),F(LIST_ITEM,2,""),F(CODE_BLOCK,3,""),F(PARAGRAPH,1,""),F(TEXT,2,"`1. text")};
typedef struct fence_fixture { const char *before, *after; const fence_node* nodes; unsigned count; } fence_fixture;
#define C(a,b,n) {a,b,n,sizeof(n)/sizeof(*n)}
static const fence_fixture fence_cases[] = {
    C("> ```c\n> text\n", "> ```c\n`> text\n", quote_literal),
    C("> ```c\n> ```\nout\n", "> ```c\n```\nout\n", quote_outer_code),
    C("- ```c\n  - text\n", "- ```c\n`- text\n", list_literal),
    C("- ```c\n  ```\nout\n", "- ```c\n```\nout\n", list_outer_code),
    C("> - ```c\n>   > text\n", "> - ```c\n> `> text\n", nested_literal),
    C("> ```c\n> > text\n> ```\n", "> ```c\n> `> text\n> ```\n", inside_code),
    C("> ~~~c\n> text\n", "> ~~~c\n~> text\n", tilde_literal),
    C("> ~~~c\n> ~~~\nout\n", "> ~~~c\n~~~\nout\n", quote_outer_code),
    C("1. ```c\n   1. text\n", "1. ```c\n`1. text\n", ordered_literal),
    C(" > ```c\n > text\n", " > ```c\n `> text\n", quote_literal)
};
static uint64_t fence_source(char* out, const char* in, unsigned ending, int bom)
{
    const char* ends[] = {"\n","\r\n","\r"}; uint64_t n = 0;
    if (bom) {memcpy(out,"\xef\xbb\xbf",3); n = 3;}
    while (*in) {
        if (*in == '\n') {size_t z = strlen(ends[ending]); memcpy(out+n,ends[ending],z); n += z;}
        else out[n++] = *in;
        in++;
    }
    out[n] = 0; return n;
}
static void fence_tree(xui_document_snapshot s, uint64_t id, unsigned depth,
    const fence_fixture* fixture, unsigned* ordinal)
{
    xui_doc_node_info_t node = {0}; const fence_node* expected; uint64_t i, child;
    node.iSize = sizeof(node); CHECK(xuiDocumentSnapshotGetNode(s,id,&node) == XUI_OK && *ordinal < fixture->count);
    expected = fixture->nodes + (*ordinal)++;
    if (node.iKind != expected->kind || depth != expected->depth)
        fprintf(stderr,"Fence golden ordinal=%u kind=%u/%u depth=%u/%u source=%s\n",*ordinal-1,node.iKind,expected->kind,depth,expected->depth,fixture->after);
    CHECK(node.iKind == expected->kind && depth == expected->depth); expect_text(s,id,expected->text);
    for (i = 0; i < node.iChildCount; i++) {
        CHECK(xuiDocumentSnapshotGetChild(s,id,i,&child) == XUI_OK); fence_tree(s,child,depth+1,fixture,ordinal);
    }
}
static xui_document_source_index fence_index(xui_document_snapshot s, const char* source, uint64_t size)
{
    xui_document_source_index index; xui_doc_source_index_info_t info = {0}; uint64_t i, end = 0; char bytes[256];
    CHECK(xuiDocumentSnapshotCreateSourceIndex(s,NULL,&index) == XUI_OK);
    info.iSize = sizeof(info); CHECK(xuiDocumentSourceIndexGetInfo(index,&info) == XUI_OK && info.iSourceBytes == size && !info.iUnclassifiedBytes);
    for (i = 0; i < info.iTokenCount; i++) {
        xui_doc_source_token_t token = {0}; token.iSize = sizeof(token);
        CHECK(xuiDocumentSourceIndexGetToken(index,i,&token) == XUI_OK && token.iSourceStart == end && token.iSourceEnd > end && token.iSourceEnd <= size);
        end = token.iSourceEnd;
    }
    CHECK(end == size && size < sizeof(bytes) && xuiDocumentSourceIndexReadSource(index,0,bytes,size) == XUI_OK && !memcmp(bytes,source,(size_t)size));
    for (i = 0; i < info.iBindingCount; i++) {
        xui_doc_source_binding_t binding = {0}; uint64_t p;
        binding.iSize = sizeof(binding); CHECK(xuiDocumentSourceIndexGetBinding(index,i,&binding) == XUI_OK);
        if (binding.iOriginKind == XUI_DOC_SOURCE_BINDING_QUOTE_INDENT) {
            for (p = binding.tRange.iSourceStart; p < binding.tRange.iSourceEnd && source[p] != '>'; p++)
                CHECK(source[p] == ' ' || source[p] == '\t');
            CHECK(p < binding.tRange.iSourceEnd && source[p] == '>');
        }
    }
    return index;
}
static void fence_golden(xui_document_snapshot s, const fence_fixture* fixture, const char* source, uint64_t size)
{
    unsigned ordinal = 0; char bytes[256]; uint64_t n; xui_document_source_index index;
    fence_tree(s,1,0,fixture,&ordinal); CHECK(ordinal == fixture->count);
    CHECK(xuiDocumentSnapshotCopySource(s,bytes,sizeof(bytes),&n) == XUI_OK && n == size && !memcmp(bytes,source,(size_t)size));
    index = fence_index(s,source,size); xuiDocumentSourceIndexRelease(index);
}
static xui_doc_source_patch_t fence_patch(const char* before, uint64_t old_size, const char* after, uint64_t size)
{
    uint64_t start = 0, suffix = 0; xui_doc_source_patch_t patch = {0};
    while (start < old_size && start < size && before[start] == after[start]) start++;
    while (suffix < old_size-start && suffix < size-start && before[old_size-suffix-1] == after[size-suffix-1]) suffix++;
    patch.iSize = sizeof(patch); patch.iStart = start; patch.iEnd = old_size-suffix;
    patch.sText = after+start; patch.iTextBytes = size-start-suffix; return patch;
}
static void fence_matrix(void)
{
    unsigned f, dialect, ending, bom, native, mode, cases = 0; uint64_t incremental = 0;
    for (f = 0; f < sizeof(fence_cases)/sizeof(*fence_cases); f++)
    for (dialect = 1; dialect <= 3; dialect++) for (ending = 0; ending < 3; ending++)
    for (bom = 0; bom < 2; bom++) for (native = 0; native < 2; native++) for (mode = 0; mode < 4; mode++) {
        char initial[256], source[256]; uint64_t old_size = fence_source(initial,fence_cases[f].before,ending,bom);
        uint64_t size = fence_source(source,fence_cases[f].after,ending,bom); fail_allocator a = {-1,0};
        xui_document d, oracle; xui_document_snapshot before, after, expected, restored; xui_document_source_index retained;
        xui_doc_desc_t desc = {0}; xui_doc_source_patch_t patch = fence_patch(initial,old_size,source,size);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc,&d) == XUI_OK && xuiDocumentLoadMarkdown(d,initial,old_size) == XUI_OK);
        if (native) {
            char* serialized; uint64_t bytes;
            CHECK(xuiDocumentAcquireSnapshot(d,&before) == XUI_OK && xuiDocumentSerialize(before,&serialized,&bytes) == XUI_OK);
            xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); CHECK(!a.live);
            CHECK(xuiDocumentDeserialize(&desc,serialized,bytes,&d) == XUI_OK); xuiDocumentFreeBuffer(serialized);
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d,&before) == XUI_OK);
        retained = fence_index(before,initial,old_size);
        if (!mode) CHECK(xuiDocumentLoadMarkdown(d,source,size) == XUI_OK);
        else if (mode == 1) {
            xui_document_transaction t; xui_doc_txn_desc_t td = {0}; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
            CHECK(xuiDocumentBeginTransaction(d,&td,&t) == XUI_OK && xuiDocumentTxnReplaceSource(t,patch.iStart,patch.iEnd,patch.sText,patch.iTextBytes) == XUI_OK && xuiDocumentTxnCommit(t,NULL) == XUI_OK);
            xuiDocumentTxnRelease(t);
        } else {
            xui_document_prepare p;
            if (mode == 2) CHECK(xuiDocumentPrepareSource(d,NULL,&patch,1,&p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_OK);
            else {
                xui_document_prepare next; uint64_t newline = ending == 1 ? 2 : 1;
                xui_doc_source_patch_t first = fence_patch(initial,old_size,source,size-newline), last = {0};
                CHECK(xuiDocumentPrepareSource(d,NULL,&first,1,&p) == XUI_OK);
                if (native) CHECK(xuiDocumentPrepareRun(p) == XUI_OK);
                last.iSize = sizeof(last); last.iStart = last.iEnd = size-newline; last.sText = source+size-newline; last.iTextBytes = newline;
                CHECK(xuiDocumentPrepareContinueSource(d,p,&last,1,&next) == XUI_OK); xuiDocumentPrepareRelease(p); p = next;
                CHECK(xuiDocumentPrepareRun(p) == XUI_OK);
            }
            CHECK(xuiDocumentPreparePublish(d,p,NULL) == XUI_OK); xuiDocumentPrepareRelease(p);
        }
        CHECK(xuiDocumentAcquireSnapshot(d,&after) == XUI_OK); fence_golden(after,fence_cases+f,source,size);
        incremental += inc_stats(d).iMarkdownIncrementalParses;
        CHECK(xuiDocumentCreate(&desc,&oracle) == XUI_OK && xuiDocumentLoadMarkdown(oracle,source,size) == XUI_OK && xuiDocumentAcquireSnapshot(oracle,&expected) == XUI_OK);
        inc_snapshot_equal(after,expected); xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
        if (mode) {
            CHECK(xuiDocumentUndo(d,NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d,&restored) == XUI_OK);
            inc_snapshot_equal(before,restored); expect_identity_tree(before,restored,1); xuiDocumentSnapshotRelease(restored);
            CHECK(xuiDocumentRedo(d,NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d,&restored) == XUI_OK);
            inc_snapshot_equal(after,restored); expect_identity_tree(after,restored,1); xuiDocumentSnapshotRelease(restored);
        }
        {char bytes[256]; CHECK(xuiDocumentSourceIndexReadSource(retained,0,bytes,old_size) == XUI_OK && !memcmp(bytes,initial,(size_t)old_size));}
        xuiDocumentSourceIndexRelease(retained); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d);
        fence_golden(after,fence_cases+f,source,size); xuiDocumentSnapshotRelease(after); CHECK(!a.live); cases++;
    }
    CHECK(cases == 1440);
    printf("Fence container goldens: %u independent tree/raw-source/index cases, %llu incremental; missing ancestors, partial/full backtick/tilde fences, quotes/lists/nesting, dialect/EOL/BOM/native/SOURCE/Prepare/Continue/history/lifetime passed\n",cases,(unsigned long long)incremental);
}
static void fence_faults(void)
{
    const unsigned selected[] = {0,2,4}; unsigned f, mode; long oom = 0, cancel = 0;
    for (f = 0; f < sizeof(selected)/sizeof(*selected); f++) for (mode = 0; mode < 2; mode++) {
        const fence_fixture* fixture = fence_cases+selected[f]; char initial[256], source[256];
        uint64_t old_size = fence_source(initial,fixture->before,1,0), size = fence_source(source,fixture->after,1,0);
        xui_doc_source_patch_t patch = fence_patch(initial,old_size,source,size); long point; int success = 0;
        for (point = 0; point < 4096 && !success; point++) {
            fail_allocator a = {-1,0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot before, after; uint64_t revision; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc,&d) == XUI_OK && xuiDocumentLoadMarkdown(d,initial,old_size) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d,&before) == XUI_OK && xuiDocumentMarkSaved(d,before) == XUI_OK);
            revision = xuiDocumentGetRevision(d); a.remaining = point;
            if (!mode) {
                xui_document_transaction t = NULL; xui_doc_txn_desc_t td = {0}; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
                result = xuiDocumentBeginTransaction(d,&td,&t);
                if (result == XUI_OK) result = xuiDocumentTxnReplaceSource(t,patch.iStart,patch.iEnd,patch.sText,patch.iTextBytes);
                if (result == XUI_OK) result = xuiDocumentTxnCommit(t,NULL);
                a.remaining = -1; if (t) xuiDocumentTxnRelease(t);
            } else {
                xui_document_prepare p = NULL; result = xuiDocumentPrepareSource(d,NULL,&patch,1,&p);
                if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
                a.remaining = -1;
                if (p) {if (result == XUI_OK) result = xuiDocumentPreparePublish(d,p,NULL); xuiDocumentPrepareRelease(p);}
            }
            CHECK(xuiDocumentAcquireSnapshot(d,&after) == XUI_OK);
            if (result == XUI_ERROR_OUT_OF_MEMORY) {
                CHECK(xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
                inc_snapshot_equal(before,after); expect_identity_tree(before,after,1); oom++;
            } else {CHECK(result == XUI_OK); fence_golden(after,fixture,source,size); success = 1;}
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success);
        if (mode) {
            success = 0;
            for (point = 0; point < 4096 && !success; point++) {
                prepare_cancel_allocator a = {0,-1,0,NULL}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_prepare p;
                xui_document_snapshot before, after; uint64_t revision; int result;
                desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
                desc.onAlloc = prepare_cancel_alloc; desc.onFree = prepare_cancel_free; desc.pAllocatorUser = &a;
                CHECK(xuiDocumentCreate(&desc,&d) == XUI_OK && xuiDocumentLoadMarkdown(d,initial,old_size) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d,&before) == XUI_OK && xuiDocumentMarkSaved(d,before) == XUI_OK && xuiDocumentPrepareSource(d,NULL,&patch,1,&p) == XUI_OK);
                revision = xuiDocumentGetRevision(d); a.prepare = p; a.remaining = point; result = xuiDocumentPrepareRun(p); a.prepare = NULL;
                if (a.fired) {
                    CHECK(result == XUI_DOC_ERROR_CANCELLED && xuiDocumentPreparePublish(d,p,NULL) == XUI_DOC_ERROR_CANCELLED && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d)); cancel++;
                } else {CHECK(result == XUI_OK && xuiDocumentPreparePublish(d,p,NULL) == XUI_OK); success = 1;}
                CHECK(xuiDocumentAcquireSnapshot(d,&after) == XUI_OK);
                if (a.fired) {inc_snapshot_equal(before,after); expect_identity_tree(before,after,1);}
                else fence_golden(after,fixture,source,size);
                xuiDocumentPrepareRelease(p); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
            }
            CHECK(success);
        }
    }
    printf("Fence container faults: %ld SOURCE/Prepare OOM and %ld cancellation checkpoints; content/identity/history/savepoint atomic and zero leaks passed\n",oom,cancel);
}
int main(void) {fence_matrix(); fence_faults(); return 0;}
