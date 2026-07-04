#define XGE_PATH_MAGIC 0x58475041u
#define XGE_SVG_MAGIC 0x58475356u
#define XGE_PATH_ROUND_CAP_SEGMENTS 8
#define XGE_PATH_ROUND_JOIN_SEGMENTS 8
#define XGE_PATH_EPSILON 0.00001f
#define XGE_SVG_CONTEXT_STACK_MAX 32
#define XGE_SVG_DASH_INLINE_MAX 16
#define XGE_SVG_ID_MAX 64
#define XGE_SVG_GRADIENT_OBJECT_BOUNDING_BOX 0
#define XGE_SVG_GRADIENT_USER_SPACE 1
#define XGE_SVG_SPREAD_PAD 0
#define XGE_SVG_SPREAD_REPEAT 1
#define XGE_SVG_SPREAD_REFLECT 2
#define XGE_SVG_CLIP_USER_SPACE 0
#define XGE_SVG_CLIP_OBJECT_BOUNDING_BOX 1
#define XGE_SVG_MASK_USER_SPACE 0
#define XGE_SVG_MASK_OBJECT_BOUNDING_BOX 1
#define XGE_SVG_STYLE_SELECTOR_TAG 0
#define XGE_SVG_STYLE_SELECTOR_CLASS 1
#define XGE_SVG_STYLE_SELECTOR_ID 2

#define NANOSVG_ALL_COLOR_KEYWORDS
#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#define malloc(sz) xrtMalloc(sz)
#define realloc(p, sz) xrtRealloc((p), (sz))
#define free(p) xrtFree(p)
#include "lib/nanosvg/nanosvg.h"
#include "lib/nanosvg/nanosvgrast.h"
#undef free
#undef realloc
#undef malloc

struct xge_path_t {
	uint32_t iMagic;
	xge_path_command_t* pCommands;
	int iCommandCount;
	int iCommandCapacity;
};

typedef struct xge_svg_transform_t {
	float fA;
	float fB;
	float fC;
	float fD;
	float fE;
	float fF;
} xge_svg_transform_t;

typedef struct xge_svg_gradient_stop_t {
	float fOffset;
	uint32_t iColor;
} xge_svg_gradient_stop_t;

typedef struct xge_svg_linear_gradient_t {
	char sId[XGE_SVG_ID_MAX];
	char sHrefId[XGE_SVG_ID_MAX];
	int iUnits;
	float fX1;
	float fY1;
	float fX2;
	float fY2;
	int iSpreadMethod;
	xge_svg_transform_t tTransform;
	xge_svg_gradient_stop_t* pStops;
	int iStopCount;
	int iStopCapacity;
} xge_svg_linear_gradient_t;

typedef struct xge_svg_radial_gradient_t {
	char sId[XGE_SVG_ID_MAX];
	char sHrefId[XGE_SVG_ID_MAX];
	int iUnits;
	float fCX;
	float fCY;
	float fR;
	float fFX;
	float fFY;
	int bHasFX;
	int bHasFY;
	int iSpreadMethod;
	xge_svg_transform_t tTransform;
	xge_svg_gradient_stop_t* pStops;
	int iStopCount;
	int iStopCapacity;
} xge_svg_radial_gradient_t;

typedef struct xge_svg_clip_path_t {
	char sId[XGE_SVG_ID_MAX];
	int iUnits;
	xge_rect_t tRect;
	xge_rect_t* pRects;
	int iRectCount;
	int iRectCapacity;
	int bHasRect;
} xge_svg_clip_path_t;

typedef struct xge_svg_mask_rect_t {
	xge_rect_t tRect;
	float fOpacity;
} xge_svg_mask_rect_t;

typedef struct xge_svg_mask_t {
	char sId[XGE_SVG_ID_MAX];
	int iUnits;
	xge_rect_t tRect;
	float fOpacity;
	xge_svg_mask_rect_t* pRects;
	int iRectCount;
	int iRectCapacity;
	int bHasRect;
} xge_svg_mask_t;

typedef struct xge_svg_view_ref_t {
	char sId[XGE_SVG_ID_MAX];
	xge_rect_t tViewBox;
	float fWidth;
	float fHeight;
	int iAspectAlignX;
	int iAspectAlignY;
	int iAspectMode;
	int bHasViewBox;
} xge_svg_view_ref_t;

typedef struct xge_svg_style_rule_t {
	int iSelectorType;
	char sSelector[XGE_SVG_ID_MAX];
	char* sStyle;
} xge_svg_style_rule_t;

typedef struct xge_svg_path_item_t {
	xge_path pPath;
	xge_path_style_t tStyle;
	float* pDashPattern;
	char sId[XGE_SVG_ID_MAX];
	char sUseGroupId[XGE_SVG_ID_MAX];
	char sFillGradientId[XGE_SVG_ID_MAX];
	char sStrokeGradientId[XGE_SVG_ID_MAX];
	char sClipPathId[XGE_SVG_ID_MAX];
	char sMaskId[XGE_SVG_ID_MAX];
	float fFillOpacity;
	float fStrokeOpacity;
	int bHidden;
	int iOrder;
} xge_svg_path_item_t;

struct xge_svg_t {
	uint32_t iMagic;
	int iRefCount;
	xge_rect_t tViewBox;
	int bHasViewBox;
	float fWidth;
	float fHeight;
	int iAspectAlignX;
	int iAspectAlignY;
	int iAspectMode;
	xge_svg_path_item_t* pPaths;
	int iPathCount;
	int iPathCapacity;
	xge_svg_linear_gradient_t* pLinearGradients;
	int iLinearGradientCount;
	int iLinearGradientCapacity;
	xge_svg_radial_gradient_t* pRadialGradients;
	int iRadialGradientCount;
	int iRadialGradientCapacity;
	xge_svg_clip_path_t* pClipPaths;
	int iClipPathCount;
	int iClipPathCapacity;
	xge_svg_mask_t* pMasks;
	int iMaskCount;
	int iMaskCapacity;
	xge_svg_view_ref_t* pViewRefs;
	int iViewRefCount;
	int iViewRefCapacity;
	xge_svg_style_rule_t* pStyleRules;
	int iStyleRuleCount;
	int iStyleRuleCapacity;
};

typedef struct xge_svg_cache_entry_t {
	char* sURI;
	xge_svg pSvg;
	struct xge_svg_cache_entry_t* pNext;
} xge_svg_cache_entry_t;

typedef struct xge_svg_parse_context_t {
	xge_path_style_t tStyle;
	float arrDash[XGE_SVG_DASH_INLINE_MAX];
	int iDashCount;
	xge_svg_transform_t tTransform;
	char sFillGradientId[XGE_SVG_ID_MAX];
	char sStrokeGradientId[XGE_SVG_ID_MAX];
	char sClipPathId[XGE_SVG_ID_MAX];
	char sMaskId[XGE_SVG_ID_MAX];
	char sElementId[XGE_SVG_ID_MAX];
	char sUseGroupId[XGE_SVG_ID_MAX];
	float fFillOpacity;
	float fStrokeOpacity;
	float fUseWidth;
	float fUseHeight;
	int bHidden;
	int bUseWidth;
	int bUseHeight;
	int iOrder;
} xge_svg_parse_context_t;

typedef struct xge_svg_pending_use_t {
	char sRefId[XGE_SVG_ID_MAX];
	xge_svg_parse_context_t tContext;
} xge_svg_pending_use_t;

static xge_svg_cache_entry_t* g_xgeSvgCacheHead;

static int __xgeSvgAttrGet(const char* pTag, const char* pTagEnd, const char* sName, const char** ppValue, int* pValueLen);

static int __xgePathValid(xge_path pPath)
{
	return (pPath != NULL) && (pPath->iMagic == XGE_PATH_MAGIC);
}

static int __xgeSvgValid(xge_svg pSvg)
{
	return (pSvg != NULL) && (pSvg->iMagic == XGE_SVG_MAGIC);
}

static int __xgePathReserve(xge_path pPath, int iNeeded)
{
	xge_path_command_t* pCommands;
	int iCapacity;

	if ( !__xgePathValid(pPath) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pPath->iCommandCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pPath->iCommandCapacity > 0) ? pPath->iCommandCapacity : 16;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pCommands = (xge_path_command_t*)xrtRealloc(pPath->pCommands, sizeof(*pCommands) * (size_t)iCapacity);
	if ( pCommands == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPath->pCommands = pCommands;
	pPath->iCommandCapacity = iCapacity;
	return XGE_OK;
}

static int __xgePathAddCommand(xge_path pPath, int iCommand, xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC)
{
	xge_path_command_t* pCommand;
	int iRet;

	if ( !__xgePathValid(pPath) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgePathReserve(pPath, pPath->iCommandCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pCommand = &pPath->pCommands[pPath->iCommandCount++];
	memset(pCommand, 0, sizeof(*pCommand));
	pCommand->iCommand = iCommand;
	pCommand->arrPoints[0] = tA;
	pCommand->arrPoints[1] = tB;
	pCommand->arrPoints[2] = tC;
	return XGE_OK;
}

static int __xgePathFlattenAdd(xge_vec2_t* pPoints, int iCapacity, int* pCount, xge_vec2_t tPoint)
{
	if ( pCount == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pPoints != NULL) && (*pCount < iCapacity) ) {
		pPoints[*pCount] = tPoint;
	}
	(*pCount)++;
	return XGE_OK;
}

static int __xgePathFlattenSteps(float fTolerance)
{
	int iSteps;

	if ( fTolerance <= 0.0f ) {
		fTolerance = 1.0f;
	}
	iSteps = (int)(16.0f / fTolerance);
	if ( iSteps < 4 ) iSteps = 4;
	if ( iSteps > 64 ) iSteps = 64;
	return iSteps;
}

static float __xgePathSqrt(float fValue)
{
	float fGuess;
	int i;

	if ( fValue <= 0.0f ) {
		return 0.0f;
	}
	fGuess = (fValue > 1.0f) ? fValue : 1.0f;
	for ( i = 0; i < 8; i++ ) {
		fGuess = 0.5f * (fGuess + (fValue / fGuess));
	}
	return fGuess;
}

int xgePathCreate(xge_path* ppPath)
{
	xge_path pPath;

	if ( ppPath == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppPath = NULL;
	pPath = (xge_path)xrtCalloc(1, sizeof(*pPath));
	if ( pPath == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPath->iMagic = XGE_PATH_MAGIC;
	*ppPath = pPath;
	return XGE_OK;
}

void xgePathDestroy(xge_path pPath)
{
	if ( !__xgePathValid(pPath) ) {
		return;
	}
	if ( pPath->pCommands != NULL ) {
		xrtFree(pPath->pCommands);
	}
	pPath->iMagic = 0;
	xrtFree(pPath);
}

int xgePathClear(xge_path pPath)
{
	if ( !__xgePathValid(pPath) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pPath->iCommandCount = 0;
	return XGE_OK;
}

int xgePathMoveTo(xge_path pPath, float fX, float fY)
{
	return __xgePathAddCommand(pPath, XGE_PATH_CMD_MOVE, (xge_vec2_t){fX, fY}, (xge_vec2_t){0.0f, 0.0f}, (xge_vec2_t){0.0f, 0.0f});
}

int xgePathLineTo(xge_path pPath, float fX, float fY)
{
	return __xgePathAddCommand(pPath, XGE_PATH_CMD_LINE, (xge_vec2_t){fX, fY}, (xge_vec2_t){0.0f, 0.0f}, (xge_vec2_t){0.0f, 0.0f});
}

int xgePathQuadTo(xge_path pPath, float fCX, float fCY, float fX, float fY)
{
	return __xgePathAddCommand(pPath, XGE_PATH_CMD_QUAD, (xge_vec2_t){fCX, fCY}, (xge_vec2_t){fX, fY}, (xge_vec2_t){0.0f, 0.0f});
}

int xgePathCubicTo(xge_path pPath, float fC1X, float fC1Y, float fC2X, float fC2Y, float fX, float fY)
{
	return __xgePathAddCommand(pPath, XGE_PATH_CMD_CUBIC, (xge_vec2_t){fC1X, fC1Y}, (xge_vec2_t){fC2X, fC2Y}, (xge_vec2_t){fX, fY});
}

int xgePathClose(xge_path pPath)
{
	return __xgePathAddCommand(pPath, XGE_PATH_CMD_CLOSE, (xge_vec2_t){0.0f, 0.0f}, (xge_vec2_t){0.0f, 0.0f}, (xge_vec2_t){0.0f, 0.0f});
}

static int __xgePathSvgIsSpace(char c)
{
	return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n') || (c == '\f');
}

static int __xgePathSvgIsCommand(char c)
{
	return (c == 'M') || (c == 'm') || (c == 'L') || (c == 'l') ||
	       (c == 'H') || (c == 'h') || (c == 'V') || (c == 'v') ||
	       (c == 'Q') || (c == 'q') || (c == 'T') || (c == 't') ||
	       (c == 'C') || (c == 'c') || (c == 'S') || (c == 's') ||
	       (c == 'A') || (c == 'a') ||
	       (c == 'Z') || (c == 'z');
}

static int __xgePathSvgCommandLower(char c)
{
	if ( (c >= 'A') && (c <= 'Z') ) {
		return c - 'A' + 'a';
	}
	return c;
}

static int __xgePathSvgCommandRelative(char c)
{
	return (c >= 'a') && (c <= 'z');
}

static void __xgePathSvgSkipSeparators(const char** ppText)
{
	const char* pText;

	pText = *ppText;
	while ( (*pText == ',') || __xgePathSvgIsSpace(*pText) ) {
		pText++;
	}
	*ppText = pText;
}

static int __xgePathSvgHasNumber(const char* sText)
{
	__xgePathSvgSkipSeparators(&sText);
	return (*sText != '\0') && !__xgePathSvgIsCommand(*sText);
}

static int __xgePathSvgReadNumber(const char** ppText, float* pValue)
{
	const char* pText;
	char* pEnd;
	double fValue;

	if ( (ppText == NULL) || (*ppText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	pText = *ppText;
	__xgePathSvgSkipSeparators(&pText);
	if ( (*pText == '\0') || __xgePathSvgIsCommand(*pText) ) {
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

static float __xgePathAbs(float fValue)
{
	return (fValue < 0.0f) ? -fValue : fValue;
}

static xge_vec2_t __xgePathSvgArcMap(float fCX, float fCY, float fRX, float fRY, float fCosPhi, float fSinPhi, float fUX, float fUY)
{
	xge_vec2_t tPoint;

	tPoint.fX = fCX + (fRX * ((fCosPhi * fUX) - (fSinPhi * fUY)));
	tPoint.fY = fCY + (fRY * ((fSinPhi * fUX) + (fCosPhi * fUY)));
	return tPoint;
}

static int __xgePathSvgArcSegmentTo(xge_path pPath, float fCX, float fCY, float fRX, float fRY, float fCosPhi, float fSinPhi, float fTheta0, float fTheta1, xge_vec2_t tEnd, int bUseExactEnd)
{
	float fDelta;
	float fAlpha;
	float fCos0;
	float fSin0;
	float fCos1;
	float fSin1;
	xge_vec2_t tC1;
	xge_vec2_t tC2;
	xge_vec2_t tP3;

	fDelta = fTheta1 - fTheta0;
	fAlpha = (4.0f / 3.0f) * tanf(fDelta * 0.25f);
	fCos0 = cosf(fTheta0);
	fSin0 = sinf(fTheta0);
	fCos1 = cosf(fTheta1);
	fSin1 = sinf(fTheta1);
	tC1 = __xgePathSvgArcMap(fCX, fCY, fRX, fRY, fCosPhi, fSinPhi, fCos0 - (fAlpha * fSin0), fSin0 + (fAlpha * fCos0));
	tC2 = __xgePathSvgArcMap(fCX, fCY, fRX, fRY, fCosPhi, fSinPhi, fCos1 + (fAlpha * fSin1), fSin1 - (fAlpha * fCos1));
	tP3 = bUseExactEnd ? tEnd : __xgePathSvgArcMap(fCX, fCY, fRX, fRY, fCosPhi, fSinPhi, fCos1, fSin1);
	return xgePathCubicTo(pPath, tC1.fX, tC1.fY, tC2.fX, tC2.fY, tP3.fX, tP3.fY);
}

static int __xgePathSvgArcTo(xge_path pPath, xge_vec2_t tCurrent, float fRX, float fRY, float fAxisDegrees, int bLargeArc, int bSweep, float fX, float fY)
{
	float fPhi;
	float fCosPhi;
	float fSinPhi;
	float fDX2;
	float fDY2;
	float fX1P;
	float fY1P;
	float fRX2;
	float fRY2;
	float fX1P2;
	float fY1P2;
	float fLambda;
	float fDenom;
	float fNum;
	float fCoef;
	float fCXP;
	float fCYP;
	float fCX;
	float fCY;
	float fUX;
	float fUY;
	float fVX;
	float fVY;
	float fTheta;
	float fDelta;
	float fStep;
	float fAbsDelta;
	int iSegments;
	int i;
	int iRet;
	xge_vec2_t tEnd;

	if ( !__xgePathValid(pPath) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tEnd = (xge_vec2_t){fX, fY};
	if ( (tCurrent.fX == fX) && (tCurrent.fY == fY) ) {
		return XGE_OK;
	}
	fRX = __xgePathAbs(fRX);
	fRY = __xgePathAbs(fRY);
	if ( (fRX <= 0.000001f) || (fRY <= 0.000001f) ) {
		return xgePathLineTo(pPath, fX, fY);
	}
	fPhi = fAxisDegrees * 0.01745329251994329577f;
	fCosPhi = cosf(fPhi);
	fSinPhi = sinf(fPhi);
	fDX2 = (tCurrent.fX - fX) * 0.5f;
	fDY2 = (tCurrent.fY - fY) * 0.5f;
	fX1P = (fCosPhi * fDX2) + (fSinPhi * fDY2);
	fY1P = (-fSinPhi * fDX2) + (fCosPhi * fDY2);
	fRX2 = fRX * fRX;
	fRY2 = fRY * fRY;
	fX1P2 = fX1P * fX1P;
	fY1P2 = fY1P * fY1P;
	fLambda = (fX1P2 / fRX2) + (fY1P2 / fRY2);
	if ( fLambda > 1.0f ) {
		float fScale = sqrtf(fLambda);
		fRX *= fScale;
		fRY *= fScale;
		fRX2 = fRX * fRX;
		fRY2 = fRY * fRY;
	}
	fDenom = (fRX2 * fY1P2) + (fRY2 * fX1P2);
	if ( fDenom <= 0.000001f ) {
		return xgePathLineTo(pPath, fX, fY);
	}
	fNum = (fRX2 * fRY2) - (fRX2 * fY1P2) - (fRY2 * fX1P2);
	if ( fNum < 0.0f ) {
		fNum = 0.0f;
	}
	fCoef = sqrtf(fNum / fDenom);
	if ( (bLargeArc != 0) == (bSweep != 0) ) {
		fCoef = -fCoef;
	}
	fCXP = fCoef * ((fRX * fY1P) / fRY);
	fCYP = fCoef * (-(fRY * fX1P) / fRX);
	fCX = (fCosPhi * fCXP) - (fSinPhi * fCYP) + ((tCurrent.fX + fX) * 0.5f);
	fCY = (fSinPhi * fCXP) + (fCosPhi * fCYP) + ((tCurrent.fY + fY) * 0.5f);
	fUX = (fX1P - fCXP) / fRX;
	fUY = (fY1P - fCYP) / fRY;
	fVX = (-fX1P - fCXP) / fRX;
	fVY = (-fY1P - fCYP) / fRY;
	fTheta = atan2f(fUY, fUX);
	fDelta = atan2f((fUX * fVY) - (fUY * fVX), (fUX * fVX) + (fUY * fVY));
	if ( (bSweep == 0) && (fDelta > 0.0f) ) {
		fDelta -= 6.28318530717958647692f;
	} else if ( (bSweep != 0) && (fDelta < 0.0f) ) {
		fDelta += 6.28318530717958647692f;
	}
	fAbsDelta = __xgePathAbs(fDelta);
	iSegments = 1;
	while ( (fAbsDelta / (float)iSegments) > 1.57079632679489661923f ) {
		iSegments++;
	}
	fStep = fDelta / (float)iSegments;
	for ( i = 0; i < iSegments; i++ ) {
		iRet = __xgePathSvgArcSegmentTo(pPath, fCX, fCY, fRX, fRY, fCosPhi, fSinPhi, fTheta + (fStep * (float)i), fTheta + (fStep * (float)(i + 1)), tEnd, i == (iSegments - 1));
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

int xgePathParseSvg(xge_path pPath, const char* sPath)
{
	const char* pText;
	xge_vec2_t tCurrent;
	xge_vec2_t tStart;
	xge_vec2_t tLastCubicControl;
	xge_vec2_t tLastQuadControl;
	char cCommand;
	int bHasCommand;
	int iPrevCommand;
	int iRet;

	if ( !__xgePathValid(pPath) || (sPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = xgePathClear(pPath);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pText = sPath;
	memset(&tCurrent, 0, sizeof(tCurrent));
	memset(&tStart, 0, sizeof(tStart));
	memset(&tLastCubicControl, 0, sizeof(tLastCubicControl));
	memset(&tLastQuadControl, 0, sizeof(tLastQuadControl));
	cCommand = 0;
	bHasCommand = 0;
	iPrevCommand = 0;
	for ( ;; ) {
		int bRelative;
		int iCommand;

		__xgePathSvgSkipSeparators(&pText);
		if ( *pText == '\0' ) {
			return XGE_OK;
		}
		if ( __xgePathSvgIsCommand(*pText) ) {
			cCommand = *pText++;
			bHasCommand = 1;
		} else if ( !bHasCommand ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		bRelative = __xgePathSvgCommandRelative(cCommand);
		iCommand = __xgePathSvgCommandLower(cCommand);
		if ( iCommand == 'z' ) {
			iRet = xgePathClose(pPath);
			if ( iRet != XGE_OK ) return iRet;
			tCurrent = tStart;
			tLastCubicControl = tCurrent;
			tLastQuadControl = tCurrent;
			iPrevCommand = 'z';
			bHasCommand = 0;
			continue;
		}
		if ( iCommand == 'm' ) {
			float fX;
			float fY;

			if ( !__xgePathSvgReadNumber(&pText, &fX) || !__xgePathSvgReadNumber(&pText, &fY) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			if ( bRelative ) {
				fX += tCurrent.fX;
				fY += tCurrent.fY;
			}
			iRet = xgePathMoveTo(pPath, fX, fY);
			if ( iRet != XGE_OK ) return iRet;
			tCurrent = (xge_vec2_t){fX, fY};
			tStart = tCurrent;
			tLastCubicControl = tCurrent;
			tLastQuadControl = tCurrent;
			iPrevCommand = 'm';
			while ( __xgePathSvgHasNumber(pText) ) {
				if ( !__xgePathSvgReadNumber(&pText, &fX) || !__xgePathSvgReadNumber(&pText, &fY) ) {
					return XGE_ERROR_INVALID_ARGUMENT;
				}
				if ( bRelative ) {
					fX += tCurrent.fX;
					fY += tCurrent.fY;
				}
				iRet = xgePathLineTo(pPath, fX, fY);
				if ( iRet != XGE_OK ) return iRet;
				tCurrent = (xge_vec2_t){fX, fY};
				tLastCubicControl = tCurrent;
				tLastQuadControl = tCurrent;
				iPrevCommand = 'l';
			}
		} else if ( iCommand == 'l' ) {
			while ( __xgePathSvgHasNumber(pText) ) {
				float fX;
				float fY;

				if ( !__xgePathSvgReadNumber(&pText, &fX) || !__xgePathSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
				if ( bRelative ) {
					fX += tCurrent.fX;
					fY += tCurrent.fY;
				}
				iRet = xgePathLineTo(pPath, fX, fY);
				if ( iRet != XGE_OK ) return iRet;
				tCurrent = (xge_vec2_t){fX, fY};
				tLastCubicControl = tCurrent;
				tLastQuadControl = tCurrent;
				iPrevCommand = 'l';
			}
		} else if ( iCommand == 'h' ) {
			while ( __xgePathSvgHasNumber(pText) ) {
				float fX;

				if ( !__xgePathSvgReadNumber(&pText, &fX) ) return XGE_ERROR_INVALID_ARGUMENT;
				if ( bRelative ) fX += tCurrent.fX;
				iRet = xgePathLineTo(pPath, fX, tCurrent.fY);
				if ( iRet != XGE_OK ) return iRet;
				tCurrent.fX = fX;
				tLastCubicControl = tCurrent;
				tLastQuadControl = tCurrent;
				iPrevCommand = 'h';
			}
		} else if ( iCommand == 'v' ) {
			while ( __xgePathSvgHasNumber(pText) ) {
				float fY;

				if ( !__xgePathSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
				if ( bRelative ) fY += tCurrent.fY;
				iRet = xgePathLineTo(pPath, tCurrent.fX, fY);
				if ( iRet != XGE_OK ) return iRet;
				tCurrent.fY = fY;
				tLastCubicControl = tCurrent;
				tLastQuadControl = tCurrent;
				iPrevCommand = 'v';
			}
		} else if ( iCommand == 'q' ) {
			while ( __xgePathSvgHasNumber(pText) ) {
				float fCX;
				float fCY;
				float fX;
				float fY;

				if ( !__xgePathSvgReadNumber(&pText, &fCX) || !__xgePathSvgReadNumber(&pText, &fCY) ||
				     !__xgePathSvgReadNumber(&pText, &fX) || !__xgePathSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
				if ( bRelative ) {
					fCX += tCurrent.fX;
					fCY += tCurrent.fY;
					fX += tCurrent.fX;
					fY += tCurrent.fY;
				}
				iRet = xgePathQuadTo(pPath, fCX, fCY, fX, fY);
				if ( iRet != XGE_OK ) return iRet;
				tLastQuadControl = (xge_vec2_t){fCX, fCY};
				tLastCubicControl = (xge_vec2_t){fX, fY};
				tCurrent = (xge_vec2_t){fX, fY};
				iPrevCommand = 'q';
			}
		} else if ( iCommand == 't' ) {
			while ( __xgePathSvgHasNumber(pText) ) {
				float fCX;
				float fCY;
				float fX;
				float fY;

				if ( !__xgePathSvgReadNumber(&pText, &fX) || !__xgePathSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
				if ( bRelative ) {
					fX += tCurrent.fX;
					fY += tCurrent.fY;
				}
				if ( (iPrevCommand == 'q') || (iPrevCommand == 't') ) {
					fCX = (2.0f * tCurrent.fX) - tLastQuadControl.fX;
					fCY = (2.0f * tCurrent.fY) - tLastQuadControl.fY;
				} else {
					fCX = tCurrent.fX;
					fCY = tCurrent.fY;
				}
				iRet = xgePathQuadTo(pPath, fCX, fCY, fX, fY);
				if ( iRet != XGE_OK ) return iRet;
				tLastQuadControl = (xge_vec2_t){fCX, fCY};
				tLastCubicControl = (xge_vec2_t){fX, fY};
				tCurrent = (xge_vec2_t){fX, fY};
				iPrevCommand = 't';
			}
		} else if ( iCommand == 'c' ) {
			while ( __xgePathSvgHasNumber(pText) ) {
				float fC1X;
				float fC1Y;
				float fC2X;
				float fC2Y;
				float fX;
				float fY;

				if ( !__xgePathSvgReadNumber(&pText, &fC1X) || !__xgePathSvgReadNumber(&pText, &fC1Y) ||
				     !__xgePathSvgReadNumber(&pText, &fC2X) || !__xgePathSvgReadNumber(&pText, &fC2Y) ||
				     !__xgePathSvgReadNumber(&pText, &fX) || !__xgePathSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
				if ( bRelative ) {
					fC1X += tCurrent.fX;
					fC1Y += tCurrent.fY;
					fC2X += tCurrent.fX;
					fC2Y += tCurrent.fY;
					fX += tCurrent.fX;
					fY += tCurrent.fY;
				}
				iRet = xgePathCubicTo(pPath, fC1X, fC1Y, fC2X, fC2Y, fX, fY);
				if ( iRet != XGE_OK ) return iRet;
				tLastCubicControl = (xge_vec2_t){fC2X, fC2Y};
				tLastQuadControl = (xge_vec2_t){fX, fY};
				tCurrent = (xge_vec2_t){fX, fY};
				iPrevCommand = 'c';
			}
		} else if ( iCommand == 's' ) {
			while ( __xgePathSvgHasNumber(pText) ) {
				float fC1X;
				float fC1Y;
				float fC2X;
				float fC2Y;
				float fX;
				float fY;

				if ( !__xgePathSvgReadNumber(&pText, &fC2X) || !__xgePathSvgReadNumber(&pText, &fC2Y) ||
				     !__xgePathSvgReadNumber(&pText, &fX) || !__xgePathSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
				if ( bRelative ) {
					fC2X += tCurrent.fX;
					fC2Y += tCurrent.fY;
					fX += tCurrent.fX;
					fY += tCurrent.fY;
				}
				if ( (iPrevCommand == 'c') || (iPrevCommand == 's') ) {
					fC1X = (2.0f * tCurrent.fX) - tLastCubicControl.fX;
					fC1Y = (2.0f * tCurrent.fY) - tLastCubicControl.fY;
				} else {
					fC1X = tCurrent.fX;
					fC1Y = tCurrent.fY;
				}
				iRet = xgePathCubicTo(pPath, fC1X, fC1Y, fC2X, fC2Y, fX, fY);
				if ( iRet != XGE_OK ) return iRet;
				tLastCubicControl = (xge_vec2_t){fC2X, fC2Y};
				tLastQuadControl = (xge_vec2_t){fX, fY};
				tCurrent = (xge_vec2_t){fX, fY};
				iPrevCommand = 's';
			}
		} else if ( iCommand == 'a' ) {
			while ( __xgePathSvgHasNumber(pText) ) {
				float fRX;
				float fRY;
				float fAxis;
				float fLargeArc;
				float fSweep;
				float fX;
				float fY;

				if ( !__xgePathSvgReadNumber(&pText, &fRX) || !__xgePathSvgReadNumber(&pText, &fRY) ||
				     !__xgePathSvgReadNumber(&pText, &fAxis) || !__xgePathSvgReadNumber(&pText, &fLargeArc) ||
				     !__xgePathSvgReadNumber(&pText, &fSweep) || !__xgePathSvgReadNumber(&pText, &fX) ||
				     !__xgePathSvgReadNumber(&pText, &fY) ) return XGE_ERROR_INVALID_ARGUMENT;
				if ( bRelative ) {
					fX += tCurrent.fX;
					fY += tCurrent.fY;
				}
				iRet = __xgePathSvgArcTo(pPath, tCurrent, fRX, fRY, fAxis, fLargeArc != 0.0f, fSweep != 0.0f, fX, fY);
				if ( iRet != XGE_OK ) return iRet;
				tCurrent = (xge_vec2_t){fX, fY};
				tLastCubicControl = tCurrent;
				tLastQuadControl = tCurrent;
				iPrevCommand = 'a';
			}
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
}

int xgePathGetCommandCount(xge_path pPath)
{
	return __xgePathValid(pPath) ? pPath->iCommandCount : 0;
}

int xgePathGetCommand(xge_path pPath, int iIndex, xge_path_command_t* pCommand)
{
	if ( !__xgePathValid(pPath) || (pCommand == NULL) || (iIndex < 0) || (iIndex >= pPath->iCommandCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pCommand = pPath->pCommands[iIndex];
	return XGE_OK;
}

int xgePathFlatten(xge_path pPath, xge_vec2_t* pPoints, int iCapacity, float fTolerance)
{
	xge_path_command_t* pCommand;
	xge_vec2_t tCurrent;
	xge_vec2_t tStart;
	xge_vec2_t tPoint;
	float t;
	float u;
	int bHasCurrent;
	int iCount;
	int iSteps;
	int i;
	int j;

	if ( !__xgePathValid(pPath) || (iCapacity < 0) || ((pPoints == NULL) && (iCapacity > 0)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tCurrent, 0, sizeof(tCurrent));
	memset(&tStart, 0, sizeof(tStart));
	bHasCurrent = 0;
	iCount = 0;
	iSteps = __xgePathFlattenSteps(fTolerance);
	for ( i = 0; i < pPath->iCommandCount; i++ ) {
		pCommand = &pPath->pCommands[i];
		if ( pCommand->iCommand == XGE_PATH_CMD_MOVE ) {
			tCurrent = pCommand->arrPoints[0];
			tStart = tCurrent;
			bHasCurrent = 1;
			__xgePathFlattenAdd(pPoints, iCapacity, &iCount, tCurrent);
		} else if ( (pCommand->iCommand == XGE_PATH_CMD_LINE) && bHasCurrent ) {
			tCurrent = pCommand->arrPoints[0];
			__xgePathFlattenAdd(pPoints, iCapacity, &iCount, tCurrent);
		} else if ( (pCommand->iCommand == XGE_PATH_CMD_QUAD) && bHasCurrent ) {
			for ( j = 1; j <= iSteps; j++ ) {
				t = (float)j / (float)iSteps;
				u = 1.0f - t;
				tPoint.fX = u * u * tCurrent.fX + 2.0f * u * t * pCommand->arrPoints[0].fX + t * t * pCommand->arrPoints[1].fX;
				tPoint.fY = u * u * tCurrent.fY + 2.0f * u * t * pCommand->arrPoints[0].fY + t * t * pCommand->arrPoints[1].fY;
				__xgePathFlattenAdd(pPoints, iCapacity, &iCount, tPoint);
			}
			tCurrent = pCommand->arrPoints[1];
		} else if ( (pCommand->iCommand == XGE_PATH_CMD_CUBIC) && bHasCurrent ) {
			for ( j = 1; j <= iSteps; j++ ) {
				t = (float)j / (float)iSteps;
				u = 1.0f - t;
				tPoint.fX = u * u * u * tCurrent.fX + 3.0f * u * u * t * pCommand->arrPoints[0].fX + 3.0f * u * t * t * pCommand->arrPoints[1].fX + t * t * t * pCommand->arrPoints[2].fX;
				tPoint.fY = u * u * u * tCurrent.fY + 3.0f * u * u * t * pCommand->arrPoints[0].fY + 3.0f * u * t * t * pCommand->arrPoints[1].fY + t * t * t * pCommand->arrPoints[2].fY;
				__xgePathFlattenAdd(pPoints, iCapacity, &iCount, tPoint);
			}
			tCurrent = pCommand->arrPoints[2];
		} else if ( (pCommand->iCommand == XGE_PATH_CMD_CLOSE) && bHasCurrent ) {
			if ( (tCurrent.fX != tStart.fX) || (tCurrent.fY != tStart.fY) ) {
				__xgePathFlattenAdd(pPoints, iCapacity, &iCount, tStart);
			}
			tCurrent = tStart;
		}
	}
	return iCount;
}

static int __xgePathFillRuleValid(int iFillRule)
{
	return (iFillRule == XGE_PATH_FILL_NON_ZERO) || (iFillRule == XGE_PATH_FILL_EVEN_ODD);
}

typedef struct xge_path_flat_contour_t {
	int iStart;
	int iCount;
	int bClosed;
} xge_path_flat_contour_t;

typedef struct xge_path_flatten_data_t {
	xge_vec2_t* pPoints;
	int iPointCount;
	int iPointCapacity;
	xge_path_flat_contour_t* pContours;
	int iContourCount;
	int iContourCapacity;
} xge_path_flatten_data_t;

typedef struct xge_path_fill_edge_t {
	float fX0;
	float fY0;
	float fX1;
	float fY1;
	int iWinding;
} xge_path_fill_edge_t;

typedef struct xge_path_active_edge_t {
	const xge_path_fill_edge_t* pEdge;
	float fXMid;
} xge_path_active_edge_t;

static int __xgeFloatCompare(const void* pA, const void* pB)
{
	float fA;
	float fB;

	fA = *(const float*)pA;
	fB = *(const float*)pB;
	return (fA < fB) ? -1 : ((fA > fB) ? 1 : 0);
}

static int __xgePathActiveEdgeCompare(const void* pA, const void* pB)
{
	const xge_path_active_edge_t* pEA;
	const xge_path_active_edge_t* pEB;

	pEA = (const xge_path_active_edge_t*)pA;
	pEB = (const xge_path_active_edge_t*)pB;
	if ( pEA->fXMid < pEB->fXMid ) return -1;
	if ( pEA->fXMid > pEB->fXMid ) return 1;
	return (pEA->pEdge < pEB->pEdge) ? -1 : ((pEA->pEdge > pEB->pEdge) ? 1 : 0);
}

static void __xgePathFlattenDataFree(xge_path_flatten_data_t* pData)
{
	if ( pData == NULL ) {
		return;
	}
	if ( pData->pPoints != NULL ) {
		xrtFree(pData->pPoints);
	}
	if ( pData->pContours != NULL ) {
		xrtFree(pData->pContours);
	}
	memset(pData, 0, sizeof(*pData));
}

static int __xgePathFlattenDataReservePoints(xge_path_flatten_data_t* pData, int iNeeded)
{
	xge_vec2_t* pPoints;
	int iCapacity;

	if ( (pData == NULL) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pData->iPointCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pData->iPointCapacity > 0) ? pData->iPointCapacity : 32;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) return XGE_ERROR_OUT_OF_MEMORY;
		iCapacity *= 2;
	}
	pPoints = (xge_vec2_t*)xrtRealloc(pData->pPoints, sizeof(*pPoints) * (size_t)iCapacity);
	if ( pPoints == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pData->pPoints = pPoints;
	pData->iPointCapacity = iCapacity;
	return XGE_OK;
}

static int __xgePathFlattenDataReserveContours(xge_path_flatten_data_t* pData, int iNeeded)
{
	xge_path_flat_contour_t* pContours;
	int iCapacity;

	if ( (pData == NULL) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pData->iContourCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pData->iContourCapacity > 0) ? pData->iContourCapacity : 8;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) return XGE_ERROR_OUT_OF_MEMORY;
		iCapacity *= 2;
	}
	pContours = (xge_path_flat_contour_t*)xrtRealloc(pData->pContours, sizeof(*pContours) * (size_t)iCapacity);
	if ( pContours == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pData->pContours = pContours;
	pData->iContourCapacity = iCapacity;
	return XGE_OK;
}

static int __xgePathFlattenDataAddPoint(xge_path_flatten_data_t* pData, xge_vec2_t tPoint)
{
	xge_path_flat_contour_t* pContour;

	if ( (pData == NULL) || (pData->iContourCount <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pContour = &pData->pContours[pData->iContourCount - 1];
	if ( pContour->iCount > 0 ) {
		xge_vec2_t tPrev = pData->pPoints[pContour->iStart + pContour->iCount - 1];
		if ( (fabsf(tPrev.fX - tPoint.fX) <= XGE_PATH_EPSILON) &&
		     (fabsf(tPrev.fY - tPoint.fY) <= XGE_PATH_EPSILON) ) {
			return XGE_OK;
		}
	}
	if ( __xgePathFlattenDataReservePoints(pData, pData->iPointCount + 1) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pData->pPoints[pData->iPointCount++] = tPoint;
	pContour->iCount++;
	return XGE_OK;
}

static int __xgePathFlattenDataBeginContour(xge_path_flatten_data_t* pData, xge_vec2_t tPoint)
{
	xge_path_flat_contour_t* pContour;
	int iRet;

	iRet = __xgePathFlattenDataReserveContours(pData, pData->iContourCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pContour = &pData->pContours[pData->iContourCount++];
	memset(pContour, 0, sizeof(*pContour));
	pContour->iStart = pData->iPointCount;
	return __xgePathFlattenDataAddPoint(pData, tPoint);
}

static int __xgePathFlattenToContours(xge_path pPath, float fTolerance, xge_path_flatten_data_t* pData)
{
	xge_path_command_t* pCommand;
	xge_vec2_t tCurrent;
	xge_vec2_t tStart;
	xge_vec2_t tPoint;
	float t;
	float u;
	int bHasCurrent;
	int iSteps;
	int i;
	int j;
	int iRet;

	if ( !__xgePathValid(pPath) || (pData == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pData, 0, sizeof(*pData));
	memset(&tCurrent, 0, sizeof(tCurrent));
	memset(&tStart, 0, sizeof(tStart));
	bHasCurrent = 0;
	iSteps = __xgePathFlattenSteps(fTolerance);
	for ( i = 0; i < pPath->iCommandCount; i++ ) {
		pCommand = &pPath->pCommands[i];
		if ( pCommand->iCommand == XGE_PATH_CMD_MOVE ) {
			tCurrent = pCommand->arrPoints[0];
			tStart = tCurrent;
			iRet = __xgePathFlattenDataBeginContour(pData, tCurrent);
			if ( iRet != XGE_OK ) goto error;
			bHasCurrent = 1;
		} else if ( (pCommand->iCommand == XGE_PATH_CMD_LINE) && bHasCurrent ) {
			tCurrent = pCommand->arrPoints[0];
			iRet = __xgePathFlattenDataAddPoint(pData, tCurrent);
			if ( iRet != XGE_OK ) goto error;
		} else if ( (pCommand->iCommand == XGE_PATH_CMD_QUAD) && bHasCurrent ) {
			for ( j = 1; j <= iSteps; j++ ) {
				t = (float)j / (float)iSteps;
				u = 1.0f - t;
				tPoint.fX = u * u * tCurrent.fX + 2.0f * u * t * pCommand->arrPoints[0].fX + t * t * pCommand->arrPoints[1].fX;
				tPoint.fY = u * u * tCurrent.fY + 2.0f * u * t * pCommand->arrPoints[0].fY + t * t * pCommand->arrPoints[1].fY;
				iRet = __xgePathFlattenDataAddPoint(pData, tPoint);
				if ( iRet != XGE_OK ) goto error;
			}
			tCurrent = pCommand->arrPoints[1];
		} else if ( (pCommand->iCommand == XGE_PATH_CMD_CUBIC) && bHasCurrent ) {
			for ( j = 1; j <= iSteps; j++ ) {
				t = (float)j / (float)iSteps;
				u = 1.0f - t;
				tPoint.fX = u * u * u * tCurrent.fX + 3.0f * u * u * t * pCommand->arrPoints[0].fX + 3.0f * u * t * t * pCommand->arrPoints[1].fX + t * t * t * pCommand->arrPoints[2].fX;
				tPoint.fY = u * u * u * tCurrent.fY + 3.0f * u * u * t * pCommand->arrPoints[0].fY + 3.0f * u * t * t * pCommand->arrPoints[1].fY + t * t * t * pCommand->arrPoints[2].fY;
				iRet = __xgePathFlattenDataAddPoint(pData, tPoint);
				if ( iRet != XGE_OK ) goto error;
			}
			tCurrent = pCommand->arrPoints[2];
		} else if ( (pCommand->iCommand == XGE_PATH_CMD_CLOSE) && bHasCurrent ) {
			if ( pData->iContourCount > 0 ) {
				pData->pContours[pData->iContourCount - 1].bClosed = 1;
			}
			tCurrent = tStart;
		}
	}
	return XGE_OK;

error:
	__xgePathFlattenDataFree(pData);
	return iRet;
}

static float __xgePathEdgeXAtY(const xge_path_fill_edge_t* pEdge, float fY)
{
	float fDY;
	float fT;

	fDY = pEdge->fY1 - pEdge->fY0;
	if ( fabsf(fDY) <= XGE_PATH_EPSILON ) {
		return pEdge->fX0;
	}
	fT = (fY - pEdge->fY0) / fDY;
	return pEdge->fX0 + (pEdge->fX1 - pEdge->fX0) * fT;
}

static int __xgePathAppendFillEdge(xge_path_fill_edge_t* pEdges, int iEdgeCapacity, int* pEdgeCount, xge_vec2_t tA, xge_vec2_t tB)
{
	xge_path_fill_edge_t* pEdge;

	if ( (pEdges == NULL) || (pEdgeCount == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( fabsf(tA.fY - tB.fY) <= XGE_PATH_EPSILON ) {
		return XGE_OK;
	}
	if ( *pEdgeCount >= iEdgeCapacity ) {
		return XGE_ERROR_BUFFER_TOO_SMALL;
	}
	pEdge = &pEdges[(*pEdgeCount)++];
	pEdge->fX0 = tA.fX;
	pEdge->fY0 = tA.fY;
	pEdge->fX1 = tB.fX;
	pEdge->fY1 = tB.fY;
	pEdge->iWinding = (tB.fY > tA.fY) ? 1 : -1;
	return XGE_OK;
}

static int __xgePathBuildFillEdges(const xge_path_flatten_data_t* pData, xge_path_fill_edge_t* pEdges, int iEdgeCapacity, int* pEdgeCount, float* pYValues, int iYCapacity, int* pYCount)
{
	const xge_path_flat_contour_t* pContour;
	xge_vec2_t tA;
	xge_vec2_t tB;
	int i;
	int j;
	int iCount;
	int iRet;

	if ( (pData == NULL) || (pEdges == NULL) || (pEdgeCount == NULL) || (pYValues == NULL) || (pYCount == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pEdgeCount = 0;
	*pYCount = 0;
	for ( i = 0; i < pData->iContourCount; i++ ) {
		pContour = &pData->pContours[i];
		iCount = pContour->iCount;
		while ( (iCount > 1) &&
		        (fabsf(pData->pPoints[pContour->iStart + iCount - 1].fX - pData->pPoints[pContour->iStart].fX) <= XGE_PATH_EPSILON) &&
		        (fabsf(pData->pPoints[pContour->iStart + iCount - 1].fY - pData->pPoints[pContour->iStart].fY) <= XGE_PATH_EPSILON) ) {
			iCount--;
		}
		if ( iCount < 3 ) {
			continue;
		}
		for ( j = 0; j < iCount; j++ ) {
			tA = pData->pPoints[pContour->iStart + j];
			tB = pData->pPoints[pContour->iStart + ((j + 1) % iCount)];
			iRet = __xgePathAppendFillEdge(pEdges, iEdgeCapacity, pEdgeCount, tA, tB);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			if ( *pYCount + 2 > iYCapacity ) {
				return XGE_ERROR_BUFFER_TOO_SMALL;
			}
			pYValues[(*pYCount)++] = tA.fY;
			pYValues[(*pYCount)++] = tB.fY;
		}
	}
	return XGE_OK;
}

static int __xgePathUniqueSortedY(float* pYValues, int iYCount)
{
	int iWrite;
	int i;

	if ( (pYValues == NULL) || (iYCount <= 0) ) {
		return 0;
	}
	qsort(pYValues, (size_t)iYCount, sizeof(*pYValues), __xgeFloatCompare);
	iWrite = 1;
	for ( i = 1; i < iYCount; i++ ) {
		if ( fabsf(pYValues[i] - pYValues[iWrite - 1]) > XGE_PATH_EPSILON ) {
			pYValues[iWrite++] = pYValues[i];
		}
	}
	return iWrite;
}

static int __xgePathEmitFillTrap(xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, uint32_t iColor, int* pVertexCount, int* pIndexCount, const xge_path_fill_edge_t* pLeft, const xge_path_fill_edge_t* pRight, float fY0, float fY1)
{
	float fLeft0;
	float fLeft1;
	float fRight0;
	float fRight1;
	int iBase;

	fLeft0 = __xgePathEdgeXAtY(pLeft, fY0);
	fLeft1 = __xgePathEdgeXAtY(pLeft, fY1);
	fRight0 = __xgePathEdgeXAtY(pRight, fY0);
	fRight1 = __xgePathEdgeXAtY(pRight, fY1);
	if ( (fabsf(fRight0 - fLeft0) <= XGE_PATH_EPSILON) && (fabsf(fRight1 - fLeft1) <= XGE_PATH_EPSILON) ) {
		return XGE_OK;
	}
	iBase = *pVertexCount;
	*pVertexCount += 4;
	*pIndexCount += 6;
	if ( (pVertices == NULL) && (pIndices == NULL) ) {
		return XGE_OK;
	}
	if ( (iBase + 4 > iVertexCapacity) || (*pIndexCount > iIndexCapacity) ) {
		return XGE_ERROR_BUFFER_TOO_SMALL;
	}
	pVertices[iBase + 0] = (xge_shape_vertex_t){fLeft0, fY0, iColor};
	pVertices[iBase + 1] = (xge_shape_vertex_t){fRight0, fY0, iColor};
	pVertices[iBase + 2] = (xge_shape_vertex_t){fRight1, fY1, iColor};
	pVertices[iBase + 3] = (xge_shape_vertex_t){fLeft1, fY1, iColor};
	pIndices[*pIndexCount - 6] = (uint32_t)(iBase + 0);
	pIndices[*pIndexCount - 5] = (uint32_t)(iBase + 1);
	pIndices[*pIndexCount - 4] = (uint32_t)(iBase + 2);
	pIndices[*pIndexCount - 3] = (uint32_t)(iBase + 0);
	pIndices[*pIndexCount - 2] = (uint32_t)(iBase + 2);
	pIndices[*pIndexCount - 1] = (uint32_t)(iBase + 3);
	return XGE_OK;
}

static int __xgePathGenerateFillMesh(const xge_path_fill_edge_t* pEdges, int iEdgeCount, float* pYValues, int iYCount, xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, uint32_t iColor, int iFillRule, int* pVertexCount, int* pIndexCount)
{
	xge_path_active_edge_t* pActive;
	float fY0;
	float fY1;
	float fYMid;
	int iActiveCount;
	int iWinding;
	int i;
	int j;
	int iRet;

	*pVertexCount = 0;
	*pIndexCount = 0;
	if ( (iEdgeCount <= 0) || (iYCount < 2) ) {
		return XGE_OK;
	}
	pActive = (xge_path_active_edge_t*)xrtMalloc(sizeof(*pActive) * (size_t)iEdgeCount);
	if ( pActive == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < iYCount - 1; i++ ) {
		fY0 = pYValues[i];
		fY1 = pYValues[i + 1];
		if ( fY1 - fY0 <= XGE_PATH_EPSILON ) {
			continue;
		}
		fYMid = (fY0 + fY1) * 0.5f;
		iActiveCount = 0;
		for ( j = 0; j < iEdgeCount; j++ ) {
			float fMinY = (pEdges[j].fY0 < pEdges[j].fY1) ? pEdges[j].fY0 : pEdges[j].fY1;
			float fMaxY = (pEdges[j].fY0 > pEdges[j].fY1) ? pEdges[j].fY0 : pEdges[j].fY1;
			if ( (fYMid > fMinY) && (fYMid < fMaxY) ) {
				pActive[iActiveCount].pEdge = &pEdges[j];
				pActive[iActiveCount].fXMid = __xgePathEdgeXAtY(&pEdges[j], fYMid);
				iActiveCount++;
			}
		}
		if ( iActiveCount < 2 ) {
			continue;
		}
		qsort(pActive, (size_t)iActiveCount, sizeof(*pActive), __xgePathActiveEdgeCompare);
		if ( iFillRule == XGE_PATH_FILL_EVEN_ODD ) {
			for ( j = 0; j + 1 < iActiveCount; j += 2 ) {
				iRet = __xgePathEmitFillTrap(pVertices, iVertexCapacity, pIndices, iIndexCapacity, iColor, pVertexCount, pIndexCount, pActive[j].pEdge, pActive[j + 1].pEdge, fY0, fY1);
				if ( iRet != XGE_OK ) {
					xrtFree(pActive);
					return iRet;
				}
			}
		} else {
			iWinding = 0;
			for ( j = 0; j + 1 < iActiveCount; j++ ) {
				iWinding += pActive[j].pEdge->iWinding;
				if ( iWinding != 0 ) {
					iRet = __xgePathEmitFillTrap(pVertices, iVertexCapacity, pIndices, iIndexCapacity, iColor, pVertexCount, pIndexCount, pActive[j].pEdge, pActive[j + 1].pEdge, fY0, fY1);
					if ( iRet != XGE_OK ) {
						xrtFree(pActive);
						return iRet;
					}
				}
			}
		}
	}
	xrtFree(pActive);
	return XGE_OK;
}

static float __xgePathContourArea(const xge_path_flatten_data_t* pData, const xge_path_flat_contour_t* pContour)
{
	float fArea;
	int iCount;
	int i;

	if ( (pData == NULL) || (pContour == NULL) ) {
		return 0.0f;
	}
	iCount = pContour->iCount;
	while ( (iCount > 1) &&
	        (fabsf(pData->pPoints[pContour->iStart + iCount - 1].fX - pData->pPoints[pContour->iStart].fX) <= XGE_PATH_EPSILON) &&
	        (fabsf(pData->pPoints[pContour->iStart + iCount - 1].fY - pData->pPoints[pContour->iStart].fY) <= XGE_PATH_EPSILON) ) {
		iCount--;
	}
	if ( iCount < 3 ) {
		return 0.0f;
	}
	fArea = 0.0f;
	for ( i = 0; i < iCount; i++ ) {
		xge_vec2_t tA = pData->pPoints[pContour->iStart + i];
		xge_vec2_t tB = pData->pPoints[pContour->iStart + ((i + 1) % iCount)];

		fArea += (tA.fX * tB.fY) - (tB.fX * tA.fY);
	}
	return fArea * 0.5f;
}

static int __xgePathPointFilled(const xge_path_flatten_data_t* pData, float fX, float fY, int iFillRule)
{
	const xge_path_flat_contour_t* pContour;
	int iParity;
	int iWinding;
	int i;
	int j;

	if ( pData == NULL ) {
		return 0;
	}
	iParity = 0;
	iWinding = 0;
	for ( i = 0; i < pData->iContourCount; i++ ) {
		int iCount;

		pContour = &pData->pContours[i];
		iCount = pContour->iCount;
		while ( (iCount > 1) &&
		        (fabsf(pData->pPoints[pContour->iStart + iCount - 1].fX - pData->pPoints[pContour->iStart].fX) <= XGE_PATH_EPSILON) &&
		        (fabsf(pData->pPoints[pContour->iStart + iCount - 1].fY - pData->pPoints[pContour->iStart].fY) <= XGE_PATH_EPSILON) ) {
			iCount--;
		}
		if ( iCount < 3 ) {
			continue;
		}
		for ( j = 0; j < iCount; j++ ) {
			xge_vec2_t tA = pData->pPoints[pContour->iStart + j];
			xge_vec2_t tB = pData->pPoints[pContour->iStart + ((j + 1) % iCount)];

			if ( ((tA.fY <= fY) && (tB.fY > fY)) || ((tB.fY <= fY) && (tA.fY > fY)) ) {
				float fXAtY = tA.fX + ((fY - tA.fY) * (tB.fX - tA.fX) / (tB.fY - tA.fY));

				if ( fXAtY > fX ) {
					if ( iFillRule == XGE_PATH_FILL_EVEN_ODD ) {
						iParity = !iParity;
					} else {
						iWinding += (tB.fY > tA.fY) ? 1 : -1;
					}
				}
			}
		}
	}
	return (iFillRule == XGE_PATH_FILL_EVEN_ODD) ? iParity : (iWinding != 0);
}

static uint32_t __xgePathColorPremulAlphaScale(uint32_t iColor, float fScale)
{
	float fAlpha;
	uint32_t iA;
	uint32_t iR;
	uint32_t iG;
	uint32_t iB;

	if ( fScale <= 0.0f ) {
		return 0x00000000u;
	}
	if ( fScale > 1.0f ) {
		fScale = 1.0f;
	}
	fAlpha = ((float)XGE_COLOR_GET_A(iColor) / 255.0f) * fScale;
	if ( fAlpha <= 0.0f ) {
		return 0x00000000u;
	}
	iA = (uint32_t)(fAlpha * 255.0f + 0.5f);
	if ( iA > 255u ) iA = 255u;
	iR = (uint32_t)((float)XGE_COLOR_GET_R(iColor) * fAlpha + 0.5f);
	iG = (uint32_t)((float)XGE_COLOR_GET_G(iColor) * fAlpha + 0.5f);
	iB = (uint32_t)((float)XGE_COLOR_GET_B(iColor) * fAlpha + 0.5f);
	if ( iR > 255u ) iR = 255u;
	if ( iG > 255u ) iG = 255u;
	if ( iB > 255u ) iB = 255u;
	return XGE_COLOR_RGBA(iR, iG, iB, iA);
}

static int __xgePathEmitFillAAEdge(xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, int* pVertexCount, int* pIndexCount, xge_vec2_t tA, xge_vec2_t tB, float fNX, float fNY, float fFringeWidth, uint32_t iInnerColor)
{
	int iBase;

	iBase = *pVertexCount;
	*pVertexCount += 4;
	*pIndexCount += 6;
	if ( (pVertices == NULL) && (pIndices == NULL) ) {
		return XGE_OK;
	}
	if ( (iBase + 4 > iVertexCapacity) || (*pIndexCount > iIndexCapacity) ) {
		return XGE_ERROR_BUFFER_TOO_SMALL;
	}
	pVertices[iBase + 0] = (xge_shape_vertex_t){tA.fX, tA.fY, iInnerColor};
	pVertices[iBase + 1] = (xge_shape_vertex_t){tB.fX, tB.fY, iInnerColor};
	pVertices[iBase + 2] = (xge_shape_vertex_t){tB.fX + (fNX * fFringeWidth), tB.fY + (fNY * fFringeWidth), 0x00000000u};
	pVertices[iBase + 3] = (xge_shape_vertex_t){tA.fX + (fNX * fFringeWidth), tA.fY + (fNY * fFringeWidth), 0x00000000u};
	pIndices[*pIndexCount - 6] = (uint32_t)(iBase + 0);
	pIndices[*pIndexCount - 5] = (uint32_t)(iBase + 1);
	pIndices[*pIndexCount - 4] = (uint32_t)(iBase + 2);
	pIndices[*pIndexCount - 3] = (uint32_t)(iBase + 0);
	pIndices[*pIndexCount - 2] = (uint32_t)(iBase + 2);
	pIndices[*pIndexCount - 1] = (uint32_t)(iBase + 3);
	return XGE_OK;
}

static int __xgePathBuildFillAAMeshFromFlat(const xge_path_flatten_data_t* pData, xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, uint32_t iColor, int iFillRule, float fFringeWidth, int* pVertexCount, int* pIndexCount)
{
	const xge_path_flat_contour_t* pContour;
	uint32_t iInnerColor;
	float fSample;
	int i;
	int j;
	int iRet;

	if ( (pData == NULL) || (pVertexCount == NULL) || (pIndexCount == NULL) ||
	     (iVertexCapacity < 0) || (iIndexCapacity < 0) ||
	     ((pVertices == NULL) != (pIndices == NULL)) ||
	     ((pVertices == NULL) && ((iVertexCapacity > 0) || (iIndexCapacity > 0))) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pVertexCount = 0;
	*pIndexCount = 0;
	if ( (pData->iPointCount < 3) || (fFringeWidth <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0u) ) {
		return XGE_OK;
	}
	iInnerColor = __xgePathColorPremulAlphaScale(iColor, 1.0f);
	fSample = (fFringeWidth > 0.05f) ? (fFringeWidth * 0.25f) : 0.05f;
	for ( i = 0; i < pData->iContourCount; i++ ) {
		float fArea;
		int iCount;

		pContour = &pData->pContours[i];
		iCount = pContour->iCount;
		while ( (iCount > 1) &&
		        (fabsf(pData->pPoints[pContour->iStart + iCount - 1].fX - pData->pPoints[pContour->iStart].fX) <= XGE_PATH_EPSILON) &&
		        (fabsf(pData->pPoints[pContour->iStart + iCount - 1].fY - pData->pPoints[pContour->iStart].fY) <= XGE_PATH_EPSILON) ) {
			iCount--;
		}
		if ( iCount < 3 ) {
			continue;
		}
		fArea = __xgePathContourArea(pData, pContour);
		for ( j = 0; j < iCount; j++ ) {
			xge_vec2_t tA = pData->pPoints[pContour->iStart + j];
			xge_vec2_t tB = pData->pPoints[pContour->iStart + ((j + 1) % iCount)];
			float fDX = tB.fX - tA.fX;
			float fDY = tB.fY - tA.fY;
			float fLen = __xgePathSqrt((fDX * fDX) + (fDY * fDY));
			float fLeftNX;
			float fLeftNY;
			float fMidX;
			float fMidY;
			int bLeftFilled;
			int bRightFilled;
			int bOutwardLeft;

			if ( fLen <= XGE_PATH_EPSILON ) {
				continue;
			}
			fLeftNX = -fDY / fLen;
			fLeftNY = fDX / fLen;
			fMidX = (tA.fX + tB.fX) * 0.5f;
			fMidY = (tA.fY + tB.fY) * 0.5f;
			bLeftFilled = __xgePathPointFilled(pData, fMidX + (fLeftNX * fSample), fMidY + (fLeftNY * fSample), iFillRule);
			bRightFilled = __xgePathPointFilled(pData, fMidX - (fLeftNX * fSample), fMidY - (fLeftNY * fSample), iFillRule);
			if ( bLeftFilled != bRightFilled ) {
				bOutwardLeft = !bLeftFilled;
			} else {
				bOutwardLeft = (fArea >= 0.0f);
			}
			iRet = __xgePathEmitFillAAEdge(pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount, tA, tB, bOutwardLeft ? fLeftNX : -fLeftNX, bOutwardLeft ? fLeftNY : -fLeftNY, fFringeWidth, iInnerColor);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
	}
	return XGE_OK;
}

static int __xgePathBuildFillMeshWithRule(xge_path pPath, xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, uint32_t iColor, int iFillRule, float fTolerance, int* pVertexCount, int* pIndexCount)
{
	xge_path_flatten_data_t tFlat;
	xge_path_fill_edge_t* pEdges;
	float* pYValues;
	int iEdgeCapacity;
	int iEdgeCount;
	int iYCapacity;
	int iYCount;
	int iRet;

	if ( !__xgePathValid(pPath) || (pVertexCount == NULL) || (pIndexCount == NULL) ||
	     (iVertexCapacity < 0) || (iIndexCapacity < 0) ||
	     ((pVertices == NULL) != (pIndices == NULL)) ||
	     ((pVertices == NULL) && ((iVertexCapacity > 0) || (iIndexCapacity > 0))) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgePathFillRuleValid(iFillRule) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pVertexCount = 0;
	*pIndexCount = 0;
	memset(&tFlat, 0, sizeof(tFlat));
	iRet = __xgePathFlattenToContours(pPath, fTolerance, &tFlat);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( tFlat.iPointCount < 3 ) {
		__xgePathFlattenDataFree(&tFlat);
		return XGE_OK;
	}
	iEdgeCapacity = tFlat.iPointCount;
	iYCapacity = tFlat.iPointCount * 2;
	pEdges = (xge_path_fill_edge_t*)xrtMalloc(sizeof(*pEdges) * (size_t)iEdgeCapacity);
	pYValues = (float*)xrtMalloc(sizeof(*pYValues) * (size_t)iYCapacity);
	if ( (pEdges == NULL) || (pYValues == NULL) ) {
		if ( pEdges != NULL ) xrtFree(pEdges);
		if ( pYValues != NULL ) xrtFree(pYValues);
		__xgePathFlattenDataFree(&tFlat);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xgePathBuildFillEdges(&tFlat, pEdges, iEdgeCapacity, &iEdgeCount, pYValues, iYCapacity, &iYCount);
	if ( iRet == XGE_OK ) {
		iYCount = __xgePathUniqueSortedY(pYValues, iYCount);
		iRet = __xgePathGenerateFillMesh(pEdges, iEdgeCount, pYValues, iYCount, pVertices, iVertexCapacity, pIndices, iIndexCapacity, iColor, iFillRule, pVertexCount, pIndexCount);
	}
	xrtFree(pYValues);
	xrtFree(pEdges);
	__xgePathFlattenDataFree(&tFlat);
	return iRet;
}

int xgePathBuildFillMesh(xge_path pPath, xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, uint32_t iColor, float fTolerance, int* pVertexCount, int* pIndexCount)
{
	return __xgePathBuildFillMeshWithRule(pPath, pVertices, iVertexCapacity, pIndices, iIndexCapacity, iColor, XGE_PATH_FILL_NON_ZERO, fTolerance, pVertexCount, pIndexCount);
}

int xgePathBuildFillMeshEx(xge_path pPath, xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, uint32_t iColor, int iFillRule, float fTolerance, int* pVertexCount, int* pIndexCount)
{
	return __xgePathBuildFillMeshWithRule(pPath, pVertices, iVertexCapacity, pIndices, iIndexCapacity, iColor, iFillRule, fTolerance, pVertexCount, pIndexCount);
}

static int __xgePathBuildFillAAMeshWithRule(xge_path pPath, xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, uint32_t iColor, int iFillRule, float fTolerance, float fFringeWidth, int* pVertexCount, int* pIndexCount)
{
	xge_path_flatten_data_t tFlat;
	int iRet;

	if ( !__xgePathValid(pPath) || (pVertexCount == NULL) || (pIndexCount == NULL) ||
	     (iVertexCapacity < 0) || (iIndexCapacity < 0) ||
	     ((pVertices == NULL) != (pIndices == NULL)) ||
	     ((pVertices == NULL) && ((iVertexCapacity > 0) || (iIndexCapacity > 0))) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgePathFillRuleValid(iFillRule) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pVertexCount = 0;
	*pIndexCount = 0;
	if ( (fFringeWidth <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0u) ) {
		return XGE_OK;
	}
	memset(&tFlat, 0, sizeof(tFlat));
	iRet = __xgePathFlattenToContours(pPath, fTolerance, &tFlat);
	if ( iRet == XGE_OK ) {
		iRet = __xgePathBuildFillAAMeshFromFlat(&tFlat, pVertices, iVertexCapacity, pIndices, iIndexCapacity, iColor, iFillRule, fFringeWidth, pVertexCount, pIndexCount);
	}
	__xgePathFlattenDataFree(&tFlat);
	return iRet;
}

int xgePathBuildFillAAMesh(xge_path pPath, xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, uint32_t iColor, float fTolerance, float fFringeWidth, int* pVertexCount, int* pIndexCount)
{
	return __xgePathBuildFillAAMeshWithRule(pPath, pVertices, iVertexCapacity, pIndices, iIndexCapacity, iColor, XGE_PATH_FILL_NON_ZERO, fTolerance, fFringeWidth, pVertexCount, pIndexCount);
}

int xgePathBuildFillAAMeshEx(xge_path pPath, xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, uint32_t iColor, int iFillRule, float fTolerance, float fFringeWidth, int* pVertexCount, int* pIndexCount)
{
	return __xgePathBuildFillAAMeshWithRule(pPath, pVertices, iVertexCapacity, pIndices, iIndexCapacity, iColor, iFillRule, fTolerance, fFringeWidth, pVertexCount, pIndexCount);
}

static int __xgePathStrokeSegmentVertexCount(int iLineCap)
{
	return (iLineCap == XGE_PATH_CAP_ROUND) ? (4 + ((XGE_PATH_ROUND_CAP_SEGMENTS + 2) * 2)) : 4;
}

static int __xgePathStrokeSegmentIndexCount(int iLineCap)
{
	return (iLineCap == XGE_PATH_CAP_ROUND) ? (6 + (XGE_PATH_ROUND_CAP_SEGMENTS * 6)) : 6;
}

static int __xgePathStrokeJoinVertexCount(int iLineJoin)
{
	if ( iLineJoin == XGE_PATH_JOIN_ROUND ) {
		return XGE_PATH_ROUND_JOIN_SEGMENTS + 2;
	}
	return (iLineJoin == XGE_PATH_JOIN_MITER) ? 4 : 3;
}

static int __xgePathStrokeJoinIndexCount(int iLineJoin)
{
	if ( iLineJoin == XGE_PATH_JOIN_ROUND ) {
		return XGE_PATH_ROUND_JOIN_SEGMENTS * 3;
	}
	return (iLineJoin == XGE_PATH_JOIN_MITER) ? 6 : 3;
}

static void __xgePathStrokeWriteRoundCap(xge_shape_vertex_t* pVertices, uint32_t* pIndices, int* pVertexWrite, int* pIndexWrite, xge_vec2_t tCenter, float fUX, float fUY, float fNX, float fNY, int bEndCap, uint32_t iColor)
{
	int iCenter;
	int iArcBase;
	int i;

	iCenter = *pVertexWrite;
	iArcBase = iCenter + 1;
	pVertices[iCenter] = (xge_shape_vertex_t){tCenter.fX, tCenter.fY, iColor};
	for ( i = 0; i <= XGE_PATH_ROUND_CAP_SEGMENTS; i++ ) {
		float fT = (float)i / (float)XGE_PATH_ROUND_CAP_SEGMENTS;
		float fAngle = fT * 3.14159265358979323846f;
		float fC = cosf(fAngle);
		float fS = sinf(fAngle);
		float fX;
		float fY;

		if ( bEndCap ) {
			fX = tCenter.fX - (fNX * fC) + (fUX * fS);
			fY = tCenter.fY - (fNY * fC) + (fUY * fS);
		} else {
			fX = tCenter.fX + (fNX * fC) - (fUX * fS);
			fY = tCenter.fY + (fNY * fC) - (fUY * fS);
		}
		pVertices[iArcBase + i] = (xge_shape_vertex_t){fX, fY, iColor};
	}
	for ( i = 0; i < XGE_PATH_ROUND_CAP_SEGMENTS; i++ ) {
		pIndices[*pIndexWrite + 0] = (uint32_t)iCenter;
		pIndices[*pIndexWrite + 1] = (uint32_t)(iArcBase + i);
		pIndices[*pIndexWrite + 2] = (uint32_t)(iArcBase + i + 1);
		*pIndexWrite += 3;
	}
	*pVertexWrite += XGE_PATH_ROUND_CAP_SEGMENTS + 2;
}

static void __xgePathStrokeWriteSegment(xge_shape_vertex_t* pVertices, uint32_t* pIndices, int* pVertexWrite, int* pIndexWrite, xge_vec2_t tA, xge_vec2_t tB, float fHalf, uint32_t iColor, int iLineCap)
{
	float fDX = tB.fX - tA.fX;
	float fDY = tB.fY - tA.fY;
	float fLen = __xgePathSqrt(fDX * fDX + fDY * fDY);
	float fUX;
	float fUY;
	float fNX;
	float fNY;
	int iBase;

	if ( fLen <= 0.000001f ) {
		return;
	}
	fUX = (fDX / fLen) * fHalf;
	fUY = (fDY / fLen) * fHalf;
	fNX = (-fDY / fLen) * fHalf;
	fNY = (fDX / fLen) * fHalf;
	if ( iLineCap == XGE_PATH_CAP_SQUARE ) {
		tA.fX -= fUX;
		tA.fY -= fUY;
		tB.fX += fUX;
		tB.fY += fUY;
	}
	iBase = *pVertexWrite;
	pVertices[iBase + 0] = (xge_shape_vertex_t){tA.fX + fNX, tA.fY + fNY, iColor};
	pVertices[iBase + 1] = (xge_shape_vertex_t){tA.fX - fNX, tA.fY - fNY, iColor};
	pVertices[iBase + 2] = (xge_shape_vertex_t){tB.fX + fNX, tB.fY + fNY, iColor};
	pVertices[iBase + 3] = (xge_shape_vertex_t){tB.fX - fNX, tB.fY - fNY, iColor};
	pIndices[*pIndexWrite + 0] = (uint32_t)(iBase + 0);
	pIndices[*pIndexWrite + 1] = (uint32_t)(iBase + 1);
	pIndices[*pIndexWrite + 2] = (uint32_t)(iBase + 2);
	pIndices[*pIndexWrite + 3] = (uint32_t)(iBase + 2);
	pIndices[*pIndexWrite + 4] = (uint32_t)(iBase + 1);
	pIndices[*pIndexWrite + 5] = (uint32_t)(iBase + 3);
	*pVertexWrite += 4;
	*pIndexWrite += 6;
	if ( iLineCap == XGE_PATH_CAP_ROUND ) {
		__xgePathStrokeWriteRoundCap(pVertices, pIndices, pVertexWrite, pIndexWrite, tA, fUX, fUY, fNX, fNY, 0, iColor);
		__xgePathStrokeWriteRoundCap(pVertices, pIndices, pVertexWrite, pIndexWrite, tB, fUX, fUY, fNX, fNY, 1, iColor);
	}
}

static int __xgePathStrokeLineIntersection(xge_vec2_t tP0, float fD0X, float fD0Y, xge_vec2_t tP1, float fD1X, float fD1Y, xge_vec2_t* pOut)
{
	float fDenom;
	float fT;

	fDenom = (fD0X * fD1Y) - (fD0Y * fD1X);
	if ( (fDenom > -0.000001f) && (fDenom < 0.000001f) ) {
		return 0;
	}
	fT = (((tP1.fX - tP0.fX) * fD1Y) - ((tP1.fY - tP0.fY) * fD1X)) / fDenom;
	pOut->fX = tP0.fX + (fD0X * fT);
	pOut->fY = tP0.fY + (fD0Y * fT);
	return 1;
}

static void __xgePathStrokeWriteBevelJoin(xge_shape_vertex_t* pVertices, uint32_t* pIndices, int* pVertexWrite, int* pIndexWrite, xge_vec2_t tPoint, xge_vec2_t tOuterA, xge_vec2_t tOuterB, uint32_t iColor)
{
	int iBase;

	iBase = *pVertexWrite;
	pVertices[iBase + 0] = (xge_shape_vertex_t){tPoint.fX, tPoint.fY, iColor};
	pVertices[iBase + 1] = (xge_shape_vertex_t){tOuterA.fX, tOuterA.fY, iColor};
	pVertices[iBase + 2] = (xge_shape_vertex_t){tOuterB.fX, tOuterB.fY, iColor};
	pIndices[*pIndexWrite + 0] = (uint32_t)(iBase + 0);
	pIndices[*pIndexWrite + 1] = (uint32_t)(iBase + 1);
	pIndices[*pIndexWrite + 2] = (uint32_t)(iBase + 2);
	*pVertexWrite += 3;
	*pIndexWrite += 3;
}

static void __xgePathStrokeWriteMiterJoin(xge_shape_vertex_t* pVertices, uint32_t* pIndices, int* pVertexWrite, int* pIndexWrite, xge_vec2_t tPoint, xge_vec2_t tOuterA, xge_vec2_t tOuterB, xge_vec2_t tMiter, uint32_t iColor)
{
	int iBase;

	iBase = *pVertexWrite;
	pVertices[iBase + 0] = (xge_shape_vertex_t){tPoint.fX, tPoint.fY, iColor};
	pVertices[iBase + 1] = (xge_shape_vertex_t){tOuterA.fX, tOuterA.fY, iColor};
	pVertices[iBase + 2] = (xge_shape_vertex_t){tMiter.fX, tMiter.fY, iColor};
	pVertices[iBase + 3] = (xge_shape_vertex_t){tOuterB.fX, tOuterB.fY, iColor};
	pIndices[*pIndexWrite + 0] = (uint32_t)(iBase + 0);
	pIndices[*pIndexWrite + 1] = (uint32_t)(iBase + 1);
	pIndices[*pIndexWrite + 2] = (uint32_t)(iBase + 2);
	pIndices[*pIndexWrite + 3] = (uint32_t)(iBase + 0);
	pIndices[*pIndexWrite + 4] = (uint32_t)(iBase + 2);
	pIndices[*pIndexWrite + 5] = (uint32_t)(iBase + 3);
	*pVertexWrite += 4;
	*pIndexWrite += 6;
}

static void __xgePathStrokeWriteRoundJoin(xge_shape_vertex_t* pVertices, uint32_t* pIndices, int* pVertexWrite, int* pIndexWrite, xge_vec2_t tPoint, float fHalf, float fStartAngle, float fEndAngle, uint32_t iColor)
{
	int iCenter;
	int iArcBase;
	int i;

	iCenter = *pVertexWrite;
	iArcBase = iCenter + 1;
	pVertices[iCenter] = (xge_shape_vertex_t){tPoint.fX, tPoint.fY, iColor};
	for ( i = 0; i <= XGE_PATH_ROUND_JOIN_SEGMENTS; i++ ) {
		float fT = (float)i / (float)XGE_PATH_ROUND_JOIN_SEGMENTS;
		float fAngle = fStartAngle + ((fEndAngle - fStartAngle) * fT);

		pVertices[iArcBase + i] = (xge_shape_vertex_t){tPoint.fX + (cosf(fAngle) * fHalf), tPoint.fY + (sinf(fAngle) * fHalf), iColor};
	}
	for ( i = 0; i < XGE_PATH_ROUND_JOIN_SEGMENTS; i++ ) {
		pIndices[*pIndexWrite + 0] = (uint32_t)iCenter;
		pIndices[*pIndexWrite + 1] = (uint32_t)(iArcBase + i);
		pIndices[*pIndexWrite + 2] = (uint32_t)(iArcBase + i + 1);
		*pIndexWrite += 3;
	}
	*pVertexWrite += XGE_PATH_ROUND_JOIN_SEGMENTS + 2;
}

static void __xgePathStrokeWriteJoin(xge_shape_vertex_t* pVertices, uint32_t* pIndices, int* pVertexWrite, int* pIndexWrite, xge_vec2_t tPrev, xge_vec2_t tPoint, xge_vec2_t tNext, float fHalf, uint32_t iColor, int iLineJoin)
{
	float fD0X;
	float fD0Y;
	float fD1X;
	float fD1Y;
	float fLen0;
	float fLen1;
	float fN0X;
	float fN0Y;
	float fN1X;
	float fN1Y;
	float fCross;
	float fSide;
	float fStartAngle;
	float fEndAngle;
	xge_vec2_t tOuterA;
	xge_vec2_t tOuterB;
	xge_vec2_t tMiter;

	fD0X = tPoint.fX - tPrev.fX;
	fD0Y = tPoint.fY - tPrev.fY;
	fD1X = tNext.fX - tPoint.fX;
	fD1Y = tNext.fY - tPoint.fY;
	fLen0 = __xgePathSqrt((fD0X * fD0X) + (fD0Y * fD0Y));
	fLen1 = __xgePathSqrt((fD1X * fD1X) + (fD1Y * fD1Y));
	if ( (fLen0 <= 0.000001f) || (fLen1 <= 0.000001f) ) {
		return;
	}
	fD0X /= fLen0;
	fD0Y /= fLen0;
	fD1X /= fLen1;
	fD1Y /= fLen1;
	fCross = (fD0X * fD1Y) - (fD0Y * fD1X);
	if ( (fCross > -0.000001f) && (fCross < 0.000001f) ) {
		return;
	}
	fN0X = -fD0Y;
	fN0Y = fD0X;
	fN1X = -fD1Y;
	fN1Y = fD1X;
	fSide = (fCross > 0.0f) ? -1.0f : 1.0f;
	tOuterA = (xge_vec2_t){tPoint.fX + (fN0X * fHalf * fSide), tPoint.fY + (fN0Y * fHalf * fSide)};
	tOuterB = (xge_vec2_t){tPoint.fX + (fN1X * fHalf * fSide), tPoint.fY + (fN1Y * fHalf * fSide)};
	if ( iLineJoin == XGE_PATH_JOIN_ROUND ) {
		fStartAngle = atan2f(tOuterA.fY - tPoint.fY, tOuterA.fX - tPoint.fX);
		fEndAngle = atan2f(tOuterB.fY - tPoint.fY, tOuterB.fX - tPoint.fX);
		if ( fCross > 0.0f ) {
			while ( fEndAngle > fStartAngle ) fEndAngle -= 6.28318530717958647692f;
		} else {
			while ( fEndAngle < fStartAngle ) fEndAngle += 6.28318530717958647692f;
		}
		__xgePathStrokeWriteRoundJoin(pVertices, pIndices, pVertexWrite, pIndexWrite, tPoint, fHalf, fStartAngle, fEndAngle, iColor);
		return;
	}
	if ( iLineJoin == XGE_PATH_JOIN_MITER ) {
		xge_vec2_t tLineA = {tPoint.fX + (fN0X * fHalf * fSide), tPoint.fY + (fN0Y * fHalf * fSide)};
		xge_vec2_t tLineB = {tPoint.fX + (fN1X * fHalf * fSide), tPoint.fY + (fN1Y * fHalf * fSide)};

		if ( __xgePathStrokeLineIntersection(tLineA, fD0X, fD0Y, tLineB, fD1X, fD1Y, &tMiter) ) {
			float fMX = tMiter.fX - tPoint.fX;
			float fMY = tMiter.fY - tPoint.fY;
			float fMLen = __xgePathSqrt((fMX * fMX) + (fMY * fMY));

			if ( fMLen <= (fHalf * 4.0f) ) {
				__xgePathStrokeWriteMiterJoin(pVertices, pIndices, pVertexWrite, pIndexWrite, tPoint, tOuterA, tOuterB, tMiter, iColor);
				return;
			}
		}
	}
	__xgePathStrokeWriteBevelJoin(pVertices, pIndices, pVertexWrite, pIndexWrite, tPoint, tOuterA, tOuterB, iColor);
}

static int __xgePathDashPatternValid(const float* pDashPattern, int iDashCount, float* pTotal)
{
	float fTotal;
	int i;

	if ( pTotal != NULL ) {
		*pTotal = 0.0f;
	}
	if ( (pDashPattern == NULL) || (iDashCount <= 0) ) {
		return 1;
	}
	fTotal = 0.0f;
	for ( i = 0; i < iDashCount; i++ ) {
		if ( pDashPattern[i] <= 0.0f ) {
			return 0;
		}
		fTotal += pDashPattern[i];
	}
	if ( fTotal <= 0.0f ) {
		return 0;
	}
	if ( pTotal != NULL ) {
		*pTotal = fTotal;
	}
	return 1;
}

static void __xgePathDashAdvance(const float* pDashPattern, int iDashCount, int* pDashIndex, float* pDashRemaining, int* pDashOn, float fAmount)
{
	while ( fAmount > 0.000001f ) {
		if ( fAmount < *pDashRemaining ) {
			*pDashRemaining -= fAmount;
			return;
		}
		fAmount -= *pDashRemaining;
		*pDashIndex = (*pDashIndex + 1) % iDashCount;
		*pDashRemaining = pDashPattern[*pDashIndex];
		*pDashOn = ((*pDashIndex % 2) == 0);
	}
}

static int __xgePathBuildDashedStrokeMeshWithStyle(xge_path pPath, xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, float fWidth, uint32_t iColor, int iLineJoin, int iLineCap, const float* pDashPattern, int iDashCount, float fDashOffset, float fTolerance, int* pVertexCount, int* pIndexCount)
{
	xge_vec2_t* pPoints;
	float fHalf;
	float fDashTotal;
	float fDashRemaining;
	int iPointCount;
	int iSegmentCount;
	int iJoinCount;
	int iVertexWrite;
	int iIndexWrite;
	int iVertexCount;
	int iIndexCount;
	int iDashIndex;
	int bDashOn;
	int bDashed;
	int i;

	if ( !__xgePathValid(pPath) || (pVertexCount == NULL) || (pIndexCount == NULL) ||
	     (fWidth <= 0.0f) || (iVertexCapacity < 0) || (iIndexCapacity < 0) ||
	     ((pVertices == NULL) && (iVertexCapacity > 0)) || ((pIndices == NULL) && (iIndexCapacity > 0)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iLineCap != XGE_PATH_CAP_BUTT) && (iLineCap != XGE_PATH_CAP_SQUARE) && (iLineCap != XGE_PATH_CAP_ROUND) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iLineJoin != XGE_PATH_JOIN_MITER) && (iLineJoin != XGE_PATH_JOIN_BEVEL) && (iLineJoin != XGE_PATH_JOIN_ROUND) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgePathDashPatternValid(pDashPattern, iDashCount, &fDashTotal) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pVertexCount = 0;
	*pIndexCount = 0;
	iPointCount = xgePathFlatten(pPath, NULL, 0, fTolerance);
	if ( iPointCount < 2 ) {
		return XGE_OK;
	}
	pPoints = (xge_vec2_t*)xrtMalloc(sizeof(*pPoints) * (size_t)iPointCount);
	if ( pPoints == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	(void)xgePathFlatten(pPath, pPoints, iPointCount, fTolerance);
	bDashed = (pDashPattern != NULL) && (iDashCount > 0);
	iDashIndex = 0;
	fDashRemaining = bDashed ? pDashPattern[0] : 0.0f;
	bDashOn = 1;
	if ( bDashed && (fDashOffset > 0.0f) ) {
		while ( fDashOffset >= fDashTotal ) {
			fDashOffset -= fDashTotal;
		}
		__xgePathDashAdvance(pDashPattern, iDashCount, &iDashIndex, &fDashRemaining, &bDashOn, fDashOffset);
	}
	iSegmentCount = 0;
	for ( i = 1; i < iPointCount; i++ ) {
		float fDX = pPoints[i].fX - pPoints[i - 1].fX;
		float fDY = pPoints[i].fY - pPoints[i - 1].fY;
		float fLen = __xgePathSqrt(fDX * fDX + fDY * fDY);

		if ( fLen <= 0.000001f ) {
			continue;
		}
		if ( !bDashed ) {
			iSegmentCount++;
		} else {
			float fRemain = fLen;

			while ( fRemain > 0.000001f ) {
				float fStep = (fRemain < fDashRemaining) ? fRemain : fDashRemaining;

				if ( bDashOn ) {
					iSegmentCount++;
				}
				__xgePathDashAdvance(pDashPattern, iDashCount, &iDashIndex, &fDashRemaining, &bDashOn, fStep);
				fRemain -= fStep;
			}
		}
	}
	if ( iSegmentCount <= 0 ) {
		xrtFree(pPoints);
		return XGE_OK;
	}
	iJoinCount = 0;
	if ( !bDashed ) {
		for ( i = 1; i < (iPointCount - 1); i++ ) {
			float fD0X = pPoints[i].fX - pPoints[i - 1].fX;
			float fD0Y = pPoints[i].fY - pPoints[i - 1].fY;
			float fD1X = pPoints[i + 1].fX - pPoints[i].fX;
			float fD1Y = pPoints[i + 1].fY - pPoints[i].fY;
			float fLen0 = __xgePathSqrt((fD0X * fD0X) + (fD0Y * fD0Y));
			float fLen1 = __xgePathSqrt((fD1X * fD1X) + (fD1Y * fD1Y));

			if ( (fLen0 > 0.000001f) && (fLen1 > 0.000001f) ) {
				float fCross;

				fD0X /= fLen0;
				fD0Y /= fLen0;
				fD1X /= fLen1;
				fD1Y /= fLen1;
				fCross = (fD0X * fD1Y) - (fD0Y * fD1X);
				if ( (fCross > -0.000001f) && (fCross < 0.000001f) ) {
					continue;
				}
				iJoinCount++;
			}
		}
	}
	iVertexCount = (iSegmentCount * __xgePathStrokeSegmentVertexCount(iLineCap)) + (iJoinCount * __xgePathStrokeJoinVertexCount(iLineJoin));
	iIndexCount = (iSegmentCount * __xgePathStrokeSegmentIndexCount(iLineCap)) + (iJoinCount * __xgePathStrokeJoinIndexCount(iLineJoin));
	*pVertexCount = iVertexCount;
	*pIndexCount = iIndexCount;
	if ( (pVertices == NULL) && (iVertexCapacity == 0) && (pIndices == NULL) && (iIndexCapacity == 0) ) {
		xrtFree(pPoints);
		return XGE_OK;
	}
	if ( (iVertexCapacity < iVertexCount) || (iIndexCapacity < iIndexCount) ) {
		xrtFree(pPoints);
		return XGE_ERROR_BUFFER_TOO_SMALL;
	}
	fHalf = fWidth * 0.5f;
	iVertexWrite = 0;
	iIndexWrite = 0;
	iDashIndex = 0;
	fDashRemaining = bDashed ? pDashPattern[0] : 0.0f;
	bDashOn = 1;
	if ( bDashed && (fDashOffset > 0.0f) ) {
		while ( fDashOffset >= fDashTotal ) {
			fDashOffset -= fDashTotal;
		}
		__xgePathDashAdvance(pDashPattern, iDashCount, &iDashIndex, &fDashRemaining, &bDashOn, fDashOffset);
	}
	iSegmentCount = 0;
	for ( i = 1; i < iPointCount; i++ ) {
		xge_vec2_t tA = pPoints[i - 1];
		xge_vec2_t tB = pPoints[i];
		float fDX = tB.fX - tA.fX;
		float fDY = tB.fY - tA.fY;
		float fLen = __xgePathSqrt(fDX * fDX + fDY * fDY);

		if ( fLen <= 0.000001f ) {
			continue;
		}
		if ( !bDashed ) {
			__xgePathStrokeWriteSegment(pVertices, pIndices, &iVertexWrite, &iIndexWrite, tA, tB, fHalf, iColor, iLineCap);
			iSegmentCount++;
		} else {
			float fDone = 0.0f;
			float fRemain = fLen;

			while ( fRemain > 0.000001f ) {
				float fStep = (fRemain < fDashRemaining) ? fRemain : fDashRemaining;

				if ( bDashOn ) {
					float fT0 = fDone / fLen;
					float fT1 = (fDone + fStep) / fLen;
					xge_vec2_t tS = {tA.fX + fDX * fT0, tA.fY + fDY * fT0};
					xge_vec2_t tE = {tA.fX + fDX * fT1, tA.fY + fDY * fT1};

					__xgePathStrokeWriteSegment(pVertices, pIndices, &iVertexWrite, &iIndexWrite, tS, tE, fHalf, iColor, iLineCap);
					iSegmentCount++;
				}
				__xgePathDashAdvance(pDashPattern, iDashCount, &iDashIndex, &fDashRemaining, &bDashOn, fStep);
				fDone += fStep;
				fRemain -= fStep;
			}
		}
	}
	if ( !bDashed ) {
		for ( i = 1; i < (iPointCount - 1); i++ ) {
			__xgePathStrokeWriteJoin(pVertices, pIndices, &iVertexWrite, &iIndexWrite, pPoints[i - 1], pPoints[i], pPoints[i + 1], fHalf, iColor, iLineJoin);
		}
	}
	*pVertexCount = iVertexWrite;
	*pIndexCount = iIndexWrite;
	xrtFree(pPoints);
	return XGE_OK;
}

int xgePathBuildDashedStrokeMesh(xge_path pPath, xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, float fWidth, uint32_t iColor, const float* pDashPattern, int iDashCount, float fDashOffset, float fTolerance, int* pVertexCount, int* pIndexCount)
{
	return __xgePathBuildDashedStrokeMeshWithStyle(pPath, pVertices, iVertexCapacity, pIndices, iIndexCapacity, fWidth, iColor, XGE_PATH_JOIN_MITER, XGE_PATH_CAP_BUTT, pDashPattern, iDashCount, fDashOffset, fTolerance, pVertexCount, pIndexCount);
}

int xgePathBuildStrokeMesh(xge_path pPath, xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, float fWidth, uint32_t iColor, float fTolerance, int* pVertexCount, int* pIndexCount)
{
	return xgePathBuildDashedStrokeMesh(pPath, pVertices, iVertexCapacity, pIndices, iIndexCapacity, fWidth, iColor, NULL, 0, 0.0f, fTolerance, pVertexCount, pIndexCount);
}

static int __xgePathEmitStrokeAAQuad(xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, int* pVertexCount, int* pIndexCount, xge_vec2_t tInnerA, xge_vec2_t tInnerB, xge_vec2_t tOuterB, xge_vec2_t tOuterA, uint32_t iInnerColor)
{
	int iBase;

	iBase = *pVertexCount;
	*pVertexCount += 4;
	*pIndexCount += 6;
	if ( (pVertices == NULL) && (pIndices == NULL) ) {
		return XGE_OK;
	}
	if ( (iBase + 4 > iVertexCapacity) || (*pIndexCount > iIndexCapacity) ) {
		return XGE_ERROR_BUFFER_TOO_SMALL;
	}
	pVertices[iBase + 0] = (xge_shape_vertex_t){tInnerA.fX, tInnerA.fY, iInnerColor};
	pVertices[iBase + 1] = (xge_shape_vertex_t){tInnerB.fX, tInnerB.fY, iInnerColor};
	pVertices[iBase + 2] = (xge_shape_vertex_t){tOuterB.fX, tOuterB.fY, 0x00000000u};
	pVertices[iBase + 3] = (xge_shape_vertex_t){tOuterA.fX, tOuterA.fY, 0x00000000u};
	pIndices[*pIndexCount - 6] = (uint32_t)(iBase + 0);
	pIndices[*pIndexCount - 5] = (uint32_t)(iBase + 1);
	pIndices[*pIndexCount - 4] = (uint32_t)(iBase + 2);
	pIndices[*pIndexCount - 3] = (uint32_t)(iBase + 0);
	pIndices[*pIndexCount - 2] = (uint32_t)(iBase + 2);
	pIndices[*pIndexCount - 1] = (uint32_t)(iBase + 3);
	return XGE_OK;
}

static int __xgePathEmitStrokeAARoundCap(xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, int* pVertexCount, int* pIndexCount, xge_vec2_t tCenter, float fUX, float fUY, float fNX, float fNY, float fHalf, float fFringeWidth, int bEndCap, uint32_t iInnerColor)
{
	int i;
	int iRet;

	for ( i = 0; i < XGE_PATH_ROUND_CAP_SEGMENTS; i++ ) {
		float fT0 = (float)i / (float)XGE_PATH_ROUND_CAP_SEGMENTS;
		float fT1 = (float)(i + 1) / (float)XGE_PATH_ROUND_CAP_SEGMENTS;
		float fAngle0 = fT0 * 3.14159265358979323846f;
		float fAngle1 = fT1 * 3.14159265358979323846f;
		float fC0 = cosf(fAngle0);
		float fS0 = sinf(fAngle0);
		float fC1 = cosf(fAngle1);
		float fS1 = sinf(fAngle1);
		float fOuter = fHalf + fFringeWidth;
		xge_vec2_t tInnerA;
		xge_vec2_t tInnerB;
		xge_vec2_t tOuterA;
		xge_vec2_t tOuterB;

		if ( bEndCap ) {
			tInnerA = (xge_vec2_t){tCenter.fX - (fNX * fC0 * fHalf) + (fUX * fS0 * fHalf), tCenter.fY - (fNY * fC0 * fHalf) + (fUY * fS0 * fHalf)};
			tInnerB = (xge_vec2_t){tCenter.fX - (fNX * fC1 * fHalf) + (fUX * fS1 * fHalf), tCenter.fY - (fNY * fC1 * fHalf) + (fUY * fS1 * fHalf)};
			tOuterA = (xge_vec2_t){tCenter.fX - (fNX * fC0 * fOuter) + (fUX * fS0 * fOuter), tCenter.fY - (fNY * fC0 * fOuter) + (fUY * fS0 * fOuter)};
			tOuterB = (xge_vec2_t){tCenter.fX - (fNX * fC1 * fOuter) + (fUX * fS1 * fOuter), tCenter.fY - (fNY * fC1 * fOuter) + (fUY * fS1 * fOuter)};
		} else {
			tInnerA = (xge_vec2_t){tCenter.fX + (fNX * fC0 * fHalf) - (fUX * fS0 * fHalf), tCenter.fY + (fNY * fC0 * fHalf) - (fUY * fS0 * fHalf)};
			tInnerB = (xge_vec2_t){tCenter.fX + (fNX * fC1 * fHalf) - (fUX * fS1 * fHalf), tCenter.fY + (fNY * fC1 * fHalf) - (fUY * fS1 * fHalf)};
			tOuterA = (xge_vec2_t){tCenter.fX + (fNX * fC0 * fOuter) - (fUX * fS0 * fOuter), tCenter.fY + (fNY * fC0 * fOuter) - (fUY * fS0 * fOuter)};
			tOuterB = (xge_vec2_t){tCenter.fX + (fNX * fC1 * fOuter) - (fUX * fS1 * fOuter), tCenter.fY + (fNY * fC1 * fOuter) - (fUY * fS1 * fOuter)};
		}
		iRet = __xgePathEmitStrokeAAQuad(pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount, tInnerA, tInnerB, tOuterB, tOuterA, iInnerColor);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgePathEmitStrokeAAForSegment(xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, int* pVertexCount, int* pIndexCount, xge_vec2_t tA, xge_vec2_t tB, float fHalf, uint32_t iInnerColor, int iLineCap, int bEmitCaps, float fFringeWidth)
{
	float fDX = tB.fX - tA.fX;
	float fDY = tB.fY - tA.fY;
	float fLen = __xgePathSqrt(fDX * fDX + fDY * fDY);
	float fUX;
	float fUY;
	float fNX;
	float fNY;
	xge_vec2_t tStart;
	xge_vec2_t tEnd;
	int iRet;

	if ( fLen <= 0.000001f ) {
		return XGE_OK;
	}
	fUX = fDX / fLen;
	fUY = fDY / fLen;
	fNX = -fDY / fLen;
	fNY = fDX / fLen;
	tStart = tA;
	tEnd = tB;
	if ( iLineCap == XGE_PATH_CAP_SQUARE ) {
		tStart.fX -= fUX * fHalf;
		tStart.fY -= fUY * fHalf;
		tEnd.fX += fUX * fHalf;
		tEnd.fY += fUY * fHalf;
	}

	iRet = __xgePathEmitStrokeAAQuad(
		pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount,
		(xge_vec2_t){tStart.fX + (fNX * fHalf), tStart.fY + (fNY * fHalf)},
		(xge_vec2_t){tEnd.fX + (fNX * fHalf), tEnd.fY + (fNY * fHalf)},
		(xge_vec2_t){tEnd.fX + (fNX * (fHalf + fFringeWidth)), tEnd.fY + (fNY * (fHalf + fFringeWidth))},
		(xge_vec2_t){tStart.fX + (fNX * (fHalf + fFringeWidth)), tStart.fY + (fNY * (fHalf + fFringeWidth))},
		iInnerColor);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgePathEmitStrokeAAQuad(
		pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount,
		(xge_vec2_t){tEnd.fX - (fNX * fHalf), tEnd.fY - (fNY * fHalf)},
		(xge_vec2_t){tStart.fX - (fNX * fHalf), tStart.fY - (fNY * fHalf)},
		(xge_vec2_t){tStart.fX - (fNX * (fHalf + fFringeWidth)), tStart.fY - (fNY * (fHalf + fFringeWidth))},
		(xge_vec2_t){tEnd.fX - (fNX * (fHalf + fFringeWidth)), tEnd.fY - (fNY * (fHalf + fFringeWidth))},
		iInnerColor);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( !bEmitCaps ) {
		return XGE_OK;
	}
	if ( iLineCap == XGE_PATH_CAP_ROUND ) {
		iRet = __xgePathEmitStrokeAARoundCap(pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount, tStart, fUX, fUY, fNX, fNY, fHalf, fFringeWidth, 0, iInnerColor);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		return __xgePathEmitStrokeAARoundCap(pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount, tEnd, fUX, fUY, fNX, fNY, fHalf, fFringeWidth, 1, iInnerColor);
	}
	iRet = __xgePathEmitStrokeAAQuad(
		pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount,
		(xge_vec2_t){tStart.fX - (fNX * fHalf), tStart.fY - (fNY * fHalf)},
		(xge_vec2_t){tStart.fX + (fNX * fHalf), tStart.fY + (fNY * fHalf)},
		(xge_vec2_t){tStart.fX + (fNX * fHalf) - (fUX * fFringeWidth), tStart.fY + (fNY * fHalf) - (fUY * fFringeWidth)},
		(xge_vec2_t){tStart.fX - (fNX * fHalf) - (fUX * fFringeWidth), tStart.fY - (fNY * fHalf) - (fUY * fFringeWidth)},
		iInnerColor);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	return __xgePathEmitStrokeAAQuad(
		pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount,
		(xge_vec2_t){tEnd.fX + (fNX * fHalf), tEnd.fY + (fNY * fHalf)},
		(xge_vec2_t){tEnd.fX - (fNX * fHalf), tEnd.fY - (fNY * fHalf)},
		(xge_vec2_t){tEnd.fX - (fNX * fHalf) + (fUX * fFringeWidth), tEnd.fY - (fNY * fHalf) + (fUY * fFringeWidth)},
		(xge_vec2_t){tEnd.fX + (fNX * fHalf) + (fUX * fFringeWidth), tEnd.fY + (fNY * fHalf) + (fUY * fFringeWidth)},
		iInnerColor);
}

static int __xgePathEmitStrokeAACapsForSegment(xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, int* pVertexCount, int* pIndexCount, xge_vec2_t tA, xge_vec2_t tB, float fHalf, uint32_t iInnerColor, int iLineCap, float fFringeWidth)
{
	float fDX = tB.fX - tA.fX;
	float fDY = tB.fY - tA.fY;
	float fLen = __xgePathSqrt(fDX * fDX + fDY * fDY);
	float fUX;
	float fUY;
	float fNX;
	float fNY;
	xge_vec2_t tStart;
	xge_vec2_t tEnd;
	int iRet;

	if ( fLen <= 0.000001f ) {
		return XGE_OK;
	}
	fUX = fDX / fLen;
	fUY = fDY / fLen;
	fNX = -fDY / fLen;
	fNY = fDX / fLen;
	tStart = tA;
	tEnd = tB;
	if ( iLineCap == XGE_PATH_CAP_SQUARE ) {
		tStart.fX -= fUX * fHalf;
		tStart.fY -= fUY * fHalf;
		tEnd.fX += fUX * fHalf;
		tEnd.fY += fUY * fHalf;
	}
	if ( iLineCap == XGE_PATH_CAP_ROUND ) {
		iRet = __xgePathEmitStrokeAARoundCap(pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount, tStart, fUX, fUY, fNX, fNY, fHalf, fFringeWidth, 0, iInnerColor);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		return __xgePathEmitStrokeAARoundCap(pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount, tEnd, fUX, fUY, fNX, fNY, fHalf, fFringeWidth, 1, iInnerColor);
	}
	iRet = __xgePathEmitStrokeAAQuad(
		pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount,
		(xge_vec2_t){tStart.fX - (fNX * fHalf), tStart.fY - (fNY * fHalf)},
		(xge_vec2_t){tStart.fX + (fNX * fHalf), tStart.fY + (fNY * fHalf)},
		(xge_vec2_t){tStart.fX + (fNX * fHalf) - (fUX * fFringeWidth), tStart.fY + (fNY * fHalf) - (fUY * fFringeWidth)},
		(xge_vec2_t){tStart.fX - (fNX * fHalf) - (fUX * fFringeWidth), tStart.fY - (fNY * fHalf) - (fUY * fFringeWidth)},
		iInnerColor);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	return __xgePathEmitStrokeAAQuad(
		pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount,
		(xge_vec2_t){tEnd.fX + (fNX * fHalf), tEnd.fY + (fNY * fHalf)},
		(xge_vec2_t){tEnd.fX - (fNX * fHalf), tEnd.fY - (fNY * fHalf)},
		(xge_vec2_t){tEnd.fX - (fNX * fHalf) + (fUX * fFringeWidth), tEnd.fY - (fNY * fHalf) + (fUY * fFringeWidth)},
		(xge_vec2_t){tEnd.fX + (fNX * fHalf) + (fUX * fFringeWidth), tEnd.fY + (fNY * fHalf) + (fUY * fFringeWidth)},
		iInnerColor);
}

static int __xgePathEmitStrokeAAEdgeAway(xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, int* pVertexCount, int* pIndexCount, xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tAwayFrom, float fFringeWidth, uint32_t iInnerColor)
{
	float fDX = tB.fX - tA.fX;
	float fDY = tB.fY - tA.fY;
	float fLen = __xgePathSqrt(fDX * fDX + fDY * fDY);
	float fNX;
	float fNY;
	float fMidX;
	float fMidY;
	float fPlusX;
	float fPlusY;
	float fMinusX;
	float fMinusY;

	if ( fLen <= 0.000001f ) {
		return XGE_OK;
	}
	fNX = -fDY / fLen;
	fNY = fDX / fLen;
	fMidX = (tA.fX + tB.fX) * 0.5f;
	fMidY = (tA.fY + tB.fY) * 0.5f;
	fPlusX = fMidX + fNX - tAwayFrom.fX;
	fPlusY = fMidY + fNY - tAwayFrom.fY;
	fMinusX = fMidX - fNX - tAwayFrom.fX;
	fMinusY = fMidY - fNY - tAwayFrom.fY;
	if ( (fPlusX * fPlusX + fPlusY * fPlusY) < (fMinusX * fMinusX + fMinusY * fMinusY) ) {
		fNX = -fNX;
		fNY = -fNY;
	}
	return __xgePathEmitStrokeAAQuad(
		pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount,
		tA,
		tB,
		(xge_vec2_t){tB.fX + (fNX * fFringeWidth), tB.fY + (fNY * fFringeWidth)},
		(xge_vec2_t){tA.fX + (fNX * fFringeWidth), tA.fY + (fNY * fFringeWidth)},
		iInnerColor);
}

static int __xgePathEmitStrokeAARoundJoin(xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, int* pVertexCount, int* pIndexCount, xge_vec2_t tPoint, float fHalf, float fStartAngle, float fEndAngle, float fFringeWidth, uint32_t iInnerColor)
{
	float fOuter;
	int i;
	int iRet;

	fOuter = fHalf + fFringeWidth;
	for ( i = 0; i < XGE_PATH_ROUND_JOIN_SEGMENTS; i++ ) {
		float fT0 = (float)i / (float)XGE_PATH_ROUND_JOIN_SEGMENTS;
		float fT1 = (float)(i + 1) / (float)XGE_PATH_ROUND_JOIN_SEGMENTS;
		float fAngle0 = fStartAngle + ((fEndAngle - fStartAngle) * fT0);
		float fAngle1 = fStartAngle + ((fEndAngle - fStartAngle) * fT1);
		xge_vec2_t tInnerA = {tPoint.fX + cosf(fAngle0) * fHalf, tPoint.fY + sinf(fAngle0) * fHalf};
		xge_vec2_t tInnerB = {tPoint.fX + cosf(fAngle1) * fHalf, tPoint.fY + sinf(fAngle1) * fHalf};
		xge_vec2_t tOuterA = {tPoint.fX + cosf(fAngle0) * fOuter, tPoint.fY + sinf(fAngle0) * fOuter};
		xge_vec2_t tOuterB = {tPoint.fX + cosf(fAngle1) * fOuter, tPoint.fY + sinf(fAngle1) * fOuter};

		iRet = __xgePathEmitStrokeAAQuad(pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount, tInnerA, tInnerB, tOuterB, tOuterA, iInnerColor);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgePathEmitStrokeAAJoin(xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, int* pVertexCount, int* pIndexCount, xge_vec2_t tPrev, xge_vec2_t tPoint, xge_vec2_t tNext, float fHalf, uint32_t iInnerColor, int iLineJoin, float fFringeWidth)
{
	float fD0X;
	float fD0Y;
	float fD1X;
	float fD1Y;
	float fLen0;
	float fLen1;
	float fN0X;
	float fN0Y;
	float fN1X;
	float fN1Y;
	float fCross;
	float fSide;
	xge_vec2_t tOuterA;
	xge_vec2_t tOuterB;
	xge_vec2_t tMiter;

	fD0X = tPoint.fX - tPrev.fX;
	fD0Y = tPoint.fY - tPrev.fY;
	fD1X = tNext.fX - tPoint.fX;
	fD1Y = tNext.fY - tPoint.fY;
	fLen0 = __xgePathSqrt((fD0X * fD0X) + (fD0Y * fD0Y));
	fLen1 = __xgePathSqrt((fD1X * fD1X) + (fD1Y * fD1Y));
	if ( (fLen0 <= 0.000001f) || (fLen1 <= 0.000001f) ) {
		return XGE_OK;
	}
	fD0X /= fLen0;
	fD0Y /= fLen0;
	fD1X /= fLen1;
	fD1Y /= fLen1;
	fCross = (fD0X * fD1Y) - (fD0Y * fD1X);
	if ( (fCross > -0.000001f) && (fCross < 0.000001f) ) {
		return XGE_OK;
	}
	fN0X = -fD0Y;
	fN0Y = fD0X;
	fN1X = -fD1Y;
	fN1Y = fD1X;
	fSide = (fCross > 0.0f) ? -1.0f : 1.0f;
	tOuterA = (xge_vec2_t){tPoint.fX + (fN0X * fHalf * fSide), tPoint.fY + (fN0Y * fHalf * fSide)};
	tOuterB = (xge_vec2_t){tPoint.fX + (fN1X * fHalf * fSide), tPoint.fY + (fN1Y * fHalf * fSide)};
	if ( iLineJoin == XGE_PATH_JOIN_ROUND ) {
		float fStartAngle = atan2f(tOuterA.fY - tPoint.fY, tOuterA.fX - tPoint.fX);
		float fEndAngle = atan2f(tOuterB.fY - tPoint.fY, tOuterB.fX - tPoint.fX);

		if ( fCross > 0.0f ) {
			while ( fEndAngle > fStartAngle ) fEndAngle -= 6.28318530717958647692f;
		} else {
			while ( fEndAngle < fStartAngle ) fEndAngle += 6.28318530717958647692f;
		}
		return __xgePathEmitStrokeAARoundJoin(pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount, tPoint, fHalf, fStartAngle, fEndAngle, fFringeWidth, iInnerColor);
	}
	if ( iLineJoin == XGE_PATH_JOIN_MITER ) {
		xge_vec2_t tLineA = {tPoint.fX + (fN0X * fHalf * fSide), tPoint.fY + (fN0Y * fHalf * fSide)};
		xge_vec2_t tLineB = {tPoint.fX + (fN1X * fHalf * fSide), tPoint.fY + (fN1Y * fHalf * fSide)};

		if ( __xgePathStrokeLineIntersection(tLineA, fD0X, fD0Y, tLineB, fD1X, fD1Y, &tMiter) ) {
			float fMX = tMiter.fX - tPoint.fX;
			float fMY = tMiter.fY - tPoint.fY;
			float fMLen = __xgePathSqrt((fMX * fMX) + (fMY * fMY));

			if ( fMLen <= (fHalf * 4.0f) ) {
				int iRet = __xgePathEmitStrokeAAEdgeAway(pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount, tOuterA, tMiter, tPoint, fFringeWidth, iInnerColor);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
				return __xgePathEmitStrokeAAEdgeAway(pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount, tMiter, tOuterB, tPoint, fFringeWidth, iInnerColor);
			}
		}
	}
	return __xgePathEmitStrokeAAEdgeAway(pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount, tOuterA, tOuterB, tPoint, fFringeWidth, iInnerColor);
}

static int __xgePathBuildDashedStrokeAAMeshWithStyle(xge_path pPath, xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, float fWidth, uint32_t iColor, int iLineJoin, int iLineCap, const float* pDashPattern, int iDashCount, float fDashOffset, float fTolerance, float fFringeWidth, int* pVertexCount, int* pIndexCount)
{
	xge_path_flatten_data_t tFlat;
	uint32_t iInnerColor;
	float fHalf;
	float fDashTotal;
	int bDashed;
	int i;
	int iRet;

	if ( !__xgePathValid(pPath) || (pVertexCount == NULL) || (pIndexCount == NULL) ||
	     (fWidth <= 0.0f) || (fFringeWidth <= 0.0f) || (iVertexCapacity < 0) || (iIndexCapacity < 0) ||
	     ((pVertices == NULL) != (pIndices == NULL)) ||
	     ((pVertices == NULL) && ((iVertexCapacity > 0) || (iIndexCapacity > 0))) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iLineCap != XGE_PATH_CAP_BUTT) && (iLineCap != XGE_PATH_CAP_SQUARE) && (iLineCap != XGE_PATH_CAP_ROUND) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iLineJoin != XGE_PATH_JOIN_MITER) && (iLineJoin != XGE_PATH_JOIN_BEVEL) && (iLineJoin != XGE_PATH_JOIN_ROUND) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgePathDashPatternValid(pDashPattern, iDashCount, &fDashTotal) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pVertexCount = 0;
	*pIndexCount = 0;
	if ( XGE_COLOR_GET_A(iColor) == 0u ) {
		return XGE_OK;
	}
	memset(&tFlat, 0, sizeof(tFlat));
	iRet = __xgePathFlattenToContours(pPath, fTolerance, &tFlat);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iInnerColor = __xgePathColorPremulAlphaScale(iColor, 1.0f);
	fHalf = fWidth * 0.5f;
	bDashed = (pDashPattern != NULL) && (iDashCount > 0);
	for ( i = 0; (iRet == XGE_OK) && (i < tFlat.iContourCount); i++ ) {
		const xge_path_flat_contour_t* pContour = &tFlat.pContours[i];
		int iCount = pContour->iCount;
		int iSegmentCount;
		int j;
		float fDashRemaining;
		int iDashIndex;
		int bDashOn;

		while ( (iCount > 1) &&
		        (fabsf(tFlat.pPoints[pContour->iStart + iCount - 1].fX - tFlat.pPoints[pContour->iStart].fX) <= XGE_PATH_EPSILON) &&
		        (fabsf(tFlat.pPoints[pContour->iStart + iCount - 1].fY - tFlat.pPoints[pContour->iStart].fY) <= XGE_PATH_EPSILON) ) {
			iCount--;
		}
		if ( iCount < 2 ) {
			continue;
		}
		iSegmentCount = pContour->bClosed ? iCount : (iCount - 1);
		iDashIndex = 0;
		fDashRemaining = bDashed ? pDashPattern[0] : 0.0f;
		bDashOn = 1;
		if ( bDashed && (fDashOffset > 0.0f) ) {
			float fOffset = fDashOffset;

			while ( fOffset >= fDashTotal ) {
				fOffset -= fDashTotal;
			}
			__xgePathDashAdvance(pDashPattern, iDashCount, &iDashIndex, &fDashRemaining, &bDashOn, fOffset);
		}
		for ( j = 0; (iRet == XGE_OK) && (j < iSegmentCount); j++ ) {
			xge_vec2_t tA = tFlat.pPoints[pContour->iStart + j];
			xge_vec2_t tB = tFlat.pPoints[pContour->iStart + ((j + 1) % iCount)];
			float fDX = tB.fX - tA.fX;
			float fDY = tB.fY - tA.fY;
			float fLen = __xgePathSqrt(fDX * fDX + fDY * fDY);

			if ( fLen <= 0.000001f ) {
				continue;
			}
			if ( !bDashed ) {
				iRet = __xgePathEmitStrokeAAForSegment(pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount, tA, tB, fHalf, iInnerColor, XGE_PATH_CAP_BUTT, 0, fFringeWidth);
			} else {
				float fDone = 0.0f;
				float fRemain = fLen;

				while ( (iRet == XGE_OK) && (fRemain > 0.000001f) ) {
					float fStep = (fRemain < fDashRemaining) ? fRemain : fDashRemaining;

					if ( bDashOn ) {
						float fT0 = fDone / fLen;
						float fT1 = (fDone + fStep) / fLen;
						xge_vec2_t tS = {tA.fX + fDX * fT0, tA.fY + fDY * fT0};
						xge_vec2_t tE = {tA.fX + fDX * fT1, tA.fY + fDY * fT1};

						iRet = __xgePathEmitStrokeAAForSegment(pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount, tS, tE, fHalf, iInnerColor, iLineCap, 1, fFringeWidth);
					}
					__xgePathDashAdvance(pDashPattern, iDashCount, &iDashIndex, &fDashRemaining, &bDashOn, fStep);
					fDone += fStep;
					fRemain -= fStep;
				}
			}
		}
		if ( (iRet == XGE_OK) && !bDashed ) {
			if ( !pContour->bClosed ) {
				iRet = __xgePathEmitStrokeAACapsForSegment(pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount, tFlat.pPoints[pContour->iStart], tFlat.pPoints[pContour->iStart + iCount - 1], fHalf, iInnerColor, iLineCap, fFringeWidth);
			}
			for ( j = 1; (iRet == XGE_OK) && (j < (iCount - 1)); j++ ) {
				iRet = __xgePathEmitStrokeAAJoin(pVertices, iVertexCapacity, pIndices, iIndexCapacity, pVertexCount, pIndexCount, tFlat.pPoints[pContour->iStart + j - 1], tFlat.pPoints[pContour->iStart + j], tFlat.pPoints[pContour->iStart + j + 1], fHalf, iInnerColor, iLineJoin, fFringeWidth);
			}
		}
	}
	__xgePathFlattenDataFree(&tFlat);
	return iRet;
}

int xgePathBuildDashedStrokeAAMesh(xge_path pPath, xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, float fWidth, uint32_t iColor, const float* pDashPattern, int iDashCount, float fDashOffset, float fTolerance, float fFringeWidth, int* pVertexCount, int* pIndexCount)
{
	return __xgePathBuildDashedStrokeAAMeshWithStyle(pPath, pVertices, iVertexCapacity, pIndices, iIndexCapacity, fWidth, iColor, XGE_PATH_JOIN_MITER, XGE_PATH_CAP_BUTT, pDashPattern, iDashCount, fDashOffset, fTolerance, fFringeWidth, pVertexCount, pIndexCount);
}

int xgePathBuildStrokeAAMesh(xge_path pPath, xge_shape_vertex_t* pVertices, int iVertexCapacity, uint32_t* pIndices, int iIndexCapacity, float fWidth, uint32_t iColor, float fTolerance, float fFringeWidth, int* pVertexCount, int* pIndexCount)
{
	return xgePathBuildDashedStrokeAAMesh(pPath, pVertices, iVertexCapacity, pIndices, iIndexCapacity, fWidth, iColor, NULL, 0, 0.0f, fTolerance, fFringeWidth, pVertexCount, pIndexCount);
}

static int __xgePathDrawMesh(xge_path pPath, const xge_path_style_t* pStyle, float fTolerance, int bScreenSpace)
{
	xge_shape_vertex_t* pVertices;
	uint32_t* pIndices;
	int iVertexCount;
	int iIndexCount;
	int iRet;
	int iFillRule;
	int iLineJoin;
	int iLineCap;

	if ( !__xgePathValid(pPath) || (pStyle == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pStyle->iSize != 0u) && (pStyle->iSize < sizeof(*pStyle)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iFillRule = (pStyle->iFillRule == XGE_PATH_FILL_EVEN_ODD) ? XGE_PATH_FILL_EVEN_ODD : XGE_PATH_FILL_NON_ZERO;
	iLineJoin = (pStyle->iLineJoin == XGE_PATH_JOIN_BEVEL || pStyle->iLineJoin == XGE_PATH_JOIN_ROUND) ? pStyle->iLineJoin : XGE_PATH_JOIN_MITER;
	iLineCap = (pStyle->iLineCap == XGE_PATH_CAP_SQUARE || pStyle->iLineCap == XGE_PATH_CAP_ROUND) ? pStyle->iLineCap : XGE_PATH_CAP_BUTT;
	if ( XGE_COLOR_GET_A(pStyle->iFillColor) != 0u ) {
		iVertexCount = 0;
		iIndexCount = 0;
		if ( bScreenSpace ) {
			iRet = __xgePathBuildFillAAMeshWithRule(pPath, NULL, 0, NULL, 0, pStyle->iFillColor, iFillRule, fTolerance, 1.0f, &iVertexCount, &iIndexCount);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			if ( (iVertexCount > 0) && (iIndexCount > 0) ) {
				pVertices = (xge_shape_vertex_t*)xrtMalloc(sizeof(*pVertices) * (size_t)iVertexCount);
				pIndices = (uint32_t*)xrtMalloc(sizeof(*pIndices) * (size_t)iIndexCount);
				if ( (pVertices == NULL) || (pIndices == NULL) ) {
					if ( pVertices != NULL ) xrtFree(pVertices);
					if ( pIndices != NULL ) xrtFree(pIndices);
					return XGE_ERROR_OUT_OF_MEMORY;
				}
				iRet = __xgePathBuildFillAAMeshWithRule(pPath, pVertices, iVertexCount, pIndices, iIndexCount, pStyle->iFillColor, iFillRule, fTolerance, 1.0f, &iVertexCount, &iIndexCount);
				if ( iRet == XGE_OK ) {
					iRet = xgeShapeMeshFillPx(pVertices, iVertexCount, pIndices, iIndexCount);
				}
				xrtFree(pIndices);
				xrtFree(pVertices);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
			}
			iVertexCount = 0;
			iIndexCount = 0;
		}
		iRet = __xgePathBuildFillMeshWithRule(pPath, NULL, 0, NULL, 0, pStyle->iFillColor, iFillRule, fTolerance, &iVertexCount, &iIndexCount);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( (iVertexCount > 0) && (iIndexCount > 0) ) {
			pVertices = (xge_shape_vertex_t*)xrtMalloc(sizeof(*pVertices) * (size_t)iVertexCount);
			pIndices = (uint32_t*)xrtMalloc(sizeof(*pIndices) * (size_t)iIndexCount);
			if ( (pVertices == NULL) || (pIndices == NULL) ) {
				if ( pVertices != NULL ) xrtFree(pVertices);
				if ( pIndices != NULL ) xrtFree(pIndices);
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			iRet = __xgePathBuildFillMeshWithRule(pPath, pVertices, iVertexCount, pIndices, iIndexCount, pStyle->iFillColor, iFillRule, fTolerance, &iVertexCount, &iIndexCount);
			if ( iRet == XGE_OK ) {
				iRet = bScreenSpace ? xgeShapeMeshFillPx(pVertices, iVertexCount, pIndices, iIndexCount) : xgeShapeMeshFill(pVertices, iVertexCount, pIndices, iIndexCount);
			}
			xrtFree(pIndices);
			xrtFree(pVertices);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
	}
	if ( (XGE_COLOR_GET_A(pStyle->iStrokeColor) != 0u) && (pStyle->fStrokeWidth > 0.0f) ) {
		iVertexCount = 0;
		iIndexCount = 0;
		if ( bScreenSpace ) {
			iRet = __xgePathBuildDashedStrokeAAMeshWithStyle(pPath, NULL, 0, NULL, 0, pStyle->fStrokeWidth, pStyle->iStrokeColor, iLineJoin, iLineCap, pStyle->pDashPattern, pStyle->iDashCount, pStyle->fDashOffset, fTolerance, 1.0f, &iVertexCount, &iIndexCount);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			if ( (iVertexCount > 0) && (iIndexCount > 0) ) {
				pVertices = (xge_shape_vertex_t*)xrtMalloc(sizeof(*pVertices) * (size_t)iVertexCount);
				pIndices = (uint32_t*)xrtMalloc(sizeof(*pIndices) * (size_t)iIndexCount);
				if ( (pVertices == NULL) || (pIndices == NULL) ) {
					if ( pVertices != NULL ) xrtFree(pVertices);
					if ( pIndices != NULL ) xrtFree(pIndices);
					return XGE_ERROR_OUT_OF_MEMORY;
				}
				iRet = __xgePathBuildDashedStrokeAAMeshWithStyle(pPath, pVertices, iVertexCount, pIndices, iIndexCount, pStyle->fStrokeWidth, pStyle->iStrokeColor, iLineJoin, iLineCap, pStyle->pDashPattern, pStyle->iDashCount, pStyle->fDashOffset, fTolerance, 1.0f, &iVertexCount, &iIndexCount);
				if ( iRet == XGE_OK ) {
					iRet = xgeShapeMeshFillPx(pVertices, iVertexCount, pIndices, iIndexCount);
				}
				xrtFree(pIndices);
				xrtFree(pVertices);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
			}
			iVertexCount = 0;
			iIndexCount = 0;
		}
		iRet = __xgePathBuildDashedStrokeMeshWithStyle(pPath, NULL, 0, NULL, 0, pStyle->fStrokeWidth, pStyle->iStrokeColor, iLineJoin, iLineCap, pStyle->pDashPattern, pStyle->iDashCount, pStyle->fDashOffset, fTolerance, &iVertexCount, &iIndexCount);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( (iVertexCount > 0) && (iIndexCount > 0) ) {
			pVertices = (xge_shape_vertex_t*)xrtMalloc(sizeof(*pVertices) * (size_t)iVertexCount);
			pIndices = (uint32_t*)xrtMalloc(sizeof(*pIndices) * (size_t)iIndexCount);
			if ( (pVertices == NULL) || (pIndices == NULL) ) {
				if ( pVertices != NULL ) xrtFree(pVertices);
				if ( pIndices != NULL ) xrtFree(pIndices);
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			iRet = __xgePathBuildDashedStrokeMeshWithStyle(pPath, pVertices, iVertexCount, pIndices, iIndexCount, pStyle->fStrokeWidth, pStyle->iStrokeColor, iLineJoin, iLineCap, pStyle->pDashPattern, pStyle->iDashCount, pStyle->fDashOffset, fTolerance, &iVertexCount, &iIndexCount);
			if ( iRet == XGE_OK ) {
				iRet = bScreenSpace ? xgeShapeMeshFillPx(pVertices, iVertexCount, pIndices, iIndexCount) : xgeShapeMeshFill(pVertices, iVertexCount, pIndices, iIndexCount);
			}
			xrtFree(pIndices);
			xrtFree(pVertices);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
	}
	return XGE_OK;
}

int xgePathDraw(xge_path pPath, const xge_path_style_t* pStyle, float fTolerance)
{
	return __xgePathDrawMesh(pPath, pStyle, fTolerance, 0);
}

int xgePathDrawPx(xge_path pPath, const xge_path_style_t* pStyle, float fTolerance)
{
	return __xgePathDrawMesh(pPath, pStyle, fTolerance, 1);
}

static xge_svg_transform_t __xgeSvgTransformIdentity(void)
{
	xge_svg_transform_t tTransform;

	tTransform.fA = 1.0f;
	tTransform.fB = 0.0f;
	tTransform.fC = 0.0f;
	tTransform.fD = 1.0f;
	tTransform.fE = 0.0f;
	tTransform.fF = 0.0f;
	return tTransform;
}

static xge_svg_transform_t __xgeSvgTransformMultiply(xge_svg_transform_t tA, xge_svg_transform_t tB)
{
	xge_svg_transform_t tRet;

	tRet.fA = (tA.fA * tB.fA) + (tA.fC * tB.fB);
	tRet.fB = (tA.fB * tB.fA) + (tA.fD * tB.fB);
	tRet.fC = (tA.fA * tB.fC) + (tA.fC * tB.fD);
	tRet.fD = (tA.fB * tB.fC) + (tA.fD * tB.fD);
	tRet.fE = (tA.fA * tB.fE) + (tA.fC * tB.fF) + tA.fE;
	tRet.fF = (tA.fB * tB.fE) + (tA.fD * tB.fF) + tA.fF;
	return tRet;
}

static xge_vec2_t __xgeSvgTransformPoint(xge_svg_transform_t tTransform, xge_vec2_t tPoint)
{
	xge_vec2_t tRet;

	tRet.fX = (tTransform.fA * tPoint.fX) + (tTransform.fC * tPoint.fY) + tTransform.fE;
	tRet.fY = (tTransform.fB * tPoint.fX) + (tTransform.fD * tPoint.fY) + tTransform.fF;
	return tRet;
}

static xge_rect_t __xgeSvgTransformRectBounds(xge_svg_transform_t tTransform, xge_rect_t tRect)
{
	xge_vec2_t arrPoints[4];
	float fMinX;
	float fMinY;
	float fMaxX;
	float fMaxY;
	int i;

	arrPoints[0] = __xgeSvgTransformPoint(tTransform, (xge_vec2_t){tRect.fX, tRect.fY});
	arrPoints[1] = __xgeSvgTransformPoint(tTransform, (xge_vec2_t){tRect.fX + tRect.fW, tRect.fY});
	arrPoints[2] = __xgeSvgTransformPoint(tTransform, (xge_vec2_t){tRect.fX + tRect.fW, tRect.fY + tRect.fH});
	arrPoints[3] = __xgeSvgTransformPoint(tTransform, (xge_vec2_t){tRect.fX, tRect.fY + tRect.fH});
	fMinX = arrPoints[0].fX;
	fMinY = arrPoints[0].fY;
	fMaxX = arrPoints[0].fX;
	fMaxY = arrPoints[0].fY;
	for ( i = 1; i < 4; i++ ) {
		if ( arrPoints[i].fX < fMinX ) fMinX = arrPoints[i].fX;
		if ( arrPoints[i].fY < fMinY ) fMinY = arrPoints[i].fY;
		if ( arrPoints[i].fX > fMaxX ) fMaxX = arrPoints[i].fX;
		if ( arrPoints[i].fY > fMaxY ) fMaxY = arrPoints[i].fY;
	}
	return (xge_rect_t){fMinX, fMinY, fMaxX - fMinX, fMaxY - fMinY};
}

static int __xgeSvgRectIntersect(xge_rect_t* pRect, xge_rect_t tClip)
{
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;

	if ( pRect == NULL ) {
		return 0;
	}
	fLeft = (pRect->fX > tClip.fX) ? pRect->fX : tClip.fX;
	fTop = (pRect->fY > tClip.fY) ? pRect->fY : tClip.fY;
	fRight = ((pRect->fX + pRect->fW) < (tClip.fX + tClip.fW)) ? (pRect->fX + pRect->fW) : (tClip.fX + tClip.fW);
	fBottom = ((pRect->fY + pRect->fH) < (tClip.fY + tClip.fH)) ? (pRect->fY + pRect->fH) : (tClip.fY + tClip.fH);
	if ( (fRight <= fLeft) || (fBottom <= fTop) ) {
		return 0;
	}
	*pRect = (xge_rect_t){fLeft, fTop, fRight - fLeft, fBottom - fTop};
	return 1;
}

static void __xgeSvgRestoreClip(int bClip, int bOldClip, xge_rect_t tOldClip)
{
	if ( !bClip ) {
		return;
	}
	(void)xgeFlush();
	if ( bOldClip ) {
		xgeClipSet(tOldClip);
	} else {
		xgeClipClear();
	}
}

static void __xgeSvgStyleDefault(xge_path_style_t* pStyle)
{
	memset(pStyle, 0, sizeof(*pStyle));
	pStyle->iSize = (uint32_t)sizeof(*pStyle);
	pStyle->iFillColor = 0x000000ffu;
	pStyle->iStrokeColor = 0x00000000u;
	pStyle->fStrokeWidth = 1.0f;
	pStyle->iFillRule = XGE_PATH_FILL_NON_ZERO;
	pStyle->iLineJoin = XGE_PATH_JOIN_MITER;
	pStyle->iLineCap = XGE_PATH_CAP_BUTT;
}

static void __xgeSvgAspectValuesDefault(int* pAlignX, int* pAlignY, int* pMode)
{
	if ( pAlignX != NULL ) *pAlignX = XGE_SVG_ASPECT_ALIGN_MID;
	if ( pAlignY != NULL ) *pAlignY = XGE_SVG_ASPECT_ALIGN_MID;
	if ( pMode != NULL ) *pMode = XGE_SVG_ASPECT_MEET;
}

static void __xgeSvgAspectDefault(xge_svg pSvg)
{
	if ( pSvg == NULL ) {
		return;
	}
	__xgeSvgAspectValuesDefault(&pSvg->iAspectAlignX, &pSvg->iAspectAlignY, &pSvg->iAspectMode);
}

static void __xgeSvgContextFixDashPointer(xge_svg_parse_context_t* pContext)
{
	if ( pContext->iDashCount > 0 ) {
		pContext->tStyle.pDashPattern = pContext->arrDash;
		pContext->tStyle.iDashCount = pContext->iDashCount;
	} else {
		pContext->tStyle.pDashPattern = NULL;
		pContext->tStyle.iDashCount = 0;
	}
}

static void __xgeSvgContextDefault(xge_svg_parse_context_t* pContext)
{
	memset(pContext, 0, sizeof(*pContext));
	__xgeSvgStyleDefault(&pContext->tStyle);
	pContext->tTransform = __xgeSvgTransformIdentity();
	pContext->fFillOpacity = 1.0f;
	pContext->fStrokeOpacity = 1.0f;
	pContext->fUseWidth = 0.0f;
	pContext->fUseHeight = 0.0f;
	pContext->bUseWidth = 0;
	pContext->bUseHeight = 0;
	__xgeSvgContextFixDashPointer(pContext);
}

static int __xgeSvgStringEqual(const char* sA, int iALen, const char* sB)
{
	int i;

	if ( (sA == NULL) || (sB == NULL) ) {
		return 0;
	}
	for ( i = 0; i < iALen; i++ ) {
		if ( sB[i] == '\0' ) {
			return 0;
		}
		if ( sA[i] != sB[i] ) {
			return 0;
		}
	}
	return sB[iALen] == '\0';
}

static int __xgeSvgStringEqualNoCase(const char* sA, int iALen, const char* sB)
{
	int i;

	if ( (sA == NULL) || (sB == NULL) ) {
		return 0;
	}
	for ( i = 0; i < iALen; i++ ) {
		char a;
		char b;

		if ( sB[i] == '\0' ) {
			return 0;
		}
		a = sA[i];
		b = sB[i];
		if ( (a >= 'A') && (a <= 'Z') ) a = (char)(a - 'A' + 'a');
		if ( (b >= 'A') && (b <= 'Z') ) b = (char)(b - 'A' + 'a');
		if ( a != b ) {
			return 0;
		}
	}
	return sB[iALen] == '\0';
}

static char* __xgeSvgStringDup(const char* sText)
{
	char* sCopy;
	size_t iSize;

	if ( sText == NULL ) {
		return NULL;
	}
	iSize = strlen(sText) + 1u;
	sCopy = (char*)xrtMalloc(iSize);
	if ( sCopy == NULL ) {
		return NULL;
	}
	memcpy(sCopy, sText, iSize);
	return sCopy;
}

static int __xgeSvgCharSpace(char c)
{
	return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n') || (c == '\f');
}

static void __xgeSvgSkipSpaces(const char** ppText, const char* pEnd)
{
	const char* pText;

	pText = *ppText;
	while ( (pText < pEnd) && (__xgeSvgCharSpace(*pText) || (*pText == ',')) ) {
		pText++;
	}
	*ppText = pText;
}

static int __xgeSvgReadFloat(const char** ppText, const char* pEnd, float* pValue)
{
	const char* pText;
	char* pAfter;
	double fValue;

	if ( (ppText == NULL) || (*ppText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	pText = *ppText;
	__xgeSvgSkipSpaces(&pText, pEnd);
	if ( pText >= pEnd ) {
		return 0;
	}
	fValue = strtod(pText, &pAfter);
	if ( pAfter == pText ) {
		return 0;
	}
	if ( pAfter > pEnd ) {
		pAfter = (char*)pEnd;
	}
	*pValue = (float)fValue;
	*ppText = pAfter;
	return 1;
}

static float __xgeSvgClamp01(float fValue)
{
	if ( fValue < 0.0f ) return 0.0f;
	if ( fValue > 1.0f ) return 1.0f;
	return fValue;
}

static int __xgeSvgReadLengthPercent(const char** ppText, const char* pEnd, float* pValue, int* pPercent)
{
	const char* pText;
	float fValue;

	if ( (ppText == NULL) || (*ppText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	pText = *ppText;
	if ( !__xgeSvgReadFloat(&pText, pEnd, &fValue) ) {
		return 0;
	}
	while ( (pText < pEnd) && __xgeSvgCharSpace(*pText) ) pText++;
	if ( (pText < pEnd) && (*pText == '%') ) {
		fValue *= 0.01f;
		pText++;
		if ( pPercent != NULL ) {
			*pPercent = 1;
		}
	} else if ( pPercent != NULL ) {
		*pPercent = 0;
	}
	*pValue = fValue;
	*ppText = pText;
	return 1;
}

static int __xgeSvgAttrLengthPercent(const char* pTag, const char* pTagEnd, const char* sName, float* pValue, int* pPercent)
{
	const char* pText;
	const char* pEnd;
	int iValueLen;

	if ( !__xgeSvgAttrGet(pTag, pTagEnd, sName, &pText, &iValueLen) ) {
		return 0;
	}
	pEnd = pText + iValueLen;
	return __xgeSvgReadLengthPercent(&pText, pEnd, pValue, pPercent);
}

static void __xgeSvgCopyId(char* sDst, int iDstSize, const char* sSrc, int iSrcLen)
{
	int i;

	if ( (sDst == NULL) || (iDstSize <= 0) ) {
		return;
	}
	sDst[0] = '\0';
	if ( sSrc == NULL ) {
		return;
	}
	while ( (iSrcLen > 0) && __xgeSvgCharSpace(*sSrc) ) {
		sSrc++;
		iSrcLen--;
	}
	while ( (iSrcLen > 0) && __xgeSvgCharSpace(sSrc[iSrcLen - 1]) ) {
		iSrcLen--;
	}
	for ( i = 0; (i < iSrcLen) && (i + 1 < iDstSize); i++ ) {
		sDst[i] = sSrc[i];
	}
	sDst[i] = '\0';
}

static int __xgeSvgParseUrlId(const char* sValue, int iValueLen, char* sOut, int iOutSize)
{
	const char* pText;
	const char* pEnd;
	const char* pId;
	int iLen;

	if ( (sValue == NULL) || (sOut == NULL) || (iOutSize <= 0) ) {
		return 0;
	}
	sOut[0] = '\0';
	pText = sValue;
	pEnd = sValue + iValueLen;
	while ( (pText < pEnd) && __xgeSvgCharSpace(*pText) ) pText++;
	if ( (pEnd - pText < 5) ||
	     !((pText[0] == 'u') || (pText[0] == 'U')) ||
	     !((pText[1] == 'r') || (pText[1] == 'R')) ||
	     !((pText[2] == 'l') || (pText[2] == 'L')) ) {
		return 0;
	}
	pText += 3;
	while ( (pText < pEnd) && __xgeSvgCharSpace(*pText) ) pText++;
	if ( (pText >= pEnd) || (*pText != '(') ) {
		return 0;
	}
	pText++;
	while ( (pText < pEnd) && __xgeSvgCharSpace(*pText) ) pText++;
	if ( (pText < pEnd) && ((*pText == '"') || (*pText == '\'')) ) {
		pText++;
	}
	if ( (pText >= pEnd) || (*pText != '#') ) {
		return 0;
	}
	pText++;
	pId = pText;
	while ( (pText < pEnd) && (*pText != ')') && (*pText != '"') && (*pText != '\'') && !__xgeSvgCharSpace(*pText) ) {
		pText++;
	}
	iLen = (int)(pText - pId);
	if ( iLen <= 0 ) {
		return 0;
	}
	__xgeSvgCopyId(sOut, iOutSize, pId, iLen);
	return sOut[0] != '\0';
}

static int __xgeSvgParseHrefId(const char* sValue, int iValueLen, char* sOut, int iOutSize)
{
	const char* pText;
	const char* pEnd;
	const char* pId;
	int iLen;

	if ( (sValue == NULL) || (sOut == NULL) || (iOutSize <= 0) ) {
		return 0;
	}
	sOut[0] = '\0';
	pText = sValue;
	pEnd = sValue + iValueLen;
	while ( (pText < pEnd) && __xgeSvgCharSpace(*pText) ) pText++;
	if ( (pText >= pEnd) || (*pText != '#') ) {
		return 0;
	}
	pText++;
	pId = pText;
	while ( (pText < pEnd) && !__xgeSvgCharSpace(*pText) && (*pText != '"') && (*pText != '\'') && (*pText != ')') ) {
		pText++;
	}
	iLen = (int)(pText - pId);
	if ( iLen <= 0 ) {
		return 0;
	}
	__xgeSvgCopyId(sOut, iOutSize, pId, iLen);
	return sOut[0] != '\0';
}

static int __xgeSvgHexValue(char c)
{
	if ( (c >= '0') && (c <= '9') ) return c - '0';
	if ( (c >= 'a') && (c <= 'f') ) return c - 'a' + 10;
	if ( (c >= 'A') && (c <= 'F') ) return c - 'A' + 10;
	return -1;
}

static uint32_t __xgeSvgColorApplyOpacity(uint32_t iColor, float fOpacity)
{
	uint32_t iAlpha;

	if ( fOpacity < 0.0f ) fOpacity = 0.0f;
	if ( fOpacity > 1.0f ) fOpacity = 1.0f;
	iAlpha = XGE_COLOR_GET_A(iColor);
	iAlpha = (uint32_t)((float)iAlpha * fOpacity + 0.5f);
	if ( iAlpha > 255u ) iAlpha = 255u;
	return (iColor & 0xffffff00u) | iAlpha;
}

static int __xgeSvgParseColor(const char* sText, int iLength, uint32_t* pColor)
{
	int r;
	int g;
	int b;
	int a;

	if ( (sText == NULL) || (pColor == NULL) ) {
		return 0;
	}
	while ( (iLength > 0) && __xgeSvgCharSpace(*sText) ) {
		sText++;
		iLength--;
	}
	while ( (iLength > 0) && __xgeSvgCharSpace(sText[iLength - 1]) ) {
		iLength--;
	}
	if ( iLength <= 0 ) {
		return 0;
	}
	if ( __xgeSvgStringEqualNoCase(sText, iLength, "none") || __xgeSvgStringEqualNoCase(sText, iLength, "transparent") ) {
		*pColor = 0x00000000u;
		return 1;
	}
	if ( __xgeSvgStringEqualNoCase(sText, iLength, "currentColor") || __xgeSvgStringEqualNoCase(sText, iLength, "black") ) {
		*pColor = 0x000000ffu;
		return 1;
	}
	if ( __xgeSvgStringEqualNoCase(sText, iLength, "white") ) {
		*pColor = 0xffffffffu;
		return 1;
	}
	if ( __xgeSvgStringEqualNoCase(sText, iLength, "red") ) {
		*pColor = 0xff0000ffu;
		return 1;
	}
	if ( __xgeSvgStringEqualNoCase(sText, iLength, "green") ) {
		*pColor = 0x008000ffu;
		return 1;
	}
	if ( __xgeSvgStringEqualNoCase(sText, iLength, "blue") ) {
		*pColor = 0x0000ffffu;
		return 1;
	}
	if ( (iLength >= 4) && (sText[0] == '#') ) {
		if ( iLength == 4 || iLength == 5 ) {
			r = __xgeSvgHexValue(sText[1]);
			g = __xgeSvgHexValue(sText[2]);
			b = __xgeSvgHexValue(sText[3]);
			a = (iLength == 5) ? __xgeSvgHexValue(sText[4]) : 15;
			if ( (r < 0) || (g < 0) || (b < 0) || (a < 0) ) return 0;
			*pColor = ((uint32_t)((r << 4) | r) << 24) |
			          ((uint32_t)((g << 4) | g) << 16) |
			          ((uint32_t)((b << 4) | b) << 8) |
			          (uint32_t)((a << 4) | a);
			return 1;
		}
		if ( iLength == 7 || iLength == 9 ) {
			int r0 = __xgeSvgHexValue(sText[1]);
			int r1 = __xgeSvgHexValue(sText[2]);
			int g0 = __xgeSvgHexValue(sText[3]);
			int g1 = __xgeSvgHexValue(sText[4]);
			int b0 = __xgeSvgHexValue(sText[5]);
			int b1 = __xgeSvgHexValue(sText[6]);
			int a0 = (iLength == 9) ? __xgeSvgHexValue(sText[7]) : 15;
			int a1 = (iLength == 9) ? __xgeSvgHexValue(sText[8]) : 15;

			if ( (r0 < 0) || (r1 < 0) || (g0 < 0) || (g1 < 0) || (b0 < 0) || (b1 < 0) || (a0 < 0) || (a1 < 0) ) return 0;
			*pColor = ((uint32_t)((r0 << 4) | r1) << 24) |
			          ((uint32_t)((g0 << 4) | g1) << 16) |
			          ((uint32_t)((b0 << 4) | b1) << 8) |
			          (uint32_t)((a0 << 4) | a1);
			return 1;
		}
	}
	if ( (iLength > 4) && __xgeSvgStringEqualNoCase(sText, 3, "rgb") ) {
		const char* pText = sText + 3;
		const char* pEnd = sText + iLength;
		float fR;
		float fG;
		float fB;

		while ( (pText < pEnd) && __xgeSvgCharSpace(*pText) ) pText++;
		if ( (pText >= pEnd) || (*pText != '(') ) return 0;
		pText++;
		if ( !__xgeSvgReadFloat(&pText, pEnd, &fR) || !__xgeSvgReadFloat(&pText, pEnd, &fG) || !__xgeSvgReadFloat(&pText, pEnd, &fB) ) return 0;
		if ( fR < 0.0f ) fR = 0.0f;
		if ( fG < 0.0f ) fG = 0.0f;
		if ( fB < 0.0f ) fB = 0.0f;
		if ( fR > 255.0f ) fR = 255.0f;
		if ( fG > 255.0f ) fG = 255.0f;
		if ( fB > 255.0f ) fB = 255.0f;
		*pColor = ((uint32_t)(fR + 0.5f) << 24) | ((uint32_t)(fG + 0.5f) << 16) | ((uint32_t)(fB + 0.5f) << 8) | 0xffu;
		return 1;
	}
	return 0;
}

static void __xgeSvgParseDashArray(const char* sValue, int iValueLen, xge_svg_parse_context_t* pContext)
{
	const char* pText;
	const char* pEnd;
	float fValue;
	int iCount;

	if ( (sValue == NULL) || (pContext == NULL) ) {
		return;
	}
	if ( __xgeSvgStringEqualNoCase(sValue, iValueLen, "none") ) {
		pContext->iDashCount = 0;
		__xgeSvgContextFixDashPointer(pContext);
		return;
	}
	pText = sValue;
	pEnd = sValue + iValueLen;
	iCount = 0;
	while ( (pText < pEnd) && (iCount < XGE_SVG_DASH_INLINE_MAX) ) {
		if ( !__xgeSvgReadFloat(&pText, pEnd, &fValue) ) {
			break;
		}
		if ( fValue > 0.0f ) {
			pContext->arrDash[iCount++] = fValue;
		}
	}
	pContext->iDashCount = iCount;
	__xgeSvgContextFixDashPointer(pContext);
}

static void __xgeSvgApplyStyleProperty(xge_svg_parse_context_t* pContext, const char* sName, int iNameLen, const char* sValue, int iValueLen)
{
	float fValue;
	const char* pValue;
	const char* pEnd;
	uint32_t iColor;

	if ( (pContext == NULL) || (sName == NULL) || (sValue == NULL) ) {
		return;
	}
	while ( (iNameLen > 0) && __xgeSvgCharSpace(*sName) ) {
		sName++;
		iNameLen--;
	}
	while ( (iNameLen > 0) && __xgeSvgCharSpace(sName[iNameLen - 1]) ) {
		iNameLen--;
	}
	while ( (iValueLen > 0) && __xgeSvgCharSpace(*sValue) ) {
		sValue++;
		iValueLen--;
	}
	while ( (iValueLen > 0) && __xgeSvgCharSpace(sValue[iValueLen - 1]) ) {
		iValueLen--;
	}
	pValue = sValue;
	pEnd = sValue + iValueLen;
	if ( __xgeSvgStringEqualNoCase(sName, iNameLen, "fill") ) {
		if ( __xgeSvgParseUrlId(sValue, iValueLen, pContext->sFillGradientId, (int)sizeof(pContext->sFillGradientId)) ) {
			pContext->tStyle.iFillColor = 0x00000000u;
		} else if ( __xgeSvgParseColor(sValue, iValueLen, &iColor) ) {
			pContext->sFillGradientId[0] = '\0';
			pContext->tStyle.iFillColor = iColor;
		}
	} else if ( __xgeSvgStringEqualNoCase(sName, iNameLen, "stroke") ) {
		if ( __xgeSvgParseUrlId(sValue, iValueLen, pContext->sStrokeGradientId, (int)sizeof(pContext->sStrokeGradientId)) ) {
			pContext->tStyle.iStrokeColor = 0x00000000u;
		} else if ( __xgeSvgParseColor(sValue, iValueLen, &iColor) ) {
			pContext->sStrokeGradientId[0] = '\0';
			pContext->tStyle.iStrokeColor = iColor;
		}
	} else if ( __xgeSvgStringEqualNoCase(sName, iNameLen, "clip-path") ) {
		if ( __xgeSvgStringEqualNoCase(sValue, iValueLen, "none") ) {
			pContext->sClipPathId[0] = '\0';
		} else {
			(void)__xgeSvgParseUrlId(sValue, iValueLen, pContext->sClipPathId, (int)sizeof(pContext->sClipPathId));
		}
	} else if ( __xgeSvgStringEqualNoCase(sName, iNameLen, "mask") ) {
		if ( __xgeSvgStringEqualNoCase(sValue, iValueLen, "none") ) {
			pContext->sMaskId[0] = '\0';
		} else {
			(void)__xgeSvgParseUrlId(sValue, iValueLen, pContext->sMaskId, (int)sizeof(pContext->sMaskId));
		}
	} else if ( __xgeSvgStringEqualNoCase(sName, iNameLen, "stroke-width") ) {
		if ( __xgeSvgReadFloat(&pValue, pEnd, &fValue) && (fValue >= 0.0f) ) {
			pContext->tStyle.fStrokeWidth = fValue;
		}
	} else if ( __xgeSvgStringEqualNoCase(sName, iNameLen, "fill-rule") ) {
		if ( __xgeSvgStringEqualNoCase(sValue, iValueLen, "evenodd") ) {
			pContext->tStyle.iFillRule = XGE_PATH_FILL_EVEN_ODD;
		} else if ( __xgeSvgStringEqualNoCase(sValue, iValueLen, "nonzero") ) {
			pContext->tStyle.iFillRule = XGE_PATH_FILL_NON_ZERO;
		}
	} else if ( __xgeSvgStringEqualNoCase(sName, iNameLen, "stroke-linejoin") ) {
		if ( __xgeSvgStringEqualNoCase(sValue, iValueLen, "round") ) {
			pContext->tStyle.iLineJoin = XGE_PATH_JOIN_ROUND;
		} else if ( __xgeSvgStringEqualNoCase(sValue, iValueLen, "bevel") ) {
			pContext->tStyle.iLineJoin = XGE_PATH_JOIN_BEVEL;
		} else if ( __xgeSvgStringEqualNoCase(sValue, iValueLen, "miter") ) {
			pContext->tStyle.iLineJoin = XGE_PATH_JOIN_MITER;
		}
	} else if ( __xgeSvgStringEqualNoCase(sName, iNameLen, "stroke-linecap") ) {
		if ( __xgeSvgStringEqualNoCase(sValue, iValueLen, "round") ) {
			pContext->tStyle.iLineCap = XGE_PATH_CAP_ROUND;
		} else if ( __xgeSvgStringEqualNoCase(sValue, iValueLen, "square") ) {
			pContext->tStyle.iLineCap = XGE_PATH_CAP_SQUARE;
		} else if ( __xgeSvgStringEqualNoCase(sValue, iValueLen, "butt") ) {
			pContext->tStyle.iLineCap = XGE_PATH_CAP_BUTT;
		}
	} else if ( __xgeSvgStringEqualNoCase(sName, iNameLen, "stroke-dasharray") ) {
		__xgeSvgParseDashArray(sValue, iValueLen, pContext);
	} else if ( __xgeSvgStringEqualNoCase(sName, iNameLen, "stroke-dashoffset") ) {
		if ( __xgeSvgReadFloat(&pValue, pEnd, &fValue) ) {
			pContext->tStyle.fDashOffset = fValue;
		}
	} else if ( __xgeSvgStringEqualNoCase(sName, iNameLen, "fill-opacity") ) {
		if ( __xgeSvgReadFloat(&pValue, pEnd, &fValue) ) {
			pContext->fFillOpacity *= __xgeSvgClamp01(fValue);
			pContext->tStyle.iFillColor = __xgeSvgColorApplyOpacity(pContext->tStyle.iFillColor, fValue);
		}
	} else if ( __xgeSvgStringEqualNoCase(sName, iNameLen, "stroke-opacity") ) {
		if ( __xgeSvgReadFloat(&pValue, pEnd, &fValue) ) {
			pContext->fStrokeOpacity *= __xgeSvgClamp01(fValue);
			pContext->tStyle.iStrokeColor = __xgeSvgColorApplyOpacity(pContext->tStyle.iStrokeColor, fValue);
		}
	} else if ( __xgeSvgStringEqualNoCase(sName, iNameLen, "opacity") ) {
		if ( __xgeSvgReadFloat(&pValue, pEnd, &fValue) ) {
			pContext->fFillOpacity *= __xgeSvgClamp01(fValue);
			pContext->fStrokeOpacity *= __xgeSvgClamp01(fValue);
			pContext->tStyle.iFillColor = __xgeSvgColorApplyOpacity(pContext->tStyle.iFillColor, fValue);
			pContext->tStyle.iStrokeColor = __xgeSvgColorApplyOpacity(pContext->tStyle.iStrokeColor, fValue);
		}
	}
}

static void __xgeSvgParseStyleAttribute(xge_svg_parse_context_t* pContext, const char* sValue, int iValueLen)
{
	const char* pText;
	const char* pEnd;
	const char* pName;
	const char* pValue;
	int iNameLen;
	int iValuePartLen;

	if ( (pContext == NULL) || (sValue == NULL) ) {
		return;
	}
	pText = sValue;
	pEnd = sValue + iValueLen;
	while ( pText < pEnd ) {
		while ( (pText < pEnd) && (__xgeSvgCharSpace(*pText) || (*pText == ';')) ) pText++;
		pName = pText;
		while ( (pText < pEnd) && (*pText != ':') && (*pText != ';') ) pText++;
		if ( (pText >= pEnd) || (*pText != ':') ) {
			break;
		}
		iNameLen = (int)(pText - pName);
		pText++;
		pValue = pText;
		while ( (pText < pEnd) && (*pText != ';') ) pText++;
		iValuePartLen = (int)(pText - pValue);
		__xgeSvgApplyStyleProperty(pContext, pName, iNameLen, pValue, iValuePartLen);
	}
}

static int __xgeSvgAttrGet(const char* pTag, const char* pTagEnd, const char* sName, const char** ppValue, int* pValueLen)
{
	const char* pText;
	int iNameNeed;

	if ( (pTag == NULL) || (pTagEnd == NULL) || (sName == NULL) || (ppValue == NULL) || (pValueLen == NULL) ) {
		return 0;
	}
	iNameNeed = (int)strlen(sName);
	pText = pTag;
	while ( pText < pTagEnd ) {
		const char* pName;
		const char* pValue;
		int iNameLen;
		char cQuote;

		while ( (pText < pTagEnd) && !((*pText == '_') || (*pText == ':') || (*pText == '-') || ((*pText >= 'a') && (*pText <= 'z')) || ((*pText >= 'A') && (*pText <= 'Z'))) ) pText++;
		pName = pText;
		while ( (pText < pTagEnd) && ((*pText == '_') || (*pText == ':') || (*pText == '-') || ((*pText >= 'a') && (*pText <= 'z')) || ((*pText >= 'A') && (*pText <= 'Z')) || ((*pText >= '0') && (*pText <= '9'))) ) pText++;
		iNameLen = (int)(pText - pName);
		while ( (pText < pTagEnd) && __xgeSvgCharSpace(*pText) ) pText++;
		if ( (pText >= pTagEnd) || (*pText != '=') ) {
			continue;
		}
		pText++;
		while ( (pText < pTagEnd) && __xgeSvgCharSpace(*pText) ) pText++;
		if ( pText >= pTagEnd ) {
			break;
		}
		cQuote = 0;
		if ( (*pText == '"') || (*pText == '\'') ) {
			cQuote = *pText++;
		}
		pValue = pText;
		if ( cQuote != 0 ) {
			while ( (pText < pTagEnd) && (*pText != cQuote) ) pText++;
		} else {
			while ( (pText < pTagEnd) && !__xgeSvgCharSpace(*pText) && (*pText != '/') ) pText++;
		}
		if ( (iNameLen == iNameNeed) && __xgeSvgStringEqual(pName, iNameLen, sName) ) {
			*ppValue = pValue;
			*pValueLen = (int)(pText - pValue);
			return 1;
		}
		if ( (pText < pTagEnd) && (cQuote != 0) ) {
			pText++;
		}
	}
	return 0;
}

static int __xgeSvgAttrFloat(const char* pTag, const char* pTagEnd, const char* sName, float* pValue)
{
	const char* pAttr;
	const char* pEnd;
	int iAttrLen;

	if ( (pValue == NULL) || !__xgeSvgAttrGet(pTag, pTagEnd, sName, &pAttr, &iAttrLen) ) {
		return 0;
	}
	pEnd = pAttr + iAttrLen;
	return __xgeSvgReadFloat(&pAttr, pEnd, pValue);
}

static int __xgeSvgParseAspectAlignToken(const char* sToken, int iTokenLen, int* pAlignX, int* pAlignY)
{
	if ( (sToken == NULL) || (pAlignX == NULL) || (pAlignY == NULL) || (iTokenLen != 8) ) {
		return 0;
	}
	if ( (sToken[0] != 'x') || (sToken[4] != 'Y') ) {
		return 0;
	}
	if ( strncmp(sToken + 1, "Min", 3) == 0 ) {
		*pAlignX = XGE_SVG_ASPECT_ALIGN_MIN;
	} else if ( strncmp(sToken + 1, "Mid", 3) == 0 ) {
		*pAlignX = XGE_SVG_ASPECT_ALIGN_MID;
	} else if ( strncmp(sToken + 1, "Max", 3) == 0 ) {
		*pAlignX = XGE_SVG_ASPECT_ALIGN_MAX;
	} else {
		return 0;
	}
	if ( strncmp(sToken + 5, "Min", 3) == 0 ) {
		*pAlignY = XGE_SVG_ASPECT_ALIGN_MIN;
	} else if ( strncmp(sToken + 5, "Mid", 3) == 0 ) {
		*pAlignY = XGE_SVG_ASPECT_ALIGN_MID;
	} else if ( strncmp(sToken + 5, "Max", 3) == 0 ) {
		*pAlignY = XGE_SVG_ASPECT_ALIGN_MAX;
	} else {
		return 0;
	}
	return 1;
}

static int __xgeSvgNextToken(const char** ppText, const char* pEnd, const char** ppToken, int* pTokenLen)
{
	const char* pText;
	const char* pToken;

	if ( (ppText == NULL) || (ppToken == NULL) || (pTokenLen == NULL) ) {
		return 0;
	}
	pText = *ppText;
	while ( (pText < pEnd) && (__xgeSvgCharSpace(*pText) || (*pText == ',')) ) pText++;
	if ( pText >= pEnd ) {
		*ppText = pText;
		return 0;
	}
	pToken = pText;
	while ( (pText < pEnd) && !__xgeSvgCharSpace(*pText) && (*pText != ',') ) pText++;
	*ppText = pText;
	*ppToken = pToken;
	*pTokenLen = (int)(pText - pToken);
	return *pTokenLen > 0;
}

static int __xgeSvgParsePreserveAspectRatioValues(const char* sValue, int iValueLen, int* pAlignX, int* pAlignY, int* pMode)
{
	const char* pText;
	const char* pEnd;
	const char* pToken;
	int iTokenLen;
	int iAlignX;
	int iAlignY;
	int iMode;

	if ( (sValue == NULL) || (iValueLen < 0) || (pAlignX == NULL) || (pAlignY == NULL) || (pMode == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pText = sValue;
	pEnd = sValue + iValueLen;
	if ( !__xgeSvgNextToken(&pText, pEnd, &pToken, &iTokenLen) ) {
		__xgeSvgAspectValuesDefault(pAlignX, pAlignY, pMode);
		return XGE_OK;
	}
	if ( __xgeSvgStringEqualNoCase(pToken, iTokenLen, "none") ) {
		*pMode = XGE_SVG_ASPECT_NONE;
		return XGE_OK;
	}
	if ( !__xgeSvgParseAspectAlignToken(pToken, iTokenLen, &iAlignX, &iAlignY) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iMode = XGE_SVG_ASPECT_MEET;
	if ( __xgeSvgNextToken(&pText, pEnd, &pToken, &iTokenLen) ) {
		if ( __xgeSvgStringEqualNoCase(pToken, iTokenLen, "slice") ) {
			iMode = XGE_SVG_ASPECT_SLICE;
		} else if ( __xgeSvgStringEqualNoCase(pToken, iTokenLen, "meet") ) {
			iMode = XGE_SVG_ASPECT_MEET;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	*pAlignX = iAlignX;
	*pAlignY = iAlignY;
	*pMode = iMode;
	return XGE_OK;
}

static int __xgeSvgParsePreserveAspectRatio(xge_svg pSvg, const char* sValue, int iValueLen)
{
	if ( !__xgeSvgValid(pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeSvgParsePreserveAspectRatioValues(sValue, iValueLen, &pSvg->iAspectAlignX, &pSvg->iAspectAlignY, &pSvg->iAspectMode);
}

static const char* __xgeSvgFindTagEnd(const char* pText)
{
	char cQuote;

	cQuote = 0;
	while ( *pText != '\0' ) {
		if ( cQuote != 0 ) {
			if ( *pText == cQuote ) {
				cQuote = 0;
			}
		} else if ( (*pText == '"') || (*pText == '\'') ) {
			cQuote = *pText;
		} else if ( *pText == '>' ) {
			return pText;
		}
		pText++;
	}
	return NULL;
}

static int __xgeSvgTagNameMatches(const char* sName, int iNameLen, const char* sExpected)
{
	int i;
	int iStart;

	iStart = 0;
	for ( i = 0; i < iNameLen; i++ ) {
		if ( sName[i] == ':' ) {
			iStart = i + 1;
		}
	}
	return __xgeSvgStringEqualNoCase(sName + iStart, iNameLen - iStart, sExpected);
}

static int __xgeSvgTagSelfClosing(const char* pTagNameEnd, const char* pTagEnd)
{
	const char* pText;

	(void)pTagNameEnd;
	pText = pTagEnd;
	while ( pText > pTagNameEnd ) {
		pText--;
		if ( __xgeSvgCharSpace(*pText) ) {
			continue;
		}
		return *pText == '/';
	}
	return 0;
}

static const char* __xgeSvgFindClosingElement(const char* pText, const char* sName)
{
	if ( (pText == NULL) || (sName == NULL) ) {
		return NULL;
	}
	while ( *pText != '\0' ) {
		const char* pName;
		const char* pNameEnd;

		pText = strchr(pText, '<');
		if ( pText == NULL ) {
			return NULL;
		}
		pName = pText + 1;
		while ( *pName != '\0' && __xgeSvgCharSpace(*pName) ) pName++;
		if ( *pName != '/' ) {
			pText++;
			continue;
		}
		pName++;
		while ( *pName != '\0' && __xgeSvgCharSpace(*pName) ) pName++;
		pNameEnd = pName;
		while ( (*pNameEnd != '\0') && !__xgeSvgCharSpace(*pNameEnd) && (*pNameEnd != '>') && (*pNameEnd != '/') ) pNameEnd++;
		if ( __xgeSvgTagNameMatches(pName, (int)(pNameEnd - pName), sName) ) {
			return pText;
		}
		pText++;
	}
	return NULL;
}

static void __xgeSvgParseStyleAttrs(const char* pTag, const char* pTagEnd, xge_svg_parse_context_t* pContext)
{
	const char* pValue;
	int iValueLen;

	if ( __xgeSvgAttrGet(pTag, pTagEnd, "style", &pValue, &iValueLen) ) {
		__xgeSvgParseStyleAttribute(pContext, pValue, iValueLen);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "fill", &pValue, &iValueLen) ) {
		__xgeSvgApplyStyleProperty(pContext, "fill", 4, pValue, iValueLen);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "stroke", &pValue, &iValueLen) ) {
		__xgeSvgApplyStyleProperty(pContext, "stroke", 6, pValue, iValueLen);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "clip-path", &pValue, &iValueLen) ) {
		__xgeSvgApplyStyleProperty(pContext, "clip-path", 9, pValue, iValueLen);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "mask", &pValue, &iValueLen) ) {
		__xgeSvgApplyStyleProperty(pContext, "mask", 4, pValue, iValueLen);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "stroke-width", &pValue, &iValueLen) ) {
		__xgeSvgApplyStyleProperty(pContext, "stroke-width", 12, pValue, iValueLen);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "fill-rule", &pValue, &iValueLen) ) {
		__xgeSvgApplyStyleProperty(pContext, "fill-rule", 9, pValue, iValueLen);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "stroke-linejoin", &pValue, &iValueLen) ) {
		__xgeSvgApplyStyleProperty(pContext, "stroke-linejoin", 15, pValue, iValueLen);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "stroke-linecap", &pValue, &iValueLen) ) {
		__xgeSvgApplyStyleProperty(pContext, "stroke-linecap", 14, pValue, iValueLen);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "stroke-dasharray", &pValue, &iValueLen) ) {
		__xgeSvgApplyStyleProperty(pContext, "stroke-dasharray", 16, pValue, iValueLen);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "stroke-dashoffset", &pValue, &iValueLen) ) {
		__xgeSvgApplyStyleProperty(pContext, "stroke-dashoffset", 17, pValue, iValueLen);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "opacity", &pValue, &iValueLen) ) {
		__xgeSvgApplyStyleProperty(pContext, "opacity", 7, pValue, iValueLen);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "fill-opacity", &pValue, &iValueLen) ) {
		__xgeSvgApplyStyleProperty(pContext, "fill-opacity", 12, pValue, iValueLen);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "stroke-opacity", &pValue, &iValueLen) ) {
		__xgeSvgApplyStyleProperty(pContext, "stroke-opacity", 14, pValue, iValueLen);
	}
}

static int __xgeSvgClassListContains(const char* sClassList, int iClassListLen, const char* sClassName)
{
	const char* pText;
	const char* pEnd;
	int iClassNameLen;

	if ( (sClassList == NULL) || (sClassName == NULL) || (sClassName[0] == '\0') || (iClassListLen <= 0) ) {
		return 0;
	}
	iClassNameLen = (int)strlen(sClassName);
	pText = sClassList;
	pEnd = sClassList + iClassListLen;
	while ( pText < pEnd ) {
		const char* pToken;
		int iTokenLen;

		while ( (pText < pEnd) && __xgeSvgCharSpace(*pText) ) pText++;
		pToken = pText;
		while ( (pText < pEnd) && !__xgeSvgCharSpace(*pText) ) pText++;
		iTokenLen = (int)(pText - pToken);
		if ( (iTokenLen == iClassNameLen) && (strncmp(pToken, sClassName, (size_t)iTokenLen) == 0) ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeSvgIdMatches(const char* sId, const char* sSelector)
{
	if ( (sId == NULL) || (sSelector == NULL) || (sId[0] == '\0') || (sSelector[0] == '\0') ) {
		return 0;
	}
	return strcmp(sId, sSelector) == 0;
}

static int __xgeSvgTagSelectorMatches(const char* sTagName, int iTagNameLen, const char* sSelector)
{
	if ( (sTagName == NULL) || (iTagNameLen <= 0) || (sSelector == NULL) || (sSelector[0] == '\0') ) {
		return 0;
	}
	return __xgeSvgStringEqualNoCase(sTagName, iTagNameLen, sSelector);
}

static void __xgeSvgApplyStyleRulesOfType(xge_svg pSvg, const char* sTagName, int iTagNameLen, const char* pClassList, int iClassListLen, const char* sId, int iSelectorType, xge_svg_parse_context_t* pContext)
{
	int i;

	if ( !__xgeSvgValid(pSvg) || (pContext == NULL) ) {
		return;
	}
	for ( i = 0; i < pSvg->iStyleRuleCount; i++ ) {
		if ( (pSvg->pStyleRules[i].sStyle == NULL) || (pSvg->pStyleRules[i].iSelectorType != iSelectorType) ) {
			continue;
		}
		if ( (iSelectorType == XGE_SVG_STYLE_SELECTOR_TAG) &&
		     __xgeSvgTagSelectorMatches(sTagName, iTagNameLen, pSvg->pStyleRules[i].sSelector) ) {
			__xgeSvgParseStyleAttribute(pContext, pSvg->pStyleRules[i].sStyle, (int)strlen(pSvg->pStyleRules[i].sStyle));
		} else if ( (iSelectorType == XGE_SVG_STYLE_SELECTOR_CLASS) &&
		     __xgeSvgClassListContains(pClassList, iClassListLen, pSvg->pStyleRules[i].sSelector) ) {
			__xgeSvgParseStyleAttribute(pContext, pSvg->pStyleRules[i].sStyle, (int)strlen(pSvg->pStyleRules[i].sStyle));
		} else if ( (iSelectorType == XGE_SVG_STYLE_SELECTOR_ID) &&
		            __xgeSvgIdMatches(sId, pSvg->pStyleRules[i].sSelector) ) {
			__xgeSvgParseStyleAttribute(pContext, pSvg->pStyleRules[i].sStyle, (int)strlen(pSvg->pStyleRules[i].sStyle));
		}
	}
}

static void __xgeSvgApplyStyleRules(xge_svg pSvg, const char* sTagName, int iTagNameLen, const char* pTag, const char* pTagEnd, xge_svg_parse_context_t* pContext)
{
	const char* pClassList;
	const char* pId;
	int iClassListLen;
	int iIdLen;
	char sId[XGE_SVG_ID_MAX];

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pContext == NULL) ) {
		return;
	}
	pClassList = NULL;
	iClassListLen = 0;
	(void)__xgeSvgAttrGet(pTag, pTagEnd, "class", &pClassList, &iClassListLen);
	sId[0] = '\0';
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "id", &pId, &iIdLen) ) {
		__xgeSvgCopyId(sId, (int)sizeof(sId), pId, iIdLen);
	}
	__xgeSvgApplyStyleRulesOfType(pSvg, sTagName, iTagNameLen, pClassList, iClassListLen, sId, XGE_SVG_STYLE_SELECTOR_TAG, pContext);
	__xgeSvgApplyStyleRulesOfType(pSvg, sTagName, iTagNameLen, pClassList, iClassListLen, sId, XGE_SVG_STYLE_SELECTOR_CLASS, pContext);
	__xgeSvgApplyStyleRulesOfType(pSvg, sTagName, iTagNameLen, pClassList, iClassListLen, sId, XGE_SVG_STYLE_SELECTOR_ID, pContext);
}

static int __xgeSvgParseTransform(const char* sValue, int iValueLen, xge_svg_transform_t* pTransform)
{
	const char* pText;
	const char* pEnd;
	xge_svg_transform_t tOut;

	if ( (sValue == NULL) || (pTransform == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pText = sValue;
	pEnd = sValue + iValueLen;
	tOut = __xgeSvgTransformIdentity();
	while ( pText < pEnd ) {
		const char* pName;
		int iNameLen;
		float arrValues[6];
		int iCount;
		xge_svg_transform_t tLocal;

		while ( (pText < pEnd) && (__xgeSvgCharSpace(*pText) || (*pText == ',')) ) pText++;
		pName = pText;
		while ( (pText < pEnd) && ((*pText >= 'a' && *pText <= 'z') || (*pText >= 'A' && *pText <= 'Z')) ) pText++;
		iNameLen = (int)(pText - pName);
		while ( (pText < pEnd) && __xgeSvgCharSpace(*pText) ) pText++;
		if ( (iNameLen <= 0) || (pText >= pEnd) || (*pText != '(') ) {
			break;
		}
		pText++;
		iCount = 0;
		while ( (pText < pEnd) && (*pText != ')') && (iCount < 6) ) {
			if ( !__xgeSvgReadFloat(&pText, pEnd, &arrValues[iCount]) ) {
				break;
			}
			iCount++;
			__xgeSvgSkipSpaces(&pText, pEnd);
		}
		while ( (pText < pEnd) && (*pText != ')') ) pText++;
		if ( (pText < pEnd) && (*pText == ')') ) pText++;
		tLocal = __xgeSvgTransformIdentity();
		if ( __xgeSvgStringEqualNoCase(pName, iNameLen, "matrix") && (iCount >= 6) ) {
			tLocal.fA = arrValues[0];
			tLocal.fB = arrValues[1];
			tLocal.fC = arrValues[2];
			tLocal.fD = arrValues[3];
			tLocal.fE = arrValues[4];
			tLocal.fF = arrValues[5];
		} else if ( __xgeSvgStringEqualNoCase(pName, iNameLen, "translate") && (iCount >= 1) ) {
			tLocal.fE = arrValues[0];
			tLocal.fF = (iCount >= 2) ? arrValues[1] : 0.0f;
		} else if ( __xgeSvgStringEqualNoCase(pName, iNameLen, "scale") && (iCount >= 1) ) {
			tLocal.fA = arrValues[0];
			tLocal.fD = (iCount >= 2) ? arrValues[1] : arrValues[0];
		} else if ( __xgeSvgStringEqualNoCase(pName, iNameLen, "rotate") && (iCount >= 1) ) {
			float fRadians = arrValues[0] * 0.01745329251994329577f;
			float fCos = cosf(fRadians);
			float fSin = sinf(fRadians);

			tLocal.fA = fCos;
			tLocal.fB = fSin;
			tLocal.fC = -fSin;
			tLocal.fD = fCos;
			if ( iCount >= 3 ) {
				xge_svg_transform_t tToCenter = __xgeSvgTransformIdentity();
				xge_svg_transform_t tBack = __xgeSvgTransformIdentity();

				tToCenter.fE = arrValues[1];
				tToCenter.fF = arrValues[2];
				tBack.fE = -arrValues[1];
				tBack.fF = -arrValues[2];
				tLocal = __xgeSvgTransformMultiply(__xgeSvgTransformMultiply(tToCenter, tLocal), tBack);
			}
		}
		tOut = __xgeSvgTransformMultiply(tOut, tLocal);
	}
	*pTransform = tOut;
	return XGE_OK;
}

static xge_svg_parse_context_t __xgeSvgElementContext(xge_svg pSvg, const xge_svg_parse_context_t* pParent, const char* sTagName, int iTagNameLen, const char* pTag, const char* pTagEnd)
{
	xge_svg_parse_context_t tChild;
	xge_svg_transform_t tLocal;
	const char* pAttrValue;
	int iAttrValueLen;

	tChild = *pParent;
	__xgeSvgContextFixDashPointer(&tChild);
	tChild.sElementId[0] = '\0';
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "id", &pAttrValue, &iAttrValueLen) ) {
		__xgeSvgCopyId(tChild.sElementId, (int)sizeof(tChild.sElementId), pAttrValue, iAttrValueLen);
	}
	__xgeSvgApplyStyleRules(pSvg, sTagName, iTagNameLen, pTag, pTagEnd, &tChild);
	__xgeSvgParseStyleAttrs(pTag, pTagEnd, &tChild);
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "transform", &pAttrValue, &iAttrValueLen) &&
	     (__xgeSvgParseTransform(pAttrValue, iAttrValueLen, &tLocal) == XGE_OK) ) {
		tChild.tTransform = __xgeSvgTransformMultiply(tChild.tTransform, tLocal);
	}
	__xgeSvgContextFixDashPointer(&tChild);
	return tChild;
}

static int __xgePathCopyTransformed(xge_path pDst, xge_path pSrc, xge_svg_transform_t tTransform)
{
	xge_path_command_t tCommand;
	int i;
	int iRet;

	if ( !__xgePathValid(pDst) || !__xgePathValid(pSrc) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = xgePathClear(pDst);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	for ( i = 0; i < pSrc->iCommandCount; i++ ) {
		tCommand = pSrc->pCommands[i];
		if ( tCommand.iCommand == XGE_PATH_CMD_MOVE ) {
			tCommand.arrPoints[0] = __xgeSvgTransformPoint(tTransform, tCommand.arrPoints[0]);
			iRet = xgePathMoveTo(pDst, tCommand.arrPoints[0].fX, tCommand.arrPoints[0].fY);
		} else if ( tCommand.iCommand == XGE_PATH_CMD_LINE ) {
			tCommand.arrPoints[0] = __xgeSvgTransformPoint(tTransform, tCommand.arrPoints[0]);
			iRet = xgePathLineTo(pDst, tCommand.arrPoints[0].fX, tCommand.arrPoints[0].fY);
		} else if ( tCommand.iCommand == XGE_PATH_CMD_QUAD ) {
			tCommand.arrPoints[0] = __xgeSvgTransformPoint(tTransform, tCommand.arrPoints[0]);
			tCommand.arrPoints[1] = __xgeSvgTransformPoint(tTransform, tCommand.arrPoints[1]);
			iRet = xgePathQuadTo(pDst, tCommand.arrPoints[0].fX, tCommand.arrPoints[0].fY, tCommand.arrPoints[1].fX, tCommand.arrPoints[1].fY);
		} else if ( tCommand.iCommand == XGE_PATH_CMD_CUBIC ) {
			tCommand.arrPoints[0] = __xgeSvgTransformPoint(tTransform, tCommand.arrPoints[0]);
			tCommand.arrPoints[1] = __xgeSvgTransformPoint(tTransform, tCommand.arrPoints[1]);
			tCommand.arrPoints[2] = __xgeSvgTransformPoint(tTransform, tCommand.arrPoints[2]);
			iRet = xgePathCubicTo(pDst, tCommand.arrPoints[0].fX, tCommand.arrPoints[0].fY, tCommand.arrPoints[1].fX, tCommand.arrPoints[1].fY, tCommand.arrPoints[2].fX, tCommand.arrPoints[2].fY);
		} else if ( tCommand.iCommand == XGE_PATH_CMD_CLOSE ) {
			iRet = xgePathClose(pDst);
		} else {
			iRet = XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeSvgGradientStopCompare(const void* pA, const void* pB)
{
	const xge_svg_gradient_stop_t* pStopA;
	const xge_svg_gradient_stop_t* pStopB;

	pStopA = (const xge_svg_gradient_stop_t*)pA;
	pStopB = (const xge_svg_gradient_stop_t*)pB;
	if ( pStopA->fOffset < pStopB->fOffset ) return -1;
	if ( pStopA->fOffset > pStopB->fOffset ) return 1;
	return 0;
}

static void __xgeSvgLinearGradientFree(xge_svg_linear_gradient_t* pGradient)
{
	if ( pGradient == NULL ) {
		return;
	}
	if ( pGradient->pStops != NULL ) {
		xrtFree(pGradient->pStops);
	}
	memset(pGradient, 0, sizeof(*pGradient));
}

static void __xgeSvgRadialGradientFree(xge_svg_radial_gradient_t* pGradient)
{
	if ( pGradient == NULL ) {
		return;
	}
	if ( pGradient->pStops != NULL ) {
		xrtFree(pGradient->pStops);
	}
	memset(pGradient, 0, sizeof(*pGradient));
}

static int __xgeSvgReserveLinearGradients(xge_svg pSvg, int iNeeded)
{
	xge_svg_linear_gradient_t* pItems;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pSvg->iLinearGradientCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pSvg->iLinearGradientCapacity > 0) ? pSvg->iLinearGradientCapacity : 4;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pItems = (xge_svg_linear_gradient_t*)xrtRealloc(pSvg->pLinearGradients, sizeof(*pItems) * (size_t)iCapacity);
	if ( pItems == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pItems + pSvg->iLinearGradientCapacity, 0, sizeof(*pItems) * (size_t)(iCapacity - pSvg->iLinearGradientCapacity));
	pSvg->pLinearGradients = pItems;
	pSvg->iLinearGradientCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeSvgReserveRadialGradients(xge_svg pSvg, int iNeeded)
{
	xge_svg_radial_gradient_t* pItems;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pSvg->iRadialGradientCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pSvg->iRadialGradientCapacity > 0) ? pSvg->iRadialGradientCapacity : 4;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pItems = (xge_svg_radial_gradient_t*)xrtRealloc(pSvg->pRadialGradients, sizeof(*pItems) * (size_t)iCapacity);
	if ( pItems == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pItems + pSvg->iRadialGradientCapacity, 0, sizeof(*pItems) * (size_t)(iCapacity - pSvg->iRadialGradientCapacity));
	pSvg->pRadialGradients = pItems;
	pSvg->iRadialGradientCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeSvgReserveClipPaths(xge_svg pSvg, int iNeeded)
{
	xge_svg_clip_path_t* pItems;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pSvg->iClipPathCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pSvg->iClipPathCapacity > 0) ? pSvg->iClipPathCapacity : 4;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pItems = (xge_svg_clip_path_t*)xrtRealloc(pSvg->pClipPaths, sizeof(*pItems) * (size_t)iCapacity);
	if ( pItems == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pItems + pSvg->iClipPathCapacity, 0, sizeof(*pItems) * (size_t)(iCapacity - pSvg->iClipPathCapacity));
	pSvg->pClipPaths = pItems;
	pSvg->iClipPathCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeSvgReserveClipPathRects(xge_svg_clip_path_t* pClipPath, int iNeeded)
{
	xge_rect_t* pRects;
	int iCapacity;

	if ( (pClipPath == NULL) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pClipPath->iRectCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pClipPath->iRectCapacity > 0) ? pClipPath->iRectCapacity : 2;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pRects = (xge_rect_t*)xrtRealloc(pClipPath->pRects, sizeof(*pRects) * (size_t)iCapacity);
	if ( pRects == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pClipPath->pRects = pRects;
	pClipPath->iRectCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeSvgReserveMasks(xge_svg pSvg, int iNeeded)
{
	xge_svg_mask_t* pItems;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pSvg->iMaskCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pSvg->iMaskCapacity > 0) ? pSvg->iMaskCapacity : 4;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pItems = (xge_svg_mask_t*)xrtRealloc(pSvg->pMasks, sizeof(*pItems) * (size_t)iCapacity);
	if ( pItems == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pItems + pSvg->iMaskCapacity, 0, sizeof(*pItems) * (size_t)(iCapacity - pSvg->iMaskCapacity));
	pSvg->pMasks = pItems;
	pSvg->iMaskCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeSvgReserveMaskRects(xge_svg_mask_t* pMask, int iNeeded)
{
	xge_svg_mask_rect_t* pRects;
	int iCapacity;

	if ( (pMask == NULL) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pMask->iRectCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pMask->iRectCapacity > 0) ? pMask->iRectCapacity : 2;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pRects = (xge_svg_mask_rect_t*)xrtRealloc(pMask->pRects, sizeof(*pRects) * (size_t)iCapacity);
	if ( pRects == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pMask->pRects = pRects;
	pMask->iRectCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeSvgReserveViewRefs(xge_svg pSvg, int iNeeded)
{
	xge_svg_view_ref_t* pItems;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pSvg->iViewRefCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pSvg->iViewRefCapacity > 0) ? pSvg->iViewRefCapacity : 4;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pItems = (xge_svg_view_ref_t*)xrtRealloc(pSvg->pViewRefs, sizeof(*pItems) * (size_t)iCapacity);
	if ( pItems == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pItems + pSvg->iViewRefCapacity, 0, sizeof(*pItems) * (size_t)(iCapacity - pSvg->iViewRefCapacity));
	pSvg->pViewRefs = pItems;
	pSvg->iViewRefCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeSvgReserveStyleRules(xge_svg pSvg, int iNeeded)
{
	xge_svg_style_rule_t* pItems;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pSvg->iStyleRuleCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pSvg->iStyleRuleCapacity > 0) ? pSvg->iStyleRuleCapacity : 4;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pItems = (xge_svg_style_rule_t*)xrtRealloc(pSvg->pStyleRules, sizeof(*pItems) * (size_t)iCapacity);
	if ( pItems == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pItems + pSvg->iStyleRuleCapacity, 0, sizeof(*pItems) * (size_t)(iCapacity - pSvg->iStyleRuleCapacity));
	pSvg->pStyleRules = pItems;
	pSvg->iStyleRuleCapacity = iCapacity;
	return XGE_OK;
}

static xge_svg_linear_gradient_t* __xgeSvgFindLinearGradient(xge_svg pSvg, const char* sId)
{
	int i;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') ) {
		return NULL;
	}
	for ( i = pSvg->iLinearGradientCount - 1; i >= 0; i-- ) {
		if ( strcmp(pSvg->pLinearGradients[i].sId, sId) == 0 ) {
			return &pSvg->pLinearGradients[i];
		}
	}
	return NULL;
}

static xge_svg_view_ref_t* __xgeSvgFindViewRef(xge_svg pSvg, const char* sId)
{
	int i;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') ) {
		return NULL;
	}
	for ( i = pSvg->iViewRefCount - 1; i >= 0; i-- ) {
		if ( strcmp(pSvg->pViewRefs[i].sId, sId) == 0 ) {
			return &pSvg->pViewRefs[i];
		}
	}
	return NULL;
}

static xge_svg_radial_gradient_t* __xgeSvgFindRadialGradient(xge_svg pSvg, const char* sId)
{
	int i;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') ) {
		return NULL;
	}
	for ( i = pSvg->iRadialGradientCount - 1; i >= 0; i-- ) {
		if ( strcmp(pSvg->pRadialGradients[i].sId, sId) == 0 ) {
			return &pSvg->pRadialGradients[i];
		}
	}
	return NULL;
}

static xge_svg_clip_path_t* __xgeSvgFindClipPath(xge_svg pSvg, const char* sId)
{
	int i;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') ) {
		return NULL;
	}
	for ( i = pSvg->iClipPathCount - 1; i >= 0; i-- ) {
		if ( strcmp(pSvg->pClipPaths[i].sId, sId) == 0 ) {
			return &pSvg->pClipPaths[i];
		}
	}
	return NULL;
}

static xge_svg_mask_t* __xgeSvgFindMask(xge_svg pSvg, const char* sId)
{
	int i;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') ) {
		return NULL;
	}
	for ( i = pSvg->iMaskCount - 1; i >= 0; i-- ) {
		if ( strcmp(pSvg->pMasks[i].sId, sId) == 0 ) {
			return &pSvg->pMasks[i];
		}
	}
	return NULL;
}

static int __xgeSvgReserveGradientStops(xge_svg_linear_gradient_t* pGradient, int iNeeded)
{
	xge_svg_gradient_stop_t* pStops;
	int iCapacity;

	if ( (pGradient == NULL) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pGradient->iStopCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pGradient->iStopCapacity > 0) ? pGradient->iStopCapacity : 4;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pStops = (xge_svg_gradient_stop_t*)xrtRealloc(pGradient->pStops, sizeof(*pStops) * (size_t)iCapacity);
	if ( pStops == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pGradient->pStops = pStops;
	pGradient->iStopCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeSvgAddGradientStop(xge_svg_linear_gradient_t* pGradient, float fOffset, uint32_t iColor)
{
	int iRet;

	if ( pGradient == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeSvgReserveGradientStops(pGradient, pGradient->iStopCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pGradient->pStops[pGradient->iStopCount].fOffset = __xgeSvgClamp01(fOffset);
	pGradient->pStops[pGradient->iStopCount].iColor = iColor;
	pGradient->iStopCount++;
	qsort(pGradient->pStops, (size_t)pGradient->iStopCount, sizeof(*pGradient->pStops), __xgeSvgGradientStopCompare);
	return XGE_OK;
}

static int __xgeSvgReserveRadialGradientStops(xge_svg_radial_gradient_t* pGradient, int iNeeded)
{
	xge_svg_gradient_stop_t* pStops;
	int iCapacity;

	if ( (pGradient == NULL) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pGradient->iStopCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pGradient->iStopCapacity > 0) ? pGradient->iStopCapacity : 4;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pStops = (xge_svg_gradient_stop_t*)xrtRealloc(pGradient->pStops, sizeof(*pStops) * (size_t)iCapacity);
	if ( pStops == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pGradient->pStops = pStops;
	pGradient->iStopCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeSvgAddRadialGradientStop(xge_svg_radial_gradient_t* pGradient, float fOffset, uint32_t iColor)
{
	int iRet;

	if ( pGradient == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeSvgReserveRadialGradientStops(pGradient, pGradient->iStopCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pGradient->pStops[pGradient->iStopCount].fOffset = __xgeSvgClamp01(fOffset);
	pGradient->pStops[pGradient->iStopCount].iColor = iColor;
	pGradient->iStopCount++;
	qsort(pGradient->pStops, (size_t)pGradient->iStopCount, sizeof(*pGradient->pStops), __xgeSvgGradientStopCompare);
	return XGE_OK;
}

static int __xgeSvgCopyGradientStops(xge_svg_gradient_stop_t** ppDstStops, int* pDstCount, int* pDstCapacity, const xge_svg_gradient_stop_t* pSrcStops, int iSrcCount)
{
	xge_svg_gradient_stop_t* pStops;

	if ( (ppDstStops == NULL) || (pDstCount == NULL) || (pDstCapacity == NULL) || (iSrcCount < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iSrcCount == 0 ) {
		return XGE_OK;
	}
	if ( pSrcStops == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iSrcCount > *pDstCapacity ) {
		pStops = (xge_svg_gradient_stop_t*)xrtRealloc(*ppDstStops, sizeof(*pStops) * (size_t)iSrcCount);
		if ( pStops == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		*ppDstStops = pStops;
		*pDstCapacity = iSrcCount;
	}
	memcpy(*ppDstStops, pSrcStops, sizeof(**ppDstStops) * (size_t)iSrcCount);
	*pDstCount = iSrcCount;
	return XGE_OK;
}

static int __xgeSvgResolveLinearGradientHref(xge_svg pSvg, xge_svg_linear_gradient_t* pGradient, int* pChanged)
{
	xge_svg_linear_gradient_t* pLinearSource;
	xge_svg_radial_gradient_t* pRadialSource;
	int iRet;

	if ( (pGradient == NULL) || (pGradient->iStopCount > 0) || (pGradient->sHrefId[0] == '\0') ) {
		return XGE_OK;
	}
	pLinearSource = __xgeSvgFindLinearGradient(pSvg, pGradient->sHrefId);
	if ( (pLinearSource != NULL) && (pLinearSource != pGradient) && (pLinearSource->iStopCount > 0) ) {
		iRet = __xgeSvgCopyGradientStops(&pGradient->pStops, &pGradient->iStopCount, &pGradient->iStopCapacity, pLinearSource->pStops, pLinearSource->iStopCount);
		if ( (iRet == XGE_OK) && (pChanged != NULL) ) {
			*pChanged = 1;
		}
		return iRet;
	}
	pRadialSource = __xgeSvgFindRadialGradient(pSvg, pGradient->sHrefId);
	if ( (pRadialSource != NULL) && (pRadialSource->iStopCount > 0) ) {
		iRet = __xgeSvgCopyGradientStops(&pGradient->pStops, &pGradient->iStopCount, &pGradient->iStopCapacity, pRadialSource->pStops, pRadialSource->iStopCount);
		if ( (iRet == XGE_OK) && (pChanged != NULL) ) {
			*pChanged = 1;
		}
		return iRet;
	}
	return XGE_OK;
}

static int __xgeSvgResolveRadialGradientHref(xge_svg pSvg, xge_svg_radial_gradient_t* pGradient, int* pChanged)
{
	xge_svg_radial_gradient_t* pRadialSource;
	xge_svg_linear_gradient_t* pLinearSource;
	int iRet;

	if ( (pGradient == NULL) || (pGradient->iStopCount > 0) || (pGradient->sHrefId[0] == '\0') ) {
		return XGE_OK;
	}
	pRadialSource = __xgeSvgFindRadialGradient(pSvg, pGradient->sHrefId);
	if ( (pRadialSource != NULL) && (pRadialSource != pGradient) && (pRadialSource->iStopCount > 0) ) {
		iRet = __xgeSvgCopyGradientStops(&pGradient->pStops, &pGradient->iStopCount, &pGradient->iStopCapacity, pRadialSource->pStops, pRadialSource->iStopCount);
		if ( (iRet == XGE_OK) && (pChanged != NULL) ) {
			*pChanged = 1;
		}
		return iRet;
	}
	pLinearSource = __xgeSvgFindLinearGradient(pSvg, pGradient->sHrefId);
	if ( (pLinearSource != NULL) && (pLinearSource->iStopCount > 0) ) {
		iRet = __xgeSvgCopyGradientStops(&pGradient->pStops, &pGradient->iStopCount, &pGradient->iStopCapacity, pLinearSource->pStops, pLinearSource->iStopCount);
		if ( (iRet == XGE_OK) && (pChanged != NULL) ) {
			*pChanged = 1;
		}
		return iRet;
	}
	return XGE_OK;
}

static int __xgeSvgResolveGradientHrefs(xge_svg pSvg)
{
	int iPass;
	int iMaxPass;
	int i;
	int iChanged;
	int iRet;

	if ( !__xgeSvgValid(pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iMaxPass = pSvg->iLinearGradientCount + pSvg->iRadialGradientCount;
	if ( iMaxPass <= 0 ) {
		return XGE_OK;
	}
	for ( iPass = 0; iPass < iMaxPass; iPass++ ) {
		iChanged = 0;
		for ( i = 0; i < pSvg->iLinearGradientCount; i++ ) {
			iRet = __xgeSvgResolveLinearGradientHref(pSvg, &pSvg->pLinearGradients[i], &iChanged);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
		for ( i = 0; i < pSvg->iRadialGradientCount; i++ ) {
			iRet = __xgeSvgResolveRadialGradientHref(pSvg, &pSvg->pRadialGradients[i], &iChanged);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
		if ( !iChanged ) {
			break;
		}
	}
	return XGE_OK;
}

static int __xgeSvgParseSpreadMethod(const char* sValue, int iValueLen)
{
	if ( __xgeSvgStringEqualNoCase(sValue, iValueLen, "repeat") ) {
		return XGE_SVG_SPREAD_REPEAT;
	}
	if ( __xgeSvgStringEqualNoCase(sValue, iValueLen, "reflect") ) {
		return XGE_SVG_SPREAD_REFLECT;
	}
	return XGE_SVG_SPREAD_PAD;
}

static xge_svg_linear_gradient_t* __xgeSvgBeginLinearGradient(xge_svg pSvg, const char* pTag, const char* pTagEnd, int* pRet)
{
	xge_svg_linear_gradient_t* pGradient;
	const char* pValue;
	int iValueLen;
	int iRet;

	if ( pRet != NULL ) {
		*pRet = XGE_OK;
	}
	if ( !__xgeSvgValid(pSvg) ) {
		if ( pRet != NULL ) *pRet = XGE_ERROR_INVALID_ARGUMENT;
		return NULL;
	}
	if ( !__xgeSvgAttrGet(pTag, pTagEnd, "id", &pValue, &iValueLen) || (iValueLen <= 0) ) {
		return NULL;
	}
	iRet = __xgeSvgReserveLinearGradients(pSvg, pSvg->iLinearGradientCount + 1);
	if ( iRet != XGE_OK ) {
		if ( pRet != NULL ) *pRet = iRet;
		return NULL;
	}
	pGradient = &pSvg->pLinearGradients[pSvg->iLinearGradientCount++];
	memset(pGradient, 0, sizeof(*pGradient));
	__xgeSvgCopyId(pGradient->sId, (int)sizeof(pGradient->sId), pValue, iValueLen);
	if ( pGradient->sId[0] == '\0' ) {
		pSvg->iLinearGradientCount--;
		return NULL;
	}
	pGradient->iUnits = XGE_SVG_GRADIENT_OBJECT_BOUNDING_BOX;
	pGradient->fX1 = 0.0f;
	pGradient->fY1 = 0.0f;
	pGradient->fX2 = 1.0f;
	pGradient->fY2 = 0.0f;
	pGradient->iSpreadMethod = XGE_SVG_SPREAD_PAD;
	pGradient->tTransform = __xgeSvgTransformIdentity();
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "gradientUnits", &pValue, &iValueLen) &&
	     __xgeSvgStringEqualNoCase(pValue, iValueLen, "userSpaceOnUse") ) {
		pGradient->iUnits = XGE_SVG_GRADIENT_USER_SPACE;
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "spreadMethod", &pValue, &iValueLen) ) {
		pGradient->iSpreadMethod = __xgeSvgParseSpreadMethod(pValue, iValueLen);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "gradientTransform", &pValue, &iValueLen) ) {
		(void)__xgeSvgParseTransform(pValue, iValueLen, &pGradient->tTransform);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "href", &pValue, &iValueLen) ) {
		(void)__xgeSvgParseHrefId(pValue, iValueLen, pGradient->sHrefId, (int)sizeof(pGradient->sHrefId));
	}
	if ( (pGradient->sHrefId[0] == '\0') && __xgeSvgAttrGet(pTag, pTagEnd, "xlink:href", &pValue, &iValueLen) ) {
		(void)__xgeSvgParseHrefId(pValue, iValueLen, pGradient->sHrefId, (int)sizeof(pGradient->sHrefId));
	}
	(void)__xgeSvgAttrLengthPercent(pTag, pTagEnd, "x1", &pGradient->fX1, NULL);
	(void)__xgeSvgAttrLengthPercent(pTag, pTagEnd, "y1", &pGradient->fY1, NULL);
	(void)__xgeSvgAttrLengthPercent(pTag, pTagEnd, "x2", &pGradient->fX2, NULL);
	(void)__xgeSvgAttrLengthPercent(pTag, pTagEnd, "y2", &pGradient->fY2, NULL);
	return pGradient;
}

static xge_svg_radial_gradient_t* __xgeSvgBeginRadialGradient(xge_svg pSvg, const char* pTag, const char* pTagEnd, int* pRet)
{
	xge_svg_radial_gradient_t* pGradient;
	const char* pValue;
	int iValueLen;
	int iRet;

	if ( pRet != NULL ) {
		*pRet = XGE_OK;
	}
	if ( !__xgeSvgValid(pSvg) ) {
		if ( pRet != NULL ) *pRet = XGE_ERROR_INVALID_ARGUMENT;
		return NULL;
	}
	if ( !__xgeSvgAttrGet(pTag, pTagEnd, "id", &pValue, &iValueLen) || (iValueLen <= 0) ) {
		return NULL;
	}
	iRet = __xgeSvgReserveRadialGradients(pSvg, pSvg->iRadialGradientCount + 1);
	if ( iRet != XGE_OK ) {
		if ( pRet != NULL ) *pRet = iRet;
		return NULL;
	}
	pGradient = &pSvg->pRadialGradients[pSvg->iRadialGradientCount++];
	memset(pGradient, 0, sizeof(*pGradient));
	__xgeSvgCopyId(pGradient->sId, (int)sizeof(pGradient->sId), pValue, iValueLen);
	if ( pGradient->sId[0] == '\0' ) {
		pSvg->iRadialGradientCount--;
		return NULL;
	}
	pGradient->iUnits = XGE_SVG_GRADIENT_OBJECT_BOUNDING_BOX;
	pGradient->fCX = 0.5f;
	pGradient->fCY = 0.5f;
	pGradient->fR = 0.5f;
	pGradient->fFX = 0.5f;
	pGradient->fFY = 0.5f;
	pGradient->iSpreadMethod = XGE_SVG_SPREAD_PAD;
	pGradient->tTransform = __xgeSvgTransformIdentity();
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "gradientUnits", &pValue, &iValueLen) &&
	     __xgeSvgStringEqualNoCase(pValue, iValueLen, "userSpaceOnUse") ) {
		pGradient->iUnits = XGE_SVG_GRADIENT_USER_SPACE;
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "spreadMethod", &pValue, &iValueLen) ) {
		pGradient->iSpreadMethod = __xgeSvgParseSpreadMethod(pValue, iValueLen);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "gradientTransform", &pValue, &iValueLen) ) {
		(void)__xgeSvgParseTransform(pValue, iValueLen, &pGradient->tTransform);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "href", &pValue, &iValueLen) ) {
		(void)__xgeSvgParseHrefId(pValue, iValueLen, pGradient->sHrefId, (int)sizeof(pGradient->sHrefId));
	}
	if ( (pGradient->sHrefId[0] == '\0') && __xgeSvgAttrGet(pTag, pTagEnd, "xlink:href", &pValue, &iValueLen) ) {
		(void)__xgeSvgParseHrefId(pValue, iValueLen, pGradient->sHrefId, (int)sizeof(pGradient->sHrefId));
	}
	(void)__xgeSvgAttrLengthPercent(pTag, pTagEnd, "cx", &pGradient->fCX, NULL);
	(void)__xgeSvgAttrLengthPercent(pTag, pTagEnd, "cy", &pGradient->fCY, NULL);
	(void)__xgeSvgAttrLengthPercent(pTag, pTagEnd, "r", &pGradient->fR, NULL);
	if ( __xgeSvgAttrLengthPercent(pTag, pTagEnd, "fx", &pGradient->fFX, NULL) ) {
		pGradient->bHasFX = 1;
	} else {
		pGradient->fFX = pGradient->fCX;
	}
	if ( __xgeSvgAttrLengthPercent(pTag, pTagEnd, "fy", &pGradient->fFY, NULL) ) {
		pGradient->bHasFY = 1;
	} else {
		pGradient->fFY = pGradient->fCY;
	}
	if ( pGradient->fR < 0.0f ) {
		pGradient->fR = 0.0f;
	}
	return pGradient;
}

static int __xgeSvgParseViewBoxValue(const char* sValue, int iValueLen, xge_rect_t* pViewBox)
{
	const char* pText;
	const char* pEnd;
	float fX;
	float fY;
	float fW;
	float fH;

	if ( (sValue == NULL) || (iValueLen < 0) || (pViewBox == NULL) ) {
		return 0;
	}
	pText = sValue;
	pEnd = sValue + iValueLen;
	if ( __xgeSvgReadFloat(&pText, pEnd, &fX) &&
	     __xgeSvgReadFloat(&pText, pEnd, &fY) &&
	     __xgeSvgReadFloat(&pText, pEnd, &fW) &&
	     __xgeSvgReadFloat(&pText, pEnd, &fH) &&
	     (fW > 0.0f) && (fH > 0.0f) ) {
		*pViewBox = (xge_rect_t){fX, fY, fW, fH};
		return 1;
	}
	return 0;
}

static int __xgeSvgAddViewRef(xge_svg pSvg, const char* pTag, const char* pTagEnd)
{
	xge_svg_view_ref_t* pViewRef;
	const char* pValue;
	xge_rect_t tViewBox;
	float fWidth;
	float fHeight;
	int iValueLen;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeSvgAttrGet(pTag, pTagEnd, "id", &pValue, &iValueLen) || (iValueLen <= 0) ) {
		return XGE_OK;
	}
	if ( !__xgeSvgAttrGet(pTag, pTagEnd, "viewBox", &pValue, &iValueLen) ||
	     !__xgeSvgParseViewBoxValue(pValue, iValueLen, &tViewBox) ) {
		return XGE_OK;
	}
	iRet = __xgeSvgReserveViewRefs(pSvg, pSvg->iViewRefCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pViewRef = &pSvg->pViewRefs[pSvg->iViewRefCount++];
	memset(pViewRef, 0, sizeof(*pViewRef));
	(void)__xgeSvgAttrGet(pTag, pTagEnd, "id", &pValue, &iValueLen);
	__xgeSvgCopyId(pViewRef->sId, (int)sizeof(pViewRef->sId), pValue, iValueLen);
	if ( pViewRef->sId[0] == '\0' ) {
		pSvg->iViewRefCount--;
		return XGE_OK;
	}
	pViewRef->tViewBox = tViewBox;
	pViewRef->bHasViewBox = 1;
	fWidth = 0.0f;
	fHeight = 0.0f;
	if ( __xgeSvgAttrFloat(pTag, pTagEnd, "width", &fWidth) && (fWidth > 0.0f) ) {
		pViewRef->fWidth = fWidth;
	}
	if ( __xgeSvgAttrFloat(pTag, pTagEnd, "height", &fHeight) && (fHeight > 0.0f) ) {
		pViewRef->fHeight = fHeight;
	}
	__xgeSvgAspectValuesDefault(&pViewRef->iAspectAlignX, &pViewRef->iAspectAlignY, &pViewRef->iAspectMode);
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "preserveAspectRatio", &pValue, &iValueLen) ) {
		iRet = __xgeSvgParsePreserveAspectRatioValues(pValue, iValueLen, &pViewRef->iAspectAlignX, &pViewRef->iAspectAlignY, &pViewRef->iAspectMode);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeSvgCssSelectorNameChar(char c)
{
	return ((c >= 'a') && (c <= 'z')) ||
	       ((c >= 'A') && (c <= 'Z')) ||
	       ((c >= '0') && (c <= '9')) ||
	       (c == '_') || (c == '-');
}

static int __xgeSvgAddStyleRule(xge_svg pSvg, int iSelectorType, const char* sSelector, int iSelectorLen, const char* sStyle, int iStyleLen)
{
	xge_svg_style_rule_t* pRule;
	char* sStyleCopy;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (sSelector == NULL) || (iSelectorLen <= 0) || (sStyle == NULL) || (iStyleLen <= 0) ) {
		return XGE_OK;
	}
	if ( (iSelectorType != XGE_SVG_STYLE_SELECTOR_TAG) &&
	     (iSelectorType != XGE_SVG_STYLE_SELECTOR_CLASS) &&
	     (iSelectorType != XGE_SVG_STYLE_SELECTOR_ID) ) {
		return XGE_OK;
	}
	iRet = __xgeSvgReserveStyleRules(pSvg, pSvg->iStyleRuleCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	sStyleCopy = (char*)xrtMalloc((size_t)iStyleLen + 1u);
	if ( sStyleCopy == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(sStyleCopy, sStyle, (size_t)iStyleLen);
	sStyleCopy[iStyleLen] = '\0';
	pRule = &pSvg->pStyleRules[pSvg->iStyleRuleCount++];
	memset(pRule, 0, sizeof(*pRule));
	pRule->iSelectorType = iSelectorType;
	__xgeSvgCopyId(pRule->sSelector, (int)sizeof(pRule->sSelector), sSelector, iSelectorLen);
	if ( pRule->sSelector[0] == '\0' ) {
		xrtFree(sStyleCopy);
		pSvg->iStyleRuleCount--;
		return XGE_OK;
	}
	pRule->sStyle = sStyleCopy;
	return XGE_OK;
}

static int __xgeSvgParseCssStyleSheet(xge_svg pSvg, const char* sCss, int iCssLen)
{
	const char* pText;
	const char* pEnd;

	if ( !__xgeSvgValid(pSvg) || (sCss == NULL) || (iCssLen <= 0) ) {
		return XGE_OK;
	}
	pText = sCss;
	pEnd = sCss + iCssLen;
	while ( pText < pEnd ) {
		const char* pSelectorStart;
		const char* pBrace;
		const char* pBodyEnd;
		const char* pSelector;
		const char* pBodyStart;
		int iRet;

		pSelectorStart = pText;
		pBrace = NULL;
		while ( pText < pEnd ) {
			if ( (pText + 1 < pEnd) && (pText[0] == '/') && (pText[1] == '*') ) {
				pText += 2;
				while ( (pText + 1 < pEnd) && !((pText[0] == '*') && (pText[1] == '/')) ) pText++;
				if ( pText + 1 < pEnd ) pText += 2;
				continue;
			}
			if ( *pText == '{' ) {
				pBrace = pText;
				break;
			}
			pText++;
		}
		if ( pBrace == NULL ) {
			break;
		}
		pBodyStart = pBrace + 1;
		pBodyEnd = pBodyStart;
		while ( (pBodyEnd < pEnd) && (*pBodyEnd != '}') ) pBodyEnd++;
		if ( pBodyEnd >= pEnd ) {
			break;
		}
		pSelector = pSelectorStart;
		iRet = XGE_OK;
		{
			int bSelectorStart = 1;

		while ( pSelector < pBrace ) {
			if ( *pSelector == ',' ) {
				bSelectorStart = 1;
				pSelector++;
				continue;
			}
			if ( __xgeSvgCharSpace(*pSelector) ) {
				pSelector++;
				continue;
			}
			if ( (*pSelector == '.') || (*pSelector == '#') ) {
				const char* pName;
				int iSelectorType;

				iSelectorType = (*pSelector == '.') ? XGE_SVG_STYLE_SELECTOR_CLASS : XGE_SVG_STYLE_SELECTOR_ID;
				pSelector++;
				pName = pSelector;
				while ( (pSelector < pBrace) && __xgeSvgCssSelectorNameChar(*pSelector) ) pSelector++;
				if ( pSelector > pName ) {
					iRet = __xgeSvgAddStyleRule(pSvg, iSelectorType, pName, (int)(pSelector - pName), pBodyStart, (int)(pBodyEnd - pBodyStart));
					if ( iRet != XGE_OK ) {
						return iRet;
					}
				}
				bSelectorStart = 0;
			} else if ( bSelectorStart && __xgeSvgCssSelectorNameChar(*pSelector) ) {
				const char* pName;

				pName = pSelector;
				while ( (pSelector < pBrace) && __xgeSvgCssSelectorNameChar(*pSelector) ) pSelector++;
				if ( pSelector > pName ) {
					iRet = __xgeSvgAddStyleRule(pSvg, XGE_SVG_STYLE_SELECTOR_TAG, pName, (int)(pSelector - pName), pBodyStart, (int)(pBodyEnd - pBodyStart));
					if ( iRet != XGE_OK ) {
						return iRet;
					}
				}
				bSelectorStart = 0;
			} else {
				bSelectorStart = 0;
				pSelector++;
			}
		}
		}
		pText = pBodyEnd + 1;
	}
	return XGE_OK;
}

static xge_svg_clip_path_t* __xgeSvgBeginClipPath(xge_svg pSvg, const char* pTag, const char* pTagEnd, int* pRet)
{
	xge_svg_clip_path_t* pClipPath;
	const char* pValue;
	int iValueLen;
	int iRet;

	if ( pRet != NULL ) {
		*pRet = XGE_OK;
	}
	if ( !__xgeSvgValid(pSvg) ) {
		if ( pRet != NULL ) *pRet = XGE_ERROR_INVALID_ARGUMENT;
		return NULL;
	}
	if ( !__xgeSvgAttrGet(pTag, pTagEnd, "id", &pValue, &iValueLen) || (iValueLen <= 0) ) {
		return NULL;
	}
	iRet = __xgeSvgReserveClipPaths(pSvg, pSvg->iClipPathCount + 1);
	if ( iRet != XGE_OK ) {
		if ( pRet != NULL ) *pRet = iRet;
		return NULL;
	}
	pClipPath = &pSvg->pClipPaths[pSvg->iClipPathCount++];
	memset(pClipPath, 0, sizeof(*pClipPath));
	__xgeSvgCopyId(pClipPath->sId, (int)sizeof(pClipPath->sId), pValue, iValueLen);
	if ( pClipPath->sId[0] == '\0' ) {
		pSvg->iClipPathCount--;
		return NULL;
	}
	pClipPath->iUnits = XGE_SVG_CLIP_USER_SPACE;
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "clipPathUnits", &pValue, &iValueLen) &&
	     __xgeSvgStringEqualNoCase(pValue, iValueLen, "objectBoundingBox") ) {
		pClipPath->iUnits = XGE_SVG_CLIP_OBJECT_BOUNDING_BOX;
	}
	return pClipPath;
}

static int __xgeSvgAddClipPathRect(xge_svg_clip_path_t* pClipPath, const char* pTag, const char* pTagEnd, const xge_svg_parse_context_t* pContext)
{
	xge_rect_t tRect;
	float fX;
	float fY;
	float fW;
	float fH;

	if ( (pClipPath == NULL) || (pContext == NULL) ) {
		return XGE_OK;
	}
	fX = 0.0f;
	fY = 0.0f;
	(void)__xgeSvgAttrFloat(pTag, pTagEnd, "x", &fX);
	(void)__xgeSvgAttrFloat(pTag, pTagEnd, "y", &fY);
	if ( !__xgeSvgAttrFloat(pTag, pTagEnd, "width", &fW) || !__xgeSvgAttrFloat(pTag, pTagEnd, "height", &fH) ) {
		return XGE_OK;
	}
	if ( (fW <= 0.0f) || (fH <= 0.0f) ) {
		return XGE_OK;
	}
	tRect = __xgeSvgTransformRectBounds(pContext->tTransform, (xge_rect_t){fX, fY, fW, fH});
	if ( (tRect.fW <= XGE_PATH_EPSILON) || (tRect.fH <= XGE_PATH_EPSILON) ) {
		return XGE_OK;
	}
	if ( __xgeSvgReserveClipPathRects(pClipPath, pClipPath->iRectCount + 1) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pClipPath->pRects[pClipPath->iRectCount++] = tRect;
	if ( !pClipPath->bHasRect ) {
		pClipPath->tRect = tRect;
		pClipPath->bHasRect = 1;
	}
	return XGE_OK;
}

static xge_svg_mask_t* __xgeSvgBeginMask(xge_svg pSvg, const char* pTag, const char* pTagEnd, int* pRet)
{
	xge_svg_mask_t* pMask;
	const char* pValue;
	int iValueLen;
	int iRet;

	if ( pRet != NULL ) {
		*pRet = XGE_OK;
	}
	if ( !__xgeSvgValid(pSvg) ) {
		if ( pRet != NULL ) *pRet = XGE_ERROR_INVALID_ARGUMENT;
		return NULL;
	}
	if ( !__xgeSvgAttrGet(pTag, pTagEnd, "id", &pValue, &iValueLen) || (iValueLen <= 0) ) {
		return NULL;
	}
	iRet = __xgeSvgReserveMasks(pSvg, pSvg->iMaskCount + 1);
	if ( iRet != XGE_OK ) {
		if ( pRet != NULL ) *pRet = iRet;
		return NULL;
	}
	pMask = &pSvg->pMasks[pSvg->iMaskCount++];
	memset(pMask, 0, sizeof(*pMask));
	__xgeSvgCopyId(pMask->sId, (int)sizeof(pMask->sId), pValue, iValueLen);
	if ( pMask->sId[0] == '\0' ) {
		pSvg->iMaskCount--;
		return NULL;
	}
	pMask->iUnits = XGE_SVG_MASK_USER_SPACE;
	pMask->fOpacity = 1.0f;
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "maskContentUnits", &pValue, &iValueLen) &&
	     __xgeSvgStringEqualNoCase(pValue, iValueLen, "objectBoundingBox") ) {
		pMask->iUnits = XGE_SVG_MASK_OBJECT_BOUNDING_BOX;
	}
	return pMask;
}

static float __xgeSvgMaskRectOpacity(const xge_svg_parse_context_t* pContext)
{
	uint32_t iColor;
	float fAlpha;
	float fLuma;

	if ( pContext == NULL ) {
		return 0.0f;
	}
	iColor = pContext->tStyle.iFillColor;
	fAlpha = (float)XGE_COLOR_GET_A(iColor) / 255.0f;
	fLuma = ((float)XGE_COLOR_GET_R(iColor) * 0.2126f + (float)XGE_COLOR_GET_G(iColor) * 0.7152f + (float)XGE_COLOR_GET_B(iColor) * 0.0722f) / 255.0f;
	return __xgeSvgClamp01(fAlpha * fLuma);
}

static int __xgeSvgAddMaskRect(xge_svg_mask_t* pMask, const char* pTag, const char* pTagEnd, const xge_svg_parse_context_t* pContext)
{
	xge_rect_t tRect;
	float fX;
	float fY;
	float fW;
	float fH;
	float fOpacity;

	if ( (pMask == NULL) || (pContext == NULL) ) {
		return XGE_OK;
	}
	fX = 0.0f;
	fY = 0.0f;
	(void)__xgeSvgAttrLengthPercent(pTag, pTagEnd, "x", &fX, NULL);
	(void)__xgeSvgAttrLengthPercent(pTag, pTagEnd, "y", &fY, NULL);
	if ( !__xgeSvgAttrLengthPercent(pTag, pTagEnd, "width", &fW, NULL) ||
	     !__xgeSvgAttrLengthPercent(pTag, pTagEnd, "height", &fH, NULL) ) {
		return XGE_OK;
	}
	if ( (fW <= 0.0f) || (fH <= 0.0f) ) {
		return XGE_OK;
	}
	fOpacity = __xgeSvgMaskRectOpacity(pContext);
	if ( fOpacity <= 0.0f ) {
		return XGE_OK;
	}
	tRect = __xgeSvgTransformRectBounds(pContext->tTransform, (xge_rect_t){fX, fY, fW, fH});
	if ( (tRect.fW <= XGE_PATH_EPSILON) || (tRect.fH <= XGE_PATH_EPSILON) ) {
		return XGE_OK;
	}
	if ( __xgeSvgReserveMaskRects(pMask, pMask->iRectCount + 1) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pMask->pRects[pMask->iRectCount++] = (xge_svg_mask_rect_t){tRect, fOpacity};
	if ( !pMask->bHasRect ) {
		pMask->tRect = tRect;
		pMask->fOpacity = fOpacity;
		pMask->bHasRect = 1;
	}
	return XGE_OK;
}

static void __xgeSvgApplyStopProperty(const char* sName, int iNameLen, const char* sValue, int iValueLen, uint32_t* pColor, float* pOpacity)
{
	const char* pValue;
	const char* pEnd;
	uint32_t iColor;
	float fValue;

	if ( (sName == NULL) || (sValue == NULL) || (pColor == NULL) || (pOpacity == NULL) ) {
		return;
	}
	while ( (iNameLen > 0) && __xgeSvgCharSpace(*sName) ) {
		sName++;
		iNameLen--;
	}
	while ( (iNameLen > 0) && __xgeSvgCharSpace(sName[iNameLen - 1]) ) {
		iNameLen--;
	}
	while ( (iValueLen > 0) && __xgeSvgCharSpace(*sValue) ) {
		sValue++;
		iValueLen--;
	}
	while ( (iValueLen > 0) && __xgeSvgCharSpace(sValue[iValueLen - 1]) ) {
		iValueLen--;
	}
	if ( __xgeSvgStringEqualNoCase(sName, iNameLen, "stop-color") ) {
		if ( __xgeSvgParseColor(sValue, iValueLen, &iColor) ) {
			*pColor = iColor;
		}
	} else if ( __xgeSvgStringEqualNoCase(sName, iNameLen, "stop-opacity") ) {
		pValue = sValue;
		pEnd = sValue + iValueLen;
		if ( __xgeSvgReadFloat(&pValue, pEnd, &fValue) ) {
			*pOpacity = __xgeSvgClamp01(fValue);
		}
	}
}

static void __xgeSvgParseStopStyle(const char* sValue, int iValueLen, uint32_t* pColor, float* pOpacity)
{
	const char* pText;
	const char* pEnd;
	const char* pName;
	const char* pValue;
	int iNameLen;
	int iValuePartLen;

	if ( sValue == NULL ) {
		return;
	}
	pText = sValue;
	pEnd = sValue + iValueLen;
	while ( pText < pEnd ) {
		while ( (pText < pEnd) && (__xgeSvgCharSpace(*pText) || (*pText == ';')) ) pText++;
		pName = pText;
		while ( (pText < pEnd) && (*pText != ':') && (*pText != ';') ) pText++;
		if ( (pText >= pEnd) || (*pText != ':') ) {
			break;
		}
		iNameLen = (int)(pText - pName);
		pText++;
		pValue = pText;
		while ( (pText < pEnd) && (*pText != ';') ) pText++;
		iValuePartLen = (int)(pText - pValue);
		__xgeSvgApplyStopProperty(pName, iNameLen, pValue, iValuePartLen, pColor, pOpacity);
		if ( (pText < pEnd) && (*pText == ';') ) pText++;
	}
}

static int __xgeSvgParseGradientStop(xge_svg_linear_gradient_t* pGradient, const char* pTag, const char* pTagEnd)
{
	const char* pValue;
	int iValueLen;
	uint32_t iColor;
	float fOpacity;
	float fOffset;

	if ( pGradient == NULL ) {
		return XGE_OK;
	}
	iColor = 0x000000ffu;
	fOpacity = 1.0f;
	fOffset = 0.0f;
	(void)__xgeSvgAttrLengthPercent(pTag, pTagEnd, "offset", &fOffset, NULL);
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "style", &pValue, &iValueLen) ) {
		__xgeSvgParseStopStyle(pValue, iValueLen, &iColor, &fOpacity);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "stop-color", &pValue, &iValueLen) ) {
		__xgeSvgApplyStopProperty("stop-color", 10, pValue, iValueLen, &iColor, &fOpacity);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "stop-opacity", &pValue, &iValueLen) ) {
		__xgeSvgApplyStopProperty("stop-opacity", 12, pValue, iValueLen, &iColor, &fOpacity);
	}
	iColor = __xgeSvgColorApplyOpacity(iColor, fOpacity);
	return __xgeSvgAddGradientStop(pGradient, fOffset, iColor);
}

static int __xgeSvgParseRadialGradientStop(xge_svg_radial_gradient_t* pGradient, const char* pTag, const char* pTagEnd)
{
	const char* pValue;
	int iValueLen;
	uint32_t iColor;
	float fOpacity;
	float fOffset;

	if ( pGradient == NULL ) {
		return XGE_OK;
	}
	iColor = 0x000000ffu;
	fOpacity = 1.0f;
	fOffset = 0.0f;
	(void)__xgeSvgAttrLengthPercent(pTag, pTagEnd, "offset", &fOffset, NULL);
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "style", &pValue, &iValueLen) ) {
		__xgeSvgParseStopStyle(pValue, iValueLen, &iColor, &fOpacity);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "stop-color", &pValue, &iValueLen) ) {
		__xgeSvgApplyStopProperty("stop-color", 10, pValue, iValueLen, &iColor, &fOpacity);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "stop-opacity", &pValue, &iValueLen) ) {
		__xgeSvgApplyStopProperty("stop-opacity", 12, pValue, iValueLen, &iColor, &fOpacity);
	}
	iColor = __xgeSvgColorApplyOpacity(iColor, fOpacity);
	return __xgeSvgAddRadialGradientStop(pGradient, fOffset, iColor);
}

static void __xgeSvgPathItemFree(xge_svg_path_item_t* pItem)
{
	if ( pItem == NULL ) {
		return;
	}
	if ( pItem->pPath != NULL ) {
		xgePathDestroy(pItem->pPath);
	}
	if ( pItem->pDashPattern != NULL ) {
		xrtFree(pItem->pDashPattern);
	}
	memset(pItem, 0, sizeof(*pItem));
}

static int __xgeSvgReservePaths(xge_svg pSvg, int iNeeded)
{
	xge_svg_path_item_t* pItems;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= pSvg->iPathCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pSvg->iPathCapacity > 0) ? pSvg->iPathCapacity : 4;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pItems = (xge_svg_path_item_t*)xrtRealloc(pSvg->pPaths, sizeof(*pItems) * (size_t)iCapacity);
	if ( pItems == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pItems + pSvg->iPathCapacity, 0, sizeof(*pItems) * (size_t)(iCapacity - pSvg->iPathCapacity));
	pSvg->pPaths = pItems;
	pSvg->iPathCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeSvgAddPath(xge_svg pSvg, xge_path pPath, const xge_svg_parse_context_t* pContext)
{
	xge_svg_path_item_t* pItem;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || !__xgePathValid(pPath) || (pContext == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeSvgReservePaths(pSvg, pSvg->iPathCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pItem = &pSvg->pPaths[pSvg->iPathCount++];
	memset(pItem, 0, sizeof(*pItem));
	pItem->pPath = pPath;
	pItem->tStyle = pContext->tStyle;
	pItem->tStyle.iSize = (uint32_t)sizeof(pItem->tStyle);
	__xgeSvgCopyId(pItem->sId, (int)sizeof(pItem->sId), pContext->sElementId, (int)strlen(pContext->sElementId));
	__xgeSvgCopyId(pItem->sUseGroupId, (int)sizeof(pItem->sUseGroupId), pContext->sUseGroupId, (int)strlen(pContext->sUseGroupId));
	__xgeSvgCopyId(pItem->sFillGradientId, (int)sizeof(pItem->sFillGradientId), pContext->sFillGradientId, (int)strlen(pContext->sFillGradientId));
	__xgeSvgCopyId(pItem->sStrokeGradientId, (int)sizeof(pItem->sStrokeGradientId), pContext->sStrokeGradientId, (int)strlen(pContext->sStrokeGradientId));
	__xgeSvgCopyId(pItem->sClipPathId, (int)sizeof(pItem->sClipPathId), pContext->sClipPathId, (int)strlen(pContext->sClipPathId));
	__xgeSvgCopyId(pItem->sMaskId, (int)sizeof(pItem->sMaskId), pContext->sMaskId, (int)strlen(pContext->sMaskId));
	pItem->fFillOpacity = pContext->fFillOpacity;
	pItem->fStrokeOpacity = pContext->fStrokeOpacity;
	pItem->bHidden = pContext->bHidden;
	pItem->iOrder = pContext->iOrder;
	if ( pContext->iDashCount > 0 ) {
		pItem->pDashPattern = (float*)xrtMalloc(sizeof(float) * (size_t)pContext->iDashCount);
		if ( pItem->pDashPattern == NULL ) {
			pItem->pPath = NULL;
			pSvg->iPathCount--;
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memcpy(pItem->pDashPattern, pContext->arrDash, sizeof(float) * (size_t)pContext->iDashCount);
		pItem->tStyle.pDashPattern = pItem->pDashPattern;
		pItem->tStyle.iDashCount = pContext->iDashCount;
	} else {
		pItem->tStyle.pDashPattern = NULL;
		pItem->tStyle.iDashCount = 0;
	}
	return XGE_OK;
}

static int __xgeSvgAddPathObject(xge_svg pSvg, xge_path pSource, const xge_svg_parse_context_t* pContext)
{
	xge_path pTransformed;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || !__xgePathValid(pSource) || (pContext == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pTransformed = NULL;
	iRet = xgePathCreate(&pTransformed);
	if ( iRet == XGE_OK ) {
		iRet = __xgePathCopyTransformed(pTransformed, pSource, pContext->tTransform);
	}
	if ( iRet != XGE_OK ) {
		xgePathDestroy(pTransformed);
		return iRet;
	}
	iRet = __xgeSvgAddPath(pSvg, pTransformed, pContext);
	if ( iRet != XGE_OK ) {
		xgePathDestroy(pTransformed);
	}
	return iRet;
}

static int __xgeSvgPathItemMatchesUseRef(const xge_svg_path_item_t* pItem, const char* sRefId)
{
	if ( (pItem == NULL) || (sRefId == NULL) || (sRefId[0] == '\0') ) {
		return 0;
	}
	if ( (pItem->sId[0] != '\0') && (strcmp(pItem->sId, sRefId) == 0) ) {
		return 1;
	}
	if ( (pItem->sUseGroupId[0] != '\0') && (strcmp(pItem->sUseGroupId, sRefId) == 0) ) {
		return 1;
	}
	return 0;
}

static int __xgeSvgCopyPathItemPayload(xge_svg_path_item_t* pDst, const xge_svg_path_item_t* pSrc)
{
	if ( (pDst == NULL) || (pSrc == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pDst->tStyle = pSrc->tStyle;
	pDst->tStyle.iSize = (uint32_t)sizeof(pDst->tStyle);
	__xgeSvgCopyId(pDst->sFillGradientId, (int)sizeof(pDst->sFillGradientId), pSrc->sFillGradientId, (int)strlen(pSrc->sFillGradientId));
	__xgeSvgCopyId(pDst->sStrokeGradientId, (int)sizeof(pDst->sStrokeGradientId), pSrc->sStrokeGradientId, (int)strlen(pSrc->sStrokeGradientId));
	__xgeSvgCopyId(pDst->sClipPathId, (int)sizeof(pDst->sClipPathId), pSrc->sClipPathId, (int)strlen(pSrc->sClipPathId));
	__xgeSvgCopyId(pDst->sMaskId, (int)sizeof(pDst->sMaskId), pSrc->sMaskId, (int)strlen(pSrc->sMaskId));
	pDst->fFillOpacity = pSrc->fFillOpacity;
	pDst->fStrokeOpacity = pSrc->fStrokeOpacity;
	pDst->bHidden = 0;
	if ( pSrc->tStyle.iDashCount > 0 ) {
		const float* pDashSource;

		pDashSource = (pSrc->pDashPattern != NULL) ? pSrc->pDashPattern : pSrc->tStyle.pDashPattern;
		if ( pDashSource == NULL ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		pDst->pDashPattern = (float*)xrtMalloc(sizeof(float) * (size_t)pSrc->tStyle.iDashCount);
		if ( pDst->pDashPattern == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memcpy(pDst->pDashPattern, pDashSource, sizeof(float) * (size_t)pSrc->tStyle.iDashCount);
		pDst->tStyle.pDashPattern = pDst->pDashPattern;
		pDst->tStyle.iDashCount = pSrc->tStyle.iDashCount;
	} else {
		pDst->pDashPattern = NULL;
		pDst->tStyle.pDashPattern = NULL;
		pDst->tStyle.iDashCount = 0;
	}
	return XGE_OK;
}

static xge_svg_transform_t __xgeSvgViewBoxMapTransform(xge_rect_t tViewBox, float fWidth, float fHeight, int iAlignX, int iAlignY, int iMode)
{
	xge_svg_transform_t tMap;
	xge_rect_t tViewport;
	float fScaleX;
	float fScaleY;
	float fScale;
	float fContentW;
	float fContentH;
	float fExtraX;
	float fExtraY;

	tMap = __xgeSvgTransformIdentity();
	if ( (tViewBox.fW <= XGE_PATH_EPSILON) || (tViewBox.fH <= XGE_PATH_EPSILON) ||
	     (fWidth <= XGE_PATH_EPSILON) || (fHeight <= XGE_PATH_EPSILON) ) {
		return tMap;
	}
	tViewport = (xge_rect_t){0.0f, 0.0f, fWidth, fHeight};
	if ( iMode != XGE_SVG_ASPECT_NONE ) {
		fScaleX = fWidth / tViewBox.fW;
		fScaleY = fHeight / tViewBox.fH;
		fScale = (iMode == XGE_SVG_ASPECT_SLICE) ?
			((fScaleX > fScaleY) ? fScaleX : fScaleY) :
			((fScaleX < fScaleY) ? fScaleX : fScaleY);
		fContentW = tViewBox.fW * fScale;
		fContentH = tViewBox.fH * fScale;
		fExtraX = fWidth - fContentW;
		fExtraY = fHeight - fContentH;
		tViewport.fW = fContentW;
		tViewport.fH = fContentH;
		if ( iAlignX == XGE_SVG_ASPECT_ALIGN_MID ) {
			tViewport.fX = fExtraX * 0.5f;
		} else if ( iAlignX == XGE_SVG_ASPECT_ALIGN_MAX ) {
			tViewport.fX = fExtraX;
		}
		if ( iAlignY == XGE_SVG_ASPECT_ALIGN_MID ) {
			tViewport.fY = fExtraY * 0.5f;
		} else if ( iAlignY == XGE_SVG_ASPECT_ALIGN_MAX ) {
			tViewport.fY = fExtraY;
		}
	}
	fScaleX = tViewport.fW / tViewBox.fW;
	fScaleY = tViewport.fH / tViewBox.fH;
	tMap.fA = fScaleX;
	tMap.fD = fScaleY;
	tMap.fE = tViewport.fX - (tViewBox.fX * fScaleX);
	tMap.fF = tViewport.fY - (tViewBox.fY * fScaleY);
	return tMap;
}

static xge_svg_transform_t __xgeSvgUseContextTransform(xge_svg pSvg, const char* sRefId, const xge_svg_parse_context_t* pUseContext)
{
	const xge_svg_view_ref_t* pViewRef;
	xge_svg_transform_t tTransform;
	float fWidth;
	float fHeight;

	tTransform = (pUseContext != NULL) ? pUseContext->tTransform : __xgeSvgTransformIdentity();
	pViewRef = __xgeSvgFindViewRef(pSvg, sRefId);
	if ( (pViewRef == NULL) || !pViewRef->bHasViewBox ) {
		return tTransform;
	}
	fWidth = pUseContext->bUseWidth ? pUseContext->fUseWidth : ((pViewRef->fWidth > 0.0f) ? pViewRef->fWidth : pViewRef->tViewBox.fW);
	fHeight = pUseContext->bUseHeight ? pUseContext->fUseHeight : ((pViewRef->fHeight > 0.0f) ? pViewRef->fHeight : pViewRef->tViewBox.fH);
	if ( (fWidth <= XGE_PATH_EPSILON) || (fHeight <= XGE_PATH_EPSILON) ) {
		return tTransform;
	}
	return __xgeSvgTransformMultiply(tTransform,
		__xgeSvgViewBoxMapTransform(pViewRef->tViewBox, fWidth, fHeight, pViewRef->iAspectAlignX, pViewRef->iAspectAlignY, pViewRef->iAspectMode));
}

static int __xgeSvgParseUseElementContext(const char* pTag, const char* pTagEnd, const xge_svg_parse_context_t* pContext, char* sRefId, int iRefIdSize, xge_svg_parse_context_t* pUseContext)
{
	xge_svg_transform_t tTranslate;
	const char* pValue;
	float fX;
	float fY;
	int iValueLen;

	if ( (pTag == NULL) || (pTagEnd == NULL) || (pContext == NULL) || (sRefId == NULL) || (iRefIdSize <= 0) || (pUseContext == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sRefId[0] = '\0';
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "href", &pValue, &iValueLen) ) {
		(void)__xgeSvgParseHrefId(pValue, iValueLen, sRefId, iRefIdSize);
	}
	if ( (sRefId[0] == '\0') && __xgeSvgAttrGet(pTag, pTagEnd, "xlink:href", &pValue, &iValueLen) ) {
		(void)__xgeSvgParseHrefId(pValue, iValueLen, sRefId, iRefIdSize);
	}
	if ( sRefId[0] == '\0' ) {
		return XGE_OK;
	}
	*pUseContext = *pContext;
	__xgeSvgContextFixDashPointer(pUseContext);
	fX = 0.0f;
	fY = 0.0f;
	(void)__xgeSvgAttrFloat(pTag, pTagEnd, "x", &fX);
	(void)__xgeSvgAttrFloat(pTag, pTagEnd, "y", &fY);
	pUseContext->fUseWidth = 0.0f;
	pUseContext->fUseHeight = 0.0f;
	pUseContext->bUseWidth = __xgeSvgAttrFloat(pTag, pTagEnd, "width", &pUseContext->fUseWidth) && (pUseContext->fUseWidth > 0.0f);
	pUseContext->bUseHeight = __xgeSvgAttrFloat(pTag, pTagEnd, "height", &pUseContext->fUseHeight) && (pUseContext->fUseHeight > 0.0f);
	tTranslate = __xgeSvgTransformIdentity();
	tTranslate.fE = fX;
	tTranslate.fF = fY;
	pUseContext->tTransform = __xgeSvgTransformMultiply(pUseContext->tTransform, tTranslate);
	return XGE_OK;
}

static int __xgeSvgCloneUseReference(xge_svg pSvg, const char* sRefId, const xge_svg_parse_context_t* pUseContext, int* pCloneCount)
{
	xge_svg_path_item_t* pSource;
	xge_svg_path_item_t* pItem;
	xge_svg_transform_t tUseTransform;
	int* pSourceIndices;
	int iOriginalCount;
	int iSourceCount;
	int iRet;
	int i;

	if ( !__xgeSvgValid(pSvg) || (sRefId == NULL) || (pUseContext == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pCloneCount != NULL ) {
		*pCloneCount = 0;
	}
	if ( sRefId[0] == '\0' ) {
		return XGE_OK;
	}
	iOriginalCount = pSvg->iPathCount;
	iSourceCount = 0;
	for ( i = 0; i < iOriginalCount; i++ ) {
		if ( __xgeSvgPathItemMatchesUseRef(&pSvg->pPaths[i], sRefId) ) {
			iSourceCount++;
		}
	}
	if ( iSourceCount <= 0 ) {
		return XGE_OK;
	}
	pSourceIndices = (int*)xrtMalloc(sizeof(*pSourceIndices) * (size_t)iSourceCount);
	if ( pSourceIndices == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iSourceCount = 0;
	for ( i = 0; i < iOriginalCount; i++ ) {
		if ( __xgeSvgPathItemMatchesUseRef(&pSvg->pPaths[i], sRefId) ) {
			pSourceIndices[iSourceCount++] = i;
		}
	}
	iRet = __xgeSvgReservePaths(pSvg, pSvg->iPathCount + iSourceCount);
	if ( iRet != XGE_OK ) {
		xrtFree(pSourceIndices);
		return iRet;
	}
	tUseTransform = __xgeSvgUseContextTransform(pSvg, sRefId, pUseContext);
	for ( i = 0; i < iSourceCount; i++ ) {
		pSource = &pSvg->pPaths[pSourceIndices[i]];
		pItem = &pSvg->pPaths[pSvg->iPathCount++];
		memset(pItem, 0, sizeof(*pItem));
		iRet = xgePathCreate(&pItem->pPath);
		if ( iRet == XGE_OK ) {
			iRet = __xgePathCopyTransformed(pItem->pPath, pSource->pPath, tUseTransform);
		}
		if ( iRet == XGE_OK ) {
			iRet = __xgeSvgCopyPathItemPayload(pItem, pSource);
		}
		if ( iRet == XGE_OK ) {
			pItem->iOrder = pUseContext->iOrder;
			__xgeSvgCopyId(pItem->sId, (int)sizeof(pItem->sId), pUseContext->sElementId, (int)strlen(pUseContext->sElementId));
			if ( pUseContext->sClipPathId[0] != '\0' ) {
				__xgeSvgCopyId(pItem->sClipPathId, (int)sizeof(pItem->sClipPathId), pUseContext->sClipPathId, (int)strlen(pUseContext->sClipPathId));
			}
			if ( pUseContext->sMaskId[0] != '\0' ) {
				__xgeSvgCopyId(pItem->sMaskId, (int)sizeof(pItem->sMaskId), pUseContext->sMaskId, (int)strlen(pUseContext->sMaskId));
			}
			pItem->fFillOpacity *= pUseContext->fFillOpacity;
			pItem->fStrokeOpacity *= pUseContext->fStrokeOpacity;
		}
		if ( iRet != XGE_OK ) {
			break;
		}
	}
	if ( iRet != XGE_OK ) {
		while ( pSvg->iPathCount > iOriginalCount ) {
			__xgeSvgPathItemFree(&pSvg->pPaths[pSvg->iPathCount - 1]);
			pSvg->iPathCount--;
		}
	}
	xrtFree(pSourceIndices);
	if ( (iRet == XGE_OK) && (pCloneCount != NULL) ) {
		*pCloneCount = iSourceCount;
	}
	return iRet;
}

static int __xgeSvgAddUseElement(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_parse_context_t* pContext, int* pCloneCount, char* sRefId, int iRefIdSize, xge_svg_parse_context_t* pUseContext)
{
	int iRet;

	if ( pCloneCount != NULL ) {
		*pCloneCount = 0;
	}
	iRet = __xgeSvgParseUseElementContext(pTag, pTagEnd, pContext, sRefId, iRefIdSize, pUseContext);
	if ( (iRet != XGE_OK) || (sRefId == NULL) || (sRefId[0] == '\0') ) {
		return iRet;
	}
	return __xgeSvgCloneUseReference(pSvg, sRefId, pUseContext, pCloneCount);
}

static int __xgeSvgReservePendingUses(xge_svg_pending_use_t** ppItems, int* pCapacity, int iNeeded)
{
	xge_svg_pending_use_t* pItems;
	int iCapacity;

	if ( (ppItems == NULL) || (pCapacity == NULL) || (iNeeded < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeeded <= *pCapacity ) {
		return XGE_OK;
	}
	iCapacity = (*pCapacity > 0) ? *pCapacity : 4;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pItems = (xge_svg_pending_use_t*)xrtRealloc(*ppItems, sizeof(*pItems) * (size_t)iCapacity);
	if ( pItems == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pItems + *pCapacity, 0, sizeof(*pItems) * (size_t)(iCapacity - *pCapacity));
	*ppItems = pItems;
	*pCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeSvgAddPendingUse(xge_svg_pending_use_t** ppItems, int* pCount, int* pCapacity, const char* sRefId, const xge_svg_parse_context_t* pUseContext)
{
	xge_svg_pending_use_t* pItem;
	int iRet;

	if ( (ppItems == NULL) || (pCount == NULL) || (pCapacity == NULL) || (sRefId == NULL) || (pUseContext == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( sRefId[0] == '\0' ) {
		return XGE_OK;
	}
	iRet = __xgeSvgReservePendingUses(ppItems, pCapacity, *pCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pItem = &(*ppItems)[(*pCount)++];
	memset(pItem, 0, sizeof(*pItem));
	__xgeSvgCopyId(pItem->sRefId, (int)sizeof(pItem->sRefId), sRefId, (int)strlen(sRefId));
	pItem->tContext = *pUseContext;
	__xgeSvgContextFixDashPointer(&pItem->tContext);
	return XGE_OK;
}

static int __xgeSvgResolvePendingUses(xge_svg pSvg, const xge_svg_pending_use_t* pItems, int iCount)
{
	int i;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iCount < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < iCount; i++ ) {
		iRet = __xgeSvgCloneUseReference(pSvg, pItems[i].sRefId, &pItems[i].tContext, NULL);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeSvgAddPathFromAttr(xge_svg pSvg, const char* sPathData, int iPathDataLen, const xge_svg_parse_context_t* pContext)
{
	char* sPathCopy;
	xge_path pSource;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (sPathData == NULL) || (iPathDataLen <= 0) || (pContext == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sPathCopy = (char*)xrtMalloc((size_t)iPathDataLen + 1u);
	if ( sPathCopy == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(sPathCopy, sPathData, (size_t)iPathDataLen);
	sPathCopy[iPathDataLen] = '\0';
	pSource = NULL;
	iRet = xgePathCreate(&pSource);
	if ( iRet == XGE_OK ) {
		iRet = xgePathParseSvg(pSource, sPathCopy);
	}
	xrtFree(sPathCopy);
	if ( iRet != XGE_OK ) {
		xgePathDestroy(pSource);
		return iRet;
	}
	iRet = __xgeSvgAddPathObject(pSvg, pSource, pContext);
	xgePathDestroy(pSource);
	return iRet;
}

static int __xgeSvgAddRectElement(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_parse_context_t* pContext)
{
	xge_path pPath;
	xge_vec2_t tCurrent;
	float fX;
	float fY;
	float fW;
	float fH;
	float fRX;
	float fRY;
	int bHasRX;
	int bHasRY;
	int iRet;

	fX = 0.0f;
	fY = 0.0f;
	fRX = 0.0f;
	fRY = 0.0f;
	(void)__xgeSvgAttrFloat(pTag, pTagEnd, "x", &fX);
	(void)__xgeSvgAttrFloat(pTag, pTagEnd, "y", &fY);
	if ( !__xgeSvgAttrFloat(pTag, pTagEnd, "width", &fW) || !__xgeSvgAttrFloat(pTag, pTagEnd, "height", &fH) ) {
		return XGE_OK;
	}
	if ( (fW <= 0.0f) || (fH <= 0.0f) ) {
		return XGE_OK;
	}
	bHasRX = __xgeSvgAttrFloat(pTag, pTagEnd, "rx", &fRX);
	bHasRY = __xgeSvgAttrFloat(pTag, pTagEnd, "ry", &fRY);
	if ( bHasRX && !bHasRY ) fRY = fRX;
	if ( bHasRY && !bHasRX ) fRX = fRY;
	if ( fRX < 0.0f ) fRX = 0.0f;
	if ( fRY < 0.0f ) fRY = 0.0f;
	if ( fRX > fW * 0.5f ) fRX = fW * 0.5f;
	if ( fRY > fH * 0.5f ) fRY = fH * 0.5f;
	pPath = NULL;
	iRet = xgePathCreate(&pPath);
	if ( iRet != XGE_OK ) return iRet;
	if ( (fRX <= 0.000001f) || (fRY <= 0.000001f) ) {
		iRet = xgePathMoveTo(pPath, fX, fY);
		if ( iRet == XGE_OK ) iRet = xgePathLineTo(pPath, fX + fW, fY);
		if ( iRet == XGE_OK ) iRet = xgePathLineTo(pPath, fX + fW, fY + fH);
		if ( iRet == XGE_OK ) iRet = xgePathLineTo(pPath, fX, fY + fH);
		if ( iRet == XGE_OK ) iRet = xgePathClose(pPath);
	} else {
		tCurrent = (xge_vec2_t){fX + fRX, fY};
		iRet = xgePathMoveTo(pPath, tCurrent.fX, tCurrent.fY);
		if ( iRet == XGE_OK ) {
			tCurrent = (xge_vec2_t){fX + fW - fRX, fY};
			iRet = xgePathLineTo(pPath, tCurrent.fX, tCurrent.fY);
		}
		if ( iRet == XGE_OK ) {
			iRet = __xgePathSvgArcTo(pPath, tCurrent, fRX, fRY, 0.0f, 0, 1, fX + fW, fY + fRY);
			tCurrent = (xge_vec2_t){fX + fW, fY + fRY};
		}
		if ( iRet == XGE_OK ) {
			tCurrent = (xge_vec2_t){fX + fW, fY + fH - fRY};
			iRet = xgePathLineTo(pPath, tCurrent.fX, tCurrent.fY);
		}
		if ( iRet == XGE_OK ) {
			iRet = __xgePathSvgArcTo(pPath, tCurrent, fRX, fRY, 0.0f, 0, 1, fX + fW - fRX, fY + fH);
			tCurrent = (xge_vec2_t){fX + fW - fRX, fY + fH};
		}
		if ( iRet == XGE_OK ) {
			tCurrent = (xge_vec2_t){fX + fRX, fY + fH};
			iRet = xgePathLineTo(pPath, tCurrent.fX, tCurrent.fY);
		}
		if ( iRet == XGE_OK ) {
			iRet = __xgePathSvgArcTo(pPath, tCurrent, fRX, fRY, 0.0f, 0, 1, fX, fY + fH - fRY);
			tCurrent = (xge_vec2_t){fX, fY + fH - fRY};
		}
		if ( iRet == XGE_OK ) {
			tCurrent = (xge_vec2_t){fX, fY + fRY};
			iRet = xgePathLineTo(pPath, tCurrent.fX, tCurrent.fY);
		}
		if ( iRet == XGE_OK ) {
			iRet = __xgePathSvgArcTo(pPath, tCurrent, fRX, fRY, 0.0f, 0, 1, fX + fRX, fY);
		}
		if ( iRet == XGE_OK ) iRet = xgePathClose(pPath);
	}
	if ( iRet == XGE_OK ) {
		iRet = __xgeSvgAddPathObject(pSvg, pPath, pContext);
	}
	xgePathDestroy(pPath);
	return iRet;
}

static int __xgeSvgAddEllipseElement(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_parse_context_t* pContext, int bCircle)
{
	xge_path pPath;
	xge_vec2_t tCurrent;
	float fCX;
	float fCY;
	float fRX;
	float fRY;
	float fR;
	int iRet;

	fCX = 0.0f;
	fCY = 0.0f;
	(void)__xgeSvgAttrFloat(pTag, pTagEnd, "cx", &fCX);
	(void)__xgeSvgAttrFloat(pTag, pTagEnd, "cy", &fCY);
	if ( bCircle ) {
		if ( !__xgeSvgAttrFloat(pTag, pTagEnd, "r", &fR) || (fR <= 0.0f) ) return XGE_OK;
		fRX = fR;
		fRY = fR;
	} else {
		if ( !__xgeSvgAttrFloat(pTag, pTagEnd, "rx", &fRX) || !__xgeSvgAttrFloat(pTag, pTagEnd, "ry", &fRY) || (fRX <= 0.0f) || (fRY <= 0.0f) ) return XGE_OK;
	}
	pPath = NULL;
	iRet = xgePathCreate(&pPath);
	if ( iRet != XGE_OK ) return iRet;
	tCurrent = (xge_vec2_t){fCX + fRX, fCY};
	iRet = xgePathMoveTo(pPath, tCurrent.fX, tCurrent.fY);
	if ( iRet == XGE_OK ) {
		iRet = __xgePathSvgArcTo(pPath, tCurrent, fRX, fRY, 0.0f, 1, 1, fCX - fRX, fCY);
		tCurrent = (xge_vec2_t){fCX - fRX, fCY};
	}
	if ( iRet == XGE_OK ) {
		iRet = __xgePathSvgArcTo(pPath, tCurrent, fRX, fRY, 0.0f, 1, 1, fCX + fRX, fCY);
	}
	if ( iRet == XGE_OK ) iRet = xgePathClose(pPath);
	if ( iRet == XGE_OK ) {
		iRet = __xgeSvgAddPathObject(pSvg, pPath, pContext);
	}
	xgePathDestroy(pPath);
	return iRet;
}

static int __xgeSvgAddLineElement(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_parse_context_t* pContext)
{
	xge_path pPath;
	float fX1;
	float fY1;
	float fX2;
	float fY2;
	int iRet;

	fX1 = 0.0f;
	fY1 = 0.0f;
	fX2 = 0.0f;
	fY2 = 0.0f;
	(void)__xgeSvgAttrFloat(pTag, pTagEnd, "x1", &fX1);
	(void)__xgeSvgAttrFloat(pTag, pTagEnd, "y1", &fY1);
	(void)__xgeSvgAttrFloat(pTag, pTagEnd, "x2", &fX2);
	(void)__xgeSvgAttrFloat(pTag, pTagEnd, "y2", &fY2);
	pPath = NULL;
	iRet = xgePathCreate(&pPath);
	if ( iRet != XGE_OK ) return iRet;
	iRet = xgePathMoveTo(pPath, fX1, fY1);
	if ( iRet == XGE_OK ) iRet = xgePathLineTo(pPath, fX2, fY2);
	if ( iRet == XGE_OK ) iRet = __xgeSvgAddPathObject(pSvg, pPath, pContext);
	xgePathDestroy(pPath);
	return iRet;
}

static int __xgeSvgAddPointsElement(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_parse_context_t* pContext, int bClose)
{
	const char* pValue;
	const char* pEnd;
	xge_path pPath;
	float fX;
	float fY;
	int iValueLen;
	int iCount;
	int iRet;

	if ( !__xgeSvgAttrGet(pTag, pTagEnd, "points", &pValue, &iValueLen) ) {
		return XGE_OK;
	}
	pEnd = pValue + iValueLen;
	pPath = NULL;
	iRet = xgePathCreate(&pPath);
	if ( iRet != XGE_OK ) return iRet;
	iCount = 0;
	while ( __xgeSvgReadFloat(&pValue, pEnd, &fX) && __xgeSvgReadFloat(&pValue, pEnd, &fY) ) {
		if ( iCount == 0 ) {
			iRet = xgePathMoveTo(pPath, fX, fY);
		} else {
			iRet = xgePathLineTo(pPath, fX, fY);
		}
		if ( iRet != XGE_OK ) {
			break;
		}
		iCount++;
	}
	if ( (iRet == XGE_OK) && bClose && (iCount > 0) ) {
		iRet = xgePathClose(pPath);
	}
	if ( (iRet == XGE_OK) && (iCount > 0) ) {
		iRet = __xgeSvgAddPathObject(pSvg, pPath, pContext);
	}
	xgePathDestroy(pPath);
	return iRet;
}

static void __xgeSvgParseViewBox(xge_svg pSvg, const char* pTag, const char* pTagEnd)
{
	const char* pValue;
	const char* pEnd;
	float fX;
	float fY;
	float fW;
	float fH;
	int iValueLen;

	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "width", &pValue, &iValueLen) ) {
		pEnd = pValue + iValueLen;
		(void)__xgeSvgReadFloat(&pValue, pEnd, &pSvg->fWidth);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "height", &pValue, &iValueLen) ) {
		pEnd = pValue + iValueLen;
		(void)__xgeSvgReadFloat(&pValue, pEnd, &pSvg->fHeight);
	}
	if ( __xgeSvgAttrGet(pTag, pTagEnd, "preserveAspectRatio", &pValue, &iValueLen) ) {
		(void)__xgeSvgParsePreserveAspectRatio(pSvg, pValue, iValueLen);
	}
	if ( !__xgeSvgAttrGet(pTag, pTagEnd, "viewBox", &pValue, &iValueLen) ) {
		if ( (pSvg->fWidth > 0.0f) && (pSvg->fHeight > 0.0f) && !pSvg->bHasViewBox ) {
			pSvg->tViewBox = (xge_rect_t){0.0f, 0.0f, pSvg->fWidth, pSvg->fHeight};
			pSvg->bHasViewBox = 1;
		}
		return;
	}
	pEnd = pValue + iValueLen;
	if ( __xgeSvgReadFloat(&pValue, pEnd, &fX) &&
	     __xgeSvgReadFloat(&pValue, pEnd, &fY) &&
	     __xgeSvgReadFloat(&pValue, pEnd, &fW) &&
	     __xgeSvgReadFloat(&pValue, pEnd, &fH) &&
	     (fW > 0.0f) && (fH > 0.0f) ) {
		pSvg->tViewBox = (xge_rect_t){fX, fY, fW, fH};
		pSvg->bHasViewBox = 1;
	}
}

static int __xgeSvgComputeBounds(xge_svg pSvg)
{
	xge_rect_t tBounds;
	int bHasBounds;
	int i;

	if ( !__xgeSvgValid(pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	bHasBounds = 0;
	memset(&tBounds, 0, sizeof(tBounds));
	for ( i = 0; i < pSvg->iPathCount; i++ ) {
		int iPointCount;
		int j;
		xge_vec2_t* pPoints;
		float fMinX;
		float fMinY;
		float fMaxX;
		float fMaxY;

		if ( pSvg->pPaths[i].bHidden ) {
			continue;
		}
		iPointCount = xgePathFlatten(pSvg->pPaths[i].pPath, NULL, 0, 1.0f);
		if ( iPointCount <= 0 ) {
			continue;
		}
		pPoints = (xge_vec2_t*)xrtMalloc(sizeof(*pPoints) * (size_t)iPointCount);
		if ( pPoints == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		(void)xgePathFlatten(pSvg->pPaths[i].pPath, pPoints, iPointCount, 1.0f);
		fMinX = pPoints[0].fX;
		fMinY = pPoints[0].fY;
		fMaxX = pPoints[0].fX;
		fMaxY = pPoints[0].fY;
		for ( j = 1; j < iPointCount; j++ ) {
			if ( pPoints[j].fX < fMinX ) fMinX = pPoints[j].fX;
			if ( pPoints[j].fY < fMinY ) fMinY = pPoints[j].fY;
			if ( pPoints[j].fX > fMaxX ) fMaxX = pPoints[j].fX;
			if ( pPoints[j].fY > fMaxY ) fMaxY = pPoints[j].fY;
		}
		xrtFree(pPoints);
		if ( !bHasBounds ) {
			tBounds = (xge_rect_t){fMinX, fMinY, fMaxX - fMinX, fMaxY - fMinY};
			bHasBounds = 1;
		} else {
			float fLeft = (fMinX < tBounds.fX) ? fMinX : tBounds.fX;
			float fTop = (fMinY < tBounds.fY) ? fMinY : tBounds.fY;
			float fRight = (fMaxX > (tBounds.fX + tBounds.fW)) ? fMaxX : (tBounds.fX + tBounds.fW);
			float fBottom = (fMaxY > (tBounds.fY + tBounds.fH)) ? fMaxY : (tBounds.fY + tBounds.fH);

			tBounds = (xge_rect_t){fLeft, fTop, fRight - fLeft, fBottom - fTop};
		}
	}
	if ( bHasBounds && (tBounds.fW > 0.0f) && (tBounds.fH > 0.0f) ) {
		pSvg->tViewBox = tBounds;
		pSvg->bHasViewBox = 1;
	}
	return XGE_OK;
}

static int __xgeSvgParseDocument(xge_svg pSvg, const char* sText)
{
	xge_svg_parse_context_t arrStack[XGE_SVG_CONTEXT_STACK_MAX];
	xge_svg_parse_context_t arrClipStack[XGE_SVG_CONTEXT_STACK_MAX];
	xge_svg_parse_context_t arrMaskStack[XGE_SVG_CONTEXT_STACK_MAX];
	xge_svg_parse_context_t arrDefsStack[XGE_SVG_CONTEXT_STACK_MAX];
	xge_svg_pending_use_t* pPendingUses;
	xge_svg_linear_gradient_t* pCurrentLinearGradient;
	xge_svg_radial_gradient_t* pCurrentRadialGradient;
	xge_svg_clip_path_t* pCurrentClipPath;
	xge_svg_mask_t* pCurrentMask;
	const char* pText;
	int iDepth;
	int iClipDepth;
	int iMaskDepth;
	int iDefsDepth;
	int iPendingUseCount;
	int iPendingUseCapacity;
	int iDocumentOrder;
	int iRet;
	int bInsideClipPath;
	int bInsideMask;
	int bInsideDefs;

	if ( !__xgeSvgValid(pSvg) || (sText == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = xgeSvgClear(pSvg);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	__xgeSvgContextDefault(&arrStack[0]);
	__xgeSvgContextDefault(&arrClipStack[0]);
	__xgeSvgContextDefault(&arrMaskStack[0]);
	__xgeSvgContextDefault(&arrDefsStack[0]);
	pPendingUses = NULL;
	pCurrentLinearGradient = NULL;
	pCurrentRadialGradient = NULL;
	pCurrentClipPath = NULL;
	pCurrentMask = NULL;
	bInsideClipPath = 0;
	bInsideMask = 0;
	bInsideDefs = 0;
	iDepth = 1;
	iClipDepth = 0;
	iMaskDepth = 0;
	iDefsDepth = 0;
	iPendingUseCount = 0;
	iPendingUseCapacity = 0;
	iDocumentOrder = 0;
	pText = sText;
	while ( *pText != '\0' ) {
		const char* pOpen;
		const char* pTagEnd;
		const char* pName;
		const char* pNameEnd;
		const char* pAttrValue;
		int iAttrValueLen;
		int iNameLen;
		int bClosing;
		int bSelfClosing;

		pOpen = strchr(pText, '<');
		if ( pOpen == NULL ) {
			break;
		}
		if ( strncmp(pOpen, "<!--", 4) == 0 ) {
			const char* pCommentEnd = strstr(pOpen + 4, "-->");
			pText = (pCommentEnd != NULL) ? (pCommentEnd + 3) : (pOpen + 4);
			continue;
		}
		pTagEnd = __xgeSvgFindTagEnd(pOpen + 1);
		if ( pTagEnd == NULL ) {
			break;
		}
		pText = pTagEnd + 1;
		pName = pOpen + 1;
		while ( (pName < pTagEnd) && __xgeSvgCharSpace(*pName) ) pName++;
		if ( (pName >= pTagEnd) || (*pName == '?') || (*pName == '!') ) {
			continue;
		}
		bClosing = 0;
		if ( *pName == '/' ) {
			bClosing = 1;
			pName++;
			while ( (pName < pTagEnd) && __xgeSvgCharSpace(*pName) ) pName++;
		}
		pNameEnd = pName;
		while ( (pNameEnd < pTagEnd) && !__xgeSvgCharSpace(*pNameEnd) && (*pNameEnd != '/') ) pNameEnd++;
		iNameLen = (int)(pNameEnd - pName);
		if ( iNameLen <= 0 ) {
			continue;
		}
		if ( bClosing ) {
			if ( bInsideClipPath ) {
				if ( __xgeSvgTagNameMatches(pName, iNameLen, "clipPath") ) {
					bInsideClipPath = 0;
					pCurrentClipPath = NULL;
					iClipDepth = 0;
				} else if ( (__xgeSvgTagNameMatches(pName, iNameLen, "g") || __xgeSvgTagNameMatches(pName, iNameLen, "svg")) && (iClipDepth > 1) ) {
					iClipDepth--;
				}
				continue;
			}
			if ( bInsideMask ) {
				if ( __xgeSvgTagNameMatches(pName, iNameLen, "mask") ) {
					bInsideMask = 0;
					pCurrentMask = NULL;
					iMaskDepth = 0;
				} else if ( (__xgeSvgTagNameMatches(pName, iNameLen, "g") || __xgeSvgTagNameMatches(pName, iNameLen, "svg")) && (iMaskDepth > 1) ) {
					iMaskDepth--;
				}
				continue;
			}
			if ( __xgeSvgTagNameMatches(pName, iNameLen, "linearGradient") ) {
				pCurrentLinearGradient = NULL;
			}
			if ( __xgeSvgTagNameMatches(pName, iNameLen, "radialGradient") ) {
				pCurrentRadialGradient = NULL;
			}
			if ( bInsideDefs ) {
				if ( __xgeSvgTagNameMatches(pName, iNameLen, "defs") ) {
					bInsideDefs = 0;
					iDefsDepth = 0;
				} else if ( (__xgeSvgTagNameMatches(pName, iNameLen, "g") ||
				             __xgeSvgTagNameMatches(pName, iNameLen, "svg") ||
				             __xgeSvgTagNameMatches(pName, iNameLen, "symbol")) &&
				            (iDefsDepth > 1) ) {
					iDefsDepth--;
				}
				continue;
			}
			if ( (__xgeSvgTagNameMatches(pName, iNameLen, "g") || __xgeSvgTagNameMatches(pName, iNameLen, "svg")) && (iDepth > 1) ) {
				iDepth--;
			}
			continue;
		}
		bSelfClosing = __xgeSvgTagSelfClosing(pNameEnd, pTagEnd);
		if ( __xgeSvgTagNameMatches(pName, iNameLen, "style") ) {
			if ( !bSelfClosing ) {
				const char* pClose;

				pClose = __xgeSvgFindClosingElement(pText, "style");
				if ( pClose != NULL ) {
					const char* pCloseEnd;

					iRet = __xgeSvgParseCssStyleSheet(pSvg, pText, (int)(pClose - pText));
					if ( iRet != XGE_OK ) {
						(void)xgeSvgClear(pSvg);
						if ( pPendingUses != NULL ) xrtFree(pPendingUses);
						return iRet;
					}
					pCloseEnd = __xgeSvgFindTagEnd(pClose + 1);
					pText = (pCloseEnd != NULL) ? (pCloseEnd + 1) : (pClose + 1);
				}
			}
			continue;
		}
		if ( __xgeSvgTagNameMatches(pName, iNameLen, "defs") ) {
			arrDefsStack[0] = __xgeSvgElementContext(pSvg, &arrStack[iDepth - 1], pName, iNameLen, pNameEnd, pTagEnd);
			arrDefsStack[0].bHidden = 1;
			__xgeSvgContextFixDashPointer(&arrDefsStack[0]);
			iDefsDepth = 1;
			bInsideDefs = !bSelfClosing;
		} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "clipPath") ) {
				arrClipStack[0] = __xgeSvgElementContext(pSvg, &arrStack[iDepth - 1], pName, iNameLen, pNameEnd, pTagEnd);
				__xgeSvgContextFixDashPointer(&arrClipStack[0]);
				iClipDepth = 1;
				pCurrentClipPath = __xgeSvgBeginClipPath(pSvg, pNameEnd, pTagEnd, &iRet);
			bInsideClipPath = 1;
			pCurrentLinearGradient = NULL;
			pCurrentRadialGradient = NULL;
			if ( iRet != XGE_OK ) {
				(void)xgeSvgClear(pSvg);
				if ( pPendingUses != NULL ) xrtFree(pPendingUses);
				return iRet;
			}
			if ( bSelfClosing ) {
				bInsideClipPath = 0;
				pCurrentClipPath = NULL;
				iClipDepth = 0;
			}
		} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "mask") ) {
			arrMaskStack[0] = __xgeSvgElementContext(pSvg, &arrStack[iDepth - 1], pName, iNameLen, pNameEnd, pTagEnd);
			__xgeSvgContextFixDashPointer(&arrMaskStack[0]);
			iMaskDepth = 1;
			pCurrentMask = __xgeSvgBeginMask(pSvg, pNameEnd, pTagEnd, &iRet);
			bInsideMask = 1;
			pCurrentLinearGradient = NULL;
			pCurrentRadialGradient = NULL;
			if ( iRet != XGE_OK ) {
				(void)xgeSvgClear(pSvg);
				if ( pPendingUses != NULL ) xrtFree(pPendingUses);
				return iRet;
			}
			if ( bSelfClosing ) {
				bInsideMask = 0;
				pCurrentMask = NULL;
				iMaskDepth = 0;
			}
		} else if ( bInsideClipPath ) {
			if ( __xgeSvgTagNameMatches(pName, iNameLen, "g") || __xgeSvgTagNameMatches(pName, iNameLen, "svg") ) {
				xge_svg_parse_context_t tChild;

				tChild = __xgeSvgElementContext(pSvg, &arrClipStack[(iClipDepth > 0) ? (iClipDepth - 1) : 0], pName, iNameLen, pNameEnd, pTagEnd);
				if ( !bSelfClosing && (iClipDepth < XGE_SVG_CONTEXT_STACK_MAX) ) {
					arrClipStack[iClipDepth++] = tChild;
					__xgeSvgContextFixDashPointer(&arrClipStack[iClipDepth - 1]);
				}
			} else if ( (pCurrentClipPath != NULL) && __xgeSvgTagNameMatches(pName, iNameLen, "rect") ) {
				xge_svg_parse_context_t tClipShapeContext;

				tClipShapeContext = __xgeSvgElementContext(pSvg, &arrClipStack[(iClipDepth > 0) ? (iClipDepth - 1) : 0], pName, iNameLen, pNameEnd, pTagEnd);
				iRet = __xgeSvgAddClipPathRect(pCurrentClipPath, pNameEnd, pTagEnd, &tClipShapeContext);
				if ( iRet != XGE_OK ) {
					(void)xgeSvgClear(pSvg);
					if ( pPendingUses != NULL ) xrtFree(pPendingUses);
					return iRet;
				}
			}
		} else if ( bInsideMask ) {
			if ( __xgeSvgTagNameMatches(pName, iNameLen, "g") || __xgeSvgTagNameMatches(pName, iNameLen, "svg") ) {
				xge_svg_parse_context_t tChild;

				tChild = __xgeSvgElementContext(pSvg, &arrMaskStack[(iMaskDepth > 0) ? (iMaskDepth - 1) : 0], pName, iNameLen, pNameEnd, pTagEnd);
				if ( !bSelfClosing && (iMaskDepth < XGE_SVG_CONTEXT_STACK_MAX) ) {
					arrMaskStack[iMaskDepth++] = tChild;
					__xgeSvgContextFixDashPointer(&arrMaskStack[iMaskDepth - 1]);
				}
			} else if ( (pCurrentMask != NULL) && __xgeSvgTagNameMatches(pName, iNameLen, "rect") ) {
				xge_svg_parse_context_t tMaskShapeContext;

				tMaskShapeContext = __xgeSvgElementContext(pSvg, &arrMaskStack[(iMaskDepth > 0) ? (iMaskDepth - 1) : 0], pName, iNameLen, pNameEnd, pTagEnd);
				iRet = __xgeSvgAddMaskRect(pCurrentMask, pNameEnd, pTagEnd, &tMaskShapeContext);
				if ( iRet != XGE_OK ) {
					(void)xgeSvgClear(pSvg);
					if ( pPendingUses != NULL ) xrtFree(pPendingUses);
					return iRet;
				}
			}
		} else if ( !bInsideDefs && (__xgeSvgTagNameMatches(pName, iNameLen, "svg") || __xgeSvgTagNameMatches(pName, iNameLen, "g")) ) {
			xge_svg_parse_context_t tChild;

			if ( __xgeSvgTagNameMatches(pName, iNameLen, "svg") ) {
				__xgeSvgParseViewBox(pSvg, pNameEnd, pTagEnd);
			}
			tChild = __xgeSvgElementContext(pSvg, &arrStack[iDepth - 1], pName, iNameLen, pNameEnd, pTagEnd);
			if ( !bSelfClosing && (iDepth < XGE_SVG_CONTEXT_STACK_MAX) ) {
				arrStack[iDepth++] = tChild;
				__xgeSvgContextFixDashPointer(&arrStack[iDepth - 1]);
			}
		} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "linearGradient") ) {
			pCurrentLinearGradient = __xgeSvgBeginLinearGradient(pSvg, pNameEnd, pTagEnd, &iRet);
			pCurrentRadialGradient = NULL;
			if ( iRet != XGE_OK ) {
				(void)xgeSvgClear(pSvg);
				if ( pPendingUses != NULL ) xrtFree(pPendingUses);
				return iRet;
			}
			if ( bSelfClosing ) {
				pCurrentLinearGradient = NULL;
			}
		} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "radialGradient") ) {
			pCurrentRadialGradient = __xgeSvgBeginRadialGradient(pSvg, pNameEnd, pTagEnd, &iRet);
			pCurrentLinearGradient = NULL;
			if ( iRet != XGE_OK ) {
				(void)xgeSvgClear(pSvg);
				if ( pPendingUses != NULL ) xrtFree(pPendingUses);
				return iRet;
			}
			if ( bSelfClosing ) {
				pCurrentRadialGradient = NULL;
			}
		} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "stop") ) {
			if ( pCurrentLinearGradient != NULL ) {
				iRet = __xgeSvgParseGradientStop(pCurrentLinearGradient, pNameEnd, pTagEnd);
			} else {
				iRet = __xgeSvgParseRadialGradientStop(pCurrentRadialGradient, pNameEnd, pTagEnd);
			}
			if ( iRet != XGE_OK ) {
				(void)xgeSvgClear(pSvg);
				if ( pPendingUses != NULL ) xrtFree(pPendingUses);
				return iRet;
			}
		} else if ( bInsideDefs ) {
			if ( __xgeSvgTagNameMatches(pName, iNameLen, "g") ||
			     __xgeSvgTagNameMatches(pName, iNameLen, "svg") ||
			     __xgeSvgTagNameMatches(pName, iNameLen, "symbol") ) {
				xge_svg_parse_context_t tChild;

				if ( __xgeSvgTagNameMatches(pName, iNameLen, "svg") ||
				     __xgeSvgTagNameMatches(pName, iNameLen, "symbol") ) {
					iRet = __xgeSvgAddViewRef(pSvg, pNameEnd, pTagEnd);
					if ( iRet != XGE_OK ) {
						(void)xgeSvgClear(pSvg);
						if ( pPendingUses != NULL ) xrtFree(pPendingUses);
						return iRet;
					}
				}
				tChild = __xgeSvgElementContext(pSvg, &arrDefsStack[(iDefsDepth > 0) ? (iDefsDepth - 1) : 0], pName, iNameLen, pNameEnd, pTagEnd);
				tChild.bHidden = 1;
				if ( tChild.sElementId[0] != '\0' ) {
					__xgeSvgCopyId(tChild.sUseGroupId, (int)sizeof(tChild.sUseGroupId), tChild.sElementId, (int)strlen(tChild.sElementId));
				}
				if ( !bSelfClosing && (iDefsDepth < XGE_SVG_CONTEXT_STACK_MAX) ) {
					arrDefsStack[iDefsDepth++] = tChild;
					__xgeSvgContextFixDashPointer(&arrDefsStack[iDefsDepth - 1]);
				}
			} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "path") ) {
				xge_svg_parse_context_t tDefContext;

				tDefContext = __xgeSvgElementContext(pSvg, &arrDefsStack[(iDefsDepth > 0) ? (iDefsDepth - 1) : 0], pName, iNameLen, pNameEnd, pTagEnd);
				tDefContext.bHidden = 1;
				if ( __xgeSvgAttrGet(pNameEnd, pTagEnd, "d", &pAttrValue, &iAttrValueLen) ) {
					iRet = __xgeSvgAddPathFromAttr(pSvg, pAttrValue, iAttrValueLen, &tDefContext);
					if ( iRet != XGE_OK ) {
						(void)xgeSvgClear(pSvg);
						if ( pPendingUses != NULL ) xrtFree(pPendingUses);
						return iRet;
					}
				}
			} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "rect") ||
			            __xgeSvgTagNameMatches(pName, iNameLen, "circle") ||
			            __xgeSvgTagNameMatches(pName, iNameLen, "ellipse") ||
			            __xgeSvgTagNameMatches(pName, iNameLen, "line") ||
			            __xgeSvgTagNameMatches(pName, iNameLen, "polyline") ||
			            __xgeSvgTagNameMatches(pName, iNameLen, "polygon") ) {
				xge_svg_parse_context_t tDefContext;

				tDefContext = __xgeSvgElementContext(pSvg, &arrDefsStack[(iDefsDepth > 0) ? (iDefsDepth - 1) : 0], pName, iNameLen, pNameEnd, pTagEnd);
				tDefContext.bHidden = 1;
				if ( __xgeSvgTagNameMatches(pName, iNameLen, "rect") ) {
					iRet = __xgeSvgAddRectElement(pSvg, pNameEnd, pTagEnd, &tDefContext);
				} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "circle") ) {
					iRet = __xgeSvgAddEllipseElement(pSvg, pNameEnd, pTagEnd, &tDefContext, 1);
				} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "ellipse") ) {
					iRet = __xgeSvgAddEllipseElement(pSvg, pNameEnd, pTagEnd, &tDefContext, 0);
				} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "line") ) {
					iRet = __xgeSvgAddLineElement(pSvg, pNameEnd, pTagEnd, &tDefContext);
				} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "polyline") ) {
					iRet = __xgeSvgAddPointsElement(pSvg, pNameEnd, pTagEnd, &tDefContext, 0);
				} else {
					iRet = __xgeSvgAddPointsElement(pSvg, pNameEnd, pTagEnd, &tDefContext, 1);
				}
				if ( iRet != XGE_OK ) {
					(void)xgeSvgClear(pSvg);
					if ( pPendingUses != NULL ) xrtFree(pPendingUses);
					return iRet;
				}
			}
		} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "use") ) {
			xge_svg_parse_context_t tUseContext;
			char sUseRefId[XGE_SVG_ID_MAX];
			int iUseCloneCount;

			tUseContext = __xgeSvgElementContext(pSvg, &arrStack[iDepth - 1], pName, iNameLen, pNameEnd, pTagEnd);
			tUseContext.iOrder = iDocumentOrder++;
			iUseCloneCount = 0;
			iRet = __xgeSvgAddUseElement(pSvg, pNameEnd, pTagEnd, &tUseContext, &iUseCloneCount, sUseRefId, (int)sizeof(sUseRefId), &tUseContext);
			if ( iRet != XGE_OK ) {
				(void)xgeSvgClear(pSvg);
				if ( pPendingUses != NULL ) xrtFree(pPendingUses);
				return iRet;
			}
			if ( (iUseCloneCount == 0) && (sUseRefId[0] != '\0') ) {
				iRet = __xgeSvgAddPendingUse(&pPendingUses, &iPendingUseCount, &iPendingUseCapacity, sUseRefId, &tUseContext);
				if ( iRet != XGE_OK ) {
					(void)xgeSvgClear(pSvg);
					if ( pPendingUses != NULL ) xrtFree(pPendingUses);
					return iRet;
				}
			}
		} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "path") ) {
			xge_svg_parse_context_t tPathContext;

			tPathContext = __xgeSvgElementContext(pSvg, &arrStack[iDepth - 1], pName, iNameLen, pNameEnd, pTagEnd);
			tPathContext.iOrder = iDocumentOrder++;
			if ( __xgeSvgAttrGet(pNameEnd, pTagEnd, "d", &pAttrValue, &iAttrValueLen) ) {
				iRet = __xgeSvgAddPathFromAttr(pSvg, pAttrValue, iAttrValueLen, &tPathContext);
				if ( iRet != XGE_OK ) {
					(void)xgeSvgClear(pSvg);
					if ( pPendingUses != NULL ) xrtFree(pPendingUses);
					return iRet;
				}
			}
		} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "rect") ||
		            __xgeSvgTagNameMatches(pName, iNameLen, "circle") ||
		            __xgeSvgTagNameMatches(pName, iNameLen, "ellipse") ||
		            __xgeSvgTagNameMatches(pName, iNameLen, "line") ||
		            __xgeSvgTagNameMatches(pName, iNameLen, "polyline") ||
		            __xgeSvgTagNameMatches(pName, iNameLen, "polygon") ) {
			xge_svg_parse_context_t tShapeContext;

			tShapeContext = __xgeSvgElementContext(pSvg, &arrStack[iDepth - 1], pName, iNameLen, pNameEnd, pTagEnd);
			tShapeContext.iOrder = iDocumentOrder++;
			if ( __xgeSvgTagNameMatches(pName, iNameLen, "rect") ) {
				iRet = __xgeSvgAddRectElement(pSvg, pNameEnd, pTagEnd, &tShapeContext);
			} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "circle") ) {
				iRet = __xgeSvgAddEllipseElement(pSvg, pNameEnd, pTagEnd, &tShapeContext, 1);
			} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "ellipse") ) {
				iRet = __xgeSvgAddEllipseElement(pSvg, pNameEnd, pTagEnd, &tShapeContext, 0);
			} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "line") ) {
				iRet = __xgeSvgAddLineElement(pSvg, pNameEnd, pTagEnd, &tShapeContext);
			} else if ( __xgeSvgTagNameMatches(pName, iNameLen, "polyline") ) {
				iRet = __xgeSvgAddPointsElement(pSvg, pNameEnd, pTagEnd, &tShapeContext, 0);
			} else {
				iRet = __xgeSvgAddPointsElement(pSvg, pNameEnd, pTagEnd, &tShapeContext, 1);
			}
			if ( iRet != XGE_OK ) {
				(void)xgeSvgClear(pSvg);
				if ( pPendingUses != NULL ) xrtFree(pPendingUses);
				return iRet;
			}
		}
	}
	iRet = __xgeSvgResolveGradientHrefs(pSvg);
	if ( iRet != XGE_OK ) {
		(void)xgeSvgClear(pSvg);
		if ( pPendingUses != NULL ) xrtFree(pPendingUses);
		return iRet;
	}
	if ( iPendingUseCount > 0 ) {
		iRet = __xgeSvgResolvePendingUses(pSvg, pPendingUses, iPendingUseCount);
		if ( iRet != XGE_OK ) {
			(void)xgeSvgClear(pSvg);
			if ( pPendingUses != NULL ) xrtFree(pPendingUses);
			return iRet;
		}
	}
	if ( pPendingUses != NULL ) {
		xrtFree(pPendingUses);
		pPendingUses = NULL;
	}
	if ( !pSvg->bHasViewBox ) {
		iRet = __xgeSvgComputeBounds(pSvg);
		if ( iRet != XGE_OK ) {
			(void)xgeSvgClear(pSvg);
			if ( pPendingUses != NULL ) xrtFree(pPendingUses);
			return iRet;
		}
	}
	return XGE_OK;
}

static xge_svg_cache_entry_t* __xgeSvgCacheFind(const char* sURI, xge_svg_cache_entry_t** ppPrev)
{
	xge_svg_cache_entry_t* pPrev;
	xge_svg_cache_entry_t* pEntry;

	if ( ppPrev != NULL ) {
		*ppPrev = NULL;
	}
	if ( sURI == NULL ) {
		return NULL;
	}
	pPrev = NULL;
	pEntry = g_xgeSvgCacheHead;
	while ( pEntry != NULL ) {
		if ( (pEntry->sURI != NULL) && (strcmp(pEntry->sURI, sURI) == 0) ) {
			if ( ppPrev != NULL ) {
				*ppPrev = pPrev;
			}
			return pEntry;
		}
		pPrev = pEntry;
		pEntry = pEntry->pNext;
	}
	return NULL;
}

int xgeSvgCreate(xge_svg* ppSvg)
{
	xge_svg pSvg;

	if ( ppSvg == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppSvg = NULL;
	pSvg = (xge_svg)xrtCalloc(1, sizeof(*pSvg));
	if ( pSvg == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSvg->iMagic = XGE_SVG_MAGIC;
	pSvg->iRefCount = 1;
	__xgeSvgAspectDefault(pSvg);
	*ppSvg = pSvg;
	return XGE_OK;
}

void xgeSvgDestroy(xge_svg pSvg)
{
	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	if ( pSvg->iRefCount > 1 ) {
		pSvg->iRefCount--;
		return;
	}
	(void)xgeSvgClear(pSvg);
	pSvg->iMagic = 0;
	pSvg->iRefCount = 0;
	xrtFree(pSvg);
}

int xgeSvgClear(xge_svg pSvg)
{
	int i;

	if ( !__xgeSvgValid(pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pSvg->iPathCount; i++ ) {
		__xgeSvgPathItemFree(&pSvg->pPaths[i]);
	}
	if ( pSvg->pPaths != NULL ) {
		xrtFree(pSvg->pPaths);
	}
	for ( i = 0; i < pSvg->iLinearGradientCount; i++ ) {
		__xgeSvgLinearGradientFree(&pSvg->pLinearGradients[i]);
	}
	if ( pSvg->pLinearGradients != NULL ) {
		xrtFree(pSvg->pLinearGradients);
	}
	for ( i = 0; i < pSvg->iRadialGradientCount; i++ ) {
		__xgeSvgRadialGradientFree(&pSvg->pRadialGradients[i]);
	}
	for ( i = 0; i < pSvg->iClipPathCount; i++ ) {
		if ( pSvg->pClipPaths[i].pRects != NULL ) {
			xrtFree(pSvg->pClipPaths[i].pRects);
		}
	}
	for ( i = 0; i < pSvg->iMaskCount; i++ ) {
		if ( pSvg->pMasks[i].pRects != NULL ) {
			xrtFree(pSvg->pMasks[i].pRects);
		}
	}
	if ( pSvg->pRadialGradients != NULL ) {
		xrtFree(pSvg->pRadialGradients);
	}
	if ( pSvg->pClipPaths != NULL ) {
		xrtFree(pSvg->pClipPaths);
	}
	if ( pSvg->pMasks != NULL ) {
		xrtFree(pSvg->pMasks);
	}
	if ( pSvg->pViewRefs != NULL ) {
		xrtFree(pSvg->pViewRefs);
	}
	for ( i = 0; i < pSvg->iStyleRuleCount; i++ ) {
		if ( pSvg->pStyleRules[i].sStyle != NULL ) {
			xrtFree(pSvg->pStyleRules[i].sStyle);
		}
	}
	if ( pSvg->pStyleRules != NULL ) {
		xrtFree(pSvg->pStyleRules);
	}
	pSvg->pPaths = NULL;
	pSvg->iPathCount = 0;
	pSvg->iPathCapacity = 0;
	pSvg->pLinearGradients = NULL;
	pSvg->iLinearGradientCount = 0;
	pSvg->iLinearGradientCapacity = 0;
	pSvg->pRadialGradients = NULL;
	pSvg->iRadialGradientCount = 0;
	pSvg->iRadialGradientCapacity = 0;
	pSvg->pClipPaths = NULL;
	pSvg->iClipPathCount = 0;
	pSvg->iClipPathCapacity = 0;
	pSvg->pMasks = NULL;
	pSvg->iMaskCount = 0;
	pSvg->iMaskCapacity = 0;
	pSvg->pViewRefs = NULL;
	pSvg->iViewRefCount = 0;
	pSvg->iViewRefCapacity = 0;
	pSvg->pStyleRules = NULL;
	pSvg->iStyleRuleCount = 0;
	pSvg->iStyleRuleCapacity = 0;
	pSvg->bHasViewBox = 0;
	pSvg->tViewBox = (xge_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	pSvg->fWidth = 0.0f;
	pSvg->fHeight = 0.0f;
	__xgeSvgAspectDefault(pSvg);
	return XGE_OK;
}

int xgeSvgLoadMemory(xge_svg pSvg, const void* pData, int iSize)
{
	char* sText;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pData == NULL) || (iSize < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sText = (char*)xrtMalloc((size_t)iSize + 1u);
	if ( sText == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( iSize > 0 ) {
		memcpy(sText, pData, (size_t)iSize);
	}
	sText[iSize] = '\0';
	iRet = __xgeSvgParseDocument(pSvg, sText);
	xrtFree(sText);
	return iRet;
}

int xgeSvgLoad(xge_svg pSvg, const char* sURI)
{
	xge_resource_t tResource;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (sURI == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tResource, 0, sizeof(tResource));
	iRet = xgeResourceLoad(sURI, &tResource);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeSvgLoadMemory(pSvg, tResource.pData, tResource.iSize);
	xgeResourceFree(&tResource);
	return iRet;
}

int xgeSvgLoadCached(const char* sURI, xge_svg* ppSvg)
{
	xge_svg_cache_entry_t* pEntry;
	xge_svg pSvg;
	char* sKey;
	int iRet;

	if ( (sURI == NULL) || (ppSvg == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppSvg = NULL;
	pEntry = __xgeSvgCacheFind(sURI, NULL);
	if ( pEntry != NULL && __xgeSvgValid(pEntry->pSvg) ) {
		iRet = xgeSvgAddRef(pEntry->pSvg);
		if ( iRet <= 0 ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		*ppSvg = pEntry->pSvg;
		return XGE_OK;
	}
	iRet = xgeSvgCreate(&pSvg);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeSvgLoad(pSvg, sURI);
	if ( iRet != XGE_OK ) {
		xgeSvgDestroy(pSvg);
		return iRet;
	}
	sKey = __xgeSvgStringDup(sURI);
	if ( sKey == NULL ) {
		xgeSvgDestroy(pSvg);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pEntry = (xge_svg_cache_entry_t*)xrtCalloc(1, sizeof(*pEntry));
	if ( pEntry == NULL ) {
		xrtFree(sKey);
		xgeSvgDestroy(pSvg);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pEntry->sURI = sKey;
	pEntry->pSvg = pSvg;
	pEntry->pNext = g_xgeSvgCacheHead;
	g_xgeSvgCacheHead = pEntry;
	iRet = xgeSvgAddRef(pSvg);
	if ( iRet <= 0 ) {
		xgeSvgCacheInvalidate(sURI);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppSvg = pSvg;
	return XGE_OK;
}

int xgeSvgAddRef(xge_svg pSvg)
{
	if ( !__xgeSvgValid(pSvg) ) {
		return 0;
	}
	if ( pSvg->iRefCount < INT32_MAX ) {
		pSvg->iRefCount++;
	}
	return pSvg->iRefCount;
}

int xgeSvgCacheInvalidate(const char* sURI)
{
	xge_svg_cache_entry_t* pPrev;
	xge_svg_cache_entry_t* pEntry;

	if ( sURI == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pEntry = __xgeSvgCacheFind(sURI, &pPrev);
	if ( pEntry == NULL ) {
		return XGE_ERROR_NOT_FOUND;
	}
	if ( pPrev != NULL ) {
		pPrev->pNext = pEntry->pNext;
	} else {
		g_xgeSvgCacheHead = pEntry->pNext;
	}
	xgeSvgDestroy(pEntry->pSvg);
	xrtFree(pEntry->sURI);
	xrtFree(pEntry);
	return XGE_OK;
}

void xgeSvgCacheClear(void)
{
	xge_svg_cache_entry_t* pEntry;
	xge_svg_cache_entry_t* pNext;

	pEntry = g_xgeSvgCacheHead;
	g_xgeSvgCacheHead = NULL;
	while ( pEntry != NULL ) {
		pNext = pEntry->pNext;
		xgeSvgDestroy(pEntry->pSvg);
		xrtFree(pEntry->sURI);
		xrtFree(pEntry);
		pEntry = pNext;
	}
}

int xgeSvgGetViewBox(xge_svg pSvg, xge_rect_t* pViewBox)
{
	if ( !__xgeSvgValid(pSvg) || (pViewBox == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !pSvg->bHasViewBox ) {
		return XGE_ERROR_NOT_FOUND;
	}
	*pViewBox = pSvg->tViewBox;
	return XGE_OK;
}

int xgeSvgGetPathCount(xge_svg pSvg)
{
	int i;
	int iCount;

	if ( !__xgeSvgValid(pSvg) ) {
		return 0;
	}
	iCount = 0;
	for ( i = 0; i < pSvg->iPathCount; i++ ) {
		if ( !pSvg->pPaths[i].bHidden ) {
			iCount++;
		}
	}
	return iCount;
}

static int __xgeSvgVisiblePathBefore(xge_svg pSvg, int iA, int iB)
{
	if ( pSvg->pPaths[iA].iOrder != pSvg->pPaths[iB].iOrder ) {
		return pSvg->pPaths[iA].iOrder < pSvg->pPaths[iB].iOrder;
	}
	return iA < iB;
}

static int __xgeSvgGetVisiblePathStorageIndex(xge_svg pSvg, int iIndex)
{
	int i;

	if ( !__xgeSvgValid(pSvg) || (iIndex < 0) ) {
		return -1;
	}
	for ( i = 0; i < pSvg->iPathCount; i++ ) {
		int j;
		int iRank;

		if ( pSvg->pPaths[i].bHidden ) {
			continue;
		}
		iRank = 0;
		for ( j = 0; j < pSvg->iPathCount; j++ ) {
			if ( pSvg->pPaths[j].bHidden || (j == i) ) {
				continue;
			}
			if ( __xgeSvgVisiblePathBefore(pSvg, j, i) ) {
				iRank++;
			}
		}
		if ( iRank == iIndex ) {
			return i;
		}
	}
	return -1;
}

static xge_svg_path_item_t* __xgeSvgGetVisiblePathItem(xge_svg pSvg, int iIndex)
{
	int iStorageIndex;

	iStorageIndex = __xgeSvgGetVisiblePathStorageIndex(pSvg, iIndex);
	if ( iStorageIndex < 0 ) {
		return NULL;
	}
	return &pSvg->pPaths[iStorageIndex];
}

int xgeSvgGetPathInfo(xge_svg pSvg, int iIndex, xge_svg_path_info_t* pInfo)
{
	xge_svg_path_item_t* pItem;

	if ( !__xgeSvgValid(pSvg) || (pInfo == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pItem = __xgeSvgGetVisiblePathItem(pSvg, iIndex);
	if ( pItem == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pInfo, 0, sizeof(*pInfo));
	pInfo->iSize = (uint32_t)sizeof(*pInfo);
	pInfo->pPath = pItem->pPath;
	pInfo->tStyle = pItem->tStyle;
	pInfo->sFillGradientId = (pItem->sFillGradientId[0] != '\0') ? pItem->sFillGradientId : NULL;
	pInfo->sStrokeGradientId = (pItem->sStrokeGradientId[0] != '\0') ? pItem->sStrokeGradientId : NULL;
	pInfo->sClipPathId = (pItem->sClipPathId[0] != '\0') ? pItem->sClipPathId : NULL;
	pInfo->sMaskId = (pItem->sMaskId[0] != '\0') ? pItem->sMaskId : NULL;
	return XGE_OK;
}

int xgeSvgSetPreserveAspectRatio(xge_svg pSvg, const char* sValue)
{
	if ( !__xgeSvgValid(pSvg) || (sValue == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeSvgParsePreserveAspectRatio(pSvg, sValue, (int)strlen(sValue));
}

int xgeSvgGetDrawViewport(xge_svg pSvg, xge_rect_t tDst, xge_rect_t* pViewport)
{
	float fScaleX;
	float fScaleY;
	float fScale;
	float fContentW;
	float fContentH;
	float fExtraX;
	float fExtraY;

	if ( !__xgeSvgValid(pSvg) || (pViewport == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !pSvg->bHasViewBox || (pSvg->tViewBox.fW <= 0.0f) || (pSvg->tViewBox.fH <= 0.0f) ) {
		return XGE_ERROR_NOT_FOUND;
	}
	if ( (tDst.fW == 0.0f) || (tDst.fH == 0.0f) || (pSvg->iAspectMode == XGE_SVG_ASPECT_NONE) ) {
		*pViewport = tDst;
		return XGE_OK;
	}
	fScaleX = tDst.fW / pSvg->tViewBox.fW;
	fScaleY = tDst.fH / pSvg->tViewBox.fH;
	if ( pSvg->iAspectMode == XGE_SVG_ASPECT_SLICE ) {
		fScale = (fScaleX > fScaleY) ? fScaleX : fScaleY;
	} else {
		fScale = (fScaleX < fScaleY) ? fScaleX : fScaleY;
	}
	fContentW = pSvg->tViewBox.fW * fScale;
	fContentH = pSvg->tViewBox.fH * fScale;
	fExtraX = tDst.fW - fContentW;
	fExtraY = tDst.fH - fContentH;
	*pViewport = tDst;
	pViewport->fW = fContentW;
	pViewport->fH = fContentH;
	if ( pSvg->iAspectAlignX == XGE_SVG_ASPECT_ALIGN_MID ) {
		pViewport->fX = tDst.fX + (fExtraX * 0.5f);
	} else if ( pSvg->iAspectAlignX == XGE_SVG_ASPECT_ALIGN_MAX ) {
		pViewport->fX = tDst.fX + fExtraX;
	}
	if ( pSvg->iAspectAlignY == XGE_SVG_ASPECT_ALIGN_MID ) {
		pViewport->fY = tDst.fY + (fExtraY * 0.5f);
	} else if ( pSvg->iAspectAlignY == XGE_SVG_ASPECT_ALIGN_MAX ) {
		pViewport->fY = tDst.fY + fExtraY;
	}
	return XGE_OK;
}

static int __xgePathGetBounds(xge_path pPath, float fTolerance, xge_rect_t* pBounds)
{
	xge_vec2_t* pPoints;
	int iPointCount;
	int i;
	float fMinX;
	float fMinY;
	float fMaxX;
	float fMaxY;

	if ( !__xgePathValid(pPath) || (pBounds == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iPointCount = xgePathFlatten(pPath, NULL, 0, fTolerance);
	if ( iPointCount <= 0 ) {
		return XGE_ERROR_NOT_FOUND;
	}
	pPoints = (xge_vec2_t*)xrtMalloc(sizeof(*pPoints) * (size_t)iPointCount);
	if ( pPoints == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	(void)xgePathFlatten(pPath, pPoints, iPointCount, fTolerance);
	fMinX = pPoints[0].fX;
	fMinY = pPoints[0].fY;
	fMaxX = pPoints[0].fX;
	fMaxY = pPoints[0].fY;
	for ( i = 1; i < iPointCount; i++ ) {
		if ( pPoints[i].fX < fMinX ) fMinX = pPoints[i].fX;
		if ( pPoints[i].fY < fMinY ) fMinY = pPoints[i].fY;
		if ( pPoints[i].fX > fMaxX ) fMaxX = pPoints[i].fX;
		if ( pPoints[i].fY > fMaxY ) fMaxY = pPoints[i].fY;
	}
	xrtFree(pPoints);
	if ( (fMaxX - fMinX <= XGE_PATH_EPSILON) || (fMaxY - fMinY <= XGE_PATH_EPSILON) ) {
		return XGE_ERROR_NOT_FOUND;
	}
	*pBounds = (xge_rect_t){fMinX, fMinY, fMaxX - fMinX, fMaxY - fMinY};
	return XGE_OK;
}

static uint32_t __xgeSvgColorLerp(uint32_t iA, uint32_t iB, float fT)
{
	float fInv;
	int r;
	int g;
	int b;
	int a;

	fT = __xgeSvgClamp01(fT);
	fInv = 1.0f - fT;
	r = (int)((float)XGE_COLOR_GET_R(iA) * fInv + (float)XGE_COLOR_GET_R(iB) * fT + 0.5f);
	g = (int)((float)XGE_COLOR_GET_G(iA) * fInv + (float)XGE_COLOR_GET_G(iB) * fT + 0.5f);
	b = (int)((float)XGE_COLOR_GET_B(iA) * fInv + (float)XGE_COLOR_GET_B(iB) * fT + 0.5f);
	a = (int)((float)XGE_COLOR_GET_A(iA) * fInv + (float)XGE_COLOR_GET_A(iB) * fT + 0.5f);
	if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
	if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
	if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;
	if ( a < 0 ) a = 0; else if ( a > 255 ) a = 255;
	return ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | (uint32_t)a;
}

static float __xgeSvgGradientSpreadT(float fT, int iSpreadMethod)
{
	float fBase;

	if ( iSpreadMethod == XGE_SVG_SPREAD_REPEAT ) {
		fT = fT - floorf(fT);
		if ( fT < 0.0f ) fT += 1.0f;
		return fT;
	}
	if ( iSpreadMethod == XGE_SVG_SPREAD_REFLECT ) {
		fBase = floorf(fT * 0.5f);
		fT = fT - fBase * 2.0f;
		if ( fT < 0.0f ) fT += 2.0f;
		if ( fT > 1.0f ) fT = 2.0f - fT;
		return __xgeSvgClamp01(fT);
	}
	return __xgeSvgClamp01(fT);
}

static uint32_t __xgeSvgLinearGradientColor(const xge_svg_linear_gradient_t* pGradient, float fT, float fOpacity)
{
	const xge_svg_gradient_stop_t* pA;
	const xge_svg_gradient_stop_t* pB;
	uint32_t iColor;
	float fLocalT;
	int i;

	if ( (pGradient == NULL) || (pGradient->iStopCount <= 0) ) {
		return 0x00000000u;
	}
	fT = __xgeSvgGradientSpreadT(fT, pGradient->iSpreadMethod);
	if ( (pGradient->iStopCount == 1) || (fT <= pGradient->pStops[0].fOffset) ) {
		return __xgeSvgColorApplyOpacity(pGradient->pStops[0].iColor, fOpacity);
	}
	if ( fT >= pGradient->pStops[pGradient->iStopCount - 1].fOffset ) {
		return __xgeSvgColorApplyOpacity(pGradient->pStops[pGradient->iStopCount - 1].iColor, fOpacity);
	}
	for ( i = 0; i + 1 < pGradient->iStopCount; i++ ) {
		pA = &pGradient->pStops[i];
		pB = &pGradient->pStops[i + 1];
		if ( (fT >= pA->fOffset) && (fT <= pB->fOffset) ) {
			if ( fabsf(pB->fOffset - pA->fOffset) <= XGE_PATH_EPSILON ) {
				iColor = pB->iColor;
			} else {
				fLocalT = (fT - pA->fOffset) / (pB->fOffset - pA->fOffset);
				iColor = __xgeSvgColorLerp(pA->iColor, pB->iColor, fLocalT);
			}
			return __xgeSvgColorApplyOpacity(iColor, fOpacity);
		}
	}
	return __xgeSvgColorApplyOpacity(pGradient->pStops[pGradient->iStopCount - 1].iColor, fOpacity);
}

static void __xgeSvgApplyLinearGradientColors(xge_shape_vertex_t* pVertices, int iVertexCount, const xge_svg_linear_gradient_t* pGradient, xge_vec2_t tA, xge_vec2_t tB, float fOpacity)
{
	float fDX;
	float fDY;
	float fLen2;
	int i;

	if ( (pVertices == NULL) || (pGradient == NULL) || (iVertexCount <= 0) ) {
		return;
	}
	fDX = tB.fX - tA.fX;
	fDY = tB.fY - tA.fY;
	fLen2 = fDX * fDX + fDY * fDY;
	for ( i = 0; i < iVertexCount; i++ ) {
		float fAlphaScale;
		float fT;

		fAlphaScale = (float)XGE_COLOR_GET_A(pVertices[i].iColor) / 255.0f;
		if ( fAlphaScale <= 0.0f ) {
			pVertices[i].iColor = 0x00000000u;
			continue;
		}
		if ( fLen2 <= XGE_PATH_EPSILON ) {
			fT = 0.0f;
		} else {
			fT = ((pVertices[i].fX - tA.fX) * fDX + (pVertices[i].fY - tA.fY) * fDY) / fLen2;
		}
		pVertices[i].iColor = __xgeSvgLinearGradientColor(pGradient, fT, fOpacity * fAlphaScale);
	}
}

static int __xgeSvgDrawLinearGradientFill(xge_path pPath, const xge_svg_linear_gradient_t* pGradient, xge_svg_transform_t tMap, int iFillRule, float fTolerance, float fFillOpacity, int bScreenSpace)
{
	xge_shape_vertex_t* pVertices;
	uint32_t* pIndices;
	xge_rect_t tBounds;
	xge_vec2_t tA;
	xge_vec2_t tB;
	int iVertexCount;
	int iIndexCount;
	int iRet;

	if ( !__xgePathValid(pPath) || (pGradient == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pGradient->iStopCount <= 0 ) {
		return XGE_OK;
	}
	iRet = __xgePathGetBounds(pPath, fTolerance, &tBounds);
	if ( iRet != XGE_OK ) {
		return (iRet == XGE_ERROR_NOT_FOUND) ? XGE_OK : iRet;
	}
	iVertexCount = 0;
	iIndexCount = 0;
	iRet = __xgePathBuildFillMeshWithRule(pPath, NULL, 0, NULL, 0, 0xffffffffu, iFillRule, fTolerance, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount <= 0) || (iIndexCount <= 0) ) {
		return iRet;
	}
	pVertices = (xge_shape_vertex_t*)xrtMalloc(sizeof(*pVertices) * (size_t)iVertexCount);
	pIndices = (uint32_t*)xrtMalloc(sizeof(*pIndices) * (size_t)iIndexCount);
	if ( (pVertices == NULL) || (pIndices == NULL) ) {
		if ( pVertices != NULL ) xrtFree(pVertices);
		if ( pIndices != NULL ) xrtFree(pIndices);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xgePathBuildFillMeshWithRule(pPath, pVertices, iVertexCount, pIndices, iIndexCount, 0xffffffffu, iFillRule, fTolerance, &iVertexCount, &iIndexCount);
	if ( iRet != XGE_OK ) {
		xrtFree(pIndices);
		xrtFree(pVertices);
		return iRet;
	}
	if ( pGradient->iUnits == XGE_SVG_GRADIENT_USER_SPACE ) {
		tA = __xgeSvgTransformPoint(tMap, __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fX1, pGradient->fY1}));
		tB = __xgeSvgTransformPoint(tMap, __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fX2, pGradient->fY2}));
	} else {
		xge_vec2_t tLocalA = __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fX1, pGradient->fY1});
		xge_vec2_t tLocalB = __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fX2, pGradient->fY2});

		tA = (xge_vec2_t){tBounds.fX + tLocalA.fX * tBounds.fW, tBounds.fY + tLocalA.fY * tBounds.fH};
		tB = (xge_vec2_t){tBounds.fX + tLocalB.fX * tBounds.fW, tBounds.fY + tLocalB.fY * tBounds.fH};
	}
	if ( bScreenSpace ) {
		xge_shape_vertex_t* pAAVertices;
		uint32_t* pAAIndices;
		int iAAVertexCount;
		int iAAIndexCount;

		iAAVertexCount = 0;
		iAAIndexCount = 0;
		iRet = __xgePathBuildFillAAMeshWithRule(pPath, NULL, 0, NULL, 0, 0xffffffffu, iFillRule, fTolerance, 1.0f, &iAAVertexCount, &iAAIndexCount);
		if ( iRet != XGE_OK ) {
			xrtFree(pIndices);
			xrtFree(pVertices);
			return iRet;
		}
		if ( (iAAVertexCount > 0) && (iAAIndexCount > 0) ) {
			pAAVertices = (xge_shape_vertex_t*)xrtMalloc(sizeof(*pAAVertices) * (size_t)iAAVertexCount);
			pAAIndices = (uint32_t*)xrtMalloc(sizeof(*pAAIndices) * (size_t)iAAIndexCount);
			if ( (pAAVertices == NULL) || (pAAIndices == NULL) ) {
				if ( pAAVertices != NULL ) xrtFree(pAAVertices);
				if ( pAAIndices != NULL ) xrtFree(pAAIndices);
				xrtFree(pIndices);
				xrtFree(pVertices);
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			iRet = __xgePathBuildFillAAMeshWithRule(pPath, pAAVertices, iAAVertexCount, pAAIndices, iAAIndexCount, 0xffffffffu, iFillRule, fTolerance, 1.0f, &iAAVertexCount, &iAAIndexCount);
			if ( iRet == XGE_OK ) {
				__xgeSvgApplyLinearGradientColors(pAAVertices, iAAVertexCount, pGradient, tA, tB, fFillOpacity);
				iRet = xgeShapeMeshFillPx(pAAVertices, iAAVertexCount, pAAIndices, iAAIndexCount);
			}
			xrtFree(pAAIndices);
			xrtFree(pAAVertices);
			if ( iRet != XGE_OK ) {
				xrtFree(pIndices);
				xrtFree(pVertices);
				return iRet;
			}
		}
	}
	__xgeSvgApplyLinearGradientColors(pVertices, iVertexCount, pGradient, tA, tB, fFillOpacity);
	iRet = bScreenSpace ? xgeShapeMeshFillPx(pVertices, iVertexCount, pIndices, iIndexCount) : xgeShapeMeshFill(pVertices, iVertexCount, pIndices, iIndexCount);
	xrtFree(pIndices);
	xrtFree(pVertices);
	return iRet;
}

static uint32_t __xgeSvgRadialGradientColor(const xge_svg_radial_gradient_t* pGradient, float fT, float fOpacity)
{
	const xge_svg_gradient_stop_t* pA;
	const xge_svg_gradient_stop_t* pB;
	uint32_t iColor;
	float fLocalT;
	int i;

	if ( (pGradient == NULL) || (pGradient->iStopCount <= 0) ) {
		return 0x00000000u;
	}
	fT = __xgeSvgGradientSpreadT(fT, pGradient->iSpreadMethod);
	if ( (pGradient->iStopCount == 1) || (fT <= pGradient->pStops[0].fOffset) ) {
		return __xgeSvgColorApplyOpacity(pGradient->pStops[0].iColor, fOpacity);
	}
	if ( fT >= pGradient->pStops[pGradient->iStopCount - 1].fOffset ) {
		return __xgeSvgColorApplyOpacity(pGradient->pStops[pGradient->iStopCount - 1].iColor, fOpacity);
	}
	for ( i = 0; i + 1 < pGradient->iStopCount; i++ ) {
		pA = &pGradient->pStops[i];
		pB = &pGradient->pStops[i + 1];
		if ( (fT >= pA->fOffset) && (fT <= pB->fOffset) ) {
			if ( fabsf(pB->fOffset - pA->fOffset) <= XGE_PATH_EPSILON ) {
				iColor = pB->iColor;
			} else {
				fLocalT = (fT - pA->fOffset) / (pB->fOffset - pA->fOffset);
				iColor = __xgeSvgColorLerp(pA->iColor, pB->iColor, fLocalT);
			}
			return __xgeSvgColorApplyOpacity(iColor, fOpacity);
		}
	}
	return __xgeSvgColorApplyOpacity(pGradient->pStops[pGradient->iStopCount - 1].iColor, fOpacity);
}

static void __xgeSvgApplyRadialGradientColors(xge_shape_vertex_t* pVertices, int iVertexCount, const xge_svg_radial_gradient_t* pGradient, xge_rect_t tBounds, xge_vec2_t tFocus, float fRadius, float fOpacity, int bObjectBoundingBox)
{
	int i;

	if ( (pVertices == NULL) || (pGradient == NULL) || (iVertexCount <= 0) ) {
		return;
	}
	for ( i = 0; i < iVertexCount; i++ ) {
		float fAlphaScale;
		float fX;
		float fY;
		float fDX;
		float fDY;
		float fT;

		fAlphaScale = (float)XGE_COLOR_GET_A(pVertices[i].iColor) / 255.0f;
		if ( fAlphaScale <= 0.0f ) {
			pVertices[i].iColor = 0x00000000u;
			continue;
		}
		if ( bObjectBoundingBox ) {
			fX = (pVertices[i].fX - tBounds.fX) / tBounds.fW;
			fY = (pVertices[i].fY - tBounds.fY) / tBounds.fH;
		} else {
			fX = pVertices[i].fX;
			fY = pVertices[i].fY;
		}
		fDX = fX - tFocus.fX;
		fDY = fY - tFocus.fY;
		fT = (fRadius > XGE_PATH_EPSILON) ? (sqrtf(fDX * fDX + fDY * fDY) / fRadius) : 0.0f;
		pVertices[i].iColor = __xgeSvgRadialGradientColor(pGradient, fT, fOpacity * fAlphaScale);
	}
}

static int __xgeSvgDrawRadialGradientFill(xge_path pPath, const xge_svg_radial_gradient_t* pGradient, xge_svg_transform_t tMap, int iFillRule, float fTolerance, float fFillOpacity, int bScreenSpace)
{
	xge_shape_vertex_t* pVertices;
	uint32_t* pIndices;
	xge_rect_t tBounds;
	xge_vec2_t tCenter;
	xge_vec2_t tFocus;
	float fRadius;
	int iVertexCount;
	int iIndexCount;
	int iRet;

	if ( !__xgePathValid(pPath) || (pGradient == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pGradient->iStopCount <= 0) || (pGradient->fR <= XGE_PATH_EPSILON) ) {
		return XGE_OK;
	}
	iRet = __xgePathGetBounds(pPath, fTolerance, &tBounds);
	if ( iRet != XGE_OK ) {
		return (iRet == XGE_ERROR_NOT_FOUND) ? XGE_OK : iRet;
	}
	iVertexCount = 0;
	iIndexCount = 0;
	iRet = __xgePathBuildFillMeshWithRule(pPath, NULL, 0, NULL, 0, 0xffffffffu, iFillRule, fTolerance, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount <= 0) || (iIndexCount <= 0) ) {
		return iRet;
	}
	pVertices = (xge_shape_vertex_t*)xrtMalloc(sizeof(*pVertices) * (size_t)iVertexCount);
	pIndices = (uint32_t*)xrtMalloc(sizeof(*pIndices) * (size_t)iIndexCount);
	if ( (pVertices == NULL) || (pIndices == NULL) ) {
		if ( pVertices != NULL ) xrtFree(pVertices);
		if ( pIndices != NULL ) xrtFree(pIndices);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xgePathBuildFillMeshWithRule(pPath, pVertices, iVertexCount, pIndices, iIndexCount, 0xffffffffu, iFillRule, fTolerance, &iVertexCount, &iIndexCount);
	if ( iRet != XGE_OK ) {
		xrtFree(pIndices);
		xrtFree(pVertices);
		return iRet;
	}
	if ( pGradient->iUnits == XGE_SVG_GRADIENT_USER_SPACE ) {
		xge_vec2_t tRadiusPoint;

		tCenter = __xgeSvgTransformPoint(tMap, __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fCX, pGradient->fCY}));
		tFocus = __xgeSvgTransformPoint(tMap, __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fFX, pGradient->fFY}));
		tRadiusPoint = __xgeSvgTransformPoint(tMap, __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fCX + pGradient->fR, pGradient->fCY}));
		fRadius = sqrtf((tRadiusPoint.fX - tCenter.fX) * (tRadiusPoint.fX - tCenter.fX) + (tRadiusPoint.fY - tCenter.fY) * (tRadiusPoint.fY - tCenter.fY));
		(void)tCenter;
	} else {
		xge_vec2_t tRadiusPoint;

		tCenter = __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fCX, pGradient->fCY});
		tFocus = __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fFX, pGradient->fFY});
		tRadiusPoint = __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fCX + pGradient->fR, pGradient->fCY});
		fRadius = sqrtf((tRadiusPoint.fX - tCenter.fX) * (tRadiusPoint.fX - tCenter.fX) + (tRadiusPoint.fY - tCenter.fY) * (tRadiusPoint.fY - tCenter.fY));
		(void)tCenter;
	}
	if ( bScreenSpace ) {
		xge_shape_vertex_t* pAAVertices;
		uint32_t* pAAIndices;
		int iAAVertexCount;
		int iAAIndexCount;

		iAAVertexCount = 0;
		iAAIndexCount = 0;
		iRet = __xgePathBuildFillAAMeshWithRule(pPath, NULL, 0, NULL, 0, 0xffffffffu, iFillRule, fTolerance, 1.0f, &iAAVertexCount, &iAAIndexCount);
		if ( iRet != XGE_OK ) {
			xrtFree(pIndices);
			xrtFree(pVertices);
			return iRet;
		}
		if ( (iAAVertexCount > 0) && (iAAIndexCount > 0) ) {
			pAAVertices = (xge_shape_vertex_t*)xrtMalloc(sizeof(*pAAVertices) * (size_t)iAAVertexCount);
			pAAIndices = (uint32_t*)xrtMalloc(sizeof(*pAAIndices) * (size_t)iAAIndexCount);
			if ( (pAAVertices == NULL) || (pAAIndices == NULL) ) {
				if ( pAAVertices != NULL ) xrtFree(pAAVertices);
				if ( pAAIndices != NULL ) xrtFree(pAAIndices);
				xrtFree(pIndices);
				xrtFree(pVertices);
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			iRet = __xgePathBuildFillAAMeshWithRule(pPath, pAAVertices, iAAVertexCount, pAAIndices, iAAIndexCount, 0xffffffffu, iFillRule, fTolerance, 1.0f, &iAAVertexCount, &iAAIndexCount);
			if ( iRet == XGE_OK ) {
				__xgeSvgApplyRadialGradientColors(pAAVertices, iAAVertexCount, pGradient, tBounds, tFocus, fRadius, fFillOpacity, pGradient->iUnits != XGE_SVG_GRADIENT_USER_SPACE);
				iRet = xgeShapeMeshFillPx(pAAVertices, iAAVertexCount, pAAIndices, iAAIndexCount);
			}
			xrtFree(pAAIndices);
			xrtFree(pAAVertices);
			if ( iRet != XGE_OK ) {
				xrtFree(pIndices);
				xrtFree(pVertices);
				return iRet;
			}
		}
	}
	__xgeSvgApplyRadialGradientColors(pVertices, iVertexCount, pGradient, tBounds, tFocus, fRadius, fFillOpacity, pGradient->iUnits != XGE_SVG_GRADIENT_USER_SPACE);
	iRet = bScreenSpace ? xgeShapeMeshFillPx(pVertices, iVertexCount, pIndices, iIndexCount) : xgeShapeMeshFill(pVertices, iVertexCount, pIndices, iIndexCount);
	xrtFree(pIndices);
	xrtFree(pVertices);
	return iRet;
}

static int __xgeSvgDrawLinearGradientStroke(xge_path pPath, const xge_svg_linear_gradient_t* pGradient, xge_svg_transform_t tMap, const xge_path_style_t* pStyle, float fTolerance, float fStrokeOpacity, int bScreenSpace)
{
	xge_shape_vertex_t* pVertices;
	uint32_t* pIndices;
	xge_rect_t tBounds;
	xge_vec2_t tA;
	xge_vec2_t tB;
	int iVertexCount;
	int iIndexCount;
	int iLineJoin;
	int iLineCap;
	int iRet;

	if ( !__xgePathValid(pPath) || (pGradient == NULL) || (pStyle == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pGradient->iStopCount <= 0) || (pStyle->fStrokeWidth <= 0.0f) ) {
		return XGE_OK;
	}
	iRet = __xgePathGetBounds(pPath, fTolerance, &tBounds);
	if ( iRet != XGE_OK ) {
		return (iRet == XGE_ERROR_NOT_FOUND) ? XGE_OK : iRet;
	}
	iLineJoin = (pStyle->iLineJoin == XGE_PATH_JOIN_BEVEL || pStyle->iLineJoin == XGE_PATH_JOIN_ROUND) ? pStyle->iLineJoin : XGE_PATH_JOIN_MITER;
	iLineCap = (pStyle->iLineCap == XGE_PATH_CAP_SQUARE || pStyle->iLineCap == XGE_PATH_CAP_ROUND) ? pStyle->iLineCap : XGE_PATH_CAP_BUTT;
	iVertexCount = 0;
	iIndexCount = 0;
	iRet = __xgePathBuildDashedStrokeMeshWithStyle(pPath, NULL, 0, NULL, 0, pStyle->fStrokeWidth, 0xffffffffu, iLineJoin, iLineCap, pStyle->pDashPattern, pStyle->iDashCount, pStyle->fDashOffset, fTolerance, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount <= 0) || (iIndexCount <= 0) ) {
		return iRet;
	}
	pVertices = (xge_shape_vertex_t*)xrtMalloc(sizeof(*pVertices) * (size_t)iVertexCount);
	pIndices = (uint32_t*)xrtMalloc(sizeof(*pIndices) * (size_t)iIndexCount);
	if ( (pVertices == NULL) || (pIndices == NULL) ) {
		if ( pVertices != NULL ) xrtFree(pVertices);
		if ( pIndices != NULL ) xrtFree(pIndices);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xgePathBuildDashedStrokeMeshWithStyle(pPath, pVertices, iVertexCount, pIndices, iIndexCount, pStyle->fStrokeWidth, 0xffffffffu, iLineJoin, iLineCap, pStyle->pDashPattern, pStyle->iDashCount, pStyle->fDashOffset, fTolerance, &iVertexCount, &iIndexCount);
	if ( iRet != XGE_OK ) {
		xrtFree(pIndices);
		xrtFree(pVertices);
		return iRet;
	}
	if ( pGradient->iUnits == XGE_SVG_GRADIENT_USER_SPACE ) {
		tA = __xgeSvgTransformPoint(tMap, __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fX1, pGradient->fY1}));
		tB = __xgeSvgTransformPoint(tMap, __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fX2, pGradient->fY2}));
	} else {
		xge_vec2_t tLocalA = __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fX1, pGradient->fY1});
		xge_vec2_t tLocalB = __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fX2, pGradient->fY2});

		tA = (xge_vec2_t){tBounds.fX + tLocalA.fX * tBounds.fW, tBounds.fY + tLocalA.fY * tBounds.fH};
		tB = (xge_vec2_t){tBounds.fX + tLocalB.fX * tBounds.fW, tBounds.fY + tLocalB.fY * tBounds.fH};
	}
	if ( bScreenSpace ) {
		xge_shape_vertex_t* pAAVertices;
		uint32_t* pAAIndices;
		int iAAVertexCount;
		int iAAIndexCount;

		iAAVertexCount = 0;
		iAAIndexCount = 0;
		iRet = __xgePathBuildDashedStrokeAAMeshWithStyle(pPath, NULL, 0, NULL, 0, pStyle->fStrokeWidth, 0xffffffffu, iLineJoin, iLineCap, pStyle->pDashPattern, pStyle->iDashCount, pStyle->fDashOffset, fTolerance, 1.0f, &iAAVertexCount, &iAAIndexCount);
		if ( iRet != XGE_OK ) {
			xrtFree(pIndices);
			xrtFree(pVertices);
			return iRet;
		}
		if ( (iAAVertexCount > 0) && (iAAIndexCount > 0) ) {
			pAAVertices = (xge_shape_vertex_t*)xrtMalloc(sizeof(*pAAVertices) * (size_t)iAAVertexCount);
			pAAIndices = (uint32_t*)xrtMalloc(sizeof(*pAAIndices) * (size_t)iAAIndexCount);
			if ( (pAAVertices == NULL) || (pAAIndices == NULL) ) {
				if ( pAAVertices != NULL ) xrtFree(pAAVertices);
				if ( pAAIndices != NULL ) xrtFree(pAAIndices);
				xrtFree(pIndices);
				xrtFree(pVertices);
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			iRet = __xgePathBuildDashedStrokeAAMeshWithStyle(pPath, pAAVertices, iAAVertexCount, pAAIndices, iAAIndexCount, pStyle->fStrokeWidth, 0xffffffffu, iLineJoin, iLineCap, pStyle->pDashPattern, pStyle->iDashCount, pStyle->fDashOffset, fTolerance, 1.0f, &iAAVertexCount, &iAAIndexCount);
			if ( iRet == XGE_OK ) {
				__xgeSvgApplyLinearGradientColors(pAAVertices, iAAVertexCount, pGradient, tA, tB, fStrokeOpacity);
				iRet = xgeShapeMeshFillPx(pAAVertices, iAAVertexCount, pAAIndices, iAAIndexCount);
			}
			xrtFree(pAAIndices);
			xrtFree(pAAVertices);
			if ( iRet != XGE_OK ) {
				xrtFree(pIndices);
				xrtFree(pVertices);
				return iRet;
			}
		}
	}
	__xgeSvgApplyLinearGradientColors(pVertices, iVertexCount, pGradient, tA, tB, fStrokeOpacity);
	iRet = bScreenSpace ? xgeShapeMeshFillPx(pVertices, iVertexCount, pIndices, iIndexCount) : xgeShapeMeshFill(pVertices, iVertexCount, pIndices, iIndexCount);
	xrtFree(pIndices);
	xrtFree(pVertices);
	return iRet;
}

static int __xgeSvgDrawRadialGradientStroke(xge_path pPath, const xge_svg_radial_gradient_t* pGradient, xge_svg_transform_t tMap, const xge_path_style_t* pStyle, float fTolerance, float fStrokeOpacity, int bScreenSpace)
{
	xge_shape_vertex_t* pVertices;
	uint32_t* pIndices;
	xge_rect_t tBounds;
	xge_vec2_t tFocus;
	float fRadius;
	int iVertexCount;
	int iIndexCount;
	int iLineJoin;
	int iLineCap;
	int iRet;

	if ( !__xgePathValid(pPath) || (pGradient == NULL) || (pStyle == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pGradient->iStopCount <= 0) || (pGradient->fR <= XGE_PATH_EPSILON) || (pStyle->fStrokeWidth <= 0.0f) ) {
		return XGE_OK;
	}
	iRet = __xgePathGetBounds(pPath, fTolerance, &tBounds);
	if ( iRet != XGE_OK ) {
		return (iRet == XGE_ERROR_NOT_FOUND) ? XGE_OK : iRet;
	}
	iLineJoin = (pStyle->iLineJoin == XGE_PATH_JOIN_BEVEL || pStyle->iLineJoin == XGE_PATH_JOIN_ROUND) ? pStyle->iLineJoin : XGE_PATH_JOIN_MITER;
	iLineCap = (pStyle->iLineCap == XGE_PATH_CAP_SQUARE || pStyle->iLineCap == XGE_PATH_CAP_ROUND) ? pStyle->iLineCap : XGE_PATH_CAP_BUTT;
	iVertexCount = 0;
	iIndexCount = 0;
	iRet = __xgePathBuildDashedStrokeMeshWithStyle(pPath, NULL, 0, NULL, 0, pStyle->fStrokeWidth, 0xffffffffu, iLineJoin, iLineCap, pStyle->pDashPattern, pStyle->iDashCount, pStyle->fDashOffset, fTolerance, &iVertexCount, &iIndexCount);
	if ( (iRet != XGE_OK) || (iVertexCount <= 0) || (iIndexCount <= 0) ) {
		return iRet;
	}
	pVertices = (xge_shape_vertex_t*)xrtMalloc(sizeof(*pVertices) * (size_t)iVertexCount);
	pIndices = (uint32_t*)xrtMalloc(sizeof(*pIndices) * (size_t)iIndexCount);
	if ( (pVertices == NULL) || (pIndices == NULL) ) {
		if ( pVertices != NULL ) xrtFree(pVertices);
		if ( pIndices != NULL ) xrtFree(pIndices);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xgePathBuildDashedStrokeMeshWithStyle(pPath, pVertices, iVertexCount, pIndices, iIndexCount, pStyle->fStrokeWidth, 0xffffffffu, iLineJoin, iLineCap, pStyle->pDashPattern, pStyle->iDashCount, pStyle->fDashOffset, fTolerance, &iVertexCount, &iIndexCount);
	if ( iRet != XGE_OK ) {
		xrtFree(pIndices);
		xrtFree(pVertices);
		return iRet;
	}
	if ( pGradient->iUnits == XGE_SVG_GRADIENT_USER_SPACE ) {
		xge_vec2_t tCenter;
		xge_vec2_t tRadiusPoint;

		tCenter = __xgeSvgTransformPoint(tMap, __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fCX, pGradient->fCY}));
		tFocus = __xgeSvgTransformPoint(tMap, __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fFX, pGradient->fFY}));
		tRadiusPoint = __xgeSvgTransformPoint(tMap, __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fCX + pGradient->fR, pGradient->fCY}));
		fRadius = sqrtf((tRadiusPoint.fX - tCenter.fX) * (tRadiusPoint.fX - tCenter.fX) + (tRadiusPoint.fY - tCenter.fY) * (tRadiusPoint.fY - tCenter.fY));
		(void)tCenter;
	} else {
		xge_vec2_t tCenter;
		xge_vec2_t tRadiusPoint;

		tCenter = __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fCX, pGradient->fCY});
		tFocus = __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fFX, pGradient->fFY});
		tRadiusPoint = __xgeSvgTransformPoint(pGradient->tTransform, (xge_vec2_t){pGradient->fCX + pGradient->fR, pGradient->fCY});
		fRadius = sqrtf((tRadiusPoint.fX - tCenter.fX) * (tRadiusPoint.fX - tCenter.fX) + (tRadiusPoint.fY - tCenter.fY) * (tRadiusPoint.fY - tCenter.fY));
		(void)tCenter;
	}
	if ( bScreenSpace ) {
		xge_shape_vertex_t* pAAVertices;
		uint32_t* pAAIndices;
		int iAAVertexCount;
		int iAAIndexCount;

		iAAVertexCount = 0;
		iAAIndexCount = 0;
		iRet = __xgePathBuildDashedStrokeAAMeshWithStyle(pPath, NULL, 0, NULL, 0, pStyle->fStrokeWidth, 0xffffffffu, iLineJoin, iLineCap, pStyle->pDashPattern, pStyle->iDashCount, pStyle->fDashOffset, fTolerance, 1.0f, &iAAVertexCount, &iAAIndexCount);
		if ( iRet != XGE_OK ) {
			xrtFree(pIndices);
			xrtFree(pVertices);
			return iRet;
		}
		if ( (iAAVertexCount > 0) && (iAAIndexCount > 0) ) {
			pAAVertices = (xge_shape_vertex_t*)xrtMalloc(sizeof(*pAAVertices) * (size_t)iAAVertexCount);
			pAAIndices = (uint32_t*)xrtMalloc(sizeof(*pAAIndices) * (size_t)iAAIndexCount);
			if ( (pAAVertices == NULL) || (pAAIndices == NULL) ) {
				if ( pAAVertices != NULL ) xrtFree(pAAVertices);
				if ( pAAIndices != NULL ) xrtFree(pAAIndices);
				xrtFree(pIndices);
				xrtFree(pVertices);
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			iRet = __xgePathBuildDashedStrokeAAMeshWithStyle(pPath, pAAVertices, iAAVertexCount, pAAIndices, iAAIndexCount, pStyle->fStrokeWidth, 0xffffffffu, iLineJoin, iLineCap, pStyle->pDashPattern, pStyle->iDashCount, pStyle->fDashOffset, fTolerance, 1.0f, &iAAVertexCount, &iAAIndexCount);
			if ( iRet == XGE_OK ) {
				__xgeSvgApplyRadialGradientColors(pAAVertices, iAAVertexCount, pGradient, tBounds, tFocus, fRadius, fStrokeOpacity, pGradient->iUnits != XGE_SVG_GRADIENT_USER_SPACE);
				iRet = xgeShapeMeshFillPx(pAAVertices, iAAVertexCount, pAAIndices, iAAIndexCount);
			}
			xrtFree(pAAIndices);
			xrtFree(pAAVertices);
			if ( iRet != XGE_OK ) {
				xrtFree(pIndices);
				xrtFree(pVertices);
				return iRet;
			}
		}
	}
	__xgeSvgApplyRadialGradientColors(pVertices, iVertexCount, pGradient, tBounds, tFocus, fRadius, fStrokeOpacity, pGradient->iUnits != XGE_SVG_GRADIENT_USER_SPACE);
	iRet = bScreenSpace ? xgeShapeMeshFillPx(pVertices, iVertexCount, pIndices, iIndexCount) : xgeShapeMeshFill(pVertices, iVertexCount, pIndices, iIndexCount);
	xrtFree(pIndices);
	xrtFree(pVertices);
	return iRet;
}

static int __xgeSvgGetClipRectCount(xge_svg pSvg, const char* sClipPathId)
{
	xge_svg_clip_path_t* pClipPath;

	if ( !__xgeSvgValid(pSvg) || (sClipPathId == NULL) || (sClipPathId[0] == '\0') ) {
		return 0;
	}
	pClipPath = __xgeSvgFindClipPath(pSvg, sClipPathId);
	if ( pClipPath == NULL ) {
		return 0;
	}
	if ( pClipPath->iRectCount > 0 ) {
		return pClipPath->iRectCount;
	}
	return pClipPath->bHasRect ? 1 : 0;
}

static int __xgeSvgResolveClipRectAt(xge_svg pSvg, const char* sClipPathId, int iRectIndex, xge_svg_transform_t tMap, xge_path pDrawPath, float fTolerance, xge_rect_t* pClipRect)
{
	xge_svg_clip_path_t* pClipPath;
	xge_rect_t tBounds;
	xge_rect_t tRect;
	xge_rect_t tSourceRect;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (sClipPathId == NULL) || (sClipPathId[0] == '\0') || (iRectIndex < 0) || (pClipRect == NULL) ) {
		return 0;
	}
	pClipPath = __xgeSvgFindClipPath(pSvg, sClipPathId);
	if ( (pClipPath == NULL) || !pClipPath->bHasRect ) {
		return 0;
	}
	if ( pClipPath->iRectCount > 0 ) {
		if ( iRectIndex >= pClipPath->iRectCount ) {
			return 0;
		}
		tSourceRect = pClipPath->pRects[iRectIndex];
	} else if ( iRectIndex == 0 ) {
		tSourceRect = pClipPath->tRect;
	} else {
		return 0;
	}
	if ( pClipPath->iUnits == XGE_SVG_CLIP_OBJECT_BOUNDING_BOX ) {
		if ( !__xgePathValid(pDrawPath) ) {
			return 0;
		}
		iRet = __xgePathGetBounds(pDrawPath, fTolerance, &tBounds);
		if ( iRet != XGE_OK ) {
			return 0;
		}
		tRect = (xge_rect_t){
			tBounds.fX + (tSourceRect.fX * tBounds.fW),
			tBounds.fY + (tSourceRect.fY * tBounds.fH),
			tSourceRect.fW * tBounds.fW,
			tSourceRect.fH * tBounds.fH
		};
	} else {
		tRect = __xgeSvgTransformRectBounds(tMap, tSourceRect);
	}
	if ( (tRect.fW <= XGE_PATH_EPSILON) || (tRect.fH <= XGE_PATH_EPSILON) ) {
		return 0;
	}
	*pClipRect = tRect;
	return 1;
}

static int __xgeSvgGetMaskRectCount(xge_svg pSvg, const char* sMaskId)
{
	xge_svg_mask_t* pMask;

	if ( !__xgeSvgValid(pSvg) || (sMaskId == NULL) || (sMaskId[0] == '\0') ) {
		return 0;
	}
	pMask = __xgeSvgFindMask(pSvg, sMaskId);
	if ( pMask == NULL ) {
		return 0;
	}
	if ( pMask->iRectCount > 0 ) {
		return pMask->iRectCount;
	}
	return pMask->bHasRect ? 1 : 0;
}

static int __xgeSvgResolveMaskRectAt(xge_svg pSvg, const char* sMaskId, int iRectIndex, xge_svg_transform_t tMap, xge_path pDrawPath, float fTolerance, xge_rect_t* pMaskRect, float* pMaskOpacity)
{
	xge_svg_mask_t* pMask;
	xge_rect_t tBounds;
	xge_rect_t tRect;
	xge_rect_t tSourceRect;
	float fSourceOpacity;
	int iRet;

	if ( pMaskOpacity != NULL ) {
		*pMaskOpacity = 1.0f;
	}
	if ( !__xgeSvgValid(pSvg) || (sMaskId == NULL) || (sMaskId[0] == '\0') || (iRectIndex < 0) || (pMaskRect == NULL) || (pMaskOpacity == NULL) ) {
		return 0;
	}
	pMask = __xgeSvgFindMask(pSvg, sMaskId);
	if ( (pMask == NULL) || !pMask->bHasRect ) {
		return 0;
	}
	if ( pMask->iRectCount > 0 ) {
		if ( iRectIndex >= pMask->iRectCount ) {
			return 0;
		}
		tSourceRect = pMask->pRects[iRectIndex].tRect;
		fSourceOpacity = pMask->pRects[iRectIndex].fOpacity;
	} else if ( iRectIndex == 0 ) {
		tSourceRect = pMask->tRect;
		fSourceOpacity = pMask->fOpacity;
	} else {
		return 0;
	}
	if ( fSourceOpacity <= 0.0f ) {
		*pMaskOpacity = 0.0f;
		return 1;
	}
	if ( pMask->iUnits == XGE_SVG_MASK_OBJECT_BOUNDING_BOX ) {
		if ( !__xgePathValid(pDrawPath) ) {
			return 0;
		}
		iRet = __xgePathGetBounds(pDrawPath, fTolerance, &tBounds);
		if ( iRet != XGE_OK ) {
			return 0;
		}
		tRect = (xge_rect_t){
			tBounds.fX + (tSourceRect.fX * tBounds.fW),
			tBounds.fY + (tSourceRect.fY * tBounds.fH),
			tSourceRect.fW * tBounds.fW,
			tSourceRect.fH * tBounds.fH
		};
	} else {
		tRect = __xgeSvgTransformRectBounds(tMap, tSourceRect);
	}
	if ( (tRect.fW <= XGE_PATH_EPSILON) || (tRect.fH <= XGE_PATH_EPSILON) ) {
		*pMaskOpacity = 0.0f;
		return 1;
	}
	*pMaskRect = tRect;
	*pMaskOpacity = __xgeSvgClamp01(fSourceOpacity);
	return 1;
}

static int __xgeSvgDrawPathItemPayload(xge_svg pSvg, int iPathIndex, xge_path pDrawPath, xge_svg_transform_t tMap, float fScaleStyle, float fTolerance, int bScreenSpace, float fMaskOpacity)
{
	xge_path_style_t tStyle;
	float arrDashInline[XGE_SVG_DASH_INLINE_MAX];
	float* pDashHeap;
	int j;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iPathIndex < 0) || (iPathIndex >= pSvg->iPathCount) || !__xgePathValid(pDrawPath) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tStyle = pSvg->pPaths[iPathIndex].tStyle;
	tStyle.fStrokeWidth *= fScaleStyle;
	pDashHeap = NULL;
	if ( fMaskOpacity < 1.0f ) {
		tStyle.iFillColor = __xgeSvgColorApplyOpacity(tStyle.iFillColor, fMaskOpacity);
		tStyle.iStrokeColor = __xgeSvgColorApplyOpacity(tStyle.iStrokeColor, fMaskOpacity);
	}
	if ( pSvg->pPaths[iPathIndex].sFillGradientId[0] != '\0' ) {
		xge_svg_linear_gradient_t* pGradient;

		pGradient = __xgeSvgFindLinearGradient(pSvg, pSvg->pPaths[iPathIndex].sFillGradientId);
		if ( pGradient != NULL ) {
			iRet = __xgeSvgDrawLinearGradientFill(pDrawPath, pGradient, tMap, tStyle.iFillRule, fTolerance, pSvg->pPaths[iPathIndex].fFillOpacity * fMaskOpacity, bScreenSpace);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			tStyle.iFillColor = 0x00000000u;
		} else {
			xge_svg_radial_gradient_t* pRadialGradient;

			pRadialGradient = __xgeSvgFindRadialGradient(pSvg, pSvg->pPaths[iPathIndex].sFillGradientId);
			if ( pRadialGradient != NULL ) {
				iRet = __xgeSvgDrawRadialGradientFill(pDrawPath, pRadialGradient, tMap, tStyle.iFillRule, fTolerance, pSvg->pPaths[iPathIndex].fFillOpacity * fMaskOpacity, bScreenSpace);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
				tStyle.iFillColor = 0x00000000u;
			}
		}
	}
	if ( (tStyle.pDashPattern != NULL) && (tStyle.iDashCount > 0) ) {
		float* pDashOut;

		if ( tStyle.iDashCount <= XGE_SVG_DASH_INLINE_MAX ) {
			pDashOut = arrDashInline;
		} else {
			pDashHeap = (float*)xrtMalloc(sizeof(float) * (size_t)tStyle.iDashCount);
			if ( pDashHeap == NULL ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			pDashOut = pDashHeap;
		}
		for ( j = 0; j < tStyle.iDashCount; j++ ) {
			pDashOut[j] = tStyle.pDashPattern[j] * fScaleStyle;
		}
		tStyle.pDashPattern = pDashOut;
	}
	if ( pSvg->pPaths[iPathIndex].sStrokeGradientId[0] != '\0' ) {
		xge_svg_linear_gradient_t* pStrokeLinearGradient;

		pStrokeLinearGradient = __xgeSvgFindLinearGradient(pSvg, pSvg->pPaths[iPathIndex].sStrokeGradientId);
		if ( pStrokeLinearGradient != NULL ) {
			iRet = __xgeSvgDrawLinearGradientStroke(pDrawPath, pStrokeLinearGradient, tMap, &tStyle, fTolerance, pSvg->pPaths[iPathIndex].fStrokeOpacity * fMaskOpacity, bScreenSpace);
			if ( iRet != XGE_OK ) {
				if ( pDashHeap != NULL ) xrtFree(pDashHeap);
				return iRet;
			}
			tStyle.iStrokeColor = 0x00000000u;
		} else {
			xge_svg_radial_gradient_t* pStrokeRadialGradient;

			pStrokeRadialGradient = __xgeSvgFindRadialGradient(pSvg, pSvg->pPaths[iPathIndex].sStrokeGradientId);
			if ( pStrokeRadialGradient != NULL ) {
				iRet = __xgeSvgDrawRadialGradientStroke(pDrawPath, pStrokeRadialGradient, tMap, &tStyle, fTolerance, pSvg->pPaths[iPathIndex].fStrokeOpacity * fMaskOpacity, bScreenSpace);
				if ( iRet != XGE_OK ) {
					if ( pDashHeap != NULL ) xrtFree(pDashHeap);
					return iRet;
				}
				tStyle.iStrokeColor = 0x00000000u;
			}
		}
	}
	iRet = bScreenSpace ? xgePathDrawPx(pDrawPath, &tStyle, fTolerance) : xgePathDraw(pDrawPath, &tStyle, fTolerance);
	if ( pDashHeap != NULL ) {
		xrtFree(pDashHeap);
	}
	return iRet;
}

static int __xgeSvgCompactRegionEdges(float* pEdges, int iCount)
{
	int i;
	int iWrite;

	if ( (pEdges == NULL) || (iCount <= 0) ) {
		return 0;
	}
	qsort(pEdges, (size_t)iCount, sizeof(*pEdges), __xgeFloatCompare);
	iWrite = 1;
	for ( i = 1; i < iCount; i++ ) {
		if ( fabsf(pEdges[i] - pEdges[iWrite - 1]) > XGE_PATH_EPSILON ) {
			pEdges[iWrite++] = pEdges[i];
		}
	}
	return iWrite;
}

static int __xgeSvgCellCoveredByRect(xge_rect_t tRect, float fX0, float fY0, float fX1, float fY1)
{
	float fCX;
	float fCY;

	if ( (fX1 - fX0 <= XGE_PATH_EPSILON) || (fY1 - fY0 <= XGE_PATH_EPSILON) ) {
		return 0;
	}
	fCX = (fX0 + fX1) * 0.5f;
	fCY = (fY0 + fY1) * 0.5f;
	return (fCX >= tRect.fX - XGE_PATH_EPSILON) &&
	       (fCY >= tRect.fY - XGE_PATH_EPSILON) &&
	       (fCX <= tRect.fX + tRect.fW + XGE_PATH_EPSILON) &&
	       (fCY <= tRect.fY + tRect.fH + XGE_PATH_EPSILON);
}

static int __xgeSvgCellCoveredByAnyClip(const xge_rect_t* pClipRects, int iClipRectCount, float fX0, float fY0, float fX1, float fY1)
{
	int i;

	if ( iClipRectCount <= 0 ) {
		return 1;
	}
	if ( pClipRects == NULL ) {
		return 0;
	}
	for ( i = 0; i < iClipRectCount; i++ ) {
		if ( __xgeSvgCellCoveredByRect(pClipRects[i], fX0, fY0, fX1, fY1) ) {
			return 1;
		}
	}
	return 0;
}

static float __xgeSvgCellMaskOpacity(const xge_svg_mask_rect_t* pMaskRects, int iMaskRectCount, float fX0, float fY0, float fX1, float fY1)
{
	float fOpacity;
	float fSource;
	int i;

	if ( iMaskRectCount <= 0 ) {
		return 1.0f;
	}
	if ( pMaskRects == NULL ) {
		return 0.0f;
	}
	fOpacity = 0.0f;
	for ( i = 0; i < iMaskRectCount; i++ ) {
		if ( __xgeSvgCellCoveredByRect(pMaskRects[i].tRect, fX0, fY0, fX1, fY1) ) {
			fSource = __xgeSvgClamp01(pMaskRects[i].fOpacity);
			fOpacity = fSource + (fOpacity * (1.0f - fSource));
		}
	}
	return __xgeSvgClamp01(fOpacity);
}

static int __xgeSvgDrawPathItemClippedPayload(xge_svg pSvg, int iPathIndex, xge_path pDrawPath, xge_svg_transform_t tMap, float fScaleStyle, float fTolerance, int bScreenSpace, xge_rect_t tClip, float fMaskOpacity)
{
	xge_rect_t tOldClip;
	xge_rect_t tDrawClip;
	int bOldClip;
	int bPathClip;
	int iRet;

	memset(&tOldClip, 0, sizeof(tOldClip));
	tDrawClip = tClip;
	tOldClip = xgeClipGet();
	bOldClip = (tOldClip.fW > 0.0f) && (tOldClip.fH > 0.0f);
	if ( bOldClip && !__xgeSvgRectIntersect(&tDrawClip, tOldClip) ) {
		return XGE_OK;
	}
	(void)xgeFlush();
	xgeClipSet(tDrawClip);
	bPathClip = 1;
	iRet = __xgeSvgDrawPathItemPayload(pSvg, iPathIndex, pDrawPath, tMap, fScaleStyle, fTolerance, bScreenSpace, fMaskOpacity);
	__xgeSvgRestoreClip(bPathClip, bOldClip, tOldClip);
	return iRet;
}

static int __xgeSvgDrawPathItemWithRectRegions(xge_svg pSvg, int iPathIndex, xge_path pDrawPath, xge_svg_transform_t tMap, float fScaleStyle, float fTolerance, int bScreenSpace)
{
	xge_rect_t* pClipRects;
	xge_svg_mask_rect_t* pMaskRects;
	float* pXEdges;
	float* pYEdges;
	int iClipSourceCount;
	int iMaskSourceCount;
	int iClipRectCount;
	int iMaskRectCount;
	int iEdgeCapacity;
	int iXEdgeCount;
	int iYEdgeCount;
	int i;
	int j;
	int iRet;

	if ( !bScreenSpace ) {
		return __xgeSvgDrawPathItemPayload(pSvg, iPathIndex, pDrawPath, tMap, fScaleStyle, fTolerance, bScreenSpace, 1.0f);
	}
	iClipSourceCount = __xgeSvgGetClipRectCount(pSvg, pSvg->pPaths[iPathIndex].sClipPathId);
	iMaskSourceCount = __xgeSvgGetMaskRectCount(pSvg, pSvg->pPaths[iPathIndex].sMaskId);
	if ( (iClipSourceCount <= 0) && (iMaskSourceCount <= 0) ) {
		return __xgeSvgDrawPathItemPayload(pSvg, iPathIndex, pDrawPath, tMap, fScaleStyle, fTolerance, bScreenSpace, 1.0f);
	}

	pClipRects = NULL;
	pMaskRects = NULL;
	pXEdges = NULL;
	pYEdges = NULL;
	iClipRectCount = 0;
	iMaskRectCount = 0;
	iRet = XGE_OK;
	if ( iClipSourceCount > 0 ) {
		pClipRects = (xge_rect_t*)xrtMalloc(sizeof(*pClipRects) * (size_t)iClipSourceCount);
		if ( pClipRects == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		for ( i = 0; i < iClipSourceCount; i++ ) {
			xge_rect_t tRect;

			if ( __xgeSvgResolveClipRectAt(pSvg, pSvg->pPaths[iPathIndex].sClipPathId, i, tMap, pDrawPath, fTolerance, &tRect) ) {
				pClipRects[iClipRectCount++] = tRect;
			}
		}
		if ( iClipRectCount <= 0 ) {
			xrtFree(pClipRects);
			return XGE_OK;
		}
	}
	if ( iMaskSourceCount > 0 ) {
		pMaskRects = (xge_svg_mask_rect_t*)xrtMalloc(sizeof(*pMaskRects) * (size_t)iMaskSourceCount);
		if ( pMaskRects == NULL ) {
			if ( pClipRects != NULL ) xrtFree(pClipRects);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		for ( i = 0; i < iMaskSourceCount; i++ ) {
			xge_rect_t tRect;
			float fOpacity;

			if ( __xgeSvgResolveMaskRectAt(pSvg, pSvg->pPaths[iPathIndex].sMaskId, i, tMap, pDrawPath, fTolerance, &tRect, &fOpacity) &&
			     (fOpacity > 0.0f) ) {
				pMaskRects[iMaskRectCount++] = (xge_svg_mask_rect_t){tRect, fOpacity};
			}
		}
		if ( iMaskRectCount <= 0 ) {
			if ( pClipRects != NULL ) xrtFree(pClipRects);
			xrtFree(pMaskRects);
			return XGE_OK;
		}
	}

	iEdgeCapacity = (iClipRectCount + iMaskRectCount) * 2;
	if ( iEdgeCapacity <= 0 ) {
		if ( pClipRects != NULL ) xrtFree(pClipRects);
		if ( pMaskRects != NULL ) xrtFree(pMaskRects);
		return XGE_OK;
	}
	pXEdges = (float*)xrtMalloc(sizeof(*pXEdges) * (size_t)iEdgeCapacity);
	pYEdges = (float*)xrtMalloc(sizeof(*pYEdges) * (size_t)iEdgeCapacity);
	if ( (pXEdges == NULL) || (pYEdges == NULL) ) {
		iRet = XGE_ERROR_OUT_OF_MEMORY;
		goto xge_svg_region_done;
	}
	iXEdgeCount = 0;
	iYEdgeCount = 0;
	for ( i = 0; i < iClipRectCount; i++ ) {
		pXEdges[iXEdgeCount++] = pClipRects[i].fX;
		pXEdges[iXEdgeCount++] = pClipRects[i].fX + pClipRects[i].fW;
		pYEdges[iYEdgeCount++] = pClipRects[i].fY;
		pYEdges[iYEdgeCount++] = pClipRects[i].fY + pClipRects[i].fH;
	}
	for ( i = 0; i < iMaskRectCount; i++ ) {
		pXEdges[iXEdgeCount++] = pMaskRects[i].tRect.fX;
		pXEdges[iXEdgeCount++] = pMaskRects[i].tRect.fX + pMaskRects[i].tRect.fW;
		pYEdges[iYEdgeCount++] = pMaskRects[i].tRect.fY;
		pYEdges[iYEdgeCount++] = pMaskRects[i].tRect.fY + pMaskRects[i].tRect.fH;
	}
	iXEdgeCount = __xgeSvgCompactRegionEdges(pXEdges, iXEdgeCount);
	iYEdgeCount = __xgeSvgCompactRegionEdges(pYEdges, iYEdgeCount);
	for ( j = 0; (j + 1 < iYEdgeCount) && (iRet == XGE_OK); j++ ) {
		float fY0;
		float fY1;

		fY0 = pYEdges[j];
		fY1 = pYEdges[j + 1];
		if ( fY1 - fY0 <= XGE_PATH_EPSILON ) {
			continue;
		}
		for ( i = 0; i + 1 < iXEdgeCount; i++ ) {
			float fX0;
			float fX1;
			float fMaskOpacity;
			xge_rect_t tCell;

			fX0 = pXEdges[i];
			fX1 = pXEdges[i + 1];
			if ( fX1 - fX0 <= XGE_PATH_EPSILON ) {
				continue;
			}
			if ( !__xgeSvgCellCoveredByAnyClip(pClipRects, iClipRectCount, fX0, fY0, fX1, fY1) ) {
				continue;
			}
			fMaskOpacity = __xgeSvgCellMaskOpacity(pMaskRects, iMaskRectCount, fX0, fY0, fX1, fY1);
			if ( fMaskOpacity <= 0.0f ) {
				continue;
			}
			tCell = (xge_rect_t){fX0, fY0, fX1 - fX0, fY1 - fY0};
			iRet = __xgeSvgDrawPathItemClippedPayload(pSvg, iPathIndex, pDrawPath, tMap, fScaleStyle, fTolerance, bScreenSpace, tCell, fMaskOpacity);
			if ( iRet != XGE_OK ) {
				break;
			}
		}
	}

xge_svg_region_done:
	if ( pYEdges != NULL ) xrtFree(pYEdges);
	if ( pXEdges != NULL ) xrtFree(pXEdges);
	if ( pMaskRects != NULL ) xrtFree(pMaskRects);
	if ( pClipRects != NULL ) xrtFree(pClipRects);
	return iRet;
}

static int __xgeSvgDrawInternal(xge_svg pSvg, xge_rect_t tDst, float fTolerance, int bScreenSpace)
{
	xge_svg_transform_t tMap;
	xge_path pDrawPath;
	xge_rect_t tViewport;
	xge_rect_t tOldClip;
	xge_rect_t tClip;
	float fScaleX;
	float fScaleY;
	float fScaleStyle;
	int i;
	int iRet;
	int iVisiblePathCount;
	int bClip;
	int bOldClip;

	if ( !__xgeSvgValid(pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !pSvg->bHasViewBox || (pSvg->tViewBox.fW <= 0.0f) || (pSvg->tViewBox.fH <= 0.0f) || (tDst.fW == 0.0f) || (tDst.fH == 0.0f) ) {
		return XGE_OK;
	}
	iRet = xgeSvgGetDrawViewport(pSvg, tDst, &tViewport);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	memset(&tOldClip, 0, sizeof(tOldClip));
	memset(&tClip, 0, sizeof(tClip));
	bClip = 0;
	bOldClip = 0;
	if ( bScreenSpace && (pSvg->iAspectMode == XGE_SVG_ASPECT_SLICE) &&
	     ((tViewport.fX < tDst.fX - XGE_PATH_EPSILON) ||
	      (tViewport.fY < tDst.fY - XGE_PATH_EPSILON) ||
	      (tViewport.fX + tViewport.fW > tDst.fX + tDst.fW + XGE_PATH_EPSILON) ||
	      (tViewport.fY + tViewport.fH > tDst.fY + tDst.fH + XGE_PATH_EPSILON)) ) {
		float fLeft;
		float fTop;
		float fRight;
		float fBottom;

		tOldClip = xgeClipGet();
		bOldClip = (tOldClip.fW > 0.0f) && (tOldClip.fH > 0.0f);
		tClip = tDst;
		if ( bOldClip ) {
			fLeft = (tClip.fX > tOldClip.fX) ? tClip.fX : tOldClip.fX;
			fTop = (tClip.fY > tOldClip.fY) ? tClip.fY : tOldClip.fY;
			fRight = ((tClip.fX + tClip.fW) < (tOldClip.fX + tOldClip.fW)) ? (tClip.fX + tClip.fW) : (tOldClip.fX + tOldClip.fW);
			fBottom = ((tClip.fY + tClip.fH) < (tOldClip.fY + tOldClip.fH)) ? (tClip.fY + tClip.fH) : (tOldClip.fY + tOldClip.fH);
			tClip.fX = fLeft;
			tClip.fY = fTop;
			tClip.fW = fRight - fLeft;
			tClip.fH = fBottom - fTop;
		}
		if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
			return XGE_OK;
		}
		(void)xgeFlush();
		xgeClipSet(tClip);
		bClip = 1;
	}
	fScaleX = tViewport.fW / pSvg->tViewBox.fW;
	fScaleY = tViewport.fH / pSvg->tViewBox.fH;
	fScaleStyle = ((fScaleX < 0.0f ? -fScaleX : fScaleX) + (fScaleY < 0.0f ? -fScaleY : fScaleY)) * 0.5f;
	tMap = __xgeSvgTransformIdentity();
	tMap.fA = fScaleX;
	tMap.fD = fScaleY;
	tMap.fE = tViewport.fX - (pSvg->tViewBox.fX * fScaleX);
	tMap.fF = tViewport.fY - (pSvg->tViewBox.fY * fScaleY);
	iRet = xgePathCreate(&pDrawPath);
	if ( iRet != XGE_OK ) {
		__xgeSvgRestoreClip(bClip, bOldClip, tOldClip);
		return iRet;
	}
	iVisiblePathCount = xgeSvgGetPathCount(pSvg);
	for ( i = 0; i < iVisiblePathCount; i++ ) {
		int iPathIndex;

		iPathIndex = __xgeSvgGetVisiblePathStorageIndex(pSvg, i);
		if ( iPathIndex < 0 ) {
			continue;
		}
		iRet = __xgePathCopyTransformed(pDrawPath, pSvg->pPaths[iPathIndex].pPath, tMap);
		if ( iRet != XGE_OK ) {
			break;
		}
		iRet = __xgeSvgDrawPathItemWithRectRegions(pSvg, iPathIndex, pDrawPath, tMap, fScaleStyle, fTolerance, bScreenSpace);
		if ( iRet != XGE_OK ) {
			break;
		}
	}
	xgePathDestroy(pDrawPath);
	__xgeSvgRestoreClip(bClip, bOldClip, tOldClip);
	return iRet;
}

int xgeSvgDraw(xge_svg pSvg, xge_rect_t tDst, float fTolerance)
{
	return __xgeSvgDrawInternal(pSvg, tDst, fTolerance, 0);
}

int xgeSvgDrawPx(xge_svg pSvg, xge_rect_t tDst, float fTolerance)
{
	return __xgeSvgDrawInternal(pSvg, tDst, fTolerance, 1);
}

static int __xgeSvgRasterRoundSize(float fValue)
{
	int iValue;

	if ( (fValue <= 0.0f) || (fValue > (float)(INT_MAX - 1)) ) {
		return 0;
	}
	iValue = (int)(fValue + 0.5f);
	return (iValue > 0) ? iValue : 1;
}

static int __xgeSvgRasterResolveSize(const NSVGimage* pImage, int iReqWidth, int iReqHeight, int* pWidth, int* pHeight)
{
	float fWidth;
	float fHeight;
	int iWidth;
	int iHeight;

	if ( (pImage == NULL) || (pWidth == NULL) || (pHeight == NULL) ||
	     (pImage->width <= 0.0f) || (pImage->height <= 0.0f) ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( (iReqWidth < 0) || (iReqHeight < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fWidth = pImage->width;
	fHeight = pImage->height;
	if ( (iReqWidth == 0) && (iReqHeight == 0) ) {
		iWidth = __xgeSvgRasterRoundSize(fWidth);
		iHeight = __xgeSvgRasterRoundSize(fHeight);
	} else if ( iReqWidth == 0 ) {
		iHeight = iReqHeight;
		iWidth = __xgeSvgRasterRoundSize(((float)iReqHeight * fWidth) / fHeight);
	} else if ( iReqHeight == 0 ) {
		iWidth = iReqWidth;
		iHeight = __xgeSvgRasterRoundSize(((float)iReqWidth * fHeight) / fWidth);
	} else {
		iWidth = iReqWidth;
		iHeight = iReqHeight;
	}
	if ( (iWidth <= 0) || (iHeight <= 0) || ((uint64_t)iWidth * (uint64_t)iHeight > (uint64_t)INT_MAX) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pWidth = iWidth;
	*pHeight = iHeight;
	return XGE_OK;
}

static void __xgeSvgPremultiplyRGBA(unsigned char* pPixels, int iWidth, int iHeight, int iStride)
{
	unsigned char* pRow;
	unsigned int iA;
	int iX;
	int iY;

	if ( (pPixels == NULL) || (iWidth <= 0) || (iHeight <= 0) || (iStride < (iWidth * 4)) ) {
		return;
	}
	for ( iY = 0; iY < iHeight; iY++ ) {
		pRow = pPixels + ((size_t)iY * (size_t)iStride);
		for ( iX = 0; iX < iWidth; iX++ ) {
			iA = (unsigned int)pRow[(iX * 4) + 3];
			pRow[(iX * 4) + 0] = (unsigned char)(((unsigned int)pRow[(iX * 4) + 0] * iA + 127u) / 255u);
			pRow[(iX * 4) + 1] = (unsigned char)(((unsigned int)pRow[(iX * 4) + 1] * iA + 127u) / 255u);
			pRow[(iX * 4) + 2] = (unsigned char)(((unsigned int)pRow[(iX * 4) + 2] * iA + 127u) / 255u);
		}
	}
}

static int __xgeSvgRasterizeMemoryInternal(const void* pData, int iSize, int iReqWidth, int iReqHeight, unsigned char* pDstPixels, int iDstStride, unsigned char** ppOwnedPixels, int* pOutWidth, int* pOutHeight)
{
	char* sText;
	NSVGimage* pImage;
	NSVGrasterizer* pRasterizer;
	unsigned char* pPixels;
	float fScaleX;
	float fScaleY;
	float fScale;
	float fTX;
	float fTY;
	int iWidth;
	int iHeight;
	int iStride;
	int iRet;
	int bOwned;

	if ( (pData == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pDstPixels == NULL) && (ppOwnedPixels == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( ppOwnedPixels != NULL ) {
		*ppOwnedPixels = NULL;
	}
	if ( pOutWidth != NULL ) {
		*pOutWidth = 0;
	}
	if ( pOutHeight != NULL ) {
		*pOutHeight = 0;
	}
	if ( iSize >= INT_MAX ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	sText = (char*)xrtMalloc((size_t)iSize + 1u);
	if ( sText == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(sText, pData, (size_t)iSize);
	sText[iSize] = '\0';
	pImage = nsvgParse(sText, "px", 96.0f);
	xrtFree(sText);
	if ( pImage == NULL ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	iRet = __xgeSvgRasterResolveSize(pImage, iReqWidth, iReqHeight, &iWidth, &iHeight);
	if ( iRet != XGE_OK ) {
		nsvgDelete(pImage);
		return iRet;
	}
	iStride = (iDstStride > 0) ? iDstStride : (iWidth * 4);
	if ( iStride < (iWidth * 4) ) {
		nsvgDelete(pImage);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pPixels = pDstPixels;
	bOwned = 0;
	if ( pPixels == NULL ) {
		if ( (uint64_t)iStride * (uint64_t)iHeight > (uint64_t)INT_MAX ) {
			nsvgDelete(pImage);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pPixels = (unsigned char*)xrtMalloc((size_t)iStride * (size_t)iHeight);
		if ( pPixels == NULL ) {
			nsvgDelete(pImage);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		bOwned = 1;
	}
	pRasterizer = nsvgCreateRasterizer();
	if ( pRasterizer == NULL ) {
		if ( bOwned ) xrtFree(pPixels);
		nsvgDelete(pImage);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	fScaleX = (float)iWidth / pImage->width;
	fScaleY = (float)iHeight / pImage->height;
	fScale = (fScaleX < fScaleY) ? fScaleX : fScaleY;
	fTX = ((float)iWidth - (pImage->width * fScale)) * 0.5f;
	fTY = ((float)iHeight - (pImage->height * fScale)) * 0.5f;
	nsvgRasterize(pRasterizer, pImage, fTX, fTY, fScale, pPixels, iWidth, iHeight, iStride);
	__xgeSvgPremultiplyRGBA(pPixels, iWidth, iHeight, iStride);
	nsvgDeleteRasterizer(pRasterizer);
	nsvgDelete(pImage);
	if ( bOwned && (ppOwnedPixels != NULL) ) {
		*ppOwnedPixels = pPixels;
	}
	if ( pOutWidth != NULL ) {
		*pOutWidth = iWidth;
	}
	if ( pOutHeight != NULL ) {
		*pOutHeight = iHeight;
	}
	return XGE_OK;
}

int xgeSvgRasterizeMemory(const void* pData, int iSize, int iWidth, int iHeight, void* pPixels, int iStride)
{
	return __xgeSvgRasterizeMemoryInternal(pData, iSize, iWidth, iHeight, (unsigned char*)pPixels, iStride, NULL, NULL, NULL);
}

int xgeSvgTextureLoadMemory(xge_texture pTexture, const void* pData, int iSize, int iWidth, int iHeight)
{
	unsigned char* pPixels;
	int iOutWidth;
	int iOutHeight;
	int iRet;

	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pPixels = NULL;
	iOutWidth = 0;
	iOutHeight = 0;
	iRet = __xgeSvgRasterizeMemoryInternal(pData, iSize, iWidth, iHeight, NULL, 0, &pPixels, &iOutWidth, &iOutHeight);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeTextureCreateRGBA(pTexture, iOutWidth, iOutHeight, pPixels);
	xrtFree(pPixels);
	return iRet;
}

int xgeSvgTextureLoad(xge_texture pTexture, const char* sURI, int iWidth, int iHeight)
{
	xge_resource_t tResource;
	int iRet;

	if ( (pTexture == NULL) || (sURI == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = xgeResourceLoad(sURI, &tResource);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeSvgTextureLoadMemory(pTexture, tResource.pData, tResource.iSize, iWidth, iHeight);
	xgeResourceFree(&tResource);
	return iRet;
}
