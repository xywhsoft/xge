#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef XUI_IME_DEMO_MODE
#error XUI_IME_DEMO_MODE must be defined before including main.inc.c
#endif

#ifndef XUI_IME_DEMO_NAME
#define XUI_IME_DEMO_NAME "XUI IME"
#endif

#ifndef XUI_IME_DEMO_DESCRIPTION
#define XUI_IME_DEMO_DESCRIPTION "Windows IME presentation mode"
#endif

#define IME_DEMO_W 900
#define IME_DEMO_H 680
#define IME_DEMO_OFFSET_X 10.0f
#define IME_DEMO_OFFSET_Y 20.0f

typedef struct xui_ime_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pStatus;
	xui_widget pInput;
	xui_widget pTextEdit;
	xui_widget pCodeEdit;
	int iFrame;
	int iMaxFrames;
} xui_ime_demo_t;

static const char* __xuiImeDemoFindFont(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\Consolas-with-Yahei_Nerd_Font.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\NotoSansSC-VF.ttf",
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

static int __xuiImeDemoRootRender(xui_widget pWidget, xui_draw_context pDraw,
	uint32_t iStateId, void* pUser)
{
	xui_ime_demo_t* pDemo;
	xui_rect_t tRect;

	(void)iStateId;
	pDemo = (xui_ime_demo_t*)pUser;
	if ( pWidget == NULL || pDraw == NULL || pDemo == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) return XUI_OK;
	if ( pDemo->tProxy.drawRectFill == NULL ) return XUI_OK;
	return pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect,
		XUI_COLOR_RGBA(239, 244, 250, 255));
}

static int __xuiImeDemoAddLabel(xui_ime_demo_t* pDemo, xui_widget* ppWidget,
	const char* sText, float fHeight, uint32_t iColor)
{
	xui_label_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE;
	iRet = xuiLabelCreate(pDemo->pContext, ppWidget, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetSizeMode(*ppWidget, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	(void)xuiWidgetSetPreferredSize(*ppWidget, (xui_vec2_t){0.0f, fHeight});
	return xuiWidgetAddChild(pDemo->pRoot, *ppWidget);
}

static int __xuiImeDemoCreateUi(xui_ime_demo_t* pDemo)
{
	xui_widget pTitle;
	xui_input_desc_t tInput;
	xui_text_edit_desc_t tTextEdit;
	xui_code_edit_desc_t tCodeEdit;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) { fprintf(stderr, "root create failed: %d\n", iRet); return iRet; }
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiImeDemoRootRender, pDemo);
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetSizeMode(pDemo->pRoot, XUI_SIZE_FILL, XUI_SIZE_FILL);
	(void)xuiWidgetSetPadding(pDemo->pRoot, (xui_thickness_t){18.0f, 16.0f, 18.0f, 16.0f});
	(void)xuiWidgetSetGap(pDemo->pRoot, 8.0f);
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, IME_DEMO_W, IME_DEMO_H});
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) { fprintf(stderr, "set root failed: %d\n", iRet); return iRet; }

	iRet = __xuiImeDemoAddLabel(pDemo, &pTitle, XUI_IME_DEMO_NAME " - " XUI_IME_DEMO_DESCRIPTION,
		30.0f, XUI_COLOR_RGBA(31, 58, 87, 255));
	if ( iRet != XUI_OK ) { fprintf(stderr, "title label failed: %d\n", iRet); return iRet; }
	iRet = __xuiImeDemoAddLabel(pDemo, &pDemo->pStatus,
		"Focus a control and type with a Windows IME. Tab changes focus.",
		24.0f, XUI_COLOR_RGBA(78, 96, 118, 255));
	if ( iRet != XUI_OK ) { fprintf(stderr, "status label failed: %d\n", iRet); return iRet; }

	memset(&tInput, 0, sizeof(tInput));
	tInput.iSize = sizeof(tInput);
	tInput.pFont = pDemo->pFont;
	tInput.sPlaceholder = "Single-line input: type pinyin here";
	tInput.iMaxLength = 0;
	iRet = xuiInputCreate(pDemo->pContext, &pDemo->pInput, &tInput);
	if ( iRet != XUI_OK ) { fprintf(stderr, "input create failed: %d\n", iRet); return iRet; }
	(void)xuiWidgetSetSizeMode(pDemo->pInput, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	(void)xuiWidgetSetPreferredSize(pDemo->pInput, (xui_vec2_t){0.0f, 38.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pInput);
	if ( iRet != XUI_OK ) { fprintf(stderr, "input add failed: %d\n", iRet); return iRet; }

	memset(&tTextEdit, 0, sizeof(tTextEdit));
	tTextEdit.iSize = sizeof(tTextEdit);
	tTextEdit.pFont = pDemo->pFont;
	tTextEdit.sText = "TextEdit marked-text test:\n1. Put the caret inside 12345.\n2. Start IME composition.\n3. Press Backspace inside the IME.\nThe document must not be deleted twice.";
	tTextEdit.bWordWrap = 1;
	tTextEdit.bLineNumbers = 1;
	iRet = xuiTextEditCreate(pDemo->pContext, &pDemo->pTextEdit, &tTextEdit);
	if ( iRet != XUI_OK ) { fprintf(stderr, "text edit create failed: %d\n", iRet); return iRet; }
	(void)xuiWidgetSetSizeMode(pDemo->pTextEdit, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	(void)xuiWidgetSetPreferredSize(pDemo->pTextEdit, (xui_vec2_t){0.0f, 190.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pTextEdit);
	if ( iRet != XUI_OK ) { fprintf(stderr, "text edit add failed: %d\n", iRet); return iRet; }

	memset(&tCodeEdit, 0, sizeof(tCodeEdit));
	tCodeEdit.iSize = sizeof(tCodeEdit);
	tCodeEdit.pFont = pDemo->pFont;
	tCodeEdit.sLanguage = "python";
	tCodeEdit.sText =
		"# CodeEdit IME test\n"
		"def greet(name):\n"
		"\tmessage = f\"Hello, {name}\"\n"
		"\treturn message\n"
		"\n"
		"# Start composition at the caret. Preedit text must not alter the document.\n";
	tCodeEdit.bShowLineNumbers = 1;
	tCodeEdit.bShowFoldMargin = 1;
	tCodeEdit.bShowMarkerMargin = 1;
	tCodeEdit.iTabColumns = 4;
	tCodeEdit.iIndentColumns = 4;
	iRet = xuiCodeEditCreate(pDemo->pContext, &pDemo->pCodeEdit, &tCodeEdit);
	if ( iRet != XUI_OK ) { fprintf(stderr, "code edit create failed: %d\n", iRet); return iRet; }
	(void)xuiWidgetSetSizeMode(pDemo->pCodeEdit, XUI_SIZE_FILL, XUI_SIZE_FILL);
	(void)xuiWidgetSetFlex(pDemo->pCodeEdit, 1.0f, 1.0f);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pCodeEdit);
	if ( iRet != XUI_OK ) { fprintf(stderr, "code edit add failed: %d\n", iRet); return iRet; }

	(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pInput);
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) fprintf(stderr, "initial layout failed: %d\n", iRet);
	return iRet;
}

static int __xuiImeDemoCreateAssets(xui_ime_demo_t* pDemo)
{
	xui_surface_desc_t tSurface;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) { fprintf(stderr, "xuiCreate failed: %d\n", iRet); return iRet; }
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) { fprintf(stderr, "xuiSetProxy failed: %d\n", iRet); return iRet; }
	iRet = xuiInputViewport(pDemo->pContext, (float)IME_DEMO_W, (float)IME_DEMO_H);
	if ( iRet != XUI_OK ) { fprintf(stderr, "xuiInputViewport failed: %d\n", iRet); return iRet; }

	memset(&tSurface, 0, sizeof(tSurface));
	tSurface.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurface.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurface.iWidth = IME_DEMO_W;
	tSurface.iHeight = IME_DEMO_H;
	tSurface.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurface);
	if ( iRet != XUI_OK ) { fprintf(stderr, "surfaceCreate failed: %d\n", iRet); return iRet; }

	sFontPath = __xuiImeDemoFindFont();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont,
		sFontPath, 16.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) { fprintf(stderr, "fontLoadFile failed: %d\n", iRet); return iRet; }
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	iRet = __xuiImeDemoCreateUi(pDemo);
	if ( iRet != XUI_OK ) fprintf(stderr, "create UI failed: %d\n", iRet);
	return iRet;
}

static void __xuiImeDemoDestroy(xui_ime_demo_t* pDemo)
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

static void __xuiImeDemoUpdateStatus(xui_ime_demo_t* pDemo)
{
	char sStatus[256];
	xge_ime_candidate_info_t tInfo;
	int iRet;

	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	iRet = xgeImeCandidateGetInfo(&tInfo);
	if ( iRet == XGE_OK && tInfo.bVisible ) {
		snprintf(sStatus, sizeof(sStatus),
			"Mode: %s | candidates: %d | selected: %d | XUI selectable: %s",
			XUI_IME_DEMO_NAME, tInfo.iCount, tInfo.iSelection,
			tInfo.bCanSelect ? "yes" : "no (native fallback)");
	} else {
		snprintf(sStatus, sizeof(sStatus),
			"Mode: %s | no candidate UI is active", XUI_IME_DEMO_NAME);
	}
	(void)xuiLabelSetText(pDemo->pStatus, sStatus);
}

static int __xuiImeDemoFrame(void* pUser)
{
	xui_ime_demo_t* pDemo;
	xui_rect_i_t tFull;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;

	pDemo = (xui_ime_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( pDemo->pContext == NULL ) {
		iRet = __xuiImeDemoCreateAssets(pDemo);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = xuiProxyXgePumpInputRect(pDemo->pContext,
		(xui_rect_t){IME_DEMO_OFFSET_X, IME_DEMO_OFFSET_Y, IME_DEMO_W, IME_DEMO_H});
	if ( iRet != XUI_OK ) return iRet;
	__xuiImeDemoUpdateStatus(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget,
		XUI_COLOR_RGBA(239, 244, 250, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFull = (xui_rect_i_t){0, 0, IME_DEMO_W, IME_DEMO_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFull, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(18, 23, 31, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, IME_DEMO_W, IME_DEMO_H};
	tDst = (xui_rect_t){IME_DEMO_OFFSET_X, IME_DEMO_OFFSET_Y, IME_DEMO_W, IME_DEMO_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst,
		XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames ) xgeQuit();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_ime_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	if ( argc == 3 && strcmp(argv[1], "--frames") == 0 ) {
		tDemo.iMaxFrames = atoi(argv[2]);
		if ( tDemo.iMaxFrames <= 0 ) return 1;
	} else if ( argc != 1 ) {
		printf("usage: %s [--frames N]\n", argv[0]);
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = IME_DEMO_W + 20;
	tDesc.iHeight = IME_DEMO_H + 50;
	tDesc.sTitle = XUI_IME_DEMO_NAME;
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) return 1;
	iRet = xgeImeSetMode(XUI_IME_DEMO_MODE);
	if ( iRet != XGE_OK ) {
		printf("xgeImeSetMode failed: %d\n", iRet);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiImeDemoFrame, &tDemo);
	__xuiImeDemoDestroy(&tDemo);
	xgeUnit();
	if ( tDemo.iMaxFrames > 0 ) {
		printf("xui_ime_demo mode=%d frames=%d result=%d\n",
			XUI_IME_DEMO_MODE, tDemo.iFrame, iRet);
	}
	return iRet == XGE_OK ? 0 : 1;
}
