/* Literal PUA outlines provide an independent complete-RGBA oracle. The
 * contextual text is never used to generate the reference image. */
#include "../xge.h"
#include "../xui_document_ui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { W=160,H=120 };
static unsigned backend,form,pass,cases,context_calls;
#define CHECK(e) do{if(!(e)){fprintf(stderr,"Word context line %d backend=%u form=%u pass=%u: %s\n",__LINE__,backend,form,pass,#e);exit(1);}}while(0)
static xui_proxy_t original;
static int shape(xui_proxy p,const xui_text_item_t* item,xui_text_shape_t* out)
{
    if(item->sContext){
        CHECK(item->iContextSize==3 && !memcmp(item->sContext,"ffi",3) &&
            item->iContextOffset>=0 && item->iContextOffset+item->iTextSize<=3 &&
            item->iScript==UINT32_C(0x4c61746e));context_calls++;
    }
    int result=original.textShape(p,item,out);
    if(backend>=4 && result==XUI_OK && out->pPaint){out->paintFree(out->pPaint);out->pPaint=NULL;out->paintFree=NULL;out->iPaintBytes=0;}
    return result;
}
static xui_doc_position_t position(xui_document d,uint64_t node,uint64_t offset)
{
    return (xui_doc_position_t){.iSize=sizeof(xui_doc_position_t),.iKind=XUI_DOC_POSITION_TEXT,
        .iDocumentId=xuiDocumentGetIdentity(d),.iRevision=xuiDocumentGetRevision(d),
        .iNodeId=node,.iOffset=offset,.iAffinity=XUI_DOC_BEFORE};
}
static void literal(xui_proxy p,xui_draw_context dc,xui_font font,const char* text,
    double x,double y,double width,double height)
{
    float left=(float)floor(x+.5),top=(float)floor(y+.5);
    xui_rect_t rect={left,top,(float)(floor(x+width+.5)-left),(float)(floor(y+height+.5)-top)};
    xui_text_item_t item={.iSize=sizeof(item),.pFont=font,.sText=text,.iTextSize=-1,
        .iFlags=XUI_TEXT_SHAPE_DEFAULT,.fDrawOffsetX=(float)(x-left)};
    CHECK(original.drawText(p,dc,&item,rect,UINT32_C(0xff314159),0)==XUI_OK);
}
static int frame(void* user)
{
    float size=*(float*)user;xui_font fonts[3],full[3];xui_surface targets[2];
    xui_proxy_t p=original=xuiProxyXge();xui_context c;
    xui_surface_desc_t surface={.iWidth=W,.iHeight=H,.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED};
    CHECK(xgeFontFallbackSet("test/data/xge_word_context_full.ttf",size)==XGE_OK);
    for(unsigned i=0;i<3;i++){
        CHECK(p.fontLoadFile(&p,&fonts[i],"test/data/xge_word_context_primary.ttf",i==2?size-3:size,0)==XUI_OK);
        CHECK(p.fontLoadFile(&p,&full[i],"test/data/xge_word_context_full.ttf",i==2?size-3:size,0)==XUI_OK);
    }
    CHECK(p.surfaceCreate(&p,&targets[0],&surface)==XUI_OK && p.surfaceCreate(&p,&targets[1],&surface)==XUI_OK);
    for(backend=0;backend<6;backend++){
        p=original;p.textShape=shape;
        if(backend&1)p.drawTextSpans=NULL;
        if(backend>=2){p.textShapeRangeMeasure=NULL;p.drawTextShapeRangeSpans=NULL;}
        CHECK(xuiCreate(&c)==XUI_OK && xuiSetProxy(c,&p)==XUI_OK && xuiSetDefaultFont(c,fonts[0])==XUI_OK);
        for(form=0;form<3;form++){
            xui_document d;xui_document_snapshot s;xui_document_renderer r;uint64_t paragraph,ids[3];
            xui_doc_desc_t profile={.iSize=sizeof(profile),.iProfile=form==2?XUI_DOCUMENT_MARKDOWN:XUI_DOCUMENT_RICH};
            xui_doc_position_t at[3];xui_doc_rect_t caret;xui_draw_context dc;unsigned char a[W*H*4],b[W*H*4];
            xui_doc_renderer_desc_t desc={.iSize=sizeof(desc),.tFonts={fonts[0],form==2?fonts[2]:fonts[1],fonts[0],fonts[1],fonts[0]},.iTextColor=UINT32_C(0xff314159)};
            double sizes[3]={form==1?size-3:size,form==2?size-3:size,size},x[4]={0};
            xui_font selected[3]={form==1?full[2]:full[0],form==2?full[2]:full[1],full[0]};
            CHECK(xuiDocumentCreate(&profile,&d)==XUI_OK);
            if(form==2){
                CHECK(xuiDocumentLoadMarkdown(d,"f**f**i",7)==XUI_OK && xuiDocumentAcquireSnapshot(d,&s)==XUI_OK &&
                    xuiDocumentSnapshotGetChild(s,1,0,&paragraph)==XUI_OK);
                for(unsigned i=0;i<3;i++)CHECK(xuiDocumentSnapshotGetChild(s,paragraph,i,&ids[i])==XUI_OK);
                xuiDocumentSnapshotRelease(s);
            }else{
                xui_document_transaction t;xui_doc_node_desc_t n={.iSize=sizeof(n),.iKind=XUI_DOC_PARAGRAPH};
                CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK && xuiDocumentTxnInsertNode(t,1,XUI_DOCUMENT_APPEND,&n,&paragraph)==XUI_OK);
                for(unsigned i=0;i<3;i++){
                    n.iKind=XUI_DOC_TEXT;n.sText="ffi"+i;n.iTextBytes=1;
                    n.tAttributes.iMarks=i==1?XUI_DOC_BOLD:0;n.tAttributes.fFontSize=form==1 && !i?size-3:0;
                    CHECK(xuiDocumentTxnInsertNode(t,paragraph,XUI_DOCUMENT_APPEND,&n,&ids[i])==XUI_OK);
                }
                CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
            }
            for(unsigned i=0;i<3;i++){at[i]=position(d,ids[i],1);x[i+1]=x[i]+sizes[i]*(i==2?.3:.4);}
            CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentRendererCreate(c,&desc,&r)==XUI_OK && xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK);
            xuiDocumentSnapshotRelease(s);xuiDocumentRelease(d);context_calls=0;
            for(pass=0;pass<3;pass++){
                if(pass==2)CHECK(xuiDocumentRendererInvalidateFonts(r)==XUI_OK);
                CHECK(xuiDocumentRendererLayout(r,pass==1?100:W,0,H)==XUI_OK && context_calls>0);
                for(unsigned i=0;i<3;i++){
                    CHECK(xuiDocumentRendererGetCaretRect(r,&at[i],&caret)==XUI_OK && fabs(caret.x-x[i+1])<.002 &&
                        fabs(caret.y-.8*(size-sizes[i]))<.002 && fabs(caret.height-sizes[i])<.002);
                    xui_doc_position_t hit;xui_doc_rect_t clicked;
                    CHECK(xuiDocumentRendererHitTest(r,caret.x,caret.y+caret.height*.5,&hit)==XUI_OK &&
                        xuiDocumentRendererGetCaretRect(r,&hit,&clicked)==XUI_OK && fabs(clicked.x-caret.x)<.002);
                }
                CHECK(p.surfaceClear(&p,targets[0],0)==XUI_OK && p.drawBegin(&p,&dc,targets[0])==XUI_OK &&
                    xuiDocumentRendererDraw(r,dc,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK && p.drawEnd(&p,dc)==XUI_OK);
                CHECK(p.surfaceClear(&p,targets[1],0)==XUI_OK && p.drawBegin(&p,&dc,targets[1])==XUI_OK);
                const char* glyphs[]={"\xee\x98\x82","\xee\x98\x82","\xee\x98\x83"};
                for(unsigned i=0;i<3;i++)literal(&p,dc,selected[i],glyphs[i],x[i],.8*(size-sizes[i]),x[i+1]-x[i],sizes[i]);
                CHECK(p.drawEnd(&p,dc)==XUI_OK && p.surfaceReadRGBA(&p,targets[0],a,W*4)==XUI_OK && p.surfaceReadRGBA(&p,targets[1],b,W*4)==XUI_OK);
                unsigned ink=0;for(unsigned i=0;i<sizeof(a);i++){
                    if(a[i]!=b[i])fprintf(stderr,"RGBA byte %u actual=%u expected=%u\n",i,a[i],b[i]);
                    CHECK(a[i]==b[i]);ink+=i%4==3 && a[i]>0;
                }CHECK(ink>20);cases++;
            }
            xuiDocumentRendererRelease(r);
        }
        xuiDestroy(c);
    }
    for(unsigned i=0;i<2;i++)p.surfaceDestroy(&p,targets[i]);
    for(unsigned i=0;i<3;i++){p.fontDestroy(&p,fonts[i]);p.fontDestroy(&p,full[i]);}
    xgeFontFallbackClear();
    printf("Word-context Document fallback: %u cumulative complete RGBA cases at %g, cross-font/size, Rich/Markdown, six cache/basic/no-paint routes, carets/hit, reflow/invalidation and released Document passed\n",cases,(double)size);
    xgeQuit();return XGE_OK;
}
int main(void)
{
    xge_desc_t e={.iWidth=W,.iHeight=H,.sTitle="Contextual whole-word fonts",.iFlags=XGE_INIT_OFFSCREEN,.iRunMode=XGE_RUN_GAME_LOOP};
    float sizes[]={40,37};for(unsigned i=0;i<2;i++){CHECK(xgeInit(&e)==XGE_OK && xgeRun(frame,&sizes[i])==XGE_OK);xgeUnit();}return 0;
}
