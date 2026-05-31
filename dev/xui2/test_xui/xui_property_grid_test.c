#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_property_grid_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct property_grid_test_data_t {
	int iSelectCount;
	int iValidateCount;
	int iChangeCount;
	int iActionCount;
	int iRenderCount;
	char sLastId[64];
	char sLastValue[128];
} property_grid_test_data_t;

static int __xuiPropertyGridTestRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 640, 420};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiPropertyGridTypeText(xui_context pContext, const char* sText)
{
	int iRet;

	if ( sText == NULL ) return XUI_OK;
	while ( *sText != '\0' ) {
		iRet = xuiInputText(pContext, (uint32_t)(unsigned char)*sText++);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiDispatchPendingEvents(pContext);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiPropertyGridClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
}

static int __xuiPropertyGridStrictInt(const char* sValue)
{
	const char* p;
	int iValue;

	if ( (sValue == NULL) || (sValue[0] == '\0') ) return 0;
	p = sValue;
	while ( *p != '\0' ) {
		if ( (*p < '0') || (*p > '9') ) return 0;
		p++;
	}
	iValue = atoi(sValue);
	return (iValue >= 1) && (iValue <= 64);
}

static void __xuiPropertyGridSelect(xui_widget pWidget, int iProperty, const char* sId, void* pUser)
{
	property_grid_test_data_t* pData;

	(void)pWidget;
	(void)iProperty;
	pData = (property_grid_test_data_t*)pUser;
	if ( pData != NULL ) {
		pData->iSelectCount++;
		snprintf(pData->sLastId, sizeof(pData->sLastId), "%s", sId != NULL ? sId : "");
	}
}

static int __xuiPropertyGridValidate(xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	property_grid_test_data_t* pData;

	(void)pWidget;
	(void)iProperty;
	(void)iType;
	pData = (property_grid_test_data_t*)pUser;
	if ( pData != NULL ) {
		pData->iValidateCount++;
	}
	if ( (sId != NULL) && (strcmp(sId, "maxPlayers") == 0) ) {
		return __xuiPropertyGridStrictInt(sValue);
	}
	return 1;
}

static void __xuiPropertyGridChange(xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	property_grid_test_data_t* pData;

	(void)pWidget;
	(void)iProperty;
	(void)iType;
	pData = (property_grid_test_data_t*)pUser;
	if ( pData != NULL ) {
		pData->iChangeCount++;
		snprintf(pData->sLastId, sizeof(pData->sLastId), "%s", sId != NULL ? sId : "");
		snprintf(pData->sLastValue, sizeof(pData->sLastValue), "%s", sValue != NULL ? sValue : "");
	}
}

static int __xuiPropertyGridAction(xui_widget pWidget, int iProperty, const char* sId, xui_rect_t tWorldRect, void* pUser)
{
	property_grid_test_data_t* pData;

	(void)sId;
	(void)tWorldRect;
	pData = (property_grid_test_data_t*)pUser;
	if ( pData != NULL ) {
		pData->iActionCount++;
	}
	(void)xuiPropertyGridSetValue(pWidget, iProperty, "picked.xge");
	return 1;
}

static int __xuiPropertyGridCustomRender(xui_widget pWidget, int iProperty, int iColumn, const xui_table_view_cell_t* pCell, xui_draw_context pDraw, xui_rect_t tRect, int iState, void* pUser)
{
	property_grid_test_data_t* pData;

	(void)pWidget;
	(void)iProperty;
	(void)iColumn;
	(void)pCell;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	pData = (property_grid_test_data_t*)pUser;
	if ( pData != NULL ) {
		pData->iRenderCount++;
	}
	return 0;
}

int main(void)
{
	static const char* arrThemes[] = {"Light", "Dark", "High contrast"};
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pGrid;
	xui_widget pTableGrid;
	xui_widget pTableView;
	xui_surface pTarget;
	xui_font pFont;
	xui_property_grid_desc_t tDesc;
	xui_property_desc_t tProp;
	xui_table_grid_editor_config_t tConfig;
	property_grid_test_data_t tData;
	xui_rect_t tCell;
	xui_rect_t tWorld;
	int iAppearance;
	int iBehavior;
	int iAdvanced;
	int pTitle;
	int pVisible;
	int pMaxPlayers;
	int pTheme;
	int pAccent;
	int pAsset;
	int pReadonly;
	int pDisabled;
	int pHidden;
	int iVisibleExpanded;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pGrid = NULL;
	pTableGrid = NULL;
	pTableView = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);
	memset(&tData, 0, sizeof(tData));

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "propertygrid", 12, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 640.0f, 420.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 640.0f, 420.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.fNameWidth = 150.0f;
	tDesc.fRowHeight = 26.0f;
	tDesc.fCategoryHeight = 28.0f;
	tDesc.iDescriptionMode = XUI_PROPERTY_GRID_DESCRIPTION_BOTH;
	tDesc.fDescriptionPanelHeight = 48.0f;
	tDesc.iEditMode = XUI_TABLE_GRID_EDIT_QUICK;
	tDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
	iRet = xuiPropertyGridCreate(pContext, &pGrid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pGrid != NULL, "propertygrid create");
	xuiWidgetSetRect(pGrid, (xui_rect_t){18.0f, 18.0f, 420.0f, 320.0f});
	iRet = xuiWidgetAddChild(pRoot, pGrid);
	XUI_TEST_CHECK(iRet == XUI_OK, "add propertygrid");
	pTableGrid = xuiPropertyGridGetTableGrid(pGrid);
	pTableView = xuiPropertyGridGetTableView(pGrid);
	XUI_TEST_CHECK(pTableGrid != NULL && pTableView != NULL, "inner widgets");
	iRet = xuiPropertyGridSetSelect(pGrid, __xuiPropertyGridSelect, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "select callback");
	iRet = xuiPropertyGridSetValidate(pGrid, __xuiPropertyGridValidate, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "validate callback");
	iRet = xuiPropertyGridSetChange(pGrid, __xuiPropertyGridChange, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "change callback");
	iRet = xuiPropertyGridSetGlobalAction(pGrid, __xuiPropertyGridAction, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "action callback");
	iRet = xuiPropertyGridSetGlobalRenderer(pGrid, __xuiPropertyGridCustomRender, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "renderer callback");

	iAppearance = xuiPropertyGridAddCategory(pGrid, "appearance", "Appearance", 1);
	iBehavior = xuiPropertyGridAddCategory(pGrid, "behavior", "Behavior", 1);
	iAdvanced = xuiPropertyGridAddCategory(pGrid, "advanced", "Advanced", 1);
	XUI_TEST_CHECK(iAppearance >= 0 && iBehavior >= 0 && iAdvanced >= 0, "categories");

	memset(&tProp, 0, sizeof(tProp));
	tProp.sId = "title";
	tProp.sName = "Title";
	tProp.sDescription = "Window title shown in the caption.";
	tProp.iType = XUI_TABLE_CELL_TYPE_TEXT;
	tProp.sValue = "Inspector";
	tProp.sDefaultValue = "Inspector";
	pTitle = xuiPropertyGridAddProperty(pGrid, iAppearance, &tProp);
	tProp.sId = "visible";
	tProp.sName = "Visible";
	tProp.sDescription = "Controls whether the item is drawn.";
	tProp.iType = XUI_TABLE_CELL_TYPE_BOOL;
	tProp.sValue = "true";
	tProp.sDefaultValue = "true";
	pVisible = xuiPropertyGridAddProperty(pGrid, iAppearance, &tProp);
	tProp.sId = "accent";
	tProp.sName = "Accent color";
	tProp.iType = XUI_TABLE_CELL_TYPE_COLOR;
	tProp.sValue = "#2F80ED";
	tProp.sDefaultValue = "#2F80ED";
	pAccent = xuiPropertyGridAddProperty(pGrid, iAppearance, &tProp);
	tProp.sId = "maxPlayers";
	tProp.sName = "Max players";
	tProp.iType = XUI_TABLE_CELL_TYPE_INT;
	tProp.sValue = "8";
	tProp.sDefaultValue = "4";
	pMaxPlayers = xuiPropertyGridAddProperty(pGrid, iBehavior, &tProp);
	tProp.sId = "theme";
	tProp.sName = "Theme";
	tProp.iType = XUI_TABLE_CELL_TYPE_ENUM;
	tProp.sValue = "Light";
	tProp.sDefaultValue = "Light";
	pTheme = xuiPropertyGridAddProperty(pGrid, iBehavior, &tProp);
	tProp.sId = "asset";
	tProp.sName = "Asset";
	tProp.iType = XUI_TABLE_CELL_TYPE_PICKER;
	tProp.sValue = "...";
	tProp.sDefaultValue = "";
	pAsset = xuiPropertyGridAddProperty(pGrid, iAdvanced, &tProp);
	tProp.sId = "readonly";
	tProp.sName = "Readonly";
	tProp.iType = XUI_TABLE_CELL_TYPE_TEXT;
	tProp.sValue = "locked";
	tProp.sDefaultValue = "locked";
	tProp.iFlags = XUI_PROPERTY_FLAG_READONLY;
	pReadonly = xuiPropertyGridAddProperty(pGrid, iAdvanced, &tProp);
	tProp.sId = "disabled";
	tProp.sName = "Disabled";
	tProp.sValue = "off";
	tProp.iFlags = XUI_PROPERTY_FLAG_DISABLED;
	pDisabled = xuiPropertyGridAddProperty(pGrid, iAdvanced, &tProp);
	tProp.sId = "hidden";
	tProp.sName = "Hidden";
	tProp.sValue = "secret";
	tProp.iFlags = XUI_PROPERTY_FLAG_HIDDEN;
	pHidden = xuiPropertyGridAddProperty(pGrid, iAdvanced, &tProp);
	XUI_TEST_CHECK(pTitle >= 0 && pVisible >= 0 && pMaxPlayers >= 0 && pTheme >= 0 && pAccent >= 0 && pAsset >= 0 && pReadonly >= 0 && pDisabled >= 0 && pHidden >= 0, "properties");

	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.arrEnumItems = arrThemes;
	tConfig.iEnumItemCount = 3;
	tConfig.iEnumSelected = 1;
	iRet = xuiPropertyGridSetEditorConfig(pGrid, pTheme, &tConfig);
	XUI_TEST_CHECK(iRet == XUI_OK, "enum config");
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.fMin = 1.0f;
	tConfig.fMax = 64.0f;
	tConfig.fStep = 1.0f;
	tConfig.iPrecision = 0;
	iRet = xuiPropertyGridSetEditorConfig(pGrid, pMaxPlayers, &tConfig);
	XUI_TEST_CHECK(iRet == XUI_OK, "int config");

	XUI_TEST_CHECK(xuiPropertyGridGetCategoryCount(pGrid) == 3, "category count");
	XUI_TEST_CHECK(xuiPropertyGridGetPropertyCount(pGrid) == 9, "property count");
	XUI_TEST_CHECK(xuiPropertyGridFindProperty(pGrid, "theme") == pTheme, "find property");
	XUI_TEST_CHECK(xuiPropertyGridGetVisibleProperty(pGrid, 0) == -1, "category visible row");
	iVisibleExpanded = xuiPropertyGridGetVisibleCount(pGrid);
	XUI_TEST_CHECK(iVisibleExpanded == 11, "visible count excludes hidden");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 640, 420, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiPropertyGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pTarget) > 0, "initial render");
	XUI_TEST_CHECK(tData.iRenderCount > 0, "custom renderer used");

	XUI_TEST_CHECK(xuiTableViewGetCellRect(pTableView, 0, 0, &tCell) == XUI_OK && tCell.fW > 100.0f, "category cell rect");
	tWorld = xuiWidgetGetWorldRect(pTableView);
	iRet = __xuiPropertyGridClick(pContext, tWorld.fX + tCell.fX + 8.0f, tWorld.fY + tCell.fY + tCell.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "category click");
	XUI_TEST_CHECK(xuiPropertyGridGetCategoryExpanded(pGrid, iAppearance) == 0 && xuiPropertyGridGetToggleCount(pGrid) == 1, "category collapsed");
	XUI_TEST_CHECK(xuiPropertyGridGetVisibleCount(pGrid) < iVisibleExpanded, "visible collapsed");
	iRet = xuiPropertyGridSetCategoryExpanded(pGrid, iAppearance, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetVisibleCount(pGrid) == iVisibleExpanded, "category expand api");

	XUI_TEST_CHECK(xuiTableViewGetCellRect(pTableView, 1, 0, &tCell) == XUI_OK, "name cell rect");
	tWorld = xuiWidgetGetWorldRect(pTableView);
	iRet = __xuiPropertyGridClick(pContext, tWorld.fX + tCell.fX + 8.0f, tWorld.fY + tCell.fY + tCell.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "name click");
	XUI_TEST_CHECK(xuiPropertyGridGetSelected(pGrid) == pTitle && tData.iSelectCount >= 1 && strcmp(tData.sLastId, "title") == 0, "name select");
	XUI_TEST_CHECK(xuiPropertyGridIsEditing(pGrid), "name quick edit starts value editor");
	iRet = __xuiPropertyGridTypeText(pContext, "Panel");
	XUI_TEST_CHECK(iRet == XUI_OK, "title type");
	iRet = xuiPropertyGridEndEdit(pGrid, 1);
	XUI_TEST_CHECK(iRet != 0 && strcmp(xuiPropertyGridGetValue(pGrid, pTitle), "Panel") == 0, "title commit");
	XUI_TEST_CHECK(tData.iChangeCount >= 1 && strcmp(tData.sLastId, "title") == 0, "title change callback");

	iRet = xuiPropertyGridBeginEdit(pGrid, pVisible);
	XUI_TEST_CHECK(iRet != 0 && xuiPropertyGridGetBool(pGrid, pVisible, 0) == 0 && !xuiPropertyGridIsEditing(pGrid), "bool toggle");

	iRet = xuiPropertyGridBeginEdit(pGrid, pMaxPlayers);
	XUI_TEST_CHECK(iRet != 0 && xuiPropertyGridIsEditing(pGrid), "int begin invalid");
	iRet = __xuiPropertyGridTypeText(pContext, "bad");
	XUI_TEST_CHECK(iRet == XUI_OK, "int type invalid");
	iRet = xuiPropertyGridEndEdit(pGrid, 1);
	XUI_TEST_CHECK(iRet == 0 && xuiPropertyGridIsEditing(pGrid), "int reject");
	iRet = xuiPropertyGridEndEdit(pGrid, 0);
	XUI_TEST_CHECK(iRet != 0, "int cancel");
	iRet = xuiPropertyGridBeginEdit(pGrid, pMaxPlayers);
	XUI_TEST_CHECK(iRet != 0 && xuiPropertyGridIsEditing(pGrid), "int begin valid");
	iRet = __xuiPropertyGridTypeText(pContext, "42");
	XUI_TEST_CHECK(iRet == XUI_OK, "int type valid");
	iRet = xuiPropertyGridEndEdit(pGrid, 1);
	XUI_TEST_CHECK(iRet != 0 && xuiPropertyGridGetInt(pGrid, pMaxPlayers, 0) == 42, "int commit");

	iRet = xuiPropertyGridBeginEdit(pGrid, pTheme);
	XUI_TEST_CHECK(iRet != 0 && xuiPropertyGridIsEditing(pGrid), "enum begin");
	iRet = xuiPropertyGridEndEdit(pGrid, 1);
	XUI_TEST_CHECK(iRet != 0 && strcmp(xuiPropertyGridGetValue(pGrid, pTheme), "Dark") == 0, "enum commit");

	iRet = xuiPropertyGridBeginEdit(pGrid, pAsset);
	XUI_TEST_CHECK(iRet != 0 && !xuiPropertyGridIsEditing(pGrid), "picker begin");
	XUI_TEST_CHECK(tData.iActionCount == 1 && strcmp(xuiPropertyGridGetValue(pGrid, pAsset), "picked.xge") == 0, "picker action");
	XUI_TEST_CHECK(xuiTableGridGetPickerCount(pTableGrid) >= 1, "tablegrid picker count");

	iRet = xuiPropertyGridBeginEdit(pGrid, pReadonly);
	XUI_TEST_CHECK(iRet == 0, "readonly not editable");
	iRet = xuiPropertyGridBeginEdit(pGrid, pDisabled);
	XUI_TEST_CHECK(iRet == 0, "disabled not editable");
	XUI_TEST_CHECK(xuiPropertyGridGetVisibleProperty(pGrid, xuiPropertyGridGetVisibleCount(pGrid) - 1) != pHidden, "hidden not visible");

	iRet = xuiPropertyGridSetColor(pGrid, pAccent, XUI_COLOR_RGBA(67, 167, 124, 255));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetColor(pGrid, pAccent, 0) == XUI_COLOR_RGBA(67, 167, 124, 255), "color helpers");
	iRet = xuiPropertyGridSetPropertyFlags(pGrid, pMaxPlayers, XUI_PROPERTY_FLAG_DIRTY | XUI_PROPERTY_FLAG_INVALID);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiPropertyGridGetPropertyFlags(pGrid, pMaxPlayers) & XUI_PROPERTY_FLAG_INVALID) != 0, "flags");
	iRet = xuiPropertyGridSetScroll(pGrid, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetScroll(pGrid) >= 0.0f, "scroll");
	iRet = xuiPropertyGridSetDescriptionMode(pGrid, XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetDescriptionMode(pGrid) == XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP, "description mode");

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "final layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "final update");
	iRet = __xuiPropertyGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pTarget) > 0, "final render");
	XUI_TEST_CHECK(xuiTableViewGetColumnCount(pTableView) == 2, "column count");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_property_grid_test passed\n");
	return 0;
}
