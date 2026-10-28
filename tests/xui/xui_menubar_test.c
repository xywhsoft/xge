#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
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

typedef struct xui_menubar_test_destroy_t {
	int iCount;
} xui_menubar_test_destroy_t;

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

static void __xuiMenuBarDestroySelected(xui_widget pMenu, int iIndex, int iValue, void* pUser)
{
	xui_menubar_test_destroy_t* pDestroy = (xui_menubar_test_destroy_t*)pUser;
	(void)iIndex;
	(void)iValue;
	pDestroy->iCount++;
	xuiWidgetDestroy(pMenu);
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
	xui_menubar_test_select_t tDirectSelect;
	xui_menubar_test_select_t tSecondSelect;
	xui_menubar_test_destroy_t tDestroy;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pMenuBar;
	xui_widget pSecondBar;
	xui_widget pTransientFocus;
	xui_widget pFileMenu;
	xui_widget pEditMenu;
	xui_widget pRecentMenu;
	xui_widget pDestroyBar;
	xui_widget pInvalidBar;
	xui_widget pLifetimeMenu;
	xui_surface pTarget;
	xui_font pFont;
	xui_menu_desc_t tMenuDesc;
	xui_menu_item_t arrFileItems[4];
	xui_menu_item_t arrEditItems[3];
	xui_menu_item_t arrRecentItems[1];
	xui_menubar_item_t arrBarItems[3];
	xui_menubar_item_t arrSecondItems[1];
	xui_menubar_item_t arrOwnedItems[1];
	xui_menubar_item_t arrDestroyItems[1];
	xui_rect_i_t tFullRect;
	xui_rect_t tFileRect;
	xui_rect_t tViewRect;
	xui_rect_t tBarWorld;
	xui_menubar_metrics_t tMetrics;
	xui_menubar_metrics_t tBadMetrics;
	xui_menubar_colors_t tColors;
	xui_menubar_desc_t tBadDesc;
	xui_accessible_node_t tAccessible;
	char sOwnedText[220];
	int iChangeCount;
	int iMeasureCount;
	int iBefore;
	int iFailed;
	int iRet;

	memset(&tSelect, 0, sizeof(tSelect));
	memset(&tDirectSelect, 0, sizeof(tDirectSelect));
	memset(&tSecondSelect, 0, sizeof(tSecondSelect));
	memset(&tDestroy, 0, sizeof(tDestroy));
	pContext = NULL;
	pRoot = NULL;
	pMenuBar = NULL;
	pSecondBar = NULL;
	pTransientFocus = NULL;
	pFileMenu = NULL;
	pEditMenu = NULL;
	pRecentMenu = NULL;
	pDestroyBar = NULL;
	pInvalidBar = NULL;
	pLifetimeMenu = NULL;
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
	iRet = xuiMenuCreate(pContext, &pRecentMenu, &tMenuDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pRecentMenu != NULL, "recent menu create");
	memset(arrRecentItems, 0, sizeof(arrRecentItems));
	arrRecentItems[0].sText = "Project";
	arrRecentItems[0].iType = XUI_MENU_ITEM_NORMAL;
	arrRecentItems[0].iState = XUI_MENU_ITEM_ENABLED;
	arrRecentItems[0].iValue = 30;
	iRet = xuiMenuSetItems(pRecentMenu, arrRecentItems, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "recent menu items");

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
	arrFileItems[3].sText = "Recent";
	arrFileItems[3].iType = XUI_MENU_ITEM_SUBMENU;
	arrFileItems[3].iState = XUI_MENU_ITEM_ENABLED;
	arrFileItems[3].pSubmenu = pRecentMenu;
	iRet = xuiMenuSetItems(pFileMenu, arrFileItems, 4);
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
	iRet = xuiMenuSetSelect(pEditMenu, __xuiMenuBarSelected, &tDirectSelect);
	XUI_TEST_CHECK(iRet == XUI_OK, "edit menu direct callback");

	memset(&tBadDesc, 0, sizeof(tBadDesc));
	tBadDesc.iSize = sizeof(tBadDesc);
	tBadDesc.bHasMetrics = 1;
	tBadDesc.tMetrics.iSize = sizeof(tBadDesc.tMetrics);
	tBadDesc.tMetrics.fHeight = NAN;
	pInvalidBar = pRoot;
	iRet = xuiMenuBarCreate(pContext, &pInvalidBar, &tBadDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidBar == NULL, "invalid descriptor rejected");

	iRet = xuiMenuBarCreate(pContext, &pMenuBar, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && pMenuBar != NULL, "menubar create");
	iRet = xuiWidgetSetAccessibleName(pMenuBar, "Main menu");
	XUI_TEST_CHECK(iRet == XUI_OK, "accessible name");
	iRet = xuiWidgetSetAccessibleDescription(pMenuBar, "Application menu bar");
	XUI_TEST_CHECK(iRet == XUI_OK, "accessible description");
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
	arrBarItems[2].iState = 0x80000000u;
	arrBarItems[2].iValue = 3;
	iRet = xuiMenuBarSetItems(pMenuBar, arrBarItems, 3);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "unknown item state rejected");
	arrBarItems[2].iState = 0;
	iRet = xuiMenuBarSetItems(pMenuBar, arrBarItems, 3);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetItemCount(pMenuBar) == 3, "menubar items");
	XUI_TEST_CHECK(!xuiMenuBarIsItemEnabled(pMenuBar, 2), "disabled item");
	XUI_TEST_CHECK(xuiMenuBarSetItemMenu(pMenuBar, 0, pRoot) == XUI_ERROR_INVALID_ARGUMENT,
		"non-menu binding rejected");

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
	iMeasureCount = xuiTestProxyGetTextMeasureCount(&tState);
	iRet = __xuiMenuBarMoveToItem(pContext, pMenuBar, 0);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarMoveToItem(pContext, pMenuBar, 1);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarMoveToItem(pContext, pMenuBar, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestProxyGetTextMeasureCount(&tState) == iMeasureCount,
		"pointer movement reuses menu bar layout measurements");
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pMenuBar) == 4, "accessible item count");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pMenuBar, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_MENU &&
		strcmp(tAccessible.sName, "Main menu") == 0 &&
		strcmp(tAccessible.sDescription, "Application menu bar") == 0 &&
		tAccessible.iRowCount == 1 && tAccessible.iColumnCount == 3,
		"accessible root metadata");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pMenuBar, 1, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_MENU_ITEM &&
		strcmp(tAccessible.sName, "File") == 0 && tAccessible.iRow == 0 &&
		tAccessible.iColumn == 0 && tAccessible.iRowCount == 1 && tAccessible.iColumnCount == 1,
		"accessible item strips mnemonic marker");
	iRet = xuiWidgetPerformAccessibleAction(pMenuBar, 2u, XUI_ACCESSIBLE_ACTION_COLLAPSE, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE, "collapsed menu rejects accessibility collapse");
	iRet = xuiWidgetPerformAccessibleAction(pMenuBar, 4u, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE, "disabled command rejects accessibility activate");

	iRet = __xuiMenuBarClickItem(pContext, pMenuBar, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarIsOpen(pMenuBar) && xuiMenuBarGetOpenIndex(pMenuBar) == 0 && xuiMenuIsOpen(pFileMenu), "click opens file");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == pMenuBar, "menubar keeps focus");
	iRet = __xuiMenuBarDispatchUp(pContext, tFileRect.fX + 4.0f, tFileRect.fY + 4.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pFileMenu), "opening release ignored");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pMenuBar, 1, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(tAccessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_COLLAPSE)) &&
		!(tAccessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_EXPAND)),
		"open menu exposes accessibility collapse only");
	iRet = xuiWidgetPerformAccessibleAction(pMenuBar, 2u, XUI_ACCESSIBLE_ACTION_COLLAPSE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuBarIsOpen(pMenuBar), "accessibility collapses menu");
	iRet = xuiWidgetPerformAccessibleAction(pMenuBar, 2u, XUI_ACCESSIBLE_ACTION_EXPAND, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetOpenIndex(pMenuBar) == 0 && xuiMenuIsOpen(pFileMenu),
		"accessibility expands menu");

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
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_END, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetOpenIndex(pMenuBar) == 1 &&
		xuiMenuGetHoverIndex(pEditMenu) == 1,
		"End moves within the open dropdown");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_HOME, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetOpenIndex(pMenuBar) == 1 &&
		xuiMenuGetHoverIndex(pEditMenu) == 0,
		"Home moves within the open dropdown");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_DOWN, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuGetHoverIndex(pEditMenu) == 1, "down moves menu hover");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_ENTER, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelect.iCount == 1 && tSelect.pMenu == pEditMenu && tSelect.iValue == 21, "enter commits open menu");
	XUI_TEST_CHECK(tDirectSelect.iCount == 0, "menubar temporarily overrides direct menu callback");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuBarIsOpen(pMenuBar) &&
		xuiGetFocusWidget(pContext) == pRoot,
		"commit closes menubar and restores prior focus");
	iRet = xuiMenuOpenForOwner(pEditMenu, pMenuBar);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pEditMenu), "open edit menu directly after menubar close");
	iRet = xuiMenuSetHoverIndex(pEditMenu, 0);
	if ( iRet == XUI_OK ) iRet = xuiMenuCommitHover(pEditMenu);
	XUI_TEST_CHECK((iRet == XUI_OK || iRet == XUI_EVENT_DISPATCH_STOP) &&
		tDirectSelect.iCount == 1 && tDirectSelect.iValue == 20 && tSelect.iCount == 1,
		"menubar restores direct menu callback");

	iRet = xuiSetFocusWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus root before F10");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_F10, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pMenuBar &&
		xuiMenuBarGetHoverIndex(pMenuBar) == 0, "global F10 enters menu mode");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_CONTEXT_MENU, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pMenuBar &&
		xuiMenuBarGetHoverIndex(pMenuBar) == 0 && !xuiMenuBarIsOpen(pMenuBar),
		"context-menu key does not activate the menu bar");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_F10, XUI_MOD_SHIFT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pMenuBar &&
		xuiMenuBarGetHoverIndex(pMenuBar) == 0,
		"Shift+F10 is not treated as menu-bar activation");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_F10, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pRoot &&
		xuiMenuBarGetHoverIndex(pMenuBar) == -1, "second F10 exits and restores focus");
	iRet = xuiWidgetCreate(pContext, &pTransientFocus);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pTransientFocus, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pTransientFocus);
	if ( iRet == XUI_OK ) iRet = xuiSetFocusWidget(pContext, pTransientFocus);
	XUI_TEST_CHECK(iRet == XUI_OK, "transient focus target");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_F10, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pMenuBar,
		"F10 captures a transient restore target");
	xuiWidgetDestroy(pTransientFocus);
	pTransientFocus = NULL;
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_F10, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == NULL &&
		xuiMenuBarGetHoverIndex(pMenuBar) == -1,
		"destroyed restore target is cleared before menu-mode exit");
	iRet = xuiSetFocusWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore root after transient target test");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_F10, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pMenuBar &&
		xuiMenuBarGetHoverIndex(pMenuBar) == 0, "F10 re-enters menu mode");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_END, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetHoverIndex(pMenuBar) == 1, "End selects last enabled item");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_HOME, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetHoverIndex(pMenuBar) == 0, "Home selects first enabled item");
	iRet = xuiMenuBarSetItemEnabled(pMenuBar, 2, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarIsItemEnabled(pMenuBar, 2), "enable item");
	iRet = xuiMenuBarSetItemMnemonic(pMenuBar, 2, 'W');
	XUI_TEST_CHECK(iRet == XUI_OK, "set mnemonic");
	iRet = xuiWidgetPerformAccessibleAction(pMenuBar, 4u, XUI_ACCESSIBLE_ACTION_EXPAND, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED, "top command rejects accessibility expand");
	tViewRect = xuiMenuBarGetItemRect(pMenuBar, 2);
	tBarWorld = xuiWidgetGetWorldRect(pMenuBar);
	iBefore = tSelect.iCount;
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_DOWN, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetOpenIndex(pMenuBar) == 0,
		"Down opens the highlighted top-level menu");
	iRet = __xuiMenuBarDispatchMove(pContext,
		tBarWorld.fX + tViewRect.fX + tViewRect.fW * 0.5f,
		tBarWorld.fY + tViewRect.fY + tViewRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelect.iCount == iBefore &&
		!xuiMenuBarIsOpen(pMenuBar) && xuiMenuBarGetHoverIndex(pMenuBar) == 2,
		"hovering a top-level command never activates it");
	iRet = __xuiMenuBarMoveToItem(pContext, pMenuBar, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetOpenIndex(pMenuBar) == 1 &&
		tSelect.iCount == iBefore, "menu tracking resumes after a command item");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuBarIsOpen(pMenuBar) &&
		xuiGetFocusWidget(pContext) == pRoot, "Escape exits pointer menu mode");

	iRet = __xuiMenuBarKey(pContext, 'E', XUI_MOD_ALT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetOpenIndex(pMenuBar) == 1,
		"Alt mnemonic reopens Edit");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_RIGHT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelect.iCount == iBefore &&
		!xuiMenuBarIsOpen(pMenuBar) && xuiMenuBarGetHoverIndex(pMenuBar) == 2,
		"keyboard tracking does not execute a top-level command");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_RIGHT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetOpenIndex(pMenuBar) == 0 &&
		xuiMenuIsOpen(pFileMenu), "keyboard tracking continues past a command item");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pRoot,
		"Escape restores focus after keyboard tracking");

	iRet = __xuiMenuBarKey(pContext, XUI_KEY_F10, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pMenuBar,
		"primary menubar enters mode before handoff");
	iRet = xuiMenuBarCreate(pContext, &pSecondBar, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pSecondBar,
		(xui_rect_t){20.0f, 60.0f, 180.0f, 28.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pSecondBar);
	memset(arrSecondItems, 0, sizeof(arrSecondItems));
	arrSecondItems[0].sText = "&Help";
	arrSecondItems[0].iState = XUI_MENUBAR_ITEM_ENABLED;
	arrSecondItems[0].iValue = 41;
	if ( iRet == XUI_OK ) iRet = xuiMenuBarSetItems(pSecondBar, arrSecondItems, 1);
	if ( iRet == XUI_OK ) iRet = xuiMenuBarSetSelect(pSecondBar,
		__xuiMenuBarSelected, &tSecondSelect);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "secondary menubar setup");
	tViewRect = xuiMenuBarGetItemRect(pSecondBar, 0);
	tBarWorld = xuiWidgetGetWorldRect(pSecondBar);
	iRet = __xuiMenuBarDispatchMove(pContext,
		tBarWorld.fX + tViewRect.fX + tViewRect.fW * 0.5f,
		tBarWorld.fY + tViewRect.fY + tViewRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarDispatchDown(pContext,
		tBarWorld.fX + tViewRect.fX + tViewRect.fW * 0.5f,
		tBarWorld.fY + tViewRect.fY + tViewRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarDispatchUp(pContext,
		tBarWorld.fX + tViewRect.fX + tViewRect.fW * 0.5f,
		tBarWorld.fY + tViewRect.fY + tViewRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetHoverIndex(pMenuBar) == -1 &&
		tSecondSelect.iCount == 1 && tSecondSelect.iValue == 41 &&
		xuiGetFocusWidget(pContext) == pRoot,
		"switching menubars closes the old mode and restores original focus");
	xuiWidgetDestroy(pSecondBar);
	pSecondBar = NULL;
	tViewRect = xuiMenuBarGetItemRect(pMenuBar, 2);
	tBarWorld = xuiWidgetGetWorldRect(pMenuBar);

	iRet = __xuiMenuBarKey(pContext, 'F', XUI_MOD_ALT);
	if ( iRet == XUI_OK ) iRet = xuiMenuSetHoverIndex(pFileMenu, 3);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarKey(pContext, XUI_KEY_RIGHT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pFileMenu) &&
		xuiMenuIsOpen(pRecentMenu), "Right opens the highlighted submenu before switching top-level menus");
	iRet = __xuiMenuBarKey(pContext, XUI_KEY_ESCAPE, 0);
	if ( iRet == XUI_OK ) iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuBarIsOpen(pMenuBar) &&
		xuiGetFocusWidget(pContext) == pRoot, "submenu Escape closes the menu session and restores focus");

	iRet = __xuiMenuBarDispatchMove(pContext,
		tBarWorld.fX + tViewRect.fX + tViewRect.fW * 0.5f,
		tBarWorld.fY + tViewRect.fY + tViewRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarDispatchDown(pContext,
		tBarWorld.fX + tViewRect.fX + tViewRect.fW * 0.5f,
		tBarWorld.fY + tViewRect.fY + tViewRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelect.iCount == 1 && xuiMenuBarGetActiveIndex(pMenuBar) == 2,
		"top command waits for pointer release");
	iRet = __xuiMenuBarDispatchMove(pContext, 500.0f, 180.0f);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarDispatchUp(pContext, 500.0f, 180.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelect.iCount == 1 && xuiMenuBarGetActiveIndex(pMenuBar) == -1,
		"dragging out cancels top command");
	iRet = __xuiMenuBarDispatchMove(pContext,
		tBarWorld.fX + tViewRect.fX + tViewRect.fW * 0.5f,
		tBarWorld.fY + tViewRect.fY + tViewRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarDispatchDown(pContext,
		tBarWorld.fX + tViewRect.fX + tViewRect.fW * 0.5f,
		tBarWorld.fY + tViewRect.fY + tViewRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pMenuBar,
		"top command captures pointer while pressed");
	iRet = xuiMenuBarClose(pMenuBar);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) != pMenuBar &&
		xuiMenuBarGetActiveIndex(pMenuBar) == -1 && tSelect.iCount == 1,
		"closing pressed menubar releases pointer capture");
	iBefore = tSelect.iCount;
	iRet = xuiInputPointerDownEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tBarWorld.fX + tViewRect.fX + tViewRect.fW * 0.5f),
		(int)(tBarWorld.fY + tViewRect.fY + tViewRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pMenuBar &&
		xuiMenuBarGetActiveIndex(pMenuBar) == 2,
		"first touch owns menubar command press");
	iRet = xuiInputPointerDownEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH,
		(int)(tBarWorld.fX + tViewRect.fX + tViewRect.fW * 0.5f),
		(int)(tBarWorld.fY + tViewRect.fY + tViewRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tBarWorld.fX + tViewRect.fX + tViewRect.fW * 0.5f),
		(int)(tBarWorld.fY + tViewRect.fY + tViewRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelect.iCount == iBefore &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pMenuBar &&
		xuiMenuBarGetActiveIndex(pMenuBar) == 2,
		"second touch cannot commit or release menubar command");
	iRet = xuiInputPointerUpEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tBarWorld.fX + tViewRect.fX + tViewRect.fW * 0.5f),
		(int)(tBarWorld.fY + tViewRect.fY + tViewRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelect.iCount == iBefore + 1 &&
		tSelect.iValue == 3 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		xuiMenuBarGetActiveIndex(pMenuBar) == -1,
		"owning touch commits menubar command exactly once");
	iBefore = tSelect.iCount;
	iRet = __xuiMenuBarDispatchMove(pContext,
		tBarWorld.fX + tViewRect.fX + tViewRect.fW * 0.5f,
		tBarWorld.fY + tViewRect.fY + tViewRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarDispatchDown(pContext,
		tBarWorld.fX + tViewRect.fX + tViewRect.fW * 0.5f,
		tBarWorld.fY + tViewRect.fY + tViewRect.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarDispatchUp(pContext,
		tBarWorld.fX + tViewRect.fX + tViewRect.fW * 0.5f,
		tBarWorld.fY + tViewRect.fY + tViewRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelect.iCount == iBefore + 1 && tSelect.iValue == 3,
		"top command activates on pointer release");
	iBefore = tSelect.iCount;
	iRet = __xuiMenuBarKey(pContext, 'W', XUI_MOD_ALT);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelect.iCount == iBefore + 1 && tSelect.iValue == 3, "top command without menu");
	iBefore = tSelect.iCount;
	iRet = xuiMenuBarSetSelect(pMenuBar, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear menubar select callback");
	iRet = xuiMenuBarOpenItem(pMenuBar, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pFileMenu), "open menu without inherited callback");
	iRet = xuiMenuSetHoverIndex(pFileMenu, 0);
	if ( iRet == XUI_OK ) iRet = xuiMenuCommitHover(pFileMenu);
	XUI_TEST_CHECK((iRet == XUI_OK || iRet == XUI_EVENT_DISPATCH_STOP) && tSelect.iCount == iBefore,
		"cleared menubar callback is not retained by child menu");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuBarIsOpen(pMenuBar), "sync closed child menu");
	iRet = xuiMenuBarSetSelect(pMenuBar, __xuiMenuBarSelected, &tSelect);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore menubar select callback");
	iRet = xuiMenuBarGetMetrics(pMenuBar, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK && tMetrics.fHeight >= 18.0f, "metrics get");
	iRet = xuiMenuBarGetColors(pMenuBar, &tColors);
	XUI_TEST_CHECK(iRet == XUI_OK && tColors.iTextColor != 0u, "colors get");
	tBadMetrics = tMetrics;
	tBadMetrics.fItemGap = NAN;
	XUI_TEST_CHECK(xuiMenuBarSetMetrics(pMenuBar, &tBadMetrics) == XUI_ERROR_INVALID_ARGUMENT,
		"nan metrics rejected");
	tBadMetrics = tMetrics;
	tBadMetrics.fPaddingX = XUI_LAYOUT_UNBOUNDED;
	XUI_TEST_CHECK(xuiMenuBarSetMetrics(pMenuBar, &tBadMetrics) == XUI_ERROR_INVALID_ARGUMENT,
		"combined horizontal geometry rejected");
	tBadMetrics = tMetrics;
	tBadMetrics.fItemPaddingX = XUI_LAYOUT_UNBOUNDED * 0.5f;
	XUI_TEST_CHECK(xuiMenuBarSetMetrics(pMenuBar, &tBadMetrics) == XUI_ERROR_INVALID_ARGUMENT,
		"maximum-capacity item geometry rejected");
	iMeasureCount = xuiTestProxyGetTextMeasureCount(&tState);
	XUI_TEST_CHECK(xuiMenuBarSetMetrics(pMenuBar, &tMetrics) == XUI_OK,
		"unchanged metrics accepted");
	(void)xuiMenuBarGetItemRect(pMenuBar, 0);
	XUI_TEST_CHECK(xuiTestProxyGetTextMeasureCount(&tState) == iMeasureCount,
		"unchanged metrics preserve the layout cache");
	XUI_TEST_CHECK(xuiMenuBarSetColors(pMenuBar, &tColors) == XUI_OK,
		"unchanged colors accepted without invalidation");
	iChangeCount = xuiMenuBarGetChangeCount(pMenuBar);
	XUI_TEST_CHECK(xuiMenuBarSetItemEnabled(pMenuBar, 2, 1) == XUI_OK &&
		xuiMenuBarGetChangeCount(pMenuBar) == iChangeCount, "idempotent enable does not change state");
	XUI_TEST_CHECK(xuiMenuBarGetChangeCount(pMenuBar) > 0, "change count");

	memset(sOwnedText, 'L', sizeof(sOwnedText) - 1u);
	sOwnedText[0] = '&';
	sOwnedText[sizeof(sOwnedText) - 1u] = '\0';
	memset(arrOwnedItems, 0, sizeof(arrOwnedItems));
	arrOwnedItems[0].sText = sOwnedText;
	arrOwnedItems[0].iState = XUI_MENUBAR_ITEM_ENABLED;
	iRet = xuiMenuBarSetItems(pMenuBar, arrOwnedItems, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "owned long text set");
	sOwnedText[1] = 'X';
	XUI_TEST_CHECK(xuiMenuBarGetItem(pMenuBar, 0)->sText[1] == 'L', "menubar owns item text");
	tFileRect = xuiMenuBarGetItemRect(pMenuBar, 0);
	XUI_TEST_CHECK(tFileRect.fW > 900.0f, "long label is not truncated to fixed buffer");

	iRet = xuiMenuCreate(pContext, &pLifetimeMenu, &tMenuDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pLifetimeMenu != NULL, "lifetime menu create");
	iRet = xuiMenuBarSetItemMenu(pMenuBar, 0, pLifetimeMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "lifetime menu binding");
	iRet = xuiMenuBarOpenItem(pMenuBar, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuBarGetOpenMenu(pMenuBar) == pLifetimeMenu,
		"lifetime menu opens");
	xuiWidgetDestroy(pLifetimeMenu);
	pLifetimeMenu = NULL;
	XUI_TEST_CHECK(xuiMenuBarGetItem(pMenuBar, 0) != NULL &&
		xuiMenuBarGetItem(pMenuBar, 0)->pMenu == NULL &&
		!xuiMenuBarIsItemEnabled(pMenuBar, 0) &&
		xuiMenuBarGetOpenIndex(pMenuBar) == -1 && xuiMenuBarGetOpenMenu(pMenuBar) == NULL,
		"destroyed menu clears, disables, and closes menu bar binding");

	iRet = xuiMenuBarCreate(pContext, &pDestroyBar, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyBar != NULL, "destroy callback bar create");
	memset(arrDestroyItems, 0, sizeof(arrDestroyItems));
	arrDestroyItems[0].sText = "Command";
	arrDestroyItems[0].iState = XUI_MENUBAR_ITEM_ENABLED;
	arrDestroyItems[0].iValue = 77;
	iRet = xuiMenuBarSetItems(pDestroyBar, arrDestroyItems, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback bar items");
	iRet = xuiMenuBarSetSelect(pDestroyBar, __xuiMenuBarDestroySelected, &tDestroy);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback set");
	iRet = xuiMenuBarOpenItem(pDestroyBar, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroy.iCount == 1, "top command callback may destroy menubar");
	pDestroyBar = NULL;

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
