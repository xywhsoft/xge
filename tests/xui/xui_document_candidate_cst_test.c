#include "../src/xui/document/xui_document_internal.h"
#include "../src/xui/document/xui_document_md4c.h"
#include "../src/xui/document/xui_document_cst_collect.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"candidate primitive line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
typedef struct fault_alloc {long remaining,cancel_at;unsigned live;atomic_int cancel;} fault_alloc;
static void* allocate(void* user,size_t size)
{
    fault_alloc* a=user;void* p;
    if(a->cancel_at==0){a->cancel_at=-1;atomic_store(&a->cancel,XUI_DOC_ERROR_CANCELLED);}
    else if(a->cancel_at>0)a->cancel_at--;
    if(a->remaining==0)return NULL;
    if(a->remaining>0)a->remaining--;
    p=malloc(size);if(p)a->live++;return p;
}
static void release(void* user,void* p)
{fault_alloc* a=user;CHECK(a->live);a->live--;free(p);}
static xui_document document(fault_alloc* a,uint64_t cap)
{
    xui_doc_desc_t desc={0};xui_document d;desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;
    desc.iMaxSyntaxRecords=cap;desc.onAlloc=allocate;desc.onFree=release;desc.pAllocatorUser=a;
    CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK);return d;
}
static void initialize(doc_cst_collector* c,doc_state* s,atomic_int* cancellation)
{
    memset(c,0,sizeof(*c));c->state=s;c->active=1;c->base=37;c->offset=11;c->end=100000;c->size=100000;
    c->cancellation=cancellation;CHECK(doc_cst_append(c,DOC_CST_SCOPE,0,0,DOC_NONE,c->base,c->end,NULL)==XUI_OK);
}
static void cleanup(doc_cst_collector* c)
{doc_free(c->records);doc_free(c->candidate_entries);memset(c,0,sizeof(*c));}
static int add(doc_cst_collector* c,unsigned value)
{MD_OFFSET start=40+value*9;return doc_cst_candidate(c,1,start,start+7,start+2,start+4);}
typedef struct key{uint64_t v[6];}key;
static int key_order(const key* a,const key* b)
{unsigned i;for(i=0;i<6;i++)if(a->v[i]!=b->v[i])return a->v[i]<b->v[i]?-1:1;return 0;}
static key entry_key(doc_cst_collector* c,uint64_t at)
{
    doc_source_cst_record* r=c->records+c->candidate_entries[at-1].record;
    key k={{r->parent,r->detail,r->start,r->end,r[1].start,r[1].end}};
    CHECK(r->kind==DOC_CST_INLINE && r->flags==DOC_CST_INLINE_CANDIDATE && r[1].kind==DOC_CST_FIELD);
    CHECK(r[1].detail==DOC_CST_FIELD_CANDIDATE_LABEL && r[1].parent==c->candidate_entries[at-1].record);
    CHECK(r[1].metadata[0]==r[1].start && r[1].metadata[1]==r[1].end);return k;
}
static unsigned inspect_tree(doc_cst_collector* c,uint64_t at,const key* low,const key* high,unsigned char* seen,unsigned* visited)
{
    key k;unsigned left,right,height;if(!at)return 0;
    CHECK(at<=c->candidate_count && !seen[at-1]);seen[at-1]=1;(*visited)++;k=entry_key(c,at);
    CHECK((!low || key_order(low,&k)<0) && (!high || key_order(&k,high)<0));
    left=inspect_tree(c,c->candidate_entries[at-1].left,low,&k,seen,visited);
    right=inspect_tree(c,c->candidate_entries[at-1].right,&k,high,seen,visited);
    CHECK(left<=right+1 && right<=left+1);height=(left>right?left:right)+1;
    CHECK(c->candidate_entries[at-1].height==height);return height;
}
static void ordered_inputs(void)
{
    unsigned mode;
    for(mode=0;mode<3;mode++){
        fault_alloc a={-1,-1,0,ATOMIC_VAR_INIT(0)};xui_document d=document(&a,200000);doc_cst_collector c;
        unsigned i,visited=0,height;unsigned char seen[4096]={0};uint64_t ids=atomic_load(&d->allocator->next_node_id);
        initialize(&c,d->state,&a.cancel);
        for(i=0;i<4096;i++)CHECK(add(&c,mode==0?i:mode==1?4095-i:(i*2053u)%4096u)==XUI_OK);
        CHECK(c.candidate_count==4096 && c.count==8193);height=inspect_tree(&c,c.candidate_root,NULL,NULL,seen,&visited);
        CHECK(visited==4096 && height<=24);
        a.remaining=0;
        for(i=0;i<4096;i++)CHECK(add(&c,i)==XUI_OK);
        CHECK(c.candidate_count==4096 && c.count==8193 && atomic_load(&d->allocator->next_node_id)==ids);
        a.remaining=-1;cleanup(&c);xuiDocumentRelease(d);CHECK(!a.live);
        printf("Candidate catalog ordering=%u unique=4096 duplicate-queries=4096 height=%u; no duplicate allocation or NodeId\n",mode,height);
    }
}
static void distinct_keys_and_limits(void)
{
    fault_alloc a={-1,-1,0,ATOMIC_VAR_INIT(0)};xui_document d=document(&a,1000);doc_cst_collector c;
    unsigned i,visited=0;unsigned char seen[16]={0};uint64_t count,owners;
    const uint64_t keys[][6]={{0,1,40,49,42,43},{0,2,40,49,42,43},{0,3,40,49,42,43},
        {1,1,40,49,42,43},{2,1,40,49,42,43},{0,1,39,49,42,43},{0,1,40,50,42,43},
        {0,1,40,49,41,43},{0,1,40,49,42,44}};
    initialize(&c,d->state,&a.cancel);
    CHECK(doc_cst_append(&c,DOC_CST_BLOCK,MD_BLOCK_P,0,0,c.base,c.end,NULL)==XUI_OK);
    CHECK(doc_cst_append(&c,DOC_CST_BLOCK,MD_BLOCK_P,0,0,c.base,c.end,NULL)==XUI_OK);c.block_count=1;
    for(i=0;i<sizeof(keys)/sizeof(*keys);i++){
        c.blocks[0]=keys[i][0];CHECK(doc_cst_candidate(&c,(int)keys[i][1],(MD_OFFSET)keys[i][2],(MD_OFFSET)keys[i][3],
            (MD_OFFSET)keys[i][4],(MD_OFFSET)keys[i][5])==XUI_OK);
    }
    CHECK(c.candidate_count==9 && c.count==21);inspect_tree(&c,c.candidate_root,NULL,NULL,seen,&visited);CHECK(visited==9);
    count=c.count;owners=c.candidate_count;
    CHECK(doc_cst_candidate(&c,0,40,49,42,43)==XUI_DOC_ERROR_FORMAT);
    CHECK(doc_cst_candidate(&c,4,40,49,42,43)==XUI_DOC_ERROR_FORMAT);
    CHECK(doc_cst_candidate(&c,1,20,49,42,43)==XUI_DOC_ERROR_FORMAT);
    CHECK(doc_cst_candidate(&c,1,40,49,39,43)==XUI_DOC_ERROR_FORMAT);
    CHECK(doc_cst_candidate(&c,1,40,49,44,43)==XUI_DOC_ERROR_FORMAT);
    CHECK(doc_cst_candidate(&c,1,40,49,42,50)==XUI_DOC_ERROR_FORMAT);
    CHECK(doc_cst_candidate(&c,1,40,99999,42,43)==XUI_DOC_ERROR_FORMAT);
    c.blocks[0]=c.count;CHECK(doc_cst_candidate(&c,1,40,49,42,43)==XUI_DOC_ERROR_FORMAT);c.blocks[0]=0;
    CHECK(c.count==count && c.candidate_count==owners);cleanup(&c);xuiDocumentRelease(d);CHECK(!a.live);
    d=document(&a,3);initialize(&c,d->state,&a.cancel);CHECK(add(&c,0)==XUI_OK);
    a.remaining=0;CHECK(add(&c,0)==XUI_OK);a.remaining=-1;
    CHECK(add(&c,1)==XUI_DOC_ERROR_LIMIT && c.count==3 && c.candidate_count==1);
    cleanup(&c);xuiDocumentRelease(d);CHECK(!a.live);
    puts("Candidate keys: distinct parent/kind/outer/label ranges retained; invalid inputs unchanged; exact cap3 accepts duplicates without allocating and rejects new owner.");
}
static void faults_and_abort(void)
{
    unsigned mode;long totals[2]={0};
    for(mode=0;mode<2;mode++)for(long point=0;point<100;point++){
        fault_alloc a={-1,-1,0,ATOMIC_VAR_INIT(0)};xui_document d=document(&a,200000);doc_cst_collector c;
        unsigned i,live=a.live;int result=XUI_OK;uint64_t revision=xuiDocumentGetRevision(d);
        initialize(&c,d->state,&a.cancel);
        if(mode)a.cancel_at=point;else a.remaining=point;
        for(i=0;i<128 && result==XUI_OK;i++)result=add(&c,i);
        a.remaining=a.cancel_at=-1;
        if(result!=XUI_OK)CHECK(result==(mode?XUI_DOC_ERROR_CANCELLED:XUI_ERROR_OUT_OF_MEMORY));
        CHECK(xuiDocumentGetRevision(d)==revision && d->state->node_count==1);
        CHECK(doc_cst_scope(&c,0,0,-1)==XUI_OK && !c.active && !c.count && !c.candidate_count && !c.candidate_root);
        atomic_store(&a.cancel,XUI_OK);c.active=1;CHECK(doc_cst_append(&c,DOC_CST_SCOPE,0,0,DOC_NONE,c.base,c.end,NULL)==XUI_OK);
        CHECK(add(&c,0)==XUI_OK && c.count==3 && c.candidate_count==1);
        cleanup(&c);CHECK(a.live==live);xuiDocumentRelease(d);CHECK(!a.live);
        if(result==XUI_OK){totals[mode]=point;break;}
        CHECK(point<99);
    }
    printf("Candidate faults: %ld allocation failures/%ld allocation-time cancellation points, abort/reset/reuse and original state preserved; zero leaks\n",totals[0],totals[1]);
}
static void definition_scope_reuse(void)
{
    const char* source="[^a]: [x][y]\n\n[^b]: [x][y]\n";
    fault_alloc a={-1,-1,0,ATOMIC_VAR_INIT(0)};xui_document d=document(&a,1000);doc_state* state;
    doc_cst_collector c={0};uint64_t i;CHECK(xuiDocumentLoadMarkdown(d,source,strlen(source))==XUI_OK);
    state=doc_state_clone(d->state);CHECK(state);c.state=state;c.source=source;c.size=strlen(source);c.cancellation=&a.cancel;
    for(i=0;i<2;i++){
        xui_doc_reference_definition_t definition;uint64_t from;doc_source_cst_header header;
        CHECK(doc_seq_read(state->references,i*sizeof(definition),&definition,sizeof(definition))==XUI_OK);
        from=definition.iSourceStart+6;
        CHECK(doc_cst_scope(&c,(MD_OFFSET)definition.iSourceStart,(MD_OFFSET)definition.iSourceEnd,1)==XUI_OK);
        CHECK(!c.candidate_count && !c.candidate_root);
        CHECK(doc_cst_block(&c,MD_BLOCK_FOOTNOTE_DEF,(MD_OFFSET)definition.iSourceStart,(MD_OFFSET)definition.iSourceEnd,1)==XUI_OK);
        CHECK(doc_cst_candidate(&c,1,(MD_OFFSET)from,(MD_OFFSET)from+6,(MD_OFFSET)from+4,(MD_OFFSET)from+5)==XUI_OK);
        CHECK(doc_cst_candidate(&c,1,(MD_OFFSET)from,(MD_OFFSET)from+6,(MD_OFFSET)from+4,(MD_OFFSET)from+5)==XUI_OK);
        CHECK(c.count==5 && c.candidate_count==1);
        CHECK(doc_cst_scope(&c,(MD_OFFSET)definition.iSourceStart,(MD_OFFSET)definition.iSourceEnd,0)==XUI_OK);
        CHECK(!c.active && !c.count && !c.candidate_count && !c.candidate_root);
        {
            doc_source_cst_record trivia;uint64_t at=from+6,ordinal=6,count=6;
            while(at<definition.iSourceEnd){CHECK(source[at]=='\n');count++;at++;}
            CHECK(doc_source_cst_info(state,i,&header)==XUI_OK && header.count==count);
            CHECK(doc_source_cst_read(state,i,5,&trivia)==XUI_OK && trivia.kind==DOC_CST_FIELD &&
                trivia.detail==DOC_CST_FIELD_WHITESPACE && trivia.parent==2 && trivia.start==5 && trivia.end==6 &&
                trivia.metadata[0]==5 && trivia.metadata[1]==6);
            for(at=from+6;at<definition.iSourceEnd;at++,ordinal++){
                CHECK(doc_source_cst_read(state,i,ordinal,&trivia)==XUI_OK && trivia.kind==DOC_CST_BREAK &&
                    trivia.flags==DOC_CST_BREAK_ENDING && trivia.parent==2 &&
                    trivia.start==at-definition.iSourceStart && trivia.end==at+1-definition.iSourceStart);
            }
        }
    }
    cleanup(&c);doc_state_release(state);xuiDocumentRelease(d);CHECK(!a.live);
    puts("Two definition scopes: completed/reset/reused catalog retains both independent grammar owners and frees all memory.");
}
int main(void)
{ordered_inputs();distinct_keys_and_limits();faults_and_abort();definition_scope_reuse();puts("Candidate primitive model/uniqueness/range/balance/lifetime checks passed.");return 0;}
