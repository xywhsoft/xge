#define main uninvoked_core_main
#include "xui_document_test.c"
#undef main
#include "../src/xui_document_source.c"

static void workspace_release(doc_si_builder* b)
{
    doc_free(b->annotations);doc_free(b->events);doc_free(b->heap);doc_free(b->index->tokens);
}
static void workspace_partition(xui_document_snapshot snapshot,fail_allocator* f,int wide)
{
    long point;uint64_t size=wide?(uint64_t)UINT32_MAX+100:UINT32_MAX,middle=size-60;
    for(point=0;point<32;point++){
        struct xui_doc_source_index_t index={0};doc_si_builder b={0};int result;unsigned live=f->live;unsigned i;
        index.snapshot=snapshot;index.info.iSourceBytes=size;b.index=&index;b.limit=UINT64_MAX;
        b.annotation_count=b.annotation_capacity=3;b.annotations=doc_alloc(snapshot->state->allocator,3*sizeof(*b.annotations));CHECK(b.annotations);
        b.used=3*sizeof(*b.annotations)+sizeof(doc_allocation);
        for(i=0;i<3;i++){
            doc_si_annotation* a=b.annotations+i;a->iSourceStart=i==2?middle:0;a->iSourceEnd=i==1?middle:size;
            a->iRecordIndex=i;a->iOriginIndex=DOC_NONE;a->iKindDetail=XUI_DOC_SOURCE_TOKEN_CONTENT;a->maximum=i?100:0;
        }
        f->remaining=point;result=doc_si_partition(&b);f->remaining=-1;
        if(result==XUI_OK){
            doc_si_event event;CHECK(b.event_unit==(wide?sizeof(doc_si_event):sizeof(doc_si_event32)));
            CHECK(index.info.iTokenCount==2 && b.token_capacity==2 && b.event_capacity==6 && b.heap_capacity==3);
            CHECK(index.tokens[0].iSourceStart==0 && index.tokens[0].iSourceEnd==middle && index.tokens[0].iRecordIndex==1);
            CHECK(index.tokens[1].iSourceStart==middle && index.tokens[1].iSourceEnd==size && index.tokens[1].iRecordIndex==2);
            if(wide){doc_si_event_set(&b,0,size,UINT64_C(1)<<40);event=doc_si_event_get(&b,0);CHECK(event.offset==size && event.annotation_phase==(UINT64_C(1)<<40));}
            workspace_release(&b);CHECK(f->live==live);break;
        }
        CHECK(result==XUI_ERROR_OUT_OF_MEMORY);workspace_release(&b);CHECK(f->live==live);
    }
    CHECK(point<32);
}
int main(void)
{
    fail_allocator f={-1,0};xui_document d;xui_document_snapshot snapshot;xui_doc_desc_t desc={0};unsigned i;
    doc_si_event large[16];doc_si_event32 small[16];
    desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_RICH;desc.onAlloc=failing_alloc;desc.onFree=failing_free;desc.pAllocatorUser=&f;
    CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK && xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK);
    workspace_partition(snapshot,&f,0);workspace_partition(snapshot,&f,1);
    for(i=0;i<16;i++){
        large[i].offset=small[i].offset=i%3?UINT32_MAX:0;
        large[i].annotation_phase=small[i].annotation_phase=(15-i)*2+(i&1);
    }
    qsort(large,16,sizeof(*large),doc_si_event_compare);qsort(small,16,sizeof(*small),doc_si_event32_compare);
    for(i=0;i<16;i++)CHECK(large[i].offset==small[i].offset && large[i].annotation_phase==small[i].annotation_phase);
    puts("Source index workspace: narrow/wide metadata partition at UINT32 boundary, wide offset and 40-bit ordinal preserved, identical end/start ordering, exact phase allocations, OOM cleanup and zero leaks; no full 4 GiB document claim");
    xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(d);CHECK(!f.live);return 0;
}
