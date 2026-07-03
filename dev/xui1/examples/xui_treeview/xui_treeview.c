#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TREE_COUNT 4
#define LABEL_COUNT 8
#define LARGE_GROUPS 8
#define LARGE_CHILDREN 22

enum {
	TREE_BASIC = 0,
	TREE_LARGE = 1,
	TREE_CHECK = 2,
	TREE_CUSTOM = 3
};

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel[TREE_COUNT];
	xge_xui_widget pTreeWidget[TREE_COUNT];
	xge_xui_label_t tLabel[LABEL_COUNT];
	xge_xui_tree_view_t tTree[TREE_COUNT];
	char arrLargeText[LARGE_GROUPS * (LARGE_CHILDREN + 1)][64];
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iSelectCount[TREE_COUNT];
	int iLastSelected[TREE_COUNT];
	int iCustomPaintCount;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	int bInteractionOK;
	int bScrollOK;
	int bCustomOK;
	int bRanInteraction;
} app_state_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_treeview font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_xui_widget NewWidget(float fHeight)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), (fHeight > 0.0f) ? xgeXuiSizePx(fHeight) : xgeXuiSizeGrow(1.0f));
	}
	return pWidget;
}

static void StylePanel(xge_xui_widget pPanel)
{
	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pPanel, xgeXuiSizePercent(100.0f), xgeXuiSizePercent(100.0f));
	xgeXuiWidgetSetPaddingPx(pPanel, 12.0f, 10.0f, 12.0f, 12.0f);
	xgeXuiWidgetSetGap(pPanel, 8.0f);
	xgeXuiWidgetSetBackground(pPanel, XGE_COLOR_RGBA(248, 250, 253, 255));
	xgeXuiWidgetSetBorder(pPanel, 1.0f, XGE_COLOR_RGBA(170, 184, 202, 255));
	xgeXuiWidgetSetRadius(pPanel, 4.0f);
}

static int AddLabel(app_state_t* pApp, xge_xui_widget pParent, const char* sText, uint32_t iColor)
{
	xge_xui_widget pWidget;
	int iIndex;

	for ( iIndex = 0; iIndex < LABEL_COUNT; iIndex++ ) {
		if ( pApp->tLabel[iIndex].pWidget == NULL ) {
			break;
		}
	}
	if ( iIndex >= LABEL_COUNT ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget = NewWidget(28.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiLabelInit(&pApp->tLabel[iIndex], pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tLabel[iIndex], iColor);
	xgeXuiLabelSetAlign(&pApp->tLabel[iIndex], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	xgeXuiWidgetAdd(pParent, pWidget);
	return XGE_OK;
}

static int AddPanel(app_state_t* pApp, xge_xui_widget pRoot, int iIndex, const char* sTitle)
{
	xge_xui_widget pPanel;

	pPanel = NewWidget(0.0f);
	if ( pPanel == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	StylePanel(pPanel);
	xgeXuiWidgetAdd(pRoot, pPanel);
	pApp->pPanel[iIndex] = pPanel;
	return AddLabel(pApp, pPanel, sTitle, XGE_COLOR_RGBA(42, 58, 78, 255));
}

static void OnTreeSelect(xge_xui_widget pWidget, int iNodeId, void* pUser)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pUser;
	if ( (pApp == NULL) || (pWidget == NULL) ) {
		return;
	}
	for ( i = 0; i < TREE_COUNT; i++ ) {
		if ( pApp->pTreeWidget[i] == pWidget ) {
			pApp->iSelectCount[i]++;
			pApp->iLastSelected[i] = iNodeId;
			break;
		}
	}
}

static int CustomTreeFloor(float fValue)
{
	int iValue;

	iValue = (int)fValue;
	return (fValue < (float)iValue) ? (iValue - 1) : iValue;
}

static int CustomTreeCeil(float fValue)
{
	int iValue;

	iValue = (int)fValue;
	return (fValue > (float)iValue) ? (iValue + 1) : iValue;
}

static float CustomTreeMin3(float fA, float fB, float fC)
{
	float fMin;

	fMin = (fA < fB) ? fA : fB;
	return (fMin < fC) ? fMin : fC;
}

static float CustomTreeMax3(float fA, float fB, float fC)
{
	float fMax;

	fMax = (fA > fB) ? fA : fB;
	return (fMax > fC) ? fMax : fC;
}

static uint32_t CustomTreeAlphaColor(uint32_t iColor, float fAlphaRate)
{
	int iAlpha;

	if ( fAlphaRate <= 0.0f ) {
		return XGE_COLOR_RGBA(0, 0, 0, 0);
	}
	if ( fAlphaRate > 1.0f ) {
		fAlphaRate = 1.0f;
	}
	iAlpha = (int)((float)XGE_COLOR_GET_A(iColor) * fAlphaRate + 0.5f);
	if ( iAlpha > 255 ) {
		iAlpha = 255;
	}
	return XGE_COLOR_RGBA(XGE_COLOR_GET_R(iColor), XGE_COLOR_GET_G(iColor), XGE_COLOR_GET_B(iColor), iAlpha);
}

static float CustomTreeTriangleEdge(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tP)
{
	return (tP.fX - tA.fX) * (tB.fY - tA.fY) - (tP.fY - tA.fY) * (tB.fX - tA.fX);
}

static int CustomTreePointInTriangle(xge_vec2_t tP, xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC)
{
	float fD1;
	float fD2;
	float fD3;
	int bNegative;
	int bPositive;

	fD1 = CustomTreeTriangleEdge(tA, tB, tP);
	fD2 = CustomTreeTriangleEdge(tB, tC, tP);
	fD3 = CustomTreeTriangleEdge(tC, tA, tP);
	bNegative = (fD1 < 0.0f) || (fD2 < 0.0f) || (fD3 < 0.0f);
	bPositive = (fD1 > 0.0f) || (fD2 > 0.0f) || (fD3 > 0.0f);
	return (bNegative && bPositive) ? 0 : 1;
}

static void CustomTreeDrawAaTriangle(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor)
{
	xge_rect_t tPixel;
	xge_vec2_t tSample;
	float fAlpha;
	int iMinX;
	int iMinY;
	int iMaxX;
	int iMaxY;
	int iX;
	int iY;
	int iSX;
	int iSY;
	int iCoverage;
	const int iSamples = 4;

	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return;
	}
	iMinX = CustomTreeFloor(CustomTreeMin3(tA.fX, tB.fX, tC.fX)) - 1;
	iMinY = CustomTreeFloor(CustomTreeMin3(tA.fY, tB.fY, tC.fY)) - 1;
	iMaxX = CustomTreeCeil(CustomTreeMax3(tA.fX, tB.fX, tC.fX)) + 1;
	iMaxY = CustomTreeCeil(CustomTreeMax3(tA.fY, tB.fY, tC.fY)) + 1;
	for ( iY = iMinY; iY <= iMaxY; iY++ ) {
		for ( iX = iMinX; iX <= iMaxX; iX++ ) {
			iCoverage = 0;
			for ( iSY = 0; iSY < iSamples; iSY++ ) {
				for ( iSX = 0; iSX < iSamples; iSX++ ) {
					tSample.fX = (float)iX + ((float)iSX + 0.5f) / (float)iSamples;
					tSample.fY = (float)iY + ((float)iSY + 0.5f) / (float)iSamples;
					if ( CustomTreePointInTriangle(tSample, tA, tB, tC) ) {
						iCoverage++;
					}
				}
			}
			if ( iCoverage > 0 ) {
				fAlpha = (float)iCoverage / (float)(iSamples * iSamples);
				tPixel = (xge_rect_t){ (float)iX, (float)iY, 1.0f, 1.0f };
				xgeShapeRectFillPx(tPixel, CustomTreeAlphaColor(iColor, fAlpha));
			}
		}
	}
}

static void DrawCustomExpander(xge_rect_t tRow, int iDepth, int bExpanded, uint32_t iColor)
{
	xge_vec2_t tA;
	xge_vec2_t tB;
	xge_vec2_t tC;
	float fBase;
	float fDepth;
	float fCX;
	float fCY;

	fBase = 9.0f;
	fDepth = 6.0f;
	fCX = tRow.fX + 12.0f + (float)iDepth * 18.0f;
	fCY = tRow.fY + tRow.fH * 0.5f;
	if ( bExpanded ) {
		tA = (xge_vec2_t){ fCX, fCY + fDepth * 0.5f };
		tB = (xge_vec2_t){ fCX + fBase * 0.5f, fCY - fDepth * 0.5f };
		tC = (xge_vec2_t){ fCX - fBase * 0.5f, fCY - fDepth * 0.5f };
	} else {
		tA = (xge_vec2_t){ fCX + fDepth * 0.5f, fCY };
		tB = (xge_vec2_t){ fCX - fDepth * 0.5f, fCY - fBase * 0.5f };
		tC = (xge_vec2_t){ fCX - fDepth * 0.5f, fCY + fBase * 0.5f };
	}
	CustomTreeDrawAaTriangle(tA, tB, tC, iColor);
}

static int CustomTreeRenderer(xge_xui_widget pWidget, int iNodeId, int iVisible, const xge_xui_tree_view_node_t* pNode, xge_rect_t tRect, int iState, void* pUser)
{
	app_state_t* pApp;
	xge_rect_t tBadge;
	xge_rect_t tText;
	uint32_t iRow;
	uint32_t iText;
	uint32_t iBadge;

	(void)pWidget;
	(void)iNodeId;
	(void)iVisible;
	pApp = (app_state_t*)pUser;
	if ( (pApp == NULL) || (pNode == NULL) ) {
		return 0;
	}
	pApp->iCustomPaintCount++;
	iRow = XGE_COLOR_RGBA(246, 249, 252, 255);
	if ( (iState & XGE_XUI_TREE_ITEM_HOVER) != 0 ) {
		iRow = XGE_COLOR_RGBA(226, 238, 250, 255);
	}
	if ( (iState & XGE_XUI_TREE_ITEM_SELECTED) != 0 ) {
		iRow = XGE_COLOR_RGBA(205, 226, 248, 255);
	}
	iText = ((iState & XGE_XUI_TREE_ITEM_DISABLED) != 0) ? XGE_COLOR_RGBA(132, 142, 154, 170) : XGE_COLOR_RGBA(36, 50, 68, 255);
	xgeShapeRectFill(tRect, iRow);
	if ( (iState & XGE_XUI_TREE_ITEM_HAS_CHILDREN) != 0 ) {
		DrawCustomExpander(tRect, pNode->iDepth, (iState & XGE_XUI_TREE_ITEM_EXPANDED) != 0, XGE_COLOR_RGBA(52, 118, 178, 235));
	}
	tText = (xge_rect_t){ tRect.fX + 28.0f + (float)pNode->iDepth * 18.0f, tRect.fY, tRect.fW - 110.0f, tRect.fH };
	if ( pApp->bFontReady ) {
		xgeTextDrawRect(&pApp->tFont, pNode->sText, tText, iText, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	if ( pNode->bHasChildren || pNode->bChecked ) {
		tBadge = (xge_rect_t){ tRect.fX + tRect.fW - 68.0f, tRect.fY + 5.0f, 54.0f, 16.0f };
		iBadge = pNode->bHasChildren ? XGE_COLOR_RGBA(66, 139, 202, 230) : XGE_COLOR_RGBA(68, 164, 124, 230);
		xgeShapeRectFill(tBadge, iBadge);
		if ( pApp->bFontReady ) {
			xgeTextDrawRect(&pApp->tFont, pNode->bHasChildren ? "Group" : "Done", tBadge, XGE_COLOR_RGBA(255, 255, 255, 255), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
	return 1;
}

static int AddTree(app_state_t* pApp, int iIndex, float fItemHeight)
{
	xge_xui_widget pWidget;

	pWidget = NewWidget(0.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pApp->pTreeWidget[iIndex] = pWidget;
	if ( xgeXuiTreeViewInit(&pApp->tTree[iIndex], &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiTreeViewSetFont(&pApp->tTree[iIndex], pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiTreeViewSetMetrics(&pApp->tTree[iIndex], fItemHeight, 18.0f);
	xgeXuiTreeViewSetSelect(&pApp->tTree[iIndex], OnTreeSelect, pApp);
	xgeXuiWidgetAdd(pApp->pPanel[iIndex], pWidget);
	return XGE_OK;
}

static void PopulateBasicTree(app_state_t* pApp)
{
	xge_xui_tree_view pTree;

	pTree = &pApp->tTree[TREE_BASIC];
	xgeXuiTreeViewAddNode(pTree, 10, -1, "Project");
	xgeXuiTreeViewAddNode(pTree, 20, 10, "src");
	xgeXuiTreeViewAddNode(pTree, 30, 20, "xge_xui_tree_view.c");
	xgeXuiTreeViewAddNode(pTree, 40, 20, "xge_xui_page.c disabled");
	xgeXuiTreeViewAddNode(pTree, 50, 10, "examples");
	xgeXuiTreeViewAddNode(pTree, 60, 50, "xui_treeview");
	xgeXuiTreeViewAddNode(pTree, 70, 10, "docs");
	xgeXuiTreeViewSetNodeExpanded(pTree, 10, 1);
	xgeXuiTreeViewSetNodeExpanded(pTree, 20, 1);
	xgeXuiTreeViewSetNodeEnabled(pTree, 40, 0);
	xgeXuiTreeViewSetNodeChecked(pTree, 30, 1);
	xgeXuiTreeViewSetNodeDecorations(pTree, 70, 0, 0);
	xgeXuiTreeViewSetSelected(pTree, 30);
}

static void PopulateLargeTree(app_state_t* pApp)
{
	xge_xui_tree_view pTree;
	int i;
	int j;
	int iGroupId;
	int iNodeId;
	int iText;

	pTree = &pApp->tTree[TREE_LARGE];
	xgeXuiTreeViewSetScrollbarMode(pTree, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiTreeViewAddNode(pTree, 1000, -1, "Large data: ScrollFrame backed");
	xgeXuiTreeViewSetNodeExpanded(pTree, 1000, 1);
	iText = 0;
	for ( i = 0; i < LARGE_GROUPS; i++ ) {
		iGroupId = 1100 + i;
		snprintf(pApp->arrLargeText[iText], sizeof(pApp->arrLargeText[iText]), "Group %02d", i + 1);
		xgeXuiTreeViewAddNode(pTree, iGroupId, 1000, pApp->arrLargeText[iText++]);
		xgeXuiTreeViewSetNodeExpanded(pTree, iGroupId, 1);
		for ( j = 0; j < LARGE_CHILDREN; j++ ) {
			iNodeId = 2000 + i * LARGE_CHILDREN + j;
			snprintf(pApp->arrLargeText[iText], sizeof(pApp->arrLargeText[iText]), "Virtual tree item %03d", i * LARGE_CHILDREN + j + 1);
			xgeXuiTreeViewAddNode(pTree, iNodeId, iGroupId, pApp->arrLargeText[iText++]);
			if ( (j % 9) == 0 ) {
				xgeXuiTreeViewSetNodeChecked(pTree, iNodeId, 1);
			}
		}
	}
	xgeXuiTreeViewSetSelected(pTree, 2032);
	xgeXuiTreeViewSetScroll(pTree, 640.0f);
}

static void PopulateCheckTree(app_state_t* pApp)
{
	xge_xui_tree_view pTree;

	pTree = &pApp->tTree[TREE_CHECK];
	xgeXuiTreeViewAddNode(pTree, 3000, -1, "Settings");
	xgeXuiTreeViewAddNode(pTree, 3010, 3000, "Enable auto save");
	xgeXuiTreeViewAddNode(pTree, 3020, 3000, "Use high contrast hover");
	xgeXuiTreeViewAddNode(pTree, 3030, 3000, "Locked option disabled");
	xgeXuiTreeViewAddNode(pTree, 3040, 3000, "No icon row");
	xgeXuiTreeViewAddNode(pTree, 3050, 3000, "Nested group");
	xgeXuiTreeViewAddNode(pTree, 3060, 3050, "Nested checked option");
	xgeXuiTreeViewSetNodeExpanded(pTree, 3000, 1);
	xgeXuiTreeViewSetNodeExpanded(pTree, 3050, 1);
	xgeXuiTreeViewSetNodeChecked(pTree, 3010, 1);
	xgeXuiTreeViewSetNodeDecorations(pTree, 3020, 1, 1);
	xgeXuiTreeViewSetNodeDecorations(pTree, 3030, 1, 1);
	xgeXuiTreeViewSetNodeEnabled(pTree, 3030, 0);
	xgeXuiTreeViewSetNodeDecorations(pTree, 3040, 0, 0);
	xgeXuiTreeViewSetNodeChecked(pTree, 3060, 1);
	xgeXuiTreeViewSetSelected(pTree, 3010);
}

static void PopulateCustomTree(app_state_t* pApp)
{
	xge_xui_tree_view pTree;

	pTree = &pApp->tTree[TREE_CUSTOM];
	xgeXuiTreeViewSetColors(pTree, XGE_COLOR_RGBA(248, 250, 253, 255), XGE_COLOR_RGBA(246, 249, 252, 255), XGE_COLOR_RGBA(205, 226, 248, 255), XGE_COLOR_RGBA(34, 48, 64, 255), XGE_COLOR_RGBA(232, 238, 247, 255), XGE_COLOR_RGBA(67, 151, 116, 240));
	xgeXuiTreeViewSetItemRenderer(pTree, CustomTreeRenderer, pApp);
	xgeXuiTreeViewAddNode(pTree, 4000, -1, "Custom renderer");
	xgeXuiTreeViewAddNode(pTree, 4010, 4000, "Task collection");
	xgeXuiTreeViewAddNode(pTree, 4020, 4010, "Completed child");
	xgeXuiTreeViewAddNode(pTree, 4030, 4010, "Disabled custom row");
	xgeXuiTreeViewAddNode(pTree, 4040, 4000, "Plain child");
	xgeXuiTreeViewSetNodeExpanded(pTree, 4000, 1);
	xgeXuiTreeViewSetNodeExpanded(pTree, 4010, 1);
	xgeXuiTreeViewSetNodeChecked(pTree, 4020, 1);
	xgeXuiTreeViewSetNodeEnabled(pTree, 4030, 0);
	xgeXuiTreeViewSetSelected(pTree, 4020);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_GRID);
	xgeXuiWidgetSetPaddingPx(pRoot, 18.0f, 18.0f, 18.0f, 18.0f);
	xgeXuiWidgetSetGap(pRoot, 12.0f);
	if ( AddPanel(pApp, pRoot, TREE_BASIC, "Basic tree: expand, selected, disabled, checked") != XGE_OK ||
	     AddPanel(pApp, pRoot, TREE_LARGE, "Large tree: full scrollbar, wheel and virtual rows") != XGE_OK ||
	     AddPanel(pApp, pRoot, TREE_CHECK, "Interaction states: checkbox, no icon, disabled") != XGE_OK ||
	     AddPanel(pApp, pRoot, TREE_CUSTOM, "Custom renderer: application-owned row paint") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( AddTree(pApp, TREE_BASIC, 24.0f) != XGE_OK ||
	     AddTree(pApp, TREE_LARGE, 22.0f) != XGE_OK ||
	     AddTree(pApp, TREE_CHECK, 26.0f) != XGE_OK ||
	     AddTree(pApp, TREE_CUSTOM, 27.0f) != XGE_OK ) {
		return XGE_ERROR;
	}
	PopulateBasicTree(pApp);
	PopulateLargeTree(pApp);
	PopulateCheckTree(pApp);
	PopulateCustomTree(pApp);
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fRootW;
	float fRootH;
	float fPanelH;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return;
	}
	fRootW = (float)iWidth;
	fRootH = (float)iHeight;
	if ( fRootW < 960.0f ) {
		fRootW = 960.0f;
	}
	if ( fRootH < 680.0f ) {
		fRootH = 680.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	fPanelH = (fRootH - 36.0f - 12.0f) * 0.5f;
	xgeXuiWidgetSetGrid(pRoot, 2, fPanelH, 12.0f, 12.0f);
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void MakeMouse(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void ClickPoint(app_state_t* pApp, float fX, float fY)
{
	xge_event_t tEvent;

	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, fX, fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
}

static xge_xui_tree_view_node_t* FindNode(xge_xui_tree_view pTree, int iId)
{
	int i;

	for ( i = 0; i < pTree->iNodeCount; i++ ) {
		if ( pTree->arrNodes[i].iId == iId ) {
			return &pTree->arrNodes[i];
		}
	}
	return NULL;
}

static void RunChecks(app_state_t* pApp)
{
	xge_xui_tree_view_node_t* pNode;
	xge_event_t tEvent;
	float fX;
	float fY;

	xgeXuiPaint(&pApp->tXui);
	pApp->bCreateOK =
		(pApp->pTreeWidget[TREE_BASIC] != NULL) &&
		(pApp->pTreeWidget[TREE_BASIC]->procEvent == xgeXuiTreeViewEventProc) &&
		(pApp->tTree[TREE_BASIC].iNodeCount == 7) &&
		(pApp->tTree[TREE_LARGE].iNodeCount > 120);
	pApp->bLayoutOK =
		(pApp->pTreeWidget[TREE_BASIC]->tRect.fW > 320.0f) &&
		(pApp->pTreeWidget[TREE_CUSTOM]->tRect.fH > 180.0f);
	if ( pApp->bRanInteraction == 0 ) {
		pApp->bStateOK =
			(xgeXuiTreeViewGetVisibleCount(&pApp->tTree[TREE_BASIC]) == 6) &&
			(xgeXuiTreeViewGetVisibleNodeId(&pApp->tTree[TREE_BASIC], 2) == 30) &&
			(xgeXuiTreeViewGetSelected(&pApp->tTree[TREE_BASIC]) == 30) &&
			(xgeXuiTreeViewGetNodeEnabled(&pApp->tTree[TREE_BASIC], 40) == 0) &&
			(xgeXuiTreeViewGetNodeChecked(&pApp->tTree[TREE_BASIC], 30) == 1);
	}

	if ( pApp->bRanInteraction == 0 ) {
		pNode = FindNode(&pApp->tTree[TREE_BASIC], 30);
		if ( pNode != NULL ) {
			ClickPoint(pApp, pNode->tRect.fX + 92.0f, pNode->tRect.fY + 12.0f);
		}
		memset(&tEvent, 0, sizeof(tEvent));
		tEvent.iType = XGE_EVENT_KEY_DOWN;
		tEvent.iParam1 = XGE_KEY_DOWN;
		xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
		pNode = FindNode(&pApp->tTree[TREE_CHECK], 3020);
		if ( pNode != NULL ) {
			fX = pNode->tRect.fX + 6.0f + (float)pNode->iDepth * 18.0f + 22.0f;
			fY = pNode->tRect.fY + 13.0f;
			ClickPoint(pApp, fX, fY);
		}
		pNode = FindNode(&pApp->tTree[TREE_CHECK], 3050);
		if ( pNode != NULL ) {
			fX = pNode->tRect.fX + 6.0f + (float)pNode->iDepth * 18.0f + 6.0f;
			fY = pNode->tRect.fY + 13.0f;
			ClickPoint(pApp, fX, fY);
		}
		pApp->bRanInteraction = 1;
	}
	pApp->bInteractionOK =
		(pApp->iLastSelected[TREE_BASIC] == 50) &&
		(pApp->iSelectCount[TREE_BASIC] >= 1) &&
		(xgeXuiTreeViewGetSelected(&pApp->tTree[TREE_BASIC]) == 50) &&
		(xgeXuiTreeViewGetNodeChecked(&pApp->tTree[TREE_CHECK], 3020) == 1) &&
		(xgeXuiTreeViewGetNodeExpanded(&pApp->tTree[TREE_CHECK], 3050) == 0);
	pApp->bScrollOK =
		(xgeXuiTreeViewGetScrollbarMode(&pApp->tTree[TREE_LARGE]) == XGE_XUI_SCROLLBAR_MODE_FULL) &&
		(xgeXuiTreeViewGetScroll(&pApp->tTree[TREE_LARGE]) > 0.0f) &&
		(xgeXuiTreeViewGetFirstVisible(&pApp->tTree[TREE_LARGE]) > 0) &&
		(xgeXuiTreeViewGetPaintVisibleCount(&pApp->tTree[TREE_LARGE]) > 0);
	pApp->bCustomOK = (pApp->iCustomPaintCount > 0) && (xgeXuiTreeViewGetSelected(&pApp->tTree[TREE_CUSTOM]) == 4020);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunChecks(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	return XGE_OK;
}

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return XGE_OK;
	}
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_treeview final-summary frames=%d create=%d layout=%d state=%d interaction=%d scroll=%d custom=%d selected=%d largeFirst=%d customPaint=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bStateOK,
			pApp->bInteractionOK,
			pApp->bScrollOK,
			pApp->bCustomOK,
			xgeXuiTreeViewGetSelected(&pApp->tTree[TREE_BASIC]),
			xgeXuiTreeViewGetFirstVisible(&pApp->tTree[TREE_LARGE]),
			pApp->iCustomPaintCount);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(236, 240, 246, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	static app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TREEVIEW_FRAMES"), 0);
	for ( i = 0; i < TREE_COUNT; i++ ) {
		tApp.iLastSelected[i] = -1;
	}
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 980;
	tDesc.iHeight = 720;
	tDesc.sTitle = "XUI TreeView";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	tApp.tScene.pUser = &tApp;
	tApp.tScene.onEnter = AppEnter;
	tApp.tScene.onLeave = AppLeave;
	tApp.tScene.onEvent = AppEvent;
	tApp.tScene.onUpdate = AppUpdate;
	tApp.tScene.onDraw = AppDraw;
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	iExitCode = xgeRun(NULL, NULL);
	xgeUnit();
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK && tApp.bInteractionOK && tApp.bScrollOK && tApp.bCustomOK) ? 0 : 3;
}
