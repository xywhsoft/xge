/*
 * af_tools.h - AnimForge drawing tool framework
 *
 * Tool state machine interface for vector drawing tools.
 * Each tool implements mouse/keyboard handlers and renders
 * preview overlays on the stage.
 */

#ifndef AF_TOOLS_H
#define AF_TOOLS_H

#include "xge.h"
#include "../core/af_doc.h"
#include "../core/af_app.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Key constants not defined in xge.h (GLFW key codes)                */
/* ------------------------------------------------------------------ */

#define AF_KEY_LEFT_SHIFT    340
#define AF_KEY_LEFT_CONTROL  341
#define AF_KEY_LEFT_ALT      342
#define AF_KEY_LEFT_SUPER    343
#define AF_KEY_RIGHT_SHIFT   344
#define AF_KEY_RIGHT_CONTROL 345
#define AF_KEY_RIGHT_ALT     346
#define AF_KEY_RIGHT_SUPER   347

/* ------------------------------------------------------------------ */
/* Forward declarations                                               */
/* ------------------------------------------------------------------ */

typedef struct af_tool_t af_tool_t;
typedef af_tool_t* af_tool;
struct af_app_t;

/* ------------------------------------------------------------------ */
/* Tool context (passed to tool handlers)                             */
/* ------------------------------------------------------------------ */

typedef struct af_tool_context_t {
	struct af_app_t* pApp;       /* application state */
	af_doc           pDoc;       /* current document */
	
	/* Stage view transform */
	float fZoom;
	float fOffsetX;
	float fOffsetY;
	
	/* Current mouse state (stage coordinates) */
	float fMouseX;
	float fMouseY;
	float fStartX;               /* drag start */
	float fStartY;
	uint32_t iButtons;
	
	/* Current paint settings */
	uint32_t iFillColor;
	uint32_t iStrokeColor;
	float    fStrokeWidth;
	int      iFillEnabled;
	int      iStrokeEnabled;
	
	/* Modifier keys */
	int bShift;
	int bCtrl;
	int bAlt;
	
	/* ShapeEx for preview rendering */
	xge_shape_ex pPreviewShape;
	xge_shape_ex_scene pPreviewScene;
} af_tool_context_t;

/* ------------------------------------------------------------------ */
/* Tool event handlers                                                */
/* ------------------------------------------------------------------ */

typedef int (*af_tool_activate_proc)(af_tool pTool, af_tool_context_t* pCtx);
typedef int (*af_tool_deactivate_proc)(af_tool pTool, af_tool_context_t* pCtx);
typedef int (*af_tool_mouse_down_proc)(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton);
typedef int (*af_tool_mouse_move_proc)(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY);
typedef int (*af_tool_mouse_up_proc)(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton);
typedef int (*af_tool_mouse_dblclick_proc)(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY);
typedef int (*af_tool_key_down_proc)(af_tool pTool, af_tool_context_t* pCtx, int iKey);
typedef int (*af_tool_render_proc)(af_tool pTool, af_tool_context_t* pCtx);
typedef int (*af_tool_cancel_proc)(af_tool pTool, af_tool_context_t* pCtx);

/* ------------------------------------------------------------------ */
/* Tool structure                                                     */
/* ------------------------------------------------------------------ */

struct af_tool_t {
	int iId;                     /* AF_TOOL_* */
	const char* sName;           /* display name */
	const char* sShortcut;       /* keyboard shortcut */
	uint32_t iCursor;            /* cursor style */
	
	/* Event handlers */
	af_tool_activate_proc      pfnActivate;
	af_tool_deactivate_proc    pfnDeactivate;
	af_tool_mouse_down_proc    pfnMouseDown;
	af_tool_mouse_move_proc    pfnMouseMove;
	af_tool_mouse_up_proc      pfnMouseUp;
	af_tool_mouse_dblclick_proc pfnMouseDblClick;
	af_tool_key_down_proc      pfnKeyDown;
	af_tool_render_proc        pfnRender;
	af_tool_cancel_proc        pfnCancel;
	
	/* Tool-specific state */
	void* pState;
};

/* ------------------------------------------------------------------ */
/* Tool manager                                                       */
/* ------------------------------------------------------------------ */

#define AF_TOOL_MAX 32

typedef struct af_tool_manager_t {
	af_tool arrTools[AF_TOOL_MAX];
	int iToolCount;
	af_tool pActiveTool;
	af_tool_context_t tContext;
	int bDragging;
} af_tool_manager_t;

/* ------------------------------------------------------------------ */
/* Tool manager API                                                   */
/* ------------------------------------------------------------------ */

int  afToolManagerInit(af_tool_manager_t* pMgr, struct af_app_t* pApp);
void afToolManagerShutdown(af_tool_manager_t* pMgr);

int  afToolManagerRegister(af_tool_manager_t* pMgr, af_tool pTool);
int  afToolManagerSetActive(af_tool_manager_t* pMgr, int iToolId);
af_tool afToolManagerGetActive(af_tool_manager_t* pMgr);
const char* afToolManagerGetActiveName(af_tool_manager_t* pMgr);

/* Input dispatch */
int  afToolManagerMouseDown(af_tool_manager_t* pMgr, float fX, float fY, uint32_t iButton);
int  afToolManagerMouseMove(af_tool_manager_t* pMgr, float fX, float fY);
int  afToolManagerMouseUp(af_tool_manager_t* pMgr, float fX, float fY, uint32_t iButton);
int  afToolManagerMouseDblClick(af_tool_manager_t* pMgr, float fX, float fY);
int  afToolManagerKeyDown(af_tool_manager_t* pMgr, int iKey);
int  afToolManagerCancel(af_tool_manager_t* pMgr);

/* Render tool overlay */
int  afToolManagerRender(af_tool_manager_t* pMgr);

/* Coordinate conversion */
void afToolScreenToStage(af_tool_manager_t* pMgr, float fScreenX, float fScreenY, float* pStageX, float* pStageY);
void afToolStageToScreen(af_tool_manager_t* pMgr, float fStageX, float fStageY, float* pScreenX, float* pScreenY);

/* ------------------------------------------------------------------ */
/* Built-in tool creation                                             */
/* ------------------------------------------------------------------ */

af_tool afToolSelectCreate(void);
af_tool afToolSubSelectCreate(void);
af_tool afToolPenCreate(void);
af_tool afToolLineCreate(void);
af_tool afToolRectCreate(void);
af_tool afToolEllipseCreate(void);
af_tool afToolPencilCreate(void);
af_tool afToolBrushCreate(void);
af_tool afToolEraserCreate(void);
af_tool afToolFillCreate(void);
af_tool afToolEyedropperCreate(void);
af_tool afToolHandCreate(void);
af_tool afToolZoomCreate(void);
af_tool afToolTextCreate(void);
af_tool afToolGradientCreate(void);

void afToolDestroy(af_tool pTool);

/* ------------------------------------------------------------------ */
/* Helper: create shape from path and add to document                 */
/* ------------------------------------------------------------------ */

int afToolCommitShape(af_tool_context_t* pCtx, af_path_t* pPath, 
                      af_paint_t* pFill, af_paint_t* pStroke, 
                      float fStrokeWidth, uint32_t* pSymbolId);

#ifdef __cplusplus
}
#endif

#endif /* AF_TOOLS_H */
