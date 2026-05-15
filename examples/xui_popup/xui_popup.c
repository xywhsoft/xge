#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define POPUP_MATRIX_COUNT 16
#define OWNER_COUNT 19
#define POPUP_COUNT 19
#define LABEL_COUNT 160

enum {
	POP_FALLBACK = POPUP_MATRIX_COUNT,
	POP_LONG,
	POP_HUGE
};

static const int g_arrAnchor[4] = {
	XGE_XUI_POPUP_ANCHOR_TOP_LEFT,
	XGE_XUI_POPUP_ANCHOR_TOP_RIGHT,
	XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT,
	XGE_XUI_POPUP_ANCHOR_BOTTOM_RIGHT
};

static const int g_arrDirection[4] = {
	XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN,
	XGE_XUI_POPUP_DIRECTION_RIGHT_UP,
	XGE_XUI_POPUP_DIRECTION_LEFT_DOWN,
	XGE_XUI_POPUP_DIRECTION_LEFT_UP
};

static const char* g_arrAnchorName[4] = { "TL", "TR", "BL", "BR" };
static const char* g_arrDirectionName[4] = { "RD", "RU", "LD", "LU" };

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pOwner[OWNER_COUNT];
	xge_xui_widget pPopupWidget[POPUP_COUNT];
	xge_xui_widget pContent[POPUP_COUNT];
	xge_xui_popup_t tPopup[POPUP_COUNT];
	xge_xui_label_t tLabel[LABEL_COUNT];
	int iLabelCount;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bCreateOK;
	int bDirectionOK;
	int bFallbackOK;
	int bScrollOK;
	int bHugeOK;
	int bDragOK;
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
	const char* arrFonts[] = { "C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/arial.ttf" };
	int i;
	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui_popup font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_font AppFont(app_state_t* pApp)
{
	return pApp->bFontReady ? &pApp->tFont : NULL;
}

static int RectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static int AddText(app_state_t* pApp, xge_xui_widget pParent, const char* sText, xge_rect_t tRect, uint32_t iText, uint32_t iBack, uint32_t iBorder)
{
	xge_xui_widget pWidget;
	xge_xui_label pLabel;

	if ( pApp->iLabelCount >= LABEL_COUNT ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pWidget, tRect);
	if ( XGE_COLOR_GET_A(iBack) != 0 ) {
		xgeXuiWidgetSetBackground(pWidget, iBack);
	}
	if ( XGE_COLOR_GET_A(iBorder) != 0 ) {
		xgeXuiWidgetSetBorder(pWidget, 1.0f, iBorder);
	}
	pLabel = &pApp->tLabel[pApp->iLabelCount++];
	xgeXuiLabelInit(pLabel, pWidget, AppFont(pApp), sText);
	xgeXuiLabelSetColor(pLabel, iText);
	xgeXuiLabelSetAlign(pLabel, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE);
	xgeXuiWidgetAdd(pParent, pWidget);
	return XGE_OK;
}

static int CreateOwner(app_state_t* pApp, int iIndex, const char* sText, xge_rect_t tRect)
{
	xge_xui_widget pRoot;
	pRoot = xgeXuiRoot(&pApp->tXui);
	pApp->pOwner[iIndex] = xgeXuiWidgetCreate();
	if ( pApp->pOwner[iIndex] == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pOwner[iIndex], tRect);
	xgeXuiWidgetSetBackground(pApp->pOwner[iIndex], XGE_COLOR_RGBA(230, 240, 250, 255));
	xgeXuiWidgetSetBorder(pApp->pOwner[iIndex], 1.0f, XGE_COLOR_RGBA(91, 143, 190, 255));
	xgeXuiWidgetAdd(pRoot, pApp->pOwner[iIndex]);
	return AddText(pApp, pApp->pOwner[iIndex], sText, (xge_rect_t){ 0.0f, 0.0f, tRect.fW, tRect.fH }, XGE_COLOR_RGBA(45, 72, 96, 255), 0, 0);
}

static int CreatePopup(app_state_t* pApp, int iIndex, int iOwner, const char* sText, float fW, float fH, int iAnchor, int iDirection)
{
	pApp->pPopupWidget[iIndex] = xgeXuiWidgetCreate();
	pApp->pContent[iIndex] = xgeXuiWidgetCreate();
	if ( (pApp->pPopupWidget[iIndex] == NULL) || (pApp->pContent[iIndex] == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPopupWidget[iIndex], (xge_rect_t){ 0.0f, 0.0f, fW, fH });
	xgeXuiWidgetSetRect(pApp->pContent[iIndex], (xge_rect_t){ 0.0f, 0.0f, fW, fH });
	xgeXuiWidgetSetLayout(pApp->pContent[iIndex], XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiPopupInit(&pApp->tPopup[iIndex], &pApp->tXui, pApp->pPopupWidget[iIndex]);
	xgeXuiPopupSetOwner(&pApp->tPopup[iIndex], pApp->pOwner[iOwner]);
	xgeXuiPopupSetAnchorPoint(&pApp->tPopup[iIndex], iAnchor);
	xgeXuiPopupSetDirection(&pApp->tPopup[iIndex], iDirection);
	xgeXuiPopupSetGap(&pApp->tPopup[iIndex], 0.0f);
	xgeXuiPopupSetClosePolicy(&pApp->tPopup[iIndex], XGE_XUI_POPUP_OUTSIDE_CLOSE, XGE_XUI_POPUP_OWNER_PASSTHROUGH, XGE_XUI_POPUP_ESCAPE_CLOSE);
	xgeXuiPopupSetConsumeInside(&pApp->tPopup[iIndex], 1);
	xgeXuiPopupSetContentWidget(&pApp->tPopup[iIndex], pApp->pContent[iIndex]);
	xgeXuiPopupSetContentSize(&pApp->tPopup[iIndex], fW, fH);
	xgeXuiPopupSetBorder(&pApp->tPopup[iIndex], XGE_COLOR_RGBA(91, 143, 190, 255));
	xgeXuiOverlayAttach(&pApp->tXui, pApp->pPopupWidget[iIndex], pApp->pOwner[iOwner], XGE_XUI_LAYER_POPUP);
	return AddText(pApp, pApp->pContent[iIndex], sText, (xge_rect_t){ 8.0f, 8.0f, fW - 16.0f, fH - 16.0f }, XGE_COLOR_RGBA(43, 66, 88, 255), XGE_COLOR_RGBA(242, 248, 253, 255), XGE_COLOR_RGBA(205, 224, 240, 255));
}

static int FillLongPopup(app_state_t* pApp)
{
	char sText[32];
	int i;
	for ( i = 0; i < 28; i++ ) {
		snprintf(sText, sizeof(sText), "Menu item %02d", i + 1);
		if ( AddText(pApp, pApp->pContent[POP_LONG], sText, (xge_rect_t){ 8.0f, 8.0f + i * 26.0f, 164.0f, 24.0f }, XGE_COLOR_RGBA(44, 66, 86, 255), (i & 1) ? XGE_COLOR_RGBA(247, 250, 253, 255) : XGE_COLOR_RGBA(235, 244, 251, 255), 0) != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	return XGE_OK;
}

static int FillHugePopup(app_state_t* pApp)
{
	int i;
	char sText[48];
	for ( i = 0; i < 18; i++ ) {
		snprintf(sText, sizeof(sText), "Huge popup content row %02d", i + 1);
		if ( AddText(pApp, pApp->pContent[POP_HUGE], sText, (xge_rect_t){ 28.0f, 26.0f + i * 38.0f, 560.0f, 30.0f }, XGE_COLOR_RGBA(40, 58, 76, 255), XGE_COLOR_RGBA(238, 246, 252, 255), XGE_COLOR_RGBA(210, 226, 238, 255)) != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	return AddText(pApp, pApp->pContent[POP_HUGE], "1000 x 760 content, viewport is clamped to window and scrollable", (xge_rect_t){ 620.0f, 26.0f, 320.0f, 86.0f }, XGE_COLOR_RGBA(42, 86, 125, 255), XGE_COLOR_RGBA(229, 241, 250, 255), XGE_COLOR_RGBA(146, 184, 216, 255));
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	char sOwner[32];
	char sPopup[64];
	int iAnchor;
	int iDirection;
	int iIndex;
	float fX;
	float fY;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, AppFont(pApp));
	XgeXuiDemoApplyRootPanel(pRoot);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_ABSOLUTE);
	if ( AddText(pApp, pRoot, "Popup combinations: anchor x direction, fallback, overflow scrolling, and huge content viewport", (xge_rect_t){ 20.0f, 16.0f, 820.0f, 28.0f }, XGE_COLOR_RGBA(36, 72, 104, 255), 0, 0) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	for ( iAnchor = 0; iAnchor < 4; iAnchor++ ) {
		for ( iDirection = 0; iDirection < 4; iDirection++ ) {
			iIndex = iAnchor * 4 + iDirection;
			fX = 64.0f + iDirection * 128.0f;
			fY = 108.0f + iAnchor * 92.0f;
			snprintf(sOwner, sizeof(sOwner), "%s + %s", g_arrAnchorName[iAnchor], g_arrDirectionName[iDirection]);
			snprintf(sPopup, sizeof(sPopup), "anchor %s\ndirection %s", g_arrAnchorName[iAnchor], g_arrDirectionName[iDirection]);
			if ( CreateOwner(pApp, iIndex, sOwner, (xge_rect_t){ fX, fY, 94.0f, 26.0f }) != XGE_OK ||
				CreatePopup(pApp, iIndex, iIndex, sPopup, 118.0f, 50.0f, g_arrAnchor[iAnchor], g_arrDirection[iDirection]) != XGE_OK ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
		}
	}

	if ( CreateOwner(pApp, POP_FALLBACK, "fallback", (xge_rect_t){ 736.0f, 430.0f, 92.0f, 28.0f }) != XGE_OK ||
		CreateOwner(pApp, POP_LONG, "long menu", (xge_rect_t){ 636.0f, 78.0f, 104.0f, 28.0f }) != XGE_OK ||
		CreateOwner(pApp, POP_HUGE, "huge popup", (xge_rect_t){ 636.0f, 142.0f, 104.0f, 28.0f }) != XGE_OK ||
		CreatePopup(pApp, POP_FALLBACK, POP_FALLBACK, "requested rightDown\nresolved inside window", 178.0f, 104.0f, XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT, XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN) != XGE_OK ||
		CreatePopup(pApp, POP_LONG, POP_LONG, "Long menu content", 180.0f, 736.0f, XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT, XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN) != XGE_OK ||
		CreatePopup(pApp, POP_HUGE, POP_HUGE, "Huge popup", 1000.0f, 760.0f, XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT, XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( FillLongPopup(pApp) != XGE_OK || FillHugePopup(pApp) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_rect_t tRect;
	xge_rect_t tThumb;
	xge_rect_t tBar;
	xge_event_t tEvent;
	int i;

	for ( i = 0; i < POPUP_MATRIX_COUNT; i++ ) {
		xgeXuiPopupSetOpen(&pApp->tPopup[i], 1);
	}
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->bCreateOK = 1;
	for ( i = 0; i < POPUP_COUNT; i++ ) {
		if ( (pApp->pPopupWidget[i] == NULL) || (pApp->pContent[i] == NULL) || (pApp->tPopup[i].pWidget == NULL) ) {
			pApp->bCreateOK = 0;
		}
	}
	pApp->bDirectionOK = 1;
	for ( i = 0; i < POPUP_MATRIX_COUNT; i++ ) {
		if ( (pApp->pPopupWidget[i]->tRect.fW <= 0.0f) || (pApp->pPopupWidget[i]->tRect.fH <= 0.0f) ) {
			pApp->bDirectionOK = 0;
		}
	}
	for ( i = 0; i < POPUP_MATRIX_COUNT; i++ ) {
		xgeXuiPopupSetOpen(&pApp->tPopup[i], 0);
	}
	xgeXuiPopupSetOpen(&pApp->tPopup[POP_FALLBACK], 1);
	tRect = xgeXuiPopupGetViewportRect(&pApp->tPopup[POP_FALLBACK]);
	pApp->bFallbackOK = (tRect.fX + tRect.fW <= (float)xgeGetWidth()) && (tRect.fY + tRect.fH <= (float)xgeGetHeight()) && (tRect.fX < pApp->pOwner[POP_FALLBACK]->tRect.fX);
	xgeXuiPopupSetOpen(&pApp->tPopup[POP_LONG], 1);
	xgeXuiPopupSetScroll(&pApp->tPopup[POP_LONG], 0.0f, 180.0f);
	pApp->bScrollOK = (pApp->tPopup[POP_LONG].bScrollEnabled != 0) && (pApp->pContent[POP_LONG]->tRect.fY < -100.0f);
	xgeXuiPopupSetOpen(&pApp->tPopup[POP_HUGE], 1);
	tRect = xgeXuiPopupGetViewportRect(&pApp->tPopup[POP_HUGE]);
	pApp->bHugeOK = (pApp->tPopup[POP_HUGE].bScrollEnabled != 0) && (tRect.fW <= (float)xgeGetWidth()) && (tRect.fH <= (float)xgeGetHeight());
	tBar = (xge_rect_t){ tRect.fX + 2.0f, tRect.fY + tRect.fH - 5.0f, tRect.fW - 4.0f, 3.0f };
	tThumb = (xge_rect_t){ tBar.fX, tBar.fY, 18.0f, tBar.fH };
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_MOUSE_DOWN;
	tEvent.iPointerId = 1;
	tEvent.iParam1 = XGE_MOUSE_LEFT;
	tEvent.fX = tThumb.fX + 2.0f;
	tEvent.fY = tThumb.fY + 1.0f;
	xgeXuiPopupEvent(&pApp->tPopup[POP_HUGE], &tEvent);
	tEvent.iType = XGE_EVENT_MOUSE_MOVE;
	tEvent.fX += 120.0f;
	xgeXuiPopupEvent(&pApp->tPopup[POP_HUGE], &tEvent);
	tEvent.iType = XGE_EVENT_MOUSE_UP;
	xgeXuiPopupEvent(&pApp->tPopup[POP_HUGE], &tEvent);
	pApp->bDragOK = pApp->tPopup[POP_HUGE].fScrollX > 20.0f;
	for ( i = 0; i < POPUP_COUNT; i++ ) {
		xgeXuiPopupSetScroll(&pApp->tPopup[i], 0.0f, 0.0f);
		xgeXuiPopupSetOpen(&pApp->tPopup[i], 0);
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
	int i;
	pApp = (app_state_t*)pScene->pUser;
	for ( i = 0; i < POPUP_COUNT; i++ ) {
		xgeXuiPopupUnit(&pApp->tPopup[i]);
	}
	for ( i = 0; i < pApp->iLabelCount; i++ ) {
		xgeXuiLabelUnit(&pApp->tLabel[i]);
	}
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	return XGE_OK;
}

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pScene->pUser;
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN && pEvent->iParam1 == XGE_KEY_ESCAPE ) {
		xgeQuit();
		return XGE_OK;
	}
	if ( pEvent->iType == XGE_EVENT_MOUSE_DOWN ) {
		for ( i = 0; i < OWNER_COUNT; i++ ) {
			if ( (pApp->pOwner[i] != NULL) && RectContains(pApp->pOwner[i]->tRect, pEvent->fX, pEvent->fY) ) {
				if ( xgeXuiPopupIsOpen(&pApp->tPopup[i]) == 0 ) {
					xgeXuiPopupSetScroll(&pApp->tPopup[i], 0.0f, 0.0f);
					xgeXuiPopupSetOpen(&pApp->tPopup[i], 1);
				} else {
					xgeXuiPopupSetOpen(&pApp->tPopup[i], 0);
				}
				return XGE_OK;
			}
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
		printf("xui_popup final-summary frames=%d create=%d direction=%d fallback=%d scroll=%d huge=%d drag=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bDirectionOK, pApp->bFallbackOK, pApp->bScrollOK, pApp->bHugeOK, pApp->bDragOK);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_POPUP_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 860;
	tDesc.iHeight = 620;
	tDesc.sTitle = "XUI Popup";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bDirectionOK && tApp.bFallbackOK && tApp.bScrollOK && tApp.bHugeOK && tApp.bDragOK) ? 0 : 3;
}
