#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUTTON_COUNT 18
#define LABEL_COUNT 3
#define TEXTURE_COUNT 4

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_texture_t tTexture[TEXTURE_COUNT];
	xge_xui_label_t tLabel[LABEL_COUNT];
	xge_xui_button_t tButton[BUTTON_COUNT];
	xge_xui_widget pButtonWidget[BUTTON_COUNT];
	int iLabelCount;
	int iButtonCount;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
} app_state_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static uint32_t BlendColor(uint32_t iA, uint32_t iB, int iStep, int iTotal)
{
	int r;
	int g;
	int b;
	int a;

	if ( iTotal <= 0 ) {
		return iA;
	}
	r = (int)XGE_COLOR_GET_R(iA) + ((int)XGE_COLOR_GET_R(iB) - (int)XGE_COLOR_GET_R(iA)) * iStep / iTotal;
	g = (int)XGE_COLOR_GET_G(iA) + ((int)XGE_COLOR_GET_G(iB) - (int)XGE_COLOR_GET_G(iA)) * iStep / iTotal;
	b = (int)XGE_COLOR_GET_B(iA) + ((int)XGE_COLOR_GET_B(iB) - (int)XGE_COLOR_GET_B(iA)) * iStep / iTotal;
	a = (int)XGE_COLOR_GET_A(iA) + ((int)XGE_COLOR_GET_A(iB) - (int)XGE_COLOR_GET_A(iA)) * iStep / iTotal;
	return XGE_COLOR_RGBA(r, g, b, a);
}

static void WritePixel(unsigned char* pPixels, int iWidth, int x, int y, uint32_t iColor)
{
	pPixels[((y * iWidth + x) * 4) + 0] = (unsigned char)XGE_COLOR_GET_R(iColor);
	pPixels[((y * iWidth + x) * 4) + 1] = (unsigned char)XGE_COLOR_GET_G(iColor);
	pPixels[((y * iWidth + x) * 4) + 2] = (unsigned char)XGE_COLOR_GET_B(iColor);
	pPixels[((y * iWidth + x) * 4) + 3] = (unsigned char)XGE_COLOR_GET_A(iColor);
}

static int CreateIconTexture(xge_texture pTexture)
{
	unsigned char arrPixels[32 * 32 * 4];
	int x;
	int y;
	uint32_t iColor;

	for ( y = 0; y < 32; y++ ) {
		for ( x = 0; x < 32; x++ ) {
			iColor = XGE_COLOR_RGBA(0, 0, 0, 0);
			if ( (x >= 7 && x <= 24 && y >= 7 && y <= 24) || (x >= 13 && x <= 18 && y >= 3 && y <= 28) || (y >= 13 && y <= 18 && x >= 3 && x <= 28) ) {
				iColor = BlendColor(XGE_COLOR_RGBA(255, 255, 255, 255), XGE_COLOR_RGBA(210, 236, 255, 255), y, 31);
			}
			if ( (x == 7 || x == 24 || y == 7 || y == 24) && (x >= 7 && x <= 24 && y >= 7 && y <= 24) ) {
				iColor = XGE_COLOR_RGBA(40, 104, 178, 255);
			}
			WritePixel(arrPixels, 32, x, y, iColor);
		}
	}
	return xgeTextureCreateRGBA(pTexture, 32, 32, arrPixels);
}

static int CreateButtonTexture(xge_texture pTexture, int iWidth, int iHeight, uint32_t iTop, uint32_t iBottom, uint32_t iBorder, int bRounded, int bPattern)
{
	unsigned char arrPixels[96 * 40 * 4];
	int x;
	int y;
	int r;
	int dx;
	int dy;
	int iInside;
	uint32_t iColor;

	r = iHeight / 2;
	for ( y = 0; y < iHeight; y++ ) {
		for ( x = 0; x < iWidth; x++ ) {
			iInside = 1;
			if ( bRounded ) {
				if ( x < r && y < r ) {
					dx = r - x - 1;
					dy = r - y - 1;
					iInside = (dx * dx + dy * dy) <= r * r;
				} else if ( x >= iWidth - r && y < r ) {
					dx = x - (iWidth - r);
					dy = r - y - 1;
					iInside = (dx * dx + dy * dy) <= r * r;
				} else if ( x < r && y >= iHeight - r ) {
					dx = r - x - 1;
					dy = y - (iHeight - r);
					iInside = (dx * dx + dy * dy) <= r * r;
				} else if ( x >= iWidth - r && y >= iHeight - r ) {
					dx = x - (iWidth - r);
					dy = y - (iHeight - r);
					iInside = (dx * dx + dy * dy) <= r * r;
				}
			}
			if ( !iInside ) {
				WritePixel(arrPixels, iWidth, x, y, XGE_COLOR_RGBA(0, 0, 0, 0));
				continue;
			}
			iColor = BlendColor(iTop, iBottom, y, iHeight - 1);
			if ( bPattern && ((x / 6) % 2 == 0) && x > r && x < iWidth - r ) {
				iColor = BlendColor(iColor, XGE_COLOR_RGBA(255, 255, 255, 255), 1, 6);
			}
			if ( x == 0 || y == 0 || x == iWidth - 1 || y == iHeight - 1 ) {
				iColor = iBorder;
			}
			WritePixel(arrPixels, iWidth, x, y, iColor);
		}
	}
	return xgeTextureCreateRGBA(pTexture, iWidth, iHeight, arrPixels);
}

static int CreateTextures(app_state_t* pApp)
{
	if ( CreateIconTexture(&pApp->tTexture[0]) != XGE_OK ||
	     CreateButtonTexture(&pApp->tTexture[1], 96, 40, XGE_COLOR_RGBA(80, 166, 235, 255), XGE_COLOR_RGBA(28, 112, 202, 255), XGE_COLOR_RGBA(20, 88, 165, 255), 0, 0) != XGE_OK ||
	     CreateButtonTexture(&pApp->tTexture[2], 64, 32, XGE_COLOR_RGBA(116, 205, 255, 255), XGE_COLOR_RGBA(30, 137, 222, 255), XGE_COLOR_RGBA(24, 92, 170, 255), 1, 0) != XGE_OK ||
	     CreateButtonTexture(&pApp->tTexture[3], 64, 32, XGE_COLOR_RGBA(167, 226, 154, 255), XGE_COLOR_RGBA(56, 154, 90, 255), XGE_COLOR_RGBA(34, 108, 66, 255), 1, 1) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_button font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_xui_widget NewWidget(void)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(42.0f));
		xgeXuiWidgetSetMarginPx(pWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	}
	return pWidget;
}

static int AddLabel(app_state_t* pApp, xge_xui_widget pRoot, const char* sText)
{
	xge_xui_widget pWidget;
	int iIndex;

	if ( pApp->iLabelCount >= LABEL_COUNT ) {
		return XGE_ERROR;
	}
	iIndex = pApp->iLabelCount++;
	pWidget = NewWidget();
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(28.0f));
	if ( xgeXuiLabelInit(&pApp->tLabel[iIndex], pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tLabel[iIndex], XGE_COLOR_RGBA(38, 48, 64, 255));
	xgeXuiLabelSetAlign(&pApp->tLabel[iIndex], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	xgeXuiWidgetAdd(pRoot, pWidget);
	return XGE_OK;
}

static xge_xui_button AddButton(app_state_t* pApp, xge_xui_widget pParent, const char* sText, uint32_t iNormal, uint32_t iHover, uint32_t iActive)
{
	xge_xui_widget pWidget;
	xge_xui_button pButton;

	if ( pApp->iButtonCount >= BUTTON_COUNT ) {
		return NULL;
	}
	pWidget = NewWidget();
	if ( pWidget == NULL ) {
		return NULL;
	}
	pButton = &pApp->tButton[pApp->iButtonCount];
	if ( xgeXuiButtonInit(pButton, &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return NULL;
	}
	xgeXuiButtonSetText(pButton, pApp->bFontReady ? &pApp->tFont : NULL, sText);
	xgeXuiButtonSetTextColor(pButton, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiButtonSetColors(pButton, iNormal, iHover, iActive, BlendColor(iHover, XGE_COLOR_RGBA(255, 255, 255, 255), 1, 3), XGE_COLOR_RGBA(150, 156, 166, 130));
	xgeXuiWidgetSetBorder(pWidget, 1.0f, BlendColor(iActive, XGE_COLOR_RGBA(0, 0, 0, 255), 1, 3));
	xgeXuiWidgetSetRadius(pWidget, 4.0f);
	xgeXuiWidgetSetPaddingPx(pWidget, 10.0f, 6.0f, 10.0f, 6.0f);
	xgeXuiWidgetAdd(pParent, pWidget);
	pApp->pButtonWidget[pApp->iButtonCount] = pWidget;
	pApp->iButtonCount++;
	return pButton;
}

static void ConfigureSelectedVisual(xge_xui_button pButton, uint32_t iSelected, uint32_t iBorder)
{
	if ( (pButton == NULL) || (pButton->pWidget == NULL) ) {
		return;
	}
	pButton->iColorChecked = iSelected;
	xgeXuiWidgetSetStateBackground(pButton->pWidget, XGE_XUI_STATE_CHECKED, iSelected);
	xgeXuiWidgetSetStateBorder(pButton->pWidget, XGE_XUI_STATE_CHECKED, 2.0f, iBorder);
}

static void SetButtonPatchState(xge_xui_button pButton, const xge_nine_patch_t* pPatch, int iState, uint32_t iColor)
{
	xge_nine_patch_t tStatePatch;

	if ( (pButton == NULL) || (pPatch == NULL) ) {
		return;
	}
	tStatePatch = *pPatch;
	xgeNinePatchSetColor(&tStatePatch, iColor);
	xgeXuiButtonSetPatch(pButton, iState, &tStatePatch);
}

static void SetButtonPatchStates(xge_xui_button pButton, const xge_nine_patch_t* pPatch)
{
	SetButtonPatchState(pButton, pPatch, XGE_XUI_STATE_NORMAL, XGE_COLOR_RGBA(235, 244, 255, 255));
	SetButtonPatchState(pButton, pPatch, XGE_XUI_STATE_HOVER, XGE_COLOR_RGBA(255, 246, 145, 255));
	SetButtonPatchState(pButton, pPatch, XGE_XUI_STATE_ACTIVE, XGE_COLOR_RGBA(92, 156, 255, 255));
	SetButtonPatchState(pButton, pPatch, XGE_XUI_STATE_CHECKED, XGE_COLOR_RGBA(255, 196, 58, 255));
	SetButtonPatchState(pButton, pPatch, XGE_XUI_STATE_DISABLED, XGE_COLOR_RGBA(150, 154, 160, 150));
}

static xge_xui_widget AddRow(xge_xui_widget pRoot, int iColumns, float fHeight)
{
	xge_xui_widget pRow;

	pRow = xgeXuiWidgetCreate();
	if ( pRow != NULL ) {
		xgeXuiWidgetSetLayout(pRow, XGE_XUI_LAYOUT_GRID);
		xgeXuiWidgetSetGrid(pRow, iColumns, fHeight, 10.0f, 10.0f);
		xgeXuiWidgetSetSize(pRow, xgeXuiSizePercent(100.0f), xgeXuiSizePx(fHeight));
		xgeXuiWidgetAdd(pRoot, pRow);
	}
	return pRow;
}

static int CreateUI(app_state_t* pApp)
{
	static const uint32_t arrNormal[7] = {
		XGE_COLOR_RGBA(212, 68, 72, 255), XGE_COLOR_RGBA(220, 123, 38, 255), XGE_COLOR_RGBA(206, 168, 48, 255),
		XGE_COLOR_RGBA(70, 164, 92, 255), XGE_COLOR_RGBA(34, 162, 174, 255), XGE_COLOR_RGBA(54, 126, 218, 255), XGE_COLOR_RGBA(138, 92, 202, 255)
	};
	static const char* arrName[7] = { "Red", "Orange", "Yellow", "Green", "Cyan", "Blue", "Purple" };
	xge_xui_widget pRoot;
	xge_xui_widget pRow;
	xge_xui_button pButton;
	xge_nine_patch_t tPatch;
	int i;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	XgeXuiDemoApplyRootPanel(pRoot);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pRoot, 22.0f, 18.0f, 22.0f, 18.0f);
	xgeXuiWidgetSetGap(pRoot, 9.0f);

	if ( AddLabel(pApp, pRoot, "Seven color schemes") != XGE_OK ) {
		return XGE_ERROR;
	}
	pRow = AddRow(pRoot, 7, 46.0f);
	if ( pRow == NULL ) {
		return XGE_ERROR;
	}
	for ( i = 0; i < 7; i++ ) {
		if ( AddButton(pApp, pRow, arrName[i], arrNormal[i], BlendColor(arrNormal[i], XGE_COLOR_RGBA(255, 255, 255, 255), 1, 5), BlendColor(arrNormal[i], XGE_COLOR_RGBA(0, 0, 0, 255), 1, 5)) == NULL ) {
			return XGE_ERROR;
		}
	}

	if ( AddLabel(pApp, pRoot, "Icon and text layout, selectable buttons") != XGE_OK ) {
		return XGE_ERROR;
	}
	pRow = AddRow(pRoot, 7, 58.0f);
	if ( pRow == NULL ) {
		return XGE_ERROR;
	}
	for ( i = 0; i < 4; i++ ) {
		pButton = AddButton(pApp, pRow, (i == 0) ? "Left" : (i == 1) ? "Right" : (i == 2) ? "Top" : "Bottom", XGE_COLOR_RGBA(56, 116, 190, 255), XGE_COLOR_RGBA(72, 140, 216, 255), XGE_COLOR_RGBA(36, 92, 164, 255));
		if ( pButton == NULL ) {
			return XGE_ERROR;
		}
		xgeXuiButtonSetIcon(pButton, &pApp->tTexture[0], (xge_rect_t){ 0.0f, 0.0f, 32.0f, 32.0f });
		xgeXuiButtonSetIconLayout(pButton, i, 18.0f, 5.0f);
	}
	pButton = AddButton(pApp, pRow, "Select", XGE_COLOR_RGBA(58, 130, 88, 255), XGE_COLOR_RGBA(70, 154, 104, 255), XGE_COLOR_RGBA(36, 104, 68, 255));
	if ( pButton == NULL ) {
		return XGE_ERROR;
	}
	xgeXuiButtonSetSelectable(pButton, 1);
	ConfigureSelectedVisual(pButton, XGE_COLOR_RGBA(18, 96, 62, 255), XGE_COLOR_RGBA(255, 210, 54, 255));
	xgeXuiButtonSetSelected(pButton, 1);
	pButton = AddButton(pApp, pRow, "Toggle", XGE_COLOR_RGBA(96, 112, 132, 255), XGE_COLOR_RGBA(116, 134, 158, 255), XGE_COLOR_RGBA(72, 88, 110, 255));
	if ( pButton == NULL ) {
		return XGE_ERROR;
	}
	xgeXuiButtonSetSelectable(pButton, 1);
	ConfigureSelectedVisual(pButton, XGE_COLOR_RGBA(32, 92, 176, 255), XGE_COLOR_RGBA(255, 210, 54, 255));
	pButton = AddButton(pApp, pRow, "Disabled", XGE_COLOR_RGBA(96, 112, 132, 255), XGE_COLOR_RGBA(116, 134, 158, 255), XGE_COLOR_RGBA(72, 88, 110, 255));
	if ( pButton == NULL ) {
		return XGE_ERROR;
	}
	xgeXuiButtonSetTextColor(pButton, XGE_COLOR_RGBA(232, 236, 242, 180));
	xgeXuiWidgetSetStateBackground(pButton->pWidget, XGE_XUI_STATE_DISABLED, XGE_COLOR_RGBA(109, 120, 136, 255));
	xgeXuiWidgetSetStateBorder(pButton->pWidget, XGE_XUI_STATE_DISABLED, 1.0f, XGE_COLOR_RGBA(58, 67, 80, 255));
	xgeXuiWidgetSetEnabled(pButton->pWidget, 0);

	if ( AddLabel(pApp, pRoot, "Image button, nine-patch button and badge") != XGE_OK ) {
		return XGE_ERROR;
	}
	pRow = AddRow(pRoot, 4, 66.0f);
	if ( pRow == NULL ) {
		return XGE_ERROR;
	}
	pButton = AddButton(pApp, pRow, "Image", XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeNinePatchInitSimple(&tPatch, &pApp->tTexture[1], (xge_rect_t){ 0.0f, 0.0f, 96.0f, 40.0f });
	SetButtonPatchStates(pButton, &tPatch);
	pButton = AddButton(pApp, pRow, "9-Slice", XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeNinePatchInit(&tPatch, &pApp->tTexture[2], (xge_rect_t){ 0.0f, 0.0f, 64.0f, 32.0f }, 0.25f, 0.25f, 0.75f, 0.75f);
	xgeNinePatchSetMode(&tPatch, XGE_NINE_PATCH_STRETCH);
	SetButtonPatchStates(pButton, &tPatch);
	pButton = AddButton(pApp, pRow, "9-Slice Tile", XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeNinePatchInit(&tPatch, &pApp->tTexture[3], (xge_rect_t){ 0.0f, 0.0f, 64.0f, 32.0f }, 0.25f, 0.25f, 0.75f, 0.75f);
	xgeNinePatchSetMode(&tPatch, XGE_NINE_PATCH_TILE);
	SetButtonPatchStates(pButton, &tPatch);
	pButton = AddButton(pApp, pRow, "Badge", XGE_COLOR_RGBA(62, 111, 188, 255), XGE_COLOR_RGBA(74, 134, 214, 255), XGE_COLOR_RGBA(38, 88, 160, 255));
	xgeXuiButtonSetBadgeVisible(pButton, 1);
	xgeXuiButtonSetBadgeSize(pButton, 12.0f);
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fRootW;
	float fRootH;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( iWidth == pApp->iLastWidth && iHeight == pApp->iLastHeight ) {
		return;
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	fRootW = (float)iWidth;
	fRootH = (float)iHeight;
	if ( fRootW < 920.0f ) {
		fRootW = 920.0f;
	}
	if ( fRootH < 520.0f ) {
		fRootH = 520.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	pApp->bCreateOK = (pApp->iButtonCount == BUTTON_COUNT) && (pApp->iLabelCount == LABEL_COUNT);
	pApp->bLayoutOK = xgeXuiRoot(&pApp->tXui)->tRect.fW >= 920.0f;
	pApp->bStateOK = (pApp->tButton[7].pIconTexture == &pApp->tTexture[0]) && (pApp->tButton[11].bSelectable == 1) && (pApp->tButton[11].bSelected == 1) && ((pApp->pButtonWidget[13]->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) && (pApp->tButton[14].arrHasPatch[0] == 1) && (pApp->tButton[17].bBadgeVisible == 1);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( CreateTextures(pApp) != XGE_OK || xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunChecks(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pScene->pUser;
	for ( i = 0; i < pApp->iButtonCount; i++ ) {
		xgeXuiButtonUnit(&pApp->tButton[i]);
	}
	for ( i = 0; i < pApp->iLabelCount; i++ ) {
		xgeXuiLabelUnit(&pApp->tLabel[i]);
	}
	xgeXuiUnit(&pApp->tXui);
	for ( i = 0; i < TEXTURE_COUNT; i++ ) {
		xgeTextureFree(&pApp->tTexture[i]);
	}
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	return XGE_OK;
}

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return XGE_OK;
	}
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	(void)fDelta;
	pApp = (app_state_t*)pScene->pUser;
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_button final-summary frames=%d create=%d layout=%d state=%d buttons=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->iButtonCount);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(229, 235, 244, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	static app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_BUTTON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 940;
	tDesc.iHeight = 560;
	tDesc.sTitle = "XUI Button";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	tApp.tScene.pUser = &tApp;
	tApp.tScene.onEnter = AppEnter;
	tApp.tScene.onLeave = AppLeave;
	tApp.tScene.onEvent = AppEvent;
	tApp.tScene.onUpdate = AppUpdate;
	tApp.tScene.onDraw = AppDraw;
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	iExitCode = xgeRun(NULL, NULL);
	xgeUnit();
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK) ? 0 : 3;
}
