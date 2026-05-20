#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <string.h>

static const char g_sAppLayoutXson[] =
	"{"
	"  \"xui\": 1,"
	"  \"tokens\": {"
	"    \"colors\": { \"shell\": \"#15191F\", \"panel\": \"#202631\", \"rail\": \"#10141A\", \"rule\": \"#3A4250\", \"accent\": \"#3277FF\", \"text\": \"#EEF4FF\" },"
	"    \"spacing\": { \"pad\": 10, \"gap\": 8, \"headerH\": 48, \"railW\": 72, \"statusH\": 28, \"contentW\": 480, \"contentH\": 420 }"
	"  },"
	"  \"styles\": {"
	"    \"shell\": { \"layout\": \"dock\", \"width\": \"100%\", \"height\": \"100%\", \"padding\": \"@spacing.pad\", \"gap\": \"@spacing.gap\", \"background\": \"@colors.shell\" },"
	"    \"header\": { \"layout\": \"row\", \"dock\": \"top\", \"height\": \"@spacing.headerH\", \"alignY\": \"center\", \"justify\": \"space-between\", \"padding\": [8, 6], \"background\": \"@colors.panel\" },"
	"    \"rail\": { \"layout\": \"column\", \"dock\": \"left\", \"width\": \"@spacing.railW\", \"gap\": 6, \"padding\": 6, \"background\": \"@colors.rail\" },"
	"    \"content\": { \"dock\": \"fill\", \"width\": \"grow\", \"height\": \"grow\", \"padding\": 6, \"contentSize\": [\"@spacing.contentW\", \"@spacing.contentH\"], \"backgroundColor\": \"@colors.panel\", \"barColor\": \"@colors.rule\", \"thumbColor\": \"@colors.accent\" },"
	"    \"status\": { \"layout\": \"row\", \"dock\": \"bottom\", \"height\": \"@spacing.statusH\", \"alignY\": \"center\", \"padding\": [8, 4], \"background\": \"@colors.panel\" },"
	"    \"title\": { \"textColor\": \"@colors.text\" },"
	"    \"action\": { \"width\": 92, \"height\": 28, \"color\": \"@colors.accent\", \"textColor\": \"@colors.text\" },"
	"    \"rule\": { \"orientation\": \"horizontal\", \"thickness\": 1, \"color\": \"@colors.rule\" }"
	"  },"
	"  \"tree\": {"
	"    \"type\": \"dock\", \"id\": \"app\", \"style\": \"shell\","
	"    \"children\": ["
	"      { \"type\": \"row\", \"id\": \"header\", \"style\": \"header\", \"children\": ["
	"        { \"type\": \"label\", \"id\": \"title\", \"style\": \"title\", \"text\": \"Dashboard\" },"
	"        { \"type\": \"button\", \"id\": \"refresh\", \"style\": \"action\", \"text\": \"Refresh\" }"
	"      ] },"
	"      { \"type\": \"column\", \"id\": \"rail\", \"style\": \"rail\", \"children\": ["
	"        { \"type\": \"button\", \"id\": \"nav-a\", \"style\": \"action\", \"text\": \"A\" },"
	"        { \"type\": \"button\", \"id\": \"nav-b\", \"style\": \"action\", \"text\": \"B\" }"
	"      ] },"
	"      { \"type\": \"scrollView\", \"id\": \"content\", \"style\": \"content\", \"children\": ["
	"        { \"type\": \"grid\", \"id\": \"cards\", \"gridColumns\": 2, \"rowHeight\": 64, \"gap\": 8, \"children\": ["
	"          { \"type\": \"panel\", \"id\": \"card-a\", \"background\": \"@colors.rail\" },"
	"          { \"type\": \"panel\", \"id\": \"card-b\", \"background\": \"@colors.rail\" }"
	"        ] }"
	"      ] },"
	"      { \"type\": \"row\", \"id\": \"status\", \"style\": \"status\", \"children\": ["
	"        { \"type\": \"separator\", \"id\": \"status-rule\", \"style\": \"rule\", \"width\": \"100%\" }"
	"      ] }"
	"    ]"
	"  }"
	"}";

int main(void)
{
	xge_xui_context_t tXui;
	static xge_xui_page_t tPage;
	xge_xui_widget pApp;
	xge_xui_widget pHeader;
	xge_xui_widget pRail;
	xge_xui_widget pContent;
	xge_xui_widget pStatus;
	xge_xui_widget pCards;
	xge_xui_scroll_view pScroll;
	xge_rect_t tRect;
	int iRet;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tPage, 0, sizeof(tPage));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		printf("xgeXuiInit failed\n");
		return 1;
	}
	iRet = xgeXuiPageLoadMemory(&tXui, g_sAppLayoutXson, (int)strlen(g_sAppLayoutXson), NULL, &tPage);
	if ( iRet != XGE_OK ) {
		printf("xgeXuiPageLoadMemory failed: %s\n", xgeXuiPageGetError(&tPage));
		xgeXuiUnit(&tXui);
		return 2;
	}
	pApp = xgeXuiPageFind(&tPage, "app");
	pHeader = xgeXuiPageFind(&tPage, "header");
	pRail = xgeXuiPageFind(&tPage, "rail");
	pContent = xgeXuiPageFind(&tPage, "content");
	pStatus = xgeXuiPageFind(&tPage, "status");
	pCards = xgeXuiPageFind(&tPage, "cards");
	if ( (pApp == NULL) || (pHeader == NULL) || (pRail == NULL) || (pContent == NULL) || (pStatus == NULL) || (pCards == NULL) ) {
		printf("app layout lookup failed\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 3;
	}
	if ( (pApp->tStyle.iLayout != XGE_XUI_LAYOUT_DOCK) || (pHeader->tStyle.iDock != XGE_XUI_DOCK_TOP) || (pRail->tStyle.iDock != XGE_XUI_DOCK_LEFT) || (pContent->tStyle.iDock != XGE_XUI_DOCK_FILL) || (pStatus->tStyle.iDock != XGE_XUI_DOCK_BOTTOM) ) {
		printf("dock role verification failed\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 4;
	}
	pScroll = (xge_xui_scroll_view)pContent->pUser;
	if ( (pScroll == NULL) || (pScroll->tModel.fContentW != 480.0f) || (pScroll->tModel.fContentH != 420.0f) || (pCards->tStyle.iGridColumns != 2) || (pCards->tStyle.fGridRowHeight != 64.0f) ) {
		printf("content scroll/grid verification failed\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 5;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 640.0f;
	tRect.fH = 360.0f;
	xgeXuiWidgetSetRect(pApp, tRect);
	xgeXuiUpdate(&tXui, 0.0f);
	if ( xgeXuiWidgetGetRect(pHeader).fH != 48.0f || xgeXuiWidgetGetRect(pRail).fW != 72.0f || xgeXuiWidgetGetRect(pStatus).fH != 28.0f || xgeXuiWidgetGetRect(pContent).fW <= 0.0f || xgeXuiWidgetGetRect(pContent).fH <= 0.0f ) {
		printf("app layout geometry verification failed\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 6;
	}
	printf("xui_xson_app_layout_lab ok: dock shell, scroll content, and status layout verified\n");
	xgeXuiPageUnload(&tPage);
	xgeXuiUnit(&tXui);
	return 0;
}
