#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	820
#define DEMO_TARGET_H	420
#define DEMO_XRF_FILE	"xui_proxy_font_demo.xrf"

typedef struct xui_proxy_font_demo_t {
	xui_proxy_t tProxy;
	xui_surface pTarget;
	xui_font pXrfFont;
	xui_font pTtfFont;
	xui_font pSmallFont;
	xui_font pLargeFont;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
} xui_proxy_font_demo_t;

static void __xuiProxyFontUsage(void)
{
	printf("usage: xui_proxy_font [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiProxyFontParseArgs(xui_proxy_font_demo_t* pDemo, int argc, char** argv)
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
			__xuiProxyFontUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static void __xuiProxyFontRect(float fX, float fY, float fW, float fH, xui_rect_t* pRect)
{
	pRect->fX = fX;
	pRect->fY = fY;
	pRect->fW = fW;
	pRect->fH = fH;
}

static int __xuiProxyFontFullPath(const char* sPath, char* sOut, int iCapacity)
{
	if ( (sPath == NULL) || (sOut == NULL) || (iCapacity <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
#if defined(_WIN32) || defined(_WIN64)
	if ( _fullpath(sOut, sPath, (size_t)iCapacity) == NULL ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
#else
	if ( strlen(sPath) >= (size_t)iCapacity ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	strcpy(sOut, sPath);
#endif
	return XGE_OK;
}

static const char* __xuiProxyFontFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\simhei.ttf",
		"C:\\Windows\\Fonts\\simsun.ttc",
		"C:\\Windows\\Fonts\\msyh.ttc"
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

static int __xuiProxyFontCreateXrf(const char* sTtfPath, const char* sXrfPath)
{
	xge_font_t tFont;
	int iRet;

	if ( (sTtfPath == NULL) || (sXrfPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tFont, 0, sizeof(tFont));
	iRet = xgeFontLoad(&tFont, sTtfPath, 24.0f);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeFontSaveXRF(&tFont, sXrfPath, 32, 95);
	xgeFontFree(&tFont);
	return iRet;
}

static int __xuiProxyFontDrawText(xui_proxy_font_demo_t* pDemo, xui_font pFont, const char* sText, float fX, float fY, float fW, float fH, uint32_t iColor, uint32_t iFlags)
{
	xui_rect_t tRect;

	__xuiProxyFontRect(fX, fY, fW, fH, &tRect);
	return pDemo->tProxy.textDraw(&pDemo->tProxy, pDemo->pTarget, pFont, sText, tRect, iColor, iFlags | XUI_TEXT_CLIP);
}

static int __xuiProxyFontDrawPanel(xui_proxy_font_demo_t* pDemo, float fX, float fY, float fW, float fH)
{
	xui_rect_t tRect;
	int iRet;

	__xuiProxyFontRect(fX, fY, fW, fH, &tRect);
	iRet = pDemo->tProxy.shapeRectFill(&pDemo->tProxy, pDemo->pTarget, tRect, XUI_COLOR_RGBA(27, 32, 37, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	return pDemo->tProxy.shapeRectStroke(&pDemo->tProxy, pDemo->pTarget, tRect, 1.5f, XUI_COLOR_RGBA(80, 92, 103, 255));
}

static int __xuiProxyFontDrawAll(xui_proxy_font_demo_t* pDemo)
{
	xui_font_metrics_t tMetrics;
	xui_vec2_t tSize;
	xui_rect_t tRect;
	char arrInfo[160];
	int iRet;

	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(15, 18, 22, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	iRet = __xuiProxyFontDrawPanel(pDemo, 24.0f, 24.0f, 366.0f, 148.0f);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xuiProxyFontDrawPanel(pDemo, 430.0f, 24.0f, 366.0f, 148.0f);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xuiProxyFontDrawPanel(pDemo, 24.0f, 202.0f, 772.0f, 188.0f);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	iRet = __xuiProxyFontDrawText(pDemo, pDemo->pXrfFont, "XRF 24px", 48.0f, 44.0f, 318.0f, 32.0f, XUI_COLOR_RGBA(128, 209, 255, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xuiProxyFontDrawText(pDemo, pDemo->pXrfFont, "Cached atlas text rendered to an XUI surface.", 48.0f, 84.0f, 318.0f, 62.0f, XUI_COLOR_RGBA(232, 238, 243, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	iRet = __xuiProxyFontDrawText(pDemo, pDemo->pTtfFont, "TTF 24px", 454.0f, 44.0f, 318.0f, 32.0f, XUI_COLOR_RGBA(159, 224, 156, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_UNDERLINE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xuiProxyFontDrawText(pDemo, pDemo->pTtfFont, "TrueType font loaded directly by the proxy.", 454.0f, 84.0f, 318.0f, 62.0f, XUI_COLOR_RGBA(232, 238, 243, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	iRet = pDemo->tProxy.fontGetMetrics(&pDemo->tProxy, pDemo->pTtfFont, &tMetrics);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = pDemo->tProxy.textMeasure(&pDemo->tProxy, pDemo->pTtfFont, "Measured width", &tSize);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	snprintf(arrInfo, sizeof(arrInfo), "TTF metrics: lineHeight %.1f, measured width %.1f", tMetrics.fLineHeight, tSize.fX);
	iRet = __xuiProxyFontDrawText(pDemo, pDemo->pSmallFont, arrInfo, 48.0f, 214.0f, 700.0f, 28.0f, XUI_COLOR_RGBA(183, 190, 198, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	iRet = __xuiProxyFontDrawText(pDemo, pDemo->pSmallFont, "Size 16: compact UI labels, loaded from TTF memory", 48.0f, 258.0f, 700.0f, 28.0f, XUI_COLOR_RGBA(236, 240, 244, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xuiProxyFontDrawText(pDemo, pDemo->pTtfFont, "Size 24: normal controls and document body text", 48.0f, 296.0f, 700.0f, 36.0f, XUI_COLOR_RGBA(236, 240, 244, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xuiProxyFontDrawText(pDemo, pDemo->pLargeFont, "Size 40: large headings", 48.0f, 340.0f, 700.0f, 42.0f, XUI_COLOR_RGBA(255, 211, 122, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	__xuiProxyFontRect(48.0f, 246.0f, tSize.fX, 3.0f, &tRect);
	return pDemo->tProxy.shapeRectFill(&pDemo->tProxy, pDemo->pTarget, tRect, XUI_COLOR_RGBA(128, 209, 255, 255));
}

static int __xuiProxyFontCreateAssets(xui_proxy_font_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	xge_resource_t tTtfResource;
	const char* sTtfPath;
	char arrXrfPath[1024];
	int iRet;

	sTtfPath = __xuiProxyFontFindTtf();
	if ( sTtfPath == NULL ) {
		printf("xui_proxy_font: no usable system TTF font found\n");
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	iRet = __xuiProxyFontFullPath(DEMO_XRF_FILE, arrXrfPath, (int)sizeof(arrXrfPath));
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_font: xrf full path failed: %d\n", iRet);
		return iRet;
	}
	iRet = __xuiProxyFontCreateXrf(sTtfPath, arrXrfPath);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_font: create xrf failed: %d\n", iRet);
		return iRet;
	}

	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pXrfFont, arrXrfPath, 0.0f, XUI_FONT_FORMAT_XRF);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_font: xrf fontLoadFile failed: %d\n", iRet);
		return iRet;
	}
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pTtfFont, sTtfPath, 24.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_font: ttf fontLoadFile failed: %d\n", iRet);
		return iRet;
	}
	memset(&tTtfResource, 0, sizeof(tTtfResource));
	iRet = xgeResourceLoad(sTtfPath, &tTtfResource);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_font: ttf resource load failed: %d\n", iRet);
		return iRet;
	}
	iRet = pDemo->tProxy.fontLoadMemory(&pDemo->tProxy, &pDemo->pSmallFont, tTtfResource.pData, tTtfResource.iSize, 16.0f, XUI_FONT_FORMAT_TTF);
	xgeResourceFree(&tTtfResource);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_font: small fontLoadMemory failed: %d\n", iRet);
		return iRet;
	}
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pLargeFont, sTtfPath, 40.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_font: large fontLoadFile failed: %d\n", iRet);
		return iRet;
	}

	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_font: target surfaceCreate failed: %d\n", iRet);
	}
	return iRet;
}

static void __xuiProxyFontDestroyAssets(xui_proxy_font_demo_t* pDemo)
{
	pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
	pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pLargeFont);
	pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pSmallFont);
	pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pTtfFont);
	pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pXrfFont);
}

static int __xuiProxyFontFrame(void* pUser)
{
	xui_proxy_font_demo_t* pDemo;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;

	pDemo = (xui_proxy_font_demo_t*)pUser;
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xuiProxyFontDrawAll(pDemo);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	xgeClear(XGE_COLOR_RGBA(10, 12, 15, 255));
	__xuiProxyFontRect(0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H, &tSrc);
	__xuiProxyFontRect(20.0f, 20.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H, &tDst);
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
	xui_proxy_font_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiProxyFontParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) {
		return 0;
	}
	if ( iRet != XGE_OK ) {
		__xuiProxyFontUsage();
		return 1;
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 860;
	tDesc.iHeight = 460;
	tDesc.sTitle = "xui_proxy_font";
	tDesc.iFlags = XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_proxy_font: xgeInit failed: %d\n", iRet);
		return 1;
	}

	tDemo.tProxy = xuiProxyXge();
	iRet = __xuiProxyFontCreateAssets(&tDemo);
	if ( iRet != XGE_OK ) {
		__xuiProxyFontDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}

	iRet = xgeRun(__xuiProxyFontFrame, &tDemo);
	__xuiProxyFontDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK) ? 0 : 1;
}
