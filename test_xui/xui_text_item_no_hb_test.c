#include "../xui.h"
#include "../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define W 100
#define H 64
#define CHECK(e) do {if (!(e)) {fprintf(stderr,"no-HB text item line %d: %s\n",__LINE__,#e);exit(1);}} while (0)

static void paint(xui_proxy p,xui_surface target,const xui_text_item_t* item,int method)
{
    xui_rect_t rect={7.25f,3.5f,80,55};xui_draw_context draw;
    CHECK(p->surfaceClear(p,target,0)==XUI_OK);
    if (method==2) CHECK(p->textDraw(p,target,item,rect,0xffffffff,XUI_TEXT_RTL|XUI_TEXT_CLIP)==XUI_OK);
    else {
        CHECK(p->drawBegin(p,&draw,target)==XUI_OK);
        if (method==1) {
            xui_text_paint_span_t span={sizeof(span),0,item->iTextSize,0xffffffff};
            CHECK(p->drawTextSpans(p,draw,item,rect,0xffffffff,XUI_TEXT_RTL|XUI_TEXT_CLIP,&span,1)==XUI_OK);
        } else CHECK(p->drawText(p,draw,item,rect,0xffffffff,XUI_TEXT_RTL|XUI_TEXT_CLIP)==XUI_OK);
        CHECK(p->drawEnd(p,draw)==XUI_OK);
    }
}
static int frame(void* ignored)
{
    xui_proxy_t p=xuiProxyXge();xui_context context;xui_font font;xui_surface target[2];
    xui_surface_desc_t desc={0};xui_text_item_t item={0},reference,bad;xui_text_shape_t shape={0};xui_vec2_t size;
    unsigned char expected[W*H*4],actual[W*H*4];const char text[3]={'i','i','i'};
    unsigned at,drawn=0;int method,test;xui_draw_context draw;
    (void)ignored;
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&p)==XUI_OK);
    CHECK(p.fontLoadFile(&p,&font,"test/data/xge_context_fixture.ttf",40,0)==XUI_OK);
    desc.iWidth=W;desc.iHeight=H;desc.iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED;
    CHECK(p.surfaceCreate(&p,&target[0],&desc)==XUI_OK && p.surfaceCreate(&p,&target[1],&desc)==XUI_OK);
    item.iSize=sizeof(item);item.pFont=font;item.sText=text;item.iTextSize=2;item.iFlags=XUI_TEXT_SHAPE_DEFAULT;
    /* Without HB, fixture cmap i is glyph 2 at 200 units per em, without locl. */
    CHECK(xuiTextShape(context,&item,&shape)==XUI_OK && shape.iClusterCount==2 && shape.iTextSize==2 && fabsf(shape.fWidth-16)<.001f);
    xuiTextShapeFree(&shape);
    CHECK(p.textMeasure(&p,&item,&size)==XUI_OK && fabsf(size.fX-16)<.001f);
    reference=item;reference.sText="\xee\x84\x82\xee\x84\x82";reference.iTextSize=6;
    paint(&p,target[1],&reference,0);
    CHECK(p.surfaceReadRGBA(&p,target[1],expected,W*4)==XUI_OK);
    for(method=0;method<3;method++) {
        paint(&p,target[0],&item,method);CHECK(p.surfaceReadRGBA(&p,target[0],actual,W*4)==XUI_OK);
        for(at=0;at<W*H;at++){CHECK(actual[at*4+3]==expected[at*4+3]);drawn+=actual[at*4+3]!=0;}
    }
    CHECK(drawn>60);
    CHECK(p.drawBegin(&p,&draw,target[0])==XUI_OK);
    for(test=0;test<5;test++) {
        bad=item;
        if(test==0){bad.sContext="iii";bad.iContextSize=3;bad.iContextOffset=1;}
        if(test==1)bad.iScript=UINT32_C(0x4c61746e);
        if(test==2)bad.sLanguage="en-US";
        if(test==3)bad.iFlags|=XUI_TEXT_SHAPE_RTL;
        if(test==4){bad.sContext=text;bad.iContextSize=2;}
        CHECK(xuiTextShape(context,&bad,&shape)==XUI_ERROR_UNSUPPORTED && !shape.pClusters);
        CHECK(p.textShape(&p,&bad,&shape)==XUI_ERROR_UNSUPPORTED && p.textMeasure(&p,&bad,&size)==XUI_ERROR_UNSUPPORTED);
        CHECK(p.drawText(&p,draw,&bad,(xui_rect_t){0,0,W,H},0xffffffff,0)==XUI_ERROR_UNSUPPORTED &&
            p.drawTextSpans(&p,draw,&bad,(xui_rect_t){0,0,W,H},0xffffffff,0,NULL,0)==XUI_ERROR_UNSUPPORTED &&
            p.textDraw(&p,target[0],&bad,(xui_rect_t){0,0,W,H},0xffffffff,0)==XUI_ERROR_UNSUPPORTED);
    }
    CHECK(p.drawEnd(&p,draw)==XUI_OK);
    item.iTextSize=0;CHECK(xuiTextShape(context,&item,&shape)==XUI_OK && !shape.iClusterCount && !shape.fWidth);xuiTextShapeFree(&shape);
    {
        xui_font owned;
        CHECK(p.fontLoadFile(&p,&owned,"test/data/xge_context_fixture.ttf",40,0)==XUI_OK);
        item.pFont=owned;item.iTextSize=2;item.iFlags|=XUI_TEXT_SHAPE_RETAIN_PAINT;
        CHECK(xuiTextShape(context,&item,&shape)==XUI_OK && shape.pPaint && shape.iPaintBytes>0);
        p.fontDestroy(&p,owned);
        CHECK(p.surfaceClear(&p,target[0],0)==XUI_OK && p.drawBegin(&p,&draw,target[0])==XUI_OK &&
            p.drawTextShapeRange(&p,draw,&shape,0,2,(xui_rect_t){7,3,80,55},0xffffffff,XUI_TEXT_CLIP)==XUI_OK &&
            p.drawEnd(&p,draw)==XUI_OK && p.surfaceReadRGBA(&p,target[0],actual,W*4)==XUI_OK &&
            memcmp(actual,expected,sizeof(actual))==0);
        xuiTextShapeFree(&shape);xuiTextShapeFree(&shape);
    }
    p.surfaceDestroy(&p,target[0]);p.surfaceDestroy(&p,target[1]);p.fontDestroy(&p,font);xuiDestroy(context);
    puts("XUI native no-HB: bounded implicit-context text through all five callbacks, literal fixture advances, 3 exact-alpha draws, retained glyph full RGBA after font release and explicit context/script/language/RTL rejection passed");
    xgeQuit();return XGE_OK;
}
int main(void)
{
    xge_desc_t desc={0};desc.iWidth=W;desc.iHeight=H;desc.iFlags=XGE_INIT_OFFSCREEN;desc.iRunMode=XGE_RUN_GAME_LOOP;
    CHECK(xgeInit(&desc)==XGE_OK && xgeRun(frame,NULL)==XGE_OK);xgeUnit();return 0;
}
