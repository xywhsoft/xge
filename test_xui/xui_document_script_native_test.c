#include "../xge.h"
#include "../xui_document_ui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum { W=400,H=160 };
static unsigned sample, form;
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s (sample=%u form=%u)\n",__FILE__,__LINE__,#e,sample,form); exit(1); } } while(0)
typedef struct sample_t {const char* text; unsigned glyphs[5],count,rtl;} sample_t;
static const sample_t samples[]={
    {"a(\xce\xbb)a",{2,12,3,13,2},5,0}, {"(\xce\xbb)a",{14,3,15,2},4,0},
    {"a\xe3\x80\x88\xce\xbb\xe3\x80\x89" "a",{2,12,3,13,2},5,0},
    {"\xe3\x80\x88\xce\xbb\xe3\x80\x89" "a",{14,3,15,2},4,0},
    {"a\xe3\x83\xbc\xe3\x82\xab",{2,16,5},3,0}, {"\xe3\x83\xbc\xe3\x82\xab",{16,5},2,0},
    {"\xe3\x81\x82\xe3\x83\xbc" "a",{4,16,2},3,0},
    {"\xdc\x90\xd9\x80\xdc\x90",{7,17,7},3,1}, {"\xd9\x80\xdc\x90",{17,7},2,1},
    {"\xd9\x80\xd8\xa8",{9,8},2,1}
};
static unsigned scalar_bytes(unsigned char c){return c<128?1:c<224?2:c<240?3:4;}
static xui_doc_position_t position(xui_document document,uint64_t node,uint64_t offset,unsigned mode)
{
    xui_doc_position_t p={0};p.iSize=sizeof(p);p.iDocumentId=xuiDocumentGetIdentity(document);
    p.iRevision=xuiDocumentGetRevision(document);p.iKind=mode==XUI_DOC_VISUAL?XUI_DOC_POSITION_TEXT:XUI_DOC_POSITION_SOURCE;
    p.iNodeId=mode==XUI_DOC_VISUAL?node:1;p.iOffset=offset;p.iAffinity=XUI_DOC_AFTER;return p;
}
static int frame(void* user)
{
    xui_proxy_t proxy=xuiProxyXge();xui_context context;xui_font font;
    xui_surface targets[2];xui_surface_desc_t surface={0};float size=*(float*)user;
    unsigned pass,cases=0;unsigned char actual[W*H*4],expected[W*H*4];
#ifdef TEST_DRAW_TEXT_ONLY
    proxy.drawTextSpans=NULL;
#endif
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy)==XUI_OK);
    surface.iWidth=W;surface.iHeight=H;surface.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
    CHECK(proxy.surfaceCreate(&proxy,&targets[0],&surface)==XUI_OK && proxy.surfaceCreate(&proxy,&targets[1],&surface)==XUI_OK);
    for(pass=0;pass<2;pass++){
        if(pass)CHECK(xgeFontFallbackSet("test/data/xge_script_fixture.ttf",size)==XGE_OK);
        CHECK(proxy.fontLoadFile(&proxy,&font,pass?"test/data/xge_script_no_alaph.ttf":"test/data/xge_script_fixture.ttf",size,0)==XUI_OK);
        CHECK(xuiSetDefaultFont(context,font)==XUI_OK);
        for(sample=0;sample<sizeof(samples)/sizeof(*samples);sample++)for(form=0;form<5;form++){
            const sample_t* value=&samples[sample];unsigned length=(unsigned)strlen(value->text),at=0,parts=0,i;
            unsigned mode=form<3?XUI_DOC_VISUAL:form==3?XUI_DOC_SOURCE_TEXT:XUI_DOC_LIVE_MARKDOWN;
            uint64_t ids[8]={0},last_bytes=0;char reference[16]={0};double width=0;
            xui_document document;xui_document_snapshot snapshot;xui_document_renderer renderer;
            xui_doc_desc_t profile={0};xui_doc_renderer_desc_t desc={0};xui_doc_position_t start,end,hit;
            xui_doc_rect_t first,last,clicked;xui_draw_context draw;
            for(i=0;i<value->count;i++){
                unsigned cp=0xe000+value->glyphs[i];reference[i*3]=(char)(0xe0|cp>>12);
                reference[i*3+1]=(char)(0x80|((cp>>6)&63));reference[i*3+2]=(char)(0x80|(cp&63));
                width+=value->glyphs[i]==12||value->glyphs[i]==13?.3:value->glyphs[i]==14||value->glyphs[i]==15?.45:
                    value->glyphs[i]==16?.9:value->glyphs[i]==17?.8:value->glyphs[i]==9?.5:.6;
            }
            profile.iSize=sizeof(profile);profile.iProfile=form>=2?XUI_DOCUMENT_MARKDOWN:XUI_DOCUMENT_RICH;
            CHECK(xuiDocumentCreate(&profile,&document)==XUI_OK);
            if(form>=2){uint64_t paragraph;CHECK(xuiDocumentLoadMarkdown(document,value->text,length)==XUI_OK &&
                xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK && xuiDocumentSnapshotGetChild(snapshot,1,0,&paragraph)==XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot,paragraph,0,&ids[0])==XUI_OK);xuiDocumentSnapshotRelease(snapshot);parts=1;last_bytes=length;
            }else{
                xui_document_transaction transaction;xui_doc_node_desc_t node={0};uint64_t paragraph;
                CHECK(xuiDocumentBeginTransaction(document,NULL,&transaction)==XUI_OK);
                node.iSize=sizeof(node);node.iKind=XUI_DOC_PARAGRAPH;
                CHECK(xuiDocumentTxnInsertNode(transaction,1,XUI_DOCUMENT_APPEND,&node,&paragraph)==XUI_OK);
                while(at<length){unsigned bytes=form?scalar_bytes((unsigned char)value->text[at]):length;
                    node.iKind=XUI_DOC_TEXT;node.sText=value->text+at;node.iTextBytes=bytes;
                    node.tAttributes.iTextColor=parts&1?XUI_COLOR_RGBA(230,30,40,255):XUI_COLOR_RGBA(30,40,230,255);
                    CHECK(parts<8 && xuiDocumentTxnInsertNode(transaction,paragraph,XUI_DOCUMENT_APPEND,&node,&ids[parts])==XUI_OK);
                    parts++;at+=bytes;last_bytes=bytes;
                }
                CHECK(xuiDocumentTxnCommit(transaction,NULL)==XUI_OK);xuiDocumentTxnRelease(transaction);
            }
            desc.iSize=sizeof(desc);desc.tFonts=(xui_doc_font_set_t){font,font,font,font,font};desc.iTextColor=XUI_COLOR_RGBA(255,255,255,255);
            CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK && xuiDocumentRendererCreate(context,&desc,&renderer)==XUI_OK &&
                xuiDocumentRendererSetSnapshot(renderer,snapshot,NULL)==XUI_OK);xuiDocumentSnapshotRelease(snapshot);
            start=position(document,ids[0],0,mode);end=position(document,ids[parts-1],mode==XUI_DOC_VISUAL?last_bytes:length,mode);
            CHECK(xuiDocumentRendererSetMode(renderer,mode)==XUI_OK && (mode!=XUI_DOC_LIVE_MARKDOWN||xuiDocumentRendererSetActivePosition(renderer,&start)==XUI_OK) &&
                xuiDocumentRendererLayout(renderer,W,0,H)==XUI_OK && xuiDocumentRendererGetCaretRect(renderer,&start,&first)==XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer,&end,&last)==XUI_OK);
            if(fabs(fabs(last.x-first.x)-width*size)>.002)fprintf(stderr,"width=%g expected=%g\n",fabs(last.x-first.x),width*size);
            CHECK(fabs(fabs(last.x-first.x)-width*size)<.002 && fabs(last.y-first.y)<.001);
            CHECK(xuiDocumentRendererHitTest(renderer,last.x,last.y+last.height*.5,&hit)==XUI_OK &&
                xuiDocumentRendererGetCaretRect(renderer,&hit,&clicked)==XUI_OK && fabs(clicked.x-last.x)<.002);
            CHECK(proxy.surfaceClear(&proxy,targets[0],0)==XUI_OK && proxy.drawBegin(&proxy,&draw,targets[0])==XUI_OK &&
                xuiDocumentRendererDraw(renderer,draw,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK && proxy.drawEnd(&proxy,draw)==XUI_OK &&
                proxy.surfaceReadRGBA(&proxy,targets[0],actual,W*4)==XUI_OK);
            CHECK(proxy.surfaceClear(&proxy,targets[1],0)==XUI_OK && proxy.drawBegin(&proxy,&draw,targets[1])==XUI_OK &&
                proxy.drawText(&proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=reference, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_CLIP|(value->rtl?XUI_TEXT_RTL:0)) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, (xui_rect_t){(float)fmin(first.x,last.x),(float)first.y,(float)(width*size),(float)first.height}, XUI_COLOR_RGBA(255,255,255,255), XUI_TEXT_CLIP|(value->rtl?XUI_TEXT_RTL:0))==XUI_OK && proxy.drawEnd(&proxy,draw)==XUI_OK &&
                proxy.surfaceReadRGBA(&proxy,targets[1],expected,W*4)==XUI_OK);
            {unsigned drawn=0;for(i=0;i<W*H;i++){if(actual[i*4+3]!=expected[i*4+3])fprintf(stderr,"alpha mismatch pixel=%u got=%u expected=%u\n",i,actual[i*4+3],expected[i*4+3]);
                CHECK(actual[i*4+3]==expected[i*4+3]);drawn+=actual[i*4+3]>0;}CHECK(drawn>20);}
            xuiDocumentRendererRelease(renderer);xuiDocumentRelease(document);cases++;
        }
        proxy.fontDestroy(&proxy,font);xgeFontFallbackClear();
    }
    proxy.surfaceDestroy(&proxy,targets[0]);proxy.surfaceDestroy(&proxy,targets[1]);xuiDestroy(context);
    printf("Native Document Script_Extensions: %u cases at %g; Rich whole/split colors, Markdown Visual/Source/Live, caret/hit, full/partial fallback, independent per-glyph widths and exact GPU alpha passed\n",cases,(double)size);
    xgeQuit();return XGE_OK;
}
int main(void)
{
    xge_desc_t engine={0};float sizes[]={40,37};unsigned i;
    engine.iWidth=W;engine.iHeight=H;engine.sTitle="Document Script_Extensions";engine.iFlags=XGE_INIT_OFFSCREEN;engine.iRunMode=XGE_RUN_GAME_LOOP;
    for(i=0;i<2;i++){CHECK(xgeInit(&engine)==XGE_OK && xgeRun(frame,&sizes[i])==XGE_OK);xgeUnit();}return 0;
}
