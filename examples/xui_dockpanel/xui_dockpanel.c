#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DOCK_WINDOW_COUNT 5
#define CONTENT_COUNT 5
#define LABEL_COUNT 17

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_dock_layout_t tDockLayout;
	xge_xui_dock_window_t arrDockWindow[DOCK_WINDOW_COUNT];
	xge_xui_widget arrContent[CONTENT_COUNT];
	xge_xui_label_t arrLabel[LABEL_COUNT];
	xge_xui_widget pDockLayoutWidget;
	xge_xui_dock_pane pDocumentPane;
	xge_xui_dock_pane pToolPane;
	xge_xui_dock_pane pPropertyPane;
	xge_xui_dock_pane pPreviewPane;
	int iContentCount;
	int iLabelCount;
	int bFontReady;
	int bDockLayoutInit;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	int bDragPreview;
	int bFloatDockPreview;
	int bGlobalDockPreview;
	int bTabReorderPreview;
	int bTabFloatPreview;
	int bSplitterPreview;
	int bClosePreview;
	int bTooltipPreview;
	int bOptionMenuPreview;
	int bOverflowMenuPreview;
	int bCtrlDragPreview;
	int bAutoHidePreview;
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
	const char* arrFonts[] = { "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/Deng.ttf", "C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/arial.ttf" };
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_dockpanel_lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_xui_widget NewContent(app_state_t* pApp)
{
	xge_xui_widget pWidget;

	if ( pApp->iContentCount >= CONTENT_COUNT ) {
		return NULL;
	}
	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return NULL;
	}
	xgeXuiWidgetSetLayout(pWidget, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetGap(pWidget, 4.0f);
	xgeXuiWidgetSetPaddingPx(pWidget, 10.0f, 9.0f, 10.0f, 9.0f);
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(255, 255, 255, 255));
	pApp->arrContent[pApp->iContentCount++] = pWidget;
	return pWidget;
}

static int AddLabel(app_state_t* pApp, xge_xui_widget pParent, const char* sText, uint32_t iColor)
{
	xge_xui_widget pWidget;
	xge_xui_label pLabel;

	if ( (pApp == NULL) || (pParent == NULL) || (pApp->iLabelCount >= LABEL_COUNT) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(23.0f));
	pLabel = &pApp->arrLabel[pApp->iLabelCount++];
	if ( xgeXuiLabelInit(pLabel, pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(pLabel, iColor);
	xgeXuiLabelSetAlign(pLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	return xgeXuiWidgetAdd(pParent, pWidget);
}

static int BuildDockWindow(app_state_t* pApp, int iIndex, const char* sTitle, int bClosable)
{
	xge_xui_widget pContent;

	if ( (pApp == NULL) || (iIndex < 0) || (iIndex >= DOCK_WINDOW_COUNT) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiDockWindowInit(&pApp->arrDockWindow[iIndex], &pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiDockWindowSetTitle(&pApp->arrDockWindow[iIndex], sTitle);
	xgeXuiDockWindowSetClosable(&pApp->arrDockWindow[iIndex], bClosable);
	pContent = NewContent(pApp);
	if ( pContent == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiDockWindowSetClientWidget(&pApp->arrDockWindow[iIndex], pContent);
	return XGE_OK;
}

static int CreateDockContent(app_state_t* pApp)
{
	xge_xui_widget pContent;

	pContent = pApp->arrContent[0];
	AddLabel(pApp, pContent, "int main(void)", XGE_COLOR_RGBA(42, 72, 108, 255));
	AddLabel(pApp, pContent, "{", XGE_COLOR_RGBA(88, 92, 98, 255));
	AddLabel(pApp, pContent, "    dock_layout_update(&layout);", XGE_COLOR_RGBA(26, 112, 172, 255));
	AddLabel(pApp, pContent, "    return XGE_OK;", XGE_COLOR_RGBA(26, 112, 172, 255));
	AddLabel(pApp, pContent, "}", XGE_COLOR_RGBA(88, 92, 98, 255));

	pContent = pApp->arrContent[1];
	AddLabel(pApp, pContent, "build: xge.dll", XGE_COLOR_RGBA(56, 68, 80, 255));
	AddLabel(pApp, pContent, "smoke: dock split tree passed", XGE_COLOR_RGBA(42, 128, 72, 255));
	AddLabel(pApp, pContent, "paint: pane tabs and buttons", XGE_COLOR_RGBA(56, 68, 80, 255));

	pContent = pApp->arrContent[2];
	AddLabel(pApp, pContent, "Solution", XGE_COLOR_RGBA(32, 72, 112, 255));
	AddLabel(pApp, pContent, "  src", XGE_COLOR_RGBA(56, 68, 80, 255));
	AddLabel(pApp, pContent, "  examples", XGE_COLOR_RGBA(56, 68, 80, 255));

	pContent = pApp->arrContent[3];
	AddLabel(pApp, pContent, "Name        DockPanel", XGE_COLOR_RGBA(56, 68, 80, 255));
	AddLabel(pApp, pContent, "Region      Document", XGE_COLOR_RGBA(56, 68, 80, 255));
	AddLabel(pApp, pContent, "State       Docked", XGE_COLOR_RGBA(42, 128, 72, 255));

	pContent = pApp->arrContent[4];
	AddLabel(pApp, pContent, "Preview viewport", XGE_COLOR_RGBA(32, 72, 112, 255));
	AddLabel(pApp, pContent, "indicator atlas: ready", XGE_COLOR_RGBA(42, 128, 72, 255));
	AddLabel(pApp, pContent, "bottom split: docked", XGE_COLOR_RGBA(56, 68, 80, 255));

	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	pRoot = xgeXuiRoot(&pApp->tXui);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pRoot, XGE_COLOR_RGBA(226, 235, 242, 255));
	pApp->pDockLayoutWidget = xgeXuiWidgetCreate();
	if ( pApp->pDockLayoutWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pDockLayoutWidget, (xge_rect_t){ 14.0f, 14.0f, 996.0f, 604.0f });
	if ( xgeXuiWidgetAdd(pRoot, pApp->pDockLayoutWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiDockLayoutInit(&pApp->tDockLayout, &pApp->tXui, pApp->pDockLayoutWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->bDockLayoutInit = 1;
	if ( BuildDockWindow(pApp, 0, "Document.c", 0) != XGE_OK ||
		BuildDockWindow(pApp, 1, "Output", 1) != XGE_OK ||
		BuildDockWindow(pApp, 2, "Toolbox", 1) != XGE_OK ||
		BuildDockWindow(pApp, 3, "Properties", 1) != XGE_OK ||
		BuildDockWindow(pApp, 4, "Preview", 1) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( CreateDockContent(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->pDocumentPane = xgeXuiDockLayoutDockWindow(&pApp->tDockLayout, &pApp->arrDockWindow[0], XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f);
	if ( pApp->pDocumentPane == NULL ) {
		return XGE_ERROR;
	}
	if ( xgeXuiDockLayoutDockWindow(&pApp->tDockLayout, &pApp->arrDockWindow[1], XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f) != pApp->pDocumentPane ) {
		return XGE_ERROR;
	}
	pApp->pToolPane = xgeXuiDockLayoutDockWindow(&pApp->tDockLayout, &pApp->arrDockWindow[2], XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_LEFT, 0.24f);
	pApp->pPropertyPane = xgeXuiDockLayoutDockWindow(&pApp->tDockLayout, &pApp->arrDockWindow[3], XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_RIGHT, 0.28f);
	pApp->pPreviewPane = xgeXuiDockLayoutDockWindow(&pApp->tDockLayout, &pApp->arrDockWindow[4], XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_BOTTOM, 0.26f);
	if ( (pApp->pToolPane == NULL) || (pApp->pPropertyPane == NULL) || (pApp->pPreviewPane == NULL) ) {
		return XGE_ERROR;
	}
	xgeXuiDockPaneSetActiveIndex(pApp->pDocumentPane, 0);
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fRootW;
	float fRootH;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	fRootW = (float)iWidth;
	fRootH = (float)iHeight;
	if ( fRootW < 1024.0f ) {
		fRootW = 1024.0f;
	}
	if ( fRootH < 640.0f ) {
		fRootH = 640.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	if ( pApp->pDockLayoutWidget != NULL ) {
		xgeXuiWidgetSetRect(pApp->pDockLayoutWidget, (xge_rect_t){ 14.0f, 14.0f, fRootW - 28.0f, fRootH - 28.0f });
	}
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	int iExpectedDocumentTabs;
	int bFloatingPreview;
	int bClosePreview;
	int bAutoHidePreview;
	int bPropertyLayoutOK;
	int bPropertyStateOK;

	bFloatingPreview = (pApp->bFloatDockPreview != 0) || (pApp->bGlobalDockPreview != 0);
	bClosePreview = (pApp->bClosePreview != 0);
	bAutoHidePreview = (pApp->bAutoHidePreview != 0);
	iExpectedDocumentTabs = ((bFloatingPreview != 0) || (bClosePreview != 0)) ? 1 : 2;
	bPropertyLayoutOK = (bAutoHidePreview != 0) ?
		((pApp->tDockLayout.pAutoHideExpandWindow == &pApp->arrDockWindow[3]) && (pApp->tDockLayout.tAutoHideExpandClientRect.fW > 60.0f) && (pApp->tDockLayout.tAutoHideExpandClientRect.fH > 40.0f)) :
		((pApp->pPropertyPane != NULL) && (pApp->pPropertyPane->tClientRect.fW > 60.0f));
	bPropertyStateOK = (bAutoHidePreview != 0) ?
		((xgeXuiDockWindowGetState(&pApp->arrDockWindow[3]) == XGE_XUI_DOCK_WINDOW_AUTO_HIDE) && (pApp->arrDockWindow[3].pClientWidget->pParent == pApp->tDockLayout.pAutoHideOverlayWidget)) :
		(pApp->arrDockWindow[3].pPane == pApp->pPropertyPane);
	pApp->bCreateOK = (pApp->iContentCount == CONTENT_COUNT) &&
		(pApp->iLabelCount == LABEL_COUNT) &&
		(xgeXuiDockPaneGetWindowCount(pApp->pDocumentPane) == iExpectedDocumentTabs) &&
		(xgeXuiDockPaneGetWindowCount(pApp->pPreviewPane) == 1) &&
		(xgeXuiDockWindowGetState(&pApp->arrDockWindow[0]) == XGE_XUI_DOCK_WINDOW_DOCKED);
	pApp->bLayoutOK = (pApp->pDocumentPane != NULL) &&
		(pApp->pToolPane != NULL) &&
		(pApp->pPreviewPane != NULL) &&
		(pApp->pDocumentPane->tClientRect.fW > 120.0f) &&
		(pApp->pToolPane->tClientRect.fW > 60.0f) &&
		(bPropertyLayoutOK != 0) &&
		(pApp->pPreviewPane->tClientRect.fH > 40.0f);
	pApp->bStateOK = (pApp->arrDockWindow[0].pClientWidget->pParent == pApp->pDockLayoutWidget) &&
		(pApp->arrDockWindow[2].pPane == pApp->pToolPane) &&
		(bPropertyStateOK != 0) &&
		(pApp->arrDockWindow[4].pPane == pApp->pPreviewPane) &&
		((bClosePreview == 0) || (xgeXuiDockWindowGetState(&pApp->arrDockWindow[1]) == XGE_XUI_DOCK_WINDOW_HIDDEN && xgeXuiDockPaneGetActiveWindow(pApp->pDocumentPane) == &pApp->arrDockWindow[0])) &&
		((bFloatingPreview == 0) || (xgeXuiDockWindowGetState(&pApp->arrDockWindow[1]) == XGE_XUI_DOCK_WINDOW_FLOATING)) &&
		(pApp->tDockLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot != NULL);
}

static int StartDragPreview(app_state_t* pApp)
{
	xge_event_t tEvent;
	float fStartX;
	float fStartY;
	float fEndX;
	float fEndY;

	if ( (pApp == NULL) || (pApp->pDocumentPane == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fStartX = pApp->pDocumentPane->tTabStripRect.fX + 12.0f;
	fStartY = pApp->pDocumentPane->tTabStripRect.fY + 10.0f;
	fEndX = pApp->pDocumentPane->tRect.fX + pApp->pDocumentPane->tRect.fW - 10.0f;
	fEndY = pApp->pDocumentPane->tRect.fY + pApp->pDocumentPane->tRect.fH * 0.50f;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 11;
	tEvent.fX = fStartX;
	tEvent.fY = fStartY;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = fEndX;
	tEvent.fY = fEndY;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	return (pApp->tDockLayout.iDragPhase == XGE_XUI_DOCK_DRAG_DRAGGING && pApp->tDockLayout.pDragOverlayWidget != NULL) ? XGE_OK : XGE_ERROR;
}

static int StartTabReorderPreview(app_state_t* pApp)
{
	xge_event_t tEvent;
	float fStartX;
	float fStartY;
	float fEndX;
	float fEndY;

	if ( (pApp == NULL) || (pApp->pDocumentPane == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fStartX = pApp->pDocumentPane->tTabStripRect.fX + 90.0f;
	fStartY = pApp->pDocumentPane->tTabStripRect.fY + 10.0f;
	fEndX = pApp->pDocumentPane->tTabStripRect.fX + 8.0f;
	fEndY = pApp->pDocumentPane->tTabStripRect.fY + 10.0f;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 14;
	tEvent.fX = fStartX;
	tEvent.fY = fStartY;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = fEndX;
	tEvent.fY = fEndY;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	return (pApp->tDockLayout.iDragPhase == XGE_XUI_DOCK_DRAG_DRAGGING && pApp->tDockLayout.pHoverPane == pApp->pDocumentPane && pApp->tDockLayout.iHoverTabIndex == 0 && pApp->tDockLayout.pDragOverlayWidget != NULL) ? XGE_OK : XGE_ERROR;
}

static int StartTabFloatPreview(app_state_t* pApp)
{
	xge_event_t tEvent;
	float fStartX;
	float fStartY;
	float fEndX;
	float fEndY;

	if ( (pApp == NULL) || (pApp->pDocumentPane == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fStartX = pApp->pDocumentPane->tTabStripRect.fX + 90.0f;
	fStartY = pApp->pDocumentPane->tTabStripRect.fY + 10.0f;
	fEndX = pApp->pDockLayoutWidget->tContentRect.fX - 90.0f;
	fEndY = pApp->pDocumentPane->tTabStripRect.fY + 32.0f;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 15;
	tEvent.fX = fStartX;
	tEvent.fY = fStartY;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = fEndX;
	tEvent.fY = fEndY;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	return (pApp->tDockLayout.iDragPhase == XGE_XUI_DOCK_DRAG_DRAGGING && pApp->tDockLayout.iHoverSide == XGE_XUI_DOCK_SIDE_NONE && pApp->tDockLayout.pHoverPane == NULL && pApp->tDockLayout.tPreviewRect.fW > 0.0f && pApp->tDockLayout.pDragOverlayWidget != NULL) ? XGE_OK : XGE_ERROR;
}

static int StartCtrlDragPreview(app_state_t* pApp)
{
	xge_event_t tEvent;
	float fStartX;
	float fStartY;
	float fEndX;
	float fEndY;

	if ( (pApp == NULL) || (pApp->pDocumentPane == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiDockPaneSetActiveIndex(pApp->pDocumentPane, 1);
	if ( xgeXuiUpdate(&pApp->tXui, 0.0f) != XGE_OK ) {
		return XGE_ERROR;
	}
	fStartX = pApp->pDocumentPane->tTabStripRect.fX + 90.0f;
	fStartY = pApp->pDocumentPane->tTabStripRect.fY + 10.0f;
	fEndX = pApp->pDocumentPane->tRect.fX + pApp->pDocumentPane->tRect.fW - 10.0f;
	fEndY = pApp->pDocumentPane->tRect.fY + pApp->pDocumentPane->tRect.fH * 0.50f;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 9;
	tEvent.fX = fStartX;
	tEvent.fY = fStartY;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.iParam2 = XGE_KEY_MOD_CTRL;
	tEvent.fX = fEndX;
	tEvent.fY = fEndY;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	return (pApp->tDockLayout.iDragPhase == XGE_XUI_DOCK_DRAG_DRAGGING && pApp->tDockLayout.iHoverSide == XGE_XUI_DOCK_SIDE_NONE && pApp->tDockLayout.pHoverPane == NULL && pApp->tDockLayout.pHoverRegion == NULL && pApp->tDockLayout.tIndicatorRect.fW <= 0.0f && pApp->tDockLayout.tPreviewRect.fW > 0.0f && pApp->tDockLayout.pDragOverlayWidget != NULL) ? XGE_OK : XGE_ERROR;
}

static int StartSplitterPreview(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_xui_dock_node pSplit;
	float fStartX;
	float fStartY;
	float fStartRatio;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pSplit = pApp->tDockLayout.arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].pRoot;
	if ( (pSplit == NULL) || (pSplit->iType != XGE_XUI_DOCK_NODE_SPLIT) || (pSplit->tSplitterRect.fW <= 0.0f) || (pSplit->tSplitterRect.fH <= 0.0f) ) {
		return XGE_ERROR;
	}
	fStartRatio = pSplit->fRatio;
	fStartX = pSplit->tSplitterRect.fX + pSplit->tSplitterRect.fW * 0.50f;
	fStartY = pSplit->tSplitterRect.fY + pSplit->tSplitterRect.fH * 0.50f;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 16;
	tEvent.fX = fStartX;
	tEvent.fY = fStartY;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || pApp->tDockLayout.pSplitterDragNode != pSplit ) {
		return XGE_ERROR;
	}
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	if ( pSplit->iAxis == XGE_XUI_ORIENTATION_HORIZONTAL ) {
		tEvent.fY += 70.0f;
	} else {
		tEvent.fX += 70.0f;
	}
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || pSplit->fRatio == fStartRatio ) {
		return XGE_ERROR;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || pApp->tDockLayout.pSplitterDragNode != NULL ) {
		return XGE_ERROR;
	}
	return xgeXuiUpdate(&pApp->tXui, 0.0f);
}

static int StartClosePreview(app_state_t* pApp)
{
	xge_event_t tEvent;
	float fCloseX;
	float fCloseY;

	if ( (pApp == NULL) || (pApp->pDocumentPane == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiDockPaneSetActiveIndex(pApp->pDocumentPane, 1);
	if ( xgeXuiUpdate(&pApp->tXui, 0.0f) != XGE_OK || pApp->pDocumentPane->tCloseRect.fW <= 0.0f || xgeXuiDockPaneGetActiveWindow(pApp->pDocumentPane) != &pApp->arrDockWindow[1] ) {
		return XGE_ERROR;
	}
	fCloseX = pApp->pDocumentPane->tCloseRect.fX + pApp->pDocumentPane->tCloseRect.fW * 0.50f;
	fCloseY = pApp->pDocumentPane->tCloseRect.fY + pApp->pDocumentPane->tCloseRect.fH * 0.50f;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 17;
	tEvent.fX = fCloseX;
	tEvent.fY = fCloseY;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	if ( xgeXuiUpdate(&pApp->tXui, 0.0f) != XGE_OK ) {
		return XGE_ERROR;
	}
	return (xgeXuiDockWindowGetState(&pApp->arrDockWindow[1]) == XGE_XUI_DOCK_WINDOW_HIDDEN && xgeXuiDockPaneGetWindowCount(pApp->pDocumentPane) == 1 && xgeXuiDockPaneGetActiveWindow(pApp->pDocumentPane) == &pApp->arrDockWindow[0]) ? XGE_OK : XGE_ERROR;
}

static int StartTooltipPreview(app_state_t* pApp)
{
	xge_event_t tEvent;

	if ( (pApp == NULL) || (pApp->pDocumentPane == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.iPointerId = 15;
	tEvent.fX = pApp->pDocumentPane->tTabStripRect.fX + 112.0f;
	tEvent.fY = pApp->pDocumentPane->tTabStripRect.fY + 10.0f;
	(void)xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	if ( xgeXuiUpdate(&pApp->tXui, 0.50f) != XGE_OK ) {
		return XGE_ERROR;
	}
	return (xgeXuiWidgetTooltipIsOpen(&pApp->tXui) && xgeXuiWidgetTooltipGetOwner(&pApp->tXui) == pApp->pDockLayoutWidget) ? XGE_OK : XGE_ERROR;
}

static int StartOptionMenuPreview(app_state_t* pApp)
{
	xge_event_t tEvent;

	if ( (pApp == NULL) || (pApp->pDocumentPane == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiDockPaneSetActiveIndex(pApp->pDocumentPane, 1);
	if ( xgeXuiUpdate(&pApp->tXui, 0.0f) != XGE_OK || pApp->pDocumentPane->tOptionRect.fW <= 0.0f ) {
		return XGE_ERROR;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 18;
	tEvent.fX = pApp->pDocumentPane->tOptionRect.fX + pApp->pDocumentPane->tOptionRect.fW * 0.50f;
	tEvent.fY = pApp->pDocumentPane->tOptionRect.fY + pApp->pDocumentPane->tOptionRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	return (xgeXuiMenuIsOpen(&pApp->tDockLayout.tOptionMenu) && pApp->tDockLayout.tOptionMenu.iItemCount >= 6) ? XGE_OK : XGE_ERROR;
}

static int StartOverflowMenuPreview(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_xui_dock_node pNode;
	xge_xui_dock_node pParent;

	if ( (pApp == NULL) || (pApp->pDocumentPane == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( pNode = pApp->pDocumentPane->pNode; pNode != NULL && pNode->pParent != NULL; pNode = pParent ) {
		pParent = pNode->pParent;
		if ( pParent->iAxis == XGE_XUI_ORIENTATION_VERTICAL ) {
			pParent->fRatio = (pParent->pFirst == pNode) ? 0.16f : 0.84f;
		}
	}
	xgeXuiDockPaneSetActiveIndex(pApp->pDocumentPane, 1);
	if ( xgeXuiUpdate(&pApp->tXui, 0.0f) != XGE_OK || pApp->pDocumentPane->tOverflowRect.fW <= 0.0f ) {
		return XGE_ERROR;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 10;
	tEvent.fX = pApp->pDocumentPane->tOverflowRect.fX + pApp->pDocumentPane->tOverflowRect.fW * 0.50f;
	tEvent.fY = pApp->pDocumentPane->tOverflowRect.fY + pApp->pDocumentPane->tOverflowRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	return (xgeXuiMenuIsOpen(&pApp->tDockLayout.tOverflowMenu) && pApp->tDockLayout.tOverflowMenu.iItemCount == 2) ? XGE_OK : XGE_ERROR;
}

static int StartFloatingDockPreview(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tFloatRect;
	float fEndX;
	float fEndY;

	if ( (pApp == NULL) || (pApp->pDocumentPane == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tFloatRect = (xge_rect_t){ 360.0f, 72.0f, 260.0f, 180.0f };
	if ( xgeXuiDockLayoutFloatWindow(&pApp->tDockLayout, &pApp->arrDockWindow[1], tFloatRect) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiUpdate(&pApp->tXui, 0.0f) != XGE_OK ) {
		return XGE_ERROR;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 12;
	tEvent.fX = pApp->arrDockWindow[1].pWindowWidget->tRect.fX + 42.0f;
	tEvent.fY = pApp->arrDockWindow[1].pWindowWidget->tRect.fY + 10.0f;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	fEndX = pApp->pDocumentPane->tRect.fX + pApp->pDocumentPane->tRect.fW * 0.50f;
	fEndY = pApp->pDocumentPane->tRect.fY + pApp->pDocumentPane->tRect.fH * 0.50f;
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = fEndX;
	tEvent.fY = fEndY;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	return (pApp->tDockLayout.iDragPhase == XGE_XUI_DOCK_DRAG_DRAGGING && pApp->tDockLayout.pHoverPane == pApp->pDocumentPane && pApp->tDockLayout.pDragOverlayWidget != NULL) ? XGE_OK : XGE_ERROR;
}

static int StartGlobalDockPreview(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tFloatRect;
	float fEndX;
	float fEndY;

	if ( (pApp == NULL) || (pApp->pDockLayoutWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tFloatRect = (xge_rect_t){ 360.0f, 72.0f, 260.0f, 180.0f };
	if ( xgeXuiDockLayoutFloatWindow(&pApp->tDockLayout, &pApp->arrDockWindow[1], tFloatRect) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiUpdate(&pApp->tXui, 0.0f) != XGE_OK ) {
		return XGE_ERROR;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 13;
	tEvent.fX = pApp->arrDockWindow[1].pWindowWidget->tRect.fX + 42.0f;
	tEvent.fY = pApp->arrDockWindow[1].pWindowWidget->tRect.fY + 10.0f;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	fEndX = pApp->pDockLayoutWidget->tContentRect.fX + 8.0f;
	fEndY = pApp->pDockLayoutWidget->tContentRect.fY + pApp->pDockLayoutWidget->tContentRect.fH * 0.50f;
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX = fEndX;
	tEvent.fY = fEndY;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	return (pApp->tDockLayout.iDragPhase == XGE_XUI_DOCK_DRAG_DRAGGING && pApp->tDockLayout.pHoverRegion == &pApp->tDockLayout.arrRegions[XGE_XUI_DOCK_REGION_LEFT] && pApp->tDockLayout.pDragOverlayWidget != NULL) ? XGE_OK : XGE_ERROR;
}

static int StartAutoHidePreview(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_xui_dock_window pWindow;

	if ( (pApp == NULL) || (pApp->pPropertyPane == NULL) ) {
		return XGE_ERROR;
	}
	pWindow = &pApp->arrDockWindow[3];
	if ( xgeXuiUpdate(&pApp->tXui, 0.0f) != XGE_OK || pApp->pPropertyPane->tAutoHideRect.fW <= 0.0f ) {
		return XGE_ERROR;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 17;
	tEvent.fX = pApp->pPropertyPane->tAutoHideRect.fX + pApp->pPropertyPane->tAutoHideRect.fW * 0.50f;
	tEvent.fY = pApp->pPropertyPane->tAutoHideRect.fY + pApp->pPropertyPane->tAutoHideRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED || xgeXuiDockWindowGetState(pWindow) != XGE_XUI_DOCK_WINDOW_AUTO_HIDE ) {
		return XGE_ERROR;
	}
	pApp->pPropertyPane = NULL;
	if ( xgeXuiUpdate(&pApp->tXui, 0.0f) != XGE_OK || pWindow->tAutoHideStripRect.fW <= 0.0f ) {
		return XGE_ERROR;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.iPointerId = 18;
	tEvent.fX = pWindow->tAutoHideStripRect.fX + pWindow->tAutoHideStripRect.fW * 0.50f;
	tEvent.fY = pWindow->tAutoHideStripRect.fY + pWindow->tAutoHideStripRect.fH * 0.50f;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	if ( xgeXuiDispatchEvent(&pApp->tXui, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return XGE_ERROR;
	}
	return (pApp->tDockLayout.pAutoHideExpandWindow == pWindow && pApp->tDockLayout.tAutoHideExpandRect.fW > 0.0f && pWindow->pClientWidget->pParent == pApp->tDockLayout.pAutoHideOverlayWidget) ? XGE_OK : XGE_ERROR;
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
	if ( pApp->bAutoHidePreview != 0 && pApp->bOverflowMenuPreview == 0 && pApp->bClosePreview == 0 && pApp->bSplitterPreview == 0 && pApp->bTabFloatPreview == 0 && pApp->bTabReorderPreview == 0 && pApp->bGlobalDockPreview == 0 && pApp->bFloatDockPreview == 0 && pApp->bDragPreview == 0 && pApp->bTooltipPreview == 0 && pApp->bOptionMenuPreview == 0 && pApp->bCtrlDragPreview == 0 && StartAutoHidePreview(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( pApp->bOverflowMenuPreview != 0 && pApp->bClosePreview == 0 && pApp->bSplitterPreview == 0 && pApp->bTabFloatPreview == 0 && pApp->bTabReorderPreview == 0 && pApp->bGlobalDockPreview == 0 && pApp->bFloatDockPreview == 0 && pApp->bDragPreview == 0 && pApp->bTooltipPreview == 0 && pApp->bOptionMenuPreview == 0 && StartOverflowMenuPreview(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( pApp->bOptionMenuPreview != 0 && pApp->bClosePreview == 0 && pApp->bSplitterPreview == 0 && pApp->bTabFloatPreview == 0 && pApp->bTabReorderPreview == 0 && pApp->bGlobalDockPreview == 0 && pApp->bFloatDockPreview == 0 && pApp->bDragPreview == 0 && pApp->bTooltipPreview == 0 && pApp->bOverflowMenuPreview == 0 && StartOptionMenuPreview(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( pApp->bTooltipPreview != 0 && pApp->bClosePreview == 0 && pApp->bSplitterPreview == 0 && pApp->bTabFloatPreview == 0 && pApp->bTabReorderPreview == 0 && pApp->bGlobalDockPreview == 0 && pApp->bFloatDockPreview == 0 && pApp->bDragPreview == 0 && pApp->bOverflowMenuPreview == 0 && StartTooltipPreview(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( pApp->bClosePreview != 0 && StartClosePreview(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( pApp->bSplitterPreview != 0 && pApp->bClosePreview == 0 && StartSplitterPreview(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( pApp->bTabFloatPreview != 0 && pApp->bClosePreview == 0 && pApp->bSplitterPreview == 0 && StartTabFloatPreview(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( pApp->bCtrlDragPreview != 0 && pApp->bClosePreview == 0 && pApp->bSplitterPreview == 0 && pApp->bTabFloatPreview == 0 && pApp->bTabReorderPreview == 0 && pApp->bGlobalDockPreview == 0 && pApp->bFloatDockPreview == 0 && pApp->bDragPreview == 0 && pApp->bTooltipPreview == 0 && pApp->bOptionMenuPreview == 0 && pApp->bOverflowMenuPreview == 0 && StartCtrlDragPreview(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( pApp->bTabReorderPreview != 0 && pApp->bClosePreview == 0 && pApp->bSplitterPreview == 0 && pApp->bTabFloatPreview == 0 && StartTabReorderPreview(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( pApp->bGlobalDockPreview != 0 && pApp->bClosePreview == 0 && pApp->bSplitterPreview == 0 && pApp->bTabReorderPreview == 0 && pApp->bTabFloatPreview == 0 && StartGlobalDockPreview(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( pApp->bFloatDockPreview != 0 && pApp->bClosePreview == 0 && pApp->bSplitterPreview == 0 && pApp->bGlobalDockPreview == 0 && pApp->bTabReorderPreview == 0 && pApp->bTabFloatPreview == 0 && StartFloatingDockPreview(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( pApp->bDragPreview != 0 && pApp->bClosePreview == 0 && pApp->bSplitterPreview == 0 && pApp->bFloatDockPreview == 0 && pApp->bGlobalDockPreview == 0 && pApp->bTabReorderPreview == 0 && pApp->bTabFloatPreview == 0 && StartDragPreview(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pScene->pUser;
	for ( i = DOCK_WINDOW_COUNT - 1; i >= 0; i-- ) {
		xgeXuiDockWindowUnit(&pApp->arrDockWindow[i]);
	}
	for ( i = 0; i < pApp->iLabelCount; i++ ) {
		xgeXuiLabelUnit(&pApp->arrLabel[i]);
	}
	for ( i = 0; i < pApp->iContentCount; i++ ) {
		if ( pApp->arrContent[i] != NULL ) {
			xgeXuiWidgetFree(pApp->arrContent[i]);
		}
	}
	if ( pApp->bDockLayoutInit ) {
		xgeXuiDockLayoutUnit(&pApp->tDockLayout);
	}
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
		printf("xui_dockpanel_lab final-summary frames=%d create=%d layout=%d state=%d labels=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->iLabelCount);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(226, 235, 242, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_DOCKPANEL_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		} else if ( strcmp(argv[i], "--drag-preview") == 0 ) {
			tApp.bDragPreview = 1;
		} else if ( strcmp(argv[i], "--float-dock-preview") == 0 ) {
			tApp.bFloatDockPreview = 1;
		} else if ( strcmp(argv[i], "--global-dock-preview") == 0 ) {
			tApp.bGlobalDockPreview = 1;
		} else if ( strcmp(argv[i], "--tab-reorder-preview") == 0 ) {
			tApp.bTabReorderPreview = 1;
		} else if ( strcmp(argv[i], "--tab-float-preview") == 0 ) {
			tApp.bTabFloatPreview = 1;
		} else if ( strcmp(argv[i], "--ctrl-drag-preview") == 0 ) {
			tApp.bCtrlDragPreview = 1;
		} else if ( strcmp(argv[i], "--splitter-preview") == 0 ) {
			tApp.bSplitterPreview = 1;
		} else if ( strcmp(argv[i], "--close-preview") == 0 ) {
			tApp.bClosePreview = 1;
		} else if ( strcmp(argv[i], "--tooltip-preview") == 0 ) {
			tApp.bTooltipPreview = 1;
		} else if ( strcmp(argv[i], "--option-menu-preview") == 0 ) {
			tApp.bOptionMenuPreview = 1;
		} else if ( strcmp(argv[i], "--overflow-menu-preview") == 0 ) {
			tApp.bOverflowMenuPreview = 1;
		} else if ( strcmp(argv[i], "--auto-hide-preview") == 0 ) {
			tApp.bAutoHidePreview = 1;
		}
	}
	tDesc.iWidth = 1024;
	tDesc.iHeight = 640;
	tDesc.sTitle = "XUI DockPanel Lab";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK) ? 0 : 3;
}
