#include "xui_internal.h"

#include <stdio.h>
#include <string.h>

typedef struct xui_code_edit_data_t {
	xui_code_document pDocument;
	xui_code_selection_model pSelection;
	xui_code_theme pTheme;
	xui_code_fold_state pFoldState;
	xui_code_annotation_store pAnnotations;
	xui_code_token_buffer pTokenBuffer;
	xui_code_provider_set pProviders;
	xui_code_margin_model pMargins;
	xui_code_command_map pCommandMap;
	xui_code_language_registry pLanguages;
	xui_widget pMenu;
	xui_widget pHScrollBar;
	xui_widget pVScrollBar;
	xui_font pFont;
	char sLanguage[32];
	char sError[160];
	int bReadonly;
	int bWordWrap;
	int iTabColumns;
	int iIndentColumns;
	int iEolMode;
	int iFlags;
	uint32_t iDisplayOptions;
	int bDragging;
	int iDragAnchor;
	int bImeComposing;
	int iImeAnchorOffset;
	char sImeComposition[256];
	xui_scroll_model_t tScrollModel;
	int bShowHScrollBar;
	int bShowVScrollBar;
	int bSyncingScrollBars;
	xui_rect_t tScrollViewportRect;
	xui_rect_t tHScrollBarRect;
	xui_rect_t tVScrollBarRect;
	float fScrollX;
	float fScrollY;
} xui_code_edit_data_t;

static int __xuiCodeEditExecuteCommand(xui_widget pWidget, xui_code_edit_data_t* pData, int iCommand, const void* pCommandData, int* pHandled);
static int __xuiCodeEditUpdateScrollModel(xui_widget pWidget, xui_code_edit_data_t* pData);

static float __xuiCodeEditMaxFloat(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static int __xuiCodeEditAlpha(uint32_t iColor)
{
	return (int)((iColor >> 24) & 0xffu);
}

static xui_font __xuiCodeEditResolveFont(xui_widget pWidget, xui_code_edit_data_t* pData)
{
	xui_context pContext;

	if ( pData != NULL && pData->pFont != NULL ) return pData->pFont;
	pContext = (pWidget != NULL) ? xuiWidgetGetContext(pWidget) : NULL;
	return (pContext != NULL) ? xuiGetDefaultFont(pContext) : NULL;
}

static float __xuiCodeEditLineHeight(xui_widget pWidget, xui_code_edit_data_t* pData)
{
	xui_context pContext;
	xui_proxy pProxy;
	xui_font pFont;
	xui_font_metrics_t tMetrics;

	pContext = (pWidget != NULL) ? xuiWidgetGetContext(pWidget) : NULL;
	pProxy = (pContext != NULL) ? xuiInternalContextGetProxy(pContext) : NULL;
	pFont = __xuiCodeEditResolveFont(pWidget, pData);
	if ( pProxy != NULL && pProxy->fontGetMetrics != NULL && pFont != NULL ) {
		memset(&tMetrics, 0, sizeof(tMetrics));
		if ( pProxy->fontGetMetrics(pProxy, pFont, &tMetrics) == XUI_OK && tMetrics.fLineHeight > 0.0f ) {
			return (tMetrics.fLineHeight > 18.0f) ? tMetrics.fLineHeight : 18.0f;
		}
	}
	return 18.0f;
}

static float __xuiCodeEditFontLineHeight(xui_widget pWidget, xui_code_edit_data_t* pData)
{
	xui_context pContext;
	xui_proxy pProxy;
	xui_font pFont;
	xui_font_metrics_t tMetrics;

	pContext = (pWidget != NULL) ? xuiWidgetGetContext(pWidget) : NULL;
	pProxy = (pContext != NULL) ? xuiInternalContextGetProxy(pContext) : NULL;
	pFont = __xuiCodeEditResolveFont(pWidget, pData);
	if ( pProxy != NULL && pProxy->fontGetMetrics != NULL && pFont != NULL ) {
		memset(&tMetrics, 0, sizeof(tMetrics));
		if ( pProxy->fontGetMetrics(pProxy, pFont, &tMetrics) == XUI_OK && tMetrics.fLineHeight > 0.0f ) {
			return tMetrics.fLineHeight;
		}
	}
	return 18.0f;
}

static float __xuiCodeEditLineTextOffsetY(xui_widget pWidget, xui_code_edit_data_t* pData, float fLineHeight)
{
	float fFontLineHeight;

	fFontLineHeight = __xuiCodeEditFontLineHeight(pWidget, pData);
	if ( fLineHeight > fFontLineHeight && fFontLineHeight > 0.0f ) {
		return (fLineHeight - fFontLineHeight) * 0.5f;
	}
	return 0.0f;
}

static float __xuiCodeEditCaretHeight(xui_widget pWidget, xui_code_edit_data_t* pData, float fLineHeight)
{
	xui_context pContext;
	xui_proxy pProxy;
	xui_font pFont;
	xui_font_metrics_t tMetrics;
	float fGlyphHeight;
	float fMaxHeight;

	if ( fLineHeight <= 1.0f ) return 1.0f;
	fMaxHeight = __xuiCodeEditFontLineHeight(pWidget, pData);
	if ( fMaxHeight <= 0.0f || fMaxHeight > fLineHeight ) fMaxHeight = fLineHeight;
	pContext = (pWidget != NULL) ? xuiWidgetGetContext(pWidget) : NULL;
	pProxy = (pContext != NULL) ? xuiInternalContextGetProxy(pContext) : NULL;
	pFont = __xuiCodeEditResolveFont(pWidget, pData);
	if ( pProxy != NULL && pProxy->fontGetMetrics != NULL && pFont != NULL ) {
		memset(&tMetrics, 0, sizeof(tMetrics));
		if ( pProxy->fontGetMetrics(pProxy, pFont, &tMetrics) == XUI_OK ) {
			fGlyphHeight = (tMetrics.fDescent < 0.0f) ? (tMetrics.fAscent - tMetrics.fDescent) : (tMetrics.fAscent + tMetrics.fDescent);
			if ( fGlyphHeight <= 0.0f ) fGlyphHeight = tMetrics.fAscent - tMetrics.fDescent;
			if ( fGlyphHeight <= 0.0f ) fGlyphHeight = tMetrics.fSize;
			if ( fGlyphHeight > 0.0f ) {
				if ( fGlyphHeight > fMaxHeight ) fGlyphHeight = fMaxHeight;
				return (fGlyphHeight > 1.0f) ? fGlyphHeight : 1.0f;
			}
		}
	}
	return fMaxHeight;
}

static float __xuiCodeEditColumnWidth(xui_widget pWidget, xui_code_edit_data_t* pData)
{
	xui_context pContext;
	xui_proxy pProxy;
	xui_font pFont;
	xui_vec2_t tSize;

	pContext = (pWidget != NULL) ? xuiWidgetGetContext(pWidget) : NULL;
	pProxy = (pContext != NULL) ? xuiInternalContextGetProxy(pContext) : NULL;
	pFont = __xuiCodeEditResolveFont(pWidget, pData);
	if ( pProxy != NULL && pProxy->textMeasure != NULL && pFont != NULL ) {
		memset(&tSize, 0, sizeof(tSize));
		if ( pProxy->textMeasure(pProxy, pFont, "M", &tSize) == XUI_OK && tSize.fX > 0.0f ) {
			return (tSize.fX > 8.0f) ? tSize.fX : 8.0f;
		}
	}
	return 8.0f;
}

static int __xuiCodeEditLineVisible(xui_code_edit_data_t* pData, int iLine)
{
	int bVisible;

	if ( pData == NULL || pData->pFoldState == NULL ) return 1;
	bVisible = 1;
	if ( xuiCodeFoldStateIsLineVisible(pData->pFoldState, iLine, &bVisible) != XUI_OK ) return 1;
	return bVisible;
}

static int __xuiCodeEditLineToVisibleRow(xui_code_edit_data_t* pData, int iLine)
{
	int i;
	int iRow;

	if ( pData == NULL || iLine <= 0 ) return 0;
	iRow = 0;
	for ( i = 0; i < iLine; i++ ) {
		if ( __xuiCodeEditLineVisible(pData, i) ) iRow++;
	}
	return iRow;
}

static int __xuiCodeEditVisibleLineCount(xui_code_edit_data_t* pData)
{
	int i;
	int iCount;
	int iVisible;

	if ( pData == NULL || pData->pDocument == NULL ) return 1;
	iCount = 0;
	iVisible = xuiCodeDocumentGetLineCount(pData->pDocument);
	for ( i = 0; i < iVisible; i++ ) {
		if ( __xuiCodeEditLineVisible(pData, i) ) iCount++;
	}
	return (iCount > 0) ? iCount : 1;
}

static int __xuiCodeEditTabColumns(const xui_code_edit_data_t* pData)
{
	if ( pData != NULL && pData->iTabColumns > 0 ) return pData->iTabColumns;
	return 4;
}

static int __xuiCodeEditTabAdvance(const xui_code_edit_data_t* pData, int iVisualColumn)
{
	int iTabColumns;
	int iRemainder;

	iTabColumns = __xuiCodeEditTabColumns(pData);
	iRemainder = iVisualColumn % iTabColumns;
	return (iRemainder == 0) ? iTabColumns : (iTabColumns - iRemainder);
}

static int __xuiCodeEditLineVisualColumn(const xui_code_edit_data_t* pData, const char* sText, int iLineStart, int iOffset)
{
	int i;
	int iColumn;

	if ( sText == NULL || iOffset <= iLineStart ) return 0;
	iColumn = 0;
	for ( i = iLineStart; i < iOffset; i++ ) {
		if ( sText[i] == '\t' ) {
			iColumn += __xuiCodeEditTabAdvance(pData, iColumn);
		} else {
			iColumn++;
		}
	}
	return iColumn;
}

static int __xuiCodeEditLineColumnFromVisual(const xui_code_edit_data_t* pData, const char* sText, int iLineStart, int iLineEnd, int iVisualColumn)
{
	int i;
	int iColumn;
	int iNextColumn;

	if ( sText == NULL || iVisualColumn <= 0 ) return 0;
	iColumn = 0;
	for ( i = iLineStart; i < iLineEnd; i++ ) {
		iNextColumn = iColumn + ((sText[i] == '\t') ? __xuiCodeEditTabAdvance(pData, iColumn) : 1);
		if ( iVisualColumn < iNextColumn ) {
			return i - iLineStart;
		}
		iColumn = iNextColumn;
	}
	return iLineEnd - iLineStart;
}

static int __xuiCodeEditLineColumnFromVisualFloat(const xui_code_edit_data_t* pData, const char* sText, int iLineStart, int iLineEnd, float fVisualColumn)
{
	int i;
	int iColumn;
	int iNextColumn;
	float fMidColumn;

	if ( sText == NULL || fVisualColumn <= 0.0f ) return 0;
	iColumn = 0;
	for ( i = iLineStart; i < iLineEnd; i++ ) {
		iNextColumn = iColumn + ((sText[i] == '\t') ? __xuiCodeEditTabAdvance(pData, iColumn) : 1);
		if ( fVisualColumn < (float)iNextColumn ) {
			fMidColumn = ((float)iColumn + (float)iNextColumn) * 0.5f;
			return (fVisualColumn >= fMidColumn) ? (i - iLineStart + 1) : (i - iLineStart);
		}
		iColumn = iNextColumn;
	}
	return iLineEnd - iLineStart;
}

static int __xuiCodeEditMaxLineLength(xui_code_edit_data_t* pData)
{
	int i;
	int iStart;
	int iEnd;
	int iLineCount;
	int iLength;
	int iMax;

	if ( pData == NULL || pData->pDocument == NULL ) return 0;
	iMax = 0;
	iLineCount = xuiCodeDocumentGetLineCount(pData->pDocument);
	for ( i = 0; i < iLineCount; i++ ) {
		if ( xuiCodeDocumentGetLineRange(pData->pDocument, i, &iStart, &iEnd) == XUI_OK ) {
			iLength = __xuiCodeEditLineVisualColumn(pData, xuiCodeDocumentGetText(pData->pDocument), iStart, iEnd);
			if ( iLength > iMax ) iMax = iLength;
		}
	}
	return iMax;
}

static int __xuiCodeEditVisibleRowToLine(xui_code_edit_data_t* pData, int iRow)
{
	int i;
	int iLineCount;
	int iVisibleRow;
	int iLastVisible;

	if ( pData == NULL || pData->pDocument == NULL ) return 0;
	if ( iRow < 0 ) iRow = 0;
	iLineCount = xuiCodeDocumentGetLineCount(pData->pDocument);
	iVisibleRow = 0;
	iLastVisible = 0;
	for ( i = 0; i < iLineCount; i++ ) {
		if ( !__xuiCodeEditLineVisible(pData, i) ) continue;
		iLastVisible = i;
		if ( iVisibleRow == iRow ) return i;
		iVisibleRow++;
	}
	return iLastVisible;
}

static void __xuiCodeEditSetError(xui_code_edit_data_t* pData, const char* sError)
{
	if ( pData == NULL ) return;
	if ( sError == NULL ) sError = "";
	strncpy(pData->sError, sError, sizeof(pData->sError) - 1u);
	pData->sError[sizeof(pData->sError) - 1u] = '\0';
}

static int __xuiCodeEditDescValid(const xui_code_edit_desc_t* pDesc)
{
	if ( pDesc == NULL ) return 1;
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) return 0;
	if ( pDesc->iTabColumns < 0 || pDesc->iIndentColumns < 0 ) return 0;
	return 1;
}

static xui_code_edit_data_t* __xuiCodeEditGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) return NULL;
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "codeedit");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) return NULL;
	return (xui_code_edit_data_t*)xuiWidgetGetTypeData(pWidget);
}

static void __xuiCodeEditDestroyOwned(xui_code_edit_data_t* pData)
{
	if ( pData == NULL ) return;
	if ( pData->pMenu != NULL ) {
		xui_widget pPopup = xuiMenuGetPopupWidget(pData->pMenu);
		if ( pPopup != NULL ) {
			xuiWidgetDestroy(pPopup);
		} else {
			xuiWidgetDestroy(pData->pMenu);
		}
		pData->pMenu = NULL;
	}
	if ( pData->pHScrollBar != NULL ) {
		xuiWidgetDestroy(pData->pHScrollBar);
		pData->pHScrollBar = NULL;
	}
	if ( pData->pVScrollBar != NULL ) {
		xuiWidgetDestroy(pData->pVScrollBar);
		pData->pVScrollBar = NULL;
	}
	xuiCodeCommandMapDestroy(pData->pCommandMap);
	xuiCodeMarginModelDestroy(pData->pMargins);
	xuiCodeProviderSetDestroy(pData->pProviders);
	xuiCodeTokenBufferDestroy(pData->pTokenBuffer);
	xuiCodeLanguageRegistryDestroy(pData->pLanguages);
	xuiCodeAnnotationStoreDestroy(pData->pAnnotations);
	xuiCodeFoldStateDestroy(pData->pFoldState);
	xuiCodeThemeDestroy(pData->pTheme);
	xuiCodeSelectionDestroy(pData->pSelection);
	xuiCodeDocumentDestroy(pData->pDocument);
	pData->pCommandMap = NULL;
	pData->pMargins = NULL;
	pData->pProviders = NULL;
	pData->pTokenBuffer = NULL;
	pData->pAnnotations = NULL;
	pData->pFoldState = NULL;
	pData->pTheme = NULL;
	pData->pSelection = NULL;
	pData->pDocument = NULL;
}

static int __xuiCodeEditMenuCommandForValue(int iValue)
{
	switch ( iValue ) {
	case 1: return XUI_CODE_COMMAND_UNDO;
	case 2: return XUI_CODE_COMMAND_REDO;
	case 3: return XUI_CODE_COMMAND_CUT;
	case 4: return XUI_CODE_COMMAND_COPY;
	case 5: return XUI_CODE_COMMAND_PASTE;
	case 6: return XUI_CODE_COMMAND_DELETE_FORWARD;
	case 7: return XUI_CODE_COMMAND_SELECT_ALL;
	case 8: return XUI_CODE_COMMAND_FIND_NEXT;
	case 9: return XUI_CODE_COMMAND_REPLACE_NEXT;
	case 10: return XUI_CODE_COMMAND_GOTO_LINE;
	case 11: return XUI_CODE_COMMAND_TOGGLE_LINE_COMMENT;
	case 12: return XUI_CODE_COMMAND_FOLD_TOGGLE;
	default: return 0;
	}
}

static void __xuiCodeEditMenuSelect(xui_widget pMenu, int iIndex, int iValue, void* pUser)
{
	xui_widget pWidget;
	xui_code_edit_data_t* pData;
	int iCommand;
	int bHandled;

	(void)pMenu;
	(void)iIndex;
	pWidget = (xui_widget)pUser;
	pData = __xuiCodeEditGetData(pWidget);
	iCommand = __xuiCodeEditMenuCommandForValue(iValue);
	if ( pData == NULL || iCommand <= 0 ) return;
	(void)__xuiCodeEditExecuteCommand(pWidget, pData, iCommand, NULL, &bHandled);
}

static uint32_t __xuiCodeEditProviderCommandState(xui_widget pWidget, xui_code_edit_data_t* pData, int iCommand, uint32_t iDefaultState)
{
	int bEnabled;
	int iRet;

	if ( pWidget == NULL || pData == NULL ) return 0u;
	bEnabled = 0;
	iRet = xuiCodeProviderCanExecuteCommand(pData->pProviders, pWidget, iCommand, &bEnabled);
	if ( iRet == XUI_OK ) return bEnabled ? XUI_MENU_ITEM_ENABLED : 0u;
	if ( iRet == XUI_ERROR_UNSUPPORTED ) return iDefaultState;
	return 0u;
}

static int __xuiCodeEditUpdateMenu(xui_widget pWidget, xui_code_edit_data_t* pData)
{
	xui_menu_item_t arrItems[15];
	uint32_t iEnabled;
	uint32_t iEditEnabled;
	uint32_t iSelectionEnabled;
	uint32_t iUndoEnabled;
	uint32_t iRedoEnabled;
	int iLen;
	int bHasSelection;

	if ( pWidget == NULL || pData == NULL || pData->pMenu == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(arrItems, 0, sizeof(arrItems));
	iEnabled = XUI_MENU_ITEM_ENABLED;
	iEditEnabled = pData->bReadonly ? 0u : XUI_MENU_ITEM_ENABLED;
	bHasSelection = xuiCodeSelectionHasSelection(pData->pSelection);
	iSelectionEnabled = bHasSelection ? XUI_MENU_ITEM_ENABLED : 0u;
	iUndoEnabled = xuiCodeDocumentCanUndo(pData->pDocument) ? XUI_MENU_ITEM_ENABLED : 0u;
	iRedoEnabled = xuiCodeDocumentCanRedo(pData->pDocument) ? XUI_MENU_ITEM_ENABLED : 0u;
	iLen = xuiCodeDocumentGetLength(pData->pDocument);

	arrItems[0].sText = "Undo";
	arrItems[0].sShortcut = "Ctrl+Z";
	arrItems[0].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[0].iState = iUndoEnabled;
	arrItems[0].iValue = 1;
	arrItems[1].sText = "Redo";
	arrItems[1].sShortcut = "Ctrl+Y";
	arrItems[1].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[1].iState = iRedoEnabled;
	arrItems[1].iValue = 2;
	arrItems[2].iType = XUI_MENU_ITEM_SEPARATOR;
	arrItems[3].sText = "Cut";
	arrItems[3].sShortcut = "Ctrl+X";
	arrItems[3].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[3].iState = iEditEnabled & iSelectionEnabled;
	arrItems[3].iValue = 3;
	arrItems[4].sText = "Copy";
	arrItems[4].sShortcut = "Ctrl+C";
	arrItems[4].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[4].iState = iSelectionEnabled;
	arrItems[4].iValue = 4;
	arrItems[5].sText = "Paste";
	arrItems[5].sShortcut = "Ctrl+V";
	arrItems[5].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[5].iState = iEditEnabled;
	arrItems[5].iValue = 5;
	arrItems[6].sText = "Delete";
	arrItems[6].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[6].iState = iEditEnabled & iSelectionEnabled;
	arrItems[6].iValue = 6;
	arrItems[7].iType = XUI_MENU_ITEM_SEPARATOR;
	arrItems[8].sText = "Select All";
	arrItems[8].sShortcut = "Ctrl+A";
	arrItems[8].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[8].iState = (iLen > 0 && !bHasSelection) ? iEnabled : 0u;
	arrItems[8].iValue = 7;
	arrItems[9].iType = XUI_MENU_ITEM_SEPARATOR;
	arrItems[10].sText = "Find";
	arrItems[10].sShortcut = "Ctrl+F";
	arrItems[10].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[10].iState = __xuiCodeEditProviderCommandState(pWidget, pData, XUI_CODE_COMMAND_FIND_NEXT, 0u);
	arrItems[10].iValue = 8;
	arrItems[11].sText = "Replace";
	arrItems[11].sShortcut = "Ctrl+H";
	arrItems[11].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[11].iState = __xuiCodeEditProviderCommandState(pWidget, pData, XUI_CODE_COMMAND_REPLACE_NEXT, 0u);
	arrItems[11].iValue = 9;
	arrItems[12].sText = "Go To Line";
	arrItems[12].sShortcut = "Ctrl+G";
	arrItems[12].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[12].iState = __xuiCodeEditProviderCommandState(pWidget, pData, XUI_CODE_COMMAND_GOTO_LINE, 0u);
	arrItems[12].iValue = 10;
	arrItems[13].sText = "Toggle Comment";
	arrItems[13].sShortcut = "Ctrl+/";
	arrItems[13].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[13].iState = iEditEnabled;
	arrItems[13].iValue = 11;
	arrItems[14].sText = "Toggle Fold";
	arrItems[14].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[14].iState = iEnabled;
	arrItems[14].iValue = 12;
	return xuiMenuSetItems(pData->pMenu, arrItems, 15);
}

static int __xuiCodeEditInitMenu(xui_widget pWidget, xui_code_edit_data_t* pData)
{
	xui_menu_desc_t tDesc;
	int iRet;

	if ( pWidget == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pOwner = pWidget;
	tDesc.pFont = pData->pFont;
	iRet = xuiMenuCreate(xuiWidgetGetContext(pWidget), &pData->pMenu, &tDesc);
	if ( iRet != XUI_OK ) {
		pData->pMenu = NULL;
		return iRet;
	}
	iRet = xuiMenuSetSelect(pData->pMenu, __xuiCodeEditMenuSelect, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiCodeEditUpdateMenu(pWidget, pData);
}

static void __xuiCodeEditHScrollBarChanged(xui_widget pBar, float fValue, void* pUser)
{
	xui_widget pWidget;
	xui_code_edit_data_t* pData;

	(void)pBar;
	pWidget = (xui_widget)pUser;
	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL || pData->bSyncingScrollBars ) return;
	(void)xuiCodeEditSetScroll(pWidget, fValue, pData->fScrollY);
}

static void __xuiCodeEditVScrollBarChanged(xui_widget pBar, float fValue, void* pUser)
{
	xui_widget pWidget;
	xui_code_edit_data_t* pData;

	(void)pBar;
	pWidget = (xui_widget)pUser;
	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL || pData->bSyncingScrollBars ) return;
	(void)xuiCodeEditSetScroll(pWidget, pData->fScrollX, fValue);
}

static int __xuiCodeEditInitScrollBars(xui_widget pWidget, xui_code_edit_data_t* pData)
{
	xui_context pContext;
	xui_scrollbar_desc_t tDesc;
	int iRet;

	if ( pWidget == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fMin = 0.0f;
	tDesc.fMax = 1.0f;
	tDesc.fPage = 1.0f;
	tDesc.iOrientation = XUI_ORIENTATION_HORIZONTAL;
	tDesc.iMode = XUI_SCROLLBAR_MODE_COMPACT;
	tDesc.iButtonMode = XUI_SCROLLBAR_BUTTONS_OFF;
	tDesc.fThickness = 8.0f;
	tDesc.fMinThumbSize = 18.0f;
	tDesc.fThumbRadius = 3.0f;
	tDesc.iTrackColor = XUI_COLOR_RGBA(226, 232, 240, 255);
	tDesc.iThumbColor = XUI_COLOR_RGBA(148, 163, 184, 255);
	tDesc.iHoverColor = XUI_COLOR_RGBA(100, 116, 139, 255);
	tDesc.iActiveColor = XUI_COLOR_RGBA(71, 85, 105, 255);
	tDesc.iFocusColor = XUI_COLOR_RGBA(59, 130, 246, 255);
	tDesc.iDisabledColor = XUI_COLOR_RGBA(203, 213, 225, 255);
	iRet = xuiScrollBarCreate(pContext, &pData->pHScrollBar, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pWidget, pData->pHScrollBar);
	if ( iRet != XUI_OK ) return iRet;
	tDesc.iOrientation = XUI_ORIENTATION_VERTICAL;
	iRet = xuiScrollBarCreate(pContext, &pData->pVScrollBar, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pWidget, pData->pVScrollBar);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetVisible(pData->pHScrollBar, 0);
	(void)xuiWidgetSetVisible(pData->pVScrollBar, 0);
	iRet = xuiScrollBarSetChange(pData->pHScrollBar, __xuiCodeEditHScrollBarChanged, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiScrollBarSetChange(pData->pVScrollBar, __xuiCodeEditVScrollBarChanged, pWidget);
	return iRet;
}

static int __xuiCodeEditInitOwned(xui_code_edit_data_t* pData, const xui_code_edit_desc_t* pDesc)
{
	int iRet;

	iRet = xuiCodeDocumentCreate(&pData->pDocument);
	if ( iRet == XUI_OK ) iRet = xuiCodeSelectionCreate(&pData->pSelection);
	if ( iRet == XUI_OK ) iRet = xuiCodeThemeCreate(&pData->pTheme);
	if ( iRet == XUI_OK ) iRet = xuiCodeFoldStateCreate(&pData->pFoldState);
	if ( iRet == XUI_OK ) iRet = xuiCodeAnnotationStoreCreate(&pData->pAnnotations);
	if ( iRet == XUI_OK ) iRet = xuiCodeTokenBufferCreate(&pData->pTokenBuffer);
	if ( iRet == XUI_OK ) iRet = xuiCodeProviderSetCreate(&pData->pProviders);
	if ( iRet == XUI_OK ) iRet = xuiCodeMarginModelCreate(&pData->pMargins);
	if ( iRet == XUI_OK ) iRet = xuiCodeCommandMapCreate(&pData->pCommandMap);
	if ( iRet == XUI_OK ) iRet = xuiCodeLanguageRegistryCreate(&pData->pLanguages);
	if ( iRet == XUI_OK ) iRet = xuiCodeLanguageRegistryLoadDefaults(pData->pLanguages);
	if ( iRet == XUI_OK ) {
		iRet = xuiCodeMarginModelLoadDefaults(
			pData->pMargins,
			(pDesc == NULL) ? 1 : pDesc->bShowLineNumbers,
			(pDesc == NULL) ? 1 : pDesc->bShowMarkerMargin,
			(pDesc == NULL) ? 1 : pDesc->bShowFoldMargin,
			(pDesc == NULL) ? 1 : pDesc->bShowDiagnosticMargin);
	}
	if ( iRet != XUI_OK ) {
		__xuiCodeEditDestroyOwned(pData);
		__xuiCodeEditSetError(pData, "CodeEdit owned state create failed");
		return iRet;
	}
	return XUI_OK;
}

static void __xuiCodeEditBuildIndent(xui_code_edit_data_t* pData, char* sIndent, int iCapacity)
{
	int i;
	int iColumns;

	if ( sIndent == NULL || iCapacity <= 0 ) return;
	if ( pData != NULL && (pData->iFlags & XUI_CODE_EDIT_INDENT_WITH_TABS) != 0 ) {
		sIndent[0] = '\t';
		if ( iCapacity > 1 ) sIndent[1] = '\0';
		return;
	}
	iColumns = (pData != NULL && pData->iIndentColumns > 0) ? pData->iIndentColumns : 4;
	if ( iColumns >= iCapacity ) iColumns = iCapacity - 1;
	for ( i = 0; i < iColumns; i++ ) sIndent[i] = ' ';
	sIndent[iColumns] = '\0';
}

static int __xuiCodeEditCommandContext(xui_widget pWidget, xui_code_edit_data_t* pData, xui_proxy_t* pProxy, xui_code_command_context_t* pContext)
{
	xui_code_language_t tLanguage;
	xui_rect_t tRect;
	const char* sLineComment;
	float fLineHeight;
	int iPageLines;
	int iRet;

	if ( pWidget == NULL || pData == NULL || pContext == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pContext, 0, sizeof(*pContext));
	if ( pProxy != NULL ) {
		iRet = xuiGetProxy(xuiWidgetGetContext(pWidget), pProxy);
		if ( iRet == XUI_OK ) pContext->pProxy = pProxy;
	}
	pContext->iSize = sizeof(*pContext);
	pContext->pWidget = pWidget;
	pContext->pDocument = pData->pDocument;
	pContext->pSelection = pData->pSelection;
	pContext->pFoldState = pData->pFoldState;
	pContext->pProviders = pData->pProviders;
	pContext->bReadonly = pData->bReadonly;
	pContext->iIndentColumns = pData->iIndentColumns;
	tRect = xuiWidgetGetRect(pWidget);
	fLineHeight = __xuiCodeEditLineHeight(pWidget, pData);
	iPageLines = (fLineHeight > 0.0f) ? (int)((tRect.fH - 8.0f) / fLineHeight) : 0;
	pContext->iPageLineCount = (iPageLines > 1) ? iPageLines : 1;
	sLineComment = "//";
	memset(&tLanguage, 0, sizeof(tLanguage));
	if ( pData->sLanguage[0] != '\0' &&
	     xuiCodeLanguageRegistryFind(pData->pLanguages, pData->sLanguage, &tLanguage) == XUI_OK &&
	     tLanguage.sLineComment != NULL ) {
		sLineComment = tLanguage.sLineComment;
	}
	pContext->sLineComment = sLineComment;
	return XUI_OK;
}

static int __xuiCodeEditUpdateScrollModel(xui_widget pWidget, xui_code_edit_data_t* pData)
{
	xui_rect_t tRect;
	xui_rect_t tViewport;
	float fMarginWidth;
	float fColumnWidth;
	float fLineHeight;
	float fContentWidth;
	float fContentHeight;
	float fScrollX;
	float fScrollY;
	float fMaxX;
	float fMaxY;
	float fBarSize;
	int bShowH;
	int bShowV;
	int bNextH;
	int bNextV;
	int i;
	int iRet;

	if ( pWidget == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	fMarginWidth = 0.0f;
	(void)xuiCodeMarginModelGetTotalWidth(pData->pMargins, &fMarginWidth);
	fColumnWidth = __xuiCodeEditColumnWidth(pWidget, pData);
	fLineHeight = __xuiCodeEditLineHeight(pWidget, pData);
	fContentWidth = fMarginWidth + 8.0f + (float)__xuiCodeEditMaxLineLength(pData) * fColumnWidth;
	fContentHeight = 8.0f + (float)__xuiCodeEditVisibleLineCount(pData) * fLineHeight;
	if ( fContentWidth < tRect.fW ) fContentWidth = tRect.fW;
	if ( fContentHeight < tRect.fH ) fContentHeight = tRect.fH;
	fBarSize = 8.0f;
	bShowH = 0;
	bShowV = 0;
	for ( i = 0; i < 4; i++ ) {
		tViewport = (xui_rect_t){
			0.0f,
			0.0f,
			__xuiCodeEditMaxFloat(0.0f, tRect.fW - (bShowV ? fBarSize : 0.0f)),
			__xuiCodeEditMaxFloat(0.0f, tRect.fH - (bShowH ? fBarSize : 0.0f))
		};
		bNextH = fContentWidth > (tViewport.fW + 0.01f);
		bNextV = fContentHeight > (tViewport.fH + 0.01f);
		if ( bNextH == bShowH && bNextV == bShowV ) break;
		bShowH = bNextH;
		bShowV = bNextV;
	}
	tViewport = (xui_rect_t){
		0.0f,
		0.0f,
		__xuiCodeEditMaxFloat(0.0f, tRect.fW - (bShowV ? fBarSize : 0.0f)),
		__xuiCodeEditMaxFloat(0.0f, tRect.fH - (bShowH ? fBarSize : 0.0f))
	};
	pData->bShowHScrollBar = bShowH;
	pData->bShowVScrollBar = bShowV;
	pData->tScrollViewportRect = tViewport;
	pData->tHScrollBarRect = bShowH ? (xui_rect_t){0.0f, tViewport.fH, tViewport.fW, fBarSize} : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	pData->tVScrollBarRect = bShowV ? (xui_rect_t){tViewport.fW, 0.0f, fBarSize, tViewport.fH} : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	if ( bShowH && !bShowV ) pData->tHScrollBarRect.fW = tRect.fW;
	if ( bShowV && !bShowH ) pData->tVScrollBarRect.fH = tRect.fH;
	iRet = xuiScrollModelSetViewport(&pData->tScrollModel, tViewport);
	if ( iRet == XUI_OK ) iRet = xuiScrollModelSetContentSize(&pData->tScrollModel, fContentWidth, fContentHeight);
	if ( iRet == XUI_OK ) iRet = xuiScrollModelSetOffset(&pData->tScrollModel, pData->fScrollX, pData->fScrollY);
	if ( iRet == XUI_OK ) {
		(void)xuiScrollModelGetOffset(&pData->tScrollModel, &fScrollX, &fScrollY);
		pData->fScrollX = fScrollX;
		pData->fScrollY = fScrollY;
	}
	if ( iRet == XUI_OK && pData->pHScrollBar != NULL && pData->pVScrollBar != NULL ) {
		(void)xuiScrollModelGetMaxOffset(&pData->tScrollModel, &fMaxX, &fMaxY);
		pData->bSyncingScrollBars = 1;
		iRet = xuiWidgetSetRect(pData->pHScrollBar, pData->tHScrollBarRect);
		if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pData->pVScrollBar, pData->tVScrollBarRect);
		if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pData->pHScrollBar, bShowH);
		if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pData->pVScrollBar, bShowV);
		if ( iRet == XUI_OK ) iRet = xuiScrollBarSetRange(pData->pHScrollBar, 0.0f, fMaxX, tViewport.fW);
		if ( iRet == XUI_OK ) iRet = xuiScrollBarSetRange(pData->pVScrollBar, 0.0f, fMaxY, tViewport.fH);
		if ( iRet == XUI_OK ) iRet = xuiScrollBarSetSteps(pData->pHScrollBar, fColumnWidth, tViewport.fW);
		if ( iRet == XUI_OK ) iRet = xuiScrollBarSetSteps(pData->pVScrollBar, fLineHeight, tViewport.fH);
		if ( iRet == XUI_OK ) iRet = xuiScrollBarSetValue(pData->pHScrollBar, pData->fScrollX);
		if ( iRet == XUI_OK ) iRet = xuiScrollBarSetValue(pData->pVScrollBar, pData->fScrollY);
		pData->bSyncingScrollBars = 0;
	}
	return iRet;
}

static int __xuiCodeEditExecuteCommand(xui_widget pWidget, xui_code_edit_data_t* pData, int iCommand, const void* pCommandData, int* pHandled)
{
	xui_code_command_context_t tContext;
	xui_proxy_t tProxy;
	char sIndent[16];
	int iRet;

	if ( pHandled != NULL ) *pHandled = 0;
	if ( pData == NULL || iCommand <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tProxy, 0, sizeof(tProxy));
	memset(sIndent, 0, sizeof(sIndent));
	__xuiCodeEditBuildIndent(pData, sIndent, (int)sizeof(sIndent));
	iRet = __xuiCodeEditCommandContext(pWidget, pData, &tProxy, &tContext);
	if ( iRet != XUI_OK ) return iRet;
	tContext.sIndent = sIndent;
	tContext.pCommandData = pCommandData;
	iRet = xuiCodeCommandExecute(&tContext, iCommand, pHandled);
	__xuiCodeEditSetError(pData, (iRet == XUI_OK) ? "" : "CodeEdit command failed");
	if ( iRet == XUI_OK && (pHandled == NULL || *pHandled) ) {
		(void)xuiCodeEditEnsureCaretVisible(pWidget);
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return iRet;
}

static int __xuiCodeEditInsertEventText(xui_widget pWidget, xui_code_edit_data_t* pData, const xui_event_t* pEvent)
{
	char sText[8];
	int iTextSize;
	int iRet;

	if ( pWidget == NULL || pData == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->bReadonly || ((pEvent->iModifiers & XUI_MOD_CTRL) != 0) ) return XUI_EVENT_DISPATCH_STOP;
	iTextSize = 0;
	memset(sText, 0, sizeof(sText));
	if ( pEvent->iTextSize > 0 && pEvent->sText[0] != '\0' ) {
		iTextSize = pEvent->iTextSize;
		if ( iTextSize > (int)sizeof(sText) - 1 ) iTextSize = (int)sizeof(sText) - 1;
		memcpy(sText, pEvent->sText, (size_t)iTextSize);
		sText[iTextSize] = '\0';
	} else if ( pEvent->iCodepoint > 0 && pEvent->iCodepoint < 0x80 ) {
		sText[0] = (char)pEvent->iCodepoint;
		sText[1] = '\0';
	}
	if ( sText[0] == '\0' ) return XUI_EVENT_DISPATCH_STOP;
	iRet = xuiCodeEditingInsertText(pData->pDocument, pData->pSelection, sText, pData->bReadonly);
	__xuiCodeEditSetError(pData, (iRet == XUI_OK) ? "" : "CodeEdit text insert failed");
	if ( iRet == XUI_OK ) {
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiCodeEditCommitText(xui_widget pWidget, xui_code_edit_data_t* pData, const char* sText)
{
	int iRet;
	int iEndRet;

	if ( pWidget == NULL || pData == NULL || sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->bReadonly ) return XUI_ERROR_UNSUPPORTED;
	if ( sText[0] == '\0' ) return XUI_OK;
	iRet = xuiCodeDocumentBeginEdit(pData->pDocument);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeEditingInsertText(pData->pDocument, pData->pSelection, sText, pData->bReadonly);
	iEndRet = xuiCodeDocumentEndEdit(pData->pDocument);
	if ( iRet == XUI_OK ) iRet = iEndRet;
	__xuiCodeEditSetError(pData, (iRet == XUI_OK) ? "" : "CodeEdit text insert failed");
	if ( iRet == XUI_OK ) {
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return iRet;
}

static int __xuiCodeEditImeComposition(xui_widget pWidget, xui_code_edit_data_t* pData, const xui_event_t* pEvent)
{
	xui_code_selection_t tSelection;
	int iTextSize;
	int iRet;

	if ( pWidget == NULL || pData == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->bReadonly ) return XUI_EVENT_DISPATCH_STOP;
	iTextSize = pEvent->iTextSize;
	if ( iTextSize < 0 ) iTextSize = (int)strlen(pEvent->sText);
	if ( iTextSize <= 0 || pEvent->sText[0] == '\0' ) {
		pData->bImeComposing = 0;
		pData->sImeComposition[0] = '\0';
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iCompositionLength > 0 ) {
		if ( iTextSize >= (int)sizeof(pData->sImeComposition) ) iTextSize = (int)sizeof(pData->sImeComposition) - 1;
		memcpy(pData->sImeComposition, pEvent->sText, (size_t)iTextSize);
		pData->sImeComposition[iTextSize] = '\0';
		memset(&tSelection, 0, sizeof(tSelection));
		if ( xuiCodeSelectionGetState(pData->pSelection, &tSelection) == XUI_OK ) {
			pData->iImeAnchorOffset = tSelection.iCaretOffset;
		}
		pData->bImeComposing = 1;
		__xuiCodeEditSetError(pData, "");
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return XUI_EVENT_DISPATCH_STOP;
	}
	pData->bImeComposing = 0;
	pData->sImeComposition[0] = '\0';
	iRet = __xuiCodeEditCommitText(pWidget, pData, pEvent->sText);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_EVENT_DISPATCH_STOP;
}

static xui_rect_t __xuiCodeEditImeCandidateRect(xui_widget pWidget, void* pUser)
{
	xui_code_edit_data_t* pData;
	xui_code_selection_t tSelection;
	xui_rect_t tWorld;
	float fMarginWidth;
	float fColumnWidth;
	float fLineHeight;
	const char* sText;
	int iStart;
	int iEnd;
	int iLine;
	int iColumn;
	int iVisualColumn;

	(void)pUser;
	tWorld = xuiWidgetGetWorldRect(pWidget);
	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL || pData->pDocument == NULL || pData->pSelection == NULL ) {
		return tWorld;
	}
	memset(&tSelection, 0, sizeof(tSelection));
	if ( xuiCodeSelectionGetState(pData->pSelection, &tSelection) != XUI_OK ) {
		return tWorld;
	}
	iLine = 0;
	iColumn = 0;
	(void)xuiCodeDocumentOffsetToLineColumn(pData->pDocument, pData->bImeComposing ? pData->iImeAnchorOffset : tSelection.iCaretOffset, &iLine, &iColumn);
	sText = xuiCodeDocumentGetText(pData->pDocument);
	iStart = 0;
	iEnd = 0;
	(void)xuiCodeDocumentGetLineRange(pData->pDocument, iLine, &iStart, &iEnd);
	iVisualColumn = __xuiCodeEditLineVisualColumn(pData, sText, iStart, iStart + iColumn);
	fMarginWidth = 0.0f;
	(void)xuiCodeMarginModelGetTotalWidth(pData->pMargins, &fMarginWidth);
	fColumnWidth = __xuiCodeEditColumnWidth(pWidget, pData);
	fLineHeight = __xuiCodeEditLineHeight(pWidget, pData);
	return (xui_rect_t){
		tWorld.fX + fMarginWidth + 4.0f + (float)iVisualColumn * fColumnWidth - pData->fScrollX,
		tWorld.fY + 4.0f + (float)__xuiCodeEditLineToVisibleRow(pData, iLine) * fLineHeight - pData->fScrollY,
		2.0f,
		fLineHeight
	};
}

static int __xuiCodeEditHitOffset(xui_widget pWidget, xui_code_edit_data_t* pData, float fX, float fY, int* pOffset)
{
	xui_rect_t tWorld;
	float fMarginWidth;
	float fColumnWidth;
	float fLineHeight;
	const char* sText;
	int iStart;
	int iEnd;
	float fVisualColumn;
	int iLine;
	int iColumn;
	int iLineCount;

	if ( pWidget == NULL || pData == NULL || pData->pDocument == NULL || pOffset == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tWorld = xuiWidgetGetWorldRect(pWidget);
	(void)__xuiCodeEditUpdateScrollModel(pWidget, pData);
	if ( (fX < tWorld.fX) || (fY < tWorld.fY) ||
	     (fX >= tWorld.fX + pData->tScrollViewportRect.fW) ||
	     (fY >= tWorld.fY + pData->tScrollViewportRect.fH) ) {
		return XUI_ERROR_UNSUPPORTED;
	}
	fMarginWidth = 0.0f;
	(void)xuiCodeMarginModelGetTotalWidth(pData->pMargins, &fMarginWidth);
	fColumnWidth = __xuiCodeEditColumnWidth(pWidget, pData);
	fLineHeight = __xuiCodeEditLineHeight(pWidget, pData);
	iLine = __xuiCodeEditVisibleRowToLine(pData, (int)((fY - tWorld.fY - 4.0f + pData->fScrollY) / fLineHeight));
	fVisualColumn = (fX - tWorld.fX - fMarginWidth - 4.0f + pData->fScrollX) / fColumnWidth;
	if ( iLine < 0 ) iLine = 0;
	if ( fVisualColumn < 0.0f ) fVisualColumn = 0.0f;
	iLineCount = xuiCodeDocumentGetLineCount(pData->pDocument);
	if ( iLineCount <= 0 ) {
		*pOffset = 0;
		return XUI_OK;
	}
	if ( iLine >= iLineCount ) iLine = iLineCount - 1;
	sText = xuiCodeDocumentGetText(pData->pDocument);
	iStart = 0;
	iEnd = 0;
	if ( xuiCodeDocumentGetLineRange(pData->pDocument, iLine, &iStart, &iEnd) != XUI_OK ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iColumn = __xuiCodeEditLineColumnFromVisualFloat(pData, sText, iStart, iEnd, fVisualColumn);
	return xuiCodeDocumentLineColumnToOffset(pData->pDocument, iLine, iColumn, pOffset);
}

static int __xuiCodeEditPointerSelect(xui_widget pWidget, xui_code_edit_data_t* pData, const xui_event_t* pEvent, int bExtend)
{
	int iOffset;
	int iRet;

	if ( pWidget == NULL || pData == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiCodeEditHitOffset(pWidget, pData, pEvent->fX, pEvent->fY, &iOffset);
	if ( iRet != XUI_OK ) return iRet;
	if ( !bExtend ) pData->iDragAnchor = iOffset;
	iRet = xuiCodeSelectionSetRange(pData->pSelection, pData->pDocument, pData->iDragAnchor, iOffset);
	if ( iRet == XUI_OK ) {
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return iRet;
}

static int __xuiCodeEditPointerSelectWordOrLine(xui_widget pWidget, xui_code_edit_data_t* pData, const xui_event_t* pEvent)
{
	int iOffset;
	int iLine;
	int iRet;

	if ( pWidget == NULL || pData == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiCodeEditHitOffset(pWidget, pData, pEvent->fX, pEvent->fY, &iOffset);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pEvent->iModifiers & XUI_MOD_CTRL) != 0u ) {
		iRet = xuiCodeDocumentOffsetToLineColumn(pData->pDocument, iOffset, &iLine, NULL);
		if ( iRet == XUI_OK ) iRet = xuiCodeSelectionSelectLine(pData->pSelection, pData->pDocument, iLine, 1);
	} else {
		iRet = xuiCodeSelectionSelectWord(pData->pSelection, pData->pDocument, iOffset);
	}
	if ( iRet == XUI_OK ) {
		pData->bDragging = 0;
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return iRet;
}

static int __xuiCodeEditDispatchMarginEvent(xui_widget pWidget, xui_code_edit_data_t* pData, const xui_event_t* pEvent, int* pDispatched)
{
	xui_code_margin_hit_t tHit;
	xui_code_margin_info_t tInfo;
	xui_rect_t tViewport;
	xui_rect_t tLineRect;
	float fLineHeight;
	int iLineCount;
	int iRet;

	if ( pDispatched != NULL ) *pDispatched = 0;
	if ( pWidget == NULL || pData == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	fLineHeight = __xuiCodeEditLineHeight(pWidget, pData);
	tViewport = xuiWidgetGetWorldRect(pWidget);
	(void)__xuiCodeEditUpdateScrollModel(pWidget, pData);
	tViewport.fW = pData->tScrollViewportRect.fW;
	tViewport.fH = pData->tScrollViewportRect.fH;
	iRet = xuiCodeMarginModelHitTest(pData->pMargins, tViewport, pEvent->fX, pEvent->fY, fLineHeight, pData->fScrollY, &tHit);
	if ( iRet == XUI_ERROR_UNSUPPORTED ) return XUI_OK;
	if ( iRet != XUI_OK ) return iRet;
	tHit.iLine = __xuiCodeEditVisibleRowToLine(pData, tHit.iLine);
	iLineCount = xuiCodeDocumentGetLineCount(pData->pDocument);
	if ( tHit.iLine < 0 || tHit.iLine >= iLineCount ) return XUI_OK;
	memset(&tInfo, 0, sizeof(tInfo));
	iRet = xuiCodeMarginModelGet(pData->pMargins, tHit.iIndex, &tInfo);
	if ( iRet != XUI_OK ) return iRet;
	tLineRect = tHit.tRect;
	tLineRect.fY = tViewport.fY + 4.0f + (float)tHit.iLine * fLineHeight - pData->fScrollY;
	tLineRect.fH = fLineHeight;
	if ( tInfo.onEvent != NULL ) {
		iRet = tInfo.onEvent(pWidget, tInfo.iId, tHit.iLine, pEvent->iType, tLineRect, tInfo.pUser);
		if ( iRet != XUI_OK ) return iRet;
	} else if ( pEvent->iType == XUI_EVENT_POINTER_DOWN && tInfo.iKind == XUI_CODE_MARGIN_FOLD ) {
		iRet = xuiCodeFoldStateToggleLine(pData->pFoldState, tHit.iLine);
		if ( iRet != XUI_OK && iRet != XUI_ERROR_UNSUPPORTED ) return iRet;
		if ( iRet == XUI_OK ) {
			(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
	}
	if ( pDispatched != NULL ) *pDispatched = 1;
	return XUI_OK;
}

static int __xuiCodeEditEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_code_edit_data_t* pData;
	int iCommand;
	int bHandled;
	int bMarginEvent;
	int iRet;

	(void)pUser;
	if ( pWidget == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	switch ( pEvent->iType ) {
	case XUI_EVENT_FOCUS:
	case XUI_EVENT_BLUR:
		(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_STYLE | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return XUI_OK;
	case XUI_EVENT_POINTER_DOWN:
		if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
		(void)xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
		iRet = __xuiCodeEditDispatchMarginEvent(pWidget, pData, pEvent, &bMarginEvent);
		if ( iRet != XUI_OK ) return iRet;
		if ( bMarginEvent ) return XUI_EVENT_DISPATCH_STOP;
		(void)xuiSetPointerCapture(xuiWidgetGetContext(pWidget), pWidget);
		pData->bDragging = 1;
		iRet = __xuiCodeEditPointerSelect(pWidget, pData, pEvent, ((pEvent->iModifiers & XUI_MOD_SHIFT) != 0));
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_MOVE:
		if ( !pData->bDragging || ((pEvent->iButtons & XUI_POINTER_BUTTON_LEFT) == 0) ) return XUI_OK;
		iRet = __xuiCodeEditPointerSelect(pWidget, pData, pEvent, 1);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_UP:
		if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
		iRet = __xuiCodeEditDispatchMarginEvent(pWidget, pData, pEvent, &bMarginEvent);
		if ( iRet != XUI_OK ) return iRet;
		pData->bDragging = 0;
		(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_WHEEL:
		iRet = xuiCodeEditSetScroll(pWidget, pData->fScrollX + pEvent->fWheelX * 32.0f, pData->fScrollY - pEvent->fWheelY * 48.0f);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		pData->bDragging = 0;
		return XUI_OK;
	case XUI_EVENT_POINTER_DOUBLE_CLICK:
		if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
		(void)xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
		iRet = __xuiCodeEditPointerSelectWordOrLine(pWidget, pData, pEvent);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_CONTEXT_MENU:
		(void)xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
		if ( pEvent->iKey != XUI_KEY_CONTEXT_MENU && !xuiCodeSelectionHasSelection(pData->pSelection) ) {
			(void)__xuiCodeEditPointerSelect(pWidget, pData, pEvent, 0);
		}
		(void)xuiCodeEditOpenMenu(pWidget, pEvent->fX, pEvent->fY);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_KEY_DOWN:
		iRet = xuiCodeCommandMapFind(pData->pCommandMap, pEvent->iKey, pEvent->iModifiers, &iCommand);
		if ( iRet != XUI_OK && pEvent->iKey >= 'a' && pEvent->iKey <= 'z' ) {
			iRet = xuiCodeCommandMapFind(pData->pCommandMap, pEvent->iKey - 'a' + 'A', pEvent->iModifiers, &iCommand);
		}
		if ( iRet != XUI_OK ) return XUI_OK;
		iRet = __xuiCodeEditExecuteCommand(pWidget, pData, iCommand, NULL, &bHandled);
		if ( iRet == XUI_OK || iRet == XUI_ERROR_UNSUPPORTED ) return XUI_EVENT_DISPATCH_STOP;
		return iRet;
	case XUI_EVENT_TEXT:
		return __xuiCodeEditInsertEventText(pWidget, pData, pEvent);
	case XUI_EVENT_IME_COMPOSITION:
		return __xuiCodeEditImeComposition(pWidget, pData, pEvent);
	case XUI_EVENT_COMMAND:
		iRet = __xuiCodeEditExecuteCommand(pWidget, pData, pEvent->iCommand, pEvent->pData, &bHandled);
		if ( iRet == XUI_OK || iRet == XUI_ERROR_UNSUPPORTED ) return bHandled ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
		return iRet;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiCodeEditInitEvents(xui_widget pWidget)
{
	int iRet;

	if ( pWidget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiCodeEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiCodeEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiCodeEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiCodeEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_UP, __xuiCodeEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_WHEEL, __xuiCodeEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiCodeEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOUBLE_CLICK, __xuiCodeEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_CONTEXT_MENU, __xuiCodeEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiCodeEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_TEXT, __xuiCodeEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_IME_COMPOSITION, __xuiCodeEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_COMMAND, __xuiCodeEditEvent, NULL);
	return iRet;
}

static int __xuiCodeEditInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_code_edit_data_t* pData;
	const xui_code_edit_desc_t* pDesc;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pTypeData == NULL) || !__xuiCodeEditDescValid((const xui_code_edit_desc_t*)pCreateData) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = (xui_code_edit_data_t*)pTypeData;
	pDesc = (const xui_code_edit_desc_t*)pCreateData;
	memset(pData, 0, sizeof(*pData));
	pData->pFont = (pDesc != NULL) ? pDesc->pFont : NULL;
	pData->bReadonly = (pDesc != NULL && pDesc->bReadonly) ? 1 : 0;
	pData->bWordWrap = (pDesc != NULL && pDesc->bWordWrap) ? 1 : 0;
	pData->iTabColumns = (pDesc != NULL && pDesc->iTabColumns > 0) ? pDesc->iTabColumns : 4;
	pData->iIndentColumns = (pDesc != NULL && pDesc->iIndentColumns > 0) ? pDesc->iIndentColumns : pData->iTabColumns;
	pData->iEolMode = (pDesc != NULL) ? pDesc->iEolMode : XUI_CODE_EOL_LF;
	pData->iFlags = (pDesc != NULL) ? pDesc->iFlags : 0;
	pData->iDisplayOptions = (uint32_t)pData->iFlags & (XUI_CODE_EDIT_SHOW_WHITESPACE | XUI_CODE_EDIT_SHOW_EOL | XUI_CODE_EDIT_SHOW_INDENT_GUIDES);
	xuiScrollModelInit(&pData->tScrollModel);
	if ( pDesc != NULL && pDesc->sLanguage != NULL ) {
		strncpy(pData->sLanguage, pDesc->sLanguage, sizeof(pData->sLanguage) - 1u);
	}
	iRet = __xuiCodeEditInitOwned(pData, pDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeDocumentSetText(pData->pDocument, (pDesc != NULL) ? pDesc->sText : NULL);
	if ( iRet != XUI_OK ) {
		__xuiCodeEditSetError(pData, xuiCodeDocumentGetLastError(pData->pDocument));
		__xuiCodeEditDestroyOwned(pData);
		return iRet;
	}
	(void)xuiWidgetSetFocusable(pWidget, 1);
	(void)xuiWidgetSetImeMode(pWidget, XUI_IME_ENABLED);
	(void)xuiWidgetSetImeCandidateRect(pWidget, __xuiCodeEditImeCandidateRect, NULL);
	iRet = __xuiCodeEditInitEvents(pWidget);
	if ( iRet != XUI_OK ) {
		__xuiCodeEditSetError(pData, "CodeEdit event wiring failed");
		__xuiCodeEditDestroyOwned(pData);
		return iRet;
	}
	iRet = __xuiCodeEditInitScrollBars(pWidget, pData);
	if ( iRet != XUI_OK ) {
		__xuiCodeEditSetError(pData, "CodeEdit scrollbar create failed");
		__xuiCodeEditDestroyOwned(pData);
		return iRet;
	}
	iRet = __xuiCodeEditInitMenu(pWidget, pData);
	if ( iRet != XUI_OK ) {
		__xuiCodeEditSetError(pData, "CodeEdit menu create failed");
		__xuiCodeEditDestroyOwned(pData);
		return iRet;
	}
	return XUI_OK;
}

static void __xuiCodeEditDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_code_edit_data_t* pData;

	(void)pWidget;
	(void)pUser;
	pData = (xui_code_edit_data_t*)pTypeData;
	__xuiCodeEditDestroyOwned(pData);
	if ( pData != NULL ) memset(pData, 0, sizeof(*pData));
}

static int __xuiCodeEditMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	(void)pWidget;
	(void)tConstraint;
	(void)pUser;
	if ( pSize == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pSize = (xui_vec2_t){320.0f, 220.0f};
	return XUI_OK;
}

static void __xuiCodeEditDefaultLayout(xui_layout_t* pLayout)
{
	if ( pLayout == NULL ) return;
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_MANUAL;
	pLayout->iWidthMode = XUI_SIZE_CONTENT;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_HIDDEN;
}

static void __xuiCodeEditDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	if ( pPolicy == NULL ) return;
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiCodeEditDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( __xuiCodeEditAlpha(iColor) == 0 ) return XUI_OK;
	return (pProxy != NULL && pProxy->drawRectFill != NULL) ? pProxy->drawRectFill(pProxy, pDraw, xuiInternalSnapRect(tRect), iColor) : XUI_OK;
}

static int __xuiCodeEditRenderLineText(xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, const char* sText, int iStart, int iEnd, xui_rect_t tRect, uint32_t iColor)
{
	char sSmall[512];
	char* sLine;
	int iLength;
	int i;
	int iRet;

	if ( pProxy == NULL || pProxy->drawText == NULL || pFont == NULL || sText == NULL ) return XUI_OK;
	if ( iEnd < iStart ) iEnd = iStart;
	iLength = iEnd - iStart;
	if ( iLength <= 0 ) return XUI_OK;
	if ( iLength >= (int)sizeof(sSmall) ) iLength = (int)sizeof(sSmall) - 1;
	sLine = sSmall;
	for ( i = 0; i < iLength; i++ ) {
		unsigned char ch = (unsigned char)sText[iStart + i];
		sLine[i] = (ch < 32u) ? ' ' : (char)ch;
	}
	sLine[iLength] = '\0';
	iRet = pProxy->drawText(pProxy, pDraw, pFont, sLine, xuiInternalSnapRect(tRect), iColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
	return iRet;
}

static int __xuiCodeEditRenderLineTextVisual(xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, const xui_code_edit_data_t* pData, const char* sText, int iLineStart, int iStart, int iEnd, float fTextX, float fY, float fColumnWidth, float fLineHeight, uint32_t iColor)
{
	xui_rect_t tRect;
	int i;
	int iSpanStart;
	int iVisualStart;
	int iVisualEnd;
	int iRet;

	if ( iEnd <= iStart ) return XUI_OK;
	i = iStart;
	while ( i < iEnd ) {
		if ( sText[i] == '\t' ) {
			i++;
			continue;
		}
		iSpanStart = i;
		while ( i < iEnd && sText[i] != '\t' ) i++;
		iVisualStart = __xuiCodeEditLineVisualColumn(pData, sText, iLineStart, iSpanStart);
		iVisualEnd = __xuiCodeEditLineVisualColumn(pData, sText, iLineStart, i);
		tRect = (xui_rect_t){
			fTextX + 4.0f + (float)iVisualStart * fColumnWidth - pData->fScrollX,
			fY,
			(float)(iVisualEnd - iVisualStart) * fColumnWidth,
			fLineHeight
		};
		iRet = __xuiCodeEditRenderLineText(pProxy, pDraw, pFont, sText, iSpanStart, i, tRect, iColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiCodeEditStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
{
	xui_style_property_t tProperty;
	int iRet;

	if ( pColor == NULL ) return 0;
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( iRet == XUI_OK && tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR ) {
		*pColor = tProperty.tValue.iColor;
		return 1;
	}
	return 0;
}

static uint32_t __xuiCodeEditColor(xui_widget pWidget, const char* sName, uint32_t iDefaultColor)
{
	uint32_t iColor;

	iColor = iDefaultColor;
	(void)__xuiCodeEditStyleColor(pWidget, sName, &iColor);
	return iColor;
}

static const char* __xuiCodeEditSyntaxColorProperty(int iTokenKind)
{
	switch ( iTokenKind ) {
	case XUI_CODE_TOKEN_KEYWORD: return "codeedit.syntax.keyword.color";
	case XUI_CODE_TOKEN_TYPE: return "codeedit.syntax.type.color";
	case XUI_CODE_TOKEN_NUMBER: return "codeedit.syntax.number.color";
	case XUI_CODE_TOKEN_STRING: return "codeedit.syntax.string.color";
	case XUI_CODE_TOKEN_CHAR: return "codeedit.syntax.char.color";
	case XUI_CODE_TOKEN_COMMENT: return "codeedit.syntax.comment.color";
	case XUI_CODE_TOKEN_PREPROCESSOR: return "codeedit.syntax.preprocessor.color";
	case XUI_CODE_TOKEN_OPERATOR: return "codeedit.syntax.operator.color";
	case XUI_CODE_TOKEN_BRACE: return "codeedit.syntax.brace.color";
	case XUI_CODE_TOKEN_ERROR: return "codeedit.syntax.error.color";
	default: break;
	}
	return NULL;
}

static int __xuiCodeEditEnsureTokens(xui_code_edit_data_t* pData)
{
	xui_code_token_t* pTokens;
	const char* sText;
	uint32_t iVersion;
	uint32_t iBufferVersion;
	int iCount;
	int iRet;

	if ( pData == NULL || pData->pDocument == NULL || pData->pTokenBuffer == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iVersion = xuiCodeDocumentGetVersion(pData->pDocument);
	if ( xuiCodeTokenBufferGetVersion(pData->pTokenBuffer, &iBufferVersion) == XUI_OK && iBufferVersion == iVersion ) {
		return XUI_OK;
	}
	sText = xuiCodeDocumentGetText(pData->pDocument);
	iCount = 0;
	iRet = xuiCodeLexerCTokenize(sText, xuiCodeDocumentGetLength(pData->pDocument), NULL, 0, &iCount);
	if ( iRet != XUI_OK ) return iRet;
	if ( iCount <= 0 ) return xuiCodeTokenBufferSet(pData->pTokenBuffer, NULL, 0, iVersion);
	pTokens = (xui_code_token_t*)xrtMalloc(sizeof(*pTokens) * (size_t)iCount);
	if ( pTokens == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	iRet = xuiCodeLexerCTokenize(sText, xuiCodeDocumentGetLength(pData->pDocument), pTokens, iCount, &iCount);
	if ( iRet == XUI_OK ) iRet = xuiCodeTokenBufferSet(pData->pTokenBuffer, pTokens, iCount, iVersion);
	xrtFree(pTokens);
	return iRet;
}

static int __xuiCodeEditRenderTokenSpan(xui_widget pWidget, xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, xui_code_edit_data_t* pData, const char* sText, int iLineStart, int iLineEnd, xui_code_token_t tToken, float fTextX, float fY, float fColumnWidth, float fLineHeight)
{
	xui_code_style_t tStyle;
	const char* sProperty;
	xui_rect_t tRect;
	uint32_t iColor;
	int iStart;
	int iEnd;
	int iRet;

	if ( tToken.iEndOffset <= iLineStart || tToken.iStartOffset >= iLineEnd ) return XUI_OK;
	iStart = (tToken.iStartOffset > iLineStart) ? tToken.iStartOffset : iLineStart;
	iEnd = (tToken.iEndOffset < iLineEnd) ? tToken.iEndOffset : iLineEnd;
	if ( iEnd <= iStart ) return XUI_OK;
	memset(&tStyle, 0, sizeof(tStyle));
	iRet = xuiCodeThemeGetTokenStyle(pData->pTheme, tToken.iKind, &tStyle);
	if ( iRet != XUI_OK || __xuiCodeEditAlpha(tStyle.iForeground) == 0 ) return XUI_OK;
	sProperty = __xuiCodeEditSyntaxColorProperty(tToken.iKind);
	iColor = (sProperty != NULL) ? __xuiCodeEditColor(pWidget, sProperty, tStyle.iForeground) : tStyle.iForeground;
	tRect = (xui_rect_t){
		fTextX + 4.0f + (float)__xuiCodeEditLineVisualColumn(pData, sText, iLineStart, iStart) * fColumnWidth - pData->fScrollX,
		fY,
		(float)(__xuiCodeEditLineVisualColumn(pData, sText, iLineStart, iEnd) - __xuiCodeEditLineVisualColumn(pData, sText, iLineStart, iStart)) * fColumnWidth,
		fLineHeight
	};
	(void)tRect;
	return __xuiCodeEditRenderLineTextVisual(pProxy, pDraw, pFont, pData, sText, iLineStart, iStart, iEnd, fTextX, fY, fColumnWidth, fLineHeight, iColor);
}

static int __xuiCodeEditRenderStyledLine(xui_widget pWidget, xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, xui_code_edit_data_t* pData, const char* sText, int iLineStart, int iLineEnd, const xui_code_token_t* pTokens, int iTokenCount, float fTextX, float fY, float fColumnWidth, float fLineHeight, uint32_t iTextColor)
{
	xui_rect_t tRect;
	int iCursor;
	int iToken;
	int iStart;
	int iEnd;
	int iRet;

	if ( iLineEnd <= iLineStart ) return XUI_OK;
	iCursor = iLineStart;
	for ( iToken = 0; iToken < iTokenCount; iToken++ ) {
		if ( pTokens[iToken].iEndOffset <= iLineStart ) continue;
		if ( pTokens[iToken].iStartOffset >= iLineEnd ) break;
		iStart = (pTokens[iToken].iStartOffset > iLineStart) ? pTokens[iToken].iStartOffset : iLineStart;
		iEnd = (pTokens[iToken].iEndOffset < iLineEnd) ? pTokens[iToken].iEndOffset : iLineEnd;
		if ( iStart > iCursor ) {
			tRect = (xui_rect_t){
				fTextX + 4.0f + (float)__xuiCodeEditLineVisualColumn(pData, sText, iLineStart, iCursor) * fColumnWidth - pData->fScrollX,
				fY,
				(float)(__xuiCodeEditLineVisualColumn(pData, sText, iLineStart, iStart) - __xuiCodeEditLineVisualColumn(pData, sText, iLineStart, iCursor)) * fColumnWidth,
				fLineHeight
			};
			(void)tRect;
			iRet = __xuiCodeEditRenderLineTextVisual(pProxy, pDraw, pFont, pData, sText, iLineStart, iCursor, iStart, fTextX, fY, fColumnWidth, fLineHeight, iTextColor);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( iEnd > iStart ) {
			iRet = __xuiCodeEditRenderTokenSpan(pWidget, pProxy, pDraw, pFont, pData, sText, iLineStart, iLineEnd, pTokens[iToken], fTextX, fY, fColumnWidth, fLineHeight);
			if ( iRet != XUI_OK ) return iRet;
			if ( iEnd > iCursor ) iCursor = iEnd;
		}
	}
	if ( iCursor < iLineEnd ) {
		tRect = (xui_rect_t){
			fTextX + 4.0f + (float)__xuiCodeEditLineVisualColumn(pData, sText, iLineStart, iCursor) * fColumnWidth - pData->fScrollX,
			fY,
			(float)(__xuiCodeEditLineVisualColumn(pData, sText, iLineStart, iLineEnd) - __xuiCodeEditLineVisualColumn(pData, sText, iLineStart, iCursor)) * fColumnWidth,
			fLineHeight
		};
		(void)tRect;
		iRet = __xuiCodeEditRenderLineTextVisual(pProxy, pDraw, pFont, pData, sText, iLineStart, iCursor, iLineEnd, fTextX, fY, fColumnWidth, fLineHeight, iTextColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiCodeEditRenderWhitespace(xui_widget pWidget, xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, xui_code_edit_data_t* pData, const char* sText, int iLineStart, int iLineEnd, float fTextX, float fY, float fColumnWidth, float fLineHeight)
{
	uint32_t iMarkerColor;
	xui_rect_t tRect;
	const char* sMark;
	int i;
	int iIndentColumns;
	int iRet;

	if ( pData == NULL || pData->iDisplayOptions == 0u ) return XUI_OK;
	if ( sText == NULL ) return XUI_OK;
	iMarkerColor = __xuiCodeEditColor(pWidget, "codeedit.whitespace.color", XUI_COLOR_RGBA(148, 163, 184, 255));
	if ( (pData->iDisplayOptions & XUI_CODE_EDIT_SHOW_INDENT_GUIDES) != 0u ) {
		iIndentColumns = 0;
		for ( i = iLineStart; i < iLineEnd; i++ ) {
			if ( sText[i] == ' ' ) iIndentColumns++;
			else if ( sText[i] == '\t' ) iIndentColumns += __xuiCodeEditTabAdvance(pData, iIndentColumns);
			else break;
			if ( iIndentColumns > 0 && (iIndentColumns % ((pData->iIndentColumns > 0) ? pData->iIndentColumns : 4)) == 0 ) {
				tRect = (xui_rect_t){
					fTextX + 4.0f + (float)iIndentColumns * fColumnWidth - pData->fScrollX,
					fY + 2.0f,
					1.0f,
					fLineHeight - 4.0f
				};
				iRet = __xuiCodeEditDrawRectFill(pProxy, pDraw, tRect, iMarkerColor);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	for ( i = iLineStart; i < iLineEnd; i++ ) {
		sMark = NULL;
		if ( (pData->iDisplayOptions & XUI_CODE_EDIT_SHOW_WHITESPACE) != 0u ) {
			if ( sText[i] == ' ' ) sMark = ".";
			else if ( sText[i] == '\t' ) sMark = ">";
		}
		if ( sMark != NULL ) {
			tRect = (xui_rect_t){
				fTextX + 4.0f + (float)__xuiCodeEditLineVisualColumn(pData, sText, iLineStart, i) * fColumnWidth - pData->fScrollX,
				fY,
				(sText[i] == '\t') ? (float)__xuiCodeEditTabAdvance(pData, __xuiCodeEditLineVisualColumn(pData, sText, iLineStart, i)) * fColumnWidth : fColumnWidth,
				fLineHeight
			};
			iRet = __xuiCodeEditRenderLineText(pProxy, pDraw, pFont, sMark, 0, (int)strlen(sMark), tRect, iMarkerColor);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	if ( (pData->iDisplayOptions & XUI_CODE_EDIT_SHOW_EOL) != 0u ) {
		tRect = (xui_rect_t){
			fTextX + 4.0f + (float)__xuiCodeEditLineVisualColumn(pData, sText, iLineStart, iLineEnd) * fColumnWidth - pData->fScrollX,
			fY,
			fColumnWidth,
			fLineHeight
		};
		iRet = __xuiCodeEditRenderLineText(pProxy, pDraw, pFont, "$", 0, 1, tRect, iMarkerColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiCodeEditRenderSelectionRange(xui_proxy pProxy, xui_draw_context pDraw, xui_code_edit_data_t* pData, xui_rect_t tContent, const xui_code_selection_t* pSelection, int iLineStart, int iLineEnd, float fTextX, float fY, float fColumnWidth, float fLineHeight, uint32_t iColor)
{
	xui_rect_t tSel;
	const char* sText;
	int iSelStart;
	int iSelEnd;
	int iStart;
	int iEnd;
	int iVisualStart;
	int iVisualEnd;

	if ( pData == NULL || pSelection == NULL ) return XUI_OK;
	iSelStart = pSelection->iAnchorOffset;
	iSelEnd = pSelection->iCaretOffset;
	if ( iSelEnd < iSelStart ) {
		int iTemp = iSelStart;
		iSelStart = iSelEnd;
		iSelEnd = iTemp;
	}
	iStart = (iSelStart > iLineStart) ? iSelStart : iLineStart;
	iEnd = (iSelEnd < iLineEnd) ? iSelEnd : iLineEnd;
	if ( iEnd <= iStart ) return XUI_OK;
	sText = xuiCodeDocumentGetText(pData->pDocument);
	iVisualStart = __xuiCodeEditLineVisualColumn(pData, sText, iLineStart, iStart);
	iVisualEnd = __xuiCodeEditLineVisualColumn(pData, sText, iLineStart, iEnd);
	tSel = (xui_rect_t){
		tContent.fX + fTextX + 4.0f + (float)iVisualStart * fColumnWidth - pData->fScrollX,
		fY,
		(float)(iVisualEnd - iVisualStart) * fColumnWidth,
		fLineHeight
	};
	if ( tSel.fX < tContent.fX ) {
		tSel.fW -= (tContent.fX - tSel.fX);
		tSel.fX = tContent.fX;
	}
	if ( tSel.fX + tSel.fW > tContent.fX + tContent.fW ) tSel.fW = tContent.fX + tContent.fW - tSel.fX;
	if ( tSel.fW <= 0.0f ) return XUI_OK;
	return __xuiCodeEditDrawRectFill(pProxy, pDraw, tSel, iColor);
}

static int __xuiCodeEditRenderSelection(xui_proxy pProxy, xui_draw_context pDraw, xui_code_edit_data_t* pData, xui_rect_t tContent, int iLine, int iLineStart, int iLineEnd, float fTextX, float fY, float fColumnWidth, float fLineHeight, uint32_t iColor)
{
	xui_code_selection_t tSelection;
	int i;
	int iCount;
	int iRet;

	if ( pData == NULL ) return XUI_OK;
	iCount = xuiCodeSelectionGetCount(pData->pSelection);
	for ( i = 0; i < iCount; i++ ) {
		memset(&tSelection, 0, sizeof(tSelection));
		iRet = xuiCodeSelectionGetAt(pData->pSelection, i, &tSelection);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiCodeEditRenderSelectionRange(pProxy, pDraw, pData, tContent, &tSelection, iLineStart, iLineEnd, fTextX, fY, fColumnWidth, fLineHeight, iColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	(void)iLine;
	return XUI_OK;
}

static int __xuiCodeEditLineHasFoldHeader(xui_code_edit_data_t* pData, int iLine)
{
	xui_code_fold_range_t tRange;
	int i;
	int iCount;

	if ( pData == NULL || pData->pFoldState == NULL ) return 0;
	iCount = xuiCodeFoldStateGetCount(pData->pFoldState);
	for ( i = 0; i < iCount; i++ ) {
		if ( xuiCodeFoldStateGetRange(pData->pFoldState, i, &tRange) == XUI_OK &&
		     tRange.iStartLine == iLine ) {
			return 1;
		}
	}
	return 0;
}

static int __xuiCodeEditRenderMargins(xui_widget pWidget, xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, xui_code_edit_data_t* pData, xui_rect_t tContent, const xui_code_margin_info_t* pMargins, int iMarginCount, int iLine, int iActiveLine, int iLineStart, int iLineEnd, float fY, float fLineHeight)
{
	xui_code_marker_t arrMarkers[8];
	xui_code_diagnostic_t arrDiagnostics[8];
	xui_rect_t tLineRect;
	char sNumber[32];
	int i;
	int iCount;
	int iRet;

	if ( pMargins == NULL || iMarginCount <= 0 ) return XUI_OK;
	for ( i = 0; i < iMarginCount; i++ ) {
		tLineRect = (xui_rect_t){pMargins[i].tRect.fX, fY, pMargins[i].tRect.fW, fLineHeight};
		if ( tLineRect.fX + tLineRect.fW > tContent.fW ) continue;
		if ( pMargins[i].onRender != NULL ) {
			iRet = pMargins[i].onRender(pWidget, pMargins[i].iId, iLine, pDraw, tLineRect, pMargins[i].pUser);
			if ( iRet != XUI_OK ) return iRet;
			continue;
		}
		if ( pMargins[i].iKind == XUI_CODE_MARGIN_LINE_NUMBER ) {
			if ( iLine == iActiveLine ) {
				iRet = __xuiCodeEditDrawRectFill(pProxy, pDraw, tLineRect, XUI_COLOR_RGBA(226, 232, 240, 255));
				if ( iRet != XUI_OK ) return iRet;
			}
			snprintf(sNumber, sizeof(sNumber), "%d", iLine + 1);
			iRet = __xuiCodeEditRenderLineText(pProxy, pDraw, pFont, sNumber, 0, (int)strlen(sNumber), (xui_rect_t){tLineRect.fX + 2.0f, tLineRect.fY, tLineRect.fW - 4.0f, tLineRect.fH}, (iLine == iActiveLine) ? XUI_COLOR_RGBA(15, 23, 42, 255) : XUI_COLOR_RGBA(100, 116, 139, 255));
			if ( iRet != XUI_OK ) return iRet;
		} else if ( pMargins[i].iKind == XUI_CODE_MARGIN_MARKER ) {
			iCount = 0;
			iRet = xuiCodeAnnotationGetMarkers(pData->pAnnotations, iLine, arrMarkers, (int)(sizeof(arrMarkers) / sizeof(arrMarkers[0])), &iCount);
			if ( iRet != XUI_OK ) return iRet;
			if ( iCount > 0 ) {
				iRet = __xuiCodeEditDrawRectFill(pProxy, pDraw, (xui_rect_t){tLineRect.fX + 4.0f, tLineRect.fY + 5.0f, 7.0f, 7.0f}, XUI_COLOR_RGBA(37, 99, 235, 255));
				if ( iRet != XUI_OK ) return iRet;
			}
		} else if ( pMargins[i].iKind == XUI_CODE_MARGIN_FOLD ) {
			if ( __xuiCodeEditLineHasFoldHeader(pData, iLine) ) {
				iRet = __xuiCodeEditRenderLineText(pProxy, pDraw, pFont, "-", 0, 1, (xui_rect_t){tLineRect.fX + 4.0f, tLineRect.fY, 8.0f, tLineRect.fH}, XUI_COLOR_RGBA(71, 85, 105, 255));
				if ( iRet != XUI_OK ) return iRet;
			}
		} else if ( pMargins[i].iKind == XUI_CODE_MARGIN_DIAGNOSTIC ) {
			iCount = 0;
			iRet = xuiCodeAnnotationGetDiagnosticsAt(pData->pAnnotations, iLineStart, arrDiagnostics, (int)(sizeof(arrDiagnostics) / sizeof(arrDiagnostics[0])), &iCount);
			if ( iRet != XUI_OK ) return iRet;
			if ( iCount == 0 && iLineEnd > iLineStart ) {
				iRet = xuiCodeAnnotationGetDiagnosticsAt(pData->pAnnotations, iLineEnd - 1, arrDiagnostics, (int)(sizeof(arrDiagnostics) / sizeof(arrDiagnostics[0])), &iCount);
				if ( iRet != XUI_OK ) return iRet;
			}
			if ( iCount > 0 ) {
				uint32_t iColor = (arrDiagnostics[0].iSeverity == XUI_CODE_DIAGNOSTIC_ERROR) ? XUI_COLOR_RGBA(220, 38, 38, 255) : XUI_COLOR_RGBA(217, 119, 6, 255);
				iRet = __xuiCodeEditDrawRectFill(pProxy, pDraw, (xui_rect_t){tLineRect.fX + 4.0f, tLineRect.fY + 5.0f, 7.0f, 7.0f}, iColor);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	return XUI_OK;
}

static int __xuiCodeEditCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_code_edit_data_t* pData;
	xui_context pContext;
	xui_proxy_t* pProxy;
	xui_font pFont;
	xui_rect_t tRect;
	xui_rect_t tContent;
	xui_rect_t tTextContent;
	xui_rect_t tCaret;
	xui_rect_t tIme;
	xui_code_margin_info_t arrMargins[12];
	xui_code_selection_t tSelection;
	const char* sText;
	uint32_t iBackgroundColor;
	uint32_t iDividerColor;
	uint32_t iTextColor;
	uint32_t iCurrentLineColor;
	uint32_t iSelectionColor;
	uint32_t iCaretColor;
	uint32_t iImeColor;
	int iLineCount;
	int iMarginCount;
	int iLine;
	int iVisibleRow;
	int iFirstVisibleRow;
	int iStart;
	int iEnd;
	int iActiveLine;
	int iCaretLine;
	int iCaretColumn;
	int iCaretStart;
	int iCaretEnd;
	int iCaretVisualColumn;
	float fY;
	float fColumnWidth;
	float fLineHeight;
	float fCaretHeight;
	float fCaretLineY;
	float fTextOffsetY;
	float fTextY;
	xui_code_token_t arrTokens[96];
	int iTokenCount;
	int iRet;

	(void)iStateId;
	(void)pUser;
	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	pProxy = xuiInternalContextGetProxy(pContext);
	if ( pProxy == NULL ) return XUI_OK;
	iRet = __xuiCodeEditUpdateScrollModel(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	tRect = xuiWidgetGetContentRect(pWidget);
	tContent = pData->tScrollViewportRect;
	memset(arrMargins, 0, sizeof(arrMargins));
	iMarginCount = 0;
	tTextContent = tContent;
	iRet = xuiCodeMarginModelLayout(pData->pMargins, tContent, arrMargins, (int)(sizeof(arrMargins) / sizeof(arrMargins[0])), &iMarginCount, &tTextContent);
	if ( iRet != XUI_OK ) return iRet;
	iBackgroundColor = pData->bReadonly ?
		__xuiCodeEditColor(pWidget, "codeedit.readonly.background_color", XUI_COLOR_RGBA(241, 245, 249, 255)) :
		__xuiCodeEditColor(pWidget, "codeedit.background.color", XUI_COLOR_RGBA(248, 250, 252, 255));
	iDividerColor = __xuiCodeEditColor(pWidget, "codeedit.divider.color", XUI_COLOR_RGBA(203, 213, 225, 255));
	iCurrentLineColor = __xuiCodeEditColor(pWidget, "codeedit.current_line.color", XUI_COLOR_RGBA(226, 232, 240, 128));
	iSelectionColor = __xuiCodeEditColor(pWidget, "codeedit.selection.color", XUI_COLOR_RGBA(78, 135, 216, 92));
	iCaretColor = __xuiCodeEditColor(pWidget, "codeedit.caret.color", XUI_COLOR_RGBA(15, 23, 42, 255));
	iImeColor = __xuiCodeEditColor(pWidget, "codeedit.ime.color", XUI_COLOR_RGBA(37, 99, 235, 255));
	iRet = __xuiCodeEditDrawRectFill(pProxy, pDraw, (xui_rect_t){0.0f, 0.0f, tRect.fW, tRect.fH}, iBackgroundColor);
	if ( iRet != XUI_OK ) return iRet;
	if ( tTextContent.fX > 0.0f ) {
		iRet = __xuiCodeEditDrawRectFill(pProxy, pDraw, (xui_rect_t){0.0f, 0.0f, tTextContent.fX, tContent.fH}, XUI_COLOR_RGBA(241, 245, 249, 255));
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiCodeEditDrawRectFill(pProxy, pDraw, (xui_rect_t){tTextContent.fX - 1.0f, 0.0f, 1.0f, tContent.fH}, iDividerColor);
	if ( iRet != XUI_OK ) return iRet;
	pFont = (pData->pFont != NULL) ? pData->pFont : xuiGetDefaultFont(pContext);
	fColumnWidth = __xuiCodeEditColumnWidth(pWidget, pData);
	fLineHeight = __xuiCodeEditLineHeight(pWidget, pData);
	fCaretHeight = __xuiCodeEditCaretHeight(pWidget, pData, fLineHeight);
	fTextOffsetY = __xuiCodeEditLineTextOffsetY(pWidget, pData, fLineHeight);
	sText = xuiCodeDocumentGetText(pData->pDocument);
	iRet = __xuiCodeEditEnsureTokens(pData);
	if ( iRet != XUI_OK ) return iRet;
	iTokenCount = 0;
	(void)xuiCodeTokenBufferGetTokens(pData->pTokenBuffer, xuiCodeDocumentGetVersion(pData->pDocument), arrTokens, (int)(sizeof(arrTokens) / sizeof(arrTokens[0])), &iTokenCount);
	iTextColor = pData->bReadonly ?
		__xuiCodeEditColor(pWidget, "codeedit.text.readonly_color", XUI_COLOR_RGBA(71, 85, 105, 255)) :
		__xuiCodeEditColor(pWidget, "codeedit.text.color", XUI_COLOR_RGBA(15, 23, 42, 255));
	iLineCount = xuiCodeDocumentGetLineCount(pData->pDocument);
	iActiveLine = -1;
	memset(&tSelection, 0, sizeof(tSelection));
	if ( xuiCodeSelectionGetState(pData->pSelection, &tSelection) == XUI_OK ) {
		(void)xuiCodeDocumentOffsetToLineColumn(pData->pDocument, tSelection.iCaretOffset, &iActiveLine, NULL);
	}
	iFirstVisibleRow = (int)((pData->fScrollY - 4.0f) / fLineHeight);
	if ( iFirstVisibleRow < 0 ) iFirstVisibleRow = 0;
	if ( iFirstVisibleRow > 0 ) iFirstVisibleRow--;
	iVisibleRow = 0;
	for ( iLine = 0; iLine < iLineCount; iLine++ ) {
		if ( !__xuiCodeEditLineVisible(pData, iLine) ) continue;
		if ( iVisibleRow < iFirstVisibleRow ) {
			iVisibleRow++;
			continue;
		}
		fY = 4.0f + (float)iVisibleRow * fLineHeight - pData->fScrollY;
		fTextY = fY + fTextOffsetY;
		if ( fY + fLineHeight < 0.0f ) continue;
		if ( fY > tContent.fH ) break;
		iRet = xuiCodeDocumentGetLineRange(pData->pDocument, iLine, &iStart, &iEnd);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiCodeEditRenderMargins(pWidget, pProxy, pDraw, pFont, pData, tContent, arrMargins, iMarginCount, iLine, iActiveLine, iStart, iEnd, fY, fLineHeight);
		if ( iRet != XUI_OK ) return iRet;
		if ( iLine == iActiveLine ) {
			iRet = __xuiCodeEditDrawRectFill(pProxy, pDraw, (xui_rect_t){tTextContent.fX, fY, tTextContent.fW, fLineHeight}, iCurrentLineColor);
			if ( iRet != XUI_OK ) return iRet;
		}
		iRet = __xuiCodeEditRenderSelection(pProxy, pDraw, pData, tTextContent, iLine, iStart, iEnd, 0.0f, fY, fColumnWidth, fLineHeight, iSelectionColor);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiCodeEditRenderStyledLine(pWidget, pProxy, pDraw, pFont, pData, sText, iStart, iEnd, arrTokens, iTokenCount, tTextContent.fX, fTextY, fColumnWidth, fLineHeight, iTextColor);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiCodeEditRenderWhitespace(pWidget, pProxy, pDraw, pFont, pData, sText, iStart, iEnd, tTextContent.fX, fTextY, fColumnWidth, fLineHeight);
		if ( iRet != XUI_OK ) return iRet;
		iVisibleRow++;
	}
	{
		int iSelectionIndex;
		int iSelectionCount;

		iSelectionCount = xuiCodeSelectionGetCount(pData->pSelection);
		for ( iSelectionIndex = 0; iSelectionIndex < iSelectionCount; iSelectionIndex++ ) {
			memset(&tSelection, 0, sizeof(tSelection));
			if ( xuiCodeSelectionGetAt(pData->pSelection, iSelectionIndex, &tSelection) != XUI_OK ) continue;
			if ( tSelection.iAnchorOffset != tSelection.iCaretOffset ) continue;
			if ( xuiCodeDocumentOffsetToLineColumn(pData->pDocument, (pData->bImeComposing && iSelectionIndex == XUI_CODE_SELECTION_PRIMARY) ? pData->iImeAnchorOffset : tSelection.iCaretOffset, &iCaretLine, &iCaretColumn) != XUI_OK ) continue;
			iCaretStart = 0;
			iCaretEnd = 0;
			(void)xuiCodeDocumentGetLineRange(pData->pDocument, iCaretLine, &iCaretStart, &iCaretEnd);
			iCaretVisualColumn = __xuiCodeEditLineVisualColumn(pData, sText, iCaretStart, iCaretStart + iCaretColumn);
			fCaretLineY = 4.0f + (float)__xuiCodeEditLineToVisibleRow(pData, iCaretLine) * fLineHeight - pData->fScrollY;
			tCaret = (xui_rect_t){
				tTextContent.fX + 4.0f + (float)iCaretVisualColumn * fColumnWidth - pData->fScrollX,
				fCaretLineY + fTextOffsetY,
				1.0f,
				fCaretHeight
			};
			if ( tCaret.fX >= 0.0f && tCaret.fX <= tContent.fW && tCaret.fY + tCaret.fH >= 0.0f && tCaret.fY <= tContent.fH ) {
				if ( iSelectionIndex == XUI_CODE_SELECTION_PRIMARY && pData->bImeComposing && pData->sImeComposition[0] != '\0' ) {
					tIme = (xui_rect_t){tCaret.fX, fCaretLineY, tContent.fW - tCaret.fX, fLineHeight};
					iRet = __xuiCodeEditRenderLineText(pProxy, pDraw, pFont, pData->sImeComposition, 0, (int)strlen(pData->sImeComposition), tIme, iImeColor);
					if ( iRet != XUI_OK ) return iRet;
					iRet = __xuiCodeEditDrawRectFill(pProxy, pDraw, (xui_rect_t){tCaret.fX, fCaretLineY + fLineHeight - 2.0f, (float)strlen(pData->sImeComposition) * fColumnWidth, 1.0f}, iImeColor);
					if ( iRet != XUI_OK ) return iRet;
				}
				iRet = __xuiCodeEditDrawRectFill(pProxy, pDraw, tCaret, iCaretColor);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	return XUI_OK;
}

static void __xuiCodeEditRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
{
	xui_style_property_info_t tInfo;

	if ( xuiStyleFindProperty(pContext, sName) != 0 ) return;
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.sName = sName;
	tInfo.iValueType = iValueType;
	tInfo.iDirtyFlags = iDirtyFlags;
	tInfo.iFlags = iFlags;
	tInfo.pWidgetType = pType;
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
}

static void __xuiCodeEditRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty = XUI_WIDGET_DIRTY_STYLE | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;

	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.text.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.text.readonly_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.readonly.background_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.current_line.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.divider.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.selection.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.caret.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.ime.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.whitespace.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.syntax.keyword.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.syntax.type.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.syntax.number.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.syntax.string.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.syntax.char.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.syntax.comment.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.syntax.preprocessor.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.syntax.operator.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.syntax.brace.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "codeedit.syntax.error.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiCodeEditRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, XUI_WIDGET_DIRTY_LAYOUT | iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
}

XUI_API xui_widget_type xuiCodeEditGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "codeedit");
	if ( pType != NULL ) {
		__xuiCodeEditRegisterStyleProperties(pContext, pType);
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "codeedit";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = 0;
	tDesc.iTypeDataSize = sizeof(xui_code_edit_data_t);
	tDesc.onInit = __xuiCodeEditInit;
	tDesc.onDestroy = __xuiCodeEditDestroy;
	tDesc.onContentMeasure = __xuiCodeEditMeasure;
	tDesc.onCacheRender = __xuiCodeEditCacheRender;
	__xuiCodeEditDefaultLayout(&tDesc.tLayout);
	__xuiCodeEditDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) return NULL;
	__xuiCodeEditRegisterStyleProperties(pContext, pType);
	return pType;
}

XUI_API int xuiCodeEditCreate(xui_context pContext, xui_widget* ppWidget, const xui_code_edit_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( !xuiInternalContextIsValid(pContext) || (ppWidget == NULL) || !__xuiCodeEditDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppWidget = NULL;
	pType = xuiCodeEditGetType(pContext);
	if ( pType == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API xui_code_document xuiCodeEditGetDocument(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? pData->pDocument : NULL;
}

XUI_API xui_code_selection_model xuiCodeEditGetSelection(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? pData->pSelection : NULL;
}

XUI_API xui_code_theme xuiCodeEditGetTheme(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? pData->pTheme : NULL;
}

XUI_API int xuiCodeEditSetTheme(xui_widget pWidget, xui_code_theme pTheme)
{
	xui_code_edit_data_t* pData;
	int iRet;

	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL || pTheme == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiCodeThemeCopy(pData->pTheme, pTheme);
	if ( iRet != XUI_OK ) return iRet;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_STYLE | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiCodeEditSetStyle(xui_widget pWidget, int iStyleId, const xui_code_style_t* pStyle)
{
	xui_code_edit_data_t* pData;
	int iRet;

	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiCodeThemeSetStyle(pData->pTheme, iStyleId, pStyle);
	if ( iRet != XUI_OK ) return iRet;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_STYLE | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API xui_code_fold_state xuiCodeEditGetFoldState(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? pData->pFoldState : NULL;
}

XUI_API xui_code_annotation_store xuiCodeEditGetAnnotations(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? pData->pAnnotations : NULL;
}

XUI_API xui_code_token_buffer xuiCodeEditGetTokenBuffer(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? pData->pTokenBuffer : NULL;
}

XUI_API xui_code_provider_set xuiCodeEditGetProviders(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? pData->pProviders : NULL;
}

XUI_API xui_code_margin_model xuiCodeEditGetMargins(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? pData->pMargins : NULL;
}

XUI_API xui_code_command_map xuiCodeEditGetCommandMap(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? pData->pCommandMap : NULL;
}

XUI_API xui_code_language_registry xuiCodeEditGetLanguageRegistry(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? pData->pLanguages : NULL;
}

XUI_API int xuiCodeEditSetLanguage(xui_widget pWidget, const char* sLanguage)
{
	xui_code_edit_data_t* pData;
	xui_code_language_t tLanguage;

	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL || sLanguage == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( sLanguage[0] != '\0' ) {
		memset(&tLanguage, 0, sizeof(tLanguage));
		if ( xuiCodeLanguageRegistryFind(pData->pLanguages, sLanguage, &tLanguage) != XUI_OK ) {
			__xuiCodeEditSetError(pData, "CodeEdit language not registered");
			return XUI_ERROR_UNSUPPORTED;
		}
	}
	strncpy(pData->sLanguage, sLanguage, sizeof(pData->sLanguage) - 1u);
	pData->sLanguage[sizeof(pData->sLanguage) - 1u] = '\0';
	__xuiCodeEditSetError(pData, "");
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API const char* xuiCodeEditGetLanguage(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? pData->sLanguage : "";
}

XUI_API xui_widget xuiCodeEditGetMenuWidget(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? pData->pMenu : NULL;
}

XUI_API int xuiCodeEditSetText(xui_widget pWidget, const char* sText)
{
	xui_code_edit_data_t* pData;
	int iRet;

	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiCodeDocumentSetText(pData->pDocument, sText);
	__xuiCodeEditSetError(pData, (iRet == XUI_OK) ? "" : xuiCodeDocumentGetLastError(pData->pDocument));
	if ( iRet == XUI_OK ) (void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return iRet;
}

XUI_API const char* xuiCodeEditGetText(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? xuiCodeDocumentGetText(pData->pDocument) : "";
}

static int __xuiCodeEditAfterReplaceAll(xui_widget pWidget, xui_code_edit_data_t* pData, const char* sReplacement, int iReplaceCount, int iRet)
{
	xui_code_range_t tRange;
	int iIndicatorCount;
	int iMarkRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iRet != XUI_OK ) {
		__xuiCodeEditSetError(pData, "CodeEdit replace failed");
		return iRet;
	}
	(void)xuiCodeSearchClearResultIndicators(pData->pAnnotations, 0);
	if ( iReplaceCount > 0 && sReplacement != NULL && sReplacement[0] != '\0' ) {
		iIndicatorCount = 0;
		iMarkRet = xuiCodeSearchMarkAllPlain(pData->pDocument, pData->pAnnotations, sReplacement, XUI_CODE_SEARCH_CASE_SENSITIVE, XUI_CODE_INDICATOR_SEARCH_RESULT, &iIndicatorCount);
		if ( iMarkRet != XUI_OK && iMarkRet != XUI_ERROR_UNSUPPORTED ) {
			__xuiCodeEditSetError(pData, "CodeEdit replace indicator update failed");
			return iMarkRet;
		}
	}
	if ( xuiCodeDocumentGetLastEditRange(pData->pDocument, &tRange) == XUI_OK ) {
		(void)xuiCodeSelectionSetRange(pData->pSelection, pData->pDocument, tRange.iStart, tRange.iEnd);
	}
	__xuiCodeEditSetError(pData, "");
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiCodeEditReplaceAllPlain(xui_widget pWidget, const char* sPattern, const char* sReplacement, uint32_t iFlags, int* pReplaceCount)
{
	xui_code_edit_data_t* pData;
	int iCount;
	int iRet;

	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->bReadonly ) {
		__xuiCodeEditSetError(pData, "CodeEdit is readonly");
		return XUI_ERROR_UNSUPPORTED;
	}
	iCount = 0;
	iRet = xuiCodeSearchReplaceAllPlain(pData->pDocument, sPattern, sReplacement, iFlags, &iCount);
	if ( pReplaceCount != NULL ) *pReplaceCount = iCount;
	return __xuiCodeEditAfterReplaceAll(pWidget, pData, sReplacement, iCount, iRet);
}

XUI_API int xuiCodeEditReplaceAllRegex(xui_widget pWidget, const char* sPattern, const char* sReplacement, uint32_t iFlags, int* pReplaceCount)
{
	xui_code_edit_data_t* pData;
	char sError[160];
	int iCount;
	int iRet;

	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->bReadonly ) {
		__xuiCodeEditSetError(pData, "CodeEdit is readonly");
		return XUI_ERROR_UNSUPPORTED;
	}
	memset(sError, 0, sizeof(sError));
	iCount = 0;
	iRet = xuiCodeSearchReplaceAllRegex(pData->pDocument, sPattern, sReplacement, iFlags, &iCount, sError, (int)sizeof(sError));
	if ( iRet != XUI_OK && sError[0] != '\0' ) __xuiCodeEditSetError(pData, sError);
	if ( pReplaceCount != NULL ) *pReplaceCount = iCount;
	return __xuiCodeEditAfterReplaceAll(pWidget, pData, sReplacement, iCount, iRet);
}

XUI_API int xuiCodeEditSetReadonly(xui_widget pWidget, int bReadonly)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->bReadonly = bReadonly ? 1 : 0;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_STYLE | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiCodeEditIsReadonly(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? pData->bReadonly : 0;
}

XUI_API xui_scroll_model_t* xuiCodeEditGetScrollModel(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL ) return NULL;
	(void)__xuiCodeEditUpdateScrollModel(pWidget, pData);
	return &pData->tScrollModel;
}

XUI_API xui_widget xuiCodeEditGetHScrollBarWidget(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL ) return NULL;
	(void)__xuiCodeEditUpdateScrollModel(pWidget, pData);
	return pData->pHScrollBar;
}

XUI_API xui_widget xuiCodeEditGetVScrollBarWidget(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL ) return NULL;
	(void)__xuiCodeEditUpdateScrollModel(pWidget, pData);
	return pData->pVScrollBar;
}

XUI_API int xuiCodeEditSetScroll(xui_widget pWidget, float fScrollX, float fScrollY)
{
	xui_code_edit_data_t* pData;
	int iRet;

	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( fScrollX < 0.0f ) fScrollX = 0.0f;
	if ( fScrollY < 0.0f ) fScrollY = 0.0f;
	pData->fScrollX = fScrollX;
	pData->fScrollY = fScrollY;
	iRet = __xuiCodeEditUpdateScrollModel(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiCodeEditGetScroll(xui_widget pWidget, float* pScrollX, float* pScrollY)
{
	xui_code_edit_data_t* pData;

	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pScrollX != NULL ) *pScrollX = pData->fScrollX;
	if ( pScrollY != NULL ) *pScrollY = pData->fScrollY;
	return XUI_OK;
}

XUI_API int xuiCodeEditEnsureCaretVisible(xui_widget pWidget)
{
	xui_code_edit_data_t* pData;
	xui_code_selection_t tSelection;
	xui_rect_t tCaret;
	const char* sText;
	float fMarginWidth;
	int iStart;
	int iEnd;
	int iLine;
	int iColumn;
	int iVisualColumn;
	int iRet;
	float fScrollX;
	float fScrollY;

	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tSelection, 0, sizeof(tSelection));
	iRet = xuiCodeSelectionGetState(pData->pSelection, &tSelection);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeDocumentOffsetToLineColumn(pData->pDocument, tSelection.iCaretOffset, &iLine, &iColumn);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCodeEditUpdateScrollModel(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	fMarginWidth = 0.0f;
	(void)xuiCodeMarginModelGetTotalWidth(pData->pMargins, &fMarginWidth);
	sText = xuiCodeDocumentGetText(pData->pDocument);
	iStart = 0;
	iEnd = 0;
	(void)xuiCodeDocumentGetLineRange(pData->pDocument, iLine, &iStart, &iEnd);
	iVisualColumn = __xuiCodeEditLineVisualColumn(pData, sText, iStart, iStart + iColumn);
	tCaret = (xui_rect_t){
		fMarginWidth + 4.0f + (float)iVisualColumn * __xuiCodeEditColumnWidth(pWidget, pData),
		(float)__xuiCodeEditLineToVisibleRow(pData, iLine) * __xuiCodeEditLineHeight(pWidget, pData),
		__xuiCodeEditColumnWidth(pWidget, pData),
		__xuiCodeEditLineHeight(pWidget, pData)
	};
	iRet = xuiScrollModelEnsureRectVisible(&pData->tScrollModel, tCaret);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiScrollModelGetOffset(&pData->tScrollModel, &fScrollX, &fScrollY);
	pData->fScrollX = fScrollX;
	pData->fScrollY = fScrollY;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiCodeEditSetDisplayOptions(xui_widget pWidget, uint32_t iOptions)
{
	xui_code_edit_data_t* pData;

	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->iDisplayOptions = iOptions & (XUI_CODE_EDIT_SHOW_WHITESPACE | XUI_CODE_EDIT_SHOW_EOL | XUI_CODE_EDIT_SHOW_INDENT_GUIDES);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_STYLE | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API uint32_t xuiCodeEditGetDisplayOptions(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? pData->iDisplayOptions : 0u;
}

XUI_API int xuiCodeEditSetTabColumns(xui_widget pWidget, int iTabColumns)
{
	xui_code_edit_data_t* pData;

	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL || iTabColumns <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->iTabColumns = iTabColumns;
	if ( pData->iIndentColumns <= 0 ) pData->iIndentColumns = iTabColumns;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiCodeEditGetTabColumns(xui_widget pWidget)
{
	xui_code_edit_data_t* pData;

	pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? __xuiCodeEditTabColumns(pData) : 0;
}

XUI_API int xuiCodeEditOpenMenu(xui_widget pWidget, float fX, float fY)
{
	xui_code_edit_data_t* pData;
	int iRet;

	pData = __xuiCodeEditGetData(pWidget);
	if ( pData == NULL || pData->pMenu == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiCodeEditUpdateMenu(pWidget, pData);
	if ( iRet != XUI_OK ) {
		__xuiCodeEditSetError(pData, "CodeEdit menu update failed");
		return iRet;
	}
	(void)xuiMenuSetSelect(pData->pMenu, __xuiCodeEditMenuSelect, pWidget);
	iRet = xuiMenuOpenAt(pData->pMenu, pWidget, fX, fY);
	__xuiCodeEditSetError(pData, (iRet == XUI_OK) ? "" : "CodeEdit menu open failed");
	return iRet;
}

XUI_API const char* xuiCodeEditGetLastError(xui_widget pWidget)
{
	xui_code_edit_data_t* pData = __xuiCodeEditGetData(pWidget);
	return (pData != NULL) ? pData->sError : "invalid codeedit widget";
}
