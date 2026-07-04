#include "../../xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 980
#define DEMO_H 640

typedef struct xge_shape_demo_t {
	xge_render_target_t tTarget;
	char sCapturePath[260];
	int iRenderMode;
	int bCaptureDone;
	int iFrame;
	int iMaxFrames;
} xge_shape_demo_t;

static xge_rect_t __xgeShapeDemoRect(float fX, float fY, float fW, float fH)
{
	xge_rect_t tRect;

	tRect.fX = fX;
	tRect.fY = fY;
	tRect.fW = fW;
	tRect.fH = fH;
	return tRect;
}

static xge_vec2_t __xgeShapeDemoPoint(float fX, float fY)
{
	xge_vec2_t tPoint;

	tPoint.fX = fX;
	tPoint.fY = fY;
	return tPoint;
}

static void __xgeShapeDemoUsage(void)
{
	printf("usage: xge_shape [--frames N] [--capture PATH] [--render aa-mesh|sdf]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xgeShapeDemoParseArgs(xge_shape_demo_t* pDemo, int argc, char** argv)
{
	int i;

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
		} else if ( strcmp(argv[i], "--capture") == 0 ) {
			if ( i + 1 >= argc ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			snprintf(pDemo->sCapturePath, sizeof(pDemo->sCapturePath), "%s", argv[++i]);
			pDemo->sCapturePath[sizeof(pDemo->sCapturePath) - 1] = '\0';
		} else if ( strncmp(argv[i], "--capture=", 10) == 0 ) {
			snprintf(pDemo->sCapturePath, sizeof(pDemo->sCapturePath), "%s", argv[i] + 10);
			pDemo->sCapturePath[sizeof(pDemo->sCapturePath) - 1] = '\0';
		} else if ( strcmp(argv[i], "--render") == 0 ) {
			if ( i + 1 >= argc ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			i++;
			if ( (strcmp(argv[i], "sdf") == 0) || (strcmp(argv[i], "SDF") == 0) ) {
				pDemo->iRenderMode = XGE_SHAPE_RENDER_SDF;
			} else if ( (strcmp(argv[i], "aa-mesh") == 0) || (strcmp(argv[i], "mesh") == 0) || (strcmp(argv[i], "AA_MESH") == 0) ) {
				pDemo->iRenderMode = XGE_SHAPE_RENDER_AA_MESH;
			} else {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( strncmp(argv[i], "--render=", 9) == 0 ) {
			const char* sMode = argv[i] + 9;
			if ( (strcmp(sMode, "sdf") == 0) || (strcmp(sMode, "SDF") == 0) ) {
				pDemo->iRenderMode = XGE_SHAPE_RENDER_SDF;
			} else if ( (strcmp(sMode, "aa-mesh") == 0) || (strcmp(sMode, "mesh") == 0) || (strcmp(sMode, "AA_MESH") == 0) ) {
				pDemo->iRenderMode = XGE_SHAPE_RENDER_AA_MESH;
			} else {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			__xgeShapeDemoUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static int __xgeShapeDemoCapture(xge_shape_demo_t* pDemo)
{
	unsigned char* pPixels;
	int iStride;
	int iRet;

	if ( (pDemo == NULL) || (pDemo->sCapturePath[0] == '\0') || (pDemo->bCaptureDone != 0) ) {
		return XGE_OK;
	}
	iStride = DEMO_W * 4;
	pPixels = (unsigned char*)malloc((size_t)iStride * DEMO_H);
	if ( pPixels == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeRenderTargetReadPixels(&pDemo->tTarget, pPixels, iStride);
	if ( iRet == XGE_OK ) {
		iRet = xgeImageSavePNG(pDemo->sCapturePath, DEMO_W, DEMO_H, pPixels, iStride);
	}
	free(pPixels);
	if ( iRet == XGE_OK ) {
		pDemo->bCaptureDone = 1;
		printf("xge_shape capture saved: %s\n", pDemo->sCapturePath);
	}
	return iRet;
}

static void __xgeShapeDemoDrawPanel(xge_rect_t tRect)
{
	xgeShapeRoundRectDrawPx(tRect, 10.0f, XGE_COLOR_RGBA(23, 28, 34, 255), 1.0f, XGE_COLOR_RGBA(72, 88, 102, 255));
}

static void __xgeShapeDemoDrawAll(void)
{
	xge_shape_round_rect_t tRadii;
	xge_rect_t tRect;
	xge_vec2_t tA;
	xge_vec2_t tB;
	xge_vec2_t tC;

	xgeClear(XGE_COLOR_RGBA(14, 18, 22, 255));
	__xgeShapeDemoDrawPanel(__xgeShapeDemoRect(24.0f, 24.0f, 288.0f, 180.0f));
	__xgeShapeDemoDrawPanel(__xgeShapeDemoRect(344.0f, 24.0f, 288.0f, 180.0f));
	__xgeShapeDemoDrawPanel(__xgeShapeDemoRect(664.0f, 24.0f, 288.0f, 180.0f));
	__xgeShapeDemoDrawPanel(__xgeShapeDemoRect(24.0f, 232.0f, 288.0f, 180.0f));
	__xgeShapeDemoDrawPanel(__xgeShapeDemoRect(344.0f, 232.0f, 288.0f, 180.0f));
	__xgeShapeDemoDrawPanel(__xgeShapeDemoRect(664.0f, 232.0f, 288.0f, 180.0f));
	__xgeShapeDemoDrawPanel(__xgeShapeDemoRect(24.0f, 440.0f, 288.0f, 176.0f));
	__xgeShapeDemoDrawPanel(__xgeShapeDemoRect(344.0f, 440.0f, 288.0f, 176.0f));
	__xgeShapeDemoDrawPanel(__xgeShapeDemoRect(664.0f, 440.0f, 288.0f, 176.0f));

	xgeShapeLinePx(58.0f, 156.0f, 166.0f, 54.0f, 8.0f, XGE_COLOR_RGBA(86, 205, 255, 255));
	tA = __xgeShapeDemoPoint(228.0f, 54.0f);
	tB = __xgeShapeDemoPoint(182.0f, 164.0f);
	tC = __xgeShapeDemoPoint(276.0f, 164.0f);
	xgeShapeTriangleStrokePx(tA, tB, tC, 6.0f, XGE_COLOR_RGBA(240, 122, 138, 255));

	xgeShapeEllipseFillPx(432.0f, 114.0f, 58.0f, 38.0f, XGE_COLOR_RGBA(89, 204, 143, 255));
	xgeShapeEllipseStrokePx(544.0f, 114.0f, 50.0f, 64.0f, 5.0f, XGE_COLOR_RGBA(255, 171, 84, 255));

	xgeShapeArcPx(724.0f, 116.0f, 58.0f, -2.55f, 0.92f, 10.0f, XGE_COLOR_RGBA(116, 213, 238, 255));
	xgeShapeCircleStrokePx(852.0f, 114.0f, 58.0f, 5.0f, XGE_COLOR_RGBA(178, 144, 246, 255));

	xgeShapePieFillPx(118.0f, 322.0f, 70.0f, 54.0f, -1.2f, 2.25f, XGE_COLOR_RGBA(244, 190, 86, 255));
	xgeShapeChordFillPx(230.0f, 322.0f, 62.0f, 56.0f, 0.25f, 3.95f, XGE_COLOR_RGBA(236, 87, 121, 255));

	tRect = __xgeShapeDemoRect(392.0f, 284.0f, 192.0f, 76.0f);
	xgeShapeCapsuleFillPx(tRect, XGE_COLOR_RGBA(60, 163, 236, 255));

	tRect = __xgeShapeDemoRect(704.0f, 276.0f, 204.0f, 92.0f);
	xgeShapeCapsuleStrokePx(tRect, 8.0f, XGE_COLOR_RGBA(108, 229, 204, 255));

	tRect = __xgeShapeDemoRect(62.0f, 486.0f, 196.0f, 84.0f);
	xgeShapeRoundRectDrawPx(tRect, 24.0f, XGE_COLOR_RGBA(62, 160, 236, 255), 5.0f, XGE_COLOR_RGBA(180, 218, 255, 255));

	tRadii.fTopLeft = 8.0f;
	tRadii.fTopRight = 32.0f;
	tRadii.fBottomRight = 18.0f;
	tRadii.fBottomLeft = 42.0f;
	xgeShapeRoundRectDrawExPx(__xgeShapeDemoRect(390.0f, 486.0f, 196.0f, 84.0f), tRadii, XGE_SHAPE_ROUND_RECT_SDF, XGE_COLOR_RGBA(240, 112, 143, 255), 6.0f, XGE_COLOR_RGBA(255, 210, 220, 255));
	xgeShapeRoundRectDrawExPx(__xgeShapeDemoRect(710.0f, 486.0f, 196.0f, 84.0f), tRadii, XGE_SHAPE_ROUND_RECT_MESH, XGE_COLOR_RGBA(94, 207, 142, 255), 6.0f, XGE_COLOR_RGBA(202, 250, 220, 255));
}

static int __xgeShapeDemoFrame(void* pUser)
{
	xge_shape_demo_t* pDemo;
	xge_pass_t tPass;
	xge_draw_t tDraw;
	xge_texture pTexture;
	int iRet;

	pDemo = (xge_shape_demo_t*)pUser;
	if ( pDemo == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xgePassInit(&tPass, &pDemo->tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(14, 18, 22, 255));
	iRet = xgePassBegin(&tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	__xgeShapeDemoDrawAll();
	iRet = xgePassEnd(&tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeShapeDemoCapture(pDemo);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xgeClear(XGE_COLOR_RGBA(8, 10, 12, 255));
	memset(&tDraw, 0, sizeof(tDraw));
	pTexture = xgeRenderTargetTexture(&pDemo->tTarget);
	tDraw.pTexture = pTexture;
	tDraw.tSrc = __xgeShapeDemoRect(0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H);
	tDraw.tDst = __xgeShapeDemoRect(20.0f, 20.0f, (float)DEMO_W, (float)DEMO_H);
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
	iRet = xgeEnd();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pDemo->iFrame++;
	if ( (pDemo->bCaptureDone != 0) || ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ) {
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_shape_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	tDemo.iRenderMode = XGE_SHAPE_RENDER_SDF;
	iRet = __xgeShapeDemoParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) {
		return 0;
	}
	if ( iRet != XGE_OK ) {
		__xgeShapeDemoUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_W + 40;
	tDesc.iHeight = DEMO_H + 40;
	tDesc.sTitle = "xge_shape";
	tDesc.iFlags = XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xge_shape: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = xgeRenderTargetCreate(&tDemo.tTarget, DEMO_W, DEMO_H);
	if ( iRet != XGE_OK ) {
		printf("xge_shape: render target create failed: %d\n", iRet);
		xgeUnit();
		return 1;
	}
	xgeShapeRenderModeSet(tDemo.iRenderMode);
	xgeShapeRoundRectModeSet(XGE_SHAPE_ROUND_RECT_AUTO);
	iRet = xgeRun(__xgeShapeDemoFrame, &tDemo);
	xgeRenderTargetFree(&tDemo.tTarget);
	xgeUnit();
	return (iRet == XGE_OK) ? 0 : 1;
}
