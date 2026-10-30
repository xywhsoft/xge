#include "xui_config.h"
#if XUI_ENABLE_LABEL
#include "xui_internal.h"
#include "xui_text_internal.h"

#include <limits.h>
#include <math.h>
#include <string.h>

typedef struct xui_label_data_t {
	char* sText;
	int iTextCapacity;
	xui_font pFont;
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iTextFlags;
	int iWrapMode;
	int bUnderline;
	float fLineGap;
	float fParagraphGap;
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
	return XUI_OK;
}

static xui_label_data_t* __xuiLabelGetData(xui_widget pWidget)
{
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
	pDesc->iTextSize = -1;
	pDesc->pFont = pFont;
	pDesc->iWrapMode = iWrapMode;
	pDesc->iFlags = iTextFlags | XUI_TEXT_CLIP;
	pDesc->fLineGap = fLineGap;
	pDesc->fParagraphGap = fParagraphGap;
	*pTextColor = xuiWidgetGetEffectiveEnabled(pWidget) ? iTextColor : iDisabledTextColor;
}

static int __xuiLabelContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_label_data_t* pData;
	xui_text_layout_desc_t tDesc;
	uint32_t iTextColor;
	int iRet;

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
	__xuiLabelResolve(pWidget, pData, &tDesc, &iTextColor);
	if ( tDesc.pFont == NULL ) {
		return XUI_OK;
	}
	tDesc.fMaxWidth = (tDesc.iWrapMode == XUI_TEXT_WRAP_NONE) ? XUI_LAYOUT_UNBOUNDED : tConstraint.fX;
	tDesc.fMaxHeight = tConstraint.fY;
	if ( (tDesc.fMaxWidth <= 0.0f) || (tDesc.fMaxWidth > XUI_LAYOUT_UNBOUNDED) ) {
		tDesc.fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	}
	if ( (tDesc.fMaxHeight <= 0.0f) || (tDesc.fMaxHeight > XUI_LAYOUT_UNBOUNDED) ) {
		tDesc.fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	}
	iRet = xuiTextMeasureLayout(xuiWidgetGetContext(pWidget), &tDesc, pSize);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( (tConstraint.fX >= 0.0f) && (tConstraint.fX < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fX > tConstraint.fX) ) pSize->fX = tConstraint.fX;
	if ( (tConstraint.fY >= 0.0f) && (tConstraint.fY < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fY > tConstraint.fY) ) pSize->fY = tConstraint.fY;
	return XUI_OK;
}

static int __xuiLabelCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_label_data_t* pData;
	xui_text_layout_desc_t tDesc;
	xui_text_layout pLayout;
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
	pLayout = NULL;
	iRet = xuiTextLayoutCreate(xuiWidgetGetContext(pWidget), &pLayout, &tDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiInternalTextLayoutDrawContext(pLayout, pDraw, tContent,
		iTextColor, 0);
	xuiTextLayoutDestroy(pLayout);
	return iRet;
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
	return pType;
}

XUI_API int xuiLabelCreate(xui_context pContext, xui_widget* ppWidget, const xui_label_desc_t* pDesc)
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
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiLabelSetText(xui_widget pWidget, const char* sText)
{
	xui_label_data_t* pData;
	int iRet;

	pData = __xuiLabelGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
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
	if ( pData->pFont == pFont ) {
		return XUI_OK;
	}
	pData->pFont = pFont;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
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
