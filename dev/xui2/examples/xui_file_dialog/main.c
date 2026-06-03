#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 860
#define DEMO_TARGET_H 540
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define XUI_DEMO_KEY_LEFT_SHIFT 340
#define XUI_DEMO_KEY_LEFT_CTRL 341
#define XUI_DEMO_KEY_LEFT_ALT 342
#define XUI_DEMO_KEY_LEFT_SUPER 343
#define XUI_DEMO_KEY_RIGHT_SHIFT 344
#define XUI_DEMO_KEY_RIGHT_CTRL 345
#define XUI_DEMO_KEY_RIGHT_ALT 346
#define XUI_DEMO_KEY_RIGHT_SUPER 347

typedef struct xui_file_dialog_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pTitle;
	xui_widget pStatus;
	xui_widget pOpenButton;
	xui_widget pSaveButton;
	xui_widget pFolderButton;
	xui_file_dialog pDialog;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int iResultCount;
	int bExerciseDone;
	int bCreateOK;
	int bRootOK;
	int bResultOK;
	char sLastPath[1024];
} xui_file_dialog_demo_t;

static void __xuiFileDialogUsage(void)
{
	printf("usage: xui_file_dialog [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiFileDialogParseArgs(xui_file_dialog_demo_t* pDemo, int argc, char** argv)
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
			__xuiFileDialogUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static int __xuiFileDialogLoadUiFont(xui_file_dialog_demo_t* pDemo)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\NotoSansSC-VF.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\msyhbd.ttc",
		"C:\\Windows\\Fonts\\simhei.ttf",
		"C:\\Windows\\Fonts\\simsun.ttc",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf"
	};
	FILE* pFile;
	int i;
	int iRet;

	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, arrPaths[i], 14.0f, XUI_FONT_FORMAT_TTF);
			if ( iRet == XUI_OK && pDemo->pFont != NULL ) {
				return XUI_OK;
			}
		}
	}
	return XUI_ERROR_FILE_NOT_FOUND;
}

static int __xuiFileDialogRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_file_dialog_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_file_dialog_demo_t*)pUser;
	if ( pWidget == NULL || pDraw == NULL || pDemo == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(234, 239, 246, 255));
		tPanel = (xui_rect_t){36.0f, 36.0f, tRect.fW - 72.0f, 190.0f};
		(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tPanel, 8.0f, XUI_COLOR_RGBA(250, 252, 255, 255));
	}
	return XUI_OK;
}

static int __xuiFileDialogAddLabel(xui_file_dialog_demo_t* pDemo, xui_widget* ppWidget, const char* sText, xui_rect_t tRect, uint32_t iColor)
{
	xui_label_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, ppWidget, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(*ppWidget, tRect);
	return xuiWidgetAddChild(pDemo->pRoot, *ppWidget);
}

static int __xuiFileDialogAddButton(xui_file_dialog_demo_t* pDemo, xui_widget* ppWidget, const char* sText, xui_rect_t tRect, xui_button_click_proc onClick)
{
	xui_button_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.fRadius = 4.0f;
	tDesc.fBorderWidth = 1.0f;
	iRet = xuiButtonCreate(pDemo->pContext, ppWidget, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(*ppWidget, tRect);
	(void)xuiButtonSetClick(*ppWidget, onClick, pDemo);
	return xuiWidgetAddChild(pDemo->pRoot, *ppWidget);
}

static void __xuiFileDialogResult(xui_file_dialog pDialog, int iResult, const char* sPath, void* pUser)
{
	xui_file_dialog_demo_t* pDemo;
	char sText[1152];

	(void)pDialog;
	pDemo = (xui_file_dialog_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iResultCount++;
	snprintf(pDemo->sLastPath, sizeof(pDemo->sLastPath), "%s", sPath != NULL ? sPath : "");
	snprintf(sText, sizeof(sText), "result=%d path=%s", iResult, pDemo->sLastPath);
	(void)xuiLabelSetText(pDemo->pStatus, sText);
}

static int __xuiFileDialogShow(xui_file_dialog_demo_t* pDemo, int iMode)
{
	xui_file_dialog_desc_t tDesc;
	int iRet;

	if ( pDemo->pDialog != NULL ) {
		xuiFileDialogDestroy(pDemo->pDialog);
		pDemo->pDialog = NULL;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sInitialDir = "";
	tDesc.sFilter = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*";
	tDesc.pFont = pDemo->pFont;
	tDesc.onResult = __xuiFileDialogResult;
	tDesc.pResultUser = pDemo;
	tDesc.fWidth = 720.0f;
	tDesc.fHeight = 520.0f;
	if ( iMode == XUI_FILE_DIALOG_MODE_SAVE_FILE ) {
		tDesc.sFileName = "new-file.txt";
		iRet = xuiSaveFileDialog(pDemo->pContext, &pDemo->pDialog, &tDesc);
	} else if ( iMode == XUI_FILE_DIALOG_MODE_SELECT_FOLDER ) {
		iRet = xuiSelectFolderDialog(pDemo->pContext, &pDemo->pDialog, &tDesc);
	} else {
		iRet = xuiOpenFileDialog(pDemo->pContext, &pDemo->pDialog, &tDesc);
	}
	return iRet;
}

static void __xuiFileDialogOpenClicked(xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)__xuiFileDialogShow((xui_file_dialog_demo_t*)pUser, XUI_FILE_DIALOG_MODE_OPEN_FILE);
}

static void __xuiFileDialogSaveClicked(xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)__xuiFileDialogShow((xui_file_dialog_demo_t*)pUser, XUI_FILE_DIALOG_MODE_SAVE_FILE);
}

static void __xuiFileDialogFolderClicked(xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)__xuiFileDialogShow((xui_file_dialog_demo_t*)pUser, XUI_FILE_DIALOG_MODE_SELECT_FOLDER);
}

static int __xuiFileDialogCreateUi(xui_file_dialog_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiFileDialogRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFileDialogAddLabel(pDemo, &pDemo->pTitle, "XUI File Dialog", (xui_rect_t){56.0f, 54.0f, 260.0f, 30.0f}, XUI_COLOR_RGBA(34, 52, 78, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFileDialogAddButton(pDemo, &pDemo->pOpenButton, "Open File", (xui_rect_t){56.0f, 108.0f, 128.0f, 34.0f}, __xuiFileDialogOpenClicked);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFileDialogAddButton(pDemo, &pDemo->pSaveButton, "Save File", (xui_rect_t){198.0f, 108.0f, 128.0f, 34.0f}, __xuiFileDialogSaveClicked);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFileDialogAddButton(pDemo, &pDemo->pFolderButton, "Select Folder", (xui_rect_t){340.0f, 108.0f, 142.0f, 34.0f}, __xuiFileDialogFolderClicked);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiFileDialogAddLabel(pDemo, &pDemo->pStatus, "result=none path=", (xui_rect_t){56.0f, 166.0f, 700.0f, 30.0f}, XUI_COLOR_RGBA(76, 92, 112, 255));
}

static uint32_t __xuiFileDialogReadButtons(void)
{
	uint32_t iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiFileDialogMapKey(int iKey)
{
	switch ( iKey ) {
	case XGE_KEY_ENTER: return XUI_KEY_ENTER;
	case XGE_KEY_TAB: return XUI_KEY_TAB;
	case XGE_KEY_ESCAPE: return XUI_KEY_ESCAPE;
	case XGE_KEY_BACKSPACE: return XUI_KEY_BACKSPACE;
	case XGE_KEY_DELETE: return XUI_KEY_DELETE;
	case XGE_KEY_LEFT: return XUI_KEY_LEFT;
	case XGE_KEY_UP: return XUI_KEY_UP;
	case XGE_KEY_RIGHT: return XUI_KEY_RIGHT;
	case XGE_KEY_DOWN: return XUI_KEY_DOWN;
	case XGE_KEY_HOME: return XUI_KEY_HOME;
	case XGE_KEY_END: return XUI_KEY_END;
	case XGE_KEY_MENU: return XUI_KEY_CONTEXT_MENU;
	default: return 0;
	}
}

static uint32_t __xuiFileDialogReadModifiers(void)
{
	uint32_t iModifiers;

	iModifiers = 0;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_SHIFT) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_SHIFT) ) iModifiers |= XUI_MOD_SHIFT;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_CTRL) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_CTRL) ) iModifiers |= XUI_MOD_CTRL;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_ALT) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_ALT) ) iModifiers |= XUI_MOD_ALT;
	if ( xgeKeyDown(XUI_DEMO_KEY_LEFT_SUPER) || xgeKeyDown(XUI_DEMO_KEY_RIGHT_SUPER) ) iModifiers |= XUI_MOD_SUPER;
	return iModifiers;
}

static int __xuiFileDialogSendKeys(xui_file_dialog_demo_t* pDemo)
{
	static const int arrKeys[] = {
		'A',
		'C',
		'V',
		'X',
		'Z',
		XGE_KEY_ENTER,
		XGE_KEY_TAB,
		XGE_KEY_ESCAPE,
		XGE_KEY_BACKSPACE,
		XGE_KEY_DELETE,
		XGE_KEY_LEFT,
		XGE_KEY_UP,
		XGE_KEY_RIGHT,
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

	iModifiers = __xuiFileDialogReadModifiers();
	iRet = xuiInputSetModifiers(pDemo->pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < (int)(sizeof(arrKeys) / sizeof(arrKeys[0])); i++ ) {
		iKey = __xuiFileDialogMapKey(arrKeys[i]);
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

static int __xuiFileDialogHandleInput(xui_file_dialog_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) && (pDemo->pDialog == NULL || !xuiFileDialogIsOpen(pDemo->pDialog)) ) {
		xgeQuit();
		return XUI_OK;
	}
	iRet = __xuiFileDialogSendKeys(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiFileDialogReadButtons();
	if ( !pDemo->bHasMouse || pDemo->fLastMouseX != fX || pDemo->fLastMouseY != fY || pDemo->iLastButtons != iButtons ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( fWheelX != 0.0f || fWheelY != 0.0f ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiFileDialogRunChecks(xui_file_dialog_demo_t* pDemo, int bExerciseInput)
{
	if ( !bExerciseInput ) {
		pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pOpenButton != NULL) && (pDemo->pStatus != NULL);
		pDemo->bRootOK = 1;
		pDemo->bResultOK = 1;
		return;
	}
	if ( pDemo->bExerciseDone ) return;
	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pOpenButton != NULL) && (pDemo->pStatus != NULL);
	if ( pDemo->bCreateOK && __xuiFileDialogShow(pDemo, XUI_FILE_DIALOG_MODE_SELECT_FOLDER) == XUI_OK ) {
		pDemo->bRootOK = xuiFileDialogGetEntryCount(pDemo->pDialog) > 0;
		if ( pDemo->bRootOK ) {
			(void)xuiFileDialogSelectIndex(pDemo->pDialog, 0);
			(void)xuiFileDialogCommit(pDemo->pDialog);
		}
		pDemo->bResultOK = (pDemo->iResultCount == 1) &&
		                    (xuiFileDialogGetResult(pDemo->pDialog) == XUI_FILE_DIALOG_RESULT_OK) &&
		                    (pDemo->sLastPath[0] != 0);
	}
	pDemo->bExerciseDone = 1;
}

static int __xuiFileDialogCreateAssets(xui_file_dialog_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
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
	iRet = __xuiFileDialogLoadUiFont(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiFileDialogCreateUi(pDemo);
}

static void __xuiFileDialogDestroyAssets(xui_file_dialog_demo_t* pDemo)
{
	if ( pDemo->pDialog != NULL ) {
		xuiFileDialogDestroy(pDemo->pDialog);
		pDemo->pDialog = NULL;
	}
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

static int __xuiFileDialogFrame(void* pUser)
{
	xui_file_dialog_demo_t* pDemo;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;
	int bAutoRun;

	pDemo = (xui_file_dialog_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiFileDialogHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiFileDialogRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(234, 239, 246, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(18, 23, 32, 255));
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
		printf("xui_file_dialog final-summary frames=%d create=%d root=%d result=%d results=%d path=%s updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bRootOK, pDemo->bResultOK, pDemo->iResultCount,
			pDemo->sLastPath, tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_file_dialog_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiFileDialogParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiFileDialogUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI File Dialog";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_file_dialog: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiFileDialogCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_file_dialog: create assets failed: %d\n", iRet);
		__xuiFileDialogDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiFileDialogFrame, &tDemo);
	__xuiFileDialogDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bRootOK && tDemo.bResultOK) ? 0 : 1;
}
