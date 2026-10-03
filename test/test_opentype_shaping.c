#include "../test/xge_opentype_fixture_path.h"
#include "xge.h"
#include "xui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); exit(1); } } while(0)
static xge_glyph_run_t shape(xge_font font, const char* text, unsigned flags)
{
    xge_text_shape_desc_t desc = {0}; xge_glyph_run_t run = {0};
    desc.iSize=sizeof(desc); desc.pFont=font; desc.sText=text; desc.iTextSize=(int)strlen(text); desc.iFlags=flags;
    CHECK(xgeTextShape(&desc,&run)==XGE_OK); return run;
}
static void close_float(float actual,float expected) { CHECK(fabsf(actual-expected)<0.001f); }
#include "xge_opentype_gdef_mutations.h"
static void contour_mutations(void)
{
    FILE* file=fopen(XGE_TEST_OPENTYPE_FIXTURE,"rb");long bytes;unsigned char *original,*data;unsigned variant;
    CHECK(file && !fseek(file,0,SEEK_END) && (bytes=ftell(file))>0 && !fseek(file,0,SEEK_SET));
    original=malloc((size_t)bytes);data=malloc((size_t)bytes);CHECK(original && data && fread(original,1,(size_t)bytes,file)==(size_t)bytes);fclose(file);
    for(variant=0;variant<3;variant++){
        xge_font_t font={0};xge_glyph_run_t run;unsigned char* caret;memcpy(data,original,(size_t)bytes);
        caret=fixture_caret(data,(size_t)bytes,variant==2?8:9,variant==2?0:1);
        fixture_put16(caret+(variant==1?0:2),variant==2?0:65535);
        CHECK(xgeFontLoadMemory(&font,data,(int)bytes,32)==XGE_OK);
        run=shape(&font,variant==2?"fi":"ffi",XGE_TEXT_SHAPE_DEFAULT);
        CHECK(run.iCaretCount==(variant==2?1:2));
        if(variant==2)close_float(run.pCarets[0].fAdvance,0);
        else{close_float(run.pCarets[0].fAdvance,1000*font.fScale/3);close_float(run.pCarets[1].fAdvance,2000*font.fScale/3);}
        xgeGlyphRunFree(&run);xgeFontFree(&font);
    }
    free(data);free(original);
    puts("Native Format 2: invalid contour index / unknown format reject entire list; valid original point at x=0 retained");
}
static void contour_bitmaps(void)
{
    const float sizes[]={8,16,32,48,96,200};unsigned i,total=0;
    for(i=0;i<sizeof(sizes)/sizeof(*sizes);i++){
        FILE* pairs=fopen("test/data/xge_opentype_points_fixture.pairs","rb");xge_font_t font={0};unsigned a,b;
        CHECK(pairs && xgeFontLoad(&font,XGE_TEST_OPENTYPE_FIXTURE,sizes[i])==XGE_OK);
        while(fscanf(pairs,"%u %u",&a,&b)==2){
            xge_glyph_bitmap_t composite={0},flat={0};
            CHECK(xgeFontGlyphRasterizeByIndex(&font,(int)a,&composite)==XGE_OK && xgeFontGlyphRasterizeByIndex(&font,(int)b,&flat)==XGE_OK);
            CHECK(composite.iWidth==flat.iWidth && composite.iHeight==flat.iHeight && composite.iOffsetX==flat.iOffsetX && composite.iOffsetY==flat.iOffsetY && composite.pPixels && flat.pPixels);
            CHECK(!memcmp(composite.pPixels,flat.pPixels,(size_t)flat.iWidth*(size_t)flat.iHeight));
            xgeGlyphBitmapFree(&composite);xgeGlyphBitmapFree(&flat);total++;
        }
        fclose(pairs);xgeFontFree(&font);
    }
    printf("Native compound / independent FontTools flattened glyphs: %u bitmap comparisons passed\n",total);
}
int main(void)
{
    xge_font_t font = {0}; xge_glyph_run_t run; float scale; xge_vec2_t measured;
    CHECK(xgeFontLoad(&font,XGE_TEST_OPENTYPE_FIXTURE,32)==XGE_OK); scale=font.fScale;
    run=shape(&font,"fi",XGE_TEXT_SHAPE_KERNING);
    CHECK(run.iGlyphCount==1 && run.pGlyphs[0].iCluster==0 && run.pGlyphs[0].iClusterEnd==2 && run.iCaretCount==1 && run.pCarets[0].iTextOffset==1);
    close_float(run.pGlyphs[0].fAdvanceX,700*scale); close_float(run.pCarets[0].fAdvance,120*scale);
    measured=xgeTextMeasure(&font,"fi"); close_float(measured.fX,run.fWidth); xgeGlyphRunFree(&run);
    run=shape(&font,"ffi",XGE_TEXT_SHAPE_KERNING);
    CHECK(run.iGlyphCount==1 && run.iCaretCount==2 && run.pCarets[0].iTextOffset==1 && run.pCarets[1].iTextOffset==2);
    close_float(run.pCarets[0].fAdvance,250*scale); close_float(run.pCarets[1].fAdvance,800*scale); xgeGlyphRunFree(&run);
    run=shape(&font,"\xce\xbb\xce\xbc",XGE_TEXT_SHAPE_KERNING);
    CHECK(run.iGlyphCount==1 && run.pGlyphs[0].iClusterEnd==4 && run.iCaretCount==1 && run.pCarets[0].iTextOffset==2);
    close_float(run.pCarets[0].fAdvance,300*scale); xgeGlyphRunFree(&run);
    {
        const char* text[]={"fx","ix","fa"}; const float widths[]={900,800,750}; unsigned k;
        for(k=0;k<3;k++) {
            run=shape(&font,text[k],XGE_TEXT_SHAPE_KERNING);
            CHECK(run.iGlyphCount==1 && run.iCaretCount==1 && run.pCarets[0].iTextOffset==1);
            close_float(run.pCarets[0].fAdvance,widths[k]*scale*.5f); xgeGlyphRunFree(&run);
        }
    }
    run=shape(&font,"a\xcc\x81",XGE_TEXT_SHAPE_KERNING);
    CHECK(run.iGlyphCount==2 && run.pGlyphs[0].iCluster==0 && run.pGlyphs[1].iCluster==0 && !run.iCaretCount);
    close_float(run.pGlyphs[1].fAdvanceX,0); close_float(run.pGlyphs[1].fOffsetX,-250*scale); close_float(run.pGlyphs[1].fOffsetY,-700*scale); xgeGlyphRunFree(&run);
    run=shape(&font,"ff\xcc\x81i",XGE_TEXT_SHAPE_KERNING);
    CHECK(run.iGlyphCount==2 && run.pGlyphs[0].iGlyph==9 && run.pGlyphs[1].iGlyph==10 &&
        run.pGlyphs[0].iClusterEnd==5 && run.pGlyphs[1].iCluster==0 && run.iCaretCount==2 &&
        run.pCarets[0].iTextOffset==1 && run.pCarets[1].iTextOffset==4);
    close_float(run.pGlyphs[0].fAdvanceX,900*scale);close_float(run.pGlyphs[0].fOffsetX,70*scale);
    close_float(run.pCarets[0].fAdvance,320*scale);close_float(run.pCarets[1].fAdvance,870*scale);xgeGlyphRunFree(&run);
    run=shape(&font,"ff\xcc\x81i",0);
    CHECK(run.iGlyphCount==2 && run.iCaretCount==2);
    close_float(run.pCarets[0].fAdvance,250*scale);close_float(run.pCarets[1].fAdvance,800*scale);xgeGlyphRunFree(&run);
    run=shape(&font,"f\xcc\x81i",XGE_TEXT_SHAPE_KERNING);
    CHECK(run.iGlyphCount==2 && run.iCaretCount==1 && run.pCarets[0].iTextOffset==3);
    close_float(run.pCarets[0].fAdvance,120*scale);xgeGlyphRunFree(&run);
    {
        const char* text[]={"fy","f\xcc\x81x","i\xcc\x81x","f\xcc\x81" "a"};
        const float advances[]={1300,900,800,750};unsigned k;
        for(k=0;k<4;k++) {
            run=shape(&font,text[k],XGE_TEXT_SHAPE_KERNING);
            CHECK(run.iGlyphCount==2 && run.iCaretCount==1 && run.pCarets[0].iTextOffset==(k?3u:1u));
            close_float(run.pCarets[0].fAdvance,advances[k]*scale*.5f);xgeGlyphRunFree(&run);
        }
    }
    run=shape(&font,"a\xcc\x81i",XGE_TEXT_SHAPE_KERNING);
    { uint32_t at; int trailing;
        CHECK(xgeGlyphRunHitTest(&run,590*scale,0,&at,&trailing)==XGE_OK && at==0 && trailing);
        CHECK(xgeGlyphRunHitTest(&run,610*scale,0,&at,&trailing)==XGE_OK && at==3 && !trailing);
    }
    xgeGlyphRunFree(&run);
    run=shape(&font,"AV",XGE_TEXT_SHAPE_KERNING); CHECK(run.iGlyphCount==2); close_float(run.pGlyphs[0].fAdvanceX+run.pGlyphs[1].fAdvanceX,1100*scale); xgeGlyphRunFree(&run);
    run=shape(&font,"AV",0); close_float(run.pGlyphs[0].fAdvanceX+run.pGlyphs[1].fAdvanceX,1200*scale); xgeGlyphRunFree(&run);
    run=shape(&font,"fi\r\nffi",XGE_TEXT_SHAPE_KERNING); CHECK(run.iGlyphCount==3 && run.pGlyphs[1].iFlags==XGE_GLYPH_POSITION_LINE_BREAK && run.iCaretCount==3); xgeGlyphRunFree(&run);
    {
        const char* paths[]={"C:/Windows/Fonts/calibri.ttf","C:/Windows/Fonts/segoeui.ttf","/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"}; unsigned i; int tested=0;
        for(i=0;i<sizeof(paths)/sizeof(*paths);i++) { FILE* input=fopen(paths[i],"rb"); xge_font_t real={0};
            if(!input)continue;
            fclose(input); CHECK(xgeFontLoad(&real,paths[i],32)==XGE_OK);
            run=shape(&real,"office affinity",XGE_TEXT_SHAPE_KERNING);
            if(run.iGlyphCount<15) {tested=1; printf("Real OpenType font: %s, 15 bytes -> %d glyphs, %d interior stops\n",paths[i],run.iGlyphCount,run.iCaretCount);}
            xgeGlyphRunFree(&run); xgeFontFree(&real);
        }
        CHECK(tested);
    }
    {
        unsigned k;
        for(k=0;k<8;k++) {
            xge_font_t owner={0},sized={0}; xge_glyph_run_t resized;
            CHECK(xgeFontLoad(&owner,XGE_TEST_OPENTYPE_FIXTURE,32)==XGE_OK && xgeFontCreateSized(&sized,&owner,48)==XGE_OK);
            run=shape(&owner,"ffi",XGE_TEXT_SHAPE_KERNING); resized=shape(&sized,"ffi",XGE_TEXT_SHAPE_KERNING);
            close_float(resized.fWidth,run.fWidth*1.5f); close_float(resized.pCarets[0].fAdvance,run.pCarets[0].fAdvance*1.5f);
            xgeFontFree(&owner); xgeFontFree(&sized);
            CHECK(owner.iRefCount==1 && sized.iRefCount==1);
            measured=xgeGlyphRunMeasure(&resized); close_float(measured.fX,1000*48.f/1000.f);
            xgeGlyphRunFree(&resized); xgeGlyphRunFree(&run); CHECK(!owner.iRefCount && !sized.iRefCount);
        }
    }
    xgeFontFree(&font);
    if(strstr(XGE_TEST_OPENTYPE_FIXTURE,"points")){contour_mutations();contour_bitmaps();}
    puts("OpenType native: GSUB fi/ffi/Greek, valid/missing/malformed GDEF stops, GPOS mark/kerning, cluster hit, CRLF, measured width, retained/resized fonts and real-font ligatures passed"); return 0;
}
