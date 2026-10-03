#include "../xge.h"
#include "../lib/harfbuzz/src/hb.h"
#include "../lib/harfbuzz/src/hb-ot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s (case %u)\n",__FILE__,__LINE__,#e,cases); exit(1); } } while(0)
static unsigned cases;
typedef struct oracle_t { hb_blob_t* blob; hb_face_t* face; hb_font_t* font; } oracle_t;
static void load_oracle(oracle_t* oracle, const char* path)
{
    oracle->blob=hb_blob_create_from_file_or_fail(path);CHECK(oracle->blob);
    oracle->face=hb_face_create(oracle->blob,0);oracle->font=hb_font_create(oracle->face);
    hb_ot_font_set_funcs(oracle->font);hb_font_set_scale(oracle->font,1000,1000);
}
static void free_oracle(oracle_t* oracle)
{ hb_font_destroy(oracle->font);hb_face_destroy(oracle->face);hb_blob_destroy(oracle->blob); }
static void shape_item(xge_font root, xge_font expected_font, oracle_t* oracle, const char* context,
    int context_bytes, int offset, int bytes, uint32_t script, hb_script_t known_script,
    const char* language, unsigned flags, int known_first_glyph)
{
    xge_text_shape_desc_t desc={0};xge_glyph_run_t run={0};hb_buffer_t* buffer=hb_buffer_create();
    hb_glyph_info_t* infos;hb_glyph_position_t* positions;unsigned count,i,order[32];
    hb_feature_t kern={HB_TAG('k','e','r','n'),!!(flags & XGE_TEXT_SHAPE_KERNING),0,(unsigned)-1};
    char item[64];CHECK(bytes<(int)sizeof(item));memcpy(item,context+offset,(size_t)bytes);
    /* Item copy intentionally has no terminating NUL: bounds are authoritative. */
    desc.iSize=sizeof(desc);desc.pFont=root;desc.sText=item;desc.iTextSize=bytes;desc.iFlags=flags;
    desc.sContext=context;desc.iContextSize=context_bytes;desc.iContextOffset=offset;desc.iScript=script;desc.sLanguage=language;
    CHECK(xgeTextShape(&desc,&run)==XGE_OK && run.iTextSize==bytes);
    /* Independent upstream HB oracle: known script and exact full context,
     * without XGE's Script_Extensions or fallback/property helper. */
    hb_buffer_set_direction(buffer,flags & XGE_TEXT_SHAPE_RTL ? HB_DIRECTION_RTL:HB_DIRECTION_LTR);
    hb_buffer_set_script(buffer,known_script);
    hb_buffer_set_language(buffer,hb_language_from_string(language?language:"und",-1));
    hb_buffer_set_cluster_level(buffer,HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
    hb_buffer_set_flags(buffer,(hb_buffer_flags_t)((offset==0?HB_BUFFER_FLAG_BOT:0)|
        (offset+bytes==context_bytes?HB_BUFFER_FLAG_EOT:0)));
    hb_buffer_add_utf8(buffer,context,context_bytes,(unsigned)offset,bytes);
    hb_shape(oracle->font,buffer,&kern,1);CHECK(hb_buffer_allocation_successful(buffer));
    infos=hb_buffer_get_glyph_infos(buffer,&count);positions=hb_buffer_get_glyph_positions(buffer,NULL);
    CHECK(count<32 && run.iGlyphCount==(int)count && count);
    /* Stable ascending-cluster order preserves HB's mark order within a cluster. */
    for(i=0;i<count;i++){unsigned j=i;order[i]=i;while(j && infos[order[j-1]].cluster>infos[order[j]].cluster){
        unsigned temp=order[j];order[j]=order[j-1];order[j-1]=temp;j--;}}
    if(known_first_glyph>=0) {
        if(infos[order[0]].codepoint!=(unsigned)known_first_glyph) {
            fprintf(stderr,"literal HB oracle: script=%08x flags=%u context=%.*s offset=%d bytes=%d got=%u expected=%d\n",
                known_script,flags,context_bytes,context,offset,bytes,infos[order[0]].codepoint,known_first_glyph);
            for(i=0;i<count;i++)fprintf(stderr,"HB glyph=%u cluster=%u native=%d/%u\n",infos[i].codepoint,infos[i].cluster,run.pGlyphs[i].iGlyph,run.pGlyphs[i].iCluster);
        }
        CHECK(infos[order[0]].codepoint==(unsigned)known_first_glyph);
    }
    if(bytes==4 && !memcmp(item,"\xd8\xa8\xd9\x8e",4)) {
        CHECK(count==2 && ((infos[0].codepoint==8 && infos[1].codepoint==13) ||
            (infos[0].codepoint==13 && infos[1].codepoint==8)));
    }
    for(i=0;i<count;i++){
        unsigned j=order[i];xge_glyph_position_t* glyph=&run.pGlyphs[i];
        if(glyph->iGlyph!=(int)infos[j].codepoint)fprintf(stderr,"context=%.*s offset=%d bytes=%d glyph=%d expected=%u\n",context_bytes,context,offset,bytes,glyph->iGlyph,infos[j].codepoint);
        CHECK(glyph->iGlyph==(int)infos[j].codepoint && glyph->pFont==expected_font);
        CHECK(glyph->iCluster==infos[j].cluster-(unsigned)offset && glyph->iCluster<glyph->iClusterEnd && glyph->iClusterEnd<=(unsigned)bytes);
        CHECK(fabsf(glyph->fAdvanceX-positions[j].x_advance*expected_font->fScale)<.001f);
        CHECK(fabsf(glyph->fOffsetX-positions[j].x_offset*expected_font->fScale)<.001f);
        CHECK(fabsf(glyph->fOffsetY+positions[j].y_offset*expected_font->fScale)<.001f);
    }
    for(i=0;i<(unsigned)run.iCaretCount;i++) CHECK(run.pCarets[i].iTextOffset<(unsigned)bytes);
    for(i=0;i<7;i++) {uint32_t cluster;int trailing;
        CHECK(xgeGlyphRunHitTest(&run,run.fWidth*i/6,0,&cluster,&trailing)==XGE_OK && cluster<=(unsigned)bytes);}
    xgeGlyphRunFree(&run);CHECK(!run.pGlyphs && !run.pBackend && !run.pCarets);
    hb_buffer_destroy(buffer);cases++;
}
static void invalid_arguments(xge_font font)
{
    xge_text_shape_desc_t desc={0};xge_glyph_run_t run={0};char bounded='i';
    desc.iSize=sizeof(desc);desc.pFont=font;desc.sText=&bounded;desc.iTextSize=0;
    CHECK(xgeTextShape(&desc,&run)==XGE_OK && !run.iGlyphCount && !run.iTextSize && !run.pBackend);xgeGlyphRunFree(&run);
    desc.sText="i";desc.iTextSize=-1;CHECK(xgeTextShape(&desc,&run)==XGE_OK && run.iGlyphCount==1);xgeGlyphRunFree(&run);
    desc.iTextSize=1;desc.sContext="xi";desc.iContextSize=-1;desc.iContextOffset=1;
    CHECK(xgeTextShape(&desc,&run)==XGE_OK && run.iGlyphCount==1 && !run.pGlyphs[0].iCluster);xgeGlyphRunFree(&run);
    desc.iContextOffset=INT_MAX;CHECK(xgeTextShape(&desc,&run)==XGE_ERROR_INVALID_ARGUMENT && !run.pBackend);
    desc.iContextOffset=-1;CHECK(xgeTextShape(&desc,&run)==XGE_ERROR_INVALID_ARGUMENT);
    desc.iContextOffset=0;CHECK(xgeTextShape(&desc,&run)==XGE_ERROR_INVALID_ARGUMENT);
    desc.iContextOffset=1;desc.sLanguage="en_US";CHECK(xgeTextShape(&desc,&run)==XGE_ERROR_INVALID_ARGUMENT);
    desc.sLanguage=NULL;desc.iScript=HB_TAG('L','a','t','1');CHECK(xgeTextShape(&desc,&run)==XGE_ERROR_INVALID_ARGUMENT);
    desc.iScript=0;desc.iTextSize=-2;CHECK(xgeTextShape(&desc,&run)==XGE_ERROR_INVALID_ARGUMENT);
    desc.iTextSize=1;desc.iContextSize=-2;CHECK(xgeTextShape(&desc,&run)==XGE_ERROR_INVALID_ARGUMENT);
    desc.sContext="\xd8\xa8";desc.iContextSize=2;desc.iContextOffset=1;desc.sText="\xa8";
    CHECK(xgeTextShape(&desc,&run)==XGE_ERROR_INVALID_ARGUMENT);
    desc.iContextOffset=0;desc.sText="\xd8";CHECK(xgeTextShape(&desc,&run)==XGE_ERROR_INVALID_ARGUMENT);
    desc.sContext=NULL;desc.iContextSize=0;desc.sText="\xc0\x80";desc.iTextSize=2;
    CHECK(xgeTextShape(&desc,&run)!=XGE_OK && !run.pBackend && !run.pGlyphs);
}
static void gdef_context(float size)
{
    xge_font_t font={0};xge_text_shape_desc_t desc={0};xge_glyph_run_t run={0};
    CHECK(xgeFontLoad(&font,"test/data/xge_opentype_fixture.ttf",size)==XGE_OK);
    desc.iSize=sizeof(desc);desc.pFont=&font;desc.sText="ffi";desc.iTextSize=3;
    desc.sContext="\xce\xbbxffiy";desc.iContextSize=7;desc.iContextOffset=3;
    desc.iScript=HB_SCRIPT_LATIN;desc.sLanguage="und";desc.iFlags=XGE_TEXT_SHAPE_KERNING;
    CHECK(xgeTextShape(&desc,&run)==XGE_OK && run.iGlyphCount==1 && run.pGlyphs[0].iGlyph==9 &&
        run.pGlyphs[0].iCluster==0 && run.pGlyphs[0].iClusterEnd==3 && run.iCaretCount==2);
    CHECK(run.pCarets[0].iTextOffset==1 && run.pCarets[1].iTextOffset==2);
    CHECK(fabsf(run.pCarets[0].fAdvance-250*font.fScale)<.001f && fabsf(run.pCarets[1].fAdvance-800*font.fScale)<.001f);
    xgeGlyphRunFree(&run);xgeFontFree(&font);CHECK(!font.iRefCount);cases++;
}
static void context_lifetime(float size)
{
    xge_font_t font={0};xge_text_shape_desc_t desc={0};xge_glyph_run_t run={0};xge_vec2_t measured;
    char* context=malloc(6);char* item=malloc(2);CHECK(context && item);
    memcpy(context,"\xd8\xa8\xd8\xa8\xd8\xa8",6);memcpy(item,context+2,2);
    CHECK(xgeFontLoad(&font,"test/data/xge_context_fixture.ttf",size)==XGE_OK);
    desc.iSize=sizeof(desc);desc.pFont=&font;desc.sText=item;desc.iTextSize=2;
    desc.sContext=context;desc.iContextSize=6;desc.iContextOffset=2;desc.iFlags=XGE_TEXT_SHAPE_RTL;
    CHECK(xgeTextShape(&desc,&run)==XGE_OK && run.iGlyphCount==1 && run.pGlyphs[0].iGlyph==8);
    memset(context,0,6);memset(item,0,2);free(context);free(item);xgeFontFree(&font);
    measured=xgeGlyphRunMeasure(&run);CHECK(measured.fX>0 && isfinite(measured.fX) && font.iRefCount>0);
    CHECK(run.pGlyphs[0].iGlyph==8 && !run.pGlyphs[0].iCluster && run.pGlyphs[0].iClusterEnd==2);
    xgeGlyphRunFree(&run);CHECK(!font.iRefCount);cases++;
}
int main(void)
{
    const char* full_path="test/data/xge_context_fixture.ttf";
    const char* probe_path="test/data/xge_context_probe.ttf";
    const char* arabic="\xd8\xa8\xd8\xa8\xd8\xa8";
    const char* marked="\xd8\xa8\xd8\xa8\xd9\x8e\xd8\xa8";
    const char* syriac="\xdc\x90\xd9\x80\xdc\x90";
    const float sizes[]={40,37};oracle_t full_oracle,probe_oracle;unsigned size,direction,explicit_script;
    load_oracle(&full_oracle,full_path);load_oracle(&probe_oracle,probe_path);
    for(size=0;size<2;size++){
        xge_font_t full={0},probe={0};CHECK(xgeFontLoad(&full,full_path,sizes[size])==XGE_OK);
        CHECK(xgeFontLoad(&probe,probe_path,sizes[size])==XGE_OK);xgeFontSetFallback(&probe,&full);
        CHECK(fabsf(full.fScale-sizes[size]/1000)<.00001f);
        for(direction=0;direction<2;direction++)for(explicit_script=0;explicit_script<2;explicit_script++){
            unsigned flags=XGE_TEXT_SHAPE_KERNING | (direction?XGE_TEXT_SHAPE_RTL:0);
            uint32_t script=explicit_script?HB_SCRIPT_ARABIC:0;
            shape_item(&full,&full,&full_oracle,arabic,6,0,2,script,HB_SCRIPT_ARABIC,"ar",flags,7);
            shape_item(&full,&full,&full_oracle,arabic,6,2,2,script,HB_SCRIPT_ARABIC,"ar",flags,8);
            shape_item(&full,&full,&full_oracle,arabic,6,4,2,script,HB_SCRIPT_ARABIC,"ar",flags,9);
            /* Literal multi-letter Arabic forms require its resolved RTL
             * direction. Explicit LTR still must match upstream HB exactly. */
            shape_item(&full,&full,&full_oracle,arabic,6,0,4,script,HB_SCRIPT_ARABIC,"ar",flags,direction?7:-1);
            shape_item(&full,&full,&full_oracle,arabic,6,2,4,script,HB_SCRIPT_ARABIC,"ar",flags,direction?8:-1);
            shape_item(&full,&full,&full_oracle,arabic,6,0,6,script,HB_SCRIPT_ARABIC,"ar",flags,direction?7:-1);
            shape_item(&full,&full,&full_oracle,marked,8,2,4,script,HB_SCRIPT_ARABIC,"ar",flags,-1);
            shape_item(&probe,&probe,&probe_oracle,arabic,6,2,2,script,HB_SCRIPT_ARABIC,"ar",flags,8);
            shape_item(&probe,&full,&full_oracle,arabic,2,0,2,script,HB_SCRIPT_ARABIC,"ar",flags,6);
            script=explicit_script?HB_SCRIPT_SYRIAC:0;
            shape_item(&full,&full,&full_oracle,syriac,6,2,2,script,HB_SCRIPT_SYRIAC,"syr",flags,11);
            script=explicit_script?HB_SCRIPT_LATIN:0;
            shape_item(&full,&full,&full_oracle,"iii",3,1,1,script,HB_SCRIPT_LATIN,NULL,flags,3);
            shape_item(&full,&full,&full_oracle,"iii",3,1,1,script,HB_SCRIPT_LATIN,"en-US",flags,4);
            shape_item(&full,&full,&full_oracle,"iii",3,1,1,script,HB_SCRIPT_LATIN,"TR-tr",flags,5);
            shape_item(&probe,&probe,&probe_oracle,"iii",3,1,1,script,HB_SCRIPT_LATIN,"tr",flags,5);
            shape_item(&probe,&full,&full_oracle,"iii",3,1,1,script,HB_SCRIPT_LATIN,"en",flags,4);
        }
        shape_item(&full,&full,&full_oracle,"iii",3,1,1,HB_TAG('l','a','t','n'),HB_SCRIPT_LATIN,"tr",XGE_TEXT_SHAPE_KERNING,5);
        /* Common/Inherited requests follow HB's documented font script-table
         * fallback; absence of a DFLT table can still select latn. */
        shape_item(&full,&full,&full_oracle,"iii",3,1,1,HB_SCRIPT_COMMON,HB_SCRIPT_COMMON,"tr",XGE_TEXT_SHAPE_KERNING,-1);
        shape_item(&full,&full,&full_oracle,"iii",3,1,1,HB_SCRIPT_INHERITED,HB_SCRIPT_INHERITED,"tr",XGE_TEXT_SHAPE_KERNING,-1);
        shape_item(&full,&full,&full_oracle,syriac,6,2,2,HB_SCRIPT_ARABIC,HB_SCRIPT_ARABIC,"ar",XGE_TEXT_SHAPE_KERNING,10);
        invalid_arguments(&full);xgeFontFree(&probe);xgeFontFree(&full);CHECK(!probe.iRefCount && !full.iRefCount);
        gdef_context(sizes[size]);
        context_lifetime(sizes[size]);
    }
    free_oracle(&full_oracle);free_oracle(&probe_oracle);
    printf("Native paragraph context: %u cases (128 independent HB glyph/GPOS/font/cluster/hit, 2 rebased GDEF, 2 borrowed-context/font lifetime); 40/37 LTR/RTL passed\n",cases);
    return 0;
}
