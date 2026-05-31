#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 900
#define DEMO_TARGET_H 450
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define LABEL_COUNT 3

#define XUI_TIMELINE_KEY_LEFT_SHIFT 340
#define XUI_TIMELINE_KEY_LEFT_CTRL 341
#define XUI_TIMELINE_KEY_LEFT_ALT 342
#define XUI_TIMELINE_KEY_LEFT_SUPER 343
#define XUI_TIMELINE_KEY_RIGHT_SHIFT 344
#define XUI_TIMELINE_KEY_RIGHT_CTRL 345
#define XUI_TIMELINE_KEY_RIGHT_ALT 346
#define XUI_TIMELINE_KEY_RIGHT_SUPER 347

typedef struct xui_timeline_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pTimeline;
	xui_widget pLabel[LABEL_COUNT];
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
	int iCurrentChanging;
	int iCurrentChanged;
	int iLayerChanged;
	int iFrameChanged;
	int iSpanChanged;
	int iLayerSelected;
	int iContextOpen;
	int iContextCommand;
	int iFrameClick;
	int iSelectionChanged;
	int iCustomPaint;
	int iBgLayer;
	int iPoseLayer;
	int iFxLayer;
	int iCameraLayer;
	int iIntroSpan;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bInteractionOK;
	int bScrollOK;
	int bPaintOK;
} xui_timeline_demo_t;

static void __xuiTimelineUsage(void)
{
	printf("usage: xui_timelineview [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiTimelineParseArgs(xui_timeline_demo_t* pDemo, int argc, char** argv)
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
			__xuiTimelineUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiTimelineFindTtf(void)
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

static int __xuiTimelineRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_timeline_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;
	(void)iStateId;
	pDemo = (xui_timeline_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
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

static void __xuiTimelineUpdateStatus(xui_timeline_demo_t* pDemo)
{
	char sText[256];
	if ( (pDemo == NULL) || (pDemo->pStatus == NULL) ) return;
	snprintf(sText, sizeof(sText), "frame=%d selections=%d current=%d/%d layer=%d frameOps=%d spanOps=%d clicks=%d",
		xuiTimeLineViewGetCurrentFrame(pDemo->pTimeline),
		xuiTimeLineViewGetSelectionCount(pDemo->pTimeline),
		pDemo->iCurrentChanging,
		pDemo->iCurrentChanged,
		pDemo->iLayerSelected,
		pDemo->iFrameChanged,
		pDemo->iSpanChanged,
		pDemo->iFrameClick);
	(void)xuiLabelSetText(pDemo->pStatus, sText);
}

static int __xuiTimelineCurrentChanging(xui_widget pWidget, int iOldFrame, int iNewFrame, void* pUser)
{
	xui_timeline_demo_t* pDemo;
	(void)pWidget;
	(void)iOldFrame;
	(void)iNewFrame;
	pDemo = (xui_timeline_demo_t*)pUser;
	if ( pDemo != NULL ) pDemo->iCurrentChanging++;
	return 1;
}

static void __xuiTimelineCurrentChanged(xui_widget pWidget, int iOldFrame, int iNewFrame, void* pUser)
{
	xui_timeline_demo_t* pDemo;
	(void)pWidget;
	(void)iOldFrame;
	(void)iNewFrame;
	pDemo = (xui_timeline_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iCurrentChanged++;
		__xuiTimelineUpdateStatus(pDemo);
	}
}

static int __xuiTimelineLayerChanging(xui_widget pWidget, int iLayer, int iChange, void* pUser)
{
	(void)pWidget;
	(void)iLayer;
	(void)iChange;
	(void)pUser;
	return 1;
}

static void __xuiTimelineLayerChanged(xui_widget pWidget, int iLayer, int iChange, void* pUser)
{
	xui_timeline_demo_t* pDemo;
	(void)pWidget;
	(void)iLayer;
	(void)iChange;
	pDemo = (xui_timeline_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iLayerChanged++;
		__xuiTimelineUpdateStatus(pDemo);
	}
}

static int __xuiTimelineFrameChanging(xui_widget pWidget, int iLayer, int iFrame, int iOldType, int iNewType, void* pUser)
{
	(void)pWidget;
	(void)iLayer;
	(void)iFrame;
	(void)iOldType;
	(void)iNewType;
	(void)pUser;
	return 1;
}

static void __xuiTimelineFrameChanged(xui_widget pWidget, int iLayer, int iFrame, int iOldType, int iNewType, void* pUser)
{
	xui_timeline_demo_t* pDemo;
	(void)pWidget;
	(void)iLayer;
	(void)iFrame;
	(void)iOldType;
	(void)iNewType;
	pDemo = (xui_timeline_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iFrameChanged++;
		__xuiTimelineUpdateStatus(pDemo);
	}
}

static int __xuiTimelineSpanChanging(xui_widget pWidget, int iSpanId, int iChange, void* pUser)
{
	(void)pWidget;
	(void)iSpanId;
	(void)iChange;
	(void)pUser;
	return 1;
}

static void __xuiTimelineSpanChanged(xui_widget pWidget, int iSpanId, int iChange, void* pUser)
{
	xui_timeline_demo_t* pDemo;
	(void)pWidget;
	(void)iSpanId;
	(void)iChange;
	pDemo = (xui_timeline_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iSpanChanged++;
		__xuiTimelineUpdateStatus(pDemo);
	}
}

static void __xuiTimelineLayerSelected(xui_widget pWidget, int iLayer, void* pUser)
{
	xui_timeline_demo_t* pDemo;
	(void)pWidget;
	(void)iLayer;
	pDemo = (xui_timeline_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iLayerSelected++;
		__xuiTimelineUpdateStatus(pDemo);
	}
}

static int __xuiTimelineContextOpening(xui_widget pWidget, const xui_timeline_hit_t* pHit, void* pUser)
{
	xui_timeline_demo_t* pDemo;
	(void)pWidget;
	(void)pHit;
	pDemo = (xui_timeline_demo_t*)pUser;
	if ( pDemo != NULL ) pDemo->iContextOpen++;
	return 1;
}

static void __xuiTimelineContextCommand(xui_widget pWidget, int iCommand, const xui_timeline_hit_t* pHit, void* pUser)
{
	xui_timeline_demo_t* pDemo;
	(void)pWidget;
	(void)iCommand;
	(void)pHit;
	pDemo = (xui_timeline_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iContextCommand++;
		__xuiTimelineUpdateStatus(pDemo);
	}
}

static void __xuiTimelineFrameClick(xui_widget pWidget, int iLayer, int iFrame, int iButton, int iModifiers, void* pUser)
{
	xui_timeline_demo_t* pDemo;
	(void)pWidget;
	(void)iLayer;
	(void)iFrame;
	(void)iButton;
	(void)iModifiers;
	pDemo = (xui_timeline_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iFrameClick++;
		__xuiTimelineUpdateStatus(pDemo);
	}
}

static void __xuiTimelineSelection(xui_widget pWidget, void* pUser)
{
	xui_timeline_demo_t* pDemo;
	(void)pWidget;
	pDemo = (xui_timeline_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iSelectionChanged++;
		__xuiTimelineUpdateStatus(pDemo);
	}
}

static int __xuiTimelineFrameRender(xui_widget pWidget, int iLayer, int iFrame, const xui_timeline_frame_t* pFrame, xui_draw_context pDraw, xui_rect_t tRect, int iState, void* pUser)
{
	xui_timeline_demo_t* pDemo;
	(void)pWidget;
	(void)iLayer;
	(void)iFrame;
	(void)pFrame;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	pDemo = (xui_timeline_demo_t*)pUser;
	if ( pDemo != NULL ) pDemo->iCustomPaint++;
	return 0;
}

static int __xuiTimelineAddLabel(xui_timeline_demo_t* pDemo, int iIndex, const char* sText, xui_rect_t tRect)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;
	if ( (iIndex < 0) || (iIndex >= LABEL_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(54, 68, 88, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pLabel, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[iIndex] = pLabel;
	return XUI_OK;
}

static int __xuiTimelineCreateTimeline(xui_timeline_demo_t* pDemo)
{
	xui_timeline_view_desc_t tDesc;
	xui_timeline_view_colors_t tColors;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.iFrameCount = 180;
	tDesc.fFrameRate = 24.0f;
	tDesc.fLayerHeaderWidth = 168.0f;
	tDesc.fFrameWidth = 12.0f;
	tDesc.fRowHeight = 25.0f;
	tDesc.fRulerHeight = 26.0f;
	tDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	iRet = xuiTimeLineViewCreate(pDemo->pContext, &pDemo->pTimeline, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pTimeline, (xui_rect_t){48.0f, 92.0f, 804.0f, 282.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pTimeline);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiTimeLineViewGetColors(pDemo->pTimeline, &tColors);
	tColors.iSpanColor = XUI_COLOR_RGBA(69, 145, 214, 205);
	tColors.iKeyFrameColor = XUI_COLOR_RGBA(38, 120, 216, 255);
	tColors.iCurrentColor = XUI_COLOR_RGBA(221, 67, 84, 255);
	(void)xuiTimeLineViewSetColors(pDemo->pTimeline, &tColors);
	(void)xuiTimeLineViewSetCurrentFrameCallbacks(pDemo->pTimeline, __xuiTimelineCurrentChanging, __xuiTimelineCurrentChanged, pDemo);
	(void)xuiTimeLineViewSetLayerCallbacks(pDemo->pTimeline, __xuiTimelineLayerChanging, __xuiTimelineLayerChanged, pDemo);
	(void)xuiTimeLineViewSetFrameCallbacks(pDemo->pTimeline, __xuiTimelineFrameChanging, __xuiTimelineFrameChanged, pDemo);
	(void)xuiTimeLineViewSetSpanCallbacks(pDemo->pTimeline, __xuiTimelineSpanChanging, __xuiTimelineSpanChanged, pDemo);
	(void)xuiTimeLineViewSetLayerSelected(pDemo->pTimeline, __xuiTimelineLayerSelected, pDemo);
	(void)xuiTimeLineViewSetContextMenu(pDemo->pTimeline, __xuiTimelineContextOpening, __xuiTimelineContextCommand, pDemo);
	(void)xuiTimeLineViewSetFrameClick(pDemo->pTimeline, __xuiTimelineFrameClick, NULL, pDemo);
	(void)xuiTimeLineViewSetSelectionChange(pDemo->pTimeline, __xuiTimelineSelection, pDemo);
	(void)xuiTimeLineViewSetRenderers(pDemo->pTimeline, NULL, NULL, __xuiTimelineFrameRender, NULL, pDemo);
	(void)xuiTimeLineViewAddLayer(pDemo->pTimeline, "Background plate", &pDemo->iBgLayer);
	(void)xuiTimeLineViewAddLayer(pDemo->pTimeline, "Character pose", &pDemo->iPoseLayer);
	(void)xuiTimeLineViewAddLayer(pDemo->pTimeline, "FX overlay", &pDemo->iFxLayer);
	(void)xuiTimeLineViewAddLayer(pDemo->pTimeline, "Camera", &pDemo->iCameraLayer);
	(void)xuiTimeLineViewSetLayerColor(pDemo->pTimeline, pDemo->iPoseLayer, XUI_COLOR_RGBA(60, 165, 116, 255));
	(void)xuiTimeLineViewSetLayerColor(pDemo->pTimeline, pDemo->iFxLayer, XUI_COLOR_RGBA(225, 145, 64, 255));
	(void)xuiTimeLineViewSetFrame(pDemo->pTimeline, pDemo->iBgLayer, 0, XUI_TIMELINE_FRAME_KEY, NULL);
	(void)xuiTimeLineViewSetFrame(pDemo->pTimeline, pDemo->iBgLayer, 45, XUI_TIMELINE_FRAME_KEY, NULL);
	(void)xuiTimeLineViewSetFrame(pDemo->pTimeline, pDemo->iPoseLayer, 8, XUI_TIMELINE_FRAME_KEY, NULL);
	(void)xuiTimeLineViewSetFrame(pDemo->pTimeline, pDemo->iPoseLayer, 20, XUI_TIMELINE_FRAME_NORMAL, NULL);
	(void)xuiTimeLineViewSetFrame(pDemo->pTimeline, pDemo->iPoseLayer, 36, XUI_TIMELINE_FRAME_KEY, NULL);
	(void)xuiTimeLineViewSetFrame(pDemo->pTimeline, pDemo->iFxLayer, 18, XUI_TIMELINE_FRAME_BLANK_KEY, NULL);
	(void)xuiTimeLineViewSetFrame(pDemo->pTimeline, pDemo->iCameraLayer, 0, XUI_TIMELINE_FRAME_KEY, NULL);
	(void)xuiTimeLineViewSetFrame(pDemo->pTimeline, pDemo->iCameraLayer, 72, XUI_TIMELINE_FRAME_KEY, NULL);
	(void)xuiTimeLineViewAddSpan(pDemo->pTimeline, pDemo->iBgLayer, 0, 45, XUI_TIMELINE_SPAN_HOLD, "opening background", &pDemo->iIntroSpan);
	(void)xuiTimeLineViewAddSpan(pDemo->pTimeline, pDemo->iPoseLayer, 8, 36, XUI_TIMELINE_SPAN_MOTION, "walk cycle", NULL);
	(void)xuiTimeLineViewAddSpan(pDemo->pTimeline, pDemo->iCameraLayer, 0, 72, XUI_TIMELINE_SPAN_MOTION, "camera move", NULL);
	(void)xuiTimeLineViewSelectRange(pDemo->pTimeline, pDemo->iPoseLayer, 8, pDemo->iPoseLayer, 12, 1);
	(void)xuiTimeLineViewSetCurrentFrame(pDemo->pTimeline, 12);
	return XUI_OK;
}

static int __xuiTimelineCreateUi(xui_timeline_demo_t* pDemo)
{
	int iRet;
	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiTimelineRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiTimelineAddLabel(pDemo, 0, "XUI TimeLineView", (xui_rect_t){48.0f, 32.0f, 220.0f, 24.0f}) != XUI_OK ||
	     __xuiTimelineAddLabel(pDemo, 1, "Layers, ruler, key frames, spans, selection, visibility, lock, scroll, and keyboard navigation", (xui_rect_t){48.0f, 58.0f, 760.0f, 22.0f}) != XUI_OK ||
	     __xuiTimelineAddLabel(pDemo, 2, "frame=0 selections=0 current=0/0 layer=0 frameOps=0 spanOps=0 clicks=0", (xui_rect_t){48.0f, 394.0f, 720.0f, 24.0f}) != XUI_OK ) {
		return XUI_ERROR;
	}
	pDemo->pStatus = pDemo->pLabel[2];
	iRet = __xuiTimelineCreateTimeline(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	__xuiTimelineUpdateStatus(pDemo);
	return XUI_OK;
}

static uint32_t __xuiTimelineReadButtons(void)
{
	uint32_t iButtons;
	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static uint32_t __xuiTimelineReadModifiers(void)
{
	uint32_t iModifiers;
	iModifiers = 0;
	if ( xgeKeyDown(XUI_TIMELINE_KEY_LEFT_SHIFT) || xgeKeyDown(XUI_TIMELINE_KEY_RIGHT_SHIFT) ) iModifiers |= XUI_MOD_SHIFT;
	if ( xgeKeyDown(XUI_TIMELINE_KEY_LEFT_CTRL) || xgeKeyDown(XUI_TIMELINE_KEY_RIGHT_CTRL) ) iModifiers |= XUI_MOD_CTRL;
	if ( xgeKeyDown(XUI_TIMELINE_KEY_LEFT_ALT) || xgeKeyDown(XUI_TIMELINE_KEY_RIGHT_ALT) ) iModifiers |= XUI_MOD_ALT;
	if ( xgeKeyDown(XUI_TIMELINE_KEY_LEFT_SUPER) || xgeKeyDown(XUI_TIMELINE_KEY_RIGHT_SUPER) ) iModifiers |= XUI_MOD_SUPER;
	return iModifiers;
}

static int __xuiTimelineHandleInput(xui_timeline_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iModifiers;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) xgeQuit();
	iModifiers = __xuiTimelineReadModifiers();
	iRet = xuiInputSetModifiers(pDemo->pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiTimelineReadButtons();
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
	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( xgeKeyPressed(XGE_KEY_LEFT) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_LEFT, iModifiers);
	if ( xgeKeyPressed(XGE_KEY_RIGHT) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_RIGHT, iModifiers);
	if ( xgeKeyPressed(XGE_KEY_UP) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_UP, iModifiers);
	if ( xgeKeyPressed(XGE_KEY_DOWN) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_DOWN, iModifiers);
	if ( xgeKeyPressed(XGE_KEY_HOME) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_HOME, iModifiers);
	if ( xgeKeyPressed(XGE_KEY_END) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_END, iModifiers);
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiTimelineClickFrame(xui_timeline_demo_t* pDemo, int iLayer, int iFrame)
{
	xui_rect_t tWorld;
	float fLayerHeader;
	float fFrameWidth;
	float fRowHeight;
	float fRulerHeight;
	float fX;
	float fY;
	int iRet;
	tWorld = xuiWidgetGetWorldRect(pDemo->pTimeline);
	(void)xuiTimeLineViewGetMetrics(pDemo->pTimeline, &fLayerHeader, &fFrameWidth, &fRowHeight, &fRulerHeight);
	fX = tWorld.fX + fLayerHeader + (float)iFrame * fFrameWidth + fFrameWidth * 0.5f;
	fY = tWorld.fY + fRulerHeight + (float)iLayer * fRowHeight + fRowHeight * 0.5f;
	iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	return iRet;
}

static void __xuiTimelineRunChecks(xui_timeline_demo_t* pDemo, int bExerciseInput)
{
	pDemo->bCreateOK = (pDemo->pTimeline != NULL) && (xuiTimeLineViewGetFrameWidget(pDemo->pTimeline) != NULL) &&
		(xuiTimeLineViewGetLayerCount(pDemo->pTimeline) == 4);
	pDemo->bLayoutOK = xuiScrollFrameIsHScrollBarVisible(xuiTimeLineViewGetFrameWidget(pDemo->pTimeline)) ? 1 : 0;
	if ( bExerciseInput && !pDemo->bExerciseDone ) {
		(void)__xuiTimelineClickFrame(pDemo, pDemo->iPoseLayer, 18);
		(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_RIGHT, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiTimeLineViewRunContextCommand(pDemo->pTimeline, XUI_TIMELINE_MENU_FRAME_KEY);
		(void)xuiTimeLineViewEnsureFrameVisible(pDemo->pTimeline, pDemo->iCameraLayer, 120);
		pDemo->bInteractionOK = (xuiTimeLineViewGetCurrentFrame(pDemo->pTimeline) >= 18) &&
			(pDemo->iFrameClick > 0) && (pDemo->iContextCommand > 0) && (pDemo->iSelectionChanged > 0);
		pDemo->bScrollOK = xuiTimeLineViewGetChangeCount(pDemo->pTimeline) > 0;
		pDemo->bExerciseDone = 1;
	} else if ( !bExerciseInput ) {
		pDemo->bInteractionOK = 1;
		pDemo->bScrollOK = 1;
	}
	pDemo->bPaintOK = pDemo->iCustomPaint > 0;
}

static int __xuiTimelineCreateAssets(xui_timeline_demo_t* pDemo)
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
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiTimelineFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiTimelineCreateUi(pDemo);
}

static void __xuiTimelineDestroyAssets(xui_timeline_demo_t* pDemo)
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

static int __xuiTimelineFrame(void* pUser)
{
	xui_timeline_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	int iRet;
	int bAutoRun;
	pDemo = (xui_timeline_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiTimelineHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiTimelineRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(232, 241, 250, 255));
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
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		printf("xui_timelineview final-summary frames=%d create=%d layout=%d interaction=%d scroll=%d paint=%d current=%d context=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bInteractionOK, pDemo->bScrollOK,
			pDemo->bPaintOK, xuiTimeLineViewGetCurrentFrame(pDemo->pTimeline), pDemo->iContextCommand,
			tStats.iUpdatedCaches, tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_timeline_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;
	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiTimelineParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiTimelineUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI TimeLineView";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_timelineview: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiTimelineCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_timelineview: create assets failed: %d\n", iRet);
		__xuiTimelineDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiTimelineFrame, &tDemo);
	__xuiTimelineDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bInteractionOK && tDemo.bScrollOK && tDemo.bPaintOK) ? 0 : 1;
}
