/* The reference image uses literal PUA glyphs at specified positions; it
 * never asks a line-breaking engine to choose reference rows. */
#include "../xge.h"
#include "../xui_document_ui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { W=100,H=100 };
static unsigned backend,form,sample,pass,cases;
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"Line17:%d backend=%u form=%u sample=%u pass=%u: %s\n",__LINE__,backend,form,sample,pass,#e);exit(1); } } while (0)
static xui_proxy_t original;
static int shape(xui_proxy p,const xui_text_item_t* item,xui_text_shape_t* out)
{
    int result=original.textShape(p,item,out);
    if(backend>=4 && result==XUI_OK && out->pPaint){out->paintFree(out->pPaint);out->pPaint=NULL;out->paintFree=NULL;out->iPaintBytes=0;}
    return result;
}
typedef struct line_case_t {
    const char* text; unsigned glyph[3],ends[3];
    unsigned rows[3],columns[3];
} line_case_t;
static const line_case_t fixtures[]={
    {"-ab",{2,4,5},{1,2,3},{0,0,1},{0,1,0}},
    {"\xe2\x80\x90" "ab",{3,4,5},{3,4,5},{0,0,1},{0,1,0}},
    {"a-\xe4\xb8\xad",{4,2,6},{1,2,5},{0,0,1},{0,1,0}},
    {"a\xe2\x80\x90\xe4\xb8\xad",{4,3,6},{1,4,7},{0,0,1},{0,1,0}}
};
static xui_doc_position_t position(xui_document_snapshot s,unsigned mode,uint64_t leaf,unsigned local,unsigned source)
{
    xui_doc_position_t p={.iSize=sizeof(p),.iKind=mode>=XUI_DOC_SOURCE_TEXT?XUI_DOC_POSITION_SOURCE:XUI_DOC_POSITION_TEXT,
        .iDocumentId=xuiDocumentSnapshotGetIdentity(s),.iRevision=xuiDocumentSnapshotGetRevision(s),
        .iNodeId=mode>=XUI_DOC_SOURCE_TEXT?1:leaf,.iOffset=mode>=XUI_DOC_SOURCE_TEXT?source:local,.iAffinity=XUI_DOC_BEFORE};
    return p;
}
static void literal(xui_proxy p,xui_draw_context dc,xui_font font,unsigned id,double x,double y)
{
    unsigned cp=0xe900+id; char text[]={(char)(0xe0|cp>>12),(char)(0x80|((cp>>6)&63)),(char)(0x80|(cp&63)),0};
    xui_text_item_t item={.iSize=sizeof(item),.pFont=font,.sText=text,.iTextSize=3,.iFlags=XUI_TEXT_SHAPE_DEFAULT};
    CHECK(original.drawText(p,dc,&item,(xui_rect_t){(float)x,(float)y,W,H-(float)y},UINT32_C(0xff314159),0)==XUI_OK);
}
static int frame(void* unused)
{
    xui_font font;xui_surface targets[2];xui_context context;xui_proxy_t p=original=xuiProxyXge();
    xui_surface_desc_t surface={.iWidth=W,.iHeight=H,.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED};
    (void)unused;
    CHECK(p.fontLoadFile(&p,&font,"test/data/xge_line17_fixture.ttf",20,0)==XUI_OK &&
        p.surfaceCreate(&p,&targets[0],&surface)==XUI_OK && p.surfaceCreate(&p,&targets[1],&surface)==XUI_OK);
    for(backend=0;backend<6;backend++){
        p=original;p.textShape=shape;
        if(backend&1)p.drawTextSpans=NULL;
        if(backend>=2){p.textShapeRangeMeasure=NULL;p.drawTextShapeRangeSpans=NULL;}
        CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&p)==XUI_OK && xuiSetDefaultFont(context,font)==XUI_OK);
        for(form=0;form<5;form++)for(sample=0;sample<sizeof(fixtures)/sizeof(*fixtures);sample++){
            const line_case_t* f=&fixtures[sample];xui_document d;xui_document_snapshot s;xui_document_renderer r;uint64_t para,leaf[3];
            unsigned mode=form<3?XUI_DOC_VISUAL:form==3?XUI_DOC_SOURCE_TEXT:XUI_DOC_LIVE_MARKDOWN;
            xui_doc_desc_t profile={.iSize=sizeof(profile),.iProfile=form<2?XUI_DOCUMENT_RICH:XUI_DOCUMENT_MARKDOWN};
            xui_doc_renderer_desc_t desc={.iSize=sizeof(desc),.tFonts={font,font,font,font,font},.iTextColor=UINT32_C(0xff314159)};
            CHECK(xuiDocumentCreate(&profile,&d)==XUI_OK);
            if(form<2){
                xui_document_transaction t;xui_doc_node_desc_t n={.iSize=sizeof(n),.iKind=XUI_DOC_PARAGRAPH};
                CHECK(xuiDocumentBeginTransaction(d,NULL,&t)==XUI_OK && xuiDocumentTxnInsertNode(t,1,XUI_DOCUMENT_APPEND,&n,&para)==XUI_OK);
                for(unsigned i=0;i<(form?3u:1u);i++){
                    unsigned start=form && i?f->ends[i-1]:0;n.iKind=XUI_DOC_TEXT;n.sText=f->text+start;
                    n.iTextBytes=form?f->ends[i]-start:strlen(f->text);
                    CHECK(xuiDocumentTxnInsertNode(t,para,XUI_DOCUMENT_APPEND,&n,&leaf[i])==XUI_OK);
                }
                CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK);xuiDocumentTxnRelease(t);
            }else{
                CHECK(xuiDocumentLoadMarkdown(d,f->text,strlen(f->text))==XUI_OK && xuiDocumentAcquireSnapshot(d,&s)==XUI_OK &&
                    xuiDocumentSnapshotGetChild(s,1,0,&para)==XUI_OK && xuiDocumentSnapshotGetChild(s,para,0,&leaf[0])==XUI_OK);
                xuiDocumentSnapshotRelease(s);
            }
            CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentRendererCreate(context,&desc,&r)==XUI_OK &&
                xuiDocumentRendererSetSnapshot(r,s,NULL)==XUI_OK && xuiDocumentRendererSetMode(r,mode)==XUI_OK);
            if(form==4){xui_doc_position_t active=position(s,mode,0,0,0);CHECK(xuiDocumentRendererSetActivePosition(r,&active)==XUI_OK);}
            xuiDocumentRelease(d);
            for(pass=0;pass<3;pass++){
                double width=pass==1?10:W;unsigned wrapped=pass==1 && form<3;xui_draw_context dc;
                if(pass==2)CHECK(xuiDocumentRendererInvalidateFonts(r)==XUI_OK);
                CHECK(xuiDocumentRendererLayout(r,width,0,H)==XUI_OK);
                for(unsigned i=0;i<3;i++){
                    unsigned start=i?f->ends[i-1]:0;
                    xui_doc_position_t at=position(s,mode,leaf[form==1?i:0],form==1?f->ends[i]-start:f->ends[i],f->ends[i]);
                    xui_doc_rect_t caret,clicked;xui_doc_position_t hit;
                    double x=(wrapped?f->columns[i]+1:i+1)*10,y=wrapped?f->rows[i]*20:0;
                    CHECK(xuiDocumentRendererGetCaretRect(r,&at,&caret)==XUI_OK);
                    if(fabs(caret.x-x)>.01 || fabs(caret.y-y)>.01)fprintf(stderr,"caret glyph=%u got=%g/%g expected=%g/%g\n",i,caret.x,caret.y,x,y);
                    CHECK(fabs(caret.x-x)<.01 && fabs(caret.y-y)<.01 && fabs(caret.height-20)<.01);
                    CHECK(xuiDocumentRendererHitTest(r,x-.2,y+10,&hit)==XUI_OK && xuiDocumentRendererGetCaretRect(r,&hit,&clicked)==XUI_OK &&
                        fabs(clicked.x-x)<.01 && fabs(clicked.y-y)<.01);
                }
                CHECK(p.surfaceClear(&p,targets[0],0)==XUI_OK && p.drawBegin(&p,&dc,targets[0])==XUI_OK &&
                    xuiDocumentRendererDraw(r,dc,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK && p.drawEnd(&p,dc)==XUI_OK &&
                    p.surfaceClear(&p,targets[1],0)==XUI_OK && p.drawBegin(&p,&dc,targets[1])==XUI_OK);
                for(unsigned i=0;i<3;i++)literal(&p,dc,font,f->glyph[i],(wrapped?f->columns[i]:i)*10,wrapped?f->rows[i]*20:0);
                unsigned char a[W*H*4],b[W*H*4];unsigned ink=0;
                CHECK(p.drawEnd(&p,dc)==XUI_OK && p.surfaceReadRGBA(&p,targets[0],a,W*4)==XUI_OK && p.surfaceReadRGBA(&p,targets[1],b,W*4)==XUI_OK);
                for(unsigned i=0;i<sizeof(a);i++){if(a[i]!=b[i])fprintf(stderr,"RGBA byte=%u got=%u expected=%u\n",i,a[i],b[i]);CHECK(a[i]==b[i]);ink+=i%4==3 && a[i]>0;}
                CHECK(ink>20);cases++;
            }
            xuiDocumentRendererRelease(r);xuiDocumentSnapshotRelease(s);
        }
        xuiDestroy(context);
    }
    p.surfaceDestroy(&p,targets[0]);p.surfaceDestroy(&p,targets[1]);p.fontDestroy(&p,font);
    printf("Unicode17 Document: %u independent complete RGBA cases; word-initial HY/HH and noninitial hyphens, Rich split items/Markdown VISUAL/SOURCE/LIVE, six paint routes, literal carets/hit, reflow/invalidation/released Document passed\n",cases);
    xgeQuit();return XGE_OK;
}
int main(void)
{
    xge_desc_t d={.iWidth=W,.iHeight=H,.sTitle="Unicode17 lines",.iFlags=XGE_INIT_OFFSCREEN,.iRunMode=XGE_RUN_GAME_LOOP};
    CHECK(xgeInit(&d)==XGE_OK && xgeRun(frame,NULL)==XGE_OK);xgeUnit();return 0;
}
