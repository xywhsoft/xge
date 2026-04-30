#include "../../xge.h"
#include "../xui_demo_style.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static const char g_sLayoutGalleryXson[] =
	"{"
	"  \"xui\": 1,"
	"  \"tokens\": {"
	"    \"colors\": {"
	"      \"root\": \"#181E2A\", \"gallery\": \"#121822\", \"column\": \"#1A202C\","
	"      \"absolute\": \"#283A54\", \"row\": \"#223830\", \"metric\": \"#282C38\","
	"      \"blue\": \"#5CA4FF\", \"green\": \"#56D496\", \"red\": \"#EC6E84\", \"gold\": \"#F6BA5C\""
	"    },"
	"    \"spacing\": { \"rootPad\": 16, \"rootGap\": 10, \"panelPad\": 8, \"panelGap\": 10, \"columnGap\": 6 }"
	"  },"
	"  \"styles\": {"
	"    \"panel\": { \"radius\": 6 },"
	"    \"root\": { \"@parent\": \"panel\", \"layout\": \"column\", \"width\": \"100%\", \"height\": \"100%\", \"padding\": \"@spacing.rootPad\", \"gap\": \"@spacing.rootGap\", \"background\": \"@colors.root\" },"
	"    \"gallery\": { \"@parent\": \"panel\", \"layout\": \"row\", \"width\": \"grow\", \"height\": \"grow\", \"padding\": \"@spacing.panelPad\", \"gap\": 12, \"background\": \"@colors.gallery\" },"
	"    \"column\": { \"@parent\": \"panel\", \"layout\": \"column\", \"width\": \"grow\", \"height\": \"grow\", \"padding\": 6, \"gap\": \"@spacing.panelGap\", \"background\": \"@colors.column\" },"
	"    \"absolutePanel\": { \"@parent\": \"panel\", \"layout\": \"absolute\", \"width\": \"grow\", \"height\": 120, \"padding\": \"@spacing.panelPad\", \"background\": \"@colors.absolute\" },"
	"    \"rowPanel\": { \"@parent\": \"panel\", \"layout\": \"row\", \"width\": \"grow\", \"height\": 84, \"padding\": \"@spacing.panelPad\", \"gap\": 10, \"background\": \"@colors.row\" },"
	"    \"columnPanel\": { \"@parent\": \"panel\", \"layout\": \"column\", \"width\": \"grow\", \"height\": 118, \"padding\": \"@spacing.panelPad\", \"gap\": \"@spacing.columnGap\", \"background\": \"#382A48\" },"
	"    \"stackPanel\": { \"@parent\": \"panel\", \"layout\": \"stack\", \"width\": \"grow\", \"height\": 92, \"padding\": \"@spacing.panelPad\", \"background\": \"#3C3026\" },"
	"    \"justifyPanel\": { \"@parent\": \"panel\", \"layout\": \"row\", \"width\": \"grow\", \"height\": 70, \"padding\": \"@spacing.panelPad\", \"gap\": 4, \"justify\": \"space-between\", \"background\": \"#222C3E\" },"
	"    \"gridPanel\": { \"@parent\": \"panel\", \"layout\": \"grid\", \"width\": \"grow\", \"height\": 108, \"padding\": \"@spacing.panelPad\", \"gridColumns\": 2, \"rowHeight\": 28, \"columnGap\": 8, \"rowGap\": 6, \"background\": \"#2E3626\" },"
	"    \"metricsPanel\": { \"@parent\": \"panel\", \"layout\": \"row\", \"width\": \"grow\", \"height\": 78, \"padding\": \"@spacing.panelPad\", \"gap\": 8, \"background\": \"@colors.metric\" },"
	"    \"tileA\": { \"@parent\": \"panel\", \"background\": \"@colors.blue\" },"
	"    \"tileB\": { \"@parent\": \"panel\", \"background\": \"@colors.gold\" },"
	"    \"tileC\": { \"@parent\": \"panel\", \"background\": \"@colors.green\" },"
	"    \"tileD\": { \"@parent\": \"panel\", \"background\": \"@colors.red\" }"
	"  },"
	"  \"tree\": {"
	"    \"type\": \"column\", \"id\": \"root\", \"style\": \"root\","
	"    \"children\": ["
	"      { \"type\": \"row\", \"id\": \"gallery\", \"style\": \"gallery\", \"children\": ["
	"        { \"type\": \"column\", \"id\": \"left\", \"style\": \"column\", \"children\": ["
	"          { \"type\": \"absolute\", \"id\": \"absolute\", \"style\": \"absolutePanel\", \"children\": ["
	"            { \"type\": \"panel\", \"id\": \"absolute-a\", \"style\": \"tileA\", \"width\": 72, \"height\": 28, \"anchor\": { \"left\": 14, \"top\": 16 } },"
	"            { \"type\": \"panel\", \"id\": \"absolute-b\", \"style\": \"tileB\", \"width\": 88, \"height\": 34, \"anchor\": { \"left\": 110, \"top\": 54 } }"
	"          ] },"
	"          { \"type\": \"row\", \"id\": \"row\", \"style\": \"rowPanel\", \"children\": ["
	"            { \"type\": \"panel\", \"id\": \"row-a\", \"style\": \"tileC\", \"width\": 72, \"height\": 24, \"alignY\": \"center\" },"
	"            { \"type\": \"panel\", \"id\": \"row-b\", \"style\": \"tileD\", \"width\": 84, \"height\": 28, \"alignY\": \"end\" }"
	"          ] },"
	"          { \"type\": \"column\", \"id\": \"column\", \"style\": \"columnPanel\", \"children\": ["
	"            { \"type\": \"panel\", \"id\": \"column-a\", \"style\": \"tileA\", \"width\": \"60%\", \"height\": 24, \"alignX\": \"end\" },"
	"            { \"type\": \"panel\", \"id\": \"column-b\", \"style\": \"tileB\", \"width\": 96, \"height\": 30, \"alignX\": \"center\" }"
	"          ] }"
	"        ] },"
	"        { \"type\": \"column\", \"id\": \"right\", \"style\": \"column\", \"children\": ["
	"          { \"type\": \"stack\", \"id\": \"stack\", \"style\": \"stackPanel\", \"children\": ["
	"            { \"type\": \"panel\", \"id\": \"stack-back\", \"style\": \"tileA\", \"width\": \"100%\", \"height\": \"100%\" },"
	"            { \"type\": \"panel\", \"id\": \"stack-front\", \"style\": \"tileB\", \"width\": 96, \"height\": 40, \"align\": \"center\" }"
	"          ] },"
	"          { \"type\": \"row\", \"id\": \"justify\", \"style\": \"justifyPanel\", \"children\": ["
	"            { \"type\": \"panel\", \"id\": \"justify-a\", \"style\": \"tileA\", \"width\": 28, \"height\": 28, \"alignY\": \"center\" },"
	"            { \"type\": \"panel\", \"id\": \"justify-b\", \"style\": \"tileC\", \"width\": 28, \"height\": 28, \"alignY\": \"center\" },"
	"            { \"type\": \"panel\", \"id\": \"justify-c\", \"style\": \"tileD\", \"width\": 28, \"height\": 28, \"alignY\": \"center\" }"
	"          ] },"
	"          { \"type\": \"grid\", \"id\": \"grid\", \"style\": \"gridPanel\", \"children\": ["
	"            { \"type\": \"panel\", \"id\": \"grid-a\", \"style\": \"tileA\", \"align\": \"stretch\" },"
	"            { \"type\": \"panel\", \"id\": \"grid-b\", \"style\": \"tileB\", \"align\": \"stretch\" },"
	"            { \"type\": \"panel\", \"id\": \"grid-c\", \"style\": \"tileC\", \"align\": \"stretch\" },"
	"            { \"type\": \"panel\", \"id\": \"grid-d\", \"style\": \"tileD\", \"align\": \"stretch\" }"
	"          ] },"
	"          { \"type\": \"row\", \"id\": \"metrics\", \"style\": \"metricsPanel\", \"children\": ["
	"            { \"type\": \"label\", \"id\": \"content\", \"style\": \"tileA\", \"text\": \"content\", \"padding\": [8, 4] },"
	"            { \"type\": \"panel\", \"id\": \"grow\", \"style\": \"tileC\", \"width\": \"grow\", \"height\": 24, \"alignY\": \"center\" },"
	"            { \"type\": \"panel\", \"id\": \"percent\", \"style\": \"tileB\", \"width\": \"28%\", \"height\": 24, \"alignY\": \"center\" },"
	"            { \"type\": \"panel\", \"id\": \"px\", \"style\": \"tileD\", \"width\": 46, \"height\": 24, \"alignY\": \"center\" }"
	"          ] }"
	"        ] }"
	"      ] }"
	"    ]"
	"  }"
	"}";

static int FloatNear(float fA, float fB, float fEpsilon)
{
	return fabsf(fA - fB) <= fEpsilon;
}

static float RectGapY(xge_rect_t tA, xge_rect_t tB)
{
	return tB.fY - (tA.fY + tA.fH);
}

static int Require(int bOK, const char* sText)
{
	if ( !bOK ) {
		printf("xui_xson_layout_gallery_lab failed: %s\n", sText);
		return 0;
	}
	return 1;
}

int main(void)
{
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_xui_widget pRoot;
	xge_xui_widget pAbsolutePanel;
	xge_xui_widget pAbsoluteA;
	xge_xui_widget pAbsoluteB;
	xge_xui_widget pRowPanel;
	xge_xui_widget pRowA;
	xge_xui_widget pRowB;
	xge_xui_widget pColumnA;
	xge_xui_widget pColumnB;
	xge_xui_widget pStackBack;
	xge_xui_widget pStackFront;
	xge_xui_widget pJustifyPanel;
	xge_xui_widget pJustifyA;
	xge_xui_widget pJustifyB;
	xge_xui_widget pJustifyC;
	xge_xui_widget pGridA;
	xge_xui_widget pGridB;
	xge_xui_widget pGridC;
	xge_xui_widget pGridD;
	xge_xui_widget pContent;
	xge_xui_widget pGrow;
	xge_xui_widget pPercent;
	xge_xui_widget pPx;
	xge_rect_t tRect;
	xge_rect_t tPanel;
	xge_rect_t tA;
	xge_rect_t tB;
	xge_rect_t tC;
	xge_rect_t tD;
	int iRet;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tPage, 0, sizeof(tPage));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		printf("xgeXuiInit failed\n");
		return 1;
	}
	iRet = xgeXuiPageLoadMemory(&tXui, g_sLayoutGalleryXson, (int)strlen(g_sLayoutGalleryXson), NULL, &tPage);
	if ( iRet != XGE_OK ) {
		printf("xgeXuiPageLoadMemory failed: %s\n", xgeXuiPageGetError(&tPage));
		xgeXuiUnit(&tXui);
		return 2;
	}
	pRoot = xgeXuiPageFind(&tPage, "root");
	pAbsolutePanel = xgeXuiPageFind(&tPage, "absolute");
	pAbsoluteA = xgeXuiPageFind(&tPage, "absolute-a");
	pAbsoluteB = xgeXuiPageFind(&tPage, "absolute-b");
	pRowPanel = xgeXuiPageFind(&tPage, "row");
	pRowA = xgeXuiPageFind(&tPage, "row-a");
	pRowB = xgeXuiPageFind(&tPage, "row-b");
	pColumnA = xgeXuiPageFind(&tPage, "column-a");
	pColumnB = xgeXuiPageFind(&tPage, "column-b");
	pStackBack = xgeXuiPageFind(&tPage, "stack-back");
	pStackFront = xgeXuiPageFind(&tPage, "stack-front");
	pJustifyPanel = xgeXuiPageFind(&tPage, "justify");
	pJustifyA = xgeXuiPageFind(&tPage, "justify-a");
	pJustifyB = xgeXuiPageFind(&tPage, "justify-b");
	pJustifyC = xgeXuiPageFind(&tPage, "justify-c");
	pGridA = xgeXuiPageFind(&tPage, "grid-a");
	pGridB = xgeXuiPageFind(&tPage, "grid-b");
	pGridC = xgeXuiPageFind(&tPage, "grid-c");
	pGridD = xgeXuiPageFind(&tPage, "grid-d");
	pContent = xgeXuiPageFind(&tPage, "content");
	pGrow = xgeXuiPageFind(&tPage, "grow");
	pPercent = xgeXuiPageFind(&tPage, "percent");
	pPx = xgeXuiPageFind(&tPage, "px");
	if ( !Require((pRoot != NULL) && (pAbsolutePanel != NULL) && (pAbsoluteA != NULL) && (pAbsoluteB != NULL) && (pRowPanel != NULL) && (pRowA != NULL) && (pRowB != NULL) && (pColumnA != NULL) && (pColumnB != NULL) && (pStackBack != NULL) && (pStackFront != NULL) && (pJustifyPanel != NULL) && (pJustifyA != NULL) && (pJustifyB != NULL) && (pJustifyC != NULL) && (pGridA != NULL) && (pGridB != NULL) && (pGridC != NULL) && (pGridD != NULL) && (pContent != NULL) && (pGrow != NULL) && (pPercent != NULL) && (pPx != NULL), "widget lookup") ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 3;
	}
	tRect.fX = 20.0f;
	tRect.fY = 20.0f;
	tRect.fW = 860.0f;
	tRect.fH = 520.0f;
	xgeXuiWidgetSetRect(pRoot, tRect);
	xgeXuiUpdate(&tXui, 0.0f);

	tPanel = pAbsolutePanel->tContentRect;
	tA = xgeXuiWidgetGetRect(pAbsoluteA);
	tB = xgeXuiWidgetGetRect(pAbsoluteB);
	if ( !Require(FloatNear(tA.fX, tPanel.fX + 14.0f, 0.5f) && FloatNear(tA.fY, tPanel.fY + 16.0f, 0.5f) && FloatNear(tA.fW, 72.0f, 0.5f) && FloatNear(tA.fH, 28.0f, 0.5f) && FloatNear(tB.fX, tPanel.fX + 110.0f, 0.5f) && FloatNear(tB.fY, tPanel.fY + 54.0f, 0.5f), "absolute anchors") ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 4;
	}

	tPanel = pRowPanel->tContentRect;
	tA = xgeXuiWidgetGetRect(pRowA);
	tB = xgeXuiWidgetGetRect(pRowB);
	if ( !Require((tA.fX < tB.fX) && FloatNear(tB.fX - (tA.fX + tA.fW), 10.0f, 1.0f) && (tA.fY > tPanel.fY) && FloatNear(tB.fY + tB.fH, tPanel.fY + tPanel.fH, 1.5f), "row gap and align") ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 5;
	}

	tA = xgeXuiWidgetGetRect(pColumnA);
	tB = xgeXuiWidgetGetRect(pColumnB);
	if ( !Require(FloatNear(RectGapY(tA, tB), 6.0f, 1.0f) && (tA.fY < tB.fY) && (tA.fX > tB.fX), "column percent and align") ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 6;
	}

	tA = xgeXuiWidgetGetRect(pStackBack);
	tB = xgeXuiWidgetGetRect(pStackFront);
	if ( !Require((tA.fW > tB.fW) && (tA.fH > tB.fH) && FloatNear(tB.fX + tB.fW * 0.5f, tA.fX + tA.fW * 0.5f, 1.0f) && FloatNear(tB.fY + tB.fH * 0.5f, tA.fY + tA.fH * 0.5f, 1.0f), "stack center") ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 7;
	}

	tPanel = pJustifyPanel->tContentRect;
	tA = xgeXuiWidgetGetRect(pJustifyA);
	tB = xgeXuiWidgetGetRect(pJustifyB);
	tC = xgeXuiWidgetGetRect(pJustifyC);
	if ( !Require(FloatNear(tA.fX, tPanel.fX, 1.0f) && FloatNear(tC.fX + tC.fW, tPanel.fX + tPanel.fW, 1.0f) && (tB.fX > tA.fX), "space-between justify") ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 8;
	}

	tA = xgeXuiWidgetGetRect(pGridA);
	tB = xgeXuiWidgetGetRect(pGridB);
	tC = xgeXuiWidgetGetRect(pGridC);
	tD = xgeXuiWidgetGetRect(pGridD);
	if ( !Require(FloatNear(tA.fY, tB.fY, 0.5f) && FloatNear(tC.fY, tD.fY, 0.5f) && (tB.fX > tA.fX) && (tC.fY > tA.fY) && (tD.fX > tC.fX), "grid placement") ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 9;
	}

	tA = xgeXuiWidgetGetRect(pContent);
	tB = xgeXuiWidgetGetRect(pGrow);
	tC = xgeXuiWidgetGetRect(pPercent);
	tD = xgeXuiWidgetGetRect(pPx);
	if ( !Require((pContent->tStyle.tWidth.iUnit == XGE_XUI_SIZE_CONTENT) && (pGrow->tStyle.tWidth.iUnit == XGE_XUI_SIZE_GROW) && (pPercent->tStyle.tWidth.iUnit == XGE_XUI_SIZE_PERCENT) && (pPx->tStyle.tWidth.iUnit == XGE_XUI_SIZE_PX) && (tA.fW > 0.0f) && (tB.fW > tD.fW) && (tC.fW > 20.0f) && (tC.fW < tB.fW) && FloatNear(tD.fW, 46.0f, 1.0f), "content/grow/percent/px sizes") ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 10;
	}

	printf("xui_xson_layout_gallery_lab ok: migrated layout gallery geometry verified\n");
	xgeXuiPageUnload(&tPage);
	xgeXuiUnit(&tXui);
	return 0;
}
