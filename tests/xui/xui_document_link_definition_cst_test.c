#define main old_collector_main
#include "xui_document_cst_collector_test.c"
#undef main

static const char* definitions[] = {
    "[a]: <uri> \"title\"\n\n[a]\n",
    "[ a b ]:\tbare(thing)\t'title'  \n",
    "[a]: <uri>\n  \"first\nlast\"\n\n[a]\n",
    "[a\n b]: <> ()\n[a b]\n",
    "[a]: <first> \"x\"\n[a]: <uri> 'title'\n[a]\n",
    "[unused]: uri\n",
    "> - [a]: <uri> (title)\n>   [a]\n",
    "> [a]:\n> \t<uri>\n> \t'title'\n>\n> [a]\n",
    "[^n]: [a]: <uri> \"title\"\n\nuse[^n]\n",
    "top\n\n[^n]: [a]: <uri> 'title'\n",
    "[a\\]b]: <u\\>ri> \"t\\\"itle\"\n",
    "[空 白]: <中文> (标题)\n[空 白]\n",
    "[a]: <uri> \"one\n two\n three\n four\n five\n six\n seven\n eight\n nine\n ten\n"
    " eleven\n twelve\n thirteen\n fourteen\n fifteen\n sixteen\n seventeen\n eighteen\n nineteen\n twenty\"\n[a]\n"
};
static void fields(doc_state* state, const char* source, unsigned fixture)
{
    const char* uri = strstr(source, "<uri>"); uint64_t i, j, matches = 0, columns = 0, endings = 0;
    for (i = 0; i < doc_seq_size(state->reference_syntax); i++) {
        uint64_t key = doc_seq_get_id(state->reference_syntax, i); doc_source_cst_header h;
        xui_doc_reference_definition_t d;
        CHECK(key && doc_source_cst_info(state, key-1, &h) == XUI_OK);
        CHECK(doc_seq_read(state->references, (key-1)*sizeof(d), &d, sizeof(d)) == XUI_OK);
        for (j = 0; j < h.count; j++) {
            doc_source_cst_record r;
            CHECK(doc_source_cst_read(state, key-1, j, &r) == XUI_OK);
            if (fixture == 7 && r.kind == DOC_CST_INDENT && (r.flags & DOC_CST_INDENT_DEFINITION) &&
                d.iSourceStart+r.end == (uint64_t)(uri-source)) {
                CHECK(!r.metadata[0] && r.metadata[1] == 4 && r.metadata[2] == 4); columns++;
            }
            if (fixture == 12 && r.kind == DOC_CST_BREAK && r.flags == DOC_CST_BREAK_ENDING) endings++;
            if (r.kind == DOC_CST_FIELD) CHECK(r.start <= r.metadata[0] && r.metadata[0] <= r.metadata[1] && r.metadata[1] <= r.end);
            if (uri && r.kind == DOC_CST_FIELD && r.detail == DOC_CST_FIELD_DESTINATION &&
                d.iSourceStart+r.start == (uint64_t)(uri-source) && d.iSourceStart+r.end == (uint64_t)(uri-source+5)) {
                CHECK(d.iSourceStart+r.metadata[0] == (uint64_t)(uri-source+1) &&
                    d.iSourceStart+r.metadata[1] == (uint64_t)(uri-source+4)); matches++;
            }
        }
    }
    if (fixture == 7) CHECK(columns == 1);
    if (fixture == 12) CHECK(endings == 20);
    /* The existing dialect renders definition-looking text inside a footnote
     * as a paragraph. It must retain that grammar rather than invent a link
     * declaration from the presence of '<uri>'. */
    if (uri && fixture != 8 && fixture != 9 && !matches) {
        fprintf(stderr, "URI absent at %llu, source %s\n", (unsigned long long)(uri-source), source);
        for (i = 0; i < doc_seq_size(state->reference_syntax); i++) {
            uint64_t k = doc_seq_get_id(state->reference_syntax, i)-1; doc_source_cst_header h;
            xui_doc_reference_definition_t d;
            CHECK(doc_source_cst_info(state, k, &h) == XUI_OK && doc_seq_read(state->references, k*sizeof(d), &d, sizeof(d)) == XUI_OK);
            fprintf(stderr, "definition %llu kind %u start %llu end %llu\n", (unsigned long long)k, d.iKind, (unsigned long long)d.iSourceStart, (unsigned long long)d.iSourceEnd);
            for (j = 0; j < h.count; j++) {
                doc_source_cst_record r; CHECK(doc_source_cst_read(state, k, j, &r) == XUI_OK);
                fprintf(stderr, "record %llu kind %u detail %u start %llu end %llu content %u %u\n", (unsigned long long)j, r.kind, r.detail, (unsigned long long)r.start, (unsigned long long)r.end, r.metadata[0], r.metadata[1]);
            }
        }
        CHECK(matches);
    }
}
static void query(xui_document_snapshot s, fault_allocator* a)
{
    xui_document_source_index index; xui_doc_source_index_info_t info = {0}; uint64_t i;
    CHECK(xuiDocumentSnapshotCreateSourceIndex(s, NULL, &index) == XUI_OK);
    info.iSize = sizeof(info); CHECK(xuiDocumentSourceIndexGetInfo(index, &info) == XUI_OK);
    a->remaining = 0;
    {
        uint64_t end = 0;
        for (i = 0; i < info.iTokenCount; i++) {
            xui_doc_source_token_t token = {0}; token.iSize = sizeof(token);
            CHECK(xuiDocumentSourceIndexGetToken(index, i, &token) == XUI_OK);
            CHECK(token.iSourceStart == end && token.iSourceEnd > end); end = token.iSourceEnd;
        }
        CHECK(end == info.iSourceBytes);
    }
    for (i = 0; i < info.iBindingCount; i++) {
        xui_doc_source_binding_t binding = {0}; binding.iSize = sizeof(binding);
        CHECK(xuiDocumentSourceIndexGetBinding(index, i, &binding) == XUI_OK);
        if (binding.iOriginKind == XUI_DOC_SOURCE_BINDING_DEFINITION_SYNTAX) {
            CHECK(binding.iMetadata & XUI_DOC_SOURCE_BINDING_SOURCE_ONLY);
            CHECK(binding.tRange.iTextStart == UINT64_MAX && binding.tRange.iTextEnd == UINT64_MAX);
        }
    }
    a->remaining = -1; xuiDocumentSourceIndexRelease(index);
}
int main(void)
{
    unsigned f, dialect, eol, bom; unsigned total = 0;
    for (f = 0; f < sizeof(definitions)/sizeof(*definitions); f++) for (dialect = 1; dialect <= 3; dialect++)
    for (eol = 0; eol < 3; eol++) for (bom = 0; bom < 2; bom++) {
        fault_allocator a = {-1, 0}; uint64_t n; char* source = source_variant(definitions[f], eol, bom, &n);
        xui_document d, reload; xui_document_snapshot s, r; char* native; uint64_t bytes;
        printf("Link CST f=%u dialect=%u eol=%u bom=%u\n", f, dialect, eol, bom);
        d = open_doc(source, n, dialect, &a); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
        check_forest(s->state); fields(s->state, source, f); query(s, &a);
        CHECK(xuiDocumentSerialize(s, &native, &bytes) == XUI_OK && xuiDocumentDeserialize(NULL, native, bytes, &reload) == XUI_OK);
        xuiDocumentFreeBuffer(native); CHECK(xuiDocumentAcquireSnapshot(reload, &r) == XUI_OK);
        compare_cst(s->state, r->state); xuiDocumentSnapshotRelease(r); xuiDocumentRelease(reload);
        xuiDocumentRelease(d); fields(s->state, source, f); xuiDocumentSnapshotRelease(s);
        CHECK(!a.live); free(source); total++;
    }
    printf("Link definition CST: %u grammar/dialect/ending/BOM, nested/used/unused/duplicate/empty fields, native reload, public allocation-free syntax bindings and lifetime cases passed\n", total);
    return 0;
}
