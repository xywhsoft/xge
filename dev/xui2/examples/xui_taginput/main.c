#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	640
#define DEMO_TARGET_H	420
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define TAGINPUT_COUNT	4

#define XUI_DEMO_KEY_LEFT_SHIFT		340
#define XUI_DEMO_KEY_LEFT_CTRL		341
#define XUI_DEMO_KEY_LEFT_ALT		342
#define XUI_DEMO_KEY_LEFT_SUPER		343
#define XUI_DEMO_KEY_RIGHT_SHIFT	344
#define XUI_DEMO_KEY_RIGHT_CTRL		345
#define XUI_DEMO_KEY_RIGHT_ALT		346
#define XUI_DEMO_KEY_RIGHT_SUPER	347

typedef struct xui_taginput_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pTagInput[TAGINPUT_COUNT];
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
	int bInputOK;
	int bStateOK;
} xui_taginput_demo_t;

static void __xuiTagInputDemoUsage(void)
{
	printf("usage: xui_taginput [--frames N] [--seconds N]\n");
}

static int __xuiTagInputDemoParseArgs(xui_taginput_demo_t* pDemo, int argc, char** argv)
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
			__xuiTagInputDemoUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiTagInputDemoFindTtf(void)
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

static int __xuiTagInputDemoRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_taginput_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_taginput_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(234, 242, 250, 255));
	}
	if ( pDemo->tProxy.drawRoundRectFill != NULL ) {
		tPanel = (xui_rect_t){28.0f, 22.0f, tRect.fW - 56.0f, tRect.fH - 44.0f};
		(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tPanel, 6.0f, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static int __xuiTagInputDemoAddLabel(xui_taginput_demo_t* pDemo, const char* sText, xui_rect_t tRect, xui_widget* ppLabel)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(57, 71, 91, 255);
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

static void __xuiTagInputDemoChanged(xui_widget pWidget, int iTagCount, void* pUser)
{
	xui_taginput_demo_t* pDemo;
	char sStatus[160];

	(void)pWidget;
	pDemo = (xui_taginput_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iChangeCount++;
	snprintf(sStatus, sizeof(sStatus), "changes=%d current-tags=%d", pDemo->iChangeCount, iTagCount);
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sStatus);
	}
}

static int __xuiTagInputDemoAddTagInput(xui_taginput_demo_t* pDemo, int iIndex, const char* const* ppTags, int iTagCount, const char* sPlaceholder, xui_rect_t tRect)
{
	xui_tag_input_desc_t tDesc;
	int iRet;

	if ( iIndex < 0 || iIndex >= TAGINPUT_COUNT ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.ppTags = ppTags;
	tDesc.iTagCount = iTagCount;
	tDesc.sPlaceholder = sPlaceholder;
	tDesc.pFont = pDemo->pFont;
	tDesc.iMaxTags = 12;
	iRet = xuiTagInputCreate(pDemo->pContext, &pDemo->pTagInput[iIndex], &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiTagInputSetChange(pDemo->pTagInput[iIndex], __xuiTagInputDemoChanged, pDemo);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pTagInput[iIndex], tRect);
	return xuiWidgetAddChild(pDemo->pRoot, pDemo->pTagInput[iIndex]);
}

static int __xuiTagInputDemoCreateUi(xui_taginput_demo_t* pDemo)
{
	static const char* arrDefaultTags[] = {"Tag 1", "Tag 2"};
	static const char* arrWrapTags[] = {"Scene", "Document", "Preview", "Inspector", "Output"};
	static const char* arrAccentTags[] = {"C", "XUI2"};
	static const char* arrDisabledTags[] = {"Readonly", "Locked"};
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiTagInputDemoRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiTagInputDemoAddLabel(pDemo, "XUI TagInput", (xui_rect_t){48.0f, 36.0f, 180.0f, 22.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiTagInputDemoAddLabel(pDemo, "Default", (xui_rect_t){58.0f, 84.0f, 92.0f, 22.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiTagInputDemoAddLabel(pDemo, "Wrapped", (xui_rect_t){58.0f, 140.0f, 92.0f, 22.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiTagInputDemoAddLabel(pDemo, "Accent", (xui_rect_t){58.0f, 242.0f, 92.0f, 22.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiTagInputDemoAddLabel(pDemo, "Disabled", (xui_rect_t){58.0f, 296.0f, 92.0f, 22.0f}, NULL);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiTagInputDemoAddTagInput(pDemo, 0, arrDefaultTags, 2, "Please input", (xui_rect_t){160.0f, 76.0f, 380.0f, 40.0f});
	if ( iRet == XUI_OK ) iRet = __xuiTagInputDemoAddTagInput(pDemo, 1, arrWrapTags, 5, "Add tag", (xui_rect_t){160.0f, 130.0f, 380.0f, 78.0f});
	if ( iRet == XUI_OK ) iRet = __xuiTagInputDemoAddTagInput(pDemo, 2, arrAccentTags, 2, "Enter language", (xui_rect_t){160.0f, 234.0f, 380.0f, 40.0f});
	if ( iRet == XUI_OK ) iRet = __xuiTagInputDemoAddTagInput(pDemo, 3, arrDisabledTags, 2, "Disabled", (xui_rect_t){160.0f, 288.0f, 380.0f, 40.0f});
	if ( iRet != XUI_OK ) return iRet;

	(void)xuiTagInputSetColors(pDemo->pTagInput[2], XUI_COLOR_RGBA(244, 253, 250, 255), XUI_COLOR_RGBA(52, 178, 122, 255),
		XUI_COLOR_RGBA(16, 185, 129, 255), XUI_COLOR_RGBA(219, 247, 235, 255), XUI_COLOR_RGBA(31, 111, 82, 255));
	(void)xuiWidgetSetEnabled(pDemo->pTagInput[3], 0);
	iRet = __xuiTagInputDemoAddLabel(pDemo, "Type text, press Enter or comma to create a tag. Backspace removes the last empty tag.",
		(xui_rect_t){58.0f, 346.0f, 520.0f, 22.0f}, &pDemo->pStatus);
	return iRet;
}

static uint32_t __xuiTagInputDemoReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiTagInputDemoSendButtonTransitions(xui_taginput_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiTagInputDemoMapKey(int iKey)
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

static uint32_t __xuiTagInputDemoReadModifiers(void)
{
	uint32_t iModifiers;

	iModifiers = 0;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_SHIFT) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_SHIFT) ) iModifiers |= XUI_MOD_SHIFT;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_CTRL) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_CTRL) ) iModifiers |= XUI_MOD_CTRL;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_ALT) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_ALT) ) iModifiers |= XUI_MOD_ALT;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_SUPER) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_SUPER) ) iModifiers |= XUI_MOD_SUPER;
	return iModifiers;
}

static int __xuiTagInputDemoSendKeys(xui_taginput_demo_t* pDemo)
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
	uint32_t iModifiers;
	uint32_t iText;
	int iKey;
	int iRet;
	int i;

	iModifiers = __xuiTagInputDemoReadModifiers();
	iRet = xuiInputSetModifiers(pDemo->pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	for ( i = 0; i < (int)(sizeof(arrKeys) / sizeof(arrKeys[0])); i++ ) {
		iKey = __xuiTagInputDemoMapKey(arrKeys[i]);
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

static int __xuiTagInputDemoHandleInput(xui_taginput_demo_t* pDemo)
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
	iButtons = __xuiTagInputDemoReadButtons();
	if ( !pDemo->bHasMouse || pDemo->fLastMouseX != fX || pDemo->fLastMouseY != fY || pDemo->iLastButtons != iButtons ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiTagInputDemoSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiTagInputDemoSendKeys(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiTagInputDemoDispatchText(xui_taginput_demo_t* pDemo, uint32_t iCodepoint)
{
	int iRet;

	iRet = xuiInputText(pDemo->pContext, iCodepoint);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static int __xuiTagInputDemoDispatchKey(xui_taginput_demo_t* pDemo, int iKey)
{
	int iRet;

	iRet = xuiInputKeyDown(pDemo->pContext, iKey, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputKeyUp(pDemo->pContext, iKey, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static void __xuiTagInputDemoRunChecks(xui_taginput_demo_t* pDemo, int bAutoRun)
{
	xui_rect_t tInput;
	xui_rect_t tClose;
	int iOk;

	if ( pDemo == NULL ) return;
	tInput = xuiTagInputGetInputRect(pDemo->pTagInput[0]);
	tClose = xuiTagInputGetCloseRect(pDemo->pTagInput[0], 0);
	pDemo->bLayoutOK = (tInput.fW >= 72.0f) && (tInput.fH > 0.0f) && (tClose.fW > 0.0f) && (tClose.fH > 0.0f);
	pDemo->bStateOK = (xuiWidgetGetCacheStateCount(pDemo->pTagInput[0]) == 4) &&
	                  ((xuiTagInputGetState(pDemo->pTagInput[3]) & XUI_WIDGET_STATE_DISABLED) != 0);
	if ( !bAutoRun || pDemo->bExerciseDone ) {
		return;
	}
	iOk = (xuiSetFocusWidget(pDemo->pContext, xuiTagInputGetInputWidget(pDemo->pTagInput[0])) == XUI_OK);
	iOk = iOk && (xuiDispatchPendingEvents(pDemo->pContext) == XUI_OK);
	iOk = iOk && (__xuiTagInputDemoDispatchText(pDemo, 'N') == XUI_OK);
	iOk = iOk && (__xuiTagInputDemoDispatchText(pDemo, 'e') == XUI_OK);
	iOk = iOk && (__xuiTagInputDemoDispatchText(pDemo, 'w') == XUI_OK);
	iOk = iOk && (__xuiTagInputDemoDispatchText(pDemo, ',') == XUI_OK);
	iOk = iOk && (xuiTagInputGetTagCount(pDemo->pTagInput[0]) == 3);
	iOk = iOk && (strcmp(xuiTagInputGetTag(pDemo->pTagInput[0], 2), "New") == 0);
	iOk = iOk && (__xuiTagInputDemoDispatchKey(pDemo, 8) == XUI_OK);
	iOk = iOk && (xuiTagInputGetTagCount(pDemo->pTagInput[0]) == 2);
	iOk = iOk && (xuiTagInputSetText(pDemo->pTagInput[1], "Shader") == XUI_OK);
	iOk = iOk && (xuiTagInputCommit(pDemo->pTagInput[1]) == XUI_OK);
	iOk = iOk && (xuiTagInputGetTagCount(pDemo->pTagInput[1]) == 6);
	pDemo->bInputOK = iOk && (pDemo->iChangeCount >= 3);
	pDemo->bExerciseDone = 1;
}

static int __xuiTagInputDemoCreateAssets(xui_taginput_demo_t* pDemo)
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
	sFontPath = __xuiTagInputDemoFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiTagInputDemoCreateUi(pDemo);
}

static void __xuiTagInputDemoDestroyAssets(xui_taginput_demo_t* pDemo)
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

static int __xuiTagInputDemoFrame(void* pUser)
{
	xui_taginput_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_taginput_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiTagInputDemoHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiTagInputDemoRunChecks(pDemo, bAutoRun);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
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
		printf("xui_taginput final-summary frames=%d create=%d layout=%d input=%d state=%d changes=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bInputOK, pDemo->bStateOK, pDemo->iChangeCount,
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_taginput_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiTagInputDemoParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiTagInputDemoUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI TagInput";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_taginput: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiTagInputDemoCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_taginput: create assets failed: %d\n", iRet);
		__xuiTagInputDemoDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	tDemo.bCreateOK = 1;
	iRet = xgeRun(__xuiTagInputDemoFrame, &tDemo);
	__xuiTagInputDemoDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bStateOK &&
	        ((tDemo.iMaxFrames <= 0 && tDemo.fMaxSeconds <= 0.0) || tDemo.bInputOK)) ? 0 : 1;
}
