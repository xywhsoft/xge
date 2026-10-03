#include "../test/xge_opentype_fixture_path.h"
#include "xge.h"
#include "xui.h"
#include "lib/harfbuzz/src/hb.h"
#include "lib/harfbuzz/src/hb-ot.h"
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do { if(!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); exit(1); } } while(0)
#define HEBREW "\xd7\x90\xd7\x91\xd7\x92"
enum { W=128,H=100 };
#ifdef TEST_IGNORE_RTL_PAINT
static xui_draw_text_spans_proc real_spans;
static int ignore_direction(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags, const xui_text_paint_span_t* spans, int count)
{
    xui_font font = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->pFont : NULL;
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;
 return real_spans(proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=text, .iTextSize=bytes, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((flags&~XUI_TEXT_RTL) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0), .sContext=pTextItem->sContext, .iContextSize=pTextItem->iContextSize, .iContextOffset=pTextItem->iContextOffset, .iScript=pTextItem->iScript, .sLanguage=pTextItem->sLanguage}, rect, color, flags&~XUI_TEXT_RTL, spans, count); }
#endif
static void near(float a,float b) { if(fabsf(a-b)>.001f)fprintf(stderr,"got %f expected %f\n",a,b); CHECK(fabsf(a-b)<.001f); }
static xge_glyph_run_t shape(xge_font font,const char* text)
{
    xge_text_shape_desc_t d={0}; xge_glyph_run_t run={0};
    d.iSize=sizeof(d); d.pFont=font; d.sText=text; d.iTextSize=(int)strlen(text); d.iFlags=XGE_TEXT_SHAPE_DEFAULT|XGE_TEXT_SHAPE_RTL;
    CHECK(xgeTextShape(&d,&run)==XGE_OK); return run;
}
/* Independently shape an SFNT buffer through the upstream C API. Compare its
 * physical glyph sequence with XGE's logical storage and explicit visual pens. */
static void reference(const char* path,xge_font font,const char* text,const xge_glyph_run_t* run)
{
    FILE* input=fopen(path,"rb"); long bytes; char* data; unsigned count,i; float pen=0;
    hb_blob_t* blob; hb_face_t* face; hb_font_t* hbfont; hb_buffer_t* buffer;
    hb_glyph_info_t* infos; hb_glyph_position_t* positions; unsigned char matched[256]={0};
    CHECK(input && !fseek(input,0,SEEK_END) && (bytes=ftell(input))>0 && !fseek(input,0,SEEK_SET));
    data=malloc((size_t)bytes); CHECK(data && fread(data,1,(size_t)bytes,input)==(size_t)bytes); fclose(input);
    blob=hb_blob_create(data,(unsigned)bytes,HB_MEMORY_MODE_READONLY,NULL,NULL); face=hb_face_create(blob,0); hbfont=hb_font_create(face);
    hb_ot_font_set_funcs(hbfont); hb_font_set_scale(hbfont,(int)hb_face_get_upem(face),(int)hb_face_get_upem(face));
    buffer=hb_buffer_create(); hb_buffer_set_direction(buffer,HB_DIRECTION_RTL); hb_buffer_set_cluster_level(buffer,HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
    hb_buffer_add_utf8(buffer,text,(int)strlen(text),0,(int)strlen(text)); hb_buffer_guess_segment_properties(buffer); hb_shape(hbfont,buffer,NULL,0);
    infos=hb_buffer_get_glyph_infos(buffer,&count); positions=hb_buffer_get_glyph_positions(buffer,NULL);
    CHECK(count==(unsigned)run->iGlyphCount && count<sizeof(matched));
    for(i=0;i<count;i++) {
        unsigned j;
        for(j=0;j<count;j++)if(!matched[j] && run->pGlyphs[j].iCluster==infos[i].cluster && run->pGlyphs[j].iGlyph==(int)infos[i].codepoint)break;
        CHECK(j<count); matched[j]=1;
        near(run->pGlyphs[j].fVisualX,pen); near(run->pGlyphs[j].fAdvanceX,positions[i].x_advance*font->fScale);
        near(run->pGlyphs[j].fOffsetX,positions[i].x_offset*font->fScale); near(run->pGlyphs[j].fOffsetY,-positions[i].y_offset*font->fScale);
        pen+=positions[i].x_advance*font->fScale;
    }
    hb_buffer_destroy(buffer); hb_font_destroy(hbfont); hb_face_destroy(face); hb_blob_destroy(blob); free(data);
}
static void geometry(float size)
{
    xge_font_t font={0}; xge_glyph_run_t run; float s; uint32_t at; int trailing;
    CHECK(xgeFontLoad(&font,XGE_TEST_OPENTYPE_FIXTURE,size)==XGE_OK); s=font.fScale;
    run=shape(&font,HEBREW); CHECK(run.iGlyphCount==1 && run.pGlyphs[0].iGlyph==20 && run.pGlyphs[0].iClusterEnd==6 && run.iCaretCount==2);
    CHECK(run.pCarets[0].iTextOffset==2 && run.pCarets[1].iTextOffset==4);
    near(run.pCarets[0].fAdvance,200*s); near(run.pCarets[1].fAdvance,750*s); near(run.pGlyphs[0].fVisualX,0); near(run.fWidth,1000*s);
    reference(XGE_TEST_OPENTYPE_FIXTURE,&font,HEBREW,&run);
    CHECK(xgeGlyphRunHitTest(&run,NAN,0,&at,&trailing)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xgeGlyphRunHitTest(&run,0,INFINITY,&at,&trailing)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xgeGlyphRunHitTest(&run,0,FLT_MAX,&at,&trailing)==XGE_OK && at==6 && !trailing);
    CHECK(xgeGlyphRunHitTest(&run,900*s,0,&at,&trailing)==XGE_OK && !at && !trailing);
    CHECK(xgeGlyphRunHitTest(&run,100*s,0,&at,&trailing)==XGE_OK && !at && trailing); xgeGlyphRunFree(&run);
    run=shape(&font,"\xd7\x90\xd6\xb0\xd7\x91\xd7\x92");
    CHECK(run.iGlyphCount==2 && run.pGlyphs[0].iGlyph==10 && run.pGlyphs[1].iGlyph==20 &&
        run.pGlyphs[0].iClusterEnd==8 && run.pGlyphs[1].iCluster==0 && run.iCaretCount==2 &&
        run.pCarets[0].iTextOffset==4 && run.pCarets[1].iTextOffset==6);
    near(run.pGlyphs[1].fAdvanceX,900*s);near(run.pGlyphs[1].fOffsetX,-30*s);
    near(run.pCarets[0].fAdvance,130*s);near(run.pCarets[1].fAdvance,680*s);
    reference(XGE_TEST_OPENTYPE_FIXTURE,&font,"\xd7\x90\xd6\xb0\xd7\x91\xd7\x92",&run);
    xgeGlyphRunFree(&run);
    run=shape(&font,"\xd7\x90 \xd7\x91"); CHECK(run.iGlyphCount==3 && run.pGlyphs[0].iCluster==0 && run.pGlyphs[1].iCluster==2 && run.pGlyphs[2].iCluster==3);
    near(run.pGlyphs[0].fVisualX,850*s); near(run.pGlyphs[1].fVisualX,600*s); near(run.pGlyphs[2].fVisualX,0);
    reference(XGE_TEST_OPENTYPE_FIXTURE,&font,"\xd7\x90 \xd7\x91",&run);
    CHECK(xgeGlyphRunHitTest(&run,1400*s,0,&at,&trailing)==XGE_OK && at==0 && !trailing);
    CHECK(xgeGlyphRunHitTest(&run,870*s,0,&at,&trailing)==XGE_OK && at==0 && trailing);
    CHECK(xgeGlyphRunHitTest(&run,590*s,0,&at,&trailing)==XGE_OK && at==3 && !trailing);
    CHECK(xgeGlyphRunHitTest(&run,-10*s,0,&at,&trailing)==XGE_OK && at==3 && trailing); xgeGlyphRunFree(&run);
    run=shape(&font,"\xd7\x90\xd6\xb0 \xd7\x91");
    CHECK(run.iGlyphCount==4 && run.pGlyphs[0].iCluster==0 && run.pGlyphs[1].iCluster==0 && !run.iCaretCount);
    near(run.pGlyphs[0].fVisualX+run.pGlyphs[0].fAdvanceX,run.pGlyphs[1].fVisualX);
    CHECK(run.pGlyphs[0].iGlyph==10 && run.pGlyphs[1].iGlyph==17); near(run.pGlyphs[0].fAdvanceX,0);
    near(run.pGlyphs[0].fOffsetX,350*s); near(run.pGlyphs[0].fOffsetY,-700*s);
    reference(XGE_TEST_OPENTYPE_FIXTURE,&font,"\xd7\x90\xd6\xb0 \xd7\x91",&run);
    CHECK(xgeGlyphRunHitTest(&run,1400*s,0,&at,&trailing)==XGE_OK && at==0 && !trailing); xgeGlyphRunFree(&run);
    run=shape(&font,"\xd7\x90\r\n\xd7\x90 \xd7\x91"); CHECK(run.iGlyphCount==5 && run.pGlyphs[1].iFlags==XGE_GLYPH_POSITION_LINE_BREAK);
    near(run.pGlyphs[0].fVisualX,0); near(run.pGlyphs[2].fVisualX,850*s);
    CHECK(xgeGlyphRunHitTest(&run,1400*s,run.fLineHeight+1,&at,&trailing)==XGE_OK && at==4 && !trailing); xgeGlyphRunFree(&run);
    xgeFontFree(&font);
    {
        xge_font_t arabic={0}; const char* text="\xd8\xb3\xd9\x84\xd8\xa7\xd9\x85";
        CHECK(xgeFontLoad(&arabic,"C:/Windows/Fonts/segoeui.ttf",size)==XGE_OK); run=shape(&arabic,text);
        CHECK(run.iGlyphCount>0 && run.iGlyphCount<=4 && run.pGlyphs[0].iCluster==0);
        reference("C:/Windows/Fonts/segoeui.ttf",&arabic,text,&run);
        { xge_glyph_metrics_t nominal; CHECK(xgeFontGlyphGet(&arabic,0x633,&nominal)==XGE_OK && run.pGlyphs[0].iGlyph!=nominal.iGlyph); }
        for(int i=1;i<run.iGlyphCount;i++)CHECK(run.pGlyphs[i].iCluster>=run.pGlyphs[i-1].iCluster && run.pGlyphs[i].fVisualX<=run.pGlyphs[i-1].fVisualX+.001f);
        printf("Arabic native: 4 codepoints -> %d contextual glyphs, %d interior stops\n",run.iGlyphCount,run.iCaretCount);
        xgeGlyphRunFree(&run); run=shape(&arabic,"(\xd7\x90)");
        reference("C:/Windows/Fonts/segoeui.ttf",&arabic,"(\xd7\x90)",&run);
        { xge_glyph_metrics_t mirrored; CHECK(run.iGlyphCount==3 && run.pGlyphs[0].iCluster==0 && run.pGlyphs[0].iCodepoint=='(' &&
            xgeFontGlyphGet(&arabic,')',&mirrored)==XGE_OK && run.pGlyphs[0].iGlyph==mirrored.iGlyph); }
        xgeFontFree(&arabic); CHECK(arabic.iRefCount>0); xgeGlyphRunFree(&run); CHECK(!arabic.iRefCount);
    }
}
static int frame(void* user)
{
    float size=*(float*)user; xui_proxy_t proxy=xuiProxyXge(); xui_context context; xui_font font;
    xui_surface surface[3]; unsigned char pixels[3][W*H*4]; xui_text_shape_t projected={0};
    const uint32_t colors[]={XUI_COLOR_RGBA(20,40,200,255),XUI_COLOR_RGBA(200,20,40,255),XUI_COLOR_RGBA(20,200,40,255)};
    xui_text_paint_span_t spans[3]={{0}};
    unsigned colored[3]={0}; unsigned v,k;
    for(k=0;k<3;k++) { spans[k].iSize=sizeof(spans[k]); spans[k].iStart=k*2; spans[k].iEnd=k*2+2; spans[k].iColor=colors[k]; }
    CHECK(proxy.fontLoadFile(&proxy,&font,XGE_TEST_OPENTYPE_FIXTURE,size,0)==XUI_OK && xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy)==XUI_OK);
    {
        xui_proxy_t without_shape=proxy; xui_context fallback; without_shape.textShape=NULL;
        CHECK(xuiCreate(&fallback)==XUI_OK && xuiSetProxy(fallback,&without_shape)==XUI_OK && xuiTextShape(fallback, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=HEBREW, .iTextSize=6, .iFlags=XUI_TEXT_SHAPE_RTL}, &projected)==XUI_ERROR_UNSUPPORTED && !projected.pClusters && !projected.iClusterCount);
        xuiTextShapeFree(&projected); xuiDestroy(fallback);
    }
#ifdef TEST_IGNORE_RTL_PAINT
    real_spans=proxy.drawTextSpans; proxy.drawTextSpans=ignore_direction;
#endif
    CHECK(xuiTextShape(context, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=HEBREW, .iTextSize=6, .iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RTL}, &projected)==XUI_OK);
    CHECK(projected.iClusterCount==1 && projected.iCaretCount==2 && projected.pCarets[0].iTextOffset==2 && projected.pCarets[1].iTextOffset==4);
    near(projected.pCarets[0].fAdvance,size*.2f); near(projected.pCarets[1].fAdvance,size*.75f); xuiTextShapeFree(&projected);
    for(v=0;v<3;v++) {
        xui_surface_desc_t target={0}; xui_draw_context draw;
        target.iKind=XUI_SURFACE_KIND_TEXTURE; target.iFormat=XUI_SURFACE_FORMAT_RGBA8; target.iWidth=W; target.iHeight=H; target.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
        CHECK(proxy.surfaceCreate(&proxy,&surface[v],&target)==XUI_OK && proxy.surfaceClear(&proxy,surface[v],0)==XUI_OK && proxy.drawBegin(&proxy,&draw,surface[v])==XUI_OK);
        if(v<2)CHECK(proxy.drawTextSpans(&proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=HEBREW, .iTextSize=6, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_RTL|XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, (xui_rect_t){0,0,W,H}, colors[0], XUI_TEXT_RTL|XUI_TEXT_CLIP, v?spans:NULL, v?3:0)==XUI_OK);
        else CHECK(proxy.drawText(&proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=HEBREW, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_RTL|XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, (xui_rect_t){0,0,W,H}, colors[0], XUI_TEXT_RTL|XUI_TEXT_CLIP)==XUI_OK);
        CHECK(proxy.drawEnd(&proxy,draw)==XUI_OK && proxy.surfaceReadRGBA(&proxy,surface[v],pixels[v],W*4)==XUI_OK);
    }
    CHECK(!memcmp(pixels[0],pixels[2],sizeof(pixels[0])));
    for(k=0;k<W*H;k++) {
        unsigned char* p=pixels[1]+k*4; unsigned zone;
        CHECK(pixels[0][k*4+3]==p[3]); if(p[3]<128)continue;
        zone=k%W+.5<size*.25?2:k%W+.5<size*.8?1:0;
        CHECK((zone==0 && p[2]>p[0]+80 && p[2]>p[1]+80) || (zone==1 && p[0]>p[1]+80 && p[0]>p[2]+80) || (zone==2 && p[1]>p[0]+80 && p[1]>p[2]+80)); colored[zone]++;
    }
    CHECK(colored[0]>10 && colored[1]>10 && colored[2]>10);
    CHECK(proxy.surfaceClear(&proxy,surface[2],0)==XUI_OK && proxy.textDraw(&proxy, surface[2], &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=HEBREW, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_RTL|XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, (xui_rect_t){0,0,W,H}, colors[0], XUI_TEXT_RTL|XUI_TEXT_CLIP)==XUI_OK &&
          proxy.surfaceReadRGBA(&proxy,surface[2],pixels[2],W*4)==XUI_OK && !memcmp(pixels[0],pixels[2],sizeof(pixels[0])));
    {
        xui_draw_context draw; int shift=80-(int)size; xui_rect_t saved,before; int enabled;
        CHECK(proxy.surfaceClear(&proxy,surface[2],0)==XUI_OK && proxy.drawBegin(&proxy,&draw,surface[2])==XUI_OK &&
              proxy.drawClipSet(&proxy,draw,(xui_rect_t){shift+size*.3f,0,size*.4f,H})==XUI_OK &&
              proxy.drawClipGet(&proxy,draw,&before,&enabled)==XUI_OK && enabled &&
              proxy.drawTextSpans(&proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=HEBREW, .iTextSize=6, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_RTL|XUI_TEXT_CLIP|XUI_TEXT_ALIGN_RIGHT) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, (xui_rect_t){10,0,70,H}, colors[0], XUI_TEXT_RTL|XUI_TEXT_CLIP|XUI_TEXT_ALIGN_RIGHT, NULL, 0)==XUI_OK &&
              proxy.drawClipGet(&proxy,draw,&saved,&enabled)==XUI_OK && enabled && saved.fX==before.fX && saved.fY==before.fY && saved.fW==before.fW && saved.fH==before.fH &&
              proxy.drawEnd(&proxy,draw)==XUI_OK && proxy.surfaceReadRGBA(&proxy,surface[2],pixels[2],W*4)==XUI_OK);
        for(k=0;k<W*H;k++) {
            unsigned x=k%W,y=k/W;
            if(x+.5f<shift+size*.3f-1 || x+.5f>shift+size*.7f+1)CHECK(!pixels[2][k*4+3]);
            if(x>shift+size*.3f+1 && x<shift+size*.7f-1)CHECK(pixels[2][k*4+3]==pixels[0][(y*W+x-shift)*4+3]);
        }
    }
    {
        const char* separated="\xd7\x90 \xd7\x91"; xui_draw_context draw;
        CHECK(proxy.surfaceClear(&proxy,surface[1],0)==XUI_OK && proxy.surfaceClear(&proxy,surface[2],0)==XUI_OK &&
              proxy.drawBegin(&proxy,&draw,surface[1])==XUI_OK && proxy.drawText(&proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=separated, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_RTL|XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, (xui_rect_t){0,0,W,H}, colors[0], XUI_TEXT_RTL|XUI_TEXT_CLIP)==XUI_OK &&
              proxy.drawEnd(&proxy,draw)==XUI_OK && proxy.textDraw(&proxy, surface[2], &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=separated, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_RTL|XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, (xui_rect_t){0,0,W,H}, colors[0], XUI_TEXT_RTL|XUI_TEXT_CLIP)==XUI_OK &&
              proxy.surfaceReadRGBA(&proxy,surface[1],pixels[1],W*4)==XUI_OK && proxy.surfaceReadRGBA(&proxy,surface[2],pixels[2],W*4)==XUI_OK &&
              !memcmp(pixels[1],pixels[2],sizeof(pixels[1])));
    }
    {
        xge_font_t raw={0}; xge_glyph_run_t run; xge_text_decoration_t d={0}; xui_draw_context draw; int left=W,right=-1; unsigned count=0;
        CHECK(xgeFontLoad(&raw,XGE_TEST_OPENTYPE_FIXTURE,size)==XGE_OK); run=shape(&raw,HEBREW);
        d.iSize=sizeof(d); d.iType=XGE_TEXT_DECORATION_UNDERLINE; d.iColor=colors[1]; d.iFlags=XGE_TEXT_DECORATION_RANGE|XGE_TEXT_DECORATION_SCREEN_SPACE; d.iStart=2; d.iEnd=4; d.fThickness=2;
        CHECK(proxy.surfaceClear(&proxy,surface[0],0)==XUI_OK && proxy.drawBegin(&proxy,&draw,surface[0])==XUI_OK);
        xgeGlyphRunDrawDecorated(&run,0,0,colors[0],XGE_DRAW_SCREEN_SPACE,&d,1);
        CHECK(proxy.drawEnd(&proxy,draw)==XUI_OK && proxy.surfaceReadRGBA(&proxy,surface[0],pixels[0],W*4)==XUI_OK);
        for(k=0;k<W*H;k++) { unsigned char* p=pixels[0]+k*4; int x=(int)(k%W); if(p[3]<128 || p[0]<=p[2]+80)continue; if(x<left)left=x; if(x>right)right=x; count++; }
        CHECK(count>10 && abs(left-(int)floor(size*.25))<=1 && abs(right-(int)ceil(size*.8)+1)<=1);
        xgeGlyphRunFree(&run); xgeFontFree(&raw);
    }
    for(v=0;v<3;v++)proxy.surfaceDestroy(&proxy,surface[v]);
    xuiDestroy(context); proxy.fontDestroy(&proxy,font); xgeQuit(); return XGE_OK;
}
int main(void)
{
    float sizes[]={40,37}; xge_desc_t engine={0}; unsigned i;
    engine.iWidth=W; engine.iHeight=H; engine.sTitle="RTL native fixture"; engine.iFlags=XGE_INIT_OFFSCREEN; engine.iRunMode=XGE_RUN_GAME_LOOP;
    for(i=0;i<sizeof(sizes)/sizeof(*sizes);i++) { geometry(sizes[i]); CHECK(xgeInit(&engine)==XGE_OK && xgeRun(frame,&sizes[i])==XGE_OK); xgeUnit(); }
    puts("Native RTL: logical clusters, upstream glyph/position oracle, Hebrew GSUB/GDEF/GPOS, Arabic joining, bracket mirroring, hit tests, CRLF, proxy carets, GPU color bands and partial underline passed"); return 0;
}
