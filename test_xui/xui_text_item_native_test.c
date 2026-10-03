#include "../xui.h"
#include "../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#define W 160
#define H 80
#define CHECK(e) do{if(!(e)){fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e);exit(1);}}while(0)
typedef struct sample_t {const char *context,*language;int offset,bytes;uint32_t script,flags;unsigned glyphs[3],count;float em;} sample_t;
#define BEH "\xd8\xa8"
#define FATHA8 "\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e"
static const sample_t samples[]={
    {"\xd8\xa8\xd8\xa8\xd8\xa8","ar",0,2,0,XUI_TEXT_SHAPE_RTL,{7},1,.7f},
    {"\xd8\xa8\xd8\xa8\xd8\xa8","ar",2,2,0,XUI_TEXT_SHAPE_RTL,{8},1,.8f},
    {"\xd8\xa8\xd8\xa8\xd8\xa8","ar",4,2,0,XUI_TEXT_SHAPE_RTL,{9},1,.9f},
    {"\xd8\xa8\xd8\xa8\xd8\xa8","ar",0,4,0,XUI_TEXT_SHAPE_RTL,{7,8},2,1.5f},
    {"\xd8\xa8\xd8\xa8\xd8\xa8","ar",0,6,0,XUI_TEXT_SHAPE_RTL,{7,8,9},3,2.4f},
    {"\xdc\x90\xd9\x80\xdc\x90","syr",2,2,0,0,{11},1,.95f},
    {"\xdc\x90\xd9\x80\xdc\x90","ar",2,2,UINT32_C(0x41726162),0,{10},1,.4f},
    {"iii",NULL,1,1,0,0,{3},1,.3f},
    {"iii","en-US",1,1,0,0,{4},1,.4f},
    {"iii","TR-tr",1,1,UINT32_C(0x4c61746e),0,{5},1,.5f},
    {BEH FATHA8 BEH BEH,"ar",18,2,0,XUI_TEXT_SHAPE_RTL,{8},1,.8f},
    {BEH BEH FATHA8 BEH,"ar",2,2,UINT32_C(0x41726162),XUI_TEXT_SHAPE_RTL,{8},1,.8f},
    {BEH FATHA8 BEH FATHA8 BEH,"ar",18,2,0,XUI_TEXT_SHAPE_RTL,{8},1,.8f},
    {BEH "\xe2\x80\x8c" FATHA8 BEH FATHA8 BEH,"ar",21,2,UINT32_C(0x41726162),XUI_TEXT_SHAPE_RTL,{7},1,.7f}
};
#undef BEH
#undef FATHA8
static xui_proxy_t original;
static const sample_t* active;
static unsigned calls[5],cases;
static float active_offset;
static void record(const xui_text_item_t* item,unsigned method)
{
    if(!active || item->sContext!=active->context)return;
    CHECK(item->iSize==sizeof(*item) && item->iContextOffset==active->offset && item->iTextSize==active->bytes);
    CHECK(item->iContextSize==-1 || item->iContextSize==(int)strlen(active->context));
    CHECK(item->iScript==active->script && item->sLanguage==active->language &&
        item->iFlags==(XUI_TEXT_SHAPE_DEFAULT|active->flags) && item->fDrawOffsetX==active_offset);calls[method]++;
}
static int shape(xui_proxy p,const xui_text_item_t* item,xui_text_shape_t* out)
{record(item,0);return original.textShape(p,item,out);}
static int measure(xui_proxy p,const xui_text_item_t* item,xui_vec2_t* out)
{record(item,1);return original.textMeasure(p,item,out);}
static int draw(xui_proxy p,xui_draw_context dc,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags)
{record(item,2);return original.drawText(p,dc,item,rect,color,flags);}
static int spans(xui_proxy p,xui_draw_context dc,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags,const xui_text_paint_span_t* colors,int count)
{record(item,3);return original.drawTextSpans(p,dc,item,rect,color,flags,colors,count);}
static int surface(xui_proxy p,xui_surface target,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags)
{record(item,4);return original.textDraw(p,target,item,rect,color,flags);}
static void paint(xui_proxy p,xui_surface target,const xui_text_item_t* item,unsigned method)
{
    xui_draw_context dc;xui_rect_t rect={11.25f,7.5f,140,64};uint32_t color=XUI_COLOR_RGBA(50,100,220,255);
    /* Deliberately disagree with the item direction: paint flags cannot
     * override the shared input that was used to measure and shape. */
    uint32_t flags=XUI_TEXT_CLIP | (item->iFlags & XUI_TEXT_SHAPE_RTL ? 0:XUI_TEXT_RTL);
    CHECK(p->surfaceClear(p,target,0)==XUI_OK);
    if(method==4) CHECK(p->textDraw(p,target,item,rect,color,flags)==XUI_OK);
    else{
        CHECK(p->drawBegin(p,&dc,target)==XUI_OK);
        if(method==3){xui_text_paint_span_t span={sizeof(span),0,item->iTextSize,color};
            CHECK(p->drawTextSpans(p,dc,item,rect,color,flags,&span,1)==XUI_OK);}
        else CHECK(p->drawText(p,dc,item,rect,color,flags)==XUI_OK);
        CHECK(p->drawEnd(p,dc)==XUI_OK);
    }
}
static void invalid(xui_context context,xui_proxy p,xui_font font,xui_surface target)
{
    xui_text_item_t item={0};xui_text_shape_t shaped={0};xui_vec2_t size;xui_draw_context dc;unsigned test;
    item.iSize=sizeof(item);item.pFont=font;item.sText="i";item.iTextSize=1;
    CHECK(p->drawBegin(p,&dc,target)==XUI_OK);
    for(test=0;test<10;test++){
        xui_text_item_t bad=item;
        if(test==0)bad.iSize=0;
        if(test==1){bad.sContext="xi";bad.iContextSize=2;bad.iContextOffset=INT_MAX;}
        if(test==2){bad.sContext="xi";bad.iContextSize=2;}
        if(test==3)bad.sLanguage="en_US";
        if(test==4)bad.iScript=UINT32_C(0x4c617430);
        if(test==5){bad.sText="\xd8";bad.sContext="\xd8\xa8";bad.iContextSize=2;}
        if(test==6)bad.fDrawOffsetX=NAN;
        if(test==7)bad.fDrawOffsetX=INFINITY;
        if(test==8)bad.fDrawOffsetX=.5001f;
        if(test==9)bad.fDrawOffsetX=-.5001f;
        CHECK(xuiTextShape(context,&bad,&shaped)==XUI_ERROR_INVALID_ARGUMENT && !shaped.pClusters && !shaped.pCarets);
        CHECK(p->textShape(p,&bad,&shaped)==XUI_ERROR_INVALID_ARGUMENT && p->textMeasure(p,&bad,&size)==XUI_ERROR_INVALID_ARGUMENT);
        CHECK(p->drawText(p,dc,&bad,(xui_rect_t){0,0,W,H},0xffffffff,0)==XUI_ERROR_INVALID_ARGUMENT &&
            p->drawTextSpans(p,dc,&bad,(xui_rect_t){0,0,W,H},0xffffffff,0,NULL,0)==XUI_ERROR_INVALID_ARGUMENT &&
            p->textDraw(p,target,&bad,(xui_rect_t){0,0,W,H},0xffffffff,0)==XUI_ERROR_INVALID_ARGUMENT);
    }
    CHECK(p->drawEnd(p,dc)==XUI_OK);
    {xui_proxy_t fallback=*p;xui_context ctx;fallback.textShape=NULL;
        CHECK(xuiCreate(&ctx)==XUI_OK && xuiSetProxy(ctx,&fallback)==XUI_OK);
        item.sContext="xi";item.iContextSize=2;item.iContextOffset=1;
        CHECK(xuiTextShape(ctx,&item,&shaped)==XUI_ERROR_UNSUPPORTED && !shaped.pClusters);xuiDestroy(ctx);}
    {unsigned version;for(version=10;version<XUI_PROXY_VERSION;version++){
        xui_proxy_t old=*p;xui_context ctx;old.iVersion=version;CHECK(xuiCreate(&ctx)==XUI_OK &&
            xuiSetProxy(ctx,&old)==XUI_ERROR_UNSUPPORTED);xuiDestroy(ctx);}}
}
static int frame(void* user)
{
    float size=*(float*)user;xui_proxy_t proxy=original=xuiProxyXge();xui_context context;xui_font font;
    xui_surface target[2];xui_surface_desc_t desc={0};unsigned i,method;unsigned char expected[W*H*4],actual[W*H*4];
    proxy.textShape=shape;proxy.textMeasure=measure;proxy.drawText=draw;proxy.drawTextSpans=spans;proxy.textDraw=surface;
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy)==XUI_OK &&
        proxy.fontLoadFile(&proxy,&font,"test/data/xge_context_fixture.ttf",size,0)==XUI_OK);
    desc.iWidth=W;desc.iHeight=H;desc.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
    CHECK(proxy.surfaceCreate(&proxy,&target[0],&desc)==XUI_OK && proxy.surfaceCreate(&proxy,&target[1],&desc)==XUI_OK);
    for(i=0;i<sizeof(samples)/sizeof(*samples);i++){
        xui_text_item_t item={0},before,reference;xui_text_shape_t shaped={0};xui_vec2_t measured;
        const sample_t* sample=&samples[i];char text[8],pua[9];unsigned glyph;
        active=sample;memset(text,0xaa,sizeof(text));memcpy(text,sample->context+sample->offset,(size_t)sample->bytes);
        item.iSize=sizeof(item);item.pFont=font;item.sText=text;item.iTextSize=sample->bytes;item.iFlags=XUI_TEXT_SHAPE_DEFAULT|sample->flags;
        item.fDrawOffsetX=active_offset=i&1?-.375f:.375f;
        item.sContext=sample->context;item.iContextSize=-1;item.iContextOffset=sample->offset;item.iScript=sample->script;item.sLanguage=sample->language;
        before=item;CHECK(xuiTextShape(context,&item,&shaped)==XUI_OK && shaped.iTextSize==sample->bytes &&
            fabsf(shaped.fWidth-size*sample->em)<.002f);
        for(glyph=0;glyph<(unsigned)shaped.iClusterCount;glyph++) CHECK(shaped.pClusters[glyph].iTextStart>=0 && shaped.pClusters[glyph].iTextEnd<=item.iTextSize);
        xuiTextShapeFree(&shaped);CHECK(proxy.textMeasure(&proxy,&item,&measured)==XUI_OK && fabsf(measured.fX-ceilf(size*sample->em-0.0001f))<.001f);
        reference=item;reference.sText=pua;reference.iTextSize=(int)sample->count*3;reference.sContext=NULL;reference.iContextSize=reference.iContextOffset=0;reference.iScript=0;reference.sLanguage=NULL;
        for(glyph=0;glyph<sample->count;glyph++){unsigned cp=0xe100+sample->glyphs[glyph];pua[glyph*3]=(char)(0xe0|(cp>>12));pua[glyph*3+1]=(char)(0x80|((cp>>6)&63));pua[glyph*3+2]=(char)(0x80|(cp&63));}
        paint(&original,target[1],&reference,2);CHECK(original.surfaceReadRGBA(&original,target[1],expected,W*4)==XUI_OK);
        for(method=2;method<5;method++){
            unsigned drawn=0;paint(&proxy,target[0],&item,method);CHECK(proxy.surfaceReadRGBA(&proxy,target[0],actual,W*4)==XUI_OK);
            for(glyph=0;glyph<W*H;glyph++){CHECK(actual[glyph*4+3]==expected[glyph*4+3]);drawn+=actual[glyph*4+3]>0;}
            CHECK(drawn>20 && !memcmp(&item,&before,sizeof(item)));cases++;
        }
    }
    active=NULL;
    {xui_text_item_t item={0};xui_text_shape_t shaped={0};item.iSize=sizeof(item);item.pFont=font;
        item.sText="\xd8\xa8\xd9\x8e";item.iTextSize=4;item.iFlags=XUI_TEXT_SHAPE_DEFAULT|XUI_TEXT_SHAPE_RTL;
        item.sContext="\xd8\xa8\xd8\xa8\xd9\x8e\xd8\xa8";item.iContextSize=8;item.iContextOffset=2;
        CHECK(xuiTextShape(context,&item,&shaped)==XUI_OK && shaped.iClusterCount==1 && shaped.pClusters[0].iTextEnd==4 && fabsf(shaped.fWidth-size*.8f)<.002f);
        xuiTextShapeFree(&shaped);item.sText="i";item.iTextSize=0;item.sContext=NULL;item.iContextSize=item.iContextOffset=0;
        CHECK(xuiTextShape(context,&item,&shaped)==XUI_OK && !shaped.iClusterCount && !shaped.fWidth);xuiTextShapeFree(&shaped);}
    invalid(context,&proxy,font,target[0]);proxy.fontDestroy(&proxy,font);
    {xui_text_item_t item={0};xui_text_shape_t shaped={0};CHECK(proxy.fontLoadFile(&proxy,&font,"test/data/xge_opentype_fixture.ttf",size,0)==XUI_OK);
        item.iSize=sizeof(item);item.pFont=font;item.sText="ffi";item.iTextSize=3;item.iFlags=XUI_TEXT_SHAPE_DEFAULT;
        item.sContext="\xce\xbbxffiy";item.iContextSize=7;item.iContextOffset=3;item.iScript=UINT32_C(0x4c61746e);
        CHECK(xuiTextShape(context,&item,&shaped)==XUI_OK && shaped.iClusterCount==1 && shaped.iCaretCount==2 && shaped.pCarets[0].iTextOffset==1 && shaped.pCarets[1].iTextOffset==2);
        CHECK(fabsf(shaped.pCarets[0].fAdvance-size*.25f)<.001f && fabsf(shaped.pCarets[1].fAdvance-size*.8f)<.001f);
        xuiTextShapeFree(&shaped);
        item.sContext="\xce\xbbx\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e\xd9\x8e" "ffiy";
        item.iContextSize=23;item.iContextOffset=19;
        CHECK(xuiTextShape(context,&item,&shaped)==XUI_OK && shaped.iClusterCount==1 && shaped.iCaretCount==2 &&
            shaped.pCarets[0].iTextOffset==1 && shaped.pCarets[1].iTextOffset==2);
        CHECK(fabsf(shaped.pCarets[0].fAdvance-size*.25f)<.001f && fabsf(shaped.pCarets[1].fAdvance-size*.8f)<.001f);
        xuiTextShapeFree(&shaped);proxy.fontDestroy(&proxy,font);}
    for(i=0;i<5;i++)CHECK(calls[i]>0);
    proxy.surfaceDestroy(&proxy,target[0]);proxy.surfaceDestroy(&proxy,target[1]);xuiDestroy(context);
    printf("XUI shared text item: %u cumulative exact-alpha cases, full context/language/script through all five callbacks, bounded bytes, paint direction independence, GPOS/GDEF and input/version rejection at %g passed\n",cases,(double)size);
    xgeQuit();return XGE_OK;
}
int main(void)
{
    xge_desc_t desc={0};float sizes[]={40,37};unsigned i;desc.iWidth=W;desc.iHeight=H;desc.iFlags=XGE_INIT_OFFSCREEN;desc.iRunMode=XGE_RUN_GAME_LOOP;
    for(i=0;i<2;i++){CHECK(xgeInit(&desc)==XGE_OK && xgeRun(frame,&sizes[i])==XGE_OK);xgeUnit();}return 0;
}
