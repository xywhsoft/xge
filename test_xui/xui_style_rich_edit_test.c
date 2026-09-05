#include "xui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>

#define CHECK(e, m) do { if (!(e)) { printf("style rich edit: %s (%d)\n", m, __LINE__); failed = 1; goto cleanup; } } while (0)
static uint32_t colors[2048];
static int colorCount;
static int (*baseText)(xui_proxy, xui_draw_context, xui_font, const char*, xui_rect_t, uint32_t, uint32_t);
static int (*baseFill)(xui_proxy, xui_draw_context, xui_rect_t, uint32_t);
static int (*baseStroke)(xui_proxy, xui_draw_context, xui_rect_t, float, uint32_t);
static int (*baseLine)(xui_proxy, xui_draw_context, float, float, float, float, float, uint32_t);
static void record(uint32_t color) { if (colorCount < 2048) colors[colorCount++] = color; }
static int has(uint32_t color) { int i; for (i=0;i<colorCount;i++) if(colors[i]==color) return 1; return 0; }
static int text(xui_proxy p,xui_draw_context d,xui_font f,const char* s,xui_rect_t r,uint32_t c,uint32_t flags)
{ record(c); return baseText(p,d,f,s,r,c,flags); }
static int fill(xui_proxy p,xui_draw_context d,xui_rect_t r,uint32_t c)
{ record(c); return baseFill(p,d,r,c); }
static int stroke(xui_proxy p,xui_draw_context d,xui_rect_t r,float w,uint32_t c)
{ record(c); return baseStroke(p,d,r,w,c); }
static int line(xui_proxy p,xui_draw_context d,float x,float y,float x2,float y2,float w,uint32_t c)
{ record(c); return baseLine(p,d,x,y,x2,y2,w,c); }
static xui_style_property_t property(const char* key,uint32_t color)
{
	xui_style_property_t p;
	memset(&p,0,sizeof(p)); p.iSize=sizeof(p); p.sName=key;
	p.tValue.iSize=sizeof(p.tValue); p.tValue.iType=XUI_STYLE_VALUE_COLOR; p.tValue.iColor=color;
	return p;
}

int main(void)
{
	static const char* const keys[]={
		"richedit.text.color","richedit.background.color","richedit.border.color",
		"richedit.border.focus_color","richedit.selection.color","richedit.cursor.color",
		"richedit.find.result_color","richedit.find.active_color","richedit.link.color",
		"richedit.quote.border_color","richedit.rule.color","richedit.paragraph.background_color",
		"richedit.table.border_color","richedit.table.header_color","richedit.table.cell_color",
		"richedit.image.placeholder_color","richedit.image.border_color","richedit.image.text_color"
	};
	xui_context ctx=NULL;
	xui_test_proxy_state_t proxy;
	xui_surface target=NULL;
	xui_widget edit=NULL;
	xui_font font=NULL;
	xui_rich_document doc=NULL;
	xui_rich_node p, table;
	xui_rich_edit_desc_t desc;
	xui_rich_text_style_t explicitStyle;
	xui_rich_image_desc_t image;
	xui_rich_table_desc_t tableDesc;
	xui_style_property_t props[18];
	xui_style_desc_t typeStyle;
	xui_rich_fragment_t before,after;
	xui_find_options_t find;
	xui_rect_i_t damage={0,0,640,600};
	uint32_t version;
	int failed=0,i;
	xuiTestProxyInit(&proxy);
	baseText=proxy.tProxy.drawText; proxy.tProxy.drawText=text;
	baseFill=proxy.tProxy.drawRectFill; proxy.tProxy.drawRectFill=fill;
	baseStroke=proxy.tProxy.drawRectStroke; proxy.tProxy.drawRectStroke=stroke;
	baseLine=proxy.tProxy.drawLine; proxy.tProxy.drawLine=line;
	CHECK(xuiCreate(&ctx)==XUI_OK,"context");
	CHECK(xuiSetProxy(ctx,&proxy.tProxy)==XUI_OK,"proxy");
	CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK,"font");
	CHECK(xuiSetDefaultFont(ctx,font)==XUI_OK,"default font");
	CHECK(xuiInputViewport(ctx,640,600)==XUI_OK,"viewport");
	CHECK(xuiTestSurfaceCreate(&proxy,&target,640,600,XUI_SURFACE_USAGE_TARGET)==XUI_OK,"target");
	CHECK(xuiRichDocumentCreate(&doc)==XUI_OK,"document");
	p=xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(doc));
	CHECK(xuiRichDocumentAppendText(doc,p,"default default ",NULL)!=NULL,"default text");
	memset(&explicitStyle,0,sizeof(explicitStyle)); explicitStyle.iSize=sizeof(explicitStyle);
	explicitStyle.iTextColor=0xc91347ff;
	CHECK(xuiRichDocumentAppendText(doc,p,"explicit ",&explicitStyle)!=NULL,"explicit text");
	CHECK(xuiRichDocumentAppendLink(doc,p,"link ","https://example.test",NULL)!=NULL,"default link");
	CHECK(xuiRichDocumentAppendLink(doc,p,"explicit link","https://example.test",&explicitStyle)!=NULL,"explicit link");
	p=xuiRichDocumentAppendBlock(doc,XUI_RICH_NODE_BLOCK_QUOTE,NULL);
	CHECK(p!=NULL && xuiRichDocumentAppendText(doc,p,"quote",NULL)!=NULL,"quote");
	memset(&image,0,sizeof(image)); image.iSize=sizeof(image); image.fWidth=80; image.fHeight=30; image.sAltText="missing"; image.sSource="missing.png";
	CHECK(xuiRichDocumentAppendImage(doc,&image)!=NULL,"placeholder image");
	memset(&tableDesc,0,sizeof(tableDesc)); tableDesc.iSize=sizeof(tableDesc); tableDesc.iRows=2; tableDesc.iColumns=2;
	table=xuiRichDocumentAppendTable(doc,&tableDesc);
	CHECK(table!=NULL,"table");
	CHECK(xuiRichDocumentAppendHorizontalRule(doc)!=NULL,"rule");
	memset(&desc,0,sizeof(desc)); desc.iSize=sizeof(desc); desc.pDocument=doc; desc.pFont=font; desc.bWordWrap=1;
	desc.iTextColor=0x293547ff; desc.iBackgroundColor=0xf3f5f7ff;
	CHECK(xuiRichEditCreate(ctx,&edit,&desc)==XUI_OK,"editor");
	CHECK(xuiSetRootWidget(ctx,edit)==XUI_OK,"root");
	CHECK(xuiWidgetSetRect(edit,(xui_rect_t){0,0,640,600})==XUI_OK,"rect");
	CHECK(xuiLayout(ctx)==XUI_OK,"layout");
	CHECK(xuiRichEditGetFragment(edit,0,&before)==XUI_OK,"initial fragment");
	version=xuiRichDocumentGetVersion(doc);
	for(i=0;i<18;i++) { props[i]=property(keys[i],0x301020ffu+((uint32_t)i<<16)); CHECK(xuiStyleFindProperty(ctx,keys[i])!=0,keys[i]); }
	CHECK(xuiStyleSetDefault(ctx,props,18)==XUI_OK,"default stylesheet");
	CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK,"themed render");
	for(i=0;i<18;i++) if(i!=3 && i!=4 && i!=5 && i!=6 && i!=7) CHECK(has(props[i].tValue.iColor),keys[i]);
	CHECK(has(explicitStyle.iTextColor),"explicit document colors survive global theme");
	CHECK(xuiRichDocumentGetVersion(doc)==version,"theme does not edit document");
	CHECK(xuiRichEditGetFragment(edit,0,&after)==XUI_OK && memcmp(&before.tRect,&after.tRect,sizeof(before.tRect))==0,"theme preserves geometry");
	CHECK(xuiRichEditSetSelection(edit,0,3)==XUI_OK && xuiSetFocusWidget(ctx,edit)==XUI_OK,"selection and focus");
	colorCount=0;
	CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK,"selected render");
	CHECK(has(props[3].tValue.iColor) && has(props[4].tValue.iColor) && has(props[5].tValue.iColor),"focus selection caret colors");
	memset(&find,0,sizeof(find)); find.iSize=sizeof(find); find.sPattern="default";
	CHECK(xuiRichEditFindNext(edit,&find)==XUI_OK,"find");
	colorCount=0;
	CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK,"find render");
	CHECK(has(props[6].tValue.iColor) && has(props[7].tValue.iColor),"both find colors");
	props[0].tValue.iColor=0x21a465ff;
	CHECK(xuiStyleSetDefault(ctx,props,18)==XUI_OK,"runtime change");
	colorCount=0;
	CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK && has(props[0].tValue.iColor),"cached text follows live theme");
	props[0]=property("richedit.background.color",0);
	memset(&typeStyle,0,sizeof(typeStyle)); typeStyle.iSize=sizeof(typeStyle); typeStyle.pProperties=props; typeStyle.iPropertyCount=1;
	CHECK(xuiStyleSetType(ctx,xuiRichEditGetType(ctx),&typeStyle)==XUI_OK,"transparent type override");
	colorCount=0;
	CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK && !has(0x311020ffu),"transparent override removes background fill");
	CHECK(xuiStyleRemoveType(ctx,xuiRichEditGetType(ctx))==XUI_OK,"remove type");
	CHECK(xuiStyleClearDefault(ctx)==XUI_OK,"clear global style");
	colorCount=0;
	CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK && has(desc.iTextColor) && has(desc.iBackgroundColor),"clear restores API colors");
	CHECK(has(0x145caaffu),"clear restores default link color");
	CHECK(xuiRichDocumentGetVersion(doc)==version,"document unchanged after all style transitions");
	printf("rich edit global style colors passed\n");
cleanup:
	if(ctx) xuiDestroy(ctx);
	if(doc) xuiRichDocumentDestroy(doc);
	if(target) proxy.tProxy.surfaceDestroy(&proxy.tProxy,target);
	if(font) proxy.tProxy.fontDestroy(&proxy.tProxy,font);
	return failed;
}
