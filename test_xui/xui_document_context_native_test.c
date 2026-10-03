/* Document context oracle: complete literal PUA glyph/mark union,
 * independent of the product's paragraph projection and item shaping. */
#include "../xge.h"
#include "../xui_document_ui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { W=400,H=160 };
static unsigned form,marks,barrier,pass,cases;
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s (form=%u marks=%u barrier=%u pass=%u)\n",__FILE__,__LINE__,#e,form,marks,barrier,pass); exit(1); } } while(0)
#define BEH "\xd8\xa8"
#define FATHA "\xd9\x8e"
#define ZWNJ "\xe2\x80\x8c"
#define RLE "\xe2\x80\xab"
#define PDF "\xe2\x80\xac"
static xui_proxy_t original;
static char displayed[1100];
static unsigned display_bytes,middle_offset;
static const char* retained_context;
static unsigned measured,drawn;
static xui_rect_t painted_rect;
static xui_font painted_font;
static uint32_t painted_color,painted_flags;
static void* target_paint;
static xui_font target_paint_font;
static int target_item(const xui_text_item_t* item)
{
    if(item->iTextSize!=2 || memcmp(item->sText,BEH,2) ||
        item->iContextOffset!=(int)middle_offset)return 0;
    CHECK(item->sContext && item->iContextSize==(int)display_bytes &&
        !memcmp(item->sContext,displayed,display_bytes) && item->sContext[display_bytes]==0);
    CHECK(item->iScript==UINT32_C(0x41726162) && item->sLanguage==NULL &&
        (item->iFlags & ~XUI_TEXT_SHAPE_RETAIN_PAINT)==(XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RTL));
    if(!retained_context)retained_context=item->sContext;
    CHECK(retained_context==item->sContext);
    return 1;
}
static int shape(xui_proxy proxy,const xui_text_item_t* item,xui_text_shape_t* out)
{
    int target=target_item(item),result;
    if(target)measured++;
    result=original.textShape(proxy,item,out);
    if(result==XUI_OK && target && out->pPaint){target_paint=out->pPaint;target_paint_font=item->pFont;}
    return result;
}
static void capture(const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags)
{
    if(!target_item(item))return;
    painted_rect=rect;painted_font=item->pFont;painted_color=color;painted_flags=flags;drawn++;
}
static int draw(xui_proxy proxy,xui_draw_context dc,const xui_text_item_t* item,
    xui_rect_t rect,uint32_t color,uint32_t flags)
{
    capture(item,rect,color,flags);
    return original.drawText(proxy,dc,item,rect,color,flags);
}
static int spans(xui_proxy proxy,xui_draw_context dc,const xui_text_item_t* item,
    xui_rect_t rect,uint32_t color,uint32_t flags,const xui_text_paint_span_t* colors,int count)
{
    if(target_item(item)){CHECK(count>0 && colors[0].iStart==0 && colors[count-1].iEnd==2);color=colors[0].iColor;}
    capture(item,rect,color,flags);
    return original.drawTextSpans(proxy,dc,item,rect,color,flags,colors,count);
}
static int cached_spans(xui_proxy proxy,xui_draw_context dc,const xui_text_shape_t* shape,
    int start,int end,xui_rect_t rect,uint32_t color,uint32_t flags,float offset,
    const xui_text_paint_span_t* spans,int count)
{
    int result=original.drawTextShapeRangeSpans(proxy,dc,shape,start,end,rect,color,flags,offset,spans,count);
    if(result==XUI_OK && shape->pPaint==target_paint){
        CHECK(start==0 && end==2 && shape->iTextSize==2 && (shape->iFlags & XUI_TEXT_SHAPE_RTL));
        if(count)color=spans[0].iColor;
        painted_rect=rect;painted_font=target_paint_font;painted_color=color;painted_flags=flags;drawn++;
    }
    return result;
}
static xui_doc_position_t position(xui_document document,uint64_t node,uint64_t offset,unsigned mode,uint32_t affinity)
{
    xui_doc_position_t p={0};p.iSize=sizeof(p);p.iDocumentId=xuiDocumentGetIdentity(document);
    p.iRevision=xuiDocumentGetRevision(document);p.iKind=mode==XUI_DOC_VISUAL?XUI_DOC_POSITION_TEXT:XUI_DOC_POSITION_SOURCE;
    p.iNodeId=mode==XUI_DOC_VISUAL?node:1;p.iOffset=offset;p.iAffinity=affinity;return p;
}
static int frame(void* user)
{
    float size=*(float*)user;unsigned lengths[]={0,5,8,32,512},i,j;
    xui_proxy_t proxy=original=xuiProxyXge();xui_context context;xui_font font,bold;
    xui_surface targets[2];xui_surface_desc_t surface={0};
    unsigned char actual[W*H*4],expected[W*H*4];
    proxy.textShape=shape;proxy.drawText=draw;proxy.drawTextSpans=spans;
    proxy.drawTextShapeRangeSpans=cached_spans;
#ifdef TEST_DRAW_TEXT_ONLY
    proxy.drawTextSpans=NULL;
#endif
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy)==XUI_OK &&
        proxy.fontLoadFile(&proxy,&font,"test/data/xge_context_fixture.ttf",size,0)==XUI_OK &&
        proxy.fontLoadFile(&proxy,&bold,"test/data/xge_context_fixture.ttf",size,0)==XUI_OK &&
        xuiSetDefaultFont(context,font)==XUI_OK);
    surface.iWidth=W;surface.iHeight=H;surface.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
    CHECK(proxy.surfaceCreate(&proxy,&targets[0],&surface)==XUI_OK && proxy.surfaceCreate(&proxy,&targets[1],&surface)==XUI_OK);
    for(i=0;i<sizeof(lengths)/sizeof(*lengths);i++)for(barrier=0;barrier<2;barrier++)for(form=0;form<7;form++){
        char source[1110];unsigned raw_bytes,raw_middle,mode=form==4?XUI_DOC_SOURCE_TEXT:form==5?XUI_DOC_LIVE_MARKDOWN:XUI_DOC_VISUAL;
        uint64_t middle=0;float middle_size=form==0?size-3:size;double advance=(barrier?.7:.8)*middle_size;
        xui_document document;xui_document_snapshot snapshot;xui_document_renderer renderer;
        xui_doc_desc_t profile={0};xui_doc_renderer_desc_t desc={0};xui_doc_position_t start,end,hit;
        xui_doc_rect_t first,last,clicked;uint64_t initial_run_bytes=0;
        target_paint=NULL;marks=lengths[i];display_bytes=0;memcpy(displayed,BEH,2);display_bytes=2;
        for(j=0;j<marks;j++){memcpy(displayed+display_bytes,FATHA,2);display_bytes+=2;}
        if(barrier){memcpy(displayed+display_bytes,ZWNJ,3);display_bytes+=3;}
        middle_offset=display_bytes;memcpy(displayed+display_bytes,BEH BEH,4);display_bytes+=4;displayed[display_bytes]=0;
        raw_middle=middle_offset;raw_bytes=display_bytes;memcpy(source,displayed,display_bytes+1);
        if(form>=2){
            memcpy(source+middle_offset,RLE BEH PDF BEH,10);raw_middle+=3;raw_bytes+=6;source[raw_bytes]=0;
        }
        profile.iSize=sizeof(profile);profile.iProfile=form>=3 && form<=5?XUI_DOCUMENT_MARKDOWN:XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile,&document)==XUI_OK);
        if(profile.iProfile==XUI_DOCUMENT_MARKDOWN){
            uint64_t paragraph;
            CHECK(xuiDocumentLoadMarkdown(document,source,raw_bytes)==XUI_OK &&
                xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK && xuiDocumentSnapshotGetChild(snapshot,1,0,&paragraph)==XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot,paragraph,0,&middle)==XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
        }else{
            xui_document_transaction transaction;xui_doc_node_desc_t node={0};uint64_t paragraph,ignored;
            CHECK(xuiDocumentBeginTransaction(document,NULL,&transaction)==XUI_OK);
            node.iSize=sizeof(node);node.iKind=form==6?XUI_DOC_CODE_BLOCK:XUI_DOC_PARAGRAPH;
            if(form==6){node.sText=source;node.iTextBytes=raw_bytes;}
            CHECK(xuiDocumentTxnInsertNode(transaction,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
            if(form==6)middle=paragraph;
            else{
                node.iKind=XUI_DOC_TEXT;node.sText=source;node.iTextBytes=form<2?raw_middle:raw_bytes;
                CHECK(xuiDocumentTxnInsertNode(transaction,paragraph,XUI_DOCUMENT_APPEND,&node,&middle)==XUI_OK);
                if(form<2){
                    node.sText=BEH;node.iTextBytes=2;node.tAttributes.fFontSize=middle_size;
                    node.tAttributes.iMarks=form==1?XUI_DOC_BOLD:0;
                    CHECK(xuiDocumentTxnInsertNode(transaction,paragraph,XUI_DOCUMENT_APPEND,&node,&middle)==XUI_OK);
                    node.tAttributes=(xui_doc_attributes_t){0};
                    CHECK(xuiDocumentTxnInsertNode(transaction,paragraph,XUI_DOCUMENT_APPEND,&node,&ignored)==XUI_OK);
                }
            }
            CHECK(xuiDocumentTxnCommit(transaction,NULL)==XUI_OK);xuiDocumentTxnRelease(transaction);
        }
        desc.iSize=sizeof(desc);desc.tFonts=(xui_doc_font_set_t){font,bold,font,bold,font};
        desc.iTextColor=XUI_COLOR_RGBA(255,255,255,255);desc.iCodeBackground=XUI_COLOR_RGBA(1,0,0,0);
        CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK && xuiDocumentRendererCreate(context,&desc,&renderer)==XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer,snapshot,NULL)==XUI_OK);xuiDocumentSnapshotRelease(snapshot);
        start=position(document,middle,form<2?0:raw_middle,mode,XUI_DOC_AFTER);
        end=start;end.iOffset+=2;end.iAffinity=XUI_DOC_BEFORE;
        CHECK(xuiDocumentRendererSetMode(renderer,mode)==XUI_OK &&
            (mode!=XUI_DOC_LIVE_MARKDOWN || xuiDocumentRendererSetActivePosition(renderer,&start)==XUI_OK));
        /* The retained snapshot/layout, not the caller's live Document, owns
         * the paragraph context during all of the following draws/reflows. */
        xuiDocumentRelease(document);retained_context=NULL;measured=0;
        for(pass=0;pass<3;pass++){
            xui_draw_context dc;xui_font_metrics_t metrics={0},base_metrics={0};xui_doc_renderer_stats_t stats={0};
            double snapped_left,snapped_width;
            unsigned pixels=0,x,y;char pua[4]={0xee,0x84,barrier?0x87:0x88,0};
            char right[1100]={0xee,0x84,barrier?0x86:0x87,0},left[]={0xee,0x84,0x89,0};
            xui_rect_t neighbors[2];double neighbor_x[2];
            if(pass==2){CHECK(xuiDocumentRendererInvalidateFonts(renderer)==XUI_OK);retained_context=NULL;}
            /* SOURCE's row tree currently clears row layouts on resize.
             * Visual/Live reflow must reuse their retained context and runs. */
            if(pass==1 && mode==XUI_DOC_SOURCE_TEXT)retained_context=NULL;
            CHECK(xuiDocumentRendererLayout(renderer,pass==1?180:W,0,H)==XUI_OK && measured>0 &&
                xuiDocumentRendererGetCaretRect(renderer,&start,&first)==XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer,&end,&last)==XUI_OK);
            if(fabs(fabs(last.x-first.x)-advance)>.002)fprintf(stderr,"caret width=%g expected=%g\n",fabs(last.x-first.x),advance);
            CHECK(fabs(fabs(last.x-first.x)-advance)<.002 && fabs(last.y-first.y)<.002);
            CHECK(xuiDocumentRendererHitTest(renderer,(first.x+last.x)*.5,first.y+first.height*.5,&hit)==XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer,&hit,&clicked)==XUI_OK &&
                fmin(fabs(clicked.x-first.x),fabs(clicked.x-last.x))<.002);
            stats.iSize=sizeof(stats);CHECK(xuiDocumentRendererGetStats(renderer,&stats)==XUI_OK);
            if(!pass)initial_run_bytes=stats.iTextRunBytes;
            if(pass==1 && mode!=XUI_DOC_SOURCE_TEXT)CHECK(stats.iTextRunBytes==initial_run_bytes);
            drawn=0;
            CHECK(proxy.surfaceClear(&proxy,targets[0],0)==XUI_OK && proxy.drawBegin(&proxy,&dc,targets[0])==XUI_OK &&
                xuiDocumentRendererDraw(renderer,dc,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK &&
                proxy.drawEnd(&proxy,dc)==XUI_OK && drawn>0 && proxy.surfaceReadRGBA(&proxy,targets[0],actual,W*4)==XUI_OK);
            CHECK(proxy.fontGetMetrics(&proxy,painted_font,&metrics)==XUI_OK);
            snapped_left=floor(fmin(first.x,last.x)+.5);
            snapped_width=floor(fmax(first.x,last.x)+.5)-snapped_left;
            if(fabsf(metrics.fSize-middle_size)>=.001f || fabs(painted_rect.fW-snapped_width)>=.002 || fabs(painted_rect.fX-snapped_left)>=.002)
                fprintf(stderr,"paint size=%g expected=%g rect=%g,%g,%g,%g carets=%g,%g advance=%g\n",(double)metrics.fSize,(double)middle_size,
                    (double)painted_rect.fX,(double)painted_rect.fY,(double)painted_rect.fW,(double)painted_rect.fH,first.x,last.x,advance);
            CHECK(fabsf(metrics.fSize-middle_size)<.001f &&
                fabs(painted_rect.fW-snapped_width)<.002 && fabs(painted_rect.fX-snapped_left)<.002);
            CHECK(original.fontGetMetrics(&original,font,&base_metrics)==XUI_OK);
            memcpy(right+3,displayed+2,marks*2);right[3+marks*2]=0;
            neighbor_x[0]=fmin(first.x,last.x)-size*.9;
            neighbor_x[1]=fmax(first.x,last.x);
            for(j=0;j<2;j++)neighbors[j]=(xui_rect_t){
                (float)floor(neighbor_x[j]+.5),
                (float)floor(first.y+metrics.fAscent-base_metrics.fAscent+.5),
                (float)ceil(size*(j?(barrier?.6:.7):.9)),(float)ceil(base_metrics.fLineHeight)};
            CHECK(original.surfaceClear(&original,targets[1],0)==XUI_OK && original.drawBegin(&original,&dc,targets[1])==XUI_OK &&
                original.drawText(&original,dc,&(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=painted_font,.sText=pua,.iTextSize=3,
                    .iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RTL,.fDrawOffsetX=(float)(fmin(first.x,last.x)-painted_rect.fX)},painted_rect,painted_color,painted_flags)==XUI_OK &&
                /* Compare the complete literal-glyph union. A neighbouring
                 * glyph can legitimately bleed into the target advance when
                 * an item boundary is no longer an artificial ink clip. */
                original.drawText(&original,dc,&(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=font,.sText=left,.iTextSize=3,
                    .iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RTL,.iScript=UINT32_C(0x41726162),
                    .fDrawOffsetX=(float)(neighbor_x[0]-neighbors[0].fX)},neighbors[0],painted_color,painted_flags)==XUI_OK &&
                original.drawText(&original,dc,&(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=font,.sText=right,.iTextSize=(int)(3+marks*2),
                    .iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RTL,.iScript=UINT32_C(0x41726162),
                    .fDrawOffsetX=(float)(neighbor_x[1]-neighbors[1].fX)},neighbors[1],painted_color,painted_flags)==XUI_OK &&
                original.drawEnd(&original,dc)==XUI_OK && original.surfaceReadRGBA(&original,targets[1],expected,W*4)==XUI_OK);
            for(y=0;y<H;y++)for(x=0;x<W;x++){
                unsigned p=y*W+x;
                if(actual[p*4+3]!=expected[p*4+3])fprintf(stderr,"alpha mismatch x=%u y=%u actual=%u expected=%u\n",x,y,actual[p*4+3],expected[p*4+3]);
                CHECK(actual[p*4+3]==expected[p*4+3]);pixels+=actual[p*4+3]>0;
            }
            CHECK(pixels>20);cases++;
        }
        xuiDocumentRendererRelease(renderer);
    }
    proxy.surfaceDestroy(&proxy,targets[0]);proxy.surfaceDestroy(&proxy,targets[1]);
    xuiDestroy(context);proxy.fontDestroy(&proxy,font);proxy.fontDestroy(&proxy,bold);
    printf("Native Document retained context: %u cumulative cases at %g; 0/5/8/32/512 marks, ZWNJ, size/font/Bidi boundaries, Rich/Markdown Visual/Source/Live/code, source projection, width reflow, font invalidation, released Document, shared measure/draw context, literal glyph advance and exact full-frame GPU glyph/mark union passed\n",cases,(double)size);
    xgeQuit();return XGE_OK;
}
int main(void)
{
    xge_desc_t engine={0};float sizes[]={40,37};unsigned i;
    engine.iWidth=W;engine.iHeight=H;engine.sTitle="Document retained paragraph context";engine.iFlags=XGE_INIT_OFFSCREEN;engine.iRunMode=XGE_RUN_GAME_LOOP;
    for(i=0;i<2;i++){CHECK(xgeInit(&engine)==XGE_OK && xgeRun(frame,&sizes[i])==XGE_OK);xgeUnit();}return 0;
}
