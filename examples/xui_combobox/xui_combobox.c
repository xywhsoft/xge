#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMBO_COUNT 5

enum {
	COMBO_BASIC = 0,
	COMBO_RICH,
	COMBO_FIXED_HEIGHT,
	COMBO_AUTO_TOP,
	COMBO_DISABLED
};

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel;
	xge_xui_widget pStatus;
	xge_xui_widget pLabel[COMBO_COUNT];
	xge_xui_widget pCombo[COMBO_COUNT];
	xge_xui_panel_t tPanel;
	xge_xui_label_t tStatus;
	xge_xui_label_t tLabel[COMBO_COUNT];
	xge_xui_combo_box_t tCombo[COMBO_COUNT];
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iSelectCount;
	int iLastSelected;
	int bCreateOK;
	int bConfigOK;
	int bOpenOK;
	int bSelectOK;
	int bDisabledOK;
	int bTopOK;
} app_state_t;

static const char* g_arrBasic[] = { "Compact", "Comfortable", "Spacious", "Custom" };
static const char* g_arrLong[] = { "Small", "Normal", "Large", "Huge", "Fit content", "Fill width", "Locked" };
static const int g_arrEnabled[] = { 1, 0, 1, 1 };
static const xge_xui_combo_box_item_t g_arrRich[] = {
	{ "Draft", 10, 1, 0, 0, NULL },
	{ "Archived", 20, 0, 0, 0, NULL },
	{ "Published", 30, 1, 0, 0, NULL },
	{ "Separator", 0, 0, 1, 0, NULL },
	{ "Scheduled", 40, 1, 0, 0, NULL }
};

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
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui_combobox font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui_combobox font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void MakeMouse(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeKey(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static void ComboSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pApp->iSelectCount++;
	pApp->iLastSelected = iIndex;
}

static int AddCombo(app_state_t* pApp, int iIndex, const char* sLabel, float fX, float fY, xge_font pFont)
{
	pApp->pLabel[iIndex] = xgeXuiWidgetCreate();
	pApp->pCombo[iIndex] = xgeXuiWidgetCreate();
	if ( (pApp->pLabel[iIndex] == NULL) || (pApp->pCombo[iIndex] == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pLabel[iIndex], (xge_rect_t){ fX, fY + 4.0f, 150.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tLabel[iIndex], pApp->pLabel[iIndex], pFont, sLabel);
	xgeXuiLabelSetColor(&pApp->tLabel[iIndex], XGE_COLOR_RGBA(60, 82, 104, 255));
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pLabel[iIndex]);

	xgeXuiWidgetSetRect(pApp->pCombo[iIndex], (xge_rect_t){ fX + 160.0f, fY, 220.0f, 32.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pCombo[iIndex], 8.0f, 5.0f, 8.0f, 5.0f);
	xgeXuiComboBoxInit(&pApp->tCombo[iIndex], &pApp->tXui, pApp->pCombo[iIndex]);
	xgeXuiComboBoxSetFont(&pApp->tCombo[iIndex], pFont);
	xgeXuiComboBoxSetSelect(&pApp->tCombo[iIndex], ComboSelect, pApp);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pCombo[iIndex]);
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	pApp->pPanel = xgeXuiWidgetCreate();
	pApp->pStatus = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pStatus == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 20.0f, 18.0f, 720.0f, 408.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "ComboBox");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);

	xgeXuiWidgetSetRect(pApp->pStatus, (xge_rect_t){ 18.0f, 32.0f, 680.0f, 30.0f });
	xgeXuiLabelInit(&pApp->tStatus, pApp->pStatus, pFont, "");
	xgeXuiLabelSetColor(&pApp->tStatus, XGE_COLOR_RGBA(36, 82, 118, 255));
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pStatus);

	if ( AddCombo(pApp, COMBO_BASIC, "basic", 18.0f, 86.0f, pFont) != XGE_OK ||
		AddCombo(pApp, COMBO_RICH, "rich items", 18.0f, 134.0f, pFont) != XGE_OK ||
		AddCombo(pApp, COMBO_FIXED_HEIGHT, "fixed popup height", 18.0f, 182.0f, pFont) != XGE_OK ||
		AddCombo(pApp, COMBO_DISABLED, "disabled", 18.0f, 230.0f, pFont) != XGE_OK ||
		AddCombo(pApp, COMBO_AUTO_TOP, "auto top popup", 18.0f, 310.0f, pFont) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiComboBoxSetItems(&pApp->tCombo[COMBO_BASIC], g_arrBasic, 4);
	xgeXuiComboBoxSetEnabledItems(&pApp->tCombo[COMBO_BASIC], g_arrEnabled, 4);
	xgeXuiComboBoxSetSelected(&pApp->tCombo[COMBO_BASIC], 0);

	xgeXuiComboBoxSetItemData(&pApp->tCombo[COMBO_RICH], g_arrRich, 5);
	xgeXuiComboBoxSetSelectedValue(&pApp->tCombo[COMBO_RICH], 30);
	xgeXuiComboBoxSetItemColors(&pApp->tCombo[COMBO_RICH], XGE_COLOR_RGBA(230, 246, 238, 255), XGE_COLOR_RGBA(205, 236, 216, 255), XGE_COLOR_RGBA(242, 244, 247, 255), XGE_COLOR_RGBA(154, 164, 176, 255));

	xgeXuiComboBoxSetItems(&pApp->tCombo[COMBO_FIXED_HEIGHT], g_arrLong, 7);
	xgeXuiComboBoxSetSelected(&pApp->tCombo[COMBO_FIXED_HEIGHT], 2);
	xgeXuiComboBoxSetPopupHeight(&pApp->tCombo[COMBO_FIXED_HEIGHT], 76.0f);

	xgeXuiComboBoxSetItems(&pApp->tCombo[COMBO_AUTO_TOP], g_arrLong, 7);
	xgeXuiComboBoxSetSelected(&pApp->tCombo[COMBO_AUTO_TOP], 1);
	xgeXuiComboBoxSetPopupMaxHeight(&pApp->tCombo[COMBO_AUTO_TOP], 160.0f);
	xgeXuiComboBoxSetPopupPlacement(&pApp->tCombo[COMBO_AUTO_TOP], XGE_XUI_COMBO_POPUP_AUTO);

	xgeXuiComboBoxSetItems(&pApp->tCombo[COMBO_DISABLED], g_arrBasic, 4);
	xgeXuiComboBoxSetSelected(&pApp->tCombo[COMBO_DISABLED], 1);
	xgeXuiWidgetSetEnabled(pApp->pCombo[COMBO_DISABLED], 0);
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_xui_combo_box pCombo;
	float fX;
	float fY;
	float fRowY;

	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->bCreateOK = (pApp->tCombo[COMBO_BASIC].pWidget != NULL) && (pApp->tCombo[COMBO_BASIC].pPopupWidget != NULL) && (pApp->tCombo[COMBO_BASIC].pListWidget != NULL);
	pApp->bConfigOK =
		(xgeXuiComboBoxGetSelected(&pApp->tCombo[COMBO_BASIC]) == 0) &&
		(xgeXuiComboBoxGetSelectedValue(&pApp->tCombo[COMBO_RICH]) == 30) &&
		(pApp->tCombo[COMBO_FIXED_HEIGHT].fPopupHeight == 76.0f) &&
		(pApp->tCombo[COMBO_AUTO_TOP].fPopupMaxHeight == 160.0f);

	pCombo = &pApp->tCombo[COMBO_BASIC];
	fX = pCombo->pWidget->tRect.fX + pCombo->pWidget->tRect.fW * 0.5f;
	fY = pCombo->pWidget->tRect.fY + pCombo->pWidget->tRect.fH * 0.5f;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
	pApp->bOpenOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (xgeXuiComboBoxIsOpen(pCombo) != 0);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	fRowY = pCombo->pListWidget->tContentRect.fY + pCombo->fItemHeight * 2.5f;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pCombo->pListWidget->tContentRect.fX + 20.0f, fRowY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bSelectOK = (xgeXuiComboBoxGetSelected(pCombo) == 2) && (pApp->iSelectCount > 0) && (pApp->iLastSelected == 2) && (xgeXuiComboBoxIsOpen(pCombo) == 0);

	xgeXuiComboBoxSetSelected(&pApp->tCombo[COMBO_BASIC], 0);
	xgeXuiComboBoxSetSelected(&pApp->tCombo[COMBO_BASIC], 1);
	pApp->bDisabledOK = (xgeXuiComboBoxGetSelected(&pApp->tCombo[COMBO_BASIC]) == -1) && ((pApp->tCombo[COMBO_DISABLED].pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0);

	pCombo = &pApp->tCombo[COMBO_AUTO_TOP];
	fX = pCombo->pWidget->tRect.fX + pCombo->pWidget->tRect.fW * 0.5f;
	fY = pCombo->pWidget->tRect.fY + pCombo->pWidget->tRect.fH * 0.5f;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->bTopOK = xgeXuiComboBoxIsOpen(pCombo) && (pCombo->pPopupWidget->tRect.fY < pCombo->pWidget->tRect.fY);
	MakeKey(&tEvent, XGE_KEY_ESCAPE);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"create=%d config=%d open=%d select=%d disabled=%d top=%d selected=%d value=%d cb=%d",
		pApp->bCreateOK,
		pApp->bConfigOK,
		pApp->bOpenOK,
		pApp->bSelectOK,
		pApp->bDisabledOK,
		pApp->bTopOK,
		xgeXuiComboBoxGetSelected(&pApp->tCombo[COMBO_BASIC]),
		xgeXuiComboBoxGetSelectedValue(&pApp->tCombo[COMBO_RICH]),
		pApp->iSelectCount);
	xgeXuiLabelSetText(&pApp->tStatus, sText);
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
	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pScene->pUser;
	for ( i = 0; i < COMBO_COUNT; i++ ) {
		xgeXuiComboBoxUnit(&pApp->tCombo[i]);
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
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN && pEvent->iParam1 == XGE_KEY_ESCAPE ) {
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
	UpdateStatus(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui_combobox final-summary frames=%d create=%d config=%d open=%d select=%d disabled=%d top=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bConfigOK,
			pApp->bOpenOK,
			pApp->bSelectOK,
			pApp->bDisabledOK,
			pApp->bTopOK);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_COMBOBOX_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 460;
	tDesc.sTitle = "XUI ComboBox";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bConfigOK && tApp.bOpenOK && tApp.bSelectOK && tApp.bDisabledOK && tApp.bTopOK) ? 0 : 3;
}
