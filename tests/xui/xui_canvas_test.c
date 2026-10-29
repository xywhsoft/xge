#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_canvas_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiCanvasNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) fD = -fD;
	return fD < 0.01f;
}

static int __xuiCanvasRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 360, 240};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pCanvas;
	xui_widget pLazyCanvas;
	xui_widget pViewport;
	xui_surface pTarget;
	xui_surface pCanvasSurface;
	xui_surface pViewportCache;
	xui_surface pOldSurface;
	xui_canvas_desc_t tDesc;
	xui_canvas_desc_t tInvalidDesc;
	xui_event_t tEvent;
	xui_rect_t tViewportWorld;
	xui_rect_t tViewportContent;
	xui_vec2_t tMeasured;
	xui_mesh_vertex_t arrVertices[3];
	xui_surface_vertex_t arrQuad[4];
	uint32_t arrIndices[3];
	float fWidth;
	float fHeight;
	float fOffsetX;
	float fOffsetY;
	float fMaxX;
	float fMaxY;
	int bPenEnabled;
	float fPenWidth;
	uint32_t iPenColor;
	int iDrawCount;
	int iSurfaceDestroyCount;
	int iRet;
	int iFailed;

	pContext = NULL;
	pRoot = NULL;
	pCanvas = NULL;
	pLazyCanvas = NULL;
	pTarget = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 360.0f, 240.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 360.0f, 240.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");
	pCanvas = (xui_widget)(uintptr_t)1;
	iRet = xuiCanvasCreate(NULL, &pCanvas, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pCanvas == NULL,
		"reject invalid context");

	memset(&tInvalidDesc, 0, sizeof(tInvalidDesc));
	tInvalidDesc.iSize = sizeof(tInvalidDesc);
	tInvalidDesc.fCanvasWidth = NAN;
	iRet = xuiCanvasCreate(pContext, &pCanvas, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pCanvas == NULL, "reject NaN canvas descriptor");
	tInvalidDesc.fCanvasWidth = INFINITY;
	iRet = xuiCanvasCreate(pContext, &pCanvas, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pCanvas == NULL, "reject infinite canvas descriptor");
	tInvalidDesc.fCanvasWidth = 32768.0f;
	iRet = xuiCanvasCreate(pContext, &pCanvas, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pCanvas == NULL, "reject oversized canvas descriptor");
	tInvalidDesc.fCanvasWidth = 320.0f;
	tInvalidDesc.fCanvasHeight = 240.0f;
	tInvalidDesc.fWheelStep = NAN;
	iRet = xuiCanvasCreate(pContext, &pCanvas, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pCanvas == NULL, "reject NaN canvas metric");
	tInvalidDesc.fWheelStep = 0.0f;
	tInvalidDesc.iScrollbarMode = 99;
	pCanvas = (xui_widget)(uintptr_t)1;
	iRet = xuiCanvasCreate(pContext, &pCanvas, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pCanvas == NULL, "reject invalid mode and clear output");

	memset(&tInvalidDesc, 0, sizeof(tInvalidDesc));
	tInvalidDesc.iSize = sizeof(tInvalidDesc);
	tInvalidDesc.fCanvasWidth = 100.4f;
	tInvalidDesc.fCanvasHeight = 80.6f;
	tInvalidDesc.iPolicyX = XUI_SCROLLBAR_POLICY_AUTO;
	tInvalidDesc.iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
	tInvalidDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	tInvalidDesc.iWheelAxis = XUI_WHEEL_AXIS_BOTH;
	tInvalidDesc.iCornerMode = XUI_SCROLL_FRAME_CORNER_NONE;
	iRet = xuiCanvasCreate(pContext, &pLazyCanvas, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pLazyCanvas != NULL, "lazy canvas create");
	iRet = xuiCanvasGetCanvasSize(pLazyCanvas, &fWidth, &fHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiCanvasNear(fWidth, 100.0f) &&
		__xuiCanvasNear(fHeight, 81.0f), "fractional canvas dimensions canonicalized");
	xuiTestProxyFailNextSurfaceCreate(&tState, XUI_ERROR_OUT_OF_MEMORY);
	iRet = xuiCanvasSetCanvasSize(pLazyCanvas, 120.4f, 90.6f);
	XUI_TEST_CHECK(iRet == XUI_OK, "untouched canvas resize stays lazy");
	iRet = xuiCanvasDrawPoint(pLazyCanvas, 1.0f, 1.0f, 2.0f, XUI_COLOR_RGBA(1, 2, 3, 0));
	XUI_TEST_CHECK(iRet == XUI_OK, "transparent draw stays lazy");
	XUI_TEST_CHECK(xuiCanvasGetSurface(pLazyCanvas) == NULL, "lazy operations preserve pending create failure");
	XUI_TEST_CHECK(xuiCanvasGetSurface(pLazyCanvas) != NULL, "lazy surface creates on demand");
	xuiWidgetDestroy(pLazyCanvas);
	pLazyCanvas = NULL;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fCanvasWidth = 480.0f;
	tDesc.fCanvasHeight = 320.0f;
	tDesc.fViewportWidth = 180.0f;
	tDesc.fViewportHeight = 140.0f;
	tDesc.iPolicyX = XUI_SCROLLBAR_POLICY_AUTO;
	tDesc.iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
	tDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
	tDesc.iWheelAxis = XUI_WHEEL_AXIS_BOTH;
	tDesc.iCornerMode = XUI_SCROLL_FRAME_CORNER_GRIP;
	tDesc.fScrollbarSize = 16.0f;
	tDesc.iClearColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	tDesc.iPenColor = XUI_COLOR_RGBA(20, 90, 210, 255);
	iRet = xuiCanvasCreate(pContext, &pCanvas, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pCanvas != NULL, "canvas create");
	XUI_TEST_CHECK(xuiScrollFrameGetScrollbarMode(xuiCanvasGetFrameWidget(pCanvas)) ==
		XUI_SCROLLBAR_MODE_FULL, "canvas delegates full scrollbar mode to scroll frame");
	iRet = xuiWidgetMeasure(pCanvas, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiCanvasNear(tMeasured.fX, 180.0f) && __xuiCanvasNear(tMeasured.fY, 140.0f), "canvas extent does not define viewport");
	iRet = xuiWidgetAddChild(pRoot, pCanvas);
	XUI_TEST_CHECK(iRet == XUI_OK, "add canvas");
	xuiWidgetSetRect(pCanvas, (xui_rect_t){12.0f, 16.0f, 180.0f, 140.0f});

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 360, 240, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiCanvasRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "initial render");

	iRet = xuiCanvasGetCanvasSize(pCanvas, &fWidth, &fHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiCanvasNear(fWidth, 480.0f) && __xuiCanvasNear(fHeight, 320.0f), "canvas size");
	XUI_TEST_CHECK(xuiCanvasGetFrameWidget(pCanvas) != NULL && xuiCanvasGetViewportWidget(pCanvas) != NULL, "child accessors");
	pCanvasSurface = xuiCanvasGetSurface(pCanvas);
	XUI_TEST_CHECK(pCanvasSurface != NULL, "surface accessor");
	pViewport = xuiCanvasGetViewportWidget(pCanvas);
	pViewportCache = xuiWidgetGetCacheSurface(pViewport,
		xuiWidgetGetStateId(pViewport));
	XUI_TEST_CHECK(pViewportCache != NULL, "viewport cache surface");
	xuiTestSurfaceReset(pViewportCache);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pViewportCache) == 0,
		"unchanged canvas surface reuses viewport cache");
	iRet = tState.tProxy.surfaceUpdateRGBA(&tState.tProxy, pCanvasSurface,
		(xui_rect_i_t){0, 0, 1, 1}, NULL, 0);
	if ( iRet == XUI_OK ) iRet = xuiRenderPrepare(pContext);
	pViewportCache = xuiWidgetGetCacheSurface(pViewport,
		xuiWidgetGetStateId(pViewport));
	XUI_TEST_CHECK(iRet == XUI_OK && pViewportCache != NULL &&
		xuiTestSurfaceGetDrawCount(pViewportCache) > 0,
		"external canvas surface update rebuilds viewport cache");
	pOldSurface = pCanvasSurface;
	xuiTestProxyFailNextSurfaceCreate(&tState, XUI_ERROR_OUT_OF_MEMORY);
	iRet = xuiCanvasSetCanvasSize(pCanvas, 300.0f, 220.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_OUT_OF_MEMORY, "resize propagates surface creation failure");
	iRet = xuiCanvasGetCanvasSize(pCanvas, &fWidth, &fHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiCanvasNear(fWidth, 480.0f) && __xuiCanvasNear(fHeight, 320.0f), "failed resize preserves dimensions");
	XUI_TEST_CHECK(xuiCanvasGetSurface(pCanvas) == pOldSurface, "failed resize preserves surface");
	iSurfaceDestroyCount = xuiTestProxyGetSurfaceDestroyCount(&tState);
	xuiTestProxySetSurfaceClearUnsupported(&tState, 1);
	xuiTestProxyFailNextDrawEnd(&tState, XUI_ERROR_UNSUPPORTED);
	iRet = xuiCanvasSetCanvasSize(pCanvas, 301.0f, 221.0f);
	xuiTestProxySetSurfaceClearUnsupported(&tState, 0);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED, "resize propagates temporary surface clear failure");
	XUI_TEST_CHECK(xuiTestProxyGetSurfaceDestroyCount(&tState) == iSurfaceDestroyCount + 1, "failed resize destroys temporary surface");
	XUI_TEST_CHECK(xuiCanvasGetSurface(pCanvas) == pOldSurface, "clear failure preserves old surface");
	iRet = xuiCanvasGetCanvasSize(pCanvas, &fWidth, &fHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiCanvasNear(fWidth, 480.0f) && __xuiCanvasNear(fHeight, 320.0f), "clear failure preserves dimensions");
	XUI_TEST_CHECK(xuiCanvasSetCanvasSize(pCanvas, NAN, 220.0f) == XUI_ERROR_INVALID_ARGUMENT, "reject NaN resize");
	XUI_TEST_CHECK(xuiCanvasSetCanvasSize(pCanvas, 32768.0f, 220.0f) == XUI_ERROR_INVALID_ARGUMENT, "reject oversized resize");

	iRet = xuiCanvasClear(pCanvas, XUI_COLOR_RGBA(250, 250, 250, 255));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetLastColor(pCanvasSurface) == XUI_COLOR_RGBA(250, 250, 250, 255), "clear");
	xuiTestProxySetSurfaceClearUnsupported(&tState, 1);
	iDrawCount = xuiCanvasGetDrawCount(pCanvas);
	iRet = xuiCanvasClear(pCanvas, XUI_COLOR_RGBA(245, 245, 245, 255));
	xuiTestProxySetSurfaceClearUnsupported(&tState, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCanvasGetDrawCount(pCanvas) == iDrawCount + 1, "fallback clear counts once");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastColor(pCanvasSurface) == XUI_COLOR_RGBA(245, 245, 245, 255), "fallback clear color");
	xuiTestProxyFailNextDrawEnd(&tState, XUI_ERROR_UNSUPPORTED);
	iDrawCount = xuiCanvasGetDrawCount(pCanvas);
	iRet = xuiCanvasDrawPoint(pCanvas, 2.0f, 3.0f, 1.0f, XUI_COLOR_WHITE);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED && xuiCanvasGetDrawCount(pCanvas) == iDrawCount, "draw end failure is propagated");
	iRet = xuiCanvasDrawRectFill(pCanvas, (xui_rect_t){20.0f, 24.0f, 80.0f, 50.0f}, XUI_COLOR_RGBA(230, 80, 90, 255));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCanvasGetDrawCount(pCanvas) >= 2, "rect fill");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillColorCount(pCanvasSurface, XUI_COLOR_RGBA(230, 80, 90, 255)) == 1, "rect color count");
	iRet = xuiCanvasDrawLine(pCanvas, 12.0f, 12.0f, 120.0f, 90.0f, 4.0f, XUI_COLOR_RGBA(20, 90, 210, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "line");
	iRet = xuiCanvasDrawCircleStroke(pCanvas, 150.0f, 90.0f, 24.0f, 3.0f, XUI_COLOR_RGBA(30, 160, 110, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "circle stroke");
	XUI_TEST_CHECK(xuiCanvasDrawPoint(pCanvas, NAN, 1.0f, 1.0f, XUI_COLOR_WHITE) == XUI_ERROR_INVALID_ARGUMENT, "reject NaN point");
	XUI_TEST_CHECK(xuiCanvasDrawLine(pCanvas, 0.0f, 0.0f, INFINITY, 1.0f, 1.0f, XUI_COLOR_WHITE) == XUI_ERROR_INVALID_ARGUMENT, "reject infinite line");
	XUI_TEST_CHECK(xuiCanvasDrawCircleFill(pCanvas, 1.0f, 1.0f, -1.0f, XUI_COLOR_WHITE) == XUI_ERROR_INVALID_ARGUMENT, "reject negative circle radius");
	XUI_TEST_CHECK(xuiCanvasDrawRectFill(pCanvas, (xui_rect_t){0, 0, -1, 1}, XUI_COLOR_WHITE) == XUI_ERROR_INVALID_ARGUMENT, "reject negative rectangle");
	XUI_TEST_CHECK(xuiCanvasSetPen(pCanvas, 1, NAN, XUI_COLOR_WHITE) == XUI_ERROR_INVALID_ARGUMENT, "reject NaN pen width");
	memset(arrQuad, 0, sizeof(arrQuad));
	arrQuad[2].fU = NAN;
	XUI_TEST_CHECK(xuiCanvasDrawSurfaceQuad(pCanvas, pTarget, arrQuad, 0) == XUI_ERROR_INVALID_ARGUMENT, "reject NaN surface quad");

	arrVertices[0] = (xui_mesh_vertex_t){220.0f, 34.0f, XUI_COLOR_RGBA(40, 80, 180, 255)};
	arrVertices[1] = (xui_mesh_vertex_t){280.0f, 110.0f, XUI_COLOR_RGBA(40, 180, 120, 255)};
	arrVertices[2] = (xui_mesh_vertex_t){190.0f, 120.0f, XUI_COLOR_RGBA(220, 140, 60, 255)};
	arrIndices[0] = 0;
	arrIndices[1] = 1;
	arrIndices[2] = 2;
	iRet = xuiCanvasDrawMeshTriangles(pCanvas, arrVertices, 3, arrIndices, 3, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestProxyGetMeshDrawCount(&tState) == 1, "mesh");
	XUI_TEST_CHECK(xuiCanvasDrawMeshTriangles(pCanvas, arrVertices, 3, arrIndices, 2, 0) == XUI_ERROR_INVALID_ARGUMENT, "reject partial mesh triangle");
	arrIndices[2] = 3;
	XUI_TEST_CHECK(xuiCanvasDrawMeshTriangles(pCanvas, arrVertices, 3, arrIndices, 3, 0) == XUI_ERROR_INVALID_ARGUMENT, "reject out of range mesh index");
	arrIndices[2] = 2;
	arrVertices[1].fX = NAN;
	XUI_TEST_CHECK(xuiCanvasDrawMeshTriangles(pCanvas, arrVertices, 3, arrIndices, 3, 0) == XUI_ERROR_INVALID_ARGUMENT, "reject NaN mesh vertex");
	arrVertices[1].fX = 280.0f;

	iRet = xuiCanvasSetOffset(pCanvas, 120.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set offset");
	iRet = xuiCanvasGetOffset(pCanvas, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiCanvasNear(fOffsetX, 120.0f) && __xuiCanvasNear(fOffsetY, 80.0f), "get offset");
	iRet = xuiCanvasScrollBy(pCanvas, XUI_LAYOUT_UNBOUNDED,
		XUI_LAYOUT_UNBOUNDED);
	if ( iRet == XUI_OK ) iRet = xuiScrollModelGetMaxOffset(
		xuiCanvasGetModel(pCanvas), &fMaxX, &fMaxY);
	if ( iRet == XUI_OK ) iRet = xuiCanvasGetOffset(pCanvas, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && fOffsetX == fMaxX && fOffsetY == fMaxY,
		"wide relative scroll saturates at the canvas boundary");
	iRet = xuiCanvasSetOffset(pCanvas, 120.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore canvas offset after wide scroll");
	iRet = __xuiCanvasRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "scrolled render");
	XUI_TEST_CHECK(xuiTestSurfaceGetDrawCount(pTarget) > 0, "scrolled render draw count");

	iRet = xuiCanvasSetCanvasSize(pCanvas, 260.0f, 200.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "resize");
	iRet = xuiCanvasGetCanvasSize(pCanvas, &fWidth, &fHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiCanvasNear(fWidth, 260.0f) && __xuiCanvasNear(fHeight, 200.0f), "resized size");
	iRet = xuiWidgetMeasure(pCanvas, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiCanvasNear(tMeasured.fX, 180.0f) && __xuiCanvasNear(tMeasured.fY, 140.0f), "canvas resize keeps viewport hint");

	iRet = xuiCanvasSetPen(pCanvas, 1, 5.0f, XUI_COLOR_RGBA(90, 70, 210, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set pen");
	iRet = xuiCanvasGetPen(pCanvas, &bPenEnabled, &fPenWidth, &iPenColor);
	XUI_TEST_CHECK(iRet == XUI_OK && bPenEnabled && __xuiCanvasNear(fPenWidth, 5.0f) && iPenColor == XUI_COLOR_RGBA(90, 70, 210, 255), "get pen");
	pViewport = xuiCanvasGetViewportWidget(pCanvas);
	tViewportWorld = xuiWidgetGetWorldRect(pViewport);
	iRet = xuiInputPointerDown(pContext, tViewportWorld.fX + 20.0f, tViewportWorld.fY + 20.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "pen down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pViewport, "pen capture");
	iRet = xuiInputPointerMove(pContext, tViewportWorld.fX + 48.0f, tViewportWorld.fY + 44.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "pen move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetRectFillColorCount(xuiCanvasGetSurface(pCanvas), XUI_COLOR_RGBA(90, 70, 210, 255)) >= 2, "pen draws");
	iDrawCount = xuiCanvasGetDrawCount(pCanvas);
	iRet = xuiInputPointerMove(pContext, tViewportWorld.fX + 48.0f,
		tViewportWorld.fY + 44.0f, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCanvasGetDrawCount(pCanvas) == iDrawCount,
		"duplicate pen sample does not redraw");
	iRet = xuiInputPointerUp(pContext, tViewportWorld.fX + 48.0f, tViewportWorld.fY + 44.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pen up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "pen release");

	iDrawCount = xuiCanvasGetDrawCount(pCanvas);
	iRet = xuiInputPointerDown(pContext, tViewportWorld.fX + 24.0f,
		tViewportWorld.fY + 24.0f, XUI_POINTER_BUTTON_LEFT,
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext,
		tViewportWorld.fX + 40.0f, tViewportWorld.fY + 36.0f,
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		xuiCanvasGetDrawCount(pCanvas) == iDrawCount + 2,
		"pointer-up position supplies the final pen sample");

	iRet = xuiInputPointerDown(pContext, tViewportWorld.fX + 20.0f, tViewportWorld.fY + 20.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "pen disable down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pViewport, "pen disable capture");
	iRet = xuiCanvasSetPen(pCanvas, 0, 0.0f, iPenColor);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "disabling pen cancels capture");
	iRet = xuiCanvasSetPen(pCanvas, 1, 5.0f, iPenColor);
	XUI_TEST_CHECK(iRet == XUI_OK, "reenable pen");
	iRet = xuiInputPointerDown(pContext, tViewportWorld.fX + 20.0f,
		tViewportWorld.fY + 20.0f, XUI_POINTER_BUTTON_LEFT,
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pViewport,
		"programmatic scroll starts with pen capture");
	iRet = xuiCanvasSetOffset(pCanvas, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL,
		"programmatic scroll cancels the active pen stroke");

	iRet = xuiInputPointerDown(pContext, tViewportWorld.fX + 20.0f, tViewportWorld.fY + 20.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "lost button down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pViewport, "lost button capture");
	iRet = xuiInputPointerMove(pContext, tViewportWorld.fX + 24.0f, tViewportWorld.fY + 24.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "lost button move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "lost button move cancels capture");

	iRet = xuiInputPointerDown(pContext, tViewportWorld.fX + 20.0f, tViewportWorld.fY + 20.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable widget down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pViewport, "disable widget capture");
	iRet = xuiWidgetSetEnabled(pCanvas, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable canvas");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "disabled canvas cancels capture");
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pCanvas, 1) == XUI_OK, "enable canvas");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK, "dispatch enabled canvas");

	iRet = xuiInputPointerDown(pContext, tViewportWorld.fX + 20.0f, tViewportWorld.fY + 20.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport mutation down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pViewport, "viewport mutation capture");
	iRet = xuiCanvasSetViewportHint(pCanvas, 181.0f, 141.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "viewport mutation cancels capture");
	iRet = xuiCanvasGetViewportHint(pCanvas, &fWidth, &fHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiCanvasNear(fWidth, 181.0f) && __xuiCanvasNear(fHeight, 141.0f), "canvas viewport hint");
	iRet = xuiScrollFrameGetViewportHint(xuiCanvasGetFrameWidget(pCanvas), &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiCanvasNear(fOffsetX, fWidth) && __xuiCanvasNear(fOffsetY, fHeight), "frame is viewport hint source");

	iRet = xuiWidgetSetPadding(pViewport, (xui_thickness_t){5.0f, 7.0f, 0.0f, 0.0f});
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiUpdate(pContext, 0.016f);
	if ( iRet == XUI_OK ) iRet = xuiCanvasSetOffset(pCanvas, 20.4f, 10.6f);
	XUI_TEST_CHECK(iRet == XUI_OK, "prepare padded viewport coordinates");
	tViewportWorld = xuiWidgetGetWorldRect(pViewport);
	tViewportContent = xuiWidgetGetContentRect(pViewport);
	tState.fLastPointX = -1.0f;
	tState.fLastPointY = -1.0f;
	iRet = xuiInputPointerDown(pContext,
		(int)(tViewportWorld.fX + tViewportContent.fX + 7.0f),
		(int)(tViewportWorld.fY + tViewportContent.fY + 9.0f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	xuiTestProxyGetLastPoint(&tState, &fWidth, &fHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiCanvasNear(fWidth, 27.0f) &&
		__xuiCanvasNear(fHeight, 20.0f), "pen matches padded viewport and snapped scroll offset");
	iRet = xuiInputPointerUp(pContext,
		(int)(tViewportWorld.fX + tViewportContent.fX + 7.0f),
		(int)(tViewportWorld.fY + tViewportContent.fY + 9.0f),
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "release padded pen");

	iRet = xuiInputPointerDownEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)(tViewportWorld.fX + tViewportContent.fX + 10.0f),
		(int)(tViewportWorld.fY + tViewportContent.fY + 10.0f),
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) == pViewport,
		"first touch owns pen capture");
	iRet = xuiInputPointerDownEx(pContext, 702u, XUI_POINTER_TYPE_TOUCH,
		(int)(tViewportWorld.fX + tViewportContent.fX + 20.0f),
		(int)(tViewportWorld.fY + tViewportContent.fY + 20.0f),
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 702u, XUI_POINTER_TYPE_TOUCH,
		(int)(tViewportWorld.fX + tViewportContent.fX + 20.0f),
		(int)(tViewportWorld.fY + tViewportContent.fY + 20.0f),
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) == pViewport &&
		xuiGetPointerCaptureEx(pContext, 702u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"second touch cannot steal or release pen capture");
	iRet = xuiInputPointerUpEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)(tViewportWorld.fX + tViewportContent.fX + 10.0f),
		(int)(tViewportWorld.fY + tViewportContent.fY + 10.0f),
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch releases pen capture");
	iRet = xuiWidgetSetPadding(pViewport, (xui_thickness_t){0});
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore viewport padding");
	iRet = xuiWidgetSetEnabled(pViewport, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable canvas viewport");
	iDrawCount = xuiCanvasGetDrawCount(pCanvas);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_DOWN;
	tEvent.pTarget = pViewport;
	tEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	tEvent.iButtons = XUI_POINTER_BUTTON_LEFT;
	tEvent.fX = tViewportWorld.fX + 20.0f;
	tEvent.fY = tViewportWorld.fY + 20.0f;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		xuiCanvasGetDrawCount(pCanvas) == iDrawCount,
		"disabled viewport rejects directly dispatched pen input");
	iRet = xuiWidgetSetEnabled(pViewport, 1);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "reenable canvas viewport");

	iRet = xuiInputPointerDown(pContext, tViewportWorld.fX + 20.0f, tViewportWorld.fY + 20.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide widget down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pViewport, "hide widget capture");
	iRet = xuiWidgetSetVisible(pCanvas, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide canvas");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "hidden canvas cancels capture");

cleanup:
	if ( pLazyCanvas != NULL ) {
		xuiWidgetDestroy(pLazyCanvas);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_canvas_test passed\n");
	return 0;
}
