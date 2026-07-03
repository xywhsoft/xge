#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EDIT_COUNT 4

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pLabelWidget[EDIT_COUNT];
	xge_xui_label_t tLabel[EDIT_COUNT];
	xge_xui_widget pEditWidget[EDIT_COUNT];
	xge_xui_text_edit_t tEdit[EDIT_COUNT];
	xge_xui_text_edit_highlight_t arrHighlights[2];
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bCreateOK;
	int bStateOK;
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

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_textedit font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_font AppFont(app_state_t* pApp)
{
	return pApp->bFontReady ? &pApp->tFont : NULL;
}

static int AddEdit(app_state_t* pApp, int iIndex, const char* sLabel, float fX, float fY, float fW, float fH)
{
	pApp->pLabelWidget[iIndex] = xgeXuiWidgetCreate();
	pApp->pEditWidget[iIndex] = xgeXuiWidgetCreate();
	if ( (pApp->pLabelWidget[iIndex] == NULL) || (pApp->pEditWidget[iIndex] == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pLabelWidget[iIndex], (xge_rect_t){ fX, fY - 30.0f, fW, 24.0f });
	xgeXuiWidgetSetRect(pApp->pEditWidget[iIndex], (xge_rect_t){ fX, fY, fW, fH });
	xgeXuiWidgetSetPaddingPx(pApp->pEditWidget[iIndex], 8.0f, 6.0f, 8.0f, 6.0f);
	xgeXuiWidgetAdd(xgeXuiRoot(&pApp->tXui), pApp->pLabelWidget[iIndex]);
	xgeXuiWidgetAdd(xgeXuiRoot(&pApp->tXui), pApp->pEditWidget[iIndex]);
	if ( xgeXuiLabelInit(&pApp->tLabel[iIndex], pApp->pLabelWidget[iIndex], AppFont(pApp), sLabel) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tLabel[iIndex], XGE_COLOR_RGBA(66, 78, 94, 255));
	xgeXuiLabelSetAlign(&pApp->tLabel[iIndex], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	if ( xgeXuiTextEditInit(&pApp->tEdit[iIndex], &pApp->tXui, pApp->pEditWidget[iIndex], AppFont(pApp)) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	int i;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, AppFont(pApp));
	XgeXuiDemoApplyRootPanel(pRoot);
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 900.0f, 620.0f });
	if ( AddEdit(pApp, 0, "Default multi-line editor", 30.0f, 72.0f, 390.0f, 190.0f) != XGE_OK ) return XGE_ERROR;
	if ( AddEdit(pApp, 1, "Line numbers, wrap and highlights", 450.0f, 72.0f, 390.0f, 190.0f) != XGE_OK ) return XGE_ERROR;
	if ( AddEdit(pApp, 2, "Custom color palette", 30.0f, 330.0f, 390.0f, 190.0f) != XGE_OK ) return XGE_ERROR;
	if ( AddEdit(pApp, 3, "Readonly disabled style", 450.0f, 330.0f, 390.0f, 190.0f) != XGE_OK ) return XGE_ERROR;

	xgeXuiTextEditSetText(&pApp->tEdit[0], "alpha\nbeta\ngamma\n\nThe editor keeps IME, selection, menu and undo behavior.");
	xgeXuiTextEditSetText(&pApp->tEdit[1], "1. wrapped text keeps paragraphs readable inside the control.\n2. line numbers use a separate background.\n3. find highlights stay independent from selection.");
	xgeXuiTextEditSetWordWrap(&pApp->tEdit[1], 1);
	xgeXuiTextEditSetLineNumbers(&pApp->tEdit[1], 1, 40.0f);
	pApp->arrHighlights[0].iStart = 0;
	pApp->arrHighlights[0].iEnd = 9;
	pApp->arrHighlights[1].iStart = 70;
	pApp->arrHighlights[1].iEnd = 80;
	xgeXuiTextEditSetFindHighlights(&pApp->tEdit[1], pApp->arrHighlights, 2);
	xgeXuiTextEditSetText(&pApp->tEdit[2], "Custom palette\nBlue border\nSoft current line\nGreen scrollbar thumb");
	xgeXuiTextEditSetFrameColors(&pApp->tEdit[2], XGE_COLOR_RGBA(246, 252, 255, 255), XGE_COLOR_RGBA(238, 248, 255, 255), XGE_COLOR_RGBA(91, 153, 202, 255), XGE_COLOR_RGBA(55, 133, 196, 255), XGE_COLOR_RGBA(33, 115, 186, 255));
	xgeXuiTextEditSetCurrentLineColor(&pApp->tEdit[2], XGE_COLOR_RGBA(226, 243, 255, 180));
	xgeXuiTextEditSetScrollbarColors(&pApp->tEdit[2], XGE_COLOR_RGBA(247, 252, 248, 255), XGE_COLOR_RGBA(164, 208, 178, 255), XGE_COLOR_RGBA(55, 148, 92, 245));
	xgeXuiTextEditSetText(&pApp->tEdit[3], "Readonly content\nDisabled text uses a quieter color.");
	xgeXuiTextEditSetReadonly(&pApp->tEdit[3], 1);
	xgeXuiWidgetSetEnabled(pApp->pEditWidget[3], 0);
	xgeXuiSetFocus(&pApp->tXui, pApp->pEditWidget[0]);

	pApp->bCreateOK = 1;
	pApp->bStateOK = 1;
	for ( i = 0; i < EDIT_COUNT; i++ ) {
		pApp->bStateOK = pApp->bStateOK && (pApp->tEdit[i].pWidget == pApp->pEditWidget[i]);
	}
	pApp->bStateOK = pApp->bStateOK && (pApp->tEdit[1].bLineNumbers != 0) && (pApp->tEdit[3].bReadonly != 0);
	return XGE_OK;
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pScene->pUser;
	for ( i = 0; i < EDIT_COUNT; i++ ) {
		xgeXuiTextEditUnit(&pApp->tEdit[i]);
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

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_textedit final-summary frames=%d create=%d state=%d edits=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bStateOK, EDIT_COUNT);
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
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TEXTEDIT_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 900;
	tDesc.iHeight = 620;
	tDesc.sTitle = "XUI TextEdit";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bStateOK) ? 0 : 3;
}

