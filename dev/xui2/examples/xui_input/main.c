#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	760
#define DEMO_TARGET_H	420
#define DEMO_LOG_COUNT	8

typedef struct xui_input_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pButtonA;
	xui_widget pButtonB;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	uint32_t iLastButtons;
	int iClickA;
	int iClickB;
	char arrLog[DEMO_LOG_COUNT][96];
	int iLogCount;
} xui_input_demo_t;

static void __xuiInputUsage(void)
{
	printf("usage: xui_input [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiInputParseArgs(xui_input_demo_t* pDemo, int argc, char** argv)
{
	int i;

	if ( pDemo == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
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
			__xuiInputUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static void __xuiInputRect(float fX, float fY, float fW, float fH, xui_rect_t* pRect)
{
	pRect->fX = fX;
	pRect->fY = fY;
	pRect->fW = fW;
	pRect->fH = fH;
}

static const char* __xuiInputFindTtf(void)
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

static const char* __xuiInputEventName(int iType)
{
	switch ( iType ) {
	case XUI_EVENT_POINTER_ENTER:
		return "enter";
	case XUI_EVENT_POINTER_LEAVE:
		return "leave";
	case XUI_EVENT_POINTER_MOVE:
		return "move";
	case XUI_EVENT_POINTER_DOWN:
		return "down";
	case XUI_EVENT_POINTER_UP:
		return "up";
	case XUI_EVENT_POINTER_CLICK:
		return "click";
	case XUI_EVENT_POINTER_WHEEL:
		return "wheel";
	case XUI_EVENT_KEY_DOWN:
		return "key down";
	case XUI_EVENT_KEY_UP:
		return "key up";
	case XUI_EVENT_TEXT:
		return "text";
	case XUI_EVENT_FOCUS:
		return "focus";
	case XUI_EVENT_BLUR:
		return "blur";
	case XUI_EVENT_VIEWPORT:
		return "viewport";
	case XUI_EVENT_DPI:
		return "dpi";
	default:
		return "event";
	}
}

static const char* __xuiInputWidgetName(xui_input_demo_t* pDemo, xui_widget pWidget)
{
	if ( pWidget == pDemo->pButtonA ) {
		return "button A";
	}
	if ( pWidget == pDemo->pButtonB ) {
		return "button B";
	}
	if ( pWidget == pDemo->pRoot ) {
		return "root";
	}
	return "none";
}

static void __xuiInputPushLog(xui_input_demo_t* pDemo, const char* sText)
{
	int iMoveCount;

	if ( pDemo->iLogCount < DEMO_LOG_COUNT ) {
		strcpy(pDemo->arrLog[pDemo->iLogCount++], sText);
		return;
	}
	iMoveCount = DEMO_LOG_COUNT - 1;
	memmove(pDemo->arrLog, pDemo->arrLog + 1, sizeof(pDemo->arrLog[0]) * (size_t)iMoveCount);
	strcpy(pDemo->arrLog[DEMO_LOG_COUNT - 1], sText);
}

static void __xuiInputRecordEvents(xui_input_demo_t* pDemo)
{
	xui_event_t tEvent;
	char arrLine[96];
	int iRet;

	while ( (iRet = xuiPollEvent(pDemo->pContext, &tEvent)) == 1 ) {
		if ( (tEvent.iType == XUI_EVENT_POINTER_CLICK) && (tEvent.pTarget == pDemo->pButtonA) ) {
			pDemo->iClickA++;
		} else if ( (tEvent.iType == XUI_EVENT_POINTER_CLICK) && (tEvent.pTarget == pDemo->pButtonB) ) {
			pDemo->iClickB++;
		}
		if ( tEvent.iType == XUI_EVENT_TEXT ) {
			snprintf(arrLine, sizeof(arrLine), "%s %s U+%04X", __xuiInputEventName(tEvent.iType), __xuiInputWidgetName(pDemo, tEvent.pTarget), (unsigned int)tEvent.iCodepoint);
		} else if ( (tEvent.iType == XUI_EVENT_KEY_DOWN) || (tEvent.iType == XUI_EVENT_KEY_UP) ) {
			snprintf(arrLine, sizeof(arrLine), "%s %s key=%d", __xuiInputEventName(tEvent.iType), __xuiInputWidgetName(pDemo, tEvent.pTarget), tEvent.iKey);
		} else if ( tEvent.iType == XUI_EVENT_POINTER_WHEEL ) {
			snprintf(arrLine, sizeof(arrLine), "%s %s %.1f %.1f", __xuiInputEventName(tEvent.iType), __xuiInputWidgetName(pDemo, tEvent.pTarget), tEvent.fWheelX, tEvent.fWheelY);
		} else {
			snprintf(arrLine, sizeof(arrLine), "%s %s %.0f %.0f", __xuiInputEventName(tEvent.iType), __xuiInputWidgetName(pDemo, tEvent.pTarget), tEvent.fX, tEvent.fY);
		}
		__xuiInputPushLog(pDemo, arrLine);
	}
}

static uint32_t __xuiInputReadButtons(void)
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

static int __xuiInputSendButtonTransitions(xui_input_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
{
	int iRet;

	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	if ( (iPressed & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	if ( (iPressed & XUI_POINTER_BUTTON_MIDDLE) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_MIDDLE, iButtons);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	if ( (iReleased & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	if ( (iReleased & XUI_POINTER_BUTTON_MIDDLE) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_MIDDLE, iButtons);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiInputSendKeys(xui_input_demo_t* pDemo)
{
	static const int arrKeys[] = {
		XGE_KEY_ESCAPE,
		XGE_KEY_ENTER,
		XGE_KEY_TAB,
		XGE_KEY_BACKSPACE,
		XGE_KEY_DELETE,
		XGE_KEY_LEFT,
		XGE_KEY_RIGHT,
		XGE_KEY_UP,
		XGE_KEY_DOWN
	};
	uint32_t iText;
	int iRet;
	int i;

	for ( i = 0; i < (int)(sizeof(arrKeys) / sizeof(arrKeys[0])); i++ ) {
		if ( xgeKeyPressed(arrKeys[i]) ) {
			iRet = xuiInputKeyDown(pDemo->pContext, arrKeys[i], 0);
			if ( iRet != XUI_OK ) {
				return iRet;
			}
		}
		if ( xgeKeyReleased(arrKeys[i]) ) {
			iRet = xuiInputKeyUp(pDemo->pContext, arrKeys[i], 0);
			if ( iRet != XUI_OK ) {
				return iRet;
			}
		}
	}
	iText = xgeTextGet();
	if ( iText != 0 ) {
		iRet = xuiInputText(pDemo->pContext, iText);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiInputHandleInput(xui_input_demo_t* pDemo)
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
	iButtons = __xuiInputReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, fX - 20.0f, fY - 20.0f, iButtons);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiInputSendButtonTransitions(pDemo, fX - 20.0f, fY - 20.0f, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( (fWheelX != 0.0f) || (fWheelY != 0.0f) ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, fX - 20.0f, fY - 20.0f, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	iRet = __xuiInputSendKeys(pDemo);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	__xuiInputRecordEvents(pDemo);
	return XUI_OK;
}

static int __xuiInputDrawText(xui_input_demo_t* pDemo, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	return pDemo->tProxy.textDraw(&pDemo->tProxy, pDemo->pTarget, pDemo->pFont, sText, tRect, iColor, iFlags | XUI_TEXT_CLIP);
}

static int __xuiInputDrawButton(xui_input_demo_t* pDemo, xui_widget pWidget, const char* sName, int iClicks)
{
	xui_rect_t tRect;
	xui_rect_t tText;
	uint32_t iState;
	uint32_t iFill;
	uint32_t iStroke;
	char arrText[64];
	int iRet;

	tRect = xuiWidgetGetWorldRect(pWidget);
	iState = xuiWidgetGetInputState(pWidget);
	iFill = XUI_COLOR_RGBA(40, 46, 52, 255);
	iStroke = XUI_COLOR_RGBA(96, 108, 121, 255);
	if ( (iState & XUI_WIDGET_STATE_ACTIVE) != 0 ) {
		iFill = XUI_COLOR_RGBA(197, 124, 54, 255);
		iStroke = XUI_COLOR_RGBA(255, 216, 128, 255);
	} else if ( (iState & XUI_WIDGET_STATE_HOVER) != 0 ) {
		iFill = XUI_COLOR_RGBA(45, 95, 131, 255);
		iStroke = XUI_COLOR_RGBA(114, 205, 245, 255);
	}
	if ( (iState & XUI_WIDGET_STATE_FOCUS) != 0 ) {
		iStroke = XUI_COLOR_RGBA(118, 224, 151, 255);
	}
	iRet = pDemo->tProxy.shapeRoundRectFill(&pDemo->tProxy, pDemo->pTarget, tRect, 8.0f, iFill);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = pDemo->tProxy.shapeRoundRectStroke(&pDemo->tProxy, pDemo->pTarget, tRect, 8.0f, 2.0f, iStroke);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	snprintf(arrText, sizeof(arrText), "%s  clicks %d", sName, iClicks);
	__xuiInputRect(tRect.fX + 12.0f, tRect.fY + 14.0f, tRect.fW - 24.0f, 30.0f, &tText);
	return __xuiInputDrawText(pDemo, arrText, tText, XUI_COLOR_RGBA(238, 243, 247, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE);
}

static int __xuiInputDrawAll(xui_input_demo_t* pDemo)
{
	xui_rect_t tRect;
	char arrLine[128];
	int i;
	int iRet;

	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(16, 19, 23, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	__xuiInputRect(24.0f, 24.0f, 318.0f, 372.0f, &tRect);
	iRet = pDemo->tProxy.shapeRoundRectFill(&pDemo->tProxy, pDemo->pTarget, tRect, 8.0f, XUI_COLOR_RGBA(25, 30, 35, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = pDemo->tProxy.shapeRoundRectStroke(&pDemo->tProxy, pDemo->pTarget, tRect, 8.0f, 1.0f, XUI_COLOR_RGBA(72, 84, 94, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	__xuiInputRect(376.0f, 24.0f, 360.0f, 372.0f, &tRect);
	iRet = pDemo->tProxy.shapeRoundRectFill(&pDemo->tProxy, pDemo->pTarget, tRect, 8.0f, XUI_COLOR_RGBA(25, 30, 35, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = pDemo->tProxy.shapeRoundRectStroke(&pDemo->tProxy, pDemo->pTarget, tRect, 8.0f, 1.0f, XUI_COLOR_RGBA(72, 84, 94, 255));
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	__xuiInputRect(44.0f, 42.0f, 278.0f, 34.0f, &tRect);
	iRet = __xuiInputDrawText(pDemo, "XUI Input", tRect, XUI_COLOR_RGBA(245, 248, 250, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	iRet = __xuiInputDrawButton(pDemo, pDemo->pButtonA, "Button A", pDemo->iClickA);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xuiInputDrawButton(pDemo, pDemo->pButtonB, "Button B", pDemo->iClickB);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	snprintf(arrLine, sizeof(arrLine), "hover: %s", __xuiInputWidgetName(pDemo, xuiGetHoverWidget(pDemo->pContext)));
	__xuiInputRect(44.0f, 302.0f, 278.0f, 24.0f, &tRect);
	iRet = __xuiInputDrawText(pDemo, arrLine, tRect, XUI_COLOR_RGBA(184, 194, 202, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	snprintf(arrLine, sizeof(arrLine), "focus: %s", __xuiInputWidgetName(pDemo, xuiGetFocusWidget(pDemo->pContext)));
	__xuiInputRect(44.0f, 330.0f, 278.0f, 24.0f, &tRect);
	iRet = __xuiInputDrawText(pDemo, arrLine, tRect, XUI_COLOR_RGBA(184, 194, 202, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	__xuiInputRect(398.0f, 42.0f, 318.0f, 34.0f, &tRect);
	iRet = __xuiInputDrawText(pDemo, "Event Queue", tRect, XUI_COLOR_RGBA(245, 248, 250, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	for ( i = 0; i < pDemo->iLogCount; i++ ) {
		__xuiInputRect(398.0f, 88.0f + (float)i * 32.0f, 318.0f, 24.0f, &tRect);
		iRet = __xuiInputDrawText(pDemo, pDemo->arrLog[i], tRect, XUI_COLOR_RGBA(207, 216, 224, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xuiInputCreateAssets(xui_input_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) {
		printf("xui_input: xuiCreate failed: %d\n", iRet);
		return iRet;
	}
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) {
		printf("xui_input: xuiSetProxy failed: %d\n", iRet);
		return iRet;
	}
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H);
	if ( iRet != XUI_OK ) {
		printf("xui_input: xuiInputViewport failed: %d\n", iRet);
		return iRet;
	}
	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pButtonA);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pButtonB);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	(void)xuiWidgetSetRect(pDemo->pButtonA, (xui_rect_t){62.0f, 104.0f, 242.0f, 74.0f});
	(void)xuiWidgetSetRect(pDemo->pButtonB, (xui_rect_t){62.0f, 206.0f, 242.0f, 74.0f});
	(void)xuiWidgetSetFocusable(pDemo->pButtonA, 1);
	(void)xuiWidgetSetFocusable(pDemo->pButtonB, 1);
	(void)xuiWidgetAddChild(pDemo->pRoot, pDemo->pButtonA);
	(void)xuiWidgetAddChild(pDemo->pRoot, pDemo->pButtonB);

	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) {
		printf("xui_input: surfaceCreate failed: %d\n", iRet);
		return iRet;
	}
	sFontPath = __xuiInputFindTtf();
	if ( sFontPath == NULL ) {
		printf("xui_input: no usable system TTF font found\n");
		return XUI_ERROR_FILE_NOT_FOUND;
	}
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 20.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) {
		printf("xui_input: fontLoadFile failed: %d\n", iRet);
		return iRet;
	}
	xuiClearEvents(pDemo->pContext);
	return XUI_OK;
}

static void __xuiInputDestroyAssets(xui_input_demo_t* pDemo)
{
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
	}
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
	}
}

static int __xuiInputFrame(void* pUser)
{
	xui_input_demo_t* pDemo;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;

	pDemo = (xui_input_demo_t*)pUser;
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xuiInputHandleInput(pDemo);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = __xuiInputDrawAll(pDemo);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	xgeClear(XGE_COLOR_RGBA(10, 12, 15, 255));
	__xuiInputRect(0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H, &tSrc);
	__xuiInputRect(20.0f, 20.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H, &tDst);
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XGE_OK ) {
		iRet = xgeEnd();
	}
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	pDemo->iFrame++;
	if ( ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ||
	     ((pDemo->fMaxSeconds > 0.0) && (xgeTimer() >= pDemo->fMaxSeconds)) ) {
		xgeQuit();
	}
	return 0;
}

int main(int argc, char** argv)
{
	xui_input_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiInputParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) {
		return 0;
	}
	if ( iRet != XGE_OK ) {
		__xuiInputUsage();
		return 1;
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 800;
	tDesc.iHeight = 460;
	tDesc.sTitle = "xui_input";
	tDesc.iFlags = XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_input: xgeInit failed: %d\n", iRet);
		return 1;
	}

	iRet = __xuiInputCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		__xuiInputDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiInputFrame, &tDemo);
	__xuiInputDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK) ? 0 : 1;
}
