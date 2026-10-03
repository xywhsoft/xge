#include "../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); exit(1); } } while(0)
static unsigned cases;
static void shape(xge_font font, const char* text, const unsigned* glyphs, unsigned count, unsigned flags, xge_font expected_font)
{
    xge_text_shape_desc_t desc = {0}; xge_glyph_run_t run = {0}; unsigned i;
    desc.iSize = sizeof(desc); desc.sText = text; desc.iTextSize = (int)strlen(text); desc.pFont = font; desc.iFlags = flags;
    CHECK(xgeTextShape(&desc, &run) == XGE_OK && run.iGlyphCount == (int)count);
    for (i = 0; i < count; i++) {
        unsigned expected = glyphs[i];
        /* HarfBuzz mirrors parentheses for a supplied RTL item direction. */
        if ((flags & XGE_TEXT_SHAPE_RTL) && expected >= 12 && expected <= 15)
            expected = expected % 2 ? expected - 1 : expected + 1;
        if (run.pGlyphs[i].iGlyph != (int)expected) fprintf(stderr,"case=%u glyph=%u got=%d expected=%u text=%s\n",cases,i,run.pGlyphs[i].iGlyph,expected,text);
        CHECK(run.pGlyphs[i].iGlyph == (int)expected && run.pGlyphs[i].pFont == expected_font);
        CHECK(run.pGlyphs[i].iCluster < run.pGlyphs[i].iClusterEnd && run.pGlyphs[i].iClusterEnd <= (unsigned)run.iTextSize);
        if (i) CHECK(run.pGlyphs[i].iCluster >= run.pGlyphs[i - 1].iClusterEnd);
        CHECK(isfinite(run.pGlyphs[i].fAdvanceX) && isfinite(run.pGlyphs[i].fVisualX));
    }
    xgeGlyphRunFree(&run); cases++;
}
int main(void)
{
    xge_font_t full = {0}, partial = {0}, probe = {0}; unsigned direction, size;
    const float sizes[] = {40,37};
    for (size = 0; size < 2; size++) {
        CHECK(xgeFontLoad(&full,"test/data/xge_script_fixture.ttf",sizes[size]) == XGE_OK);
        CHECK(xgeFontLoad(&partial,"test/data/xge_script_no_alaph.ttf",sizes[size]) == XGE_OK);
        CHECK(xgeFontLoad(&probe,"test/data/xge_script_probe_tatweel.ttf",sizes[size]) == XGE_OK);
        xgeFontSetFallback(&partial, &full);
        xgeFontSetFallback(&probe, &full);
        for (direction = 0; direction < 2; direction++) {
            unsigned flags = XGE_TEXT_SHAPE_KERNING | (direction ? XGE_TEXT_SHAPE_RTL : 0);
            const unsigned latin_greek[] = {2,12,3,13,2}, greek[] = {14,3,15,2}, shared[] = {2,16,5}, prefix[] = {16,5};
            const unsigned syriac[] = {7,17,7}, arabic[] = {9,8}, syrc_prefix[] = {17,7}, suffix[] = {4,16,2};
            shape(&full,"a(\xce\xbb)a",latin_greek,5,flags,&full);
            shape(&full,"a\xe3\x80\x88\xce\xbb\xe3\x80\x89" "a",latin_greek,5,flags,&full);
            shape(&full,"(\xce\xbb)a",greek,4,flags,&full);
            shape(&full,"\xe3\x80\x88\xce\xbb\xe3\x80\x89" "a",greek,4,flags,&full);
            shape(&full,"a\xe3\x83\xbc\xe3\x82\xab",shared,3,flags,&full);
            shape(&full,"\xe3\x83\xbc\xe3\x82\xab",prefix,2,flags,&full);
            shape(&full,"\xe3\x81\x82\xe3\x83\xbc" "a",suffix,3,flags,&full);
            shape(&full,"\xdc\x90\xd9\x80\xdc\x90",syriac,3,flags,&full);
            shape(&partial,"\xdc\x90\xd9\x80\xdc\x90",syriac,3,flags,&full);
            shape(&full,"\xd9\x80\xdc\x90",syrc_prefix,2,flags,&full);
            shape(&probe,"\xd9\x80\xdc\x90",syrc_prefix,2,flags,&probe);
            shape(&full,"\xd9\x80\xd8\xa8",arabic,2,flags,&full);
        }
        xgeFontFree(&partial); xgeFontFree(&probe); xgeFontFree(&full); CHECK(!partial.iRefCount && !probe.iRefCount && !full.iRefCount);
    }
    printf("Native SFNT Script_Extensions: %u actual glyph/font cases at 40/37, LTR/RTL; kana-only locl, Arabic-primary tatweel in Syriac, paired punctuation and complete-word fallback passed\n",cases);
    return 0;
}
