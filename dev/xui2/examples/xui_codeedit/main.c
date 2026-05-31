#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 820
#define DEMO_H 520
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f

#ifndef XGE_KEY_LEFT_SHIFT
#define XGE_KEY_LEFT_SHIFT 340
#define XGE_KEY_LEFT_CONTROL 341
#define XGE_KEY_LEFT_ALT 342
#define XGE_KEY_RIGHT_SHIFT 344
#define XGE_KEY_RIGHT_CONTROL 345
#define XGE_KEY_RIGHT_ALT 346
#endif

typedef struct xui_codeedit_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pCodeEdit;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	uint32_t iLastButtons;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bRenderOK;
	int bLexerOK;
	int bInputOK;
	int bMenuOK;
	int bScrollOK;
} xui_codeedit_demo_t;

static const char* g_sCodeEditDemoText =
	"#include <stdio.h>\n"
	"\n"
	"static int add(int a, int b) {\n"
	"\treturn a + b;\n"
	"}\n"
	"\n"
	"int main(void) {\n"
	"\tprintf(\"hello xui codeedit: %d\\n\", add(2, 3));\n"
	"\treturn 0;\n"
	"}\n";

static void __xuiCodeEditUsage(void)
{
	printf("usage: xui_codeedit [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiCodeEditParseArgs(xui_codeedit_demo_t* pDemo, int argc, char** argv)
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
			__xuiCodeEditUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiCodeEditFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\consola.ttf",
		"C:\\Windows\\Fonts\\cascadiamono.ttf",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc"
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

static int __xuiCodeEditRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_codeedit_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tHeader;

	(void)iStateId;
	pDemo = (xui_codeedit_demo_t*)pUser;
	if ( pWidget == NULL || pDraw == NULL || pDemo == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(234, 238, 245, 255));
	}
	if ( pDemo->tProxy.drawRoundRectFill != NULL ) {
		tHeader = (xui_rect_t){24.0f, 18.0f, tRect.fW - 48.0f, 44.0f};
		(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tHeader, 6.0f, XUI_COLOR_RGBA(247, 250, 255, 255));
	}
	return XUI_OK;
}

static int __xuiCodeEditCreateUi(xui_codeedit_demo_t* pDemo)
{
	xui_code_edit_desc_t tDesc;
	xui_code_fold_range_t arrFolds[16];
	xui_code_range_t tRange;
	int iFoldCount;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiCodeEditRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.sText = g_sCodeEditDemoText;
	tDesc.sLanguage = "c";
	tDesc.bShowLineNumbers = 1;
	tDesc.bShowFoldMargin = 1;
	tDesc.bShowMarkerMargin = 1;
	tDesc.bShowDiagnosticMargin = 1;
	tDesc.iTabColumns = 4;
	tDesc.iIndentColumns = 4;
	tDesc.iFlags = XUI_CODE_EDIT_SHOW_WHITESPACE | XUI_CODE_EDIT_SHOW_EOL | XUI_CODE_EDIT_SHOW_INDENT_GUIDES;
	iRet = xuiCodeEditCreate(pDemo->pContext, &pDemo->pCodeEdit, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pCodeEdit, (xui_rect_t){28.0f, 72.0f, DEMO_W - 56.0f, DEMO_H - 106.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pCodeEdit);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pCodeEdit);

	(void)xuiCodeAnnotationSetMarker(xuiCodeEditGetAnnotations(pDemo->pCodeEdit), 6, XUI_CODE_MARKER_BOOKMARK, 0u, "main", 1u);
	if ( xuiCodeFoldCBuildRanges(g_sCodeEditDemoText, -1, arrFolds, 16, &iFoldCount) == XUI_OK && iFoldCount > 0 ) {
		(void)xuiCodeFoldStateSetRanges(xuiCodeEditGetFoldState(pDemo->pCodeEdit), arrFolds, iFoldCount);
	}
	if ( xuiCodeSearchFindPlain(xuiCodeEditGetDocument(pDemo->pCodeEdit), "printf", 0, XUI_CODE_SEARCH_CASE_SENSITIVE, &tRange) == XUI_OK ) {
		(void)xuiCodeAnnotationSetIndicator(xuiCodeEditGetAnnotations(pDemo->pCodeEdit), XUI_CODE_INDICATOR_SEARCH_RESULT, XUI_CODE_INDICATOR_SEARCH_RESULT, tRange.iStart, tRange.iEnd, 0u, 2u);
	}
	return XUI_OK;
}

static void __xuiCodeEditLayout(xui_codeedit_demo_t* pDemo)
{
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H});
	(void)xuiWidgetSetRect(pDemo->pCodeEdit, (xui_rect_t){28.0f, 72.0f, DEMO_W - 56.0f, DEMO_H - 106.0f});
	(void)xuiLayout(pDemo->pContext);
}

static uint32_t __xuiCodeEditReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static uint32_t __xuiCodeEditReadModifiers(void)
{
	uint32_t iModifiers;

	iModifiers = 0;
	if ( xgeKeyDown(XGE_KEY_LEFT_SHIFT) || xgeKeyDown(XGE_KEY_RIGHT_SHIFT) ) iModifiers |= XUI_MOD_SHIFT;
	if ( xgeKeyDown(XGE_KEY_LEFT_CONTROL) || xgeKeyDown(XGE_KEY_RIGHT_CONTROL) ) iModifiers |= XUI_MOD_CTRL;
	if ( xgeKeyDown(XGE_KEY_LEFT_ALT) || xgeKeyDown(XGE_KEY_RIGHT_ALT) ) iModifiers |= XUI_MOD_ALT;
	return iModifiers;
}

static int __xuiCodeEditMapKey(int iKey)
{
	switch ( iKey ) {
	case XGE_KEY_ENTER: return XUI_KEY_ENTER;
	case XGE_KEY_TAB: return XUI_KEY_TAB;
	case XGE_KEY_BACKSPACE: return XUI_KEY_BACKSPACE;
	case XGE_KEY_DELETE: return XUI_KEY_DELETE;
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

static int __xuiCodeEditSendKeyTransitions(xui_codeedit_demo_t* pDemo)
{
	static const int arrKeys[] = {
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
		'A',
		'C',
		'V',
		'X',
		'Y',
		'Z',
		'/',
		XGE_KEY_MENU
	};
	uint32_t iModifiers;
	uint32_t iText;
	int iKey;
	int i;
	int iRet;

	iModifiers = __xuiCodeEditReadModifiers();
	iRet = xuiInputSetModifiers(pDemo->pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	for ( i = 0; i < (int)(sizeof(arrKeys) / sizeof(arrKeys[0])); i++ ) {
		iKey = __xuiCodeEditMapKey(arrKeys[i]);
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

static int __xuiCodeEditHandleInput(xui_codeedit_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	float fUiX;
	float fUiY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	fUiX = fX - DEMO_OFFSET_X;
	fUiY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiCodeEditReadButtons();
	if ( !pDemo->bHasMouse || pDemo->fLastMouseX != fX || pDemo->fLastMouseY != fY || pDemo->iLastButtons != iButtons ) {
		iRet = xuiInputPointerMove(pDemo->pContext, fUiX, fUiY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fUiX, fUiY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iPressed & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fUiX, fUiY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fUiX, fUiY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fUiX, fUiY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( fWheelX != 0.0f || fWheelY != 0.0f ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, fUiX, fUiY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiCodeEditSendKeyTransitions(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiCodeEditRunChecks(xui_codeedit_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tRect;
	float fScrollX;
	float fScrollY;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pCodeEdit != NULL) &&
		(xuiCodeEditGetDocument(pDemo->pCodeEdit) != NULL) &&
		(strcmp(xuiCodeEditGetLanguage(pDemo->pCodeEdit), "c") == 0);
	tRect = xuiWidgetGetRect(pDemo->pCodeEdit);
	pDemo->bLayoutOK = (tRect.fW > 700.0f) && (tRect.fH > 350.0f);
	pDemo->bLexerOK = xuiCodeTokenBufferGetCount(xuiCodeEditGetTokenBuffer(pDemo->pCodeEdit)) > 0;
	pDemo->bMenuOK = xuiCodeEditGetMenuWidget(pDemo->pCodeEdit) != NULL;
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bLayoutOK ) {
		(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pCodeEdit);
		(void)xuiCodeEditSetScroll(pDemo->pCodeEdit, 0.0f, 36.0f);
		(void)xuiInputText(pDemo->pContext, '!');
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bExerciseDone = 1;
	}
	(void)xuiCodeEditGetScroll(pDemo->pCodeEdit, &fScrollX, &fScrollY);
	pDemo->bScrollOK = (xuiCodeEditGetScrollModel(pDemo->pCodeEdit) != NULL) &&
		(xuiCodeEditGetHScrollBarWidget(pDemo->pCodeEdit) != NULL) &&
		(xuiCodeEditGetVScrollBarWidget(pDemo->pCodeEdit) != NULL) &&
		(fScrollY >= 0.0f);
	pDemo->bInputOK = !bExerciseInput || (strchr(xuiCodeEditGetText(pDemo->pCodeEdit), '!') != NULL);
}

static int __xuiCodeEditCreateAssets(xui_codeedit_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_W, (float)DEMO_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_W;
	tSurfaceDesc.iHeight = DEMO_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiCodeEditFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiCodeEditCreateUi(pDemo);
}

static void __xuiCodeEditDestroyAssets(xui_codeedit_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
		pDemo->pFont = NULL;
	}
}

static int __xuiCodeEditFrame(void* pUser)
{
	xui_codeedit_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int bAutoRun;
	int iRet;

	pDemo = (xui_codeedit_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiCodeEditHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiCodeEditLayout(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiCodeEditRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_W, DEMO_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bRenderOK = 1;
	xgeClear(XUI_COLOR_RGBA(20, 24, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_W, (float)DEMO_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
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
		printf("xui_codeedit final-summary frames=%d create=%d layout=%d render=%d lexer=%d input=%d menu=%d scroll=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bRenderOK, pDemo->bLexerOK, pDemo->bInputOK, pDemo->bMenuOK, pDemo->bScrollOK,
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_codeedit_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;
	int bAutoRun;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiCodeEditParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiCodeEditUsage();
		return 1;
	}
	bAutoRun = (tDemo.iMaxFrames > 0) || (tDemo.fMaxSeconds > 0.0);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_W + 20;
	tDesc.iHeight = DEMO_H + 50;
	tDesc.sTitle = "XUI CodeEdit";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_codeedit: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiCodeEditCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_codeedit: create assets failed: %d\n", iRet);
		__xuiCodeEditDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiCodeEditFrame, &tDemo);
	__xuiCodeEditDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bRenderOK &&
		tDemo.bLexerOK && tDemo.bMenuOK && tDemo.bScrollOK && (!bAutoRun || tDemo.bInputOK)) ? 0 : 1;
}
