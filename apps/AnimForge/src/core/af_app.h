/*
 * af_app.h - AnimForge application framework
 *
 * Central application state: XUI context, proxy, document, undo stack,
 * editor panels, and the frame loop.
 */

#ifndef AF_APP_H
#define AF_APP_H

#include "xui.h"
#include "xge.h"
#include "af_doc.h"
#include "af_undo.h"
#include "../anim/af_playback.h"

/* Forward declaration for tool manager */
typedef struct af_tool_manager_t af_tool_manager_t;
/* Forward declaration for timeline UI */
typedef struct af_timeline_ui_t af_timeline_ui_t;

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Constants                                                          */
/* ------------------------------------------------------------------ */

#define AF_APP_WIDTH       1400
#define AF_APP_HEIGHT      900
#define AF_APP_TITLE       "AnimForge"

/* Dark theme colors */
#define AF_COLOR_BG         XUI_COLOR_RGBA(37, 37, 38, 255)    /* #252526 */
#define AF_COLOR_PANEL      XUI_COLOR_RGBA(45, 45, 48, 255)    /* #2D2D30 */
#define AF_COLOR_HIGHLIGHT  XUI_COLOR_RGBA(63, 63, 70, 255)    /* #3F3F46 */
#define AF_COLOR_ACCENT     XUI_COLOR_RGBA(0, 122, 204, 255)   /* #007ACC */
#define AF_COLOR_KEYFRAME   XUI_COLOR_RGBA(232, 168, 56, 255)  /* #E8A838 */
#define AF_COLOR_TEXT        XUI_COLOR_RGBA(220, 220, 220, 255)
#define AF_COLOR_TEXT_DIM    XUI_COLOR_RGBA(140, 140, 140, 255)
#define AF_COLOR_STAGE_BG    XUI_COLOR_RGBA(128, 128, 128, 255)
#define AF_COLOR_STAGE       XUI_COLOR_RGBA(255, 255, 255, 255)
#define AF_COLOR_TIMELINE_BG XUI_COLOR_RGBA(30, 30, 30, 255)   /* #1E1E1E */

/* ------------------------------------------------------------------ */
/* Tool types                                                         */
/* ------------------------------------------------------------------ */

#define AF_TOOL_SELECT       0
#define AF_TOOL_SUBSELECT    1
#define AF_TOOL_PEN          2
#define AF_TOOL_LINE         3
#define AF_TOOL_RECT         4
#define AF_TOOL_ELLIPSE      5
#define AF_TOOL_PENCIL       6
#define AF_TOOL_BRUSH        7
#define AF_TOOL_TEXT         8
#define AF_TOOL_ERASER       9
#define AF_TOOL_FILL         10
#define AF_TOOL_EYEDROPPER   11
#define AF_TOOL_GRADIENT     12
#define AF_TOOL_HAND         13
#define AF_TOOL_ZOOM         14
#define AF_TOOL_COUNT        15

/* ------------------------------------------------------------------ */
/* Stage view state                                                   */
/* ------------------------------------------------------------------ */

typedef struct af_stage_view_t {
	float fOffsetX;       /* pan offset */
	float fOffsetY;
	float fZoom;          /* zoom factor (1.0 = 100%) */
	int   bShowGrid;
	int   bShowRulers;
	int   bSnapGrid;
	float fGridSize;
} af_stage_view_t;

/* ------------------------------------------------------------------ */
/* Application state                                                  */
/* ------------------------------------------------------------------ */

typedef struct af_app_t {
	/* XUI infrastructure */
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font    pFont;
	xui_font    pFontMono;
	xui_widget  pRoot;

	/* Dock panels and windows */
	xui_widget  pDock;
	xui_widget  pMenuBar;
	xui_widget  pToolbar;
	xui_widget  pStatusBar;
	int         iWinStage;
	int         iWinTimeline;
	int         iWinLibrary;
	int         iWinProperty;
	int         iWinColor;
	int         iWinCurve;
	int         iPaneDoc;
	int         iPaneTimeline;

	/* Stage canvas */
	xui_widget  pStageCanvas;
	af_stage_view_t tView;
	xui_rect_t  tStageRect;   /* computed stage rect in screen coords */

	/* Document */
	af_doc_t    tDoc;
	af_undo_stack_t tUndo;

	/* Tool manager */
	af_tool_manager_t* pToolMgr;

	/* Playback engine */
	af_playback_t tPlayback;

	/* Timeline UI */
	af_timeline_ui_t* pTimelineUi;

	/* Editor state */
	int         iActiveTool;
	int         iCurrentFrame;
	uint32_t    iActiveLayerId;
	uint32_t    iSelectedInstance;
	float       fFillColor;
	float       fStrokeColor;
	uint32_t    iFillColorRGBA;
	uint32_t    iStrokeColorRGBA;
	float       fStrokeWidth;

	/* Playback */
	int         bPlaying;
	int         bLoop;
	float       fPlaySpeed;
	double      fPlayAccum;

	/* Runtime */
	int         iFrame;
	int         iMaxFrames;
	double      fMaxSeconds;
	int         bHasMouse;
	float       fLastMouseX;
	float       fLastMouseY;
	uint32_t    iLastButtons;
} af_app_t;

/* ------------------------------------------------------------------ */
/* Application API                                                    */
/* ------------------------------------------------------------------ */

int  afAppInit(af_app_t* pApp);
void afAppShutdown(af_app_t* pApp);
int  afAppCreateUi(af_app_t* pApp);
int  afAppFrame(af_app_t* pApp);
int  afAppHandleInput(af_app_t* pApp);
void afAppRender(af_app_t* pApp);

/* Document operations */
int  afAppNewDocument(af_app_t* pApp, float fW, float fH, float fFps, uint32_t iFrames);
int  afAppOpenDocument(af_app_t* pApp, const char* sPath);
int  afAppSaveDocument(af_app_t* pApp, const char* sPath);

/* Tool */
void afAppSetTool(af_app_t* pApp, int iTool);

/* Playback */
void afAppPlay(af_app_t* pApp);
void afAppStop(af_app_t* pApp);
void afAppStepFrame(af_app_t* pApp, int iDelta);

#ifdef __cplusplus
}
#endif

#endif /* AF_APP_H */
