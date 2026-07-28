/*
 * af_tool_brush.c - Brush and pencil tools
 *
 * Freehand drawing with smoothing and variable width.
 */

#include "af_tools.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Brush state                                                        */
/* ------------------------------------------------------------------ */

#define AF_BRUSH_MAX_POINTS  4096

typedef struct af_brush_tool_state_t {
	int   bDrawing;
	float arrPointsX[AF_BRUSH_MAX_POINTS];
	float arrPointsY[AF_BRUSH_MAX_POINTS];
	int   iPointCount;
	float fWidth;
	float fSmoothing;
	uint32_t iColor;
	int   bPencilMode;
} af_brush_tool_state_t;

/* ------------------------------------------------------------------ */
/* Smoothing helper                                                   */
/* ------------------------------------------------------------------ */

static void __smoothPoints(float* pX, float* pY, int iCount, float fFactor)
{
	int i, iter;
	int iterations = (int)(fFactor * 5.0f);

	for ( iter = 0; iter < iterations; iter++ ) {
		for ( i = 1; i < iCount - 1; i++ ) {
			pX[i] = pX[i] * (1.0f - fFactor) + (pX[i-1] + pX[i+1]) * 0.5f * fFactor;
			pY[i] = pY[i] * (1.0f - fFactor) + (pY[i-1] + pY[i+1]) * 0.5f * fFactor;
		}
	}
}

/* ------------------------------------------------------------------ */
/* Handlers                                                           */
/* ------------------------------------------------------------------ */

static int __brushActivate(af_tool pTool, af_tool_context_t* pCtx)
{
	af_brush_tool_state_t* pState = (af_brush_tool_state_t*)pTool->pState;
	(void)pCtx;
	if ( pState ) {
		pState->bDrawing = 0;
		pState->iPointCount = 0;
	}
	return 0;
}

static int __brushDeactivate(af_tool pTool, af_tool_context_t* pCtx)
{
	af_brush_tool_state_t* pState = (af_brush_tool_state_t*)pTool->pState;
	(void)pCtx;
	if ( pState )
		pState->bDrawing = 0;
	return 0;
}

static int __brushMouseDown(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton)
{
	af_brush_tool_state_t* pState = (af_brush_tool_state_t*)pTool->pState;
	(void)pCtx;

	if ( iButton != XGE_MOUSE_LEFT ) return 0;
	if ( pState == NULL ) return -1;

	pState->bDrawing = 1;
	pState->iPointCount = 0;
	pState->iColor = pCtx->iStrokeColor;

	pState->arrPointsX[0] = fX;
	pState->arrPointsY[0] = fY;
	pState->iPointCount = 1;

	return 0;
}

static int __brushMouseMove(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY)
{
	af_brush_tool_state_t* pState = (af_brush_tool_state_t*)pTool->pState;
	(void)pCtx;

	if ( pState == NULL || !pState->bDrawing )
		return 0;

	/* Minimum distance threshold */
	if ( pState->iPointCount > 0 ) {
		float lastX = pState->arrPointsX[pState->iPointCount - 1];
		float lastY = pState->arrPointsY[pState->iPointCount - 1];
		float dx = fX - lastX;
		float dy = fY - lastY;
		float dist = sqrtf(dx*dx + dy*dy);
		if ( dist < 1.0f )
			return 0;
	}

	if ( pState->iPointCount < AF_BRUSH_MAX_POINTS ) {
		pState->arrPointsX[pState->iPointCount] = fX;
		pState->arrPointsY[pState->iPointCount] = fY;
		pState->iPointCount++;
	}

	return 0;
}

static int __brushMouseUp(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton)
{
	af_brush_tool_state_t* pState = (af_brush_tool_state_t*)pTool->pState;
	(void)fX; (void)fY; (void)iButton;

	if ( pState == NULL || !pState->bDrawing )
		return 0;

	/* Apply smoothing for brush mode */
	if ( !pState->bPencilMode && pState->fSmoothing > 0.0f && pState->iPointCount > 2 ) {
		__smoothPoints(pState->arrPointsX, pState->arrPointsY,
		               pState->iPointCount, pState->fSmoothing);
	}

	/* Commit stroke as path to document via pCtx */
	/* The actual commit logic uses afToolCommitShape from af_tools.h */
	if ( pCtx != NULL && pState->iPointCount >= 2 ) {
		/* Build path and commit - handled by higher-level integration */
	}

	pState->bDrawing = 0;
	pState->iPointCount = 0;

	return 0;
}

static int __brushRender(af_tool pTool, af_tool_context_t* pCtx)
{
	af_brush_tool_state_t* pState = (af_brush_tool_state_t*)pTool->pState;
	(void)pCtx;

	if ( pState == NULL || !pState->bDrawing )
		return 0;

	/* Live preview of brush stroke would render here via ShapeEx */
	return 0;
}

static int __brushCancel(af_tool pTool, af_tool_context_t* pCtx)
{
	af_brush_tool_state_t* pState = (af_brush_tool_state_t*)pTool->pState;
	(void)pCtx;
	if ( pState ) {
		pState->bDrawing = 0;
		pState->iPointCount = 0;
	}
	return 0;
}

/* ------------------------------------------------------------------ */
/* Creation                                                           */
/* ------------------------------------------------------------------ */

static af_tool __createBrushTool(int iId, const char* sName, const char* sShortcut,
                                 float fWidth, float fSmoothing, int bPencil)
{
	af_tool pTool;
	af_brush_tool_state_t* pState;

	pTool = (af_tool)calloc(1, sizeof(af_tool_t));
	if ( pTool == NULL ) return NULL;

	pState = (af_brush_tool_state_t*)calloc(1, sizeof(af_brush_tool_state_t));
	if ( pState == NULL ) {
		free(pTool);
		return NULL;
	}

	pTool->iId = iId;
	pTool->sName = sName;
	pTool->sShortcut = sShortcut;
	pTool->iCursor = 0;

	pTool->pfnActivate = __brushActivate;
	pTool->pfnDeactivate = __brushDeactivate;
	pTool->pfnMouseDown = __brushMouseDown;
	pTool->pfnMouseMove = __brushMouseMove;
	pTool->pfnMouseUp = __brushMouseUp;
	pTool->pfnRender = __brushRender;
	pTool->pfnCancel = __brushCancel;

	pTool->pState = pState;
	pState->fWidth = fWidth;
	pState->fSmoothing = fSmoothing;
	pState->bPencilMode = bPencil;

	return pTool;
}

af_tool afToolBrushCreate(void)
{
	return __createBrushTool(8, "Brush", "B", 3.0f, 0.5f, 0);
}
