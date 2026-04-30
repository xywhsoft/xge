#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t app_state_t;

typedef struct hyperlink_palette_t {
	uint32_t iNormal;
	uint32_t iHover;
	uint32_t iActive;
	uint32_t iFocus;
	uint32_t iVisited;
	uint32_t iDisabled;
} hyperlink_palette_t;

typedef struct hyperlink_item_t {
	app_state_t* pApp;
	xge_xui_widget pWidget;
	xge_xui_button_t tButton;
	const char* sName;
	const char* sUrl;
	hyperlink_palette_t tPalette;
	int bTrackVisited;
	int bVisited;
	int iOpenCount;
	int iPointerEnterCount;
	int iPointerLeaveCount;
	int iFocusInCount;
	int iFocusOutCount;
	int bHoverSeen;
	int bActiveSeen;
	int bFocusSeen;
} hyperlink_item_t;

struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pNoteWidget;
	xge_xui_widget pPanelWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tNoteLabel;
	xge_xui_panel_t tPanel;
	hyperlink_item_t tDefaultLink;
	hyperlink_item_t tCustomLink;
	hyperlink_item_t tTransientLink;
	hyperlink_item_t tDisabledLink;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bDefaultOK;
	int bCustomOK;
	int bTransientOK;
	int bDisabledOK;
	int bMouseOK;
	int bKeyboardOK;
	int bVisitedOK;
} ;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static void MakeMouseEvent(xge_event_t* pEvent, int iType, int iButton, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = iButton;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeKeyEvent(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static xge_vec2_t WidgetCenter(xge_xui_widget pWidget)
{
	xge_vec2_t tCenter;

	tCenter.fX = pWidget->tRect.fX + (pWidget->tRect.fW * 0.5f);
	tCenter.fY = pWidget->tRect.fY + (pWidget->tRect.fH * 0.5f);
	return tCenter;
}

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-hyperlink-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-hyperlink-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void HyperLinkOpen(xge_xui_widget pWidget, void* pUser)
{
	hyperlink_item_t* pLink;

	(void)pWidget;
	pLink = (hyperlink_item_t*)pUser;
	if ( pLink == NULL ) {
		return;
	}
	pLink->iOpenCount++;
	if ( pLink->bTrackVisited ) {
		pLink->bVisited = 1;
	}
}

static int HyperLinkCapture(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	hyperlink_item_t* pLink;

	(void)pWidget;
	pLink = (hyperlink_item_t*)pUser;
	if ( (pLink == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	switch ( pEvent->iType ) {
		case XGE_EVENT_XUI_POINTER_ENTER:
			pLink->iPointerEnterCount++;
			break;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			pLink->iPointerLeaveCount++;
			break;

		case XGE_EVENT_XUI_FOCUS_IN:
			pLink->iFocusInCount++;
			break;

		case XGE_EVENT_XUI_FOCUS_OUT:
			pLink->iFocusOutCount++;
			break;

		default:
			break;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static void SetLinkPalette(hyperlink_item_t* pLink, hyperlink_palette_t tPalette)
{
	pLink->tPalette = tPalette;
}

static void RefreshLinkVisual(hyperlink_item_t* pLink)
{
	int iState;
	uint32_t iTextColor;

	if ( pLink == NULL ) {
		return;
	}
	iState = xgeXuiButtonGetState(&pLink->tButton);
	if ( (iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		iTextColor = pLink->tPalette.iDisabled;
	} else if ( (iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		iTextColor = pLink->tPalette.iActive;
	} else if ( (iState & XGE_XUI_STATE_HOVER) != 0 ) {
		iTextColor = pLink->tPalette.iHover;
	} else if ( (iState & XGE_XUI_STATE_FOCUS) != 0 ) {
		iTextColor = pLink->tPalette.iFocus;
	} else if ( pLink->bTrackVisited && pLink->bVisited ) {
		iTextColor = pLink->tPalette.iVisited;
	} else {
		iTextColor = pLink->tPalette.iNormal;
	}
	if ( (iState & XGE_XUI_STATE_HOVER) != 0 ) {
		pLink->bHoverSeen = 1;
	}
	if ( (iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		pLink->bActiveSeen = 1;
	}
	if ( (iState & XGE_XUI_STATE_FOCUS) != 0 ) {
		pLink->bFocusSeen = 1;
	}
	xgeXuiButtonSetTextColor(&pLink->tButton, iTextColor);
}

static void RefreshAllLinks(app_state_t* pApp)
{
	RefreshLinkVisual(&pApp->tDefaultLink);
	RefreshLinkVisual(&pApp->tCustomLink);
	RefreshLinkVisual(&pApp->tTransientLink);
	RefreshLinkVisual(&pApp->tDisabledLink);
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
	tRoot.fX = 24.0f;
	tRoot.fY = 20.0f;
	tRoot.fW = (float)iWidth - 48.0f;
	tRoot.fH = (float)iHeight - 40.0f;
	if ( tRoot.fW < 560.0f ) {
		tRoot.fW = 560.0f;
	}
	if ( tRoot.fH < 300.0f ) {
		tRoot.fH = 300.0f;
	}
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pPanelWidget, (xge_rect_t){ 0.0f, 56.0f, tRoot.fW, 188.0f });
	xgeXuiWidgetSetRect(pApp->tDefaultLink.pWidget, (xge_rect_t){ 28.0f, 42.0f, tRoot.fW - 56.0f, 28.0f });
	xgeXuiWidgetSetRect(pApp->tCustomLink.pWidget, (xge_rect_t){ 28.0f, 80.0f, tRoot.fW - 56.0f, 28.0f });
	xgeXuiWidgetSetRect(pApp->tTransientLink.pWidget, (xge_rect_t){ 28.0f, 118.0f, tRoot.fW - 56.0f, 28.0f });
	xgeXuiWidgetSetRect(pApp->tDisabledLink.pWidget, (xge_rect_t){ 28.0f, 156.0f, tRoot.fW - 56.0f, 28.0f });
	xgeXuiWidgetSetRect(pApp->pNoteWidget, (xge_rect_t){ 0.0f, tRoot.fH - 54.0f, tRoot.fW, 54.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[320];

	snprintf(
		sText,
		sizeof(sText),
		"default=%d custom=%d transient=%d disabled=%d mouse=%d key=%d visited=%d open=%d/%d/%d/%d",
		pApp->bDefaultOK,
		pApp->bCustomOK,
		pApp->bTransientOK,
		pApp->bDisabledOK,
		pApp->bMouseOK,
		pApp->bKeyboardOK,
		pApp->bVisitedOK,
		pApp->tDefaultLink.iOpenCount,
		pApp->tCustomLink.iOpenCount,
		pApp->tTransientLink.iOpenCount,
		pApp->tDisabledLink.iOpenCount);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int InitLink(hyperlink_item_t* pLink, app_state_t* pApp, xge_xui_widget pWidget, const char* sName, const char* sUrl, int bTrackVisited)
{
	if ( (pLink == NULL) || (pApp == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pLink, 0, sizeof(*pLink));
	pLink->pApp = pApp;
	pLink->pWidget = pWidget;
	pLink->sName = sName;
	pLink->sUrl = sUrl;
	pLink->bTrackVisited = bTrackVisited;
	if ( xgeXuiButtonInit(&pLink->tButton, &pApp->tXui, pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiButtonSetText(&pLink->tButton, pApp->bFontReady ? &pApp->tFont : NULL, sUrl);
	xgeXuiButtonSetColors(
		&pLink->tButton,
		XGE_COLOR_RGBA(0, 0, 0, 0),
		XGE_COLOR_RGBA(0, 0, 0, 0),
		XGE_COLOR_RGBA(0, 0, 0, 0),
		XGE_COLOR_RGBA(0, 0, 0, 0),
		XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeXuiButtonSetClick(&pLink->tButton, HyperLinkOpen, pLink);
	pLink->tButton.iTextFlags = XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeXuiWidgetSetRadius(pWidget, 0.0f);
	xgeXuiWidgetSetPaddingPx(pWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetCaptureEventUser(pWidget, HyperLinkCapture, pLink);
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_theme_t tTheme;
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	tTheme.iTextColor = XGE_COLOR_RGBA(236, 242, 248, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(18, 22, 30, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(30, 36, 48, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(76, 94, 118, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(90, 170, 255, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(42, 128, 232, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(66, 154, 255, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(26, 96, 190, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(255, 214, 104, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(86, 90, 98, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pNoteWidget = xgeXuiWidgetCreate();
	pApp->pPanelWidget = xgeXuiWidgetCreate();
	pApp->tDefaultLink.pWidget = xgeXuiWidgetCreate();
	pApp->tCustomLink.pWidget = xgeXuiWidgetCreate();
	pApp->tTransientLink.pWidget = xgeXuiWidgetCreate();
	pApp->tDisabledLink.pWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pNoteWidget == NULL) || (pApp->pPanelWidget == NULL) || (pApp->tDefaultLink.pWidget == NULL) || (pApp->tCustomLink.pWidget == NULL) || (pApp->tTransientLink.pWidget == NULL) || (pApp->tDisabledLink.pWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(22, 28, 38, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	if ( xgeXuiWidgetAdd(pRoot, pApp->pRootPanel) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(40, 54, 74, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 12.0f, 10.0f, 12.0f, 10.0f);
	xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pApp->bFontReady ? &pApp->tFont : NULL, "xui hyperlink lab");
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanelWidget);
	xgeXuiPanelSetTitle(&pApp->tPanel, pApp->bFontReady ? &pApp->tFont : NULL, "HyperLink semantics via XUI button primitive");
	xgeXuiPanelSetBackground(&pApp->tPanel, XGE_COLOR_RGBA(28, 36, 48, 255));
	xgeXuiPanelSetTitleColor(&pApp->tPanel, XGE_COLOR_RGBA(244, 248, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pPanelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetBackground(pApp->pNoteWidget, XGE_COLOR_RGBA(24, 30, 40, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pNoteWidget, 12.0f, 8.0f, 12.0f, 8.0f);
	xgeXuiLabelInit(&pApp->tNoteLabel, pApp->pNoteWidget, pApp->bFontReady ? &pApp->tFont : NULL, "default visited | custom palette | transient(no visited) | disabled | mouse + Enter/Space");
	xgeXuiLabelSetColor(&pApp->tNoteLabel, XGE_COLOR_RGBA(214, 222, 232, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pNoteWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	if ( InitLink(&pApp->tDefaultLink, pApp, pApp->tDefaultLink.pWidget, "default", "https://xge.dev/default-visited", 1) != XGE_OK ) {
		return XGE_ERROR;
	}
	SetLinkPalette(&pApp->tDefaultLink, (hyperlink_palette_t){
		XGE_COLOR_RGBA(94, 170, 255, 255),
		XGE_COLOR_RGBA(128, 198, 255, 255),
		XGE_COLOR_RGBA(226, 236, 248, 255),
		XGE_COLOR_RGBA(255, 214, 104, 255),
		XGE_COLOR_RGBA(184, 128, 214, 255),
		XGE_COLOR_RGBA(112, 116, 124, 255)
	});
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->tDefaultLink.pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	if ( InitLink(&pApp->tCustomLink, pApp, pApp->tCustomLink.pWidget, "custom", "https://xge.dev/custom-colors", 1) != XGE_OK ) {
		return XGE_ERROR;
	}
	SetLinkPalette(&pApp->tCustomLink, (hyperlink_palette_t){
		XGE_COLOR_RGBA(255, 232, 196, 255),
		XGE_COLOR_RGBA(255, 255, 255, 255),
		XGE_COLOR_RGBA(244, 188, 108, 255),
		XGE_COLOR_RGBA(255, 216, 112, 255),
		XGE_COLOR_RGBA(176, 118, 210, 255),
		XGE_COLOR_RGBA(112, 116, 124, 255)
	});
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->tCustomLink.pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	if ( InitLink(&pApp->tTransientLink, pApp, pApp->tTransientLink.pWidget, "transient", "https://xge.dev/transient-no-visited", 0) != XGE_OK ) {
		return XGE_ERROR;
	}
	SetLinkPalette(&pApp->tTransientLink, (hyperlink_palette_t){
		XGE_COLOR_RGBA(112, 224, 156, 255),
		XGE_COLOR_RGBA(154, 242, 184, 255),
		XGE_COLOR_RGBA(220, 245, 228, 255),
		XGE_COLOR_RGBA(126, 232, 164, 255),
		XGE_COLOR_RGBA(112, 224, 156, 255),
		XGE_COLOR_RGBA(112, 116, 124, 255)
	});
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->tTransientLink.pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	if ( InitLink(&pApp->tDisabledLink, pApp, pApp->tDisabledLink.pWidget, "disabled", "https://xge.dev/disabled-link", 1) != XGE_OK ) {
		return XGE_ERROR;
	}
	SetLinkPalette(&pApp->tDisabledLink, (hyperlink_palette_t){
		XGE_COLOR_RGBA(94, 170, 255, 255),
		XGE_COLOR_RGBA(128, 198, 255, 255),
		XGE_COLOR_RGBA(226, 236, 248, 255),
		XGE_COLOR_RGBA(255, 214, 104, 255),
		XGE_COLOR_RGBA(184, 128, 214, 255),
		XGE_COLOR_RGBA(112, 116, 124, 255)
	});
	xgeXuiWidgetSetEnabled(pApp->tDisabledLink.pWidget, 0);
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->tDisabledLink.pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	RefreshAllLinks(pApp);
	return XGE_OK;
}

static void DispatchAndRefresh(app_state_t* pApp, const xge_event_t* pEvent)
{
	(void)xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	RefreshAllLinks(pApp);
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tCenter;
	int bDefaultInitialOK;
	int bDefaultClickOK;
	int bCustomHoverOK;
	int bCustomMouseOK;
	int bCustomKeyOK;
	int bTransientOK;
	int bDisabledOK;

	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RefreshAllLinks(pApp);

	bDefaultInitialOK =
		(pApp->tDefaultLink.iOpenCount == 0) &&
		(pApp->tDefaultLink.bVisited == 0) &&
		(pApp->tDefaultLink.tButton.iTextColor == pApp->tDefaultLink.tPalette.iNormal);

	tCenter = WidgetCenter(pApp->tDefaultLink.pWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tCenter.fX, tCenter.fY);
	DispatchAndRefresh(pApp, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	DispatchAndRefresh(pApp, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	DispatchAndRefresh(pApp, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, 6.0f, 6.0f);
	DispatchAndRefresh(pApp, &tEvent);
	bDefaultClickOK =
		bDefaultInitialOK &&
		(pApp->tDefaultLink.iOpenCount == 1) &&
		(pApp->tDefaultLink.bVisited != 0) &&
		(pApp->tDefaultLink.tButton.iClickCount == 1) &&
		(pApp->tDefaultLink.iPointerEnterCount >= 1) &&
		(pApp->tDefaultLink.iPointerLeaveCount >= 1) &&
		(pApp->tDefaultLink.bHoverSeen != 0) &&
		(pApp->tDefaultLink.bActiveSeen != 0);

	tCenter = WidgetCenter(pApp->tCustomLink.pWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tCenter.fX, tCenter.fY);
	DispatchAndRefresh(pApp, &tEvent);
	bCustomHoverOK =
		(pApp->tCustomLink.tButton.iTextColor == pApp->tCustomLink.tPalette.iHover) &&
		(pApp->tCustomLink.iPointerEnterCount >= 1);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	DispatchAndRefresh(pApp, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	DispatchAndRefresh(pApp, &tEvent);
	bCustomMouseOK =
		bCustomHoverOK &&
		(pApp->tCustomLink.iOpenCount == 1) &&
		(pApp->tCustomLink.bVisited != 0) &&
		(pApp->tCustomLink.bActiveSeen != 0);
	xgeXuiSetFocus(&pApp->tXui, pApp->tCustomLink.pWidget);
	RefreshAllLinks(pApp);
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	DispatchAndRefresh(pApp, &tEvent);
	xgeXuiSetFocus(&pApp->tXui, NULL);
	RefreshAllLinks(pApp);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, 8.0f, 8.0f);
	DispatchAndRefresh(pApp, &tEvent);
	bCustomKeyOK =
		(pApp->tCustomLink.iOpenCount == 2) &&
		(pApp->tCustomLink.bFocusSeen != 0) &&
		(pApp->tCustomLink.tButton.iClickCount == 2);

	tCenter = WidgetCenter(pApp->tDisabledLink.pWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tCenter.fX, tCenter.fY);
	DispatchAndRefresh(pApp, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	DispatchAndRefresh(pApp, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	DispatchAndRefresh(pApp, &tEvent);
	xgeXuiSetFocus(&pApp->tXui, pApp->tDisabledLink.pWidget);
	RefreshAllLinks(pApp);
	bDisabledOK =
		(pApp->tDisabledLink.iOpenCount == 0) &&
		((xgeXuiButtonGetState(&pApp->tDisabledLink.tButton) & XGE_XUI_STATE_DISABLED) != 0) &&
		(pApp->tDisabledLink.tButton.iTextColor == pApp->tDisabledLink.tPalette.iDisabled);

	tCenter = WidgetCenter(pApp->tTransientLink.pWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tCenter.fX, tCenter.fY);
	DispatchAndRefresh(pApp, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	DispatchAndRefresh(pApp, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	DispatchAndRefresh(pApp, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, 8.0f, 8.0f);
	DispatchAndRefresh(pApp, &tEvent);
	xgeXuiSetFocus(&pApp->tXui, NULL);
	RefreshAllLinks(pApp);
	bTransientOK =
		(pApp->tTransientLink.iOpenCount == 1) &&
		(pApp->tTransientLink.bVisited == 0) &&
		(pApp->tTransientLink.bHoverSeen != 0) &&
		(pApp->tTransientLink.bActiveSeen != 0);

	pApp->bDefaultOK =
		bDefaultClickOK &&
		(pApp->tDefaultLink.tButton.iTextColor == pApp->tDefaultLink.tPalette.iVisited);
	pApp->bCustomOK =
		bCustomMouseOK &&
		bCustomKeyOK &&
		(pApp->tCustomLink.tButton.iTextColor == pApp->tCustomLink.tPalette.iVisited);
	pApp->bTransientOK = bTransientOK;
	pApp->bDisabledOK = bDisabledOK;
	pApp->bMouseOK = bDefaultClickOK && bCustomMouseOK && bTransientOK;
	pApp->bKeyboardOK = bCustomKeyOK;
	pApp->bVisitedOK =
		(pApp->tDefaultLink.bVisited != 0) &&
		(pApp->tCustomLink.bVisited != 0) &&
		(pApp->tTransientLink.bVisited == 0) &&
		(pApp->tDefaultLink.tButton.iTextColor == pApp->tDefaultLink.tPalette.iVisited) &&
		(pApp->tCustomLink.tButton.iTextColor == pApp->tCustomLink.tPalette.iVisited);
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
	(void)LoadFont(pApp);
	return RunStaticChecks(pApp);
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiButtonUnit(&pApp->tDisabledLink.tButton);
	xgeXuiButtonUnit(&pApp->tTransientLink.tButton);
	xgeXuiButtonUnit(&pApp->tCustomLink.tButton);
	xgeXuiButtonUnit(&pApp->tDefaultLink.tButton);
	xgeXuiPanelUnit(&pApp->tPanel);
	xgeXuiLabelUnit(&pApp->tNoteLabel);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
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
	RefreshAllLinks(pApp);
	UpdateStatus(pApp);
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-hyperlink-lab final-summary frames=%d default=%d custom=%d transient=%d disabled=%d mouse=%d keyboard=%d visited=%d open=%d/%d/%d/%d visited_flags=%d/%d/%d disabled_state=%d focus=%d/%d enter=%d/%d leave=%d/%d\n",
			pApp->iFrameCount,
			pApp->bDefaultOK,
			pApp->bCustomOK,
			pApp->bTransientOK,
			pApp->bDisabledOK,
			pApp->bMouseOK,
			pApp->bKeyboardOK,
			pApp->bVisitedOK,
			pApp->tDefaultLink.iOpenCount,
			pApp->tCustomLink.iOpenCount,
			pApp->tTransientLink.iOpenCount,
			pApp->tDisabledLink.iOpenCount,
			pApp->tDefaultLink.bVisited,
			pApp->tCustomLink.bVisited,
			pApp->tTransientLink.bVisited,
			(xgeXuiButtonGetState(&pApp->tDisabledLink.tButton) & XGE_XUI_STATE_DISABLED) != 0,
			pApp->tDefaultLink.iFocusOutCount,
			pApp->tCustomLink.iFocusInCount,
			pApp->tDefaultLink.iPointerEnterCount,
			pApp->tCustomLink.iPointerEnterCount,
			pApp->tDefaultLink.iPointerLeaveCount,
			pApp->tTransientLink.iPointerLeaveCount);
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
	iFrameLimit = ArgInt(getenv("XGE_XUI_HYPERLINK_LAB_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			iFrameLimit = ArgInt(argv[++i], iFrameLimit);
		}
	}

	tDesc.iWidth = 780;
	tDesc.iHeight = 380;
	tDesc.sTitle = "XGE XUI HyperLink Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		printf("xui-hyperlink-lab init failed\n");
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	AppUnit(&tApp);
	xgeUnit();
	return (tApp.bDefaultOK && tApp.bCustomOK && tApp.bTransientOK && tApp.bDisabledOK && tApp.bMouseOK && tApp.bKeyboardOK && tApp.bVisitedOK) ? 0 : 3;
}

