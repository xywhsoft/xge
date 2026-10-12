#define main collector_matrix_main
#include "xui_document_cst_collector_test.c"
#undef main
typedef struct cancel_allocator{fault_allocator base;long at;xui_document document;int result;}cancel_allocator;
static void* cancel_alloc(void* user,size_t bytes)
{
    cancel_allocator* a=user;
    if(a->document && a->at==0){a->at=-1;a->result=xuiDocumentCancelPrepare(a->document);}
    else if(a->at>0)a->at--;
    return fault_alloc(&a->base,bytes);
}
static void cancel_free(void* user,void* p){cancel_allocator* a=user;fault_free(&a->base,p);}
static xui_document fault_doc(cancel_allocator* a)
{
    const char* source="prior\n\n[^saved]: **old**\n";xui_doc_desc_t desc={0};xui_document d;
    desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.iMarkdownDialect=3;
    desc.onAlloc=cancel_alloc;desc.onFree=cancel_free;desc.pAllocatorUser=a;
    CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,source,strlen(source))==XUI_OK);
    CHECK(xuiDocumentClearHistory(d)==XUI_OK);return d;
}
static void unchanged(xui_document d,xui_document_snapshot saved,uint64_t id,uint64_t revision,unsigned live,cancel_allocator* a)
{
    xui_doc_stats_t stats={0};stats.iSize=sizeof(stats);
    CHECK(d->state==saved->state && xuiDocumentGetIdentity(d)==id && xuiDocumentGetRevision(d)==revision && !xuiDocumentIsDirty(d) && a->base.live==live);
    CHECK(xuiDocumentGetStats(d,&stats)==XUI_OK && !stats.iUndoCount && !stats.iRedoCount && check_forest(saved->state)==1);
}
#ifndef DOC_CST_FAULT_HELPERS_ONLY
int main(void)
{
    unsigned fixtures[]={0,3,5,8,14},i;long load_failures=0,prepare_failures=0,cancel_points=0;
    for(i=0;i<sizeof(fixtures)/sizeof(*fixtures);i++){
        cancel_allocator a={{-1,0},-1,NULL,0};xui_document d=fault_doc(&a);xui_document_snapshot saved;
        const char* source=cases[fixtures[i]];uint64_t id=xuiDocumentGetIdentity(d),revision=xuiDocumentGetRevision(d);unsigned live;long point;int succeeded=0;
        CHECK(xuiDocumentAcquireSnapshot(d,&saved)==XUI_OK && xuiDocumentMarkSaved(d,saved)==XUI_OK);live=a.base.live;
        for(point=0;point<3000;point++){
            int result;a.base.remaining=point;result=xuiDocumentLoadMarkdown(d,source,strlen(source));a.base.remaining=-1;
            if(result==XUI_OK){CHECK(check_forest(d->state));load_failures+=point;succeeded=1;break;}
            CHECK(result==XUI_ERROR_OUT_OF_MEMORY);unchanged(d,saved,id,revision,live,&a);
        }
        CHECK(succeeded);xuiDocumentSnapshotRelease(saved);xuiDocumentRelease(d);CHECK(!a.base.live);
        for(unsigned mode=0;mode<2;mode++){
            cancel_allocator b={{-1,0},-1,NULL,0};xui_document current=fault_doc(&b);xui_document_prepare p;
            xui_doc_source_patch_t patch={0};xui_document_snapshot before;uint64_t n=doc_seq_size(current->state->source);
            CHECK(xuiDocumentAcquireSnapshot(current,&before)==XUI_OK && xuiDocumentMarkSaved(current,before)==XUI_OK);
            id=xuiDocumentGetIdentity(current);revision=xuiDocumentGetRevision(current);live=b.base.live;patch.iSize=sizeof(patch);
            patch.iEnd=n;patch.sText=source;patch.iTextBytes=strlen(source);succeeded=0;
            for(point=0;point<3000;point++){
                int result;CHECK(xuiDocumentPrepareSource(current,NULL,&patch,1,&p)==XUI_OK);
                if(mode){b.document=current;b.at=point;b.result=XUI_ERROR_UNSUPPORTED;}else b.base.remaining=point;
                result=xuiDocumentPrepareRun(p);b.base.remaining=-1;b.document=NULL;b.at=-1;
                if(result==XUI_OK){CHECK(xuiDocumentCancelPrepare(current)==XUI_OK);succeeded=1;
                    if(mode)cancel_points+=point;else prepare_failures+=point;}
                else{CHECK(result==(mode?XUI_DOC_ERROR_CANCELLED:XUI_ERROR_OUT_OF_MEMORY));
                    if(mode)CHECK(b.result==XUI_OK);else CHECK(xuiDocumentCancelPrepare(current)==XUI_OK);}
                xuiDocumentPrepareRelease(p);unchanged(current,before,id,revision,live,&b);if(succeeded)break;
            }
            CHECK(succeeded);xuiDocumentSnapshotRelease(before);xuiDocumentRelease(current);CHECK(!b.base.live);
        }
    }
    printf("Hidden CST faults: %ld Load OOM, %ld Prepare Run OOM, %ld allocation-time cancellation checkpoints; existing nonempty arena/source/identity/revision/savepoint/history unchanged, snapshots survive and zero leaks\n",load_failures,prepare_failures,cancel_points);return 0;
}
#endif
