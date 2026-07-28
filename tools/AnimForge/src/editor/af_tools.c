/*
 * af_tools.c - AnimForge tool manager implementation
 */

#include "af_tools.h"
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Tool manager lifecycle                                             */
/* ------------------------------------------------------------------ */

int afToolManagerInit(af_tool_manager_t* pMgr, struct af_app_t* pApp)
{
	if ( pMgr == NULL ) return -1;
	memset(pMgr, 0, sizeof(af_tool_manager_t));
	
	pMgr->tContext.pApp = pApp;
	if ( pApp != NULL ) {
		pMgr->tContext.pDoc = &pApp->tDoc;
		pMgr->tContext.fZoom = pApp->tView.fZoom;
		pMgr->tContext.fOffsetX = pApp->tView.fOffsetX;
		pMgr->tContext.fOffsetY = pApp->tView.fOffsetY;
		pMgr->tContext.iFillColor = pApp->iFillColorRGBA;
		pMgr->tContext.iStrokeColor = pApp->iStrokeColorRGBA;
		pMgr->tContext.fStrokeWidth = pApp->fStrokeWidth;
		pMgr->tContext.iFillEnabled = 1;
		pMgr->tContext.iStrokeEnabled = 1;
	}
	
	/* Create preview shape for tool overlays */
	xgeShapeExCreate(&pMgr->tContext.pPreviewShape);
	xgeShapeExSceneCreate(&pMgr->tContext.pPreviewScene);
	
	return 0;
}

void afToolManagerShutdown(af_tool_manager_t* pMgr)
{
	int i;
	if ( pMgr == NULL ) return;
	
	/* Destroy all registered tools */
	for ( i = 0; i < pMgr->iToolCount; i++ ) {
		if ( pMgr->arrTools[i] != NULL ) {
			afToolDestroy(pMgr->arrTools[i]);
			pMgr->arrTools[i] = NULL;
		}
	}
	
	if ( pMgr->tContext.pPreviewShape != NULL ) {
		xgeShapeExDestroy(pMgr->tContext.pPreviewShape);
		pMgr->tContext.pPreviewShape = NULL;
	}
	if ( pMgr->tContext.pPreviewScene != NULL ) {
		xgeShapeExSceneDestroy(pMgr->tContext.pPreviewScene);
		pMgr->tContext.pPreviewScene = NULL;
	}
	
	pMgr->iToolCount = 0;
	pMgr->pActiveTool = NULL;
}

/* ------------------------------------------------------------------ */
/* Tool registration                                                  */
/* ------------------------------------------------------------------ */

int afToolManagerRegister(af_tool_manager_t* pMgr, af_tool pTool)
{
	if ( pMgr == NULL || pTool == NULL ) return -1;
	if ( pMgr->iToolCount >= AF_TOOL_MAX ) return -2;
	
	pMgr->arrTools[pMgr->iToolCount++] = pTool;
	return 0;
}

int afToolManagerSetActive(af_tool_manager_t* pMgr, int iToolId)
{
	int i;
	af_tool pNewTool = NULL;
	
	if ( pMgr == NULL ) return -1;
	
	/* Find tool by ID */
	for ( i = 0; i < pMgr->iToolCount; i++ ) {
		if ( pMgr->arrTools[i]->iId == iToolId ) {
			pNewTool = pMgr->arrTools[i];
			break;
		}
	}
	if ( pNewTool == NULL ) return -2;
	
	/* Deactivate current tool */
	if ( pMgr->pActiveTool != NULL && pMgr->pActiveTool->pfnDeactivate != NULL ) {
		pMgr->pActiveTool->pfnDeactivate(pMgr->pActiveTool, &pMgr->tContext);
	}
	
	/* Activate new tool */
	pMgr->pActiveTool = pNewTool;
	if ( pNewTool->pfnActivate != NULL ) {
		pNewTool->pfnActivate(pNewTool, &pMgr->tContext);
	}
	
	/* Update app state */
	if ( pMgr->tContext.pApp != NULL ) {
		pMgr->tContext.pApp->iActiveTool = iToolId;
	}
	
	return 0;
}

af_tool afToolManagerGetActive(af_tool_manager_t* pMgr)
{
	if ( pMgr == NULL ) return NULL;
	return pMgr->pActiveTool;
}

const char* afToolManagerGetActiveName(af_tool_manager_t* pMgr)
{
	if ( pMgr == NULL || pMgr->pActiveTool == NULL ) return "None";
	return pMgr->pActiveTool->sName ? pMgr->pActiveTool->sName : "Unknown";
}

/* ------------------------------------------------------------------ */
/* Input dispatch                                                     */
/* ------------------------------------------------------------------ */

int afToolManagerMouseDown(af_tool_manager_t* pMgr, float fX, float fY, uint32_t iButton)
{
	af_tool pTool;
	if ( pMgr == NULL ) return -1;
	
	pTool = pMgr->pActiveTool;
	if ( pTool == NULL ) return 0;
	
	/* Update context */
	pMgr->tContext.fMouseX = fX;
	pMgr->tContext.fMouseY = fY;
	pMgr->tContext.fStartX = fX;
	pMgr->tContext.fStartY = fY;
	pMgr->tContext.iButtons = iButton;
	pMgr->bDragging = 1;
	
	/* Sync modifiers */
	pMgr->tContext.bShift = xgeKeyDown(AF_KEY_LEFT_SHIFT) || xgeKeyDown(AF_KEY_RIGHT_SHIFT);
	pMgr->tContext.bCtrl = xgeKeyDown(AF_KEY_LEFT_CONTROL) || xgeKeyDown(AF_KEY_RIGHT_CONTROL);
	pMgr->tContext.bAlt = xgeKeyDown(AF_KEY_LEFT_ALT) || xgeKeyDown(AF_KEY_RIGHT_ALT);
	
	if ( pTool->pfnMouseDown != NULL ) {
		return pTool->pfnMouseDown(pTool, &pMgr->tContext, fX, fY, iButton);
	}
	return 0;
}

int afToolManagerMouseMove(af_tool_manager_t* pMgr, float fX, float fY)
{
	af_tool pTool;
	if ( pMgr == NULL ) return -1;
	
	pTool = pMgr->pActiveTool;
	if ( pTool == NULL ) return 0;
	
	pMgr->tContext.fMouseX = fX;
	pMgr->tContext.fMouseY = fY;
	
	/* Sync modifiers */
	pMgr->tContext.bShift = xgeKeyDown(AF_KEY_LEFT_SHIFT) || xgeKeyDown(AF_KEY_RIGHT_SHIFT);
	pMgr->tContext.bCtrl = xgeKeyDown(AF_KEY_LEFT_CONTROL) || xgeKeyDown(AF_KEY_RIGHT_CONTROL);
	pMgr->tContext.bAlt = xgeKeyDown(AF_KEY_LEFT_ALT) || xgeKeyDown(AF_KEY_RIGHT_ALT);
	
	if ( pTool->pfnMouseMove != NULL ) {
		return pTool->pfnMouseMove(pTool, &pMgr->tContext, fX, fY);
	}
	return 0;
}

int afToolManagerMouseUp(af_tool_manager_t* pMgr, float fX, float fY, uint32_t iButton)
{
	af_tool pTool;
	if ( pMgr == NULL ) return -1;
	
	pTool = pMgr->pActiveTool;
	if ( pTool == NULL ) return 0;
	
	pMgr->tContext.fMouseX = fX;
	pMgr->tContext.fMouseY = fY;
	pMgr->tContext.iButtons = 0;
	pMgr->bDragging = 0;
	
	if ( pTool->pfnMouseUp != NULL ) {
		return pTool->pfnMouseUp(pTool, &pMgr->tContext, fX, fY, iButton);
	}
	return 0;
}

int afToolManagerMouseDblClick(af_tool_manager_t* pMgr, float fX, float fY)
{
	af_tool pTool;
	if ( pMgr == NULL ) return -1;
	
	pTool = pMgr->pActiveTool;
	if ( pTool == NULL ) return 0;
	
	if ( pTool->pfnMouseDblClick != NULL ) {
		return pTool->pfnMouseDblClick(pTool, &pMgr->tContext, fX, fY);
	}
	return 0;
}

int afToolManagerKeyDown(af_tool_manager_t* pMgr, int iKey)
{
	af_tool pTool;
	if ( pMgr == NULL ) return -1;
	
	pTool = pMgr->pActiveTool;
	if ( pTool == NULL ) return 0;
	
	if ( pTool->pfnKeyDown != NULL ) {
		return pTool->pfnKeyDown(pTool, &pMgr->tContext, iKey);
	}
	return 0;
}

int afToolManagerCancel(af_tool_manager_t* pMgr)
{
	af_tool pTool;
	if ( pMgr == NULL ) return -1;
	
	pTool = pMgr->pActiveTool;
	if ( pTool == NULL ) return 0;
	
	pMgr->bDragging = 0;
	
	if ( pTool->pfnCancel != NULL ) {
		return pTool->pfnCancel(pTool, &pMgr->tContext);
	}
	return 0;
}

/* ------------------------------------------------------------------ */
/* Render                                                             */
/* ------------------------------------------------------------------ */

int afToolManagerRender(af_tool_manager_t* pMgr)
{
	af_tool pTool;
	if ( pMgr == NULL ) return -1;
	
	pTool = pMgr->pActiveTool;
	if ( pTool == NULL ) return 0;
	
	/* Sync view transform from app */
	if ( pMgr->tContext.pApp != NULL ) {
		pMgr->tContext.fZoom = pMgr->tContext.pApp->tView.fZoom;
		pMgr->tContext.fOffsetX = pMgr->tContext.pApp->tView.fOffsetX;
		pMgr->tContext.fOffsetY = pMgr->tContext.pApp->tView.fOffsetY;
	}
	
	if ( pTool->pfnRender != NULL ) {
		return pTool->pfnRender(pTool, &pMgr->tContext);
	}
	return 0;
}

/* ------------------------------------------------------------------ */
/* Coordinate conversion                                              */
/* ------------------------------------------------------------------ */

void afToolScreenToStage(af_tool_manager_t* pMgr, float fScreenX, float fScreenY, float* pStageX, float* pStageY)
{
	float fZoom, fOffX, fOffY;
	float fStageW, fStageH;
	
	if ( pMgr == NULL ) return;
	
	fZoom = pMgr->tContext.fZoom;
	fOffX = pMgr->tContext.fOffsetX;
	fOffY = pMgr->tContext.fOffsetY;
	
	if ( pMgr->tContext.pDoc != NULL ) {
		fStageW = pMgr->tContext.pDoc->fStageWidth;
		fStageH = pMgr->tContext.pDoc->fStageHeight;
	} else {
		fStageW = 640.0f;
		fStageH = 480.0f;
	}
	
	/* Stage is centered in view with offset */
	if ( pStageX != NULL ) {
		*pStageX = (fScreenX - fOffX) / fZoom;
	}
	if ( pStageY != NULL ) {
		*pStageY = (fScreenY - fOffY) / fZoom;
	}
}

void afToolStageToScreen(af_tool_manager_t* pMgr, float fStageX, float fStageY, float* pScreenX, float* pScreenY)
{
	float fZoom, fOffX, fOffY;
	
	if ( pMgr == NULL ) return;
	
	fZoom = pMgr->tContext.fZoom;
	fOffX = pMgr->tContext.fOffsetX;
	fOffY = pMgr->tContext.fOffsetY;
	
	if ( pScreenX != NULL ) {
		*pScreenX = fStageX * fZoom + fOffX;
	}
	if ( pScreenY != NULL ) {
		*pScreenY = fStageY * fZoom + fOffY;
	}
}

/* ------------------------------------------------------------------ */
/* Tool destroy helper                                                */
/* ------------------------------------------------------------------ */

void afToolDestroy(af_tool pTool)
{
	if ( pTool == NULL ) return;
	if ( pTool->pState != NULL ) {
		free(pTool->pState);
	}
	free(pTool);
}

/* ------------------------------------------------------------------ */
/* Commit shape to document                                           */
/* ------------------------------------------------------------------ */

int afToolCommitShape(af_tool_context_t* pCtx, af_path_t* pPath, 
                      af_paint_t* pFill, af_paint_t* pStroke, 
                      float fStrokeWidth, uint32_t* pSymbolId)
{
	af_doc pDoc;
	af_symbol_t* pSym;
	uint32_t iSymbolId;
	int ret;
	
	if ( pCtx == NULL || pPath == NULL ) return -1;
	pDoc = pCtx->pDoc;
	if ( pDoc == NULL ) return -1;
	
	/* Create a new GRAPHIC symbol */
	ret = afDocAddSymbol(pDoc, AF_SYMBOL_GRAPHIC, "Shape", &iSymbolId);
	if ( ret != 0 ) return ret;
	
	pSym = afDocFindSymbol(pDoc, iSymbolId);
	if ( pSym == NULL ) return -1;
	
	/* Copy path to first shape record */
	pSym->iShapeCount = 1;
	memcpy(&pSym->arrShapes[0].tPath, pPath, sizeof(af_path_t));
	
	/* Set fill */
	if ( pFill != NULL ) {
		memcpy(&pSym->arrShapes[0].tFill, pFill, sizeof(af_paint_t));
	} else {
		pSym->arrShapes[0].tFill.iType = AF_PAINT_SOLID;
		pSym->arrShapes[0].tFill.iColor = pCtx->iFillColor;
	}
	
	/* Set stroke */
	if ( pStroke != NULL ) {
		memcpy(&pSym->arrShapes[0].tStroke, pStroke, sizeof(af_paint_t));
	} else if ( pCtx->iStrokeEnabled ) {
		pSym->arrShapes[0].tStroke.iType = AF_PAINT_SOLID;
		pSym->arrShapes[0].tStroke.iColor = pCtx->iStrokeColor;
	}
	
	pSym->arrShapes[0].fStrokeWidth = fStrokeWidth;
	
	/* Calculate bounds */
	if ( pPath->iPointCount > 0 ) {
		float fMinX = pPath->arrPoints[0];
		float fMinY = pPath->arrPoints[1];
		float fMaxX = fMinX;
		float fMaxY = fMinY;
		int i;
		for ( i = 1; i < pPath->iPointCount; i++ ) {
			float x = pPath->arrPoints[i * 2];
			float y = pPath->arrPoints[i * 2 + 1];
			if ( x < fMinX ) fMinX = x;
			if ( y < fMinY ) fMinY = y;
			if ( x > fMaxX ) fMaxX = x;
			if ( y > fMaxY ) fMaxY = y;
		}
		pSym->fBoundsX = fMinX;
		pSym->fBoundsY = fMinY;
		pSym->fBoundsW = fMaxX - fMinX;
		pSym->fBoundsH = fMaxY - fMinY;
	}
	
	pDoc->bModified = 1;
	
	if ( pSymbolId != NULL ) *pSymbolId = iSymbolId;
	return 0;
}
