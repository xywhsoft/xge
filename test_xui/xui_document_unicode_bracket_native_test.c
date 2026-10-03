/* Isolated ASCII brackets must use their Unicode paragraph's resolved script. */
#include "../xge.h"
#include "../xui_document_ui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum{W=180,H=180};
#define CHECK(e) do{if(!(e)){fprintf(stderr,"Unicode bracket %d: %s\n",__LINE__,#e);exit(1);}}while(0)
static xui_proxy_t original;static unsigned shaped,raw,cached,cases;
static int capture_shape(xui_proxy p,const xui_text_item_t* item,xui_text_shape_t* out)
{shaped++;return original.textShape(p,item,out);}
static int capture_raw(xui_proxy p,xui_draw_context d,const xui_text_item_t* item,xui_rect_t r,uint32_t color,uint32_t flags)
{raw++;return original.drawText(p,d,item,r,color,flags);}
static int capture_spans(xui_proxy p,xui_draw_context d,const xui_text_item_t* item,xui_rect_t r,uint32_t color,uint32_t flags,const xui_text_paint_span_t* spans,int count)
{raw++;return original.drawTextSpans(p,d,item,r,color,flags,spans,count);}
static int capture_range(xui_proxy p,xui_draw_context d,const xui_text_shape_t* shape,int start,int end,xui_rect_t r,uint32_t color,uint32_t flags,float offset,const xui_text_paint_span_t* spans,int count)
{CHECK(shape->pPaint);cached++;return original.drawTextShapeRangeSpans(p,d,shape,start,end,r,color,flags,offset,spans,count);}
static xui_font resolve(xui_context c,const char* family,uint32_t marks,float size,void* user)
{(void)c;(void)marks;(void)size;return family && family[0]?*(xui_font*)user:NULL;}
static xui_doc_position_t position(uint64_t identity,uint64_t revision,uint64_t node,unsigned offset,uint32_t affinity)
{xui_doc_position_t p={.iSize=sizeof(p),.iKind=XUI_DOC_POSITION_TEXT,.iDocumentId=identity,.iRevision=revision,.iNodeId=node,.iOffset=offset,.iAffinity=affinity};return p;}
static void literal(xui_proxy p,xui_draw_context d,xui_font font,unsigned glyph,double x,double y,uint32_t color)
{
    unsigned cp=0xe000+glyph;char text[]={(char)(0xe0|cp>>12),(char)(0x80|((cp>>6)&63)),(char)(0x80|(cp&63)),0};
    xui_rect_t rect={(int)floor(x+.5),(int)floor(y+.5),W,H-(int)floor(y+.5)};
    CHECK(original.drawText(p,d,&(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=font,.sText=text,.iTextSize=3,.iFlags=XUI_TEXT_SHAPE_DEFAULT,.fDrawOffsetX=(float)(x-rect.fX)},rect,color,0)==XUI_OK);
}
static int frame(void* user)
{
    float size=*(float*)user;xui_proxy_t p=original=xuiProxyXge();xui_context c;xui_font fonts[2];
    xui_surface targets[2];xui_surface_desc_t surface={.iWidth=W,.iHeight=H,.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED};
    const unsigned glyphs[]={14,3,15,2},bytes[]={1,2,1,1};const char* parts[]={"(","\xce\xbb",")","a"};
    const double advances[]={.45,.6,.45,.6};
    p.textShape=capture_shape;p.drawText=capture_raw;p.drawTextSpans=capture_spans;p.drawTextShapeRangeSpans=capture_range;
#ifdef TEST_DRAW_TEXT_ONLY
    p.drawTextSpans=NULL;
#endif
    CHECK(xuiCreate(&c)==XUI_OK && xuiSetProxy(c,&p)==XUI_OK);
    for(unsigned i=0;i<2;i++)CHECK(p.fontLoadFile(&p,&fonts[i],"test/data/xge_script_fixture.ttf",size,0)==XUI_OK);
    CHECK(fonts[0]!=fonts[1] && xuiSetDefaultFont(c,fonts[0])==XUI_OK && p.surfaceCreate(&p,&targets[0],&surface)==XUI_OK && p.surfaceCreate(&p,&targets[1],&surface)==XUI_OK);
    for(unsigned form=0;form<2;form++){
        xui_document document;xui_document_snapshot snapshot;xui_document_renderer r;
        xui_doc_desc_t profile={.iSize=sizeof(profile),.iProfile=form?XUI_DOCUMENT_MARKDOWN:XUI_DOCUMENT_RICH};
        xui_doc_renderer_desc_t desc={0};uint64_t leaves[4],paragraph;uint32_t colors[]={UINT32_C(0xff2040c8),UINT32_C(0xffc82040),UINT32_C(0xff20c840),UINT32_C(0xff2040c8)};
        unsigned font_indices[]={0,1,0,0};uint64_t identity,revision;
        CHECK(xuiDocumentCreate(&profile,&document)==XUI_OK);
        if(form){
            /* Spaces make punctuation-only emphasis unambiguous CommonMark. */
            const char* md="**(** \xce\xbb **)** a";
            CHECK(xuiDocumentLoadMarkdown(document,md,strlen(md))==XUI_OK && xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK && xuiDocumentSnapshotGetChild(snapshot,1,0,&paragraph)==XUI_OK);
            for(unsigned i=0;i<4;i++)CHECK(xuiDocumentSnapshotGetChild(snapshot,paragraph,i,&leaves[i])==XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);font_indices[0]=font_indices[2]=1;font_indices[1]=0;
            for(unsigned i=0;i<4;i++)colors[i]=colors[0];
        }else{
            xui_document_transaction txn;xui_doc_node_desc_t node={.iSize=sizeof(node),.iKind=XUI_DOC_PARAGRAPH};
            CHECK(xuiDocumentBeginTransaction(document,NULL,&txn)==XUI_OK && xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
            for(unsigned i=0;i<4;i++){
                node.iKind=XUI_DOC_TEXT;node.sText=parts[i];node.iTextBytes=bytes[i];node.tAttributes.iTextColor=colors[i];
                snprintf(node.tAttributes.sFontFamily,sizeof(node.tAttributes.sFontFamily),"%s",i==1?"alternate":"");
                CHECK(xuiDocumentTxnInsertNode(txn,paragraph,XUI_DOCUMENT_APPEND,&node,&leaves[i])==XUI_OK);
            }
            CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
        }
        identity=xuiDocumentGetIdentity(document);revision=xuiDocumentGetRevision(document);
        desc.iSize=sizeof(desc);desc.tFonts=(xui_doc_font_set_t){fonts[0],fonts[1],fonts[0],fonts[1],fonts[0]};desc.iTextColor=colors[0];desc.onFont=form?NULL:resolve;desc.pUser=&fonts[1];
        CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK && xuiDocumentRendererCreate(c,&desc,&r)==XUI_OK && xuiDocumentRendererSetSnapshot(r,snapshot,NULL)==XUI_OK);xuiDocumentSnapshotRelease(snapshot);
        for(unsigned pass=0;pass<3;pass++){
            xui_draw_context draw;unsigned before;double ox=pass==1?.25:0;
            xui_rect_t clip=pass==1?(xui_rect_t){5,0,W-5,H}:(xui_rect_t){0,0,W,H};
            CHECK(xuiDocumentRendererLayout(r,pass==1?size*.95:W,0,H)==XUI_OK);
            if(pass==2)xuiDocumentRelease(document);
            before=shaped;raw=cached=0;
            CHECK(p.surfaceClear(&p,targets[0],0)==XUI_OK && p.drawBegin(&p,&draw,targets[0])==XUI_OK && xuiDocumentRendererDraw(r,draw,ox,0,clip,NULL,0)==XUI_OK && p.drawEnd(&p,draw)==XUI_OK && shaped==before && raw==0 && cached>=4);
            CHECK(p.surfaceClear(&p,targets[1],0)==XUI_OK && p.drawBegin(&p,&draw,targets[1])==XUI_OK && p.drawClipSet(&p,draw,clip)==XUI_OK);
            for(unsigned i=0;i<4;i++){
                unsigned offset=form && (i==1 || i==3)?1:0;
                xui_doc_rect_t a,b;xui_doc_position_t start=position(identity,revision,leaves[i],offset,XUI_DOC_AFTER),end=position(identity,revision,leaves[i],offset+bytes[i],XUI_DOC_BEFORE);
                CHECK(xuiDocumentRendererGetCaretRect(r,&start,&a)==XUI_OK && xuiDocumentRendererGetCaretRect(r,&end,&b)==XUI_OK && fabs(b.x-a.x-size*advances[i])<.001 && a.y==b.y);
                literal(&p,draw,fonts[font_indices[i]],glyphs[i],ox+a.x,a.y,colors[i]);
            }
            CHECK(p.drawEnd(&p,draw)==XUI_OK);
            {unsigned char actual[W*H*4],expected[W*H*4];unsigned ink=0;
                CHECK(p.surfaceReadRGBA(&p,targets[0],actual,W*4)==XUI_OK && p.surfaceReadRGBA(&p,targets[1],expected,W*4)==XUI_OK);
                for(unsigned i=0;i<sizeof(actual);i++){CHECK(actual[i]==expected[i]);if(i%4==3 && actual[i])ink++;}CHECK(ink>20);
            }cases++;
        }xuiDocumentRendererRelease(r);
    }
    p.surfaceDestroy(&p,targets[0]);p.surfaceDestroy(&p,targets[1]);xuiDestroy(c);for(unsigned i=0;i<2;i++)p.fontDestroy(&p,fonts[i]);xgeQuit();return XGE_OK;
}
int main(void)
{
    const float sizes[]={40,37};
    for(unsigned i=0;i<2;i++){float size=sizes[i];xge_desc_t desc={.iWidth=W,.iHeight=H,.sTitle="Unicode bracket fonts",.iFlags=XGE_INIT_OFFSCREEN,.iRunMode=XGE_RUN_GAME_LOOP};CHECK(xgeInit(&desc)==XGE_OK && xgeRun(frame,&size)==XGE_OK);xgeUnit();}
    printf("Native Unicode brackets: %u exact full RGBA cases; Rich alternate family and Markdown bold font boundaries, Greek script, wrap/clip/fractional X/released Document, zero raw draw/reshape passed\n",cases);return 0;
}
