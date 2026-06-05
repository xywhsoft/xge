#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 860
#define DEMO_TARGET_H 560
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define DEMO_ICON_COUNT 8

typedef struct xui_inventory_grid_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_surface arrIcons[DEMO_ICON_COUNT];
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pBackpack;
	xui_widget pQuickbar;
	xui_widget pEquipment;
	xui_widget pStatus;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	uint32_t iLastButtons;
	int iSelects;
	int iActivates;
	int iContexts;
	int iDrags;
	int iDrops;
	int iSplits;
	int iLastSlot;
	int iLastDropFrom;
	int iLastDropTo;
	int iLastDropMode;
	int iLastSplitSlot;
	int iLastSplitCount;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bDynamicOK;
	int bGamepadOK;
} xui_inventory_grid_demo_t;

static void __xuiInventoryDemoUsage(void)
{
	printf("usage: xui_inventory_grid [--frames N] [--seconds N]\n");
}

static int __xuiInventoryDemoParseArgs(xui_inventory_grid_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->fMaxSeconds = atof(argv[++i]);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__xuiInventoryDemoUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiInventoryDemoFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\simhei.ttf"
	};
	FILE* pFile;
	int i;

	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPaths[i];
		}
	}
	return NULL;
}

static int __xuiInventoryDemoRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_inventory_grid_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_inventory_grid_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(232, 241, 250, 255));
	}
	if ( pDemo->tProxy.drawRoundRectFill != NULL ) {
		tPanel = (xui_rect_t){28.0f, 22.0f, tRect.fW - 56.0f, tRect.fH - 44.0f};
		(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tPanel, 6.0f, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static int __xuiInventoryDemoAddLabel(xui_inventory_grid_demo_t* pDemo, const char* sText, xui_rect_t tRect, xui_widget pParent, uint32_t iColor, uint32_t iFlags, xui_widget* ppLabel)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = iFlags | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pLabel, tRect);
	(void)xuiWidgetSetHitTestVisible(pLabel, 0);
	iRet = xuiWidgetAddChild(pParent, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	if ( ppLabel != NULL ) *ppLabel = pLabel;
	return XUI_OK;
}

static int __xuiInventoryDemoCreateIcon(xui_inventory_grid_demo_t* pDemo, int iIndex, uint32_t iColor)
{
	uint32_t arrPixels[24 * 24];
	int i;

	if ( (pDemo == NULL) || (iIndex < 0) || (iIndex >= DEMO_ICON_COUNT) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < (int)(sizeof(arrPixels) / sizeof(arrPixels[0])); i++ ) {
		arrPixels[i] = iColor;
	}
	return pDemo->tProxy.surfaceCreateRGBA(&pDemo->tProxy, &pDemo->arrIcons[iIndex], 24, 24, arrPixels, 24 * (int)sizeof(uint32_t), 0u);
}

static void __xuiInventoryDemoFillSlot(xui_inventory_slot_t* pSlot, int iSlotId, int iItemId, int iCount, int iMaxCount, const char* sText, const char* sHotkey, xui_surface pIcon, uint32_t iQuality, uint32_t iFlags)
{
	memset(pSlot, 0, sizeof(*pSlot));
	pSlot->iSize = sizeof(*pSlot);
	pSlot->iSlotId = iSlotId;
	pSlot->iItemId = iItemId;
	pSlot->iCount = iCount;
	pSlot->iMaxCount = iMaxCount;
	pSlot->iQualityColor = iQuality;
	pSlot->iIconTint = XUI_COLOR_WHITE;
	pSlot->pIcon = pIcon;
	pSlot->tIconSrc = (xui_rect_t){0.0f, 0.0f, 24.0f, 24.0f};
	pSlot->iFlags = iFlags;
	pSlot->fCooldownRate = 0.0f;
	pSlot->fDurabilityRate = 1.0f;
	if ( sText != NULL ) {
		strncpy(pSlot->sText, sText, sizeof(pSlot->sText) - 1u);
		pSlot->sText[sizeof(pSlot->sText) - 1u] = '\0';
	}
	if ( sHotkey != NULL ) {
		strncpy(pSlot->sHotkey, sHotkey, sizeof(pSlot->sHotkey) - 1u);
		pSlot->sHotkey[sizeof(pSlot->sHotkey) - 1u] = '\0';
	}
}

static int __xuiInventoryDemoSetSlot(xui_widget pGrid, int iSlot, const xui_inventory_slot_t* pSlot)
{
	if ( (pSlot == NULL) || ((pSlot->iFlags & XUI_INVENTORY_SLOT_EMPTY) != 0u) ) {
		return xuiInventoryGridClearSlot(pGrid, iSlot);
	}
	return xuiInventoryGridSetSlot(pGrid, iSlot, pSlot);
}

static void __xuiInventoryDemoSelected(xui_widget pWidget, int iSlot, void* pUser)
{
	xui_inventory_grid_demo_t* pDemo;

	(void)pWidget;
	pDemo = (xui_inventory_grid_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iSelects++;
		pDemo->iLastSlot = iSlot;
	}
}

static void __xuiInventoryDemoActivated(xui_widget pWidget, int iSlot, int iButton, void* pUser)
{
	xui_inventory_grid_demo_t* pDemo;

	(void)pWidget;
	(void)iButton;
	pDemo = (xui_inventory_grid_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iActivates++;
		pDemo->iLastSlot = iSlot;
	}
}

static void __xuiInventoryDemoContext(xui_widget pWidget, int iSlot, float fX, float fY, void* pUser)
{
	xui_inventory_grid_demo_t* pDemo;
	xui_inventory_slot_t tSlot;

	pDemo = (xui_inventory_grid_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iContexts++;
		pDemo->iLastSlot = iSlot;
		if ( (pWidget != NULL) && (iSlot >= 0) &&
		     (xuiInventoryGridGetSlot(pWidget, iSlot, &tSlot) == XUI_OK) &&
		     ((tSlot.iFlags & XUI_INVENTORY_SLOT_EMPTY) == 0u) &&
		     (tSlot.iCount > 1) ) {
			(void)xuiInventoryGridOpenSplitPopup(pWidget, iSlot, fX, fY);
		}
	}
}

static int __xuiInventoryDemoDrag(xui_widget pWidget, int iSlot, void* pUser)
{
	xui_inventory_grid_demo_t* pDemo;

	(void)pWidget;
	pDemo = (xui_inventory_grid_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iDrags++;
		pDemo->iLastSlot = iSlot;
	}
	return 1;
}

static int __xuiInventoryDemoDrop(xui_widget pWidget, int iFromSlot, int iToSlot, int iDropMode, void* pUser)
{
	xui_inventory_grid_demo_t* pDemo;
	xui_inventory_slot_t tFrom;
	xui_inventory_slot_t tTo;

	pDemo = (xui_inventory_grid_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDemo == NULL) || (iFromSlot < 0) || (iToSlot < 0) ) {
		return 0;
	}
	if ( xuiInventoryGridGetSlot(pWidget, iFromSlot, &tFrom) != XUI_OK ||
	     xuiInventoryGridGetSlot(pWidget, iToSlot, &tTo) != XUI_OK ) {
		return 0;
	}
	(void)__xuiInventoryDemoSetSlot(pWidget, iToSlot, &tFrom);
	(void)__xuiInventoryDemoSetSlot(pWidget, iFromSlot, &tTo);
	pDemo->iDrops++;
	pDemo->iLastDropFrom = iFromSlot;
	pDemo->iLastDropTo = iToSlot;
	pDemo->iLastDropMode = iDropMode;
	return 1;
}

static void __xuiInventoryDemoSplit(xui_widget pWidget, int iSlot, int iCount, void* pUser)
{
	xui_inventory_grid_demo_t* pDemo;

	(void)pWidget;
	pDemo = (xui_inventory_grid_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iSplits++;
		pDemo->iLastSplitSlot = iSlot;
		pDemo->iLastSplitCount = iCount;
	}
}

static int __xuiInventoryDemoTooltip(xui_widget pWidget, int iSlot, const xui_inventory_slot_t* pSlot, char* sBuffer, int iCapacity, void* pUser)
{
	(void)pWidget;
	(void)iSlot;
	(void)pUser;
	if ( (sBuffer == NULL) || (iCapacity <= 0) || (pSlot == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pSlot->iMaxCount > 1 ) {
		snprintf(sBuffer, (size_t)iCapacity, "Stack %d / %d, drag to move or right click to split.", pSlot->iCount, pSlot->iMaxCount);
	} else if ( (pSlot->iFlags & XUI_INVENTORY_SLOT_LOCKED) != 0u ) {
		snprintf(sBuffer, (size_t)iCapacity, "This inventory slot is locked.");
	} else if ( (pSlot->iFlags & XUI_INVENTORY_SLOT_DISABLED) != 0u ) {
		snprintf(sBuffer, (size_t)iCapacity, "This equipment slot is unavailable.");
	} else {
		snprintf(sBuffer, (size_t)iCapacity, "Drag to reorder. Double click or press Enter to activate.");
	}
	sBuffer[iCapacity - 1] = '\0';
	return XUI_OK;
}

static int __xuiInventoryDemoSetupCallbacks(xui_inventory_grid_demo_t* pDemo, xui_widget pGrid)
{
	int iRet;

	iRet = xuiInventoryGridSetSelectCallback(pGrid, __xuiInventoryDemoSelected, pDemo);
	if ( iRet == XUI_OK ) iRet = xuiInventoryGridSetActivateCallback(pGrid, __xuiInventoryDemoActivated, pDemo);
	if ( iRet == XUI_OK ) iRet = xuiInventoryGridSetContextCallback(pGrid, __xuiInventoryDemoContext, pDemo);
	if ( iRet == XUI_OK ) iRet = xuiInventoryGridSetDragCallback(pGrid, __xuiInventoryDemoDrag, pDemo);
	if ( iRet == XUI_OK ) iRet = xuiInventoryGridSetDropCallback(pGrid, __xuiInventoryDemoDrop, pDemo);
	if ( iRet == XUI_OK ) iRet = xuiInventoryGridSetSplitCallback(pGrid, __xuiInventoryDemoSplit, pDemo);
	if ( iRet == XUI_OK ) iRet = xuiInventoryGridSetTooltipCallback(pGrid, __xuiInventoryDemoTooltip, pDemo);
	return iRet;
}

static int __xuiInventoryDemoCreateGrid(xui_inventory_grid_demo_t* pDemo, xui_widget* ppGrid, xui_rect_t tRect, int iSlots, int iColumns, float fSlotSize, float fGap, int iSelectionMode, uint32_t iSelectedColor)
{
	xui_inventory_grid_desc_t tDesc;
	xui_inventory_grid_layout_t tLayout;
	xui_inventory_grid_colors_t tColors;
	int iRet;

	memset(&tLayout, 0, sizeof(tLayout));
	tLayout.iSize = sizeof(tLayout);
	tLayout.iColumns = iColumns;
	tLayout.iSelectionMode = iSelectionMode;
	tLayout.fSlotSize = fSlotSize;
	tLayout.fSlotGap = fGap;
	tLayout.fPadding = 8.0f;
	tLayout.fIconPadding = 8.0f;
	tLayout.fBorderWidth = 1.0f;
	tLayout.fRadius = 5.0f;
	tLayout.fWheelStep = fSlotSize + fGap;
	tLayout.fDragThreshold = 5.0f;
	memset(&tColors, 0, sizeof(tColors));
	tColors.iSize = sizeof(tColors);
	tColors.iSelectedColor = iSelectedColor;
	tColors.iFocusColor = iSelectedColor;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.iSlotCount = iSlots;
	tDesc.tLayout = tLayout;
	tDesc.tColors = tColors;
	tDesc.bHasLayout = 1;
	tDesc.bHasColors = 1;
	iRet = xuiInventoryGridCreate(pDemo->pContext, ppGrid, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(*ppGrid, tRect);
	iRet = __xuiInventoryDemoSetupCallbacks(pDemo, *ppGrid);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pDemo->pRoot, *ppGrid);
	return iRet;
}

static int __xuiInventoryDemoPopulate(xui_inventory_grid_demo_t* pDemo)
{
	xui_inventory_slot_t tSlot;
	uint32_t arrQuality[] = {
		XUI_COLOR_RGBA(118, 166, 208, 255),
		XUI_COLOR_RGBA(86, 180, 122, 255),
		XUI_COLOR_RGBA(90, 122, 220, 255),
		XUI_COLOR_RGBA(172, 104, 220, 255),
		XUI_COLOR_RGBA(224, 146, 72, 255)
	};
	int i;

	for ( i = 0; i < 18; i++ ) {
		char sText[32];
		snprintf(sText, sizeof(sText), "Item %02d", i + 1);
		__xuiInventoryDemoFillSlot(&tSlot, i, 1000 + i, (i % 4 == 0) ? (i + 2) : 1, 99,
			sText, (i < 9) ? " " : NULL, pDemo->arrIcons[i % DEMO_ICON_COUNT], arrQuality[i % 5], 0u);
		if ( i < 9 ) {
			tSlot.sHotkey[0] = (char)('1' + i);
			tSlot.sHotkey[1] = '\0';
		}
		if ( i == 2 ) {
			tSlot.iFlags |= XUI_INVENTORY_SLOT_COOLDOWN;
			tSlot.fCooldownRate = 0.42f;
		}
		if ( i == 3 ) {
			tSlot.iFlags |= XUI_INVENTORY_SLOT_COOLDOWN | XUI_INVENTORY_SLOT_COOLDOWN_RADIAL;
			tSlot.fCooldownRate = 0.68f;
		}
		if ( i == 4 ) {
			tSlot.iFlags |= XUI_INVENTORY_SLOT_DURABILITY;
			tSlot.fDurabilityRate = 0.56f;
		}
		if ( i == 7 ) {
			tSlot.iFlags |= XUI_INVENTORY_SLOT_EQUIPPED;
		}
		(void)xuiInventoryGridSetSlot(pDemo->pBackpack, i, &tSlot);
	}
	__xuiInventoryDemoFillSlot(&tSlot, 30, 3000, 1, 1, "Locked", NULL, NULL, 0u, XUI_INVENTORY_SLOT_LOCKED);
	(void)xuiInventoryGridSetSlot(pDemo->pBackpack, 30, &tSlot);

	for ( i = 0; i < 10; i++ ) {
		char sHotkey[4];
		snprintf(sHotkey, sizeof(sHotkey), "%d", (i + 1) % 10);
		__xuiInventoryDemoFillSlot(&tSlot, i, 2000 + i, (i % 3) + 1, 50,
			(i % 2 == 0) ? "Skill" : "Tool", sHotkey, pDemo->arrIcons[(i + 2) % DEMO_ICON_COUNT], arrQuality[(i + 1) % 5], 0u);
		(void)xuiInventoryGridSetSlot(pDemo->pQuickbar, i, &tSlot);
	}
	for ( i = 0; i < 6; i++ ) {
		__xuiInventoryDemoFillSlot(&tSlot, i, 4000 + i, 1, 1,
			(i == 0) ? "Helm" : (i == 1) ? "Armor" : (i == 2) ? "Ring" : "Gear",
			NULL, pDemo->arrIcons[(i + 4) % DEMO_ICON_COUNT], arrQuality[(i + 2) % 5], (i == 5) ? XUI_INVENTORY_SLOT_DISABLED : 0u);
		(void)xuiInventoryGridSetSlot(pDemo->pEquipment, i, &tSlot);
	}
	(void)xuiInventoryGridSetSelected(pDemo->pBackpack, 0, 1, 0);
	(void)xuiInventoryGridSetCurrent(pDemo->pBackpack, 0, 0);
	return XUI_OK;
}

static int __xuiInventoryDemoCreateUi(xui_inventory_grid_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiInventoryDemoRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiInventoryDemoAddLabel(pDemo, "XUI InventoryGrid", (xui_rect_t){48.0f, 38.0f, 240.0f, 24.0f}, pDemo->pRoot, XUI_COLOR_RGBA(45, 58, 78, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE, NULL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiInventoryDemoAddLabel(pDemo, "Backpack", (xui_rect_t){48.0f, 74.0f, 120.0f, 18.0f}, pDemo->pRoot, XUI_COLOR_RGBA(74, 88, 108, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE, NULL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiInventoryDemoAddLabel(pDemo, "Equipment", (xui_rect_t){618.0f, 74.0f, 120.0f, 18.0f}, pDemo->pRoot, XUI_COLOR_RGBA(74, 88, 108, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE, NULL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiInventoryDemoAddLabel(pDemo, "Quick slots", (xui_rect_t){48.0f, 398.0f, 120.0f, 18.0f}, pDemo->pRoot, XUI_COLOR_RGBA(74, 88, 108, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE, NULL);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiInventoryDemoCreateGrid(pDemo, &pDemo->pBackpack, (xui_rect_t){48.0f, 98.0f, 498.0f, 284.0f}, 32, 8, 42.0f, 7.0f, XUI_INVENTORY_SELECTION_MULTI, XUI_COLOR_RGBA(47, 128, 237, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiInventoryDemoCreateGrid(pDemo, &pDemo->pQuickbar, (xui_rect_t){48.0f, 422.0f, 510.0f, 66.0f}, 10, 10, 42.0f, 7.0f, XUI_INVENTORY_SELECTION_SINGLE, XUI_COLOR_RGBA(42, 178, 120, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiInventoryDemoCreateGrid(pDemo, &pDemo->pEquipment, (xui_rect_t){618.0f, 98.0f, 136.0f, 196.0f}, 6, 2, 48.0f, 8.0f, XUI_INVENTORY_SELECTION_SINGLE, XUI_COLOR_RGBA(224, 146, 72, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiInventoryDemoPopulate(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiInventoryDemoAddLabel(pDemo, "selects=0 activates=0 context=0 drags=0 drops=0 last=none", (xui_rect_t){48.0f, 506.0f, 720.0f, 22.0f}, pDemo->pRoot, XUI_COLOR_RGBA(74, 88, 108, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE, &pDemo->pStatus);
}

static uint32_t __xuiInventoryDemoReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiInventoryDemoSendButtonTransitions(xui_inventory_grid_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
{
	int iRet;

	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iPressed & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiInventoryDemoHandleInput(xui_inventory_grid_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	fX -= DEMO_OFFSET_X;
	fY -= DEMO_OFFSET_Y;
	iButtons = __xuiInventoryDemoReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, fX, fY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (fWheelX != 0.0f) || (fWheelY != 0.0f) ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, fX, fY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiInventoryDemoSendButtonTransitions(pDemo, fX, fY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiInventoryDemoUpdateStatus(xui_inventory_grid_demo_t* pDemo)
{
	xui_inventory_visible_range_t tRange;
	xui_inventory_slot_query_t tQuery;
	char sText[384];
	int arrSlots[8];
	int iQueryCount;
	int iVisible;
	int iPainted;
	int iTopSlot;

	if ( (pDemo == NULL) || (pDemo->pStatus == NULL) ) return;
	iVisible = 0;
	iPainted = 0;
	iQueryCount = 0;
	iTopSlot = -1;
	if ( (pDemo->pBackpack != NULL) && (xuiInventoryGridGetLastPaintRange(pDemo->pBackpack, &tRange) == XUI_OK) ) {
		iVisible = tRange.iSlotCount;
		iPainted = tRange.iPaintSlotCount;
	}
	if ( pDemo->pBackpack != NULL ) {
		memset(&tQuery, 0, sizeof(tQuery));
		tQuery.iSize = sizeof(tQuery);
		tQuery.iFlags = XUI_INVENTORY_QUERY_EXCLUDE_EMPTY | XUI_INVENTORY_QUERY_SORT_DESCENDING;
		tQuery.iSortMode = XUI_INVENTORY_SORT_COUNT;
		if ( xuiInventoryGridQuerySlots(pDemo->pBackpack, &tQuery, arrSlots, (int)(sizeof(arrSlots) / sizeof(arrSlots[0])), &iQueryCount) == XUI_OK ) {
			iTopSlot = (iQueryCount > 0) ? arrSlots[0] : -1;
		}
	}
	snprintf(sText, sizeof(sText), "selects=%d activates=%d context=%d drags=%d drops=%d splits=%d lastSlot=%d drop=%d->%d mode=%d split=%d:%d visible=%d paint=%d filtered=%d top=%d gamepad=%d",
		pDemo->iSelects, pDemo->iActivates, pDemo->iContexts, pDemo->iDrags, pDemo->iDrops, pDemo->iSplits,
		pDemo->iLastSlot, pDemo->iLastDropFrom, pDemo->iLastDropTo, pDemo->iLastDropMode, pDemo->iLastSplitSlot, pDemo->iLastSplitCount,
		iVisible, iPainted, iQueryCount, iTopSlot, pDemo->bGamepadOK);
	(void)xuiLabelSetText(pDemo->pStatus, sText);
}

static void __xuiInventoryDemoRunChecks(xui_inventory_grid_demo_t* pDemo, int bAutoRun)
{
	xui_scroll_model_t* pScroll;
	xui_inventory_visible_range_t tRange;
	xui_inventory_slot_query_t tQuery;
	xui_inventory_gamepad_profile_t tGamepadProfile;
	float fScrollY;
	int arrSlots[8];
	int iQueryCount;
	int iOk;

	if ( (pDemo == NULL) || !bAutoRun || pDemo->bExerciseDone ) return;
	iOk = (xuiInventoryGridGetSlotCount(pDemo->pBackpack) == 32);
	iOk = iOk && (xuiInventoryGridSetSelected(pDemo->pBackpack, 3, 1, 1) == XUI_OK);
	iOk = iOk && (xuiInventoryGridSetSlotAnimation(pDemo->pBackpack, 0, (xui_animation_object_t*)0x1200, 1u, 1.0f, XUI_COLOR_WHITE) == XUI_OK);
	iOk = iOk && (xuiInventoryGridGetSlotAnimation(pDemo->pBackpack, 0, NULL, NULL, NULL) == (xui_animation_object_t*)0x1200);
	iOk = iOk && (xuiInventoryGridClearSlotAnimation(pDemo->pBackpack, 0) == XUI_OK);
	iOk = iOk && (xuiInventoryGridEnsureSlotVisible(pDemo->pBackpack, 31) == XUI_OK);
	iOk = iOk && (xuiInventoryGridOpenSplitPopup(pDemo->pBackpack, 0, -1.0f, -1.0f) == XUI_OK);
	iOk = iOk && (xuiInventoryGridIsSplitPopupOpen(pDemo->pBackpack) != 0);
	iOk = iOk && (xuiInventoryGridCommitSplitPopup(pDemo->pBackpack) == XUI_OK);
	iOk = iOk && (pDemo->iSplits >= 1);
	iOk = iOk && (xuiInventoryGridGetTooltipVisible(pDemo->pBackpack) != 0);
	iOk = iOk && (xuiInventoryGridSetTooltipVisible(pDemo->pBackpack, 0) == XUI_OK);
	iOk = iOk && (xuiInventoryGridGetTooltipVisible(pDemo->pBackpack) == 0);
	iOk = iOk && (xuiInventoryGridSetTooltipVisible(pDemo->pBackpack, 1) == XUI_OK);
	iOk = iOk && (xuiInventoryGridGetVisibleRange(pDemo->pBackpack, &tRange) == XUI_OK);
	iOk = iOk && (tRange.iSlotCount > 0) && (tRange.iSlotCount <= xuiInventoryGridGetSlotCount(pDemo->pBackpack));
	memset(&tQuery, 0, sizeof(tQuery));
	tQuery.iSize = sizeof(tQuery);
	tQuery.iFlags = XUI_INVENTORY_QUERY_EXCLUDE_EMPTY | XUI_INVENTORY_QUERY_SORT_DESCENDING;
	tQuery.iSortMode = XUI_INVENTORY_SORT_COUNT;
	iOk = iOk && (xuiInventoryGridQuerySlots(pDemo->pBackpack, &tQuery, arrSlots, (int)(sizeof(arrSlots) / sizeof(arrSlots[0])), &iQueryCount) == XUI_OK);
	iOk = iOk && (iQueryCount > 0) && (arrSlots[0] >= 0);
	iOk = iOk && (xuiInventoryGridGetGamepadProfile(pDemo->pBackpack, &tGamepadProfile) == XUI_OK);
	iOk = iOk && (tGamepadProfile.iAcceptButton == XUI_INVENTORY_GAMEPAD_BUTTON_ACCEPT);
	iOk = iOk && (xuiInventoryGridSetCurrent(pDemo->pBackpack, 0, 0) == XUI_OK);
	iOk = iOk && (xuiInventoryGridGamepadButton(pDemo->pBackpack, tGamepadProfile.iRightButton, 1, 0u) == XUI_OK);
	iOk = iOk && (xuiInventoryGridGetCurrent(pDemo->pBackpack) == 1);
	iOk = iOk && (xuiInventoryGridGamepadButton(pDemo->pBackpack, tGamepadProfile.iAcceptButton, 1, 0u) == XUI_OK);
	iOk = iOk && (pDemo->iActivates > 0);
	pDemo->bGamepadOK = iOk ? 1 : 0;
	pScroll = xuiInventoryGridGetScrollModel(pDemo->pBackpack);
	fScrollY = 0.0f;
	if ( pScroll != NULL ) {
		(void)xuiScrollModelGetOffset(pScroll, NULL, &fScrollY);
	}
	iOk = iOk && (pScroll != NULL) && (fScrollY >= 0.0f);
	pDemo->bDynamicOK = iOk;
	pDemo->bExerciseDone = 1;
}

static int __xuiInventoryDemoCreateAssets(xui_inventory_grid_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	uint32_t arrIconColors[DEMO_ICON_COUNT] = {
		XUI_COLOR_RGBA(47, 128, 237, 255),
		XUI_COLOR_RGBA(42, 178, 120, 255),
		XUI_COLOR_RGBA(224, 146, 72, 255),
		XUI_COLOR_RGBA(172, 104, 220, 255),
		XUI_COLOR_RGBA(212, 76, 92, 255),
		XUI_COLOR_RGBA(48, 173, 194, 255),
		XUI_COLOR_RGBA(120, 142, 160, 255),
		XUI_COLOR_RGBA(235, 194, 70, 255)
	};
	int i;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiInventoryDemoFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	for ( i = 0; i < DEMO_ICON_COUNT; i++ ) {
		iRet = __xuiInventoryDemoCreateIcon(pDemo, i, arrIconColors[i]);
		if ( iRet != XUI_OK ) return iRet;
	}
	return __xuiInventoryDemoCreateUi(pDemo);
}

static void __xuiInventoryDemoDestroyAssets(xui_inventory_grid_demo_t* pDemo)
{
	int i;

	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
	}
	for ( i = 0; i < DEMO_ICON_COUNT; i++ ) {
		if ( pDemo->arrIcons[i] != NULL ) {
			pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->arrIcons[i]);
			pDemo->arrIcons[i] = NULL;
		}
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
		pDemo->pFont = NULL;
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
		pDemo->pTarget = NULL;
	}
}

static int __xuiInventoryDemoFrame(void* pUser)
{
	xui_inventory_grid_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_inventory_grid_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiInventoryDemoHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiInventoryDemoRunChecks(pDemo, bAutoRun);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiInventoryDemoUpdateStatus(pDemo);
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bLayoutOK = (xuiWidgetGetCacheSurface(pDemo->pBackpack, 0) != NULL) || (xuiInventoryGridGetSlotCount(pDemo->pBackpack) == 32);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bLayoutOK = (xuiWidgetGetCacheSurface(pDemo->pBackpack, 0) != NULL) &&
	                   (xuiWidgetGetCacheSurface(pDemo->pQuickbar, 0) != NULL) &&
	                   (xuiWidgetGetCacheSurface(pDemo->pEquipment, 0) != NULL);
	xgeClear(XUI_COLOR_RGBA(22, 26, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		memset(&tCacheStats, 0, sizeof(tCacheStats));
		tStats.iSize = sizeof(tStats);
		tCacheStats.iSize = sizeof(tCacheStats);
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		(void)xuiGetCacheStats(pDemo->pContext, &tCacheStats);
		printf("xui_inventory_grid final-summary frames=%d create=%d layout=%d dynamic=%d gamepad=%d slots=%d changes=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bDynamicOK, pDemo->bGamepadOK,
			xuiInventoryGridGetSlotCount(pDemo->pBackpack), xuiInventoryGridGetChangeCount(pDemo->pBackpack),
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_inventory_grid_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	tDemo.iLastSlot = -1;
	tDemo.iLastDropFrom = -1;
	tDemo.iLastDropTo = -1;
	tDemo.iLastSplitSlot = -1;
	iRet = __xuiInventoryDemoParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiInventoryDemoUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI InventoryGrid";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_inventory_grid: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiInventoryDemoCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_inventory_grid: create assets failed: %d\n", iRet);
		__xuiInventoryDemoDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	tDemo.bCreateOK = 1;
	iRet = xgeRun(__xuiInventoryDemoFrame, &tDemo);
	__xuiInventoryDemoDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK &&
	        ((tDemo.iMaxFrames <= 0 && tDemo.fMaxSeconds <= 0.0) || tDemo.bDynamicOK)) ? 0 : 1;
}
