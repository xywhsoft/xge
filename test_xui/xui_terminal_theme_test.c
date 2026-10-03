#include "xui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>

#define CHECK(e) do { if (!(e)) { printf("terminal theme line %d: %s\n", __LINE__, #e); failed=1; goto cleanup; } } while (0)
static uint32_t drawn[512];
static int count;
static xui_draw_text_proc baseText;
static int text(xui_proxy p, xui_draw_context d, const xui_text_item_t* pTextItem, xui_rect_t r, uint32_t c, uint32_t flags)
{
    if(count<512) drawn[count++]=c;
    return baseText(p, d, pTextItem, r, c, flags);
}
static int has(uint32_t color)
{ for(int i=0;i<count;i++) if(drawn[i]==color) return 1; return 0; }
static int write_text(xui_widget terminal,const char* value)
{
	int result=xuiTerminalWriteText(terminal,value);
	return result==XUI_OK ? xuiTerminalFlush(terminal) : result;
}
static int cell_is(xui_widget terminal,int x,int y,uint32_t fg,uint32_t bg)
{
	xui_terminal_cell_t cell={0}; cell.iSize=sizeof(cell);
	if(xuiTerminalGetCell(terminal,x,y,&cell)!=XUI_OK) return 0;
	if(cell.iFgColor==fg && cell.iBgColor==bg) return 1;
	printf("cell %d,%d: %08x/%08x expected %08x/%08x\n",x,y,cell.iFgColor,cell.iBgColor,fg,bg);
	return 0;
}
static int theme(xui_widget terminal,uint32_t fg,uint32_t bg,uint32_t red)
{
	const char* keys[]={"terminal.foreground.color","terminal.background.color","terminal.palette.1"};
	uint32_t colors[]={fg,bg,red};
	xui_style_property_t properties[3]={0};
	for(int i=0;i<3;i++) {
		properties[i].iSize=sizeof(properties[i]); properties[i].sName=keys[i];
		properties[i].tValue.iSize=sizeof(properties[i].tValue);
		properties[i].tValue.iType=XUI_STYLE_VALUE_COLOR; properties[i].tValue.iColor=colors[i];
	}
	return xuiWidgetSetInlineStyle(terminal,properties,3);
}
int main(void)
{
	xui_context ctx=NULL; xui_widget terminal=NULL; xui_font font=NULL;
	xui_test_proxy_state_t proxy; xui_terminal_desc_t desc={0};
	char before[1024],after[1024];
	int failed=0;
	const uint32_t oldfg=0x112233ff,oldbg=0x223344ff,oldred=0x334455ff;
	const uint32_t fg=0xaabbccff,bg=0xddeeffff,red=0x778899ff;
	xuiTestProxyInit(&proxy); baseText=proxy.tProxy.drawText; proxy.tProxy.drawText=text;
	CHECK(xuiCreate(&ctx)==XUI_OK && xuiSetProxy(ctx,&proxy.tProxy)==XUI_OK);
	CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK);
	CHECK(xuiSetDefaultFont(ctx,font)==XUI_OK && xuiInputViewport(ctx,324,56)==XUI_OK);
	desc.iSize=sizeof(desc); desc.pFont=font; desc.iColumns=40; desc.iRows=3;
	desc.fCellWidth=8; desc.fCellHeight=16; desc.fPadding=4; desc.iScrollbackLimit=64;
	CHECK(xuiTerminalCreate(ctx,&terminal,&desc)==XUI_OK);
	CHECK(xuiSetRootWidget(ctx,terminal)==XUI_OK && xuiWidgetSetRect(terminal,(xui_rect_t){0,0,324,56})==XUI_OK);
	CHECK(xuiLayout(ctx)==XUI_OK && xuiTerminalResize(terminal,40,3)==XUI_OK);
	CHECK(theme(terminal,oldfg,oldbg,oldred)==XUI_OK);
	CHECK(write_text(terminal,"D\x1b[31;41mI\x1b[38;2;51;68;85;48;2;34;51;68mT\x1b[38;2;17;34;51mC\x1b[0m\x1b" "7")==XUI_OK);
	CHECK(cell_is(terminal,0,0,oldfg,oldbg) && cell_is(terminal,1,0,oldred,oldred));
	CHECK(theme(terminal,fg,bg,red)==XUI_OK);
	CHECK(cell_is(terminal,0,0,fg,bg) && cell_is(terminal,1,0,red,red));
	CHECK(cell_is(terminal,2,0,oldred,oldbg) && cell_is(terminal,3,0,oldfg,oldbg));
	/* Saved/current defaults retain provenance, including RGB collisions. */
	CHECK(write_text(terminal,"\x1b[31m\x1b" "8S\x1b[31mI")==XUI_OK);
	CHECK(cell_is(terminal,4,0,fg,bg) && cell_is(terminal,5,0,red,bg));
	CHECK(theme(terminal,oldfg,oldbg,oldred)==XUI_OK);
	CHECK(write_text(terminal,"J\x1b[38;5;42;48;5;42mX\x1b[0m")==XUI_OK);
	CHECK(cell_is(terminal,6,0,oldred,oldbg));
	CHECK(xuiTerminalSetPalette(terminal,42,0xabcdefFF)==XUI_OK);
	CHECK(cell_is(terminal,7,0,0xabcdefFF,0xabcdefFF));
	/* Main screen, alternate screen and blanks all follow the live theme. */
	CHECK(write_text(terminal,"\x1b[?1049hA")==XUI_OK);
	CHECK(theme(terminal,fg,bg,red)==XUI_OK && cell_is(terminal,0,0,fg,bg));
	CHECK(write_text(terminal,"\x1b[?1049l")==XUI_OK);
	CHECK(cell_is(terminal,0,0,fg,bg) && cell_is(terminal,20,0,fg,bg));
	/* Force the colored row into compressed history; repaint, do not replay. */
	CHECK(write_text(terminal,"\r\nnext\r\nlast\r\nend")==XUI_OK);
	xui_terminal_stats_t stats={0}; stats.iSize=sizeof(stats);
	CHECK(xuiTerminalGetStats(terminal,&stats)==XUI_OK && stats.iHistoryDisplayRows>0);
	CHECK(xuiTerminalSerializeText(terminal,before,sizeof(before))>0);
	CHECK(xuiScrollModelSetOffset(xuiTerminalGetScrollModel(terminal),0,0)==XUI_OK);
	for(int ligatures=0;ligatures<2;ligatures++) {
		CHECK(xuiTerminalSetLigaturesEnabled(terminal,ligatures)==XUI_OK);
		CHECK(theme(terminal,fg,bg,red)==XUI_OK);
		count=0; CHECK(xuiRenderPrepare(ctx)==XUI_OK);
		CHECK(has(fg) && has(red) && has(oldred) && has(oldfg));
		CHECK(theme(terminal,oldfg,oldbg,oldred)==XUI_OK);
		count=0; CHECK(xuiRenderPrepare(ctx)==XUI_OK);
		CHECK(has(oldfg) && has(oldred));
	}
	CHECK(xuiTerminalSerializeText(terminal,after,sizeof(after))>0 && !strcmp(before,after));
	CHECK(xuiTerminalResize(terminal,40,8)==XUI_OK && theme(terminal,fg,bg,red)==XUI_OK);
	CHECK(cell_is(terminal,0,0,fg,bg) && cell_is(terminal,1,0,red,red) && cell_is(terminal,2,0,oldred,oldbg));
	puts("terminal live theme passed: defaults, indexed, RGB collisions, saved cursor, alt screen, history, resize and both render paths");
cleanup:
	if(ctx) xuiDestroy(ctx);
	if(font) proxy.tProxy.fontDestroy(&proxy.tProxy,font);
	return failed;
}
