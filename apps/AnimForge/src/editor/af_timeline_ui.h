/*
 * af_timeline_ui.h - AnimForge timeline panel
 *
 * Custom-drawn timeline widget: layer list, frame ruler,
 * keyframe markers, tween spans, playhead.
 */

#ifndef AF_TIMELINE_UI_H
#define AF_TIMELINE_UI_H

#include "xui.h"
#include "../core/af_app.h"
#include "../anim/af_playback.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Layout constants */
#define AF_TL_LAYER_WIDTH     160   /* layer list column width */
#define AF_TL_FRAME_WIDTH     12    /* pixels per frame cell */
#define AF_TL_ROW_HEIGHT      22    /* pixels per layer row */
#define AF_TL_RULER_HEIGHT    24    /* frame ruler height */
#define AF_TL_HEADER_HEIGHT   24    /* layer header row */
#define AF_TL_ICON_WIDTH      18    /* visibility/lock icon column width */

/* Context menu command IDs */
#define AF_TL_CTX_INSERT_KF       1001
#define AF_TL_CTX_INSERT_BLANK_KF 1002
#define AF_TL_CTX_CLEAR_KF        1003
#define AF_TL_CTX_CREATE_TWEEN    1004
#define AF_TL_CTX_NEW_LAYER       1010
#define AF_TL_CTX_DELETE_LAYER    1011
#define AF_TL_CTX_RENAME_LAYER    1012
#define AF_TL_CTX_DUP_LAYER       1013

/* Timeline UI state */
typedef struct af_timeline_ui_t {
	xui_widget pCanvas;       /* the canvas widget */
	af_app_t*  pApp;          /* owning app */

	int   iScrollX;           /* horizontal scroll (frames) */
	int   iScrollY;           /* vertical scroll (layers) */
	int   iPlayheadFrame;     /* current playhead position */
	int   bDraggingPlayhead;
	int   iSelectFrameA;      /* frame selection range */
	int   iSelectFrameB;
	int   iSelectLayer;       /* selected layer index (-1 none) */

	/* Context menus */
	xui_widget pCtxFrame;     /* right-click menu on frame area */
	xui_widget pCtxLayer;     /* right-click menu on layer area */
	int   iCtxLayer;          /* layer index at context menu time */
	int   iCtxFrame;          /* frame at context menu time */
} af_timeline_ui_t;

/* Create timeline panel and attach to dock */
int afTimelineUiCreate(af_timeline_ui_t* pTl, af_app_t* pApp, xui_widget pParent);

/* Set playhead position (from playback system) */
void afTimelineUiSetFrame(af_timeline_ui_t* pTl, int iFrame);

/* Get selected layer ID (0 if none) */
uint32_t afTimelineUiGetSelectedLayer(af_timeline_ui_t* pTl);

#ifdef __cplusplus
}
#endif

#endif /* AF_TIMELINE_UI_H */
