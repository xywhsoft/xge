#include "../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do{if(!(e)){fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e);exit(1);}}while(0)
static xge_glyph_run_t shape(xge_font font,const char* text,unsigned flags)
{xge_text_shape_desc_t desc={0};xge_glyph_run_t run={0};desc.iSize=sizeof(desc);desc.sText=text;desc.iTextSize=(int)strlen(text);desc.pFont=font;desc.iFlags=flags;
 CHECK(xgeTextShape(&desc,&run)==XGE_OK);return run;}
static void near(float actual,float expected){if(fabsf(actual-expected)>.001f)fprintf(stderr,"actual=%g expected=%g\n",actual,expected);CHECK(fabsf(actual-expected)<.001f);}
static void same(const xge_glyph_run_t* actual,const xge_glyph_run_t* expected,xge_font font)
{
    int i;CHECK(actual->iGlyphCount==expected->iGlyphCount && actual->iCaretCount==expected->iCaretCount);
    near(actual->fWidth,expected->fWidth);near(actual->fHeight,expected->fHeight);
    for(i=0;i<actual->iGlyphCount;i++){
        const xge_glyph_position_t *a=&actual->pGlyphs[i],*b=&expected->pGlyphs[i];
        CHECK(a->pFont==font && a->iGlyph==b->iGlyph && a->iCluster==b->iCluster && a->iClusterEnd==b->iClusterEnd);
        near(a->fAdvanceX,b->fAdvanceX);near(a->fOffsetX,b->fOffsetX);near(a->fOffsetY,b->fOffsetY);near(a->fVisualX,b->fVisualX);
    }
    for(i=0;i<actual->iCaretCount;i++){CHECK(actual->pCarets[i].iTextOffset==expected->pCarets[i].iTextOffset);near(actual->pCarets[i].fAdvance,expected->pCarets[i].fAdvance);}
}
int main(void)
{
    const char* paths[]={"no_mark","no_base","no_i"};const char* texts[]={"a\xcc\x81","ffi","ff\xcc\x81i","\xce\xbb\xce\xbc","\xd7\x90\xd6\xb0\xd7\x91\xd7\x92"};
    unsigned p,t;size_t cases=0;
    for(p=0;p<3;p++)for(t=0;t<5;t++){
        xge_font_t root={0},full={0};char path[128];xge_glyph_run_t actual,expected;unsigned flags=XGE_TEXT_SHAPE_DEFAULT|(t==4?XGE_TEXT_SHAPE_RTL:0);
        if((p==0 && (t==1 || t==3)) || (p==1 && (t==1 || t==2 || t==3)) || (p==2 && t==0))continue;
        snprintf(path,sizeof(path),"test/data/xge_fallback_%s.ttf",paths[p]);
        CHECK(xgeFontLoad(&root,path,37)==XGE_OK && xgeFontLoad(&full,"test/data/xge_opentype_fixture.ttf",37)==XGE_OK);
        xgeFontSetFallback(&root,&full);actual=shape(&root,texts[t],flags);expected=shape(&full,texts[t],flags);
        same(&actual,&expected,&full);xgeGlyphRunFree(&expected);
        /* The selected fallback must outlive both callers and the root's
         * fallback ownership. A run may retain only the selected font. */
        xgeFontFree(&root);xgeFontFree(&full);CHECK(!root.iRefCount && full.iRefCount==1);
        near(xgeGlyphRunMeasure(&actual).fX,actual.fWidth);xgeGlyphRunFree(&actual);CHECK(!full.iRefCount);cases++;
    }
    {
        xge_font_t root={0},full={0};xge_glyph_run_t run;
        CHECK(xgeFontLoad(&root,"test/data/xge_fallback_composed.ttf",37)==XGE_OK && xgeFontLoad(&full,"test/data/xge_opentype_fixture.ttf",37)==XGE_OK);
        xgeFontSetFallback(&root,&full);run=shape(&root,"a\xcc\x81",XGE_TEXT_SHAPE_DEFAULT);
        CHECK(run.iGlyphCount==1 && run.pGlyphs[0].pFont==&root && run.pGlyphs[0].iGlyph==4 && run.pGlyphs[0].iClusterEnd==3 && !run.iCaretCount);
        xgeGlyphRunFree(&run);run=shape(&root,"a\xe2\x80\x8d\xef\xb8\x8f",XGE_TEXT_SHAPE_DEFAULT);
        {int i;for(i=0;i<run.iGlyphCount;i++)CHECK(run.pGlyphs[i].pFont==&root);}
        xgeGlyphRunFree(&run);xgeFontFree(&root);xgeFontFree(&full);
    }
    printf("Native SFNT fallback: %zu complete cluster/word glyph, GPOS/GDEF, RTL and retained-font oracles; NFC primary and ignorable coverage passed\n",cases);return 0;
}
