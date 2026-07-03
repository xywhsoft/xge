#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 840
#define DEMO_TARGET_H 420
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define TREE_COUNT 3
#define LABEL_COUNT 5
#define LARGE_GROUPS 6
#define LARGE_CHILDREN 12

typedef struct xui_treeview_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pLabel[LABEL_COUNT];
	xui_widget pTree[TREE_COUNT];
	xui_widget pStatus;
	char arrLargeText[96][40];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int iSelectCount[TREE_COUNT];
	int iLastSelected[TREE_COUNT];
	int iCustomPaintCount;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	int bInteractionOK;
	int bScrollOK;
	int bCustomOK;
} xui_treeview_demo_t;

static void __xuiTreeViewUsage(void)
{
	printf("usage: xui_treeview [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiTreeViewParseArgs(xui_treeview_demo_t* pDemo, int argc, char** argv)
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
			__xuiTreeViewUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiTreeViewFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc",
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

static int __xuiTreeViewRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_treeview_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_treeview_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(234, 242, 250, 255));
	}
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		tPanel = (xui_rect_t){24.0f, 22.0f, tRect.fW - 48.0f, tRect.fH - 44.0f};
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tPanel, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static void __xuiTreeViewSelected(xui_widget pWidget, int iNodeId, void* pUser)
{
	xui_treeview_demo_t* pDemo;
	char sText[160];
	int i;

	pDemo = (xui_treeview_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	for ( i = 0; i < TREE_COUNT; i++ ) {
		if ( pDemo->pTree[i] == pWidget ) {
			pDemo->iSelectCount[i]++;
			pDemo->iLastSelected[i] = iNodeId;
			break;
		}
	}
	snprintf(sText, sizeof(sText), "Selected node=%d changes=%d", iNodeId, (i >= 0 && i < TREE_COUNT) ? pDemo->iSelectCount[i] : 0);
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sText);
	}
}

static int __xuiTreeViewCustomRender(xui_widget pWidget, int iNodeId, int iVisible, const xui_tree_view_node_t* pNode, xui_draw_context pDraw, xui_rect_t tRect, int iState, void* pUser)
{
	xui_treeview_demo_t* pDemo;
	xui_proxy pProxy;
	xui_rect_t tFill;
	xui_rect_t tText;
	xui_rect_t tBadge;
	uint32_t iRow;
	uint32_t iText;
	uint32_t iBadge;
	uint32_t iExpander;
	uint32_t iExpanderBg;

	(void)pWidget;
	(void)iNodeId;
	(void)iVisible;
	pDemo = (xui_treeview_demo_t*)pUser;
	if ( (pDemo == NULL) || (pNode == NULL) ) return 0;
	pDemo->iCustomPaintCount++;
	pProxy = &pDemo->tProxy;
	iRow = XUI_COLOR_RGBA(246, 250, 252, 255);
	if ( (iState & XUI_TREE_ITEM_HOVER) != 0 ) iRow = XUI_COLOR_RGBA(226, 238, 250, 255);
	if ( (iState & XUI_TREE_ITEM_SELECTED) != 0 ) iRow = XUI_COLOR_RGBA(55, 128, 205, 255);
	if ( pProxy->drawRectFill != NULL ) {
		tFill = (xui_rect_t){tRect.fX + 3.0f, tRect.fY + 2.0f, tRect.fW - 6.0f, tRect.fH - 4.0f};
		(void)pProxy->drawRectFill(pProxy, pDraw, tFill, iRow);
	}
	if ( (iState & XUI_TREE_ITEM_HAS_CHILDREN) != 0 && pProxy->drawTriangleFill != NULL ) {
		xui_vec2_t a;
		xui_vec2_t b;
		xui_vec2_t c;
		float cx = tRect.fX + 14.0f + (float)pNode->iDepth * 18.0f;
		float cy = tRect.fY + tRect.fH * 0.5f;
		iExpander = ((iState & XUI_TREE_ITEM_SELECTED) != 0) ? XUI_COLOR_RGBA(255, 255, 255, 255) : XUI_COLOR_RGBA(45, 102, 166, 235);
		iExpanderBg = ((iState & XUI_TREE_ITEM_SELECTED) != 0) ? XUI_COLOR_RGBA(18, 86, 178, 145) : XUI_COLOR_RGBA(205, 225, 246, 165);
		if ( pProxy->drawRectFill != NULL ) {
			(void)pProxy->drawRectFill(pProxy, pDraw, (xui_rect_t){cx - 8.0f, cy - 8.0f, 16.0f, 16.0f}, iExpanderBg);
		}
		if ( (iState & XUI_TREE_ITEM_EXPANDED) != 0 ) {
			a = (xui_vec2_t){cx - 5.0f, cy - 2.5f};
			b = (xui_vec2_t){cx + 5.0f, cy - 2.5f};
			c = (xui_vec2_t){cx, cy + 4.0f};
		} else {
			a = (xui_vec2_t){cx - 2.5f, cy - 5.0f};
			b = (xui_vec2_t){cx - 2.5f, cy + 5.0f};
			c = (xui_vec2_t){cx + 4.5f, cy};
		}
		(void)pProxy->drawTriangleFill(pProxy, pDraw, a, b, c, iExpander);
	}
	iText = ((iState & XUI_TREE_ITEM_SELECTED) != 0) ? XUI_COLOR_RGBA(255, 255, 255, 255) : XUI_COLOR_RGBA(31, 50, 73, 255);
	if ( (iState & XUI_TREE_ITEM_DISABLED) != 0 ) iText = XUI_COLOR_RGBA(132, 146, 162, 210);
	if ( pDemo->pFont != NULL && pProxy->drawText != NULL ) {
		tText = (xui_rect_t){tRect.fX + 30.0f + (float)pNode->iDepth * 18.0f, tRect.fY, tRect.fW - 106.0f, tRect.fH};
		(void)pProxy->drawText(pProxy, pDraw, pDemo->pFont, pNode->sText, tText, iText, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	if ( pNode->bHasChildren || pNode->bChecked ) {
		tBadge = (xui_rect_t){tRect.fX + tRect.fW - 64.0f, tRect.fY + 5.0f, 50.0f, 16.0f};
		iBadge = pNode->bHasChildren ? XUI_COLOR_RGBA(65, 139, 202, 230) : XUI_COLOR_RGBA(48, 157, 112, 230);
		if ( pProxy->drawRectFill != NULL ) {
			(void)pProxy->drawRectFill(pProxy, pDraw, tBadge, iBadge);
		}
		if ( pDemo->pFont != NULL && pProxy->drawText != NULL ) {
			(void)pProxy->drawText(pProxy, pDraw, pDemo->pFont, pNode->bHasChildren ? "Group" : "Done", tBadge, XUI_COLOR_RGBA(255, 255, 255, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		}
	}
	return 1;
}

static int __xuiTreeViewAddLabel(xui_treeview_demo_t* pDemo, int iIndex, const char* sText, xui_rect_t tRect)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= LABEL_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(54, 68, 88, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pLabel, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[iIndex] = pLabel;
	return XUI_OK;
}

static int __xuiTreeViewAddTree(xui_treeview_demo_t* pDemo, int iIndex, xui_rect_t tRect)
{
	xui_tree_view_desc_t tDesc;
	xui_widget pTree;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= TREE_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.fItemHeight = (iIndex == 2) ? 25.0f : 24.0f;
	iRet = xuiTreeViewCreate(pDemo->pContext, &pTree, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiTreeViewSetSelect(pTree, __xuiTreeViewSelected, pDemo);
	if ( iIndex == 2 ) {
		(void)xuiTreeViewSetColors(pTree,
			XUI_COLOR_RGBA(247, 253, 250, 255),
			XUI_COLOR_RGBA(103, 176, 146, 255),
			XUI_COLOR_RGBA(38, 141, 104, 255),
			XUI_COLOR_RGBA(255, 255, 255, 0),
			XUI_COLOR_RGBA(226, 247, 239, 255),
			XUI_COLOR_RGBA(42, 151, 110, 255),
			XUI_COLOR_RGBA(31, 76, 60, 255),
			XUI_COLOR_RGBA(128, 152, 144, 210));
		(void)xuiTreeViewSetScrollbarColors(pTree,
			XUI_COLOR_RGBA(219, 239, 233, 255),
			XUI_COLOR_RGBA(47, 150, 110, 245),
			XUI_COLOR_RGBA(31, 170, 120, 250),
			XUI_COLOR_RGBA(19, 125, 88, 255),
			XUI_COLOR_RGBA(39, 150, 106, 180),
			XUI_COLOR_RGBA(158, 174, 168, 135));
		(void)xuiTreeViewSetItemRenderer(pTree, __xuiTreeViewCustomRender, pDemo);
	}
	xuiWidgetSetRect(pTree, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pTree);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pTree);
		return iRet;
	}
	pDemo->pTree[iIndex] = pTree;
	return XUI_OK;
}

static void __xuiTreeViewPopulateBasic(xui_treeview_demo_t* pDemo)
{
	xui_widget pTree = pDemo->pTree[0];

	(void)xuiTreeViewAddNode(pTree, 10, -1, "Project");
	(void)xuiTreeViewAddNode(pTree, 20, 10, "src");
	(void)xuiTreeViewAddNode(pTree, 30, 20, "xui_tree_view.c");
	(void)xuiTreeViewAddNode(pTree, 40, 20, "disabled file");
	(void)xuiTreeViewAddNode(pTree, 50, 10, "examples");
	(void)xuiTreeViewAddNode(pTree, 60, 50, "xui_treeview");
	(void)xuiTreeViewAddNode(pTree, 70, 10, "docs");
	(void)xuiTreeViewSetNodeExpanded(pTree, 10, 1);
	(void)xuiTreeViewSetNodeExpanded(pTree, 20, 1);
	(void)xuiTreeViewSetNodeEnabled(pTree, 40, 0);
	(void)xuiTreeViewSetNodeChecked(pTree, 30, 1);
	(void)xuiTreeViewSetNodeDecorations(pTree, 70, 0, 0);
	(void)xuiTreeViewSetSelected(pTree, 30);
}

static void __xuiTreeViewPopulateCheck(xui_treeview_demo_t* pDemo)
{
	xui_widget pTree = pDemo->pTree[1];

	(void)xuiTreeViewAddNode(pTree, 3000, -1, "Settings");
	(void)xuiTreeViewAddNode(pTree, 3010, 3000, "Enable auto save");
	(void)xuiTreeViewAddNode(pTree, 3020, 3000, "Use high contrast hover");
	(void)xuiTreeViewAddNode(pTree, 3030, 3000, "Locked option disabled");
	(void)xuiTreeViewAddNode(pTree, 3040, 3000, "No icon row");
	(void)xuiTreeViewAddNode(pTree, 3050, 3000, "Nested group");
	(void)xuiTreeViewAddNode(pTree, 3060, 3050, "Nested checked option");
	(void)xuiTreeViewSetNodeExpanded(pTree, 3000, 1);
	(void)xuiTreeViewSetNodeExpanded(pTree, 3050, 1);
	(void)xuiTreeViewSetNodeChecked(pTree, 3010, 1);
	(void)xuiTreeViewSetNodeDecorations(pTree, 3020, 1, 1);
	(void)xuiTreeViewSetNodeDecorations(pTree, 3030, 1, 1);
	(void)xuiTreeViewSetNodeEnabled(pTree, 3030, 0);
	(void)xuiTreeViewSetNodeDecorations(pTree, 3040, 0, 0);
	(void)xuiTreeViewSetNodeChecked(pTree, 3060, 1);
	(void)xuiTreeViewSetSelected(pTree, 3010);
}

static void __xuiTreeViewPopulateLarge(xui_treeview_demo_t* pDemo)
{
	xui_widget pTree = pDemo->pTree[2];
	int i;
	int j;
	int iText;
	int iGroupId;
	int iNodeId;

	(void)xuiTreeViewSetScrollbarMode(pTree, XUI_SCROLLBAR_MODE_FULL);
	(void)xuiTreeViewAddNode(pTree, 1000, -1, "Large data");
	(void)xuiTreeViewSetNodeExpanded(pTree, 1000, 1);
	iText = 0;
	for ( i = 0; i < LARGE_GROUPS; i++ ) {
		iGroupId = 1100 + i;
		snprintf(pDemo->arrLargeText[iText], sizeof(pDemo->arrLargeText[iText]), "Group %02d", i + 1);
		(void)xuiTreeViewAddNode(pTree, iGroupId, 1000, pDemo->arrLargeText[iText++]);
		(void)xuiTreeViewSetNodeExpanded(pTree, iGroupId, 1);
		for ( j = 0; j < LARGE_CHILDREN; j++ ) {
			iNodeId = 2000 + i * LARGE_CHILDREN + j;
			snprintf(pDemo->arrLargeText[iText], sizeof(pDemo->arrLargeText[iText]), "Virtual item %03d", i * LARGE_CHILDREN + j + 1);
			(void)xuiTreeViewAddNode(pTree, iNodeId, iGroupId, pDemo->arrLargeText[iText++]);
			if ( (j % 5) == 0 ) {
				(void)xuiTreeViewSetNodeChecked(pTree, iNodeId, 1);
			}
		}
	}
	(void)xuiTreeViewSetSelected(pTree, 2032);
	(void)xuiTreeViewSetScroll(pTree, 420.0f);
}

static int __xuiTreeViewCreateUi(xui_treeview_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiTreeViewRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiTreeViewAddLabel(pDemo, 0, "XUI TreeView", (xui_rect_t){48.0f, 30.0f, 180.0f, 24.0f}) != XUI_OK ||
	     __xuiTreeViewAddLabel(pDemo, 1, "Basic tree", (xui_rect_t){48.0f, 64.0f, 180.0f, 22.0f}) != XUI_OK ||
	     __xuiTreeViewAddLabel(pDemo, 2, "Check tree", (xui_rect_t){306.0f, 64.0f, 180.0f, 22.0f}) != XUI_OK ||
	     __xuiTreeViewAddLabel(pDemo, 3, "Custom large tree", (xui_rect_t){564.0f, 64.0f, 180.0f, 22.0f}) != XUI_OK ||
	     __xuiTreeViewAddLabel(pDemo, 4, "Selected node=none changes=0", (xui_rect_t){48.0f, 364.0f, 420.0f, 24.0f}) != XUI_OK ) {
		return XUI_ERROR;
	}
	pDemo->pStatus = pDemo->pLabel[4];
	if ( __xuiTreeViewAddTree(pDemo, 0, (xui_rect_t){48.0f, 90.0f, 226.0f, 255.0f}) != XUI_OK ||
	     __xuiTreeViewAddTree(pDemo, 1, (xui_rect_t){306.0f, 90.0f, 226.0f, 255.0f}) != XUI_OK ||
	     __xuiTreeViewAddTree(pDemo, 2, (xui_rect_t){564.0f, 90.0f, 226.0f, 255.0f}) != XUI_OK ) {
		return XUI_ERROR;
	}
	__xuiTreeViewPopulateBasic(pDemo);
	__xuiTreeViewPopulateCheck(pDemo);
	__xuiTreeViewPopulateLarge(pDemo);
	return XUI_OK;
}

static uint32_t __xuiTreeViewReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiTreeViewHandleInput(xui_treeview_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	uint32_t iMods;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiTreeViewReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( fWheelX != 0.0f || fWheelY != 0.0f ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iMods = 0;
	if ( xgeKeyPressed(XGE_KEY_UP) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_UP, iMods);
	if ( xgeKeyPressed(XGE_KEY_DOWN) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_DOWN, iMods);
	if ( xgeKeyPressed(XGE_KEY_LEFT) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_LEFT, iMods);
	if ( xgeKeyPressed(XGE_KEY_RIGHT) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_RIGHT, iMods);
	if ( xgeKeyPressed(XGE_KEY_HOME) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_HOME, iMods);
	if ( xgeKeyPressed(XGE_KEY_END) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_END, iMods);
	if ( xgeKeyPressed(XGE_KEY_ENTER) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_ENTER, iMods);
	if ( xgeKeyPressed(XGE_KEY_SPACE) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_SPACE, iMods);
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiTreeViewClickNode(xui_treeview_demo_t* pDemo, xui_widget pTree, int iNodeId, float fOffsetX)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;
	float fX;
	float fY;
	int iVisible;
	int iRet;

	iVisible = xuiTreeViewGetVisibleIndexOfId(pTree, iNodeId);
	if ( iVisible < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	tWorld = xuiWidgetGetWorldRect(pTree);
	tItem = xuiTreeViewGetItemRect(pTree, iVisible);
	fX = tWorld.fX + tItem.fX + fOffsetX;
	fY = tWorld.fY + tItem.fY + tItem.fH * 0.5f;
	iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	return iRet;
}

static void __xuiTreeViewRunChecks(xui_treeview_demo_t* pDemo, int bExerciseInput)
{
	const xui_tree_view_node_t* pNode;
	float fCheckX;
	float fExpanderX;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pTree[0] != NULL) && (xuiTreeViewGetFrameWidget(pDemo->pTree[0]) != NULL);
	pDemo->bLayoutOK = xuiScrollFrameIsVScrollBarVisible(xuiTreeViewGetFrameWidget(pDemo->pTree[2])) &&
		!xuiScrollFrameIsHScrollBarVisible(xuiTreeViewGetFrameWidget(pDemo->pTree[0]));
	if ( !pDemo->bExerciseDone ) {
		pDemo->bStateOK =
			(xuiTreeViewGetVisibleCount(pDemo->pTree[0]) == 6) &&
			(xuiTreeViewGetVisibleNodeId(pDemo->pTree[0], 2) == 30) &&
			(xuiTreeViewGetSelected(pDemo->pTree[0]) == 30) &&
			(xuiTreeViewGetNodeEnabled(pDemo->pTree[0], 40) == 0) &&
			(xuiTreeViewGetNodeChecked(pDemo->pTree[0], 30) == 1);
	}
	if ( bExerciseInput && !pDemo->bExerciseDone ) {
		(void)__xuiTreeViewClickNode(pDemo, pDemo->pTree[0], 50, 88.0f);
		pNode = xuiTreeViewGetNodeById(pDemo->pTree[1], 3020);
		fCheckX = (pNode != NULL) ? (8.0f + (float)pNode->iDepth * 18.0f + 23.0f) : 34.0f;
		(void)__xuiTreeViewClickNode(pDemo, pDemo->pTree[1], 3020, fCheckX);
		pNode = xuiTreeViewGetNodeById(pDemo->pTree[1], 3050);
		fExpanderX = (pNode != NULL) ? (8.0f + (float)pNode->iDepth * 18.0f + 6.0f) : 14.0f;
		(void)__xuiTreeViewClickNode(pDemo, pDemo->pTree[1], 3050, fExpanderX);
		(void)xuiTreeViewEnsureVisible(pDemo->pTree[2], 2035);
		pDemo->bExerciseDone = 1;
	} else if ( !bExerciseInput ) {
		pDemo->bInteractionOK = 1;
		pDemo->bScrollOK = 1;
	}
	if ( pDemo->bExerciseDone ) {
		pDemo->bInteractionOK =
			(pDemo->iLastSelected[0] == 50) &&
			(pDemo->iSelectCount[0] >= 1) &&
			(xuiTreeViewGetSelected(pDemo->pTree[0]) == 50) &&
			(xuiTreeViewGetNodeChecked(pDemo->pTree[1], 3020) == 1) &&
			(xuiTreeViewGetNodeExpanded(pDemo->pTree[1], 3050) == 0);
		pDemo->bScrollOK = (xuiTreeViewGetScroll(pDemo->pTree[2]) > 0.0f);
	}
	pDemo->bCustomOK = (pDemo->iCustomPaintCount > 0) && (xuiTreeViewGetSelected(pDemo->pTree[2]) == 2032);
}

static int __xuiTreeViewCreateAssets(xui_treeview_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
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
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiTreeViewFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiTreeViewCreateUi(pDemo);
}

static void __xuiTreeViewDestroyAssets(xui_treeview_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
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

static int __xuiTreeViewFrame(void* pUser)
{
	xui_treeview_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_treeview_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiTreeViewHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiTreeViewRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(234, 242, 250, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(18, 23, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		printf("xui_treeview final-summary frames=%d create=%d layout=%d state=%d interaction=%d scroll=%d custom=%d selected=%d scrollY=%.2f customPaint=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bStateOK, pDemo->bInteractionOK,
			pDemo->bScrollOK, pDemo->bCustomOK, xuiTreeViewGetSelected(pDemo->pTree[0]),
			xuiTreeViewGetScroll(pDemo->pTree[2]), pDemo->iCustomPaintCount, tStats.iUpdatedCaches, tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_treeview_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;
	int i;

	memset(&tDemo, 0, sizeof(tDemo));
	for ( i = 0; i < TREE_COUNT; i++ ) {
		tDemo.iLastSelected[i] = -1;
	}
	iRet = __xuiTreeViewParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiTreeViewUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI TreeView";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_treeview: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiTreeViewCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_treeview: create assets failed: %d\n", iRet);
		__xuiTreeViewDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiTreeViewFrame, &tDemo);
	__xuiTreeViewDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bStateOK && tDemo.bInteractionOK && tDemo.bScrollOK && tDemo.bCustomOK) ? 0 : 1;
}
