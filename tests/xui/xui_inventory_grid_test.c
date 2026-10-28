#include "xui.h"
#include "src/xui/core/xui_xrt_port.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_inventory_grid_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_inventory_grid_test_state_t {
	int iSelects;
	int iLastSelect;
	int iActivates;
	int iLastActivate;
	int iContexts;
	int iLastContext;
	int iDrags;
	int iLastDrag;
	int iDrops;
	int iDropCalls;
	int iDropAccept;
	int iLastDropFrom;
	int iLastDropTo;
	int iLastDropMode;
	int iSplits;
	int iLastSplitSlot;
	int iLastSplitCount;
	int iTooltips;
	int iLastTooltipSlot;
	int iRenders;
	int iAnimationRenders;
	int iAnimationSlot;
	uint32_t iAnimationFlags;
	uint32_t iAnimationTint;
} xui_inventory_grid_test_state_t;

typedef struct xui_inventory_grid_reentry_state_t {
	int iCalls;
	int iNewSlotCount;
} xui_inventory_grid_reentry_state_t;

static int __xuiInventoryGridTestDispatch(xui_context pContext)
{
	return xuiDispatchPendingEvents(pContext);
}

static void __xuiInventoryGridTestSelect(xui_widget pWidget, int iSlot, void* pUser)
{
	xui_inventory_grid_test_state_t* pState;

	(void)pWidget;
	pState = (xui_inventory_grid_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iSelects++;
		pState->iLastSelect = iSlot;
	}
}

static void __xuiInventoryGridTestActivate(xui_widget pWidget, int iSlot, int iButton, void* pUser)
{
	xui_inventory_grid_test_state_t* pState;

	(void)pWidget;
	(void)iButton;
	pState = (xui_inventory_grid_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iActivates++;
		pState->iLastActivate = iSlot;
	}
}

static void __xuiInventoryGridTestContext(xui_widget pWidget, int iSlot, float fX, float fY, void* pUser)
{
	xui_inventory_grid_test_state_t* pState;

	(void)pWidget;
	(void)fX;
	(void)fY;
	pState = (xui_inventory_grid_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iContexts++;
		pState->iLastContext = iSlot;
	}
}

static int __xuiInventoryGridTestDrag(xui_widget pWidget, int iSlot, void* pUser)
{
	xui_inventory_grid_test_state_t* pState;

	(void)pWidget;
	pState = (xui_inventory_grid_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iDrags++;
		pState->iLastDrag = iSlot;
	}
	return 1;
}

static int __xuiInventoryGridTestDisablingDrag(xui_widget pWidget, int iSlot, void* pUser)
{
	int* pCalls = (int*)pUser;

	(void)iSlot;
	if ( pCalls != NULL ) {
		(*pCalls)++;
	}
	(void)xuiWidgetSetEnabled(pWidget, 0);
	return 1;
}

static int __xuiInventoryGridTestDrop(xui_widget pWidget, int iFromSlot, int iToSlot, int iDropMode, void* pUser)
{
	xui_inventory_grid_test_state_t* pState;

	(void)pWidget;
	pState = (xui_inventory_grid_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iDropCalls++;
		pState->iLastDropFrom = iFromSlot;
		pState->iLastDropTo = iToSlot;
		pState->iLastDropMode = iDropMode;
		if ( !pState->iDropAccept ) {
			return 0;
		}
		pState->iDrops++;
	}
	return 1;
}

static void __xuiInventoryGridTestSplit(xui_widget pWidget, int iSlot, int iCount, void* pUser)
{
	xui_inventory_grid_test_state_t* pState;

	(void)pWidget;
	pState = (xui_inventory_grid_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iSplits++;
		pState->iLastSplitSlot = iSlot;
		pState->iLastSplitCount = iCount;
	}
}

static int __xuiInventoryGridTestTooltip(xui_widget pWidget, int iSlot, const xui_inventory_slot_t* pSlot, char* sBuffer, int iCapacity, void* pUser)
{
	xui_inventory_grid_test_state_t* pState;

	(void)pWidget;
	pState = (xui_inventory_grid_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iTooltips++;
		pState->iLastTooltipSlot = iSlot;
	}
	if ( (sBuffer != NULL) && (iCapacity > 0) && (pSlot != NULL) ) {
		snprintf(sBuffer, (size_t)iCapacity, "Tooltip item=%d count=%d", pSlot->iItemId, pSlot->iCount);
		sBuffer[iCapacity - 1] = '\0';
	}
	return XUI_OK;
}

static int __xuiInventoryGridTestRender(xui_widget pWidget, int iSlot, const xui_inventory_slot_t* pSlot, xui_draw_context_t* pDraw, xui_rect_t tRect, uint32_t iState, void* pUser)
{
	xui_inventory_grid_test_state_t* pState;

	(void)pWidget;
	(void)iSlot;
	(void)pSlot;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	pState = (xui_inventory_grid_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iRenders++;
	}
	return 0;
}

static int __xuiInventoryGridTestAnimationRender(xui_widget pWidget, int iSlot, const xui_inventory_slot_t* pSlot, xui_animation_object_t* pAnimation, xui_draw_context_t* pDraw, xui_rect_t tRect, uint32_t iState, uint32_t iFlags, uint32_t iTint, void* pUser)
{
	xui_inventory_grid_test_state_t* pState;

	(void)pWidget;
	(void)pSlot;
	(void)pAnimation;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	pState = (xui_inventory_grid_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iAnimationRenders++;
		pState->iAnimationSlot = iSlot;
		pState->iAnimationFlags = iFlags;
		pState->iAnimationTint = iTint;
	}
	return XUI_OK;
}

static int __xuiInventoryGridTestMinCountFilter(xui_widget pWidget, int iSlot, const xui_inventory_slot_t* pSlot, void* pUser)
{
	int* pMinCount;

	(void)pWidget;
	(void)iSlot;
	pMinCount = (int*)pUser;
	if ( (pSlot == NULL) || (pMinCount == NULL) ) return 0;
	return pSlot->iCount >= *pMinCount;
}

static int __xuiInventoryGridTestMutatingFilter(xui_widget pWidget, int iSlot, const xui_inventory_slot_t* pSlot, void* pUser)
{
	xui_inventory_grid_reentry_state_t* pState = (xui_inventory_grid_reentry_state_t*)pUser;
	(void)iSlot;
	(void)pSlot;
	if ( pState != NULL && pState->iCalls++ == 0 ) {
		(void)xuiInventoryGridSetSlotCount(pWidget, pState->iNewSlotCount);
	}
	return 1;
}

static void __xuiInventoryGridTestMutatingSelect(xui_widget pWidget, int iSlot, void* pUser)
{
	xui_inventory_grid_reentry_state_t* pState = (xui_inventory_grid_reentry_state_t*)pUser;

	(void)iSlot;
	if ( (pState != NULL) && (pState->iCalls++ == 0) ) {
		(void)xuiInventoryGridSetSlotCount(pWidget, pState->iNewSlotCount);
	}
}

static int __xuiInventoryGridTestDestroyingFilter(xui_widget pWidget, int iSlot, const xui_inventory_slot_t* pSlot, void* pUser)
{
	int* pCalls = (int*)pUser;
	(void)iSlot;
	(void)pSlot;
	if ( pCalls != NULL && (*pCalls)++ == 0 ) xuiWidgetDestroy(pWidget);
	return 1;
}

static int __xuiInventoryGridTestDestroyingContextFilter(xui_widget pWidget, int iSlot, const xui_inventory_slot_t* pSlot, void* pUser)
{
	xui_context pContext = (xui_context)pUser;
	(void)pWidget;
	(void)iSlot;
	(void)pSlot;
	xuiDestroy(pContext);
	return 1;
}

static int __xuiInventoryGridTestDestroyingCompare(xui_widget pWidget, int iSlotA, const xui_inventory_slot_t* pSlotA,
	int iSlotB, const xui_inventory_slot_t* pSlotB, void* pUser)
{
	int* pCalls = (int*)pUser;
	(void)iSlotA;
	(void)pSlotA;
	(void)iSlotB;
	(void)pSlotB;
	if ( pCalls != NULL && (*pCalls)++ == 0 ) xuiWidgetDestroy(pWidget);
	return 0;
}

static void __xuiInventoryGridTestDestroyingSplit(xui_widget pWidget, int iSlot, int iCount, void* pUser)
{
	int* pCalls = (int*)pUser;
	(void)iSlot;
	(void)iCount;
	if ( pCalls != NULL ) (*pCalls)++;
	xuiWidgetDestroy(pWidget);
}

static void __xuiInventoryGridTestSlot(xui_inventory_slot_t* pSlot, int iSlotId, int iItemId, int iCount, const char* sText, const char* sHotkey, uint32_t iQualityColor)
{
	memset(pSlot, 0, sizeof(*pSlot));
	pSlot->iSize = sizeof(*pSlot);
	pSlot->iSlotId = iSlotId;
	pSlot->iItemId = iItemId;
	pSlot->iCount = iCount;
	pSlot->iMaxCount = 99;
	pSlot->iQualityColor = iQualityColor;
	pSlot->iIconTint = XUI_COLOR_WHITE;
	pSlot->fDurabilityRate = 0.75f;
	if ( sText != NULL ) {
		strncpy(pSlot->sText, sText, sizeof(pSlot->sText) - 1u);
		pSlot->sText[sizeof(pSlot->sText) - 1u] = '\0';
	}
	if ( sHotkey != NULL ) {
		strncpy(pSlot->sHotkey, sHotkey, sizeof(pSlot->sHotkey) - 1u);
		pSlot->sHotkey[sizeof(pSlot->sHotkey) - 1u] = '\0';
	}
}

int main(void)
{
	xui_test_proxy_state_t tProxyState;
	xui_inventory_grid_test_state_t tState;
	xui_context pContext;
	xui_context pTempContext;
	xui_widget pRoot;
	xui_widget pGrid;
	xui_widget pChild;
	xui_widget pTempGrid;
	xui_surface pTarget;
	xui_surface pIcon;
	xui_surface pCache;
	xui_font pFont;
	xui_inventory_grid_desc_t tDesc;
	xui_inventory_grid_layout_t tLayout;
	xui_inventory_grid_layout_t tCompactLayout;
	xui_inventory_grid_colors_t tColors;
	xui_style_property_t arrStyle[3];
	xui_inventory_slot_t tSlot;
	xui_inventory_slot_t tOutSlot;
	xui_accessible_node_t tAccessible;
	xui_inventory_hit_t tHit;
	xui_inventory_visible_range_t tVisibleRange;
	xui_inventory_visible_range_t tPaintRange;
	xui_inventory_slot_query_t tQuery;
	xui_inventory_gamepad_profile_t tGamepadProfile;
	xui_inventory_grid_reentry_state_t tReentry;
	xui_event_t tPointer;
	xui_rect_t tSlot0;
	xui_rect_t tSlot2;
	xui_rect_t tSlot5;
	xui_rect_t tSlot4;
	xui_rect_t tSlot6;
	xui_rect_t tStyledSlot4;
	xui_rect_t tWorld;
	xui_rect_t tTooltipRect;
	xui_rect_i_t tFullRect;
	xui_scroll_model_t* pScroll;
	xui_widget pSplitInput;
	xui_widget pSplitPopup;
	xui_animation_object_t* pAnimation;
	xvalue* pXsonValue;
	xvalue* pSavedXsonValue;
	xvalue* pKind;
	xvalue* pSlots;
	const char* sKind;
	const char* sXsonPath;
	uint32_t iAnimationFlags;
	uint32_t iAnimationTint;
	float fAnimationScale;
	float fScrollX;
	float fScrollY;
	char sXson[32768];
	int arrQuerySlots[32];
	int iQueryCount;
	int iMinCount;
	int iMeshDrawBefore;
	int iCallbackCalls;
	int iChangeBefore;
	int iCurrentBefore;
	int iActivatesBefore;
	int iSelectedCount;
	int i;
	int iFailed;
	int iRet;

	pContext = NULL;
	pTempContext = NULL;
	pRoot = NULL;
	pGrid = NULL;
	pChild = NULL;
	pTempGrid = NULL;
	pTarget = NULL;
	pIcon = NULL;
	pCache = NULL;
	pFont = NULL;
	pXsonValue = NULL;
	pSavedXsonValue = NULL;
	sXsonPath = "build\\xui_inventory_grid_state.xson";
	iFailed = 0;
	memset(&tState, 0, sizeof(tState));
	tState.iDropAccept = 1;
	xuiTestProxyInit(&tProxyState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tProxyState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tProxyState.tProxy.fontLoadMemory(&tProxyState.tProxy, &pFont, "inventorygrid", 13, 13.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 360.0f, 240.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = xuiTestSurfaceCreate(&tProxyState, &pIcon, 24, 24, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && pIcon != NULL, "icon surface create");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	(void)xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 360.0f, 240.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	__xuiInventoryGridTestSlot(&tSlot, 0, 1, 1, "Invalid", "", XUI_COLOR_WHITE);
	tSlot.iSize = sizeof(tSlot) - 1u;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrSlots = &tSlot;
	tDesc.iSlotCount = 1;
	pTempGrid = pRoot;
	iRet = xuiInventoryGridCreate(pContext, &pTempGrid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pTempGrid == NULL, "descriptor rejects malformed initial slot");

	memset(&tLayout, 0, sizeof(tLayout));
	tLayout.iSize = sizeof(tLayout);
	tLayout.iColumns = 4;
	tLayout.iSelectionMode = XUI_INVENTORY_SELECTION_MULTI;
	tLayout.fSlotSize = 36.0f;
	tLayout.fSlotGap = 4.0f;
	tLayout.fPadding = 6.0f;
	tLayout.fIconPadding = 7.0f;
	tLayout.fBorderWidth = 1.0f;
	tLayout.fWheelStep = 40.0f;
	tLayout.fDragThreshold = 4.0f;
	memset(&tColors, 0, sizeof(tColors));
	tColors.iSize = sizeof(tColors);
	tColors.iSelectedColor = XUI_COLOR_RGBA(44, 126, 214, 255);
	tColors.iDropColor = XUI_COLOR_RGBA(40, 170, 110, 255);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.iSlotCount = 24;
	tDesc.tLayout = tLayout;
	tDesc.tColors = tColors;
	tDesc.bHasLayout = 1;
	tDesc.bHasColors = 1;
	iRet = xuiInventoryGridCreate(pContext, &pGrid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pGrid != NULL, "inventory create");
	XUI_TEST_CHECK(xuiInventoryGridGetType(pContext) != NULL, "inventory type");
	XUI_TEST_CHECK(xuiInventoryGridGetFont(pGrid) == pFont, "inventory font from descriptor");
	XUI_TEST_CHECK(xuiInventoryGridSetFont(pGrid, NULL) == XUI_OK && xuiInventoryGridGetFont(pGrid) == pFont, "inventory default font");
	XUI_TEST_CHECK(xuiInventoryGridSetFont(pGrid, pFont) == XUI_OK && xuiInventoryGridGetFont(pGrid) == pFont, "inventory explicit font");
	(void)xuiWidgetSetRect(pGrid, (xui_rect_t){20.0f, 20.0f, 180.0f, 120.0f});
	iRet = xuiWidgetAddChild(pRoot, pGrid);
	XUI_TEST_CHECK(iRet == XUI_OK, "add inventory");
	iRet = xuiWidgetCreate(pContext, &pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && pChild != NULL, "inventory descendant create");
	XUI_TEST_CHECK(xuiWidgetSetHitTestVisible(pChild, 0) == XUI_OK &&
		xuiWidgetAddChild(pGrid, pChild) == XUI_OK, "inventory descendant add");

	iRet = xuiInventoryGridSetSelectCallback(pGrid, __xuiInventoryGridTestSelect, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "select callback");
	iRet = xuiInventoryGridSetActivateCallback(pGrid, __xuiInventoryGridTestActivate, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "activate callback");
	iRet = xuiInventoryGridSetContextCallback(pGrid, __xuiInventoryGridTestContext, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "context callback");
	iRet = xuiInventoryGridSetDragCallback(pGrid, __xuiInventoryGridTestDrag, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag callback");
	iRet = xuiInventoryGridSetDropCallback(pGrid, __xuiInventoryGridTestDrop, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "drop callback");
	iRet = xuiInventoryGridSetSplitCallback(pGrid, __xuiInventoryGridTestSplit, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "split callback");
	iRet = xuiInventoryGridSetTooltipCallback(pGrid, __xuiInventoryGridTestTooltip, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "tooltip callback");
	iRet = xuiInventoryGridSetRenderCallback(pGrid, __xuiInventoryGridTestRender, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "render callback");
	iRet = xuiInventoryGridSetAnimationRenderCallback(pGrid, __xuiInventoryGridTestAnimationRender, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "animation render callback");

	__xuiInventoryGridTestSlot(&tSlot, 0, 100, 12, "Sword", "1", XUI_COLOR_RGBA(90, 176, 118, 255));
	tSlot.pIcon = pIcon;
	tSlot.tIconSrc = (xui_rect_t){0.0f, 0.0f, 24.0f, 24.0f};
	tSlot.iFlags |= XUI_INVENTORY_SLOT_DURABILITY | XUI_INVENTORY_SLOT_COOLDOWN;
	tSlot.fCooldownRate = 0.35f;
	iRet = xuiInventoryGridSetSlot(pGrid, 0, &tSlot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set slot 0");
	__xuiInventoryGridTestSlot(&tSlot, 2, 102, 1, "Shield", "2", XUI_COLOR_RGBA(94, 148, 220, 255));
	tSlot.iFlags |= XUI_INVENTORY_SLOT_COOLDOWN | XUI_INVENTORY_SLOT_COOLDOWN_RADIAL;
	tSlot.fCooldownRate = 0.66f;
	iRet = xuiInventoryGridSetSlot(pGrid, 2, &tSlot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set slot 2");
	__xuiInventoryGridTestSlot(&tSlot, 5, 100, 20, "Potion", "5", XUI_COLOR_RGBA(208, 142, 74, 255));
	iRet = xuiInventoryGridSetSlot(pGrid, 5, &tSlot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set slot 5");
	__xuiInventoryGridTestSlot(&tSlot, 23, 123, 3, "Gem", "Q", XUI_COLOR_RGBA(162, 106, 220, 255));
	tSlot.iFlags |= XUI_INVENTORY_SLOT_COOLDOWN | XUI_INVENTORY_SLOT_COOLDOWN_RADIAL;
	tSlot.fCooldownRate = 0.5f;
	iRet = xuiInventoryGridSetSlot(pGrid, 23, &tSlot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set slot 23");
	__xuiInventoryGridTestSlot(&tSlot, 6, 0, 0, NULL, NULL, 0u);
	tSlot.iFlags |= XUI_INVENTORY_SLOT_LOCKED;
	iRet = xuiInventoryGridSetSlot(pGrid, 6, &tSlot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set locked slot");
	__xuiInventoryGridTestSlot(&tSlot, 1, 201, 1, NULL, NULL, XUI_COLOR_WHITE);
	memset(tSlot.sText, 'a', sizeof(tSlot.sText));
	tSlot.sText[sizeof(tSlot.sText) - 2u] = (char)0xE4;
	tSlot.sText[sizeof(tSlot.sText) - 1u] = '\0';
	XUI_TEST_CHECK(xuiInventoryGridSetSlot(pGrid, 1, &tSlot) == XUI_OK, "set truncated utf8 slot");
	XUI_TEST_CHECK(xuiInventoryGridGetSlot(pGrid, 1, &tOutSlot) == XUI_OK &&
		strlen(tOutSlot.sText) == sizeof(tOutSlot.sText) - 2u &&
		(unsigned char)tOutSlot.sText[sizeof(tOutSlot.sText) - 3u] == (unsigned char)'a', "slot text trims incomplete utf8");
	XUI_TEST_CHECK(xuiInventoryGridClearSlot(pGrid, 1) == XUI_OK, "clear truncated utf8 slot");

	XUI_TEST_CHECK(xuiInventoryGridGetSlotCount(pGrid) == 24, "slot count");
	iRet = xuiInventoryGridGetSlot(pGrid, 0, &tOutSlot);
	XUI_TEST_CHECK(iRet == XUI_OK && tOutSlot.iItemId == 100 && tOutSlot.iCount == 12 && strcmp(tOutSlot.sHotkey, "1") == 0, "slot roundtrip");
	iRet = xuiInventoryGridGetLayout(pGrid, &tLayout);
	XUI_TEST_CHECK(iRet == XUI_OK && tLayout.iColumns == 4 && tLayout.iSelectionMode == XUI_INVENTORY_SELECTION_MULTI, "layout roundtrip");
	iRet = xuiInventoryGridGetColors(pGrid, &tColors);
	XUI_TEST_CHECK(iRet == XUI_OK && tColors.iSelectedColor == XUI_COLOR_RGBA(44, 126, 214, 255), "colors roundtrip");

	iChangeBefore = xuiInventoryGridGetChangeCount(pGrid);
	iRet = xuiInventoryGridSetSlot(pGrid, 0, &tOutSlot);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInventoryGridGetChangeCount(pGrid) == iChangeBefore, "identical slot is idempotent");
	tSlot = tOutSlot;
	tSlot.iSize = sizeof(tSlot) - 1u;
	XUI_TEST_CHECK(xuiInventoryGridSetSlot(pGrid, 0, &tSlot) == XUI_ERROR_INVALID_ARGUMENT, "undersized slot rejected");
	tSlot = tOutSlot;
	tSlot.fCooldownRate = NAN;
	XUI_TEST_CHECK(xuiInventoryGridSetSlot(pGrid, 0, &tSlot) == XUI_ERROR_INVALID_ARGUMENT, "nan slot rate rejected");
	tSlot = tOutSlot;
	tSlot.tIconSrc.fW = INFINITY;
	XUI_TEST_CHECK(xuiInventoryGridSetSlot(pGrid, 0, &tSlot) == XUI_ERROR_INVALID_ARGUMENT, "infinite icon rect rejected");
	tColors.iSize = sizeof(tColors) - 1u;
	XUI_TEST_CHECK(xuiInventoryGridSetColors(pGrid, &tColors) == XUI_ERROR_INVALID_ARGUMENT, "undersized colors rejected");
	iRet = xuiInventoryGridGetLayout(pGrid, &tLayout);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout before invalid input");
	tLayout.iSelectionMode = 999;
	XUI_TEST_CHECK(xuiInventoryGridSetLayout(pGrid, &tLayout) == XUI_ERROR_INVALID_ARGUMENT, "invalid selection mode rejected");
	XUI_TEST_CHECK(xuiInventoryGridSetMetrics(pGrid, NAN, 1.0f, 1.0f, 1.0f, 1.0f) == XUI_ERROR_INVALID_ARGUMENT, "nan metrics rejected");
	XUI_TEST_CHECK(xuiInventoryGridHitTest(pGrid, NAN, 10.0f, &tHit) == XUI_ERROR_INVALID_ARGUMENT, "nan hit coordinate rejected");
	XUI_TEST_CHECK(xuiInventoryGridSetSlotAnimation(pGrid, 0, NULL, 0u, NAN, 0u) == XUI_ERROR_INVALID_ARGUMENT, "nan animation scale rejected");

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	XUI_TEST_CHECK(xuiInventoryGridGetLayout(pGrid, &tCompactLayout) == XUI_OK,
		"get compact inventory layout");
	tCompactLayout.iColumns = 8;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.iSlotCount = 3;
	tDesc.tLayout = tCompactLayout;
	tDesc.bHasLayout = 1;
	XUI_TEST_CHECK(xuiInventoryGridCreate(pContext, &pTempGrid, &tDesc) == XUI_OK && pTempGrid != NULL,
		"create compact fixed-column inventory");
	XUI_TEST_CHECK(xuiWidgetSetRect(pTempGrid, (xui_rect_t){20.0f, 160.0f, 320.0f, 60.0f}) == XUI_OK &&
		xuiWidgetAddChild(pRoot, pTempGrid) == XUI_OK && xuiLayout(pContext) == XUI_OK,
		"layout compact fixed-column inventory");
	XUI_TEST_CHECK(xuiInventoryGridGetVisibleRange(pTempGrid, &tVisibleRange) == XUI_OK &&
		tVisibleRange.iTotalColumns == 3 && tVisibleRange.iTotalRows == 1,
		"fixed columns clamp to populated slot count");
	xuiWidgetDestroy(pTempGrid);
	pTempGrid = NULL;
	XUI_TEST_CHECK(xuiLayout(pContext) == XUI_OK, "layout after compact inventory destroy");
	tCompactLayout.iColumns = 3;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.iSlotCount = 3;
	tDesc.tLayout = tCompactLayout;
	tDesc.bHasLayout = 1;
	XUI_TEST_CHECK(xuiInventoryGridCreate(pContext, &pTempGrid, &tDesc) == XUI_OK && pTempGrid != NULL,
		"create context reentry inventory");
	XUI_TEST_CHECK(xuiWidgetSetRect(pTempGrid, (xui_rect_t){200.0f, 160.0f, 150.0f, 60.0f}) == XUI_OK &&
		xuiWidgetAddChild(pRoot, pTempGrid) == XUI_OK && xuiLayout(pContext) == XUI_OK &&
		xuiInventoryGridGetSlotRect(pTempGrid, 2, &tSlot2) == XUI_OK,
		"layout context reentry inventory");
	memset(&tReentry, 0, sizeof(tReentry));
	tReentry.iNewSlotCount = 1;
	XUI_TEST_CHECK(xuiInventoryGridSetSelectCallback(pTempGrid, __xuiInventoryGridTestMutatingSelect, &tReentry) == XUI_OK &&
		xuiInventoryGridSetContextCallback(pTempGrid, __xuiInventoryGridTestContext, &tState) == XUI_OK,
		"set context reentry callbacks");
	i = tState.iContexts;
	tWorld = xuiWidgetGetWorldRect(pTempGrid);
	memset(&tPointer, 0, sizeof(tPointer));
	tPointer.iSize = sizeof(tPointer);
	tPointer.iType = XUI_EVENT_CONTEXT_MENU;
	tPointer.pTarget = pTempGrid;
	tPointer.iPointerType = XUI_POINTER_TYPE_MOUSE;
	tPointer.iButton = XUI_POINTER_BUTTON_RIGHT;
	tPointer.fX = tWorld.fX + tSlot2.fX + 8.0f;
	tPointer.fY = tWorld.fY + tSlot2.fY + 8.0f;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK && tReentry.iCalls == 1 &&
		xuiInventoryGridGetSlotCount(pTempGrid) == 1 && tState.iContexts == i,
		"selection reentry suppresses stale context target");
	xuiWidgetDestroy(pTempGrid);
	pTempGrid = NULL;
	XUI_TEST_CHECK(xuiLayout(pContext) == XUI_OK, "layout after context reentry inventory destroy");
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pGrid) == 25, "accessible root and slot count");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pGrid, 0, &tAccessible) == XUI_OK &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_LIST && tAccessible.iRowCount == 6 &&
		tAccessible.iColumnCount == 4 &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_MULTISELECTABLE) != 0u,
		"accessible multi-selection inventory root");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pGrid, 1, &tAccessible) == XUI_OK &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_LIST_ITEM && strcmp(tAccessible.sName, "Sword") == 0 &&
		strcmp(tAccessible.sValue, "12") == 0 && tAccessible.iRow == 0 && tAccessible.iColumn == 0 &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_SELECTABLE) != 0u &&
		(tAccessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE)) != 0u,
		"accessible inventory slot");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pGrid, 7, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0u &&
		xuiWidgetPerformAccessibleAction(pGrid, 8u, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) == XUI_ERROR_INVALID_STATE,
		"accessible locked slot rejects action");
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pGrid, 4u, XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK &&
		xuiInventoryGridGetSelected(pGrid, 2) && xuiInventoryGridGetCurrent(pGrid) == 2,
		"accessible slot selection");
	XUI_TEST_CHECK(xuiInventoryGridClearSelection(pGrid) == XUI_OK && xuiInventoryGridGetCurrent(pGrid) == -1,
		"clear accessible selection");
	iRet = xuiInventoryGridGetSlotRect(pGrid, 0, &tSlot0);
	XUI_TEST_CHECK(iRet == XUI_OK && tSlot0.fW == 36.0f && tSlot0.fH == 36.0f, "slot rect 0");
	iRet = xuiInventoryGridGetSlotRect(pGrid, 2, &tSlot2);
	XUI_TEST_CHECK(iRet == XUI_OK && tSlot2.fX > tSlot0.fX, "slot rect 2");
	iRet = xuiInventoryGridGetSlotRect(pGrid, 5, &tSlot5);
	XUI_TEST_CHECK(iRet == XUI_OK && tSlot5.fY > tSlot0.fY, "slot rect 5");
	iRet = xuiInventoryGridGetSlotRect(pGrid, 4, &tSlot4);
	XUI_TEST_CHECK(iRet == XUI_OK && tSlot4.fY > tSlot0.fY, "slot rect 4 fixed columns");
	iRet = xuiInventoryGridGetSlotRect(pGrid, 6, &tSlot6);
	XUI_TEST_CHECK(iRet == XUI_OK && tSlot6.fY > tSlot0.fY, "slot rect 6 fixed columns");

	iRet = xuiInventoryGridGetLayout(pGrid, &tLayout);
	XUI_TEST_CHECK(iRet == XUI_OK, "get layout before auto");
	tLayout.iColumns = 0;
	iRet = xuiInventoryGridSetLayout(pGrid, &tLayout);
	XUI_TEST_CHECK(iRet == XUI_OK, "set auto columns");
	(void)xuiWidgetSetRect(pGrid, (xui_rect_t){20.0f, 20.0f, 220.0f, 120.0f});
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout auto columns");
	iRet = xuiInventoryGridGetLayout(pGrid, &tLayout);
	XUI_TEST_CHECK(iRet == XUI_OK && tLayout.iColumns == 0, "get auto columns");
	iRet = xuiInventoryGridGetSlotRect(pGrid, 4, &tStyledSlot4);
	XUI_TEST_CHECK(iRet == XUI_OK && tStyledSlot4.fY == tSlot0.fY, "slot rect 4 auto columns");
	tLayout.iColumns = 4;
	iRet = xuiInventoryGridSetLayout(pGrid, &tLayout);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore fixed columns");
	(void)xuiWidgetSetRect(pGrid, (xui_rect_t){20.0f, 20.0f, 180.0f, 120.0f});
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout fixed columns");
	memset(arrStyle, 0, sizeof(arrStyle));
	arrStyle[0].iSize = sizeof(arrStyle[0]);
	arrStyle[0].sName = "inventory.columns";
	arrStyle[0].tValue.iSize = sizeof(arrStyle[0].tValue);
	arrStyle[0].tValue.iType = XUI_STYLE_VALUE_INT;
	arrStyle[0].tValue.iInt = 5;
	arrStyle[1].iSize = sizeof(arrStyle[1]);
	arrStyle[1].sName = "inventory.slot.size";
	arrStyle[1].tValue.iSize = sizeof(arrStyle[1].tValue);
	arrStyle[1].tValue.iType = XUI_STYLE_VALUE_FLOAT;
	arrStyle[1].tValue.fFloat = 30.0f;
	arrStyle[2].iSize = sizeof(arrStyle[2]);
	arrStyle[2].sName = "inventory.background.color";
	arrStyle[2].tValue.iSize = sizeof(arrStyle[2].tValue);
	arrStyle[2].tValue.iType = XUI_STYLE_VALUE_COLOR;
	arrStyle[2].tValue.iColor = XUI_COLOR_RGBA(12, 34, 56, 255);
	iRet = xuiWidgetSetInlineStyle(pGrid, arrStyle, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "inline style set");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout inline style");
	iRet = xuiInventoryGridGetSlotRect(pGrid, 4, &tStyledSlot4);
	XUI_TEST_CHECK(iRet == XUI_OK && tStyledSlot4.fW == 30.0f && tStyledSlot4.fY == tSlot0.fY, "inline style metrics");
	iRet = xuiWidgetSetInlineStyle(pGrid, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "inline style clear");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout after style clear");
	tWorld = xuiWidgetGetWorldRect(pGrid);
	iRet = xuiInventoryGridHitTest(pGrid, tWorld.fX + tSlot0.fX + 18.0f, tWorld.fY + tSlot0.fY + 18.0f, &tHit);
	if ( !(iRet == XUI_OK && tHit.iSlot == 0 && tHit.iPart == XUI_INVENTORY_HIT_ICON) ) {
		printf("hit debug: ret=%d slot=%d part=%d world=(%d,%d,%d,%d) slot0=(%d,%d,%d,%d)\n",
			iRet, tHit.iSlot, tHit.iPart, tWorld.fX, tWorld.fY, tWorld.fW, tWorld.fH, tSlot0.fX, tSlot0.fY, tSlot0.fW, tSlot0.fH);
	}
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iSlot == 0 && tHit.iPart == XUI_INVENTORY_HIT_ICON, "hit icon");
	iCurrentBefore = xuiInventoryGridGetCurrent(pGrid);
	memset(&tPointer, 0, sizeof(tPointer));
	tPointer.iSize = sizeof(tPointer);
	tPointer.iType = XUI_EVENT_POINTER_DOWN;
	tPointer.pTarget = pChild;
	tPointer.iPointerType = XUI_POINTER_TYPE_MOUSE;
	tPointer.iButton = XUI_POINTER_BUTTON_LEFT;
	tPointer.iButtons = XUI_POINTER_BUTTON_LEFT;
	tPointer.fX = tWorld.fX + tSlot2.fX + 8.0f;
	tPointer.fY = tWorld.fY + tSlot2.fY + 8.0f;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK &&
		xuiInventoryGridGetCurrent(pGrid) == iCurrentBefore && xuiInventoryGridGetActiveSlot(pGrid) == -1,
		"descendant pointer event does not drive inventory");

	iRet = xuiInputPointerMove(pContext, tWorld.fX + tSlot0.fX + 8.0f, tWorld.fY + tSlot0.fY + 8.0f, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "pointer move");
	XUI_TEST_CHECK(xuiInventoryGridGetHoverSlot(pGrid) == 0, "hover slot");
	XUI_TEST_CHECK(xuiInventoryGridGetTooltipVisible(pGrid) != 0, "tooltip visible default");
	iRet = xuiUpdate(pContext, 0.22f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tooltip update");
	tTooltipRect = xuiWidgetTooltipGetRect(pContext);
	XUI_TEST_CHECK(xuiWidgetTooltipIsOpen(pContext) != 0 && xuiWidgetTooltipGetOwner(pContext) == pGrid, "tooltip opens");
	XUI_TEST_CHECK(xuiInventoryGridGetTooltipSlot(pGrid) == 0 && tState.iTooltips > 0 && tState.iLastTooltipSlot == 0, "tooltip slot callback");
	XUI_TEST_CHECK(tTooltipRect.fW >= 200.0f && tTooltipRect.fH >= 58.0f, "tooltip rect");
	iRet = xuiInventoryGridSetTooltipVisible(pGrid, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInventoryGridGetTooltipVisible(pGrid) == 0 && xuiInventoryGridGetTooltipSlot(pGrid) == -1, "tooltip hide");
	XUI_TEST_CHECK(xuiWidgetTooltipIsOpen(pContext) == 0, "tooltip closed");
	iRet = xuiInventoryGridSetTooltipVisible(pGrid, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInventoryGridGetTooltipVisible(pGrid) != 0, "tooltip show");
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tSlot0.fX + 8.0f, tWorld.fY + tSlot0.fY + 8.0f, XUI_POINTER_BUTTON_LEFT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "left down");
	XUI_TEST_CHECK(xuiInventoryGridGetCurrent(pGrid) == 0 && xuiInventoryGridGetSelected(pGrid, 0), "left select");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + tSlot0.fX + 8.0f, tWorld.fY + tSlot0.fY + 8.0f, XUI_POINTER_BUTTON_LEFT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "left up");
	XUI_TEST_CHECK(tState.iActivates == 1 && tState.iLastActivate == 0, "click activate");
	iActivatesBefore = tState.iActivates;
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tSlot6.fX + 8.0f, tWorld.fY + tSlot6.fY + 8.0f, XUI_POINTER_BUTTON_LEFT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK &&
		xuiInventoryGridGetActiveSlot(pGrid) == -1 && xuiGetPointerCapture(pContext) == NULL,
		"locked slot remains hoverable but does not capture");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + tSlot6.fX + 8.0f, tWorld.fY + tSlot6.fY + 8.0f, XUI_POINTER_BUTTON_LEFT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK &&
		tState.iActivates == iActivatesBefore, "locked slot does not activate");
	i = tState.iContexts;
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tSlot6.fX + 8.0f, tWorld.fY + tSlot6.fY + 8.0f, XUI_POINTER_BUTTON_RIGHT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK,
		"locked slot right down");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + tSlot6.fX + 8.0f, tWorld.fY + tSlot6.fY + 8.0f, XUI_POINTER_BUTTON_RIGHT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK &&
		tState.iContexts == i + 1 && tState.iLastContext == 6,
		"locked slot preserves context actions");
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tSlot0.fX + 8.0f, tWorld.fY + tSlot0.fY + 8.0f, XUI_POINTER_BUTTON_LEFT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK &&
		xuiGetPointerCapture(pContext) == pGrid, "capture before lost mouse button");
	iRet = xuiInputPointerMove(pContext, tWorld.fX + tSlot0.fX + 10.0f, tWorld.fY + tSlot0.fY + 10.0f, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK &&
		xuiGetPointerCapture(pContext) == NULL && xuiInventoryGridGetActiveSlot(pGrid) == -1,
		"lost mouse button cancels inventory capture");
	iCurrentBefore = xuiInventoryGridGetCurrent(pGrid);
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pRoot, 0) == XUI_OK, "disable inventory ancestor");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pGrid, 0, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0u,
		"accessible inventory reflects disabled ancestor");
	memset(&tPointer, 0, sizeof(tPointer));
	tPointer.iSize = sizeof(tPointer);
	tPointer.iType = XUI_EVENT_POINTER_DOWN;
	tPointer.pTarget = pGrid;
	tPointer.iPointerType = XUI_POINTER_TYPE_MOUSE;
	tPointer.iButton = XUI_POINTER_BUTTON_LEFT;
	tPointer.iButtons = XUI_POINTER_BUTTON_LEFT;
	tPointer.fX = tWorld.fX + tSlot2.fX + 8.0f;
	tPointer.fY = tWorld.fY + tSlot2.fY + 8.0f;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK && xuiInventoryGridGetCurrent(pGrid) == iCurrentBefore && xuiInventoryGridGetActiveSlot(pGrid) == -1, "effective disabled state blocks pointer input");
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pRoot, 1) == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK,
		"restore inventory ancestor");
	XUI_TEST_CHECK(xuiWidgetSetVisible(pRoot, 0) == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK,
		"hide inventory ancestor");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pGrid, 0, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_OFFSCREEN) != 0u && tAccessible.iActions == 0,
		"hidden inventory root is offscreen and inert");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pGrid, 1, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_OFFSCREEN) != 0u && tAccessible.iActions == 0,
		"hidden inventory slot is offscreen and inert");
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pGrid, 2u, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) != XUI_OK,
		"hidden inventory rejects accessibility action");
	iCurrentBefore = xuiInventoryGridGetCurrent(pGrid);
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK &&
		xuiInventoryGridGamepadButton(pGrid, XUI_INVENTORY_GAMEPAD_BUTTON_RIGHT, 1, 0u) == XUI_OK &&
		xuiInventoryGridGetCurrent(pGrid) == iCurrentBefore && xuiInventoryGridGetActiveSlot(pGrid) == -1,
		"hidden inventory blocks direct pointer and gamepad input");
	XUI_TEST_CHECK(xuiWidgetSetVisible(pRoot, 1) == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK,
		"show inventory ancestor");

	iRet = xuiInputSetModifiers(pContext, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set ctrl");
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tSlot2.fX + 8.0f, tWorld.fY + tSlot2.fY + 8.0f, XUI_POINTER_BUTTON_LEFT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "ctrl down");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + tSlot2.fX + 8.0f, tWorld.fY + tSlot2.fY + 8.0f, XUI_POINTER_BUTTON_LEFT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "ctrl up");
	iRet = xuiInputSetModifiers(pContext, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear ctrl");
	XUI_TEST_CHECK(xuiInventoryGridGetSelected(pGrid, 0) && xuiInventoryGridGetSelected(pGrid, 2), "ctrl multi select");
	iRet = xuiInventoryGridGetLayout(pGrid, &tLayout);
	XUI_TEST_CHECK(iRet == XUI_OK, "get layout before single selection");
	tLayout.iSelectionMode = XUI_INVENTORY_SELECTION_SINGLE;
	iRet = xuiInventoryGridSetLayout(pGrid, &tLayout);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiInventoryGridGetSelected(pGrid, 0) && xuiInventoryGridGetSelected(pGrid, 2), "single mode collapses prior multi selection");
	tLayout.iSelectionMode = XUI_INVENTORY_SELECTION_MULTI;
	XUI_TEST_CHECK(xuiInventoryGridSetLayout(pGrid, &tLayout) == XUI_OK, "restore multi selection mode");
	XUI_TEST_CHECK(xuiInventoryGridSetSelected(pGrid, 0, 1, 1) == XUI_OK && xuiInventoryGridGetSelected(pGrid, 0), "programmatic selection");
	iChangeBefore = xuiInventoryGridGetChangeCount(pGrid);
	i = tState.iSelects;
	XUI_TEST_CHECK(xuiInventoryGridSetSelected(pGrid, 0, 1, 1) == XUI_OK &&
		xuiInventoryGridGetChangeCount(pGrid) == iChangeBefore && tState.iSelects == i,
		"identical programmatic selection is idempotent");
	XUI_TEST_CHECK(xuiInventoryGridSetSelected(pGrid, 6, 1, 1) == XUI_OK && !xuiInventoryGridGetSelected(pGrid, 6), "locked slot cannot be selected");
	XUI_TEST_CHECK(xuiInventoryGridSetCurrent(pGrid, 2, 0) == XUI_OK, "vertical navigation starts above locked slot");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK &&
		xuiInventoryGridGetCurrent(pGrid) == 10,
		"keyboard down skips locked slot within the same column");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_UP, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK &&
		xuiInventoryGridGetCurrent(pGrid) == 2,
		"keyboard up skips locked slot within the same column");
	XUI_TEST_CHECK(xuiInventoryGridGamepadButton(pGrid, XUI_INVENTORY_GAMEPAD_BUTTON_DOWN, 1, 0u) == XUI_OK &&
		xuiInventoryGridGetCurrent(pGrid) == 10,
		"gamepad down skips locked slot within the same column");
	XUI_TEST_CHECK(xuiInventoryGridGamepadButton(pGrid, XUI_INVENTORY_GAMEPAD_BUTTON_UP, 1, 0u) == XUI_OK &&
		xuiInventoryGridGetCurrent(pGrid) == 2,
		"gamepad up skips locked slot within the same column");
	XUI_TEST_CHECK(xuiInventoryGridSetCurrent(pGrid, 2, 0) == XUI_OK, "keyboard current start");
	iRet = xuiInputSetModifiers(pContext, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set ctrl for keyboard navigation");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK &&
		xuiInventoryGridGetCurrent(pGrid) == 3 && !xuiInventoryGridGetSelected(pGrid, 3),
		"ctrl arrow moves focus without selection");
	iActivatesBefore = tState.iActivates;
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK &&
		xuiInventoryGridGetSelected(pGrid, 3) && tState.iActivates == iActivatesBefore,
		"ctrl space toggles selection without activation");
	iRet = xuiInputKeyDown(pContext, 'A', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "ctrl a dispatch");
	iSelectedCount = 0;
	for ( i = 0; i < xuiInventoryGridGetSlotCount(pGrid); i++ ) {
		if ( xuiInventoryGridGetSelected(pGrid, i) ) iSelectedCount++;
	}
	XUI_TEST_CHECK(iSelectedCount == 23 && !xuiInventoryGridGetSelected(pGrid, 6), "ctrl a selects every enabled slot");
	XUI_TEST_CHECK(xuiInputSetModifiers(pContext, 0u) == XUI_OK, "clear keyboard ctrl");
	XUI_TEST_CHECK(xuiInventoryGridGetSlot(pGrid, 0, &tOutSlot) == XUI_OK, "get first slot for home test");
	tSlot = tOutSlot;
	tSlot.iFlags |= XUI_INVENTORY_SLOT_DISABLED;
	XUI_TEST_CHECK(xuiInventoryGridSetSlot(pGrid, 0, &tSlot) == XUI_OK && xuiInventoryGridSetCurrent(pGrid, 5, 0) == XUI_OK, "disable first slot");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_HOME, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK && xuiInventoryGridGetCurrent(pGrid) == 1,
		"home skips disabled first slot");
	XUI_TEST_CHECK(xuiInventoryGridSetSlot(pGrid, 0, &tOutSlot) == XUI_OK, "restore first slot");
	XUI_TEST_CHECK(xuiInventoryGridGetSlot(pGrid, 23, &tOutSlot) == XUI_OK, "get last slot for end test");
	tSlot = tOutSlot;
	tSlot.iFlags |= XUI_INVENTORY_SLOT_DISABLED;
	XUI_TEST_CHECK(xuiInventoryGridSetSlot(pGrid, 23, &tSlot) == XUI_OK && xuiInventoryGridSetCurrent(pGrid, 5, 0) == XUI_OK, "disable last slot");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_END, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK && xuiInventoryGridGetCurrent(pGrid) == 22,
		"end skips disabled last slot");
	XUI_TEST_CHECK(xuiInventoryGridSetSlot(pGrid, 23, &tOutSlot) == XUI_OK, "restore last slot");
	XUI_TEST_CHECK(xuiInventoryGridSetCurrent(pGrid, 2, 0) == XUI_OK, "restore keyboard current");

	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "key right");
	XUI_TEST_CHECK(xuiInventoryGridGetCurrent(pGrid) == 3, "key move current");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "space activate");
	XUI_TEST_CHECK(tState.iActivates >= 2 && tState.iLastActivate == 3, "key activate");

	iRet = xuiInputPointerDown(pContext, tWorld.fX + tSlot2.fX + 8.0f, tWorld.fY + tSlot2.fY + 8.0f, XUI_POINTER_BUTTON_RIGHT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "right down");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + tSlot2.fX + 8.0f, tWorld.fY + tSlot2.fY + 8.0f, XUI_POINTER_BUTTON_RIGHT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "right up context");
	XUI_TEST_CHECK(tState.iContexts >= 1 && tState.iLastContext == 2, "context callback");

	iRet = xuiInventoryGridGetGamepadProfile(pGrid, &tGamepadProfile);
	XUI_TEST_CHECK(iRet == XUI_OK && tGamepadProfile.iRightButton == XUI_INVENTORY_GAMEPAD_BUTTON_RIGHT, "gamepad profile default");
	iRet = xuiInventoryGridSetCurrent(pGrid, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "gamepad current start");
	iRet = xuiInventoryGridGamepadButton(pGrid, tGamepadProfile.iRightButton, 1, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInventoryGridGetCurrent(pGrid) == 1, "gamepad right");
	iRet = xuiInventoryGridGamepadButton(pGrid, tGamepadProfile.iDownButton, 1, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInventoryGridGetCurrent(pGrid) == 5, "gamepad down");
	iRet = xuiInventoryGridGamepadButton(pGrid, tGamepadProfile.iAcceptButton, 1, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iLastActivate == 5, "gamepad accept");
	iRet = xuiInventoryGridGamepadButton(pGrid, tGamepadProfile.iContextButton, 1, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iLastContext == 5, "gamepad context");
	tGamepadProfile.iRightButton = 99;
	tGamepadProfile.iFlags |= XUI_INVENTORY_GAMEPAD_WRAP_ROWS;
	iRet = xuiInventoryGridSetGamepadProfile(pGrid, &tGamepadProfile);
	XUI_TEST_CHECK(iRet == XUI_OK, "gamepad set profile");
	iRet = xuiInventoryGridSetCurrent(pGrid, 3, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "gamepad wrap start");
	iRet = xuiInventoryGridGamepadButton(pGrid, XUI_INVENTORY_GAMEPAD_BUTTON_RIGHT, 1, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInventoryGridGetCurrent(pGrid) == 3, "gamepad old button ignored");
	iRet = xuiInventoryGridGamepadButton(pGrid, 99, 1, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInventoryGridGetCurrent(pGrid) == 0, "gamepad custom wrap");
	iRet = xuiInventoryGridSetGamepadProfile(pGrid, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "gamepad reset profile");
	iRet = xuiInventoryGridGetGamepadProfile(pGrid, &tGamepadProfile);
	XUI_TEST_CHECK(iRet == XUI_OK && tGamepadProfile.iRightButton == XUI_INVENTORY_GAMEPAD_BUTTON_RIGHT, "gamepad reset profile get");

	iRet = xuiInventoryGridOpenSplitPopup(pGrid, 2, tWorld.fX + tSlot2.fX + 8.0f, tWorld.fY + tSlot2.fY + 8.0f);
	XUI_TEST_CHECK(iRet != XUI_OK && !xuiInventoryGridIsSplitPopupOpen(pGrid), "split rejects single count");
	iRet = xuiInventoryGridOpenSplitPopup(pGrid, 5, tWorld.fX + tSlot5.fX + 8.0f, tWorld.fY + tSlot5.fY + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInventoryGridIsSplitPopupOpen(pGrid), "split opens stack");
	XUI_TEST_CHECK(xuiInventoryGridGetSplitSlot(pGrid) == 5 && xuiInventoryGridGetSplitCount(pGrid) == 10, "split default count");
	XUI_TEST_CHECK(xuiInventoryGridCloseSplitPopup(pGrid) == XUI_OK && !xuiInventoryGridIsSplitPopupOpen(pGrid), "split close api");
	iRet = xuiInventoryGridOpenSplitPopup(pGrid, 5, tWorld.fX + tSlot5.fX + 8.0f, tWorld.fY + tSlot5.fY + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInventoryGridIsSplitPopupOpen(pGrid), "split reopens after close");
	pSplitPopup = xuiInventoryGridGetSplitPopupWidget(pGrid);
	pSplitInput = xuiInventoryGridGetSplitInputWidget(pGrid);
	XUI_TEST_CHECK(pSplitPopup != NULL && pSplitInput != NULL, "split popup widgets");
	iRet = xuiNumericInputSetValue(pSplitInput, 7.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "split set count");
	iRet = xuiInventoryGridCommitSplitPopup(pGrid);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiInventoryGridIsSplitPopupOpen(pGrid), "split commit closes");
	XUI_TEST_CHECK(tState.iSplits == 1 && tState.iLastSplitSlot == 5 && tState.iLastSplitCount == 7, "split callback fired");
	iRet = xuiInventoryGridOpenSplitPopup(pGrid, 5, tWorld.fX + tSlot5.fX + 8.0f, tWorld.fY + tSlot5.fY + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInventoryGridIsSplitPopupOpen(pGrid), "split opens for gamepad cancel");
	iRet = xuiInventoryGridGamepadButton(pGrid, tGamepadProfile.iCancelButton, 1, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiInventoryGridIsSplitPopupOpen(pGrid), "gamepad cancel closes split");
	XUI_TEST_CHECK(tState.iSplits == 1, "gamepad cancel does not split");

	iRet = xuiInputPointerDown(pContext, tWorld.fX + tSlot0.fX + 8.0f, tWorld.fY + tSlot0.fY + 8.0f, XUI_POINTER_BUTTON_LEFT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "drag down");
	iRet = xuiInputPointerMove(pContext, tWorld.fX + tSlot5.fX + 18.0f, tWorld.fY + tSlot5.fY + 18.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "drag move");
	XUI_TEST_CHECK(xuiInventoryGridGetDragSource(pGrid) == 0 && xuiInventoryGridGetDropTarget(pGrid) == 5, "drag target");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + tSlot5.fX + 18.0f, tWorld.fY + tSlot5.fY + 18.0f, XUI_POINTER_BUTTON_LEFT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "drag up");
	XUI_TEST_CHECK(tState.iDrags >= 1 && tState.iLastDrag == 0 && tState.iDropCalls == 1 && tState.iDrops == 1 && tState.iLastDropFrom == 0 && tState.iLastDropTo == 5 && tState.iLastDropMode == XUI_INVENTORY_DROP_STACK, "drag drop callback");
	tState.iDropAccept = 0;
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tSlot2.fX + 8.0f, tWorld.fY + tSlot2.fY + 8.0f, XUI_POINTER_BUTTON_LEFT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "reject drag down");
	iRet = xuiInputPointerMove(pContext, tWorld.fX + tSlot5.fX + 18.0f, tWorld.fY + tSlot5.fY + 18.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "reject drag move");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + tSlot5.fX + 18.0f, tWorld.fY + tSlot5.fY + 18.0f, XUI_POINTER_BUTTON_LEFT, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "reject drag up");
	iRet = xuiInventoryGridGetSlot(pGrid, 2, &tOutSlot);
	XUI_TEST_CHECK(iRet == XUI_OK && tOutSlot.iItemId == 102, "rejected source unchanged");
	iRet = xuiInventoryGridGetSlot(pGrid, 5, &tOutSlot);
	XUI_TEST_CHECK(iRet == XUI_OK && tOutSlot.iItemId == 100, "rejected target unchanged");
	XUI_TEST_CHECK(tState.iDropCalls == 2 && tState.iDrops == 1, "rejected drop not accepted");
	tState.iDropAccept = 1;

	iCallbackCalls = 0;
	XUI_TEST_CHECK(xuiInventoryGridSetDragCallback(pGrid, __xuiInventoryGridTestDisablingDrag, &iCallbackCalls) == XUI_OK,
		"set disabling drag callback");
	memset(&tPointer, 0, sizeof(tPointer));
	tPointer.iSize = sizeof(tPointer);
	tPointer.pTarget = pGrid;
	tPointer.iType = XUI_EVENT_POINTER_DOWN;
	tPointer.iPointerId = 2001;
	tPointer.iPointerType = XUI_POINTER_TYPE_TOUCH;
	tPointer.iButton = XUI_POINTER_BUTTON_LEFT;
	tPointer.iButtons = XUI_POINTER_BUTTON_LEFT;
	tPointer.fX = tWorld.fX + tSlot0.fX + 8.0f;
	tPointer.fY = tWorld.fY + tSlot0.fY + 8.0f;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK && xuiInventoryGridGetActiveSlot(pGrid) == 0,
		"reentrant drag starts pointer sequence");
	tPointer.iType = XUI_EVENT_POINTER_MOVE;
	tPointer.fX = tWorld.fX + tSlot5.fX + 18.0f;
	tPointer.fY = tWorld.fY + tSlot5.fY + 18.0f;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK && iCallbackCalls == 1 &&
		!xuiWidgetGetEffectiveEnabled(pGrid) && xuiInventoryGridGetActiveSlot(pGrid) == -1 &&
		xuiInventoryGridGetDragSource(pGrid) == -1 &&
		xuiGetPointerCaptureEx(pContext, 2001, XUI_POINTER_TYPE_TOUCH) == NULL,
		"drag callback disable cancels pointer before drag state commits");
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pGrid, 1) == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK &&
		xuiInventoryGridSetDragCallback(pGrid, __xuiInventoryGridTestDrag, &tState) == XUI_OK,
		"restore inventory after reentrant drag");

	memset(&tPointer, 0, sizeof(tPointer));
	tPointer.iSize = sizeof(tPointer);
	tPointer.pTarget = pGrid;
	tPointer.iType = XUI_EVENT_POINTER_DOWN;
	tPointer.iPointerId = 1001;
	tPointer.iPointerType = XUI_POINTER_TYPE_TOUCH;
	tPointer.iButton = XUI_POINTER_BUTTON_LEFT;
	tPointer.iButtons = XUI_POINTER_BUTTON_LEFT;
	tPointer.fX = tWorld.fX + tSlot0.fX + 8.0f;
	tPointer.fY = tWorld.fY + tSlot0.fY + 8.0f;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK && xuiInventoryGridGetActiveSlot(pGrid) == 0, "touch owns drag sequence");
	tPointer.iPointerId = 1002;
	tPointer.fX = tWorld.fX + tSlot2.fX + 8.0f;
	tPointer.fY = tWorld.fY + tSlot2.fY + 8.0f;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK && xuiInventoryGridGetActiveSlot(pGrid) == 0, "second touch cannot steal drag");
	tPointer.iType = XUI_EVENT_POINTER_CAPTURE_LOST;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK && xuiInventoryGridGetActiveSlot(pGrid) == 0, "unrelated capture loss ignored");
	tPointer.iType = XUI_EVENT_POINTER_MOVE;
	tPointer.iPointerId = 1001;
	tPointer.fX = tWorld.fX + tSlot5.fX + 18.0f;
	tPointer.fY = tWorld.fY + tSlot5.fY + 18.0f;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK && xuiInventoryGridGetDragSource(pGrid) == 0, "owning touch continues drag");
	iRet = xuiReleasePointerCaptureEx(pContext, 1001, XUI_POINTER_TYPE_TOUCH, pGrid);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK && xuiInventoryGridGetActiveSlot(pGrid) == -1 && xuiInventoryGridGetDragSource(pGrid) == -1, "matching capture loss cancels drag");

	iRet = xuiInventoryGridSetSlotAnimation(pGrid, 0, (xui_animation_object_t*)0x1234, 7u, 1.25f, XUI_COLOR_RGBA(200, 210, 255, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set animation");
	pAnimation = xuiInventoryGridGetSlotAnimation(pGrid, 0, &iAnimationFlags, &fAnimationScale, &iAnimationTint);
	XUI_TEST_CHECK(pAnimation == (xui_animation_object_t*)0x1234 && iAnimationFlags == 7u && fAnimationScale > 1.2f && iAnimationTint == XUI_COLOR_RGBA(200, 210, 255, 255), "get animation");
	iRet = xuiInventoryGridToXValue(pGrid, &pXsonValue);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiXrtValueIsObject(pXsonValue), "xson value");
	pSlots = xuiXrtValueObjectGet(pXsonValue, "slots", 0);
	XUI_TEST_CHECK(xuiXrtValueIsArray(pSlots) && xrtValueCount(pSlots) == 24u, "xson slot array");
	xrtValueRelease(pXsonValue);
	pXsonValue = NULL;
	iRet = xuiInventoryGridExportXSON(pGrid, sXson, (int)sizeof(sXson));
	XUI_TEST_CHECK(iRet > 0 && iRet < (int)sizeof(sXson) && strstr(sXson, "xui.inventorygrid") != NULL && strstr(sXson, "Potion") != NULL && strstr(sXson, "animation") != NULL, "export xson");
	iRet = xuiInventoryGridSaveXSONFile(pGrid, sXsonPath);
	XUI_TEST_CHECK(iRet == XUI_OK, "save xson file");
	pSavedXsonValue = xrtXlonParseFile(sXsonPath);
	pKind = xuiXrtValueIsObject(pSavedXsonValue) ? xuiXrtValueObjectGet(pSavedXsonValue, "kind", 0) : NULL;
	sKind = xuiXrtValueIsText(pKind) ? xuiXrtValueGetText(pKind) : NULL;
	XUI_TEST_CHECK(sKind != NULL && strcmp(sKind, "xui.inventorygrid") == 0, "saved xson parses");
	xrtValueRelease(pSavedXsonValue);
	pSavedXsonValue = NULL;
	iRet = xuiInventoryGridClearSlotAnimation(pGrid, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInventoryGridGetSlotAnimation(pGrid, 0, NULL, NULL, NULL) == NULL, "clear animation");

	iRet = xuiInventoryGridEnsureSlotVisible(pGrid, 23);
	XUI_TEST_CHECK(iRet == XUI_OK, "ensure visible");
	pScroll = xuiInventoryGridGetScrollModel(pGrid);
	XUI_TEST_CHECK(pScroll != NULL, "scroll model");
	fScrollY = 0.0f;
	iRet = xuiScrollModelGetOffset(pScroll, NULL, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK && fScrollY > 0.0f, "scroll offset");
	XUI_TEST_CHECK(xuiInventoryGridGetLayout(pGrid, &tLayout) == XUI_OK, "get layout for horizontal wheel");
	tLayout.iColumns = 8;
	XUI_TEST_CHECK(xuiInventoryGridSetLayout(pGrid, &tLayout) == XUI_OK,
		"enable horizontal inventory overflow without pre-layout");
	XUI_TEST_CHECK(xuiScrollModelSetOffset(pScroll, 0.0f, 0.0f) == XUI_OK, "reset inventory scroll");
	XUI_TEST_CHECK(xuiInputSetModifiers(pContext, XUI_MOD_SHIFT) == XUI_OK, "set shift wheel modifier");
	iRet = xuiInputPointerWheel(pContext, tWorld.fX + 8.0f, tWorld.fY + 8.0f, 0.0f, -1.0f, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiInventoryGridTestDispatch(pContext) == XUI_OK, "shift wheel dispatch");
	fScrollX = 0.0f;
	XUI_TEST_CHECK(xuiScrollModelGetOffset(pScroll, &fScrollX, NULL) == XUI_OK && fScrollX > 0.0f,
		"shift wheel synchronizes pending layout and scrolls horizontally");
	XUI_TEST_CHECK(xuiInputSetModifiers(pContext, 0u) == XUI_OK, "clear shift wheel modifier");
	tLayout.iColumns = 4;
	XUI_TEST_CHECK(xuiInventoryGridSetLayout(pGrid, &tLayout) == XUI_OK && xuiLayout(pContext) == XUI_OK &&
		xuiScrollModelSetOffset(pScroll, 0.0f, 0.0f) == XUI_OK, "restore inventory columns and scroll");
	XUI_TEST_CHECK(xuiInventoryGridEnsureSlotVisible(pGrid, 23) == XUI_OK, "restore animated slot visibility");
	iRet = xuiInventoryGridSetSlotAnimation(pGrid, 23, (xui_animation_object_t*)0x2345, 9u, 1.0f, XUI_COLOR_RGBA(92, 166, 255, 190));
	XUI_TEST_CHECK(iRet == XUI_OK, "set visible animation");

	memset(&tQuery, 0, sizeof(tQuery));
	tQuery.iSize = sizeof(tQuery);
	tQuery.iFlags = XUI_INVENTORY_QUERY_EXCLUDE_EMPTY;
	tQuery.iSortMode = XUI_INVENTORY_SORT_COUNT;
	iRet = xuiInventoryGridQuerySlots(pGrid, &tQuery, arrQuerySlots, (int)(sizeof(arrQuerySlots) / sizeof(arrQuerySlots[0])), &iQueryCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iQueryCount == 4, "query non-empty count");
	XUI_TEST_CHECK(arrQuerySlots[0] == 2 && arrQuerySlots[1] == 23 && arrQuerySlots[2] == 0 && arrQuerySlots[3] == 5, "query count sort");
	tQuery.iFlags = XUI_INVENTORY_QUERY_EXCLUDE_EMPTY | XUI_INVENTORY_QUERY_CASE_INSENSITIVE;
	tQuery.iSortMode = XUI_INVENTORY_SORT_TEXT;
	tQuery.iRequiredFlags = XUI_INVENTORY_SLOT_COOLDOWN;
	tQuery.iRejectedFlags = 0u;
	tQuery.sTextContains = NULL;
	iRet = xuiInventoryGridQuerySlots(pGrid, &tQuery, arrQuerySlots, 2, &iQueryCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iQueryCount == 3 && arrQuerySlots[0] == 23 && arrQuerySlots[1] == 2, "query capacity and text sort");
	tQuery.iRequiredFlags = 0u;
	tQuery.sTextContains = "s";
	iRet = xuiInventoryGridQuerySlots(pGrid, &tQuery, arrQuerySlots, (int)(sizeof(arrQuerySlots) / sizeof(arrQuerySlots[0])), &iQueryCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iQueryCount == 2 && arrQuerySlots[0] == 2 && arrQuerySlots[1] == 0, "query text contains");
	iMinCount = 10;
	memset(&tQuery, 0, sizeof(tQuery));
	tQuery.iSize = sizeof(tQuery);
	tQuery.iFlags = XUI_INVENTORY_QUERY_EXCLUDE_EMPTY | XUI_INVENTORY_QUERY_SORT_DESCENDING;
	tQuery.iSortMode = XUI_INVENTORY_SORT_COUNT;
	tQuery.onFilter = __xuiInventoryGridTestMinCountFilter;
	tQuery.pFilterUser = &iMinCount;
	iRet = xuiInventoryGridQuerySlots(pGrid, &tQuery, arrQuerySlots, (int)(sizeof(arrQuerySlots) / sizeof(arrQuerySlots[0])), &iQueryCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iQueryCount == 2 && arrQuerySlots[0] == 5 && arrQuerySlots[1] == 0, "query callback filter");
	arrQuerySlots[0] = 0;
	arrQuerySlots[1] = 2;
	arrQuerySlots[2] = 5;
	iRet = xuiInventoryGridSortSlots(pGrid, arrQuerySlots, 3, XUI_INVENTORY_SORT_COUNT, XUI_INVENTORY_QUERY_SORT_DESCENDING, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && arrQuerySlots[0] == 5 && arrQuerySlots[1] == 0 && arrQuerySlots[2] == 2, "sort existing slots");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.iSlotCount = 4;
	iRet = xuiInventoryGridCreate(pContext, &pTempGrid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTempGrid != NULL, "create query mutation grid");
	memset(&tReentry, 0, sizeof(tReentry));
	tReentry.iNewSlotCount = 1;
	memset(&tQuery, 0, sizeof(tQuery));
	tQuery.iSize = sizeof(tQuery);
	tQuery.onFilter = __xuiInventoryGridTestMutatingFilter;
	tQuery.pFilterUser = &tReentry;
	iRet = xuiInventoryGridQuerySlots(pTempGrid, &tQuery, arrQuerySlots, 32, &iQueryCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iQueryCount == 4 && tReentry.iCalls == 4 && xuiInventoryGridGetSlotCount(pTempGrid) == 1, "query traverses immutable snapshot during mutation");
	xuiWidgetDestroy(pTempGrid);
	pTempGrid = NULL;

	tDesc.iSlotCount = 2;
	iRet = xuiInventoryGridCreate(pContext, &pTempGrid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTempGrid != NULL, "create query destroy grid");
	iCallbackCalls = 0;
	memset(&tQuery, 0, sizeof(tQuery));
	tQuery.iSize = sizeof(tQuery);
	tQuery.onFilter = __xuiInventoryGridTestDestroyingFilter;
	tQuery.pFilterUser = &iCallbackCalls;
	iQueryCount = 77;
	iRet = xuiInventoryGridQuerySlots(pTempGrid, &tQuery, arrQuerySlots, 32, &iQueryCount);
	pTempGrid = NULL;
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iQueryCount == 0 && iCallbackCalls == 1, "query aborts safely when callback destroys grid");

	tDesc.iSlotCount = 3;
	iRet = xuiInventoryGridCreate(pContext, &pTempGrid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTempGrid != NULL, "create sort destroy grid");
	arrQuerySlots[0] = 2;
	arrQuerySlots[1] = 0;
	arrQuerySlots[2] = 1;
	iCallbackCalls = 0;
	iRet = xuiInventoryGridSortSlots(pTempGrid, arrQuerySlots, 3, XUI_INVENTORY_SORT_CUSTOM, 0u, __xuiInventoryGridTestDestroyingCompare, &iCallbackCalls);
	pTempGrid = NULL;
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iCallbackCalls == 1 && arrQuerySlots[0] == 2 && arrQuerySlots[1] == 0 && arrQuerySlots[2] == 1, "destroyed comparator leaves caller order unchanged");

	tDesc.iSlotCount = 1;
	iRet = xuiInventoryGridCreate(pContext, &pTempGrid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTempGrid != NULL, "create clear-all grid");
	__xuiInventoryGridTestSlot(&tSlot, 0, 900, 2, "Clear me", "", XUI_COLOR_WHITE);
	XUI_TEST_CHECK(xuiInventoryGridSetSlot(pTempGrid, 0, &tSlot) == XUI_OK &&
		xuiInventoryGridSetSelected(pTempGrid, 0, 1, 0) == XUI_OK, "prepare clear-all grid");
	XUI_TEST_CHECK(xuiInventoryGridClearAll(pTempGrid) == XUI_OK &&
		xuiInventoryGridGetSlot(pTempGrid, 0, &tOutSlot) == XUI_OK &&
		(tOutSlot.iFlags & XUI_INVENTORY_SLOT_EMPTY) != 0u && xuiInventoryGridGetCurrent(pTempGrid) == -1,
		"clear-all resets slots and selection");
	xuiWidgetDestroy(pTempGrid);
	pTempGrid = NULL;

	iRet = xuiInventoryGridCreate(pContext, &pTempGrid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTempGrid != NULL, "create split destroy grid");
	__xuiInventoryGridTestSlot(&tSlot, 0, 901, 10, "Stack", "", XUI_COLOR_WHITE);
	XUI_TEST_CHECK(xuiInventoryGridSetSlot(pTempGrid, 0, &tSlot) == XUI_OK, "set split destroy slot");
	iCallbackCalls = 0;
	XUI_TEST_CHECK(xuiInventoryGridSetSplitCallback(pTempGrid, __xuiInventoryGridTestDestroyingSplit, &iCallbackCalls) == XUI_OK, "set destroying split callback");
	XUI_TEST_CHECK(xuiInventoryGridOpenSplitPopup(pTempGrid, 0, 10.0f, 10.0f) == XUI_OK, "open split destroy popup");
	iRet = xuiInventoryGridCommitSplitPopup(pTempGrid);
	pTempGrid = NULL;
	XUI_TEST_CHECK(iRet == XUI_OK && iCallbackCalls == 1, "split callback may destroy grid safely");

	XUI_TEST_CHECK(xuiCreate(&pTempContext) == XUI_OK && pTempContext != NULL, "create context destroy query context");
	XUI_TEST_CHECK(xuiInventoryGridCreate(pTempContext, &pTempGrid, &tDesc) == XUI_OK && pTempGrid != NULL, "create context destroy query grid");
	memset(&tQuery, 0, sizeof(tQuery));
	tQuery.iSize = sizeof(tQuery);
	tQuery.onFilter = __xuiInventoryGridTestDestroyingContextFilter;
	tQuery.pFilterUser = pTempContext;
	iQueryCount = 77;
	iRet = xuiInventoryGridQuerySlots(pTempGrid, &tQuery, arrQuerySlots, 32, &iQueryCount);
	pTempGrid = NULL;
	pTempContext = NULL;
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iQueryCount == 0, "query aborts safely when callback destroys context");

	iRet = xuiTestSurfaceCreate(&tProxyState, &pTarget, 360, 240, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	memset(arrStyle, 0, sizeof(arrStyle));
	arrStyle[0].iSize = sizeof(arrStyle[0]);
	arrStyle[0].sName = "inventory.background.color";
	arrStyle[0].tValue.iSize = sizeof(arrStyle[0].tValue);
	arrStyle[0].tValue.iType = XUI_STYLE_VALUE_COLOR;
	arrStyle[0].tValue.iColor = XUI_COLOR_RGBA(12, 34, 56, 255);
	iRet = xuiWidgetSetInlineStyle(pGrid, arrStyle, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render inline color");
	tFullRect = (xui_rect_i_t){0, 0, 360, 240};
	iMeshDrawBefore = xuiTestProxyGetMeshDrawCount(&tProxyState);
	tState.iAnimationRenders = 0;
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	pCache = xuiWidgetGetCacheSurface(pGrid, 0);
	XUI_TEST_CHECK(pCache != NULL, "grid cache");
	XUI_TEST_CHECK(tState.iRenders > 0, "custom render called");
	XUI_TEST_CHECK(tState.iAnimationRenders > 0 && tState.iAnimationSlot == 23 && tState.iAnimationFlags == 9u && tState.iAnimationTint == XUI_COLOR_RGBA(92, 166, 255, 190), "animation render callback called");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillCount(pCache) > 0, "cache drew shapes");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillColorCount(pCache, XUI_COLOR_RGBA(12, 34, 56, 255)) > 0, "style color rendered");
	XUI_TEST_CHECK(xuiTestSurfaceGetTextDrawCount(pCache) > 0, "cache drew text");
	XUI_TEST_CHECK(xuiTestProxyGetMeshDrawCount(&tProxyState) > iMeshDrawBefore, "radial cooldown mesh rendered");
	XUI_TEST_CHECK(xuiTestProxyGetLastMeshVertexCount(&tProxyState) >= 4, "radial cooldown mesh vertices");
	XUI_TEST_CHECK(xuiTestProxyGetLastMeshIndexCount(&tProxyState) >= 6, "radial cooldown mesh indices");

	iRet = xuiInventoryGridSetSlotCount(pGrid, 10000);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInventoryGridGetSlotCount(pGrid) == 10000, "large slot count");
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pGrid) == 10001, "large accessible slot count");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout large inventory");
	iRet = xuiInventoryGridGetVisibleRange(pGrid, &tVisibleRange);
	XUI_TEST_CHECK(iRet == XUI_OK, "visible range query");
	XUI_TEST_CHECK(tVisibleRange.iTotalColumns == 4 && tVisibleRange.iTotalRows == 2500, "large range totals");
	XUI_TEST_CHECK(tVisibleRange.iSlotCount > 0 && tVisibleRange.iSlotCount < 10000 && tVisibleRange.iColumnCount <= 4, "large range virtualized");
	tState.iRenders = 0;
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render large inventory");
	iRet = xuiInventoryGridGetLastPaintRange(pGrid, &tPaintRange);
	XUI_TEST_CHECK(iRet == XUI_OK, "last paint range");
	XUI_TEST_CHECK(tPaintRange.iPaintSlotCount == xuiInventoryGridGetLastPaintSlotCount(pGrid), "paint count api");
	XUI_TEST_CHECK(tPaintRange.iPaintSlotCount == tState.iRenders, "paint count matches render callback");
	XUI_TEST_CHECK(tPaintRange.iPaintSlotCount > 0 && tPaintRange.iPaintSlotCount < 100, "large render skips offscreen slots");
	iRet = xuiInventoryGridEnsureSlotVisible(pGrid, 9999);
	XUI_TEST_CHECK(iRet == XUI_OK, "ensure large last visible");
	iRet = xuiInventoryGridGetVisibleRange(pGrid, &tVisibleRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tVisibleRange.iFirstSlot <= 9999 && tVisibleRange.iLastSlot >= 9999, "large visible range includes last slot");

cleanup:
	if ( pTempGrid != NULL ) {
		xuiWidgetDestroy(pTempGrid);
	}
	if ( pTempContext != NULL ) {
		xuiDestroy(pTempContext);
	}
	if ( pXsonValue != NULL ) {
		xrtValueRelease(pXsonValue);
	}
	if ( pSavedXsonValue != NULL ) {
		xrtValueRelease(pSavedXsonValue);
	}
	if ( sXsonPath != NULL ) {
		remove(sXsonPath);
	}
	if ( pTarget != NULL ) {
		tProxyState.tProxy.surfaceDestroy(&tProxyState.tProxy, pTarget);
	}
	if ( pIcon != NULL ) {
		tProxyState.tProxy.surfaceDestroy(&tProxyState.tProxy, pIcon);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tProxyState.tProxy.fontDestroy(&tProxyState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_inventory_grid_test passed\n");
	return 0;
}
