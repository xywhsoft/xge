#define XGE_SVG_MAGIC 0x58475356u
#define XGE_SVG_ATTR_MAX 4096
#define XGE_SVG_STACK_MAX 128
#define XGE_SVG_DASH_MAX 16
#define XGE_SVG_ID_MAX 128
#define XGE_SVG_STYLE_SELECTOR_TAG 0
#define XGE_SVG_STYLE_SELECTOR_CLASS 1
#define XGE_SVG_STYLE_SELECTOR_ID 2
#define XGE_SVG_STYLE_SELECTOR_SIMPLE 3
#define XGE_SVG_CLIP_USER_SPACE 0
#define XGE_SVG_CLIP_OBJECT_BOUNDING_BOX 1
#define XGE_SVG_MASK_USER_SPACE 0
#define XGE_SVG_MASK_OBJECT_BOUNDING_BOX 1
#define XGE_SVG_LENGTH_BASIS_X 0
#define XGE_SVG_LENGTH_BASIS_Y 1
#define XGE_SVG_LENGTH_BASIS_OTHER 2
#define XGE_SVG_GRADIENT_HAS_X1			0x00000001
#define XGE_SVG_GRADIENT_HAS_Y1			0x00000002
#define XGE_SVG_GRADIENT_HAS_X2			0x00000004
#define XGE_SVG_GRADIENT_HAS_Y2			0x00000008
#define XGE_SVG_GRADIENT_HAS_CX			0x00000010
#define XGE_SVG_GRADIENT_HAS_CY			0x00000020
#define XGE_SVG_GRADIENT_HAS_R			0x00000040
#define XGE_SVG_GRADIENT_HAS_FX			0x00000080
#define XGE_SVG_GRADIENT_HAS_FY			0x00000100
#define XGE_SVG_GRADIENT_HAS_UNITS		0x00000200
#define XGE_SVG_GRADIENT_HAS_SPREAD		0x00000400
#define XGE_SVG_GRADIENT_HAS_TRANSFORM	0x00000800
#define XGE_SVG_GRADIENT_HAS_STOPS		0x00001000
#define XGE_SVG_GRADIENT_RESOLVED		0x00002000

typedef struct xge_svg_style_t {
	uint32_t iCurrentColor;
	uint32_t iFillColor;
	char sFillGradientId[XGE_SVG_ID_MAX];
	uint32_t iStrokeColor;
	char sStrokeGradientId[XGE_SVG_ID_MAX];
	uint32_t iStopColor;
	float fOpacity;
	float fFillOpacity;
	float fStrokeOpacity;
	float fStopOpacity;
	float fStrokeWidth;
	float fMiterLimit;
	float fDashPattern[XGE_SVG_DASH_MAX];
	float fDashOffset;
	char sClipId[XGE_SVG_ID_MAX];
	char sMaskId[XGE_SVG_ID_MAX];
	int iDashCount;
	int iFillRule;
	int iLineCap;
	int iLineJoin;
	int bStrokeFirst;
	int bVisible;
	int bVisibility;
	int bStopColorSet;
	int bStopOpacitySet;
	xge_shape_ex_matrix_t tTransform;
} xge_svg_style_t;

typedef struct xge_svg_cache_entry_t {
	char* sURI;
	xge_svg pSvg;
	struct xge_svg_cache_entry_t* pNext;
} xge_svg_cache_entry_t;

typedef struct xge_svg_def_t {
	char* sId;
	xge_shape_ex* pShapes;
	xge_rect_t tViewBox;
	int iShapeCount;
	int iShapeCapacity;
	int bHasViewBox;
	int iAspectAlignX;
	int iAspectAlignY;
	int iAspectMeetOrSlice;
} xge_svg_def_t;

typedef struct xge_svg_clip_path_t {
	char* sId;
	xge_rect_t tRect;
	int iUnits;
	int bHasRect;
} xge_svg_clip_path_t;

typedef struct xge_svg_mask_rect_t {
	xge_rect_t tRect;
	float fOpacity;
} xge_svg_mask_rect_t;

typedef struct xge_svg_mask_t {
	char* sId;
	xge_svg_mask_rect_t* pRects;
	int iRectCount;
	int iRectCapacity;
	int iContentUnits;
} xge_svg_mask_t;

typedef struct xge_svg_linear_gradient_t {
	char* sId;
	char* sHrefId;
	float fX1;
	float fY1;
	float fX2;
	float fY2;
	int iUnits;
	int iSpread;
	int iFlags;
	xge_svg_style_t tStyle;
	xge_shape_ex_matrix_t tTransform;
	xge_shape_ex_color_stop_t* pStops;
	int iStopCount;
	int iStopCapacity;
} xge_svg_linear_gradient_t;

typedef struct xge_svg_radial_gradient_t {
	char* sId;
	char* sHrefId;
	float fCX;
	float fCY;
	float fR;
	float fFX;
	float fFY;
	int iUnits;
	int iSpread;
	int iFlags;
	xge_svg_style_t tStyle;
	xge_shape_ex_matrix_t tTransform;
	xge_shape_ex_color_stop_t* pStops;
	int iStopCount;
	int iStopCapacity;
} xge_svg_radial_gradient_t;

typedef struct xge_svg_style_rule_t {
	int iSelectorType;
	int iSpecificity;
	char* sName;
	char* sStyle;
} xge_svg_style_rule_t;

typedef struct xge_svg_pending_use_t {
	char sId[XGE_SVG_ID_MAX];
	xge_svg_style_t tStyle;
	float fX;
	float fY;
	float fW;
	float fH;
	int bHasWidth;
	int bHasHeight;
	int iTargetDef;
	int bResolved;
} xge_svg_pending_use_t;

struct xge_svg_t {
	uint32_t iMagic;
	int iRefCount;
	xge_shape_ex_scene pScene;
	xge_svg_def_t* pDefs;
	int iDefCount;
	int iDefCapacity;
	xge_svg_clip_path_t* pClipPaths;
	int iClipPathCount;
	int iClipPathCapacity;
	xge_svg_mask_t* pMasks;
	int iMaskCount;
	int iMaskCapacity;
	xge_svg_linear_gradient_t* pLinearGradients;
	int iLinearGradientCount;
	int iLinearGradientCapacity;
	xge_svg_radial_gradient_t* pRadialGradients;
	int iRadialGradientCount;
	int iRadialGradientCapacity;
	xge_svg_style_rule_t* pStyleRules;
	int iStyleRuleCount;
	int iStyleRuleCapacity;
	xge_svg_pending_use_t* pPendingUses;
	int iPendingUseCount;
	int iPendingUseCapacity;
	xge_rect_t tViewBox;
	int bHasViewBox;
	float fWidth;
	float fHeight;
	int iAspectAlignX;
	int iAspectAlignY;
	int iAspectMeetOrSlice;
};

static xge_svg_cache_entry_t* g_xgeSvgCacheHead;

static int __xgeSvgValid(xge_svg pSvg)
{
	return (pSvg != NULL) && (pSvg->iMagic == XGE_SVG_MAGIC);
}

static void __xgeSvgDefReset(xge_svg_def_t* pDef)
{
	int i;

	if ( pDef == NULL ) {
		return;
	}
	for ( i = 0; i < pDef->iShapeCount; i++ ) {
		xgeShapeExDestroy(pDef->pShapes[i]);
	}
	xrtFree(pDef->pShapes);
	xrtFree(pDef->sId);
	memset(pDef, 0, sizeof(*pDef));
}

static void __xgeSvgDefsClear(xge_svg pSvg)
{
	int i;

	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	for ( i = 0; i < pSvg->iDefCount; i++ ) {
		__xgeSvgDefReset(&pSvg->pDefs[i]);
	}
	xrtFree(pSvg->pDefs);
	pSvg->pDefs = NULL;
	pSvg->iDefCount = 0;
	pSvg->iDefCapacity = 0;
}

static int __xgeSvgDefFind(xge_svg pSvg, const char* sId)
{
	int i;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') ) {
		return -1;
	}
	for ( i = 0; i < pSvg->iDefCount; i++ ) {
		if ( (pSvg->pDefs[i].sId != NULL) && (strcmp(pSvg->pDefs[i].sId, sId) == 0) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeSvgDefGetOrCreate(xge_svg pSvg, const char* sId, int* pIndex)
{
	xge_svg_def_t* pDefs;
	int iIndex;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') || (pIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xgeSvgDefFind(pSvg, sId);
	if ( iIndex >= 0 ) {
		*pIndex = iIndex;
		return XGE_OK;
	}
	if ( pSvg->iDefCount >= pSvg->iDefCapacity ) {
		iCapacity = pSvg->iDefCapacity > 0 ? pSvg->iDefCapacity * 2 : 16;
		pDefs = (xge_svg_def_t*)xrtRealloc(pSvg->pDefs, (size_t)iCapacity * sizeof(*pDefs));
		if ( pDefs == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pDefs + pSvg->iDefCapacity, 0, (size_t)(iCapacity - pSvg->iDefCapacity) * sizeof(*pDefs));
		pSvg->pDefs = pDefs;
		pSvg->iDefCapacity = iCapacity;
	}
	iIndex = pSvg->iDefCount++;
	pSvg->pDefs[iIndex].sId = __xgeStrDup(sId);
	if ( pSvg->pDefs[iIndex].sId == NULL ) {
		pSvg->iDefCount--;
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSvg->pDefs[iIndex].iAspectAlignX = XGE_SVG_ASPECT_ALIGN_MID;
	pSvg->pDefs[iIndex].iAspectAlignY = XGE_SVG_ASPECT_ALIGN_MID;
	pSvg->pDefs[iIndex].iAspectMeetOrSlice = XGE_SVG_ASPECT_MEET;
	*pIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgDefAddShape(xge_svg_def_t* pDef, xge_shape_ex pShape)
{
	xge_shape_ex* pShapes;
	int iCapacity;

	if ( (pDef == NULL) || (pShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pDef->iShapeCount >= pDef->iShapeCapacity ) {
		iCapacity = pDef->iShapeCapacity > 0 ? pDef->iShapeCapacity * 2 : 4;
		pShapes = (xge_shape_ex*)xrtRealloc(pDef->pShapes, (size_t)iCapacity * sizeof(*pShapes));
		if ( pShapes == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pDef->pShapes = pShapes;
		pDef->iShapeCapacity = iCapacity;
	}
	xgeShapeExAddRef(pShape);
	pDef->pShapes[pDef->iShapeCount++] = pShape;
	return XGE_OK;
}

static void __xgeSvgClipPathReset(xge_svg_clip_path_t* pClip)
{
	if ( pClip == NULL ) {
		return;
	}
	xrtFree(pClip->sId);
	memset(pClip, 0, sizeof(*pClip));
}

static void __xgeSvgClipPathsClear(xge_svg pSvg)
{
	int i;

	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	for ( i = 0; i < pSvg->iClipPathCount; i++ ) {
		__xgeSvgClipPathReset(&pSvg->pClipPaths[i]);
	}
	xrtFree(pSvg->pClipPaths);
	pSvg->pClipPaths = NULL;
	pSvg->iClipPathCount = 0;
	pSvg->iClipPathCapacity = 0;
}

static int __xgeSvgClipPathFind(xge_svg pSvg, const char* sId)
{
	int i;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') ) {
		return -1;
	}
	for ( i = 0; i < pSvg->iClipPathCount; i++ ) {
		if ( (pSvg->pClipPaths[i].sId != NULL) && (strcmp(pSvg->pClipPaths[i].sId, sId) == 0) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeSvgClipPathGetOrCreate(xge_svg pSvg, const char* sId, int* pIndex)
{
	xge_svg_clip_path_t* pClipPaths;
	int iIndex;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') || (pIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xgeSvgClipPathFind(pSvg, sId);
	if ( iIndex >= 0 ) {
		*pIndex = iIndex;
		return XGE_OK;
	}
	if ( pSvg->iClipPathCount >= pSvg->iClipPathCapacity ) {
		iCapacity = pSvg->iClipPathCapacity > 0 ? pSvg->iClipPathCapacity * 2 : 8;
		pClipPaths = (xge_svg_clip_path_t*)xrtRealloc(pSvg->pClipPaths, (size_t)iCapacity * sizeof(*pClipPaths));
		if ( pClipPaths == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pClipPaths + pSvg->iClipPathCapacity, 0, (size_t)(iCapacity - pSvg->iClipPathCapacity) * sizeof(*pClipPaths));
		pSvg->pClipPaths = pClipPaths;
		pSvg->iClipPathCapacity = iCapacity;
	}
	iIndex = pSvg->iClipPathCount++;
	pSvg->pClipPaths[iIndex].sId = __xgeStrDup(sId);
	if ( pSvg->pClipPaths[iIndex].sId == NULL ) {
		pSvg->iClipPathCount--;
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	*pIndex = iIndex;
	return XGE_OK;
}

static void __xgeSvgMaskReset(xge_svg_mask_t* pMask)
{
	if ( pMask == NULL ) {
		return;
	}
	xrtFree(pMask->sId);
	xrtFree(pMask->pRects);
	memset(pMask, 0, sizeof(*pMask));
}

static void __xgeSvgMasksClear(xge_svg pSvg)
{
	int i;

	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	for ( i = 0; i < pSvg->iMaskCount; i++ ) {
		__xgeSvgMaskReset(&pSvg->pMasks[i]);
	}
	xrtFree(pSvg->pMasks);
	pSvg->pMasks = NULL;
	pSvg->iMaskCount = 0;
	pSvg->iMaskCapacity = 0;
}

static int __xgeSvgMaskFind(xge_svg pSvg, const char* sId)
{
	int i;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') ) {
		return -1;
	}
	for ( i = 0; i < pSvg->iMaskCount; i++ ) {
		if ( (pSvg->pMasks[i].sId != NULL) && (strcmp(pSvg->pMasks[i].sId, sId) == 0) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeSvgMaskGetOrCreate(xge_svg pSvg, const char* sId, int* pIndex)
{
	xge_svg_mask_t* pMasks;
	int iIndex;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') || (pIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xgeSvgMaskFind(pSvg, sId);
	if ( iIndex >= 0 ) {
		*pIndex = iIndex;
		return XGE_OK;
	}
	if ( pSvg->iMaskCount >= pSvg->iMaskCapacity ) {
		iCapacity = pSvg->iMaskCapacity > 0 ? pSvg->iMaskCapacity * 2 : 8;
		pMasks = (xge_svg_mask_t*)xrtRealloc(pSvg->pMasks, (size_t)iCapacity * sizeof(*pMasks));
		if ( pMasks == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pMasks + pSvg->iMaskCapacity, 0, (size_t)(iCapacity - pSvg->iMaskCapacity) * sizeof(*pMasks));
		pSvg->pMasks = pMasks;
		pSvg->iMaskCapacity = iCapacity;
	}
	iIndex = pSvg->iMaskCount++;
	pSvg->pMasks[iIndex].sId = __xgeStrDup(sId);
	if ( pSvg->pMasks[iIndex].sId == NULL ) {
		pSvg->iMaskCount--;
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSvg->pMasks[iIndex].iContentUnits = XGE_SVG_MASK_USER_SPACE;
	*pIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgMaskAddRect(xge_svg_mask_t* pMask, xge_svg_mask_rect_t tRect)
{
	xge_svg_mask_rect_t* pRects;
	int iCapacity;

	if ( pMask == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pMask->iRectCount >= pMask->iRectCapacity ) {
		iCapacity = pMask->iRectCapacity > 0 ? pMask->iRectCapacity * 2 : 4;
		pRects = (xge_svg_mask_rect_t*)xrtRealloc(pMask->pRects, (size_t)iCapacity * sizeof(*pRects));
		if ( pRects == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pMask->pRects = pRects;
		pMask->iRectCapacity = iCapacity;
	}
	pMask->pRects[pMask->iRectCount++] = tRect;
	return XGE_OK;
}

static void __xgeSvgLinearGradientReset(xge_svg_linear_gradient_t* pGradient)
{
	if ( pGradient == NULL ) {
		return;
	}
	xrtFree(pGradient->sId);
	xrtFree(pGradient->sHrefId);
	xrtFree(pGradient->pStops);
	memset(pGradient, 0, sizeof(*pGradient));
}

static void __xgeSvgLinearGradientsClear(xge_svg pSvg)
{
	int i;

	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	for ( i = 0; i < pSvg->iLinearGradientCount; i++ ) {
		__xgeSvgLinearGradientReset(&pSvg->pLinearGradients[i]);
	}
	xrtFree(pSvg->pLinearGradients);
	pSvg->pLinearGradients = NULL;
	pSvg->iLinearGradientCount = 0;
	pSvg->iLinearGradientCapacity = 0;
}

static void __xgeSvgStyleRuleReset(xge_svg_style_rule_t* pRule)
{
	if ( pRule == NULL ) {
		return;
	}
	xrtFree(pRule->sName);
	xrtFree(pRule->sStyle);
	memset(pRule, 0, sizeof(*pRule));
}

static void __xgeSvgStyleRulesClear(xge_svg pSvg)
{
	int i;

	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	for ( i = 0; i < pSvg->iStyleRuleCount; i++ ) {
		__xgeSvgStyleRuleReset(&pSvg->pStyleRules[i]);
	}
	xrtFree(pSvg->pStyleRules);
	pSvg->pStyleRules = NULL;
	pSvg->iStyleRuleCount = 0;
	pSvg->iStyleRuleCapacity = 0;
}

static void __xgeSvgPendingUsesClear(xge_svg pSvg)
{
	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	xrtFree(pSvg->pPendingUses);
	pSvg->pPendingUses = NULL;
	pSvg->iPendingUseCount = 0;
	pSvg->iPendingUseCapacity = 0;
}

static int __xgeSvgPendingUseAdd(xge_svg pSvg, const char* sId, const xge_svg_style_t* pStyle, int iTargetDef, float fX, float fY, float fW, float fH, int bHasWidth, int bHasHeight)
{
	xge_svg_pending_use_t* pPendingUses;
	xge_svg_pending_use_t* pUse;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') || (pStyle == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pSvg->iPendingUseCount >= pSvg->iPendingUseCapacity ) {
		iCapacity = pSvg->iPendingUseCapacity > 0 ? pSvg->iPendingUseCapacity * 2 : 16;
		pPendingUses = (xge_svg_pending_use_t*)xrtRealloc(pSvg->pPendingUses, (size_t)iCapacity * sizeof(*pPendingUses));
		if ( pPendingUses == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pPendingUses + pSvg->iPendingUseCapacity, 0, (size_t)(iCapacity - pSvg->iPendingUseCapacity) * sizeof(*pPendingUses));
		pSvg->pPendingUses = pPendingUses;
		pSvg->iPendingUseCapacity = iCapacity;
	}
	pUse = &pSvg->pPendingUses[pSvg->iPendingUseCount++];
	memset(pUse, 0, sizeof(*pUse));
	{
		size_t iLen = strlen(sId);
		if ( iLen >= sizeof(pUse->sId) ) {
			iLen = sizeof(pUse->sId) - 1u;
		}
		memcpy(pUse->sId, sId, iLen);
		pUse->sId[iLen] = '\0';
	}
	pUse->tStyle = *pStyle;
	pUse->fX = fX;
	pUse->fY = fY;
	pUse->fW = fW;
	pUse->fH = fH;
	pUse->bHasWidth = bHasWidth;
	pUse->bHasHeight = bHasHeight;
	pUse->iTargetDef = iTargetDef;
	return XGE_OK;
}

static void __xgeSvgTrimRange(const char** ppStart, const char** ppEnd)
{
	const char* pStart;
	const char* pEnd;

	if ( (ppStart == NULL) || (ppEnd == NULL) || (*ppStart == NULL) || (*ppEnd == NULL) ) {
		return;
	}
	pStart = *ppStart;
	pEnd = *ppEnd;
	while ( (pStart < pEnd) && ((*pStart == ' ') || (*pStart == '\t') || (*pStart == '\r') || (*pStart == '\n')) ) {
		pStart++;
	}
	while ( (pEnd > pStart) && ((pEnd[-1] == ' ') || (pEnd[-1] == '\t') || (pEnd[-1] == '\r') || (pEnd[-1] == '\n')) ) {
		pEnd--;
	}
	*ppStart = pStart;
	*ppEnd = pEnd;
}

static int __xgeSvgRangeStartsWith(const char* pStart, const char* pEnd, const char* sText)
{
	int iLen;

	if ( (pStart == NULL) || (pEnd == NULL) || (sText == NULL) || (pEnd < pStart) ) {
		return 0;
	}
	iLen = (int)strlen(sText);
	return ((int)(pEnd - pStart) >= iLen) && (strncmp(pStart, sText, (size_t)iLen) == 0);
}

static int __xgeSvgRangeDup(const char* pStart, const char* pEnd, char** ppOut)
{
	char* sOut;
	int iLen;

	if ( ppOut == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppOut = NULL;
	if ( (pStart == NULL) || (pEnd == NULL) || (pEnd < pStart) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iLen = (int)(pEnd - pStart);
	sOut = (char*)xrtMalloc((size_t)iLen + 1u);
	if ( sOut == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( iLen > 0 ) {
		memcpy(sOut, pStart, (size_t)iLen);
	}
	sOut[iLen] = '\0';
	*ppOut = sOut;
	return XGE_OK;
}

static int __xgeSvgStyleSelectorSpecificity(int iSelectorType, const char* sName);

static int __xgeSvgStyleRuleAdd(xge_svg pSvg, int iSelectorType, const char* pNameStart, const char* pNameEnd, const char* pStyleStart, const char* pStyleEnd)
{
	xge_svg_style_rule_t* pRules;
	xge_svg_style_rule_t* pRule;
	int iCapacity;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pNameStart == NULL) || (pNameEnd == NULL) || (pStyleStart == NULL) || (pStyleEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeSvgTrimRange(&pNameStart, &pNameEnd);
	if ( pNameEnd <= pNameStart ) {
		return XGE_OK;
	}
	if ( pSvg->iStyleRuleCount >= pSvg->iStyleRuleCapacity ) {
		iCapacity = pSvg->iStyleRuleCapacity > 0 ? pSvg->iStyleRuleCapacity * 2 : 16;
		pRules = (xge_svg_style_rule_t*)xrtRealloc(pSvg->pStyleRules, (size_t)iCapacity * sizeof(*pRules));
		if ( pRules == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pRules + pSvg->iStyleRuleCapacity, 0, (size_t)(iCapacity - pSvg->iStyleRuleCapacity) * sizeof(*pRules));
		pSvg->pStyleRules = pRules;
		pSvg->iStyleRuleCapacity = iCapacity;
	}
	pRule = &pSvg->pStyleRules[pSvg->iStyleRuleCount];
	memset(pRule, 0, sizeof(*pRule));
	pRule->iSelectorType = iSelectorType;
	iRet = __xgeSvgRangeDup(pNameStart, pNameEnd, &pRule->sName);
	if ( iRet == XGE_OK ) {
		pRule->iSpecificity = __xgeSvgStyleSelectorSpecificity(iSelectorType, pRule->sName);
	}
	if ( iRet == XGE_OK ) {
		__xgeSvgTrimRange(&pStyleStart, &pStyleEnd);
		iRet = __xgeSvgRangeDup(pStyleStart, pStyleEnd, &pRule->sStyle);
	}
	if ( iRet != XGE_OK ) {
		__xgeSvgStyleRuleReset(pRule);
		return iRet;
	}
	pSvg->iStyleRuleCount++;
	return XGE_OK;
}

static int __xgeSvgLinearGradientFind(xge_svg pSvg, const char* sId)
{
	int i;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') ) {
		return -1;
	}
	for ( i = 0; i < pSvg->iLinearGradientCount; i++ ) {
		if ( (pSvg->pLinearGradients[i].sId != NULL) && (strcmp(pSvg->pLinearGradients[i].sId, sId) == 0) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeSvgLinearGradientGetOrCreate(xge_svg pSvg, const char* sId, int* pIndex)
{
	xge_svg_linear_gradient_t* pGradients;
	int iIndex;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') || (pIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xgeSvgLinearGradientFind(pSvg, sId);
	if ( iIndex >= 0 ) {
		*pIndex = iIndex;
		return XGE_OK;
	}
	if ( pSvg->iLinearGradientCount >= pSvg->iLinearGradientCapacity ) {
		iCapacity = pSvg->iLinearGradientCapacity > 0 ? pSvg->iLinearGradientCapacity * 2 : 8;
		pGradients = (xge_svg_linear_gradient_t*)xrtRealloc(pSvg->pLinearGradients, (size_t)iCapacity * sizeof(*pGradients));
		if ( pGradients == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pGradients + pSvg->iLinearGradientCapacity, 0, (size_t)(iCapacity - pSvg->iLinearGradientCapacity) * sizeof(*pGradients));
		pSvg->pLinearGradients = pGradients;
		pSvg->iLinearGradientCapacity = iCapacity;
	}
	iIndex = pSvg->iLinearGradientCount++;
	pSvg->pLinearGradients[iIndex].sId = __xgeStrDup(sId);
	if ( pSvg->pLinearGradients[iIndex].sId == NULL ) {
		pSvg->iLinearGradientCount--;
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSvg->pLinearGradients[iIndex].fX1 = 0.0f;
	pSvg->pLinearGradients[iIndex].fY1 = 0.0f;
	pSvg->pLinearGradients[iIndex].fX2 = 1.0f;
	pSvg->pLinearGradients[iIndex].fY2 = 0.0f;
	pSvg->pLinearGradients[iIndex].iUnits = XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX;
	pSvg->pLinearGradients[iIndex].iSpread = XGE_SHAPE_EX_GRADIENT_SPREAD_PAD;
	pSvg->pLinearGradients[iIndex].tTransform = __xgeShapeExMatrixIdentity();
	*pIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgLinearGradientAddStop(xge_svg_linear_gradient_t* pGradient, xge_shape_ex_color_stop_t tStop)
{
	xge_shape_ex_color_stop_t* pStops;
	int iCapacity;

	if ( pGradient == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pGradient->iStopCount >= pGradient->iStopCapacity ) {
		iCapacity = pGradient->iStopCapacity > 0 ? pGradient->iStopCapacity * 2 : 4;
		pStops = (xge_shape_ex_color_stop_t*)xrtRealloc(pGradient->pStops, (size_t)iCapacity * sizeof(*pStops));
		if ( pStops == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pGradient->pStops = pStops;
		pGradient->iStopCapacity = iCapacity;
	}
	if ( tStop.fOffset < 0.0f ) tStop.fOffset = 0.0f;
	if ( tStop.fOffset > 1.0f ) tStop.fOffset = 1.0f;
	pGradient->pStops[pGradient->iStopCount++] = tStop;
	pGradient->iFlags |= XGE_SVG_GRADIENT_HAS_STOPS;
	return XGE_OK;
}

static void __xgeSvgRadialGradientReset(xge_svg_radial_gradient_t* pGradient)
{
	if ( pGradient == NULL ) {
		return;
	}
	xrtFree(pGradient->sId);
	xrtFree(pGradient->sHrefId);
	xrtFree(pGradient->pStops);
	memset(pGradient, 0, sizeof(*pGradient));
}

static void __xgeSvgRadialGradientsClear(xge_svg pSvg)
{
	int i;

	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	for ( i = 0; i < pSvg->iRadialGradientCount; i++ ) {
		__xgeSvgRadialGradientReset(&pSvg->pRadialGradients[i]);
	}
	xrtFree(pSvg->pRadialGradients);
	pSvg->pRadialGradients = NULL;
	pSvg->iRadialGradientCount = 0;
	pSvg->iRadialGradientCapacity = 0;
}

static int __xgeSvgRadialGradientFind(xge_svg pSvg, const char* sId)
{
	int i;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') ) {
		return -1;
	}
	for ( i = 0; i < pSvg->iRadialGradientCount; i++ ) {
		if ( (pSvg->pRadialGradients[i].sId != NULL) && (strcmp(pSvg->pRadialGradients[i].sId, sId) == 0) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeSvgRadialGradientGetOrCreate(xge_svg pSvg, const char* sId, int* pIndex)
{
	xge_svg_radial_gradient_t* pGradients;
	int iIndex;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') || (pIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xgeSvgRadialGradientFind(pSvg, sId);
	if ( iIndex >= 0 ) {
		*pIndex = iIndex;
		return XGE_OK;
	}
	if ( pSvg->iRadialGradientCount >= pSvg->iRadialGradientCapacity ) {
		iCapacity = pSvg->iRadialGradientCapacity > 0 ? pSvg->iRadialGradientCapacity * 2 : 8;
		pGradients = (xge_svg_radial_gradient_t*)xrtRealloc(pSvg->pRadialGradients, (size_t)iCapacity * sizeof(*pGradients));
		if ( pGradients == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pGradients + pSvg->iRadialGradientCapacity, 0, (size_t)(iCapacity - pSvg->iRadialGradientCapacity) * sizeof(*pGradients));
		pSvg->pRadialGradients = pGradients;
		pSvg->iRadialGradientCapacity = iCapacity;
	}
	iIndex = pSvg->iRadialGradientCount++;
	pSvg->pRadialGradients[iIndex].sId = __xgeStrDup(sId);
	if ( pSvg->pRadialGradients[iIndex].sId == NULL ) {
		pSvg->iRadialGradientCount--;
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSvg->pRadialGradients[iIndex].fCX = 0.5f;
	pSvg->pRadialGradients[iIndex].fCY = 0.5f;
	pSvg->pRadialGradients[iIndex].fR = 0.5f;
	pSvg->pRadialGradients[iIndex].fFX = 0.5f;
	pSvg->pRadialGradients[iIndex].fFY = 0.5f;
	pSvg->pRadialGradients[iIndex].iUnits = XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX;
	pSvg->pRadialGradients[iIndex].iSpread = XGE_SHAPE_EX_GRADIENT_SPREAD_PAD;
	pSvg->pRadialGradients[iIndex].tTransform = __xgeShapeExMatrixIdentity();
	*pIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgRadialGradientAddStop(xge_svg_radial_gradient_t* pGradient, xge_shape_ex_color_stop_t tStop)
{
	xge_shape_ex_color_stop_t* pStops;
	int iCapacity;

	if ( pGradient == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pGradient->iStopCount >= pGradient->iStopCapacity ) {
		iCapacity = pGradient->iStopCapacity > 0 ? pGradient->iStopCapacity * 2 : 4;
		pStops = (xge_shape_ex_color_stop_t*)xrtRealloc(pGradient->pStops, (size_t)iCapacity * sizeof(*pStops));
		if ( pStops == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pGradient->pStops = pStops;
		pGradient->iStopCapacity = iCapacity;
	}
	if ( tStop.fOffset < 0.0f ) tStop.fOffset = 0.0f;
	if ( tStop.fOffset > 1.0f ) tStop.fOffset = 1.0f;
	pGradient->pStops[pGradient->iStopCount++] = tStop;
	pGradient->iFlags |= XGE_SVG_GRADIENT_HAS_STOPS;
	return XGE_OK;
}

static int __xgeSvgResolveLinearGradient(xge_svg pSvg, int iIndex, int iDepth)
{
	xge_svg_linear_gradient_t* pGradient;
	xge_svg_linear_gradient_t* pSource;
	int iSource;
	int i;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iIndex < 0) || (iIndex >= pSvg->iLinearGradientCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iDepth > pSvg->iLinearGradientCount ) {
		return XGE_OK;
	}
	pGradient = &pSvg->pLinearGradients[iIndex];
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_RESOLVED) != 0 ) {
		return XGE_OK;
	}
	if ( (pGradient->sHrefId == NULL) || (pGradient->sHrefId[0] == '\0') ) {
		pGradient->iFlags |= XGE_SVG_GRADIENT_RESOLVED;
		return XGE_OK;
	}
	iSource = __xgeSvgLinearGradientFind(pSvg, pGradient->sHrefId);
	if ( iSource < 0 ) {
		return XGE_OK;
	}
	if ( iSource == iIndex ) {
		pGradient->iFlags |= XGE_SVG_GRADIENT_RESOLVED;
		return XGE_OK;
	}
	iRet = __xgeSvgResolveLinearGradient(pSvg, iSource, iDepth + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pSource = &pSvg->pLinearGradients[iSource];
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_X1) == 0 ) pGradient->fX1 = pSource->fX1;
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_Y1) == 0 ) pGradient->fY1 = pSource->fY1;
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_X2) == 0 ) pGradient->fX2 = pSource->fX2;
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_Y2) == 0 ) pGradient->fY2 = pSource->fY2;
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_UNITS) == 0 ) pGradient->iUnits = pSource->iUnits;
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_SPREAD) == 0 ) pGradient->iSpread = pSource->iSpread;
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_TRANSFORM) == 0 ) pGradient->tTransform = pSource->tTransform;
	if ( ((pGradient->iFlags & XGE_SVG_GRADIENT_HAS_STOPS) == 0) && (pSource->iStopCount > 0) ) {
		pGradient->iStopCount = 0;
		for ( i = 0; i < pSource->iStopCount; i++ ) {
			iRet = __xgeSvgLinearGradientAddStop(pGradient, pSource->pStops[i]);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
	}
	pGradient->iFlags |= XGE_SVG_GRADIENT_RESOLVED;
	return XGE_OK;
}

static int __xgeSvgResolveRadialGradient(xge_svg pSvg, int iIndex, int iDepth)
{
	xge_svg_radial_gradient_t* pGradient;
	xge_svg_radial_gradient_t* pSource;
	int iSource;
	int i;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iIndex < 0) || (iIndex >= pSvg->iRadialGradientCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iDepth > pSvg->iRadialGradientCount ) {
		return XGE_OK;
	}
	pGradient = &pSvg->pRadialGradients[iIndex];
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_RESOLVED) != 0 ) {
		return XGE_OK;
	}
	if ( (pGradient->sHrefId == NULL) || (pGradient->sHrefId[0] == '\0') ) {
		pGradient->iFlags |= XGE_SVG_GRADIENT_RESOLVED;
		return XGE_OK;
	}
	iSource = __xgeSvgRadialGradientFind(pSvg, pGradient->sHrefId);
	if ( iSource < 0 ) {
		return XGE_OK;
	}
	if ( iSource == iIndex ) {
		pGradient->iFlags |= XGE_SVG_GRADIENT_RESOLVED;
		return XGE_OK;
	}
	iRet = __xgeSvgResolveRadialGradient(pSvg, iSource, iDepth + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pSource = &pSvg->pRadialGradients[iSource];
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_CX) == 0 ) pGradient->fCX = pSource->fCX;
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_CY) == 0 ) pGradient->fCY = pSource->fCY;
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_R) == 0 ) pGradient->fR = pSource->fR;
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_FX) == 0 ) pGradient->fFX = pSource->fFX;
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_FY) == 0 ) pGradient->fFY = pSource->fFY;
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_UNITS) == 0 ) pGradient->iUnits = pSource->iUnits;
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_SPREAD) == 0 ) pGradient->iSpread = pSource->iSpread;
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_TRANSFORM) == 0 ) pGradient->tTransform = pSource->tTransform;
	if ( ((pGradient->iFlags & XGE_SVG_GRADIENT_HAS_STOPS) == 0) && (pSource->iStopCount > 0) ) {
		pGradient->iStopCount = 0;
		for ( i = 0; i < pSource->iStopCount; i++ ) {
			iRet = __xgeSvgRadialGradientAddStop(pGradient, pSource->pStops[i]);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
	}
	pGradient->iFlags |= XGE_SVG_GRADIENT_RESOLVED;
	return XGE_OK;
}

static int __xgeSvgResolveGradients(xge_svg pSvg)
{
	int i;
	int iRet;

	if ( !__xgeSvgValid(pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pSvg->iLinearGradientCount; i++ ) {
		iRet = __xgeSvgResolveLinearGradient(pSvg, i, 0);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	for ( i = 0; i < pSvg->iRadialGradientCount; i++ ) {
		iRet = __xgeSvgResolveRadialGradient(pSvg, i, 0);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeSvgIsNameEnd(char c)
{
	return (c == '\0') || (c == '>') || (c == '/') || (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
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

static int __xgeSvgTagNameEquals(const char* pTag, const char* sName)
{
	size_t iLen;

	if ( (pTag == NULL) || (sName == NULL) || (*pTag != '<') ) {
		return 0;
	}
	pTag++;
	if ( *pTag == '/' ) {
		return 0;
	}
	while ( (*pTag == ' ') || (*pTag == '\t') || (*pTag == '\r') || (*pTag == '\n') ) {
		pTag++;
	}
	iLen = strlen(sName);
	return (strncmp(pTag, sName, iLen) == 0) && __xgeSvgIsNameEnd(pTag[iLen]);
}

static int __xgeSvgTagNameCopy(const char* pTag, char* sOut, int iOutCapacity)
{
	const char* p;
	int iLen;

	if ( (pTag == NULL) || (sOut == NULL) || (iOutCapacity <= 0) || (*pTag != '<') ) {
		return 0;
	}
	sOut[0] = '\0';
	p = pTag + 1;
	if ( *p == '/' ) {
		return 0;
	}
	while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') ) {
		p++;
	}
	iLen = 0;
	while ( (p[iLen] != '\0') && !__xgeSvgIsNameEnd(p[iLen]) ) {
		iLen++;
	}
	if ( iLen <= 0 ) {
		return 0;
	}
	if ( iLen >= iOutCapacity ) {
		iLen = iOutCapacity - 1;
	}
	memcpy(sOut, p, (size_t)iLen);
	sOut[iLen] = '\0';
	return 1;
}

static int __xgeSvgIsCloseTagName(const char* pTag, const char* sName)
{
	size_t iLen;

	if ( (pTag == NULL) || (sName == NULL) || (*pTag != '<') ) {
		return 0;
	}
	pTag++;
	while ( (*pTag == ' ') || (*pTag == '\t') || (*pTag == '\r') || (*pTag == '\n') ) {
		pTag++;
	}
	if ( *pTag != '/' ) {
		return 0;
	}
	pTag++;
	while ( (*pTag == ' ') || (*pTag == '\t') || (*pTag == '\r') || (*pTag == '\n') ) {
		pTag++;
	}
	iLen = strlen(sName);
	return (strncmp(pTag, sName, iLen) == 0) && __xgeSvgIsNameEnd(pTag[iLen]);
}

static int __xgeSvgIsSelfClosingTag(const char* pTag, const char* pTagEnd)
{
	const char* p;

	if ( (pTag == NULL) || (pTagEnd == NULL) || (pTagEnd <= pTag) ) {
		return 0;
	}
	p = pTagEnd;
	while ( (p > pTag) && ((p[-1] == ' ') || (p[-1] == '\t') || (p[-1] == '\r') || (p[-1] == '\n')) ) {
		p--;
	}
	return (p > pTag) && (p[-1] == '/');
}

static int __xgeSvgCssSelectorSupported(const char* pStart, const char* pEnd)
{
	const char* p;

	if ( (pStart == NULL) || (pEnd == NULL) || (pEnd <= pStart) ) {
		return 0;
	}
	for ( p = pStart; p < pEnd; p++ ) {
		if ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') ||
		     (*p == '>') || (*p == '+') || (*p == '~') || (*p == '[') ||
		     (*p == ']') || (*p == ':') || (*p == '*') ) {
			return 0;
		}
	}
	return 1;
}

static int __xgeSvgStyleSelectorSpecificity(int iSelectorType, const char* sName)
{
	const char* p;
	int iSpecificity;

	if ( sName == NULL ) {
		return 0;
	}
	if ( iSelectorType == XGE_SVG_STYLE_SELECTOR_ID ) {
		return 100;
	}
	if ( iSelectorType == XGE_SVG_STYLE_SELECTOR_CLASS ) {
		return 10;
	}
	if ( iSelectorType == XGE_SVG_STYLE_SELECTOR_TAG ) {
		return 1;
	}
	if ( iSelectorType != XGE_SVG_STYLE_SELECTOR_SIMPLE ) {
		return 0;
	}
	p = sName;
	iSpecificity = 0;
	if ( (*p != '\0') && (*p != '.') && (*p != '#') ) {
		while ( (*p != '\0') && (*p != '.') && (*p != '#') ) {
			p++;
		}
		iSpecificity += 1;
	}
	while ( *p != '\0' ) {
		if ( *p == '.' ) {
			p++;
			if ( (*p == '\0') || (*p == '.') || (*p == '#') ) {
				return 0;
			}
			while ( (*p != '\0') && (*p != '.') && (*p != '#') ) {
				p++;
			}
			iSpecificity += 10;
		} else if ( *p == '#' ) {
			p++;
			if ( (*p == '\0') || (*p == '.') || (*p == '#') ) {
				return 0;
			}
			while ( (*p != '\0') && (*p != '.') && (*p != '#') ) {
				p++;
			}
			iSpecificity += 100;
		} else {
			return 0;
		}
	}
	return iSpecificity;
}

static const char* __xgeSvgCssSkipAtRule(const char* pStart, const char* pEnd)
{
	const char* p;
	int iDepth;
	char cQuote;

	if ( (pStart == NULL) || (pEnd == NULL) || (pStart >= pEnd) ) {
		return pEnd;
	}
	p = pStart;
	iDepth = 0;
	cQuote = 0;
	while ( p < pEnd ) {
		if ( cQuote != 0 ) {
			if ( *p == '\\' ) {
				p += (p + 1 < pEnd) ? 2 : 1;
				continue;
			}
			if ( *p == cQuote ) {
				cQuote = 0;
			}
			p++;
			continue;
		}
		if ( (*p == '"') || (*p == '\'') ) {
			cQuote = *p++;
			continue;
		}
		if ( ((p + 1) < pEnd) && (p[0] == '/') && (p[1] == '*') ) {
			p += 2;
			while ( ((p + 1) < pEnd) && !((p[0] == '*') && (p[1] == '/')) ) {
				p++;
			}
			if ( (p + 1) < pEnd ) {
				p += 2;
			}
			continue;
		}
		if ( (*p == ';') && (iDepth == 0) ) {
			return p + 1;
		}
		if ( *p == '{' ) {
			iDepth++;
			p++;
			continue;
		}
		if ( *p == '}' ) {
			if ( iDepth > 0 ) {
				iDepth--;
				p++;
				if ( iDepth == 0 ) {
					return p;
				}
				continue;
			}
			return p + 1;
		}
		p++;
	}
	return pEnd;
}

static int __xgeSvgParseStyleSelector(xge_svg pSvg, const char* pStart, const char* pEnd, const char* pStyleStart, const char* pStyleEnd)
{
	__xgeSvgTrimRange(&pStart, &pEnd);
	if ( pEnd <= pStart ) {
		return XGE_OK;
	}
	if ( !__xgeSvgCssSelectorSupported(pStart, pEnd) ) {
		return XGE_OK;
	}
	return __xgeSvgStyleRuleAdd(pSvg, XGE_SVG_STYLE_SELECTOR_SIMPLE, pStart, pEnd, pStyleStart, pStyleEnd);
}

static int __xgeSvgParseStyleSelectorList(xge_svg pSvg, const char* pStart, const char* pEnd, const char* pStyleStart, const char* pStyleEnd)
{
	const char* p;
	const char* pItemStart;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pStart == NULL) || (pEnd == NULL) || (pStyleStart == NULL) || (pStyleEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	p = pStart;
	pItemStart = pStart;
	while ( p <= pEnd ) {
		if ( (p == pEnd) || (*p == ',') ) {
			iRet = __xgeSvgParseStyleSelector(pSvg, pItemStart, p, pStyleStart, pStyleEnd);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			pItemStart = p + 1;
		}
		p++;
	}
	return XGE_OK;
}

static int __xgeSvgParseStyleText(xge_svg pSvg, const char* pStart, const char* pEnd)
{
	const char* p;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pStart == NULL) || (pEnd == NULL) || (pEnd < pStart) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	p = pStart;
	while ( p < pEnd ) {
		const char* pSelectorStart;
		const char* pSelectorEnd;
		const char* pStyleStart;
		const char* pStyleEnd;

		while ( p < pEnd ) {
			if ( (p + 1 < pEnd) && (p[0] == '/') && (p[1] == '*') ) {
				p += 2;
				while ( (p + 1 < pEnd) && !((p[0] == '*') && (p[1] == '/')) ) {
					p++;
				}
				if ( p + 1 < pEnd ) {
					p += 2;
				}
				continue;
			}
			if ( __xgeSvgRangeStartsWith(p, pEnd, "<!--") ) {
				p += 4;
				continue;
			}
			if ( __xgeSvgRangeStartsWith(p, pEnd, "-->") ) {
				p += 3;
				continue;
			}
			if ( (*p != ' ') && (*p != '\t') && (*p != '\r') && (*p != '\n') ) {
				break;
			}
			p++;
		}
		if ( (p < pEnd) && (*p == '@') ) {
			p = __xgeSvgCssSkipAtRule(p, pEnd);
			continue;
		}
		pSelectorStart = p;
		while ( (p < pEnd) && (*p != '{') ) {
			p++;
		}
		if ( p >= pEnd ) {
			break;
		}
		pSelectorEnd = p;
		p++;
		pStyleStart = p;
		while ( (p < pEnd) && (*p != '}') ) {
			p++;
		}
		pStyleEnd = p;
		iRet = __xgeSvgParseStyleSelectorList(pSvg, pSelectorStart, pSelectorEnd, pStyleStart, pStyleEnd);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( p < pEnd ) {
			p++;
		}
	}
	return XGE_OK;
}

static int __xgeSvgParseStyleBlocks(xge_svg pSvg, const char* sText)
{
	const char* p;

	if ( !__xgeSvgValid(pSvg) || (sText == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	p = sText;
	while ( (p = strstr(p, "<style")) != NULL ) {
		const char* pTagEnd = __xgeSvgFindTagEnd(p);
		const char* pClose;
		int iRet;

		if ( (pTagEnd == NULL) || !__xgeSvgTagNameEquals(p, "style") ) {
			p++;
			continue;
		}
		pClose = strstr(pTagEnd + 1, "</style");
		if ( pClose == NULL ) {
			break;
		}
		{
			const char* pStyleStart = pTagEnd + 1;
			const char* pStyleEnd = pClose;

			__xgeSvgTrimRange(&pStyleStart, &pStyleEnd);
			if ( __xgeSvgRangeStartsWith(pStyleStart, pStyleEnd, "<![CDATA[") ) {
				const char* pCDataEnd;

				pStyleStart += 9;
				pCDataEnd = strstr(pStyleStart, "]]>");
				if ( (pCDataEnd != NULL) && (pCDataEnd <= pStyleEnd) ) {
					pStyleEnd = pCDataEnd;
				}
			}
			__xgeSvgTrimRange(&pStyleStart, &pStyleEnd);
			if ( __xgeSvgRangeStartsWith(pStyleStart, pStyleEnd, "<!--") ) {
				pStyleStart += 4;
			}
			if ( ((pStyleEnd - pStyleStart) >= 3) && (strncmp(pStyleEnd - 3, "-->", 3) == 0) ) {
				pStyleEnd -= 3;
			}
			iRet = __xgeSvgParseStyleText(pSvg, pStyleStart, pStyleEnd);
		}
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		p = pClose + 7;
	}
	return XGE_OK;
}

static int __xgeSvgAttrCopy(const char* pTag, const char* pTagEnd, const char* sName, char* sOut, int iOutCapacity)
{
	const char* p;
	size_t iNameLen;

	if ( (pTag == NULL) || (pTagEnd == NULL) || (sName == NULL) || (sOut == NULL) || (iOutCapacity <= 0) ) {
		return 0;
	}
	sOut[0] = '\0';
	iNameLen = strlen(sName);
	p = pTag;
	while ( p < pTagEnd ) {
		const char* pValue;
		const char* pValueEnd;
		int iLen;
		char cQuote;

		if ( ((p == pTag) || __xgeSvgIsNameEnd(*(p - 1)) || (*(p - 1) == '<')) &&
		     ((size_t)(pTagEnd - p) > iNameLen) &&
		     (strncmp(p, sName, iNameLen) == 0) ) {
			const char* q = p + iNameLen;
			while ( (q < pTagEnd) && ((*q == ' ') || (*q == '\t') || (*q == '\r') || (*q == '\n')) ) q++;
			if ( (q >= pTagEnd) || (*q != '=') ) {
				p++;
				continue;
			}
			q++;
			while ( (q < pTagEnd) && ((*q == ' ') || (*q == '\t') || (*q == '\r') || (*q == '\n')) ) q++;
			if ( q >= pTagEnd ) {
				return 0;
			}
			cQuote = 0;
			if ( (*q == '"') || (*q == '\'') ) {
				cQuote = *q++;
			}
			pValue = q;
			if ( cQuote != 0 ) {
				while ( (q < pTagEnd) && (*q != cQuote) ) q++;
				pValueEnd = q;
			} else {
				while ( (q < pTagEnd) && !__xgeSvgIsNameEnd(*q) ) q++;
				pValueEnd = q;
			}
			iLen = (int)(pValueEnd - pValue);
			if ( iLen >= iOutCapacity ) {
				iLen = iOutCapacity - 1;
			}
			memcpy(sOut, pValue, (size_t)iLen);
			sOut[iLen] = '\0';
			return 1;
		}
		p++;
	}
	return 0;
}

static int __xgeSvgHrefIdCopy(const char* pTag, const char* pTagEnd, char* sOut, int iOutCapacity)
{
	char sValue[XGE_SVG_ATTR_MAX];
	const char* pValue;
	int iLen;

	if ( (sOut == NULL) || (iOutCapacity <= 0) ) {
		return 0;
	}
	sOut[0] = '\0';
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "href", sValue, sizeof(sValue)) &&
	     !__xgeSvgAttrCopy(pTag, pTagEnd, "xlink:href", sValue, sizeof(sValue)) ) {
		return 0;
	}
	pValue = sValue;
	while ( (*pValue == ' ') || (*pValue == '\t') || (*pValue == '\r') || (*pValue == '\n') ) pValue++;
	if ( pValue[0] == '#' ) {
		pValue++;
	} else if ( strncmp(pValue, "url(#", 5) == 0 ) {
		pValue += 5;
	}
	iLen = 0;
	while ( (pValue[iLen] != '\0') && (pValue[iLen] != ')') &&
	        (pValue[iLen] != ' ') && (pValue[iLen] != '\t') &&
	        (pValue[iLen] != '\r') && (pValue[iLen] != '\n') ) {
		iLen++;
	}
	if ( iLen <= 0 ) {
		return 0;
	}
	if ( iLen >= iOutCapacity ) {
		iLen = iOutCapacity - 1;
	}
	memcpy(sOut, pValue, (size_t)iLen);
	sOut[iLen] = '\0';
	return 1;
}

static int __xgeSvgStyleCopy(const char* sStyle, const char* sName, char* sOut, int iOutCapacity)
{
	const char* p;
	size_t iNameLen;

	if ( (sStyle == NULL) || (sName == NULL) || (sOut == NULL) || (iOutCapacity <= 0) ) {
		return 0;
	}
	iNameLen = strlen(sName);
	p = sStyle;
	while ( *p != '\0' ) {
		for ( ;; ) {
			while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') || (*p == ';') ) p++;
			if ( (p[0] == '/') && (p[1] == '*') ) {
				p += 2;
				while ( (p[0] != '\0') && !((p[0] == '*') && (p[1] == '/')) ) p++;
				if ( p[0] != '\0' ) p += 2;
				continue;
			}
			break;
		}
		if ( strncmp(p, sName, iNameLen) == 0 ) {
			const char* q = p + iNameLen;
			while ( (*q == ' ') || (*q == '\t') || (*q == '\r') || (*q == '\n') ) q++;
			if ( *q == ':' ) {
				const char* pValue;
				int iLen;

				q++;
				for ( ;; ) {
					while ( (*q == ' ') || (*q == '\t') || (*q == '\r') || (*q == '\n') ) q++;
					if ( (q[0] == '/') && (q[1] == '*') ) {
						q += 2;
						while ( (q[0] != '\0') && !((q[0] == '*') && (q[1] == '/')) ) q++;
						if ( q[0] != '\0' ) q += 2;
						continue;
					}
					break;
				}
				pValue = q;
				while ( (*q != '\0') && (*q != ';') ) q++;
				iLen = 0;
				while ( pValue < q ) {
					if ( ((pValue + 1) < q) && (pValue[0] == '/') && (pValue[1] == '*') ) {
						pValue += 2;
						while ( ((pValue + 1) < q) && !((pValue[0] == '*') && (pValue[1] == '/')) ) pValue++;
						if ( (pValue + 1) < q ) pValue += 2;
						if ( (iLen > 0) && (iLen < (iOutCapacity - 1)) && (sOut[iLen - 1] != ' ') ) {
							sOut[iLen++] = ' ';
						}
						continue;
					}
					if ( (iLen == 0) && ((*pValue == ' ') || (*pValue == '\t') || (*pValue == '\r') || (*pValue == '\n')) ) {
						pValue++;
						continue;
					}
					if ( iLen < (iOutCapacity - 1) ) {
						sOut[iLen++] = *pValue;
					}
					pValue++;
				}
				while ( (iLen > 0) && ((sOut[iLen - 1] == ' ') || (sOut[iLen - 1] == '\t') || (sOut[iLen - 1] == '\r') || (sOut[iLen - 1] == '\n')) ) {
					iLen--;
				}
				sOut[iLen] = '\0';
				return 1;
			}
		}
		while ( (*p != '\0') && (*p != ';') ) p++;
	}
	return 0;
}

static int __xgeSvgAttrOrStyleCopy(const char* pTag, const char* pTagEnd, const char* sStyle, const char* sName, char* sOut, int iOutCapacity)
{
	if ( __xgeSvgStyleCopy(sStyle, sName, sOut, iOutCapacity) ) {
		return 1;
	}
	return __xgeSvgAttrCopy(pTag, pTagEnd, sName, sOut, iOutCapacity);
}

static int __xgeSvgParseFloat(const char* sText, float* pValue)
{
	char* pEnd;
	double fValue;

	if ( (sText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	fValue = strtod(sText, &pEnd);
	if ( pEnd == sText ) {
		return 0;
	}
	*pValue = (float)fValue;
	return 1;
}

static void __xgeSvgSkipSpaces(const char** ppText);

static int __xgeSvgParseFloatOrPercent(const char* sText, float* pValue)
{
	char* pEnd;
	double fValue;

	if ( (sText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	while ( (*sText == ' ') || (*sText == '\t') || (*sText == '\r') || (*sText == '\n') ) sText++;
	fValue = strtod(sText, &pEnd);
	if ( pEnd == sText ) {
		return 0;
	}
	while ( (*pEnd == ' ') || (*pEnd == '\t') ) pEnd++;
	if ( *pEnd == '%' ) {
		fValue /= 100.0;
	}
	*pValue = (float)fValue;
	return 1;
}

static int __xgeSvgParseOpacity(const char* sText, float* pValue)
{
	const char* p;
	char* pEnd;
	double fValue;

	if ( (sText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	p = sText;
	__xgeSvgSkipSpaces(&p);
	fValue = strtod(p, &pEnd);
	if ( pEnd == p ) {
		return 0;
	}
	p = pEnd;
	__xgeSvgSkipSpaces(&p);
	if ( *p == '%' ) {
		fValue /= 100.0;
		p++;
		__xgeSvgSkipSpaces(&p);
	}
	if ( *p != '\0' ) {
		return 0;
	}
	if ( fValue < 0.0 ) fValue = 0.0;
	if ( fValue > 1.0 ) fValue = 1.0;
	*pValue = (float)fValue;
	return 1;
}

static int __xgeSvgUnitStartsWith(const char* pText, const char* sUnit)
{
	int i;

	if ( (pText == NULL) || (sUnit == NULL) ) {
		return 0;
	}
	for ( i = 0; sUnit[i] != '\0'; i++ ) {
		char a = pText[i];
		char b = sUnit[i];
		if ( (a >= 'A') && (a <= 'Z') ) a = (char)(a - 'A' + 'a');
		if ( a != b ) {
			return 0;
		}
	}
	return 1;
}

static float __xgeSvgLengthPercentRef(xge_svg pSvg, int iBasis)
{
	float fW;
	float fH;

	fW = 0.0f;
	fH = 0.0f;
	if ( __xgeSvgValid(pSvg) ) {
		if ( pSvg->bHasViewBox ) {
			fW = pSvg->tViewBox.fW;
			fH = pSvg->tViewBox.fH;
		}
		if ( fW <= 0.0f ) fW = pSvg->fWidth;
		if ( fH <= 0.0f ) fH = pSvg->fHeight;
	}
	if ( fW <= 0.0f ) fW = 1.0f;
	if ( fH <= 0.0f ) fH = 1.0f;
	if ( iBasis == XGE_SVG_LENGTH_BASIS_X ) return fW;
	if ( iBasis == XGE_SVG_LENGTH_BASIS_Y ) return fH;
	return sqrtf((fW * fW + fH * fH) * 0.5f);
}

static int __xgeSvgParseLengthToken(const char** ppText, float fPercentRef, float* pValue)
{
	const char* p;
	char* pNumberEnd;
	double fValue;
	float fScale;

	if ( (ppText == NULL) || (*ppText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	p = *ppText;
	while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') || (*p == ',') ) p++;
	fValue = strtod(p, &pNumberEnd);
	if ( pNumberEnd == p ) {
		return 0;
	}
	p = pNumberEnd;
	while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') ) p++;
	fScale = 1.0f;
	if ( *p == '%' ) {
		fValue = fValue * (double)fPercentRef / 100.0;
		p++;
	} else if ( __xgeSvgUnitStartsWith(p, "px") ) {
		p += 2;
	} else if ( __xgeSvgUnitStartsWith(p, "pt") ) {
		fScale = 96.0f / 72.0f;
		p += 2;
	} else if ( __xgeSvgUnitStartsWith(p, "pc") ) {
		fScale = 16.0f;
		p += 2;
	} else if ( __xgeSvgUnitStartsWith(p, "in") ) {
		fScale = 96.0f;
		p += 2;
	} else if ( __xgeSvgUnitStartsWith(p, "cm") ) {
		fScale = 96.0f / 2.54f;
		p += 2;
	} else if ( __xgeSvgUnitStartsWith(p, "mm") ) {
		fScale = 96.0f / 25.4f;
		p += 2;
	} else if ( __xgeSvgUnitStartsWith(p, "q") ) {
		fScale = 96.0f / 101.6f;
		p += 1;
	} else if ( __xgeSvgUnitStartsWith(p, "em") || __xgeSvgUnitStartsWith(p, "ex") ) {
		p += 2;
	} else if ( ((*p >= 'a') && (*p <= 'z')) || ((*p >= 'A') && (*p <= 'Z')) ) {
		return 0;
	}
	*pValue = (float)fValue * fScale;
	*ppText = p;
	return 1;
}

static int __xgeSvgParseLength(const char* sText, float fPercentRef, float* pValue)
{
	const char* p;

	if ( (sText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	p = sText;
	if ( !__xgeSvgParseLengthToken(&p, fPercentRef, pValue) ) {
		return 0;
	}
	while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') ) p++;
	return *p == '\0';
}

static float __xgeSvgAttrFloat(const char* pTag, const char* pTagEnd, const char* sName, float fDefault)
{
	char sValue[XGE_SVG_ATTR_MAX];
	float fValue;

	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, sName, sValue, sizeof(sValue)) ) {
		return fDefault;
	}
	return __xgeSvgParseFloat(sValue, &fValue) ? fValue : fDefault;
}

static float __xgeSvgAttrLengthRef(const char* pTag, const char* pTagEnd, const char* sName, float fPercentRef, float fDefault)
{
	char sValue[XGE_SVG_ATTR_MAX];
	float fValue;

	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, sName, sValue, sizeof(sValue)) ) {
		return fDefault;
	}
	return __xgeSvgParseLength(sValue, fPercentRef, &fValue) ? fValue : fDefault;
}

static int __xgeSvgAttrLengthCopy(const char* pTag, const char* pTagEnd, const char* sName, float fPercentRef, float* pValue)
{
	char sValue[XGE_SVG_ATTR_MAX];

	if ( (pValue == NULL) || !__xgeSvgAttrCopy(pTag, pTagEnd, sName, sValue, sizeof(sValue)) ) {
		return 0;
	}
	return __xgeSvgParseLength(sValue, fPercentRef, pValue);
}

static float __xgeSvgAttrLength(xge_svg pSvg, const char* pTag, const char* pTagEnd, const char* sName, int iBasis, float fDefault)
{
	return __xgeSvgAttrLengthRef(pTag, pTagEnd, sName, __xgeSvgLengthPercentRef(pSvg, iBasis), fDefault);
}

static float __xgeSvgAttrFloatOrPercent(const char* pTag, const char* pTagEnd, const char* sName, float fDefault)
{
	char sValue[XGE_SVG_ATTR_MAX];
	float fValue;

	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, sName, sValue, sizeof(sValue)) ) {
		return fDefault;
	}
	return __xgeSvgParseFloatOrPercent(sValue, &fValue) ? fValue : fDefault;
}

static int __xgeSvgAttrFloatOrPercentCopy(const char* pTag, const char* pTagEnd, const char* sName, float* pValue)
{
	char sValue[XGE_SVG_ATTR_MAX];

	if ( pValue == NULL ) {
		return 0;
	}
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, sName, sValue, sizeof(sValue)) ) {
		return 0;
	}
	return __xgeSvgParseFloatOrPercent(sValue, pValue);
}

static int __xgeSvgUrlIdCopyFromValue(const char* sValue, char* sOut, int iOutCapacity)
{
	const char* p;
	char cQuote;
	int iLen;

	if ( (sValue == NULL) || (sOut == NULL) || (iOutCapacity <= 0) ) {
		return 0;
	}
	sOut[0] = '\0';
	while ( (*sValue == ' ') || (*sValue == '\t') || (*sValue == '\r') || (*sValue == '\n') ) sValue++;
	if ( strncmp(sValue, "url(", 4) != 0 ) {
		return 0;
	}
	p = sValue + 4;
	while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') ) p++;
	cQuote = 0;
	if ( (*p == '"') || (*p == '\'') ) {
		cQuote = *p++;
	}
	if ( *p != '#' ) {
		return 0;
	}
	p++;
	iLen = 0;
	while ( (p[iLen] != '\0') &&
	        ((cQuote != 0) ? (p[iLen] != cQuote) : (p[iLen] != ')')) &&
	        (p[iLen] != ' ') && (p[iLen] != '\t') &&
	        (p[iLen] != '\r') && (p[iLen] != '\n') ) {
		iLen++;
	}
	if ( iLen <= 0 ) {
		return 0;
	}
	if ( iLen >= iOutCapacity ) {
		iLen = iOutCapacity - 1;
	}
	memcpy(sOut, p, (size_t)iLen);
	sOut[iLen] = '\0';
	return 1;
}

static int __xgeSvgHexNibble(char c)
{
	if ( c >= '0' && c <= '9' ) return c - '0';
	if ( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
	if ( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
	return -1;
}

static int __xgeSvgIsSpace(int c)
{
	return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n') || (c == '\f');
}

static int __xgeSvgIsHexChar(int c)
{
	return ((c >= '0') && (c <= '9')) ||
	       ((c >= 'a') && (c <= 'f')) ||
	       ((c >= 'A') && (c <= 'F'));
}

static void __xgeSvgSkipSpaces(const char** ppText)
{
	if ( ppText == NULL ) {
		return;
	}
	while ( __xgeSvgIsSpace((unsigned char)**ppText) ) {
		(*ppText)++;
	}
}

static uint32_t __xgeSvgColorAlpha(uint32_t iColor, float fAlpha)
{
	int iA;

	if ( fAlpha < 0.0f ) fAlpha = 0.0f;
	if ( fAlpha > 1.0f ) fAlpha = 1.0f;
	iA = (int)((float)XGE_COLOR_GET_A(iColor) * fAlpha + 0.5f);
	if ( iA < 0 ) iA = 0;
	if ( iA > 255 ) iA = 255;
	return XGE_COLOR_RGBA(XGE_COLOR_GET_R(iColor), XGE_COLOR_GET_G(iColor), XGE_COLOR_GET_B(iColor), iA);
}

typedef struct xge_svg_color_name_t {
	const char* sName;
	unsigned char r;
	unsigned char g;
	unsigned char b;
} xge_svg_color_name_t;

static int __xgeSvgAsciiLower(int c)
{
	if ( (c >= 'A') && (c <= 'Z') ) {
		return c + ('a' - 'A');
	}
	return c;
}

static int __xgeSvgColorNameEquals(const char* sText, const char* sName)
{
	const char* pEnd;

	if ( (sText == NULL) || (sName == NULL) ) {
		return 0;
	}
	while ( (*sText == ' ') || (*sText == '\t') || (*sText == '\r') || (*sText == '\n') ) {
		sText++;
	}
	pEnd = sText + strlen(sText);
	while ( (pEnd > sText) && ((pEnd[-1] == ' ') || (pEnd[-1] == '\t') || (pEnd[-1] == '\r') || (pEnd[-1] == '\n')) ) {
		pEnd--;
	}
	while ( (sText < pEnd) && (*sName != '\0') ) {
		if ( __xgeSvgAsciiLower((unsigned char)*sText) != __xgeSvgAsciiLower((unsigned char)*sName) ) {
			return 0;
		}
		sText++;
		sName++;
	}
	return (sText == pEnd) && (*sName == '\0');
}

static int __xgeSvgParseNamedColor(const char* sText, uint32_t* pColor)
{
	static const xge_svg_color_name_t sColors[] = {
		{"aliceblue", 240, 248, 255}, {"antiquewhite", 250, 235, 215}, {"aqua", 0, 255, 255}, {"aquamarine", 127, 255, 212},
		{"azure", 240, 255, 255}, {"beige", 245, 245, 220}, {"bisque", 255, 228, 196}, {"black", 0, 0, 0},
		{"blanchedalmond", 255, 235, 205}, {"blue", 0, 0, 255}, {"blueviolet", 138, 43, 226}, {"brown", 165, 42, 42},
		{"burlywood", 222, 184, 135}, {"cadetblue", 95, 158, 160}, {"chartreuse", 127, 255, 0}, {"chocolate", 210, 105, 30},
		{"coral", 255, 127, 80}, {"cornflowerblue", 100, 149, 237}, {"cornsilk", 255, 248, 220}, {"crimson", 220, 20, 60},
		{"cyan", 0, 255, 255}, {"darkblue", 0, 0, 139}, {"darkcyan", 0, 139, 139}, {"darkgoldenrod", 184, 134, 11},
		{"darkgray", 169, 169, 169}, {"darkgreen", 0, 100, 0}, {"darkgrey", 169, 169, 169}, {"darkkhaki", 189, 183, 107},
		{"darkmagenta", 139, 0, 139}, {"darkolivegreen", 85, 107, 47}, {"darkorange", 255, 140, 0}, {"darkorchid", 153, 50, 204},
		{"darkred", 139, 0, 0}, {"darksalmon", 233, 150, 122}, {"darkseagreen", 143, 188, 143}, {"darkslateblue", 72, 61, 139},
		{"darkslategray", 47, 79, 79}, {"darkslategrey", 47, 79, 79}, {"darkturquoise", 0, 206, 209}, {"darkviolet", 148, 0, 211},
		{"deeppink", 255, 20, 147}, {"deepskyblue", 0, 191, 255}, {"dimgray", 105, 105, 105}, {"dimgrey", 105, 105, 105},
		{"dodgerblue", 30, 144, 255}, {"firebrick", 178, 34, 34}, {"floralwhite", 255, 250, 240}, {"forestgreen", 34, 139, 34},
		{"fuchsia", 255, 0, 255}, {"gainsboro", 220, 220, 220}, {"ghostwhite", 248, 248, 255}, {"gold", 255, 215, 0},
		{"goldenrod", 218, 165, 32}, {"gray", 128, 128, 128}, {"green", 0, 128, 0}, {"greenyellow", 173, 255, 47},
		{"grey", 128, 128, 128}, {"honeydew", 240, 255, 240}, {"hotpink", 255, 105, 180}, {"indianred", 205, 92, 92},
		{"indigo", 75, 0, 130}, {"ivory", 255, 255, 240}, {"khaki", 240, 230, 140}, {"lavender", 230, 230, 250},
		{"lavenderblush", 255, 240, 245}, {"lawngreen", 124, 252, 0}, {"lemonchiffon", 255, 250, 205}, {"lightblue", 173, 216, 230},
		{"lightcoral", 240, 128, 128}, {"lightcyan", 224, 255, 255}, {"lightgoldenrodyellow", 250, 250, 210}, {"lightgray", 211, 211, 211},
		{"lightgreen", 144, 238, 144}, {"lightgrey", 211, 211, 211}, {"lightpink", 255, 182, 193}, {"lightsalmon", 255, 160, 122},
		{"lightseagreen", 32, 178, 170}, {"lightskyblue", 135, 206, 250}, {"lightslategray", 119, 136, 153}, {"lightslategrey", 119, 136, 153},
		{"lightsteelblue", 176, 196, 222}, {"lightyellow", 255, 255, 224}, {"lime", 0, 255, 0}, {"limegreen", 50, 205, 50},
		{"linen", 250, 240, 230}, {"magenta", 255, 0, 255}, {"maroon", 128, 0, 0}, {"mediumaquamarine", 102, 205, 170},
		{"mediumblue", 0, 0, 205}, {"mediumorchid", 186, 85, 211}, {"mediumpurple", 147, 112, 219}, {"mediumseagreen", 60, 179, 113},
		{"mediumslateblue", 123, 104, 238}, {"mediumspringgreen", 0, 250, 154}, {"mediumturquoise", 72, 209, 204}, {"mediumvioletred", 199, 21, 133},
		{"midnightblue", 25, 25, 112}, {"mintcream", 245, 255, 250}, {"mistyrose", 255, 228, 225}, {"moccasin", 255, 228, 181},
		{"navajowhite", 255, 222, 173}, {"navy", 0, 0, 128}, {"oldlace", 253, 245, 230}, {"olive", 128, 128, 0},
		{"olivedrab", 107, 142, 35}, {"orange", 255, 165, 0}, {"orangered", 255, 69, 0}, {"orchid", 218, 112, 214},
		{"palegoldenrod", 238, 232, 170}, {"palegreen", 152, 251, 152}, {"paleturquoise", 175, 238, 238}, {"palevioletred", 219, 112, 147},
		{"papayawhip", 255, 239, 213}, {"peachpuff", 255, 218, 185}, {"peru", 205, 133, 63}, {"pink", 255, 192, 203},
		{"plum", 221, 160, 221}, {"powderblue", 176, 224, 230}, {"purple", 128, 0, 128}, {"red", 255, 0, 0},
		{"rosybrown", 188, 143, 143}, {"royalblue", 65, 105, 225}, {"saddlebrown", 139, 69, 19}, {"salmon", 250, 128, 114},
		{"sandybrown", 244, 164, 96}, {"seagreen", 46, 139, 87}, {"seashell", 255, 245, 238}, {"sienna", 160, 82, 45},
		{"silver", 192, 192, 192}, {"skyblue", 135, 206, 235}, {"slateblue", 106, 90, 205}, {"slategray", 112, 128, 144},
		{"slategrey", 112, 128, 144}, {"snow", 255, 250, 250}, {"springgreen", 0, 255, 127}, {"steelblue", 70, 130, 180},
		{"tan", 210, 180, 140}, {"teal", 0, 128, 128}, {"thistle", 216, 191, 216}, {"tomato", 255, 99, 71},
		{"turquoise", 64, 224, 208}, {"violet", 238, 130, 238}, {"wheat", 245, 222, 179}, {"white", 255, 255, 255},
		{"whitesmoke", 245, 245, 245}, {"yellow", 255, 255, 0}, {"yellowgreen", 154, 205, 50}
	};
	int i;

	if ( (sText == NULL) || (pColor == NULL) ) {
		return 0;
	}
	if ( __xgeSvgColorNameEquals(sText, "transparent") ) {
		*pColor = XGE_COLOR_RGBA(0, 0, 0, 0);
		return 1;
	}
	for ( i = 0; i < (int)(sizeof(sColors) / sizeof(sColors[0])); i++ ) {
		if ( __xgeSvgColorNameEquals(sText, sColors[i].sName) ) {
			*pColor = XGE_COLOR_RGBA(sColors[i].r, sColors[i].g, sColors[i].b, 255);
			return 1;
		}
	}
	return 0;
}

static int __xgeSvgColorByteFromDouble(double fValue, int bPercent, int bAlpha)
{
	double fByte;
	int iByte;

	if ( fValue != fValue ) {
		return -1;
	}
	if ( bAlpha ) {
		if ( bPercent ) {
			fByte = fValue * 255.0 / 100.0;
		} else if ( fValue <= 1.0 ) {
			fByte = fValue * 255.0;
		} else {
			fByte = fValue;
		}
	} else {
		if ( bPercent ) {
			fByte = fValue * 255.0 / 100.0;
		} else {
			fByte = fValue;
		}
	}
	if ( fByte < 0.0 ) fByte = 0.0;
	if ( fByte > 255.0 ) fByte = 255.0;
	iByte = (int)(fByte + 0.5);
	if ( iByte < 0 ) iByte = 0;
	if ( iByte > 255 ) iByte = 255;
	return iByte;
}

static int __xgeSvgParseColorComponent(const char** ppText, int bAlpha, int* pValue)
{
	const char* p;
	char* pEnd;
	double fValue;
	int bPercent;
	int iValue;

	if ( (ppText == NULL) || (*ppText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	p = *ppText;
	__xgeSvgSkipSpaces(&p);
	fValue = strtod(p, &pEnd);
	if ( pEnd == p ) {
		return 0;
	}
	p = pEnd;
	__xgeSvgSkipSpaces(&p);
	bPercent = 0;
	if ( *p == '%' ) {
		bPercent = 1;
		p++;
	}
	iValue = __xgeSvgColorByteFromDouble(fValue, bPercent, bAlpha);
	if ( iValue < 0 ) {
		return 0;
	}
	*pValue = iValue;
	*ppText = p;
	return 1;
}

static void __xgeSvgSkipColorSeparator(const char** ppText)
{
	const char* p;

	if ( (ppText == NULL) || (*ppText == NULL) ) {
		return;
	}
	p = *ppText;
	__xgeSvgSkipSpaces(&p);
	if ( *p == ',' ) {
		p++;
	}
	__xgeSvgSkipSpaces(&p);
	*ppText = p;
}

static int __xgeSvgColorFunctionArgs(const char* sText, const char* sName, const char** ppArgs)
{
	const char* p;

	if ( (sText == NULL) || (sName == NULL) || (ppArgs == NULL) ) {
		return 0;
	}
	p = sText;
	__xgeSvgSkipSpaces(&p);
	while ( *sName != '\0' ) {
		if ( __xgeSvgAsciiLower((unsigned char)*p) != __xgeSvgAsciiLower((unsigned char)*sName) ) {
			return 0;
		}
		p++;
		sName++;
	}
	__xgeSvgSkipSpaces(&p);
	if ( *p != '(' ) {
		return 0;
	}
	*ppArgs = p + 1;
	return 1;
}

static int __xgeSvgParseRgbColor(const char* sText, uint32_t* pColor)
{
	const char* p;
	int r;
	int g;
	int b;
	int a;

	if ( !__xgeSvgColorFunctionArgs(sText, "rgb", &p) &&
	     !__xgeSvgColorFunctionArgs(sText, "rgba", &p) ) {
		return 0;
	}
	if ( !__xgeSvgParseColorComponent(&p, 0, &r) ) return 0;
	__xgeSvgSkipColorSeparator(&p);
	if ( !__xgeSvgParseColorComponent(&p, 0, &g) ) return 0;
	__xgeSvgSkipColorSeparator(&p);
	if ( !__xgeSvgParseColorComponent(&p, 0, &b) ) return 0;
	a = 255;
	__xgeSvgSkipSpaces(&p);
	if ( *p == '/' ) {
		p++;
		if ( !__xgeSvgParseColorComponent(&p, 1, &a) ) return 0;
	} else if ( *p == ',' ) {
		p++;
		if ( !__xgeSvgParseColorComponent(&p, 1, &a) ) return 0;
	} else if ( *p != ')' ) {
		if ( !__xgeSvgParseColorComponent(&p, 1, &a) ) return 0;
	}
	__xgeSvgSkipSpaces(&p);
	if ( *p != ')' ) {
		return 0;
	}
	p++;
	__xgeSvgSkipSpaces(&p);
	if ( *p != '\0' ) {
		return 0;
	}
	*pColor = XGE_COLOR_RGBA(r, g, b, a);
	return 1;
}

static float __xgeSvgHueToRgb(float p, float q, float t)
{
	if ( t < 0.0f ) t += 1.0f;
	if ( t > 1.0f ) t -= 1.0f;
	if ( t < (1.0f / 6.0f) ) return p + (q - p) * 6.0f * t;
	if ( t < 0.5f ) return q;
	if ( t < (2.0f / 3.0f) ) return p + (q - p) * ((2.0f / 3.0f) - t) * 6.0f;
	return p;
}

static int __xgeSvgParseHueComponent(const char** ppText, float* pHue)
{
	const char* p;
	char* pEnd;
	double fValue;

	if ( (ppText == NULL) || (*ppText == NULL) || (pHue == NULL) ) {
		return 0;
	}
	p = *ppText;
	__xgeSvgSkipSpaces(&p);
	fValue = strtod(p, &pEnd);
	if ( pEnd == p ) {
		return 0;
	}
	p = pEnd;
	__xgeSvgSkipSpaces(&p);
	if ( (p[0] == 't') && (p[1] == 'u') && (p[2] == 'r') && (p[3] == 'n') ) {
		fValue *= 360.0;
		p += 4;
	} else if ( (p[0] == 'r') && (p[1] == 'a') && (p[2] == 'd') ) {
		fValue *= 57.2957795130823208768;
		p += 3;
	} else if ( (p[0] == 'g') && (p[1] == 'r') && (p[2] == 'a') && (p[3] == 'd') ) {
		fValue *= 0.9;
		p += 4;
	} else if ( (p[0] == 'd') && (p[1] == 'e') && (p[2] == 'g') ) {
		p += 3;
	}
	if ( fValue != fValue ) {
		return 0;
	}
	while ( fValue < 0.0 ) fValue += 360.0;
	while ( fValue >= 360.0 ) fValue -= 360.0;
	*pHue = (float)(fValue / 360.0);
	*ppText = p;
	return 1;
}

static int __xgeSvgParseHslPercentComponent(const char** ppText, float* pValue)
{
	const char* p;
	char* pEnd;
	double fValue;
	int bPercent;

	if ( (ppText == NULL) || (*ppText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	p = *ppText;
	__xgeSvgSkipSpaces(&p);
	fValue = strtod(p, &pEnd);
	if ( pEnd == p ) {
		return 0;
	}
	p = pEnd;
	__xgeSvgSkipSpaces(&p);
	bPercent = 0;
	if ( *p == '%' ) {
		bPercent = 1;
		p++;
	}
	if ( fValue != fValue ) {
		return 0;
	}
	if ( bPercent || (fValue > 1.0) || (fValue < -1.0) ) {
		fValue /= 100.0;
	}
	if ( fValue < 0.0 ) fValue = 0.0;
	if ( fValue > 1.0 ) fValue = 1.0;
	*pValue = (float)fValue;
	*ppText = p;
	return 1;
}

static int __xgeSvgParseHslColor(const char* sText, uint32_t* pColor)
{
	const char* p;
	float h;
	float s;
	float l;
	float r;
	float g;
	float b;
	float q;
	float v;
	int a;

	if ( !__xgeSvgColorFunctionArgs(sText, "hsl", &p) &&
	     !__xgeSvgColorFunctionArgs(sText, "hsla", &p) ) {
		return 0;
	}
	if ( !__xgeSvgParseHueComponent(&p, &h) ) return 0;
	__xgeSvgSkipColorSeparator(&p);
	if ( !__xgeSvgParseHslPercentComponent(&p, &s) ) return 0;
	__xgeSvgSkipColorSeparator(&p);
	if ( !__xgeSvgParseHslPercentComponent(&p, &l) ) return 0;
	a = 255;
	__xgeSvgSkipSpaces(&p);
	if ( *p == '/' ) {
		p++;
		if ( !__xgeSvgParseColorComponent(&p, 1, &a) ) return 0;
	} else if ( *p == ',' ) {
		p++;
		if ( !__xgeSvgParseColorComponent(&p, 1, &a) ) return 0;
	} else if ( *p != ')' ) {
		if ( !__xgeSvgParseColorComponent(&p, 1, &a) ) return 0;
	}
	__xgeSvgSkipSpaces(&p);
	if ( *p != ')' ) {
		return 0;
	}
	p++;
	__xgeSvgSkipSpaces(&p);
	if ( *p != '\0' ) {
		return 0;
	}
	if ( s <= 0.0f ) {
		r = l;
		g = l;
		b = l;
	} else {
		q = (l < 0.5f) ? (l * (1.0f + s)) : (l + s - l * s);
		v = 2.0f * l - q;
		r = __xgeSvgHueToRgb(v, q, h + (1.0f / 3.0f));
		g = __xgeSvgHueToRgb(v, q, h);
		b = __xgeSvgHueToRgb(v, q, h - (1.0f / 3.0f));
	}
	*pColor = XGE_COLOR_RGBA(
		__xgeSvgColorByteFromDouble((double)r * 255.0, 0, 0),
		__xgeSvgColorByteFromDouble((double)g * 255.0, 0, 0),
		__xgeSvgColorByteFromDouble((double)b * 255.0, 0, 0),
		a);
	return 1;
}

static int __xgeSvgParseHexColor(const char* sText, uint32_t* pColor)
{
	int iNibbles[8];
	const char* p;
	int iCount;
	int r;
	int g;
	int b;
	int a;

	if ( (sText == NULL) || (pColor == NULL) || (sText[0] != '#') ) {
		return 0;
	}
	p = sText + 1;
	iCount = 0;
	while ( __xgeSvgIsHexChar((unsigned char)*p) && (iCount < 8) ) {
		iNibbles[iCount++] = __xgeSvgHexNibble(*p);
		p++;
	}
	if ( __xgeSvgIsHexChar((unsigned char)*p) ) {
		return 0;
	}
	__xgeSvgSkipSpaces(&p);
	if ( *p != '\0' ) {
		return 0;
	}
	if ( iCount == 3 || iCount == 4 ) {
		r = (iNibbles[0] << 4) | iNibbles[0];
		g = (iNibbles[1] << 4) | iNibbles[1];
		b = (iNibbles[2] << 4) | iNibbles[2];
		a = (iCount == 4) ? ((iNibbles[3] << 4) | iNibbles[3]) : 255;
		*pColor = XGE_COLOR_RGBA(r, g, b, a);
		return 1;
	}
	if ( iCount == 6 || iCount == 8 ) {
		r = (iNibbles[0] << 4) | iNibbles[1];
		g = (iNibbles[2] << 4) | iNibbles[3];
		b = (iNibbles[4] << 4) | iNibbles[5];
		a = (iCount == 8) ? ((iNibbles[6] << 4) | iNibbles[7]) : 255;
		*pColor = XGE_COLOR_RGBA(r, g, b, a);
		return 1;
	}
	return 0;
}

static int __xgeSvgParseColor(const char* sText, uint32_t* pColor)
{
	if ( (sText == NULL) || (pColor == NULL) ) {
		return 0;
	}
	__xgeSvgSkipSpaces(&sText);
	if ( __xgeSvgColorNameEquals(sText, "none") ) {
		*pColor = XGE_COLOR_RGBA(0, 0, 0, 0);
		return 1;
	}
	if ( sText[0] == '#' ) {
		return __xgeSvgParseHexColor(sText, pColor);
	}
	if ( __xgeSvgParseRgbColor(sText, pColor) ) {
		return 1;
	}
	if ( __xgeSvgParseHslColor(sText, pColor) ) {
		return 1;
	}
	return __xgeSvgParseNamedColor(sText, pColor);
}

static int __xgeSvgParseFourFloats(const char* sText, float* pA, float* pB, float* pC, float* pD)
{
	const char* p;
	char* pEnd;

	if ( (sText == NULL) || (pA == NULL) || (pB == NULL) || (pC == NULL) || (pD == NULL) ) {
		return 0;
	}
	p = sText;
	*pA = (float)strtod(p, &pEnd); if ( pEnd == p ) return 0; p = pEnd;
	*pB = (float)strtod(p, &pEnd); if ( pEnd == p ) return 0; p = pEnd;
	*pC = (float)strtod(p, &pEnd); if ( pEnd == p ) return 0; p = pEnd;
	*pD = (float)strtod(p, &pEnd); if ( pEnd == p ) return 0;
	return 1;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixTranslate(float fX, float fY)
{
	xge_shape_ex_matrix_t tMatrix = __xgeShapeExMatrixIdentity();
	tMatrix.fE = fX;
	tMatrix.fF = fY;
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixScale(float fX, float fY)
{
	xge_shape_ex_matrix_t tMatrix = __xgeShapeExMatrixIdentity();
	tMatrix.fA = fX;
	tMatrix.fD = fY;
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixRotate(float fDegrees)
{
	xge_shape_ex_matrix_t tMatrix = __xgeShapeExMatrixIdentity();
	float fRadians = fDegrees * (3.14159265358979323846f / 180.0f);
	float fCos = cosf(fRadians);
	float fSin = sinf(fRadians);

	tMatrix.fA = fCos;
	tMatrix.fB = fSin;
	tMatrix.fC = -fSin;
	tMatrix.fD = fCos;
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixSkewX(float fDegrees)
{
	xge_shape_ex_matrix_t tMatrix = __xgeShapeExMatrixIdentity();
	float fRadians = fDegrees * (3.14159265358979323846f / 180.0f);

	tMatrix.fC = tanf(fRadians);
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixSkewY(float fDegrees)
{
	xge_shape_ex_matrix_t tMatrix = __xgeShapeExMatrixIdentity();
	float fRadians = fDegrees * (3.14159265358979323846f / 180.0f);

	tMatrix.fB = tanf(fRadians);
	return tMatrix;
}

static xge_rect_t __xgeSvgMatrixRectBounds(xge_shape_ex_matrix_t tMatrix, xge_rect_t tRect)
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
		xge_vec2_t tPoint = tP[i];

		tP[i].fX = (tMatrix.fA * tPoint.fX) + (tMatrix.fC * tPoint.fY) + tMatrix.fE;
		tP[i].fY = (tMatrix.fB * tPoint.fX) + (tMatrix.fD * tPoint.fY) + tMatrix.fF;
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

static void __xgeSvgParsePreserveAspectRatioFields(const char* sValue, int* pAlignX, int* pAlignY, int* pMeetOrSlice)
{
	if ( pAlignX != NULL ) *pAlignX = XGE_SVG_ASPECT_ALIGN_MID;
	if ( pAlignY != NULL ) *pAlignY = XGE_SVG_ASPECT_ALIGN_MID;
	if ( pMeetOrSlice != NULL ) *pMeetOrSlice = XGE_SVG_ASPECT_MEET;
	if ( (sValue == NULL) || (sValue[0] == '\0') ) {
		return;
	}
	if ( strstr(sValue, "none") != NULL ) {
		if ( pMeetOrSlice != NULL ) *pMeetOrSlice = XGE_SVG_ASPECT_NONE;
		return;
	}
	if ( pAlignX != NULL ) {
		if ( strstr(sValue, "xMin") != NULL ) *pAlignX = XGE_SVG_ASPECT_ALIGN_MIN;
		if ( strstr(sValue, "xMid") != NULL ) *pAlignX = XGE_SVG_ASPECT_ALIGN_MID;
		if ( strstr(sValue, "xMax") != NULL ) *pAlignX = XGE_SVG_ASPECT_ALIGN_MAX;
	}
	if ( pAlignY != NULL ) {
		if ( strstr(sValue, "YMin") != NULL ) *pAlignY = XGE_SVG_ASPECT_ALIGN_MIN;
		if ( strstr(sValue, "YMid") != NULL ) *pAlignY = XGE_SVG_ASPECT_ALIGN_MID;
		if ( strstr(sValue, "YMax") != NULL ) *pAlignY = XGE_SVG_ASPECT_ALIGN_MAX;
	}
	if ( (pMeetOrSlice != NULL) && (strstr(sValue, "slice") != NULL) ) {
		*pMeetOrSlice = XGE_SVG_ASPECT_SLICE;
	}
}

static xge_rect_t __xgeSvgAspectViewport(xge_rect_t tViewBox, xge_rect_t tDst, int iAlignX, int iAlignY, int iMeetOrSlice)
{
	float fScaleX;
	float fScaleY;
	float fScale;
	xge_rect_t tOut;

	if ( (tViewBox.fW <= 0.0f) || (tViewBox.fH <= 0.0f) || (iMeetOrSlice == XGE_SVG_ASPECT_NONE) ) {
		return tDst;
	}
	fScaleX = tDst.fW / tViewBox.fW;
	fScaleY = tDst.fH / tViewBox.fH;
	fScale = (iMeetOrSlice == XGE_SVG_ASPECT_SLICE) ? fmaxf(fScaleX, fScaleY) : fminf(fScaleX, fScaleY);
	tOut.fW = tViewBox.fW * fScale;
	tOut.fH = tViewBox.fH * fScale;
	tOut.fX = tDst.fX;
	tOut.fY = tDst.fY;
	if ( iAlignX == XGE_SVG_ASPECT_ALIGN_MID ) tOut.fX += (tDst.fW - tOut.fW) * 0.5f;
	else if ( iAlignX == XGE_SVG_ASPECT_ALIGN_MAX ) tOut.fX += (tDst.fW - tOut.fW);
	if ( iAlignY == XGE_SVG_ASPECT_ALIGN_MID ) tOut.fY += (tDst.fH - tOut.fH) * 0.5f;
	else if ( iAlignY == XGE_SVG_ASPECT_ALIGN_MAX ) tOut.fY += (tDst.fH - tOut.fH);
	return tOut;
}

static xge_shape_ex_matrix_t __xgeSvgViewBoxMatrix(xge_rect_t tViewBox, xge_rect_t tViewport)
{
	xge_shape_ex_matrix_t tMatrix = __xgeShapeExMatrixIdentity();
	float fScaleX;
	float fScaleY;

	if ( (tViewBox.fW <= 0.0f) || (tViewBox.fH <= 0.0f) ) {
		return tMatrix;
	}
	fScaleX = tViewport.fW / tViewBox.fW;
	fScaleY = tViewport.fH / tViewBox.fH;
	tMatrix.fA = fScaleX;
	tMatrix.fD = fScaleY;
	tMatrix.fE = tViewport.fX - tViewBox.fX * fScaleX;
	tMatrix.fF = tViewport.fY - tViewBox.fY * fScaleY;
	return tMatrix;
}

static int __xgeSvgParseTransform(const char* sText, xge_shape_ex_matrix_t* pMatrix)
{
	xge_shape_ex_matrix_t tOut;
	const char* p;

	if ( (sText == NULL) || (pMatrix == NULL) ) {
		return 0;
	}
	tOut = __xgeShapeExMatrixIdentity();
	p = sText;
	while ( *p != '\0' ) {
		while ( (*p == ' ') || (*p == '\t') || (*p == ',') ) p++;
		if ( strncmp(p, "matrix", 6) == 0 ) {
			float a, b, c, d, e, f;
			xge_shape_ex_matrix_t tM;
			p = strchr(p, '(');
			if ( p == NULL ) return 0;
			p++;
			if ( sscanf(p, "%f%*[, ]%f%*[, ]%f%*[, ]%f%*[, ]%f%*[, ]%f", &a, &b, &c, &d, &e, &f) != 6 ) return 0;
			tM.fA = a; tM.fB = b; tM.fC = c; tM.fD = d; tM.fE = e; tM.fF = f;
			tOut = __xgeShapeExMatrixMul(tOut, tM);
		} else if ( strncmp(p, "translate", 9) == 0 ) {
			float x = 0.0f, y = 0.0f;
			p = strchr(p, '(');
			if ( p == NULL ) return 0;
			p++;
			sscanf(p, "%f%*[, ]%f", &x, &y);
			tOut = __xgeShapeExMatrixMul(tOut, __xgeSvgMatrixTranslate(x, y));
		} else if ( strncmp(p, "scale", 5) == 0 ) {
			float x = 1.0f, y = 1.0f;
			int n;
			p = strchr(p, '(');
			if ( p == NULL ) return 0;
			p++;
			n = sscanf(p, "%f%*[, ]%f", &x, &y);
			if ( n == 1 ) y = x;
			tOut = __xgeShapeExMatrixMul(tOut, __xgeSvgMatrixScale(x, y));
		} else if ( strncmp(p, "rotate", 6) == 0 ) {
			float a = 0.0f, cx = 0.0f, cy = 0.0f;
			int n;
			p = strchr(p, '(');
			if ( p == NULL ) return 0;
			p++;
			n = sscanf(p, "%f%*[, ]%f%*[, ]%f", &a, &cx, &cy);
			if ( n == 3 ) {
				tOut = __xgeShapeExMatrixMul(tOut, __xgeSvgMatrixTranslate(cx, cy));
				tOut = __xgeShapeExMatrixMul(tOut, __xgeSvgMatrixRotate(a));
				tOut = __xgeShapeExMatrixMul(tOut, __xgeSvgMatrixTranslate(-cx, -cy));
			} else {
				tOut = __xgeShapeExMatrixMul(tOut, __xgeSvgMatrixRotate(a));
			}
		} else if ( strncmp(p, "skewX", 5) == 0 ) {
			float a = 0.0f;
			p = strchr(p, '(');
			if ( p == NULL ) return 0;
			p++;
			if ( sscanf(p, "%f", &a) != 1 ) return 0;
			tOut = __xgeShapeExMatrixMul(tOut, __xgeSvgMatrixSkewX(a));
		} else if ( strncmp(p, "skewY", 5) == 0 ) {
			float a = 0.0f;
			p = strchr(p, '(');
			if ( p == NULL ) return 0;
			p++;
			if ( sscanf(p, "%f", &a) != 1 ) return 0;
			tOut = __xgeShapeExMatrixMul(tOut, __xgeSvgMatrixSkewY(a));
		} else {
			break;
		}
		p = strchr(p, ')');
		if ( p == NULL ) break;
		p++;
	}
	*pMatrix = tOut;
	return 1;
}

static int __xgeSvgParseGradientSpread(const char* sText)
{
	if ( sText == NULL ) {
		return XGE_SHAPE_EX_GRADIENT_SPREAD_PAD;
	}
	if ( strcmp(sText, "reflect") == 0 ) {
		return XGE_SHAPE_EX_GRADIENT_SPREAD_REFLECT;
	}
	if ( strcmp(sText, "repeat") == 0 ) {
		return XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT;
	}
	return XGE_SHAPE_EX_GRADIENT_SPREAD_PAD;
}

static int __xgeSvgParseDashArray(const char* sText, float fPercentRef, float* pPattern, int* pCount)
{
	const char* p;
	int iCount;

	if ( (sText == NULL) || (pPattern == NULL) || (pCount == NULL) ) {
		return 0;
	}
	while ( (*sText == ' ') || (*sText == '\t') || (*sText == '\r') || (*sText == '\n') ) sText++;
	if ( strcmp(sText, "none") == 0 ) {
		*pCount = 0;
		return 1;
	}
	p = sText;
	iCount = 0;
	while ( *p != '\0' ) {
		float fValue;

		while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') || (*p == ',') ) p++;
		if ( *p == '\0' ) {
			break;
		}
		if ( !__xgeSvgParseLengthToken(&p, fPercentRef, &fValue) ) {
			return 0;
		}
		if ( (fValue > 0.0f) && (iCount < XGE_SVG_DASH_MAX) ) {
			pPattern[iCount++] = fValue;
		}
	}
	*pCount = iCount;
	return 1;
}

static xge_svg_style_t __xgeSvgStyleDefault(void)
{
	xge_svg_style_t tStyle;

	memset(&tStyle, 0, sizeof(tStyle));
	tStyle.iCurrentColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	tStyle.iFillColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	tStyle.iStrokeColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	tStyle.iStopColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	tStyle.fOpacity = 1.0f;
	tStyle.fFillOpacity = 1.0f;
	tStyle.fStrokeOpacity = 1.0f;
	tStyle.fStopOpacity = 1.0f;
	tStyle.fStrokeWidth = 1.0f;
	tStyle.fMiterLimit = 4.0f;
	tStyle.iFillRule = XGE_SHAPE_EX_FILL_NON_ZERO;
	tStyle.iLineCap = XGE_SHAPE_EX_CAP_BUTT;
	tStyle.iLineJoin = XGE_SHAPE_EX_JOIN_MITER;
	tStyle.bVisible = 1;
	tStyle.bVisibility = 1;
	tStyle.tTransform = __xgeShapeExMatrixIdentity();
	return tStyle;
}

static void __xgeSvgStyleResetStopPaint(xge_svg_style_t* pStyle)
{
	if ( pStyle == NULL ) {
		return;
	}
	pStyle->iStopColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	pStyle->fStopOpacity = 1.0f;
	pStyle->bStopColorSet = 0;
	pStyle->bStopOpacitySet = 0;
}

static int __xgeSvgStylePropertyCopy(const char* pTag, const char* pTagEnd, const char* sStyle, const char* sName, char* sOut, int iOutCapacity, int bAllowAttrs)
{
	if ( __xgeSvgStyleCopy(sStyle, sName, sOut, iOutCapacity) ) {
		return 1;
	}
	if ( bAllowAttrs && (pTag != NULL) && (pTagEnd != NULL) ) {
		return __xgeSvgAttrCopy(pTag, pTagEnd, sName, sOut, iOutCapacity);
	}
	if ( (sOut != NULL) && (iOutCapacity > 0) ) {
		sOut[0] = '\0';
	}
	return 0;
}

static void __xgeSvgStyleApplyProperties(xge_svg pSvg, xge_svg_style_t* pStyle, const char* pTag, const char* pTagEnd, const char* sStyle, int bAllowAttrs, int bAllowTransform)
{
	char sValue[XGE_SVG_ATTR_MAX];
	float fValue;
	float fLengthRef;

	if ( pStyle == NULL ) {
		return;
	}
	fLengthRef = __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_OTHER);
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "color", sValue, sizeof(sValue), bAllowAttrs) &&
	     !__xgeSvgColorNameEquals(sValue, "currentColor") ) {
		__xgeSvgParseColor(sValue, &pStyle->iCurrentColor);
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "display", sValue, sizeof(sValue), bAllowAttrs) && (strcmp(sValue, "none") == 0) ) {
		pStyle->bVisible = 0;
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "visibility", sValue, sizeof(sValue), bAllowAttrs) ) {
		if ( strcmp(sValue, "visible") == 0 ) {
			pStyle->bVisibility = 1;
		} else if ( (strcmp(sValue, "hidden") == 0) || (strcmp(sValue, "collapse") == 0) ) {
			pStyle->bVisibility = 0;
		}
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "stop-color", sValue, sizeof(sValue), bAllowAttrs) ) {
		if ( __xgeSvgColorNameEquals(sValue, "currentColor") ) {
			pStyle->iStopColor = pStyle->iCurrentColor;
			pStyle->bStopColorSet = 1;
		} else if ( __xgeSvgParseColor(sValue, &pStyle->iStopColor) ) {
			pStyle->bStopColorSet = 1;
		}
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "stop-opacity", sValue, sizeof(sValue), bAllowAttrs) && __xgeSvgParseOpacity(sValue, &fValue) ) {
		pStyle->fStopOpacity = fValue;
		pStyle->bStopOpacitySet = 1;
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "fill", sValue, sizeof(sValue), bAllowAttrs) ) {
		if ( __xgeSvgUrlIdCopyFromValue(sValue, pStyle->sFillGradientId, sizeof(pStyle->sFillGradientId)) ) {
			pStyle->iFillColor = XGE_COLOR_RGBA(0, 0, 0, 255);
		} else if ( __xgeSvgColorNameEquals(sValue, "currentColor") ) {
			pStyle->sFillGradientId[0] = '\0';
			pStyle->iFillColor = pStyle->iCurrentColor;
		} else {
			pStyle->sFillGradientId[0] = '\0';
			__xgeSvgParseColor(sValue, &pStyle->iFillColor);
		}
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "stroke", sValue, sizeof(sValue), bAllowAttrs) ) {
		if ( __xgeSvgUrlIdCopyFromValue(sValue, pStyle->sStrokeGradientId, sizeof(pStyle->sStrokeGradientId)) ) {
			pStyle->iStrokeColor = XGE_COLOR_RGBA(0, 0, 0, 255);
		} else if ( __xgeSvgColorNameEquals(sValue, "currentColor") ) {
			pStyle->sStrokeGradientId[0] = '\0';
			pStyle->iStrokeColor = pStyle->iCurrentColor;
		} else {
			pStyle->sStrokeGradientId[0] = '\0';
			__xgeSvgParseColor(sValue, &pStyle->iStrokeColor);
		}
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "opacity", sValue, sizeof(sValue), bAllowAttrs) && __xgeSvgParseOpacity(sValue, &fValue) ) {
		pStyle->fOpacity *= fValue;
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "fill-opacity", sValue, sizeof(sValue), bAllowAttrs) && __xgeSvgParseOpacity(sValue, &fValue) ) {
		pStyle->fFillOpacity = fValue;
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "stroke-opacity", sValue, sizeof(sValue), bAllowAttrs) && __xgeSvgParseOpacity(sValue, &fValue) ) {
		pStyle->fStrokeOpacity = fValue;
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "stroke-width", sValue, sizeof(sValue), bAllowAttrs) && __xgeSvgParseLength(sValue, fLengthRef, &fValue) ) {
		pStyle->fStrokeWidth = fValue;
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "stroke-miterlimit", sValue, sizeof(sValue), bAllowAttrs) && __xgeSvgParseFloat(sValue, &fValue) ) {
		pStyle->fMiterLimit = fValue;
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "stroke-dasharray", sValue, sizeof(sValue), bAllowAttrs) ) {
		__xgeSvgParseDashArray(sValue, fLengthRef, pStyle->fDashPattern, &pStyle->iDashCount);
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "stroke-dashoffset", sValue, sizeof(sValue), bAllowAttrs) && __xgeSvgParseLength(sValue, fLengthRef, &fValue) ) {
		pStyle->fDashOffset = fValue;
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "clip-path", sValue, sizeof(sValue), bAllowAttrs) ) {
		if ( strcmp(sValue, "none") == 0 ) {
			pStyle->sClipId[0] = '\0';
		} else {
			__xgeSvgUrlIdCopyFromValue(sValue, pStyle->sClipId, sizeof(pStyle->sClipId));
		}
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "mask", sValue, sizeof(sValue), bAllowAttrs) ) {
		if ( strcmp(sValue, "none") == 0 ) {
			pStyle->sMaskId[0] = '\0';
		} else {
			__xgeSvgUrlIdCopyFromValue(sValue, pStyle->sMaskId, sizeof(pStyle->sMaskId));
		}
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "fill-rule", sValue, sizeof(sValue), bAllowAttrs) ) {
		pStyle->iFillRule = (strcmp(sValue, "evenodd") == 0) ? XGE_SHAPE_EX_FILL_EVEN_ODD : XGE_SHAPE_EX_FILL_NON_ZERO;
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "paint-order", sValue, sizeof(sValue), bAllowAttrs) ) {
		pStyle->bStrokeFirst = (strncmp(sValue, "stroke", 6) == 0);
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "stroke-linecap", sValue, sizeof(sValue), bAllowAttrs) ) {
		if ( strcmp(sValue, "round") == 0 ) pStyle->iLineCap = XGE_SHAPE_EX_CAP_ROUND;
		else if ( strcmp(sValue, "square") == 0 ) pStyle->iLineCap = XGE_SHAPE_EX_CAP_SQUARE;
		else pStyle->iLineCap = XGE_SHAPE_EX_CAP_BUTT;
	}
	if ( __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "stroke-linejoin", sValue, sizeof(sValue), bAllowAttrs) ) {
		if ( strcmp(sValue, "round") == 0 ) pStyle->iLineJoin = XGE_SHAPE_EX_JOIN_ROUND;
		else if ( strcmp(sValue, "bevel") == 0 ) pStyle->iLineJoin = XGE_SHAPE_EX_JOIN_BEVEL;
		else pStyle->iLineJoin = XGE_SHAPE_EX_JOIN_MITER;
	}
	if ( bAllowTransform && __xgeSvgStylePropertyCopy(pTag, pTagEnd, sStyle, "transform", sValue, sizeof(sValue), bAllowAttrs) ) {
		xge_shape_ex_matrix_t tTransform;
		if ( __xgeSvgParseTransform(sValue, &tTransform) ) {
			pStyle->tTransform = __xgeShapeExMatrixMul(pStyle->tTransform, tTransform);
		}
	}
}

static void __xgeSvgStyleApplyDeclaration(xge_svg pSvg, xge_svg_style_t* pStyle, const char* sStyle)
{
	__xgeSvgStyleApplyProperties(pSvg, pStyle, NULL, NULL, sStyle, 0, 1);
}

static void __xgeSvgStyleApplyPresentationAttrs(xge_svg pSvg, xge_svg_style_t* pStyle, const char* pTag, const char* pTagEnd)
{
	if ( (pStyle == NULL) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return;
	}
	__xgeSvgStyleApplyProperties(pSvg, pStyle, pTag, pTagEnd, NULL, 1, 1);
}

static void __xgeSvgStyleApplyInlineStyle(xge_svg pSvg, xge_svg_style_t* pStyle, const char* pTag, const char* pTagEnd)
{
	char sStyle[XGE_SVG_ATTR_MAX];

	if ( (pStyle == NULL) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return;
	}
	sStyle[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "style", sStyle, sizeof(sStyle));
	__xgeSvgStyleApplyProperties(pSvg, pStyle, NULL, NULL, sStyle, 0, 1);
}

static int __xgeSvgClassListContains(const char* sClassList, const char* sName)
{
	const char* p;
	int iNameLen;

	if ( (sClassList == NULL) || (sName == NULL) || (sName[0] == '\0') ) {
		return 0;
	}
	iNameLen = (int)strlen(sName);
	p = sClassList;
	while ( *p != '\0' ) {
		const char* pStart;
		const char* pEnd;
		while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') ) {
			p++;
		}
		pStart = p;
		while ( (*p != '\0') && (*p != ' ') && (*p != '\t') && (*p != '\r') && (*p != '\n') ) {
			p++;
		}
		pEnd = p;
		if ( ((int)(pEnd - pStart) == iNameLen) && (strncmp(pStart, sName, (size_t)iNameLen) == 0) ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeSvgSimpleSelectorReadToken(const char** ppText, char* sOut, int iOutCapacity)
{
	const char* p;
	const char* pStart;
	int iLen;

	if ( (ppText == NULL) || (*ppText == NULL) || (sOut == NULL) || (iOutCapacity <= 0) ) {
		return 0;
	}
	sOut[0] = '\0';
	p = *ppText;
	pStart = p;
	while ( (*p != '\0') && (*p != '.') && (*p != '#') ) {
		p++;
	}
	iLen = (int)(p - pStart);
	if ( iLen <= 0 ) {
		return 0;
	}
	if ( iLen >= iOutCapacity ) {
		iLen = iOutCapacity - 1;
	}
	memcpy(sOut, pStart, (size_t)iLen);
	sOut[iLen] = '\0';
	*ppText = p;
	return 1;
}

static int __xgeSvgSimpleSelectorMatches(const char* sSelector, const char* pTag, const char* pTagEnd)
{
	const char* p;
	char sToken[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	char sTagName[XGE_SVG_ATTR_MAX];
	int bMatched;

	if ( (sSelector == NULL) || (sSelector[0] == '\0') || (pTag == NULL) || (pTagEnd == NULL) ) {
		return 0;
	}
	p = sSelector;
	bMatched = 0;
	if ( (*p != '.') && (*p != '#') ) {
		if ( !__xgeSvgSimpleSelectorReadToken(&p, sToken, sizeof(sToken)) ) {
			return 0;
		}
		if ( !__xgeSvgTagNameCopy(pTag, sTagName, sizeof(sTagName)) || (strcmp(sTagName, sToken) != 0) ) {
			return 0;
		}
		bMatched = 1;
	}
	while ( *p != '\0' ) {
		if ( *p == '.' ) {
			p++;
			if ( !__xgeSvgSimpleSelectorReadToken(&p, sToken, sizeof(sToken)) ) {
				return 0;
			}
			if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "class", sValue, sizeof(sValue)) ||
			     !__xgeSvgClassListContains(sValue, sToken) ) {
				return 0;
			}
			bMatched = 1;
		} else if ( *p == '#' ) {
			p++;
			if ( !__xgeSvgSimpleSelectorReadToken(&p, sToken, sizeof(sToken)) ) {
				return 0;
			}
			if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "id", sValue, sizeof(sValue)) ||
			     (strcmp(sValue, sToken) != 0) ) {
				return 0;
			}
			bMatched = 1;
		} else {
			return 0;
		}
	}
	return bMatched;
}

static int __xgeSvgStyleRuleMatches(const xge_svg_style_rule_t* pRule, const char* pTag, const char* pTagEnd)
{
	char sValue[XGE_SVG_ATTR_MAX];

	if ( (pRule == NULL) || (pRule->sName == NULL) || (pRule->sStyle == NULL) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return 0;
	}
	if ( pRule->iSelectorType == XGE_SVG_STYLE_SELECTOR_SIMPLE ) {
		return __xgeSvgSimpleSelectorMatches(pRule->sName, pTag, pTagEnd);
	}
	if ( pRule->iSelectorType == XGE_SVG_STYLE_SELECTOR_TAG ) {
		return __xgeSvgTagNameEquals(pTag, pRule->sName);
	}
	if ( pRule->iSelectorType == XGE_SVG_STYLE_SELECTOR_ID ) {
		return __xgeSvgAttrCopy(pTag, pTagEnd, "id", sValue, sizeof(sValue)) && (strcmp(sValue, pRule->sName) == 0);
	}
	if ( pRule->iSelectorType == XGE_SVG_STYLE_SELECTOR_CLASS ) {
		return __xgeSvgAttrCopy(pTag, pTagEnd, "class", sValue, sizeof(sValue)) && __xgeSvgClassListContains(sValue, pRule->sName);
	}
	return 0;
}

static void __xgeSvgStyleApplyCss(xge_svg pSvg, xge_svg_style_t* pStyle, const char* pTag, const char* pTagEnd)
{
	int iLastSpecificity;
	int iLastIndex;

	if ( !__xgeSvgValid(pSvg) || (pStyle == NULL) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return;
	}
	iLastSpecificity = -1;
	iLastIndex = -1;
	for ( ;; ) {
		int i;
		int iBestIndex;
		int iBestSpecificity;

		iBestIndex = -1;
		iBestSpecificity = INT32_MAX;
		for ( i = 0; i < pSvg->iStyleRuleCount; i++ ) {
			int iSpecificity = pSvg->pStyleRules[i].iSpecificity;
			if ( (iSpecificity < iLastSpecificity) ||
			     ((iSpecificity == iLastSpecificity) && (i <= iLastIndex)) ) {
				continue;
			}
			if ( (iSpecificity > iBestSpecificity) ||
			     ((iSpecificity == iBestSpecificity) && (iBestIndex >= 0) && (i >= iBestIndex)) ) {
				continue;
			}
			if ( __xgeSvgStyleRuleMatches(&pSvg->pStyleRules[i], pTag, pTagEnd) ) {
				iBestIndex = i;
				iBestSpecificity = iSpecificity;
			}
		}
		if ( iBestIndex < 0 ) {
			break;
		}
		__xgeSvgStyleApplyDeclaration(pSvg, pStyle, pSvg->pStyleRules[iBestIndex].sStyle);
		iLastSpecificity = iBestSpecificity;
		iLastIndex = iBestIndex;
	}
}

static xge_svg_style_t __xgeSvgStyleResolve(xge_svg pSvg, const xge_svg_style_t* pParentStyle, const char* pTag, const char* pTagEnd)
{
	xge_svg_style_t tStyle;

	if ( pParentStyle != NULL ) {
		tStyle = *pParentStyle;
	} else {
		tStyle = __xgeSvgStyleDefault();
	}
	__xgeSvgStyleApplyPresentationAttrs(pSvg, &tStyle, pTag, pTagEnd);
	__xgeSvgStyleApplyCss(pSvg, &tStyle, pTag, pTagEnd);
	__xgeSvgStyleApplyInlineStyle(pSvg, &tStyle, pTag, pTagEnd);
	return tStyle;
}

static void __xgeSvgApplyClipPath(xge_svg pSvg, xge_shape_ex pShape, const char* sClipId)
{
	int iClip;
	xge_rect_t tClip;

	if ( !__xgeSvgValid(pSvg) || (pShape == NULL) || (sClipId == NULL) || (sClipId[0] == '\0') ) {
		return;
	}
	iClip = __xgeSvgClipPathFind(pSvg, sClipId);
	if ( (iClip < 0) || !pSvg->pClipPaths[iClip].bHasRect ) {
		return;
	}
	tClip = pSvg->pClipPaths[iClip].tRect;
	if ( pSvg->pClipPaths[iClip].iUnits == XGE_SVG_CLIP_OBJECT_BOUNDING_BOX ) {
		xge_rect_t tBounds;

		if ( xgeShapeExGetBounds(pShape, 0.0f, &tBounds) != XGE_OK ) {
			return;
		}
		tClip.fX = tBounds.fX + tClip.fX * tBounds.fW;
		tClip.fY = tBounds.fY + tClip.fY * tBounds.fH;
		tClip.fW = tClip.fW * tBounds.fW;
		tClip.fH = tClip.fH * tBounds.fH;
	}
	xgeShapeExClipRectSet(pShape, tClip);
}

static void __xgeSvgApplyShapeStyle(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle)
{
	float fFillOpacity;
	float fStrokeOpacity;

	if ( (pShape == NULL) || (pStyle == NULL) ) {
		return;
	}
	fFillOpacity = pStyle->fOpacity * pStyle->fFillOpacity;
	fStrokeOpacity = pStyle->fOpacity * pStyle->fStrokeOpacity;
	xgeShapeExFillColor(pShape, __xgeSvgColorAlpha(pStyle->iFillColor, fFillOpacity));
	if ( pStyle->sFillGradientId[0] != '\0' ) {
		int iGradient = __xgeSvgLinearGradientFind(pSvg, pStyle->sFillGradientId);
		__xgeSvgResolveGradients(pSvg);
		iGradient = __xgeSvgLinearGradientFind(pSvg, pStyle->sFillGradientId);
		if ( (iGradient >= 0) && (pSvg->pLinearGradients[iGradient].iStopCount > 0) ) {
			xge_svg_linear_gradient_t* pGradient = &pSvg->pLinearGradients[iGradient];
			xge_shape_ex_color_stop_t arrStops[64];
			xge_shape_ex_color_stop_t* pStops;
			int i;

			pStops = (pGradient->iStopCount <= (int)(sizeof(arrStops) / sizeof(arrStops[0]))) ? arrStops :
				(xge_shape_ex_color_stop_t*)xrtMalloc((size_t)pGradient->iStopCount * sizeof(*pStops));
			if ( pStops != NULL ) {
				for ( i = 0; i < pGradient->iStopCount; i++ ) {
					pStops[i] = pGradient->pStops[i];
					pStops[i].iColor = __xgeSvgColorAlpha(pStops[i].iColor, fFillOpacity);
				}
				if ( xgeShapeExFillLinearGradient(pShape, pGradient->fX1, pGradient->fY1, pGradient->fX2, pGradient->fY2, pGradient->iUnits, pStops, pGradient->iStopCount) == XGE_OK ) {
					xgeShapeExFillGradientSpread(pShape, pGradient->iSpread);
					xgeShapeExFillGradientTransformSet(pShape, &pGradient->tTransform);
				}
				if ( pStops != arrStops ) {
					xrtFree(pStops);
				}
			}
		} else {
			iGradient = __xgeSvgRadialGradientFind(pSvg, pStyle->sFillGradientId);
			if ( (iGradient >= 0) && (pSvg->pRadialGradients[iGradient].iStopCount > 0) ) {
				xge_svg_radial_gradient_t* pGradient = &pSvg->pRadialGradients[iGradient];
				xge_shape_ex_color_stop_t arrStops[64];
				xge_shape_ex_color_stop_t* pStops;
				int i;

				pStops = (pGradient->iStopCount <= (int)(sizeof(arrStops) / sizeof(arrStops[0]))) ? arrStops :
					(xge_shape_ex_color_stop_t*)xrtMalloc((size_t)pGradient->iStopCount * sizeof(*pStops));
				if ( pStops != NULL ) {
					for ( i = 0; i < pGradient->iStopCount; i++ ) {
						pStops[i] = pGradient->pStops[i];
						pStops[i].iColor = __xgeSvgColorAlpha(pStops[i].iColor, fFillOpacity);
					}
					if ( xgeShapeExFillRadialGradient(pShape, pGradient->fCX, pGradient->fCY, pGradient->fR, pGradient->fFX, pGradient->fFY, pGradient->iUnits, pStops, pGradient->iStopCount) == XGE_OK ) {
						xgeShapeExFillGradientSpread(pShape, pGradient->iSpread);
						xgeShapeExFillGradientTransformSet(pShape, &pGradient->tTransform);
					}
					if ( pStops != arrStops ) {
						xrtFree(pStops);
					}
				}
			}
		}
	}
	xgeShapeExStrokeColor(pShape, __xgeSvgColorAlpha(pStyle->iStrokeColor, fStrokeOpacity));
	if ( pStyle->sStrokeGradientId[0] != '\0' ) {
		int iGradient;

		__xgeSvgResolveGradients(pSvg);
		iGradient = __xgeSvgLinearGradientFind(pSvg, pStyle->sStrokeGradientId);
		if ( (iGradient >= 0) && (pSvg->pLinearGradients[iGradient].iStopCount > 0) ) {
			xge_svg_linear_gradient_t* pGradient = &pSvg->pLinearGradients[iGradient];
			xge_shape_ex_color_stop_t arrStops[64];
			xge_shape_ex_color_stop_t* pStops;
			int i;

			pStops = (pGradient->iStopCount <= (int)(sizeof(arrStops) / sizeof(arrStops[0]))) ? arrStops :
				(xge_shape_ex_color_stop_t*)xrtMalloc((size_t)pGradient->iStopCount * sizeof(*pStops));
			if ( pStops != NULL ) {
				for ( i = 0; i < pGradient->iStopCount; i++ ) {
					pStops[i] = pGradient->pStops[i];
					pStops[i].iColor = __xgeSvgColorAlpha(pStops[i].iColor, fStrokeOpacity);
				}
				if ( xgeShapeExStrokeLinearGradient(pShape, pGradient->fX1, pGradient->fY1, pGradient->fX2, pGradient->fY2, pGradient->iUnits, pStops, pGradient->iStopCount) == XGE_OK ) {
					xgeShapeExStrokeGradientSpread(pShape, pGradient->iSpread);
					xgeShapeExStrokeGradientTransformSet(pShape, &pGradient->tTransform);
				}
				if ( pStops != arrStops ) {
					xrtFree(pStops);
				}
			}
		} else {
			iGradient = __xgeSvgRadialGradientFind(pSvg, pStyle->sStrokeGradientId);
			if ( (iGradient >= 0) && (pSvg->pRadialGradients[iGradient].iStopCount > 0) ) {
				xge_svg_radial_gradient_t* pGradient = &pSvg->pRadialGradients[iGradient];
				xge_shape_ex_color_stop_t arrStops[64];
				xge_shape_ex_color_stop_t* pStops;
				int i;

				pStops = (pGradient->iStopCount <= (int)(sizeof(arrStops) / sizeof(arrStops[0]))) ? arrStops :
					(xge_shape_ex_color_stop_t*)xrtMalloc((size_t)pGradient->iStopCount * sizeof(*pStops));
				if ( pStops != NULL ) {
					for ( i = 0; i < pGradient->iStopCount; i++ ) {
						pStops[i] = pGradient->pStops[i];
						pStops[i].iColor = __xgeSvgColorAlpha(pStops[i].iColor, fStrokeOpacity);
					}
					if ( xgeShapeExStrokeRadialGradient(pShape, pGradient->fCX, pGradient->fCY, pGradient->fR, pGradient->fFX, pGradient->fFY, pGradient->iUnits, pStops, pGradient->iStopCount) == XGE_OK ) {
						xgeShapeExStrokeGradientSpread(pShape, pGradient->iSpread);
						xgeShapeExStrokeGradientTransformSet(pShape, &pGradient->tTransform);
					}
					if ( pStops != arrStops ) {
						xrtFree(pStops);
					}
				}
			}
		}
	}
	xgeShapeExStrokeWidth(pShape, pStyle->fStrokeWidth);
	xgeShapeExStrokeMiterLimit(pShape, pStyle->fMiterLimit);
	xgeShapeExStrokeDash(pShape, pStyle->fDashPattern, pStyle->iDashCount, pStyle->fDashOffset);
	xgeShapeExFillRule(pShape, pStyle->iFillRule);
	xgeShapeExStrokeCap(pShape, pStyle->iLineCap);
	xgeShapeExStrokeJoin(pShape, pStyle->iLineJoin);
	xgeShapeExPaintOrder(pShape, pStyle->bStrokeFirst);
	xgeShapeExVisible(pShape, pStyle->bVisible && pStyle->bVisibility);
	xgeShapeExTransformSet(pShape, &pStyle->tTransform);
	if ( pStyle->sClipId[0] != '\0' ) {
		__xgeSvgApplyClipPath(pSvg, pShape, pStyle->sClipId);
	}
}

static int __xgeSvgParsePoints(xge_shape_ex pShape, const char* sPoints, int bClose)
{
	const char* p;
	char* pEnd;
	float fX;
	float fY;
	int bFirst;

	if ( (pShape == NULL) || (sPoints == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	p = sPoints;
	bFirst = 1;
	while ( *p != '\0' ) {
		while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') || (*p == ',') ) p++;
		if ( *p == '\0' ) break;
		fX = (float)strtod(p, &pEnd);
		if ( pEnd == p ) return XGE_ERROR_INVALID_ARGUMENT;
		p = pEnd;
		while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') || (*p == ',') ) p++;
		fY = (float)strtod(p, &pEnd);
		if ( pEnd == p ) return XGE_ERROR_INVALID_ARGUMENT;
		p = pEnd;
		if ( bFirst ) {
			xgeShapeExMoveTo(pShape, fX, fY);
			bFirst = 0;
		} else {
			xgeShapeExLineTo(pShape, fX, fY);
		}
	}
	if ( bClose ) {
		xgeShapeExClose(pShape);
	}
	return XGE_OK;
}

static int __xgeSvgAddShape(xge_svg pSvg, xge_shape_ex pShape, int iDefIndex)
{
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iDefIndex >= 0 ) {
		if ( iDefIndex >= pSvg->iDefCount ) {
			xgeShapeExDestroy(pShape);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iRet = __xgeSvgDefAddShape(&pSvg->pDefs[iDefIndex], pShape);
	} else {
		iRet = xgeShapeExSceneAdd(pSvg->pScene, pShape);
	}
	xgeShapeExDestroy(pShape);
	return iRet;
}

static xge_rect_t __xgeSvgRectIntersect(xge_rect_t a, xge_rect_t b)
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

static xge_rect_t __xgeSvgObjectRectToBounds(xge_rect_t tRect, xge_rect_t tBounds)
{
	tRect.fX = tBounds.fX + tRect.fX * tBounds.fW;
	tRect.fY = tBounds.fY + tRect.fY * tBounds.fH;
	tRect.fW = tRect.fW * tBounds.fW;
	tRect.fH = tRect.fH * tBounds.fH;
	return tRect;
}

static int __xgeSvgAddStyledShape(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iDefIndex)
{
	int iMask;
	int i;
	int bAdded;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pStyle != NULL ) {
		__xgeSvgApplyClipPath(pSvg, pShape, pStyle->sClipId);
	}
	if ( (pStyle == NULL) || (pStyle->sMaskId[0] == '\0') ) {
		return __xgeSvgAddShape(pSvg, pShape, iDefIndex);
	}
	iMask = __xgeSvgMaskFind(pSvg, pStyle->sMaskId);
	if ( iMask < 0 ) {
		return __xgeSvgAddShape(pSvg, pShape, iDefIndex);
	}
	if ( pSvg->pMasks[iMask].iRectCount <= 0 ) {
		xgeShapeExDestroy(pShape);
		return XGE_OK;
	}
	bAdded = 0;
	for ( i = 0; i < pSvg->pMasks[iMask].iRectCount; i++ ) {
		xge_svg_mask_rect_t tMaskRect = pSvg->pMasks[iMask].pRects[i];
		xge_shape_ex pClone;
		xge_rect_t tClip;

		if ( (tMaskRect.fOpacity <= 0.0f) || (tMaskRect.tRect.fW <= 0.0f) || (tMaskRect.tRect.fH <= 0.0f) ) {
			continue;
		}
		iRet = xgeShapeExClone(pShape, &pClone);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		tClip = tMaskRect.tRect;
		if ( pSvg->pMasks[iMask].iContentUnits == XGE_SVG_MASK_OBJECT_BOUNDING_BOX ) {
			xge_rect_t tBounds;

			iRet = xgeShapeExGetBounds(pClone, 0.0f, &tBounds);
			if ( iRet != XGE_OK ) {
				xgeShapeExDestroy(pClone);
				xgeShapeExDestroy(pShape);
				return iRet;
			}
			tClip = __xgeSvgObjectRectToBounds(tClip, tBounds);
		}
		if ( pClone->bClipRect ) {
			tClip = __xgeSvgRectIntersect(pClone->tClipRect, tClip);
		}
		xgeShapeExClipRectSet(pClone, tClip);
		xgeShapeExOpacity(pClone, pClone->fOpacity * tMaskRect.fOpacity);
		iRet = __xgeSvgAddShape(pSvg, pClone, iDefIndex);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		bAdded = 1;
	}
	xgeShapeExDestroy(pShape);
	return bAdded ? XGE_OK : XGE_OK;
}

static int __xgeSvgShapeTargetDef(xge_svg pSvg, const char* pTag, const char* pTagEnd, int bInDefs, int iCurrentDef, int* pError)
{
	char sId[XGE_SVG_ATTR_MAX];
	int iDefIndex;
	int iRet;

	if ( pError != NULL ) {
		*pError = XGE_OK;
	}
	if ( !bInDefs ) {
		return -1;
	}
	if ( iCurrentDef >= 0 ) {
		return iCurrentDef;
	}
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "id", sId, sizeof(sId)) || (sId[0] == '\0') ) {
		return -2;
	}
	iRet = __xgeSvgDefGetOrCreate(pSvg, sId, &iDefIndex);
	if ( iRet != XGE_OK ) {
		if ( pError != NULL ) {
			*pError = iRet;
		}
		return -2;
	}
	return iDefIndex;
}

static int __xgeSvgParseElement(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_style_t* pParentStyle, int bInDefs, int iCurrentDef)
{
	char sValue[XGE_SVG_ATTR_MAX];
	xge_svg_style_t tStyle;
	xge_shape_ex pShape;
	int iTargetDef;
	int iRet;

	if ( pParentStyle == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iTargetDef = __xgeSvgShapeTargetDef(pSvg, pTag, pTagEnd, bInDefs, iCurrentDef, &iRet);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( iTargetDef == -2 ) {
		return XGE_OK;
	}
	if ( __xgeSvgTagNameEquals(pTag, "path") ) {
		if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "d", sValue, sizeof(sValue)) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = xgeShapeExAppendSvgPath(pShape, sValue);
		if ( iRet == XGE_OK ) {
			tStyle = __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd);
			__xgeSvgApplyShapeStyle(pSvg, pShape, &tStyle);
			return __xgeSvgAddStyledShape(pSvg, pShape, &tStyle, iTargetDef);
		}
		xgeShapeExDestroy(pShape);
		return iRet;
	}
	if ( __xgeSvgTagNameEquals(pTag, "rect") ) {
		float fX = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, 0.0f);
		float fY = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, 0.0f);
		float fW = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, 0.0f);
		float fH = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, 0.0f);
		float fRX = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "rx", XGE_SVG_LENGTH_BASIS_X, 0.0f);
		float fRY = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "ry", XGE_SVG_LENGTH_BASIS_Y, fRX);

		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = xgeShapeExAppendRect(pShape, fX, fY, fW, fH, fRX, fRY, 1);
		if ( iRet == XGE_OK ) {
			tStyle = __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd);
			__xgeSvgApplyShapeStyle(pSvg, pShape, &tStyle);
			return __xgeSvgAddStyledShape(pSvg, pShape, &tStyle, iTargetDef);
		}
		xgeShapeExDestroy(pShape);
		return iRet;
	}
	if ( __xgeSvgTagNameEquals(pTag, "circle") || __xgeSvgTagNameEquals(pTag, "ellipse") ) {
		float fCX = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "cx", XGE_SVG_LENGTH_BASIS_X, 0.0f);
		float fCY = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "cy", XGE_SVG_LENGTH_BASIS_Y, 0.0f);
		float fR = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "r", XGE_SVG_LENGTH_BASIS_OTHER, 0.0f);
		float fRX = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "rx", XGE_SVG_LENGTH_BASIS_X, fR);
		float fRY = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "ry", XGE_SVG_LENGTH_BASIS_Y, fR);

		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = xgeShapeExAppendCircle(pShape, fCX, fCY, fRX, fRY, 1);
		if ( iRet == XGE_OK ) {
			tStyle = __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd);
			__xgeSvgApplyShapeStyle(pSvg, pShape, &tStyle);
			return __xgeSvgAddStyledShape(pSvg, pShape, &tStyle, iTargetDef);
		}
		xgeShapeExDestroy(pShape);
		return iRet;
	}
	if ( __xgeSvgTagNameEquals(pTag, "line") ) {
		float fX1 = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "x1", XGE_SVG_LENGTH_BASIS_X, 0.0f);
		float fY1 = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "y1", XGE_SVG_LENGTH_BASIS_Y, 0.0f);
		float fX2 = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "x2", XGE_SVG_LENGTH_BASIS_X, 0.0f);
		float fY2 = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "y2", XGE_SVG_LENGTH_BASIS_Y, 0.0f);

		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		xgeShapeExMoveTo(pShape, fX1, fY1);
		xgeShapeExLineTo(pShape, fX2, fY2);
		tStyle = __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd);
		__xgeSvgApplyShapeStyle(pSvg, pShape, &tStyle);
		return __xgeSvgAddStyledShape(pSvg, pShape, &tStyle, iTargetDef);
	}
	if ( __xgeSvgTagNameEquals(pTag, "polygon") || __xgeSvgTagNameEquals(pTag, "polyline") ) {
		int bClose = __xgeSvgTagNameEquals(pTag, "polygon");

		if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "points", sValue, sizeof(sValue)) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = __xgeSvgParsePoints(pShape, sValue, bClose);
		if ( iRet == XGE_OK ) {
			tStyle = __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd);
			__xgeSvgApplyShapeStyle(pSvg, pShape, &tStyle);
			return __xgeSvgAddStyledShape(pSvg, pShape, &tStyle, iTargetDef);
		}
		xgeShapeExDestroy(pShape);
		return iRet;
	}
	return XGE_OK;
}

static int __xgeSvgApplyUseInstance(xge_svg pSvg, int iDefIndex, const xge_svg_style_t* pUseStyle, int iTargetDef, float fX, float fY, float fW, float fH, int bHasWidth, int bHasHeight)
{
	xge_svg_style_t tStyle;
	xge_shape_ex_matrix_t tUseTransform;
	int iSourceShapeCount;
	int i;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pUseStyle == NULL) || (iDefIndex < 0) || (iDefIndex >= pSvg->iDefCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tStyle = *pUseStyle;
	if ( pSvg->pDefs[iDefIndex].bHasViewBox ) {
		xge_rect_t tDst;
		xge_rect_t tViewport;

		if ( !bHasWidth ) fW = pSvg->pDefs[iDefIndex].tViewBox.fW;
		if ( !bHasHeight ) fH = pSvg->pDefs[iDefIndex].tViewBox.fH;
		if ( (fW <= 0.0f) || (fH <= 0.0f) ) {
			return XGE_OK;
		}
		tDst.fX = fX;
		tDst.fY = fY;
		tDst.fW = fW;
		tDst.fH = fH;
		tViewport = __xgeSvgAspectViewport(pSvg->pDefs[iDefIndex].tViewBox, tDst,
			pSvg->pDefs[iDefIndex].iAspectAlignX,
			pSvg->pDefs[iDefIndex].iAspectAlignY,
			pSvg->pDefs[iDefIndex].iAspectMeetOrSlice);
		tUseTransform = __xgeSvgViewBoxMatrix(pSvg->pDefs[iDefIndex].tViewBox, tViewport);
		tStyle.tTransform = __xgeShapeExMatrixMul(tStyle.tTransform, tUseTransform);
	} else if ( (fabsf(fX) > 0.0f) || (fabsf(fY) > 0.0f) ) {
		tUseTransform = __xgeSvgMatrixTranslate(fX, fY);
		tStyle.tTransform = __xgeShapeExMatrixMul(tStyle.tTransform, tUseTransform);
	}
	iSourceShapeCount = pSvg->pDefs[iDefIndex].iShapeCount;
	for ( i = 0; i < iSourceShapeCount; i++ ) {
		xge_shape_ex pClone;
		xge_shape_ex_matrix_t tShapeTransform;

		iRet = xgeShapeExClone(pSvg->pDefs[iDefIndex].pShapes[i], &pClone);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		tShapeTransform = __xgeShapeExMatrixMul(tStyle.tTransform, pClone->tTransform);
		xgeShapeExTransformSet(pClone, &tShapeTransform);
		xgeShapeExOpacity(pClone, pClone->fOpacity * tStyle.fOpacity);
		if ( !tStyle.bVisible || !tStyle.bVisibility ) {
			xgeShapeExVisible(pClone, 0);
		}
		if ( tStyle.sClipId[0] != '\0' ) {
			__xgeSvgApplyClipPath(pSvg, pClone, tStyle.sClipId);
		}
		iRet = __xgeSvgAddStyledShape(pSvg, pClone, &tStyle, iTargetDef);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeSvgParseUse(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_style_t* pParentStyle, int bInDefs, int iCurrentDef)
{
	char sId[XGE_SVG_ATTR_MAX];
	xge_svg_style_t tStyle;
	float fX;
	float fY;
	float fW;
	float fH;
	int bHasWidth;
	int bHasHeight;
	int iDefIndex;
	int iTargetDef;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pParentStyle == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeSvgHrefIdCopy(pTag, pTagEnd, sId, sizeof(sId)) ) {
		return XGE_OK;
	}
	iTargetDef = __xgeSvgShapeTargetDef(pSvg, pTag, pTagEnd, bInDefs, iCurrentDef, &iRet);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( iTargetDef == -2 ) {
		return XGE_OK;
	}
	tStyle = __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd);
	fX = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, 0.0f);
	fY = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, 0.0f);
	fW = 0.0f;
	fH = 0.0f;
	bHasWidth = __xgeSvgAttrLengthCopy(pTag, pTagEnd, "width", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_X), &fW);
	bHasHeight = __xgeSvgAttrLengthCopy(pTag, pTagEnd, "height", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_Y), &fH);
	iDefIndex = __xgeSvgDefFind(pSvg, sId);
	if ( iDefIndex < 0 ) {
		return __xgeSvgPendingUseAdd(pSvg, sId, &tStyle, iTargetDef, fX, fY, fW, fH, bHasWidth, bHasHeight);
	}
	return __xgeSvgApplyUseInstance(pSvg, iDefIndex, &tStyle, iTargetDef, fX, fY, fW, fH, bHasWidth, bHasHeight);
}

static int __xgeSvgFinalizeDefPendingUses(xge_svg pSvg, int iDefIndex, int* pDefStates)
{
	int i;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iDefIndex < 0) || (iDefIndex >= pSvg->iDefCount) || (pDefStates == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pDefStates[iDefIndex] == 2 ) {
		return XGE_OK;
	}
	if ( pDefStates[iDefIndex] == 1 ) {
		return XGE_OK;
	}
	pDefStates[iDefIndex] = 1;
	for ( i = 0; i < pSvg->iPendingUseCount; i++ ) {
		xge_svg_pending_use_t* pUse = &pSvg->pPendingUses[i];
		int iSourceDef;

		if ( pUse->bResolved || (pUse->iTargetDef != iDefIndex) ) {
			continue;
		}
		iSourceDef = __xgeSvgDefFind(pSvg, pUse->sId);
		if ( iSourceDef < 0 ) {
			continue;
		}
		if ( iSourceDef != iDefIndex ) {
			iRet = __xgeSvgFinalizeDefPendingUses(pSvg, iSourceDef, pDefStates);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
		iRet = __xgeSvgApplyUseInstance(pSvg, iSourceDef, &pUse->tStyle, pUse->iTargetDef, pUse->fX, pUse->fY, pUse->fW, pUse->fH, pUse->bHasWidth, pUse->bHasHeight);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		pUse->bResolved = 1;
	}
	pDefStates[iDefIndex] = 2;
	return XGE_OK;
}

static int __xgeSvgResolvePendingUses(xge_svg pSvg)
{
	int* pDefStates;
	int i;
	int iRet;

	if ( !__xgeSvgValid(pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pSvg->iPendingUseCount <= 0 ) {
		return XGE_OK;
	}
	pDefStates = (int*)xrtCalloc((size_t)(pSvg->iDefCount > 0 ? pSvg->iDefCount : 1), sizeof(*pDefStates));
	if ( pDefStates == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < pSvg->iDefCount; i++ ) {
		iRet = __xgeSvgFinalizeDefPendingUses(pSvg, i, pDefStates);
		if ( iRet != XGE_OK ) {
			xrtFree(pDefStates);
			return iRet;
		}
	}
	for ( i = 0; i < pSvg->iPendingUseCount; i++ ) {
		xge_svg_pending_use_t* pUse = &pSvg->pPendingUses[i];
		int iSourceDef;

		if ( pUse->bResolved || (pUse->iTargetDef >= 0) ) {
			continue;
		}
		iSourceDef = __xgeSvgDefFind(pSvg, pUse->sId);
		if ( iSourceDef < 0 ) {
			continue;
		}
		iRet = __xgeSvgFinalizeDefPendingUses(pSvg, iSourceDef, pDefStates);
		if ( iRet != XGE_OK ) {
			xrtFree(pDefStates);
			return iRet;
		}
		iRet = __xgeSvgApplyUseInstance(pSvg, iSourceDef, &pUse->tStyle, pUse->iTargetDef, pUse->fX, pUse->fY, pUse->fW, pUse->fH, pUse->bHasWidth, pUse->bHasHeight);
		if ( iRet != XGE_OK ) {
			xrtFree(pDefStates);
			return iRet;
		}
		pUse->bResolved = 1;
	}
	xrtFree(pDefStates);
	__xgeSvgPendingUsesClear(pSvg);
	return XGE_OK;
}

static int __xgeSvgParseLinearGradient(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_style_t* pParentStyle, int* pGradientIndex)
{
	char sId[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	int iIndex;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pGradientIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pGradientIndex = -1;
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "id", sId, sizeof(sId)) || (sId[0] == '\0') ) {
		return XGE_OK;
	}
	iRet = __xgeSvgLinearGradientGetOrCreate(pSvg, sId, &iIndex);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xrtFree(pSvg->pLinearGradients[iIndex].sHrefId);
	pSvg->pLinearGradients[iIndex].sHrefId = NULL;
	pSvg->pLinearGradients[iIndex].fX1 = 0.0f;
	pSvg->pLinearGradients[iIndex].fY1 = 0.0f;
	pSvg->pLinearGradients[iIndex].fX2 = 1.0f;
	pSvg->pLinearGradients[iIndex].fY2 = 0.0f;
	pSvg->pLinearGradients[iIndex].iUnits = XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX;
	pSvg->pLinearGradients[iIndex].iSpread = XGE_SHAPE_EX_GRADIENT_SPREAD_PAD;
	pSvg->pLinearGradients[iIndex].tStyle = __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd);
	__xgeSvgStyleResetStopPaint(&pSvg->pLinearGradients[iIndex].tStyle);
	pSvg->pLinearGradients[iIndex].tTransform = __xgeShapeExMatrixIdentity();
	pSvg->pLinearGradients[iIndex].iFlags = 0;
	pSvg->pLinearGradients[iIndex].iStopCount = 0;
	if ( __xgeSvgHrefIdCopy(pTag, pTagEnd, sValue, sizeof(sValue)) ) {
		pSvg->pLinearGradients[iIndex].sHrefId = __xgeStrDup(sValue);
		if ( pSvg->pLinearGradients[iIndex].sHrefId == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( __xgeSvgAttrFloatOrPercentCopy(pTag, pTagEnd, "x1", &pSvg->pLinearGradients[iIndex].fX1) ) {
		pSvg->pLinearGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_X1;
	}
	if ( __xgeSvgAttrFloatOrPercentCopy(pTag, pTagEnd, "y1", &pSvg->pLinearGradients[iIndex].fY1) ) {
		pSvg->pLinearGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_Y1;
	}
	if ( __xgeSvgAttrFloatOrPercentCopy(pTag, pTagEnd, "x2", &pSvg->pLinearGradients[iIndex].fX2) ) {
		pSvg->pLinearGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_X2;
	}
	if ( __xgeSvgAttrFloatOrPercentCopy(pTag, pTagEnd, "y2", &pSvg->pLinearGradients[iIndex].fY2) ) {
		pSvg->pLinearGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_Y2;
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "gradientUnits", sValue, sizeof(sValue)) ) {
		pSvg->pLinearGradients[iIndex].iUnits = (strcmp(sValue, "userSpaceOnUse") == 0) ? XGE_SHAPE_EX_GRADIENT_USER_SPACE : XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX;
		pSvg->pLinearGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_UNITS;
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "spreadMethod", sValue, sizeof(sValue)) ) {
		pSvg->pLinearGradients[iIndex].iSpread = __xgeSvgParseGradientSpread(sValue);
		pSvg->pLinearGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_SPREAD;
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "gradientTransform", sValue, sizeof(sValue)) ) {
		xge_shape_ex_matrix_t tTransform;

		if ( __xgeSvgParseTransform(sValue, &tTransform) ) {
			pSvg->pLinearGradients[iIndex].tTransform = tTransform;
			pSvg->pLinearGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_TRANSFORM;
		}
	}
	*pGradientIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgParseRadialGradient(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_style_t* pParentStyle, int* pGradientIndex)
{
	char sId[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	int iIndex;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pGradientIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pGradientIndex = -1;
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "id", sId, sizeof(sId)) || (sId[0] == '\0') ) {
		return XGE_OK;
	}
	iRet = __xgeSvgRadialGradientGetOrCreate(pSvg, sId, &iIndex);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xrtFree(pSvg->pRadialGradients[iIndex].sHrefId);
	pSvg->pRadialGradients[iIndex].sHrefId = NULL;
	pSvg->pRadialGradients[iIndex].fCX = 0.5f;
	pSvg->pRadialGradients[iIndex].fCY = 0.5f;
	pSvg->pRadialGradients[iIndex].fR = 0.5f;
	pSvg->pRadialGradients[iIndex].fFX = 0.5f;
	pSvg->pRadialGradients[iIndex].fFY = 0.5f;
	pSvg->pRadialGradients[iIndex].iUnits = XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX;
	pSvg->pRadialGradients[iIndex].iSpread = XGE_SHAPE_EX_GRADIENT_SPREAD_PAD;
	pSvg->pRadialGradients[iIndex].tStyle = __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd);
	__xgeSvgStyleResetStopPaint(&pSvg->pRadialGradients[iIndex].tStyle);
	pSvg->pRadialGradients[iIndex].tTransform = __xgeShapeExMatrixIdentity();
	pSvg->pRadialGradients[iIndex].iFlags = 0;
	pSvg->pRadialGradients[iIndex].iStopCount = 0;
	if ( __xgeSvgHrefIdCopy(pTag, pTagEnd, sValue, sizeof(sValue)) ) {
		pSvg->pRadialGradients[iIndex].sHrefId = __xgeStrDup(sValue);
		if ( pSvg->pRadialGradients[iIndex].sHrefId == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( __xgeSvgAttrFloatOrPercentCopy(pTag, pTagEnd, "cx", &pSvg->pRadialGradients[iIndex].fCX) ) {
		pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_CX;
	}
	if ( __xgeSvgAttrFloatOrPercentCopy(pTag, pTagEnd, "cy", &pSvg->pRadialGradients[iIndex].fCY) ) {
		pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_CY;
	}
	if ( __xgeSvgAttrFloatOrPercentCopy(pTag, pTagEnd, "r", &pSvg->pRadialGradients[iIndex].fR) ) {
		pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_R;
	}
	if ( __xgeSvgAttrFloatOrPercentCopy(pTag, pTagEnd, "fx", &pSvg->pRadialGradients[iIndex].fFX) ) {
		pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_FX;
	} else if ( pSvg->pRadialGradients[iIndex].sHrefId == NULL ) {
		pSvg->pRadialGradients[iIndex].fFX = pSvg->pRadialGradients[iIndex].fCX;
	}
	if ( __xgeSvgAttrFloatOrPercentCopy(pTag, pTagEnd, "fy", &pSvg->pRadialGradients[iIndex].fFY) ) {
		pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_FY;
	} else if ( pSvg->pRadialGradients[iIndex].sHrefId == NULL ) {
		pSvg->pRadialGradients[iIndex].fFY = pSvg->pRadialGradients[iIndex].fCY;
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "gradientUnits", sValue, sizeof(sValue)) ) {
		pSvg->pRadialGradients[iIndex].iUnits = (strcmp(sValue, "userSpaceOnUse") == 0) ? XGE_SHAPE_EX_GRADIENT_USER_SPACE : XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX;
		pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_UNITS;
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "spreadMethod", sValue, sizeof(sValue)) ) {
		pSvg->pRadialGradients[iIndex].iSpread = __xgeSvgParseGradientSpread(sValue);
		pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_SPREAD;
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "gradientTransform", sValue, sizeof(sValue)) ) {
		xge_shape_ex_matrix_t tTransform;

		if ( __xgeSvgParseTransform(sValue, &tTransform) ) {
			pSvg->pRadialGradients[iIndex].tTransform = tTransform;
			pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_TRANSFORM;
		}
	}
	if ( pSvg->pRadialGradients[iIndex].fR <= 0.0f ) {
		pSvg->pRadialGradients[iIndex].fR = 0.5f;
	}
	*pGradientIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgParseGradientStopData(xge_svg pSvg, const xge_svg_style_t* pGradientStyle, const char* pTag, const char* pTagEnd, xge_shape_ex_color_stop_t* pStop)
{
	xge_svg_style_t tBaseStyle;
	xge_svg_style_t tStopStyle;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pStop == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pStop->fOffset = __xgeSvgAttrFloatOrPercent(pTag, pTagEnd, "offset", 0.0f);
	tBaseStyle = pGradientStyle != NULL ? *pGradientStyle : __xgeSvgStyleDefault();
	__xgeSvgStyleResetStopPaint(&tBaseStyle);
	tStopStyle = __xgeSvgStyleResolve(pSvg, &tBaseStyle, pTag, pTagEnd);
	pStop->iColor = tStopStyle.bStopColorSet ? tStopStyle.iStopColor : XGE_COLOR_RGBA(0, 0, 0, 255);
	pStop->iColor = __xgeSvgColorAlpha(pStop->iColor, tStopStyle.bStopOpacitySet ? tStopStyle.fStopOpacity : 1.0f);
	return XGE_OK;
}

static int __xgeSvgParseGradientStop(xge_svg pSvg, int iGradientIndex, const char* pTag, const char* pTagEnd)
{
	xge_shape_ex_color_stop_t tStop;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iGradientIndex < 0) || (iGradientIndex >= pSvg->iLinearGradientCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeSvgParseGradientStopData(pSvg, &pSvg->pLinearGradients[iGradientIndex].tStyle, pTag, pTagEnd, &tStop);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	return __xgeSvgLinearGradientAddStop(&pSvg->pLinearGradients[iGradientIndex], tStop);
}

static int __xgeSvgParseRadialGradientStop(xge_svg pSvg, int iGradientIndex, const char* pTag, const char* pTagEnd)
{
	xge_shape_ex_color_stop_t tStop;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iGradientIndex < 0) || (iGradientIndex >= pSvg->iRadialGradientCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeSvgParseGradientStopData(pSvg, &pSvg->pRadialGradients[iGradientIndex].tStyle, pTag, pTagEnd, &tStop);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	return __xgeSvgRadialGradientAddStop(&pSvg->pRadialGradients[iGradientIndex], tStop);
}

static void __xgeSvgParseRoot(xge_svg pSvg, const char* pTag, const char* pTagEnd)
{
	char sValue[XGE_SVG_ATTR_MAX];
	float fX;
	float fY;
	float fW;
	float fH;

	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "viewBox", sValue, sizeof(sValue)) &&
	     __xgeSvgParseFourFloats(sValue, &fX, &fY, &fW, &fH) && (fW > 0.0f) && (fH > 0.0f) ) {
		pSvg->tViewBox.fX = fX;
		pSvg->tViewBox.fY = fY;
		pSvg->tViewBox.fW = fW;
		pSvg->tViewBox.fH = fH;
		pSvg->bHasViewBox = 1;
	}
	pSvg->fWidth = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, pSvg->bHasViewBox ? pSvg->tViewBox.fW : 0.0f);
	pSvg->fHeight = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, pSvg->bHasViewBox ? pSvg->tViewBox.fH : 0.0f);
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "preserveAspectRatio", sValue, sizeof(sValue)) ) {
		xgeSvgSetPreserveAspectRatio(pSvg, sValue);
	}
}

static void __xgeSvgApplyNestedSvgViewport(xge_svg pSvg, xge_svg_style_t* pStyle, const char* pTag, const char* pTagEnd)
{
	char sValue[XGE_SVG_ATTR_MAX];
	xge_rect_t tViewBox;
	xge_rect_t tDst;
	xge_rect_t tViewport;
	xge_shape_ex_matrix_t tViewportMatrix;
	int bHasViewBox;
	int iAlignX;
	int iAlignY;
	int iMeetOrSlice;

	if ( !__xgeSvgValid(pSvg) || (pStyle == NULL) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return;
	}
	memset(&tViewBox, 0, sizeof(tViewBox));
	bHasViewBox = 0;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "viewBox", sValue, sizeof(sValue)) &&
	     __xgeSvgParseFourFloats(sValue, &tViewBox.fX, &tViewBox.fY, &tViewBox.fW, &tViewBox.fH) &&
	     (tViewBox.fW > 0.0f) && (tViewBox.fH > 0.0f) ) {
		bHasViewBox = 1;
	}
	tDst.fX = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, 0.0f);
	tDst.fY = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, 0.0f);
	tDst.fW = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, bHasViewBox ? tViewBox.fW : 0.0f);
	tDst.fH = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, bHasViewBox ? tViewBox.fH : 0.0f);
	if ( bHasViewBox && (tDst.fW > 0.0f) && (tDst.fH > 0.0f) ) {
		iAlignX = XGE_SVG_ASPECT_ALIGN_MID;
		iAlignY = XGE_SVG_ASPECT_ALIGN_MID;
		iMeetOrSlice = XGE_SVG_ASPECT_MEET;
		if ( __xgeSvgAttrCopy(pTag, pTagEnd, "preserveAspectRatio", sValue, sizeof(sValue)) ) {
			__xgeSvgParsePreserveAspectRatioFields(sValue, &iAlignX, &iAlignY, &iMeetOrSlice);
		}
		tViewport = __xgeSvgAspectViewport(tViewBox, tDst, iAlignX, iAlignY, iMeetOrSlice);
		tViewportMatrix = __xgeSvgViewBoxMatrix(tViewBox, tViewport);
		pStyle->tTransform = __xgeShapeExMatrixMul(pStyle->tTransform, tViewportMatrix);
	} else if ( (fabsf(tDst.fX) > 0.0f) || (fabsf(tDst.fY) > 0.0f) ) {
		tViewportMatrix = __xgeSvgMatrixTranslate(tDst.fX, tDst.fY);
		pStyle->tTransform = __xgeShapeExMatrixMul(pStyle->tTransform, tViewportMatrix);
	}
}

static void __xgeSvgParseSymbolMeta(xge_svg pSvg, int iDefIndex, const char* pTag, const char* pTagEnd)
{
	char sValue[XGE_SVG_ATTR_MAX];
	float fX;
	float fY;
	float fW;
	float fH;

	if ( !__xgeSvgValid(pSvg) || (iDefIndex < 0) || (iDefIndex >= pSvg->iDefCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return;
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "viewBox", sValue, sizeof(sValue)) &&
	     __xgeSvgParseFourFloats(sValue, &fX, &fY, &fW, &fH) && (fW > 0.0f) && (fH > 0.0f) ) {
		pSvg->pDefs[iDefIndex].tViewBox.fX = fX;
		pSvg->pDefs[iDefIndex].tViewBox.fY = fY;
		pSvg->pDefs[iDefIndex].tViewBox.fW = fW;
		pSvg->pDefs[iDefIndex].tViewBox.fH = fH;
		pSvg->pDefs[iDefIndex].bHasViewBox = 1;
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "preserveAspectRatio", sValue, sizeof(sValue)) ) {
		__xgeSvgParsePreserveAspectRatioFields(sValue,
			&pSvg->pDefs[iDefIndex].iAspectAlignX,
			&pSvg->pDefs[iDefIndex].iAspectAlignY,
			&pSvg->pDefs[iDefIndex].iAspectMeetOrSlice);
	}
}

static int __xgeSvgParseClipPath(xge_svg pSvg, const char* pTag, const char* pTagEnd, int* pClipIndex)
{
	char sId[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	int iIndex;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pClipIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pClipIndex = -1;
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "id", sId, sizeof(sId)) || (sId[0] == '\0') ) {
		return XGE_OK;
	}
	iRet = __xgeSvgClipPathGetOrCreate(pSvg, sId, &iIndex);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	memset(&pSvg->pClipPaths[iIndex].tRect, 0, sizeof(pSvg->pClipPaths[iIndex].tRect));
	pSvg->pClipPaths[iIndex].iUnits = XGE_SVG_CLIP_USER_SPACE;
	pSvg->pClipPaths[iIndex].bHasRect = 0;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "clipPathUnits", sValue, sizeof(sValue)) &&
	     (strcmp(sValue, "objectBoundingBox") == 0) ) {
		pSvg->pClipPaths[iIndex].iUnits = XGE_SVG_CLIP_OBJECT_BOUNDING_BOX;
	}
	*pClipIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgParseClipRect(xge_svg pSvg, int iClipIndex, const char* pTag, const char* pTagEnd, xge_shape_ex_matrix_t tParentTransform)
{
	xge_rect_t tRect;
	char sValue[XGE_SVG_ATTR_MAX];
	xge_shape_ex_matrix_t tTransform;

	if ( !__xgeSvgValid(pSvg) || (iClipIndex < 0) || (iClipIndex >= pSvg->iClipPathCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeSvgTagNameEquals(pTag, "rect") ) {
		return XGE_OK;
	}
	if ( pSvg->pClipPaths[iClipIndex].iUnits == XGE_SVG_CLIP_OBJECT_BOUNDING_BOX ) {
		tRect.fX = __xgeSvgAttrLengthRef(pTag, pTagEnd, "x", 1.0f, 0.0f);
		tRect.fY = __xgeSvgAttrLengthRef(pTag, pTagEnd, "y", 1.0f, 0.0f);
		tRect.fW = __xgeSvgAttrLengthRef(pTag, pTagEnd, "width", 1.0f, 0.0f);
		tRect.fH = __xgeSvgAttrLengthRef(pTag, pTagEnd, "height", 1.0f, 0.0f);
	} else {
		tRect.fX = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, 0.0f);
		tRect.fY = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, 0.0f);
		tRect.fW = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, 0.0f);
		tRect.fH = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, 0.0f);
	}
	if ( tRect.fW < 0.0f ) tRect.fW = 0.0f;
	if ( tRect.fH < 0.0f ) tRect.fH = 0.0f;
	tTransform = tParentTransform;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "transform", sValue, sizeof(sValue)) ) {
		xge_shape_ex_matrix_t tLocalTransform;

		if ( __xgeSvgParseTransform(sValue, &tLocalTransform) ) {
			tTransform = __xgeShapeExMatrixMul(tTransform, tLocalTransform);
		}
	}
	tRect = __xgeSvgMatrixRectBounds(tTransform, tRect);
	if ( pSvg->pClipPaths[iClipIndex].bHasRect ) {
		xge_rect_t tOld = pSvg->pClipPaths[iClipIndex].tRect;
		float fLeft = fminf(tOld.fX, tRect.fX);
		float fTop = fminf(tOld.fY, tRect.fY);
		float fRight = fmaxf(tOld.fX + tOld.fW, tRect.fX + tRect.fW);
		float fBottom = fmaxf(tOld.fY + tOld.fH, tRect.fY + tRect.fH);

		tRect.fX = fLeft;
		tRect.fY = fTop;
		tRect.fW = fRight - fLeft;
		tRect.fH = fBottom - fTop;
	}
	pSvg->pClipPaths[iClipIndex].tRect = tRect;
	pSvg->pClipPaths[iClipIndex].bHasRect = 1;
	return XGE_OK;
}

static float __xgeSvgMaskRectOpacity(const char* pTag, const char* pTagEnd)
{
	char sStyle[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	uint32_t iColor;
	float fOpacity;
	float fFillOpacity;
	float fLuma;

	sStyle[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "style", sStyle, sizeof(sStyle));
	iColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	fOpacity = 1.0f;
	fFillOpacity = 1.0f;
	if ( __xgeSvgAttrOrStyleCopy(pTag, pTagEnd, sStyle, "fill", sValue, sizeof(sValue)) ) {
		__xgeSvgParseColor(sValue, &iColor);
	}
	if ( __xgeSvgAttrOrStyleCopy(pTag, pTagEnd, sStyle, "opacity", sValue, sizeof(sValue)) ) {
		__xgeSvgParseOpacity(sValue, &fOpacity);
	}
	if ( __xgeSvgAttrOrStyleCopy(pTag, pTagEnd, sStyle, "fill-opacity", sValue, sizeof(sValue)) ) {
		__xgeSvgParseOpacity(sValue, &fFillOpacity);
	}
	fLuma = (0.2126f * (float)XGE_COLOR_GET_R(iColor) + 0.7152f * (float)XGE_COLOR_GET_G(iColor) + 0.0722f * (float)XGE_COLOR_GET_B(iColor)) / 255.0f;
	fOpacity *= fFillOpacity * ((float)XGE_COLOR_GET_A(iColor) / 255.0f) * fLuma;
	if ( fOpacity < 0.0f ) fOpacity = 0.0f;
	if ( fOpacity > 1.0f ) fOpacity = 1.0f;
	return fOpacity;
}

static int __xgeSvgParseMask(xge_svg pSvg, const char* pTag, const char* pTagEnd, int* pMaskIndex)
{
	char sId[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	int iIndex;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pMaskIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pMaskIndex = -1;
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "id", sId, sizeof(sId)) || (sId[0] == '\0') ) {
		return XGE_OK;
	}
	iRet = __xgeSvgMaskGetOrCreate(pSvg, sId, &iIndex);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xrtFree(pSvg->pMasks[iIndex].pRects);
	pSvg->pMasks[iIndex].pRects = NULL;
	pSvg->pMasks[iIndex].iRectCount = 0;
	pSvg->pMasks[iIndex].iRectCapacity = 0;
	pSvg->pMasks[iIndex].iContentUnits = XGE_SVG_MASK_USER_SPACE;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "maskContentUnits", sValue, sizeof(sValue)) &&
	     (strcmp(sValue, "objectBoundingBox") == 0) ) {
		pSvg->pMasks[iIndex].iContentUnits = XGE_SVG_MASK_OBJECT_BOUNDING_BOX;
	}
	*pMaskIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgParseMaskRect(xge_svg pSvg, int iMaskIndex, const char* pTag, const char* pTagEnd, xge_shape_ex_matrix_t tParentTransform)
{
	xge_svg_mask_rect_t tMaskRect;
	char sValue[XGE_SVG_ATTR_MAX];
	xge_shape_ex_matrix_t tTransform;

	if ( !__xgeSvgValid(pSvg) || (iMaskIndex < 0) || (iMaskIndex >= pSvg->iMaskCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeSvgTagNameEquals(pTag, "rect") ) {
		return XGE_OK;
	}
	if ( pSvg->pMasks[iMaskIndex].iContentUnits == XGE_SVG_MASK_OBJECT_BOUNDING_BOX ) {
		tMaskRect.tRect.fX = __xgeSvgAttrLengthRef(pTag, pTagEnd, "x", 1.0f, 0.0f);
		tMaskRect.tRect.fY = __xgeSvgAttrLengthRef(pTag, pTagEnd, "y", 1.0f, 0.0f);
		tMaskRect.tRect.fW = __xgeSvgAttrLengthRef(pTag, pTagEnd, "width", 1.0f, 0.0f);
		tMaskRect.tRect.fH = __xgeSvgAttrLengthRef(pTag, pTagEnd, "height", 1.0f, 0.0f);
	} else {
		tMaskRect.tRect.fX = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, 0.0f);
		tMaskRect.tRect.fY = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, 0.0f);
		tMaskRect.tRect.fW = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, 0.0f);
		tMaskRect.tRect.fH = __xgeSvgAttrLength(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, 0.0f);
	}
	if ( tMaskRect.tRect.fW < 0.0f ) tMaskRect.tRect.fW = 0.0f;
	if ( tMaskRect.tRect.fH < 0.0f ) tMaskRect.tRect.fH = 0.0f;
	tTransform = tParentTransform;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "transform", sValue, sizeof(sValue)) ) {
		xge_shape_ex_matrix_t tLocalTransform;

		if ( __xgeSvgParseTransform(sValue, &tLocalTransform) ) {
			tTransform = __xgeShapeExMatrixMul(tTransform, tLocalTransform);
		}
	}
	tMaskRect.tRect = __xgeSvgMatrixRectBounds(tTransform, tMaskRect.tRect);
	tMaskRect.fOpacity = __xgeSvgMaskRectOpacity(pTag, pTagEnd);
	return __xgeSvgMaskAddRect(&pSvg->pMasks[iMaskIndex], tMaskRect);
}

int xgeSvgCreate(xge_svg* ppSvg)
{
	xge_svg pSvg;
	int iRet;

	if ( ppSvg == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pSvg = (xge_svg)xrtCalloc(1, sizeof(*pSvg));
	if ( pSvg == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeShapeExSceneCreate(&pSvg->pScene);
	if ( iRet != XGE_OK ) {
		xrtFree(pSvg);
		return iRet;
	}
	pSvg->iMagic = XGE_SVG_MAGIC;
	pSvg->iRefCount = 1;
	pSvg->iAspectAlignX = XGE_SVG_ASPECT_ALIGN_MID;
	pSvg->iAspectAlignY = XGE_SVG_ASPECT_ALIGN_MID;
	pSvg->iAspectMeetOrSlice = XGE_SVG_ASPECT_MEET;
	*ppSvg = pSvg;
	return XGE_OK;
}

void xgeSvgDestroy(xge_svg pSvg)
{
	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	pSvg->iRefCount--;
	if ( pSvg->iRefCount > 0 ) {
		return;
	}
	__xgeSvgDefsClear(pSvg);
	__xgeSvgClipPathsClear(pSvg);
	__xgeSvgMasksClear(pSvg);
	__xgeSvgLinearGradientsClear(pSvg);
	__xgeSvgRadialGradientsClear(pSvg);
	__xgeSvgStyleRulesClear(pSvg);
	__xgeSvgPendingUsesClear(pSvg);
	pSvg->iMagic = 0;
	xgeShapeExSceneDestroy(pSvg->pScene);
	xrtFree(pSvg);
}

int xgeSvgAddRef(xge_svg pSvg)
{
	if ( !__xgeSvgValid(pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pSvg->iRefCount++;
	return XGE_OK;
}

int xgeSvgClear(xge_svg pSvg)
{
	if ( !__xgeSvgValid(pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeShapeExSceneClear(pSvg->pScene);
	__xgeSvgDefsClear(pSvg);
	__xgeSvgClipPathsClear(pSvg);
	__xgeSvgMasksClear(pSvg);
	__xgeSvgLinearGradientsClear(pSvg);
	__xgeSvgRadialGradientsClear(pSvg);
	__xgeSvgStyleRulesClear(pSvg);
	__xgeSvgPendingUsesClear(pSvg);
	pSvg->bHasViewBox = 0;
	pSvg->fWidth = 0.0f;
	pSvg->fHeight = 0.0f;
	pSvg->tViewBox.fX = 0.0f;
	pSvg->tViewBox.fY = 0.0f;
	pSvg->tViewBox.fW = 0.0f;
	pSvg->tViewBox.fH = 0.0f;
	pSvg->iAspectAlignX = XGE_SVG_ASPECT_ALIGN_MID;
	pSvg->iAspectAlignY = XGE_SVG_ASPECT_ALIGN_MID;
	pSvg->iAspectMeetOrSlice = XGE_SVG_ASPECT_MEET;
	return XGE_OK;
}

int xgeSvgLoadMemory(xge_svg pSvg, const void* pData, int iSize)
{
	char* sText;
	const char* p;
	xge_svg_style_t arrStack[XGE_SVG_STACK_MAX];
	int arrDefStack[XGE_SVG_STACK_MAX];
	int iStackTop;
	int iDefsDepth;
	int iSkipDepth;
	int iLinearGradientIndex;
	int iRadialGradientIndex;
	int iClipPathIndex;
	int iClipTransformTop;
	xge_shape_ex_matrix_t arrClipTransformStack[XGE_SVG_STACK_MAX];
	int iMaskIndex;
	int iMaskTransformTop;
	xge_shape_ex_matrix_t arrMaskTransformStack[XGE_SVG_STACK_MAX];
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pData == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = xgeSvgClear(pSvg);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	sText = (char*)xrtMalloc((size_t)iSize + 1u);
	if ( sText == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(sText, pData, (size_t)iSize);
	sText[iSize] = '\0';
	arrStack[0] = __xgeSvgStyleDefault();
	arrDefStack[0] = -1;
	iStackTop = 0;
	iDefsDepth = 0;
	iSkipDepth = 0;
	iLinearGradientIndex = -1;
	iRadialGradientIndex = -1;
	iClipPathIndex = -1;
	iClipTransformTop = 0;
	arrClipTransformStack[0] = __xgeShapeExMatrixIdentity();
	iMaskIndex = -1;
	iMaskTransformTop = 0;
	arrMaskTransformStack[0] = __xgeShapeExMatrixIdentity();
	iRet = __xgeSvgParseStyleBlocks(pSvg, sText);
	if ( iRet != XGE_OK ) {
		xrtFree(sText);
		return iRet;
	}
	p = sText;
	while ( (p = strchr(p, '<')) != NULL ) {
		const char* pTagEnd = __xgeSvgFindTagEnd(p);
		int bSelfClosing;
		int bSkipTag;
		if ( pTagEnd == NULL ) {
			break;
		}
		bSelfClosing = __xgeSvgIsSelfClosingTag(p, pTagEnd);
		bSkipTag = __xgeSvgTagNameEquals(p, "pattern") ||
		           __xgeSvgTagNameEquals(p, "style");
		if ( __xgeSvgIsCloseTagName(p, "clipPath") ) {
			iClipPathIndex = -1;
			iClipTransformTop = 0;
			arrClipTransformStack[0] = __xgeShapeExMatrixIdentity();
			p = pTagEnd + 1;
			continue;
		}
		if ( (iClipPathIndex >= 0) && __xgeSvgIsCloseTagName(p, "g") ) {
			if ( iClipTransformTop > 0 ) {
				iClipTransformTop--;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgIsCloseTagName(p, "mask") ) {
			iMaskIndex = -1;
			iMaskTransformTop = 0;
			arrMaskTransformStack[0] = __xgeShapeExMatrixIdentity();
			p = pTagEnd + 1;
			continue;
		}
		if ( (iMaskIndex >= 0) && __xgeSvgIsCloseTagName(p, "g") ) {
			if ( iMaskTransformTop > 0 ) {
				iMaskTransformTop--;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgIsCloseTagName(p, "pattern") ||
		     __xgeSvgIsCloseTagName(p, "style") ) {
			if ( iSkipDepth > 0 ) {
				iSkipDepth--;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgIsCloseTagName(p, "linearGradient") ) {
			iLinearGradientIndex = -1;
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgIsCloseTagName(p, "radialGradient") ) {
			iRadialGradientIndex = -1;
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgIsCloseTagName(p, "defs") ) {
			if ( iDefsDepth > 0 ) {
				iDefsDepth--;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( iMaskIndex >= 0 ) {
			if ( __xgeSvgTagNameEquals(p, "g") ) {
				if ( !bSelfClosing ) {
					xge_shape_ex_matrix_t tTransform = arrMaskTransformStack[iMaskTransformTop];
					char sTransform[XGE_SVG_ATTR_MAX];

					if ( iMaskTransformTop >= (XGE_SVG_STACK_MAX - 1) ) {
						xrtFree(sText);
						return XGE_ERROR_OUT_OF_MEMORY;
					}
					if ( __xgeSvgAttrCopy(p, pTagEnd, "transform", sTransform, sizeof(sTransform)) ) {
						xge_shape_ex_matrix_t tLocalTransform;

						if ( __xgeSvgParseTransform(sTransform, &tLocalTransform) ) {
							tTransform = __xgeShapeExMatrixMul(tTransform, tLocalTransform);
						}
					}
					arrMaskTransformStack[++iMaskTransformTop] = tTransform;
				}
				p = pTagEnd + 1;
				continue;
			}
			iRet = __xgeSvgParseMaskRect(pSvg, iMaskIndex, p, pTagEnd, arrMaskTransformStack[iMaskTransformTop]);
			if ( iRet != XGE_OK ) {
				xrtFree(sText);
				return iRet;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( iClipPathIndex >= 0 ) {
			if ( __xgeSvgTagNameEquals(p, "g") ) {
				if ( !bSelfClosing ) {
					xge_shape_ex_matrix_t tTransform = arrClipTransformStack[iClipTransformTop];
					char sTransform[XGE_SVG_ATTR_MAX];

					if ( iClipTransformTop >= (XGE_SVG_STACK_MAX - 1) ) {
						xrtFree(sText);
						return XGE_ERROR_OUT_OF_MEMORY;
					}
					if ( __xgeSvgAttrCopy(p, pTagEnd, "transform", sTransform, sizeof(sTransform)) ) {
						xge_shape_ex_matrix_t tLocalTransform;

						if ( __xgeSvgParseTransform(sTransform, &tLocalTransform) ) {
							tTransform = __xgeShapeExMatrixMul(tTransform, tLocalTransform);
						}
					}
					arrClipTransformStack[++iClipTransformTop] = tTransform;
				}
				p = pTagEnd + 1;
				continue;
			}
			iRet = __xgeSvgParseClipRect(pSvg, iClipPathIndex, p, pTagEnd, arrClipTransformStack[iClipTransformTop]);
			if ( iRet != XGE_OK ) {
				xrtFree(sText);
				return iRet;
			}
			if ( __xgeSvgTagNameEquals(p, "clipPath") && !bSelfClosing ) {
				iClipPathIndex = -1;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( iSkipDepth > 0 ) {
			if ( bSkipTag && !bSelfClosing ) {
				iSkipDepth++;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( bSkipTag ) {
			if ( !bSelfClosing ) {
				iSkipDepth++;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgTagNameEquals(p, "clipPath") ) {
			iRet = __xgeSvgParseClipPath(pSvg, p, pTagEnd, &iClipPathIndex);
			if ( iRet != XGE_OK ) {
				xrtFree(sText);
				return iRet;
			}
			iClipTransformTop = 0;
			arrClipTransformStack[0] = __xgeShapeExMatrixIdentity();
			if ( bSelfClosing ) {
				iClipPathIndex = -1;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgTagNameEquals(p, "mask") ) {
			iRet = __xgeSvgParseMask(pSvg, p, pTagEnd, &iMaskIndex);
			if ( iRet != XGE_OK ) {
				xrtFree(sText);
				return iRet;
			}
			iMaskTransformTop = 0;
			arrMaskTransformStack[0] = __xgeShapeExMatrixIdentity();
			if ( bSelfClosing ) {
				iMaskIndex = -1;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( iLinearGradientIndex >= 0 ) {
			if ( __xgeSvgTagNameEquals(p, "stop") ) {
				iRet = __xgeSvgParseGradientStop(pSvg, iLinearGradientIndex, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					xrtFree(sText);
					return iRet;
				}
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( iRadialGradientIndex >= 0 ) {
			if ( __xgeSvgTagNameEquals(p, "stop") ) {
				iRet = __xgeSvgParseRadialGradientStop(pSvg, iRadialGradientIndex, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					xrtFree(sText);
					return iRet;
				}
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgTagNameEquals(p, "defs") ) {
			if ( !bSelfClosing ) {
				iDefsDepth++;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgTagNameEquals(p, "linearGradient") ) {
			iRet = __xgeSvgParseLinearGradient(pSvg, p, pTagEnd, &arrStack[iStackTop], &iLinearGradientIndex);
			if ( iRet != XGE_OK ) {
				xrtFree(sText);
				return iRet;
			}
			if ( bSelfClosing ) {
				iLinearGradientIndex = -1;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgTagNameEquals(p, "radialGradient") ) {
			iRet = __xgeSvgParseRadialGradient(pSvg, p, pTagEnd, &arrStack[iStackTop], &iRadialGradientIndex);
			if ( iRet != XGE_OK ) {
				xrtFree(sText);
				return iRet;
			}
			if ( bSelfClosing ) {
				iRadialGradientIndex = -1;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgIsCloseTagName(p, "g") ||
		     __xgeSvgIsCloseTagName(p, "symbol") ||
		     __xgeSvgIsCloseTagName(p, "svg") ) {
			if ( iStackTop > 0 ) {
				iStackTop--;
			}
		} else if ( __xgeSvgTagNameEquals(p, "svg") ) {
			xge_svg_style_t tStyle;
			if ( iStackTop == 0 ) {
				__xgeSvgParseRoot(pSvg, p, pTagEnd);
			}
			tStyle = __xgeSvgStyleResolve(pSvg, &arrStack[iStackTop], p, pTagEnd);
			if ( iStackTop > 0 ) {
				__xgeSvgApplyNestedSvgViewport(pSvg, &tStyle, p, pTagEnd);
			}
			if ( !bSelfClosing ) {
				if ( iStackTop >= (XGE_SVG_STACK_MAX - 1) ) {
					xrtFree(sText);
					return XGE_ERROR_OUT_OF_MEMORY;
				}
				arrStack[++iStackTop] = tStyle;
				arrDefStack[iStackTop] = arrDefStack[iStackTop - 1];
			}
		} else if ( __xgeSvgTagNameEquals(p, "g") || __xgeSvgTagNameEquals(p, "symbol") ) {
			xge_svg_style_t tStyle;
			int iDefIndex = arrDefStack[iStackTop];
			int bCapture = (iDefsDepth > 0) || (iDefIndex >= 0) || __xgeSvgTagNameEquals(p, "symbol");
			char sId[XGE_SVG_ATTR_MAX];
			tStyle = __xgeSvgStyleResolve(pSvg, &arrStack[iStackTop], p, pTagEnd);
			if ( bCapture && __xgeSvgAttrCopy(p, pTagEnd, "id", sId, sizeof(sId)) && (sId[0] != '\0') ) {
				iRet = __xgeSvgDefGetOrCreate(pSvg, sId, &iDefIndex);
				if ( iRet != XGE_OK ) {
					xrtFree(sText);
					return iRet;
				}
				if ( __xgeSvgTagNameEquals(p, "symbol") ) {
					__xgeSvgParseSymbolMeta(pSvg, iDefIndex, p, pTagEnd);
				}
			}
			if ( !bSelfClosing ) {
				if ( iStackTop >= (XGE_SVG_STACK_MAX - 1) ) {
					xrtFree(sText);
					return XGE_ERROR_OUT_OF_MEMORY;
				}
				arrStack[++iStackTop] = tStyle;
				arrDefStack[iStackTop] = iDefIndex;
			}
		} else if ( __xgeSvgTagNameEquals(p, "use") ) {
			iRet = __xgeSvgParseUse(pSvg, p, pTagEnd, &arrStack[iStackTop], (iDefsDepth > 0) || (arrDefStack[iStackTop] >= 0), arrDefStack[iStackTop]);
			if ( iRet != XGE_OK ) {
				xrtFree(sText);
				return iRet;
			}
		} else {
			iRet = __xgeSvgParseElement(pSvg, p, pTagEnd, &arrStack[iStackTop], (iDefsDepth > 0) || (arrDefStack[iStackTop] >= 0), arrDefStack[iStackTop]);
			if ( iRet != XGE_OK ) {
				xrtFree(sText);
				return iRet;
			}
		}
		p = pTagEnd + 1;
	}
	iRet = __xgeSvgResolveGradients(pSvg);
	if ( iRet != XGE_OK ) {
		xrtFree(sText);
		return iRet;
	}
	iRet = __xgeSvgResolvePendingUses(pSvg);
	if ( iRet != XGE_OK ) {
		xrtFree(sText);
		return iRet;
	}
	xrtFree(sText);
	if ( !pSvg->bHasViewBox ) {
		pSvg->tViewBox.fX = 0.0f;
		pSvg->tViewBox.fY = 0.0f;
		pSvg->tViewBox.fW = pSvg->fWidth > 0.0f ? pSvg->fWidth : 100.0f;
		pSvg->tViewBox.fH = pSvg->fHeight > 0.0f ? pSvg->fHeight : 100.0f;
		pSvg->bHasViewBox = 1;
	}
	return XGE_OK;
}

int xgeSvgLoad(xge_svg pSvg, const char* sURI)
{
	size_t iSize;
	void* pData;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (sURI == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iSize = 0;
	pData = __xgeFileGetAll(sURI, &iSize);
	if ( pData == NULL ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	iRet = xgeSvgLoadMemory(pSvg, pData, (int)iSize);
	xrtFree(pData);
	return iRet;
}

int xgeSvgLoadCached(const char* sURI, xge_svg* ppSvg)
{
	xge_svg_cache_entry_t* pEntry;
	xge_svg pSvg;
	int iRet;

	if ( (sURI == NULL) || (ppSvg == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( pEntry = g_xgeSvgCacheHead; pEntry != NULL; pEntry = pEntry->pNext ) {
		if ( strcmp(pEntry->sURI, sURI) == 0 ) {
			xgeSvgAddRef(pEntry->pSvg);
			*ppSvg = pEntry->pSvg;
			return XGE_OK;
		}
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
	pEntry = (xge_svg_cache_entry_t*)xrtCalloc(1, sizeof(*pEntry));
	if ( pEntry == NULL ) {
		xgeSvgDestroy(pSvg);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pEntry->sURI = __xgeStrDup(sURI);
	if ( pEntry->sURI == NULL ) {
		xrtFree(pEntry);
		xgeSvgDestroy(pSvg);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeSvgAddRef(pSvg);
	pEntry->pSvg = pSvg;
	pEntry->pNext = g_xgeSvgCacheHead;
	g_xgeSvgCacheHead = pEntry;
	*ppSvg = pSvg;
	return XGE_OK;
}

int xgeSvgCacheInvalidate(const char* sURI)
{
	xge_svg_cache_entry_t** ppEntry;

	if ( sURI == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	ppEntry = &g_xgeSvgCacheHead;
	while ( *ppEntry != NULL ) {
		xge_svg_cache_entry_t* pEntry = *ppEntry;
		if ( strcmp(pEntry->sURI, sURI) == 0 ) {
			*ppEntry = pEntry->pNext;
			xgeSvgDestroy(pEntry->pSvg);
			xrtFree(pEntry->sURI);
			xrtFree(pEntry);
			return XGE_OK;
		}
		ppEntry = &pEntry->pNext;
	}
	return XGE_ERROR_NOT_FOUND;
}

void xgeSvgCacheClear(void)
{
	xge_svg_cache_entry_t* pEntry = g_xgeSvgCacheHead;

	while ( pEntry != NULL ) {
		xge_svg_cache_entry_t* pNext = pEntry->pNext;
		xgeSvgDestroy(pEntry->pSvg);
		xrtFree(pEntry->sURI);
		xrtFree(pEntry);
		pEntry = pNext;
	}
	g_xgeSvgCacheHead = NULL;
}

int xgeSvgGetViewBox(xge_svg pSvg, xge_rect_t* pViewBox)
{
	if ( !__xgeSvgValid(pSvg) || (pViewBox == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pViewBox = pSvg->tViewBox;
	return pSvg->bHasViewBox ? XGE_OK : XGE_ERROR_NOT_FOUND;
}

int xgeSvgSetPreserveAspectRatio(xge_svg pSvg, const char* sValue)
{
	if ( !__xgeSvgValid(pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeSvgParsePreserveAspectRatioFields(sValue, &pSvg->iAspectAlignX, &pSvg->iAspectAlignY, &pSvg->iAspectMeetOrSlice);
	return XGE_OK;
}

int xgeSvgGetDrawViewport(xge_svg pSvg, xge_rect_t tDst, xge_rect_t* pViewport)
{
	xge_rect_t tViewBox;

	if ( !__xgeSvgValid(pSvg) || (pViewport == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tViewBox = pSvg->tViewBox;
	*pViewport = __xgeSvgAspectViewport(tViewBox, tDst, pSvg->iAspectAlignX, pSvg->iAspectAlignY, pSvg->iAspectMeetOrSlice);
	return XGE_OK;
}

static int __xgeSvgDrawInternal(xge_svg pSvg, xge_rect_t tDst, float fTolerance, int bScreenSpace)
{
	xge_shape_ex_matrix_t tMatrix;
	xge_rect_t tViewport;
	xge_rect_t tViewBox;
	int iRet;

	if ( !__xgeSvgValid(pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tViewBox = pSvg->tViewBox;
	if ( (tViewBox.fW <= 0.0f) || (tViewBox.fH <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = xgeSvgGetDrawViewport(pSvg, tDst, &tViewport);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tMatrix = __xgeSvgViewBoxMatrix(tViewBox, tViewport);
	return __xgeShapeExSceneDrawInternal(pSvg->pScene, fTolerance, bScreenSpace, tMatrix, 1.0f);
}

int xgeSvgDraw(xge_svg pSvg, xge_rect_t tDst, float fTolerance)
{
	return __xgeSvgDrawInternal(pSvg, tDst, fTolerance, 0);
}

int xgeSvgDrawPx(xge_svg pSvg, xge_rect_t tDst, float fTolerance)
{
	return __xgeSvgDrawInternal(pSvg, tDst, fTolerance, 1);
}

int xgeSvgRasterizeMemory(const void* pData, int iSize, int iWidth, int iHeight, void* pPixels, int iStride)
{
	xge_svg pSvg;
	xge_render_target_t tTarget;
	xge_pass_t tPass;
	xge_rect_t tDst;
	int bStarted;
	int iRet;

	if ( (pData == NULL) || (iSize <= 0) || (iWidth <= 0) || (iHeight <= 0) || (pPixels == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iWidth > (INT32_MAX / 4) || iHeight > (INT32_MAX / (iWidth * 4)) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( iStride <= 0 ) {
		iStride = iWidth * 4;
	}
	if ( iStride < (iWidth * 4) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}

	pSvg = NULL;
	memset(&tTarget, 0, sizeof(tTarget));
	memset(&tPass, 0, sizeof(tPass));
	bStarted = 0;
	iRet = xgeSvgCreate(&pSvg);
	if ( iRet == XGE_OK ) {
		iRet = xgeSvgLoadMemory(pSvg, pData, iSize);
	}
	if ( iRet == XGE_OK ) {
		iRet = xgeRenderTargetCreate(&tTarget, iWidth, iHeight);
	}
	if ( iRet == XGE_OK ) {
		if ( g_xge.bRenderActive == 0 ) {
			iRet = xgeBegin();
			if ( iRet == XGE_OK ) {
				bStarted = 1;
			}
		}
	}
	if ( iRet == XGE_OK ) {
		xgePassInit(&tPass, &tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(0, 0, 0, 0));
		iRet = xgePassBegin(&tPass);
	}
	if ( iRet == XGE_OK ) {
		tDst.fX = 0.0f;
		tDst.fY = 0.0f;
		tDst.fW = (float)iWidth;
		tDst.fH = (float)iHeight;
		iRet = xgeSvgDrawPx(pSvg, tDst, 0.5f);
	}
	if ( tPass.bActive ) {
		int iEndRet = xgePassEnd(&tPass);
		if ( iRet == XGE_OK ) {
			iRet = iEndRet;
		}
	}
	if ( iRet == XGE_OK ) {
		iRet = xgeRenderTargetReadPixels(&tTarget, pPixels, iStride);
	}
	if ( bStarted != 0 ) {
		int iEndRet = xgeEnd();
		if ( iRet == XGE_OK ) {
			iRet = iEndRet;
		}
	}
	xgeRenderTargetFree(&tTarget);
	xgeSvgDestroy(pSvg);
	return iRet;
}

int xgeSvgTextureLoad(xge_texture pTexture, const char* sURI, int iWidth, int iHeight)
{
	xge_resource_t tResource;
	int iRet;

	if ( (pTexture == NULL) || (sURI == NULL) || (iWidth <= 0) || (iHeight <= 0) ) {
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

int xgeSvgTextureLoadMemory(xge_texture pTexture, const void* pData, int iSize, int iWidth, int iHeight)
{
	unsigned char* pPixels;
	int iStride;
	int iRet;

	if ( (pTexture == NULL) || (pData == NULL) || (iSize <= 0) || (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iWidth > (INT32_MAX / 4) || iHeight > (INT32_MAX / (iWidth * 4)) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iStride = iWidth * 4;
	pPixels = (unsigned char*)xrtMalloc((size_t)iStride * (size_t)iHeight);
	if ( pPixels == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeSvgRasterizeMemory(pData, iSize, iWidth, iHeight, pPixels, iStride);
	if ( iRet == XGE_OK ) {
		iRet = xgeTextureCreateRGBA(pTexture, iWidth, iHeight, pPixels);
	}
	xrtFree(pPixels);
	return iRet;
}
