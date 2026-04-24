#include "../../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pOwnerWidget;
	xge_xui_widget pHintWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tOwnerLabel;
	xge_xui_label_t tHint;
	xge_xui_tooltip_t tTooltip;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iLegacyCaptureCount;
	int bInitOK;
	int bOpenCloseOK;
	int bOffsetOK;
	int bEnabledOK;
	int bTextOK;
	int bDirectProcOK;
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

static int FloatNear(float fA, float fB, float fEpsilon)
{
	return fabsf(fA - fB) <= fEpsilon;
}

static void MakeMouseEvent(xge_event_t* pEvent, int iType, int iButton, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = iButton;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static xge_vec2_t WidgetPoint(xge_xui_widget pWidget, float fRatioX, float fRatioY)
{
	xge_vec2_t tPoint;

	tPoint.fX = pWidget->tRect.fX + (pWidget->tRect.fW * fRatioX);
	tPoint.fY = pWidget->tRect.fY + (pWidget->tRect.fH * fRatioY);
	return tPoint;
}

static int LegacyCapture(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( (pApp != NULL) && (pEvent != NULL) && (pEvent->iType == XGE_EVENT_MOUSE_MOVE) ) {
		pApp->iLegacyCaptureCount++;
	}
	return XGE_XUI_EVENT_CONTINUE;
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
			printf("xui-tooltip-owner-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-tooltip-owner-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRoot;
	float fInnerW;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}

	tRoot.fX = 18.0f;
	tRoot.fY = 18.0f;
	tRoot.fW = (float)iWidth - 36.0f;
	tRoot.fH = (float)iHeight - 36.0f;
	if ( tRoot.fW < 620.0f ) {
		tRoot.fW = 620.0f;
	}
	if ( tRoot.fH < 320.0f ) {
		tRoot.fH = 320.0f;
	}
	fInnerW = tRoot.fW - 40.0f;

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pOwnerWidget, (xge_rect_t){ 24.0f, 84.0f, 240.0f, 44.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 24.0f, tRoot.fH - 46.0f, fInnerW, 24.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d open=%d offset=%d enabled=%d text=%d direct=%d legacy=%d",
		pApp->bInitOK,
		pApp->bOpenCloseOK,
		pApp->bOffsetOK,
		pApp->bEnabledOK,
		pApp->bTextOK,
		pApp->bDirectProcOK,
		pApp->iLegacyCaptureCount);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_theme_t tTheme;
	xge_xui_widget pRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(238, 242, 248, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(20, 26, 34, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(34, 42, 56, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 96, 118, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(102, 170, 255, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(50, 64, 84, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(68, 84, 108, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(44, 58, 78, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(54, 74, 104, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(78, 82, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pOwnerWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pOwnerWidget == NULL) || (pApp->pHintWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(24, 30, 40, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pOwnerWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(46, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pOwnerWidget, XGE_COLOR_RGBA(54, 72, 98, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pOwnerWidget, 12.0f, 10.0f, 12.0f, 10.0f);
	xgeXuiWidgetSetFocusable(pApp->pOwnerWidget, 1);
	xgeXuiWidgetSetCaptureEventUser(pApp->pOwnerWidget, LegacyCapture, pApp);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui tooltip owner lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tOwnerLabel, pApp->pOwnerWidget, pFont, "Tooltip target") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tOwnerLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tHint, pApp->pHintWidget, pFont, "Auto-check covers owner capture chain, open/close, text gate, enabled gate and offset relayout.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tHint, XGE_COLOR_RGBA(164, 182, 206, 255));

	if ( xgeXuiTooltipInit(&pApp->tTooltip, &pApp->tXui, pApp->pOwnerWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tPoint;
	xge_rect_t tRect;
	int iRet;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	pApp->bInitOK =
		(pApp->tTooltip.pOwner == pApp->pOwnerWidget) &&
		(pApp->tTooltip.procOldCapture == LegacyCapture) &&
		(pApp->tTooltip.pOldCaptureUser == pApp) &&
		(pApp->pOwnerWidget->procCaptureEvent == xgeXuiTooltipOwnerEventProc) &&
		(pApp->pOwnerWidget->pCaptureUser == &pApp->tTooltip) &&
		(pApp->tTooltip.bEnabled != 0);

	xgeXuiTooltipSetText(&pApp->tTooltip, pApp->bFontReady ? &pApp->tFont : NULL, "Tooltip text");
	xgeXuiTooltipSetColors(&pApp->tTooltip, XGE_COLOR_RGBA(38, 48, 66, 245), XGE_COLOR_RGBA(255, 248, 224, 255));
	pApp->bTextOK =
		(pApp->tTooltip.sText != NULL) &&
		(strcmp(pApp->tTooltip.sText, "Tooltip text") == 0) &&
		(pApp->tTooltip.iBackgroundColor == XGE_COLOR_RGBA(38, 48, 66, 245)) &&
		(pApp->tTooltip.iTextColor == XGE_COLOR_RGBA(255, 248, 224, 255));

	tPoint = WidgetPoint(pApp->pOwnerWidget, 0.5f, 0.5f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tPoint.fX, tPoint.fY);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	tRect = xgeXuiWidgetGetRect(pApp->tTooltip.pPopupWidget);
	pApp->bOpenCloseOK =
		(iRet == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiTooltipIsOpen(&pApp->tTooltip) != 0) &&
		(pApp->iLegacyCaptureCount == 1) &&
		(xgeXuiWidgetIsVisible(pApp->tTooltip.pPopupWidget) != 0) &&
		(tRect.fW > 0.0f) &&
		(tRect.fH > 0.0f);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tPoint.fX, tPoint.fY);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bOpenCloseOK =
		pApp->bOpenCloseOK &&
		(iRet == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiTooltipIsOpen(&pApp->tTooltip) == 0);

	xgeXuiTooltipSetOffset(&pApp->tTooltip, 14.0f, 10.0f);
	xgeXuiTooltipSetOpen(&pApp->tTooltip, 1);
	tRect = xgeXuiWidgetGetRect(pApp->tTooltip.pPopupWidget);
	pApp->bOffsetOK =
		(xgeXuiTooltipIsOpen(&pApp->tTooltip) != 0) &&
		FloatNear(tRect.fX, pApp->pOwnerWidget->tRect.fX + 14.0f, 0.01f) &&
		FloatNear(tRect.fY, pApp->pOwnerWidget->tRect.fY + pApp->pOwnerWidget->tRect.fH + 10.0f, 0.01f);

	xgeXuiTooltipSetEnabled(&pApp->tTooltip, 0);
	pApp->bEnabledOK =
		(pApp->tTooltip.bEnabled == 0) &&
		(xgeXuiTooltipIsOpen(&pApp->tTooltip) == 0);
	xgeXuiTooltipSetOpen(&pApp->tTooltip, 1);
	pApp->bEnabledOK =
		pApp->bEnabledOK &&
		(xgeXuiTooltipIsOpen(&pApp->tTooltip) == 0);
	xgeXuiTooltipSetEnabled(&pApp->tTooltip, 1);
	xgeXuiTooltipSetText(&pApp->tTooltip, pApp->bFontReady ? &pApp->tFont : NULL, "");
	xgeXuiTooltipSetOpen(&pApp->tTooltip, 1);
	pApp->bTextOK =
		pApp->bTextOK &&
		(xgeXuiTooltipIsOpen(&pApp->tTooltip) == 0);
	xgeXuiTooltipSetText(&pApp->tTooltip, pApp->bFontReady ? &pApp->tFont : NULL, "Tooltip text");

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tPoint.fX, tPoint.fY);
	iRet = xgeXuiTooltipOwnerEventProc(pApp->pOwnerWidget, &tEvent, &pApp->tTooltip);
	pApp->bDirectProcOK =
		(iRet == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiTooltipIsOpen(&pApp->tTooltip) != 0) &&
		(pApp->iLegacyCaptureCount == 2);
	MakeMouseEvent(&tEvent, XGE_EVENT_XUI_POINTER_LEAVE, 0, tPoint.fX, tPoint.fY);
	(void)xgeXuiTooltipOwnerEventProc(pApp->pOwnerWidget, &tEvent, &pApp->tTooltip);
	pApp->bDirectProcOK =
		pApp->bDirectProcOK &&
		(xgeXuiTooltipIsOpen(&pApp->tTooltip) == 0);

	xgeXuiTooltipSetText(&pApp->tTooltip, pApp->bFontReady ? &pApp->tFont : NULL, "Hover target");
	xgeXuiTooltipSetOffset(&pApp->tTooltip, 14.0f, 10.0f);
	xgeXuiTooltipSetEnabled(&pApp->tTooltip, 1);
	xgeXuiTooltipSetOpen(&pApp->tTooltip, 1);
	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( LoadFont(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	return RunChecks(pApp);
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiTooltipUnit(&pApp->tTooltip);
	xgeXuiLabelUnit(&pApp->tHint);
	xgeXuiLabelUnit(&pApp->tOwnerLabel);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	xgeXuiUnit(&pApp->tXui);
	memset(pApp, 0, sizeof(*pApp));
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	LayoutRoot(pApp);
	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());

	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( pApp->iFrameCount >= pApp->iFrameLimit ) {
		printf(
			"xui-tooltip-owner-lab final-summary frames=%d init=%d open=%d offset=%d enabled=%d text=%d direct=%d legacy=%d open=%d pos=%.2f,%.2f size=%.2f,%.2f\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bOpenCloseOK,
			pApp->bOffsetOK,
			pApp->bEnabledOK,
			pApp->bTextOK,
			pApp->bDirectProcOK,
			pApp->iLegacyCaptureCount,
			xgeXuiTooltipIsOpen(&pApp->tTooltip),
			pApp->tTooltip.pPopupWidget->tRect.fX,
			pApp->tTooltip.pPopupWidget->tRect.fY,
			pApp->tTooltip.pPopupWidget->tRect.fW,
			pApp->tTooltip.pPopupWidget->tRect.fH);
		printf("xui-tooltip-owner-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
		xgeQuit();
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int iFrameLimit;
	int iExitCode;
	int i;

	iFrameLimit = 180;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && (i + 1) < argc ) {
			iFrameLimit = ArgInt(argv[i + 1], iFrameLimit);
			i++;
		}
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 700;
	tDesc.iHeight = 360;
	tDesc.sTitle = "XGE XUI Tooltip Owner Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	iExitCode =
		(tApp.bInitOK && tApp.bOpenCloseOK && tApp.bOffsetOK && tApp.bEnabledOK && tApp.bTextOK && tApp.bDirectProcOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}
