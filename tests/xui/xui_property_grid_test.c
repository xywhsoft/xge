#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
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
	int iContextCount;
	int iContextPart;
	int iContextCategory;
	int iContextProperty;
	char sLastId[64];
	char sLastValue[128];
} property_grid_test_data_t;

typedef struct property_grid_lifecycle_data_t {
	int iCallCount;
	int bIdStable;
} property_grid_lifecycle_data_t;

static int __xuiPropertyGridContext(xui_widget pWidget, int iPart, int iCategory, int iProperty,
	float fX, float fY, void* pUser)
{
	property_grid_test_data_t* pData = (property_grid_test_data_t*)pUser;
	(void)pWidget;
	(void)fX;
	(void)fY;
	if ( pData != NULL ) {
		pData->iContextCount++;
		pData->iContextPart = iPart;
		pData->iContextCategory = iCategory;
		pData->iContextProperty = iProperty;
	}
	return XUI_EVENT_DISPATCH_STOP;
}

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

static int __xuiPropertyGridContextClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, XUI_POINTER_BUTTON_RIGHT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
}

static xui_widget __xuiPropertyGridFindPopupOwner(xui_context pContext, xui_widget_type pOwnerType)
{
	xui_widget pScan;
	xui_widget pOwner;
	xui_widget_type pPopupType;

	if ( pContext == NULL ) return NULL;
	pPopupType = xuiPopupGetType(pContext);
	pScan = xuiGetFocusWidget(pContext);
	while ( pScan != NULL ) {
		if ( xuiWidgetIsType(pScan, pPopupType) ) {
			pOwner = xuiPopupGetOwner(pScan);
			if ( pOwner != NULL && (pOwnerType == NULL || xuiWidgetIsType(pOwner, pOwnerType)) ) {
				return pOwner;
			}
		}
		pScan = xuiWidgetGetParent(pScan);
	}
	return NULL;
}

static int __xuiPropertyGridCheckNumericEditor(xui_context pContext, xui_widget pTableGrid, const char* sExpected)
{
	xui_widget_type pNumericType;
	xui_widget pEditor;
	xui_widget pInput;
	int iEditorLayer;
	int iEditorZ;
	int iInputLayer;
	int iInputZ;
	int iStart;
	int iEnd;
	int iExpectedLength;
	int iNodeCount;
	int iEditorNode;
	int iInputNode;
	int i;
	xui_render_node_t tNode;

	if ( (pContext == NULL) || (pTableGrid == NULL) || (sExpected == NULL) ) {
		return 0;
	}
	pNumericType = xuiNumericInputGetType(pContext);
	pEditor = NULL;
	for ( pInput = xuiWidgetGetFirstChild(pTableGrid); pInput != NULL; pInput = xuiWidgetGetNextSibling(pInput) ) {
		if ( xuiWidgetGetVisible(pInput) && xuiWidgetIsType(pInput, pNumericType) ) {
			pEditor = pInput;
			break;
		}
	}
	if ( pEditor == NULL ) {
		return 0;
	}
	pInput = xuiNumericInputGetInputWidget(pEditor);
	if ( (pInput == NULL) || !xuiWidgetGetVisible(pInput) ) {
		return 0;
	}
	if ( strcmp(xuiNumericInputGetText(pEditor), sExpected) != 0 ) {
		return 0;
	}
	if ( strcmp(xuiInputGetText(pInput), sExpected) != 0 ) {
		return 0;
	}
	if ( xuiInputGetSelection(pInput, &iStart, &iEnd) != XUI_OK ) {
		return 0;
	}
	iExpectedLength = (int)strlen(sExpected);
	if ( (iStart != 0) || (iEnd != iExpectedLength) ) {
		return 0;
	}
	if ( xuiWidgetGetLayer(pEditor, &iEditorLayer, &iEditorZ) != XUI_OK ) {
		return 0;
	}
	if ( xuiWidgetGetLayer(pInput, &iInputLayer, &iInputZ) != XUI_OK ) {
		return 0;
	}
	if ( (iInputLayer != iEditorLayer) || (iInputZ < iEditorZ) ) {
		return 0;
	}
	iEditorNode = -1;
	iInputNode = -1;
	iNodeCount = xuiGetRenderNodeCount(pContext);
	for ( i = 0; i < iNodeCount; i++ ) {
		if ( xuiGetRenderNode(pContext, i, &tNode) != XUI_OK ) {
			return 0;
		}
		if ( tNode.pWidget == pEditor ) {
			iEditorNode = i;
		} else if ( tNode.pWidget == pInput ) {
			iInputNode = i;
		}
	}
	if ( (iEditorNode < 0) || (iInputNode <= iEditorNode) ) {
		return 0;
	}
	return 1;
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

static int __xuiPropertyGridRejectAll(xui_widget pWidget, int iProperty, const char* sId,
	const char* sValue, int iType, void* pUser)
{
	(void)pWidget;
	(void)iProperty;
	(void)sId;
	(void)sValue;
	(void)iType;
	(void)pUser;
	return 0;
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

static int __xuiPropertyGridCountRender(xui_widget pWidget, int iProperty, int iColumn,
	const xui_table_view_cell_t* pCell, xui_draw_context pDraw, xui_rect_t tRect,
	int iState, void* pUser)
{
	int* pCount = (int*)pUser;
	(void)pWidget;
	(void)iProperty;
	(void)iColumn;
	(void)pCell;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	if ( pCount != NULL ) (*pCount)++;
	return 0;
}

static void __xuiPropertyGridDestroySelect(xui_widget pWidget, int iProperty, const char* sId, void* pUser)
{
	int* pCount = (int*)pUser;
	(void)iProperty;
	(void)sId;
	if ( pCount != NULL ) (*pCount)++;
	xuiWidgetDestroy(pWidget);
}

static void __xuiPropertyGridDestroyChange(xui_widget pWidget, int iProperty, const char* sId,
	const char* sValue, int iType, void* pUser)
{
	int* pCount = (int*)pUser;
	(void)iProperty;
	(void)sId;
	(void)sValue;
	(void)iType;
	if ( pCount != NULL ) (*pCount)++;
	xuiWidgetDestroy(pWidget);
}

static int __xuiPropertyGridDestroyRender(xui_widget pWidget, int iProperty, int iColumn,
	const xui_table_view_cell_t* pCell, xui_draw_context pDraw, xui_rect_t tRect, int iState, void* pUser)
{
	int* pCount = (int*)pUser;
	(void)iProperty;
	(void)iColumn;
	(void)pCell;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	if ( pCount != NULL ) (*pCount)++;
	xuiWidgetDestroy(pWidget);
	return 1;
}

static int __xuiPropertyGridDestroyValidate(xui_widget pWidget, int iProperty, const char* sId,
	const char* sValue, int iType, void* pUser)
{
	property_grid_lifecycle_data_t* pData = (property_grid_lifecycle_data_t*)pUser;
	(void)iProperty;
	(void)sValue;
	(void)iType;
	if ( pData != NULL ) pData->iCallCount++;
	xuiWidgetDestroy(pWidget);
	if ( pData != NULL && sId != NULL && strcmp(sId, "value") == 0 ) pData->bIdStable = 1;
	return 1;
}

static int __xuiPropertyGridDestroyAction(xui_widget pWidget, int iProperty, const char* sId,
	xui_rect_t tWorldRect, void* pUser)
{
	property_grid_lifecycle_data_t* pData = (property_grid_lifecycle_data_t*)pUser;
	(void)iProperty;
	(void)tWorldRect;
	if ( pData != NULL ) pData->iCallCount++;
	xuiWidgetDestroy(pWidget);
	if ( pData != NULL && sId != NULL && strcmp(sId, "value") == 0 ) pData->bIdStable = 1;
	return 1;
}

static int __xuiPropertyGridCreateLifecycleGrid(xui_context pContext, xui_font pFont,
	int iType, xui_widget* ppGrid, int* pProperty)
{
	xui_property_grid_desc_t tDesc;
	xui_property_desc_t tProp;
	xui_widget pGrid;
	int iCategory;
	int iProperty;
	int iRet;

	if ( ppGrid == NULL || pProperty == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppGrid = NULL;
	*pProperty = -1;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.iDescriptionMode = XUI_PROPERTY_GRID_DESCRIPTION_NONE;
	tDesc.iEditMode = XUI_TABLE_GRID_EDIT_QUICK;
	tDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	iRet = xuiPropertyGridCreate(pContext, &pGrid, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iCategory = xuiPropertyGridAddCategory(pGrid, "lifecycle", "Lifecycle", 1);
	memset(&tProp, 0, sizeof(tProp));
	tProp.sId = "value";
	tProp.sName = "Value";
	tProp.sValue = "initial";
	tProp.iType = iType;
	iProperty = xuiPropertyGridAddProperty(pGrid, iCategory, &tProp);
	if ( iCategory < 0 || iProperty < 0 ) {
		xuiWidgetDestroy(pGrid);
		return XUI_ERROR_INVALID_STATE;
	}
	*ppGrid = pGrid;
	*pProperty = iProperty;
	return XUI_OK;
}

static int __xuiPropertyGridRebuildOomPreserves(xui_context pContext, xui_font pFont)
{
	xui_property_grid_desc_t tDesc;
	xui_property_desc_t tProp;
	xui_widget pGrid;
	xui_widget pTableView;
	xallocationbudget* pBudget;
	xallocationscope tScope;
	int iCategory;
	int iProperty;
	int iRet;
	int bResult;
	int bBudgetEntered;

	pGrid = NULL;
	pBudget = NULL;
	bResult = 0;
	bBudgetEntered = 0;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.iDescriptionMode = XUI_PROPERTY_GRID_DESCRIPTION_NONE;
	tDesc.iEditMode = XUI_TABLE_GRID_EDIT_QUICK;
	tDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	if ( xuiPropertyGridCreate(pContext, &pGrid, &tDesc) != XUI_OK || pGrid == NULL ) goto cleanup;
	iCategory = xuiPropertyGridAddCategory(pGrid, "oom", "OOM", 0);
	memset(&tProp, 0, sizeof(tProp));
	tProp.sId = "value";
	tProp.sName = "Value";
	tProp.sValue = "initial";
	tProp.iType = XUI_TABLE_CELL_TYPE_TEXT;
	iProperty = xuiPropertyGridAddProperty(pGrid, iCategory, &tProp);
	pTableView = xuiPropertyGridGetTableView(pGrid);
	if ( iCategory < 0 || iProperty < 0 || pTableView == NULL ||
	     xuiPropertyGridGetVisibleCount(pGrid) != 1 || xuiTableViewGetRowCount(pTableView) != 1 ) goto cleanup;
	memset(&tScope, 0, sizeof(tScope));
	if ( !xrtAllocationBudgetCreate(1u, &pBudget) || !xrtAllocationBudgetEnter(pBudget, &tScope) ) goto cleanup;
	bBudgetEntered = 1;
	iRet = xuiPropertyGridSetCategoryExpanded(pGrid, iCategory, 1);
	(void)xrtAllocationBudgetLeave(&tScope);
	bBudgetEntered = 0;
	xrtAllocationBudgetFree(pBudget);
	pBudget = NULL;
	if ( iRet != XUI_ERROR_OUT_OF_MEMORY ||
	     xuiPropertyGridGetCategoryExpanded(pGrid, iCategory) != 0 ||
	     xuiPropertyGridGetToggleCount(pGrid) != 0 ||
	     xuiPropertyGridGetVisibleCount(pGrid) != 1 ||
	     xuiTableViewGetRowCount(pTableView) != 1 ) goto cleanup;
	if ( xuiPropertyGridSetCategoryExpanded(pGrid, iCategory, 1) != XUI_OK ||
	     xuiPropertyGridGetVisibleCount(pGrid) != 2 || xuiTableViewGetRowCount(pTableView) != 2 ) goto cleanup;
	xuiWidgetDestroy(pGrid);
	pGrid = NULL;

	if ( xuiPropertyGridCreate(pContext, &pGrid, &tDesc) != XUI_OK || pGrid == NULL ) goto cleanup;
	iCategory = xuiPropertyGridAddCategory(pGrid, "oom", "OOM", 1);
	pTableView = xuiPropertyGridGetTableView(pGrid);
	if ( iCategory < 0 || pTableView == NULL || xuiPropertyGridGetVisibleCount(pGrid) != 1 ||
	     xuiTableViewGetRowCount(pTableView) != 1 ) goto cleanup;
	memset(&tScope, 0, sizeof(tScope));
	if ( !xrtAllocationBudgetCreate(1u, &pBudget) || !xrtAllocationBudgetEnter(pBudget, &tScope) ) goto cleanup;
	bBudgetEntered = 1;
	iProperty = xuiPropertyGridAddProperty(pGrid, iCategory, &tProp);
	(void)xrtAllocationBudgetLeave(&tScope);
	bBudgetEntered = 0;
	xrtAllocationBudgetFree(pBudget);
	pBudget = NULL;
	if ( iProperty != -1 || xuiPropertyGridGetPropertyCount(pGrid) != 0 ||
	     xuiPropertyGridFindProperty(pGrid, "value") != -1 ||
	     xuiPropertyGridGetVisibleCount(pGrid) != 1 || xuiTableViewGetRowCount(pTableView) != 1 ) goto cleanup;
	iProperty = xuiPropertyGridAddProperty(pGrid, iCategory, &tProp);
	if ( iProperty != 0 || xuiPropertyGridGetPropertyCount(pGrid) != 1 ||
	     xuiPropertyGridGetVisibleCount(pGrid) != 2 || xuiTableViewGetRowCount(pTableView) != 2 ) goto cleanup;
	xuiWidgetDestroy(pGrid);
	pGrid = NULL;

	if ( xuiPropertyGridCreate(pContext, &pGrid, &tDesc) != XUI_OK || pGrid == NULL ) goto cleanup;
	pTableView = xuiPropertyGridGetTableView(pGrid);
	if ( pTableView == NULL ) goto cleanup;
	memset(&tScope, 0, sizeof(tScope));
	if ( !xrtAllocationBudgetCreate(1u, &pBudget) || !xrtAllocationBudgetEnter(pBudget, &tScope) ) goto cleanup;
	bBudgetEntered = 1;
	iCategory = xuiPropertyGridAddCategory(pGrid, "oom", "OOM", 1);
	(void)xrtAllocationBudgetLeave(&tScope);
	bBudgetEntered = 0;
	xrtAllocationBudgetFree(pBudget);
	pBudget = NULL;
	if ( iCategory != -1 || xuiPropertyGridGetCategoryCount(pGrid) != 0 ||
	     xuiPropertyGridFindCategory(pGrid, "oom") != -1 ||
	     xuiPropertyGridGetVisibleCount(pGrid) != 0 || xuiTableViewGetRowCount(pTableView) != 0 ) goto cleanup;
	iCategory = xuiPropertyGridAddCategory(pGrid, "oom", "OOM", 1);
	if ( iCategory != 0 || xuiPropertyGridGetCategoryCount(pGrid) != 1 ||
	     xuiPropertyGridGetVisibleCount(pGrid) != 1 || xuiTableViewGetRowCount(pTableView) != 1 ) goto cleanup;
	xuiWidgetDestroy(pGrid);
	pGrid = NULL;

	if ( xuiPropertyGridCreate(pContext, &pGrid, &tDesc) != XUI_OK || pGrid == NULL ) goto cleanup;
	iCategory = xuiPropertyGridAddCategory(pGrid, "oom", "OOM", 1);
	iProperty = xuiPropertyGridAddProperty(pGrid, iCategory, &tProp);
	pTableView = xuiPropertyGridGetTableView(pGrid);
	if ( iCategory < 0 || iProperty < 0 || pTableView == NULL ||
	     xuiPropertyGridSetSelected(pGrid, iProperty) != XUI_OK ) goto cleanup;
	memset(&tScope, 0, sizeof(tScope));
	if ( !xrtAllocationBudgetCreate(1u, &pBudget) || !xrtAllocationBudgetEnter(pBudget, &tScope) ) goto cleanup;
	bBudgetEntered = 1;
	iRet = xuiPropertyGridSetPropertyFlags(pGrid, iProperty, XUI_PROPERTY_FLAG_HIDDEN);
	(void)xrtAllocationBudgetLeave(&tScope);
	bBudgetEntered = 0;
	xrtAllocationBudgetFree(pBudget);
	pBudget = NULL;
	if ( iRet != XUI_ERROR_OUT_OF_MEMORY || xuiPropertyGridGetPropertyFlags(pGrid, iProperty) != 0 ||
	     xuiPropertyGridGetSelected(pGrid) != iProperty || xuiPropertyGridGetVisibleCount(pGrid) != 2 ||
	     xuiTableViewGetRowCount(pTableView) != 2 ) goto cleanup;
	if ( xuiPropertyGridSetPropertyFlags(pGrid, iProperty, XUI_PROPERTY_FLAG_HIDDEN) != XUI_OK ||
	     xuiPropertyGridGetSelected(pGrid) != -1 || xuiPropertyGridGetVisibleCount(pGrid) != 1 ||
	     xuiTableViewGetRowCount(pTableView) != 1 ) goto cleanup;
	bResult = 1;

cleanup:
	if ( pBudget != NULL ) {
		if ( bBudgetEntered ) (void)xrtAllocationBudgetLeave(&tScope);
		xrtAllocationBudgetFree(pBudget);
	}
	if ( pGrid != NULL ) xuiWidgetDestroy(pGrid);
	return bResult;
}

int main(void)
{
	static const char* arrThemes[] = {"Light", "Dark", "High contrast"};
	static const xui_combobox_item_t arrLayoutDirections[] = {
		{"Vertical Layout (Top / Bottom)", XUI_ORIENTATION_HORIZONTAL, 1, 0, 0, NULL},
		{"Horizontal Layout (Left / Right)", XUI_ORIENTATION_VERTICAL, 1, 0, 0, NULL}
	};
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pGrid;
	xui_widget pTableGrid;
	xui_widget pTableView;
	xui_widget pViewport;
	xui_widget pPopupOwner;
	xui_widget pProbe;
	xui_surface pTarget;
	xui_font pFont;
	xui_property_grid_desc_t tDesc;
	xui_property_desc_t tProp;
	xui_property_grid_style_t tStyle;
	xui_table_grid_editor_config_t tConfig;
	xui_widget_content_measure_proc onMeasure;
	property_grid_test_data_t tData;
	property_grid_lifecycle_data_t tLifecycle;
	xui_event_t tEvent;
	xui_rect_t tCell;
	xui_rect_t tOtherCell;
	xui_rect_t tViewport;
	xui_rect_t tWorld;
	xui_vec2_t tMeasured;
	char sLongId[XUI_PROPERTY_GRID_ID_CAPACITY + 1];
	char sLongValue[XUI_PROPERTY_GRID_VALUE_CAPACITY + 1];
	char sOwnedTheme0[32];
	char sOwnedTheme1[32];
	const char* arrOwnedThemes[2];
	uint32_t arrOwnedPalette[2];
	float fColumnWidth;
	float fNameWidth;
	float fRowHeight;
	float fCategoryHeight;
	void* pMeasureUser;
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
	int pLayoutDirection;
	int iVisibleExpanded;
	int iFailed;
	int iRet;
	int iRow;
	int iSelectedRow;
	int iSelectedColumn;
	int i;
	int iProbeProperty;
	int iCallbackCount;
	int iTableChangeCount;
	int iRenderCount;

	pContext = NULL;
	pRoot = NULL;
	pGrid = NULL;
	pTableGrid = NULL;
	pTableView = NULL;
	pViewport = NULL;
	pPopupOwner = NULL;
	pProbe = NULL;
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
	XUI_TEST_CHECK(__xuiPropertyGridRebuildOomPreserves(pContext, pFont),
	               "visible-row rebuild failures preserve model and table state");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fNameWidth = NAN;
	pProbe = pRoot;
	iRet = xuiPropertyGridCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pProbe == NULL,
	               "invalid descriptor clears output");
	tDesc.fNameWidth = 0.0f;
	tDesc.iDescriptionMode = 99;
	pProbe = pRoot;
	iRet = xuiPropertyGridCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pProbe == NULL,
	               "invalid descriptor mode rejected");
	tDesc.iDescriptionMode = XUI_PROPERTY_GRID_DESCRIPTION_NONE;
	tDesc.iEditMode = 99;
	pProbe = pRoot;
	iRet = xuiPropertyGridCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pProbe == NULL,
	               "invalid edit mode rejected");
	tDesc.iEditMode = XUI_TABLE_GRID_EDIT_DISPLAY;
	tDesc.iScrollbarMode = 99;
	pProbe = pRoot;
	iRet = xuiPropertyGridCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pProbe == NULL,
	               "invalid scrollbar mode rejected");
	tDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	tDesc.fNameWidth = 10.0f;
	pProbe = pRoot;
	iRet = xuiPropertyGridCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pProbe == NULL,
	               "undersized descriptor metric rejected");
	tDesc.fNameWidth = 0.0f;
	tDesc.fRowHeight = XUI_LAYOUT_UNBOUNDED;
	pProbe = pRoot;
	iRet = xuiPropertyGridCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pProbe == NULL,
	               "combined descriptor geometry rejected");
	tDesc.fRowHeight = 0.0f;
	tDesc.iDescriptionMode = XUI_PROPERTY_GRID_DESCRIPTION_PANEL;
	tDesc.fDescriptionPanelHeight = XUI_LAYOUT_UNBOUNDED;
	pProbe = pRoot;
	iRet = xuiPropertyGridCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pProbe == NULL,
	               "combined description geometry rejected");
	tDesc.iDescriptionMode = XUI_PROPERTY_GRID_DESCRIPTION_NONE;
	tDesc.fDescriptionPanelHeight = 0.0f;
	pProbe = pRoot;
	iRet = xuiPropertyGridCreate(NULL, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pProbe == NULL,
	               "invalid context clears output");

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
	xuiWidgetSetRect(pGrid, (xui_rect_t){18.0f, 18.0f, 420.0f, 380.0f});
	iRet = xuiWidgetAddChild(pRoot, pGrid);
	XUI_TEST_CHECK(iRet == XUI_OK, "add propertygrid");
	pTableGrid = xuiPropertyGridGetTableGrid(pGrid);
	pTableView = xuiPropertyGridGetTableView(pGrid);
	XUI_TEST_CHECK(pTableGrid != NULL && pTableView != NULL, "inner widgets");
	onMeasure = NULL;
	pMeasureUser = NULL;
	iRet = xuiWidgetGetContentMeasureCallback(pGrid, &onMeasure, &pMeasureUser);
	XUI_TEST_CHECK(iRet == XUI_OK && onMeasure != NULL,
	               "propertygrid content measure callback");
	tMeasured = (xui_vec2_t){0.0f, 0.0f};
	iRet = onMeasure(pGrid, (xui_vec2_t){60.0f, 12.0f}, &tMeasured, pMeasureUser);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 60.0f && tMeasured.fY == 12.0f,
	               "content measure honors tight constraint");
	XUI_TEST_CHECK(xuiPropertyGridGetScrollbarMode(pGrid) == XUI_SCROLLBAR_MODE_FULL,
	               "descriptor scrollbar mode honored");
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
	iRet = xuiPropertyGridSetContextMenu(pGrid, __xuiPropertyGridContext, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "context callback");

	iAppearance = xuiPropertyGridAddCategory(pGrid, "appearance", "Appearance", 1);
	iBehavior = xuiPropertyGridAddCategory(pGrid, "behavior", "Behavior", 1);
	iAdvanced = xuiPropertyGridAddCategory(pGrid, "advanced", "Advanced", 1);
	XUI_TEST_CHECK(iAppearance >= 0 && iBehavior >= 0 && iAdvanced >= 0, "categories");
	memset(sLongId, 'x', XUI_PROPERTY_GRID_ID_CAPACITY);
	sLongId[XUI_PROPERTY_GRID_ID_CAPACITY] = '\0';
	XUI_TEST_CHECK(xuiPropertyGridAddCategory(pGrid, NULL, NULL, 1) < 0,
	               "empty effective category id rejected");
	XUI_TEST_CHECK(xuiPropertyGridAddCategory(pGrid, sLongId, "Long", 1) < 0,
	               "long category id rejected");
	XUI_TEST_CHECK(xuiPropertyGridAddCategory(pGrid, NULL, "appearance", 1) < 0,
	               "duplicate effective category id rejected");

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
	tProp.sId = "layoutDirection";
	tProp.sName = "Layout Direction";
	tProp.sDescription = "Pane layout direction.";
	tProp.iType = XUI_TABLE_CELL_TYPE_ENUM;
	tProp.sValue = "1";
	tProp.sDefaultValue = "1";
	tProp.iFlags = 0;
	pLayoutDirection = xuiPropertyGridAddProperty(pGrid, iAdvanced, &tProp);
	XUI_TEST_CHECK(pTitle >= 0 && pVisible >= 0 && pMaxPlayers >= 0 && pTheme >= 0 && pAccent >= 0 && pAsset >= 0 && pReadonly >= 0 && pDisabled >= 0 && pHidden >= 0 && pLayoutDirection >= 0, "properties");
	tProp.sId = sLongId;
	XUI_TEST_CHECK(xuiPropertyGridAddProperty(pGrid, iAppearance, &tProp) < 0,
	               "long property id rejected");

	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.arrEnumItems = arrThemes;
	tConfig.iEnumItemCount = 3;
	tConfig.iEnumSelected = 1;
	iRet = xuiPropertyGridSetEditorConfig(pGrid, pTheme, &tConfig);
	XUI_TEST_CHECK(iRet == XUI_OK, "enum config");
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.arrEnumItemData = arrLayoutDirections;
	tConfig.iEnumItemCount = 2;
	tConfig.bEnumUseValue = 1;
	tConfig.iEnumSelectedValue = 1;
	iRet = xuiPropertyGridSetEditorConfig(pGrid, pLayoutDirection, &tConfig);
	XUI_TEST_CHECK(iRet == XUI_OK, "value enum config");
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.fMin = 1.0f;
	tConfig.fMax = 64.0f;
	tConfig.fStep = 1.0f;
	tConfig.iPrecision = 0;
	iRet = xuiPropertyGridSetEditorConfig(pGrid, pMaxPlayers, &tConfig);
	XUI_TEST_CHECK(iRet == XUI_OK, "int config");
	snprintf(sOwnedTheme0, sizeof(sOwnedTheme0), "%s", "Light");
	snprintf(sOwnedTheme1, sizeof(sOwnedTheme1), "%s", "Dark");
	arrOwnedThemes[0] = sOwnedTheme0;
	arrOwnedThemes[1] = sOwnedTheme1;
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.arrEnumItems = arrOwnedThemes;
	tConfig.iEnumItemCount = 2;
	tConfig.iEnumSelected = 1;
	iRet = xuiPropertyGridSetEditorConfig(pGrid, pTheme, &tConfig);
	XUI_TEST_CHECK(iRet == XUI_OK, "enum config snapshots mutable text");
	snprintf(sOwnedTheme0, sizeof(sOwnedTheme0), "%s", "Mutated Light");
	snprintf(sOwnedTheme1, sizeof(sOwnedTheme1), "%s", "Mutated Dark");
	arrOwnedPalette[0] = XUI_COLOR_RGBA(12, 34, 56, 255);
	arrOwnedPalette[1] = XUI_COLOR_RGBA(78, 90, 12, 255);
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.arrPalette = arrOwnedPalette;
	tConfig.iPaletteCount = 2;
	iRet = xuiPropertyGridSetEditorConfig(pGrid, pAccent, &tConfig);
	XUI_TEST_CHECK(iRet == XUI_OK, "palette config snapshots mutable colors");
	arrOwnedPalette[0] = XUI_COLOR_RGBA(1, 2, 3, 255);
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.iEnumItemCount = 1;
	XUI_TEST_CHECK(xuiPropertyGridSetEditorConfig(pGrid, pTheme, &tConfig) == XUI_ERROR_INVALID_ARGUMENT,
	               "enum config requires items");
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.iPaletteCount = 1;
	XUI_TEST_CHECK(xuiPropertyGridSetEditorConfig(pGrid, pAccent, &tConfig) == XUI_ERROR_INVALID_ARGUMENT,
	               "palette config requires colors");
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.arrEnumItems = arrThemes;
	tConfig.iEnumItemCount = XUI_PROPERTY_GRID_OPTION_CAPACITY + 1;
	XUI_TEST_CHECK(xuiPropertyGridSetEditorConfig(pGrid, pTheme, &tConfig) == XUI_ERROR_INVALID_ARGUMENT,
	               "oversized enum config rejected without truncation");
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.arrPalette = arrOwnedPalette;
	tConfig.iPaletteCount = XUI_COLOR_PICKER_PALETTE_CAPACITY + 1;
	XUI_TEST_CHECK(xuiPropertyGridSetEditorConfig(pGrid, pAccent, &tConfig) == XUI_ERROR_INVALID_ARGUMENT,
	               "oversized palette config rejected without deferred failure");
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.fStep = NAN;
	XUI_TEST_CHECK(xuiPropertyGridSetEditorConfig(pGrid, pMaxPlayers, &tConfig) == XUI_ERROR_INVALID_ARGUMENT,
	               "nonfinite editor metrics rejected");
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.arrEnumItems = arrThemes;
	tConfig.iEnumItemCount = 3;
	tConfig.iEnumSelected = 3;
	XUI_TEST_CHECK(xuiPropertyGridSetEditorConfig(pGrid, pTheme, &tConfig) == XUI_ERROR_INVALID_ARGUMENT,
	               "out of range enum selection rejected");
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.arrEnumItems = arrThemes;
	tConfig.iEnumItemCount = 3;
	tConfig.bEnumUseValue = 1;
	XUI_TEST_CHECK(xuiPropertyGridSetEditorConfig(pGrid, pTheme, &tConfig) == XUI_ERROR_INVALID_ARGUMENT,
	               "value enum requires item values");
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.tDefaultRangeSpan = -1;
	XUI_TEST_CHECK(xuiPropertyGridSetEditorConfig(pGrid, pMaxPlayers, &tConfig) == XUI_ERROR_INVALID_ARGUMENT,
	               "negative range span rejected");
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.bDateHasMin = 1;
	tConfig.bDateHasMax = 1;
	tConfig.tDateMin = 200;
	tConfig.tDateMax = 100;
	XUI_TEST_CHECK(xuiPropertyGridSetEditorConfig(pGrid, pMaxPlayers, &tConfig) == XUI_ERROR_INVALID_ARGUMENT,
	               "reversed date bounds rejected");

	XUI_TEST_CHECK(xuiPropertyGridGetCategoryCount(pGrid) == 3, "category count");
	XUI_TEST_CHECK(xuiPropertyGridGetPropertyCount(pGrid) == 10, "property count");
	XUI_TEST_CHECK(xuiPropertyGridFindProperty(pGrid, "theme") == pTheme, "find property");
	XUI_TEST_CHECK(xuiPropertyGridFindProperty(pGrid, "layoutDirection") == pLayoutDirection, "find value enum property");
	XUI_TEST_CHECK(xuiPropertyGridGetVisibleProperty(pGrid, 0) == -1, "category visible row");
	iVisibleExpanded = xuiPropertyGridGetVisibleCount(pGrid);
	XUI_TEST_CHECK(iVisibleExpanded == 12, "visible count excludes hidden");
	XUI_TEST_CHECK(xuiPropertyGridGetFont(pGrid) == pFont && xuiTableGridGetFont(pTableGrid) == pFont && xuiTableViewGetFont(pTableView) == pFont, "propertygrid fonts");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 640, 420, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	XUI_TEST_CHECK(xuiWidgetGetRect(pTableGrid).fH == 332.0f,
	               "description panel track reserves height");
	iRow = -1;
	for ( i = 0; i < xuiPropertyGridGetVisibleCount(pGrid); i++ ) {
		if ( xuiPropertyGridGetVisibleProperty(pGrid, i) == pLayoutDirection ) {
			iRow = i;
			break;
		}
	}
	XUI_TEST_CHECK(iRow >= 0, "value enum visible row");
	iRet = xuiTableViewEnsureCellVisible(pTableView, iRow, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "value enum ensure visible");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiPropertyGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pTarget) > 0, "initial render");
	XUI_TEST_CHECK(tData.iRenderCount > 0, "custom renderer used");
	XUI_TEST_CHECK(strcmp(xuiPropertyGridGetValue(pGrid, pLayoutDirection), "1") == 0, "value enum stores raw value");
	pViewport = xuiTableViewGetViewportWidget(pTableView);
	XUI_TEST_CHECK(pViewport != NULL, "propertygrid table viewport");
	iTableChangeCount = xuiTableViewGetChangeCount(pTableView);
	xuiWidgetClearDirty(pGrid, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pTableView, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pViewport, XUI_WIDGET_DIRTY_ALL);
	iRet = xuiPropertyGridSetGlobalRenderer(pGrid, __xuiPropertyGridCustomRender, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetChangeCount(pTableView) == iTableChangeCount &&
	               xuiWidgetGetDirtyFlags(pGrid) == 0u &&
	               xuiWidgetGetDirtyFlags(pTableView) == 0u &&
	               xuiWidgetGetDirtyFlags(pViewport) == 0u,
	               "idempotent global renderer avoids refresh");
	iRenderCount = 0;
	iRet = xuiPropertyGridSetRenderer(pGrid, pLayoutDirection,
		__xuiPropertyGridCountRender, &iRenderCount);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetChangeCount(pTableView) == iTableChangeCount &&
	               (xuiWidgetGetDirtyFlags(pTableView) & XUI_WIDGET_DIRTY_LAYOUT) == 0u &&
	               (xuiWidgetGetDirtyFlags(pTableView) & XUI_WIDGET_DIRTY_CACHE) != 0u &&
	               (xuiWidgetGetDirtyFlags(pViewport) & XUI_WIDGET_DIRTY_CACHE) != 0u,
	               "property renderer invalidates cells without data refresh");
	iRet = __xuiPropertyGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && iRenderCount > 0,
	               "replacement property renderer used");
	xuiWidgetClearDirty(pTableView, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pViewport, XUI_WIDGET_DIRTY_ALL);
	iRet = xuiPropertyGridSetRenderer(pGrid, pLayoutDirection,
		__xuiPropertyGridCountRender, &iRenderCount);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetDirtyFlags(pTableView) == 0u &&
	               xuiWidgetGetDirtyFlags(pViewport) == 0u,
	               "idempotent property renderer avoids repaint");
	iRet = xuiPropertyGridSetRenderer(pGrid, pLayoutDirection, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetChangeCount(pTableView) == iTableChangeCount &&
	               (xuiWidgetGetDirtyFlags(pViewport) & XUI_WIDGET_DIRTY_CACHE) != 0u,
	               "clearing property renderer invalidates cells only");
	xuiWidgetClearDirty(pTableView, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pViewport, XUI_WIDGET_DIRTY_ALL);
	iTableChangeCount = xuiTableViewGetChangeCount(pTableView);
	iRet = xuiPropertyGridSetValue(pGrid, pLayoutDirection, "0");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetChangeCount(pTableView) == iTableChangeCount &&
	               (xuiWidgetGetDirtyFlags(pTableView) & XUI_WIDGET_DIRTY_LAYOUT) == 0u &&
	               (xuiWidgetGetDirtyFlags(pViewport) & XUI_WIDGET_DIRTY_CACHE) != 0u,
	               "value update invalidates cells without data refresh");
	XUI_TEST_CHECK(xuiPropertyGridSetValue(pGrid, pLayoutDirection, "1") == XUI_OK,
	               "restore value after invalidation check");
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.fMin = 1.0f;
	tConfig.fMax = 64.0f;
	tConfig.fStep = 1.0f;
	xuiWidgetClearDirty(pTableView, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pViewport, XUI_WIDGET_DIRTY_ALL);
	iTableChangeCount = xuiTableViewGetChangeCount(pTableView);
	iRet = xuiPropertyGridSetEditorConfig(pGrid, pMaxPlayers, &tConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetChangeCount(pTableView) == iTableChangeCount &&
	               (xuiWidgetGetDirtyFlags(pTableView) & XUI_WIDGET_DIRTY_LAYOUT) == 0u &&
	               (xuiWidgetGetDirtyFlags(pViewport) & XUI_WIDGET_DIRTY_CACHE) != 0u,
	               "editor config invalidates cells without data refresh");
	xuiWidgetClearDirty(pTableView, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pViewport, XUI_WIDGET_DIRTY_ALL);
	XUI_TEST_CHECK(xuiPropertyGridSetEditorConfig(pGrid, pTitle, NULL) == XUI_OK &&
	               xuiWidgetGetDirtyFlags(pTableView) == 0u &&
	               xuiWidgetGetDirtyFlags(pViewport) == 0u,
	               "clearing absent editor config is idempotent");
	xuiWidgetClearDirty(pGrid, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pTableView, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pViewport, XUI_WIDGET_DIRTY_ALL);
	iTableChangeCount = xuiTableViewGetChangeCount(pTableView);
	iRet = xuiPropertyGridSetSelected(pGrid, pMaxPlayers);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetSelected(pGrid) == pMaxPlayers &&
	               xuiTableViewGetChangeCount(pTableView) == iTableChangeCount + 1 &&
	               (xuiWidgetGetDirtyFlags(pGrid) & XUI_WIDGET_DIRTY_LAYOUT) == 0u &&
	               (xuiWidgetGetDirtyFlags(pGrid) & XUI_WIDGET_DIRTY_CACHE) != 0u,
	               "programmatic selection refreshes description without table refresh");
	xuiWidgetClearDirty(pGrid, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pTableView, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pViewport, XUI_WIDGET_DIRTY_ALL);
	iTableChangeCount = xuiTableViewGetChangeCount(pTableView);
	iRet = xuiPropertyGridSetSelected(pGrid, pMaxPlayers);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetChangeCount(pTableView) == iTableChangeCount &&
	               xuiWidgetGetDirtyFlags(pGrid) == 0u &&
	               xuiWidgetGetDirtyFlags(pTableView) == 0u &&
	               xuiWidgetGetDirtyFlags(pViewport) == 0u,
	               "idempotent programmatic selection avoids refresh");
	XUI_TEST_CHECK(xuiPropertyGridSetSelected(pGrid, -1) == XUI_OK,
	               "clear programmatic selection");

	XUI_TEST_CHECK(xuiTableViewGetCellRect(pTableView, 0, 0, &tCell) == XUI_OK && tCell.fW > 100.0f, "category cell rect");
	tWorld = xuiWidgetGetWorldRect(pTableView);
	iRet = __xuiPropertyGridContextClick(pContext,
		tWorld.fX + tCell.fX + 8.0f,
		tWorld.fY + tCell.fY + tCell.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iContextCount == 1,
		"category context callback");
	XUI_TEST_CHECK(tData.iContextPart == XUI_CONTEXT_TARGET_HEADER &&
		tData.iContextCategory == iAppearance && tData.iContextProperty == -1,
		"category context semantics");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_DOWN;
	tEvent.pTarget = pTableView;
	tEvent.fX = (int)(tWorld.fX + tCell.fX + 8.0f);
	tEvent.fY = (int)(tWorld.fY + tCell.fY + tCell.fH * 0.5f);
	tEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	tEvent.iButtons = XUI_POINTER_BUTTON_LEFT;
	tEvent.iPointerId = 7;
	iRet = xuiWidgetSetEnabled(pGrid, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridBeginEdit(pGrid, pTitle) == 0 &&
	               xuiPropertyGridGetCategoryExpanded(pGrid, iAppearance) == 1 &&
	               xuiPropertyGridGetToggleCount(pGrid) == 0,
	               "disabled propertygrid ignores direct pointer input");
	tEvent.iType = XUI_EVENT_POINTER_UP;
	tEvent.iButtons = 0;
	(void)xuiDispatchEvent(pContext, &tEvent);
	iRet = xuiWidgetSetEnabled(pGrid, 1);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore propertygrid enabled");
	iRet = xuiWidgetSetVisible(pGrid, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	tEvent.iType = XUI_EVENT_POINTER_DOWN;
	tEvent.iButtons = XUI_POINTER_BUTTON_LEFT;
	if ( iRet == XUI_OK ) (void)xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(xuiPropertyGridBeginEdit(pGrid, pTitle) == 0,
	               "hidden propertygrid does not begin edit");
	XUI_TEST_CHECK(xuiPropertyGridGetCategoryExpanded(pGrid, iAppearance) == 1 &&
	               xuiPropertyGridGetToggleCount(pGrid) == 0,
	               "hidden propertygrid ignores direct pointer input");
	tEvent.iType = XUI_EVENT_POINTER_UP;
	tEvent.iButtons = 0;
	(void)xuiDispatchEvent(pContext, &tEvent);
	iRet = xuiWidgetSetVisible(pGrid, 1);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore propertygrid visible");
	iRet = __xuiPropertyGridClick(pContext, tWorld.fX + tCell.fX + 8.0f, tWorld.fY + tCell.fY + tCell.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "category click");
	XUI_TEST_CHECK(xuiPropertyGridGetCategoryExpanded(pGrid, iAppearance) == 0 && xuiPropertyGridGetToggleCount(pGrid) == 1, "category collapsed");
	XUI_TEST_CHECK(xuiPropertyGridGetVisibleCount(pGrid) < iVisibleExpanded, "visible collapsed");
	iRet = xuiPropertyGridSetCategoryExpanded(pGrid, iAppearance, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetVisibleCount(pGrid) == iVisibleExpanded, "category expand api");

	XUI_TEST_CHECK(xuiTableViewGetCellRect(pTableView, 1, 0, &tCell) == XUI_OK, "name cell rect");
	XUI_TEST_CHECK(xuiTableViewGetCellRect(pTableView, 2, 0, &tOtherCell) == XUI_OK, "other name cell rect");
	tWorld = xuiWidgetGetWorldRect(pTableView);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_UP;
	tEvent.pTarget = pTableView;
	tEvent.fX = (int)(tWorld.fX + tCell.fX + 8.0f);
	tEvent.fY = (int)(tWorld.fY + tCell.fY + tCell.fH * 0.5f);
	tEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	tEvent.iPointerId = 11;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiPropertyGridIsEditing(pGrid),
	               "unmatched name release does not begin edit");
	tEvent.iType = XUI_EVENT_POINTER_DOWN;
	tEvent.iButtons = XUI_POINTER_BUTTON_LEFT;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	tEvent.iType = XUI_EVENT_POINTER_UP;
	tEvent.iButtons = 0;
	tEvent.fX = (int)(tWorld.fX + tOtherCell.fX + 8.0f);
	tEvent.fY = (int)(tWorld.fY + tOtherCell.fY + tOtherCell.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiPropertyGridIsEditing(pGrid) &&
	               xuiPropertyGridGetSelected(pGrid) == pTitle,
	               "cross-row name release does not edit replacement row");
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

	iRow = -1;
	for ( i = 0; i < xuiPropertyGridGetVisibleCount(pGrid); i++ ) {
		if ( xuiPropertyGridGetVisibleProperty(pGrid, i) == pMaxPlayers ) {
			iRow = i;
			break;
		}
	}
	XUI_TEST_CHECK(iRow >= 0, "int visible row");
	iRet = xuiTableViewGetCellRect(pTableView, iRow, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK, "int quick rect");
	tWorld = xuiWidgetGetWorldRect(pTableView);
	iRet = __xuiPropertyGridClick(pContext, tWorld.fX + tCell.fX + tCell.fW * 0.5f, tWorld.fY + tCell.fY + tCell.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridIsEditing(pGrid), "int quick edit");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "int quick layout");
	iRet = __xuiPropertyGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "int quick render");
	XUI_TEST_CHECK(__xuiPropertyGridCheckNumericEditor(pContext, pTableGrid, "8"), "int quick editor shows initial value");
	iRet = xuiPropertyGridEndEdit(pGrid, 0);
	XUI_TEST_CHECK(iRet != 0 && !xuiPropertyGridIsEditing(pGrid), "int quick cancel");

	iRow = -1;
	for ( i = 0; i < xuiPropertyGridGetVisibleCount(pGrid); i++ ) {
		if ( xuiPropertyGridGetVisibleProperty(pGrid, i) == pTheme ) {
			iRow = i;
			break;
		}
	}
	XUI_TEST_CHECK(iRow >= 0, "enum visible row");
	iRet = xuiTableViewGetCellRect(pTableView, iRow, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK, "enum quick rect");
	tWorld = xuiWidgetGetWorldRect(pTableView);
	iRet = __xuiPropertyGridClick(pContext, tWorld.fX + tCell.fX + tCell.fW * 0.5f, tWorld.fY + tCell.fY + tCell.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridIsEditing(pGrid), "enum quick edit");
	pPopupOwner = __xuiPropertyGridFindPopupOwner(pContext, xuiComboBoxGetType(pContext));
	XUI_TEST_CHECK(pPopupOwner != NULL && xuiComboBoxIsOpen(pPopupOwner), "enum quick popup open");
	XUI_TEST_CHECK(strcmp(xuiComboBoxGetItemText(pPopupOwner, 0), "Light") == 0 &&
	               strcmp(xuiComboBoxGetItemText(pPopupOwner, 1), "Dark") == 0,
	               "enum editor uses saved text snapshot");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "enum quick layout");
	iRet = __xuiPropertyGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxIsOpen(pPopupOwner), "enum quick popup survives render");
	(void)xuiComboBoxClose(pPopupOwner);
	iRet = xuiPropertyGridEndEdit(pGrid, 0);
	XUI_TEST_CHECK(iRet != 0 && !xuiPropertyGridIsEditing(pGrid), "enum quick cancel");

	iRow = -1;
	for ( i = 0; i < xuiPropertyGridGetVisibleCount(pGrid); i++ ) {
		if ( xuiPropertyGridGetVisibleProperty(pGrid, i) == pAccent ) {
			iRow = i;
			break;
		}
	}
	XUI_TEST_CHECK(iRow >= 0, "color visible row");
	iRet = xuiTableViewGetCellRect(pTableView, iRow, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK, "color quick rect");
	tWorld = xuiWidgetGetWorldRect(pTableView);
	iRet = __xuiPropertyGridClick(pContext, tWorld.fX + tCell.fX + tCell.fW * 0.5f, tWorld.fY + tCell.fY + tCell.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridIsEditing(pGrid), "color quick edit");
	pPopupOwner = __xuiPropertyGridFindPopupOwner(pContext, xuiColorPickerGetType(pContext));
	XUI_TEST_CHECK(pPopupOwner != NULL && xuiColorPickerIsOpen(pPopupOwner), "color quick popup open");
	XUI_TEST_CHECK(xuiColorPickerGetPaletteCount(pPopupOwner) == 2 &&
	               xuiColorPickerGetPaletteColor(pPopupOwner, 0) == XUI_COLOR_RGBA(12, 34, 56, 255),
	               "color editor uses saved palette snapshot");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "color quick layout");
	iRet = __xuiPropertyGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerIsOpen(pPopupOwner), "color quick popup survives render");
	(void)xuiColorPickerClose(pPopupOwner);
	iRet = xuiPropertyGridEndEdit(pGrid, 0);
	XUI_TEST_CHECK(iRet != 0 && !xuiPropertyGridIsEditing(pGrid), "color quick cancel");

	iRet = xuiPropertyGridBeginEdit(pGrid, pMaxPlayers);
	XUI_TEST_CHECK(iRet != 0 && xuiPropertyGridIsEditing(pGrid), "int begin invalid");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "int editor layout invalid");
	iRet = __xuiPropertyGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "int editor render invalid");
	XUI_TEST_CHECK(__xuiPropertyGridCheckNumericEditor(pContext, pTableGrid, "8"), "int editor shows initial value invalid");
	iRet = __xuiPropertyGridTypeText(pContext, "bad");
	XUI_TEST_CHECK(iRet == XUI_OK, "int type invalid");
	iRet = xuiPropertyGridEndEdit(pGrid, 1);
	XUI_TEST_CHECK(iRet == 0 && xuiPropertyGridIsEditing(pGrid), "int reject");
	iRet = xuiPropertyGridSetCategoryExpanded(pGrid, iBehavior, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridIsEditing(pGrid),
	               "idempotent category update preserves edit");
	iRet = xuiPropertyGridSetMetrics(pGrid, 150.0f, 26.0f, 28.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridIsEditing(pGrid),
	               "idempotent metrics preserve edit");
	iRet = xuiPropertyGridSetDescriptionMode(pGrid, XUI_PROPERTY_GRID_DESCRIPTION_BOTH, 48.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridIsEditing(pGrid),
	               "idempotent description mode preserves edit");
	iRet = xuiPropertyGridSetPropertyFlags(pGrid, pMaxPlayers,
		XUI_PROPERTY_FLAG_DIRTY | XUI_PROPERTY_FLAG_INVALID);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridIsEditing(pGrid),
	               "visual flags preserve edit");
	xuiWidgetClearDirty(pGrid, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pTableView, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pViewport, XUI_WIDGET_DIRTY_ALL);
	iTableChangeCount = xuiTableViewGetChangeCount(pTableView);
	iRet = xuiPropertyGridSetDescriptionMode(pGrid,
		XUI_PROPERTY_GRID_DESCRIPTION_PANEL, 48.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridIsEditing(pGrid) &&
	               xuiPropertyGridGetDescriptionMode(pGrid) == XUI_PROPERTY_GRID_DESCRIPTION_PANEL &&
	               xuiTableViewGetChangeCount(pTableView) == iTableChangeCount &&
	               (xuiWidgetGetDirtyFlags(pGrid) & XUI_WIDGET_DIRTY_LAYOUT) == 0u &&
	               (xuiWidgetGetDirtyFlags(pViewport) & XUI_WIDGET_DIRTY_CACHE) != 0u,
	               "tooltip-only description change preserves invalid edit");
	iRet = xuiPropertyGridSetDescriptionMode(pGrid,
		XUI_PROPERTY_GRID_DESCRIPTION_BOTH, 48.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridIsEditing(pGrid) &&
	               xuiPropertyGridGetDescriptionMode(pGrid) == XUI_PROPERTY_GRID_DESCRIPTION_BOTH &&
	               (xuiWidgetGetDirtyFlags(pGrid) & XUI_WIDGET_DIRTY_LAYOUT) == 0u,
	               "restoring tooltip preserves invalid edit");
	iRet = xuiPropertyGridSetPropertyFlags(pGrid, pMaxPlayers,
		XUI_PROPERTY_FLAG_DIRTY | XUI_PROPERTY_FLAG_INVALID | XUI_PROPERTY_FLAG_READONLY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && xuiPropertyGridIsEditing(pGrid) &&
	               (xuiPropertyGridGetPropertyFlags(pGrid, pMaxPlayers) & XUI_PROPERTY_FLAG_READONLY) == 0,
	               "structural flags reject invalid edit transactionally");
	iRet = xuiPropertyGridSetCategoryExpanded(pGrid, iBehavior, 0);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && xuiPropertyGridIsEditing(pGrid) &&
	               xuiPropertyGridGetCategoryExpanded(pGrid, iBehavior) == 1,
	               "category collapse rejects invalid edit transactionally");
	iRet = xuiPropertyGridSetDescriptionMode(pGrid, XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && xuiPropertyGridIsEditing(pGrid) &&
	               xuiPropertyGridGetDescriptionMode(pGrid) == XUI_PROPERTY_GRID_DESCRIPTION_BOTH,
	               "description layout rejects invalid edit transactionally");
	iRet = xuiPropertyGridEndEdit(pGrid, 0);
	XUI_TEST_CHECK(iRet != 0, "int cancel");
	iRet = xuiPropertyGridSetPropertyFlags(pGrid, pMaxPlayers, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset visual flags");
	iRet = xuiPropertyGridBeginEdit(pGrid, pMaxPlayers);
	XUI_TEST_CHECK(iRet != 0 && xuiPropertyGridIsEditing(pGrid), "int begin valid");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "int editor layout valid");
	iRet = __xuiPropertyGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "int editor render valid");
	XUI_TEST_CHECK(__xuiPropertyGridCheckNumericEditor(pContext, pTableGrid, "8"), "int editor shows initial value valid");
	iRet = __xuiPropertyGridTypeText(pContext, "42");
	XUI_TEST_CHECK(iRet == XUI_OK, "int type valid");
	iRet = xuiPropertyGridEndEdit(pGrid, 1);
	XUI_TEST_CHECK(iRet != 0 && xuiPropertyGridGetInt(pGrid, pMaxPlayers, 0) == 42, "int commit");

	iRet = xuiPropertyGridBeginEdit(pGrid, pTheme);
	XUI_TEST_CHECK(iRet != 0 && xuiPropertyGridIsEditing(pGrid), "enum begin");
	iRet = xuiPropertyGridEndEdit(pGrid, 1);
	XUI_TEST_CHECK(iRet != 0 && strcmp(xuiPropertyGridGetValue(pGrid, pTheme), "Dark") == 0, "enum commit");

	iRet = xuiPropertyGridSetValue(pGrid, pLayoutDirection, "invalid");
	if ( iRet == XUI_OK ) iRet = xuiPropertyGridBeginEdit(pGrid, pLayoutDirection) ? XUI_OK : XUI_ERROR_INVALID_STATE;
	pPopupOwner = __xuiPropertyGridFindPopupOwner(pContext, xuiComboBoxGetType(pContext));
	XUI_TEST_CHECK(iRet == XUI_OK && pPopupOwner != NULL &&
	               xuiComboBoxGetSelected(pPopupOwner) == -1,
	               "malformed value enum does not select zero item");
	(void)xuiComboBoxClose(pPopupOwner);
	(void)xuiPropertyGridEndEdit(pGrid, 0);
	(void)xuiPropertyGridSetValue(pGrid, pLayoutDirection, "1");
	iRet = xuiPropertyGridBeginEdit(pGrid, pLayoutDirection);
	XUI_TEST_CHECK(iRet != 0 && xuiPropertyGridIsEditing(pGrid), "value enum begin");
	iRet = xuiPropertyGridEndEdit(pGrid, 1);
	XUI_TEST_CHECK(iRet != 0 && strcmp(xuiPropertyGridGetValue(pGrid, pLayoutDirection), "1") == 0, "value enum commit");

	iRet = xuiPropertyGridBeginEdit(pGrid, pAsset);
	XUI_TEST_CHECK(iRet != 0 && !xuiPropertyGridIsEditing(pGrid), "picker begin");
	XUI_TEST_CHECK(tData.iActionCount == 1 && strcmp(xuiPropertyGridGetValue(pGrid, pAsset), "picked.xge") == 0, "picker action");
	XUI_TEST_CHECK(xuiTableGridGetPickerCount(pTableGrid) >= 1, "tablegrid picker count");

	iRow = -1;
	for ( i = 0; i < xuiPropertyGridGetVisibleCount(pGrid); i++ ) {
		if ( xuiPropertyGridGetVisibleProperty(pGrid, i) == pReadonly ) {
			iRow = i;
			break;
		}
	}
	iRet = xuiTableViewSetSelectedCell(pTableView, iRow, 1);
	iSelectedRow = -1;
	iSelectedColumn = -1;
	(void)xuiTableViewGetSelectedCell(pTableView, &iSelectedRow, &iSelectedColumn);
	XUI_TEST_CHECK(iRet == XUI_OK && iRow >= 0 && iSelectedRow == iRow && iSelectedColumn == 1,
	               "readonly value remains selectable");
	iRet = xuiPropertyGridBeginEdit(pGrid, pReadonly);
	XUI_TEST_CHECK(iRet == 0 && !xuiPropertyGridIsEditing(pGrid), "readonly not editable");
	iRet = xuiPropertyGridBeginEdit(pGrid, pDisabled);
	XUI_TEST_CHECK(iRet == 0, "disabled not editable");
	XUI_TEST_CHECK(xuiPropertyGridGetVisibleProperty(pGrid, xuiPropertyGridGetVisibleCount(pGrid) - 1) != pHidden, "hidden not visible");

	fColumnWidth = xuiTableViewGetColumnWidth(pTableView, 1);
	iRet = xuiPropertyGridBeginEdit(pGrid, pTitle);
	XUI_TEST_CHECK(iRet != 0 && xuiPropertyGridIsEditing(pGrid), "resize edit begin");
	iRet = xuiWidgetSetRect(pGrid, (xui_rect_t){18.0f, 18.0f, 520.0f, 380.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "resize propertygrid during edit");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridIsEditing(pGrid) &&
	               xuiTableViewGetColumnWidth(pTableView, 1) == fColumnWidth,
	               "layout defers columns while editing");
	iRet = xuiPropertyGridEndEdit(pGrid, 0);
	XUI_TEST_CHECK(iRet != 0, "resize edit cancel");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetColumnWidth(pTableView, 1) > fColumnWidth + 50.0f,
	               "deferred columns synchronize after edit");
	iRet = xuiWidgetSetRect(pGrid, (xui_rect_t){18.0f, 18.0f, 420.0f, 380.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "restore propertygrid size");
	iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore propertygrid layout");
	iRet = xuiWidgetSetRect(pGrid, (xui_rect_t){18.0f, 18.0f, 48.0f, 380.0f});
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiUpdate(pContext, 0.016f);
	tViewport = xuiTableViewGetViewportRect(pTableView);
	XUI_TEST_CHECK(iRet == XUI_OK && tViewport.fW >= 2.0f &&
	               xuiTableViewGetColumnWidth(pTableView, 0) >= 1.0f &&
	               xuiTableViewGetColumnWidth(pTableView, 1) >= 1.0f &&
	               xuiTableViewGetColumnWidth(pTableView, 0) +
	               xuiTableViewGetColumnWidth(pTableView, 1) <= tViewport.fW + 0.01f,
	               "tiny propertygrid keeps columns inside viewport");
	iRet = xuiWidgetSetRect(pGrid, (xui_rect_t){18.0f, 18.0f, 420.0f, 380.0f});
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore propertygrid after tiny layout");

	iRet = xuiPropertyGridSetColor(pGrid, pAccent, XUI_COLOR_RGBA(67, 167, 124, 255));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetColor(pGrid, pAccent, 0) == XUI_COLOR_RGBA(67, 167, 124, 255), "color helpers");
	iRet = xuiPropertyGridSetPropertyFlags(pGrid, pMaxPlayers, XUI_PROPERTY_FLAG_DIRTY | XUI_PROPERTY_FLAG_INVALID);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiPropertyGridGetPropertyFlags(pGrid, pMaxPlayers) & XUI_PROPERTY_FLAG_INVALID) != 0, "flags");
	iRet = xuiPropertyGridSetScroll(pGrid, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetScroll(pGrid) >= 0.0f, "scroll");
	iRet = xuiPropertyGridSetDescriptionMode(pGrid, XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetDescriptionMode(pGrid) == XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP, "description mode");
	XUI_TEST_CHECK(xuiPropertyGridSetDescriptionMode(pGrid, 99, 0.0f) == XUI_ERROR_INVALID_ARGUMENT,
	               "invalid description mode rejected");
	XUI_TEST_CHECK(xuiPropertyGridSetDescriptionMode(pGrid, XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP, NAN) == XUI_ERROR_INVALID_ARGUMENT,
	               "nonfinite description metrics rejected");
	XUI_TEST_CHECK(xuiPropertyGridSetMetrics(pGrid, NAN, 0.0f, 0.0f) == XUI_ERROR_INVALID_ARGUMENT,
	               "nonfinite metrics rejected");
	XUI_TEST_CHECK(xuiPropertyGridSetMetrics(pGrid, 10.0f, 0.0f, 0.0f) == XUI_ERROR_INVALID_ARGUMENT &&
	               xuiPropertyGridSetMetrics(pGrid, 0.0f, XUI_LAYOUT_UNBOUNDED, 0.0f) == XUI_ERROR_INVALID_ARGUMENT,
	               "invalid and combined metrics rejected");
	XUI_TEST_CHECK(xuiPropertyGridSetDescriptionMode(pGrid, XUI_PROPERTY_GRID_DESCRIPTION_PANEL,
	               XUI_LAYOUT_UNBOUNDED) == XUI_ERROR_INVALID_ARGUMENT,
	               "combined description metrics rejected");
	iRet = xuiPropertyGridGetMetrics(pGrid, &fNameWidth, &fRowHeight, &fCategoryHeight);
	if ( iRet == XUI_OK ) iRet = xuiPropertyGridSetMetrics(pGrid, 0.0f, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetMetrics(pGrid, &tMeasured.fX,
	               &tMeasured.fY, &fColumnWidth) == XUI_OK &&
	               tMeasured.fX == fNameWidth && tMeasured.fY == fRowHeight &&
	               fColumnWidth == fCategoryHeight,
	               "zero metrics preserve configured values");
	XUI_TEST_CHECK(xuiPropertyGridSetScroll(pGrid, NAN) == XUI_ERROR_INVALID_ARGUMENT,
	               "nonfinite scroll rejected");
	XUI_TEST_CHECK(xuiPropertyGridSetFloat(pGrid, pMaxPlayers, NAN) == XUI_ERROR_INVALID_ARGUMENT &&
	               xuiPropertyGridSetFloat(pGrid, pMaxPlayers, INFINITY) == XUI_ERROR_INVALID_ARGUMENT,
	               "nonfinite float values rejected");
	iRet = xuiPropertyGridSetValue(pGrid, pMaxPlayers, "42oops");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetInt(pGrid, pMaxPlayers, 17) == 17,
	               "malformed int uses default");
	iRet = xuiPropertyGridSetValue(pGrid, pMaxPlayers, "999999999999999999999999");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetInt(pGrid, pMaxPlayers, 19) == 19,
	               "overflow int uses default");
	iRet = xuiPropertyGridSetValue(pGrid, pMaxPlayers, "nan");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetFloat(pGrid, pMaxPlayers, 2.5f) == 2.5f,
	               "nonfinite parsed float uses default");
	iRet = xuiPropertyGridSetValue(pGrid, pAccent, "#43A77C trailing");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetColor(pGrid, pAccent, 0x12345678u) == 0x12345678u,
	               "color parser rejects trailing data");
	iRet = xuiPropertyGridSetValue(pGrid, pVisible, "not-a-bool");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetBool(pGrid, pVisible, 1) == 1 &&
	               xuiPropertyGridGetBool(pGrid, pVisible, 0) == 0,
	               "malformed bool uses caller default");
	iRet = xuiPropertyGridSetValue(pGrid, pVisible, "  ON\t");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridGetBool(pGrid, pVisible, 0) == 1,
	               "bool parser accepts trimmed token");
	(void)xuiPropertyGridSetBool(pGrid, pVisible, 0);
	memset(sLongValue, 'a', XUI_PROPERTY_GRID_VALUE_CAPACITY - 3);
	sLongValue[XUI_PROPERTY_GRID_VALUE_CAPACITY - 3] = 'e';
	sLongValue[XUI_PROPERTY_GRID_VALUE_CAPACITY - 2] = (char)0xcc;
	sLongValue[XUI_PROPERTY_GRID_VALUE_CAPACITY - 1] = (char)0x81;
	sLongValue[XUI_PROPERTY_GRID_VALUE_CAPACITY] = '\0';
	iRet = xuiPropertyGridSetValue(pGrid, pTitle, sLongValue);
	XUI_TEST_CHECK(iRet == XUI_OK &&
	               strlen(xuiPropertyGridGetValue(pGrid, pTitle)) ==
	               XUI_PROPERTY_GRID_VALUE_CAPACITY - 3,
	               "long value truncates at complete UTF-8 grapheme");
	(void)xuiPropertyGridSetValue(pGrid, pTitle, "Panel");
	iRet = xuiPropertyGridSetColors(pGrid,
		XUI_COLOR_RGBA(1, 2, 3, 255), XUI_COLOR_RGBA(4, 5, 6, 255),
		XUI_COLOR_RGBA(7, 8, 9, 255), XUI_COLOR_RGBA(10, 11, 12, 255),
		XUI_COLOR_RGBA(13, 14, 15, 255), XUI_COLOR_RGBA(16, 17, 18, 255));
	if ( iRet == XUI_OK ) iRet = xuiPropertyGridGetStyle(pGrid, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK &&
	               tStyle.iCategoryTextColor == XUI_COLOR_RGBA(16, 17, 18, 255) &&
	               tStyle.iCategoryIconColor == XUI_COLOR_RGBA(16, 17, 18, 255) &&
	               tStyle.iNameTextColor == XUI_COLOR_RGBA(16, 17, 18, 255) &&
	               tStyle.iValueTextColor == XUI_COLOR_RGBA(16, 17, 18, 255),
	               "six-color helper applies generic text color consistently");
	XUI_TEST_CHECK(xuiPropertyGridSetStyle(pGrid, &tStyle) == XUI_OK,
	               "idempotent style accepted");
	(void)xuiPropertyGridSetInt(pGrid, pMaxPlayers, 42);
	(void)xuiPropertyGridSetColor(pGrid, pAccent, XUI_COLOR_RGBA(67, 167, 124, 255));

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "final layout");
	XUI_TEST_CHECK(xuiWidgetGetRect(pTableGrid).fH == 380.0f,
	               "table track fills after description panel is hidden");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "final update");
	iRet = __xuiPropertyGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pTarget) > 0, "final render");
	XUI_TEST_CHECK(xuiTableViewGetColumnCount(pTableView) == 2, "column count");

	iRet = __xuiPropertyGridCreateLifecycleGrid(pContext, pFont, XUI_TABLE_CELL_TYPE_TEXT, &pProbe, &iProbeProperty);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear lifecycle grid create");
	iRet = xuiWidgetAddChild(pRoot, pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear lifecycle grid attach");
	(void)xuiWidgetSetRect(pProbe, (xui_rect_t){20.0f, 20.0f, 260.0f, 120.0f});
	(void)xuiPropertyGridSetValidate(pProbe, __xuiPropertyGridRejectAll, NULL);
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear lifecycle grid layout");
	iRet = xuiPropertyGridBeginEdit(pProbe, iProbeProperty);
	XUI_TEST_CHECK(iRet != 0 && xuiPropertyGridIsEditing(pProbe), "clear lifecycle edit begin");
	iRet = __xuiPropertyGridTypeText(pContext, "invalid");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPropertyGridEndEdit(pProbe, 1) == 0 && xuiPropertyGridIsEditing(pProbe),
	               "clear lifecycle invalid edit");
	iRet = xuiPropertyGridClear(pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiPropertyGridIsEditing(pProbe) &&
	               xuiPropertyGridGetPropertyCount(pProbe) == 0,
	               "clear cancels invalid edit");
	xuiWidgetDestroy(pProbe);
	pProbe = NULL;

	(void)xuiWidgetSetVisible(pGrid, 0);
	iRet = __xuiPropertyGridCreateLifecycleGrid(pContext, pFont, XUI_TABLE_CELL_TYPE_TEXT, &pProbe, &iProbeProperty);
	XUI_TEST_CHECK(iRet == XUI_OK, "select lifecycle grid create");
	(void)xuiWidgetSetRect(pProbe, (xui_rect_t){20.0f, 20.0f, 260.0f, 120.0f});
	iRet = xuiWidgetAddChild(pRoot, pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK, "select lifecycle grid attach");
	iCallbackCount = 0;
	(void)xuiPropertyGridSetSelect(pProbe, __xuiPropertyGridDestroySelect, &iCallbackCount);
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "select lifecycle layout");
	pTableView = xuiPropertyGridGetTableView(pProbe);
	XUI_TEST_CHECK(pTableView != NULL && xuiTableViewGetCellRect(pTableView, 1, 0, &tCell) == XUI_OK,
	               "select lifecycle cell");
	tWorld = xuiWidgetGetWorldRect(pTableView);
	iRet = __xuiPropertyGridClick(pContext, tWorld.fX + tCell.fX + 8.0f,
		tWorld.fY + tCell.fY + tCell.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && iCallbackCount == 1,
	               "select callback may destroy propertygrid");
	pProbe = NULL;

	iRet = __xuiPropertyGridCreateLifecycleGrid(pContext, pFont, XUI_TABLE_CELL_TYPE_TEXT, &pProbe, &iProbeProperty);
	XUI_TEST_CHECK(iRet == XUI_OK, "change lifecycle grid create");
	(void)xuiWidgetSetRect(pProbe, (xui_rect_t){20.0f, 20.0f, 260.0f, 120.0f});
	iRet = xuiWidgetAddChild(pRoot, pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK, "change lifecycle grid attach");
	iCallbackCount = 0;
	(void)xuiPropertyGridSetChange(pProbe, __xuiPropertyGridDestroyChange, &iCallbackCount);
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "change lifecycle grid layout");
	iRet = xuiPropertyGridBeginEdit(pProbe, iProbeProperty);
	XUI_TEST_CHECK(iRet != 0, "change lifecycle edit begin");
	iRet = __xuiPropertyGridTypeText(pContext, "changed");
	XUI_TEST_CHECK(iRet == XUI_OK, "change lifecycle type");
	(void)xuiPropertyGridEndEdit(pProbe, 1);
	XUI_TEST_CHECK(iCallbackCount == 1, "change callback may destroy propertygrid");
	pProbe = NULL;

	iRet = __xuiPropertyGridCreateLifecycleGrid(pContext, pFont, XUI_TABLE_CELL_TYPE_TEXT, &pProbe, &iProbeProperty);
	XUI_TEST_CHECK(iRet == XUI_OK, "render lifecycle grid create");
	(void)xuiWidgetSetRect(pProbe, (xui_rect_t){20.0f, 20.0f, 260.0f, 120.0f});
	iRet = xuiWidgetAddChild(pRoot, pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK, "render lifecycle grid attach");
	iCallbackCount = 0;
	(void)xuiPropertyGridSetGlobalRenderer(pProbe, __xuiPropertyGridDestroyRender, &iCallbackCount);
	iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = __xuiPropertyGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && iCallbackCount == 1,
	               "render callback may destroy propertygrid");
	pProbe = NULL;

	iRet = __xuiPropertyGridCreateLifecycleGrid(pContext, pFont, XUI_TABLE_CELL_TYPE_TEXT, &pProbe, &iProbeProperty);
	XUI_TEST_CHECK(iRet == XUI_OK, "validate lifecycle grid create");
	(void)xuiWidgetSetRect(pProbe, (xui_rect_t){20.0f, 20.0f, 260.0f, 120.0f});
	iRet = xuiWidgetAddChild(pRoot, pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK, "validate lifecycle grid attach");
	memset(&tLifecycle, 0, sizeof(tLifecycle));
	(void)xuiPropertyGridSetValidate(pProbe, __xuiPropertyGridDestroyValidate, &tLifecycle);
	iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiPropertyGridBeginEdit(pProbe, iProbeProperty) ? XUI_OK : XUI_ERROR_INVALID_STATE;
	if ( iRet == XUI_OK ) iRet = __xuiPropertyGridTypeText(pContext, "changed");
	if ( iRet == XUI_OK ) (void)xuiPropertyGridEndEdit(pProbe, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && tLifecycle.iCallCount == 1 && tLifecycle.bIdStable,
	               "validate callback keeps id snapshot while destroying propertygrid");
	pProbe = NULL;

	iRet = __xuiPropertyGridCreateLifecycleGrid(pContext, pFont, XUI_TABLE_CELL_TYPE_CUSTOM, &pProbe, &iProbeProperty);
	XUI_TEST_CHECK(iRet == XUI_OK, "action lifecycle grid create");
	(void)xuiWidgetSetRect(pProbe, (xui_rect_t){20.0f, 20.0f, 260.0f, 120.0f});
	iRet = xuiWidgetAddChild(pRoot, pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK, "action lifecycle grid attach");
	memset(&tLifecycle, 0, sizeof(tLifecycle));
	(void)xuiPropertyGridSetAction(pProbe, iProbeProperty, __xuiPropertyGridDestroyAction, &tLifecycle);
	iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) (void)xuiPropertyGridBeginEdit(pProbe, iProbeProperty);
	XUI_TEST_CHECK(iRet == XUI_OK && tLifecycle.iCallCount == 1 && tLifecycle.bIdStable,
	               "action callback keeps id snapshot while destroying propertygrid");
	pProbe = NULL;

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
