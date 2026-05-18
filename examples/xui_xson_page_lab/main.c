#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <string.h>

static const char g_sPageXson[] =
	"{"
	"  \"xui\": 1,"
	"  \"name\": \"xson-page-lab\","
	"  \"styles\": {"
	"    \"fill\": { \"width\": \"100%\", \"height\": \"100%\" },"
	"    \"panel\": { \"@parent\": \"fill\", \"layout\": \"column\", \"gap\": 8, \"background\": \"#202830\" },"
	"    \"row\": { \"layout\": \"row\", \"height\": 48, \"alignY\": \"center\" }"
	"  },"
	"  \"tree\": {"
	"    \"type\": \"column\","
	"    \"id\": \"root\","
	"    \"style\": \"panel\","
	"    \"children\": ["
	"      { \"type\": \"row\", \"name\": \"toolbar\", \"style\": \"row\", \"width\": \"100%\" },"
	"      { \"type\": \"grid\", \"name\": \"content\", \"style\": \"fill\", \"gridColumns\": 2, \"gap\": 6 }"
	"    ]"
	"  }"
	"}";

int main(void)
{
	xge_xui_context_t tXui;
	static xge_xui_page_t tPage;
	xge_xui_widget pRoot;
	xge_xui_widget pToolbar;
	xge_xui_widget pContent;
	int iRet;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tPage, 0, sizeof(tPage));
	iRet = xgeXuiInit(&tXui);
	if ( iRet != XGE_OK ) {
		printf("xgeXuiInit failed: %d\n", iRet);
		return 1;
	}
	iRet = xgeXuiPageLoadMemory(&tXui, g_sPageXson, (int)strlen(g_sPageXson), NULL, &tPage);
	if ( iRet != XGE_OK ) {
		printf("xgeXuiPageLoadMemory failed: %s\n", xgeXuiPageGetError(&tPage));
		xgeXuiUnit(&tXui);
		return 2;
	}
	pRoot = xgeXuiPageRoot(&tPage);
	pToolbar = xgeXuiPageFind(&tPage, "toolbar");
	pContent = xgeXuiPageFind(&tPage, "content");
	if ( (pRoot == NULL) || (pToolbar == NULL) || (pContent == NULL) ) {
		printf("page lookup failed\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 3;
	}
	if ( (pRoot->tStyle.iLayout != XGE_XUI_LAYOUT_COLUMN) || (pToolbar->tStyle.tHeight.fValue != 48.0f) || (pContent->tStyle.iGridColumns != 2) ) {
		printf("page style verification failed\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 4;
	}
	printf("xui_xson_page_lab ok: root=%s children=%s,%s\n", pRoot->sName, pToolbar->sName, pContent->sName);
	xgeXuiPageUnload(&tPage);
	xgeXuiUnit(&tXui);
	return 0;
}
