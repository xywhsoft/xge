#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_menubar_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_menubar_test_select_t {
	xui_widget pMenu;
	int iCount;
	int iIndex;
	int iValue;
} xui_menubar_test_select_t;

static void __xuiMenuBarSelected(xui_widget pMenu, int iIndex, int iValue, void* pUser)
{
	xui_menubar_test_select_t* pSelect;

	pSelect = (xui_menubar_test_select_t*)pUser;
	if ( pSelect != NULL ) {
		pSelect->pMenu = pMenu;
		pSelect->iCount++;
		pSelect->iIndex = iIndex;
		pSelect->iValue = iValue;
	}
}

static int __xuiMenuBarDispatchMove(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiMenuBarDispatchDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiMenuBarDispatchUp(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiMenuBarKey(xui_context pContext, int iKey, uint32_t iModifiers)
{
	int iRet;

	iRet = xuiInputKeyDown(pContext, iKey, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiMenuBarClickItem(xui_context pContext, xui_widget pMenuBar, int iIndex)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;
	float fX;
	float fY;
	int iRet;

	tWorld = xuiWidgetGetWorldRect(pMenuBar);
	tItem = xuiMenuBarGetItemRect(pMenuBar, iIndex);
	fX = tWorld.fX + tItem.fX + tItem.fW * 0.5f;
	fY = tWorld.fY + tItem.fY + tItem.fH * 0.5f;
	iRet = __xuiMenuBarDispatchMove(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiMenuBarDispatchDown(pContext, fX, fY);
}

static int __xuiMenuBarMoveToItem(xui_context pContext, xui_widget pMenuBar, int iIndex)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;

	tWorld = xuiWidgetGetWorldRect(pMenuBar);
	tItem = xuiMenuBarGetItemRect(pMenuBar, iIndex);
	return __xuiMenuBarDispatchMove(pContext, tWorld.fX + tItem.fX + tItem.fW * 0.5f, tWorld.fY + tItem.fY + tItem.fH * 0.5f);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_menubar_test_select_t tSelect;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pMenuBar;
	xui_widget pFileMenu;
	xui_widget pEditMenu;
	xui_surface pTarget;
	xui_font pFont;
	xui_menu_desc_t tMenuDesc;
	xui_menu_item_t arrFileItems[3];
	xui_menu_item_t arrEditItems[3];
	xui_menubar_item_t arrBarItems[3];
	xui_rect_i_t tFullRect;
	xui_rect_t tFileRect;
	xui_menubar_metrics_t tMetrics;
	xui_menubar_colors_t tColors;
	int iFailed;
	int iRet;

	memset(&tSelect, 0, sizeof(tSelect));
	pContext = NULL;
	pRoot = NULL;
	pMenuBar = NULL;
	pFileMenu = NULL;
	pEditMenu = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 520.0f, 320.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "body", 4, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	(void)xuiSetDefaultFont(pContext, pFont);

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	(void)xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 520.0f, 320.0f});
	(void)xuiWidgetSetFocusable(pRoot, 1);
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	memset(&tMenuDesc, 0, sizeof(tMenuDesc));
	tMenuDesc.iSize = sizeof(tMenuDesc);
	tMenuDesc.pFont = pFont;
	iRet = xuiMenuCreate(pContext, &pFileMenu, &tMenuDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pFileMenu != NULL, "file menu create");
	memset(arrFileItems, 0, sizeof(arrFileItems));
	arrFileItems[0].sText = "New";
	arrFileItems[0].sShortcut = "Ctrl+N";
	arrFileItems[0].iType = XUI_MENU_ITEM_NORMAL;
	arrFileItems[0].iState = XUI_MENU_ITEM_ENABLED;
	arrFileItems[0].iValue = 10;
	arrFileItems[1].sText = "Open";
	arrFileItems[1].iType = XUI_MENU_ITEM_NORMAL;
	arrFileItems[1].iState = XUI_MENU_ITEM_ENABLED;
	arrFileItems[1].iValue = 11;
	arrFileItems[2].sText = "Disabled";
	arrFileItems[2].iType = XUI_MENU_ITEM_NORMAL;
	arrFileItems[2].iState = 0;
	arrFileItems[2].iValue = 12;
	iRet = xuiMenuSetItems(pFileMenu, arrFileItems, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "file menu items");

	iRet = xuiMenuCreate(pContext, &pEditMenu, &tMenuDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pEditMenu != NULL, "edit menu create");
	memset(arrEditItems, 0, sizeof(arrEditItems));
	arrEditItems[0].sText = "Undo";
	arrEditItems[0].sShortcut = "Ctrl+Z";
	arrEditItems[0].iType = XUI_MENU_ITEM_NORMAL;
	arrEditItems[0].iState = XUI_MENU_ITEM_ENABLED;
	arrEditItems[0].iValue = 20;
	arrEditItems[1].sText = "Copy";
	arrEditItems[1].sShortcut = "Ctrl+C";
	arrEditItems[1].iType = XUI_MENU_ITEM_NORMAL;
	arrEditItems[1].iState = XUI_MENU_ITEM_ENABLED;
	arrEditItems[1].iValue = 21;
	arrEditItems[2].sText = "Readonly";
	arrEditItems[2].iType = XUI_MENU_ITEM_NORMAL;
	arrEditItems[2].iState = 0;
	arrEditItems[2].iValue = 22;
	iRet = xuiMenuSetItems(pEditMenu, arrEditItems, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "edit menu items");

	iRet = xuiMenuBarCreate(pContext, &pMenuBar, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && pMenuBar != NULL, "menubar create");
	(void)xuiWidgetSetRect(pMenuBar, (xui_rect_t){20.0f, 18.0f, 360.0f, 28.0f});
	iRet = xuiWidgetAddChild(pRoot, pMenuBar);
	XUI_TEST_CHECK(iRet == XUI_OK, "menubar add");
	iRet = xuiMenuBarSetSelect(pMenuBar, __xuiMenuBarSelected, &tSelect);
	XUI_TEST_CHECK(iRet == XUI_OK, "select set");

	memset(arrBarItems, 0, sizeof(arrBarItems));
	arrBarItems[0].sText = "&File";
	arrBarItems[0].iState = XUI_MENUBAR_ITEM_ENABLED;
	arrBarItems[0].iValue = 1;
	arrBarItems[0].pMenu = pFileMenu;
	arrBarItems[1].sText = "&Edit";
	arrBarItems[1].iState = XUI_MENUBAR_ITEM_ENABLED;
	arrBarItems[1].iValue = 2;
	arrBarItems[1].pMenu = pEditMenu;
	arrBarItems[2].sText = "&View";
	arrBarItems[2].iState = 0;
	arrBarItems[2].iValue = 3;
	iRet = xuiMenuBarSetItems(pMenuBar, arrBarItems, 3);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetItemCount(pMenuBar) == 3, "menubar items");
	XUI_TEST_CHECK(!xuiMenuBarIsItemEnabled(pMenuBar, 2), "disabled item");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 520, 320, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	tFullRect = (xui_rect_i_t){0, 0, 520, 320};
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetCacheSurface(pMenuBar, xuiWidgetGetStateId(pMenuBar)) != NULL, "render cache");
	tFileRect = xuiMenuBarGetItemRect(pMenuBar, 0);
	XUI_TEST_CHECK(tFileRect.fW >= 20.0f && tFileRect.fH > 10.0f, "item rect");

	iRet = __xuiMenuBarClickItem(pContext, pMenuBar, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarIsOpen(pMenuBar) && xuiMenuBarGetOpenIndex(pMenuBar) == 0 && xuiMenuIsOpen(pFileMenu), "click opens file");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == pMenuBar, "menubar keeps focus");
	iRet = __xuiMenuBarDispatchUp(pContext, tFileRect.fX + 4.0f, tFileRect.fY + 4.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pFileMenu), "opening release ignored");

	iRet = __xuiMenuBarMoveToItem(pContext, pMenuBar, 1);
	if ( iRet == XUI_OK ) iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuIsOpen(pFileMenu) && xuiMenuIsOpen(pEditMenu) && xuiMenuBarGetOpenIndex(pMenuBar) == 1, "hover switches menu");
	iRet = __xuiMenuBarClickItem(pContext, pMenuBar, 1);
	if ( iRet == XUI_OK ) iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuBarIsOpen(pMenuBar), "click open item closes");
	(void)__xuiMenuBarDispatchUp(pContext, 60.0f, 30.0f);

	iRet = xuiSetFocusWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus root");
	iRet = __xuiMenuBarKey(pContext, 'F', XUI_MOD_ALT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetOpenIndex(pMenuBar) == 0 && xuiMenuIsOpen(pFileMenu), "alt mnemonic opens");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_RIGHT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetOpenIndex(pMenuBar) == 1 && xuiMenuIsOpen(pEditMenu), "right switches open menu");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_DOWN, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuGetHoverIndex(pEditMenu) == 1, "down moves menu hover");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_ENTER, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelect.iCount == 1 && tSelect.pMenu == pEditMenu && tSelect.iValue == 21, "enter commits open menu");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuBarIsOpen(pMenuBar), "commit closes menubar");

	iRet = xuiSetFocusWidget(pContext, pMenuBar);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus menubar");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_F10, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetHoverIndex(pMenuBar) >= 0, "f10 focuses item");
	iRet = xuiMenuBarSetItemEnabled(pMenuBar, 2, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarIsItemEnabled(pMenuBar, 2), "enable item");
	iRet = xuiMenuBarSetItemMnemonic(pMenuBar, 2, 'W');
	XUI_TEST_CHECK(iRet == XUI_OK, "set mnemonic");
	iRet = __xuiMenuBarKey(pContext, 'W', XUI_MOD_ALT);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelect.iCount == 2 && tSelect.iValue == 3, "top command without menu");
	iRet = xuiMenuBarGetMetrics(pMenuBar, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK && tMetrics.fHeight >= 18.0f, "metrics get");
	iRet = xuiMenuBarGetColors(pMenuBar, &tColors);
	XUI_TEST_CHECK(iRet == XUI_OK && tColors.iTextColor != 0u, "colors get");
	XUI_TEST_CHECK(xuiMenuBarGetChangeCount(pMenuBar) > 0, "change count");

cleanup:
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_menubar_test passed\n");
	return 0;
}
