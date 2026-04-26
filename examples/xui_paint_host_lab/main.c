#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const unsigned char g_tgaImage[] = {
	0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 32, 40,
	40, 80, 220, 255, 60, 210, 80, 220, 220, 120, 60, 180, 240, 240, 240, 255
};

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_xui_host_t tHost;
	xge_font_t tFont;
	xge_texture_t tTexture;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pHostPanelWidget;
	xge_xui_widget pImageWidget;
	xge_xui_widget pClipPanelWidget;
	xge_xui_widget pClipChildWidget;
	xge_xui_widget pCustomWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_panel_t tHostPanel;
	xge_xui_image_t tImage;
	int bFontReady;
	int bTextureReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iDrawRectCalls;
	int iDrawImageCalls;
	int iDrawTextCalls;
	int iMeasureCalls;
	int iClipSetCalls;
	int iClipClearCalls;
	int iRefreshCallbackCalls;
	int iCustomPaintCalls;
	int iPaintCountLast;
	int iPaintCountFirst;
	int iPaintFlushFirst;
	int iRefreshBaseline;
	int bManualRefreshRaised;
	int bRefreshCleared;
	int bHostOK;
	int bPaintOK;
	int bClipOK;
	int bCustomPaintOK;
	int bRefreshOK;
	int bStaticChecked;
} app_state_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simhei.ttf",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], 18.0f) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-paint-host-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-paint-host-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static int LoadTexture(app_state_t* pApp)
{
	if ( xgeTextureLoadMemory(&pApp->tTexture, g_tgaImage, (int)sizeof(g_tgaImage)) != XGE_OK ) {
		printf("xui-paint-host-lab texture load failed\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pApp->bTextureReady = 1;
	return XGE_OK;
}

static void HostDrawRect(xge_rect_t tRect, uint32_t iColor, void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	pApp->iDrawRectCalls++;
	xgeShapeRectFillPx(tRect, iColor);
}

static void HostDrawImage(const xge_draw_t* pDraw, void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	pApp->iDrawImageCalls++;
	xgeDrawEx(pDraw);
}

static void HostDrawTextRect(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags, void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	pApp->iDrawTextCalls++;
	xgeTextDrawRect(pFont, sText, tRect, iColor, iFlags);
}

static xge_vec2_t HostMeasureText(xge_font pFont, const char* sText, void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	pApp->iMeasureCalls++;
	return xgeTextMeasure(pFont, sText);
}

static void HostClipSet(xge_rect_t tRect, void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	pApp->iClipSetCalls++;
	xgeClipSet(tRect);
}

static void HostClipClear(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	pApp->iClipClearCalls++;
	xgeClipClear();
}

static void HostRequestRefresh(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	pApp->iRefreshCallbackCalls++;
}

static void CustomPaint(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;
	xge_rect_t tRect;

	pApp = (app_state_t*)pUser;
	if ( (pApp == NULL) || (pWidget == NULL) ) {
		return;
	}
	pApp->iCustomPaintCalls++;
	tRect = pWidget->tContentRect;
	xgeShapeRectStrokePx(tRect, 2.0f, XGE_COLOR_RGBA(255, 208, 96, 255));
	xgeShapeLinePx(tRect.fX, tRect.fY, tRect.fX + tRect.fW, tRect.fY + tRect.fH, 2.0f, XGE_COLOR_RGBA(96, 220, 176, 255));
	xgeShapeLinePx(tRect.fX + tRect.fW, tRect.fY, tRect.fX, tRect.fY + tRect.fH, 2.0f, XGE_COLOR_RGBA(96, 164, 255, 255));
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRoot;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	tRoot.fX = 20.0f;
	tRoot.fY = 20.0f;
	tRoot.fW = (float)iWidth - 40.0f;
	tRoot.fH = (float)iHeight - 40.0f;
	if ( tRoot.fW < 420.0f ) {
		tRoot.fW = 420.0f;
	}
	if ( tRoot.fH < 220.0f ) {
		tRoot.fH = 220.0f;
	}
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 38.0f });
	xgeXuiWidgetSetRect(pApp->pHostPanelWidget, (xge_rect_t){ 20.0f, 56.0f, 192.0f, 118.0f });
	xgeXuiWidgetSetRect(pApp->pImageWidget, (xge_rect_t){ 232.0f, 56.0f, 92.0f, 92.0f });
	xgeXuiWidgetSetRect(pApp->pClipPanelWidget, (xge_rect_t){ 344.0f, 56.0f, 124.0f, 76.0f });
	xgeXuiWidgetSetRect(pApp->pClipChildWidget, (xge_rect_t){ -18.0f, 12.0f, 188.0f, 42.0f });
	xgeXuiWidgetSetRect(pApp->pCustomWidget, (xge_rect_t){ 232.0f, 162.0f, 236.0f, 74.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"host=%d paint=%d clip=%d custom=%d refresh=%d cmds=%d flush=%d host(rect=%d image=%d text=%d measure=%d clip=%d/%d refresh=%d)",
		pApp->bHostOK,
		pApp->bPaintOK,
		pApp->bClipOK,
		pApp->bCustomPaintOK,
		pApp->bRefreshOK,
		pApp->iPaintCountLast,
		pApp->tXui.iPaintFlushCount,
		pApp->iDrawRectCalls,
		pApp->iDrawImageCalls,
		pApp->iDrawTextCalls,
		pApp->iMeasureCalls,
		pApp->iClipSetCalls,
		pApp->iClipClearCalls,
		pApp->iRefreshCallbackCalls);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_theme_t tTheme;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	memset(&pApp->tHost, 0, sizeof(pApp->tHost));
	pApp->tHost.draw_rect = HostDrawRect;
	pApp->tHost.draw_image = HostDrawImage;
	pApp->tHost.draw_text_rect = HostDrawTextRect;
	pApp->tHost.measure_text = HostMeasureText;
	pApp->tHost.clip_set = HostClipSet;
	pApp->tHost.clip_clear = HostClipClear;
	pApp->tHost.request_refresh = HostRequestRefresh;
	pApp->tHost.pUser = pApp;
	xgeXuiSetHost(&pApp->tXui, &pApp->tHost);

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(236, 240, 248, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(24, 30, 42, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(32, 40, 54, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(92, 164, 255, 255);
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 8.0f;
	tTheme.fRadius = 6.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pHostPanelWidget = xgeXuiWidgetCreate();
	pApp->pImageWidget = xgeXuiWidgetCreate();
	pApp->pClipPanelWidget = xgeXuiWidgetCreate();
	pApp->pClipChildWidget = xgeXuiWidgetCreate();
	pApp->pCustomWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pHostPanelWidget == NULL) || (pApp->pImageWidget == NULL) || (pApp->pClipPanelWidget == NULL) || (pApp->pClipChildWidget == NULL) || (pApp->pCustomWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(24, 30, 42, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 58, 76, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui paint host lab");
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);

	xgeXuiPanelInit(&pApp->tHostPanel, pApp->pHostPanelWidget);
	xgeXuiPanelSetTitle(&pApp->tHostPanel, pFont, "Host + paint command counters");
	xgeXuiPanelSetBackground(&pApp->tHostPanel, XGE_COLOR_RGBA(48, 66, 92, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pHostPanelWidget, 10.0f, 10.0f, 10.0f, 10.0f);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHostPanelWidget);

	xgeXuiImageInit(&pApp->tImage, pApp->pImageWidget, pApp->bTextureReady ? &pApp->tTexture : NULL);
	xgeXuiImageSetMode(&pApp->tImage, XGE_XUI_IMAGE_FIT);
	xgeXuiWidgetSetBackground(pApp->pImageWidget, XGE_COLOR_RGBA(30, 38, 54, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pImageWidget, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pImageWidget);

	xgeXuiWidgetSetBackground(pApp->pClipPanelWidget, XGE_COLOR_RGBA(56, 44, 74, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pClipPanelWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetClip(pApp->pClipPanelWidget, 1);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pClipPanelWidget);

	xgeXuiWidgetSetBackground(pApp->pClipChildWidget, XGE_COLOR_RGBA(240, 132, 98, 255));
	xgeXuiWidgetSetRadius(pApp->pClipChildWidget, 6.0f);
	xgeXuiWidgetAdd(pApp->pClipPanelWidget, pApp->pClipChildWidget);

	xgeXuiWidgetSetBackground(pApp->pCustomWidget, XGE_COLOR_RGBA(34, 46, 62, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pCustomWidget, 10.0f, 10.0f, 10.0f, 10.0f);
	xgeXuiWidgetSetPaint(pApp->pCustomWidget, CustomPaint, pApp);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pCustomWidget);
	return XGE_OK;
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	(void)LoadFont(pApp);
	(void)LoadTexture(pApp);
	if ( CreateUI(pApp) != XGE_OK ) {
		xgeXuiUnit(&pApp->tXui);
		if ( pApp->bTextureReady ) {
			xgeTextureFree(&pApp->tTexture);
		}
		if ( pApp->bFontReady ) {
			xgeFontFree(&pApp->tFont);
		}
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiImageUnit(&pApp->tImage);
	xgeXuiPanelUnit(&pApp->tHostPanel);
	if ( pApp->bTextureReady ) {
		xgeTextureFree(&pApp->tTexture);
	}
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	xgeXuiUnit(&pApp->tXui);
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	pApp->iPaintCountLast = xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	if ( pApp->bStaticChecked == 0 ) {
		pApp->iPaintCountFirst = pApp->tXui.iPaintCommandCount;
		pApp->iPaintFlushFirst = pApp->tXui.iPaintFlushCount;
		pApp->bHostOK = (xgeXuiGetHost(&pApp->tXui) == &pApp->tHost) && (pApp->iMeasureCalls > 0);
		pApp->bPaintOK =
			(pApp->iPaintCountFirst > 0) &&
			(pApp->iPaintFlushFirst >= pApp->iPaintCountFirst) &&
			(pApp->iDrawRectCalls > 0) &&
			(pApp->iDrawImageCalls > 0) &&
			(pApp->iDrawTextCalls > 0);
		pApp->bClipOK = (pApp->iClipSetCalls > 0) && (pApp->iClipClearCalls > 0);
		pApp->bCustomPaintOK = (pApp->iCustomPaintCalls > 0);
		pApp->bRefreshCleared = (xgeXuiRefreshNeeded(&pApp->tXui) == 0);
		pApp->iRefreshBaseline = pApp->iRefreshCallbackCalls;
		xgeXuiRefreshRequest(&pApp->tXui);
		pApp->bManualRefreshRaised =
			(xgeXuiRefreshNeeded(&pApp->tXui) != 0) &&
			(pApp->iRefreshCallbackCalls > pApp->iRefreshBaseline);
		xgeXuiWidgetMarkPaint(pApp->pCustomWidget);
		pApp->bStaticChecked = 1;
	} else if ( pApp->bRefreshOK == 0 ) {
		pApp->bRefreshOK =
			pApp->bRefreshCleared &&
			pApp->bManualRefreshRaised &&
			(xgeXuiRefreshNeeded(&pApp->tXui) == 0);
	}

	UpdateStatus(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-paint-host-lab final-summary frames=%d host=%d paint=%d clip=%d custom=%d refresh=%d cmds=%d flush=%d rect=%d image=%d text=%d measure=%d clip_calls=%d/%d refresh_calls=%d custom_calls=%d texture=%d font=%d\n",
			pApp->iFrameCount,
			pApp->bHostOK,
			pApp->bPaintOK,
			pApp->bClipOK,
			pApp->bCustomPaintOK,
			pApp->bRefreshOK,
			pApp->iPaintCountFirst,
			pApp->iPaintFlushFirst,
			pApp->iDrawRectCalls,
			pApp->iDrawImageCalls,
			pApp->iDrawTextCalls,
			pApp->iMeasureCalls,
			pApp->iClipSetCalls,
			pApp->iClipClearCalls,
			pApp->iRefreshCallbackCalls,
			pApp->iCustomPaintCalls,
			pApp->bTextureReady,
			pApp->bFontReady);
		printf("xui-paint-host-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
		xgeQuit();
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int iFrameLimit;
	int i;

	memset(&tDesc, 0, sizeof(tDesc));
	iFrameLimit = ArgInt(getenv("XGE_XUI_PAINT_HOST_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			iFrameLimit = ArgInt(argv[++i], iFrameLimit);
		}
	}

	tDesc.iWidth = 520;
	tDesc.iHeight = 270;
	tDesc.sTitle = "XGE XUI Paint Host Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		printf("xui-paint-host-lab init failed\n");
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	AppUnit(&tApp);
	xgeUnit();
	return 0;
}

