#include <float.h>
#include <limits.h>

#define XGE_SHAPE_EX_MAGIC 0x58475358u
#define XGE_SHAPE_EX_SCENE_MAGIC 0x58475343u
#define XGE_SHAPE_EX_DEFAULT_TOLERANCE 0.35f
#define XGE_SHAPE_EX_KAPPA 0.5522847498307936f
#define XGE_SHAPE_EX_PI 3.14159265358979323846f
#define XGE_SHAPE_EX_TAU 6.28318530717958647692f
#define XGE_SHAPE_EX_EPSILON 0.00001f
#define XGE_SHAPE_EX_FILL_SOLID 0
#define XGE_SHAPE_EX_FILL_LINEAR_GRADIENT 1
#define XGE_SHAPE_EX_FILL_RADIAL_GRADIENT 2

struct xge_shape_ex_t {
	uint32_t iMagic;
	int iRefCount;
	uint8_t* pCommands;
	int iCommandCount;
	int iCommandCapacity;
	xge_vec2_t* pPoints;
	int iPointCount;
	int iPointCapacity;
	xge_vec2_t tCurrent;
	xge_vec2_t tStart;
	xge_vec2_t tLastCubicControl;
	xge_vec2_t tLastQuadControl;
	int bHasCurrent;
	int bHasStart;
	int iFillType;
	uint32_t iFillColor;
	int iStrokeType;
	uint32_t iStrokeColor;
	float fStrokeWidth;
	float fFillX1;
	float fFillY1;
	float fFillX2;
	float fFillY2;
	float fFillR;
	float fFillFX;
	float fFillFY;
	int iFillGradientUnits;
	int iFillGradientSpread;
	xge_shape_ex_matrix_t tFillGradientTransform;
	xge_shape_ex_color_stop_t* pFillStops;
	int iFillStopCount;
	int iFillStopCapacity;
	float fStrokeX1;
	float fStrokeY1;
	float fStrokeX2;
	float fStrokeY2;
	float fStrokeR;
	float fStrokeFX;
	float fStrokeFY;
	int iStrokeGradientUnits;
	int iStrokeGradientSpread;
	xge_shape_ex_matrix_t tStrokeGradientTransform;
	xge_shape_ex_color_stop_t* pStrokeStops;
	int iStrokeStopCount;
	int iStrokeStopCapacity;
	int iFillRule;
	int iLineJoin;
	int iLineCap;
	float fMiterLimit;
	float* pDashPattern;
	int iDashCount;
	float fDashOffset;
	int bStrokeFirst;
	float fOpacity;
	int bVisible;
	int bClipRect;
	xge_rect_t tClipRect;
	xge_shape_ex_matrix_t tTransform;
};

struct xge_shape_ex_scene_t {
	uint32_t iMagic;
	int iRefCount;
	xge_shape_ex* pShapes;
	int iShapeCount;
	int iShapeCapacity;
	float fOpacity;
	int bVisible;
	xge_shape_ex_matrix_t tTransform;
};

typedef struct xge_shape_ex_flat_contour_t {
	int iStart;
	int iCount;
	int bClosed;
} xge_shape_ex_flat_contour_t;

typedef struct xge_shape_ex_flat_path_t {
	xge_vec2_t* pPoints;
	int iPointCount;
	int iPointCapacity;
	xge_shape_ex_flat_contour_t* pContours;
	int iContourCount;
	int iContourCapacity;
} xge_shape_ex_flat_path_t;

static xge_shape_ex_matrix_t __xgeShapeExMatrixIdentity(void)
{
	xge_shape_ex_matrix_t tMatrix;

	tMatrix.fA = 1.0f;
	tMatrix.fB = 0.0f;
	tMatrix.fC = 0.0f;
	tMatrix.fD = 1.0f;
	tMatrix.fE = 0.0f;
	tMatrix.fF = 0.0f;
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeShapeExMatrixMul(xge_shape_ex_matrix_t tParent, xge_shape_ex_matrix_t tLocal)
{
	xge_shape_ex_matrix_t tOut;

	tOut.fA = (tParent.fA * tLocal.fA) + (tParent.fC * tLocal.fB);
	tOut.fB = (tParent.fB * tLocal.fA) + (tParent.fD * tLocal.fB);
	tOut.fC = (tParent.fA * tLocal.fC) + (tParent.fC * tLocal.fD);
	tOut.fD = (tParent.fB * tLocal.fC) + (tParent.fD * tLocal.fD);
	tOut.fE = (tParent.fA * tLocal.fE) + (tParent.fC * tLocal.fF) + tParent.fE;
	tOut.fF = (tParent.fB * tLocal.fE) + (tParent.fD * tLocal.fF) + tParent.fF;
	return tOut;
}

static xge_vec2_t __xgeShapeExMatrixPoint(xge_shape_ex_matrix_t tMatrix, xge_vec2_t tPoint)
{
	xge_vec2_t tOut;

	tOut.fX = (tMatrix.fA * tPoint.fX) + (tMatrix.fC * tPoint.fY) + tMatrix.fE;
	tOut.fY = (tMatrix.fB * tPoint.fX) + (tMatrix.fD * tPoint.fY) + tMatrix.fF;
	return tOut;
}

static xge_rect_t __xgeShapeExMatrixRectBounds(xge_shape_ex_matrix_t tMatrix, xge_rect_t tRect)
{
	xge_vec2_t tP[4];
	float fMinX;
	float fMinY;
	float fMaxX;
	float fMaxY;
	int i;

	tP[0].fX = tRect.fX; tP[0].fY = tRect.fY;
	tP[1].fX = tRect.fX + tRect.fW; tP[1].fY = tRect.fY;
	tP[2].fX = tRect.fX + tRect.fW; tP[2].fY = tRect.fY + tRect.fH;
	tP[3].fX = tRect.fX; tP[3].fY = tRect.fY + tRect.fH;
	for ( i = 0; i < 4; i++ ) {
		tP[i] = __xgeShapeExMatrixPoint(tMatrix, tP[i]);
	}
	fMinX = fMaxX = tP[0].fX;
	fMinY = fMaxY = tP[0].fY;
	for ( i = 1; i < 4; i++ ) {
		if ( tP[i].fX < fMinX ) fMinX = tP[i].fX;
		if ( tP[i].fY < fMinY ) fMinY = tP[i].fY;
		if ( tP[i].fX > fMaxX ) fMaxX = tP[i].fX;
		if ( tP[i].fY > fMaxY ) fMaxY = tP[i].fY;
	}
	tRect.fX = fMinX;
	tRect.fY = fMinY;
	tRect.fW = fMaxX - fMinX;
	tRect.fH = fMaxY - fMinY;
	return tRect;
}

static xge_rect_t __xgeShapeExRectIntersect(xge_rect_t a, xge_rect_t b)
{
	float fLeft = fmaxf(a.fX, b.fX);
	float fTop = fmaxf(a.fY, b.fY);
	float fRight = fminf(a.fX + a.fW, b.fX + b.fW);
	float fBottom = fminf(a.fY + a.fH, b.fY + b.fH);
	xge_rect_t tOut;

	tOut.fX = fLeft;
	tOut.fY = fTop;
	tOut.fW = fRight - fLeft;
	tOut.fH = fBottom - fTop;
	if ( tOut.fW < 0.0f ) tOut.fW = 0.0f;
	if ( tOut.fH < 0.0f ) tOut.fH = 0.0f;
	return tOut;
}

static int __xgeShapeExMatrixInverse(xge_shape_ex_matrix_t tMatrix, xge_shape_ex_matrix_t* pInverse)
{
	float fDet;
	xge_shape_ex_matrix_t tOut;

	if ( pInverse == NULL ) {
		return 0;
	}
	fDet = (tMatrix.fA * tMatrix.fD) - (tMatrix.fB * tMatrix.fC);
	if ( fabsf(fDet) <= XGE_SHAPE_EX_EPSILON ) {
		return 0;
	}
	tOut.fA = tMatrix.fD / fDet;
	tOut.fB = -tMatrix.fB / fDet;
	tOut.fC = -tMatrix.fC / fDet;
	tOut.fD = tMatrix.fA / fDet;
	tOut.fE = ((tMatrix.fC * tMatrix.fF) - (tMatrix.fD * tMatrix.fE)) / fDet;
	tOut.fF = ((tMatrix.fB * tMatrix.fE) - (tMatrix.fA * tMatrix.fF)) / fDet;
	*pInverse = tOut;
	return 1;
}

static float __xgeShapeExMatrixStrokeScale(xge_shape_ex_matrix_t tMatrix)
{
	float fSX;
	float fSY;
	float fScale;

	fSX = sqrtf((tMatrix.fA * tMatrix.fA) + (tMatrix.fB * tMatrix.fB));
	fSY = sqrtf((tMatrix.fC * tMatrix.fC) + (tMatrix.fD * tMatrix.fD));
	fScale = (fSX + fSY) * 0.5f;
	if ( fScale <= XGE_SHAPE_EX_EPSILON ) {
		fScale = 1.0f;
	}
	return fScale;
}

static int __xgeShapeExValid(xge_shape_ex pShape)
{
	return (pShape != NULL) && (pShape->iMagic == XGE_SHAPE_EX_MAGIC);
}

static int __xgeShapeExSceneValid(xge_shape_ex_scene pScene)
{
	return (pScene != NULL) && (pScene->iMagic == XGE_SHAPE_EX_SCENE_MAGIC);
}

static uint32_t __xgeShapeExColorOpacity(uint32_t iColor, float fOpacity)
{
	int iA;

	if ( fOpacity < 0.0f ) fOpacity = 0.0f;
	if ( fOpacity > 1.0f ) fOpacity = 1.0f;
	iA = (int)((float)XGE_COLOR_GET_A(iColor) * fOpacity + 0.5f);
	if ( iA < 0 ) iA = 0;
	if ( iA > 255 ) iA = 255;
	return XGE_COLOR_RGBA(XGE_COLOR_GET_R(iColor), XGE_COLOR_GET_G(iColor), XGE_COLOR_GET_B(iColor), iA);
}

static uint32_t __xgeShapeExColorLerp(uint32_t iA, uint32_t iB, float fT)
{
	int r;
	int g;
	int b;
	int a;

	if ( fT < 0.0f ) fT = 0.0f;
	if ( fT > 1.0f ) fT = 1.0f;
	r = (int)((float)XGE_COLOR_GET_R(iA) + ((float)XGE_COLOR_GET_R(iB) - (float)XGE_COLOR_GET_R(iA)) * fT + 0.5f);
	g = (int)((float)XGE_COLOR_GET_G(iA) + ((float)XGE_COLOR_GET_G(iB) - (float)XGE_COLOR_GET_G(iA)) * fT + 0.5f);
	b = (int)((float)XGE_COLOR_GET_B(iA) + ((float)XGE_COLOR_GET_B(iB) - (float)XGE_COLOR_GET_B(iA)) * fT + 0.5f);
	a = (int)((float)XGE_COLOR_GET_A(iA) + ((float)XGE_COLOR_GET_A(iB) - (float)XGE_COLOR_GET_A(iA)) * fT + 0.5f);
	if ( r < 0 ) r = 0;
	if ( r > 255 ) r = 255;
	if ( g < 0 ) g = 0;
	if ( g > 255 ) g = 255;
	if ( b < 0 ) b = 0;
	if ( b > 255 ) b = 255;
	if ( a < 0 ) a = 0;
	if ( a > 255 ) a = 255;
	return XGE_COLOR_RGBA(r, g, b, a);
}

static float __xgeShapeExGradientSpreadT(int iSpread, float fT)
{
	if ( iSpread == XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT ) {
		fT = fT - floorf(fT);
		if ( fT < 0.0f ) fT += 1.0f;
		return fT;
	}
	if ( iSpread == XGE_SHAPE_EX_GRADIENT_SPREAD_REFLECT ) {
		fT = fT - floorf(fT * 0.5f) * 2.0f;
		if ( fT < 0.0f ) fT += 2.0f;
		return fT <= 1.0f ? fT : (2.0f - fT);
	}
	if ( fT < 0.0f ) return 0.0f;
	if ( fT > 1.0f ) return 1.0f;
	return fT;
}

static xge_vec2_t __xgeShapeExGradientSamplePoint(int iUnits, xge_shape_ex_matrix_t tGradientTransform, xge_vec2_t tPoint, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, int* pOk)
{
	xge_vec2_t tSample;
	xge_shape_ex_matrix_t tInverse;

	if ( pOk != NULL ) {
		*pOk = 1;
	}
	if ( iUnits == XGE_SHAPE_EX_GRADIENT_USER_SPACE ) {
		if ( !__xgeShapeExMatrixInverse(tMatrix, &tInverse) ) {
			if ( pOk != NULL ) {
				*pOk = 0;
			}
			return tPoint;
		}
		tSample = __xgeShapeExMatrixPoint(tInverse, tPoint);
	} else {
		float fW = fabsf(tBounds.fW) > XGE_SHAPE_EX_EPSILON ? tBounds.fW : 1.0f;
		float fH = fabsf(tBounds.fH) > XGE_SHAPE_EX_EPSILON ? tBounds.fH : 1.0f;

		tSample.fX = (tPoint.fX - tBounds.fX) / fW;
		tSample.fY = (tPoint.fY - tBounds.fY) / fH;
	}
	if ( !__xgeShapeExMatrixInverse(tGradientTransform, &tInverse) ) {
		if ( pOk != NULL ) {
			*pOk = 0;
		}
		return tSample;
	}
	tSample = __xgeShapeExMatrixPoint(tInverse, tSample);
	return tSample;
}

static uint32_t __xgeShapeExGradientStopColor(const xge_shape_ex_color_stop_t* pStops, int iStopCount, uint32_t iFallbackColor, int iSpread, float fT)
{
	int i;

	if ( (pStops == NULL) || (iStopCount <= 0) ) {
		return iFallbackColor;
	}
	fT = __xgeShapeExGradientSpreadT(iSpread, fT);
	if ( fT <= pStops[0].fOffset ) {
		return pStops[0].iColor;
	}
	for ( i = 1; i < iStopCount; i++ ) {
		float fA = pStops[i - 1].fOffset;
		float fB = pStops[i].fOffset;

		if ( fT <= fB ) {
			float fLocal = (fabsf(fB - fA) <= XGE_SHAPE_EX_EPSILON) ? 0.0f : ((fT - fA) / (fB - fA));
			return __xgeShapeExColorLerp(pStops[i - 1].iColor, pStops[i].iColor, fLocal);
		}
	}
	return pStops[iStopCount - 1].iColor;
}

static uint32_t __xgeShapeExLinearGradientColorEx(float fX1, float fY1, float fX2, float fY2, int iUnits, xge_shape_ex_matrix_t tGradientTransform, const xge_shape_ex_color_stop_t* pStops, int iStopCount, uint32_t iFallbackColor, int iSpread, xge_vec2_t tPoint, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix)
{
	xge_vec2_t tA;
	xge_vec2_t tB;
	xge_vec2_t tSample;
	float fDX;
	float fDY;
	float fLenSq;
	float fT;
	int bOk;

	if ( (pStops == NULL) || (iStopCount <= 0) ) {
		return iFallbackColor;
	}
	tSample = __xgeShapeExGradientSamplePoint(iUnits, tGradientTransform, tPoint, tBounds, tMatrix, &bOk);
	if ( !bOk ) {
		return iFallbackColor;
	}
	tA.fX = fX1;
	tA.fY = fY1;
	tB.fX = fX2;
	tB.fY = fY2;
	fDX = tB.fX - tA.fX;
	fDY = tB.fY - tA.fY;
	fLenSq = (fDX * fDX) + (fDY * fDY);
	if ( fLenSq <= XGE_SHAPE_EX_EPSILON ) {
		return pStops[iStopCount - 1].iColor;
	}
	fT = (((tSample.fX - tA.fX) * fDX) + ((tSample.fY - tA.fY) * fDY)) / fLenSq;
	return __xgeShapeExGradientStopColor(pStops, iStopCount, iFallbackColor, iSpread, fT);
}

static float __xgeShapeExRadialGradientT(xge_vec2_t tPoint, xge_vec2_t tCenter, xge_vec2_t tFocal, float fRadius)
{
	float fDX;
	float fDY;
	float fFX;
	float fFY;
	float fA;
	float fB;
	float fC;
	float fDisc;
	float fS1;
	float fS2;
	float fS;

	if ( fRadius <= XGE_SHAPE_EX_EPSILON ) {
		return 1.0f;
	}
	fDX = tPoint.fX - tFocal.fX;
	fDY = tPoint.fY - tFocal.fY;
	fA = (fDX * fDX) + (fDY * fDY);
	if ( fA <= XGE_SHAPE_EX_EPSILON ) {
		return 0.0f;
	}
	fFX = tFocal.fX - tCenter.fX;
	fFY = tFocal.fY - tCenter.fY;
	fB = 2.0f * ((fDX * fFX) + (fDY * fFY));
	fC = (fFX * fFX) + (fFY * fFY) - (fRadius * fRadius);
	fDisc = (fB * fB) - (4.0f * fA * fC);
	if ( fDisc <= XGE_SHAPE_EX_EPSILON ) {
		return sqrtf(fA) / fRadius;
	}
	fDisc = sqrtf(fDisc);
	fS1 = (-fB + fDisc) / (2.0f * fA);
	fS2 = (-fB - fDisc) / (2.0f * fA);
	fS = fS1 > fS2 ? fS1 : fS2;
	if ( fS <= XGE_SHAPE_EX_EPSILON ) {
		fS = fS1 > XGE_SHAPE_EX_EPSILON ? fS1 : fS2;
	}
	if ( fS <= XGE_SHAPE_EX_EPSILON ) {
		return sqrtf(fA) / fRadius;
	}
	return 1.0f / fS;
}

static uint32_t __xgeShapeExRadialGradientColorEx(float fCX, float fCY, float fRadius, float fFX, float fFY, int iUnits, xge_shape_ex_matrix_t tGradientTransform, const xge_shape_ex_color_stop_t* pStops, int iStopCount, uint32_t iFallbackColor, int iSpread, xge_vec2_t tPoint, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix)
{
	xge_vec2_t tCenter;
	xge_vec2_t tFocal;
	xge_vec2_t tSample;
	float fT;
	int bOk;

	if ( (pStops == NULL) || (iStopCount <= 0) ) {
		return iFallbackColor;
	}
	tSample = __xgeShapeExGradientSamplePoint(iUnits, tGradientTransform, tPoint, tBounds, tMatrix, &bOk);
	if ( !bOk ) {
		return iFallbackColor;
	}
	tCenter.fX = fCX;
	tCenter.fY = fCY;
	tFocal.fX = fFX;
	tFocal.fY = fFY;
	fT = __xgeShapeExRadialGradientT(tSample, tCenter, tFocal, fRadius);
	return __xgeShapeExGradientStopColor(pStops, iStopCount, iFallbackColor, iSpread, fT);
}

static uint32_t __xgeShapeExLinearGradientColor(const xge_shape_ex pShape, xge_vec2_t tPoint, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix)
{
	if ( (pShape == NULL) || (pShape->iFillType != XGE_SHAPE_EX_FILL_LINEAR_GRADIENT) ) {
		return pShape != NULL ? pShape->iFillColor : XGE_COLOR_RGBA(0, 0, 0, 0);
	}
	return __xgeShapeExLinearGradientColorEx(pShape->fFillX1, pShape->fFillY1, pShape->fFillX2, pShape->fFillY2,
		pShape->iFillGradientUnits, pShape->tFillGradientTransform, pShape->pFillStops, pShape->iFillStopCount,
		pShape->iFillColor, pShape->iFillGradientSpread, tPoint, tBounds, tMatrix);
}

static uint32_t __xgeShapeExRadialGradientColor(const xge_shape_ex pShape, xge_vec2_t tPoint, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix)
{
	if ( (pShape == NULL) || (pShape->iFillType != XGE_SHAPE_EX_FILL_RADIAL_GRADIENT) ) {
		return pShape != NULL ? pShape->iFillColor : XGE_COLOR_RGBA(0, 0, 0, 0);
	}
	return __xgeShapeExRadialGradientColorEx(pShape->fFillX1, pShape->fFillY1, pShape->fFillR, pShape->fFillFX, pShape->fFillFY,
		pShape->iFillGradientUnits, pShape->tFillGradientTransform, pShape->pFillStops, pShape->iFillStopCount,
		pShape->iFillColor, pShape->iFillGradientSpread, tPoint, tBounds, tMatrix);
}

static int __xgeShapeExFillVisible(xge_shape_ex pShape, uint32_t iSolidColor)
{
	int i;

	if ( pShape == NULL ) {
		return 0;
	}
	if ( pShape->iFillType == XGE_SHAPE_EX_FILL_SOLID ) {
		return XGE_COLOR_GET_A(iSolidColor) != 0u;
	}
	for ( i = 0; i < pShape->iFillStopCount; i++ ) {
		if ( XGE_COLOR_GET_A(pShape->pFillStops[i].iColor) != 0u ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeShapeExStrokeVisible(xge_shape_ex pShape, uint32_t iSolidColor)
{
	int i;

	if ( pShape == NULL ) {
		return 0;
	}
	if ( pShape->iStrokeType == XGE_SHAPE_EX_FILL_SOLID ) {
		return XGE_COLOR_GET_A(iSolidColor) != 0u;
	}
	for ( i = 0; i < pShape->iStrokeStopCount; i++ ) {
		if ( XGE_COLOR_GET_A(pShape->pStrokeStops[i].iColor) != 0u ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeShapeExReserveCommands(xge_shape_ex pShape, int iNeeded)
{
	uint8_t* pCommands;
	int iCapacity;

	if ( !__xgeShapeExValid(pShape) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pShape->iCommandCapacity ) {
		return XGE_OK;
	}
	iCapacity = pShape->iCommandCapacity > 0 ? pShape->iCommandCapacity : 16;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pCommands = (uint8_t*)xrtRealloc(pShape->pCommands, (size_t)iCapacity * sizeof(*pCommands));
	if ( pCommands == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pShape->pCommands = pCommands;
	pShape->iCommandCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeShapeExReservePoints(xge_shape_ex pShape, int iNeeded)
{
	xge_vec2_t* pPoints;
	int iCapacity;

	if ( !__xgeShapeExValid(pShape) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pShape->iPointCapacity ) {
		return XGE_OK;
	}
	iCapacity = pShape->iPointCapacity > 0 ? pShape->iPointCapacity : 32;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pPoints = (xge_vec2_t*)xrtRealloc(pShape->pPoints, (size_t)iCapacity * sizeof(*pPoints));
	if ( pPoints == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pShape->pPoints = pPoints;
	pShape->iPointCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeShapeExReserveFillStops(xge_shape_ex pShape, int iNeeded)
{
	xge_shape_ex_color_stop_t* pStops;
	int iCapacity;

	if ( !__xgeShapeExValid(pShape) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pShape->iFillStopCapacity ) {
		return XGE_OK;
	}
	iCapacity = pShape->iFillStopCapacity > 0 ? pShape->iFillStopCapacity : 4;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pStops = (xge_shape_ex_color_stop_t*)xrtRealloc(pShape->pFillStops, (size_t)iCapacity * sizeof(*pStops));
	if ( pStops == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pShape->pFillStops = pStops;
	pShape->iFillStopCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeShapeExReserveStrokeStops(xge_shape_ex pShape, int iNeeded)
{
	xge_shape_ex_color_stop_t* pStops;
	int iCapacity;

	if ( !__xgeShapeExValid(pShape) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pShape->iStrokeStopCapacity ) {
		return XGE_OK;
	}
	iCapacity = pShape->iStrokeStopCapacity > 0 ? pShape->iStrokeStopCapacity : 4;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pStops = (xge_shape_ex_color_stop_t*)xrtRealloc(pShape->pStrokeStops, (size_t)iCapacity * sizeof(*pStops));
	if ( pStops == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pShape->pStrokeStops = pStops;
	pShape->iStrokeStopCapacity = iCapacity;
	return XGE_OK;
}

static void __xgeShapeExSortStops(xge_shape_ex_color_stop_t* pStops, int iCount)
{
	int i;
	int j;

	if ( (pStops == NULL) || (iCount <= 1) ) {
		return;
	}
	for ( i = 1; i < iCount; i++ ) {
		xge_shape_ex_color_stop_t tStop = pStops[i];
		j = i - 1;
		while ( (j >= 0) && (pStops[j].fOffset > tStop.fOffset) ) {
			pStops[j + 1] = pStops[j];
			j--;
		}
		pStops[j + 1] = tStop;
	}
}

static int __xgeShapeExAppendCommand(xge_shape_ex pShape, uint8_t iCommand, const xge_vec2_t* pPoints, int iPointCount)
{
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (iPointCount < 0) || ((pPoints == NULL) && (iPointCount > 0)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeShapeExReserveCommands(pShape, pShape->iCommandCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeShapeExReservePoints(pShape, pShape->iPointCount + iPointCount);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pShape->pCommands[pShape->iCommandCount++] = iCommand;
	if ( iPointCount > 0 ) {
		memcpy(pShape->pPoints + pShape->iPointCount, pPoints, (size_t)iPointCount * sizeof(*pPoints));
		pShape->iPointCount += iPointCount;
	}
	return XGE_OK;
}

static int __xgeShapeExAddMoveTo(xge_shape_ex pShape, float fX, float fY)
{
	xge_vec2_t tPoint;
	int iRet;

	tPoint.fX = fX;
	tPoint.fY = fY;
	iRet = __xgeShapeExAppendCommand(pShape, XGE_SHAPE_EX_CMD_MOVE_TO, &tPoint, 1);
	if ( iRet == XGE_OK ) {
		pShape->tCurrent = tPoint;
		pShape->tStart = tPoint;
		pShape->tLastCubicControl = tPoint;
		pShape->tLastQuadControl = tPoint;
		pShape->bHasCurrent = 1;
		pShape->bHasStart = 1;
	}
	return iRet;
}

static int __xgeShapeExAddLineTo(xge_shape_ex pShape, float fX, float fY)
{
	xge_vec2_t tPoint;
	int iRet;

	if ( !pShape->bHasCurrent ) {
		return __xgeShapeExAddMoveTo(pShape, fX, fY);
	}
	tPoint.fX = fX;
	tPoint.fY = fY;
	iRet = __xgeShapeExAppendCommand(pShape, XGE_SHAPE_EX_CMD_LINE_TO, &tPoint, 1);
	if ( iRet == XGE_OK ) {
		pShape->tCurrent = tPoint;
		pShape->tLastCubicControl = tPoint;
		pShape->tLastQuadControl = tPoint;
	}
	return iRet;
}

static int __xgeShapeExAddCubicTo(xge_shape_ex pShape, float fC1X, float fC1Y, float fC2X, float fC2Y, float fX, float fY)
{
	xge_vec2_t arrPoints[3];
	int iRet;

	if ( !pShape->bHasCurrent ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	arrPoints[0].fX = fC1X;
	arrPoints[0].fY = fC1Y;
	arrPoints[1].fX = fC2X;
	arrPoints[1].fY = fC2Y;
	arrPoints[2].fX = fX;
	arrPoints[2].fY = fY;
	iRet = __xgeShapeExAppendCommand(pShape, XGE_SHAPE_EX_CMD_CUBIC_TO, arrPoints, 3);
	if ( iRet == XGE_OK ) {
		pShape->tLastCubicControl = arrPoints[1];
		pShape->tLastQuadControl = arrPoints[2];
		pShape->tCurrent = arrPoints[2];
	}
	return iRet;
}

static int __xgeShapeExFlatReservePoints(xge_shape_ex_flat_path_t* pFlat, int iNeeded)
{
	xge_vec2_t* pPoints;
	int iCapacity;

	if ( (pFlat == NULL) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pFlat->iPointCapacity ) {
		return XGE_OK;
	}
	iCapacity = pFlat->iPointCapacity > 0 ? pFlat->iPointCapacity : 64;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pPoints = (xge_vec2_t*)xrtRealloc(pFlat->pPoints, (size_t)iCapacity * sizeof(*pPoints));
	if ( pPoints == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pFlat->pPoints = pPoints;
	pFlat->iPointCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeShapeExFlatReserveContours(xge_shape_ex_flat_path_t* pFlat, int iNeeded)
{
	xge_shape_ex_flat_contour_t* pContours;
	int iCapacity;

	if ( (pFlat == NULL) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pFlat->iContourCapacity ) {
		return XGE_OK;
	}
	iCapacity = pFlat->iContourCapacity > 0 ? pFlat->iContourCapacity : 8;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pContours = (xge_shape_ex_flat_contour_t*)xrtRealloc(pFlat->pContours, (size_t)iCapacity * sizeof(*pContours));
	if ( pContours == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pFlat->pContours = pContours;
	pFlat->iContourCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeShapeExFlatBeginContour(xge_shape_ex_flat_path_t* pFlat, xge_vec2_t tPoint)
{
	xge_shape_ex_flat_contour_t* pContour;
	int iRet;

	iRet = __xgeShapeExFlatReserveContours(pFlat, pFlat->iContourCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeShapeExFlatReservePoints(pFlat, pFlat->iPointCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pContour = &pFlat->pContours[pFlat->iContourCount++];
	pContour->iStart = pFlat->iPointCount;
	pContour->iCount = 1;
	pContour->bClosed = 0;
	pFlat->pPoints[pFlat->iPointCount++] = tPoint;
	return XGE_OK;
}

static int __xgeShapeExFlatAddPoint(xge_shape_ex_flat_path_t* pFlat, xge_vec2_t tPoint)
{
	xge_shape_ex_flat_contour_t* pContour;
	xge_vec2_t tPrev;
	int iRet;

	if ( (pFlat == NULL) || (pFlat->iContourCount <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pContour = &pFlat->pContours[pFlat->iContourCount - 1];
	if ( pContour->iCount > 0 ) {
		tPrev = pFlat->pPoints[pContour->iStart + pContour->iCount - 1];
		if ( (fabsf(tPrev.fX - tPoint.fX) <= XGE_SHAPE_EX_EPSILON) &&
		     (fabsf(tPrev.fY - tPoint.fY) <= XGE_SHAPE_EX_EPSILON) ) {
			return XGE_OK;
		}
	}
	iRet = __xgeShapeExFlatReservePoints(pFlat, pFlat->iPointCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pFlat->pPoints[pFlat->iPointCount++] = tPoint;
	pContour->iCount++;
	return XGE_OK;
}

static void __xgeShapeExFlatCloseContour(xge_shape_ex_flat_path_t* pFlat)
{
	xge_shape_ex_flat_contour_t* pContour;

	if ( (pFlat == NULL) || (pFlat->iContourCount <= 0) ) {
		return;
	}
	pContour = &pFlat->pContours[pFlat->iContourCount - 1];
	pContour->bClosed = 1;
}

static void __xgeShapeExFlatFree(xge_shape_ex_flat_path_t* pFlat)
{
	if ( pFlat == NULL ) {
		return;
	}
	xrtFree(pFlat->pPoints);
	xrtFree(pFlat->pContours);
	memset(pFlat, 0, sizeof(*pFlat));
}

static int __xgeShapeExCubicStepCount(xge_vec2_t tP0, xge_vec2_t tP1, xge_vec2_t tP2, xge_vec2_t tP3, float fTolerance)
{
	float fLen;
	int iSteps;

	if ( fTolerance <= 0.0f ) {
		fTolerance = XGE_SHAPE_EX_DEFAULT_TOLERANCE;
	}
	fLen = hypotf(tP1.fX - tP0.fX, tP1.fY - tP0.fY) +
	       hypotf(tP2.fX - tP1.fX, tP2.fY - tP1.fY) +
	       hypotf(tP3.fX - tP2.fX, tP3.fY - tP2.fY);
	iSteps = (int)ceilf(fLen / (fTolerance * 18.0f));
	if ( iSteps < 4 ) iSteps = 4;
	if ( iSteps > 96 ) iSteps = 96;
	return iSteps;
}

static xge_vec2_t __xgeShapeExCubicPoint(xge_vec2_t tP0, xge_vec2_t tP1, xge_vec2_t tP2, xge_vec2_t tP3, float t)
{
	float u;
	xge_vec2_t tOut;

	u = 1.0f - t;
	tOut.fX = (u * u * u * tP0.fX) + (3.0f * u * u * t * tP1.fX) + (3.0f * u * t * t * tP2.fX) + (t * t * t * tP3.fX);
	tOut.fY = (u * u * u * tP0.fY) + (3.0f * u * u * t * tP1.fY) + (3.0f * u * t * t * tP2.fY) + (t * t * t * tP3.fY);
	return tOut;
}

static int __xgeShapeExFlatten(xge_shape_ex pShape, xge_shape_ex_matrix_t tMatrix, float fTolerance, xge_shape_ex_flat_path_t* pFlat)
{
	xge_vec2_t tCurrent;
	xge_vec2_t tStart;
	int bHasCurrent;
	int bHasStart;
	int iPointRead;
	int i;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (pFlat == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pFlat, 0, sizeof(*pFlat));
	memset(&tCurrent, 0, sizeof(tCurrent));
	memset(&tStart, 0, sizeof(tStart));
	bHasCurrent = 0;
	bHasStart = 0;
	iPointRead = 0;
	for ( i = 0; i < pShape->iCommandCount; i++ ) {
		uint8_t iCommand = pShape->pCommands[i];

		if ( iCommand == XGE_SHAPE_EX_CMD_MOVE_TO ) {
			tCurrent = __xgeShapeExMatrixPoint(tMatrix, pShape->pPoints[iPointRead++]);
			iRet = __xgeShapeExFlatBeginContour(pFlat, tCurrent);
			if ( iRet != XGE_OK ) {
				__xgeShapeExFlatFree(pFlat);
				return iRet;
			}
			tStart = tCurrent;
			bHasCurrent = 1;
			bHasStart = 1;
		} else if ( (iCommand == XGE_SHAPE_EX_CMD_LINE_TO) && bHasCurrent ) {
			tCurrent = __xgeShapeExMatrixPoint(tMatrix, pShape->pPoints[iPointRead++]);
			iRet = __xgeShapeExFlatAddPoint(pFlat, tCurrent);
			if ( iRet != XGE_OK ) {
				__xgeShapeExFlatFree(pFlat);
				return iRet;
			}
		} else if ( (iCommand == XGE_SHAPE_EX_CMD_CUBIC_TO) && bHasCurrent ) {
			xge_vec2_t tP0 = tCurrent;
			xge_vec2_t tP1 = __xgeShapeExMatrixPoint(tMatrix, pShape->pPoints[iPointRead++]);
			xge_vec2_t tP2 = __xgeShapeExMatrixPoint(tMatrix, pShape->pPoints[iPointRead++]);
			xge_vec2_t tP3 = __xgeShapeExMatrixPoint(tMatrix, pShape->pPoints[iPointRead++]);
			int iSteps = __xgeShapeExCubicStepCount(tP0, tP1, tP2, tP3, fTolerance);
			int j;

			for ( j = 1; j <= iSteps; j++ ) {
				xge_vec2_t tPoint = __xgeShapeExCubicPoint(tP0, tP1, tP2, tP3, (float)j / (float)iSteps);
				iRet = __xgeShapeExFlatAddPoint(pFlat, tPoint);
				if ( iRet != XGE_OK ) {
					__xgeShapeExFlatFree(pFlat);
					return iRet;
				}
			}
			tCurrent = tP3;
		} else if ( (iCommand == XGE_SHAPE_EX_CMD_CLOSE) && bHasCurrent && bHasStart ) {
			iRet = __xgeShapeExFlatAddPoint(pFlat, tStart);
			if ( iRet != XGE_OK ) {
				__xgeShapeExFlatFree(pFlat);
				return iRet;
			}
			__xgeShapeExFlatCloseContour(pFlat);
			tCurrent = tStart;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static float __xgeShapeExContourArea(const xge_vec2_t* pPoints, int iCount)
{
	float fArea;
	int i;

	fArea = 0.0f;
	for ( i = 0; i < iCount; i++ ) {
		const xge_vec2_t* pA = &pPoints[i];
		const xge_vec2_t* pB = &pPoints[(i + 1) % iCount];
		fArea += (pA->fX * pB->fY) - (pB->fX * pA->fY);
	}
	return fArea * 0.5f;
}

static float __xgeShapeExCross(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC)
{
	return ((tB.fX - tA.fX) * (tC.fY - tA.fY)) - ((tB.fY - tA.fY) * (tC.fX - tA.fX));
}

static int __xgeShapeExPointInTriangle(xge_vec2_t tP, xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC)
{
	float fC0 = __xgeShapeExCross(tA, tB, tP);
	float fC1 = __xgeShapeExCross(tB, tC, tP);
	float fC2 = __xgeShapeExCross(tC, tA, tP);

	return ((fC0 >= -XGE_SHAPE_EX_EPSILON) && (fC1 >= -XGE_SHAPE_EX_EPSILON) && (fC2 >= -XGE_SHAPE_EX_EPSILON)) ||
	       ((fC0 <= XGE_SHAPE_EX_EPSILON) && (fC1 <= XGE_SHAPE_EX_EPSILON) && (fC2 <= XGE_SHAPE_EX_EPSILON));
}

static int __xgeShapeExPointInTriangleStrict(xge_vec2_t tP, xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC)
{
	float fC0 = __xgeShapeExCross(tA, tB, tP);
	float fC1 = __xgeShapeExCross(tB, tC, tP);
	float fC2 = __xgeShapeExCross(tC, tA, tP);

	return ((fC0 > XGE_SHAPE_EX_EPSILON) && (fC1 > XGE_SHAPE_EX_EPSILON) && (fC2 > XGE_SHAPE_EX_EPSILON)) ||
	       ((fC0 < -XGE_SHAPE_EX_EPSILON) && (fC1 < -XGE_SHAPE_EX_EPSILON) && (fC2 < -XGE_SHAPE_EX_EPSILON));
}

static int __xgeShapeExPointInContour(xge_vec2_t tP, const xge_vec2_t* pPoints, int iCount)
{
	int bInside;
	int i;
	int j;

	if ( (pPoints == NULL) || (iCount < 3) ) {
		return 0;
	}
	bInside = 0;
	j = iCount - 1;
	for ( i = 0; i < iCount; i++ ) {
		float fYi = pPoints[i].fY;
		float fYj = pPoints[j].fY;

		if ( ((fYi > tP.fY) != (fYj > tP.fY)) ) {
			float fX = ((pPoints[j].fX - pPoints[i].fX) * (tP.fY - fYi) / (fYj - fYi)) + pPoints[i].fX;
			if ( tP.fX < fX ) {
				bInside = !bInside;
			}
		}
		j = i;
	}
	return bInside;
}

static float __xgeShapeExDistanceSq(xge_vec2_t tA, xge_vec2_t tB)
{
	float fDX = tA.fX - tB.fX;
	float fDY = tA.fY - tB.fY;
	return (fDX * fDX) + (fDY * fDY);
}

static int __xgeShapeExOrientation(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC)
{
	float fCross = __xgeShapeExCross(tA, tB, tC);

	if ( fCross > XGE_SHAPE_EX_EPSILON ) return 1;
	if ( fCross < -XGE_SHAPE_EX_EPSILON ) return -1;
	return 0;
}

static int __xgeShapeExOnSegment(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tP)
{
	return (tP.fX >= fminf(tA.fX, tB.fX) - XGE_SHAPE_EX_EPSILON) &&
	       (tP.fX <= fmaxf(tA.fX, tB.fX) + XGE_SHAPE_EX_EPSILON) &&
	       (tP.fY >= fminf(tA.fY, tB.fY) - XGE_SHAPE_EX_EPSILON) &&
	       (tP.fY <= fmaxf(tA.fY, tB.fY) + XGE_SHAPE_EX_EPSILON) &&
	       (fabsf(__xgeShapeExCross(tA, tB, tP)) <= XGE_SHAPE_EX_EPSILON);
}

static int __xgeShapeExSegmentsIntersect(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, xge_vec2_t tD)
{
	int iO1 = __xgeShapeExOrientation(tA, tB, tC);
	int iO2 = __xgeShapeExOrientation(tA, tB, tD);
	int iO3 = __xgeShapeExOrientation(tC, tD, tA);
	int iO4 = __xgeShapeExOrientation(tC, tD, tB);

	if ( (iO1 != iO2) && (iO3 != iO4) ) {
		return 1;
	}
	if ( (iO1 == 0) && __xgeShapeExOnSegment(tA, tB, tC) ) return 1;
	if ( (iO2 == 0) && __xgeShapeExOnSegment(tA, tB, tD) ) return 1;
	if ( (iO3 == 0) && __xgeShapeExOnSegment(tC, tD, tA) ) return 1;
	if ( (iO4 == 0) && __xgeShapeExOnSegment(tC, tD, tB) ) return 1;
	return 0;
}

static int __xgeShapeExSamePoint(xge_vec2_t tA, xge_vec2_t tB)
{
	return (fabsf(tA.fX - tB.fX) <= XGE_SHAPE_EX_EPSILON) &&
	       (fabsf(tA.fY - tB.fY) <= XGE_SHAPE_EX_EPSILON);
}

static int __xgeShapeExTriangulateContour(const xge_vec2_t* pPoints, int iCount, xge_shape_vertex_t** ppVertices, int* pVertexCount, uint32_t** ppIndices, int* pIndexCount, uint32_t iColor)
{
	xge_shape_vertex_t* pVertices;
	uint32_t* pIndices;
	int* pOrder;
	int iWorkCount;
	int iWrite;
	int iGuard;
	float fArea;
	int i;

	if ( (pPoints == NULL) || (iCount < 3) || (ppVertices == NULL) || (pVertexCount == NULL) || (ppIndices == NULL) || (pIndexCount == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	while ( (iCount > 2) &&
	        (fabsf(pPoints[iCount - 1].fX - pPoints[0].fX) <= XGE_SHAPE_EX_EPSILON) &&
	        (fabsf(pPoints[iCount - 1].fY - pPoints[0].fY) <= XGE_SHAPE_EX_EPSILON) ) {
		iCount--;
	}
	if ( iCount < 3 ) {
		*ppVertices = NULL;
		*pVertexCount = 0;
		*ppIndices = NULL;
		*pIndexCount = 0;
		return XGE_OK;
	}
	pVertices = (xge_shape_vertex_t*)xrtMalloc((size_t)iCount * sizeof(*pVertices));
	pIndices = (uint32_t*)xrtMalloc((size_t)(iCount - 2) * 3u * sizeof(*pIndices));
	pOrder = (int*)xrtMalloc((size_t)iCount * sizeof(*pOrder));
	if ( (pVertices == NULL) || (pIndices == NULL) || (pOrder == NULL) ) {
		xrtFree(pVertices);
		xrtFree(pIndices);
		xrtFree(pOrder);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < iCount; i++ ) {
		pVertices[i].fX = pPoints[i].fX;
		pVertices[i].fY = pPoints[i].fY;
		pVertices[i].iColor = iColor;
		pOrder[i] = i;
	}
	fArea = __xgeShapeExContourArea(pPoints, iCount);
	iWorkCount = iCount;
	iWrite = 0;
	iGuard = 0;
	while ( iWorkCount > 2 && iGuard < iCount * iCount ) {
		int bCut = 0;

		for ( i = 0; i < iWorkCount; i++ ) {
			int iPrev = pOrder[(i + iWorkCount - 1) % iWorkCount];
			int iCurr = pOrder[i];
			int iNext = pOrder[(i + 1) % iWorkCount];
			float fCross = __xgeShapeExCross(pPoints[iPrev], pPoints[iCurr], pPoints[iNext]);
			int bConvex = (fArea >= 0.0f) ? (fCross > XGE_SHAPE_EX_EPSILON) : (fCross < -XGE_SHAPE_EX_EPSILON);
			int bContains = 0;
			int j;

			if ( !bConvex ) {
				continue;
			}
			for ( j = 0; j < iWorkCount; j++ ) {
				int iTest = pOrder[j];
				if ( (iTest == iPrev) || (iTest == iCurr) || (iTest == iNext) ) {
					continue;
				}
				if ( __xgeShapeExPointInTriangleStrict(pPoints[iTest], pPoints[iPrev], pPoints[iCurr], pPoints[iNext]) ) {
					bContains = 1;
					break;
				}
			}
			if ( bContains ) {
				continue;
			}
			if ( fArea >= 0.0f ) {
				pIndices[iWrite++] = (uint32_t)iPrev;
				pIndices[iWrite++] = (uint32_t)iCurr;
				pIndices[iWrite++] = (uint32_t)iNext;
			} else {
				pIndices[iWrite++] = (uint32_t)iNext;
				pIndices[iWrite++] = (uint32_t)iCurr;
				pIndices[iWrite++] = (uint32_t)iPrev;
			}
			memmove(&pOrder[i], &pOrder[i + 1], (size_t)(iWorkCount - i - 1) * sizeof(*pOrder));
			iWorkCount--;
			bCut = 1;
			break;
		}
		if ( !bCut ) {
			break;
		}
		iGuard++;
	}
	xrtFree(pOrder);
	if ( iWrite <= 0 ) {
		xrtFree(pVertices);
		xrtFree(pIndices);
		*ppVertices = NULL;
		*pVertexCount = 0;
		*ppIndices = NULL;
		*pIndexCount = 0;
		return XGE_OK;
	}
	*ppVertices = pVertices;
	*pVertexCount = iCount;
	*ppIndices = pIndices;
	*pIndexCount = iWrite;
	return XGE_OK;
}

static int __xgeShapeExSubdivideTriangles(xge_shape_vertex_t** ppVertices, int* pVertexCount, uint32_t** ppIndices, int* pIndexCount)
{
	xge_shape_vertex_t* pVertices;
	xge_shape_vertex_t* pNewVertices;
	uint32_t* pIndices;
	uint32_t* pNewIndices;
	int iVertexCount;
	int iIndexCount;
	int iTriangleCount;
	int iNewVertexCount;
	int iNewIndexCount;
	int i;

	if ( (ppVertices == NULL) || (pVertexCount == NULL) || (ppIndices == NULL) || (pIndexCount == NULL) ||
	     (*ppVertices == NULL) || (*ppIndices == NULL) || (*pVertexCount <= 0) || (*pIndexCount <= 0) || ((*pIndexCount % 3) != 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVertices = *ppVertices;
	pIndices = *ppIndices;
	iVertexCount = *pVertexCount;
	iIndexCount = *pIndexCount;
	iTriangleCount = iIndexCount / 3;
	if ( (iTriangleCount <= 0) || (iVertexCount > (INT_MAX - iTriangleCount)) || (iTriangleCount > (INT_MAX / 9)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iNewVertexCount = iVertexCount + iTriangleCount;
	iNewIndexCount = iTriangleCount * 9;
	pNewVertices = (xge_shape_vertex_t*)xrtMalloc((size_t)iNewVertexCount * sizeof(*pNewVertices));
	pNewIndices = (uint32_t*)xrtMalloc((size_t)iNewIndexCount * sizeof(*pNewIndices));
	if ( (pNewVertices == NULL) || (pNewIndices == NULL) ) {
		xrtFree(pNewVertices);
		xrtFree(pNewIndices);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(pNewVertices, pVertices, (size_t)iVertexCount * sizeof(*pNewVertices));
	for ( i = 0; i < iTriangleCount; i++ ) {
		uint32_t iA = pIndices[i * 3 + 0];
		uint32_t iB = pIndices[i * 3 + 1];
		uint32_t iC = pIndices[i * 3 + 2];
		uint32_t iCenter = (uint32_t)(iVertexCount + i);
		int iOut = i * 9;

		if ( (iA >= (uint32_t)iVertexCount) || (iB >= (uint32_t)iVertexCount) || (iC >= (uint32_t)iVertexCount) ) {
			xrtFree(pNewVertices);
			xrtFree(pNewIndices);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		pNewVertices[iCenter].fX = (pVertices[iA].fX + pVertices[iB].fX + pVertices[iC].fX) / 3.0f;
		pNewVertices[iCenter].fY = (pVertices[iA].fY + pVertices[iB].fY + pVertices[iC].fY) / 3.0f;
		pNewVertices[iCenter].iColor = pVertices[iA].iColor;
		pNewIndices[iOut + 0] = iA;
		pNewIndices[iOut + 1] = iB;
		pNewIndices[iOut + 2] = iCenter;
		pNewIndices[iOut + 3] = iB;
		pNewIndices[iOut + 4] = iC;
		pNewIndices[iOut + 5] = iCenter;
		pNewIndices[iOut + 6] = iC;
		pNewIndices[iOut + 7] = iA;
		pNewIndices[iOut + 8] = iCenter;
	}
	xrtFree(pVertices);
	xrtFree(pIndices);
	*ppVertices = pNewVertices;
	*pVertexCount = iNewVertexCount;
	*ppIndices = pNewIndices;
	*pIndexCount = iNewIndexCount;
	return XGE_OK;
}

static xge_vec2_t __xgeShapeExRadialGradientCenterPoint(const xge_shape_ex pShape, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix)
{
	xge_vec2_t tCenter;

	if ( (pShape != NULL) && (pShape->iFillGradientUnits == XGE_SHAPE_EX_GRADIENT_USER_SPACE) ) {
		tCenter.fX = pShape->fFillX1;
		tCenter.fY = pShape->fFillY1;
		tCenter = __xgeShapeExMatrixPoint(pShape->tFillGradientTransform, tCenter);
		return __xgeShapeExMatrixPoint(tMatrix, tCenter);
	}
	tCenter.fX = tBounds.fX + ((pShape != NULL ? pShape->fFillX1 : 0.5f) * tBounds.fW);
	tCenter.fY = tBounds.fY + ((pShape != NULL ? pShape->fFillY1 : 0.5f) * tBounds.fH);
	if ( pShape != NULL ) {
		tCenter.fX = pShape->fFillX1;
		tCenter.fY = pShape->fFillY1;
		tCenter = __xgeShapeExMatrixPoint(pShape->tFillGradientTransform, tCenter);
		tCenter.fX = tBounds.fX + (tCenter.fX * tBounds.fW);
		tCenter.fY = tBounds.fY + (tCenter.fY * tBounds.fH);
	}
	return tCenter;
}

static int __xgeShapeExContourConvex(const xge_vec2_t* pPoints, int iCount)
{
	int iSign;
	int i;

	if ( (pPoints == NULL) || (iCount < 3) ) {
		return 0;
	}
	iSign = 0;
	for ( i = 0; i < iCount; i++ ) {
		xge_vec2_t tA = pPoints[i];
		xge_vec2_t tB = pPoints[(i + 1) % iCount];
		xge_vec2_t tC = pPoints[(i + 2) % iCount];
		float fCross = __xgeShapeExCross(tA, tB, tC);

		if ( fabsf(fCross) <= XGE_SHAPE_EX_EPSILON ) {
			continue;
		}
		if ( iSign == 0 ) {
			iSign = fCross > 0.0f ? 1 : -1;
		} else if ( ((fCross > 0.0f) ? 1 : -1) != iSign ) {
			return 0;
		}
	}
	return iSign != 0;
}

static int __xgeShapeExBuildCenterFan(const xge_vec2_t* pPoints, int iCount, xge_vec2_t tCenter, uint32_t iColor, xge_shape_vertex_t** ppVertices, int* pVertexCount, uint32_t** ppIndices, int* pIndexCount)
{
	xge_shape_vertex_t* pVertices;
	uint32_t* pIndices;
	float fArea;
	int i;

	if ( (pPoints == NULL) || (ppVertices == NULL) || (pVertexCount == NULL) || (ppIndices == NULL) || (pIndexCount == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	while ( (iCount > 2) && __xgeShapeExSamePoint(pPoints[iCount - 1], pPoints[0]) ) {
		iCount--;
	}
	if ( (iCount < 3) || !__xgeShapeExContourConvex(pPoints, iCount) || !__xgeShapeExPointInContour(tCenter, pPoints, iCount) ) {
		*ppVertices = NULL;
		*pVertexCount = 0;
		*ppIndices = NULL;
		*pIndexCount = 0;
		return XGE_OK;
	}
	pVertices = (xge_shape_vertex_t*)xrtMalloc((size_t)(iCount + 1) * sizeof(*pVertices));
	pIndices = (uint32_t*)xrtMalloc((size_t)iCount * 3u * sizeof(*pIndices));
	if ( (pVertices == NULL) || (pIndices == NULL) ) {
		xrtFree(pVertices);
		xrtFree(pIndices);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pVertices[0].fX = tCenter.fX;
	pVertices[0].fY = tCenter.fY;
	pVertices[0].iColor = iColor;
	for ( i = 0; i < iCount; i++ ) {
		pVertices[i + 1].fX = pPoints[i].fX;
		pVertices[i + 1].fY = pPoints[i].fY;
		pVertices[i + 1].iColor = iColor;
	}
	fArea = __xgeShapeExContourArea(pPoints, iCount);
	for ( i = 0; i < iCount; i++ ) {
		uint32_t iA = (uint32_t)(i + 1);
		uint32_t iB = (uint32_t)(((i + 1) % iCount) + 1);

		pIndices[i * 3 + 0] = 0;
		if ( fArea >= 0.0f ) {
			pIndices[i * 3 + 1] = iA;
			pIndices[i * 3 + 2] = iB;
		} else {
			pIndices[i * 3 + 1] = iB;
			pIndices[i * 3 + 2] = iA;
		}
	}
	*ppVertices = pVertices;
	*pVertexCount = iCount + 1;
	*ppIndices = pIndices;
	*pIndexCount = iCount * 3;
	return XGE_OK;
}

static xge_rect_t __xgeShapeExFlatBounds(const xge_shape_ex_flat_path_t* pFlat)
{
	xge_rect_t tBounds;
	int i;

	memset(&tBounds, 0, sizeof(tBounds));
	if ( (pFlat == NULL) || (pFlat->iPointCount <= 0) ) {
		return tBounds;
	}
	tBounds.fX = pFlat->pPoints[0].fX;
	tBounds.fY = pFlat->pPoints[0].fY;
	tBounds.fW = pFlat->pPoints[0].fX;
	tBounds.fH = pFlat->pPoints[0].fY;
	for ( i = 1; i < pFlat->iPointCount; i++ ) {
		if ( pFlat->pPoints[i].fX < tBounds.fX ) tBounds.fX = pFlat->pPoints[i].fX;
		if ( pFlat->pPoints[i].fY < tBounds.fY ) tBounds.fY = pFlat->pPoints[i].fY;
		if ( pFlat->pPoints[i].fX > tBounds.fW ) tBounds.fW = pFlat->pPoints[i].fX;
		if ( pFlat->pPoints[i].fY > tBounds.fH ) tBounds.fH = pFlat->pPoints[i].fY;
	}
	tBounds.fW -= tBounds.fX;
	tBounds.fH -= tBounds.fY;
	if ( tBounds.fW < 0.0f ) tBounds.fW = 0.0f;
	if ( tBounds.fH < 0.0f ) tBounds.fH = 0.0f;
	return tBounds;
}

static void __xgeShapeExApplyFillPaint(xge_shape_ex pShape, xge_shape_vertex_t* pVertices, int iVertexCount, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, uint32_t iFallbackColor, float fOpacity)
{
	int i;

	if ( (pShape == NULL) || (pVertices == NULL) || (iVertexCount <= 0) ) {
		return;
	}
	if ( (pShape->iFillType != XGE_SHAPE_EX_FILL_LINEAR_GRADIENT) && (pShape->iFillType != XGE_SHAPE_EX_FILL_RADIAL_GRADIENT) ) {
		for ( i = 0; i < iVertexCount; i++ ) {
			pVertices[i].iColor = iFallbackColor;
		}
		return;
	}
	for ( i = 0; i < iVertexCount; i++ ) {
		xge_vec2_t tPoint;

		tPoint.fX = pVertices[i].fX;
		tPoint.fY = pVertices[i].fY;
		if ( pShape->iFillType == XGE_SHAPE_EX_FILL_RADIAL_GRADIENT ) {
			pVertices[i].iColor = __xgeShapeExColorOpacity(__xgeShapeExRadialGradientColor(pShape, tPoint, tBounds, tMatrix), fOpacity);
		} else {
			pVertices[i].iColor = __xgeShapeExColorOpacity(__xgeShapeExLinearGradientColor(pShape, tPoint, tBounds, tMatrix), fOpacity);
		}
	}
}

static void __xgeShapeExApplyStrokePaint(xge_shape_ex pShape, xge_shape_vertex_t* pVertices, int iVertexCount, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, uint32_t iFallbackColor, float fOpacity)
{
	int i;

	if ( (pShape == NULL) || (pVertices == NULL) || (iVertexCount <= 0) ) {
		return;
	}
	if ( (pShape->iStrokeType != XGE_SHAPE_EX_FILL_LINEAR_GRADIENT) && (pShape->iStrokeType != XGE_SHAPE_EX_FILL_RADIAL_GRADIENT) ) {
		for ( i = 0; i < iVertexCount; i++ ) {
			pVertices[i].iColor = iFallbackColor;
		}
		return;
	}
	for ( i = 0; i < iVertexCount; i++ ) {
		xge_vec2_t tPoint;
		uint32_t iColor;

		tPoint.fX = pVertices[i].fX;
		tPoint.fY = pVertices[i].fY;
		if ( pShape->iStrokeType == XGE_SHAPE_EX_FILL_RADIAL_GRADIENT ) {
			iColor = __xgeShapeExRadialGradientColorEx(pShape->fStrokeX1, pShape->fStrokeY1, pShape->fStrokeR, pShape->fStrokeFX, pShape->fStrokeFY,
				pShape->iStrokeGradientUnits, pShape->tStrokeGradientTransform, pShape->pStrokeStops, pShape->iStrokeStopCount,
				pShape->iStrokeColor, pShape->iStrokeGradientSpread, tPoint, tBounds, tMatrix);
		} else {
			iColor = __xgeShapeExLinearGradientColorEx(pShape->fStrokeX1, pShape->fStrokeY1, pShape->fStrokeX2, pShape->fStrokeY2,
				pShape->iStrokeGradientUnits, pShape->tStrokeGradientTransform, pShape->pStrokeStops, pShape->iStrokeStopCount,
				pShape->iStrokeColor, pShape->iStrokeGradientSpread, tPoint, tBounds, tMatrix);
		}
		pVertices[i].iColor = __xgeShapeExColorOpacity(iColor, fOpacity);
	}
}

static xge_vec2_t* __xgeShapeExContourCopyOriented(const xge_shape_ex_flat_path_t* pFlat, int iContour, int bWantPositive, int* pCount)
{
	const xge_shape_ex_flat_contour_t* pContour;
	const xge_vec2_t* pSrc;
	xge_vec2_t* pOut;
	float fArea;
	int iCount;
	int i;

	if ( (pFlat == NULL) || (iContour < 0) || (iContour >= pFlat->iContourCount) || (pCount == NULL) ) {
		return NULL;
	}
	pContour = &pFlat->pContours[iContour];
	pSrc = pFlat->pPoints + pContour->iStart;
	iCount = pContour->iCount;
	while ( (iCount > 2) && __xgeShapeExSamePoint(pSrc[iCount - 1], pSrc[0]) ) {
		iCount--;
	}
	if ( iCount < 3 ) {
		return NULL;
	}
	pOut = (xge_vec2_t*)xrtMalloc((size_t)iCount * sizeof(*pOut));
	if ( pOut == NULL ) {
		return NULL;
	}
	fArea = __xgeShapeExContourArea(pSrc, iCount);
	if ( ((fArea >= 0.0f) && bWantPositive) || ((fArea < 0.0f) && !bWantPositive) ) {
		memcpy(pOut, pSrc, (size_t)iCount * sizeof(*pOut));
	} else {
		for ( i = 0; i < iCount; i++ ) {
			pOut[i] = pSrc[iCount - 1 - i];
		}
	}
	*pCount = iCount;
	return pOut;
}

static int __xgeShapeExBridgeVisible(const xge_vec2_t* pPoly, int iPolyCount, int iPolyVertex, const xge_vec2_t* pHole, int iHoleCount, int iHoleVertex)
{
	xge_vec2_t tA;
	xge_vec2_t tB;
	int i;

	if ( (pPoly == NULL) || (pHole == NULL) || (iPolyCount < 3) || (iHoleCount < 3) ) {
		return 0;
	}
	tA = pPoly[iPolyVertex];
	tB = pHole[iHoleVertex];
	for ( i = 0; i < iPolyCount; i++ ) {
		int iNext = (i + 1) % iPolyCount;

		if ( (i == iPolyVertex) || (iNext == iPolyVertex) ) {
			continue;
		}
		if ( __xgeShapeExSegmentsIntersect(tA, tB, pPoly[i], pPoly[iNext]) ) {
			return 0;
		}
	}
	for ( i = 0; i < iHoleCount; i++ ) {
		int iNext = (i + 1) % iHoleCount;

		if ( (i == iHoleVertex) || (iNext == iHoleVertex) ) {
			continue;
		}
		if ( __xgeShapeExSegmentsIntersect(tA, tB, pHole[i], pHole[iNext]) ) {
			return 0;
		}
	}
	return 1;
}

static int __xgeShapeExBridgeOneHole(xge_vec2_t** ppPoly, int* pPolyCount, const xge_shape_ex_flat_path_t* pFlat, int iHoleContour)
{
	xge_vec2_t* pPoly;
	xge_vec2_t* pHole;
	xge_vec2_t* pNew;
	float fBestDistance;
	float fBestRayX;
	xge_vec2_t tBridgePoint;
	int iPolyCount;
	int iHoleCount;
	int iHoleVertex;
	int iBridgeVertex;
	int iBridgeEdge;
	int iWrite;
	int i;

	if ( (ppPoly == NULL) || (*ppPoly == NULL) || (pPolyCount == NULL) || (pFlat == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pPoly = *ppPoly;
	iPolyCount = *pPolyCount;
	pHole = __xgeShapeExContourCopyOriented(pFlat, iHoleContour, 0, &iHoleCount);
	if ( pHole == NULL ) {
		return XGE_OK;
	}
	iHoleVertex = 0;
	for ( i = 1; i < iHoleCount; i++ ) {
		if ( (pHole[i].fX > pHole[iHoleVertex].fX) ||
		     ((fabsf(pHole[i].fX - pHole[iHoleVertex].fX) <= XGE_SHAPE_EX_EPSILON) && (pHole[i].fY < pHole[iHoleVertex].fY)) ) {
			iHoleVertex = i;
		}
	}
	iBridgeVertex = -1;
	iBridgeEdge = -1;
	fBestDistance = FLT_MAX;
	fBestRayX = FLT_MAX;
	tBridgePoint = pHole[iHoleVertex];
	for ( i = 0; i < iPolyCount; i++ ) {
		int iNext = (i + 1) % iPolyCount;
		xge_vec2_t tA = pPoly[i];
		xge_vec2_t tB = pPoly[iNext];
		float fY = pHole[iHoleVertex].fY;
		float fX;

		if ( fabsf(tA.fY - tB.fY) <= XGE_SHAPE_EX_EPSILON ) {
			continue;
		}
		if ( ((tA.fY > fY) == (tB.fY > fY)) ) {
			continue;
		}
		fX = tA.fX + ((fY - tA.fY) * (tB.fX - tA.fX) / (tB.fY - tA.fY));
		if ( fX <= pHole[iHoleVertex].fX + XGE_SHAPE_EX_EPSILON ) {
			continue;
		}
		if ( fX < fBestRayX ) {
			fBestRayX = fX;
			iBridgeEdge = i;
			tBridgePoint.fX = fX;
			tBridgePoint.fY = fY;
		}
	}
	if ( iBridgeEdge >= 0 ) {
		pNew = (xge_vec2_t*)xrtMalloc((size_t)(iPolyCount + iHoleCount + 3) * sizeof(*pNew));
		if ( pNew == NULL ) {
			xrtFree(pHole);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iWrite = 0;
		for ( i = 0; i <= iBridgeEdge; i++ ) {
			pNew[iWrite++] = pPoly[i];
		}
		pNew[iWrite++] = tBridgePoint;
		for ( i = 0; i < iHoleCount; i++ ) {
			pNew[iWrite++] = pHole[(iHoleVertex + i) % iHoleCount];
		}
		pNew[iWrite++] = pHole[iHoleVertex];
		pNew[iWrite++] = tBridgePoint;
		for ( i = iBridgeEdge + 1; i < iPolyCount; i++ ) {
			pNew[iWrite++] = pPoly[i];
		}
		xrtFree(pHole);
		xrtFree(pPoly);
		*ppPoly = pNew;
		*pPolyCount = iWrite;
		return XGE_OK;
	}
	for ( i = 0; i < iPolyCount; i++ ) {
		float fDistance;

		if ( pPoly[i].fX < (pHole[iHoleVertex].fX - XGE_SHAPE_EX_EPSILON) ) {
			continue;
		}
		if ( !__xgeShapeExBridgeVisible(pPoly, iPolyCount, i, pHole, iHoleCount, iHoleVertex) ) {
			continue;
		}
		fDistance = __xgeShapeExDistanceSq(pPoly[i], pHole[iHoleVertex]);
		if ( fDistance < fBestDistance ) {
			fBestDistance = fDistance;
			iBridgeVertex = i;
		}
	}
	if ( iBridgeVertex < 0 ) {
		for ( i = 0; i < iPolyCount; i++ ) {
			float fDistance;

			if ( !__xgeShapeExBridgeVisible(pPoly, iPolyCount, i, pHole, iHoleCount, iHoleVertex) ) {
				continue;
			}
			fDistance = __xgeShapeExDistanceSq(pPoly[i], pHole[iHoleVertex]);
			if ( fDistance < fBestDistance ) {
				fBestDistance = fDistance;
				iBridgeVertex = i;
			}
		}
	}
	if ( iBridgeVertex < 0 ) {
		xrtFree(pHole);
		return XGE_OK;
	}
	pNew = (xge_vec2_t*)xrtMalloc((size_t)(iPolyCount + iHoleCount + 2) * sizeof(*pNew));
	if ( pNew == NULL ) {
		xrtFree(pHole);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iWrite = 0;
	for ( i = 0; i <= iBridgeVertex; i++ ) {
		pNew[iWrite++] = pPoly[i];
	}
	for ( i = 0; i < iHoleCount; i++ ) {
		pNew[iWrite++] = pHole[(iHoleVertex + i) % iHoleCount];
	}
	pNew[iWrite++] = pHole[iHoleVertex];
	pNew[iWrite++] = pPoly[iBridgeVertex];
	for ( i = iBridgeVertex + 1; i < iPolyCount; i++ ) {
		pNew[iWrite++] = pPoly[i];
	}
	xrtFree(pHole);
	xrtFree(pPoly);
	*ppPoly = pNew;
	*pPolyCount = iWrite;
	return XGE_OK;
}

static int __xgeShapeExDrawFillMesh(xge_shape_ex pShape, const xge_vec2_t* pPoints, int iCount, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, float fOpacity, uint32_t iColor, int bScreenSpace)
{
	xge_shape_vertex_t* pVertices;
	uint32_t* pIndices;
	int iVertexCount;
	int iIndexCount;
	int iRet;
	int iPass;
	int bUsedCenterFan;

	pVertices = NULL;
	pIndices = NULL;
	iVertexCount = 0;
	iIndexCount = 0;
	bUsedCenterFan = 0;
	if ( (pShape != NULL) && (pShape->iFillType == XGE_SHAPE_EX_FILL_RADIAL_GRADIENT) ) {
		xge_vec2_t tCenter = __xgeShapeExRadialGradientCenterPoint(pShape, tBounds, tMatrix);

		iRet = __xgeShapeExBuildCenterFan(pPoints, iCount, tCenter, iColor, &pVertices, &iVertexCount, &pIndices, &iIndexCount);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		bUsedCenterFan = pVertices != NULL;
	}
	if ( pVertices == NULL ) {
		iRet = __xgeShapeExTriangulateContour(pPoints, iCount, &pVertices, &iVertexCount, &pIndices, &iIndexCount, iColor);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	if ( (pShape != NULL) && (pShape->iFillType == XGE_SHAPE_EX_FILL_RADIAL_GRADIENT) && (iIndexCount > 0) ) {
		int iPassCount = bUsedCenterFan ? 3 : 2;

		for ( iPass = 0; iPass < iPassCount; iPass++ ) {
			iRet = __xgeShapeExSubdivideTriangles(&pVertices, &iVertexCount, &pIndices, &iIndexCount);
			if ( iRet != XGE_OK ) {
				xrtFree(pVertices);
				xrtFree(pIndices);
				return iRet;
			}
		}
	}
	__xgeShapeExApplyFillPaint(pShape, pVertices, iVertexCount, tBounds, tMatrix, iColor, fOpacity);
	if ( iIndexCount > 0 ) {
		iRet = bScreenSpace ? xgeShapeMeshFillPx(pVertices, iVertexCount, pIndices, iIndexCount) : xgeShapeMeshFill(pVertices, iVertexCount, pIndices, iIndexCount);
	} else {
		iRet = XGE_OK;
	}
	xrtFree(pVertices);
	xrtFree(pIndices);
	return iRet;
}

static int __xgeShapeExDrawCompoundFill(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, int iOuter, const int* pParent, const int* pDepth, const float* pAreas, int iFillRule, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, float fOpacity, uint32_t iColor, int bScreenSpace)
{
	xge_vec2_t* pPoly;
	int iPolyCount;
	int i;
	int iRet;

	pPoly = __xgeShapeExContourCopyOriented(pFlat, iOuter, 1, &iPolyCount);
	if ( pPoly == NULL ) {
		return XGE_OK;
	}
	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		int bHole;

		if ( (pParent[i] != iOuter) || (pDepth[i] != (pDepth[iOuter] + 1)) ) {
			continue;
		}
		bHole = (iFillRule == XGE_SHAPE_EX_FILL_EVEN_ODD) ? 1 : ((pAreas[i] * pAreas[iOuter]) < 0.0f);
		if ( !bHole ) {
			continue;
		}
		iRet = __xgeShapeExBridgeOneHole(&pPoly, &iPolyCount, pFlat, i);
		if ( iRet != XGE_OK ) {
			xrtFree(pPoly);
			return iRet;
		}
	}
	iRet = __xgeShapeExDrawFillMesh(pShape, pPoly, iPolyCount, tBounds, tMatrix, fOpacity, iColor, bScreenSpace);
	xrtFree(pPoly);
	return iRet;
}

static int __xgeShapeExDrawFill(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, int iFillRule, xge_shape_ex_matrix_t tMatrix, float fOpacity, uint32_t iColor, int bScreenSpace)
{
	int i;
	int iRet;
	int* pParent;
	int* pDepth;
	float* pAreas;
	int bHasNested;
	xge_rect_t tBounds;

	if ( (pFlat == NULL) || !__xgeShapeExFillVisible(pShape, iColor) ) {
		return XGE_OK;
	}
	tBounds = __xgeShapeExFlatBounds(pFlat);
	pParent = (int*)xrtMalloc((size_t)pFlat->iContourCount * sizeof(*pParent));
	pDepth = (int*)xrtMalloc((size_t)pFlat->iContourCount * sizeof(*pDepth));
	pAreas = (float*)xrtMalloc((size_t)pFlat->iContourCount * sizeof(*pAreas));
	if ( (pParent == NULL) || (pDepth == NULL) || (pAreas == NULL) ) {
		xrtFree(pParent);
		xrtFree(pDepth);
		xrtFree(pAreas);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	bHasNested = 0;
	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		const xge_shape_ex_flat_contour_t* pContour = &pFlat->pContours[i];
		const xge_vec2_t* pPoints = pFlat->pPoints + pContour->iStart;
		float fBestArea = FLT_MAX;
		int j;

		pParent[i] = -1;
		pDepth[i] = 0;
		pAreas[i] = 0.0f;
		if ( pContour->iCount < 3 ) {
			continue;
		}
		pAreas[i] = __xgeShapeExContourArea(pPoints, pContour->iCount);
		for ( j = 0; j < pFlat->iContourCount; j++ ) {
			const xge_shape_ex_flat_contour_t* pOther = &pFlat->pContours[j];
			const xge_vec2_t* pOtherPoints = pFlat->pPoints + pOther->iStart;
			float fOtherArea;

			if ( (i == j) || (pOther->iCount < 3) ) {
				continue;
			}
			fOtherArea = fabsf(__xgeShapeExContourArea(pOtherPoints, pOther->iCount));
			if ( fOtherArea <= fabsf(pAreas[i]) + XGE_SHAPE_EX_EPSILON ) {
				continue;
			}
			if ( !__xgeShapeExPointInContour(pPoints[0], pOtherPoints, pOther->iCount) ) {
				continue;
			}
			if ( fOtherArea < fBestArea ) {
				fBestArea = fOtherArea;
				pParent[i] = j;
			}
		}
	}
	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		int iParent = pParent[i];
		while ( iParent >= 0 ) {
			pDepth[i]++;
			iParent = pParent[iParent];
		}
		if ( pDepth[i] > 0 ) {
			bHasNested = 1;
		}
	}
	if ( !bHasNested ) {
		for ( i = 0; i < pFlat->iContourCount; i++ ) {
			const xge_shape_ex_flat_contour_t* pContour = &pFlat->pContours[i];

			if ( pContour->iCount < 3 ) {
				continue;
			}
			iRet = __xgeShapeExDrawFillMesh(pShape, pFlat->pPoints + pContour->iStart, pContour->iCount, tBounds, tMatrix, fOpacity, iColor, bScreenSpace);
			if ( iRet != XGE_OK ) {
				xrtFree(pParent);
				xrtFree(pDepth);
				xrtFree(pAreas);
				return iRet;
			}
		}
		xrtFree(pParent);
		xrtFree(pDepth);
		xrtFree(pAreas);
		return XGE_OK;
	}
	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		const xge_shape_ex_flat_contour_t* pContour = &pFlat->pContours[i];

		if ( (pContour->iCount < 3) || ((pDepth[i] % 2) != 0) ) {
			continue;
		}
		iRet = __xgeShapeExDrawCompoundFill(pShape, pFlat, i, pParent, pDepth, pAreas, iFillRule, tBounds, tMatrix, fOpacity, iColor, bScreenSpace);
		if ( iRet != XGE_OK ) {
			xrtFree(pParent);
			xrtFree(pDepth);
			xrtFree(pAreas);
			return iRet;
		}
	}
	xrtFree(pParent);
	xrtFree(pDepth);
	xrtFree(pAreas);
	return XGE_OK;
}

static int __xgeShapeExDrawStrokePaintMesh(xge_shape_ex pShape, xge_shape_vertex_t* pVertices, int iVertexCount, const uint32_t* pIndices, int iIndexCount, uint32_t iColor, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, float fOpacity, int bScreenSpace)
{
	if ( (pVertices == NULL) || (iVertexCount <= 0) || (pIndices == NULL) || (iIndexCount <= 0) ) {
		return XGE_OK;
	}
	__xgeShapeExApplyStrokePaint(pShape, pVertices, iVertexCount, tBounds, tMatrix, iColor, fOpacity);
	return bScreenSpace ? xgeShapeMeshFillPx(pVertices, iVertexCount, pIndices, iIndexCount) : xgeShapeMeshFill(pVertices, iVertexCount, pIndices, iIndexCount);
}

static int __xgeShapeExDrawStrokeRoundCap(xge_shape_ex pShape, xge_vec2_t tCenter, float fDirX, float fDirY, float fHalfWidth, int bEndCap, uint32_t iColor, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, float fOpacity, int bScreenSpace)
{
	xge_shape_vertex_t tVertices[18];
	uint32_t iIndices[48];
	float fBase;
	float fStart;
	int iSegments;
	int i;

	if ( (pShape == NULL) || (pShape->iStrokeType == XGE_SHAPE_EX_FILL_SOLID) || (fHalfWidth <= XGE_SHAPE_EX_EPSILON) ) {
		return XGE_OK;
	}
	iSegments = 12;
	fBase = atan2f(fDirY, fDirX);
	fStart = bEndCap ? (fBase - XGE_SHAPE_EX_PI * 0.5f) : (fBase + XGE_SHAPE_EX_PI * 0.5f);
	tVertices[0].fX = tCenter.fX;
	tVertices[0].fY = tCenter.fY;
	tVertices[0].iColor = iColor;
	for ( i = 0; i <= iSegments; i++ ) {
		float fA = fStart + ((float)i / (float)iSegments) * XGE_SHAPE_EX_PI;
		tVertices[i + 1].fX = tCenter.fX + cosf(fA) * fHalfWidth;
		tVertices[i + 1].fY = tCenter.fY + sinf(fA) * fHalfWidth;
		tVertices[i + 1].iColor = iColor;
	}
	for ( i = 0; i < iSegments; i++ ) {
		iIndices[i * 3 + 0] = 0;
		iIndices[i * 3 + 1] = (uint32_t)(i + 1);
		iIndices[i * 3 + 2] = (uint32_t)(i + 2);
	}
	return __xgeShapeExDrawStrokePaintMesh(pShape, tVertices, iSegments + 2, iIndices, iSegments * 3, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
}

static int __xgeShapeExLineIntersection(xge_vec2_t tP, xge_vec2_t tR, xge_vec2_t tQ, xge_vec2_t tS, xge_vec2_t* pOut)
{
	float fDen;
	float fT;

	if ( pOut == NULL ) {
		return 0;
	}
	fDen = tR.fX * tS.fY - tR.fY * tS.fX;
	if ( fabsf(fDen) <= XGE_SHAPE_EX_EPSILON ) {
		return 0;
	}
	fT = ((tQ.fX - tP.fX) * tS.fY - (tQ.fY - tP.fY) * tS.fX) / fDen;
	pOut->fX = tP.fX + tR.fX * fT;
	pOut->fY = tP.fY + tR.fY * fT;
	return 1;
}

static int __xgeShapeExDrawStrokeJoin(xge_shape_ex pShape, xge_vec2_t tPrev, xge_vec2_t tCurr, xge_vec2_t tNext, float fWidth, uint32_t iColor, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, float fOpacity, int bScreenSpace)
{
	xge_shape_vertex_t tVertices[34];
	uint32_t iIndices[96];
	xge_vec2_t tD1;
	xge_vec2_t tD2;
	xge_vec2_t tN1;
	xge_vec2_t tN2;
	xge_vec2_t tOuter1;
	xge_vec2_t tOuter2;
	float fL1;
	float fL2;
	float fHalfWidth;
	float fCross;
	int iVertexCount;

	if ( (pShape == NULL) || (pShape->iStrokeType == XGE_SHAPE_EX_FILL_SOLID) || (fWidth <= XGE_SHAPE_EX_EPSILON) ) {
		return XGE_OK;
	}
	tD1.fX = tCurr.fX - tPrev.fX;
	tD1.fY = tCurr.fY - tPrev.fY;
	tD2.fX = tNext.fX - tCurr.fX;
	tD2.fY = tNext.fY - tCurr.fY;
	fL1 = hypotf(tD1.fX, tD1.fY);
	fL2 = hypotf(tD2.fX, tD2.fY);
	if ( (fL1 <= XGE_SHAPE_EX_EPSILON) || (fL2 <= XGE_SHAPE_EX_EPSILON) ) {
		return XGE_OK;
	}
	tD1.fX /= fL1; tD1.fY /= fL1;
	tD2.fX /= fL2; tD2.fY /= fL2;
	fCross = tD1.fX * tD2.fY - tD1.fY * tD2.fX;
	if ( fabsf(fCross) <= XGE_SHAPE_EX_EPSILON ) {
		return XGE_OK;
	}
	fHalfWidth = fWidth * 0.5f;
	tN1.fX = -tD1.fY * fHalfWidth;
	tN1.fY = tD1.fX * fHalfWidth;
	tN2.fX = -tD2.fY * fHalfWidth;
	tN2.fY = tD2.fX * fHalfWidth;
	if ( fCross > 0.0f ) {
		tOuter1 = tN1;
		tOuter2 = tN2;
	} else {
		tOuter1.fX = -tN1.fX;
		tOuter1.fY = -tN1.fY;
		tOuter2.fX = -tN2.fX;
		tOuter2.fY = -tN2.fY;
	}
	if ( pShape->iLineJoin == XGE_SHAPE_EX_JOIN_ROUND ) {
		float fA0 = atan2f(tOuter1.fY, tOuter1.fX);
		float fA1 = atan2f(tOuter2.fY, tOuter2.fX);
		float fDelta = fA1 - fA0;
		int iSegments;
		int i;

		while ( fDelta <= -XGE_SHAPE_EX_PI ) {
			fDelta += XGE_SHAPE_EX_TAU;
		}
		while ( fDelta > XGE_SHAPE_EX_PI ) {
			fDelta -= XGE_SHAPE_EX_TAU;
		}
		iSegments = (int)ceilf(fabsf(fDelta) / (XGE_SHAPE_EX_PI / 12.0f));
		if ( iSegments < 1 ) {
			iSegments = 1;
		}
		if ( iSegments > 32 ) {
			iSegments = 32;
		}
		tVertices[0].fX = tCurr.fX;
		tVertices[0].fY = tCurr.fY;
		tVertices[0].iColor = iColor;
		for ( i = 0; i <= iSegments; i++ ) {
			float fA = fA0 + fDelta * ((float)i / (float)iSegments);
			tVertices[i + 1].fX = tCurr.fX + cosf(fA) * fHalfWidth;
			tVertices[i + 1].fY = tCurr.fY + sinf(fA) * fHalfWidth;
			tVertices[i + 1].iColor = iColor;
		}
		for ( i = 0; i < iSegments; i++ ) {
			iIndices[i * 3 + 0] = 0;
			iIndices[i * 3 + 1] = (uint32_t)(i + 1);
			iIndices[i * 3 + 2] = (uint32_t)(i + 2);
		}
		return __xgeShapeExDrawStrokePaintMesh(pShape, tVertices, iSegments + 2, iIndices, iSegments * 3, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
	}
	iVertexCount = 0;
	if ( pShape->iLineJoin == XGE_SHAPE_EX_JOIN_MITER ) {
		xge_vec2_t tMiter;
		xge_vec2_t tP1;
		xge_vec2_t tP2;

		tP1.fX = tCurr.fX + tOuter1.fX; tP1.fY = tCurr.fY + tOuter1.fY;
		tP2.fX = tCurr.fX + tOuter2.fX; tP2.fY = tCurr.fY + tOuter2.fY;
		if ( __xgeShapeExLineIntersection(tP1, tD1, tP2, tD2, &tMiter) &&
		     (hypotf(tMiter.fX - tCurr.fX, tMiter.fY - tCurr.fY) <= fHalfWidth * pShape->fMiterLimit) ) {
			tVertices[iVertexCount].fX = tCurr.fX + tOuter1.fX; tVertices[iVertexCount].fY = tCurr.fY + tOuter1.fY; tVertices[iVertexCount++].iColor = iColor;
			tVertices[iVertexCount].fX = tMiter.fX; tVertices[iVertexCount].fY = tMiter.fY; tVertices[iVertexCount++].iColor = iColor;
			tVertices[iVertexCount].fX = tCurr.fX + tOuter2.fX; tVertices[iVertexCount].fY = tCurr.fY + tOuter2.fY; tVertices[iVertexCount++].iColor = iColor;
			iIndices[0] = 0; iIndices[1] = 1; iIndices[2] = 2;
			return __xgeShapeExDrawStrokePaintMesh(pShape, tVertices, iVertexCount, iIndices, 3, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
		}
	}
	tVertices[iVertexCount].fX = tCurr.fX; tVertices[iVertexCount].fY = tCurr.fY; tVertices[iVertexCount++].iColor = iColor;
	tVertices[iVertexCount].fX = tCurr.fX + tOuter1.fX; tVertices[iVertexCount].fY = tCurr.fY + tOuter1.fY; tVertices[iVertexCount++].iColor = iColor;
	tVertices[iVertexCount].fX = tCurr.fX + tOuter2.fX; tVertices[iVertexCount].fY = tCurr.fY + tOuter2.fY; tVertices[iVertexCount++].iColor = iColor;
	iIndices[0] = 0; iIndices[1] = 1; iIndices[2] = 2;
	return __xgeShapeExDrawStrokePaintMesh(pShape, tVertices, iVertexCount, iIndices, 3, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
}

static int __xgeShapeExDrawStrokeSegment(xge_shape_ex pShape, xge_vec2_t tA, xge_vec2_t tB, float fWidth, uint32_t iColor, int bCapStart, int bCapEnd, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, float fOpacity, int bScreenSpace)
{
	if ( (pShape != NULL) && (pShape->iStrokeType != XGE_SHAPE_EX_FILL_SOLID) ) {
		xge_shape_vertex_t tVertices[4];
		uint32_t iIndices[6];
		float fDX = tB.fX - tA.fX;
		float fDY = tB.fY - tA.fY;
		float fLen = hypotf(fDX, fDY);
		float fNX;
		float fNY;
		float fDirX;
		float fDirY;
		float fHalfWidth;

		if ( fLen <= XGE_SHAPE_EX_EPSILON ) {
			return XGE_OK;
		}
		fDirX = fDX / fLen;
		fDirY = fDY / fLen;
		fHalfWidth = fWidth * 0.5f;
		if ( pShape->iLineCap == XGE_SHAPE_EX_CAP_SQUARE ) {
			if ( bCapStart ) {
				tA.fX -= fDirX * fHalfWidth;
				tA.fY -= fDirY * fHalfWidth;
			}
			if ( bCapEnd ) {
				tB.fX += fDirX * fHalfWidth;
				tB.fY += fDirY * fHalfWidth;
			}
		}
		fNX = -fDirY * fHalfWidth;
		fNY = fDirX * fHalfWidth;
		tVertices[0].fX = tA.fX + fNX; tVertices[0].fY = tA.fY + fNY; tVertices[0].iColor = iColor;
		tVertices[1].fX = tB.fX + fNX; tVertices[1].fY = tB.fY + fNY; tVertices[1].iColor = iColor;
		tVertices[2].fX = tB.fX - fNX; tVertices[2].fY = tB.fY - fNY; tVertices[2].iColor = iColor;
		tVertices[3].fX = tA.fX - fNX; tVertices[3].fY = tA.fY - fNY; tVertices[3].iColor = iColor;
		iIndices[0] = 0; iIndices[1] = 1; iIndices[2] = 2;
		iIndices[3] = 0; iIndices[4] = 2; iIndices[5] = 3;
		__xgeShapeExDrawStrokePaintMesh(pShape, tVertices, 4, iIndices, 6, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
		if ( pShape->iLineCap == XGE_SHAPE_EX_CAP_ROUND ) {
			if ( bCapStart ) {
				__xgeShapeExDrawStrokeRoundCap(pShape, tA, fDirX, fDirY, fHalfWidth, 0, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
			}
			if ( bCapEnd ) {
				__xgeShapeExDrawStrokeRoundCap(pShape, tB, fDirX, fDirY, fHalfWidth, 1, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
			}
		}
		return XGE_OK;
	}
	if ( bScreenSpace ) {
		xgeShapeLinePx(tA.fX, tA.fY, tB.fX, tB.fY, fWidth, iColor);
	} else {
		xgeShapeLine(tA.fX, tA.fY, tB.fX, tB.fY, fWidth, iColor);
	}
	return XGE_OK;
}

static float __xgeShapeExDashTotal(const float* pDashPattern, int iDashCount)
{
	float fDashTotal;
	int i;

	fDashTotal = 0.0f;
	for ( i = 0; i < iDashCount; i++ ) {
		if ( pDashPattern[i] > 0.0f ) {
			fDashTotal += pDashPattern[i];
		}
	}
	return fDashTotal;
}

static void __xgeShapeExDashStateInit(const float* pDashPattern, int iDashCount, float fDashOffset, int* pDashIndex, int* pDashOn, float* pDashRemaining)
{
	float fDashTotal;
	float fOffset;

	*pDashIndex = 0;
	*pDashOn = 1;
	*pDashRemaining = (iDashCount > 0) ? pDashPattern[0] : 0.0f;
	fDashTotal = __xgeShapeExDashTotal(pDashPattern, iDashCount);
	if ( fDashTotal <= XGE_SHAPE_EX_EPSILON ) {
		return;
	}
	fOffset = fmodf(fDashOffset, fDashTotal);
	if ( fOffset < 0.0f ) {
		fOffset += fDashTotal;
	}
	while ( (fOffset > XGE_SHAPE_EX_EPSILON) && (iDashCount > 0) ) {
		float fStep = pDashPattern[*pDashIndex];

		if ( fStep <= XGE_SHAPE_EX_EPSILON ) {
			*pDashIndex = (*pDashIndex + 1) % iDashCount;
			*pDashOn = !*pDashOn;
			continue;
		}
		if ( fStep > fOffset ) {
			*pDashRemaining = fStep - fOffset;
			return;
		}
		fOffset -= fStep;
		*pDashIndex = (*pDashIndex + 1) % iDashCount;
		*pDashOn = !*pDashOn;
	}
	*pDashRemaining = pDashPattern[*pDashIndex];
}

static void __xgeShapeExDashStateNext(const float* pDashPattern, int iDashCount, int* pDashIndex, int* pDashOn, float* pDashRemaining)
{
	if ( iDashCount <= 0 ) {
		*pDashRemaining = 0.0f;
		return;
	}
	do {
		*pDashIndex = (*pDashIndex + 1) % iDashCount;
		*pDashOn = !*pDashOn;
		*pDashRemaining = pDashPattern[*pDashIndex];
	} while ( (*pDashRemaining <= XGE_SHAPE_EX_EPSILON) );
}

static int __xgeShapeExDrawDashedSegmentState(xge_shape_ex pShape, xge_vec2_t tA, xge_vec2_t tB, float fWidth, uint32_t iColor, const float* pDashPattern, int iDashCount, int* pDashIndex, int* pDashOn, float* pDashRemaining, int bCapStart, int bCapEnd, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, float fOpacity, int bScreenSpace)
{
	float fDX;
	float fDY;
	float fLen;
	float fPos;

	if ( (pDashPattern == NULL) || (iDashCount <= 0) || (pDashIndex == NULL) || (pDashOn == NULL) || (pDashRemaining == NULL) ) {
		return __xgeShapeExDrawStrokeSegment(pShape, tA, tB, fWidth, iColor, bCapStart, bCapEnd, tBounds, tMatrix, fOpacity, bScreenSpace);
	}
	fDX = tB.fX - tA.fX;
	fDY = tB.fY - tA.fY;
	fLen = hypotf(fDX, fDY);
	if ( fLen <= XGE_SHAPE_EX_EPSILON ) {
		return XGE_OK;
	}
	fPos = 0.0f;
	while ( fPos < fLen - XGE_SHAPE_EX_EPSILON ) {
		float fStep = *pDashRemaining;
		float fNext;

		if ( fStep <= XGE_SHAPE_EX_EPSILON ) {
			__xgeShapeExDashStateNext(pDashPattern, iDashCount, pDashIndex, pDashOn, pDashRemaining);
			continue;
		}
		fNext = fPos + fStep;
		if ( fNext > fLen ) {
			fNext = fLen;
		}
		if ( *pDashOn && (fNext > fPos) ) {
			xge_vec2_t tS;
			xge_vec2_t tE;
			float fT0 = fPos / fLen;
			float fT1 = fNext / fLen;
			int iRet;

			tS.fX = tA.fX + fDX * fT0;
			tS.fY = tA.fY + fDY * fT0;
			tE.fX = tA.fX + fDX * fT1;
			tE.fY = tA.fY + fDY * fT1;
			iRet = __xgeShapeExDrawStrokeSegment(pShape, tS, tE, fWidth, iColor, 1, 1, tBounds, tMatrix, fOpacity, bScreenSpace);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
		*pDashRemaining -= (fNext - fPos);
		fPos = fNext;
		if ( *pDashRemaining <= XGE_SHAPE_EX_EPSILON ) {
			__xgeShapeExDashStateNext(pDashPattern, iDashCount, pDashIndex, pDashOn, pDashRemaining);
		}
	}
	return XGE_OK;
}

static int __xgeShapeExDrawStroke(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, float fWidth, uint32_t iColor, const float* pDashPattern, int iDashCount, float fDashOffset, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, float fOpacity, int bScreenSpace)
{
	int i;

	if ( (pFlat == NULL) || (fWidth <= 0.0f) || !__xgeShapeExStrokeVisible(pShape, iColor) ) {
		return XGE_OK;
	}
	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		const xge_shape_ex_flat_contour_t* pContour = &pFlat->pContours[i];
		int iSegmentCount;
		float fDashTotal;
		float fDashRemaining = 0.0f;
		int iDashIndex = 0;
		int bDashOn = 1;
		int j;

		if ( pContour->iCount < 2 ) {
			continue;
		}
		iSegmentCount = pContour->bClosed ? pContour->iCount : (pContour->iCount - 1);
		fDashTotal = __xgeShapeExDashTotal(pDashPattern, iDashCount);
		if ( fDashTotal > XGE_SHAPE_EX_EPSILON ) {
			__xgeShapeExDashStateInit(pDashPattern, iDashCount, fDashOffset, &iDashIndex, &bDashOn, &fDashRemaining);
		}
		for ( j = 0; j < iSegmentCount; j++ ) {
			xge_vec2_t tA = pFlat->pPoints[pContour->iStart + j];
			xge_vec2_t tB = pFlat->pPoints[pContour->iStart + ((j + 1) % pContour->iCount)];
			int bCapStart = (!pContour->bClosed && (j == 0));
			int bCapEnd = (!pContour->bClosed && (j == (iSegmentCount - 1)));
			int iRet;

			if ( fDashTotal > XGE_SHAPE_EX_EPSILON ) {
				iRet = __xgeShapeExDrawDashedSegmentState(pShape, tA, tB, fWidth, iColor, pDashPattern, iDashCount, &iDashIndex, &bDashOn, &fDashRemaining, bCapStart, bCapEnd, tBounds, tMatrix, fOpacity, bScreenSpace);
			} else {
				iRet = __xgeShapeExDrawStrokeSegment(pShape, tA, tB, fWidth, iColor, bCapStart, bCapEnd, tBounds, tMatrix, fOpacity, bScreenSpace);
			}
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
		if ( (fDashTotal <= XGE_SHAPE_EX_EPSILON) && (pShape != NULL) && (pShape->iStrokeType != XGE_SHAPE_EX_FILL_SOLID) && (iSegmentCount > 1) ) {
			int iJoinStart = pContour->bClosed ? 0 : 1;
			int iJoinEnd = pContour->bClosed ? pContour->iCount : (pContour->iCount - 1);

			for ( j = iJoinStart; j < iJoinEnd; j++ ) {
				int iPrev = (j + pContour->iCount - 1) % pContour->iCount;
				int iNext = (j + 1) % pContour->iCount;
				xge_vec2_t tPrev = pFlat->pPoints[pContour->iStart + iPrev];
				xge_vec2_t tCurr = pFlat->pPoints[pContour->iStart + j];
				xge_vec2_t tNext = pFlat->pPoints[pContour->iStart + iNext];
				int iRet = __xgeShapeExDrawStrokeJoin(pShape, tPrev, tCurr, tNext, fWidth, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);

				if ( iRet != XGE_OK ) {
					return iRet;
				}
			}
		}
	}
	return XGE_OK;
}

static int __xgeShapeExDrawInternal(xge_shape_ex pShape, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tParent, float fParentOpacity)
{
	xge_shape_ex_flat_path_t tFlat;
	xge_shape_ex_matrix_t tMatrix;
	uint32_t iFillColor;
	uint32_t iStrokeColor;
	xge_rect_t tStrokeBounds;
	float fOpacity;
	float fStrokeScale;
	float fStrokeWidth;
	float fDashOffset;
	float* pDashPattern;
	xge_rect_t tOldClip;
	int bOldClip;
	int bClipApplied;
	int iRet;

	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !pShape->bVisible ) {
		return XGE_OK;
	}
	fOpacity = fParentOpacity * pShape->fOpacity;
	if ( fOpacity <= 0.0f ) {
		return XGE_OK;
	}
	tMatrix = __xgeShapeExMatrixMul(tParent, pShape->tTransform);
	bClipApplied = 0;
	bOldClip = 0;
	memset(&tOldClip, 0, sizeof(tOldClip));
	if ( pShape->bClipRect ) {
		xge_rect_t tClip = __xgeShapeExMatrixRectBounds(tMatrix, pShape->tClipRect);

		tOldClip = xgeClipGet();
		bOldClip = (tOldClip.fW > 0.0f) && (tOldClip.fH > 0.0f);
		if ( bOldClip ) {
			tClip = __xgeShapeExRectIntersect(tOldClip, tClip);
		}
		xgeClipSet(tClip);
		bClipApplied = 1;
	}
	iRet = __xgeShapeExFlatten(pShape, tMatrix, fTolerance, &tFlat);
	if ( iRet != XGE_OK ) {
		if ( bClipApplied ) {
			if ( bOldClip ) xgeClipSet(tOldClip);
			else xgeClipClear();
		}
		return iRet;
	}
	iFillColor = __xgeShapeExColorOpacity(pShape->iFillColor, fOpacity);
	iStrokeColor = __xgeShapeExColorOpacity(pShape->iStrokeColor, fOpacity);
	tStrokeBounds = __xgeShapeExFlatBounds(&tFlat);
	fStrokeScale = __xgeShapeExMatrixStrokeScale(tMatrix);
	fStrokeWidth = pShape->fStrokeWidth * fStrokeScale;
	fDashOffset = pShape->fDashOffset * fStrokeScale;
	pDashPattern = pShape->pDashPattern;
	if ( (pShape->pDashPattern != NULL) && (pShape->iDashCount > 0) && (fabsf(fStrokeScale - 1.0f) > XGE_SHAPE_EX_EPSILON) ) {
		int i;

		pDashPattern = (float*)xrtMalloc((size_t)pShape->iDashCount * sizeof(*pDashPattern));
		if ( pDashPattern == NULL ) {
			__xgeShapeExFlatFree(&tFlat);
			if ( bClipApplied ) {
				if ( bOldClip ) xgeClipSet(tOldClip);
				else xgeClipClear();
			}
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		for ( i = 0; i < pShape->iDashCount; i++ ) {
			pDashPattern[i] = pShape->pDashPattern[i] * fStrokeScale;
		}
	}
	if ( pShape->bStrokeFirst ) {
		iRet = __xgeShapeExDrawStroke(pShape, &tFlat, fStrokeWidth, iStrokeColor, pDashPattern, pShape->iDashCount, fDashOffset, tStrokeBounds, tMatrix, fOpacity, bScreenSpace);
		if ( iRet == XGE_OK ) {
			iRet = __xgeShapeExDrawFill(pShape, &tFlat, pShape->iFillRule, tMatrix, fOpacity, iFillColor, bScreenSpace);
		}
	} else {
		iRet = __xgeShapeExDrawFill(pShape, &tFlat, pShape->iFillRule, tMatrix, fOpacity, iFillColor, bScreenSpace);
		if ( iRet == XGE_OK ) {
			iRet = __xgeShapeExDrawStroke(pShape, &tFlat, fStrokeWidth, iStrokeColor, pDashPattern, pShape->iDashCount, fDashOffset, tStrokeBounds, tMatrix, fOpacity, bScreenSpace);
		}
	}
	if ( pDashPattern != pShape->pDashPattern ) {
		xrtFree(pDashPattern);
	}
	__xgeShapeExFlatFree(&tFlat);
	if ( bClipApplied ) {
		if ( bOldClip ) xgeClipSet(tOldClip);
		else xgeClipClear();
	}
	return iRet;
}

static int __xgeShapeExSceneDrawInternal(xge_shape_ex_scene pScene, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tParent, float fParentOpacity)
{
	xge_shape_ex_matrix_t tMatrix;
	float fOpacity;
	int i;
	int iRet;

	if ( !__xgeShapeExSceneValid(pScene) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !pScene->bVisible ) {
		return XGE_OK;
	}
	fOpacity = fParentOpacity * pScene->fOpacity;
	if ( fOpacity <= 0.0f ) {
		return XGE_OK;
	}
	tMatrix = __xgeShapeExMatrixMul(tParent, pScene->tTransform);
	for ( i = 0; i < pScene->iShapeCount; i++ ) {
		iRet = __xgeShapeExDrawInternal(pScene->pShapes[i], fTolerance, bScreenSpace, tMatrix, fOpacity);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeShapeExSvgIsSpace(char c)
{
	return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n') || (c == '\f');
}

static int __xgeShapeExSvgIsCommand(char c)
{
	switch ( c ) {
		case 'M': case 'm': case 'L': case 'l': case 'H': case 'h': case 'V': case 'v':
		case 'C': case 'c': case 'S': case 's': case 'Q': case 'q': case 'T': case 't':
		case 'A': case 'a': case 'Z': case 'z':
			return 1;
		default:
			return 0;
	}
}

static char __xgeShapeExSvgLower(char c)
{
	return (char)((c >= 'A' && c <= 'Z') ? (c + 32) : c);
}

static void __xgeShapeExSvgSkipSeparators(const char** ppText)
{
	const char* pText = *ppText;

	while ( (*pText == ',') || __xgeShapeExSvgIsSpace(*pText) ) {
		pText++;
	}
	*ppText = pText;
}

static int __xgeShapeExSvgHasNumber(const char* sText)
{
	__xgeShapeExSvgSkipSeparators(&sText);
	return (*sText != '\0') && !__xgeShapeExSvgIsCommand(*sText);
}

static int __xgeShapeExSvgReadNumber(const char** ppText, float* pValue)
{
	const char* pText;
	char* pEnd;
	double fValue;

	if ( (ppText == NULL) || (*ppText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	pText = *ppText;
	__xgeShapeExSvgSkipSeparators(&pText);
	if ( (*pText == '\0') || __xgeShapeExSvgIsCommand(*pText) ) {
		return 0;
	}
	fValue = strtod(pText, &pEnd);
	if ( pEnd == pText ) {
		return 0;
	}
	*pValue = (float)fValue;
	*ppText = pEnd;
	return 1;
}

static int __xgeShapeExSvgArcTo(xge_shape_ex pShape, xge_vec2_t tCurrent, float fRX, float fRY, float fAxisDegrees, int bLargeArc, int bSweep, float fX, float fY)
{
	float fPhi;
	float fCosPhi;
	float fSinPhi;
	float fDX2;
	float fDY2;
	float fX1p;
	float fY1p;
	float fLambda;
	float fSign;
	float fNumerator;
	float fDenominator;
	float fCoef;
	float fCXp;
	float fCYp;
	float fCX;
	float fCY;
	float fUX;
	float fUY;
	float fVX;
	float fVY;
	float fTheta;
	float fDelta;
	int iSegments;
	int i;

	fRX = fabsf(fRX);
	fRY = fabsf(fRY);
	if ( (fRX <= XGE_SHAPE_EX_EPSILON) || (fRY <= XGE_SHAPE_EX_EPSILON) ||
	     (hypotf(fX - tCurrent.fX, fY - tCurrent.fY) <= XGE_SHAPE_EX_EPSILON) ) {
		return xgeShapeExLineTo(pShape, fX, fY);
	}
	fPhi = fAxisDegrees * (XGE_SHAPE_EX_PI / 180.0f);
	fCosPhi = cosf(fPhi);
	fSinPhi = sinf(fPhi);
	fDX2 = (tCurrent.fX - fX) * 0.5f;
	fDY2 = (tCurrent.fY - fY) * 0.5f;
	fX1p = (fCosPhi * fDX2) + (fSinPhi * fDY2);
	fY1p = (-fSinPhi * fDX2) + (fCosPhi * fDY2);
	fLambda = (fX1p * fX1p) / (fRX * fRX) + (fY1p * fY1p) / (fRY * fRY);
	if ( fLambda > 1.0f ) {
		float fScale = sqrtf(fLambda);
		fRX *= fScale;
		fRY *= fScale;
	}
	fSign = (bLargeArc == bSweep) ? -1.0f : 1.0f;
	fNumerator = (fRX * fRX * fRY * fRY) - (fRX * fRX * fY1p * fY1p) - (fRY * fRY * fX1p * fX1p);
	fDenominator = (fRX * fRX * fY1p * fY1p) + (fRY * fRY * fX1p * fX1p);
	fCoef = (fDenominator <= XGE_SHAPE_EX_EPSILON) ? 0.0f : fSign * sqrtf(fmaxf(0.0f, fNumerator / fDenominator));
	fCXp = fCoef * ((fRX * fY1p) / fRY);
	fCYp = fCoef * (-(fRY * fX1p) / fRX);
	fCX = (fCosPhi * fCXp) - (fSinPhi * fCYp) + ((tCurrent.fX + fX) * 0.5f);
	fCY = (fSinPhi * fCXp) + (fCosPhi * fCYp) + ((tCurrent.fY + fY) * 0.5f);
	fUX = (fX1p - fCXp) / fRX;
	fUY = (fY1p - fCYp) / fRY;
	fVX = (-fX1p - fCXp) / fRX;
	fVY = (-fY1p - fCYp) / fRY;
	fTheta = atan2f(fUY, fUX);
	fDelta = atan2f((fUX * fVY) - (fUY * fVX), (fUX * fVX) + (fUY * fVY));
	if ( !bSweep && fDelta > 0.0f ) {
		fDelta -= XGE_SHAPE_EX_TAU;
	} else if ( bSweep && fDelta < 0.0f ) {
		fDelta += XGE_SHAPE_EX_TAU;
	}
	iSegments = (int)ceilf(fabsf(fDelta) / (XGE_SHAPE_EX_PI * 0.5f));
	if ( iSegments < 1 ) {
		iSegments = 1;
	}
	for ( i = 0; i < iSegments; i++ ) {
		float fT0 = fTheta + fDelta * ((float)i / (float)iSegments);
		float fT1 = fTheta + fDelta * ((float)(i + 1) / (float)iSegments);
		float fAlpha = (4.0f / 3.0f) * tanf((fT1 - fT0) * 0.25f);
		float fCos0 = cosf(fT0);
		float fSin0 = sinf(fT0);
		float fCos1 = cosf(fT1);
		float fSin1 = sinf(fT1);
		xge_vec2_t tC1;
		xge_vec2_t tC2;
		xge_vec2_t tP;

		tC1.fX = fCX + fRX * ((fCos0 - fAlpha * fSin0) * fCosPhi) - fRY * ((fSin0 + fAlpha * fCos0) * fSinPhi);
		tC1.fY = fCY + fRX * ((fCos0 - fAlpha * fSin0) * fSinPhi) + fRY * ((fSin0 + fAlpha * fCos0) * fCosPhi);
		tC2.fX = fCX + fRX * ((fCos1 + fAlpha * fSin1) * fCosPhi) - fRY * ((fSin1 - fAlpha * fCos1) * fSinPhi);
		tC2.fY = fCY + fRX * ((fCos1 + fAlpha * fSin1) * fSinPhi) + fRY * ((fSin1 - fAlpha * fCos1) * fCosPhi);
		if ( i == (iSegments - 1) ) {
			tP.fX = fX;
			tP.fY = fY;
		} else {
			tP.fX = fCX + fRX * fCos1 * fCosPhi - fRY * fSin1 * fSinPhi;
			tP.fY = fCY + fRX * fCos1 * fSinPhi + fRY * fSin1 * fCosPhi;
		}
		if ( xgeShapeExCubicTo(pShape, tC1.fX, tC1.fY, tC2.fX, tC2.fY, tP.fX, tP.fY) != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	return XGE_OK;
}

int xgeShapeExCreate(xge_shape_ex* ppShape)
{
	xge_shape_ex pShape;

	if ( ppShape == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pShape = (xge_shape_ex)xrtCalloc(1, sizeof(*pShape));
	if ( pShape == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pShape->iMagic = XGE_SHAPE_EX_MAGIC;
	pShape->iRefCount = 1;
	pShape->iFillColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	pShape->iStrokeType = XGE_SHAPE_EX_FILL_SOLID;
	pShape->iStrokeColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	pShape->fStrokeWidth = 0.0f;
	pShape->iFillRule = XGE_SHAPE_EX_FILL_NON_ZERO;
	pShape->iLineJoin = XGE_SHAPE_EX_JOIN_BEVEL;
	pShape->iLineCap = XGE_SHAPE_EX_CAP_SQUARE;
	pShape->fMiterLimit = 4.0f;
	pShape->fOpacity = 1.0f;
	pShape->bVisible = 1;
	pShape->iFillGradientSpread = XGE_SHAPE_EX_GRADIENT_SPREAD_PAD;
	pShape->tFillGradientTransform = __xgeShapeExMatrixIdentity();
	pShape->iStrokeGradientSpread = XGE_SHAPE_EX_GRADIENT_SPREAD_PAD;
	pShape->tStrokeGradientTransform = __xgeShapeExMatrixIdentity();
	pShape->tTransform = __xgeShapeExMatrixIdentity();
	*ppShape = pShape;
	return XGE_OK;
}

int xgeShapeExAddRef(xge_shape_ex pShape)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pShape->iRefCount++;
	return XGE_OK;
}

int xgeShapeExClone(xge_shape_ex pShape, xge_shape_ex* ppClone)
{
	xge_shape_ex pClone;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (ppClone == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppClone = NULL;
	iRet = xgeShapeExCreate(&pClone);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeShapeExReserveCommands(pClone, pShape->iCommandCount);
	if ( iRet == XGE_OK ) {
		iRet = __xgeShapeExReservePoints(pClone, pShape->iPointCount);
	}
	if ( iRet != XGE_OK ) {
		xgeShapeExDestroy(pClone);
		return iRet;
	}
	if ( pShape->iCommandCount > 0 ) {
		memcpy(pClone->pCommands, pShape->pCommands, (size_t)pShape->iCommandCount * sizeof(*pShape->pCommands));
	}
	if ( pShape->iPointCount > 0 ) {
		memcpy(pClone->pPoints, pShape->pPoints, (size_t)pShape->iPointCount * sizeof(*pShape->pPoints));
	}
	pClone->iCommandCount = pShape->iCommandCount;
	pClone->iPointCount = pShape->iPointCount;
	pClone->tCurrent = pShape->tCurrent;
	pClone->tStart = pShape->tStart;
	pClone->tLastCubicControl = pShape->tLastCubicControl;
	pClone->tLastQuadControl = pShape->tLastQuadControl;
	pClone->bHasCurrent = pShape->bHasCurrent;
	pClone->bHasStart = pShape->bHasStart;
	pClone->iFillType = pShape->iFillType;
	pClone->iFillColor = pShape->iFillColor;
	pClone->iStrokeType = pShape->iStrokeType;
	pClone->iStrokeColor = pShape->iStrokeColor;
	pClone->fStrokeWidth = pShape->fStrokeWidth;
	pClone->fFillX1 = pShape->fFillX1;
	pClone->fFillY1 = pShape->fFillY1;
	pClone->fFillX2 = pShape->fFillX2;
	pClone->fFillY2 = pShape->fFillY2;
	pClone->fFillR = pShape->fFillR;
	pClone->fFillFX = pShape->fFillFX;
	pClone->fFillFY = pShape->fFillFY;
	pClone->iFillGradientUnits = pShape->iFillGradientUnits;
	pClone->iFillGradientSpread = pShape->iFillGradientSpread;
	pClone->tFillGradientTransform = pShape->tFillGradientTransform;
	pClone->fStrokeX1 = pShape->fStrokeX1;
	pClone->fStrokeY1 = pShape->fStrokeY1;
	pClone->fStrokeX2 = pShape->fStrokeX2;
	pClone->fStrokeY2 = pShape->fStrokeY2;
	pClone->fStrokeR = pShape->fStrokeR;
	pClone->fStrokeFX = pShape->fStrokeFX;
	pClone->fStrokeFY = pShape->fStrokeFY;
	pClone->iStrokeGradientUnits = pShape->iStrokeGradientUnits;
	pClone->iStrokeGradientSpread = pShape->iStrokeGradientSpread;
	pClone->tStrokeGradientTransform = pShape->tStrokeGradientTransform;
	pClone->iFillRule = pShape->iFillRule;
	pClone->iLineJoin = pShape->iLineJoin;
	pClone->iLineCap = pShape->iLineCap;
	pClone->fMiterLimit = pShape->fMiterLimit;
	pClone->bStrokeFirst = pShape->bStrokeFirst;
	pClone->fOpacity = pShape->fOpacity;
	pClone->bVisible = pShape->bVisible;
	pClone->bClipRect = pShape->bClipRect;
	pClone->tClipRect = pShape->tClipRect;
	pClone->tTransform = pShape->tTransform;
	if ( (pShape->pDashPattern != NULL) && (pShape->iDashCount > 0) ) {
		iRet = xgeShapeExStrokeDash(pClone, pShape->pDashPattern, pShape->iDashCount, pShape->fDashOffset);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pClone);
			return iRet;
		}
	}
	if ( (pShape->pFillStops != NULL) && (pShape->iFillStopCount > 0) ) {
		iRet = __xgeShapeExReserveFillStops(pClone, pShape->iFillStopCount);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pClone);
			return iRet;
		}
		memcpy(pClone->pFillStops, pShape->pFillStops, (size_t)pShape->iFillStopCount * sizeof(*pShape->pFillStops));
		pClone->iFillStopCount = pShape->iFillStopCount;
	}
	if ( (pShape->pStrokeStops != NULL) && (pShape->iStrokeStopCount > 0) ) {
		iRet = __xgeShapeExReserveStrokeStops(pClone, pShape->iStrokeStopCount);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pClone);
			return iRet;
		}
		memcpy(pClone->pStrokeStops, pShape->pStrokeStops, (size_t)pShape->iStrokeStopCount * sizeof(*pShape->pStrokeStops));
		pClone->iStrokeStopCount = pShape->iStrokeStopCount;
	}
	*ppClone = pClone;
	return XGE_OK;
}

void xgeShapeExDestroy(xge_shape_ex pShape)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return;
	}
	pShape->iRefCount--;
	if ( pShape->iRefCount > 0 ) {
		return;
	}
	pShape->iMagic = 0;
	xrtFree(pShape->pCommands);
	xrtFree(pShape->pPoints);
	xrtFree(pShape->pDashPattern);
	xrtFree(pShape->pFillStops);
	xrtFree(pShape->pStrokeStops);
	xrtFree(pShape);
}

int xgeShapeExReset(xge_shape_ex pShape)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pShape->iCommandCount = 0;
	pShape->iPointCount = 0;
	pShape->bHasCurrent = 0;
	pShape->bHasStart = 0;
	memset(&pShape->tCurrent, 0, sizeof(pShape->tCurrent));
	memset(&pShape->tStart, 0, sizeof(pShape->tStart));
	memset(&pShape->tLastCubicControl, 0, sizeof(pShape->tLastCubicControl));
	memset(&pShape->tLastQuadControl, 0, sizeof(pShape->tLastQuadControl));
	return XGE_OK;
}

int xgeShapeExMoveTo(xge_shape_ex pShape, float fX, float fY)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeShapeExAddMoveTo(pShape, fX, fY);
}

int xgeShapeExLineTo(xge_shape_ex pShape, float fX, float fY)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeShapeExAddLineTo(pShape, fX, fY);
}

int xgeShapeExQuadTo(xge_shape_ex pShape, float fCX, float fCY, float fX, float fY)
{
	xge_vec2_t tC1;
	xge_vec2_t tC2;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || !pShape->bHasCurrent ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tC1.fX = pShape->tCurrent.fX + (2.0f / 3.0f) * (fCX - pShape->tCurrent.fX);
	tC1.fY = pShape->tCurrent.fY + (2.0f / 3.0f) * (fCY - pShape->tCurrent.fY);
	tC2.fX = fX + (2.0f / 3.0f) * (fCX - fX);
	tC2.fY = fY + (2.0f / 3.0f) * (fCY - fY);
	iRet = __xgeShapeExAddCubicTo(pShape, tC1.fX, tC1.fY, tC2.fX, tC2.fY, fX, fY);
	if ( iRet == XGE_OK ) {
		pShape->tLastQuadControl.fX = fCX;
		pShape->tLastQuadControl.fY = fCY;
	}
	return iRet;
}

int xgeShapeExCubicTo(xge_shape_ex pShape, float fC1X, float fC1Y, float fC2X, float fC2Y, float fX, float fY)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeShapeExAddCubicTo(pShape, fC1X, fC1Y, fC2X, fC2Y, fX, fY);
}

int xgeShapeExClose(xge_shape_ex pShape)
{
	int iRet;

	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeShapeExAppendCommand(pShape, XGE_SHAPE_EX_CMD_CLOSE, NULL, 0);
	if ( iRet == XGE_OK ) {
		pShape->tCurrent = pShape->tStart;
		pShape->tLastCubicControl = pShape->tCurrent;
		pShape->tLastQuadControl = pShape->tCurrent;
	}
	return iRet;
}

int xgeShapeExAppendRect(xge_shape_ex pShape, float fX, float fY, float fW, float fH, float fRX, float fRY, int bClockwise)
{
	float fRMaxX;
	float fRMaxY;

	if ( !__xgeShapeExValid(pShape) || (fW < 0.0f) || (fH < 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fRMaxX = fW * 0.5f;
	fRMaxY = fH * 0.5f;
	if ( fRX < 0.0f ) fRX = 0.0f;
	if ( fRY < 0.0f ) fRY = 0.0f;
	if ( fRX > fRMaxX ) fRX = fRMaxX;
	if ( fRY > fRMaxY ) fRY = fRMaxY;
	if ( (fRX <= XGE_SHAPE_EX_EPSILON) || (fRY <= XGE_SHAPE_EX_EPSILON) ) {
		if ( bClockwise ) {
			xgeShapeExMoveTo(pShape, fX, fY);
			xgeShapeExLineTo(pShape, fX + fW, fY);
			xgeShapeExLineTo(pShape, fX + fW, fY + fH);
			xgeShapeExLineTo(pShape, fX, fY + fH);
		} else {
			xgeShapeExMoveTo(pShape, fX, fY);
			xgeShapeExLineTo(pShape, fX, fY + fH);
			xgeShapeExLineTo(pShape, fX + fW, fY + fH);
			xgeShapeExLineTo(pShape, fX + fW, fY);
		}
		return xgeShapeExClose(pShape);
	}
	if ( bClockwise ) {
		xgeShapeExMoveTo(pShape, fX + fRX, fY);
		xgeShapeExLineTo(pShape, fX + fW - fRX, fY);
		xgeShapeExCubicTo(pShape, fX + fW - fRX + fRX * XGE_SHAPE_EX_KAPPA, fY, fX + fW, fY + fRY - fRY * XGE_SHAPE_EX_KAPPA, fX + fW, fY + fRY);
		xgeShapeExLineTo(pShape, fX + fW, fY + fH - fRY);
		xgeShapeExCubicTo(pShape, fX + fW, fY + fH - fRY + fRY * XGE_SHAPE_EX_KAPPA, fX + fW - fRX + fRX * XGE_SHAPE_EX_KAPPA, fY + fH, fX + fW - fRX, fY + fH);
		xgeShapeExLineTo(pShape, fX + fRX, fY + fH);
		xgeShapeExCubicTo(pShape, fX + fRX - fRX * XGE_SHAPE_EX_KAPPA, fY + fH, fX, fY + fH - fRY + fRY * XGE_SHAPE_EX_KAPPA, fX, fY + fH - fRY);
		xgeShapeExLineTo(pShape, fX, fY + fRY);
		xgeShapeExCubicTo(pShape, fX, fY + fRY - fRY * XGE_SHAPE_EX_KAPPA, fX + fRX - fRX * XGE_SHAPE_EX_KAPPA, fY, fX + fRX, fY);
	} else {
		xgeShapeExMoveTo(pShape, fX + fRX, fY);
		xgeShapeExCubicTo(pShape, fX + fRX - fRX * XGE_SHAPE_EX_KAPPA, fY, fX, fY + fRY - fRY * XGE_SHAPE_EX_KAPPA, fX, fY + fRY);
		xgeShapeExLineTo(pShape, fX, fY + fH - fRY);
		xgeShapeExCubicTo(pShape, fX, fY + fH - fRY + fRY * XGE_SHAPE_EX_KAPPA, fX + fRX - fRX * XGE_SHAPE_EX_KAPPA, fY + fH, fX + fRX, fY + fH);
		xgeShapeExLineTo(pShape, fX + fW - fRX, fY + fH);
		xgeShapeExCubicTo(pShape, fX + fW - fRX + fRX * XGE_SHAPE_EX_KAPPA, fY + fH, fX + fW, fY + fH - fRY + fRY * XGE_SHAPE_EX_KAPPA, fX + fW, fY + fH - fRY);
		xgeShapeExLineTo(pShape, fX + fW, fY + fRY);
		xgeShapeExCubicTo(pShape, fX + fW, fY + fRY - fRY * XGE_SHAPE_EX_KAPPA, fX + fW - fRX + fRX * XGE_SHAPE_EX_KAPPA, fY, fX + fW - fRX, fY);
		xgeShapeExLineTo(pShape, fX + fRX, fY);
	}
	return xgeShapeExClose(pShape);
}

int xgeShapeExAppendCircle(xge_shape_ex pShape, float fCX, float fCY, float fRX, float fRY, int bClockwise)
{
	float kx;
	float ky;

	if ( !__xgeShapeExValid(pShape) || (fRX < 0.0f) || (fRY < 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	kx = fRX * XGE_SHAPE_EX_KAPPA;
	ky = fRY * XGE_SHAPE_EX_KAPPA;
	if ( bClockwise ) {
		xgeShapeExMoveTo(pShape, fCX, fCY - fRY);
		xgeShapeExCubicTo(pShape, fCX + kx, fCY - fRY, fCX + fRX, fCY - ky, fCX + fRX, fCY);
		xgeShapeExCubicTo(pShape, fCX + fRX, fCY + ky, fCX + kx, fCY + fRY, fCX, fCY + fRY);
		xgeShapeExCubicTo(pShape, fCX - kx, fCY + fRY, fCX - fRX, fCY + ky, fCX - fRX, fCY);
		xgeShapeExCubicTo(pShape, fCX - fRX, fCY - ky, fCX - kx, fCY - fRY, fCX, fCY - fRY);
	} else {
		xgeShapeExMoveTo(pShape, fCX, fCY - fRY);
		xgeShapeExCubicTo(pShape, fCX - kx, fCY - fRY, fCX - fRX, fCY - ky, fCX - fRX, fCY);
		xgeShapeExCubicTo(pShape, fCX - fRX, fCY + ky, fCX - kx, fCY + fRY, fCX, fCY + fRY);
		xgeShapeExCubicTo(pShape, fCX + kx, fCY + fRY, fCX + fRX, fCY + ky, fCX + fRX, fCY);
		xgeShapeExCubicTo(pShape, fCX + fRX, fCY - ky, fCX + kx, fCY - fRY, fCX, fCY - fRY);
	}
	return xgeShapeExClose(pShape);
}

int xgeShapeExAppendSvgPath(xge_shape_ex pShape, const char* sPath)
{
	const char* pText;
	char cCommand;
	char cPrevCommand;
	int bRelative;

	if ( !__xgeShapeExValid(pShape) || (sPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pText = sPath;
	cCommand = 0;
	cPrevCommand = 0;
	while ( 1 ) {
		__xgeShapeExSvgSkipSeparators(&pText);
		if ( *pText == '\0' ) {
			break;
		}
		if ( __xgeShapeExSvgIsCommand(*pText) ) {
			cCommand = *pText++;
		} else if ( cCommand == 0 ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		bRelative = (cCommand >= 'a') && (cCommand <= 'z');
		switch ( __xgeShapeExSvgLower(cCommand) ) {
			case 'm': {
				float fX;
				float fY;
				int bFirst = 1;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					if ( !__xgeShapeExSvgReadNumber(&pText, &fX) || !__xgeShapeExSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( bRelative && pShape->bHasCurrent ) {
						fX += pShape->tCurrent.fX;
						fY += pShape->tCurrent.fY;
					}
					if ( bFirst ) {
						xgeShapeExMoveTo(pShape, fX, fY);
						bFirst = 0;
					} else {
						xgeShapeExLineTo(pShape, fX, fY);
					}
				}
				cPrevCommand = cCommand;
				cCommand = bRelative ? 'l' : 'L';
				break;
			}
			case 'l': {
				float fX;
				float fY;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					if ( !__xgeShapeExSvgReadNumber(&pText, &fX) || !__xgeShapeExSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( bRelative ) {
						fX += pShape->tCurrent.fX;
						fY += pShape->tCurrent.fY;
					}
					xgeShapeExLineTo(pShape, fX, fY);
				}
				cPrevCommand = cCommand;
				break;
			}
			case 'h': {
				float fX;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					if ( !__xgeShapeExSvgReadNumber(&pText, &fX) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( bRelative ) fX += pShape->tCurrent.fX;
					xgeShapeExLineTo(pShape, fX, pShape->tCurrent.fY);
				}
				cPrevCommand = cCommand;
				break;
			}
			case 'v': {
				float fY;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					if ( !__xgeShapeExSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( bRelative ) fY += pShape->tCurrent.fY;
					xgeShapeExLineTo(pShape, pShape->tCurrent.fX, fY);
				}
				cPrevCommand = cCommand;
				break;
			}
			case 'c': {
				float fC1X, fC1Y, fC2X, fC2Y, fX, fY;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					if ( !__xgeShapeExSvgReadNumber(&pText, &fC1X) || !__xgeShapeExSvgReadNumber(&pText, &fC1Y) ||
					     !__xgeShapeExSvgReadNumber(&pText, &fC2X) || !__xgeShapeExSvgReadNumber(&pText, &fC2Y) ||
					     !__xgeShapeExSvgReadNumber(&pText, &fX) || !__xgeShapeExSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( bRelative ) {
						fC1X += pShape->tCurrent.fX; fC1Y += pShape->tCurrent.fY;
						fC2X += pShape->tCurrent.fX; fC2Y += pShape->tCurrent.fY;
						fX += pShape->tCurrent.fX; fY += pShape->tCurrent.fY;
					}
					xgeShapeExCubicTo(pShape, fC1X, fC1Y, fC2X, fC2Y, fX, fY);
				}
				cPrevCommand = cCommand;
				break;
			}
			case 's': {
				float fC1X, fC1Y, fC2X, fC2Y, fX, fY;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					if ( !__xgeShapeExSvgReadNumber(&pText, &fC2X) || !__xgeShapeExSvgReadNumber(&pText, &fC2Y) ||
					     !__xgeShapeExSvgReadNumber(&pText, &fX) || !__xgeShapeExSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( (__xgeShapeExSvgLower(cPrevCommand) == 'c') || (__xgeShapeExSvgLower(cPrevCommand) == 's') ) {
						fC1X = (2.0f * pShape->tCurrent.fX) - pShape->tLastCubicControl.fX;
						fC1Y = (2.0f * pShape->tCurrent.fY) - pShape->tLastCubicControl.fY;
					} else {
						fC1X = pShape->tCurrent.fX;
						fC1Y = pShape->tCurrent.fY;
					}
					if ( bRelative ) {
						fC2X += pShape->tCurrent.fX; fC2Y += pShape->tCurrent.fY;
						fX += pShape->tCurrent.fX; fY += pShape->tCurrent.fY;
					}
					xgeShapeExCubicTo(pShape, fC1X, fC1Y, fC2X, fC2Y, fX, fY);
					cPrevCommand = cCommand;
				}
				cPrevCommand = cCommand;
				break;
			}
			case 'q': {
				float fCX, fCY, fX, fY;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					if ( !__xgeShapeExSvgReadNumber(&pText, &fCX) || !__xgeShapeExSvgReadNumber(&pText, &fCY) ||
					     !__xgeShapeExSvgReadNumber(&pText, &fX) || !__xgeShapeExSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( bRelative ) {
						fCX += pShape->tCurrent.fX; fCY += pShape->tCurrent.fY;
						fX += pShape->tCurrent.fX; fY += pShape->tCurrent.fY;
					}
					xgeShapeExQuadTo(pShape, fCX, fCY, fX, fY);
				}
				cPrevCommand = cCommand;
				break;
			}
			case 't': {
				float fCX, fCY, fX, fY;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					if ( !__xgeShapeExSvgReadNumber(&pText, &fX) || !__xgeShapeExSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( (__xgeShapeExSvgLower(cPrevCommand) == 'q') || (__xgeShapeExSvgLower(cPrevCommand) == 't') ) {
						fCX = (2.0f * pShape->tCurrent.fX) - pShape->tLastQuadControl.fX;
						fCY = (2.0f * pShape->tCurrent.fY) - pShape->tLastQuadControl.fY;
					} else {
						fCX = pShape->tCurrent.fX;
						fCY = pShape->tCurrent.fY;
					}
					if ( bRelative ) {
						fX += pShape->tCurrent.fX;
						fY += pShape->tCurrent.fY;
					}
					xgeShapeExQuadTo(pShape, fCX, fCY, fX, fY);
					cPrevCommand = cCommand;
				}
				cPrevCommand = cCommand;
				break;
			}
			case 'a': {
				float fRX, fRY, fAxis, fLargeArc, fSweep, fX, fY;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					xge_vec2_t tCurrent = pShape->tCurrent;
					if ( !__xgeShapeExSvgReadNumber(&pText, &fRX) || !__xgeShapeExSvgReadNumber(&pText, &fRY) ||
					     !__xgeShapeExSvgReadNumber(&pText, &fAxis) || !__xgeShapeExSvgReadNumber(&pText, &fLargeArc) ||
					     !__xgeShapeExSvgReadNumber(&pText, &fSweep) || !__xgeShapeExSvgReadNumber(&pText, &fX) ||
					     !__xgeShapeExSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( bRelative ) {
						fX += pShape->tCurrent.fX;
						fY += pShape->tCurrent.fY;
					}
					__xgeShapeExSvgArcTo(pShape, tCurrent, fRX, fRY, fAxis, fLargeArc != 0.0f, fSweep != 0.0f, fX, fY);
				}
				cPrevCommand = cCommand;
				break;
			}
			case 'z':
				xgeShapeExClose(pShape);
				cPrevCommand = cCommand;
				break;
			default:
				return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

int xgeShapeExFillColor(xge_shape_ex pShape, uint32_t iColor)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->iFillType = XGE_SHAPE_EX_FILL_SOLID;
	pShape->iFillColor = iColor;
	return XGE_OK;
}

int xgeShapeExFillLinearGradient(xge_shape_ex pShape, float fX1, float fY1, float fX2, float fY2, int iUnits, const xge_shape_ex_color_stop_t* pStops, int iStopCount)
{
	int i;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (pStops == NULL) || (iStopCount <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iUnits != XGE_SHAPE_EX_GRADIENT_USER_SPACE) && (iUnits != XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeShapeExReserveFillStops(pShape, iStopCount);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	for ( i = 0; i < iStopCount; i++ ) {
		pShape->pFillStops[i] = pStops[i];
		if ( pShape->pFillStops[i].fOffset < 0.0f ) pShape->pFillStops[i].fOffset = 0.0f;
		if ( pShape->pFillStops[i].fOffset > 1.0f ) pShape->pFillStops[i].fOffset = 1.0f;
	}
	__xgeShapeExSortStops(pShape->pFillStops, iStopCount);
	pShape->iFillStopCount = iStopCount;
	pShape->iFillType = XGE_SHAPE_EX_FILL_LINEAR_GRADIENT;
	pShape->iFillColor = pShape->pFillStops[iStopCount - 1].iColor;
	pShape->fFillX1 = fX1;
	pShape->fFillY1 = fY1;
	pShape->fFillX2 = fX2;
	pShape->fFillY2 = fY2;
	pShape->iFillGradientUnits = iUnits;
	pShape->iFillGradientSpread = XGE_SHAPE_EX_GRADIENT_SPREAD_PAD;
	pShape->tFillGradientTransform = __xgeShapeExMatrixIdentity();
	return XGE_OK;
}

int xgeShapeExFillRadialGradient(xge_shape_ex pShape, float fCX, float fCY, float fRadius, float fFX, float fFY, int iUnits, const xge_shape_ex_color_stop_t* pStops, int iStopCount)
{
	int i;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (pStops == NULL) || (iStopCount <= 0) || (fRadius <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iUnits != XGE_SHAPE_EX_GRADIENT_USER_SPACE) && (iUnits != XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeShapeExReserveFillStops(pShape, iStopCount);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	for ( i = 0; i < iStopCount; i++ ) {
		pShape->pFillStops[i] = pStops[i];
		if ( pShape->pFillStops[i].fOffset < 0.0f ) pShape->pFillStops[i].fOffset = 0.0f;
		if ( pShape->pFillStops[i].fOffset > 1.0f ) pShape->pFillStops[i].fOffset = 1.0f;
	}
	__xgeShapeExSortStops(pShape->pFillStops, iStopCount);
	pShape->iFillStopCount = iStopCount;
	pShape->iFillType = XGE_SHAPE_EX_FILL_RADIAL_GRADIENT;
	pShape->iFillColor = pShape->pFillStops[iStopCount - 1].iColor;
	pShape->fFillX1 = fCX;
	pShape->fFillY1 = fCY;
	pShape->fFillX2 = fCX + fRadius;
	pShape->fFillY2 = fCY;
	pShape->fFillR = fRadius;
	pShape->fFillFX = fFX;
	pShape->fFillFY = fFY;
	pShape->iFillGradientUnits = iUnits;
	pShape->iFillGradientSpread = XGE_SHAPE_EX_GRADIENT_SPREAD_PAD;
	pShape->tFillGradientTransform = __xgeShapeExMatrixIdentity();
	return XGE_OK;
}

int xgeShapeExFillGradientSpread(xge_shape_ex pShape, int iSpread)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iSpread != XGE_SHAPE_EX_GRADIENT_SPREAD_PAD) &&
	     (iSpread != XGE_SHAPE_EX_GRADIENT_SPREAD_REFLECT) &&
	     (iSpread != XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pShape->iFillGradientSpread = iSpread;
	return XGE_OK;
}

int xgeShapeExFillGradientTransformSet(xge_shape_ex pShape, const xge_shape_ex_matrix_t* pMatrix)
{
	if ( !__xgeShapeExValid(pShape) || (pMatrix == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pShape->tFillGradientTransform = *pMatrix;
	return XGE_OK;
}

int xgeShapeExFillGradientTransformIdentity(xge_shape_ex pShape)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pShape->tFillGradientTransform = __xgeShapeExMatrixIdentity();
	return XGE_OK;
}

int xgeShapeExStrokeColor(xge_shape_ex pShape, uint32_t iColor)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->iStrokeType = XGE_SHAPE_EX_FILL_SOLID;
	pShape->iStrokeColor = iColor;
	return XGE_OK;
}

int xgeShapeExStrokeLinearGradient(xge_shape_ex pShape, float fX1, float fY1, float fX2, float fY2, int iUnits, const xge_shape_ex_color_stop_t* pStops, int iStopCount)
{
	int i;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (pStops == NULL) || (iStopCount <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iUnits != XGE_SHAPE_EX_GRADIENT_USER_SPACE) && (iUnits != XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeShapeExReserveStrokeStops(pShape, iStopCount);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	for ( i = 0; i < iStopCount; i++ ) {
		pShape->pStrokeStops[i] = pStops[i];
		if ( pShape->pStrokeStops[i].fOffset < 0.0f ) pShape->pStrokeStops[i].fOffset = 0.0f;
		if ( pShape->pStrokeStops[i].fOffset > 1.0f ) pShape->pStrokeStops[i].fOffset = 1.0f;
	}
	__xgeShapeExSortStops(pShape->pStrokeStops, iStopCount);
	pShape->iStrokeStopCount = iStopCount;
	pShape->iStrokeType = XGE_SHAPE_EX_FILL_LINEAR_GRADIENT;
	pShape->iStrokeColor = pShape->pStrokeStops[iStopCount - 1].iColor;
	pShape->fStrokeX1 = fX1;
	pShape->fStrokeY1 = fY1;
	pShape->fStrokeX2 = fX2;
	pShape->fStrokeY2 = fY2;
	pShape->iStrokeGradientUnits = iUnits;
	pShape->iStrokeGradientSpread = XGE_SHAPE_EX_GRADIENT_SPREAD_PAD;
	pShape->tStrokeGradientTransform = __xgeShapeExMatrixIdentity();
	return XGE_OK;
}

int xgeShapeExStrokeRadialGradient(xge_shape_ex pShape, float fCX, float fCY, float fRadius, float fFX, float fFY, int iUnits, const xge_shape_ex_color_stop_t* pStops, int iStopCount)
{
	int i;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (pStops == NULL) || (iStopCount <= 0) || (fRadius <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iUnits != XGE_SHAPE_EX_GRADIENT_USER_SPACE) && (iUnits != XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeShapeExReserveStrokeStops(pShape, iStopCount);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	for ( i = 0; i < iStopCount; i++ ) {
		pShape->pStrokeStops[i] = pStops[i];
		if ( pShape->pStrokeStops[i].fOffset < 0.0f ) pShape->pStrokeStops[i].fOffset = 0.0f;
		if ( pShape->pStrokeStops[i].fOffset > 1.0f ) pShape->pStrokeStops[i].fOffset = 1.0f;
	}
	__xgeShapeExSortStops(pShape->pStrokeStops, iStopCount);
	pShape->iStrokeStopCount = iStopCount;
	pShape->iStrokeType = XGE_SHAPE_EX_FILL_RADIAL_GRADIENT;
	pShape->iStrokeColor = pShape->pStrokeStops[iStopCount - 1].iColor;
	pShape->fStrokeX1 = fCX;
	pShape->fStrokeY1 = fCY;
	pShape->fStrokeX2 = fCX + fRadius;
	pShape->fStrokeY2 = fCY;
	pShape->fStrokeR = fRadius;
	pShape->fStrokeFX = fFX;
	pShape->fStrokeFY = fFY;
	pShape->iStrokeGradientUnits = iUnits;
	pShape->iStrokeGradientSpread = XGE_SHAPE_EX_GRADIENT_SPREAD_PAD;
	pShape->tStrokeGradientTransform = __xgeShapeExMatrixIdentity();
	return XGE_OK;
}

int xgeShapeExStrokeGradientSpread(xge_shape_ex pShape, int iSpread)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iSpread != XGE_SHAPE_EX_GRADIENT_SPREAD_PAD) &&
	     (iSpread != XGE_SHAPE_EX_GRADIENT_SPREAD_REFLECT) &&
	     (iSpread != XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pShape->iStrokeGradientSpread = iSpread;
	return XGE_OK;
}

int xgeShapeExStrokeGradientTransformSet(xge_shape_ex pShape, const xge_shape_ex_matrix_t* pMatrix)
{
	if ( !__xgeShapeExValid(pShape) || (pMatrix == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pShape->tStrokeGradientTransform = *pMatrix;
	return XGE_OK;
}

int xgeShapeExStrokeGradientTransformIdentity(xge_shape_ex pShape)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pShape->tStrokeGradientTransform = __xgeShapeExMatrixIdentity();
	return XGE_OK;
}

int xgeShapeExStrokeWidth(xge_shape_ex pShape, float fWidth)
{
	if ( !__xgeShapeExValid(pShape) || (fWidth < 0.0f) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->fStrokeWidth = fWidth;
	return XGE_OK;
}

int xgeShapeExStrokeCap(xge_shape_ex pShape, int iCap)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( (iCap != XGE_SHAPE_EX_CAP_BUTT) && (iCap != XGE_SHAPE_EX_CAP_ROUND) && (iCap != XGE_SHAPE_EX_CAP_SQUARE) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->iLineCap = iCap;
	return XGE_OK;
}

int xgeShapeExStrokeJoin(xge_shape_ex pShape, int iJoin)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( (iJoin != XGE_SHAPE_EX_JOIN_MITER) && (iJoin != XGE_SHAPE_EX_JOIN_ROUND) && (iJoin != XGE_SHAPE_EX_JOIN_BEVEL) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->iLineJoin = iJoin;
	return XGE_OK;
}

int xgeShapeExStrokeMiterLimit(xge_shape_ex pShape, float fLimit)
{
	if ( !__xgeShapeExValid(pShape) || (fLimit < 0.0f) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->fMiterLimit = fLimit;
	return XGE_OK;
}

int xgeShapeExStrokeDash(xge_shape_ex pShape, const float* pDashPattern, int iDashCount, float fDashOffset)
{
	float* pCopy;

	if ( !__xgeShapeExValid(pShape) || (iDashCount < 0) || ((pDashPattern == NULL) && (iDashCount > 0)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xrtFree(pShape->pDashPattern);
	pShape->pDashPattern = NULL;
	pShape->iDashCount = 0;
	pShape->fDashOffset = fDashOffset;
	if ( iDashCount == 0 ) {
		return XGE_OK;
	}
	pCopy = (float*)xrtMalloc((size_t)iDashCount * sizeof(*pCopy));
	if ( pCopy == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(pCopy, pDashPattern, (size_t)iDashCount * sizeof(*pCopy));
	pShape->pDashPattern = pCopy;
	pShape->iDashCount = iDashCount;
	return XGE_OK;
}

int xgeShapeExFillRule(xge_shape_ex pShape, int iRule)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( (iRule != XGE_SHAPE_EX_FILL_NON_ZERO) && (iRule != XGE_SHAPE_EX_FILL_EVEN_ODD) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->iFillRule = iRule;
	return XGE_OK;
}

int xgeShapeExPaintOrder(xge_shape_ex pShape, int bStrokeFirst)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->bStrokeFirst = bStrokeFirst != 0;
	return XGE_OK;
}

int xgeShapeExOpacity(xge_shape_ex pShape, float fOpacity)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( fOpacity < 0.0f ) fOpacity = 0.0f;
	if ( fOpacity > 1.0f ) fOpacity = 1.0f;
	pShape->fOpacity = fOpacity;
	return XGE_OK;
}

int xgeShapeExVisible(xge_shape_ex pShape, int bVisible)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->bVisible = bVisible != 0;
	return XGE_OK;
}

int xgeShapeExGetBounds(xge_shape_ex pShape, float fTolerance, xge_rect_t* pBounds)
{
	xge_shape_ex_flat_path_t tFlat;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (pBounds == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( fTolerance <= 0.0f ) {
		fTolerance = XGE_SHAPE_EX_DEFAULT_TOLERANCE;
	}
	iRet = __xgeShapeExFlatten(pShape, __xgeShapeExMatrixIdentity(), fTolerance, &tFlat);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	*pBounds = __xgeShapeExFlatBounds(&tFlat);
	__xgeShapeExFlatFree(&tFlat);
	return XGE_OK;
}

int xgeShapeExClipRectSet(xge_shape_ex pShape, xge_rect_t tRect)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( tRect.fW < 0.0f ) tRect.fW = 0.0f;
	if ( tRect.fH < 0.0f ) tRect.fH = 0.0f;
	pShape->tClipRect = tRect;
	pShape->bClipRect = 1;
	return XGE_OK;
}

int xgeShapeExClipClear(xge_shape_ex pShape)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	memset(&pShape->tClipRect, 0, sizeof(pShape->tClipRect));
	pShape->bClipRect = 0;
	return XGE_OK;
}

int xgeShapeExTransformSet(xge_shape_ex pShape, const xge_shape_ex_matrix_t* pMatrix)
{
	if ( !__xgeShapeExValid(pShape) || (pMatrix == NULL) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->tTransform = *pMatrix;
	return XGE_OK;
}

int xgeShapeExTransformIdentity(xge_shape_ex pShape)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->tTransform = __xgeShapeExMatrixIdentity();
	return XGE_OK;
}

int xgeShapeExDraw(xge_shape_ex pShape, float fTolerance)
{
	return __xgeShapeExDrawInternal(pShape, fTolerance, 0, __xgeShapeExMatrixIdentity(), 1.0f);
}

int xgeShapeExDrawPx(xge_shape_ex pShape, float fTolerance)
{
	return __xgeShapeExDrawInternal(pShape, fTolerance, 1, __xgeShapeExMatrixIdentity(), 1.0f);
}

int xgeShapeExSceneCreate(xge_shape_ex_scene* ppScene)
{
	xge_shape_ex_scene pScene;

	if ( ppScene == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pScene = (xge_shape_ex_scene)xrtCalloc(1, sizeof(*pScene));
	if ( pScene == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pScene->iMagic = XGE_SHAPE_EX_SCENE_MAGIC;
	pScene->iRefCount = 1;
	pScene->fOpacity = 1.0f;
	pScene->bVisible = 1;
	pScene->tTransform = __xgeShapeExMatrixIdentity();
	*ppScene = pScene;
	return XGE_OK;
}

int xgeShapeExSceneAddRef(xge_shape_ex_scene pScene)
{
	if ( !__xgeShapeExSceneValid(pScene) ) return XGE_ERROR_INVALID_ARGUMENT;
	pScene->iRefCount++;
	return XGE_OK;
}

void xgeShapeExSceneDestroy(xge_shape_ex_scene pScene)
{
	int i;

	if ( !__xgeShapeExSceneValid(pScene) ) {
		return;
	}
	pScene->iRefCount--;
	if ( pScene->iRefCount > 0 ) {
		return;
	}
	for ( i = 0; i < pScene->iShapeCount; i++ ) {
		xgeShapeExDestroy(pScene->pShapes[i]);
	}
	pScene->iMagic = 0;
	xrtFree(pScene->pShapes);
	xrtFree(pScene);
}

int xgeShapeExSceneClear(xge_shape_ex_scene pScene)
{
	int i;

	if ( !__xgeShapeExSceneValid(pScene) ) return XGE_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pScene->iShapeCount; i++ ) {
		xgeShapeExDestroy(pScene->pShapes[i]);
	}
	pScene->iShapeCount = 0;
	return XGE_OK;
}

int xgeShapeExSceneAdd(xge_shape_ex_scene pScene, xge_shape_ex pShape)
{
	xge_shape_ex* pShapes;
	int iCapacity;

	if ( !__xgeShapeExSceneValid(pScene) || !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( pScene->iShapeCount >= pScene->iShapeCapacity ) {
		iCapacity = pScene->iShapeCapacity > 0 ? pScene->iShapeCapacity * 2 : 8;
		pShapes = (xge_shape_ex*)xrtRealloc(pScene->pShapes, (size_t)iCapacity * sizeof(*pShapes));
		if ( pShapes == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pScene->pShapes = pShapes;
		pScene->iShapeCapacity = iCapacity;
	}
	xgeShapeExAddRef(pShape);
	pScene->pShapes[pScene->iShapeCount++] = pShape;
	return XGE_OK;
}

int xgeShapeExSceneTransformSet(xge_shape_ex_scene pScene, const xge_shape_ex_matrix_t* pMatrix)
{
	if ( !__xgeShapeExSceneValid(pScene) || (pMatrix == NULL) ) return XGE_ERROR_INVALID_ARGUMENT;
	pScene->tTransform = *pMatrix;
	return XGE_OK;
}

int xgeShapeExSceneTransformIdentity(xge_shape_ex_scene pScene)
{
	if ( !__xgeShapeExSceneValid(pScene) ) return XGE_ERROR_INVALID_ARGUMENT;
	pScene->tTransform = __xgeShapeExMatrixIdentity();
	return XGE_OK;
}

int xgeShapeExSceneOpacity(xge_shape_ex_scene pScene, float fOpacity)
{
	if ( !__xgeShapeExSceneValid(pScene) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( fOpacity < 0.0f ) fOpacity = 0.0f;
	if ( fOpacity > 1.0f ) fOpacity = 1.0f;
	pScene->fOpacity = fOpacity;
	return XGE_OK;
}

int xgeShapeExSceneDraw(xge_shape_ex_scene pScene, float fTolerance)
{
	return __xgeShapeExSceneDrawInternal(pScene, fTolerance, 0, __xgeShapeExMatrixIdentity(), 1.0f);
}

int xgeShapeExSceneDrawPx(xge_shape_ex_scene pScene, float fTolerance)
{
	return __xgeShapeExSceneDrawInternal(pScene, fTolerance, 1, __xgeShapeExMatrixIdentity(), 1.0f);
}
