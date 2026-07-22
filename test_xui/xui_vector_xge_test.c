#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <string.h>

#define TEST_W 32
#define TEST_H 32

typedef struct xui_vector_xge_test_t {
	int iResult;
} xui_vector_xge_test_t;

static int __xuiVectorXgeFrame(void* pUser)
{
	xui_vector_xge_test_t* pTest = (xui_vector_xge_test_t*)pUser;
	xui_proxy_t tProxy;
	xui_surface_desc_t tSurfaceDesc;
	xui_path_style_t tStyle;
	xui_context pContext = NULL;
	xui_surface pSurface = NULL;
	xui_painter pPainter = NULL;
	xui_path pPath = NULL;
	unsigned char arrPixels[TEST_W * TEST_H * 4];
	int iLeft;
	int iRight;
	int iRet;

	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	tProxy = xuiProxyXge();
	iRet = xuiCreate(&pContext);
	if ( iRet == XUI_OK ) iRet = xuiSetProxy(pContext, &tProxy);
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = TEST_W;
	tSurfaceDesc.iHeight = TEST_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
	if ( iRet == XUI_OK ) iRet = tProxy.surfaceCreate(&tProxy, &pSurface, &tSurfaceDesc);
	if ( iRet == XUI_OK ) iRet = tProxy.surfaceClear(&tProxy, pSurface, XUI_COLOR_RGBA(0, 0, 0, 0));
	if ( iRet == XUI_OK ) iRet = xuiPainterBegin(pContext, pSurface, &pPainter);
	if ( iRet == XUI_OK ) iRet = xuiPathCreate(&pPath);
	if ( iRet == XUI_OK ) iRet = xuiPathMoveTo(pPath, 2.0f, 2.0f);
	if ( iRet == XUI_OK ) iRet = xuiPathLineTo(pPath, 14.0f, 2.0f);
	if ( iRet == XUI_OK ) iRet = xuiPathLineTo(pPath, 14.0f, 30.0f);
	if ( iRet == XUI_OK ) iRet = xuiPathLineTo(pPath, 2.0f, 30.0f);
	if ( iRet == XUI_OK ) iRet = xuiPathClose(pPath);
	memset(&tStyle, 0, sizeof(tStyle));
	tStyle.iSize = (uint32_t)sizeof(tStyle);
	tStyle.iFillColor = XUI_COLOR_RGBA(24, 180, 72, 255);
	if ( iRet == XUI_OK ) iRet = xuiPainterDrawPath(pPainter, pPath, &tStyle, 0.25f);
	tStyle.iFillColor = XUI_COLOR_RGBA(48, 96, 220, 255);
	if ( iRet == XUI_OK ) {
		iRet = xuiPainterDrawSvgPath(pPainter, "M0 0 H12 V28 H0 Z",
			(xui_rect_t){0.0f, 0.0f, 12.0f, 28.0f},
			(xui_rect_t){18.0f, 2.0f, 12.0f, 28.0f}, &tStyle, 0.25f);
	}
	if ( pPainter != NULL ) {
		int iEndRet = xuiPainterEnd(pPainter);
		pPainter = NULL;
		if ( iRet == XUI_OK ) iRet = iEndRet;
	}
	if ( iRet == XUI_OK ) iRet = tProxy.surfaceReadRGBA(&tProxy, pSurface, arrPixels, TEST_W * 4);
	iLeft = ((16 * TEST_W) + 8) * 4;
	iRight = ((16 * TEST_W) + 24) * 4;
	if ( iRet == XUI_OK &&
	     ((arrPixels[iLeft + 1] < 120u) || (arrPixels[iLeft + 3] < 240u) ||
	      (arrPixels[iRight + 2] < 160u) || (arrPixels[iRight + 3] < 240u)) ) {
		printf("xui_vector_xge_test failed: left=%u,%u,%u,%u right=%u,%u,%u,%u\n",
			(unsigned)arrPixels[iLeft + 0], (unsigned)arrPixels[iLeft + 1],
			(unsigned)arrPixels[iLeft + 2], (unsigned)arrPixels[iLeft + 3],
			(unsigned)arrPixels[iRight + 0], (unsigned)arrPixels[iRight + 1],
			(unsigned)arrPixels[iRight + 2], (unsigned)arrPixels[iRight + 3]);
		iRet = XUI_ERROR_BACKEND_FAILED;
	}
	if ( pPath != NULL ) xuiPathDestroy(pPath);
	if ( pSurface != NULL ) tProxy.surfaceDestroy(&tProxy, pSurface);
	if ( pContext != NULL ) xuiDestroy(pContext);
	pTest->iResult = iRet;
	if ( xgeEnd() != XGE_OK && pTest->iResult == XUI_OK ) pTest->iResult = XUI_ERROR_BACKEND_FAILED;
	xgeQuit();
	return pTest->iResult;
}

int main(void)
{
	xui_vector_xge_test_t tTest;
	xge_desc_t tDesc;
	int iRet;

	memset(&tTest, 0, sizeof(tTest));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = TEST_W;
	tDesc.iHeight = TEST_H;
	tDesc.sTitle = "xui_vector_xge_test";
	tDesc.iFlags = XGE_INIT_OFFSCREEN;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	iRet = xgeInit(&tDesc);
	if ( iRet == XGE_OK ) iRet = xgeRun(__xuiVectorXgeFrame, &tTest);
	xgeUnit();
	if ( iRet != XGE_OK || tTest.iResult != XUI_OK ) {
		printf("xui_vector_xge_test failed: run=%d result=%d\n", iRet, tTest.iResult);
		return 1;
	}
	printf("xui_vector_xge_test passed\n");
	return 0;
}
