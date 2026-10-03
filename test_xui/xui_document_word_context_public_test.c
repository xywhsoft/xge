/* Literal cmap/advance and font-identity oracles, no renderer/window needed. */
#include "../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do{if(!(e)){fprintf(stderr,"word context line %d case %u: %s\n",__LINE__,cases,#e);exit(1);}}while(0)
static unsigned cases;
static void check_item(xge_font root,xge_font wanted,const char* context,int offset,int bytes,
    unsigned script,unsigned flags,int glyph,float width,int copy)
{
    xge_glyph_run_t run={0};char item[8];CHECK(bytes<=(int)sizeof(item));
    memcpy(item,context+offset,(size_t)bytes);
    xge_text_shape_desc_t desc={.iSize=sizeof(desc),.pFont=root,.sText=copy?item:context+offset,
        .iTextSize=bytes,.sContext=context,.iContextSize=(int)strlen(context),.iContextOffset=offset,
        .iScript=script,.sLanguage="en",.iFlags=flags};
    int result=xgeTextShape(&desc,&run);
    if(result!=XGE_OK || run.iGlyphCount!=1 || run.pGlyphs[0].pFont!=wanted ||
        run.pGlyphs[0].iGlyph!=glyph || fabsf(run.fWidth-width)>.002f)
        fprintf(stderr,"context=%s offset=%d bytes=%d result=%d count=%d glyph=%d width=%g expected=%d/%g font=%p wanted=%p\n",
            context,offset,bytes,result,run.iGlyphCount,run.iGlyphCount?run.pGlyphs[0].iGlyph:-1,
            run.fWidth,glyph,width,run.iGlyphCount?(void*)run.pGlyphs[0].pFont:NULL,(void*)wanted);
    CHECK(result==XGE_OK && run.iGlyphCount==1 &&
        run.pGlyphs[0].pFont==wanted && run.pGlyphs[0].iGlyph==glyph &&
        run.pGlyphs[0].iCluster==0 && run.pGlyphs[0].iClusterEnd==(unsigned)bytes &&
        fabsf(run.fWidth-width)<.002f && fabsf(run.pGlyphs[0].fAdvanceX-(glyph==9?0:width))<.002f);
    CHECK(xgeGlyphRunRetainedBytes(&run)>0);xgeGlyphRunFree(&run);cases++;
}
int main(void)
{
    const float sizes[]={40,37};
    for(unsigned size=0;size<2;size++){
        xge_font_t root={0},full={0};float s=sizes[size];
        CHECK(xgeFontLoad(&root,"test/data/xge_word_context_primary.ttf",s)==XGE_OK &&
            xgeFontLoad(&full,"test/data/xge_word_context_full.ttf",s)==XGE_OK);
        xgeFontSetFallback(&root,&full);
        for(unsigned rtl=0;rtl<2;rtl++)for(int copy=0;copy<2;copy++){
            unsigned flags=XGE_TEXT_SHAPE_DEFAULT|(rtl?XGE_TEXT_SHAPE_RTL:0);
            for(int i=0;i<3;i++)check_item(&root,&full,"ffi",i,1,0,flags,i==2?3:2,s*(i==2?.3f:.4f),copy);
            check_item(&root,&full,"fxi",1,1,0,flags,4,s*.5f,copy);
            check_item(&root,&root,"ff if",0,1,0,flags,2,s*.2f,copy);
            check_item(&root,&full,"ff if",4,1,0,flags,2,s*.4f,copy);
            check_item(&root,&root,"f,i",0,1,0,flags,2,s*.2f,copy);
            check_item(&root,&root,"f\ni",0,1,0,flags,2,s*.2f,copy);
            check_item(&root,&root,"f\tix",0,1,0,flags,2,s*.2f,copy);
            check_item(&root,&root,"(f)i",1,1,0,flags,2,s*.2f,copy);
            check_item(&root,&root,"f\xce\xb1",0,1,0,flags,2,s*.2f,copy);
            check_item(&root,&full,"f\xce\xb1",1,2,0,flags,8,s*.6f,copy);
            /* Explicit script uses the same policy as a complete-item input. */
            check_item(&root,&full,"f\xce\xb1",0,1,UINT32_C(0x4c61746e),flags,2,s*.4f,copy);
            check_item(&root,&root,"\xc3\xa9 i",0,2,0,flags,11,s*.5f,copy);
            /* Root covers the NFC-composed grapheme, but not its e or acute
             * slice. Whole-word coverage alone must not select it for either. */
            check_item(&root,&root,"e\xcc\x81 i",0,3,0,flags,11,s*.5f,copy);
            check_item(&root,&full,"e\xcc\x81 i",0,1,0,flags,10,s*.4f,copy);
            /* Standalone marks retain their outward-rounded raster ink bound. */
            check_item(&root,&full,"e\xcc\x81 i",1,2,0,flags,9,ceilf(s*.1f),copy);
            check_item(&root,&full,"e\xcc\x81i",0,1,0,flags,10,s*.4f,copy);
            /* A single item can cross several words. Cached successful and
             * failed coverage must stop at the contextual word boundary. */
            const char* context="(ffi ff)";char item[6]={'f','f','i',' ','f','f'};
            xge_glyph_run_t run={0};xge_text_shape_desc_t desc={.iSize=sizeof(desc),.pFont=&root,
                .sText=copy?item:context+1,.iTextSize=6,.sContext=context,.iContextSize=8,
                .iContextOffset=1,.iFlags=flags};
            const int ids[]={2,2,3,1,2,2};const float advance[]={.4f,.4f,.3f,.25f,.2f,.2f};
            CHECK(xgeTextShape(&desc,&run)==XGE_OK && run.iGlyphCount==6 && fabsf(run.fWidth-s*1.75f)<.002f);
            for(int i=0;i<6;i++)CHECK(run.pGlyphs[i].pFont==(i<3?&full:&root) &&
                run.pGlyphs[i].iGlyph==ids[i] && run.pGlyphs[i].iCluster==(unsigned)i &&
                run.pGlyphs[i].iClusterEnd==(unsigned)(i+1) && fabsf(run.pGlyphs[i].fAdvanceX-s*advance[i])<.002f);
            xgeGlyphRunFree(&run);cases++;
        }
        /* Neither word nor item bytes are retained after publication. Only
         * the selected font survives both caller releases. */
        char context[3]={'f','f','i'},item[1]={'f'};xge_glyph_run_t run={0};
        xge_text_shape_desc_t desc={.iSize=sizeof(desc),.pFont=&root,.sText=item,.iTextSize=1,
            .sContext=context,.iContextSize=3,.iContextOffset=1,.iFlags=XGE_TEXT_SHAPE_DEFAULT};
        CHECK(xgeTextShape(&desc,&run)==XGE_OK && run.pGlyphs[0].pFont==&full);
        size_t kept=xgeGlyphRunRetainedBytes(&run);memset(context,'x',3);item[0]='x';
        xgeFontFree(&root);xgeFontFree(&full);
        CHECK(!root.iRefCount && full.iRefCount==1 && xgeGlyphRunRetainedBytes(&run)==kept &&
            fabsf(xgeGlyphRunMeasure(&run).fX-s*.4f)<.002f);
        xgeGlyphRunFree(&run);CHECK(!full.iRefCount);cases++;
    }
    printf("Public contextual whole-word fallback: %u literal font/glyph/advance/cluster cases; copied non-NUL items, both directions, word/script boundaries, NFC slice coverage and retained font lifetime passed\n",cases);
    return 0;
}
