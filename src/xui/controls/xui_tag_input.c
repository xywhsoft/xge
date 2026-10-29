#include "xui_config.h"
#if XUI_ENABLE_TAG_INPUT
#include "xui_internal.h"

#include <limits.h>
#include <math.h>
#include <string.h>

#define XUI_TAG_INPUT_STATE_NORMAL	0
#define XUI_TAG_INPUT_STATE_HOVER	XUI_WIDGET_STATE_HOVER
#define XUI_TAG_INPUT_STATE_FOCUS	XUI_WIDGET_STATE_FOCUS
#define XUI_TAG_INPUT_STATE_DISABLED	XUI_WIDGET_STATE_DISABLED
#define XUI_TAG_INPUT_CACHE_COUNT	4
#define XUI_TAG_INPUT_DEFAULT_WIDTH	320.0f
#define XUI_TAG_INPUT_MIN_INPUT_WIDTH	72.0f

typedef struct xui_tag_input_data_t {
	xui_widget pInput;
	xui_font pFont;
	xui_widget_event_proc onInputKeyDown;
	void* pInputKeyDownUser;
	char* sPlaceholder;
	char* arrTags[XUI_TAG_INPUT_TAG_CAPACITY];
	xui_rect_t arrTagRect[XUI_TAG_INPUT_TAG_CAPACITY];
	xui_rect_t arrCloseRect[XUI_TAG_INPUT_TAG_CAPACITY];
	xui_tag_input_change_proc onChange;
	void* pChangeUser;
	xui_tag_input_context_proc onContext;
	void* pContextUser;
	int iTagCount;
	int iMaxTags;
	int iMaxLength;
	int iChangeCount;
	int iHoverClose;
	int iActiveClose;
	int bPointerActive;
	uint64_t iActivePointerId;
	int iActivePointerType;
	int bHover;
	int bSyncingInput;
	uint32_t iTextColor;
	uint32_t iPlaceholderColor;
	uint32_t iDisabledTextColor;
	uint32_t iBackgroundColor;
	uint32_t iHoverBackgroundColor;
	uint32_t iFocusBackgroundColor;
	uint32_t iDisabledBackgroundColor;
	uint32_t iBorderColor;
	uint32_t iHoverBorderColor;
	uint32_t iFocusBorderColor;
	uint32_t iTagBackgroundColor;
	uint32_t iTagHoverBackgroundColor;
	uint32_t iTagTextColor;
	uint32_t iTagCloseColor;
	uint32_t iTagCloseHoverColor;
	float fBorderWidth;
	float fTagHeight;
	float fPaddingX;
	float fPaddingY;
	float fTagPaddingX;
	float fTagGap;
	float fRowGap;
	xui_rect_t tInputRect;
} xui_tag_input_data_t;

static xui_tag_input_data_t* __xuiTagInputGetData(xui_widget pWidget);
static int __xuiTagInputSyncInputStyle(xui_widget pWidget, xui_tag_input_data_t* pData);
static int __xuiTagInputCommitTextEx(xui_widget pWidget, xui_tag_input_data_t* pData,
	const char* sText, int iSize, int bNotify);
static int __xuiTagInputCommitText(xui_widget pWidget, xui_tag_input_data_t* pData, const char* sText, int iSize);
static int __xuiTagInputRemoveAt(xui_widget pWidget, xui_tag_input_data_t* pData, int iIndex, int bNotify);

static int __xuiTagInputDescValid(const xui_tag_input_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) {
		return 0;
	}
	if ( (pDesc->iTagCount < 0) ||
	     (pDesc->iTagCount > XUI_TAG_INPUT_TAG_CAPACITY) ||
	     (pDesc->iTagCount > 0 && pDesc->ppTags == NULL) ||
	     (pDesc->iMaxTags < 0) ||
	     (pDesc->iMaxTags > XUI_TAG_INPUT_TAG_CAPACITY) ||
	     (pDesc->iMaxLength < 0) ||
	     !isfinite(pDesc->fBorderWidth) ||
	     !isfinite(pDesc->fTagHeight) ||
	     (pDesc->fBorderWidth < 0.0f) ||
	     (pDesc->fTagHeight < 0.0f) ||
	     (pDesc->fBorderWidth > XUI_LAYOUT_UNBOUNDED) ||
	     (pDesc->fTagHeight > XUI_LAYOUT_UNBOUNDED) ||
	     (pDesc->iMaxTags > 0 && pDesc->iTagCount > pDesc->iMaxTags) ) {
		return 0;
	}
	return 1;
}

static int __xuiTagInputAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static float __xuiTagInputMax(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static char* __xuiTagInputDuplicateRange(const char* sText, int iSize)
{
	char* sNew;
	size_t iLength;

	if ( sText == NULL ) {
		sText = "";
		iSize = 0;
	}
	if ( iSize < 0 ) {
		iLength = strlen(sText);
		if ( iLength > (size_t)INT_MAX ) return NULL;
		iSize = (int)iLength;
	}
	if ( iSize > INT_MAX - 1 ) return NULL;
	sNew = (char*)xrtMalloc((size_t)iSize + 1u);
	if ( sNew == NULL ) {
		return NULL;
	}
	if ( iSize > 0 ) {
		memcpy(sNew, sText, (size_t)iSize);
	}
	sNew[iSize] = '\0';
	return sNew;
}

static int __xuiTagInputIsTrimChar(unsigned char c)
{
	return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n') || (c == ',') || (c == ';');
}

static char* __xuiTagInputTrimDuplicate(const char* sText, int iSize)
{
	int iStart;
	int iEnd;

	if ( sText == NULL ) {
		sText = "";
		iSize = 0;
	}
	if ( iSize < 0 ) {
		size_t iLength = strlen(sText);
		if ( iLength > (size_t)(INT_MAX - 1) ) return NULL;
		iSize = (int)iLength;
	}
	iStart = 0;
	iEnd = iSize;
	while ( (iStart < iEnd) && __xuiTagInputIsTrimChar((unsigned char)sText[iStart]) ) {
		iStart++;
	}
	while ( (iEnd > iStart) && __xuiTagInputIsTrimChar((unsigned char)sText[iEnd - 1]) ) {
		iEnd--;
	}
	if ( iEnd <= iStart ) {
		return __xuiTagInputDuplicateRange("", 0);
	}
	return __xuiTagInputDuplicateRange(sText + iStart, iEnd - iStart);
}

static int __xuiTagInputDelimiter(char c)
{
	return (c == ',') || (c == ';') || (c == '\n') || (c == '\r');
}

static xui_font __xuiTagInputFont(xui_widget pWidget, xui_tag_input_data_t* pData)
{
	return (pData != NULL && pData->pFont != NULL) ? pData->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
}

static xui_vec2_t __xuiTagInputMeasureText(xui_widget pWidget, xui_font pFont, const char* sText)
{
	xui_proxy pProxy;
	xui_vec2_t tSize;
	int iTextSize;

	tSize = (xui_vec2_t){0.0f, 0.0f};
	if ( sText == NULL ) {
		sText = "";
	}
	iTextSize = (int)strlen(sText);
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy != NULL) && (pProxy->textMeasure != NULL) && (pFont != NULL) &&
	     (pProxy->textMeasure(pProxy, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=sText, .iTextSize=iTextSize, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &tSize) == XUI_OK) ) {
		return tSize;
	}
	tSize.fX = (float)iTextSize * 7.0f;
	tSize.fY = 16.0f;
	return tSize;
}

static float __xuiTagInputLineHeight(xui_widget pWidget, xui_font pFont, float fFallback)
{
	xui_proxy pProxy;
	xui_font_metrics_t tMetrics;

	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy != NULL) && (pProxy->fontGetMetrics != NULL) && (pFont != NULL) &&
	     (pProxy->fontGetMetrics(pProxy, pFont, &tMetrics) == XUI_OK) &&
	     (tMetrics.fLineHeight > 0.0f) ) {
		return tMetrics.fLineHeight;
	}
	return fFallback;
}

static void __xuiTagInputDefaults(xui_tag_input_data_t* pData)
{
	memset(pData, 0, sizeof(*pData));
	pData->iMaxTags = XUI_TAG_INPUT_TAG_CAPACITY;
	pData->iHoverClose = -1;
	pData->iActiveClose = -1;
	pData->iActivePointerType = XUI_POINTER_TYPE_MOUSE;
	pData->iTextColor = XUI_COLOR_RGBA(31, 41, 55, 255);
	pData->iPlaceholderColor = XUI_COLOR_RGBA(135, 148, 166, 255);
	pData->iDisabledTextColor = XUI_COLOR_RGBA(150, 160, 172, 255);
	pData->iBackgroundColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iHoverBackgroundColor = XUI_COLOR_RGBA(250, 253, 255, 255);
	pData->iFocusBackgroundColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iDisabledBackgroundColor = XUI_COLOR_RGBA(242, 245, 249, 255);
	pData->iBorderColor = XUI_COLOR_RGBA(198, 207, 218, 255);
	pData->iHoverBorderColor = XUI_COLOR_RGBA(126, 176, 226, 255);
	pData->iFocusBorderColor = XUI_COLOR_RGBA(47, 128, 237, 255);
	pData->iTagBackgroundColor = XUI_COLOR_RGBA(245, 247, 250, 255);
	pData->iTagHoverBackgroundColor = XUI_COLOR_RGBA(235, 241, 248, 255);
	pData->iTagTextColor = XUI_COLOR_RGBA(31, 41, 55, 255);
	pData->iTagCloseColor = XUI_COLOR_RGBA(116, 128, 144, 255);
	pData->iTagCloseHoverColor = XUI_COLOR_RGBA(66, 78, 94, 255);
	pData->fBorderWidth = 1.0f;
	pData->fTagHeight = 24.0f;
	pData->fPaddingX = 6.0f;
	pData->fPaddingY = 4.0f;
	pData->fTagPaddingX = 10.0f;
	pData->fTagGap = 6.0f;
	pData->fRowGap = 6.0f;
}

static void __xuiTagInputApplyDesc(xui_tag_input_data_t* pData, const xui_tag_input_desc_t* pDesc)
{
	if ( (pData == NULL) || (pDesc == NULL) ) {
		return;
	}
	pData->pFont = pDesc->pFont;
	if ( pDesc->iMaxTags > 0 && pDesc->iMaxTags <= XUI_TAG_INPUT_TAG_CAPACITY ) pData->iMaxTags = pDesc->iMaxTags;
	if ( pDesc->iMaxLength > 0 ) pData->iMaxLength = pDesc->iMaxLength;
	if ( pDesc->iTextColor != 0 ) pData->iTextColor = pDesc->iTextColor;
	if ( pDesc->iPlaceholderColor != 0 ) pData->iPlaceholderColor = pDesc->iPlaceholderColor;
	if ( pDesc->iDisabledTextColor != 0 ) pData->iDisabledTextColor = pDesc->iDisabledTextColor;
	if ( pDesc->iBackgroundColor != 0 ) pData->iBackgroundColor = pDesc->iBackgroundColor;
	if ( pDesc->iHoverBackgroundColor != 0 ) pData->iHoverBackgroundColor = pDesc->iHoverBackgroundColor;
	if ( pDesc->iFocusBackgroundColor != 0 ) pData->iFocusBackgroundColor = pDesc->iFocusBackgroundColor;
	if ( pDesc->iDisabledBackgroundColor != 0 ) pData->iDisabledBackgroundColor = pDesc->iDisabledBackgroundColor;
	if ( pDesc->iBorderColor != 0 ) pData->iBorderColor = pDesc->iBorderColor;
	if ( pDesc->iHoverBorderColor != 0 ) pData->iHoverBorderColor = pDesc->iHoverBorderColor;
	if ( pDesc->iFocusBorderColor != 0 ) pData->iFocusBorderColor = pDesc->iFocusBorderColor;
	if ( pDesc->iTagBackgroundColor != 0 ) pData->iTagBackgroundColor = pDesc->iTagBackgroundColor;
	if ( pDesc->iTagHoverBackgroundColor != 0 ) pData->iTagHoverBackgroundColor = pDesc->iTagHoverBackgroundColor;
	if ( pDesc->iTagTextColor != 0 ) pData->iTagTextColor = pDesc->iTagTextColor;
	if ( pDesc->iTagCloseColor != 0 ) pData->iTagCloseColor = pDesc->iTagCloseColor;
	if ( pDesc->iTagCloseHoverColor != 0 ) pData->iTagCloseHoverColor = pDesc->iTagCloseHoverColor;
	if ( pDesc->fBorderWidth > 0.0f ) pData->fBorderWidth = pDesc->fBorderWidth;
	if ( pDesc->fTagHeight > 0.0f ) pData->fTagHeight = pDesc->fTagHeight;
}

static int __xuiTagInputStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR) ) {
		*pColor = tProperty.tValue.iColor;
		return 1;
	}
	return 0;
}

static int __xuiTagInputStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) &&
	     isfinite(tProperty.tValue.fFloat) && tProperty.tValue.fFloat >= 0.0f &&
	     tProperty.tValue.fFloat <= XUI_LAYOUT_UNBOUNDED ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static xui_font __xuiTagInputStyleFont(xui_widget pWidget, xui_font pBaseFont)
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

static void __xuiTagInputResolve(xui_widget pWidget, xui_tag_input_data_t* pData, xui_tag_input_data_t* pResolved)
{
	if ( (pWidget == NULL) || (pData == NULL) || (pResolved == NULL) ) {
		return;
	}
	*pResolved = *pData;
	pResolved->pFont = __xuiTagInputStyleFont(pWidget, __xuiTagInputFont(pWidget, pData));
	(void)__xuiTagInputStyleColor(pWidget, "taginput.text.color", &pResolved->iTextColor);
	(void)__xuiTagInputStyleColor(pWidget, "taginput.placeholder.color", &pResolved->iPlaceholderColor);
	(void)__xuiTagInputStyleColor(pWidget, "taginput.text.disabled_color", &pResolved->iDisabledTextColor);
	(void)__xuiTagInputStyleColor(pWidget, "taginput.background.color", &pResolved->iBackgroundColor);
	(void)__xuiTagInputStyleColor(pWidget, "taginput.background.hover_color", &pResolved->iHoverBackgroundColor);
	(void)__xuiTagInputStyleColor(pWidget, "taginput.background.focus_color", &pResolved->iFocusBackgroundColor);
	(void)__xuiTagInputStyleColor(pWidget, "taginput.background.disabled_color", &pResolved->iDisabledBackgroundColor);
	(void)__xuiTagInputStyleColor(pWidget, "taginput.border.color", &pResolved->iBorderColor);
	(void)__xuiTagInputStyleColor(pWidget, "taginput.border.hover_color", &pResolved->iHoverBorderColor);
	(void)__xuiTagInputStyleColor(pWidget, "taginput.border.focus_color", &pResolved->iFocusBorderColor);
	(void)__xuiTagInputStyleColor(pWidget, "taginput.tag.background_color", &pResolved->iTagBackgroundColor);
	(void)__xuiTagInputStyleColor(pWidget, "taginput.tag.hover_background_color", &pResolved->iTagHoverBackgroundColor);
	(void)__xuiTagInputStyleColor(pWidget, "taginput.tag.text_color", &pResolved->iTagTextColor);
	(void)__xuiTagInputStyleColor(pWidget, "taginput.tag.close_color", &pResolved->iTagCloseColor);
	(void)__xuiTagInputStyleColor(pWidget, "taginput.tag.close_hover_color", &pResolved->iTagCloseHoverColor);
	(void)__xuiTagInputStyleFloat(pWidget, "taginput.border.width", &pResolved->fBorderWidth);
	(void)__xuiTagInputStyleFloat(pWidget, "taginput.tag.height", &pResolved->fTagHeight);
}

static void __xuiTagInputNotify(xui_widget pWidget, xui_tag_input_data_t* pData)
{
	if ( (pWidget == NULL) || (pData == NULL) ) {
		return;
	}
	pData->iChangeCount++;
	if ( pData->onChange != NULL ) {
		pData->onChange(pWidget, pData->iTagCount, pData->pChangeUser);
	}
}

static int __xuiTagInputAlive(xui_context pContext, xui_widget pWidget,
	const xui_tag_input_data_t* pExpected)
{
	return xuiInternalContextIsValid(pContext) && !xuiInternalContextDestroyPending(pContext) &&
		xuiInternalWidgetIsValid(pWidget) && __xuiTagInputGetData(pWidget) == pExpected;
}

static int __xuiTagInputPointerMatches(const xui_tag_input_data_t* pData,
	const xui_event_t* pEvent)
{
	return (pData != NULL) && (pEvent != NULL) && pData->bPointerActive &&
		(pData->iActivePointerId == pEvent->iPointerId) &&
		(pData->iActivePointerType == pEvent->iPointerType);
}

static int __xuiTagInputCancelPointer(xui_widget pWidget,
	xui_tag_input_data_t* pData, int bInvalidate);

static void __xuiTagInputClampTag(const xui_tag_input_data_t* pData, char* sTag)
{
	if ( pData == NULL || sTag == NULL || pData->iMaxLength <= 0 ) return;
	if ( strlen(sTag) <= (size_t)pData->iMaxLength ) return;
	sTag[pData->iMaxLength] = '\0';
	xuiInternalUtf8TrimIncomplete(sTag);
}

static int __xuiTagInputAddPrepared(xui_widget pWidget, xui_tag_input_data_t* pData, char* sTag, int bNotify)
{
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (sTag == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiTagInputClampTag(pData, sTag);
	if ( sTag[0] == '\0' ) {
		xrtFree(sTag);
		return XUI_OK;
	}
	if ( pData->iTagCount >= pData->iMaxTags || pData->iTagCount >= XUI_TAG_INPUT_TAG_CAPACITY ) {
		xrtFree(sTag);
		return XUI_ERROR_BUFFER_TOO_SMALL;
	}
	pData->arrTags[pData->iTagCount++] = sTag;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	if ( bNotify ) __xuiTagInputNotify(pWidget, pData);
	return XUI_OK;
}

static int __xuiTagInputCommitTextEx(xui_widget pWidget, xui_tag_input_data_t* pData,
	const char* sText, int iSize, int bNotify)
{
	char* sTag;

	sTag = __xuiTagInputTrimDuplicate(sText, iSize);
	if ( sTag == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	return __xuiTagInputAddPrepared(pWidget, pData, sTag, bNotify);
}

static int __xuiTagInputCommitText(xui_widget pWidget, xui_tag_input_data_t* pData,
	const char* sText, int iSize)
{
	return __xuiTagInputCommitTextEx(pWidget, pData, sText, iSize, 1);
}

static int __xuiTagInputRemoveAt(xui_widget pWidget, xui_tag_input_data_t* pData, int iIndex, int bNotify)
{
	int i;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iTagCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiTagInputCancelPointer(pWidget, pData, 0);
	if ( iRet != XUI_OK ) return iRet;
	if ( pData->arrTags[iIndex] != NULL ) {
		xrtFree(pData->arrTags[iIndex]);
	}
	for ( i = iIndex; i < pData->iTagCount - 1; i++ ) {
		pData->arrTags[i] = pData->arrTags[i + 1];
	}
	pData->iTagCount--;
	pData->arrTags[pData->iTagCount] = NULL;
	pData->iHoverClose = -1;
	pData->iActiveClose = -1;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	if ( bNotify ) __xuiTagInputNotify(pWidget, pData);
	return XUI_OK;
}

static int __xuiTagInputSetInputText(xui_widget pWidget, xui_tag_input_data_t* pData, const char* sText)
{
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->pInput == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData->bSyncingInput = 1;
	iRet = xuiInputSetText(pData->pInput, sText);
	pData->bSyncingInput = 0;
	return iRet;
}

static int __xuiTagInputCommitDelimited(xui_widget pWidget, xui_tag_input_data_t* pData, const char* sText)
{
	xui_context pContext;
	char* sStable;
	int iLen;
	int iStart;
	int i;
	int iRet;
	int bFound;

	if ( (pWidget == NULL) || (pData == NULL) || (sText == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	sStable = __xuiTagInputDuplicateRange(sText, -1);
	if ( sStable == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	iLen = (int)strlen(sStable);
	iStart = 0;
	bFound = 0;
	for ( i = 0; i < iLen; i++ ) {
		if ( __xuiTagInputDelimiter(sStable[i]) ) {
			iRet = __xuiTagInputCommitText(pWidget, pData, sStable + iStart, i - iStart);
			if ( iRet == XUI_ERROR_BUFFER_TOO_SMALL ) {
				int iEnd = iLen;

				while ( iEnd > iStart &&
					__xuiTagInputIsTrimChar((unsigned char)sStable[iEnd - 1]) ) {
					iEnd--;
				}
				sStable[iEnd] = '\0';
				iRet = __xuiTagInputSetInputText(pWidget, pData, sStable + iStart);
				xrtFree(sStable);
				return iRet;
			}
			if ( iRet != XUI_OK ) {
				xrtFree(sStable);
				return iRet;
			}
			if ( !__xuiTagInputAlive(pContext, pWidget, pData) ) {
				xrtFree(sStable);
				return XUI_OK;
			}
			iStart = i + 1;
			bFound = 1;
		}
	}
	if ( bFound ) {
		iRet = __xuiTagInputSetInputText(pWidget, pData, sStable + iStart);
		xrtFree(sStable);
		return iRet;
	}
	xrtFree(sStable);
	return XUI_OK;
}

static void __xuiTagInputClearRects(xui_tag_input_data_t* pData)
{
	int i;

	if ( pData == NULL ) {
		return;
	}
	for ( i = 0; i < XUI_TAG_INPUT_TAG_CAPACITY; i++ ) {
		pData->arrTagRect[i] = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		pData->arrCloseRect[i] = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	}
	pData->tInputRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

static float __xuiTagInputMeasureTagWidth(xui_widget pWidget, xui_tag_input_data_t* pData, xui_font pFont, const char* sText)
{
	xui_vec2_t tText;

	tText = __xuiTagInputMeasureText(pWidget, pFont, sText);
	return xuiInternalSnapSize(tText.fX + pData->fTagPaddingX * 2.0f + 18.0f);
}

static float __xuiTagInputLayoutRects(xui_widget pWidget, xui_tag_input_data_t* pData, xui_rect_t tRect, int bStore)
{
	xui_font pFont;
	float fLeft;
	float fRight;
	float fX;
	float fY;
	float fTagW;
	float fTagH;
	float fLineH;
	float fInputW;
	float fCloseSize;
	int i;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return 0.0f;
	}
	if ( bStore ) {
		__xuiTagInputClearRects(pData);
	}
	pFont = __xuiTagInputFont(pWidget, pData);
	fTagH = pData->fTagHeight;
	fLineH = __xuiTagInputMax(fTagH, __xuiTagInputLineHeight(pWidget, pFont, 16.0f) + 8.0f);
	fCloseSize = 12.0f;
	fLeft = tRect.fX + pData->fPaddingX;
	fRight = tRect.fX + tRect.fW - pData->fPaddingX;
	if ( fRight < fLeft ) {
		fRight = fLeft;
	}
	fX = fLeft;
	fY = tRect.fY + pData->fPaddingY;
	for ( i = 0; i < pData->iTagCount; i++ ) {
		fTagW = __xuiTagInputMeasureTagWidth(pWidget, pData, pFont, pData->arrTags[i]);
		if ( (fX > fLeft) && ((fX + fTagW) > fRight) ) {
			fX = fLeft;
			fY += fLineH + pData->fRowGap;
		}
		if ( bStore ) {
			pData->arrTagRect[i] = xuiInternalRectFromFloatNearest(
				fX, fY + (fLineH - fTagH) * 0.5f, fTagW, fTagH);
			pData->arrCloseRect[i] = xuiInternalRectFromFloatNearest(
				fX + fTagW - pData->fTagPaddingX - fCloseSize,
				fY + (fLineH - fCloseSize) * 0.5f,
				fCloseSize,
				fCloseSize);
		}
		fX += fTagW + pData->fTagGap;
	}
	fInputW = fRight - fX;
	if ( (fX > fLeft) && (fInputW < XUI_TAG_INPUT_MIN_INPUT_WIDTH) ) {
		fX = fLeft;
		fY += fLineH + pData->fRowGap;
		fInputW = fRight - fX;
	}
	if ( fInputW < XUI_TAG_INPUT_MIN_INPUT_WIDTH ) {
		fInputW = XUI_TAG_INPUT_MIN_INPUT_WIDTH;
	}
	if ( bStore ) {
		pData->tInputRect = xuiInternalRectFromFloatNearest(fX, fY, fInputW, fLineH);
	}
	return (fY - tRect.fY) + fLineH + pData->fPaddingY;
}

static uint32_t __xuiTagInputComputeState(xui_widget pWidget, xui_tag_input_data_t* pData)
{
	xui_context pContext;
	xui_widget pFocus;
	uint32_t iState;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return 0;
	}
	iState = 0;
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) {
		return XUI_WIDGET_STATE_DISABLED;
	}
	if ( pData->bHover ) {
		iState |= XUI_WIDGET_STATE_HOVER;
	}
	pContext = xuiWidgetGetContext(pWidget);
	pFocus = xuiGetFocusWidget(pContext);
	if ( (pFocus == pWidget) || (pFocus == pData->pInput) ) {
		iState |= XUI_WIDGET_STATE_FOCUS;
	}
	return iState;
}

static uint32_t __xuiTagInputStateId(uint32_t iState)
{
	if ( (iState & XUI_WIDGET_STATE_DISABLED) != 0 ) {
		return XUI_WIDGET_STATE_DISABLED;
	}
	if ( (iState & XUI_WIDGET_STATE_FOCUS) != 0 ) {
		return XUI_WIDGET_STATE_FOCUS;
	}
	if ( (iState & XUI_WIDGET_STATE_HOVER) != 0 ) {
		return XUI_WIDGET_STATE_HOVER;
	}
	return 0;
}

static int __xuiTagInputSyncState(xui_widget pWidget, xui_tag_input_data_t* pData)
{
	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return xuiWidgetSetStateId(pWidget, __xuiTagInputStateId(__xuiTagInputComputeState(pWidget, pData)));
}

static int __xuiTagInputPointInRect(xui_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static int __xuiTagInputCloseHit(xui_widget pWidget, xui_tag_input_data_t* pData, float fWorldX, float fWorldY)
{
	xui_rect_t tWorld;
	float fX;
	float fY;
	int i;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return -1;
	}
	tWorld = xuiWidgetGetWorldRect(pWidget);
	fX = fWorldX - tWorld.fX;
	fY = fWorldY - tWorld.fY;
	for ( i = 0; i < pData->iTagCount; i++ ) {
		if ( __xuiTagInputPointInRect(pData->arrCloseRect[i], fX, fY) ) {
			return i;
		}
	}
	return -1;
}

static int __xuiTagInputTagHit(xui_widget pWidget, xui_tag_input_data_t* pData, float fWorldX, float fWorldY)
{
	xui_rect_t tWorld;
	float fX;
	float fY;
	int i;

	if ( (pWidget == NULL) || (pData == NULL) ) return -1;
	tWorld = xuiWidgetGetWorldRect(pWidget);
	fX = fWorldX - tWorld.fX;
	fY = fWorldY - tWorld.fY;
	for ( i = 0; i < pData->iTagCount; ++i ) {
		if ( __xuiTagInputPointInRect(pData->arrTagRect[i], fX, fY) ) return i;
	}
	return -1;
}

static int __xuiTagInputInvalidateTag(xui_widget pWidget, xui_tag_input_data_t* pData, int iIndex)
{
	xui_rect_t tRect;

	if ( (pWidget == NULL) || (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iTagCount) ) {
		return XUI_OK;
	}
	tRect = pData->arrTagRect[iIndex];
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	tRect.fX -= 2.0f;
	tRect.fY -= 2.0f;
	tRect.fW += 4.0f;
	tRect.fH += 4.0f;
	return xuiWidgetInvalidateRect(pWidget, tRect, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __xuiTagInputSetHoverClose(xui_widget pWidget, xui_tag_input_data_t* pData, int iIndex)
{
	int iOldHover;
	int iRet;

	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iHoverClose == iIndex ) {
		return XUI_OK;
	}
	iOldHover = pData->iHoverClose;
	pData->iHoverClose = iIndex;
	iRet = __xuiTagInputInvalidateTag(pWidget, pData, iOldHover);
	if ( iRet == XUI_OK ) {
		iRet = __xuiTagInputInvalidateTag(pWidget, pData, iIndex);
	}
	return iRet;
}

static int __xuiTagInputCancelPointer(xui_widget pWidget,
	xui_tag_input_data_t* pData, int bInvalidate)
{
	xui_context pContext;
	uint64_t iPointerId;
	int iPointerType;
	int bPointerActive;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	iPointerId = pData->iActivePointerId;
	iPointerType = pData->iActivePointerType;
	bPointerActive = pData->bPointerActive;
	pData->iActiveClose = -1;
	pData->bPointerActive = 0;
	pData->iActivePointerId = 0;
	pData->iActivePointerType = XUI_POINTER_TYPE_MOUSE;
	if ( bPointerActive && xuiInternalContextIsValid(pContext) &&
	     xuiGetPointerCaptureEx(pContext, iPointerId, iPointerType) == pWidget ) {
		iRet = xuiReleasePointerCaptureEx(pContext, iPointerId, iPointerType, pWidget);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( bInvalidate && __xuiTagInputAlive(pContext, pWidget, pData) ) {
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return XUI_OK;
}

static int __xuiTagInputDrawFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( __xuiTagInputAlpha(iColor) == 0 ) {
		return XUI_OK;
	}
	return (pProxy->drawRectFill != NULL) ? pProxy->drawRectFill(pProxy, pDraw, tRect, iColor) : XUI_OK;
}

static int __xuiTagInputDrawStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	if ( (fWidth <= 0.0f) || (__xuiTagInputAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	tRect = xuiInternalStrokeCenterRectInside(tRect, fWidth, NULL);
	return (pProxy->drawRectStroke != NULL) ? pProxy->drawRectStroke(pProxy, pDraw, tRect, fWidth, iColor) : XUI_OK;
}

static int __xuiTagInputDrawClose(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	float fX0;
	float fY0;
	float fX1;
	float fY1;
	int iRet;

	if ( pProxy->drawLine == NULL || __xuiTagInputAlpha(iColor) == 0 ) {
		return XUI_OK;
	}
	fX0 = tRect.fX + 3.0f;
	fY0 = tRect.fY + 3.0f;
	fX1 = tRect.fX + tRect.fW - 3.0f;
	fY1 = tRect.fY + tRect.fH - 3.0f;
	iRet = pProxy->drawLine(pProxy, pDraw, fX0, fY0, fX1, fY1, 1.25f, iColor);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	return pProxy->drawLine(pProxy, pDraw, fX0, fY1, fX1, fY0, 1.25f, iColor);
}

static int __xuiTagInputContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_tag_input_data_t* pData;
	xui_tag_input_data_t tResolved;
	float fWidth;
	float fHeight;

	pData = (xui_tag_input_data_t*)pUser;
	if ( (pWidget == NULL) || (pData == NULL) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiTagInputResolve(pWidget, pData, &tResolved);
	fWidth = tConstraint.fX;
	if ( (fWidth <= 0.0f) || (fWidth > XUI_LAYOUT_UNBOUNDED) ) {
		fWidth = XUI_TAG_INPUT_DEFAULT_WIDTH;
	}
	fHeight = __xuiTagInputLayoutRects(pWidget, &tResolved,
		xuiInternalRectFromFloatNearest(0.0f, 0.0f, fWidth, 10000.0f), 0);
	pSize->fX = xuiInternalSnapSize(fWidth);
	pSize->fY = xuiInternalSnapSize(__xuiTagInputMax(fHeight, 34.0f));
	return XUI_OK;
}

static int __xuiTagInputLayoutPrepare(xui_widget pWidget, void* pUser)
{
	xui_tag_input_data_t* pData = (xui_tag_input_data_t*)pUser;
	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pInput == NULL ) return XUI_OK;
	return __xuiTagInputSyncInputStyle(pWidget, pData);
}

static int __xuiTagInputLayoutChildren(xui_widget pWidget, xui_rect_t tContentRect, void* pUser)
{
	xui_tag_input_data_t* pData;
	xui_tag_input_data_t tResolved;

	pData = (xui_tag_input_data_t*)pUser;
	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->pInput == NULL ) {
		return XUI_OK;
	}
	__xuiTagInputResolve(pWidget, pData, &tResolved);
	__xuiTagInputLayoutRects(pWidget, &tResolved, tContentRect, 1);
	pData->tInputRect = tResolved.tInputRect;
	memcpy(pData->arrTagRect, tResolved.arrTagRect, sizeof(pData->arrTagRect));
	memcpy(pData->arrCloseRect, tResolved.arrCloseRect, sizeof(pData->arrCloseRect));
	return xuiLayoutArrangeChild(pWidget, pData->pInput, pData->tInputRect);
}

static int __xuiTagInputCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_tag_input_data_t* pData;
	xui_tag_input_data_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tRect;
	xui_rect_t tTag;
	xui_rect_t tText;
	uint32_t iBackground;
	uint32_t iBorder;
	uint32_t iTagBackground;
	uint32_t iCloseColor;
	int i;
	int iRet;

	(void)iStateId;
	pData = (xui_tag_input_data_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiTagInputResolve(pWidget, pData, &tResolved);
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect = xuiInternalSnapRect(tRect);
	iBackground = tResolved.iBackgroundColor;
	iBorder = tResolved.iBorderColor;
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) {
		iBackground = tResolved.iDisabledBackgroundColor;
		iBorder = tResolved.iBorderColor;
	} else if ( (iStateId & XUI_WIDGET_STATE_FOCUS) != 0 ) {
		iBackground = tResolved.iFocusBackgroundColor;
		iBorder = tResolved.iFocusBorderColor;
	} else if ( (iStateId & XUI_WIDGET_STATE_HOVER) != 0 ) {
		iBackground = tResolved.iHoverBackgroundColor;
		iBorder = tResolved.iHoverBorderColor;
	}
	iRet = __xuiTagInputDrawFill(pProxy, pDraw, tRect, iBackground);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiTagInputDrawStroke(pProxy, pDraw, tRect, tResolved.fBorderWidth, iBorder);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < tResolved.iTagCount; i++ ) {
		tTag = xuiInternalSnapRect(tResolved.arrTagRect[i]);
		iTagBackground = (i == pData->iHoverClose || i == pData->iActiveClose) ? tResolved.iTagHoverBackgroundColor : tResolved.iTagBackgroundColor;
		iRet = __xuiTagInputDrawFill(pProxy, pDraw, tTag, iTagBackground);
		if ( iRet != XUI_OK ) return iRet;
		tText = tTag;
		tText.fX += tResolved.fTagPaddingX;
		tText.fW -= tResolved.fTagPaddingX * 2.0f + 18.0f;
		if ( tText.fW > 0.0f && tResolved.pFont != NULL ) {
			iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=tResolved.pFont, .sText=tResolved.arrTags[i], .iTextSize=(int)strlen(tResolved.arrTags[i]), .iFlags=XUI_TEXT_SHAPE_DEFAULT}, tText, tResolved.iTagTextColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
		iCloseColor = (i == pData->iHoverClose || i == pData->iActiveClose) ? tResolved.iTagCloseHoverColor : tResolved.iTagCloseColor;
		iRet = __xuiTagInputDrawClose(pProxy, pDraw, tResolved.arrCloseRect[i], iCloseColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiTagInputPointerInside(xui_widget pWidget, const xui_event_t* pEvent)
{
	xui_rect_t tWorld;

	if ( (pWidget == NULL) || (pEvent == NULL) ) {
		return 0;
	}
	tWorld = xuiWidgetGetWorldRect(pWidget);
	return (pEvent->fX >= tWorld.fX) && (pEvent->fY >= tWorld.fY) &&
	       (pEvent->fX < tWorld.fX + tWorld.fW) && (pEvent->fY < tWorld.fY + tWorld.fH);
}

static int __xuiTagInputEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_tag_input_data_t* pData;
	xui_context pContext;
	const char* sText;
	int iHit;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_MOVE:
		if ( pData->bPointerActive && !__xuiTagInputPointerMatches(pData, pEvent) ) return XUI_OK;
		pData->bHover = __xuiTagInputPointerInside(pWidget, pEvent);
		iHit = __xuiTagInputCloseHit(pWidget, pData, pEvent->fX, pEvent->fY);
		iRet = __xuiTagInputSetHoverClose(pWidget, pData, iHit);
		if ( iRet != XUI_OK ) return iRet;
		return __xuiTagInputSyncState(pWidget, pData);
	case XUI_EVENT_POINTER_LEAVE:
		if ( pData->bPointerActive && !__xuiTagInputPointerMatches(pData, pEvent) ) return XUI_OK;
		pData->bHover = 0;
		if ( pData->iActiveClose < 0 ) {
			iRet = __xuiTagInputSetHoverClose(pWidget, pData, -1);
			if ( iRet != XUI_OK ) return iRet;
		}
		return __xuiTagInputSyncState(pWidget, pData);
	case XUI_EVENT_POINTER_DOWN:
		if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT || pEvent->iButton == 0 ) {
			if ( pData->bPointerActive ) {
				return __xuiTagInputPointerMatches(pData, pEvent) ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
			}
			iHit = __xuiTagInputCloseHit(pWidget, pData, pEvent->fX, pEvent->fY);
			if ( iHit >= 0 && pData->pInput != NULL && !xuiInputIsReadonly(pData->pInput) ) {
				iRet = xuiSetPointerCaptureEx(pContext, pEvent->iPointerId,
					pEvent->iPointerType, pWidget);
				if ( iRet != XUI_OK ) return iRet;
				if ( !__xuiTagInputAlive(pContext, pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
				pData->iActiveClose = iHit;
				pData->iHoverClose = iHit;
				pData->bPointerActive = 1;
				pData->iActivePointerId = pEvent->iPointerId;
				pData->iActivePointerType = pEvent->iPointerType;
				iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
				if ( iRet != XUI_OK ) {
					(void)__xuiTagInputCancelPointer(pWidget, pData, 0);
					return iRet;
				}
				return XUI_EVENT_DISPATCH_STOP;
			}
			if ( pEvent->pTarget == pWidget && pData->pInput != NULL ) {
				iRet = xuiSetFocusWidget(pContext, pData->pInput);
				return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
			}
		}
		break;
	case XUI_EVENT_POINTER_UP:
		if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT || pEvent->iButton == 0 ) {
			if ( pData->iActiveClose >= 0 && __xuiTagInputPointerMatches(pData, pEvent) ) {
				int iActive = pData->iActiveClose;
				iHit = __xuiTagInputCloseHit(pWidget, pData, pEvent->fX, pEvent->fY);
				iRet = __xuiTagInputCancelPointer(pWidget, pData, 0);
				if ( iRet != XUI_OK ) return iRet;
				iRet = __xuiTagInputSetHoverClose(pWidget, pData, iHit);
				if ( iRet != XUI_OK ) return iRet;
				if ( iHit == iActive ) {
					iRet = __xuiTagInputRemoveAt(pWidget, pData, iActive, 1);
				}
				return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
			}
		}
		break;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( !__xuiTagInputPointerMatches(pData, pEvent) ) return XUI_OK;
		return __xuiTagInputCancelPointer(pWidget, pData, 1);
	case XUI_EVENT_FOCUS:
	case XUI_EVENT_BLUR:
		iRet = __xuiTagInputSyncState(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	case XUI_EVENT_ENABLED_CHANGED:
	case XUI_EVENT_VISIBLE_CHANGED:
		if ( !xuiWidgetGetEnabled(pWidget) || !xuiWidgetGetVisible(pWidget) ) {
			iRet = __xuiTagInputCancelPointer(pWidget, pData, 0);
			if ( iRet != XUI_OK ) return iRet;
		}
		iRet = __xuiTagInputSyncInputStyle(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiTagInputSyncState(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	case XUI_EVENT_KEY_DOWN:
		if ( pEvent->pTarget != pWidget ) {
			break;
		}
		if ( pData->pInput != NULL && xuiInputIsReadonly(pData->pInput) ) {
			return XUI_OK;
		}
		if ( pEvent->iKey == XUI_KEY_ENTER ) {
			iRet = xuiTagInputCommit(pWidget);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		if ( pEvent->iKey == XUI_KEY_BACKSPACE && pData->pInput != NULL ) {
			sText = xuiInputGetText(pData->pInput);
			if ( (sText == NULL || sText[0] == '\0') && pData->iTagCount > 0 ) {
				iRet = __xuiTagInputRemoveAt(pWidget, pData, pData->iTagCount - 1, 1);
				return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
			}
		}
		break;
	case XUI_EVENT_TEXT:
		if ( pEvent->pTarget != pWidget ) {
			break;
		}
		if ( pData->pInput != NULL && xuiInputIsReadonly(pData->pInput) ) return XUI_OK;
		if ( pEvent->iTextSize > 0 ) {
			for ( iHit = 0; iHit < pEvent->iTextSize; iHit++ ) {
				if ( __xuiTagInputDelimiter(pEvent->sText[iHit]) ) {
					iRet = xuiTagInputCommit(pWidget);
					return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
				}
			}
		} else if ( pEvent->iCodepoint == ',' || pEvent->iCodepoint == ';' ) {
			iRet = xuiTagInputCommit(pWidget);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		break;
	case XUI_EVENT_CONTEXT_MENU: {
		xui_rect_t tWorld;
		xui_rect_t tAnchor;
		float fX;
		float fY;
		if ( pData->onContext == NULL || pEvent->pTarget != pWidget ) break;
		tWorld = xuiWidgetGetWorldRect(pWidget);
		tAnchor = tWorld;
		if ( pEvent->iKey == XUI_KEY_CONTEXT_MENU ) {
			iHit = pData->iTagCount - 1;
			if ( iHit >= 0 ) {
				tAnchor = pData->arrTagRect[iHit];
				tAnchor.fX += tWorld.fX;
				tAnchor.fY += tWorld.fY;
			}
		} else {
			iHit = __xuiTagInputTagHit(pWidget, pData, pEvent->fX, pEvent->fY);
		}
		xuiInternalContextMenuPoint(pEvent, tAnchor, &fX, &fY);
		return pData->onContext(pWidget, iHit, fX, fY, pData->pContextUser);
	}
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiTagInputChildKeyDown(xui_widget pInput, const xui_event_t* pEvent, void* pUser)
{
	xui_widget pTagInput;
	xui_tag_input_data_t* pData;
	const char* sText;
	int iRet;

	pTagInput = (xui_widget)pUser;
	pData = __xuiTagInputGetData(pTagInput);
	if ( pData == NULL || pEvent == NULL ) {
		return XUI_OK;
	}
	if ( xuiInputIsReadonly(pInput) ) return XUI_OK;
	if ( pEvent->iKey == XUI_KEY_BACKSPACE ) {
		sText = xuiInputGetText(pInput);
		if ( (sText == NULL || sText[0] == '\0') && pData->iTagCount > 0 ) {
			iRet = __xuiTagInputRemoveAt(pTagInput, pData, pData->iTagCount - 1, 1);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
	}
	if ( pData->onInputKeyDown != NULL ) {
		return pData->onInputKeyDown(pInput, pEvent, pData->pInputKeyDownUser);
	}
	return XUI_OK;
}

static int __xuiTagInputPrepareEdit(xui_widget pWidget, xui_widget pSource,
	int iType, int* pSkipDefault, void* pUser)
{
	xui_tag_input_data_t* pData = (xui_tag_input_data_t*)pUser;

	(void)pSkipDefault;
	if ( pData == NULL || pSource != pData->pInput || __xuiTagInputGetData(pWidget) != pData ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iType == XUI_EDIT_EVENT_COMMIT ) return xuiTagInputCommit(pWidget);
	return XUI_OK;
}

static void __xuiTagInputInputChanged(xui_widget pInput, const char* sText, void* pUser)
{
	xui_widget pTagInput;
	xui_tag_input_data_t* pData;
	xui_context pContext;
	int iRet;

	(void)pInput;
	pTagInput = (xui_widget)pUser;
	pData = __xuiTagInputGetData(pTagInput);
	if ( pData == NULL || pData->bSyncingInput ) {
		return;
	}
	pContext = xuiWidgetGetContext(pTagInput);
	if ( sText != NULL ) {
		iRet = __xuiTagInputCommitDelimited(pTagInput, pData, sText);
		if ( iRet != XUI_OK || !__xuiTagInputAlive(pContext, pTagInput, pData) ) return;
	}
	(void)xuiWidgetInvalidate(pTagInput, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void __xuiTagInputDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_OVERLAY;
	pLayout->iWidthMode = XUI_SIZE_FIXED;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_VISIBLE;
	pLayout->iAlignX = XUI_ALIGN_START;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fPreferredWidth = XUI_TAG_INPUT_DEFAULT_WIDTH;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static void __xuiTagInputDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE | XUI_CACHE_UPDATE_ALL_STATES;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiTagInputInitCacheStates(xui_widget pWidget)
{
	static const uint32_t arrState[XUI_TAG_INPUT_CACHE_COUNT] = {
		XUI_TAG_INPUT_STATE_NORMAL,
		XUI_TAG_INPUT_STATE_HOVER,
		XUI_TAG_INPUT_STATE_FOCUS,
		XUI_TAG_INPUT_STATE_DISABLED
	};
	int i;
	int iRet;

	iRet = xuiWidgetSetCacheStateCount(pWidget, XUI_TAG_INPUT_CACHE_COUNT);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	for ( i = 0; i < XUI_TAG_INPUT_CACHE_COUNT; i++ ) {
		iRet = xuiWidgetSetCacheStateId(pWidget, i, arrState[i]);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiTagInputInitEvents(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_ENTER, __xuiTagInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiTagInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiTagInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiTagInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_UP, __xuiTagInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiTagInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiTagInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiTagInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiTagInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiTagInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiTagInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_TEXT, __xuiTagInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_CONTEXT_MENU, __xuiTagInputEvent, NULL);
	return iRet;
}

static int __xuiTagInputSyncInputColors(xui_widget pInput, const xui_tag_input_data_t* pResolved)
{
	uint32_t arrColors[7];
	int iRet;

	iRet = xuiInputGetColors(pInput, &arrColors[0], &arrColors[1], &arrColors[2], &arrColors[3]);
	if ( iRet != XUI_OK ) return iRet;
	if ( arrColors[0] != 0 || arrColors[1] != pResolved->iTextColor || arrColors[2] != 0 || arrColors[3] != 0 ) {
		iRet = xuiInputSetColors(pInput, 0, pResolved->iTextColor, 0, 0);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = xuiInputGetErrorColors(pInput, &arrColors[0], &arrColors[1]);
	if ( iRet != XUI_OK ) return iRet;
	if ( arrColors[0] != 0 || arrColors[1] != 0 ) {
		iRet = xuiInputSetErrorColors(pInput, 0, 0);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = xuiInputGetExtendedColors(pInput, &arrColors[0], &arrColors[1], &arrColors[2],
		&arrColors[3], &arrColors[4], &arrColors[5], &arrColors[6]);
	if ( iRet != XUI_OK ) return iRet;
	if ( arrColors[0] != pResolved->iPlaceholderColor || arrColors[1] != pResolved->iDisabledTextColor ||
	     arrColors[2] != 0 || arrColors[3] != 0 || arrColors[4] != 0 || arrColors[6] != pResolved->iTextColor ) {
		/* Selection belongs to the child Input, including its API base color. */
		return xuiInputSetExtendedColors(pInput, pResolved->iPlaceholderColor, pResolved->iDisabledTextColor,
			0, 0, 0, arrColors[5], pResolved->iTextColor);
	}
	return XUI_OK;
}

static int __xuiTagInputPreparePaint(xui_widget pWidget)
{
	xui_tag_input_data_t* pData = __xuiTagInputGetData(pWidget);
	xui_tag_input_data_t tResolved;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pInput == NULL ) return XUI_OK;
	__xuiTagInputResolve(pWidget, pData, &tResolved);
	return __xuiTagInputSyncInputColors(pData->pInput, &tResolved);
}

static int __xuiTagInputSyncInputStyle(xui_widget pWidget, xui_tag_input_data_t* pData)
{
	xui_tag_input_data_t tResolved;
	int bEnabled;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->pInput == NULL) ) {
		return (pWidget != NULL && pData != NULL) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiTagInputResolve(pWidget, pData, &tResolved);
	bEnabled = xuiWidgetGetEffectiveEnabled(pWidget) && xuiWidgetGetVisible(pWidget);
	iRet = __xuiTagInputSyncInputColors(pData->pInput, &tResolved);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputSetBorderWidth(pData->pInput, 0.0f);
	if ( iRet != XUI_OK ) return iRet;
	if ( strcmp(xuiInputGetPlaceholder(pData->pInput),
	     (pData->sPlaceholder != NULL) ? pData->sPlaceholder : "") != 0 ) {
		iRet = xuiInputSetPlaceholder(pData->pInput,
			(pData->sPlaceholder != NULL) ? pData->sPlaceholder : "");
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( xuiInputGetFont(pData->pInput) != tResolved.pFont ) {
		iRet = xuiInputSetFont(pData->pInput, tResolved.pFont);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( xuiInputGetMaxLength(pData->pInput) != pData->iMaxLength ) {
		iRet = xuiInputSetMaxLength(pData->pInput, pData->iMaxLength);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = xuiWidgetSetEnabled(pData->pInput, bEnabled);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetVisible(pData->pInput, xuiWidgetGetVisible(pWidget));
	if ( iRet != XUI_OK ) return iRet;
	return xuiWidgetSetHitTestVisible(pData->pInput, bEnabled);
}

static int __xuiTagInputCreateInputChild(xui_widget pWidget, xui_tag_input_data_t* pData, const xui_tag_input_desc_t* pDesc)
{
	xui_input_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = (pDesc != NULL) ? pDesc->sText : "";
	tDesc.sPlaceholder = (pData->sPlaceholder != NULL) ? pData->sPlaceholder : "";
	tDesc.pFont = pData->pFont;
	tDesc.iMaxLength = pData->iMaxLength;
	tDesc.iTextAlign = XUI_INPUT_ALIGN_LEFT;
	tDesc.iTextColor = pData->iTextColor;
	tDesc.iPlaceholderColor = pData->iPlaceholderColor;
	tDesc.iDisabledTextColor = pData->iDisabledTextColor;
	tDesc.iBackgroundColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	tDesc.iHoverBackgroundColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	tDesc.iDisabledBackgroundColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	tDesc.iBorderColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	tDesc.iHoverBorderColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	tDesc.iFocusBorderColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	tDesc.iErrorBackgroundColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	tDesc.iErrorBorderColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	tDesc.fBorderWidth = 0.0f;
	iRet = xuiInputCreate(xuiWidgetGetContext(pWidget), &pData->pInput, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputSetChange(pData->pInput, __xuiTagInputInputChanged, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetGetEventHandler(pData->pInput, XUI_EVENT_KEY_DOWN, &pData->onInputKeyDown, &pData->pInputKeyDownUser);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pInput, XUI_EVENT_KEY_DOWN, __xuiTagInputChildKeyDown, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pWidget, pData->pInput);
	if ( iRet == XUI_OK ) iRet = xuiInternalEditDelegate(pWidget, pData->pInput);
	if ( iRet == XUI_OK ) iRet = xuiInternalEditSetPrepare(pWidget, __xuiTagInputPrepareEdit, pData);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pData->pInput);
		pData->pInput = NULL;
		return iRet;
	}
	{
		xui_edit_behavior_t tBehavior;
		memset(&tBehavior, 0, sizeof(tBehavior));
		tBehavior.iSize = sizeof(tBehavior);
		tBehavior.iTabBehavior = XUI_EDIT_TAB_FOCUS;
		tBehavior.iEnterBehavior = XUI_EDIT_ENTER_COMMIT;
		tBehavior.iEscapeBehavior = XUI_EDIT_ESCAPE_DEFAULT;
		iRet = xuiEditSetBehavior(pWidget, &tBehavior);
		if ( iRet != XUI_OK ) return iRet;
	}
	return __xuiTagInputSyncInputStyle(pWidget, pData);
}

static int __xuiTagInputAccessibleCount(xui_widget pWidget, void* pUser)
{
	xui_tag_input_data_t* pData = (xui_tag_input_data_t*)pUser;
	return __xuiTagInputAlive(xuiWidgetGetContext(pWidget), pWidget, pData) ? 1 + pData->iTagCount : 0;
}

static int __xuiTagInputAccessibleGet(xui_widget pWidget, int iIndex,
	xui_accessible_node_t* pNode, void* pUser)
{
	xui_tag_input_data_t* pData = (xui_tag_input_data_t*)pUser;
	xui_rect_t tWorld;
	xui_rect_t tTag;
	int iTag;
	int bEnabled;
	int bReadonly;

	if ( pNode == NULL || iIndex < 0 ||
	     !__xuiTagInputAlive(xuiWidgetGetContext(pWidget), pWidget, pData) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tWorld = xuiWidgetGetWorldRect(pWidget);
	bEnabled = xuiWidgetGetEffectiveEnabled(pWidget);
	bReadonly = pData->pInput != NULL && xuiInputIsReadonly(pData->pInput);
	if ( iIndex == 0 ) {
		pNode->iId = 1;
		pNode->iRole = XUI_ACCESSIBLE_ROLE_GROUP;
		pNode->sName = xuiWidgetGetAccessibleName(pWidget) != NULL ?
			xuiWidgetGetAccessibleName(pWidget) : "Tag input";
		pNode->sDescription = xuiWidgetGetAccessibleDescription(pWidget);
		pNode->tBounds = tWorld;
		if ( !bEnabled ) pNode->iState |= XUI_ACCESSIBLE_STATE_DISABLED;
		if ( bReadonly ) pNode->iState |= XUI_ACCESSIBLE_STATE_READONLY;
		if ( !xuiWidgetGetEffectiveVisible(pWidget) ) pNode->iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
		if ( bEnabled && pData->pInput != NULL && xuiWidgetGetEffectiveFocusable(pData->pInput) ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSABLE;
			pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_FOCUS);
		}
		if ( pData->pInput != NULL && xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pData->pInput ) {
			pNode->iState |= XUI_ACCESSIBLE_STATE_FOCUSED;
		}
		return XUI_OK;
	}
	iTag = iIndex - 1;
	if ( iTag < 0 || iTag >= pData->iTagCount ) return XUI_ERROR_INVALID_ARGUMENT;
	tTag = pData->arrTagRect[iTag];
	pNode->iId = (uint64_t)iTag + 2u;
	pNode->iParentId = 1;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_BUTTON;
	pNode->sName = pData->arrTags[iTag];
	pNode->sDescription = "Remove tag";
	pNode->tBounds = (xui_rect_t){tWorld.fX + tTag.fX, tWorld.fY + tTag.fY, tTag.fW, tTag.fH};
	if ( !bEnabled ) pNode->iState |= XUI_ACCESSIBLE_STATE_DISABLED;
	if ( bReadonly ) pNode->iState |= XUI_ACCESSIBLE_STATE_READONLY;
	if ( !xuiWidgetGetEffectiveVisible(pWidget) || tTag.fW <= 0.0f || tTag.fH <= 0.0f ) {
		pNode->iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
	}
	if ( bEnabled && !bReadonly ) {
		pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_CLOSE);
	}
	return XUI_OK;
}

static int __xuiTagInputAccessibleAction(xui_widget pWidget, uint64_t iNodeId,
	int iAction, const void* pPayload, void* pUser)
{
	xui_tag_input_data_t* pData = (xui_tag_input_data_t*)pUser;
	int iTag;

	(void)pPayload;
	if ( !__xuiTagInputAlive(xuiWidgetGetContext(pWidget), pWidget, pData) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) {
		return XUI_ERROR_INVALID_STATE;
	}
	if ( iNodeId == 0 || iNodeId == 1 ) {
		if ( iAction != XUI_ACCESSIBLE_ACTION_FOCUS || pData->pInput == NULL ) {
			return XUI_ERROR_UNSUPPORTED;
		}
		return xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pData->pInput);
	}
	if ( iNodeId < 2 || iNodeId > (uint64_t)pData->iTagCount + 1u ) {
		return XUI_ERROR_NOT_FOUND;
	}
	if ( iAction != XUI_ACCESSIBLE_ACTION_CLOSE ) return XUI_ERROR_UNSUPPORTED;
	if ( pData->pInput != NULL && xuiInputIsReadonly(pData->pInput) ) {
		return XUI_ERROR_INVALID_STATE;
	}
	iTag = (int)iNodeId - 2;
	return __xuiTagInputRemoveAt(pWidget, pData, iTag, 1);
}

static int __xuiTagInputInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_tag_input_data_t* pData;
	const xui_tag_input_desc_t* pDesc;
	int i;
	int iRet;

	(void)pUser;
	pData = (xui_tag_input_data_t*)pTypeData;
	pDesc = (const xui_tag_input_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiTagInputDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiTagInputDefaults(pData);
	__xuiTagInputApplyDesc(pData, pDesc);
	if ( pData->pFont == NULL ) {
		pData->pFont = xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	}
	pData->sPlaceholder = __xuiTagInputDuplicateRange((pDesc != NULL) ? pDesc->sPlaceholder : "", -1);
	if ( pData->sPlaceholder == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	if ( (pDesc != NULL) && (pDesc->ppTags != NULL) ) {
		for ( i = 0; i < pDesc->iTagCount; i++ ) {
			iRet = __xuiTagInputCommitTextEx(pWidget, pData, pDesc->ppTags[i], -1, 0);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	iRet = xuiWidgetSetFocusable(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetHitTestVisible(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = __xuiTagInputInitCacheStates(pWidget);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiTagInputInitEvents(pWidget);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiTagInputCreateInputChild(pWidget, pData, pDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetAccessibilityProvider(pWidget,
		__xuiTagInputAccessibleCount, __xuiTagInputAccessibleGet,
		__xuiTagInputAccessibleAction, pData);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiTagInputSyncState(pWidget, pData);
}

static void __xuiTagInputDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_tag_input_data_t* pData;
	int i;

	(void)pUser;
	pData = (xui_tag_input_data_t*)pTypeData;
	if ( pData == NULL ) {
		return;
	}
	(void)__xuiTagInputCancelPointer(pWidget, pData, 0);
	for ( i = 0; i < XUI_TAG_INPUT_TAG_CAPACITY; i++ ) {
		if ( pData->arrTags[i] != NULL ) {
			xrtFree(pData->arrTags[i]);
		}
	}
	if ( pData->sPlaceholder != NULL ) {
		xrtFree(pData->sPlaceholder);
	}
	memset(pData, 0, sizeof(*pData));
}

static void __xuiTagInputRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
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

static void __xuiTagInputRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;
	uint32_t iLayoutDirty;

	pType->onPreparePaint = __xuiTagInputPreparePaint;
	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	iLayoutDirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.text.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.placeholder.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.text.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.background.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.background.focus_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.background.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.border.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.border.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.border.focus_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.tag.background_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.tag.hover_background_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.tag.text_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.tag.close_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.tag.close_hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.border.width", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "taginput.tag.height", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiTagInputRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, iLayoutDirty, XUI_STYLE_PROPERTY_INHERITED);
}

static xui_tag_input_data_t* __xuiTagInputGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return NULL;
	}
	pContext = xuiWidgetGetContext(pWidget);
	pType = xuiWidgetFindType(pContext, "taginput");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_tag_input_data_t*)xuiWidgetGetTypeData(pWidget);
}

XUI_API xui_widget_type xuiTagInputGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	xui_layout_t tLayout;
	xui_cache_policy_t tPolicy;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "taginput");
	if ( pType != NULL ) {
		__xuiTagInputRegisterStyleProperties(pContext, pType);
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "taginput";
	tDesc.iTypeDataSize = sizeof(xui_tag_input_data_t);
	tDesc.onInit = __xuiTagInputInit;
	tDesc.onDestroy = __xuiTagInputDestroy;
	tDesc.onContentMeasure = __xuiTagInputContentMeasure;
	tDesc.onLayoutPrepare = __xuiTagInputLayoutPrepare;
	tDesc.onLayoutChildren = __xuiTagInputLayoutChildren;
	tDesc.onCacheRender = __xuiTagInputCacheRender;
	__xuiTagInputDefaultLayout(&tLayout);
	__xuiTagInputDefaultCachePolicy(&tPolicy);
	tDesc.tLayout = tLayout;
	tDesc.tCachePolicy = tPolicy;
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	__xuiTagInputRegisterStyleProperties(pContext, pType);
	return pType;
}

XUI_API int xuiTagInputCreate(xui_context pContext, xui_widget* ppWidget, const xui_tag_input_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppWidget = NULL;
	if ( !xuiInternalContextIsValid(pContext) || !__xuiTagInputDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = xuiTagInputGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiTagInputSetChange(xui_widget pWidget, xui_tag_input_change_proc onChange, void* pUser)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onChange = onChange;
	pData->pChangeUser = pUser;
	return XUI_OK;
}

XUI_API int xuiTagInputSetContextMenu(xui_widget pWidget, xui_tag_input_context_proc onContext, void* pUser)
{
	xui_tag_input_data_t* pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onContext = onContext;
	pData->pContextUser = pUser;
	return XUI_OK;
}

XUI_API int xuiTagInputAddTag(xui_widget pWidget, const char* sText)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiTagInputCommitText(pWidget, pData, sText, -1);
}

XUI_API int xuiTagInputRemoveTag(xui_widget pWidget, int iIndex)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiTagInputRemoveAt(pWidget, pData, iIndex, 1);
}

XUI_API int xuiTagInputClearTags(xui_widget pWidget)
{
	xui_tag_input_data_t* pData;
	int i;
	int iRet;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iTagCount == 0 ) return XUI_OK;
	iRet = __xuiTagInputCancelPointer(pWidget, pData, 0);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < pData->iTagCount; i++ ) {
		if ( pData->arrTags[i] != NULL ) {
			xrtFree(pData->arrTags[i]);
			pData->arrTags[i] = NULL;
		}
	}
	pData->iTagCount = 0;
	pData->iHoverClose = -1;
	pData->iActiveClose = -1;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	__xuiTagInputNotify(pWidget, pData);
	return XUI_OK;
}

XUI_API int xuiTagInputSetTags(xui_widget pWidget, const char* const* ppTags, int iCount)
{
	xui_tag_input_data_t* pData;
	char* arrNew[XUI_TAG_INPUT_TAG_CAPACITY];
	int i;
	int iNewCount;
	int iRet;

	pData = __xuiTagInputGetData(pWidget);
	if ( (pData == NULL) || (iCount < 0) || (iCount > XUI_TAG_INPUT_TAG_CAPACITY) ||
	     (iCount > 0 && ppTags == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(arrNew, 0, sizeof(arrNew));
	iNewCount = 0;
	for ( i = 0; i < iCount; i++ ) {
		char* sTag = __xuiTagInputTrimDuplicate(ppTags[i], -1);
		if ( sTag == NULL ) {
			iRet = XUI_ERROR_OUT_OF_MEMORY;
			goto failed;
		}
		if ( sTag[0] == '\0' ) {
			xrtFree(sTag);
			continue;
		}
		__xuiTagInputClampTag(pData, sTag);
		if ( sTag[0] == '\0' ) {
			xrtFree(sTag);
			continue;
		}
		if ( iNewCount >= pData->iMaxTags ) {
			xrtFree(sTag);
			iRet = XUI_ERROR_BUFFER_TOO_SMALL;
			goto failed;
		}
		arrNew[iNewCount++] = sTag;
	}
	if ( iNewCount == pData->iTagCount ) {
		for ( i = 0; i < iNewCount; i++ ) {
			if ( strcmp(arrNew[i], pData->arrTags[i]) != 0 ) break;
		}
		if ( i == iNewCount ) {
			for ( i = 0; i < iNewCount; i++ ) xrtFree(arrNew[i]);
			return XUI_OK;
		}
	}
	iRet = __xuiTagInputCancelPointer(pWidget, pData, 0);
	if ( iRet != XUI_OK ) goto failed;
	for ( i = 0; i < pData->iTagCount; i++ ) {
		if ( pData->arrTags[i] != NULL ) xrtFree(pData->arrTags[i]);
		pData->arrTags[i] = NULL;
	}
	for ( i = 0; i < iNewCount; i++ ) {
		pData->arrTags[i] = arrNew[i];
		arrNew[i] = NULL;
	}
	pData->iTagCount = iNewCount;
	pData->iHoverClose = -1;
	pData->iActiveClose = -1;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	__xuiTagInputNotify(pWidget, pData);
	return XUI_OK;

failed:
	for ( i = 0; i < XUI_TAG_INPUT_TAG_CAPACITY; i++ ) {
		if ( arrNew[i] != NULL ) xrtFree(arrNew[i]);
	}
	return iRet;
}

XUI_API int xuiTagInputGetTagCount(xui_widget pWidget)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	return (pData != NULL) ? pData->iTagCount : 0;
}

XUI_API const char* xuiTagInputGetTag(xui_widget pWidget, int iIndex)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iTagCount ) return NULL;
	return pData->arrTags[iIndex];
}

XUI_API int xuiTagInputSetText(xui_widget pWidget, const char* sText)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL || pData->pInput == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiTagInputSetInputText(pWidget, pData, sText);
}

XUI_API const char* xuiTagInputGetText(xui_widget pWidget)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	return (pData != NULL && pData->pInput != NULL) ? xuiInputGetText(pData->pInput) : "";
}

XUI_API int xuiTagInputCommit(xui_widget pWidget)
{
	xui_tag_input_data_t* pData;
	xui_context pContext;
	const char* sText;
	int iRet;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL || pData->pInput == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	sText = xuiInputGetText(pData->pInput);
	iRet = __xuiTagInputCommitText(pWidget, pData, sText, -1);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiTagInputAlive(pContext, pWidget, pData) ) return XUI_OK;
	return __xuiTagInputSetInputText(pWidget, pData, "");
}

XUI_API int xuiTagInputSetPlaceholder(xui_widget pWidget, const char* sText)
{
	xui_tag_input_data_t* pData;
	char* sNew;
	char* sOld;
	int iRet;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( sText == NULL ) sText = "";
	if ( pData->sPlaceholder != NULL && strcmp(pData->sPlaceholder, sText) == 0 ) return XUI_OK;
	sNew = __xuiTagInputDuplicateRange(sText, -1);
	if ( sNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	if ( pData->pInput != NULL ) {
		iRet = xuiInputSetPlaceholder(pData->pInput, sNew);
		if ( iRet != XUI_OK ) {
			xrtFree(sNew);
			return iRet;
		}
	}
	sOld = pData->sPlaceholder;
	pData->sPlaceholder = sNew;
	if ( sOld != NULL ) xrtFree(sOld);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API const char* xuiTagInputGetPlaceholder(xui_widget pWidget)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	return (pData != NULL && pData->sPlaceholder != NULL) ? pData->sPlaceholder : "";
}

XUI_API int xuiTagInputSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_tag_input_data_t* pData;
	xui_font pOldFont;
	int iRet;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pFont = (pFont != NULL) ? pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	if ( pData->pFont == pFont ) return XUI_OK;
	pOldFont = pData->pFont;
	pData->pFont = pFont;
	iRet = __xuiTagInputSyncInputStyle(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->pFont = pOldFont;
		(void)__xuiTagInputSyncInputStyle(pWidget, pData);
		return iRet;
	}
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API xui_font xuiTagInputGetFont(xui_widget pWidget)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API int xuiTagInputSetMaxTags(xui_widget pWidget, int iMaxTags)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL || iMaxTags < 0 || iMaxTags > XUI_TAG_INPUT_TAG_CAPACITY ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iMaxTags == 0 ) iMaxTags = XUI_TAG_INPUT_TAG_CAPACITY;
	if ( iMaxTags < pData->iTagCount ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iMaxTags == iMaxTags ) return XUI_OK;
	pData->iMaxTags = iMaxTags;
	return XUI_OK;
}

XUI_API int xuiTagInputGetMaxTags(xui_widget pWidget)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	return (pData != NULL) ? pData->iMaxTags : 0;
}

XUI_API int xuiTagInputSetMaxLength(xui_widget pWidget, int iMaxLength)
{
	xui_tag_input_data_t* pData;
	char* sTag;
	size_t iOldLength;
	int iOldMaxLength;
	int iOldTagCount;
	int iNewTagCount;
	int bTagsChanged;
	int i;
	int iRet;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL || iMaxLength < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iMaxLength == iMaxLength ) return XUI_OK;
	iRet = __xuiTagInputCancelPointer(pWidget, pData, 0);
	if ( iRet != XUI_OK ) return iRet;
	iOldMaxLength = pData->iMaxLength;
	pData->iMaxLength = iMaxLength;
	iRet = __xuiTagInputSyncInputStyle(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->iMaxLength = iOldMaxLength;
		(void)__xuiTagInputSyncInputStyle(pWidget, pData);
		return iRet;
	}
	iOldTagCount = pData->iTagCount;
	iNewTagCount = 0;
	bTagsChanged = 0;
	for ( i = 0; i < iOldTagCount; i++ ) {
		sTag = pData->arrTags[i];
		iOldLength = strlen(sTag);
		__xuiTagInputClampTag(pData, sTag);
		if ( sTag[0] == '\0' ) {
			xrtFree(sTag);
			bTagsChanged = 1;
			continue;
		}
		if ( strlen(sTag) != iOldLength ) bTagsChanged = 1;
		pData->arrTags[iNewTagCount++] = sTag;
	}
	for ( i = iNewTagCount; i < iOldTagCount; i++ ) pData->arrTags[i] = NULL;
	pData->iTagCount = iNewTagCount;
	if ( !bTagsChanged ) return XUI_OK;
	pData->iHoverClose = -1;
	pData->iActiveClose = -1;
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	xuiInternalAccessibilityQueue(pWidget,
		iNewTagCount == iOldTagCount ? XUI_ACCESSIBLE_EVENT_NODE_CHANGED :
		XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
	__xuiTagInputNotify(pWidget, pData);
	return XUI_OK;
}

XUI_API int xuiTagInputGetMaxLength(xui_widget pWidget)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	return (pData != NULL) ? pData->iMaxLength : 0;
}

XUI_API int xuiTagInputSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iBorder, uint32_t iFocusBorder, uint32_t iTagBackground, uint32_t iTagText)
{
	xui_tag_input_data_t* pData;
	uint32_t arrOld[8];
	int iRet;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	arrOld[0] = pData->iBackgroundColor;
	arrOld[1] = pData->iHoverBackgroundColor;
	arrOld[2] = pData->iFocusBackgroundColor;
	arrOld[3] = pData->iBorderColor;
	arrOld[4] = pData->iHoverBorderColor;
	arrOld[5] = pData->iFocusBorderColor;
	arrOld[6] = pData->iTagBackgroundColor;
	arrOld[7] = pData->iTagTextColor;
	pData->iBackgroundColor = iBackground;
	pData->iHoverBackgroundColor = iBackground;
	pData->iFocusBackgroundColor = iBackground;
	pData->iBorderColor = iBorder;
	pData->iHoverBorderColor = iFocusBorder;
	pData->iFocusBorderColor = iFocusBorder;
	pData->iTagBackgroundColor = iTagBackground;
	pData->iTagTextColor = iTagText;
	iRet = __xuiTagInputSyncInputStyle(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->iBackgroundColor = arrOld[0];
		pData->iHoverBackgroundColor = arrOld[1];
		pData->iFocusBackgroundColor = arrOld[2];
		pData->iBorderColor = arrOld[3];
		pData->iHoverBorderColor = arrOld[4];
		pData->iFocusBorderColor = arrOld[5];
		pData->iTagBackgroundColor = arrOld[6];
		pData->iTagTextColor = arrOld[7];
		(void)__xuiTagInputSyncInputStyle(pWidget, pData);
		return iRet;
	}
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiTagInputGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pBorder, uint32_t* pFocusBorder, uint32_t* pTagBackground, uint32_t* pTagText)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBackground != NULL ) *pBackground = pData->iBackgroundColor;
	if ( pBorder != NULL ) *pBorder = pData->iBorderColor;
	if ( pFocusBorder != NULL ) *pFocusBorder = pData->iFocusBorderColor;
	if ( pTagBackground != NULL ) *pTagBackground = pData->iTagBackgroundColor;
	if ( pTagText != NULL ) *pTagText = pData->iTagTextColor;
	return XUI_OK;
}

XUI_API int xuiTagInputSetExtendedColors(xui_widget pWidget, uint32_t iText, uint32_t iPlaceholder, uint32_t iDisabledText, uint32_t iHoverBackground, uint32_t iFocusBackground, uint32_t iDisabledBackground, uint32_t iHoverBorder, uint32_t iTagHoverBackground, uint32_t iTagClose, uint32_t iTagCloseHover)
{
	xui_tag_input_data_t* pData;
	uint32_t arrOld[10];
	int iRet;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	arrOld[0] = pData->iTextColor;
	arrOld[1] = pData->iPlaceholderColor;
	arrOld[2] = pData->iDisabledTextColor;
	arrOld[3] = pData->iHoverBackgroundColor;
	arrOld[4] = pData->iFocusBackgroundColor;
	arrOld[5] = pData->iDisabledBackgroundColor;
	arrOld[6] = pData->iHoverBorderColor;
	arrOld[7] = pData->iTagHoverBackgroundColor;
	arrOld[8] = pData->iTagCloseColor;
	arrOld[9] = pData->iTagCloseHoverColor;
	pData->iTextColor = iText;
	pData->iPlaceholderColor = iPlaceholder;
	pData->iDisabledTextColor = iDisabledText;
	pData->iHoverBackgroundColor = iHoverBackground;
	pData->iFocusBackgroundColor = iFocusBackground;
	pData->iDisabledBackgroundColor = iDisabledBackground;
	pData->iHoverBorderColor = iHoverBorder;
	pData->iTagHoverBackgroundColor = iTagHoverBackground;
	pData->iTagCloseColor = iTagClose;
	pData->iTagCloseHoverColor = iTagCloseHover;
	iRet = __xuiTagInputSyncInputStyle(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->iTextColor = arrOld[0];
		pData->iPlaceholderColor = arrOld[1];
		pData->iDisabledTextColor = arrOld[2];
		pData->iHoverBackgroundColor = arrOld[3];
		pData->iFocusBackgroundColor = arrOld[4];
		pData->iDisabledBackgroundColor = arrOld[5];
		pData->iHoverBorderColor = arrOld[6];
		pData->iTagHoverBackgroundColor = arrOld[7];
		pData->iTagCloseColor = arrOld[8];
		pData->iTagCloseHoverColor = arrOld[9];
		(void)__xuiTagInputSyncInputStyle(pWidget, pData);
		return iRet;
	}
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiTagInputGetExtendedColors(xui_widget pWidget, uint32_t* pText, uint32_t* pPlaceholder, uint32_t* pDisabledText, uint32_t* pHoverBackground, uint32_t* pFocusBackground, uint32_t* pDisabledBackground, uint32_t* pHoverBorder, uint32_t* pTagHoverBackground, uint32_t* pTagClose, uint32_t* pTagCloseHover)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pText != NULL ) *pText = pData->iTextColor;
	if ( pPlaceholder != NULL ) *pPlaceholder = pData->iPlaceholderColor;
	if ( pDisabledText != NULL ) *pDisabledText = pData->iDisabledTextColor;
	if ( pHoverBackground != NULL ) *pHoverBackground = pData->iHoverBackgroundColor;
	if ( pFocusBackground != NULL ) *pFocusBackground = pData->iFocusBackgroundColor;
	if ( pDisabledBackground != NULL ) *pDisabledBackground = pData->iDisabledBackgroundColor;
	if ( pHoverBorder != NULL ) *pHoverBorder = pData->iHoverBorderColor;
	if ( pTagHoverBackground != NULL ) *pTagHoverBackground = pData->iTagHoverBackgroundColor;
	if ( pTagClose != NULL ) *pTagClose = pData->iTagCloseColor;
	if ( pTagCloseHover != NULL ) *pTagCloseHover = pData->iTagCloseHoverColor;
	return XUI_OK;
}

XUI_API int xuiTagInputSetVisualMetrics(xui_widget pWidget, float fBorderWidth, float fTagHeight)
{
	xui_tag_input_data_t* pData;
	float fOldBorderWidth;
	float fOldTagHeight;
	int iRet;

	if ( !isfinite(fBorderWidth) || !isfinite(fTagHeight) ||
	     (fBorderWidth < 0.0f) || (fTagHeight < 0.0f) ||
	     (fBorderWidth > XUI_LAYOUT_UNBOUNDED) || (fTagHeight > XUI_LAYOUT_UNBOUNDED) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	fTagHeight = (fTagHeight > 0.0f) ? fTagHeight : 24.0f;
	if ( pData->fBorderWidth == fBorderWidth && pData->fTagHeight == fTagHeight ) return XUI_OK;
	fOldBorderWidth = pData->fBorderWidth;
	fOldTagHeight = pData->fTagHeight;
	pData->fBorderWidth = fBorderWidth;
	pData->fTagHeight = fTagHeight;
	iRet = __xuiTagInputSyncInputStyle(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->fBorderWidth = fOldBorderWidth;
		pData->fTagHeight = fOldTagHeight;
		(void)__xuiTagInputSyncInputStyle(pWidget, pData);
		return iRet;
	}
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiTagInputGetVisualMetrics(xui_widget pWidget, float* pBorderWidth, float* pTagHeight)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBorderWidth != NULL ) *pBorderWidth = pData->fBorderWidth;
	if ( pTagHeight != NULL ) *pTagHeight = pData->fTagHeight;
	return XUI_OK;
}

XUI_API xui_rect_t xuiTagInputGetTagRect(xui_widget pWidget, int iIndex)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iTagCount ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return pData->arrTagRect[iIndex];
}

XUI_API xui_rect_t xuiTagInputGetCloseRect(xui_widget pWidget, int iIndex)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iTagCount ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return pData->arrCloseRect[iIndex];
}

XUI_API xui_rect_t xuiTagInputGetInputRect(xui_widget pWidget)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	return (pData != NULL) ? pData->tInputRect : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

XUI_API xui_widget xuiTagInputGetInputWidget(xui_widget pWidget)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	return (pData != NULL) ? pData->pInput : NULL;
}

XUI_API uint32_t xuiTagInputGetState(xui_widget pWidget)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	if ( pData == NULL ) return 0;
	(void)__xuiTagInputSyncState(pWidget, pData);
	return __xuiTagInputComputeState(pWidget, pData);
}

XUI_API int xuiTagInputGetChangeCount(xui_widget pWidget)
{
	xui_tag_input_data_t* pData;

	pData = __xuiTagInputGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

#endif
