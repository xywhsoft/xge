/* Real C buffer integration with a portable, bounded UTF-8 decoder shim. */
#include "../xge.h"
#include "../lib/harfbuzz/src/hb.h"
#include "../lib/harfbuzz/src/hb-ot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define CHECK(e) do{if(!(e)){fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e);exit(1);}}while(0)
static int __xgeTextUTF8DecodeBounded(const char** at,const char* end,uint32_t* cp)
{
    const unsigned char* text=(const unsigned char*)*at;unsigned value,n,i;
    if(*at>=end)return XGE_ERROR_INVALID_ARGUMENT;
    if(text[0]<0x80){*cp=text[0];(*at)++;return XGE_OK;}
    if(text[0]>=0xc2 && text[0]<=0xdf){n=2;value=text[0]&31;}
    else if(text[0]>=0xe0 && text[0]<=0xef){n=3;value=text[0]&15;}
    else if(text[0]>=0xf0 && text[0]<=0xf4){n=4;value=text[0]&7;}
    else return XGE_ERROR_INVALID_ARGUMENT;
    if((size_t)(end-*at)<n)return XGE_ERROR_INVALID_ARGUMENT;
    for(i=1;i<n;i++){if((text[i]&0xc0)!=0x80)return XGE_ERROR_INVALID_ARGUMENT;value=(value<<6)|(text[i]&63);}
    if((n==3 && value<0x800) || (n==4 && value<0x10000) || value>0x10ffff ||
        (value>=0xd800 && value<=0xdfff))return XGE_ERROR_INVALID_ARGUMENT;
    *cp=value;*at+=n;return XGE_OK;
}
static size_t visits;
#define XGE_OT_CONTEXT_VISIT() (visits++)
#include "../src/xge_text_opentype_context.inl"
#undef XGE_OT_CONTEXT_VISIT
static void properties(hb_buffer_t* buffer)
{
    hb_buffer_set_direction(buffer,HB_DIRECTION_RTL);hb_buffer_set_script(buffer,HB_SCRIPT_ARABIC);
    hb_buffer_set_language(buffer,hb_language_from_string("ar",2));
    hb_buffer_set_cluster_level(buffer,HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
}
static void large(hb_font_t* font,int neighbors)
{
    const int marks=100000;int bytes=marks*4+2+neighbors*4,offset=marks*2+neighbors*2,i,at=0;
    char* context=malloc((size_t)bytes);char item[2];xge_text_shape_desc_t desc={0};
    hb_buffer_t* buffer=hb_buffer_create();hb_glyph_info_t* infos;unsigned count;
    CHECK(context);if(neighbors){memcpy(context+at,"\xd8\xa8",2);at+=2;}
    for(i=0;i<marks;i++){memcpy(context+at,"\xd9\x8e",2);at+=2;}
    memcpy(context+at,"\xd8\xa8",2);at+=2;
    for(i=0;i<marks;i++){memcpy(context+at,"\xd9\x8e",2);at+=2;}
    if(neighbors){memcpy(context+at,"\xd8\xa8",2);at+=2;}CHECK(at==bytes);
    memcpy(item,context+offset,2);desc.sText=item;desc.iTextSize=2;desc.sContext=context;
    desc.iContextSize=bytes;desc.iContextOffset=offset;properties(buffer);visits=0;
    CHECK(__xgeOtBufferAddContext(buffer,&desc,item,item+2)==XGE_OK);
    CHECK(visits==(size_t)marks*2+(unsigned)neighbors*2);
    /* Neither borrowed buffer is retained by HB after the synchronous add. */
    memset(context,0,(size_t)bytes);free(context);memset(item,0,2);
    hb_shape(font,buffer,NULL,0);CHECK(hb_buffer_allocation_successful(buffer));
    infos=hb_buffer_get_glyph_infos(buffer,&count);CHECK(count==1 && infos[0].codepoint==(unsigned)(neighbors?8:6) && infos[0].cluster==(unsigned)offset);
    printf("Joining context scale: %d exact non-NUL bytes, %zu neighbor scalar visits, literal %s glyph; borrowed buffers released before shape\n",bytes,visits,neighbors?"medial":"isolated");
    hb_buffer_destroy(buffer);
}
int main(void)
{
    hb_blob_t* blob=hb_blob_create_from_file_or_fail("test/data/xge_context_fixture.ttf");hb_face_t* face;hb_font_t* font;
    hb_buffer_t* buffer; xge_text_shape_desc_t desc={0};char item[2]={'i','i'};unsigned count;
    CHECK(blob);face=hb_face_create(blob,0);font=hb_font_create(face);hb_ot_font_set_funcs(font);hb_font_set_scale(font,1000,1000);
    large(font,1);large(font,0);
    buffer=hb_buffer_create();desc.sText=item;desc.iTextSize=2;desc.sContext="xii";desc.iContextSize=3;desc.iContextOffset=1;
    properties(buffer);visits=0;CHECK(__xgeOtBufferAddContext(buffer,&desc,item,item+2)==XGE_OK && visits==1);
    CHECK(hb_buffer_get_glyph_infos(buffer,&count) && count==2);
    hb_buffer_clear_contents(buffer);desc.sContext="\x80ii";
    CHECK(__xgeOtBufferAddContext(buffer,&desc,item,item+2)==XGE_ERROR_INVALID_ARGUMENT && !hb_buffer_get_length(buffer));
    hb_buffer_clear_contents(buffer);desc.sContext="xii";
    CHECK(!hb_buffer_set_length(buffer,UINT_MAX));
    CHECK(__xgeOtBufferAddContext(buffer,&desc,item,item+2)==XGE_ERROR_OUT_OF_MEMORY);
    hb_buffer_destroy(buffer);hb_font_destroy(font);hb_face_destroy(face);hb_blob_destroy(blob);
    puts("Portable C joining context: bounded buffers, linear neighbor-only reads, absent effective neighbors, malformed nearby UTF-8 and failed HB allocation passed");return 0;
}
