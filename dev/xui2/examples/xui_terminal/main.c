#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 760
#define DEMO_TARGET_H 470
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f

typedef struct xui_terminal_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pTerminal;
	xui_widget pStatus;
	xui_terminal_session_t* pSession;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int iInputBytes;
	int iResizeCount;
	int iLastCols;
	int iLastRows;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bDynamicOK;
} xui_terminal_demo_t;

static void __xuiTerminalDemoUsage(void)
{
	printf("usage: xui_terminal [--frames N] [--seconds N]\n");
}

static int __xuiTerminalDemoParseArgs(xui_terminal_demo_t* pDemo, int argc, char** argv)
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
			__xuiTerminalDemoUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiTerminalDemoFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\NotoSansMonoCJKsc-Regular.otf",
		"C:\\Windows\\Fonts\\SarasaMonoSC-Regular.ttf",
		"C:\\Windows\\Fonts\\simsun.ttc",
		"C:\\Windows\\Fonts\\simhei.ttf",
		"C:\\Windows\\Fonts\\NotoSansSC-VF.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\Deng.ttf",
		"C:\\Windows\\Fonts\\CascadiaMono.ttf",
		"C:\\Windows\\Fonts\\consola.ttf",
		"C:\\Windows\\Fonts\\segoeui.ttf"
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

static int __xuiTerminalDemoRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_terminal_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_terminal_demo_t*)pUser;
	if ( pWidget == NULL || pDraw == NULL || pDemo == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(232, 241, 250, 255));
	}
	if ( pDemo->tProxy.drawRoundRectFill != NULL ) {
		tPanel = (xui_rect_t){28.0f, 22.0f, tRect.fW - 56.0f, tRect.fH - 44.0f};
		(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tPanel, 6.0f, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static int __xuiTerminalDemoAddLabel(xui_terminal_demo_t* pDemo, const char* sText, xui_rect_t tRect, xui_widget pParent, uint32_t iColor, uint32_t iFlags, xui_widget* ppLabel)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = iFlags | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pLabel, tRect);
	(void)xuiWidgetSetHitTestVisible(pLabel, 0);
	iRet = xuiWidgetAddChild(pParent, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	if ( ppLabel != NULL ) *ppLabel = pLabel;
	return XUI_OK;
}

static void __xuiTerminalDemoInput(xui_widget pWidget, const uint8_t* pData, int iSize, void* pUser)
{
	xui_terminal_demo_t* pDemo = (xui_terminal_demo_t*)pUser;

	(void)pWidget;
	(void)pData;
	if ( pDemo != NULL && iSize > 0 ) {
		pDemo->iInputBytes += iSize;
	}
}

static void __xuiTerminalDemoResize(xui_widget pWidget, int iColumns, int iRows, void* pUser)
{
	xui_terminal_demo_t* pDemo = (xui_terminal_demo_t*)pUser;

	(void)pWidget;
	if ( pDemo != NULL ) {
		pDemo->iResizeCount++;
		pDemo->iLastCols = iColumns;
		pDemo->iLastRows = iRows;
	}
}

static int __xuiTerminalDemoCreateUi(xui_terminal_demo_t* pDemo)
{
	xui_terminal_desc_t tDesc;
	xui_terminal_session_desc_t tSessionDesc;
	int iRet;
	int i;
	char sLine[96];

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiTerminalDemoRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiTerminalDemoAddLabel(pDemo, "XUI Terminal", (xui_rect_t){48.0f, 38.0f, 180.0f, 24.0f}, pDemo->pRoot, XUI_COLOR_RGBA(45, 58, 78, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE, NULL);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.iColumns = 86;
	tDesc.iRows = 22;
	tDesc.iScrollbackLimit = 256;
	tDesc.fCellWidth = 0.0f;
	tDesc.fCellHeight = 16.0f;
	tDesc.fPadding = 8.0f;
	tDesc.iBackgroundColor = XUI_COLOR_RGBA(17, 24, 34, 255);
	tDesc.iForegroundColor = XUI_COLOR_RGBA(220, 232, 244, 255);
	tDesc.iCursorColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	iRet = xuiTerminalCreate(pDemo->pContext, &pDemo->pTerminal, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pTerminal, (xui_rect_t){48.0f, 72.0f, 664.0f, 350.0f});
	(void)xuiTerminalSetInputCallback(pDemo->pTerminal, __xuiTerminalDemoInput, pDemo);
	(void)xuiTerminalSetResizeCallback(pDemo->pTerminal, __xuiTerminalDemoResize, pDemo);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pTerminal);
	if ( iRet != XUI_OK ) return iRet;

	(void)xuiTerminalWriteText(pDemo->pTerminal, "\x1b[1;36mXUI Terminal\x1b[0m\r\n");
	(void)xuiTerminalWriteText(pDemo->pTerminal, "frontend parser, screen buffer, scrollback, colors, and fake session.\r\n\r\n");
	(void)xuiTerminalWriteText(pDemo->pTerminal, "\x1b[32mgreen\x1b[0m  \x1b[33myellow\x1b[0m  \x1b[38;5;196m256-color red\x1b[0m  \x1b[38;2;120;180;255mtrue color\x1b[0m\r\n");
	(void)xuiTerminalWriteText(pDemo->pTerminal, "rewrite demo: progress 0%");
	(void)xuiTerminalWriteText(pDemo->pTerminal, "\rrewrite demo: progress 100%\r\n");
	for ( i = 0; i < 18; i++ ) {
		snprintf(sLine, sizeof(sLine), "scrollback line %02d\r\n", i + 1);
		(void)xuiTerminalWriteText(pDemo->pTerminal, sLine);
	}
	(void)xuiTerminalFlush(pDemo->pTerminal);

	memset(&tSessionDesc, 0, sizeof(tSessionDesc));
	tSessionDesc.iSize = sizeof(tSessionDesc);
	tSessionDesc.bEcho = 1;
	tSessionDesc.sPrompt = "$ ";
	pDemo->pSession = xuiTerminalCreateFakeSession(&tSessionDesc);
	if ( pDemo->pSession == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	iRet = xuiTerminalAttachSession(pDemo->pTerminal, pDemo->pSession);
	if ( iRet != XUI_OK ) return iRet;

	return __xuiTerminalDemoAddLabel(pDemo, "input=0 resize=0 cols=0 rows=0 changes=0", (xui_rect_t){48.0f, 432.0f, 664.0f, 22.0f}, pDemo->pRoot, XUI_COLOR_RGBA(74, 88, 108, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE, &pDemo->pStatus);
}

static int __xuiTerminalDemoCreateAssets(xui_terminal_demo_t* pDemo)
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
	sFontPath = __xuiTerminalDemoFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiTerminalDemoCreateUi(pDemo);
}

static void __xuiTerminalDemoDestroyAssets(xui_terminal_demo_t* pDemo)
{
	if ( pDemo->pSession != NULL ) {
		xuiTerminalSessionDestroy(pDemo->pSession);
		pDemo->pSession = NULL;
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

static void __xuiTerminalDemoRunChecks(xui_terminal_demo_t* pDemo, int bAutoRun)
{
	char sText[4096];
	int iOk;

	if ( pDemo == NULL || !bAutoRun || pDemo->bExerciseDone ) return;
	iOk = (xuiTerminalInputText(pDemo->pTerminal, "help") == XUI_OK);
	iOk = iOk && (xuiTerminalInputText(pDemo->pTerminal, "\r") == XUI_OK);
	iOk = iOk && (xuiTerminalSetBracketedPaste(pDemo->pTerminal, 1) == XUI_OK);
	iOk = iOk && (xuiTerminalPasteText(pDemo->pTerminal, "pasted text") == XUI_OK);
	iOk = iOk && (xuiTerminalSelectAll(pDemo->pTerminal) == XUI_OK);
	iOk = iOk && (xuiTerminalGetSelectionText(pDemo->pTerminal, sText, (int)sizeof(sText)) > 0);
	iOk = iOk && (xuiTerminalCopySelection(pDemo->pTerminal) == XUI_OK);
	pDemo->bDynamicOK = iOk;
	pDemo->bExerciseDone = 1;
}

static void __xuiTerminalDemoUpdateStatus(xui_terminal_demo_t* pDemo)
{
	char sText[160];

	if ( pDemo == NULL || pDemo->pStatus == NULL ) return;
	snprintf(sText, sizeof(sText), "input=%d resize=%d cols=%d rows=%d changes=%d",
		pDemo->iInputBytes,
		pDemo->iResizeCount,
		xuiTerminalGetColumns(pDemo->pTerminal),
		xuiTerminalGetRows(pDemo->pTerminal),
		xuiTerminalGetChangeCount(pDemo->pTerminal));
	(void)xuiLabelSetText(pDemo->pStatus, sText);
}

static int __xuiTerminalDemoFrame(void* pUser)
{
	xui_terminal_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_terminal_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) xgeQuit();
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiTerminalDemoRunChecks(pDemo, bAutoRun);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiTerminalDemoUpdateStatus(pDemo);
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bLayoutOK = (xuiTerminalGetColumns(pDemo->pTerminal) > 0 && xuiTerminalGetRows(pDemo->pTerminal) > 0);
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
		printf("xui_terminal final-summary frames=%d create=%d layout=%d dynamic=%d cols=%d rows=%d input=%d changes=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bDynamicOK,
			xuiTerminalGetColumns(pDemo->pTerminal), xuiTerminalGetRows(pDemo->pTerminal),
			pDemo->iInputBytes, xuiTerminalGetChangeCount(pDemo->pTerminal),
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_terminal_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiTerminalDemoParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiTerminalDemoUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Terminal";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_terminal: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiTerminalDemoCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_terminal: create assets failed: %d\n", iRet);
		__xuiTerminalDemoDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	tDemo.bCreateOK = 1;
	iRet = xgeRun(__xuiTerminalDemoFrame, &tDemo);
	__xuiTerminalDemoDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK &&
	        ((tDemo.iMaxFrames <= 0 && tDemo.fMaxSeconds <= 0.0) || tDemo.bDynamicOK)) ? 0 : 1;
}
