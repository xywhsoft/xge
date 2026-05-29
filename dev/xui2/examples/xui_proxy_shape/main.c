#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	820
#define DEMO_TARGET_H	480

typedef struct xui_proxy_shape_demo_t {
	xui_proxy_t tProxy;
	xui_surface pTarget;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
} xui_proxy_shape_demo_t;

static void __xuiProxyShapeUsage(void)
{
	printf("usage: xui_proxy_shape [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiProxyShapeParseArgs(xui_proxy_shape_demo_t* pDemo, int argc, char** argv)
{
	int i;

	if ( pDemo == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			pDemo->fMaxSeconds = atof(argv[++i]);
			if ( pDemo->fMaxSeconds <= 0.0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
			if ( pDemo->fMaxSeconds <= 0.0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			__xuiProxyShapeUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static void __xuiProxyShapeRect(float fX, float fY, float fW, float fH, xui_rect_t* pRect)
{
	pRect->fX = fX;
	pRect->fY = fY;
	pRect->fW = fW;
	pRect->fH = fH;
}

static xui_vec2_t __xuiProxyShapePoint(float fX, float fY)
{
	xui_vec2_t tPoint;

	tPoint.fX = fX;
	tPoint.fY = fY;
	return tPoint;
}

static xui_rect_t __xuiProxyShapeCell(int iCol, int iRow)
{
	xui_rect_t tRect;

	tRect.fX = 24.0f + (float)iCol * 158.0f;
	tRect.fY = 26.0f + (float)iRow * 222.0f;
	tRect.fW = 140.0f;
	tRect.fH = 190.0f;
	return tRect;
}

static int __xuiProxyShapeDrawCellFrame(xui_proxy_shape_demo_t* pDemo, xui_rect_t tCell)
{
	return pDemo->tProxy.shapeRoundRectStroke(&pDemo->tProxy, pDemo->pTarget, tCell, 14.0f, 1.5f, XUI_COLOR_RGBA(74, 88, 99, 255));
}

static int __xuiProxyShapeDrawAll(xui_proxy_shape_demo_t* pDemo)
{
	xui_rect_t tCell;
	xui_rect_t tRect;
	xui_vec2_t tA;
	xui_vec2_t tB;
	xui_vec2_t tC;
	int i;
	int iRet;

	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(20, 24, 28, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	for ( i = 0; i < 10; i++ ) {
		tCell = __xuiProxyShapeCell(i % 5, i / 5);
		iRet = __xuiProxyShapeDrawCellFrame(pDemo, tCell);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}

	tCell = __xuiProxyShapeCell(0, 0);
	iRet = pDemo->tProxy.shapePoint(&pDemo->tProxy, pDemo->pTarget, tCell.fX + 70.0f, tCell.fY + 94.0f, 22.0f, XUI_COLOR_RGBA(250, 205, 80, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	tCell = __xuiProxyShapeCell(1, 0);
	iRet = pDemo->tProxy.shapeLine(&pDemo->tProxy, pDemo->pTarget, tCell.fX + 26.0f, tCell.fY + 142.0f, tCell.fX + 114.0f, tCell.fY + 48.0f, 7.0f, XUI_COLOR_RGBA(86, 205, 255, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	tCell = __xuiProxyShapeCell(2, 0);
	tA = __xuiProxyShapePoint(tCell.fX + 70.0f, tCell.fY + 38.0f);
	tB = __xuiProxyShapePoint(tCell.fX + 26.0f, tCell.fY + 146.0f);
	tC = __xuiProxyShapePoint(tCell.fX + 116.0f, tCell.fY + 146.0f);
	iRet = pDemo->tProxy.shapeTriangleStroke(&pDemo->tProxy, pDemo->pTarget, tA, tB, tC, 5.0f, XUI_COLOR_RGBA(236, 115, 129, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	tCell = __xuiProxyShapeCell(3, 0);
	tA = __xuiProxyShapePoint(tCell.fX + 70.0f, tCell.fY + 38.0f);
	tB = __xuiProxyShapePoint(tCell.fX + 24.0f, tCell.fY + 148.0f);
	tC = __xuiProxyShapePoint(tCell.fX + 116.0f, tCell.fY + 148.0f);
	iRet = pDemo->tProxy.shapeTriangleFill(&pDemo->tProxy, pDemo->pTarget, tA, tB, tC, XUI_COLOR_RGBA(94, 207, 142, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	tCell = __xuiProxyShapeCell(4, 0);
	__xuiProxyShapeRect(tCell.fX + 28.0f, tCell.fY + 52.0f, 84.0f, 88.0f, &tRect);
	iRet = pDemo->tProxy.shapeRectStroke(&pDemo->tProxy, pDemo->pTarget, tRect, 5.0f, XUI_COLOR_RGBA(170, 141, 245, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	tCell = __xuiProxyShapeCell(0, 1);
	__xuiProxyShapeRect(tCell.fX + 30.0f, tCell.fY + 54.0f, 80.0f, 82.0f, &tRect);
	iRet = pDemo->tProxy.shapeRectFill(&pDemo->tProxy, pDemo->pTarget, tRect, XUI_COLOR_RGBA(62, 160, 236, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	tCell = __xuiProxyShapeCell(1, 1);
	iRet = pDemo->tProxy.shapeCircleStroke(&pDemo->tProxy, pDemo->pTarget, tCell.fX + 70.0f, tCell.fY + 95.0f, 44.0f, 5.0f, XUI_COLOR_RGBA(255, 151, 83, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	tCell = __xuiProxyShapeCell(2, 1);
	iRet = pDemo->tProxy.shapeCircleFill(&pDemo->tProxy, pDemo->pTarget, tCell.fX + 70.0f, tCell.fY + 95.0f, 45.0f, XUI_COLOR_RGBA(236, 87, 121, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	tCell = __xuiProxyShapeCell(3, 1);
	__xuiProxyShapeRect(tCell.fX + 24.0f, tCell.fY + 54.0f, 92.0f, 82.0f, &tRect);
	iRet = pDemo->tProxy.shapeRoundRectStroke(&pDemo->tProxy, pDemo->pTarget, tRect, 24.0f, 5.0f, XUI_COLOR_RGBA(102, 223, 205, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	tCell = __xuiProxyShapeCell(4, 1);
	__xuiProxyShapeRect(tCell.fX + 24.0f, tCell.fY + 54.0f, 92.0f, 82.0f, &tRect);
	return pDemo->tProxy.shapeRoundRectFill(&pDemo->tProxy, pDemo->pTarget, tRect, 24.0f, XUI_COLOR_RGBA(244, 190, 86, 255));
}

static int __xuiProxyShapeFrame(void* pUser)
{
	xui_proxy_shape_demo_t* pDemo;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;

	pDemo = (xui_proxy_shape_demo_t*)pUser;
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xuiProxyShapeDrawAll(pDemo);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	xgeClear(XGE_COLOR_RGBA(12, 15, 18, 255));
	__xuiProxyShapeRect(0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H, &tSrc);
	__xuiProxyShapeRect(20.0f, 20.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H, &tDst);
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XGE_OK ) {
		iRet = xgeEnd();
	}
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	pDemo->iFrame++;
	if ( ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ||
	     ((pDemo->fMaxSeconds > 0.0) && (xgeTimer() >= pDemo->fMaxSeconds)) ) {
		xgeQuit();
	}
	return 0;
}

int main(int argc, char** argv)
{
	xui_proxy_shape_demo_t tDemo;
	xui_surface_desc_t tSurfaceDesc;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiProxyShapeParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) {
		return 0;
	}
	if ( iRet != XGE_OK ) {
		__xuiProxyShapeUsage();
		return 1;
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 860;
	tDesc.iHeight = 520;
	tDesc.sTitle = "xui_proxy_shape";
	tDesc.iFlags = XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_shape: xgeInit failed: %d\n", iRet);
		return 1;
	}

	tDemo.tProxy = xuiProxyXge();
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = tDemo.tProxy.surfaceCreate(&tDemo.tProxy, &tDemo.pTarget, &tSurfaceDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_shape: surfaceCreate failed: %d\n", iRet);
		xgeUnit();
		return 1;
	}

	iRet = xgeRun(__xuiProxyShapeFrame, &tDemo);
	tDemo.tProxy.surfaceDestroy(&tDemo.tProxy, tDemo.pTarget);
	xgeUnit();
	return (iRet == XGE_OK) ? 0 : 1;
}
