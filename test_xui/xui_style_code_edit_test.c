#include "xui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>

#define CHECK(e,m) do { if(!(e)) { printf("code editor style: %s (%d)\n",m,__LINE__); failed=1; goto cleanup; } } while(0)
static uint32_t colors[4096];
static int count;
static int (*baseText)(xui_proxy,xui_draw_context,xui_font,const char*,xui_rect_t,uint32_t,uint32_t);
static int (*baseFill)(xui_proxy,xui_draw_context,xui_rect_t,uint32_t);
static int has(uint32_t c) { int i; for(i=0;i<count;i++) if(colors[i]==c) return 1; return 0; }
static int text(xui_proxy p,xui_draw_context d,xui_font f,const char* s,xui_rect_t r,uint32_t c,uint32_t flags)
{ if(count<4096) colors[count++]=c; return baseText(p,d,f,s,r,c,flags); }
static int fill(xui_proxy p,xui_draw_context d,xui_rect_t r,uint32_t c)
{ if(count<4096) colors[count++]=c; return baseFill(p,d,r,c); }
static int signature(xui_widget w,int offset,xui_code_signature_help_t* help,void* user)
{
	static xui_code_signature_parameter_t parameter;
	(void)w; (void)offset; (void)user;
	memset(&parameter,0,sizeof(parameter)); parameter.iSize=sizeof(parameter);
	parameter.sLabel="value"; parameter.tLabelRange=(xui_code_range_t){5,10};
	help->iSize=sizeof(*help); help->sLabel="call(value)"; help->sDocumentation="Documentation";
	help->pParameters=&parameter; help->iParameterCount=1; help->iActiveParameter=0;
	return XUI_OK;
}
int main(void)
{
	static const char* const keys[]={"codeedit.margin.background_color","codeedit.line_number.color",
		"codeedit.line_number.active_color","codeedit.line_number.active_background_color",
		"codeedit.marker.color","codeedit.fold.color","codeedit.assist.active_color",
		"codeedit.assist.text.color","codeedit.assist.documentation.color","codeedit.diagnostic.info.color"};
	xui_test_proxy_state_t proxy;
	xui_context ctx=NULL;
	xui_surface target=NULL;
	xui_font font=NULL;
	xui_widget edit=NULL;
	xui_code_edit_desc_t desc;
	xui_code_fold_range_t fold;
	xui_code_diagnostic_t diagnostic;
	xui_style_property_t props[10];
	xui_rect_i_t damage={0,0,640,480};
	int i,failed=0;
	xuiTestProxyInit(&proxy);
	baseText=proxy.tProxy.drawText; proxy.tProxy.drawText=text;
	baseFill=proxy.tProxy.drawRectFill; proxy.tProxy.drawRectFill=fill;
	CHECK(xuiCreate(&ctx)==XUI_OK && xuiSetProxy(ctx,&proxy.tProxy)==XUI_OK,"context");
	CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK,"font");
	CHECK(xuiSetDefaultFont(ctx,font)==XUI_OK && xuiInputViewport(ctx,640,480)==XUI_OK,"viewport");
	CHECK(xuiTestSurfaceCreate(&proxy,&target,640,480,XUI_SURFACE_USAGE_TARGET)==XUI_OK,"target");
	memset(&desc,0,sizeof(desc)); desc.iSize=sizeof(desc); desc.pFont=font;
	desc.sText="int main(void) {\n return 0;\n}\n"; desc.sLanguage="c";
	desc.bShowLineNumbers=1; desc.bShowFoldMargin=1; desc.bShowMarkerMargin=1; desc.bShowDiagnosticMargin=1;
	CHECK(xuiCodeEditCreate(ctx,&edit,&desc)==XUI_OK,"editor");
	CHECK(xuiSetRootWidget(ctx,edit)==XUI_OK && xuiWidgetSetRect(edit,(xui_rect_t){0,0,640,480})==XUI_OK,"root");
	CHECK(xuiCodeAnnotationSetMarker(xuiCodeEditGetAnnotations(edit),1,XUI_CODE_MARKER_BOOKMARK,0,"mark",0)==XUI_OK,"marker");
	memset(&fold,0,sizeof(fold)); fold.iSize=sizeof(fold); fold.iStartLine=0; fold.iEndLine=2; fold.iFlags=XUI_CODE_FOLD_HEADER;
	CHECK(xuiCodeFoldStateSetRanges(xuiCodeEditGetFoldState(edit),&fold,1)==XUI_OK,"fold");
	memset(&diagnostic,0,sizeof(diagnostic)); diagnostic.iSize=sizeof(diagnostic);
	diagnostic.tRange=(xui_code_range_t){0,3}; diagnostic.iSeverity=XUI_CODE_DIAGNOSTIC_INFO; diagnostic.sMessage="Info";
	CHECK(xuiCodeAnnotationSetDiagnostics(xuiCodeEditGetAnnotations(edit),&diagnostic,1)==XUI_OK,"diagnostic");
	CHECK(xuiSetFocusWidget(ctx,edit)==XUI_OK && xuiLayout(ctx)==XUI_OK,"focus layout");
	for(i=0;i<10;i++) {
		CHECK(xuiStyleFindProperty(ctx,keys[i])!=0,keys[i]);
		memset(&props[i],0,sizeof(props[i])); props[i].iSize=sizeof(props[i]); props[i].sName=keys[i];
		props[i].tValue.iSize=sizeof(props[i].tValue); props[i].tValue.iType=XUI_STYLE_VALUE_COLOR;
		props[i].tValue.iColor=0x621324ffu+((uint32_t)i<<16);
	}
	CHECK(xuiStyleSetDefault(ctx,props,10)==XUI_OK,"default stylesheet");
	CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK,"render margins");
	for(i=0;i<6;i++) CHECK(has(props[i].tValue.iColor),keys[i]);
	CHECK(has(props[9].tValue.iColor),"diagnostic margin preserves info severity theme");
	CHECK(xuiCodeProviderSetSignature(xuiCodeEditGetProviders(edit),signature,NULL)==XUI_OK,"provider");
	CHECK(xuiCodeEditRequestSignatureHelp(edit)==XUI_OK && xuiCodeEditIsSignatureHelpOpen(edit),"open assist");
	count=0;
	CHECK(xuiLayout(ctx)==XUI_OK && xuiRender(ctx,target,&damage,1)==XUI_OK,"assist render");
	for(i=6;i<9;i++) CHECK(has(props[i].tValue.iColor),keys[i]);
	count=0;
	CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK && count==0,"warm editor and assist keep paint caches");
	props[7].tValue.iColor=0xb72745ffu;
	CHECK(xuiStyleSetDefault(ctx,props,10)==XUI_OK,"live theme");
	count=0;
	CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK,"live assist render without update");
	CHECK(has(props[7].tValue.iColor) && xuiCodeEditIsSignatureHelpOpen(edit),"open cached assist follows owner theme");
	props[0].tValue.iColor=0; props[6].tValue.iColor=0;
	CHECK(xuiStyleSetDefault(ctx,props,10)==XUI_OK,"transparent margins and assist");
	count=0;
	CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK && !has(0x621324ffu) && !has(0x681324ffu),"transparent colors suppress old fills without update");
	CHECK(xuiStyleClearDefault(ctx)==XUI_OK,"clear");
	count=0;
	CHECK(xuiRender(ctx,target,&damage,1)==XUI_OK,"restore render without update");
	CHECK(has(XUI_COLOR_RGBA(15,23,42,255)) && has(XUI_COLOR_RGBA(71,85,105,255)),"assist restores defaults");
	CHECK(has(XUI_COLOR_RGBA(241,245,249,255)),"margin restores defaults");
	puts("code editor global style colors passed");
cleanup:
	if(ctx) xuiDestroy(ctx);
	if(target) proxy.tProxy.surfaceDestroy(&proxy.tProxy,target);
	if(font) proxy.tProxy.fontDestroy(&proxy.tProxy,font);
	return failed;
}
