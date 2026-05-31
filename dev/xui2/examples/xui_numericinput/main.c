#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	680
#define DEMO_TARGET_H	360
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define NUMERIC_COUNT	5
#define LABEL_COUNT	5

#define XUI_DEMO_KEY_LEFT_SHIFT		340
#define XUI_DEMO_KEY_LEFT_CTRL		341
#define XUI_DEMO_KEY_LEFT_ALT		342
#define XUI_DEMO_KEY_LEFT_SUPER		343
#define XUI_DEMO_KEY_RIGHT_SHIFT	344
#define XUI_DEMO_KEY_RIGHT_CTRL		345
#define XUI_DEMO_KEY_RIGHT_ALT		346
#define XUI_DEMO_KEY_RIGHT_SUPER	347

typedef struct xui_numericinput_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pLabel[LABEL_COUNT];
	xui_widget pNumeric[NUMERIC_COUNT];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int iChangeCount;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	int bKeyOK;
	int bSpinOK;
	int bWheelOK;
	int bErrorOK;
	int bNoSpinOK;
	int bMenuOK;
} xui_numericinput_demo_t;

static void __xuiNumericInputUsage(void)
{
	printf("usage: xui_numericinput [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiNumericInputParseArgs(xui_numericinput_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->fMaxSeconds = atof(argv[++i]);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__xuiNumericInputUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiNumericInputFindTtf(void)
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

static int __xuiNumericInputRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_numericinput_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_numericinput_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(235, 242, 250, 255));
	}
	if ( pDemo->tProxy.drawRoundRectFill != NULL ) {
		tPanel = (xui_rect_t){24.0f, 20.0f, tRect.fW - 48.0f, tRect.fH - 40.0f};
		(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tPanel, 6.0f, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static void __xuiNumericInputChanged(xui_widget pWidget, float fValue, void* pUser)
{
	xui_numericinput_demo_t* pDemo;

	(void)pWidget;
	(void)fValue;
	pDemo = (xui_numericinput_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iChangeCount++;
	}
}

static int __xuiNumericInputAddLabel(xui_numericinput_demo_t* pDemo, int iIndex, const char* sText)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= LABEL_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(55, 67, 86, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[iIndex] = pLabel;
	return XUI_OK;
}

static int __xuiNumericInputAddControl(xui_numericinput_demo_t* pDemo, int iIndex, float fValue, int bInteger)
{
	xui_numeric_input_desc_t tDesc;
	xui_widget pNumeric;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= NUMERIC_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.sPlaceholder = "0";
	tDesc.fMin = 0.0f;
	tDesc.fMax = 100.0f;
	tDesc.fStep = bInteger ? 1.0f : 0.25f;
	tDesc.fValue = fValue;
	tDesc.iPrecision = bInteger ? 0 : 2;
	tDesc.bInteger = bInteger;
	iRet = xuiNumericInputCreate(pDemo->pContext, &pNumeric, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiNumericInputSetChange(pNumeric, __xuiNumericInputChanged, pDemo);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pNumeric);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pNumeric);
		return iRet;
	}
	pDemo->pNumeric[iIndex] = pNumeric;
	return XUI_OK;
}

static int __xuiNumericInputCreateUi(xui_numericinput_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiNumericInputRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiNumericInputAddLabel(pDemo, 0, "Default value") != XUI_OK ||
	     __xuiNumericInputAddControl(pDemo, 0, 12.5f, 0) != XUI_OK ||
	     __xuiNumericInputAddLabel(pDemo, 1, "Integer") != XUI_OK ||
	     __xuiNumericInputAddControl(pDemo, 1, 42.0f, 1) != XUI_OK ||
	     __xuiNumericInputAddLabel(pDemo, 2, "Green accent") != XUI_OK ||
	     __xuiNumericInputAddControl(pDemo, 2, 64.25f, 0) != XUI_OK ||
	     __xuiNumericInputAddLabel(pDemo, 3, "Readonly") != XUI_OK ||
	     __xuiNumericInputAddControl(pDemo, 3, 8.0f, 0) != XUI_OK ||
	     __xuiNumericInputAddLabel(pDemo, 4, "No spinner") != XUI_OK ||
	     __xuiNumericInputAddControl(pDemo, 4, 33.0f, 0) != XUI_OK ) {
		return XUI_ERROR;
	}
	(void)xuiNumericInputSetSpinnerColors(pDemo->pNumeric[2],
		XUI_COLOR_RGBA(238, 249, 245, 255),
		XUI_COLOR_RGBA(220, 242, 234, 255),
		XUI_COLOR_RGBA(196, 228, 216, 255),
		XUI_COLOR_RGBA(180, 214, 201, 255),
		XUI_COLOR_RGBA(32, 132, 94, 255),
		XUI_COLOR_RGBA(140, 170, 158, 150));
	(void)xuiNumericInputSetReadonly(pDemo->pNumeric[3], 1);
	(void)xuiNumericInputSetSpinnerVisible(pDemo->pNumeric[4], 0);
	(void)xuiNumericInputSetMenuTitle(pDemo->pNumeric[0], XUI_INPUT_MENU_COPY, "Copy Number");
	return XUI_OK;
}

static void __xuiNumericInputLayout(xui_numericinput_demo_t* pDemo)
{
	int i;
	float fY;

	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	for ( i = 0; i < NUMERIC_COUNT; i++ ) {
		fY = 52.0f + (float)i * 48.0f;
		(void)xuiWidgetSetRect(pDemo->pLabel[i], (xui_rect_t){48.0f, fY, 160.0f, 28.0f});
		(void)xuiWidgetSetRect(pDemo->pNumeric[i], (xui_rect_t){230.0f, fY, 180.0f, 30.0f});
	}
	(void)xuiLayout(pDemo->pContext);
}

static uint32_t __xuiNumericInputReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiNumericInputSendButtonTransitions(xui_numericinput_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
{
	int iRet;

	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iPressed & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiNumericInputMapKey(int iKey)
{
	switch ( iKey ) {
	case XGE_KEY_ENTER: return XUI_KEY_ENTER;
	case XGE_KEY_TAB: return XUI_KEY_TAB;
	case XGE_KEY_BACKSPACE: return 8;
	case XGE_KEY_DELETE: return 46;
	case XGE_KEY_LEFT: return XUI_KEY_LEFT;
	case XGE_KEY_RIGHT: return XUI_KEY_RIGHT;
	case XGE_KEY_UP: return XUI_KEY_UP;
	case XGE_KEY_DOWN: return XUI_KEY_DOWN;
	case XGE_KEY_HOME: return XUI_KEY_HOME;
	case XGE_KEY_END: return XUI_KEY_END;
	case XGE_KEY_MENU: return XUI_KEY_CONTEXT_MENU;
	default: return 0;
	}
}

static uint32_t __xuiNumericInputReadModifiers(void)
{
	uint32_t iModifiers;

	iModifiers = 0;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_SHIFT) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_SHIFT) ) iModifiers |= XUI_MOD_SHIFT;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_CTRL) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_CTRL) ) iModifiers |= XUI_MOD_CTRL;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_ALT) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_ALT) ) iModifiers |= XUI_MOD_ALT;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_SUPER) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_SUPER) ) iModifiers |= XUI_MOD_SUPER;
	return iModifiers;
}

static int __xuiNumericInputSendKeys(xui_numericinput_demo_t* pDemo)
{
	static const int arrKeys[] = {
		'A',
		'C',
		'V',
		'X',
		'Z',
		XGE_KEY_ENTER,
		XGE_KEY_TAB,
		XGE_KEY_BACKSPACE,
		XGE_KEY_DELETE,
		XGE_KEY_LEFT,
		XGE_KEY_RIGHT,
		XGE_KEY_UP,
		XGE_KEY_DOWN,
		XGE_KEY_HOME,
		XGE_KEY_END,
		XGE_KEY_MENU
	};
	uint32_t iText;
	uint32_t iModifiers;
	int iKey;
	int iRet;
	int i;

	iModifiers = __xuiNumericInputReadModifiers();
	iRet = xuiInputSetModifiers(pDemo->pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < (int)(sizeof(arrKeys) / sizeof(arrKeys[0])); i++ ) {
		iKey = __xuiNumericInputMapKey(arrKeys[i]);
		if ( iKey == 0 ) iKey = arrKeys[i];
		if ( xgeKeyPressed(arrKeys[i]) ) {
			iRet = xuiInputKeyDown(pDemo->pContext, iKey, iModifiers);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( xgeKeyReleased(arrKeys[i]) ) {
			iRet = xuiInputKeyUp(pDemo->pContext, iKey, iModifiers);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	while ( (iText = xgeTextGet()) != 0 ) {
		if ( (iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT)) == 0u ) {
			iRet = xuiInputText(pDemo->pContext, iText);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiNumericInputHandleInput(xui_numericinput_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiNumericInputReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( fWheelX != 0.0f || fWheelY != 0.0f ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiNumericInputSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiNumericInputSendKeys(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiNumericInputAutoClick(xui_numericinput_demo_t* pDemo, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static int __xuiNumericInputNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) fD = -fD;
	return fD < 0.02f;
}

static void __xuiNumericInputRunChecks(xui_numericinput_demo_t* pDemo, int bExerciseInput)
{
	xui_widget pInput;
	xui_widget pMenu;
	xui_rect_t tWorld;
	xui_rect_t tButton;
	xui_rect_t tSpinner;
	float fBefore;
	int i;

	pDemo->bCreateOK = (pDemo->pRoot != NULL);
	for ( i = 0; i < NUMERIC_COUNT; i++ ) {
		if ( pDemo->pNumeric[i] == NULL ) pDemo->bCreateOK = 0;
	}
	tSpinner = xuiNumericInputGetSpinnerRect(pDemo->pNumeric[0]);
	pDemo->bLayoutOK = (xuiWidgetGetRect(pDemo->pNumeric[0]).fW > 120.0f) && (tSpinner.fW >= 20.0f);
	pDemo->bStateOK = __xuiNumericInputNear(xuiNumericInputGetValue(pDemo->pNumeric[0]), 12.5f) ||
	                  (xuiNumericInputGetChangeCount(pDemo->pNumeric[0]) > 0);
	pDemo->bNoSpinOK = (xuiNumericInputGetSpinnerRect(pDemo->pNumeric[4]).fW == 0.0f);
	pMenu = xuiNumericInputGetMenuWidget(pDemo->pNumeric[0]);
	pDemo->bMenuOK = (pMenu != NULL) && (strcmp(xuiNumericInputGetMenuTitle(pDemo->pNumeric[0], XUI_INPUT_MENU_COPY), "Copy Number") == 0);
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bLayoutOK ) {
		pInput = xuiNumericInputGetInputWidget(pDemo->pNumeric[0]);
		(void)xuiSetFocusWidget(pDemo->pContext, pInput);
		fBefore = xuiNumericInputGetValue(pDemo->pNumeric[0]);
		(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_UP, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bKeyOK = xuiNumericInputGetValue(pDemo->pNumeric[0]) > fBefore;

		tWorld = xuiWidgetGetWorldRect(pDemo->pNumeric[0]);
		tButton = xuiNumericInputGetButtonRect(pDemo->pNumeric[0], XUI_NUMERIC_INPUT_BUTTON_UP);
		fBefore = xuiNumericInputGetValue(pDemo->pNumeric[0]);
		(void)__xuiNumericInputAutoClick(pDemo, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f);
		pDemo->bSpinOK = xuiNumericInputGetValue(pDemo->pNumeric[0]) > fBefore;

		fBefore = xuiNumericInputGetValue(pDemo->pNumeric[0]);
		(void)xuiInputPointerWheel(pDemo->pContext, tWorld.fX + 20.0f, tWorld.fY + 15.0f, 0.0f, -1.0f, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bWheelOK = xuiNumericInputGetValue(pDemo->pNumeric[0]) < fBefore;

		(void)xuiNumericInputSetText(pDemo->pNumeric[0], "bad");
		(void)xuiNumericInputCommit(pDemo->pNumeric[0]);
		pDemo->bErrorOK = xuiNumericInputGetError(pDemo->pNumeric[0]) != 0;
		(void)xuiNumericInputSetText(pDemo->pNumeric[0], "18.50");
		(void)xuiNumericInputCommit(pDemo->pNumeric[0]);
		pDemo->bExerciseDone = 1;
	}
}

static int __xuiNumericInputCreateAssets(xui_numericinput_demo_t* pDemo)
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
	sFontPath = __xuiNumericInputFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiNumericInputCreateUi(pDemo);
}

static void __xuiNumericInputDestroyAssets(xui_numericinput_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
		pDemo->pFont = NULL;
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
		pDemo->pTarget = NULL;
	}
}

static int __xuiNumericInputFrame(void* pUser)
{
	xui_numericinput_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_numericinput_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiNumericInputHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiNumericInputLayout(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiNumericInputRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(22, 26, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		memset(&tCacheStats, 0, sizeof(tCacheStats));
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		(void)xuiGetCacheStats(pDemo->pContext, &tCacheStats);
		printf("xui_numericinput final-summary frames=%d create=%d layout=%d state=%d key=%d spin=%d wheel=%d error=%d noSpin=%d menu=%d changes=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bStateOK, pDemo->bKeyOK, pDemo->bSpinOK,
			pDemo->bWheelOK, pDemo->bErrorOK, pDemo->bNoSpinOK, pDemo->bMenuOK, pDemo->iChangeCount,
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_numericinput_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiNumericInputParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiNumericInputUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI NumericInput";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_numericinput: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiNumericInputCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_numericinput: create assets failed: %d\n", iRet);
		__xuiNumericInputDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiNumericInputFrame, &tDemo);
	__xuiNumericInputDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK &&
	        tDemo.bCreateOK &&
	        tDemo.bLayoutOK &&
	        tDemo.bStateOK &&
	        tDemo.bKeyOK &&
	        tDemo.bSpinOK &&
	        tDemo.bWheelOK &&
	        tDemo.bErrorOK &&
	        tDemo.bNoSpinOK &&
	        tDemo.bMenuOK) ? 0 : 1;
}
