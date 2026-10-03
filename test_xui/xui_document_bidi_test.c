#include "../test/xge_opentype_fixture_path.h"
#include "../xge.h"
#include "../xui_document_ui.h"
#include "../src/xui_document_layout_internal.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); exit(1); } } while(0)
#define ALEPH "\xd7\x90"
#define BET "\xd7\x91"
#define GIMEL "\xd7\x92"
#define HEB ALEPH BET GIMEL
enum { W=320,H=180 };
static xui_doc_position_t pos(xui_document d,uint64_t id,uint64_t offset)
{
    xui_doc_position_t p={0}; p.iSize=sizeof(p); p.iKind=XUI_DOC_POSITION_TEXT;
    p.iDocumentId=xuiDocumentGetIdentity(d); p.iRevision=xuiDocumentGetRevision(d);
    p.iNodeId=id; p.iOffset=offset; p.iAffinity=XUI_DOC_AFTER; return p;
}
static uint64_t insert(xui_document_transaction t,uint64_t parent,const char* text,uint32_t color)
{
    xui_doc_node_desc_t n={0}; uint64_t id;
    n.iSize=sizeof(n); n.iKind=text?XUI_DOC_TEXT:XUI_DOC_PARAGRAPH;
    n.sText=text; n.iTextBytes=text?strlen(text):0; n.tAttributes.iTextColor=color;
    CHECK(xuiDocumentTxnInsertNode(t,parent,XUI_DOCUMENT_APPEND,&n,&id)==XUI_OK); return id;
}
static void plain(xui_document d,const char* expected)
{
    xui_document_snapshot s;char* text;uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentSnapshotCopyPlainText(s,&text,&bytes)==XUI_OK);
    CHECK(bytes==strlen(expected) && !strcmp(text,expected));xuiDocumentFreeBuffer(text);xuiDocumentSnapshotRelease(s);
}
#include "xui_document_ligature_mark_cases.h"
#include "xui_document_font_fallback_cases.h"
static void mixed(xui_context ctx,xui_proxy proxy,xui_font font,const xui_doc_renderer_desc_t* desc,
    xui_surface surface,xui_surface reference,float size)
{
    xui_document d; xui_document_transaction t; xui_document_snapshot s; xui_document_renderer r;
    uint64_t paragraph,id; xui_text_shape_t left={0},middle={0},right={0}; xui_doc_position_t p;
    xui_doc_rect_t a,b,rects[8]; uint64_t count; xui_draw_context draw;
    unsigned char actual[W*H*4],expected[W*H*4]; unsigned k;
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
    paragraph=insert(t,1,NULL,0);id=insert(t,paragraph,"f " HEB " i",0);
    CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentRendererCreate(ctx,desc,&r)==XUI_OK &&
        xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK);xuiDocumentSnapshotRelease(s);
    CHECK(xuiTextShape(ctx, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText="f ", .iTextSize=2, .iFlags=0}, &left)==XUI_OK && xuiTextShape(ctx, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=HEB, .iTextSize=6, .iFlags=XUI_TEXT_SHAPE_RTL}, &middle)==XUI_OK &&
        xuiTextShape(ctx, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=" i", .iTextSize=2, .iFlags=0}, &right)==XUI_OK);
    CHECK(xuiDocumentRendererLayout(r,W,0,H)==XUI_OK);
    p=pos(d,id,2);p.iAffinity=XUI_DOC_BEFORE;
    CHECK(xuiDocumentRendererGetCaretRect(r,&p,&a)==XUI_OK && fabs(a.x-left.fWidth)<.001);
    p.iAffinity=XUI_DOC_AFTER;
    CHECK(xuiDocumentRendererGetCaretRect(r,&p,&b)==XUI_OK && fabs(b.x-left.fWidth-size)<.001 && fabs(a.y-b.y)<.001);
    p=pos(d,id,8);p.iAffinity=XUI_DOC_BEFORE;
    CHECK(xuiDocumentRendererGetCaretRect(r,&p,&a)==XUI_OK && fabs(a.x-left.fWidth)<.001);
    p.iAffinity=XUI_DOC_AFTER;
    CHECK(xuiDocumentRendererGetCaretRect(r,&p,&b)==XUI_OK && fabs(b.x-left.fWidth-size)<.001);
    {
        xui_doc_range_t selected={pos(d,id,0),pos(d,id,4)};
        CHECK(xuiDocumentRendererGetRangeRects(r,&selected,rects,8,&count)==XUI_OK && count==2);
        CHECK(fabs(rects[0].x)<.001 && fabs(rects[0].width-left.fWidth)<.001 &&
            fabs(rects[1].x-left.fWidth-size*.8)<.001 && fabs(rects[1].width-size*.2)<.001);
    }
    CHECK(proxy->surfaceClear(proxy,surface,0)==XUI_OK && proxy->drawBegin(proxy,&draw,surface)==XUI_OK &&
        xuiDocumentRendererDraw(r,draw,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK && proxy->drawEnd(proxy,draw)==XUI_OK);
    CHECK(proxy->surfaceClear(proxy,reference,0)==XUI_OK && proxy->drawBegin(proxy,&draw,reference)==XUI_OK);
    CHECK(proxy->drawText(proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText="f ", .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, (xui_rect_t){0,(float)a.y,left.fWidth,left.fLineHeight}, desc->iTextColor, XUI_TEXT_CLIP)==XUI_OK);
    CHECK(proxy->drawText(proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=HEB, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_CLIP|XUI_TEXT_RTL) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, (xui_rect_t){left.fWidth,(float)a.y,middle.fWidth,middle.fLineHeight}, desc->iTextColor, XUI_TEXT_CLIP|XUI_TEXT_RTL)==XUI_OK);
    CHECK(proxy->drawText(proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=" i", .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, (xui_rect_t){left.fWidth+size,(float)a.y,right.fWidth,right.fLineHeight}, desc->iTextColor, XUI_TEXT_CLIP)==XUI_OK);
    CHECK(proxy->drawEnd(proxy,draw)==XUI_OK && proxy->surfaceReadRGBA(proxy,surface,actual,W*4)==XUI_OK &&
        proxy->surfaceReadRGBA(proxy,reference,expected,W*4)==XUI_OK);
    for(k=0;k<W*H;k++)CHECK(actual[k*4+3]==expected[k*4+3]);
    /* The same source boundary has an upstream first-row caret and a
     * downstream RTL caret after wrapping. */
    CHECK(xuiDocumentRendererLayout(r,left.fWidth+size+1,0,H)==XUI_OK);
    p=pos(d,id,2);p.iAffinity=XUI_DOC_BEFORE;
    CHECK(xuiDocumentRendererGetCaretRect(r,&p,&a)==XUI_OK && fabs(a.x-left.fWidth)<.001);
    p.iAffinity=XUI_DOC_AFTER;
    CHECK(xuiDocumentRendererGetCaretRect(r,&p,&b)==XUI_OK && fabs(b.x-size)<.001 && b.y>a.y);
    xuiTextShapeFree(&left);xuiTextShapeFree(&middle);xuiTextShapeFree(&right);
    xuiDocumentRendererRelease(r);xuiDocumentRelease(d);
}
static void hard_break(xui_context ctx,const xui_doc_renderer_desc_t* desc,float size)
{
    xui_document d; xui_document_transaction t; xui_document_snapshot s; xui_document_renderer r;
    xui_doc_node_desc_t atom={0}; uint64_t paragraph,first,last,brk; xui_doc_rect_t a,b;
    xui_doc_position_t p;
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
    paragraph=insert(t,1,NULL,0);first=insert(t,paragraph,HEB,0);
    atom.iSize=sizeof(atom);atom.iKind=XUI_DOC_HARD_BREAK;
    CHECK(xuiDocumentTxnInsertNode(t,paragraph,XUI_DOCUMENT_APPEND,&atom,&brk)==XUI_OK);
    last=insert(t,paragraph,"i )",0);
    CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentRendererCreate(ctx,desc,&r)==XUI_OK &&
        xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK);xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentRendererLayout(r,W,0,H)==XUI_OK);
    p=pos(d,first,0);CHECK(xuiDocumentRendererGetCaretRect(r,&p,&a)==XUI_OK && fabs(a.x-size)<.001);
    p=pos(d,last,0);CHECK(xuiDocumentRendererGetCaretRect(r,&p,&b)==XUI_OK && b.x>size*.7 && b.y>a.y);
    /* A HardBreak preserves the original RTL paragraph base. The trailing
     * bracket and intervening space appear physically before the Latin i. */
    p=pos(d,last,3);p.iAffinity=XUI_DOC_BEFORE;
    CHECK(xuiDocumentRendererGetCaretRect(r,&p,&a)==XUI_OK && fabs(a.x)<.001 && fabs(a.y-b.y)<.001);
    xuiDocumentRendererRelease(r);xuiDocumentRelease(d);
}
static void isolates(xui_context ctx,xui_proxy proxy,const xui_doc_renderer_desc_t* desc,
    xui_surface target,xui_surface reference)
{
    const char* text="f \xe2\x81\xa7" HEB "\xe2\x81\xa9 i";
    xui_doc_desc_t dd={0};xui_document d;xui_document_snapshot s;xui_document_renderer r;xui_draw_context draw;
    uint64_t paragraph,id,total;xui_doc_rect_t selected[4];xui_doc_range_t range;
    unsigned char pixels[W*H*4],expected[W*H*4];unsigned k;
    dd.iSize=sizeof(dd);dd.iProfile=XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&dd,&d)==XUI_OK && xuiDocumentLoadMarkdown(d,text,strlen(text))==XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentRendererCreate(ctx,desc,&r)==XUI_OK &&
        xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(s,1,0,&paragraph)==XUI_OK && xuiDocumentSnapshotGetChild(s,paragraph,0,&id)==XUI_OK);
    xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentRendererLayout(r,W,0,H)==XUI_OK && proxy->surfaceClear(proxy,target,0)==XUI_OK &&
        proxy->drawBegin(proxy,&draw,target)==XUI_OK && xuiDocumentRendererDraw(r,draw,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK &&
        proxy->drawEnd(proxy,draw)==XUI_OK && proxy->surfaceReadRGBA(proxy,target,pixels,W*4)==XUI_OK &&
        proxy->surfaceReadRGBA(proxy,reference,expected,W*4)==XUI_OK);
    for(k=0;k<W*H;k++)if(pixels[k*4+3]!=expected[k*4+3]) {
        size_t i;doc_render_block* block=&r->blocks[0];
        fprintf(stderr,"Isolate mismatch x=%u y=%u actual=%u expected=%u\n",k%W,k/W,pixels[k*4+3],expected[k*4+3]);
        for(i=0;i<block->fragment_count;i++)fprintf(stderr,"f%zu source=%llu..%llu level=%u flags=%x x=%.2f w=%.2f\n",i,
            (unsigned long long)block->fragments[i].start,(unsigned long long)block->fragments[i].end,
            block->fragments[i].bidi_level,block->fragments[i].flags,block->fragments[i].x,block->fragments[i].width);
        for(i=0;i<block->paint_group_count;i++)fprintf(stderr,"g%zu f=%zu..%zu level=%u text=[%s]\n",i,
            block->paint_groups[i].first_fragment,block->paint_groups[i].end_fragment,block->paint_groups[i].bidi_level,block->paint_groups[i].text);
        CHECK(0);
    }
    plain(d,"f \xe2\x81\xa7" HEB "\xe2\x81\xa9 i\n");
    range=(xui_doc_range_t){pos(d,id,0),pos(d,id,strlen(text))};
    CHECK(xuiDocumentRendererGetRangeRects(r,&range,selected,4,&total)==XUI_OK && total==1);
    xuiDocumentRendererRelease(r);xuiDocumentRelease(d);
}
static void real_scripts(xui_context ctx,xui_proxy proxy,const xui_doc_renderer_desc_t* desc,
    xui_surface target,xui_surface reference,float size)
{
    const char* arabic="\xd8\xb3\xd9\x84\xd8\xa7\xd9\x85";
    xui_font font;xui_doc_renderer_desc_t styled=*desc;xui_document d;xui_document_transaction t;
    xui_document_snapshot s;xui_document_renderer r;xui_draw_context draw;uint64_t paragraph,id;
    xui_text_shape_t a={0},h={0},whole={0};float advance=0;int i;unsigned k;
    unsigned char actual[W*H*4],expected[W*H*4];
    CHECK(proxy->fontLoadFile(proxy,&font,"C:/Windows/Fonts/segoeui.ttf",size,0)==XUI_OK);
    styled.tFonts=(xui_doc_font_set_t){font,font,font,font,font};
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
    paragraph=insert(t,1,NULL,0);id=insert(t,paragraph,HEB " \xd8\xb3\xd9\x84\xd8\xa7\xd9\x85",0);
    CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentRendererCreate(ctx,&styled,&r)==XUI_OK &&
        xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK);xuiDocumentSnapshotRelease(s);
    CHECK(xuiTextShape(ctx, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=arabic, .iTextSize=8, .iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RTL}, &a)==XUI_OK &&
        xuiTextShape(ctx, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=HEB " ", .iTextSize=7, .iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RTL}, &h)==XUI_OK);
    for(i=0;i<a.iClusterCount;i++)advance+=a.pClusters[i].fAdvance;
    CHECK(xuiTextShape(ctx, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=HEB " \xd8\xb3\xd9\x84\xd8\xa7\xd9\x85", .iTextSize=15, .iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RTL}, &whole)==XUI_OK &&
        fabs(whole.fWidth-fmaxf(a.fWidth,advance+h.fWidth))<.002);
    CHECK(xuiDocumentRendererLayout(r,W,0,H)==XUI_OK && proxy->surfaceClear(proxy,target,0)==XUI_OK &&
        proxy->drawBegin(proxy,&draw,target)==XUI_OK && xuiDocumentRendererDraw(r,draw,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK &&
        proxy->drawEnd(proxy,draw)==XUI_OK);
    CHECK(proxy->surfaceClear(proxy,reference,0)==XUI_OK && proxy->drawBegin(proxy,&draw,reference)==XUI_OK);
    /* The public native renderer receives the complete font/direction item,
     * independently of Document fragments, groups and visual layout. Script
     * sub-items retain context and fractional pens inside that item; there
     * is one rounded outer clip rather than an artificial script-edge clip.
     * Separate-script metrics above remain an independent width check. */
    CHECK(proxy->drawText(proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=HEB " \xd8\xb3\xd9\x84\xd8\xa7\xd9\x85", .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_CLIP|XUI_TEXT_RTL) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, (xui_rect_t){0,0,floorf(whole.fWidth+.5f),floorf(whole.fLineHeight+.5f)}, desc->iTextColor, XUI_TEXT_CLIP|XUI_TEXT_RTL)==XUI_OK &&
        proxy->drawEnd(proxy,draw)==XUI_OK && proxy->surfaceReadRGBA(proxy,target,actual,W*4)==XUI_OK &&
        proxy->surfaceReadRGBA(proxy,reference,expected,W*4)==XUI_OK);
    for(k=0;k<W*H;k++)if(actual[k*4+3]!=expected[k*4+3]) {
        fprintf(stderr,"Real-script union mismatch x=%u y=%u actual=%u expected=%u advance=%g\n",k%W,k/W,actual[k*4+3],expected[k*4+3],advance);
        fprintf(stderr,"Arabic h=%g ascent=%g width=%g; Hebrew h=%g ascent=%g width=%g; whole h=%g ascent=%g width=%g\n",
            a.fLineHeight,a.fAscent,a.fWidth,h.fLineHeight,h.fAscent,h.fWidth,whole.fLineHeight,whole.fAscent,whole.fWidth);
        for(size_t group=0;group<r->blocks[0].paint_group_count;group++){
            doc_render_paint_group* g=&r->blocks[0].paint_groups[group];doc_fragment* f=&r->blocks[0].fragments[g->origin_fragment];
            fprintf(stderr,"g%zu text=[%s] script=%x level=%u x=%g y=%g width=%g h=%g ascent=%g baseline=%g offset=%u\n",
                group,g->text,g->script,g->bidi_level,doc_render_group_x(&r->blocks[0],g),f->y,g->paint_width,g->line_height,g->ascent,f->baseline,g->context_offset);
        }
        CHECK(0);
    }
    { xui_doc_position_t p=pos(d,id,7);xui_doc_rect_t caret;
      CHECK(xuiDocumentRendererGetCaretRect(r,&p,&caret)==XUI_OK && fabs(caret.x-advance)<.002); }
    xuiTextShapeFree(&a);xuiTextShapeFree(&h);xuiTextShapeFree(&whole);xuiDocumentRendererRelease(r);xuiDocumentRelease(d);proxy->fontDestroy(proxy,font);
}
static void soft_hyphen(xui_context ctx,const xui_doc_renderer_desc_t* desc,float size)
{
    xui_document d;xui_document_transaction t;xui_document_snapshot s;xui_document_renderer r;
    uint64_t paragraph,id;xui_doc_position_t p;xui_doc_rect_t caret;
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
    /* RLO exercises an odd directional item inside an LTR paragraph with a
     * Unicode-legal discretionary break; HL/BA/HL itself forbids that cut. */
    paragraph=insert(t,1,NULL,0);id=insert(t,paragraph,"f \xe2\x80\xae" "aaa\xc2\xad" "aaa\xe2\x80\xac",0);
    CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentRendererCreate(ctx,desc,&r)==XUI_OK &&
        xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK);xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentRendererLayout(r,size*3.05+.01,0,H)==XUI_OK);
    p=pos(d,id,5);CHECK(xuiDocumentRendererGetCaretRect(r,&p,&caret)==XUI_OK);
    if(fabs(caret.x-size*3.05)>.001) {
        size_t i;doc_render_block* block=&r->blocks[0];fprintf(stderr,"Hyphen caret x=%.3f y=%.3f size=%.3f\n",caret.x,caret.y,size);
        for(i=0;i<block->fragment_count;i++)fprintf(stderr,"f%zu offset=%llu..%llu level=%u flags=%x x=%.2f w=%.2f y=%.2f\n",i,
            (unsigned long long)block->fragments[i].start,(unsigned long long)block->fragments[i].end,block->fragments[i].bidi_level,
            block->fragments[i].flags,block->fragments[i].x,block->fragments[i].width,block->fragments[i].y);
    }
    CHECK(fabs(caret.x-size*3.05)<.001);
    p=pos(d,id,8);p.iAffinity=XUI_DOC_BEFORE;
    CHECK(xuiDocumentRendererGetCaretRect(r,&p,&caret)==XUI_OK && fabs(caret.x-size*1.25)<.001);
    xuiDocumentRendererRelease(r);xuiDocumentRelease(d);
}
static void code_rows(xui_context ctx,const xui_doc_renderer_desc_t* desc,float size)
{
    xui_document d;xui_document_transaction t;xui_document_snapshot s;xui_document_renderer r;
    xui_doc_node_desc_t n={0};uint64_t id;xui_doc_position_t p;xui_doc_rect_t a,b;
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
    n.iSize=sizeof(n);n.iKind=XUI_DOC_CODE_BLOCK;n.sText=HEB "\r\nf " HEB;n.iTextBytes=strlen(n.sText);
    CHECK(xuiDocumentTxnInsertNode(t,1,XUI_DOCUMENT_APPEND,&n,&id)==XUI_OK && xuiDocumentTxnCommit(t,NULL)==XUI_OK);
    xuiDocumentTxnRelease(t);CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentRendererCreate(ctx,desc,&r)==XUI_OK &&
        xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK);xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentRendererLayout(r,W,0,H)==XUI_OK);
    p=pos(d,id,0);CHECK(xuiDocumentRendererGetCaretRect(r,&p,&a)==XUI_OK && fabs(a.x-8-size)<.001);
    p=pos(d,id,6);p.iAffinity=XUI_DOC_BEFORE;CHECK(xuiDocumentRendererGetCaretRect(r,&p,&b)==XUI_OK && fabs(b.x-8)<.001 && fabs(a.y-b.y)<.001);
    p=pos(d,id,8);CHECK(xuiDocumentRendererGetCaretRect(r,&p,&b)==XUI_OK && fabs(b.x-8)<.001 && b.y>a.y);
    xuiDocumentRendererRelease(r);xuiDocumentRelease(d);
}
static int object_measure(xui_document_snapshot s,xui_doc_node_id id,float width,float zoom,xui_vec2_t* size,float* baseline,void* user)
{
    (void)s;(void)id;(void)width;(void)user;*size=(xui_vec2_t){30*zoom,20*zoom};*baseline=16*zoom;return XUI_OK;
}
static void object_edges(xui_context ctx,const xui_doc_renderer_desc_t* desc,float size)
{
    xui_document d;xui_document_transaction t;xui_document_snapshot s;xui_document_renderer r;
    xui_doc_renderer_desc_t styled=*desc;xui_doc_node_desc_t n={0};uint64_t paragraph,id;
    xui_doc_position_t before,after,hit;xui_doc_rect_t a,b,clicked,selected[2];xui_doc_range_t range;uint64_t total;
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK);
    paragraph=insert(t,1,NULL,0);insert(t,paragraph,ALEPH,0);
    n.iSize=sizeof(n);n.iKind=XUI_DOC_MATH;n.sText="x^2";n.iTextBytes=3;
    CHECK(xuiDocumentTxnInsertNode(t,paragraph,XUI_DOCUMENT_APPEND,&n,&id)==XUI_OK);insert(t,paragraph,BET,0);
    CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);styled.onObjectMeasure=object_measure;
    CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentRendererCreate(ctx,&styled,&r)==XUI_OK &&
        xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK);xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentRendererLayout(r,W,0,H)==XUI_OK);
    before=pos(d,paragraph,1);before.iKind=XUI_DOC_POSITION_GAP;
    after=pos(d,paragraph,2);after.iKind=XUI_DOC_POSITION_GAP;after.iAffinity=XUI_DOC_BEFORE;
    CHECK(xuiDocumentRendererGetCaretRect(r,&before,&a)==XUI_OK && xuiDocumentRendererGetCaretRect(r,&after,&b)==XUI_OK &&
        fabs(a.x-size*.6-30)<.001 && fabs(b.x-size*.6)<.001);
    CHECK(xuiDocumentRendererHitTest(r,b.x+1,b.y+b.height*.5,&hit)==XUI_OK && hit.iKind==XUI_DOC_POSITION_GAP &&
        hit.iNodeId==paragraph && hit.iOffset==2 && xuiDocumentRendererGetCaretRect(r,&hit,&clicked)==XUI_OK && fabs(clicked.x-b.x)<.001);
    CHECK(xuiDocumentRendererHitTest(r,a.x-1,a.y+a.height*.5,&hit)==XUI_OK && hit.iNodeId==paragraph && hit.iOffset==1 &&
        xuiDocumentRendererGetCaretRect(r,&hit,&clicked)==XUI_OK && fabs(clicked.x-a.x)<.001);
    range=(xui_doc_range_t){before,after};CHECK(xuiDocumentRendererGetRangeRects(r,&range,selected,2,&total)==XUI_OK && total==1 &&
        fabs(selected[0].x-b.x)<.001 && fabs(selected[0].width-30)<.001);
    xuiDocumentRendererRelease(r);xuiDocumentRelease(d);
}
static int frame(void* user)
{
    xui_proxy_t proxy=xuiProxyXge(); xui_context ctx; xui_font font;
    xui_doc_renderer_desc_t desc={0}; xui_document d[3]; xui_document_renderer r[3]; xui_surface surface[3];
    uint64_t ids[3][3]={{0}}; xui_doc_rect_t caret[3][4]; unsigned char pixels[3][W*H*4];
    const char* letters[]={ALEPH,BET,GIMEL};
    const uint32_t colors[]={XUI_COLOR_RGBA(20,40,200,255),XUI_COLOR_RGBA(200,20,40,255),XUI_COLOR_RGBA(20,200,40,255)};
    float size=*(float*)user; unsigned v,k,pass,colored[3]={0};
#ifdef TEST_DRAW_TEXT_ONLY
    proxy.drawTextSpans=NULL;
#endif
    CHECK(proxy.fontLoadFile(&proxy,&font,XGE_TEST_OPENTYPE_FIXTURE,size,0)==XUI_OK);
    CHECK(xuiCreate(&ctx)==XUI_OK && xuiSetProxy(ctx,&proxy)==XUI_OK && xuiSetDefaultFont(ctx,font)==XUI_OK);
    desc.iSize=sizeof(desc); desc.tFonts=(xui_doc_font_set_t){font,font,font,font,font}; desc.iTextColor=colors[0];
    for(v=0;v<3;v++) {
        xui_document_snapshot s; xui_surface_desc_t target={0}; xui_doc_desc_t dd={0};
        dd.iSize=sizeof(dd); dd.iProfile=XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(v==2?&dd:NULL,&d[v])==XUI_OK);
        if(v==2) {
            uint64_t paragraph; const char* md=ALEPH "*" BET "*" GIMEL;
            CHECK(xuiDocumentLoadMarkdown(d[v],md,strlen(md))==XUI_OK && xuiDocumentAcquireSnapshot(d[v],&s)==XUI_OK);
            CHECK(xuiDocumentSnapshotGetChild(s,1,0,&paragraph)==XUI_OK);
            for(k=0;k<3;k++)CHECK(xuiDocumentSnapshotGetChild(s,paragraph,k,&ids[v][k])==XUI_OK);
            xuiDocumentSnapshotRelease(s);
        } else {
            xui_document_transaction t; uint64_t paragraph;
            CHECK(xuiDocumentBeginTransaction(d[v],NULL,&t)==XUI_OK); paragraph=insert(t,1,NULL,0);
            if(v)for(k=0;k<3;k++)ids[v][k]=insert(t,paragraph,letters[k],colors[k]);
            else ids[v][0]=ids[v][1]=ids[v][2]=insert(t,paragraph,HEB,0);
            CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK); xuiDocumentTxnRelease(t);
        }
        CHECK(xuiDocumentAcquireSnapshot(d[v],&s)==XUI_OK && xuiDocumentRendererCreate(ctx,&desc,&r[v])==XUI_OK &&
            xuiDocumentRendererSetSnapshot(r[v],s,NULL)==XUI_OK); xuiDocumentSnapshotRelease(s);
        target.iKind=XUI_SURFACE_KIND_TEXTURE; target.iFormat=XUI_SURFACE_FORMAT_RGBA8; target.iWidth=W; target.iHeight=H;
        target.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
        CHECK(proxy.surfaceCreate(&proxy,&surface[v],&target)==XUI_OK);
    }
    for(pass=0;pass<3;pass++)for(v=0;v<3;v++) {
        xui_draw_context draw;
        CHECK(xuiDocumentRendererLayout(r[v],pass==1?size+1:W,0,H)==XUI_OK);
        for(k=0;k<4;k++) {
            xui_doc_position_t p=pos(d[v],ids[v][k==3?2:k],v?(k==3?2:0):k*2);
            CHECK(xuiDocumentRendererGetCaretRect(r[v],&p,&caret[v][k])==XUI_OK);
            if(fabs(caret[v][k].x-caret[v][0].x+size*(k==0?0:k==1?.2:k==2?.75:1))>.001)
                fprintf(stderr,"RTL stop %u: %.3f origin %.3f size %.3f\n",k,caret[v][k].x,caret[v][0].x,size);
            CHECK(fabs(caret[v][k].x-caret[v][0].x+size*(k==0?0:k==1?.2:k==2?.75:1))<.001);
            CHECK(fabs(caret[v][k].y-caret[v][0].y)<.001);
            if(k>0 && k<3) {
                xui_doc_position_t hit; xui_doc_rect_t clicked;
                CHECK(xuiDocumentRendererHitTest(r[v],caret[v][k].x+.1,caret[v][k].y+10,&hit)==XUI_OK &&
                    xuiDocumentRendererGetCaretRect(r[v],&hit,&clicked)==XUI_OK && fabs(clicked.x-caret[v][k].x)<.001);
            }
        }
        {
            xui_doc_range_t range={pos(d[v],ids[v][0],0),pos(d[v],ids[v][v?1:0],v?0:2)};
            xui_doc_rect_t rects[4]; uint64_t total;
            CHECK(xuiDocumentRendererGetRangeRects(r[v],&range,rects,4,&total)==XUI_OK && total==1 &&
                fabs(rects[0].width-size*.2)<.001 && fabs(rects[0].x-caret[v][1].x)<.001);
            range.tCaret=pos(d[v],ids[v][2],v?2:6);
            CHECK(xuiDocumentRendererGetRangeRects(r[v],&range,rects,4,&total)==XUI_OK && total==1 && fabs(rects[0].width-size)<.001);
        }
        CHECK(proxy.surfaceClear(&proxy,surface[v],0)==XUI_OK && proxy.drawBegin(&proxy,&draw,surface[v])==XUI_OK &&
            xuiDocumentRendererDraw(r[v],draw,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK && proxy.drawEnd(&proxy,draw)==XUI_OK &&
            proxy.surfaceReadRGBA(&proxy,surface[v],pixels[v],W*4)==XUI_OK);
    }
    for(k=0;k<W*H;k++) {
        unsigned char* p=pixels[1]+k*4; unsigned zone;
        CHECK(pixels[0][k*4+3]==p[3] && pixels[0][k*4+3]==pixels[2][k*4+3]);
        if(p[3]<128)continue;
        zone=k%W+.5>caret[1][1].x?0:k%W+.5>caret[1][2].x?1:2;
        CHECK((zone==0 && p[2]>p[0]+80 && p[2]>p[1]+80) || (zone==1 && p[0]>p[1]+80 && p[0]>p[2]+80) ||
            (zone==2 && p[1]>p[0]+80 && p[1]>p[2]+80)); colored[zone]++;
    }
    CHECK(colored[0]>10 && colored[1]>10 && colored[2]>10);
    {
        xui_document_transaction t;xui_document_snapshot s;xui_doc_attributes_t attrs={0};xui_draw_context draw;
        unsigned char decorated[W*H*4];unsigned count=0;int left=W,right=-1;
        attrs.iTextColor=colors[1];attrs.iMarks=XUI_DOC_UNDERLINE;
        CHECK(xuiDocumentBeginTransaction(d[1],NULL,&t)==XUI_OK && xuiDocumentTxnSetAttributes(t,ids[1][1],&attrs)==XUI_OK &&
            xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentAcquireSnapshot(d[1],&s)==XUI_OK && xuiDocumentRendererSetSnapshot(r[1],s,NULL)==XUI_OK);
        xuiDocumentSnapshotRelease(s);CHECK(xuiDocumentRendererLayout(r[1],W,0,H)==XUI_OK &&
            proxy.surfaceClear(&proxy,surface[1],0)==XUI_OK && proxy.drawBegin(&proxy,&draw,surface[1])==XUI_OK &&
            xuiDocumentRendererDraw(r[1],draw,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK && proxy.drawEnd(&proxy,draw)==XUI_OK &&
            proxy.surfaceReadRGBA(&proxy,surface[1],decorated,W*4)==XUI_OK);
        for(k=0;k<W*H;k++) {
            unsigned char* p=decorated+k*4;int x=(int)(k%W);
            if(pixels[1][k*4+3] || p[3]<128 || p[0]<=p[1]+80 || p[0]<=p[2]+80)continue;
            if(x<left)left=x;
            if(x>right)right=x;
            count++;
        }
        CHECK(count>10 && abs(left-(int)floor(caret[1][2].x+.5))<=1 && abs(right-(int)floor(caret[1][1].x+.5)+1)<=1);
    }
    {
        xui_widget editor; xui_doc_editor_desc_t ed={0}; xui_doc_range_t selected={pos(d[0],ids[0][0],0),pos(d[0],ids[0][0],0)};
        ed.iSize=sizeof(ed);ed.tView.iSize=sizeof(ed.tView);ed.tView.pDocument=d[0];ed.tView.tRenderer=desc;ed.iMode=XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(ctx,&ed,&editor)==XUI_OK && xuiSetRootWidget(ctx,editor)==XUI_OK &&
            xuiWidgetSetRect(editor,(xui_rect_t){0,0,W,H})==XUI_OK && xuiLayout(ctx)==XUI_OK && xuiSetFocusWidget(ctx,editor)==XUI_OK &&
            xuiDocumentViewSetSelection(editor,&selected)==XUI_OK);
        for(k=1;k<=3;k++)CHECK(xuiInputKeyDown(ctx,XUI_KEY_LEFT,0)==XUI_OK && xuiDocumentViewGetSelection(editor,&selected)==XUI_OK && selected.tCaret.iOffset==k*2);
        for(k=3;k>0;k--)CHECK(xuiInputKeyDown(ctx,XUI_KEY_RIGHT,0)==XUI_OK && xuiDocumentViewGetSelection(editor,&selected)==XUI_OK && selected.tCaret.iOffset==(k-1)*2);
        CHECK(xuiInputKeyDown(ctx,XUI_KEY_HOME,0)==XUI_OK && xuiDocumentViewGetSelection(editor,&selected)==XUI_OK && selected.tCaret.iOffset==6);
        CHECK(xuiInputKeyDown(ctx,XUI_KEY_END,0)==XUI_OK && xuiDocumentViewGetSelection(editor,&selected)==XUI_OK && selected.tCaret.iOffset==0);
        CHECK(xuiInputKeyDown(ctx,XUI_KEY_LEFT,XUI_MOD_SHIFT)==XUI_OK && xuiDocumentViewGetSelection(editor,&selected)==XUI_OK &&
            selected.tAnchor.iOffset==0 && selected.tCaret.iOffset==2);
        CHECK(xuiInputKeyDown(ctx,XUI_KEY_RIGHT,0)==XUI_OK && xuiDocumentViewGetSelection(editor,&selected)==XUI_OK && selected.tCaret.iOffset==0);
        selected.tAnchor=pos(d[0],ids[0][0],0);selected.tCaret=pos(d[0],ids[0][0],6);
        CHECK(xuiDocumentViewSetSelection(editor,&selected)==XUI_OK && xuiInputKeyDown(ctx,XUI_KEY_LEFT,0)==XUI_OK &&
            xuiDocumentViewGetSelection(editor,&selected)==XUI_OK && selected.tCaret.iOffset==6);
        selected.tAnchor=selected.tCaret=pos(d[0],ids[0][0],2);
        CHECK(xuiDocumentViewSetSelection(editor,&selected)==XUI_OK && xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_BACKSPACE)==XUI_OK);
        plain(d[0],BET GIMEL "\n");CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK);plain(d[0],HEB "\n");
        selected.tAnchor=selected.tCaret=pos(d[0],ids[0][0],2);
        CHECK(xuiDocumentViewSetSelection(editor,&selected)==XUI_OK && xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_DELETE)==XUI_OK);
        plain(d[0],ALEPH GIMEL "\n");CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK);plain(d[0],HEB "\n");
        xuiWidgetDestroy(editor);
    }
    mixed(ctx,&proxy,font,&desc,surface[0],surface[1],size);
    isolates(ctx,&proxy,&desc,surface[2],surface[0]);
    hard_break(ctx,&desc,size);
    soft_hyphen(ctx,&desc,size);
    code_rows(ctx,&desc,size);
    object_edges(ctx,&desc,size);
    real_scripts(ctx,&proxy,&desc,surface[0],surface[1],size);
    CHECK(xuiDocumentLoadMarkdown(d[2],HEB,6)==XUI_OK);
    for(v=0;v<2;v++) {
        xui_document_snapshot s; xui_document_renderer source; xui_doc_position_t p=pos(d[2],1,0),hit;
        xui_draw_context draw;
        xui_doc_rect_t origin,stop,clicked; p.iKind=XUI_DOC_POSITION_SOURCE;
        CHECK(xuiDocumentAcquireSnapshot(d[2],&s)==XUI_OK && xuiDocumentRendererCreate(ctx,&desc,&source)==XUI_OK &&
            xuiDocumentRendererSetSnapshot(source,s,NULL)==XUI_OK);xuiDocumentSnapshotRelease(s);
        CHECK(xuiDocumentRendererSetMode(source,v?XUI_DOC_LIVE_MARKDOWN:XUI_DOC_SOURCE_TEXT)==XUI_OK &&
            (!v || xuiDocumentRendererSetActivePosition(source,&p)==XUI_OK) &&
            xuiDocumentRendererLayout(source,W,0,H)==XUI_OK && xuiDocumentRendererGetCaretRect(source,&p,&origin)==XUI_OK);
        for(k=1;k<4;k++) {
            p.iOffset=k*2;
            CHECK(xuiDocumentRendererGetCaretRect(source,&p,&stop)==XUI_OK &&
                fabs(stop.x-origin.x+size*(k==1?.2:k==2?.75:1))<.001);
            CHECK(xuiDocumentRendererHitTest(source,stop.x+.1,stop.y+10,&hit)==XUI_OK && hit.iKind==XUI_DOC_POSITION_SOURCE &&
                xuiDocumentRendererGetCaretRect(source,&hit,&clicked)==XUI_OK && fabs(clicked.x-stop.x)<.001);
        }
        CHECK(proxy.surfaceClear(&proxy,surface[2],0)==XUI_OK && proxy.drawBegin(&proxy,&draw,surface[2])==XUI_OK &&
            xuiDocumentRendererDraw(source,draw,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK && proxy.drawEnd(&proxy,draw)==XUI_OK &&
            proxy.surfaceReadRGBA(&proxy,surface[2],pixels[2],W*4)==XUI_OK);
        for(k=0;k<W*H;k++)CHECK(pixels[2][k*4+3]==pixels[1][k*4+3]);
        xuiDocumentRendererRelease(source);
    }
    document_ligature_marks(ctx,&proxy,&desc,surface,size,0);
    document_ligature_marks(ctx,&proxy,&desc,surface,size,1);
    document_font_fallback_cases(ctx,&proxy,&desc,surface,size);
    for(v=0;v<3;v++){proxy.surfaceDestroy(&proxy,surface[v]);xuiDocumentRendererRelease(r[v]);xuiDocumentRelease(d[v]);}
    xuiDestroy(ctx);proxy.fontDestroy(&proxy,font);
    puts("Native Document Bidi: Rich/Markdown/Source/Live RTL, mixed direction affinity/selection/wrap, isolates, HardBreak base, visual arrows/logical deletion/Undo and three-color GPU pixels passed");
    xgeQuit();return XGE_OK;
}
int main(void)
{
    xge_desc_t engine={0}; float sizes[]={40,37}; unsigned i;
    engine.iWidth=W;engine.iHeight=H;engine.sTitle="Document Bidi fixture";engine.iFlags=XGE_INIT_OFFSCREEN;engine.iRunMode=XGE_RUN_GAME_LOOP;
    for(i=0;i<sizeof(sizes)/sizeof(*sizes);i++){CHECK(xgeInit(&engine)==XGE_OK && xgeRun(frame,&sizes[i])==XGE_OK);xgeUnit();}
    return 0;
}
