#ifndef ANT_APP_H
#define ANT_APP_H

#include "xge.h"
#include "xui.h"

#define ANT_W 1400
#define ANT_H 900
#define ANT_MENU_H 26.0f
#define ANT_TOOLBAR_H 32.0f
#define ANT_STATUS_H 28.0f
#define ANT_MAX_FRAMES 64
#define ANT_MAX_LAYERS 8
#define ANT_TIMELINE_H 180.0f

enum { ANT_CMD_PLAY=1, ANT_CMD_STOP, ANT_CMD_STEP_FWD, ANT_CMD_STEP_BACK, ANT_CMD_ADD_FRAME, ANT_CMD_DEL_FRAME, ANT_CMD_EXPORT, ANT_CMD_EXIT, ANT_CMD_ABOUT };

/* Single animation frame on a layer */
typedef struct {
	int iDuration; /* in ticks (1/60s) */
	uint32_t iColor; /* placeholder visual */
	int bKeyframe;
} ant_frame_t;

/* Animation layer (track) */
typedef struct {
	char sName[64];
	ant_frame_t arrFrames[ANT_MAX_FRAMES];
	int iFrameCount;
	int bVisible;
} ant_layer_t;

/* App state */
typedef struct {
	xui_context pContext; xui_proxy_t tProxy; xui_surface pTarget;
	int iTargetW, iTargetH; xui_font pFont; xui_widget pRoot;
	xui_widget pMenuBar, pFileMenu, pHelpMenu, pToolbar, pStatus, pDock;
	xui_widget pCanvasWidget, pTimelineWidget;

	ant_layer_t arrLayers[ANT_MAX_LAYERS]; int iLayerCount;
	int iCurLayer, iCurFrame;
	int bPlaying; float fPlayTime; int iFPS;
	int iTotalTicks; /* total duration in ticks */

	int bMouse; float fMouseX, fMouseY; uint32_t iButtons;
	int bCreateOK, bRenderOK, iFrame, iFrameLimit;
} ant_app_t;

int AnimTimelineCreateUI(ant_app_t* a);
void AnimTimelineDestroy(ant_app_t* a);
int AnimTimelineFrame(void* pUser);
int AnimTimelineHandleInput(ant_app_t* a);
int AnimTimelineSyncViewport(ant_app_t* a);
int AnimTimelineCanvasRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser);
int AnimTimelineTimelineRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser);
void AnimTimelinePlayUpdate(ant_app_t* a, float dt);
void AnimTimelineRecalcDuration(ant_app_t* a);

#endif
