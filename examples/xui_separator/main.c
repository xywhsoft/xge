#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 940
#define DEMO_TARGET_H 560
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define LABEL_COUNT 17
#define SEPARATOR_COUNT 12
#define ROW_COUNT 4
#define GROUP_COUNT 2

typedef struct xui_separator_box_t {
	xui_proxy pProxy;
	uint32_t iFill;
	uint32_t iStroke;
	float fRadius;
	float fStrokeWidth;
} xui_separator_box_t;

typedef struct xui_separator_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pLeft;
	xui_widget pRight;
	xui_widget pGroup[GROUP_COUNT];
	xui_widget pRow[ROW_COUNT];
	xui_widget pLabel[LABEL_COUNT];
	xui_widget pSeparator[SEPARATOR_COUNT];
	xui_separator_box_t tRootBox;
	xui_separator_box_t tPanelBox[2];
	xui_separator_box_t tGroupBox[GROUP_COUNT];
	xui_separator_box_t tRowBox[ROW_COUNT];
	int iLabelCount;
	int iSeparatorCount;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
} xui_separator_demo_t;

static void __xuiSeparatorUsage(void)
{
	printf("usage: xui_separator [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiSeparatorParseArgs(xui_separator_demo_t* pDemo, int argc, char** argv)
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
			__xuiSeparatorUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiSeparatorFindTtf(void)
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

static void __xuiSeparatorThickness(float fLeft, float fTop, float fRight, float fBottom, xui_thickness_t* pOut)
{
	pOut->fLeft = fLeft;
	pOut->fTop = fTop;
	pOut->fRight = fRight;
	pOut->fBottom = fBottom;
}

static void __xuiSeparatorSetupBox(xui_separator_box_t* pBox, xui_proxy pProxy, uint32_t iFill, uint32_t iStroke, float fRadius, float fStrokeWidth)
{
	pBox->pProxy = pProxy;
	pBox->iFill = iFill;
	pBox->iStroke = iStroke;
	pBox->fRadius = fRadius;
	pBox->fStrokeWidth = fStrokeWidth;
}

static int __xuiSeparatorBoxRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_separator_box_t* pBox;
	xui_rect_t tRect;
	int iRet;

	(void)iStateId;
	pBox = (xui_separator_box_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pBox == NULL) || (pBox->pProxy == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	iRet = pBox->pProxy->drawRectFill(pBox->pProxy, pDraw, tRect, pBox->iFill);
	if ( (iRet == XUI_OK) && (pBox->fStrokeWidth > 0.0f) ) {
		iRet = pBox->pProxy->drawRectStroke(pBox->pProxy, pDraw, tRect, pBox->fStrokeWidth, pBox->iStroke);
	}
	return iRet;
}

static int __xuiSeparatorSetFillLayout(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	}
	return iRet;
}

static int __xuiSeparatorSetFixedHeight(xui_widget pWidget, float fHeight)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetSetPreferredSize(pWidget, (xui_vec2_t){0.0f, fHeight});
	}
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	}
	return iRet;
}

static int __xuiSeparatorSetFixedWidth(xui_widget pWidget, float fWidth)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FIXED, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetSetPreferredSize(pWidget, (xui_vec2_t){fWidth, 0.0f});
	}
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetSetAlign(pWidget, XUI_ALIGN_START, XUI_ALIGN_STRETCH);
	}
	return iRet;
}

static int __xuiSeparatorAddLabel(xui_separator_demo_t* pDemo, xui_widget pParent, const char* sText, uint32_t iColor, float fHeight, int bFill)
{
	xui_label_desc_t tDesc;
	xui_thickness_t tPadding;
	xui_widget pLabel;
	int iRet;

	if ( pDemo->iLabelCount >= LABEL_COUNT ) {
		return XUI_ERROR;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iDisabledTextColor = XUI_COLOR_RGBA(126, 134, 148, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	__xuiSeparatorThickness(6.0f, 5.0f, 6.0f, 5.0f, &tPadding);
	(void)xuiWidgetSetPadding(pLabel, tPadding);
	if ( bFill ) {
		(void)__xuiSeparatorSetFillLayout(pLabel);
	} else {
		(void)__xuiSeparatorSetFixedHeight(pLabel, fHeight);
	}
	iRet = xuiWidgetAddChild(pParent, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[pDemo->iLabelCount++] = pLabel;
	return XUI_OK;
}

static int __xuiSeparatorAddSeparator(xui_separator_demo_t* pDemo, xui_widget pParent, int iOrientation, int iLineStyle, int iAlign, float fThickness, uint32_t iColor, float fCrossSize)
{
	xui_separator_desc_t tDesc;
	xui_widget pSeparator;
	int iRet;

	if ( pDemo->iSeparatorCount >= SEPARATOR_COUNT ) {
		return XUI_ERROR;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iColor = iColor;
	tDesc.fThickness = fThickness;
	tDesc.iOrientation = iOrientation;
	tDesc.iLineStyle = iLineStyle;
	iRet = xuiSeparatorCreate(pDemo->pContext, &pSeparator, &tDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	(void)xuiSeparatorSetAlign(pSeparator, iAlign);
	if ( iOrientation == XUI_SEPARATOR_VERTICAL ) {
		(void)__xuiSeparatorSetFixedWidth(pSeparator, fCrossSize);
	} else {
		(void)__xuiSeparatorSetFixedHeight(pSeparator, fCrossSize);
	}
	iRet = xuiWidgetAddChild(pParent, pSeparator);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pSeparator);
		return iRet;
	}
	pDemo->pSeparator[pDemo->iSeparatorCount++] = pSeparator;
	return XUI_OK;
}

static int __xuiSeparatorAddRightRow(xui_separator_demo_t* pDemo, xui_widget pParent, int iRow, int iStyleA, int iStyleB, const char* sA, const char* sB, const char* sC)
{
	xui_thickness_t tPadding;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRow[iRow]);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	__xuiSeparatorSetupBox(&pDemo->tRowBox[iRow], &pDemo->tProxy, XUI_COLOR_RGBA(248, 250, 253, 255), XUI_COLOR_RGBA(0, 0, 0, 0), 3.0f, 0.0f);
	__xuiSeparatorThickness(8.0f, 8.0f, 8.0f, 8.0f, &tPadding);
	(void)xuiWidgetSetLayoutType(pDemo->pRow[iRow], XUI_LAYOUT_ROW);
	(void)xuiWidgetSetGap(pDemo->pRow[iRow], 8.0f);
	(void)xuiWidgetSetPadding(pDemo->pRow[iRow], tPadding);
	(void)__xuiSeparatorSetFixedHeight(pDemo->pRow[iRow], 72.0f);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRow[iRow], __xuiSeparatorBoxRender, &pDemo->tRowBox[iRow]);
	iRet = xuiWidgetAddChild(pParent, pDemo->pRow[iRow]);
	if ( iRet != XUI_OK ) return iRet;
	if ( (__xuiSeparatorAddLabel(pDemo, pDemo->pRow[iRow], sA, XUI_COLOR_RGBA(38, 48, 64, 255), 0.0f, 1) != XUI_OK) ||
	     (__xuiSeparatorAddSeparator(pDemo, pDemo->pRow[iRow], XUI_SEPARATOR_VERTICAL, iStyleA, XUI_ALIGN_CENTER, 2.0f, XUI_COLOR_RGBA(44, 116, 190, 220), 14.0f) != XUI_OK) ||
	     (__xuiSeparatorAddLabel(pDemo, pDemo->pRow[iRow], sB, XUI_COLOR_RGBA(50, 58, 72, 255), 0.0f, 1) != XUI_OK) ||
	     (__xuiSeparatorAddSeparator(pDemo, pDemo->pRow[iRow], XUI_SEPARATOR_VERTICAL, iStyleB, XUI_ALIGN_CENTER, 2.0f, XUI_COLOR_RGBA(210, 132, 42, 230), 14.0f) != XUI_OK) ||
	     (__xuiSeparatorAddLabel(pDemo, pDemo->pRow[iRow], sC, XUI_COLOR_RGBA(38, 48, 64, 255), 0.0f, 1) != XUI_OK) ) {
		return XUI_ERROR;
	}
	return XUI_OK;
}

static int __xuiSeparatorCreateUi(xui_separator_demo_t* pDemo)
{
	xui_thickness_t tPadding;
	int i;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	__xuiSeparatorSetupBox(&pDemo->tRootBox, &pDemo->tProxy, XUI_COLOR_RGBA(229, 235, 244, 255), XUI_COLOR_RGBA(0, 0, 0, 0), 0.0f, 0.0f);
	__xuiSeparatorThickness(18.0f, 18.0f, 18.0f, 18.0f, &tPadding);
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_GRID);
	(void)xuiWidgetSetGap(pDemo->pRoot, 14.0f);
	(void)xuiWidgetSetPadding(pDemo->pRoot, tPadding);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiSeparatorBoxRender, &pDemo->tRootBox);
	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pLeft);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRight);
	if ( iRet != XUI_OK ) return iRet;
	__xuiSeparatorSetupBox(&pDemo->tPanelBox[0], &pDemo->tProxy, XUI_COLOR_RGBA(248, 250, 253, 255), XUI_COLOR_RGBA(170, 184, 202, 255), 5.0f, 1.0f);
	__xuiSeparatorSetupBox(&pDemo->tPanelBox[1], &pDemo->tProxy, XUI_COLOR_RGBA(241, 245, 250, 255), XUI_COLOR_RGBA(170, 184, 202, 255), 5.0f, 1.0f);
	__xuiSeparatorThickness(14.0f, 14.0f, 14.0f, 14.0f, &tPadding);
	(void)xuiWidgetSetLayoutType(pDemo->pLeft, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetGap(pDemo->pLeft, 8.0f);
	(void)xuiWidgetSetPadding(pDemo->pLeft, tPadding);
	(void)__xuiSeparatorSetFillLayout(pDemo->pLeft);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pLeft, __xuiSeparatorBoxRender, &pDemo->tPanelBox[0]);
	(void)xuiWidgetSetLayoutType(pDemo->pRight, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetGap(pDemo->pRight, 12.0f);
	(void)xuiWidgetSetPadding(pDemo->pRight, tPadding);
	(void)__xuiSeparatorSetFillLayout(pDemo->pRight);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRight, __xuiSeparatorBoxRender, &pDemo->tPanelBox[1]);
	(void)xuiWidgetAddChild(pDemo->pRoot, pDemo->pLeft);
	(void)xuiWidgetAddChild(pDemo->pRoot, pDemo->pRight);

	if ( (__xuiSeparatorAddLabel(pDemo, pDemo->pLeft, "A compact page reads better when related thoughts stay together.", XUI_COLOR_RGBA(36, 42, 52, 255), 54.0f, 0) != XUI_OK) ||
	     (__xuiSeparatorAddSeparator(pDemo, pDemo->pLeft, XUI_SEPARATOR_HORIZONTAL, XUI_SEPARATOR_SOLID, XUI_ALIGN_CENTER, 1.0f, XUI_COLOR_RGBA(80, 102, 130, 190), 14.0f) != XUI_OK) ||
	     (__xuiSeparatorAddLabel(pDemo, pDemo->pLeft, "The first break is quiet. It marks a pause without becoming a decoration.", XUI_COLOR_RGBA(48, 58, 72, 255), 58.0f, 0) != XUI_OK) ||
	     (__xuiSeparatorAddSeparator(pDemo, pDemo->pLeft, XUI_SEPARATOR_HORIZONTAL, XUI_SEPARATOR_DOT, XUI_ALIGN_CENTER, 2.0f, XUI_COLOR_RGBA(46, 124, 214, 210), 16.0f) != XUI_OK) ||
	     (__xuiSeparatorAddLabel(pDemo, pDemo->pLeft, "A dotted break is lighter and works well between small notes or metadata rows.", XUI_COLOR_RGBA(48, 58, 72, 255), 58.0f, 0) != XUI_OK) ||
	     (__xuiSeparatorAddSeparator(pDemo, pDemo->pLeft, XUI_SEPARATOR_HORIZONTAL, XUI_SEPARATOR_DASH, XUI_ALIGN_START, 3.0f, XUI_COLOR_RGBA(210, 132, 42, 220), 18.0f) != XUI_OK) ||
	     (__xuiSeparatorAddLabel(pDemo, pDemo->pLeft, "A dashed break is stronger. Aligning it to the top edge shows the line is independent of widget height.", XUI_COLOR_RGBA(48, 58, 72, 255), 64.0f, 0) != XUI_OK) ||
	     (__xuiSeparatorAddSeparator(pDemo, pDemo->pLeft, XUI_SEPARATOR_HORIZONTAL, XUI_SEPARATOR_DASH_DOT, XUI_ALIGN_END, 2.0f, XUI_COLOR_RGBA(92, 116, 210, 220), 18.0f) != XUI_OK) ||
	     (__xuiSeparatorAddLabel(pDemo, pDemo->pLeft, "The last break sits on the lower edge, leaving the paragraph rhythm intact.", XUI_COLOR_RGBA(36, 42, 52, 255), 0.0f, 1) != XUI_OK) ) {
		return XUI_ERROR;
	}

	for ( i = 0; i < GROUP_COUNT; i++ ) {
		iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pGroup[i]);
		if ( iRet != XUI_OK ) return iRet;
		__xuiSeparatorSetupBox(&pDemo->tGroupBox[i], &pDemo->tProxy, XUI_COLOR_RGBA(231, 238, 247, 255), XUI_COLOR_RGBA(170, 184, 202, 255), 4.0f, 1.0f);
		__xuiSeparatorThickness(10.0f, 10.0f, 10.0f, 10.0f, &tPadding);
		(void)xuiWidgetSetLayoutType(pDemo->pGroup[i], XUI_LAYOUT_COLUMN);
		(void)xuiWidgetSetGap(pDemo->pGroup[i], 10.0f);
		(void)xuiWidgetSetPadding(pDemo->pGroup[i], tPadding);
		(void)__xuiSeparatorSetFillLayout(pDemo->pGroup[i]);
		(void)xuiWidgetSetFlex(pDemo->pGroup[i], 1.0f, 1.0f);
		(void)xuiWidgetSetCacheRenderCallback(pDemo->pGroup[i], __xuiSeparatorBoxRender, &pDemo->tGroupBox[i]);
		(void)xuiWidgetAddChild(pDemo->pRight, pDemo->pGroup[i]);
	}
	if ( (__xuiSeparatorAddRightRow(pDemo, pDemo->pGroup[0], 0, XUI_SEPARATOR_SOLID, XUI_SEPARATOR_DOT, "Profile", "Scope", "Status") != XUI_OK) ||
	     (__xuiSeparatorAddRightRow(pDemo, pDemo->pGroup[0], 1, XUI_SEPARATOR_SOLID, XUI_SEPARATOR_DOT, "Author", "Review", "Ready") != XUI_OK) ||
	     (__xuiSeparatorAddRightRow(pDemo, pDemo->pGroup[1], 2, XUI_SEPARATOR_DASH, XUI_SEPARATOR_DASH_DOT, "Plan", "Build", "Verify") != XUI_OK) ||
	     (__xuiSeparatorAddRightRow(pDemo, pDemo->pGroup[1], 3, XUI_SEPARATOR_DASH, XUI_SEPARATOR_DASH_DOT, "Input", "Output", "Trace") != XUI_OK) ) {
		return XUI_ERROR;
	}
	return XUI_OK;
}

static void __xuiSeparatorLayout(xui_separator_demo_t* pDemo)
{
	float fPanelW;
	float fPanelH;

	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	fPanelW = ((float)DEMO_TARGET_W - 36.0f - 14.0f) * 0.5f;
	fPanelH = (float)DEMO_TARGET_H - 36.0f;
	(void)xuiWidgetSetGridMetrics(pDemo->pRoot, 2, fPanelW, fPanelH);
}

static void __xuiSeparatorRunChecks(xui_separator_demo_t* pDemo)
{
	xui_rect_t tLeft;
	xui_rect_t tRight;
	xui_rect_t tLine;

	tLeft = xuiWidgetGetRect(pDemo->pLeft);
	tRight = xuiWidgetGetRect(pDemo->pRight);
	tLine = xuiSeparatorGetLineRect(pDemo->pSeparator[2]);
	pDemo->bCreateOK = (pDemo->iLabelCount == LABEL_COUNT) && (pDemo->iSeparatorCount == SEPARATOR_COUNT);
	pDemo->bLayoutOK = (tLeft.fW > 300.0f) && (tRight.fW > 240.0f) && (tLine.fY == 0.0f) && (tLine.fH == 3.0f);
	pDemo->bStateOK =
		(xuiSeparatorGetLineStyle(pDemo->pSeparator[0]) == XUI_SEPARATOR_SOLID) &&
		(xuiSeparatorGetLineStyle(pDemo->pSeparator[1]) == XUI_SEPARATOR_DOT) &&
		(xuiSeparatorGetLineStyle(pDemo->pSeparator[2]) == XUI_SEPARATOR_DASH) &&
		(xuiSeparatorGetLineStyle(pDemo->pSeparator[3]) == XUI_SEPARATOR_DASH_DOT) &&
		(xuiSeparatorGetOrientation(pDemo->pSeparator[4]) == XUI_SEPARATOR_VERTICAL);
}

static int __xuiSeparatorCreateAssets(xui_separator_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiSeparatorFindTtf();
	if ( sFontPath == NULL ) {
		printf("xui_separator: no usable system TTF font found\n");
		return XUI_ERROR_FILE_NOT_FOUND;
	}
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 16.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiSeparatorCreateUi(pDemo);
}

static void __xuiSeparatorDestroyAssets(xui_separator_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
	}
}

static int __xuiSeparatorFrame(void* pUser)
{
	xui_separator_demo_t* pDemo;
	xui_render_stats_t tStats;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;

	pDemo = (xui_separator_demo_t*)pUser;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	__xuiSeparatorLayout(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	__xuiSeparatorRunChecks(pDemo);
	xgeClear(XUI_COLOR_RGBA(22, 26, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) {
		iRet = xgeEnd();
	}
	if ( iRet != XUI_OK ) return iRet;
	pDemo->iFrame++;
	if ( ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ||
	     ((pDemo->fMaxSeconds > 0.0) && (xgeTimer() >= pDemo->fMaxSeconds)) ) {
		memset(&tStats, 0, sizeof(tStats));
		tStats.iSize = sizeof(tStats);
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		printf("xui_separator final-summary frames=%d create=%d layout=%d state=%d labels=%d separators=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame,
			pDemo->bCreateOK,
			pDemo->bLayoutOK,
			pDemo->bStateOK,
			pDemo->iLabelCount,
			pDemo->iSeparatorCount,
			tStats.iUpdatedCaches,
			tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_separator_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiSeparatorParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) {
		return 0;
	}
	if ( iRet != XGE_OK ) {
		__xuiSeparatorUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Separator";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_separator: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiSeparatorCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_separator: create assets failed: %d\n", iRet);
		__xuiSeparatorDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiSeparatorFrame, &tDemo);
	__xuiSeparatorDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bStateOK) ? 0 : 1;
}
