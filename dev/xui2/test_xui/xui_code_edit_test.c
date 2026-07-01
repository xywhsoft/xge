#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_edit_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiCodeEditDispatchText(xui_context pContext, uint32_t iCodepoint)
{
	int iRet;

	iRet = xuiInputText(pContext, iCodepoint);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiCodeEditDispatchKey(xui_context pContext, int iKey, uint32_t iModifiers)
{
	int iRet;

	iRet = xuiInputKeyDown(pContext, iKey, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiCodeEditDispatchIme(xui_context pContext, const char* sText, int iCompositionLength)
{
	int iRet;

	iRet = xuiInputImeComposition(pContext, sText, (int)strlen(sText), 0, iCompositionLength);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiCodeEditPointerDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiCodeEditPointerMove(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiCodeEditPointerUp(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiCodeEditDoubleClick(xui_context pContext, float fX, float fY, uint32_t iModifiers)
{
	int iRet;

	iRet = xuiInputSetModifiers(pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCodeEditPointerDown(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCodeEditPointerUp(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCodeEditPointerDown(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCodeEditPointerUp(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	return xuiInputSetModifiers(pContext, 0u);
}

static int __xuiCodeEditRightClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, XUI_POINTER_BUTTON_RIGHT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiCodeEditWheel(xui_context pContext, float fX, float fY, float fWheelX, float fWheelY)
{
	int iRet;

	iRet = xuiInputPointerWheel(pContext, fX, fY, fWheelX, fWheelY, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiCodeEditClickMenuItem(xui_context pContext, xui_widget pMenu, int iIndex)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;
	float fX;
	float fY;
	int iRet;

	tWorld = xuiWidgetGetWorldRect(pMenu);
	tItem = xuiMenuGetItemRect(pMenu, iIndex);
	fX = tWorld.fX + tItem.fX + tItem.fW * 0.5f;
	fY = tWorld.fY + tItem.fY + tItem.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static xui_style_property_t __xuiCodeEditStyleColorProp(const char* sName, uint32_t iColor)
{
	xui_style_property_t tProperty;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	tProperty.sName = sName;
	tProperty.iDirtyFlags = XUI_WIDGET_DIRTY_STYLE | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	tProperty.tValue.iSize = sizeof(tProperty.tValue);
	tProperty.tValue.iType = XUI_STYLE_VALUE_COLOR;
	tProperty.tValue.iColor = iColor;
	return tProperty;
}

static int __xuiCodeEditCustomMarginRender(xui_widget pWidget, int iMarginId, int iLine, xui_draw_context pDraw, xui_rect_t tRect, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)iMarginId;
	(void)iLine;
	(void)pDraw;
	(void)tRect;
	pCount = (int*)pUser;
	if ( pCount != NULL ) (*pCount)++;
	return XUI_OK;
}

static int __xuiCodeEditCustomMarginEvent(xui_widget pWidget, int iMarginId, int iLine, int iEvent, xui_rect_t tRect, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)iEvent;
	(void)tRect;
	if ( iMarginId != 20 || iLine != 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	pCount = (int*)pUser;
	if ( pCount != NULL ) (*pCount)++;
	return XUI_OK;
}

static int __xuiCodeEditHostCommand(xui_widget pWidget, int iCommand, const void* pCommandData, int* pHandled, void* pUser)
{
	int* pSeen;

	(void)pWidget;
	pSeen = (int*)pUser;
	if ( (iCommand != XUI_CODE_COMMAND_USER_BASE + 1 && iCommand != XUI_CODE_COMMAND_FIND_NEXT && iCommand != XUI_CODE_COMMAND_GOTO_LINE) ||
	     pCommandData != NULL ) return XUI_ERROR_UNSUPPORTED;
	if ( pSeen != NULL ) (*pSeen)++;
	if ( pHandled != NULL ) *pHandled = 1;
	return XUI_OK;
}

static int __xuiCodeEditHostCommandEnabled(xui_widget pWidget, int iCommand, int* pEnabled, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( pEnabled == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCommand == XUI_CODE_COMMAND_FIND_NEXT || iCommand == XUI_CODE_COMMAND_GOTO_LINE ) {
		*pEnabled = 1;
		return XUI_OK;
	}
	if ( iCommand == XUI_CODE_COMMAND_REPLACE_NEXT ) {
		*pEnabled = 0;
		return XUI_OK;
	}
	return XUI_ERROR_UNSUPPORTED;
}

typedef struct xui_code_edit_find_activate_t {
	int iCount;
	xui_widget pEditor;
	int iStart;
	int iEnd;
} xui_code_edit_find_activate_t;

static void __xuiCodeEditFindActivated(xui_code_find_scope pScope, xui_widget pEditor, const xui_code_find_result_t* pResult, void* pUser)
{
	xui_code_edit_find_activate_t* pState;

	(void)pScope;
	pState = (xui_code_edit_find_activate_t*)pUser;
	if ( pState == NULL || pResult == NULL ) return;
	pState->iCount++;
	pState->pEditor = pEditor;
	pState->iStart = pResult->iStart;
	pState->iEnd = pResult->iEnd;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_code_edit_find_activate_t tFindActivate;
	xui_context pContext;
	xui_widget_type pType;
	xui_widget pRoot;
	xui_widget pCodeEdit;
	xui_widget pHScrollBar;
	xui_widget pVScrollBar;
	xui_widget pHitWidget;
	xui_widget pTabIndentEdit;
	xui_widget pToyEdit;
	xui_widget pPanel;
	xui_widget pPanelEdit;
	xui_widget pTabs;
	xui_widget pTabEditA;
	xui_widget pTabEditB;
	xui_widget pDock;
	xui_widget pDockEdit;
	xui_widget pDebuggerEdit;
	xui_widget pFindWindow;
	xui_widget pMenu;
	xui_surface pTarget;
	xui_surface pCodeCache;
	xui_font pFont;
	xui_code_edit_desc_t tDesc;
	xui_panel_desc_t tPanelDesc;
	xui_tabs_desc_t tTabsDesc;
	xui_dock_panel_desc_t tDockDesc;
	xui_dock_window_info_t tDockInfo;
	xui_code_selection_t tSelection;
	xui_code_style_t tStyle;
	xui_style_property_t tProperty;
	xui_style_property_t arrStyleProperties[2];
	xui_rect_t tImeRect;
	xui_rect_t tScrollBarRect;
	xui_rect_t tScrollBarWorld;
	xui_rect_t tScrollBarThumb;
	xui_code_range_t tRange;
	xui_code_fold_range_t tFoldRange;
	xui_code_diagnostic_t tDiagnostic;
	xui_code_margin_desc_t tCustomMargin;
	xui_code_margin_info_t tMargin;
	xui_code_language_t tToyLanguage;
	xui_code_indicator_t arrIndicators[4];
	xui_find_options_t tFind;
	xui_code_find_result_t tFindResult;
	xui_code_find_scope pFindScope;
	xui_code_theme pOverrideTheme;
	xui_scroll_model_t* pScrollModel;
	xui_layout_t tLayout;
	xui_cache_policy_t tCachePolicy;
	char sLongText[2048];
	const char* arrTabItems[2];
	int iMarginCount;
	int iCustomMarginCallbackCount;
	int iSelectStart;
	int iSelectEnd;
	int iLine;
	int iColumn;
	int iTokenCount;
	int iTextDrawCount;
	int iHostCommandSeen;
	int iDockWindow;
	int iDockPane;
	int iLongTextOffset;
	int iLongLine;
	uint32_t iCurrentLineColor;
	uint32_t iReadonlyBackgroundColor;
	uint32_t iImeColor;
	float fTextOriginX;
	float fPointerCharWidth;
	float fScrollX;
	float fScrollY;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pCodeEdit = NULL;
	pHScrollBar = NULL;
	pVScrollBar = NULL;
	pTabIndentEdit = NULL;
	pToyEdit = NULL;
	pPanel = NULL;
	pPanelEdit = NULL;
	pTabs = NULL;
	pTabEditA = NULL;
	pTabEditB = NULL;
	pDock = NULL;
	pDockEdit = NULL;
	pDebuggerEdit = NULL;
	pFindWindow = NULL;
	pMenu = NULL;
	pTarget = NULL;
	pCodeCache = NULL;
	pFont = NULL;
	pOverrideTheme = NULL;
	pFindScope = NULL;
	pScrollModel = NULL;
	memset(&tFindActivate, 0, sizeof(tFindActivate));
	iHostCommandSeen = 0;
	iDockWindow = -1;
	iDockPane = -1;
	iCustomMarginCallbackCount = 0;
	iCurrentLineColor = XUI_COLOR_RGBA(91, 141, 239, 96);
	iReadonlyBackgroundColor = XUI_COLOR_RGBA(236, 241, 248, 255);
	iImeColor = XUI_COLOR_RGBA(22, 104, 214, 255);
	iFailed = 0;
	xuiTestProxyInit(&tState);
	fPointerCharWidth = 14.0f * 0.5f;

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadFile(&tState.tProxy, &pFont, "codeedit.ttf", 14.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 640.0f, 480.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport set");
	pType = xuiCodeEditGetType(pContext);
	XUI_TEST_CHECK(pType != NULL && strcmp(xuiWidgetTypeGetName(pType), "codeedit") == 0, "type register");
	XUI_TEST_CHECK(xuiCodeEditGetType(pContext) == pType, "type reuse");
	XUI_TEST_CHECK(xuiStyleFindProperty(pContext, "codeedit.whitespace.color") != 0u, "codeedit style property registered");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 640.0f, 480.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.sText = "int main(void) {\n\treturn 0;\n}\n";
	tDesc.sLanguage = "c";
	tDesc.bReadonly = 1;
	tDesc.bShowLineNumbers = 1;
	tDesc.bShowFoldMargin = 1;
	tDesc.bShowMarkerMargin = 1;
	tDesc.bShowDiagnosticMargin = 1;
	tDesc.iTabColumns = 4;
	tDesc.iIndentColumns = 4;
	iRet = xuiCodeEditCreate(pContext, &pCodeEdit, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pCodeEdit != NULL, "create");
	tLayout = xuiWidgetGetLayout(pCodeEdit);
	XUI_TEST_CHECK(tLayout.iOverflow == XUI_OVERFLOW_HIDDEN && tLayout.iWidthMode == XUI_SIZE_CONTENT && tLayout.iHeightMode == XUI_SIZE_CONTENT, "default layout applied");
	tCachePolicy = xuiWidgetGetCachePolicy(pCodeEdit);
	XUI_TEST_CHECK(tCachePolicy.iPolicy == XUI_CACHE_POLICY_SELF && (tCachePolicy.iFlags & XUI_CACHE_CLEAR_ON_UPDATE) != 0, "default cache policy applied");
	xuiWidgetSetRect(pCodeEdit, (xui_rect_t){20.0f, 30.0f, 300.0f, 220.0f});
	iRet = xuiWidgetAddChild(pRoot, pCodeEdit);
	XUI_TEST_CHECK(iRet == XUI_OK, "add child");
	iRet = xuiCodeAnnotationSetMarker(xuiCodeEditGetAnnotations(pCodeEdit), 1, XUI_CODE_MARKER_BOOKMARK, 0u, "mark", 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "margin marker set");
	memset(&tDiagnostic, 0, sizeof(tDiagnostic));
	tDiagnostic.iSize = sizeof(tDiagnostic);
	tDiagnostic.tRange.iStart = 18;
	tDiagnostic.tRange.iEnd = 24;
	tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
	tDiagnostic.sMessage = "diagnostic";
	iRet = xuiCodeAnnotationSetDiagnostics(xuiCodeEditGetAnnotations(pCodeEdit), &tDiagnostic, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "margin diagnostic set");
	memset(&tFoldRange, 0, sizeof(tFoldRange));
	tFoldRange.iSize = sizeof(tFoldRange);
	tFoldRange.iStartLine = 0;
	tFoldRange.iEndLine = 2;
	tFoldRange.iFlags = XUI_CODE_FOLD_HEADER;
	iRet = xuiCodeFoldStateSetRanges(xuiCodeEditGetFoldState(pCodeEdit), &tFoldRange, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "margin fold set");
	memset(&tCustomMargin, 0, sizeof(tCustomMargin));
	tCustomMargin.iSize = sizeof(tCustomMargin);
	tCustomMargin.iId = 20;
	tCustomMargin.iKind = XUI_CODE_MARGIN_CUSTOM;
	tCustomMargin.fWidth = 10.0f;
	tCustomMargin.iFlags = XUI_CODE_MARGIN_VISIBLE;
	tCustomMargin.onRender = __xuiCodeEditCustomMarginRender;
	tCustomMargin.onEvent = __xuiCodeEditCustomMarginEvent;
	tCustomMargin.pUser = &iCustomMarginCallbackCount;
	iRet = xuiCodeMarginModelAdd(xuiCodeEditGetMargins(pCodeEdit), &tCustomMargin);
	XUI_TEST_CHECK(iRet == XUI_OK, "custom margin add");
	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 640, 480, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "render initial");
	pCodeCache = xuiWidgetGetCacheSurface(pCodeEdit, xuiWidgetGetStateId(pCodeEdit));
	XUI_TEST_CHECK(pCodeCache != NULL, "codeedit cache surface");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillCount(pCodeCache) > 0, "render rect fills");
	XUI_TEST_CHECK(xuiTestSurfaceGetTextDrawCount(pCodeCache) >= 6, "render text and margins");
	XUI_TEST_CHECK(iCustomMarginCallbackCount > 0, "custom margin render callback");
	iTokenCount = xuiCodeTokenBufferGetCount(xuiCodeEditGetTokenBuffer(pCodeEdit));
	XUI_TEST_CHECK(iTokenCount > 0, "render populates token buffer");
	arrStyleProperties[0] = __xuiCodeEditStyleColorProp("codeedit.current_line.color", iCurrentLineColor);
	arrStyleProperties[1] = __xuiCodeEditStyleColorProp("codeedit.readonly.background_color", iReadonlyBackgroundColor);
	iRet = xuiWidgetSetInlineStyle(pCodeEdit, arrStyleProperties, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "base render inline styles set");
	xuiTestSurfaceReset(pCodeCache);
	(void)xuiTestSurfaceGetRectFillColorCount(pCodeCache, iCurrentLineColor);
	(void)xuiTestSurfaceGetRectFillColorCount(pCodeCache, iReadonlyBackgroundColor);
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "base render styled");
	pCodeCache = xuiWidgetGetCacheSurface(pCodeEdit, xuiWidgetGetStateId(pCodeEdit));
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillColorCount(pCodeCache, iCurrentLineColor) > 0, "current line renders");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillColorCount(pCodeCache, iReadonlyBackgroundColor) > 0, "readonly background renders");
	iRet = xuiWidgetSetInlineStyle(pCodeEdit, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "base render inline styles clear");
	XUI_TEST_CHECK(xuiWidgetIsType(pCodeEdit, pType), "widget type");
	XUI_TEST_CHECK(xuiCodeEditGetDocument(pCodeEdit) != NULL, "document handle");
	XUI_TEST_CHECK(xuiCodeEditGetSelection(pCodeEdit) != NULL, "selection handle");
	XUI_TEST_CHECK(xuiCodeEditGetTheme(pCodeEdit) != NULL, "theme handle");
	XUI_TEST_CHECK(xuiCodeEditGetFoldState(pCodeEdit) != NULL, "fold state handle");
	XUI_TEST_CHECK(xuiCodeEditGetAnnotations(pCodeEdit) != NULL, "annotation handle");
	XUI_TEST_CHECK(xuiCodeEditGetTokenBuffer(pCodeEdit) != NULL, "token buffer handle");
	XUI_TEST_CHECK(xuiCodeEditGetProviders(pCodeEdit) != NULL, "provider handle");
	XUI_TEST_CHECK(xuiCodeEditGetMargins(pCodeEdit) != NULL, "margin handle");
	XUI_TEST_CHECK(xuiCodeEditGetCommandMap(pCodeEdit) != NULL, "command map handle");
	XUI_TEST_CHECK(xuiCodeEditGetLanguageRegistry(pCodeEdit) != NULL, "language registry handle");
	pMenu = xuiCodeEditGetMenuWidget(pCodeEdit);
	XUI_TEST_CHECK(pMenu != NULL, "menu handle");
	XUI_TEST_CHECK(strcmp(xuiCodeEditGetText(pCodeEdit), "int main(void) {\n\treturn 0;\n}\n") == 0, "initial text");
	XUI_TEST_CHECK(xuiCodeEditIsReadonly(pCodeEdit) == 1, "readonly initial");
	XUI_TEST_CHECK(xuiCodeEditGetDisplayOptions(pCodeEdit) == 0u, "display options initial");
	iRet = xuiCodeMarginModelGetCount(xuiCodeEditGetMargins(pCodeEdit));
	XUI_TEST_CHECK(iRet == 6, "default margin count");
	iRet = xuiCodeMarginModelGet(xuiCodeEditGetMargins(pCodeEdit), 0, &tMargin);
	XUI_TEST_CHECK(iRet == XUI_OK && tMargin.iKind == XUI_CODE_MARGIN_CHANGE, "change margin");
	iRet = xuiCodeMarginModelGetTotalWidth(xuiCodeEditGetMargins(pCodeEdit), &fTextOriginX);
	XUI_TEST_CHECK(iRet == XUI_OK && fTextOriginX > 0.0f, "margin total width");
	iMarginCount = xuiCodeCommandMapGetCount(xuiCodeEditGetCommandMap(pCodeEdit));
	XUI_TEST_CHECK(iMarginCount > 0, "command map defaults");
	iCustomMarginCallbackCount = 0;
	iRet = __xuiCodeEditPointerDown(pContext, 20.0f + fTextOriginX - 5.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && iCustomMarginCallbackCount == 1, "custom margin pointer down event");
	iRet = __xuiCodeEditPointerUp(pContext, 20.0f + fTextOriginX - 5.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && iCustomMarginCallbackCount == 2, "custom margin pointer up event");
	iRet = xuiCodeFoldStateGetRange(xuiCodeEditGetFoldState(pCodeEdit), 0, &tFoldRange);
	XUI_TEST_CHECK(iRet == XUI_OK && (tFoldRange.iFlags & XUI_CODE_FOLD_COLLAPSED) == 0u, "fold margin initially expanded");
	iRet = __xuiCodeEditPointerDown(pContext, 20.0f + 4.0f + 16.0f + 7.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "fold margin pointer down");
	iRet = xuiCodeFoldStateGetRange(xuiCodeEditGetFoldState(pCodeEdit), 0, &tFoldRange);
	XUI_TEST_CHECK(iRet == XUI_OK && (tFoldRange.iFlags & XUI_CODE_FOLD_COLLAPSED) != 0u, "fold margin toggles collapsed");
	iRet = __xuiCodeEditPointerUp(pContext, 20.0f + 4.0f + 16.0f + 7.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "fold margin pointer up");
	iRet = __xuiCodeEditPointerDown(pContext, 20.0f + 4.0f + 16.0f + 7.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "fold margin expand pointer down");
	iRet = xuiCodeFoldStateGetRange(xuiCodeEditGetFoldState(pCodeEdit), 0, &tFoldRange);
	XUI_TEST_CHECK(iRet == XUI_OK && (tFoldRange.iFlags & XUI_CODE_FOLD_COLLAPSED) == 0u, "fold margin toggles expanded");
	iRet = __xuiCodeEditPointerUp(pContext, 20.0f + 4.0f + 16.0f + 7.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "fold margin expand pointer up");
	iRet = __xuiCodeEditPointerDown(pContext, 20.0f + 4.0f + 8.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "marker margin pointer down consumes event");
	iRet = __xuiCodeEditPointerUp(pContext, 20.0f + 4.0f + 8.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "marker margin pointer up");

	iRet = xuiCodeEditSetReadonly(pCodeEdit, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeEditIsReadonly(pCodeEdit) == 0, "readonly set");
	iRet = xuiCodeEditSetText(pCodeEdit, "alpha\r\nbeta");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pCodeEdit), "alpha\nbeta") == 0, "set text");
	iRet = xuiCodeDocumentGetLastEditRange(xuiCodeEditGetDocument(pCodeEdit), &tRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tRange.iStart == 0 && tRange.iEnd == 10, "document edit range");
	iRet = xuiCodeEditSetText(pCodeEdit, "\xC3\x28");
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && strstr(xuiCodeEditGetLastError(pCodeEdit), "UTF-8") != NULL, "codeedit invalid utf8 error");
	iRet = xuiCodeEditSetText(pCodeEdit, "alpha\nbeta");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeEditGetLastError(pCodeEdit)[0] == '\0', "codeedit error clears after set text");
	iRet = xuiSetFocusWidget(pContext, pCodeEdit);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pCodeEdit, "focus codeedit");
	XUI_TEST_CHECK(xuiWidgetGetImeMode(pCodeEdit) == XUI_IME_ENABLED && xuiHasImeCandidateRect(pContext), "ime enabled on focus");
	tImeRect = xuiGetImeCandidateRect(pContext);
	XUI_TEST_CHECK(tImeRect.fX >= 20.0f && tImeRect.fY >= 30.0f && tImeRect.fW > 0.0f && tImeRect.fH > 0.0f, "ime candidate rect");
	iRet = xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pCodeEdit), xuiCodeEditGetDocument(pCodeEdit), 5, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "caret set");
	iRet = __xuiCodeEditDispatchText(pContext, '!');
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pCodeEdit), "alpha!\nbeta") == 0, "text event insert");
	iRet = __xuiCodeEditDispatchKey(pContext, XUI_KEY_BACKSPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pCodeEdit), "alpha\nbeta") == 0, "key command delete back");
	iRet = __xuiCodeEditDispatchText(pContext, 0x4F60u);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pCodeEdit), "alpha\xE4\xBD\xA0\nbeta") == 0, "unicode text event insert");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "unicode layout");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "unicode render");
	iRet = __xuiCodeEditDispatchKey(pContext, XUI_KEY_BACKSPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pCodeEdit), "alpha\nbeta") == 0, "unicode backspace deletes character");
	iRet = xuiCodeEditSetText(pCodeEdit, "alpha\nbeta");
	XUI_TEST_CHECK(iRet == XUI_OK, "unicode text reset");
	iRet = xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pCodeEdit), xuiCodeEditGetDocument(pCodeEdit), 5, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab caret reset");
	iRet = __xuiCodeEditDispatchKey(pContext, XUI_KEY_TAB, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pCodeEdit), "alpha\t\nbeta") == 0, "key command tab default");
	iRet = xuiCodeEditSetText(pCodeEdit, "\t\talpha\nbeta");
	XUI_TEST_CHECK(iRet == XUI_OK, "auto indent text reset");
	iRet = xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pCodeEdit), xuiCodeEditGetDocument(pCodeEdit), 7, 7);
	XUI_TEST_CHECK(iRet == XUI_OK, "auto indent caret set");
	iRet = __xuiCodeEditDispatchKey(pContext, XUI_KEY_ENTER, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pCodeEdit), "\t\talpha\n\t\t\nbeta") == 0, "enter inherits tab indent");
	iRet = xuiCodeSelectionGetState(xuiCodeEditGetSelection(pCodeEdit), &tSelection);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelection.iCaretOffset == 10, "enter caret after inherited indent");
	iRet = xuiCodeEditSetText(pCodeEdit, "    alpha\nbeta");
	XUI_TEST_CHECK(iRet == XUI_OK, "auto space indent text reset");
	iRet = xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pCodeEdit), xuiCodeEditGetDocument(pCodeEdit), 9, 9);
	XUI_TEST_CHECK(iRet == XUI_OK, "auto space indent caret set");
	iRet = __xuiCodeEditDispatchKey(pContext, XUI_KEY_ENTER, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pCodeEdit), "    alpha\n    \nbeta") == 0, "enter inherits space indent");
	iRet = xuiCodeEditSetText(pCodeEdit, "    alpha\n    beta");
	XUI_TEST_CHECK(iRet == XUI_OK, "outdent text reset");
	iRet = xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pCodeEdit), xuiCodeEditGetDocument(pCodeEdit), 0, xuiCodeDocumentGetLength(xuiCodeEditGetDocument(pCodeEdit)));
	XUI_TEST_CHECK(iRet == XUI_OK, "outdent range set");
	iRet = xuiCommandDispatch(pContext, pCodeEdit, XUI_CODE_COMMAND_OUTDENT, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "outdent dispatch");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pCodeEdit), "alpha\nbeta") == 0, "outdent command removes spaces");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.sText = "tab";
	tDesc.iTabColumns = 4;
	tDesc.iIndentColumns = 4;
	tDesc.iFlags = XUI_CODE_EDIT_INDENT_WITH_TABS;
	iRet = xuiCodeEditCreate(pContext, &pTabIndentEdit, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTabIndentEdit != NULL, "tab indent codeedit create");
	iRet = xuiCodeSelectionGotoOffset(xuiCodeEditGetSelection(pTabIndentEdit), xuiCodeEditGetDocument(pTabIndentEdit), 3, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab indent caret");
	iRet = xuiCommandDispatch(pContext, pTabIndentEdit, XUI_CODE_COMMAND_INSERT_TAB, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab indent dispatch");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pTabIndentEdit), "tab\t") == 0, "tab indent policy");
	iRet = xuiCodeEditSetText(pTabIndentEdit, "wide");
	XUI_TEST_CHECK(iRet == XUI_OK, "expand tabs text reset");
	iRet = xuiCodeSelectionGotoOffset(xuiCodeEditGetSelection(pTabIndentEdit), xuiCodeEditGetDocument(pTabIndentEdit), 4, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "expand tabs caret");
	iRet = xuiCodeEditSetIndentColumns(pTabIndentEdit, 3);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeEditGetIndentColumns(pTabIndentEdit) == 3, "indent columns set");
	iRet = xuiCodeEditSetExpandTabs(pTabIndentEdit, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeEditGetExpandTabs(pTabIndentEdit) == 1, "expand tabs enabled");
	iRet = xuiCommandDispatch(pContext, pTabIndentEdit, XUI_CODE_COMMAND_INSERT_TAB, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "expand tabs dispatch");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pTabIndentEdit), "wide   ") == 0, "expand tabs inserts spaces");
	iRet = xuiCodeEditSetText(pTabIndentEdit, "raw");
	XUI_TEST_CHECK(iRet == XUI_OK, "raw tab text reset");
	iRet = xuiCodeSelectionGotoOffset(xuiCodeEditGetSelection(pTabIndentEdit), xuiCodeEditGetDocument(pTabIndentEdit), 3, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "raw tab caret");
	iRet = xuiCodeEditSetExpandTabs(pTabIndentEdit, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeEditGetExpandTabs(pTabIndentEdit) == 0, "expand tabs disabled");
	iRet = xuiCommandDispatch(pContext, pTabIndentEdit, XUI_CODE_COMMAND_INSERT_TAB, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "raw tab dispatch");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pTabIndentEdit), "raw\t") == 0, "raw tab after disabling expand");
	iRet = xuiCodeEditSetText(pCodeEdit, "l00\nl01\nl02\nl03\nl04\nl05\nl06\nl07\nl08\nl09\nl10\nl11\nl12\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "page key text reset");
	iRet = xuiCodeSelectionGotoLineColumn(xuiCodeEditGetSelection(pCodeEdit), xuiCodeEditGetDocument(pCodeEdit), 0, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "page key caret set");
	iRet = __xuiCodeEditDispatchKey(pContext, XUI_KEY_PAGE_DOWN, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "page down key");
	iRet = xuiCodeSelectionGetState(xuiCodeEditGetSelection(pCodeEdit), &tSelection);
	XUI_TEST_CHECK(iRet == XUI_OK, "page down state");
	iRet = xuiCodeDocumentOffsetToLineColumn(xuiCodeEditGetDocument(pCodeEdit), tSelection.iCaretOffset, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == XUI_OK && iLine == 11 && iColumn == 0, "page down viewport target");
	iRet = __xuiCodeEditDispatchKey(pContext, XUI_KEY_PAGE_UP, XUI_MOD_SHIFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "shift page up key");
	iRet = xuiCodeSelectionGetState(xuiCodeEditGetSelection(pCodeEdit), &tSelection);
	XUI_TEST_CHECK(iRet == XUI_OK, "shift page up state");
	iRet = xuiCodeDocumentOffsetToLineColumn(xuiCodeEditGetDocument(pCodeEdit), tSelection.iCaretOffset, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == XUI_OK && iLine == 0 && iColumn == 0 && tSelection.iAnchorOffset != tSelection.iCaretOffset, "shift page up selection");
	iRet = xuiInputKeyUp(pContext, XUI_KEY_PAGE_UP, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "shift page up key release");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "shift page up release dispatch");
	iRet = xuiCodeEditSetText(pCodeEdit, "alpha    \nbeta");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore text after page key");
	iRet = xuiCodeEditSetText(pCodeEdit, "foo foo food");
	XUI_TEST_CHECK(iRet == XUI_OK, "replace plain text reset");
	iRet = xuiCodeEditReplaceAllPlain(pCodeEdit, "foo", "bar", XUI_CODE_SEARCH_WHOLE_WORD, &iMarginCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iMarginCount == 2 && strcmp(xuiCodeEditGetText(pCodeEdit), "bar bar food") == 0, "codeedit plain replace all");
	iRet = xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pCodeEdit), &iSelectStart, &iSelectEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectStart == 0 && iSelectEnd == 7, "plain replace selection range");
	iRet = xuiCodeAnnotationGetIndicatorsAt(xuiCodeEditGetAnnotations(pCodeEdit), 1, arrIndicators, 4, &iMarginCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iMarginCount == 1 && arrIndicators[0].iIndicator == XUI_CODE_INDICATOR_SEARCH_RESULT, "plain replace indicators");
	iRet = xuiCodeEditSetText(pCodeEdit, "int a = 1;\nint b = 2;\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "replace regex text reset");
	iRet = xuiCodeEditReplaceAllRegex(pCodeEdit, "int ([a-z]) = ([0-9]);", "long $1 = $2;", XUI_CODE_SEARCH_CASE_SENSITIVE, &iMarginCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iMarginCount == 2 && strcmp(xuiCodeEditGetText(pCodeEdit), "long a = 1;\nlong b = 2;\n") == 0, "codeedit regex replace all");
	iRet = xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pCodeEdit), &iSelectStart, &iSelectEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectStart == 0 && iSelectEnd == xuiCodeDocumentGetLength(xuiCodeEditGetDocument(pCodeEdit)), "regex replace selection range");
	iRet = xuiCodeEditSetText(pCodeEdit, "alpha    \nbeta");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore text after replace");
	iRet = xuiCodeEditSetText(pCodeEdit, "alpha beta alpha\nbeta alpha");
	XUI_TEST_CHECK(iRet == XUI_OK, "find api text reset");
	iRet = xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pCodeEdit), xuiCodeEditGetDocument(pCodeEdit), 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "find api caret start");
	memset(&tFind, 0, sizeof(tFind));
	tFind.iSize = sizeof(tFind);
	tFind.sPattern = "alpha";
	tFind.iFlags = XUI_FIND_CASE_SENSITIVE;
	iRet = xuiCodeEditFindAll(pCodeEdit, &tFind, &iMarginCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iMarginCount == 3 && xuiCodeEditGetFindResultCount(pCodeEdit) == 3, "codeedit find all");
	iRet = xuiCodeAnnotationGetIndicatorsAt(xuiCodeEditGetAnnotations(pCodeEdit), 1, arrIndicators, 4, &iMarginCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iMarginCount == 1 && arrIndicators[0].iIndicator == XUI_CODE_INDICATOR_SEARCH_RESULT, "codeedit find indicators");
	iRet = xuiCodeEditFindNext(pCodeEdit, &tFind);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit find next");
	iRet = xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pCodeEdit), &iSelectStart, &iSelectEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectStart == 0 && iSelectEnd == 5, "codeedit find next selection");
	iRet = xuiCodeEditFindNext(pCodeEdit, &tFind);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit find next second");
	iRet = xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pCodeEdit), &iSelectStart, &iSelectEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectStart == 11 && iSelectEnd == 16, "codeedit find next second selection");
	iRet = xuiCodeEditFindPrevious(pCodeEdit, &tFind);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit find previous");
	iRet = xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pCodeEdit), &iSelectStart, &iSelectEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectStart == 0 && iSelectEnd == 5, "codeedit find previous selection");
	iRet = xuiCodeEditGetFindResult(pCodeEdit, 1, &tFindResult);
	XUI_TEST_CHECK(iRet == XUI_OK && tFindResult.iStart == 11 && tFindResult.iLine == 0 && strstr(tFindResult.sPreview, "alpha beta") != NULL, "codeedit find result details");

	iRet = xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pCodeEdit), xuiCodeEditGetDocument(pCodeEdit), 11, 27);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit selection find range");
	tFind.iFlags = XUI_FIND_CASE_SENSITIVE | XUI_FIND_SELECTION;
	iRet = xuiCodeEditFindNext(pCodeEdit, &tFind);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit selection find first");
	iRet = xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pCodeEdit), &iSelectStart, &iSelectEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectStart == 11 && iSelectEnd == 16, "codeedit selection find first range");
	iRet = xuiCodeEditFindNext(pCodeEdit, &tFind);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit selection find second");
	iRet = xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pCodeEdit), &iSelectStart, &iSelectEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectStart == 22 && iSelectEnd == 27, "codeedit selection find stays in selection");

	iRet = xuiCodeEditSetText(pCodeEdit, "cat dog cat");
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit replace current text");
	iRet = xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pCodeEdit), xuiCodeEditGetDocument(pCodeEdit), 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit replace current caret");
	memset(&tFind, 0, sizeof(tFind));
	tFind.iSize = sizeof(tFind);
	tFind.sPattern = "cat";
	tFind.sReplacement = "fox";
	tFind.iFlags = XUI_FIND_CASE_SENSITIVE;
	iRet = xuiCodeEditFindNext(pCodeEdit, &tFind);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit replace find current");
	iRet = xuiCodeEditReplaceCurrent(pCodeEdit, &tFind);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pCodeEdit), "fox dog cat") == 0, "codeedit replace current");
	iRet = xuiCodeEditReplaceAll(pCodeEdit, &tFind, &iMarginCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iMarginCount == 1 && strcmp(xuiCodeEditGetText(pCodeEdit), "fox dog fox") == 0, "codeedit replace all find api");

	iRet = xuiCodeEditSetText(pCodeEdit, "int a = 1;\nint b = 2;\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit regex replace text");
	memset(&tFind, 0, sizeof(tFind));
	tFind.iSize = sizeof(tFind);
	tFind.sPattern = "int ([a-z]) = ([0-9]);";
	tFind.sReplacement = "long $1 = $2;";
	tFind.iFlags = XUI_FIND_CASE_SENSITIVE | XUI_FIND_REGEX;
	iRet = xuiCodeEditReplaceAll(pCodeEdit, &tFind, &iMarginCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iMarginCount == 2 && strcmp(xuiCodeEditGetText(pCodeEdit), "long a = 1;\nlong b = 2;\n") == 0, "codeedit regex replace find api");
	iRet = xuiCodeEditSetText(pCodeEdit, "a\nb");
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit escape find text");
	memset(&tFind, 0, sizeof(tFind));
	tFind.iSize = sizeof(tFind);
	tFind.sPattern = "a\\nb";
	tFind.iFlags = XUI_FIND_CASE_SENSITIVE | XUI_FIND_ESCAPE;
	iRet = xuiCodeEditFindNext(pCodeEdit, &tFind);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit escape find");
	iRet = xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pCodeEdit), &iSelectStart, &iSelectEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectStart == 0 && iSelectEnd == 3, "codeedit escape find selection");

	iRet = xuiCodeEditSetText(pCodeEdit, "find me");
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit hotkey find text");
	iRet = xuiSetFocusWidget(pContext, pCodeEdit);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit focus before find hotkey");
	iRet = __xuiCodeEditDispatchKey(pContext, 'F', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit ctrl f");
	pFindWindow = xuiCodeEditGetFindWindow(pCodeEdit);
	XUI_TEST_CHECK(pFindWindow != NULL && xuiWindowIsOpen(pFindWindow), "codeedit ctrl f opens find window");
	iRet = xuiSetFocusWidget(pContext, pCodeEdit);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit focus before replace hotkey");
	iRet = __xuiCodeEditDispatchKey(pContext, 'H', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit ctrl h");
	pFindWindow = xuiCodeEditGetFindWindow(pCodeEdit);
	XUI_TEST_CHECK(pFindWindow != NULL && xuiWindowIsOpen(pFindWindow), "codeedit ctrl h opens replace window");
	iRet = xuiWindowSetOpen(pFindWindow, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit close find window");

	iRet = xuiCodeEditSetText(pCodeEdit, "main alpha\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "scope main text");
	iRet = xuiCodeEditSetText(pTabIndentEdit, "other alpha\nalpha");
	XUI_TEST_CHECK(iRet == XUI_OK, "scope second text");
	iRet = xuiCodeFindScopeCreate(&pFindScope);
	XUI_TEST_CHECK(iRet == XUI_OK && pFindScope != NULL, "find scope create");
	iRet = xuiCodeFindScopeAddEditor(pFindScope, pCodeEdit);
	XUI_TEST_CHECK(iRet == XUI_OK, "find scope add main");
	iRet = xuiCodeFindScopeAddEditor(pFindScope, pTabIndentEdit);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeFindScopeGetEditorCount(pFindScope) == 2, "find scope add second");
	iRet = xuiCodeFindScopeSetActivate(pFindScope, __xuiCodeEditFindActivated, &tFindActivate);
	XUI_TEST_CHECK(iRet == XUI_OK, "find scope activate callback");
	memset(&tFind, 0, sizeof(tFind));
	tFind.iSize = sizeof(tFind);
	tFind.sPattern = "alpha";
	tFind.iFlags = XUI_FIND_CASE_SENSITIVE;
	iRet = xuiCodeFindScopeFindAll(pFindScope, &tFind, &iMarginCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iMarginCount == 3 && xuiCodeFindScopeGetResultCount(pFindScope) == 3, "find scope all");
	iRet = xuiCodeFindScopeGetResult(pFindScope, 1, &tFindResult);
	XUI_TEST_CHECK(iRet == XUI_OK && tFindResult.pEditor == pTabIndentEdit && tFindResult.iStart == 6, "find scope result editor");
	iRet = xuiCodeFindScopeActivateResult(pFindScope, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && tFindActivate.iCount == 1 && tFindActivate.pEditor == pTabIndentEdit, "find scope activate");
	iRet = xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pTabIndentEdit), &iSelectStart, &iSelectEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectStart == 12 && iSelectEnd == 17, "find scope activated selection");
	xuiCodeFindScopeDestroy(pFindScope);
	pFindScope = NULL;
	iRet = xuiCodeEditSetText(pCodeEdit, "alpha    \nbeta");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore text after find api");
	iRet = xuiSetFocusWidget(pContext, pCodeEdit);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore focus after find api");
	iRet = xuiCommandDispatch(pContext, pCodeEdit, XUI_CODE_COMMAND_SELECT_ALL, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "command dispatch select all");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch command event");
	iRet = xuiCodeSelectionGetState(xuiCodeEditGetSelection(pCodeEdit), &tSelection);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelection.iAnchorOffset == 0 && tSelection.iCaretOffset == xuiCodeDocumentGetLength(xuiCodeEditGetDocument(pCodeEdit)), "command selected all");
	iRet = xuiCodeEditSetReadonly(pCodeEdit, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "readonly enable");
	iRet = __xuiCodeEditDispatchText(pContext, '?');
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pCodeEdit), "alpha    \nbeta") == 0, "readonly blocks text event");
	iRet = xuiCodeEditSetReadonly(pCodeEdit, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "readonly disable for ime");
	iRet = xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pCodeEdit), xuiCodeEditGetDocument(pCodeEdit), 5, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "ime caret set");
	tProperty = __xuiCodeEditStyleColorProp("codeedit.ime.color", iImeColor);
	iRet = xuiWidgetSetInlineStyle(pCodeEdit, &tProperty, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "ime inline style set");
	iRet = __xuiCodeEditDispatchIme(pContext, "pre", 3);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pCodeEdit), "alpha    \nbeta") == 0, "ime preedit leaves text unchanged");
	pCodeCache = xuiWidgetGetCacheSurface(pCodeEdit, xuiWidgetGetStateId(pCodeEdit));
	xuiTestSurfaceReset(pCodeCache);
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetLastTextColor(pCodeCache) == iImeColor, "ime preedit renders");
	iRet = __xuiCodeEditDispatchIme(pContext, "", 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pCodeEdit), "alpha    \nbeta") == 0, "ime cancel leaves text unchanged");
	iRet = __xuiCodeEditDispatchIme(pContext, "Z", 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pCodeEdit), "alphaZ    \nbeta") == 0, "ime commit inserts text");
	iRet = xuiWidgetSetInlineStyle(pCodeEdit, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "ime inline style clear");
	iRet = xuiCodeEditSetText(pCodeEdit, "abcdef\nxyz");
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer text reset");
	iRet = __xuiCodeEditPointerDown(pContext, 20.0f + fTextOriginX + 4.0f + fPointerCharWidth * 2.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pCodeEdit && xuiGetPointerCapture(pContext) == pCodeEdit, "pointer down focuses and captures");
	iRet = xuiCodeSelectionGetState(xuiCodeEditGetSelection(pCodeEdit), &tSelection);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelection.iCaretOffset == 2, "pointer down caret");
	iRet = __xuiCodeEditPointerMove(pContext, 20.0f + fTextOriginX + 4.0f + fPointerCharWidth * 5.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer drag move");
	iRet = xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pCodeEdit), &iSelectStart, &iSelectEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectStart == 2 && iSelectEnd == 5, "pointer drag range");
	iRet = __xuiCodeEditPointerMove(pContext, -500.0f, -500.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer drag outside top left");
	iRet = xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pCodeEdit), &iSelectStart, &iSelectEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectStart == 0 && iSelectEnd == 2, "pointer drag clamps to start");
	iRet = __xuiCodeEditPointerMove(pContext, 5000.0f, 5000.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer drag outside bottom right");
	iRet = xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pCodeEdit), &iSelectStart, &iSelectEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectStart == 2 && iSelectEnd == xuiCodeDocumentGetLength(xuiCodeEditGetDocument(pCodeEdit)), "pointer drag clamps to end");
	iRet = xuiInputPointerMove(pContext, 5000.0f, 5000.0f, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "pointer drag lost button releases capture");
	pCodeCache = xuiWidgetGetCacheSurface(pCodeEdit, xuiWidgetGetStateId(pCodeEdit));
	xuiTestSurfaceReset(pCodeCache);
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit vertical scrollbar layout after drag");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	pCodeCache = xuiWidgetGetCacheSurface(pCodeEdit, xuiWidgetGetStateId(pCodeEdit));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetRectFillCount(pCodeCache) >= 2, "render selection fill");
	iRet = __xuiCodeEditPointerUp(pContext, 20.0f + fTextOriginX + 4.0f + fPointerCharWidth * 5.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "pointer up releases capture");
	iRet = __xuiCodeEditDoubleClick(pContext, 20.0f + fTextOriginX + 4.0f + fPointerCharWidth * 2.0f, 30.0f + 9.0f, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "double click word");
	iRet = xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pCodeEdit), &iSelectStart, &iSelectEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectStart == 0 && iSelectEnd == 6, "double click selects word");
	iRet = __xuiCodeEditDoubleClick(pContext, 20.0f + fTextOriginX + 4.0f + 8.0f * 1.0f, 30.0f + 18.0f + 9.0f, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK, "ctrl double click line");
	iRet = xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pCodeEdit), &iSelectStart, &iSelectEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectStart == 7 && iSelectEnd == 10, "ctrl double click selects line");
	iRet = xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pCodeEdit), xuiCodeEditGetDocument(pCodeEdit), 2, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore selection before menu");
	iRet = xuiCodeEditOpenMenu(pCodeEdit, 80.0f, 90.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "open menu api");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "menu layout");
	XUI_TEST_CHECK(xuiMenuGetItemCount(pMenu) == 15, "menu item count");
	XUI_TEST_CHECK(xuiMenuGetItem(pMenu, 2)->iType == XUI_MENU_ITEM_SEPARATOR && xuiMenuGetItem(pMenu, 9)->iType == XUI_MENU_ITEM_SEPARATOR, "menu separators");
	XUI_TEST_CHECK((xuiMenuGetItem(pMenu, 3)->iState & XUI_MENU_ITEM_ENABLED) != 0u, "cut enabled with selection");
	iRet = __xuiCodeEditClickMenuItem(pContext, pMenu, 6);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pCodeEdit), "abf\nxyz") == 0, "menu delete selection");
	iRet = xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pCodeEdit), &iSelectStart, &iSelectEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectStart == iSelectEnd, "menu delete clears selection");
	iRet = xuiCodeEditSetText(pCodeEdit, "readonly");
	XUI_TEST_CHECK(iRet == XUI_OK, "readonly menu text");
	iRet = xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pCodeEdit), xuiCodeEditGetDocument(pCodeEdit), 0, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "readonly menu selection");
	iRet = xuiCodeEditSetReadonly(pCodeEdit, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "readonly menu enable");
	iRet = xuiCodeEditOpenMenu(pCodeEdit, 82.0f, 92.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "readonly menu open");
	XUI_TEST_CHECK((xuiMenuGetItem(pMenu, 3)->iState & XUI_MENU_ITEM_ENABLED) == 0u, "readonly cut disabled");
	XUI_TEST_CHECK((xuiMenuGetItem(pMenu, 4)->iState & XUI_MENU_ITEM_ENABLED) != 0u, "readonly copy enabled");
	XUI_TEST_CHECK((xuiMenuGetItem(pMenu, 5)->iState & XUI_MENU_ITEM_ENABLED) == 0u, "readonly paste disabled");
	iRet = xuiMenuClose(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "close menu");
	iRet = xuiCodeEditSetReadonly(pCodeEdit, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "readonly menu disable");
	iRet = __xuiCodeEditRightClick(pContext, 20.0f + fTextOriginX + 12.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "right click opens menu");
	iRet = xuiMenuClose(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "close right menu before scroll");
	iRet = xuiCodeEditSetText(pCodeEdit, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\nline1\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9\nline10\nline11\nline12\nline13\nline14\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "scroll range text");
	iRet = xuiCodeEditSetScroll(pCodeEdit, 16.0f, 18.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set scroll");
	iRet = xuiCodeEditGetScroll(pCodeEdit, &fScrollX, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK && fScrollX == 16.0f && fScrollY == 18.0f, "get scroll");
	pScrollModel = xuiCodeEditGetScrollModel(pCodeEdit);
	XUI_TEST_CHECK(pScrollModel != NULL, "codeedit scroll model");
	iRet = xuiScrollModelGetOffset(pScrollModel, &fScrollX, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK && fScrollX == 16.0f && fScrollY == 18.0f, "scroll model offset");
	pHScrollBar = xuiCodeEditGetHScrollBarWidget(pCodeEdit);
	pVScrollBar = xuiCodeEditGetVScrollBarWidget(pCodeEdit);
	XUI_TEST_CHECK(pHScrollBar != NULL && pVScrollBar != NULL, "codeedit internal scrollbars");
	XUI_TEST_CHECK(xuiWidgetGetParent(pHScrollBar) == pCodeEdit && xuiWidgetGetParent(pVScrollBar) == pCodeEdit, "codeedit scrollbar parent");
	XUI_TEST_CHECK(xuiWidgetGetVisible(pHScrollBar) && xuiWidgetGetVisible(pVScrollBar), "codeedit scrollbars visible");
	XUI_TEST_CHECK(xuiScrollBarGetValue(pHScrollBar) == 16.0f && xuiScrollBarGetValue(pVScrollBar) == 18.0f, "codeedit scrollbar values");
	tScrollBarRect = xuiWidgetGetRect(pHScrollBar);
	XUI_TEST_CHECK(tScrollBarRect.fH > 0.0f && tScrollBarRect.fY > 0.0f, "codeedit horizontal scrollbar rect");
	tScrollBarRect = xuiWidgetGetRect(pVScrollBar);
	XUI_TEST_CHECK(tScrollBarRect.fW > 0.0f && tScrollBarRect.fX > 0.0f, "codeedit vertical scrollbar rect");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit scrollbar layout before input");
	tScrollBarWorld = xuiWidgetGetWorldRect(pVScrollBar);
	tScrollBarThumb = xuiScrollBarGetThumbRect(pVScrollBar);
	pHitWidget = xuiHitTest(pContext, tScrollBarWorld.fX + tScrollBarThumb.fX + tScrollBarThumb.fW * 0.5f, tScrollBarWorld.fY + tScrollBarThumb.fY + tScrollBarThumb.fH * 0.5f, XUI_WIDGET_HIT_DEFAULT);
	XUI_TEST_CHECK(pHitWidget == pVScrollBar, "codeedit vertical scrollbar thumb hit");
	iRet = __xuiCodeEditPointerDown(pContext, tScrollBarWorld.fX + tScrollBarThumb.fX + tScrollBarThumb.fW * 0.5f, tScrollBarWorld.fY + tScrollBarThumb.fY + tScrollBarThumb.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pVScrollBar, "codeedit vertical scrollbar thumb down");
	iRet = __xuiCodeEditPointerMove(pContext, tScrollBarWorld.fX + tScrollBarThumb.fX + tScrollBarThumb.fW * 0.5f, tScrollBarWorld.fY + tScrollBarThumb.fY + tScrollBarThumb.fH * 0.5f + 36.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit vertical scrollbar thumb drag");
	iRet = __xuiCodeEditPointerUp(pContext, tScrollBarWorld.fX + tScrollBarThumb.fX + tScrollBarThumb.fW * 0.5f, tScrollBarWorld.fY + tScrollBarThumb.fY + tScrollBarThumb.fH * 0.5f + 36.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "codeedit vertical scrollbar thumb up");
	iRet = xuiCodeEditGetScroll(pCodeEdit, &fScrollX, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK && fScrollY > 18.0f, "codeedit vertical scrollbar updates scroll");
	pCodeCache = xuiWidgetGetCacheSurface(pCodeEdit, xuiWidgetGetStateId(pCodeEdit));
	xuiTestSurfaceReset(pCodeCache);
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	pCodeCache = xuiWidgetGetCacheSurface(pCodeEdit, xuiWidgetGetStateId(pCodeEdit));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetTextDrawCount(pCodeCache) > 0, "codeedit vertical scrollbar keeps text visible");
	iRet = xuiCodeEditSetScroll(pCodeEdit, -5.0f, -7.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "negative scroll clamps");
	iRet = xuiCodeEditGetScroll(pCodeEdit, &fScrollX, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK && fScrollX == 0.0f && fScrollY == 0.0f, "scroll clamped");
	iRet = __xuiCodeEditWheel(pContext, 40.0f, 42.0f, 1.0f, -1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel event");
	iRet = xuiCodeEditGetScroll(pCodeEdit, &fScrollX, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK && fScrollX == 32.0f && fScrollY == 48.0f, "wheel updates scroll");
	iRet = xuiCodeEditSetText(pCodeEdit, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "caret follow text");
	iRet = xuiCodeEditSetScroll(pCodeEdit, 160.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "caret follow scroll setup");
	iRet = __xuiCodeEditPointerDown(pContext, 20.0f + fTextOriginX + 4.0f, 30.0f + 18.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "caret follow pointer down");
	iRet = __xuiCodeEditPointerUp(pContext, 20.0f + fTextOriginX + 4.0f, 30.0f + 18.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "caret follow pointer up");
	iRet = xuiCodeEditGetScroll(pCodeEdit, &fScrollX, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK && fScrollX == 0.0f, "caret follow resets horizontal scroll");
	iRet = xuiCodeEditSetText(pCodeEdit,
		"line00\nline01\nline02\nline03\nline04\nline05\nline06\nline07\nline08\nline09\n"
		"line10\nline11\nline12\nline13\nline14\nline15\nline16\nline17\nline18\nline19\n"
		"line20\nline21\nline22\nline23\nline24\nline25\nline26\nline27\nline28\nline29\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "drag auto scroll text");
	iRet = xuiCodeEditSetScroll(pCodeEdit, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag auto scroll setup");
	iRet = __xuiCodeEditPointerDown(pContext, 20.0f + fTextOriginX + 4.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag auto scroll pointer down");
	iRet = __xuiCodeEditPointerMove(pContext, 20.0f + fTextOriginX + 4.0f, 30.0f + 260.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag auto scroll pointer move");
	iRet = xuiCodeEditGetScroll(pCodeEdit, &fScrollX, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK && fScrollY > 0.0f, "drag auto scroll updates vertical scroll");
	iRet = __xuiCodeEditPointerUp(pContext, 20.0f + fTextOriginX + 4.0f, 30.0f + 260.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "drag auto scroll pointer up");
	iRet = xuiCodeEditSetDisplayOptions(pCodeEdit, XUI_CODE_EDIT_SHOW_WHITESPACE | XUI_CODE_EDIT_SHOW_EOL | XUI_CODE_EDIT_SHOW_INDENT_GUIDES);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeEditGetDisplayOptions(pCodeEdit) == (XUI_CODE_EDIT_SHOW_WHITESPACE | XUI_CODE_EDIT_SHOW_EOL | XUI_CODE_EDIT_SHOW_INDENT_GUIDES), "display options set");
	tProperty = __xuiCodeEditStyleColorProp("codeedit.whitespace.color", XUI_COLOR_RGBA(23, 98, 201, 255));
	iRet = xuiWidgetSetInlineStyle(pCodeEdit, &tProperty, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit inline style set");
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pCodeEdit, "codeedit.whitespace.color", &tProperty);
	XUI_TEST_CHECK(iRet == XUI_OK && tProperty.tValue.iColor == XUI_COLOR_RGBA(23, 98, 201, 255), "codeedit style resolves");
	iRet = xuiCodeEditSetScroll(pCodeEdit, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "display render scroll reset");
	iRet = xuiCodeEditSetText(pCodeEdit, "    a b\tc");
	XUI_TEST_CHECK(iRet == XUI_OK, "display render text");
	pCodeCache = xuiWidgetGetCacheSurface(pCodeEdit, xuiWidgetGetStateId(pCodeEdit));
	xuiTestSurfaceReset(pCodeCache);
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "display options render");
	pCodeCache = xuiWidgetGetCacheSurface(pCodeEdit, xuiWidgetGetStateId(pCodeEdit));
	iTextDrawCount = xuiTestSurfaceGetTextDrawCount(pCodeCache);
	XUI_TEST_CHECK(iTextDrawCount >= 4, "whitespace and eol markers draw");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillCount(pCodeCache) > 0, "indent guides draw");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pCodeCache) == XUI_COLOR_RGBA(23, 98, 201, 255), "display marker style color");
	iRet = xuiWidgetSetInlineStyle(pCodeEdit, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit inline style clear");
	iRet = xuiCodeEditSetDisplayOptions(pCodeEdit, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeEditGetDisplayOptions(pCodeEdit) == 0u, "display options clear");
	iRet = xuiCodeEditSetScroll(pCodeEdit, 0.0f, 18.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "scroll hit test setup");
	iRet = xuiCodeEditSetText(pCodeEdit, "line0\nline1\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9\nline10\nline11\nline12\nline13\nline14\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "scroll hit text");
	iRet = xuiCodeSelectionGotoLineColumn(xuiCodeEditGetSelection(pCodeEdit), xuiCodeEditGetDocument(pCodeEdit), 2, 5, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "ensure visible caret set");
	iRet = xuiCodeEditEnsureCaretVisible(pCodeEdit);
	XUI_TEST_CHECK(iRet == XUI_OK, "ensure visible caret");
	iRet = xuiCodeEditSetScroll(pCodeEdit, 0.0f, 18.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "scroll hit restore");
	iRet = __xuiCodeEditPointerDown(pContext, 20.0f + fTextOriginX + 4.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "scroll hit pointer down");
	iRet = xuiCodeSelectionGetState(xuiCodeEditGetSelection(pCodeEdit), &tSelection);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelection.iCaretOffset == 6, "scroll affects hit test");
	iRet = __xuiCodeEditPointerUp(pContext, 20.0f + fTextOriginX + 4.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "scroll hit pointer up");
	iRet = xuiCodeEditSetText(pCodeEdit, "\ta\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "tab visual text");
	iRet = xuiCodeEditSetScroll(pCodeEdit, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab visual scroll reset");
	iRet = xuiCodeEditSetTabColumns(pCodeEdit, 4);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeEditGetTabColumns(pCodeEdit) == 4, "tab columns set four");
	iRet = __xuiCodeEditPointerDown(pContext, 20.0f + fTextOriginX + 4.0f + 8.0f * 1.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab visual pointer four front");
	iRet = xuiCodeSelectionGetState(xuiCodeEditGetSelection(pCodeEdit), &tSelection);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelection.iCaretOffset == 0, "tab visual hit four front");
	iRet = __xuiCodeEditPointerUp(pContext, 20.0f + fTextOriginX + 4.0f + 8.0f * 1.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab visual pointer four front up");
	iRet = __xuiCodeEditPointerDown(pContext, 20.0f + fTextOriginX + 4.0f + 8.0f * 3.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab visual pointer four");
	iRet = xuiCodeSelectionGetState(xuiCodeEditGetSelection(pCodeEdit), &tSelection);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelection.iCaretOffset == 1, "tab visual hit four columns");
	iRet = __xuiCodeEditPointerUp(pContext, 20.0f + fTextOriginX + 4.0f + 8.0f * 3.0f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab visual pointer four up");
	iRet = xuiCodeEditSetTabColumns(pCodeEdit, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeEditGetTabColumns(pCodeEdit) == 2, "tab columns set two");
	iRet = __xuiCodeEditPointerDown(pContext, 20.0f + fTextOriginX + 4.0f + 8.0f * 0.25f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab visual pointer two front");
	iRet = xuiCodeSelectionGetState(xuiCodeEditGetSelection(pCodeEdit), &tSelection);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelection.iCaretOffset == 0, "tab visual hit two front");
	iRet = __xuiCodeEditPointerUp(pContext, 20.0f + fTextOriginX + 4.0f + 8.0f * 0.25f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab visual pointer two front up");
	iRet = __xuiCodeEditPointerDown(pContext, 20.0f + fTextOriginX + 4.0f + 8.0f * 1.5f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab visual pointer two");
	iRet = xuiCodeSelectionGetState(xuiCodeEditGetSelection(pCodeEdit), &tSelection);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelection.iCaretOffset == 1, "tab visual hit two columns");
	iRet = __xuiCodeEditPointerUp(pContext, 20.0f + fTextOriginX + 4.0f + 8.0f * 1.5f, 30.0f + 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab visual pointer two up");
	memset(&tStyle, 0, sizeof(tStyle));
	tStyle.iSize = sizeof(tStyle);
	tStyle.iForeground = XUI_COLOR_RGBA(201, 47, 119, 255);
	iRet = xuiCodeThemeSetStyle(xuiCodeEditGetTheme(pCodeEdit), XUI_CODE_STYLE_TYPE, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "set type style");
	iRet = xuiCodeEditSetScroll(pCodeEdit, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "token render scroll reset");
	iRet = xuiCodeEditSetText(pCodeEdit, "int");
	XUI_TEST_CHECK(iRet == XUI_OK, "token render text");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "token render");
	pCodeCache = xuiWidgetGetCacheSurface(pCodeEdit, xuiWidgetGetStateId(pCodeEdit));
	XUI_TEST_CHECK(xuiCodeTokenBufferGetCount(xuiCodeEditGetTokenBuffer(pCodeEdit)) == 1, "token buffer updated after set text");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pCodeCache) == XUI_COLOR_RGBA(201, 47, 119, 255), "token color rendered");
	iLongTextOffset = 0;
	for ( iLongLine = 0; iLongLine < 140 && iLongTextOffset < (int)sizeof(sLongText) - 5; iLongLine++ ) {
		iLongTextOffset += snprintf(sLongText + iLongTextOffset, sizeof(sLongText) - (size_t)iLongTextOffset, "int\n");
	}
	iRet = xuiCodeEditSetText(pCodeEdit, sLongText);
	XUI_TEST_CHECK(iRet == XUI_OK, "long token text set");
	iRet = xuiCodeEditSetScroll(pCodeEdit, 0.0f, 72.0f * 18.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "long token text scroll");
	xuiTestSurfaceReset(pCodeCache);
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	iTokenCount = xuiCodeTokenBufferGetCount(xuiCodeEditGetTokenBuffer(pCodeEdit));
	XUI_TEST_CHECK(iRet == XUI_OK && iTokenCount > 0 && iTokenCount < 140, "long token buffer populated visible range");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pCodeCache) == XUI_COLOR_RGBA(201, 47, 119, 255), "scrolled token color rendered");
	tProperty = __xuiCodeEditStyleColorProp("codeedit.syntax.type.color", XUI_COLOR_RGBA(34, 177, 76, 255));
	iRet = xuiWidgetSetInlineStyle(pCodeEdit, &tProperty, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "syntax inline style set");
	xuiTestSurfaceReset(pCodeCache);
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetLastTextColor(pCodeCache) == XUI_COLOR_RGBA(34, 177, 76, 255), "syntax inline style overrides token");
	iRet = xuiWidgetSetInlineStyle(pCodeEdit, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "syntax inline style clear");
	iRet = xuiCodeThemeCreate(&pOverrideTheme);
	XUI_TEST_CHECK(iRet == XUI_OK && pOverrideTheme != NULL, "override theme create");
	memset(&tStyle, 0, sizeof(tStyle));
	tStyle.iSize = sizeof(tStyle);
	tStyle.iForeground = XUI_COLOR_RGBA(12, 144, 88, 255);
	iRet = xuiCodeThemeSetStyle(pOverrideTheme, XUI_CODE_STYLE_TYPE, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "override theme style");
	iRet = xuiCodeEditSetTheme(pCodeEdit, pOverrideTheme);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit set theme");
	xuiTestSurfaceReset(pCodeCache);
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetLastTextColor(pCodeCache) == XUI_COLOR_RGBA(12, 144, 88, 255), "codeedit theme override render");
	memset(&tStyle, 0, sizeof(tStyle));
	tStyle.iSize = sizeof(tStyle);
	tStyle.iForeground = XUI_COLOR_RGBA(88, 12, 144, 255);
	iRet = xuiCodeEditSetStyle(pCodeEdit, XUI_CODE_STYLE_TYPE, &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "codeedit set style");
	xuiTestSurfaceReset(pCodeCache);
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetLastTextColor(pCodeCache) == XUI_COLOR_RGBA(88, 12, 144, 255), "codeedit style override render");

	memset(&tPanelDesc, 0, sizeof(tPanelDesc));
	tPanelDesc.iSize = sizeof(tPanelDesc);
	tPanelDesc.sTitle = "Source";
	tPanelDesc.pFont = pFont;
	tPanelDesc.fHeaderHeight = 24.0f;
	iRet = xuiPanelCreate(pContext, &pPanel, &tPanelDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pPanel != NULL, "panel create for codeedit");
	xuiWidgetSetRect(pPanel, (xui_rect_t){20.0f, 260.0f, 220.0f, 160.0f});
	iRet = xuiWidgetAddChild(pRoot, pPanel);
	XUI_TEST_CHECK(iRet == XUI_OK, "panel add to root");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.sText = "panel();\n";
	tDesc.sLanguage = "c";
	tDesc.bShowLineNumbers = 1;
	iRet = xuiCodeEditCreate(pContext, &pPanelEdit, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pPanelEdit != NULL, "panel codeedit create");
	iRet = xuiPanelAddChild(pPanel, pPanelEdit);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetParent(pPanelEdit) == xuiPanelGetClientWidget(pPanel), "panel codeedit parent");

	arrTabItems[0] = "main.c";
	arrTabItems[1] = "util.c";
	memset(&tTabsDesc, 0, sizeof(tTabsDesc));
	tTabsDesc.iSize = sizeof(tTabsDesc);
	tTabsDesc.arrItems = arrTabItems;
	tTabsDesc.iItemCount = 2;
	tTabsDesc.iSelected = 0;
	tTabsDesc.pFont = pFont;
	tTabsDesc.fTabWidth = 96.0f;
	tTabsDesc.fTabHeight = 28.0f;
	iRet = xuiTabsCreate(pContext, &pTabs, &tTabsDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTabs != NULL, "tabs create for codeedit");
	xuiWidgetSetRect(pTabs, (xui_rect_t){260.0f, 260.0f, 220.0f, 160.0f});
	iRet = xuiWidgetAddChild(pRoot, pTabs);
	XUI_TEST_CHECK(iRet == XUI_OK, "tabs add to root");
	tDesc.sText = "int a;\n";
	iRet = xuiCodeEditCreate(pContext, &pTabEditA, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTabEditA != NULL, "tab codeedit a create");
	tDesc.sText = "int b;\n";
	iRet = xuiCodeEditCreate(pContext, &pTabEditB, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTabEditB != NULL, "tab codeedit b create");
	iRet = xuiTabsAddPageChild(pTabs, 0, pTabEditA);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab add codeedit a");
	iRet = xuiTabsAddPageChild(pTabs, 1, pTabEditB);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab add codeedit b");
	iRet = xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pTabEditA), xuiCodeEditGetDocument(pTabEditA), 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab a selection");
	iRet = xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pTabEditB), xuiCodeEditGetDocument(pTabEditB), 4, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab b selection");
	iRet = xuiCodeEditSetScroll(pTabEditA, 12.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab a scroll");
	iRet = xuiCodeEditSetScroll(pTabEditB, 0.0f, 18.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab b scroll");
	XUI_TEST_CHECK(strcmp(xuiCodeEditGetText(pTabEditA), "int a;\n") == 0 && strcmp(xuiCodeEditGetText(pTabEditB), "int b;\n") == 0, "tab documents independent");
	iRet = xuiCodeEditGetScroll(pTabEditA, &fScrollX, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK && fScrollX == 12.0f && fScrollY == 0.0f, "tab a scroll independent");
	iRet = xuiCodeEditGetScroll(pTabEditB, &fScrollX, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK && fScrollX == 0.0f && fScrollY == 18.0f, "tab b scroll independent");

	memset(&tDockDesc, 0, sizeof(tDockDesc));
	tDockDesc.iSize = sizeof(tDockDesc);
	tDockDesc.pFont = pFont;
	iRet = xuiDockPanelCreate(pContext, &pDock, &tDockDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDock != NULL, "dock create for codeedit");
	xuiWidgetSetRect(pDock, (xui_rect_t){500.0f, 260.0f, 120.0f, 160.0f});
	iRet = xuiWidgetAddChild(pRoot, pDock);
	XUI_TEST_CHECK(iRet == XUI_OK, "dock add to root");
	tDesc.sText = "dock();\n";
	iRet = xuiCodeEditCreate(pContext, &pDockEdit, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDockEdit != NULL, "dock codeedit create");
	iRet = xuiDockPanelAddWindow(pDock, "dock.c", pDockEdit, &iDockWindow);
	XUI_TEST_CHECK(iRet == XUI_OK && iDockWindow >= 0, "dock codeedit window");
	iRet = xuiDockPanelDockWindow(pDock, iDockWindow, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, &iDockPane);
	XUI_TEST_CHECK(iRet == XUI_OK && iDockPane >= 0, "dock codeedit document pane");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "integration layout");
	iRet = xuiDockPanelGetWindowInfo(pDock, iDockWindow, &tDockInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tDockInfo.pClientWidget == pDockEdit && xuiWidgetGetVisible(tDockInfo.pHostWidget), "dock codeedit visible host");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.sText = "int debug_line(void) {\n\treturn 1;\n}\n";
	tDesc.sLanguage = "c";
	tDesc.bReadonly = 1;
	tDesc.bShowLineNumbers = 1;
	tDesc.bShowMarkerMargin = 1;
	tDesc.bShowDiagnosticMargin = 1;
	iRet = xuiCodeEditCreate(pContext, &pDebuggerEdit, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDebuggerEdit != NULL, "debugger codeedit create");
	xuiWidgetSetRect(pDebuggerEdit, (xui_rect_t){20.0f, 425.0f, 300.0f, 48.0f});
	iRet = xuiWidgetAddChild(pRoot, pDebuggerEdit);
	XUI_TEST_CHECK(iRet == XUI_OK, "debugger codeedit add");
	iRet = xuiCodeAnnotationSetMarker(xuiCodeEditGetAnnotations(pDebuggerEdit), 0, XUI_CODE_MARKER_BREAKPOINT, 0u, "breakpoint", 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "debugger breakpoint marker");
	iRet = xuiCodeAnnotationSetMarker(xuiCodeEditGetAnnotations(pDebuggerEdit), 1, XUI_CODE_MARKER_EXECUTION_LINE, 0u, "current", 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "debugger execution marker");
	memset(&tDiagnostic, 0, sizeof(tDiagnostic));
	tDiagnostic.iSize = sizeof(tDiagnostic);
	tDiagnostic.tRange.iStart = 4;
	tDiagnostic.tRange.iEnd = 14;
	tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_WARNING;
	tDiagnostic.sMessage = "watch value unavailable";
	iRet = xuiCodeAnnotationSetDiagnostics(xuiCodeEditGetAnnotations(pDebuggerEdit), &tDiagnostic, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "debugger diagnostic");
	iRet = xuiSetFocusWidget(pContext, pDebuggerEdit);
	XUI_TEST_CHECK(iRet == XUI_OK, "debugger focus");
	iRet = __xuiCodeEditDispatchText(pContext, '!');
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pDebuggerEdit), "int debug_line(void) {\n\treturn 1;\n}\n") == 0, "debugger readonly blocks text");
	iRet = xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pDebuggerEdit), xuiCodeEditGetDocument(pDebuggerEdit), 0, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "debugger copy selection");
	iRet = xuiCommandDispatch(pContext, pDebuggerEdit, XUI_CODE_COMMAND_COPY, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "debugger copy dispatch");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "debugger copy event");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "debugger layout");
	iRet = xuiCodeAnnotationGetMarkers(xuiCodeEditGetAnnotations(pDebuggerEdit), 0, NULL, 0, &iMarginCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iMarginCount == 1, "debugger breakpoint readable");
	XUI_TEST_CHECK(xuiCodeAnnotationGetDiagnosticCount(xuiCodeEditGetAnnotations(pDebuggerEdit)) == 1, "debugger diagnostic readable");

	iRet = xuiCodeProviderSetCommand(xuiCodeEditGetProviders(pCodeEdit), __xuiCodeEditHostCommand, &iHostCommandSeen);
	XUI_TEST_CHECK(iRet == XUI_OK, "host command provider set");
	iRet = xuiCodeProviderSetCommandEnabled(xuiCodeEditGetProviders(pCodeEdit), __xuiCodeEditHostCommandEnabled, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "host command enabled provider set");
	iRet = xuiCodeEditOpenMenu(pCodeEdit, 88.0f, 98.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "host command menu open");
	XUI_TEST_CHECK((xuiMenuGetItem(pMenu, 10)->iState & XUI_MENU_ITEM_ENABLED) != 0u, "find menu enabled internally");
	XUI_TEST_CHECK((xuiMenuGetItem(pMenu, 11)->iState & XUI_MENU_ITEM_ENABLED) != 0u, "replace menu enabled internally");
	XUI_TEST_CHECK((xuiMenuGetItem(pMenu, 12)->iState & XUI_MENU_ITEM_ENABLED) != 0u, "goto menu enabled by provider");
	iRet = __xuiCodeEditClickMenuItem(pContext, pMenu, 10);
	pFindWindow = xuiCodeEditGetFindWindow(pCodeEdit);
	XUI_TEST_CHECK(iRet == XUI_OK && iHostCommandSeen == 0 && pFindWindow != NULL && xuiWindowIsOpen(pFindWindow), "find menu opens internal window");
	iRet = xuiWindowSetOpen(pFindWindow, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "close internal find window from menu");
	iRet = xuiMenuClose(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "host command menu close");
	iRet = xuiCodeCommandMapBind(xuiCodeEditGetCommandMap(pCodeEdit), 'S', XUI_MOD_CTRL, XUI_CODE_COMMAND_USER_BASE + 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "host save command bind");
	iRet = xuiSetFocusWidget(pContext, pCodeEdit);
	XUI_TEST_CHECK(iRet == XUI_OK, "host command focus");
	iRet = __xuiCodeEditDispatchKey(pContext, 'S', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && iHostCommandSeen == 1, "host command routed through provider");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.sText = "one\ntwo\n";
	tDesc.sLanguage = "c";
	tDesc.iTabColumns = 4;
	tDesc.iIndentColumns = 4;
	iRet = xuiCodeEditCreate(pContext, &pToyEdit, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pToyEdit != NULL, "toy codeedit create");
	XUI_TEST_CHECK(strcmp(xuiCodeEditGetLanguage(pToyEdit), "c") == 0, "toy initial language");
	xuiWidgetSetRect(pToyEdit, (xui_rect_t){340.0f, 30.0f, 260.0f, 120.0f});
	iRet = xuiWidgetAddChild(pRoot, pToyEdit);
	XUI_TEST_CHECK(iRet == XUI_OK, "toy codeedit add child");
	memset(&tToyLanguage, 0, sizeof(tToyLanguage));
	tToyLanguage.iSize = sizeof(tToyLanguage);
	tToyLanguage.sId = "toy";
	tToyLanguage.sName = "Toy";
	tToyLanguage.sLineComment = "#";
	iRet = xuiCodeLanguageRegistryRegister(xuiCodeEditGetLanguageRegistry(pToyEdit), &tToyLanguage);
	XUI_TEST_CHECK(iRet == XUI_OK, "toy language register on widget");
	iRet = xuiCodeEditSetLanguage(pToyEdit, "missing");
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED && strstr(xuiCodeEditGetLastError(pToyEdit), "language") != NULL, "missing language rejected");
	iRet = xuiCodeEditSetLanguage(pToyEdit, "toy");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetLanguage(pToyEdit), "toy") == 0, "toy language set");
	iRet = xuiCodeSelectionSetRange(xuiCodeEditGetSelection(pToyEdit), xuiCodeEditGetDocument(pToyEdit), 0, xuiCodeDocumentGetLength(xuiCodeEditGetDocument(pToyEdit)));
	XUI_TEST_CHECK(iRet == XUI_OK, "toy select all");
	iRet = xuiCommandDispatch(pContext, pToyEdit, XUI_CODE_COMMAND_TOGGLE_LINE_COMMENT, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "toy toggle comment command dispatch");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeEditGetText(pToyEdit), "#one\n#two\n") == 0, "toy line comment metadata");

cleanup:
	if ( pFindScope != NULL ) xuiCodeFindScopeDestroy(pFindScope);
	if ( pTarget != NULL ) tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	xuiDestroy(pContext);
	xuiCodeThemeDestroy(pOverrideTheme);
	if ( pFont != NULL ) tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	if ( iFailed ) return 1;
	printf("xui_code_edit_test passed\n");
	return 0;
}
