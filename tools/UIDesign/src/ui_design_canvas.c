#include "ui_design_canvas.h"

#include <string.h>

#define UI_DESIGN_CANVAS_MODE_NONE 0
#define UI_DESIGN_CANVAS_MODE_MOVE 1
#define UI_DESIGN_CANVAS_MODE_RESIZE 2

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

static int __uiDesignRectContains(xui_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX <= tRect.fX + tRect.fW) && (fY <= tRect.fY + tRect.fH);
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
	ui_design_node_t* pParent;

	if ( (pApp == NULL) || (pNode == NULL) ) return 0;
	if ( pNode->iParentId == 0 ) return 1;
	pParent = uiDesignModelGetNode(&pApp->tModel, pNode->iParentId);
	if ( pParent == NULL ) return 1;
	return uiDesignNodeGetPropertyInt(pParent, "layout.type", XUI_LAYOUT_MANUAL) == XUI_LAYOUT_MANUAL;
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
	for ( x = 20.0f; x < tRect.fW; x += 20.0f ) {
		iRet = pApp->tProxy.drawLine(&pApp->tProxy, pDraw, x, 0.0f, x, tRect.fH, 1.0f, XUI_COLOR_RGBA(232, 237, 244, 255));
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( y = 20.0f; y < tRect.fH; y += 20.0f ) {
		iRet = pApp->tProxy.drawLine(&pApp->tProxy, pDraw, 0.0f, y, tRect.fW, y, 1.0f, XUI_COLOR_RGBA(232, 237, 244, 255));
		if ( iRet != XUI_OK ) return iRet;
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
	int i;
	int iRet;

	(void)pWidget;
	(void)iStateId;
	pApp = (ui_design_app_t*)pUser;
	if ( (pApp == NULL) || (pDraw == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode = uiDesignModelGetSelected(&pApp->tModel);
	if ( pNode == NULL ) return XUI_OK;
	if ( uiDesignModelGetAbsoluteRect(&pApp->tModel, pNode->iId, &tRect) != XUI_OK ) return XUI_OK;
	iRet = pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, tRect, 2.0f, XUI_COLOR_RGBA(37, 124, 214, 255));
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < 8; i++ ) {
		tHandle = __uiDesignHandleRect(tRect, i);
		iRet = pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, tHandle, XUI_COLOR_RGBA(255, 255, 255, 255));
		if ( iRet != XUI_OK ) return iRet;
		iRet = pApp->tProxy.drawRectStroke(&pApp->tProxy, pDraw, tHandle, 1.0f, XUI_COLOR_RGBA(37, 124, 214, 255));
		if ( iRet != XUI_OK ) return iRet;
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

int uiDesignCanvasDropTool(ui_design_app_t* pApp, ui_design_node_type_t iType, float fWorldX, float fWorldY)
{
	float fDesignX;
	float fDesignY;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( uiDesignCanvasWorldToDesign(pApp, fWorldX, fWorldY, &fDesignX, &fDesignY) != XUI_OK ) return XUI_ERROR;
	return uiDesignAppAddNodeAt(pApp, iType, fDesignX, fDesignY, NULL);
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
		if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
		(void)xuiSetFocusWidget(pApp->pContext, pWidget);
		if ( uiDesignCanvasWorldToDesign(pApp, pEvent->fX, pEvent->fY, &fX, &fY) != XUI_OK ) return XUI_OK;
		if ( pApp->iActiveTool != UI_DESIGN_NODE_NONE ) {
			(void)uiDesignAppAddNodeAt(pApp, pApp->iActiveTool, fX, fY, NULL);
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
				(void)xuiSetPointerCapture(pApp->pContext, pWidget);
				return XUI_EVENT_DISPATCH_STOP;
			}
		}
		iHit = uiDesignModelHitTest(&pApp->tModel, fX, fY);
		(void)uiDesignAppSelectNode(pApp, iHit);
		if ( iHit > 0 ) {
			pNode = uiDesignModelGetNode(&pApp->tModel, iHit);
			if ( pNode != NULL && __uiDesignCanvasCanFreeTransform(pApp, pNode) ) {
				pApp->iCanvasDragMode = UI_DESIGN_CANVAS_MODE_MOVE;
				pApp->iCanvasDragHandle = -1;
				pApp->iCanvasDragNodeId = iHit;
				pApp->fCanvasDragStartX = fX;
				pApp->fCanvasDragStartY = fY;
				pApp->tCanvasDragStartRect = pNode->tRect;
				(void)xuiSetPointerCapture(pApp->pContext, pWidget);
			}
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_MOVE:
		if ( pApp->iCanvasDragMode == UI_DESIGN_CANVAS_MODE_NONE ) return XUI_OK;
		if ( uiDesignCanvasWorldToDesign(pApp, pEvent->fX, pEvent->fY, &fX, &fY) != XUI_OK ) return XUI_EVENT_DISPATCH_STOP;
		fDX = fX - pApp->fCanvasDragStartX;
		fDY = fY - pApp->fCanvasDragStartY;
		tRect = pApp->tCanvasDragStartRect;
		if ( pApp->iCanvasDragMode == UI_DESIGN_CANVAS_MODE_MOVE ) {
			tRect.fX += fDX;
			tRect.fY += fDY;
			if ( tRect.fX < 0.0f ) tRect.fX = 0.0f;
			if ( tRect.fY < 0.0f ) tRect.fY = 0.0f;
		} else if ( pApp->iCanvasDragMode == UI_DESIGN_CANVAS_MODE_RESIZE ) {
			tRect = __uiDesignResizeRect(tRect, pApp->iCanvasDragHandle, fDX, fDY);
		}
		(void)uiDesignAppSetNodeRect(pApp, pApp->iCanvasDragNodeId, tRect);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_UP:
		if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT && pApp->iCanvasDragMode != UI_DESIGN_CANVAS_MODE_NONE ) {
			pApp->iCanvasDragMode = UI_DESIGN_CANVAS_MODE_NONE;
			pApp->iCanvasDragHandle = -1;
			pApp->iCanvasDragNodeId = 0;
			(void)xuiReleasePointerCapture(pApp->pContext, pWidget);
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		pApp->iCanvasDragMode = UI_DESIGN_CANVAS_MODE_NONE;
		pApp->iCanvasDragHandle = -1;
		pApp->iCanvasDragNodeId = 0;
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
	return iRet;
}
