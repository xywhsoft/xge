/*
 * af_tool_fill.c - Paint bucket and gradient tools
 */

#include "af_tools.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------ */
/* Fill tool state                                                    */
/* ------------------------------------------------------------------ */

typedef struct af_fill_tool_state_t {
	int      bActive;
	float    fTolerance;
	int      iMode;  /* 0=fill, 1=gradient */
	float    fGradStartX, fGradStartY;
	float    fGradEndX, fGradEndY;
	int      bDragging;
} af_fill_tool_state_t;

/* ------------------------------------------------------------------ */
/* Fill tool handlers                                                 */
/* ------------------------------------------------------------------ */

static int __fillActivate(af_tool pTool, af_tool_context_t* pCtx)
{
	(void)pTool; (void)pCtx;
	return 0;
}

static int __fillDeactivate(af_tool pTool, af_tool_context_t* pCtx)
{
	af_fill_tool_state_t* pState = (af_fill_tool_state_t*)pTool->pState;
	(void)pCtx;
	if ( pState ) pState->bActive = 0;
	return 0;
}

static int __fillMouseDown(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton)
{
	af_fill_tool_state_t* pState = (af_fill_tool_state_t*)pTool->pState;
	if ( iButton != XGE_MOUSE_LEFT ) return 0;
	if ( pState == NULL ) return -1;

	pState->bActive = 1;

	if ( pState->iMode == 0 ) {
		/* Flood fill: find shape under cursor and fill it */
		af_app_t* pApp = pCtx->pApp;
		if ( pApp && pApp->iSelectedInstance != 0 ) {
			af_element_t* pElem = afDocFindElement(&pApp->tDoc, pApp->iSelectedInstance);
			if ( pElem ) {
				af_symbol_t* pSym = afDocFindSymbol(&pApp->tDoc, pElem->iSymbolId);
				if ( pSym && pSym->iType == AF_SYMBOL_GRAPHIC ) {
					int si;
					for ( si = 0; si < pSym->iShapeCount; si++ ) {
						pSym->arrShapes[si].tFill.iType = AF_PAINT_SOLID;
						pSym->arrShapes[si].tFill.iColor = pCtx->iFillColor;
					}
					pApp->tDoc.bModified = 1;
				}
			}
		}
	} else {
		/* Gradient: start drag */
		pState->bDragging = 1;
		pState->fGradStartX = fX;
		pState->fGradStartY = fY;
		pState->fGradEndX = fX;
		pState->fGradEndY = fY;
	}

	return 0;
}

static int __fillMouseMove(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY)
{
	af_fill_tool_state_t* pState = (af_fill_tool_state_t*)pTool->pState;
	(void)pCtx;
	if ( pState && pState->bDragging ) {
		pState->fGradEndX = fX;
		pState->fGradEndY = fY;
	}
	return 0;
}

static int __fillMouseUp(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton)
{
	af_fill_tool_state_t* pState = (af_fill_tool_state_t*)pTool->pState;
	(void)fX; (void)fY; (void)iButton;

	if ( pState && pState->bDragging && pState->iMode == 1 ) {
		/* Apply gradient to selected element */
		af_app_t* pApp = pCtx->pApp;
		if ( pApp && pApp->iSelectedInstance != 0 ) {
			af_element_t* pElem = afDocFindElement(&pApp->tDoc, pApp->iSelectedInstance);
			if ( pElem ) {
				af_symbol_t* pSym = afDocFindSymbol(&pApp->tDoc, pElem->iSymbolId);
				if ( pSym && pSym->iType == AF_SYMBOL_GRAPHIC && pSym->iShapeCount > 0 ) {
					af_paint_t* pPaint = &pSym->arrShapes[0].tFill;
					pPaint->iType = AF_PAINT_LINEAR;
					pPaint->fX1 = pState->fGradStartX;
					pPaint->fY1 = pState->fGradStartY;
					pPaint->fX2 = pState->fGradEndX;
					pPaint->fY2 = pState->fGradEndY;
					if ( pPaint->iStopCount < 2 ) {
						pPaint->iStopCount = 2;
						pPaint->arrStops[0].fOffset = 0.0f;
						pPaint->arrStops[0].iColor = pCtx->iFillColor;
						pPaint->arrStops[1].fOffset = 1.0f;
						pPaint->arrStops[1].iColor = pCtx->iStrokeColor;
					}
					pApp->tDoc.bModified = 1;
				}
			}
		}
		pState->bDragging = 0;
	}

	if ( pState ) pState->bActive = 0;
	return 0;
}

static int __fillRender(af_tool pTool, af_tool_context_t* pCtx)
{
	(void)pTool; (void)pCtx;
	return 0;
}

static int __fillCancel(af_tool pTool, af_tool_context_t* pCtx)
{
	af_fill_tool_state_t* pState = (af_fill_tool_state_t*)pTool->pState;
	(void)pCtx;
	if ( pState ) { pState->bActive = 0; pState->bDragging = 0; }
	return 0;
}

/* ------------------------------------------------------------------ */
/* Creation                                                           */
/* ------------------------------------------------------------------ */

static af_tool __createFillTool(int iId, const char* sName, const char* sShortcut, int iMode)
{
	af_tool pTool;
	af_fill_tool_state_t* pState;

	pTool = (af_tool)calloc(1, sizeof(af_tool_t));
	if ( pTool == NULL ) return NULL;

	pState = (af_fill_tool_state_t*)calloc(1, sizeof(af_fill_tool_state_t));
	if ( pState == NULL ) { free(pTool); return NULL; }

	pTool->iId = iId;
	pTool->sName = sName;
	pTool->sShortcut = sShortcut;
	pTool->iCursor = 0;

	pTool->pfnActivate = __fillActivate;
	pTool->pfnDeactivate = __fillDeactivate;
	pTool->pfnMouseDown = __fillMouseDown;
	pTool->pfnMouseMove = __fillMouseMove;
	pTool->pfnMouseUp = __fillMouseUp;
	pTool->pfnRender = __fillRender;
	pTool->pfnCancel = __fillCancel;

	pTool->pState = pState;
	pState->fTolerance = 32.0f;
	pState->iMode = iMode;

	return pTool;
}

af_tool afToolFillCreate(void)
{
	return __createFillTool(AF_TOOL_FILL, "Paint Bucket", "K", 0);
}

/* Gradient tool uses same code with mode=1, registered as AF_TOOL_GRADIENT */
af_tool afToolGradientCreate(void)
{
	return __createFillTool(AF_TOOL_GRADIENT, "Gradient", "G", 1);
}
