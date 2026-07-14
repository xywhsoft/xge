#include <float.h>
#include <limits.h>
#include <stdarg.h>

#define XGE_SHAPE_EX_MAGIC 0x58475358u
#define XGE_SHAPE_EX_PATH_MEASURE_MAGIC 0x5847504Du
#define XGE_SHAPE_EX_SCENE_MAGIC 0x58475343u
#define XGE_SHAPE_EX_DEFAULT_TOLERANCE 0.35f
#define XGE_SHAPE_EX_KAPPA 0.5522847498307936f
#define XGE_SHAPE_EX_PI 3.14159265358979323846f
#define XGE_SHAPE_EX_TAU 6.28318530717958647692f
#define XGE_SHAPE_EX_EPSILON 0.00001f
#define XGE_SHAPE_EX_EXPLICIT_JOIN_ANGLE (XGE_SHAPE_EX_PI / 3.0f)
#define XGE_SHAPE_EX_CLIP_BOUNDS_MAX_DEPTH 32
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
	float fFillFR;
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
	float fStrokeFR;
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
	int iBlend;
	int bBlendSet;
	int bClipRect;
	xge_rect_t tClipRect;
	xge_shape_ex* pClipShapes;
	int* pClipShapeModes;
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
	int iBlend;
	int bBlendSet;
	int bClipRect;
	xge_rect_t tClipRect;
	xge_shape_ex* pClipShapes;
	int* pClipShapeModes;
	int iClipShapeCount;
	int iClipShapeCapacity;
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

typedef struct xge_shape_ex_blend_renderer_t {
	xge_shader_t tShader;
	int bInitialized;
} xge_shape_ex_blend_renderer_t;

typedef int (*xge_shape_ex_blend_draw_proc)(void* pUser, xge_shape_ex_matrix_t tLocalParent);

static xge_shape_ex_blend_renderer_t g_xgeShapeExBlendRenderer;

static int __xgeShapeExFloatFinite(float fValue)
{
	return (fValue == fValue) && (fValue >= -FLT_MAX) && (fValue <= FLT_MAX);
}

static int __xgeShapeExCoordsFinite(float fX, float fY)
{
	return __xgeShapeExFloatFinite(fX) && __xgeShapeExFloatFinite(fY);
}

static int __xgeShapeExPointFinite(xge_vec2_t tPoint)
{
	return __xgeShapeExCoordsFinite(tPoint.fX, tPoint.fY);
}

static int __xgeShapeExRectFinite(xge_rect_t tRect)
{
	return __xgeShapeExCoordsFinite(tRect.fX, tRect.fY) && __xgeShapeExCoordsFinite(tRect.fW, tRect.fH);
}

static int __xgeShapeExMatrixFinite(const xge_shape_ex_matrix_t* pMatrix)
{
	return (pMatrix != NULL) &&
	       __xgeShapeExCoordsFinite(pMatrix->fA, pMatrix->fB) &&
	       __xgeShapeExCoordsFinite(pMatrix->fC, pMatrix->fD) &&
	       __xgeShapeExCoordsFinite(pMatrix->fE, pMatrix->fF);
}

static int __xgeShapeExStopsFinite(const xge_shape_ex_color_stop_t* pStops, int iStopCount)
{
	int i;

	if ( (pStops == NULL) || (iStopCount <= 0) ) {
		return 0;
	}
	for ( i = 0; i < iStopCount; i++ ) {
		if ( !__xgeShapeExFloatFinite(pStops[i].fOffset) ) {
			return 0;
		}
	}
	return 1;
}

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

static int __xgeShapeExPathHasSvgPrefix(xge_shape_ex pShape, const xge_shape_ex_path_snapshot_t* pSnapshot)
{
	if ( (pShape == NULL) || (pSnapshot == NULL) ) {
		return 0;
	}
	return (pShape->iCommandCount > pSnapshot->iCommandCount) &&
	       (pShape->pCommands != NULL) &&
	       (pShape->pCommands[pSnapshot->iCommandCount] == XGE_SHAPE_EX_CMD_MOVE_TO);
}

static int __xgeShapeExSvgPathFinish(xge_shape_ex pShape, const xge_shape_ex_path_snapshot_t* pSnapshot, int iRet)
{
	if ( iRet == XGE_OK ) {
		return XGE_OK;
	}
	if ( (iRet == XGE_ERROR_INVALID_ARGUMENT) && __xgeShapeExPathHasSvgPrefix(pShape, pSnapshot) ) {
		return XGE_OK;
	}
	__xgeShapeExPathRestore(pShape, pSnapshot);
	return iRet;
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
	if ( (pMatrix == NULL) || !__xgeShapeExCoordsFinite(fTX, fTY) ) {
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
	if ( (pMatrix == NULL) || !__xgeShapeExCoordsFinite(fSX, fSY) ) {
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

	if ( (pMatrix == NULL) || !__xgeShapeExFloatFinite(fRadians) ) {
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
	if ( (pMatrix == NULL) || !__xgeShapeExCoordsFinite(fXRadians, fYRadians) ) {
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

static int __xgeShapeExBlendValid(int iBlend)
{
	return (iBlend >= XGE_BLEND_NONE) && (iBlend <= XGE_BLEND_LUMINOSITY);
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
		case XGE_SHAPE_EX_CMD_QUAD_TO:
			return 2;
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
	if ( fT < pStops[0].fOffset ) {
		return pStops[0].iColor;
	}
	for ( i = 0; i < iStopCount; i++ ) {
		if ( fT == pStops[i].fOffset ) {
			while ( ((i + 1) < iStopCount) && (pStops[i + 1].fOffset == pStops[i].fOffset) ) i++;
			return pStops[i].iColor;
		}
		if ( (i == 0) || (fT > pStops[i].fOffset) ) {
			continue;
		}
		{
			float fA = pStops[i - 1].fOffset;
			float fB = pStops[i].fOffset;
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

static float __xgeShapeExRadialGradientTEx(xge_vec2_t tPoint, xge_vec2_t tCenter, xge_vec2_t tFocal, float fRadius, float fFocalRadius)
{
	xge_vec2_t tP;
	xge_vec2_t tD;
	xge_vec2_t tDiff;
	float fDR;
	float fDist;
	float fScale;
	float fMaxFR;
	float fDen;
	float fA;
	float fPrecision;
	float fB;
	float fC;
	float fDisc;
	int bFocalClamped;

	if ( fRadius < 0.02f ) {
		return 1.0f;
	}
	if ( fFocalRadius < 0.0f ) {
		fFocalRadius = 0.0f;
	}
	if ( fFocalRadius >= fRadius ) {
		fFocalRadius = fRadius - XGE_SHAPE_EX_EPSILON;
		if ( fFocalRadius < 0.0f ) {
			fFocalRadius = 0.0f;
		}
	}
	bFocalClamped = 0;
	tDiff.fX = tCenter.fX - tFocal.fX;
	tDiff.fY = tCenter.fY - tFocal.fY;
	fDist = sqrtf((tDiff.fX * tDiff.fX) + (tDiff.fY * tDiff.fY));
	if ( (fRadius - fDist) < 0.02f ) {
		if ( fDist < 0.02f ) {
			fDist = 0.02f;
			tDiff.fX = 0.02f;
			tDiff.fY = 0.0f;
		}
		fScale = fRadius * 0.98f / fDist;
		tDiff.fX *= fScale;
		tDiff.fY *= fScale;
		fDist *= fScale;
		tFocal.fX = tCenter.fX - tDiff.fX;
		tFocal.fY = tCenter.fY - tDiff.fY;
		bFocalClamped = 1;
	}
	fMaxFR = (fRadius - fDist) * 0.98f;
	if ( fFocalRadius > fMaxFR ) {
		fFocalRadius = fMaxFR > 0.0f ? fMaxFR : 0.0f;
	}
	tP.fX = tPoint.fX - tFocal.fX;
	tP.fY = tPoint.fY - tFocal.fY;
	tD.fX = tCenter.fX - tFocal.fX;
	tD.fY = tCenter.fY - tFocal.fY;
	fDR = fRadius - fFocalRadius;
	fC = (tP.fX * tP.fX) + (tP.fY * tP.fY) - (fFocalRadius * fFocalRadius);
	fA = (fDR * fDR) - (tD.fX * tD.fX) - (tD.fY * tD.fY);
	fPrecision = fmaxf(0.00001f, fmaxf(fDR * fDR, 1.0f) * 0.0001f);
	fDen = (fDR * fFocalRadius) + (tP.fX * tD.fX) + (tP.fY * tD.fY);
	if ( fA < fPrecision ) {
		if ( fabsf(fDen) <= XGE_SHAPE_EX_EPSILON ) {
			return 1.0f;
		}
		return 0.5f * fC / fDen;
	}
	fB = fDen / fA;
	fDisc = (fB * fB) + (fC / fA);
	if ( fDisc < 0.0f ) return 0.0f;
	fDisc = sqrtf(fDisc);
	if ( bFocalClamped && (fB < 0.0f) ) return 0.0f;
	return fDisc - fB;
}

static uint32_t __xgeShapeExRadialGradientColorEx(float fCX, float fCY, float fRadius, float fFX, float fFY, float fFocalRadius, int iUnits, xge_shape_ex_matrix_t tGradientTransform, const xge_shape_ex_color_stop_t* pStops, int iStopCount, uint32_t iFallbackColor, int iSpread, xge_vec2_t tPoint, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix)
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
	fT = __xgeShapeExRadialGradientTEx(tSample, tCenter, tFocal, fRadius, fFocalRadius);
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
		pShape->fFillFR,
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

static int __xgeShapeExClipModeValid(int iMode)
{
	return (iMode == XGE_SHAPE_EX_CLIP_INTERSECT) || (iMode == XGE_SHAPE_EX_CLIP_SUBTRACT);
}

static int __xgeShapeExReserveClipShapes(xge_shape_ex pShape, int iNeeded)
{
	xge_shape_ex* pShapes;
	int* pModes;
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
	pShapes = (xge_shape_ex*)xrtMalloc((size_t)iCapacity * sizeof(*pShapes));
	if ( pShapes == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pModes = (int*)xrtMalloc((size_t)iCapacity * sizeof(*pModes));
	if ( pModes == NULL ) {
		xrtFree(pShapes);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( pShape->iClipShapeCount > 0 ) {
		memcpy(pShapes, pShape->pClipShapes, (size_t)pShape->iClipShapeCount * sizeof(*pShapes));
		if ( pShape->pClipShapeModes != NULL ) {
			memcpy(pModes, pShape->pClipShapeModes, (size_t)pShape->iClipShapeCount * sizeof(*pModes));
		} else {
			int i;

			for ( i = 0; i < pShape->iClipShapeCount; i++ ) {
				pModes[i] = XGE_SHAPE_EX_CLIP_INTERSECT;
			}
		}
	}
	xrtFree(pShape->pClipShapes);
	xrtFree(pShape->pClipShapeModes);
	pShape->pClipShapes = pShapes;
	pShape->pClipShapeModes = pModes;
	pShape->iClipShapeCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeShapeExClipShapeReferences(xge_shape_ex pShape, xge_shape_ex pTarget, int iDepth, int* pReferences)
{
	int i;

	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExValid(pTarget) || (pReferences == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pReferences = 0;
	if ( pShape == pTarget ) {
		*pReferences = 1;
		return XGE_OK;
	}
	if ( iDepth >= XGE_SHAPE_EX_CLIP_BOUNDS_MAX_DEPTH ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pShape->iClipShapeCount; i++ ) {
		int bReferences;
		int iRet;

		iRet = __xgeShapeExClipShapeReferences(pShape->pClipShapes[i], pTarget, iDepth + 1, &bReferences);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( bReferences ) {
			*pReferences = 1;
			return XGE_OK;
		}
	}
	return XGE_OK;
}

static int __xgeShapeExClipShapeAddRefEx(xge_shape_ex pShape, xge_shape_ex pClipShape, int iMode)
{
	int iRet;
	int bWouldCycle;

	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExValid(pClipShape) || (pShape == pClipShape) || !__xgeShapeExClipModeValid(iMode) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	bWouldCycle = 0;
	iRet = __xgeShapeExClipShapeReferences(pClipShape, pShape, 0, &bWouldCycle);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( bWouldCycle ) {
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
	pShape->pClipShapes[pShape->iClipShapeCount] = pClipShape;
	pShape->pClipShapeModes[pShape->iClipShapeCount] = iMode;
	pShape->iClipShapeCount++;
	return XGE_OK;
}

static int __xgeShapeExClipShapeAddRef(xge_shape_ex pShape, xge_shape_ex pClipShape)
{
	return __xgeShapeExClipShapeAddRefEx(pShape, pClipShape, XGE_SHAPE_EX_CLIP_INTERSECT);
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
	int i;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (iPointCount < 0) || ((pPoints == NULL) && (iPointCount > 0)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < iPointCount; i++ ) {
		if ( !__xgeShapeExPointFinite(pPoints[i]) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
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
	xge_vec2_t tPrev;
	int bHadCurrent;
	int iRet;

	tPoint.fX = fX;
	tPoint.fY = fY;
	tPrev = pShape->tCurrent;
	bHadCurrent = pShape->bHasCurrent;
	iRet = __xgeShapeExAppendCommand(pShape, XGE_SHAPE_EX_CMD_LINE_TO, &tPoint, 1);
	if ( iRet == XGE_OK ) {
		pShape->tCurrent = tPoint;
		pShape->tLastCubicControl = tPoint;
		pShape->tLastQuadControl = tPoint;
		pShape->bHasCurrent = 1;
		if ( !bHadCurrent ) {
			pShape->tStart = tPoint;
			pShape->bHasStart = 1;
		} else if ( !pShape->bHasStart ) {
			pShape->tStart = tPrev;
			pShape->bHasStart = 1;
		}
	}
	return iRet;
}

static int __xgeShapeExAddCubicTo(xge_shape_ex pShape, float fC1X, float fC1Y, float fC2X, float fC2Y, float fX, float fY)
{
	xge_vec2_t arrPoints[3];
	xge_vec2_t tPrev;
	int bHadCurrent;
	int iRet;

	arrPoints[0].fX = fC1X;
	arrPoints[0].fY = fC1Y;
	arrPoints[1].fX = fC2X;
	arrPoints[1].fY = fC2Y;
	arrPoints[2].fX = fX;
	arrPoints[2].fY = fY;
	tPrev = pShape->tCurrent;
	bHadCurrent = pShape->bHasCurrent;
	iRet = __xgeShapeExAppendCommand(pShape, XGE_SHAPE_EX_CMD_CUBIC_TO, arrPoints, 3);
	if ( iRet == XGE_OK ) {
		pShape->tLastCubicControl = arrPoints[1];
		pShape->tLastQuadControl = arrPoints[2];
		pShape->tCurrent = arrPoints[2];
		pShape->bHasCurrent = 1;
		if ( bHadCurrent && !pShape->bHasStart ) {
			pShape->tStart = tPrev;
			pShape->bHasStart = 1;
		}
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

static int __xgeShapeExQuadStepCount(xge_vec2_t tP0, xge_vec2_t tP1, xge_vec2_t tP2, float fTolerance)
{
	float fLen;
	int iSteps;

	if ( fTolerance <= 0.0f ) {
		fTolerance = XGE_SHAPE_EX_DEFAULT_TOLERANCE;
	}
	fLen = hypotf(tP1.fX - tP0.fX, tP1.fY - tP0.fY) +
	       hypotf(tP2.fX - tP1.fX, tP2.fY - tP1.fY);
	iSteps = (int)ceilf(fLen / (fTolerance * 18.0f));
	if ( iSteps < 3 ) iSteps = 3;
	if ( iSteps > 64 ) iSteps = 64;
	return iSteps;
}

static xge_vec2_t __xgeShapeExQuadPoint(xge_vec2_t tP0, xge_vec2_t tP1, xge_vec2_t tP2, float t)
{
	float u;
	xge_vec2_t tOut;

	u = 1.0f - t;
	tOut.fX = (u * u * tP0.fX) + (2.0f * u * t * tP1.fX) + (t * t * tP2.fX);
	tOut.fY = (u * u * tP0.fY) + (2.0f * u * t * tP1.fY) + (t * t * tP2.fY);
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
		} else if ( iCommand == XGE_SHAPE_EX_CMD_LINE_TO ) {
			xge_vec2_t tPrev = tCurrent;

			tCurrent = __xgeShapeExMatrixPoint(tMatrix, pShape->pPoints[iPointRead++]);
			if ( !bHasCurrent ) {
				iRet = __xgeShapeExFlatBeginContour(pFlat, tCurrent);
				if ( iRet != XGE_OK ) {
					__xgeShapeExFlatFree(pFlat);
					return iRet;
				}
				tStart = tCurrent;
				bHasCurrent = 1;
				bHasStart = 1;
			} else {
				iRet = __xgeShapeExFlatAddPoint(pFlat, tCurrent);
				if ( iRet != XGE_OK ) {
					__xgeShapeExFlatFree(pFlat);
					return iRet;
				}
				if ( !bHasStart ) {
					tStart = tPrev;
					bHasStart = 1;
				}
			}
		} else if ( (iCommand == XGE_SHAPE_EX_CMD_QUAD_TO) && bHasCurrent ) {
			xge_vec2_t tP0 = tCurrent;
			xge_vec2_t tP1 = __xgeShapeExMatrixPoint(tMatrix, pShape->pPoints[iPointRead++]);
			xge_vec2_t tP2 = __xgeShapeExMatrixPoint(tMatrix, pShape->pPoints[iPointRead++]);
			int iSteps = __xgeShapeExQuadStepCount(tP0, tP1, tP2, fTolerance);
			int j;

			if ( !bHasStart ) {
				iRet = __xgeShapeExFlatBeginContour(pFlat, tP0);
				if ( iRet != XGE_OK ) {
					__xgeShapeExFlatFree(pFlat);
					return iRet;
				}
				tStart = tP0;
				bHasStart = 1;
			}
			for ( j = 1; j <= iSteps; j++ ) {
				xge_vec2_t tPoint = __xgeShapeExQuadPoint(tP0, tP1, tP2, (float)j / (float)iSteps);
				iRet = __xgeShapeExFlatAddPoint(pFlat, tPoint);
				if ( iRet != XGE_OK ) {
					__xgeShapeExFlatFree(pFlat);
					return iRet;
				}
			}
			tCurrent = tP2;
		} else if ( iCommand == XGE_SHAPE_EX_CMD_CUBIC_TO ) {
			xge_vec2_t tP0 = tCurrent;
			xge_vec2_t tP1 = __xgeShapeExMatrixPoint(tMatrix, pShape->pPoints[iPointRead++]);
			xge_vec2_t tP2 = __xgeShapeExMatrixPoint(tMatrix, pShape->pPoints[iPointRead++]);
			xge_vec2_t tP3 = __xgeShapeExMatrixPoint(tMatrix, pShape->pPoints[iPointRead++]);
			int iSteps = __xgeShapeExCubicStepCount(tP0, tP1, tP2, tP3, fTolerance);
			int j;

			if ( !bHasCurrent ) {
				tCurrent = tP3;
				bHasCurrent = 1;
				continue;
			}
			if ( !bHasStart ) {
				iRet = __xgeShapeExFlatBeginContour(pFlat, tP0);
				if ( iRet != XGE_OK ) {
					__xgeShapeExFlatFree(pFlat);
					return iRet;
				}
				tStart = tP0;
				bHasStart = 1;
			}
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

static int __xgeShapeExRectContainsPoint(xge_rect_t tRect, xge_vec2_t tPoint)
{
	return (tPoint.fX >= tRect.fX - XGE_SHAPE_EX_EPSILON) &&
	       (tPoint.fX <= tRect.fX + tRect.fW + XGE_SHAPE_EX_EPSILON) &&
	       (tPoint.fY >= tRect.fY - XGE_SHAPE_EX_EPSILON) &&
	       (tPoint.fY <= tRect.fY + tRect.fH + XGE_SHAPE_EX_EPSILON);
}

static int __xgeShapeExFlatContainsPoint(const xge_shape_ex_flat_path_t* pFlat, int iFillRule, xge_vec2_t tPoint)
{
	int iWinding;
	int iCrossings;
	int i;

	if ( pFlat == NULL ) {
		return 0;
	}
	iWinding = 0;
	iCrossings = 0;
	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		const xge_shape_ex_flat_contour_t* pContour = &pFlat->pContours[i];
		const xge_vec2_t* pPoints;
		int j;

		if ( (pContour->iCount < 3) || (pContour->iStart < 0) || ((pContour->iStart + pContour->iCount) > pFlat->iPointCount) ) {
			continue;
		}
		pPoints = pFlat->pPoints + pContour->iStart;
		for ( j = 0; j < pContour->iCount; j++ ) {
			xge_vec2_t tA = pPoints[j];
			xge_vec2_t tB = pPoints[(j + 1) % pContour->iCount];
			float fCross;

			if ( __xgeShapeExOnSegment(tA, tB, tPoint) ) {
				return 1;
			}
			if ( (tA.fY > tPoint.fY) == (tB.fY > tPoint.fY) ) {
				continue;
			}
			fCross = __xgeShapeExCross(tA, tB, tPoint);
			if ( iFillRule == XGE_SHAPE_EX_FILL_EVEN_ODD ) {
				float fX = ((tB.fX - tA.fX) * (tPoint.fY - tA.fY) / (tB.fY - tA.fY)) + tA.fX;

				if ( tPoint.fX < fX ) {
					iCrossings++;
				}
			} else if ( (tA.fY <= tPoint.fY) && (tB.fY > tPoint.fY) ) {
				if ( fCross > XGE_SHAPE_EX_EPSILON ) {
					iWinding++;
				}
			} else if ( (tA.fY > tPoint.fY) && (tB.fY <= tPoint.fY) ) {
				if ( fCross < -XGE_SHAPE_EX_EPSILON ) {
					iWinding--;
				}
			}
		}
	}
	return (iFillRule == XGE_SHAPE_EX_FILL_EVEN_ODD) ? ((iCrossings & 1) != 0) : (iWinding != 0);
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

static int __xgeShapeExSamePoint(xge_vec2_t tA, xge_vec2_t tB);

static int __xgeShapeExFlatHasIntersectingEdges(const xge_shape_ex_flat_path_t* pFlat)
{
	int i;

	if ( pFlat == NULL ) {
		return 0;
	}
	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		const xge_shape_ex_flat_contour_t* pAContour = &pFlat->pContours[i];
		const xge_vec2_t* pAPoints = pFlat->pPoints + pAContour->iStart;
		int iACount = pAContour->iCount;
		int iA;

		while ( (iACount > 2) && __xgeShapeExSamePoint(pAPoints[iACount - 1], pAPoints[0]) ) {
			iACount--;
		}
		if ( iACount < 3 ) {
			continue;
		}
		for ( iA = 0; iA < iACount; iA++ ) {
			xge_vec2_t tA0 = pAPoints[iA];
			xge_vec2_t tA1 = pAPoints[(iA + 1) % iACount];
			int j;

			if ( __xgeShapeExSamePoint(tA0, tA1) ) {
				continue;
			}
			for ( j = i; j < pFlat->iContourCount; j++ ) {
				const xge_shape_ex_flat_contour_t* pBContour = &pFlat->pContours[j];
				const xge_vec2_t* pBPoints = pFlat->pPoints + pBContour->iStart;
				int iBCount = pBContour->iCount;
				int iBStart = (j == i) ? (iA + 1) : 0;
				int iB;

				while ( (iBCount > 2) && __xgeShapeExSamePoint(pBPoints[iBCount - 1], pBPoints[0]) ) {
					iBCount--;
				}
				for ( iB = iBStart; iB < iBCount; iB++ ) {
					xge_vec2_t tB0;
					xge_vec2_t tB1;

					if ( (j == i) && ((iB == iA) || (iB == ((iA + 1) % iACount)) || (((iB + 1) % iACount) == iA)) ) {
						continue;
					}
					tB0 = pBPoints[iB];
					tB1 = pBPoints[(iB + 1) % iBCount];
					if ( __xgeShapeExSamePoint(tB0, tB1) ) {
						continue;
					}
					if ( __xgeShapeExSegmentsIntersect(tA0, tA1, tB0, tB1) ) {
						return 1;
					}
				}
			}
		}
	}
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
	if ( (iTriangleCount <= 0) || (iTriangleCount > ((INT_MAX - iVertexCount) / 3)) || (iTriangleCount > (INT_MAX / 12)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iNewVertexCount = iVertexCount + (iTriangleCount * 3);
	iNewIndexCount = iTriangleCount * 12;
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
		uint32_t iAB = (uint32_t)(iVertexCount + (i * 3));
		uint32_t iBC = iAB + 1u;
		uint32_t iCA = iAB + 2u;
		int iOut = i * 12;

		if ( (iA >= (uint32_t)iVertexCount) || (iB >= (uint32_t)iVertexCount) || (iC >= (uint32_t)iVertexCount) ) {
			xrtFree(pNewVertices);
			xrtFree(pNewIndices);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		pNewVertices[iAB].fX = (pVertices[iA].fX + pVertices[iB].fX) * 0.5f;
		pNewVertices[iAB].fY = (pVertices[iA].fY + pVertices[iB].fY) * 0.5f;
		pNewVertices[iAB].iColor = pVertices[iA].iColor;
		pNewVertices[iBC].fX = (pVertices[iB].fX + pVertices[iC].fX) * 0.5f;
		pNewVertices[iBC].fY = (pVertices[iB].fY + pVertices[iC].fY) * 0.5f;
		pNewVertices[iBC].iColor = pVertices[iB].iColor;
		pNewVertices[iCA].fX = (pVertices[iC].fX + pVertices[iA].fX) * 0.5f;
		pNewVertices[iCA].fY = (pVertices[iC].fY + pVertices[iA].fY) * 0.5f;
		pNewVertices[iCA].iColor = pVertices[iC].iColor;
		pNewIndices[iOut + 0] = iA;
		pNewIndices[iOut + 1] = iAB;
		pNewIndices[iOut + 2] = iCA;
		pNewIndices[iOut + 3] = iAB;
		pNewIndices[iOut + 4] = iB;
		pNewIndices[iOut + 5] = iBC;
		pNewIndices[iOut + 6] = iCA;
		pNewIndices[iOut + 7] = iBC;
		pNewIndices[iOut + 8] = iC;
		pNewIndices[iOut + 9] = iAB;
		pNewIndices[iOut + 10] = iBC;
		pNewIndices[iOut + 11] = iCA;
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

static int __xgeShapeExLinearGradientSubdivisionPasses(const xge_shape_ex_color_stop_t* pStops, int iStopCount, int iSpread)
{
	int i;

	if ( iSpread != XGE_SHAPE_EX_GRADIENT_SPREAD_PAD ) {
		return 6;
	}
	for ( i = 1; (pStops != NULL) && (i < iStopCount); i++ ) {
		if ( fabsf(pStops[i].fOffset - pStops[i - 1].fOffset) <= XGE_SHAPE_EX_EPSILON ) {
			return 7;
		}
	}
	if ( iStopCount > 4 ) {
		return 5;
	}
	if ( iStopCount > 2 ) {
		return 4;
	}
	return 0;
}

static int __xgeShapeExRadialGradientCorrectionPasses(float fCX, float fCY, float fRadius, float fFX, float fFY, float fFocalRadius)
{
	float fDX;
	float fDY;
	float fDist;
	float fMaxFR;

	if ( fRadius < 0.02f ) {
		return 0;
	}
	fDX = fCX - fFX;
	fDY = fCY - fFY;
	fDist = sqrtf((fDX * fDX) + (fDY * fDY));
	if ( (fRadius - fDist) < 0.02f ) {
		return 1;
	}
	fMaxFR = (fRadius - fDist) * 0.98f;
	return fFocalRadius > fMaxFR ? 1 : 0;
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
				pShape->fStrokeFR,
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
		int iPassCount = bUsedCenterFan ? 5 : 4;

		if ( pShape->iFillGradientSpread != XGE_SHAPE_EX_GRADIENT_SPREAD_PAD ) {
			iPassCount = 6;
		}
		iPassCount += __xgeShapeExRadialGradientCorrectionPasses(
			pShape->fFillX1, pShape->fFillY1, pShape->fFillR,
			pShape->fFillFX, pShape->fFillFY, pShape->fFillFR
		);

		for ( iPass = 0; iPass < iPassCount; iPass++ ) {
			iRet = __xgeShapeExSubdivideTriangles(&pVertices, &iVertexCount, &pIndices, &iIndexCount);
			if ( iRet != XGE_OK ) {
				xrtFree(pVertices);
				xrtFree(pIndices);
				return iRet;
			}
		}
	}
	if ( (pShape != NULL) && (pShape->iFillType == XGE_SHAPE_EX_FILL_LINEAR_GRADIENT) && (iIndexCount > 0) ) {
		int iPassCount = __xgeShapeExLinearGradientSubdivisionPasses(pShape->pFillStops, pShape->iFillStopCount, pShape->iFillGradientSpread);

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

typedef struct xge_shape_ex_scan_edge_t {
	xge_vec2_t tA;
	xge_vec2_t tB;
	float fXMid;
	int iWinding;
} xge_shape_ex_scan_edge_t;

typedef struct xge_shape_ex_scan_mesh_t {
	xge_shape_vertex_t* pVertices;
	uint32_t* pIndices;
	int iVertexCount;
	int iIndexCount;
	int iVertexCapacity;
	int iIndexCapacity;
} xge_shape_ex_scan_mesh_t;

static int __xgeShapeExFloatCompare(const void* pA, const void* pB)
{
	float fA = *(const float*)pA;
	float fB = *(const float*)pB;

	return (fA > fB) - (fA < fB);
}

static int __xgeShapeExScanEdgeCompare(const void* pA, const void* pB)
{
	const xge_shape_ex_scan_edge_t* pEdgeA = (const xge_shape_ex_scan_edge_t*)pA;
	const xge_shape_ex_scan_edge_t* pEdgeB = (const xge_shape_ex_scan_edge_t*)pB;

	if ( pEdgeA->fXMid < pEdgeB->fXMid ) return -1;
	if ( pEdgeA->fXMid > pEdgeB->fXMid ) return 1;
	return pEdgeA->iWinding - pEdgeB->iWinding;
}

static float __xgeShapeExScanEdgeXAt(const xge_shape_ex_scan_edge_t* pEdge, float fY)
{
	float fDY = pEdge->tB.fY - pEdge->tA.fY;

	if ( fabsf(fDY) <= XGE_SHAPE_EX_EPSILON ) {
		return pEdge->tA.fX;
	}
	return pEdge->tA.fX + ((fY - pEdge->tA.fY) * (pEdge->tB.fX - pEdge->tA.fX) / fDY);
}

static int __xgeShapeExScanAppendLevel(float** ppLevels, int* pCount, int* pCapacity, float fY)
{
	float* pLevels;
	int iCapacity;

	if ( (*pCount + 1) > *pCapacity ) {
		iCapacity = (*pCapacity > 0) ? (*pCapacity * 2) : 32;
		while ( iCapacity < (*pCount + 1) ) {
			iCapacity *= 2;
		}
		pLevels = (float*)xrtRealloc(*ppLevels, (size_t)iCapacity * sizeof(*pLevels));
		if ( pLevels == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		*ppLevels = pLevels;
		*pCapacity = iCapacity;
	}
	(*ppLevels)[(*pCount)++] = fY;
	return XGE_OK;
}

static int __xgeShapeExScanProperIntersectionY(const xge_shape_ex_scan_edge_t* pA, const xge_shape_ex_scan_edge_t* pB, float* pY)
{
	float fRX = pA->tB.fX - pA->tA.fX;
	float fRY = pA->tB.fY - pA->tA.fY;
	float fSX = pB->tB.fX - pB->tA.fX;
	float fSY = pB->tB.fY - pB->tA.fY;
	float fQPX = pB->tA.fX - pA->tA.fX;
	float fQPY = pB->tA.fY - pA->tA.fY;
	float fDenominator = (fRX * fSY) - (fRY * fSX);
	float fT;
	float fU;

	if ( fabsf(fDenominator) <= XGE_SHAPE_EX_EPSILON ) {
		return 0;
	}
	fT = ((fQPX * fSY) - (fQPY * fSX)) / fDenominator;
	fU = ((fQPX * fRY) - (fQPY * fRX)) / fDenominator;
	if ( (fT <= XGE_SHAPE_EX_EPSILON) || (fT >= 1.0f - XGE_SHAPE_EX_EPSILON) ||
	     (fU <= XGE_SHAPE_EX_EPSILON) || (fU >= 1.0f - XGE_SHAPE_EX_EPSILON) ) {
		return 0;
	}
	*pY = pA->tA.fY + fT * fRY;
	return 1;
}

static int __xgeShapeExScanMeshReserve(xge_shape_ex_scan_mesh_t* pMesh, int iAddVertices, int iAddIndices)
{
	int iCapacity;

	if ( (pMesh->iVertexCount + iAddVertices) > pMesh->iVertexCapacity ) {
		xge_shape_vertex_t* pVertices;

		iCapacity = (pMesh->iVertexCapacity > 0) ? (pMesh->iVertexCapacity * 2) : 64;
		while ( iCapacity < (pMesh->iVertexCount + iAddVertices) ) iCapacity *= 2;
		pVertices = (xge_shape_vertex_t*)xrtRealloc(pMesh->pVertices, (size_t)iCapacity * sizeof(*pVertices));
		if ( pVertices == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
		pMesh->pVertices = pVertices;
		pMesh->iVertexCapacity = iCapacity;
	}
	if ( (pMesh->iIndexCount + iAddIndices) > pMesh->iIndexCapacity ) {
		uint32_t* pIndices;

		iCapacity = (pMesh->iIndexCapacity > 0) ? (pMesh->iIndexCapacity * 2) : 96;
		while ( iCapacity < (pMesh->iIndexCount + iAddIndices) ) iCapacity *= 2;
		pIndices = (uint32_t*)xrtRealloc(pMesh->pIndices, (size_t)iCapacity * sizeof(*pIndices));
		if ( pIndices == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
		pMesh->pIndices = pIndices;
		pMesh->iIndexCapacity = iCapacity;
	}
	return XGE_OK;
}

static int __xgeShapeExScanMeshAppendBand(xge_shape_ex_scan_mesh_t* pMesh, const xge_shape_ex_scan_edge_t* pLeft, const xge_shape_ex_scan_edge_t* pRight, float fY0, float fY1, uint32_t iColor)
{
	float fLeft0 = __xgeShapeExScanEdgeXAt(pLeft, fY0);
	float fLeft1 = __xgeShapeExScanEdgeXAt(pLeft, fY1);
	float fRight0 = __xgeShapeExScanEdgeXAt(pRight, fY0);
	float fRight1 = __xgeShapeExScanEdgeXAt(pRight, fY1);
	uint32_t iBase;
	int iRet;

	if ( (fRight0 <= fLeft0 + XGE_SHAPE_EX_EPSILON) && (fRight1 <= fLeft1 + XGE_SHAPE_EX_EPSILON) ) {
		return XGE_OK;
	}
	iRet = __xgeShapeExScanMeshReserve(pMesh, 4, 6);
	if ( iRet != XGE_OK ) return iRet;
	iBase = (uint32_t)pMesh->iVertexCount;
	pMesh->pVertices[pMesh->iVertexCount++] = (xge_shape_vertex_t){fLeft0, fY0, iColor};
	pMesh->pVertices[pMesh->iVertexCount++] = (xge_shape_vertex_t){fRight0, fY0, iColor};
	pMesh->pVertices[pMesh->iVertexCount++] = (xge_shape_vertex_t){fRight1, fY1, iColor};
	pMesh->pVertices[pMesh->iVertexCount++] = (xge_shape_vertex_t){fLeft1, fY1, iColor};
	pMesh->pIndices[pMesh->iIndexCount++] = iBase;
	pMesh->pIndices[pMesh->iIndexCount++] = iBase + 1u;
	pMesh->pIndices[pMesh->iIndexCount++] = iBase + 2u;
	pMesh->pIndices[pMesh->iIndexCount++] = iBase;
	pMesh->pIndices[pMesh->iIndexCount++] = iBase + 2u;
	pMesh->pIndices[pMesh->iIndexCount++] = iBase + 3u;
	return XGE_OK;
}

static int __xgeShapeExBuildScanlineFillMesh(const xge_shape_ex_flat_path_t* pFlat, int iFillRule, uint32_t iColor, xge_shape_ex_scan_mesh_t* pMesh)
{
	xge_shape_ex_scan_edge_t* pEdges;
	xge_shape_ex_scan_edge_t* pActive;
	float* pLevels;
	int iEdgeCount;
	int iEdgeCapacity;
	int iLevelCount;
	int iLevelCapacity;
	int i;
	int iRet;

	memset(pMesh, 0, sizeof(*pMesh));
	pEdges = NULL;
	pActive = NULL;
	pLevels = NULL;
	iEdgeCount = 0;
	iEdgeCapacity = 0;
	iLevelCount = 0;
	iLevelCapacity = 0;
	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		const xge_shape_ex_flat_contour_t* pContour = &pFlat->pContours[i];
		const xge_vec2_t* pPoints = pFlat->pPoints + pContour->iStart;
		int iCount = pContour->iCount;
		int j;

		while ( (iCount > 2) && __xgeShapeExSamePoint(pPoints[iCount - 1], pPoints[0]) ) iCount--;
		if ( iCount < 3 ) continue;
		for ( j = 0; j < iCount; j++ ) {
			xge_vec2_t tA = pPoints[j];
			xge_vec2_t tB = pPoints[(j + 1) % iCount];

			iRet = __xgeShapeExScanAppendLevel(&pLevels, &iLevelCount, &iLevelCapacity, tA.fY);
			if ( iRet != XGE_OK ) goto xge_shape_ex_scanline_fail;
			if ( fabsf(tB.fY - tA.fY) <= XGE_SHAPE_EX_EPSILON ) continue;
			if ( (iEdgeCount + 1) > iEdgeCapacity ) {
				xge_shape_ex_scan_edge_t* pNewEdges;
				iEdgeCapacity = (iEdgeCapacity > 0) ? (iEdgeCapacity * 2) : 32;
				pNewEdges = (xge_shape_ex_scan_edge_t*)xrtRealloc(pEdges, (size_t)iEdgeCapacity * sizeof(*pNewEdges));
				if ( pNewEdges == NULL ) {
					iRet = XGE_ERROR_OUT_OF_MEMORY;
					goto xge_shape_ex_scanline_fail;
				}
				pEdges = pNewEdges;
			}
			pEdges[iEdgeCount].tA = tA;
			pEdges[iEdgeCount].tB = tB;
			pEdges[iEdgeCount].fXMid = 0.0f;
			pEdges[iEdgeCount].iWinding = (tA.fY < tB.fY) ? 1 : -1;
			iEdgeCount++;
		}
	}
	for ( i = 0; i < iEdgeCount; i++ ) {
		int j;

		for ( j = i + 1; j < iEdgeCount; j++ ) {
			float fY;

			if ( __xgeShapeExScanProperIntersectionY(&pEdges[i], &pEdges[j], &fY) ) {
				iRet = __xgeShapeExScanAppendLevel(&pLevels, &iLevelCount, &iLevelCapacity, fY);
				if ( iRet != XGE_OK ) goto xge_shape_ex_scanline_fail;
			}
		}
	}
	if ( (iEdgeCount <= 0) || (iLevelCount < 2) ) {
		iRet = XGE_OK;
		goto xge_shape_ex_scanline_done;
	}
	qsort(pLevels, (size_t)iLevelCount, sizeof(*pLevels), __xgeShapeExFloatCompare);
	{
		int iWrite = 0;

		for ( i = 0; i < iLevelCount; i++ ) {
			if ( (iWrite == 0) || (fabsf(pLevels[i] - pLevels[iWrite - 1]) > XGE_SHAPE_EX_EPSILON) ) {
				pLevels[iWrite++] = pLevels[i];
			}
		}
		iLevelCount = iWrite;
	}
	pActive = (xge_shape_ex_scan_edge_t*)xrtMalloc((size_t)iEdgeCount * sizeof(*pActive));
	if ( pActive == NULL ) {
		iRet = XGE_ERROR_OUT_OF_MEMORY;
		goto xge_shape_ex_scanline_fail;
	}
	for ( i = 0; i + 1 < iLevelCount; i++ ) {
		float fY0 = pLevels[i];
		float fY1 = pLevels[i + 1];
		float fYMid;
		int iActiveCount;
		int j;

		if ( fY1 <= fY0 + XGE_SHAPE_EX_EPSILON ) continue;
		fYMid = (fY0 + fY1) * 0.5f;
		iActiveCount = 0;
		for ( j = 0; j < iEdgeCount; j++ ) {
			float fMinY = fminf(pEdges[j].tA.fY, pEdges[j].tB.fY);
			float fMaxY = fmaxf(pEdges[j].tA.fY, pEdges[j].tB.fY);

			if ( (fYMid <= fMinY) || (fYMid >= fMaxY) ) continue;
			pActive[iActiveCount] = pEdges[j];
			pActive[iActiveCount].fXMid = __xgeShapeExScanEdgeXAt(&pActive[iActiveCount], fYMid);
			iActiveCount++;
		}
		if ( iActiveCount < 2 ) continue;
		qsort(pActive, (size_t)iActiveCount, sizeof(*pActive), __xgeShapeExScanEdgeCompare);
		{
			int iState = 0;
			int bInside = 0;
			xge_shape_ex_scan_edge_t tLeft;

			for ( j = 0; j < iActiveCount; ) {
				int k = j + 1;
				int iDelta = pActive[j].iWinding;
				int bWasInside = bInside;

				while ( (k < iActiveCount) && (fabsf(pActive[k].fXMid - pActive[j].fXMid) <= XGE_SHAPE_EX_EPSILON) ) {
					iDelta += pActive[k].iWinding;
					k++;
				}
				if ( iFillRule == XGE_SHAPE_EX_FILL_EVEN_ODD ) iState += (k - j);
				else iState += iDelta;
				bInside = (iFillRule == XGE_SHAPE_EX_FILL_EVEN_ODD) ? ((iState & 1) != 0) : (iState != 0);
				if ( !bWasInside && bInside ) {
					tLeft = pActive[j];
				} else if ( bWasInside && !bInside ) {
					iRet = __xgeShapeExScanMeshAppendBand(pMesh, &tLeft, &pActive[j], fY0, fY1, iColor);
					if ( iRet != XGE_OK ) goto xge_shape_ex_scanline_fail;
				}
				j = k;
			}
		}
	}
	iRet = XGE_OK;
	goto xge_shape_ex_scanline_done;

xge_shape_ex_scanline_fail:
	xrtFree(pMesh->pVertices);
	xrtFree(pMesh->pIndices);
	memset(pMesh, 0, sizeof(*pMesh));
xge_shape_ex_scanline_done:
	xrtFree(pEdges);
	xrtFree(pActive);
	xrtFree(pLevels);
	return iRet;
}

static int __xgeShapeExDrawScanlineFill(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, int iFillRule, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, float fOpacity, uint32_t iColor, int bScreenSpace)
{
	xge_shape_ex_scan_mesh_t tMesh;
	int iRet;
	int iPass;

	iRet = __xgeShapeExBuildScanlineFillMesh(pFlat, iFillRule, iColor, &tMesh);
	if ( iRet != XGE_OK ) return iRet;
	if ( (pShape != NULL) && (pShape->iFillType == XGE_SHAPE_EX_FILL_RADIAL_GRADIENT) && (tMesh.iIndexCount > 0) ) {
		int iPassCount = (pShape->iFillGradientSpread == XGE_SHAPE_EX_GRADIENT_SPREAD_PAD) ? 4 : 6;

		iPassCount += __xgeShapeExRadialGradientCorrectionPasses(
			pShape->fFillX1, pShape->fFillY1, pShape->fFillR,
			pShape->fFillFX, pShape->fFillFY, pShape->fFillFR
		);
		for ( iPass = 0; iPass < iPassCount; iPass++ ) {
			iRet = __xgeShapeExSubdivideTriangles(&tMesh.pVertices, &tMesh.iVertexCount, &tMesh.pIndices, &tMesh.iIndexCount);
			if ( iRet != XGE_OK ) goto xge_shape_ex_scanline_draw_done;
		}
	} else if ( (pShape != NULL) && (pShape->iFillType == XGE_SHAPE_EX_FILL_LINEAR_GRADIENT) && (tMesh.iIndexCount > 0) ) {
		int iPassCount = __xgeShapeExLinearGradientSubdivisionPasses(pShape->pFillStops, pShape->iFillStopCount, pShape->iFillGradientSpread);

		for ( iPass = 0; iPass < iPassCount; iPass++ ) {
			iRet = __xgeShapeExSubdivideTriangles(&tMesh.pVertices, &tMesh.iVertexCount, &tMesh.pIndices, &tMesh.iIndexCount);
			if ( iRet != XGE_OK ) goto xge_shape_ex_scanline_draw_done;
		}
	}
	__xgeShapeExApplyFillPaint(pShape, tMesh.pVertices, tMesh.iVertexCount, tBounds, tMatrix, iColor, fOpacity);
	if ( tMesh.iIndexCount > 0 ) {
		iRet = bScreenSpace ? xgeShapeMeshFillPx(tMesh.pVertices, tMesh.iVertexCount, tMesh.pIndices, tMesh.iIndexCount) :
		                       xgeShapeMeshFill(tMesh.pVertices, tMesh.iVertexCount, tMesh.pIndices, tMesh.iIndexCount);
	} else {
		iRet = XGE_OK;
	}

xge_shape_ex_scanline_draw_done:
	xrtFree(tMesh.pVertices);
	xrtFree(tMesh.pIndices);
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
	if ( __xgeShapeExFlatHasIntersectingEdges(pFlat) ) {
		return __xgeShapeExDrawScanlineFill(pShape, pFlat, iFillRule, tBounds, tMatrix, fOpacity, iColor, bScreenSpace);
	}
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

static void __xgeShapeExTransformVertices(xge_shape_vertex_t* pVertices, int iVertexCount, xge_shape_ex_matrix_t tMatrix)
{
	int i;

	if ( pVertices == NULL ) {
		return;
	}
	for ( i = 0; i < iVertexCount; i++ ) {
		xge_vec2_t tPoint;

		tPoint.fX = pVertices[i].fX;
		tPoint.fY = pVertices[i].fY;
		tPoint = __xgeShapeExMatrixPoint(tMatrix, tPoint);
		pVertices[i].fX = tPoint.fX;
		pVertices[i].fY = tPoint.fY;
	}
}

static int __xgeShapeExDrawStrokePaintMesh(xge_shape_ex pShape, xge_shape_vertex_t* pVertices, int iVertexCount, const uint32_t* pIndices, int iIndexCount, uint32_t iColor, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, const xge_shape_ex_matrix_t* pGeometryMatrix, float fOpacity, int bScreenSpace)
{
	xge_shape_vertex_t* pDrawVertices;
	uint32_t* pDrawIndices;
	int iDrawVertexCount;
	int iDrawIndexCount;
	int bUseTempMesh;
	int iRet;

	if ( (pVertices == NULL) || (iVertexCount <= 0) || (pIndices == NULL) || (iIndexCount <= 0) ) {
		return XGE_OK;
	}
	pDrawVertices = pVertices;
	pDrawIndices = (uint32_t*)pIndices;
	iDrawVertexCount = iVertexCount;
	iDrawIndexCount = iIndexCount;
	bUseTempMesh = 0;
	if ( (pShape != NULL) && ((pShape->iStrokeType == XGE_SHAPE_EX_FILL_RADIAL_GRADIENT) || (pShape->iStrokeType == XGE_SHAPE_EX_FILL_LINEAR_GRADIENT)) && (iIndexCount >= 3) ) {
		int iPass;
		int iPassCount;

		if ( pShape->iStrokeType == XGE_SHAPE_EX_FILL_RADIAL_GRADIENT ) {
			iPassCount = (pShape->iStrokeGradientSpread == XGE_SHAPE_EX_GRADIENT_SPREAD_PAD) ? 5 : 6;
			iPassCount += __xgeShapeExRadialGradientCorrectionPasses(
				pShape->fStrokeX1, pShape->fStrokeY1, pShape->fStrokeR,
				pShape->fStrokeFX, pShape->fStrokeFY, pShape->fStrokeFR
			);
		} else {
			iPassCount = __xgeShapeExLinearGradientSubdivisionPasses(pShape->pStrokeStops, pShape->iStrokeStopCount, pShape->iStrokeGradientSpread);
		}
		if ( iPassCount <= 0 ) {
			goto xge_shape_ex_stroke_paint_no_subdivide;
		}
		pDrawVertices = (xge_shape_vertex_t*)xrtMalloc((size_t)iVertexCount * sizeof(*pDrawVertices));
		pDrawIndices = (uint32_t*)xrtMalloc((size_t)iIndexCount * sizeof(*pDrawIndices));
		if ( (pDrawVertices == NULL) || (pDrawIndices == NULL) ) {
			xrtFree(pDrawVertices);
			xrtFree(pDrawIndices);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memcpy(pDrawVertices, pVertices, (size_t)iVertexCount * sizeof(*pDrawVertices));
		memcpy(pDrawIndices, pIndices, (size_t)iIndexCount * sizeof(*pDrawIndices));
		bUseTempMesh = 1;
		for ( iPass = 0; iPass < iPassCount; iPass++ ) {
			iRet = __xgeShapeExSubdivideTriangles(&pDrawVertices, &iDrawVertexCount, &pDrawIndices, &iDrawIndexCount);
			if ( iRet != XGE_OK ) {
				xrtFree(pDrawVertices);
				xrtFree(pDrawIndices);
				return iRet;
			}
		}
	}
xge_shape_ex_stroke_paint_no_subdivide:
	__xgeShapeExApplyStrokePaint(pShape, pDrawVertices, iDrawVertexCount, tBounds, tMatrix, iColor, fOpacity);
	if ( pGeometryMatrix != NULL ) {
		__xgeShapeExTransformVertices(pDrawVertices, iDrawVertexCount, *pGeometryMatrix);
	}
	iRet = bScreenSpace ? xgeShapeMeshFillPx(pDrawVertices, iDrawVertexCount, pDrawIndices, iDrawIndexCount) : xgeShapeMeshFill(pDrawVertices, iDrawVertexCount, pDrawIndices, iDrawIndexCount);
	if ( bUseTempMesh ) {
		xrtFree(pDrawVertices);
		xrtFree(pDrawIndices);
	}
	return iRet;
}

static int __xgeShapeExDrawStrokeRoundCap(xge_shape_ex pShape, xge_vec2_t tCenter, float fDirX, float fDirY, float fHalfWidth, int bEndCap, uint32_t iColor, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, const xge_shape_ex_matrix_t* pGeometryMatrix, float fOpacity, int bScreenSpace)
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
	return __xgeShapeExDrawStrokePaintMesh(pShape, tVertices, iSegments + 2, iIndices, iSegments * 3, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
}

static int __xgeShapeExDrawStrokeZeroLengthCap(xge_shape_ex pShape, xge_vec2_t tCenter, float fWidth, uint32_t iColor, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, const xge_shape_ex_matrix_t* pGeometryMatrix, float fOpacity, int bScreenSpace)
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
		return __xgeShapeExDrawStrokePaintMesh(pShape, tVertices, iSegments + 2, iIndices, iSegments * 3, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
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
		return __xgeShapeExDrawStrokePaintMesh(pShape, tVertices, 4, iIndices, 6, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
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

static int __xgeShapeExDrawStrokeJoin(xge_shape_ex pShape, xge_vec2_t tPrev, xge_vec2_t tCurr, xge_vec2_t tNext, float fWidth, uint32_t iColor, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, const xge_shape_ex_matrix_t* pGeometryMatrix, float fOpacity, int bScreenSpace)
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
		return __xgeShapeExDrawStrokePaintMesh(pShape, tVertices, iSegments + 2, iIndices, iSegments * 3, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
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
			return __xgeShapeExDrawStrokePaintMesh(pShape, tVertices, iVertexCount, iIndices, 3, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
		}
	}
	tVertices[iVertexCount].fX = tCurr.fX; tVertices[iVertexCount].fY = tCurr.fY; tVertices[iVertexCount++].iColor = iColor;
	tVertices[iVertexCount].fX = tCurr.fX + tOuter1.fX; tVertices[iVertexCount].fY = tCurr.fY + tOuter1.fY; tVertices[iVertexCount++].iColor = iColor;
	tVertices[iVertexCount].fX = tCurr.fX + tOuter2.fX; tVertices[iVertexCount].fY = tCurr.fY + tOuter2.fY; tVertices[iVertexCount++].iColor = iColor;
	iIndices[0] = 0; iIndices[1] = 1; iIndices[2] = 2;
	return __xgeShapeExDrawStrokePaintMesh(pShape, tVertices, iVertexCount, iIndices, 3, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
}

static int __xgeShapeExDrawStrokeSegment(xge_shape_ex pShape, xge_vec2_t tA, xge_vec2_t tB, float fWidth, uint32_t iColor, int bCapStart, int bCapEnd, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, const xge_shape_ex_matrix_t* pGeometryMatrix, float fOpacity, int bScreenSpace)
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
		int iRet;

		if ( fLen <= XGE_SHAPE_EX_EPSILON ) {
			if ( !bCapStart && !bCapEnd ) {
				return XGE_OK;
			}
			return __xgeShapeExDrawStrokeZeroLengthCap(pShape, tA, fWidth, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
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
		iRet = __xgeShapeExDrawStrokePaintMesh(pShape, tVertices, 4, iIndices, 6, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( pShape->iLineCap == XGE_SHAPE_EX_CAP_ROUND ) {
			if ( bCapStart ) {
				iRet = __xgeShapeExDrawStrokeRoundCap(pShape, tA, fDirX, fDirY, fHalfWidth, 0, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
			}
			if ( bCapEnd ) {
				iRet = __xgeShapeExDrawStrokeRoundCap(pShape, tB, fDirX, fDirY, fHalfWidth, 1, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
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

static int __xgeShapeExDrawStrokeContourMesh(xge_shape_ex pShape, const xge_vec2_t* pSrc, int iCount, int bClosed, float fWidth, uint32_t iColor, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, const xge_shape_ex_matrix_t* pGeometryMatrix, float fOpacity, int bScreenSpace)
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
			return __xgeShapeExDrawStrokeZeroLengthCap(pShape, tPoint, fWidth, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
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
	iRet = __xgeShapeExDrawStrokePaintMesh(pShape, pVertices, iPointCount * 2, pIndices, iIndexCount, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
	if ( (iRet == XGE_OK) && !bClosed && (pShape->iLineCap == XGE_SHAPE_EX_CAP_ROUND) ) {
		xge_vec2_t tDir;

		if ( __xgeShapeExNormalizeSegment(pPoints[0], pPoints[1], &tDir) ) {
			iRet = __xgeShapeExDrawStrokeRoundCap(pShape, pPoints[0], tDir.fX, tDir.fY, fHalfWidth, 0, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
		}
		if ( (iRet == XGE_OK) && __xgeShapeExNormalizeSegment(pPoints[iPointCount - 2], pPoints[iPointCount - 1], &tDir) ) {
			iRet = __xgeShapeExDrawStrokeRoundCap(pShape, pPoints[iPointCount - 1], tDir.fX, tDir.fY, fHalfWidth, 1, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
		}
	}
	xrtFree(pVertices);
	xrtFree(pIndices);
	xrtFree(pPoints);
	return iRet;
}

static int __xgeShapeExDrawStrokeSegmentJoinContour(xge_shape_ex pShape, const xge_vec2_t* pSrc, int iCount, int bClosed, float fWidth, uint32_t iColor, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, const xge_shape_ex_matrix_t* pGeometryMatrix, float fOpacity, int bScreenSpace)
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
			return __xgeShapeExDrawStrokeZeroLengthCap(pShape, tPoint, fWidth, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
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

		iRet = __xgeShapeExDrawStrokeSegment(pShape, tA, tB, fWidth, iColor, bCapStart, bCapEnd, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
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

			iRet = __xgeShapeExDrawStrokeJoin(pShape, pPoints[iPrev], pPoints[i], pPoints[iNext], fWidth, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
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

static int __xgeShapeExDashLogicalCount(int iDashCount)
{
	if ( iDashCount <= 0 ) {
		return 0;
	}
	if ( (iDashCount & 1) != 0 ) {
		if ( iDashCount > (INT_MAX / 2) ) {
			return 0;
		}
		return iDashCount * 2;
	}
	return iDashCount;
}

static float __xgeShapeExDashTotal(const float* pDashPattern, int iDashCount)
{
	float fDashTotal;
	int iLogicalCount;
	int i;

	if ( (pDashPattern == NULL) || (iDashCount <= 0) ) {
		return 0.0f;
	}
	iLogicalCount = __xgeShapeExDashLogicalCount(iDashCount);
	fDashTotal = 0.0f;
	for ( i = 0; i < iLogicalCount; i++ ) {
		float fValue = pDashPattern[i % iDashCount];

		if ( fValue > 0.0f ) {
			fDashTotal += fValue;
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
	int iLogicalCount;

	*pDashIndex = 0;
	*pDashOn = 1;
	*pDashRemaining = (iDashCount > 0) ? pDashPattern[0] : 0.0f;
	iLogicalCount = __xgeShapeExDashLogicalCount(iDashCount);
	fDashTotal = __xgeShapeExDashTotal(pDashPattern, iDashCount);
	if ( (iLogicalCount <= 0) || (fDashTotal <= XGE_SHAPE_EX_EPSILON) ) {
		return;
	}
	fOffset = fmodf(fDashOffset, fDashTotal);
	if ( fOffset < 0.0f ) {
		fOffset += fDashTotal;
	}
	while ( fOffset > XGE_SHAPE_EX_EPSILON ) {
		float fStep = pDashPattern[*pDashIndex % iDashCount];

		if ( fStep <= XGE_SHAPE_EX_EPSILON ) {
			*pDashIndex = (*pDashIndex + 1) % iLogicalCount;
			*pDashOn = !*pDashOn;
			continue;
		}
		if ( fStep > fOffset ) {
			*pDashRemaining = fStep - fOffset;
			return;
		}
		fOffset -= fStep;
		*pDashIndex = (*pDashIndex + 1) % iLogicalCount;
		*pDashOn = !*pDashOn;
	}
	*pDashRemaining = pDashPattern[*pDashIndex % iDashCount];
}

static void __xgeShapeExDashStateNext(const float* pDashPattern, int iDashCount, int* pDashIndex, int* pDashOn, float* pDashRemaining)
{
	int iLogicalCount;
	int iGuard;

	iLogicalCount = __xgeShapeExDashLogicalCount(iDashCount);
	if ( (pDashPattern == NULL) || (iLogicalCount <= 0) ) {
		*pDashRemaining = 0.0f;
		return;
	}
	iGuard = 0;
	do {
		*pDashIndex = (*pDashIndex + 1) % iLogicalCount;
		*pDashOn = !*pDashOn;
		*pDashRemaining = pDashPattern[*pDashIndex % iDashCount];
		iGuard++;
	} while ( (*pDashRemaining <= XGE_SHAPE_EX_EPSILON) && (iGuard <= iLogicalCount) );
}

static int __xgeShapeExStrokePointInZeroCap(xge_shape_ex pShape, xge_vec2_t tCenter, float fHalfWidth, float fTolerance, xge_vec2_t tPoint)
{
	float fHitWidth;
	float fDX;
	float fDY;

	if ( (pShape == NULL) || (fHalfWidth <= XGE_SHAPE_EX_EPSILON) ) {
		return 0;
	}
	if ( pShape->iLineCap == XGE_SHAPE_EX_CAP_BUTT ) {
		return 0;
	}
	fHitWidth = fHalfWidth + fmaxf(fTolerance, XGE_SHAPE_EX_EPSILON);
	fDX = tPoint.fX - tCenter.fX;
	fDY = tPoint.fY - tCenter.fY;
	if ( pShape->iLineCap == XGE_SHAPE_EX_CAP_ROUND ) {
		return (fDX * fDX + fDY * fDY) <= (fHitWidth * fHitWidth);
	}
	return (fabsf(fDX) <= fHitWidth) && (fabsf(fDY) <= fHitWidth);
}

static int __xgeShapeExStrokeSegmentContainsPoint(xge_shape_ex pShape, xge_vec2_t tA, xge_vec2_t tB, int bCapStart, int bCapEnd, float fHalfWidth, float fTolerance, xge_vec2_t tPoint)
{
	float fDX;
	float fDY;
	float fLen;
	float fDirX;
	float fDirY;
	float fNX;
	float fNY;
	float fAlong;
	float fAcross;
	float fStart;
	float fEnd;
	float fHitWidth;

	if ( (pShape == NULL) || (fHalfWidth <= XGE_SHAPE_EX_EPSILON) ) {
		return 0;
	}
	fDX = tB.fX - tA.fX;
	fDY = tB.fY - tA.fY;
	fLen = hypotf(fDX, fDY);
	if ( fLen <= XGE_SHAPE_EX_EPSILON ) {
		return __xgeShapeExStrokePointInZeroCap(pShape, tA, fHalfWidth, fTolerance, tPoint);
	}
	fDirX = fDX / fLen;
	fDirY = fDY / fLen;
	fNX = -fDirY;
	fNY = fDirX;
	fStart = 0.0f;
	fEnd = fLen;
	if ( pShape->iLineCap == XGE_SHAPE_EX_CAP_SQUARE ) {
		if ( bCapStart ) {
			fStart -= fHalfWidth;
		}
		if ( bCapEnd ) {
			fEnd += fHalfWidth;
		}
	}
	fAlong = ((tPoint.fX - tA.fX) * fDirX) + ((tPoint.fY - tA.fY) * fDirY);
	fAcross = fabsf(((tPoint.fX - tA.fX) * fNX) + ((tPoint.fY - tA.fY) * fNY));
	fHitWidth = fHalfWidth + fmaxf(fTolerance, XGE_SHAPE_EX_EPSILON);
	if ( (fAlong >= fStart - fTolerance) && (fAlong <= fEnd + fTolerance) && (fAcross <= fHitWidth) ) {
		return 1;
	}
	if ( pShape->iLineCap == XGE_SHAPE_EX_CAP_ROUND ) {
		float fHitWidthSq = fHitWidth * fHitWidth;

		if ( bCapStart ) {
			float fCapDX = tPoint.fX - tA.fX;
			float fCapDY = tPoint.fY - tA.fY;

			if ( (fCapDX * fCapDX + fCapDY * fCapDY) <= fHitWidthSq ) {
				return 1;
			}
		}
		if ( bCapEnd ) {
			float fCapDX = tPoint.fX - tB.fX;
			float fCapDY = tPoint.fY - tB.fY;

			if ( (fCapDX * fCapDX + fCapDY * fCapDY) <= fHitWidthSq ) {
				return 1;
			}
		}
	}
	return 0;
}

static int __xgeShapeExStrokeJoinContainsPoint(xge_shape_ex pShape, xge_vec2_t tPrev, xge_vec2_t tCurr, xge_vec2_t tNext, float fHalfWidth, float fTolerance, xge_vec2_t tPoint)
{
	xge_vec2_t tD1;
	xge_vec2_t tD2;
	xge_vec2_t tN1;
	xge_vec2_t tN2;
	xge_vec2_t tOuter1;
	xge_vec2_t tOuter2;
	float fCross;
	float fHitHalfWidth;

	if ( (pShape == NULL) || (fHalfWidth <= XGE_SHAPE_EX_EPSILON) ) {
		return 0;
	}
	if ( !__xgeShapeExNormalizeSegment(tPrev, tCurr, &tD1) ||
	     !__xgeShapeExNormalizeSegment(tCurr, tNext, &tD2) ) {
		return 0;
	}
	fCross = tD1.fX * tD2.fY - tD1.fY * tD2.fX;
	if ( fabsf(fCross) <= XGE_SHAPE_EX_EPSILON ) {
		return 0;
	}
	fHitHalfWidth = fHalfWidth + fmaxf(fTolerance, XGE_SHAPE_EX_EPSILON);
	tN1.fX = -tD1.fY * fHitHalfWidth;
	tN1.fY =  tD1.fX * fHitHalfWidth;
	tN2.fX = -tD2.fY * fHitHalfWidth;
	tN2.fY =  tD2.fX * fHitHalfWidth;
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
		float fDX = tPoint.fX - tCurr.fX;
		float fDY = tPoint.fY - tCurr.fY;

		return (fDX * fDX + fDY * fDY) <= (fHitHalfWidth * fHitHalfWidth);
	}
	if ( pShape->iLineJoin == XGE_SHAPE_EX_JOIN_MITER ) {
		xge_vec2_t tMiter;
		xge_vec2_t tP1;
		xge_vec2_t tP2;

		tP1.fX = tCurr.fX + tOuter1.fX;
		tP1.fY = tCurr.fY + tOuter1.fY;
		tP2.fX = tCurr.fX + tOuter2.fX;
		tP2.fY = tCurr.fY + tOuter2.fY;
		if ( __xgeShapeExLineIntersection(tP1, tD1, tP2, tD2, &tMiter) &&
		     (hypotf(tMiter.fX - tCurr.fX, tMiter.fY - tCurr.fY) <= fHitHalfWidth * pShape->fMiterLimit) ) {
			return __xgeShapeExPointInTriangle(tPoint, tP1, tMiter, tP2);
		}
	}
	{
		xge_vec2_t tP1;
		xge_vec2_t tP2;

		tP1.fX = tCurr.fX + tOuter1.fX;
		tP1.fY = tCurr.fY + tOuter1.fY;
		tP2.fX = tCurr.fX + tOuter2.fX;
		tP2.fY = tCurr.fY + tOuter2.fY;
		return __xgeShapeExPointInTriangle(tPoint, tCurr, tP1, tP2);
	}
}

static int __xgeShapeExFlatStrokeSolidContainsPoint(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, float fStrokeWidth, float fTolerance, xge_vec2_t tPoint)
{
	float fHalfWidth;
	int i;

	if ( (pShape == NULL) || (pFlat == NULL) || (fStrokeWidth <= XGE_SHAPE_EX_EPSILON) ) {
		return 0;
	}
	fHalfWidth = fStrokeWidth * 0.5f;
	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		const xge_shape_ex_flat_contour_t* pContour = &pFlat->pContours[i];
		const xge_vec2_t* pSrc;
		xge_vec2_t* pPoints;
		int iPointCount;
		int iSegmentCount;
		int j;

		if ( (pContour->iCount <= 0) || (pContour->iStart < 0) || ((pContour->iStart + pContour->iCount) > pFlat->iPointCount) ) {
			continue;
		}
		pSrc = pFlat->pPoints + pContour->iStart;
		pPoints = NULL;
		iPointCount = 0;
		if ( __xgeShapeExCompactStrokePoints(pSrc, pContour->iCount, pContour->bClosed, &pPoints, &iPointCount) != XGE_OK ) {
			return 0;
		}
		if ( iPointCount == 1 ) {
			int bHit = __xgeShapeExStrokePointInZeroCap(pShape, pPoints[0], fHalfWidth, fTolerance, tPoint);

			xrtFree(pPoints);
			if ( bHit ) {
				return 1;
			}
			continue;
		}
		if ( iPointCount < 2 ) {
			xrtFree(pPoints);
			continue;
		}
		iSegmentCount = pContour->bClosed ? iPointCount : (iPointCount - 1);
		for ( j = 0; j < iSegmentCount; j++ ) {
			xge_vec2_t tA = pPoints[j];
			xge_vec2_t tB = pPoints[(j + 1) % iPointCount];
			int bCapStart = (!pContour->bClosed && (j == 0));
			int bCapEnd = (!pContour->bClosed && (j == (iSegmentCount - 1)));

			if ( __xgeShapeExStrokeSegmentContainsPoint(pShape, tA, tB, bCapStart, bCapEnd, fHalfWidth, fTolerance, tPoint) ) {
				xrtFree(pPoints);
				return 1;
			}
		}
		if ( iSegmentCount > 1 ) {
			int iJoinStart = pContour->bClosed ? 0 : 1;
			int iJoinEnd = pContour->bClosed ? iPointCount : (iPointCount - 1);

			for ( j = iJoinStart; j < iJoinEnd; j++ ) {
				int iPrev = (j + iPointCount - 1) % iPointCount;
				int iNext = (j + 1) % iPointCount;

				if ( __xgeShapeExStrokeJoinContainsPoint(pShape, pPoints[iPrev], pPoints[j], pPoints[iNext], fHalfWidth, fTolerance, tPoint) ) {
					xrtFree(pPoints);
					return 1;
				}
			}
		}
		xrtFree(pPoints);
	}
	return 0;
}

static int __xgeShapeExFlatStrokeDashedContainsPoint(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, float fStrokeWidth, const float* pDashPattern, int iDashCount, float fDashOffset, float fTolerance, xge_vec2_t tPoint)
{
	float fDashTotal;
	float fDashMin;
	int i;

	if ( (pFlat == NULL) || (pDashPattern == NULL) || (iDashCount <= 0) ) {
		return __xgeShapeExFlatStrokeSolidContainsPoint(pShape, pFlat, fStrokeWidth, fTolerance, tPoint);
	}
	fDashTotal = __xgeShapeExDashTotal(pDashPattern, iDashCount);
	if ( fDashTotal <= XGE_SHAPE_EX_EPSILON ) {
		return __xgeShapeExFlatStrokeSolidContainsPoint(pShape, pFlat, fStrokeWidth, fTolerance, tPoint);
	}
	fDashMin = __xgeShapeExDashMinPositive(pDashPattern, iDashCount);
	if ( fDashMin <= XGE_SHAPE_EX_EPSILON ) {
		return __xgeShapeExFlatStrokeSolidContainsPoint(pShape, pFlat, fStrokeWidth, fTolerance, tPoint);
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
		__xgeShapeExDashStateInit(pDashPattern, iDashCount, fDashOffset, &iDashIndex, &bDashOn, &fDashRemaining);
		if ( fContourLength <= XGE_SHAPE_EX_EPSILON ) {
			if ( bDashOn && (fDashRemaining > XGE_SHAPE_EX_EPSILON) &&
			     __xgeShapeExStrokePointInZeroCap(pShape, pFlat->pPoints[pContour->iStart], fStrokeWidth * 0.5f, fTolerance, tPoint) ) {
				return 1;
			}
			continue;
		}
		fPos = 0.0f;
		fDashEpsilon = fmaxf(XGE_SHAPE_EX_EPSILON, fContourLength * 0.000001f);
		fGuardEstimate = (fContourLength / fDashMin) + (float)__xgeShapeExDashLogicalCount(iDashCount) + 16.0f;
		if ( fGuardEstimate > 2000000.0f ) {
			return 0;
		}
		iGuard = 0;
		iGuardLimit = (int)ceilf(fGuardEstimate);
		while ( fPos < fContourLength - fDashEpsilon ) {
			float fStep = fDashRemaining;
			float fNext;

			if ( ++iGuard > iGuardLimit ) {
				return 0;
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
					return 0;
				}
				if ( __xgeShapeExFlatStrokeSolidContainsPoint(pShape, &tDashFlat, fStrokeWidth, fTolerance, tPoint) ) {
					__xgeShapeExFlatFree(&tDashFlat);
					return 1;
				}
				__xgeShapeExFlatFree(&tDashFlat);
			}
			fDashRemaining -= (fNext - fPos);
			fPos = fNext;
			if ( fDashRemaining <= fDashEpsilon ) {
				__xgeShapeExDashStateNext(pDashPattern, iDashCount, &iDashIndex, &bDashOn, &fDashRemaining);
			}
		}
	}
	return 0;
}

static int __xgeShapeExDrawStrokeSolid(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, float fWidth, uint32_t iColor, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, const xge_shape_ex_matrix_t* pGeometryMatrix, float fOpacity, int bScreenSpace)
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
				iRet = __xgeShapeExDrawStrokeSegmentJoinContour(pShape, pFlat->pPoints + pContour->iStart, pContour->iCount, pContour->bClosed, fWidth, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
			} else {
				iRet = __xgeShapeExDrawStrokeContourMesh(pShape, pFlat->pPoints + pContour->iStart, pContour->iCount, pContour->bClosed, fWidth, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
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

			iRet = __xgeShapeExDrawStrokeSegment(pShape, tA, tB, fWidth, iColor, bCapStart, bCapEnd, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
	}
	return XGE_OK;
}

static int __xgeShapeExDrawStrokeDashed(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, float fWidth, uint32_t iColor, const float* pDashPattern, int iDashCount, float fDashOffset, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, const xge_shape_ex_matrix_t* pGeometryMatrix, float fOpacity, int bScreenSpace)
{
	float fDashTotal;
	float fDashMin;
	int i;

	if ( (pFlat == NULL) || (pDashPattern == NULL) || (iDashCount <= 0) ) {
		return __xgeShapeExDrawStrokeSolid(pShape, pFlat, fWidth, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
	}
	fDashTotal = __xgeShapeExDashTotal(pDashPattern, iDashCount);
	if ( fDashTotal <= XGE_SHAPE_EX_EPSILON ) {
		return __xgeShapeExDrawStrokeSolid(pShape, pFlat, fWidth, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
	}
	fDashMin = __xgeShapeExDashMinPositive(pDashPattern, iDashCount);
	if ( fDashMin <= XGE_SHAPE_EX_EPSILON ) {
		return __xgeShapeExDrawStrokeSolid(pShape, pFlat, fWidth, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
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
				int iRet = __xgeShapeExDrawStrokeZeroLengthCap(pShape, pFlat->pPoints[pContour->iStart], fWidth, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
			}
			continue;
		}
		__xgeShapeExDashStateInit(pDashPattern, iDashCount, fDashOffset, &iDashIndex, &bDashOn, &fDashRemaining);
		fPos = 0.0f;
		fDashEpsilon = fmaxf(XGE_SHAPE_EX_EPSILON, fContourLength * 0.000001f);
		fGuardEstimate = (fContourLength / fDashMin) + (float)__xgeShapeExDashLogicalCount(iDashCount) + 16.0f;
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
				iRet = __xgeShapeExDrawStrokeSolid(pShape, &tDashFlat, fWidth, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
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

static int __xgeShapeExDrawStroke(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, float fWidth, uint32_t iColor, const float* pDashPattern, int iDashCount, float fDashOffset, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix, const xge_shape_ex_matrix_t* pGeometryMatrix, float fOpacity, int bScreenSpace)
{
	if ( (pFlat == NULL) || (fWidth <= 0.0f) || !__xgeShapeExStrokeVisible(pShape, iColor) ) {
		return XGE_OK;
	}
	if ( __xgeShapeExDashTotal(pDashPattern, iDashCount) > XGE_SHAPE_EX_EPSILON ) {
		return __xgeShapeExDrawStrokeDashed(pShape, pFlat, fWidth, iColor, pDashPattern, iDashCount, fDashOffset, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
	}
	return __xgeShapeExDrawStrokeSolid(pShape, pFlat, fWidth, iColor, tBounds, tMatrix, pGeometryMatrix, fOpacity, bScreenSpace);
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

#define XGE_SHAPE_EX_STENCIL_FINAL_BIT 0x80u
#define XGE_SHAPE_EX_STENCIL_DEPTH_MASK 0x7Fu
#ifndef GL_NOTEQUAL
#define GL_NOTEQUAL 0x0205
#endif
#ifndef GL_INCR
#define GL_INCR 0x1E02
#endif
#ifndef GL_DECR
#define GL_DECR 0x1E03
#endif
#ifndef GL_ALWAYS
#define GL_ALWAYS 0x0207
#endif
#ifndef GL_ZERO
#define GL_ZERO 0
#endif

#define XGE_SHAPE_EX_STENCIL_FINAL_MARK 0
#define XGE_SHAPE_EX_STENCIL_FINAL_CLEAR 1

static int __xgeShapeExDrawStencilViewportPass(void)
{
	xge_rect_t tViewport;

	tViewport = xgeViewportGet();
	if ( (tViewport.fW <= 0.0f) || (tViewport.fH <= 0.0f) ) {
		return XGE_OK;
	}
	xgeShapeRectFillPx(tViewport, XGE_COLOR_RGBA(255, 255, 255, 255));
	return __xgeShapeAutoBatchFlush();
}

static int __xgeShapeExMarkStencilFinalDepth(int iDepthValue)
{
	if ( (iDepthValue <= 0) || (iDepthValue > (int)XGE_SHAPE_EX_STENCIL_DEPTH_MASK) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	glStencilMask(XGE_SHAPE_EX_STENCIL_FINAL_BIT);
	glStencilFunc(GL_EQUAL, (GLint)(XGE_SHAPE_EX_STENCIL_FINAL_BIT | (uint32_t)iDepthValue), XGE_SHAPE_EX_STENCIL_DEPTH_MASK);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	return __xgeShapeExDrawStencilViewportPass();
}

static int __xgeShapeExClearStencilFinalDepth(int iDepthValue)
{
	if ( (iDepthValue <= 0) || (iDepthValue > (int)XGE_SHAPE_EX_STENCIL_DEPTH_MASK) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	glStencilMask(XGE_SHAPE_EX_STENCIL_FINAL_BIT);
	glStencilFunc(GL_EQUAL, (GLint)iDepthValue, XGE_SHAPE_EX_STENCIL_DEPTH_MASK);
	glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
	return __xgeShapeExDrawStencilViewportPass();
}

static int __xgeShapeExApplyStencilFinalDepth(int iDepthValue, int iFinalOp)
{
	if ( iFinalOp == XGE_SHAPE_EX_STENCIL_FINAL_CLEAR ) {
		return __xgeShapeExClearStencilFinalDepth(iDepthValue);
	}
	return __xgeShapeExMarkStencilFinalDepth(iDepthValue);
}

static int __xgeShapeExMarkStencilFinalViewport(void)
{
	glStencilMask(XGE_SHAPE_EX_STENCIL_FINAL_BIT);
	glStencilFunc(GL_ALWAYS, (GLint)XGE_SHAPE_EX_STENCIL_FINAL_BIT, XGE_SHAPE_EX_STENCIL_FINAL_BIT);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	return __xgeShapeExDrawStencilViewportPass();
}

static int __xgeShapeExClearStencilGateDepth(int iDepthValue)
{
	if ( (iDepthValue <= 0) || (iDepthValue > (int)XGE_SHAPE_EX_STENCIL_DEPTH_MASK) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	glStencilMask(XGE_SHAPE_EX_STENCIL_DEPTH_MASK);
	glStencilFunc(GL_EQUAL, (GLint)iDepthValue, XGE_SHAPE_EX_STENCIL_DEPTH_MASK);
	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
	return __xgeShapeExDrawStencilViewportPass();
}

static int __xgeShapeExDrawStencilClipShapeGeometry(xge_shape_ex pClipShape, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tParent)
{
	xge_shape_ex_flat_path_t tFlat;
	xge_shape_ex_flat_path_t tTrimmed;
	const xge_shape_ex_flat_path_t* pDrawFlat;
	xge_shape_ex_matrix_t tMatrix;
	struct xge_shape_ex_t tStencilShape;
	uint32_t iFillColor;
	uint32_t iStrokeColor;
	xge_rect_t tOldClip;
	xge_rect_t tStrokeBounds;
	float fStrokeScale;
	float fStrokeWidth;
	float fDashOffset;
	float* pDashPattern;
	int bOldClip;
	int bClipApplied;
	int iRet;

	memset(&tFlat, 0, sizeof(tFlat));
	memset(&tTrimmed, 0, sizeof(tTrimmed));
	memset(&tOldClip, 0, sizeof(tOldClip));
	bOldClip = 0;
	bClipApplied = 0;
	if ( !__xgeShapeExValid(pClipShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !pClipShape->bVisible ) {
		return XGE_OK;
	}
	tMatrix = __xgeShapeExMatrixMul(tParent, pClipShape->tTransform);
	if ( pClipShape->bClipRect ) {
		xge_rect_t tClip = __xgeShapeExMatrixRectBounds(tMatrix, pClipShape->tClipRect);

		if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
			return XGE_OK;
		}
		iRet = __xgeShapeAutoBatchFlush();
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		tOldClip = xgeClipGet();
		bOldClip = (tOldClip.fW > 0.0f) && (tOldClip.fH > 0.0f);
		if ( bOldClip ) {
			tClip = __xgeShapeExRectIntersect(tOldClip, tClip);
		}
		if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
			return XGE_OK;
		}
		xgeClipSet(tClip);
		bClipApplied = 1;
	}
	iRet = __xgeShapeExFlatten(pClipShape, tMatrix, fTolerance, &tFlat);
	if ( iRet != XGE_OK ) {
		if ( bClipApplied ) {
			if ( bOldClip ) xgeClipSet(tOldClip);
			else xgeClipClear();
		}
		return iRet;
	}
	pDrawFlat = &tFlat;
	if ( pClipShape->bTrimPath ) {
		iRet = __xgeShapeExFlatTrim(&tFlat, pClipShape->fTrimBegin, pClipShape->fTrimEnd, pClipShape->bTrimSimultaneous, &tTrimmed);
		if ( iRet != XGE_OK ) {
			__xgeShapeExFlatFree(&tFlat);
			if ( bClipApplied ) {
				if ( bOldClip ) xgeClipSet(tOldClip);
				else xgeClipClear();
			}
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
				if ( bClipApplied ) {
					if ( bOldClip ) xgeClipSet(tOldClip);
					else xgeClipClear();
				}
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			for ( i = 0; i < pClipShape->iDashCount; i++ ) {
				pDashPattern[i] = pClipShape->pDashPattern[i] * fStrokeScale;
			}
		}
		if ( pClipShape->bStrokeFirst ) {
			iRet = __xgeShapeExDrawStroke(&tStencilShape, pDrawFlat, fStrokeWidth, iStrokeColor, pDashPattern, pClipShape->iDashCount, fDashOffset, tStrokeBounds, tMatrix, NULL, 1.0f, bScreenSpace);
			if ( iRet == XGE_OK ) {
				iRet = __xgeShapeExDrawFill(&tStencilShape, pDrawFlat, pClipShape->iFillRule, tMatrix, 1.0f, iFillColor, bScreenSpace);
			}
		} else {
			iRet = __xgeShapeExDrawFill(&tStencilShape, pDrawFlat, pClipShape->iFillRule, tMatrix, 1.0f, iFillColor, bScreenSpace);
			if ( iRet == XGE_OK ) {
				iRet = __xgeShapeExDrawStroke(&tStencilShape, pDrawFlat, fStrokeWidth, iStrokeColor, pDashPattern, pClipShape->iDashCount, fDashOffset, tStrokeBounds, tMatrix, NULL, 1.0f, bScreenSpace);
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

static int __xgeShapeExStencilClipHasIncludeChildren(xge_shape_ex pClipShape)
{
	int i;

	if ( !__xgeShapeExValid(pClipShape) ) {
		return 0;
	}
	for ( i = 0; i < pClipShape->iClipShapeCount; i++ ) {
		int iMode = pClipShape->pClipShapeModes != NULL ? pClipShape->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

		if ( iMode != XGE_SHAPE_EX_CLIP_SUBTRACT ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeShapeExProtectStencilClipShape(xge_shape_ex pClipShape, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tParent, int iParentDepth);

static int __xgeShapeExUnprotectStencilClipShape(xge_shape_ex pClipShape, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tParent, int iProtectedDepth)
{
	xge_shape_ex_matrix_t tChildParent;
	int iRet;
	int i;

	if ( !__xgeShapeExValid(pClipShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !pClipShape->bVisible ) {
		return XGE_OK;
	}
	if ( (iProtectedDepth <= 0) || (iProtectedDepth > (int)XGE_SHAPE_EX_STENCIL_DEPTH_MASK) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	if ( __xgeShapeExStencilClipHasIncludeChildren(pClipShape) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	glStencilMask(XGE_SHAPE_EX_STENCIL_DEPTH_MASK);
	glStencilFunc(GL_EQUAL, (GLint)iProtectedDepth, XGE_SHAPE_EX_STENCIL_DEPTH_MASK);
	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
	iRet = __xgeShapeExDrawStencilClipShapeGeometry(pClipShape, fTolerance, bScreenSpace, tParent);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeShapeAutoBatchFlush();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tChildParent = __xgeShapeExMatrixMul(tParent, pClipShape->tTransform);
	for ( i = 0; i < pClipShape->iClipShapeCount; i++ ) {
		int iMode = pClipShape->pClipShapeModes != NULL ? pClipShape->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

		if ( iMode != XGE_SHAPE_EX_CLIP_SUBTRACT ) {
			continue;
		}
		iRet = __xgeShapeExProtectStencilClipShape(pClipShape->pClipShapes[i], fTolerance, bScreenSpace, tChildParent, iProtectedDepth - 1);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeShapeExProtectStencilClipShape(xge_shape_ex pClipShape, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tParent, int iParentDepth)
{
	xge_shape_ex_matrix_t tChildParent;
	int iProtectedDepth;
	int iRet;
	int i;

	if ( !__xgeShapeExValid(pClipShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !pClipShape->bVisible ) {
		return XGE_OK;
	}
	if ( (iParentDepth < 0) || (iParentDepth >= (int)XGE_SHAPE_EX_STENCIL_DEPTH_MASK) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	if ( __xgeShapeExStencilClipHasIncludeChildren(pClipShape) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	iProtectedDepth = iParentDepth + 1;
	glStencilMask(XGE_SHAPE_EX_STENCIL_DEPTH_MASK);
	glStencilFunc(GL_EQUAL, (GLint)iParentDepth, XGE_SHAPE_EX_STENCIL_DEPTH_MASK);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	iRet = __xgeShapeExDrawStencilClipShapeGeometry(pClipShape, fTolerance, bScreenSpace, tParent);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeShapeAutoBatchFlush();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tChildParent = __xgeShapeExMatrixMul(tParent, pClipShape->tTransform);
	for ( i = 0; i < pClipShape->iClipShapeCount; i++ ) {
		int iMode = pClipShape->pClipShapeModes != NULL ? pClipShape->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

		if ( iMode != XGE_SHAPE_EX_CLIP_SUBTRACT ) {
			continue;
		}
		iRet = __xgeShapeExUnprotectStencilClipShape(pClipShape->pClipShapes[i], fTolerance, bScreenSpace, tChildParent, iProtectedDepth);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeShapeExDrawStencilClipShape(xge_shape_ex pClipShape, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tParent, int iParentDepth, int iFinalOp)
{
	xge_shape_ex_matrix_t tChildParent;
	int iGateDepth;
	int iRet;
	int i;
	int iIncludeCount;
	int iSubtractCount;

	if ( !__xgeShapeExValid(pClipShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !pClipShape->bVisible ) {
		return XGE_OK;
	}
	if ( (iParentDepth < 0) || (iParentDepth >= (int)XGE_SHAPE_EX_STENCIL_DEPTH_MASK) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	iGateDepth = iParentDepth + 1;
	glStencilMask(XGE_SHAPE_EX_STENCIL_DEPTH_MASK);
	glStencilFunc(GL_EQUAL, (GLint)iParentDepth, XGE_SHAPE_EX_STENCIL_DEPTH_MASK);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	iRet = __xgeShapeExDrawStencilClipShapeGeometry(pClipShape, fTolerance, bScreenSpace, tParent);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeShapeAutoBatchFlush();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iIncludeCount = 0;
	iSubtractCount = 0;
	for ( i = 0; i < pClipShape->iClipShapeCount; i++ ) {
		int iMode = pClipShape->pClipShapeModes != NULL ? pClipShape->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

		if ( iMode == XGE_SHAPE_EX_CLIP_SUBTRACT ) {
			iSubtractCount++;
		} else {
			iIncludeCount++;
		}
	}
	tChildParent = __xgeShapeExMatrixMul(tParent, pClipShape->tTransform);
	if ( (iFinalOp == XGE_SHAPE_EX_STENCIL_FINAL_CLEAR) && (iSubtractCount > 0) ) {
		if ( iGateDepth >= (int)XGE_SHAPE_EX_STENCIL_DEPTH_MASK ) {
			return XGE_ERROR_UNSUPPORTED;
		}
		for ( i = 0; i < pClipShape->iClipShapeCount; i++ ) {
			int iMode = pClipShape->pClipShapeModes != NULL ? pClipShape->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

			if ( iMode != XGE_SHAPE_EX_CLIP_SUBTRACT ) {
				continue;
			}
			iRet = __xgeShapeExProtectStencilClipShape(pClipShape->pClipShapes[i], fTolerance, bScreenSpace, tChildParent, iGateDepth);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
		if ( iIncludeCount <= 0 ) {
			iRet = __xgeShapeExApplyStencilFinalDepth(iGateDepth, iFinalOp);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
		for ( i = 0; i < pClipShape->iClipShapeCount; i++ ) {
			int iMode = pClipShape->pClipShapeModes != NULL ? pClipShape->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

			if ( iMode == XGE_SHAPE_EX_CLIP_SUBTRACT ) {
				continue;
			}
			iRet = __xgeShapeExDrawStencilClipShape(pClipShape->pClipShapes[i], fTolerance, bScreenSpace, tChildParent, iGateDepth, iFinalOp);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
		iRet = __xgeShapeExClearStencilGateDepth(iGateDepth + 1);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		return __xgeShapeExClearStencilGateDepth(iGateDepth);
	}
	if ( iIncludeCount <= 0 ) {
		iRet = __xgeShapeExApplyStencilFinalDepth(iGateDepth, iFinalOp);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	for ( i = 0; i < pClipShape->iClipShapeCount; i++ ) {
		int iMode = pClipShape->pClipShapeModes != NULL ? pClipShape->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

		if ( iMode == XGE_SHAPE_EX_CLIP_SUBTRACT ) {
			continue;
		}
		iRet = __xgeShapeExDrawStencilClipShape(pClipShape->pClipShapes[i], fTolerance, bScreenSpace, tChildParent, iGateDepth, iFinalOp);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	if ( iFinalOp == XGE_SHAPE_EX_STENCIL_FINAL_MARK ) {
		for ( i = 0; i < pClipShape->iClipShapeCount; i++ ) {
			int iMode = pClipShape->pClipShapeModes != NULL ? pClipShape->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

			if ( iMode != XGE_SHAPE_EX_CLIP_SUBTRACT ) {
				continue;
			}
			iRet = __xgeShapeExDrawStencilClipShape(pClipShape->pClipShapes[i], fTolerance, bScreenSpace, tChildParent, iGateDepth, XGE_SHAPE_EX_STENCIL_FINAL_CLEAR);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
	}
	return __xgeShapeExClearStencilGateDepth(iGateDepth);
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

static int __xgeShapeExBeginStencilClipList(const xge_shape_ex* pClipShapes, const int* pClipShapeModes, int iClipShapeCount, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tMatrix)
{
	int i;
	int iRet;
	int iIncludeCount;

	if ( (pClipShapes == NULL) || (iClipShapeCount <= 0) ) {
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
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	iIncludeCount = 0;
	for ( i = 0; i < iClipShapeCount; i++ ) {
		int iMode = pClipShapeModes != NULL ? pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

		if ( iMode != XGE_SHAPE_EX_CLIP_SUBTRACT ) {
			iIncludeCount++;
		}
	}
	if ( iIncludeCount <= 0 ) {
		iRet = __xgeShapeExMarkStencilFinalViewport();
		if ( iRet != XGE_OK ) {
			return __xgeShapeExEndStencilClip(1, iRet);
		}
	}
	for ( i = 0; i < iClipShapeCount; i++ ) {
		int iMode = pClipShapeModes != NULL ? pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

		if ( iMode == XGE_SHAPE_EX_CLIP_SUBTRACT ) {
			continue;
		}
		iRet = __xgeShapeExDrawStencilClipShape(pClipShapes[i], fTolerance, bScreenSpace, tMatrix, 0, XGE_SHAPE_EX_STENCIL_FINAL_MARK);
		if ( iRet != XGE_OK ) {
			return __xgeShapeExEndStencilClip(1, iRet);
		}
	}
	for ( i = 0; i < iClipShapeCount; i++ ) {
		int iMode = pClipShapeModes != NULL ? pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

		if ( iMode != XGE_SHAPE_EX_CLIP_SUBTRACT ) {
			continue;
		}
		iRet = __xgeShapeExDrawStencilClipShape(pClipShapes[i], fTolerance, bScreenSpace, tMatrix, 0, XGE_SHAPE_EX_STENCIL_FINAL_CLEAR);
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
	glStencilFunc(GL_NOTEQUAL, 0, XGE_SHAPE_EX_STENCIL_FINAL_BIT);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	return XGE_OK;
}

static int __xgeShapeExBeginStencilClip(xge_shape_ex pShape, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tMatrix)
{
	if ( pShape == NULL ) {
		return XGE_OK;
	}
	return __xgeShapeExBeginStencilClipList(pShape->pClipShapes, pShape->pClipShapeModes, pShape->iClipShapeCount, fTolerance, bScreenSpace, tMatrix);
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
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	iRet = __xgeShapeExDrawStencilClipShape(pClipShape, fTolerance, bScreenSpace, tParent, 0, XGE_SHAPE_EX_STENCIL_FINAL_MARK);
	if ( iRet != XGE_OK ) {
		return __xgeShapeExEndStencilClip(1, iRet);
	}
	iRet = __xgeShapeAutoBatchFlush();
	if ( iRet != XGE_OK ) {
		return __xgeShapeExEndStencilClip(1, iRet);
	}
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glStencilMask(0x00u);
	glStencilFunc(GL_NOTEQUAL, 0, XGE_SHAPE_EX_STENCIL_FINAL_BIT);
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

static int __xgeShapeExBlendBegin(int bBlendSet, int iBlend, int* pOldBlend, int* pBlendApplied)
{
	int iOldBlend;
	int iRet;

	if ( (pOldBlend == NULL) || (pBlendApplied == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pOldBlend = XGE_BLEND_ALPHA;
	*pBlendApplied = 0;
	if ( !bBlendSet ) {
		return XGE_OK;
	}
	if ( !__xgeShapeExBlendValid(iBlend) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iOldBlend = xgeBlendGet();
	*pOldBlend = iOldBlend;
	if ( iOldBlend == iBlend ) {
		return XGE_OK;
	}
	iRet = __xgeShapeAutoBatchFlush();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xgeBlendSet(iBlend);
	*pBlendApplied = 1;
	return XGE_OK;
}

static int __xgeShapeExBlendEnd(int iOldBlend, int bBlendApplied, int iRet)
{
	if ( bBlendApplied ) {
		int iFlushRet = __xgeShapeAutoBatchFlush();

		if ( iRet == XGE_OK ) {
			iRet = iFlushRet;
		}
		xgeBlendSet(iOldBlend);
	}
	return iRet;
}

static int __xgeShapeExDrawDispatch(xge_shape_ex pShape, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tParent, float fParentOpacity, int bSuppressOwnBlend);

static int __xgeShapeExDrawInternal(xge_shape_ex pShape, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tParent, float fParentOpacity, int bSuppressOwnBlend)
{
	xge_shape_ex_flat_path_t tFlat;
	xge_shape_ex_flat_path_t tTrimmed;
	xge_shape_ex_flat_path_t tStrokeFlat;
	xge_shape_ex_flat_path_t tStrokeTrimmed;
	const xge_shape_ex_flat_path_t* pDrawFlat;
	const xge_shape_ex_flat_path_t* pStrokeFlat;
	xge_shape_ex_matrix_t tMatrix;
	xge_shape_ex_matrix_t tStrokePaintMatrix;
	xge_shape_ex_matrix_t tIdentity;
	const xge_shape_ex_matrix_t* pStrokeGeometryMatrix;
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
	int iOldBlend;
	int bBlendApplied;
	int iRet;

	memset(&tFlat, 0, sizeof(tFlat));
	memset(&tTrimmed, 0, sizeof(tTrimmed));
	memset(&tStrokeFlat, 0, sizeof(tStrokeFlat));
	memset(&tStrokeTrimmed, 0, sizeof(tStrokeTrimmed));
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
	iOldBlend = XGE_BLEND_ALPHA;
	bBlendApplied = 0;
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
	fStrokeScale = pShape->bStrokeNonScaling ? 1.0f : __xgeShapeExMatrixStrokeScale(tMatrix);
	fStrokeWidth = pShape->fStrokeWidth * fStrokeScale;
	fDashOffset = pShape->fDashOffset * fStrokeScale;
	pDashPattern = pShape->pDashPattern;
	pStrokeFlat = pDrawFlat;
	tStrokeBounds = __xgeShapeExFlatBounds(pStrokeFlat);
	tStrokePaintMatrix = tMatrix;
	pStrokeGeometryMatrix = NULL;
	if ( !pShape->bStrokeNonScaling && __xgeShapeExStrokeVisible(pShape, iStrokeColor) ) {
		float fLocalTolerance;

		tIdentity = __xgeShapeExMatrixIdentity();
		fLocalTolerance = fTolerance;
		if ( fStrokeScale > XGE_SHAPE_EX_EPSILON ) {
			fLocalTolerance = fTolerance / fStrokeScale;
			if ( fLocalTolerance <= XGE_SHAPE_EX_EPSILON ) {
				fLocalTolerance = fTolerance;
			}
		}
		iRet = __xgeShapeExFlatten(pShape, tIdentity, fLocalTolerance, &tStrokeFlat);
		if ( iRet != XGE_OK ) {
			if ( pDashPattern != pShape->pDashPattern ) {
				xrtFree(pDashPattern);
			}
			__xgeShapeExFlatFree(&tTrimmed);
			__xgeShapeExFlatFree(&tFlat);
			return __xgeShapeExFinishShapeClips(bClipApplied, bOldClip, tOldClip, bStencilApplied, iRet);
		}
		pStrokeFlat = &tStrokeFlat;
		if ( pShape->bTrimPath ) {
			iRet = __xgeShapeExFlatTrim(&tStrokeFlat, pShape->fTrimBegin, pShape->fTrimEnd, pShape->bTrimSimultaneous, &tStrokeTrimmed);
			if ( iRet != XGE_OK ) {
				if ( pDashPattern != pShape->pDashPattern ) {
					xrtFree(pDashPattern);
				}
				__xgeShapeExFlatFree(&tStrokeFlat);
				__xgeShapeExFlatFree(&tTrimmed);
				__xgeShapeExFlatFree(&tFlat);
				return __xgeShapeExFinishShapeClips(bClipApplied, bOldClip, tOldClip, bStencilApplied, iRet);
			}
			pStrokeFlat = &tStrokeTrimmed;
		}
		tStrokeBounds = __xgeShapeExFlatBounds(pStrokeFlat);
		tStrokePaintMatrix = tIdentity;
		pStrokeGeometryMatrix = &tMatrix;
		fStrokeWidth = pShape->fStrokeWidth;
		fDashOffset = pShape->fDashOffset;
		fStrokeScale = 1.0f;
	}
	if ( (pShape->pDashPattern != NULL) && (pShape->iDashCount > 0) && (fabsf(fStrokeScale - 1.0f) > XGE_SHAPE_EX_EPSILON) ) {
		int i;

		pDashPattern = (float*)xrtMalloc((size_t)pShape->iDashCount * sizeof(*pDashPattern));
		if ( pDashPattern == NULL ) {
			__xgeShapeExFlatFree(&tStrokeTrimmed);
			__xgeShapeExFlatFree(&tStrokeFlat);
			__xgeShapeExFlatFree(&tTrimmed);
			__xgeShapeExFlatFree(&tFlat);
			return __xgeShapeExFinishShapeClips(bClipApplied, bOldClip, tOldClip, bStencilApplied, XGE_ERROR_OUT_OF_MEMORY);
		}
		for ( i = 0; i < pShape->iDashCount; i++ ) {
			pDashPattern[i] = pShape->pDashPattern[i] * fStrokeScale;
		}
	}
	if ( (pShape->iStrokeGradientUnits == XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX) &&
	     ((pShape->iStrokeType == XGE_SHAPE_EX_FILL_LINEAR_GRADIENT) || (pShape->iStrokeType == XGE_SHAPE_EX_FILL_RADIAL_GRADIENT)) ) {
		float fStrokeHalf = fStrokeWidth * 0.5f;

		tStrokeBounds.fX -= fStrokeHalf;
		tStrokeBounds.fY -= fStrokeHalf;
		tStrokeBounds.fW += fStrokeWidth;
		tStrokeBounds.fH += fStrokeWidth;
	}
	iRet = __xgeShapeExBlendBegin(
		bSuppressOwnBlend ? 0 : pShape->bBlendSet,
		pShape->iBlend,
		&iOldBlend,
		&bBlendApplied
	);
	if ( iRet != XGE_OK ) {
		if ( pDashPattern != pShape->pDashPattern ) {
			xrtFree(pDashPattern);
		}
		__xgeShapeExFlatFree(&tTrimmed);
		__xgeShapeExFlatFree(&tFlat);
		__xgeShapeExFlatFree(&tStrokeTrimmed);
		__xgeShapeExFlatFree(&tStrokeFlat);
		return __xgeShapeExFinishShapeClips(bClipApplied, bOldClip, tOldClip, bStencilApplied, iRet);
	}
	if ( pShape->bStrokeFirst ) {
		iRet = __xgeShapeExDrawStroke(pShape, pStrokeFlat, fStrokeWidth, iStrokeColor, pDashPattern, pShape->iDashCount, fDashOffset, tStrokeBounds, tStrokePaintMatrix, pStrokeGeometryMatrix, fOpacity, bScreenSpace);
		if ( iRet == XGE_OK ) {
			iRet = __xgeShapeExDrawFill(pShape, pDrawFlat, pShape->iFillRule, tMatrix, fOpacity, iFillColor, bScreenSpace);
		}
	} else {
		iRet = __xgeShapeExDrawFill(pShape, pDrawFlat, pShape->iFillRule, tMatrix, fOpacity, iFillColor, bScreenSpace);
		if ( iRet == XGE_OK ) {
			iRet = __xgeShapeExDrawStroke(pShape, pStrokeFlat, fStrokeWidth, iStrokeColor, pDashPattern, pShape->iDashCount, fDashOffset, tStrokeBounds, tStrokePaintMatrix, pStrokeGeometryMatrix, fOpacity, bScreenSpace);
		}
	}
	iRet = __xgeShapeExBlendEnd(iOldBlend, bBlendApplied, iRet);
	if ( pDashPattern != pShape->pDashPattern ) {
		xrtFree(pDashPattern);
	}
	__xgeShapeExFlatFree(&tStrokeTrimmed);
	__xgeShapeExFlatFree(&tStrokeFlat);
	__xgeShapeExFlatFree(&tTrimmed);
	__xgeShapeExFlatFree(&tFlat);
	return __xgeShapeExFinishShapeClips(bClipApplied, bOldClip, tOldClip, bStencilApplied, iRet);
}

static int __xgeShapeExSceneDrawInternal(xge_shape_ex_scene pScene, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tParent, float fParentOpacity, int bSuppressOwnBlend)
{
	xge_shape_ex_matrix_t tMatrix;
	float fOpacity;
	xge_rect_t tOldClip;
	int bOldClip;
	int bClipApplied;
	int bStencilApplied;
	int iOldBlend;
	int bBlendApplied;
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
	bClipApplied = 0;
	bStencilApplied = 0;
	bOldClip = 0;
	memset(&tOldClip, 0, sizeof(tOldClip));
	if ( pScene->bClipRect ) {
		xge_rect_t tClip = __xgeShapeExMatrixRectBounds(tMatrix, pScene->tClipRect);

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
	if ( pScene->iClipShapeCount > 0 ) {
		iRet = __xgeShapeExBeginStencilClipList(pScene->pClipShapes, pScene->pClipShapeModes, pScene->iClipShapeCount, fTolerance, bScreenSpace, tMatrix);
		if ( iRet != XGE_OK ) {
			return __xgeShapeExFinishShapeClips(bClipApplied, bOldClip, tOldClip, 0, iRet);
		}
		bStencilApplied = 1;
	}
	iRet = __xgeShapeExBlendBegin(
		bSuppressOwnBlend ? 0 : pScene->bBlendSet,
		pScene->iBlend,
		&iOldBlend,
		&bBlendApplied
	);
	if ( iRet != XGE_OK ) {
		return __xgeShapeExFinishShapeClips(bClipApplied, bOldClip, tOldClip, bStencilApplied, iRet);
	}
	for ( i = 0; i < pScene->iShapeCount; i++ ) {
		iRet = __xgeShapeExDrawDispatch(pScene->pShapes[i], fTolerance, bScreenSpace, tMatrix, fOpacity, 0);
		if ( iRet != XGE_OK ) {
			break;
		}
	}
	iRet = __xgeShapeExBlendEnd(iOldBlend, bBlendApplied, iRet);
	return __xgeShapeExFinishShapeClips(bClipApplied, bOldClip, tOldClip, bStencilApplied, iRet);
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

static int __xgeShapeExSvgIsDigit(char c)
{
	return (c >= '0') && (c <= '9');
}

static int __xgeShapeExSvgScanNumberEnd(const char* pText, const char** ppEnd)
{
	const char* p;
	int iDigitCount;
	int iFracDigitCount;
	int iExpDigitCount;

	if ( (pText == NULL) || (ppEnd == NULL) ) {
		return 0;
	}
	p = pText;
	if ( (*p == '+') || (*p == '-') ) {
		p++;
	}
	iDigitCount = 0;
	while ( __xgeShapeExSvgIsDigit(*p) ) {
		iDigitCount++;
		p++;
	}
	iFracDigitCount = 0;
	if ( *p == '.' ) {
		p++;
		while ( __xgeShapeExSvgIsDigit(*p) ) {
			iFracDigitCount++;
			p++;
		}
	}
	if ( (iDigitCount + iFracDigitCount) <= 0 ) {
		return 0;
	}
	if ( (*p == 'e') || (*p == 'E') ) {
		p++;
		if ( (*p == '+') || (*p == '-') ) {
			p++;
		}
		iExpDigitCount = 0;
		while ( __xgeShapeExSvgIsDigit(*p) ) {
			iExpDigitCount++;
			p++;
		}
		if ( iExpDigitCount <= 0 ) {
			return 0;
		}
	}
	*ppEnd = p;
	return 1;
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
	const char* pNumberEnd;
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
	if ( !__xgeShapeExSvgScanNumberEnd(pText, &pNumberEnd) ) {
		return 0;
	}
	fValue = strtod(pText, &pEnd);
	if ( (pEnd == pText) || (pEnd != pNumberEnd) ) {
		return 0;
	}
	if ( !__xgeShapeExSvgDoubleToFloat(fValue, pValue) ) {
		return 0;
	}
	*ppText = pNumberEnd;
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

	if ( fabsf(fSweepRadians) <= XGE_SHAPE_EX_EPSILON ) {
		return XGE_OK;
	}
	tStart = __xgeShapeExEllipsePoint(fCX, fCY, fRX, fRY, fStartRadians);
	if ( bMoveToStart ) {
		iRet = xgeShapeExMoveTo(pShape, tStart.fX, tStart.fY);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	} else if ( !pShape->bHasCurrent ) {
		return XGE_ERROR_INVALID_ARGUMENT;
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
	pShape->bTrimSimultaneous = 0;
	pShape->fOpacity = 1.0f;
	pShape->bVisible = 1;
	pShape->iBlend = XGE_BLEND_ALPHA;
	pShape->bBlendSet = 0;
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

static int __xgeShapeExCloneInternal(xge_shape_ex pShape, xge_shape_ex* ppClone, xge_shape_ex* pCloneStack, int iCloneStackCount)
{
	xge_shape_ex pClone;
	int iRet;
	int i;

	if ( !__xgeShapeExValid(pShape) || (ppClone == NULL) || (pCloneStack == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppClone = NULL;
	if ( iCloneStackCount >= XGE_SHAPE_EX_CLIP_BOUNDS_MAX_DEPTH ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < iCloneStackCount; i++ ) {
		if ( pCloneStack[i] == pShape ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	pCloneStack[iCloneStackCount++] = pShape;
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
	pClone->fFillFR = pShape->fFillFR;
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
	pClone->fStrokeFR = pShape->fStrokeFR;
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
	pClone->iBlend = pShape->iBlend;
	pClone->bBlendSet = pShape->bBlendSet;
	pClone->bClipRect = pShape->bClipRect;
	pClone->tClipRect = pShape->tClipRect;
	pClone->tTransform = pShape->tTransform;
	if ( pShape->iClipShapeCount > 0 ) {
		iRet = __xgeShapeExReserveClipShapes(pClone, pShape->iClipShapeCount);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pClone);
			return iRet;
		}
		for ( i = 0; i < pShape->iClipShapeCount; i++ ) {
			xge_shape_ex pClipClone;

			pClipClone = NULL;
			iRet = __xgeShapeExCloneInternal(pShape->pClipShapes[i], &pClipClone, pCloneStack, iCloneStackCount);
			if ( iRet == XGE_OK ) {
				int iMode = pShape->pClipShapeModes != NULL ? pShape->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

				iRet = __xgeShapeExClipShapeAddRefEx(pClone, pClipClone, iMode);
				xgeShapeExDestroy(pClipClone);
			}
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

int xgeShapeExClone(xge_shape_ex pShape, xge_shape_ex* ppClone)
{
	xge_shape_ex arrCloneStack[XGE_SHAPE_EX_CLIP_BOUNDS_MAX_DEPTH];

	return __xgeShapeExCloneInternal(pShape, ppClone, arrCloneStack, 0);
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
	xrtFree(pShape->pClipShapeModes);
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
	xge_vec2_t arrPoints[2];
	xge_vec2_t tPrev;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || !pShape->bHasCurrent ||
	     !__xgeShapeExCoordsFinite(fCX, fCY) || !__xgeShapeExCoordsFinite(fX, fY) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tPrev = pShape->tCurrent;
	arrPoints[0].fX = fCX;
	arrPoints[0].fY = fCY;
	arrPoints[1].fX = fX;
	arrPoints[1].fY = fY;
	iRet = __xgeShapeExAppendCommand(pShape, XGE_SHAPE_EX_CMD_QUAD_TO, arrPoints, 2);
	if ( iRet == XGE_OK ) {
		pShape->tLastCubicControl = arrPoints[1];
		pShape->tLastQuadControl = arrPoints[0];
		pShape->tCurrent = arrPoints[1];
		if ( !pShape->bHasStart ) {
			pShape->tStart = tPrev;
			pShape->bHasStart = 1;
		}
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

	if ( !__xgeShapeExValid(pShape) || !pShape->bHasCurrent ||
	     !__xgeShapeExCoordsFinite(fRX, fRY) || !__xgeShapeExFloatFinite(fAxisDegrees) || !__xgeShapeExCoordsFinite(fX, fY) ||
	     ((bLargeArc != 0) && (bLargeArc != 1)) || ((bSweep != 0) && (bSweep != 1)) ) {
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
	if ( (pShape->iCommandCount > 0) &&
	     (pShape->pCommands[pShape->iCommandCount - 1] == XGE_SHAPE_EX_CMD_CLOSE) ) {
		return XGE_OK;
	}
	iRet = __xgeShapeExAppendCommand(pShape, XGE_SHAPE_EX_CMD_CLOSE, NULL, 0);
	if ( (iRet == XGE_OK) && pShape->bHasStart ) {
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

	if ( !__xgeShapeExValid(pShape) ||
	     !__xgeShapeExCoordsFinite(fX1, fY1) || !__xgeShapeExCoordsFinite(fX2, fY2) || !__xgeShapeExCoordsFinite(fX3, fY3) ) {
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

int xgeShapeExAppendLine(xge_shape_ex pShape, float fX1, float fY1, float fX2, float fY2)
{
	xge_shape_ex_path_snapshot_t tSnapshot = {0};
	int iRet;

	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExCoordsFinite(fX1, fY1) || !__xgeShapeExCoordsFinite(fX2, fY2) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeShapeExPathSnapshot(pShape, &tSnapshot);
	iRet = xgeShapeExMoveTo(pShape, fX1, fY1);
	if ( iRet != XGE_OK ) goto fail;
	iRet = xgeShapeExLineTo(pShape, fX2, fY2);
	if ( iRet != XGE_OK ) goto fail;
	return XGE_OK;
fail:
	__xgeShapeExPathRestore(pShape, &tSnapshot);
	return iRet;
}

static int __xgeShapeExAppendPointList(xge_shape_ex pShape, const xge_vec2_t* pPoints, int iPointCount, int bClose)
{
	xge_shape_ex_path_snapshot_t tSnapshot = {0};
	int i;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (pPoints == NULL) || (iPointCount <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < iPointCount; i++ ) {
		if ( !__xgeShapeExPointFinite(pPoints[i]) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	__xgeShapeExPathSnapshot(pShape, &tSnapshot);
	iRet = xgeShapeExMoveTo(pShape, pPoints[0].fX, pPoints[0].fY);
	if ( iRet != XGE_OK ) goto fail;
	for ( i = 1; i < iPointCount; i++ ) {
		iRet = xgeShapeExLineTo(pShape, pPoints[i].fX, pPoints[i].fY);
		if ( iRet != XGE_OK ) goto fail;
	}
	if ( bClose ) {
		iRet = xgeShapeExClose(pShape);
		if ( iRet != XGE_OK ) goto fail;
	}
	return XGE_OK;
fail:
	__xgeShapeExPathRestore(pShape, &tSnapshot);
	return iRet;
}

int xgeShapeExAppendPolyline(xge_shape_ex pShape, const xge_vec2_t* pPoints, int iPointCount)
{
	return __xgeShapeExAppendPointList(pShape, pPoints, iPointCount, 0);
}

int xgeShapeExAppendPolygon(xge_shape_ex pShape, const xge_vec2_t* pPoints, int iPointCount)
{
	return __xgeShapeExAppendPointList(pShape, pPoints, iPointCount, 1);
}

int xgeShapeExAppendRect(xge_shape_ex pShape, float fX, float fY, float fW, float fH, float fRX, float fRY, int bClockwise)
{
	xge_shape_ex_path_snapshot_t tSnapshot = {0};
	float fHalfW;
	float fHalfH;
	float fKX;
	float fKY;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExCoordsFinite(fX, fY) ||
	     !__xgeShapeExCoordsFinite(fW, fH) || !__xgeShapeExCoordsFinite(fRX, fRY) ) {
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

	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExCoordsFinite(fX, fY) ||
	     !__xgeShapeExCoordsFinite(fW, fH) || (fW < 0.0f) || (fH < 0.0f) ) {
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

	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExCoordsFinite(fCX, fCY) || !__xgeShapeExCoordsFinite(fRX, fRY) ) {
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

int xgeShapeExAppendEllipse(xge_shape_ex pShape, float fCX, float fCY, float fRX, float fRY, int bClockwise)
{
	return xgeShapeExAppendCircle(pShape, fCX, fCY, fRX, fRY, bClockwise);
}

int xgeShapeExAppendArc(xge_shape_ex pShape, float fCX, float fCY, float fRX, float fRY, float fStartRadians, float fEndRadians)
{
	xge_shape_ex_path_snapshot_t tSnapshot = {0};
	float fSweep;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExCoordsFinite(fCX, fCY) ||
	     !__xgeShapeExCoordsFinite(fRX, fRY) || !__xgeShapeExCoordsFinite(fStartRadians, fEndRadians) ||
	     (fRX < 0.0f) || (fRY < 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeShapeExPathSnapshot(pShape, &tSnapshot);
	fSweep = __xgeShapeExClampSweep(fEndRadians - fStartRadians);
	if ( fabsf(fSweep) <= XGE_SHAPE_EX_EPSILON ) {
		return XGE_OK;
	}
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

	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExCoordsFinite(fCX, fCY) ||
	     !__xgeShapeExCoordsFinite(fRX, fRY) || !__xgeShapeExCoordsFinite(fStartRadians, fEndRadians) ||
	     (fRX < 0.0f) || (fRY < 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeShapeExPathSnapshot(pShape, &tSnapshot);
	fSweep = __xgeShapeExClampSweep(fEndRadians - fStartRadians);
	if ( fabsf(fSweep) <= XGE_SHAPE_EX_EPSILON ) {
		return XGE_OK;
	}
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

	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExCoordsFinite(fCX, fCY) ||
	     !__xgeShapeExCoordsFinite(fRX, fRY) || !__xgeShapeExCoordsFinite(fStartRadians, fEndRadians) ||
	     (fRX < 0.0f) || (fRY < 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeShapeExPathSnapshot(pShape, &tSnapshot);
	fSweep = __xgeShapeExClampSweep(fEndRadians - fStartRadians);
	if ( fabsf(fSweep) <= XGE_SHAPE_EX_EPSILON ) {
		return XGE_OK;
	}
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
	int bValidateHasCurrent;
	int bValidateHasStart;
	int i;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (pCommands == NULL) || (iCommandCount <= 0) || (pPoints == NULL) || (iPointCount <= 0) ) {
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
	for ( i = 0; i < iPointCount; i++ ) {
		if ( !__xgeShapeExPointFinite(pPoints[i]) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	bValidateHasCurrent = pShape->bHasCurrent;
	bValidateHasStart = pShape->bHasStart;
	for ( i = 0; i < iCommandCount; i++ ) {
		switch ( pCommands[i] ) {
			case XGE_SHAPE_EX_CMD_CLOSE:
				if ( bValidateHasStart ) {
					bValidateHasCurrent = 1;
				}
				break;
			case XGE_SHAPE_EX_CMD_MOVE_TO:
				bValidateHasCurrent = 1;
				bValidateHasStart = 1;
				break;
			case XGE_SHAPE_EX_CMD_LINE_TO:
				if ( !bValidateHasCurrent ) {
					bValidateHasCurrent = 1;
					bValidateHasStart = 1;
				} else if ( !bValidateHasStart ) {
					bValidateHasStart = 1;
				}
				break;
			case XGE_SHAPE_EX_CMD_QUAD_TO:
				if ( !bValidateHasCurrent ) {
					return XGE_ERROR_INVALID_ARGUMENT;
				}
				if ( !bValidateHasStart ) {
					bValidateHasStart = 1;
				}
				break;
			case XGE_SHAPE_EX_CMD_CUBIC_TO:
				if ( !bValidateHasCurrent ) {
					bValidateHasCurrent = 1;
					break;
				}
				if ( !bValidateHasStart ) {
					bValidateHasStart = 1;
				}
				break;
			default:
				return XGE_ERROR_INVALID_ARGUMENT;
		}
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
			{
				xge_vec2_t tPrev = tCurrent;

				tCurrent = pPoints[iPointRead++];
				tLastCubicControl = tCurrent;
				tLastQuadControl = tCurrent;
				if ( !bHasCurrent ) {
					tStart = tCurrent;
					bHasStart = 1;
				}
				bHasCurrent = 1;
				if ( !bHasStart ) {
					tStart = tPrev;
					bHasStart = 1;
				}
				break;
			}
			case XGE_SHAPE_EX_CMD_QUAD_TO:
				if ( !bHasCurrent ) {
					return XGE_ERROR_INVALID_ARGUMENT;
				}
			{
				xge_vec2_t tPrev = tCurrent;

				tLastQuadControl = pPoints[iPointRead];
				tCurrent = pPoints[iPointRead + 1];
				tLastCubicControl = tCurrent;
				iPointRead += 2;
				bHasCurrent = 1;
				if ( !bHasStart ) {
					tStart = tPrev;
					bHasStart = 1;
				}
				break;
			}
			case XGE_SHAPE_EX_CMD_CUBIC_TO:
			{
				xge_vec2_t tPrev = tCurrent;

				if ( !bHasCurrent ) {
					tLastCubicControl = pPoints[iPointRead + 1];
					tLastQuadControl = pPoints[iPointRead + 2];
					tCurrent = pPoints[iPointRead + 2];
					iPointRead += 3;
					bHasCurrent = 1;
					break;
				}
				tLastCubicControl = pPoints[iPointRead + 1];
				tLastQuadControl = pPoints[iPointRead + 2];
				tCurrent = pPoints[iPointRead + 2];
				iPointRead += 3;
				bHasCurrent = 1;
				if ( !bHasStart ) {
					tStart = tPrev;
					bHasStart = 1;
				}
				break;
			}
			default:
				return XGE_ERROR_INVALID_ARGUMENT;
		}
		pShape->pCommands[pShape->iCommandCount + i] = pCommands[i];
	}
	if ( iPointCount > 0 ) {
		memcpy(pShape->pPoints + pShape->iPointCount, pPoints, (size_t)iPointCount * sizeof(*pPoints));
	}
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

typedef int (*xge_shape_ex_svg_path_event_proc)(void* pUser, uint8_t iCommand, xge_vec2_t tStart, xge_vec2_t tControl1, xge_vec2_t tControl2, xge_vec2_t tEnd);

static int __xgeShapeExSvgPathEvent(xge_shape_ex_svg_path_event_proc pEvent, void* pUser, uint8_t iCommand, xge_vec2_t tStart, xge_vec2_t tControl1, xge_vec2_t tControl2, xge_vec2_t tEnd)
{
	if ( pEvent == NULL ) {
		return XGE_OK;
	}
	return pEvent(pUser, iCommand, tStart, tControl1, tControl2, tEnd);
}

static int __xgeShapeExSvgPathArcEvent(xge_shape_ex pShape, int iCommandBefore, int iPointBefore, xge_vec2_t tStart, xge_shape_ex_svg_path_event_proc pEvent, void* pUser)
{
	xge_vec2_t tZero = {0.0f, 0.0f};
	xge_vec2_t tFirstC1 = {0.0f, 0.0f};
	xge_vec2_t tLastC2 = {0.0f, 0.0f};
	xge_vec2_t tEnd = {0.0f, 0.0f};
	int iPointRead;
	int bHasCubic;
	int i;

	if ( pEvent == NULL ) {
		return XGE_OK;
	}
	if ( !__xgeShapeExValid(pShape) || (iCommandBefore < 0) || (iPointBefore < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pShape->iCommandCount <= iCommandBefore ) {
		return XGE_OK;
	}
	iPointRead = iPointBefore;
	bHasCubic = 0;
	for ( i = iCommandBefore; i < pShape->iCommandCount; i++ ) {
		uint8_t iCommand = pShape->pCommands[i];
		int iPointCount = __xgeShapeExCommandPointCount(iCommand);

		if ( iPointCount < 0 ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( (iPointRead + iPointCount) > pShape->iPointCount ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( iCommand == XGE_SHAPE_EX_CMD_LINE_TO ) {
			return __xgeShapeExSvgPathEvent(pEvent, pUser, XGE_SHAPE_EX_CMD_LINE_TO, tStart, tZero, tZero, pShape->pPoints[iPointRead]);
		}
		if ( iCommand == XGE_SHAPE_EX_CMD_CUBIC_TO ) {
			if ( !bHasCubic ) {
				tFirstC1 = pShape->pPoints[iPointRead];
				bHasCubic = 1;
			}
			tLastC2 = pShape->pPoints[iPointRead + 1];
			tEnd = pShape->pPoints[iPointRead + 2];
		}
		iPointRead += iPointCount;
	}
	if ( bHasCubic ) {
		return __xgeShapeExSvgPathEvent(pEvent, pUser, XGE_SHAPE_EX_CMD_CUBIC_TO, tStart, tFirstC1, tLastC2, tEnd);
	}
	return XGE_OK;
}

static int __xgeShapeExAppendSvgPathNoRollback(xge_shape_ex pShape, const char* sPath, xge_shape_ex_svg_path_event_proc pEvent, void* pUser)
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
		if ( (__xgeShapeExSvgLower(cCommand) != 'm') && !pShape->bHasCurrent ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
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
						iRet = __xgeShapeExSvgPathEvent(pEvent, pUser, XGE_SHAPE_EX_CMD_MOVE_TO, pShape->tCurrent, (xge_vec2_t){0.0f, 0.0f}, (xge_vec2_t){0.0f, 0.0f}, pShape->tCurrent);
						if ( iRet != XGE_OK ) return iRet;
						bFirst = 0;
					} else {
						xge_vec2_t tStart = pShape->tCurrent;
						iRet = xgeShapeExLineTo(pShape, fX, fY);
						if ( iRet != XGE_OK ) return iRet;
						iRet = __xgeShapeExSvgPathEvent(pEvent, pUser, XGE_SHAPE_EX_CMD_LINE_TO, tStart, (xge_vec2_t){0.0f, 0.0f}, (xge_vec2_t){0.0f, 0.0f}, pShape->tCurrent);
						if ( iRet != XGE_OK ) return iRet;
					}
				}
				if ( bFirst ) {
					return XGE_ERROR_INVALID_ARGUMENT;
				}
				cPrevCommand = cCommand;
				cCommand = bRelative ? 'l' : 'L';
				break;
			}
			case 'l': {
				float fX;
				float fY;
				int bConsumed = 0;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					if ( !__xgeShapeExSvgReadNumber(&pText, &fX) || !__xgeShapeExSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( bRelative ) {
						fX += pShape->tCurrent.fX;
						fY += pShape->tCurrent.fY;
					}
					{
						xge_vec2_t tStart = pShape->tCurrent;
						iRet = xgeShapeExLineTo(pShape, fX, fY);
						if ( iRet != XGE_OK ) return iRet;
						iRet = __xgeShapeExSvgPathEvent(pEvent, pUser, XGE_SHAPE_EX_CMD_LINE_TO, tStart, (xge_vec2_t){0.0f, 0.0f}, (xge_vec2_t){0.0f, 0.0f}, pShape->tCurrent);
						if ( iRet != XGE_OK ) return iRet;
					}
					bConsumed = 1;
				}
				if ( !bConsumed ) return XGE_ERROR_INVALID_ARGUMENT;
				cPrevCommand = cCommand;
				break;
			}
			case 'h': {
				float fX;
				int bConsumed = 0;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					if ( !__xgeShapeExSvgReadNumber(&pText, &fX) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( bRelative ) fX += pShape->tCurrent.fX;
					{
						xge_vec2_t tStart = pShape->tCurrent;
						iRet = xgeShapeExLineTo(pShape, fX, pShape->tCurrent.fY);
						if ( iRet != XGE_OK ) return iRet;
						iRet = __xgeShapeExSvgPathEvent(pEvent, pUser, XGE_SHAPE_EX_CMD_LINE_TO, tStart, (xge_vec2_t){0.0f, 0.0f}, (xge_vec2_t){0.0f, 0.0f}, pShape->tCurrent);
						if ( iRet != XGE_OK ) return iRet;
					}
					bConsumed = 1;
				}
				if ( !bConsumed ) return XGE_ERROR_INVALID_ARGUMENT;
				cPrevCommand = cCommand;
				break;
			}
			case 'v': {
				float fY;
				int bConsumed = 0;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					if ( !__xgeShapeExSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( bRelative ) fY += pShape->tCurrent.fY;
					{
						xge_vec2_t tStart = pShape->tCurrent;
						iRet = xgeShapeExLineTo(pShape, pShape->tCurrent.fX, fY);
						if ( iRet != XGE_OK ) return iRet;
						iRet = __xgeShapeExSvgPathEvent(pEvent, pUser, XGE_SHAPE_EX_CMD_LINE_TO, tStart, (xge_vec2_t){0.0f, 0.0f}, (xge_vec2_t){0.0f, 0.0f}, pShape->tCurrent);
						if ( iRet != XGE_OK ) return iRet;
					}
					bConsumed = 1;
				}
				if ( !bConsumed ) return XGE_ERROR_INVALID_ARGUMENT;
				cPrevCommand = cCommand;
				break;
			}
			case 'c': {
				float fC1X, fC1Y, fC2X, fC2Y, fX, fY;
				int bConsumed = 0;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					if ( !__xgeShapeExSvgReadNumber(&pText, &fC1X) || !__xgeShapeExSvgReadNumber(&pText, &fC1Y) ||
					     !__xgeShapeExSvgReadNumber(&pText, &fC2X) || !__xgeShapeExSvgReadNumber(&pText, &fC2Y) ||
					     !__xgeShapeExSvgReadNumber(&pText, &fX) || !__xgeShapeExSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( bRelative ) {
						fC1X += pShape->tCurrent.fX; fC1Y += pShape->tCurrent.fY;
						fC2X += pShape->tCurrent.fX; fC2Y += pShape->tCurrent.fY;
						fX += pShape->tCurrent.fX; fY += pShape->tCurrent.fY;
					}
					{
						xge_vec2_t tStart = pShape->tCurrent;
						iRet = xgeShapeExCubicTo(pShape, fC1X, fC1Y, fC2X, fC2Y, fX, fY);
						if ( iRet != XGE_OK ) return iRet;
						iRet = __xgeShapeExSvgPathEvent(pEvent, pUser, XGE_SHAPE_EX_CMD_CUBIC_TO, tStart, (xge_vec2_t){fC1X, fC1Y}, (xge_vec2_t){fC2X, fC2Y}, pShape->tCurrent);
						if ( iRet != XGE_OK ) return iRet;
					}
					bConsumed = 1;
				}
				if ( !bConsumed ) return XGE_ERROR_INVALID_ARGUMENT;
				cPrevCommand = cCommand;
				break;
			}
			case 's': {
				float fC1X, fC1Y, fC2X, fC2Y, fX, fY;
				int bConsumed = 0;
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
					{
						xge_vec2_t tStart = pShape->tCurrent;
						iRet = xgeShapeExCubicTo(pShape, fC1X, fC1Y, fC2X, fC2Y, fX, fY);
						if ( iRet != XGE_OK ) return iRet;
						iRet = __xgeShapeExSvgPathEvent(pEvent, pUser, XGE_SHAPE_EX_CMD_CUBIC_TO, tStart, (xge_vec2_t){fC1X, fC1Y}, (xge_vec2_t){fC2X, fC2Y}, pShape->tCurrent);
						if ( iRet != XGE_OK ) return iRet;
					}
					cPrevCommand = cCommand;
					bConsumed = 1;
				}
				if ( !bConsumed ) return XGE_ERROR_INVALID_ARGUMENT;
				cPrevCommand = cCommand;
				break;
			}
			case 'q': {
				float fCX, fCY, fX, fY;
				int bConsumed = 0;
				while ( __xgeShapeExSvgHasNumber(pText) ) {
					if ( !__xgeShapeExSvgReadNumber(&pText, &fCX) || !__xgeShapeExSvgReadNumber(&pText, &fCY) ||
					     !__xgeShapeExSvgReadNumber(&pText, &fX) || !__xgeShapeExSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
					if ( bRelative ) {
						fCX += pShape->tCurrent.fX; fCY += pShape->tCurrent.fY;
						fX += pShape->tCurrent.fX; fY += pShape->tCurrent.fY;
					}
					{
						xge_vec2_t tStart = pShape->tCurrent;
						iRet = xgeShapeExQuadTo(pShape, fCX, fCY, fX, fY);
						if ( iRet != XGE_OK ) return iRet;
						iRet = __xgeShapeExSvgPathEvent(pEvent, pUser, XGE_SHAPE_EX_CMD_QUAD_TO, tStart, (xge_vec2_t){fCX, fCY}, (xge_vec2_t){0.0f, 0.0f}, pShape->tCurrent);
						if ( iRet != XGE_OK ) return iRet;
					}
					bConsumed = 1;
				}
				if ( !bConsumed ) return XGE_ERROR_INVALID_ARGUMENT;
				cPrevCommand = cCommand;
				break;
			}
			case 't': {
				float fCX, fCY, fX, fY;
				int bConsumed = 0;
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
					{
						xge_vec2_t tStart = pShape->tCurrent;
						iRet = xgeShapeExQuadTo(pShape, fCX, fCY, fX, fY);
						if ( iRet != XGE_OK ) return iRet;
						iRet = __xgeShapeExSvgPathEvent(pEvent, pUser, XGE_SHAPE_EX_CMD_QUAD_TO, tStart, (xge_vec2_t){fCX, fCY}, (xge_vec2_t){0.0f, 0.0f}, pShape->tCurrent);
						if ( iRet != XGE_OK ) return iRet;
					}
					cPrevCommand = cCommand;
					bConsumed = 1;
				}
				if ( !bConsumed ) return XGE_ERROR_INVALID_ARGUMENT;
				cPrevCommand = cCommand;
				break;
			}
			case 'a': {
				float fRX, fRY, fAxis, fX, fY;
				int bLargeArcFlag;
				int bSweepFlag;
				int bConsumed = 0;
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
					{
						xge_vec2_t tStart = pShape->tCurrent;
						int iCommandBefore = pShape->iCommandCount;
						int iPointBefore = pShape->iPointCount;
						iArcRet = xgeShapeExArcTo(pShape, fRX, fRY, fAxis, bLargeArcFlag, bSweepFlag, fX, fY);
						if ( iArcRet != XGE_OK ) {
							return iArcRet;
						}
						iRet = __xgeShapeExSvgPathArcEvent(pShape, iCommandBefore, iPointBefore, tStart, pEvent, pUser);
						if ( iRet != XGE_OK ) return iRet;
					}
					bConsumed = 1;
				}
				if ( !bConsumed ) return XGE_ERROR_INVALID_ARGUMENT;
				cPrevCommand = cCommand;
				break;
			}
			case 'z':
			{
				xge_vec2_t tStart = pShape->tCurrent;
				xge_vec2_t tEnd = pShape->tStart;
				iRet = xgeShapeExClose(pShape);
				if ( iRet != XGE_OK ) return iRet;
				iRet = __xgeShapeExSvgPathEvent(pEvent, pUser, XGE_SHAPE_EX_CMD_CLOSE, tStart, (xge_vec2_t){0.0f, 0.0f}, (xge_vec2_t){0.0f, 0.0f}, tEnd);
				if ( iRet != XGE_OK ) return iRet;
				cPrevCommand = cCommand;
				cCommand = 0;
				break;
			}
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
	iRet = __xgeShapeExAppendSvgPathNoRollback(pShape, sPath, NULL, NULL);
	return __xgeShapeExSvgPathFinish(pShape, &tSnapshot, iRet);
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

static int __xgeShapeExSvgPathAppendFormat(char* sBuffer, int iBufferSize, int* pLength, const char* sFormat, ...)
{
	va_list args;
	int iWritten;

	if ( (pLength == NULL) || (sFormat == NULL) || (*pLength < 0) || (iBufferSize < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	va_start(args, sFormat);
	if ( (sBuffer != NULL) && (*pLength < iBufferSize) ) {
		iWritten = vsnprintf(sBuffer + *pLength, (size_t)(iBufferSize - *pLength), sFormat, args);
	} else {
		iWritten = vsnprintf(NULL, 0, sFormat, args);
	}
	va_end(args);
	if ( iWritten < 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iWritten > INT_MAX - *pLength ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pLength += iWritten;
	return XGE_OK;
}

int xgeShapeExGetSvgPathData(xge_shape_ex pShape, char* sBuffer, int iBufferSize, int* pRequiredSize)
{
	int iPointRead;
	int iLength;
	int i;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (iBufferSize < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (sBuffer != NULL) && (iBufferSize > 0) ) {
		sBuffer[0] = '\0';
	}
	iPointRead = 0;
	iLength = 0;
	for ( i = 0; i < pShape->iCommandCount; i++ ) {
		uint8_t iCommand = pShape->pCommands[i];

		if ( i > 0 ) {
			iRet = __xgeShapeExSvgPathAppendFormat(sBuffer, iBufferSize, &iLength, " ");
			if ( iRet != XGE_OK ) return iRet;
		}
		if ( iCommand == XGE_SHAPE_EX_CMD_MOVE_TO ) {
			if ( iPointRead + 1 > pShape->iPointCount ) return XGE_ERROR_INVALID_ARGUMENT;
			iRet = __xgeShapeExSvgPathAppendFormat(sBuffer, iBufferSize, &iLength, "M%.9g %.9g", pShape->pPoints[iPointRead].fX, pShape->pPoints[iPointRead].fY);
			iPointRead++;
		} else if ( iCommand == XGE_SHAPE_EX_CMD_LINE_TO ) {
			if ( iPointRead + 1 > pShape->iPointCount ) return XGE_ERROR_INVALID_ARGUMENT;
			iRet = __xgeShapeExSvgPathAppendFormat(sBuffer, iBufferSize, &iLength, "L%.9g %.9g", pShape->pPoints[iPointRead].fX, pShape->pPoints[iPointRead].fY);
			iPointRead++;
		} else if ( iCommand == XGE_SHAPE_EX_CMD_QUAD_TO ) {
			if ( iPointRead + 2 > pShape->iPointCount ) return XGE_ERROR_INVALID_ARGUMENT;
			iRet = __xgeShapeExSvgPathAppendFormat(sBuffer, iBufferSize, &iLength, "Q%.9g %.9g %.9g %.9g",
				pShape->pPoints[iPointRead].fX, pShape->pPoints[iPointRead].fY,
				pShape->pPoints[iPointRead + 1].fX, pShape->pPoints[iPointRead + 1].fY);
			iPointRead += 2;
		} else if ( iCommand == XGE_SHAPE_EX_CMD_CUBIC_TO ) {
			if ( iPointRead + 3 > pShape->iPointCount ) return XGE_ERROR_INVALID_ARGUMENT;
			iRet = __xgeShapeExSvgPathAppendFormat(sBuffer, iBufferSize, &iLength, "C%.9g %.9g %.9g %.9g %.9g %.9g",
				pShape->pPoints[iPointRead].fX, pShape->pPoints[iPointRead].fY,
				pShape->pPoints[iPointRead + 1].fX, pShape->pPoints[iPointRead + 1].fY,
				pShape->pPoints[iPointRead + 2].fX, pShape->pPoints[iPointRead + 2].fY);
			iPointRead += 3;
		} else if ( iCommand == XGE_SHAPE_EX_CMD_CLOSE ) {
			iRet = __xgeShapeExSvgPathAppendFormat(sBuffer, iBufferSize, &iLength, "Z");
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	if ( iPointRead != pShape->iPointCount ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (sBuffer != NULL) && (iBufferSize > 0) ) {
		if ( iLength >= iBufferSize ) {
			sBuffer[iBufferSize - 1] = '\0';
		} else {
			sBuffer[iLength] = '\0';
		}
	}
	if ( pRequiredSize != NULL ) {
		*pRequiredSize = iLength + 1;
	}
	if ( (sBuffer != NULL) && (iBufferSize < iLength + 1) ) {
		return XGE_ERROR_BUFFER_TOO_SMALL;
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

	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExCoordsFinite(fX1, fY1) || !__xgeShapeExCoordsFinite(fX2, fY2) ||
	     !__xgeShapeExStopsFinite(pStops, iStopCount) ) {
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
	return xgeShapeExFillRadialGradientEx(pShape, fCX, fCY, fRadius, fFX, fFY, 0.0f, iUnits, pStops, iStopCount);
}

int xgeShapeExFillRadialGradientEx(xge_shape_ex pShape, float fCX, float fCY, float fRadius, float fFX, float fFY, float fFocalRadius, int iUnits, const xge_shape_ex_color_stop_t* pStops, int iStopCount)
{
	int i;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExCoordsFinite(fCX, fCY) ||
	     !__xgeShapeExCoordsFinite(fRadius, fFocalRadius) || !__xgeShapeExCoordsFinite(fFX, fFY) ||
	     !__xgeShapeExStopsFinite(pStops, iStopCount) || (fRadius < 0.0f) || (fFocalRadius < 0.0f) ) {
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
	pShape->fFillFR = fFocalRadius;
	pShape->iFillGradientUnits = iUnits;
	pShape->iFillGradientSpread = XGE_SHAPE_EX_GRADIENT_SPREAD_PAD;
	pShape->tFillGradientTransform = __xgeShapeExMatrixIdentity();
	return XGE_OK;
}

int xgeShapeExFillRadialGradientGet(xge_shape_ex pShape, float* pCX, float* pCY, float* pRadius, float* pFX, float* pFY, int* pUnits, const xge_shape_ex_color_stop_t** ppStops, int* pStopCount)
{
	return xgeShapeExFillRadialGradientGetEx(pShape, pCX, pCY, pRadius, pFX, pFY, NULL, pUnits, ppStops, pStopCount);
}

int xgeShapeExFillRadialGradientGetEx(xge_shape_ex pShape, float* pCX, float* pCY, float* pRadius, float* pFX, float* pFY, float* pFocalRadius, int* pUnits, const xge_shape_ex_color_stop_t** ppStops, int* pStopCount)
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
	if ( pFocalRadius != NULL ) *pFocalRadius = pShape->fFillFR;
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
	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExMatrixFinite(pMatrix) ) {
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

	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExCoordsFinite(fX1, fY1) || !__xgeShapeExCoordsFinite(fX2, fY2) ||
	     !__xgeShapeExStopsFinite(pStops, iStopCount) ) {
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
	return xgeShapeExStrokeRadialGradientEx(pShape, fCX, fCY, fRadius, fFX, fFY, 0.0f, iUnits, pStops, iStopCount);
}

int xgeShapeExStrokeRadialGradientEx(xge_shape_ex pShape, float fCX, float fCY, float fRadius, float fFX, float fFY, float fFocalRadius, int iUnits, const xge_shape_ex_color_stop_t* pStops, int iStopCount)
{
	int i;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExCoordsFinite(fCX, fCY) ||
	     !__xgeShapeExCoordsFinite(fRadius, fFocalRadius) || !__xgeShapeExCoordsFinite(fFX, fFY) ||
	     !__xgeShapeExStopsFinite(pStops, iStopCount) || (fRadius < 0.0f) || (fFocalRadius < 0.0f) ) {
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
	pShape->fStrokeFR = fFocalRadius;
	pShape->iStrokeGradientUnits = iUnits;
	pShape->iStrokeGradientSpread = XGE_SHAPE_EX_GRADIENT_SPREAD_PAD;
	pShape->tStrokeGradientTransform = __xgeShapeExMatrixIdentity();
	return XGE_OK;
}

int xgeShapeExStrokeRadialGradientGet(xge_shape_ex pShape, float* pCX, float* pCY, float* pRadius, float* pFX, float* pFY, int* pUnits, const xge_shape_ex_color_stop_t** ppStops, int* pStopCount)
{
	return xgeShapeExStrokeRadialGradientGetEx(pShape, pCX, pCY, pRadius, pFX, pFY, NULL, pUnits, ppStops, pStopCount);
}

int xgeShapeExStrokeRadialGradientGetEx(xge_shape_ex pShape, float* pCX, float* pCY, float* pRadius, float* pFX, float* pFY, float* pFocalRadius, int* pUnits, const xge_shape_ex_color_stop_t** ppStops, int* pStopCount)
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
	if ( pFocalRadius != NULL ) *pFocalRadius = pShape->fStrokeFR;
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
	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExMatrixFinite(pMatrix) ) {
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
	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExFloatFinite(fWidth) ) return XGE_ERROR_INVALID_ARGUMENT;
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
	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExFloatFinite(fLimit) || (fLimit < 0.0f) ) return XGE_ERROR_INVALID_ARGUMENT;
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

	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExFloatFinite(fDashOffset) ||
	     (iDashCount < 0) || ((pDashPattern == NULL) && (iDashCount > 0)) || ((pDashPattern != NULL) && (iDashCount == 0)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( ((iDashCount & 1) != 0) && (iDashCount > (INT_MAX / 2)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < iDashCount; i++ ) {
		if ( !__xgeShapeExFloatFinite(pDashPattern[i]) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
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
	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExCoordsFinite(fBegin, fEnd) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pShape->bTrimPath = 1;
	pShape->fTrimBegin = fBegin;
	pShape->fTrimEnd = fEnd;
	pShape->bTrimSimultaneous = bSimultaneous != 0;
	return XGE_OK;
}

int xgeShapeExTrimPathGet(xge_shape_ex pShape, float* pBegin, float* pEnd, int* pSimultaneous, int* pEnabled)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pBegin != NULL ) {
		*pBegin = pShape->fTrimBegin;
	}
	if ( pEnd != NULL ) {
		*pEnd = pShape->fTrimEnd;
	}
	if ( pSimultaneous != NULL ) {
		*pSimultaneous = pShape->bTrimSimultaneous;
	}
	if ( pEnabled != NULL ) {
		*pEnabled = pShape->bTrimPath;
	}
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
	pShape->bTrimSimultaneous = 0;
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
	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExFloatFinite(fOpacity) ) return XGE_ERROR_INVALID_ARGUMENT;
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

int xgeShapeExBlend(xge_shape_ex pShape, int iBlend)
{
	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExBlendValid(iBlend) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pShape->iBlend = iBlend;
	pShape->bBlendSet = 1;
	return XGE_OK;
}

int xgeShapeExBlendClear(xge_shape_ex pShape)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pShape->iBlend = XGE_BLEND_ALPHA;
	pShape->bBlendSet = 0;
	return XGE_OK;
}

int xgeShapeExBlendGet(xge_shape_ex pShape, int* pBlend, int* pBlendSet)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pBlend != NULL ) {
		*pBlend = pShape->iBlend;
	}
	if ( pBlendSet != NULL ) {
		*pBlendSet = pShape->bBlendSet;
	}
	return XGE_OK;
}

static xge_rect_t __xgeShapeExExpandBoundsForStroke(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, xge_rect_t tBounds, xge_shape_ex_matrix_t tMatrix);

static int __xgeShapeExGetBoundsInternalEx(xge_shape_ex pShape, float fTolerance, xge_shape_ex_matrix_t tParent, xge_rect_t* pBounds, int iDepth)
{
	xge_shape_ex_flat_path_t tFlat;
	xge_shape_ex_flat_path_t tTrimmed;
	const xge_shape_ex_flat_path_t* pBoundsFlat;
	xge_shape_ex_matrix_t tMatrix;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (pBounds == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iDepth > XGE_SHAPE_EX_CLIP_BOUNDS_MAX_DEPTH ) {
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
	if ( pShape->bClipRect ) {
		*pBounds = __xgeShapeExRectIntersect(*pBounds, __xgeShapeExMatrixRectBounds(tMatrix, pShape->tClipRect));
	}
	if ( pShape->iClipShapeCount > 0 ) {
		xge_rect_t tClipUnion;
		int bHasClipBounds;
		int bHasIncludeClip;
		int i;

		memset(&tClipUnion, 0, sizeof(tClipUnion));
		bHasClipBounds = 0;
		bHasIncludeClip = 0;
		for ( i = 0; i < pShape->iClipShapeCount; i++ ) {
			xge_rect_t tClipBounds;
			int iMode = pShape->pClipShapeModes != NULL ? pShape->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

			if ( iMode == XGE_SHAPE_EX_CLIP_SUBTRACT ) {
				continue;
			}
			bHasIncludeClip = 1;

			iRet = __xgeShapeExGetBoundsInternalEx(pShape->pClipShapes[i], fTolerance, tMatrix, &tClipBounds, iDepth + 1);
			if ( iRet != XGE_OK ) {
				__xgeShapeExFlatFree(&tTrimmed);
				__xgeShapeExFlatFree(&tFlat);
				return iRet;
			}
			if ( (tClipBounds.fW > 0.0f) && (tClipBounds.fH > 0.0f) ) {
				if ( !bHasClipBounds ) {
					tClipUnion = tClipBounds;
					bHasClipBounds = 1;
				} else {
					tClipUnion = __xgeShapeExRectUnion(tClipUnion, tClipBounds);
				}
			}
		}
		if ( bHasIncludeClip ) {
			if ( bHasClipBounds ) {
				*pBounds = __xgeShapeExRectIntersect(*pBounds, tClipUnion);
			} else {
				memset(pBounds, 0, sizeof(*pBounds));
			}
		}
	}
	__xgeShapeExFlatFree(&tTrimmed);
	__xgeShapeExFlatFree(&tFlat);
	return XGE_OK;
}

static int __xgeShapeExGetBoundsInternal(xge_shape_ex pShape, float fTolerance, xge_shape_ex_matrix_t tParent, xge_rect_t* pBounds)
{
	return __xgeShapeExGetBoundsInternalEx(pShape, fTolerance, tParent, pBounds, 0);
}

static xge_rect_t __xgeShapeExBoundsIncludePoint(xge_rect_t tBounds, xge_vec2_t tPoint)
{
	float fLeft = tBounds.fX;
	float fTop = tBounds.fY;
	float fRight = tBounds.fX + tBounds.fW;
	float fBottom = tBounds.fY + tBounds.fH;

	if ( tPoint.fX < fLeft ) fLeft = tPoint.fX;
	if ( tPoint.fY < fTop ) fTop = tPoint.fY;
	if ( tPoint.fX > fRight ) fRight = tPoint.fX;
	if ( tPoint.fY > fBottom ) fBottom = tPoint.fY;
	tBounds.fX = fLeft;
	tBounds.fY = fTop;
	tBounds.fW = fRight - fLeft;
	tBounds.fH = fBottom - fTop;
	if ( tBounds.fW < 0.0f ) tBounds.fW = 0.0f;
	if ( tBounds.fH < 0.0f ) tBounds.fH = 0.0f;
	return tBounds;
}

static xge_rect_t __xgeShapeExBoundsIncludeCircle(xge_rect_t tBounds, xge_vec2_t tCenter, float fRadius)
{
	xge_vec2_t tPoint;

	if ( fRadius <= 0.0f ) {
		return __xgeShapeExBoundsIncludePoint(tBounds, tCenter);
	}
	tPoint.fX = tCenter.fX - fRadius;
	tPoint.fY = tCenter.fY - fRadius;
	tBounds = __xgeShapeExBoundsIncludePoint(tBounds, tPoint);
	tPoint.fX = tCenter.fX + fRadius;
	tPoint.fY = tCenter.fY + fRadius;
	return __xgeShapeExBoundsIncludePoint(tBounds, tPoint);
}

static int __xgeShapeExStrokeMiterJoinWithinLimit(xge_vec2_t tPrev, xge_vec2_t tCurr, xge_vec2_t tNext, float fMiterLimit)
{
	xge_vec2_t tD1;
	xge_vec2_t tD2;
	float fCross;
	float fDot;
	float fAngle;
	float fSin;

	if ( !__xgeShapeExNormalizeSegment(tPrev, tCurr, &tD1) ||
	     !__xgeShapeExNormalizeSegment(tCurr, tNext, &tD2) ) {
		return 0;
	}
	fCross = tD1.fX * tD2.fY - tD1.fY * tD2.fX;
	if ( fabsf(fCross) <= XGE_SHAPE_EX_EPSILON ) {
		return 0;
	}
	fDot = tD1.fX * tD2.fX + tD1.fY * tD2.fY;
	if ( fDot < -1.0f ) fDot = -1.0f;
	if ( fDot > 1.0f ) fDot = 1.0f;
	fAngle = acosf(fDot);
	fSin = sinf(fAngle * 0.5f);
	if ( fSin <= XGE_SHAPE_EX_EPSILON ) {
		return 0;
	}
	return (1.0f / fSin) <= (fMiterLimit + XGE_SHAPE_EX_EPSILON);
}

static int __xgeShapeExFlatStrokeHasDrawableContour(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat)
{
	int i;

	if ( (pShape == NULL) || (pFlat == NULL) ) {
		return 0;
	}
	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		const xge_shape_ex_flat_contour_t* pContour = &pFlat->pContours[i];
		const xge_vec2_t* pSrc;
		xge_vec2_t* pPoints;
		int iPointCount;

		if ( (pContour->iCount <= 0) || (pContour->iStart < 0) || ((pContour->iStart + pContour->iCount) > pFlat->iPointCount) ) {
			continue;
		}
		pSrc = pFlat->pPoints + pContour->iStart;
		pPoints = NULL;
		iPointCount = 0;
		if ( __xgeShapeExCompactStrokePoints(pSrc, pContour->iCount, pContour->bClosed, &pPoints, &iPointCount) != XGE_OK ) {
			return 1;
		}
		xrtFree(pPoints);
		if ( iPointCount >= 2 ) {
			return 1;
		}
		if ( (iPointCount == 1) && (pShape->iLineCap != XGE_SHAPE_EX_CAP_BUTT) ) {
			return 1;
		}
	}
	return 0;
}

static xge_rect_t __xgeShapeExExpandBoundsForStrokeOutlines(xge_shape_ex pShape, const xge_shape_ex_flat_path_t* pFlat, xge_rect_t tBounds, float fStrokeWidth)
{
	float fHalfWidth;
	float fMiterLimit;
	int i;

	if ( (pFlat == NULL) || (fStrokeWidth <= 0.0f) ) {
		return tBounds;
	}
	fHalfWidth = fStrokeWidth * 0.5f;
	fMiterLimit = pShape->fMiterLimit;
	if ( fMiterLimit < 1.0f ) {
		fMiterLimit = 1.0f;
	}
	for ( i = 0; i < pFlat->iContourCount; i++ ) {
		const xge_shape_ex_flat_contour_t* pContour = &pFlat->pContours[i];
		const xge_vec2_t* pSrc;
		xge_vec2_t* pPoints;
		int iPointCount;
		int iJoinStart;
		int iJoinEnd;
		int j;

		if ( (pContour->iCount <= 0) || (pContour->iStart < 0) || ((pContour->iStart + pContour->iCount) > pFlat->iPointCount) ) {
			continue;
		}
		pSrc = pFlat->pPoints + pContour->iStart;
		pPoints = NULL;
		iPointCount = 0;
		if ( __xgeShapeExCompactStrokePoints(pSrc, pContour->iCount, pContour->bClosed, &pPoints, &iPointCount) != XGE_OK ) {
			return tBounds;
		}
		if ( iPointCount == 1 ) {
			if ( pShape->iLineCap != XGE_SHAPE_EX_CAP_BUTT ) {
				tBounds = __xgeShapeExBoundsIncludeCircle(tBounds, pPoints[0], fHalfWidth);
			}
			xrtFree(pPoints);
			continue;
		}
		if ( !pContour->bClosed && (pShape->iLineCap == XGE_SHAPE_EX_CAP_SQUARE) ) {
			xge_vec2_t tLeft;
			xge_vec2_t tRight;

			__xgeShapeExStrokeEndpointPair(pPoints[0], pPoints[1], fHalfWidth, pShape->iLineCap, 0, &tLeft, &tRight);
			tBounds = __xgeShapeExBoundsIncludePoint(tBounds, tLeft);
			tBounds = __xgeShapeExBoundsIncludePoint(tBounds, tRight);
			__xgeShapeExStrokeEndpointPair(pPoints[iPointCount - 1], pPoints[iPointCount - 2], fHalfWidth, pShape->iLineCap, 1, &tLeft, &tRight);
			tBounds = __xgeShapeExBoundsIncludePoint(tBounds, tLeft);
			tBounds = __xgeShapeExBoundsIncludePoint(tBounds, tRight);
		}
		if ( iPointCount < 3 ) {
			xrtFree(pPoints);
			continue;
		}
		if ( pShape->iLineJoin != XGE_SHAPE_EX_JOIN_MITER ) {
			xrtFree(pPoints);
			continue;
		}
		iJoinStart = pContour->bClosed ? 0 : 1;
		iJoinEnd = pContour->bClosed ? iPointCount : (iPointCount - 1);
		for ( j = iJoinStart; j < iJoinEnd; j++ ) {
			int iPrev = (j + iPointCount - 1) % iPointCount;
			int iNext = (j + 1) % iPointCount;
			xge_vec2_t tLeft = __xgeShapeExStrokeSidePoint(pPoints[iPrev], pPoints[j], pPoints[iNext], fHalfWidth, fMiterLimit, 1);
			xge_vec2_t tRight = __xgeShapeExStrokeSidePoint(pPoints[iPrev], pPoints[j], pPoints[iNext], fHalfWidth, fMiterLimit, -1);

			if ( !__xgeShapeExStrokeMiterJoinWithinLimit(pPoints[iPrev], pPoints[j], pPoints[iNext], fMiterLimit) ) {
				continue;
			}
			tBounds = __xgeShapeExBoundsIncludePoint(tBounds, tLeft);
			tBounds = __xgeShapeExBoundsIncludePoint(tBounds, tRight);
		}
		xrtFree(pPoints);
	}
	return tBounds;
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
	if ( !__xgeShapeExFlatStrokeHasDrawableContour(pShape, pFlat) ) {
		return tBounds;
	}
	fExpand = fStrokeWidth * 0.5f;
	tBounds.fX -= fExpand;
	tBounds.fY -= fExpand;
	tBounds.fW += fExpand * 2.0f;
	tBounds.fH += fExpand * 2.0f;
	tBounds = __xgeShapeExExpandBoundsForStrokeOutlines(pShape, pFlat, tBounds, fStrokeWidth);
	return tBounds;
}

static int __xgeShapeExGetLocalBoundsEx(xge_shape_ex pShape, float fTolerance, xge_rect_t* pBounds, int iDepth)
{
	xge_shape_ex_flat_path_t tFlat;
	xge_shape_ex_flat_path_t tTrimmed;
	const xge_shape_ex_flat_path_t* pBoundsFlat;
	int iRet;

	if ( !__xgeShapeExValid(pShape) || (pBounds == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iDepth > XGE_SHAPE_EX_CLIP_BOUNDS_MAX_DEPTH ) {
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
	if ( pShape->bClipRect ) {
		*pBounds = __xgeShapeExRectIntersect(*pBounds, pShape->tClipRect);
	}
	if ( pShape->iClipShapeCount > 0 ) {
		xge_rect_t tClipUnion;
		int bHasClipBounds;
		int bHasIncludeClip;
		int i;

		memset(&tClipUnion, 0, sizeof(tClipUnion));
		bHasClipBounds = 0;
		bHasIncludeClip = 0;
		for ( i = 0; i < pShape->iClipShapeCount; i++ ) {
			xge_rect_t tClipBounds;
			int iMode = pShape->pClipShapeModes != NULL ? pShape->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

			if ( iMode == XGE_SHAPE_EX_CLIP_SUBTRACT ) {
				continue;
			}
			bHasIncludeClip = 1;

			iRet = __xgeShapeExGetBoundsInternalEx(pShape->pClipShapes[i], fTolerance, __xgeShapeExMatrixIdentity(), &tClipBounds, iDepth + 1);
			if ( iRet != XGE_OK ) {
				__xgeShapeExFlatFree(&tTrimmed);
				__xgeShapeExFlatFree(&tFlat);
				return iRet;
			}
			if ( (tClipBounds.fW > 0.0f) && (tClipBounds.fH > 0.0f) ) {
				if ( !bHasClipBounds ) {
					tClipUnion = tClipBounds;
					bHasClipBounds = 1;
				} else {
					tClipUnion = __xgeShapeExRectUnion(tClipUnion, tClipBounds);
				}
			}
		}
		if ( bHasIncludeClip ) {
			if ( bHasClipBounds ) {
				*pBounds = __xgeShapeExRectIntersect(*pBounds, tClipUnion);
			} else {
				memset(pBounds, 0, sizeof(*pBounds));
			}
		}
	}
	__xgeShapeExFlatFree(&tTrimmed);
	__xgeShapeExFlatFree(&tFlat);
	return XGE_OK;
}

static int __xgeShapeExGetLocalBounds(xge_shape_ex pShape, float fTolerance, xge_rect_t* pBounds)
{
	return __xgeShapeExGetLocalBoundsEx(pShape, fTolerance, pBounds, 0);
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

static int __xgeShapeExContainsPointInternal(xge_shape_ex pShape, xge_vec2_t tPoint, float fTolerance, xge_shape_ex_matrix_t tParent, int iDepth, int* pContains)
{
	xge_shape_ex_flat_path_t tFlat;
	xge_shape_ex_flat_path_t tTrimmed;
	const xge_shape_ex_flat_path_t* pHitFlat;
	xge_shape_ex_matrix_t tMatrix;
	xge_shape_ex_matrix_t tInverse;
	int iRet;
	int bFillVisible;
	int bStrokeVisible;

	if ( !__xgeShapeExValid(pShape) || (pContains == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pContains = 0;
	if ( iDepth > XGE_SHAPE_EX_CLIP_BOUNDS_MAX_DEPTH ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( fTolerance <= 0.0f ) {
		fTolerance = XGE_SHAPE_EX_DEFAULT_TOLERANCE;
	}
	if ( !pShape->bVisible || (pShape->fOpacity <= 0.0f) ) {
		return XGE_OK;
	}
	bFillVisible = __xgeShapeExFillVisible(pShape, pShape->iFillColor);
	bStrokeVisible = (pShape->fStrokeWidth > 0.0f) && __xgeShapeExStrokeVisible(pShape, pShape->iStrokeColor);
	if ( !bFillVisible && !bStrokeVisible ) {
		return XGE_OK;
	}
	tMatrix = __xgeShapeExMatrixMul(tParent, pShape->tTransform);
	if ( pShape->bClipRect ) {
		xge_vec2_t tLocalPoint;

		if ( !__xgeShapeExMatrixInverse(tMatrix, &tInverse) ) {
			return XGE_OK;
		}
		tLocalPoint = __xgeShapeExMatrixPoint(tInverse, tPoint);
		if ( !__xgeShapeExRectContainsPoint(pShape->tClipRect, tLocalPoint) ) {
			return XGE_OK;
		}
	}
	if ( pShape->iClipShapeCount > 0 ) {
		int bAnyClipContains;
		int bHasIncludeClip;
		int i;

		bAnyClipContains = 0;
		bHasIncludeClip = 0;
		for ( i = 0; i < pShape->iClipShapeCount; i++ ) {
			int bClipContains;
			int iMode = pShape->pClipShapeModes != NULL ? pShape->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

			iRet = __xgeShapeExContainsPointInternal(pShape->pClipShapes[i], tPoint, fTolerance, tMatrix, iDepth + 1, &bClipContains);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			if ( iMode == XGE_SHAPE_EX_CLIP_SUBTRACT ) {
				if ( bClipContains ) {
					return XGE_OK;
				}
				continue;
			}
			bHasIncludeClip = 1;
			if ( bClipContains ) {
				bAnyClipContains = 1;
			}
		}
		if ( bHasIncludeClip && !bAnyClipContains ) {
			return XGE_OK;
		}
	}
	iRet = __xgeShapeExFlatten(pShape, tMatrix, fTolerance, &tFlat);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	memset(&tTrimmed, 0, sizeof(tTrimmed));
	pHitFlat = &tFlat;
	if ( pShape->bTrimPath ) {
		iRet = __xgeShapeExFlatTrim(&tFlat, pShape->fTrimBegin, pShape->fTrimEnd, pShape->bTrimSimultaneous, &tTrimmed);
		if ( iRet != XGE_OK ) {
			__xgeShapeExFlatFree(&tFlat);
			return iRet;
		}
		pHitFlat = &tTrimmed;
	}
	if ( bFillVisible ) {
		*pContains = __xgeShapeExFlatContainsPoint(pHitFlat, pShape->iFillRule, tPoint);
	}
	if ( !*pContains && bStrokeVisible ) {
		float fStrokeScale = pShape->bStrokeNonScaling ? 1.0f : __xgeShapeExMatrixStrokeScale(tMatrix);
		float fStrokeWidth = pShape->fStrokeWidth * fStrokeScale;
		float fDashOffset = pShape->fDashOffset * fStrokeScale;
		const float* pDashPattern = pShape->pDashPattern;
		float* pScaledDashPattern = NULL;

		if ( (pShape->pDashPattern != NULL) && (pShape->iDashCount > 0) && (fabsf(fStrokeScale - 1.0f) > XGE_SHAPE_EX_EPSILON) ) {
			int i;

			pScaledDashPattern = (float*)xrtMalloc((size_t)pShape->iDashCount * sizeof(*pScaledDashPattern));
			if ( pScaledDashPattern == NULL ) {
				__xgeShapeExFlatFree(&tTrimmed);
				__xgeShapeExFlatFree(&tFlat);
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			for ( i = 0; i < pShape->iDashCount; i++ ) {
				pScaledDashPattern[i] = pShape->pDashPattern[i] * fStrokeScale;
			}
			pDashPattern = pScaledDashPattern;
		}
		if ( fStrokeWidth > XGE_SHAPE_EX_EPSILON ) {
			if ( __xgeShapeExDashTotal(pDashPattern, pShape->iDashCount) > XGE_SHAPE_EX_EPSILON ) {
				*pContains = __xgeShapeExFlatStrokeDashedContainsPoint(pShape, pHitFlat, fStrokeWidth, pDashPattern, pShape->iDashCount, fDashOffset, fTolerance, tPoint);
			} else {
				*pContains = __xgeShapeExFlatStrokeSolidContainsPoint(pShape, pHitFlat, fStrokeWidth, fTolerance, tPoint);
			}
		}
		xrtFree(pScaledDashPattern);
	}
	__xgeShapeExFlatFree(&tTrimmed);
	__xgeShapeExFlatFree(&tFlat);
	return XGE_OK;
}

static int __xgeShapeExSceneHitTestInternal(xge_shape_ex_scene pScene, xge_vec2_t tPoint, float fTolerance, xge_shape_ex_matrix_t tParent, xge_shape_ex* ppShape)
{
	xge_shape_ex_matrix_t tMatrix;
	int i;
	int iRet;

	if ( !__xgeShapeExSceneValid(pScene) || (ppShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppShape = NULL;
	if ( !pScene->bVisible || (pScene->fOpacity <= 0.0f) ) {
		return XGE_OK;
	}
	if ( fTolerance <= 0.0f ) {
		fTolerance = XGE_SHAPE_EX_DEFAULT_TOLERANCE;
	}
	tMatrix = __xgeShapeExMatrixMul(tParent, pScene->tTransform);
	if ( pScene->bClipRect ) {
		xge_shape_ex_matrix_t tInverse;
		xge_vec2_t tLocalPoint;

		if ( !__xgeShapeExMatrixInverse(tMatrix, &tInverse) ) {
			return XGE_OK;
		}
		tLocalPoint = __xgeShapeExMatrixPoint(tInverse, tPoint);
		if ( !__xgeShapeExRectContainsPoint(pScene->tClipRect, tLocalPoint) ) {
			return XGE_OK;
		}
	}
	if ( pScene->iClipShapeCount > 0 ) {
		int bAnyClipContains = 0;
		int bHasIncludeClip = 0;

		for ( i = 0; i < pScene->iClipShapeCount; i++ ) {
			int bClipContains = 0;
			int iMode = pScene->pClipShapeModes != NULL ? pScene->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

			iRet = __xgeShapeExContainsPointInternal(pScene->pClipShapes[i], tPoint, fTolerance, tMatrix, 0, &bClipContains);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			if ( iMode == XGE_SHAPE_EX_CLIP_SUBTRACT ) {
				if ( bClipContains ) {
					return XGE_OK;
				}
				continue;
			}
			bHasIncludeClip = 1;
			if ( bClipContains ) {
				bAnyClipContains = 1;
			}
		}
		if ( bHasIncludeClip && !bAnyClipContains ) {
			return XGE_OK;
		}
	}
	for ( i = pScene->iShapeCount - 1; i >= 0; i-- ) {
		xge_shape_ex pShape = pScene->pShapes[i];
		int bContains = 0;
		int bFillVisible;
		int bStrokeVisible;

		if ( !__xgeShapeExValid(pShape) || !pShape->bVisible || (pShape->fOpacity <= 0.0f) ) {
			continue;
		}
		bFillVisible = __xgeShapeExFillVisible(pShape, pShape->iFillColor);
		bStrokeVisible = (pShape->fStrokeWidth > 0.0f) && __xgeShapeExStrokeVisible(pShape, pShape->iStrokeColor);
		if ( !bFillVisible && !bStrokeVisible ) {
			continue;
		}
		iRet = __xgeShapeExContainsPointInternal(pShape, tPoint, fTolerance, tMatrix, 0, &bContains);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( bContains ) {
			*ppShape = pShape;
			return XGE_OK;
		}
	}
	return XGE_OK;
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

int xgeShapeExContainsPoint(xge_shape_ex pShape, float fX, float fY, float fTolerance, int* pContains)
{
	return xgeShapeExContainsPointEx(pShape, fX, fY, fTolerance, NULL, pContains);
}

int xgeShapeExContainsPointEx(xge_shape_ex pShape, float fX, float fY, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, int* pContains)
{
	xge_shape_ex_matrix_t tParent;
	xge_vec2_t tPoint;

	if ( !__xgeShapeExValid(pShape) || (pContains == NULL) || !__xgeShapeExCoordsFinite(fX, fY) || !__xgeShapeExFloatFinite(fTolerance) ||
	     ((pParentMatrix != NULL) && !__xgeShapeExMatrixFinite(pParentMatrix)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tParent = (pParentMatrix != NULL) ? *pParentMatrix : __xgeShapeExMatrixIdentity();
	tPoint.fX = fX;
	tPoint.fY = fY;
	return __xgeShapeExContainsPointInternal(pShape, tPoint, fTolerance, tParent, 0, pContains);
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
	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExRectFinite(tRect) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( tRect.fW < 0.0f ) tRect.fW = 0.0f;
	if ( tRect.fH < 0.0f ) tRect.fH = 0.0f;
	pShape->tClipRect = tRect;
	pShape->bClipRect = 1;
	return XGE_OK;
}

int xgeShapeExClipRectGet(xge_shape_ex pShape, xge_rect_t* pRect, int* pEnabled)
{
	if ( !__xgeShapeExValid(pShape) || ((pRect == NULL) && (pEnabled == NULL)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pRect != NULL ) {
		if ( pShape->bClipRect ) {
			*pRect = pShape->tClipRect;
		} else {
			memset(pRect, 0, sizeof(*pRect));
		}
	}
	if ( pEnabled != NULL ) {
		*pEnabled = pShape->bClipRect;
	}
	return XGE_OK;
}

int xgeShapeExClipShapeAdd(xge_shape_ex pShape, xge_shape_ex pClipShape)
{
	return __xgeShapeExClipShapeAddRef(pShape, pClipShape);
}

int xgeShapeExClipShapeAddEx(xge_shape_ex pShape, xge_shape_ex pClipShape, int iMode)
{
	return __xgeShapeExClipShapeAddRefEx(pShape, pClipShape, iMode);
}

int xgeShapeExClipShapeGetCount(xge_shape_ex pShape, int* pCount)
{
	if ( !__xgeShapeExValid(pShape) || (pCount == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pCount = pShape->iClipShapeCount;
	return XGE_OK;
}

int xgeShapeExClipShapeGetAt(xge_shape_ex pShape, int iIndex, xge_shape_ex* ppClipShape)
{
	if ( !__xgeShapeExValid(pShape) || (ppClipShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iIndex < 0) || (iIndex >= pShape->iClipShapeCount) ) {
		*ppClipShape = NULL;
		return XGE_ERROR_NOT_FOUND;
	}
	*ppClipShape = pShape->pClipShapes[iIndex];
	return XGE_OK;
}

int xgeShapeExClipShapeGetAtEx(xge_shape_ex pShape, int iIndex, xge_shape_ex* ppClipShape, int* pMode)
{
	if ( !__xgeShapeExValid(pShape) || ((ppClipShape == NULL) && (pMode == NULL)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iIndex < 0) || (iIndex >= pShape->iClipShapeCount) ) {
		if ( ppClipShape != NULL ) {
			*ppClipShape = NULL;
		}
		if ( pMode != NULL ) {
			*pMode = XGE_SHAPE_EX_CLIP_INTERSECT;
		}
		return XGE_ERROR_NOT_FOUND;
	}
	if ( ppClipShape != NULL ) {
		*ppClipShape = pShape->pClipShapes[iIndex];
	}
	if ( pMode != NULL ) {
		*pMode = pShape->pClipShapeModes != NULL ? pShape->pClipShapeModes[iIndex] : XGE_SHAPE_EX_CLIP_INTERSECT;
	}
	return XGE_OK;
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
	if ( !__xgeShapeExValid(pShape) || !__xgeShapeExMatrixFinite(pMatrix) ) return XGE_ERROR_INVALID_ARGUMENT;
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
	int iRet;

	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	iRet = xgeShapeExMatrixTranslate(&tLocal, fTX, fTY);
	if ( iRet != XGE_OK ) return iRet;
	return __xgeShapeExTransformConcat(&pShape->tTransform, &tLocal);
}

int xgeShapeExTransformScale(xge_shape_ex pShape, float fSX, float fSY)
{
	xge_shape_ex_matrix_t tLocal;
	int iRet;

	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	iRet = xgeShapeExMatrixScale(&tLocal, fSX, fSY);
	if ( iRet != XGE_OK ) return iRet;
	return __xgeShapeExTransformConcat(&pShape->tTransform, &tLocal);
}

int xgeShapeExTransformRotate(xge_shape_ex pShape, float fRadians)
{
	xge_shape_ex_matrix_t tLocal;
	int iRet;

	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	iRet = xgeShapeExMatrixRotate(&tLocal, fRadians);
	if ( iRet != XGE_OK ) return iRet;
	return __xgeShapeExTransformConcat(&pShape->tTransform, &tLocal);
}

int xgeShapeExTransformSkew(xge_shape_ex pShape, float fXRadians, float fYRadians)
{
	xge_shape_ex_matrix_t tLocal;
	int iRet;

	if ( !__xgeShapeExValid(pShape) ) return XGE_ERROR_INVALID_ARGUMENT;
	iRet = xgeShapeExMatrixSkew(&tLocal, fXRadians, fYRadians);
	if ( iRet != XGE_OK ) return iRet;
	return __xgeShapeExTransformConcat(&pShape->tTransform, &tLocal);
}

static int __xgeShapeExBlendNeedsComposite(int iBlend)
{
	switch ( iBlend ) {
		case XGE_BLEND_MULTIPLY:
		case XGE_BLEND_SCREEN:
		case XGE_BLEND_DARKEN:
		case XGE_BLEND_LIGHTEN:
		case XGE_BLEND_OVERLAY:
		case XGE_BLEND_COLOR_DODGE:
		case XGE_BLEND_COLOR_BURN:
		case XGE_BLEND_HARD_LIGHT:
		case XGE_BLEND_SOFT_LIGHT:
		case XGE_BLEND_DIFFERENCE:
		case XGE_BLEND_EXCLUSION:
		case XGE_BLEND_HUE:
		case XGE_BLEND_SATURATION:
		case XGE_BLEND_COLOR:
		case XGE_BLEND_LUMINOSITY:
			return 1;
		default:
			return 0;
	}
}

#define XGE_SHAPE_EX_BLEND_SOURCE_SCENE 0
#define XGE_SHAPE_EX_BLEND_SOURCE_SHAPE 1
#define XGE_SHAPE_EX_BLEND_SOURCE_GRADIENT 2
#define XGE_SHAPE_EX_BLEND_PAINT_ALL 0
#define XGE_SHAPE_EX_BLEND_PAINT_FILL 1
#define XGE_SHAPE_EX_BLEND_PAINT_STROKE 2

static int __xgeShapeExBlendShapeSource(xge_shape_ex pShape, int iPaint)
{
	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_SHAPE_EX_BLEND_SOURCE_SHAPE;
	}
	if ( (iPaint != XGE_SHAPE_EX_BLEND_PAINT_STROKE) &&
	     ((pShape->iFillType == XGE_SHAPE_EX_FILL_LINEAR_GRADIENT) ||
	      (pShape->iFillType == XGE_SHAPE_EX_FILL_RADIAL_GRADIENT)) ) {
		return XGE_SHAPE_EX_BLEND_SOURCE_GRADIENT;
	}
	if ( (iPaint != XGE_SHAPE_EX_BLEND_PAINT_FILL) &&
	     ((pShape->iStrokeType == XGE_SHAPE_EX_FILL_LINEAR_GRADIENT) ||
	      (pShape->iStrokeType == XGE_SHAPE_EX_FILL_RADIAL_GRADIENT)) ) {
		return XGE_SHAPE_EX_BLEND_SOURCE_GRADIENT;
	}
	return XGE_SHAPE_EX_BLEND_SOURCE_SHAPE;
}

static int __xgeShapeExBlendRendererEnsure(void)
{
	char sHeader[128];
	char sVertex[1536];
	char sFragment[16384];
	int iRet;

	if ( g_xgeShapeExBlendRenderer.bInitialized ) {
		return XGE_OK;
	}
	if ( __xgeGraphicsShaderHeaderGet(XGE_GPU_BACKEND_NONE, sHeader, (int)sizeof(sHeader)) < 0 ) {
		return XGE_ERROR_GPU_FAILED;
	}
	snprintf(
		sVertex, sizeof(sVertex),
		"%s"
		"layout(location = 0) in vec4 aPosition;\n"
		"layout(location = 1) in vec2 aUV;\n"
		"uniform vec2 uResolution;\n"
		"out vec2 vUV;\n"
		"void main() {\n"
		"  vec2 pos = (aPosition.xy / uResolution) * 2.0 - 1.0;\n"
		"  pos.y = -pos.y;\n"
		"  gl_Position = vec4(pos, 0.0, 1.0);\n"
		"  vUV = aUV;\n"
		"}\n",
		sHeader
	);
	snprintf(
		sFragment, sizeof(sFragment),
		"%s"
		"in vec2 vUV;\n"
		"uniform sampler2D uTexture;\n"
		"uniform sampler2D uTexture2;\n"
		"uniform int uBlendMode;\n"
		"uniform int uBlendSource;\n"
		"uniform float uBlendOpacity;\n"
		"out vec4 FragColor;\n"
		"const vec3 ONE = vec3(1.0);\n"
		"const vec3 LUM_I = vec3(54.0, 182.0, 19.0);\n"
		"vec3 channel8(vec3 color) { return floor(clamp(color, vec3(0.0), ONE) * 255.0 + 0.5); }\n"
		"float lum8(vec3 color) { return floor(dot(channel8(color), LUM_I) / 256.0); }\n"
		"vec3 truncDiv(vec3 value, float divisor) { return sign(value) * floor(abs(value) / divisor); }\n"
		"vec3 setLum(vec3 color, float l) {\n"
		"  vec3 value = channel8(color);\n"
		"  value += vec3(l - lum8(color));\n"
		"  float n = min(value.r, min(value.g, value.b));\n"
		"  float x = max(value.r, max(value.g, value.b));\n"
		"  if (n < 0.0) { value = vec3(l) + truncDiv((value - vec3(l)) * l, l - n); x = max(value.r, max(value.g, value.b)); }\n"
		"  if (x > 255.0) value = vec3(l) + truncDiv((value - vec3(l)) * (255.0 - l), x - l);\n"
		"  return value / 255.0;\n"
		"}\n"
		"float sat(vec3 color) {\n"
		"  return max(color.r, max(color.g, color.b)) - min(color.r, min(color.g, color.b));\n"
		"}\n"
		"vec3 setSat(vec3 color, float s) {\n"
		"  float rMin = step(color.r, color.g) * step(color.r, color.b);\n"
		"  float gMin = (1.0 - rMin) * step(color.g, color.r) * step(color.g, color.b);\n"
		"  vec3 minMask = vec3(rMin, gMin, 1.0 - rMin - gMin);\n"
		"  float bMax = step(color.r, color.b) * step(color.g, color.b);\n"
		"  float gMax = (1.0 - bMax) * step(color.r, color.g) * step(color.b, color.g);\n"
		"  vec3 maxMask = vec3(1.0 - bMax - gMax, gMax, bMax);\n"
		"  vec3 midMask = vec3(1.0) - minMask - maxMask;\n"
		"  float cMin = dot(color, minMask);\n"
		"  float cMid = dot(color, midMask);\n"
		"  float cMax = dot(color, maxMask);\n"
		"  float delta = cMax - cMin;\n"
		"  float deltaMask = sign(delta);\n"
		"  float scale = deltaMask * s / max(delta, 0.000001);\n"
		"  return maxMask * (s * deltaMask) + midMask * ((cMid - cMin) * scale);\n"
		"}\n"
		"vec3 overlayChannel(vec3 a, vec3 b) {\n"
		"  vec3 low = min(ONE, 2.0 * a * b);\n"
		"  vec3 high = ONE - min(ONE, 2.0 * (ONE - a) * (ONE - b));\n"
		"  return mix(low, high, step(vec3(0.5), b));\n"
		"}\n"
		"void main() {\n"
		"  vec4 src = texture(uTexture, vUV);\n"
		"  vec4 dst = texture(uTexture2, vUV);\n"
		"  vec3 Dc = dst.rgb;\n"
		"  float Da = dst.a;\n"
		"  vec3 sourceColor = src.a > 0.0 ? src.rgb / src.a : vec3(0.0);\n"
		"  vec3 Sc = uBlendSource == 1 ? src.rgb : (uBlendSource == 2 ? mix(Dc, sourceColor, src.a) : sourceColor);\n"
		"  vec3 Rc = Sc;\n"
		"  if (uBlendMode == 3) {\n"
		"    if (Da > 0.0) { Rc = Sc * min(ONE, Dc / Da); Rc = mix(Sc, Rc, Da); }\n"
		"  } else if (uBlendMode == 4) {\n"
		"    Rc = Sc + Dc - Sc * Dc;\n"
		"  } else if (uBlendMode == 6) {\n"
		"    if (Da > 0.0) { Rc = min(Sc, min(ONE, Dc / Da)); Rc = mix(Sc, Rc, Da); }\n"
		"  } else if (uBlendMode == 7) {\n"
		"    Rc = max(Sc, Dc);\n"
		"  } else if (uBlendMode == 8) {\n"
		"    if (Da > 0.0) { vec3 d = min(ONE, Dc / Da); Rc = overlayChannel(Sc, d); Rc = mix(Sc, Rc, Da); }\n"
		"  } else if (uBlendMode == 9) {\n"
		"    if (Da > 0.0) {\n"
		"      vec3 d = min(ONE, Dc / Da);\n"
		"      Rc.r = d.r > 0.0 ? (Sc.r < 1.0 ? min(1.0, d.r / (1.0 - Sc.r)) : 1.0) : 0.0;\n"
		"      Rc.g = d.g > 0.0 ? (Sc.g < 1.0 ? min(1.0, d.g / (1.0 - Sc.g)) : 1.0) : 0.0;\n"
		"      Rc.b = d.b > 0.0 ? (Sc.b < 1.0 ? min(1.0, d.b / (1.0 - Sc.b)) : 1.0) : 0.0;\n"
		"      Rc = mix(Sc, Rc, Da);\n"
		"    }\n"
		"  } else if (uBlendMode == 10) {\n"
		"    if (Da > 0.0) {\n"
		"      vec3 d = min(ONE, Dc / Da);\n"
		"      Rc.r = Sc.r > 0.0 ? 1.0 - min(1.0, (1.0 - d.r) / Sc.r) : (d.r < 1.0 ? 0.0 : 1.0);\n"
		"      Rc.g = Sc.g > 0.0 ? 1.0 - min(1.0, (1.0 - d.g) / Sc.g) : (d.g < 1.0 ? 0.0 : 1.0);\n"
		"      Rc.b = Sc.b > 0.0 ? 1.0 - min(1.0, (1.0 - d.b) / Sc.b) : (d.b < 1.0 ? 0.0 : 1.0);\n"
		"      Rc = mix(Sc, Rc, Da);\n"
		"    }\n"
		"  } else if (uBlendMode == 11) {\n"
		"    if (Da > 0.0) { vec3 d = min(ONE, Dc / Da); Rc = overlayChannel(d, Sc); Rc = mix(Sc, Rc, Da); }\n"
		"  } else if (uBlendMode == 12) {\n"
		"    if (Da > 0.0) {\n"
		"      vec3 d = min(ONE, Dc / Da);\n"
		"      vec3 dLow = ((16.0 * d - 12.0) * d + 4.0) * d;\n"
		"      vec3 dHigh = sqrt(d);\n"
		"      vec3 dd = mix(dHigh, dLow, step(d, vec3(0.25)));\n"
		"      vec3 low = d - (ONE - 2.0 * Sc) * d * (ONE - d);\n"
		"      vec3 high = d + (2.0 * Sc - ONE) * (dd - d);\n"
		"      Rc = mix(high, low, step(Sc, vec3(0.5)));\n"
		"      Rc = clamp(Rc, vec3(0.0), ONE);\n"
		"      Rc = mix(Sc, Rc, Da);\n"
		"    }\n"
		"  } else if (uBlendMode == 13) {\n"
		"    Rc = abs(Dc - Sc);\n"
		"  } else if (uBlendMode == 14) {\n"
		"    Rc = Sc + Dc - 2.0 * Sc * Dc;\n"
		"  } else if (uBlendMode == 15) {\n"
		"    if (Da > 0.0) { vec3 d = min(ONE, Dc / Da); Rc = setSat(Sc, sat(d)); Rc = setLum(Rc, lum8(d)); Rc = mix(Sc, Rc, Da); }\n"
		"  } else if (uBlendMode == 16) {\n"
		"    if (Da > 0.0) {\n"
		"      vec3 d = min(ONE, Dc / Da); float s = sat(Sc); float n = min(d.r, min(d.g, d.b)); float x = max(d.r, max(d.g, d.b));\n"
		"      Rc = vec3(0.0); if (x > n) Rc = (d - vec3(n)) * (s / (x - n)); Rc = setLum(Rc, lum8(d)); Rc = mix(Sc, Rc, Da);\n"
		"    }\n"
		"  } else if (uBlendMode == 17) {\n"
		"    if (Da > 0.0) { vec3 d = min(ONE, Dc / Da); Rc = setLum(Sc, lum8(d)); Rc = mix(Sc, Rc, Da); }\n"
		"  } else if (uBlendMode == 18) {\n"
		"    if (Da > 0.0) { vec3 d = min(ONE, Dc / Da); Rc = setLum(d, lum8(Sc)); Rc = mix(Sc, Rc, Da); }\n"
		"  }\n"
		"  if (src.a <= 0.0) FragColor = dst;\n"
		"  else if (uBlendSource != 0) FragColor = vec4(Rc, 1.0);\n"
		"  else FragColor = mix(dst, vec4(Rc, 1.0), src.a * uBlendOpacity);\n"
		"}\n",
		sHeader
	);
	iRet = xgeShaderCreate(&g_xgeShapeExBlendRenderer.tShader, sVertex, sFragment);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	g_xgeShapeExBlendRenderer.bInitialized = 1;
	return XGE_OK;
}

static xge_shape_ex_matrix_t __xgeShapeExScreenParent(xge_shape_ex_matrix_t tParent, int bScreenSpace)
{
	xge_shape_ex_matrix_t tScreen;
	xge_vec2_t tP0;
	xge_vec2_t tPX;
	xge_vec2_t tPY;

	if ( bScreenSpace ) {
		return tParent;
	}
	tP0 = xgeWorldToScreen(__xgeShapeExMatrixPoint(tParent, (xge_vec2_t){0.0f, 0.0f}));
	tPX = xgeWorldToScreen(__xgeShapeExMatrixPoint(tParent, (xge_vec2_t){1.0f, 0.0f}));
	tPY = xgeWorldToScreen(__xgeShapeExMatrixPoint(tParent, (xge_vec2_t){0.0f, 1.0f}));
	tScreen.fA = tPX.fX - tP0.fX;
	tScreen.fB = tPX.fY - tP0.fY;
	tScreen.fC = tPY.fX - tP0.fX;
	tScreen.fD = tPY.fY - tP0.fY;
	tScreen.fE = tP0.fX;
	tScreen.fF = tP0.fY;
	return tScreen;
}

static int __xgeShapeExCompositeBlend(
	int iBlend,
	int iSource,
	float fOpacity,
	xge_rect_t tBounds,
	xge_shape_ex_blend_draw_proc pDraw,
	void* pUser,
	xge_shape_ex_matrix_t tScreenParent
)
{
	xge_render_target_t tSource;
	xge_render_target_t tDestination;
	xge_pass_t tPass;
	xge_material_t tMaterial;
	xge_draw_t tDraw;
	xge_rect_t tOldClip;
	xge_rect_t tFrame;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;
	int bOldClip;
	int iWidth;
	int iHeight;
	int iRet;

	if ( ((iBlend != XGE_BLEND_ALPHA) && !__xgeShapeExBlendNeedsComposite(iBlend)) ||
	     ((iSource != XGE_SHAPE_EX_BLEND_SOURCE_SCENE) &&
	      (iSource != XGE_SHAPE_EX_BLEND_SOURCE_SHAPE) &&
	      (iSource != XGE_SHAPE_EX_BLEND_SOURCE_GRADIENT)) ||
	     !__xgeShapeExFloatFinite(fOpacity) ||
	     (pDraw == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( fOpacity <= 0.0f ) return XGE_OK;
	if ( fOpacity > 1.0f ) fOpacity = 1.0f;
	tFrame = (xge_rect_t){0.0f, 0.0f, (float)g_xge.iWidth, (float)g_xge.iHeight};
	tBounds = __xgeShapeExRectIntersect(tBounds, tFrame);
	bOldClip = g_xge.bClipEnabled;
	tOldClip = xgeClipGet();
	if ( bOldClip ) {
		tBounds = __xgeShapeExRectIntersect(tBounds, tOldClip);
	}
	if ( (tBounds.fW <= 0.0f) || (tBounds.fH <= 0.0f) ) {
		return XGE_OK;
	}
	fLeft = floorf(tBounds.fX);
	fTop = floorf(tBounds.fY);
	fRight = ceilf(tBounds.fX + tBounds.fW);
	fBottom = ceilf(tBounds.fY + tBounds.fH);
	if ( fLeft < 0.0f ) fLeft = 0.0f;
	if ( fTop < 0.0f ) fTop = 0.0f;
	if ( fRight > (float)g_xge.iWidth ) fRight = (float)g_xge.iWidth;
	if ( fBottom > (float)g_xge.iHeight ) fBottom = (float)g_xge.iHeight;
	iWidth = (int)(fRight - fLeft);
	iHeight = (int)(fBottom - fTop);
	if ( (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_OK;
	}
	if ( glGetIntegerv != NULL ) {
		GLint iMaxTextureSize = 0;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &iMaxTextureSize);
		if ( (iMaxTextureSize > 0) && ((iWidth > iMaxTextureSize) || (iHeight > iMaxTextureSize)) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	memset(&tSource, 0, sizeof(tSource));
	memset(&tDestination, 0, sizeof(tDestination));
	memset(&tPass, 0, sizeof(tPass));
	iRet = __xgeShapeAutoBatchFlush();
	if ( iRet == XGE_OK ) {
		iRet = xgeRenderTargetCreate(&tSource, iWidth, iHeight);
	}
	if ( iRet == XGE_OK ) {
		iRet = xgeRenderTargetCreate(&tDestination, iWidth, iHeight);
	}
	if ( iRet == XGE_OK ) {
		iRet = xgeRenderTargetCaptureCurrent(&tDestination, (int)fLeft, (int)fTop);
	}
	if ( iRet == XGE_OK ) {
		xgeClipClear();
		xgePassInit(&tPass, &tSource, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(0, 0, 0, 0));
		iRet = xgePassBegin(&tPass);
	}
	if ( iRet == XGE_OK ) {
		xge_shape_ex_matrix_t tOffset = __xgeShapeExMatrixIdentity();
		xge_shape_ex_matrix_t tLocalParent;

		tOffset.fE = -fLeft;
		tOffset.fF = -fTop;
		tLocalParent = __xgeShapeExMatrixMul(tOffset, tScreenParent);
		iRet = pDraw(pUser, tLocalParent);
		if ( iRet == XGE_OK ) {
			iRet = __xgeShapeAutoBatchFlush();
		}
	}
	if ( tPass.bActive ) {
		int iEndRet = xgePassEnd(&tPass);
		if ( iRet == XGE_OK ) iRet = iEndRet;
	}
	if ( bOldClip ) xgeClipSet(tOldClip);
	else xgeClipClear();
	if ( iRet == XGE_OK ) {
		iRet = __xgeShapeExBlendRendererEnsure();
	}
	if ( iRet == XGE_OK ) {
		iRet = xgeShaderUniform1i(&g_xgeShapeExBlendRenderer.tShader, "uBlendMode", iBlend);
	}
	if ( iRet == XGE_OK ) {
		iRet = xgeShaderUniform1i(&g_xgeShapeExBlendRenderer.tShader, "uBlendSource", iSource);
	}
	if ( iRet == XGE_OK ) {
		iRet = xgeShaderUniform1f(&g_xgeShapeExBlendRenderer.tShader, "uBlendOpacity", fOpacity);
	}
	if ( iRet == XGE_OK ) {
		xgeMaterialInit(&tMaterial);
		xgeMaterialSetShader(&tMaterial, &g_xgeShapeExBlendRenderer.tShader);
		xgeMaterialSetTexture(&tMaterial, xgeRenderTargetTexture(&tSource));
		xgeMaterialSetTexture2(&tMaterial, xgeRenderTargetTexture(&tDestination));
		xgeMaterialSetBlend(&tMaterial, XGE_BLEND_NONE);
		memset(&tDraw, 0, sizeof(tDraw));
		tDraw.pTexture = xgeRenderTargetTexture(&tSource);
		tDraw.tSrc = (xge_rect_t){0.0f, 0.0f, (float)iWidth, (float)iHeight};
		tDraw.tDst = (xge_rect_t){fLeft, fTop, (float)iWidth, (float)iHeight};
		tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		tDraw.iFlags = XGE_DRAW_SCREEN_SPACE | XGE_DRAW_FLIP_Y;
		xgeMaterialDraw(&tMaterial, &tDraw);
		xgeMaterialFree(&tMaterial);
		iRet = xgeFlush();
	}
	xgeRenderTargetFree(&tDestination);
	xgeRenderTargetFree(&tSource);
	return iRet;
}

typedef struct xge_shape_ex_blend_shape_context_t {
	xge_shape_ex pShape;
	float fTolerance;
	float fParentOpacity;
	int iPaint;
} xge_shape_ex_blend_shape_context_t;

static int __xgeShapeExBlendShapeDraw(void* pUser, xge_shape_ex_matrix_t tLocalParent)
{
	xge_shape_ex_blend_shape_context_t* pContext = (xge_shape_ex_blend_shape_context_t*)pUser;
	xge_shape_ex_t tPaint;

	if ( pContext->iPaint == XGE_SHAPE_EX_BLEND_PAINT_ALL ) {
		return __xgeShapeExDrawInternal(
			pContext->pShape, pContext->fTolerance, 1, tLocalParent, pContext->fParentOpacity, 1
		);
	}
	tPaint = *pContext->pShape;
	if ( pContext->iPaint == XGE_SHAPE_EX_BLEND_PAINT_FILL ) {
		tPaint.fStrokeWidth = 0.0f;
		tPaint.iStrokeColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	} else {
		tPaint.iFillType = XGE_SHAPE_EX_FILL_SOLID;
		tPaint.iFillColor = XGE_COLOR_RGBA(0, 0, 0, 0);
		tPaint.pFillStops = NULL;
		tPaint.iFillStopCount = 0;
		tPaint.iFillStopCapacity = 0;
	}
	return __xgeShapeExDrawInternal(
		&tPaint, pContext->fTolerance, 1, tLocalParent, pContext->fParentOpacity, 1
	);
}

static int __xgeShapeExDrawDispatch(xge_shape_ex pShape, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tParent, float fParentOpacity, int bSuppressOwnBlend)
{
	xge_shape_ex_blend_shape_context_t tContext;
	xge_shape_ex_matrix_t tScreenParent;
	xge_rect_t tBounds;
	int iRet;

	if ( !__xgeShapeExValid(pShape) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !bSuppressOwnBlend && pShape->bBlendSet && __xgeShapeExBlendNeedsComposite(pShape->iBlend) ) {
		int bFillVisible;
		int bStrokeVisible;
		int iFirstPaint;
		int iSecondPaint;

		tScreenParent = __xgeShapeExScreenParent(tParent, bScreenSpace);
		iRet = __xgeShapeExGetBoundsInternal(pShape, fTolerance, tScreenParent, &tBounds);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		tContext.pShape = pShape;
		tContext.fTolerance = fTolerance;
		tContext.fParentOpacity = fParentOpacity;
		bFillVisible = __xgeShapeExFillVisible(pShape, pShape->iFillColor);
		bStrokeVisible = (pShape->fStrokeWidth > 0.0f) && __xgeShapeExStrokeVisible(pShape, pShape->iStrokeColor);
		if ( bFillVisible && bStrokeVisible ) {
			iFirstPaint = pShape->bStrokeFirst ? XGE_SHAPE_EX_BLEND_PAINT_STROKE : XGE_SHAPE_EX_BLEND_PAINT_FILL;
			iSecondPaint = pShape->bStrokeFirst ? XGE_SHAPE_EX_BLEND_PAINT_FILL : XGE_SHAPE_EX_BLEND_PAINT_STROKE;
			tContext.iPaint = iFirstPaint;
			iRet = __xgeShapeExCompositeBlend(
				pShape->iBlend, __xgeShapeExBlendShapeSource(pShape, iFirstPaint), 1.0f, tBounds,
				__xgeShapeExBlendShapeDraw, &tContext, tScreenParent
			);
			if ( iRet != XGE_OK ) return iRet;
			tContext.iPaint = iSecondPaint;
			return __xgeShapeExCompositeBlend(
				pShape->iBlend, __xgeShapeExBlendShapeSource(pShape, iSecondPaint), 1.0f, tBounds,
				__xgeShapeExBlendShapeDraw, &tContext, tScreenParent
			);
		}
		tContext.iPaint = bStrokeVisible ? XGE_SHAPE_EX_BLEND_PAINT_STROKE : XGE_SHAPE_EX_BLEND_PAINT_FILL;
		return __xgeShapeExCompositeBlend(
			pShape->iBlend, __xgeShapeExBlendShapeSource(pShape, tContext.iPaint), 1.0f, tBounds,
			__xgeShapeExBlendShapeDraw, &tContext, tScreenParent
		);
	}
	return __xgeShapeExDrawInternal(
		pShape, fTolerance, bScreenSpace, tParent, fParentOpacity, bSuppressOwnBlend
	);
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
	return __xgeShapeExDrawDispatch(pShape, fTolerance, 0, tParent, fParentOpacity, 0);
}

int xgeShapeExDrawPxEx(xge_shape_ex pShape, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, float fParentOpacity)
{
	xge_shape_ex_matrix_t tParent;

	tParent = pParentMatrix != NULL ? *pParentMatrix : __xgeShapeExMatrixIdentity();
	return __xgeShapeExDrawDispatch(pShape, fTolerance, 1, tParent, fParentOpacity, 0);
}

static void __xgeShapeExSceneClipShapesClearInternal(xge_shape_ex_scene pScene)
{
	int i;

	if ( !__xgeShapeExSceneValid(pScene) ) {
		return;
	}
	for ( i = 0; i < pScene->iClipShapeCount; i++ ) {
		xgeShapeExDestroy(pScene->pClipShapes[i]);
	}
	pScene->iClipShapeCount = 0;
}

static int __xgeShapeExSceneReserveClipShapes(xge_shape_ex_scene pScene, int iNeeded)
{
	xge_shape_ex* pShapes;
	int* pModes;
	int iCapacity;

	if ( !__xgeShapeExSceneValid(pScene) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pScene->iClipShapeCapacity ) {
		return XGE_OK;
	}
	iCapacity = pScene->iClipShapeCapacity > 0 ? pScene->iClipShapeCapacity : 4;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pShapes = (xge_shape_ex*)xrtMalloc((size_t)iCapacity * sizeof(*pShapes));
	if ( pShapes == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pModes = (int*)xrtMalloc((size_t)iCapacity * sizeof(*pModes));
	if ( pModes == NULL ) {
		xrtFree(pShapes);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( pScene->iClipShapeCount > 0 ) {
		memcpy(pShapes, pScene->pClipShapes, (size_t)pScene->iClipShapeCount * sizeof(*pShapes));
		if ( pScene->pClipShapeModes != NULL ) {
			memcpy(pModes, pScene->pClipShapeModes, (size_t)pScene->iClipShapeCount * sizeof(*pModes));
		} else {
			int i;

			for ( i = 0; i < pScene->iClipShapeCount; i++ ) {
				pModes[i] = XGE_SHAPE_EX_CLIP_INTERSECT;
			}
		}
	}
	xrtFree(pScene->pClipShapes);
	xrtFree(pScene->pClipShapeModes);
	pScene->pClipShapes = pShapes;
	pScene->pClipShapeModes = pModes;
	pScene->iClipShapeCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeShapeExSceneClipShapeAddRefEx(xge_shape_ex_scene pScene, xge_shape_ex pClipShape, int iMode)
{
	int iRet;

	if ( !__xgeShapeExSceneValid(pScene) || !__xgeShapeExValid(pClipShape) || !__xgeShapeExClipModeValid(iMode) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeShapeExSceneReserveClipShapes(pScene, pScene->iClipShapeCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeShapeExAddRef(pClipShape);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pScene->pClipShapes[pScene->iClipShapeCount] = pClipShape;
	pScene->pClipShapeModes[pScene->iClipShapeCount] = iMode;
	pScene->iClipShapeCount++;
	return XGE_OK;
}

static int __xgeShapeExSceneClipShapeAddRef(xge_shape_ex_scene pScene, xge_shape_ex pClipShape)
{
	return __xgeShapeExSceneClipShapeAddRefEx(pScene, pClipShape, XGE_SHAPE_EX_CLIP_INTERSECT);
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
	pScene->iBlend = XGE_BLEND_ALPHA;
	pScene->bBlendSet = 0;
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
	pClone->iBlend = pScene->iBlend;
	pClone->bBlendSet = pScene->bBlendSet;
	pClone->bClipRect = pScene->bClipRect;
	pClone->tClipRect = pScene->tClipRect;
	pClone->tTransform = pScene->tTransform;
	for ( i = 0; i < pScene->iClipShapeCount; i++ ) {
		xge_shape_ex pClipClone;

		pClipClone = NULL;
		iRet = xgeShapeExClone(pScene->pClipShapes[i], &pClipClone);
		if ( iRet == XGE_OK ) {
			int iMode = pScene->pClipShapeModes != NULL ? pScene->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

			iRet = __xgeShapeExSceneClipShapeAddRefEx(pClone, pClipClone, iMode);
			xgeShapeExDestroy(pClipClone);
		}
		if ( iRet != XGE_OK ) {
			xgeShapeExSceneDestroy(pClone);
			return iRet;
		}
	}
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
	__xgeShapeExSceneClipShapesClearInternal(pScene);
	pScene->iMagic = 0;
	xrtFree(pScene->pShapes);
	xrtFree(pScene->pClipShapes);
	xrtFree(pScene->pClipShapeModes);
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
	if ( !__xgeShapeExSceneValid(pScene) || !__xgeShapeExMatrixFinite(pMatrix) ) return XGE_ERROR_INVALID_ARGUMENT;
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
	int iRet;

	if ( !__xgeShapeExSceneValid(pScene) ) return XGE_ERROR_INVALID_ARGUMENT;
	iRet = xgeShapeExMatrixTranslate(&tLocal, fTX, fTY);
	if ( iRet != XGE_OK ) return iRet;
	return __xgeShapeExTransformConcat(&pScene->tTransform, &tLocal);
}

int xgeShapeExSceneTransformScale(xge_shape_ex_scene pScene, float fSX, float fSY)
{
	xge_shape_ex_matrix_t tLocal;
	int iRet;

	if ( !__xgeShapeExSceneValid(pScene) ) return XGE_ERROR_INVALID_ARGUMENT;
	iRet = xgeShapeExMatrixScale(&tLocal, fSX, fSY);
	if ( iRet != XGE_OK ) return iRet;
	return __xgeShapeExTransformConcat(&pScene->tTransform, &tLocal);
}

int xgeShapeExSceneTransformRotate(xge_shape_ex_scene pScene, float fRadians)
{
	xge_shape_ex_matrix_t tLocal;
	int iRet;

	if ( !__xgeShapeExSceneValid(pScene) ) return XGE_ERROR_INVALID_ARGUMENT;
	iRet = xgeShapeExMatrixRotate(&tLocal, fRadians);
	if ( iRet != XGE_OK ) return iRet;
	return __xgeShapeExTransformConcat(&pScene->tTransform, &tLocal);
}

int xgeShapeExSceneTransformSkew(xge_shape_ex_scene pScene, float fXRadians, float fYRadians)
{
	xge_shape_ex_matrix_t tLocal;
	int iRet;

	if ( !__xgeShapeExSceneValid(pScene) ) return XGE_ERROR_INVALID_ARGUMENT;
	iRet = xgeShapeExMatrixSkew(&tLocal, fXRadians, fYRadians);
	if ( iRet != XGE_OK ) return iRet;
	return __xgeShapeExTransformConcat(&pScene->tTransform, &tLocal);
}

int xgeShapeExSceneOpacity(xge_shape_ex_scene pScene, float fOpacity)
{
	if ( !__xgeShapeExSceneValid(pScene) || !__xgeShapeExFloatFinite(fOpacity) ) return XGE_ERROR_INVALID_ARGUMENT;
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

int xgeShapeExSceneBlend(xge_shape_ex_scene pScene, int iBlend)
{
	if ( !__xgeShapeExSceneValid(pScene) || !__xgeShapeExBlendValid(iBlend) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pScene->iBlend = iBlend;
	pScene->bBlendSet = 1;
	return XGE_OK;
}

int xgeShapeExSceneBlendClear(xge_shape_ex_scene pScene)
{
	if ( !__xgeShapeExSceneValid(pScene) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pScene->iBlend = XGE_BLEND_ALPHA;
	pScene->bBlendSet = 0;
	return XGE_OK;
}

int xgeShapeExSceneBlendGet(xge_shape_ex_scene pScene, int* pBlend, int* pBlendSet)
{
	if ( !__xgeShapeExSceneValid(pScene) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pBlend != NULL ) {
		*pBlend = pScene->iBlend;
	}
	if ( pBlendSet != NULL ) {
		*pBlendSet = pScene->bBlendSet;
	}
	return XGE_OK;
}

int xgeShapeExSceneClipRectSet(xge_shape_ex_scene pScene, xge_rect_t tRect)
{
	if ( !__xgeShapeExSceneValid(pScene) || !__xgeShapeExRectFinite(tRect) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( tRect.fW < 0.0f ) tRect.fW = 0.0f;
	if ( tRect.fH < 0.0f ) tRect.fH = 0.0f;
	pScene->tClipRect = tRect;
	pScene->bClipRect = 1;
	return XGE_OK;
}

int xgeShapeExSceneClipRectGet(xge_shape_ex_scene pScene, xge_rect_t* pRect, int* pEnabled)
{
	if ( !__xgeShapeExSceneValid(pScene) || ((pRect == NULL) && (pEnabled == NULL)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pRect != NULL ) {
		if ( pScene->bClipRect ) {
			*pRect = pScene->tClipRect;
		} else {
			memset(pRect, 0, sizeof(*pRect));
		}
	}
	if ( pEnabled != NULL ) {
		*pEnabled = pScene->bClipRect;
	}
	return XGE_OK;
}

int xgeShapeExSceneClipShapeAdd(xge_shape_ex_scene pScene, xge_shape_ex pClipShape)
{
	return __xgeShapeExSceneClipShapeAddRef(pScene, pClipShape);
}

int xgeShapeExSceneClipShapeAddEx(xge_shape_ex_scene pScene, xge_shape_ex pClipShape, int iMode)
{
	return __xgeShapeExSceneClipShapeAddRefEx(pScene, pClipShape, iMode);
}

int xgeShapeExSceneClipShapeGetCount(xge_shape_ex_scene pScene, int* pCount)
{
	if ( !__xgeShapeExSceneValid(pScene) || (pCount == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pCount = pScene->iClipShapeCount;
	return XGE_OK;
}

int xgeShapeExSceneClipShapeGetAt(xge_shape_ex_scene pScene, int iIndex, xge_shape_ex* ppClipShape)
{
	if ( !__xgeShapeExSceneValid(pScene) || (ppClipShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iIndex < 0) || (iIndex >= pScene->iClipShapeCount) ) {
		*ppClipShape = NULL;
		return XGE_ERROR_NOT_FOUND;
	}
	*ppClipShape = pScene->pClipShapes[iIndex];
	return XGE_OK;
}

int xgeShapeExSceneClipShapeGetAtEx(xge_shape_ex_scene pScene, int iIndex, xge_shape_ex* ppClipShape, int* pMode)
{
	if ( !__xgeShapeExSceneValid(pScene) || ((ppClipShape == NULL) && (pMode == NULL)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iIndex < 0) || (iIndex >= pScene->iClipShapeCount) ) {
		if ( ppClipShape != NULL ) {
			*ppClipShape = NULL;
		}
		if ( pMode != NULL ) {
			*pMode = XGE_SHAPE_EX_CLIP_INTERSECT;
		}
		return XGE_ERROR_NOT_FOUND;
	}
	if ( ppClipShape != NULL ) {
		*ppClipShape = pScene->pClipShapes[iIndex];
	}
	if ( pMode != NULL ) {
		*pMode = pScene->pClipShapeModes != NULL ? pScene->pClipShapeModes[iIndex] : XGE_SHAPE_EX_CLIP_INTERSECT;
	}
	return XGE_OK;
}

int xgeShapeExSceneClipShapeClear(xge_shape_ex_scene pScene)
{
	if ( !__xgeShapeExSceneValid(pScene) ) return XGE_ERROR_INVALID_ARGUMENT;
	__xgeShapeExSceneClipShapesClearInternal(pScene);
	return XGE_OK;
}

int xgeShapeExSceneClipClear(xge_shape_ex_scene pScene)
{
	if ( !__xgeShapeExSceneValid(pScene) ) return XGE_ERROR_INVALID_ARGUMENT;
	memset(&pScene->tClipRect, 0, sizeof(pScene->tClipRect));
	pScene->bClipRect = 0;
	__xgeShapeExSceneClipShapesClearInternal(pScene);
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
	if ( pScene->bClipRect ) {
		tOut = __xgeShapeExRectIntersect(tOut, __xgeShapeExMatrixRectBounds(tMatrix, pScene->tClipRect));
	}
	if ( pScene->iClipShapeCount > 0 ) {
		xge_rect_t tClipUnion;
		int bHasClipBounds;
		int bHasIncludeClip;

		memset(&tClipUnion, 0, sizeof(tClipUnion));
		bHasClipBounds = 0;
		bHasIncludeClip = 0;
		for ( i = 0; i < pScene->iClipShapeCount; i++ ) {
			xge_rect_t tClipBounds;
			int iRet;
			int iMode = pScene->pClipShapeModes != NULL ? pScene->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

			if ( iMode == XGE_SHAPE_EX_CLIP_SUBTRACT ) {
				continue;
			}
			bHasIncludeClip = 1;

			iRet = __xgeShapeExGetBoundsInternal(pScene->pClipShapes[i], fTolerance, tMatrix, &tClipBounds);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			if ( (tClipBounds.fW > 0.0f) && (tClipBounds.fH > 0.0f) ) {
				if ( !bHasClipBounds ) {
					tClipUnion = tClipBounds;
					bHasClipBounds = 1;
				} else {
					tClipUnion = __xgeShapeExRectUnion(tClipUnion, tClipBounds);
				}
			}
		}
		if ( bHasIncludeClip ) {
			if ( bHasClipBounds ) {
				tOut = __xgeShapeExRectIntersect(tOut, tClipUnion);
			} else {
				memset(&tOut, 0, sizeof(tOut));
			}
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
	if ( pScene->bClipRect ) {
		tOut = __xgeShapeExRectIntersect(tOut, pScene->tClipRect);
	}
	if ( pScene->iClipShapeCount > 0 ) {
		xge_rect_t tClipUnion;
		int bHasClipBounds;
		int bHasIncludeClip;

		memset(&tClipUnion, 0, sizeof(tClipUnion));
		bHasClipBounds = 0;
		bHasIncludeClip = 0;
		for ( i = 0; i < pScene->iClipShapeCount; i++ ) {
			xge_rect_t tClipBounds;
			int iRet;
			int iMode = pScene->pClipShapeModes != NULL ? pScene->pClipShapeModes[i] : XGE_SHAPE_EX_CLIP_INTERSECT;

			if ( iMode == XGE_SHAPE_EX_CLIP_SUBTRACT ) {
				continue;
			}
			bHasIncludeClip = 1;

			iRet = __xgeShapeExGetBoundsInternal(pScene->pClipShapes[i], fTolerance, __xgeShapeExMatrixIdentity(), &tClipBounds);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			if ( (tClipBounds.fW > 0.0f) && (tClipBounds.fH > 0.0f) ) {
				if ( !bHasClipBounds ) {
					tClipUnion = tClipBounds;
					bHasClipBounds = 1;
				} else {
					tClipUnion = __xgeShapeExRectUnion(tClipUnion, tClipBounds);
				}
			}
		}
		if ( bHasIncludeClip ) {
			if ( bHasClipBounds ) {
				tOut = __xgeShapeExRectIntersect(tOut, tClipUnion);
			} else {
				memset(&tOut, 0, sizeof(tOut));
			}
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

int xgeShapeExSceneContainsPoint(xge_shape_ex_scene pScene, float fX, float fY, float fTolerance, int* pContains)
{
	return xgeShapeExSceneContainsPointEx(pScene, fX, fY, fTolerance, NULL, pContains);
}

int xgeShapeExSceneContainsPointEx(xge_shape_ex_scene pScene, float fX, float fY, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, int* pContains)
{
	xge_shape_ex pHitShape;
	int iRet;

	if ( pContains == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pHitShape = NULL;
	iRet = xgeShapeExSceneHitTestEx(pScene, fX, fY, fTolerance, pParentMatrix, &pHitShape);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	*pContains = pHitShape != NULL;
	return XGE_OK;
}

int xgeShapeExSceneHitTest(xge_shape_ex_scene pScene, float fX, float fY, float fTolerance, xge_shape_ex* ppShape)
{
	return xgeShapeExSceneHitTestEx(pScene, fX, fY, fTolerance, NULL, ppShape);
}

int xgeShapeExSceneHitTestEx(xge_shape_ex_scene pScene, float fX, float fY, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, xge_shape_ex* ppShape)
{
	xge_shape_ex_matrix_t tParent;
	xge_vec2_t tPoint;

	if ( !__xgeShapeExSceneValid(pScene) || (ppShape == NULL) || !__xgeShapeExCoordsFinite(fX, fY) || !__xgeShapeExFloatFinite(fTolerance) ||
	     ((pParentMatrix != NULL) && !__xgeShapeExMatrixFinite(pParentMatrix)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tParent = (pParentMatrix != NULL) ? *pParentMatrix : __xgeShapeExMatrixIdentity();
	tPoint.fX = fX;
	tPoint.fY = fY;
	return __xgeShapeExSceneHitTestInternal(pScene, tPoint, fTolerance, tParent, ppShape);
}

typedef struct xge_shape_ex_blend_scene_context_t {
	xge_shape_ex_scene pScene;
	float fTolerance;
	float fParentOpacity;
} xge_shape_ex_blend_scene_context_t;

static int __xgeShapeExBlendSceneDraw(void* pUser, xge_shape_ex_matrix_t tLocalParent)
{
	xge_shape_ex_blend_scene_context_t* pContext = (xge_shape_ex_blend_scene_context_t*)pUser;
	return __xgeShapeExSceneDrawInternal(
		pContext->pScene, pContext->fTolerance, 1, tLocalParent, pContext->fParentOpacity, 1
	);
}

static int __xgeShapeExSceneDrawDispatch(xge_shape_ex_scene pScene, float fTolerance, int bScreenSpace, xge_shape_ex_matrix_t tParent, float fParentOpacity)
{
	xge_shape_ex_blend_scene_context_t tContext;
	xge_shape_ex_matrix_t tScreenParent;
	xge_rect_t tBounds;
	int iRet;

	if ( !__xgeShapeExSceneValid(pScene) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pScene->bBlendSet && __xgeShapeExBlendNeedsComposite(pScene->iBlend) ) {
		tScreenParent = __xgeShapeExScreenParent(tParent, bScreenSpace);
		iRet = xgeShapeExSceneGetBounds(pScene, fTolerance, &tBounds);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		tBounds = __xgeShapeExMatrixRectBounds(tScreenParent, tBounds);
		tContext.pScene = pScene;
		tContext.fTolerance = fTolerance;
		tContext.fParentOpacity = fParentOpacity;
		return __xgeShapeExCompositeBlend(
			pScene->iBlend, XGE_SHAPE_EX_BLEND_SOURCE_SCENE, 1.0f, tBounds,
			__xgeShapeExBlendSceneDraw, &tContext, tScreenParent
		);
	}
	return __xgeShapeExSceneDrawInternal(
		pScene, fTolerance, bScreenSpace, tParent, fParentOpacity, 0
	);
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
	return __xgeShapeExSceneDrawDispatch(pScene, fTolerance, 0, tParent, fParentOpacity);
}

int xgeShapeExSceneDrawPxEx(xge_shape_ex_scene pScene, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, float fParentOpacity)
{
	xge_shape_ex_matrix_t tParent;

	tParent = pParentMatrix != NULL ? *pParentMatrix : __xgeShapeExMatrixIdentity();
	return __xgeShapeExSceneDrawDispatch(pScene, fTolerance, 1, tParent, fParentOpacity);
}
