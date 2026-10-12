/*
 * af_tool_pen.c - Pen and Pencil drawing tools
 *
 * Pen: click to add points, drag for curves, double-click/Enter to finish
 * Pencil: freehand drawing with smoothing
 */

#include "af_tools.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------ */
/* Pen tool state                                                     */
/* ------------------------------------------------------------------ */

#define AF_PEN_MAX_POINTS 1024

typedef struct af_pen_point_t {
	float fX, fY;
	float fCtrlInX, fCtrlInY;    /* control point in */
	float fCtrlOutX, fCtrlOutY;  /* control point out */
	int bHasCtrlIn;
	int bHasCtrlOut;
	int bCorner;                 /* corner vs smooth point */
} af_pen_point_t;

typedef struct af_pen_tool_state_t {
	int bActive;
	af_pen_point_t arrPoints[AF_PEN_MAX_POINTS];
	int iPointCount;
	int bDragging;               /* dragging control handle */
	float fPreviewX, fPreviewY;  /* current mouse for preview line */
} af_pen_tool_state_t;

/* ------------------------------------------------------------------ */
/* Pencil tool state                                                  */
/* ------------------------------------------------------------------ */

#define AF_PENCIL_MAX_POINTS 4096

typedef struct af_pencil_tool_state_t {
	int bDrawing;
	float arrPoints[AF_PENCIL_MAX_POINTS * 2];
	int iPointCount;
} af_pencil_tool_state_t;

/* ------------------------------------------------------------------ */
/* Pen tool handlers                                                  */
/* ------------------------------------------------------------------ */

static int __penActivate(af_tool pTool, af_tool_context_t* pCtx)
{
	af_pen_tool_state_t* pState = (af_pen_tool_state_t*)pTool->pState;
	(void)pCtx;
	if ( pState != NULL ) {
		pState->bActive = 0;
		pState->iPointCount = 0;
	}
	return 0;
}

static int __penDeactivate(af_tool pTool, af_tool_context_t* pCtx)
{
	af_pen_tool_state_t* pState = (af_pen_tool_state_t*)pTool->pState;
	(void)pCtx;
	if ( pState != NULL ) {
		pState->bActive = 0;
		pState->iPointCount = 0;
	}
	return 0;
}

static void __penCommitPath(af_tool pTool, af_tool_context_t* pCtx)
{
	af_pen_tool_state_t* pState = (af_pen_tool_state_t*)pTool->pState;
	af_path_t path;
	int i, pi;
	
	if ( pState == NULL || pState->iPointCount < 2 ) return;
	
	memset(&path, 0, sizeof(path));
	pi = 0;
	
	/* First point: move to */
	path.arrCommands[path.iCommandCount++] = AF_PATH_CMD_MOVE_TO;
	path.arrPoints[pi*2] = pState->arrPoints[0].fX;
	path.arrPoints[pi*2+1] = pState->arrPoints[0].fY;
	pi++;
	
	/* Subsequent points */
	for ( i = 1; i < pState->iPointCount; i++ ) {
		af_pen_point_t* pPrev = &pState->arrPoints[i-1];
		af_pen_point_t* pCur = &pState->arrPoints[i];
		
		if ( pPrev->bHasCtrlOut || pCur->bHasCtrlIn ) {
			/* Cubic bezier */
			float c1x = pPrev->bHasCtrlOut ? pPrev->fCtrlOutX : pPrev->fX;
			float c1y = pPrev->bHasCtrlOut ? pPrev->fCtrlOutY : pPrev->fY;
			float c2x = pCur->bHasCtrlIn ? pCur->fCtrlInX : pCur->fX;
			float c2y = pCur->bHasCtrlIn ? pCur->fCtrlInY : pCur->fY;
			
			path.arrCommands[path.iCommandCount++] = AF_PATH_CMD_CUBIC_TO;
			path.arrPoints[pi*2] = c1x; path.arrPoints[pi*2+1] = c1y; pi++;
			path.arrPoints[pi*2] = c2x; path.arrPoints[pi*2+1] = c2y; pi++;
			path.arrPoints[pi*2] = pCur->fX; path.arrPoints[pi*2+1] = pCur->fY; pi++;
		} else {
			/* Line to */
			path.arrCommands[path.iCommandCount++] = AF_PATH_CMD_LINE_TO;
			path.arrPoints[pi*2] = pCur->fX;
			path.arrPoints[pi*2+1] = pCur->fY;
			pi++;
		}
	}
	
	path.iPointCount = pi;
	
	/* Commit to document */
	afToolCommitShape(pCtx, &path, NULL, NULL, pCtx->fStrokeWidth, NULL);
	
	/* Reset */
	pState->bActive = 0;
	pState->iPointCount = 0;
	xgeRenderRequest();
}

static int __penMouseDown(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton)
{
	af_pen_tool_state_t* pState = (af_pen_tool_state_t*)pTool->pState;
	af_pen_point_t* pPt;
	
	(void)pCtx;
	if ( pState == NULL ) return -1;
	if ( iButton != XGE_MOUSE_LEFT ) return 0;
	
	/* Add new point */
	if ( pState->iPointCount >= AF_PEN_MAX_POINTS ) return 0;
	
	pPt = &pState->arrPoints[pState->iPointCount];
	memset(pPt, 0, sizeof(af_pen_point_t));
	pPt->fX = fX;
	pPt->fY = fY;
	pState->iPointCount++;
	pState->bActive = 1;
	pState->bDragging = 1;
	pState->fPreviewX = fX;
	pState->fPreviewY = fY;
	
	xgeRenderRequest();
	return 0;
}

static int __penMouseMove(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY)
{
	af_pen_tool_state_t* pState = (af_pen_tool_state_t*)pTool->pState;
	(void)pCtx;
	
	if ( pState == NULL ) return 0;
	
	pState->fPreviewX = fX;
	pState->fPreviewY = fY;
	
	/* If dragging, update control handles */
	if ( pState->bDragging && pState->iPointCount > 0 ) {
		af_pen_point_t* pPt = &pState->arrPoints[pState->iPointCount - 1];
		float dx = fX - pPt->fX;
		float dy = fY - pPt->fY;
		
		if ( fabsf(dx) > 2.0f || fabsf(dy) > 2.0f ) {
			pPt->fCtrlOutX = fX;
			pPt->fCtrlOutY = fY;
			pPt->bHasCtrlOut = 1;
			
			/* Mirror control in */
			pPt->fCtrlInX = pPt->fX - dx;
			pPt->fCtrlInY = pPt->fY - dy;
			pPt->bHasCtrlIn = 1;
		}
	}
	
	xgeRenderRequest();
	return 0;
}

static int __penMouseUp(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton)
{
	af_pen_tool_state_t* pState = (af_pen_tool_state_t*)pTool->pState;
	(void)pCtx; (void)fX; (void)fY; (void)iButton;
	
	if ( pState != NULL ) {
		pState->bDragging = 0;
	}
	return 0;
}

static int __penDblClick(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY)
{
	(void)fX; (void)fY;
	__penCommitPath(pTool, pCtx);
	return 0;
}

static int __penKeyDown(af_tool pTool, af_tool_context_t* pCtx, int iKey)
{
	af_pen_tool_state_t* pState = (af_pen_tool_state_t*)pTool->pState;
	
	if ( pState == NULL ) return 0;
	
	if ( iKey == XGE_KEY_ENTER ) {
		__penCommitPath(pTool, pCtx);
		return 1;
	}
	if ( iKey == XGE_KEY_ESCAPE ) {
		pState->bActive = 0;
		pState->iPointCount = 0;
		xgeRenderRequest();
		return 1;
	}
	if ( iKey == XGE_KEY_BACKSPACE && pState->iPointCount > 0 ) {
		pState->iPointCount--;
		xgeRenderRequest();
		return 1;
	}
	return 0;
}

static int __penRender(af_tool pTool, af_tool_context_t* pCtx)
{
	af_pen_tool_state_t* pState = (af_pen_tool_state_t*)pTool->pState;
	xge_shape_ex pShape;
	int i;
	
	if ( pState == NULL || !pState->bActive || pState->iPointCount == 0 ) return 0;
	
	if ( xgeShapeExCreate(&pShape) != XGE_OK ) return -1;
	
	/* Draw path so far */
	xgeShapeExMoveTo(pShape, pState->arrPoints[0].fX, pState->arrPoints[0].fY);
	
	for ( i = 1; i < pState->iPointCount; i++ ) {
		af_pen_point_t* pPrev = &pState->arrPoints[i-1];
		af_pen_point_t* pCur = &pState->arrPoints[i];
		
		if ( pPrev->bHasCtrlOut || pCur->bHasCtrlIn ) {
			float c1x = pPrev->bHasCtrlOut ? pPrev->fCtrlOutX : pPrev->fX;
			float c1y = pPrev->bHasCtrlOut ? pPrev->fCtrlOutY : pPrev->fY;
			float c2x = pCur->bHasCtrlIn ? pCur->fCtrlInX : pCur->fX;
			float c2y = pCur->bHasCtrlIn ? pCur->fCtrlInY : pCur->fY;
			xgeShapeExCubicTo(pShape, c1x, c1y, c2x, c2y, pCur->fX, pCur->fY);
		} else {
			xgeShapeExLineTo(pShape, pCur->fX, pCur->fY);
		}
	}
	
	/* Preview line to mouse */
	if ( !pState->bDragging ) {
		xgeShapeExLineTo(pShape, pState->fPreviewX, pState->fPreviewY);
	}
	
	xgeShapeExStrokeColor(pShape, pCtx->iStrokeColor);
	xgeShapeExStrokeWidth(pShape, pCtx->fStrokeWidth);
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
	
	/* Draw anchor points */
	for ( i = 0; i < pState->iPointCount; i++ ) {
		af_pen_point_t* pPt = &pState->arrPoints[i];
		xge_shape_ex pDot;
		
		if ( xgeShapeExCreate(&pDot) != XGE_OK ) continue;
		
		/* Anchor square */
		xgeShapeExAppendRect(pDot, pPt->fX - 3.0f, pPt->fY - 3.0f, 6.0f, 6.0f, 0, 0, 1);
		xgeShapeExFillColor(pDot, XUI_COLOR_RGBA(255, 255, 255, 255));
		xgeShapeExStrokeColor(pDot, XUI_COLOR_RGBA(0, 122, 204, 255));
		xgeShapeExStrokeWidth(pDot, 1.0f);
		xgeShapeExDraw(pDot, 0.25f);
		xgeShapeExDestroy(pDot);
		
		/* Control handles */
		if ( pPt->bHasCtrlOut ) {
			xge_shape_ex pHandle;
			if ( xgeShapeExCreate(&pHandle) == XGE_OK ) {
				xgeShapeExMoveTo(pHandle, pPt->fX, pPt->fY);
				xgeShapeExLineTo(pHandle, pPt->fCtrlOutX, pPt->fCtrlOutY);
				xgeShapeExStrokeColor(pHandle, XUI_COLOR_RGBA(0, 122, 204, 180));
				xgeShapeExStrokeWidth(pHandle, 1.0f);
				xgeShapeExDraw(pHandle, 0.25f);
				xgeShapeExDestroy(pHandle);
				
				if ( xgeShapeExCreate(&pHandle) == XGE_OK ) {
					xgeShapeExAppendCircle(pHandle, pPt->fCtrlOutX, pPt->fCtrlOutY, 3.0f, 3.0f, 1);
					xgeShapeExFillColor(pHandle, XUI_COLOR_RGBA(0, 122, 204, 255));
					xgeShapeExDraw(pHandle, 0.25f);
					xgeShapeExDestroy(pHandle);
				}
			}
		}
	}
	
	return 0;
}

static int __penCancel(af_tool pTool, af_tool_context_t* pCtx)
{
	af_pen_tool_state_t* pState = (af_pen_tool_state_t*)pTool->pState;
	(void)pCtx;
	if ( pState != NULL ) {
		pState->bActive = 0;
		pState->iPointCount = 0;
		pState->bDragging = 0;
	}
	return 0;
}

af_tool afToolPenCreate(void)
{
	af_tool pTool;
	af_pen_tool_state_t* pState;
	
	pTool = (af_tool)calloc(1, sizeof(af_tool_t));
	if ( pTool == NULL ) return NULL;
	
	pState = (af_pen_tool_state_t*)calloc(1, sizeof(af_pen_tool_state_t));
	if ( pState == NULL ) {
		free(pTool);
		return NULL;
	}
	
	pTool->iId = AF_TOOL_PEN;
	pTool->sName = "Pen";
	pTool->sShortcut = "P";
	pTool->iCursor = 0;
	
	pTool->pfnActivate = __penActivate;
	pTool->pfnDeactivate = __penDeactivate;
	pTool->pfnMouseDown = __penMouseDown;
	pTool->pfnMouseMove = __penMouseMove;
	pTool->pfnMouseUp = __penMouseUp;
	pTool->pfnMouseDblClick = __penDblClick;
	pTool->pfnKeyDown = __penKeyDown;
	pTool->pfnRender = __penRender;
	pTool->pfnCancel = __penCancel;
	
	pTool->pState = pState;
	return pTool;
}

/* ------------------------------------------------------------------ */
/* Pencil tool handlers                                               */
/* ------------------------------------------------------------------ */

static int __pencilMouseDown(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton)
{
	af_pencil_tool_state_t* pState = (af_pencil_tool_state_t*)pTool->pState;
	(void)pCtx;
	
	if ( pState == NULL ) return -1;
	if ( iButton != XGE_MOUSE_LEFT ) return 0;
	
	pState->bDrawing = 1;
	pState->iPointCount = 1;
	pState->arrPoints[0] = fX;
	pState->arrPoints[1] = fY;
	
	return 0;
}

static int __pencilMouseMove(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY)
{
	af_pencil_tool_state_t* pState = (af_pencil_tool_state_t*)pTool->pState;
	(void)pCtx;
	
	if ( pState == NULL || !pState->bDrawing ) return 0;
	
	/* Add point if moved enough */
	if ( pState->iPointCount > 0 ) {
		float lastX = pState->arrPoints[(pState->iPointCount-1)*2];
		float lastY = pState->arrPoints[(pState->iPointCount-1)*2+1];
		float dist = sqrtf((fX-lastX)*(fX-lastX) + (fY-lastY)*(fY-lastY));
		
		if ( dist >= 2.0f && pState->iPointCount < AF_PENCIL_MAX_POINTS ) {
			pState->arrPoints[pState->iPointCount*2] = fX;
			pState->arrPoints[pState->iPointCount*2+1] = fY;
			pState->iPointCount++;
			xgeRenderRequest();
		}
	}
	return 0;
}

static int __pencilMouseUp(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton)
{
	af_pencil_tool_state_t* pState = (af_pencil_tool_state_t*)pTool->pState;
	af_path_t path;
	int i;
	
	(void)iButton;
	
	if ( pState == NULL || !pState->bDrawing ) return 0;
	pState->bDrawing = 0;
	
	if ( pState->iPointCount < 2 ) return 0;
	
	/* Build path from points */
	memset(&path, 0, sizeof(path));
	
	path.arrCommands[0] = AF_PATH_CMD_MOVE_TO;
	path.arrPoints[0] = pState->arrPoints[0];
	path.arrPoints[1] = pState->arrPoints[1];
	path.iCommandCount = 1;
	path.iPointCount = 1;
	
	for ( i = 1; i < pState->iPointCount && path.iCommandCount < AF_DOC_MAX_PATH_CMDS; i++ ) {
		path.arrCommands[path.iCommandCount++] = AF_PATH_CMD_LINE_TO;
		path.arrPoints[path.iPointCount*2] = pState->arrPoints[i*2];
		path.arrPoints[path.iPointCount*2+1] = pState->arrPoints[i*2+1];
		path.iPointCount++;
	}
	
	/* Commit - pencil draws stroke only */
	{
		af_paint_t fill, stroke;
		memset(&fill, 0, sizeof(fill));
		fill.iType = AF_PAINT_NONE;
		memset(&stroke, 0, sizeof(stroke));
		stroke.iType = AF_PAINT_SOLID;
		stroke.iColor = pCtx->iStrokeColor;
		afToolCommitShape(pCtx, &path, &fill, &stroke, pCtx->fStrokeWidth, NULL);
	}
	
	pState->iPointCount = 0;
	xgeRenderRequest();
	return 0;
}

static int __pencilRender(af_tool pTool, af_tool_context_t* pCtx)
{
	af_pencil_tool_state_t* pState = (af_pencil_tool_state_t*)pTool->pState;
	xge_shape_ex pShape;
	int i;
	
	if ( pState == NULL || !pState->bDrawing || pState->iPointCount < 2 ) return 0;
	
	if ( xgeShapeExCreate(&pShape) != XGE_OK ) return -1;
	
	xgeShapeExMoveTo(pShape, pState->arrPoints[0], pState->arrPoints[1]);
	for ( i = 1; i < pState->iPointCount; i++ ) {
		xgeShapeExLineTo(pShape, pState->arrPoints[i*2], pState->arrPoints[i*2+1]);
	}
	
	xgeShapeExStrokeColor(pShape, pCtx->iStrokeColor);
	xgeShapeExStrokeWidth(pShape, pCtx->fStrokeWidth);
	xgeShapeExStrokeCap(pShape, 1);  /* round cap */
	xgeShapeExStrokeJoin(pShape, 1); /* round join */
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
	
	return 0;
}

af_tool afToolPencilCreate(void)
{
	af_tool pTool;
	af_pencil_tool_state_t* pState;
	
	pTool = (af_tool)calloc(1, sizeof(af_tool_t));
	if ( pTool == NULL ) return NULL;
	
	pState = (af_pencil_tool_state_t*)calloc(1, sizeof(af_pencil_tool_state_t));
	if ( pState == NULL ) {
		free(pTool);
		return NULL;
	}
	
	pTool->iId = AF_TOOL_PENCIL;
	pTool->sName = "Pencil";
	pTool->sShortcut = "Y";
	pTool->iCursor = 0;
	
	pTool->pfnMouseDown = __pencilMouseDown;
	pTool->pfnMouseMove = __pencilMouseMove;
	pTool->pfnMouseUp = __pencilMouseUp;
	pTool->pfnRender = __pencilRender;
	
	pTool->pState = pState;
	return pTool;
}

/* Stub creators for other tools */
af_tool afToolSubSelectCreate(void) { return NULL; }
af_tool afToolEraserCreate(void) { return NULL; }
af_tool afToolEyedropperCreate(void) { return NULL; }
af_tool afToolHandCreate(void) { return NULL; }
af_tool afToolZoomCreate(void) { return NULL; }
