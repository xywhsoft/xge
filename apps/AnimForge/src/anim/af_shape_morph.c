/*
 * af_shape_morph.c - Shape tween vertex interpolation
 *
 * Implements shape morphing between two keyframes by interpolating
 * path vertices. Requires isomorphic paths (same command structure).
 * Provides automatic vertex matching via resampling for non-matching paths.
 */

#include "af_shape_morph.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/* Path compatibility check                                           */
/* ------------------------------------------------------------------ */

int afShapeMorphCheckCompatible(const af_path_t* pFrom, const af_path_t* pTo)
{
	int i;

	if ( pFrom->iCommandCount != pTo->iCommandCount )
		return 0;

	for ( i = 0; i < pFrom->iCommandCount; i++ ) {
		if ( pFrom->arrCommands[i] != pTo->arrCommands[i] )
			return 0;
	}

	return 1;
}

/* ------------------------------------------------------------------ */
/* Path resampling for non-matching paths                             */
/* ------------------------------------------------------------------ */

/* Count total anchor points in a path */
static int __afCountAnchors(const af_path_t* pPath)
{
	int count = 0;
	int i;
	for ( i = 0; i < pPath->iCommandCount; i++ ) {
		switch ( pPath->arrCommands[i] ) {
		case AF_PATH_CMD_MOVE_TO:
		case AF_PATH_CMD_LINE_TO:
			count += 1;
			break;
		case AF_PATH_CMD_QUAD_TO:
			count += 1; /* endpoint only for morphing */
			break;
		case AF_PATH_CMD_CUBIC_TO:
			count += 1; /* endpoint only */
			break;
		case AF_PATH_CMD_CLOSE:
			break;
		}
	}
	return count;
}

/* Linear interpolation between two points */
static void __afLerpPoint(float ax, float ay, float bx, float by, float t,
                          float* px, float* py)
{
	*px = ax + (bx - ax) * t;
	*py = ay + (by - ay) * t;
}

/* Resample a path to have exactly N evenly-spaced points along its length */
int afShapeMorphResample(const af_path_t* pSrc, af_path_t* pDst, int iTargetPoints)
{
	/* Collect all points from source path */
	float ptsX[AF_DOC_MAX_PATH_PTS];
	float ptsY[AF_DOC_MAX_PATH_PTS];
	int ptCount = 0;
	int i;
	float totalLen = 0.0f;
	float segLen;
	float step;
	float accum;
	int srcIdx;

	/* Extract anchor points */
	for ( i = 0; i < pSrc->iPointCount && ptCount < AF_DOC_MAX_PATH_PTS; i++ ) {
		ptsX[ptCount] = pSrc->arrPoints[i * 2];
		ptsY[ptCount] = pSrc->arrPoints[i * 2 + 1];
		ptCount++;
	}

	if ( ptCount < 2 ) {
		/* Degenerate path, just copy */
		*pDst = *pSrc;
		return 0;
	}

	/* Calculate total polyline length */
	for ( i = 1; i < ptCount; i++ ) {
		float dx = ptsX[i] - ptsX[i-1];
		float dy = ptsY[i] - ptsY[i-1];
		totalLen += sqrtf(dx*dx + dy*dy);
	}

	if ( totalLen < 1e-6f ) {
		*pDst = *pSrc;
		return 0;
	}

	/* Resample at even intervals */
	step = totalLen / (float)(iTargetPoints - 1);
	accum = 0.0f;
	srcIdx = 0;

	memset(pDst, 0, sizeof(*pDst));

	/* First point */
	pDst->arrPoints[0] = ptsX[0];
	pDst->arrPoints[1] = ptsY[0];
	pDst->iPointCount = 1;
	pDst->arrCommands[0] = AF_PATH_CMD_MOVE_TO;
	pDst->iCommandCount = 1;

	for ( i = 1; i < iTargetPoints && pDst->iPointCount < AF_DOC_MAX_PATH_PTS; i++ ) {
		float targetDist = step * (float)i;
		float px, py;

		/* Walk along source polyline to find position */
		while ( srcIdx < ptCount - 2 ) {
			float dx = ptsX[srcIdx+1] - ptsX[srcIdx];
			float dy = ptsY[srcIdx+1] - ptsY[srcIdx];
			segLen = sqrtf(dx*dx + dy*dy);
			if ( accum + segLen >= targetDist )
				break;
			accum += segLen;
			srcIdx++;
		}

		/* Interpolate within current segment */
		{
			float dx = ptsX[srcIdx+1] - ptsX[srcIdx];
			float dy = ptsY[srcIdx+1] - ptsY[srcIdx];
			segLen = sqrtf(dx*dx + dy*dy);
			if ( segLen > 1e-6f ) {
				float t = (targetDist - accum) / segLen;
				if ( t < 0.0f ) t = 0.0f;
				if ( t > 1.0f ) t = 1.0f;
				__afLerpPoint(ptsX[srcIdx], ptsY[srcIdx], ptsX[srcIdx+1], ptsY[srcIdx+1], t, &px, &py);
			} else {
				px = ptsX[srcIdx];
				py = ptsY[srcIdx];
			}
		}

		pDst->arrPoints[pDst->iPointCount * 2] = px;
		pDst->arrPoints[pDst->iPointCount * 2 + 1] = py;
		pDst->iPointCount++;
		pDst->arrCommands[pDst->iCommandCount] = AF_PATH_CMD_LINE_TO;
		pDst->iCommandCount++;
	}

	return 0;
}

/* ------------------------------------------------------------------ */
/* Shape morph interpolation                                          */
/* ------------------------------------------------------------------ */

int afShapeMorphInterpolate(const af_shape_record_t* pFrom,
                            const af_shape_record_t* pTo,
                            float fT,
                            af_shape_record_t* pOut)
{
	int i;
	int useFrom = 1;
	af_path_t resampledFrom, resampledTo;
	const af_path_t* pPathFrom;
	const af_path_t* pPathTo;
	int ptCount;

	/* Check if paths are compatible */
	if ( afShapeMorphCheckCompatible(&pFrom->tPath, &pTo->tPath) ) {
		pPathFrom = &pFrom->tPath;
		pPathTo = &pTo->tPath;
	} else {
		/* Resample both paths to same point count */
		int fromPts = __afCountAnchors(&pFrom->tPath);
		int toPts = __afCountAnchors(&pTo->tPath);
		int targetPts = (fromPts > toPts) ? fromPts : toPts;
		if ( targetPts < 2 ) targetPts = 2;

		afShapeMorphResample(&pFrom->tPath, &resampledFrom, targetPts);
		afShapeMorphResample(&pTo->tPath, &resampledTo, targetPts);
		pPathFrom = &resampledFrom;
		pPathTo = &resampledTo;
		useFrom = 0;
	}

	/* Initialize output */
	memset(pOut, 0, sizeof(*pOut));

	/* Interpolate path points */
	ptCount = pPathFrom->iPointCount;
	if ( pPathTo->iPointCount < ptCount )
		ptCount = pPathTo->iPointCount;

	pOut->tPath.iCommandCount = pPathFrom->iCommandCount;
	memcpy(pOut->tPath.arrCommands, pPathFrom->arrCommands, pPathFrom->iCommandCount);

	for ( i = 0; i < ptCount; i++ ) {
		float ax = pPathFrom->arrPoints[i * 2];
		float ay = pPathFrom->arrPoints[i * 2 + 1];
		float bx = pPathTo->arrPoints[i * 2];
		float by = pPathTo->arrPoints[i * 2 + 1];
		pOut->tPath.arrPoints[i * 2] = ax + (bx - ax) * fT;
		pOut->tPath.arrPoints[i * 2 + 1] = ay + (by - ay) * fT;
	}
	pOut->tPath.iPointCount = ptCount;

	/* Interpolate fill color */
	if ( pFrom->tFill.iType == AF_PAINT_SOLID && pTo->tFill.iType == AF_PAINT_SOLID ) {
		uint32_t c0 = pFrom->tFill.iColor;
		uint32_t c1 = pTo->tFill.iColor;
		int r0 = (c0 >> 24) & 0xFF, g0 = (c0 >> 16) & 0xFF;
		int b0 = (c0 >> 8) & 0xFF, a0 = c0 & 0xFF;
		int r1 = (c1 >> 24) & 0xFF, g1 = (c1 >> 16) & 0xFF;
		int b1 = (c1 >> 8) & 0xFF, a1 = c1 & 0xFF;
		int r = r0 + (int)((r1 - r0) * fT);
		int g = g0 + (int)((g1 - g0) * fT);
		int b = b0 + (int)((b1 - b0) * fT);
		int a = a0 + (int)((a1 - a0) * fT);
		pOut->tFill.iType = AF_PAINT_SOLID;
		pOut->tFill.iColor = ((uint32_t)r << 24) | ((uint32_t)g << 16) |
		                     ((uint32_t)b << 8) | (uint32_t)a;
	} else {
		/* Use 'from' fill for non-solid or mismatched types */
		pOut->tFill = (fT < 0.5f) ? pFrom->tFill : pTo->tFill;
	}

	/* Interpolate stroke */
	pOut->fStrokeWidth = pFrom->fStrokeWidth + (pTo->fStrokeWidth - pFrom->fStrokeWidth) * fT;
	if ( pFrom->tStroke.iType == AF_PAINT_SOLID && pTo->tStroke.iType == AF_PAINT_SOLID ) {
		uint32_t c0 = pFrom->tStroke.iColor;
		uint32_t c1 = pTo->tStroke.iColor;
		int r0 = (c0 >> 24) & 0xFF, g0 = (c0 >> 16) & 0xFF;
		int b0 = (c0 >> 8) & 0xFF, a0 = c0 & 0xFF;
		int r1 = (c1 >> 24) & 0xFF, g1 = (c1 >> 16) & 0xFF;
		int b1 = (c1 >> 8) & 0xFF, a1 = c1 & 0xFF;
		int r = r0 + (int)((r1 - r0) * fT);
		int g = g0 + (int)((g1 - g0) * fT);
		int b = b0 + (int)((b1 - b0) * fT);
		int a = a0 + (int)((a1 - a0) * fT);
		pOut->tStroke.iType = AF_PAINT_SOLID;
		pOut->tStroke.iColor = ((uint32_t)r << 24) | ((uint32_t)g << 16) |
		                       ((uint32_t)b << 8) | (uint32_t)a;
	} else {
		pOut->tStroke = (fT < 0.5f) ? pFrom->tStroke : pTo->tStroke;
	}

	/* Copy other attributes from nearer keyframe */
	pOut->iFillRule = (fT < 0.5f) ? pFrom->iFillRule : pTo->iFillRule;
	pOut->iLineCap = (fT < 0.5f) ? pFrom->iLineCap : pTo->iLineCap;
	pOut->iLineJoin = (fT < 0.5f) ? pFrom->iLineJoin : pTo->iLineJoin;
	pOut->fMiterLimit = pFrom->fMiterLimit + (pTo->fMiterLimit - pFrom->fMiterLimit) * fT;

	(void)useFrom;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Multi-shape morph (entire symbol)                                  */
/* ------------------------------------------------------------------ */

int afShapeMorphInterpolateSymbol(const af_symbol_t* pFrom,
                                  const af_symbol_t* pTo,
                                  float fT,
                                  af_symbol_t* pOut)
{
	int i;
	int shapeCount;

	if ( pFrom->iType != AF_SYMBOL_GRAPHIC || pTo->iType != AF_SYMBOL_GRAPHIC )
		return -1;

	/* Use minimum shape count */
	shapeCount = (pFrom->iShapeCount < pTo->iShapeCount) ?
	              pFrom->iShapeCount : pTo->iShapeCount;

	/* Copy base symbol info */
	*pOut = *pFrom;
	pOut->iShapeCount = shapeCount;

	/* Interpolate each shape record */
	for ( i = 0; i < shapeCount; i++ ) {
		afShapeMorphInterpolate(&pFrom->arrShapes[i], &pTo->arrShapes[i], fT,
		                        &pOut->arrShapes[i]);
	}

	/* Interpolate bounds */
	pOut->fBoundsX = pFrom->fBoundsX + (pTo->fBoundsX - pFrom->fBoundsX) * fT;
	pOut->fBoundsY = pFrom->fBoundsY + (pTo->fBoundsY - pFrom->fBoundsY) * fT;
	pOut->fBoundsW = pFrom->fBoundsW + (pTo->fBoundsW - pFrom->fBoundsW) * fT;
	pOut->fBoundsH = pFrom->fBoundsH + (pTo->fBoundsH - pFrom->fBoundsH) * fT;

	return 0;
}

/* ------------------------------------------------------------------ */
/* Shape hint points (for guided morphing)                            */
/* ------------------------------------------------------------------ */

int afShapeMorphApplyHints(af_path_t* pPath, const af_shape_hint_t* pHints, int iHintCount)
{
	int h, i;

	for ( h = 0; h < iHintCount; h++ ) {
		const af_shape_hint_t* pHint = &pHints[h];
		int idx = pHint->iVertexIndex;

		if ( idx < 0 || idx >= pPath->iPointCount )
			continue;

		/* Apply hint offset */
		pPath->arrPoints[idx * 2] += pHint->fOffsetX;
		pPath->arrPoints[idx * 2 + 1] += pHint->fOffsetY;
	}

	return 0;
}

/* ------------------------------------------------------------------ */
/* Auto-generate shape hints by matching closest vertices             */
/* ------------------------------------------------------------------ */

int afShapeMorphAutoMatch(const af_path_t* pFrom, const af_path_t* pTo,
                          af_shape_hint_t* pHints, int iMaxHints, int* pHintCount)
{
	int fromPts = pFrom->iPointCount;
	int toPts = pTo->iPointCount;
	int count = 0;
	int i;

	if ( fromPts == 0 || toPts == 0 ) {
		if ( pHintCount ) *pHintCount = 0;
		return 0;
	}

	/* Simple 1:1 matching by index (for isomorphic paths) */
	for ( i = 0; i < fromPts && i < toPts && count < iMaxHints; i++ ) {
		float dx = pTo->arrPoints[i * 2] - pFrom->arrPoints[i * 2];
		float dy = pTo->arrPoints[i * 2 + 1] - pFrom->arrPoints[i * 2 + 1];

		pHints[count].iVertexIndex = i;
		pHints[count].fOffsetX = dx;
		pHints[count].fOffsetY = dy;
		count++;
	}

	if ( pHintCount ) *pHintCount = count;
	return 0;
}
