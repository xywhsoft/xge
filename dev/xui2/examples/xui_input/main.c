#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	640
#define DEMO_TARGET_H	420
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define INPUT_COUNT	4

#define XUI_DEMO_KEY_LEFT_SHIFT		340
#define XUI_DEMO_KEY_LEFT_CTRL		341
#define XUI_DEMO_KEY_LEFT_ALT		342
#define XUI_DEMO_KEY_LEFT_SUPER		343
#define XUI_DEMO_KEY_RIGHT_SHIFT	344
#define XUI_DEMO_KEY_RIGHT_CTRL		345
#define XUI_DEMO_KEY_RIGHT_ALT		346
#define XUI_DEMO_KEY_RIGHT_SUPER	347

typedef struct xui_input_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pStatus;
	xui_widget pInput[INPUT_COUNT];
	xui_input_decoration pSearchDecoration;
	xui_input_decoration pClearDecoration;
	xui_input_decoration pEyeDecoration;
	xui_input_decoration pGoDecoration;
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
	int bMenuOK;
	int bInputOK;
	int bHotkeyOK;
	int bDecorationOK;
	int iDecorationClickCount;
} xui_input_demo_t;

static void __xuiInputUsage(void)
{
	printf("usage: xui_input [--frames N] [--seconds N]\n");
}

static int __xuiInputParseArgs(xui_input_demo_t* pDemo, int argc, char** argv)
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
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			__xuiInputUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiInputFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
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

static int __xuiInputRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_input_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_input_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(236, 243, 251, 255));
	}
	if ( pDemo->tProxy.drawRoundRectFill != NULL ) {
		tPanel = (xui_rect_t){26.0f, 20.0f, tRect.fW - 52.0f, tRect.fH - 40.0f};
		(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tPanel, 6.0f, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static int __xuiInputSetFillLayout(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetDock(pWidget, XUI_DOCK_FILL);
	return iRet;
}

static int __xuiInputSetFixedHeight(xui_widget pWidget, float fHeight)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetPreferredSize(pWidget, (xui_vec2_t){0.0f, fHeight});
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	return iRet;
}

static int __xuiInputAddLabel(xui_input_demo_t* pDemo, const char* sText, xui_widget* ppLabel)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(76, 89, 109, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)__xuiInputSetFixedHeight(pLabel, 20.0f);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	if ( ppLabel != NULL ) *ppLabel = pLabel;
	return XUI_OK;
}

static void __xuiInputChanged(xui_widget pWidget, const char* sText, void* pUser)
{
	xui_input_demo_t* pDemo;
	char sStatus[160];

	(void)pWidget;
	pDemo = (xui_input_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iChangeCount++;
	if ( sText == NULL ) sText = "";
	snprintf(sStatus, sizeof(sStatus), "变更 %d: %s", pDemo->iChangeCount, sText);
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sStatus);
	}
}

static void __xuiInputEyeClick(xui_widget pWidget, xui_input_decoration pDecoration, void* pUser)
{
	xui_input_demo_t* pDemo;
	int bPassword;

	(void)pDecoration;
	pDemo = (xui_input_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDemo == NULL) ) return;
	bPassword = !xuiInputIsPassword(pWidget);
	(void)xuiInputSetPassword(pWidget, bPassword);
	pDemo->iDecorationClickCount++;
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, bPassword ? "password hidden" : "password visible");
	}
}

static void __xuiInputGoClick(xui_widget pWidget, xui_input_decoration pDecoration, void* pUser)
{
	xui_input_demo_t* pDemo;
	char sStatus[128];

	(void)pWidget;
	(void)pDecoration;
	pDemo = (xui_input_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iDecorationClickCount++;
	snprintf(sStatus, sizeof(sStatus), "GO click %d", pDemo->iDecorationClickCount);
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sStatus);
	}
}

static int __xuiInputAddInput(xui_input_demo_t* pDemo, int iIndex, const char* sText, const char* sPlaceholder)
{
	xui_input_desc_t tDesc;
	xui_widget pInput;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= INPUT_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.sPlaceholder = sPlaceholder;
	tDesc.pFont = pDemo->pFont;
	tDesc.iMaxLength = 64;
	tDesc.iTextAlign = XUI_INPUT_ALIGN_LEFT;
	iRet = xuiInputCreate(pDemo->pContext, &pInput, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)__xuiInputSetFixedHeight(pInput, 32.0f);
	(void)xuiInputSetChange(pInput, __xuiInputChanged, pDemo);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pInput);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pInput);
		return iRet;
	}
	pDemo->pInput[iIndex] = pInput;
	return XUI_OK;
}

static int __xuiInputAddIconDecoration(xui_widget pInput, int iSide, int iIcon, xui_input_decoration_click_proc onClick, void* pUser, xui_input_decoration* ppDecoration)
{
	xui_input_decoration_desc_t tDesc;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iKind = XUI_INPUT_DECORATION_ICON;
	tDesc.iVisibleMode = XUI_INPUT_DECORATION_VISIBLE_ALWAYS;
	tDesc.iIcon = iIcon;
	tDesc.onClick = onClick;
	tDesc.pUser = pUser;
	return xuiInputDecorationAdd(pInput, iSide, ppDecoration, &tDesc);
}

static int __xuiInputAddClearDecoration(xui_widget pInput, xui_input_decoration* ppDecoration)
{
	xui_input_decoration_desc_t tDesc;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iKind = XUI_INPUT_DECORATION_CLEAR;
	tDesc.iVisibleMode = XUI_INPUT_DECORATION_VISIBLE_NOT_EMPTY;
	return xuiInputDecorationAdd(pInput, XUI_INPUT_DECORATION_SIDE_TRAILING, ppDecoration, &tDesc);
}

static int __xuiInputAddTextDecoration(xui_widget pInput, const char* sText, xui_input_decoration_click_proc onClick, void* pUser, xui_input_decoration* ppDecoration)
{
	xui_input_decoration_desc_t tDesc;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iKind = XUI_INPUT_DECORATION_TEXT;
	tDesc.iVisibleMode = XUI_INPUT_DECORATION_VISIBLE_ALWAYS;
	tDesc.sText = sText;
	tDesc.fWidth = 42.0f;
	tDesc.fPadding = 8.0f;
	tDesc.onClick = onClick;
	tDesc.pUser = pUser;
	return xuiInputDecorationAdd(pInput, XUI_INPUT_DECORATION_SIDE_TRAILING, ppDecoration, &tDesc);
}

static int __xuiInputCreateUi(xui_input_demo_t* pDemo)
{
	xui_thickness_t tPadding;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetGap(pDemo->pRoot, 8.0f);
	tPadding = (xui_thickness_t){44.0f, 28.0f, 44.0f, 28.0f};
	(void)xuiWidgetSetPadding(pDemo->pRoot, tPadding);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiInputRootRender, pDemo);
	(void)__xuiInputSetFillLayout(pDemo->pRoot);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	if ( __xuiInputAddLabel(pDemo, "默认输入", NULL) != XUI_OK ||
	     __xuiInputAddInput(pDemo, 0, "XUI2 Input", "请输入文本") != XUI_OK ||
	     __xuiInputAddLabel(pDemo, "密码", NULL) != XUI_OK ||
	     __xuiInputAddInput(pDemo, 1, "secret", "请输入密码") != XUI_OK ||
	     __xuiInputAddLabel(pDemo, "只读", NULL) != XUI_OK ||
	     __xuiInputAddInput(pDemo, 2, "readonly value", "") != XUI_OK ||
	     __xuiInputAddLabel(pDemo, "错误状态", NULL) != XUI_OK ||
	     __xuiInputAddInput(pDemo, 3, "invalid value", "必填") != XUI_OK ||
	     __xuiInputAddLabel(pDemo, "等待输入", &pDemo->pStatus) != XUI_OK ) {
		return XUI_ERROR;
	}
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, "等待输入：尚未输入");
	}
	(void)xuiInputSetPassword(pDemo->pInput[1], 1);
	(void)xuiInputSetReadonly(pDemo->pInput[2], 1);
	(void)xuiInputSetError(pDemo->pInput[3], 1);
	if ( __xuiInputAddIconDecoration(pDemo->pInput[0], XUI_INPUT_DECORATION_SIDE_LEADING, XUI_INPUT_ICON_SEARCH, NULL, NULL, &pDemo->pSearchDecoration) != XUI_OK ||
	     __xuiInputAddClearDecoration(pDemo->pInput[0], &pDemo->pClearDecoration) != XUI_OK ||
	     __xuiInputAddIconDecoration(pDemo->pInput[1], XUI_INPUT_DECORATION_SIDE_LEADING, XUI_INPUT_ICON_LOCK, NULL, NULL, NULL) != XUI_OK ||
	     __xuiInputAddIconDecoration(pDemo->pInput[1], XUI_INPUT_DECORATION_SIDE_TRAILING, XUI_INPUT_ICON_EYE, __xuiInputEyeClick, pDemo, &pDemo->pEyeDecoration) != XUI_OK ||
	     __xuiInputAddTextDecoration(pDemo->pInput[3], "GO", __xuiInputGoClick, pDemo, &pDemo->pGoDecoration) != XUI_OK ) {
		return XUI_ERROR;
	}
	(void)xuiInputSetMenuTitle(pDemo->pInput[3], XUI_INPUT_MENU_COPY, "复制文本");
	(void)xuiInputSetMenuTitle(pDemo->pInput[3], XUI_INPUT_MENU_PASTE, "粘贴文本");
	(void)xuiInputSetColors(pDemo->pInput[3],
		XUI_COLOR_RGBA(255, 250, 250, 255),
		XUI_COLOR_RGBA(74, 35, 35, 255),
		XUI_COLOR_RGBA(220, 72, 72, 255),
		XUI_COLOR_RGBA(220, 72, 72, 255));
	return XUI_OK;
}

static void __xuiInputLayout(xui_input_demo_t* pDemo)
{
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiLayout(pDemo->pContext);
}

static uint32_t __xuiInputReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiInputSendButtonTransitions(xui_input_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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
	if ( (iPressed & XUI_POINTER_BUTTON_MIDDLE) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_MIDDLE, iButtons);
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
	if ( (iReleased & XUI_POINTER_BUTTON_MIDDLE) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_MIDDLE, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiInputMapKey(int iKey)
{
	switch ( iKey ) {
	case XGE_KEY_ENTER: return XUI_KEY_ENTER;
	case XGE_KEY_TAB: return XUI_KEY_TAB;
	case XGE_KEY_BACKSPACE: return 8;
	case XGE_KEY_DELETE: return 46;
	case XGE_KEY_LEFT: return XUI_KEY_LEFT;
	case XGE_KEY_RIGHT: return XUI_KEY_RIGHT;
	case XGE_KEY_HOME: return XUI_KEY_HOME;
	case XGE_KEY_END: return XUI_KEY_END;
	case XGE_KEY_MENU: return XUI_KEY_CONTEXT_MENU;
	default: return 0;
	}
}

static uint32_t __xuiInputReadModifiers(void)
{
	uint32_t iModifiers;

	iModifiers = 0;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_SHIFT) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_SHIFT) ) iModifiers |= XUI_MOD_SHIFT;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_CTRL) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_CTRL) ) iModifiers |= XUI_MOD_CTRL;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_ALT) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_ALT) ) iModifiers |= XUI_MOD_ALT;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_SUPER) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_SUPER) ) iModifiers |= XUI_MOD_SUPER;
	return iModifiers;
}

static int __xuiInputSendKeys(xui_input_demo_t* pDemo)
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
		XGE_KEY_HOME,
		XGE_KEY_END,
		XGE_KEY_MENU
	};
	uint32_t iText;
	uint32_t iModifiers;
	int iKey;
	int iRet;
	int i;

	iModifiers = __xuiInputReadModifiers();
	iRet = xuiInputSetModifiers(pDemo->pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	for ( i = 0; i < (int)(sizeof(arrKeys) / sizeof(arrKeys[0])); i++ ) {
		iKey = __xuiInputMapKey(arrKeys[i]);
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

static int __xuiInputDispatchKey(xui_input_demo_t* pDemo, int iKey, uint32_t iModifiers)
{
	int iRet;

	iRet = xuiInputSetModifiers(pDemo->pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputKeyDown(pDemo->pContext, iKey, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputKeyUp(pDemo->pContext, iKey, 0u);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	return xuiInputSetModifiers(pDemo->pContext, 0u);
}

static int __xuiInputHandleInput(xui_input_demo_t* pDemo)
{
	float fX;
	float fY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	xgeMouseGet(&fX, &fY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiInputReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiInputSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiInputSendKeys(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiInputRightClick(xui_input_demo_t* pDemo, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, XUI_POINTER_BUTTON_RIGHT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static void __xuiInputRunChecks(xui_input_demo_t* pDemo, int bExerciseInput)
{
	xui_widget pMenu;
	xui_rect_t tRect;
	xui_rect_t tSearchRect;
	xui_rect_t tClearRect;
	xui_rect_t tEyeRect;
	xui_rect_t tGoRect;
	const xui_menu_item_t* pItem;
	int bInput0NotEmpty;
	int iStart;
	int iEnd;
	int i;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pStatus != NULL);
	for ( i = 0; i < INPUT_COUNT; i++ ) {
		if ( pDemo->pInput[i] == NULL ) pDemo->bCreateOK = 0;
	}
	tRect = xuiWidgetGetRect(pDemo->pInput[0]);
	pDemo->bLayoutOK = (tRect.fW > 240.0f) && (tRect.fH >= 30.0f);
	pDemo->bStateOK = xuiInputIsPassword(pDemo->pInput[1]) &&
	                  xuiInputIsReadonly(pDemo->pInput[2]) &&
	                  xuiInputGetError(pDemo->pInput[3]) &&
	                  (strcmp(xuiInputGetMenuTitle(pDemo->pInput[3], XUI_INPUT_MENU_COPY), "复制文本") == 0);
	tSearchRect = xuiInputDecorationGetRect(pDemo->pInput[0], pDemo->pSearchDecoration);
	tClearRect = xuiInputDecorationGetRect(pDemo->pInput[0], pDemo->pClearDecoration);
	tEyeRect = xuiInputDecorationGetRect(pDemo->pInput[1], pDemo->pEyeDecoration);
	tGoRect = xuiInputDecorationGetRect(pDemo->pInput[3], pDemo->pGoDecoration);
	bInput0NotEmpty = (xuiInputGetText(pDemo->pInput[0])[0] != '\0');
	pDemo->bDecorationOK = (tSearchRect.fW > 0.0f) &&
	                       (bInput0NotEmpty ? (tClearRect.fW > 0.0f) : (tClearRect.fW == 0.0f)) &&
	                       (tEyeRect.fW > 0.0f) && (tGoRect.fW > 0.0f);
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bLayoutOK ) {
		tRect = xuiWidgetGetWorldRect(pDemo->pInput[0]);
		(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pInput[0]);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputText(pDemo->pContext, '!');
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputSetSelection(pDemo->pInput[0], 0, (int)strlen(xuiInputGetText(pDemo->pInput[0])));
		(void)__xuiInputRightClick(pDemo, tRect.fX + 16.0f, tRect.fY + tRect.fH * 0.5f);
		pMenu = xuiInputGetMenuWidget(pDemo->pInput[0]);
		if ( pMenu != NULL ) {
			(void)xuiMenuClose(pMenu);
		}
		(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pInput[0]);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputSetText(pDemo->pInput[0], "alpha beta");
		(void)xuiInputSetSelection(pDemo->pInput[0], (int)strlen(xuiInputGetText(pDemo->pInput[0])), (int)strlen(xuiInputGetText(pDemo->pInput[0])));
		pDemo->bHotkeyOK = (__xuiInputDispatchKey(pDemo, XUI_KEY_LEFT, XUI_MOD_CTRL) == XUI_OK);
		(void)xuiInputGetSelection(pDemo->pInput[0], &iStart, &iEnd);
		pDemo->bHotkeyOK = pDemo->bHotkeyOK && (iStart == 6) && (iEnd == 6);
		pDemo->bHotkeyOK = pDemo->bHotkeyOK && (__xuiInputDispatchKey(pDemo, XUI_KEY_RIGHT, XUI_MOD_CTRL | XUI_MOD_SHIFT) == XUI_OK);
		(void)xuiInputGetSelection(pDemo->pInput[0], &iStart, &iEnd);
		pDemo->bHotkeyOK = pDemo->bHotkeyOK && (iStart == 6) && (iEnd == 10);
		pDemo->bHotkeyOK = pDemo->bHotkeyOK && (__xuiInputDispatchKey(pDemo, 'A', XUI_MOD_CTRL) == XUI_OK);
		(void)xuiInputGetSelection(pDemo->pInput[0], &iStart, &iEnd);
		pDemo->bHotkeyOK = pDemo->bHotkeyOK && (iStart == 0) && (iEnd == (int)strlen(xuiInputGetText(pDemo->pInput[0])));
		pDemo->bHotkeyOK = pDemo->bHotkeyOK && (__xuiInputDispatchKey(pDemo, 'X', XUI_MOD_CTRL) == XUI_OK);
		pDemo->bHotkeyOK = pDemo->bHotkeyOK && (strcmp(xuiInputGetText(pDemo->pInput[0]), "") == 0);
		pDemo->bExerciseDone = 1;
	}
	pMenu = xuiInputGetMenuWidget(pDemo->pInput[0]);
	pItem = (pMenu != NULL) ? xuiMenuGetItem(pMenu, 1) : NULL;
	pDemo->bMenuOK = (pMenu != NULL) && (xuiMenuGetItemCount(pMenu) == 8) &&
	                 (pItem != NULL) && (pItem->iType == XUI_MENU_ITEM_SEPARATOR);
	pDemo->bInputOK = !bExerciseInput || ((pDemo->iChangeCount > 0) && pDemo->bHotkeyOK);
}

static int __xuiInputCreateAssets(xui_input_demo_t* pDemo)
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
	sFontPath = __xuiInputFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiInputCreateUi(pDemo);
}

static void __xuiInputDestroyAssets(xui_input_demo_t* pDemo)
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

static int __xuiInputFrame(void* pUser)
{
	xui_input_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_input_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiInputHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiInputLayout(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiInputRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(22, 26, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		memset(&tCacheStats, 0, sizeof(tCacheStats));
		tStats.iSize = sizeof(tStats);
		tCacheStats.iSize = sizeof(tCacheStats);
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		(void)xuiGetCacheStats(pDemo->pContext, &tCacheStats);
		printf("xui_input final-summary frames=%d create=%d layout=%d state=%d menu=%d input=%d hotkey=%d decoration=%d changes=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bStateOK, pDemo->bMenuOK, pDemo->bInputOK, pDemo->bHotkeyOK, pDemo->bDecorationOK, pDemo->iChangeCount,
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_input_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiInputParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiInputUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Input";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_input: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiInputCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_input: create assets failed: %d\n", iRet);
		__xuiInputDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiInputFrame, &tDemo);
	__xuiInputDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bStateOK && tDemo.bMenuOK && tDemo.bInputOK && tDemo.bDecorationOK) ? 0 : 1;
}
