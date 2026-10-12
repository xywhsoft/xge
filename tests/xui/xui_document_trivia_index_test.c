#define main prior_public_body_main
#include "xui_document_body_index_test.c"
#undef main
typedef struct public_trivia_golden { uint32_t kind;uint64_t start,end; } public_trivia_golden;
static char* public_trivia_source(const char* source,unsigned eol,unsigned bom)
{
    size_t i,n=strlen(source),used=0;char* bytes=malloc(n*2+4);BODY_CHECK(bytes);
    if(bom){memcpy(bytes,"\xef\xbb\xbf",3);used=3;}
    for(i=0;i<n;i++){
        if(source[i]=='\n'){if(eol)bytes[used++]='\r';if(eol!=2)bytes[used++]='\n';}else bytes[used++]=source[i];
    }
    bytes[used]=0;return bytes;
}
static uint64_t public_trivia_offset(const char* original,uint64_t offset,unsigned eol,unsigned bom)
{
    uint64_t i,out=bom?3:0;for(i=0;i<offset;i++)out+=original[i]=='\n' && eol==1?2:1;return out;
}
static void public_trivia_owned(xui_document_source_index index,const public_trivia_golden* expected)
{
    xui_doc_source_index_info_t info={0};uint64_t i,hits=0;info.iSize=sizeof(info);
    BODY_CHECK(xuiDocumentSourceIndexGetInfo(index,&info)==XUI_OK);
    for(i=0;i<info.iBindingCount;i++){
        xui_doc_source_binding_t b={0};xui_doc_source_record_t r={0},parent={0};b.iSize=sizeof(b);r.iSize=sizeof(r);parent.iSize=sizeof(parent);
        BODY_CHECK(xuiDocumentSourceIndexGetBinding(index,i,&b)==XUI_OK);
        if(b.iOriginKind!=XUI_DOC_SOURCE_BINDING_DOCUMENT_SYNTAX || b.tRange.iKind!=expected->kind ||
            b.tRange.iSourceStart!=expected->start || b.tRange.iSourceEnd!=expected->end)continue;
        BODY_CHECK(xuiDocumentSourceIndexGetRecord(index,b.tRange.iRecordIndex,&r)==XUI_OK &&
            r.iKind==XUI_DOC_SOURCE_RECORD_SYNTAX && (r.iFlags&XUI_DOC_SOURCE_RECORD_SYNTAX_DOCUMENT) && !r.iNodeId &&
            r.iDetail==(expected->kind==XUI_DOC_SOURCE_TOKEN_HORIZONTAL_SPACE?XUI_DOC_SOURCE_SYNTAX_FIELD:XUI_DOC_SOURCE_SYNTAX_BREAK));
        BODY_CHECK((b.tRange.iFlags&XUI_DOC_SOURCE_TOKEN_PARSER_CONFIRMED) && (b.iMetadata&XUI_DOC_SOURCE_BINDING_SOURCE_ONLY) &&
            !b.tRange.iNodeId && b.tRange.iTextStart==UINT64_MAX && b.tRange.iTextEnd==UINT64_MAX &&
            r.iSourceStart==expected->start && r.iSourceEnd==expected->end &&
            xuiDocumentSourceIndexGetRecord(index,r.iParentIndex,&parent)==XUI_OK &&
            parent.iSourceStart<=r.iSourceStart && parent.iSourceEnd>=r.iSourceEnd);
        hits++;
    }
    if(hits!=1)fprintf(stderr,"public trivia kind=%u range=%llu..%llu owned=%llu\n",expected->kind,
        (unsigned long long)expected->start,(unsigned long long)expected->end,(unsigned long long)hits);
    BODY_CHECK(hits==1);
}
static void public_trivia_goldens(void)
{
    const char* source[]={" \t\n\n", "#  heading  #  \n\n", "a b \t\n"};
    const public_trivia_golden first[]={{XUI_DOC_SOURCE_TOKEN_HORIZONTAL_SPACE,0,2},{XUI_DOC_SOURCE_TOKEN_LINE_ENDING,2,3},{XUI_DOC_SOURCE_TOKEN_LINE_ENDING,3,4}};
    const public_trivia_golden heading[]={{XUI_DOC_SOURCE_TOKEN_HORIZONTAL_SPACE,1,3},{XUI_DOC_SOURCE_TOKEN_HORIZONTAL_SPACE,10,12},
        {XUI_DOC_SOURCE_TOKEN_HORIZONTAL_SPACE,13,15},{XUI_DOC_SOURCE_TOKEN_LINE_ENDING,15,16},{XUI_DOC_SOURCE_TOKEN_LINE_ENDING,16,17}};
    const public_trivia_golden paragraph[]={{XUI_DOC_SOURCE_TOKEN_HORIZONTAL_SPACE,3,5},{XUI_DOC_SOURCE_TOKEN_LINE_ENDING,5,6}};
    const public_trivia_golden* cases[]={first,heading,paragraph};const unsigned counts[]={3,5,2};unsigned fixture,eol,bom;uint64_t total=0;
    for(fixture=0;fixture<3;fixture++)for(eol=0;eol<3;eol++)for(bom=0;bom<2;bom++){
        body_allocator allocator={-1,0};xui_doc_desc_t desc={0};xui_document d;xui_document_snapshot snapshot;
        xui_document_source_index index;xui_doc_stats_t before={0},after={0};uint64_t i;char* bytes=public_trivia_source(source[fixture],eol,bom);
        desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_EXTENDED;
        desc.onAlloc=body_alloc;desc.onFree=body_free;desc.pAllocatorUser=&allocator;if(!fixture)desc.iMaxNodes=1;
        BODY_CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,bytes,strlen(bytes))==XUI_OK &&
            xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentSnapshotCreateSourceIndex(snapshot,NULL,&index)==XUI_OK);
        before.iSize=after.iSize=sizeof(before);BODY_CHECK(xuiDocumentGetStats(d,&before)==XUI_OK);allocator.remaining=0;
        for(i=0;i<counts[fixture];i++){
            public_trivia_golden expected=cases[fixture][i];expected.start=public_trivia_offset(source[fixture],expected.start,eol,bom);
            expected.end=public_trivia_offset(source[fixture],expected.end,eol,bom);public_trivia_owned(index,&expected);total++;
        }
        body_partition(index,bytes);BODY_CHECK(xuiDocumentGetStats(d,&after)==XUI_OK && before.iNodes==after.iNodes &&
            before.iUndoCount==after.iUndoCount && before.iRedoCount==after.iRedoCount);allocator.remaining=-1;
        xuiDocumentRelease(d);body_partition(index,bytes);xuiDocumentSnapshotRelease(snapshot);xuiDocumentSourceIndexRelease(index);
        BODY_CHECK(!allocator.live);free(bytes);
    }
    printf("Public trivia: %llu independent exact owned ranges, confirmed roles/parents, all endings/BOM, no query allocations or semantic/history mutation and retained lifetime\n",(unsigned long long)total);
}
static void public_trivia_edits(void)
{
    const char* source[]={" \t\n\n", "#  heading  #  \n\n", "a b \t\n"};
    const char* expected[]={"  \n\n", "#   heading  #  \n\n", "a b \t \n"};
    const char* replacement[]={"  ","   "," \t "};const uint64_t starts[]={0,1,3},ends[]={2,3,5};unsigned fixture,eol,bom,mode,total=0;
    for(fixture=0;fixture<3;fixture++)for(eol=0;eol<3;eol++)for(bom=0;bom<2;bom++)for(mode=0;mode<3;mode++){
        body_allocator allocator={-1,0};xui_doc_desc_t desc={0};xui_document d,oracle;xui_document_snapshot before,after,full,undo;
        xui_document_source_index old,actual,wanted,restored;char* bytes=public_trivia_source(source[fixture],eol,bom);
        char* final=public_trivia_source(expected[fixture],eol,bom);uint64_t start=public_trivia_offset(source[fixture],starts[fixture],eol,bom);
        uint64_t end=public_trivia_offset(source[fixture],ends[fixture],eol,bom);
        desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=XUI_MD_EXTENDED;
        desc.onAlloc=body_alloc;desc.onFree=body_free;desc.pAllocatorUser=&allocator;
        BODY_CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,bytes,strlen(bytes))==XUI_OK &&
            xuiDocumentAcquireSnapshot(d,&before)==XUI_OK && xuiDocumentMarkSaved(d,before)==XUI_OK &&
            xuiDocumentSnapshotCreateSourceIndex(before,NULL,&old)==XUI_OK);
        if(!mode){
            xui_document_transaction t;xui_doc_txn_desc_t td={0};td.iSize=sizeof(td);td.iDomain=XUI_DOC_SOURCE;
            BODY_CHECK(xuiDocumentBeginTransaction(d,&td,&t)==XUI_OK && xuiDocumentTxnReplaceSource(t,start,end,replacement[fixture],strlen(replacement[fixture]))==XUI_OK &&
                xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
        }else{
            xui_document_prepare p;xui_doc_source_patch_t patch={0};patch.iSize=sizeof(patch);patch.iStart=start;patch.iEnd=end;
            patch.sText=replacement[fixture];patch.iTextBytes=strlen(replacement[fixture]);
            BODY_CHECK(xuiDocumentPrepareSource(d,NULL,&patch,1,&p)==XUI_OK && xuiDocumentPrepareRun(p)==XUI_OK);
            if(mode==2){
                xui_document_prepare next;xui_doc_source_patch_t extra={0};size_t n=strlen(final);char* expanded=realloc(final,n+5);BODY_CHECK(expanded);final=expanded;
                memcpy(final+n,"\n \t\n",5);extra.iSize=sizeof(extra);extra.iStart=extra.iEnd=n;extra.sText="\n \t\n";extra.iTextBytes=4;
                BODY_CHECK(xuiDocumentPrepareContinueSource(d,p,&extra,1,&next)==XUI_OK);xuiDocumentPrepareRelease(p);p=next;
                BODY_CHECK(xuiDocumentPrepareRun(p)==XUI_OK);
            }
            BODY_CHECK(xuiDocumentPreparePublish(d,p,NULL)==XUI_OK);xuiDocumentPrepareRelease(p);
        }
        BODY_CHECK(xuiDocumentAcquireSnapshot(d,&after)==XUI_OK && xuiDocumentSnapshotCreateSourceIndex(after,NULL,&actual)==XUI_OK);
        body_partition(actual,final);body_partition(old,bytes);
        BODY_CHECK(xuiDocumentCreate(&desc,&oracle)==XUI_OK && xuiDocumentLoadMarkdown(oracle,final,strlen(final))==XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle,&full)==XUI_OK && xuiDocumentSnapshotCreateSourceIndex(full,NULL,&wanted)==XUI_OK);body_same(actual,wanted);
        BODY_CHECK(xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d,&undo)==XUI_OK &&
            xuiDocumentSnapshotCreateSourceIndex(undo,NULL,&restored)==XUI_OK);body_same(restored,old);xuiDocumentSourceIndexRelease(restored);xuiDocumentSnapshotRelease(undo);
        BODY_CHECK(xuiDocumentRedo(d,NULL)==XUI_OK);xuiDocumentSourceIndexRelease(actual);xuiDocumentSourceIndexRelease(wanted);xuiDocumentSourceIndexRelease(old);
        xuiDocumentSnapshotRelease(before);xuiDocumentSnapshotRelease(after);xuiDocumentSnapshotRelease(full);xuiDocumentRelease(d);xuiDocumentRelease(oracle);
        BODY_CHECK(!allocator.live);free(bytes);free(final);total++;
    }
    printf("Public trivia edits: %u whitespace SOURCE/Prepare/READY Continue, exact raw bytes, all public indexes full-parse equal, Undo/Redo and zero leaks\n",total);
}
int main(void){public_trivia_goldens();public_trivia_edits();return 0;}
