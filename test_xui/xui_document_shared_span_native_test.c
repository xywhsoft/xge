/* Literal PUA glyphs are independent of contextual GSUB and style splitting. */
#include "../src/xui_document_layout_internal.h"
#include "../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xui_document_context_span_samples.h"
enum { W=160,H=180 };
#define CHECK(e) do{if(!(e)){fprintf(stderr,"shared span line %d: %s\n",__LINE__,#e);exit(1);}}while(0)
static xui_proxy_t original;
static unsigned shapes,raw,retained,cases;
static int shape_capture(xui_proxy p,const xui_text_item_t* item,xui_text_shape_t* out)
{shapes++;return original.textShape(p,item,out);}
static int raw_capture(xui_proxy p,xui_draw_context d,const xui_text_item_t* item,
    xui_rect_t rect,uint32_t color,uint32_t flags)
{raw++;return original.drawText(p,d,item,rect,color,flags);}
static int spans_capture(xui_proxy p,xui_draw_context d,const xui_text_item_t* item,
    xui_rect_t rect,uint32_t color,uint32_t flags,const xui_text_paint_span_t* spans,int count)
{raw++;return original.drawTextSpans(p,d,item,rect,color,flags,spans,count);}
static int range_capture(xui_proxy p,xui_draw_context d,const xui_text_shape_t* shape,
    int start,int end,xui_rect_t rect,uint32_t color,uint32_t flags,float offset,
    const xui_text_paint_span_t* spans,int count)
{
    CHECK(shape->pPaint && start>=0 && end<=2*SPAN_SCALAR_BYTES+1 && end>start && offset>=-.5f && offset<=.5f);
    for(int i=0;i<count;i++)CHECK(spans[i].iStart>=start && spans[i].iEnd<=end);
    retained++;return original.drawTextShapeRangeSpans(p,d,shape,start,end,rect,color,flags,offset,spans,count);
}
static void equal_pixels(xui_proxy p,xui_surface a,xui_surface b)
{
    unsigned char actual[W*H*4],expected[W*H*4];unsigned ink=0;
    CHECK(p->surfaceReadRGBA(p,a,actual,W*4)==XUI_OK && p->surfaceReadRGBA(p,b,expected,W*4)==XUI_OK);
    for(unsigned i=0;i<sizeof(actual);i++){
        if(actual[i]!=expected[i]){unsigned at=i/4*4;
            fprintf(stderr,"RGBA case %u byte %u: %u != %u; pixel=(%u,%u) actual=%u,%u,%u,%u expected=%u,%u,%u,%u\n",
                cases,i,actual[i],expected[i],i/4%W,i/4/W,actual[at],actual[at+1],actual[at+2],actual[at+3],
                expected[at],expected[at+1],expected[at+2],expected[at+3]);}
        CHECK(actual[i]==expected[i]);if(i%4==3 && actual[i])ink++;
    }
    CHECK(ink>20);cases++;
}
static void literal(xui_proxy p,xui_draw_context draw,xui_font font,const char* text,
    double x,double y,uint32_t color)
{
    xui_rect_t rect={(int)floor(x+.5),(int)floor(y+.5),W,H-(int)floor(y+.5)};
    CHECK(original.drawText(p,draw,&(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=font,
        .sText=text,.iTextSize=-1,.iFlags=XUI_TEXT_SHAPE_DEFAULT,.fDrawOffsetX=(float)(x-rect.fX)},rect,color,0)==XUI_OK);
}
static xui_document create_document(unsigned form,uint64_t* leaves,uint32_t* colors)
{
    xui_document d;xui_document_transaction txn;xui_document_snapshot snapshot;
    xui_doc_desc_t profile={0};xui_doc_node_desc_t node={0};uint64_t paragraph;
    profile.iSize=sizeof(profile);profile.iProfile=form==2?XUI_DOCUMENT_MARKDOWN:XUI_DOCUMENT_RICH;
    CHECK(xuiDocumentCreate(&profile,&d)==XUI_OK);
    if(form==2){
        CHECK(xuiDocumentLoadMarkdown(d,SPAN_MARKDOWN,sizeof(SPAN_MARKDOWN)-1)==XUI_OK && xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot,1,0,&paragraph)==XUI_OK);
        for(unsigned i=0;i<3;i++)CHECK(xuiDocumentSnapshotGetChild(snapshot,paragraph,i,&leaves[i])==XUI_OK);
        for(unsigned i=0;i<3;i++){
            xui_doc_node_info_t info={0};info.iSize=sizeof(info);
            CHECK(xuiDocumentSnapshotGetNode(snapshot,leaves[i],&info)==XUI_OK && info.iTextBytes==span_offsets[i+1]-span_offsets[i]);
            colors[i]=colors[0];
        }
        xuiDocumentSnapshotRelease(snapshot);return d;
    }else{
        CHECK(xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK);
        node.iSize=sizeof(node);node.iKind=XUI_DOC_PARAGRAPH;
        CHECK(xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
        for(unsigned i=0;i<(form?3u:1u);i++){
            node.iKind=XUI_DOC_TEXT;node.sText=form?SPAN_BODY+span_offsets[i]:SPAN_BODY;
            node.iTextBytes=form?span_offsets[i+1]-span_offsets[i]:sizeof(SPAN_BODY)-1;
            node.tAttributes.iTextColor=colors[i];
            CHECK(xuiDocumentTxnInsertNode(txn,paragraph,XUI_DOCUMENT_APPEND,&node,&leaves[i])==XUI_OK);
        }
    }
    CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);return d;
}
static void document_cases(xui_proxy p,xui_context context,xui_font font,float size,xui_surface* targets)
{
    const double advances[]={size*.8,size*.25,size*.9};
    const double widths[]={size*1.125,100,size*.75,size*.3,size*1.125};
    for(unsigned form=0;form<3;form++){
        uint32_t colors[]={UINT32_C(0xff2040c8),UINT32_C(0xffc82040),UINT32_C(0xff20c840)};
        uint64_t leaves[3]={0};xui_document d=create_document(form,leaves,colors);
        xui_document_renderer r;xui_document_snapshot snapshot;xui_doc_renderer_desc_t desc={0};
        void* key=NULL;
        desc.iSize=sizeof(desc);desc.tFonts=(xui_doc_font_set_t){font,font,font,font,font};desc.iTextColor=colors[0];
        CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererCreate(context,&desc,&r)==XUI_OK &&
            xuiDocumentRendererSetSnapshot(r,snapshot,NULL)==XUI_OK);xuiDocumentSnapshotRelease(snapshot);
        for(unsigned pass=0;pass<5;pass++){
            xui_draw_context draw;unsigned shaped;
            double ox=pass%2?.25:0;
            xui_rect_t clip=pass==3?(xui_rect_t){5,0,70,H}:(xui_rect_t){0,0,W,H};
            CHECK(xuiDocumentRendererLayout(r,widths[pass],0,H)==XUI_OK);
            doc_render_block* block=&r->blocks[0];
            CHECK(block->fragment_count==3);
            for(unsigned i=0;i<3;i++){
                if(fabs(block->fragments[i].width-advances[i])>=.001)
                    fprintf(stderr,"%s context form=%u pass=%u size=%g fragment=%u advance=%g expected=%g\n",SPAN_NAME,form,pass,size,i,block->fragments[i].width,advances[i]);
                CHECK(fabs(block->fragments[i].width-advances[i])<.001);
#ifdef TEST_RTL_CONTEXT
                {double x=0;for(unsigned j=i+1;j<3;j++)if(fabs(block->fragments[j].y-block->fragments[i].y)<.001)x+=advances[j];
                    CHECK(fabs(block->fragments[i].x-x)<.001 && (block->fragments[i].bidi_level & 1));}
#endif
            }
            if(form || pass!=1){
                CHECK(block->paint_seed_count==1 && block->paint_seeds[0].offsets &&
                    doc_render_seed_shape(block,&block->paint_seeds[0])->pPaint);
                void* current=doc_render_seed_shape(block,&block->paint_seeds[0])->pPaint;
                if(!key)key=current;else {
                    if(key!=current)fprintf(stderr,"cache change form=%u pass=%u size=%g shapes=%u\n",form,pass,size,shapes);
                    CHECK(key==current);
                }
                for(size_t i=0;i<block->paint_group_count;i++)CHECK(block->paint_groups[i].shared_seed==1);
            }
            if(pass==2 && form==1){
                xui_document_transaction txn;xui_document_change_set change;xui_doc_node_info_t info={0};
                info.iSize=sizeof(info);
                CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK &&
                    xuiDocumentSnapshotGetNode(snapshot,leaves[2],&info)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK);
                colors[2]=UINT32_C(0xffc88020);info.tAttributes.iTextColor=colors[2];
                CHECK(xuiDocumentTxnSetAttributes(txn,leaves[2],&info.tAttributes)==XUI_OK && xuiDocumentTxnCommit(txn,&change)==XUI_OK);
                xuiDocumentTxnRelease(txn);xuiDocumentSnapshotRelease(snapshot);
                shaped=shapes;
                CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererSetSnapshot(r,snapshot,change)==XUI_OK &&
                    xuiDocumentRendererLayout(r,widths[pass],0,H)==XUI_OK && shapes==shaped);
                xuiDocumentSnapshotRelease(snapshot);xuiDocumentChangeSetRelease(change);
                block=&r->blocks[0];CHECK(doc_render_seed_shape(block,&block->paint_seeds[0])->pPaint==key);
            }
            if(pass==4){xuiDocumentRelease(d);d=NULL;}
            shaped=shapes;raw=retained=0;
            CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->drawBegin(p,&draw,targets[0])==XUI_OK &&
                xuiDocumentRendererDraw(r,draw,ox,0,clip,NULL,0)==XUI_OK && p->drawEnd(p,draw)==XUI_OK && raw==0 && shapes==shaped);
            if(form || pass!=1){
                CHECK(retained==block->paint_group_count && retained>0);
            }
            CHECK(p->surfaceClear(p,targets[1],0)==XUI_OK && p->drawBegin(p,&draw,targets[1])==XUI_OK &&
                p->drawClipSet(p,draw,clip)==XUI_OK);
            for(unsigned i=0;i<3;i++)if(i!=1)literal(p,draw,font,i?SPAN_PUA_CONTEXT_LAST:SPAN_PUA_CONTEXT_FIRST,
                ox+block->fragments[i].x,block->fragments[i].y,colors[form?i:0]);
            CHECK(p->drawEnd(p,draw)==XUI_OK);equal_pixels(p,targets[0],targets[1]);
        }
        xuiDocumentRendererRelease(r);
    }
}
static void range_contract(xui_proxy p,xui_font font,xui_surface* targets)
{
    xui_text_shape_t shape={0};xui_vec2_t range={9,9};xui_draw_context draw;
    xui_text_item_t item={.iSize=sizeof(item),.pFont=font,.sText=SPAN_LIGATURE,.iTextSize=2*SPAN_SCALAR_BYTES,
        .iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RETAIN_PAINT|SPAN_DIRECTION};
    CHECK(original.textShape(p,&item,&shape)==XUI_OK && shape.pPaint);
    CHECK(original.textShapeRangeMeasure(p,&shape,0,SPAN_SCALAR_BYTES,&range)==XUI_ERROR_UNSUPPORTED && range.fX==0 && range.fY==0);
    CHECK(original.drawTextShapeRangeSpans(p,NULL,&shape,0,SPAN_SCALAR_BYTES,(xui_rect_t){0,0,W,H},~0u,0,0,NULL,0)==XUI_ERROR_UNSUPPORTED);
    CHECK(original.textShapeRangeMeasure(p,&shape,0,2*SPAN_SCALAR_BYTES,&range)==XUI_OK && range.fX>0 && range.fY>0);
    CHECK(original.drawTextShapeRangeSpans(p,NULL,&shape,0,2*SPAN_SCALAR_BYTES,(xui_rect_t){0,0,W,H},~0u,0,NAN,NULL,0)==XUI_ERROR_INVALID_ARGUMENT);
    CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->drawBegin(p,&draw,targets[0])==XUI_OK);
    xui_text_paint_span_t spans[]={{sizeof(*spans),0,SPAN_SCALAR_BYTES,UINT32_C(0xff2040c8)},{sizeof(*spans),SPAN_SCALAR_BYTES,2*SPAN_SCALAR_BYTES,UINT32_C(0xffc82040)}};
    CHECK(original.drawTextShapeRangeSpans(p,draw,&shape,0,2*SPAN_SCALAR_BYTES,(xui_rect_t){0,0,W,H},~0u,XUI_TEXT_CLIP,.25f,spans,2)==XUI_OK && p->drawEnd(p,draw)==XUI_OK);
    CHECK(p->surfaceClear(p,targets[1],0)==XUI_OK && p->drawBegin(p,&draw,targets[1])==XUI_OK);
    /* No GDEF stops in this fixture: the native contract uses equal ligature halves. */
    for(unsigned i=0;i<2;i++){
        int boundary=(int)floor(.25+range.fX/2+.5);
        unsigned side=SPAN_DIRECTION?1-i:i;
        CHECK(p->drawClipSet(p,draw,(xui_rect_t){side?boundary:0,0,side?W-boundary:boundary,H})==XUI_OK);
        literal(p,draw,font,SPAN_PUA_LIGATURE,.25,0,spans[i].iColor);
    }
    CHECK(p->drawEnd(p,draw)==XUI_OK);equal_pixels(p,targets[0],targets[1]);xuiTextShapeFree(&shape);
}
static void document_ligature(xui_proxy p,xui_context context,xui_font font,float size,xui_surface* targets)
{
    const uint32_t colors[]={UINT32_C(0xff2040c8),UINT32_C(0xffc82040)};
    xui_document d;xui_document_transaction txn;xui_document_snapshot snapshot;xui_document_renderer r;
    xui_doc_renderer_desc_t desc={0};xui_doc_node_desc_t node={0};uint64_t paragraph,leaf;
    CHECK(xuiDocumentCreate(NULL,&d)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK);
    node.iSize=sizeof(node);node.iKind=XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
    for(unsigned i=0;i<2;i++){
        node.iKind=XUI_DOC_TEXT;node.sText=SPAN_LIGATURE+i*SPAN_SCALAR_BYTES;node.iTextBytes=SPAN_SCALAR_BYTES;node.tAttributes.iTextColor=colors[i];
        CHECK(xuiDocumentTxnInsertNode(txn,paragraph,XUI_DOCUMENT_APPEND,&node,&leaf)==XUI_OK);
    }
    CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
    desc.iSize=sizeof(desc);desc.tFonts=(xui_doc_font_set_t){font,font,font,font,font};desc.iTextColor=colors[0];
    CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererCreate(context,&desc,&r)==XUI_OK &&
        xuiDocumentRendererSetSnapshot(r,snapshot,NULL)==XUI_OK);xuiDocumentSnapshotRelease(snapshot);
    for(unsigned pass=0;pass<3;pass++){
        xui_draw_context draw;
        CHECK(xuiDocumentRendererLayout(r,pass==1?size*.4:100,0,H)==XUI_OK);
        doc_render_block* block=&r->blocks[0];
        CHECK(block->fragment_count==2 && block->paint_seed_count==1);
        if(pass==1){
            CHECK(block->fragments[1].y>0 && fabs(block->fragments[0].width-size*.6)<.001 &&
                fabs(block->fragments[1].width-size*.25)<.001 && block->paint_group_count==2);
            for(unsigned i=0;i<2;i++)CHECK(!block->paint_groups[i].shared_seed);
        }else CHECK(block->paint_group_count==1 && block->paint_groups[0].shared_seed==1 &&
            fabs(block->fragments[0].width-size*.375)<.001 && fabs(block->fragments[1].width-size*.375)<.001);
        if(pass==2)xuiDocumentRelease(d);
        CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->drawBegin(p,&draw,targets[0])==XUI_OK &&
            xuiDocumentRendererDraw(r,draw,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK && p->drawEnd(p,draw)==XUI_OK &&
            p->surfaceClear(p,targets[1],0)==XUI_OK && p->drawBegin(p,&draw,targets[1])==XUI_OK);
        for(unsigned i=0;i<2;i++){
            int boundary=(int)floor(size*.375+.5);
            unsigned side=SPAN_DIRECTION?1-i:i;
            if(pass!=1)CHECK(p->drawClipSet(p,draw,(xui_rect_t){side?boundary:0,0,side?W-boundary:boundary,H})==XUI_OK);
            literal(p,draw,font,pass==1?(i?SPAN_PUA_LIG_LAST:SPAN_PUA_LIG_FIRST):SPAN_PUA_LIGATURE,
                pass==1?block->fragments[i].x:0,pass==1?block->fragments[i].y:0,colors[i]);
        }
        CHECK(p->drawEnd(p,draw)==XUI_OK);equal_pixels(p,targets[0],targets[1]);
    }
    xuiDocumentRendererRelease(r);
}
#ifdef TEST_RTL_CONTEXT
#include "xui_document_rtl_span_cases.h"
#endif
static int frame(void* user)
{
    float size=*(float*)user;xui_proxy_t p=original=xuiProxyXge();xui_context context;xui_font font;
    xui_surface targets[2];xui_surface_desc_t desc={0};
    p.textShape=shape_capture;p.drawText=raw_capture;p.drawTextSpans=spans_capture;p.drawTextShapeRangeSpans=range_capture;
#ifdef TEST_DRAW_TEXT_ONLY
    p.drawTextSpans=NULL;
#endif
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&p)==XUI_OK &&
        p.fontLoadFile(&p,&font,SPAN_FONT,size,0)==XUI_OK && xuiSetDefaultFont(context,font)==XUI_OK);
    desc.iWidth=W;desc.iHeight=H;desc.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
    CHECK(p.surfaceCreate(&p,&targets[0],&desc)==XUI_OK && p.surfaceCreate(&p,&targets[1],&desc)==XUI_OK);
#ifdef TEST_RTL_RANGE_ONLY
    rtl_range_cases(&p,font,size,targets);
#else
    document_cases(&p,context,font,size,targets);range_contract(&p,font,targets);
    document_ligature(&p,context,font,size,targets);
#ifdef TEST_RTL_CONTEXT
    rtl_range_cases(&p,font,size,targets);rtl_mixed_cases(&p,context,font,size,targets);
#endif
#endif
    p.surfaceDestroy(&p,targets[0]);p.surfaceDestroy(&p,targets[1]);xuiDestroy(context);p.fontDestroy(&p,font);
    xgeQuit();return XGE_OK;
}
int main(void)
{
    const float sizes[]={40,37};
    for(unsigned i=0;i<2;i++){
        xge_desc_t desc={0};float size=sizes[i];desc.iWidth=W;desc.iHeight=H;desc.sTitle="Document shared span";
        desc.iFlags=XGE_INIT_OFFSCREEN;desc.iRunMode=XGE_RUN_GAME_LOOP;
        CHECK(xgeInit(&desc)==XGE_OK && xgeRun(frame,&size)==XGE_OK);xgeUnit();
    }
    printf("Native %s shared spans: %u exact full RGBA cases; whole/cross-colour Rich and Markdown emphasis, contextual GSUB wraps, fractional X, clipping, width/cache reuse, Rich colour-only edits, released Document and ligature rejection/colour passed\n",SPAN_NAME,cases);
    return 0;
}
