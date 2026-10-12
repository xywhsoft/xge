#define main uninvoked_core_main
#include "xui_document_test.c"
#undef main
typedef struct intern_fault{fail_allocator base;long cancel_at;atomic_int cancellation;}intern_fault;
static void* intern_alloc(void* user,size_t size)
{
    intern_fault* a=user;if(a->cancel_at==0){a->cancel_at=-1;atomic_store(&a->cancellation,XUI_DOC_ERROR_CANCELLED);}
    else if(a->cancel_at>0)a->cancel_at--;
    return failing_alloc(&a->base,size);
}
static void intern_free(void* user,void* p){intern_fault* a=user;failing_free(&a->base,p);}
static doc_state* intern_state(xui_document d)
{
    const char* source="aa\n\nbb\n\ncc\n\n";doc_state* s=doc_state_clone(d->state);doc_blob* blob;
    CHECK(s);blob=doc_blob_new(d->allocator,source,12);CHECK(blob);
    doc_seq_release(s->source);s->source=doc_seq_blob_range(d->allocator,blob,0,12);doc_blob_release(blob);CHECK(s->source);return s;
}
static void intern_records(doc_source_cst_record* r,int distinct)
{
    unsigned i;memset(r,0,8*sizeof(*r));r[0].kind=DOC_CST_SCOPE;r[0].parent=DOC_NONE;r[0].end=12;
    r[1].kind=DOC_CST_BLOCK;r[1].detail=MD_BLOCK_DOC;r[1].end=12;
    for(i=0;i<3;i++){
        r[2+i*2].kind=DOC_CST_BLOCK;r[2+i*2].detail=MD_BLOCK_P;r[2+i*2].parent=1;r[2+i*2].start=i*4;r[2+i*2].end=i*4+3;
        r[3+i*2].kind=DOC_CST_TEXT;r[3+i*2].detail=distinct && i==2?MD_TEXT_CODE:MD_TEXT_NORMAL;
        r[3+i*2].parent=2+i*2;r[3+i*2].start=i*4;r[3+i*2].end=i*4+2;
    }
}
static void intern_golden(doc_state* s,int distinct)
{
    unsigned i;memory_oracle graph={0};unsigned blobs=0;
    CHECK(doc_source_forest_count(s)==3);
    CHECK(doc_seq_get_value_item(s->source_syntax,0)==doc_seq_get_value_item(s->source_syntax,1));
    CHECK((doc_seq_get_value_item(s->source_syntax,0)==doc_seq_get_value_item(s->source_syntax,2))==!distinct);
    for(i=0;i<3;i++){
        doc_source_cst_record r,e={0};doc_source_cst_header h;uint64_t start;
        CHECK(doc_source_forest_info(s,i,&start,&h)==XUI_OK && start==i*4 && h.count==3 && h.source_bytes==3);
        e.kind=DOC_CST_SCOPE;e.detail=MD_BLOCK_P;e.parent=DOC_NONE;e.end=3;
        CHECK(doc_source_forest_read(s,i,0,&r)==XUI_OK && !memcmp(&r,&e,sizeof(r)));
        e.kind=DOC_CST_BLOCK;e.parent=0;
        CHECK(doc_source_forest_read(s,i,1,&r)==XUI_OK && !memcmp(&r,&e,sizeof(r)));
        e.kind=DOC_CST_TEXT;e.detail=distinct && i==2?MD_TEXT_CODE:MD_TEXT_NORMAL;e.parent=1;e.end=2;
        CHECK(doc_source_forest_read(s,i,2,&r)==XUI_OK && !memcmp(&r,&e,sizeof(r)));
    }
    memory_oracle_visit(&graph,s->source_syntax,1);
    for(i=0;i<graph.count;i++){doc_allocation* a=(doc_allocation*)graph.entries[i].pointer-1;if(a->value.kind==DOC_MEMORY_BLOB)blobs++;}
    CHECK(blobs==(distinct?2u:1u));free(graph.entries);
}
int main(void)
{
    intern_fault a={{-1,0},-1,ATOMIC_VAR_INIT(0)};xui_doc_desc_t desc={0};xui_document d;doc_state* state;
    doc_source_cst_record records[8];uint64_t id;unsigned mode;long totals[2]={0};
    desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;desc.bDisableHistory=1;
    desc.onAlloc=intern_alloc;desc.onFree=intern_free;desc.pAllocatorUser=&a;
    CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK);state=intern_state(d);id=atomic_load(&d->allocator->next_node_id);
    intern_records(records,0);CHECK(doc_source_forest_build(state,records,8,12,&a.cancellation)==XUI_OK);intern_golden(state,0);
    intern_records(records,1);CHECK(doc_source_forest_build(state,records,8,12,&a.cancellation)==XUI_OK);intern_golden(state,1);
    for(mode=0;mode<2;mode++){
        int succeeded=0;doc_sequence* old=state->source_syntax;unsigned live=a.base.live;
        intern_records(records,0);
        for(long point=0;point<300;point++){
            int result;if(mode)a.cancel_at=point;else a.base.remaining=point;
            result=doc_source_forest_build(state,records,8,12,&a.cancellation);a.base.remaining=a.cancel_at=-1;atomic_store(&a.cancellation,XUI_OK);
            if(result==XUI_OK){totals[mode]=point;succeeded=1;intern_golden(state,0);break;}
            CHECK(result==(mode?XUI_DOC_ERROR_CANCELLED:XUI_ERROR_OUT_OF_MEMORY));CHECK(state->source_syntax==old && a.base.live==live);intern_golden(state,1);
        }
        CHECK(succeeded);
        intern_records(records,1);CHECK(doc_source_forest_build(state,records,8,12,&a.cancellation)==XUI_OK);intern_golden(state,1);
    }
    CHECK(atomic_load(&d->allocator->next_node_id)==id);doc_state_release(state);xuiDocumentRelease(d);CHECK(!a.base.live);
    printf("Relative grammar interning: different raw text/base shares exactly one arena; distinct grammar retained as two arenas; full independent record golden and graph; %ld OOM/%ld cancellation points atomic; no NodeIds or leaks\n",totals[0],totals[1]);return 0;
}
