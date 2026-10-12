#define main existing_collector_main
#include "xui_document_cst_collector_test.c"
#undef main
static xui_doc_memory_stats_t document_memory(xui_document d)
{
    xui_doc_memory_stats_t m={0};m.iSize=sizeof(m);
    CHECK(xuiDocumentGetMemoryStats(d,&m)==XUI_OK);return m;
}
#include "xui_document_memory_oracle.h"
static void integrated_forest_equal(doc_state* a,doc_state* b)
{
    uint64_t i,j,count=doc_source_forest_count(a);
    if(count!=doc_source_forest_count(b))fprintf(stderr,"ordinary root count %llu/%llu\n",
        (unsigned long long)count,(unsigned long long)doc_source_forest_count(b));
    CHECK(count==doc_source_forest_count(b));
    for(i=0;i<count;i++){
        uint64_t x,y;doc_source_cst_header p,q;
        CHECK(doc_source_forest_info(a,i,&x,&p)==XUI_OK && doc_source_forest_info(b,i,&y,&q)==XUI_OK);
        if(x!=y || memcmp(&p,&q,sizeof(p)))fprintf(stderr,"ordinary root %llu base %llu/%llu bytes %llu/%llu count %llu/%llu\n",
            (unsigned long long)i,(unsigned long long)x,(unsigned long long)y,
            (unsigned long long)p.source_bytes,(unsigned long long)q.source_bytes,
            (unsigned long long)p.count,(unsigned long long)q.count);
        CHECK(x==y && !memcmp(&p,&q,sizeof(p)));
        for(j=0;j<p.count;j++){
            doc_source_cst_record r,s;
            CHECK(doc_source_forest_read(a,i,j,&r)==XUI_OK && doc_source_forest_read(b,i,j,&s)==XUI_OK);
            if(memcmp(&r,&s,sizeof(r)))fprintf(stderr,"ordinary root=%llu ordinal=%llu kind=%u/%u detail=%u/%u flags=%u/%u parent=%llu/%llu offsets=%llu..%llu/%llu..%llu\n",
                (unsigned long long)i,(unsigned long long)j,r.kind,s.kind,r.detail,s.detail,r.flags,s.flags,
                (unsigned long long)r.parent,(unsigned long long)s.parent,
                (unsigned long long)r.start,(unsigned long long)r.end,(unsigned long long)s.start,(unsigned long long)s.end);
            CHECK(!memcmp(&r,&s,sizeof(r)));
        }
    }
}
static const char* transitions[] = {
    "head\n\n**bold** [a](<uri> 'title')\n\nTAIL\n",
    "# Heading #\n\n> - [x] *item*\n>   continuation\n>\n\nend\n",
    "one | two\n:-- | --:\nx | `y|z`\n\nend\n",
    "~~~ lang extra\nx &amp; y\n~~~\n\nTAIL\n",
    "---\nkey: value\n...\n\nbody\n\nTAIL\n",
    "aa <!-- raw --> <b>HTML</b>\n\n<!-- block -->\n\nTAIL\n",
    "> [r]:\n> \t<uri>\n> \t'title'\n>\n> [r] **bold**\n\nTAIL\n",
    "[r]: first 'title'\n\n[r]\n\nordinary\n\nTAIL\n",
    "![![*alt*](inner)](outer) and 中文\n\nTAIL\n",
    "head[^a]\n\n[^a]: **bold** [a](<uri> \"title\")\n[^b]: *hidden*\n",
    "head[^a] [r]\n\n[r]: target 'title'\n[^a]: [r] **body**\n[^b]: [r] [^a]\n",
    "head[^a]\n\n[^a]: first\n[^a]: *second*\n[^b]: **third**\n",
    "head[^a]\n\n[^a]: first[^b]\n[^b]: next[^a]\n[^unused]: [^b]\n",
    "[x]: first\n[^a]: *body*\n[x]: second\n[^b]: [x]\n",
    "> [^a]: > - *item*\n>     >   continuation\n>\n> [^b]: **other**\n",
    "[^a]: a | b\n    :-- | --:\n    x | y\n\n[^b]: `code`\n",
    "[^a]: **中文** &amp; \\*\n\n[^b]: <b>HTML</b>\n",
    "[^a]:\n\n\t```lang\n\tx\n\t```\n\n[^b]: more\n"
};
static void exact_forest(doc_state* state)
{
    uint64_t i, count = doc_seq_size(state->reference_syntax), definitions = doc_seq_size(state->references)/sizeof(xui_doc_reference_definition_t);
    for (i = 0; i < count; i++) {
        uint64_t key = doc_seq_get_id(state->reference_syntax, i); doc_source_cst_header h;
        CHECK(key && key <= definitions && (!i || doc_seq_get_id(state->reference_syntax, i-1) < key));
        CHECK(doc_source_cst_info(state, key-1, &h) == XUI_OK);
    }
    CHECK(check_forest(state) == count);
}
static int next_cst_binding(xui_document_source_index index, uint64_t* at, uint64_t count,
    xui_doc_source_binding_t* result, xui_doc_source_record_t* record)
{
    while (*at < count) {
        result->iSize = sizeof(*result); CHECK(xuiDocumentSourceIndexGetBinding(index, (*at)++, result) == XUI_OK);
        if (result->iOriginKind != XUI_DOC_SOURCE_BINDING_DEFINITION_SYNTAX) continue;
        record->iSize = sizeof(*record); CHECK(xuiDocumentSourceIndexGetRecord(index, result->tRange.iRecordIndex, record) == XUI_OK);
        result->tRange.iRecordIndex = 0; record->iParentIndex = 0; return 1;
    }
    return 0;
}

/* Every retained ordinary grammar entry must have exactly one public source
 * record, its original parent/content ranges, and at least one owned binding.
 * This checks completeness independently of the projector's role switch. */
static void body_projection_complete(xui_document_snapshot snapshot,xui_document_source_index index)
{
    xui_doc_source_index_info_t info={0};uint64_t i,j=0,owner=0,first=0,base=0,total=0;
    doc_source_cst_header header={0};unsigned* seen;info.iSize=sizeof(info);
    CHECK(xuiDocumentSourceIndexGetInfo(index,&info)==XUI_OK);
    seen=calloc((size_t)info.iRecordCount,sizeof(*seen));CHECK(seen);
    for(i=0;i<doc_source_forest_count(snapshot->state);i++){
        uint64_t raw;doc_source_cst_header h;CHECK(doc_source_forest_info(snapshot->state,i,&raw,&h)==XUI_OK);total+=h.count;
    }
    for(i=0;i<info.iRecordCount;i++){
        xui_doc_source_record_t r={0};doc_source_cst_record s;r.iSize=sizeof(r);
        CHECK(xuiDocumentSourceIndexGetRecord(index,i,&r)==XUI_OK);if(!(r.iFlags&XUI_DOC_SOURCE_RECORD_SYNTAX_DOCUMENT))continue;
        if(!j){first=i;CHECK(doc_source_forest_info(snapshot->state,owner,&base,&header)==XUI_OK);}
        CHECK(r.iKind==XUI_DOC_SOURCE_RECORD_SYNTAX && !r.iNodeId && r.iOriginIndex==owner &&
            doc_source_forest_read(snapshot->state,owner,j,&s)==XUI_OK);
        CHECK(r.iDetail==s.kind && r.iSourceStart==base+s.start && r.iSourceEnd==base+s.end &&
            r.iParentIndex==(!j?0:first+s.parent));
        CHECK(r.iContentStart==base+(s.kind==DOC_CST_FIELD?s.metadata[0]:s.start) &&
            r.iContentEnd==base+(s.kind==DOC_CST_FIELD?s.metadata[1]:s.end));
        seen[i]=1;j++;total--;if(j==header.count){owner++;j=0;}
    }
    CHECK(!total && !j && owner==doc_source_forest_count(snapshot->state));
    for(i=0;i<info.iBindingCount;i++){
        xui_doc_source_binding_t b={0};xui_doc_source_record_t r={0};doc_source_cst_record s;b.iSize=sizeof(b);r.iSize=sizeof(r);
        CHECK(xuiDocumentSourceIndexGetBinding(index,i,&b)==XUI_OK);if(b.iOriginKind!=XUI_DOC_SOURCE_BINDING_DOCUMENT_SYNTAX)continue;
        CHECK(xuiDocumentSourceIndexGetRecord(index,b.tRange.iRecordIndex,&r)==XUI_OK && seen[b.tRange.iRecordIndex]);
        CHECK(doc_source_forest_info(snapshot->state,r.iOriginIndex,&base,&header)==XUI_OK &&
            doc_source_forest_read(snapshot->state,r.iOriginIndex,b.iOriginIndex,&s)==XUI_OK);
        CHECK(s.kind==r.iDetail && b.tRange.iOriginalSourceStart==base+s.start && b.tRange.iOriginalSourceEnd==base+s.end &&
            !b.tRange.iNodeId && b.tRange.iTextStart==UINT64_MAX && b.tRange.iTextEnd==UINT64_MAX &&
            (b.iMetadata&XUI_DOC_SOURCE_BINDING_SOURCE_ONLY));seen[b.tRange.iRecordIndex]++;
    }
    for(i=0;i<info.iRecordCount;i++)if(seen[i])CHECK(seen[i]>=2);
    free(seen);
}
static void source_projection_equal(xui_document_snapshot actual,xui_document_source_index a,
    xui_document_snapshot full,xui_document_source_index b)
{
    xui_doc_source_index_info_t x={0},y={0};uint64_t i;x.iSize=y.iSize=sizeof(x);
    CHECK(xuiDocumentSourceIndexGetInfo(a,&x)==XUI_OK && xuiDocumentSourceIndexGetInfo(b,&y)==XUI_OK);
    CHECK(x.iSourceBytes==y.iSourceBytes && x.iRecordCount==y.iRecordCount && x.iBindingCount==y.iBindingCount &&
        x.iTokenCount==y.iTokenCount && x.iUnclassifiedBytes==y.iUnclassifiedBytes && x.iOpaqueBytes==y.iOpaqueBytes);
    body_projection_complete(actual,a);body_projection_complete(full,b);
    for(i=0;i<x.iRecordCount;i++){
        xui_doc_source_record_t p={0},q={0};p.iSize=q.iSize=sizeof(p);
        CHECK(xuiDocumentSourceIndexGetRecord(a,i,&p)==XUI_OK && xuiDocumentSourceIndexGetRecord(b,i,&q)==XUI_OK);
        CHECK(!!p.iNodeId==!!q.iNodeId);p.iNodeId=q.iNodeId=0;CHECK(!memcmp(&p,&q,sizeof(p)));
    }
    for(i=0;i<x.iTokenCount;i++){
        xui_doc_source_token_t p={0},q={0};p.iSize=q.iSize=sizeof(p);
        CHECK(xuiDocumentSourceIndexGetToken(a,i,&p)==XUI_OK && xuiDocumentSourceIndexGetToken(b,i,&q)==XUI_OK);
        CHECK(!!p.iNodeId==!!q.iNodeId);p.iNodeId=q.iNodeId=0;CHECK(!memcmp(&p,&q,sizeof(p)));
    }
    for(i=0;i<x.iBindingCount;i++){
        xui_doc_source_binding_t p={0},q={0};p.iSize=q.iSize=sizeof(p);
        CHECK(xuiDocumentSourceIndexGetBinding(a,i,&p)==XUI_OK && xuiDocumentSourceIndexGetBinding(b,i,&q)==XUI_OK);
        CHECK(!!p.tRange.iNodeId==!!q.tRange.iNodeId);p.tRange.iNodeId=q.tRange.iNodeId=0;CHECK(!memcmp(&p,&q,sizeof(p)));
    }
}

static void equivalent(xui_document_snapshot actual, xui_document_snapshot full)
{
    xui_document_source_index a, b; xui_doc_source_index_info_t ai = {0}, bi = {0}; uint64_t at = 0, bt = 0;
    char *ah, *bh; uint64_t an, bn;
    integrated_forest_equal(actual->state,full->state);
    exact_forest(actual->state); exact_forest(full->state);
    CHECK(doc_seq_size(actual->state->reference_syntax) == doc_seq_size(full->state->reference_syntax));
    compare_cst(actual->state, full->state);
    CHECK(xuiDocumentExportHtml(actual, &ah, &an) == XUI_OK && xuiDocumentExportHtml(full, &bh, &bn) == XUI_OK);
    CHECK(an == bn && !memcmp(ah, bh, (size_t)an)); xuiDocumentFreeBuffer(ah); xuiDocumentFreeBuffer(bh);
    CHECK(xuiDocumentSnapshotCreateSourceIndex(actual, NULL, &a) == XUI_OK && xuiDocumentSnapshotCreateSourceIndex(full, NULL, &b) == XUI_OK);
    ai.iSize = bi.iSize = sizeof(ai); CHECK(xuiDocumentSourceIndexGetInfo(a, &ai) == XUI_OK && xuiDocumentSourceIndexGetInfo(b, &bi) == XUI_OK);
    for (;;) {
        xui_doc_source_binding_t av = {0}, bv = {0}; xui_doc_source_record_t ar = {0}, br = {0};
        int af = next_cst_binding(a, &at, ai.iBindingCount, &av, &ar), bf = next_cst_binding(b, &bt, bi.iBindingCount, &bv, &br);
        CHECK(af == bf); if (!af) break;
        CHECK(!memcmp(&av, &bv, sizeof(av)) && !memcmp(&ar, &br, sizeof(ar)));
    }
    source_projection_equal(actual,a,full,b);
    xuiDocumentSourceIndexRelease(a); xuiDocumentSourceIndexRelease(b);
}
static uint64_t codepoint_end(const char* source, uint64_t at, uint64_t n)
{
    if (at == n) return at;
    do { at++; } while (at < n && ((unsigned char)source[at] & 0xc0) == 0x80);
    return at;
}
static void transition_case(const char* source, uint64_t n, uint64_t at, unsigned operation, unsigned mode)
{
    static const char* insert[] = {"x", "", "**"}; uint64_t end = operation ? codepoint_end(source, at, n) : at;
    uint64_t added = strlen(insert[operation]), expected_n = n-(end-at)+added;
    char* expected = malloc((size_t)expected_n+1); fault_allocator allocator = {-1, 0};
    xui_document d = open_doc(source, n, 3, &allocator), oracle; xui_document_snapshot before, after, full, undo;
    CHECK(expected); memcpy(expected, source, (size_t)at); memcpy(expected+at, insert[operation], (size_t)added);
    memcpy(expected+at+added, source+end, (size_t)(n-end)); expected[expected_n] = 0;
    CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
    memory_oracle_check(d);
    if (!mode) {
        xui_document_transaction t; xui_doc_txn_desc_t desc = {0}; desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE;
        int result;
        CHECK(xuiDocumentBeginTransaction(d, &desc, &t) == XUI_OK);
        result = xuiDocumentTxnReplaceSource(t, at, end, insert[operation], added);
        if (result != XUI_OK) fprintf(stderr, "SOURCE edit returned %d at %llu\n", result, (unsigned long long)at);
        CHECK(result == XUI_OK);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    } else {
        xui_document_prepare p; xui_doc_source_patch_t patch = {0}; patch.iSize = sizeof(patch);
        patch.iStart = at; patch.iEnd = end; patch.sText = insert[operation]; patch.iTextBytes = added;
        CHECK(xuiDocumentPrepareSource(d,NULL,&patch,1,&p)==XUI_OK);
        if(mode==2)doc_memory_current(d->state,NULL);
        {int result=xuiDocumentPrepareRun(p);if(mode==2)doc_memory_current(NULL,d->state);CHECK(result==XUI_OK);}
        if(mode==3){
            xui_document_prepare next;xui_doc_source_patch_t extra={0};
            char* extended=realloc(expected,(size_t)expected_n+6);CHECK(extended);expected=extended;
            memcpy(expected+expected_n,"\nEND\n",6);
            extra.iSize=sizeof(extra);extra.iStart=extra.iEnd=expected_n;extra.sText="\nEND\n";extra.iTextBytes=5;
            expected_n+=5;
            CHECK(xuiDocumentPrepareContinueSource(d,p,&extra,1,&next)==XUI_OK);xuiDocumentPrepareRelease(p);p=next;
            CHECK(xuiDocumentPrepareRun(p)==XUI_OK);
        }
        memory_oracle_check(d);
        CHECK(d->state == before->state && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); xuiDocumentPrepareRelease(p);
    }
    oracle = open_doc(expected, expected_n, 3, &allocator);
    CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &full) == XUI_OK);
    equivalent(after, full);memory_oracle_check(d);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &undo) == XUI_OK);
    equivalent(undo, before);memory_oracle_check(d);xuiDocumentSnapshotRelease(undo);
    CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);memory_oracle_check(d);exact_forest(before->state);exact_forest(after->state);
    xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentSnapshotRelease(full);
    xuiDocumentRelease(d); xuiDocumentRelease(oracle); CHECK(!allocator.live); free(expected);
}
#ifndef DOC_CST_TRANSITION_NO_MAIN
int main(void)
{
    unsigned fixture, eol, bom, op, mode; uint64_t total = 0;
    for (fixture = 0; fixture < sizeof(transitions)/sizeof(*transitions); fixture++)
    for (eol = 0; eol < 3; eol++) for (bom = 0; bom < 2; bom++) {
        uint64_t n, at; char* source = source_variant(transitions[fixture], eol, bom, &n);
        for (at = bom ? 3 : 0; at < n; at++) {
            if (((unsigned char)source[at] & 0xc0) == 0x80) continue;
            for (op = 0; op < 3; op++) for (mode = 0; mode < 4; mode++) {
                fprintf(stderr, "Integrated CST transition fixture=%u eol=%u bom=%u at=%llu op=%u mode=%u\n", fixture, eol, bom, (unsigned long long)at, op, mode);
                transition_case(source, n, at, op, mode); total++;
            }
        }
        free(source);
    }
    printf("Integrated CST transitions: %llu source insert/delete/replace and Prepare cases; global/duplicate definitions, nested dependencies/containers/table/UTF8, exact forest/all public source records and bindings/byte tokens/HTML/full parse/Undo/Redo, complete ordinary projection, zero leaks\n", (unsigned long long)total);
    return 0;
}
#endif
