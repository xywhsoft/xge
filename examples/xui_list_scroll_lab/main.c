#include "../../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* g_arrListItems[] = {
	"Alpha",
	"Bravo",
	"Charlie",
	"Delta",
	"Echo",
	"Foxtrot",
	"Golf",
	"Hotel",
	"India",
	"Juliet"
};

static const int g_arrListEnabled[] = {
	1, 0, 1, 1, 0, 1, 1, 0, 1, 1
};

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pScrollPanelWidget;
	xge_xui_widget pListPanelWidget;
	xge_xui_widget pScrollCaptionWidget;
	xge_xui_widget pScrollWidget;
	xge_xui_widget pListCaptionWidget;
	xge_xui_widget pListWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tScrollCaption;
	xge_xui_label_t tListCaption;
	xge_xui_panel_t tScrollPanel;
	xge_xui_panel_t tListPanel;
	xge_xui_scroll_view_t tScrollView;
	xge_xui_list_view_t tListView;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iListSelectCount;
	int iLastListSelected;
	int bScrollViewOK;
	int bListViewOK;
	int bDisabledOK;
	int bHoverOK;
	int bKeyboardOK;
	int bScrollOpsOK;
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

static float ClampFloat(float fValue, float fMin, float fMax)
{
	if ( fValue < fMin ) {
		return fMin;
	}
	if ( fValue > fMax ) {
		return fMax;
	}
	return fValue;
}

static void MakeMouseEvent(xge_event_t* pEvent, int iType, int iButton, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = iButton;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeWheelEvent(xge_event_t* pEvent, float fX, float fY, float fDX, float fDY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_MOUSE_WHEEL;
	pEvent->fX = fX;
	pEvent->fY = fY;
	pEvent->fDX = fDX;
	pEvent->fDY = fDY;
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

	tCenter.fX = pWidget->tRect.fX + pWidget->tRect.fW * 0.5f;
	tCenter.fY = pWidget->tRect.fY + pWidget->tRect.fH * 0.5f;
	return tCenter;
}

static xge_rect_t ScrollViewVerticalThumbRect(xge_xui_scroll_view pScroll)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fVisible;
	float fTrackLen;
	float fLen;
	float fMaxScroll;

	tBar.fX = pScroll->pWidget->tContentRect.fX + pScroll->pWidget->tContentRect.fW - 4.0f;
	tBar.fY = pScroll->pWidget->tContentRect.fY;
	tBar.fW = 4.0f;
	tBar.fH = pScroll->pWidget->tContentRect.fH;
	tThumb = tBar;
	fVisible = pScroll->pWidget->tContentRect.fH;
	fTrackLen = tBar.fH;
	if ( pScroll->fContentH <= fVisible ) {
		return tThumb;
	}
	fLen = fTrackLen * (fVisible / pScroll->fContentH);
	if ( fLen < 8.0f ) {
		fLen = 8.0f;
	}
	if ( fLen > fTrackLen ) {
		fLen = fTrackLen;
	}
	tThumb.fH = fLen;
	fMaxScroll = pScroll->fContentH - fVisible;
	if ( fMaxScroll > 0.0f && fTrackLen > fLen ) {
		tThumb.fY += (fTrackLen - fLen) * (pScroll->fScrollY / fMaxScroll);
	}
	return tThumb;
}

static xge_rect_t ScrollViewHorizontalThumbRect(xge_xui_scroll_view pScroll)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fVisible;
	float fTrackLen;
	float fLen;
	float fMaxScroll;

	tBar.fX = pScroll->pWidget->tContentRect.fX;
	tBar.fY = pScroll->pWidget->tContentRect.fY + pScroll->pWidget->tContentRect.fH - 4.0f;
	tBar.fW = pScroll->pWidget->tContentRect.fW;
	tBar.fH = 4.0f;
	tThumb = tBar;
	fVisible = pScroll->pWidget->tContentRect.fW;
	fTrackLen = tBar.fW;
	if ( pScroll->fContentW <= fVisible ) {
		return tThumb;
	}
	fLen = fTrackLen * (fVisible / pScroll->fContentW);
	if ( fLen < 8.0f ) {
		fLen = 8.0f;
	}
	if ( fLen > fTrackLen ) {
		fLen = fTrackLen;
	}
	tThumb.fW = fLen;
	fMaxScroll = pScroll->fContentW - fVisible;
	if ( fMaxScroll > 0.0f && fTrackLen > fLen ) {
		tThumb.fX += (fTrackLen - fLen) * (pScroll->fScrollX / fMaxScroll);
	}
	return tThumb;
}

static xge_rect_t ListViewThumbRect(xge_xui_list_view pList)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fContentH;
	float fTrackLen;
	float fLen;
	float fMaxScroll;

	tBar.fX = pList->pWidget->tContentRect.fX + pList->pWidget->tContentRect.fW - 4.0f;
	tBar.fY = pList->pWidget->tContentRect.fY;
	tBar.fW = 4.0f;
	tBar.fH = pList->pWidget->tContentRect.fH;
	tThumb = tBar;
	fContentH = (float)pList->iItemCount * pList->fItemHeight;
	fTrackLen = tBar.fH;
	if ( fContentH <= pList->pWidget->tContentRect.fH ) {
		return tThumb;
	}
	fLen = fTrackLen * (pList->pWidget->tContentRect.fH / fContentH);
	if ( fLen < 8.0f ) {
		fLen = 8.0f;
	}
	if ( fLen > fTrackLen ) {
		fLen = fTrackLen;
	}
	tThumb.fH = fLen;
	fMaxScroll = fContentH - pList->pWidget->tContentRect.fH;
	if ( fMaxScroll > 0.0f && fTrackLen > fLen ) {
		tThumb.fY += (fTrackLen - fLen) * (pList->fScrollY / fMaxScroll);
	}
	return tThumb;
}

static int NextEnabledListIndex(int iStart, int iStep)
{
	int i;
	int iCount;

	iCount = (int)(sizeof(g_arrListItems) / sizeof(g_arrListItems[0]));
	if ( iStep == 0 ) {
		iStep = 1;
	}
	if ( iStart < 0 ) {
		iStart = (iStep > 0) ? 0 : (iCount - 1);
	}
	if ( iStart >= iCount ) {
		iStart = (iStep > 0) ? (iCount - 1) : 0;
	}
	for ( i = iStart; (i >= 0) && (i < iCount); i += iStep ) {
		if ( g_arrListEnabled[i] != 0 ) {
			return i;
		}
	}
	return -1;
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
			printf("xui-list-scroll-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-list-scroll-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void ListSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iListSelectCount++;
		pApp->iLastListSelected = iIndex;
	}
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRoot;
	float fPanelW;
	float fPanelH;
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
	if ( tRoot.fW < 720.0f ) {
		tRoot.fW = 720.0f;
	}
	if ( tRoot.fH < 360.0f ) {
		tRoot.fH = 360.0f;
	}
	fPanelW = (tRoot.fW - 54.0f) * 0.5f;
	fPanelH = tRoot.fH - 78.0f;

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 38.0f });
	xgeXuiWidgetSetRect(pApp->pScrollPanelWidget, (xge_rect_t){ 18.0f, 58.0f, fPanelW, fPanelH });
	xgeXuiWidgetSetRect(pApp->pListPanelWidget, (xge_rect_t){ 36.0f + fPanelW, 58.0f, fPanelW, fPanelH });
	xgeXuiWidgetSetRect(pApp->pScrollCaptionWidget, (xge_rect_t){ 18.0f, 36.0f, fPanelW - 36.0f, 20.0f });
	xgeXuiWidgetSetRect(pApp->pScrollWidget, (xge_rect_t){ 18.0f, 58.0f, fPanelW - 36.0f, fPanelH - 78.0f });
	xgeXuiWidgetSetRect(pApp->pListCaptionWidget, (xge_rect_t){ 18.0f, 36.0f, fPanelW - 36.0f, 20.0f });
	xgeXuiWidgetSetRect(pApp->pListWidget, (xge_rect_t){ 18.0f, 58.0f, fPanelW - 36.0f, 152.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"scroll=%d list=%d disabled=%d hover=%d keys=%d scroll_ops=%d select=%d last=%d",
		pApp->bScrollViewOK,
		pApp->bListViewOK,
		pApp->bDisabledOK,
		pApp->bHoverOK,
		pApp->bKeyboardOK,
		pApp->bScrollOpsOK,
		pApp->iListSelectCount,
		pApp->iLastListSelected);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_theme_t tTheme;
	xge_xui_widget pRoot;
	xge_font pFont;
	uint32_t iCaptionColor;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(240, 244, 250, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(20, 26, 36, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(32, 40, 54, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 98, 122, 255);
	tTheme.iSelectionColor = XGE_COLOR_RGBA(96, 164, 255, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(96, 214, 144, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(56, 124, 214, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(78, 146, 236, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(42, 96, 176, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(86, 108, 144, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(84, 86, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pScrollPanelWidget = xgeXuiWidgetCreate();
	pApp->pListPanelWidget = xgeXuiWidgetCreate();
	pApp->pScrollCaptionWidget = xgeXuiWidgetCreate();
	pApp->pScrollWidget = xgeXuiWidgetCreate();
	pApp->pListCaptionWidget = xgeXuiWidgetCreate();
	pApp->pListWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pScrollPanelWidget == NULL) || (pApp->pListPanelWidget == NULL) || (pApp->pScrollCaptionWidget == NULL) || (pApp->pScrollWidget == NULL) || (pApp->pListCaptionWidget == NULL) || (pApp->pListWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(22, 28, 38, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	if ( xgeXuiWidgetAdd(pRoot, pApp->pRootPanel) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui list scroll lab");
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiPanelInit(&pApp->tScrollPanel, pApp->pScrollPanelWidget);
	xgeXuiPanelSetTitle(&pApp->tScrollPanel, pFont, "ScrollView");
	xgeXuiPanelSetBackground(&pApp->tScrollPanel, XGE_COLOR_RGBA(32, 40, 54, 255));
	xgeXuiPanelSetTitleColor(&pApp->tScrollPanel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pScrollPanelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiPanelInit(&pApp->tListPanel, pApp->pListPanelWidget);
	xgeXuiPanelSetTitle(&pApp->tListPanel, pFont, "ListView");
	xgeXuiPanelSetBackground(&pApp->tListPanel, XGE_COLOR_RGBA(32, 40, 54, 255));
	xgeXuiPanelSetTitleColor(&pApp->tListPanel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pListPanelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	iCaptionColor = XGE_COLOR_RGBA(198, 208, 224, 255);
	xgeXuiLabelInit(&pApp->tScrollCaption, pApp->pScrollCaptionWidget, pFont, "wheel, drag, thumb drag, page jump");
	xgeXuiLabelSetColor(&pApp->tScrollCaption, iCaptionColor);
	if ( xgeXuiWidgetAdd(pApp->pScrollPanelWidget, pApp->pScrollCaptionWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiScrollViewInit(&pApp->tScrollView, &pApp->tXui, pApp->pScrollWidget);
	if ( xgeXuiWidgetAdd(pApp->pScrollPanelWidget, pApp->pScrollWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiLabelInit(&pApp->tListCaption, pApp->pListCaptionWidget, pFont, "disabled items, hover, up/down, page up/down");
	xgeXuiLabelSetColor(&pApp->tListCaption, iCaptionColor);
	if ( xgeXuiWidgetAdd(pApp->pListPanelWidget, pApp->pListCaptionWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiListViewInit(&pApp->tListView, &pApp->tXui, pApp->pListWidget);
	if ( xgeXuiWidgetAdd(pApp->pListPanelWidget, pApp->pListWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tThumb;
	xge_vec2_t tCenter;
	float fX;
	float fY;
	float fMaxX;
	float fMaxY;
	float fMaxListScroll;
	int iVisibleRows;
	int iExpectedPageDown;
	int iExpectedPageUp;
	int bScrollDefaultsOK;
	int bListDefaultsOK;
	int bScrollViewOpsOK;
	int bListScrollOpsOK;

	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	bScrollDefaultsOK =
		(xgeXuiWidgetIsFocusable(pApp->pScrollWidget) != 0) &&
		((pApp->pScrollWidget->iFlags & XGE_XUI_WIDGET_CLIP) != 0) &&
		(pApp->tScrollView.iBackgroundColor == XGE_COLOR_RGBA(24, 28, 34, 255)) &&
		(pApp->tScrollView.iBarColor == XGE_COLOR_RGBA(64, 72, 84, 180)) &&
		(pApp->tScrollView.iThumbColor == XGE_COLOR_RGBA(160, 172, 188, 220));
	xgeXuiScrollViewSetContentSize(&pApp->tScrollView, 520.0f, 480.0f);
	xgeXuiScrollViewSetColors(&pApp->tScrollView, XGE_COLOR_RGBA(28, 34, 42, 255), XGE_COLOR_RGBA(74, 86, 102, 200), XGE_COLOR_RGBA(196, 214, 238, 245));
	fMaxX = 520.0f - pApp->pScrollWidget->tContentRect.fW;
	fMaxY = 480.0f - pApp->pScrollWidget->tContentRect.fH;
	xgeXuiScrollViewSetOffset(&pApp->tScrollView, 999.0f, 999.0f);
	xgeXuiScrollViewGetOffset(&pApp->tScrollView, &fX, &fY);
	bScrollViewOpsOK =
		FloatNear(fX, fMaxX, 0.01f) &&
		FloatNear(fY, fMaxY, 0.01f);
	xgeXuiScrollViewSetOffset(&pApp->tScrollView, 0.0f, 0.0f);
	tCenter = WidgetCenter(pApp->pScrollWidget);
	MakeWheelEvent(&tEvent, tCenter.fX, tCenter.fY, -1.0f, -1.0f);
	xgeXuiScrollViewEvent(&pApp->tScrollView, &tEvent);
	xgeXuiScrollViewGetOffset(&pApp->tScrollView, &fX, &fY);
	bScrollViewOpsOK =
		bScrollViewOpsOK &&
		(fX > 0.0f) &&
		(fY > 0.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiScrollViewEvent(&pApp->tScrollView, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tCenter.fX - 24.0f, tCenter.fY - 18.0f);
	xgeXuiScrollViewEvent(&pApp->tScrollView, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX - 24.0f, tCenter.fY - 18.0f);
	xgeXuiScrollViewEvent(&pApp->tScrollView, &tEvent);
	xgeXuiScrollViewGetOffset(&pApp->tScrollView, &fX, &fY);
	bScrollViewOpsOK =
		bScrollViewOpsOK &&
		(fX > 32.0f) &&
		(fY > 32.0f) &&
		(pApp->tScrollView.bDragging == 0);
	xgeXuiScrollViewSetOffset(&pApp->tScrollView, 0.0f, 0.0f);
	tThumb = ScrollViewVerticalThumbRect(&pApp->tScrollView);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tThumb.fX + 1.0f, tThumb.fY + tThumb.fH + 20.0f);
	xgeXuiScrollViewEvent(&pApp->tScrollView, &tEvent);
	xgeXuiScrollViewGetOffset(&pApp->tScrollView, &fX, &fY);
	bScrollViewOpsOK =
		bScrollViewOpsOK &&
		(fY > 64.0f);
	xgeXuiScrollViewSetOffset(&pApp->tScrollView, 0.0f, 0.0f);
	tThumb = ScrollViewHorizontalThumbRect(&pApp->tScrollView);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tThumb.fX + 1.0f, tThumb.fY + 1.0f);
	xgeXuiScrollViewEvent(&pApp->tScrollView, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tThumb.fX + 60.0f, tThumb.fY + 1.0f);
	xgeXuiScrollViewEvent(&pApp->tScrollView, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tThumb.fX + 60.0f, tThumb.fY + 1.0f);
	xgeXuiScrollViewEvent(&pApp->tScrollView, &tEvent);
	xgeXuiScrollViewGetOffset(&pApp->tScrollView, &fX, &fY);
	pApp->bScrollViewOK =
		bScrollDefaultsOK &&
		bScrollViewOpsOK &&
		(pApp->tScrollView.iBackgroundColor == XGE_COLOR_RGBA(28, 34, 42, 255)) &&
		(pApp->tScrollView.iBarColor == XGE_COLOR_RGBA(74, 86, 102, 200)) &&
		(pApp->tScrollView.iThumbColor == XGE_COLOR_RGBA(196, 214, 238, 245)) &&
		(fX > 80.0f) &&
		(fY < 0.01f);

	bListDefaultsOK =
		(pApp->tListView.iSelected == -1) &&
		(pApp->tListView.iHover == -1) &&
		FloatNear(pApp->tListView.fItemHeight, 24.0f, 0.01f) &&
		(pApp->tListView.iBackgroundColor == XGE_COLOR_RGBA(24, 28, 34, 255)) &&
		(pApp->tListView.iRowColor == XGE_COLOR_RGBA(36, 42, 50, 255));
	xgeXuiListViewSetFont(&pApp->tListView, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiListViewSetItems(&pApp->tListView, g_arrListItems, (int)(sizeof(g_arrListItems) / sizeof(g_arrListItems[0])));
	xgeXuiListViewSetEnabledItems(&pApp->tListView, g_arrListEnabled, (int)(sizeof(g_arrListEnabled) / sizeof(g_arrListEnabled[0])));
	xgeXuiListViewSetItemHeight(&pApp->tListView, 24.0f);
	xgeXuiListViewSetSelect(&pApp->tListView, ListSelect, pApp);
	xgeXuiListViewSetColors(&pApp->tListView, XGE_COLOR_RGBA(24, 28, 34, 255), XGE_COLOR_RGBA(46, 54, 66, 255), XGE_COLOR_RGBA(72, 132, 208, 255), XGE_COLOR_RGBA(248, 250, 252, 255), XGE_COLOR_RGBA(74, 86, 102, 200), XGE_COLOR_RGBA(196, 214, 238, 245));
	xgeXuiListViewSetDisabledTextColor(&pApp->tListView, XGE_COLOR_RGBA(128, 138, 150, 220));
	fMaxListScroll = (float)(sizeof(g_arrListItems) / sizeof(g_arrListItems[0])) * pApp->tListView.fItemHeight - pApp->pListWidget->tContentRect.fH;
	xgeXuiListViewSetScroll(&pApp->tListView, 999.0f);
	bListScrollOpsOK =
		FloatNear(xgeXuiListViewGetScroll(&pApp->tListView), fMaxListScroll, 0.01f);
	xgeXuiListViewSetScroll(&pApp->tListView, 0.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, pApp->pListWidget->tContentRect.fX + 20.0f, pApp->pListWidget->tContentRect.fY + 60.0f);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	pApp->bHoverOK = (pApp->tListView.iHover == 2);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_XUI_POINTER_LEAVE;
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	pApp->bHoverOK = pApp->bHoverOK && (pApp->tListView.iHover == -1);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, pApp->pListWidget->tContentRect.fX + 20.0f, pApp->pListWidget->tContentRect.fY + 36.0f);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	pApp->bDisabledOK = (xgeXuiListViewGetSelected(&pApp->tListView) == -1);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, pApp->pListWidget->tContentRect.fX + 20.0f, pApp->pListWidget->tContentRect.fY + 12.0f);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	pApp->bListViewOK =
		bListDefaultsOK &&
		(xgeXuiListViewGetSelected(&pApp->tListView) == 0) &&
		(pApp->iListSelectCount == 1) &&
		(pApp->iLastListSelected == 0) &&
		(pApp->tListView.pFont == (pApp->bFontReady ? &pApp->tFont : NULL)) &&
		(pApp->tListView.iDisabledTextColor == XGE_COLOR_RGBA(128, 138, 150, 220));
	MakeWheelEvent(&tEvent, pApp->pListWidget->tContentRect.fX + 8.0f, pApp->pListWidget->tContentRect.fY + 8.0f, 0.0f, -1.0f);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	bListScrollOpsOK =
		bListScrollOpsOK &&
		FloatNear(xgeXuiListViewGetScroll(&pApp->tListView), 24.0f, 0.01f);
	xgeXuiListViewSetScroll(&pApp->tListView, 0.0f);
	tThumb = ListViewThumbRect(&pApp->tListView);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tThumb.fX + 1.0f, tThumb.fY + tThumb.fH + 18.0f);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	bListScrollOpsOK =
		bListScrollOpsOK &&
		FloatNear(xgeXuiListViewGetScroll(&pApp->tListView), fMaxListScroll, 0.01f);
	xgeXuiListViewSetScroll(&pApp->tListView, 0.0f);
	tThumb = ListViewThumbRect(&pApp->tListView);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tThumb.fX + 1.0f, tThumb.fY + 1.0f);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tThumb.fX + 1.0f, tThumb.fY + 34.0f);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tThumb.fX + 1.0f, tThumb.fY + 34.0f);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	bListScrollOpsOK =
		bListScrollOpsOK &&
		(xgeXuiListViewGetScroll(&pApp->tListView) > 12.0f) &&
		(pApp->tListView.bDraggingThumb == 0);

	xgeXuiListViewSetScroll(&pApp->tListView, 0.0f);
	xgeXuiListViewSetSelected(&pApp->tListView, 0);
	xgeXuiSetFocus(&pApp->tXui, pApp->pListWidget);
	iVisibleRows = (int)(pApp->pListWidget->tContentRect.fH / pApp->tListView.fItemHeight);
	if ( iVisibleRows < 1 ) {
		iVisibleRows = 1;
	}
	iExpectedPageDown = NextEnabledListIndex(0 + iVisibleRows, 1);
	iExpectedPageUp = NextEnabledListIndex(iExpectedPageDown - iVisibleRows, -1);
	MakeKeyEvent(&tEvent, XGE_KEY_DOWN);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	pApp->bKeyboardOK = (xgeXuiListViewGetSelected(&pApp->tListView) == 2);
	MakeKeyEvent(&tEvent, XGE_KEY_UP);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	pApp->bKeyboardOK = pApp->bKeyboardOK && (xgeXuiListViewGetSelected(&pApp->tListView) == 0);
	MakeKeyEvent(&tEvent, XGE_KEY_PAGE_DOWN);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	pApp->bKeyboardOK =
		pApp->bKeyboardOK &&
		(xgeXuiListViewGetSelected(&pApp->tListView) == iExpectedPageDown) &&
		(xgeXuiListViewGetScroll(&pApp->tListView) >= 0.0f);
	MakeKeyEvent(&tEvent, XGE_KEY_PAGE_UP);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	pApp->bKeyboardOK = pApp->bKeyboardOK && (xgeXuiListViewGetSelected(&pApp->tListView) == iExpectedPageUp);
	MakeKeyEvent(&tEvent, XGE_KEY_END);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	pApp->bKeyboardOK = pApp->bKeyboardOK && (xgeXuiListViewGetSelected(&pApp->tListView) == 9);
	MakeKeyEvent(&tEvent, XGE_KEY_HOME);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	pApp->bKeyboardOK = pApp->bKeyboardOK && (xgeXuiListViewGetSelected(&pApp->tListView) == 0);
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	xgeXuiListViewEvent(&pApp->tListView, &tEvent);
	pApp->bKeyboardOK =
		pApp->bKeyboardOK &&
		(pApp->iListSelectCount >= 8) &&
		(pApp->iLastListSelected == 0);
	pApp->bScrollOpsOK = bScrollViewOpsOK && bListScrollOpsOK;

	UpdateStatus(pApp);
	return XGE_OK;
}

static void DrawScrollViewContent(app_state_t* pApp)
{
	xge_rect_t tClip;
	xge_rect_t tItem;
	xge_rect_t tText;
	float fScrollX;
	float fScrollY;
	int i;
	uint32_t iStripe;

	if ( pApp->bFontReady == 0 ) {
		return;
	}
	xgeXuiScrollViewGetOffset(&pApp->tScrollView, &fScrollX, &fScrollY);
	tClip = pApp->pScrollWidget->tContentRect;
	if ( tClip.fW > 6.0f ) {
		tClip.fW -= 6.0f;
	}
	if ( tClip.fH > 6.0f ) {
		tClip.fH -= 6.0f;
	}
	xgeClipSet(tClip);
	for ( i = 0; i < 12; i++ ) {
		tItem.fX = pApp->pScrollWidget->tContentRect.fX + 12.0f + (float)((i % 3) * 150) - fScrollX;
		tItem.fY = pApp->pScrollWidget->tContentRect.fY + 12.0f + (float)(i * 34) - fScrollY;
		tItem.fW = 126.0f;
		tItem.fH = 24.0f;
		iStripe = (i % 2 == 0) ? XGE_COLOR_RGBA(54, 70, 92, 255) : XGE_COLOR_RGBA(40, 54, 72, 255);
		xgeShapeRectFillPx(tItem, iStripe);
		xgeShapeRectStrokePx(tItem, 1.0f, XGE_COLOR_RGBA(96, 112, 142, 255));
		tText = tItem;
		tText.fX += 8.0f;
		tText.fW -= 16.0f;
		xgeTextDrawRect(&pApp->tFont, g_arrListItems[i % 10], tText, XGE_COLOR_RGBA(248, 250, 252, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	xgeClipClear();
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	pApp->iLastListSelected = -1;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	(void)LoadFont(pApp);
	return RunStaticChecks(pApp);
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiListViewUnit(&pApp->tListView);
	xgeXuiScrollViewUnit(&pApp->tScrollView);
	xgeXuiPanelUnit(&pApp->tListPanel);
	xgeXuiPanelUnit(&pApp->tScrollPanel);
	xgeXuiLabelUnit(&pApp->tListCaption);
	xgeXuiLabelUnit(&pApp->tScrollCaption);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	xgeXuiUnit(&pApp->tXui);
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;
	float fScrollX;
	float fScrollY;

	pApp = (app_state_t*)pUser;
	LayoutRoot(pApp);
	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiPaint(&pApp->tXui);
	DrawScrollViewContent(pApp);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( pApp->iFrameCount >= pApp->iFrameLimit ) {
		xgeXuiScrollViewGetOffset(&pApp->tScrollView, &fScrollX, &fScrollY);
		printf(
			"xui-list-scroll-lab final-summary frames=%d scroll=%d list=%d disabled=%d hover=%d keys=%d scroll_ops=%d scroll=%.2f,%.2f list(selected=%d hover=%d scroll=%.2f cb=%d last=%d item_h=%.2f)\n",
			pApp->iFrameCount,
			pApp->bScrollViewOK,
			pApp->bListViewOK,
			pApp->bDisabledOK,
			pApp->bHoverOK,
			pApp->bKeyboardOK,
			pApp->bScrollOpsOK,
			fScrollX,
			fScrollY,
			xgeXuiListViewGetSelected(&pApp->tListView),
			pApp->tListView.iHover,
			xgeXuiListViewGetScroll(&pApp->tListView),
			pApp->iListSelectCount,
			pApp->iLastListSelected,
			pApp->tListView.fItemHeight);
		printf("xui-list-scroll-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	iFrameLimit = ArgInt(getenv("XGE_XUI_LIST_SCROLL_FRAMES"), 180);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			iFrameLimit = ArgInt(argv[++i], iFrameLimit);
		}
	}

	tDesc.iWidth = 780;
	tDesc.iHeight = 420;
	tDesc.sTitle = "XGE XUI List Scroll Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		printf("xui-list-scroll-lab init failed\n");
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	AppUnit(&tApp);
	xgeUnit();
	return 0;
}
