#include "../test/xge_opentype_fixture_path.h"
#include "../xge.h"
#include "../xui_document_ui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); exit(1); } } while(0)
enum { W=128,H=100 };
#ifdef TEST_LEGACY_LIGATURE_PAINT
static xui_draw_text_spans_proc real_spans;
static int legacy_spans(xui_proxy p, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{

    return real_spans(p, draw, pTextItem, rect, count?spans[count-1].iColor:color, flags, NULL, 0);
}
#endif
static void plain(xui_document d,const char* expected)
{
    xui_document_snapshot s; char* text; uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(d,&s)==XUI_OK && xuiDocumentSnapshotCopyPlainText(s,&text,&bytes)==XUI_OK);
    if(bytes!=strlen(expected) || strcmp(text,expected)) fprintf(stderr,"Expected [%s] %zu bytes, got [%s] %llu bytes\n",expected,strlen(expected),text,(unsigned long long)bytes);
    CHECK(bytes==strlen(expected) && !strcmp(text,expected)); xuiDocumentFreeBuffer(text); xuiDocumentSnapshotRelease(s);
}
static xui_doc_position_t pos(xui_document d,uint64_t id,uint64_t offset)
{
    xui_doc_position_t p={0}; p.iSize=sizeof(p); p.iKind=XUI_DOC_POSITION_TEXT;
    p.iDocumentId=xuiDocumentGetIdentity(d); p.iRevision=xuiDocumentGetRevision(d); p.iNodeId=id; p.iOffset=offset; return p;
}
static uint64_t insert(xui_document_transaction t,uint64_t parent,const char* text,uint32_t color)
{
    xui_doc_node_desc_t n={0}; uint64_t id;
    n.iSize=sizeof(n); n.iKind=text?XUI_DOC_TEXT:XUI_DOC_PARAGRAPH; n.sText=text; n.iTextBytes=text?strlen(text):0;
    n.tAttributes.iTextColor=color; CHECK(xuiDocumentTxnInsertNode(t,parent,XUI_DOCUMENT_APPEND,&n,&id)==XUI_OK); return id;
}
#include "xui_document_ligature_mark_cases.h"
static int frame(void* user)
{
    xui_proxy_t proxy=xuiProxyXge(); xui_context context; xui_font font; xui_text_shape_t shape={0};
    xui_doc_renderer_desc_t desc={0}; xui_document d[3]; xui_document_renderer r[3]; xui_surface surface[3];
    uint64_t ids[3][3]={{0}}; xui_doc_rect_t carets[3][4]; unsigned char pixels[3][W*H*4];
    const uint32_t colors[]={XUI_COLOR_RGBA(20,40,200,255),XUI_COLOR_RGBA(200,20,40,255),XUI_COLOR_RGBA(20,200,40,255)};
    unsigned v,k,pass; unsigned colored[3]={0}; float size=*(float*)user;
#ifdef TEST_LEGACY_LIGATURE_PAINT
    real_spans=proxy.drawTextSpans; proxy.drawTextSpans=legacy_spans;
#endif
    CHECK(proxy.fontLoadFile(&proxy,&font,XGE_TEST_OPENTYPE_FIXTURE,size,0)==XUI_OK);
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy)==XUI_OK && xuiSetDefaultFont(context,font)==XUI_OK);
    CHECK(xuiTextShape(context, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText="ffi", .iTextSize=3, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &shape)==XUI_OK);
    CHECK(shape.iClusterCount==1 && shape.iCaretCount==2 && shape.pCarets[0].iTextOffset==1 && shape.pCarets[1].iTextOffset==2);
    CHECK(fabs(shape.pCarets[0].fAdvance-size*.25)<.001 && fabs(shape.pCarets[1].fAdvance-size*.8)<.001);
    xuiTextShapeFree(&shape);
    desc.iSize=sizeof(desc); desc.tFonts=(xui_doc_font_set_t){font,font,font,font,font}; desc.iTextColor=colors[0];
    for(v=0;v<3;v++) {
        xui_document_snapshot s; xui_surface_desc_t target={0};
        xui_doc_desc_t document_desc={0};
        document_desc.iSize=sizeof(document_desc); document_desc.iProfile=XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(v==2?&document_desc:NULL,&d[v])==XUI_OK);
        if(v==2) {
            uint64_t paragraph;
            CHECK(xuiDocumentLoadMarkdown(d[v],"f*f*i",5)==XUI_OK && xuiDocumentAcquireSnapshot(d[v],&s)==XUI_OK);
            CHECK(xuiDocumentSnapshotGetChild(s,1,0,&paragraph)==XUI_OK);
            for(k=0;k<3;k++) CHECK(xuiDocumentSnapshotGetChild(s,paragraph,k,&ids[v][k])==XUI_OK);
            xuiDocumentSnapshotRelease(s);
        } else {
            xui_document_transaction t; uint64_t paragraph;
            CHECK(xuiDocumentBeginTransaction(d[v],NULL,&t)==XUI_OK); paragraph=insert(t,1,NULL,0);
            if(v) for(k=0;k<3;k++) ids[v][k]=insert(t,paragraph,k==2?"i":"f",colors[k]);
            else ids[v][0]=ids[v][1]=ids[v][2]=insert(t,paragraph,"ffi",0);
            CHECK(xuiDocumentTxnCommit(t,NULL)==XUI_OK); xuiDocumentTxnRelease(t);
        }
        CHECK(xuiDocumentAcquireSnapshot(d[v],&s)==XUI_OK && xuiDocumentRendererCreate(context,&desc,&r[v])==XUI_OK &&
            xuiDocumentRendererSetSnapshot(r[v],s,NULL)==XUI_OK); xuiDocumentSnapshotRelease(s);
        target.iKind=XUI_SURFACE_KIND_TEXTURE; target.iFormat=XUI_SURFACE_FORMAT_RGBA8; target.iWidth=W; target.iHeight=H;
        target.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
        CHECK(proxy.surfaceCreate(&proxy,&surface[v],&target)==XUI_OK);
    }
    for(pass=0;pass<3;pass++) for(v=0;v<3;v++) {
        xui_draw_context draw;
        CHECK(xuiDocumentRendererLayout(r[v],pass==1?size+1:W,0,H)==XUI_OK);
        for(k=0;k<4;k++) {
            xui_doc_position_t p=pos(d[v],ids[v][k==3?2:k],v?(k==3?1:0):k);
            CHECK(xuiDocumentRendererGetCaretRect(r[v],&p,&carets[v][k])==XUI_OK);
            CHECK(fabs(carets[v][k].x-carets[v][0].x-size*(k==0?0:k==1?.25:k==2?.8:1))<.001);
            CHECK(fabs(carets[v][k].y-carets[v][0].y)<.001);
            if(k>0 && k<3) {
                xui_doc_position_t hit; xui_doc_rect_t clicked;
                CHECK(xuiDocumentRendererHitTest(r[v],carets[v][k].x-.1,carets[v][k].y+10,&hit)==XUI_OK &&
                    xuiDocumentRendererGetCaretRect(r[v],&hit,&clicked)==XUI_OK && fabs(clicked.x-carets[v][k].x)<.001);
            }
        }
        {
            xui_doc_range_t range={pos(d[v],ids[v][0],0),pos(d[v],ids[v][v?1:0],v?0:1)};
            xui_doc_rect_t rects[4]; uint64_t total;
            CHECK(xuiDocumentRendererGetRangeRects(r[v],&range,rects,4,&total)==XUI_OK && total==1 && fabs(rects[0].width-size*.25)<.001);
        }
        CHECK(proxy.surfaceClear(&proxy,surface[v],0)==XUI_OK && proxy.drawBegin(&proxy,&draw,surface[v])==XUI_OK &&
            xuiDocumentRendererDraw(r[v],draw,0,0,(xui_rect_t){0,0,W,H},NULL,0)==XUI_OK && proxy.drawEnd(&proxy,draw)==XUI_OK &&
            proxy.surfaceReadRGBA(&proxy,surface[v],pixels[v],W*4)==XUI_OK);
    }
    for(k=0;k<W*H;k++) {
        unsigned char* p=pixels[1]+k*4; unsigned zone;
        CHECK(pixels[0][k*4+3]==p[3] && pixels[0][k*4+3]==pixels[2][k*4+3]);
        if(p[3]<128)continue;
        zone=(k%W+.5<carets[1][1].x)?0:(k%W+.5<carets[1][2].x)?1:2;
        CHECK((zone==0 && p[2]>p[0]+80 && p[2]>p[1]+80) ||
              (zone==1 && p[0]>p[1]+80 && p[0]>p[2]+80) ||
              (zone==2 && p[1]>p[0]+80 && p[1]>p[2]+80)); colored[zone]++;
    }
    CHECK(colored[0]>10 && colored[1]>10 && colored[2]>10);
    CHECK(xuiDocumentLoadMarkdown(d[2],"ffi",3)==XUI_OK);
    for(k=0;k<2;k++) {
        xui_document_renderer source_renderer; xui_document_snapshot s;
        xui_doc_position_t p=pos(d[2],1,0),hit; xui_doc_rect_t origin,caret,clicked; unsigned stop;
        p.iKind=XUI_DOC_POSITION_SOURCE;
        CHECK(xuiDocumentAcquireSnapshot(d[2],&s)==XUI_OK && xuiDocumentRendererCreate(context,&desc,&source_renderer)==XUI_OK &&
            xuiDocumentRendererSetSnapshot(source_renderer,s,NULL)==XUI_OK); xuiDocumentSnapshotRelease(s);
        CHECK(xuiDocumentRendererSetMode(source_renderer,k?XUI_DOC_LIVE_MARKDOWN:XUI_DOC_SOURCE_TEXT)==XUI_OK &&
            (!k || xuiDocumentRendererSetActivePosition(source_renderer,&p)==XUI_OK) && xuiDocumentRendererLayout(source_renderer,W,0,H)==XUI_OK &&
            xuiDocumentRendererGetCaretRect(source_renderer,&p,&origin)==XUI_OK);
        for(stop=1;stop<=2;stop++) {
            p.iOffset=stop;
            CHECK(xuiDocumentRendererGetCaretRect(source_renderer,&p,&caret)==XUI_OK && fabs(caret.x-origin.x-size*(stop==1?.25:.8))<.001 &&
                xuiDocumentRendererHitTest(source_renderer,caret.x-.1,caret.y+10,&hit)==XUI_OK && hit.iKind==XUI_DOC_POSITION_SOURCE &&
                xuiDocumentRendererGetCaretRect(source_renderer,&hit,&clicked)==XUI_OK && fabs(clicked.x-caret.x)<.001);
        }
        xuiDocumentRendererRelease(source_renderer);
    }
    {
        xge_font_t raw={0}; xge_text_shape_desc_t shaped={0}; xge_glyph_run_t run={0};
        xge_text_decoration_t decoration={0}; xui_draw_context draw; int left=W,right=-1; unsigned count=0;
        CHECK(xgeFontLoad(&raw,XGE_TEST_OPENTYPE_FIXTURE,size)==XGE_OK);
        shaped.iSize=sizeof(shaped); shaped.pFont=&raw; shaped.sText="ffi"; shaped.iTextSize=3; shaped.iFlags=XGE_TEXT_SHAPE_DEFAULT;
        CHECK(xgeTextShape(&shaped,&run)==XGE_OK);
        decoration.iSize=sizeof(decoration); decoration.iType=XGE_TEXT_DECORATION_UNDERLINE; decoration.iColor=colors[1];
        decoration.iFlags=XGE_TEXT_DECORATION_RANGE|XGE_TEXT_DECORATION_SCREEN_SPACE; decoration.iStart=1; decoration.iEnd=2; decoration.fThickness=2;
        CHECK(proxy.surfaceClear(&proxy,surface[0],0)==XUI_OK && proxy.drawBegin(&proxy,&draw,surface[0])==XUI_OK);
        xgeGlyphRunDrawDecorated(&run,0,0,colors[0],XGE_DRAW_SCREEN_SPACE,&decoration,1);
        CHECK(proxy.drawEnd(&proxy,draw)==XUI_OK && proxy.surfaceReadRGBA(&proxy,surface[0],pixels[0],W*4)==XUI_OK);
        for(k=0;k<W*H;k++) {
            unsigned char* p=pixels[0]+k*4; int x=(int)(k%W);
            if(p[3]<128 || p[0]<=p[2]+80)continue;
            if(x<left)left=x;
            if(x>right)right=x;
            count++;
        }
        CHECK(count>10 && abs(left-(int)floor(size*.25))<=1 && abs(right-(int)ceil(size*.8)+1)<=1);
        xgeGlyphRunFree(&run); xgeFontFree(&raw);
    }
    {
        xui_widget editor; xui_doc_editor_desc_t ed={0}; xui_doc_range_t selection={pos(d[0],ids[0][0],0),pos(d[0],ids[0][0],0)};
        ed.iSize=sizeof(ed); ed.tView.iSize=sizeof(ed.tView); ed.tView.pDocument=d[0]; ed.tView.tRenderer=desc; ed.iMode=XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context,&ed,&editor)==XUI_OK && xuiSetRootWidget(context,editor)==XUI_OK &&
            xuiWidgetSetRect(editor,(xui_rect_t){0,0,W,H})==XUI_OK && xuiLayout(context)==XUI_OK && xuiSetFocusWidget(context,editor)==XUI_OK &&
            xuiDocumentViewSetSelection(editor,&selection)==XUI_OK);
        for(k=1;k<=2;k++) {
            CHECK(xuiInputKeyDown(context,XUI_KEY_RIGHT,0)==XUI_OK && xuiDocumentViewGetSelection(editor,&selection)==XUI_OK && selection.tCaret.iOffset==k);
        }
        CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_BACKSPACE)==XUI_OK); plain(d[0],"fi\n");
        CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK); plain(d[0],"ffi\n");
        selection.tAnchor=selection.tCaret=pos(d[0],ids[0][0],2);
        CHECK(xuiDocumentViewSetSelection(editor,&selection)==XUI_OK && xuiDocumentEditorInsertText(editor,"x",1)==XUI_OK); plain(d[0],"ffxi\n");
        CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK); plain(d[0],"ffi\n");
        xuiWidgetDestroy(editor);
    }
    document_ligature_marks(context,&proxy,&desc,surface,size,0);
    for(v=0;v<3;v++) { proxy.surfaceDestroy(&proxy,surface[v]); xuiDocumentRendererRelease(r[v]); xuiDocumentRelease(d[v]); }
    xuiDestroy(context); proxy.fontDestroy(&proxy,font);
    puts("Native Document OpenType: Rich joined/split colors and Markdown, VISUAL/SOURCE/LIVE GDEF caret/hit/ranges/reflow, identical alpha and three-color GPU pixels, partial native underline, Editor arrows/delete/insert/Undo passed");
    xgeQuit(); return XGE_OK;
}
int main(void)
{
    xge_desc_t engine={0}; float sizes[]={40,37}; unsigned i;
    engine.iWidth=W; engine.iHeight=H; engine.sTitle="Document OpenType fixture";
    engine.iFlags=XGE_INIT_OFFSCREEN; engine.iRunMode=XGE_RUN_GAME_LOOP;
    for(i=0;i<sizeof(sizes)/sizeof(*sizes);i++) {
        CHECK(xgeInit(&engine)==XGE_OK && xgeRun(frame,&sizes[i])==XGE_OK); xgeUnit();
    }
    return 0;
}
