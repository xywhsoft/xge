#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const char g_sVirtualListXson[] =
	"{"
	"  \"xui\": 1,"
	"  \"tokens\": {"
	"    \"colors\": { \"panel\": \"#111820\", \"row\": \"#1D2630\", \"bar\": \"#3A4250\", \"thumb\": \"#2A7FFF\", \"text\": \"#EEF4FF\" },"
	"    \"spacing\": { \"rows\": 64, \"itemH\": 24, \"scrollY\": 24, \"gap\": 4 }"
	"  },"
	"  \"styles\": {"
	"    \"list\": { \"width\": 160, \"height\": 96, \"padding\": 4, \"backgroundColor\": \"@colors.panel\", \"barColor\": \"@colors.bar\", \"thumbColor\": \"@colors.thumb\" },"
	"    \"rowItem\": { \"layout\": \"row\", \"height\": \"@spacing.itemH\", \"padding\": [6, 2], \"gap\": \"@spacing.gap\", \"background\": \"@colors.row\", \"alignY\": \"center\" },"
	"    \"rowText\": { \"textColor\": \"@colors.text\" }"
	"  },"
	"  \"templates\": {"
	"    \"inventoryRow\": { \"type\": \"row\", \"id\": \"row-template\", \"style\": \"rowItem\", \"children\": ["
	"      { \"type\": \"label\", \"id\": \"row-label\", \"style\": \"rowText\", \"text\": \"Item\" }"
	"    ] }"
	"  },"
	"  \"tree\": {"
	"    \"type\": \"virtualList\","
	"    \"id\": \"inventory\","
	"    \"style\": \"list\","
	"    \"itemCount\": \"@spacing.rows\","
	"    \"itemHeight\": \"@spacing.itemH\","
	"    \"scrollY\": \"@spacing.scrollY\","
	"    \"itemTemplate\": \"inventoryRow\""
	"  }"
	"}";

int main(void)
{
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_xui_widget pRoot;
	xge_xui_widget pSlot;
	xge_xui_widget pLabel;
	xge_xui_virtual_list pList;
	xge_rect_t tRect;
	int iRet;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tPage, 0, sizeof(tPage));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		printf("xgeXuiInit failed\n");
		return 1;
	}
	iRet = xgeXuiPageLoadMemory(&tXui, g_sVirtualListXson, (int)strlen(g_sVirtualListXson), NULL, &tPage);
	if ( iRet != XGE_OK ) {
		printf("xgeXuiPageLoadMemory failed: %s\n", xgeXuiPageGetError(&tPage));
		xgeXuiUnit(&tXui);
		return 2;
	}
	pRoot = xgeXuiPageFind(&tPage, "inventory");
	if ( (pRoot == NULL) || (pRoot->procEvent != xgeXuiVirtualListEventProc) || (pRoot->procPaint != xgeXuiVirtualListPaintProc) || (pRoot->procLayout != xgeXuiVirtualListLayoutProc) || (tPage.iVirtualListCount != 1) ) {
		printf("virtual list control verification failed\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 3;
	}
	pList = (xge_xui_virtual_list)pRoot->pUser;
	if ( (pList == NULL) || (pList->iItemCount != 64) || (pList->fItemHeight != 24.0f) || (pList->fScrollY != 24.0f) || (pRoot->tStyle.iBackgroundColor != XGE_COLOR_RGBA(0x11, 0x18, 0x20, 0xFF)) ) {
		printf("virtual list property verification failed\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 4;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 160.0f;
	tRect.fH = 96.0f;
	xgeXuiWidgetSetRect(pRoot, tRect);
	xgeXuiUpdate(&tXui, 0.0f);
	pSlot = xgeXuiVirtualListGetSlotWidget(pList, 0);
	pLabel = xgeXuiPageFind(&tPage, "row-label");
	if ( (xgeXuiVirtualListGetFirstVisible(pList) != 1) || (xgeXuiVirtualListGetVisibleCount(pList) != 5) || (pSlot == NULL) || (pSlot->pInternal != (void*)(intptr_t)2) || (pLabel == NULL) || (tPage.iLabelCount != 5) ) {
		printf("virtual list initial window verification failed\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 5;
	}
	tRect = xgeXuiWidgetGetRect(pSlot);
	if ( (tRect.fX != 4.0f) || (tRect.fY != 4.0f) || (tRect.fW != 152.0f) || (tRect.fH != 24.0f) ) {
		printf("virtual list slot geometry verification failed\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 6;
	}
	xgeXuiVirtualListSetScroll(pList, 72.0f);
	xgeXuiUpdate(&tXui, 0.0f);
	pSlot = xgeXuiVirtualListGetSlotWidget(pList, 0);
	if ( (xgeXuiVirtualListGetFirstVisible(pList) != 3) || (xgeXuiVirtualListGetVisibleCount(pList) != 5) || (pSlot == NULL) || (pSlot->pInternal != (void*)(intptr_t)4) || (tPage.iLabelCount != 5) ) {
		printf("virtual list reuse verification failed\n");
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 7;
	}
	printf("xui_xson_virtual_list_lab ok: itemTemplate windowing and slot reuse verified\n");
	xgeXuiPageUnload(&tPage);
	xgeXuiUnit(&tXui);
	return 0;
}
