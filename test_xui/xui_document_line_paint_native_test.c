/* Independent per-glyph PUA pixel oracle for retained final Unicode/RTL rows. */
#include "../xge.h"
#include "../xui_document_ui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum{W=180,H=180};
#define CHECK(e) do{if(!(e)){fprintf(stderr,"line paint %d: %s\n",__LINE__,#e);exit(1);}}while(0)
typedef struct sample_t{const char* text;const char* font;unsigned base,glyphs[4],count;}sample_t;
static const sample_t samples[]={
    {"(\xce\xbb)a","test/data/xge_script_fixture.ttf",0xe000,{14,3,15,2},4},
    {"\xe3\x83\xbc\xe3\x82\xab","test/data/xge_script_fixture.ttf",0xe000,{16,5},2},
    {"\xd8\xa8\xd8\xa8\xd8\xa8","test/data/xge_context_fixture.ttf",0xe100,{7,8,9},3},
    {"\xd7\x90 \xd7\x91","test/data/xge_rtl_context_fixture.ttf",0xe600,{4,1,5},3}
};
static xui_proxy_t original;
static unsigned shaped,raw_draws,cached_draws,cases;
static int shape_capture(xui_proxy p,const xui_text_item_t* item,xui_text_shape_t* out)
{shaped++;return original.textShape(p,item,out);}
static int draw_capture(xui_proxy p,xui_draw_context d,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags)
{raw_draws++;return original.drawText(p,d,item,rect,color,flags);}
static int spans_capture(xui_proxy p,xui_draw_context d,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags,
    const xui_text_paint_span_t* spans,int count)
{raw_draws++;return original.drawTextSpans(p,d,item,rect,color,flags,spans,count);}
static int cached_capture(xui_proxy p,xui_draw_context d,const xui_text_shape_t* shape,int start,int end,
    xui_rect_t rect,uint32_t color,uint32_t flags,float offset,const xui_text_paint_span_t* spans,int count)
{
    CHECK(shape->pPaint && start>=0 && end<=shape->iTextSize && end>start);
    if(shape->iFlags & XUI_TEXT_SHAPE_RTL){
        xui_vec2_t measured={0};
        CHECK(original.textShapeRangeMeasure(p,shape,0,shape->iTextSize,&measured)==XUI_OK && measured.fX==shape->fWidth && measured.fY>0);
        if(shape->iClusterCount>0 && shape->pClusters[0].iTextEnd>1){
            measured=(xui_vec2_t){1,1};
            CHECK(original.textShapeRangeMeasure(p,shape,1,shape->iTextSize,&measured)==XUI_ERROR_UNSUPPORTED && measured.fX==0 && measured.fY==0);
            CHECK(original.drawTextShapeRangeSpans(p,d,shape,1,shape->iTextSize,rect,color,flags,offset,spans,count)==XUI_ERROR_UNSUPPORTED);
        }
    }
    cached_draws++;return original.drawTextShapeRangeSpans(p,d,shape,start,end,rect,color,flags,offset,spans,count);
}
static unsigned scalar_bytes(unsigned char c){return c<128?1:c<224?2:c<240?3:4;}
static xui_doc_position_t position(uint64_t identity,uint64_t revision,uint64_t node,uint64_t offset,unsigned mode,uint32_t affinity)
{
    xui_doc_position_t p={0};p.iSize=sizeof(p);p.iDocumentId=identity;p.iRevision=revision;p.iAffinity=affinity;
    p.iKind=mode==XUI_DOC_VISUAL?XUI_DOC_POSITION_TEXT:XUI_DOC_POSITION_SOURCE;
    p.iNodeId=mode==XUI_DOC_VISUAL?node:1;p.iOffset=offset;return p;
}
static void literal(xui_proxy p,xui_draw_context draw,xui_font font,unsigned cp,double x,double y,uint32_t color)
{
    char text[4]={(char)(0xe0|cp>>12),(char)(0x80|((cp>>6)&63)),(char)(0x80|(cp&63)),0};
    xui_rect_t rect={(int)floor(x+.5),(int)floor(y+.5),W,H-(int)floor(y+.5)};
    CHECK(original.drawText(p,draw,&(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=font,.sText=text,
        .iTextSize=3,.iFlags=XUI_TEXT_SHAPE_DEFAULT,.fDrawOffsetX=(float)(x-rect.fX)},rect,color,0)==XUI_OK);
}
static void sample_cases(xui_proxy p,xui_context context,xui_font font,const sample_t* sample,float size,xui_surface* targets)
{
    for(unsigned form=0;form<5;form++){
        xui_document d;xui_document_snapshot snapshot;xui_document_renderer r;
        xui_doc_desc_t profile={0};xui_doc_renderer_desc_t desc={0};
        uint64_t leaves[4]={0},paragraph,identity,revision;unsigned offsets[5]={0};
        uint32_t colors[]={UINT32_C(0xff314159),UINT32_C(0xff2040c8),UINT32_C(0xffc82040),UINT32_C(0xff20c840)};
        unsigned mode=form<3?XUI_DOC_VISUAL:form==3?XUI_DOC_SOURCE_TEXT:XUI_DOC_LIVE_MARKDOWN;
        for(unsigned i=0;i<sample->count;i++)offsets[i+1]=offsets[i]+scalar_bytes((unsigned char)sample->text[offsets[i]]);
        profile.iSize=sizeof(profile);profile.iProfile=form<2?XUI_DOCUMENT_RICH:XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&profile,&d)==XUI_OK);
        if(form<2){
            xui_document_transaction txn;xui_doc_node_desc_t node={0};
            CHECK(xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK);node.iSize=sizeof(node);node.iKind=XUI_DOC_PARAGRAPH;
            CHECK(xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
            for(unsigned i=0;i<(form?sample->count:1);i++){
                node.iKind=XUI_DOC_TEXT;node.sText=sample->text+offsets[i];node.iTextBytes=form?offsets[i+1]-offsets[i]:strlen(sample->text);
                node.tAttributes.iTextColor=colors[form?i:0];
                CHECK(xuiDocumentTxnInsertNode(txn,paragraph,XUI_DOCUMENT_APPEND,&node,&leaves[i])==XUI_OK);
            }
            CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK);xuiDocumentTxnRelease(txn);
        }else{
            CHECK(xuiDocumentLoadMarkdown(d,sample->text,strlen(sample->text))==XUI_OK && xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot,1,0,&paragraph)==XUI_OK && xuiDocumentSnapshotGetChild(snapshot,paragraph,0,&leaves[0])==XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
        }
        identity=xuiDocumentGetIdentity(d);revision=xuiDocumentGetRevision(d);
        desc.iSize=sizeof(desc);desc.tFonts=(xui_doc_font_set_t){font,font,font,font,font};desc.iTextColor=colors[0];
        CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererCreate(context,&desc,&r)==XUI_OK &&
            xuiDocumentRendererSetSnapshot(r,snapshot,NULL)==XUI_OK && xuiDocumentRendererSetMode(r,mode)==XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        if(mode==XUI_DOC_LIVE_MARKDOWN){xui_doc_position_t start=position(identity,revision,leaves[0],0,mode,XUI_DOC_AFTER);
            CHECK(xuiDocumentRendererSetActivePosition(r,&start)==XUI_OK);}
        for(unsigned pass=0;pass<3;pass++){
            double width=pass==1?size*.95:W,ox=pass==1?.25:0;
            xui_rect_t clip=pass==1?(xui_rect_t){5,0,W-5,H}:(xui_rect_t){0,0,W,H};
            xui_draw_context draw;unsigned before;
            CHECK(xuiDocumentRendererLayout(r,width,0,H)==XUI_OK);
            if(form==1 && pass==2){
                xui_document_transaction txn;xui_document_change_set changes;xui_doc_node_info_t info={0};
                info.iSize=sizeof(info);CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK &&
                    xuiDocumentSnapshotGetNode(snapshot,leaves[sample->count-1],&info)==XUI_OK && xuiDocumentBeginTransaction(d,NULL,&txn)==XUI_OK);
                colors[sample->count-1]=UINT32_C(0xffa0e040);info.tAttributes.iTextColor=colors[sample->count-1];
                CHECK(xuiDocumentTxnSetAttributes(txn,leaves[sample->count-1],&info.tAttributes)==XUI_OK && xuiDocumentTxnCommit(txn,&changes)==XUI_OK);
                xuiDocumentTxnRelease(txn);xuiDocumentSnapshotRelease(snapshot);before=shaped;
                CHECK(xuiDocumentAcquireSnapshot(d,&snapshot)==XUI_OK && xuiDocumentRendererSetSnapshot(r,snapshot,changes)==XUI_OK &&
                    xuiDocumentRendererLayout(r,width,0,H)==XUI_OK && shaped==before);
                xuiDocumentSnapshotRelease(snapshot);xuiDocumentChangeSetRelease(changes);revision=xuiDocumentGetRevision(d);
            }
            if(pass==2){xuiDocumentRelease(d);d=NULL;}
            raw_draws=cached_draws=0;before=shaped;
            CHECK(p->surfaceClear(p,targets[0],0)==XUI_OK && p->drawBegin(p,&draw,targets[0])==XUI_OK &&
                xuiDocumentRendererDraw(r,draw,ox,0,clip,NULL,0)==XUI_OK && p->drawEnd(p,draw)==XUI_OK &&
                cached_draws>0 && raw_draws==0 && shaped==before);
            CHECK(p->surfaceClear(p,targets[1],0)==XUI_OK && p->drawBegin(p,&draw,targets[1])==XUI_OK && p->drawClipSet(p,draw,clip)==XUI_OK);
            for(unsigned i=0;i<sample->count;i++){
                unsigned from=form==1?0:offsets[i],to=form==1?offsets[i+1]-offsets[i]:offsets[i+1];
                uint64_t node=leaves[form==1?i:0];xui_doc_rect_t a,b;
                xui_doc_position_t start=position(identity,revision,node,from,mode,XUI_DOC_AFTER);
                xui_doc_position_t end=position(identity,revision,node,to,mode,XUI_DOC_BEFORE);
                CHECK(xuiDocumentRendererGetCaretRect(r,&start,&a)==XUI_OK && xuiDocumentRendererGetCaretRect(r,&end,&b)==XUI_OK && fabs(a.y-b.y)<.001);
                literal(p,draw,font,sample->base+sample->glyphs[i],ox+fmin(a.x,b.x),a.y,colors[form==1?i:0]);
            }
            CHECK(p->drawEnd(p,draw)==XUI_OK);
            {unsigned char actual[W*H*4],expected[W*H*4];unsigned ink=0;
                CHECK(p->surfaceReadRGBA(p,targets[0],actual,W*4)==XUI_OK && p->surfaceReadRGBA(p,targets[1],expected,W*4)==XUI_OK);
                for(unsigned i=0;i<sizeof(actual);i++){
                    if(actual[i]!=expected[i])fprintf(stderr,"sample=%s form=%u pass=%u size=%g byte=%u got=%u expected=%u\n",sample->text,form,pass,size,i,actual[i],expected[i]);
                    CHECK(actual[i]==expected[i]);if(i%4==3 && actual[i])ink++;
                }CHECK(ink>20);cases++;
            }
        }
        xuiDocumentRendererRelease(r);
    }
}
static int frame(void* user)
{
    float size=*(float*)user;xui_proxy_t p=original=xuiProxyXge();xui_context context;xui_font font;xui_surface targets[2];xui_surface_desc_t surface={0};
    p.textShape=shape_capture;p.drawText=draw_capture;p.drawTextSpans=spans_capture;p.drawTextShapeRangeSpans=cached_capture;
#ifdef TEST_DRAW_TEXT_ONLY
    p.drawTextSpans=NULL;
#endif
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&p)==XUI_OK);
    surface.iWidth=W;surface.iHeight=H;surface.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
    CHECK(p.surfaceCreate(&p,&targets[0],&surface)==XUI_OK && p.surfaceCreate(&p,&targets[1],&surface)==XUI_OK);
    for(unsigned i=0;i<sizeof(samples)/sizeof(*samples);i++){
        CHECK(p.fontLoadFile(&p,&font,samples[i].font,size,0)==XUI_OK && xuiSetDefaultFont(context,font)==XUI_OK);
        sample_cases(&p,context,font,&samples[i],size,targets);p.fontDestroy(&p,font);
    }
    p.surfaceDestroy(&p,targets[0]);p.surfaceDestroy(&p,targets[1]);xuiDestroy(context);
    xgeQuit();return XGE_OK;
}
int main(void)
{
    const float sizes[]={40,37};
    for(unsigned i=0;i<2;i++){
        float size=sizes[i];xge_desc_t desc={0};desc.iWidth=W;desc.iHeight=H;desc.sTitle="Retained Unicode final lines";
        desc.iFlags=XGE_INIT_OFFSCREEN;desc.iRunMode=XGE_RUN_GAME_LOOP;
        CHECK(xgeInit(&desc)==XGE_OK && xgeRun(frame,&size)==XGE_OK);xgeUnit();
    }
    printf("Native retained Unicode final rows: %u exact full RGBA cases, Greek script context/Kana locl/Arabic joining RTL/Hebrew contextual GSUB, Rich full/split colors and Markdown visual/source/live, wrap/clip/fractional X, colour cache, released Document and zero raw draw reshaping passed\n",cases);return 0;
}
