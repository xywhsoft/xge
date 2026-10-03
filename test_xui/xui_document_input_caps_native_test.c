#include "../xge.h"
#include "../xui_document_ui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { W=320, H=120 };
static unsigned form;
#define CHECK(e) do {if(!(e)){fprintf(stderr,"Document input caps line %d form=%u: %s\n",__LINE__,form,#e);exit(1);}}while(0)
static xui_proxy_t original;
static xui_rect_t paint_rect;
static float paint_offset;
static unsigned paints;
static void* retained_input;
static int shape_text(xui_proxy p,const xui_text_item_t* item,xui_text_shape_t* out)
{
    int target=item->iTextSize==6 && !memcmp(item->sText,"ii\xc3\xa9ii",6);
#ifdef TEST_NO_HB
    if(target)CHECK(!item->sContext && !item->iScript && !item->sLanguage && !(item->iFlags & XUI_TEXT_SHAPE_RTL));
#endif
    int result=original.textShape(p,item,out);
    if(result==XUI_OK && target && out->pPaint)retained_input=out->pPaint;
    return result;
}
static void capture(const xui_text_item_t* item,xui_rect_t rect)
{
    if(item->iTextSize!=6 || memcmp(item->sText,"ii\xc3\xa9ii",6))return;
#ifdef TEST_NO_HB
    CHECK(!item->sContext && !item->iScript && !item->sLanguage && !(item->iFlags & XUI_TEXT_SHAPE_RTL));
#endif
    paint_rect=rect;paint_offset=item->fDrawOffsetX;paints++;
}
static int draw_text(xui_proxy p,xui_draw_context draw,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags)
{capture(item,rect);return original.drawText(p,draw,item,rect,color,flags);}
static int draw_spans(xui_proxy p,xui_draw_context draw,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags,
    const xui_text_paint_span_t* spans,int count)
{capture(item,rect);return original.drawTextSpans(p,draw,item,rect,color,flags,spans,count);}
static int draw_cached(xui_proxy p,xui_draw_context draw,const xui_text_shape_t* shape,
    int start,int end,xui_rect_t rect,uint32_t color,uint32_t flags,float offset,
    const xui_text_paint_span_t* spans,int count)
{
    int result=original.drawTextShapeRangeSpans(p,draw,shape,start,end,rect,color,flags,offset,spans,count);
    if(result==XUI_OK && shape->pPaint && shape->pPaint==retained_input){
        CHECK(start==0 && end==6 && shape->iTextSize==6);
        paint_rect=rect;paint_offset=offset;paints++;
    }
    return result;
}
static int frame(void* ignored)
{
    xui_proxy_t proxy=original=xuiProxyXge();xui_context context;xui_font font;xui_proxy_caps_t caps;
    xui_surface target,reference;xui_surface_desc_t surface={0};
    unsigned char actual[W*H*4],expected[W*H*4];
    const char text[]="ii\xc3\xa9ii";unsigned rendered=0;
    (void)ignored;
    proxy.drawText=draw_text;proxy.drawTextSpans=draw_spans;
    proxy.textShape=shape_text;proxy.drawTextShapeRangeSpans=draw_cached;
#ifdef TEST_DRAW_TEXT_ONLY
    proxy.drawTextSpans=NULL;
#endif
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy)==XUI_OK &&
        proxy.fontLoadFile(&proxy,&font,"test/data/xge_context_fixture.ttf",40,0)==XUI_OK &&
        xuiSetDefaultFont(context,font)==XUI_OK);
    CHECK(xuiGetProxyCaps(context,&caps)==XUI_OK);
    unsigned inputs=XUI_PROXY_CAP_TEXT_CONTEXT|XUI_PROXY_CAP_TEXT_SCRIPT|XUI_PROXY_CAP_TEXT_LANGUAGE|XUI_PROXY_CAP_TEXT_RTL;
#ifdef TEST_NO_HB
    CHECK(!(caps.iCaps & inputs));
#else
    CHECK((caps.iCaps & inputs)==inputs);
#endif
    surface.iWidth=W;surface.iHeight=H;surface.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
    CHECK(proxy.surfaceCreate(&proxy,&target,&surface)==XUI_OK && proxy.surfaceCreate(&proxy,&reference,&surface)==XUI_OK);
    for(form=0;form<5;form++){
        retained_input=NULL;
        xui_document document;xui_document_snapshot snapshot;xui_document_renderer renderer;
        xui_doc_desc_t profile={0};xui_doc_renderer_desc_t desc={0};
        xui_document_transaction transaction;xui_doc_node_desc_t spec={0};uint64_t paragraph=0,node=0;
        xui_doc_position_t position={0};xui_doc_rect_t first,last;xui_draw_context draw;
        unsigned mode=form==3?XUI_DOC_SOURCE_TEXT:form==4?XUI_DOC_LIVE_MARKDOWN:XUI_DOC_VISUAL;
        profile.iSize=sizeof(profile);profile.iProfile=form>=2?XUI_DOCUMENT_MARKDOWN:XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile,&document)==XUI_OK);
        if(form>=2){
            CHECK(xuiDocumentLoadMarkdown(document,text,sizeof(text)-1)==XUI_OK &&
                xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot,1,0,&paragraph)==XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot,paragraph,0,&node)==XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
        }else{
            CHECK(xuiDocumentBeginTransaction(document,NULL,&transaction)==XUI_OK);
            spec.iSize=sizeof(spec);spec.iKind=form?XUI_DOC_CODE_BLOCK:XUI_DOC_PARAGRAPH;
            if(form){spec.sText=text;spec.iTextBytes=sizeof(text)-1;spec.sInfo="c";}
            CHECK(xuiDocumentTxnInsertNode(transaction,1,0,&spec,&paragraph)==XUI_OK);
            node=paragraph;
            if(!form){spec.iKind=XUI_DOC_TEXT;spec.sText=text;spec.iTextBytes=sizeof(text)-1;
                CHECK(xuiDocumentTxnInsertNode(transaction,paragraph,0,&spec,&node)==XUI_OK);}
            CHECK(xuiDocumentTxnCommit(transaction,NULL)==XUI_OK);xuiDocumentTxnRelease(transaction);
        }
        desc.iSize=sizeof(desc);desc.tFonts=(xui_doc_font_set_t){font,font,font,font,font};
        desc.iTextColor=0xffffffff;desc.iCodeBackground=XUI_COLOR_RGBA(1,0,0,0);
        CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK &&
            xuiDocumentRendererCreate(context,&desc,&renderer)==XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer,snapshot,NULL)==XUI_OK &&
            xuiDocumentRendererSetMode(renderer,mode)==XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        position.iSize=sizeof(position);position.iDocumentId=xuiDocumentGetIdentity(document);
        position.iRevision=xuiDocumentGetRevision(document);position.iAffinity=XUI_DOC_AFTER;
        position.iKind=mode==XUI_DOC_VISUAL?XUI_DOC_POSITION_TEXT:XUI_DOC_POSITION_SOURCE;
        position.iNodeId=mode==XUI_DOC_VISUAL?node:1;
        if(mode==XUI_DOC_LIVE_MARKDOWN)CHECK(xuiDocumentRendererSetActivePosition(renderer,&position)==XUI_OK);
        CHECK(xuiDocumentRendererLayout(renderer,W,0,H)==XUI_OK);
        CHECK(xuiDocumentRendererGetCaretRect(renderer,&position,&first)==XUI_OK);
        position.iOffset=sizeof(text)-1;CHECK(xuiDocumentRendererGetCaretRect(renderer,&position,&last)==XUI_OK);
        CHECK(last.x>first.x && last.y==first.y);
        CHECK(fabs(last.x-first.x-
#ifdef TEST_NO_HB
            56
#else
            72
#endif
            )<.003);
        paints=0;
        CHECK(proxy.surfaceClear(&proxy,target,0)==XUI_OK && proxy.drawBegin(&proxy,&draw,target)==XUI_OK &&
            xuiDocumentRendererDraw(renderer,draw,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK &&
            proxy.drawEnd(&proxy,draw)==XUI_OK);
        CHECK(paints==1);
        /* Literal aliases bypass the language substitution; e-acute is
         * intentionally absent in this fixture and uses .notdef (600/em). */
        const char literal[]=
#ifdef TEST_NO_HB
            "\xee\x84\x82\xee\x84\x82\xc3\xa9\xee\x84\x82\xee\x84\x82";
#else
            "\xee\x84\x83\xee\x84\x83\xc3\xa9\xee\x84\x83\xee\x84\x83";
#endif
        CHECK(original.surfaceClear(&original,reference,0)==XUI_OK && original.drawBegin(&original,&draw,reference)==XUI_OK &&
            original.drawText(&original,draw,&(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=font,.sText=literal,
                .iTextSize=sizeof(literal)-1,.iFlags=XUI_TEXT_SHAPE_DEFAULT,.fDrawOffsetX=paint_offset},paint_rect,0xffffffff,0)==XUI_OK &&
            original.drawEnd(&original,draw)==XUI_OK && original.surfaceReadRGBA(&original,target,actual,W*4)==XUI_OK &&
            original.surfaceReadRGBA(&original,reference,expected,W*4)==XUI_OK);
        unsigned ink=0;for(unsigned i=0;i<W*H;i++){CHECK(actual[i*4+3]==expected[i*4+3]);ink+=actual[i*4+3]!=0;}CHECK(ink>60);
        rendered++;
        xuiDocumentRendererRelease(renderer);xuiDocumentRelease(document);
    }
    printf("Document native input capabilities: %u Rich/code/Markdown Visual/Source/Live ordinary non-ASCII, literal widths and exact full-frame alpha cases passed\n",rendered);
    proxy.surfaceDestroy(&proxy,target);proxy.surfaceDestroy(&proxy,reference);proxy.fontDestroy(&proxy,font);xuiDestroy(context);
    xgeQuit();return XGE_OK;
}
int main(void)
{
    xge_desc_t desc={0};desc.iWidth=W;desc.iHeight=H;desc.iFlags=XGE_INIT_OFFSCREEN;desc.iRunMode=XGE_RUN_GAME_LOOP;
    CHECK(xgeInit(&desc)==XGE_OK && xgeRun(frame,NULL)==XGE_OK);xgeUnit();return 0;
}
