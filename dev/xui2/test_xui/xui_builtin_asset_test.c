#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_builtin_asset_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiBuiltinAssetRectEq(xui_rect_t tRect, float fX, float fY, float fW, float fH)
{
	return (tRect.fX == fX) && (tRect.fY == fY) && (tRect.fW == fW) && (tRect.fH == fH);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_surface pAtlas;
	xui_surface pAtlasAgain;
	xui_surface pTarget;
	xui_painter pPainter;
	xui_rect_t tRect;
	xui_rect_t tLastSrc;
	int iWidth;
	int iHeight;
	int iFailed;
	int iRet;

	pContext = NULL;
	pAtlas = NULL;
	pAtlasAgain = NULL;
	pTarget = NULL;
	pPainter = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");

	XUI_TEST_CHECK(xuiBuiltinAssetGetCount() == 50, "asset count");
	XUI_TEST_CHECK(strcmp(xuiBuiltinAssetGetName(0), "msgbox_info") == 0, "first asset name");
	XUI_TEST_CHECK(strcmp(xuiBuiltinAssetGetName(49), "file_dialog_refresh") == 0, "last asset name");
	XUI_TEST_CHECK(xuiBuiltinAssetGetName(-1) == NULL, "invalid negative name");
	XUI_TEST_CHECK(xuiBuiltinAssetGetName(50) == NULL, "invalid overflow name");
	XUI_TEST_CHECK(xuiBuiltinAssetGetAtlasSize(&iWidth, &iHeight) == XUI_OK, "atlas size");
	XUI_TEST_CHECK(iWidth == 512 && iHeight == 288, "atlas dimensions");
	XUI_TEST_CHECK(xuiBuiltinAssetGetAtlasSize(NULL, NULL) == XUI_ERROR_INVALID_ARGUMENT, "atlas size invalid args");
	XUI_TEST_CHECK(xuiBuiltinAssetGetRect("button_badge", &tRect) == XUI_OK, "button badge rect");
	XUI_TEST_CHECK(__xuiBuiltinAssetRectEq(tRect, 0.0f, 72.0f, 16.0f, 16.0f), "button badge rect value");
	XUI_TEST_CHECK(xuiBuiltinAssetGetRect("file_dialog_image", &tRect) == XUI_OK, "file dialog image rect");
	XUI_TEST_CHECK(__xuiBuiltinAssetRectEq(tRect, 144.0f, 220.0f, 16.0f, 16.0f), "file dialog image rect value");
	XUI_TEST_CHECK(xuiBuiltinAssetGetRect("file_dialog_refresh", &tRect) == XUI_OK, "file dialog refresh rect");
	XUI_TEST_CHECK(__xuiBuiltinAssetRectEq(tRect, 198.0f, 220.0f, 16.0f, 16.0f), "file dialog refresh rect value");
	XUI_TEST_CHECK(xuiBuiltinAssetGetRectByIndex(16, &tRect) == XUI_OK, "input search by index");
	XUI_TEST_CHECK(__xuiBuiltinAssetRectEq(tRect, 76.0f, 72.0f, 12.0f, 12.0f), "input search rect value");
	XUI_TEST_CHECK(xuiBuiltinAssetGetRect("missing", &tRect) == XUI_ERROR_FILE_NOT_FOUND, "missing rect");
	XUI_TEST_CHECK(xuiBuiltinAssetGetRect(NULL, &tRect) == XUI_ERROR_INVALID_ARGUMENT, "rect invalid name");
	XUI_TEST_CHECK(xuiBuiltinAssetGetRectByIndex(50, &tRect) == XUI_ERROR_INVALID_ARGUMENT, "rect invalid index");
	XUI_TEST_CHECK(xuiBuiltinAssetGetAtlas(pContext, &pAtlas) == XUI_ERROR_NOT_INITIALIZED, "atlas requires proxy");

	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	XUI_TEST_CHECK(xuiBuiltinAssetGetAtlas(pContext, &pAtlas) == XUI_OK && pAtlas != NULL, "atlas load");
	XUI_TEST_CHECK(xuiBuiltinAssetGetAtlas(pContext, &pAtlasAgain) == XUI_OK, "atlas load cached");
	XUI_TEST_CHECK(pAtlasAgain == pAtlas, "atlas cache identity");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 128, 64, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiPainterBegin(pContext, pTarget, &pPainter);
	XUI_TEST_CHECK(iRet == XUI_OK && pPainter != NULL, "painter begin");
	iRet = xuiPainterDrawSurface(pPainter, pAtlas, tRect, (xui_rect_t){10.0f, 12.0f, 12.0f, 12.0f}, XUI_COLOR_WHITE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "draw builtin asset");
	iRet = xuiPainterEnd(pPainter);
	pPainter = NULL;
	XUI_TEST_CHECK(iRet == XUI_OK, "painter end");
	XUI_TEST_CHECK(xuiTestSurfaceGetDrawCount(pTarget) == 1, "draw count");
	tLastSrc = xuiTestSurfaceGetLastSrc(pTarget);
	XUI_TEST_CHECK(__xuiBuiltinAssetRectEq(tLastSrc, 76.0f, 72.0f, 12.0f, 12.0f), "draw src rect");

cleanup:
	if ( pPainter != NULL ) {
		xuiPainterEnd(pPainter);
	}
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_builtin_asset_test passed\n");
	return 0;
}
