/*
 * af_tool_shape.c - Rectangle, Ellipse, and Line drawing tools
 */

#include "af_tools.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------ */
/* Shape tool state                                                   */
/* ------------------------------------------------------------------ */

typedef struct af_shape_tool_state_t {
	int bDrawing;
	float fStartX, fStartY;
	float fCurX, fCurY;
	int iShapeType;  /* 0=rect, 1=ellipse, 2=line */
} af_shape_tool_state_t;

/* ------------------------------------------------------------------ */
/* Common handlers                                                    */
/* ------------------------------------------------------------------ */

static int __shapeActivate(af_tool pTool, af_tool_context_t* pCtx)
{
	(void)pTool; (void)pCtx;
	return 0;
}

static int __shapeDeactivate(af_tool pTool, af_tool_context_t* pCtx)
{
	af_shape_tool_state_t* pState = (af_shape_tool_state_t*)pTool->pState;
	(void)pCtx;
	if ( pState != NULL ) {
		pState->bDrawing = 0;
	}
	return 0;
}

static int __shapeMouseDown(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton)
{
	af_shape_tool_state_t* pState = (af_shape_tool_state_t*)pTool->pState;
	(void)pCtx;
	
	if ( iButton != XGE_MOUSE_LEFT ) return 0;
	if ( pState == NULL ) return -1;
	
	pState->bDrawing = 1;
	pState->fStartX = fX;
	pState->fStartY = fY;
	pState->fCurX = fX;
	pState->fCurY = fY;
	
	return 0;
}

static int __shapeMouseMove(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY)
{
	af_shape_tool_state_t* pState = (af_shape_tool_state_t*)pTool->pState;
	(void)pCtx;
	
	if ( pState == NULL || !pState->bDrawing ) return 0;
	
	pState->fCurX = fX;
	pState->fCurY = fY;
	
	/* Request redraw for preview */
	xgeRenderRequest();
	return 0;
}

static int __shapeMouseUp(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton)
{
	af_shape_tool_state_t* pState = (af_shape_tool_state_t*)pTool->pState;
	af_path_t path;
	float x, y, w, h;
	
	(void)iButton;
	
	if ( pState == NULL || !pState->bDrawing ) return 0;
	pState->bDrawing = 0;
	pState->fCurX = fX;
	pState->fCurY = fY;
	
	/* Calculate shape bounds */
	x = (pState->fStartX < fX) ? pState->fStartX : fX;
	y = (pState->fStartY < fY) ? pState->fStartY : fY;
	w = fabsf(fX - pState->fStartX);
	h = fabsf(fY - pState->fStartY);
	
	/* Ignore tiny shapes */
	if ( w < 2.0f && h < 2.0f ) return 0;
	
	/* Build path based on shape type */
	memset(&path, 0, sizeof(path));
	
	if ( pState->iShapeType == 0 ) {
		/* Rectangle */
		path.arrCommands[0] = AF_PATH_CMD_MOVE_TO;
		path.arrPoints[0] = x; path.arrPoints[1] = y;
		path.arrCommands[1] = AF_PATH_CMD_LINE_TO;
		path.arrPoints[2] = x + w; path.arrPoints[3] = y;
		path.arrCommands[2] = AF_PATH_CMD_LINE_TO;
		path.arrPoints[4] = x + w; path.arrPoints[5] = y + h;
		path.arrCommands[3] = AF_PATH_CMD_LINE_TO;
		path.arrPoints[6] = x; path.arrPoints[7] = y + h;
		path.arrCommands[4] = AF_PATH_CMD_CLOSE;
		path.iCommandCount = 5;
		path.iPointCount = 4;
	} else if ( pState->iShapeType == 1 ) {
		/* Ellipse - approximate with 4 cubic beziers */
		float cx = x + w * 0.5f;
		float cy = y + h * 0.5f;
		float rx = w * 0.5f;
		float ry = h * 0.5f;
		float kx = rx * 0.5522847498f;  /* magic number for circle approximation */
		float ky = ry * 0.5522847498f;
		int pi = 0;
		
		path.arrCommands[0] = AF_PATH_CMD_MOVE_TO;
		path.arrPoints[pi*2] = cx + rx; path.arrPoints[pi*2+1] = cy; pi++;
		
		path.arrCommands[1] = AF_PATH_CMD_CUBIC_TO;
		path.arrPoints[pi*2] = cx + rx; path.arrPoints[pi*2+1] = cy + ky; pi++;
		path.arrPoints[pi*2] = cx + kx; path.arrPoints[pi*2+1] = cy + ry; pi++;
		path.arrPoints[pi*2] = cx; path.arrPoints[pi*2+1] = cy + ry; pi++;
		
		path.arrCommands[2] = AF_PATH_CMD_CUBIC_TO;
		path.arrPoints[pi*2] = cx - kx; path.arrPoints[pi*2+1] = cy + ry; pi++;
		path.arrPoints[pi*2] = cx - rx; path.arrPoints[pi*2+1] = cy + ky; pi++;
		path.arrPoints[pi*2] = cx - rx; path.arrPoints[pi*2+1] = cy; pi++;
		
		path.arrCommands[3] = AF_PATH_CMD_CUBIC_TO;
		path.arrPoints[pi*2] = cx - rx; path.arrPoints[pi*2+1] = cy - ky; pi++;
		path.arrPoints[pi*2] = cx - kx; path.arrPoints[pi*2+1] = cy - ry; pi++;
		path.arrPoints[pi*2] = cx; path.arrPoints[pi*2+1] = cy - ry; pi++;
		
		path.arrCommands[4] = AF_PATH_CMD_CUBIC_TO;
		path.arrPoints[pi*2] = cx + kx; path.arrPoints[pi*2+1] = cy - ry; pi++;
		path.arrPoints[pi*2] = cx + rx; path.arrPoints[pi*2+1] = cy - ky; pi++;
		path.arrPoints[pi*2] = cx + rx; path.arrPoints[pi*2+1] = cy; pi++;
		
		path.arrCommands[5] = AF_PATH_CMD_CLOSE;
		path.iCommandCount = 6;
		path.iPointCount = pi;
	} else {
		/* Line */
		path.arrCommands[0] = AF_PATH_CMD_MOVE_TO;
		path.arrPoints[0] = pState->fStartX; path.arrPoints[1] = pState->fStartY;
		path.arrCommands[1] = AF_PATH_CMD_LINE_TO;
		path.arrPoints[2] = fX; path.arrPoints[3] = fY;
		path.iCommandCount = 2;
		path.iPointCount = 2;
	}
	
	/* Commit to document */
	afToolCommitShape(pCtx, &path, NULL, NULL, pCtx->fStrokeWidth, NULL);
	xgeRenderRequest();
	
	return 0;
}

static int __shapeRender(af_tool pTool, af_tool_context_t* pCtx)
{
	af_shape_tool_state_t* pState = (af_shape_tool_state_t*)pTool->pState;
	xge_shape_ex pShape;
	float x, y, w, h;
	
	if ( pState == NULL || !pState->bDrawing ) return 0;
	
	/* Calculate preview bounds */
	x = (pState->fStartX < pState->fCurX) ? pState->fStartX : pState->fCurX;
	y = (pState->fStartY < pState->fCurY) ? pState->fStartY : pState->fCurY;
	w = fabsf(pState->fCurX - pState->fStartX);
	h = fabsf(pState->fCurY - pState->fStartY);
	
	if ( w < 1.0f && h < 1.0f ) return 0;
	
	/* Create preview shape */
	if ( xgeShapeExCreate(&pShape) != XGE_OK ) return -1;
	
	if ( pState->iShapeType == 0 ) {
		/* Rectangle preview */
		xgeShapeExAppendRect(pShape, x, y, w, h, 0.0f, 0.0f, 1);
	} else if ( pState->iShapeType == 1 ) {
		/* Ellipse preview */
		xgeShapeExAppendEllipse(pShape, x + w * 0.5f, y + h * 0.5f, w * 0.5f, h * 0.5f, 1);
	} else {
		/* Line preview */
		xgeShapeExMoveTo(pShape, pState->fStartX, pState->fStartY);
		xgeShapeExLineTo(pShape, pState->fCurX, pState->fCurY);
	}
	
	/* Style: semi-transparent fill + solid stroke */
	if ( pState->iShapeType != 2 ) {
		xgeShapeExFillColor(pShape, (pCtx->iFillColor & 0xFFFFFF00) | 0x80);
	}
	xgeShapeExStrokeColor(pShape, pCtx->iStrokeColor);
	xgeShapeExStrokeWidth(pShape, pCtx->fStrokeWidth);
	
	/* Draw */
	xgeShapeExDraw(pShape, 0.5f);
	xgeShapeExDestroy(pShape);
	
	return 0;
}

static int __shapeCancel(af_tool pTool, af_tool_context_t* pCtx)
{
	af_shape_tool_state_t* pState = (af_shape_tool_state_t*)pTool->pState;
	(void)pCtx;
	if ( pState != NULL ) {
		pState->bDrawing = 0;
	}
	return 0;
}

/* ------------------------------------------------------------------ */
/* Tool creation                                                      */
/* ------------------------------------------------------------------ */

static af_tool __createShapeTool(int iId, const char* sName, const char* sShortcut, int iShapeType)
{
	af_tool pTool;
	af_shape_tool_state_t* pState;
	
	pTool = (af_tool)calloc(1, sizeof(af_tool_t));
	if ( pTool == NULL ) return NULL;
	
	pState = (af_shape_tool_state_t*)calloc(1, sizeof(af_shape_tool_state_t));
	if ( pState == NULL ) {
		free(pTool);
		return NULL;
	}
	
	pTool->iId = iId;
	pTool->sName = sName;
	pTool->sShortcut = sShortcut;
	pTool->iCursor = 0;  /* crosshair */
	
	pTool->pfnActivate = __shapeActivate;
	pTool->pfnDeactivate = __shapeDeactivate;
	pTool->pfnMouseDown = __shapeMouseDown;
	pTool->pfnMouseMove = __shapeMouseMove;
	pTool->pfnMouseUp = __shapeMouseUp;
	pTool->pfnRender = __shapeRender;
	pTool->pfnCancel = __shapeCancel;
	
	pTool->pState = pState;
	pState->iShapeType = iShapeType;
	
	return pTool;
}

af_tool afToolRectCreate(void)
{
	return __createShapeTool(AF_TOOL_RECT, "Rectangle", "R", 0);
}

af_tool afToolEllipseCreate(void)
{
	return __createShapeTool(AF_TOOL_ELLIPSE, "Ellipse", "O", 1);
}

af_tool afToolLineCreate(void)
{
	return __createShapeTool(AF_TOOL_LINE, "Line", "N", 2);
}
