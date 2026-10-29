#include "../src/xui/document/xui_document_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(c) do{if(!(c)){fprintf(stderr,"Prepared semantic line %d: %s\n",__LINE__,#c);exit(1);}}while(0)
typedef struct allocation {uint64_t calls,live;} allocation;
static void* allocate(void* user,size_t bytes)
{allocation* a=user;void* p=malloc(bytes);a->calls++;if(p)a->live++;return p;}
static void deallocate(void* user,void* p)
{allocation* a=user;if(p){CHECK(a->live);a->live--;free(p);}}
static xui_document markdown(allocation* a,const char* text)
{
    xui_document d;xui_doc_desc_t desc={0};desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;
    desc.onAlloc=allocate;desc.onFree=deallocate;desc.pAllocatorUser=a;
    CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK);CHECK(xuiDocumentLoadMarkdown(d,text,strlen(text))==XUI_OK);return d;
}
static xui_document rich(allocation* a,float width,xui_doc_node_id* table)
{
    xui_document d;xui_document_transaction t;xui_doc_desc_t desc={0};desc.iSize=sizeof(desc);
    desc.iProfile=XUI_DOCUMENT_RICH;desc.onAlloc=allocate;desc.onFree=deallocate;desc.pAllocatorUser=a;
    CHECK(xuiDocumentCreate(&desc,&d)==XUI_OK);CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(t,XUI_DOCUMENT_ROOT,0,1,2,0,table)==XUI_OK);
    CHECK(xuiDocumentTxnSetTableColumnWidth(t,*table,0,width)==XUI_OK);
    CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);return d;
}
static xui_doc_position_t find(xui_document_snapshot snapshot,const char* text)
{
    xui_doc_range_t range;uint64_t count;
    CHECK(xuiDocumentSnapshotFind(snapshot,XUI_DOC_SEMANTIC,text,strlen(text),NULL,&range,1,&count)==XUI_OK&&count==1);
    return range.tAnchor;
}
static void input_lineage(void)
{
    allocation a={0};xui_document d=markdown(&a,"abcdef\n");xui_document_snapshot base,preview;
    xui_document_prepare first,second,restart;xui_doc_position_t original,p,q,unchanged;
    xui_doc_txn_desc_t desc={0};xui_document_change_set change;uint64_t caret,calls;int quality,i;
    CHECK(xuiDocumentAcquireSnapshot(d,&base)==XUI_OK);original=find(base,"e");
    desc.iSize=sizeof(desc);desc.iDomain=XUI_DOC_SOURCE;
    CHECK(doc_prepare_visual_text(d,&desc,original.iNodeId,2,2,"\xe4\xb8\xad",3,&first,&preview,&caret)==XUI_OK);
    CHECK(doc_prepare_visual_attach(d,NULL,first)==XUI_OK);xuiDocumentSnapshotRelease(preview);
    CHECK(xuiDocumentPrepareMapPosition(NULL,first,&original,&p,&quality)==XUI_OK&&quality==XUI_DOC_MAP_EXACT&&p.iOffset==7);
    CHECK(xuiDocumentPrepareSemanticPosition(first,original.iNodeId,XUI_DOC_POSITION_TEXT,7,XUI_DOC_AFTER,&q)==XUI_OK);
    CHECK(q.iOffset==p.iOffset&&q.iInputGeneration==p.iInputGeneration);
    unchanged=q;quality=77;
    CHECK(xuiDocumentPrepareSemanticPosition(first,original.iNodeId,XUI_DOC_POSITION_TEXT,3,XUI_DOC_AFTER,&q)==XUI_DOC_ERROR_UTF8);
    CHECK(!memcmp(&q,&unchanged,sizeof(q)));
    CHECK(doc_prepare_visual_continue(d,first,original.iNodeId,5,5,"*",1,caret,&second,&preview,&caret)==XUI_OK);
    CHECK(doc_prepare_visual_attach(d,first,second)==XUI_OK);xuiDocumentSnapshotRelease(preview);
    CHECK(xuiDocumentPrepareMapPosition(first,second,&p,&q,&quality)==XUI_OK&&q.iOffset==8);
    CHECK(xuiDocumentPrepareMapPosition(second,first,&q,&q,&quality)==XUI_OK&&q.iOffset==7);
    CHECK(xuiDocumentPrepareMapPosition(second,NULL,&(xui_doc_position_t){0},&q,&quality)!=XUI_OK);
    calls=a.calls;
    for(i=0;i<5000;i++){
        CHECK(xuiDocumentPrepareMapPosition(NULL,second,&original,&q,&quality)==XUI_OK&&q.iOffset==8);
        CHECK(xuiDocumentPrepareMapPosition(second,NULL,&q,&q,&quality)==XUI_OK&&q.iOffset==4&&!q.iInputGeneration);
    }
    CHECK(a.calls==calls);CHECK(xuiDocumentCancelPrepare(d)==XUI_OK);
    CHECK(xuiDocumentPrepareMapPosition(first,second,&p,&q,&quality)==XUI_OK&&q.iOffset==8);
    CHECK(doc_prepare_visual_text(d,&desc,original.iNodeId,2,2,"\xe4\xb8\xad",3,&restart,&preview,&caret)==XUI_OK);
    CHECK(doc_prepare_visual_attach(d,NULL,restart)==XUI_OK);xuiDocumentSnapshotRelease(preview);
    unchanged=q;quality=77;
    CHECK(xuiDocumentPrepareMapPosition(second,restart,&q,&q,&quality)==XUI_DOC_ERROR_STALE);
    CHECK(!memcmp(&q,&unchanged,sizeof(q))&&quality==77);
    CHECK(xuiDocumentPrepareMapPosition(NULL,restart,&original,&q,&quality)==XUI_OK&&q.iOffset==7);
    CHECK(xuiDocumentPrepareRun(restart)==XUI_OK);
    CHECK(xuiDocumentPreparePublish(d,restart,&change)==XUI_OK);
    xuiDocumentPrepareRelease(restart);
    CHECK(xuiDocumentMapPosition(change,&q,&p,&quality)==XUI_OK&&!p.iInputGeneration&&p.iRevision==xuiDocumentGetRevision(d));
    {xui_document_snapshot committed;xui_doc_position_t expected;
     CHECK(xuiDocumentAcquireSnapshot(d,&committed)==XUI_OK);expected=find(committed,"e");
     CHECK(p.iNodeId==expected.iNodeId&&p.iOffset==expected.iOffset);xuiDocumentSnapshotRelease(committed);}
    xuiDocumentChangeSetRelease(change);xuiDocumentPrepareRelease(first);xuiDocumentPrepareRelease(second);
    xuiDocumentSnapshotRelease(base);xuiDocumentRelease(d);CHECK(!a.live);
    puts("Prepared semantic lineage: UTF8/alias/skipped/reverse/cancel/restart/atomic/no-allocation and retained publication passed.");
}
static void equivalence(void)
{
    allocation a={0},b={0};xui_document left=markdown(&a,"a &amp; b &amp; c ![alt](img)\n\n---\n");
    xui_document right=markdown(&b,"a & b & c ![alt](img)\n\n---\n");
    xui_document_snapshot ls,rs;xui_doc_position_t p,q,expected;doc_node *lp,*rp,*image;uint64_t calls,i;int quality;
    CHECK(xuiDocumentAcquireSnapshot(left,&ls)==XUI_OK);CHECK(xuiDocumentAcquireSnapshot(right,&rs)==XUI_OK);
    CHECK(doc_semantic_equal(ls->state,rs->state));p=find(ls,"b");expected=find(rs,"b");
    CHECK(doc_semantic_map_position(ls->state,rs->state,&p,&q,&quality)==XUI_OK);
    CHECK(quality==XUI_DOC_MAP_EXACT&&q.iKind==XUI_DOC_POSITION_TEXT&&q.iNodeId==expected.iNodeId&&q.iOffset==expected.iOffset);
    lp=doc_index_get(ls->state->index,doc_seq_get_id(doc_index_get(ls->state->index,DOC_ROOT)->children,0));
    rp=doc_index_get(rs->state->index,doc_seq_get_id(doc_index_get(rs->state->index,DOC_ROOT)->children,0));
    CHECK(doc_seq_size(lp->children)>doc_seq_size(rp->children));
    image=doc_index_get(ls->state->index,doc_seq_get_id(lp->children,doc_seq_size(lp->children)-1));CHECK(image->kind==XUI_DOC_IMAGE);
    p.iKind=XUI_DOC_POSITION_GAP;p.iNodeId=lp->id;p.iOffset=doc_seq_size(lp->children)-1;
    CHECK(doc_semantic_map_position(ls->state,rs->state,&p,&q,&quality)==XUI_OK);
    CHECK(q.iKind==XUI_DOC_POSITION_GAP&&q.iNodeId==rp->id&&q.iOffset==doc_seq_size(rp->children)-1);
    p.iOffset++;
    CHECK(doc_semantic_map_position(ls->state,rs->state,&p,&q,&quality)==XUI_OK);
    CHECK(q.iKind==XUI_DOC_POSITION_GAP&&q.iNodeId==rp->id&&q.iOffset==doc_seq_size(rp->children));
    p.iNodeId=image->id;p.iKind=XUI_DOC_POSITION_TEXT;p.iOffset=0;
    CHECK(doc_semantic_map_position(ls->state,rs->state,&p,&q,&quality)==XUI_OK);
    CHECK(q.iKind==XUI_DOC_POSITION_TEXT&&q.iOffset==0&&doc_index_get(rs->state->index,q.iNodeId)->kind==XUI_DOC_IMAGE);
    p.iNodeId=lp->id;p.iKind=XUI_DOC_POSITION_GAP;p.iOffset=1;
    CHECK(doc_semantic_map_position(ls->state,rs->state,&p,&q,&quality)==XUI_OK);
    CHECK(q.iKind==XUI_DOC_POSITION_TEXT&&q.iNodeId==expected.iNodeId&&q.iOffset==2&&quality==XUI_DOC_MAP_COLLAPSED);
    calls=a.calls+b.calls;
    for(i=0;i<5000;i++)CHECK(doc_semantic_map_position(ls->state,rs->state,&p,&q,&quality)==XUI_OK);
    CHECK(a.calls+b.calls==calls);
    xuiDocumentSnapshotRelease(ls);xuiDocumentSnapshotRelease(rs);xuiDocumentRelease(left);xuiDocumentRelease(right);
    CHECK(!a.live&&!b.live);puts("Semantic equivalent trees: entity repartition/coalescing, internal gaps and image before/after/child, no allocation passed.");
}
static void table_geometry_equivalence(void)
{
    allocation a={0},b={0},c={0};xui_doc_node_id at,bt,ct;
    xui_document left=rich(&a,80,&at),different=rich(&b,120,&bt),same=rich(&c,80,&ct);
    xui_document_snapshot ls,ds,ss;
    CHECK(xuiDocumentAcquireSnapshot(left,&ls)==XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(different,&ds)==XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(same,&ss)==XUI_OK);
    CHECK(!doc_semantic_equal(ls->state,ds->state));
    CHECK(!doc_semantic_subtree_equal(ls->state,at,ds->state,bt));
    CHECK(doc_semantic_equal(ls->state,ss->state));
    CHECK(doc_semantic_subtree_equal(ls->state,at,ss->state,ct));
    xuiDocumentSnapshotRelease(ls);xuiDocumentSnapshotRelease(ds);xuiDocumentSnapshotRelease(ss);
    xuiDocumentRelease(left);xuiDocumentRelease(different);xuiDocumentRelease(same);
    CHECK(!a.live&&!b.live&&!c.live);puts("Semantic table geometry: distinct widths differ and equal widths match.");
}
int main(void){input_lineage();equivalence();table_geometry_equivalence();puts("Unified prepared semantic mapping tests passed.");return 0;}
