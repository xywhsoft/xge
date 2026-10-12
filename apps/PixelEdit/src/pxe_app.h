#ifndef PXE_APP_H
#define PXE_APP_H

#include "xge.h"
#include "xui.h"

/* ─── Constants ─── */
#define PXE_W 1400
#define PXE_H 900
#define PXE_MENU_H 26.0f
#define PXE_TOOLBAR_H 32.0f
#define PXE_STATUS_H 28.0f
#define PXE_CANVAS_MAX 64
#define PXE_LAYER_MAX 8
#define PXE_FRAME_MAX 32
#define PXE_PALETTE_MAX 64
#define PXE_HISTORY_MAX 64

/* ─── Tools ─── */
enum {
	PXE_TOOL_PENCIL = 0,
	PXE_TOOL_ERASER,
	PXE_TOOL_FILL,
	PXE_TOOL_PICKER,
	PXE_TOOL_LINE,
	PXE_TOOL_RECT,
	PXE_TOOL_COUNT
};

/* ─── Commands ─── */
enum {
	PXE_CMD_NEW = 1,
	PXE_CMD_EXPORT_PNG,
	PXE_CMD_EXPORT_STRIP,
	PXE_CMD_EXIT,
	PXE_CMD_UNDO,
	PXE_CMD_REDO,
	PXE_CMD_CLEAR,
	PXE_CMD_ABOUT
};

/* ─── Layer ─── */
typedef struct {
	uint32_t arrPixels[PXE_CANVAS_MAX * PXE_CANVAS_MAX];
	char sName[32];
	int bVisible;
	uint8_t iAlpha;
} pxe_layer_t;

/* ─── Application State ─── */
typedef struct {
	/* XUI2 core */
	xui_context pContext;
	xui_proxy_t tProxy;
	xui_surface pTarget;
	int iTargetW, iTargetH;
	xui_font pFont;
	xui_widget pRoot;

	/* Widgets */
	xui_widget pMenuBar;
	xui_widget pFileMenu;
	xui_widget pEditMenu;
	xui_widget pHelpMenu;
	xui_widget pToolbar;
	xui_widget pStatus;
	xui_widget pDock;
	xui_widget pCanvasWidget;
	xui_widget pPaletteWidget;

	/* Pixel data */
	pxe_layer_t arrLayers[PXE_LAYER_MAX];
	int iLayerCount;
	int iActiveLayer;
	int iCanvasW, iCanvasH;

	/* Frames */
	int iFrameCount;
	int iActiveFrame;

	/* Tools */
	int iTool;
	uint32_t iColor;
	int iZoom;

	/* Palette */
	uint32_t arrPalette[PXE_PALETTE_MAX];
	int iPaletteCount;

	/* Interaction */
	int bDrawing;
	int iLastX, iLastY;
	int bMouse;
	float fMouseX, fMouseY;
	uint32_t iButtons;

	/* State */
	int bCreateOK;
	int bRenderOK;
	int iFrame;
	int iFrameLimit;
} pxe_app_t;

/* ─── Functions ─── */
int PixelEditCreateUI(pxe_app_t* pApp);
void PixelEditDestroy(pxe_app_t* pApp);
int PixelEditFrame(void* pUser);
int PixelEditHandleInput(pxe_app_t* pApp);
int PixelEditSyncViewport(pxe_app_t* pApp);

/* Canvas render callback */
int PixelEditCanvasRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser);
int PixelEditCanvasEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser);

/* Palette render callback */
int PixelEditPaletteRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser);
int PixelEditPaletteEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser);

/* Drawing tools */
void PixelEditSetPixel(pxe_app_t* pApp, int iX, int iY, uint32_t iColor);
void PixelEditDrawLine(pxe_app_t* pApp, int iX0, int iY0, int iX1, int iY1, uint32_t iColor);
void PixelEditFloodFill(pxe_app_t* pApp, int iX, int iY, uint32_t iColor);

/* Export */
int PixelEditExportPNG(pxe_app_t* pApp, const char* sPath);

#endif /* PXE_APP_H */
