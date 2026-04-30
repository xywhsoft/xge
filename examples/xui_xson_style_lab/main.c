#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <string.h>

static const char g_sStyleXson[] =
	"{"
	"  \"xui\": 1,"
	"  \"tokens\": {"
	"    \"colors\": { \"panel\": \"#101820\", \"accent\": \"#2A7FFF\", \"text\": \"#F0F4FF\" },"
	"    \"spacing\": { \"pad\": 12, \"gap\": 6, \"toolbarH\": 44 }"
	"  },"
	"  \"styles\": {"
	"    \"fill\": { \"width\": \"100%\", \"height\": \"100%\" },"
	"    \"panel\": { \"@parent\": \"fill\", \"layout\": \"column\", \"padding\": \"@spacing.pad\", \"gap\": \"@spacing.gap\", \"background\": \"@colors.panel\", \"radius\": 4 },"
	"    \"toolbar\": { \"@parent\": \"panel\", \"layout\": \"row\", \"height\": \"@spacing.toolbarH\", \"alignY\": \"center\", \"justify\": \"space-between\" },"
	"    \"action\": { \"color\": \"@colors.accent\", \"textColor\": \"@colors.text\", \"width\": 96, \"height\": 28 }"
	"  },"
	"  \"tree\": {"
	"    \"type\": \"column\","
	"    \"id\": \"root\","
	"    \"style\": \"panel\","
	"    \"children\": ["
	"      { \"type\": \"row\", \"id\": \"toolbar\", \"style\": \"toolbar\", \"padding\": [4, 5, 6, 7] },"
	"      { \"type\": \"button\", \"id\": \"action\", \"style\": \"action\", \"text\": \"Apply\" }"
	"    ]"
	"  }"
	"}";

int main(void)
{
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_xui_widget pRoot;
	xge_xui_widget pToolbar;
	xge_xui_widget pAction;
	xge_xui_button pButton;
	int iRet;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tPage, 0, sizeof(tPage));
	iRet = xgeXuiInit(&tXui);
	if ( iRet != XGE_OK ) {
		printf("xgeXuiInit failed: %d\n", iRet);
		return 1;
	}
	iRet = xgeXuiPageLoadMemory(&tXui, g_sStyleXson, (int)strlen(g_sStyleXson), NULL, &tPage);
	if ( iRet != XGE_OK ) {
		printf("xgeXuiPageLoadMemory failed: %s\n", xgeXuiPageGetError(&tPage));
		xgeXuiUnit(&tXui);
		return 2;
	}
	pRoot = xgeXuiPageFind(&tPage, "root");
	pToolbar = xgeXuiPageFind(&tPage, "toolbar");
	pAction = xgeXuiPageFind(&tPage, "action");
	if ( (pRoot == NULL) || (pToolbar == NULL) || (pAction == NULL) ) {
		printf("style lab lookup failed\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 3;
	}
	pButton = (xge_xui_button)pAction->pUser;
	if ( pButton == NULL ) {
		printf("style lab button control missing\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 4;
	}
	if ( (pRoot->tStyle.iLayout != XGE_XUI_LAYOUT_COLUMN) || (pRoot->tStyle.tPadding.tLeft.fValue != 12.0f) || (pRoot->tStyle.fGap != 6.0f) || (pRoot->tStyle.iBackgroundColor != XGE_COLOR_RGBA(0x10, 0x18, 0x20, 0xFF)) ) {
		printf("root inherited style verification failed\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 5;
	}
	if ( (pToolbar->tStyle.iLayout != XGE_XUI_LAYOUT_ROW) || (pToolbar->tStyle.tHeight.fValue != 44.0f) || (pToolbar->tStyle.iJustify != XGE_XUI_JUSTIFY_SPACE_BETWEEN) || (pToolbar->tStyle.tPadding.tLeft.fValue != 4.0f) || (pToolbar->tStyle.tPadding.tBottom.fValue != 7.0f) ) {
		printf("toolbar parent/inline style verification failed\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 6;
	}
	if ( (pAction->tStyle.tWidth.fValue != 96.0f) || (pAction->tStyle.tHeight.fValue != 28.0f) || (pButton->iColorNormal != XGE_COLOR_RGBA(0x2A, 0x7F, 0xFF, 0xFF)) || (pButton->iTextColor != XGE_COLOR_RGBA(0xF0, 0xF4, 0xFF, 0xFF)) ) {
		printf("button style verification failed\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 7;
	}
	printf("xui_xson_style_lab ok: inherited tokens and inline overrides verified\n");
	xgeXuiPageUnload(&tPage);
	xgeXuiUnit(&tXui);
	return 0;
}
