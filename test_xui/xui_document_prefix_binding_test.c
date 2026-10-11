/* Independent raw-prefix goldens. The shared source-index suite supplies
 * public-API partition, linear-query, native, lifetime and fault helpers. */
#define main source_index_suite_main
#include "xui_document_source_index_test.c"
#undef main

typedef struct prefix_expected {
    unsigned origin;
    uint64_t start, end, first, marker, content, last;
} prefix_expected;
typedef struct prefix_fixture {
    const char* source;
    unsigned footnote, count;
    prefix_expected expected[4];
} prefix_fixture;
#define Q(a,z,s,m,c,e) {XUI_DOC_SOURCE_BINDING_QUOTE_INDENT,a,z,s,m,c,e}
#define L(a,z,s,c,e) {XUI_DOC_SOURCE_BINDING_LIST_FIRST_INDENT,a,z,s,UINT64_MAX,c,e}
static const prefix_fixture prefix_cases[] = {
    {"> text\n",0,1,{Q(0,2,0,0,2,2)}},
    {" > text\n",0,1,{Q(0,3,0,1,3,3)}},
    {"  > text\n",0,1,{Q(0,4,0,2,4,4)}},
    {"   > text\n",0,1,{Q(0,5,0,3,5,5)}},
    {"- text\n",0,1,{L(0,2,0,2,2)}},
    {" - text\n",0,1,{L(0,3,0,3,3)}},
    {"  - text\n",0,1,{L(0,4,0,4,4)}},
    {"   - text\n",0,1,{L(0,5,0,5,5)}},
    /* On an opener line the accepted parent prefix includes its scanned
     * post-marker indentation; explicit continuation uses the optional one
     * column, so the child can start inside that same raw Tab there. */
    {">   - text\n",0,2,{Q(0,4,0,0,2,4),L(4,6,4,6,6)}},
    {"-   > text\n",0,2,{L(0,4,0,4,4),Q(4,6,4,4,6,6)}},
    {">  > text\n",0,2,{Q(0,3,0,0,2,3),Q(3,5,3,3,5,5)}},
    {"> \t- text\n",0,2,{Q(0,3,0,0,2,4),L(3,5,4,6,6)}},
    {">\t- text\n",0,2,{Q(0,2,0,0,2,4),L(2,4,4,6,6)}},
    {">\t> text\n",0,2,{Q(0,2,0,0,2,4),Q(2,4,4,4,6,6)}},
    {">  > a\n>  > b\n",0,4,{Q(0,3,0,0,2,3),Q(3,5,3,3,5,5),Q(7,10,0,0,2,3),Q(9,12,2,3,5,5)}},
    {"> > a\n>\t> b\n",0,4,{Q(0,2,0,0,2,2),Q(2,4,2,2,4,4),Q(6,8,0,0,2,4),Q(7,10,2,4,6,6)}},
    {"> - a\n>\t- b\n",0,4,{Q(0,2,0,0,2,2),L(2,4,2,4,4),Q(6,8,0,0,2,4),L(8,10,4,6,6)}},
    {"> z\n>\t- b\n",0,3,{Q(0,2,0,0,2,2),Q(4,6,0,0,2,4),L(5,8,2,6,6)}},
    {"  - a\n   - b\n",0,2,{L(0,4,0,4,4),L(6,11,0,5,5)}},
    {"  1. a\n   2. b\n",0,2,{L(0,5,0,5,5),L(7,13,0,6,6)}},
    /* Colon whitespace and removed continuation indentation belong to the
     * footnote definition, not to the body's quote/list container. */
    {"u[^f]\n\n[^f]:   > text\n",1,1,{Q(15,17,0,0,2,2)}},
    {"u[^f]\n\n[^f]: body\n\n      > text\n",1,1,{Q(23,27,0,2,4,4)}},
    {"u[^f]\n\n[^f]: body\n\n      - text\n",1,1,{L(23,27,0,4,4)}},
    {"u[^f]\n\n[^f]: >\t- text\n",1,2,{Q(13,15,0,0,2,4),L(15,17,4,6,6)}},
    {"u[^f]\n\n[^f]: body\n\n    >\t- text\n",1,2,{Q(23,25,0,0,2,4),L(25,27,4,6,6)}}
};
#undef Q
#undef L

static uint64_t prefix_offset(const char* source, uint64_t offset, unsigned ending, unsigned bom, unsigned shift)
{
    uint64_t i, result = offset + bom * 3 + shift;
    if (ending == 1) for (i = 0; i < offset; i++) if (source[i] == '\n') result++;
    return result;
}
static void prefix_oracle(xui_document_source_index index, const prefix_fixture* fixture,
    unsigned ending, unsigned bom, unsigned shift)
{
    xui_doc_source_index_info_t info = {0}; unsigned seen[4] = {0}; uint64_t i;
    info.iSize = sizeof(info); CHECK(xuiDocumentSourceIndexGetInfo(index, &info) == XUI_OK);
    for (i = 0; i < info.iBindingCount; i++) {
        xui_doc_source_binding_t b = {0}; unsigned j, match = 0; b.iSize = sizeof(b);
        CHECK(xuiDocumentSourceIndexGetBinding(index, i, &b) == XUI_OK);
        if (b.iOriginKind != XUI_DOC_SOURCE_BINDING_QUOTE_INDENT && b.iOriginKind != XUI_DOC_SOURCE_BINDING_LIST_FIRST_INDENT) continue;
        for (j = 0; j < fixture->count; j++) {
            const prefix_expected* e = fixture->expected + j;
            if (b.iOriginKind != e->origin || b.tRange.iSourceStart != prefix_offset(fixture->source, e->start, ending, bom, shift) ||
                b.tRange.iSourceEnd != prefix_offset(fixture->source, e->end, ending, bom, shift)) continue;
            CHECK(b.iMetadata == XUI_DOC_SOURCE_BINDING_COLUMNS && b.iStartColumn == e->first && b.iMarkerColumn == e->marker &&
                b.iContentColumn == e->content && b.iEndColumn == e->last && (b.tRange.iFlags & XUI_DOC_SOURCE_TOKEN_PARSER_CONFIRMED));
            CHECK(!seen[j]); seen[j]++; match++;
        }
        if (match != 1) fprintf(stderr,"unmatched prefix origin=%u raw=%llu..%llu columns=%llu/%llu/%llu/%llu source=%s\n",b.iOriginKind,
            (unsigned long long)b.tRange.iSourceStart,(unsigned long long)b.tRange.iSourceEnd,(unsigned long long)b.iStartColumn,
            (unsigned long long)b.iMarkerColumn,(unsigned long long)b.iContentColumn,(unsigned long long)b.iEndColumn,fixture->source);
        CHECK(match == 1);
    }
    for (i = 0; i < fixture->count; i++) CHECK(seen[i] == 1);
}
static void prefix_matrix(void)
{
    unsigned fixture, dialect, ending, bom, native, mode, cases = 0;
    const char* endings[] = {"\n","\r\n","\r"};
    for (fixture = 0; fixture < sizeof(prefix_cases)/sizeof(*prefix_cases); fixture++)
    for (dialect = prefix_cases[fixture].footnote ? 3 : 1; dialect <= 3; dialect++)
    for (ending = 0; ending < 3; ending++) for (bom = 0; bom < 2; bom++)
    for (native = 0; native < 2; native++) for (mode = 0; mode < 2; mode++) {
        const prefix_fixture* f = prefix_cases + fixture; char source[1024], shifted[1024]; uint64_t i, size = 0, length;
        fail_allocator allocator = {-1,0}; xui_document d, fresh; xui_document_snapshot before, after, expected;
        xui_document_source_index original, changed, full; char* serialized; unsigned live;
        if (bom) {memcpy(source,"\xef\xbb\xbf",3); size = 3;}
        for (i = 0; f->source[i]; i++) {
            if (f->source[i] == '\n') {memcpy(source+size,endings[ending],strlen(endings[ending]));size += strlen(endings[ending]);}
            else source[size++] = f->source[i];
        }
        source[size] = 0; d = open_source(source,size,dialect,&allocator);
        if (native) {
            CHECK(xuiDocumentAcquireSnapshot(d,&before) == XUI_OK && xuiDocumentSerialize(before,&serialized,&length) == XUI_OK);
            xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); CHECK(!allocator.live);
            {xui_doc_desc_t desc = {0};desc.iSize = sizeof(desc);desc.onAlloc = failing_alloc;desc.onFree = failing_free;desc.pAllocatorUser = &allocator;
                CHECK(xuiDocumentDeserialize(&desc,serialized,length,&d) == XUI_OK);}
            xuiDocumentFreeBuffer(serialized);
        }
        CHECK(xuiDocumentAcquireSnapshot(d,&before) == XUI_OK && xuiDocumentMarkSaved(d,before) == XUI_OK);
        original = make_index(before); live = allocator.live; allocator.remaining = 0;
        prefix_oracle(original,f,ending,bom,0); index_check(original,source,size,1);
        CHECK(allocator.live == live); allocator.remaining = -1;
        if (!mode) {
            xui_document_transaction txn; xui_doc_txn_desc_t td = {0}; td.iSize=sizeof(td);td.iDomain=XUI_DOC_SOURCE;
            CHECK(xuiDocumentBeginTransaction(d,&td,&txn) == XUI_OK && xuiDocumentTxnReplaceSource(txn,bom*3,bom*3,"prior\n\n",7) == XUI_OK &&
                xuiDocumentTxnCommit(txn,NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
        } else {
            xui_doc_source_patch_t patch = {0};xui_document_prepare prepare;patch.iSize=sizeof(patch);patch.iStart=patch.iEnd=bom*3;
            patch.sText="prior\n\n";patch.iTextBytes=7;
            CHECK(xuiDocumentPrepareSource(d,NULL,&patch,1,&prepare) == XUI_OK && xuiDocumentPrepareRun(prepare) == XUI_OK &&
                xuiDocumentPreparePublish(d,prepare,NULL) == XUI_OK);xuiDocumentPrepareRelease(prepare);
        }
        memcpy(shifted,source,bom*3);memcpy(shifted+bom*3,"prior\n\n",7);memcpy(shifted+bom*3+7,source+bom*3,(size_t)(size-bom*3)+1);
        CHECK(xuiDocumentAcquireSnapshot(d,&after) == XUI_OK);changed=make_index(after);prefix_oracle(changed,f,ending,bom,7);
        fresh=open_source(shifted,size+7,dialect,NULL);CHECK(xuiDocumentAcquireSnapshot(fresh,&expected) == XUI_OK);full=make_index(expected);
        compare_indices(changed,full);index_check(changed,shifted,size+7,1);
        xuiDocumentSourceIndexRelease(full);xuiDocumentSnapshotRelease(expected);
        CHECK(xuiDocumentUndo(d,NULL) == XUI_OK && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d,&expected) == XUI_OK);
        full=make_index(expected);compare_indices(original,full);prefix_oracle(full,f,ending,bom,0);
        CHECK(xuiDocumentRedo(d,NULL) == XUI_OK);prefix_oracle(original,f,ending,bom,0);
        xuiDocumentSourceIndexRelease(original);xuiDocumentSourceIndexRelease(changed);xuiDocumentSourceIndexRelease(full);
        xuiDocumentSnapshotRelease(before);xuiDocumentSnapshotRelease(after);xuiDocumentSnapshotRelease(expected);
        xuiDocumentRelease(d);xuiDocumentRelease(fresh);CHECK(!allocator.live);cases++;
    }
    printf("Prefix bindings golden: %u independent raw-range/column/BOM/EOL/native/SOURCE/Prepare/fresh-parse/Undo cases; shared Tabs and dedented footnotes; no query allocations\n",cases);
}
static void prefix_differential(void)
{
    const unsigned selected[] = {2,6,15,18,19,21,23,24};
    const char* replacements[] = {""," ","\t",">","-","\n","a","  "};
    const char* endings[] = {"\n","\r\n","\r"};
    unsigned fixture,ending,bom,mode,replacement,erase,cases=0;
    for(fixture=0;fixture<sizeof(selected)/sizeof(*selected);fixture++)
    for(ending=0;ending<3;ending++)for(bom=0;bom<2;bom++)for(mode=0;mode<2;mode++) {
        const prefix_fixture* f=prefix_cases+selected[fixture];char source[1024],changed[1024];unsigned char positions[1024]={0};
        uint64_t i,j,size=0; xui_document d; xui_document_snapshot baseline; xui_document_source_index original;
        if(bom){memcpy(source,"\xef\xbb\xbf",3);size=3;}
        for(i=0;f->source[i];i++) {
            if(f->source[i]=='\n'){memcpy(source+size,endings[ending],strlen(endings[ending]));size+=strlen(endings[ending]);}
            else source[size++]=f->source[i];
        }
        source[size]=0;
        for(i=0;i<f->count;i++) {
            uint64_t start=prefix_offset(f->source,f->expected[i].start,ending,bom,0);
            uint64_t end=prefix_offset(f->source,f->expected[i].end,ending,bom,0);
            if(start>bom*3)start--;
            for(j=start;j<=end+1 && j<=size;j++)positions[j]=1;
        }
        d=open_source(source,size,3,NULL);CHECK(xuiDocumentAcquireSnapshot(d,&baseline)==XUI_OK && xuiDocumentMarkSaved(d,baseline)==XUI_OK);
        original=make_index(baseline);
        for(i=bom*3;i<=size;i++)if(positions[i])
        for(replacement=0;replacement<sizeof(replacements)/sizeof(*replacements);replacement++)for(erase=0;erase<2;erase++) {
            uint64_t remove=erase && i<size, add=strlen(replacements[replacement]), revision=xuiDocumentGetRevision(d);
            xui_document fresh; xui_document_snapshot actual,expected; xui_document_source_index a,e;
            if(!remove && !add)continue;
            memcpy(changed,source,(size_t)i);memcpy(changed+i,replacements[replacement],(size_t)add);
            memcpy(changed+i+add,source+i+remove,(size_t)(size-i-remove)+1);
            if(!mode) {
                xui_document_transaction txn;xui_doc_txn_desc_t desc={0};desc.iSize=sizeof(desc);desc.iDomain=XUI_DOC_SOURCE;
                CHECK(xuiDocumentBeginTransaction(d,&desc,&txn)==XUI_OK && xuiDocumentTxnReplaceSource(txn,i,i+remove,replacements[replacement],add)==XUI_OK &&
                    xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
            } else {
                xui_doc_source_patch_t patch={0};xui_document_prepare prepare;patch.iSize=sizeof(patch);patch.iStart=i;patch.iEnd=i+remove;
                patch.sText=replacements[replacement];patch.iTextBytes=add;
                CHECK(xuiDocumentPrepareSource(d,NULL,&patch,1,&prepare)==XUI_OK && xuiDocumentPrepareRun(prepare)==XUI_OK &&
                    xuiDocumentPreparePublish(d,prepare,NULL)==XUI_OK);xuiDocumentPrepareRelease(prepare);
            }
            CHECK(xuiDocumentAcquireSnapshot(d,&actual)==XUI_OK);a=make_index(actual);index_check(a,changed,size-remove+add,1);
            fresh=open_source(changed,size-remove+add,3,NULL);CHECK(xuiDocumentAcquireSnapshot(fresh,&expected)==XUI_OK);e=make_index(expected);compare_indices(a,e);
            xuiDocumentSourceIndexRelease(a);xuiDocumentSourceIndexRelease(e);xuiDocumentSnapshotRelease(actual);xuiDocumentSnapshotRelease(expected);xuiDocumentRelease(fresh);
            if(xuiDocumentGetRevision(d)!=revision)CHECK(xuiDocumentUndo(d,NULL)==XUI_OK);
            CHECK(!xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d,&actual)==XUI_OK);a=make_index(actual);compare_indices(original,a);
            xuiDocumentSourceIndexRelease(a);xuiDocumentSnapshotRelease(actual);cases++;
        }
        xuiDocumentSourceIndexRelease(original);xuiDocumentSnapshotRelease(baseline);xuiDocumentRelease(d);
    }
    printf("Prefix bindings differential: %u insert/replace/remove prefix bytes, SOURCE/Prepare, full raw/record/token/binding/query and Undo oracle cases\n",cases);
}
static void prefix_parse_faults(void)
{
    const unsigned selected[]={2,6,15,18,21,24};unsigned i;long total=0;
    for(i=0;i<sizeof(selected)/sizeof(*selected);i++) {
        const prefix_fixture* f=prefix_cases+selected[i];fail_allocator allocator={-1,0};xui_document d=open_source("kept\n",5,3,&allocator);
        xui_document_snapshot before;xui_document_source_index original;long point;unsigned live;uint64_t revision;int succeeded=0;
        CHECK(xuiDocumentAcquireSnapshot(d,&before)==XUI_OK && xuiDocumentMarkSaved(d,before)==XUI_OK);original=make_index(before);
        live=allocator.live;revision=xuiDocumentGetRevision(d);
        for(point=0;point<2000;point++) {
            int result; xui_document_snapshot current;xui_document_source_index index;
            allocator.remaining=point;result=xuiDocumentLoadMarkdown(d,f->source,strlen(f->source));allocator.remaining=-1;
            CHECK(xuiDocumentAcquireSnapshot(d,&current)==XUI_OK);index=make_index(current);
            if(result==XUI_OK){prefix_oracle(index,f,0,0,0);succeeded=1;total+=point;}
            else CHECK(result==XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d)==revision && !xuiDocumentIsDirty(d));
            if(!succeeded)compare_indices(original,index);
            xuiDocumentSourceIndexRelease(index);xuiDocumentSnapshotRelease(current);
            if(succeeded)break;
            CHECK(allocator.live==live);
        }
        CHECK(succeeded);xuiDocumentSourceIndexRelease(original);xuiDocumentSnapshotRelease(before);xuiDocumentRelease(d);CHECK(!allocator.live);
    }
    printf("Prefix parser faults: %ld OOM checkpoints, retained source/tree/index/revision/savepoint atomic and zero leaks\n",total);
}
int main(int argc,char** argv)
{
    if (argc == 2 && !strcmp(argv[1],"--transport")) return transport();
    if (argc == 2 && !strcmp(argv[1],"--differential")) {prefix_differential();return 0;}
    prefix_matrix();
    if (argc == 2 && !strcmp(argv[1],"--prefix")) return 0;
    prefix_differential();prefix_parse_faults();
    return source_index_suite_main(0,NULL);
}
