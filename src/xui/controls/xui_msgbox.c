#include "xui_config.h"
#if XUI_ENABLE_MSGBOX
#include "xui_internal.h"
#include "xui_text_internal.h"

#include <limits.h>
#include <string.h>

#define XUI_MSGBOX_MAGIC 0x584D5342u
#define XUI_MSGBOX_PRESET_BUTTON_TITLE_COUNT 4

typedef struct xui_msgbox_button_paint_t {
	uint32_t iBaseFill, iBaseBorder;
	uint32_t iFill, iBorder;
	int iMask;
} xui_msgbox_button_paint_t;

struct xui_msgbox_t {
	uint32_t iMagic;
	xui_context pContext;
	xui_widget pWindow;
	xui_widget pContent;
	xui_widget pBackdrop;
	xui_widget arrButtons[XUI_MSGBOX_BUTTON_CAPACITY];
	char* sTitle;
	char* sMessage;
	char* arrButtonText[XUI_MSGBOX_BUTTON_CAPACITY];
	char* arrPresetButtonTitle[XUI_MSGBOX_PRESET_BUTTON_TITLE_COUNT];
	int arrButtonResult[XUI_MSGBOX_BUTTON_CAPACITY];
	int arrButtonSemantic[XUI_MSGBOX_BUTTON_CAPACITY];
	xui_msgbox_metrics_t tMetrics;
	xui_msgbox_colors_t tColors;
	xui_msgbox_button_paint_t arrButtonPaint[XUI_MSGBOX_BUTTON_CAPACITY][6];
	uint32_t arrBaseMuted[XUI_MSGBOX_BUTTON_CAPACITY];
	uint32_t arrAppliedMuted[XUI_MSGBOX_BUTTON_CAPACITY];
	int arrStyledMuted[XUI_MSGBOX_BUTTON_CAPACITY];
	uint32_t iPaintStyleHash;
	int bPaintReady;
	int bHasColors;
	xui_msgbox_result_proc onResult;
	void* pResultUser;
	xui_widget_event_proc onWindowEvent;
	void* pWindowEventUser;
	xui_font pFont;
	xui_surface pIconSurface;
	xui_rect_t tIconSrc;
	xui_rect_t tIconRect;
	xui_rect_t tMessageRect;
	xui_rect_t tBackdropRect;
	xui_rect_t arrButtonRect[XUI_MSGBOX_BUTTON_CAPACITY];
	int iType;
	int iButtons;
	int iButtonCount;
	int iResult;
	int iWrapLineCount;
	int iResultCount;
	int iChangeCount;
	int iModalZ;
	int bModal;
	int bOpen;
	int bCustomIcon;
	int bBuiltinIcon;
};

static int __xuiMsgBoxValid(xui_msgbox pBox)
{
	return (pBox != NULL) && (pBox->iMagic == XUI_MSGBOX_MAGIC);
}

static int __xuiMsgBoxStyleColor(xui_msgbox pBox, const char* sName, uint32_t* pColor)
{
	xui_style_property_t tProperty;
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	if ( xuiWidgetGetResolvedStyleProperty(pBox->pWindow, sName, &tProperty) == XUI_OK &&
		tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR ) {
		*pColor = tProperty.tValue.iColor;
		return 1;
	}
	return 0;
}

static uint32_t __xuiMsgBoxColor(xui_msgbox pBox, const char* sName, uint32_t iBase)
{
	(void)__xuiMsgBoxStyleColor(pBox, sName, &iBase);
	return iBase;
}

static int __xuiMsgBoxPreparePaint(xui_widget pWidget);
static int __xuiMsgBoxBackdropPreparePaint(xui_widget pWidget);
static int __xuiMsgBoxApplyButtonVisual(xui_msgbox pBox);

static xui_widget_type __xuiMsgBoxEnsureType(xui_context pContext)
{
	static const char* arrNames[] = {
		"msgbox.backdrop.color", "msgbox.client.color", "msgbox.text.color", "msgbox.text.muted_color",
		"msgbox.icon.color", "msgbox.icon.text.color", "msgbox.button.color", "msgbox.button.hover_color",
		"msgbox.button.active_color", "msgbox.button.focus_color", "msgbox.button.disabled_color",
		"msgbox.button.border_color"
	};
	xui_widget_type pType = xuiWidgetFindType(pContext, "msgbox");
	xui_widget_type_desc_t tDesc;
	xui_style_property_info_t tInfo;
	size_t i;
	if ( pType != NULL ) return pType;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "msgbox";
	tDesc.pParent = xuiWindowGetType(pContext);
	if ( tDesc.pParent == NULL ) return NULL;
	tDesc.iTypeDataSize = tDesc.pParent->iTypeDataSize;
	if ( xuiWidgetRegisterType(pContext, &pType, &tDesc) != XUI_OK ) return NULL;
	pType->onPreparePaint = __xuiMsgBoxPreparePaint;
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.pWidgetType = pType;
	tInfo.iValueType = XUI_STYLE_VALUE_COLOR;
	tInfo.iDirtyFlags = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	tInfo.iFlags = XUI_STYLE_PROPERTY_INHERITED;
	for ( i = 0; i < sizeof(arrNames) / sizeof(arrNames[0]); ++i ) {
		tInfo.sName = arrNames[i];
		(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
	}
	return pType;
}

static float __xuiMsgBoxMax(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static float __xuiMsgBoxMin(float fA, float fB)
{
	return (fA < fB) ? fA : fB;
}

static float __xuiMsgBoxClamp(float fValue, float fMin, float fMax)
{
	if ( fMax < fMin ) {
		return fMin;
	}
	if ( fValue < fMin ) return fMin;
	if ( fValue > fMax ) return fMax;
	return fValue;
}

static int __xuiMsgBoxFloatValid(float fValue)
{
	return (fValue == fValue) && (fValue >= 0.0f) && (fValue <= XUI_LAYOUT_UNBOUNDED);
}

static int __xuiMsgBoxRectValid(xui_rect_t tRect)
{
	return tRect.fX >= -XUI_LAYOUT_UNBOUNDED && tRect.fX <= XUI_LAYOUT_UNBOUNDED &&
	       tRect.fY >= -XUI_LAYOUT_UNBOUNDED && tRect.fY <= XUI_LAYOUT_UNBOUNDED &&
	       __xuiMsgBoxFloatValid(tRect.fW) && __xuiMsgBoxFloatValid(tRect.fH);
}

static int __xuiMsgBoxTypeValid(int iType)
{
	return (iType >= XUI_MSGBOX_ICON_NONE) && (iType <= XUI_MSGBOX_ICON_ERROR);
}

static int __xuiMsgBoxButtonsValid(int iButtons)
{
	return (iButtons >= XUI_MSGBOX_BUTTON_OK) && (iButtons <= XUI_MSGBOX_BUTTON_CUSTOM);
}

static int __xuiMsgBoxPresetButtonTitleIndex(int iTitle)
{
	switch ( iTitle ) {
	case XUI_MSGBOX_BUTTON_TITLE_OK:
		return 0;
	case XUI_MSGBOX_BUTTON_TITLE_CANCEL:
		return 1;
	case XUI_MSGBOX_BUTTON_TITLE_YES:
		return 2;
	case XUI_MSGBOX_BUTTON_TITLE_NO:
		return 3;
	default:
		return -1;
	}
}

static const char* __xuiMsgBoxDefaultPresetButtonTitle(int iTitle)
{
	switch ( iTitle ) {
	case XUI_MSGBOX_BUTTON_TITLE_CANCEL:
		return "Cancel";
	case XUI_MSGBOX_BUTTON_TITLE_YES:
		return "Yes";
	case XUI_MSGBOX_BUTTON_TITLE_NO:
		return "No";
	case XUI_MSGBOX_BUTTON_TITLE_OK:
	default:
		return "OK";
	}
}

static int __xuiMsgBoxSemanticValid(int iSemantic)
{
	return (iSemantic == XUI_BUTTON_SEMANTIC_DEFAULT) ||
	       (iSemantic == XUI_BUTTON_SEMANTIC_PRIMARY) ||
	       (iSemantic == XUI_BUTTON_SEMANTIC_DANGER);
}

static int __xuiMsgBoxButtonModelValid(const xui_msgbox_button_t* pButtons,
	int iCount)
{
	int i;

	if ( iCount < 0 || iCount > XUI_MSGBOX_BUTTON_CAPACITY ||
	     (iCount > 0 && pButtons == NULL) ) return 0;
	for ( i = 0; i < iCount; i++ ) {
		if ( !__xuiMsgBoxSemanticValid(pButtons[i].iSemantic) ) return 0;
	}
	return 1;
}

static char* __xuiMsgBoxDupText(const char* sText)
{
	char* sCopy;
	size_t iLen;

	if ( sText == NULL ) {
		sText = "";
	}
	iLen = strlen(sText);
	sCopy = (char*)xrtMalloc(iLen + 1u);
	if ( sCopy == NULL ) {
		return NULL;
	}
	memcpy(sCopy, sText, iLen + 1u);
	return sCopy;
}

static int __xuiMsgBoxReplaceText(char** ppText, const char* sText)
{
	char* sCopy;

	if ( ppText == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	sCopy = __xuiMsgBoxDupText(sText);
	if ( sCopy == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( *ppText != NULL ) {
		xrtFree(*ppText);
	}
	*ppText = sCopy;
	return XUI_OK;
}

static int __xuiMsgBoxTextSame(const char* sA, const char* sB)
{
	if ( sA == NULL ) sA = "";
	if ( sB == NULL ) sB = "";
	return strcmp(sA, sB) == 0;
}

static int __xuiMsgBoxButtonModelSame(xui_msgbox pBox, int iButtons,
	const xui_msgbox_button_t* pButtons, int iCount)
{
	int i;

	if ( (pBox == NULL) || (pBox->iButtons != iButtons) || (pBox->iButtonCount != iCount) ) {
		return 0;
	}
	for ( i = 0; i < iCount; i++ ) {
		int iSemantic = __xuiMsgBoxSemanticValid(pButtons[i].iSemantic) ?
			pButtons[i].iSemantic : XUI_BUTTON_SEMANTIC_DEFAULT;
		if ( !__xuiMsgBoxTextSame(pBox->arrButtonText[i], pButtons[i].sText) ||
		     pBox->arrButtonResult[i] != pButtons[i].iResult ||
		     pBox->arrButtonSemantic[i] != iSemantic ) {
			return 0;
		}
	}
	return 1;
}

static int __xuiMsgBoxApplyButtonModel(xui_msgbox pBox, int iButtons,
	const xui_msgbox_button_t* pButtons, int iCount)
{
	char* arrText[XUI_MSGBOX_BUTTON_CAPACITY] = {NULL};
	int arrSemantic[XUI_MSGBOX_BUTTON_CAPACITY];
	int i;
	int iRet;

	if ( !__xuiMsgBoxValid(pBox) || !__xuiMsgBoxButtonsValid(iButtons) ||
	     !__xuiMsgBoxButtonModelValid(pButtons, iCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( __xuiMsgBoxButtonModelSame(pBox, iButtons, pButtons, iCount) ) {
		return XUI_OK;
	}
	for ( i = 0; i < XUI_MSGBOX_BUTTON_CAPACITY; i++ ) {
		arrSemantic[i] = XUI_BUTTON_SEMANTIC_DEFAULT;
		if ( i < iCount ) {
			arrText[i] = __xuiMsgBoxDupText(pButtons[i].sText);
			if ( arrText[i] == NULL ) {
				iRet = XUI_ERROR_OUT_OF_MEMORY;
				goto failed;
			}
			arrSemantic[i] = pButtons[i].iSemantic;
		}
	}
	for ( i = 0; i < XUI_MSGBOX_BUTTON_CAPACITY; i++ ) {
		if ( pBox->arrButtons[i] == NULL ) continue;
		iRet = xuiButtonSetText(pBox->arrButtons[i], (i < iCount) ? arrText[i] : "");
		if ( iRet == XUI_OK ) iRet = xuiButtonSetSemantic(pBox->arrButtons[i], arrSemantic[i]);
		if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pBox->arrButtons[i], i < iCount);
		if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pBox->arrButtons[i], i < iCount);
		if ( iRet != XUI_OK ) goto restore;
	}
	for ( i = 0; i < XUI_MSGBOX_BUTTON_CAPACITY; i++ ) {
		if ( pBox->arrButtonText[i] != NULL ) xrtFree(pBox->arrButtonText[i]);
		pBox->arrButtonText[i] = arrText[i];
		arrText[i] = NULL;
		pBox->arrButtonResult[i] = (i < iCount) ? pButtons[i].iResult : XUI_MSGBOX_RESULT_CLOSE;
		pBox->arrButtonSemantic[i] = arrSemantic[i];
	}
	pBox->iButtons = iButtons;
	pBox->iButtonCount = iCount;
	return XUI_OK;

restore:
	for ( i = 0; i < XUI_MSGBOX_BUTTON_CAPACITY; i++ ) {
		if ( pBox->arrButtons[i] == NULL ) continue;
		(void)xuiButtonSetText(pBox->arrButtons[i],
			(i < pBox->iButtonCount && pBox->arrButtonText[i] != NULL) ? pBox->arrButtonText[i] : "");
		(void)xuiButtonSetSemantic(pBox->arrButtons[i],
			(i < pBox->iButtonCount) ? pBox->arrButtonSemantic[i] : XUI_BUTTON_SEMANTIC_DEFAULT);
		(void)xuiWidgetSetVisible(pBox->arrButtons[i], i < pBox->iButtonCount);
		(void)xuiWidgetSetEnabled(pBox->arrButtons[i], i < pBox->iButtonCount);
	}
failed:
	for ( i = 0; i < XUI_MSGBOX_BUTTON_CAPACITY; i++ ) {
		if ( arrText[i] != NULL ) xrtFree(arrText[i]);
	}
	return iRet;
}

static void __xuiMsgBoxDefaultMetrics(xui_msgbox_metrics_t* pMetrics)
{
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iSize = sizeof(*pMetrics);
	pMetrics->fPaddingX = 18.0f;
	pMetrics->fPaddingY = 16.0f;
	pMetrics->fIconSize = 38.0f;
	pMetrics->fIconGap = 14.0f;
	pMetrics->fButtonWidth = 78.0f;
	pMetrics->fButtonHeight = 28.0f;
	pMetrics->fButtonGap = 8.0f;
	pMetrics->fFooterHeight = 50.0f;
	pMetrics->fMinMessageHeight = 38.0f;
	pMetrics->fMinWidth = 320.0f;
	pMetrics->fMaxWidth = 680.0f;
	pMetrics->fMaxWidthRatio = 0.8f;
}

static void __xuiMsgBoxDefaultColors(xui_msgbox_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iSize = sizeof(*pColors);
	pColors->iBackdropColor = XUI_COLOR_RGBA(18, 42, 66, 90);
	pColors->iClientColor = XUI_COLOR_RGBA(250, 253, 255, 255);
	pColors->iMessageColor = XUI_COLOR_RGBA(34, 52, 78, 255);
	pColors->iMutedTextColor = XUI_COLOR_RGBA(93, 113, 138, 255);
	pColors->iIconColor = XUI_COLOR_RGBA(35, 132, 214, 255);
	pColors->iButtonColor = XUI_COLOR_RGBA(247, 251, 255, 255);
	pColors->iButtonHoverColor = XUI_COLOR_RGBA(232, 243, 253, 255);
	pColors->iButtonActiveColor = XUI_COLOR_RGBA(212, 231, 248, 255);
	pColors->iButtonFocusColor = XUI_COLOR_RGBA(47, 128, 214, 255);
	pColors->iButtonDisabledColor = XUI_COLOR_RGBA(226, 235, 243, 255);
}

static int __xuiMsgBoxMetricsValid(const xui_msgbox_metrics_t* pMetrics)
{
	if ( pMetrics == NULL ) return 0;
	if ( (pMetrics->iSize != 0) && (pMetrics->iSize < sizeof(*pMetrics)) ) return 0;
	return __xuiMsgBoxFloatValid(pMetrics->fPaddingX) &&
	       __xuiMsgBoxFloatValid(pMetrics->fPaddingY) &&
	       __xuiMsgBoxFloatValid(pMetrics->fIconSize) &&
	       __xuiMsgBoxFloatValid(pMetrics->fIconGap) &&
	       __xuiMsgBoxFloatValid(pMetrics->fButtonWidth) &&
	       __xuiMsgBoxFloatValid(pMetrics->fButtonHeight) &&
	       __xuiMsgBoxFloatValid(pMetrics->fButtonGap) &&
	       __xuiMsgBoxFloatValid(pMetrics->fFooterHeight) &&
	       __xuiMsgBoxFloatValid(pMetrics->fMinMessageHeight) &&
	       __xuiMsgBoxFloatValid(pMetrics->fMinWidth) &&
	       __xuiMsgBoxFloatValid(pMetrics->fMaxWidth) &&
	       (pMetrics->fMinWidth <= pMetrics->fMaxWidth) &&
	       (pMetrics->fButtonHeight <= pMetrics->fFooterHeight) &&
	       (pMetrics->fMaxWidthRatio >= 0.1f) &&
	       (pMetrics->fMaxWidthRatio <= 1.0f);
}

static int __xuiMsgBoxColorsValid(const xui_msgbox_colors_t* pColors)
{
	if ( pColors == NULL ) return 0;
	return (pColors->iSize == 0) || (pColors->iSize >= sizeof(*pColors));
}

static int __xuiMsgBoxDescValid(const xui_msgbox_desc_t* pDesc)
{
	if ( pDesc == NULL ) return 1;
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) return 0;
	if ( !__xuiMsgBoxTypeValid(pDesc->iType) ) return 0;
	if ( !__xuiMsgBoxButtonsValid(pDesc->iButtons) ) return 0;
	if ( (pDesc->iCustomButtonCount < 0) || (pDesc->iCustomButtonCount > XUI_MSGBOX_BUTTON_CAPACITY) ) return 0;
	if ( (pDesc->iCustomButtonCount > 0) && (pDesc->pCustomButtons == NULL) ) return 0;
	if ( !__xuiMsgBoxRectValid(pDesc->tIconSrc) ) return 0;
	if ( pDesc->bHasCustomIcon && pDesc->pIconSurface != NULL &&
	     (pDesc->tIconSrc.fW <= 0 || pDesc->tIconSrc.fH <= 0) ) return 0;
	if ( !__xuiMsgBoxButtonModelValid(pDesc->pCustomButtons,
	     pDesc->iCustomButtonCount) ) return 0;
	if ( pDesc->bHasMetrics && !__xuiMsgBoxMetricsValid(&pDesc->tMetrics) ) return 0;
	if ( pDesc->bHasColors && !__xuiMsgBoxColorsValid(&pDesc->tColors) ) return 0;
	return 1;
}

static xui_font __xuiMsgBoxFont(xui_msgbox pBox)
{
	if ( pBox == NULL ) return NULL;
	return (pBox->pFont != NULL) ? pBox->pFont : xuiGetDefaultFont(pBox->pContext);
}

static const char* __xuiMsgBoxIconName(int iType)
{
	switch ( iType ) {
	case XUI_MSGBOX_ICON_INFO:
		return "msgbox_info";
	case XUI_MSGBOX_ICON_QUEST:
		return "msgbox_quest";
	case XUI_MSGBOX_ICON_WAR:
		return "msgbox_war";
	case XUI_MSGBOX_ICON_ERROR:
		return "msgbox_error";
	default:
		return NULL;
	}
}

static const char* __xuiMsgBoxIconFallbackText(int iType)
{
	switch ( iType ) {
	case XUI_MSGBOX_ICON_QUEST:
		return "?";
	case XUI_MSGBOX_ICON_WAR:
		return "!";
	case XUI_MSGBOX_ICON_ERROR:
		return "x";
	case XUI_MSGBOX_ICON_INFO:
	default:
		return "i";
	}
}

static uint32_t __xuiMsgBoxIconColor(int iType)
{
	switch ( iType ) {
	case XUI_MSGBOX_ICON_WAR:
		return XUI_COLOR_RGBA(219, 141, 36, 255);
	case XUI_MSGBOX_ICON_ERROR:
		return XUI_COLOR_RGBA(210, 70, 84, 255);
	case XUI_MSGBOX_ICON_QUEST:
		return XUI_COLOR_RGBA(59, 126, 204, 255);
	case XUI_MSGBOX_ICON_INFO:
	default:
		return XUI_COLOR_RGBA(35, 132, 214, 255);
	}
}

static xui_rect_t __xuiMsgBoxRootRect(xui_msgbox pBox)
{
	xui_widget pRoot;
	xui_rect_t tRect;

	tRect = (xui_rect_t){0.0f, 0.0f, 640.0f, 420.0f};
	if ( pBox == NULL ) {
		return tRect;
	}
	pRoot = xuiGetRootWidget(pBox->pContext);
	if ( pRoot != NULL ) {
		tRect = xuiWidgetGetRect(pRoot);
		if ( (tRect.fW <= 1.0f) || (tRect.fH <= 1.0f) ) {
			tRect = (xui_rect_t){0.0f, 0.0f, 640.0f, 420.0f};
		}
	}
	return tRect;
}

static int __xuiMsgBoxRectSame(xui_rect_t tA, xui_rect_t tB)
{
	return tA.fX == tB.fX && tA.fY == tB.fY &&
	       tA.fW == tB.fW && tA.fH == tB.fH;
}

static int __xuiMsgBoxMeasureText(xui_msgbox pBox, float fWidth,
	float fHeight, xui_vec2_t* pSize, int* pLineCount)
{
	xui_text_layout_desc_t tDesc;
	xui_text_layout pLayout;
	xui_font pFont;
	int iRet;

	if ( (pBox == NULL) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pSize->fX = 0.0f;
	pSize->fY = 0.0f;
	if ( pLineCount != NULL ) {
		*pLineCount = 0;
	}
	pFont = __xuiMsgBoxFont(pBox);
	if ( pFont == NULL ) {
		if ( pBox->sMessage != NULL && pBox->sMessage[0] != '\0' && fHeight > 0.0f ) {
			pSize->fX = __xuiMsgBoxMin((float)strlen(pBox->sMessage) * 7.0f, fWidth);
			pSize->fY = __xuiMsgBoxMin(18.0f, fHeight);
			if ( pLineCount != NULL ) *pLineCount = 1;
		}
		return XUI_OK;
	}
	if ( fWidth <= 0.0f || fHeight <= 0.0f ) return XUI_OK;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = (pBox->sMessage != NULL) ? pBox->sMessage : "";
	tDesc.iTextSize = -1;
	tDesc.pFont = pFont;
	tDesc.fMaxWidth = fWidth;
	tDesc.fMaxHeight = fHeight;
	tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
	tDesc.iFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP;
	tDesc.fLineGap = 2.0f;
	pLayout = NULL;
	iRet = xuiTextLayoutCreate(pBox->pContext, &pLayout, &tDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	*pSize = xuiTextLayoutGetSize(pLayout);
	if ( pLineCount != NULL ) {
		*pLineCount = xuiTextLayoutGetLineCount(pLayout);
	}
	xuiTextLayoutDestroy(pLayout);
	return XUI_OK;
}

static int __xuiMsgBoxHasIcon(xui_msgbox pBox)
{
	if ( pBox == NULL ) return 0;
	if ( pBox->bCustomIcon && (pBox->pIconSurface != NULL) ) return 1;
	return pBox->iType != XUI_MSGBOX_ICON_NONE;
}

static const char* __xuiMsgBoxPresetButtonTitle(xui_msgbox pBox, int iTitle)
{
	int iIndex;

	iIndex = __xuiMsgBoxPresetButtonTitleIndex(iTitle);
	if ( (pBox != NULL) && (iIndex >= 0) && (pBox->arrPresetButtonTitle[iIndex] != NULL) ) {
		return pBox->arrPresetButtonTitle[iIndex];
	}
	return __xuiMsgBoxDefaultPresetButtonTitle(iTitle);
}

static int __xuiMsgBoxApplyPresetButtons(xui_msgbox pBox, int iButtons)
{
	xui_msgbox_button_t arrButtons[3];
	int iCount;

	if ( !__xuiMsgBoxValid(pBox) || !__xuiMsgBoxButtonsValid(iButtons) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(arrButtons, 0, sizeof(arrButtons));
	iCount = 0;
	if ( iButtons == XUI_MSGBOX_BUTTON_OK ) {
		arrButtons[0] = (xui_msgbox_button_t){__xuiMsgBoxPresetButtonTitle(pBox, XUI_MSGBOX_BUTTON_TITLE_OK), XUI_MSGBOX_RESULT_OK, XUI_BUTTON_SEMANTIC_PRIMARY};
		iCount = 1;
	} else if ( iButtons == XUI_MSGBOX_BUTTON_OK_CANCEL ) {
		arrButtons[0] = (xui_msgbox_button_t){__xuiMsgBoxPresetButtonTitle(pBox, XUI_MSGBOX_BUTTON_TITLE_OK), XUI_MSGBOX_RESULT_OK, XUI_BUTTON_SEMANTIC_PRIMARY};
		arrButtons[1] = (xui_msgbox_button_t){__xuiMsgBoxPresetButtonTitle(pBox, XUI_MSGBOX_BUTTON_TITLE_CANCEL), XUI_MSGBOX_RESULT_CANCEL, XUI_BUTTON_SEMANTIC_DEFAULT};
		iCount = 2;
	} else if ( iButtons == XUI_MSGBOX_BUTTON_YES_NO ) {
		arrButtons[0] = (xui_msgbox_button_t){__xuiMsgBoxPresetButtonTitle(pBox, XUI_MSGBOX_BUTTON_TITLE_YES), XUI_MSGBOX_RESULT_YES, XUI_BUTTON_SEMANTIC_PRIMARY};
		arrButtons[1] = (xui_msgbox_button_t){__xuiMsgBoxPresetButtonTitle(pBox, XUI_MSGBOX_BUTTON_TITLE_NO), XUI_MSGBOX_RESULT_NO, XUI_BUTTON_SEMANTIC_DEFAULT};
		iCount = 2;
	} else if ( iButtons == XUI_MSGBOX_BUTTON_YES_NO_CANCEL ) {
		arrButtons[0] = (xui_msgbox_button_t){__xuiMsgBoxPresetButtonTitle(pBox, XUI_MSGBOX_BUTTON_TITLE_YES), XUI_MSGBOX_RESULT_YES, XUI_BUTTON_SEMANTIC_PRIMARY};
		arrButtons[1] = (xui_msgbox_button_t){__xuiMsgBoxPresetButtonTitle(pBox, XUI_MSGBOX_BUTTON_TITLE_NO), XUI_MSGBOX_RESULT_NO, XUI_BUTTON_SEMANTIC_DEFAULT};
		arrButtons[2] = (xui_msgbox_button_t){__xuiMsgBoxPresetButtonTitle(pBox, XUI_MSGBOX_BUTTON_TITLE_CANCEL), XUI_MSGBOX_RESULT_CANCEL, XUI_BUTTON_SEMANTIC_DEFAULT};
		iCount = 3;
	}
	return __xuiMsgBoxApplyButtonModel(pBox, iButtons, arrButtons, iCount);
}

static int __xuiMsgBoxApplyButtonVisual(xui_msgbox pBox)
{
	int i;
	int iRet;

	if ( !__xuiMsgBoxValid(pBox) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < XUI_MSGBOX_BUTTON_CAPACITY; i++ ) {
		if ( pBox->arrButtons[i] == NULL ) {
			continue;
		}
		iRet = xuiButtonSetFont(pBox->arrButtons[i], __xuiMsgBoxFont(pBox));
		if ( iRet == XUI_OK ) iRet = xuiButtonSetBorder(pBox->arrButtons[i], 1.0f, XUI_COLOR_RGBA(166, 196, 224, 255));
		if ( iRet == XUI_OK ) iRet = xuiButtonSetDisabledTextColor(pBox->arrButtons[i], pBox->tColors.iMutedTextColor);
		if ( pBox->arrButtonSemantic[i] == XUI_BUTTON_SEMANTIC_DEFAULT ) {
			if ( iRet == XUI_OK ) iRet = xuiButtonSetColors(pBox->arrButtons[i],
				pBox->tColors.iButtonColor,
				pBox->tColors.iButtonHoverColor,
				pBox->tColors.iButtonActiveColor,
				pBox->tColors.iButtonFocusColor,
				pBox->tColors.iButtonDisabledColor);
		}
		if ( iRet != XUI_OK ) return iRet;
	}
	pBox->bPaintReady = 0;
	return XUI_OK;
}

static float __xuiMsgBoxMeasureButtonWidth(xui_msgbox pBox, int iIndex)
{
	xui_vec2_t tContentSize;
	xui_thickness_t tPadding;
	xui_widget pButton;
	float fWidth;

	fWidth = pBox->tMetrics.fButtonWidth;
	if ( (iIndex < 0) || (iIndex >= pBox->iButtonCount) ) {
		return (float)xuiInternalPixelCeil(fWidth);
	}
	pButton = pBox->arrButtons[iIndex];
	if ( pButton == NULL ) {
		return (float)xuiInternalPixelCeil(fWidth);
	}
	tContentSize = (xui_vec2_t){0.0f, 0.0f};
	if ( xuiWidgetMeasureContent(pButton,
		(xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED},
		&tContentSize) == XUI_OK ) {
		tPadding = xuiWidgetGetPadding(pButton);
		fWidth = __xuiMsgBoxMax(fWidth,
			tContentSize.fX + tPadding.fLeft + tPadding.fRight);
	}
	return (float)xuiInternalPixelCeil(fWidth);
}

static int __xuiMsgBoxLayout(xui_msgbox pBox)
{
	xui_rect_t tRoot;
	xui_rect_t tWin;
	xui_rect_t tContent;
	xui_vec2_t tTextSize;
	float arrButtonWidth[XUI_MSGBOX_BUTTON_CAPACITY];
	float fTitleBar;
	float fBorder;
	float fResizeGrip;
	float fButtonSize;
	float fRootMaxWidth;
	float fRootAvailableWidth;
	float fRootAvailableHeight;
	float fMaxWidth;
	float fMinWidth;
	float fWidth;
	float fClientW;
	float fClientH;
	float fIconBlock;
	float fIconSize;
	float fIconGap;
	float fMessageW;
	float fMessageH;
	float fMaxRowH;
	float fPaddingX;
	float fRowH;
	float fPaddingY;
	float fFooterHeight;
	float fFooterTop;
	float fButtonsW;
	float fButtonsRequiredWidth;
	float fButtonArea;
	float fButtonWidthArea;
	float fButtonScale;
	float fButtonGap;
	float fButtonHeight;
	float fButtonsX;
	float fButtonsY;
	float fButtonX;
	float fWindowH;
	int bIcon;
	int iWindowWidth;
	int iWindowHeight;
	int i;
	int iRet;

	if ( !__xuiMsgBoxValid(pBox) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tRoot = __xuiMsgBoxRootRect(pBox);
	fTitleBar = 30.0f;
	fBorder = 1.0f;
	fResizeGrip = 0.0f;
	fButtonSize = 28.0f;
	(void)xuiWindowGetChrome(pBox->pWindow, &fTitleBar, &fBorder, &fResizeGrip, &fButtonSize);
	(void)fResizeGrip;
	(void)fButtonSize;
	bIcon = __xuiMsgBoxHasIcon(pBox);
	fRootAvailableWidth = (tRoot.fW > 24) ? (float)(tRoot.fW - 24) : (float)tRoot.fW;
	fRootAvailableHeight = (tRoot.fH > 24) ? (float)(tRoot.fH - 24) : (float)tRoot.fH;
	if ( fRootAvailableWidth < 1.0f ) fRootAvailableWidth = 1.0f;
	if ( fRootAvailableHeight < 1.0f ) fRootAvailableHeight = 1.0f;
	fPaddingX = __xuiMsgBoxMin(pBox->tMetrics.fPaddingX,
		__xuiMsgBoxMax(0.0f, (fRootAvailableWidth - fBorder * 2.0f) * 0.5f));
	fClientH = __xuiMsgBoxMax(0.0f,
		fRootAvailableHeight - fTitleBar - fBorder * 2.0f);
	fFooterHeight = __xuiMsgBoxMin(pBox->tMetrics.fFooterHeight,
		fClientH);
	fPaddingY = __xuiMsgBoxMin(pBox->tMetrics.fPaddingY,
		__xuiMsgBoxMax(0.0f, fClientH - fFooterHeight));
	fMaxRowH = __xuiMsgBoxMax(0.0f,
		fClientH - fFooterHeight - fPaddingY);
	fIconSize = bIcon ? __xuiMsgBoxMin(pBox->tMetrics.fIconSize,
		__xuiMsgBoxMin(fMaxRowH,
			__xuiMsgBoxMax(0.0f, fRootAvailableWidth - fBorder * 2.0f -
				fPaddingX * 2.0f))) : 0.0f;
	fIconGap = (fIconSize > 0.0f) ?
		__xuiMsgBoxMin(pBox->tMetrics.fIconGap,
			__xuiMsgBoxMax(0.0f, fRootAvailableWidth - fBorder * 2.0f -
				fPaddingX * 2.0f - fIconSize)) : 0.0f;
	fIconBlock = (fIconSize > 0.0f) ?
		(fIconSize + fIconGap) : 0.0f;
	fButtonsW = 0.0f;
	for ( i = 0; i < XUI_MSGBOX_BUTTON_CAPACITY; i++ ) {
		arrButtonWidth[i] = 0.0f;
		if ( i < pBox->iButtonCount ) {
			arrButtonWidth[i] = __xuiMsgBoxMeasureButtonWidth(pBox, i);
			fButtonsW += arrButtonWidth[i];
		}
	}
	if ( pBox->iButtonCount > 1 ) {
		fButtonsW += (float)(pBox->iButtonCount - 1) * pBox->tMetrics.fButtonGap;
	}
	fButtonsRequiredWidth = fBorder * 2.0f + fPaddingX * 2.0f + fButtonsW;
	fRootMaxWidth = tRoot.fW * pBox->tMetrics.fMaxWidthRatio;
	fMaxWidth = __xuiMsgBoxMin(pBox->tMetrics.fMaxWidth, fRootMaxWidth);
	if ( fMaxWidth < __xuiMsgBoxMin(220.0f, fRootAvailableWidth) ) {
		fMaxWidth = __xuiMsgBoxMin(fRootAvailableWidth,
			pBox->tMetrics.fMaxWidth);
	}
	fMaxWidth = __xuiMsgBoxClamp(fMaxWidth, 1.0f,
		fRootAvailableWidth);
	if ( fButtonsRequiredWidth > fMaxWidth ) {
		fMaxWidth = __xuiMsgBoxMin(fButtonsRequiredWidth, fRootAvailableWidth);
	}
	fMinWidth = __xuiMsgBoxMin(pBox->tMetrics.fMinWidth, fMaxWidth);
	fMessageW = __xuiMsgBoxMax(1.0f, fMaxWidth - fBorder * 2.0f -
		fPaddingX * 2.0f - fIconBlock);
	iRet = __xuiMsgBoxMeasureText(pBox, fMessageW,
		XUI_LAYOUT_UNBOUNDED, &tTextSize, &pBox->iWrapLineCount);
	if ( iRet != XUI_OK ) return iRet;
	fWidth = __xuiMsgBoxMax(
		fBorder * 2.0f + fPaddingX * 2.0f + fIconBlock + tTextSize.fX,
		fButtonsRequiredWidth);
	fWidth = __xuiMsgBoxClamp(fWidth, fMinWidth, fMaxWidth);
	iWindowWidth = xuiInternalSnapSize(fWidth);
	if ( iWindowWidth > (int)fRootAvailableWidth )
		iWindowWidth = (int)fRootAvailableWidth;
	if ( iWindowWidth < 1 ) iWindowWidth = 1;
	fWidth = (float)iWindowWidth;
	fClientW = __xuiMsgBoxMax(1.0f, fWidth - fBorder * 2.0f);
	fMessageW = __xuiMsgBoxMax(0.0f, fClientW -
		fPaddingX * 2.0f - fIconBlock);
	iRet = __xuiMsgBoxMeasureText(pBox, fMessageW, fMaxRowH,
		&tTextSize, &pBox->iWrapLineCount);
	if ( iRet != XUI_OK ) return iRet;
	fMessageH = __xuiMsgBoxMax(tTextSize.fY,
		__xuiMsgBoxMin(pBox->tMetrics.fMinMessageHeight, fMaxRowH));
	fMessageH = __xuiMsgBoxMin(fMessageH, fMaxRowH);
	fRowH = __xuiMsgBoxMax(fMessageH, fIconSize);
	fClientH = fPaddingY + fRowH + fFooterHeight;
	fWindowH = fTitleBar + fClientH + fBorder * 2.0f;
	iWindowHeight = xuiInternalSnapSize(fWindowH);
	if ( iWindowHeight > (int)fRootAvailableHeight )
		iWindowHeight = (int)fRootAvailableHeight;
	if ( iWindowHeight < 1 ) iWindowHeight = 1;
	tWin = xuiInternalRectFromFloatNearest(
		(float)tRoot.fX + ((float)tRoot.fW - (float)iWindowWidth) * 0.5f,
		(float)tRoot.fY + ((float)tRoot.fH - (float)iWindowHeight) * 0.5f,
		(float)iWindowWidth, (float)iWindowHeight);
	iRet = xuiWindowSetMinSize(pBox->pWindow,
		__xuiMsgBoxMin(pBox->tMetrics.fMinWidth, (float)tWin.fW),
		__xuiMsgBoxMin(120.0f, (float)tWin.fH));
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetRect(pBox->pWindow, tWin);
	if ( iRet != XUI_OK ) return iRet;
	tContent = xuiInternalRectFromFloatNearest(0.0f, 0.0f,
		__xuiMsgBoxMax(0.0f, (float)tWin.fW - fBorder * 2.0f),
		__xuiMsgBoxMax(0.0f, (float)tWin.fH - fTitleBar - fBorder * 2.0f));
	iRet = xuiWidgetSetRect(pBox->pContent, tContent);
	if ( iRet != XUI_OK ) return iRet;
	fClientW = (float)tContent.fW;
	fClientH = (float)tContent.fH;
	if ( fIconSize > 0.0f ) {
		pBox->tIconRect = xuiInternalRectFromFloatNearest(
			fPaddingX,
			fPaddingY + (fRowH - fIconSize) * 0.5f,
			fIconSize, fIconSize);
	} else {
		pBox->tIconRect = (xui_rect_t){0, 0, 0, 0};
	}
	fMessageW = __xuiMsgBoxMax(0.0f, fClientW -
		fPaddingX * 2.0f - fIconBlock);
	pBox->tMessageRect = xuiInternalRectFromFloatNearest(
		fPaddingX + fIconBlock,
		fPaddingY + (fRowH - fMessageH) * 0.5f,
		fMessageW, fMessageH);
	fFooterHeight = __xuiMsgBoxMin(fFooterHeight, fClientH);
	fFooterTop = fClientH - fFooterHeight;
	fButtonHeight = __xuiMsgBoxMin(pBox->tMetrics.fButtonHeight,
		fFooterHeight);
	fButtonArea = __xuiMsgBoxMax(0.0f,
		fClientW - fPaddingX * 2.0f);
	fButtonGap = pBox->tMetrics.fButtonGap;
	if ( pBox->iButtonCount > 1 ) {
		float fMaxGap = __xuiMsgBoxMax(0.0f,
			(fButtonArea - (float)pBox->iButtonCount) /
			(float)(pBox->iButtonCount - 1));
		if ( fButtonGap > fMaxGap ) fButtonGap = fMaxGap;
	}
	fButtonWidthArea = fButtonArea;
	if ( pBox->iButtonCount > 1 ) {
		fButtonWidthArea = __xuiMsgBoxMax(0.0f, fButtonArea -
			(float)(pBox->iButtonCount - 1) * fButtonGap);
	}
	fButtonsW = 0.0f;
	for ( i = 0; i < pBox->iButtonCount; i++ )
		fButtonsW += arrButtonWidth[i];
	if ( fButtonsW > fButtonWidthArea && fButtonsW > 0.0f ) {
		fButtonScale = fButtonWidthArea / fButtonsW;
		for ( i = 0; i < pBox->iButtonCount; i++ )
			arrButtonWidth[i] *= fButtonScale;
		fButtonsW = fButtonWidthArea;
	}
	if ( pBox->iButtonCount > 1 )
		fButtonsW += (float)(pBox->iButtonCount - 1) * fButtonGap;
	fButtonsX = __xuiMsgBoxMax(fPaddingX,
		fClientW - fPaddingX - fButtonsW);
	fButtonsY = fFooterTop + (fFooterHeight - fButtonHeight) * 0.5f;
	fButtonX = fButtonsX;
	for ( i = 0; i < XUI_MSGBOX_BUTTON_CAPACITY; i++ ) {
		if ( i < pBox->iButtonCount ) {
			pBox->arrButtonRect[i] = xuiInternalRectFromFloatNearest(
				fButtonX, fButtonsY, arrButtonWidth[i], fButtonHeight);
			fButtonX += arrButtonWidth[i] + fButtonGap;
			if ( pBox->arrButtons[i] != NULL ) {
				iRet = xuiWidgetSetRect(pBox->arrButtons[i], pBox->arrButtonRect[i]);
				if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pBox->arrButtons[i], 1);
				if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pBox->arrButtons[i], 1);
				if ( iRet != XUI_OK ) return iRet;
			}
		} else {
			pBox->arrButtonRect[i] = (xui_rect_t){0, 0, 0, 0};
			if ( pBox->arrButtons[i] != NULL ) {
				iRet = xuiWidgetSetVisible(pBox->arrButtons[i], 0);
				if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pBox->arrButtons[i], 0);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	pBox->tBackdropRect = tRoot;
	iRet = xuiWidgetSetRect(pBox->pBackdrop, tRoot);
	if ( iRet == XUI_OK ) iRet = xuiWidgetInvalidate(pBox->pWindow, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet == XUI_OK ) iRet = xuiWidgetInvalidate(pBox->pContent, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet == XUI_OK ) iRet = xuiWidgetInvalidate(pBox->pBackdrop, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return iRet;
}

static int __xuiMsgBoxSyncLayout(xui_msgbox pBox)
{
	xui_rect_t tRoot;

	if ( !__xuiMsgBoxValid(pBox) || !pBox->bOpen ) {
		return XUI_OK;
	}
	tRoot = __xuiMsgBoxRootRect(pBox);
	if ( __xuiMsgBoxRectSame(tRoot, pBox->tBackdropRect) ) {
		return XUI_OK;
	}
	return __xuiMsgBoxLayout(pBox);
}

static void __xuiMsgBoxStyleButtons(xui_msgbox pBox)
{
	static const uint32_t arrStates[] = {0, XUI_WIDGET_STATE_HOVER, XUI_WIDGET_STATE_ACTIVE,
		XUI_WIDGET_STATE_FOCUS, XUI_WIDGET_STATE_DISABLED, XUI_BUTTON_STATE_CHECKED};
	static const char* arrFillNames[] = {"msgbox.button.color", "msgbox.button.hover_color",
		"msgbox.button.active_color", "msgbox.button.color", "msgbox.button.disabled_color",
		"msgbox.button.active_color"};
	int i, j;
	for ( i = 0; i < XUI_MSGBOX_BUTTON_CAPACITY; ++i ) {
		if ( pBox->arrButtons[i] == NULL ) continue;
		{
			uint32_t iOld = xuiButtonGetDisabledTextColor(pBox->arrButtons[i]);
			uint32_t iColor = iOld;
			if ( pBox->arrStyledMuted[i] && iColor == pBox->arrAppliedMuted[i] ) iColor = pBox->arrBaseMuted[i];
			pBox->arrBaseMuted[i] = iColor;
			pBox->arrStyledMuted[i] = __xuiMsgBoxStyleColor(pBox, "msgbox.text.muted_color", &iColor);
			pBox->arrAppliedMuted[i] = iColor;
			if ( iColor != iOld ) (void)xuiButtonSetDisabledTextColor(pBox->arrButtons[i], iColor);
		}
		for ( j = 0; j < 6; ++j ) {
			xui_msgbox_button_paint_t* pPaint = &pBox->arrButtonPaint[i][j];
			uint32_t iFill, iBorder, iOldFill, iOldBorder;
			float fWidth;
			int iMask = 0;
			(void)xuiButtonGetStateVisual(pBox->arrButtons[i], arrStates[j], &iOldFill, &fWidth, &iOldBorder);
			iFill = iOldFill;
			iBorder = iOldBorder;
			/* Keep caller edits made while a service style is active as the new base. */
			if ( (pPaint->iMask & 1) && iFill == pPaint->iFill ) iFill = pPaint->iBaseFill;
			if ( (pPaint->iMask & 2) && iBorder == pPaint->iBorder ) iBorder = pPaint->iBaseBorder;
			pPaint->iBaseFill = iFill;
			pPaint->iBaseBorder = iBorder;
			if ( __xuiMsgBoxStyleColor(pBox, arrFillNames[j], &iFill) ) iMask |= 1;
			if ( __xuiMsgBoxStyleColor(pBox, "msgbox.button.border_color", &iBorder) ) iMask |= 2;
			if ( j == 3 && __xuiMsgBoxStyleColor(pBox, "msgbox.button.focus_color", &iBorder) ) iMask |= 2;
			pPaint->iFill = iFill;
			pPaint->iBorder = iBorder;
			pPaint->iMask = iMask;
			if ( iFill != iOldFill || iBorder != iOldBorder )
				(void)xuiButtonSetStateVisual(pBox->arrButtons[i], arrStates[j], iFill, fWidth, iBorder);
		}
	}
}

static int __xuiMsgBoxSyncPaint(xui_msgbox pBox)
{
	uint32_t iHash;
	if ( !__xuiMsgBoxValid(pBox) || pBox->pContent == NULL || pBox->pBackdrop == NULL ) return 0;
	iHash = xuiWidgetGetStyleHash(pBox->pWindow);
	__xuiMsgBoxStyleButtons(pBox);
	if ( pBox->bPaintReady && pBox->iPaintStyleHash == iHash ) return 0;
	pBox->iPaintStyleHash = iHash;
	pBox->bPaintReady = 1;
	(void)xuiWidgetInvalidate(pBox->pContent, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	(void)xuiWidgetInvalidate(pBox->pBackdrop, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	(void)xuiWidgetInvalidate(xuiWindowGetClientWidget(pBox->pWindow), XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return 1;
}

static int __xuiMsgBoxPreparePaint(xui_widget pWidget)
{
	xui_context pContext = xuiWidgetGetContext(pWidget);
	xui_widget_type pParent = xuiWidgetFindType(pContext, "msgbox")->pParent;
	int iRet;
	/* The framework selects one hook; preserve Window's chrome dependencies. */
	for ( ; pParent != NULL && pParent->onPreparePaint == NULL; pParent = pParent->pParent ) {}
	if ( pParent != NULL ) {
		iRet = pParent->onPreparePaint(pWidget);
		if ( iRet != XUI_OK ) return iRet;
		if ( !xuiInternalWidgetIsValid(pWidget) || xuiInternalContextDestroyPending(pContext) || !xuiWidgetGetVisible(pWidget) ) return XUI_OK;
	}
	(void)__xuiMsgBoxSyncPaint((xui_msgbox)pWidget->pUpdateUser);
	return XUI_OK;
}

static int __xuiMsgBoxBackdropPreparePaint(xui_widget pWidget)
{
	/* The backdrop is an earlier overlay sibling of its window. */
	(void)__xuiMsgBoxSyncPaint((xui_msgbox)pWidget->pCacheRenderUser);
	return XUI_OK;
}

static int __xuiMsgBoxClientRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_msgbox pBox = (xui_msgbox)pUser;
	xui_proxy pProxy;
	uint32_t iColor;
	(void)iStateId;
	if ( !__xuiMsgBoxValid(pBox) ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxy = xuiInternalContextGetProxy(pBox->pContext);
	(void)xuiWindowGetColors(pBox->pWindow, NULL, &iColor, NULL, NULL, NULL, NULL, NULL, NULL);
	iColor = __xuiMsgBoxColor(pBox, "window.client.color", iColor);
	iColor = __xuiMsgBoxColor(pBox, "msgbox.client.color", iColor);
	if ( (iColor & 0xffu) == 0u || pProxy == NULL || pProxy->drawRectFill == NULL ) return XUI_OK;
	return pProxy->drawRectFill(pProxy, pDraw, xuiWidgetGetContentRect(pWidget), iColor);
}

static int __xuiMsgBoxWindowUpdate(xui_widget pWidget, float fDelta, void* pUser)
{
	(void)pWidget;
	(void)fDelta;
	(void)__xuiMsgBoxSyncPaint((xui_msgbox)pUser);
	return __xuiMsgBoxSyncLayout((xui_msgbox)pUser);
}

static int __xuiMsgBoxDrawTextLayout(xui_msgbox pBox, xui_draw_context pDraw)
{
	xui_text_layout_desc_t tDesc;
	xui_text_layout pLayout;
	xui_proxy pProxy;
	xui_font pFont;
	int iRet;

	pProxy = xuiInternalContextGetProxy(pBox->pContext);
	pFont = __xuiMsgBoxFont(pBox);
	if ( (pProxy == NULL) || (pProxy->drawText == NULL) || (pDraw == NULL) || (pFont == NULL) ) {
		return XUI_OK;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = (pBox->sMessage != NULL) ? pBox->sMessage : "";
	tDesc.iTextSize = -1;
	tDesc.pFont = pFont;
	tDesc.fMaxWidth = pBox->tMessageRect.fW;
	tDesc.fMaxHeight = pBox->tMessageRect.fH;
	tDesc.iWrapMode = XUI_TEXT_WRAP_WORD;
	tDesc.iFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP;
	tDesc.fLineGap = 2.0f;
	pLayout = NULL;
	iRet = xuiTextLayoutCreate(pBox->pContext, &pLayout, &tDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiInternalTextLayoutDrawContext(pLayout, pDraw,
		pBox->tMessageRect,
		__xuiMsgBoxColor(pBox, "msgbox.text.color",
			pBox->tColors.iMessageColor), 0);
	xuiTextLayoutDestroy(pLayout);
	return iRet;
}

static int __xuiMsgBoxDrawIconFallback(xui_msgbox pBox, xui_draw_context pDraw, xui_proxy pProxy)
{
	xui_rect_t tIcon;
	xui_rect_t tText;
	uint32_t iColor;
	int iRet;

	if ( (pBox == NULL) || (pProxy == NULL) || (pDraw == NULL) ) {
		return XUI_OK;
	}
	tIcon = xuiInternalSnapRect(pBox->tIconRect);
	iColor = __xuiMsgBoxColor(pBox, "msgbox.icon.color", pBox->tColors.iIconColor);
	if ( pProxy->drawCircleFill != NULL ) {
		iRet = pProxy->drawCircleFill(pProxy, pDraw, tIcon.fX + tIcon.fW * 0.5f, tIcon.fY + tIcon.fH * 0.5f, tIcon.fW * 0.5f, iColor);
	} else if ( pProxy->drawRectFill != NULL ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, tIcon, iColor);
	} else {
		iRet = XUI_OK;
	}
	if ( iRet != XUI_OK ) return iRet;
	if ( pProxy->drawText != NULL ) {
		tText = tIcon;
		return pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=__xuiMsgBoxFont(pBox), .sText=__xuiMsgBoxIconFallbackText(pBox->iType), .iTextSize=1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tText, __xuiMsgBoxColor(pBox, "msgbox.icon.text.color", XUI_COLOR_WHITE), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	return XUI_OK;
}

static int __xuiMsgBoxDrawIcon(xui_msgbox pBox, xui_draw_context pDraw, xui_proxy pProxy)
{
	xui_surface pSurface;
	xui_rect_t tSrc;
	const char* sName;
	int iRet;

	if ( !__xuiMsgBoxHasIcon(pBox) ) {
		return XUI_OK;
	}
	if ( (pProxy == NULL) || (pProxy->drawSurface == NULL) ) {
		return __xuiMsgBoxDrawIconFallback(pBox, pDraw, pProxy);
	}
	pSurface = NULL;
	tSrc = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	if ( pBox->bCustomIcon && pBox->pIconSurface != NULL ) {
		pSurface = pBox->pIconSurface;
		tSrc = pBox->tIconSrc;
	} else if ( pBox->bBuiltinIcon ) {
		sName = __xuiMsgBoxIconName(pBox->iType);
		if ( sName != NULL ) {
			iRet = xuiBuiltinAssetGetAtlas(pBox->pContext, &pSurface);
			if ( iRet == XUI_OK ) {
				iRet = xuiBuiltinAssetGetRect(sName, &tSrc);
			}
			if ( iRet != XUI_OK ) {
				pSurface = NULL;
			}
		}
	}
	if ( pSurface == NULL ) {
		return __xuiMsgBoxDrawIconFallback(pBox, pDraw, pProxy);
	}
	return pProxy->drawSurface(pProxy, pDraw, pSurface, tSrc, xuiInternalSnapRect(pBox->tIconRect), pBox->bCustomIcon ? XUI_COLOR_WHITE :
		__xuiMsgBoxColor(pBox, "msgbox.icon.color", pBox->tColors.iIconColor), 0);
}

static int __xuiMsgBoxContentRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_msgbox pBox;
	xui_proxy pProxy;
	int iRet;

	(void)pWidget;
	(void)iStateId;
	pBox = (xui_msgbox)pUser;
	if ( !__xuiMsgBoxValid(pBox) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pBox->pContext);
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	iRet = __xuiMsgBoxDrawIcon(pBox, pDraw, pProxy);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiMsgBoxDrawTextLayout(pBox, pDraw);
}

static int __xuiMsgBoxBackdropRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_msgbox pBox;
	xui_proxy pProxy;
	xui_rect_t tRect;

	(void)iStateId;
	pBox = (xui_msgbox)pUser;
	if ( !__xuiMsgBoxValid(pBox) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(pBox->pContext);
	if ( (pProxy == NULL) || (pProxy->drawRectFill == NULL) ) {
		return XUI_OK;
	}
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	return pProxy->drawRectFill(pProxy, pDraw, xuiInternalSnapRect(tRect), __xuiMsgBoxColor(pBox, "msgbox.backdrop.color", pBox->tColors.iBackdropColor));
}

static int __xuiMsgBoxBackdropEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_msgbox pBox;

	(void)pWidget;
	pBox = (xui_msgbox)pUser;
	if ( !__xuiMsgBoxValid(pBox) || (pEvent == NULL) || !pBox->bOpen || !pBox->bModal ) {
		return XUI_OK;
	}
	if ( pEvent->iType == XUI_EVENT_POINTER_DOWN ) {
		xui_widget pFocus = (pBox->iButtonCount > 0) ? pBox->arrButtons[0] : pBox->pWindow;
		(void)xuiSetFocusWidget(pBox->pContext, pFocus);
	}
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_LEAVE:
	case XUI_EVENT_POINTER_MOVE:
	case XUI_EVENT_POINTER_DOWN:
	case XUI_EVENT_POINTER_UP:
	case XUI_EVENT_POINTER_CLICK:
	case XUI_EVENT_POINTER_WHEEL:
	case XUI_EVENT_KEY_DOWN:
	case XUI_EVENT_KEY_UP:
	case XUI_EVENT_TEXT:
		return XUI_EVENT_DISPATCH_STOP;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiMsgBoxWindowEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_msgbox pBox;
	xui_context pContext;
	int iRet;
	int bSkipWindowEvent;

	pBox = (xui_msgbox)pUser;
	pContext = xuiWidgetGetContext(pWidget);
	iRet = XUI_OK;
	bSkipWindowEvent = (pEvent != NULL) &&
		(pEvent->iType == XUI_EVENT_POINTER_DOWN) &&
		(pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE) &&
		(pEvent->pTarget != pWidget);
	if ( !bSkipWindowEvent && __xuiMsgBoxValid(pBox) && (pBox->onWindowEvent != NULL) ) {
		iRet = pBox->onWindowEvent(pWidget, pEvent, pBox->pWindowEventUser);
		if ( iRet < 0 ) {
			return iRet;
		}
	}
	if ( !xuiInternalWidgetIsValid(pWidget) || xuiInternalContextDestroyPending(pContext) ) {
		return iRet;
	}
	if ( __xuiMsgBoxValid(pBox) && pBox->bOpen && pBox->bModal ) {
		(void)xuiInternalWidgetSetTreeLayer(pWidget, XUI_LAYER_MODAL, pBox->iModalZ + 1);
	}
	return iRet;
}

static void __xuiMsgBoxCloseResult(xui_msgbox pBox, int iResult)
{
	if ( !__xuiMsgBoxValid(pBox) ) {
		return;
	}
	pBox->iResult = iResult;
	if ( pBox->iResultCount < INT_MAX ) pBox->iResultCount++;
	(void)xuiMsgBoxSetOpen(pBox, 0);
	if ( pBox->onResult != NULL ) {
		pBox->onResult(pBox, iResult, pBox->pResultUser);
	}
}

static void __xuiMsgBoxWindowClose(xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	__xuiMsgBoxCloseResult((xui_msgbox)pUser, XUI_MSGBOX_RESULT_CLOSE);
}

static void __xuiMsgBoxDefaultAction(xui_widget pWidget, void* pUser)
{
	xui_msgbox pBox;

	(void)pWidget;
	pBox = (xui_msgbox)pUser;
	if ( !__xuiMsgBoxValid(pBox) ) {
		return;
	}
	__xuiMsgBoxCloseResult(pBox, (pBox->iButtonCount > 0) ? pBox->arrButtonResult[0] : XUI_MSGBOX_RESULT_OK);
}

static void __xuiMsgBoxCancelAction(xui_widget pWidget, void* pUser)
{
	xui_msgbox pBox;
	int i;

	(void)pWidget;
	pBox = (xui_msgbox)pUser;
	if ( !__xuiMsgBoxValid(pBox) ) return;
	for ( i = 0; i < pBox->iButtonCount; i++ ) {
		if ( pBox->arrButtonResult[i] == XUI_MSGBOX_RESULT_CANCEL ) {
			__xuiMsgBoxCloseResult(pBox, XUI_MSGBOX_RESULT_CANCEL);
			return;
		}
	}
	__xuiMsgBoxCloseResult(pBox, XUI_MSGBOX_RESULT_CLOSE);
}

static void __xuiMsgBoxButtonClick(xui_widget pWidget, void* pUser)
{
	xui_msgbox pBox;
	int i;

	pBox = (xui_msgbox)pUser;
	if ( !__xuiMsgBoxValid(pBox) ) {
		return;
	}
	for ( i = 0; i < pBox->iButtonCount; i++ ) {
		if ( pBox->arrButtons[i] == pWidget ) {
			__xuiMsgBoxCloseResult(pBox, pBox->arrButtonResult[i]);
			return;
		}
	}
}

static int __xuiMsgBoxApplyOverlay(xui_msgbox pBox)
{
	int iRet;

	if ( !__xuiMsgBoxValid(pBox) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pBox->bOpen && pBox->bModal ) {
		if ( xuiWidgetGetParent(pBox->pBackdrop) == NULL ) {
			pBox->iModalZ = xuiInternalOverlayNextZ(pBox->pContext, XUI_LAYER_MODAL);
		}
		iRet = xuiWidgetSetVisible(pBox->pBackdrop, 1);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiWidgetSetEnabled(pBox->pBackdrop, 1);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiOverlayAttach(pBox->pContext, pBox->pWindow, pBox->pBackdrop, XUI_LAYER_MODAL, pBox->iModalZ);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiInternalWidgetSetTreeLayer(pBox->pWindow, XUI_LAYER_MODAL, pBox->iModalZ + 1);
		if ( iRet != XUI_OK ) return iRet;
		pBox->pWindow->bModalFocus = 1;
		return xuiOverlayBringToFront(pBox->pWindow);
	}
	pBox->pWindow->bModalFocus = 0;
	iRet = xuiWidgetSetVisible(pBox->pBackdrop, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetEnabled(pBox->pBackdrop, 0);
	if ( iRet != XUI_OK ) return iRet;
	if ( xuiWidgetGetParent(pBox->pBackdrop) != NULL ) {
		iRet = xuiOverlayDetach(pBox->pBackdrop);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pBox->bOpen ) {
		iRet = xuiInternalWidgetSetTreeLayer(pBox->pWindow, XUI_LAYER_FLOATING, XUI_WINDOW_Z_TOPMOST);
		if ( iRet != XUI_OK ) return iRet;
		return xuiOverlayBringToFront(pBox->pWindow);
	}
	return XUI_OK;
}

XUI_API int xuiMsgBoxCreate(xui_context pContext, xui_msgbox* ppBox, const xui_msgbox_desc_t* pDesc)
{
	xui_msgbox pBox;
	xui_window_desc_t tWindowDesc;
	xui_button_desc_t tButtonDesc;
	xui_widget_type pType;
	xui_widget pClient;
	int i;
	int iRet;

	if ( ppBox == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppBox = NULL;
	if ( !xuiInternalContextIsValid(pContext) || !__xuiMsgBoxDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = __xuiMsgBoxEnsureType(pContext);
	if ( pType == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	pBox = (xui_msgbox)xrtCalloc(1, sizeof(*pBox));
	if ( pBox == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pBox->iMagic = XUI_MSGBOX_MAGIC;
	pBox->pContext = pContext;
	pBox->pFont = (pDesc != NULL && pDesc->pFont != NULL) ? pDesc->pFont : xuiGetDefaultFont(pContext);
	__xuiMsgBoxDefaultMetrics(&pBox->tMetrics);
	__xuiMsgBoxDefaultColors(&pBox->tColors);
	if ( pDesc != NULL && pDesc->bHasMetrics ) pBox->tMetrics = pDesc->tMetrics;
	if ( pDesc != NULL && pDesc->bHasColors ) pBox->tColors = pDesc->tColors;
	pBox->bHasColors = pDesc != NULL && pDesc->bHasColors;
	pBox->tMetrics.iSize = sizeof(pBox->tMetrics);
	pBox->tColors.iSize = sizeof(pBox->tColors);
	pBox->iType = (pDesc != NULL) ? pDesc->iType : XUI_MSGBOX_ICON_INFO;
	pBox->iButtons = (pDesc != NULL) ? pDesc->iButtons : XUI_MSGBOX_BUTTON_OK;
	pBox->iResult = XUI_MSGBOX_RESULT_CLOSE;
	pBox->bModal = (pDesc != NULL && pDesc->bHasModal) ? (pDesc->bModal ? 1 : 0) : 1;
	pBox->bBuiltinIcon = 1;
	if ( pDesc != NULL && pDesc->bHasCustomIcon && pDesc->pIconSurface != NULL ) {
		pBox->pIconSurface = pDesc->pIconSurface;
		pBox->tIconSrc = pDesc->tIconSrc;
		pBox->bCustomIcon = 1;
		pBox->bBuiltinIcon = 0;
	}
	if ( !pBox->bHasColors ) pBox->tColors.iIconColor = __xuiMsgBoxIconColor(pBox->iType);
	iRet = __xuiMsgBoxReplaceText(&pBox->sTitle, (pDesc != NULL && pDesc->sTitle != NULL) ? pDesc->sTitle : "Message");
	if ( iRet == XUI_OK ) iRet = __xuiMsgBoxReplaceText(&pBox->sMessage, (pDesc != NULL && pDesc->sMessage != NULL) ? pDesc->sMessage : "");
	if ( iRet != XUI_OK ) {
		xuiMsgBoxDestroy(pBox);
		return iRet;
	}
	memset(&tWindowDesc, 0, sizeof(tWindowDesc));
	tWindowDesc.iSize = sizeof(tWindowDesc);
	tWindowDesc.sTitle = pBox->sTitle;
	tWindowDesc.pFont = pBox->pFont;
	tWindowDesc.bClosed = 1;
	tWindowDesc.bTopMost = 1;
	tWindowDesc.bNotResizable = 1;
	tWindowDesc.bHideCollapse = 1;
	tWindowDesc.bHideMaximize = 1;
	tWindowDesc.fTitleBarHeight = 30.0f;
	tWindowDesc.fBorderWidth = 1.0f;
	tWindowDesc.fButtonSize = 20.0f;
	tWindowDesc.iBackgroundColor = XUI_COLOR_RGBA(245, 250, 254, 255);
	tWindowDesc.iClientColor = pBox->tColors.iClientColor;
	tWindowDesc.iTitleBarColor = XUI_COLOR_RGBA(231, 241, 250, 255);
	tWindowDesc.iInactiveTitleBarColor = XUI_COLOR_RGBA(236, 244, 251, 255);
	tWindowDesc.iTitleTextColor = XUI_COLOR_RGBA(35, 54, 80, 255);
	tWindowDesc.iInactiveTitleTextColor = XUI_COLOR_RGBA(91, 108, 130, 255);
	tWindowDesc.iBorderColor = XUI_COLOR_RGBA(135, 173, 211, 255);
	tWindowDesc.iActiveBorderColor = XUI_COLOR_RGBA(47, 128, 214, 255);
	tWindowDesc.iButtonColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	tWindowDesc.iButtonHoverColor = XUI_COLOR_RGBA(218, 234, 249, 255);
	tWindowDesc.iButtonActiveColor = XUI_COLOR_RGBA(200, 222, 244, 255);
	iRet = xuiWidgetCreateTyped(pContext, pType, &pBox->pWindow, &tWindowDesc);
	if ( iRet != XUI_OK ) {
		xuiMsgBoxDestroy(pBox);
		return iRet;
	}
	iRet = xuiWindowSetClose(pBox->pWindow, __xuiMsgBoxWindowClose, pBox);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetDefaultAction(pBox->pWindow, __xuiMsgBoxDefaultAction, pBox);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCancelAction(pBox->pWindow, __xuiMsgBoxCancelAction, pBox);
	if ( iRet == XUI_OK ) iRet = xuiWidgetGetEventCallback(pBox->pWindow, &pBox->onWindowEvent, &pBox->pWindowEventUser);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventCallback(pBox->pWindow, __xuiMsgBoxWindowEvent, pBox);
	if ( iRet != XUI_OK ) {
		xuiMsgBoxDestroy(pBox);
		return iRet;
	}
	pBox->pWindow->onUpdate = __xuiMsgBoxWindowUpdate;
	pBox->pWindow->pUpdateUser = pBox;
	pClient = xuiWindowGetClientWidget(pBox->pWindow);
	if ( pClient == NULL ) {
		xuiMsgBoxDestroy(pBox);
		return XUI_ERROR_NOT_INITIALIZED;
	}
	iRet = xuiWidgetSetCacheRenderCallback(pClient, __xuiMsgBoxClientRender, pBox);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetLayoutType(pClient, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pClient, XUI_FLOW_ABSOLUTE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetPadding(pClient, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetGap(pClient, 0.0f);
	if ( iRet != XUI_OK ) {
		xuiMsgBoxDestroy(pBox);
		return iRet;
	}
	iRet = xuiWidgetCreate(pContext, &pBox->pContent);
	if ( iRet != XUI_OK ) {
		xuiMsgBoxDestroy(pBox);
		return iRet;
	}
	iRet = xuiWidgetSetHitTestVisible(pBox->pContent, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCacheRenderCallback(pBox->pContent, __xuiMsgBoxContentRender, pBox);
	if ( iRet == XUI_OK ) iRet = xuiWindowAddChild(pBox->pWindow, pBox->pContent);
	if ( iRet != XUI_OK ) {
		xuiMsgBoxDestroy(pBox);
		return iRet;
	}
	memset(&tButtonDesc, 0, sizeof(tButtonDesc));
	tButtonDesc.iSize = sizeof(tButtonDesc);
	tButtonDesc.pFont = pBox->pFont;
	tButtonDesc.fBorderWidth = 1.0f;
	tButtonDesc.iBorderColor = XUI_COLOR_RGBA(166, 196, 224, 255);
	for ( i = 0; i < XUI_MSGBOX_BUTTON_CAPACITY; i++ ) {
		tButtonDesc.sText = "";
		iRet = xuiButtonCreate(pContext, &pBox->arrButtons[i], &tButtonDesc);
		if ( iRet != XUI_OK ) {
			xuiMsgBoxDestroy(pBox);
			return iRet;
		}
		iRet = xuiButtonSetClick(pBox->arrButtons[i], __xuiMsgBoxButtonClick, pBox);
		if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pBox->arrButtons[i], 0);
		if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pBox->arrButtons[i], 0);
		if ( iRet == XUI_OK ) iRet = xuiWindowAddChild(pBox->pWindow, pBox->arrButtons[i]);
		if ( iRet != XUI_OK ) {
			xuiMsgBoxDestroy(pBox);
			return iRet;
		}
	}
	{
		xui_widget_type pBackdropType = xuiWidgetFindType(pContext, "msgbox-backdrop");
		if ( pBackdropType == NULL ) {
			xui_widget_type_desc_t tTypeDesc;
			memset(&tTypeDesc, 0, sizeof(tTypeDesc));
			tTypeDesc.iSize = sizeof(tTypeDesc);
			tTypeDesc.sName = "msgbox-backdrop";
			tTypeDesc.pParent = xuiWidgetGetBaseType();
			iRet = xuiWidgetRegisterType(pContext, &pBackdropType, &tTypeDesc);
			if ( iRet == XUI_OK ) pBackdropType->onPreparePaint = __xuiMsgBoxBackdropPreparePaint;
		}
		iRet = pBackdropType != NULL ? xuiWidgetCreateTyped(pContext, pBackdropType, &pBox->pBackdrop, NULL) : XUI_ERROR_NOT_INITIALIZED;
	}
	if ( iRet != XUI_OK ) {
		xuiMsgBoxDestroy(pBox);
		return iRet;
	}
	iRet = xuiWidgetSetVisible(pBox->pBackdrop, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pBox->pBackdrop, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pBox->pBackdrop, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pBox->pBackdrop, XUI_FLOW_ABSOLUTE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCacheRenderCallback(pBox->pBackdrop, __xuiMsgBoxBackdropRender, pBox);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pBox->pBackdrop, XUI_EVENT_POINTER_ENTER, __xuiMsgBoxBackdropEvent, pBox);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pBox->pBackdrop, XUI_EVENT_POINTER_LEAVE, __xuiMsgBoxBackdropEvent, pBox);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pBox->pBackdrop, XUI_EVENT_POINTER_MOVE, __xuiMsgBoxBackdropEvent, pBox);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pBox->pBackdrop, XUI_EVENT_POINTER_DOWN, __xuiMsgBoxBackdropEvent, pBox);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pBox->pBackdrop, XUI_EVENT_POINTER_UP, __xuiMsgBoxBackdropEvent, pBox);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pBox->pBackdrop, XUI_EVENT_POINTER_CLICK, __xuiMsgBoxBackdropEvent, pBox);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pBox->pBackdrop, XUI_EVENT_POINTER_WHEEL, __xuiMsgBoxBackdropEvent, pBox);
	if ( iRet != XUI_OK ) {
		xuiMsgBoxDestroy(pBox);
		return iRet;
	}
	iRet = (pDesc != NULL && (pDesc->iButtons == XUI_MSGBOX_BUTTON_CUSTOM || pDesc->iCustomButtonCount > 0)) ?
		xuiMsgBoxSetCustomButtons(pBox, pDesc->pCustomButtons, pDesc->iCustomButtonCount) :
		__xuiMsgBoxApplyPresetButtons(pBox, pBox->iButtons);
	if ( iRet != XUI_OK ) {
		xuiMsgBoxDestroy(pBox);
		return iRet;
	}
	iRet = __xuiMsgBoxApplyButtonVisual(pBox);
	if ( iRet != XUI_OK ) {
		xuiMsgBoxDestroy(pBox);
		return iRet;
	}
	iRet = __xuiMsgBoxLayout(pBox);
	if ( iRet != XUI_OK ) {
		xuiMsgBoxDestroy(pBox);
		return iRet;
	}
	*ppBox = pBox;
	return XUI_OK;
}

XUI_API void xuiMsgBoxDestroy(xui_msgbox pBox)
{
	int i;

	if ( !__xuiMsgBoxValid(pBox) ) {
		return;
	}
	(void)xuiMsgBoxSetOpen(pBox, 0);
	if ( pBox->pContent != NULL && xuiWidgetGetParent(pBox->pContent) == NULL ) {
		xuiWidgetDestroy(pBox->pContent);
		pBox->pContent = NULL;
	}
	for ( i = 0; i < XUI_MSGBOX_BUTTON_CAPACITY; i++ ) {
		if ( pBox->arrButtons[i] != NULL && xuiWidgetGetParent(pBox->arrButtons[i]) == NULL ) {
			xuiWidgetDestroy(pBox->arrButtons[i]);
			pBox->arrButtons[i] = NULL;
		}
	}
	if ( pBox->pBackdrop != NULL ) {
		xuiWidgetDestroy(pBox->pBackdrop);
		pBox->pBackdrop = NULL;
	}
	if ( pBox->pWindow != NULL ) {
		xuiWidgetDestroy(pBox->pWindow);
		pBox->pWindow = NULL;
		pBox->pContent = NULL;
		for ( i = 0; i < XUI_MSGBOX_BUTTON_CAPACITY; i++ ) pBox->arrButtons[i] = NULL;
	}
	for ( i = 0; i < XUI_MSGBOX_BUTTON_CAPACITY; i++ ) {
		if ( pBox->arrButtonText[i] != NULL ) {
			xrtFree(pBox->arrButtonText[i]);
			pBox->arrButtonText[i] = NULL;
		}
	}
	for ( i = 0; i < XUI_MSGBOX_PRESET_BUTTON_TITLE_COUNT; i++ ) {
		if ( pBox->arrPresetButtonTitle[i] != NULL ) {
			xrtFree(pBox->arrPresetButtonTitle[i]);
			pBox->arrPresetButtonTitle[i] = NULL;
		}
	}
	if ( pBox->sTitle != NULL ) xrtFree(pBox->sTitle);
	if ( pBox->sMessage != NULL ) xrtFree(pBox->sMessage);
	pBox->iMagic = 0;
	xrtFree(pBox);
}

XUI_API int xuiMsgBoxSetText(xui_msgbox pBox, const char* sTitle, const char* sMessage)
{
	char* sNewTitle;
	char* sNewMessage;
	char* sOldTitle;
	char* sOldMessage;
	int iRet;

	if ( !__xuiMsgBoxValid(pBox) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( __xuiMsgBoxTextSame(pBox->sTitle, sTitle) && __xuiMsgBoxTextSame(pBox->sMessage, sMessage) ) {
		return XUI_OK;
	}
	sNewTitle = __xuiMsgBoxDupText(sTitle);
	if ( sNewTitle == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	sNewMessage = __xuiMsgBoxDupText(sMessage);
	if ( sNewMessage == NULL ) {
		xrtFree(sNewTitle);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	sOldTitle = pBox->sTitle;
	sOldMessage = pBox->sMessage;
	pBox->sTitle = sNewTitle;
	pBox->sMessage = sNewMessage;
	iRet = xuiWindowSetTitle(pBox->pWindow, pBox->sTitle);
	if ( iRet == XUI_OK ) iRet = __xuiMsgBoxLayout(pBox);
	if ( iRet != XUI_OK ) {
		pBox->sTitle = sOldTitle;
		pBox->sMessage = sOldMessage;
		(void)xuiWindowSetTitle(pBox->pWindow, pBox->sTitle);
		(void)__xuiMsgBoxLayout(pBox);
		xrtFree(sNewTitle);
		xrtFree(sNewMessage);
		return iRet;
	}
	if ( sOldTitle != NULL ) xrtFree(sOldTitle);
	if ( sOldMessage != NULL ) xrtFree(sOldMessage);
	if ( pBox->iChangeCount < INT_MAX ) pBox->iChangeCount++;
	return XUI_OK;
}

XUI_API int xuiMsgBoxSetTitle(xui_msgbox pBox, const char* sTitle)
{
	char* sNewTitle;
	int iRet;

	if ( !__xuiMsgBoxValid(pBox) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( __xuiMsgBoxTextSame(pBox->sTitle, sTitle) ) return XUI_OK;
	sNewTitle = __xuiMsgBoxDupText(sTitle);
	if ( sNewTitle == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	iRet = xuiWindowSetTitle(pBox->pWindow, sNewTitle);
	if ( iRet != XUI_OK ) {
		xrtFree(sNewTitle);
		return iRet;
	}
	if ( pBox->sTitle != NULL ) xrtFree(pBox->sTitle);
	pBox->sTitle = sNewTitle;
	if ( pBox->iChangeCount < INT_MAX ) pBox->iChangeCount++;
	return XUI_OK;
}

XUI_API const char* xuiMsgBoxGetTitle(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->sTitle : NULL;
}

XUI_API int xuiMsgBoxSetMessage(xui_msgbox pBox, const char* sMessage)
{
	char* sNewMessage;
	char* sOldMessage;
	int iRet;

	if ( !__xuiMsgBoxValid(pBox) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( __xuiMsgBoxTextSame(pBox->sMessage, sMessage) ) return XUI_OK;
	sNewMessage = __xuiMsgBoxDupText(sMessage);
	if ( sNewMessage == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	sOldMessage = pBox->sMessage;
	pBox->sMessage = sNewMessage;
	iRet = __xuiMsgBoxLayout(pBox);
	if ( iRet != XUI_OK ) {
		pBox->sMessage = sOldMessage;
		(void)__xuiMsgBoxLayout(pBox);
		xrtFree(sNewMessage);
		return iRet;
	}
	if ( sOldMessage != NULL ) xrtFree(sOldMessage);
	if ( pBox->iChangeCount < INT_MAX ) pBox->iChangeCount++;
	return XUI_OK;
}

XUI_API const char* xuiMsgBoxGetMessage(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->sMessage : NULL;
}

XUI_API int xuiMsgBoxSetType(xui_msgbox pBox, int iType)
{
	int iOldType;
	int bOldBuiltinIcon;
	int bOldCustomIcon;
	xui_surface pOldIconSurface;
	uint32_t iOldIconColor;
	int iRet;

	if ( !__xuiMsgBoxValid(pBox) || !__xuiMsgBoxTypeValid(iType) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pBox->iType == iType && pBox->bBuiltinIcon && !pBox->bCustomIcon ) return XUI_OK;
	iOldType = pBox->iType;
	bOldBuiltinIcon = pBox->bBuiltinIcon;
	bOldCustomIcon = pBox->bCustomIcon;
	pOldIconSurface = pBox->pIconSurface;
	iOldIconColor = pBox->tColors.iIconColor;
	pBox->iType = iType;
	pBox->bBuiltinIcon = 1;
	pBox->bCustomIcon = 0;
	pBox->pIconSurface = NULL;
	if ( !pBox->bHasColors ) pBox->tColors.iIconColor = __xuiMsgBoxIconColor(iType);
	iRet = __xuiMsgBoxLayout(pBox);
	if ( iRet != XUI_OK ) {
		pBox->iType = iOldType;
		pBox->bBuiltinIcon = bOldBuiltinIcon;
		pBox->bCustomIcon = bOldCustomIcon;
		pBox->pIconSurface = pOldIconSurface;
		pBox->tColors.iIconColor = iOldIconColor;
		(void)__xuiMsgBoxLayout(pBox);
		return iRet;
	}
	if ( pBox->iChangeCount < INT_MAX ) pBox->iChangeCount++;
	return XUI_OK;
}

XUI_API int xuiMsgBoxGetType(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->iType : XUI_MSGBOX_ICON_NONE;
}

XUI_API int xuiMsgBoxSetIconSurface(xui_msgbox pBox, xui_surface pSurface, xui_rect_t tSrc)
{
	xui_surface pOldSurface;
	xui_rect_t tOldSrc;
	int iOldType;
	int bOldCustom;
	int bOldBuiltin;
	int iRet;

	if ( !__xuiMsgBoxValid(pBox) || !__xuiMsgBoxRectValid(tSrc) ||
	     (pSurface != NULL && (tSrc.fW <= 0 || tSrc.fH <= 0)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pBox->pIconSurface == pSurface && memcmp(&pBox->tIconSrc, &tSrc, sizeof(tSrc)) == 0 &&
	     pBox->bCustomIcon == (pSurface != NULL) ) return XUI_OK;
	pOldSurface = pBox->pIconSurface;
	tOldSrc = pBox->tIconSrc;
	iOldType = pBox->iType;
	bOldCustom = pBox->bCustomIcon;
	bOldBuiltin = pBox->bBuiltinIcon;
	pBox->pIconSurface = pSurface;
	pBox->tIconSrc = tSrc;
	pBox->bCustomIcon = (pSurface != NULL);
	pBox->bBuiltinIcon = (pSurface == NULL);
	if ( pSurface == NULL ) {
		pBox->iType = XUI_MSGBOX_ICON_NONE;
	}
	iRet = __xuiMsgBoxLayout(pBox);
	if ( iRet != XUI_OK ) {
		pBox->pIconSurface = pOldSurface;
		pBox->tIconSrc = tOldSrc;
		pBox->iType = iOldType;
		pBox->bCustomIcon = bOldCustom;
		pBox->bBuiltinIcon = bOldBuiltin;
		(void)__xuiMsgBoxLayout(pBox);
		return iRet;
	}
	if ( pBox->iChangeCount < INT_MAX ) pBox->iChangeCount++;
	return XUI_OK;
}

XUI_API int xuiMsgBoxUseBuiltinIcon(xui_msgbox pBox)
{
	int iRet;
	xui_surface pOldSurface;
	int bOldCustom;
	int bOldBuiltin;

	if ( !__xuiMsgBoxValid(pBox) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pBox->bBuiltinIcon && !pBox->bCustomIcon && pBox->pIconSurface == NULL ) return XUI_OK;
	pOldSurface = pBox->pIconSurface;
	bOldCustom = pBox->bCustomIcon;
	bOldBuiltin = pBox->bBuiltinIcon;
	pBox->pIconSurface = NULL;
	pBox->bCustomIcon = 0;
	pBox->bBuiltinIcon = 1;
	iRet = __xuiMsgBoxLayout(pBox);
	if ( iRet != XUI_OK ) {
		pBox->pIconSurface = pOldSurface;
		pBox->bCustomIcon = bOldCustom;
		pBox->bBuiltinIcon = bOldBuiltin;
		(void)__xuiMsgBoxLayout(pBox);
		return iRet;
	}
	if ( pBox->iChangeCount < INT_MAX ) pBox->iChangeCount++;
	return XUI_OK;
}

XUI_API int xuiMsgBoxSetButtons(xui_msgbox pBox, int iButtons)
{
	int iRet;

	if ( !__xuiMsgBoxValid(pBox) || !__xuiMsgBoxButtonsValid(iButtons) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pBox->iButtons == iButtons && iButtons != XUI_MSGBOX_BUTTON_CUSTOM && pBox->iButtonCount > 0 ) {
		return XUI_OK;
	}
	iRet = __xuiMsgBoxApplyPresetButtons(pBox, iButtons);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiMsgBoxApplyButtonVisual(pBox);
	if ( iRet != XUI_OK ) return iRet;
	if ( pBox->iChangeCount < INT_MAX ) pBox->iChangeCount++;
	return __xuiMsgBoxLayout(pBox);
}

XUI_API int xuiMsgBoxSetCustomButtons(xui_msgbox pBox, const xui_msgbox_button_t* pButtons, int iCount)
{
	int iRet;

	if ( !__xuiMsgBoxValid(pBox) ||
	     !__xuiMsgBoxButtonModelValid(pButtons, iCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( __xuiMsgBoxButtonModelSame(pBox, XUI_MSGBOX_BUTTON_CUSTOM, pButtons, iCount) ) return XUI_OK;
	iRet = __xuiMsgBoxApplyButtonModel(pBox, XUI_MSGBOX_BUTTON_CUSTOM, pButtons, iCount);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiMsgBoxApplyButtonVisual(pBox);
	if ( iRet != XUI_OK ) return iRet;
	if ( pBox->iChangeCount < INT_MAX ) pBox->iChangeCount++;
	return __xuiMsgBoxLayout(pBox);
}

XUI_API int xuiMsgBoxSetPresetButtonTitle(xui_msgbox pBox, int iTitle, const char* sText)
{
	char* sNewText;
	char* sOldText;
	int iIndex;
	int iRet;

	if ( !__xuiMsgBoxValid(pBox) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xuiMsgBoxPresetButtonTitleIndex(iTitle);
	if ( iIndex < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (sText == NULL) || (sText[0] == 0) ) sText = NULL;
	if ( (sText == NULL && pBox->arrPresetButtonTitle[iIndex] == NULL) ||
	     (sText != NULL && pBox->arrPresetButtonTitle[iIndex] != NULL &&
	      strcmp(sText, pBox->arrPresetButtonTitle[iIndex]) == 0) ) return XUI_OK;
	sNewText = (sText != NULL) ? __xuiMsgBoxDupText(sText) : NULL;
	if ( sText != NULL && sNewText == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	sOldText = pBox->arrPresetButtonTitle[iIndex];
	pBox->arrPresetButtonTitle[iIndex] = sNewText;
	if ( pBox->iButtons != XUI_MSGBOX_BUTTON_CUSTOM ) {
		iRet = __xuiMsgBoxApplyPresetButtons(pBox, pBox->iButtons);
		if ( iRet == XUI_OK ) iRet = __xuiMsgBoxApplyButtonVisual(pBox);
		if ( iRet == XUI_OK ) iRet = __xuiMsgBoxLayout(pBox);
		if ( iRet != XUI_OK ) {
			pBox->arrPresetButtonTitle[iIndex] = sOldText;
			(void)__xuiMsgBoxApplyPresetButtons(pBox, pBox->iButtons);
			(void)__xuiMsgBoxApplyButtonVisual(pBox);
			(void)__xuiMsgBoxLayout(pBox);
			if ( sNewText != NULL ) xrtFree(sNewText);
			return iRet;
		}
		if ( sOldText != NULL ) xrtFree(sOldText);
		if ( pBox->iChangeCount < INT_MAX ) pBox->iChangeCount++;
		return XUI_OK;
	}
	if ( sOldText != NULL ) xrtFree(sOldText);
	if ( pBox->iChangeCount < INT_MAX ) pBox->iChangeCount++;
	return XUI_OK;
}

XUI_API const char* xuiMsgBoxGetPresetButtonTitle(xui_msgbox pBox, int iTitle)
{
	if ( !__xuiMsgBoxValid(pBox) || (__xuiMsgBoxPresetButtonTitleIndex(iTitle) < 0) ) {
		return NULL;
	}
	return __xuiMsgBoxPresetButtonTitle(pBox, iTitle);
}

XUI_API int xuiMsgBoxSetResult(xui_msgbox pBox, xui_msgbox_result_proc onResult, void* pUser)
{
	if ( !__xuiMsgBoxValid(pBox) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pBox->onResult = onResult;
	pBox->pResultUser = pUser;
	return XUI_OK;
}

XUI_API int xuiMsgBoxSetModal(xui_msgbox pBox, int bModal)
{
	int bOldModal;
	int iRet;

	if ( !__xuiMsgBoxValid(pBox) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	bModal = bModal ? 1 : 0;
	if ( pBox->bModal == bModal ) return XUI_OK;
	bOldModal = pBox->bModal;
	pBox->bModal = bModal;
	iRet = __xuiMsgBoxApplyOverlay(pBox);
	if ( iRet != XUI_OK ) {
		pBox->bModal = bOldModal;
		(void)__xuiMsgBoxApplyOverlay(pBox);
		return iRet;
	}
	if ( pBox->iChangeCount < INT_MAX ) pBox->iChangeCount++;
	return XUI_OK;
}

XUI_API int xuiMsgBoxIsModal(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->bModal : 0;
}

XUI_API int xuiMsgBoxSetOpen(xui_msgbox pBox, int bOpen)
{
	xui_widget pFocus;
	int iRet;

	if ( !__xuiMsgBoxValid(pBox) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	bOpen = bOpen ? 1 : 0;
	if ( pBox->bOpen == bOpen ) {
		if ( bOpen ) {
			iRet = __xuiMsgBoxLayout(pBox);
			if ( iRet == XUI_OK ) iRet = xuiWindowSetOpen(pBox->pWindow, 1);
			if ( iRet == XUI_OK ) iRet = __xuiMsgBoxApplyOverlay(pBox);
			return iRet;
		}
		return XUI_OK;
	}
	if ( bOpen ) {
		pBox->iResult = XUI_MSGBOX_RESULT_CLOSE;
		iRet = __xuiMsgBoxLayout(pBox);
		if ( iRet != XUI_OK ) return iRet;
		pBox->bOpen = 1;
		/* Establish the modal layer before Window activation. Otherwise a
		 * nested message box is rejected by the currently active modal root. */
		iRet = __xuiMsgBoxApplyOverlay(pBox);
		if ( iRet == XUI_OK ) iRet = xuiWindowSetOpen(pBox->pWindow, 1);
		if ( iRet != XUI_OK ) {
			pBox->bOpen = 0;
			(void)__xuiMsgBoxApplyOverlay(pBox);
			(void)xuiWindowSetOpen(pBox->pWindow, 0);
			return iRet;
		}
		pFocus = (pBox->iButtonCount > 0) ? pBox->arrButtons[0] : pBox->pWindow;
		(void)xuiSetFocusWidget(pBox->pContext, pFocus);
	} else {
		iRet = xuiWindowSetOpen(pBox->pWindow, 0);
		if ( iRet != XUI_OK ) return iRet;
		pBox->bOpen = 0;
		iRet = __xuiMsgBoxApplyOverlay(pBox);
		if ( iRet != XUI_OK ) {
			pBox->bOpen = 1;
			(void)xuiWindowSetOpen(pBox->pWindow, 1);
			(void)__xuiMsgBoxApplyOverlay(pBox);
			return iRet;
		}
	}
	if ( pBox->iChangeCount < INT_MAX ) pBox->iChangeCount++;
	return XUI_OK;
}

XUI_API int xuiMsgBoxIsOpen(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->bOpen : 0;
}

XUI_API int xuiMsgBoxGetResult(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->iResult : XUI_MSGBOX_RESULT_CLOSE;
}

XUI_API int xuiMsgBoxSetMetrics(xui_msgbox pBox, const xui_msgbox_metrics_t* pMetrics)
{
	xui_msgbox_metrics_t tOldMetrics;
	xui_msgbox_metrics_t tNewMetrics;
	int iRet;

	if ( !__xuiMsgBoxValid(pBox) || !__xuiMsgBoxMetricsValid(pMetrics) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tNewMetrics = *pMetrics;
	tNewMetrics.iSize = sizeof(tNewMetrics);
	if ( memcmp(&pBox->tMetrics, &tNewMetrics, sizeof(tNewMetrics)) == 0 ) return XUI_OK;
	tOldMetrics = pBox->tMetrics;
	pBox->tMetrics = tNewMetrics;
	iRet = __xuiMsgBoxLayout(pBox);
	if ( iRet != XUI_OK ) {
		pBox->tMetrics = tOldMetrics;
		(void)__xuiMsgBoxLayout(pBox);
		return iRet;
	}
	if ( pBox->iChangeCount < INT_MAX ) pBox->iChangeCount++;
	return XUI_OK;
}

XUI_API int xuiMsgBoxGetMetrics(xui_msgbox pBox, xui_msgbox_metrics_t* pMetrics)
{
	if ( !__xuiMsgBoxValid(pBox) || (pMetrics == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pMetrics = pBox->tMetrics;
	pMetrics->iSize = sizeof(*pMetrics);
	return XUI_OK;
}

XUI_API int xuiMsgBoxSetColors(xui_msgbox pBox, const xui_msgbox_colors_t* pColors)
{
	xui_msgbox_colors_t tOldColors;
	xui_msgbox_colors_t tNewColors;
	int bOldHasColors;
	uint32_t iBackground, iTitle, iText, iBorder, iButton, iHover, iActive;
	uint32_t iInactiveTitle, iInactiveText, iActiveBorder, iCloseHover, iCloseActive;
	int iRet;
	if ( !__xuiMsgBoxValid(pBox) || !__xuiMsgBoxColorsValid(pColors) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tNewColors = *pColors;
	tNewColors.iSize = sizeof(tNewColors);
	if ( pBox->bHasColors && memcmp(&pBox->tColors, &tNewColors, sizeof(tNewColors)) == 0 ) return XUI_OK;
	tOldColors = pBox->tColors;
	bOldHasColors = pBox->bHasColors;
	pBox->tColors = tNewColors;
	pBox->bHasColors = 1;
	iRet = xuiWindowGetColors(pBox->pWindow, &iBackground, NULL, &iTitle, &iText, &iBorder, &iButton, &iHover, &iActive);
	if ( iRet != XUI_OK ) {
		pBox->tColors = tOldColors;
		pBox->bHasColors = bOldHasColors;
		return iRet;
	}
	iRet = xuiWindowGetStateColors(pBox->pWindow, &iInactiveTitle, &iInactiveText, &iActiveBorder, &iCloseHover, &iCloseActive);
	if ( iRet != XUI_OK ) {
		pBox->tColors = tOldColors;
		pBox->bHasColors = bOldHasColors;
		return iRet;
	}
	if ( iRet == XUI_OK ) iRet = xuiWindowSetColors(pBox->pWindow,
		iBackground, pBox->tColors.iClientColor, iTitle, iText, iBorder, iButton, iHover, iActive);
	if ( iRet == XUI_OK ) iRet = xuiWindowSetStateColors(pBox->pWindow, iInactiveTitle, iInactiveText, iActiveBorder, iCloseHover, iCloseActive);
	if ( iRet == XUI_OK ) iRet = __xuiMsgBoxApplyButtonVisual(pBox);
	if ( iRet != XUI_OK ) {
		pBox->tColors = tOldColors;
		pBox->bHasColors = bOldHasColors;
		(void)xuiWindowSetColors(pBox->pWindow,
			iBackground, pBox->tColors.iClientColor, iTitle, iText, iBorder, iButton, iHover, iActive);
		(void)xuiWindowSetStateColors(pBox->pWindow, iInactiveTitle, iInactiveText, iActiveBorder, iCloseHover, iCloseActive);
		(void)__xuiMsgBoxApplyButtonVisual(pBox);
		return iRet;
	}
	(void)__xuiMsgBoxSyncPaint(pBox);
	if ( pBox->iChangeCount < INT_MAX ) pBox->iChangeCount++;
	return XUI_OK;
}

XUI_API int xuiMsgBoxGetColors(xui_msgbox pBox, xui_msgbox_colors_t* pColors)
{
	if ( !__xuiMsgBoxValid(pBox) || (pColors == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pColors = pBox->tColors;
	pColors->iSize = sizeof(*pColors);
	return XUI_OK;
}

XUI_API xui_widget xuiMsgBoxGetWindowWidget(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->pWindow : NULL;
}

XUI_API xui_widget xuiMsgBoxGetContentWidget(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->pContent : NULL;
}

XUI_API xui_widget xuiMsgBoxGetBackdropWidget(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->pBackdrop : NULL;
}

XUI_API xui_widget xuiMsgBoxGetButtonWidget(xui_msgbox pBox, int iIndex)
{
	if ( !__xuiMsgBoxValid(pBox) || (iIndex < 0) || (iIndex >= pBox->iButtonCount) ) {
		return NULL;
	}
	return pBox->arrButtons[iIndex];
}

XUI_API int xuiMsgBoxGetButtonCount(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->iButtonCount : 0;
}

XUI_API xui_rect_t xuiMsgBoxGetIconRect(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->tIconRect : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

XUI_API xui_rect_t xuiMsgBoxGetMessageRect(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->tMessageRect : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

XUI_API xui_rect_t xuiMsgBoxGetButtonRect(xui_msgbox pBox, int iIndex)
{
	if ( !__xuiMsgBoxValid(pBox) || (iIndex < 0) || (iIndex >= pBox->iButtonCount) ) {
		return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	}
	return pBox->arrButtonRect[iIndex];
}

XUI_API xui_rect_t xuiMsgBoxGetBackdropRect(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->tBackdropRect : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

XUI_API int xuiMsgBoxGetWrapLineCount(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->iWrapLineCount : 0;
}

XUI_API int xuiMsgBoxGetResultCount(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->iResultCount : 0;
}

XUI_API int xuiMsgBoxGetChangeCount(xui_msgbox pBox)
{
	return __xuiMsgBoxValid(pBox) ? pBox->iChangeCount : 0;
}

#endif
