#include "../test/xge_opentype_fixture_path.h"
/* Exercise the actual C caret mapper with real HB glyphs/font tables. The
 * small owner/allocator shim excludes native font rasterization and GPU. */
#include "../xge.h"
#include "../lib/harfbuzz/src/hb.h"
#include "../lib/harfbuzz/src/hb-ot.h"
#include "../lib/libunibreak/src/graphemebreak.h"
#include "../lib/libunibreak/src/unibreakdef.h"
#include "../src/xge_unicode_grapheme.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define CHECK(e) do {if(!(e)){fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e);exit(1);}}while(0)
typedef struct allocation_owner {void* data;size_t bytes;} allocation_owner;
static allocation_owner allocations[8];
static size_t allocation_calls,fail_at,live_owners,live_bytes;
static allocation_owner* find_owner(void* data)
{size_t i;for(i=0;i<8;i++)if(allocations[i].data==data)return &allocations[i];CHECK(0);return NULL;}
static void* caret_malloc(size_t bytes)
{
    void* data;allocation_owner* owner;if(++allocation_calls==fail_at)return NULL;
    data=malloc(bytes);if(!data)return NULL;owner=find_owner(NULL);owner->data=data;owner->bytes=bytes;
    live_owners++;live_bytes+=bytes;return data;
}
static void caret_free(void* data)
{
    if(data){allocation_owner* owner=find_owner(data);CHECK(live_owners && live_bytes>=owner->bytes);
        live_owners--;live_bytes-=owner->bytes;owner->data=NULL;owner->bytes=0;free(data);}
}
static void* caret_realloc(void* data,size_t bytes)
{
    void* p;allocation_owner* owner;if(!data)return caret_malloc(bytes);
    if(++allocation_calls==fail_at)return NULL;
    owner=find_owner(data);p=realloc(data,bytes);
    if(!p)return NULL;
    live_bytes=live_bytes-owner->bytes+bytes;owner->data=p;owner->bytes=bytes;return p;
}
#define xrtMalloc caret_malloc
#define xrtRealloc caret_realloc
#define xrtFree caret_free
#include "../src/xge_font_contours.inl"
typedef struct xge_glyph_run_backend_t {int iCaretCapacity;char* pGraphemeBreaks;} xge_glyph_run_backend_t;
static hb_font_t* fixture_font;
static xge_tt_source_t contour_source;
static hb_bool_t contour(hb_font_t* font,void* font_data,hb_codepoint_t glyph,unsigned index,
    hb_position_t* x,hb_position_t* y,void* user)
{
    xge_tt_point_t point;int sx,sy;unsigned upem=hb_face_get_upem(hb_font_get_face(font));(void)user;
    if(!__xgeTTPoint(font_data,glyph,index,&point))return 0;
    hb_font_get_scale(font,&sx,&sy);*x=(hb_position_t)floor(point.x*sx/upem+.5);*y=(hb_position_t)floor(point.y*sy/upem+.5);return 1;
}
static const unsigned char* sfnt_table(const unsigned char* data,size_t bytes,const char* tag,uint32_t* length)
{
    unsigned i,n;CHECK(bytes>=12);n=__xgeTTU16(data+4);CHECK(__xgeTTRange(12,(size_t)n*16,bytes));
    for(i=0;i<n;i++){const unsigned char* record=data+12+i*16;
        if(!memcmp(record,tag,4)){uint32_t at=__xgeTTU32(record+8);*length=__xgeTTU32(record+12);
            CHECK(__xgeTTRange(at,*length,bytes));return data+at;}}
    CHECK(0);return NULL;
}
static void contour_font(hb_face_t* face,const unsigned char* data,size_t bytes)
{
    uint32_t hl,ml,gl,ll;const unsigned char *h,*m,*g,*l;hb_font_funcs_t* funcs;hb_font_t* parent=fixture_font;
    h=sfnt_table(data,bytes,"head",&hl);m=sfnt_table(data,bytes,"maxp",&ml);
    g=sfnt_table(data,bytes,"glyf",&gl);l=sfnt_table(data,bytes,"loca",&ll);
    CHECK(__xgeTTSourceInit(&contour_source,h,hl,m,ml,g,gl,l,ll));
    hb_font_make_immutable(parent);fixture_font=hb_font_create_sub_font(parent);hb_font_destroy(parent);
    funcs=hb_font_funcs_create();hb_font_funcs_set_glyph_contour_point_func(funcs,contour,NULL,NULL);
    hb_font_set_funcs(fixture_font,funcs,&contour_source,NULL);hb_font_funcs_destroy(funcs);
    CHECK(hb_font_get_face(fixture_font)==face);
}
static hb_font_t* __xgeFontShapeFont(xge_font font){(void)font;return fixture_font;}
static void __xgeOtGraphemesUtf8(const utf8_t* text,size_t bytes,const char* language,char* breaks)
{(void)language;__xgeGraphemeMap(text,bytes,breaks,ub_get_next_char_utf8);}
#include "../src/xge_text_opentype_caret_validation.inl"
#include "../src/xge_text_opentype_carets.inl"
static void near(float actual,float expected)
{if(fabsf(actual-expected)>.001f)fprintf(stderr,"actual=%g expected=%g\n",actual,expected);CHECK(fabsf(actual-expected)<.001f);}
static hb_buffer_t* shaped(const char* text,unsigned start,unsigned flags)
{
    hb_buffer_t* b=hb_buffer_create();hb_feature_t kern={HB_TAG('k','e','r','n'),!!(flags&XGE_TEXT_SHAPE_KERNING),0,(unsigned)-1};
    hb_buffer_set_direction(b,flags&XGE_TEXT_SHAPE_RTL?HB_DIRECTION_RTL:HB_DIRECTION_LTR);
    hb_buffer_set_cluster_level(b,HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
    hb_buffer_add_utf8(b,text,(int)strlen(text),start,(int)strlen(text)-(int)start);
    hb_buffer_guess_segment_properties(b);hb_shape(fixture_font,b,&kern,1);CHECK(hb_buffer_allocation_successful(b));return b;
}
static void release_run(xge_glyph_run_t* run,xge_glyph_run_backend_t* owner)
{caret_free(run->pCarets);caret_free(owner->pGraphemeBreaks);CHECK(!live_owners && !live_bytes);}
static size_t exercise(const char* text,unsigned flags,const uint32_t* offsets,const float* stops,unsigned expected,
    int spacing_mark)
{
    xge_font_t font={0};xge_text_shape_desc_t desc={0};hb_buffer_t* buffer=shaped(text,0,flags);
    hb_glyph_info_t* infos;hb_glyph_position_t* positions;unsigned glyphs,j;size_t baseline=0,point;float advance=0;
    infos=hb_buffer_get_glyph_infos(buffer,&glyphs);positions=hb_buffer_get_glyph_positions(buffer,NULL);
    CHECK(glyphs>1);
    for(j=0;j<glyphs;j++) {
        CHECK(infos[j].cluster==0);
        if(spacing_mark && hb_ot_layout_get_glyph_class(hb_font_get_face(fixture_font),infos[j].codepoint)==HB_OT_LAYOUT_GLYPH_CLASS_MARK)
            positions[j].x_advance=100;
        advance+=positions[j].x_advance;
    }
    font.fScale=.04f;desc.iFlags=flags;desc.sText=text;desc.iTextSize=(int)strlen(text);
    for(point=0;point<=baseline;point++) {
        xge_glyph_run_t run={0};xge_glyph_run_backend_t owner={0};int result;
        run.iTextSize=desc.iTextSize;run.pBackend=&owner;allocation_calls=0;fail_at=point;
        result=__xgeOtCarets(&desc,&run,&font,0,(uint32_t)strlen(text),infos,positions,glyphs,advance*font.fScale);fail_at=0;
        if(!point) {
            CHECK(result==XGE_OK && run.iCaretCount==(int)expected && (!expected || run.pCarets));baseline=allocation_calls;
            for(j=0;j<expected;j++){CHECK(run.pCarets[j].iTextOffset==offsets[j]);near(run.pCarets[j].fAdvance,stops[j]*font.fScale);}
        } else CHECK(result==XGE_ERROR_OUT_OF_MEMORY && !run.iCaretCount && !run.pCarets);
        release_run(&run,&owner);
    }
    hb_buffer_destroy(buffer);return baseline;
}
static void append_rollback(void)
{
    const char* text="fi ff\xcc\x81i";xge_font_t font={0};xge_text_shape_desc_t desc={0};hb_buffer_t* buffer;
    hb_glyph_info_t first={0},*infos;hb_glyph_position_t position={0},*positions;unsigned glyphs;size_t point;
    font.fScale=.04f;desc.sText=text;desc.iTextSize=(int)strlen(text);desc.iFlags=XGE_TEXT_SHAPE_KERNING;
    first.codepoint=8;position.x_advance=700;buffer=shaped(text,3,desc.iFlags);
    infos=hb_buffer_get_glyph_infos(buffer,&glyphs);positions=hb_buffer_get_glyph_positions(buffer,NULL);CHECK(glyphs==2);
    for(point=1;point<=2;point++) {
        xge_glyph_run_t run={0};xge_glyph_run_backend_t owner={0};xge_glyph_caret_t previous;
        size_t bytes,owners;run.iTextSize=desc.iTextSize;run.pBackend=&owner;
        CHECK(__xgeOtCarets(&desc,&run,&font,0,2,&first,&position,1,700*font.fScale)==XGE_OK && run.iCaretCount==1 && run.pCarets);
        previous=run.pCarets[0];bytes=live_bytes;owners=live_owners;allocation_calls=0;fail_at=point;
        CHECK(__xgeOtCarets(&desc,&run,&font,3,8,infos,positions,glyphs,900*font.fScale)==XGE_ERROR_OUT_OF_MEMORY);fail_at=0;
        CHECK(run.iCaretCount==1 && run.pCarets[0].iTextOffset==previous.iTextOffset && run.pCarets[0].fAdvance==previous.fAdvance &&
            live_bytes==bytes && live_owners==owners);
        CHECK(__xgeOtCarets(&desc,&run,&font,3,8,infos,positions,glyphs,900*font.fScale)==XGE_OK && run.iCaretCount==3);
        near(run.pCarets[1].fAdvance,320*font.fScale);near(run.pCarets[2].fAdvance,870*font.fScale);release_run(&run,&owner);
    }
    hb_buffer_destroy(buffer);
}
int main(void)
{
    FILE* file=fopen(XGE_TEST_OPENTYPE_FIXTURE,"rb");long bytes;char* data;hb_blob_t* blob;hb_face_t* face;size_t failed=0;
    const uint32_t three[]={1,4},three_marks[]={1,6},two[]={3},hebrew[]={4,6},multiple[]={1};
    const float placed[]={320,870},unplaced[]={250,800},fi[]={120},rtl[]={130,680},fx[]={450},ix[]={400},fa[]={375},multi[]={650},spacing[]={1000.f/3,2000.f/3};
    CHECK(file && !fseek(file,0,SEEK_END) && (bytes=ftell(file))>0 && !fseek(file,0,SEEK_SET));
    data=malloc((size_t)bytes);CHECK(data && fread(data,1,(size_t)bytes,file)==(size_t)bytes);fclose(file);
    blob=hb_blob_create(data,(unsigned)bytes,HB_MEMORY_MODE_READONLY,NULL,NULL);face=hb_face_create(blob,0);fixture_font=hb_font_create(face);
    hb_ot_font_set_funcs(fixture_font);hb_font_set_scale(fixture_font,1000,1000);
    /* Both branches are compiled so the actual bounded parser remains covered
     * by warnings/analyzers in the ordinary Format 1 build as well. */
    if(strstr(XGE_TEST_OPENTYPE_FIXTURE,"points"))contour_font(face,(const unsigned char*)data,(size_t)bytes);
    failed+=exercise("ff\xcc\x81i",XGE_TEXT_SHAPE_KERNING,three,placed,2,0);
    failed+=exercise("ff\xcc\x81i",0,three,unplaced,2,0);
    failed+=exercise("ff\xcc\x81\xcc\x81i",XGE_TEXT_SHAPE_KERNING,three_marks,placed,2,0);
    failed+=exercise("f\xcc\x81i",XGE_TEXT_SHAPE_KERNING,two,fi,1,0);
    failed+=exercise("\xd7\x90\xd6\xb0\xd7\x91\xd7\x92",XGE_TEXT_SHAPE_DEFAULT|XGE_TEXT_SHAPE_RTL,hebrew,rtl,2,0);
    failed+=exercise("f\xcc\x81x",XGE_TEXT_SHAPE_KERNING,two,fx,1,0);
    failed+=exercise("i\xcc\x81x",XGE_TEXT_SHAPE_KERNING,two,ix,1,0);
    failed+=exercise("f\xcc\x81" "a",XGE_TEXT_SHAPE_KERNING,two,fa,1,0);
    failed+=exercise("fy",XGE_TEXT_SHAPE_KERNING,multiple,multi,1,0);
    failed+=exercise("ff\xcc\x81i",XGE_TEXT_SHAPE_KERNING,three,spacing,2,1);
    append_rollback();hb_font_destroy(fixture_font);hb_face_destroy(face);hb_blob_destroy(blob);free(data);
    printf("Portable actual C GDEF caret mapper: LTR/RTL ligature+marks, GPOS placement, multiple marks, malformed/ambiguous/spacing fallback, %zu allocation failures and 2 append rollbacks passed; no native GPU/IME claim\n",failed);
    return 0;
}
