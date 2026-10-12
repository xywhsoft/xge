#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_cascader_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_cascader_test_change_t {
	int iCount;
	int iLeaf;
	int iDepth;
	int arrValues[XUI_CASCADER_PATH_CAPACITY];
} xui_cascader_test_change_t;

static void __xuiCascaderChanged(xui_widget pWidget, int iLeafIndex, const int* arrValues, int iDepth, void* pUser)
{
	xui_cascader_test_change_t* pChange;
	int i;

	(void)pWidget;
	pChange = (xui_cascader_test_change_t*)pUser;
	pChange->iCount++;
	pChange->iLeaf = iLeafIndex;
	pChange->iDepth = iDepth;
	for ( i = 0; i < XUI_CASCADER_PATH_CAPACITY; i++ ) {
		pChange->arrValues[i] = (arrValues != NULL && i < iDepth) ? arrValues[i] : -1;
	}
}

static int __xuiCascaderRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 520, 360};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiCascaderDispatchDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiCascaderClickItem(xui_context pContext, xui_widget pCascader, int iColumn, int iItem)
{
	xui_widget pPanel;
	xui_rect_t tWorld;
	xui_rect_t tItem;

	pPanel = xuiCascaderGetPanelWidget(pCascader);
	if ( pPanel == NULL ) return XUI_ERROR;
	tWorld = xuiWidgetGetWorldRect(pPanel);
	tItem = xuiCascaderGetItemRect(pCascader, iColumn, iItem);
	return __xuiCascaderDispatchDown(pContext, tWorld.fX + tItem.fX + tItem.fW * 0.5f, tWorld.fY + tItem.fY + tItem.fH * 0.5f);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_cascader_test_change_t tChange;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pCascader;
	xui_widget pPanel;
	xui_widget pPopup;
	xui_surface pTarget;
	xui_font pFont;
	xui_cascader_desc_t tDesc;
	xui_cascader_item_t arrItems[11];
	int arrSelected[3];
	int arrPath[3];
	xui_rect_t tButton;
	xui_rect_t tClear;
	xui_rect_t tPopupRect;
	uint32_t iState;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pCascader = NULL;
	pPanel = NULL;
	pPopup = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tChange, 0, sizeof(tChange));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "cascader", 8, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 520.0f, 360.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 520.0f, 360.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(arrItems, 0, sizeof(arrItems));
	arrItems[0] = (xui_cascader_item_t){"Guide", 1, -1, 0u, NULL};
	arrItems[1] = (xui_cascader_item_t){"Components", 2, -1, 0u, NULL};
	arrItems[2] = (xui_cascader_item_t){"Resources", 3, -1, 0u, NULL};
	arrItems[3] = (xui_cascader_item_t){"Design", 11, 0, XUI_CASCADER_ITEM_LEAF, NULL};
	arrItems[4] = (xui_cascader_item_t){"Navigation", 12, 0, 0u, NULL};
	arrItems[5] = (xui_cascader_item_t){"Side nav", 121, 4, XUI_CASCADER_ITEM_LEAF, NULL};
	arrItems[6] = (xui_cascader_item_t){"Top nav", 122, 4, XUI_CASCADER_ITEM_LEAF, NULL};
	arrItems[7] = (xui_cascader_item_t){"Disabled leaf", 123, 4, XUI_CASCADER_ITEM_DISABLED | XUI_CASCADER_ITEM_LEAF, NULL};
	arrItems[8] = (xui_cascader_item_t){"Forms", 21, 1, 0u, NULL};
	arrItems[9] = (xui_cascader_item_t){"Input", 211, 8, XUI_CASCADER_ITEM_LEAF, NULL};
	arrItems[10] = (xui_cascader_item_t){"DatePicker", 212, 8, XUI_CASCADER_ITEM_LEAF, NULL};
	arrSelected[0] = 1;
	arrSelected[1] = 12;
	arrSelected[2] = 121;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItems = arrItems;
	tDesc.iItemCount = 11;
	tDesc.arrSelectedValues = arrSelected;
	tDesc.iSelectedDepth = 3;
	tDesc.pFont = pFont;
	tDesc.bClearable = 1;
	tDesc.fColumnWidth = 128.0f;
	tDesc.fPopupMaxHeight = 160.0f;
	iRet = xuiCascaderCreate(pContext, &pCascader, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pCascader != NULL, "cascader create");
	iRet = xuiCascaderSetChange(pCascader, __xuiCascaderChanged, &tChange);
	XUI_TEST_CHECK(iRet == XUI_OK, "change callback");
	xuiWidgetSetRect(pCascader, (xui_rect_t){40.0f, 32.0f, 220.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pCascader);
	XUI_TEST_CHECK(iRet == XUI_OK, "add cascader");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 520, 360, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiCascaderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");

	XUI_TEST_CHECK(xuiCascaderGetItemCount(pCascader) == 11, "item count");
	XUI_TEST_CHECK(xuiCascaderGetSelectedLeaf(pCascader) == 5, "initial leaf");
	XUI_TEST_CHECK(xuiCascaderGetSelectedDepth(pCascader) == 3, "initial depth");
	XUI_TEST_CHECK(strcmp(xuiCascaderGetPlaceholder(pCascader), "Select") == 0, "default placeholder");
	XUI_TEST_CHECK(strcmp(xuiCascaderGetSelectedText(pCascader), "Guide / Navigation / Side nav") == 0, "full display");
	XUI_TEST_CHECK(xuiCascaderGetSelectedPath(pCascader, arrPath, 3) == 3 && arrPath[0] == 1 && arrPath[1] == 12 && arrPath[2] == 121, "selected values");
	iRet = xuiCascaderSetShowAllLevels(pCascader, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCascaderGetSelectedText(pCascader), "Side nav") == 0, "leaf display");
	iRet = xuiCascaderSetShowAllLevels(pCascader, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore full display");
	tButton = xuiCascaderGetButtonRect(pCascader);
	tClear = xuiCascaderGetClearRect(pCascader);
	XUI_TEST_CHECK(tButton.fW > 20.0f && tButton.fH > 20.0f && tClear.fW > 12.0f, "owner rects");

	iRet = xuiCascaderOpen(pCascader);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCascaderIsOpen(pCascader), "open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "open layout");
	iRet = __xuiCascaderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "open render");
	pPanel = xuiCascaderGetPanelWidget(pCascader);
	pPopup = xuiCascaderGetPopupWidget(pCascader);
	XUI_TEST_CHECK(pPanel != NULL && pPopup != NULL && xuiGetFocusWidget(pContext) == pPanel, "panel popup focus");
	XUI_TEST_CHECK(xuiCascaderGetColumnCount(pCascader) == 3, "column count");
	iState = xuiCascaderGetState(pCascader);
	XUI_TEST_CHECK((iState & XUI_CASCADER_STATE_OPEN) != 0u, "open state");
	tPopupRect = xuiPopupGetPopupRect(pPopup);
	XUI_TEST_CHECK(tPopupRect.fY >= 60.0f && tPopupRect.fW >= 380.0f, "popup rect");

	iRet = __xuiCascaderClickItem(pContext, pCascader, 2, 7);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCascaderIsOpen(pCascader), "disabled click keeps open");
	XUI_TEST_CHECK(tChange.iCount == 0 && xuiCascaderGetSelectedLeaf(pCascader) == 5, "disabled no change");
	iRet = __xuiCascaderClickItem(pContext, pCascader, 2, 6);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiCascaderIsOpen(pCascader), "leaf click closes");
	XUI_TEST_CHECK(tChange.iCount == 1 && tChange.iLeaf == 6 && tChange.iDepth == 3, "leaf callback");
	XUI_TEST_CHECK(tChange.arrValues[0] == 1 && tChange.arrValues[1] == 12 && tChange.arrValues[2] == 122, "callback values");
	XUI_TEST_CHECK(xuiCascaderGetSelectedLeaf(pCascader) == 6 && strcmp(xuiCascaderGetSelectedText(pCascader), "Guide / Navigation / Top nav") == 0, "selected top nav");

	iRet = xuiSetFocusWidget(pContext, pCascader);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus owner");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DELETE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "delete input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tChange.iCount == 2 && xuiCascaderGetSelectedDepth(pCascader) == 0, "keyboard clear");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCascaderIsOpen(pCascader), "key down opens");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key right input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCascaderGetColumnCount(pCascader) >= 2, "key right opens child column");

	iRet = xuiWidgetSetEnabled(pCascader, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable cascader");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable dispatch");
	iRet = xuiCascaderOpen(pCascader);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiCascaderIsOpen(pCascader), "disabled does not open");
	iState = xuiCascaderGetState(pCascader);
	XUI_TEST_CHECK((iState & XUI_WIDGET_STATE_DISABLED) != 0u, "disabled state");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_cascader_test passed\n");
	return 0;
}
