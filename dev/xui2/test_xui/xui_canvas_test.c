#include "xui.h"
#include "xui_test_proxy.h"

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
	xui_widget pViewport;
	xui_surface pTarget;
	xui_surface pCanvasSurface;
	xui_canvas_desc_t tDesc;
	xui_rect_t tViewportWorld;
	xui_mesh_vertex_t arrVertices[3];
	uint32_t arrIndices[3];
	float fWidth;
	float fHeight;
	float fOffsetX;
	float fOffsetY;
	int bPenEnabled;
	float fPenWidth;
	uint32_t iPenColor;
	int iRet;
	int iFailed;

	pContext = NULL;
	pRoot = NULL;
	pCanvas = NULL;
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

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fCanvasWidth = 480.0f;
	tDesc.fCanvasHeight = 320.0f;
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

	iRet = xuiCanvasClear(pCanvas, XUI_COLOR_RGBA(250, 250, 250, 255));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetLastColor(pCanvasSurface) == XUI_COLOR_RGBA(250, 250, 250, 255), "clear");
	iRet = xuiCanvasDrawRectFill(pCanvas, (xui_rect_t){20.0f, 24.0f, 80.0f, 50.0f}, XUI_COLOR_RGBA(230, 80, 90, 255));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCanvasGetDrawCount(pCanvas) >= 2, "rect fill");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillColorCount(pCanvasSurface, XUI_COLOR_RGBA(230, 80, 90, 255)) == 1, "rect color count");
	iRet = xuiCanvasDrawLine(pCanvas, 12.0f, 12.0f, 120.0f, 90.0f, 4.0f, XUI_COLOR_RGBA(20, 90, 210, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "line");
	iRet = xuiCanvasDrawCircleStroke(pCanvas, 150.0f, 90.0f, 24.0f, 3.0f, XUI_COLOR_RGBA(30, 160, 110, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "circle stroke");

	arrVertices[0] = (xui_mesh_vertex_t){220.0f, 34.0f, XUI_COLOR_RGBA(40, 80, 180, 255)};
	arrVertices[1] = (xui_mesh_vertex_t){280.0f, 110.0f, XUI_COLOR_RGBA(40, 180, 120, 255)};
	arrVertices[2] = (xui_mesh_vertex_t){190.0f, 120.0f, XUI_COLOR_RGBA(220, 140, 60, 255)};
	arrIndices[0] = 0;
	arrIndices[1] = 1;
	arrIndices[2] = 2;
	iRet = xuiCanvasDrawMeshTriangles(pCanvas, arrVertices, 3, arrIndices, 3, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestProxyGetMeshDrawCount(&tState) == 1, "mesh");

	iRet = xuiCanvasSetOffset(pCanvas, 120.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set offset");
	iRet = xuiCanvasGetOffset(pCanvas, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiCanvasNear(fOffsetX, 120.0f) && __xuiCanvasNear(fOffsetY, 80.0f), "get offset");
	iRet = __xuiCanvasRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "scrolled render");
	XUI_TEST_CHECK(xuiTestSurfaceGetDrawCount(pTarget) > 0, "scrolled render draw count");

	iRet = xuiCanvasSetCanvasSize(pCanvas, 260.0f, 200.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "resize");
	iRet = xuiCanvasGetCanvasSize(pCanvas, &fWidth, &fHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiCanvasNear(fWidth, 260.0f) && __xuiCanvasNear(fHeight, 200.0f), "resized size");

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
	iRet = xuiInputPointerUp(pContext, tViewportWorld.fX + 48.0f, tViewportWorld.fY + 44.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pen up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "pen release");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_canvas_test passed\n");
	return 0;
}
