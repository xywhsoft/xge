#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
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

typedef struct xui_menu_test_destroy_t {
	int iCount;
} xui_menu_test_destroy_t;

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

static void __xuiMenuDestroySelected(xui_widget pMenu, int iIndex, int iValue, void* pUser)
{
	xui_menu_test_destroy_t* pDestroy = (xui_menu_test_destroy_t*)pUser;
	(void)iIndex;
	(void)iValue;
	pDestroy->iCount++;
	xuiWidgetDestroy(pMenu);
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
	float fX;
	float fY;
	int iRet;

	tWorld = xuiWidgetGetWorldRect(pMenu);
	tItem = xuiMenuGetItemRect(pMenu, iIndex);
	fX = tWorld.fX + tItem.fX + tItem.fW * 0.5f;
	fY = tWorld.fY + tItem.fY + tItem.fH * 0.5f;
	iRet = __xuiMenuDispatchDown(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiMenuDispatchUp(pContext, fX, fY);
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
	xui_menu_test_select_t tInheritedSelect;
	xui_menu_test_select_t tShortcutSelect;
	xui_menu_test_destroy_t tDestroy;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pOwner;
	xui_widget pMenu;
	xui_widget pSubmenu;
	xui_widget pDestroyMenu;
	xui_widget pShortcutMenu;
	xui_widget pScrollMenu;
	xui_widget pLifetimeParent;
	xui_widget pLifetimeSubmenu;
	xui_widget pInvalidMenu;
	xui_surface pTarget;
	xui_surface pMenuCache;
	xui_menu_desc_t tDesc;
	xui_menu_item_t arrItems[7];
	xui_menu_item_t arrEquivalent[7];
	xui_menu_item_t arrSubItems[3];
	xui_menu_item_t arrWideItems[1];
	xui_menu_item_t arrCycleItems[1];
	xui_menu_item_t arrDestroyItems[1];
	xui_menu_item_t arrShortcutItems[3];
	xui_menu_item_t arrScrollItems[13];
	xui_menu_metrics_t tMetrics;
	xui_menu_metrics_t tBadMetrics;
	xui_menu_desc_t tBadDesc;
	xui_rect_t tPopupRect;
	xui_rect_t tOwnerRect;
	xui_rect_t tItemRect;
	xui_rect_t tMenuWorld;
	xui_accessible_node_t tAccessible;
	const char* sWideText;
	const char* sWideShortcut;
	const char* sScrollLabel;
	char sOwnedText[32];
	char sOwnedShortcut[32];
	float fWidth;
	float fHeight;
	float fPopupW;
	float fPopupH;
	float fExpectedWidth;
	float fScrollX;
	float fScrollY;
	float fScrollBeforeX;
	int iMeasureCount;
	int iBefore;
	int i;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pOwner = NULL;
	pMenu = NULL;
	pSubmenu = NULL;
	pDestroyMenu = NULL;
	pShortcutMenu = NULL;
	pScrollMenu = NULL;
	pLifetimeParent = NULL;
	pLifetimeSubmenu = NULL;
	pInvalidMenu = NULL;
	pTarget = NULL;
	pMenuCache = NULL;
	iFailed = 0;
	memset(&tSelect, 0, sizeof(tSelect));
	memset(&tInheritedSelect, 0, sizeof(tInheritedSelect));
	memset(&tShortcutSelect, 0, sizeof(tShortcutSelect));
	memset(&tDestroy, 0, sizeof(tDestroy));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 640.0f, 320.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 640.0f, 320.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	iRet = xuiWidgetCreate(pContext, &pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK && pOwner != NULL, "owner create");
	xuiWidgetSetRect(pOwner, (xui_rect_t){36.0f, 30.0f, 120.0f, 28.0f});
	xuiWidgetSetFocusable(pOwner, 1);
	iRet = xuiWidgetAddChild(pRoot, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK, "add owner");

	memset(&tBadDesc, 0, sizeof(tBadDesc));
	tBadDesc.iSize = sizeof(tBadDesc);
	tBadDesc.bHasMetrics = 1;
	tBadDesc.tMetrics.iSize = sizeof(tBadDesc.tMetrics);
	tBadDesc.tMetrics.fItemHeight = NAN;
	tBadDesc.tMetrics.fSeparatorHeight = 9.0f;
	pInvalidMenu = pOwner;
	iRet = xuiMenuCreate(pContext, &pInvalidMenu, &tBadDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidMenu == NULL, "invalid descriptor metrics rejected");
	tBadDesc.tMetrics.fItemHeight = XUI_LAYOUT_UNBOUNDED;
	pInvalidMenu = pOwner;
	iRet = xuiMenuCreate(pContext, &pInvalidMenu, &tBadDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidMenu == NULL,
		"combined descriptor geometry rejected");

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
	arrSubItems[1].sText = "Sub second";
	arrSubItems[1].iType = XUI_MENU_ITEM_NORMAL;
	arrSubItems[1].iState = XUI_MENU_ITEM_ENABLED;
	arrSubItems[1].iValue = 51;
	arrSubItems[2].sText = "Sub third";
	arrSubItems[2].iType = XUI_MENU_ITEM_NORMAL;
	arrSubItems[2].iState = XUI_MENU_ITEM_ENABLED;
	arrSubItems[2].iValue = 52;
	iRet = xuiMenuSetItems(pSubmenu, arrSubItems, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "submenu items");

	iRet = xuiMenuCreate(pContext, &pMenu, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pMenu != NULL, "menu create");
	iRet = xuiWidgetSetAccessibleName(pMenu, "Application menu");
	XUI_TEST_CHECK(iRet == XUI_OK, "accessible name");
	iRet = xuiWidgetSetAccessibleDescription(pMenu, "Application commands");
	XUI_TEST_CHECK(iRet == XUI_OK, "accessible description");
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
	iMeasureCount = xuiTestProxyGetTextMeasureCount(&tState);
	iRet = __xuiMenuMoveToItem(pContext, pMenu, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "cached measure pointer move");
	iRet = __xuiMenuMoveToItem(pContext, pMenu, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestProxyGetTextMeasureCount(&tState) == iMeasureCount,
		"pointer moves reuse menu text measurements");
	iRet = xuiPopupGetContentSize(xuiMenuGetPopupWidget(pMenu), &fPopupW, &fPopupH);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiMenuNear(fWidth, fPopupW) && __xuiMenuNear(fHeight, fPopupH), "popup content size");
	XUI_TEST_CHECK(xuiMenuGetContentWidget(pMenu) == pMenu && xuiMenuGetOwner(pMenu) == pOwner, "menu internals");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == pMenu, "menu focused");
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pMenu) == 8, "menu accessibility exposes logical items");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pMenu, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_MENU &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_EXPANDED) &&
		strcmp(tAccessible.sName, "Application menu") == 0 &&
		strcmp(tAccessible.sDescription, "Application commands") == 0 &&
		tAccessible.iRowCount == 7 && tAccessible.iColumnCount == 1 &&
		(tAccessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS)),
		"menu accessibility root");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pMenu, 2, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_MENU_ITEM &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_CHECKED) && tAccessible.iRow == 1 &&
		tAccessible.iColumn == 0 && tAccessible.iRowCount == 1 && tAccessible.iColumnCount == 1 &&
		strcmp(tAccessible.sDescription, "G") == 0 &&
		(tAccessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_TOGGLE)) &&
		(tAccessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS)),
		"menu accessibility checked item");
	iRet = xuiWidgetPerformAccessibleAction(pMenu, 2u, XUI_ACCESSIBLE_ACTION_TOGGLE, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED && xuiMenuIsOpen(pMenu),
		"normal item rejects accessibility toggle");
	iRet = xuiWidgetPerformAccessibleAction(pMenu, 3u, XUI_ACCESSIBLE_ACTION_EXPAND, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED && xuiMenuIsOpen(pMenu),
		"check item rejects accessibility expand");
	iRet = xuiWidgetPerformAccessibleAction(pMenu, 7u, XUI_ACCESSIBLE_ACTION_COLLAPSE, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE, "collapsed submenu rejects accessibility collapse");
	iRet = xuiWidgetPerformAccessibleAction(pMenu, 7u, XUI_ACCESSIBLE_ACTION_EXPAND, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pSubmenu), "accessibility expands submenu");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pMenu, 6, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(tAccessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_COLLAPSE)) &&
		!(tAccessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_EXPAND)),
		"expanded submenu exposes collapse only");
	iRet = xuiWidgetPerformAccessibleAction(pMenu, 7u, XUI_ACCESSIBLE_ACTION_COLLAPSE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuIsOpen(pSubmenu), "accessibility collapses submenu");
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
	iRet = xuiInputKeyDown(pContext, XUI_KEY_END, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiMenuGetHoverIndex(pMenu) == 5, "End selects last enabled item");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_HOME, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiMenuGetHoverIndex(pMenu) == 0, "Home selects first enabled item");
	iRet = __xuiMenuClickItem(pContext, pMenu, 6);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "disabled does not close");
	XUI_TEST_CHECK(tSelect.iCount == 3, "disabled no select");

	iRet = __xuiMenuMoveToItem(pContext, pMenu, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "move to submenu item");
	iRet = xuiUpdate(pContext, 0.19f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuIsOpen(pSubmenu), "submenu waits for hover delay");
	iRet = xuiUpdate(pContext, 0.02f);
	XUI_TEST_CHECK(iRet == XUI_OK, "submenu hover delay update");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "submenu layout");
	XUI_TEST_CHECK(xuiMenuIsOpen(pSubmenu), "submenu opened on hover");
	for ( i = 0; i < 7; i++ ) arrEquivalent[i] = *xuiMenuGetItem(pMenu, i);
	iMeasureCount = xuiTestProxyGetTextMeasureCount(&tState);
	iRet = xuiMenuSetItems(pMenu, arrEquivalent, 7);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuGetHoverIndex(pMenu) == 5 &&
		xuiMenuIsOpen(pSubmenu) &&
		xuiTestProxyGetTextMeasureCount(&tState) == iMeasureCount,
		"repeated items preserve active submenu and measurement cache");
	tMenuWorld = xuiWidgetGetWorldRect(pMenu);
	tItemRect = xuiMenuGetItemRect(pMenu, 6);
	iRet = __xuiMenuDispatchMove(pContext,
		tMenuWorld.fX + tItemRect.fX + tItemRect.fW - 5.0f,
		tMenuWorld.fY + tItemRect.fY + tItemRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuGetHoverIndex(pMenu) == 5 && xuiMenuIsOpen(pSubmenu),
		"diagonal submenu path keeps current item");
	iRet = xuiUpdate(pContext, 0.10f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuGetHoverIndex(pMenu) == 5 && xuiMenuIsOpen(pSubmenu),
		"submenu aim grace period");
	iRet = xuiUpdate(pContext, 0.21f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuGetHoverIndex(pMenu) == -1 && !xuiMenuIsOpen(pSubmenu),
		"submenu aim eventually applies deferred disabled row");
	iRet = __xuiMenuMoveToItem(pContext, pMenu, 5);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiUpdate(pContext, 0.21f) == XUI_OK && xuiMenuIsOpen(pSubmenu),
		"submenu reopens after aim deferral");
	iRet = __xuiMenuClickItem(pContext, pSubmenu, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuIsOpen(pMenu) && !xuiMenuIsOpen(pSubmenu), "submenu select closes root");
	XUI_TEST_CHECK(tSelect.iCount == 4 && tSelect.pMenu == pSubmenu && tSelect.iIndex == 0 && tSelect.iValue == 50, "submenu inherited callback");

	iRet = xuiMenuSetSelect(pMenu, __xuiMenuSelected, &tInheritedSelect);
	XUI_TEST_CHECK(iRet == XUI_OK, "replace parent select callback");
	iRet = xuiMenuOpenForOwner(pMenu, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK, "open for refreshed inherited callback");
	iRet = xuiMenuSetHoverIndex(pMenu, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "hover submenu for refreshed callback");
	iRet = xuiMenuCommitHover(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pSubmenu), "open submenu for refreshed callback");
	iRet = __xuiMenuClickItem(pContext, pSubmenu, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && tInheritedSelect.iCount == 1 && tInheritedSelect.pMenu == pSubmenu,
		"submenu resolves current parent callback without stale inheritance");
	iRet = xuiMenuSetSelect(pMenu, __xuiMenuSelected, &tSelect);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore parent select callback");

	memset(arrCycleItems, 0, sizeof(arrCycleItems));
	arrCycleItems[0].sText = "Cycle";
	arrCycleItems[0].iType = XUI_MENU_ITEM_SUBMENU;
	arrCycleItems[0].iState = XUI_MENU_ITEM_ENABLED;
	arrCycleItems[0].pSubmenu = pMenu;
	iRet = xuiMenuSetItems(pSubmenu, arrCycleItems, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "configure deferred submenu cycle");
	iRet = xuiMenuOpenForOwner(pMenu, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK, "open cycle parent");
	iRet = xuiMenuSetHoverIndex(pMenu, 5);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuCommitHover(pMenu) == XUI_OK, "open cycle child");
	iRet = xuiMenuSetHoverIndex(pSubmenu, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuCommitHover(pSubmenu) == XUI_ERROR_INVALID_STATE,
		"runtime submenu cycle rejected");
	iRet = xuiMenuClose(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "close cycle menu chain");
	iRet = xuiMenuSetItems(pSubmenu, arrSubItems, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore submenu items");

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
	iMeasureCount = xuiTestProxyGetTextMeasureCount(&tState);
	iRet = xuiMenuSetMetrics(pMenu, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiTestProxyGetTextMeasureCount(&tState) == iMeasureCount,
		"unchanged metrics avoid redundant measurement");

	tBadMetrics = tMetrics;
	tBadMetrics.fPaddingX = NAN;
	iRet = xuiMenuSetMetrics(pMenu, &tBadMetrics);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "nan metrics rejected");
	tBadMetrics = tMetrics;
	tBadMetrics.fItemHeight = XUI_LAYOUT_UNBOUNDED;
	iRet = xuiMenuSetMetrics(pMenu, &tBadMetrics);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "combined height metrics rejected");
	tBadMetrics = tMetrics;
	tBadMetrics.fPaddingX = XUI_LAYOUT_UNBOUNDED;
	iRet = xuiMenuSetMetrics(pMenu, &tBadMetrics);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "combined width metrics rejected");
	iRet = xuiMenuOpenAt(pMenu, pOwner, NAN, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && !xuiMenuIsOpen(pMenu), "nan popup coordinate rejected");
	iRet = xuiMenuOpenAt(pMenu, pOwner, XUI_LAYOUT_UNBOUNDED + 1024.0f, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && !xuiMenuIsOpen(pMenu),
		"out-of-range popup coordinate rejected before integer conversion");
	iRet = xuiMenuSetItemState(pMenu, 0, 0x80000000u, 0x80000000u);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "unknown item state rejected");

	strcpy(sOwnedText, "Owned label");
	strcpy(sOwnedShortcut, "Ctrl+O");
	arrWideItems[0].sText = sOwnedText;
	arrWideItems[0].sShortcut = sOwnedShortcut;
	iRet = xuiMenuSetItems(pMenu, arrWideItems, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "owned item set");
	strcpy(sOwnedText, "Changed");
	strcpy(sOwnedShortcut, "Changed");
	XUI_TEST_CHECK(strcmp(xuiMenuGetItem(pMenu, 0)->sText, "Owned label") == 0 &&
		strcmp(xuiMenuGetItem(pMenu, 0)->sShortcut, "Ctrl+O") == 0, "menu owns text copies");

	iRet = xuiMenuOpenForOwner(pMenu, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK, "open multi-touch menu");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi-touch menu layout");
	tMenuWorld = xuiWidgetGetWorldRect(pMenu);
	tItemRect = xuiMenuGetItemRect(pMenu, 0);
	iBefore = tSelect.iCount;
	iRet = xuiInputPointerDownEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tMenuWorld.fX + tItemRect.fX + tItemRect.fW * 0.5f),
		(int)(tMenuWorld.fY + tItemRect.fY + tItemRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pMenu,
		"first touch owns menu item press");
	iRet = xuiInputPointerDownEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH,
		(int)(tMenuWorld.fX + tItemRect.fX + tItemRect.fW * 0.5f),
		(int)(tMenuWorld.fY + tItemRect.fY + tItemRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tMenuWorld.fX + tItemRect.fX + tItemRect.fW * 0.5f),
		(int)(tMenuWorld.fY + tItemRect.fY + tItemRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelect.iCount == iBefore && xuiMenuIsOpen(pMenu) &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pMenu,
		"second touch cannot commit or release menu item");
	iRet = xuiInputPointerUpEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tMenuWorld.fX + tItemRect.fX + tItemRect.fW * 0.5f),
		(int)(tMenuWorld.fY + tItemRect.fY + tItemRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelect.iCount == iBefore + 1 &&
		tSelect.iValue == arrWideItems[0].iValue && !xuiMenuIsOpen(pMenu) &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch commits menu item exactly once");

	iRet = xuiMenuOpenForOwner(pMenu, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK, "open disable-during-press menu");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable-during-press menu layout");
	tMenuWorld = xuiWidgetGetWorldRect(pMenu);
	tItemRect = xuiMenuGetItemRect(pMenu, 0);
	iRet = xuiInputPointerDownEx(pContext, 503u, XUI_POINTER_TYPE_TOUCH,
		(int)(tMenuWorld.fX + tItemRect.fX + tItemRect.fW * 0.5f),
		(int)(tMenuWorld.fY + tItemRect.fY + tItemRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiMenuSetItemState(pMenu, 0,
		XUI_MENU_ITEM_ENABLED, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 503u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"disabling pressed menu item releases owner capture");
	iRet = xuiInputPointerUpEx(pContext, 503u, XUI_POINTER_TYPE_TOUCH,
		(int)(tMenuWorld.fX + tItemRect.fX + tItemRect.fW * 0.5f),
		(int)(tMenuWorld.fY + tItemRect.fY + tItemRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiMenuSetItemState(pMenu, 0,
		XUI_MENU_ITEM_ENABLED, XUI_MENU_ITEM_ENABLED);
	if ( iRet == XUI_OK ) iRet = xuiMenuClose(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelect.iCount == iBefore + 1,
		"release after disabling menu item does not commit");

	iRet = xuiMenuOpenForOwner(pMenu, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK, "open drag cancel menu");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag cancel layout");
	tPopupRect = xuiPopupGetPopupRect(xuiMenuGetPopupWidget(pMenu));
	iRet = __xuiMenuDispatchDown(pContext, tPopupRect.fX + 12.0f, tPopupRect.fY + 12.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag cancel down");
	iRet = __xuiMenuDispatchUp(pContext, tPopupRect.fX + tPopupRect.fW + 30.0f, tPopupRect.fY + 12.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "dragging outside cancels activation");
	iRet = xuiMenuClose(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "close drag cancel menu");

	iRet = xuiMenuCreate(pContext, &pShortcutMenu, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pShortcutMenu != NULL, "shortcut validation menu create");
	memset(arrShortcutItems, 0, sizeof(arrShortcutItems));
	arrShortcutItems[0].sText = "Malformed multiple keys";
	arrShortcutItems[0].sShortcut = "Ctrl+A+B";
	arrShortcutItems[0].iType = XUI_MENU_ITEM_NORMAL;
	arrShortcutItems[0].iState = XUI_MENU_ITEM_ENABLED;
	arrShortcutItems[0].iValue = 101;
	arrShortcutItems[1].sText = "Malformed function key";
	arrShortcutItems[1].sShortcut = "F1x";
	arrShortcutItems[1].iType = XUI_MENU_ITEM_NORMAL;
	arrShortcutItems[1].iState = XUI_MENU_ITEM_ENABLED;
	arrShortcutItems[1].iValue = 102;
	arrShortcutItems[2].sText = "Valid function key";
	arrShortcutItems[2].sShortcut = "F12";
	arrShortcutItems[2].iType = XUI_MENU_ITEM_NORMAL;
	arrShortcutItems[2].iState = XUI_MENU_ITEM_ENABLED;
	arrShortcutItems[2].iValue = 103;
	iRet = xuiMenuSetItems(pShortcutMenu, arrShortcutItems, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "shortcut validation items");
	iRet = xuiMenuSetSelect(pShortcutMenu, __xuiMenuSelected, &tShortcutSelect);
	XUI_TEST_CHECK(iRet == XUI_OK, "shortcut validation callback");
	iRet = xuiMenuOpenForOwner(pShortcutMenu, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pShortcutMenu), "shortcut validation menu open");
	iRet = xuiInputKeyDown(pContext, 'B', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiMenuIsOpen(pShortcutMenu) && tShortcutSelect.iCount == 0,
		"multiple key shortcut rejected");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_F1, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiMenuIsOpen(pShortcutMenu) && tShortcutSelect.iCount == 0,
		"malformed function key shortcut rejected");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_F12, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK &&
		!xuiMenuIsOpen(pShortcutMenu) && tShortcutSelect.iCount == 1 &&
		tShortcutSelect.iIndex == 2 && tShortcutSelect.iValue == 103,
		"valid function key shortcut activates");

	iRet = xuiMenuCreate(pContext, &pScrollMenu, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pScrollMenu != NULL, "scroll menu create");
	iRet = xuiMenuGetMetrics(pScrollMenu, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK, "scroll menu metrics");
	tMetrics.fMaxHeight = 76.0f;
	iRet = xuiMenuSetMetrics(pScrollMenu, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK, "scroll menu max height");
	sScrollLabel = "A deliberately wide menu command used to verify that vertical "
		"keyboard navigation preserves horizontal scrolling";
	memset(arrScrollItems, 0, sizeof(arrScrollItems));
	for ( iBefore = 0; iBefore < 12; iBefore++ ) {
		arrScrollItems[iBefore].sText = sScrollLabel;
		arrScrollItems[iBefore].iType = XUI_MENU_ITEM_NORMAL;
		arrScrollItems[iBefore].iState = XUI_MENU_ITEM_ENABLED;
		arrScrollItems[iBefore].iValue = 200 + iBefore;
	}
	arrScrollItems[12].sText = "Appended while pressed";
	arrScrollItems[12].iType = XUI_MENU_ITEM_NORMAL;
	arrScrollItems[12].iState = XUI_MENU_ITEM_ENABLED;
	arrScrollItems[12].iValue = 212;
	iRet = xuiMenuSetItems(pScrollMenu, arrScrollItems, 12);
	XUI_TEST_CHECK(iRet == XUI_OK, "scroll menu items");
	iRet = xuiMenuOpenForOwner(pScrollMenu, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pScrollMenu), "scroll menu open");
	iRet = xuiPopupSetScroll(xuiMenuGetPopupWidget(pScrollMenu), 48.0f, 0.0f);
	if ( iRet == XUI_OK ) iRet = xuiPopupGetScroll(xuiMenuGetPopupWidget(pScrollMenu),
		&fScrollBeforeX, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK && fScrollBeforeX > 0.0f && __xuiMenuNear(fScrollY, 0.0f),
		"scroll menu starts with horizontal offset");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_END, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiPopupGetScroll(xuiMenuGetPopupWidget(pScrollMenu),
		&fScrollX, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuGetHoverIndex(pScrollMenu) == 11 &&
		fScrollY > 0.0f && __xuiMenuNear(fScrollX, fScrollBeforeX),
		"End reveals last item without changing horizontal scroll");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_HOME, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiPopupGetScroll(xuiMenuGetPopupWidget(pScrollMenu),
		&fScrollX, &fScrollY);
	tItemRect = xuiMenuGetItemRect(pScrollMenu, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuGetHoverIndex(pScrollMenu) == 0 &&
		fScrollY <= (float)tItemRect.fY && __xuiMenuNear(fScrollX, fScrollBeforeX),
		"Home reveals first item without changing horizontal scroll");
	tMenuWorld = xuiWidgetGetWorldRect(pScrollMenu);
	tPopupRect = xuiPopupGetPopupRect(xuiMenuGetPopupWidget(pScrollMenu));
	iRet = xuiInputPointerDownEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)(tPopupRect.fX + 20.0f),
		(int)(tMenuWorld.fY + tItemRect.fY + tItemRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) == pScrollMenu,
		"scroll menu press owns touch capture");
	iRet = xuiMenuAddItem(pScrollMenu, &arrScrollItems[12]);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuGetItemCount(pScrollMenu) == 13 &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"adding an item cancels the active menu press");
	iRet = xuiInputPointerUpEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)(tPopupRect.fX + 20.0f),
		(int)(tMenuWorld.fY + tItemRect.fY + tItemRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pScrollMenu),
		"release after item mutation does not activate stale press");
	iRet = xuiMenuClose(pScrollMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "scroll menu close");

	iRet = xuiMenuCreate(pContext, &pLifetimeParent, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pLifetimeParent != NULL, "lifetime parent create");
	iRet = xuiMenuCreate(pContext, &pLifetimeSubmenu, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pLifetimeSubmenu != NULL, "lifetime submenu create");
	memset(arrDestroyItems, 0, sizeof(arrDestroyItems));
	arrDestroyItems[0].sText = "Transient submenu";
	arrDestroyItems[0].iType = XUI_MENU_ITEM_SUBMENU;
	arrDestroyItems[0].iState = XUI_MENU_ITEM_ENABLED;
	arrDestroyItems[0].pSubmenu = pLifetimeSubmenu;
	iRet = xuiMenuSetItems(pLifetimeParent, arrDestroyItems, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "lifetime submenu binding");
	xuiWidgetDestroy(pLifetimeSubmenu);
	pLifetimeSubmenu = NULL;
	XUI_TEST_CHECK(xuiMenuGetItem(pLifetimeParent, 0) != NULL &&
		xuiMenuGetItem(pLifetimeParent, 0)->pSubmenu == NULL &&
		(xuiMenuGetItemState(pLifetimeParent, 0) & XUI_MENU_ITEM_ENABLED) == 0u,
		"destroyed submenu clears and disables parent binding");
	xuiWidgetDestroy(pLifetimeParent);
	pLifetimeParent = NULL;

	iRet = xuiMenuCreate(pContext, &pDestroyMenu, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyMenu != NULL, "destroy callback menu create");
	memset(arrDestroyItems, 0, sizeof(arrDestroyItems));
	arrDestroyItems[0].sText = "Destroy";
	arrDestroyItems[0].iType = XUI_MENU_ITEM_NORMAL;
	arrDestroyItems[0].iState = XUI_MENU_ITEM_ENABLED;
	arrDestroyItems[0].iValue = 91;
	iRet = xuiMenuSetItems(pDestroyMenu, arrDestroyItems, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback menu items");
	iRet = xuiMenuSetSelect(pDestroyMenu, __xuiMenuDestroySelected, &tDestroy);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback set");
	iRet = xuiMenuOpenForOwner(pDestroyMenu, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback menu open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback layout");
	iRet = __xuiMenuClickItem(pContext, pDestroyMenu, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroy.iCount == 1, "select callback may destroy menu");
	pDestroyMenu = NULL;

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
