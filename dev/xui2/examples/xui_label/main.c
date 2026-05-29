#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 960
#define DEMO_TARGET_H 680
#define LABEL_AREA_COUNT 4
#define LABEL_PER_AREA 9
#define LABEL_COUNT (LABEL_AREA_COUNT * LABEL_PER_AREA)

typedef struct xui_label_box_t {
	xui_proxy_t* pProxy;
	uint32_t iFill;
	uint32_t iStroke;
	float fRadius;
	float fStrokeWidth;
} xui_label_box_t;

typedef struct xui_label_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_font pSmallFont;
	xui_font pLargeFont;
	xui_widget pRoot;
	xui_widget pArea[LABEL_AREA_COUNT];
	xui_widget pCell[LABEL_COUNT];
	xui_widget pLabel[LABEL_COUNT];
	xui_label_box_t tAreaBox[LABEL_AREA_COUNT];
	xui_label_box_t tCellBox[LABEL_COUNT];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	int iLastSkinPhase;
} xui_label_demo_t;

static void __xuiLabelUsage(void)
{
	printf("usage: xui_label [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiLabelParseArgs(xui_label_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			pDemo->fMaxSeconds = atof(argv[++i]);
			if ( pDemo->fMaxSeconds <= 0.0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
			if ( pDemo->fMaxSeconds <= 0.0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			__xuiLabelUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiLabelFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\simhei.ttf"
	};
	FILE* pFile;
	int i;

	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPaths[i];
		}
	}
	return NULL;
}

static xui_style_value_t __xuiLabelStyleColor(uint32_t iColor)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_COLOR;
	tValue.iColor = iColor;
	return tValue;
}

static xui_style_value_t __xuiLabelStyleString(const char* sText)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_STRING;
	tValue.sText = sText;
	return tValue;
}

static xui_style_value_t __xuiLabelStyleToken(const char* sText)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_TOKEN;
	tValue.sText = sText;
	return tValue;
}

static xui_style_property_t __xuiLabelStyleProp(const char* sName, xui_style_value_t tValue)
{
	xui_style_property_t tProp;

	memset(&tProp, 0, sizeof(tProp));
	tProp.iSize = sizeof(tProp);
	tProp.sName = sName;
	tProp.tValue = tValue;
	return tProp;
}

static xui_style_desc_t __xuiLabelStyleDesc(const xui_style_property_t* pProps, int iCount)
{
	xui_style_desc_t tStyle;

	memset(&tStyle, 0, sizeof(tStyle));
	tStyle.iSize = sizeof(tStyle);
	tStyle.pProperties = pProps;
	tStyle.iPropertyCount = iCount;
	return tStyle;
}

static uint32_t __xuiLabelAlignFlag(int iIndex)
{
	static const uint32_t arrFlags[LABEL_PER_AREA] = {
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP,
		XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_TOP,
		XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_TOP,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE,
		XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE,
		XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_BOTTOM,
		XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_BOTTOM,
		XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_BOTTOM
	};

	return arrFlags[iIndex % LABEL_PER_AREA] | XUI_TEXT_CLIP;
}

static const char* __xuiLabelAlignText(int iIndex)
{
	static const char* arrText[LABEL_PER_AREA] = {
		"Left Top",
		"Center Top",
		"Right Top",
		"Left Middle",
		"Center Middle",
		"Right Middle",
		"Left Bottom",
		"Center Bottom",
		"Right Bottom"
	};

	return arrText[iIndex % LABEL_PER_AREA];
}

static int __xuiLabelBoxRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_label_box_t* pBox;
	xui_rect_t tRect;
	int iRet;

	(void)iStateId;
	pBox = (xui_label_box_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pBox == NULL) || (pBox->pProxy == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	iRet = pBox->pProxy->drawRoundRectFill(pBox->pProxy, pDraw, tRect, pBox->fRadius, pBox->iFill);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( pBox->fStrokeWidth > 0.0f ) {
		iRet = pBox->pProxy->drawRoundRectStroke(pBox->pProxy, pDraw, tRect, pBox->fRadius, pBox->fStrokeWidth, pBox->iStroke);
	}
	return iRet;
}

static void __xuiLabelThickness(float fLeft, float fTop, float fRight, float fBottom, xui_thickness_t* pOut)
{
	pOut->fLeft = fLeft;
	pOut->fTop = fTop;
	pOut->fRight = fRight;
	pOut->fBottom = fBottom;
}

static int __xuiLabelSetFillLayout(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	}
	return iRet;
}

static int __xuiLabelCreateStyles(xui_label_demo_t* pDemo)
{
	xui_style_property_t arrProps[2];
	xui_style_desc_t tStyle;
	xui_style_value_t tValue;
	int iRet;

	(void)xuiLabelGetType(pDemo->pContext);
	arrProps[0] = __xuiLabelStyleProp("text.color", __xuiLabelStyleColor(XUI_COLOR_RGBA(36, 42, 52, 255)));
	tStyle = __xuiLabelStyleDesc(arrProps, 1);
	iRet = xuiStyleSetClass(pDemo->pContext, "body-label", &tStyle);
	if ( iRet != XUI_OK ) return iRet;

	arrProps[0] = __xuiLabelStyleProp("text.color", __xuiLabelStyleColor(XUI_COLOR_RGBA(32, 62, 96, 255)));
	tStyle = __xuiLabelStyleDesc(arrProps, 1);
	iRet = xuiStyleSetClass(pDemo->pContext, "blue-label", &tStyle);
	if ( iRet != XUI_OK ) return iRet;

	arrProps[0] = __xuiLabelStyleProp("font.name", __xuiLabelStyleString("small"));
	arrProps[1] = __xuiLabelStyleProp("text.color", __xuiLabelStyleColor(XUI_COLOR_RGBA(34, 42, 56, 255)));
	tStyle = __xuiLabelStyleDesc(arrProps, 2);
	iRet = xuiStyleSetClass(pDemo->pContext, "small-label", &tStyle);
	if ( iRet != XUI_OK ) return iRet;

	arrProps[0] = __xuiLabelStyleProp("font.name", __xuiLabelStyleString("large"));
	arrProps[1] = __xuiLabelStyleProp("text.color", __xuiLabelStyleColor(XUI_COLOR_RGBA(34, 42, 56, 255)));
	tStyle = __xuiLabelStyleDesc(arrProps, 2);
	iRet = xuiStyleSetClass(pDemo->pContext, "large-label", &tStyle);
	if ( iRet != XUI_OK ) return iRet;

	tValue = __xuiLabelStyleColor(XUI_COLOR_RGBA(20, 112, 210, 255));
	iRet = xuiStyleSetToken(pDemo->pContext, "demo.label.accent", &tValue);
	if ( iRet != XUI_OK ) return iRet;
	arrProps[0] = __xuiLabelStyleProp("text.color", __xuiLabelStyleToken("demo.label.accent"));
	tStyle = __xuiLabelStyleDesc(arrProps, 1);
	return xuiStyleSetClass(pDemo->pContext, "accent-label", &tStyle);
}

static int __xuiLabelAddArea(xui_label_demo_t* pDemo, int iArea, uint32_t iFill)
{
	xui_thickness_t tPadding;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pArea[iArea]);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pDemo->tAreaBox[iArea].pProxy = &pDemo->tProxy;
	pDemo->tAreaBox[iArea].iFill = iFill;
	pDemo->tAreaBox[iArea].iStroke = XUI_COLOR_RGBA(170, 184, 202, 255);
	pDemo->tAreaBox[iArea].fRadius = 5.0f;
	pDemo->tAreaBox[iArea].fStrokeWidth = 1.0f;
	__xuiLabelThickness(10.0f, 10.0f, 10.0f, 10.0f, &tPadding);
	(void)xuiWidgetSetLayoutType(pDemo->pArea[iArea], XUI_LAYOUT_GRID);
	(void)xuiWidgetSetGap(pDemo->pArea[iArea], 8.0f);
	(void)xuiWidgetSetGridMetrics(pDemo->pArea[iArea], 3, 0.0f, 0.0f);
	(void)xuiWidgetSetPadding(pDemo->pArea[iArea], tPadding);
	(void)__xuiLabelSetFillLayout(pDemo->pArea[iArea]);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pArea[iArea], __xuiLabelBoxRender, &pDemo->tAreaBox[iArea]);
	return xuiWidgetAddChild(pDemo->pRoot, pDemo->pArea[iArea]);
}

static int __xuiLabelAddLabel(xui_label_demo_t* pDemo, int iArea, int iSlot, const char* sText, uint32_t iFlags, const char* sClass, uint32_t iTextColor)
{
	static const uint32_t arrBg[LABEL_PER_AREA] = {
		XUI_COLOR_RGBA(234, 242, 255, 255), XUI_COLOR_RGBA(233, 248, 239, 255), XUI_COLOR_RGBA(255, 243, 216, 255),
		XUI_COLOR_RGBA(243, 234, 251, 255), XUI_COLOR_RGBA(255, 236, 236, 255), XUI_COLOR_RGBA(232, 246, 250, 255),
		XUI_COLOR_RGBA(244, 246, 250, 255), XUI_COLOR_RGBA(239, 242, 231, 255), XUI_COLOR_RGBA(248, 238, 247, 255)
	};
	xui_label_desc_t tDesc;
	xui_thickness_t tMargin;
	xui_thickness_t tPadding;
	int iIndex;
	int iRet;

	iIndex = iArea * LABEL_PER_AREA + iSlot;
	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pCell[iIndex]);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pDemo->tCellBox[iIndex].pProxy = &pDemo->tProxy;
	pDemo->tCellBox[iIndex].iFill = (iArea == 2) ? arrBg[iSlot] : XUI_COLOR_RGBA(255, 255, 255, 0);
	pDemo->tCellBox[iIndex].iStroke = (iArea == 1) ? XUI_COLOR_RGBA(74, 126, 184, 255) : XUI_COLOR_RGBA(210, 219, 230, 0);
	pDemo->tCellBox[iIndex].fRadius = 4.0f;
	pDemo->tCellBox[iIndex].fStrokeWidth = (iArea == 1 || iArea == 2) ? 1.0f : 0.0f;
	__xuiLabelThickness(4.0f, 4.0f, 4.0f, 4.0f, &tMargin);
	__xuiLabelThickness(7.0f, 6.0f, 7.0f, 6.0f, &tPadding);
	(void)xuiWidgetSetLayoutType(pDemo->pCell[iIndex], XUI_LAYOUT_OVERLAY);
	(void)xuiWidgetSetMargin(pDemo->pCell[iIndex], tMargin);
	(void)xuiWidgetSetPadding(pDemo->pCell[iIndex], tPadding);
	(void)__xuiLabelSetFillLayout(pDemo->pCell[iIndex]);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pCell[iIndex], __xuiLabelBoxRender, &pDemo->tCellBox[iIndex]);

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iTextColor;
	tDesc.iDisabledTextColor = XUI_COLOR_RGBA(130, 138, 150, 255);
	tDesc.iTextFlags = iFlags;
	tDesc.iWrapMode = XUI_TEXT_WRAP_NONE;
	iRet = xuiLabelCreate(pDemo->pContext, &pDemo->pLabel[iIndex], &tDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	(void)__xuiLabelSetFillLayout(pDemo->pLabel[iIndex]);
	if ( sClass != NULL ) {
		(void)xuiWidgetAddStyleClass(pDemo->pLabel[iIndex], sClass);
	}
	iRet = xuiWidgetAddChild(pDemo->pCell[iIndex], pDemo->pLabel[iIndex]);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	return xuiWidgetAddChild(pDemo->pArea[iArea], pDemo->pCell[iIndex]);
}

static int __xuiLabelCreateUi(xui_label_demo_t* pDemo)
{
	xui_thickness_t tPadding;
	int i;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_GRID);
	(void)xuiWidgetSetGap(pDemo->pRoot, 12.0f);
	(void)xuiWidgetSetGridMetrics(pDemo->pRoot, 2, 0.0f, 0.0f);
	__xuiLabelThickness(18.0f, 18.0f, 18.0f, 18.0f, &tPadding);
	(void)xuiWidgetSetPadding(pDemo->pRoot, tPadding);
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});

	iRet = __xuiLabelCreateStyles(pDemo);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	for ( i = 0; i < LABEL_AREA_COUNT; i++ ) {
		iRet = __xuiLabelAddArea(pDemo, i, (i == 2) ? XUI_COLOR_RGBA(231, 238, 247, 255) : XUI_COLOR_RGBA(248, 250, 253, 255));
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	for ( i = 0; i < LABEL_PER_AREA; i++ ) {
		iRet = __xuiLabelAddLabel(pDemo, 0, i, __xuiLabelAlignText(i), __xuiLabelAlignFlag(i), "body-label", XUI_COLOR_RGBA(36, 42, 52, 255));
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( i = 0; i < LABEL_PER_AREA; i++ ) {
		iRet = __xuiLabelAddLabel(pDemo, 1, i, __xuiLabelAlignText(i), __xuiLabelAlignFlag(i), "blue-label", XUI_COLOR_RGBA(32, 62, 96, 255));
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( i = 0; i < LABEL_PER_AREA; i++ ) {
		iRet = __xuiLabelAddLabel(pDemo, 2, i, __xuiLabelAlignText(i), __xuiLabelAlignFlag(i), "body-label", XUI_COLOR_RGBA(34, 46, 62, 255));
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiLabelAddLabel(pDemo, 3, 0, "Small font", XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP, "small-label", XUI_COLOR_RGBA(34, 42, 56, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiLabelAddLabel(pDemo, 3, 1, "Large font", XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP, "large-label", XUI_COLOR_RGBA(34, 42, 56, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiLabelAddLabel(pDemo, 3, 2, "Disabled", XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP, "body-label", XUI_COLOR_RGBA(34, 42, 56, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiLabelAddLabel(pDemo, 3, 3, "Underline", XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP, "accent-label", XUI_COLOR_RGBA(26, 96, 176, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiLabelAddLabel(pDemo, 3, 4, "Border + fill", XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP, "body-label", XUI_COLOR_RGBA(38, 58, 82, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiLabelAddLabel(pDemo, 3, 5, "Cache rendered", XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP, "body-label", XUI_COLOR_RGBA(96, 56, 18, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiLabelAddLabel(pDemo, 3, 6, "Two lines\ncentered", XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP, "body-label", XUI_COLOR_RGBA(36, 42, 52, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiLabelAddLabel(pDemo, 3, 7, "Padded text", XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_BOTTOM | XUI_TEXT_CLIP, "body-label", XUI_COLOR_RGBA(36, 42, 52, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiLabelAddLabel(pDemo, 3, 8, "Dynamic skin", XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP, "accent-label", XUI_COLOR_RGBA(20, 112, 210, 255));
	if ( iRet != XUI_OK ) return iRet;

	(void)xuiWidgetSetEnabled(pDemo->pLabel[(3 * LABEL_PER_AREA) + 2], 0);
	(void)xuiLabelSetUnderline(pDemo->pLabel[(3 * LABEL_PER_AREA) + 3], 1);
	pDemo->tCellBox[(3 * LABEL_PER_AREA) + 4].iFill = XUI_COLOR_RGBA(229, 240, 255, 255);
	pDemo->tCellBox[(3 * LABEL_PER_AREA) + 4].iStroke = XUI_COLOR_RGBA(42, 120, 210, 255);
	pDemo->tCellBox[(3 * LABEL_PER_AREA) + 4].fStrokeWidth = 1.0f;
	pDemo->tCellBox[(3 * LABEL_PER_AREA) + 7].iFill = XUI_COLOR_RGBA(248, 244, 236, 255);
	__xuiLabelThickness(12.0f, 10.0f, 12.0f, 10.0f, &tPadding);
	(void)xuiWidgetSetPadding(pDemo->pCell[(3 * LABEL_PER_AREA) + 7], tPadding);
	return XUI_OK;
}

static void __xuiLabelLayout(xui_label_demo_t* pDemo)
{
	float fAreaW;
	float fAreaH;
	float fCellH;
	int i;

	fAreaW = ((float)DEMO_TARGET_W - 36.0f - 12.0f) * 0.5f;
	fAreaH = ((float)DEMO_TARGET_H - 36.0f - 12.0f) * 0.5f;
	fCellH = (fAreaH - 20.0f - 16.0f) / 3.0f;
	(void)xuiWidgetSetGridMetrics(pDemo->pRoot, 2, fAreaW, fAreaH);
	for ( i = 0; i < LABEL_AREA_COUNT; i++ ) {
		(void)xuiWidgetSetGridMetrics(pDemo->pArea[i], 3, (fAreaW - 20.0f - 16.0f) / 3.0f, fCellH);
	}
	(void)xuiLayout(pDemo->pContext);
}

static void __xuiLabelUpdateDynamic(xui_label_demo_t* pDemo)
{
	xui_style_value_t tValue;
	char arrText[64];
	int iPhase;

	iPhase = (pDemo->iFrame / 90) & 1;
	if ( iPhase != pDemo->iLastSkinPhase ) {
		tValue = __xuiLabelStyleColor(iPhase ? XUI_COLOR_RGBA(184, 82, 22, 255) : XUI_COLOR_RGBA(20, 112, 210, 255));
		(void)xuiStyleSetToken(pDemo->pContext, "demo.label.accent", &tValue);
		pDemo->iLastSkinPhase = iPhase;
	}
	snprintf(arrText, sizeof(arrText), "Dynamic skin %d", pDemo->iFrame);
	(void)xuiLabelSetText(pDemo->pLabel[(3 * LABEL_PER_AREA) + 8], arrText);
}

static void __xuiLabelRunChecks(xui_label_demo_t* pDemo)
{
	xui_rect_t tArea;
	xui_rect_t tLabel;
	int i;

	pDemo->bCreateOK = (pDemo->pRoot != NULL);
	for ( i = 0; i < LABEL_AREA_COUNT; i++ ) {
		if ( pDemo->pArea[i] == NULL ) {
			pDemo->bCreateOK = 0;
		}
	}
	for ( i = 0; i < LABEL_COUNT; i++ ) {
		if ( (pDemo->pCell[i] == NULL) || (pDemo->pLabel[i] == NULL) ) {
			pDemo->bCreateOK = 0;
		}
	}
	tArea = xuiWidgetGetRect(pDemo->pArea[0]);
	tLabel = xuiWidgetGetRect(pDemo->pLabel[0]);
	pDemo->bLayoutOK = (tArea.fW > 300.0f) && (tArea.fH > 240.0f) && (tLabel.fW > 80.0f) && (tLabel.fH > 40.0f);
	pDemo->bStateOK =
		(xuiWidgetGetEnabled(pDemo->pLabel[(3 * LABEL_PER_AREA) + 2]) == 0) &&
		(xuiLabelGetUnderline(pDemo->pLabel[(3 * LABEL_PER_AREA) + 3]) != 0) &&
		(xuiWidgetGetCacheSurface(pDemo->pLabel[(3 * LABEL_PER_AREA) + 8], xuiWidgetGetStateId(pDemo->pLabel[(3 * LABEL_PER_AREA) + 8])) != NULL);
}

static int __xuiLabelCreateAssets(xui_label_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) {
		printf("xui_label: xuiCreate failed: %d\n", iRet);
		return iRet;
	}
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) {
		printf("xui_label: xuiSetProxy failed: %d\n", iRet);
		return iRet;
	}
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H);
	if ( iRet != XUI_OK ) {
		return iRet;
	}

	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) {
		printf("xui_label: surfaceCreate failed: %d\n", iRet);
		return iRet;
	}
	sFontPath = __xuiLabelFindTtf();
	if ( sFontPath == NULL ) {
		printf("xui_label: no usable system TTF font found\n");
		return XUI_ERROR_FILE_NOT_FOUND;
	}
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 16.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pSmallFont, sFontPath, 10.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pLargeFont, sFontPath, 24.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	(void)xuiRegisterFont(pDemo->pContext, "body", pDemo->pFont);
	(void)xuiRegisterFont(pDemo->pContext, "small", pDemo->pSmallFont);
	(void)xuiRegisterFont(pDemo->pContext, "large", pDemo->pLargeFont);
	pDemo->iLastSkinPhase = -1;
	return __xuiLabelCreateUi(pDemo);
}

static void __xuiLabelDestroyAssets(xui_label_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
	}
	if ( pDemo->pLargeFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pLargeFont);
	}
	if ( pDemo->pSmallFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pSmallFont);
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
	}
}

static int __xuiLabelFrame(void* pUser)
{
	xui_label_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;

	pDemo = (xui_label_demo_t*)pUser;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	__xuiLabelUpdateDynamic(pDemo);
	__xuiLabelLayout(pDemo);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	tFullRect.iX = 0;
	tFullRect.iY = 0;
	tFullRect.iW = DEMO_TARGET_W;
	tFullRect.iH = DEMO_TARGET_H;
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	__xuiLabelRunChecks(pDemo);

	xgeClear(XUI_COLOR_RGBA(22, 26, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){10.0f, 20.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) {
		iRet = xgeEnd();
	}
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pDemo->iFrame++;
	if ( ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ||
	     ((pDemo->fMaxSeconds > 0.0) && (xgeTimer() >= pDemo->fMaxSeconds)) ) {
		memset(&tStats, 0, sizeof(tStats));
		memset(&tCacheStats, 0, sizeof(tCacheStats));
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		(void)xuiGetCacheStats(pDemo->pContext, &tCacheStats);
		printf(
			"xui_label final-summary frames=%d create=%d layout=%d state=%d labels=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame,
			pDemo->bCreateOK,
			pDemo->bLayoutOK,
			pDemo->bStateOK,
			LABEL_COUNT,
			tStats.iUpdatedCaches,
			tStats.iDrawnCaches,
			tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_label_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiLabelParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) {
		return 0;
	}
	if ( iRet != XGE_OK ) {
		__xuiLabelUsage();
		return 1;
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 980;
	tDesc.iHeight = 720;
	tDesc.sTitle = "xui_label";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_label: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiLabelCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		__xuiLabelDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiLabelFrame, &tDemo);
	__xuiLabelDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bStateOK) ? 0 : 1;
}
