#include "xui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>

#define CHECK(e,m) do { if (!(e)) { printf("tooltip styles: %s\n",m); failed=1; goto cleanup; } } while (0)
static uint32_t colors[128];
static int count;
static int (*baseFill)(xui_proxy,xui_draw_context,xui_rect_t,uint32_t);
static int (*baseStroke)(xui_proxy,xui_draw_context,xui_rect_t,float,uint32_t);
static int (*baseText)(xui_proxy,xui_draw_context,xui_font,const char*,xui_rect_t,uint32_t,uint32_t);
static int fill(xui_proxy p,xui_draw_context d,xui_rect_t r,uint32_t c)
{ if(count<128) colors[count++]=c; return baseFill(p,d,r,c); }
static int stroke(xui_proxy p,xui_draw_context d,xui_rect_t r,float w,uint32_t c)
{ if(count<128) colors[count++]=c; return baseStroke(p,d,r,w,c); }
static int text(xui_proxy p,xui_draw_context d,xui_font f,const char* s,xui_rect_t r,uint32_t c,uint32_t flags)
{ if(count<128) colors[count++]=c; return baseText(p,d,f,s,r,c,flags); }
static int has(uint32_t c) { int i; for(i=0;i<count;++i) if(colors[i]==c) return 1; return 0; }
static xui_vec2_t measure_custom(xui_context ctx,xui_widget owner,void* user)
{ (void)ctx; (void)owner; (void)user; return (xui_vec2_t){80,24}; }
static int paint_custom(xui_context ctx,xui_widget owner,xui_draw_context draw,xui_rect_t rect,void* user)
{ xui_proxy p=user; (void)ctx; (void)owner; return p->drawRectFill(p,draw,rect,0x345678ff); }

int main(void)
{
	static const char* const names[]={"tooltip.background.color","tooltip.border.color","tooltip.text.color"};
	xui_test_proxy_state_t proxy;
	xui_context ctx=NULL;
	xui_widget root=NULL,owner=NULL;
	xui_font font=NULL;
	xui_style_property_t props[3];
	xui_chrome_style_t chrome,base;
	xui_tooltip_desc_t tip={0};
	int failed=0,i;
	xuiTestProxyInit(&proxy);
	baseFill=proxy.tProxy.drawRectFill; proxy.tProxy.drawRectFill=fill;
	baseStroke=proxy.tProxy.drawRectStroke; proxy.tProxy.drawRectStroke=stroke;
	baseText=proxy.tProxy.drawText; proxy.tProxy.drawText=text;
	CHECK(xuiCreate(&ctx)==XUI_OK && xuiSetProxy(ctx,&proxy.tProxy)==XUI_OK,"context");
	CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK,"font");
	CHECK(xuiSetDefaultFont(ctx,font)==XUI_OK && xuiInputViewport(ctx,400,200)==XUI_OK,"viewport");
	CHECK(xuiWidgetCreate(ctx,&root)==XUI_OK && xuiSetRootWidget(ctx,root)==XUI_OK,"root");
	CHECK(xuiWidgetSetLayoutType(root,XUI_LAYOUT_MANUAL)==XUI_OK,"manual");
	CHECK(xuiWidgetCreate(ctx,&owner)==XUI_OK && xuiWidgetAddChild(root,owner)==XUI_OK,"owner");
	CHECK(xuiWidgetSetRect(owner,(xui_rect_t){20,20,100,30})==XUI_OK,"owner rect");
	CHECK(xuiWidgetSetTooltipText(owner,"Styled tooltip")==XUI_OK,"text");
	CHECK(xuiLayout(ctx)==XUI_OK && xuiInputPointerMove(ctx,30,30,0)==XUI_OK,"hover");
	CHECK(xuiUpdate(ctx,1.0f)==XUI_OK && xuiWidgetTooltipIsOpen(ctx),"open");
	CHECK(xuiGetChromeStyle(ctx,&base)==XUI_OK && xuiRenderPrepare(ctx)==XUI_OK,"warm");
	count=0;
	CHECK(xuiRenderPrepare(ctx)==XUI_OK && count==0,"warm cache should not repaint");
	for(i=0;i<3;++i) {
		CHECK(xuiStyleFindProperty(ctx,names[i])!=0,"registered");
		memset(&props[i],0,sizeof(props[i])); props[i].iSize=sizeof(props[i]); props[i].sName=names[i];
		props[i].tValue.iSize=sizeof(props[i].tValue); props[i].tValue.iType=XUI_STYLE_VALUE_COLOR;
		props[i].tValue.iColor=0x895634ffu+((uint32_t)i<<16);
	}
	CHECK(xuiStyleSetDefault(ctx,props,3)==XUI_OK && xuiRenderPrepare(ctx)==XUI_OK,"live colors without update");
	for(i=0;i<3;++i) CHECK(has(props[i].tValue.iColor),names[i]);
	CHECK(xuiGetChromeStyle(ctx,&chrome)==XUI_OK && chrome.iTooltipColor==base.iTooltipColor,"API base preserved");
	for(i=0;i<3;++i) props[i].tValue.iColor=0;
	count=0;
	CHECK(xuiStyleSetDefault(ctx,props,3)==XUI_OK && xuiRenderPrepare(ctx)==XUI_OK,"transparent sheet");
	for(i=0;i<count;++i) CHECK((colors[i]&255u)==0,"transparent color must not regain alpha");
	count=0;
	CHECK(xuiStyleClearDefault(ctx)==XUI_OK && xuiRenderPrepare(ctx)==XUI_OK,"clear sheet");
	CHECK(has(base.iTooltipColor) && has(base.iTooltipTextColor) && has(base.iPopupBorderColor),"restore API base");
	chrome=base; chrome.iTooltipColor=0xb78241ff; chrome.iTooltipTextColor=0x278346ff;
	count=0;
	CHECK(xuiSetChromeStyle(ctx,&chrome)==XUI_OK && xuiRenderPrepare(ctx)==XUI_OK,"live ChromeStyle");
	CHECK(has(chrome.iTooltipColor) && has(chrome.iTooltipTextColor),"ChromeStyle invalidates existing tooltip");
	tip.iSize=sizeof(tip); tip.iType=XUI_TOOLTIP_CUSTOM; tip.fDelay=0;
	tip.onMeasure=measure_custom; tip.onPaint=paint_custom; tip.pUser=&proxy.tProxy;
	CHECK(xuiWidgetSetTooltip(owner,&tip)==XUI_OK && xuiUpdate(ctx,1.0f)==XUI_OK,"custom tooltip");
	count=0;
	CHECK(xuiRenderPrepare(ctx)==XUI_OK && has(0x345678ff),"custom content color retained");
	puts("tooltip global styles passed");
cleanup:
	if(ctx) xuiDestroy(ctx);
	if(font) proxy.tProxy.fontDestroy(&proxy.tProxy,font);
	return failed;
}
