/* Portable dependency test: no XGE graphics/OS backend is linked here. */
#include "../lib/harfbuzz/src/hb.h"
#include "../lib/harfbuzz/src/hb-ot.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); exit(1); } } while(0)
int main(void)
{
    FILE* input=fopen("test/data/xge_opentype_fixture.ttf","rb"); long size; char* data;
    hb_blob_t* blob; hb_face_t* face; hb_font_t* font; hb_buffer_t* buffer;
    hb_glyph_info_t* info; hb_glyph_position_t* positions; unsigned count, carets; hb_position_t stops[2];
    CHECK(input && !fseek(input,0,SEEK_END) && (size=ftell(input))>0 && !fseek(input,0,SEEK_SET));
    data=malloc((size_t)size); CHECK(data && fread(data,1,(size_t)size,input)==(size_t)size); fclose(input);
    blob=hb_blob_create(data,(unsigned)size,HB_MEMORY_MODE_READONLY,NULL,NULL); face=hb_face_create(blob,0); font=hb_font_create(face);
    hb_ot_font_set_funcs(font); hb_font_set_scale(font,1000,1000);
    buffer=hb_buffer_create(); hb_buffer_add_utf8(buffer,"ffi",3,0,3); hb_buffer_guess_segment_properties(buffer); hb_shape(font,buffer,NULL,0);
    info=hb_buffer_get_glyph_infos(buffer,&count); positions=hb_buffer_get_glyph_positions(buffer,NULL);
    CHECK(count==1 && info[0].cluster==0 && positions[0].x_advance==1000);
    carets=2; CHECK(hb_ot_layout_get_ligature_carets(font,HB_DIRECTION_LTR,info[0].codepoint,0,&carets,stops)==2 &&
        carets==2 && stops[0]==250 && stops[1]==800);
    hb_buffer_destroy(buffer); hb_font_destroy(font); hb_face_destroy(face); hb_blob_destroy(blob); free(data);
    puts("Portable HarfBuzz C dependency: fixture GSUB/GPOS advance and GDEF stops passed; no native XGE platform verification claimed"); return 0;
}
