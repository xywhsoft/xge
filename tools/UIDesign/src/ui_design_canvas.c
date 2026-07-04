#include "ui_design_canvas.h"
#include "ui_design_inspector.h"

#include <stdio.h>
#include <string.h>

#define UI_DESIGN_CANVAS_MODE_NONE 0
#define UI_DESIGN_CANVAS_MODE_MOVE 1
#define UI_DESIGN_CANVAS_MODE_RESIZE 2
#define UI_DESIGN_CANVAS_MODE_CREATE 3
#define UI_DESIGN_CANVAS_MODE_MARQUEE 4

#define UI_DESIGN_PLACE_CLICK_THRESHOLD 4.0f
#define UI_DESIGN_PLACE_MIN_SIZE 12.0f

#define UI_DESIGN_CANVAS_MENU_COPY 1
#define UI_DESIGN_CANVAS_MENU_CUT 2
#define UI_DESIGN_CANVAS_MENU_PASTE 3
#define UI_DESIGN_CANVAS_MENU_DELETE 4

#define UI_DESIGN_HANDLE_NW 0
#define UI_DESIGN_HANDLE_N 1
#define UI_DESIGN_HANDLE_NE 2
#define UI_DESIGN_HANDLE_E 3
#define UI_DESIGN_HANDLE_SE 4
#define UI_DESIGN_HANDLE_S 5
#define UI_DESIGN_HANDLE_SW 6
#define UI_DESIGN_HANDLE_W 7

static float __uiDesignMinF(float a, float b)
{
	return a < b ? a : b;
}

static float __uiDesignMaxF(float a, float b)
{
	return a > b ? a : b;
}

static float __uiDesignAbsF(float a)
{
	return a < 0.0f ? -a : a;
}

static xui_rect_t __uiDesignNormalizeRect(float fX0, float fY0, float fX1, float fY1)
{
	xui_rect_t tRect;

	tRect.fX = __uiDesignMinF(fX0, fX1);
	tRect.fY = __uiDesignMinF(fY0, fY1);
	tRect.fW = __uiDesignAbsF(fX1 - fX0);
	tRect.fH = __uiDesignAbsF(fY1 - fY0);
	return tRect;
}

static int __uiDesignRectContains(xui_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX <= tRect.fX + tRect.fW) && (fY <= tRect.fY + tRect.fH);
}

static int __uiDesignRectContainsRect(xui_rect_t tOuter, xui_rect_t tInner)
{
	return (tInner.fX >= tOuter.fX) &&
	       (tInner.fY >= tOuter.fY) &&
	       (tInner.fX + tInner.fW <= tOuter.fX + tOuter.fW) &&
	       (tInner.fY + tInner.fH <= tOuter.fY + tOuter.fH);
}

static int __uiDesignRectIntersects(xui_rect_t a, xui_rect_t b)
{
	return (a.fX <= b.fX + b.fW) &&
	       (a.fX + a.fW >= b.fX) &&
	       (a.fY <= b.fY + b.fH) &&
	       (a.fY + a.fH >= b.fY);
}

static float __uiDesignSnapValue(float fValue)
{
	const float fGrid = 20.0f;

	if ( fValue >= 0.0f ) return (float)((int)((fValue + fGrid * 0.5f) / fGrid)) * fGrid;
	return (float)((int)((fValue - fGrid * 0.5f) / fGrid)) * fGrid;
}

static xui_rect_t __uiDesignSnapRect(ui_design_app_t* pApp, xui_rect_t tRect, int bSize)
{
	if ( (pApp == NULL) || !pApp->bSnapEnabled ) return tRect;
	tRect.fX = __uiDesignSnapValue(tRect.fX);
	tRect.fY = __uiDesignSnapValue(tRect.fY);
	if ( bSize ) {
		tRect.fW = __uiDesignMaxF(UI_DESIGN_PLACE_MIN_SIZE, __uiDesignSnapValue(tRect.fW));
		tRect.fH = __uiDesignMaxF(UI_DESIGN_PLACE_MIN_SIZE, __uiDesignSnapValue(tRect.fH));
	}
	return tRect;
}

static xui_rect_t __uiDesignHandleRect(xui_rect_t tRect, int iHandle)
{
	float x;
	float y;
	const float s = 7.0f;

	switch ( iHandle ) {
	case UI_DESIGN_HANDLE_NW: x = tRect.fX; y = tRect.fY; break;
	case UI_DESIGN_HANDLE_N: x = tRect.fX + tRect.fW * 0.5f; y = tRect.fY; break;
	case UI_DESIGN_HANDLE_NE: x = tRect.fX + tRect.fW; y = tRect.fY; break;
	case UI_DESIGN_HANDLE_E: x = tRect.fX + tRect.fW; y = tRect.fY + tRect.fH * 0.5f; break;
	case UI_DESIGN_HANDLE_SE: x = tRect.fX + tRect.fW; y = tRect.fY + tRect.fH; break;
	case UI_DESIGN_HANDLE_S: x = tRect.fX + tRect.fW * 0.5f; y = tRect.fY + tRect.fH; break;
	case UI_DESIGN_HANDLE_SW: x = tRect.fX; y = tRect.fY + tRect.fH; break;
	case UI_DESIGN_HANDLE_W: x = tRect.fX; y = tRect.fY + tRect.fH * 0.5f; break;
	default: x = tRect.fX; y = tRect.fY; break;
	}
	return (xui_rect_t){x - s * 0.5f, y - s * 0.5f, s, s};
}

static int __uiDesignHitHandle(xui_rect_t tRect, float fX, float fY)
{
	int i;

	for ( i = 0; i < 8; i++ ) {
		if ( __uiDesignRectContains(__uiDesignHandleRect(tRect, i), fX, fY) ) return i;
	}
	return -1;
}

static int __uiDesignCanvasCanFreeTransform(ui_design_app_t* pApp, const ui_design_node_t* pNode)
{
	if ( (pApp == NULL) || (pNode == NULL) ) return 0;
	return uiDesignModelCanFreeTransformNode(&pApp->tModel, pNode);
}

static void __uiDesignCanvasContextMenuSelect(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	ui_design_app_t* pApp;
	int iTarget;

	(void)pWidget;
	(void)iIndex;
	pApp = (ui_design_app_t*)pUser;
	if ( pApp == NULL ) return;
	iTarget = pApp->iContextMenuNodeId;
	switch ( iValue ) {
	case UI_DESIGN_CANVAS_MENU_COPY:
		if ( iTarget > 0 ) (void)uiDesignAppExecuteCommand(pApp, UI_DESIGN_COMMAND_EDIT_COPY);
		break;
	case UI_DESIGN_CANVAS_MENU_CUT:
		if ( iTarget > 0 ) (void)uiDesignAppExecuteCommand(pApp, UI_DESIGN_COMMAND_EDIT_CUT);
		break;
	case UI_DESIGN_CANVAS_MENU_PASTE:
		if ( pApp->bContextMenuHasDesignPoint ) {
			if ( uiDesignAppBeginHistoryTransaction(pApp, UI_DESIGN_COMMAND_EDIT_PASTE, "Paste") == XUI_OK ) {
				if ( uiDesignAppPasteClipboard(pApp, pApp->fContextMenuDesignX, pApp->fContextMenuDesignY, NULL) == XUI_OK ) {
					(void)uiDesignAppCommitHistoryTransaction(pApp);
				} else {
					uiDesignAppCancelHistoryTransaction(pApp);
				}
			}
		}
		break;
	case UI_DESIGN_CANVAS_MENU_DELETE:
		if ( iTarget > 0 ) (void)uiDesignAppExecuteCommand(pApp, UI_DESIGN_COMMAND_EDIT_DELETE);
		break;
	default:
		break;
	}
}

static int __uiDesignCanvasOpenContextMenu(ui_design_app_t* pApp, xui_widget pOwner, float fWorldX, float fWorldY, float fDesignX, float fDesignY, int iHit)
{
	xui_menu_item_t arrItems[5];
	uint32_t iNodeState;
	uint32_t iPasteState;

	if ( (pApp == NULL) || (pOwner == NULL) || (pApp->pCanvasContextMenu == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(arrItems, 0, sizeof(arrItems));
	iNodeState = (iHit > 0) ? XUI_MENU_ITEM_ENABLED : 0u;
	iPasteState = (pApp->iClipboardNodeCount > 0) ? XUI_MENU_ITEM_ENABLED : 0u;
	arrItems[0].sText = "Copy";
	arrItems[0].sShortcut = "Ctrl+C";
	arrItems[0].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[0].iState = iNodeState;
	arrItems[0].iValue = UI_DESIGN_CANVAS_MENU_COPY;
	arrItems[1].sText = "Cut";
	arrItems[1].sShortcut = "Ctrl+X";
	arrItems[1].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[1].iState = iNodeState;
	arrItems[1].iValue = UI_DESIGN_CANVAS_MENU_CUT;
	arrItems[2].sText = "Paste";
	arrItems[2].sShortcut = "Ctrl+V";
	arrItems[2].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[2].iState = iPasteState;
	arrItems[2].iValue = UI_DESIGN_CANVAS_MENU_PASTE;
	arrItems[3].iType = XUI_MENU_ITEM_SEPARATOR;
	arrItems[4].sText = "Delete";
	arrItems[4].sShortcut = "Del";
	arrItems[4].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[4].iState = iNodeState | XUI_MENU_ITEM_DANGER;
	arrItems[4].iValue = UI_DESIGN_CANVAS_MENU_DELETE;
	pApp->iContextMenuNodeId = iHit;
	pApp->fContextMenuDesignX = fDesignX;
	pApp->fContextMenuDesignY = fDesignY;
	pApp->bContextMenuHasDesignPoint = 1;
	(void)xuiMenuSetItems(pApp->pCanvasContextMenu, arrItems, 5);
	return xuiMenuOpenAt(pApp->pCanvasContextMenu, pOwner, fWorldX, fWorldY);
}

static uint32_t __uiDesignSelectionColor(ui_design_app_t* pApp, const ui_design_node_t* pNode)
{
	return __uiDesignCanvasCanFreeTransform(pApp, pNode) ?
		XUI_COLOR_RGBA(37, 124, 214, 255) :
		XUI_COLOR_RGBA(118, 136, 158, 255);
}

static int __uiDesignCanvasMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( pSize == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pSize = tConstraint;
	if ( pSize->fX <= 0.0f || pSize->fX >= XUI_LAYOUT_UNBOUNDED ) pSize->fX = 640.0f;
	if ( pSize->fY <= 0.0f || pSize->fY >= XUI_LAYOUT_UNBOUNDED ) pSize->fY = 420.0f;
	return XUI_OK;
}

static int __uiDesignCanvasArrange(xui_widget pWidget, xui_rect_t tContentRect, void* pUser)
{
	ui_design_app_t* pApp;
	xui_rect_t tArtboard;
	float fW;
	float fH;
	int iRet;

	(void)pWidget;
	pApp = (ui_design_app_t*)pUser;
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	fW = __uiDesignMinF(900.0f, tContentRect.fW - 56.0f);
	fH = __uiDesignMinF(600.0f, tContentRect.fH - 56.0f);
	if ( fW < 320.0f ) fW = tContentRect.fW - 28.0f;
	if ( fH < 240.0f ) fH = tContentRect.fH - 28.0f;
	if ( fW < 80.0f ) fW = 80.0f;
	if ( fH < 80.0f ) fH = 80.0f;
	tArtboard = (xui_rect_t){tContentRect.fX + (tContentRect.fW - fW) * 0.5f, tContentRect.fY + (tContentRect.fH - fH) * 0.5f, fW, fH};
	if ( pApp->pArtboard != NULL ) {
		iRet = xuiWidgetArrange(pApp->pArtboard, tArtboard);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pApp->pOverlay != NULL ) {
		iRet = xuiWidgetArrange(pApp->pOverlay, tArtboard);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __uiDesignCanvasRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	ui_design_app_t* pApp;
	xui_rect_t tRect;

	(void)iStateId;
	pApp = (ui_design_app_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pApp == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	return pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, tRect, XUI_COLOR_RGBA(236, 241, 247, 255));
}

static int __uiDesignArtboardRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	ui_design_app_t* pApp;
	xui_rect_t tRect;
	float x;
	float y;
	int iRet;

	(void)iStateId;
	pApp = (ui_design_app_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pApp == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	iRet = pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, tRect, XUI_COLOR_RGBA(255, 255, 255, 255));
	if ( iRet != XUI_OK ) return iRet;
	if ( pApp->bGridVisible ) {
		for ( x = 20.0f; x < tRect.fW; x += 20.0f ) {
			iRet = pApp->tProxy.drawLine(&pApp->tProxy, pDraw, x, 0.0f, x, tRect.fH, 1.0f, XUI_COLOR_RGBA(232, 237, 244, 255));
			if ( iRet != XUI_OK ) return iRet;
		}
		for ( y = 20.0f; y < tRect.fH; y += 20.0f ) {
			iRet = pApp->tProxy.drawLine(&pApp->tProxy, pDraw, 0.0f, y, tRect.fW, y, 1.0f, XUI_COLOR_RGBA(232, 237, 244, 255));
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	iRet = pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, tRect, 1.0f, XUI_COLOR_RGBA(132, 151, 174, 255));
	if ( iRet != XUI_OK ) return iRet;
	if ( pApp->tModel.iNodeCount == 0 ) {
		return pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, "Empty artboard", (xui_rect_t){0.0f, tRect.fH * 0.5f - 18.0f, tRect.fW, 36.0f}, XUI_COLOR_RGBA(118, 136, 158, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	return XUI_OK;
}

static int __uiDesignOverlayRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	ui_design_app_t* pApp;
	ui_design_node_t* pNode;
	xui_rect_t tRect;
	xui_rect_t tHandle;
	uint32_t iColor;
	int iSelectedId;
	int i;
	int iRet;

	(void)pWidget;
	(void)iStateId;
	pApp = (ui_design_app_t*)pUser;
	if ( (pApp == NULL) || (pDraw == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( ((pApp->iCanvasDragMode == UI_DESIGN_CANVAS_MODE_CREATE && pApp->iActiveTool != UI_DESIGN_NODE_NONE) ||
	      pApp->iCanvasDragMode == UI_DESIGN_CANVAS_MODE_MARQUEE) &&
	     pApp->tCanvasDragStartRect.fW > 0.5f &&
	     pApp->tCanvasDragStartRect.fH > 0.5f ) {
		tRect = pApp->tCanvasDragStartRect;
		iRet = pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, tRect, XUI_COLOR_RGBA(53, 131, 205, 36));
		if ( iRet != XUI_OK ) return iRet;
		iRet = pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, tRect, 1.0f, XUI_COLOR_RGBA(37, 124, 214, 210));
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( i = 0; i < uiDesignModelGetSelectionCount(&pApp->tModel); ++i ) {
		iSelectedId = uiDesignModelGetSelectionId(&pApp->tModel, i);
		pNode = uiDesignModelGetNode(&pApp->tModel, iSelectedId);
		if ( pNode == NULL ) continue;
		if ( uiDesignModelGetAbsoluteRect(&pApp->tModel, pNode->iId, &tRect) != XUI_OK ) continue;
		iColor = __uiDesignSelectionColor(pApp, pNode);
		iRet = pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, tRect, (pNode->iId == pApp->tModel.iSelectedId) ? 2.0f : 1.0f, iColor);
		if ( iRet != XUI_OK ) return iRet;
		if ( pNode->iId == pApp->tModel.iSelectedId && __uiDesignCanvasCanFreeTransform(pApp, pNode) ) {
			int j;
			for ( j = 0; j < 8; j++ ) {
				tHandle = __uiDesignHandleRect(tRect, j);
				iRet = pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, tHandle, XUI_COLOR_RGBA(255, 255, 255, 255));
				if ( iRet != XUI_OK ) return iRet;
				iRet = pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, tHandle, 1.0f, iColor);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	return XUI_OK;
}

int uiDesignCanvasWorldToDesign(ui_design_app_t* pApp, float fWorldX, float fWorldY, float* pDesignX, float* pDesignY)
{
	xui_rect_t tWorld;
	float fX;
	float fY;

	if ( (pApp == NULL) || (pApp->pArtboard == NULL) || (pDesignX == NULL) || (pDesignY == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tWorld = xuiWidgetGetWorldRect(pApp->pArtboard);
	fX = fWorldX - tWorld.fX;
	fY = fWorldY - tWorld.fY;
	if ( fX < 0.0f || fY < 0.0f || fX > tWorld.fW || fY > tWorld.fH ) return XUI_ERROR;
	*pDesignX = fX;
	*pDesignY = fY;
	return XUI_OK;
}

static int __uiDesignCanvasWorldToDesignClamped(ui_design_app_t* pApp, float fWorldX, float fWorldY, float* pDesignX, float* pDesignY)
{
	xui_rect_t tWorld;
	float fX;
	float fY;

	if ( (pApp == NULL) || (pApp->pArtboard == NULL) || (pDesignX == NULL) || (pDesignY == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tWorld = xuiWidgetGetWorldRect(pApp->pArtboard);
	fX = fWorldX - tWorld.fX;
	fY = fWorldY - tWorld.fY;
	if ( fX < 0.0f ) fX = 0.0f;
	if ( fY < 0.0f ) fY = 0.0f;
	if ( fX > tWorld.fW ) fX = tWorld.fW;
	if ( fY > tWorld.fH ) fY = tWorld.fH;
	*pDesignX = fX;
	*pDesignY = fY;
	return XUI_OK;
}

int uiDesignCanvasDropTool(ui_design_app_t* pApp, ui_design_node_type_t iType, float fWorldX, float fWorldY)
{
	float fDesignX;
	float fDesignY;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( uiDesignCanvasWorldToDesign(pApp, fWorldX, fWorldY, &fDesignX, &fDesignY) != XUI_OK ) return XUI_ERROR;
	return uiDesignAppAddNodeAt(pApp, iType, fDesignX, fDesignY, NULL);
}

int uiDesignCanvasPlaceToolRect(ui_design_app_t* pApp, ui_design_node_type_t iType, xui_rect_t tDesignRect, int* pId)
{
	ui_design_node_t* pNode;
	xui_rect_t tParentHost;
	xui_rect_t tLocal;
	int iId;
	int iRet;

	if ( (pApp == NULL) || (iType == UI_DESIGN_NODE_NONE) ) return XUI_ERROR_INVALID_ARGUMENT;
	tDesignRect = __uiDesignNormalizeRect(tDesignRect.fX, tDesignRect.fY, tDesignRect.fX + tDesignRect.fW, tDesignRect.fY + tDesignRect.fH);
	if ( tDesignRect.fW < UI_DESIGN_PLACE_MIN_SIZE ) tDesignRect.fW = UI_DESIGN_PLACE_MIN_SIZE;
	if ( tDesignRect.fH < UI_DESIGN_PLACE_MIN_SIZE ) tDesignRect.fH = UI_DESIGN_PLACE_MIN_SIZE;
	iId = 0;
	iRet = uiDesignAppAddNodeAt(pApp, iType, tDesignRect.fX + tDesignRect.fW * 0.5f, tDesignRect.fY + tDesignRect.fH * 0.5f, &iId);
	if ( iRet != XUI_OK ) return iRet;
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( uiDesignModelCanFreeTransformNode(&pApp->tModel, pNode) ) {
		tLocal = tDesignRect;
		if ( pNode->iParentId != 0 && uiDesignModelGetChildHostRect(&pApp->tModel, pNode->iParentId, &tParentHost) == XUI_OK ) {
			tLocal.fX -= tParentHost.fX;
			tLocal.fY -= tParentHost.fY;
			if ( tLocal.fX < 0.0f ) {
				tLocal.fW += tLocal.fX;
				tLocal.fX = 0.0f;
			}
			if ( tLocal.fY < 0.0f ) {
				tLocal.fH += tLocal.fY;
				tLocal.fY = 0.0f;
			}
			tLocal.fW = __uiDesignMaxF(UI_DESIGN_PLACE_MIN_SIZE, tLocal.fW);
			tLocal.fH = __uiDesignMaxF(UI_DESIGN_PLACE_MIN_SIZE, tLocal.fH);
		}
		iRet = uiDesignAppSetNodeRect(pApp, iId, tLocal);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pId != NULL ) *pId = iId;
	return XUI_OK;
}

int uiDesignCanvasSelectRect(ui_design_app_t* pApp, xui_rect_t tDesignRect)
{
	ui_design_node_t* pNode;
	xui_rect_t tNodeRect;
	char sStatus[64];
	int i;
	int iRet;
	int iCount;
	int bHit;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tDesignRect = __uiDesignNormalizeRect(tDesignRect.fX, tDesignRect.fY, tDesignRect.fX + tDesignRect.fW, tDesignRect.fY + tDesignRect.fH);
	pApp->iEditingProperty = UI_DESIGN_PROPERTY_NONE;
	pApp->iEditLength = 0;
	pApp->sEditBuffer[0] = '\0';
	iRet = uiDesignModelClearSelection(&pApp->tModel);
	if ( iRet != XUI_OK ) return iRet;
	iCount = 0;
	for ( i = 0; i < pApp->tModel.iNodeCount; ++i ) {
		pNode = &pApp->tModel.arrNodes[i];
		if ( !pNode->bVisible ) continue;
		if ( uiDesignModelGetAbsoluteRect(&pApp->tModel, pNode->iId, &tNodeRect) != XUI_OK ) continue;
		bHit = pApp->bMarqueeSelectContain ?
			__uiDesignRectContainsRect(tDesignRect, tNodeRect) :
			__uiDesignRectIntersects(tDesignRect, tNodeRect);
		if ( !bHit ) continue;
		iRet = uiDesignModelAddSelection(&pApp->tModel, pNode->iId);
		if ( iRet != XUI_OK ) return iRet;
		iCount++;
	}
	(void)uiDesignInspectorRefresh(pApp);
	snprintf(sStatus, sizeof(sStatus), "%d selected", iCount);
	uiDesignAppSetStatus(pApp, (iCount > 0) ? sStatus : "Ready");
	uiDesignAppUpdateCommandUI(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

static xui_rect_t __uiDesignResizeRect(xui_rect_t tStart, int iHandle, float fDX, float fDY)
{
	xui_rect_t tRect;
	float fRight;
	float fBottom;
	const float fMinW = 12.0f;
	const float fMinH = 12.0f;

	tRect = tStart;
	fRight = tStart.fX + tStart.fW;
	fBottom = tStart.fY + tStart.fH;
	switch ( iHandle ) {
	case UI_DESIGN_HANDLE_NW:
		tRect.fX = tStart.fX + fDX;
		tRect.fY = tStart.fY + fDY;
		tRect.fW = fRight - tRect.fX;
		tRect.fH = fBottom - tRect.fY;
		break;
	case UI_DESIGN_HANDLE_N:
		tRect.fY = tStart.fY + fDY;
		tRect.fH = fBottom - tRect.fY;
		break;
	case UI_DESIGN_HANDLE_NE:
		tRect.fY = tStart.fY + fDY;
		tRect.fW = tStart.fW + fDX;
		tRect.fH = fBottom - tRect.fY;
		break;
	case UI_DESIGN_HANDLE_E:
		tRect.fW = tStart.fW + fDX;
		break;
	case UI_DESIGN_HANDLE_SE:
		tRect.fW = tStart.fW + fDX;
		tRect.fH = tStart.fH + fDY;
		break;
	case UI_DESIGN_HANDLE_S:
		tRect.fH = tStart.fH + fDY;
		break;
	case UI_DESIGN_HANDLE_SW:
		tRect.fX = tStart.fX + fDX;
		tRect.fW = fRight - tRect.fX;
		tRect.fH = tStart.fH + fDY;
		break;
	case UI_DESIGN_HANDLE_W:
		tRect.fX = tStart.fX + fDX;
		tRect.fW = fRight - tRect.fX;
		break;
	default:
		break;
	}
	if ( tRect.fW < fMinW ) {
		if ( iHandle == UI_DESIGN_HANDLE_W || iHandle == UI_DESIGN_HANDLE_NW || iHandle == UI_DESIGN_HANDLE_SW ) tRect.fX = fRight - fMinW;
		tRect.fW = fMinW;
	}
	if ( tRect.fH < fMinH ) {
		if ( iHandle == UI_DESIGN_HANDLE_N || iHandle == UI_DESIGN_HANDLE_NW || iHandle == UI_DESIGN_HANDLE_NE ) tRect.fY = fBottom - fMinH;
		tRect.fH = fMinH;
	}
	if ( tRect.fX < 0.0f ) tRect.fX = 0.0f;
	if ( tRect.fY < 0.0f ) tRect.fY = 0.0f;
	return tRect;
}

static void __uiDesignCanvasCaptureMoveNodes(ui_design_app_t* pApp, int iHit)
{
	ui_design_node_t* pNode;
	int i;
	int iId;

	if ( pApp == NULL ) return;
	pApp->iCanvasDragNodeCount = 0;
	if ( iHit > 0 && uiDesignModelIsSelected(&pApp->tModel, iHit) ) {
		for ( i = 0; i < uiDesignModelGetSelectionCount(&pApp->tModel); ++i ) {
			iId = uiDesignModelGetSelectionId(&pApp->tModel, i);
			pNode = uiDesignModelGetNode(&pApp->tModel, iId);
			if ( pNode == NULL || !__uiDesignCanvasCanFreeTransform(pApp, pNode) ) continue;
			if ( pApp->iCanvasDragNodeCount >= UI_DESIGN_MAX_NODES ) break;
			pApp->arrCanvasDragNodeIds[pApp->iCanvasDragNodeCount] = iId;
			pApp->arrCanvasDragStartRects[pApp->iCanvasDragNodeCount] = pNode->tRect;
			pApp->iCanvasDragNodeCount++;
		}
	} else if ( iHit > 0 ) {
		pNode = uiDesignModelGetNode(&pApp->tModel, iHit);
		if ( pNode != NULL && __uiDesignCanvasCanFreeTransform(pApp, pNode) ) {
			pApp->arrCanvasDragNodeIds[0] = iHit;
			pApp->arrCanvasDragStartRects[0] = pNode->tRect;
			pApp->iCanvasDragNodeCount = 1;
		}
	}
}

static int __uiDesignOverlayEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	ui_design_app_t* pApp;
	ui_design_node_t* pNode;
	xui_rect_t tAbs;
	xui_rect_t tRect;
	float fX;
	float fY;
	float fDX;
	float fDY;
	int iHit;
	int iHandle;

	pApp = (ui_design_app_t*)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) || (pApp == NULL) ) return XUI_OK;
	if ( pEvent->iPhase != XUI_EVENT_PHASE_TARGET ) return XUI_OK;
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_DOWN:
		if ( pEvent->iButton == XUI_POINTER_BUTTON_RIGHT ) {
			(void)xuiSetFocusWidget(pApp->pContext, pWidget);
			if ( uiDesignCanvasWorldToDesign(pApp, pEvent->fX, pEvent->fY, &fX, &fY) != XUI_OK ) return XUI_OK;
			iHit = uiDesignModelHitTest(&pApp->tModel, fX, fY);
			if ( iHit <= 0 || !uiDesignModelIsSelected(&pApp->tModel, iHit) ) (void)uiDesignAppSelectNode(pApp, iHit);
			(void)__uiDesignCanvasOpenContextMenu(pApp, pWidget, pEvent->fX, pEvent->fY, fX, fY, iHit);
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
		(void)xuiSetFocusWidget(pApp->pContext, pWidget);
		if ( uiDesignCanvasWorldToDesign(pApp, pEvent->fX, pEvent->fY, &fX, &fY) != XUI_OK ) return XUI_OK;
		if ( pApp->iActiveTool != UI_DESIGN_NODE_NONE ) {
			pApp->iCanvasDragMode = UI_DESIGN_CANVAS_MODE_CREATE;
			pApp->iCanvasDragHandle = -1;
			pApp->iCanvasDragNodeId = 0;
			pApp->fCanvasDragStartX = fX;
			pApp->fCanvasDragStartY = fY;
			pApp->tCanvasDragStartRect = (xui_rect_t){fX, fY, 0.0f, 0.0f};
			(void)xuiSetPointerCapture(pApp->pContext, pWidget);
			uiDesignAppInvalidate(pApp);
			return XUI_EVENT_DISPATCH_STOP;
		}
		pNode = uiDesignModelGetSelected(&pApp->tModel);
		if ( pNode != NULL && uiDesignModelGetAbsoluteRect(&pApp->tModel, pNode->iId, &tAbs) == XUI_OK ) {
			iHandle = __uiDesignHitHandle(tAbs, fX, fY);
			if ( iHandle >= 0 && __uiDesignCanvasCanFreeTransform(pApp, pNode) ) {
				pApp->iCanvasDragMode = UI_DESIGN_CANVAS_MODE_RESIZE;
				pApp->iCanvasDragHandle = iHandle;
				pApp->iCanvasDragNodeId = pNode->iId;
				pApp->fCanvasDragStartX = fX;
				pApp->fCanvasDragStartY = fY;
				pApp->tCanvasDragStartRect = pNode->tRect;
				(void)uiDesignAppBeginHistoryTransaction(pApp, UI_DESIGN_COMMAND_NONE, "Resize");
				(void)xuiSetPointerCapture(pApp->pContext, pWidget);
				return XUI_EVENT_DISPATCH_STOP;
			}
		}
		iHit = uiDesignModelHitTest(&pApp->tModel, fX, fY);
		if ( (pEvent->iModifiers & XUI_MOD_CTRL) != 0u && iHit > 0 ) {
			(void)uiDesignAppToggleNodeSelection(pApp, iHit);
		} else if ( (pEvent->iModifiers & XUI_MOD_SHIFT) != 0u && iHit > 0 ) {
			(void)uiDesignAppAddNodeSelection(pApp, iHit);
		} else if ( iHit > 0 && !uiDesignModelIsSelected(&pApp->tModel, iHit) ) {
			(void)uiDesignAppSelectNode(pApp, iHit);
		}
		if ( iHit <= 0 ) {
			pApp->iCanvasDragMode = UI_DESIGN_CANVAS_MODE_MARQUEE;
			pApp->iCanvasDragHandle = -1;
			pApp->iCanvasDragNodeId = 0;
			pApp->fCanvasDragStartX = fX;
			pApp->fCanvasDragStartY = fY;
			pApp->tCanvasDragStartRect = (xui_rect_t){fX, fY, 0.0f, 0.0f};
			(void)xuiSetPointerCapture(pApp->pContext, pWidget);
			uiDesignAppInvalidate(pApp);
		} else {
			pNode = uiDesignModelGetNode(&pApp->tModel, iHit);
			if ( pNode != NULL && __uiDesignCanvasCanFreeTransform(pApp, pNode) && uiDesignModelIsSelected(&pApp->tModel, iHit) ) {
				pApp->iCanvasDragMode = UI_DESIGN_CANVAS_MODE_MOVE;
				pApp->iCanvasDragHandle = -1;
				pApp->iCanvasDragNodeId = iHit;
				pApp->fCanvasDragStartX = fX;
				pApp->fCanvasDragStartY = fY;
				pApp->tCanvasDragStartRect = pNode->tRect;
				__uiDesignCanvasCaptureMoveNodes(pApp, iHit);
				(void)uiDesignAppBeginHistoryTransaction(pApp, UI_DESIGN_COMMAND_NONE, "Move");
				(void)xuiSetPointerCapture(pApp->pContext, pWidget);
			}
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_MOVE:
		if ( pApp->iCanvasDragMode == UI_DESIGN_CANVAS_MODE_NONE ) return XUI_OK;
		if ( pApp->iCanvasDragMode == UI_DESIGN_CANVAS_MODE_CREATE || pApp->iCanvasDragMode == UI_DESIGN_CANVAS_MODE_MARQUEE ) {
			if ( __uiDesignCanvasWorldToDesignClamped(pApp, pEvent->fX, pEvent->fY, &fX, &fY) != XUI_OK ) return XUI_EVENT_DISPATCH_STOP;
			pApp->tCanvasDragStartRect = __uiDesignNormalizeRect(pApp->fCanvasDragStartX, pApp->fCanvasDragStartY, fX, fY);
			uiDesignAppInvalidate(pApp);
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( uiDesignCanvasWorldToDesign(pApp, pEvent->fX, pEvent->fY, &fX, &fY) != XUI_OK ) return XUI_EVENT_DISPATCH_STOP;
		fDX = fX - pApp->fCanvasDragStartX;
		fDY = fY - pApp->fCanvasDragStartY;
		tRect = pApp->tCanvasDragStartRect;
		if ( pApp->iCanvasDragMode == UI_DESIGN_CANVAS_MODE_MOVE ) {
			int iDrag;
			for ( iDrag = 0; iDrag < pApp->iCanvasDragNodeCount; ++iDrag ) {
				tRect = pApp->arrCanvasDragStartRects[iDrag];
				tRect.fX += fDX;
				tRect.fY += fDY;
				if ( tRect.fX < 0.0f ) tRect.fX = 0.0f;
				if ( tRect.fY < 0.0f ) tRect.fY = 0.0f;
				tRect = __uiDesignSnapRect(pApp, tRect, 0);
				(void)uiDesignAppSetNodeRect(pApp, pApp->arrCanvasDragNodeIds[iDrag], tRect);
			}
			return XUI_EVENT_DISPATCH_STOP;
		} else if ( pApp->iCanvasDragMode == UI_DESIGN_CANVAS_MODE_RESIZE ) {
			tRect = __uiDesignResizeRect(tRect, pApp->iCanvasDragHandle, fDX, fDY);
			tRect = __uiDesignSnapRect(pApp, tRect, 1);
		}
		(void)uiDesignAppSetNodeRect(pApp, pApp->iCanvasDragNodeId, tRect);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_UP:
		if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT && pApp->iCanvasDragMode != UI_DESIGN_CANVAS_MODE_NONE ) {
			if ( pApp->iCanvasDragMode == UI_DESIGN_CANVAS_MODE_MARQUEE ) {
				if ( __uiDesignCanvasWorldToDesignClamped(pApp, pEvent->fX, pEvent->fY, &fX, &fY) != XUI_OK ) {
					fX = pApp->fCanvasDragStartX;
					fY = pApp->fCanvasDragStartY;
				}
				tRect = __uiDesignNormalizeRect(pApp->fCanvasDragStartX, pApp->fCanvasDragStartY, fX, fY);
				pApp->iCanvasDragMode = UI_DESIGN_CANVAS_MODE_NONE;
				pApp->iCanvasDragHandle = -1;
				pApp->iCanvasDragNodeId = 0;
				pApp->iCanvasDragNodeCount = 0;
				pApp->tCanvasDragStartRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
				(void)xuiReleasePointerCapture(pApp->pContext, pWidget);
				if ( tRect.fW > UI_DESIGN_PLACE_CLICK_THRESHOLD || tRect.fH > UI_DESIGN_PLACE_CLICK_THRESHOLD ) {
					(void)uiDesignCanvasSelectRect(pApp, tRect);
				} else {
					(void)uiDesignAppSelectNode(pApp, 0);
				}
				uiDesignAppInvalidate(pApp);
				return XUI_EVENT_DISPATCH_STOP;
			}
			if ( pApp->iCanvasDragMode == UI_DESIGN_CANVAS_MODE_CREATE ) {
				if ( __uiDesignCanvasWorldToDesignClamped(pApp, pEvent->fX, pEvent->fY, &fX, &fY) != XUI_OK ) {
					fX = pApp->fCanvasDragStartX;
					fY = pApp->fCanvasDragStartY;
				}
				tRect = __uiDesignNormalizeRect(pApp->fCanvasDragStartX, pApp->fCanvasDragStartY, fX, fY);
				pApp->iCanvasDragMode = UI_DESIGN_CANVAS_MODE_NONE;
				pApp->iCanvasDragHandle = -1;
				pApp->iCanvasDragNodeId = 0;
				pApp->iCanvasDragNodeCount = 0;
				pApp->tCanvasDragStartRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
				(void)xuiReleasePointerCapture(pApp->pContext, pWidget);
				(void)uiDesignAppBeginHistoryTransaction(pApp, UI_DESIGN_COMMAND_NONE, "Create");
				if ( pApp->iActiveTool != UI_DESIGN_NODE_NONE &&
				     (tRect.fW > UI_DESIGN_PLACE_CLICK_THRESHOLD || tRect.fH > UI_DESIGN_PLACE_CLICK_THRESHOLD) ) {
					tRect = __uiDesignSnapRect(pApp, tRect, 1);
					if ( uiDesignCanvasPlaceToolRect(pApp, pApp->iActiveTool, tRect, NULL) == XUI_OK ) {
						(void)uiDesignAppCommitHistoryTransaction(pApp);
					} else {
						uiDesignAppCancelHistoryTransaction(pApp);
					}
				} else if ( pApp->iActiveTool != UI_DESIGN_NODE_NONE ) {
					if ( uiDesignAppAddNodeAt(pApp, pApp->iActiveTool, pApp->fCanvasDragStartX, pApp->fCanvasDragStartY, NULL) == XUI_OK ) {
						(void)uiDesignAppCommitHistoryTransaction(pApp);
					} else {
						uiDesignAppCancelHistoryTransaction(pApp);
					}
				} else {
					uiDesignAppCancelHistoryTransaction(pApp);
				}
				uiDesignAppInvalidate(pApp);
				return XUI_EVENT_DISPATCH_STOP;
			}
			pApp->iCanvasDragMode = UI_DESIGN_CANVAS_MODE_NONE;
			pApp->iCanvasDragHandle = -1;
			pApp->iCanvasDragNodeId = 0;
			pApp->iCanvasDragNodeCount = 0;
			pApp->tCanvasDragStartRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
			(void)xuiReleasePointerCapture(pApp->pContext, pWidget);
			(void)uiDesignAppCommitHistoryTransaction(pApp);
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		(void)uiDesignAppCommitHistoryTransaction(pApp);
		pApp->iCanvasDragMode = UI_DESIGN_CANVAS_MODE_NONE;
		pApp->iCanvasDragHandle = -1;
		pApp->iCanvasDragNodeId = 0;
		pApp->iCanvasDragNodeCount = 0;
		pApp->tCanvasDragStartRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		return XUI_EVENT_DISPATCH_STOP;
	default:
		break;
	}
	return XUI_OK;
}

static int __uiDesignCanvasCreateChild(ui_design_app_t* pApp, xui_widget* ppWidget, xui_widget_cache_render_proc onRender)
{
	xui_cache_policy_t tPolicy;
	int iRet;

	iRet = xuiWidgetCreate(pApp->pContext, ppWidget);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tPolicy, 0, sizeof(tPolicy));
	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	(void)xuiWidgetSetLayoutType(*ppWidget, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCachePolicy(*ppWidget, &tPolicy);
	(void)xuiWidgetSetCacheRenderCallback(*ppWidget, onRender, pApp);
	return XUI_OK;
}

int uiDesignCanvasCreate(ui_design_app_t* pApp)
{
	xui_cache_policy_t tPolicy;
	xui_menu_desc_t tMenuDesc;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiWidgetCreate(pApp->pContext, &pApp->pCanvas);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tPolicy, 0, sizeof(tPolicy));
	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	(void)xuiWidgetSetLayoutType(pApp->pCanvas, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetLayoutCallbacks(pApp->pCanvas, __uiDesignCanvasMeasure, __uiDesignCanvasArrange, pApp);
	(void)xuiWidgetSetCachePolicy(pApp->pCanvas, &tPolicy);
	(void)xuiWidgetSetCacheRenderCallback(pApp->pCanvas, __uiDesignCanvasRender, pApp);
	iRet = __uiDesignCanvasCreateChild(pApp, &pApp->pArtboard, __uiDesignArtboardRender);
	if ( iRet == XUI_OK ) iRet = __uiDesignCanvasCreateChild(pApp, &pApp->pOverlay, __uiDesignOverlayRender);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetHitTestVisible(pApp->pArtboard, 0);
	(void)xuiWidgetSetFocusable(pApp->pOverlay, 1);
	(void)xuiWidgetSetTabStop(pApp->pOverlay, 1);
	(void)xuiWidgetSetEventCallback(pApp->pOverlay, __uiDesignOverlayEvent, pApp);
	(void)xuiWidgetSetEventInterest(pApp->pOverlay, XUI_EVENT_MASK_POINTER | XUI_EVENT_MASK_CAPTURE | XUI_EVENT_MASK_FOCUS, 1);
	iRet = xuiWidgetAddChild(pApp->pCanvas, pApp->pArtboard);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pApp->pCanvas, pApp->pOverlay);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tMenuDesc, 0, sizeof(tMenuDesc));
	tMenuDesc.iSize = sizeof(tMenuDesc);
	tMenuDesc.pOwner = pApp->pOverlay;
	tMenuDesc.pFont = pApp->pFont;
	iRet = xuiMenuCreate(pApp->pContext, &pApp->pCanvasContextMenu, &tMenuDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiMenuSetSelect(pApp->pCanvasContextMenu, __uiDesignCanvasContextMenuSelect, pApp);
	return iRet;
}
