/*
 * af_stage.c - Stage rendering and view transform
 *
 * Handles the stage canvas rendering, view pan/zoom transforms,
 * rulers, grid, guides, and rendering of document content.
 */

#include "af_stage.h"
#include "af_draw_helper.h"
#include "../core/af_app.h"
#include <math.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* View transform helpers                                             */
/* ------------------------------------------------------------------ */

void afStageScreenToWorld(const af_stage_view_t* pView, float fSX, float fSY,
                          float* pWX, float* pWY)
{
	*pWX = (fSX - pView->fOffsetX) / pView->fZoom;
	*pWY = (fSY - pView->fOffsetY) / pView->fZoom;
}

void afStageWorldToScreen(const af_stage_view_t* pView, float fWX, float fWY,
                          float* pSX, float* pSY)
{
	*pSX = fWX * pView->fZoom + pView->fOffsetX;
	*pSY = fWY * pView->fZoom + pView->fOffsetY;
}

void afStageZoomAt(af_stage_view_t* pView, float fFactor, float fCenterX, float fCenterY)
{
	float oldZoom = pView->fZoom;
	float newZoom = oldZoom * fFactor;

	/* Clamp zoom */
	if ( newZoom < 0.05f ) newZoom = 0.05f;
	if ( newZoom > 32.0f ) newZoom = 32.0f;

	/* Adjust offset to zoom towards cursor */
	pView->fOffsetX = fCenterX - (fCenterX - pView->fOffsetX) * (newZoom / oldZoom);
	pView->fOffsetY = fCenterY - (fCenterY - pView->fOffsetY) * (newZoom / oldZoom);
	pView->fZoom = newZoom;
}

void afStageFitToView(af_stage_view_t* pView, float fStageW, float fStageH,
                      float fCanvasW, float fCanvasH)
{
	float scaleX = (fCanvasW - 40.0f) / fStageW;
	float scaleY = (fCanvasH - 40.0f) / fStageH;
	float zoom = (scaleX < scaleY) ? scaleX : scaleY;

	if ( zoom > 8.0f ) zoom = 8.0f;
	if ( zoom < 0.05f ) zoom = 0.05f;

	pView->fZoom = zoom;
	pView->fOffsetX = (fCanvasW - fStageW * zoom) * 0.5f;
	pView->fOffsetY = (fCanvasH - fStageH * zoom) * 0.5f;
}

void afStagePan(af_stage_view_t* pView, float fDX, float fDY)
{
	pView->fOffsetX += fDX;
	pView->fOffsetY += fDY;
}

/* ------------------------------------------------------------------ */
/* Grid snapping                                                      */
/* ------------------------------------------------------------------ */

float afStageSnapToGrid(const af_stage_view_t* pView, float fValue)
{
	if ( !pView->bSnapGrid || pView->fGridSize <= 0.0f )
		return fValue;
	return floorf(fValue / pView->fGridSize + 0.5f) * pView->fGridSize;
}

void afStageSnapPoint(const af_stage_view_t* pView, float* pX, float* pY)
{
	*pX = afStageSnapToGrid(pView, *pX);
	*pY = afStageSnapToGrid(pView, *pY);
}

/* ------------------------------------------------------------------ */
/* Guide management                                                   */
/* ------------------------------------------------------------------ */

void afStageInitGuides(af_stage_guides_t* pGuides)
{
	memset(pGuides, 0, sizeof(*pGuides));
}

int afStageAddGuide(af_stage_guides_t* pGuides, int bVertical, float fPosition)
{
	if ( pGuides->iCount >= AF_STAGE_MAX_GUIDES )
		return -1;

	pGuides->arrGuides[pGuides->iCount].bVertical = bVertical;
	pGuides->arrGuides[pGuides->iCount].fPosition = fPosition;
	pGuides->iCount++;
	return 0;
}

int afStageRemoveGuide(af_stage_guides_t* pGuides, int iIndex)
{
	int i;
	if ( iIndex < 0 || iIndex >= pGuides->iCount )
		return -1;

	for ( i = iIndex; i < pGuides->iCount - 1; i++ )
		pGuides->arrGuides[i] = pGuides->arrGuides[i + 1];
	pGuides->iCount--;
	return 0;
}

void afStageClearGuides(af_stage_guides_t* pGuides)
{
	pGuides->iCount = 0;
}

float afStageSnapToGuides(const af_stage_guides_t* pGuides, float fValue, int bVertical, float fThreshold)
{
	int i;
	float best = fValue;
	float bestDist = fThreshold;

	for ( i = 0; i < pGuides->iCount; i++ ) {
		if ( pGuides->arrGuides[i].bVertical != bVertical )
			continue;
		float dist = fabsf(pGuides->arrGuides[i].fPosition - fValue);
		if ( dist < bestDist ) {
			bestDist = dist;
			best = pGuides->arrGuides[i].fPosition;
		}
	}
	return best;
}

/* ------------------------------------------------------------------ */
/* Stage rendering (called from Canvas render callback)               */
/* ------------------------------------------------------------------ */

void afStageRenderBackground(xui_proxy_t* pProxy, xui_draw_context pCtx, const af_stage_view_t* pView,
                             float fStageW, float fStageH, float fCanvasW, float fCanvasH)
{
	float sx, sy, sw, sh;

	/* Fill canvas background (gray workspace) */
	afDrawFillRect(pProxy, pCtx, 0, 0, fCanvasW, fCanvasH, AF_COLOR_STAGE_BG);

	/* Calculate stage rect in screen coords */
	afStageWorldToScreen(pView, 0, 0, &sx, &sy);
	sw = fStageW * pView->fZoom;
	sh = fStageH * pView->fZoom;

	/* Draw white stage */
	afDrawFillRect(pProxy, pCtx, sx, sy, sw, sh, AF_COLOR_STAGE);

	/* Stage border */
	afDrawStrokeRect(pProxy, pCtx, sx, sy, sw, sh, 1.0f, XUI_COLOR_RGBA(0, 0, 0, 255));
}

void afStageRenderGrid(xui_proxy_t* pProxy, xui_draw_context pCtx, const af_stage_view_t* pView,
                       float fStageW, float fStageH)
{
	float gridSize = pView->fGridSize;
	float x, y;
	uint32_t gridColor = XUI_COLOR_RGBA(200, 200, 200, 80);

	if ( !pView->bShowGrid || gridSize <= 0.0f )
		return;

	/* Vertical lines */
	for ( x = 0; x <= fStageW; x += gridSize ) {
		float sx, sy1, sy2;
		afStageWorldToScreen(pView, x, 0, &sx, &sy1);
		afStageWorldToScreen(pView, x, fStageH, &sx, &sy2);
		afDrawLine(pProxy, pCtx, sx, sy1, sx, sy2, 1.0f, gridColor);
	}

	/* Horizontal lines */
	for ( y = 0; y <= fStageH; y += gridSize ) {
		float sx1, sx2, sy;
		afStageWorldToScreen(pView, 0, y, &sx1, &sy);
		afStageWorldToScreen(pView, fStageW, y, &sx2, &sy);
		afDrawLine(pProxy, pCtx, sx1, sy, sx2, sy, 1.0f, gridColor);
	}
}

void afStageRenderRulers(xui_proxy_t* pProxy, xui_draw_context pCtx, const af_stage_view_t* pView,
                         float fCanvasW, float fCanvasH, float fStageW, float fStageH)
{
	float x;
	uint32_t rulerBg = XUI_COLOR_RGBA(50, 50, 54, 255);
	uint32_t tickColor = XUI_COLOR_RGBA(120, 120, 120, 255);
	float rulerH = 20.0f;
	float step = 50.0f;

	if ( !pView->bShowRulers )
		return;

	/* Top ruler background */
	afDrawFillRect(pProxy, pCtx, 0, 0, fCanvasW, rulerH, rulerBg);

	/* Left ruler background */
	afDrawFillRect(pProxy, pCtx, 0, 0, rulerH, fCanvasH, rulerBg);

	/* Top ruler ticks */
	for ( x = 0; x <= fStageW; x += step ) {
		float sx, sy;
		afStageWorldToScreen(pView, x, 0, &sx, &sy);
		if ( sx >= rulerH && sx <= fCanvasW ) {
			afDrawLine(pProxy, pCtx, sx, rulerH - 6, sx, rulerH, 1.0f, tickColor);
		}
	}

	/* Left ruler ticks */
	for ( x = 0; x <= fStageH; x += step ) {
		float sx, sy;
		afStageWorldToScreen(pView, 0, x, &sx, &sy);
		if ( sy >= rulerH && sy <= fCanvasH ) {
			afDrawLine(pProxy, pCtx, rulerH - 6, sy, rulerH, sy, 1.0f, tickColor);
		}
	}
}

void afStageRenderGuides(xui_proxy_t* pProxy, xui_draw_context pCtx, const af_stage_view_t* pView,
                         const af_stage_guides_t* pGuides, float fCanvasW, float fCanvasH)
{
	int i;
	uint32_t guideColor = XUI_COLOR_RGBA(0, 180, 255, 180);

	for ( i = 0; i < pGuides->iCount; i++ ) {
		float sx, sy;
		if ( pGuides->arrGuides[i].bVertical ) {
			afStageWorldToScreen(pView, pGuides->arrGuides[i].fPosition, 0, &sx, &sy);
			afDrawLine(pProxy, pCtx, sx, 0, sx, fCanvasH, 1.0f, guideColor);
		} else {
			afStageWorldToScreen(pView, 0, pGuides->arrGuides[i].fPosition, &sx, &sy);
			afDrawLine(pProxy, pCtx, 0, sy, fCanvasW, sy, 1.0f, guideColor);
		}
	}
}

/* ------------------------------------------------------------------ */
/* Element rendering on stage                                         */
/* ------------------------------------------------------------------ */

void afStageRenderElement(xui_proxy_t* pProxy, xui_draw_context pCtx, const af_stage_view_t* pView,
                          af_doc pDoc, const af_element_t* pElem)
{
	af_symbol_t* pSym;
	float sx, sy;

	if ( !(pElem->iFlags & AF_ELEM_FLAG_VISIBLE) )
		return;

	pSym = afDocFindSymbol(pDoc, pElem->iSymbolId);
	if ( pSym == NULL )
		return;

	/* Transform to screen coordinates */
	afStageWorldToScreen(pView, pElem->fTx, pElem->fTy, &sx, &sy);

	/* Render based on symbol type */
	if ( pSym->iType == AF_SYMBOL_GRAPHIC ) {
		int si;
		for ( si = 0; si < pSym->iShapeCount; si++ ) {
			af_shape_record_t* pShape = &pSym->arrShapes[si];
			int pi;
			float lastX = 0, lastY = 0;
			uint32_t strokeCol = XUI_COLOR_RGBA(0,0,0,255);
			float strokeW = 1.0f;

			if ( pShape->tStroke.iType == AF_PAINT_SOLID ) {
				strokeCol = pShape->tStroke.iColor;
				strokeW = pShape->fStrokeWidth * pView->fZoom;
			}

			/* Draw path segments as lines */
			{
				int ptIdx = 0;
				for ( pi = 0; pi < pShape->tPath.iCommandCount; pi++ ) {
					uint8_t cmd = pShape->tPath.arrCommands[pi];
					switch ( cmd ) {
					case AF_PATH_CMD_MOVE_TO:
						lastX = pShape->tPath.arrPoints[ptIdx * 2];
						lastY = pShape->tPath.arrPoints[ptIdx * 2 + 1];
						ptIdx++;
						break;
					case AF_PATH_CMD_LINE_TO: {
						float x1, y1, x2, y2;
						float nx = pShape->tPath.arrPoints[ptIdx * 2];
						float ny = pShape->tPath.arrPoints[ptIdx * 2 + 1];
						afStageWorldToScreen(pView, pElem->fTx + lastX * pElem->fScaleX,
						                     pElem->fTy + lastY * pElem->fScaleY, &x1, &y1);
						afStageWorldToScreen(pView, pElem->fTx + nx * pElem->fScaleX,
						                     pElem->fTy + ny * pElem->fScaleY, &x2, &y2);
						afDrawLine(pProxy, pCtx, x1, y1, x2, y2, strokeW, strokeCol);
						lastX = nx;
						lastY = ny;
						ptIdx++;
						break;
					}
					case AF_PATH_CMD_QUAD_TO:
					case AF_PATH_CMD_CUBIC_TO:
						ptIdx += (cmd == AF_PATH_CMD_QUAD_TO) ? 2 : 3;
						break;
					case AF_PATH_CMD_CLOSE:
						break;
					}
				}
			}
		}
	} else if ( pSym->iType == AF_SYMBOL_SPRITE ) {
		/* Draw sprite bounds rectangle as placeholder */
		float w = pSym->fBoundsW * pElem->fScaleX * pView->fZoom;
		float h = pSym->fBoundsH * pElem->fScaleY * pView->fZoom;
		uint32_t col = XUI_COLOR_RGBA(100, 150, 200, (int)(180 * pElem->fOpacity));
		afDrawFillRect(pProxy, pCtx, sx, sy, w, h, col);
		afDrawStrokeRect(pProxy, pCtx, sx, sy, w, h, 1.0f, XUI_COLOR_RGBA(60, 100, 160, 255));
	}
}

/* ------------------------------------------------------------------ */
/* Selection handles rendering                                        */
/* ------------------------------------------------------------------ */

void afStageRenderSelection(xui_proxy_t* pProxy, xui_draw_context pCtx, const af_stage_view_t* pView,
                            af_doc pDoc, uint32_t iInstanceId)
{
	af_element_t* pElem;
	af_symbol_t* pSym;
	float sx, sy, sw, sh;
	uint32_t handleColor = AF_COLOR_ACCENT;
	float handleSize = 6.0f;
	int i;
	float handles[8][2];

	if ( iInstanceId == 0 )
		return;

	pElem = afDocFindElement(pDoc, iInstanceId);
	if ( pElem == NULL )
		return;

	pSym = afDocFindSymbol(pDoc, pElem->iSymbolId);
	if ( pSym == NULL )
		return;

	/* Calculate selection bounds in screen coords */
	afStageWorldToScreen(pView, pElem->fTx + pSym->fBoundsX * pElem->fScaleX,
	                     pElem->fTy + pSym->fBoundsY * pElem->fScaleY, &sx, &sy);
	sw = pSym->fBoundsW * pElem->fScaleX * pView->fZoom;
	sh = pSym->fBoundsH * pElem->fScaleY * pView->fZoom;

	/* Draw selection rectangle */
	afDrawStrokeRect(pProxy, pCtx, sx, sy, sw, sh, 1.0f, handleColor);

	/* 8 resize handles */
	handles[0][0] = sx;          handles[0][1] = sy;
	handles[1][0] = sx + sw/2;   handles[1][1] = sy;
	handles[2][0] = sx + sw;     handles[2][1] = sy;
	handles[3][0] = sx + sw;     handles[3][1] = sy + sh/2;
	handles[4][0] = sx + sw;     handles[4][1] = sy + sh;
	handles[5][0] = sx + sw/2;   handles[5][1] = sy + sh;
	handles[6][0] = sx;          handles[6][1] = sy + sh;
	handles[7][0] = sx;          handles[7][1] = sy + sh/2;

	for ( i = 0; i < 8; i++ ) {
		afDrawFillRect(pProxy, pCtx, handles[i][0] - handleSize/2,
		               handles[i][1] - handleSize/2, handleSize, handleSize,
		               XUI_COLOR_RGBA(255, 255, 255, 255));
		afDrawStrokeRect(pProxy, pCtx, handles[i][0] - handleSize/2,
		                 handles[i][1] - handleSize/2, handleSize, handleSize,
		                 1.0f, handleColor);
	}

	/* Rotation handle (above top-center) */
	{
		float rx = sx + sw/2;
		float ry = sy - 20.0f;
		afDrawLine(pProxy, pCtx, sx + sw/2, sy, rx, ry, 1.0f, handleColor);
		afDrawCircleFill(pProxy, pCtx, rx, ry, 4.0f, handleColor);
	}
}
