/* Actual GPU output is compared to literal PUA glyph aliases in a project-owned
 * font. The oracle never shapes a clipped substring of the contextual text. */
#include "../src/xui_document_layout_internal.h"
#include "../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { W=160,H=160 };
#define CHECK(e) do{if(!(e)){fprintf(stderr,"ASCII retained paint line %d: %s\n",__LINE__,#e);exit(1);}}while(0)
static xui_proxy_t original;
static unsigned retained_draws,raw_draws,shapes;
static xui_rect_t rows[2];
static int capture_shape(xui_proxy p,const xui_text_item_t* item,xui_text_shape_t* shape)
{shapes++;return original.textShape(p,item,shape);}
static int capture_raw(xui_proxy p,xui_draw_context draw,const xui_text_item_t* item,
    xui_rect_t rect,uint32_t color,uint32_t flags)
{raw_draws++;return original.drawText(p,draw,item,rect,color,flags);}
static int capture_range(xui_proxy p,xui_draw_context draw,const xui_text_shape_t* shape,
    int start,int end,xui_rect_t rect,uint32_t color,uint32_t flags)
{
    CHECK(retained_draws<2 && start==(retained_draws?2:0) && end==(retained_draws?3:2));
    rows[retained_draws++]=rect;
    return original.drawTextShapeRange(p,draw,shape,start,end,rect,color,flags);
}
static int capture_range_spans(xui_proxy p,xui_draw_context draw,const xui_text_shape_t* shape,
    int start,int end,xui_rect_t rect,uint32_t color,uint32_t flags,float offset,
    const xui_text_paint_span_t* spans,int count)
{
    CHECK(retained_draws<2 && start==(retained_draws?2:0) && end==(retained_draws?3:2));
    rows[retained_draws++]=rect;
    CHECK(offset==0);
    return original.drawTextShapeRangeSpans(p,draw,shape,start,end,rect,color,flags,offset,spans,count);
}
static void pixels_equal(xui_proxy p,xui_surface actual,xui_surface expected)
{
    unsigned char a[W*H*4],b[W*H*4];unsigned i,ink=0;
    CHECK(p->surfaceReadRGBA(p,actual,a,W*4)==XUI_OK && p->surfaceReadRGBA(p,expected,b,W*4)==XUI_OK);
    for(i=0;i<sizeof(a);i++){
        if(a[i]!=b[i])fprintf(stderr,"pixel byte %u: %u != %u\n",i,a[i],b[i]);
        CHECK(a[i]==b[i]);if(i%4==3 && a[i])ink++;
    }
    CHECK(ink>20);
}
static void literal(xui_proxy p,xui_draw_context draw,xui_font font,const char* text,xui_rect_t rect)
{
    CHECK(original.drawText(p,draw,&(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=font,
        .sText=text,.iTextSize=-1,.iFlags=XUI_TEXT_SHAPE_DEFAULT},rect,UINT32_C(0xff314159),XUI_TEXT_CLIP)==XUI_OK);
}
static void document_cases(xui_proxy p,xui_context context,xui_font font,xui_surface* targets)
{
    unsigned profile,pass;
    for(profile=0;profile<2;profile++){
        xui_document document;xui_document_transaction transaction;xui_document_snapshot snapshot;
        xui_document_renderer renderer;xui_doc_desc_t desc={0};xui_doc_node_desc_t child={0};
        xui_doc_renderer_desc_t layout={0};xui_doc_position_t before={0},after;
        xui_doc_rect_t caret_before,caret_after;uint64_t paragraph,leaf;
        desc.iSize=sizeof(desc);desc.iProfile=profile?XUI_DOCUMENT_MARKDOWN:XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&desc,&document)==XUI_OK);
        if(profile){
            CHECK(xuiDocumentLoadMarkdown(document,"a b",3)==XUI_OK &&
                xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot,1,0,&paragraph)==XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot,paragraph,0,&leaf)==XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
        }else{
            CHECK(xuiDocumentBeginTransaction(document,NULL,&transaction)==XUI_OK);
            child.iSize=sizeof(child);child.iKind=XUI_DOC_PARAGRAPH;
            CHECK(xuiDocumentTxnInsertNode(transaction,1,XUI_DOCUMENT_APPEND,&child,&paragraph)==XUI_OK);
            child.iKind=XUI_DOC_TEXT;child.sText="a b";child.iTextBytes=3;
            CHECK(xuiDocumentTxnInsertNode(transaction,paragraph,XUI_DOCUMENT_APPEND,&child,&leaf)==XUI_OK &&
                xuiDocumentTxnCommit(transaction,NULL)==XUI_OK);xuiDocumentTxnRelease(transaction);
        }
        layout.iSize=sizeof(layout);layout.tFonts=(xui_doc_font_set_t){font,font,font,font,font};
        layout.iTextColor=UINT32_C(0xff314159);
        CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK &&
            xuiDocumentRendererCreate(context,&layout,&renderer)==XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer,snapshot,NULL)==XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        before.iSize=sizeof(before);before.iDocumentId=xuiDocumentGetIdentity(document);
        before.iRevision=xuiDocumentGetRevision(document);before.iKind=XUI_DOC_POSITION_TEXT;
        before.iNodeId=leaf;before.iOffset=2;before.iAffinity=XUI_DOC_BEFORE;
        after=before;after.iAffinity=XUI_DOC_AFTER;
        for(pass=0;pass<3;pass++){
            xui_draw_context draw;unsigned measured;
            CHECK(xuiDocumentRendererLayout(renderer,pass==1?100:45,0,H)==XUI_OK);
            if(pass==1)continue;
            CHECK(xuiDocumentRendererGetCaretRect(renderer,&before,&caret_before)==XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer,&after,&caret_after)==XUI_OK &&
                fabs(caret_before.x-42)<.001 && caret_before.y==0 &&
                caret_after.x==0 && caret_after.y>0);
            CHECK(renderer->blocks[0].runs[0].shape.pPaint &&
                renderer->blocks[0].runs[0].shape.iPaintBytes>3*sizeof(xge_glyph_position_t) &&
                renderer->blocks[0].cache_bytes>=renderer->blocks[0].runs[0].shape.iPaintBytes+
                    renderer->blocks[0].fragment_capacity*sizeof(doc_fragment));
            retained_draws=raw_draws=0;measured=shapes;
            CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->drawBegin(p,&draw,targets[0])==XUI_OK &&
                xuiDocumentRendererDraw(renderer,draw,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK &&
                p->drawEnd(p,draw)==XUI_OK && retained_draws==2 && raw_draws==0 && shapes==measured);
            CHECK(p->surfaceClear(p,targets[1],0)==XUI_OK && p->drawBegin(p,&draw,targets[1])==XUI_OK);
            literal(p,draw,font,"\xee\x90\x84 ",rows[0]);literal(p,draw,font,"\xee\x90\x85",rows[1]);
            CHECK(p->drawEnd(p,draw)==XUI_OK);pixels_equal(p,targets[0],targets[1]);
        }
        xuiDocumentRelease(document); /* Renderer owns the immutable snapshot. */
        CHECK(xuiDocumentRendererLayout(renderer,45,0,H)==XUI_OK);
        retained_draws=raw_draws=0;
        {xui_draw_context draw;CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->drawBegin(p,&draw,targets[0])==XUI_OK &&
            xuiDocumentRendererDraw(renderer,draw,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK && p->drawEnd(p,draw)==XUI_OK);}
        CHECK(retained_draws==2);pixels_equal(p,targets[0],targets[1]);xuiDocumentRendererRelease(renderer);
    }
}
static void lifetime_and_cuts(xui_proxy p,xui_surface* targets,xui_font reference)
{
    xui_font font;xui_text_shape_t a={0},b={0},ligature={0},empty={0};xui_draw_context draw;
    char source[]="a b";xui_text_item_t item={0};xui_rect_t rect={0,0,42,50};
    CHECK(p->fontLoadFile(p,&font,"test/data/xge_ascii_context_fixture.ttf",40,0)==XUI_OK);
    item.iSize=sizeof(item);item.pFont=font;item.sText=source;item.iTextSize=3;
    item.iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RETAIN_PAINT;
    CHECK(original.textShape(p,&item,&a)==XUI_OK && original.textShape(p,&item,&b)==XUI_OK);
    item.sText="fi";item.iTextSize=2;
    CHECK(original.textShape(p,&item,&ligature)==XUI_OK && ligature.iClusterCount==1);
    CHECK(original.drawTextShapeRange(p,NULL,&ligature,0,1,rect,~0u,0)==XUI_ERROR_UNSUPPORTED &&
        original.drawTextShapeRange(p,NULL,&ligature,1,2,rect,~0u,0)==XUI_ERROR_UNSUPPORTED &&
        original.drawTextShapeRange(p,NULL,&ligature,-1,2,rect,~0u,0)==XUI_ERROR_INVALID_ARGUMENT);
    item.sText="";item.iTextSize=0;CHECK(original.textShape(p,&item,&empty)==XUI_OK && !empty.pPaint);
    memset(source,'x',3);p->fontDestroy(p,font); /* Neither source bytes nor the caller's font lifetime are borrowed. */
    xuiTextShapeFree(&b);
    CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->drawBegin(p,&draw,targets[0])==XUI_OK &&
        original.drawTextShapeRange(p,draw,&a,0,2,rect,UINT32_C(0xff314159),XUI_TEXT_CLIP)==XUI_OK &&
        p->drawEnd(p,draw)==XUI_OK && p->surfaceClear(p,targets[1],0)==XUI_OK &&
        p->drawBegin(p,&draw,targets[1])==XUI_OK);
    literal(p,draw,reference,"\xee\x90\x84 ",rect);CHECK(p->drawEnd(p,draw)==XUI_OK);
    pixels_equal(p,targets[0],targets[1]);xuiTextShapeFree(&a);
    /* The last outstanding shape still keeps the primary/fallback carriers alive. */
    CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->drawBegin(p,&draw,targets[0])==XUI_OK &&
        original.drawTextShapeRange(p,draw,&ligature,0,2,rect,UINT32_C(0xff314159),XUI_TEXT_CLIP)==XUI_OK &&
        p->drawEnd(p,draw)==XUI_OK && p->surfaceClear(p,targets[1],0)==XUI_OK && p->drawBegin(p,&draw,targets[1])==XUI_OK);
    literal(p,draw,reference,"\xee\x90\x88",rect);CHECK(p->drawEnd(p,draw)==XUI_OK);
    pixels_equal(p,targets[0],targets[1]);xuiTextShapeFree(&ligature);xuiTextShapeFree(&ligature);xuiTextShapeFree(&empty);
}
static int frame(void* ignored)
{
    xui_proxy_t proxy=original=xuiProxyXge();xui_context context;xui_font font;
    xui_surface targets[2];xui_surface_desc_t desc={0};(void)ignored;
    proxy.textShape=capture_shape;proxy.drawText=capture_raw;proxy.drawTextShapeRange=capture_range;
    proxy.drawTextShapeRangeSpans=capture_range_spans;
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy)==XUI_OK &&
        proxy.fontLoadFile(&proxy,&font,"test/data/xge_ascii_context_fixture.ttf",40,0)==XUI_OK &&
        xuiSetDefaultFont(context,font)==XUI_OK);
    desc.iWidth=W;desc.iHeight=H;desc.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
    CHECK(proxy.surfaceCreate(&proxy,&targets[0],&desc)==XUI_OK && proxy.surfaceCreate(&proxy,&targets[1],&desc)==XUI_OK);
    document_cases(&proxy,context,font,targets);xuiDestroy(context);
    lifetime_and_cuts(&proxy,targets,font);
    proxy.surfaceDestroy(&proxy,targets[0]);proxy.surfaceDestroy(&proxy,targets[1]);proxy.fontDestroy(&proxy,font);
    puts("Native retained ASCII paint: Rich/Markdown wrapping, affinity, width-only reflow, cache accounting, no draw reshaping, released Document/context/font/source, multiple owners and ligature-cut rejection; exact full RGBA oracle passed");
    xgeQuit();return XGE_OK;
}
int main(void)
{
    xge_desc_t desc={0};desc.iWidth=W;desc.iHeight=H;desc.sTitle="Document ASCII contextual glyphs";
    desc.iFlags=XGE_INIT_OFFSCREEN;desc.iRunMode=XGE_RUN_GAME_LOOP;
    CHECK(xgeInit(&desc)==XGE_OK && xgeRun(frame,NULL)==XGE_OK);xgeUnit();return 0;
}
