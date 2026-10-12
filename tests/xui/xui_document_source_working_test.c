/* Peak accounting uses allocator observations, not private builder sizes.
 * The dump mode compares every public record/token/binding against the
 * accepted parent DLL; budget changes must not alter decoded source roles. */
#define main source_index_suite_main
#include "xui_document_source_index_test.c"
#undef main
#include <stddef.h>

typedef union working_header { max_align_t align; size_t bytes; } working_header;
typedef struct working_allocator {
    uint64_t live, peak;
    long remaining;
} working_allocator;
static void* working_alloc(void* user, size_t bytes)
{
    working_allocator* a = user; working_header* p;
    if (!a->remaining) return NULL;
    if (a->remaining > 0) a->remaining--;
    CHECK(bytes <= SIZE_MAX - sizeof(*p)); p = malloc(sizeof(*p) + bytes);
    if (!p) return NULL;
    p->bytes = bytes; a->live += bytes;
    if (a->peak < a->live) a->peak = a->live;
    return p + 1;
}
static void working_free(void* user, void* pointer)
{
    working_allocator* a = user; working_header* p = (working_header*)pointer - 1;
    CHECK(a->live >= p->bytes); a->live -= p->bytes; free(p);
}
static const char* working_paragraph =
    "> - [x] **label** [![a](image \"asset\")](url 'title') &amp; `code`\n>   continuation\n>\n\n";
static char* working_source(uint64_t count, uint64_t* size)
{
    uint64_t unit = strlen(working_paragraph), i; char* source;
    CHECK(count <= (SIZE_MAX - 1) / unit); *size = count * unit;
    source = malloc((size_t)*size + 1); CHECK(source);
    for (i = 0; i < count; i++) memcpy(source + i * unit, working_paragraph, (size_t)unit);
    source[*size] = 0; return source;
}
static void working_budget(void)
{
    unsigned power; long faults = 0;
    for (power = 0; power < 5; power++) {
        uint64_t count = UINT64_C(512) << power, bytes, base, measured; int result;
        char* source = working_source(count, &bytes); working_allocator a = {0, 0, -1};
        xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot s;
        xui_document_source_index unlimited, index = NULL; xui_doc_source_index_info_t info = {0};
        xui_doc_source_index_desc_t budget = {0}; xui_doc_memory_stats_t before = {0}, after = {0};
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = 3;
        desc.onAlloc = working_alloc; desc.onFree = working_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, bytes) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && xuiDocumentMarkSaved(d, s) == XUI_OK);
        base = a.live; a.peak = base; budget.iSize = sizeof(budget); budget.iMaxWorkingBytes = UINT64_MAX;
        CHECK(xuiDocumentSnapshotCreateSourceIndex(s, &budget, &unlimited) == XUI_OK);
        measured = a.peak - base; info.iSize = sizeof(info);
        CHECK(xuiDocumentSourceIndexGetInfo(unlimited, &info) == XUI_OK && !info.iUnclassifiedBytes);
        CHECK(info.iPeakWorkingBytes == measured && info.iAllocatedBytes == a.live - base);
        CHECK(power < 4 ? measured <= UINT64_C(64) * 1024 * 1024 : measured > UINT64_C(64) * 1024 * 1024);
        result = xuiDocumentSnapshotCreateSourceIndex(s, NULL, &index);
        if (power < 4) {
            CHECK(result == XUI_OK && index); compare_indices(unlimited, index);
            xuiDocumentSourceIndexRelease(index);
        } else CHECK(result == XUI_DOC_ERROR_LIMIT && !index);
        CHECK(a.live - base == info.iAllocatedBytes);
        budget.iMaxWorkingBytes = measured;
        CHECK(xuiDocumentSnapshotCreateSourceIndex(s, &budget, &index) == XUI_OK);
        compare_indices(unlimited, index); xuiDocumentSourceIndexRelease(index);
        budget.iMaxWorkingBytes--;
        CHECK(xuiDocumentSnapshotCreateSourceIndex(s, &budget, &index) == XUI_DOC_ERROR_LIMIT && !index);
        CHECK(a.live - base == info.iAllocatedBytes);
        before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetMemoryStats(d, &before) == XUI_OK);
        if (!power) {
            long point; int succeeded = 0; budget.iMaxWorkingBytes = UINT64_MAX;
            for (point = 0; point < 1000; point++) {
                a.remaining = point; result = xuiDocumentSnapshotCreateSourceIndex(s, &budget, &index); a.remaining = -1;
                if (result == XUI_OK) {
                    compare_indices(unlimited, index); xuiDocumentSourceIndexRelease(index); succeeded = 1; faults = point;
                } else CHECK(result == XUI_ERROR_OUT_OF_MEMORY && !index);
                CHECK(a.live - base == info.iAllocatedBytes && !xuiDocumentIsDirty(d));
                CHECK(xuiDocumentGetMemoryStats(d, &after) == XUI_OK && before.iCurrentBytes == after.iCurrentBytes &&
                    before.iHistoryBytes == after.iHistoryBytes && before.iOtherBytes == after.iOtherBytes &&
                    before.iSnapshotCount == after.iSnapshotCount && before.iLiveBytes == after.iLiveBytes);
                if (succeeded) break;
            }
            CHECK(succeeded);
        }
        index_check(unlimited, source, bytes, 0);
        printf("Working source index: %llu containers / %llu source bytes / %llu independently observed peak bytes / %llu retained bytes / default %d\n",
            (unsigned long long)count, (unsigned long long)bytes, (unsigned long long)measured,
            (unsigned long long)info.iAllocatedBytes, result);
        xuiDocumentSourceIndexRelease(unlimited); CHECK(a.live == base);
        xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d); CHECK(!a.live); free(source);
    }
    printf("Working source index faults: %ld OOM points; exact peak and peak-minus-one, unchanged content/history and complete reclamation\n", faults);
}
static void working_dump_one(const char* source, uint64_t bytes, unsigned dialect)
{
    xui_document d = open_source(source, bytes, dialect, NULL); xui_document_snapshot s;
    xui_document_source_index index; xui_doc_source_index_desc_t desc = {0}; xui_doc_source_index_info_t info = {0};
    uint64_t header[7], i;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); desc.iSize = sizeof(desc); desc.iMaxWorkingBytes = UINT64_MAX;
    CHECK(xuiDocumentSnapshotCreateSourceIndex(s, &desc, &index) == XUI_OK); info.iSize = sizeof(info);
    CHECK(xuiDocumentSourceIndexGetInfo(index, &info) == XUI_OK);
    header[0] = bytes; header[1] = dialect; header[2] = info.iRecordCount; header[3] = info.iTokenCount;
    header[4] = info.iBindingCount; header[5] = info.iUnclassifiedBytes; header[6] = info.iOpaqueBytes;
    CHECK(fwrite(header, sizeof(header), 1, stdout) == 1);
    for (i = 0; i < info.iRecordCount; i++) {
        xui_doc_source_record_t r = {0}; r.iSize = sizeof(r);
        CHECK(xuiDocumentSourceIndexGetRecord(index, i, &r) == XUI_OK); r.iNodeId = 0;
        CHECK(fwrite(&r, sizeof(r), 1, stdout) == 1);
    }
    for (i = 0; i < info.iTokenCount; i++) {
        xui_doc_source_token_t t = {0}; t.iSize = sizeof(t);
        CHECK(xuiDocumentSourceIndexGetToken(index, i, &t) == XUI_OK); t.iNodeId = 0;
        CHECK(fwrite(&t, sizeof(t), 1, stdout) == 1);
    }
    for (i = 0; i < info.iBindingCount; i++) {
        xui_doc_source_binding_t b = {0}; b.iSize = sizeof(b);
        CHECK(xuiDocumentSourceIndexGetBinding(index, i, &b) == XUI_OK); b.tRange.iNodeId = 0;
        CHECK(fwrite(&b, sizeof(b), 1, stdout) == 1);
    }
    xuiDocumentSourceIndexRelease(index); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
}
static int working_dump(void)
{
    unsigned i, dialect;
#ifdef _WIN32
    CHECK(_setmode(_fileno(stdout), _O_BINARY) != -1);
#endif
    for (i = 0; i < sizeof(fixtures) / sizeof(*fixtures); i++)
        for (dialect = 1; dialect <= 3; dialect++) working_dump_one(fixtures[i], strlen(fixtures[i]), dialect);
    for (i = 0; i < 4; i++) {
        uint64_t bytes; char* source = working_source(UINT64_C(512) << i, &bytes);
        working_dump_one(source, bytes, 3); free(source);
    }
    CHECK(fflush(stdout) == 0); return 0;
}
int main(int argc, char** argv)
{
    if (argc == 2 && !strcmp(argv[1], "--dump")) return working_dump();
    if (argc == 2 && !strcmp(argv[1], "--transport")) return transport();
    working_budget();
    if (argc == 2 && !strcmp(argv[1], "--working")) return 0;
    return source_index_suite_main(0, NULL);
}
