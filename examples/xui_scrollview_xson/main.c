#include "../../xge.h"
#include "../xui_demo_style.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XSON_BUFFER_SIZE 24576

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_font_t tFont;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bLayoutOK;
	int bCoordinateOK;
	int bXsonOK;
	int bEventOK;
	char sXson[XSON_BUFFER_SIZE];
} app_state_t;

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent);

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
			printf("xui_scrollview_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static int AppendText(char* sBuffer, int* pOffset, int iCapacity, const char* sText)
{
	int iLen;

	if ( (sBuffer == NULL) || (pOffset == NULL) || (sText == NULL) || (*pOffset >= iCapacity) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iLen = (int)strlen(sText);
	if ( (*pOffset + iLen) >= iCapacity ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(sBuffer + *pOffset, sText, (size_t)iLen);
	*pOffset += iLen;
	sBuffer[*pOffset] = 0;
	return XGE_OK;
}

static int BuildXson(app_state_t* pApp)
{
	int iOffset;

	iOffset = 0;
	pApp->sXson[0] = 0;
	return AppendText(
		pApp->sXson,
		&iOffset,
		XSON_BUFFER_SIZE,
		"{\"xui\":1,"
		"\"styles\":{"
		"\"root\":{\"type\":\"panel\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":18,\"background\":\"#E5EBF4FF\"},"
		"\"panel\":{\"type\":\"panel\",\"width\":\"100%\",\"height\":\"100%\",\"background\":\"#F6F9FCFF\",\"borderColor\":\"#58A0DCFF\",\"borderWidth\":1},"
		"\"status\":{\"type\":\"label\",\"height\":34,\"textColor\":\"#2A3748FF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
		"\"scroll\":{\"type\":\"scrollView\",\"background\":\"#EBF2FAFF\",\"borderColor\":\"#82A0C2FF\",\"borderWidth\":1,\"contentSize\":[1120,820],\"offset\":[120,80],\"wheelAxis\":\"both\",\"contentDrag\":true,\"barColor\":\"#D2E4F2FF\",\"thumbColor\":\"#4E8CC6FF\"},"
		"\"scrollCompact\":{\"type\":\"scrollView\",\"background\":\"#F2F8F4FF\",\"borderColor\":\"#82A0C2FF\",\"borderWidth\":1,\"contentSize\":[1120,420],\"offset\":[120,70],\"wheelAxis\":\"both\",\"contentDrag\":true,\"scrollbarMode\":\"compact\",\"barColor\":\"#DCE8DCFF\",\"thumbColor\":\"#529260FF\"},"
		"\"card\":{\"type\":\"label\",\"width\":260,\"height\":42,\"padding\":[8,6,8,6],\"background\":\"#F8FAFDE6\",\"borderColor\":\"#AAB8CAFF\",\"borderWidth\":1,\"textColor\":\"#2A3748FF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
		"\"action\":{\"type\":\"button\",\"width\":160,\"height\":36,\"color\":\"#3A7ECCFF\",\"hoverColor\":\"#4B92E0FF\",\"activeColor\":\"#2460AAFF\",\"textColor\":\"#F6FAFFFF\"}"
		"},"
		"\"tree\":{\"type\":\"panel\",\"id\":\"root\",\"style\":\"root\",\"children\":["
		"{\"type\":\"panel\",\"id\":\"shell\",\"style\":\"panel\",\"children\":["
		"{\"type\":\"label\",\"id\":\"status\",\"style\":\"status\",\"anchor\":{\"left\":24,\"top\":20},\"width\":840,\"text\":\"XSON ScrollView: content size, offset, wheel axis and child widgets\"},"
		"{\"type\":\"scrollView\",\"id\":\"scroll\",\"style\":\"scroll\",\"anchor\":{\"left\":24,\"top\":66},\"width\":840,\"height\":520,\"children\":["
		"{\"type\":\"label\",\"id\":\"card0\",\"style\":\"card\",\"anchor\":{\"left\":40,\"top\":38},\"text\":\"content origin: 40, 38\"},"
		"{\"type\":\"label\",\"id\":\"card1\",\"style\":\"card\",\"anchor\":{\"left\":420,\"top\":96},\"text\":\"label loaded from XSON\"},"
		"{\"type\":\"label\",\"id\":\"card2\",\"style\":\"card\",\"anchor\":{\"left\":760,\"top\":250},\"text\":\"large content surface\"},"
		"{\"type\":\"label\",\"id\":\"card3\",\"style\":\"card\",\"anchor\":{\"left\":120,\"top\":610},\"width\":300,\"text\":\"screen -> viewport -> content\"},"
		"{\"type\":\"label\",\"id\":\"card4\",\"style\":\"card\",\"anchor\":{\"left\":820,\"top\":720},\"width\":220,\"text\":\"bottom-right child\"},"
		"{\"type\":\"button\",\"id\":\"action0\",\"style\":\"action\",\"anchor\":{\"left\":520,\"top\":360},\"text\":\"Child Button\"},"
		"{\"type\":\"button\",\"id\":\"action1\",\"style\":\"action\",\"anchor\":{\"left\":880,\"top\":640},\"text\":\"Ensure Target\"}"
		"]},"
		"{\"type\":\"scrollView\",\"id\":\"scrollCompact\",\"style\":\"scrollCompact\",\"anchor\":{\"left\":24,\"top\":606},\"width\":840,\"height\":120,\"children\":["
		"{\"type\":\"label\",\"id\":\"compact0\",\"style\":\"card\",\"anchor\":{\"left\":40,\"top\":36},\"text\":\"compact scrollbar mode\"},"
		"{\"type\":\"label\",\"id\":\"compact1\",\"style\":\"card\",\"anchor\":{\"left\":420,\"top\":128},\"text\":\"thin bars, same content mapping\"},"
		"{\"type\":\"label\",\"id\":\"compact2\",\"style\":\"card\",\"anchor\":{\"left\":820,\"top\":340},\"width\":220,\"text\":\"compact target\"}"
		"]}"
		"]}"
		"]}}");
}

static int CreateUI(app_state_t* pApp)
{
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( BuildXson(pApp) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, pApp->sXson, (int)strlen(pApp->sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_scrollview_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_widget pShell;
	xge_xui_widget pStatus;
	xge_xui_widget pScroll;
	xge_xui_widget pCompactScroll;
	float fRootW;
	float fRootH;
	float fFullH;
	float fCompactH;
	float fGap;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	pRoot = xgeXuiPageFind(&pApp->tPage, "root");
	pShell = xgeXuiPageFind(&pApp->tPage, "shell");
	pStatus = xgeXuiPageFind(&pApp->tPage, "status");
	pScroll = xgeXuiPageFind(&pApp->tPage, "scroll");
	pCompactScroll = xgeXuiPageFind(&pApp->tPage, "scrollCompact");
	if ( (pRoot == NULL) || (pShell == NULL) || (pStatus == NULL) || (pScroll == NULL) || (pCompactScroll == NULL) ) {
		return;
	}
	fRootW = (float)iWidth;
	fRootH = (float)iHeight;
	if ( fRootW < 860.0f ) {
		fRootW = 860.0f;
	}
	if ( fRootH < 640.0f ) {
		fRootH = 640.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	xgeXuiWidgetSetRect(pShell, (xge_rect_t){ 18.0f, 18.0f, fRootW - 36.0f, fRootH - 36.0f });
	xgeXuiWidgetSetRect(pStatus, (xge_rect_t){ 24.0f, 20.0f, fRootW - 84.0f, 34.0f });
	fGap = 22.0f;
	fCompactH = 170.0f;
	fFullH = fRootH - 128.0f - fCompactH - fGap;
	if ( fFullH < 280.0f ) {
		fFullH = 280.0f;
	}
	xgeXuiWidgetSetRect(pScroll, (xge_rect_t){ 24.0f, 66.0f, fRootW - 84.0f, fFullH });
	xgeXuiWidgetSetRect(pCompactScroll, (xge_rect_t){ 24.0f, 66.0f + fFullH + fGap, fRootW - 84.0f, fCompactH });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void DrawScrollContent(app_state_t* pApp)
{
	xge_xui_widget pScrollWidget;
	xge_xui_scroll_view pScroll;
	xge_rect_t tClip;
	xge_rect_t tRect;
	float fX;
	float fY;
	float x;
	float y;
	int i;

	pScrollWidget = xgeXuiPageFind(&pApp->tPage, "scroll");
	pScroll = (pApp->tPage.iScrollViewCount > 0) ? &pApp->tPage.arrScrollView[0] : NULL;
	if ( (pScrollWidget == NULL) || (pScroll == NULL) ) {
		return;
	}
	xgeXuiScrollViewGetOffset(pScroll, &fX, &fY);
	tClip = pScroll->tFrame.tViewportRect;
	xgeClipSet(tClip);
	for ( x = 0.0f; x <= 1120.0f; x += 80.0f ) {
		xgeShapeLinePx(tClip.fX + x - fX, tClip.fY - fY, tClip.fX + x - fX, tClip.fY + 820.0f - fY, 1.0f, XGE_COLOR_RGBA(198, 214, 232, 255));
	}
	for ( y = 0.0f; y <= 820.0f; y += 80.0f ) {
		xgeShapeLinePx(tClip.fX - fX, tClip.fY + y - fY, tClip.fX + 1120.0f - fX, tClip.fY + y - fY, 1.0f, XGE_COLOR_RGBA(198, 214, 232, 255));
	}
	for ( i = 0; i < 6; i++ ) {
		tRect.fX = tClip.fX + 62.0f + (float)(i * 154) - fX;
		tRect.fY = tClip.fY + 166.0f + (float)((i % 3) * 118) - fY;
		tRect.fW = 120.0f;
		tRect.fH = 72.0f;
		xgeShapeRectFillPx(tRect, (i % 2) ? XGE_COLOR_RGBA(218, 236, 248, 255) : XGE_COLOR_RGBA(230, 246, 238, 255));
		xgeShapeRectStrokePx(tRect, 1.0f, XGE_COLOR_RGBA(112, 146, 184, 255));
	}
	xgeClipClear();
}

static void RunChecks(app_state_t* pApp)
{
	xge_xui_widget pScrollWidget;
	xge_xui_widget pCompactWidget;
	xge_xui_widget pAction;
	xge_xui_scroll_view pScroll;
	xge_xui_scroll_view pCompactScroll;
	float fViewX;
	float fViewY;
	float fContentX;
	float fContentY;

	pScrollWidget = xgeXuiPageFind(&pApp->tPage, "scroll");
	pCompactWidget = xgeXuiPageFind(&pApp->tPage, "scrollCompact");
	pAction = xgeXuiPageFind(&pApp->tPage, "action1");
	pScroll = (pApp->tPage.iScrollViewCount > 0) ? &pApp->tPage.arrScrollView[0] : NULL;
	pCompactScroll = (pApp->tPage.iScrollViewCount > 1) ? &pApp->tPage.arrScrollView[1] : NULL;
	pApp->bCreateOK =
		(pScrollWidget != NULL) &&
		(pCompactWidget != NULL) &&
		(pScroll != NULL) &&
		(pCompactScroll != NULL) &&
		(pApp->tPage.iScrollViewCount == 2) &&
		(pApp->tPage.iLabelCount >= 9) &&
		(pApp->tPage.iButtonCount >= 2);
	if ( (pScrollWidget == NULL) || (pCompactWidget == NULL) || (pScroll == NULL) || (pCompactScroll == NULL) ) {
		pApp->bLayoutOK = 0;
		pApp->bCoordinateOK = 0;
		pApp->bXsonOK = 0;
		return;
	}
	xgeXuiScrollModelScreenToViewport(&pScroll->tModel, pScroll->tFrame.tViewportRect.fX, pScroll->tFrame.tViewportRect.fY, &fViewX, &fViewY);
	xgeXuiScrollModelScreenToContent(&pScroll->tModel, pScroll->tFrame.tViewportRect.fX, pScroll->tFrame.tViewportRect.fY, &fContentX, &fContentY);
	pApp->bLayoutOK = (pScrollWidget->tContentRect.fW > 500.0f) && (pScrollWidget->tContentRect.fH > 360.0f);
	pApp->bCoordinateOK =
		FloatNear(fViewX, 0.0f, 0.01f) &&
		FloatNear(fViewY, 0.0f, 0.01f) &&
		FloatNear(fContentX, pScroll->tModel.fScrollX, 0.01f) &&
		FloatNear(fContentY, pScroll->tModel.fScrollY, 0.01f);
	pApp->bXsonOK =
		(pScroll->tModel.fContentW == 1120.0f) &&
		(pScroll->tModel.fContentH == 820.0f) &&
		(pScroll->tFrame.iWheelAxis == XGE_XUI_WHEEL_AXIS_BOTH) &&
		(pScroll->tFrame.bContentDragEnabled != 0) &&
		(xgeXuiScrollViewGetScrollbarMode(pScroll) == XGE_XUI_SCROLLBAR_MODE_FULL) &&
		(xgeXuiScrollViewGetScrollbarMode(pCompactScroll) == XGE_XUI_SCROLLBAR_MODE_COMPACT) &&
		(pCompactScroll->tFrame.tHScrollBar.iMode == XGE_XUI_SCROLLBAR_MODE_COMPACT) &&
		(pCompactScroll->tFrame.tVScrollBar.iMode == XGE_XUI_SCROLLBAR_MODE_COMPACT) &&
		(pCompactScroll->tFrame.bShowHScroll != 0) &&
		(pCompactScroll->tFrame.bShowVScroll != 0) &&
		(pAction != NULL);
}

static void RunEventCheck(app_state_t* pApp)
{
	xge_xui_widget pScrollWidget;
	xge_xui_scroll_view pScroll;
	xge_event_t tEvent;
	float fOldX;
	float fOldY;
	float fNewX;
	float fNewY;

	if ( pApp->bEventOK != 0 ) {
		return;
	}
	pScrollWidget = xgeXuiPageFind(&pApp->tPage, "scroll");
	pScroll = (pApp->tPage.iScrollViewCount > 0) ? &pApp->tPage.arrScrollView[0] : NULL;
	if ( (pScrollWidget == NULL) || (pScroll == NULL) ) {
		return;
	}
	xgeXuiScrollViewGetOffset(pScroll, &fOldX, &fOldY);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.fX = pScrollWidget->tContentRect.fX + 120.0f;
	tEvent.fY = pScrollWidget->tContentRect.fY + 120.0f;
	AppEvent(&pApp->tScene, &tEvent);
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX -= 24.0f;
	tEvent.fY -= 20.0f;
	AppEvent(&pApp->tScene, &tEvent);
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	AppEvent(&pApp->tScene, &tEvent);
	xgeXuiScrollViewGetOffset(pScroll, &fNewX, &fNewY);
	pApp->bEventOK = (fNewX > fOldX) && (fNewY > fOldY);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunChecks(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiPageUnload(&pApp->tPage);
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	return XGE_OK;
}

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return XGE_OK;
	}
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;
	xge_xui_scroll_view pScroll;

	pApp = (app_state_t*)pScene->pUser;
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	RunEventCheck(pApp);
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		pScroll = (pApp->tPage.iScrollViewCount > 0) ? &pApp->tPage.arrScrollView[0] : NULL;
		printf(
			"xui_scrollview_xson final-summary frames=%d create=%d layout=%d coord=%d xson=%d event=%d scroll=%.2f,%.2f content=%.2fx%.2f\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bCoordinateOK,
			pApp->bXsonOK,
			pApp->bEventOK,
			(pScroll != NULL) ? pScroll->tModel.fScrollX : 0.0f,
			(pScroll != NULL) ? pScroll->tModel.fScrollY : 0.0f,
			(pScroll != NULL) ? pScroll->tModel.fContentW : 0.0f,
			(pScroll != NULL) ? pScroll->tModel.fContentH : 0.0f);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(229, 235, 244, 255));
	DrawScrollContent(pApp);
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	static app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_SCROLLVIEW_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 960;
	tDesc.iHeight = 700;
	tDesc.sTitle = "XUI ScrollView XSON";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	tApp.tScene.pUser = &tApp;
	tApp.tScene.onEnter = AppEnter;
	tApp.tScene.onLeave = AppLeave;
	tApp.tScene.onEvent = AppEvent;
	tApp.tScene.onUpdate = AppUpdate;
	tApp.tScene.onDraw = AppDraw;
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	iExitCode = xgeRun(NULL, NULL);
	xgeUnit();
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bCoordinateOK && tApp.bXsonOK && tApp.bEventOK) ? 0 : 3;
}
