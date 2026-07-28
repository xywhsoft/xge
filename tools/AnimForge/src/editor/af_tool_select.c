/*
 * af_tool_select.c - Selection and transform tool
 *
 * Click to select objects, drag to move, handles for scale/rotate.
 */

#include "af_tools.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------ */
/* Selection tool state                                               */
/* ------------------------------------------------------------------ */

#define AF_SELECT_MAX_HANDLES 8

typedef struct af_select_tool_state_t {
	int bDragging;
	int bMarquee;              /* marquee selection mode */
	float fStartX, fStartY;
	float fCurX, fCurY;
	
	/* Selection */
	uint32_t arrSelected[256];
	int iSelectedCount;
	
	/* Transform */
	int iActiveHandle;         /* -1 = none, 0-7 = scale handles, 8 = rotate */
	float fOrigBounds[4];      /* original bounds x,y,w,h */
} af_select_tool_state_t;

/* ------------------------------------------------------------------ */
/* Helper: hit test symbols                                           */
/* ------------------------------------------------------------------ */

static uint32_t __hitTestSymbol(af_doc pDoc, float fX, float fY)
{
	int i;
	if ( pDoc == NULL ) return 0;
	
	/* Check symbols in reverse order (top-most first) */
	for ( i = pDoc->iSymbolCount - 1; i >= 0; i-- ) {
		af_symbol_t* pSym = &pDoc->arrSymbols[i];
		float bx = pSym->fBoundsX;
		float by = pSym->fBoundsY;
		float bw = pSym->fBoundsW;
		float bh = pSym->fBoundsH;
		
		/* Simple bounds check */
		if ( fX >= bx && fX <= bx + bw && fY >= by && fY <= by + bh ) {
			return pSym->iId;
		}
	}
	return 0;
}

/* ------------------------------------------------------------------ */
/* Selection tool handlers                                            */
/* ------------------------------------------------------------------ */

static int __selectActivate(af_tool pTool, af_tool_context_t* pCtx)
{
	(void)pTool; (void)pCtx;
	return 0;
}

static int __selectDeactivate(af_tool pTool, af_tool_context_t* pCtx)
{
	af_select_tool_state_t* pState = (af_select_tool_state_t*)pTool->pState;
	(void)pCtx;
	if ( pState != NULL ) {
		pState->bDragging = 0;
		pState->bMarquee = 0;
	}
	return 0;
}

static int __selectMouseDown(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton)
{
	af_select_tool_state_t* pState = (af_select_tool_state_t*)pTool->pState;
	uint32_t iHitId;
	
	if ( pState == NULL ) return -1;
	if ( iButton != XGE_MOUSE_LEFT ) return 0;
	
	pState->fStartX = fX;
	pState->fStartY = fY;
	pState->fCurX = fX;
	pState->fCurY = fY;
	
	/* Check if clicking on a scale handle of already-selected object */
	if ( pState->iSelectedCount == 1 ) {
		af_symbol_t* pSym = afDocFindSymbol(pCtx->pDoc, pState->arrSelected[0]);
		if ( pSym != NULL ) {
			float bx = pSym->fBoundsX - 2.0f;
			float by = pSym->fBoundsY - 2.0f;
			float bw = pSym->fBoundsW + 4.0f;
			float bh = pSym->fBoundsH + 4.0f;
			float hs = 6.0f; /* handle hit radius */
			/* Check corner handles: TL=0, TR=1, BL=2, BR=3 */
			float hpos[4][2] = {
				{ bx, by }, { bx + bw, by },
				{ bx, by + bh }, { bx + bw, by + bh }
			};
			int h;
			for ( h = 0; h < 4; h++ ) {
				if ( fabsf(fX - hpos[h][0]) <= hs && fabsf(fY - hpos[h][1]) <= hs ) {
					pState->iActiveHandle = h;
					pState->fOrigBounds[0] = pSym->fBoundsX;
					pState->fOrigBounds[1] = pSym->fBoundsY;
					pState->fOrigBounds[2] = pSym->fBoundsW;
					pState->fOrigBounds[3] = pSym->fBoundsH;
					pState->bDragging = 1;
					return 0;
				}
			}
			/* Check rotate handle (above top-center) */
			{
				float rx = bx + bw * 0.5f;
				float ry = by - 20.0f;
				if ( fabsf(fX - rx) <= hs && fabsf(fY - ry) <= hs ) {
					pState->iActiveHandle = 8; /* rotate */
					pState->fOrigBounds[0] = pSym->fBoundsX;
					pState->fOrigBounds[1] = pSym->fBoundsY;
					pState->fOrigBounds[2] = pSym->fBoundsW;
					pState->fOrigBounds[3] = pSym->fBoundsH;
					pState->bDragging = 1;
					return 0;
				}
			}
		}
	}
	
	/* Hit test */
	iHitId = __hitTestSymbol(pCtx->pDoc, fX, fY);
	
	if ( iHitId != 0 ) {
		/* Check if already selected */
		int bFound = 0;
		int i;
		for ( i = 0; i < pState->iSelectedCount; i++ ) {
			if ( pState->arrSelected[i] == iHitId ) {
				bFound = 1;
				break;
			}
		}
		
		if ( !bFound ) {
			if ( pCtx->bShift ) {
				/* Add to selection */
				if ( pState->iSelectedCount < 256 ) {
					pState->arrSelected[pState->iSelectedCount++] = iHitId;
				}
			} else {
				/* Replace selection */
				pState->arrSelected[0] = iHitId;
				pState->iSelectedCount = 1;
			}
		} else if ( pCtx->bCtrl ) {
			/* Remove from selection */
			int j;
			for ( j = i; j < pState->iSelectedCount - 1; j++ ) {
				pState->arrSelected[j] = pState->arrSelected[j+1];
			}
			pState->iSelectedCount--;
		}
		
		pState->bDragging = 1;
		
		/* Update app selection */
		if ( pCtx->pApp != NULL && pState->iSelectedCount > 0 ) {
			pCtx->pApp->iSelectedInstance = pState->arrSelected[0];
		}
	} else {
		/* Start marquee selection */
		if ( !pCtx->bShift ) {
			pState->iSelectedCount = 0;
		}
		pState->bMarquee = 1;
	}
	
	xgeRenderRequest();
	return 0;
}

static int __selectMouseMove(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY)
{
	af_select_tool_state_t* pState = (af_select_tool_state_t*)pTool->pState;
	
	if ( pState == NULL ) return 0;
	
	pState->fCurX = fX;
	pState->fCurY = fY;
	
	/* Handle scale/rotate drag */
	if ( pState->bDragging && pState->iActiveHandle >= 0 && pState->iSelectedCount == 1 ) {
		af_symbol_t* pSym = afDocFindSymbol(pCtx->pDoc, pState->arrSelected[0]);
		if ( pSym != NULL ) {
			if ( pState->iActiveHandle == 8 ) {
				/* Rotate: compute angle from center to mouse */
				float cx = pState->fOrigBounds[0] + pState->fOrigBounds[2] * 0.5f;
				float cy = pState->fOrigBounds[1] + pState->fOrigBounds[3] * 0.5f;
				float angle = atan2f(fY - cy, fX - cx) * 180.0f / 3.14159265f + 90.0f;
				/* Snap to 15 degree increments if shift held */
				if ( pCtx->bShift ) {
					angle = floorf(angle / 15.0f + 0.5f) * 15.0f;
				}
				/* Apply rotation to instances of this symbol on the active keyframe */
				if ( pCtx->pApp != NULL ) {
					af_doc pDoc = pCtx->pDoc;
					int li, ki, ei;
					for ( li = 0; li < pDoc->tTimeline.iLayerCount; li++ ) {
						af_layer_t* pLayer = &pDoc->tTimeline.arrLayers[li];
						for ( ki = 0; ki < pLayer->iKeyframeCount; ki++ ) {
							af_keyframe_t* pKf = &pLayer->arrKeyframes[ki];
							for ( ei = 0; ei < pKf->iElementCount; ei++ ) {
								if ( pKf->arrElements[ei].iSymbolId == pState->arrSelected[0] ) {
									pKf->arrElements[ei].fRotation = angle;
								}
							}
						}
					}
				}
			} else {
				/* Scale: adjust bounds based on which handle is dragged */
				float dx = fX - pState->fStartX;
				float dy = fY - pState->fStartY;
				float ox = pState->fOrigBounds[0];
				float oy = pState->fOrigBounds[1];
				float ow = pState->fOrigBounds[2];
				float oh = pState->fOrigBounds[3];
				
				switch ( pState->iActiveHandle ) {
				case 0: /* TL */
					pSym->fBoundsX = ox + dx;
					pSym->fBoundsY = oy + dy;
					pSym->fBoundsW = ow - dx;
					pSym->fBoundsH = oh - dy;
					break;
				case 1: /* TR */
					pSym->fBoundsY = oy + dy;
					pSym->fBoundsW = ow + dx;
					pSym->fBoundsH = oh - dy;
					break;
				case 2: /* BL */
					pSym->fBoundsX = ox + dx;
					pSym->fBoundsW = ow - dx;
					pSym->fBoundsH = oh + dy;
					break;
				case 3: /* BR */
					pSym->fBoundsW = ow + dx;
					pSym->fBoundsH = oh + dy;
					break;
				}
				/* Enforce minimum size */
				if ( pSym->fBoundsW < 2.0f ) pSym->fBoundsW = 2.0f;
				if ( pSym->fBoundsH < 2.0f ) pSym->fBoundsH = 2.0f;
			}
			pCtx->pDoc->bModified = 1;
		}
	}
	
	if ( pState->bDragging || pState->bMarquee ) {
		xgeRenderRequest();
	}
	return 0;
}

static int __selectMouseUp(af_tool pTool, af_tool_context_t* pCtx, float fX, float fY, uint32_t iButton)
{
	af_select_tool_state_t* pState = (af_select_tool_state_t*)pTool->pState;
	(void)iButton;
	
	if ( pState == NULL ) return 0;
	
	/* If we were scaling/rotating, just commit */
	if ( pState->iActiveHandle >= 0 ) {
		pState->iActiveHandle = -1;
		pState->bDragging = 0;
		xgeRenderRequest();
		return 0;
	}
	
	if ( pState->bDragging && pState->iSelectedCount > 0 ) {
		/* Apply move transform to selected symbols */
		float dx = fX - pState->fStartX;
		float dy = fY - pState->fStartY;
		
		if ( fabsf(dx) > 0.5f || fabsf(dy) > 0.5f ) {
			int i;
			for ( i = 0; i < pState->iSelectedCount; i++ ) {
				af_symbol_t* pSym = afDocFindSymbol(pCtx->pDoc, pState->arrSelected[i]);
				if ( pSym != NULL ) {
					pSym->fBoundsX += dx;
					pSym->fBoundsY += dy;
					/* Also move path points */
					if ( pSym->iShapeCount > 0 ) {
						int j, k;
						for ( j = 0; j < pSym->iShapeCount; j++ ) {
							for ( k = 0; k < pSym->arrShapes[j].tPath.iPointCount; k++ ) {
								pSym->arrShapes[j].tPath.arrPoints[k*2] += dx;
								pSym->arrShapes[j].tPath.arrPoints[k*2+1] += dy;
							}
						}
					}
				}
			}
			pCtx->pDoc->bModified = 1;
		}
	}
	
	if ( pState->bMarquee ) {
		/* Select all symbols within marquee rect */
		float mx = (pState->fStartX < fX) ? pState->fStartX : fX;
		float my = (pState->fStartY < fY) ? pState->fStartY : fY;
		float mw = fabsf(fX - pState->fStartX);
		float mh = fabsf(fY - pState->fStartY);
		
		if ( mw > 2.0f && mh > 2.0f ) {
			int i;
			af_doc pDoc = pCtx->pDoc;
			for ( i = 0; i < pDoc->iSymbolCount && pState->iSelectedCount < 256; i++ ) {
				af_symbol_t* pSym = &pDoc->arrSymbols[i];
				/* Check if symbol bounds intersect marquee */
				if ( pSym->fBoundsX + pSym->fBoundsW >= mx &&
				     pSym->fBoundsX <= mx + mw &&
				     pSym->fBoundsY + pSym->fBoundsH >= my &&
				     pSym->fBoundsY <= my + mh ) {
					pState->arrSelected[pState->iSelectedCount++] = pSym->iId;
				}
			}
		}
	}
	
	pState->bDragging = 0;
	pState->bMarquee = 0;
	xgeRenderRequest();
	return 0;
}

static int __selectKeyDown(af_tool pTool, af_tool_context_t* pCtx, int iKey)
{
	af_select_tool_state_t* pState = (af_select_tool_state_t*)pTool->pState;
	
	if ( pState == NULL ) return 0;
	
	/* Delete key removes selected symbols */
	if ( iKey == XGE_KEY_DELETE && pState->iSelectedCount > 0 ) {
		int i;
		for ( i = 0; i < pState->iSelectedCount; i++ ) {
			afDocRemoveSymbol(pCtx->pDoc, pState->arrSelected[i]);
		}
		pState->iSelectedCount = 0;
		pCtx->pDoc->bModified = 1;
		xgeRenderRequest();
		return 1;
	}
	
	/* Escape clears selection */
	if ( iKey == XGE_KEY_ESCAPE ) {
		pState->iSelectedCount = 0;
		xgeRenderRequest();
		return 1;
	}
	
	/* Ctrl+A selects all */
	if ( iKey == 'A' && pCtx->bCtrl ) {
		int i;
		pState->iSelectedCount = 0;
		for ( i = 0; i < pCtx->pDoc->iSymbolCount && pState->iSelectedCount < 256; i++ ) {
			pState->arrSelected[pState->iSelectedCount++] = pCtx->pDoc->arrSymbols[i].iId;
		}
		xgeRenderRequest();
		return 1;
	}
	
	return 0;
}

static int __selectRender(af_tool pTool, af_tool_context_t* pCtx)
{
	af_select_tool_state_t* pState = (af_select_tool_state_t*)pTool->pState;
	int i;
	
	if ( pState == NULL ) return 0;
	
	/* Draw selection highlights */
	for ( i = 0; i < pState->iSelectedCount; i++ ) {
		af_symbol_t* pSym = afDocFindSymbol(pCtx->pDoc, pState->arrSelected[i]);
		if ( pSym != NULL ) {
			xge_shape_ex pBox;
			float bx = pSym->fBoundsX - 2.0f;
			float by = pSym->fBoundsY - 2.0f;
			float bw = pSym->fBoundsW + 4.0f;
			float bh = pSym->fBoundsH + 4.0f;
			
			if ( xgeShapeExCreate(&pBox) == XGE_OK ) {
				xgeShapeExAppendRect(pBox, bx, by, bw, bh, 0, 0, 1);
				xgeShapeExStrokeColor(pBox, XUI_COLOR_RGBA(0, 122, 204, 255));
				xgeShapeExStrokeWidth(pBox, 1.5f);
				xgeShapeExStrokeDash(pBox, (float[]){4.0f, 2.0f}, 2, 0.0f);
				xgeShapeExDraw(pBox, 0.25f);
				xgeShapeExDestroy(pBox);
			}
			
			/* Draw corner handles */
			if ( xgeShapeExCreate(&pBox) == XGE_OK ) {
				float hs = 4.0f;  /* handle size */
				/* Top-left */
				xgeShapeExAppendRect(pBox, bx - hs, by - hs, hs*2, hs*2, 0, 0, 1);
				/* Top-right */
				xgeShapeExAppendRect(pBox, bx + bw - hs, by - hs, hs*2, hs*2, 0, 0, 1);
				/* Bottom-left */
				xgeShapeExAppendRect(pBox, bx - hs, by + bh - hs, hs*2, hs*2, 0, 0, 1);
				/* Bottom-right */
				xgeShapeExAppendRect(pBox, bx + bw - hs, by + bh - hs, hs*2, hs*2, 0, 0, 1);
				
				xgeShapeExFillColor(pBox, XUI_COLOR_RGBA(255, 255, 255, 255));
				xgeShapeExStrokeColor(pBox, XUI_COLOR_RGBA(0, 122, 204, 255));
				xgeShapeExStrokeWidth(pBox, 1.0f);
				xgeShapeExDraw(pBox, 0.25f);
				xgeShapeExDestroy(pBox);
			}
			
			/* Draw rotate handle (circle above top-center) */
			if ( xgeShapeExCreate(&pBox) == XGE_OK ) {
				float rx = bx + bw * 0.5f;
				float ry = by - 20.0f;
				/* Line from top-center to handle */
				xgeShapeExMoveTo(pBox, bx + bw * 0.5f, by);
				xgeShapeExLineTo(pBox, rx, ry);
				xgeShapeExStrokeColor(pBox, XUI_COLOR_RGBA(0, 122, 204, 255));
				xgeShapeExStrokeWidth(pBox, 1.0f);
				xgeShapeExDraw(pBox, 0.25f);
				xgeShapeExDestroy(pBox);
			}
			if ( xgeShapeExCreate(&pBox) == XGE_OK ) {
				float rx = bx + bw * 0.5f;
				float ry = by - 20.0f;
				xgeShapeExAppendEllipse(pBox, rx, ry, 5.0f, 5.0f, 1);
				xgeShapeExFillColor(pBox, XUI_COLOR_RGBA(0, 200, 100, 255));
				xgeShapeExStrokeColor(pBox, XUI_COLOR_RGBA(255, 255, 255, 255));
				xgeShapeExStrokeWidth(pBox, 1.0f);
				xgeShapeExDraw(pBox, 0.25f);
				xgeShapeExDestroy(pBox);
			}
		}
	}
	
	/* Draw marquee selection rect */
	if ( pState->bMarquee ) {
		xge_shape_ex pMarquee;
		float mx = (pState->fStartX < pState->fCurX) ? pState->fStartX : pState->fCurX;
		float my = (pState->fStartY < pState->fCurY) ? pState->fStartY : pState->fCurY;
		float mw = fabsf(pState->fCurX - pState->fStartX);
		float mh = fabsf(pState->fCurY - pState->fStartY);
		
		if ( xgeShapeExCreate(&pMarquee) == XGE_OK ) {
			xgeShapeExAppendRect(pMarquee, mx, my, mw, mh, 0, 0, 1);
			xgeShapeExFillColor(pMarquee, XUI_COLOR_RGBA(0, 122, 204, 40));
			xgeShapeExStrokeColor(pMarquee, XUI_COLOR_RGBA(0, 122, 204, 200));
			xgeShapeExStrokeWidth(pMarquee, 1.0f);
			xgeShapeExDraw(pMarquee, 0.25f);
			xgeShapeExDestroy(pMarquee);
		}
	}
	
	return 0;
}

af_tool afToolSelectCreate(void)
{
	af_tool pTool;
	af_select_tool_state_t* pState;
	
	pTool = (af_tool)calloc(1, sizeof(af_tool_t));
	if ( pTool == NULL ) return NULL;
	
	pState = (af_select_tool_state_t*)calloc(1, sizeof(af_select_tool_state_t));
	if ( pState == NULL ) {
		free(pTool);
		return NULL;
	}
	
	pTool->iId = AF_TOOL_SELECT;
	pTool->sName = "Select";
	pTool->sShortcut = "V";
	pTool->iCursor = 0;  /* arrow */
	
	pTool->pfnActivate = __selectActivate;
	pTool->pfnDeactivate = __selectDeactivate;
	pTool->pfnMouseDown = __selectMouseDown;
	pTool->pfnMouseMove = __selectMouseMove;
	pTool->pfnMouseUp = __selectMouseUp;
	pTool->pfnKeyDown = __selectKeyDown;
	pTool->pfnRender = __selectRender;
	
	pTool->pState = pState;
	pState->iActiveHandle = -1;
	
	return pTool;
}
