#include "xui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>

#define CHECK(e,m) do { if(!(e)) { printf("disabled editor style: %s\n",m); failed=1; goto cleanup; } } while(0)
static uint32_t strokeColors[64];
static int strokeCount;
static int (*baseStroke)(xui_proxy,xui_draw_context,xui_rect_t,float,uint32_t);
static int stroke(xui_proxy p,xui_draw_context d,xui_rect_t r,float w,uint32_t c)
{ if(strokeCount<64) strokeColors[strokeCount++]=c; return baseStroke(p,d,r,w,c); }
static int has(uint32_t c) { int i; for(i=0;i<strokeCount;i++) if(strokeColors[i]==c) return 1; return 0; }
int main(void)
{
	xui_test_proxy_state_t proxy;
	xui_context ctx=NULL;
	xui_widget edit=NULL;
	xui_font font=NULL;
	xui_text_edit_desc_t desc;
	xui_style_property_t prop;
	int failed=0;
	xuiTestProxyInit(&proxy);
	baseStroke=proxy.tProxy.drawRectStroke; proxy.tProxy.drawRectStroke=stroke;
	CHECK(xuiCreate(&ctx)==XUI_OK && xuiSetProxy(ctx,&proxy.tProxy)==XUI_OK,"context");
	CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK,"font");
	CHECK(xuiSetDefaultFont(ctx,font)==XUI_OK && xuiInputViewport(ctx,400,200)==XUI_OK,"viewport");
	memset(&desc,0,sizeof(desc)); desc.iSize=sizeof(desc); desc.pFont=font; desc.sText="Disabled"; desc.iBorderColor=0x123456ff;
	CHECK(xuiTextEditCreate(ctx,&edit,&desc)==XUI_OK && xuiSetRootWidget(ctx,edit)==XUI_OK,"editor");
	CHECK(xuiWidgetSetRect(edit,(xui_rect_t){0,0,400,200})==XUI_OK && xuiWidgetSetEnabled(edit,0)==XUI_OK,"disable");
	memset(&prop,0,sizeof(prop)); prop.iSize=sizeof(prop); prop.sName="textedit.border.disabled_color";
	prop.tValue.iSize=sizeof(prop.tValue); prop.tValue.iType=XUI_STYLE_VALUE_COLOR; prop.tValue.iColor=0x983412ff;
	CHECK(xuiStyleSetDefault(ctx,&prop,1)==XUI_OK && xuiRenderPrepare(ctx)==XUI_OK && has(prop.tValue.iColor),"disabled border obeys global color including alpha");
	prop.tValue.iColor=0; strokeCount=0;
	CHECK(xuiStyleSetDefault(ctx,&prop,1)==XUI_OK && xuiRenderPrepare(ctx)==XUI_OK && !has(0x983412ff),"disabled border transparent override");
	prop.sName="textedit.border.color"; strokeCount=0;
	CHECK(xuiStyleSetDefault(ctx,&prop,1)==XUI_OK && xuiRenderPrepare(ctx)==XUI_OK && !has(0x78),"disabled fallback does not resurrect transparent border");
	strokeCount=0;
	CHECK(xuiStyleClearDefault(ctx)==XUI_OK && xuiRenderPrepare(ctx)==XUI_OK && has(0x12345678),"clear restores API derived disabled border");
	puts("disabled editor global styles passed");
cleanup:
	if(ctx) xuiDestroy(ctx);
	if(font) proxy.tProxy.fontDestroy(&proxy.tProxy,font);
	return failed;
}
