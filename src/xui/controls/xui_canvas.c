#include "xui_config.h"
#if XUI_ENABLE_CANVAS
#include "xui_internal.h"

#include <math.h>
#include <string.h>

#define XUI_CANVAS_DEFAULT_WIDTH 640.0f
#define XUI_CANVAS_DEFAULT_HEIGHT 480.0f
#define XUI_CANVAS_MAX_SURFACE_SIZE 32767.0f

typedef struct xui_canvas_data_t {
	xui_widget pFrame;
	xui_widget pViewport;
	xui_surface pSurface;
	float fCanvasWidth;
	float fCanvasHeight;
	float fPenWidth;
	float fLastPenX;
	float fLastPenY;
	uint64_t iPenPointerId;
	int iPenPointerType;
	int bPenEnabled;
	int bPenColorExplicit;
	int bPenDown;
	int bSurfaceGenerationValid;
	int iChangeCount;
	int iDrawCount;
	uint32_t iSurfaceGeneration;
	uint32_t iClearColor;
	uint32_t iBackgroundColor;
	uint32_t iBorderColor;
	uint32_t iPenColor;
	uint32_t iViewportBackgroundColor;
	uint32_t iViewportBorderColor;
	int bViewportStyleSeen;
} xui_canvas_data_t;

static int __xuiCanvasDescValid(const xui_canvas_desc_t* pDesc)
{
	if ( pDesc == NULL ) return 1;
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) return 0;
	if ( (pDesc->iPolicyX != XUI_SCROLLBAR_POLICY_AUTO) &&
	     (pDesc->iPolicyX != XUI_SCROLLBAR_POLICY_ALWAYS) &&
	     (pDesc->iPolicyX != XUI_SCROLLBAR_POLICY_HIDDEN) ) return 0;
	if ( (pDesc->iPolicyY != XUI_SCROLLBAR_POLICY_AUTO) &&
	     (pDesc->iPolicyY != XUI_SCROLLBAR_POLICY_ALWAYS) &&
	     (pDesc->iPolicyY != XUI_SCROLLBAR_POLICY_HIDDEN) ) return 0;
	if ( (pDesc->iScrollbarMode != XUI_SCROLLBAR_MODE_FULL) &&
	     (pDesc->iScrollbarMode != XUI_SCROLLBAR_MODE_COMPACT) ) return 0;
	if ( (pDesc->iWheelAxis != XUI_WHEEL_AXIS_VERTICAL) &&
	     (pDesc->iWheelAxis != XUI_WHEEL_AXIS_HORIZONTAL) &&
	     (pDesc->iWheelAxis != XUI_WHEEL_AXIS_BOTH) ) return 0;
	if ( (pDesc->iCornerMode != XUI_SCROLL_FRAME_CORNER_NONE) &&
	     (pDesc->iCornerMode != XUI_SCROLL_FRAME_CORNER_AUTO) &&
	     (pDesc->iCornerMode != XUI_SCROLL_FRAME_CORNER_GRIP) ) return 0;
	if ( !isfinite(pDesc->fCanvasWidth) || !isfinite(pDesc->fCanvasHeight) ||
	     !isfinite(pDesc->fOffsetX) || !isfinite(pDesc->fOffsetY) ||
	     !isfinite(pDesc->fPenWidth) || !isfinite(pDesc->fScrollbarSize) ||
	     !isfinite(pDesc->fMinThumbSize) || !isfinite(pDesc->fButtonSize) ||
	     !isfinite(pDesc->fWheelStep) || !isfinite(pDesc->fViewportWidth) ||
	     !isfinite(pDesc->fViewportHeight) ||
	     (pDesc->fCanvasWidth < 0.0f) || (pDesc->fCanvasWidth > XUI_CANVAS_MAX_SURFACE_SIZE) ||
	     (pDesc->fCanvasHeight < 0.0f) || (pDesc->fCanvasHeight > XUI_CANVAS_MAX_SURFACE_SIZE) ||
	     (pDesc->fPenWidth < 0.0f) || (pDesc->fScrollbarSize < 0.0f) ||
	     (pDesc->fMinThumbSize < 0.0f) || (pDesc->fButtonSize < 0.0f) ||
	     (pDesc->fWheelStep < 0.0f) ||
	     (pDesc->fOffsetX < -XUI_LAYOUT_UNBOUNDED) || (pDesc->fOffsetX > XUI_LAYOUT_UNBOUNDED) ||
	     (pDesc->fOffsetY < -XUI_LAYOUT_UNBOUNDED) || (pDesc->fOffsetY > XUI_LAYOUT_UNBOUNDED) ||
	     (pDesc->fViewportWidth < 0.0f) || (pDesc->fViewportWidth > XUI_LAYOUT_UNBOUNDED) ||
	     (pDesc->fViewportHeight < 0.0f) || (pDesc->fViewportHeight > XUI_LAYOUT_UNBOUNDED) ) return 0;
	return 1;
}

static int __xuiCanvasAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static float __xuiCanvasMaxFloat(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static int __xuiCanvasToIntSize(float fValue)
{
	int iValue;

	if ( fValue <= 1.0f ) return 1;
	if ( fValue > XUI_CANVAS_MAX_SURFACE_SIZE ) return (int)XUI_CANVAS_MAX_SURFACE_SIZE;
	iValue = (int)(fValue + 0.5f);
	return (iValue > 0) ? iValue : 1;
}

static float __xuiCanvasCanonicalSize(float fValue)
{
	return (float)__xuiCanvasToIntSize(fValue);
}

static int __xuiCanvasRectValid(const xui_rect_t* pRect)
{
	return pRect != NULL && pRect->fW >= 0 && pRect->fH >= 0;
}

static xui_canvas_data_t* __xuiCanvasGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) return NULL;
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "canvas");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) return NULL;
	return (xui_canvas_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiCanvasNoDraw(xui_widget pWidget)
{
	return (__xuiCanvasGetData(pWidget) != NULL) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static xui_proxy __xuiCanvasProxy(xui_widget pWidget)
{
	return xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
}

static int __xuiCanvasStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
{
	xui_style_property_t tProperty;
	int iRet;

	if ( (pWidget == NULL) || (sName == NULL) || (pColor == NULL) ) return 0;
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR) ) {
		*pColor = tProperty.tValue.iColor;
		return 1;
	}
	return 0;
}

static void __xuiCanvasResolve(xui_widget pWidget, const xui_canvas_data_t* pData, xui_canvas_data_t* pResolved)
{
	*pResolved = *pData;
	(void)__xuiCanvasStyleColor(pWidget, "canvas.background.color", &pResolved->iBackgroundColor);
	(void)__xuiCanvasStyleColor(pWidget, "canvas.border.color", &pResolved->iBorderColor);
	if ( !pData->bPenColorExplicit ) {
		(void)__xuiCanvasStyleColor(pWidget, "canvas.pen.color", &pResolved->iPenColor);
	}
}

static int __xuiCanvasInvalidateViewport(xui_canvas_data_t* pData)
{
	if ( (pData == NULL) || (pData->pViewport == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiWidgetInvalidate(pData->pViewport, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void __xuiCanvasRememberSurfaceGeneration(xui_widget pWidget,
	xui_canvas_data_t* pData)
{
	xui_proxy pProxy;
	uint32_t iGeneration;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->pSurface == NULL) ) {
		if ( pData != NULL ) pData->bSurfaceGenerationValid = 0;
		return;
	}
	pProxy = __xuiCanvasProxy(pWidget);
	if ( (pProxy == NULL) || (pProxy->surfaceGetGeneration == NULL) ||
	     (pProxy->surfaceGetGeneration(pProxy, pData->pSurface, &iGeneration) != XUI_OK) ) {
		pData->bSurfaceGenerationValid = 0;
		return;
	}
	pData->iSurfaceGeneration = iGeneration;
	pData->bSurfaceGenerationValid = 1;
}

static int __xuiCanvasSyncSurfaceGeneration(xui_widget pWidget,
	xui_canvas_data_t* pData)
{
	xui_proxy pProxy;
	uint32_t iGeneration;
	int bChanged;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->pSurface == NULL) ) return 0;
	pProxy = __xuiCanvasProxy(pWidget);
	if ( (pProxy == NULL) || (pProxy->surfaceGetGeneration == NULL) ||
	     (pProxy->surfaceGetGeneration(pProxy, pData->pSurface, &iGeneration) != XUI_OK) ) {
		pData->bSurfaceGenerationValid = 0;
		return 0;
	}
	bChanged = pData->bSurfaceGenerationValid &&
		pData->iSurfaceGeneration != iGeneration;
	pData->iSurfaceGeneration = iGeneration;
	pData->bSurfaceGenerationValid = 1;
	return bChanged;
}

static int __xuiCanvasGetSnappedOffset(const xui_canvas_data_t* pData,
	float* pOffsetX, float* pOffsetY)
{
	float fOffsetX;
	float fOffsetY;
	int iRet;

	if ( (pData == NULL) || (pData->pFrame == NULL) ||
	     (pOffsetX == NULL) || (pOffsetY == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	fOffsetX = 0.0f;
	fOffsetY = 0.0f;
	iRet = xuiScrollFrameGetOffset(pData->pFrame, &fOffsetX, &fOffsetY);
	if ( iRet != XUI_OK ) return iRet;
	*pOffsetX = (float)xuiInternalSnapPixel(fOffsetX);
	*pOffsetY = (float)xuiInternalSnapPixel(fOffsetY);
	return XUI_OK;
}

static int __xuiCanvasPreparePaint(xui_widget pWidget)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	xui_canvas_data_t tResolved;
	int bInvalidate;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiCanvasResolve(pWidget, pData, &tResolved);
	bInvalidate = __xuiCanvasSyncSurfaceGeneration(pWidget, pData);
	if ( !pData->bViewportStyleSeen || pData->iViewportBackgroundColor != tResolved.iBackgroundColor ||
	     pData->iViewportBorderColor != tResolved.iBorderColor ) {
		pData->bViewportStyleSeen = 1;
		pData->iViewportBackgroundColor = tResolved.iBackgroundColor;
		pData->iViewportBorderColor = tResolved.iBorderColor;
		bInvalidate = 1;
	}
	return bInvalidate ? __xuiCanvasInvalidateViewport(pData) : XUI_OK;
}

static int __xuiCanvasClearSurface(xui_proxy pProxy, xui_surface pSurface, float fWidth,
	float fHeight, uint32_t iColor)
{
	xui_draw_context pDraw;
	xui_rect_t tRect;
	int iRet;
	int iEndRet;

	if ( (pProxy == NULL) || (pSurface == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pProxy->surfaceClear != NULL ) {
		iRet = pProxy->surfaceClear(pProxy, pSurface, iColor);
		if ( iRet != XUI_ERROR_UNSUPPORTED ) return iRet;
	}
	if ( (pProxy->drawBegin == NULL) || (pProxy->drawClearRect == NULL) || (pProxy->drawEnd == NULL) ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	pDraw = NULL;
	iRet = pProxy->drawBegin(pProxy, &pDraw, pSurface);
	if ( iRet != XUI_OK ) return iRet;
	tRect = (xui_rect_t){0, 0, __xuiCanvasToIntSize(fWidth), __xuiCanvasToIntSize(fHeight)};
	iRet = pProxy->drawClearRect(pProxy, pDraw, tRect, iColor);
	iEndRet = pProxy->drawEnd(pProxy, pDraw);
	return (iRet != XUI_OK) ? iRet : iEndRet;
}

static int __xuiCanvasCreateSurface(xui_widget pWidget, float fWidth, float fHeight,
	uint32_t iClearColor, int bClear, xui_surface* ppSurface)
{
	xui_surface_desc_t tDesc;
	xui_surface pSurface;
	xui_proxy pProxy;
	int iRet;

	if ( (pWidget == NULL) || (ppSurface == NULL) || !isfinite(fWidth) || !isfinite(fHeight) ||
	     (fWidth <= 0.0f) || (fHeight <= 0.0f) ||
	     (fWidth > XUI_CANVAS_MAX_SURFACE_SIZE) || (fHeight > XUI_CANVAS_MAX_SURFACE_SIZE) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppSurface = NULL;
	pProxy = __xuiCanvasProxy(pWidget);
	if ( (pProxy == NULL) || (pProxy->surfaceCreate == NULL) || (pProxy->surfaceDestroy == NULL) ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tDesc.iWidth = __xuiCanvasToIntSize(fWidth);
	tDesc.iHeight = __xuiCanvasToIntSize(fHeight);
	tDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	pSurface = NULL;
	iRet = pProxy->surfaceCreate(pProxy, &pSurface, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	if ( pSurface == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	if ( bClear ) iRet = __xuiCanvasClearSurface(pProxy, pSurface, fWidth, fHeight, iClearColor);
	if ( iRet != XUI_OK ) {
		if ( pProxy->surfaceDestroy != NULL ) pProxy->surfaceDestroy(pProxy, pSurface);
		return iRet;
	}
	*ppSurface = pSurface;
	return XUI_OK;
}

static int __xuiCanvasEnsureSurface(xui_widget pWidget, xui_canvas_data_t* pData, int bClear)
{
	xui_surface pSurface;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pSurface != NULL ) return XUI_OK;
	pSurface = NULL;
	iRet = __xuiCanvasCreateSurface(pWidget, pData->fCanvasWidth, pData->fCanvasHeight,
		pData->iClearColor, bClear, &pSurface);
	if ( iRet != XUI_OK ) return iRet;
	pData->pSurface = pSurface;
	__xuiCanvasRememberSurfaceGeneration(pWidget, pData);
	return XUI_OK;
}

static void __xuiCanvasCancelPen(xui_widget pWidget, xui_canvas_data_t* pData);

static int __xuiCanvasWithDraw(xui_widget pWidget, int (*onDraw)(xui_proxy, xui_draw_context, void*), void* pUser)
{
	xui_canvas_data_t* pData;
	xui_proxy pProxy;
	xui_draw_context pDraw;
	int iRet;
	int iEndRet;

	pData = __xuiCanvasGetData(pWidget);
	if ( (pData == NULL) || (onDraw == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxy = __xuiCanvasProxy(pWidget);
	if ( (pProxy == NULL) || (pProxy->drawBegin == NULL) || (pProxy->drawEnd == NULL) ) return XUI_ERROR_NOT_INITIALIZED;
	iRet = __xuiCanvasEnsureSurface(pWidget, pData, 1);
	if ( iRet != XUI_OK ) return iRet;
	pDraw = NULL;
	iRet = pProxy->drawBegin(pProxy, &pDraw, pData->pSurface);
	if ( iRet != XUI_OK ) return iRet;
	iRet = onDraw(pProxy, pDraw, pUser);
	iEndRet = pProxy->drawEnd(pProxy, pDraw);
	if ( iRet == XUI_OK ) iRet = iEndRet;
	if ( iRet == XUI_OK ) {
		__xuiCanvasRememberSurfaceGeneration(pWidget, pData);
		pData->iDrawCount++;
		iRet = __xuiCanvasInvalidateViewport(pData);
	}
	return iRet;
}

static void __xuiCanvasFrameChanged(xui_widget pFrame, float fOffsetX, float fOffsetY, void* pUser)
{
	xui_widget pWidget;
	xui_canvas_data_t* pData;

	(void)pFrame;
	(void)fOffsetX;
	(void)fOffsetY;
	pWidget = (xui_widget)pUser;
	pData = __xuiCanvasGetData(pWidget);
	if ( pData == NULL ) return;
	__xuiCanvasCancelPen(pWidget, pData);
	pData->iChangeCount++;
	(void)__xuiCanvasInvalidateViewport(pData);
}

static int __xuiCanvasViewportRender(xui_widget pViewport, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_widget pWidget;
	xui_canvas_data_t* pData;
	xui_canvas_data_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tViewport;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	float fOffsetX;
	float fOffsetY;
	int iRet;

	(void)iStateId;
	pWidget = (xui_widget)pUser;
	pData = __xuiCanvasGetData(pWidget);
	if ( (pData == NULL) || (pViewport == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxy = __xuiCanvasProxy(pWidget);
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	__xuiCanvasResolve(pWidget, pData, &tResolved);
	tViewport = xuiWidgetGetContentRect(pViewport);
	iRet = XUI_OK;
	if ( (__xuiCanvasAlpha(tResolved.iBackgroundColor) != 0) && (pProxy->drawRectFill != NULL) ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, tViewport, tResolved.iBackgroundColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiCanvasEnsureSurface(pWidget, pData, 1);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCanvasGetSnappedOffset(pData, &fOffsetX, &fOffsetY);
	if ( iRet != XUI_OK ) return iRet;
	tSrc = (xui_rect_t){fOffsetX, fOffsetY, tViewport.fW, tViewport.fH};
	tDst = tViewport;
	if ( tSrc.fX < 0.0f ) tSrc.fX = 0.0f;
	if ( tSrc.fY < 0.0f ) tSrc.fY = 0.0f;
	if ( (tSrc.fX + tSrc.fW) > pData->fCanvasWidth ) tSrc.fW = __xuiCanvasMaxFloat(0.0f, pData->fCanvasWidth - tSrc.fX);
	if ( (tSrc.fY + tSrc.fH) > pData->fCanvasHeight ) tSrc.fH = __xuiCanvasMaxFloat(0.0f, pData->fCanvasHeight - tSrc.fY);
	if ( (tSrc.fW > 0.0f) && (tSrc.fH > 0.0f) && (pProxy->drawSurface != NULL) ) {
		tDst.fW = tSrc.fW;
		tDst.fH = tSrc.fH;
		iRet = pProxy->drawSurface(pProxy, pDraw, pData->pSurface, tSrc, tDst, XUI_COLOR_WHITE, 0);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (__xuiCanvasAlpha(tResolved.iBorderColor) != 0) && (pProxy->drawRectStroke != NULL) ) {
		iRet = pProxy->drawRectStroke(pProxy, pDraw, tViewport, 1.0f, tResolved.iBorderColor);
	}
	return iRet;
}

static int __xuiCanvasEventToContent(xui_canvas_data_t* pData,
	const xui_event_t* pEvent, xui_vec2_t* pPoint)
{
	xui_rect_t tViewport;
	xui_rect_t tContent;
	float fOffsetX;
	float fOffsetY;
	int iRet;

	if ( (pData == NULL) || (pEvent == NULL) || (pPoint == NULL) ||
	     (pData->pViewport == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tViewport = xuiWidgetGetWorldRect(pData->pViewport);
	tContent = xuiWidgetGetContentRect(pData->pViewport);
	iRet = __xuiCanvasGetSnappedOffset(pData, &fOffsetX, &fOffsetY);
	if ( iRet != XUI_OK ) return iRet;
	*pPoint = (xui_vec2_t){pEvent->fX - tViewport.fX - tContent.fX + fOffsetX,
		pEvent->fY - tViewport.fY - tContent.fY + fOffsetY};
	return XUI_OK;
}

static int __xuiCanvasPenPointerMatches(const xui_canvas_data_t* pData,
	const xui_event_t* pEvent)
{
	return pData != NULL && pEvent != NULL && pData->bPenDown &&
	       pData->iPenPointerId == pEvent->iPointerId &&
	       pData->iPenPointerType == pEvent->iPointerType;
}

static void __xuiCanvasCancelPen(xui_widget pWidget, xui_canvas_data_t* pData)
{
	xui_context pContext;
	uint64_t iPointerId;
	int iPointerType;

	if ( (pWidget == NULL) || (pData == NULL) || !pData->bPenDown ) return;
	iPointerId = pData->iPenPointerId;
	iPointerType = pData->iPenPointerType;
	pData->bPenDown = 0;
	pData->iPenPointerId = 0;
	pData->iPenPointerType = XUI_POINTER_TYPE_MOUSE;
	pContext = xuiWidgetGetContext(pWidget);
	if ( xuiInternalContextIsValid(pContext) && (pData->pViewport != NULL) &&
	     (xuiGetPointerCaptureEx(pContext, iPointerId, iPointerType) == pData->pViewport) ) {
		(void)xuiReleasePointerCaptureEx(pContext, iPointerId, iPointerType, pData->pViewport);
	}
}

static int __xuiCanvasDrawPenTo(xui_widget pWidget, xui_canvas_data_t* pData,
	xui_vec2_t tPoint, uint32_t iColor)
{
	int iRet;

	if ( (pData->fLastPenX == tPoint.fX) && (pData->fLastPenY == tPoint.fY) ) {
		return XUI_OK;
	}
	iRet = xuiCanvasDrawLine(pWidget, pData->fLastPenX, pData->fLastPenY,
		tPoint.fX, tPoint.fY, pData->fPenWidth, iColor);
	if ( iRet == XUI_OK ) {
		pData->fLastPenX = tPoint.fX;
		pData->fLastPenY = tPoint.fY;
	}
	return iRet;
}

static int __xuiCanvasFinishScroll(xui_widget pWidget, xui_canvas_data_t* pData,
	float fOldX, float fOldY)
{
	float fNewX;
	float fNewY;
	int iRet;

	iRet = xuiScrollFrameGetOffset(pData->pFrame, &fNewX, &fNewY);
	if ( iRet != XUI_OK ) {
		(void)xuiScrollFrameSetOffset(pData->pFrame, fOldX, fOldY);
		return iRet;
	}
	if ( (fOldX == fNewX) && (fOldY == fNewY) ) return XUI_OK;
	iRet = __xuiCanvasInvalidateViewport(pData);
	if ( iRet != XUI_OK ) {
		(void)xuiScrollFrameSetOffset(pData->pFrame, fOldX, fOldY);
		(void)__xuiCanvasInvalidateViewport(pData);
		return iRet;
	}
	__xuiCanvasCancelPen(pWidget, pData);
	return XUI_OK;
}

static int __xuiCanvasEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_canvas_data_t* pData;

	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiCanvasGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (pEvent->iType == XUI_EVENT_ENABLED_CHANGED) || (pEvent->iType == XUI_EVENT_VISIBLE_CHANGED) ) {
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) {
			__xuiCanvasCancelPen(pWidget, pData);
		}
	}
	return XUI_OK;
}

static int __xuiCanvasViewportEvent(xui_widget pViewport, const xui_event_t* pEvent, void* pUser)
{
	xui_widget pWidget;
	xui_context pContext;
	xui_canvas_data_t* pData;
	xui_canvas_data_t tResolved;
	xui_vec2_t tPoint;
	int iRet;

	pWidget = (xui_widget)pUser;
	pData = __xuiCanvasGetData(pWidget);
	if ( (pData == NULL) || (pEvent == NULL) ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	if ( pEvent->iType == XUI_EVENT_POINTER_CAPTURE_LOST ) {
		if ( __xuiCanvasPenPointerMatches(pData, pEvent) ) {
			pData->bPenDown = 0;
			pData->iPenPointerId = 0;
			pData->iPenPointerType = XUI_POINTER_TYPE_MOUSE;
		}
		return XUI_OK;
	}
	if ( (pEvent->iType == XUI_EVENT_ENABLED_CHANGED) || (pEvent->iType == XUI_EVENT_VISIBLE_CHANGED) ) {
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ||
		     !xuiWidgetGetEffectiveEnabled(pViewport) || !xuiWidgetGetEffectiveVisible(pViewport) ) {
			__xuiCanvasCancelPen(pWidget, pData);
		}
		return XUI_OK;
	}
	if ( !pData->bPenEnabled || !xuiWidgetGetEffectiveEnabled(pWidget) ||
	     !xuiWidgetGetEffectiveVisible(pWidget) ||
	     !xuiWidgetGetEffectiveEnabled(pViewport) ||
	     !xuiWidgetGetEffectiveVisible(pViewport) ) {
		__xuiCanvasCancelPen(pWidget, pData);
		return XUI_OK;
	}
	__xuiCanvasResolve(pWidget, pData, &tResolved);
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_DOWN:
		if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT || pData->bPenDown ) return XUI_OK;
		iRet = __xuiCanvasEventToContent(pData, pEvent, &tPoint);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiSetPointerCaptureEx(pContext, pEvent->iPointerId,
			pEvent->iPointerType, pViewport);
		if ( iRet != XUI_OK ) {
			if ( xuiGetPointerCaptureEx(pContext, pEvent->iPointerId,
			     pEvent->iPointerType) == pViewport ) {
				(void)xuiReleasePointerCaptureEx(pContext,
					pEvent->iPointerId, pEvent->iPointerType, pViewport);
			}
			return iRet;
		}
		if ( xuiGetPointerCaptureEx(pContext, pEvent->iPointerId,
		     pEvent->iPointerType) != pViewport ||
		     !xuiWidgetGetEffectiveEnabled(pWidget) ||
		     !xuiWidgetGetEffectiveVisible(pWidget) ||
		     !xuiWidgetGetEffectiveEnabled(pViewport) ||
		     !xuiWidgetGetEffectiveVisible(pViewport) ) {
			(void)xuiReleasePointerCaptureEx(pContext,
				pEvent->iPointerId, pEvent->iPointerType, pViewport);
			return XUI_OK;
		}
		pData->bPenDown = 1;
		pData->iPenPointerId = pEvent->iPointerId;
		pData->iPenPointerType = pEvent->iPointerType;
		pData->fLastPenX = tPoint.fX;
		pData->fLastPenY = tPoint.fY;
		iRet = xuiCanvasDrawPoint(pWidget, tPoint.fX, tPoint.fY, pData->fPenWidth, tResolved.iPenColor);
		if ( iRet != XUI_OK ) {
			__xuiCanvasCancelPen(pWidget, pData);
			return iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_MOVE:
		if ( !__xuiCanvasPenPointerMatches(pData, pEvent) ) return XUI_OK;
		if ( xuiGetPointerCaptureEx(pContext, pEvent->iPointerId,
		     pEvent->iPointerType) != pViewport ) {
			__xuiCanvasCancelPen(pWidget, pData);
			return XUI_OK;
		}
		if ( (pEvent->iButtons & XUI_POINTER_BUTTON_LEFT) == 0u ) {
			__xuiCanvasCancelPen(pWidget, pData);
			return XUI_EVENT_DISPATCH_STOP;
		}
		iRet = __xuiCanvasEventToContent(pData, pEvent, &tPoint);
		if ( iRet != XUI_OK ) {
			__xuiCanvasCancelPen(pWidget, pData);
			return iRet;
		}
		iRet = __xuiCanvasDrawPenTo(pWidget, pData, tPoint, tResolved.iPenColor);
		if ( iRet != XUI_OK ) {
			__xuiCanvasCancelPen(pWidget, pData);
			return iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_UP:
		if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT &&
		     __xuiCanvasPenPointerMatches(pData, pEvent) ) {
			iRet = XUI_OK;
			if ( xuiGetPointerCaptureEx(pContext, pEvent->iPointerId,
			     pEvent->iPointerType) == pViewport ) {
				iRet = __xuiCanvasEventToContent(pData, pEvent, &tPoint);
				if ( iRet == XUI_OK ) {
					iRet = __xuiCanvasDrawPenTo(pWidget, pData, tPoint,
						tResolved.iPenColor);
				}
			}
			__xuiCanvasCancelPen(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	default:
		break;
	}
	return XUI_OK;
}

static void __xuiCanvasDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_OVERLAY;
	pLayout->iWidthMode = XUI_SIZE_CONTENT;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_CLIP;
	pLayout->iMeasureContainment = XUI_MEASURE_CONTAIN_BOTH;
	pLayout->iAlignX = XUI_ALIGN_START;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static int __xuiCanvasContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_canvas_data_t* pData = (xui_canvas_data_t*)pUser;
	(void)pWidget;
	(void)tConstraint;
	if ( pData == NULL || pSize == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pFrame == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	return xuiScrollFrameGetViewportHint(pData->pFrame, &pSize->fX, &pSize->fY);
}

static void __xuiCanvasDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiCanvasInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_canvas_data_t* pData;
	const xui_canvas_desc_t* pDesc;
	xui_scroll_frame_desc_t tFrameDesc;
	float fCanvasWidth;
	float fCanvasHeight;
	float fViewportWidth;
	float fViewportHeight;
	int iRet;

	(void)pUser;
	pData = (xui_canvas_data_t*)pTypeData;
	pDesc = (const xui_canvas_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiCanvasDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pData, 0, sizeof(*pData));
	fCanvasWidth = (pDesc != NULL && pDesc->fCanvasWidth > 0.0f) ? pDesc->fCanvasWidth : XUI_CANVAS_DEFAULT_WIDTH;
	fCanvasHeight = (pDesc != NULL && pDesc->fCanvasHeight > 0.0f) ? pDesc->fCanvasHeight : XUI_CANVAS_DEFAULT_HEIGHT;
	fViewportWidth = (pDesc != NULL && pDesc->fViewportWidth > 0.0f) ? pDesc->fViewportWidth : XUI_CANVAS_DEFAULT_WIDTH;
	fViewportHeight = (pDesc != NULL && pDesc->fViewportHeight > 0.0f) ? pDesc->fViewportHeight : XUI_CANVAS_DEFAULT_HEIGHT;
	pData->fCanvasWidth = __xuiCanvasCanonicalSize(fCanvasWidth);
	pData->fCanvasHeight = __xuiCanvasCanonicalSize(fCanvasHeight);
	pData->iPenPointerType = XUI_POINTER_TYPE_MOUSE;
	pData->fPenWidth = (pDesc != NULL && pDesc->fPenWidth > 0.0f) ? pDesc->fPenWidth : 3.0f;
	pData->bPenEnabled = (pDesc != NULL) ? (pDesc->bPenEnabled != 0) : 0;
	pData->iClearColor = (pDesc != NULL && pDesc->iClearColor != 0) ? pDesc->iClearColor : XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iBackgroundColor = (pDesc != NULL && pDesc->iBackgroundColor != 0) ? pDesc->iBackgroundColor : XUI_COLOR_RGBA(246, 249, 252, 255);
	pData->iBorderColor = (pDesc != NULL && pDesc->iBorderColor != 0) ? pDesc->iBorderColor : XUI_COLOR_RGBA(190, 205, 220, 255);
	pData->iPenColor = (pDesc != NULL && pDesc->iPenColor != 0) ? pDesc->iPenColor : XUI_COLOR_RGBA(34, 107, 214, 255);
	pData->bPenColorExplicit = (pDesc != NULL && pDesc->iPenColor != 0);
	memset(&tFrameDesc, 0, sizeof(tFrameDesc));
	tFrameDesc.iSize = sizeof(tFrameDesc);
	tFrameDesc.fContentWidth = pData->fCanvasWidth;
	tFrameDesc.fContentHeight = pData->fCanvasHeight;
	tFrameDesc.fViewportWidth = fViewportWidth;
	tFrameDesc.fViewportHeight = fViewportHeight;
	tFrameDesc.fOffsetX = (pDesc != NULL) ? pDesc->fOffsetX : 0.0f;
	tFrameDesc.fOffsetY = (pDesc != NULL) ? pDesc->fOffsetY : 0.0f;
	tFrameDesc.iPolicyX = (pDesc != NULL) ? pDesc->iPolicyX : XUI_SCROLLBAR_POLICY_AUTO;
	tFrameDesc.iPolicyY = (pDesc != NULL) ? pDesc->iPolicyY : XUI_SCROLLBAR_POLICY_AUTO;
	tFrameDesc.iScrollbarMode = (pDesc != NULL) ? pDesc->iScrollbarMode : XUI_SCROLLBAR_MODE_COMPACT;
	tFrameDesc.iWheelAxis = (pDesc != NULL) ? pDesc->iWheelAxis : XUI_WHEEL_AXIS_BOTH;
	tFrameDesc.iCornerMode = (pDesc != NULL) ? pDesc->iCornerMode : XUI_SCROLL_FRAME_CORNER_GRIP;
	tFrameDesc.bContentDragEnabled = (pDesc != NULL) ? pDesc->bContentDragEnabled : 0;
	tFrameDesc.fScrollbarSize = (pDesc != NULL) ? pDesc->fScrollbarSize : 8.0f;
	tFrameDesc.fMinThumbSize = (pDesc != NULL) ? pDesc->fMinThumbSize : 18.0f;
	tFrameDesc.fButtonSize = (pDesc != NULL) ? pDesc->fButtonSize : 0.0f;
	tFrameDesc.fWheelStep = (pDesc != NULL) ? pDesc->fWheelStep : 48.0f;
	tFrameDesc.iBackgroundColor = pData->iBackgroundColor;
	if ( pDesc != NULL ) {
		tFrameDesc.iTrackColor = pDesc->iTrackColor;
		tFrameDesc.iThumbColor = pDesc->iThumbColor;
		tFrameDesc.iHoverColor = pDesc->iHoverColor;
		tFrameDesc.iActiveColor = pDesc->iActiveColor;
		tFrameDesc.iFocusColor = pDesc->iFocusColor;
		tFrameDesc.iDisabledColor = pDesc->iDisabledColor;
		tFrameDesc.iButtonColor = pDesc->iButtonColor;
		tFrameDesc.iButtonIconColor = pDesc->iButtonIconColor;
		tFrameDesc.iCornerColor = pDesc->iCornerColor;
		tFrameDesc.iGripColor = pDesc->iGripColor;
	}
	iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiCanvasEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiCanvasEvent, NULL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiScrollFrameCreate(xuiWidgetGetContext(pWidget), &pData->pFrame, &tFrameDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pWidget, pData->pFrame);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pData->pFrame);
		pData->pFrame = NULL;
		return iRet;
	}
	iRet = xuiWidgetSetFlowMode(pData->pFrame, XUI_FLOW_BLOCK);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pData->pFrame, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAlign(pData->pFrame, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameSetChange(pData->pFrame, __xuiCanvasFrameChanged, pWidget);
	if ( iRet != XUI_OK ) goto cleanup_frame;
	pData->pViewport = xuiScrollFrameGetViewportWidget(pData->pFrame);
	if ( pData->pViewport == NULL ) {
		iRet = XUI_ERROR_NOT_INITIALIZED;
		goto cleanup_frame;
	}
	iRet = xuiWidgetSetCacheRenderCallback(pData->pViewport, __xuiCanvasViewportRender, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pViewport, XUI_EVENT_POINTER_DOWN, __xuiCanvasViewportEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pViewport, XUI_EVENT_POINTER_MOVE, __xuiCanvasViewportEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pViewport, XUI_EVENT_POINTER_UP, __xuiCanvasViewportEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pViewport, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiCanvasViewportEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pViewport, XUI_EVENT_ENABLED_CHANGED, __xuiCanvasViewportEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pViewport, XUI_EVENT_VISIBLE_CHANGED, __xuiCanvasViewportEvent, pWidget);
	if ( iRet == XUI_OK ) return XUI_OK;

cleanup_frame:
	pData->pViewport = NULL;
	if ( pData->pFrame != NULL ) {
		xuiWidgetDestroy(pData->pFrame);
		pData->pFrame = NULL;
	}
	return iRet;
}

static void __xuiCanvasDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_canvas_data_t* pData;
	xui_proxy pProxy;

	(void)pUser;
	pData = (xui_canvas_data_t*)pTypeData;
	if ( pData != NULL ) {
		__xuiCanvasCancelPen(pWidget, pData);
		pProxy = (pWidget != NULL) ? __xuiCanvasProxy(pWidget) : NULL;
		if ( (pProxy != NULL) && (pProxy->surfaceDestroy != NULL) && (pData->pSurface != NULL) ) {
			pProxy->surfaceDestroy(pProxy, pData->pSurface);
		}
		memset(pData, 0, sizeof(*pData));
	}
}

static void __xuiCanvasRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
{
	xui_style_property_info_t tInfo;

	if ( xuiStyleFindProperty(pContext, sName) != 0 ) return;
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.sName = sName;
	tInfo.iValueType = iValueType;
	tInfo.iDirtyFlags = iDirtyFlags;
	tInfo.iFlags = iFlags;
	tInfo.pWidgetType = pType;
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
}

static void __xuiCanvasRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;

	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiCanvasRegisterStyleProperty(pContext, pType, "canvas.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCanvasRegisterStyleProperty(pContext, pType, "canvas.border.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCanvasRegisterStyleProperty(pContext, pType, "canvas.pen.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
}

XUI_API xui_widget_type xuiCanvasGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "canvas");
	if ( pType != NULL ) {
		pType->onPreparePaint = __xuiCanvasPreparePaint;
		__xuiCanvasRegisterStyleProperties(pContext, pType);
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "canvas";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_canvas_data_t);
	tDesc.onInit = __xuiCanvasInit;
	tDesc.onDestroy = __xuiCanvasDestroy;
	tDesc.onContentMeasure = __xuiCanvasContentMeasure;
	__xuiCanvasDefaultLayout(&tDesc.tLayout);
	__xuiCanvasDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) return NULL;
	pType->onPreparePaint = __xuiCanvasPreparePaint;
	__xuiCanvasRegisterStyleProperties(pContext, pType);
	return pType;
}

XUI_API int xuiCanvasCreate(xui_context pContext, xui_widget* ppWidget, const xui_canvas_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	if ( !xuiInternalContextIsValid(pContext) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiCanvasDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	pType = xuiCanvasGetType(pContext);
	if ( pType == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API xui_widget xuiCanvasGetFrameWidget(xui_widget pWidget)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	return (pData != NULL) ? pData->pFrame : NULL;
}

XUI_API xui_widget xuiCanvasGetViewportWidget(xui_widget pWidget)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	return (pData != NULL) ? pData->pViewport : NULL;
}

XUI_API xui_scroll_model_t* xuiCanvasGetModel(xui_widget pWidget)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	return (pData != NULL && pData->pFrame != NULL) ? xuiScrollFrameGetModel(pData->pFrame) : NULL;
}

XUI_API xui_surface xuiCanvasGetSurface(xui_widget pWidget)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	if ( pData == NULL ) return NULL;
	(void)__xuiCanvasEnsureSurface(pWidget, pData, 1);
	return pData->pSurface;
}

XUI_API int xuiCanvasSetCanvasSize(xui_widget pWidget, float fWidth, float fHeight)
{
	xui_canvas_data_t* pData;
	xui_proxy pProxy;
	xui_surface pOldSurface;
	xui_surface pNewSurface;
	float fOldOffsetX;
	float fOldOffsetY;
	float fNewWidth;
	float fNewHeight;
	int iRet;

	pData = __xuiCanvasGetData(pWidget);
	if ( (pData == NULL) || !isfinite(fWidth) || !isfinite(fHeight) ||
	     (fWidth <= 0.0f) || (fHeight <= 0.0f) ||
	     (fWidth > XUI_CANVAS_MAX_SURFACE_SIZE) || (fHeight > XUI_CANVAS_MAX_SURFACE_SIZE) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	fNewWidth = __xuiCanvasCanonicalSize(fWidth);
	fNewHeight = __xuiCanvasCanonicalSize(fHeight);
	if ( (pData->fCanvasWidth == fNewWidth) && (pData->fCanvasHeight == fNewHeight) ) return XUI_OK;
	iRet = xuiScrollFrameGetOffset(pData->pFrame, &fOldOffsetX, &fOldOffsetY);
	if ( iRet != XUI_OK ) return iRet;
	pProxy = __xuiCanvasProxy(pWidget);
	pNewSurface = NULL;
	if ( pData->pSurface != NULL ) {
		iRet = __xuiCanvasCreateSurface(pWidget, fNewWidth, fNewHeight,
			pData->iClearColor, 1, &pNewSurface);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = xuiScrollFrameSetContentSize(pData->pFrame, fNewWidth, fNewHeight);
	if ( iRet != XUI_OK ) {
		if ( pNewSurface != NULL ) pProxy->surfaceDestroy(pProxy, pNewSurface);
		return iRet;
	}
	iRet = __xuiCanvasInvalidateViewport(pData);
	if ( iRet != XUI_OK ) {
		(void)xuiScrollFrameSetContentSize(pData->pFrame, pData->fCanvasWidth, pData->fCanvasHeight);
		(void)xuiScrollFrameSetOffset(pData->pFrame, fOldOffsetX, fOldOffsetY);
		if ( pNewSurface != NULL ) pProxy->surfaceDestroy(pProxy, pNewSurface);
		return iRet;
	}
	__xuiCanvasCancelPen(pWidget, pData);
	pOldSurface = pData->pSurface;
	pData->pSurface = pNewSurface;
	pData->fCanvasWidth = fNewWidth;
	pData->fCanvasHeight = fNewHeight;
	__xuiCanvasRememberSurfaceGeneration(pWidget, pData);
	if ( pOldSurface != NULL ) pProxy->surfaceDestroy(pProxy, pOldSurface);
	return XUI_OK;
}

XUI_API int xuiCanvasGetCanvasSize(xui_widget pWidget, float* pWidth, float* pHeight)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pWidth != NULL ) *pWidth = pData->fCanvasWidth;
	if ( pHeight != NULL ) *pHeight = pData->fCanvasHeight;
	return XUI_OK;
}

XUI_API int xuiCanvasSetViewportHint(xui_widget pWidget, float fWidth, float fHeight)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	float fOldWidth;
	float fOldHeight;
	int iRet;
	if ( pData == NULL || !isfinite(fWidth) || !isfinite(fHeight) ||
	     fWidth <= 0.0f || fHeight <= 0.0f ||
	     fWidth > XUI_LAYOUT_UNBOUNDED || fHeight > XUI_LAYOUT_UNBOUNDED ||
	     fWidth != fWidth || fHeight != fHeight ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xuiScrollFrameGetViewportHint(pData->pFrame, &fOldWidth, &fOldHeight);
	if ( iRet != XUI_OK ) return iRet;
	if ( fOldWidth == fWidth && fOldHeight == fHeight ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiScrollFrameSetViewportHint(pData->pFrame, fWidth, fHeight);
	if ( iRet != XUI_OK ) return iRet;
	__xuiCanvasCancelPen(pWidget, pData);
	return XUI_OK;
}

XUI_API int xuiCanvasGetViewportHint(xui_widget pWidget, float* pWidth, float* pHeight)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiScrollFrameGetViewportHint(pData->pFrame, pWidth, pHeight);
}

XUI_API int xuiCanvasSetOffset(xui_widget pWidget, float fOffsetX, float fOffsetY)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	float fOldX;
	float fOldY;
	int iRet;

	if ( (pData == NULL) || (pData->pFrame == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiScrollFrameGetOffset(pData->pFrame, &fOldX, &fOldY);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiScrollFrameSetOffset(pData->pFrame, fOffsetX, fOffsetY);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiCanvasFinishScroll(pWidget, pData, fOldX, fOldY);
}

XUI_API int xuiCanvasScrollBy(xui_widget pWidget, float fDeltaX, float fDeltaY)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	float fOffsetX;
	float fOffsetY;
	int iRet;

	if ( (pData == NULL) || (pData->pFrame == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiScrollFrameGetOffset(pData->pFrame, &fOffsetX, &fOffsetY);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiScrollFrameScrollBy(pData->pFrame, fDeltaX, fDeltaY);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiCanvasFinishScroll(pWidget, pData, fOffsetX, fOffsetY);
}

XUI_API int xuiCanvasGetOffset(xui_widget pWidget, float* pOffsetX, float* pOffsetY)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	return (pData != NULL && pData->pFrame != NULL) ? xuiScrollFrameGetOffset(pData->pFrame, pOffsetX, pOffsetY) : XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API int xuiCanvasEnsureRectVisible(xui_widget pWidget, xui_rect_t tContentRect)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	float fOldX;
	float fOldY;
	int iRet;

	if ( (pData == NULL) || (pData->pFrame == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiScrollFrameGetOffset(pData->pFrame, &fOldX, &fOldY);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiScrollFrameEnsureRectVisible(pData->pFrame, tContentRect);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiCanvasFinishScroll(pWidget, pData, fOldX, fOldY);
}

XUI_API int xuiCanvasClear(xui_widget pWidget, uint32_t iColor)
{
	xui_canvas_data_t* pData;
	xui_proxy pProxy;
	xui_surface pSurface;
	int iRet;

	pData = __xuiCanvasGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxy = __xuiCanvasProxy(pWidget);
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	if ( pData->pSurface == NULL ) {
		pSurface = NULL;
		iRet = __xuiCanvasCreateSurface(pWidget, pData->fCanvasWidth, pData->fCanvasHeight,
			iColor, 1, &pSurface);
		if ( iRet != XUI_OK ) return iRet;
		pData->pSurface = pSurface;
	} else {
		iRet = __xuiCanvasClearSurface(pProxy, pData->pSurface, pData->fCanvasWidth,
			pData->fCanvasHeight, iColor);
	}
	if ( iRet == XUI_OK ) {
		__xuiCanvasRememberSurfaceGeneration(pWidget, pData);
		pData->iClearColor = iColor;
		pData->iDrawCount++;
		iRet = __xuiCanvasInvalidateViewport(pData);
	}
	return iRet;
}

typedef struct xui_canvas_clear_rect_args_t { xui_rect_t tRect; uint32_t iColor; } xui_canvas_clear_rect_args_t;
static int __xuiCanvasDoClearRect(xui_proxy pProxy, xui_draw_context pDraw, void* pUser)
{
	xui_canvas_clear_rect_args_t* pArgs = (xui_canvas_clear_rect_args_t*)pUser;
	return (pProxy->drawClearRect != NULL) ? pProxy->drawClearRect(pProxy, pDraw, pArgs->tRect, pArgs->iColor) : XUI_ERROR_UNSUPPORTED;
}
XUI_API int xuiCanvasClearRect(xui_widget pWidget, xui_rect_t tRect, uint32_t iColor)
{
	xui_canvas_clear_rect_args_t tArgs;
	if ( !__xuiCanvasRectValid(&tRect) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( tRect.fW == 0.0f || tRect.fH == 0.0f ) return __xuiCanvasNoDraw(pWidget);
	tArgs.tRect = tRect;
	tArgs.iColor = iColor;
	return __xuiCanvasWithDraw(pWidget, __xuiCanvasDoClearRect, &tArgs);
}

typedef struct xui_canvas_surface_args_t { xui_surface pSurface; xui_rect_t tSrc; xui_rect_t tDst; uint32_t iColor; uint32_t iFlags; } xui_canvas_surface_args_t;
static int __xuiCanvasDoSurface(xui_proxy pProxy, xui_draw_context pDraw, void* pUser)
{
	xui_canvas_surface_args_t* pArgs = (xui_canvas_surface_args_t*)pUser;
	if ( (__xuiCanvasAlpha(pArgs->iColor) == 0) || (pArgs->tSrc.fW <= 0) ||
	     (pArgs->tSrc.fH <= 0) || (pArgs->tDst.fW <= 0) || (pArgs->tDst.fH <= 0) ) {
		return XUI_OK;
	}
	return (pProxy->drawSurface != NULL) ? pProxy->drawSurface(pProxy, pDraw, pArgs->pSurface, pArgs->tSrc, pArgs->tDst, pArgs->iColor, pArgs->iFlags) : XUI_ERROR_UNSUPPORTED;
}
XUI_API int xuiCanvasDrawSurface(xui_widget pWidget, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags)
{
	xui_canvas_surface_args_t tArgs;
	if ( (pSurface == NULL) || !__xuiCanvasRectValid(&tSrc) ||
	     !__xuiCanvasRectValid(&tDst) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (__xuiCanvasAlpha(iColor) == 0) || (tSrc.fW == 0.0f) ||
	     (tSrc.fH == 0.0f) || (tDst.fW == 0.0f) || (tDst.fH == 0.0f) ) {
		return __xuiCanvasNoDraw(pWidget);
	}
	tArgs.pSurface = pSurface; tArgs.tSrc = tSrc; tArgs.tDst = tDst; tArgs.iColor = iColor; tArgs.iFlags = iFlags;
	return __xuiCanvasWithDraw(pWidget, __xuiCanvasDoSurface, &tArgs);
}

typedef struct xui_canvas_quad_args_t { xui_surface pSurface; const xui_surface_vertex_t* pVertices; uint32_t iFlags; } xui_canvas_quad_args_t;
static int __xuiCanvasDoSurfaceQuad(xui_proxy pProxy, xui_draw_context pDraw, void* pUser)
{
	xui_canvas_quad_args_t* pArgs = (xui_canvas_quad_args_t*)pUser;
	if ( ((pArgs->pVertices[0].iColor | pArgs->pVertices[1].iColor | pArgs->pVertices[2].iColor | pArgs->pVertices[3].iColor) & 0xffu) == 0u ) {
		return XUI_OK;
	}
	return (pProxy->drawSurfaceQuad != NULL) ? pProxy->drawSurfaceQuad(pProxy, pDraw, pArgs->pSurface, pArgs->pVertices, pArgs->iFlags) : XUI_ERROR_UNSUPPORTED;
}
XUI_API int xuiCanvasDrawSurfaceQuad(xui_widget pWidget, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags)
{
	xui_canvas_quad_args_t tArgs;
	uint32_t iAlphaMask;
	int i;
	if ( (pSurface == NULL) || (pVertices == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iAlphaMask = 0u;
	for ( i = 0; i < 4; i++ ) {
		if ( !isfinite(pVertices[i].fX) || !isfinite(pVertices[i].fY) ||
		     !isfinite(pVertices[i].fZ) || !isfinite(pVertices[i].fW) ||
		     !isfinite(pVertices[i].fU) || !isfinite(pVertices[i].fV) ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		iAlphaMask |= pVertices[i].iColor;
	}
	if ( (iAlphaMask & 0xffu) == 0u ) return __xuiCanvasNoDraw(pWidget);
	tArgs.pSurface = pSurface; tArgs.pVertices = pVertices; tArgs.iFlags = iFlags;
	return __xuiCanvasWithDraw(pWidget, __xuiCanvasDoSurfaceQuad, &tArgs);
}

typedef struct xui_canvas_mesh_args_t { const xui_mesh_vertex_t* pVertices; int iVertexCount; const uint32_t* pIndices; int iIndexCount; uint32_t iFlags; } xui_canvas_mesh_args_t;
static int __xuiCanvasDoMesh(xui_proxy pProxy, xui_draw_context pDraw, void* pUser)
{
	xui_canvas_mesh_args_t* pArgs = (xui_canvas_mesh_args_t*)pUser;
	int i;
	uint32_t iAlphaMask;

	iAlphaMask = 0u;
	for ( i = 0; i < pArgs->iVertexCount; i++ ) {
		iAlphaMask |= pArgs->pVertices[i].iColor;
	}
	if ( (iAlphaMask & 0xffu) == 0u ) {
		return XUI_OK;
	}
	return (pProxy->drawMeshTriangles != NULL) ? pProxy->drawMeshTriangles(pProxy, pDraw, pArgs->pVertices, pArgs->iVertexCount, pArgs->pIndices, pArgs->iIndexCount, pArgs->iFlags) : XUI_ERROR_UNSUPPORTED;
}
XUI_API int xuiCanvasDrawMeshTriangles(xui_widget pWidget, const xui_mesh_vertex_t* pVertices, int iVertexCount, const uint32_t* pIndices, int iIndexCount, uint32_t iFlags)
{
	xui_canvas_mesh_args_t tArgs;
	uint32_t iAlphaMask;
	int i;
	if ( (pVertices == NULL) || (iVertexCount <= 0) || (pIndices == NULL) ||
	     (iIndexCount <= 0) || ((iIndexCount % 3) != 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	iAlphaMask = 0u;
	for ( i = 0; i < iVertexCount; i++ ) {
		if ( !isfinite(pVertices[i].fX) || !isfinite(pVertices[i].fY) ) return XUI_ERROR_INVALID_ARGUMENT;
		iAlphaMask |= pVertices[i].iColor;
	}
	for ( i = 0; i < iIndexCount; i++ ) {
		if ( pIndices[i] >= (uint32_t)iVertexCount ) return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (iAlphaMask & 0xffu) == 0u ) return __xuiCanvasNoDraw(pWidget);
	tArgs.pVertices = pVertices; tArgs.iVertexCount = iVertexCount; tArgs.pIndices = pIndices; tArgs.iIndexCount = iIndexCount; tArgs.iFlags = iFlags;
	return __xuiCanvasWithDraw(pWidget, __xuiCanvasDoMesh, &tArgs);
}

typedef struct xui_canvas_line_args_t { float fX0; float fY0; float fX1; float fY1; float fWidth; uint32_t iColor; } xui_canvas_line_args_t;
static int __xuiCanvasDoPoint(xui_proxy pProxy, xui_draw_context pDraw, void* pUser)
{
	xui_canvas_line_args_t* pArgs = (xui_canvas_line_args_t*)pUser;
	if ( (pArgs->fWidth <= 0.0f) || (__xuiCanvasAlpha(pArgs->iColor) == 0) ) {
		return XUI_OK;
	}
	return (pProxy->drawPoint != NULL) ? pProxy->drawPoint(pProxy, pDraw, pArgs->fX0, pArgs->fY0, pArgs->fWidth, pArgs->iColor) : XUI_ERROR_UNSUPPORTED;
}
static int __xuiCanvasDoLine(xui_proxy pProxy, xui_draw_context pDraw, void* pUser)
{
	xui_canvas_line_args_t* pArgs = (xui_canvas_line_args_t*)pUser;
	if ( (pArgs->fWidth <= 0.0f) || (__xuiCanvasAlpha(pArgs->iColor) == 0) ) {
		return XUI_OK;
	}
	return (pProxy->drawLine != NULL) ? pProxy->drawLine(pProxy, pDraw, pArgs->fX0, pArgs->fY0, pArgs->fX1, pArgs->fY1, pArgs->fWidth, pArgs->iColor) : XUI_ERROR_UNSUPPORTED;
}
XUI_API int xuiCanvasDrawPoint(xui_widget pWidget, float fX, float fY, float fSize, uint32_t iColor)
{
	xui_canvas_line_args_t tArgs = {fX, fY, 0.0f, 0.0f, fSize, iColor};
	if ( !isfinite(fX) || !isfinite(fY) || !isfinite(fSize) || (fSize < 0.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( fSize == 0.0f || __xuiCanvasAlpha(iColor) == 0 ) return __xuiCanvasNoDraw(pWidget);
	return __xuiCanvasWithDraw(pWidget, __xuiCanvasDoPoint, &tArgs);
}
XUI_API int xuiCanvasDrawLine(xui_widget pWidget, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor)
{
	xui_canvas_line_args_t tArgs = {fX0, fY0, fX1, fY1, fWidth, iColor};
	if ( !isfinite(fX0) || !isfinite(fY0) || !isfinite(fX1) || !isfinite(fY1) ||
	     !isfinite(fWidth) || (fWidth < 0.0f) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( fWidth == 0.0f || __xuiCanvasAlpha(iColor) == 0 ) return __xuiCanvasNoDraw(pWidget);
	return __xuiCanvasWithDraw(pWidget, __xuiCanvasDoLine, &tArgs);
}

typedef struct xui_canvas_triangle_args_t { xui_vec2_t tA; xui_vec2_t tB; xui_vec2_t tC; float fWidth; uint32_t iColor; } xui_canvas_triangle_args_t;
static int __xuiCanvasDoTriangleFill(xui_proxy pProxy, xui_draw_context pDraw, void* pUser)
{
	xui_canvas_triangle_args_t* pArgs = (xui_canvas_triangle_args_t*)pUser;
	if ( __xuiCanvasAlpha(pArgs->iColor) == 0 ) {
		return XUI_OK;
	}
	return (pProxy->drawTriangleFill != NULL) ? pProxy->drawTriangleFill(pProxy, pDraw, pArgs->tA, pArgs->tB, pArgs->tC, pArgs->iColor) : XUI_ERROR_UNSUPPORTED;
}
static int __xuiCanvasDoTriangleStroke(xui_proxy pProxy, xui_draw_context pDraw, void* pUser)
{
	xui_canvas_triangle_args_t* pArgs = (xui_canvas_triangle_args_t*)pUser;
	if ( (pArgs->fWidth <= 0.0f) || (__xuiCanvasAlpha(pArgs->iColor) == 0) ) {
		return XUI_OK;
	}
	return (pProxy->drawTriangleStroke != NULL) ? pProxy->drawTriangleStroke(pProxy, pDraw, pArgs->tA, pArgs->tB, pArgs->tC, pArgs->fWidth, pArgs->iColor) : XUI_ERROR_UNSUPPORTED;
}
XUI_API int xuiCanvasDrawTriangleFill(xui_widget pWidget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor)
{
	xui_canvas_triangle_args_t tArgs = {tA, tB, tC, 0.0f, iColor};
	if ( !isfinite(tA.fX) || !isfinite(tA.fY) || !isfinite(tB.fX) ||
	     !isfinite(tB.fY) || !isfinite(tC.fX) || !isfinite(tC.fY) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( __xuiCanvasAlpha(iColor) == 0 ) return __xuiCanvasNoDraw(pWidget);
	return __xuiCanvasWithDraw(pWidget, __xuiCanvasDoTriangleFill, &tArgs);
}
XUI_API int xuiCanvasDrawTriangleStroke(xui_widget pWidget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor)
{
	xui_canvas_triangle_args_t tArgs = {tA, tB, tC, fWidth, iColor};
	if ( !isfinite(tA.fX) || !isfinite(tA.fY) || !isfinite(tB.fX) ||
	     !isfinite(tB.fY) || !isfinite(tC.fX) || !isfinite(tC.fY) ||
	     !isfinite(fWidth) || (fWidth < 0.0f) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( fWidth == 0.0f || __xuiCanvasAlpha(iColor) == 0 ) return __xuiCanvasNoDraw(pWidget);
	return __xuiCanvasWithDraw(pWidget, __xuiCanvasDoTriangleStroke, &tArgs);
}

typedef struct xui_canvas_rect_args_t { xui_rect_t tRect; float fA; float fB; uint32_t iColor; } xui_canvas_rect_args_t;
static int __xuiCanvasDoRectFill(xui_proxy pProxy, xui_draw_context pDraw, void* pUser)
{
	xui_canvas_rect_args_t* pArgs = (xui_canvas_rect_args_t*)pUser;
	if ( (pArgs->tRect.fW <= 0.0f) || (pArgs->tRect.fH <= 0.0f) || (__xuiCanvasAlpha(pArgs->iColor) == 0) ) {
		return XUI_OK;
	}
	return (pProxy->drawRectFill != NULL) ? pProxy->drawRectFill(pProxy, pDraw, pArgs->tRect, pArgs->iColor) : XUI_ERROR_UNSUPPORTED;
}
static int __xuiCanvasDoRectStroke(xui_proxy pProxy, xui_draw_context pDraw, void* pUser)
{
	xui_canvas_rect_args_t* pArgs = (xui_canvas_rect_args_t*)pUser;
	if ( (pArgs->tRect.fW <= 0.0f) || (pArgs->tRect.fH <= 0.0f) || (pArgs->fA <= 0.0f) || (__xuiCanvasAlpha(pArgs->iColor) == 0) ) {
		return XUI_OK;
	}
	return (pProxy->drawRectStroke != NULL) ? pProxy->drawRectStroke(pProxy, pDraw, pArgs->tRect, pArgs->fA, pArgs->iColor) : XUI_ERROR_UNSUPPORTED;
}
XUI_API int xuiCanvasDrawRectFill(xui_widget pWidget, xui_rect_t tRect, uint32_t iColor)
{
	xui_canvas_rect_args_t tArgs = {tRect, 0.0f, 0.0f, iColor};
	if ( !__xuiCanvasRectValid(&tRect) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( tRect.fW == 0.0f || tRect.fH == 0.0f || __xuiCanvasAlpha(iColor) == 0 ) {
		return __xuiCanvasNoDraw(pWidget);
	}
	return __xuiCanvasWithDraw(pWidget, __xuiCanvasDoRectFill, &tArgs);
}
XUI_API int xuiCanvasDrawRectStroke(xui_widget pWidget, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	xui_canvas_rect_args_t tArgs = {tRect, fWidth, 0.0f, iColor};
	if ( !__xuiCanvasRectValid(&tRect) || !isfinite(fWidth) || (fWidth < 0.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( tRect.fW == 0.0f || tRect.fH == 0.0f || fWidth == 0.0f ||
	     __xuiCanvasAlpha(iColor) == 0 ) return __xuiCanvasNoDraw(pWidget);
	return __xuiCanvasWithDraw(pWidget, __xuiCanvasDoRectStroke, &tArgs);
}
typedef struct xui_canvas_circle_args_t { float fX; float fY; float fRadius; float fWidth; uint32_t iColor; } xui_canvas_circle_args_t;
static int __xuiCanvasDoCircleFill(xui_proxy pProxy, xui_draw_context pDraw, void* pUser)
{
	xui_canvas_circle_args_t* pArgs = (xui_canvas_circle_args_t*)pUser;
	if ( (pArgs->fRadius <= 0.0f) || (__xuiCanvasAlpha(pArgs->iColor) == 0) ) {
		return XUI_OK;
	}
	return (pProxy->drawCircleFill != NULL) ? pProxy->drawCircleFill(pProxy, pDraw, pArgs->fX, pArgs->fY, pArgs->fRadius, pArgs->iColor) : XUI_ERROR_UNSUPPORTED;
}
static int __xuiCanvasDoCircleStroke(xui_proxy pProxy, xui_draw_context pDraw, void* pUser)
{
	xui_canvas_circle_args_t* pArgs = (xui_canvas_circle_args_t*)pUser;
	if ( (pArgs->fRadius <= 0.0f) || (pArgs->fWidth <= 0.0f) || (__xuiCanvasAlpha(pArgs->iColor) == 0) ) {
		return XUI_OK;
	}
	return (pProxy->drawCircleStroke != NULL) ? pProxy->drawCircleStroke(pProxy, pDraw, pArgs->fX, pArgs->fY, pArgs->fRadius, pArgs->fWidth, pArgs->iColor) : XUI_ERROR_UNSUPPORTED;
}
XUI_API int xuiCanvasDrawCircleFill(xui_widget pWidget, float fX, float fY, float fRadius, uint32_t iColor)
{
	xui_canvas_circle_args_t tArgs = {fX, fY, fRadius, 0.0f, iColor};
	if ( !isfinite(fX) || !isfinite(fY) || !isfinite(fRadius) || (fRadius < 0.0f) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( fRadius == 0.0f || __xuiCanvasAlpha(iColor) == 0 ) return __xuiCanvasNoDraw(pWidget);
	return __xuiCanvasWithDraw(pWidget, __xuiCanvasDoCircleFill, &tArgs);
}
XUI_API int xuiCanvasDrawCircleStroke(xui_widget pWidget, float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	xui_canvas_circle_args_t tArgs = {fX, fY, fRadius, fWidth, iColor};
	if ( !isfinite(fX) || !isfinite(fY) || !isfinite(fRadius) || !isfinite(fWidth) ||
	     (fRadius < 0.0f) || (fWidth < 0.0f) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( fRadius == 0.0f || fWidth == 0.0f || __xuiCanvasAlpha(iColor) == 0 ) {
		return __xuiCanvasNoDraw(pWidget);
	}
	return __xuiCanvasWithDraw(pWidget, __xuiCanvasDoCircleStroke, &tArgs);
}

typedef struct xui_canvas_text_args_t { xui_font pFont; const char* sText; xui_rect_t tRect; uint32_t iColor; uint32_t iFlags; } xui_canvas_text_args_t;
static int __xuiCanvasDoText(xui_proxy pProxy, xui_draw_context pDraw, void* pUser)
{
	xui_canvas_text_args_t* pArgs = (xui_canvas_text_args_t*)pUser;
	if ( (pArgs->sText[0] == '\0') || (pArgs->tRect.fW <= 0.0f) || (pArgs->tRect.fH <= 0.0f) || (__xuiCanvasAlpha(pArgs->iColor) == 0) ) {
		return XUI_OK;
	}
	return (pProxy->drawText != NULL) ? pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pArgs->pFont, .sText=pArgs->sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((pArgs->iFlags) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, pArgs->tRect, pArgs->iColor, pArgs->iFlags) : XUI_ERROR_UNSUPPORTED;
}
XUI_API int xuiCanvasDrawText(xui_widget pWidget, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	xui_canvas_text_args_t tArgs;
	if ( (pFont == NULL) || (sText == NULL) || !__xuiCanvasRectValid(&tRect) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( sText[0] == '\0' || tRect.fW == 0.0f || tRect.fH == 0.0f ||
	     __xuiCanvasAlpha(iColor) == 0 ) return __xuiCanvasNoDraw(pWidget);
	tArgs.pFont = pFont; tArgs.sText = sText; tArgs.tRect = tRect; tArgs.iColor = iColor; tArgs.iFlags = iFlags;
	return __xuiCanvasWithDraw(pWidget, __xuiCanvasDoText, &tArgs);
}

XUI_API int xuiCanvasSetPen(xui_widget pWidget, int bEnabled, float fWidth, uint32_t iColor)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	if ( (pData == NULL) || !isfinite(fWidth) || (fWidth < 0.0f) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !bEnabled ) __xuiCanvasCancelPen(pWidget, pData);
	pData->bPenEnabled = (bEnabled != 0);
	if ( fWidth > 0.0f ) pData->fPenWidth = fWidth;
	pData->iPenColor = iColor;
	pData->bPenColorExplicit = 1;
	return XUI_OK;
}

XUI_API int xuiCanvasGetPen(xui_widget pWidget, int* pEnabled, float* pWidth, uint32_t* pColor)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEnabled != NULL ) *pEnabled = pData->bPenEnabled;
	if ( pWidth != NULL ) *pWidth = pData->fPenWidth;
	if ( pColor != NULL ) *pColor = pData->iPenColor;
	return XUI_OK;
}

XUI_API int xuiCanvasSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iBorder, uint32_t iPen)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iBackgroundColor == iBackground && pData->iBorderColor == iBorder &&
	     pData->iPenColor == iPen && pData->bPenColorExplicit ) return XUI_OK;
	iRet = __xuiCanvasInvalidateViewport(pData);
	if ( iRet != XUI_OK ) return iRet;
	pData->iBackgroundColor = iBackground;
	pData->iBorderColor = iBorder;
	pData->iPenColor = iPen;
	pData->bPenColorExplicit = 1;
	return XUI_OK;
}

XUI_API int xuiCanvasGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pBorder, uint32_t* pPen)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBackground != NULL ) *pBackground = pData->iBackgroundColor;
	if ( pBorder != NULL ) *pBorder = pData->iBorderColor;
	if ( pPen != NULL ) *pPen = pData->iPenColor;
	return XUI_OK;
}

XUI_API int xuiCanvasSetScrollbarPolicy(xui_widget pWidget, int iPolicyX, int iPolicyY)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	int iOldPolicyX;
	int iOldPolicyY;
	int iRet;
	if ( (pData == NULL) || (pData->pFrame == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiScrollFrameGetScrollbarPolicy(pData->pFrame, &iOldPolicyX, &iOldPolicyY);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiScrollFrameSetScrollbarPolicy(pData->pFrame, iPolicyX, iPolicyY);
	if ( iRet == XUI_OK && (iOldPolicyX != iPolicyX || iOldPolicyY != iPolicyY) ) {
		__xuiCanvasCancelPen(pWidget, pData);
	}
	return iRet;
}

XUI_API int xuiCanvasSetScrollbarMode(xui_widget pWidget, int iMode)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	int iOldMode;
	int iRet;
	if ( (pData == NULL) || (pData->pFrame == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iOldMode = xuiScrollFrameGetScrollbarMode(pData->pFrame);
	iRet = xuiScrollFrameSetScrollbarMode(pData->pFrame, iMode);
	if ( iRet == XUI_OK && iOldMode != iMode ) __xuiCanvasCancelPen(pWidget, pData);
	return iRet;
}

XUI_API int xuiCanvasGetChangeCount(xui_widget pWidget)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

XUI_API int xuiCanvasGetDrawCount(xui_widget pWidget)
{
	xui_canvas_data_t* pData = __xuiCanvasGetData(pWidget);
	return (pData != NULL) ? pData->iDrawCount : 0;
}

#endif
