#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	680
#define DEMO_TARGET_H	520
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f

#define XUI_DEMO_KEY_LEFT_SHIFT		340
#define XUI_DEMO_KEY_LEFT_CTRL		341
#define XUI_DEMO_KEY_LEFT_ALT		342
#define XUI_DEMO_KEY_LEFT_SUPER		343
#define XUI_DEMO_KEY_RIGHT_SHIFT	344
#define XUI_DEMO_KEY_RIGHT_CTRL		345
#define XUI_DEMO_KEY_RIGHT_ALT		346
#define XUI_DEMO_KEY_RIGHT_SUPER	347

typedef struct xui_textedit_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pTitle;
	xui_widget pTextEdit;
	xui_widget pPlainTextEdit;
	xui_widget pStatus;
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
	int bPlainOK;
	int bMenuOK;
	int bInputOK;
} xui_textedit_demo_t;

static void __xuiTextEditUsage(void)
{
	printf("usage: xui_textedit [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiTextEditParseArgs(xui_textedit_demo_t* pDemo, int argc, char** argv)
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
			__xuiTextEditUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiTextEditFindTtf(void)
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

static int __xuiTextEditRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_textedit_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_textedit_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(236, 243, 251, 255));
	}
	if ( pDemo->tProxy.drawRoundRectFill != NULL ) {
		tPanel = (xui_rect_t){24.0f, 20.0f, tRect.fW - 48.0f, tRect.fH - 40.0f};
		(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tPanel, 6.0f, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static int __xuiTextEditAddLabel(xui_textedit_demo_t* pDemo, xui_widget* ppLabel, const char* sText, xui_rect_t tRect, uint32_t iColor)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pLabel, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	if ( ppLabel != NULL ) *ppLabel = pLabel;
	return XUI_OK;
}

static void __xuiTextEditChanged(xui_widget pWidget, const char* sText, void* pUser)
{
	xui_textedit_demo_t* pDemo;
	char sStatus[180];
	int iLen;

	pDemo = (xui_textedit_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iChangeCount++;
	iLen = (sText != NULL) ? (int)strlen(sText) : 0;
	snprintf(sStatus, sizeof(sStatus), "变更 %d  字节 %d  行数 %d", pDemo->iChangeCount, iLen, xuiTextEditGetLineCount(pWidget));
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sStatus);
	}
}

static int __xuiTextEditCreateUi(xui_textedit_demo_t* pDemo)
{
	xui_text_edit_desc_t tDesc;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiTextEditRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiTextEditAddLabel(pDemo, &pDemo->pTitle, "XUI2 TextEdit", (xui_rect_t){38.0f, 28.0f, 320.0f, 26.0f}, XUI_COLOR_RGBA(54, 69, 89, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiTextEditAddLabel(pDemo, NULL, "显示行号", (xui_rect_t){38.0f, 58.0f, DEMO_TARGET_W - 76.0f, 20.0f}, XUI_COLOR_RGBA(83, 96, 116, 255));
	if ( iRet != XUI_OK ) return iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText =
		"XUI2 TextEdit\n"
		"支持多行输入、选择、剪贴板、撤销、重做、软换行和右键菜单。\n"
		"右键菜单复用 Input 的命令和默认中文标题：撤销 / 剪切 / 复制 / 粘贴 / 删除 / 全选。\n"
		"\n"
		"滚轮可以滚动文本；Enter 插入新行；Ctrl+Z / Ctrl+Y 用于撤销和重做。";
	tDesc.sPlaceholder = "请输入多行文本";
	tDesc.pFont = pDemo->pFont;
	tDesc.iMaxLength = 4096;
	tDesc.bWordWrap = 1;
	tDesc.bLineNumbers = 1;
	tDesc.fLineNumberWidth = 46.0f;
	iRet = xuiTextEditCreate(pDemo->pContext, &pDemo->pTextEdit, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pTextEdit, (xui_rect_t){38.0f, 84.0f, DEMO_TARGET_W - 76.0f, 170.0f});
	(void)xuiTextEditSetChange(pDemo->pTextEdit, __xuiTextEditChanged, pDemo);
	(void)xuiTextEditSetMenuTitle(pDemo->pTextEdit, XUI_INPUT_MENU_COPY, "复制文本");
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pTextEdit);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiTextEditAddLabel(pDemo, NULL, "不显示行号", (xui_rect_t){38.0f, 270.0f, DEMO_TARGET_W - 76.0f, 20.0f}, XUI_COLOR_RGBA(83, 96, 116, 255));
	if ( iRet != XUI_OK ) return iRet;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText =
		"Plain TextEdit\n"
		"This editor keeps the same editing behavior, menu, selection, IME and scrolling support.\n"
		"Line number rendering is disabled for compact form layouts.\n"
		"Double-click, shortcuts and context menu should behave the same.";
	tDesc.sPlaceholder = "TextEdit without line numbers";
	tDesc.pFont = pDemo->pFont;
	tDesc.iMaxLength = 4096;
	tDesc.bWordWrap = 1;
	tDesc.bLineNumbers = 0;
	tDesc.fLineNumberWidth = 0.0f;
	iRet = xuiTextEditCreate(pDemo->pContext, &pDemo->pPlainTextEdit, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pPlainTextEdit, (xui_rect_t){38.0f, 296.0f, DEMO_TARGET_W - 76.0f, 144.0f});
	(void)xuiTextEditSetChange(pDemo->pPlainTextEdit, __xuiTextEditChanged, pDemo);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pPlainTextEdit);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiTextEditAddLabel(pDemo, &pDemo->pStatus, "等待输入", (xui_rect_t){38.0f, 458.0f, DEMO_TARGET_W - 76.0f, 26.0f}, XUI_COLOR_RGBA(83, 96, 116, 255));
	if ( iRet != XUI_OK ) return iRet;
	return XUI_OK;
}

static void __xuiTextEditLayout(xui_textedit_demo_t* pDemo)
{
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiLayout(pDemo->pContext);
}

static uint32_t __xuiTextEditReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiTextEditSendButtonTransitions(xui_textedit_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiTextEditMapKey(int iKey)
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
	case XGE_KEY_PAGE_UP: return XUI_KEY_PAGE_UP;
	case XGE_KEY_PAGE_DOWN: return XUI_KEY_PAGE_DOWN;
	case XGE_KEY_HOME: return XUI_KEY_HOME;
	case XGE_KEY_END: return XUI_KEY_END;
	case XGE_KEY_MENU: return XUI_KEY_CONTEXT_MENU;
	default: return 0;
	}
}

static uint32_t __xuiTextEditReadModifiers(void)
{
	uint32_t iModifiers;

	iModifiers = 0;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_SHIFT) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_SHIFT) ) iModifiers |= XUI_MOD_SHIFT;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_CTRL) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_CTRL) ) iModifiers |= XUI_MOD_CTRL;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_ALT) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_ALT) ) iModifiers |= XUI_MOD_ALT;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_SUPER) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_SUPER) ) iModifiers |= XUI_MOD_SUPER;
	return iModifiers;
}

static int __xuiTextEditSendKeys(xui_textedit_demo_t* pDemo)
{
	static const int arrKeys[] = {
		'A',
		'C',
		'V',
		'X',
		'Y',
		'Z',
		XGE_KEY_ENTER,
		XGE_KEY_TAB,
		XGE_KEY_BACKSPACE,
		XGE_KEY_DELETE,
		XGE_KEY_LEFT,
		XGE_KEY_RIGHT,
		XGE_KEY_UP,
		XGE_KEY_DOWN,
		XGE_KEY_PAGE_UP,
		XGE_KEY_PAGE_DOWN,
		XGE_KEY_HOME,
		XGE_KEY_END,
		XGE_KEY_MENU
	};
	uint32_t iText;
	uint32_t iModifiers;
	int iKey;
	int iRet;
	int i;

	iModifiers = __xuiTextEditReadModifiers();
	iRet = xuiInputSetModifiers(pDemo->pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	for ( i = 0; i < (int)(sizeof(arrKeys) / sizeof(arrKeys[0])); i++ ) {
		iKey = __xuiTextEditMapKey(arrKeys[i]);
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

static int __xuiTextEditHandleInput(xui_textedit_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiTextEditReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiTextEditSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	if ( fWheelX != 0.0f || fWheelY != 0.0f ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiTextEditSendKeys(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiTextEditRightClick(xui_textedit_demo_t* pDemo, float fX, float fY)
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

static void __xuiTextEditRunChecks(xui_textedit_demo_t* pDemo, int bExerciseInput)
{
	xui_widget pMenu;
	xui_rect_t tRect;
	xui_rect_t tPlainRect;
	const xui_menu_item_t* pItem;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pTextEdit != NULL) && (pDemo->pPlainTextEdit != NULL) && (pDemo->pStatus != NULL);
	tRect = xuiWidgetGetRect(pDemo->pTextEdit);
	tPlainRect = xuiWidgetGetRect(pDemo->pPlainTextEdit);
	pDemo->bLayoutOK = (tRect.fW > 500.0f) && (tRect.fH > 140.0f) &&
	                   (tPlainRect.fW > 500.0f) && (tPlainRect.fH > 100.0f) &&
	                   (xuiTextEditGetLineCount(pDemo->pTextEdit) >= 5) &&
	                   (xuiTextEditGetLineCount(pDemo->pPlainTextEdit) >= 4);
	pDemo->bStateOK = xuiTextEditGetWordWrap(pDemo->pTextEdit) &&
	                  xuiTextEditGetLineNumbers(pDemo->pTextEdit) &&
	                  (xuiTextEditGetLineNumberWidth(pDemo->pTextEdit) >= 46.0f) &&
	                  (strcmp(xuiTextEditGetMenuTitle(pDemo->pTextEdit, XUI_INPUT_MENU_COPY), "复制文本") == 0);
	pDemo->bPlainOK = xuiTextEditGetWordWrap(pDemo->pPlainTextEdit) &&
	                  !xuiTextEditGetLineNumbers(pDemo->pPlainTextEdit);
	pDemo->bStateOK = pDemo->bStateOK && pDemo->bPlainOK;
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bLayoutOK ) {
		tRect = xuiWidgetGetWorldRect(pDemo->pTextEdit);
		(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pTextEdit);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputText(pDemo->pContext, '!');
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pPlainTextEdit);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputText(pDemo->pContext, '?');
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiTextEditSetSelection(pDemo->pTextEdit, 0, (int)strlen(xuiTextEditGetText(pDemo->pTextEdit)));
		(void)__xuiTextEditRightClick(pDemo, tRect.fX + 18.0f, tRect.fY + 28.0f);
		(void)xuiTextEditSetScroll(pDemo->pTextEdit, 0.0f, 24.0f);
		pDemo->bExerciseDone = 1;
	}
	pMenu = xuiTextEditGetMenuWidget(pDemo->pTextEdit);
	pItem = (pMenu != NULL) ? xuiMenuGetItem(pMenu, 1) : NULL;
	pDemo->bMenuOK = (pMenu != NULL) && (xuiMenuGetItemCount(pMenu) == 8) &&
	                 (pItem != NULL) && (pItem->iType == XUI_MENU_ITEM_SEPARATOR);
	pDemo->bInputOK = !bExerciseInput || ((pDemo->iChangeCount > 1) &&
	                                      (strchr(xuiTextEditGetText(pDemo->pTextEdit), '!') != NULL) &&
	                                      (strchr(xuiTextEditGetText(pDemo->pPlainTextEdit), '?') != NULL));
}

static int __xuiTextEditCreateAssets(xui_textedit_demo_t* pDemo)
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
	sFontPath = __xuiTextEditFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiTextEditCreateUi(pDemo);
}

static void __xuiTextEditDestroyAssets(xui_textedit_demo_t* pDemo)
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

static int __xuiTextEditFrame(void* pUser)
{
	xui_textedit_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_textedit_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiTextEditHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiTextEditLayout(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiTextEditRunChecks(pDemo, bAutoRun);
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
		printf("xui_textedit final-summary frames=%d create=%d layout=%d state=%d plain=%d menu=%d input=%d changes=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bStateOK, pDemo->bPlainOK, pDemo->bMenuOK, pDemo->bInputOK, pDemo->iChangeCount,
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_textedit_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiTextEditParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiTextEditUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI TextEdit";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_textedit: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiTextEditCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_textedit: create assets failed: %d\n", iRet);
		__xuiTextEditDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiTextEditFrame, &tDemo);
	__xuiTextEditDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bStateOK && tDemo.bPlainOK && tDemo.bMenuOK && tDemo.bInputOK) ? 0 : 1;
}
