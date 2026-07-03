#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 900
#define DEMO_TARGET_H 520
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define CANVAS_COUNT 2
#define LABEL_COUNT 6

typedef struct xui_canvas_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pCanvas[CANVAS_COUNT];
	xui_widget pLabel[LABEL_COUNT];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bDrawOK;
	int bScrollOK;
	int bPenOK;
	int bContentDrawn;
} xui_canvas_demo_t;

static void __xuiCanvasDemoUsage(void)
{
	printf("usage: xui_canvas [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiCanvasDemoParseArgs(xui_canvas_demo_t* pDemo, int argc, char** argv)
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
			__xuiCanvasDemoUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiCanvasDemoFindTtf(void)
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

static int __xuiCanvasDemoRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_canvas_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_canvas_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(233, 241, 250, 255));
	}
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		tPanel = (xui_rect_t){28.0f, 22.0f, tRect.fW - 56.0f, tRect.fH - 44.0f};
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tPanel, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static int __xuiCanvasDemoAddLabel(xui_canvas_demo_t* pDemo, int iIndex, const char* sText, xui_rect_t tRect, uint32_t iColor)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( (pDemo == NULL) || (iIndex < 0) || (iIndex >= LABEL_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetRect(pLabel, tRect);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[iIndex] = pLabel;
	return XUI_OK;
}

static int __xuiCanvasDemoAddCanvas(xui_canvas_demo_t* pDemo, int iIndex, xui_rect_t tRect, int iMode, float fCanvasW, float fCanvasH, float fOffsetX, float fOffsetY)
{
	xui_canvas_desc_t tDesc;
	xui_widget pCanvas;
	int iRet;

	if ( (pDemo == NULL) || (iIndex < 0) || (iIndex >= CANVAS_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fCanvasWidth = fCanvasW;
	tDesc.fCanvasHeight = fCanvasH;
	tDesc.fOffsetX = fOffsetX;
	tDesc.fOffsetY = fOffsetY;
	tDesc.iPolicyX = XUI_SCROLLBAR_POLICY_AUTO;
	tDesc.iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
	tDesc.iScrollbarMode = iMode;
	tDesc.iWheelAxis = XUI_WHEEL_AXIS_BOTH;
	tDesc.iCornerMode = (iMode == XUI_SCROLLBAR_MODE_FULL) ? XUI_SCROLL_FRAME_CORNER_GRIP : XUI_SCROLL_FRAME_CORNER_AUTO;
	tDesc.bContentDragEnabled = 1;
	tDesc.bPenEnabled = 1;
	tDesc.fPenWidth = (iMode == XUI_SCROLLBAR_MODE_FULL) ? 4.0f : 3.0f;
	tDesc.fScrollbarSize = (iMode == XUI_SCROLLBAR_MODE_FULL) ? 16.0f : 8.0f;
	tDesc.fMinThumbSize = 24.0f;
	tDesc.fWheelStep = 40.0f;
	tDesc.iClearColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	tDesc.iBackgroundColor = XUI_COLOR_RGBA(244, 248, 253, 255);
	tDesc.iBorderColor = XUI_COLOR_RGBA(140, 174, 210, 255);
	tDesc.iPenColor = XUI_COLOR_RGBA(47, 128, 214, 255);
	tDesc.iTrackColor = XUI_COLOR_RGBA(221, 232, 244, 255);
	tDesc.iThumbColor = XUI_COLOR_RGBA(133, 168, 205, 255);
	tDesc.iHoverColor = XUI_COLOR_RGBA(103, 150, 198, 255);
	tDesc.iActiveColor = XUI_COLOR_RGBA(47, 128, 214, 255);
	tDesc.iCornerColor = XUI_COLOR_RGBA(232, 240, 249, 255);
	tDesc.iGripColor = XUI_COLOR_RGBA(122, 151, 184, 255);
	iRet = xuiCanvasCreate(pDemo->pContext, &pCanvas, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetRect(pCanvas, tRect);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pDemo->pRoot, pCanvas);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pCanvas);
		return iRet;
	}
	pDemo->pCanvas[iIndex] = pCanvas;
	return XUI_OK;
}

static int __xuiCanvasDemoDrawGrid(xui_widget pCanvas, float fWidth, float fHeight)
{
	int iRet;
	float fX;
	float fY;

	for ( fX = 0.0f; fX <= fWidth; fX += 40.0f ) {
		iRet = xuiCanvasDrawLine(pCanvas, fX, 0.0f, fX, fHeight, 1.0f, XUI_COLOR_RGBA(229, 235, 244, 255));
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( fY = 0.0f; fY <= fHeight; fY += 40.0f ) {
		iRet = xuiCanvasDrawLine(pCanvas, 0.0f, fY, fWidth, fY, 1.0f, XUI_COLOR_RGBA(229, 235, 244, 255));
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiCanvasDemoDrawPrimary(xui_canvas_demo_t* pDemo)
{
	xui_mesh_vertex_t arrVertices[3];
	uint32_t arrIndices[3];
	xui_widget pCanvas;
	int iRet;

	pCanvas = pDemo->pCanvas[0];
	iRet = xuiCanvasClear(pCanvas, XUI_COLOR_RGBA(255, 255, 255, 255));
	if ( iRet == XUI_OK ) iRet = __xuiCanvasDemoDrawGrid(pCanvas, 980.0f, 640.0f);
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawText(pCanvas, pDemo->pFont, "Persistent drawing surface", (xui_rect_t){32.0f, 26.0f, 260.0f, 28.0f}, XUI_COLOR_RGBA(37, 53, 75, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawRectFill(pCanvas, (xui_rect_t){42.0f, 82.0f, 220.0f, 112.0f}, XUI_COLOR_RGBA(222, 238, 255, 255));
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawRectStroke(pCanvas, (xui_rect_t){42.0f, 82.0f, 220.0f, 112.0f}, 1.0f, XUI_COLOR_RGBA(126, 168, 208, 255));
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawText(pCanvas, pDemo->pFont, "Shapes, text and pen strokes", (xui_rect_t){58.0f, 100.0f, 188.0f, 26.0f}, XUI_COLOR_RGBA(63, 82, 108, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawText(pCanvas, pDemo->pFont, "are retained in the texture.", (xui_rect_t){58.0f, 132.0f, 188.0f, 26.0f}, XUI_COLOR_RGBA(63, 82, 108, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawCircleFill(pCanvas, 410.0f, 150.0f, 46.0f, XUI_COLOR_RGBA(42, 166, 117, 210));
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawCircleStroke(pCanvas, 410.0f, 150.0f, 46.0f, 3.0f, XUI_COLOR_RGBA(24, 128, 86, 255));
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawTriangleFill(pCanvas, (xui_vec2_t){604.0f, 80.0f}, (xui_vec2_t){720.0f, 190.0f}, (xui_vec2_t){548.0f, 212.0f}, XUI_COLOR_RGBA(239, 156, 54, 220));
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawTriangleStroke(pCanvas, (xui_vec2_t){604.0f, 80.0f}, (xui_vec2_t){720.0f, 190.0f}, (xui_vec2_t){548.0f, 212.0f}, 3.0f, XUI_COLOR_RGBA(190, 109, 32, 255));
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawLine(pCanvas, 72.0f, 320.0f, 880.0f, 462.0f, 5.0f, XUI_COLOR_RGBA(47, 128, 214, 230));
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawLine(pCanvas, 80.0f, 462.0f, 880.0f, 320.0f, 3.0f, XUI_COLOR_RGBA(212, 83, 116, 210));
	arrVertices[0] = (xui_mesh_vertex_t){704.0f, 388.0f, XUI_COLOR_RGBA(78, 96, 214, 255)};
	arrVertices[1] = (xui_mesh_vertex_t){884.0f, 522.0f, XUI_COLOR_RGBA(35, 172, 120, 255)};
	arrVertices[2] = (xui_mesh_vertex_t){632.0f, 554.0f, XUI_COLOR_RGBA(245, 170, 65, 255)};
	arrIndices[0] = 0;
	arrIndices[1] = 1;
	arrIndices[2] = 2;
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawMeshTriangles(pCanvas, arrVertices, 3, arrIndices, 3, 0);
	return iRet;
}

static int __xuiCanvasDemoDrawCompact(xui_canvas_demo_t* pDemo)
{
	xui_widget pCanvas;
	int iRet;

	pCanvas = pDemo->pCanvas[1];
	iRet = xuiCanvasClear(pCanvas, XUI_COLOR_RGBA(252, 254, 255, 255));
	if ( iRet == XUI_OK ) iRet = __xuiCanvasDemoDrawGrid(pCanvas, 760.0f, 500.0f);
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawText(pCanvas, pDemo->pFont, "Compact scrollbars + built-in pen", (xui_rect_t){120.0f, 72.0f, 320.0f, 26.0f}, XUI_COLOR_RGBA(37, 53, 75, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawRectFill(pCanvas, (xui_rect_t){164.0f, 132.0f, 310.0f, 126.0f}, XUI_COLOR_RGBA(232, 246, 239, 255));
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawRectStroke(pCanvas, (xui_rect_t){164.0f, 132.0f, 310.0f, 126.0f}, 1.0f, XUI_COLOR_RGBA(96, 184, 134, 255));
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawCircleFill(pCanvas, 560.0f, 220.0f, 38.0f, XUI_COLOR_RGBA(84, 144, 226, 220));
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawLine(pCanvas, 130.0f, 334.0f, 650.0f, 380.0f, 4.0f, XUI_COLOR_RGBA(124, 92, 220, 220));
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawText(pCanvas, pDemo->pFont, "Drag to draw, wheel to scroll.", (xui_rect_t){178.0f, 162.0f, 250.0f, 26.0f}, XUI_COLOR_RGBA(50, 76, 96, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet == XUI_OK ) iRet = xuiCanvasDrawText(pCanvas, pDemo->pFont, "Content coordinates are persistent.", (xui_rect_t){178.0f, 194.0f, 250.0f, 26.0f}, XUI_COLOR_RGBA(50, 76, 96, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	return iRet;
}

static int __xuiCanvasDemoDrawContent(xui_canvas_demo_t* pDemo)
{
	int iRet;

	iRet = __xuiCanvasDemoDrawPrimary(pDemo);
	if ( iRet == XUI_OK ) iRet = __xuiCanvasDemoDrawCompact(pDemo);
	return iRet;
}

static int __xuiCanvasDemoCreateUi(xui_canvas_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiCanvasDemoRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiCanvasDemoAddLabel(pDemo, 0, "XUI Canvas", (xui_rect_t){48.0f, 38.0f, 180.0f, 24.0f}, XUI_COLOR_RGBA(37, 53, 75, 255));
	if ( iRet == XUI_OK ) iRet = __xuiCanvasDemoAddLabel(pDemo, 1, "Full scrollbar surface", (xui_rect_t){48.0f, 66.0f, 220.0f, 22.0f}, XUI_COLOR_RGBA(67, 86, 112, 255));
	if ( iRet == XUI_OK ) iRet = __xuiCanvasDemoAddLabel(pDemo, 2, "Compact interactive canvas", (xui_rect_t){604.0f, 66.0f, 240.0f, 22.0f}, XUI_COLOR_RGBA(67, 86, 112, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCanvasDemoAddCanvas(pDemo, 0, (xui_rect_t){48.0f, 96.0f, 520.0f, 340.0f}, XUI_SCROLLBAR_MODE_FULL, 980.0f, 640.0f, 0.0f, 0.0f);
	if ( iRet == XUI_OK ) iRet = __xuiCanvasDemoAddCanvas(pDemo, 1, (xui_rect_t){604.0f, 96.0f, 260.0f, 340.0f}, XUI_SCROLLBAR_MODE_COMPACT, 760.0f, 500.0f, 120.0f, 60.0f);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCanvasDemoAddLabel(pDemo, 3, "API drawing: grid, text, geometry and mesh.", (xui_rect_t){48.0f, 448.0f, 460.0f, 22.0f}, XUI_COLOR_RGBA(67, 86, 112, 255));
	if ( iRet == XUI_OK ) iRet = __xuiCanvasDemoAddLabel(pDemo, 4, "Built-in pen persists strokes in content coordinates.", (xui_rect_t){604.0f, 448.0f, 310.0f, 22.0f}, XUI_COLOR_RGBA(67, 86, 112, 255));
	if ( iRet == XUI_OK ) iRet = __xuiCanvasDemoAddLabel(pDemo, 5, "Scroll, drag content, or draw with the left mouse button.", (xui_rect_t){48.0f, 476.0f, 640.0f, 22.0f}, XUI_COLOR_RGBA(94, 113, 138, 255));
	return iRet;
}

static uint32_t __xuiCanvasDemoReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiCanvasDemoSendButtonTransitions(xui_canvas_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiCanvasDemoHandleInput(xui_canvas_demo_t* pDemo)
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
	iButtons = __xuiCanvasDemoReadButtons();
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
	iRet = __xuiCanvasDemoSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiCanvasDemoRunChecks(xui_canvas_demo_t* pDemo, int bAutoRun)
{
	xui_rect_t tWorld;
	float fOffsetX;
	float fOffsetY;
	int iRet;

	if ( pDemo == NULL ) return;
	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pCanvas[0] != NULL) && (pDemo->pCanvas[1] != NULL) &&
	                   (xuiCanvasGetSurface(pDemo->pCanvas[0]) != NULL) && (xuiCanvasGetSurface(pDemo->pCanvas[1]) != NULL);
	pDemo->bLayoutOK = (xuiCanvasGetViewportWidget(pDemo->pCanvas[0]) != NULL) &&
	                   (xuiWidgetGetRect(pDemo->pCanvas[0]).fW > 500.0f) &&
	                   (xuiWidgetGetRect(pDemo->pCanvas[1]).fH > 320.0f);
	pDemo->bDrawOK = (xuiCanvasGetDrawCount(pDemo->pCanvas[0]) > 20) && (xuiCanvasGetDrawCount(pDemo->pCanvas[1]) > 10);
	iRet = xuiCanvasGetOffset(pDemo->pCanvas[1], &fOffsetX, &fOffsetY);
	pDemo->bScrollOK = (iRet == XUI_OK) && (fOffsetX > 0.0f) && (fOffsetY > 0.0f);
	if ( !bAutoRun ) {
		pDemo->bPenOK = 1;
		return;
	}
	if ( pDemo->bExerciseDone || !pDemo->bLayoutOK ) {
		return;
	}
	(void)xuiCanvasEnsureRectVisible(pDemo->pCanvas[0], (xui_rect_t){760.0f, 470.0f, 160.0f, 100.0f});
	iRet = xuiCanvasGetOffset(pDemo->pCanvas[0], &fOffsetX, &fOffsetY);
	if ( iRet == XUI_OK && fOffsetX > 0.0f && fOffsetY > 0.0f ) {
		pDemo->bScrollOK = 1;
	}
	tWorld = xuiWidgetGetWorldRect(xuiCanvasGetViewportWidget(pDemo->pCanvas[1]));
	(void)xuiInputPointerMove(pDemo->pContext, tWorld.fX + 82.0f, tWorld.fY + 128.0f, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerDown(pDemo->pContext, tWorld.fX + 82.0f, tWorld.fY + 128.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerMove(pDemo->pContext, tWorld.fX + 142.0f, tWorld.fY + 174.0f, XUI_POINTER_BUTTON_LEFT);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerUp(pDemo->pContext, tWorld.fX + 142.0f, tWorld.fY + 174.0f, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	pDemo->bPenOK = (xuiCanvasGetDrawCount(pDemo->pCanvas[1]) > 12);
	pDemo->bExerciseDone = 1;
}

static int __xuiCanvasDemoCreateAssets(xui_canvas_demo_t* pDemo)
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
	sFontPath = __xuiCanvasDemoFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiCanvasDemoCreateUi(pDemo);
}

static void __xuiCanvasDemoDestroyAssets(xui_canvas_demo_t* pDemo)
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

static int __xuiCanvasDemoFrame(void* pUser)
{
	xui_canvas_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_canvas_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	if ( !pDemo->bContentDrawn ) {
		iRet = __xuiCanvasDemoDrawContent(pDemo);
		if ( iRet != XUI_OK ) return iRet;
		pDemo->bContentDrawn = 1;
	}
	iRet = __xuiCanvasDemoHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiCanvasDemoRunChecks(pDemo, bAutoRun);
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
		printf("xui_canvas final-summary frames=%d create=%d layout=%d draw=%d scroll=%d pen=%d draws0=%d draws1=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bDrawOK, pDemo->bScrollOK, pDemo->bPenOK,
			xuiCanvasGetDrawCount(pDemo->pCanvas[0]), xuiCanvasGetDrawCount(pDemo->pCanvas[1]),
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_canvas_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiCanvasDemoParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiCanvasDemoUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Canvas";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_canvas: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiCanvasDemoCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_canvas: create assets failed: %d\n", iRet);
		__xuiCanvasDemoDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiCanvasDemoFrame, &tDemo);
	__xuiCanvasDemoDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bDrawOK &&
	        tDemo.bScrollOK && ((tDemo.iMaxFrames <= 0 && tDemo.fMaxSeconds <= 0.0) || tDemo.bPenOK)) ? 0 : 1;
}
