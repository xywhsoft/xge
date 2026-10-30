#include "xui_config.h"
#if XUI_ENABLE_HYPERLINK
#include "xui_internal.h"
#include "xui_pointer_press_internal.h"
#include "xui_text_internal.h"

#include <limits.h>
#include <math.h>
#include <string.h>

#define XUI_HYPERLINK_VISUAL_NORMAL	0
#define XUI_HYPERLINK_VISUAL_HOVER	1
#define XUI_HYPERLINK_VISUAL_ACTIVE	2
#define XUI_HYPERLINK_VISUAL_FOCUS	3
#define XUI_HYPERLINK_VISUAL_COUNT	6

typedef struct xui_hyperlink_data_t {
	char* sText;
	int iTextCapacity;
	xui_font pFont;
	xui_hyperlink_click_proc onClick;
	void* pClickUser;
	uint32_t iTextColor;
	uint32_t iHoverTextColor;
	uint32_t iActiveTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iFocusColor;
	uint32_t iTextFlags;
	int iWrapMode;
	int bUnderline;
	int bHoverUnderline;
	int bActiveUnderline;
	int bKeyboardActive;
	xui_pointer_press_t tPointerPress;
	int iClickCount;
	float fLineGap;
	float fParagraphGap;
	float fFocusWidth;
} xui_hyperlink_data_t;

static int __xuiHyperlinkNonNegativeFloatValid(float fValue)
{
	return isfinite(fValue) && fValue >= 0.0f && fValue <= XUI_LAYOUT_UNBOUNDED;
}

static int __xuiHyperlinkWrapModeValid(int iWrapMode)
{
	return (iWrapMode == XUI_TEXT_WRAP_NONE) ||
	       (iWrapMode == XUI_TEXT_WRAP_WORD) ||
	       (iWrapMode == XUI_TEXT_WRAP_CHAR);
}

static int __xuiHyperlinkDescValid(const xui_hyperlink_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) {
		return 0;
	}
	if ( !__xuiHyperlinkWrapModeValid(pDesc->iWrapMode) ) {
		return 0;
	}
	if ( !__xuiHyperlinkNonNegativeFloatValid(pDesc->fLineGap) ||
	     !__xuiHyperlinkNonNegativeFloatValid(pDesc->fParagraphGap) ) {
		return 0;
	}
	return 1;
}

static int __xuiHyperlinkTextSet(xui_hyperlink_data_t* pData, const char* sText)
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
	if ( iLength >= (size_t)INT_MAX ) return XUI_ERROR_INVALID_ARGUMENT;
	iNeed = (int)iLength + 1;
	iRet = xuiInternalTextBufferReserve(&pData->sText,
		&pData->iTextCapacity, iNeed);
	if ( iRet != XUI_OK ) return iRet;
	memmove(pData->sText, sText, (size_t)iNeed);
	return XUI_OK;
}

static xui_hyperlink_data_t* __xuiHyperlinkGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return NULL;
	}
	pContext = xuiWidgetGetContext(pWidget);
	pType = xuiWidgetFindType(pContext, "hyperlink");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_hyperlink_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiHyperlinkQueryCursor(xui_widget pWidget, int iX, int iY, void* pUser)
{
	(void)iX;
	(void)iY;
	(void)pUser;
	return xuiWidgetGetEffectiveEnabled(pWidget) ? XUI_CURSOR_HAND : XUI_CURSOR_NOT_ALLOWED;
}

static int __xuiHyperlinkStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
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

static int __xuiHyperlinkStyleInt(xui_widget pWidget, const char* sName, int* pValue)
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

static int __xuiHyperlinkStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) &&
	     __xuiHyperlinkNonNegativeFloatValid(tProperty.tValue.fFloat) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static xui_font __xuiHyperlinkStyleFont(xui_widget pWidget, xui_font pBaseFont)
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

static int __xuiHyperlinkVisualIndexFromStateId(uint32_t iStateId)
{
	if ( (iStateId & XUI_WIDGET_STATE_ACTIVE) != 0 ) {
		return XUI_HYPERLINK_VISUAL_ACTIVE;
	}
	if ( (iStateId & XUI_WIDGET_STATE_HOVER) != 0 ) {
		return XUI_HYPERLINK_VISUAL_HOVER;
	}
	if ( (iStateId & XUI_WIDGET_STATE_FOCUS) != 0 ) {
		return XUI_HYPERLINK_VISUAL_FOCUS;
	}
	return XUI_HYPERLINK_VISUAL_NORMAL;
}

static uint32_t __xuiHyperlinkComputeState(xui_widget pWidget, xui_hyperlink_data_t* pData)
{
	uint32_t iState;

	iState = xuiWidgetGetInputState(pWidget);
	if ( (pData != NULL) && pData->bKeyboardActive ) {
		iState |= XUI_WIDGET_STATE_ACTIVE;
	}
	return iState;
}

static uint32_t __xuiHyperlinkVisualStateId(uint32_t iState)
{
	uint32_t iVisual = 0;

	if ( (iState & XUI_WIDGET_STATE_DISABLED) != 0 ) {
		return 0;
	}
	if ( (iState & XUI_WIDGET_STATE_ACTIVE) != 0 ) {
		iVisual = XUI_WIDGET_STATE_ACTIVE;
	} else if ( (iState & XUI_WIDGET_STATE_HOVER) != 0 ) {
		iVisual = XUI_WIDGET_STATE_HOVER;
	}
	if ( (iState & XUI_WIDGET_STATE_FOCUS) != 0 ) {
		iVisual |= XUI_WIDGET_STATE_FOCUS;
	}
	return iVisual;
}

static int __xuiHyperlinkSyncState(xui_widget pWidget, xui_hyperlink_data_t* pData)
{
	uint32_t iState;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iState = __xuiHyperlinkComputeState(pWidget, pData);
	return xuiWidgetSetStateId(pWidget, __xuiHyperlinkVisualStateId(iState));
}

static void __xuiHyperlinkResolve(xui_widget pWidget, xui_hyperlink_data_t* pData, uint32_t iStateId, xui_text_layout_desc_t* pDesc, uint32_t* pTextColor)
{
	uint32_t iTextColor;
	uint32_t iHoverTextColor;
	uint32_t iActiveTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iTextFlags;
	int iWrapMode;
	int iStyleTextFlags;
	int bUnderline;
	int bHoverUnderline;
	int bActiveUnderline;
	int iVisual;
	float fLineGap;
	float fParagraphGap;
	xui_font pFont;

	iTextColor = pData->iTextColor;
	iHoverTextColor = pData->iHoverTextColor;
	iActiveTextColor = pData->iActiveTextColor;
	iDisabledTextColor = pData->iDisabledTextColor;
	iTextFlags = pData->iTextFlags | XUI_TEXT_CLIP;
	iWrapMode = pData->iWrapMode;
	iStyleTextFlags = (int)iTextFlags;
	bUnderline = pData->bUnderline;
	bHoverUnderline = pData->bHoverUnderline;
	bActiveUnderline = pData->bActiveUnderline;
	fLineGap = pData->fLineGap;
	fParagraphGap = pData->fParagraphGap;
	pFont = (pData->pFont != NULL) ? pData->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));

	(void)__xuiHyperlinkStyleColor(pWidget, "text.color", &iTextColor);
	(void)__xuiHyperlinkStyleColor(pWidget, "text.disabled_color", &iDisabledTextColor);
	if ( __xuiHyperlinkStyleInt(pWidget, "text.flags", &iStyleTextFlags) ) {
		iTextFlags = (uint32_t)iStyleTextFlags;
	}
	(void)__xuiHyperlinkStyleInt(pWidget, "text.wrap", &iWrapMode);
	(void)__xuiHyperlinkStyleInt(pWidget, "text.underline", &bUnderline);
	(void)__xuiHyperlinkStyleFloat(pWidget, "text.line_gap", &fLineGap);
	(void)__xuiHyperlinkStyleFloat(pWidget, "text.paragraph_gap", &fParagraphGap);

	(void)__xuiHyperlinkStyleColor(pWidget, "hyperlink.text.color", &iTextColor);
	(void)__xuiHyperlinkStyleColor(pWidget, "hyperlink.text.hover_color", &iHoverTextColor);
	(void)__xuiHyperlinkStyleColor(pWidget, "hyperlink.text.active_color", &iActiveTextColor);
	(void)__xuiHyperlinkStyleColor(pWidget, "hyperlink.text.disabled_color", &iDisabledTextColor);
	(void)__xuiHyperlinkStyleInt(pWidget, "hyperlink.text.hover_underline", &bHoverUnderline);
	(void)__xuiHyperlinkStyleInt(pWidget, "hyperlink.text.active_underline", &bActiveUnderline);
	pFont = __xuiHyperlinkStyleFont(pWidget, pFont);

	if ( !__xuiHyperlinkWrapModeValid(iWrapMode) ) {
		iWrapMode = XUI_TEXT_WRAP_NONE;
	}
	if ( !__xuiHyperlinkNonNegativeFloatValid(fLineGap) ) {
		fLineGap = 0.0f;
	}
	if ( !__xuiHyperlinkNonNegativeFloatValid(fParagraphGap) ) {
		fParagraphGap = 0.0f;
	}
	iVisual = __xuiHyperlinkVisualIndexFromStateId(iStateId);
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) {
		*pTextColor = iDisabledTextColor;
	} else if ( iVisual == XUI_HYPERLINK_VISUAL_ACTIVE ) {
		*pTextColor = iActiveTextColor;
		if ( bActiveUnderline ) {
			iTextFlags |= XUI_TEXT_UNDERLINE;
		}
	} else if ( iVisual == XUI_HYPERLINK_VISUAL_HOVER ) {
		*pTextColor = iHoverTextColor;
		if ( bHoverUnderline ) {
			iTextFlags |= XUI_TEXT_UNDERLINE;
		}
	} else {
		*pTextColor = iTextColor;
		if ( bUnderline ) {
			iTextFlags |= XUI_TEXT_UNDERLINE;
		}
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
}

static int __xuiHyperlinkContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_hyperlink_data_t* pData;
	xui_text_layout_desc_t tDesc;
	uint32_t iTextColor;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSize->fX = 0.0f;
	pSize->fY = 0.0f;
	pData = __xuiHyperlinkGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiHyperlinkResolve(pWidget, pData, 0, &tDesc, &iTextColor);
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

static int __xuiHyperlinkCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_hyperlink_data_t* pData;
	xui_text_layout_desc_t tDesc;
	xui_text_layout pLayout;
	xui_rect_t tContent;
	xui_rect_t tFocusRect;
	xui_proxy pProxy;
	uint32_t iTextColor;
	float fFocusWidth;
	uint32_t iFocusColor;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiHyperlinkGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiHyperlinkResolve(pWidget, pData, iStateId, &tDesc, &iTextColor);
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
	if ( iRet == XUI_OK && (iStateId & XUI_WIDGET_STATE_FOCUS) != 0 &&
	     pProxy->drawRectStroke != NULL ) {
		iFocusColor = pData->iFocusColor;
		fFocusWidth = pData->fFocusWidth;
		(void)__xuiHyperlinkStyleColor(pWidget, "hyperlink.focus.color", &iFocusColor);
		(void)__xuiHyperlinkStyleFloat(pWidget, "hyperlink.focus.width", &fFocusWidth);
		if ( fFocusWidth > 0.0f && (iFocusColor & 0xffu) != 0u ) {
			if ( tContent.fW > fFocusWidth && tContent.fH > fFocusWidth ) {
				tFocusRect = xuiInternalStrokeCenterRectInside(tContent,
					fFocusWidth, NULL);
				iRet = pProxy->drawRectStroke(pProxy, pDraw, tFocusRect,
					fFocusWidth, iFocusColor);
			}
		}
	}
	xuiTextLayoutDestroy(pLayout);
	return iRet;
}

static int __xuiHyperlinkDoClick(xui_widget pWidget, xui_hyperlink_data_t* pData)
{
	xui_hyperlink_click_proc onClick;
	void* pClickUser;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) {
		return XUI_OK;
	}
	if ( pData->iClickCount < INT_MAX ) pData->iClickCount++;
	iRet = __xuiHyperlinkSyncState(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	onClick = pData->onClick;
	pClickUser = pData->pClickUser;
	if ( onClick != NULL ) onClick(pWidget, pClickUser);
	return XUI_OK;
}

static void __xuiHyperlinkDefaultAction(xui_widget pWidget, void* pUser)
{
	xui_hyperlink_data_t* pData;

	(void)pUser;
	pData = __xuiHyperlinkGetData(pWidget);
	if ( pData == NULL ) {
		return;
	}
	(void)__xuiHyperlinkDoClick(pWidget, pData);
}

static int __xuiHyperlinkEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_hyperlink_data_t* pData;
	xui_context pContext;
	int bLeftButton;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiHyperlinkGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) {
		pData->bKeyboardActive = 0;
		iRet = xuiInternalPointerPressCancel(pContext, pWidget, &pData->tPointerPress);
		if ( iRet != XUI_OK ) return iRet;
		return __xuiHyperlinkSyncState(pWidget, pData);
	}
	bLeftButton = (pEvent->iButton == 0) || (pEvent->iButton == XUI_POINTER_BUTTON_LEFT);
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_LEAVE:
	case XUI_EVENT_POINTER_MOVE:
	case XUI_EVENT_FOCUS:
		return __xuiHyperlinkSyncState(pWidget, pData);
	case XUI_EVENT_BLUR:
		pData->bKeyboardActive = 0;
		return __xuiHyperlinkSyncState(pWidget, pData);
	case XUI_EVENT_ENABLED_CHANGED:
	case XUI_EVENT_VISIBLE_CHANGED:
		if ( pEvent->pTarget != pWidget ) return XUI_OK;
		pData->bKeyboardActive = 0;
		iRet = xuiInternalPointerPressCancel(pContext, pWidget,
			&pData->tPointerPress);
		if ( iRet != XUI_OK ) return iRet;
		return __xuiHyperlinkSyncState(pWidget, pData);
	case XUI_EVENT_POINTER_DOWN:
		if ( bLeftButton ) {
			if ( pData->tPointerPress.tOwner.bActive ) {
				return xuiInternalPointerPressMatches(&pData->tPointerPress, pEvent) ?
					XUI_EVENT_DISPATCH_STOP : XUI_OK;
			}
			iRet = xuiInternalPointerPressBegin(pContext, pWidget, pEvent,
				&pData->tPointerPress);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiHyperlinkSyncState(pWidget, pData);
			if ( iRet != XUI_OK ) {
				(void)xuiInternalPointerPressCancel(pContext, pWidget, &pData->tPointerPress);
				return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_UP:
		if ( bLeftButton && xuiInternalPointerPressMatches(&pData->tPointerPress, pEvent) ) {
			iRet = xuiInternalPointerPressEnd(pContext, pWidget, pEvent,
				&pData->tPointerPress);
			if ( iRet != XUI_OK ) return iRet;
			iRet = __xuiHyperlinkSyncState(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( !xuiInternalPointerPressCaptureLost(&pData->tPointerPress, pEvent) ) return XUI_OK;
		return __xuiHyperlinkSyncState(pWidget, pData);
	case XUI_EVENT_POINTER_CLICK:
		if ( bLeftButton ) {
			if ( !xuiInternalPointerPressAcceptClick(&pData->tPointerPress, pEvent) ) {
				return XUI_EVENT_DISPATCH_STOP;
			}
			iRet = __xuiHyperlinkDoClick(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_KEY_DOWN:
		if ( pEvent->iKey == XUI_KEY_SPACE ) {
			pData->bKeyboardActive = 1;
			iRet = __xuiHyperlinkSyncState(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_KEY_UP:
		if ( pEvent->iKey == XUI_KEY_SPACE ) {
			if ( pData->bKeyboardActive ) {
				pData->bKeyboardActive = 0;
				iRet = __xuiHyperlinkDoClick(pWidget, pData);
				if ( iRet != XUI_OK ) return iRet;
				return XUI_EVENT_DISPATCH_STOP;
			}
			iRet = __xuiHyperlinkSyncState(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	default:
		break;
	}
	return XUI_OK;
}

static void __xuiHyperlinkDefaultLayout(xui_layout_t* pLayout)
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

static void __xuiHyperlinkDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE | XUI_CACHE_UPDATE_ALL_STATES;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiHyperlinkInitCacheStates(xui_widget pWidget)
{
	static const uint32_t arrState[XUI_HYPERLINK_VISUAL_COUNT] = {
		0,
		XUI_WIDGET_STATE_HOVER,
		XUI_WIDGET_STATE_ACTIVE,
		XUI_WIDGET_STATE_FOCUS,
		XUI_WIDGET_STATE_HOVER | XUI_WIDGET_STATE_FOCUS,
		XUI_WIDGET_STATE_ACTIVE | XUI_WIDGET_STATE_FOCUS
	};
	int i;
	int iRet;

	iRet = xuiWidgetSetCacheStateCount(pWidget, XUI_HYPERLINK_VISUAL_COUNT);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	for ( i = XUI_HYPERLINK_VISUAL_COUNT - 1; i >= 0; i-- ) {
		iRet = xuiWidgetSetCacheStateId(pWidget, i, arrState[i]);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiHyperlinkInitEvents(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetDefaultAction(pWidget, __xuiHyperlinkDefaultAction, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_ENTER, __xuiHyperlinkEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiHyperlinkEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiHyperlinkEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiHyperlinkEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_UP, __xuiHyperlinkEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CLICK, __xuiHyperlinkEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiHyperlinkEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiHyperlinkEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiHyperlinkEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiHyperlinkEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiHyperlinkEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiHyperlinkEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_UP, __xuiHyperlinkEvent, NULL);
	return iRet;
}

static int __xuiHyperlinkInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_hyperlink_data_t* pData;
	const xui_hyperlink_desc_t* pDesc;
	xui_theme_t tTheme;
	xui_context pContext;
	int iRet;

	(void)pUser;
	pData = (xui_hyperlink_data_t*)pTypeData;
	pDesc = (const xui_hyperlink_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiHyperlinkDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	memset(&tTheme, 0, sizeof(tTheme));
	tTheme.iSize = sizeof(tTheme);
	(void)xuiGetTheme(pContext, &tTheme);
	pData->pFont = (pDesc != NULL && pDesc->pFont != NULL) ? pDesc->pFont : xuiGetDefaultFont(pContext);
	pData->iTextColor = (pDesc != NULL && pDesc->iTextColor != 0) ? pDesc->iTextColor : tTheme.iAccentColor;
	pData->iHoverTextColor = (pDesc != NULL && pDesc->iHoverTextColor != 0) ? pDesc->iHoverTextColor : XUI_COLOR_RGBA(28, 110, 214, 255);
	pData->iActiveTextColor = (pDesc != NULL && pDesc->iActiveTextColor != 0) ? pDesc->iActiveTextColor : XUI_COLOR_RGBA(16, 78, 172, 255);
	pData->iDisabledTextColor = (pDesc != NULL && pDesc->iDisabledTextColor != 0) ? pDesc->iDisabledTextColor : tTheme.iStateDisabledColor;
	pData->iFocusColor = tTheme.iStateFocusColor;
	pData->iTextFlags = ((pDesc != NULL && pDesc->iTextFlags != 0) ? pDesc->iTextFlags : (XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP)) | XUI_TEXT_CLIP;
	pData->iWrapMode = (pDesc != NULL) ? pDesc->iWrapMode : XUI_TEXT_WRAP_NONE;
	pData->bUnderline = (pDesc != NULL) ? (pDesc->bUnderline ? 1 : 0) : 0;
	pData->bHoverUnderline = (pDesc != NULL) ? (pDesc->bHoverUnderline ? 1 : 0) : 1;
	pData->bActiveUnderline = (pDesc != NULL) ? (pDesc->bActiveUnderline ? 1 : 0) : 1;
	pData->fLineGap = (pDesc != NULL) ? pDesc->fLineGap : 0.0f;
	pData->fParagraphGap = (pDesc != NULL) ? pDesc->fParagraphGap : 0.0f;
	pData->fFocusWidth = 1.0f;
	iRet = __xuiHyperlinkTextSet(pData, (pDesc != NULL) ? pDesc->sText : "");
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiWidgetSetFocusable(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetHitTestVisible(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = __xuiHyperlinkInitCacheStates(pWidget);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = __xuiHyperlinkInitEvents(pWidget);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	return __xuiHyperlinkSyncState(pWidget, pData);
}

static void __xuiHyperlinkDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_hyperlink_data_t* pData;

	(void)pUser;
	pData = (xui_hyperlink_data_t*)pTypeData;
	if ( pData == NULL ) {
		return;
	}
	(void)xuiInternalPointerPressCancel(xuiWidgetGetContext(pWidget), pWidget,
		&pData->tPointerPress);
	if ( pData->sText != NULL ) {
		xrtFree(pData->sText);
	}
	memset(pData, 0, sizeof(*pData));
}

static void __xuiHyperlinkRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
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

static void __xuiHyperlinkRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "text.color", XUI_STYLE_VALUE_COLOR, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "text.disabled_color", XUI_STYLE_VALUE_COLOR, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "text.flags", XUI_STYLE_VALUE_INT, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "text.wrap", XUI_STYLE_VALUE_INT, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "text.underline", XUI_STYLE_VALUE_BOOL, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "text.line_gap", XUI_STYLE_VALUE_FLOAT, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "text.paragraph_gap", XUI_STYLE_VALUE_FLOAT, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, XUI_STYLE_PROPERTY_INHERITED);
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "hyperlink.text.color", XUI_STYLE_VALUE_COLOR, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, 0);
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "hyperlink.text.hover_color", XUI_STYLE_VALUE_COLOR, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, 0);
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "hyperlink.text.active_color", XUI_STYLE_VALUE_COLOR, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, 0);
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "hyperlink.text.disabled_color", XUI_STYLE_VALUE_COLOR, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, 0);
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "hyperlink.text.hover_underline", XUI_STYLE_VALUE_BOOL, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, 0);
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "hyperlink.text.active_underline", XUI_STYLE_VALUE_BOOL, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, 0);
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "hyperlink.focus.color", XUI_STYLE_VALUE_COLOR, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, 0);
	__xuiHyperlinkRegisterStyleProperty(pContext, pType, "hyperlink.focus.width", XUI_STYLE_VALUE_FLOAT, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER, 0);
}

static int __xuiHyperlinkAccessibleNode(xui_widget pWidget, xui_accessible_node_t* pNode)
{
	xui_hyperlink_data_t* pData = __xuiHyperlinkGetData(pWidget);

	if ( pData == NULL || pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_LINK;
	pNode->sName = xuiWidgetGetAccessibleName(pWidget) != NULL ?
		xuiWidgetGetAccessibleName(pWidget) : pData->sText;
	pNode->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE);
	return XUI_OK;
}

static const xui_internal_accessibility_adapter_t g_xuiHyperlinkAccessible = {
	.getNode = __xuiHyperlinkAccessibleNode,
	.performAction = NULL
};

XUI_API xui_widget_type xuiHyperlinkGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "hyperlink");
	if ( pType != NULL ) {
		__xuiHyperlinkRegisterStyleProperties(pContext, pType);
		pType->pAccessibleAdapter = &g_xuiHyperlinkAccessible;
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "hyperlink";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_hyperlink_data_t);
	tDesc.onInit = __xuiHyperlinkInit;
	tDesc.onDestroy = __xuiHyperlinkDestroy;
	tDesc.onContentMeasure = __xuiHyperlinkContentMeasure;
	tDesc.onCacheRender = __xuiHyperlinkCacheRender;
	tDesc.onQueryCursor = __xuiHyperlinkQueryCursor;
	__xuiHyperlinkDefaultLayout(&tDesc.tLayout);
	__xuiHyperlinkDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	__xuiHyperlinkRegisterStyleProperties(pContext, pType);
	pType->pAccessibleAdapter = &g_xuiHyperlinkAccessible;
	return pType;
}

XUI_API int xuiHyperlinkCreate(xui_context pContext, xui_widget* ppWidget, const xui_hyperlink_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppWidget = NULL;
	if ( !__xuiHyperlinkDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = xuiHyperlinkGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiHyperlinkSetClick(xui_widget pWidget, xui_hyperlink_click_proc onClick, void* pUser)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData->onClick = onClick;
	pData->pClickUser = pUser;
	return XUI_OK;
}

XUI_API int xuiHyperlinkSetText(xui_widget pWidget, const char* sText)
{
	xui_hyperlink_data_t* pData;
	int iRet;

	pData = __xuiHyperlinkGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( sText == NULL ) {
		sText = "";
	}
	if ( (pData->sText != NULL) && (strcmp(pData->sText, sText) == 0) ) {
		return XUI_OK;
	}
	iRet = __xuiHyperlinkTextSet(pData, sText);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API const char* xuiHyperlinkGetText(xui_widget pWidget)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL && pData->sText != NULL) ? pData->sText : NULL;
}

XUI_API int xuiHyperlinkSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->pFont == pFont ) {
		return XUI_OK;
	}
	pData->pFont = pFont;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API xui_font xuiHyperlinkGetFont(xui_widget pWidget)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

static int __xuiHyperlinkSetOneColor(xui_widget pWidget, uint32_t* pSlot, uint32_t iColor)
{
	if ( pSlot == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( *pSlot == iColor ) {
		return XUI_OK;
	}
	*pSlot = iColor;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiHyperlinkSetTextColor(xui_widget pWidget, uint32_t iColor)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? __xuiHyperlinkSetOneColor(pWidget, &pData->iTextColor, iColor) : XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API uint32_t xuiHyperlinkGetTextColor(xui_widget pWidget)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? pData->iTextColor : 0;
}

XUI_API int xuiHyperlinkSetHoverTextColor(xui_widget pWidget, uint32_t iColor)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? __xuiHyperlinkSetOneColor(pWidget, &pData->iHoverTextColor, iColor) : XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API uint32_t xuiHyperlinkGetHoverTextColor(xui_widget pWidget)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? pData->iHoverTextColor : 0;
}

XUI_API int xuiHyperlinkSetActiveTextColor(xui_widget pWidget, uint32_t iColor)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? __xuiHyperlinkSetOneColor(pWidget, &pData->iActiveTextColor, iColor) : XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API uint32_t xuiHyperlinkGetActiveTextColor(xui_widget pWidget)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? pData->iActiveTextColor : 0;
}

XUI_API int xuiHyperlinkSetDisabledTextColor(xui_widget pWidget, uint32_t iColor)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? __xuiHyperlinkSetOneColor(pWidget, &pData->iDisabledTextColor, iColor) : XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API uint32_t xuiHyperlinkGetDisabledTextColor(xui_widget pWidget)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? pData->iDisabledTextColor : 0;
}

XUI_API int xuiHyperlinkSetTextColors(xui_widget pWidget, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iDisabled)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (pData->iTextColor == iNormal) &&
	     (pData->iHoverTextColor == iHover) &&
	     (pData->iActiveTextColor == iActive) &&
	     (pData->iDisabledTextColor == iDisabled) ) {
		return XUI_OK;
	}
	pData->iTextColor = iNormal;
	pData->iHoverTextColor = iHover;
	pData->iActiveTextColor = iActive;
	pData->iDisabledTextColor = iDisabled;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiHyperlinkSetTextFlags(xui_widget pWidget, uint32_t iTextFlags)
{
	xui_hyperlink_data_t* pData;
	uint32_t iDirtyFlags;

	pData = __xuiHyperlinkGetData(pWidget);
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

XUI_API uint32_t xuiHyperlinkGetTextFlags(xui_widget pWidget)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? pData->iTextFlags : 0;
}

XUI_API int xuiHyperlinkSetWrapMode(xui_widget pWidget, int iWrapMode)
{
	xui_hyperlink_data_t* pData;

	if ( !__xuiHyperlinkWrapModeValid(iWrapMode) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiHyperlinkGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->iWrapMode == iWrapMode ) {
		return XUI_OK;
	}
	pData->iWrapMode = iWrapMode;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiHyperlinkGetWrapMode(xui_widget pWidget)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? pData->iWrapMode : XUI_TEXT_WRAP_NONE;
}

XUI_API int xuiHyperlinkSetUnderline(xui_widget pWidget, int bNormal, int bHover, int bActive)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	bNormal = bNormal ? 1 : 0;
	bHover = bHover ? 1 : 0;
	bActive = bActive ? 1 : 0;
	if ( (pData->bUnderline == bNormal) &&
	     (pData->bHoverUnderline == bHover) &&
	     (pData->bActiveUnderline == bActive) ) {
		return XUI_OK;
	}
	pData->bUnderline = bNormal;
	pData->bHoverUnderline = bHover;
	pData->bActiveUnderline = bActive;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiHyperlinkGetUnderline(xui_widget pWidget)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? pData->bUnderline : 0;
}

XUI_API int xuiHyperlinkGetHoverUnderline(xui_widget pWidget)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? pData->bHoverUnderline : 0;
}

XUI_API int xuiHyperlinkGetActiveUnderline(xui_widget pWidget)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? pData->bActiveUnderline : 0;
}

XUI_API int xuiHyperlinkSetLineGap(xui_widget pWidget, float fLineGap)
{
	xui_hyperlink_data_t* pData;

	if ( !__xuiHyperlinkNonNegativeFloatValid(fLineGap) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiHyperlinkGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->fLineGap == fLineGap ) {
		return XUI_OK;
	}
	pData->fLineGap = fLineGap;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API float xuiHyperlinkGetLineGap(xui_widget pWidget)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? pData->fLineGap : 0.0f;
}

XUI_API int xuiHyperlinkSetParagraphGap(xui_widget pWidget, float fParagraphGap)
{
	xui_hyperlink_data_t* pData;

	if ( !__xuiHyperlinkNonNegativeFloatValid(fParagraphGap) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiHyperlinkGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->fParagraphGap == fParagraphGap ) {
		return XUI_OK;
	}
	pData->fParagraphGap = fParagraphGap;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API float xuiHyperlinkGetParagraphGap(xui_widget pWidget)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? pData->fParagraphGap : 0.0f;
}

XUI_API uint32_t xuiHyperlinkGetState(xui_widget pWidget)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	if ( pData == NULL ) {
		return 0;
	}
	(void)__xuiHyperlinkSyncState(pWidget, pData);
	return __xuiHyperlinkComputeState(pWidget, pData);
}

XUI_API int xuiHyperlinkGetClickCount(xui_widget pWidget)
{
	xui_hyperlink_data_t* pData;

	pData = __xuiHyperlinkGetData(pWidget);
	return (pData != NULL) ? pData->iClickCount : 0;
}

#endif
