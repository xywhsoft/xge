#include "../xui_document_ui.h"
#include "xui_test_proxy.h"
#include "../src/xui_document_layout_internal.h"
#include "../src/xui_text_internal.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); exit(1); } } while(0)
typedef union header { max_align_t align; size_t size; } header;
static size_t allocations,live,live_bytes,fail_at;
void* xuiBidiTestMalloc(size_t bytes)
{
    header* p;if(++allocations==fail_at || bytes>SIZE_MAX-sizeof(*p))return NULL;
    p=malloc(sizeof(*p)+bytes);if(!p)return NULL;p->size=bytes;live++;live_bytes+=bytes;return p+1;
}
void xuiBidiTestFree(void* data)
{
    if(data){header* p=(header*)data-1;CHECK(live && live_bytes>=p->size);live--;live_bytes-=p->size;free(p);}
}
void* xuiBidiTestRealloc(void* data,size_t bytes)
{
    header* p;size_t old;if(!data)return xuiBidiTestMalloc(bytes);
    if(++allocations==fail_at || bytes>SIZE_MAX-sizeof(*p))return NULL;
    old=((header*)data-1)->size;
    p=realloc((header*)data-1,sizeof(*p)+bytes);if(!p)return NULL;p->size=bytes;live_bytes=live_bytes-old+bytes;return p+1;
}
static xui_doc_position_t pos(xui_document d,uint64_t id,uint64_t offset)
{
    xui_doc_position_t p={0};p.iSize=sizeof(p);p.iKind=XUI_DOC_POSITION_TEXT;
    p.iDocumentId=xuiDocumentGetIdentity(d);p.iRevision=xuiDocumentGetRevision(d);
    p.iNodeId=id;p.iOffset=offset;p.iAffinity=XUI_DOC_AFTER;return p;
}
static void check(xui_document d,xui_document_renderer r,uint64_t id,const char* text)
{
    xui_doc_position_t p=pos(d,id,2);xui_doc_rect_t upstream,downstream,rects[4];uint64_t count;
    xui_doc_range_t selected={pos(d,id,0),pos(d,id,4)};
    xui_document_snapshot s;char* plain;uint64_t bytes;
    p.iAffinity=XUI_DOC_BEFORE;CHECK(xuiDocumentRendererGetCaretRect(r,&p,&upstream)==XUI_OK);
    p.iAffinity=XUI_DOC_AFTER;CHECK(xuiDocumentRendererGetCaretRect(r,&p,&downstream)==XUI_OK && downstream.x>upstream.x);
    CHECK(xuiDocumentRendererGetRangeRects(r,&selected,rects,4,&count)==XUI_OK && count==2);
    CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentSnapshotCopyPlainText(s,&plain,&bytes)==XUI_OK);
    CHECK(bytes==strlen(text)+1 && !memcmp(plain,text,strlen(text)) && plain[strlen(text)]=='\n');
    xuiDocumentFreeBuffer(plain);xuiDocumentSnapshotRelease(s);CHECK(!live);
}
static uint64_t insert(xui_document_transaction t,uint64_t parent,uint32_t kind,const char* text)
{
    xui_doc_node_desc_t n={0};uint64_t id;n.iSize=sizeof(n);n.iKind=kind;n.sText=text;n.iTextBytes=text?strlen(text):0;
    n.tAttributes.iRowSpan=n.tAttributes.iColumnSpan=1;
    CHECK(xuiDocumentTxnInsertNode(t,parent,XUI_DOCUMENT_APPEND,&n,&id)==XUI_OK);return id;
}
static void table(xui_context ctx)
{
    xui_document d;xui_document_transaction t;xui_document_snapshot s;xui_document_renderer r;
    uint64_t table_id,row,cell,paragraph,ids[2];unsigned i;char long_text[97];xui_doc_rect_t a,b; xui_doc_position_t p;
    memset(long_text,'f',96);long_text[96]=0;
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
    table_id=insert(t,1,XUI_DOC_TABLE,NULL);row=insert(t,table_id,XUI_DOC_ROW,NULL);
    for(i=0;i<2;i++) {
        cell=insert(t,row,XUI_DOC_CELL,NULL);paragraph=insert(t,cell,XUI_DOC_PARAGRAPH,NULL);
        ids[i]=insert(t,paragraph,XUI_DOC_TEXT,i?long_text:"\xd7\x90\xd7\x91\xd7\x92");
    }
    CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentRendererCreate(ctx,NULL,&r)==XUI_OK &&
        xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK);xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentRendererLayout(r,320,0,400)==XUI_OK && !live);
    p=pos(d,ids[0],0);CHECK(xuiDocumentRendererGetCaretRect(r,&p,&a)==XUI_OK);
    p.iOffset=6;CHECK(xuiDocumentRendererGetCaretRect(r,&p,&b)==XUI_OK && b.x<a.x);
    p=pos(d,ids[1],0);CHECK(xuiDocumentRendererGetCaretRect(r,&p,&a)==XUI_OK);
    p.iOffset=2;CHECK(xuiDocumentRendererGetCaretRect(r,&p,&b)==XUI_OK && b.x>a.x);
    CHECK(xuiDocumentRendererLayout(r,160,0,400)==XUI_OK && !live && xuiDocumentRendererLayout(r,320,0,400)==XUI_OK && !live);
    xuiDocumentRendererRelease(r);xuiDocumentRelease(d);
}
static int object_measure(xui_document_snapshot s,xui_doc_node_id id,float width,float zoom,xui_vec2_t* size,float* baseline,void* user)
{
    (void)s;(void)id;(void)width;(void)user;*size=(xui_vec2_t){30*zoom,20*zoom};*baseline=16*zoom;return XUI_OK;
}
static void object(xui_context ctx)
{
    xui_document d;xui_document_transaction t;xui_document_snapshot s;xui_document_renderer r;
    xui_doc_renderer_desc_t desc={0};uint64_t paragraph;xui_doc_position_t before,after,hit;xui_doc_rect_t a,b,clicked;
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
    paragraph=insert(t,1,XUI_DOC_PARAGRAPH,NULL);insert(t,paragraph,XUI_DOC_TEXT,"\xd7\x90");
    insert(t,paragraph,XUI_DOC_MATH,"x^2");insert(t,paragraph,XUI_DOC_TEXT,"\xd7\x91");
    CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);desc.iSize=sizeof(desc);desc.onObjectMeasure=object_measure;
    CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentRendererCreate(ctx,&desc,&r)==XUI_OK &&
        xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK);xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentRendererLayout(r,160,0,180)==XUI_OK && !live);
    before=pos(d,paragraph,1);before.iKind=XUI_DOC_POSITION_GAP;after=pos(d,paragraph,2);after.iKind=XUI_DOC_POSITION_GAP;after.iAffinity=XUI_DOC_BEFORE;
    CHECK(xuiDocumentRendererGetCaretRect(r,&before,&a)==XUI_OK && xuiDocumentRendererGetCaretRect(r,&after,&b)==XUI_OK && fabs(a.x-b.x-30)<.001);
    CHECK(xuiDocumentRendererHitTest(r,b.x+1,b.y+b.height*.5,&hit)==XUI_OK && hit.iNodeId==paragraph && hit.iOffset==2 &&
        xuiDocumentRendererGetCaretRect(r,&hit,&clicked)==XUI_OK && fabs(clicked.x-b.x)<.001);
    xuiDocumentRendererRelease(r);xuiDocumentRelease(d);CHECK(!live);
}
static void formats(void)
{
    const char* text="a\xd8\x9c\xe2\x80\x8e\xe2\x80\x8f\xe2\x80\xaa\xe2\x80\xab\xe2\x80\xac\xe2\x80\xad\xe2\x80\xae\xe2\x81\xa6\xe2\x81\xa7\xe2\x81\xa8\xe2\x81\xa9" "b";
    char display[3]={0};unsigned char map[64]={0};size_t i,bytes=strlen(text);
    CHECK(xuiInternalTextCopyDisplay(text,(int)bytes,display)==2 && !strcmp(display,"ab"));
    CHECK(xuiInternalTextBreakMap(text,(int)bytes,map)==XUI_OK);
    for(i=1;i<bytes-1;i++)CHECK(map[i]&XUI_LB_INVISIBLE);
    CHECK(!(map[0]&XUI_LB_INVISIBLE) && !(map[bytes-1]&XUI_LB_INVISIBLE));
}
static void continuation(xui_context ctx)
{
    const char* pattern="f \xd7\x90\xd7\x91\xd7\x92 \xd8\xa8\xd8\xaa\xd8\xab i \xe2\x81\xa7\xd7\x90\xe2\x81\xa9 ";
    const double width=4096;
    size_t unit=strlen(pattern),bytes=unit*1500,i,baseline,point;
    char* text=malloc(bytes+1);xui_document d;xui_document_transaction t;xui_document_snapshot s;
    xui_document_renderer r;uint64_t paragraph,id;
    CHECK(text);for(i=0;i<bytes;i+=unit)memcpy(text+i,pattern,unit);text[bytes]=0;
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
    paragraph=insert(t,1,XUI_DOC_PARAGRAPH,NULL);id=insert(t,paragraph,XUI_DOC_TEXT,text);
    CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK);
    CHECK(xuiDocumentRendererCreate(ctx,NULL,&r)==XUI_OK && xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK &&
        xuiDocumentRendererLayout(r,width,0,60)==XUI_OK);
    CHECK(r->count==1 && r->blocks[0].partial && r->blocks[0].continuation && live && live_bytes);
    allocations=0;
    CHECK(doc_layout_block_extend(r,&r->blocks[0],width,r->blocks[0].partial_bottom+1)==XUI_OK);
    baseline=allocations;CHECK(baseline>6);xuiDocumentRendererRelease(r);CHECK(!live && !live_bytes);
    fprintf(stderr,"Document Bidi continuation scanning all %zu line allocation points\n",baseline);
    for(point=1;point<=baseline;point++) {
        doc_render_block* b;doc_fragment* fragments;doc_render_line* lines;size_t* visual;
        size_t count,line_count,group_count,owners,retained;uint64_t cutoff;double bottom,height;
        char** paints;size_t* paint_bytes;int result;
        CHECK(xuiDocumentRendererCreate(ctx,NULL,&r)==XUI_OK && xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK &&
            xuiDocumentRendererLayout(r,width,0,60)==XUI_OK);
        b=&r->blocks[0];CHECK(b->partial && b->visual_fragments && !b->layout_bidi);
        count=b->fragment_count;line_count=b->line_count;group_count=b->paint_group_count;
        cutoff=b->partial_cutoff;bottom=b->partial_bottom;height=b->height;owners=live;retained=live_bytes;
        CHECK(retained>bytes && b->cache_bytes==doc_render_block_cache_bytes(b) && b->cache_bytes>=retained);
        fragments=malloc(count*sizeof(*fragments));lines=malloc(line_count*sizeof(*lines));visual=malloc(count*sizeof(*visual));
        paints=calloc(group_count,sizeof(*paints));paint_bytes=calloc(group_count,sizeof(*paint_bytes));
        CHECK(fragments && lines && visual && paints && paint_bytes);
        memcpy(fragments,b->fragments,count*sizeof(*fragments));memcpy(lines,b->lines,line_count*sizeof(*lines));
        memcpy(visual,b->visual_fragments,count*sizeof(*visual));
        for(i=0;i<group_count;i++) {
            paint_bytes[i]=(size_t)b->paint_groups[i].bytes;
            paints[i]=malloc(paint_bytes[i]+1);CHECK(paints[i]);memcpy(paints[i],b->paint_groups[i].text,paint_bytes[i]+1);
        }
        allocations=0;fail_at=point;result=doc_layout_block_extend(r,b,width,bottom+1);fail_at=0;
        CHECK(result==XUI_ERROR_OUT_OF_MEMORY && live==owners && live_bytes==retained && !b->layout_bidi);
        CHECK(b->partial_cutoff==cutoff && b->partial_bottom==bottom && b->height==height &&
            b->fragment_count==count && b->line_count==line_count && b->paint_group_count==group_count);
        CHECK(!memcmp(fragments,b->fragments,count*sizeof(*fragments)) && !memcmp(lines,b->lines,line_count*sizeof(*lines)) &&
            !memcmp(visual,b->visual_fragments,count*sizeof(*visual)));
        for(i=0;i<group_count;i++) {
            CHECK(b->paint_groups[i].bytes==paint_bytes[i] && !memcmp(paints[i],b->paint_groups[i].text,paint_bytes[i]+1));free(paints[i]);
        }
        CHECK(doc_layout_block_extend(r,b,width,bottom+1)==XUI_OK && live==owners && live_bytes==retained);
        CHECK(b->partial_cutoff>cutoff && b->partial_bottom>bottom && b->cache_bytes==doc_render_block_cache_bytes(b));
        /* This test calls the private extender to isolate rollback. Apply the
         * one-block renderer accounting normally done by its public wrapper. */
        r->cache_bytes=b->cache_bytes;
        for(i=0;i<lines[line_count-1].first;i++)CHECK(!memcmp(&fragments[i],&b->fragments[i],sizeof(*fragments)));
        if(point==baseline) {
            xui_doc_position_t p=pos(d,id,unit*1000);xui_doc_rect_t caret;
            CHECK(xuiDocumentRendererGetCaretRect(r,&p,&caret)==XUI_OK && caret.y>bottom);
        }
        free(fragments);free(lines);free(visual);free(paints);free(paint_bytes);
        xuiDocumentRendererRelease(r);CHECK(!live && !live_bytes);
    }
    printf("Document Bidi continuation: %zu line allocation failures restore fragments, visual order, lines and paint strings; retry preserves full-paragraph owners and cache accounting\n",baseline);
    xuiDocumentSnapshotRelease(s);xuiDocumentRelease(d);free(text);
}
int main(void)
{
    const char* text="f \xd7\x90\xd7\x91\xd7\x92 i";
    xui_test_proxy_state_t proxy;xui_context ctx;xui_font font;xui_document d;
    xui_document_transaction t;xui_document_snapshot s;xui_document_renderer r;
    xui_doc_node_desc_t n={0};uint64_t paragraph,id;size_t baseline,point,recovered=0;
    xuiTestProxyInit(&proxy);CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,NULL,16,0)==XUI_OK);
    CHECK(xuiCreate(&ctx)==XUI_OK && xuiSetProxy(ctx,&proxy.tProxy)==XUI_OK && xuiSetDefaultFont(ctx,font)==XUI_OK);
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
    n.iSize=sizeof(n);n.iKind=XUI_DOC_PARAGRAPH;CHECK(xuiDocumentTxnInsertNode(t,1,XUI_DOCUMENT_APPEND,&n,&paragraph)==XUI_OK);
    n.iKind=XUI_DOC_TEXT;n.sText=text;n.iTextBytes=strlen(text);
    CHECK(xuiDocumentTxnInsertNode(t,paragraph,XUI_DOCUMENT_APPEND,&n,&id)==XUI_OK && xuiDocumentTxnCommit(t,NULL)==XUI_OK);
    xuiDocumentTxnRelease(t);CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK);
    CHECK(xuiDocumentRendererCreate(ctx,NULL,&r)==XUI_OK && xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK);
    allocations=0;CHECK(xuiDocumentRendererLayout(r,160,0,180)==XUI_OK);baseline=allocations;check(d,r,id,text);
    xuiDocumentRendererRelease(r);CHECK(!live && baseline>10);
    for(point=1;point<=baseline;point++) {
        int result;CHECK(xuiDocumentRendererCreate(ctx,NULL,&r)==XUI_OK && xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK);
        allocations=0;fail_at=point;result=xuiDocumentRendererLayout(r,160,0,180);fail_at=0;
        CHECK(result==XUI_OK || result==XUI_ERROR_OUT_OF_MEMORY);
        CHECK(!live);if(result==XUI_OK)recovered++;
        CHECK(xuiDocumentRendererLayout(r,160,0,180)==XUI_OK);check(d,r,id,text);
        {
            size_t seeds=r->blocks[0].paint_seed_count;unsigned reflow;
            for(reflow=0;reflow<4;reflow++) {
                CHECK(xuiDocumentRendererLayout(r,80,0,180)==XUI_OK && !live);
                CHECK(xuiDocumentRendererLayout(r,160,0,180)==XUI_OK);check(d,r,id,text);
                CHECK(r->blocks[0].paint_seed_count==seeds);
            }
        }
        xuiDocumentRendererRelease(r);CHECK(!live);
    }
    xuiDocumentSnapshotRelease(s);xuiDocumentRelease(d);table(ctx);object(ctx);formats();continuation(ctx);xuiDestroy(ctx);proxy.tProxy.fontDestroy(&proxy.tProxy,font);
    printf("Document Bidi allocation rollback: %zu analysis/line allocations, recovered=%zu; retry/reflow/ranges preserve logical text and release all owners\n",baseline,recovered);
    return 0;
}
