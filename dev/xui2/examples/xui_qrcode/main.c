#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	700
#define DEMO_TARGET_H	430
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define QRCODE_COUNT	3

typedef struct xui_qrcode_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_surface pIcon;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pQrCode[QRCODE_COUNT];
	xui_widget pStatus;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bDynamicOK;
} xui_qrcode_demo_t;

static void __xuiQrCodeDemoUsage(void)
{
	printf("usage: xui_qrcode [--frames N] [--seconds N]\n");
}

static int __xuiQrCodeDemoParseArgs(xui_qrcode_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->fMaxSeconds = atof(argv[++i]);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__xuiQrCodeDemoUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiQrCodeDemoFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\simhei.ttf"
	};
	FILE* pFile;
	int i;

	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPaths[i];
		}
	}
	return NULL;
}

static int __xuiQrCodeDemoRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_qrcode_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_qrcode_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(234, 242, 250, 255));
	}
	if ( pDemo->tProxy.drawRoundRectFill != NULL ) {
		tPanel = (xui_rect_t){28.0f, 22.0f, tRect.fW - 56.0f, tRect.fH - 44.0f};
		(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tPanel, 6.0f, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static int __xuiQrCodeDemoAddLabel(xui_qrcode_demo_t* pDemo, const char* sText, xui_rect_t tRect, xui_widget* ppLabel)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(57, 71, 91, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pLabel, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	if ( ppLabel != NULL ) *ppLabel = pLabel;
	return XUI_OK;
}

static int __xuiQrCodeDemoCreateIcon(xui_qrcode_demo_t* pDemo)
{
	uint32_t arrPixels[32 * 32];
	int iX;
	int iY;

	for ( iY = 0; iY < 32; iY++ ) {
		for ( iX = 0; iX < 32; iX++ ) {
			if ( iX < 4 || iY < 4 || iX >= 28 || iY >= 28 ) {
				arrPixels[iY * 32 + iX] = XUI_COLOR_RGBA(255, 255, 255, 255);
			} else if ( iX < 16 ) {
				arrPixels[iY * 32 + iX] = XUI_COLOR_RGBA(47, 128, 237, 255);
			} else if ( iY < 16 ) {
				arrPixels[iY * 32 + iX] = XUI_COLOR_RGBA(16, 185, 129, 255);
			} else {
				arrPixels[iY * 32 + iX] = XUI_COLOR_RGBA(245, 158, 11, 255);
			}
		}
	}
	return pDemo->tProxy.surfaceCreateRGBA(&pDemo->tProxy, &pDemo->pIcon, 32, 32, arrPixels, 32 * (int)sizeof(uint32_t), XUI_SURFACE_ALPHA_PREMULTIPLIED);
}

static int __xuiQrCodeDemoAddQrCode(xui_qrcode_demo_t* pDemo, int iIndex, const char* sValue, uint32_t iForeground, uint32_t iBackground, xui_surface pIcon, xui_rect_t tRect)
{
	xui_qrcode_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sValue = sValue;
	tDesc.iForegroundColor = iForeground;
	tDesc.iBackgroundColor = iBackground;
	tDesc.fPadding = 10.0f;
	tDesc.pIconSurface = pIcon;
	tDesc.tIconSrc = (xui_rect_t){0.0f, 0.0f, 32.0f, 32.0f};
	tDesc.fIconSize = (pIcon != NULL) ? 34.0f : 0.0f;
	iRet = xuiQrCodeCreate(pDemo->pContext, &pDemo->pQrCode[iIndex], &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pQrCode[iIndex], tRect);
	return xuiWidgetAddChild(pDemo->pRoot, pDemo->pQrCode[iIndex]);
}

static int __xuiQrCodeDemoCreateUi(xui_qrcode_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiQrCodeDemoRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiQrCodeDemoAddLabel(pDemo, "XUI QRCode", (xui_rect_t){48.0f, 38.0f, 180.0f, 24.0f}, NULL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiQrCodeDemoCreateIcon(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiQrCodeDemoAddQrCode(pDemo, 0, "https://example.com/xui2/qrcode", XUI_COLOR_RGBA(0, 0, 0, 255), XUI_COLOR_WHITE, NULL,
		(xui_rect_t){58.0f, 88.0f, 170.0f, 170.0f});
	if ( iRet == XUI_OK ) iRet = __xuiQrCodeDemoAddQrCode(pDemo, 1, "XUI2 QRCode text value", XUI_COLOR_RGBA(18, 112, 214, 255), XUI_COLOR_RGBA(244, 250, 255, 255), NULL,
		(xui_rect_t){264.0f, 88.0f, 170.0f, 170.0f});
	if ( iRet == XUI_OK ) iRet = __xuiQrCodeDemoAddQrCode(pDemo, 2, "https://xui.local/icon", XUI_COLOR_RGBA(22, 101, 52, 255), XUI_COLOR_WHITE, pDemo->pIcon,
		(xui_rect_t){470.0f, 88.0f, 170.0f, 170.0f});
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiQrCodeDemoAddLabel(pDemo, "Default URL", (xui_rect_t){58.0f, 272.0f, 170.0f, 22.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiQrCodeDemoAddLabel(pDemo, "Custom colors", (xui_rect_t){264.0f, 272.0f, 170.0f, 22.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiQrCodeDemoAddLabel(pDemo, "Center icon", (xui_rect_t){470.0f, 272.0f, 170.0f, 22.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiQrCodeDemoAddLabel(pDemo, "Value is encoded dynamically. Middle icon is optional.", (xui_rect_t){58.0f, 330.0f, 560.0f, 22.0f}, &pDemo->pStatus);
	return iRet;
}

static void __xuiQrCodeDemoRunChecks(xui_qrcode_demo_t* pDemo, int bAutoRun)
{
	int iOk;

	if ( pDemo == NULL ) return;
	pDemo->bLayoutOK = (xuiQrCodeGetModuleCount(pDemo->pQrCode[0]) > 0) &&
	                   (xuiQrCodeGetModule(pDemo->pQrCode[0], 0, 0) == 1) &&
	                   (xuiQrCodeGetModule(pDemo->pQrCode[0], 3, 3) == 1) &&
	                   (xuiQrCodeGetIcon(pDemo->pQrCode[2]) == pDemo->pIcon);
	if ( !bAutoRun || pDemo->bExerciseDone ) {
		return;
	}
	iOk = (xuiQrCodeSetValue(pDemo->pQrCode[0], "https://example.com/xui2/qrcode/updated") == XUI_OK);
	iOk = iOk && (xuiQrCodeGetChangeCount(pDemo->pQrCode[0]) == 1);
	iOk = iOk && (xuiQrCodeGetModuleCount(pDemo->pQrCode[0]) > 0);
	iOk = iOk && (xuiQrCodeSetColors(pDemo->pQrCode[1], XUI_COLOR_RGBA(124, 58, 237, 255), XUI_COLOR_RGBA(250, 245, 255, 255)) == XUI_OK);
	pDemo->bDynamicOK = iOk;
	pDemo->bExerciseDone = 1;
}

static int __xuiQrCodeDemoCreateAssets(xui_qrcode_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiQrCodeDemoFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiQrCodeDemoCreateUi(pDemo);
}

static void __xuiQrCodeDemoDestroyAssets(xui_qrcode_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
		pDemo->pFont = NULL;
	}
	if ( pDemo->pIcon != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pIcon);
		pDemo->pIcon = NULL;
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
		pDemo->pTarget = NULL;
	}
}

static int __xuiQrCodeDemoFrame(void* pUser)
{
	xui_qrcode_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_qrcode_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiQrCodeDemoRunChecks(pDemo, bAutoRun);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(22, 26, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		memset(&tCacheStats, 0, sizeof(tCacheStats));
		tStats.iSize = sizeof(tStats);
		tCacheStats.iSize = sizeof(tCacheStats);
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		(void)xuiGetCacheStats(pDemo->pContext, &tCacheStats);
		printf("xui_qrcode final-summary frames=%d create=%d layout=%d dynamic=%d version=%d modules=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bDynamicOK,
			xuiQrCodeGetVersion(pDemo->pQrCode[0]), xuiQrCodeGetModuleCount(pDemo->pQrCode[0]),
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_qrcode_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiQrCodeDemoParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiQrCodeDemoUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI QRCode";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_qrcode: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiQrCodeDemoCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_qrcode: create assets failed: %d\n", iRet);
		__xuiQrCodeDemoDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	tDemo.bCreateOK = 1;
	iRet = xgeRun(__xuiQrCodeDemoFrame, &tDemo);
	__xuiQrCodeDemoDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK &&
	        ((tDemo.iMaxFrames <= 0 && tDemo.fMaxSeconds <= 0.0) || tDemo.bDynamicOK)) ? 0 : 1;
}
