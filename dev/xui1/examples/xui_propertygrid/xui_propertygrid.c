#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CATEGORY_COUNT 4
#define PROPERTY_HINT_COUNT 4

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel;
	xge_xui_widget pGridWidget;
	xge_xui_widget pHintWidget[PROPERTY_HINT_COUNT];
	xge_xui_label_t tHint[PROPERTY_HINT_COUNT];
	xge_xui_property_grid_t tGrid;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iChangeCount;
	int iSelectCount;
	int iActionCount;
	int bCreateOK;
	int bLayoutOK;
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
	const char* arrFonts[] = { "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/Deng.ttf", "C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/arial.ttf" };
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_propertygrid font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_xui_widget NewWidget(float fHeight)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), (fHeight > 0.0f) ? xgeXuiSizePx(fHeight) : xgeXuiSizeGrow(1.0f));
	}
	return pWidget;
}

static int AddHint(app_state_t* pApp, xge_xui_widget pParent, int iIndex, const char* sText)
{
	xge_xui_widget pWidget;

	if ( iIndex < 0 || iIndex >= PROPERTY_HINT_COUNT ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pWidget = NewWidget(24.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiLabelInit(&pApp->tHint[iIndex], pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tHint[iIndex], XGE_COLOR_RGBA(42, 58, 78, 255));
	xgeXuiLabelSetAlign(&pApp->tHint[iIndex], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	pApp->pHintWidget[iIndex] = pWidget;
	return xgeXuiWidgetAdd(pParent, pWidget);
}

static void PropertySelect(xge_xui_widget pWidget, int iProperty, const char* sId, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iProperty;
	(void)sId;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSelectCount++;
	}
}

static int PropertyValidate(xge_xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	(void)pWidget;
	(void)iProperty;
	(void)iType;
	(void)pUser;
	if ( sId != NULL && strcmp(sId, "runtime.maxPlayers") == 0 ) {
		return atoi(sValue != NULL ? sValue : "0") > 0;
	}
	return 1;
}

static void PropertyChange(xge_xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iProperty;
	(void)sId;
	(void)sValue;
	(void)iType;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iChangeCount++;
	}
}

static int PropertyAction(xge_xui_widget pWidget, int iProperty, const char* sId, xge_rect_t tRect, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)tRect;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return 0;
	}
	pApp->iActionCount++;
	if ( sId != NULL && strcmp(sId, "asset.path") == 0 ) {
		xgeXuiPropertyGridSetValue(&pApp->tGrid, iProperty, "picked://asset/hero.png");
		return 1;
	}
	if ( sId != NULL && strcmp(sId, "custom.preview") == 0 ) {
		xgeXuiPropertyGridSetValue(&pApp->tGrid, iProperty, "custom action");
		return 1;
	}
	return 0;
}

static int CustomRenderer(xge_xui_widget pWidget, int iProperty, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, int iState, void* pUser)
{
	app_state_t* pApp;
	xge_rect_t tSwatch;
	xge_rect_t tText;

	(void)pWidget;
	(void)iProperty;
	(void)iColumn;
	(void)iState;
	pApp = (app_state_t*)pUser;
	tSwatch = tRect;
	tSwatch.fX += 8.0f;
	tSwatch.fY += 5.0f;
	tSwatch.fW = 36.0f;
	tSwatch.fH -= 10.0f;
	xgeShapeRectFillPx(tSwatch, XGE_COLOR_RGBA(46, 124, 214, 255));
	xgeShapeRectStrokePx(tSwatch, 1.0f, XGE_COLOR_RGBA(20, 70, 128, 255));
	tText = tRect;
	tText.fX += 52.0f;
	tText.fW -= 58.0f;
	xgeTextDrawRect(pApp != NULL && pApp->bFontReady ? &pApp->tFont : NULL, (pCell != NULL && pCell->sText != NULL) ? pCell->sText : "", tText, XGE_COLOR_RGBA(42, 52, 66, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	return 1;
}

static int AddProp(xge_xui_property_grid pGrid, int iCategory, const char* sId, const char* sName, const char* sDescription, int iType, const char* sValue, const char* sDefault, int iFlags)
{
	xge_xui_property_desc_t tDesc;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.sId = sId;
	tDesc.sName = sName;
	tDesc.sDescription = sDescription;
	tDesc.iType = iType;
	tDesc.sValue = sValue;
	tDesc.sDefaultValue = sDefault;
	tDesc.iFlags = iFlags;
	return xgeXuiPropertyGridAddProperty(pGrid, iCategory, &tDesc);
}

static int FillPropertyGrid(app_state_t* pApp)
{
	static const char* arrTheme[] = { "Blue", "Green", "Orange", "Purple" };
	xge_xui_table_grid_editor_config_t tEnum;
	xge_xui_property_grid pGrid;
	int iAppearance;
	int iRuntime;
	int iState;
	int iCustom;
	int iProp;

	pGrid = &pApp->tGrid;
	xgeXuiPropertyGridSetSelect(pGrid, PropertySelect, pApp);
	xgeXuiPropertyGridSetValidate(pGrid, PropertyValidate, pApp);
	xgeXuiPropertyGridSetChange(pGrid, PropertyChange, pApp);
	xgeXuiPropertyGridSetGlobalAction(pGrid, PropertyAction, pApp);
	iAppearance = xgeXuiPropertyGridAddCategory(pGrid, "appearance", "Appearance", 1);
	iRuntime = xgeXuiPropertyGridAddCategory(pGrid, "runtime", "Runtime", 1);
	iState = xgeXuiPropertyGridAddCategory(pGrid, "states", "States", 1);
	iCustom = xgeXuiPropertyGridAddCategory(pGrid, "custom", "Custom", 1);
	if ( iAppearance < 0 || iRuntime < 0 || iState < 0 || iCustom < 0 ) {
		return XGE_ERROR;
	}
	AddProp(pGrid, iAppearance, "window.title", "Title", "Plain text property", XGE_XUI_TABLE_CELL_TYPE_TEXT, "Inspector", "Inspector", 0);
	AddProp(pGrid, iAppearance, "appearance.enabled", "Enabled", "Bool uses the TableGrid default checkbox display", XGE_XUI_TABLE_CELL_TYPE_BOOL, "true", "false", 0);
	AddProp(pGrid, iAppearance, "appearance.accent", "Accent Color", "Color property uses ColorPicker editor", XGE_XUI_TABLE_CELL_TYPE_COLOR, "#2E7CD6", "#2E7CD6", 0);
	iProp = AddProp(pGrid, iAppearance, "appearance.theme", "Theme", "Enum options are supplied by editor config", XGE_XUI_TABLE_CELL_TYPE_ENUM, "Blue", "Blue", 0);
	memset(&tEnum, 0, sizeof(tEnum));
	tEnum.arrEnumItems = arrTheme;
	tEnum.iEnumItemCount = (int)(sizeof(arrTheme) / sizeof(arrTheme[0]));
	tEnum.iEnumSelected = 0;
	xgeXuiPropertyGridSetEditorConfig(pGrid, iProp, &tEnum);
	AddProp(pGrid, iRuntime, "runtime.maxPlayers", "Max Players", "Integer property with validation", XGE_XUI_TABLE_CELL_TYPE_INT, "4", "4", 0);
	AddProp(pGrid, iRuntime, "runtime.opacity", "Opacity", "Float property", XGE_XUI_TABLE_CELL_TYPE_FLOAT, "0.85", "1.0", 0);
	AddProp(pGrid, iRuntime, "runtime.notes", "Notes", "Textarea opens a popup editor", XGE_XUI_TABLE_CELL_TYPE_TEXTAREA, "Line 1 summary\nFull multiline description", "Line 1 summary", 0);
	AddProp(pGrid, iRuntime, "asset.path", "Asset Path", "Picker only triggers callback", XGE_XUI_TABLE_CELL_TYPE_PICKER, "Browse", "Browse", 0);
	AddProp(pGrid, iState, "state.readonly", "Readonly", "Readonly values are visible but not editable", XGE_XUI_TABLE_CELL_TYPE_TEXT, "locked", "locked", XGE_XUI_PROPERTY_FLAG_READONLY);
	AddProp(pGrid, iState, "state.disabled", "Disabled", "Disabled property uses muted rendering", XGE_XUI_TABLE_CELL_TYPE_TEXT, "disabled", "disabled", XGE_XUI_PROPERTY_FLAG_DISABLED);
	AddProp(pGrid, iState, "state.dirty", "Dirty", "Dirty marker is inherited from TableGrid", XGE_XUI_TABLE_CELL_TYPE_TEXT, "changed", "default", 0);
	AddProp(pGrid, iState, "state.invalid", "Invalid", "Invalid border is inherited from TableGrid", XGE_XUI_TABLE_CELL_TYPE_TEXT, "bad value", "valid", XGE_XUI_PROPERTY_FLAG_INVALID);
	iProp = AddProp(pGrid, iCustom, "custom.preview", "Preview", "Custom renderer and action callback", XGE_XUI_TABLE_CELL_TYPE_CUSTOM, "custom swatch renderer", "custom swatch renderer", 0);
	xgeXuiPropertyGridSetRenderer(pGrid, iProp, CustomRenderer, pApp);
	xgeXuiPropertyGridSetSelected(pGrid, xgeXuiPropertyGridFindProperty(pGrid, "appearance.accent"));
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pRoot, 20.0f, 18.0f, 20.0f, 18.0f);
	xgeXuiWidgetSetGap(pRoot, 8.0f);
	xgeXuiWidgetSetBackground(pRoot, XGE_COLOR_RGBA(236, 240, 246, 255));
	pApp->pPanel = NewWidget(0.0f);
	if ( pApp->pPanel == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetLayout(pApp->pPanel, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 12.0f, 10.0f, 12.0f, 12.0f);
	xgeXuiWidgetSetGap(pApp->pPanel, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pPanel, XGE_COLOR_RGBA(248, 250, 253, 255));
	xgeXuiWidgetSetBorder(pApp->pPanel, 1.0f, XGE_COLOR_RGBA(170, 184, 202, 255));
	if ( xgeXuiWidgetAdd(pRoot, pApp->pPanel) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( AddHint(pApp, pApp->pPanel, 0, "PropertyGrid: categories, typed values, readonly and state markers") != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->pGridWidget = NewWidget(0.0f);
	if ( pApp->pGridWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetBorder(pApp->pGridWidget, 1.0f, XGE_COLOR_RGBA(170, 184, 202, 255));
	if ( xgeXuiPropertyGridInit(&pApp->tGrid, &pApp->tXui, pApp->pGridWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiPropertyGridSetFont(&pApp->tGrid, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiPropertyGridSetMetrics(&pApp->tGrid, 165.0f, 25.0f, 27.0f);
	if ( FillPropertyGrid(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiWidgetAdd(pApp->pPanel, pApp->pGridWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( AddHint(pApp, pApp->pPanel, 1, "Enum, bool, color, textarea, picker, and custom all reuse TableGrid behavior") != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fRootW;
	float fRootH;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( iWidth == pApp->iLastWidth && iHeight == pApp->iLastHeight ) {
		return;
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	fRootW = (float)iWidth;
	fRootH = (float)iHeight;
	if ( fRootW < 920.0f ) {
		fRootW = 920.0f;
	}
	if ( fRootH < 560.0f ) {
		fRootH = 560.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	int iDirty;
	int iCategory;

	iDirty = xgeXuiPropertyGridFindProperty(&pApp->tGrid, "state.dirty");
	iCategory = xgeXuiPropertyGridFindCategory(&pApp->tGrid, "runtime");
	pApp->bCreateOK = pApp->tGrid.iCategoryCount == 4 && pApp->tGrid.iPropertyCount >= 13;
	pApp->bLayoutOK = pApp->pGridWidget != NULL &&
		pApp->pGridWidget->tRect.fW > 500.0f &&
		pApp->tGrid.tGrid.tTable.fHeaderHeight == 0.0f &&
		pApp->tGrid.tGrid.iEditMode == XGE_XUI_TABLE_GRID_EDIT_QUICK &&
		pApp->tGrid.tGrid.tTable.pFrame != NULL &&
		pApp->tGrid.tGrid.tTable.pFrame->iScrollbarPolicyX == XGE_XUI_SCROLLBAR_POLICY_HIDDEN &&
		xgeXuiPropertyGridGetVisibleCount(&pApp->tGrid) > 8;
	pApp->bStateOK = iDirty >= 0 &&
		(xgeXuiPropertyGridGetPropertyFlags(&pApp->tGrid, iDirty) & XGE_XUI_PROPERTY_FLAG_INVALID) == 0 &&
		xgeXuiPropertyGridGetCategoryExpanded(&pApp->tGrid, iCategory) == 1 &&
		xgeXuiPropertyGridFindProperty(&pApp->tGrid, "custom.preview") >= 0;
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
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
	int i;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiPropertyGridUnit(&pApp->tGrid);
	for ( i = 0; i < PROPERTY_HINT_COUNT; i++ ) {
		if ( pApp->pHintWidget[i] != NULL ) {
			xgeXuiLabelUnit(&pApp->tHint[i]);
		}
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
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_propertygrid final-summary frames=%d create=%d layout=%d state=%d select=%d change=%d action=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->iSelectCount, pApp->iChangeCount, pApp->iActionCount);
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
	static app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_PROPERTYGRID_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 960;
	tDesc.iHeight = 600;
	tDesc.sTitle = "XUI PropertyGrid";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK) ? 0 : 3;
}
