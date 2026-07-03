#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_font_t tFont;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bCreateOK;
	int bFieldsOK;
	int bLayoutOK;
	int bFallbackOK;
	int bScrollOK;
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"absolute\",\"width\":\"100%\",\"height\":\"100%\",\"background\":\"#E8F1F8FF\"},"
"\"owner\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"width\":104,\"height\":28,\"textAlign\":\"center\",\"textVAlign\":\"middle\",\"background\":\"#E6F0FAFF\",\"borderWidth\":1,\"borderColor\":\"#5B8FBEFF\",\"textColor\":\"#2D4860FF\"},"
"\"popup\":{\"type\":\"popup\",\"width\":160,\"height\":72,\"background\":\"#F8FAFDFF\",\"borderColor\":\"#5B8FBEFF\",\"gap\":0,\"outsidePolicy\":\"close\",\"escapePolicy\":\"close\",\"focusPolicy\":\"popup\",\"consumeInside\":true},"
"\"content\":{\"type\":\"absolute\",\"width\":\"100%\",\"height\":\"100%\"},"
"\"item\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"height\":24,\"textAlign\":\"center\",\"textVAlign\":\"middle\",\"textColor\":\"#2C4256FF\",\"background\":\"#F7FAFDFF\"}"
"},"
"\"tree\":{\"type\":\"absolute\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"label\",\"id\":\"title\",\"font\":\"@fonts.body\",\"x\":20,\"y\":16,\"width\":720,\"height\":28,\"text\":\"Popup XSON: direction, fallback, and overflow scroll\",\"textColor\":\"#244868FF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"{\"type\":\"label\",\"id\":\"ownerA\",\"style\":\"owner\",\"x\":70,\"y\":92,\"text\":\"rightDown\"},"
"{\"type\":\"popup\",\"id\":\"popupA\",\"style\":\"popup\",\"owner\":\"ownerA\",\"anchorPoint\":\"bottomLeft\",\"direction\":\"rightDown\",\"contentWidth\":160,\"contentHeight\":72,\"children\":["
"{\"type\":\"absolute\",\"id\":\"contentA\",\"style\":\"content\",\"children\":[{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":8,\"width\":144,\"text\":\"XSON rightDown\"},{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":36,\"width\":144,\"text\":\"anchor + direction\"}]}]},"
"{\"type\":\"label\",\"id\":\"ownerB\",\"style\":\"owner\",\"x\":698,\"y\":430,\"text\":\"fallback\"},"
"{\"type\":\"popup\",\"id\":\"popupB\",\"style\":\"popup\",\"owner\":\"ownerB\",\"anchorPoint\":\"bottomLeft\",\"direction\":\"rightDown\",\"contentWidth\":190,\"contentHeight\":110,\"children\":["
"{\"type\":\"absolute\",\"id\":\"contentB\",\"style\":\"content\",\"children\":[{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":8,\"width\":174,\"text\":\"requested rightDown\"},{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":36,\"width\":174,\"text\":\"falls back inside\"}]}]},"
"{\"type\":\"label\",\"id\":\"ownerC\",\"style\":\"owner\",\"x\":520,\"y\":76,\"text\":\"long menu\"},"
"{\"type\":\"popup\",\"id\":\"popupC\",\"style\":\"popup\",\"owner\":\"ownerC\",\"anchorPoint\":\"bottomLeft\",\"direction\":\"rightDown\",\"contentWidth\":180,\"contentHeight\":720,\"children\":["
"{\"type\":\"absolute\",\"id\":\"contentC\",\"style\":\"content\",\"children\":["
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":8,\"width\":164,\"text\":\"Menu 01\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":34,\"width\":164,\"text\":\"Menu 02\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":60,\"width\":164,\"text\":\"Menu 03\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":86,\"width\":164,\"text\":\"Menu 04\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":112,\"width\":164,\"text\":\"Menu 05\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":138,\"width\":164,\"text\":\"Menu 06\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":164,\"width\":164,\"text\":\"Menu 07\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":190,\"width\":164,\"text\":\"Menu 08\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":216,\"width\":164,\"text\":\"Menu 09\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":242,\"width\":164,\"text\":\"Menu 10\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":268,\"width\":164,\"text\":\"Menu 11\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":294,\"width\":164,\"text\":\"Menu 12\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":320,\"width\":164,\"text\":\"Menu 13\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":346,\"width\":164,\"text\":\"Menu 14\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":372,\"width\":164,\"text\":\"Menu 15\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":398,\"width\":164,\"text\":\"Menu 16\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":424,\"width\":164,\"text\":\"Menu 17\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":450,\"width\":164,\"text\":\"Menu 18\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":476,\"width\":164,\"text\":\"Menu 19\"},"
"{\"type\":\"label\",\"style\":\"item\",\"x\":8,\"y\":502,\"width\":164,\"text\":\"Menu 20\"}"
"]}]}"
"]}}";

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;
	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int RectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = { "C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/arial.ttf" };
	int i;
	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui_popup_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_xui_popup PopupById(app_state_t* pApp, const char* sId)
{
	xge_xui_widget pWidget;
	pWidget = xgeXuiPageFind(&pApp->tPage, sId);
	return (pWidget != NULL) ? (xge_xui_popup)pWidget->pUser : NULL;
}

static int BindPopupContent(app_state_t* pApp, const char* sPopup, const char* sContent)
{
	xge_xui_popup pPopup;
	xge_xui_widget pContent;
	pPopup = PopupById(pApp, sPopup);
	pContent = xgeXuiPageFind(&pApp->tPage, sContent);
	if ( (pPopup == NULL) || (pContent == NULL) ) {
		return XGE_ERROR;
	}
	xgeXuiPopupSetContentWidget(pPopup, pContent);
	return XGE_OK;
}

static void SetRectById(app_state_t* pApp, const char* sId, xge_rect_t tRect)
{
	xge_xui_widget pWidget;
	pWidget = xgeXuiPageFind(&pApp->tPage, sId);
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetRect(pWidget, tRect);
	}
}

static void LayoutContent(app_state_t* pApp, const char* sContent, int iCount)
{
	xge_xui_widget pContent;
	xge_xui_widget pChild;
	int i;
	pContent = xgeXuiPageFind(&pApp->tPage, sContent);
	if ( pContent == NULL ) {
		return;
	}
	for ( i = 0, pChild = pContent->pFirstChild; (pChild != NULL) && (i < iCount); pChild = pChild->pNextSibling, i++ ) {
		xgeXuiWidgetSetRect(pChild, (xge_rect_t){ 8.0f, 8.0f + i * 26.0f, 164.0f, 24.0f });
	}
}

static int CreateUI(app_state_t* pApp)
{
	if ( pApp->bFontReady ) {
		xgeXuiTokenSetFont(&pApp->tXui, "body", &pApp->tFont);
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_popup_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	SetRectById(pApp, "title", (xge_rect_t){ 20.0f, 16.0f, 720.0f, 28.0f });
	SetRectById(pApp, "ownerA", (xge_rect_t){ 70.0f, 92.0f, 104.0f, 28.0f });
	SetRectById(pApp, "ownerB", (xge_rect_t){ 698.0f, 430.0f, 104.0f, 28.0f });
	SetRectById(pApp, "ownerC", (xge_rect_t){ 520.0f, 76.0f, 104.0f, 28.0f });
	LayoutContent(pApp, "contentA", 2);
	LayoutContent(pApp, "contentB", 2);
	LayoutContent(pApp, "contentC", 20);
	if ( BindPopupContent(pApp, "popupA", "contentA") != XGE_OK || BindPopupContent(pApp, "popupB", "contentB") != XGE_OK || BindPopupContent(pApp, "popupC", "contentC") != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_xui_popup pA;
	xge_xui_popup pB;
	xge_xui_popup pC;
	xge_rect_t tRect;
	xge_rect_t tRectC;
	xge_rect_t tContent;
	float fScrollX;
	float fScrollY;

	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pA = PopupById(pApp, "popupA");
	pB = PopupById(pApp, "popupB");
	pC = PopupById(pApp, "popupC");
	if ( pA != NULL ) {
		xgeXuiPopupSetOpen(pA, 1);
	}
	if ( pB != NULL ) {
		xgeXuiPopupSetOpen(pB, 1);
	}
	if ( pC != NULL ) {
		xgeXuiPopupSetOpen(pC, 1);
	}
	if ( pA != NULL ) {
		xgeXuiPopupApplyPlacement(pA);
	}
	if ( pB != NULL ) {
		xgeXuiPopupApplyPlacement(pB);
	}
	if ( pC != NULL ) {
		xgeXuiPopupApplyPlacement(pC);
	}
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->bCreateOK = (pA != NULL) && (pB != NULL) && (pC != NULL) && (xgeXuiPageFind(&pApp->tPage, "contentC") != NULL);
	pApp->bFieldsOK = (pA != NULL) && (pA->iAnchorPoint == XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT) && (pA->iDirection == XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN) && (pA->fContentW == 160.0f) && (pA->fContentH == 72.0f);
	tRect = (pB != NULL) ? xgeXuiPopupGetViewportRect(pB) : (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	tRectC = (pC != NULL) ? xgeXuiPopupGetViewportRect(pC) : (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	pApp->bLayoutOK = (pB != NULL) && (pC != NULL) && (tRect.fW >= 188.0f) && (tRect.fH >= 108.0f) && (tRectC.fW >= 178.0f) && (tRectC.fH > 300.0f);
	pApp->bFallbackOK = (pB != NULL) &&
		(tRect.fX + tRect.fW <= (float)xgeGetWidth()) &&
		(tRect.fY + tRect.fH <= (float)xgeGetHeight()) &&
		((tRect.fX < xgeXuiPageFind(&pApp->tPage, "ownerB")->tRect.fX) || (tRect.fY < xgeXuiPageFind(&pApp->tPage, "ownerB")->tRect.fY));
	if ( pC != NULL ) {
		xgeXuiPopupSetScroll(pC, 0.0f, 180.0f);
	}
	tRect = (pC != NULL) ? xgeXuiPopupGetViewportRect(pC) : (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	tContent = (pC != NULL) ? xgeXuiPopupGetContentRect(pC) : (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	xgeXuiPopupGetScroll(pC, &fScrollX, &fScrollY);
	pApp->bScrollOK = (pC != NULL) && (tContent.fH > tRect.fH) && (fScrollY > 100.0f);
	if ( pA != NULL ) {
		xgeXuiPopupSetScroll(pA, 0.0f, 0.0f);
		xgeXuiPopupSetOpen(pA, 0);
	}
	if ( pB != NULL ) {
		xgeXuiPopupSetScroll(pB, 0.0f, 0.0f);
		xgeXuiPopupSetOpen(pB, 0);
	}
	if ( pC != NULL ) {
		xgeXuiPopupSetScroll(pC, 0.0f, 0.0f);
		xgeXuiPopupSetOpen(pC, 0);
	}
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;
	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
		return XGE_ERROR;
	}
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
	xge_xui_widget pOwner;
	xge_xui_popup pPopup;

	pApp = (app_state_t*)pScene->pUser;
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN && pEvent->iParam1 == XGE_KEY_ESCAPE ) {
		xgeQuit();
		return XGE_OK;
	}
	if ( pEvent->iType == XGE_EVENT_MOUSE_DOWN ) {
		pOwner = xgeXuiPageFind(&pApp->tPage, "ownerA");
		if ( (pOwner != NULL) && RectContains(pOwner->tRect, pEvent->fX, pEvent->fY) ) {
			pPopup = PopupById(pApp, "popupA");
			if ( pPopup != NULL ) {
				if ( xgeXuiPopupIsOpen(pPopup) == 0 ) {
					xgeXuiPopupSetScroll(pPopup, 0.0f, 0.0f);
					xgeXuiPopupSetOpen(pPopup, 1);
				} else {
					xgeXuiPopupSetOpen(pPopup, 0);
				}
			}
			return XGE_OK;
		}
		pOwner = xgeXuiPageFind(&pApp->tPage, "ownerB");
		if ( (pOwner != NULL) && RectContains(pOwner->tRect, pEvent->fX, pEvent->fY) ) {
			pPopup = PopupById(pApp, "popupB");
			if ( pPopup != NULL ) {
				if ( xgeXuiPopupIsOpen(pPopup) == 0 ) {
					xgeXuiPopupSetScroll(pPopup, 0.0f, 0.0f);
					xgeXuiPopupSetOpen(pPopup, 1);
				} else {
					xgeXuiPopupSetOpen(pPopup, 0);
				}
			}
			return XGE_OK;
		}
		pOwner = xgeXuiPageFind(&pApp->tPage, "ownerC");
		if ( (pOwner != NULL) && RectContains(pOwner->tRect, pEvent->fX, pEvent->fY) ) {
			pPopup = PopupById(pApp, "popupC");
			if ( pPopup != NULL ) {
				if ( xgeXuiPopupIsOpen(pPopup) == 0 ) {
					xgeXuiPopupSetScroll(pPopup, 0.0f, 0.0f);
					xgeXuiPopupSetOpen(pPopup, 1);
				} else {
					xgeXuiPopupSetOpen(pPopup, 0);
				}
			}
			return XGE_OK;
		}
	}
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;
	pApp = (app_state_t*)pScene->pUser;
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		xge_xui_popup pB = PopupById(pApp, "popupB");
		xge_rect_t tRect = (pB != NULL) ? xgeXuiPopupGetViewportRect(pB) : (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
		printf("xui_popup_xson final-summary frames=%d create=%d fields=%d layout=%d fallback=%d scroll=%d rect=%.1f,%.1f,%.1f,%.1f win=%d,%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bFieldsOK, pApp->bLayoutOK, pApp->bFallbackOK, pApp->bScrollOK, tRect.fX, tRect.fY, tRect.fW, tRect.fH, xgeGetWidth(), xgeGetHeight());
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;
	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(232, 241, 248, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_POPUP_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 820;
	tDesc.iHeight = 560;
	tDesc.sTitle = "XUI Popup XSON";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bFieldsOK && tApp.bLayoutOK && tApp.bFallbackOK && tApp.bScrollOK) ? 0 : 3;
}
