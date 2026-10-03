/* Whole-paragraph oracle, deliberately not HB's bounded item API. */
#include "../xge.h"
#include "../lib/harfbuzz/src/hb.h"
#include "../lib/harfbuzz/src/hb-ot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s (case %u)\n",__FILE__,__LINE__,#e,cases); exit(1); } } while(0)
static unsigned cases;
static void marked_item(xge_font font,hb_font_t* oracle)
{
    const char text[]="\xd8\xa8\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e\xd8\xa8\xd9\x8e\xd8\xa8";
    xge_text_shape_desc_t desc={0};xge_glyph_run_t run={0};hb_buffer_t* buffer=hb_buffer_create();
    hb_glyph_info_t* infos;hb_glyph_position_t* positions;unsigned i,j,count,found=0;
    desc.iSize=sizeof(desc);desc.pFont=font;desc.sText=text+18;desc.iTextSize=4;
    desc.sContext=text;desc.iContextSize=24;desc.iContextOffset=18;desc.iScript=HB_SCRIPT_ARABIC;
    desc.sLanguage="ar";desc.iFlags=XGE_TEXT_SHAPE_DEFAULT|XGE_TEXT_SHAPE_RTL;
    hb_buffer_set_direction(buffer,HB_DIRECTION_RTL);hb_buffer_set_script(buffer,HB_SCRIPT_ARABIC);
    hb_buffer_set_language(buffer,hb_language_from_string("ar",2));
    hb_buffer_set_cluster_level(buffer,HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
    hb_buffer_set_flags(buffer,HB_BUFFER_FLAG_BOT|HB_BUFFER_FLAG_EOT);
    hb_buffer_add_utf8(buffer,text,24,0,24);hb_shape(oracle,buffer,NULL,0);
    CHECK(hb_buffer_allocation_successful(buffer) && xgeTextShape(&desc,&run)==XGE_OK && run.iGlyphCount==2);
    infos=hb_buffer_get_glyph_infos(buffer,&count);positions=hb_buffer_get_glyph_positions(buffer,NULL);
    for(i=0;i<2;i++){
        CHECK(run.pGlyphs[i].iGlyph==8 || run.pGlyphs[i].iGlyph==13);
        CHECK(!run.pGlyphs[i].iCluster && run.pGlyphs[i].iClusterEnd==4);
        for(j=0;j<count;j++)if(infos[j].cluster==18 && infos[j].codepoint==(unsigned)run.pGlyphs[i].iGlyph){
            CHECK(fabsf(run.pGlyphs[i].fAdvanceX-positions[j].x_advance*font->fScale)<.001f);
            CHECK(fabsf(run.pGlyphs[i].fOffsetX-positions[j].x_offset*font->fScale)<.001f);
            CHECK(fabsf(run.pGlyphs[i].fOffsetY+positions[j].y_offset*font->fScale)<.001f);found++;
        }
    }
    CHECK(found==2);xgeGlyphRunFree(&run);hb_buffer_destroy(buffer);cases++;
}
static void check_item(xge_font root, xge_font font, hb_font_t* oracle, const char* context, int bytes,
    int offset, unsigned script, unsigned flags, int expected)
{
    xge_text_shape_desc_t desc={0}; xge_glyph_run_t run={0}; char item[2];
    hb_buffer_t* buffer=hb_buffer_create(); hb_glyph_info_t* infos;
    hb_glyph_position_t* positions; unsigned count,i,found=0;
    memcpy(item,context+offset,2); /* An exact allocation with no NUL. */
    desc.iSize=sizeof(desc);desc.pFont=root;desc.sText=item;desc.iTextSize=2;
    desc.sContext=context;desc.iContextSize=bytes;desc.iContextOffset=offset;
    desc.iFlags=flags;desc.iScript=script;desc.sLanguage="ar";
    hb_buffer_set_direction(buffer,flags & XGE_TEXT_SHAPE_RTL ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
    hb_buffer_set_script(buffer,HB_SCRIPT_ARABIC);hb_buffer_set_language(buffer,hb_language_from_string("ar",2));
    hb_buffer_set_cluster_level(buffer,HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
    hb_buffer_set_flags(buffer,HB_BUFFER_FLAG_BOT|HB_BUFFER_FLAG_EOT);
    hb_buffer_add_utf8(buffer,context,bytes,0,bytes);hb_shape(oracle,buffer,NULL,0);
    CHECK(hb_buffer_allocation_successful(buffer));
    infos=hb_buffer_get_glyph_infos(buffer,&count);positions=hb_buffer_get_glyph_positions(buffer,NULL);
    CHECK(xgeTextShape(&desc,&run)==XGE_OK && run.iGlyphCount==1);
    for(i=0;i<count;i++)if(infos[i].cluster==(unsigned)offset && infos[i].codepoint==(unsigned)expected){
        if(run.pGlyphs[0].iGlyph!=expected)fprintf(stderr,"context bytes=%d offset=%d expected=%d native=%d\n",bytes,offset,expected,run.pGlyphs[0].iGlyph);
        found++;CHECK(run.pGlyphs[0].iGlyph==expected && run.pGlyphs[0].pFont==font);
        CHECK(fabsf(run.pGlyphs[0].fAdvanceX-positions[i].x_advance*font->fScale)<.001f);
        CHECK(fabsf(run.pGlyphs[0].fOffsetX-positions[i].x_offset*font->fScale)<.001f);
        CHECK(fabsf(run.pGlyphs[0].fOffsetY+positions[i].y_offset*font->fScale)<.001f);
    }
    if(!found || run.pGlyphs[0].iGlyph!=expected){
        fprintf(stderr,"context bytes=%d offset=%d expected=%d native=%d full matches=%u\n",bytes,offset,expected,run.pGlyphs[0].iGlyph,found);
        for(i=0;i<count;i++)fprintf(stderr,"whole glyph=%u cluster=%u\n",infos[i].codepoint,infos[i].cluster);
    }
    CHECK(found==1 && run.pGlyphs[0].iCluster==0 && run.pGlyphs[0].iClusterEnd==2);
    xgeGlyphRunFree(&run);hb_buffer_destroy(buffer);cases++;
}
int main(void)
{
    const unsigned lengths[]={0,4,5,6,8,32,512};
    const char* marks[]={"\xd9\x8e","\xcc\x81","\xe2\x80\x8e","\xf3\xa0\x84\x80"};
    const char* barriers[]={""," ","\xe2\x80\x8c","\xe2\x80\x8d"};
    const float sizes[]={40,37};unsigned size,mark,length,side,barrier,explicit_script;
    hb_blob_t* blob=hb_blob_create_from_file_or_fail("test/data/xge_context_fixture.ttf");
    hb_face_t* face;hb_font_t* oracle;CHECK(blob);
    face=hb_face_create(blob,0);oracle=hb_font_create(face);hb_ot_font_set_funcs(oracle);hb_font_set_scale(oracle,1000,1000);
    for(size=0;size<2;size++){
        xge_font_t font={0},probe={0};CHECK(xgeFontLoad(&font,"test/data/xge_context_fixture.ttf",sizes[size])==XGE_OK);
        CHECK(xgeFontLoad(&probe,"test/data/xge_context_medi_probe.ttf",sizes[size])==XGE_OK);xgeFontSetFallback(&probe,&font);
        for(mark=0;mark<4;mark++)for(length=0;length<7;length++)for(side=0;side<3;side++)for(barrier=0;barrier<4;barrier++){
            char context[4200];int bytes=0,offset,repeat,n=(int)lengths[length];int m=(int)strlen(marks[mark]);
            int expected=barrier==1 || barrier==2 ? (side==1 ? 9 : 7) : 8;
            memcpy(context+bytes,"\xd8\xa8",2);bytes+=2;
            if(side!=1){memcpy(context+bytes,barriers[barrier],strlen(barriers[barrier]));bytes+=(int)strlen(barriers[barrier]);
                for(repeat=0;repeat<n;repeat++){memcpy(context+bytes,marks[mark],(size_t)m);bytes+=m;}}
            offset=bytes;memcpy(context+bytes,"\xd8\xa8",2);bytes+=2;
            if(side!=0){for(repeat=0;repeat<n;repeat++){memcpy(context+bytes,marks[mark],(size_t)m);bytes+=m;}
                memcpy(context+bytes,barriers[barrier],strlen(barriers[barrier]));bytes+=(int)strlen(barriers[barrier]);}
            memcpy(context+bytes,"\xd8\xa8",2);bytes+=2;
            /* With two barriers the isolated form is intentional. */
            if(side==2 && (barrier==1 || barrier==2))expected=6;
            /* Arabic's resolved direction is RTL. Explicit LTR has a
             * different HB whole-run reversal policy, covered by the
             * separate bounded-item oracle rather than this property. */
            for(explicit_script=0;explicit_script<2;explicit_script++)
            {
                check_item(&font,&font,oracle,context,bytes,offset,explicit_script?HB_SCRIPT_ARABIC:0,
                    XGE_TEXT_SHAPE_DEFAULT|XGE_TEXT_SHAPE_RTL,expected);
                /* The medial-only probe cannot cover the word's init/fina
                 * edges. The complete font wins unless nonzero acute marks
                 * make that entire word unrenderable in both fonts. In that
                 * case local grapheme fallback still covers this medial item.
                 * Neither fixture has U+0301; Fatha is mapped, LRM/VS ignored. */
                check_item(&probe,expected==8 && mark==1 && n>0?&probe:&font,oracle,context,bytes,offset,explicit_script?HB_SCRIPT_ARABIC:0,
                    XGE_TEXT_SHAPE_DEFAULT|XGE_TEXT_SHAPE_RTL,expected);
            }
        }
        marked_item(&font,oracle);
        xgeFontFree(&probe);xgeFontFree(&font);CHECK(!probe.iRefCount && !font.iRefCount);
    }
    hb_font_destroy(oracle);hb_face_destroy(face);hb_blob_destroy(blob);
    printf("Native joining context: %u whole-paragraph and literal glyph/GPOS comparisons; 0/4/5/6/8/32/512 transparent scalars, both sides, ZWNJ/ZWJ/space barriers, 40/37 RTL and automatic/explicit script passed\n",cases);
    return 0;
}
