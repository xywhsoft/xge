#include "xui_internal.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XUI_TEXT_EDIT_KEY_BACKSPACE	8
#define XUI_TEXT_EDIT_KEY_DELETE	46
#define XUI_TEXT_EDIT_HISTORY_LIMIT	64

typedef struct xui_text_edit_line_t {
	int iStart;
	int iEnd;
	float fY;
	float fW;
} xui_text_edit_line_t;

typedef struct xui_text_edit_history_t {
	char* sText;
	int iCursor;
	int iSelectStart;
	int iSelectEnd;
	float fScrollX;
	float fScrollY;
} xui_text_edit_history_t;

typedef struct xui_text_edit_data_t {
	char* sText;
	int iTextCapacity;
	char* sPlaceholder;
	int iPlaceholderCapacity;
	char* sScratch;
	int iScratchCapacity;
	char* sLineText;
	int iLineTextCapacity;
	char* arrMenuTitle[XUI_INPUT_MENU_COUNT];
	xui_text_edit_line_t* pLines;
	int iLineCount;
	int iLineCapacity;
	xui_widget pMenu;
	xui_font pFont;
	xui_text_edit_change_proc onChange;
	void* pChangeUser;
	int iChangeCount;
	int iCursor;
	int iAnchor;
	int iSelectStart;
	int iSelectEnd;
	int bDragging;
	float fScrollX;
	float fScrollY;
	float fContentWidth;
	float fContentHeight;
	float fLastLayoutWidth;
	float fLineHeight;
	int bLinesDirty;
	int iMaxLength;
	int bReadonly;
	int bWordWrap;
	int bLineNumbers;
	float fLineNumberWidth;
	xui_text_edit_history_t arrUndo[XUI_TEXT_EDIT_HISTORY_LIMIT];
	xui_text_edit_history_t arrRedo[XUI_TEXT_EDIT_HISTORY_LIMIT];
	int iUndoCount;
	int iRedoCount;
	uint32_t iTextColor;
	uint32_t iPlaceholderColor;
	uint32_t iDisabledTextColor;
	uint32_t iBackgroundColor;
	uint32_t iHoverBackgroundColor;
	uint32_t iDisabledBackgroundColor;
	uint32_t iBorderColor;
	uint32_t iHoverBorderColor;
	uint32_t iFocusBorderColor;
	uint32_t iSelectionColor;
	uint32_t iCursorColor;
	uint32_t iLineNumberColor;
	uint32_t iLineNumberBackgroundColor;
	uint32_t iLineNumberBorderColor;
	float fRadius;
	float fBorderWidth;
	float fLineGap;
	xui_rect_t tTextRect;
	xui_rect_t tCursorRect;
} xui_text_edit_data_t;

static const char* g_xuiTextEditDefaultMenuTitles[XUI_INPUT_MENU_COUNT] = {
	"撤销",
	"剪切",
	"复制",
	"粘贴",
	"删除",
	"全选"
};

static int __xuiTextEditDescValid(const xui_text_edit_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) {
		return 0;
	}
	if ( (pDesc->iMaxLength < 0) ||
	     (pDesc->fRadius < 0.0f) ||
	     (pDesc->fBorderWidth < 0.0f) ||
	     (pDesc->fLineGap < 0.0f) ||
	     (pDesc->fLineNumberWidth < 0.0f) ) {
		return 0;
	}
	return 1;
}

static int __xuiTextEditAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static uint32_t __xuiTextEditColorWithAlpha(uint32_t iColor, uint32_t iAlpha)
{
	return (iColor & 0xffffff00u) | (iAlpha & 0xffu);
}

static int __xuiTextEditTextReserve(char** psText, int* pCapacity, int iCapacity)
{
	char* sNew;

	if ( (psText == NULL) || (pCapacity == NULL) || (iCapacity <= 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iCapacity <= *pCapacity ) {
		return XUI_OK;
	}
	sNew = (char*)xrtRealloc(*psText, (size_t)iCapacity);
	if ( sNew == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	*psText = sNew;
	*pCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiTextEditStringSet(char** psText, int* pCapacity, const char* sText)
{
	int iNeed;
	int iRet;

	if ( sText == NULL ) {
		sText = "";
	}
	iNeed = (int)strlen(sText) + 1;
	iRet = __xuiTextEditTextReserve(psText, pCapacity, iNeed);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	memcpy(*psText, sText, (size_t)iNeed);
	return XUI_OK;
}

static char* __xuiTextEditStringDuplicate(const char* sText)
{
	char* sCopy;
	int iCapacity;

	sCopy = NULL;
	iCapacity = 0;
	if ( __xuiTextEditStringSet(&sCopy, &iCapacity, sText) != XUI_OK ) {
		return NULL;
	}
	return sCopy;
}

static int __xuiTextEditUtf8IsCont(unsigned char c)
{
	return (c & 0xc0u) == 0x80u;
}

static int __xuiTextEditUtf8Next(const char* sText, int iLen, int iPos)
{
	unsigned char c;
	int iNext;

	if ( sText == NULL ) {
		return 0;
	}
	if ( iPos < 0 ) {
		iPos = 0;
	}
	if ( iPos >= iLen ) {
		return iLen;
	}
	c = (unsigned char)sText[iPos];
	if ( c < 0x80u ) {
		iNext = iPos + 1;
	} else if ( (c & 0xe0u) == 0xc0u ) {
		iNext = iPos + 2;
	} else if ( (c & 0xf0u) == 0xe0u ) {
		iNext = iPos + 3;
	} else if ( (c & 0xf8u) == 0xf0u ) {
		iNext = iPos + 4;
	} else {
		iNext = iPos + 1;
	}
	if ( iNext > iLen ) {
		iNext = iLen;
	}
	return iNext;
}

static int __xuiTextEditUtf8Prev(const char* sText, int iLen, int iPos)
{
	if ( sText == NULL ) {
		return 0;
	}
	if ( iPos > iLen ) {
		iPos = iLen;
	}
	if ( iPos <= 0 ) {
		return 0;
	}
	iPos--;
	while ( (iPos > 0) && __xuiTextEditUtf8IsCont((unsigned char)sText[iPos]) ) {
		iPos--;
	}
	return iPos;
}

static int __xuiTextEditUtf8Clamp(const char* sText, int iLen, int iPos)
{
	if ( iPos < 0 ) {
		return 0;
	}
	if ( iPos > iLen ) {
		return iLen;
	}
	while ( (iPos > 0) && (iPos < iLen) && __xuiTextEditUtf8IsCont((unsigned char)sText[iPos]) ) {
		iPos--;
	}
	return iPos;
}

static int __xuiTextEditUtf8ClampBytes(const char* sText, int iMaxBytes)
{
	int iLen;
	int iPos;
	int iNext;

	if ( sText == NULL || iMaxBytes <= 0 ) {
		return 0;
	}
	iLen = (int)strlen(sText);
	if ( iLen <= iMaxBytes ) {
		return iLen;
	}
	iPos = 0;
	while ( iPos < iLen ) {
		iNext = __xuiTextEditUtf8Next(sText, iLen, iPos);
		if ( iNext > iMaxBytes ) {
			break;
		}
		iPos = iNext;
	}
	return iPos;
}

static int __xuiTextEditCharClass(const char* sText, int iPos)
{
	unsigned char c;

	if ( (sText == NULL) || (iPos < 0) || (sText[iPos] == '\0') ) {
		return 0;
	}
	c = (unsigned char)sText[iPos];
	if ( c <= 32u ) {
		return 0;
	}
	if ( c >= 128u ) {
		return 1;
	}
	if ( ((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'Z')) ||
	     ((c >= 'a') && (c <= 'z')) || (c == '_') ) {
		return 1;
	}
	return 2;
}

static int __xuiTextEditPrevWord(xui_text_edit_data_t* pData)
{
	const char* sText;
	int iLen;
	int iPos;
	int iPrev;
	int iClass;

	if ( pData == NULL || pData->sText == NULL ) {
		return 0;
	}
	sText = pData->sText;
	iLen = (int)strlen(sText);
	iPos = __xuiTextEditUtf8Prev(sText, iLen, pData->iCursor);
	while ( iPos > 0 && __xuiTextEditCharClass(sText, iPos) == 0 ) {
		iPos = __xuiTextEditUtf8Prev(sText, iLen, iPos);
	}
	iClass = __xuiTextEditCharClass(sText, iPos);
	while ( iPos > 0 ) {
		iPrev = __xuiTextEditUtf8Prev(sText, iLen, iPos);
		if ( __xuiTextEditCharClass(sText, iPrev) != iClass ) {
			break;
		}
		iPos = iPrev;
	}
	return iPos;
}

static int __xuiTextEditNextWord(xui_text_edit_data_t* pData)
{
	const char* sText;
	int iLen;
	int iPos;
	int iNext;
	int iClass;

	if ( pData == NULL || pData->sText == NULL ) {
		return 0;
	}
	sText = pData->sText;
	iLen = (int)strlen(sText);
	iPos = __xuiTextEditUtf8Clamp(sText, iLen, pData->iCursor);
	if ( iPos >= iLen ) {
		return iLen;
	}
	iClass = __xuiTextEditCharClass(sText, iPos);
	while ( iPos < iLen ) {
		iNext = __xuiTextEditUtf8Next(sText, iLen, iPos);
		if ( iNext <= iPos ) {
			break;
		}
		if ( __xuiTextEditCharClass(sText, iNext) != iClass ) {
			iPos = iNext;
			break;
		}
		iPos = iNext;
	}
	while ( iPos < iLen && __xuiTextEditCharClass(sText, iPos) == 0 ) {
		iNext = __xuiTextEditUtf8Next(sText, iLen, iPos);
		if ( iNext <= iPos ) {
			break;
		}
		iPos = iNext;
	}
	return iPos;
}

static int __xuiTextEditUtf8Encode(uint32_t iCodepoint, char* sText)
{
	if ( sText == NULL ) {
		return 0;
	}
	if ( iCodepoint <= 0x7fu ) {
		sText[0] = (char)iCodepoint;
		return 1;
	}
	if ( iCodepoint <= 0x7ffu ) {
		sText[0] = (char)(0xc0u | ((iCodepoint >> 6) & 0x1fu));
		sText[1] = (char)(0x80u | (iCodepoint & 0x3fu));
		return 2;
	}
	if ( iCodepoint <= 0xffffu ) {
		sText[0] = (char)(0xe0u | ((iCodepoint >> 12) & 0x0fu));
		sText[1] = (char)(0x80u | ((iCodepoint >> 6) & 0x3fu));
		sText[2] = (char)(0x80u | (iCodepoint & 0x3fu));
		return 3;
	}
	if ( iCodepoint <= 0x10ffffu ) {
		sText[0] = (char)(0xf0u | ((iCodepoint >> 18) & 0x07u));
		sText[1] = (char)(0x80u | ((iCodepoint >> 12) & 0x3fu));
		sText[2] = (char)(0x80u | ((iCodepoint >> 6) & 0x3fu));
		sText[3] = (char)(0x80u | (iCodepoint & 0x3fu));
		return 4;
	}
	return 0;
}

static xui_text_edit_data_t* __xuiTextEditGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return NULL;
	}
	pContext = xuiWidgetGetContext(pWidget);
	pType = xuiWidgetFindType(pContext, "textedit");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_text_edit_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiTextEditStyleColor(xui_widget pWidget, const char* sName, const char* sFallback, uint32_t* pColor)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet != XUI_OK) && (sFallback != NULL) ) {
		memset(&tProperty, 0, sizeof(tProperty));
		tProperty.iSize = sizeof(tProperty);
		iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sFallback, &tProperty);
	}
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR) ) {
		*pColor = tProperty.tValue.iColor;
		return 1;
	}
	return 0;
}

static int __xuiTextEditStyleFloat(xui_widget pWidget, const char* sName, const char* sFallback, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet != XUI_OK) && (sFallback != NULL) ) {
		memset(&tProperty, 0, sizeof(tProperty));
		tProperty.iSize = sizeof(tProperty);
		iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sFallback, &tProperty);
	}
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) && (tProperty.tValue.fFloat >= 0.0f) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static xui_font __xuiTextEditStyleFont(xui_widget pWidget, xui_font pBaseFont)
{
	xui_style_property_t tProperty;
	xui_font pFont;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, "font.name", &tProperty);
	if ( (iRet == XUI_OK) &&
	     (tProperty.tValue.iType == XUI_STYLE_VALUE_STRING) &&
	     (tProperty.tValue.sText != NULL) ) {
		pFont = xuiFindFont(xuiWidgetGetContext(pWidget), tProperty.tValue.sText);
		if ( pFont != NULL ) {
			return pFont;
		}
	}
	return pBaseFont;
}

static void __xuiTextEditResolve(xui_widget pWidget, xui_text_edit_data_t* pData, xui_text_edit_data_t* pResolved)
{
	*pResolved = *pData;
	pResolved->pFont = (pData->pFont != NULL) ? pData->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	(void)__xuiTextEditStyleColor(pWidget, "textedit.text.color", "input.text.color", &pResolved->iTextColor);
	(void)__xuiTextEditStyleColor(pWidget, "textedit.placeholder.color", "input.placeholder.color", &pResolved->iPlaceholderColor);
	(void)__xuiTextEditStyleColor(pWidget, "textedit.text.disabled_color", "input.text.disabled_color", &pResolved->iDisabledTextColor);
	(void)__xuiTextEditStyleColor(pWidget, "textedit.background.color", "input.background.color", &pResolved->iBackgroundColor);
	(void)__xuiTextEditStyleColor(pWidget, "textedit.background.hover_color", "input.background.hover_color", &pResolved->iHoverBackgroundColor);
	(void)__xuiTextEditStyleColor(pWidget, "textedit.background.disabled_color", "input.background.disabled_color", &pResolved->iDisabledBackgroundColor);
	(void)__xuiTextEditStyleColor(pWidget, "textedit.border.color", "input.border.color", &pResolved->iBorderColor);
	(void)__xuiTextEditStyleColor(pWidget, "textedit.border.hover_color", "input.border.hover_color", &pResolved->iHoverBorderColor);
	(void)__xuiTextEditStyleColor(pWidget, "textedit.border.focus_color", "input.border.focus_color", &pResolved->iFocusBorderColor);
	(void)__xuiTextEditStyleColor(pWidget, "textedit.selection.color", "input.selection.color", &pResolved->iSelectionColor);
	(void)__xuiTextEditStyleColor(pWidget, "textedit.cursor.color", "input.cursor.color", &pResolved->iCursorColor);
	(void)__xuiTextEditStyleColor(pWidget, "textedit.line_number.color", NULL, &pResolved->iLineNumberColor);
	(void)__xuiTextEditStyleColor(pWidget, "textedit.line_number.background_color", NULL, &pResolved->iLineNumberBackgroundColor);
	(void)__xuiTextEditStyleColor(pWidget, "textedit.line_number.border_color", NULL, &pResolved->iLineNumberBorderColor);
	(void)__xuiTextEditStyleFloat(pWidget, "textedit.radius", "input.radius", &pResolved->fRadius);
	(void)__xuiTextEditStyleFloat(pWidget, "textedit.border.width", "input.border.width", &pResolved->fBorderWidth);
	(void)__xuiTextEditStyleFloat(pWidget, "textedit.line_gap", NULL, &pResolved->fLineGap);
	(void)__xuiTextEditStyleFloat(pWidget, "textedit.line_number.width", NULL, &pResolved->fLineNumberWidth);
	pResolved->pFont = __xuiTextEditStyleFont(pWidget, pResolved->pFont);
}

static float __xuiTextEditLineNumberWidth(const xui_text_edit_data_t* pResolved)
{
	if ( (pResolved == NULL) || !pResolved->bLineNumbers ) {
		return 0.0f;
	}
	return (pResolved->fLineNumberWidth > 0.0f) ? pResolved->fLineNumberWidth : 44.0f;
}

static xui_rect_t __xuiTextEditTextContentRect(xui_widget pWidget, const xui_text_edit_data_t* pResolved)
{
	xui_rect_t tContent;
	float fLineNumberWidth;

	tContent = xuiWidgetGetContentRect(pWidget);
	fLineNumberWidth = __xuiTextEditLineNumberWidth(pResolved);
	if ( fLineNumberWidth > 0.0f ) {
		if ( fLineNumberWidth < tContent.fW ) {
			tContent.fX += fLineNumberWidth;
			tContent.fW -= fLineNumberWidth;
		} else {
			tContent.fX += tContent.fW;
			tContent.fW = 0.0f;
		}
	}
	return tContent;
}

static xui_rect_t __xuiTextEditLineNumberRect(xui_widget pWidget, const xui_text_edit_data_t* pResolved)
{
	xui_rect_t tContent;
	float fLineNumberWidth;

	tContent = xuiWidgetGetContentRect(pWidget);
	fLineNumberWidth = __xuiTextEditLineNumberWidth(pResolved);
	if ( (fLineNumberWidth <= 0.0f) || (tContent.fW <= 0.0f) ) {
		return (xui_rect_t){tContent.fX, tContent.fY, 0.0f, tContent.fH};
	}
	if ( fLineNumberWidth > tContent.fW ) {
		fLineNumberWidth = tContent.fW;
	}
	tContent.fW = fLineNumberWidth;
	return tContent;
}

static void __xuiTextEditSelectionRange(xui_text_edit_data_t* pData, int* pStart, int* pEnd)
{
	int iStart;
	int iEnd;

	iStart = (pData != NULL) ? pData->iSelectStart : 0;
	iEnd = (pData != NULL) ? pData->iSelectEnd : 0;
	if ( iStart > iEnd ) {
		int iTmp = iStart;
		iStart = iEnd;
		iEnd = iTmp;
	}
	if ( pStart != NULL ) {
		*pStart = iStart;
	}
	if ( pEnd != NULL ) {
		*pEnd = iEnd;
	}
}

static int __xuiTextEditHasSelectionData(xui_text_edit_data_t* pData)
{
	int iStart;
	int iEnd;

	if ( pData == NULL ) {
		return 0;
	}
	__xuiTextEditSelectionRange(pData, &iStart, &iEnd);
	return iStart != iEnd;
}

static int __xuiTextEditSetSelectionData(xui_text_edit_data_t* pData, int iStart, int iEnd)
{
	int iLen;

	if ( (pData == NULL) || (pData->sText == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iLen = (int)strlen(pData->sText);
	iStart = __xuiTextEditUtf8Clamp(pData->sText, iLen, iStart);
	iEnd = __xuiTextEditUtf8Clamp(pData->sText, iLen, iEnd);
	pData->iSelectStart = iStart;
	pData->iSelectEnd = iEnd;
	pData->iAnchor = iStart;
	pData->iCursor = iEnd;
	return XUI_OK;
}

static int __xuiTextEditClearSelectionData(xui_text_edit_data_t* pData)
{
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData->iSelectStart = pData->iCursor;
	pData->iSelectEnd = pData->iCursor;
	pData->iAnchor = pData->iCursor;
	return XUI_OK;
}

static void __xuiTextEditMarkLinesDirty(xui_text_edit_data_t* pData)
{
	if ( pData != NULL ) {
		pData->bLinesDirty = 1;
	}
}

static int __xuiTextEditInvalidateText(xui_widget pWidget)
{
	xui_text_edit_data_t* pData;

	pData = __xuiTextEditGetData(pWidget);
	__xuiTextEditMarkLinesDirty(pData);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiTextEditInvalidatePaint(xui_widget pWidget)
{
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiTextEditAssignTextBytes(xui_text_edit_data_t* pData, const char* sText, int iSize)
{
	int iRet;
	int iLen;

	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( sText == NULL ) {
		sText = "";
	}
	if ( iSize < 0 ) {
		iSize = (int)strlen(sText);
	}
	if ( pData->iMaxLength > 0 && iSize > pData->iMaxLength ) {
		iSize = __xuiTextEditUtf8ClampBytes(sText, pData->iMaxLength);
	}
	iRet = __xuiTextEditTextReserve(&pData->sText, &pData->iTextCapacity, iSize + 1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( iSize > 0 ) {
		memcpy(pData->sText, sText, (size_t)iSize);
	}
	pData->sText[iSize] = '\0';
	iLen = (int)strlen(pData->sText);
	pData->iCursor = __xuiTextEditUtf8Clamp(pData->sText, iLen, pData->iCursor);
	pData->iSelectStart = __xuiTextEditUtf8Clamp(pData->sText, iLen, pData->iSelectStart);
	pData->iSelectEnd = __xuiTextEditUtf8Clamp(pData->sText, iLen, pData->iSelectEnd);
	__xuiTextEditMarkLinesDirty(pData);
	return XUI_OK;
}

static void __xuiTextEditHistoryStateFree(xui_text_edit_history_t* pState)
{
	if ( pState == NULL ) {
		return;
	}
	if ( pState->sText != NULL ) {
		xrtFree(pState->sText);
	}
	memset(pState, 0, sizeof(*pState));
}

static void __xuiTextEditHistoryClear(xui_text_edit_history_t* pStates, int* pCount)
{
	int i;

	if ( (pStates == NULL) || (pCount == NULL) ) {
		return;
	}
	for ( i = 0; i < *pCount; i++ ) {
		__xuiTextEditHistoryStateFree(&pStates[i]);
	}
	*pCount = 0;
}

static int __xuiTextEditHistoryCapture(xui_text_edit_data_t* pData, xui_text_edit_history_t* pState)
{
	if ( (pData == NULL) || (pState == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pState, 0, sizeof(*pState));
	pState->sText = __xuiTextEditStringDuplicate((pData->sText != NULL) ? pData->sText : "");
	if ( pState->sText == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pState->iCursor = pData->iCursor;
	pState->iSelectStart = pData->iSelectStart;
	pState->iSelectEnd = pData->iSelectEnd;
	pState->fScrollX = pData->fScrollX;
	pState->fScrollY = pData->fScrollY;
	return XUI_OK;
}

static int __xuiTextEditHistoryPush(xui_text_edit_history_t* pStates, int* pCount, const xui_text_edit_history_t* pState)
{
	if ( (pStates == NULL) || (pCount == NULL) || (pState == NULL) || (pState->sText == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( *pCount >= XUI_TEXT_EDIT_HISTORY_LIMIT ) {
		__xuiTextEditHistoryStateFree(&pStates[0]);
		memmove(pStates, pStates + 1, sizeof(pStates[0]) * (size_t)(XUI_TEXT_EDIT_HISTORY_LIMIT - 1));
		*pCount = XUI_TEXT_EDIT_HISTORY_LIMIT - 1;
	}
	pStates[*pCount] = *pState;
	(*pCount)++;
	return XUI_OK;
}

static int __xuiTextEditRecordUndo(xui_text_edit_data_t* pData)
{
	xui_text_edit_history_t tState;
	int iRet;

	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiTextEditHistoryCapture(pData, &tState);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = __xuiTextEditHistoryPush(pData->arrUndo, &pData->iUndoCount, &tState);
	if ( iRet != XUI_OK ) {
		__xuiTextEditHistoryStateFree(&tState);
		return iRet;
	}
	__xuiTextEditHistoryClear(pData->arrRedo, &pData->iRedoCount);
	return XUI_OK;
}

static void __xuiTextEditNotifyChange(xui_widget pWidget, xui_text_edit_data_t* pData)
{
	if ( pData == NULL ) {
		return;
	}
	pData->iChangeCount++;
	if ( pData->onChange != NULL ) {
		pData->onChange(pWidget, (pData->sText != NULL) ? pData->sText : "", pData->pChangeUser);
	}
}

static int __xuiTextEditLineReserve(xui_text_edit_data_t* pData, int iCapacity)
{
	xui_text_edit_line_t* pLines;

	if ( (pData == NULL) || (iCapacity <= 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iCapacity <= pData->iLineCapacity ) {
		return XUI_OK;
	}
	if ( iCapacity < pData->iLineCapacity * 2 ) {
		iCapacity = pData->iLineCapacity * 2;
	}
	if ( iCapacity < 16 ) {
		iCapacity = 16;
	}
	pLines = (xui_text_edit_line_t*)xrtRealloc(pData->pLines, sizeof(*pLines) * (size_t)iCapacity);
	if ( pLines == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pData->pLines = pLines;
	pData->iLineCapacity = iCapacity;
	return XUI_OK;
}

static float __xuiTextEditLineHeight(xui_widget pWidget, xui_font pFont, float fLineGap)
{
	xui_proxy pProxy;
	xui_font_metrics_t tMetrics;
	float fHeight;

	fHeight = 16.0f;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy != NULL) && (pProxy->fontGetMetrics != NULL) && (pFont != NULL) &&
	     (pProxy->fontGetMetrics(pProxy, pFont, &tMetrics) == XUI_OK) &&
	     (tMetrics.fLineHeight > 0.0f) ) {
		fHeight = tMetrics.fLineHeight;
	}
	fHeight += fLineGap;
	if ( fHeight < 12.0f ) {
		fHeight = 12.0f;
	}
	return fHeight;
}

static int __xuiTextEditBuildLineText(xui_text_edit_data_t* pData, int iStart, int iEnd, char** psText)
{
	int iSize;
	int iRet;

	if ( (pData == NULL) || (pData->sText == NULL) || (psText == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iEnd < iStart ) {
		iEnd = iStart;
	}
	iSize = iEnd - iStart;
	iRet = __xuiTextEditTextReserve(&pData->sLineText, &pData->iLineTextCapacity, iSize + 1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( iSize > 0 ) {
		memcpy(pData->sLineText, pData->sText + iStart, (size_t)iSize);
	}
	pData->sLineText[iSize] = '\0';
	*psText = pData->sLineText;
	return XUI_OK;
}

static xui_vec2_t __xuiTextEditMeasureRange(xui_widget pWidget, xui_text_edit_data_t* pData, xui_font pFont, int iStart, int iEnd)
{
	xui_proxy pProxy;
	xui_vec2_t tSize;
	int iSize;
	int iRet;

	tSize.fX = 0.0f;
	tSize.fY = pData->fLineHeight;
	if ( (pWidget == NULL) || (pData == NULL) || (pData->sText == NULL) || (iEnd <= iStart) || (pFont == NULL) ) {
		return tSize;
	}
	iSize = iEnd - iStart;
	iRet = __xuiTextEditTextReserve(&pData->sScratch, &pData->iScratchCapacity, iSize + 1);
	if ( iRet != XUI_OK ) {
		return tSize;
	}
	memcpy(pData->sScratch, pData->sText + iStart, (size_t)iSize);
	pData->sScratch[iSize] = '\0';
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy != NULL) && (pProxy->textMeasure != NULL) ) {
		(void)pProxy->textMeasure(pProxy, pFont, pData->sScratch, &tSize);
	}
	return tSize;
}

static int __xuiTextEditAddLine(xui_widget pWidget, xui_text_edit_data_t* pData, xui_font pFont, int iStart, int iEnd)
{
	xui_text_edit_line_t* pLine;
	xui_vec2_t tSize;
	int iRet;

	iRet = __xuiTextEditLineReserve(pData, pData->iLineCount + 1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	tSize = __xuiTextEditMeasureRange(pWidget, pData, pFont, iStart, iEnd);
	pLine = &pData->pLines[pData->iLineCount];
	pLine->iStart = iStart;
	pLine->iEnd = iEnd;
	pLine->fY = (float)pData->iLineCount * pData->fLineHeight;
	pLine->fW = tSize.fX;
	if ( pLine->fW > pData->fContentWidth ) {
		pData->fContentWidth = pLine->fW;
	}
	pData->iLineCount++;
	return XUI_OK;
}

static int __xuiTextEditBuildWrappedSegment(xui_widget pWidget, xui_text_edit_data_t* pData, xui_font pFont, int iStart, int iEnd, float fMaxWidth)
{
	xui_vec2_t tSize;
	int iLineStart;
	int iPos;
	int iPrev;
	int iNext;
	int iRet;

	if ( (iStart >= iEnd) || !pData->bWordWrap || (fMaxWidth <= 16.0f) ) {
		return __xuiTextEditAddLine(pWidget, pData, pFont, iStart, iEnd);
	}
	iLineStart = iStart;
	while ( iLineStart < iEnd ) {
		iPos = iLineStart;
		iPrev = iLineStart;
		while ( iPos < iEnd ) {
			iNext = __xuiTextEditUtf8Next(pData->sText, iEnd, iPos);
			tSize = __xuiTextEditMeasureRange(pWidget, pData, pFont, iLineStart, iNext);
			if ( (tSize.fX > fMaxWidth) && (iPrev > iLineStart) ) {
				break;
			}
			iPrev = iNext;
			iPos = iNext;
		}
		if ( iPrev <= iLineStart ) {
			iPrev = __xuiTextEditUtf8Next(pData->sText, iEnd, iLineStart);
		}
		iRet = __xuiTextEditAddLine(pWidget, pData, pFont, iLineStart, iPrev);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
		iLineStart = iPrev;
	}
	return XUI_OK;
}

static int __xuiTextEditBuildLines(xui_widget pWidget, xui_text_edit_data_t* pData, xui_font pFont, float fWrapWidth)
{
	int iLen;
	int iStart;
	int iBreak;
	int iEnd;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->sText == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !pData->bLinesDirty && (pData->fLastLayoutWidth == fWrapWidth) ) {
		return XUI_OK;
	}
	pData->iLineCount = 0;
	pData->fContentWidth = 0.0f;
	pData->fLineHeight = __xuiTextEditLineHeight(pWidget, pFont, pData->fLineGap);
	pData->fLastLayoutWidth = fWrapWidth;
	iLen = (int)strlen(pData->sText);
	if ( iLen == 0 ) {
		iRet = __xuiTextEditAddLine(pWidget, pData, pFont, 0, 0);
		if ( iRet != XUI_OK ) return iRet;
		pData->fContentHeight = pData->fLineHeight;
		pData->bLinesDirty = 0;
		return XUI_OK;
	}
	iStart = 0;
	while ( 1 ) {
		iBreak = iStart;
		while ( (iBreak < iLen) && (pData->sText[iBreak] != '\n') ) {
			iBreak++;
		}
		iEnd = iBreak;
		if ( (iEnd > iStart) && (pData->sText[iEnd - 1] == '\r') ) {
			iEnd--;
		}
		iRet = __xuiTextEditBuildWrappedSegment(pWidget, pData, pFont, iStart, iEnd, fWrapWidth);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
		if ( iBreak >= iLen ) {
			break;
		}
		iStart = iBreak + 1;
		if ( iStart == iLen ) {
			iRet = __xuiTextEditAddLine(pWidget, pData, pFont, iLen, iLen);
			if ( iRet != XUI_OK ) return iRet;
			break;
		}
	}
	pData->fContentHeight = (float)pData->iLineCount * pData->fLineHeight;
	if ( pData->fContentHeight < pData->fLineHeight ) {
		pData->fContentHeight = pData->fLineHeight;
	}
	pData->bLinesDirty = 0;
	return XUI_OK;
}

static int __xuiTextEditFindLineForPos(xui_text_edit_data_t* pData, int iPos)
{
	int i;

	if ( (pData == NULL) || (pData->iLineCount <= 0) ) {
		return 0;
	}
	for ( i = 0; i < pData->iLineCount; i++ ) {
		if ( iPos < pData->pLines[i].iStart ) {
			return (i > 0) ? (i - 1) : 0;
		}
		if ( iPos <= pData->pLines[i].iEnd ) {
			return i;
		}
	}
	return pData->iLineCount - 1;
}

static void __xuiTextEditClampScroll(xui_widget pWidget, xui_text_edit_data_t* pData)
{
	xui_text_edit_data_t tResolved;
	xui_rect_t tContent;
	float fMaxX;
	float fMaxY;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return;
	}
	__xuiTextEditResolve(pWidget, pData, &tResolved);
	tContent = __xuiTextEditTextContentRect(pWidget, &tResolved);
	fMaxX = pData->fContentWidth - tContent.fW + 4.0f;
	fMaxY = pData->fContentHeight - tContent.fH;
	if ( pData->bWordWrap ) {
		fMaxX = 0.0f;
	}
	if ( fMaxX < 0.0f ) fMaxX = 0.0f;
	if ( fMaxY < 0.0f ) fMaxY = 0.0f;
	if ( pData->fScrollX < 0.0f ) pData->fScrollX = 0.0f;
	if ( pData->fScrollY < 0.0f ) pData->fScrollY = 0.0f;
	if ( pData->fScrollX > fMaxX ) pData->fScrollX = fMaxX;
	if ( pData->fScrollY > fMaxY ) pData->fScrollY = fMaxY;
}

static int __xuiTextEditUpdateCursorRect(xui_widget pWidget, xui_text_edit_data_t* pData)
{
	xui_text_edit_data_t tResolved;
	xui_rect_t tContent;
	xui_text_edit_line_t* pLine;
	xui_vec2_t tPrefix;
	float fWrapWidth;
	int iLine;
	int iCursor;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiTextEditResolve(pWidget, pData, &tResolved);
	tContent = __xuiTextEditTextContentRect(pWidget, &tResolved);
	fWrapWidth = (tContent.fW > 1.0f) ? tContent.fW : 1.0f;
	iRet = __xuiTextEditBuildLines(pWidget, pData, tResolved.pFont, fWrapWidth);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	__xuiTextEditClampScroll(pWidget, pData);
	iLine = __xuiTextEditFindLineForPos(pData, pData->iCursor);
	pLine = &pData->pLines[iLine];
	iCursor = pData->iCursor;
	if ( iCursor < pLine->iStart ) iCursor = pLine->iStart;
	if ( iCursor > pLine->iEnd ) iCursor = pLine->iEnd;
	tPrefix = __xuiTextEditMeasureRange(pWidget, pData, tResolved.pFont, pLine->iStart, iCursor);
	pData->tCursorRect = (xui_rect_t){
		tContent.fX + tPrefix.fX - pData->fScrollX,
		tContent.fY + pLine->fY - pData->fScrollY + 2.0f,
		1.0f,
		pData->fLineHeight - 4.0f
	};
	pData->tTextRect = tContent;
	return XUI_OK;
}

static int __xuiTextEditEnsureCursorVisible(xui_widget pWidget, xui_text_edit_data_t* pData)
{
	xui_text_edit_data_t tResolved;
	xui_rect_t tContent;
	xui_text_edit_line_t* pLine;
	xui_vec2_t tPrefix;
	float fWrapWidth;
	float fCursorX;
	float fCursorY;
	int iLine;
	int iCursor;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiTextEditResolve(pWidget, pData, &tResolved);
	tContent = __xuiTextEditTextContentRect(pWidget, &tResolved);
	fWrapWidth = (tContent.fW > 1.0f) ? tContent.fW : 1.0f;
	iRet = __xuiTextEditBuildLines(pWidget, pData, tResolved.pFont, fWrapWidth);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iLine = __xuiTextEditFindLineForPos(pData, pData->iCursor);
	pLine = &pData->pLines[iLine];
	iCursor = pData->iCursor;
	if ( iCursor < pLine->iStart ) iCursor = pLine->iStart;
	if ( iCursor > pLine->iEnd ) iCursor = pLine->iEnd;
	tPrefix = __xuiTextEditMeasureRange(pWidget, pData, tResolved.pFont, pLine->iStart, iCursor);
	fCursorX = tPrefix.fX;
	fCursorY = pLine->fY;
	if ( fCursorX < pData->fScrollX ) {
		pData->fScrollX = fCursorX;
	} else if ( fCursorX > pData->fScrollX + tContent.fW - 2.0f ) {
		pData->fScrollX = fCursorX - tContent.fW + 2.0f;
	}
	if ( fCursorY < pData->fScrollY ) {
		pData->fScrollY = fCursorY;
	} else if ( fCursorY + pData->fLineHeight > pData->fScrollY + tContent.fH ) {
		pData->fScrollY = fCursorY + pData->fLineHeight - tContent.fH;
	}
	__xuiTextEditClampScroll(pWidget, pData);
	return __xuiTextEditUpdateCursorRect(pWidget, pData);
}

static int __xuiTextEditSyncCursor(xui_widget pWidget, xui_text_edit_data_t* pData)
{
	xui_context pContext;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiTextEditEnsureCursorVisible(pWidget, pData);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pContext = xuiWidgetGetContext(pWidget);
	if ( (pContext != NULL) && (xuiGetFocusWidget(pContext) == pWidget) ) {
		(void)xuiInternalInputSyncIme(pContext);
	}
	return __xuiTextEditInvalidatePaint(pWidget);
}

static int __xuiTextEditSetTextInternal(xui_widget pWidget, xui_text_edit_data_t* pData, const char* sText, int bNotify, int bUndo)
{
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( bUndo ) {
		iRet = __xuiTextEditRecordUndo(pData);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	iRet = __xuiTextEditAssignTextBytes(pData, sText, -1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pData->iCursor = (int)strlen(pData->sText);
	(void)__xuiTextEditClearSelectionData(pData);
	(void)__xuiTextEditEnsureCursorVisible(pWidget, pData);
	if ( bNotify ) {
		__xuiTextEditNotifyChange(pWidget, pData);
	}
	return __xuiTextEditInvalidateText(pWidget);
}

static int __xuiTextEditDeleteRange(xui_widget pWidget, xui_text_edit_data_t* pData, int iStart, int iEnd, int bNotify, int bUndo)
{
	int iLen;
	int iMove;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->sText == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iLen = (int)strlen(pData->sText);
	iStart = __xuiTextEditUtf8Clamp(pData->sText, iLen, iStart);
	iEnd = __xuiTextEditUtf8Clamp(pData->sText, iLen, iEnd);
	if ( iStart > iEnd ) {
		int iTmp = iStart;
		iStart = iEnd;
		iEnd = iTmp;
	}
	if ( iStart == iEnd ) {
		return XUI_OK;
	}
	if ( bUndo ) {
		iRet = __xuiTextEditRecordUndo(pData);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	iMove = iLen - iEnd + 1;
	memmove(pData->sText + iStart, pData->sText + iEnd, (size_t)iMove);
	pData->iCursor = iStart;
	(void)__xuiTextEditClearSelectionData(pData);
	__xuiTextEditMarkLinesDirty(pData);
	(void)__xuiTextEditEnsureCursorVisible(pWidget, pData);
	if ( bNotify ) {
		__xuiTextEditNotifyChange(pWidget, pData);
	}
	return __xuiTextEditInvalidateText(pWidget);
}

static int __xuiTextEditInsertText(xui_widget pWidget, xui_text_edit_data_t* pData, const char* sInsert, int iInsertSize)
{
	int iStart;
	int iEnd;
	int iLen;
	int iSelected;
	int iAllowed;
	int iNewLen;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (sInsert == NULL) || pData->bReadonly ) {
		return XUI_OK;
	}
	if ( iInsertSize < 0 ) {
		iInsertSize = (int)strlen(sInsert);
	}
	if ( iInsertSize <= 0 ) {
		return XUI_OK;
	}
	__xuiTextEditSelectionRange(pData, &iStart, &iEnd);
	iLen = (int)strlen(pData->sText);
	iSelected = iEnd - iStart;
	if ( pData->iMaxLength > 0 ) {
		iAllowed = pData->iMaxLength - (iLen - iSelected);
		if ( iAllowed <= 0 ) {
			return XUI_OK;
		}
		if ( iInsertSize > iAllowed ) {
			iInsertSize = __xuiTextEditUtf8ClampBytes(sInsert, iAllowed);
		}
		if ( iInsertSize <= 0 ) {
			return XUI_OK;
		}
	}
	iRet = __xuiTextEditRecordUndo(pData);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iNewLen = iLen - iSelected + iInsertSize;
	iRet = __xuiTextEditTextReserve(&pData->sText, &pData->iTextCapacity, iNewLen + 1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	memmove(pData->sText + iStart + iInsertSize, pData->sText + iEnd, (size_t)(iLen - iEnd + 1));
	memcpy(pData->sText + iStart, sInsert, (size_t)iInsertSize);
	pData->iCursor = iStart + iInsertSize;
	(void)__xuiTextEditClearSelectionData(pData);
	__xuiTextEditMarkLinesDirty(pData);
	(void)__xuiTextEditEnsureCursorVisible(pWidget, pData);
	__xuiTextEditNotifyChange(pWidget, pData);
	return __xuiTextEditInvalidateText(pWidget);
}

static int __xuiTextEditCopySelection(xui_widget pWidget, xui_text_edit_data_t* pData)
{
	xui_proxy pProxy;
	int iStart;
	int iEnd;
	int iLen;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || !__xuiTextEditHasSelectionData(pData) ) {
		return XUI_OK;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy == NULL) || (pProxy->clipboardSetText == NULL) ) {
		return XUI_OK;
	}
	__xuiTextEditSelectionRange(pData, &iStart, &iEnd);
	iLen = iEnd - iStart;
	iRet = __xuiTextEditTextReserve(&pData->sScratch, &pData->iScratchCapacity, iLen + 1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	memcpy(pData->sScratch, pData->sText + iStart, (size_t)iLen);
	pData->sScratch[iLen] = '\0';
	return pProxy->clipboardSetText(pProxy, pData->sScratch);
}

static int __xuiTextEditNormalizeLineBreaks(xui_text_edit_data_t* pData, const char* sText, int iSize, char** psText, int* pSize)
{
	int i;
	int iOut;
	int iRet;

	if ( (pData == NULL) || (sText == NULL) || (psText == NULL) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iSize < 0 ) {
		iSize = (int)strlen(sText);
	}
	iRet = __xuiTextEditTextReserve(&pData->sScratch, &pData->iScratchCapacity, iSize + 1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iOut = 0;
	for ( i = 0; i < iSize; i++ ) {
		if ( sText[i] == '\r' ) {
			if ( (i + 1 < iSize) && (sText[i + 1] == '\n') ) {
				continue;
			}
			pData->sScratch[iOut++] = '\n';
		} else {
			pData->sScratch[iOut++] = sText[i];
		}
	}
	pData->sScratch[iOut] = '\0';
	*psText = pData->sScratch;
	*pSize = iOut;
	return XUI_OK;
}

static int __xuiTextEditPasteClipboard(xui_widget pWidget, xui_text_edit_data_t* pData)
{
	xui_proxy pProxy;
	char sBuffer[4096];
	char* sText;
	int iLen;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || pData->bReadonly ) {
		return XUI_OK;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy == NULL) || (pProxy->clipboardGetText == NULL) ) {
		return XUI_OK;
	}
	memset(sBuffer, 0, sizeof(sBuffer));
	iLen = pProxy->clipboardGetText(pProxy, sBuffer, (int)sizeof(sBuffer));
	if ( iLen < 0 ) {
		return XUI_OK;
	}
	sBuffer[sizeof(sBuffer) - 1u] = '\0';
	iRet = __xuiTextEditNormalizeLineBreaks(pData, sBuffer, -1, &sText, &iLen);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	return __xuiTextEditInsertText(pWidget, pData, sText, iLen);
}

static int __xuiTextEditMoveCursor(xui_widget pWidget, xui_text_edit_data_t* pData, int iNewCursor, int bExtend)
{
	int iLen;
	int iAnchor;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->sText == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iLen = (int)strlen(pData->sText);
	iNewCursor = __xuiTextEditUtf8Clamp(pData->sText, iLen, iNewCursor);
	if ( bExtend ) {
		iAnchor = __xuiTextEditHasSelectionData(pData) ? pData->iAnchor : pData->iCursor;
		pData->iAnchor = iAnchor;
		pData->iCursor = iNewCursor;
		pData->iSelectStart = iAnchor;
		pData->iSelectEnd = iNewCursor;
	} else {
		pData->iCursor = iNewCursor;
		(void)__xuiTextEditClearSelectionData(pData);
	}
	return __xuiTextEditSyncCursor(pWidget, pData);
}

static int __xuiTextEditSelectWordAt(xui_widget pWidget, xui_text_edit_data_t* pData, int iCursor)
{
	const char* sText;
	int iLen;
	int iStart;
	int iEnd;
	int iPrev;
	int iNext;
	int iClass;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->sText == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	sText = pData->sText;
	iLen = (int)strlen(sText);
	if ( iLen <= 0 ) {
		return __xuiTextEditMoveCursor(pWidget, pData, 0, 0);
	}
	iStart = __xuiTextEditUtf8Clamp(sText, iLen, iCursor);
	if ( iStart >= iLen ) {
		iStart = __xuiTextEditUtf8Prev(sText, iLen, iLen);
	}
	iClass = __xuiTextEditCharClass(sText, iStart);
	if ( iClass == 0 ) {
		return __xuiTextEditMoveCursor(pWidget, pData, iCursor, 0);
	}
	iEnd = __xuiTextEditUtf8Next(sText, iLen, iStart);
	while ( iStart > 0 ) {
		iPrev = __xuiTextEditUtf8Prev(sText, iLen, iStart);
		if ( __xuiTextEditCharClass(sText, iPrev) != iClass ) {
			break;
		}
		iStart = iPrev;
	}
	while ( iEnd < iLen ) {
		iNext = __xuiTextEditUtf8Next(sText, iLen, iEnd);
		if ( __xuiTextEditCharClass(sText, iEnd) != iClass ) {
			break;
		}
		iEnd = iNext;
	}
	(void)__xuiTextEditSetSelectionData(pData, iStart, iEnd);
	return __xuiTextEditSyncCursor(pWidget, pData);
}

static int __xuiTextEditMoveCursorVertical(xui_widget pWidget, xui_text_edit_data_t* pData, int iDelta, int bExtend)
{
	xui_text_edit_data_t tResolved;
	xui_rect_t tContent;
	xui_text_edit_line_t* pLine;
	xui_vec2_t tPrefix;
	float fQueryX;
	float fPrev;
	float fNext;
	float fWrapWidth;
	int iLine;
	int iTarget;
	int iPos;
	int iNext;
	int iCursor;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiTextEditResolve(pWidget, pData, &tResolved);
	tContent = __xuiTextEditTextContentRect(pWidget, &tResolved);
	fWrapWidth = (tContent.fW > 1.0f) ? tContent.fW : 1.0f;
	iRet = __xuiTextEditBuildLines(pWidget, pData, tResolved.pFont, fWrapWidth);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iLine = __xuiTextEditFindLineForPos(pData, pData->iCursor);
	iTarget = iLine + iDelta;
	if ( iTarget < 0 ) iTarget = 0;
	if ( iTarget >= pData->iLineCount ) iTarget = pData->iLineCount - 1;
	pLine = &pData->pLines[iLine];
	iCursor = pData->iCursor;
	if ( iCursor < pLine->iStart ) iCursor = pLine->iStart;
	if ( iCursor > pLine->iEnd ) iCursor = pLine->iEnd;
	tPrefix = __xuiTextEditMeasureRange(pWidget, pData, tResolved.pFont, pLine->iStart, iCursor);
	fQueryX = tPrefix.fX;
	pLine = &pData->pLines[iTarget];
	iPos = pLine->iStart;
	fPrev = 0.0f;
	while ( iPos < pLine->iEnd ) {
		iNext = __xuiTextEditUtf8Next(pData->sText, pLine->iEnd, iPos);
		tPrefix = __xuiTextEditMeasureRange(pWidget, pData, tResolved.pFont, pLine->iStart, iNext);
		fNext = tPrefix.fX;
		if ( fQueryX < (fPrev + fNext) * 0.5f ) {
			return __xuiTextEditMoveCursor(pWidget, pData, iPos, bExtend);
		}
		iPos = iNext;
		fPrev = fNext;
	}
	return __xuiTextEditMoveCursor(pWidget, pData, pLine->iEnd, bExtend);
}

static int __xuiTextEditLineStartForCursor(xui_widget pWidget, xui_text_edit_data_t* pData)
{
	xui_text_edit_data_t tResolved;
	xui_rect_t tContent;
	float fWrapWidth;
	int iRet;
	int iLine;

	__xuiTextEditResolve(pWidget, pData, &tResolved);
	tContent = __xuiTextEditTextContentRect(pWidget, &tResolved);
	fWrapWidth = (tContent.fW > 1.0f) ? tContent.fW : 1.0f;
	iRet = __xuiTextEditBuildLines(pWidget, pData, tResolved.pFont, fWrapWidth);
	if ( iRet != XUI_OK ) {
		return pData->iCursor;
	}
	iLine = __xuiTextEditFindLineForPos(pData, pData->iCursor);
	return pData->pLines[iLine].iStart;
}

static int __xuiTextEditLineEndForCursor(xui_widget pWidget, xui_text_edit_data_t* pData)
{
	xui_text_edit_data_t tResolved;
	xui_rect_t tContent;
	float fWrapWidth;
	int iRet;
	int iLine;

	__xuiTextEditResolve(pWidget, pData, &tResolved);
	tContent = __xuiTextEditTextContentRect(pWidget, &tResolved);
	fWrapWidth = (tContent.fW > 1.0f) ? tContent.fW : 1.0f;
	iRet = __xuiTextEditBuildLines(pWidget, pData, tResolved.pFont, fWrapWidth);
	if ( iRet != XUI_OK ) {
		return pData->iCursor;
	}
	iLine = __xuiTextEditFindLineForPos(pData, pData->iCursor);
	return pData->pLines[iLine].iEnd;
}

static int __xuiTextEditCursorFromPoint(xui_widget pWidget, xui_text_edit_data_t* pData, float fLocalX, float fLocalY)
{
	xui_text_edit_data_t tResolved;
	xui_rect_t tContent;
	xui_text_edit_line_t* pLine;
	xui_vec2_t tPrefix;
	float fQueryX;
	float fLineQuery;
	float fPrev;
	float fNext;
	float fWrapWidth;
	int iLine;
	int iPos;
	int iNext;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->sText == NULL) ) {
		return 0;
	}
	__xuiTextEditResolve(pWidget, pData, &tResolved);
	tContent = __xuiTextEditTextContentRect(pWidget, &tResolved);
	fWrapWidth = (tContent.fW > 1.0f) ? tContent.fW : 1.0f;
	iRet = __xuiTextEditBuildLines(pWidget, pData, tResolved.pFont, fWrapWidth);
	if ( iRet != XUI_OK || pData->iLineCount <= 0 ) {
		return 0;
	}
	fLineQuery = fLocalY - tContent.fY + pData->fScrollY;
	iLine = (int)(fLineQuery / pData->fLineHeight);
	if ( iLine < 0 ) iLine = 0;
	if ( iLine >= pData->iLineCount ) iLine = pData->iLineCount - 1;
	pLine = &pData->pLines[iLine];
	fQueryX = fLocalX - tContent.fX + pData->fScrollX;
	if ( fQueryX <= 0.0f ) {
		return pLine->iStart;
	}
	iPos = pLine->iStart;
	fPrev = 0.0f;
	while ( iPos < pLine->iEnd ) {
		iNext = __xuiTextEditUtf8Next(pData->sText, pLine->iEnd, iPos);
		tPrefix = __xuiTextEditMeasureRange(pWidget, pData, tResolved.pFont, pLine->iStart, iNext);
		fNext = tPrefix.fX;
		if ( fQueryX < (fPrev + fNext) * 0.5f ) {
			return iPos;
		}
		iPos = iNext;
		fPrev = fNext;
	}
	return pLine->iEnd;
}

static int __xuiTextEditHitSetCursor(xui_widget pWidget, xui_text_edit_data_t* pData, const xui_event_t* pEvent, int bExtend)
{
	xui_rect_t tWorld;
	int iCursor;

	if ( (pWidget == NULL) || (pData == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tWorld = xuiWidgetGetWorldRect(pWidget);
	iCursor = __xuiTextEditCursorFromPoint(pWidget, pData, pEvent->fX - tWorld.fX, pEvent->fY - tWorld.fY);
	return __xuiTextEditMoveCursor(pWidget, pData, iCursor, bExtend);
}

static int __xuiTextEditDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fRadius, uint32_t iColor)
{
	if ( __xuiTextEditAlpha(iColor) == 0 ) {
		return XUI_OK;
	}
	tRect = xuiInternalSnapRect(tRect);
	if ( (fRadius > 0.0f) && (pProxy->drawRoundRectFill != NULL) ) {
		return pProxy->drawRoundRectFill(pProxy, pDraw, tRect, xuiInternalSnapPixel(fRadius), iColor);
	}
	return (pProxy->drawRectFill != NULL) ? pProxy->drawRectFill(pProxy, pDraw, tRect, iColor) : XUI_OK;
}

static int __xuiTextEditDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fRadius, float fWidth, uint32_t iColor)
{
	if ( (fWidth <= 0.0f) || (__xuiTextEditAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	tRect = xuiInternalStrokeCenterRectInside(tRect, fWidth, &fRadius);
	if ( (fRadius > 0.0f) && (pProxy->drawRoundRectStroke != NULL) ) {
		return pProxy->drawRoundRectStroke(pProxy, pDraw, tRect, fRadius, xuiInternalSnapSize(fWidth), iColor);
	}
	return (pProxy->drawRectStroke != NULL) ? pProxy->drawRectStroke(pProxy, pDraw, tRect, xuiInternalSnapSize(fWidth), iColor) : XUI_OK;
}

static int __xuiTextEditContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_text_edit_data_t* pData;
	xui_text_edit_data_t tResolved;
	xui_proxy pProxy;
	xui_font_metrics_t tMetrics;

	(void)tConstraint;
	(void)pUser;
	if ( (pWidget == NULL) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiTextEditResolve(pWidget, pData, &tResolved);
	pSize->fX = 320.0f;
	pSize->fY = 120.0f;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy != NULL) && (pProxy->fontGetMetrics != NULL) && (tResolved.pFont != NULL) &&
	     (pProxy->fontGetMetrics(pProxy, tResolved.pFont, &tMetrics) == XUI_OK) &&
	     (tMetrics.fLineHeight > 0.0f) ) {
		pSize->fY = tMetrics.fLineHeight * 6.0f + tResolved.fLineGap * 5.0f;
	}
	pSize->fX += __xuiTextEditLineNumberWidth(&tResolved);
	if ( pSize->fY < 90.0f ) {
		pSize->fY = 90.0f;
	}
	return XUI_OK;
}

static int __xuiTextEditLineStartsPhysicalLine(xui_text_edit_data_t* pData, int iLine)
{
	if ( (pData == NULL) || (iLine <= 0) || (iLine >= pData->iLineCount) ) {
		return 1;
	}
	return pData->pLines[iLine].iStart > pData->pLines[iLine - 1].iEnd;
}

static int __xuiTextEditLineNumberForLine(xui_text_edit_data_t* pData, int iLine)
{
	int iPos;
	int iLimit;
	int iNumber;

	if ( (pData == NULL) || (pData->sText == NULL) || (iLine < 0) || (iLine >= pData->iLineCount) ) {
		return 1;
	}
	iLimit = pData->pLines[iLine].iStart;
	iNumber = 1;
	for ( iPos = 0; (iPos < iLimit) && (pData->sText[iPos] != '\0'); iPos++ ) {
		if ( pData->sText[iPos] == '\n' ) {
			iNumber++;
		}
	}
	return iNumber;
}

static int __xuiTextEditDrawLineNumbers(xui_widget pWidget, xui_draw_context pDraw, xui_proxy pProxy, xui_text_edit_data_t* pData, xui_text_edit_data_t* pResolved, xui_rect_t tTextContent)
{
	xui_rect_t tGutter;
	xui_rect_t tBorder;
	xui_rect_t tLine;
	xui_text_edit_line_t* pLine;
	char sNumber[32];
	float fY;
	int i;
	int iRet;

	tGutter = __xuiTextEditLineNumberRect(pWidget, pResolved);
	if ( (tGutter.fW <= 0.0f) || (tGutter.fH <= 0.0f) ) {
		return XUI_OK;
	}
	iRet = __xuiTextEditDrawRectFill(pProxy, pDraw, tGutter, 0.0f, pResolved->iLineNumberBackgroundColor);
	if ( iRet != XUI_OK ) return iRet;
	tBorder = (xui_rect_t){tGutter.fX + tGutter.fW - 1.0f, tGutter.fY, 1.0f, tGutter.fH};
	iRet = __xuiTextEditDrawRectFill(pProxy, pDraw, tBorder, 0.0f, pResolved->iLineNumberBorderColor);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pResolved->pFont == NULL) || (pProxy->drawText == NULL) ) {
		return XUI_OK;
	}
	for ( i = 0; i < pData->iLineCount; i++ ) {
		pLine = &pData->pLines[i];
		fY = tTextContent.fY + pLine->fY - pData->fScrollY;
		if ( fY + pData->fLineHeight < tTextContent.fY ) {
			continue;
		}
		if ( fY > tTextContent.fY + tTextContent.fH ) {
			break;
		}
		if ( !__xuiTextEditLineStartsPhysicalLine(pData, i) ) {
			continue;
		}
		snprintf(sNumber, sizeof(sNumber), "%d", __xuiTextEditLineNumberForLine(pData, i));
		tLine = tGutter;
		tLine.fY = fY;
		tLine.fH = pData->fLineHeight;
		tLine.fX += 2.0f;
		tLine.fW -= 8.0f;
		if ( tLine.fW < 0.0f ) {
			tLine.fW = 0.0f;
		}
		iRet = pProxy->drawText(pProxy, pDraw, pResolved->pFont, sNumber, tLine, pResolved->iLineNumberColor,
			XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiTextEditDrawSelectionLine(xui_widget pWidget, xui_draw_context pDraw, xui_proxy pProxy, xui_text_edit_data_t* pData, xui_text_edit_data_t* pResolved, xui_rect_t tContent, int iLine)
{
	xui_text_edit_line_t* pLine;
	xui_vec2_t tPrefix0;
	xui_vec2_t tPrefix1;
	xui_rect_t tSel;
	int iStart;
	int iEnd;
	int iSelStart;
	int iSelEnd;
	float fX0;
	float fX1;

	if ( !__xuiTextEditHasSelectionData(pData) ) {
		return XUI_OK;
	}
	if ( (iLine < 0) || (iLine >= pData->iLineCount) ) {
		return XUI_OK;
	}
	pLine = &pData->pLines[iLine];
	__xuiTextEditSelectionRange(pData, &iStart, &iEnd);
	iSelStart = (iStart > pLine->iStart) ? iStart : pLine->iStart;
	iSelEnd = (iEnd < pLine->iEnd) ? iEnd : pLine->iEnd;
	if ( (iSelEnd <= iSelStart) && !((iStart <= pLine->iEnd) && (iEnd > pLine->iEnd)) ) {
		return XUI_OK;
	}
	tPrefix0 = __xuiTextEditMeasureRange(pWidget, pData, pResolved->pFont, pLine->iStart, iSelStart);
	tPrefix1 = __xuiTextEditMeasureRange(pWidget, pData, pResolved->pFont, pLine->iStart, iSelEnd);
	fX0 = tContent.fX + tPrefix0.fX - pData->fScrollX;
	fX1 = tContent.fX + tPrefix1.fX - pData->fScrollX;
	if ( (iSelEnd == iSelStart) && (iEnd > pLine->iEnd) ) {
		fX1 = fX0 + 4.0f;
	}
	if ( fX0 < tContent.fX ) fX0 = tContent.fX;
	if ( fX1 > tContent.fX + tContent.fW ) fX1 = tContent.fX + tContent.fW;
	if ( fX1 <= fX0 ) {
		return XUI_OK;
	}
	tSel.fX = fX0;
	tSel.fY = tContent.fY + pLine->fY - pData->fScrollY + 1.0f;
	tSel.fW = fX1 - fX0;
	tSel.fH = pData->fLineHeight - 2.0f;
	if ( tSel.fY < tContent.fY ) {
		tSel.fH -= (tContent.fY - tSel.fY);
		tSel.fY = tContent.fY;
	}
	if ( tSel.fY + tSel.fH > tContent.fY + tContent.fH ) {
		tSel.fH = tContent.fY + tContent.fH - tSel.fY;
	}
	if ( tSel.fH <= 0.0f ) {
		return XUI_OK;
	}
	return __xuiTextEditDrawRectFill(pProxy, pDraw, tSel, 2.0f, pResolved->iSelectionColor);
}

static int __xuiTextEditCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_text_edit_data_t* pData;
	xui_text_edit_data_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tRect;
	xui_rect_t tContent;
	xui_rect_t tLine;
	xui_rect_t tCursor;
	xui_text_edit_line_t* pLine;
	char* sLineText;
	uint32_t iState;
	uint32_t iBackground;
	uint32_t iBorder;
	uint32_t iText;
	float fWrapWidth;
	float fY;
	int i;
	int iLen;
	int iRet;

	(void)iStateId;
	(void)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiTextEditResolve(pWidget, pData, &tResolved);
	iState = xuiTextEditGetState(pWidget);
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect = xuiInternalSnapRect(tRect);
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	iBackground = tResolved.iBackgroundColor;
	iBorder = tResolved.iBorderColor;
	if ( (iState & XUI_WIDGET_STATE_DISABLED) != 0 ) {
		iBackground = tResolved.iDisabledBackgroundColor;
		iBorder = __xuiTextEditColorWithAlpha(tResolved.iBorderColor, 120);
	} else if ( (iState & XUI_WIDGET_STATE_FOCUS) != 0 ) {
		iBorder = tResolved.iFocusBorderColor;
	} else if ( (iState & XUI_WIDGET_STATE_HOVER) != 0 ) {
		iBackground = tResolved.iHoverBackgroundColor;
		iBorder = tResolved.iHoverBorderColor;
	}
	iRet = __xuiTextEditDrawRectFill(pProxy, pDraw, tRect, tResolved.fRadius, iBackground);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiTextEditDrawRectStroke(pProxy, pDraw, tRect, tResolved.fRadius, tResolved.fBorderWidth, iBorder);
	if ( iRet != XUI_OK ) return iRet;

	tContent = __xuiTextEditTextContentRect(pWidget, &tResolved);
	if ( (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) {
		return XUI_OK;
	}
	pData->tTextRect = tContent;
	fWrapWidth = (tContent.fW > 1.0f) ? tContent.fW : 1.0f;
	iRet = __xuiTextEditBuildLines(pWidget, pData, tResolved.pFont, fWrapWidth);
	if ( iRet != XUI_OK ) return iRet;
	__xuiTextEditClampScroll(pWidget, pData);
	iRet = __xuiTextEditDrawLineNumbers(pWidget, pDraw, pProxy, pData, &tResolved, tContent);
	if ( iRet != XUI_OK ) return iRet;
	iLen = (pData->sText != NULL) ? (int)strlen(pData->sText) : 0;
	if ( (iLen == 0) && (pData->sPlaceholder != NULL) && (pData->sPlaceholder[0] != '\0') && (tResolved.pFont != NULL) && (pProxy->drawText != NULL) ) {
		iRet = pProxy->drawText(pProxy, pDraw, tResolved.pFont, pData->sPlaceholder, tContent, tResolved.iPlaceholderColor,
			XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( iLen > 0 && (tResolved.pFont != NULL) && (pProxy->drawText != NULL) ) {
		for ( i = 0; i < pData->iLineCount; i++ ) {
			pLine = &pData->pLines[i];
			fY = tContent.fY + pLine->fY - pData->fScrollY;
			if ( fY + pData->fLineHeight < tContent.fY ) {
				continue;
			}
			if ( fY > tContent.fY + tContent.fH ) {
				break;
			}
			iRet = __xuiTextEditDrawSelectionLine(pWidget, pDraw, pProxy, pData, &tResolved, tContent, i);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiTextEditBuildLineText(pData, pLine->iStart, pLine->iEnd, &sLineText);
			if ( iRet != XUI_OK ) return iRet;
			tLine = tContent;
			tLine.fY = fY;
			tLine.fH = pData->fLineHeight;
			if ( !pData->bWordWrap ) {
				tLine.fX = tContent.fX - pData->fScrollX;
				tLine.fW = tContent.fW + pData->fScrollX;
				if ( tLine.fX < tContent.fX ) {
					tLine.fW -= (tContent.fX - tLine.fX);
					tLine.fX = tContent.fX;
				}
			}
			iText = ((iState & XUI_WIDGET_STATE_DISABLED) != 0) ? tResolved.iDisabledTextColor : tResolved.iTextColor;
			iRet = pProxy->drawText(pProxy, pDraw, tResolved.pFont, sLineText, tLine, iText,
				XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	(void)__xuiTextEditUpdateCursorRect(pWidget, pData);
	tCursor = pData->tCursorRect;
	if ( tCursor.fX < tContent.fX ) tCursor.fX = tContent.fX;
	if ( tCursor.fX > tContent.fX + tContent.fW ) tCursor.fX = tContent.fX + tContent.fW;
	if ( tCursor.fY < tContent.fY ) {
		tCursor.fH -= (tContent.fY - tCursor.fY);
		tCursor.fY = tContent.fY;
	}
	if ( tCursor.fY + tCursor.fH > tContent.fY + tContent.fH ) {
		tCursor.fH = tContent.fY + tContent.fH - tCursor.fY;
	}
	if ( ((iState & XUI_WIDGET_STATE_FOCUS) != 0) &&
	     ((iState & XUI_WIDGET_STATE_DISABLED) == 0) &&
	     !__xuiTextEditHasSelectionData(pData) &&
	     (tCursor.fH > 0.0f) &&
	     (pProxy->drawRectFill != NULL) ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, xuiInternalSnapRect(tCursor), tResolved.iCursorColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiTextEditUpdateMenu(xui_widget pWidget, xui_text_edit_data_t* pData)
{
	xui_menu_item_t arrItems[8];
	uint32_t iEnabled;
	int bHasSelection;
	int bReadonly;
	int bAllSelected;
	int iLen;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->pMenu == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(arrItems, 0, sizeof(arrItems));
	bHasSelection = __xuiTextEditHasSelectionData(pData);
	bReadonly = pData->bReadonly;
	iLen = (pData->sText != NULL) ? (int)strlen(pData->sText) : 0;
	bAllSelected = bHasSelection && (pData->iSelectStart == 0 || pData->iSelectEnd == 0) &&
	               (pData->iSelectStart == iLen || pData->iSelectEnd == iLen);
	iEnabled = XUI_MENU_ITEM_ENABLED;
	arrItems[0].sText = xuiTextEditGetMenuTitle(pWidget, XUI_INPUT_MENU_UNDO);
	arrItems[0].sShortcut = "Ctrl+Z";
	arrItems[0].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[0].iState = (pData->iUndoCount > 0) ? iEnabled : 0u;
	arrItems[0].iValue = XUI_INPUT_MENU_UNDO;
	arrItems[1].iType = XUI_MENU_ITEM_SEPARATOR;
	arrItems[2].sText = xuiTextEditGetMenuTitle(pWidget, XUI_INPUT_MENU_CUT);
	arrItems[2].sShortcut = "Ctrl+X";
	arrItems[2].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[2].iState = (bHasSelection && !bReadonly) ? iEnabled : 0u;
	arrItems[2].iValue = XUI_INPUT_MENU_CUT;
	arrItems[3].sText = xuiTextEditGetMenuTitle(pWidget, XUI_INPUT_MENU_COPY);
	arrItems[3].sShortcut = "Ctrl+C";
	arrItems[3].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[3].iState = bHasSelection ? iEnabled : 0u;
	arrItems[3].iValue = XUI_INPUT_MENU_COPY;
	arrItems[4].sText = xuiTextEditGetMenuTitle(pWidget, XUI_INPUT_MENU_PASTE);
	arrItems[4].sShortcut = "Ctrl+V";
	arrItems[4].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[4].iState = !bReadonly ? iEnabled : 0u;
	arrItems[4].iValue = XUI_INPUT_MENU_PASTE;
	arrItems[5].sText = xuiTextEditGetMenuTitle(pWidget, XUI_INPUT_MENU_DELETE);
	arrItems[5].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[5].iState = (bHasSelection && !bReadonly) ? iEnabled : 0u;
	arrItems[5].iValue = XUI_INPUT_MENU_DELETE;
	arrItems[6].iType = XUI_MENU_ITEM_SEPARATOR;
	arrItems[7].sText = xuiTextEditGetMenuTitle(pWidget, XUI_INPUT_MENU_SELECT_ALL);
	arrItems[7].sShortcut = "Ctrl+A";
	arrItems[7].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[7].iState = (iLen > 0 && !bAllSelected) ? iEnabled : 0u;
	arrItems[7].iValue = XUI_INPUT_MENU_SELECT_ALL;
	return xuiMenuSetItems(pData->pMenu, arrItems, 8);
}

static void __xuiTextEditMenuSelect(xui_widget pMenu, int iIndex, int iValue, void* pUser)
{
	xui_widget pTextEdit;

	(void)pMenu;
	(void)iIndex;
	pTextEdit = (xui_widget)pUser;
	if ( pTextEdit == NULL ) {
		return;
	}
	(void)xuiSetFocusWidget(xuiWidgetGetContext(pTextEdit), pTextEdit);
	switch ( iValue ) {
	case XUI_INPUT_MENU_UNDO:
		(void)xuiTextEditUndo(pTextEdit);
		break;
	case XUI_INPUT_MENU_CUT:
		(void)xuiTextEditCut(pTextEdit);
		break;
	case XUI_INPUT_MENU_COPY:
		(void)xuiTextEditCopy(pTextEdit);
		break;
	case XUI_INPUT_MENU_PASTE:
		(void)xuiTextEditPaste(pTextEdit);
		break;
	case XUI_INPUT_MENU_DELETE:
		(void)xuiTextEditDeleteSelection(pTextEdit);
		break;
	case XUI_INPUT_MENU_SELECT_ALL:
		(void)xuiTextEditSelectAll(pTextEdit);
		break;
	default:
		break;
	}
}

static int __xuiTextEditHandleKey(xui_widget pWidget, xui_text_edit_data_t* pData, const xui_event_t* pEvent)
{
	int iLen;
	int iStart;
	int iEnd;
	int iKey;
	int bShift;
	int bCtrl;

	if ( (pWidget == NULL) || (pData == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iLen = (pData->sText != NULL) ? (int)strlen(pData->sText) : 0;
	iKey = pEvent->iKey;
	bShift = ((pEvent->iModifiers & XUI_MOD_SHIFT) != 0);
	bCtrl = ((pEvent->iModifiers & XUI_MOD_CTRL) != 0);
	if ( bCtrl ) {
		iKey = toupper((unsigned char)iKey);
		if ( iKey == 'A' ) {
			(void)xuiTextEditSelectAll(pWidget);
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( iKey == 'C' ) {
			(void)xuiTextEditCopy(pWidget);
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( iKey == 'X' ) {
			(void)xuiTextEditCut(pWidget);
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( iKey == 'V' ) {
			(void)xuiTextEditPaste(pWidget);
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( iKey == 'Z' ) {
			(void)xuiTextEditUndo(pWidget);
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( iKey == 'Y' ) {
			(void)xuiTextEditRedo(pWidget);
			return XUI_EVENT_DISPATCH_STOP;
		}
	}
	switch ( pEvent->iKey ) {
	case XUI_KEY_LEFT:
		if ( __xuiTextEditHasSelectionData(pData) && !bShift ) {
			__xuiTextEditSelectionRange(pData, &iStart, &iEnd);
			return __xuiTextEditMoveCursor(pWidget, pData, iStart, 0) == XUI_OK ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
		}
		(void)__xuiTextEditMoveCursor(pWidget, pData,
			bCtrl ? __xuiTextEditPrevWord(pData) : __xuiTextEditUtf8Prev(pData->sText, iLen, pData->iCursor), bShift);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_RIGHT:
		if ( __xuiTextEditHasSelectionData(pData) && !bShift ) {
			__xuiTextEditSelectionRange(pData, &iStart, &iEnd);
			return __xuiTextEditMoveCursor(pWidget, pData, iEnd, 0) == XUI_OK ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
		}
		(void)__xuiTextEditMoveCursor(pWidget, pData,
			bCtrl ? __xuiTextEditNextWord(pData) : __xuiTextEditUtf8Next(pData->sText, iLen, pData->iCursor), bShift);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_UP:
		(void)__xuiTextEditMoveCursorVertical(pWidget, pData, -1, bShift);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_DOWN:
		(void)__xuiTextEditMoveCursorVertical(pWidget, pData, 1, bShift);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_PAGE_UP:
		(void)__xuiTextEditMoveCursorVertical(pWidget, pData, -5, bShift);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_PAGE_DOWN:
		(void)__xuiTextEditMoveCursorVertical(pWidget, pData, 5, bShift);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_HOME:
		if ( bCtrl ) {
			(void)__xuiTextEditMoveCursor(pWidget, pData, 0, bShift);
		} else {
			(void)__xuiTextEditMoveCursor(pWidget, pData, __xuiTextEditLineStartForCursor(pWidget, pData), bShift);
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_END:
		if ( bCtrl ) {
			(void)__xuiTextEditMoveCursor(pWidget, pData, iLen, bShift);
		} else {
			(void)__xuiTextEditMoveCursor(pWidget, pData, __xuiTextEditLineEndForCursor(pWidget, pData), bShift);
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_ENTER:
		if ( !pData->bReadonly ) {
			(void)__xuiTextEditInsertText(pWidget, pData, "\n", 1);
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_TEXT_EDIT_KEY_BACKSPACE:
		if ( pData->bReadonly ) return XUI_EVENT_DISPATCH_STOP;
		if ( __xuiTextEditHasSelectionData(pData) ) {
			(void)xuiTextEditDeleteSelection(pWidget);
		} else if ( pData->iCursor > 0 ) {
			(void)__xuiTextEditDeleteRange(pWidget, pData,
				bCtrl ? __xuiTextEditPrevWord(pData) : __xuiTextEditUtf8Prev(pData->sText, iLen, pData->iCursor),
				pData->iCursor, 1, 1);
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_TEXT_EDIT_KEY_DELETE:
		if ( pData->bReadonly ) return XUI_EVENT_DISPATCH_STOP;
		if ( __xuiTextEditHasSelectionData(pData) ) {
			(void)xuiTextEditDeleteSelection(pWidget);
		} else if ( pData->iCursor < iLen ) {
			(void)__xuiTextEditDeleteRange(pWidget, pData, pData->iCursor,
				bCtrl ? __xuiTextEditNextWord(pData) : __xuiTextEditUtf8Next(pData->sText, iLen, pData->iCursor), 1, 1);
		}
		return XUI_EVENT_DISPATCH_STOP;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiTextEditEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_text_edit_data_t* pData;
	xui_context pContext;
	char sText[8];
	int iTextSize;
	xui_rect_t tWorld;
	float fX;
	float fY;
	int iCursor;

	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_LEAVE:
	case XUI_EVENT_ENABLED_CHANGED:
	case XUI_EVENT_VISIBLE_CHANGED:
		return __xuiTextEditInvalidatePaint(pWidget);
	case XUI_EVENT_FOCUS:
	case XUI_EVENT_BLUR:
		return __xuiTextEditInvalidateText(pWidget);
	case XUI_EVENT_BOUNDS_CHANGED:
		__xuiTextEditMarkLinesDirty(pData);
		return __xuiTextEditInvalidateText(pWidget);
	case XUI_EVENT_POINTER_DOWN:
		if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT || pEvent->iButton == 0 ) {
			(void)xuiSetFocusWidget(pContext, pWidget);
			(void)xuiSetPointerCapture(pContext, pWidget);
			pData->bDragging = 1;
			(void)__xuiTextEditHitSetCursor(pWidget, pData, pEvent, ((pEvent->iModifiers & XUI_MOD_SHIFT) != 0));
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_MOVE:
		if ( pData->bDragging ) {
			(void)__xuiTextEditHitSetCursor(pWidget, pData, pEvent, 1);
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_UP:
		if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT || pEvent->iButton == 0 ) {
			pData->bDragging = 0;
			(void)xuiReleasePointerCapture(pContext, pWidget);
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_WHEEL:
		(void)xuiTextEditScrollBy(pWidget, -pEvent->fWheelX * 24.0f, -pEvent->fWheelY * (pData->fLineHeight > 0.0f ? pData->fLineHeight * 3.0f : 48.0f));
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		pData->bDragging = 0;
		return XUI_OK;
	case XUI_EVENT_POINTER_DOUBLE_CLICK:
		if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT || pEvent->iButton == 0 ) {
			tWorld = xuiWidgetGetWorldRect(pWidget);
			iCursor = __xuiTextEditCursorFromPoint(pWidget, pData, pEvent->fX - tWorld.fX, pEvent->fY - tWorld.fY);
			(void)__xuiTextEditSelectWordAt(pWidget, pData, iCursor);
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_CONTEXT_MENU:
		(void)xuiSetFocusWidget(pContext, pWidget);
		if ( pEvent->iKey != XUI_KEY_CONTEXT_MENU && !__xuiTextEditHasSelectionData(pData) ) {
			(void)__xuiTextEditHitSetCursor(pWidget, pData, pEvent, 0);
		}
		if ( pEvent->iKey == XUI_KEY_CONTEXT_MENU ) {
			tWorld = xuiWidgetGetWorldRect(pWidget);
			fX = tWorld.fX + pData->tCursorRect.fX;
			fY = tWorld.fY + pData->tCursorRect.fY + pData->tCursorRect.fH;
		} else {
			fX = pEvent->fX;
			fY = pEvent->fY;
		}
		(void)xuiTextEditOpenMenu(pWidget, fX, fY);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_KEY_DOWN:
		return __xuiTextEditHandleKey(pWidget, pData, pEvent);
	case XUI_EVENT_TEXT:
		if ( pData->bReadonly || ((pEvent->iModifiers & XUI_MOD_CTRL) != 0) ) {
			return XUI_EVENT_DISPATCH_STOP;
		}
		iTextSize = 0;
		if ( pEvent->iTextSize > 0 && pEvent->sText[0] != '\0' ) {
			iTextSize = pEvent->iTextSize;
			if ( iTextSize > (int)sizeof(sText) - 1 ) {
				iTextSize = (int)sizeof(sText) - 1;
			}
			memcpy(sText, pEvent->sText, (size_t)iTextSize);
		} else if ( pEvent->iCodepoint >= 32u ) {
			iTextSize = __xuiTextEditUtf8Encode(pEvent->iCodepoint, sText);
		}
		if ( iTextSize > 0 ) {
			sText[iTextSize] = '\0';
			(void)__xuiTextEditInsertText(pWidget, pData, sText, iTextSize);
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_IME_COMPOSITION:
		if ( pData->bReadonly ) {
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pEvent->iTextSize > 0 && pEvent->sText[0] != '\0' ) {
			(void)__xuiTextEditInsertText(pWidget, pData, pEvent->sText, pEvent->iTextSize);
		}
		return __xuiTextEditInvalidatePaint(pWidget) == XUI_OK ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
	default:
		break;
	}
	return XUI_OK;
}

static xui_rect_t __xuiTextEditImeRect(xui_widget pWidget, void* pUser)
{
	xui_text_edit_data_t* pData;
	xui_rect_t tWorld;
	xui_rect_t tRect;

	(void)pUser;
	pData = __xuiTextEditGetData(pWidget);
	memset(&tRect, 0, sizeof(tRect));
	if ( pData == NULL ) {
		return tRect;
	}
	(void)__xuiTextEditUpdateCursorRect(pWidget, pData);
	tWorld = xuiWidgetGetWorldRect(pWidget);
	tRect = pData->tCursorRect;
	tRect.fX += tWorld.fX;
	tRect.fY += tWorld.fY + tRect.fH;
	tRect.fH = 1.0f;
	return tRect;
}

static void __xuiTextEditDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_MANUAL;
	pLayout->iWidthMode = XUI_SIZE_CONTENT;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_HIDDEN;
	pLayout->iAlignX = XUI_ALIGN_START;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static void __xuiTextEditDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiTextEditInitEvents(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_ENTER, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_UP, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_WHEEL, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOUBLE_CLICK, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_CONTEXT_MENU, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_TEXT, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_IME_COMPOSITION, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiTextEditEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BOUNDS_CHANGED, __xuiTextEditEvent, NULL);
	return iRet;
}

static int __xuiTextEditInitMenu(xui_widget pWidget, xui_text_edit_data_t* pData)
{
	xui_menu_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pOwner = pWidget;
	tDesc.pFont = pData->pFont;
	iRet = xuiMenuCreate(xuiWidgetGetContext(pWidget), &pData->pMenu, &tDesc);
	if ( iRet != XUI_OK ) {
		pData->pMenu = NULL;
		return iRet;
	}
	iRet = xuiMenuSetSelect(pData->pMenu, __xuiTextEditMenuSelect, pWidget);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	return __xuiTextEditUpdateMenu(pWidget, pData);
}

static int __xuiTextEditInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_text_edit_data_t* pData;
	const xui_text_edit_desc_t* pDesc;
	xui_theme_t tTheme;
	xui_thickness_t tPadding;
	xui_context pContext;
	int iRet;

	(void)pUser;
	pData = (xui_text_edit_data_t*)pTypeData;
	pDesc = (const xui_text_edit_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiTextEditDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	memset(&tTheme, 0, sizeof(tTheme));
	tTheme.iSize = sizeof(tTheme);
	(void)xuiGetTheme(pContext, &tTheme);
	pData->pFont = (pDesc != NULL && pDesc->pFont != NULL) ? pDesc->pFont : xuiGetDefaultFont(pContext);
	pData->iMaxLength = (pDesc != NULL) ? pDesc->iMaxLength : 0;
	pData->bReadonly = (pDesc != NULL) ? (pDesc->bReadonly != 0) : 0;
	pData->bWordWrap = (pDesc != NULL) ? (pDesc->bWordWrap != 0) : 1;
	pData->bLineNumbers = (pDesc != NULL) ? (pDesc->bLineNumbers != 0) : 0;
	pData->fLineNumberWidth = (pDesc != NULL && pDesc->fLineNumberWidth > 0.0f) ? pDesc->fLineNumberWidth : 44.0f;
	pData->iTextColor = (pDesc != NULL && pDesc->iTextColor != 0) ? pDesc->iTextColor : XUI_COLOR_RGBA(31, 41, 55, 255);
	pData->iPlaceholderColor = (pDesc != NULL && pDesc->iPlaceholderColor != 0) ? pDesc->iPlaceholderColor : XUI_COLOR_RGBA(135, 148, 166, 255);
	pData->iDisabledTextColor = (pDesc != NULL && pDesc->iDisabledTextColor != 0) ? pDesc->iDisabledTextColor : XUI_COLOR_RGBA(150, 160, 172, 255);
	pData->iBackgroundColor = (pDesc != NULL && pDesc->iBackgroundColor != 0) ? pDesc->iBackgroundColor : XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iHoverBackgroundColor = (pDesc != NULL && pDesc->iHoverBackgroundColor != 0) ? pDesc->iHoverBackgroundColor : XUI_COLOR_RGBA(249, 252, 255, 255);
	pData->iDisabledBackgroundColor = (pDesc != NULL && pDesc->iDisabledBackgroundColor != 0) ? pDesc->iDisabledBackgroundColor : XUI_COLOR_RGBA(242, 245, 249, 255);
	pData->iBorderColor = (pDesc != NULL && pDesc->iBorderColor != 0) ? pDesc->iBorderColor : XUI_COLOR_RGBA(176, 190, 207, 255);
	pData->iHoverBorderColor = (pDesc != NULL && pDesc->iHoverBorderColor != 0) ? pDesc->iHoverBorderColor : XUI_COLOR_RGBA(105, 166, 226, 255);
	pData->iFocusBorderColor = (pDesc != NULL && pDesc->iFocusBorderColor != 0) ? pDesc->iFocusBorderColor : XUI_COLOR_RGBA(47, 128, 237, 255);
	pData->iSelectionColor = (pDesc != NULL && pDesc->iSelectionColor != 0) ? pDesc->iSelectionColor : XUI_COLOR_RGBA(47, 128, 237, 78);
	pData->iCursorColor = (pDesc != NULL && pDesc->iCursorColor != 0) ? pDesc->iCursorColor : XUI_COLOR_RGBA(33, 94, 170, 255);
	pData->iLineNumberColor = (pDesc != NULL && pDesc->iLineNumberColor != 0) ? pDesc->iLineNumberColor : XUI_COLOR_RGBA(112, 129, 150, 255);
	pData->iLineNumberBackgroundColor = (pDesc != NULL && pDesc->iLineNumberBackgroundColor != 0) ? pDesc->iLineNumberBackgroundColor : XUI_COLOR_RGBA(246, 249, 253, 255);
	pData->iLineNumberBorderColor = (pDesc != NULL && pDesc->iLineNumberBorderColor != 0) ? pDesc->iLineNumberBorderColor : XUI_COLOR_RGBA(213, 224, 238, 255);
	pData->fRadius = (pDesc != NULL && pDesc->fRadius > 0.0f) ? pDesc->fRadius : 4.0f;
	pData->fBorderWidth = (pDesc != NULL && pDesc->fBorderWidth > 0.0f) ? pDesc->fBorderWidth : 1.0f;
	pData->fLineGap = (pDesc != NULL && pDesc->fLineGap > 0.0f) ? pDesc->fLineGap : 2.0f;
	pData->fLastLayoutWidth = -1.0f;
	pData->bLinesDirty = 1;
	iRet = __xuiTextEditAssignTextBytes(pData, (pDesc != NULL) ? pDesc->sText : "", -1);
	if ( iRet != XUI_OK ) return iRet;
	pData->iCursor = (int)strlen(pData->sText);
	(void)__xuiTextEditClearSelectionData(pData);
	iRet = __xuiTextEditStringSet(&pData->sPlaceholder, &pData->iPlaceholderCapacity, (pDesc != NULL) ? pDesc->sPlaceholder : "");
	if ( iRet != XUI_OK ) return iRet;
	tPadding = (xui_thickness_t){8.0f, 6.0f, 8.0f, 6.0f};
	(void)xuiWidgetSetPadding(pWidget, tPadding);
	(void)xuiWidgetSetFocusable(pWidget, 1);
	(void)xuiWidgetSetTabStop(pWidget, 1);
	(void)xuiWidgetSetImeMode(pWidget, XUI_IME_AUTO);
	(void)xuiWidgetSetImeCandidateRect(pWidget, __xuiTextEditImeRect, NULL);
	iRet = __xuiTextEditInitEvents(pWidget);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiTextEditInitMenu(pWidget, pData);
}

static void __xuiTextEditDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_text_edit_data_t* pData;
	xui_widget pPopup;
	int i;

	(void)pUser;
	pData = (xui_text_edit_data_t*)pTypeData;
	if ( pData == NULL ) {
		return;
	}
	if ( pData->pMenu != NULL ) {
		pPopup = xuiMenuGetPopupWidget(pData->pMenu);
		if ( pPopup != NULL ) {
			xuiWidgetDestroy(pPopup);
		} else {
			xuiWidgetDestroy(pData->pMenu);
		}
		pData->pMenu = NULL;
	}
	if ( pData->sText != NULL ) xrtFree(pData->sText);
	if ( pData->sPlaceholder != NULL ) xrtFree(pData->sPlaceholder);
	if ( pData->sScratch != NULL ) xrtFree(pData->sScratch);
	if ( pData->sLineText != NULL ) xrtFree(pData->sLineText);
	if ( pData->pLines != NULL ) xrtFree(pData->pLines);
	for ( i = 0; i < XUI_INPUT_MENU_COUNT; i++ ) {
		if ( pData->arrMenuTitle[i] != NULL ) {
			xrtFree(pData->arrMenuTitle[i]);
		}
	}
	__xuiTextEditHistoryClear(pData->arrUndo, &pData->iUndoCount);
	__xuiTextEditHistoryClear(pData->arrRedo, &pData->iRedoCount);
	if ( (pWidget != NULL) && (xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget) ) {
		(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
	}
	memset(pData, 0, sizeof(*pData));
}

static void __xuiTextEditRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
{
	xui_style_property_info_t tInfo;

	if ( xuiStyleFindProperty(pContext, sName) != 0 ) {
		return;
	}
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.sName = sName;
	tInfo.iValueType = iValueType;
	tInfo.iDirtyFlags = iDirtyFlags;
	tInfo.iFlags = iFlags;
	tInfo.pWidgetType = pType;
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
}

static void __xuiTextEditRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	uint32_t iLayoutDirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;

	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.text.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.placeholder.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.text.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.background.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.background.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.border.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.border.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.border.focus_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.selection.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.cursor.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.line_number.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.line_number.background_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.line_number.border_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.radius", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.border.width", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.line_gap", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "textedit.line_number.width", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiTextEditRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, iLayoutDirty, XUI_STYLE_PROPERTY_INHERITED);
}

XUI_API xui_widget_type xuiTextEditGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "textedit");
	if ( pType != NULL ) {
		__xuiTextEditRegisterStyleProperties(pContext, pType);
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "textedit";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_text_edit_data_t);
	tDesc.onInit = __xuiTextEditInit;
	tDesc.onDestroy = __xuiTextEditDestroy;
	tDesc.onContentMeasure = __xuiTextEditContentMeasure;
	tDesc.onCacheRender = __xuiTextEditCacheRender;
	__xuiTextEditDefaultLayout(&tDesc.tLayout);
	__xuiTextEditDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	__xuiTextEditRegisterStyleProperties(pContext, pType);
	return pType;
}

XUI_API int xuiTextEditCreate(xui_context pContext, xui_widget* ppWidget, const xui_text_edit_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( (ppWidget == NULL) || !__xuiTextEditDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppWidget = NULL;
	pType = xuiTextEditGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiTextEditSetChange(xui_widget pWidget, xui_text_edit_change_proc onChange, void* pUser)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onChange = onChange;
	pData->pChangeUser = pUser;
	return XUI_OK;
}

XUI_API int xuiTextEditSetText(xui_widget pWidget, const char* sText)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTextEditHistoryClear(pData->arrUndo, &pData->iUndoCount);
	__xuiTextEditHistoryClear(pData->arrRedo, &pData->iRedoCount);
	return __xuiTextEditSetTextInternal(pWidget, pData, sText, 0, 0);
}

XUI_API const char* xuiTextEditGetText(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	return (pData != NULL && pData->sText != NULL) ? pData->sText : "";
}

XUI_API int xuiTextEditSetPlaceholder(xui_widget pWidget, const char* sText)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiTextEditStringSet(&pData->sPlaceholder, &pData->iPlaceholderCapacity, sText);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiTextEditInvalidatePaint(pWidget);
}

XUI_API const char* xuiTextEditGetPlaceholder(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	return (pData != NULL && pData->sPlaceholder != NULL) ? pData->sPlaceholder : "";
}

XUI_API int xuiTextEditSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->pFont = pFont;
	if ( pData->pMenu != NULL ) {
		(void)xuiMenuSetFont(pData->pMenu, pFont);
	}
	return __xuiTextEditInvalidateText(pWidget);
}

XUI_API xui_font xuiTextEditGetFont(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API int xuiTextEditSetMaxLength(xui_widget pWidget, int iMaxLength)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( (pData == NULL) || (iMaxLength < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->iMaxLength = iMaxLength;
	if ( iMaxLength > 0 ) {
		(void)__xuiTextEditAssignTextBytes(pData, pData->sText, -1);
	}
	return __xuiTextEditInvalidateText(pWidget);
}

XUI_API int xuiTextEditGetMaxLength(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	return (pData != NULL) ? pData->iMaxLength : 0;
}

XUI_API int xuiTextEditSetReadonly(xui_widget pWidget, int bReadonly)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->bReadonly = bReadonly != 0;
	return __xuiTextEditInvalidatePaint(pWidget);
}

XUI_API int xuiTextEditIsReadonly(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	return (pData != NULL) ? pData->bReadonly : 0;
}

XUI_API int xuiTextEditSetWordWrap(xui_widget pWidget, int bWordWrap)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->bWordWrap = bWordWrap != 0;
	pData->fScrollX = 0.0f;
	return __xuiTextEditInvalidateText(pWidget);
}

XUI_API int xuiTextEditGetWordWrap(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	return (pData != NULL) ? pData->bWordWrap : 0;
}

XUI_API int xuiTextEditSetLineNumbers(xui_widget pWidget, int bVisible, float fWidth)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( (pData == NULL) || (fWidth < 0.0f) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->bLineNumbers = bVisible != 0;
	if ( fWidth > 0.0f ) {
		pData->fLineNumberWidth = fWidth;
	}
	pData->fScrollX = 0.0f;
	__xuiTextEditMarkLinesDirty(pData);
	return __xuiTextEditInvalidateText(pWidget);
}

XUI_API int xuiTextEditGetLineNumbers(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	return (pData != NULL) ? pData->bLineNumbers : 0;
}

XUI_API float xuiTextEditGetLineNumberWidth(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	return (pData != NULL) ? pData->fLineNumberWidth : 0.0f;
}

XUI_API int xuiTextEditSetLineNumberColors(xui_widget pWidget, uint32_t iText, uint32_t iBackground, uint32_t iBorder)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iText != 0 ) pData->iLineNumberColor = iText;
	if ( iBackground != 0 ) pData->iLineNumberBackgroundColor = iBackground;
	if ( iBorder != 0 ) pData->iLineNumberBorderColor = iBorder;
	return __xuiTextEditInvalidatePaint(pWidget);
}

XUI_API int xuiTextEditSetSelection(xui_widget pWidget, int iStart, int iEnd)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiTextEditSetSelectionData(pData, iStart, iEnd);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiTextEditSyncCursor(pWidget, pData);
}

XUI_API int xuiTextEditGetSelection(xui_widget pWidget, int* pStart, int* pEnd)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTextEditSelectionRange(pData, pStart, pEnd);
	return XUI_OK;
}

XUI_API int xuiTextEditSelectAll(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiTextEditSetSelection(pWidget, 0, (int)strlen(pData->sText));
}

XUI_API int xuiTextEditHasSelection(xui_widget pWidget)
{
	return __xuiTextEditHasSelectionData(__xuiTextEditGetData(pWidget));
}

XUI_API int xuiTextEditCopy(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiTextEditCopySelection(pWidget, pData);
}

XUI_API int xuiTextEditCut(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->bReadonly ) return XUI_OK;
	(void)__xuiTextEditCopySelection(pWidget, pData);
	return xuiTextEditDeleteSelection(pWidget);
}

XUI_API int xuiTextEditPaste(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiTextEditPasteClipboard(pWidget, pData);
}

XUI_API int xuiTextEditDeleteSelection(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	int iStart;
	int iEnd;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->bReadonly || !__xuiTextEditHasSelectionData(pData) ) return XUI_OK;
	__xuiTextEditSelectionRange(pData, &iStart, &iEnd);
	return __xuiTextEditDeleteRange(pWidget, pData, iStart, iEnd, 1, 1);
}

static int __xuiTextEditApplyHistoryState(xui_widget pWidget, xui_text_edit_data_t* pData, const xui_text_edit_history_t* pState)
{
	int iRet;
	int iLen;

	if ( (pWidget == NULL) || (pData == NULL) || (pState == NULL) || (pState->sText == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiTextEditAssignTextBytes(pData, pState->sText, -1);
	if ( iRet != XUI_OK ) return iRet;
	iLen = (int)strlen(pData->sText);
	pData->iCursor = __xuiTextEditUtf8Clamp(pData->sText, iLen, pState->iCursor);
	pData->iSelectStart = __xuiTextEditUtf8Clamp(pData->sText, iLen, pState->iSelectStart);
	pData->iSelectEnd = __xuiTextEditUtf8Clamp(pData->sText, iLen, pState->iSelectEnd);
	pData->iAnchor = pData->iSelectStart;
	pData->fScrollX = pState->fScrollX;
	pData->fScrollY = pState->fScrollY;
	(void)__xuiTextEditEnsureCursorVisible(pWidget, pData);
	__xuiTextEditNotifyChange(pWidget, pData);
	return __xuiTextEditInvalidateText(pWidget);
}

XUI_API int xuiTextEditUndo(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	xui_text_edit_history_t tCurrent;
	xui_text_edit_history_t tUndo;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iUndoCount <= 0 ) return XUI_OK;
	iRet = __xuiTextEditHistoryCapture(pData, &tCurrent);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiTextEditHistoryPush(pData->arrRedo, &pData->iRedoCount, &tCurrent);
	if ( iRet != XUI_OK ) {
		__xuiTextEditHistoryStateFree(&tCurrent);
		return iRet;
	}
	tUndo = pData->arrUndo[pData->iUndoCount - 1];
	memset(&pData->arrUndo[pData->iUndoCount - 1], 0, sizeof(pData->arrUndo[0]));
	pData->iUndoCount--;
	iRet = __xuiTextEditApplyHistoryState(pWidget, pData, &tUndo);
	__xuiTextEditHistoryStateFree(&tUndo);
	return iRet;
}

XUI_API int xuiTextEditRedo(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	xui_text_edit_history_t tCurrent;
	xui_text_edit_history_t tRedo;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iRedoCount <= 0 ) return XUI_OK;
	iRet = __xuiTextEditHistoryCapture(pData, &tCurrent);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiTextEditHistoryPush(pData->arrUndo, &pData->iUndoCount, &tCurrent);
	if ( iRet != XUI_OK ) {
		__xuiTextEditHistoryStateFree(&tCurrent);
		return iRet;
	}
	tRedo = pData->arrRedo[pData->iRedoCount - 1];
	memset(&pData->arrRedo[pData->iRedoCount - 1], 0, sizeof(pData->arrRedo[0]));
	pData->iRedoCount--;
	iRet = __xuiTextEditApplyHistoryState(pWidget, pData, &tRedo);
	__xuiTextEditHistoryStateFree(&tRedo);
	return iRet;
}

XUI_API int xuiTextEditCanUndo(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	return (pData != NULL) ? (pData->iUndoCount > 0) : 0;
}

XUI_API int xuiTextEditCanRedo(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	return (pData != NULL) ? (pData->iRedoCount > 0) : 0;
}

XUI_API int xuiTextEditSetScroll(xui_widget pWidget, float fScrollX, float fScrollY)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->fScrollX = fScrollX;
	pData->fScrollY = fScrollY;
	(void)__xuiTextEditUpdateCursorRect(pWidget, pData);
	__xuiTextEditClampScroll(pWidget, pData);
	return __xuiTextEditInvalidatePaint(pWidget);
}

XUI_API int xuiTextEditGetScroll(xui_widget pWidget, float* pScrollX, float* pScrollY)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pScrollX != NULL ) *pScrollX = pData->fScrollX;
	if ( pScrollY != NULL ) *pScrollY = pData->fScrollY;
	return XUI_OK;
}

XUI_API int xuiTextEditScrollBy(xui_widget pWidget, float fDeltaX, float fDeltaY)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiTextEditSetScroll(pWidget, pData->fScrollX + fDeltaX, pData->fScrollY + fDeltaY);
}

XUI_API int xuiTextEditSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iText, uint32_t iBorder, uint32_t iFocus)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->iBackgroundColor = iBackground;
	pData->iHoverBackgroundColor = iBackground;
	pData->iTextColor = iText;
	pData->iBorderColor = iBorder;
	pData->iHoverBorderColor = iFocus;
	pData->iFocusBorderColor = iFocus;
	return __xuiTextEditInvalidatePaint(pWidget);
}

XUI_API int xuiTextEditSetMenuTitle(xui_widget pWidget, int iCommand, const char* sTitle)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	char* sNew;
	if ( (pData == NULL) || (iCommand < 0) || (iCommand >= XUI_INPUT_MENU_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (sTitle == NULL) || (sTitle[0] == '\0') ) {
		if ( pData->arrMenuTitle[iCommand] != NULL ) {
			xrtFree(pData->arrMenuTitle[iCommand]);
			pData->arrMenuTitle[iCommand] = NULL;
		}
		return XUI_OK;
	}
	sNew = __xuiTextEditStringDuplicate(sTitle);
	if ( sNew == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( pData->arrMenuTitle[iCommand] != NULL ) xrtFree(pData->arrMenuTitle[iCommand]);
	pData->arrMenuTitle[iCommand] = sNew;
	return XUI_OK;
}

XUI_API const char* xuiTextEditGetMenuTitle(xui_widget pWidget, int iCommand)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( (pData == NULL) || (iCommand < 0) || (iCommand >= XUI_INPUT_MENU_COUNT) ) {
		return "";
	}
	return (pData->arrMenuTitle[iCommand] != NULL) ? pData->arrMenuTitle[iCommand] : g_xuiTextEditDefaultMenuTitles[iCommand];
}

XUI_API int xuiTextEditOpenMenu(xui_widget pWidget, float fX, float fY)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	int iRet;
	if ( (pData == NULL) || (pData->pMenu == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiTextEditUpdateMenu(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiMenuSetSelect(pData->pMenu, __xuiTextEditMenuSelect, pWidget);
	return xuiMenuOpenAt(pData->pMenu, pWidget, fX, fY);
}

XUI_API xui_widget xuiTextEditGetMenuWidget(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	return (pData != NULL) ? pData->pMenu : NULL;
}

XUI_API xui_rect_t xuiTextEditGetTextRect(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	return (pData != NULL) ? pData->tTextRect : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

XUI_API xui_rect_t xuiTextEditGetCursorRect(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData != NULL ) {
		(void)__xuiTextEditUpdateCursorRect(pWidget, pData);
		return pData->tCursorRect;
	}
	return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

XUI_API int xuiTextEditGetLineCount(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	if ( pData == NULL ) return 0;
	(void)__xuiTextEditUpdateCursorRect(pWidget, pData);
	return pData->iLineCount;
}

XUI_API uint32_t xuiTextEditGetState(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	uint32_t iState;
	if ( pData == NULL ) return 0u;
	iState = xuiWidgetGetInputState(pWidget);
	if ( !xuiWidgetGetEnabled(pWidget) ) {
		iState |= XUI_WIDGET_STATE_DISABLED;
	}
	if ( pData->bReadonly ) {
		iState |= XUI_TEXT_EDIT_STATE_READONLY;
	}
	return iState;
}

XUI_API int xuiTextEditGetChangeCount(xui_widget pWidget)
{
	xui_text_edit_data_t* pData = __xuiTextEditGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}
