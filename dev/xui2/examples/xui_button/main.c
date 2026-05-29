#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	940
#define DEMO_TARGET_H	560
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define BUTTON_COUNT	18
#define LABEL_COUNT	3
#define SURFACE_COUNT	4

typedef struct xui_button_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTargetSurface;
	xui_surface pSurface[SURFACE_COUNT];
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pLabel[LABEL_COUNT];
	xui_widget pRow[LABEL_COUNT];
	xui_widget pButton[BUTTON_COUNT];
	int iLabelCount;
	int iButtonCount;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int iClickCount;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	int bInputOK;
} xui_button_demo_t;

static uint32_t __xuiButtonColorR(uint32_t iColor) { return (iColor >> 24) & 0xffu; }
static uint32_t __xuiButtonColorG(uint32_t iColor) { return (iColor >> 16) & 0xffu; }
static uint32_t __xuiButtonColorB(uint32_t iColor) { return (iColor >> 8) & 0xffu; }
static uint32_t __xuiButtonColorA(uint32_t iColor) { return iColor & 0xffu; }

static void __xuiButtonUsage(void)
{
	printf("usage: xui_button [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiButtonParseArgs(xui_button_demo_t* pDemo, int argc, char** argv)
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
			__xuiButtonUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiButtonFindTtf(void)
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

static uint32_t __xuiButtonBlend(uint32_t iA, uint32_t iB, int iStep, int iTotal)
{
	int r;
	int g;
	int b;
	int a;

	if ( iTotal <= 0 ) {
		return iA;
	}
	r = (int)__xuiButtonColorR(iA) + ((int)__xuiButtonColorR(iB) - (int)__xuiButtonColorR(iA)) * iStep / iTotal;
	g = (int)__xuiButtonColorG(iA) + ((int)__xuiButtonColorG(iB) - (int)__xuiButtonColorG(iA)) * iStep / iTotal;
	b = (int)__xuiButtonColorB(iA) + ((int)__xuiButtonColorB(iB) - (int)__xuiButtonColorB(iA)) * iStep / iTotal;
	a = (int)__xuiButtonColorA(iA) + ((int)__xuiButtonColorA(iB) - (int)__xuiButtonColorA(iA)) * iStep / iTotal;
	return XUI_COLOR_RGBA(r, g, b, a);
}

static void __xuiButtonWritePixel(unsigned char* pPixels, int iWidth, int x, int y, uint32_t iColor)
{
	pPixels[((y * iWidth + x) * 4) + 0] = (unsigned char)__xuiButtonColorR(iColor);
	pPixels[((y * iWidth + x) * 4) + 1] = (unsigned char)__xuiButtonColorG(iColor);
	pPixels[((y * iWidth + x) * 4) + 2] = (unsigned char)__xuiButtonColorB(iColor);
	pPixels[((y * iWidth + x) * 4) + 3] = (unsigned char)__xuiButtonColorA(iColor);
}

static int __xuiButtonCreateIconSurface(xui_button_demo_t* pDemo, xui_surface* ppSurface)
{
	unsigned char arrPixels[32 * 32 * 4];
	int x;
	int y;
	uint32_t iColor;

	for ( y = 0; y < 32; y++ ) {
		for ( x = 0; x < 32; x++ ) {
			iColor = XUI_COLOR_RGBA(0, 0, 0, 0);
			if ( (x >= 7 && x <= 24 && y >= 7 && y <= 24) ||
			     (x >= 13 && x <= 18 && y >= 3 && y <= 28) ||
			     (y >= 13 && y <= 18 && x >= 3 && x <= 28) ) {
				iColor = __xuiButtonBlend(XUI_COLOR_RGBA(255, 255, 255, 255), XUI_COLOR_RGBA(210, 236, 255, 255), y, 31);
			}
			if ( (x == 7 || x == 24 || y == 7 || y == 24) &&
			     (x >= 7 && x <= 24 && y >= 7 && y <= 24) ) {
				iColor = XUI_COLOR_RGBA(40, 104, 178, 255);
			}
			__xuiButtonWritePixel(arrPixels, 32, x, y, iColor);
		}
	}
	return pDemo->tProxy.surfaceCreateRGBA(&pDemo->tProxy, ppSurface, 32, 32, arrPixels, 32 * 4, XUI_SURFACE_ALPHA_PREMULTIPLIED);
}

static int __xuiButtonCreateButtonSurface(xui_button_demo_t* pDemo, xui_surface* ppSurface, int iWidth, int iHeight, uint32_t iTop, uint32_t iBottom, uint32_t iBorder, int bRounded, int bPattern)
{
	unsigned char arrPixels[96 * 40 * 4];
	int x;
	int y;
	int r;
	int dx;
	int dy;
	int bInside;
	uint32_t iColor;

	if ( (iWidth > 96) || (iHeight > 40) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	r = iHeight / 2;
	for ( y = 0; y < iHeight; y++ ) {
		for ( x = 0; x < iWidth; x++ ) {
			bInside = 1;
			if ( bRounded ) {
				if ( x < r && y < r ) {
					dx = r - x - 1;
					dy = r - y - 1;
					bInside = (dx * dx + dy * dy) <= r * r;
				} else if ( x >= iWidth - r && y < r ) {
					dx = x - (iWidth - r);
					dy = r - y - 1;
					bInside = (dx * dx + dy * dy) <= r * r;
				} else if ( x < r && y >= iHeight - r ) {
					dx = r - x - 1;
					dy = y - (iHeight - r);
					bInside = (dx * dx + dy * dy) <= r * r;
				} else if ( x >= iWidth - r && y >= iHeight - r ) {
					dx = x - (iWidth - r);
					dy = y - (iHeight - r);
					bInside = (dx * dx + dy * dy) <= r * r;
				}
			}
			if ( !bInside ) {
				__xuiButtonWritePixel(arrPixels, iWidth, x, y, XUI_COLOR_RGBA(0, 0, 0, 0));
				continue;
			}
			iColor = __xuiButtonBlend(iTop, iBottom, y, iHeight - 1);
			if ( bPattern && ((x / 6) % 2 == 0) && x > r && x < iWidth - r ) {
				iColor = __xuiButtonBlend(iColor, XUI_COLOR_RGBA(255, 255, 255, 255), 1, 6);
			}
			if ( x == 0 || y == 0 || x == iWidth - 1 || y == iHeight - 1 ) {
				iColor = iBorder;
			}
			__xuiButtonWritePixel(arrPixels, iWidth, x, y, iColor);
		}
	}
	return pDemo->tProxy.surfaceCreateRGBA(&pDemo->tProxy, ppSurface, iWidth, iHeight, arrPixels, iWidth * 4, XUI_SURFACE_ALPHA_PREMULTIPLIED);
}

static int __xuiButtonCreateSurfaces(xui_button_demo_t* pDemo)
{
	int iRet;

	iRet = __xuiButtonCreateIconSurface(pDemo, &pDemo->pSurface[0]);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiButtonCreateButtonSurface(pDemo, &pDemo->pSurface[1], 96, 40, XUI_COLOR_RGBA(80, 166, 235, 255), XUI_COLOR_RGBA(28, 112, 202, 255), XUI_COLOR_RGBA(20, 88, 165, 255), 0, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiButtonCreateButtonSurface(pDemo, &pDemo->pSurface[2], 64, 32, XUI_COLOR_RGBA(116, 205, 255, 255), XUI_COLOR_RGBA(30, 137, 222, 255), XUI_COLOR_RGBA(24, 92, 170, 255), 1, 0);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiButtonCreateButtonSurface(pDemo, &pDemo->pSurface[3], 64, 32, XUI_COLOR_RGBA(167, 226, 154, 255), XUI_COLOR_RGBA(56, 154, 90, 255), XUI_COLOR_RGBA(34, 108, 66, 255), 1, 1);
}

static void __xuiButtonThickness(float fLeft, float fTop, float fRight, float fBottom, xui_thickness_t* pOut)
{
	pOut->fLeft = fLeft;
	pOut->fTop = fTop;
	pOut->fRight = fRight;
	pOut->fBottom = fBottom;
}

static int __xuiButtonSetFillLayout(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	}
	return iRet;
}

static int __xuiButtonSetFixedHeight(xui_widget pWidget, float fHeight)
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

static void __xuiButtonClick(xui_widget pWidget, void* pUser)
{
	xui_button_demo_t* pDemo;

	(void)pWidget;
	pDemo = (xui_button_demo_t*)pUser;
	pDemo->iClickCount++;
}

static int __xuiButtonAddLabel(xui_button_demo_t* pDemo, const char* sText)
{
	xui_label_desc_t tDesc;
	int iIndex;
	int iRet;

	if ( pDemo->iLabelCount >= LABEL_COUNT ) {
		return XUI_ERROR;
	}
	iIndex = pDemo->iLabelCount++;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(38, 48, 64, 255);
	tDesc.iDisabledTextColor = XUI_COLOR_RGBA(126, 134, 148, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pDemo->pLabel[iIndex], &tDesc);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	(void)__xuiButtonSetFixedHeight(pDemo->pLabel[iIndex], 28.0f);
	return xuiWidgetAddChild(pDemo->pRoot, pDemo->pLabel[iIndex]);
}

static int __xuiButtonAddRow(xui_button_demo_t* pDemo, int iColumns, float fHeight)
{
	int iIndex;
	int iRet;

	iIndex = pDemo->iLabelCount - 1;
	if ( (iIndex < 0) || (iIndex >= LABEL_COUNT) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRow[iIndex]);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	(void)xuiWidgetSetLayoutType(pDemo->pRow[iIndex], XUI_LAYOUT_GRID);
	(void)xuiWidgetSetGap(pDemo->pRow[iIndex], 10.0f);
	(void)xuiWidgetSetGridMetrics(pDemo->pRow[iIndex], iColumns, 0.0f, fHeight);
	(void)__xuiButtonSetFixedHeight(pDemo->pRow[iIndex], fHeight);
	return xuiWidgetAddChild(pDemo->pRoot, pDemo->pRow[iIndex]);
}

static xui_widget __xuiButtonAddButton(xui_button_demo_t* pDemo, xui_widget pParent, const char* sText, uint32_t iNormal, uint32_t iHover, uint32_t iActive)
{
	xui_button_desc_t tDesc;
	xui_widget pButton;
	xui_thickness_t tPadding;

	if ( pDemo->iButtonCount >= BUTTON_COUNT ) {
		return NULL;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	if ( xuiButtonCreate(pDemo->pContext, &pButton, &tDesc) != XUI_OK ) {
		return NULL;
	}
	(void)xuiButtonSetTextColor(pButton, XUI_COLOR_RGBA(255, 255, 255, 255));
	(void)xuiButtonSetColors(pButton, iNormal, iHover, iActive, __xuiButtonBlend(iHover, XUI_COLOR_RGBA(255, 255, 255, 255), 1, 3), XUI_COLOR_RGBA(150, 156, 166, 130));
	(void)xuiButtonSetBorder(pButton, 1.0f, __xuiButtonBlend(iActive, XUI_COLOR_RGBA(0, 0, 0, 255), 1, 3));
	(void)xuiButtonSetRadius(pButton, 4.0f);
	__xuiButtonThickness(10.0f, 6.0f, 10.0f, 6.0f, &tPadding);
	(void)xuiWidgetSetPadding(pButton, tPadding);
	(void)__xuiButtonSetFillLayout(pButton);
	(void)xuiButtonSetClick(pButton, __xuiButtonClick, pDemo);
	if ( xuiWidgetAddChild(pParent, pButton) != XUI_OK ) {
		xuiWidgetDestroy(pButton);
		return NULL;
	}
	pDemo->pButton[pDemo->iButtonCount++] = pButton;
	return pButton;
}

static void __xuiButtonConfigureSelectedVisual(xui_widget pButton, uint32_t iSelected, uint32_t iBorder)
{
	(void)xuiButtonSetStateVisual(pButton, XUI_BUTTON_STATE_CHECKED, iSelected, 2.0f, iBorder);
}

static void __xuiButtonSetPatchState(xui_widget pButton, const xui_nine_patch_t* pPatch, uint32_t iState, uint32_t iColor)
{
	xui_nine_patch_t tStatePatch;

	if ( (pButton == NULL) || (pPatch == NULL) ) {
		return;
	}
	tStatePatch = *pPatch;
	tStatePatch.iColor = iColor;
	(void)xuiButtonSetPatch(pButton, iState, &tStatePatch);
}

static void __xuiButtonSetPatchStates(xui_widget pButton, const xui_nine_patch_t* pPatch)
{
	__xuiButtonSetPatchState(pButton, pPatch, 0, XUI_COLOR_RGBA(235, 244, 255, 255));
	__xuiButtonSetPatchState(pButton, pPatch, XUI_WIDGET_STATE_HOVER, XUI_COLOR_RGBA(255, 246, 145, 255));
	__xuiButtonSetPatchState(pButton, pPatch, XUI_WIDGET_STATE_ACTIVE, XUI_COLOR_RGBA(92, 156, 255, 255));
	__xuiButtonSetPatchState(pButton, pPatch, XUI_BUTTON_STATE_CHECKED, XUI_COLOR_RGBA(255, 196, 58, 255));
	__xuiButtonSetPatchState(pButton, pPatch, XUI_WIDGET_STATE_DISABLED, XUI_COLOR_RGBA(150, 154, 160, 150));
}

static int __xuiButtonCreateUi(xui_button_demo_t* pDemo)
{
	static const uint32_t arrNormal[7] = {
		XUI_COLOR_RGBA(212, 68, 72, 255), XUI_COLOR_RGBA(220, 123, 38, 255), XUI_COLOR_RGBA(206, 168, 48, 255),
		XUI_COLOR_RGBA(70, 164, 92, 255), XUI_COLOR_RGBA(34, 162, 174, 255), XUI_COLOR_RGBA(54, 126, 218, 255), XUI_COLOR_RGBA(138, 92, 202, 255)
	};
	static const char* arrName[7] = { "Red", "Orange", "Yellow", "Green", "Cyan", "Blue", "Purple" };
	xui_widget pRow;
	xui_widget pButton;
	xui_thickness_t tPadding;
	xui_nine_patch_t tPatch;
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
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetGap(pDemo->pRoot, 9.0f);
	__xuiButtonThickness(22.0f, 18.0f, 22.0f, 18.0f, &tPadding);
	(void)xuiWidgetSetPadding(pDemo->pRoot, tPadding);
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});

	iRet = __xuiButtonAddLabel(pDemo, "Seven color schemes");
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiButtonAddRow(pDemo, 7, 46.0f);
	if ( iRet != XUI_OK ) return iRet;
	pRow = pDemo->pRow[0];
	for ( i = 0; i < 7; i++ ) {
		if ( __xuiButtonAddButton(pDemo, pRow, arrName[i], arrNormal[i], __xuiButtonBlend(arrNormal[i], XUI_COLOR_RGBA(255, 255, 255, 255), 1, 5), __xuiButtonBlend(arrNormal[i], XUI_COLOR_RGBA(0, 0, 0, 255), 1, 5)) == NULL ) {
			return XUI_ERROR;
		}
	}

	iRet = __xuiButtonAddLabel(pDemo, "Icon and text layout, selectable buttons");
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiButtonAddRow(pDemo, 7, 58.0f);
	if ( iRet != XUI_OK ) return iRet;
	pRow = pDemo->pRow[1];
	for ( i = 0; i < 4; i++ ) {
		pButton = __xuiButtonAddButton(pDemo, pRow, (i == 0) ? "Left" : (i == 1) ? "Right" : (i == 2) ? "Top" : "Bottom", XUI_COLOR_RGBA(56, 116, 190, 255), XUI_COLOR_RGBA(72, 140, 216, 255), XUI_COLOR_RGBA(36, 92, 164, 255));
		if ( pButton == NULL ) return XUI_ERROR;
		(void)xuiButtonSetIcon(pButton, pDemo->pSurface[0], (xui_rect_t){0.0f, 0.0f, 32.0f, 32.0f});
		(void)xuiButtonSetIconLayout(pButton, i, 18.0f, 5.0f);
	}
	pButton = __xuiButtonAddButton(pDemo, pRow, "Select", XUI_COLOR_RGBA(58, 130, 88, 255), XUI_COLOR_RGBA(70, 154, 104, 255), XUI_COLOR_RGBA(36, 104, 68, 255));
	if ( pButton == NULL ) return XUI_ERROR;
	(void)xuiButtonSetSelectable(pButton, 1);
	__xuiButtonConfigureSelectedVisual(pButton, XUI_COLOR_RGBA(18, 96, 62, 255), XUI_COLOR_RGBA(255, 210, 54, 255));
	(void)xuiButtonSetSelected(pButton, 1);
	pButton = __xuiButtonAddButton(pDemo, pRow, "Toggle", XUI_COLOR_RGBA(96, 112, 132, 255), XUI_COLOR_RGBA(116, 134, 158, 255), XUI_COLOR_RGBA(72, 88, 110, 255));
	if ( pButton == NULL ) return XUI_ERROR;
	(void)xuiButtonSetSelectable(pButton, 1);
	__xuiButtonConfigureSelectedVisual(pButton, XUI_COLOR_RGBA(32, 92, 176, 255), XUI_COLOR_RGBA(255, 210, 54, 255));
	pButton = __xuiButtonAddButton(pDemo, pRow, "Disabled", XUI_COLOR_RGBA(96, 112, 132, 255), XUI_COLOR_RGBA(116, 134, 158, 255), XUI_COLOR_RGBA(72, 88, 110, 255));
	if ( pButton == NULL ) return XUI_ERROR;
	(void)xuiButtonSetTextColor(pButton, XUI_COLOR_RGBA(232, 236, 242, 180));
	(void)xuiButtonSetDisabledTextColor(pButton, XUI_COLOR_RGBA(232, 236, 242, 140));
	(void)xuiButtonSetStateVisual(pButton, XUI_WIDGET_STATE_DISABLED, XUI_COLOR_RGBA(109, 120, 136, 255), 1.0f, XUI_COLOR_RGBA(58, 67, 80, 255));
	(void)xuiWidgetSetEnabled(pButton, 0);

	iRet = __xuiButtonAddLabel(pDemo, "Image button, nine-patch button and badge");
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiButtonAddRow(pDemo, 4, 66.0f);
	if ( iRet != XUI_OK ) return iRet;
	pRow = pDemo->pRow[2];
	pButton = __xuiButtonAddButton(pDemo, pRow, "Image", XUI_COLOR_RGBA(0, 0, 0, 0), XUI_COLOR_RGBA(0, 0, 0, 0), XUI_COLOR_RGBA(0, 0, 0, 0));
	if ( pButton == NULL ) return XUI_ERROR;
	memset(&tPatch, 0, sizeof(tPatch));
	tPatch.iSize = sizeof(tPatch);
	tPatch.pSurface = pDemo->pSurface[1];
	tPatch.tSrc = (xui_rect_t){0.0f, 0.0f, 96.0f, 40.0f};
	tPatch.iColor = XUI_COLOR_WHITE;
	tPatch.iMode = XUI_NINE_PATCH_STRETCH;
	__xuiButtonSetPatchStates(pButton, &tPatch);
	pButton = __xuiButtonAddButton(pDemo, pRow, "9-Slice", XUI_COLOR_RGBA(0, 0, 0, 0), XUI_COLOR_RGBA(0, 0, 0, 0), XUI_COLOR_RGBA(0, 0, 0, 0));
	if ( pButton == NULL ) return XUI_ERROR;
	tPatch.pSurface = pDemo->pSurface[2];
	tPatch.tSrc = (xui_rect_t){0.0f, 0.0f, 64.0f, 32.0f};
	tPatch.tSlice = (xui_thickness_t){16.0f, 8.0f, 16.0f, 8.0f};
	tPatch.iMode = XUI_NINE_PATCH_STRETCH;
	__xuiButtonSetPatchStates(pButton, &tPatch);
	pButton = __xuiButtonAddButton(pDemo, pRow, "9-Slice Tile", XUI_COLOR_RGBA(0, 0, 0, 0), XUI_COLOR_RGBA(0, 0, 0, 0), XUI_COLOR_RGBA(0, 0, 0, 0));
	if ( pButton == NULL ) return XUI_ERROR;
	tPatch.pSurface = pDemo->pSurface[3];
	tPatch.iMode = XUI_NINE_PATCH_TILE;
	__xuiButtonSetPatchStates(pButton, &tPatch);
	pButton = __xuiButtonAddButton(pDemo, pRow, "Badge", XUI_COLOR_RGBA(62, 111, 188, 255), XUI_COLOR_RGBA(74, 134, 214, 255), XUI_COLOR_RGBA(38, 88, 160, 255));
	if ( pButton == NULL ) return XUI_ERROR;
	(void)xuiButtonSetBadgeVisible(pButton, 1);
	(void)xuiButtonSetBadgeSize(pButton, 12.0f);
	return XUI_OK;
}

static void __xuiButtonLayout(xui_button_demo_t* pDemo)
{
	float fContentW;
	float fItemW;
	int i;
	static const int arrColumns[LABEL_COUNT] = {7, 7, 4};
	static const float arrHeight[LABEL_COUNT] = {46.0f, 58.0f, 66.0f};

	fContentW = (float)DEMO_TARGET_W - 44.0f;
	for ( i = 0; i < LABEL_COUNT; i++ ) {
		if ( pDemo->pRow[i] == NULL ) {
			continue;
		}
		fItemW = (fContentW - 10.0f * (float)(arrColumns[i] - 1)) / (float)arrColumns[i];
		(void)xuiWidgetSetGridMetrics(pDemo->pRow[i], arrColumns[i], fItemW, arrHeight[i]);
	}
	(void)xuiLayout(pDemo->pContext);
}

static uint32_t __xuiButtonReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) {
		iButtons |= XUI_POINTER_BUTTON_LEFT;
	}
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) {
		iButtons |= XUI_POINTER_BUTTON_RIGHT;
	}
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) {
		iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	}
	return iButtons;
}

static int __xuiButtonSendButtonTransitions(xui_button_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
{
	int iRet;

	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiButtonHandleInput(xui_button_demo_t* pDemo)
{
	float fX;
	float fY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiButtonReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiButtonSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiButtonRunChecks(xui_button_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tRoot;
	xui_rect_t tButton;
	float fX;
	float fY;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->iButtonCount == BUTTON_COUNT) && (pDemo->iLabelCount == LABEL_COUNT);
	if ( !pDemo->bCreateOK ) {
		return;
	}
	tRoot = xuiWidgetGetRect(pDemo->pRoot);
	tButton = xuiWidgetGetRect(pDemo->pButton[0]);
	pDemo->bLayoutOK = (tRoot.fW >= 920.0f) && (tButton.fW > 70.0f) && (tButton.fH >= 40.0f);
	pDemo->bStateOK =
		(xuiButtonGetIconSurface(pDemo->pButton[7]) == pDemo->pSurface[0]) &&
		(xuiButtonIsSelected(pDemo->pButton[11]) != 0) &&
		(xuiWidgetGetEnabled(pDemo->pButton[13]) == 0) &&
		(xuiButtonHasPatch(pDemo->pButton[14], 0) != 0) &&
		(xuiButtonGetBadgeVisible(pDemo->pButton[17]) != 0);
	if ( bExerciseInput && !pDemo->bExerciseDone ) {
		tButton = xuiWidgetGetWorldRect(pDemo->pButton[0]);
		fX = tButton.fX + tButton.fW * 0.5f;
		fY = tButton.fY + tButton.fH * 0.5f;
		(void)xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bExerciseDone = 1;
	}
	pDemo->bInputOK = !bExerciseInput || (pDemo->iClickCount > 0);
}

static int __xuiButtonCreateAssets(xui_button_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) {
		printf("xui_button: xuiCreate failed: %d\n", iRet);
		return iRet;
	}
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) {
		printf("xui_button: xuiSetProxy failed: %d\n", iRet);
		return iRet;
	}
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTargetSurface, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiButtonFindTtf();
	if ( sFontPath == NULL ) {
		printf("xui_button: no usable system TTF font found\n");
		return XUI_ERROR_FILE_NOT_FOUND;
	}
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 16.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	iRet = __xuiButtonCreateSurfaces(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiButtonCreateUi(pDemo);
}

static void __xuiButtonDestroyAssets(xui_button_demo_t* pDemo)
{
	int i;

	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
	}
	for ( i = 0; i < SURFACE_COUNT; i++ ) {
		if ( pDemo->pSurface[i] != NULL ) {
			pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pSurface[i]);
		}
	}
	if ( pDemo->pTargetSurface != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTargetSurface);
	}
}

static int __xuiButtonFrame(void* pUser)
{
	xui_button_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_button_demo_t*)pUser;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiButtonHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiButtonLayout(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiButtonRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTargetSurface, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTargetSurface, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(22, 26, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTargetSurface, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XGE_OK ) {
		iRet = xgeEnd();
	}
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ||
	     ((pDemo->fMaxSeconds > 0.0) && (xgeTimer() >= pDemo->fMaxSeconds)) ) {
		memset(&tStats, 0, sizeof(tStats));
		tStats.iSize = sizeof(tStats);
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		printf("xui_button final-summary frames=%d create=%d layout=%d state=%d input=%d buttons=%d clicks=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame,
			pDemo->bCreateOK,
			pDemo->bLayoutOK,
			pDemo->bStateOK,
			pDemo->bInputOK,
			pDemo->iButtonCount,
			pDemo->iClickCount,
			tStats.iUpdatedCaches,
			tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_button_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiButtonParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) {
		return 0;
	}
	if ( iRet != XGE_OK ) {
		__xuiButtonUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Button";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_button: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiButtonCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		__xuiButtonDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiButtonFrame, &tDemo);
	__xuiButtonDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bStateOK && ((tDemo.iMaxFrames <= 0 && tDemo.fMaxSeconds <= 0.0) || tDemo.bInputOK)) ? 0 : 1;
}
