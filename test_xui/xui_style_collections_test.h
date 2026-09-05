#ifndef XUI_STYLE_COLLECTIONS_TEST_H
#define XUI_STYLE_COLLECTIONS_TEST_H
#include "xui_test_proxy.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(e) do { if (!(e)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #e); exit(1); } } while (0)
#define COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))
#define COLOR_A 0x193751ffu
#define COLOR_B 0x619723ffu
#define COLOR_C 0x971963ffu
#define COLOR_D 0x3179a3ffu
typedef struct collection_key_t { const char* sKey; size_t iOffset; } collection_key_t;
typedef void (*collection_resolve_proc)(xui_widget, void*, void*);
static xui_test_proxy_state_t gProxy;
static xui_context gContext;
static xui_font gFont;
static xui_surface gTarget;
static uint32_t gColors[16384];
static int gColorCount, gMeasures, gDraws;
static xui_draw_rect_fill_proc gFill;
static xui_draw_rect_stroke_proc gStroke;
static xui_draw_text_proc gText;
static xui_draw_line_proc gLine;
static xui_draw_triangle_fill_proc gTriangle;
static xui_draw_circle_fill_proc gCircle;
static xui_text_measure_proc gMeasure;

static void collectionRecord(uint32_t c) { gDraws++; if ((c & 255u) && gColorCount < COUNT(gColors)) gColors[gColorCount++] = c; }
static int collectionFill(xui_proxy p, xui_draw_context d, xui_rect_t r, uint32_t c) { collectionRecord(c); return gFill(p,d,r,c); }
static int collectionStroke(xui_proxy p, xui_draw_context d, xui_rect_t r, float w, uint32_t c) { collectionRecord(c); return gStroke(p,d,r,w,c); }
static int collectionText(xui_proxy p, xui_draw_context d, xui_font f, const char* s, xui_rect_t r, uint32_t c, uint32_t flags) { collectionRecord(c); return gText(p,d,f,s,r,c,flags); }
static int collectionLine(xui_proxy p, xui_draw_context d, float x, float y, float a, float b, float w, uint32_t c) { collectionRecord(c); return gLine(p,d,x,y,a,b,w,c); }
static int collectionTriangle(xui_proxy p, xui_draw_context d, xui_vec2_t a, xui_vec2_t b, xui_vec2_t c, uint32_t color) { collectionRecord(color); return gTriangle(p,d,a,b,c,color); }
static int collectionCircle(xui_proxy p, xui_draw_context d, float x, float y, float r, uint32_t c) { collectionRecord(c); return gCircle(p,d,x,y,r,c); }
static int collectionMeasure(xui_proxy p, xui_font f, const char* s, xui_vec2_t* v) { gMeasures++; return gMeasure(p,f,s,v); }
static int collectionSeen(uint32_t c) { int i; for(i=0;i<gColorCount;i++) if(gColors[i]==c) return 1; return 0; }

static void collectionInit(void)
{
    xuiTestProxyInit(&gProxy);
    gFill=gProxy.tProxy.drawRectFill; gProxy.tProxy.drawRectFill=collectionFill;
    gStroke=gProxy.tProxy.drawRectStroke; gProxy.tProxy.drawRectStroke=collectionStroke;
    gText=gProxy.tProxy.drawText; gProxy.tProxy.drawText=collectionText;
    gLine=gProxy.tProxy.drawLine; gProxy.tProxy.drawLine=collectionLine;
    gTriangle=gProxy.tProxy.drawTriangleFill; gProxy.tProxy.drawTriangleFill=collectionTriangle;
    gCircle=gProxy.tProxy.drawCircleFill; gProxy.tProxy.drawCircleFill=collectionCircle;
    gMeasure=gProxy.tProxy.textMeasure; gProxy.tProxy.textMeasure=collectionMeasure;
    CHECK(xuiCreate(&gContext)==XUI_OK);
    CHECK(xuiSetProxy(gContext,&gProxy.tProxy)==XUI_OK);
    CHECK(gProxy.tProxy.fontLoadMemory(&gProxy.tProxy,&gFont,NULL,0,16,0)==XUI_OK);
    CHECK(xuiSetDefaultFont(gContext,gFont)==XUI_OK);
    CHECK(xuiInputViewport(gContext,640,480)==XUI_OK);
    CHECK(xuiTestSurfaceCreate(&gProxy,&gTarget,640,480,XUI_SURFACE_USAGE_TARGET)==XUI_OK);
}
static void collectionAttach(xui_widget w)
{
    CHECK(xuiWidgetSetRect(w,(xui_rect_t){0,0,640,480})==XUI_OK);
    CHECK(xuiSetRootWidget(gContext,w)==XUI_OK);
    CHECK(xuiLayout(gContext)==XUI_OK);
    CHECK(xuiRenderPrepare(gContext)==XUI_OK);
}
static void collectionRender(void)
{
    int measures=gMeasures, draws;
    gColorCount=0;
    CHECK(xuiRender(gContext,gTarget,NULL,0)==XUI_OK);
    CHECK(gMeasures==measures);
    draws=gDraws;
    CHECK(xuiRender(gContext,gTarget,NULL,0)==XUI_OK);
    CHECK(gDraws==draws);
    CHECK(gMeasures==measures);
}
static xui_style_property_t collectionProperty(const char* key, uint32_t color)
{
    xui_style_property_t p={0}; p.iSize=sizeof(p); p.sName=key;
    p.tValue.iSize=sizeof(p.tValue); p.tValue.iType=XUI_STYLE_VALUE_COLOR; p.tValue.iColor=color;
    return p;
}
static void collectionInline(xui_widget w, const char* key, uint32_t c)
{
    xui_style_property_t p=collectionProperty(key,c);
    CHECK(xuiWidgetSetInlineStyle(w,&p,1)==XUI_OK);
    CHECK((xuiWidgetGetDirtyFlags(w)&XUI_WIDGET_DIRTY_LAYOUT)==0);
}
static void collectionVerifyRenderedCascade(xui_widget w, const char* key, uint32_t baseColor)
{
    xui_widget_type type=xuiWidgetGetType(w);
    xui_style_property_t p=collectionProperty(key,COLOR_A);
    xui_style_desc_t style={0}; xui_style_value_t token={0};
    style.iSize=sizeof(style); style.pProperties=&p; style.iPropertyCount=1;
    CHECK(xuiStyleSetDefault(gContext,&p,1)==XUI_OK);
    collectionRender(); CHECK(collectionSeen(COLOR_A));
    token.iSize=sizeof(token); token.iType=XUI_STYLE_VALUE_COLOR; token.iColor=COLOR_B;
    CHECK(xuiStyleSetToken(gContext,"collections.paint",&token)==XUI_OK);
    p.tValue.iType=XUI_STYLE_VALUE_TOKEN; p.tValue.sText="collections.paint";
    CHECK(xuiStyleSetType(gContext,type,&style)==XUI_OK);
    collectionRender(); CHECK(collectionSeen(COLOR_B)); CHECK(!collectionSeen(COLOR_A));
    token.iColor=COLOR_D; CHECK(xuiStyleSetToken(gContext,"collections.paint",&token)==XUI_OK);
    collectionRender(); CHECK(collectionSeen(COLOR_D)); CHECK(!collectionSeen(COLOR_B));
    p=collectionProperty(key,COLOR_C);
    CHECK(xuiStyleSetClass(gContext,"collections.paint",&style)==XUI_OK);
    CHECK(xuiWidgetAddStyleClass(w,"collections.paint")==XUI_OK);
    collectionRender(); CHECK(collectionSeen(COLOR_C)); CHECK(!collectionSeen(COLOR_D));
    collectionInline(w,key,COLOR_B);
    collectionRender(); CHECK(collectionSeen(COLOR_B)); CHECK(!collectionSeen(COLOR_C));
    collectionInline(w,key,0);
    collectionRender(); CHECK(!collectionSeen(COLOR_B)); CHECK(!collectionSeen(COLOR_C));
    collectionInline(w,key,COLOR_C&0xffffff00u);
    collectionRender(); CHECK(!collectionSeen(COLOR_C));
    CHECK(xuiWidgetSetInlineStyle(w,NULL,0)==XUI_OK);
    collectionRender(); CHECK(collectionSeen(COLOR_C));
    CHECK(xuiWidgetRemoveStyleClass(w,"collections.paint")==XUI_OK);
    CHECK(xuiStyleRemoveClass(gContext,"collections.paint")==XUI_OK);
    collectionRender(); CHECK(collectionSeen(COLOR_D)); CHECK(!collectionSeen(COLOR_C));
    CHECK(xuiStyleRemoveType(gContext,type)==XUI_OK);
    collectionRender(); CHECK(collectionSeen(COLOR_A)); CHECK(!collectionSeen(COLOR_D));
    CHECK(xuiStyleClearDefault(gContext)==XUI_OK);
    collectionRender(); CHECK(collectionSeen(baseColor)); CHECK(!collectionSeen(COLOR_A));
    CHECK((xuiWidgetGetDirtyFlags(w)&XUI_WIDGET_DIRTY_LAYOUT)==0);
    printf("verified rendered cascade and warm frames: %s\n",key);
}
static void collectionVerifyKeys(xui_widget w, void* base, size_t size, collection_resolve_proc resolve, const collection_key_t* keys, int count)
{
    int i; void* effective=malloc(size); xui_widget_type type=xuiWidgetGetType(w);
    CHECK(effective!=NULL);
    for(i=0;i<count;i++) {
        uint32_t original=*(uint32_t*)((char*)base+keys[i].iOffset);
        xui_style_property_t p=collectionProperty(keys[i].sKey,COLOR_A);
        xui_style_property_info_t info={0}; xui_style_desc_t style={0}; xui_style_value_t token={0};
        CHECK(xuiStyleGetPropertyInfo(gContext,xuiStyleFindProperty(gContext,keys[i].sKey),&info)==XUI_OK);
        CHECK(info.iDirtyFlags==(XUI_WIDGET_DIRTY_CACHE|XUI_WIDGET_DIRTY_RENDER));
        style.iSize=sizeof(style); style.pProperties=&p; style.iPropertyCount=1;
        CHECK(xuiStyleSetDefault(gContext,&p,1)==XUI_OK);
        resolve(w,base,effective); CHECK(*(uint32_t*)((char*)effective+keys[i].iOffset)==COLOR_A);
        token.iSize=sizeof(token); token.iType=XUI_STYLE_VALUE_COLOR; token.iColor=COLOR_B;
        CHECK(xuiStyleSetToken(gContext,"collections.token",&token)==XUI_OK);
        p.tValue.iType=XUI_STYLE_VALUE_TOKEN; p.tValue.sText="collections.token";
        CHECK(xuiStyleSetType(gContext,type,&style)==XUI_OK);
        resolve(w,base,effective); CHECK(*(uint32_t*)((char*)effective+keys[i].iOffset)==COLOR_B);
        token.iColor=COLOR_D; CHECK(xuiStyleSetToken(gContext,"collections.token",&token)==XUI_OK);
        resolve(w,base,effective); CHECK(*(uint32_t*)((char*)effective+keys[i].iOffset)==COLOR_D);
        p=collectionProperty(keys[i].sKey,COLOR_C);
        CHECK(xuiStyleSetClass(gContext,"collections",&style)==XUI_OK);
        CHECK(xuiWidgetAddStyleClass(w,"collections")==XUI_OK);
        resolve(w,base,effective); CHECK(*(uint32_t*)((char*)effective+keys[i].iOffset)==COLOR_C);
        collectionInline(w,keys[i].sKey,0);
        resolve(w,base,effective); CHECK(*(uint32_t*)((char*)effective+keys[i].iOffset)==0);
        CHECK(*(uint32_t*)((char*)base+keys[i].iOffset)==original);
        collectionInline(w,keys[i].sKey,COLOR_C&0xffffff00u);
        resolve(w,base,effective); CHECK(*(uint32_t*)((char*)effective+keys[i].iOffset)==(COLOR_C&0xffffff00u));
        CHECK(xuiWidgetSetInlineStyle(w,NULL,0)==XUI_OK);
        resolve(w,base,effective); CHECK(*(uint32_t*)((char*)effective+keys[i].iOffset)==COLOR_C);
        CHECK(xuiWidgetRemoveStyleClass(w,"collections")==XUI_OK);
        CHECK(xuiStyleRemoveClass(gContext,"collections")==XUI_OK);
        resolve(w,base,effective); CHECK(*(uint32_t*)((char*)effective+keys[i].iOffset)==COLOR_D);
        CHECK(xuiStyleRemoveType(gContext,type)==XUI_OK);
        resolve(w,base,effective); CHECK(*(uint32_t*)((char*)effective+keys[i].iOffset)==COLOR_A);
        CHECK(xuiStyleClearDefault(gContext)==XUI_OK);
        resolve(w,base,effective); CHECK(*(uint32_t*)((char*)effective+keys[i].iOffset)==original);
        CHECK((xuiWidgetGetDirtyFlags(w)&XUI_WIDGET_DIRTY_LAYOUT)==0);
    }
    free(effective);
    printf("verified %d effective color keys (default/type/token/class/inline/zero/clear)\n",count);
}
static void collectionDone(void)
{
    xuiDestroy(gContext);
    gProxy.tProxy.surfaceDestroy(&gProxy.tProxy,gTarget);
    gProxy.tProxy.fontDestroy(&gProxy.tProxy,gFont);
}
#endif
