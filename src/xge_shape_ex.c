#include <float.h>
#include <limits.h>

#define XGE_SHAPE_EX_MAGIC 0x58475358u
#define XGE_SHAPE_EX_PATH_MEASURE_MAGIC 0x5847504Du
#define XGE_SHAPE_EX_SCENE_MAGIC 0x58475343u
#define XGE_SHAPE_EX_DEFAULT_TOLERANCE 0.35f
#define XGE_SHAPE_EX_KAPPA 0.5522847498307936f
#define XGE_SHAPE_EX_PI 3.14159265358979323846f
#define XGE_SHAPE_EX_TAU 6.28318530717958647692f
#define XGE_SHAPE_EX_EPSILON 0.00001f
#define XGE_SHAPE_EX_EXPLICIT_JOIN_ANGLE (XGE_SHAPE_EX_PI / 3.0f)
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
	int bStrokeNonScaling;
	float* pDashPattern;
	int iDashCount;
	float fDashOffset;
	int bTrimPath;
	float fTrimBegin;
	float fTrimEnd;
	int bTrimSimultaneous;
	int bStrokeFirst;
	int bStencilPaint;
	float fOpacity;
	int bVisible;
	int bClipRect;
	xge_rect_t tClipRect;
	xge_shape_ex* pClipShapes;
	int iClipShapeCount;
	int iClipShapeCapacity;
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

struct xge_shape_ex_path_measure_t {
	uint32_t iMagic;
	xge_shape_ex_flat_path_t tFlat;
	float fLength;
};

typedef struct xge_shape_ex_path_snapshot_t {
	int iCommandCount;
	int iPointCount;
	xge_vec2_t tCurrent;
	xge_vec2_t tStart;
	xge_vec2_t tLastCubicControl;
	xge_vec2_t tLastQuadControl;
	int bHasCurrent;
	int bHasStart;
} xge_shape_ex_path_snapshot_t;

static void __xgeShapeExPathSnapshot(xge_shape_ex pShape, xge_shape_ex_path_snapshot_t* pSnapshot)
{
	if ( (pShape == NULL) || (pSnapshot == NULL) ) {
		return;
	}
	pSnapshot->iCommandCount = pShape->iCommandCount;
	pSnapshot->iPointCount = pShape->iPointCount;
	pSnapshot->tCurrent = pShape->tCurrent;
	pSnapshot->tStart = pShape->tStart;
	pSnapshot->tLastCubicControl = pShape->tLastCubicControl;
	pSnapshot->tLastQuadControl = pShape->tLastQuadControl;
	pSnapshot->bHasCurrent = pShape->bHasCurrent;
	pSnapshot->bHasStart = pShape->bHasStart;
}

static void __xgeShapeExPathRestore(xge_shape_ex pShape, const xge_shape_ex_path_snapshot_t* pSnapshot)
{
	if ( (pShape == NULL) || (pSnapshot == NULL) ) {
		return;
	}
	pShape->iCommandCount = pSnapshot->iCommandCount;
	pShape->iPointCount = pSnapshot->iPointCount;
	pShape->tCurrent = pSnapshot->tCurrent;
	pShape->tStart = pSnapshot->tStart;
	pShape->tLastCubicControl = pSnapshot->tLastCubicControl;
	pShape->tLastQuadControl = pSnapshot->tLastQuadControl;
	pShape->bHasCurrent = pSnapshot->bHasCurrent;
	pShape->bHasStart = pSnapshot->bHasStart;
}

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

static xge_rect_t __xgeShapeExRectUnion(xge_rect_t a, xge_rect_t b)
{
	float fLeft = fminf(a.fX, b.fX);
	float fTop = fminf(a.fY, b.fY);
	float fRight = fmaxf(a.fX + a.fW, b.fX + b.fW);
	float fBottom = fmaxf(a.fY + a.fH, b.fY + b.fH);
	xge_rect_t tOut;

	tOut.fX = fLeft;
	tOut.fY = fTop;
	tOut.fW = fRight - fLeft;
	tOut.fH = fBottom - fTop;
	if ( tOut.fW < 0.0f ) tOut.fW = 0.0f;
	if ( tOut.fH < 0.0f ) tOut.fH = 0.0f;
	return tOut;
}

int xgeShapeExMatrixIdentity(xge_shape_ex_matrix_t* pMatrix)
{
	if ( pMatrix == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pMatrix = __xgeShapeExMatrixIdentity();
	return XGE_OK;
}

int xgeShapeExMatrixMultiply(xge_shape_ex_matrix_t* pOut, const xge_shape_ex_matrix_t* pParent, const xge_shape_ex_matrix_t* pLocal)
{
	if ( (pOut == NULL) || (pParent == NULL) || (pLocal == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pOut = __xgeShapeExMatrixMul(*pParent, *pLocal);
	return XGE_OK;
}

int xgeShapeExMatrixTranslate(xge_shape_ex_matrix_t* pMatrix, float fTX, float fTY)
{
	if ( pMatrix == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pMatrix->fA = 1.0f;
	pMatrix->fB = 0.0f;
	pMatrix->fC = 0.0f;
	pMatrix->fD = 1.0f;
	pMatrix->fE = fTX;
	pMatrix->fF = fTY;
	return XGE_OK;
}

int xgeShapeExMatrixScale(xge_shape_ex_matrix_t* pMatrix, float fSX, float fSY)
{
	if ( pMatrix == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pMatrix->fA = fSX;
	pMatrix->fB = 0.0f;
	pMatrix->fC = 0.0f;
	pMatrix->fD = fSY;
	pMatrix->fE = 0.0f;
	pMatrix->fF = 0.0f;
	return XGE_OK;
}

int xgeShapeExMatrixRotate(xge_shape_ex_matrix_t* pMatrix, float fRadians)
{
	float fS;
	float fC;

	if ( pMatrix == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fS = sinf(fRadians);
	fC = cosf(fRadians);
	pMatrix->fA = fC;
	pMatrix->fB = fS;
	pMatrix->fC = -fS;
	pMatrix->fD = fC;
	pMatrix->fE = 0.0f;
	pMatrix->fF = 0.0f;
	return XGE_OK;
}

int xgeShapeExMatrixSkew(xge_shape_ex_matrix_t* pMatrix, float fXRadians, float fYRadians)
{
	if ( pMatrix == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pMatrix->fA = 1.0f;
	pMatrix->fB = tanf(fYRadians);
	pMatrix->fC = tanf(fXRadians);
	pMatrix->fD = 1.0f;
	pMatrix->fE = 0.0f;
	pMatrix->fF = 0.0f;
	return XGE_OK;
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

int xgeShapeExMatrixInvert(xge_shape_ex_matrix_t* pOut, const xge_shape_ex_matrix_t* pMatrix)
{
	if ( (pOut == NULL) || (pMatrix == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeShapeExMatrixInverse(*pMatrix, pOut) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return XGE_OK;
}

int xgeShapeExMatrixPoint(xge_vec2_t* pOut, const xge_shape_ex_matrix_t* pMatrix, xge_vec2_t tPoint)
{
	if ( (pOut == NULL) || (pMatrix == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pOut = __xgeShapeExMatrixPoint(*pMatrix, tPoint);
	return XGE_OK;
}

int xgeShapeExMatrixVector(xge_vec2_t* pOut, const xge_shape_ex_matrix_t* pMatrix, xge_vec2_t tVector)
{
	if ( (pOut == NULL) || (pMatrix == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pOut->fX = (pMatrix->fA * tVector.fX) + (pMatrix->fC * tVector.fY);
	pOut->fY = (pMatrix->fB * tVector.fX) + (pMatrix->fD * tVector.fY);
	return XGE_OK;
}

int xgeShapeExMatrixRectBounds(xge_rect_t* pOut, const xge_shape_ex_matrix_t* pMatrix, xge_rect_t tRect)
{
	if ( (pOut == NULL) || (pMatrix == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pOut = __xgeShapeExMatrixRectBounds(*pMatrix, tRect);
	return XGE_OK;
}

static float __xgeShapeExMatrixStrokeScale(xge_shape_ex_matrix_t tMatrix)
{
	float fSX;
	float fSY;
	float fScale;

	fSX = sqrtf((tMatrix.fA * tMatrix.fA) + (tMatrix.fB * tMatrix.fB));
	fSY = sqrtf((tMatrix.fC * tMatrix.fC) + (tMatrix.fD * tMatrix.fD));
	fScale = sqrtf(fSX * fSY);
	if ( fScale <= XGE_SHAPE_EX_EPSILON ) {
		fScale = 1.0f;
	}
	return fScale;
}

int xgeShapeExMatrixStrokeScale(float* pScale, const xge_shape_ex_matrix_t* pMatrix)
{
	if ( (pScale == NULL) || (pMatrix == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pScale = __xgeShapeExMatrixStrokeScale(*pMatrix);
	return XGE_OK;
}

static int __xgeShapeExValid(xge_shape_ex pShape)
{
	return (pShape != NULL) && (pShape->iMagic == XGE_SHAPE_EX_MAGIC);
}

static int __xgeShapeExPathMeasureValid(xge_shape_ex_path_measure pMeasure)
{
	return (pMeasure != NULL) && (pMeasure->iMagic == XGE_SHAPE_EX_PATH_MEASURE_MAGIC);
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

static int __xgeShapeExCommandPointCount(uint8_t iCommand)
{
	switch ( iCommand ) {
		case XGE_SHAPE_EX_CMD_CLOSE:
			return 0;
		case XGE_SHAPE_EX_CMD_MOVE_TO:
		case XGE_SHAPE_EX_CMD_LINE_TO:
			return 1;
		case XGE_SHAPE_EX_CMD_CUBIC_TO:
			return 3;
		default:
			return -1;
	}
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

static void __xgeShapeExClipShapesClearInternal(xge_shape_ex pShape)
{
	int i;

	if ( pShape == NULL ) {
		return;
	}
	for ( i = 0; i < pShape->iClipShapeCount; i++ ) {
		xgeShapeExDestroy(pShape->pClipShapes[i]);
	}
	pShape->iClipShapeCount = 0;
}

static int __xgeShapeExReserveClipShapes(xge_shape_ex pShape, int iNeeded)
{
	xge_shape_ex* pShapes;
	int iCapacity;

	if ( !__xgeShapeExValid(pShape) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pShape->iClipShapeCapacity ) {
		return XGE_OK;
	}
	iCapacity = pShape->iClipShapeCapacity > 0 ? pShape->iClipShapeCapacity : 4;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pShapes = (xge_shape_ex*)xrtRealloc(pShape->pClipShapes, (size_t)iCapacity * sizeof(*pShapes));
	if ( pShapes == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pShape->pClipShapes = pShapes;
	pShape->iClipShapeCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeShapeExClipShapeAddRef(xge_shape_ex pShape, xge_shape_ex pClipShape)
{
	int iRet;

	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExValid(pClipShape) || (pShape == pClipShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeShapeExReserveClipShapes(pShape, pShape->iClipShapeCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeShapeExAddRef(pClipShape);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pShape->pClipShapes[pShape->iClipShapeCount++] = pClipShape;
	return XGE_OK;
}

static void __xgeShapeExNormalizeStops(xge_shape_ex_color_stop_t* pStops, int iCount)
{
	int i;
	float fPrev;

	if ( (pStops == NULL) || (iCount <= 0) ) {
		return;
	}
	fPrev = 0.0f;
	for ( i = 0; i < iCount; i++ ) {
		if ( pStops[i].fOffset < fPrev ) {
			pStops[i].fOffset = fPrev;
		} else if ( pStops[i].fOffset > 1.0f ) {
			pStops[i].fOffset = 1.0f;
		}
		fPrev = pStops[i].fOffset;
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
			if ( pContour->iCount != 1 ) {
				return XGE_OK;
			}
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

static int __xgeShapeExFlatCopy(const xge_shape_ex_flat_path_t* pSrc, xge_shape_ex_flat_path_t* pDst)
{
	if ( (pSrc == NULL) || (pDst == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pDst, 0, sizeof(*pDst));
	if ( pSrc->iPointCount > 0 ) {
		pDst->pPoints = (xge_vec2_t*)xrtMalloc((size_t)pSrc->iPointCount * sizeof(*pDst->pPoints));
		if ( pDst->pPoints == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memcpy(pDst->pPoints, pSrc->pPoints, (size_t)pSrc->iPointCount * sizeof(*pDst->pPoints));
		pDst->iPointCount = pSrc->iPointCount;
		pDst->iPointCapacity = pSrc->iPointCount;
	}
	if ( pSrc->iContourCount > 0 ) {
		pDst->pContours = (xge_shape_ex_flat_contour_t*)xrtMalloc((size_t)pSrc->iContourCount * sizeof(*pDst->pContours));
		if ( pDst->pContours == NULL ) {
			__xgeShapeExFlatFree(pDst);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memcpy(pDst->pContours, pSrc->pContours, (size_t)pSrc->iContourCount * sizeof(*pDst->pContours));
		pDst->iContourCount = pSrc->iContourCount;
		pDst->iContourCapacity = pSrc->iContourCount;
	}
	return XGE_OK;
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
		} else if ( iCommand == XGE_SHAPE_EX_CMD_CLOSE ) {
			if ( !bHasCurrent || !bHasStart ) {
				continue;
			}
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

static float __xgeShapeExFlatLength(const xge_shape_ex_flat_path_t* pFlat)
{
	float fLength;
	int i;

	if ( pFlat == NULL ) {
		return 0.0f;
	}
	fLength = 0.0f;
	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		const xge_shape_ex_flat_contour_t* pContour = &pFlat->pContours[i];
		const xge_vec2_t* pPoints;
		int j;

		if ( (pContour->iCount <= 1) || (pContour->iStart < 0) || ((pContour->iStart + pContour->iCount) > pFlat->iPointCount) ) {
			continue;
		}
		pPoints = pFlat->pPoints + pContour->iStart;
		for ( j = 1; j < pContour->iCount; j++ ) {
			fLength += hypotf(pPoints[j].fX - pPoints[j - 1].fX, pPoints[j].fY - pPoints[j - 1].fY);
		}
		if ( pContour->bClosed ) {
			fLength += hypotf(pPoints[0].fX - pPoints[pContour->iCount - 1].fX, pPoints[0].fY - pPoints[pContour->iCount - 1].fY);
		}
	}
	return fLength;
}

static void __xgeShapeExSetSampleResult(xge_vec2_t tPoint, xge_vec2_t tTangent, xge_vec2_t* pPoint, xge_vec2_t* pTangent)
{
	float fLen;

	if ( pPoint != NULL ) {
		*pPoint = tPoint;
	}
	if ( pTangent == NULL ) {
		return;
	}
	fLen = hypotf(tTangent.fX, tTangent.fY);
	if ( fLen <= XGE_SHAPE_EX_EPSILON ) {
		pTangent->fX = 1.0f;
		pTangent->fY = 0.0f;
		return;
	}
	pTangent->fX = tTangent.fX / fLen;
	pTangent->fY = tTangent.fY / fLen;
}

static int __xgeShapeExFlatPointAtLength(const xge_shape_ex_flat_path_t* pFlat, float fDistance, xge_vec2_t* pPoint, xge_vec2_t* pTangent)
{
	xge_vec2_t tFirst;
	xge_vec2_t tLast;
	xge_vec2_t tLastTangent;
	float fRemaining;
	int bHaveFirst;
	int bHaveLast;
	int i;

	if ( pFlat == NULL ) {
		return 0;
	}
	if ( fDistance < 0.0f ) {
		fDistance = 0.0f;
	}
	memset(&tFirst, 0, sizeof(tFirst));
	memset(&tLast, 0, sizeof(tLast));
	tLastTangent.fX = 1.0f;
	tLastTangent.fY = 0.0f;
	fRemaining = fDistance;
	bHaveFirst = 0;
	bHaveLast = 0;
	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		const xge_shape_ex_flat_contour_t* pContour = &pFlat->pContours[i];
		const xge_vec2_t* pPoints;
		int j;

		if ( (pContour->iCount <= 0) || (pContour->iStart < 0) || ((pContour->iStart + pContour->iCount) > pFlat->iPointCount) ) {
			continue;
		}
		pPoints = pFlat->pPoints + pContour->iStart;
		if ( !bHaveFirst ) {
			tFirst = pPoints[0];
			tLast = tFirst;
			bHaveFirst = 1;
		}
		if ( pContour->iCount <= 1 ) {
			continue;
		}
		for ( j = 1; j < pContour->iCount; j++ ) {
			xge_vec2_t tA = pPoints[j - 1];
			xge_vec2_t tB = pPoints[j];
			xge_vec2_t tDelta;
			float fSegment;

			tDelta.fX = tB.fX - tA.fX;
			tDelta.fY = tB.fY - tA.fY;
			fSegment = hypotf(tDelta.fX, tDelta.fY);
			if ( fSegment <= XGE_SHAPE_EX_EPSILON ) {
				continue;
			}
			if ( fRemaining <= fSegment ) {
				float fT = fRemaining / fSegment;
				xge_vec2_t tPoint;

				tPoint.fX = tA.fX + tDelta.fX * fT;
				tPoint.fY = tA.fY + tDelta.fY * fT;
				__xgeShapeExSetSampleResult(tPoint, tDelta, pPoint, pTangent);
				return 1;
			}
			fRemaining -= fSegment;
			tLast = tB;
			tLastTangent = tDelta;
			bHaveLast = 1;
		}
		if ( pContour->bClosed ) {
			xge_vec2_t tA = pPoints[pContour->iCount - 1];
			xge_vec2_t tB = pPoints[0];
			xge_vec2_t tDelta;
			float fSegment;

			tDelta.fX = tB.fX - tA.fX;
			tDelta.fY = tB.fY - tA.fY;
			fSegment = hypotf(tDelta.fX, tDelta.fY);
			if ( fSegment > XGE_SHAPE_EX_EPSILON ) {
				if ( fRemaining <= fSegment ) {
					float fT = fRemaining / fSegment;
					xge_vec2_t tPoint;

					tPoint.fX = tA.fX + tDelta.fX * fT;
					tPoint.fY = tA.fY + tDelta.fY * fT;
					__xgeShapeExSetSampleResult(tPoint, tDelta, pPoint, pTangent);
					return 1;
				}
				fRemaining -= fSegment;
				tLast = tB;
				tLastTangent = tDelta;
				bHaveLast = 1;
			}
		}
	}
	if ( bHaveLast ) {
		__xgeShapeExSetSampleResult(tLast, tLastTangent, pPoint, pTangent);
		return 1;
	}
	if ( bHaveFirst ) {
		xge_vec2_t tTangent;

		tTangent.fX = 1.0f;
		tTangent.fY = 0.0f;
		__xgeShapeExSetSampleResult(tFirst, tTangent, pPoint, pTangent);
		return 1;
	}
	return 0;
}

static xge_vec2_t __xgeShapeExLerpPoint(xge_vec2_t tA, xge_vec2_t tB, float fT)
{
	xge_vec2_t tOut;

	if ( fT < 0.0f ) fT = 0.0f;
	if ( fT > 1.0f ) fT = 1.0f;
	tOut.fX = tA.fX + (tB.fX - tA.fX) * fT;
	tOut.fY = tA.fY + (tB.fY - tA.fY) * fT;
	return tOut;
}

static float __xgeShapeExWrap01(float fValue)
{
	float fOut = fmodf(fValue, 1.0f);

	if ( fOut < 0.0f ) {
		fOut += 1.0f;
	}
	if ( fOut >= 1.0f ) {
		fOut -= 1.0f;
	}
	return fOut;
}

static int __xgeShapeExTrimRangeFractions(float fBegin, float fEnd, float pStarts[2], float pEnds[2], int* pCount)
{
	float fSpan;
	float fStart;
	float fStop;

	if ( (pStarts == NULL) || (pEnds == NULL) || (pCount == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pCount = 0;
	fSpan = fEnd - fBegin;
	if ( fabsf(fSpan) <= XGE_SHAPE_EX_EPSILON ) {
		return XGE_OK;
	}
	if ( fabsf(fSpan) >= (1.0f - XGE_SHAPE_EX_EPSILON) ) {
		pStarts[0] = 0.0f;
		pEnds[0] = 1.0f;
		*pCount = 1;
		return XGE_OK;
	}
	fStart = __xgeShapeExWrap01(fBegin);
	fStop = __xgeShapeExWrap01(fEnd);
	if ( fabsf(fStart - fStop) <= XGE_SHAPE_EX_EPSILON ) {
		return XGE_OK;
	}
	if ( fStop > fStart ) {
		pStarts[0] = fStart;
		pEnds[0] = fStop;
		*pCount = 1;
		return XGE_OK;
	}
	pStarts[0] = fStart;
	pEnds[0] = 1.0f;
	pStarts[1] = 0.0f;
	pEnds[1] = fStop;
	*pCount = 2;
	return XGE_OK;
}

static float __xgeShapeExFlatContourLength(const xge_shape_ex_flat_path_t* pFlat, const xge_shape_ex_flat_contour_t* pContour)
{
	const xge_vec2_t* pPoints;
	float fLength;
	int iSegmentCount;
	int i;

	if ( (pFlat == NULL) || (pContour == NULL) || (pContour->iCount <= 1) ||
	     (pContour->iStart < 0) || ((pContour->iStart + pContour->iCount) > pFlat->iPointCount) ) {
		return 0.0f;
	}
	pPoints = pFlat->pPoints + pContour->iStart;
	iSegmentCount = pContour->bClosed ? pContour->iCount : (pContour->iCount - 1);
	fLength = 0.0f;
	for ( i = 0; i < iSegmentCount; i++ ) {
		xge_vec2_t tA = pPoints[i];
		xge_vec2_t tB = pPoints[(i + 1) % pContour->iCount];

		fLength += hypotf(tB.fX - tA.fX, tB.fY - tA.fY);
	}
	return fLength;
}

static int __xgeShapeExFlatAddTrimmedContourRange(const xge_shape_ex_flat_path_t* pFlat, int iContour, float fStartDistance, float fEndDistance, xge_shape_ex_flat_path_t* pOut)
{
	const xge_shape_ex_flat_contour_t* pContour;
	const xge_vec2_t* pPoints;
	float fPos;
	int iSegmentCount;
	int bStarted;
	int i;

	if ( (pFlat == NULL) || (pOut == NULL) || (iContour < 0) || (iContour >= pFlat->iContourCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( fEndDistance <= fStartDistance + XGE_SHAPE_EX_EPSILON ) {
		return XGE_OK;
	}
	pContour = &pFlat->pContours[iContour];
	if ( (pContour->iCount <= 1) || (pContour->iStart < 0) || ((pContour->iStart + pContour->iCount) > pFlat->iPointCount) ) {
		return XGE_OK;
	}
	pPoints = pFlat->pPoints + pContour->iStart;
	iSegmentCount = pContour->bClosed ? pContour->iCount : (pContour->iCount - 1);
	fPos = 0.0f;
	bStarted = 0;
	for ( i = 0; i < iSegmentCount; i++ ) {
		xge_vec2_t tA = pPoints[i];
		xge_vec2_t tB = pPoints[(i + 1) % pContour->iCount];
		float fSegment = hypotf(tB.fX - tA.fX, tB.fY - tA.fY);
		float fSegmentEnd;
		float fOverlapStart;
		float fOverlapEnd;
		xge_vec2_t tStart;
		xge_vec2_t tEnd;
		int iRet;

		if ( fSegment <= XGE_SHAPE_EX_EPSILON ) {
			continue;
		}
		fSegmentEnd = fPos + fSegment;
		if ( fSegmentEnd <= fStartDistance + XGE_SHAPE_EX_EPSILON ) {
			fPos = fSegmentEnd;
			continue;
		}
		if ( fPos >= fEndDistance - XGE_SHAPE_EX_EPSILON ) {
			break;
		}
		fOverlapStart = fmaxf(fStartDistance, fPos);
		fOverlapEnd = fminf(fEndDistance, fSegmentEnd);
		if ( fOverlapEnd <= fOverlapStart + XGE_SHAPE_EX_EPSILON ) {
			fPos = fSegmentEnd;
			continue;
		}
		tStart = __xgeShapeExLerpPoint(tA, tB, (fOverlapStart - fPos) / fSegment);
		tEnd = __xgeShapeExLerpPoint(tA, tB, (fOverlapEnd - fPos) / fSegment);
		if ( !bStarted ) {
			iRet = __xgeShapeExFlatBeginContour(pOut, tStart);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			bStarted = 1;
		}
		iRet = __xgeShapeExFlatAddPoint(pOut, tEnd);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		fPos = fSegmentEnd;
	}
	return XGE_OK;
}

static int __xgeShapeExFlatTrimSimultaneous(const xge_shape_ex_flat_path_t* pFlat, const float* pStarts, const float* pEnds, int iRangeCount, xge_shape_ex_flat_path_t* pOut)
{
	int i;

	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		const xge_shape_ex_flat_contour_t* pContour = &pFlat->pContours[i];
		float fLength = __xgeShapeExFlatContourLength(pFlat, pContour);
		int j;

		if ( fLength <= XGE_SHAPE_EX_EPSILON ) {
			continue;
		}
		for ( j = 0; j < iRangeCount; j++ ) {
			int iRet = __xgeShapeExFlatAddTrimmedContourRange(pFlat, i, pStarts[j] * fLength, pEnds[j] * fLength, pOut);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
	}
	return XGE_OK;
}

static int __xgeShapeExFlatTrimCombined(const xge_shape_ex_flat_path_t* pFlat, const float* pStarts, const float* pEnds, int iRangeCount, xge_shape_ex_flat_path_t* pOut)
{
	float fTotal;
	int iRange;

	fTotal = __xgeShapeExFlatLength(pFlat);
	if ( fTotal <= XGE_SHAPE_EX_EPSILON ) {
		return XGE_OK;
	}
	for ( iRange = 0; iRange < iRangeCount; iRange++ ) {
		float fRangeStart = pStarts[iRange] * fTotal;
		float fRangeEnd = pEnds[iRange] * fTotal;
		float fPos = 0.0f;
		int i;

		for ( i = 0; i < pFlat->iContourCount; i++ ) {
			const xge_shape_ex_flat_contour_t* pContour = &pFlat->pContours[i];
			float fContourLength = __xgeShapeExFlatContourLength(pFlat, pContour);
			float fContourEnd = fPos + fContourLength;
			float fLocalStart;
			float fLocalEnd;
			int iRet;

			if ( fContourLength <= XGE_SHAPE_EX_EPSILON ) {
				continue;
			}
			if ( fContourEnd <= fRangeStart + XGE_SHAPE_EX_EPSILON ) {
				fPos = fContourEnd;
				continue;
			}
			if ( fPos >= fRangeEnd - XGE_SHAPE_EX_EPSILON ) {
				break;
			}
			fLocalStart = fmaxf(fRangeStart, fPos) - fPos;
			fLocalEnd = fminf(fRangeEnd, fContourEnd) - fPos;
			iRet = __xgeShapeExFlatAddTrimmedContourRange(pFlat, i, fLocalStart, fLocalEnd, pOut);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			fPos = fContourEnd;
		}
	}
	return XGE_OK;
}

static int __xgeShapeExFlatTrim(const xge_shape_ex_flat_path_t* pFlat, float fBegin, float fEnd, int bSimultaneous, xge_shape_ex_flat_path_t* pOut)
{
	float arrStarts[2];
	float arrEnds[2];
	int iRangeCount;
	int iRet;

	if ( (pFlat == NULL) || (pOut == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pOut, 0, sizeof(*pOut));
	iRet = __xgeShapeExTrimRangeFractions(fBegin, fEnd, arrStarts, arrEnds, &iRangeCount);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( iRangeCount <= 0 ) {
		return XGE_OK;
	}
	if ( (iRangeCount == 1) && (arrStarts[0] <= XGE_SHAPE_EX_EPSILON) && (arrEnds[0] >= 1.0f - XGE_SHAPE_EX_EPSILON) ) {
		return __xgeShapeExFlatCopy(pFlat, pOut);
	}
	iRet = bSimultaneous ? __xgeShapeExFlatTrimSimultaneous(pFlat, arrStarts, arrEnds, iRangeCount, pOut)
	                     : __xgeShapeExFlatTrimCombined(pFlat, arrStarts, arrEnds, iRangeCount, pOut);
	if ( iRet != XGE_OK ) {
		__xgeShapeExFlatFree(pOut);
	}
	return iRet;
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

	if ( (pShape == NULL) || (fHalfWidth <= XGE_SHAPE_EX_EPSILON) ) {
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

static int __xgeShapeExDrawStrokeZeroLengthCap(xge_shape_ex pShape, xge_vec2_t tCenter, float fWidth, uint32_t iColor, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, float fOpacity, int bScreenSpace)
{
	float fHalfWidth;
	int i;

	if ( (pShape == NULL) || (fWidth <= XGE_SHAPE_EX_EPSILON) ) {
		return XGE_OK;
	}
	if ( pShape->iLineCap == XGE_SHAPE_EX_CAP_BUTT ) {
		return XGE_OK;
	}
	fHalfWidth = fWidth * 0.5f;
	if ( pShape->iLineCap == XGE_SHAPE_EX_CAP_ROUND ) {
		xge_shape_vertex_t tVertices[26];
		uint32_t iIndices[72];
		int iSegments = 24;

		tVertices[0].fX = tCenter.fX;
		tVertices[0].fY = tCenter.fY;
		tVertices[0].iColor = iColor;
		for ( i = 0; i <= iSegments; i++ ) {
			float fA = ((float)i / (float)iSegments) * XGE_SHAPE_EX_TAU;
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
	{
		xge_shape_vertex_t tVertices[4];
		uint32_t iIndices[6];

		tVertices[0].fX = tCenter.fX - fHalfWidth; tVertices[0].fY = tCenter.fY - fHalfWidth; tVertices[0].iColor = iColor;
		tVertices[1].fX = tCenter.fX + fHalfWidth; tVertices[1].fY = tCenter.fY - fHalfWidth; tVertices[1].iColor = iColor;
		tVertices[2].fX = tCenter.fX + fHalfWidth; tVertices[2].fY = tCenter.fY + fHalfWidth; tVertices[2].iColor = iColor;
		tVertices[3].fX = tCenter.fX - fHalfWidth; tVertices[3].fY = tCenter.fY + fHalfWidth; tVertices[3].iColor = iColor;
		iIndices[0] = 0; iIndices[1] = 1; iIndices[2] = 2;
		iIndices[3] = 0; iIndices[4] = 2; iIndices[5] = 3;
		return __xgeShapeExDrawStrokePaintMesh(pShape, tVertices, 4, iIndices, 6, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
	}
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

	if ( (pShape == NULL) || (fWidth <= XGE_SHAPE_EX_EPSILON) ) {
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
		tOuter1.fX = -tN1.fX;
		tOuter1.fY = -tN1.fY;
		tOuter2.fX = -tN2.fX;
		tOuter2.fY = -tN2.fY;
	} else {
		tOuter1 = tN1;
		tOuter2 = tN2;
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
	if ( pShape != NULL ) {
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
			if ( !bCapStart && !bCapEnd ) {
				return XGE_OK;
			}
			return __xgeShapeExDrawStrokeZeroLengthCap(pShape, tA, fWidth, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
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

static int __xgeShapeExCompactStrokePoints(const xge_vec2_t* pSrc, int iCount, int bClosed, xge_vec2_t** ppPoints, int* pCount)
{
	xge_vec2_t* pPoints;
	int iWrite;
	int i;

	if ( (pSrc == NULL) || (ppPoints == NULL) || (pCount == NULL) || (iCount <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppPoints = NULL;
	*pCount = 0;
	pPoints = (xge_vec2_t*)xrtMalloc((size_t)iCount * sizeof(*pPoints));
	if ( pPoints == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iWrite = 0;
	for ( i = 0; i < iCount; i++ ) {
		if ( (iWrite > 0) && __xgeShapeExSamePoint(pPoints[iWrite - 1], pSrc[i]) ) {
			continue;
		}
		pPoints[iWrite++] = pSrc[i];
	}
	if ( bClosed ) {
		while ( (iWrite > 1) && __xgeShapeExSamePoint(pPoints[iWrite - 1], pPoints[0]) ) {
			iWrite--;
		}
	}
	if ( iWrite <= 0 ) {
		xrtFree(pPoints);
		return XGE_OK;
	}
	*ppPoints = pPoints;
	*pCount = iWrite;
	return XGE_OK;
}

static int __xgeShapeExNormalizeSegment(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t* pDir)
{
	float fDX;
	float fDY;
	float fLen;

	if ( pDir == NULL ) {
		return 0;
	}
	fDX = tB.fX - tA.fX;
	fDY = tB.fY - tA.fY;
	fLen = hypotf(fDX, fDY);
	if ( fLen <= XGE_SHAPE_EX_EPSILON ) {
		pDir->fX = 0.0f;
		pDir->fY = 0.0f;
		return 0;
	}
	pDir->fX = fDX / fLen;
	pDir->fY = fDY / fLen;
	return 1;
}

static xge_vec2_t __xgeShapeExStrokeSidePoint(xge_vec2_t tPrev, xge_vec2_t tCurr, xge_vec2_t tNext, float fHalfWidth, float fMiterLimit, int iSide)
{
	xge_vec2_t tD1;
	xge_vec2_t tD2;
	xge_vec2_t tN1;
	xge_vec2_t tN2;
	xge_vec2_t tP1;
	xge_vec2_t tP2;
	xge_vec2_t tOut;
	float fMaxMiter;

	if ( !__xgeShapeExNormalizeSegment(tPrev, tCurr, &tD1) ||
	     !__xgeShapeExNormalizeSegment(tCurr, tNext, &tD2) ) {
		tOut = tCurr;
		return tOut;
	}
	tN1.fX = -tD1.fY * fHalfWidth * (float)iSide;
	tN1.fY =  tD1.fX * fHalfWidth * (float)iSide;
	tN2.fX = -tD2.fY * fHalfWidth * (float)iSide;
	tN2.fY =  tD2.fX * fHalfWidth * (float)iSide;
	tP1.fX = tCurr.fX + tN1.fX;
	tP1.fY = tCurr.fY + tN1.fY;
	tP2.fX = tCurr.fX + tN2.fX;
	tP2.fY = tCurr.fY + tN2.fY;
	fMaxMiter = fHalfWidth * fMiterLimit;
	if ( fMaxMiter < fHalfWidth ) {
		fMaxMiter = fHalfWidth;
	}
	if ( __xgeShapeExLineIntersection(tP1, tD1, tP2, tD2, &tOut) ) {
		float fDX = tOut.fX - tCurr.fX;
		float fDY = tOut.fY - tCurr.fY;
		float fLen = hypotf(fDX, fDY);

		if ( (fLen > XGE_SHAPE_EX_EPSILON) && (fLen > fMaxMiter) ) {
			tOut.fX = tCurr.fX + (fDX / fLen) * fMaxMiter;
			tOut.fY = tCurr.fY + (fDY / fLen) * fMaxMiter;
		}
		return tOut;
	}
	tOut.fX = tN1.fX + tN2.fX;
	tOut.fY = tN1.fY + tN2.fY;
	if ( hypotf(tOut.fX, tOut.fY) <= XGE_SHAPE_EX_EPSILON ) {
		tOut = tN2;
	} else {
		float fLen = hypotf(tOut.fX, tOut.fY);

		tOut.fX = (tOut.fX / fLen) * fHalfWidth;
		tOut.fY = (tOut.fY / fLen) * fHalfWidth;
	}
	tOut.fX += tCurr.fX;
	tOut.fY += tCurr.fY;
	return tOut;
}

static void __xgeShapeExStrokeEndpointPair(xge_vec2_t tPoint, xge_vec2_t tOther, float fHalfWidth, int iCap, int bEnd, xge_vec2_t* pLeft, xge_vec2_t* pRight)
{
	xge_vec2_t tDir;
	xge_vec2_t tBase;
	float fNX;
	float fNY;

	tBase = tPoint;
	if ( !__xgeShapeExNormalizeSegment(bEnd ? tOther : tPoint, bEnd ? tPoint : tOther, &tDir) ) {
		tDir.fX = 1.0f;
		tDir.fY = 0.0f;
	}
	if ( iCap == XGE_SHAPE_EX_CAP_SQUARE ) {
		if ( bEnd ) {
			tBase.fX += tDir.fX * fHalfWidth;
			tBase.fY += tDir.fY * fHalfWidth;
		} else {
			tBase.fX -= tDir.fX * fHalfWidth;
			tBase.fY -= tDir.fY * fHalfWidth;
		}
	}
	fNX = -tDir.fY * fHalfWidth;
	fNY =  tDir.fX * fHalfWidth;
	pLeft->fX = tBase.fX + fNX;
	pLeft->fY = tBase.fY + fNY;
	pRight->fX = tBase.fX - fNX;
	pRight->fY = tBase.fY - fNY;
}

static int __xgeShapeExDrawStrokeContourMesh(xge_shape_ex pShape, const xge_vec2_t* pSrc, int iCount, int bClosed, float fWidth, uint32_t iColor, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, float fOpacity, int bScreenSpace)
{
	xge_shape_vertex_t* pVertices;
	uint32_t* pIndices;
	xge_vec2_t* pPoints;
	float fHalfWidth;
	float fMiterLimit;
	int iPointCount;
	int iSegmentCount;
	int iIndexCount;
	int iRet;
	int i;

	if ( (pShape == NULL) || (pSrc == NULL) || (iCount < 2) || (fWidth <= XGE_SHAPE_EX_EPSILON) ) {
		return XGE_OK;
	}
	pVertices = NULL;
	pIndices = NULL;
	pPoints = NULL;
	iRet = __xgeShapeExCompactStrokePoints(pSrc, iCount, bClosed, &pPoints, &iPointCount);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( iPointCount < 2 ) {
		xge_vec2_t tPoint;

		if ( iPointCount == 1 ) {
			tPoint = pPoints[0];
			xrtFree(pPoints);
			return __xgeShapeExDrawStrokeZeroLengthCap(pShape, tPoint, fWidth, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
		}
		xrtFree(pPoints);
		return XGE_OK;
	}
	iSegmentCount = bClosed ? iPointCount : (iPointCount - 1);
	if ( iSegmentCount <= 0 ) {
		xrtFree(pPoints);
		return XGE_OK;
	}
	iIndexCount = iSegmentCount * 6;
	pVertices = (xge_shape_vertex_t*)xrtMalloc((size_t)iPointCount * 2u * sizeof(*pVertices));
	pIndices = (uint32_t*)xrtMalloc((size_t)iIndexCount * sizeof(*pIndices));
	if ( (pVertices == NULL) || (pIndices == NULL) ) {
		xrtFree(pVertices);
		xrtFree(pIndices);
		xrtFree(pPoints);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	fHalfWidth = fWidth * 0.5f;
	fMiterLimit = pShape->fMiterLimit;
	if ( pShape->iLineJoin != XGE_SHAPE_EX_JOIN_MITER ) {
		fMiterLimit = 1.0f;
	}
	if ( fMiterLimit < 1.0f ) {
		fMiterLimit = 1.0f;
	}
	for ( i = 0; i < iPointCount; i++ ) {
		xge_vec2_t tLeft;
		xge_vec2_t tRight;

		if ( !bClosed && (i == 0) ) {
			__xgeShapeExStrokeEndpointPair(pPoints[0], pPoints[1], fHalfWidth, pShape->iLineCap, 0, &tLeft, &tRight);
		} else if ( !bClosed && (i == (iPointCount - 1)) ) {
			__xgeShapeExStrokeEndpointPair(pPoints[iPointCount - 1], pPoints[iPointCount - 2], fHalfWidth, pShape->iLineCap, 1, &tLeft, &tRight);
		} else {
			int iPrev = (i + iPointCount - 1) % iPointCount;
			int iNext = (i + 1) % iPointCount;

			tLeft = __xgeShapeExStrokeSidePoint(pPoints[iPrev], pPoints[i], pPoints[iNext], fHalfWidth, fMiterLimit, 1);
			tRight = __xgeShapeExStrokeSidePoint(pPoints[iPrev], pPoints[i], pPoints[iNext], fHalfWidth, fMiterLimit, -1);
		}
		pVertices[i * 2 + 0].fX = tLeft.fX;
		pVertices[i * 2 + 0].fY = tLeft.fY;
		pVertices[i * 2 + 0].iColor = iColor;
		pVertices[i * 2 + 1].fX = tRight.fX;
		pVertices[i * 2 + 1].fY = tRight.fY;
		pVertices[i * 2 + 1].iColor = iColor;
	}
	for ( i = 0; i < iSegmentCount; i++ ) {
		uint32_t iA = (uint32_t)(i * 2);
		uint32_t iB = (uint32_t)(((i + 1) % iPointCount) * 2);
		int iIndex = i * 6;

		pIndices[iIndex + 0] = iA;
		pIndices[iIndex + 1] = iB;
		pIndices[iIndex + 2] = iB + 1;
		pIndices[iIndex + 3] = iA;
		pIndices[iIndex + 4] = iB + 1;
		pIndices[iIndex + 5] = iA + 1;
	}
	iRet = __xgeShapeExDrawStrokePaintMesh(pShape, pVertices, iPointCount * 2, pIndices, iIndexCount, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
	if ( (iRet == XGE_OK) && !bClosed && (pShape->iLineCap == XGE_SHAPE_EX_CAP_ROUND) ) {
		xge_vec2_t tDir;

		if ( __xgeShapeExNormalizeSegment(pPoints[0], pPoints[1], &tDir) ) {
			iRet = __xgeShapeExDrawStrokeRoundCap(pShape, pPoints[0], tDir.fX, tDir.fY, fHalfWidth, 0, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
		}
		if ( (iRet == XGE_OK) && __xgeShapeExNormalizeSegment(pPoints[iPointCount - 2], pPoints[iPointCount - 1], &tDir) ) {
			iRet = __xgeShapeExDrawStrokeRoundCap(pShape, pPoints[iPointCount - 1], tDir.fX, tDir.fY, fHalfWidth, 1, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
		}
	}
	xrtFree(pVertices);
	xrtFree(pIndices);
	xrtFree(pPoints);
	return iRet;
}

static int __xgeShapeExDrawStrokeSegmentJoinContour(xge_shape_ex pShape, const xge_vec2_t* pSrc, int iCount, int bClosed, float fWidth, uint32_t iColor, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, float fOpacity, int bScreenSpace)
{
	xge_vec2_t* pPoints;
	int iPointCount;
	int iSegmentCount;
	int iRet;
	int i;

	if ( (pShape == NULL) || (pSrc == NULL) || (iCount < 2) || (fWidth <= XGE_SHAPE_EX_EPSILON) ) {
		return XGE_OK;
	}
	pPoints = NULL;
	iPointCount = 0;
	iRet = __xgeShapeExCompactStrokePoints(pSrc, iCount, bClosed, &pPoints, &iPointCount);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( iPointCount < 2 ) {
		xge_vec2_t tPoint;

		if ( iPointCount == 1 ) {
			tPoint = pPoints[0];
			xrtFree(pPoints);
			return __xgeShapeExDrawStrokeZeroLengthCap(pShape, tPoint, fWidth, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
		}
		xrtFree(pPoints);
		return XGE_OK;
	}
	iSegmentCount = bClosed ? iPointCount : (iPointCount - 1);
	for ( i = 0; i < iSegmentCount; i++ ) {
		xge_vec2_t tA = pPoints[i];
		xge_vec2_t tB = pPoints[(i + 1) % iPointCount];
		int bCapStart = (!bClosed && (i == 0));
		int bCapEnd = (!bClosed && (i == (iSegmentCount - 1)));

		iRet = __xgeShapeExDrawStrokeSegment(pShape, tA, tB, fWidth, iColor, bCapStart, bCapEnd, tBounds, tMatrix, fOpacity, bScreenSpace);
		if ( iRet != XGE_OK ) {
			xrtFree(pPoints);
			return iRet;
		}
	}
	if ( iSegmentCount > 1 ) {
		int iJoinStart = bClosed ? 0 : 1;
		int iJoinEnd = bClosed ? iPointCount : (iPointCount - 1);

		for ( i = iJoinStart; i < iJoinEnd; i++ ) {
			int iPrev = (i + iPointCount - 1) % iPointCount;
			int iNext = (i + 1) % iPointCount;

			iRet = __xgeShapeExDrawStrokeJoin(pShape, pPoints[iPrev], pPoints[i], pPoints[iNext], fWidth, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
			if ( iRet != XGE_OK ) {
				xrtFree(pPoints);
				return iRet;
			}
		}
	}
	xrtFree(pPoints);
	return XGE_OK;
}

static float __xgeShapeExStrokeJoinAngle(xge_vec2_t tPrev, xge_vec2_t tCurr, xge_vec2_t tNext)
{
	xge_vec2_t tD1;
	xge_vec2_t tD2;
	float fDot;

	if ( !__xgeShapeExNormalizeSegment(tPrev, tCurr, &tD1) ||
	     !__xgeShapeExNormalizeSegment(tCurr, tNext, &tD2) ) {
		return 0.0f;
	}
	fDot = tD1.fX * tD2.fX + tD1.fY * tD2.fY;
	if ( fDot < -1.0f ) fDot = -1.0f;
	if ( fDot > 1.0f ) fDot = 1.0f;
	return acosf(fDot);
}

static int __xgeShapeExStrokeContourWantsExplicitJoins(const xge_vec2_t* pSrc, int iCount, int bClosed)
{
	xge_vec2_t* pPoints;
	int iPointCount;
	int iSharpCount;
	int iJoinStart;
	int iJoinEnd;
	int i;

	if ( (pSrc == NULL) || (iCount < 3) ) {
		return 0;
	}
	pPoints = NULL;
	iPointCount = 0;
	if ( __xgeShapeExCompactStrokePoints(pSrc, iCount, bClosed, &pPoints, &iPointCount) != XGE_OK ) {
		return 0;
	}
	if ( iPointCount < 3 ) {
		xrtFree(pPoints);
		return 0;
	}
	iSharpCount = 0;
	iJoinStart = bClosed ? 0 : 1;
	iJoinEnd = bClosed ? iPointCount : (iPointCount - 1);
	for ( i = iJoinStart; i < iJoinEnd; i++ ) {
		int iPrev = (i + iPointCount - 1) % iPointCount;
		int iNext = (i + 1) % iPointCount;
		float fAngle = __xgeShapeExStrokeJoinAngle(pPoints[iPrev], pPoints[i], pPoints[iNext]);

		if ( fAngle >= XGE_SHAPE_EX_EXPLICIT_JOIN_ANGLE ) {
			iSharpCount++;
		}
	}
	xrtFree(pPoints);
	return (iSharpCount > 0) && ((iSharpCount * 3) >= (iJoinEnd - iJoinStart));
}

static int __xgeShapeExStrokeContourMiterLimitExceeded(const xge_vec2_t* pSrc, int iCount, int bClosed, float fMiterLimit)
{
	xge_vec2_t* pPoints;
	int iPointCount;
	int iJoinStart;
	int iJoinEnd;
	int i;

	if ( (pSrc == NULL) || (iCount < 3) ) {
		return 0;
	}
	if ( fMiterLimit < 1.0f ) {
		fMiterLimit = 1.0f;
	}
	pPoints = NULL;
	iPointCount = 0;
	if ( __xgeShapeExCompactStrokePoints(pSrc, iCount, bClosed, &pPoints, &iPointCount) != XGE_OK ) {
		return 0;
	}
	if ( iPointCount < 3 ) {
		xrtFree(pPoints);
		return 0;
	}
	iJoinStart = bClosed ? 0 : 1;
	iJoinEnd = bClosed ? iPointCount : (iPointCount - 1);
	for ( i = iJoinStart; i < iJoinEnd; i++ ) {
		int iPrev = (i + iPointCount - 1) % iPointCount;
		int iNext = (i + 1) % iPointCount;
		float fAngle = __xgeShapeExStrokeJoinAngle(pPoints[iPrev], pPoints[i], pPoints[iNext]);

		if ( fAngle >= XGE_SHAPE_EX_EXPLICIT_JOIN_ANGLE ) {
			float fSin = sinf(fAngle * 0.5f);

			if ( (fSin > XGE_SHAPE_EX_EPSILON) && ((1.0f / fSin) > (fMiterLimit + XGE_SHAPE_EX_EPSILON)) ) {
				xrtFree(pPoints);
				return 1;
			}
		}
	}
	xrtFree(pPoints);
	return 0;
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

static float __xgeShapeExDashMinPositive(const float* pDashPattern, int iDashCount)
{
	float fMin;
	int i;

	fMin = 0.0f;
	for ( i = 0; i < iDashCount; i++ ) {
		if ( pDashPattern[i] > XGE_SHAPE_EX_EPSILON ) {
			if ( (fMin <= 0.0f) || (pDashPattern[i] < fMin) ) {
				fMin = pDashPattern[i];
			}
		}
	}
	return fMin;
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

static int __xgeShapeExDrawStrokeSolid(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, float fWidth, uint32_t iColor, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, float fOpacity, int bScreenSpace)
{
	int i;

	if ( (pFlat == NULL) || (fWidth <= 0.0f) ) {
		return XGE_OK;
	}
	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		const xge_shape_ex_flat_contour_t* pContour = &pFlat->pContours[i];
		int iSegmentCount;
		int j;

		if ( pContour->iCount < 2 ) {
			continue;
		}
		iSegmentCount = pContour->bClosed ? pContour->iCount : (pContour->iCount - 1);
		if ( pShape != NULL ) {
			int iRet;
			int bExplicitJoin;

			if ( pShape->iLineJoin == XGE_SHAPE_EX_JOIN_MITER ) {
				bExplicitJoin = __xgeShapeExStrokeContourMiterLimitExceeded(pFlat->pPoints + pContour->iStart, pContour->iCount, pContour->bClosed, pShape->fMiterLimit);
			} else {
				bExplicitJoin = __xgeShapeExStrokeContourWantsExplicitJoins(pFlat->pPoints + pContour->iStart, pContour->iCount, pContour->bClosed);
			}
			if ( bExplicitJoin ) {
				iRet = __xgeShapeExDrawStrokeSegmentJoinContour(pShape, pFlat->pPoints + pContour->iStart, pContour->iCount, pContour->bClosed, fWidth, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
			} else {
				iRet = __xgeShapeExDrawStrokeContourMesh(pShape, pFlat->pPoints + pContour->iStart, pContour->iCount, pContour->bClosed, fWidth, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
			}
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			continue;
		}
		for ( j = 0; j < iSegmentCount; j++ ) {
			xge_vec2_t tA = pFlat->pPoints[pContour->iStart + j];
			xge_vec2_t tB = pFlat->pPoints[pContour->iStart + ((j + 1) % pContour->iCount)];
			int bCapStart = (!pContour->bClosed && (j == 0));
			int bCapEnd = (!pContour->bClosed && (j == (iSegmentCount - 1)));
			int iRet;

			iRet = __xgeShapeExDrawStrokeSegment(pShape, tA, tB, fWidth, iColor, bCapStart, bCapEnd, tBounds, tMatrix, fOpacity, bScreenSpace);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
	}
	return XGE_OK;
}

static int __xgeShapeExDrawStrokeDashed(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, float fWidth, uint32_t iColor, const float* pDashPattern, int iDashCount, float fDashOffset, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, float fOpacity, int bScreenSpace)
{
	float fDashTotal;
	float fDashMin;
	int i;

	if ( (pFlat == NULL) || (pDashPattern == NULL) || (iDashCount <= 0) ) {
		return __xgeShapeExDrawStrokeSolid(pShape, pFlat, fWidth, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
	}
	fDashTotal = __xgeShapeExDashTotal(pDashPattern, iDashCount);
	if ( fDashTotal <= XGE_SHAPE_EX_EPSILON ) {
		return __xgeShapeExDrawStrokeSolid(pShape, pFlat, fWidth, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
	}
	fDashMin = __xgeShapeExDashMinPositive(pDashPattern, iDashCount);
	if ( fDashMin <= XGE_SHAPE_EX_EPSILON ) {
		return __xgeShapeExDrawStrokeSolid(pShape, pFlat, fWidth, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
	}
	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		const xge_shape_ex_flat_contour_t* pContour = &pFlat->pContours[i];
		float fContourLength;
		float fDashRemaining = 0.0f;
		float fPos;
		float fDashEpsilon;
		float fGuardEstimate;
		int iGuard;
		int iGuardLimit;
		int iDashIndex = 0;
		int bDashOn = 1;

		if ( (pContour->iCount < 2) || (pContour->iStart < 0) || ((pContour->iStart + pContour->iCount) > pFlat->iPointCount) ) {
			continue;
		}
		fContourLength = __xgeShapeExFlatContourLength(pFlat, pContour);
		if ( fContourLength <= XGE_SHAPE_EX_EPSILON ) {
			__xgeShapeExDashStateInit(pDashPattern, iDashCount, fDashOffset, &iDashIndex, &bDashOn, &fDashRemaining);
			if ( bDashOn && (fDashRemaining > XGE_SHAPE_EX_EPSILON) ) {
				int iRet = __xgeShapeExDrawStrokeZeroLengthCap(pShape, pFlat->pPoints[pContour->iStart], fWidth, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
			}
			continue;
		}
		__xgeShapeExDashStateInit(pDashPattern, iDashCount, fDashOffset, &iDashIndex, &bDashOn, &fDashRemaining);
		fPos = 0.0f;
		fDashEpsilon = fmaxf(XGE_SHAPE_EX_EPSILON, fContourLength * 0.000001f);
		fGuardEstimate = (fContourLength / fDashMin) + (float)iDashCount + 16.0f;
		if ( fGuardEstimate > 2000000.0f ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iGuard = 0;
		iGuardLimit = (int)ceilf(fGuardEstimate);
		while ( fPos < fContourLength - fDashEpsilon ) {
			float fStep = fDashRemaining;
			float fNext;

			if ( ++iGuard > iGuardLimit ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( fStep <= fDashEpsilon ) {
				__xgeShapeExDashStateNext(pDashPattern, iDashCount, &iDashIndex, &bDashOn, &fDashRemaining);
				continue;
			}
			fNext = fPos + fStep;
			if ( fNext <= fPos + fDashEpsilon ) {
				fDashRemaining = 0.0f;
				__xgeShapeExDashStateNext(pDashPattern, iDashCount, &iDashIndex, &bDashOn, &fDashRemaining);
				continue;
			}
			if ( fNext > fContourLength ) {
				fNext = fContourLength;
			}
			if ( bDashOn && (fNext > fPos + fDashEpsilon) ) {
				xge_shape_ex_flat_path_t tDashFlat;
				int iRet;

				memset(&tDashFlat, 0, sizeof(tDashFlat));
				iRet = __xgeShapeExFlatAddTrimmedContourRange(pFlat, i, fPos, fNext, &tDashFlat);
				if ( iRet != XGE_OK ) {
					__xgeShapeExFlatFree(&tDashFlat);
					return iRet;
				}
				iRet = __xgeShapeExDrawStrokeSolid(pShape, &tDashFlat, fWidth, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
				__xgeShapeExFlatFree(&tDashFlat);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
			}
			fDashRemaining -= (fNext - fPos);
			fPos = fNext;
			if ( fDashRemaining <= fDashEpsilon ) {
				__xgeShapeExDashStateNext(pDashPattern, iDashCount, &iDashIndex, &bDashOn, &fDashRemaining);
			}
		}
	}
	return XGE_OK;
}

static int __xgeShapeExDrawStroke(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, float fWidth, uint32_t iColor, const float* pDashPattern, int iDashCount, float fDashOffset, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, float fOpacity, int bScreenSpace)
{
	if ( (pFlat == NULL) || (fWidth <= 0.0f) || !__xgeShapeExStrokeVisible(pShape, iColor) ) {
		return XGE_OK;
	}
	if ( __xgeShapeExDashTotal(pDashPattern, iDashCount) > XGE_SHAPE_EX_EPSILON ) {
		return __xgeShapeExDrawStrokeDashed(pShape, pFlat, fWidth, iColor, pDashPattern, iDashCount, fDashOffset, tBounds, tMatrix, fOpacity, bScreenSpace);
	}
	return __xgeShapeExDrawStrokeSolid(pShape, pFlat, fWidth, iColor, tBounds, tMatrix, fOpacity, bScreenSpace);
}

static int __xgeShapeExStencilAvailable(void)
{
	return (glEnable != NULL) &&
		(glDisable != NULL) &&
		(glClear != NULL) &&
		(glClearStencil != NULL) &&
		(glStencilFunc != NULL) &&
		(glStencilOp != NULL) &&
		(glStencilMask != NULL) &&
		(glColorMask != NULL);
}

static int __xgeShapeExDrawStencilClipShape(xge_shape_ex pClipShape, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tParent)
{
	xge_shape_ex_flat_path_t tFlat;
	xge_shape_ex_flat_path_t tTrimmed;
	const xge_shape_ex_flat_path_t* pDrawFlat;
	xge_shape_ex_matrix_t tMatrix;
	struct xge_shape_ex_t tStencilShape;
	uint32_t iFillColor;
	uint32_t iStrokeColor;
	xge_rect_t tStrokeBounds;
	float fStrokeScale;
	float fStrokeWidth;
	float fDashOffset;
	float* pDashPattern;
	int iRet;

	memset(&tFlat, 0, sizeof(tFlat));
	memset(&tTrimmed, 0, sizeof(tTrimmed));
	if ( !__xgeShapeExValid(pClipShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !pClipShape->bVisible ) {
		return XGE_OK;
	}
	tMatrix = __xgeShapeExMatrixMul(tParent, pClipShape->tTransform);
	iRet = __xgeShapeExFlatten(pClipShape, tMatrix, fTolerance, &tFlat);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pDrawFlat = &tFlat;
	if ( pClipShape->bTrimPath ) {
		iRet = __xgeShapeExFlatTrim(&tFlat, pClipShape->fTrimBegin, pClipShape->fTrimEnd, pClipShape->bTrimSimultaneous, &tTrimmed);
		if ( iRet != XGE_OK ) {
			__xgeShapeExFlatFree(&tFlat);
			return iRet;
		}
		pDrawFlat = &tTrimmed;
	}
	tStencilShape = *pClipShape;
	if ( pClipShape->bStencilPaint ) {
		iFillColor = pClipShape->iFillColor;
		iStrokeColor = pClipShape->iStrokeColor;
		tStrokeBounds = __xgeShapeExFlatBounds(pDrawFlat);
		fStrokeScale = pClipShape->bStrokeNonScaling ? 1.0f : __xgeShapeExMatrixStrokeScale(tMatrix);
		fStrokeWidth = pClipShape->fStrokeWidth * fStrokeScale;
		fDashOffset = pClipShape->fDashOffset * fStrokeScale;
		pDashPattern = pClipShape->pDashPattern;
		if ( (pClipShape->pDashPattern != NULL) && (pClipShape->iDashCount > 0) && (fabsf(fStrokeScale - 1.0f) > XGE_SHAPE_EX_EPSILON) ) {
			int i;

			pDashPattern = (float*)xrtMalloc((size_t)pClipShape->iDashCount * sizeof(*pDashPattern));
			if ( pDashPattern == NULL ) {
				__xgeShapeExFlatFree(&tTrimmed);
				__xgeShapeExFlatFree(&tFlat);
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			for ( i = 0; i < pClipShape->iDashCount; i++ ) {
				pDashPattern[i] = pClipShape->pDashPattern[i] * fStrokeScale;
			}
		}
		if ( pClipShape->bStrokeFirst ) {
			iRet = __xgeShapeExDrawStroke(&tStencilShape, pDrawFlat, fStrokeWidth, iStrokeColor, pDashPattern, pClipShape->iDashCount, fDashOffset, tStrokeBounds, tMatrix, 1.0f, bScreenSpace);
			if ( iRet == XGE_OK ) {
				iRet = __xgeShapeExDrawFill(&tStencilShape, pDrawFlat, pClipShape->iFillRule, tMatrix, 1.0f, iFillColor, bScreenSpace);
			}
		} else {
			iRet = __xgeShapeExDrawFill(&tStencilShape, pDrawFlat, pClipShape->iFillRule, tMatrix, 1.0f, iFillColor, bScreenSpace);
			if ( iRet == XGE_OK ) {
				iRet = __xgeShapeExDrawStroke(&tStencilShape, pDrawFlat, fStrokeWidth, iStrokeColor, pDashPattern, pClipShape->iDashCount, fDashOffset, tStrokeBounds, tMatrix, 1.0f, bScreenSpace);
			}
		}
		if ( pDashPattern != pClipShape->pDashPattern ) {
			xrtFree(pDashPattern);
		}
	} else {
		tStencilShape.iFillType = XGE_SHAPE_EX_FILL_SOLID;
		tStencilShape.iFillColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		tStencilShape.pFillStops = NULL;
		tStencilShape.iFillStopCount = 0;
		tStencilShape.iFillStopCapacity = 0;
		iRet = __xgeShapeExDrawFill(&tStencilShape, pDrawFlat, pClipShape->iFillRule, tMatrix, 1.0f, XGE_COLOR_RGBA(255, 255, 255, 255), bScreenSpace);
	}
	__xgeShapeExFlatFree(&tTrimmed);
	__xgeShapeExFlatFree(&tFlat);
	return iRet;
}

static int __xgeShapeExEndStencilClip(int bStencilApplied, int iRet)
{
	if ( bStencilApplied ) {
		int iFlushRet = __xgeShapeAutoBatchFlush();

		if ( iRet == XGE_OK ) {
			iRet = iFlushRet;
		}
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glStencilMask(0xFFu);
		glDisable(GL_STENCIL_TEST);
	}
	return iRet;
}

static int __xgeShapeExBeginStencilClip(xge_shape_ex pShape, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tMatrix)
{
	int i;
	int iRet;

	if ( (pShape == NULL) || (pShape->iClipShapeCount <= 0) ) {
		return XGE_OK;
	}
	if ( !__xgeShapeExStencilAvailable() ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	iRet = __xgeShapeAutoBatchFlush();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	glEnable(GL_STENCIL_TEST);
	glStencilMask(0xFFu);
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_ALWAYS, 1, 0xFFu);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	for ( i = 0; i < pShape->iClipShapeCount; i++ ) {
		iRet = __xgeShapeExDrawStencilClipShape(pShape->pClipShapes[i], fTolerance, bScreenSpace, tMatrix);
		if ( iRet != XGE_OK ) {
			return __xgeShapeExEndStencilClip(1, iRet);
		}
	}
	iRet = __xgeShapeAutoBatchFlush();
	if ( iRet != XGE_OK ) {
		return __xgeShapeExEndStencilClip(1, iRet);
	}
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glStencilMask(0x00u);
	glStencilFunc(GL_EQUAL, 1, 0xFFu);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	return XGE_OK;
}

static int __xgeShapeExStencilClipBeginPublic(xge_shape_ex pClipShape, float fTolerance, int bScreenSpace, const xge_shape_ex_matrix_t* pParentMatrix, int* pApplied)
{
	xge_shape_ex_matrix_t tParent;
	int iRet;

	if ( pApplied == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pApplied = 0;
	if ( pClipShape == NULL ) {
		return XGE_OK;
	}
	if ( !__xgeShapeExValid(pClipShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeShapeExStencilAvailable() ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	tParent = pParentMatrix != NULL ? *pParentMatrix : __xgeShapeExMatrixIdentity();
	iRet = __xgeShapeAutoBatchFlush();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	glEnable(GL_STENCIL_TEST);
	glStencilMask(0xFFu);
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_ALWAYS, 1, 0xFFu);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	iRet = __xgeShapeExDrawStencilClipShape(pClipShape, fTolerance, bScreenSpace, tParent);
	if ( iRet != XGE_OK ) {
		return __xgeShapeExEndStencilClip(1, iRet);
	}
	iRet = __xgeShapeAutoBatchFlush();
	if ( iRet != XGE_OK ) {
		return __xgeShapeExEndStencilClip(1, iRet);
	}
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glStencilMask(0x00u);
	glStencilFunc(GL_EQUAL, 1, 0xFFu);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	*pApplied = 1;
	return XGE_OK;
}

static int __xgeShapeExFinishClip(int bClipApplied, int bOldClip, xge_rect_t tOldClip, int iRet)
{
	if ( bClipApplied ) {
		int iFlushRet = __xgeShapeAutoBatchFlush();

		if ( iRet == XGE_OK ) {
			iRet = iFlushRet;
		}
		if ( bOldClip ) xgeClipSet(tOldClip);
		else xgeClipClear();
	}
	return iRet;
}

static int __xgeShapeExFinishShapeClips(int bClipApplied, int bOldClip, xge_rect_t tOldClip, int bStencilApplied, int iRet)
{
	iRet = __xgeShapeExEndStencilClip(bStencilApplied, iRet);
	return __xgeShapeExFinishClip(bClipApplied, bOldClip, tOldClip, iRet);
}

static int __xgeShapeExDrawInternal(xge_shape_ex pShape, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tParent, float fParentOpacity)
{
	xge_shape_ex_flat_path_t tFlat;
	xge_shape_ex_flat_path_t tTrimmed;
	const xge_shape_ex_flat_path_t* pDrawFlat;
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
	int bStencilApplied;
	int iRet;

	memset(&tFlat, 0, sizeof(tFlat));
	memset(&tTrimmed, 0, sizeof(tTrimmed));
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
	bStencilApplied = 0;
	bOldClip = 0;
	memset(&tOldClip, 0, sizeof(tOldClip));
	if ( pShape->bClipRect ) {
		xge_rect_t tClip = __xgeShapeExMatrixRectBounds(tMatrix, pShape->tClipRect);

		iRet = __xgeShapeAutoBatchFlush();
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		tOldClip = xgeClipGet();
		bOldClip = (tOldClip.fW > 0.0f) && (tOldClip.fH > 0.0f);
		if ( bOldClip ) {
			tClip = __xgeShapeExRectIntersect(tOldClip, tClip);
		}
		xgeClipSet(tClip);
		bClipApplied = 1;
	}
	if ( pShape->iClipShapeCount > 0 ) {
		iRet = __xgeShapeExBeginStencilClip(pShape, fTolerance, bScreenSpace, tMatrix);
		if ( iRet != XGE_OK ) {
			return __xgeShapeExFinishShapeClips(bClipApplied, bOldClip, tOldClip, 0, iRet);
		}
		bStencilApplied = 1;
	}
	iRet = __xgeShapeExFlatten(pShape, tMatrix, fTolerance, &tFlat);
	if ( iRet != XGE_OK ) {
		return __xgeShapeExFinishShapeClips(bClipApplied, bOldClip, tOldClip, bStencilApplied, iRet);
	}
	pDrawFlat = &tFlat;
	if ( pShape->bTrimPath ) {
		iRet = __xgeShapeExFlatTrim(&tFlat, pShape->fTrimBegin, pShape->fTrimEnd, pShape->bTrimSimultaneous, &tTrimmed);
		if ( iRet != XGE_OK ) {
			__xgeShapeExFlatFree(&tFlat);
			return __xgeShapeExFinishShapeClips(bClipApplied, bOldClip, tOldClip, bStencilApplied, iRet);
		}
		pDrawFlat = &tTrimmed;
	}
	iFillColor = __xgeShapeExColorOpacity(pShape->iFillColor, fOpacity);
	iStrokeColor = __xgeShapeExColorOpacity(pShape->iStrokeColor, fOpacity);
	tStrokeBounds = __xgeShapeExFlatBounds(pDrawFlat);
	fStrokeScale = pShape->bStrokeNonScaling ? 1.0f : __xgeShapeExMatrixStrokeScale(tMatrix);
	fStrokeWidth = pShape->fStrokeWidth * fStrokeScale;
	fDashOffset = pShape->fDashOffset * fStrokeScale;
	pDashPattern = pShape->pDashPattern;
	if ( (pShape->pDashPattern != NULL) && (pShape->iDashCount > 0) && (fabsf(fStrokeScale - 1.0f) > XGE_SHAPE_EX_EPSILON) ) {
		int i;

		pDashPattern = (float*)xrtMalloc((size_t)pShape->iDashCount * sizeof(*pDashPattern));
		if ( pDashPattern == NULL ) {
			__xgeShapeExFlatFree(&tTrimmed);
			__xgeShapeExFlatFree(&tFlat);
			return __xgeShapeExFinishShapeClips(bClipApplied, bOldClip, tOldClip, bStencilApplied, XGE_ERROR_OUT_OF_MEMORY);
		}
		for ( i = 0; i < pShape->iDashCount; i++ ) {
			pDashPattern[i] = pShape->pDashPattern[i] * fStrokeScale;
		}
	}
	if ( pShape->bStrokeFirst ) {
		iRet = __xgeShapeExDrawStroke(pShape, pDrawFlat, fStrokeWidth, iStrokeColor, pDashPattern, pShape->iDashCount, fDashOffset, tStrokeBounds, tMatrix, fOpacity, bScreenSpace);
		if ( iRet == XGE_OK ) {
			iRet = __xgeShapeExDrawFill(pShape, pDrawFlat, pShape->iFillRule, tMatrix, fOpacity, iFillColor, bScreenSpace);
		}
	} else {
		iRet = __xgeShapeExDrawFill(pShape, pDrawFlat, pShape->iFillRule, tMatrix, fOpacity, iFillColor, bScreenSpace);
		if ( iRet == XGE_OK ) {
			iRet = __xgeShapeExDrawStroke(pShape, pDrawFlat, fStrokeWidth, iStrokeColor, pDashPattern, pShape->iDashCount, fDashOffset, tStrokeBounds, tMatrix, fOpacity, bScreenSpace);
		}
	}
	if ( pDashPattern != pShape->pDashPattern ) {
		xrtFree(pDashPattern);
	}
	__xgeShapeExFlatFree(&tTrimmed);
	__xgeShapeExFlatFree(&tFlat);
	return __xgeShapeExFinishShapeClips(bClipApplied, bOldClip, tOldClip, bStencilApplied, iRet);
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

static int __xgeShapeExSvgDoubleToFloat(double fValue, float* pValue)
{
	if ( pValue == NULL ) {
		return 0;
	}
	if ( fValue != fValue ) {
		return 0;
	}
	if ( (fValue < -(double)FLT_MAX) || (fValue > (double)FLT_MAX) ) {
		return 0;
	}
	*pValue = (float)fValue;
	return 1;
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
	if ( !__xgeShapeExSvgDoubleToFloat(fValue, pValue) ) {
		return 0;
	}
	*ppText = pEnd;
	return 1;
}

static int __xgeShapeExSvgReadFlag(const char** ppText, int* pValue)
{
	const char* pText;

	if ( (ppText == NULL) || (*ppText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	pText = *ppText;
	__xgeShapeExSvgSkipSeparators(&pText);
	if ( (*pText != '0') && (*pText != '1') ) {
		return 0;
	}
	*pValue = *pText - '0';
	pText++;
	__xgeShapeExSvgSkipSeparators(&pText);
	*ppText = pText;
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
	int iRet;

	fRX = fabsf(fRX);
	fRY = fabsf(fRY);
	if ( hypotf(fX - tCurrent.fX, fY - tCurrent.fY) <= XGE_SHAPE_EX_EPSILON ) {
		return XGE_OK;
	}
	if ( (fRX <= XGE_SHAPE_EX_EPSILON) || (fRY <= XGE_SHAPE_EX_EPSILON) ) {
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
		iRet = xgeShapeExCubicTo(pShape, tC1.fX, tC1.fY, tC2.fX, tC2.fY, tP.fX, tP.fY);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static xge_vec2_t __xgeShapeExEllipsePoint(float fCX, float fCY, float fRX, float fRY, float fRadians)
{
	xge_vec2_t tPoint;

	tPoint.fX = fCX + cosf(fRadians) * fRX;
	tPoint.fY = fCY + sinf(fRadians) * fRY;
	return tPoint;
}

static float __xgeShapeExClampSweep(float fSweep)
{
	if ( fSweep > XGE_SHAPE_EX_TAU ) {
		return XGE_SHAPE_EX_TAU;
	}
	if ( fSweep < -XGE_SHAPE_EX_TAU ) {
		return -XGE_SHAPE_EX_TAU;
	}
	return fSweep;
}

static int __xgeShapeExAppendEllipseArcSegments(xge_shape_ex pShape, float fCX, float fCY, float fRX, float fRY, float fStartRadians, float fSweepRadians, int bMoveToStart)
{
	xge_vec2_t tStart;
	int iSegments;
	int i;
	int iRet;

	tStart = __xgeShapeExEllipsePoint(fCX, fCY, fRX, fRY, fStartRadians);
	if ( bMoveToStart ) {
		iRet = xgeShapeExMoveTo(pShape, tStart.fX, tStart.fY);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	} else if ( !pShape->bHasCurrent ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( fabsf(fSweepRadians) <= XGE_SHAPE_EX_EPSILON ) {
		return XGE_OK;
	}
	iSegments = (int)ceilf(fabsf(fSweepRadians) / (XGE_SHAPE_EX_PI * 0.5f));
	if ( iSegments < 1 ) {
		iSegments = 1;
	}
	for ( i = 0; i < iSegments; i++ ) {
		float fT0 = fStartRadians + fSweepRadians * ((float)i / (float)iSegments);
		float fT1 = fStartRadians + fSweepRadians * ((float)(i + 1) / (float)iSegments);
		float fAlpha = (4.0f / 3.0f) * tanf((fT1 - fT0) * 0.25f);
		float fCos0 = cosf(fT0);
		float fSin0 = sinf(fT0);
		float fCos1 = cosf(fT1);
		float fSin1 = sinf(fT1);
		xge_vec2_t tC1;
		xge_vec2_t tC2;
		xge_vec2_t tP;

		tC1.fX = fCX + fRX * (fCos0 - fAlpha * fSin0);
		tC1.fY = fCY + fRY * (fSin0 + fAlpha * fCos0);
		tC2.fX = fCX + fRX * (fCos1 + fAlpha * fSin1);
		tC2.fY = fCY + fRY * (fSin1 - fAlpha * fCos1);
		tP = __xgeShapeExEllipsePoint(fCX, fCY, fRX, fRY, fT1);
		iRet = xgeShapeExCubicTo(pShape, tC1.fX, tC1.fY, tC2.fX, tC2.fY, tP.fX, tP.fY);
		if ( iRet != XGE_OK ) {
			return iRet;
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
	pShape->fTrimBegin = 0.0f;
	pShape->fTrimEnd = 1.0f;
	pShape->bTrimSimultaneous = 1;
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
	pClone->bStrokeNonScaling = pShape->bStrokeNonScaling;
	pClone->bTrimPath = pShape->bTrimPath;
	pClone->fTrimBegin = pShape->fTrimBegin;
	pClone->fTrimEnd = pShape->fTrimEnd;
	pClone->bTrimSimultaneous = pShape->bTrimSimultaneous;
	pClone->bStrokeFirst = pShape->bStrokeFirst;
	pClone->bStencilPaint = pShape->bStencilPaint;
	pClone->fOpacity = pShape->fOpacity;
	pClone->bVisible = pShape->bVisible;
	pClone->bClipRect = pShape->bClipRect;
	pClone->tClipRect = pShape->tClipRect;
	pClone->tTransform = pShape->tTransform;
	if ( pShape->iClipShapeCount > 0 ) {
		int i;

		iRet = __xgeShapeExReserveClipShapes(pClone, pShape->iClipShapeCount);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pClone);
			return iRet;
		}
		for ( i = 0; i < pShape->iClipShapeCount; i++ ) {
			iRet = __xgeShapeExClipShapeAddRef(pClone, pShape->pClipShapes[i]);
			if ( iRet != XGE_OK ) {
				xgeShapeExDestroy(pClone);
				return iRet;
			}
		}
	}
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
	__xgeShapeExClipShapesClearInternal(pShape);
	xrtFree(pShape->pCommands);
	xrtFree(pShape->pPoints);
	xrtFree(pShape->pDashPattern);
	xrtFree(pShape->pFillStops);
	xrtFree(pShape->pStrokeStops);
	xrtFree(pShape->pClipShapes);
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

int xgeShapeExArcTo(xge_shape_ex pShape, float fRX, float fRY, float fAxisDegrees, int bLargeArc, int bSweep, float fX, float fY)
{
	xge_shape_ex_path_snapshot_t tSnapshot = {0};
	int iRet;

	if ( !__xgeShapeExValid(pShape) || !pShape->bHasCurrent ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeShapeExPathSnapshot(pShape, &tSnapshot);
	iRet = __xgeShapeExSvgArcTo(pShape, pShape->tCurrent, fRX, fRY, fAxisDegrees, bLargeArc, bSweep, fX, fY);
	if ( iRet != XGE_OK ) {
		__xgeShapeExPathRestore(pShape, &tSnapshot);
	}
	return iRet;
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

int xgeShapeExAppendTriangle(xge_shape_ex pShape, float fX1, float fY1, float fX2, float fY2, float fX3, float fY3, int bClockwise)
{
	xge_shape_ex_path_snapshot_t tSnapshot = {0};
	int iRet;

	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeShapeExPathSnapshot(pShape, &tSnapshot);
	iRet = xgeShapeExMoveTo(pShape, fX1, fY1);
	if ( iRet != XGE_OK ) goto fail;
	if ( bClockwise ) {
		iRet = xgeShapeExLineTo(pShape, fX2, fY2);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExLineTo(pShape, fX3, fY3);
		if ( iRet != XGE_OK ) goto fail;
	} else {
		iRet = xgeShapeExLineTo(pShape, fX3, fY3);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExLineTo(pShape, fX2, fY2);
		if ( iRet != XGE_OK ) goto fail;
	}
	iRet = xgeShapeExClose(pShape);
	if ( iRet != XGE_OK ) goto fail;
	return XGE_OK;
fail:
	__xgeShapeExPathRestore(pShape, &tSnapshot);
	return iRet;
}

int xgeShapeExAppendRect(xge_shape_ex pShape, float fX, float fY, float fW, float fH, float fRX, float fRY, int bClockwise)
{
	xge_shape_ex_path_snapshot_t tSnapshot = {0};
	float fHalfW;
	float fHalfH;
	float fKX;
	float fKY;
	int iRet;

	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeShapeExPathSnapshot(pShape, &tSnapshot);
	if ( (fabsf(fRX) <= XGE_SHAPE_EX_EPSILON) && (fabsf(fRY) <= XGE_SHAPE_EX_EPSILON) ) {
		iRet = xgeShapeExMoveTo(pShape, fX + fW, fY);
		if ( iRet != XGE_OK ) goto fail;
		if ( bClockwise ) {
			iRet = xgeShapeExLineTo(pShape, fX + fW, fY + fH);
			if ( iRet != XGE_OK ) goto fail;
			iRet = xgeShapeExLineTo(pShape, fX, fY + fH);
			if ( iRet != XGE_OK ) goto fail;
			iRet = xgeShapeExLineTo(pShape, fX, fY);
			if ( iRet != XGE_OK ) goto fail;
		} else {
			iRet = xgeShapeExLineTo(pShape, fX, fY);
			if ( iRet != XGE_OK ) goto fail;
			iRet = xgeShapeExLineTo(pShape, fX, fY + fH);
			if ( iRet != XGE_OK ) goto fail;
			iRet = xgeShapeExLineTo(pShape, fX + fW, fY + fH);
			if ( iRet != XGE_OK ) goto fail;
		}
		iRet = xgeShapeExClose(pShape);
		if ( iRet != XGE_OK ) goto fail;
		return XGE_OK;
	}
	fHalfW = fW * 0.5f;
	fHalfH = fH * 0.5f;
	if ( fRX > fHalfW ) fRX = fHalfW;
	if ( fRY > fHalfH ) fRY = fHalfH;
	fKX = fRX * XGE_SHAPE_EX_KAPPA;
	fKY = fRY * XGE_SHAPE_EX_KAPPA;
	iRet = xgeShapeExMoveTo(pShape, fX + fW, fY + fRY);
	if ( iRet != XGE_OK ) goto fail;
	if ( bClockwise ) {
		iRet = xgeShapeExLineTo(pShape, fX + fW, fY + fH - fRY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExCubicTo(pShape, fX + fW, fY + fH - fRY + fKY, fX + fW - fRX + fKX, fY + fH, fX + fW - fRX, fY + fH);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExLineTo(pShape, fX + fRX, fY + fH);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExCubicTo(pShape, fX + fRX - fKX, fY + fH, fX, fY + fH - fRY + fKY, fX, fY + fH - fRY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExLineTo(pShape, fX, fY + fRY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExCubicTo(pShape, fX, fY + fRY - fKY, fX + fRX - fKX, fY, fX + fRX, fY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExLineTo(pShape, fX + fW - fRX, fY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExCubicTo(pShape, fX + fW - fRX + fKX, fY, fX + fW, fY + fRY - fKY, fX + fW, fY + fRY);
		if ( iRet != XGE_OK ) goto fail;
	} else {
		iRet = xgeShapeExCubicTo(pShape, fX + fW, fY + fRY - fKY, fX + fW - fRX + fKX, fY, fX + fW - fRX, fY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExLineTo(pShape, fX + fRX, fY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExCubicTo(pShape, fX + fRX - fKX, fY, fX, fY + fRY - fKY, fX, fY + fRY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExLineTo(pShape, fX, fY + fH - fRY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExCubicTo(pShape, fX, fY + fH - fRY + fKY, fX + fRX - fKX, fY + fH, fX + fRX, fY + fH);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExLineTo(pShape, fX + fW - fRX, fY + fH);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExCubicTo(pShape, fX + fW - fRX + fKX, fY + fH, fX + fW, fY + fH - fRY + fKY, fX + fW, fY + fH - fRY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExLineTo(pShape, fX + fW, fY + fRY);
		if ( iRet != XGE_OK ) goto fail;
	}
	iRet = xgeShapeExClose(pShape);
	if ( iRet != XGE_OK ) goto fail;
	return XGE_OK;
fail:
	__xgeShapeExPathRestore(pShape, &tSnapshot);
	return iRet;
}

int xgeShapeExAppendCapsule(xge_shape_ex pShape, float fX, float fY, float fW, float fH, int bClockwise)
{
	float fRadius;

	if ( !__xgeShapeExValid(pShape) || (fW < 0.0f) || (fH < 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fRadius = (fW < fH ? fW : fH) * 0.5f;
	return xgeShapeExAppendRect(pShape, fX, fY, fW, fH, fRadius, fRadius, bClockwise);
}

int xgeShapeExAppendCircle(xge_shape_ex pShape, float fCX, float fCY, float fRX, float fRY, int bClockwise)
{
	xge_shape_ex_path_snapshot_t tSnapshot = {0};
	float kx;
	float ky;
	int iRet;

	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeShapeExPathSnapshot(pShape, &tSnapshot);
	kx = fRX * XGE_SHAPE_EX_KAPPA;
	ky = fRY * XGE_SHAPE_EX_KAPPA;
	if ( bClockwise ) {
		iRet = xgeShapeExMoveTo(pShape, fCX, fCY - fRY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExCubicTo(pShape, fCX + kx, fCY - fRY, fCX + fRX, fCY - ky, fCX + fRX, fCY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExCubicTo(pShape, fCX + fRX, fCY + ky, fCX + kx, fCY + fRY, fCX, fCY + fRY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExCubicTo(pShape, fCX - kx, fCY + fRY, fCX - fRX, fCY + ky, fCX - fRX, fCY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExCubicTo(pShape, fCX - fRX, fCY - ky, fCX - kx, fCY - fRY, fCX, fCY - fRY);
		if ( iRet != XGE_OK ) goto fail;
	} else {
		iRet = xgeShapeExMoveTo(pShape, fCX, fCY - fRY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExCubicTo(pShape, fCX - kx, fCY - fRY, fCX - fRX, fCY - ky, fCX - fRX, fCY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExCubicTo(pShape, fCX - fRX, fCY + ky, fCX - kx, fCY + fRY, fCX, fCY + fRY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExCubicTo(pShape, fCX + kx, fCY + fRY, fCX + fRX, fCY + ky, fCX + fRX, fCY);
		if ( iRet != XGE_OK ) goto fail;
		iRet = xgeShapeExCubicTo(pShape, fCX + fRX, fCY - ky, fCX + kx, fCY - fRY, fCX, fCY - fRY);
		if ( iRet != XGE_OK ) goto fail;
	}
	iRet = xgeShapeExClose(pShape);
	if ( iRet != XGE_OK ) goto fail;
	return XGE_OK;
fail:
	__xgeShapeExPathRestore(pShape, &tSnapshot);
	return iRet;
}

int xgeShapeExAppendArc(xge_shape_ex pShape, float fCX, float fCY, float fRX, float fRY, float fStartRadians, float fEndRadians)
{
	xge_shape_ex_path_snapshot_t tSnapshot = {0};
	float fSweep;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (fRX < 0.0f) || (fRY < 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeShapeExPathSnapshot(pShape, &tSnapshot);
	fSweep = __xgeShapeExClampSweep(fEndRadians - fStartRadians);
	iRet = __xgeShapeExAppendEllipseArcSegments(pShape, fCX, fCY, fRX, fRY, fStartRadians, fSweep, 1);
	if ( iRet != XGE_OK ) {
		goto fail;
	}
	if ( fabsf(fSweep) >= (XGE_SHAPE_EX_TAU - XGE_SHAPE_EX_EPSILON) ) {
		iRet = xgeShapeExClose(pShape);
		if ( iRet != XGE_OK ) goto fail;
	}
	return XGE_OK;
fail:
	__xgeShapeExPathRestore(pShape, &tSnapshot);
	return iRet;
}

int xgeShapeExAppendPie(xge_shape_ex pShape, float fCX, float fCY, float fRX, float fRY, float fStartRadians, float fEndRadians)
{
	xge_shape_ex_path_snapshot_t tSnapshot = {0};
	xge_vec2_t tStart;
	float fSweep;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (fRX < 0.0f) || (fRY < 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeShapeExPathSnapshot(pShape, &tSnapshot);
	fSweep = __xgeShapeExClampSweep(fEndRadians - fStartRadians);
	if ( fabsf(fSweep) >= (XGE_SHAPE_EX_TAU - XGE_SHAPE_EX_EPSILON) ) {
		iRet = xgeShapeExAppendCircle(pShape, fCX, fCY, fRX, fRY, fSweep >= 0.0f);
		if ( iRet != XGE_OK ) goto fail;
		return XGE_OK;
	}
	tStart = __xgeShapeExEllipsePoint(fCX, fCY, fRX, fRY, fStartRadians);
	iRet = xgeShapeExMoveTo(pShape, fCX, fCY);
	if ( iRet != XGE_OK ) goto fail;
	iRet = xgeShapeExLineTo(pShape, tStart.fX, tStart.fY);
	if ( iRet != XGE_OK ) goto fail;
	iRet = __xgeShapeExAppendEllipseArcSegments(pShape, fCX, fCY, fRX, fRY, fStartRadians, fSweep, 0);
	if ( iRet != XGE_OK ) goto fail;
	iRet = xgeShapeExClose(pShape);
	if ( iRet != XGE_OK ) goto fail;
	return XGE_OK;
fail:
	__xgeShapeExPathRestore(pShape, &tSnapshot);
	return iRet;
}

int xgeShapeExAppendChord(xge_shape_ex pShape, float fCX, float fCY, float fRX, float fRY, float fStartRadians, float fEndRadians)
{
	xge_shape_ex_path_snapshot_t tSnapshot = {0};
	float fSweep;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (fRX < 0.0f) || (fRY < 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeShapeExPathSnapshot(pShape, &tSnapshot);
	fSweep = __xgeShapeExClampSweep(fEndRadians - fStartRadians);
	if ( fabsf(fSweep) >= (XGE_SHAPE_EX_TAU - XGE_SHAPE_EX_EPSILON) ) {
		iRet = xgeShapeExAppendCircle(pShape, fCX, fCY, fRX, fRY, fSweep >= 0.0f);
		if ( iRet != XGE_OK ) goto fail;
		return XGE_OK;
	}
	iRet = __xgeShapeExAppendEllipseArcSegments(pShape, fCX, fCY, fRX, fRY, fStartRadians, fSweep, 1);
	if ( iRet != XGE_OK ) goto fail;
	iRet = xgeShapeExClose(pShape);
	if ( iRet != XGE_OK ) goto fail;
	return XGE_OK;
fail:
	__xgeShapeExPathRestore(pShape, &tSnapshot);
	return iRet;
}

int xgeShapeExAppendPath(xge_shape_ex pShape, const uint8_t* pCommands, int iCommandCount, const xge_vec2_t* pPoints, int iPointCount)
{
	xge_vec2_t tCurrent;
	xge_vec2_t tStart;
	xge_vec2_t tLastCubicControl;
	xge_vec2_t tLastQuadControl;
	int bHasCurrent;
	int bHasStart;
	int iPointRead;
	int iPointNeeded;
	int i;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (pCommands == NULL) || (pPoints == NULL) || (iCommandCount <= 0) || (iPointCount <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iPointNeeded = 0;
	for ( i = 0; i < iCommandCount; i++ ) {
		int iCount = __xgeShapeExCommandPointCount(pCommands[i]);

		if ( iCount < 0 ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( iPointNeeded > (INT_MAX - iCount) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iPointNeeded += iCount;
	}
	if ( iPointNeeded != iPointCount ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeShapeExReserveCommands(pShape, pShape->iCommandCount + iCommandCount);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeShapeExReservePoints(pShape, pShape->iPointCount + iPointCount);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tCurrent = pShape->tCurrent;
	tStart = pShape->tStart;
	tLastCubicControl = pShape->tLastCubicControl;
	tLastQuadControl = pShape->tLastQuadControl;
	bHasCurrent = pShape->bHasCurrent;
	bHasStart = pShape->bHasStart;
	iPointRead = 0;
	for ( i = 0; i < iCommandCount; i++ ) {
		switch ( pCommands[i] ) {
			case XGE_SHAPE_EX_CMD_CLOSE:
				if ( bHasStart ) {
					tCurrent = tStart;
					tLastCubicControl = tCurrent;
					tLastQuadControl = tCurrent;
					bHasCurrent = 1;
				}
				break;
			case XGE_SHAPE_EX_CMD_MOVE_TO:
				tCurrent = pPoints[iPointRead++];
				tStart = tCurrent;
				tLastCubicControl = tCurrent;
				tLastQuadControl = tCurrent;
				bHasCurrent = 1;
				bHasStart = 1;
				break;
			case XGE_SHAPE_EX_CMD_LINE_TO:
				tCurrent = pPoints[iPointRead++];
				tLastCubicControl = tCurrent;
				tLastQuadControl = tCurrent;
				bHasCurrent = 1;
				if ( !bHasStart ) {
					tStart = tCurrent;
					bHasStart = 1;
				}
				break;
			case XGE_SHAPE_EX_CMD_CUBIC_TO:
				tLastCubicControl = pPoints[iPointRead + 1];
				tLastQuadControl = pPoints[iPointRead + 2];
				tCurrent = pPoints[iPointRead + 2];
				iPointRead += 3;
				bHasCurrent = 1;
				if ( !bHasStart ) {
					tStart = tCurrent;
					bHasStart = 1;
				}
				break;
			default:
				return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	memcpy(pShape->pCommands + pShape->iCommandCount, pCommands, (size_t)iCommandCount * sizeof(*pCommands));
	memcpy(pShape->pPoints + pShape->iPointCount, pPoints, (size_t)iPointCount * sizeof(*pPoints));
	pShape->iCommandCount += iCommandCount;
	pShape->iPointCount += iPointCount;
	pShape->tCurrent = tCurrent;
	pShape->tStart = tStart;
	pShape->tLastCubicControl = tLastCubicControl;
	pShape->tLastQuadControl = tLastQuadControl;
	pShape->bHasCurrent = bHasCurrent;
	pShape->bHasStart = bHasStart;
	return XGE_OK;
}

static int __xgeShapeExAppendSvgPathNoRollback(xge_shape_ex pShape, const char* sPath)
{
	const char* pText;
	char cCommand;
	char cPrevCommand;
	int bRelative;
	int iRet;

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
						iRet = xgeShapeExMoveTo(pShape, fX, fY);
						if ( iRet != XGE_OK ) return iRet;
						bFirst = 0;
					} else {
						iRet = xgeShapeExLineTo(pShape, fX, fY);
						if ( iRet != XGE_OK ) return iRet;
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
					iRet = xgeShapeExLineTo(pShape, fX, fY);
					if ( iRet != XGE_OK ) return iRet;
				}
				cPrevCommand = cCommand;
				break;
			}
			case 'h': {
				float fX;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					if ( !__xgeShapeExSvgReadNumber(&pText, &fX) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( bRelative ) fX += pShape->tCurrent.fX;
					iRet = xgeShapeExLineTo(pShape, fX, pShape->tCurrent.fY);
					if ( iRet != XGE_OK ) return iRet;
				}
				cPrevCommand = cCommand;
				break;
			}
			case 'v': {
				float fY;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					if ( !__xgeShapeExSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( bRelative ) fY += pShape->tCurrent.fY;
					iRet = xgeShapeExLineTo(pShape, pShape->tCurrent.fX, fY);
					if ( iRet != XGE_OK ) return iRet;
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
					iRet = xgeShapeExCubicTo(pShape, fC1X, fC1Y, fC2X, fC2Y, fX, fY);
					if ( iRet != XGE_OK ) return iRet;
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
					iRet = xgeShapeExCubicTo(pShape, fC1X, fC1Y, fC2X, fC2Y, fX, fY);
					if ( iRet != XGE_OK ) return iRet;
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
					iRet = xgeShapeExQuadTo(pShape, fCX, fCY, fX, fY);
					if ( iRet != XGE_OK ) return iRet;
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
					iRet = xgeShapeExQuadTo(pShape, fCX, fCY, fX, fY);
					if ( iRet != XGE_OK ) return iRet;
					cPrevCommand = cCommand;
				}
				cPrevCommand = cCommand;
				break;
			}
			case 'a': {
				float fRX, fRY, fAxis, fX, fY;
				int bLargeArcFlag;
				int bSweepFlag;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					int iArcRet;
					if ( !__xgeShapeExSvgReadNumber(&pText, &fRX) || !__xgeShapeExSvgReadNumber(&pText, &fRY) ||
					     !__xgeShapeExSvgReadNumber(&pText, &fAxis) || !__xgeShapeExSvgReadFlag(&pText, &bLargeArcFlag) ||
					     !__xgeShapeExSvgReadFlag(&pText, &bSweepFlag) || !__xgeShapeExSvgReadNumber(&pText, &fX) ||
					     !__xgeShapeExSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( bRelative ) {
						fX += pShape->tCurrent.fX;
						fY += pShape->tCurrent.fY;
					}
					iArcRet = xgeShapeExArcTo(pShape, fRX, fRY, fAxis, bLargeArcFlag, bSweepFlag, fX, fY);
					if ( iArcRet != XGE_OK ) {
						return iArcRet;
					}
				}
				cPrevCommand = cCommand;
				break;
			}
			case 'z':
				iRet = xgeShapeExClose(pShape);
				if ( iRet != XGE_OK ) return iRet;
				cPrevCommand = cCommand;
				break;
			default:
				return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

int xgeShapeExAppendSvgPath(xge_shape_ex pShape, const char* sPath)
{
	xge_shape_ex_path_snapshot_t tSnapshot = {0};
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (sPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeShapeExPathSnapshot(pShape, &tSnapshot);
	iRet = __xgeShapeExAppendSvgPathNoRollback(pShape, sPath);
	if ( iRet != XGE_OK ) {
		__xgeShapeExPathRestore(pShape, &tSnapshot);
	}
	return iRet;
}

int xgeShapeExGetPath(xge_shape_ex pShape, const uint8_t** ppCommands, int* pCommandCount, const xge_vec2_t** ppPoints, int* pPointCount)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( ppCommands != NULL ) {
		*ppCommands = pShape->pCommands;
	}
	if ( pCommandCount != NULL ) {
		*pCommandCount = pShape->iCommandCount;
	}
	if ( ppPoints != NULL ) {
		*ppPoints = pShape->pPoints;
	}
	if ( pPointCount != NULL ) {
		*pPointCount = pShape->iPointCount;
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

int xgeShapeExFillTypeGet(xge_shape_ex pShape, int* pType)
{
	if ( !__xgeShapeExValid(pShape) || (pType == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pType = pShape->iFillType;
	return XGE_OK;
}

int xgeShapeExFillColorGet(xge_shape_ex pShape, uint32_t* pColor)
{
	if ( !__xgeShapeExValid(pShape) || (pColor == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pColor = pShape->iFillColor;
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
	__xgeShapeExNormalizeStops(pShape->pFillStops, iStopCount);
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

int xgeShapeExFillLinearGradientGet(xge_shape_ex pShape, float* pX1, float* pY1, float* pX2, float* pY2, int* pUnits, const xge_shape_ex_color_stop_t** ppStops, int* pStopCount)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pShape->iFillType != XGE_SHAPE_EX_FILL_LINEAR_GRADIENT ) {
		return XGE_ERROR_NOT_FOUND;
	}
	if ( pX1 != NULL ) *pX1 = pShape->fFillX1;
	if ( pY1 != NULL ) *pY1 = pShape->fFillY1;
	if ( pX2 != NULL ) *pX2 = pShape->fFillX2;
	if ( pY2 != NULL ) *pY2 = pShape->fFillY2;
	if ( pUnits != NULL ) *pUnits = pShape->iFillGradientUnits;
	if ( ppStops != NULL ) *ppStops = pShape->pFillStops;
	if ( pStopCount != NULL ) *pStopCount = pShape->iFillStopCount;
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
	__xgeShapeExNormalizeStops(pShape->pFillStops, iStopCount);
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

int xgeShapeExFillRadialGradientGet(xge_shape_ex pShape, float* pCX, float* pCY, float* pRadius, float* pFX, float* pFY, int* pUnits, const xge_shape_ex_color_stop_t** ppStops, int* pStopCount)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pShape->iFillType != XGE_SHAPE_EX_FILL_RADIAL_GRADIENT ) {
		return XGE_ERROR_NOT_FOUND;
	}
	if ( pCX != NULL ) *pCX = pShape->fFillX1;
	if ( pCY != NULL ) *pCY = pShape->fFillY1;
	if ( pRadius != NULL ) *pRadius = pShape->fFillR;
	if ( pFX != NULL ) *pFX = pShape->fFillFX;
	if ( pFY != NULL ) *pFY = pShape->fFillFY;
	if ( pUnits != NULL ) *pUnits = pShape->iFillGradientUnits;
	if ( ppStops != NULL ) *ppStops = pShape->pFillStops;
	if ( pStopCount != NULL ) *pStopCount = pShape->iFillStopCount;
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

int xgeShapeExFillGradientSpreadGet(xge_shape_ex pShape, int* pSpread)
{
	if ( !__xgeShapeExValid(pShape) || (pSpread == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pSpread = pShape->iFillGradientSpread;
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

int xgeShapeExFillGradientTransformGet(xge_shape_ex pShape, xge_shape_ex_matrix_t* pMatrix)
{
	if ( !__xgeShapeExValid(pShape) || (pMatrix == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pMatrix = pShape->tFillGradientTransform;
	return XGE_OK;
}

int xgeShapeExStrokeColor(xge_shape_ex pShape, uint32_t iColor)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->iStrokeType = XGE_SHAPE_EX_FILL_SOLID;
	pShape->iStrokeColor = iColor;
	return XGE_OK;
}

int xgeShapeExStrokeTypeGet(xge_shape_ex pShape, int* pType)
{
	if ( !__xgeShapeExValid(pShape) || (pType == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pType = pShape->iStrokeType;
	return XGE_OK;
}

int xgeShapeExStrokeColorGet(xge_shape_ex pShape, uint32_t* pColor)
{
	if ( !__xgeShapeExValid(pShape) || (pColor == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pColor = pShape->iStrokeColor;
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
	__xgeShapeExNormalizeStops(pShape->pStrokeStops, iStopCount);
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

int xgeShapeExStrokeLinearGradientGet(xge_shape_ex pShape, float* pX1, float* pY1, float* pX2, float* pY2, int* pUnits, const xge_shape_ex_color_stop_t** ppStops, int* pStopCount)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pShape->iStrokeType != XGE_SHAPE_EX_FILL_LINEAR_GRADIENT ) {
		return XGE_ERROR_NOT_FOUND;
	}
	if ( pX1 != NULL ) *pX1 = pShape->fStrokeX1;
	if ( pY1 != NULL ) *pY1 = pShape->fStrokeY1;
	if ( pX2 != NULL ) *pX2 = pShape->fStrokeX2;
	if ( pY2 != NULL ) *pY2 = pShape->fStrokeY2;
	if ( pUnits != NULL ) *pUnits = pShape->iStrokeGradientUnits;
	if ( ppStops != NULL ) *ppStops = pShape->pStrokeStops;
	if ( pStopCount != NULL ) *pStopCount = pShape->iStrokeStopCount;
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
	__xgeShapeExNormalizeStops(pShape->pStrokeStops, iStopCount);
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

int xgeShapeExStrokeRadialGradientGet(xge_shape_ex pShape, float* pCX, float* pCY, float* pRadius, float* pFX, float* pFY, int* pUnits, const xge_shape_ex_color_stop_t** ppStops, int* pStopCount)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pShape->iStrokeType != XGE_SHAPE_EX_FILL_RADIAL_GRADIENT ) {
		return XGE_ERROR_NOT_FOUND;
	}
	if ( pCX != NULL ) *pCX = pShape->fStrokeX1;
	if ( pCY != NULL ) *pCY = pShape->fStrokeY1;
	if ( pRadius != NULL ) *pRadius = pShape->fStrokeR;
	if ( pFX != NULL ) *pFX = pShape->fStrokeFX;
	if ( pFY != NULL ) *pFY = pShape->fStrokeFY;
	if ( pUnits != NULL ) *pUnits = pShape->iStrokeGradientUnits;
	if ( ppStops != NULL ) *ppStops = pShape->pStrokeStops;
	if ( pStopCount != NULL ) *pStopCount = pShape->iStrokeStopCount;
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

int xgeShapeExStrokeGradientSpreadGet(xge_shape_ex pShape, int* pSpread)
{
	if ( !__xgeShapeExValid(pShape) || (pSpread == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pSpread = pShape->iStrokeGradientSpread;
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

int xgeShapeExStrokeGradientTransformGet(xge_shape_ex pShape, xge_shape_ex_matrix_t* pMatrix)
{
	if ( !__xgeShapeExValid(pShape) || (pMatrix == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pMatrix = pShape->tStrokeGradientTransform;
	return XGE_OK;
}

int xgeShapeExStrokeWidth(xge_shape_ex pShape, float fWidth)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( fWidth < 0.0f ) fWidth = 0.0f;
	pShape->fStrokeWidth = fWidth;
	return XGE_OK;
}

int xgeShapeExStrokeWidthGet(xge_shape_ex pShape, float* pWidth)
{
	if ( !__xgeShapeExValid(pShape) || (pWidth == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pWidth = pShape->fStrokeWidth;
	return XGE_OK;
}

int xgeShapeExStrokeCap(xge_shape_ex pShape, int iCap)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( (iCap != XGE_SHAPE_EX_CAP_BUTT) && (iCap != XGE_SHAPE_EX_CAP_ROUND) && (iCap != XGE_SHAPE_EX_CAP_SQUARE) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->iLineCap = iCap;
	return XGE_OK;
}

int xgeShapeExStrokeCapGet(xge_shape_ex pShape, int* pCap)
{
	if ( !__xgeShapeExValid(pShape) || (pCap == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pCap = pShape->iLineCap;
	return XGE_OK;
}

int xgeShapeExStrokeJoin(xge_shape_ex pShape, int iJoin)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( (iJoin != XGE_SHAPE_EX_JOIN_MITER) && (iJoin != XGE_SHAPE_EX_JOIN_ROUND) && (iJoin != XGE_SHAPE_EX_JOIN_BEVEL) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->iLineJoin = iJoin;
	return XGE_OK;
}

int xgeShapeExStrokeJoinGet(xge_shape_ex pShape, int* pJoin)
{
	if ( !__xgeShapeExValid(pShape) || (pJoin == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pJoin = pShape->iLineJoin;
	return XGE_OK;
}

int xgeShapeExStrokeMiterLimit(xge_shape_ex pShape, float fLimit)
{
	if ( !__xgeShapeExValid(pShape) || (fLimit < 0.0f) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->fMiterLimit = fLimit;
	return XGE_OK;
}

int xgeShapeExStrokeMiterLimitGet(xge_shape_ex pShape, float* pLimit)
{
	if ( !__xgeShapeExValid(pShape) || (pLimit == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pLimit = pShape->fMiterLimit;
	return XGE_OK;
}

int xgeShapeExStrokeNonScaling(xge_shape_ex pShape, int bNonScaling)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->bStrokeNonScaling = bNonScaling != 0;
	return XGE_OK;
}

int xgeShapeExStrokeNonScalingGet(xge_shape_ex pShape, int* pNonScaling)
{
	if ( !__xgeShapeExValid(pShape) || (pNonScaling == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pNonScaling = pShape->bStrokeNonScaling;
	return XGE_OK;
}

int xgeShapeExStrokeDash(xge_shape_ex pShape, const float* pDashPattern, int iDashCount, float fDashOffset)
{
	float* pCopy;
	int i;

	if ( !__xgeShapeExValid(pShape) || (iDashCount < 0) || ((pDashPattern == NULL) && (iDashCount > 0)) || ((pDashPattern != NULL) && (iDashCount == 0)) ) {
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
	for ( i = 0; i < iDashCount; i++ ) {
		pCopy[i] = pDashPattern[i] < 0.0f ? 0.0f : pDashPattern[i];
	}
	pShape->pDashPattern = pCopy;
	pShape->iDashCount = iDashCount;
	return XGE_OK;
}

int xgeShapeExStrokeDashGet(xge_shape_ex pShape, const float** ppDashPattern, int* pDashCount, float* pDashOffset)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( ppDashPattern != NULL ) {
		*ppDashPattern = pShape->pDashPattern;
	}
	if ( pDashCount != NULL ) {
		*pDashCount = pShape->iDashCount;
	}
	if ( pDashOffset != NULL ) {
		*pDashOffset = pShape->fDashOffset;
	}
	return XGE_OK;
}

int xgeShapeExTrimPath(xge_shape_ex pShape, float fBegin, float fEnd, int bSimultaneous)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pShape->bTrimPath = 1;
	pShape->fTrimBegin = fBegin;
	pShape->fTrimEnd = fEnd;
	pShape->bTrimSimultaneous = bSimultaneous != 0;
	return XGE_OK;
}

int xgeShapeExTrimClear(xge_shape_ex pShape)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pShape->bTrimPath = 0;
	pShape->fTrimBegin = 0.0f;
	pShape->fTrimEnd = 1.0f;
	pShape->bTrimSimultaneous = 1;
	return XGE_OK;
}

int xgeShapeExFillRule(xge_shape_ex pShape, int iRule)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( (iRule != XGE_SHAPE_EX_FILL_NON_ZERO) && (iRule != XGE_SHAPE_EX_FILL_EVEN_ODD) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->iFillRule = iRule;
	return XGE_OK;
}

int xgeShapeExFillRuleGet(xge_shape_ex pShape, int* pRule)
{
	if ( !__xgeShapeExValid(pShape) || (pRule == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pRule = pShape->iFillRule;
	return XGE_OK;
}

int xgeShapeExPaintOrder(xge_shape_ex pShape, int bStrokeFirst)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->bStrokeFirst = bStrokeFirst != 0;
	return XGE_OK;
}

int xgeShapeExPaintOrderGet(xge_shape_ex pShape, int* pStrokeFirst)
{
	if ( !__xgeShapeExValid(pShape) || (pStrokeFirst == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pStrokeFirst = pShape->bStrokeFirst;
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

int xgeShapeExOpacityGet(xge_shape_ex pShape, float* pOpacity)
{
	if ( !__xgeShapeExValid(pShape) || (pOpacity == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pOpacity = pShape->fOpacity;
	return XGE_OK;
}

int xgeShapeExVisible(xge_shape_ex pShape, int bVisible)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	pShape->bVisible = bVisible != 0;
	return XGE_OK;
}

int xgeShapeExVisibleGet(xge_shape_ex pShape, int* pVisible)
{
	if ( !__xgeShapeExValid(pShape) || (pVisible == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pVisible = pShape->bVisible;
	return XGE_OK;
}

static xge_rect_t __xgeShapeExExpandBoundsForStroke(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix);

static int __xgeShapeExGetBoundsInternal(xge_shape_ex pShape, float fTolerance, xge_shape_ex_matrix_t tParent, xge_rect_t* pBounds)
{
	xge_shape_ex_flat_path_t tFlat;
	xge_shape_ex_flat_path_t tTrimmed;
	const xge_shape_ex_flat_path_t* pBoundsFlat;
	xge_shape_ex_matrix_t tMatrix;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (pBounds == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( fTolerance <= 0.0f ) {
		fTolerance = XGE_SHAPE_EX_DEFAULT_TOLERANCE;
	}
	tMatrix = __xgeShapeExMatrixMul(tParent, pShape->tTransform);
	iRet = __xgeShapeExFlatten(pShape, tMatrix, fTolerance, &tFlat);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	memset(&tTrimmed, 0, sizeof(tTrimmed));
	pBoundsFlat = &tFlat;
	if ( pShape->bTrimPath ) {
		iRet = __xgeShapeExFlatTrim(&tFlat, pShape->fTrimBegin, pShape->fTrimEnd, pShape->bTrimSimultaneous, &tTrimmed);
		if ( iRet != XGE_OK ) {
			__xgeShapeExFlatFree(&tFlat);
			return iRet;
		}
		pBoundsFlat = &tTrimmed;
	}
	*pBounds = __xgeShapeExExpandBoundsForStroke(pShape, pBoundsFlat, __xgeShapeExFlatBounds(pBoundsFlat), tMatrix);
	__xgeShapeExFlatFree(&tTrimmed);
	__xgeShapeExFlatFree(&tFlat);
	return XGE_OK;
}

static xge_rect_t __xgeShapeExExpandBoundsForStroke(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix)
{
	float fStrokeScale;
	float fStrokeWidth;
	float fExpand;

	if ( !__xgeShapeExValid(pShape) || (pFlat == NULL) || (pFlat->iPointCount <= 0) ) {
		return tBounds;
	}
	if ( (pShape->fStrokeWidth <= 0.0f) || !__xgeShapeExStrokeVisible(pShape, pShape->iStrokeColor) ) {
		return tBounds;
	}
	fStrokeScale = pShape->bStrokeNonScaling ? 1.0f : __xgeShapeExMatrixStrokeScale(tMatrix);
	fStrokeWidth = pShape->fStrokeWidth * fStrokeScale;
	if ( fStrokeWidth <= 0.0f ) {
		return tBounds;
	}
	fExpand = fStrokeWidth * 0.5f;
	tBounds.fX -= fExpand;
	tBounds.fY -= fExpand;
	tBounds.fW += fExpand * 2.0f;
	tBounds.fH += fExpand * 2.0f;
	return tBounds;
}

static int __xgeShapeExGetLocalBounds(xge_shape_ex pShape, float fTolerance, xge_rect_t* pBounds)
{
	xge_shape_ex_flat_path_t tFlat;
	xge_shape_ex_flat_path_t tTrimmed;
	const xge_shape_ex_flat_path_t* pBoundsFlat;
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
	memset(&tTrimmed, 0, sizeof(tTrimmed));
	pBoundsFlat = &tFlat;
	if ( pShape->bTrimPath ) {
		iRet = __xgeShapeExFlatTrim(&tFlat, pShape->fTrimBegin, pShape->fTrimEnd, pShape->bTrimSimultaneous, &tTrimmed);
		if ( iRet != XGE_OK ) {
			__xgeShapeExFlatFree(&tFlat);
			return iRet;
		}
		pBoundsFlat = &tTrimmed;
	}
	*pBounds = __xgeShapeExExpandBoundsForStroke(pShape, pBoundsFlat, __xgeShapeExFlatBounds(pBoundsFlat), __xgeShapeExMatrixIdentity());
	__xgeShapeExFlatFree(&tTrimmed);
	__xgeShapeExFlatFree(&tFlat);
	return XGE_OK;
}

static void __xgeShapeExRectToOBB(xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, xge_vec2_t* pPoints4)
{
	xge_vec2_t tPoint;

	tPoint.fX = tBounds.fX;
	tPoint.fY = tBounds.fY;
	pPoints4[0] = __xgeShapeExMatrixPoint(tMatrix, tPoint);
	tPoint.fX = tBounds.fX + tBounds.fW;
	tPoint.fY = tBounds.fY;
	pPoints4[1] = __xgeShapeExMatrixPoint(tMatrix, tPoint);
	tPoint.fX = tBounds.fX + tBounds.fW;
	tPoint.fY = tBounds.fY + tBounds.fH;
	pPoints4[2] = __xgeShapeExMatrixPoint(tMatrix, tPoint);
	tPoint.fX = tBounds.fX;
	tPoint.fY = tBounds.fY + tBounds.fH;
	pPoints4[3] = __xgeShapeExMatrixPoint(tMatrix, tPoint);
}

static int __xgeShapeExGetOBBInternal(xge_shape_ex pShape, float fTolerance, xge_shape_ex_matrix_t tParent, xge_vec2_t* pPoints4)
{
	xge_shape_ex_matrix_t tMatrix;
	xge_rect_t tBounds;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (pPoints4 == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeShapeExGetLocalBounds(pShape, fTolerance, &tBounds);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tMatrix = __xgeShapeExMatrixMul(tParent, pShape->tTransform);
	__xgeShapeExRectToOBB(tBounds, tMatrix, pPoints4);
	return XGE_OK;
}

static int __xgeShapeExRectIntersects(xge_rect_t a, xge_rect_t b)
{
	if ( (a.fW <= 0.0f) || (a.fH <= 0.0f) || (b.fW <= 0.0f) || (b.fH <= 0.0f) ) {
		return 0;
	}
	return (a.fX < (b.fX + b.fW)) && ((a.fX + a.fW) > b.fX) && (a.fY < (b.fY + b.fH)) && ((a.fY + a.fH) > b.fY);
}

int xgeShapeExGetBounds(xge_shape_ex pShape, float fTolerance, xge_rect_t* pBounds)
{
	return __xgeShapeExGetBoundsInternal(pShape, fTolerance, __xgeShapeExMatrixIdentity(), pBounds);
}

int xgeShapeExGetOBB(xge_shape_ex pShape, float fTolerance, xge_vec2_t* pPoints4)
{
	return __xgeShapeExGetOBBInternal(pShape, fTolerance, __xgeShapeExMatrixIdentity(), pPoints4);
}

int xgeShapeExBoundsIntersects(xge_shape_ex pShape, xge_rect_t tRect, float fTolerance, int* pIntersects)
{
	xge_rect_t tBounds;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (pIntersects == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pIntersects = 0;
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_OK;
	}
	iRet = xgeShapeExGetBounds(pShape, fTolerance, &tBounds);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	*pIntersects = __xgeShapeExRectIntersects(tBounds, tRect);
	return XGE_OK;
}

int xgeShapeExGetLength(xge_shape_ex pShape, float fTolerance, float* pLength)
{
	xge_shape_ex_path_measure pMeasure;
	int iRet;

	pMeasure = NULL;
	iRet = xgeShapeExPathMeasureCreate(&pMeasure, pShape, NULL, fTolerance);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeShapeExPathMeasureGetLength(pMeasure, pLength);
	xgeShapeExPathMeasureDestroy(pMeasure);
	return iRet;
}

int xgeShapeExGetPointAtLength(xge_shape_ex pShape, float fDistance, float fTolerance, xge_vec2_t* pPoint, xge_vec2_t* pTangent)
{
	xge_shape_ex_path_measure pMeasure;
	int iRet;

	pMeasure = NULL;
	iRet = xgeShapeExPathMeasureCreate(&pMeasure, pShape, NULL, fTolerance);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeShapeExPathMeasureGetPointAtLength(pMeasure, fDistance, pPoint, pTangent);
	xgeShapeExPathMeasureDestroy(pMeasure);
	return iRet;
}

int xgeShapeExPathMeasureCreate(xge_shape_ex_path_measure* ppMeasure, xge_shape_ex pShape, const xge_shape_ex_matrix_t* pMatrix, float fTolerance)
{
	xge_shape_ex_path_measure pMeasure;
	xge_shape_ex_matrix_t tMatrix;
	int iRet;

	if ( (ppMeasure == NULL) || !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppMeasure = NULL;
	if ( fTolerance <= 0.0f ) {
		fTolerance = XGE_SHAPE_EX_DEFAULT_TOLERANCE;
	}
	tMatrix = pMatrix != NULL ? *pMatrix : __xgeShapeExMatrixIdentity();
	pMeasure = (xge_shape_ex_path_measure)xrtCalloc(1, sizeof(*pMeasure));
	if ( pMeasure == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xgeShapeExFlatten(pShape, tMatrix, fTolerance, &pMeasure->tFlat);
	if ( iRet != XGE_OK ) {
		xrtFree(pMeasure);
		return iRet;
	}
	pMeasure->fLength = __xgeShapeExFlatLength(&pMeasure->tFlat);
	pMeasure->iMagic = XGE_SHAPE_EX_PATH_MEASURE_MAGIC;
	*ppMeasure = pMeasure;
	return XGE_OK;
}

void xgeShapeExPathMeasureDestroy(xge_shape_ex_path_measure pMeasure)
{
	if ( !__xgeShapeExPathMeasureValid(pMeasure) ) {
		return;
	}
	pMeasure->iMagic = 0;
	__xgeShapeExFlatFree(&pMeasure->tFlat);
	memset(pMeasure, 0, sizeof(*pMeasure));
	xrtFree(pMeasure);
}

int xgeShapeExPathMeasureGetLength(xge_shape_ex_path_measure pMeasure, float* pLength)
{
	if ( !__xgeShapeExPathMeasureValid(pMeasure) || (pLength == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pLength = pMeasure->fLength;
	return XGE_OK;
}

int xgeShapeExPathMeasureGetPointAtLength(xge_shape_ex_path_measure pMeasure, float fDistance, xge_vec2_t* pPoint, xge_vec2_t* pTangent)
{
	if ( !__xgeShapeExPathMeasureValid(pMeasure) || ((pPoint == NULL) && (pTangent == NULL)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeShapeExFlatPointAtLength(&pMeasure->tFlat, fDistance, pPoint, pTangent) ? XGE_OK : XGE_ERROR_NOT_FOUND;
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

int xgeShapeExClipShapeAdd(xge_shape_ex pShape, xge_shape_ex pClipShape)
{
	return __xgeShapeExClipShapeAddRef(pShape, pClipShape);
}

int xgeShapeExClipShapeClear(xge_shape_ex pShape)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	__xgeShapeExClipShapesClearInternal(pShape);
	return XGE_OK;
}

int xgeShapeExClipClear(xge_shape_ex pShape)
{
	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	memset(&pShape->tClipRect, 0, sizeof(pShape->tClipRect));
	pShape->bClipRect = 0;
	__xgeShapeExClipShapesClearInternal(pShape);
	return XGE_OK;
}

int xgeShapeExStencilClipBegin(xge_shape_ex pClipShape, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, int* pApplied)
{
	return __xgeShapeExStencilClipBeginPublic(pClipShape, fTolerance, 0, pParentMatrix, pApplied);
}

int xgeShapeExStencilClipBeginPx(xge_shape_ex pClipShape, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, int* pApplied)
{
	return __xgeShapeExStencilClipBeginPublic(pClipShape, fTolerance, 1, pParentMatrix, pApplied);
}

int xgeShapeExStencilClipEnd(int bApplied, int iRet)
{
	return __xgeShapeExEndStencilClip(bApplied != 0, iRet);
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

int xgeShapeExTransformGet(xge_shape_ex pShape, xge_shape_ex_matrix_t* pMatrix)
{
	if ( !__xgeShapeExValid(pShape) || (pMatrix == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pMatrix = pShape->tTransform;
	return XGE_OK;
}

static int __xgeShapeExTransformConcat(xge_shape_ex_matrix_t* pTransform, const xge_shape_ex_matrix_t* pLocal)
{
	if ( (pTransform == NULL) || (pLocal == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pTransform = __xgeShapeExMatrixMul(*pTransform, *pLocal);
	return XGE_OK;
}

int xgeShapeExTransformTranslate(xge_shape_ex pShape, float fTX, float fTY)
{
	xge_shape_ex_matrix_t tLocal;

	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	xgeShapeExMatrixTranslate(&tLocal, fTX, fTY);
	return __xgeShapeExTransformConcat(&pShape->tTransform, &tLocal);
}

int xgeShapeExTransformScale(xge_shape_ex pShape, float fSX, float fSY)
{
	xge_shape_ex_matrix_t tLocal;

	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	xgeShapeExMatrixScale(&tLocal, fSX, fSY);
	return __xgeShapeExTransformConcat(&pShape->tTransform, &tLocal);
}

int xgeShapeExTransformRotate(xge_shape_ex pShape, float fRadians)
{
	xge_shape_ex_matrix_t tLocal;

	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	xgeShapeExMatrixRotate(&tLocal, fRadians);
	return __xgeShapeExTransformConcat(&pShape->tTransform, &tLocal);
}

int xgeShapeExDraw(xge_shape_ex pShape, float fTolerance)
{
	return xgeShapeExDrawEx(pShape, fTolerance, NULL, 1.0f);
}

int xgeShapeExDrawPx(xge_shape_ex pShape, float fTolerance)
{
	return xgeShapeExDrawPxEx(pShape, fTolerance, NULL, 1.0f);
}

int xgeShapeExDrawEx(xge_shape_ex pShape, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, float fParentOpacity)
{
	xge_shape_ex_matrix_t tParent;

	tParent = pParentMatrix != NULL ? *pParentMatrix : __xgeShapeExMatrixIdentity();
	return __xgeShapeExDrawInternal(pShape, fTolerance, 0, tParent, fParentOpacity);
}

int xgeShapeExDrawPxEx(xge_shape_ex pShape, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, float fParentOpacity)
{
	xge_shape_ex_matrix_t tParent;

	tParent = pParentMatrix != NULL ? *pParentMatrix : __xgeShapeExMatrixIdentity();
	return __xgeShapeExDrawInternal(pShape, fTolerance, 1, tParent, fParentOpacity);
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

int xgeShapeExSceneClone(xge_shape_ex_scene pScene, xge_shape_ex_scene* ppClone)
{
	xge_shape_ex_scene pClone;
	int i;
	int iRet;

	if ( !__xgeShapeExSceneValid(pScene) || (ppClone == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppClone = NULL;
	iRet = xgeShapeExSceneCreate(&pClone);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pClone->fOpacity = pScene->fOpacity;
	pClone->bVisible = pScene->bVisible;
	pClone->tTransform = pScene->tTransform;
	for ( i = 0; i < pScene->iShapeCount; i++ ) {
		xge_shape_ex pShapeClone;

		pShapeClone = NULL;
		iRet = xgeShapeExClone(pScene->pShapes[i], &pShapeClone);
		if ( iRet != XGE_OK ) {
			xgeShapeExSceneDestroy(pClone);
			return iRet;
		}
		iRet = xgeShapeExSceneAdd(pClone, pShapeClone);
		xgeShapeExDestroy(pShapeClone);
		if ( iRet != XGE_OK ) {
			xgeShapeExSceneDestroy(pClone);
			return iRet;
		}
	}
	*ppClone = pClone;
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

static int __xgeShapeExSceneReserveShapes(xge_shape_ex_scene pScene, int iNeeded)
{
	xge_shape_ex* pShapes;
	int iCapacity;

	if ( !__xgeShapeExSceneValid(pScene) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pScene->iShapeCapacity ) {
		return XGE_OK;
	}
	iCapacity = pScene->iShapeCapacity > 0 ? pScene->iShapeCapacity * 2 : 8;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT_MAX / 2) ) {
			iCapacity = iNeeded;
			break;
		}
		iCapacity *= 2;
	}
	pShapes = (xge_shape_ex*)xrtRealloc(pScene->pShapes, (size_t)iCapacity * sizeof(*pShapes));
	if ( pShapes == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pScene->pShapes = pShapes;
	pScene->iShapeCapacity = iCapacity;
	return XGE_OK;
}

int xgeShapeExSceneAdd(xge_shape_ex_scene pScene, xge_shape_ex pShape)
{
	int iRet;

	if ( !__xgeShapeExSceneValid(pScene) || !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	iRet = __xgeShapeExSceneReserveShapes(pScene, pScene->iShapeCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeShapeExAddRef(pShape);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pScene->pShapes[pScene->iShapeCount++] = pShape;
	return XGE_OK;
}

int xgeShapeExSceneInsert(xge_shape_ex_scene pScene, xge_shape_ex pShape, xge_shape_ex pBefore)
{
	int i;
	int iRet;

	if ( !__xgeShapeExSceneValid(pScene) || !__xgeShapeExValid(pShape) || !__xgeShapeExValid(pBefore) || (pShape == pBefore) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pScene->iShapeCount; i++ ) {
		if ( pScene->pShapes[i] == pBefore ) {
			iRet = __xgeShapeExSceneReserveShapes(pScene, pScene->iShapeCount + 1);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			iRet = xgeShapeExAddRef(pShape);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			memmove(pScene->pShapes + i + 1, pScene->pShapes + i, (size_t)(pScene->iShapeCount - i) * sizeof(*pScene->pShapes));
			pScene->pShapes[i] = pShape;
			pScene->iShapeCount++;
			return XGE_OK;
		}
	}
	return XGE_ERROR_NOT_FOUND;
}

int xgeShapeExSceneRemove(xge_shape_ex_scene pScene, xge_shape_ex pShape)
{
	int i;

	if ( !__xgeShapeExSceneValid(pScene) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pShape == NULL ) {
		return xgeShapeExSceneClear(pScene);
	}
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pScene->iShapeCount; i++ ) {
		if ( pScene->pShapes[i] == pShape ) {
			xgeShapeExDestroy(pScene->pShapes[i]);
			if ( i + 1 < pScene->iShapeCount ) {
				memmove(pScene->pShapes + i, pScene->pShapes + i + 1, (size_t)(pScene->iShapeCount - i - 1) * sizeof(*pScene->pShapes));
			}
			pScene->iShapeCount--;
			pScene->pShapes[pScene->iShapeCount] = NULL;
			return XGE_OK;
		}
	}
	return XGE_ERROR_NOT_FOUND;
}

int xgeShapeExSceneGetCount(xge_shape_ex_scene pScene, int* pCount)
{
	if ( !__xgeShapeExSceneValid(pScene) || (pCount == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pCount = pScene->iShapeCount;
	return XGE_OK;
}

int xgeShapeExSceneGetAt(xge_shape_ex_scene pScene, int iIndex, xge_shape_ex* ppShape)
{
	if ( !__xgeShapeExSceneValid(pScene) || (ppShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iIndex < 0) || (iIndex >= pScene->iShapeCount) ) {
		*ppShape = NULL;
		return XGE_ERROR_NOT_FOUND;
	}
	*ppShape = pScene->pShapes[iIndex];
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

int xgeShapeExSceneTransformGet(xge_shape_ex_scene pScene, xge_shape_ex_matrix_t* pMatrix)
{
	if ( !__xgeShapeExSceneValid(pScene) || (pMatrix == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pMatrix = pScene->tTransform;
	return XGE_OK;
}

int xgeShapeExSceneTransformTranslate(xge_shape_ex_scene pScene, float fTX, float fTY)
{
	xge_shape_ex_matrix_t tLocal;

	if ( !__xgeShapeExSceneValid(pScene) ) return XGE_ERROR_INVALID_ARGUMENT;
	xgeShapeExMatrixTranslate(&tLocal, fTX, fTY);
	return __xgeShapeExTransformConcat(&pScene->tTransform, &tLocal);
}

int xgeShapeExSceneTransformScale(xge_shape_ex_scene pScene, float fSX, float fSY)
{
	xge_shape_ex_matrix_t tLocal;

	if ( !__xgeShapeExSceneValid(pScene) ) return XGE_ERROR_INVALID_ARGUMENT;
	xgeShapeExMatrixScale(&tLocal, fSX, fSY);
	return __xgeShapeExTransformConcat(&pScene->tTransform, &tLocal);
}

int xgeShapeExSceneTransformRotate(xge_shape_ex_scene pScene, float fRadians)
{
	xge_shape_ex_matrix_t tLocal;

	if ( !__xgeShapeExSceneValid(pScene) ) return XGE_ERROR_INVALID_ARGUMENT;
	xgeShapeExMatrixRotate(&tLocal, fRadians);
	return __xgeShapeExTransformConcat(&pScene->tTransform, &tLocal);
}

int xgeShapeExSceneOpacity(xge_shape_ex_scene pScene, float fOpacity)
{
	if ( !__xgeShapeExSceneValid(pScene) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( fOpacity < 0.0f ) fOpacity = 0.0f;
	if ( fOpacity > 1.0f ) fOpacity = 1.0f;
	pScene->fOpacity = fOpacity;
	return XGE_OK;
}

int xgeShapeExSceneOpacityGet(xge_shape_ex_scene pScene, float* pOpacity)
{
	if ( !__xgeShapeExSceneValid(pScene) || (pOpacity == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pOpacity = pScene->fOpacity;
	return XGE_OK;
}

int xgeShapeExSceneVisible(xge_shape_ex_scene pScene, int bVisible)
{
	if ( !__xgeShapeExSceneValid(pScene) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pScene->bVisible = bVisible != 0;
	return XGE_OK;
}

int xgeShapeExSceneVisibleGet(xge_shape_ex_scene pScene, int* pVisible)
{
	if ( !__xgeShapeExSceneValid(pScene) || (pVisible == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pVisible = pScene->bVisible;
	return XGE_OK;
}

int xgeShapeExSceneGetBounds(xge_shape_ex_scene pScene, float fTolerance, xge_rect_t* pBounds)
{
	xge_shape_ex_matrix_t tMatrix;
	xge_rect_t tOut;
	int bHasBounds;
	int i;

	if ( !__xgeShapeExSceneValid(pScene) || (pBounds == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( fTolerance <= 0.0f ) {
		fTolerance = XGE_SHAPE_EX_DEFAULT_TOLERANCE;
	}
	memset(&tOut, 0, sizeof(tOut));
	bHasBounds = 0;
	tMatrix = pScene->tTransform;
	for ( i = 0; i < pScene->iShapeCount; i++ ) {
		xge_rect_t tShapeBounds;
		int iRet;

		iRet = __xgeShapeExGetBoundsInternal(pScene->pShapes[i], fTolerance, tMatrix, &tShapeBounds);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( !bHasBounds ) {
			tOut = tShapeBounds;
			bHasBounds = 1;
		} else {
			tOut = __xgeShapeExRectUnion(tOut, tShapeBounds);
		}
	}
	*pBounds = tOut;
	return XGE_OK;
}

int xgeShapeExSceneGetOBB(xge_shape_ex_scene pScene, float fTolerance, xge_vec2_t* pPoints4)
{
	xge_rect_t tOut;
	int bHasBounds;
	int i;

	if ( !__xgeShapeExSceneValid(pScene) || (pPoints4 == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( fTolerance <= 0.0f ) {
		fTolerance = XGE_SHAPE_EX_DEFAULT_TOLERANCE;
	}
	memset(&tOut, 0, sizeof(tOut));
	bHasBounds = 0;
	for ( i = 0; i < pScene->iShapeCount; i++ ) {
		xge_rect_t tShapeBounds;
		int iRet;

		iRet = __xgeShapeExGetBoundsInternal(pScene->pShapes[i], fTolerance, __xgeShapeExMatrixIdentity(), &tShapeBounds);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( !bHasBounds ) {
			tOut = tShapeBounds;
			bHasBounds = 1;
		} else {
			tOut = __xgeShapeExRectUnion(tOut, tShapeBounds);
		}
	}
	__xgeShapeExRectToOBB(tOut, pScene->tTransform, pPoints4);
	return XGE_OK;
}

int xgeShapeExSceneBoundsIntersects(xge_shape_ex_scene pScene, xge_rect_t tRect, float fTolerance, int* pIntersects)
{
	xge_rect_t tBounds;
	int iRet;

	if ( !__xgeShapeExSceneValid(pScene) || (pIntersects == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pIntersects = 0;
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_OK;
	}
	iRet = xgeShapeExSceneGetBounds(pScene, fTolerance, &tBounds);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	*pIntersects = __xgeShapeExRectIntersects(tBounds, tRect);
	return XGE_OK;
}

int xgeShapeExSceneDraw(xge_shape_ex_scene pScene, float fTolerance)
{
	return xgeShapeExSceneDrawEx(pScene, fTolerance, NULL, 1.0f);
}

int xgeShapeExSceneDrawPx(xge_shape_ex_scene pScene, float fTolerance)
{
	return xgeShapeExSceneDrawPxEx(pScene, fTolerance, NULL, 1.0f);
}

int xgeShapeExSceneDrawEx(xge_shape_ex_scene pScene, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, float fParentOpacity)
{
	xge_shape_ex_matrix_t tParent;

	tParent = pParentMatrix != NULL ? *pParentMatrix : __xgeShapeExMatrixIdentity();
	return __xgeShapeExSceneDrawInternal(pScene, fTolerance, 0, tParent, fParentOpacity);
}

int xgeShapeExSceneDrawPxEx(xge_shape_ex_scene pScene, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, float fParentOpacity)
{
	xge_shape_ex_matrix_t tParent;

	tParent = pParentMatrix != NULL ? *pParentMatrix : __xgeShapeExMatrixIdentity();
	return __xgeShapeExSceneDrawInternal(pScene, fTolerance, 1, tParent, fParentOpacity);
}
