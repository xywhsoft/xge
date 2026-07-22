#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_vector_smoke_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_surface pSurface;
	xui_painter pPainter;
	xui_path pPath;
	xui_path_style_t tStyle;
	float arrDash[2];
	int iPathBase;
	int iSvgPathBase;
	int iFailed;
	int iRet;

	pContext = NULL;
	pSurface = NULL;
	pPainter = NULL;
	pPath = NULL;
	iFailed = 0;
	arrDash[0] = 12.0f;
	arrDash[1] = 6.0f;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiTestSurfaceCreate(&tState, &pSurface, 320, 220, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pSurface != NULL, "surface create");
	iRet = xuiPainterBegin(pContext, pSurface, &pPainter);
	XUI_TEST_CHECK(iRet == XUI_OK && pPainter != NULL, "painter begin");

	iRet = xuiPathCreate(&pPath);
	XUI_TEST_CHECK(iRet == XUI_OK && pPath != NULL, "path create");
	iRet = xuiPathMoveTo(pPath, 38.0f, 142.0f);
	if ( iRet == XUI_OK ) iRet = xuiPathCubicTo(pPath, 70.0f, 32.0f, 156.0f, 32.0f, 190.0f, 142.0f);
	if ( iRet == XUI_OK ) iRet = xuiPathQuadTo(pPath, 116.0f, 186.0f, 38.0f, 142.0f);
	if ( iRet == XUI_OK ) iRet = xuiPathClose(pPath);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPathGetCommandCount(pPath) == 4, "path command recording");

	iPathBase = xuiTestProxyGetPathDrawCount(&tState);
	iSvgPathBase = xuiTestProxyGetSvgPathDrawCount(&tState);
	iRet = xuiPainterFillPath(pPainter, pPath, XUI_COLOR_RGBA(58, 132, 210, 180), 1.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "fill path");
	memset(&tStyle, 0, sizeof(tStyle));
	tStyle.iSize = sizeof(tStyle);
	tStyle.iStrokeColor = XUI_COLOR_RGBA(18, 64, 124, 255);
	tStyle.fStrokeWidth = 6.0f;
	tStyle.iLineJoin = XUI_PATH_JOIN_ROUND;
	tStyle.iLineCap = XUI_PATH_CAP_ROUND;
	iRet = xuiPainterDrawPath(pPainter, pPath, &tStyle, 1.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "round stroke path");
	tStyle.iFillColor = XUI_COLOR_RGBA(220, 104, 70, 180);
	tStyle.iFillRule = XUI_PATH_FILL_EVEN_ODD;
	tStyle.iStrokeColor = XUI_COLOR_RGBA(120, 44, 28, 255);
	tStyle.fStrokeWidth = 3.0f;
	tStyle.iLineJoin = XUI_PATH_JOIN_MITER;
	tStyle.iLineCap = XUI_PATH_CAP_BUTT;
	iRet = xuiPainterDrawSvgPath(pPainter, "m0 0 h48 v48 h-48 z",
		(xui_rect_t){0.0f, 0.0f, 48.0f, 48.0f},
		(xui_rect_t){230.0f, 42.0f, 48.0f, 48.0f}, &tStyle, 1.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "svg path backend draw");
	XUI_TEST_CHECK(xuiVectorIconGetCount() >= 4 && xuiVectorIconGetName(0) != NULL, "vector icon catalog");
	iRet = xuiPainterDrawVectorIcon(pPainter, "search", (xui_rect_t){236.0f, 116.0f, 42.0f, 42.0f}, XUI_COLOR_RGBA(42, 96, 160, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "vector icon draw search");
	iRet = xuiPainterDrawVectorIcon(pPainter, "check", (xui_rect_t){236.0f, 164.0f, 42.0f, 42.0f}, XUI_COLOR_RGBA(48, 132, 82, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "vector icon draw check");

	iRet = xuiPathClear(pPath);
	XUI_TEST_CHECK(iRet == XUI_OK, "path clear");
	iRet = xuiPathMoveTo(pPath, 34.0f, 42.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "dash move");
	iRet = xuiPathCubicTo(pPath, 90.0f, 18.0f, 144.0f, 82.0f, 208.0f, 42.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "dash cubic");
	tStyle.pDashPattern = arrDash;
	tStyle.iDashCount = 2;
	tStyle.fDashOffset = 3.0f;
	tStyle.iFillColor = 0;
	tStyle.iLineJoin = XUI_PATH_JOIN_BEVEL;
	tStyle.iLineCap = XUI_PATH_CAP_SQUARE;
	iRet = xuiPainterDrawPath(pPainter, pPath, &tStyle, 1.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "dashed stroke path");

	iRet = xuiPainterEnd(pPainter);
	pPainter = NULL;
	XUI_TEST_CHECK(iRet == XUI_OK, "painter end");
	XUI_TEST_CHECK(xuiTestProxyGetPathDrawCount(&tState) == iPathBase + 3, "ShapeEx path backend draw count");
	XUI_TEST_CHECK(xuiTestProxyGetSvgPathDrawCount(&tState) == iSvgPathBase + 3, "ShapeEx SVG path backend draw count");
	XUI_TEST_CHECK(xuiTestProxyGetLastPathCommandCount(&tState) == 2, "last path command count");

cleanup:
	if ( pPainter != NULL ) {
		xuiPainterEnd(pPainter);
	}
	if ( pPath != NULL ) {
		xuiPathDestroy(pPath);
	}
	if ( pSurface != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pSurface);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_vector_smoke_test passed\n");
	return 0;
}
