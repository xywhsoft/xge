#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 780
#define DEMO_TARGET_H 500
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f

typedef struct xui_virtual_joystick_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pJoystick;
	xge_scene_t tScene;
	xui_virtual_joystick_state_t tState;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	uint32_t iLastButtons;
	int iChangeCount;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bChannelOK;
	int bPointerOK;
	int bAtlasOK;
} xui_virtual_joystick_demo_t;

static void __xuiVirtualJoystickDemoUsage(void)
{
	printf("usage: xui_virtual_joystick [--frames N] [--seconds N]\n");
}

static int __xuiVirtualJoystickDemoParseArgs(xui_virtual_joystick_demo_t* pDemo, int argc, char** argv)
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
			__xuiVirtualJoystickDemoUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiVirtualJoystickDemoFindTtf(void)
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

static const char* __xuiVirtualJoystickDemoSourceName(int iSource)
{
	switch ( iSource ) {
	case XUI_VIRTUAL_JOYSTICK_SOURCE_POINTER: return "pointer";
	case XUI_VIRTUAL_JOYSTICK_SOURCE_CHANNEL: return "channel";
	case XUI_VIRTUAL_JOYSTICK_SOURCE_PROGRAM: return "program";
	default: return "none";
	}
}

static int __xuiVirtualJoystickDemoRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_virtual_joystick_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;
	xui_rect_t tCard;
	xui_rect_t tAxis;
	xui_rect_t tKnob;
	char sText[256];
	float fCenterX;
	float fCenterY;

	(void)iStateId;
	pDemo = (xui_virtual_joystick_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(229, 237, 247, 255));
	tPanel = (xui_rect_t){28.0f, 22.0f, tRect.fW - 56.0f, tRect.fH - 44.0f};
	(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tPanel, 7.0f, XUI_COLOR_RGBA(247, 250, 254, 255));
	(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, "XUI VirtualJoystick",
		(xui_rect_t){48.0f, 40.0f, 300.0f, 26.0f}, XUI_COLOR_RGBA(35, 50, 72, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, "Drag the joystick, touch it, or hold WASD / arrow keys.",
		(xui_rect_t){48.0f, 68.0f, 500.0f, 24.0f}, XUI_COLOR_RGBA(82, 101, 126, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);

	tCard = (xui_rect_t){430.0f, 120.0f, 270.0f, 250.0f};
	(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tCard, 6.0f, XUI_COLOR_RGBA(237, 243, 250, 255));
	(void)pDemo->tProxy.drawRoundRectStroke(&pDemo->tProxy, pDraw, tCard, 6.0f, 1.0f, XUI_COLOR_RGBA(145, 174, 210, 255));
	snprintf(sText, sizeof(sText), "x=%.2f  y=%.2f", pDemo->tState.fX, pDemo->tState.fY);
	(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, sText,
		(xui_rect_t){452.0f, 142.0f, 220.0f, 24.0f}, XUI_COLOR_RGBA(38, 52, 74, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	snprintf(sText, sizeof(sText), "magnitude=%.2f  angle=%.2f", pDemo->tState.fMagnitude, pDemo->tState.fAngle);
	(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, sText,
		(xui_rect_t){452.0f, 170.0f, 230.0f, 24.0f}, XUI_COLOR_RGBA(38, 52, 74, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	snprintf(sText, sizeof(sText), "source=%s  changes=%d", __xuiVirtualJoystickDemoSourceName(pDemo->tState.iSource), pDemo->iChangeCount);
	(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, sText,
		(xui_rect_t){452.0f, 198.0f, 230.0f, 24.0f}, XUI_COLOR_RGBA(38, 52, 74, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);

	tAxis = (xui_rect_t){492.0f, 250.0f, 120.0f, 120.0f};
	(void)pDemo->tProxy.drawCircleFill(&pDemo->tProxy, pDraw, tAxis.fX + 60.0f, tAxis.fY + 60.0f, 60.0f, XUI_COLOR_RGBA(255, 255, 255, 255));
	(void)pDemo->tProxy.drawCircleStroke(&pDemo->tProxy, pDraw, tAxis.fX + 60.0f, tAxis.fY + 60.0f, 60.0f, 1.0f, XUI_COLOR_RGBA(170, 190, 215, 255));
	(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, (xui_rect_t){tAxis.fX + 8.0f, tAxis.fY + 59.0f, 104.0f, 2.0f}, XUI_COLOR_RGBA(198, 213, 230, 255));
	(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, (xui_rect_t){tAxis.fX + 59.0f, tAxis.fY + 8.0f, 2.0f, 104.0f}, XUI_COLOR_RGBA(198, 213, 230, 255));
	fCenterX = tAxis.fX + 60.0f + pDemo->tState.fX * 48.0f;
	fCenterY = tAxis.fY + 60.0f + pDemo->tState.fY * 48.0f;
	tKnob = (xui_rect_t){fCenterX - 8.0f, fCenterY - 8.0f, 16.0f, 16.0f};
	(void)pDemo->tProxy.drawCircleFill(&pDemo->tProxy, pDraw, fCenterX, fCenterY, 8.0f, XUI_COLOR_RGBA(47, 128, 237, 255));
	(void)pDemo->tProxy.drawCircleStroke(&pDemo->tProxy, pDraw, fCenterX, fCenterY, 8.0f, 2.0f, XUI_COLOR_RGBA(255, 255, 255, 245));

	snprintf(sText, sizeof(sText), "atlas=%d layout=%d channel=%d pointer=%d", pDemo->bAtlasOK, pDemo->bLayoutOK, pDemo->bChannelOK, pDemo->bPointerOK);
	(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, sText,
		(xui_rect_t){48.0f, 444.0f, 660.0f, 24.0f}, XUI_COLOR_RGBA(58, 75, 99, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	(void)tKnob;
	return XUI_OK;
}

static void __xuiVirtualJoystickDemoChanged(xui_widget pWidget, const xui_virtual_joystick_state_t* pState, void* pUser)
{
	xui_virtual_joystick_demo_t* pDemo;

	(void)pWidget;
	pDemo = (xui_virtual_joystick_demo_t*)pUser;
	if ( (pDemo != NULL) && (pState != NULL) ) {
		pDemo->tState = *pState;
		pDemo->iChangeCount++;
		if ( pDemo->pRoot != NULL ) {
			(void)xuiWidgetInvalidate(pDemo->pRoot, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
	}
}

static int __xuiVirtualJoystickDemoCreateUi(xui_virtual_joystick_demo_t* pDemo)
{
	xui_virtual_joystick_desc_t tDesc;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiVirtualJoystickDemoRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fRadius = 86.0f;
	tDesc.fKnobSize = 56.0f;
	tDesc.fDeadZone = 0.07f;
	tDesc.bUseBuiltinAtlas = 1;
	tDesc.onChange = __xuiVirtualJoystickDemoChanged;
	tDesc.pChangeUser = pDemo;
	iRet = xuiVirtualJoystickCreate(pDemo->pContext, &pDemo->pJoystick, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pJoystick, (xui_rect_t){78.0f, 180.0f, 210.0f, 210.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pJoystick);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bAtlasOK = xuiVirtualJoystickGetUseBuiltinAtlas(pDemo->pJoystick) != 0;
	return XUI_OK;
}

static int __xuiVirtualJoystickDemoCreateAssets(xui_virtual_joystick_demo_t* pDemo)
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
	sFontPath = __xuiVirtualJoystickDemoFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiVirtualJoystickDemoCreateUi(pDemo);
}

static void __xuiVirtualJoystickDemoDestroyAssets(xui_virtual_joystick_demo_t* pDemo)
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

static uint32_t __xuiVirtualJoystickDemoReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiVirtualJoystickDemoSendButtonTransitions(xui_virtual_joystick_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
{
	int iRet;

	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iPressed & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_RIGHT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iPressed & XUI_POINTER_BUTTON_MIDDLE) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_MIDDLE, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_MIDDLE) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_MIDDLE, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiVirtualJoystickDemoSetKeyboardChannels(xui_virtual_joystick_demo_t* pDemo)
{
	int iRet;
	int bLeft;
	int bRight;
	int bUp;
	int bDown;

	bLeft = xgeKeyDown('A') || xgeKeyDown(XGE_KEY_LEFT);
	bRight = xgeKeyDown('D') || xgeKeyDown(XGE_KEY_RIGHT);
	bUp = xgeKeyDown('W') || xgeKeyDown(XGE_KEY_UP);
	bDown = xgeKeyDown('S') || xgeKeyDown(XGE_KEY_DOWN);
	iRet = xuiVirtualJoystickSetChannel(pDemo->pJoystick, XUI_VIRTUAL_JOYSTICK_CHANNEL_LEFT, bLeft, 1.0f, 1);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiVirtualJoystickSetChannel(pDemo->pJoystick, XUI_VIRTUAL_JOYSTICK_CHANNEL_RIGHT, bRight, 1.0f, 1);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiVirtualJoystickSetChannel(pDemo->pJoystick, XUI_VIRTUAL_JOYSTICK_CHANNEL_UP, bUp, 1.0f, 1);
	if ( iRet != XUI_OK ) return iRet;
	return xuiVirtualJoystickSetChannel(pDemo->pJoystick, XUI_VIRTUAL_JOYSTICK_CHANNEL_DOWN, bDown, 1.0f, 1);
}

static int __xuiVirtualJoystickDemoHandleMouseInput(xui_virtual_joystick_demo_t* pDemo)
{
	float fX;
	float fY;
	float fUiX;
	float fUiY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	fUiX = fX - DEMO_OFFSET_X;
	fUiY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiVirtualJoystickDemoReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, fUiX, fUiY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiVirtualJoystickDemoSendButtonTransitions(pDemo, fUiX, fUiY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiVirtualJoystickDemoSetKeyboardChannels(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiVirtualJoystickDemoSendTouchPoint(xui_virtual_joystick_demo_t* pDemo, int iPhase, const xge_touch_point_t* pPoint)
{
	float fX;
	float fY;

	if ( pDemo == NULL || pPoint == NULL || pPoint->bChanged == 0 ) {
		return XUI_OK;
	}
	fX = pPoint->fX - DEMO_OFFSET_X;
	fY = pPoint->fY - DEMO_OFFSET_Y;
	switch ( iPhase ) {
	case XGE_TOUCH_BEGIN:
		(void)xuiInputPointerMoveEx(pDemo->pContext, pPoint->iId, XUI_POINTER_TYPE_TOUCH, fX, fY, 0);
		return xuiInputPointerDownEx(pDemo->pContext, pPoint->iId, XUI_POINTER_TYPE_TOUCH, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	case XGE_TOUCH_MOVE:
	case XGE_TOUCH_STATIONARY:
		return xuiInputPointerMoveEx(pDemo->pContext, pPoint->iId, XUI_POINTER_TYPE_TOUCH, fX, fY, XUI_POINTER_BUTTON_LEFT);
	case XGE_TOUCH_END:
		return xuiInputPointerUpEx(pDemo->pContext, pPoint->iId, XUI_POINTER_TYPE_TOUCH, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	case XGE_TOUCH_CANCEL:
		return xuiInputPointerCancelEx(pDemo->pContext, pPoint->iId, XUI_POINTER_TYPE_TOUCH);
	default:
		return XUI_OK;
	}
}

static int __xuiVirtualJoystickDemoSceneEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	xui_virtual_joystick_demo_t* pDemo;
	const xge_touch_event_t* pTouch;
	int i;
	int iRet;

	if ( pScene == NULL || pEvent == NULL ) return XGE_OK;
	pDemo = (xui_virtual_joystick_demo_t*)pScene->pUser;
	if ( pDemo == NULL ) return XGE_OK;
	if ( pEvent->iType != XGE_EVENT_TOUCH_BEGIN &&
	     pEvent->iType != XGE_EVENT_TOUCH_MOVE &&
	     pEvent->iType != XGE_EVENT_TOUCH_END &&
	     pEvent->iType != XGE_EVENT_TOUCH_CANCEL ) {
		return XGE_OK;
	}
	pTouch = (const xge_touch_event_t*)pEvent->pData;
	if ( pTouch == NULL ) return XGE_OK;
	for ( i = 0; i < pTouch->iCount; i++ ) {
		iRet = __xuiVirtualJoystickDemoSendTouchPoint(pDemo, pTouch->arrPoints[i].iPhase, &pTouch->arrPoints[i]);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XGE_OK;
}

static void __xuiVirtualJoystickDemoRunChecks(xui_virtual_joystick_demo_t* pDemo, int bAutoRun)
{
	xui_rect_t tBase;
	xui_rect_t tWorld;
	xui_virtual_joystick_state_t tState;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pJoystick != NULL);
	pDemo->bLayoutOK = (xuiWidgetGetRect(pDemo->pJoystick).fW > 180.0f) && (xuiVirtualJoystickGetBaseRect(pDemo->pJoystick).fW > 150.0f);
	if ( !bAutoRun || pDemo->bExerciseDone || !pDemo->bLayoutOK ) {
		if ( !bAutoRun ) {
			pDemo->bChannelOK = 1;
			pDemo->bPointerOK = 1;
		}
		return;
	}
	(void)xuiVirtualJoystickSetChannel(pDemo->pJoystick, XUI_VIRTUAL_JOYSTICK_CHANNEL_RIGHT, 1, 1.0f, 1);
	(void)xuiVirtualJoystickSetChannel(pDemo->pJoystick, XUI_VIRTUAL_JOYSTICK_CHANNEL_UP, 1, 1.0f, 1);
	(void)xuiVirtualJoystickGetState(pDemo->pJoystick, &tState);
	pDemo->bChannelOK = tState.iSource == XUI_VIRTUAL_JOYSTICK_SOURCE_CHANNEL && tState.fX > 0.60f && tState.fY < -0.60f;
	(void)xuiVirtualJoystickClearChannels(pDemo->pJoystick, 1);
	tBase = xuiVirtualJoystickGetBaseRect(pDemo->pJoystick);
	tWorld = xuiWidgetGetWorldRect(pDemo->pJoystick);
	(void)xuiInputPointerMoveEx(pDemo->pContext, 303, XUI_POINTER_TYPE_TOUCH, tWorld.fX + tBase.fX + tBase.fW, tWorld.fY + tBase.fY + tBase.fH * 0.5f, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerDownEx(pDemo->pContext, 303, XUI_POINTER_TYPE_TOUCH, tWorld.fX + tBase.fX + tBase.fW, tWorld.fY + tBase.fY + tBase.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiVirtualJoystickGetState(pDemo->pJoystick, &tState);
	pDemo->bPointerOK = (xuiGetPointerCaptureEx(pDemo->pContext, 303, XUI_POINTER_TYPE_TOUCH) == pDemo->pJoystick) &&
		(tState.iSource == XUI_VIRTUAL_JOYSTICK_SOURCE_POINTER) && (tState.fX > 0.90f);
	(void)xuiInputPointerUpEx(pDemo->pContext, 303, XUI_POINTER_TYPE_TOUCH, tWorld.fX + tBase.fX + tBase.fW, tWorld.fY + tBase.fY + tBase.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	pDemo->bPointerOK = pDemo->bPointerOK && (xuiGetPointerCaptureEx(pDemo->pContext, 303, XUI_POINTER_TYPE_TOUCH) == NULL);
	pDemo->bExerciseDone = 1;
}

static int __xuiVirtualJoystickDemoFrame(void* pUser)
{
	xui_virtual_joystick_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_virtual_joystick_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiVirtualJoystickDemoHandleMouseInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiVirtualJoystickDemoRunChecks(pDemo, bAutoRun);
	(void)xuiVirtualJoystickGetState(pDemo->pJoystick, &pDemo->tState);
	(void)xuiWidgetInvalidate(pDemo->pRoot, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 237, 247, 255));
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
	if ( ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ||
	     ((pDemo->fMaxSeconds > 0.0) && (xgeTimer() >= pDemo->fMaxSeconds)) ) {
		memset(&tStats, 0, sizeof(tStats));
		tStats.iSize = sizeof(tStats);
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		printf("xui_virtual_joystick final-summary frames=%d create=%d layout=%d channel=%d pointer=%d atlas=%d changes=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bChannelOK, pDemo->bPointerOK, pDemo->bAtlasOK,
			xuiVirtualJoystickGetChangeCount(pDemo->pJoystick), tStats.iUpdatedCaches, tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_virtual_joystick_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiVirtualJoystickDemoParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiVirtualJoystickDemoUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI VirtualJoystick";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_virtual_joystick: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiVirtualJoystickDemoCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_virtual_joystick: create failed: %d\n", iRet);
		__xuiVirtualJoystickDemoDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	memset(&tDemo.tScene, 0, sizeof(tDemo.tScene));
	tDemo.tScene.pUser = &tDemo;
	tDemo.tScene.onEvent = __xuiVirtualJoystickDemoSceneEvent;
	(void)xgeSceneSet(&tDemo.tScene);
	iRet = xgeRun(__xuiVirtualJoystickDemoFrame, &tDemo);
	__xuiVirtualJoystickDemoDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK &&
	        tDemo.bCreateOK &&
	        tDemo.bLayoutOK &&
	        ((tDemo.iMaxFrames <= 0 && tDemo.fMaxSeconds <= 0.0) || (tDemo.bChannelOK && tDemo.bPointerOK && tDemo.bAtlasOK))) ? 0 : 1;
}
