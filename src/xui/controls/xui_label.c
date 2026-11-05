#include "xui_config.h"
#if XUI_ENABLE_LABEL
#include "xui_internal.h"
#include "xui_text_internal.h"

#include <limits.h>
#include <math.h>
#include <string.h>

#define XUI_LABEL_MEASURE_CACHE_CAPACITY 4

typedef struct xui_label_measure_key_t {
	xui_font pFont;
	uint32_t iTextVersion;
	uint32_t iContentGeneration;
	uint32_t iDpiGeneration;
	uint32_t iTextFlags;
	int iWrapMode;
	float fLineGap;
	float fParagraphGap;
} xui_label_measure_key_t;

typedef struct xui_label_measure_entry_t {
	xui_vec2_t tConstraint;
	xui_vec2_t tSize;
} xui_label_measure_entry_t;

typedef struct xui_label_data_t {
    xui_text_layout pLayout; /* actual font/paint owner, not a temporary */
	char* sText;
	int iTextCapacity;
	int iTextSize;
	uint32_t iTextVersion;
	xui_font pFont;
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iTextFlags;
	int iWrapMode;
	int bUnderline;
	float fLineGap;
	float fParagraphGap;
	xui_label_measure_key_t tMeasureKey;
	xui_label_measure_entry_t tMeasureCache[XUI_LABEL_MEASURE_CACHE_CAPACITY];
	int iMeasureCacheCount;
	int bMeasureKeyValid;
	int bMeasuring;
} xui_label_data_t;

static int __xuiLabelWrapModeValid(int iWrapMode)
{
	return (iWrapMode == XUI_TEXT_WRAP_NONE) ||
	       (iWrapMode == XUI_TEXT_WRAP_WORD) ||
	       (iWrapMode == XUI_TEXT_WRAP_CHAR);
}

static int __xuiLabelDescValid(const xui_label_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) {
		return 0;
	}
	if ( !__xuiLabelWrapModeValid(pDesc->iWrapMode) ) {
		return 0;
	}
	if ( !isfinite(pDesc->fLineGap) || !isfinite(pDesc->fParagraphGap) ||
	     (pDesc->fLineGap < 0.0f) || (pDesc->fLineGap > XUI_LAYOUT_UNBOUNDED) ||
	     (pDesc->fParagraphGap < 0.0f) || (pDesc->fParagraphGap > XUI_LAYOUT_UNBOUNDED) ) {
		return 0;
	}
	return 1;
}

static int __xuiLabelTextSet(xui_label_data_t* pData, const char* sText)
{
	size_t iLength;
	int iNeed;
	int iRet;

	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( sText == NULL ) {
		sText = "";
	}
	iLength = strlen(sText);
	if ( iLength >= (size_t)INT_MAX ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNeed = (int)iLength + 1;
	iRet = xuiInternalTextBufferReserve(&pData->sText,
		&pData->iTextCapacity, iNeed);
	if ( iRet != XUI_OK ) return iRet;
	memmove(pData->sText, sText, (size_t)iNeed);
	pData->iTextSize = (int)iLength;
	if ( ++pData->iTextVersion == 0u ) pData->iTextVersion = 1u;
	pData->iMeasureCacheCount = 0;
	pData->bMeasureKeyValid = 0;
	return XUI_OK;
}

static xui_label_data_t* __xuiLabelGetData(xui_widget pWidget)
{
    if(xuiInternalWidgetIsValid(pWidget) && pWidget->pContext->bCloseRequested)return NULL;
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return NULL;
	}
	pContext = xuiWidgetGetContext(pWidget);
	pType = xuiWidgetFindType(pContext, "label");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_label_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiLabelStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
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

static int __xuiLabelStyleInt(xui_widget pWidget, const char* sName, int* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) &&
	     ((tProperty.tValue.iType == XUI_STYLE_VALUE_INT) || (tProperty.tValue.iType == XUI_STYLE_VALUE_BOOL)) ) {
		*pValue = tProperty.tValue.iInt;
		return 1;
	}
	return 0;
}

static int __xuiLabelStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) &&
	     isfinite(tProperty.tValue.fFloat) && (tProperty.tValue.fFloat >= 0.0f) &&
	     (tProperty.tValue.fFloat <= XUI_LAYOUT_UNBOUNDED) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static xui_font __xuiLabelStyleFont(xui_widget pWidget, xui_font pBaseFont)
{
	xui_style_property_t tProperty;
	xui_context pContext;
	xui_font pFont;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, "font.name", &tProperty);
	if ( (iRet == XUI_OK) &&
	     (tProperty.tValue.iType == XUI_STYLE_VALUE_STRING) &&
	     (tProperty.tValue.sText != NULL) ) {
		pContext = xuiWidgetGetContext(pWidget);
		pFont = xuiFindFont(pContext, tProperty.tValue.sText);
		if ( pFont != NULL ) {
			return pFont;
		}
	}
	return pBaseFont;
}

static void __xuiLabelResolve(xui_widget pWidget, xui_label_data_t* pData, xui_text_layout_desc_t* pDesc, uint32_t* pTextColor)
{
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iTextFlags;
	int iWrapMode;
	int iStyleTextFlags;
	int bUnderline;
	float fLineGap;
	float fParagraphGap;
	xui_font pFont;

	iTextColor = pData->iTextColor;
	iDisabledTextColor = pData->iDisabledTextColor;
	iTextFlags = pData->iTextFlags | XUI_TEXT_CLIP;
	iWrapMode = pData->iWrapMode;
	iStyleTextFlags = (int)iTextFlags;
	bUnderline = pData->bUnderline;
	fLineGap = pData->fLineGap;
	fParagraphGap = pData->fParagraphGap;
	pFont = (pData->pFont != NULL) ? pData->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));

	(void)__xuiLabelStyleColor(pWidget, "text.color", &iTextColor);
	(void)__xuiLabelStyleColor(pWidget, "text.disabled_color", &iDisabledTextColor);
	if ( __xuiLabelStyleInt(pWidget, "text.flags", &iStyleTextFlags) ) {
		iTextFlags = (uint32_t)iStyleTextFlags;
	}
	(void)__xuiLabelStyleInt(pWidget, "text.wrap", &iWrapMode);
	(void)__xuiLabelStyleInt(pWidget, "text.underline", &bUnderline);
	(void)__xuiLabelStyleFloat(pWidget, "text.line_gap", &fLineGap);
	(void)__xuiLabelStyleFloat(pWidget, "text.paragraph_gap", &fParagraphGap);
	pFont = __xuiLabelStyleFont(pWidget, pFont);

	if ( !__xuiLabelWrapModeValid(iWrapMode) ) {
		iWrapMode = XUI_TEXT_WRAP_NONE;
	}
	if ( fLineGap < 0.0f ) {
		fLineGap = 0.0f;
	}
	if ( fParagraphGap < 0.0f ) {
		fParagraphGap = 0.0f;
	}
	if ( bUnderline ) {
		iTextFlags |= XUI_TEXT_UNDERLINE;
	}
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iSize = sizeof(*pDesc);
	pDesc->sText = (pData->sText != NULL) ? pData->sText : "";
	pDesc->iTextSize = pData->iTextSize;
	pDesc->pFont = pFont;
	pDesc->iWrapMode = iWrapMode;
	pDesc->iFlags = iTextFlags | XUI_TEXT_CLIP;
	pDesc->fLineGap = fLineGap;
	pDesc->fParagraphGap = fParagraphGap;
	*pTextColor = xuiWidgetGetEffectiveEnabled(pWidget) ? iTextColor : iDisabledTextColor;
}

static xui_label_measure_key_t __xuiLabelMeasureKey(xui_widget pWidget,
	xui_label_data_t* pData, const xui_text_layout_desc_t* pDesc)
{
	xui_label_measure_key_t tKey = {0};
	tKey.pFont = pDesc->pFont;
	tKey.iTextVersion = pData->iTextVersion;
	tKey.iContentGeneration = pWidget->iContentMeasureGeneration;
	tKey.iDpiGeneration = pWidget->pContext->iDpiGeneration;
	tKey.iTextFlags = pDesc->iFlags;
	tKey.iWrapMode = pDesc->iWrapMode;
	tKey.fLineGap = pDesc->fLineGap;
	tKey.fParagraphGap = pDesc->fParagraphGap;
	return tKey;
}

static int __xuiLabelMeasureKeyEqual(const xui_label_measure_key_t* a,
	const xui_label_measure_key_t* b)
{
	return a->pFont == b->pFont && a->iTextVersion == b->iTextVersion &&
		a->iContentGeneration == b->iContentGeneration && a->iDpiGeneration == b->iDpiGeneration &&
		a->iTextFlags == b->iTextFlags && a->iWrapMode == b->iWrapMode &&
		a->fLineGap == b->fLineGap && a->fParagraphGap == b->fParagraphGap;
}

static int __xuiLabelContentMeasureImpl(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_label_data_t* pData;
	xui_text_layout_desc_t tDesc;
	xui_label_measure_key_t tKey, tCurrentKey;
	xui_label_measure_entry_t tEntry;
	uint32_t iTextColor;
	int iRet, i;

	(void)pUser;
	if ( (pWidget == NULL) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSize->fX = 0.0f;
	pSize->fY = 0.0f;
	pData = __xuiLabelGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->bMeasuring ) return XUI_ERROR_INVALID_STATE;
	__xuiLabelResolve(pWidget, pData, &tDesc, &iTextColor);
	if ( tDesc.pFont == NULL ) {
		return XUI_OK;
	}
	tKey = __xuiLabelMeasureKey(pWidget, pData, &tDesc);
	if ( !pData->bMeasureKeyValid || !__xuiLabelMeasureKeyEqual(&pData->tMeasureKey, &tKey) ) {
		pData->tMeasureKey = tKey;
		pData->bMeasureKeyValid = 1;
		pData->iMeasureCacheCount = 0;
	}
	for ( i = 0; i < pData->iMeasureCacheCount; i++ ) {
		if ( pData->tMeasureCache[i].tConstraint.fX == tConstraint.fX &&
		     pData->tMeasureCache[i].tConstraint.fY == tConstraint.fY ) {
			tEntry = pData->tMeasureCache[i];
			if ( i > 0 ) memmove(pData->tMeasureCache + 1, pData->tMeasureCache, (size_t)i * sizeof(tEntry));
			pData->tMeasureCache[0] = tEntry;
			*pSize = tEntry.tSize;
			return XUI_OK;
		}
	}
	tDesc.fMaxWidth = (tDesc.iWrapMode == XUI_TEXT_WRAP_NONE) ? XUI_LAYOUT_UNBOUNDED : tConstraint.fX;
	tDesc.fMaxHeight = tConstraint.fY;
	if ( (tDesc.fMaxWidth <= 0.0f) || (tDesc.fMaxWidth > XUI_LAYOUT_UNBOUNDED) ) {
		tDesc.fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	}
	if ( (tDesc.fMaxHeight <= 0.0f) || (tDesc.fMaxHeight > XUI_LAYOUT_UNBOUNDED) ) {
		tDesc.fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	}
	pData->bMeasuring = 1;
	iRet = xuiInternalTextLayoutEnsure(xuiWidgetGetContext(pWidget), &pData->pLayout, &tDesc);
    if(iRet==XUI_OK)*pSize=xuiTextLayoutGetSize(pData->pLayout);
	/* Proxy callbacks may change or retire the label while shaping text. */
	if ( !xuiInternalWidgetIsValid(pWidget) || pWidget->bDestroyPending || pWidget->pTypeData != pData ) {
		return XUI_ERROR_INVALID_STATE;
	}
	pData->bMeasuring = 0;
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( (tConstraint.fX >= 0.0f) && (tConstraint.fX < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fX > tConstraint.fX) ) pSize->fX = tConstraint.fX;
	if ( (tConstraint.fY >= 0.0f) && (tConstraint.fY < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fY > tConstraint.fY) ) pSize->fY = tConstraint.fY;
	__xuiLabelResolve(pWidget, pData, &tDesc, &iTextColor);
	tCurrentKey = __xuiLabelMeasureKey(pWidget, pData, &tDesc);
	if ( pData->bMeasureKeyValid && __xuiLabelMeasureKeyEqual(&tKey, &tCurrentKey) ) {
        /* Four numeric results cover alternating constraints. Their Font
         * identity is protected by the same native owned Layout slot. */
		i = pData->iMeasureCacheCount;
		if ( i < XUI_LABEL_MEASURE_CACHE_CAPACITY ) pData->iMeasureCacheCount++;
		else i = XUI_LABEL_MEASURE_CACHE_CAPACITY - 1;
		if ( i > 0 ) memmove(pData->tMeasureCache + 1, pData->tMeasureCache, (size_t)i * sizeof(tEntry));
		pData->tMeasureCache[0].tConstraint = tConstraint;
		pData->tMeasureCache[0].tSize = *pSize;
	}
	return XUI_OK;
}
static int __xuiLabelContentMeasure(xui_widget widget,xui_vec2_t constraint,xui_vec2_t* size,void* user)
{
    xui_label_data_t* data=__xuiLabelGetData(widget);if(!data)return XUI_ERROR_INVALID_ARGUMENT;
    if(data->bMeasuring)return XUI_ERROR_BUSY;
    xui_context context=widget->pContext;xui_call_scope_t scope={0};
    int status=xuiCallScopeBegin(context,&scope);if(status!=XUI_OK)return status;
    status=__xuiLabelContentMeasureImpl(widget,constraint,size,user);
    data->bMeasuring=0;
    if(status==XUI_OK && (!xuiInternalWidgetIsValid(widget) || context->bCloseRequested || context->bDestroyPending))status=XUI_ERROR_INVALID_STATE;
    xerror* first=xrtTakeError();status=xuiInternalCallScopeEndOutcome(&scope,status,&first);xrtSetErrorTake(first);return status;
}


static int __xuiLabelCacheRenderImpl(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_label_data_t* pData;
	xui_text_layout_desc_t tDesc;
	xui_rect_t tContent;
	xui_proxy pProxy;
	uint32_t iTextColor;
	int iRet;

	(void)iStateId;
	(void)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiLabelGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiLabelResolve(pWidget, pData, &tDesc, &iTextColor);
	if ( (tDesc.pFont == NULL) || (pProxy->drawText == NULL) || ((iTextColor & 0xffu) == 0u) ||
	     (tDesc.sText == NULL) || (tDesc.sText[0] == '\0') ) {
		return XUI_OK;
	}
	tContent = xuiWidgetGetContentRect(pWidget);
	if ( (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) {
		return XUI_OK;
	}
	tDesc.fMaxWidth = tContent.fW;
	tDesc.fMaxHeight = tContent.fH;
    if(pData->bMeasureKeyValid && pData->tMeasureKey.pFont!=tDesc.pFont)pData->bMeasureKeyValid=0;
    iRet=xuiInternalTextLayoutEnsure(xuiWidgetGetContext(pWidget),&pData->pLayout,&tDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiInternalTextLayoutDrawContext(pData->pLayout, pDraw, tContent,
		iTextColor, 0);
	return iRet;
}
static int __xuiLabelCacheRender(xui_widget widget,xui_draw_context draw,uint32_t state,void* user)
{
    xui_label_data_t* data=__xuiLabelGetData(widget);if(!data)return XUI_ERROR_INVALID_ARGUMENT;
    if(data->bMeasuring)return XUI_ERROR_BUSY;
    xui_context context=widget->pContext;xui_call_scope_t scope={0};
    int status=xuiCallScopeBegin(context,&scope);if(status!=XUI_OK)return status;
    data->bMeasuring=2;
    status=__xuiLabelCacheRenderImpl(widget,draw,state,user);
    data->bMeasuring=0;
    if(status==XUI_OK && (!xuiInternalWidgetIsValid(widget) || context->bCloseRequested || context->bDestroyPending))status=XUI_ERROR_INVALID_STATE;
    xerror* first=xrtTakeError();status=xuiInternalCallScopeEndOutcome(&scope,status,&first);xrtSetErrorTake(first);return status;
}


static int __xuiLabelInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_label_data_t* pData;
	const xui_label_desc_t* pDesc;
	xui_theme_t tTheme;
	xui_context pContext;
	int iRet;

	(void)pUser;
	pData = (xui_label_data_t*)pTypeData;
	pDesc = (const xui_label_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiLabelDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	memset(&tTheme, 0, sizeof(tTheme));
	tTheme.iSize = sizeof(tTheme);
	(void)xuiGetTheme(pContext, &tTheme);
	pData->pFont = (pDesc != NULL && pDesc->pFont != NULL) ? pDesc->pFont : xuiGetDefaultFont(pContext);
	pData->iTextColor = (pDesc != NULL && pDesc->iTextColor != 0) ? pDesc->iTextColor : tTheme.iTextColor;
	pData->iDisabledTextColor = (pDesc != NULL && pDesc->iDisabledTextColor != 0) ? pDesc->iDisabledTextColor : tTheme.iStateDisabledColor;
	pData->iTextFlags = ((pDesc != NULL && pDesc->iTextFlags != 0) ? pDesc->iTextFlags : (XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP)) | XUI_TEXT_CLIP;
	pData->iWrapMode = (pDesc != NULL) ? pDesc->iWrapMode : XUI_TEXT_WRAP_NONE;
	pData->bUnderline = (pDesc != NULL) ? (pDesc->bUnderline ? 1 : 0) : 0;
	pData->fLineGap = (pDesc != NULL) ? pDesc->fLineGap : 0.0f;
	pData->fParagraphGap = (pDesc != NULL) ? pDesc->fParagraphGap : 0.0f;
	iRet = __xuiLabelTextSet(pData, (pDesc != NULL) ? pDesc->sText : "");
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiWidgetSetFocusable(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetHitTestVisible(pWidget, 0);
	return iRet;
}

static void __xuiLabelDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_label_data_t* pData;

	(void)pWidget;
	(void)pUser;
	pData = (xui_label_data_t*)pTypeData;
	if ( pData == NULL ) {
		return;
	}
    if(pData->pLayout){bool consumed=false;int status=xuiTextLayoutDestroyOwnerChecked(pData->pLayout,&consumed);if(status<0)xuiInternalContextRecordFailure(pWidget->pContext,status);if(consumed)pData->pLayout=NULL;}
	if ( pData->sText != NULL ) {
		xrtFree(pData->sText);
	}
	memset(pData, 0, sizeof(*pData));
}

static void __xuiLabelDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_MANUAL;
	pLayout->iWidthMode = XUI_SIZE_CONTENT;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_VISIBLE;
	pLayout->iAlignX = XUI_ALIGN_START;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static void __xuiLabelDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static void __xuiLabelRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
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

static void __xuiLabelRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	__xuiLabelRegisterStyleProperty(pContext, pType, "text.color", XUI_STYLE_VALUE_COLOR, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiLabelRegisterStyleProperty(pContext, pType, "text.disabled_color", XUI_STYLE_VALUE_COLOR, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiLabelRegisterStyleProperty(pContext, pType, "text.flags", XUI_STYLE_VALUE_INT, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiLabelRegisterStyleProperty(pContext, pType, "text.wrap", XUI_STYLE_VALUE_INT, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiLabelRegisterStyleProperty(pContext, pType, "text.underline", XUI_STYLE_VALUE_BOOL, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiLabelRegisterStyleProperty(pContext, pType, "text.line_gap", XUI_STYLE_VALUE_FLOAT, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiLabelRegisterStyleProperty(pContext, pType, "text.paragraph_gap", XUI_STYLE_VALUE_FLOAT, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiLabelRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
}

static int __xuiLabelAccessibleNode(xui_widget pWidget, xui_accessible_node_t* pNode)
{
	pNode->iRole = XUI_ACCESSIBLE_ROLE_TEXT;
	pNode->sName = xuiWidgetGetAccessibleName(pWidget) != NULL ?
		xuiWidgetGetAccessibleName(pWidget) : xuiLabelGetText(pWidget);
	return XUI_OK;
}

static const xui_internal_accessibility_adapter_t g_xuiLabelAccessible = {.getNode = __xuiLabelAccessibleNode, .performAction = NULL};

static bool __xuiLabelTrace(xui_widget widget,const void* raw,xrtownershipvisitor visit,void* user)
{
    (void)widget;const xui_label_data_t* data=raw;
    return !data->bMeasuring && (!data->pLayout || xuiTextLayoutTraceOwnership(data->pLayout,visit,user));
}
static int __xuiLabelReady(xui_widget widget,const void* raw)
{
    (void)widget;const xui_label_data_t* data=raw;
    if(data->bMeasuring)return XUI_ERROR_BUSY;
    return data->pLayout ? xuiInternalTextLayoutDestroyReady(data->pLayout):XUI_OK;
}
static int __xuiLabelPrepare(xui_widget widget,void* raw,bool* complete)
{
    (void)widget;xui_label_data_t* data=raw;if(!complete)return XUI_ERROR_INVALID_ARGUMENT;
    *complete=false;if(data->bMeasuring)return XUI_STATUS_DEFERRED;
    if(!data->pLayout){*complete=true;return XUI_OK;}
    data->bMeasuring=1;bool consumed=false;int status=xuiTextLayoutDestroyOwnerChecked(data->pLayout,&consumed);
    if(consumed)data->pLayout=NULL;
    data->bMeasuring=0;*complete=consumed;return status;
}
XUI_API xui_widget_type xuiLabelGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "label");
	if ( pType != NULL ) {
		__xuiLabelRegisterStyleProperties(pContext, pType);
		pType->pAccessibleAdapter = &g_xuiLabelAccessible;
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "label";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_label_data_t);
	tDesc.onInit = __xuiLabelInit;
	tDesc.onDestroy = __xuiLabelDestroy;
	tDesc.onContentMeasure = __xuiLabelContentMeasure;
	tDesc.onCacheRender = __xuiLabelCacheRender;
	__xuiLabelDefaultLayout(&tDesc.tLayout);
	__xuiLabelDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	__xuiLabelRegisterStyleProperties(pContext, pType);
	pType->pAccessibleAdapter = &g_xuiLabelAccessible;
	    static const size_t fontOffsets[]={offsetof(xui_label_data_t,pFont)};
    pType->pFontOffsets=fontOffsets;pType->iFontSlotCount=sizeof(fontOffsets)/sizeof(fontOffsets[0]);
    pType->onTraceOwnership=__xuiLabelTrace;pType->onDestroyReady=__xuiLabelReady;pType->onDestroyPrepare=__xuiLabelPrepare;
    return pType;
}

static int __xuiLabelCreateWithOwner(xui_context pContext, xui_widget* ppWidget, const xui_label_desc_t* pDesc,xui_widget pNativeOwner)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppWidget = NULL;
	if ( !__xuiLabelDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = xuiLabelGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	return (pNativeOwner ? xuiInternalWidgetCreatePart(pNativeOwner,pType,ppWidget,pDesc) : xuiWidgetCreateTyped(pContext,pType,ppWidget,pDesc));
}

XUI_API int xuiLabelCreate(xui_context pContext, xui_widget* ppWidget, const xui_label_desc_t* pDesc)
{return __xuiLabelCreateWithOwner(pContext,ppWidget,pDesc,NULL);}
int xuiInternalLabelCreatePart(xui_widget pOwner,xui_widget* ppWidget,const xui_label_desc_t* pDesc)
{
    if(ppWidget)*ppWidget=NULL;
    if(!xuiInternalWidgetIsValid(pOwner) || !ppWidget)return XUI_ERROR_INVALID_ARGUMENT;
    xui_context context=xuiWidgetGetContext(pOwner);
    xuiInternalOperationEnter(context);
    int status=__xuiLabelCreateWithOwner(context,ppWidget,pDesc,pOwner);
    xuiInternalOperationLeave(context);
    return status;
}


XUI_API int xuiLabelSetText(xui_widget pWidget, const char* sText)
{
	xui_label_data_t* pData;
	int iRet;

	pData = __xuiLabelGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
    if(pData->bMeasuring)return XUI_ERROR_BUSY;
	if ( sText == NULL ) {
		sText = "";
	}
	if ( (pData->sText != NULL) && (strcmp(pData->sText, sText) == 0) ) {
		return XUI_OK;
	}
	iRet = __xuiLabelTextSet(pData, sText);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API const char* xuiLabelGetText(xui_widget pWidget)
{
	xui_label_data_t* pData;

	pData = __xuiLabelGetData(pWidget);
	return (pData != NULL && pData->sText != NULL) ? pData->sText : NULL;
}

XUI_API int xuiLabelSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_label_data_t* pData;

	pData = __xuiLabelGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
    if(pData->bMeasuring)return XUI_ERROR_BUSY;
	if ( pData->pFont == pFont ) {
		return XUI_OK;
	}
	xui_font_slot_update_t fontUpdate={0};
    int fontStatus=xuiInternalFontSlotBegin(pWidget->pContext,&pData->pFont,pFont,pWidget->bTypeInitialized!=0,&fontUpdate);
    if(fontStatus!=XUI_OK)return fontStatus;
	return xuiInternalFontSlotEnd(&fontUpdate,true,xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER));
}

XUI_API xui_font xuiLabelGetFont(xui_widget pWidget)
{
	xui_label_data_t* pData;

	pData = __xuiLabelGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API int xuiLabelSetTextColor(xui_widget pWidget, uint32_t iColor)
{
	xui_label_data_t* pData;

	pData = __xuiLabelGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iTextColor == iColor ) {
		return XUI_OK;
	}
	pData->iTextColor = iColor;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API uint32_t xuiLabelGetTextColor(xui_widget pWidget)
{
	xui_label_data_t* pData;

	pData = __xuiLabelGetData(pWidget);
	return (pData != NULL) ? pData->iTextColor : 0;
}

XUI_API int xuiLabelSetDisabledTextColor(xui_widget pWidget, uint32_t iColor)
{
	xui_label_data_t* pData;

	pData = __xuiLabelGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iDisabledTextColor == iColor ) {
		return XUI_OK;
	}
	pData->iDisabledTextColor = iColor;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API uint32_t xuiLabelGetDisabledTextColor(xui_widget pWidget)
{
	xui_label_data_t* pData;

	pData = __xuiLabelGetData(pWidget);
	return (pData != NULL) ? pData->iDisabledTextColor : 0;
}

XUI_API int xuiLabelSetTextFlags(xui_widget pWidget, uint32_t iTextFlags)
{
	xui_label_data_t* pData;
	uint32_t iDirtyFlags;

	pData = __xuiLabelGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iTextFlags |= XUI_TEXT_CLIP;
	if ( pData->iTextFlags == iTextFlags ) {
		return XUI_OK;
	}
	iDirtyFlags = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	if ( ((pData->iTextFlags ^ iTextFlags) & XUI_TEXT_RTL) != 0 ) {
		iDirtyFlags |= XUI_WIDGET_DIRTY_LAYOUT;
	}
	pData->iTextFlags = iTextFlags;
	return xuiWidgetInvalidate(pWidget, iDirtyFlags);
}

XUI_API uint32_t xuiLabelGetTextFlags(xui_widget pWidget)
{
	xui_label_data_t* pData;

	pData = __xuiLabelGetData(pWidget);
	return (pData != NULL) ? pData->iTextFlags : 0;
}

XUI_API int xuiLabelSetWrapMode(xui_widget pWidget, int iWrapMode)
{
	xui_label_data_t* pData;

	if ( !__xuiLabelWrapModeValid(iWrapMode) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiLabelGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iWrapMode == iWrapMode ) {
		return XUI_OK;
	}
	pData->iWrapMode = iWrapMode;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiLabelGetWrapMode(xui_widget pWidget)
{
	xui_label_data_t* pData;

	pData = __xuiLabelGetData(pWidget);
	return (pData != NULL) ? pData->iWrapMode : XUI_TEXT_WRAP_NONE;
}

XUI_API int xuiLabelSetUnderline(xui_widget pWidget, int bUnderline)
{
	xui_label_data_t* pData;

	pData = __xuiLabelGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	bUnderline = bUnderline ? 1 : 0;
	if ( pData->bUnderline == bUnderline ) {
		return XUI_OK;
	}
	pData->bUnderline = bUnderline;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiLabelGetUnderline(xui_widget pWidget)
{
	xui_label_data_t* pData;

	pData = __xuiLabelGetData(pWidget);
	return (pData != NULL) ? pData->bUnderline : 0;
}

XUI_API int xuiLabelSetLineGap(xui_widget pWidget, float fLineGap)
{
	xui_label_data_t* pData;

	if ( !isfinite(fLineGap) || fLineGap < 0.0f || fLineGap > XUI_LAYOUT_UNBOUNDED ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiLabelGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->fLineGap == fLineGap ) {
		return XUI_OK;
	}
	pData->fLineGap = fLineGap;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API float xuiLabelGetLineGap(xui_widget pWidget)
{
	xui_label_data_t* pData;

	pData = __xuiLabelGetData(pWidget);
	return (pData != NULL) ? pData->fLineGap : 0.0f;
}

XUI_API int xuiLabelSetParagraphGap(xui_widget pWidget, float fParagraphGap)
{
	xui_label_data_t* pData;

	if ( !isfinite(fParagraphGap) || fParagraphGap < 0.0f || fParagraphGap > XUI_LAYOUT_UNBOUNDED ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiLabelGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->fParagraphGap == fParagraphGap ) {
		return XUI_OK;
	}
	pData->fParagraphGap = fParagraphGap;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API float xuiLabelGetParagraphGap(xui_widget pWidget)
{
	xui_label_data_t* pData;

	pData = __xuiLabelGetData(pWidget);
	return (pData != NULL) ? pData->fParagraphGap : 0.0f;
}

#endif
