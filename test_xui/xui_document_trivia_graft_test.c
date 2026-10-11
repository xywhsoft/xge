#define DOC_CST_TRANSITION_NO_MAIN
#include "xui_document_whole_cst_test.c"

static const char* graft_fixtures[]={
    "[^n]: unused\n\nalpha target\n\ntail\n",
    "before\n\n[^n]: unused\n\nalpha target\n\ntail\n",
    "alpha target\n\n[^n]: unused\n\ntail\n",
    "alpha target\n\ntail\n\n[^n]: unused\n",
    "before\n\nalpha target\n\n[^n]: unused\n\ntail\n",
    " \t\n\n[^n]: unused\n\nalpha target\n\n \t\ntail\n\n \t\n"
};
static char* graft_shape(const char* input,const char* ending,int bom)
{
    uint64_t i,at=bom?3:0,capacity=strlen(input)*2+4;char* source=malloc((size_t)capacity);CHECK(source);
    if(bom)memcpy(source,"\xef\xbb\xbf",3);
    for(i=0;input[i];i++){
        if(input[i]=='\n'){size_t bytes=strlen(ending);memcpy(source+at,ending,bytes);at+=bytes;}
        else source[at++]=input[i];
    }
    source[at]=0;return source;
}
static void graft_edit(const char* source,unsigned mode)
{
    fault_allocator f={-1,0};xui_document d=open_doc(source,strlen(source),3,&f),oracle;
    xui_document_snapshot before,after,full,restored;xui_doc_stats_t old_stats={0},stats={0};
    const char* target=strstr(source,"target");uint64_t at;char* edited;
    CHECK(target);at=(uint64_t)(target-source);edited=malloc(strlen(source)+1);CHECK(edited);
    memcpy(edited,source,(size_t)at);memcpy(edited+at,"[^n]",4);strcpy(edited+at+4,target+6);
    CHECK(xuiDocumentAcquireSnapshot(d,&before)==XUI_OK && xuiDocumentMarkSaved(d,before)==XUI_OK);
    old_stats.iSize=stats.iSize=sizeof(stats);CHECK(xuiDocumentGetStats(d,&old_stats)==XUI_OK);
    if(!mode){
        xui_document_transaction t;xui_doc_txn_desc_t desc={0};desc.iSize=sizeof(desc);desc.iDomain=XUI_DOC_SOURCE;
        CHECK(xuiDocumentBeginTransaction(d,&desc,&t)==XUI_OK && xuiDocumentTxnReplaceSource(t,at,at+6,"[^n]",4)==XUI_OK &&
            xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
    }else{
        xui_document_prepare p;xui_doc_source_patch_t patch={0};patch.iSize=sizeof(patch);patch.iStart=at;patch.iEnd=at+6;patch.sText="[^n]";patch.iTextBytes=4;
        CHECK(xuiDocumentPrepareSource(d,NULL,&patch,1,&p)==XUI_OK && xuiDocumentPrepareRun(p)==XUI_OK &&
            xuiDocumentPreparePublish(d,p,NULL)==XUI_OK);xuiDocumentPrepareRelease(p);
    }
    CHECK(xuiDocumentGetStats(d,&stats)==XUI_OK);
    if(stats.iMarkdownIncrementalParses-old_stats.iMarkdownIncrementalParses!=1)
        fprintf(stderr,"Graft incremental mode=%u delta=%llu source=[%s]\n",mode,
            (unsigned long long)(stats.iMarkdownIncrementalParses-old_stats.iMarkdownIncrementalParses),source);
    CHECK(stats.iMarkdownIncrementalParses-old_stats.iMarkdownIncrementalParses==1);
    oracle=open_doc(edited,strlen(edited),3,&f);CHECK(xuiDocumentAcquireSnapshot(d,&after)==XUI_OK &&
        xuiDocumentAcquireSnapshot(oracle,&full)==XUI_OK);equivalent(after,full);memory_oracle_check(d);
    CHECK(xuiDocumentUndo(d,NULL)==XUI_OK && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
    equivalent(restored,before);xuiDocumentSnapshotRelease(restored);memory_oracle_check(d);
    CHECK(xuiDocumentRedo(d,NULL)==XUI_OK && xuiDocumentAcquireSnapshot(d,&restored)==XUI_OK);
    equivalent(restored,after);xuiDocumentSnapshotRelease(restored);memory_oracle_check(d);
    xuiDocumentSnapshotRelease(before);xuiDocumentSnapshotRelease(after);xuiDocumentSnapshotRelease(full);
    xuiDocumentRelease(oracle);xuiDocumentRelease(d);CHECK(!f.live);free(edited);
}
typedef struct graft_fault {long remaining,cancel_at;unsigned live;atomic_int cancel;xui_document document;} graft_fault;
static void* graft_allocate(void* user,size_t bytes)
{
    graft_fault* f=user;void* p;
    if(f->cancel_at==0){f->cancel_at=-1;if(f->document)CHECK(xuiDocumentCancelPrepare(f->document)==XUI_OK);else atomic_store(&f->cancel,XUI_DOC_ERROR_CANCELLED);}
    else if(f->cancel_at>0)f->cancel_at--;
    if(!f->remaining)return NULL;
    if(f->remaining>0)f->remaining--;
    p=malloc(bytes);if(p)f->live++;return p;
}
static void graft_release(void* user,void* p)
{graft_fault* f=user;CHECK(f->live);f->live--;free(p);}
static void graft_primitives(void)
{
    const char* original="aa\r\n\r\nTAIL\r\n\r\nzz\r\n",*masked="aa\r\n\r\n    \r\n\r\nzz\r\n";
    unsigned mode;long faults[2]={0};
    for(mode=0;mode<2;mode++){
        graft_fault f={-1,-1,0,ATOMIC_VAR_INIT(0),NULL};xui_document d;xui_document_snapshot actual;
        xui_doc_desc_t desc={0};long point;
        desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=3;
        desc.onAlloc=graft_allocate;desc.onFree=graft_release;desc.pAllocatorUser=&f;
        CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,original,strlen(original))==XUI_OK &&
            xuiDocumentAcquireSnapshot(d,&actual)==XUI_OK && xuiDocumentLoadMarkdown(d,masked,strlen(masked))==XUI_OK);
        for(point=0;point<128;point++){
            doc_state* state=doc_state_clone(d->state);doc_sequence* prior;unsigned live;uint64_t ids;int result;CHECK(state);
            doc_seq_release(state->source);state->source=actual->state->source;doc_seq_retain(state->source);
            prior=state->source_syntax;live=f.live;ids=atomic_load(&state->allocator->next_node_id);
            CHECK(doc_source_forest_graft(state,actual->state,6,12,5,&f.cancel)==XUI_DOC_ERROR_UNREPRESENTABLE &&
                state->source_syntax==prior && f.live==live);
            CHECK(doc_source_forest_graft(state,actual->state,6,12,1,&f.cancel)==XUI_DOC_ERROR_UNREPRESENTABLE &&
                state->source_syntax==prior && f.live==live);
            CHECK(doc_source_forest_graft(state,actual->state,7,10,6,&f.cancel)==XUI_DOC_ERROR_UNREPRESENTABLE &&
                state->source_syntax==prior && f.live==live);
            CHECK(doc_source_forest_graft(state,actual->state,7,12,7,&f.cancel)==XUI_DOC_ERROR_UNREPRESENTABLE &&
                state->source_syntax==prior && f.live==live);
            CHECK(doc_source_forest_graft(state,actual->state,6,9,6,&f.cancel)==XUI_DOC_ERROR_UNREPRESENTABLE &&
                state->source_syntax==prior && f.live==live);
            if(mode)f.cancel_at=point;else f.remaining=point;
            result=doc_source_forest_graft(state,actual->state,6,12,6,&f.cancel);f.remaining=f.cancel_at=-1;atomic_store(&f.cancel,XUI_OK);
            CHECK(atomic_load(&state->allocator->next_node_id)==ids && state->node_count==d->state->node_count);
            if(result==XUI_OK){integrated_forest_equal(state,actual->state);doc_state_release(state);faults[mode]=point;break;}
            CHECK(result==(mode?XUI_DOC_ERROR_CANCELLED:XUI_ERROR_OUT_OF_MEMORY) && state->source_syntax==prior && f.live==live);
            doc_state_release(state);
        }
        CHECK(point<128);xuiDocumentSnapshotRelease(actual);xuiDocumentRelease(d);CHECK(!f.live);
    }
    printf("Trivia graft primitive: exact prefix/suffix grammar equals original, no semantic nodes/IDs, physical CRLF plus prior/destination ordinary grammar cuts rejected; %ld OOM/%ld cancellation checkpoints atomic and zero leaks\n",faults[0],faults[1]);
}
static char* graft_long_separator(unsigned width)
{
    const char* prefix="[^n]: unused\n\nalpha target\n\n",*middle="\ntail\n\n";
    size_t a=strlen(prefix),b=strlen(middle);unsigned i;char* source=malloc(a+b+2*width+2);CHECK(source);
    memcpy(source,prefix,a);for(i=0;i<width;i++)source[a+i]=i%3?' ':'\t';
    memcpy(source+a+width,middle,b);for(i=0;i<width;i++)source[a+width+b+i]=i%3?' ':'\t';
    source[a+2*width+b]='\n';source[a+2*width+b+1]=0;return source;
}
static void graft_prepare_faults(void)
{
    char* input=graft_long_separator(1025),*source=graft_shape(input,"\r\n",1);unsigned mode;long failures[2]={0};free(input);
    for(mode=0;mode<2;mode++){
        graft_fault f={-1,-1,0,ATOMIC_VAR_INIT(0),NULL};xui_doc_desc_t desc={0};xui_document d;xui_document_snapshot saved;
        uint64_t revision;unsigned live;long point;const char* target=strstr(source,"target");xui_doc_source_patch_t patch={0};
        desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=3;
        desc.onAlloc=graft_allocate;desc.onFree=graft_release;desc.pAllocatorUser=&f;
        CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,source,strlen(source))==XUI_OK &&
            xuiDocumentClearHistory(d)==XUI_OK && xuiDocumentAcquireSnapshot(d,&saved)==XUI_OK && xuiDocumentMarkSaved(d,saved)==XUI_OK);
        revision=xuiDocumentGetRevision(d);live=f.live;
        patch.iSize=sizeof(patch);patch.iStart=target-source;patch.iEnd=patch.iStart+6;patch.sText="[^n]";patch.iTextBytes=4;
        for(point=0;point<2000;point++){
            xui_document_prepare pending;xui_doc_stats_t stats={0};int result;
            CHECK(xuiDocumentPrepareSource(d,NULL,&patch,1,&pending)==XUI_OK);
            if(mode){f.document=d;f.cancel_at=point;}else f.remaining=point;
            result=xuiDocumentPrepareRun(pending);f.remaining=f.cancel_at=-1;f.document=NULL;
            if(result!=XUI_OK)CHECK(result==(mode?XUI_DOC_ERROR_CANCELLED:XUI_ERROR_OUT_OF_MEMORY));
            if(result==XUI_OK || !mode)CHECK(xuiDocumentCancelPrepare(d)==XUI_OK);
            xuiDocumentPrepareRelease(pending);stats.iSize=sizeof(stats);
            CHECK(d->state==saved->state && xuiDocumentGetRevision(d)==revision && !xuiDocumentIsDirty(d) && f.live==live &&
                xuiDocumentGetStats(d,&stats)==XUI_OK && !stats.iUndoCount && !stats.iRedoCount);
            memory_oracle_check(d);
            if(result==XUI_OK){failures[mode]=point;break;}
        }
        CHECK(point<2000);xuiDocumentSnapshotRelease(saved);xuiDocumentRelease(d);CHECK(!f.live);
    }
    free(source);printf("Trivia graft long-separator Prepare: %ld OOM/%ld cancellation checkpoints, saved source/tree/history/revision unchanged and zero leaks\n",failures[0],failures[1]);
}
int main(void)
{
    const char* endings[]={"\n","\r\n","\r"};unsigned i,e,b,m,count=0;
    CHECK(sizeof(transitions)/sizeof(*transitions)==18);
    for(i=0;i<sizeof(graft_fixtures)/sizeof(*graft_fixtures);i++)for(e=0;e<3;e++)for(b=0;b<2;b++)for(m=0;m<2;m++){
        char* source=graft_shape(graft_fixtures[i],endings[e],b);graft_edit(source,m);free(source);count++;
    }
    {unsigned widths[]={511,512,513,1025},w;
    for(w=0;w<4;w++)for(e=0;e<3;e++)for(b=0;b<2;b++)for(m=0;m<2;m++){
        char* input=graft_long_separator(widths[w]),*source=graft_shape(input,endings[e],b);graft_edit(source,m);free(source);free(input);count++;
    }}
    CHECK(count==120);graft_primitives();graft_prepare_faults();
    puts("Trivia graft: 120 SOURCE/Prepare first footnote uses (including 511/512/513/1025-byte separators), all endings/BOM and prefix/suffix placement; required incremental path, ordinary/definition syntax, all public source data, HTML, full parse, memory graph, Undo/Redo and zero leaks");return 0;
}
