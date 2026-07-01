#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_menu_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_menu_test_select_t {
	xui_widget pMenu;
	int iCount;
	int iIndex;
	int iValue;
} xui_menu_test_select_t;

static int __xuiMenuNear(float fA, float fB)
{
	float fD = fA - fB;
	if ( fD < 0.0f ) fD = -fD;
	return fD < 0.01f;
}

static int __xuiMenuRectEq(xui_rect_t tRect, float fX, float fY, float fW, float fH)
{
	return (tRect.fX == fX) && (tRect.fY == fY) && (tRect.fW == fW) && (tRect.fH == fH);
}

static void __xuiMenuSelected(xui_widget pMenu, int iIndex, int iValue, void* pUser)
{
	xui_menu_test_select_t* pSelect;

	pSelect = (xui_menu_test_select_t*)pUser;
	pSelect->pMenu = pMenu;
	pSelect->iCount++;
	pSelect->iIndex = iIndex;
	pSelect->iValue = iValue;
}

static int __xuiMenuRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 420, 320};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiMenuDispatchDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiMenuDispatchUp(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiMenuDispatchMove(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiMenuClickItem(xui_context pContext, xui_widget pMenu, int iIndex)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;

	tWorld = xuiWidgetGetWorldRect(pMenu);
	tItem = xuiMenuGetItemRect(pMenu, iIndex);
	return __xuiMenuDispatchDown(pContext, tWorld.fX + tItem.fX + tItem.fW * 0.5f, tWorld.fY + tItem.fY + tItem.fH * 0.5f);
}

static int __xuiMenuMoveToItem(xui_context pContext, xui_widget pMenu, int iIndex)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;

	tWorld = xuiWidgetGetWorldRect(pMenu);
	tItem = xuiMenuGetItemRect(pMenu, iIndex);
	return __xuiMenuDispatchMove(pContext, tWorld.fX + tItem.fX + tItem.fW * 0.5f, tWorld.fY + tItem.fY + tItem.fH * 0.5f);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_menu_test_select_t tSelect;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pOwner;
	xui_widget pMenu;
	xui_widget pSubmenu;
	xui_surface pTarget;
	xui_surface pMenuCache;
	xui_menu_desc_t tDesc;
	xui_menu_item_t arrItems[7];
	xui_menu_item_t arrSubItems[1];
	xui_menu_item_t arrWideItems[1];
	xui_menu_metrics_t tMetrics;
	xui_rect_t tPopupRect;
	xui_rect_t tOwnerRect;
	const char* sWideText;
	const char* sWideShortcut;
	float fWidth;
	float fHeight;
	float fPopupW;
	float fPopupH;
	float fExpectedWidth;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pOwner = NULL;
	pMenu = NULL;
	pSubmenu = NULL;
	pTarget = NULL;
	pMenuCache = NULL;
	iFailed = 0;
	memset(&tSelect, 0, sizeof(tSelect));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 420.0f, 320.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 420.0f, 320.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	iRet = xuiWidgetCreate(pContext, &pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK && pOwner != NULL, "owner create");
	xuiWidgetSetRect(pOwner, (xui_rect_t){36.0f, 30.0f, 120.0f, 28.0f});
	xuiWidgetSetFocusable(pOwner, 1);
	iRet = xuiWidgetAddChild(pRoot, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK, "add owner");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pOwner = pOwner;
	iRet = xuiMenuCreate(pContext, &pSubmenu, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pSubmenu != NULL, "submenu create");
	memset(arrSubItems, 0, sizeof(arrSubItems));
	arrSubItems[0].sText = "Sub action";
	arrSubItems[0].sShortcut = "Ctrl+Shift+S";
	arrSubItems[0].iType = XUI_MENU_ITEM_NORMAL;
	arrSubItems[0].iState = XUI_MENU_ITEM_ENABLED;
	arrSubItems[0].iValue = 50;
	iRet = xuiMenuSetItems(pSubmenu, arrSubItems, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "submenu items");

	iRet = xuiMenuCreate(pContext, &pMenu, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pMenu != NULL, "menu create");
	iRet = xuiMenuSetSelect(pMenu, __xuiMenuSelected, &tSelect);
	XUI_TEST_CHECK(iRet == XUI_OK, "select callback");

	memset(arrItems, 0, sizeof(arrItems));
	arrItems[0].sText = "New file";
	arrItems[0].sShortcut = "Ctrl+N";
	arrItems[0].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[0].iState = XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DEFAULT;
	arrItems[0].iValue = 10;
	arrItems[1].sText = "Show grid";
	arrItems[1].sShortcut = "G";
	arrItems[1].iType = XUI_MENU_ITEM_CHECK;
	arrItems[1].iState = XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_CHECKED;
	arrItems[1].iValue = 20;
	arrItems[2].iType = XUI_MENU_ITEM_SEPARATOR;
	arrItems[3].sText = "Small icons";
	arrItems[3].iType = XUI_MENU_ITEM_RADIO;
	arrItems[3].iState = XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_CHECKED;
	arrItems[3].iValue = 30;
	arrItems[4].sText = "Large icons";
	arrItems[4].iType = XUI_MENU_ITEM_RADIO;
	arrItems[4].iState = XUI_MENU_ITEM_ENABLED;
	arrItems[4].iValue = 31;
	arrItems[5].sText = "More";
	arrItems[5].iType = XUI_MENU_ITEM_SUBMENU;
	arrItems[5].iState = XUI_MENU_ITEM_ENABLED;
	arrItems[5].pSubmenu = pSubmenu;
	arrItems[6].sText = "Disabled action";
	arrItems[6].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[6].iState = 0;
	arrItems[6].iValue = 60;
	iRet = xuiMenuSetItems(pMenu, arrItems, 7);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuGetItemCount(pMenu) == 7, "set items");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 420, 320, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "initial layout");
	iRet = xuiSetFocusWidget(pContext, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pOwner, "focus owner");

	iRet = xuiMenuOpenForOwner(pMenu, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "open for owner");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "open layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiMenuRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	iRet = xuiMenuMeasure(pMenu, &fWidth, &fHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && fWidth >= 112.0f && fHeight > 120.0f, "menu measure");
	iRet = xuiPopupGetContentSize(xuiMenuGetPopupWidget(pMenu), &fPopupW, &fPopupH);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiMenuNear(fWidth, fPopupW) && __xuiMenuNear(fHeight, fPopupH), "popup content size");
	XUI_TEST_CHECK(xuiMenuGetContentWidget(pMenu) == pMenu && xuiMenuGetOwner(pMenu) == pOwner, "menu internals");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == pMenu, "menu focused");
	pMenuCache = xuiWidgetGetCacheSurface(pMenu, xuiWidgetGetStateId(pMenu));
	XUI_TEST_CHECK(pMenuCache != NULL, "menu cache surface");
	XUI_TEST_CHECK(!__xuiMenuRectEq(xuiTestSurfaceGetLastRect(pMenuCache), 1.0f, 1.0f, fWidth - 2.0f, fHeight - 2.0f), "no menu panel focus frame");
	tOwnerRect = xuiWidgetGetWorldRect(pOwner);
	iRet = __xuiMenuDispatchUp(pContext, tOwnerRect.fX + 8.0f, tOwnerRect.fY + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "opening release ignored");
	iRet = __xuiMenuDispatchDown(pContext, tOwnerRect.fX + 8.0f, tOwnerRect.fY + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuIsOpen(pMenu), "owner press closes menu");
	(void)__xuiMenuDispatchUp(pContext, tOwnerRect.fX + 8.0f, tOwnerRect.fY + 8.0f);

	iRet = xuiMenuOpenForOwner(pMenu, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "reopen after owner close");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "reopen layout after owner close");

	iRet = __xuiMenuClickItem(pContext, pMenu, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuIsOpen(pMenu), "check click closes");
	XUI_TEST_CHECK(tSelect.iCount == 1 && tSelect.iIndex == 1 && tSelect.iValue == 20, "check select callback");
	XUI_TEST_CHECK((xuiMenuGetItemState(pMenu, 1) & XUI_MENU_ITEM_CHECKED) == 0u, "check toggled");

	iRet = xuiMenuOpenAt(pMenu, pOwner, 120.0f, 90.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "open at");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "open at layout");
	tPopupRect = xuiPopupGetPopupRect(xuiMenuGetPopupWidget(pMenu));
	XUI_TEST_CHECK(__xuiMenuNear(tPopupRect.fX, 120.0f) && __xuiMenuNear(tPopupRect.fY, 90.0f), "fixed anchor");
	iRet = __xuiMenuClickItem(pContext, pMenu, 4);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuIsOpen(pMenu), "radio click closes");
	XUI_TEST_CHECK((xuiMenuGetItemState(pMenu, 3) & XUI_MENU_ITEM_CHECKED) == 0u, "radio previous clear");
	XUI_TEST_CHECK((xuiMenuGetItemState(pMenu, 4) & XUI_MENU_ITEM_CHECKED) != 0u, "radio selected");

	iRet = xuiMenuOpenForOwner(pMenu, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK, "reopen shortcut");
	iRet = xuiInputKeyDown(pContext, 'N', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK, "shortcut input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuIsOpen(pMenu), "shortcut closes");
	XUI_TEST_CHECK(tSelect.iCount == 3 && tSelect.iIndex == 0 && tSelect.iValue == 10, "shortcut selects");

	iRet = xuiMenuOpenForOwner(pMenu, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK, "reopen disabled");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "disabled layout");
	iRet = __xuiMenuClickItem(pContext, pMenu, 6);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "disabled does not close");
	XUI_TEST_CHECK(tSelect.iCount == 3, "disabled no select");

	iRet = __xuiMenuMoveToItem(pContext, pMenu, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "move to submenu item");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "submenu layout");
	XUI_TEST_CHECK(xuiMenuIsOpen(pSubmenu), "submenu opened on hover");
	iRet = __xuiMenuClickItem(pContext, pSubmenu, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuIsOpen(pMenu) && !xuiMenuIsOpen(pSubmenu), "submenu select closes root");
	XUI_TEST_CHECK(tSelect.iCount == 4 && tSelect.pMenu == pSubmenu && tSelect.iIndex == 0 && tSelect.iValue == 50, "submenu inherited callback");

	memset(arrWideItems, 0, sizeof(arrWideItems));
	sWideText = "Export selection with all metadata";
	sWideShortcut = "Ctrl+Shift+Alt+E";
	arrWideItems[0].sText = sWideText;
	arrWideItems[0].sShortcut = sWideShortcut;
	arrWideItems[0].iType = XUI_MENU_ITEM_NORMAL;
	arrWideItems[0].iState = XUI_MENU_ITEM_ENABLED;
	iRet = xuiMenuSetItems(pMenu, arrWideItems, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "wide item set");
	iRet = xuiMenuGetMetrics(pMenu, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK, "wide metrics");
	iRet = xuiMenuMeasure(pMenu, &fWidth, &fHeight);
	XUI_TEST_CHECK(iRet == XUI_OK, "wide measure");
	fExpectedWidth = tMetrics.fPaddingX * 2.0f + tMetrics.fMarkWidth + tMetrics.fIconWidth +
		4.0f + ((float)strlen(sWideText) * 7.0f) + 6.0f +
		tMetrics.fShortcutGap + ((float)strlen(sWideShortcut) * 7.0f) + 6.0f +
		4.0f + tMetrics.fArrowWidth;
	XUI_TEST_CHECK(fWidth >= fExpectedWidth, "wide text and shortcut width");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_menu_test passed\n");
	return 0;
}
