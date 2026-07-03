#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	860
#define DEMO_TARGET_H	500
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define DEMO_POINT_MAX	16
#define DEMO_TRAIL_MAX	32

typedef struct xui_multitouch_point_t {
	int bUsed;
	int bDown;
	uint64_t iPointerId;
	int iPointerType;
	float fX;
	float fY;
	float arrTrailX[DEMO_TRAIL_MAX];
	float arrTrailY[DEMO_TRAIL_MAX];
	int iTrailCount;
	uint32_t iColor;
} xui_multitouch_point_t;

typedef struct xui_multitouch_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pPad;
	xge_scene_t tScene;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	xui_multitouch_point_t arrPoints[DEMO_POINT_MAX];
	int iDownCount;
	int iMoveCount;
	int iUpCount;
	int iCancelCount;
	int iMouseEventCount;
	int iTouchEventCount;
	int iRawTouchEventCount;
	int iActiveCount;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bEventOK;
	int bCaptureOK;
} xui_multitouch_demo_t;

static const uint32_t g_arrPointColors[] = {
	XUI_COLOR_RGBA(47, 128, 214, 230),
	XUI_COLOR_RGBA(31, 166, 116, 230),
	XUI_COLOR_RGBA(237, 137, 54, 230),
	XUI_COLOR_RGBA(128, 94, 214, 230),
	XUI_COLOR_RGBA(214, 72, 105, 230),
	XUI_COLOR_RGBA(41, 172, 188, 230)
};

static void __xuiMultiTouchUsage(void)
{
	printf("usage: xui_multitouch [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiMultiTouchParseArgs(xui_multitouch_demo_t* pDemo, int argc, char** argv)
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
			__xuiMultiTouchUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiMultiTouchFindTtf(void)
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

static int __xuiMultiTouchActiveCount(xui_multitouch_demo_t* pDemo)
{
	int i;
	int iCount;

	iCount = 0;
	for ( i = 0; i < DEMO_POINT_MAX; i++ ) {
		if ( pDemo->arrPoints[i].bUsed && pDemo->arrPoints[i].bDown ) {
			iCount++;
		}
	}
	return iCount;
}

static const char* __xuiMultiTouchPointerTypeName(int iPointerType)
{
	switch ( iPointerType ) {
	case XUI_POINTER_TYPE_TOUCH: return "touch";
	case XUI_POINTER_TYPE_PEN: return "pen";
	default: return "mouse";
	}
}

static xui_multitouch_point_t* __xuiMultiTouchFindPoint(xui_multitouch_demo_t* pDemo, uint64_t iPointerId, int iPointerType, int bCreate)
{
	xui_multitouch_point_t* pReusable;
	xui_multitouch_point_t* pPoint;
	int i;
	int iColorIndex;

	pReusable = NULL;
	for ( i = 0; i < DEMO_POINT_MAX; i++ ) {
		pPoint = &pDemo->arrPoints[i];
		if ( pPoint->bUsed &&
		     (pPoint->iPointerId == iPointerId) &&
		     (pPoint->iPointerType == iPointerType) ) {
			return pPoint;
		}
		if ( pReusable == NULL && (!pPoint->bUsed || !pPoint->bDown) ) {
			pReusable = pPoint;
		}
	}
	if ( !bCreate || pReusable == NULL ) {
		return NULL;
	}
	memset(pReusable, 0, sizeof(*pReusable));
	iColorIndex = (int)(iPointerId % (uint64_t)(sizeof(g_arrPointColors) / sizeof(g_arrPointColors[0])));
	pReusable->bUsed = 1;
	pReusable->iPointerId = iPointerId;
	pReusable->iPointerType = iPointerType;
	pReusable->iColor = g_arrPointColors[iColorIndex];
	return pReusable;
}

static void __xuiMultiTouchPushTrail(xui_multitouch_point_t* pPoint, float fX, float fY)
{
	int i;

	if ( pPoint == NULL ) return;
	if ( pPoint->iTrailCount < DEMO_TRAIL_MAX ) {
		pPoint->arrTrailX[pPoint->iTrailCount] = fX;
		pPoint->arrTrailY[pPoint->iTrailCount] = fY;
		pPoint->iTrailCount++;
		return;
	}
	for ( i = 1; i < DEMO_TRAIL_MAX; i++ ) {
		pPoint->arrTrailX[i - 1] = pPoint->arrTrailX[i];
		pPoint->arrTrailY[i - 1] = pPoint->arrTrailY[i];
	}
	pPoint->arrTrailX[DEMO_TRAIL_MAX - 1] = fX;
	pPoint->arrTrailY[DEMO_TRAIL_MAX - 1] = fY;
}

static void __xuiMultiTouchRecordPoint(xui_multitouch_demo_t* pDemo, uint64_t iPointerId, int iPointerType, float fX, float fY, int bDown)
{
	xui_multitouch_point_t* pPoint;

	pPoint = __xuiMultiTouchFindPoint(pDemo, iPointerId, iPointerType, 1);
	if ( pPoint == NULL ) return;
	pPoint->fX = fX;
	pPoint->fY = fY;
	pPoint->bDown = bDown ? 1 : 0;
	__xuiMultiTouchPushTrail(pPoint, fX, fY);
	pDemo->iActiveCount = __xuiMultiTouchActiveCount(pDemo);
	if ( pDemo->pPad != NULL ) {
		(void)xuiWidgetInvalidate(pDemo->pPad, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	if ( pDemo->pRoot != NULL ) {
		(void)xuiWidgetInvalidate(pDemo->pRoot, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
}

static int __xuiMultiTouchPadEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_multitouch_demo_t* pDemo;
	xui_context pContext;

	pDemo = (xui_multitouch_demo_t*)pUser;
	if ( pDemo == NULL || pEvent == NULL || pEvent->iPhase != XUI_EVENT_PHASE_TARGET ) {
		return XUI_OK;
	}
	pContext = xuiWidgetGetContext(pWidget);
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_DOWN:
		__xuiMultiTouchRecordPoint(pDemo, pEvent->iPointerId, pEvent->iPointerType, pEvent->fX, pEvent->fY, 1);
		if ( pEvent->iPointerType == XUI_POINTER_TYPE_TOUCH ) pDemo->iTouchEventCount++;
		else pDemo->iMouseEventCount++;
		pDemo->iDownCount++;
		(void)xuiSetPointerCapture(pContext, pWidget);
		if ( xuiGetPointerCaptureEx(pContext, pEvent->iPointerId, pEvent->iPointerType) == pWidget ) {
			pDemo->bCaptureOK = 1;
		}
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_MOVE:
		__xuiMultiTouchRecordPoint(pDemo, pEvent->iPointerId, pEvent->iPointerType, pEvent->fX, pEvent->fY,
			(pEvent->iButtons & XUI_POINTER_BUTTON_LEFT) != 0);
		if ( pEvent->iPointerType == XUI_POINTER_TYPE_TOUCH ) pDemo->iTouchEventCount++;
		else pDemo->iMouseEventCount++;
		pDemo->iMoveCount++;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_UP:
		__xuiMultiTouchRecordPoint(pDemo, pEvent->iPointerId, pEvent->iPointerType, pEvent->fX, pEvent->fY, 0);
		if ( pEvent->iPointerType == XUI_POINTER_TYPE_TOUCH ) pDemo->iTouchEventCount++;
		else pDemo->iMouseEventCount++;
		pDemo->iUpCount++;
		(void)xuiReleasePointerCapture(pContext, pWidget);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		return XUI_EVENT_DISPATCH_STOP;
	default:
		return XUI_OK;
	}
}

static int __xuiMultiTouchRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_multitouch_demo_t* pDemo;
	xui_rect_t tRect;
	char sStatus[256];

	(void)iStateId;
	pDemo = (xui_multitouch_demo_t*)pUser;
	if ( pDemo == NULL ) return XUI_OK;
	tRect = xuiWidgetGetContentRect(pWidget);
	(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(246, 249, 253, 255));
	(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, "XUI MultiTouch",
		(xui_rect_t){28.0f, 22.0f, 240.0f, 26.0f}, XUI_COLOR_RGBA(38, 52, 74, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont,
		"Touch with multiple fingers, or drag the mouse to verify the compatible single-pointer path.",
		(xui_rect_t){28.0f, 50.0f, 760.0f, 24.0f}, XUI_COLOR_RGBA(74, 92, 118, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	snprintf(sStatus, sizeof(sStatus), "active=%d down=%d move=%d up=%d cancel=%d touchEvents=%d mouseEvents=%d capture=%d",
		pDemo->iActiveCount, pDemo->iDownCount, pDemo->iMoveCount, pDemo->iUpCount, pDemo->iCancelCount,
		pDemo->iTouchEventCount, pDemo->iMouseEventCount, pDemo->bCaptureOK);
	(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, sStatus,
		(xui_rect_t){28.0f, 452.0f, 800.0f, 24.0f}, XUI_COLOR_RGBA(59, 76, 100, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	return XUI_OK;
}

static int __xuiMultiTouchPadRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_multitouch_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tWorld;
	xui_multitouch_point_t* pPoint;
	float fX;
	float fY;
	float fPrevX;
	float fPrevY;
	uint32_t iColor;
	char sLabel[64];
	int i;
	int j;

	(void)iStateId;
	pDemo = (xui_multitouch_demo_t*)pUser;
	if ( pDemo == NULL ) return XUI_OK;
	tRect = xuiWidgetGetContentRect(pWidget);
	tWorld = xuiWidgetGetWorldRect(pWidget);
	(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(255, 255, 255, 255));
	(void)pDemo->tProxy.drawRectStroke(&pDemo->tProxy, pDraw, tRect, 1.0f, XUI_COLOR_RGBA(131, 164, 203, 255));
	for ( fX = 40.0f; fX < tRect.fW; fX += 40.0f ) {
		(void)pDemo->tProxy.drawLine(&pDemo->tProxy, pDraw, fX, 0.0f, fX, tRect.fH, 1.0f, XUI_COLOR_RGBA(229, 235, 244, 255));
	}
	for ( fY = 40.0f; fY < tRect.fH; fY += 40.0f ) {
		(void)pDemo->tProxy.drawLine(&pDemo->tProxy, pDraw, 0.0f, fY, tRect.fW, fY, 1.0f, XUI_COLOR_RGBA(229, 235, 244, 255));
	}
	if ( pDemo->iActiveCount == 0 ) {
		(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, "No active pointer",
			(xui_rect_t){0.0f, tRect.fH * 0.5f - 14.0f, tRect.fW, 28.0f}, XUI_COLOR_RGBA(119, 139, 164, 255), XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	for ( i = 0; i < DEMO_POINT_MAX; i++ ) {
		pPoint = &pDemo->arrPoints[i];
		if ( !pPoint->bUsed ) continue;
		iColor = pPoint->bDown ? pPoint->iColor : XUI_COLOR_RGBA(128, 146, 168, 150);
		for ( j = 1; j < pPoint->iTrailCount; j++ ) {
			fPrevX = pPoint->arrTrailX[j - 1] - tWorld.fX;
			fPrevY = pPoint->arrTrailY[j - 1] - tWorld.fY;
			fX = pPoint->arrTrailX[j] - tWorld.fX;
			fY = pPoint->arrTrailY[j] - tWorld.fY;
			(void)pDemo->tProxy.drawLine(&pDemo->tProxy, pDraw, fPrevX, fPrevY, fX, fY, 3.0f, XUI_COLOR_RGBA(96, 126, 166, 95));
		}
		fX = pPoint->fX - tWorld.fX;
		fY = pPoint->fY - tWorld.fY;
		(void)pDemo->tProxy.drawCircleFill(&pDemo->tProxy, pDraw, fX, fY, pPoint->bDown ? 22.0f : 14.0f, iColor);
		(void)pDemo->tProxy.drawCircleStroke(&pDemo->tProxy, pDraw, fX, fY, pPoint->bDown ? 22.0f : 14.0f, 2.0f, XUI_COLOR_RGBA(255, 255, 255, 235));
		snprintf(sLabel, sizeof(sLabel), "%s %llu", __xuiMultiTouchPointerTypeName(pPoint->iPointerType), (unsigned long long)pPoint->iPointerId);
		(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, sLabel,
			(xui_rect_t){fX + 28.0f, fY - 13.0f, 160.0f, 26.0f}, XUI_COLOR_RGBA(37, 53, 75, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	return XUI_OK;
}

static int __xuiMultiTouchCreateUi(xui_multitouch_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiMultiTouchRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pPad);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pPad, (xui_rect_t){28.0f, 92.0f, 804.0f, 344.0f});
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pPad, __xuiMultiTouchPadRender, pDemo);
	(void)xuiWidgetSetEventCallback(pDemo->pPad, __xuiMultiTouchPadEvent, pDemo);
	(void)xuiWidgetSetEventInterest(pDemo->pPad, XUI_EVENT_MASK_POINTER | XUI_EVENT_MASK_CAPTURE, 1);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pPad);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_OK;
}

static uint32_t __xuiMultiTouchReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiMultiTouchSendButtonTransitions(xui_multitouch_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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
	return XUI_OK;
}

static int __xuiMultiTouchHandleMouseInput(xui_multitouch_demo_t* pDemo)
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
	iButtons = __xuiMultiTouchReadButtons();
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
	iRet = __xuiMultiTouchSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiMultiTouchSendTouchPoint(xui_multitouch_demo_t* pDemo, int iPhase, const xge_touch_point_t* pPoint)
{
	float fX;
	float fY;

	if ( pDemo == NULL || pPoint == NULL || pPoint->bChanged == 0 ) {
		return XUI_OK;
	}
	fX = pPoint->fX - DEMO_OFFSET_X;
	fY = pPoint->fY - DEMO_OFFSET_Y;
	pDemo->iRawTouchEventCount++;
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
		__xuiMultiTouchRecordPoint(pDemo, pPoint->iId, XUI_POINTER_TYPE_TOUCH, fX, fY, 0);
		pDemo->iCancelCount++;
		return xuiInputPointerCancelEx(pDemo->pContext, pPoint->iId, XUI_POINTER_TYPE_TOUCH);
	default:
		return XUI_OK;
	}
}

static int __xuiMultiTouchSceneEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	xui_multitouch_demo_t* pDemo;
	const xge_touch_event_t* pTouch;
	int i;
	int iRet;

	if ( pScene == NULL || pEvent == NULL ) return XGE_OK;
	pDemo = (xui_multitouch_demo_t*)pScene->pUser;
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
		iRet = __xuiMultiTouchSendTouchPoint(pDemo, pTouch->arrPoints[i].iPhase, &pTouch->arrPoints[i]);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XGE_OK;
}

static int __xuiMultiTouchCreateAssets(xui_multitouch_demo_t* pDemo)
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
	sFontPath = __xuiMultiTouchFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiMultiTouchCreateUi(pDemo);
}

static void __xuiMultiTouchDestroyAssets(xui_multitouch_demo_t* pDemo)
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

static void __xuiMultiTouchRunChecks(xui_multitouch_demo_t* pDemo, int bAutoRun)
{
	xui_rect_t tPad;
	float fX1;
	float fY1;
	float fX2;
	float fY2;

	if ( pDemo == NULL ) return;
	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pPad != NULL);
	pDemo->bLayoutOK = (xuiWidgetGetRect(pDemo->pPad).fW > 700.0f) && (xuiWidgetGetRect(pDemo->pPad).fH > 300.0f);
	if ( !bAutoRun ) {
		pDemo->bEventOK = 1;
		pDemo->bCaptureOK = 1;
		return;
	}
	if ( pDemo->bExerciseDone || !pDemo->bLayoutOK ) {
		pDemo->bEventOK = (pDemo->iDownCount >= 2) && (pDemo->iMoveCount >= 1) && (pDemo->iUpCount >= 1);
		return;
	}
	tPad = xuiWidgetGetWorldRect(pDemo->pPad);
	fX1 = tPad.fX + 120.0f;
	fY1 = tPad.fY + 110.0f;
	fX2 = tPad.fX + 520.0f;
	fY2 = tPad.fY + 210.0f;
	(void)xuiInputPointerMoveEx(pDemo->pContext, 101, XUI_POINTER_TYPE_TOUCH, fX1, fY1, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerDownEx(pDemo->pContext, 101, XUI_POINTER_TYPE_TOUCH, fX1, fY1, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	pDemo->bCaptureOK = pDemo->bCaptureOK && (xuiGetPointerCaptureEx(pDemo->pContext, 101, XUI_POINTER_TYPE_TOUCH) == pDemo->pPad);
	(void)xuiInputPointerMoveEx(pDemo->pContext, 202, XUI_POINTER_TYPE_TOUCH, fX2, fY2, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerDownEx(pDemo->pContext, 202, XUI_POINTER_TYPE_TOUCH, fX2, fY2, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	pDemo->bCaptureOK = pDemo->bCaptureOK &&
		(xuiGetPointerCaptureEx(pDemo->pContext, 101, XUI_POINTER_TYPE_TOUCH) == pDemo->pPad) &&
		(xuiGetPointerCaptureEx(pDemo->pContext, 202, XUI_POINTER_TYPE_TOUCH) == pDemo->pPad);
	(void)xuiInputPointerMoveEx(pDemo->pContext, 101, XUI_POINTER_TYPE_TOUCH, fX1 + 150.0f, fY1 + 70.0f, XUI_POINTER_BUTTON_LEFT);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerUpEx(pDemo->pContext, 101, XUI_POINTER_TYPE_TOUCH, fX1 + 150.0f, fY1 + 70.0f, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	pDemo->bCaptureOK = pDemo->bCaptureOK &&
		(xuiGetPointerCaptureEx(pDemo->pContext, 101, XUI_POINTER_TYPE_TOUCH) == NULL) &&
		(xuiGetPointerCaptureEx(pDemo->pContext, 202, XUI_POINTER_TYPE_TOUCH) == pDemo->pPad);
	(void)xuiInputPointerCancelEx(pDemo->pContext, 202, XUI_POINTER_TYPE_TOUCH);
	__xuiMultiTouchRecordPoint(pDemo, 202, XUI_POINTER_TYPE_TOUCH, fX2, fY2, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	pDemo->bCaptureOK = pDemo->bCaptureOK && (xuiGetPointerCaptureEx(pDemo->pContext, 202, XUI_POINTER_TYPE_TOUCH) == NULL);
	pDemo->bEventOK = (pDemo->iDownCount >= 2) && (pDemo->iMoveCount >= 1) && (pDemo->iUpCount >= 1);
	pDemo->bExerciseDone = 1;
}

static int __xuiMultiTouchFrame(void* pUser)
{
	xui_multitouch_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_multitouch_demo_t*)pUser;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiMultiTouchHandleMouseInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiMultiTouchRunChecks(pDemo, bAutoRun);
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
	if ( iRet == XGE_OK ) {
		iRet = xgeEnd();
	}
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ||
	     ((pDemo->fMaxSeconds > 0.0) && (xgeTimer() >= pDemo->fMaxSeconds)) ) {
		memset(&tStats, 0, sizeof(tStats));
		tStats.iSize = sizeof(tStats);
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		printf("xui_multitouch final-summary frames=%d create=%d layout=%d events=%d capture=%d active=%d down=%d move=%d up=%d cancel=%d rawTouch=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bEventOK, pDemo->bCaptureOK,
			pDemo->iActiveCount, pDemo->iDownCount, pDemo->iMoveCount, pDemo->iUpCount, pDemo->iCancelCount,
			pDemo->iRawTouchEventCount, tStats.iUpdatedCaches, tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_multitouch_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	tDemo.bCaptureOK = 1;
	iRet = __xuiMultiTouchParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) {
		return 0;
	}
	if ( iRet != XGE_OK ) {
		__xuiMultiTouchUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI MultiTouch";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_multitouch: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiMultiTouchCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_multitouch: create failed: %d\n", iRet);
		__xuiMultiTouchDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	memset(&tDemo.tScene, 0, sizeof(tDemo.tScene));
	tDemo.tScene.pUser = &tDemo;
	tDemo.tScene.onEvent = __xuiMultiTouchSceneEvent;
	(void)xgeSceneSet(&tDemo.tScene);
	iRet = xgeRun(__xuiMultiTouchFrame, &tDemo);
	__xuiMultiTouchDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK &&
	        tDemo.bCreateOK &&
	        tDemo.bLayoutOK &&
	        ((tDemo.iMaxFrames <= 0 && tDemo.fMaxSeconds <= 0.0) || (tDemo.bEventOK && tDemo.bCaptureOK))) ? 0 : 1;
}
