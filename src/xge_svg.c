#define XGE_SVG_MAGIC 0x58475356u
#define XGE_SVG_ATTR_MAX 4096
#define XGE_SVG_STACK_MAX 128
#define XGE_SVG_DASH_MAX 32
#define XGE_SVG_ID_MAX 128
#define XGE_SVG_STYLE_SELECTOR_TAG 0
#define XGE_SVG_STYLE_SELECTOR_CLASS 1
#define XGE_SVG_STYLE_SELECTOR_ID 2
#define XGE_SVG_STYLE_SELECTOR_SIMPLE 3
#define XGE_SVG_SELECTOR_COMBINATOR_CHILD 1
#define XGE_SVG_SELECTOR_COMBINATOR_DESCENDANT 2
#define XGE_SVG_SELECTOR_COMBINATOR_ADJACENT 3
#define XGE_SVG_SELECTOR_COMBINATOR_GENERAL_SIBLING 4
#define XGE_SVG_CLIP_USER_SPACE 0
#define XGE_SVG_CLIP_OBJECT_BOUNDING_BOX 1
#define XGE_SVG_MASK_USER_SPACE 0
#define XGE_SVG_MASK_OBJECT_BOUNDING_BOX 1
#define XGE_SVG_PATTERN_USER_SPACE 0
#define XGE_SVG_PATTERN_OBJECT_BOUNDING_BOX 1
#define XGE_SVG_MASK_TYPE_LUMINANCE 0
#define XGE_SVG_MASK_TYPE_ALPHA 1
#define XGE_SVG_FILTER_UNITS_USER_SPACE 0
#define XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX 1
#define XGE_SVG_MARKER_UNITS_STROKE_WIDTH 0
#define XGE_SVG_MARKER_UNITS_USER_SPACE 1
#define XGE_SVG_LENGTH_BASIS_X 0
#define XGE_SVG_LENGTH_BASIS_Y 1
#define XGE_SVG_LENGTH_BASIS_OTHER 2
#define XGE_SVG_STYLE_IMPORTANCE_ANY -1
#define XGE_SVG_STYLE_IMPORTANCE_NORMAL 0
#define XGE_SVG_STYLE_IMPORTANCE_IMPORTANT 1
#define XGE_SVG_STYLE_APPLY_ALL 0
#define XGE_SVG_STYLE_APPLY_FONT_SIZE 1
#define XGE_SVG_STYLE_APPLY_EXCEPT_FONT_SIZE 2
#define XGE_SVG_TEXT_ANCHOR_START 0
#define XGE_SVG_TEXT_ANCHOR_MIDDLE 1
#define XGE_SVG_TEXT_ANCHOR_END 2
#define XGE_SVG_TEXT_SPACE_DEFAULT 0
#define XGE_SVG_TEXT_SPACE_PRESERVE 1
#define XGE_SVG_TEXT_BASELINE_ALPHABETIC 0
#define XGE_SVG_TEXT_BASELINE_MIDDLE 1
#define XGE_SVG_TEXT_BASELINE_HANGING 2
#define XGE_SVG_TEXT_BASELINE_TEXT_BEFORE_EDGE 3
#define XGE_SVG_TEXT_BASELINE_TEXT_AFTER_EDGE 4
#define XGE_SVG_TEXT_DECORATION_UNDERLINE		0x00000001
#define XGE_SVG_TEXT_DECORATION_OVERLINE		0x00000002
#define XGE_SVG_TEXT_DECORATION_LINE_THROUGH	0x00000004
#define XGE_SVG_TEXT_LENGTH_ADJUST_SPACING 0
#define XGE_SVG_TEXT_LENGTH_ADJUST_SPACING_AND_GLYPHS 1
#define XGE_SVG_MARKER_POSITION_START 0
#define XGE_SVG_MARKER_POSITION_MID 1
#define XGE_SVG_MARKER_POSITION_END 2
#define XGE_SVG_PAINT_FILL 0
#define XGE_SVG_PAINT_STROKE 1
#define XGE_SVG_PAINT_MARKERS 2
#define XGE_SVG_PAINT_COMPONENT_COUNT 3
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
#define XGE_SVG_PATTERN_HAS_X				0x00000001
#define XGE_SVG_PATTERN_HAS_Y				0x00000002
#define XGE_SVG_PATTERN_HAS_WIDTH			0x00000004
#define XGE_SVG_PATTERN_HAS_HEIGHT			0x00000008
#define XGE_SVG_PATTERN_HAS_UNITS			0x00000010
#define XGE_SVG_PATTERN_HAS_CONTENT_UNITS	0x00000020
#define XGE_SVG_PATTERN_HAS_TRANSFORM		0x00000040
#define XGE_SVG_PATTERN_HAS_VIEWBOX			0x00000080
#define XGE_SVG_PATTERN_HAS_ASPECT			0x00000100
#define XGE_SVG_PATTERN_RESOLVED			0x00000200
#define XGE_SVG_DRAW_ITEM_SHAPE 1
#define XGE_SVG_DRAW_ITEM_TEXT 2
#define XGE_SVG_DRAW_ITEM_SVG_IMAGE 3
#define XGE_SVG_DRAW_ITEM_RASTER_IMAGE 4
#define XGE_SVG_FLOAT_ABS_MAX 3.40282346638528860e+38

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
	float fFontSize;
	float fLetterSpacing;
	float fWordSpacing;
	float fDashPattern[XGE_SVG_DASH_MAX];
	float fDashOffset;
	char sClipId[XGE_SVG_ID_MAX];
	char sMaskId[XGE_SVG_ID_MAX];
	char sFilterId[XGE_SVG_ID_MAX];
	char sMarkerStartId[XGE_SVG_ID_MAX];
	char sMarkerMidId[XGE_SVG_ID_MAX];
	char sMarkerEndId[XGE_SVG_ID_MAX];
	int iDashCount;
	int iFillRule;
	int iClipRule;
	int iLineCap;
	int iLineJoin;
	int iTextAnchor;
	int iTextSpace;
	int iTextBaseline;
	int iTextDecoration;
	int iPaintOrder[XGE_SVG_PAINT_COMPONENT_COUNT];
	int bStrokeFirst;
	int bVisible;
	int bVisibility;
	int bStopColorSet;
	int bStopOpacitySet;
	int bColorSet;
	int bFillSet;
	int bFillCurrentColor;
	int bStrokeSet;
	int bStrokeCurrentColor;
	int bStrokeNonScaling;
	xge_shape_ex_matrix_t tTransform;
} xge_svg_style_t;

typedef struct xge_svg_cache_entry_t {
	char* sURI;
	xge_svg pSvg;
	struct xge_svg_cache_entry_t* pNext;
} xge_svg_cache_entry_t;

typedef struct xge_svg_text_item_t {
	char* sText;
	xge_svg_style_t tStyle;
	xge_shape_ex pTextPath;
	float fX;
	float fY;
	float fAnchorAdvance;
	float fTextPathOffset;
	float fTextScaleX;
	float fTextExtraSpacing;
	int bClipRect;
	xge_rect_t tClipRect;
	xge_shape_ex pMaskShape;
} xge_svg_text_item_t;

typedef struct xge_svg_text_layout_t {
	int bHasTextLength;
	int iLengthAdjust;
	float fTextLength;
	float fScaleX;
	float fExtraSpacing;
	float fNaturalAdvance;
	int iGapCount;
} xge_svg_text_layout_t;

typedef struct xge_svg_image_item_t {
	xge_svg pSvg;
	xge_svg_style_t tStyle;
	xge_rect_t tRect;
	int bClipRect;
	xge_rect_t tClipRect;
	xge_shape_ex pMaskShape;
} xge_svg_image_item_t;

typedef struct xge_svg_raster_image_t {
	int iRefCount;
	xge_image_t tImage;
	xge_texture_t tTexture;
} xge_svg_raster_image_t;

typedef struct xge_svg_raster_item_t {
	xge_svg_raster_image_t* pRaster;
	xge_svg_style_t tStyle;
	xge_rect_t tRect;
	uint32_t iColor;
	int iAspectAlignX;
	int iAspectAlignY;
	int iAspectMeetOrSlice;
	int bClipRect;
	xge_rect_t tClipRect;
	xge_shape_ex pMaskShape;
} xge_svg_raster_item_t;

typedef struct xge_svg_draw_item_t {
	int iType;
	union {
		xge_shape_ex pShape;
		xge_svg_text_item_t tText;
		xge_svg_image_item_t tImage;
		xge_svg_raster_item_t tRaster;
	} u;
} xge_svg_draw_item_t;

typedef struct xge_svg_def_t {
	char* sId;
	xge_shape_ex* pShapes;
	xge_svg_style_t* pShapeStyles;
	xge_svg_text_item_t* pTexts;
	xge_svg_image_item_t* pImages;
	xge_svg_raster_item_t* pRasters;
	xge_rect_t tViewBox;
	int iShapeCount;
	int iShapeCapacity;
	int iTextCount;
	int iTextCapacity;
	int iImageCount;
	int iImageCapacity;
	int iRasterCount;
	int iRasterCapacity;
	int bHasViewBox;
	int iAspectAlignX;
	int iAspectAlignY;
	int iAspectMeetOrSlice;
} xge_svg_def_t;

typedef struct xge_svg_marker_t {
	char* sId;
	int iDefIndex;
	float fRefX;
	float fRefY;
	float fWidth;
	float fHeight;
	float fOrientDegrees;
	int bOrientAuto;
	int bOrientAutoStartReverse;
	int iUnits;
} xge_svg_marker_t;

typedef struct xge_svg_pattern_t {
	char* sId;
	char* sHrefId;
	int iDefIndex;
	xge_rect_t tRect;
	int iUnits;
	int iContentUnits;
	xge_shape_ex_matrix_t tTransform;
	int iFlags;
} xge_svg_pattern_t;

typedef struct xge_svg_clip_path_t {
	char* sId;
	xge_rect_t* pRects;
	xge_shape_ex* pShapes;
	int iUnits;
	int iRectCount;
	int iRectCapacity;
	int iShapeCount;
	int iShapeCapacity;
	int bHasRect;
	int bHasShape;
} xge_svg_clip_path_t;

typedef struct xge_svg_mask_rect_t {
	xge_rect_t tRect;
	float fOpacity;
} xge_svg_mask_rect_t;

typedef struct xge_svg_mask_shape_t {
	xge_shape_ex pShape;
	float fOpacity;
} xge_svg_mask_shape_t;

typedef struct xge_svg_mask_t {
	char* sId;
	xge_svg_mask_rect_t* pRects;
	xge_svg_mask_shape_t* pShapes;
	xge_rect_t tRegion;
	int iRectCount;
	int iRectCapacity;
	int iShapeCount;
	int iShapeCapacity;
	int iUnits;
	int iContentUnits;
	int iMaskType;
} xge_svg_mask_t;

typedef struct xge_svg_filter_t {
	char* sId;
	xge_rect_t tRegion;
	float fOffsetX;
	float fOffsetY;
	float fDropShadowX;
	float fDropShadowY;
	float fDropShadowStdDeviation;
	float fDropShadowOpacity;
	uint32_t iDropShadowColor;
	int iFilterUnits;
	int iUnits;
	int bHasOffset;
	int bHasDropShadow;
} xge_svg_filter_t;

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

typedef struct xge_svg_element_stack_item_t {
	const char* pTag;
	const char* pTagEnd;
	int iNodeIndex;
} xge_svg_element_stack_item_t;

typedef struct xge_svg_element_node_t {
	const char* pTag;
	const char* pTagEnd;
	int iParentIndex;
	int iPrevSiblingIndex;
	int iNextSiblingIndex;
	int iChildCount;
	int bHasTextContent;
} xge_svg_element_node_t;

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
	int iMainIdDefIndex;
	int iExtraMainIdDefIndex;
	int bResolved;
} xge_svg_pending_use_t;

typedef struct xge_svg_font_cache_t {
	float fSize;
	xge_font_t tFont;
} xge_svg_font_cache_t;

struct xge_svg_t {
	uint32_t iMagic;
	int iRefCount;
	xge_shape_ex_scene pScene;
	xge_svg_draw_item_t* pItems;
	int iItemCount;
	int iItemCapacity;
	xge_svg_def_t* pDefs;
	int iDefCount;
	int iDefCapacity;
	xge_svg_marker_t* pMarkers;
	int iMarkerCount;
	int iMarkerCapacity;
	xge_svg_pattern_t* pPatterns;
	int iPatternCount;
	int iPatternCapacity;
	xge_svg_clip_path_t* pClipPaths;
	int iClipPathCount;
	int iClipPathCapacity;
	xge_svg_mask_t* pMasks;
	int iMaskCount;
	int iMaskCapacity;
	xge_svg_filter_t* pFilters;
	int iFilterCount;
	int iFilterCapacity;
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
	xge_svg_font_cache_t* pFontCaches;
	int iFontCacheCount;
	int iFontCacheCapacity;
	xge_rect_t tViewBox;
	int bHasViewBox;
	float fWidth;
	float fHeight;
	int iAspectAlignX;
	int iAspectAlignY;
	int iAspectMeetOrSlice;
	char* sBaseDir;
	xge_svg_element_node_t* pElementNodes;
	int iElementNodeCount;
	int iElementNodeCapacity;
	int iCurrentElementNode;
	int arrLastChildByDepth[XGE_SVG_STACK_MAX];
	xge_svg_element_stack_item_t arrElementStack[XGE_SVG_STACK_MAX];
	int iElementStackTop;
};

typedef int (*xge_svg_marker_apply_proc)(xge_svg pSvg, const xge_svg_style_t* pStyle, int iTargetDef, void* pUser);

static xge_svg_cache_entry_t* g_xgeSvgCacheHead;

static int __xgeSvgTagNameEquals(const char* pTag, const char* sName);
static int __xgeSvgLoadMemoryEx(xge_svg pSvg, const void* pData, int iSize, const char* sBaseDir);
static int __xgeSvgRasterizeMemoryEx(const void* pData, int iSize, int iWidth, int iHeight, void* pPixels, int iStride, const char* sBaseDir);
static int __xgeSvgTextureLoadMemoryEx(xge_texture pTexture, const void* pData, int iSize, int iWidth, int iHeight, const char* sBaseDir);
static int __xgeSvgUriHasExt(const char* sURI, const char* sExt);
static int __xgeSvgColorNameEquals(const char* sText, const char* sName);
static int __xgeSvgParseColor(const char* sText, uint32_t* pColor);
static void __xgeSvgSkipSpaces(const char** ppText);
static float __xgeSvgMaskStyleOpacity(const xge_svg_style_t* pStyle, int iMaskType);
static xge_rect_t __xgeSvgObjectRectToBounds(xge_rect_t tRect, xge_rect_t tBounds);
static xge_shape_ex_matrix_t __xgeSvgMatrixIdentity(void);
static xge_shape_ex_matrix_t __xgeSvgMatrixMul(xge_shape_ex_matrix_t tParent, xge_shape_ex_matrix_t tLocal);
static int __xgeSvgMatrixInvert(xge_shape_ex_matrix_t tMatrix, xge_shape_ex_matrix_t* pOut);
static xge_vec2_t __xgeSvgMatrixPoint(xge_shape_ex_matrix_t tMatrix, xge_vec2_t tPoint);
static xge_rect_t __xgeSvgMatrixRectBounds(xge_shape_ex_matrix_t tMatrix, xge_rect_t tRect);

static int __xgeSvgValid(xge_svg pSvg)
{
	return (pSvg != NULL) && (pSvg->iMagic == XGE_SVG_MAGIC);
}

static char* __xgeSvgUriBaseDir(const char* sURI)
{
	const char* pLastSlash;
	const char* p;
	size_t iLen;
	char* sBase;

	if ( sURI == NULL ) {
		return NULL;
	}
	pLastSlash = NULL;
	for ( p = sURI; *p != '\0'; p++ ) {
		if ( (*p == '/') || (*p == '\\') ) {
			pLastSlash = p;
		}
	}
	if ( pLastSlash == NULL ) {
		return __xgeStrDup("");
	}
	iLen = (size_t)(pLastSlash - sURI) + 1u;
	sBase = (char*)xrtMalloc(iLen + 1u);
	if ( sBase == NULL ) {
		return NULL;
	}
	memcpy(sBase, sURI, iLen);
	sBase[iLen] = '\0';
	return sBase;
}

static int __xgeSvgUriIsAbsolute(const char* sURI)
{
	if ( (sURI == NULL) || (sURI[0] == '\0') ) {
		return 0;
	}
	if ( __xgeUriSchemeLen(sURI) > 0 ) {
		return 1;
	}
	return xrtPathIsAbs((str)sURI, 0) ? 1 : 0;
}

static int __xgeSvgRangeDupTrimmed(const char* sText, char** ppOut)
{
	const char* pStart;
	const char* pEnd;
	size_t iLen;
	char* sOut;

	if ( (sText == NULL) || (ppOut == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppOut = NULL;
	pStart = sText;
	while ( (*pStart == ' ') || (*pStart == '\t') || (*pStart == '\r') || (*pStart == '\n') || (*pStart == '\f') ) {
		pStart++;
	}
	pEnd = pStart + strlen(pStart);
	while ( (pEnd > pStart) &&
	        ((pEnd[-1] == ' ') || (pEnd[-1] == '\t') || (pEnd[-1] == '\r') || (pEnd[-1] == '\n') || (pEnd[-1] == '\f')) ) {
		pEnd--;
	}
	iLen = (size_t)(pEnd - pStart);
	sOut = (char*)xrtMalloc(iLen + 1u);
	if ( sOut == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(sOut, pStart, iLen);
	sOut[iLen] = '\0';
	*ppOut = sOut;
	return XGE_OK;
}

static char* __xgeSvgResolveHrefUri(xge_svg pSvg, const char* sHref)
{
	char* sTrimmed;
	char* sResolved;
	size_t iBaseLen;
	size_t iHrefLen;
	int iRet;

	if ( (sHref == NULL) || (sHref[0] == '\0') ) {
		return NULL;
	}
	sTrimmed = NULL;
	iRet = __xgeSvgRangeDupTrimmed(sHref, &sTrimmed);
	if ( iRet != XGE_OK ) {
		return NULL;
	}
	if ( (sTrimmed[0] == '\0') || (sTrimmed[0] == '#') || __xgeSvgUriIsAbsolute(sTrimmed) ||
	     !__xgeSvgValid(pSvg) || (pSvg->sBaseDir == NULL) || (pSvg->sBaseDir[0] == '\0') ) {
		return sTrimmed;
	}
	iBaseLen = strlen(pSvg->sBaseDir);
	iHrefLen = strlen(sTrimmed);
	if ( iBaseLen > (SIZE_MAX - iHrefLen - 2u) ) {
		xrtFree(sTrimmed);
		return NULL;
	}
	sResolved = (char*)xrtMalloc(iBaseLen + iHrefLen + 2u);
	if ( sResolved == NULL ) {
		xrtFree(sTrimmed);
		return NULL;
	}
	memcpy(sResolved, pSvg->sBaseDir, iBaseLen);
	if ( (iBaseLen > 0) && (pSvg->sBaseDir[iBaseLen - 1] != '/') && (pSvg->sBaseDir[iBaseLen - 1] != '\\') ) {
		sResolved[iBaseLen++] = '/';
	}
	memcpy(sResolved + iBaseLen, sTrimmed, iHrefLen + 1u);
	xrtFree(sTrimmed);
	return sResolved;
}

static int __xgeSvgBaseDirSet(xge_svg pSvg, const char* sBaseDir)
{
	char* sCopy;

	if ( !__xgeSvgValid(pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sCopy = NULL;
	if ( (sBaseDir != NULL) && (sBaseDir[0] != '\0') ) {
		sCopy = __xgeStrDup(sBaseDir);
		if ( sCopy == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	xrtFree(pSvg->sBaseDir);
	pSvg->sBaseDir = sCopy;
	return XGE_OK;
}

static int __xgeSvgRasterImageCreateFromMemory(const void* pData, int iSize, xge_svg_raster_image_t** ppImage)
{
	xge_svg_raster_image_t* pImage;
	int iRet;

	if ( (pData == NULL) || (iSize <= 0) || (ppImage == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppImage = NULL;
	pImage = (xge_svg_raster_image_t*)xrtCalloc(1, sizeof(*pImage));
	if ( pImage == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeImageLoadMemoryEx(&pImage->tImage, pData, iSize, XGE_IMAGE_PREMULTIPLIED);
	if ( iRet != XGE_OK ) {
		xrtFree(pImage);
		return iRet;
	}
	pImage->iRefCount = 1;
	*ppImage = pImage;
	return XGE_OK;
}

static int __xgeSvgRasterImageAddRef(xge_svg_raster_image_t* pImage)
{
	if ( pImage == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pImage->iRefCount < INT32_MAX ) {
		pImage->iRefCount++;
	}
	return pImage->iRefCount;
}

static void __xgeSvgRasterImageDestroy(xge_svg_raster_image_t* pImage)
{
	if ( pImage == NULL ) {
		return;
	}
	if ( pImage->iRefCount > 1 ) {
		pImage->iRefCount--;
		return;
	}
	xgeTextureFree(&pImage->tTexture);
	xgeImageFree(&pImage->tImage);
	memset(pImage, 0, sizeof(*pImage));
	xrtFree(pImage);
}

static int __xgeSvgRasterImageEnsureTexture(xge_svg_raster_image_t* pImage)
{
	if ( (pImage == NULL) || (pImage->tImage.pPixels == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pImage->tTexture.iRefCount > 0 ) {
		return XGE_OK;
	}
	return xgeTextureCreateFromImage(&pImage->tTexture, &pImage->tImage);
}

static void __xgeSvgDrawItemReset(xge_svg_draw_item_t* pItem)
{
	if ( pItem == NULL ) {
		return;
	}
	if ( pItem->iType == XGE_SVG_DRAW_ITEM_SHAPE ) {
		xgeShapeExDestroy(pItem->u.pShape);
	} else if ( pItem->iType == XGE_SVG_DRAW_ITEM_TEXT ) {
		xrtFree(pItem->u.tText.sText);
		xgeShapeExDestroy(pItem->u.tText.pTextPath);
		xgeShapeExDestroy(pItem->u.tText.pMaskShape);
	} else if ( pItem->iType == XGE_SVG_DRAW_ITEM_SVG_IMAGE ) {
		xgeSvgDestroy(pItem->u.tImage.pSvg);
		xgeShapeExDestroy(pItem->u.tImage.pMaskShape);
	} else if ( pItem->iType == XGE_SVG_DRAW_ITEM_RASTER_IMAGE ) {
		__xgeSvgRasterImageDestroy(pItem->u.tRaster.pRaster);
		xgeShapeExDestroy(pItem->u.tRaster.pMaskShape);
	}
	memset(pItem, 0, sizeof(*pItem));
}

static void __xgeSvgDrawItemsClear(xge_svg pSvg)
{
	int i;

	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	for ( i = 0; i < pSvg->iItemCount; i++ ) {
		__xgeSvgDrawItemReset(&pSvg->pItems[i]);
	}
	xrtFree(pSvg->pItems);
	pSvg->pItems = NULL;
	pSvg->iItemCount = 0;
	pSvg->iItemCapacity = 0;
}

static int __xgeSvgDrawItemReserve(xge_svg pSvg)
{
	xge_svg_draw_item_t* pItems;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pSvg->iItemCount < pSvg->iItemCapacity ) {
		return XGE_OK;
	}
	iCapacity = pSvg->iItemCapacity > 0 ? pSvg->iItemCapacity * 2 : 32;
	pItems = (xge_svg_draw_item_t*)xrtRealloc(pSvg->pItems, (size_t)iCapacity * sizeof(*pItems));
	if ( pItems == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pItems + pSvg->iItemCapacity, 0, (size_t)(iCapacity - pSvg->iItemCapacity) * sizeof(*pItems));
	pSvg->pItems = pItems;
	pSvg->iItemCapacity = iCapacity;
	return XGE_OK;
}

static void __xgeSvgFontCacheClear(xge_svg pSvg)
{
	int i;

	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	for ( i = 0; i < pSvg->iFontCacheCount; i++ ) {
		xgeFontFree(&pSvg->pFontCaches[i].tFont);
	}
	xrtFree(pSvg->pFontCaches);
	pSvg->pFontCaches = NULL;
	pSvg->iFontCacheCount = 0;
	pSvg->iFontCacheCapacity = 0;
}

static float __xgeSvgFontCacheKey(float fSize)
{
	if ( fSize < 0.25f ) {
		return 0.25f;
	}
	return floorf((fSize * 4.0f) + 0.5f) * 0.25f;
}

static int __xgeSvgFontCacheGet(xge_svg pSvg, float fSize, xge_font* ppFont)
{
	xge_svg_font_cache_t* pCaches;
	xge_svg_font_cache_t* pCache;
	float fKey;
	int iCapacity;
	int iRet;
	int i;

	if ( !__xgeSvgValid(pSvg) || (ppFont == NULL) || (fSize <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppFont = NULL;
	fKey = __xgeSvgFontCacheKey(fSize);
	for ( i = 0; i < pSvg->iFontCacheCount; i++ ) {
		if ( fabsf(pSvg->pFontCaches[i].fSize - fKey) <= 0.001f ) {
			*ppFont = &pSvg->pFontCaches[i].tFont;
			return XGE_OK;
		}
	}
	if ( pSvg->iFontCacheCount >= pSvg->iFontCacheCapacity ) {
		iCapacity = pSvg->iFontCacheCapacity > 0 ? pSvg->iFontCacheCapacity * 2 : 4;
		pCaches = (xge_svg_font_cache_t*)xrtRealloc(pSvg->pFontCaches, (size_t)iCapacity * sizeof(*pCaches));
		if ( pCaches == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pSvg->pFontCaches = pCaches;
		pSvg->iFontCacheCapacity = iCapacity;
	}
	pCache = &pSvg->pFontCaches[pSvg->iFontCacheCount];
	memset(pCache, 0, sizeof(*pCache));
	iRet = xgeFontFallbackGet(&pCache->tFont, fKey);
	if ( iRet != XGE_OK ) {
		memset(pCache, 0, sizeof(*pCache));
		return iRet;
	}
	pCache->fSize = fKey;
	pSvg->iFontCacheCount++;
	*ppFont = &pCache->tFont;
	return XGE_OK;
}

static void __xgeSvgElementNodesClear(xge_svg pSvg)
{
	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	xrtFree(pSvg->pElementNodes);
	pSvg->pElementNodes = NULL;
	pSvg->iElementNodeCount = 0;
	pSvg->iElementNodeCapacity = 0;
	pSvg->iCurrentElementNode = -1;
	memset(pSvg->arrLastChildByDepth, 0xff, sizeof(pSvg->arrLastChildByDepth));
}

static int __xgeSvgElementNodeEnsure(xge_svg pSvg, const char* pTag, const char* pTagEnd, int* pNodeIndex)
{
	xge_svg_element_node_t* pNodes;
	xge_svg_element_node_t* pNode;
	int iCapacity;
	int iDepth;
	int iParentIndex;
	int iPrevSiblingIndex;
	int iIndex;
	int i;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pSvg->iCurrentElementNode >= 0) && (pSvg->iCurrentElementNode < pSvg->iElementNodeCount) &&
	     (pSvg->pElementNodes[pSvg->iCurrentElementNode].pTag == pTag) &&
	     (pSvg->pElementNodes[pSvg->iCurrentElementNode].pTagEnd == pTagEnd) ) {
		if ( pNodeIndex != NULL ) {
			*pNodeIndex = pSvg->iCurrentElementNode;
		}
		return XGE_OK;
	}
	for ( i = pSvg->iCurrentElementNode + 1; i < pSvg->iElementNodeCount; i++ ) {
		if ( (pSvg->pElementNodes[i].pTag == pTag) &&
		     (pSvg->pElementNodes[i].pTagEnd == pTagEnd) ) {
			pSvg->iCurrentElementNode = i;
			if ( pNodeIndex != NULL ) {
				*pNodeIndex = i;
			}
			return XGE_OK;
		}
	}
	for ( i = 0; (i < pSvg->iElementNodeCount) && (i <= pSvg->iCurrentElementNode); i++ ) {
		if ( (pSvg->pElementNodes[i].pTag == pTag) &&
		     (pSvg->pElementNodes[i].pTagEnd == pTagEnd) ) {
			pSvg->iCurrentElementNode = i;
			if ( pNodeIndex != NULL ) {
				*pNodeIndex = i;
			}
			return XGE_OK;
		}
	}
	iDepth = pSvg->iElementStackTop;
	if ( iDepth < 0 ) {
		iDepth = 0;
	}
	if ( iDepth >= XGE_SVG_STACK_MAX ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iParentIndex = (pSvg->iElementStackTop > 0) ? pSvg->arrElementStack[pSvg->iElementStackTop - 1].iNodeIndex : -1;
	iPrevSiblingIndex = pSvg->arrLastChildByDepth[iDepth];
	if ( pSvg->iElementNodeCount >= pSvg->iElementNodeCapacity ) {
		iCapacity = pSvg->iElementNodeCapacity > 0 ? pSvg->iElementNodeCapacity * 2 : 128;
		pNodes = (xge_svg_element_node_t*)xrtRealloc(pSvg->pElementNodes, (size_t)iCapacity * sizeof(*pNodes));
		if ( pNodes == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pSvg->pElementNodes = pNodes;
		pSvg->iElementNodeCapacity = iCapacity;
	}
	iIndex = pSvg->iElementNodeCount++;
	pNode = &pSvg->pElementNodes[iIndex];
	pNode->pTag = pTag;
	pNode->pTagEnd = pTagEnd;
	pNode->iParentIndex = iParentIndex;
	pNode->iPrevSiblingIndex = iPrevSiblingIndex;
	pNode->iNextSiblingIndex = -1;
	pNode->iChildCount = 0;
	pNode->bHasTextContent = 0;
	if ( (iParentIndex >= 0) && (iParentIndex < pSvg->iElementNodeCount) ) {
		pSvg->pElementNodes[iParentIndex].iChildCount++;
	}
	if ( (iPrevSiblingIndex >= 0) && (iPrevSiblingIndex < pSvg->iElementNodeCount) ) {
		pSvg->pElementNodes[iPrevSiblingIndex].iNextSiblingIndex = iIndex;
	}
	pSvg->arrLastChildByDepth[iDepth] = iIndex;
	pSvg->iCurrentElementNode = iIndex;
	if ( pNodeIndex != NULL ) {
		*pNodeIndex = iIndex;
	}
	return XGE_OK;
}

static int __xgeSvgElementStackPush(xge_svg pSvg, const char* pTag, const char* pTagEnd)
{
	int iNodeIndex;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pSvg->iElementStackTop >= XGE_SVG_STACK_MAX ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iNodeIndex = -1;
	iRet = __xgeSvgElementNodeEnsure(pSvg, pTag, pTagEnd, &iNodeIndex);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pSvg->arrElementStack[pSvg->iElementStackTop].pTag = pTag;
	pSvg->arrElementStack[pSvg->iElementStackTop].pTagEnd = pTagEnd;
	pSvg->arrElementStack[pSvg->iElementStackTop].iNodeIndex = iNodeIndex;
	pSvg->iElementStackTop++;
	if ( pSvg->iElementStackTop < XGE_SVG_STACK_MAX ) {
		pSvg->arrLastChildByDepth[pSvg->iElementStackTop] = -1;
	}
	return XGE_OK;
}

static void __xgeSvgElementStackPop(xge_svg pSvg, const char* sName)
{
	if ( !__xgeSvgValid(pSvg) || (pSvg->iElementStackTop <= 0) ) {
		return;
	}
	if ( (sName != NULL) && !__xgeSvgTagNameEquals(pSvg->arrElementStack[pSvg->iElementStackTop - 1].pTag, sName) ) {
		return;
	}
	pSvg->iElementStackTop--;
	if ( (pSvg->iElementStackTop + 1) < XGE_SVG_STACK_MAX ) {
		pSvg->arrLastChildByDepth[pSvg->iElementStackTop + 1] = -1;
	}
	pSvg->iCurrentElementNode = -1;
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
	for ( i = 0; i < pDef->iTextCount; i++ ) {
		xrtFree(pDef->pTexts[i].sText);
		xgeShapeExDestroy(pDef->pTexts[i].pTextPath);
		xgeShapeExDestroy(pDef->pTexts[i].pMaskShape);
	}
	for ( i = 0; i < pDef->iImageCount; i++ ) {
		xgeSvgDestroy(pDef->pImages[i].pSvg);
		xgeShapeExDestroy(pDef->pImages[i].pMaskShape);
	}
	for ( i = 0; i < pDef->iRasterCount; i++ ) {
		__xgeSvgRasterImageDestroy(pDef->pRasters[i].pRaster);
		xgeShapeExDestroy(pDef->pRasters[i].pMaskShape);
	}
	xrtFree(pDef->pShapes);
	xrtFree(pDef->pShapeStyles);
	xrtFree(pDef->pTexts);
	xrtFree(pDef->pImages);
	xrtFree(pDef->pRasters);
	xrtFree(pDef->sId);
	memset(pDef, 0, sizeof(*pDef));
}

static void __xgeSvgDefShapesClear(xge_svg_def_t* pDef)
{
	int i;

	if ( pDef == NULL ) {
		return;
	}
	for ( i = 0; i < pDef->iShapeCount; i++ ) {
		xgeShapeExDestroy(pDef->pShapes[i]);
	}
	for ( i = 0; i < pDef->iTextCount; i++ ) {
		xrtFree(pDef->pTexts[i].sText);
		xgeShapeExDestroy(pDef->pTexts[i].pTextPath);
		xgeShapeExDestroy(pDef->pTexts[i].pMaskShape);
	}
	for ( i = 0; i < pDef->iImageCount; i++ ) {
		xgeSvgDestroy(pDef->pImages[i].pSvg);
		xgeShapeExDestroy(pDef->pImages[i].pMaskShape);
	}
	for ( i = 0; i < pDef->iRasterCount; i++ ) {
		__xgeSvgRasterImageDestroy(pDef->pRasters[i].pRaster);
		xgeShapeExDestroy(pDef->pRasters[i].pMaskShape);
	}
	pDef->iShapeCount = 0;
	pDef->iTextCount = 0;
	pDef->iImageCount = 0;
	pDef->iRasterCount = 0;
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

static int __xgeSvgDefAddShape(xge_svg_def_t* pDef, xge_shape_ex pShape, const xge_svg_style_t* pStyle)
{
	xge_shape_ex* pShapes;
	xge_svg_style_t* pShapeStyles;
	int iCapacity;
	int iIndex;

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
		pShapeStyles = (xge_svg_style_t*)xrtRealloc(pDef->pShapeStyles, (size_t)iCapacity * sizeof(*pShapeStyles));
		if ( pShapeStyles == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pDef->pShapeStyles = pShapeStyles;
		pDef->iShapeCapacity = iCapacity;
	}
	xgeShapeExAddRef(pShape);
	iIndex = pDef->iShapeCount++;
	pDef->pShapes[iIndex] = pShape;
	if ( pStyle != NULL ) {
		pDef->pShapeStyles[iIndex] = *pStyle;
	} else {
		memset(&pDef->pShapeStyles[iIndex], 0, sizeof(pDef->pShapeStyles[iIndex]));
	}
	return XGE_OK;
}

static int __xgeSvgDefAddText(xge_svg_def_t* pDef, const xge_svg_text_item_t* pText)
{
	xge_svg_text_item_t* pTexts;
	int iCapacity;
	int iIndex;

	if ( (pDef == NULL) || (pText == NULL) || (pText->sText == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pDef->iTextCount >= pDef->iTextCapacity ) {
		iCapacity = pDef->iTextCapacity > 0 ? pDef->iTextCapacity * 2 : 2;
		pTexts = (xge_svg_text_item_t*)xrtRealloc(pDef->pTexts, (size_t)iCapacity * sizeof(*pTexts));
		if ( pTexts == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pDef->pTexts = pTexts;
		pDef->iTextCapacity = iCapacity;
	}
	iIndex = pDef->iTextCount;
	pDef->pTexts[iIndex] = *pText;
	pDef->pTexts[iIndex].sText = __xgeStrDup(pText->sText);
	if ( pDef->pTexts[iIndex].sText == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( pDef->pTexts[iIndex].pTextPath != NULL ) {
		if ( xgeShapeExAddRef(pDef->pTexts[iIndex].pTextPath) != XGE_OK ) {
			xrtFree(pDef->pTexts[iIndex].sText);
			memset(&pDef->pTexts[iIndex], 0, sizeof(pDef->pTexts[iIndex]));
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( pDef->pTexts[iIndex].pMaskShape != NULL ) {
		if ( xgeShapeExAddRef(pDef->pTexts[iIndex].pMaskShape) != XGE_OK ) {
			xrtFree(pDef->pTexts[iIndex].sText);
			xgeShapeExDestroy(pDef->pTexts[iIndex].pTextPath);
			memset(&pDef->pTexts[iIndex], 0, sizeof(pDef->pTexts[iIndex]));
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	pDef->iTextCount++;
	return XGE_OK;
}

static int __xgeSvgDefAddImage(xge_svg_def_t* pDef, const xge_svg_image_item_t* pImage)
{
	xge_svg_image_item_t* pImages;
	int iCapacity;
	int iRet;

	if ( (pDef == NULL) || (pImage == NULL) || !__xgeSvgValid(pImage->pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pDef->iImageCount >= pDef->iImageCapacity ) {
		iCapacity = pDef->iImageCapacity > 0 ? pDef->iImageCapacity * 2 : 2;
		pImages = (xge_svg_image_item_t*)xrtRealloc(pDef->pImages, (size_t)iCapacity * sizeof(*pImages));
		if ( pImages == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pDef->pImages = pImages;
		pDef->iImageCapacity = iCapacity;
	}
	pDef->pImages[pDef->iImageCount] = *pImage;
	xgeSvgAddRef(pImage->pSvg);
	if ( pImage->pMaskShape != NULL ) {
		iRet = xgeShapeExAddRef(pImage->pMaskShape);
		if ( iRet != XGE_OK ) {
			xgeSvgDestroy(pDef->pImages[pDef->iImageCount].pSvg);
			memset(&pDef->pImages[pDef->iImageCount], 0, sizeof(pDef->pImages[pDef->iImageCount]));
			return iRet;
		}
	}
	pDef->iImageCount++;
	return XGE_OK;
}

static int __xgeSvgDefAddRasterImage(xge_svg_def_t* pDef, const xge_svg_raster_item_t* pImage)
{
	xge_svg_raster_item_t* pImages;
	int iCapacity;
	int iRet;

	if ( (pDef == NULL) || (pImage == NULL) || (pImage->pRaster == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pDef->iRasterCount >= pDef->iRasterCapacity ) {
		iCapacity = pDef->iRasterCapacity > 0 ? pDef->iRasterCapacity * 2 : 2;
		pImages = (xge_svg_raster_item_t*)xrtRealloc(pDef->pRasters, (size_t)iCapacity * sizeof(*pImages));
		if ( pImages == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pDef->pRasters = pImages;
		pDef->iRasterCapacity = iCapacity;
	}
	pDef->pRasters[pDef->iRasterCount] = *pImage;
	iRet = __xgeSvgRasterImageAddRef(pImage->pRaster);
	if ( iRet <= 0 ) {
		memset(&pDef->pRasters[pDef->iRasterCount], 0, sizeof(pDef->pRasters[pDef->iRasterCount]));
		return iRet;
	}
	if ( pImage->pMaskShape != NULL ) {
		iRet = xgeShapeExAddRef(pImage->pMaskShape);
		if ( iRet != XGE_OK ) {
			__xgeSvgRasterImageDestroy(pDef->pRasters[pDef->iRasterCount].pRaster);
			memset(&pDef->pRasters[pDef->iRasterCount], 0, sizeof(pDef->pRasters[pDef->iRasterCount]));
			return iRet;
		}
	}
	pDef->iRasterCount++;
	return XGE_OK;
}

static void __xgeSvgMarkerReset(xge_svg_marker_t* pMarker)
{
	if ( pMarker == NULL ) {
		return;
	}
	xrtFree(pMarker->sId);
	memset(pMarker, 0, sizeof(*pMarker));
	pMarker->iDefIndex = -1;
}

static void __xgeSvgMarkersClear(xge_svg pSvg)
{
	int i;

	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	for ( i = 0; i < pSvg->iMarkerCount; i++ ) {
		__xgeSvgMarkerReset(&pSvg->pMarkers[i]);
	}
	xrtFree(pSvg->pMarkers);
	pSvg->pMarkers = NULL;
	pSvg->iMarkerCount = 0;
	pSvg->iMarkerCapacity = 0;
}

static int __xgeSvgMarkerFind(xge_svg pSvg, const char* sId)
{
	int i;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') ) {
		return -1;
	}
	for ( i = 0; i < pSvg->iMarkerCount; i++ ) {
		if ( (pSvg->pMarkers[i].sId != NULL) && (strcmp(pSvg->pMarkers[i].sId, sId) == 0) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeSvgMarkerGetOrCreate(xge_svg pSvg, const char* sId, int* pIndex)
{
	xge_svg_marker_t* pMarkers;
	int iIndex;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') || (pIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xgeSvgMarkerFind(pSvg, sId);
	if ( iIndex >= 0 ) {
		*pIndex = iIndex;
		return XGE_OK;
	}
	if ( pSvg->iMarkerCount >= pSvg->iMarkerCapacity ) {
		iCapacity = pSvg->iMarkerCapacity > 0 ? pSvg->iMarkerCapacity * 2 : 8;
		pMarkers = (xge_svg_marker_t*)xrtRealloc(pSvg->pMarkers, (size_t)iCapacity * sizeof(*pMarkers));
		if ( pMarkers == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pMarkers + pSvg->iMarkerCapacity, 0, (size_t)(iCapacity - pSvg->iMarkerCapacity) * sizeof(*pMarkers));
		pSvg->pMarkers = pMarkers;
		pSvg->iMarkerCapacity = iCapacity;
	}
	iIndex = pSvg->iMarkerCount++;
	memset(&pSvg->pMarkers[iIndex], 0, sizeof(pSvg->pMarkers[iIndex]));
	pSvg->pMarkers[iIndex].sId = __xgeStrDup(sId);
	if ( pSvg->pMarkers[iIndex].sId == NULL ) {
		pSvg->iMarkerCount--;
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSvg->pMarkers[iIndex].iDefIndex = -1;
	pSvg->pMarkers[iIndex].fWidth = 3.0f;
	pSvg->pMarkers[iIndex].fHeight = 3.0f;
	pSvg->pMarkers[iIndex].iUnits = XGE_SVG_MARKER_UNITS_STROKE_WIDTH;
	*pIndex = iIndex;
	return XGE_OK;
}

static void __xgeSvgPatternReset(xge_svg_pattern_t* pPattern)
{
	if ( pPattern == NULL ) {
		return;
	}
	xrtFree(pPattern->sId);
	xrtFree(pPattern->sHrefId);
	memset(pPattern, 0, sizeof(*pPattern));
	pPattern->iDefIndex = -1;
	pPattern->iUnits = XGE_SVG_PATTERN_OBJECT_BOUNDING_BOX;
	pPattern->iContentUnits = XGE_SVG_PATTERN_USER_SPACE;
	pPattern->tTransform = __xgeSvgMatrixIdentity();
}

static void __xgeSvgPatternsClear(xge_svg pSvg)
{
	int i;

	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	for ( i = 0; i < pSvg->iPatternCount; i++ ) {
		__xgeSvgPatternReset(&pSvg->pPatterns[i]);
	}
	xrtFree(pSvg->pPatterns);
	pSvg->pPatterns = NULL;
	pSvg->iPatternCount = 0;
	pSvg->iPatternCapacity = 0;
}

static int __xgeSvgPatternFind(xge_svg pSvg, const char* sId)
{
	int i;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') ) {
		return -1;
	}
	for ( i = 0; i < pSvg->iPatternCount; i++ ) {
		if ( (pSvg->pPatterns[i].sId != NULL) && (strcmp(pSvg->pPatterns[i].sId, sId) == 0) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeSvgPatternGetOrCreate(xge_svg pSvg, const char* sId, int* pIndex)
{
	xge_svg_pattern_t* pPatterns;
	int iIndex;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') || (pIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xgeSvgPatternFind(pSvg, sId);
	if ( iIndex >= 0 ) {
		*pIndex = iIndex;
		return XGE_OK;
	}
	if ( pSvg->iPatternCount >= pSvg->iPatternCapacity ) {
		iCapacity = pSvg->iPatternCapacity > 0 ? pSvg->iPatternCapacity * 2 : 8;
		pPatterns = (xge_svg_pattern_t*)xrtRealloc(pSvg->pPatterns, (size_t)iCapacity * sizeof(*pPatterns));
		if ( pPatterns == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pPatterns + pSvg->iPatternCapacity, 0, (size_t)(iCapacity - pSvg->iPatternCapacity) * sizeof(*pPatterns));
		pSvg->pPatterns = pPatterns;
		pSvg->iPatternCapacity = iCapacity;
	}
	iIndex = pSvg->iPatternCount++;
	memset(&pSvg->pPatterns[iIndex], 0, sizeof(pSvg->pPatterns[iIndex]));
	pSvg->pPatterns[iIndex].sId = __xgeStrDup(sId);
	if ( pSvg->pPatterns[iIndex].sId == NULL ) {
		pSvg->iPatternCount--;
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSvg->pPatterns[iIndex].iDefIndex = -1;
	pSvg->pPatterns[iIndex].tRect.fX = 0.0f;
	pSvg->pPatterns[iIndex].tRect.fY = 0.0f;
	pSvg->pPatterns[iIndex].tRect.fW = 0.0f;
	pSvg->pPatterns[iIndex].tRect.fH = 0.0f;
	pSvg->pPatterns[iIndex].iUnits = XGE_SVG_PATTERN_OBJECT_BOUNDING_BOX;
	pSvg->pPatterns[iIndex].iContentUnits = XGE_SVG_PATTERN_USER_SPACE;
	pSvg->pPatterns[iIndex].tTransform = __xgeSvgMatrixIdentity();
	pSvg->pPatterns[iIndex].iFlags = 0;
	*pIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgDefHasContent(const xge_svg_def_t* pDef)
{
	return (pDef != NULL) && ((pDef->iShapeCount > 0) || (pDef->iTextCount > 0) || (pDef->iImageCount > 0) || (pDef->iRasterCount > 0));
}

static int __xgeSvgDefCopyContent(xge_svg_def_t* pDst, const xge_svg_def_t* pSrc)
{
	int i;
	int iRet;

	if ( (pDst == NULL) || (pSrc == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pSrc->iShapeCount; i++ ) {
		xge_shape_ex pClone;

		iRet = xgeShapeExClone(pSrc->pShapes[i], &pClone);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		iRet = __xgeSvgDefAddShape(pDst, pClone, (pSrc->pShapeStyles != NULL) ? &pSrc->pShapeStyles[i] : NULL);
		xgeShapeExDestroy(pClone);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	for ( i = 0; i < pSrc->iTextCount; i++ ) {
		iRet = __xgeSvgDefAddText(pDst, &pSrc->pTexts[i]);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	for ( i = 0; i < pSrc->iImageCount; i++ ) {
		iRet = __xgeSvgDefAddImage(pDst, &pSrc->pImages[i]);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	for ( i = 0; i < pSrc->iRasterCount; i++ ) {
		iRet = __xgeSvgDefAddRasterImage(pDst, &pSrc->pRasters[i]);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeSvgDefPrependTransform(xge_svg_def_t* pDef, xge_shape_ex_matrix_t tTransform)
{
	int i;
	int iRet;

	if ( pDef == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pDef->iShapeCount; i++ ) {
		xge_shape_ex pShape = pDef->pShapes[i];

		if ( pShape != NULL ) {
			xge_shape_ex_matrix_t tShapeTransform = __xgeSvgMatrixMul(tTransform, pShape->tTransform);
			iRet = xgeShapeExTransformSet(pShape, &tShapeTransform);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
	}
	for ( i = 0; i < pDef->iTextCount; i++ ) {
		pDef->pTexts[i].tStyle.tTransform = __xgeSvgMatrixMul(tTransform, pDef->pTexts[i].tStyle.tTransform);
	}
	for ( i = 0; i < pDef->iImageCount; i++ ) {
		pDef->pImages[i].tStyle.tTransform = __xgeSvgMatrixMul(tTransform, pDef->pImages[i].tStyle.tTransform);
	}
	for ( i = 0; i < pDef->iRasterCount; i++ ) {
		pDef->pRasters[i].tStyle.tTransform = __xgeSvgMatrixMul(tTransform, pDef->pRasters[i].tStyle.tTransform);
	}
	return XGE_OK;
}

static int __xgeSvgResolvePattern(xge_svg pSvg, int iIndex, int iDepth)
{
	xge_svg_pattern_t* pPattern;
	xge_svg_pattern_t* pSource;
	xge_svg_def_t* pDef;
	xge_svg_def_t* pSourceDef;
	int iSource;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iIndex < 0) || (iIndex >= pSvg->iPatternCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iDepth > pSvg->iPatternCount ) {
		return XGE_OK;
	}
	pPattern = &pSvg->pPatterns[iIndex];
	if ( (pPattern->iFlags & XGE_SVG_PATTERN_RESOLVED) != 0 ) {
		return XGE_OK;
	}
	if ( (pPattern->sHrefId == NULL) || (pPattern->sHrefId[0] == '\0') ) {
		pPattern->iFlags |= XGE_SVG_PATTERN_RESOLVED;
		return XGE_OK;
	}
	iSource = __xgeSvgPatternFind(pSvg, pPattern->sHrefId);
	if ( iSource < 0 ) {
		return XGE_OK;
	}
	if ( iSource == iIndex ) {
		pPattern->iFlags |= XGE_SVG_PATTERN_RESOLVED;
		return XGE_OK;
	}
	iRet = __xgeSvgResolvePattern(pSvg, iSource, iDepth + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pSource = &pSvg->pPatterns[iSource];
	if ( (pPattern->iFlags & XGE_SVG_PATTERN_HAS_X) == 0 ) pPattern->tRect.fX = pSource->tRect.fX;
	if ( (pPattern->iFlags & XGE_SVG_PATTERN_HAS_Y) == 0 ) pPattern->tRect.fY = pSource->tRect.fY;
	if ( (pPattern->iFlags & XGE_SVG_PATTERN_HAS_WIDTH) == 0 ) pPattern->tRect.fW = pSource->tRect.fW;
	if ( (pPattern->iFlags & XGE_SVG_PATTERN_HAS_HEIGHT) == 0 ) pPattern->tRect.fH = pSource->tRect.fH;
	if ( (pPattern->iFlags & XGE_SVG_PATTERN_HAS_UNITS) == 0 ) pPattern->iUnits = pSource->iUnits;
	if ( (pPattern->iFlags & XGE_SVG_PATTERN_HAS_CONTENT_UNITS) == 0 ) pPattern->iContentUnits = pSource->iContentUnits;
	if ( (pPattern->iFlags & XGE_SVG_PATTERN_HAS_TRANSFORM) == 0 ) pPattern->tTransform = pSource->tTransform;
	if ( (pPattern->iDefIndex >= 0) && (pPattern->iDefIndex < pSvg->iDefCount) &&
	     (pSource->iDefIndex >= 0) && (pSource->iDefIndex < pSvg->iDefCount) ) {
		pDef = &pSvg->pDefs[pPattern->iDefIndex];
		pSourceDef = &pSvg->pDefs[pSource->iDefIndex];
		if ( ((pPattern->iFlags & XGE_SVG_PATTERN_HAS_VIEWBOX) == 0) && pSourceDef->bHasViewBox ) {
			pDef->tViewBox = pSourceDef->tViewBox;
			pDef->bHasViewBox = 1;
		}
		if ( (pPattern->iFlags & XGE_SVG_PATTERN_HAS_ASPECT) == 0 ) {
			pDef->iAspectAlignX = pSourceDef->iAspectAlignX;
			pDef->iAspectAlignY = pSourceDef->iAspectAlignY;
			pDef->iAspectMeetOrSlice = pSourceDef->iAspectMeetOrSlice;
		}
		if ( !__xgeSvgDefHasContent(pDef) && __xgeSvgDefHasContent(pSourceDef) ) {
			iRet = __xgeSvgDefCopyContent(pDef, pSourceDef);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
		}
	}
	pPattern->iFlags |= XGE_SVG_PATTERN_RESOLVED;
	return XGE_OK;
}

static void __xgeSvgClipPathReset(xge_svg_clip_path_t* pClip)
{
	int i;

	if ( pClip == NULL ) {
		return;
	}
	xrtFree(pClip->sId);
	xrtFree(pClip->pRects);
	for ( i = 0; i < pClip->iShapeCount; i++ ) {
		xgeShapeExDestroy(pClip->pShapes[i]);
	}
	xrtFree(pClip->pShapes);
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

static int __xgeSvgRectAppend(xge_rect_t** ppRects, int* pCount, int* pCapacity, xge_rect_t tRect)
{
	xge_rect_t* pRects;
	int iCapacity;

	if ( (ppRects == NULL) || (pCount == NULL) || (pCapacity == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_OK;
	}
	if ( *pCount >= *pCapacity ) {
		iCapacity = *pCapacity > 0 ? *pCapacity * 2 : 4;
		pRects = (xge_rect_t*)xrtRealloc(*ppRects, (size_t)iCapacity * sizeof(*pRects));
		if ( pRects == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		*ppRects = pRects;
		*pCapacity = iCapacity;
	}
	(*ppRects)[(*pCount)++] = tRect;
	return XGE_OK;
}

static int __xgeSvgRectSubtract(xge_rect_t tRect, xge_rect_t tCut, xge_rect_t* pOut, int iOutCapacity)
{
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;
	float fRectRight;
	float fRectBottom;
	int iCount;

	if ( (pOut == NULL) || (iOutCapacity < 4) ) {
		return 0;
	}
	fRectRight = tRect.fX + tRect.fW;
	fRectBottom = tRect.fY + tRect.fH;
	fLeft = fmaxf(tRect.fX, tCut.fX);
	fTop = fmaxf(tRect.fY, tCut.fY);
	fRight = fminf(fRectRight, tCut.fX + tCut.fW);
	fBottom = fminf(fRectBottom, tCut.fY + tCut.fH);
	if ( (fRight <= fLeft) || (fBottom <= fTop) ) {
		pOut[0] = tRect;
		return 1;
	}
	iCount = 0;
	if ( fTop > tRect.fY ) {
		pOut[iCount++] = (xge_rect_t){tRect.fX, tRect.fY, tRect.fW, fTop - tRect.fY};
	}
	if ( fBottom < fRectBottom ) {
		pOut[iCount++] = (xge_rect_t){tRect.fX, fBottom, tRect.fW, fRectBottom - fBottom};
	}
	if ( fLeft > tRect.fX ) {
		pOut[iCount++] = (xge_rect_t){tRect.fX, fTop, fLeft - tRect.fX, fBottom - fTop};
	}
	if ( fRight < fRectRight ) {
		pOut[iCount++] = (xge_rect_t){fRight, fTop, fRectRight - fRight, fBottom - fTop};
	}
	return iCount;
}

static int __xgeSvgClipPathAddRect(xge_svg_clip_path_t* pClip, xge_rect_t tRect)
{
	xge_rect_t* pPieces;
	int iPieceCount;
	int iPieceCapacity;
	int i;
	int iRet;

	if ( pClip == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_OK;
	}
	pPieces = NULL;
	iPieceCount = 0;
	iPieceCapacity = 0;
	iRet = __xgeSvgRectAppend(&pPieces, &iPieceCount, &iPieceCapacity, tRect);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	for ( i = 0; i < pClip->iRectCount && iPieceCount > 0; i++ ) {
		xge_rect_t* pNextPieces;
		int iNextCount;
		int iNextCapacity;
		int j;

		pNextPieces = NULL;
		iNextCount = 0;
		iNextCapacity = 0;
		for ( j = 0; j < iPieceCount; j++ ) {
			xge_rect_t arrSubtracted[4];
			int iSubCount;
			int k;

			iSubCount = __xgeSvgRectSubtract(pPieces[j], pClip->pRects[i], arrSubtracted, 4);
			for ( k = 0; k < iSubCount; k++ ) {
				iRet = __xgeSvgRectAppend(&pNextPieces, &iNextCount, &iNextCapacity, arrSubtracted[k]);
				if ( iRet != XGE_OK ) {
					xrtFree(pPieces);
					xrtFree(pNextPieces);
					return iRet;
				}
			}
		}
		xrtFree(pPieces);
		pPieces = pNextPieces;
		iPieceCount = iNextCount;
		iPieceCapacity = iNextCapacity;
	}
	for ( i = 0; i < iPieceCount; i++ ) {
		iRet = __xgeSvgRectAppend(&pClip->pRects, &pClip->iRectCount, &pClip->iRectCapacity, pPieces[i]);
		if ( iRet != XGE_OK ) {
			xrtFree(pPieces);
			return iRet;
		}
		pClip->bHasRect = 1;
	}
	xrtFree(pPieces);
	return XGE_OK;
}

static int __xgeSvgClipPathAddShape(xge_svg_clip_path_t* pClip, xge_shape_ex pShape)
{
	xge_shape_ex* pShapes;
	int iCapacity;
	int iRet;

	if ( (pClip == NULL) || (pShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pClip->iShapeCount >= pClip->iShapeCapacity ) {
		iCapacity = pClip->iShapeCapacity > 0 ? pClip->iShapeCapacity * 2 : 4;
		pShapes = (xge_shape_ex*)xrtRealloc(pClip->pShapes, (size_t)iCapacity * sizeof(*pShapes));
		if ( pShapes == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pClip->pShapes = pShapes;
		pClip->iShapeCapacity = iCapacity;
	}
	iRet = xgeShapeExAddRef(pShape);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pClip->pShapes[pClip->iShapeCount++] = pShape;
	pClip->bHasShape = 1;
	return XGE_OK;
}

static void __xgeSvgMaskReset(xge_svg_mask_t* pMask)
{
	int i;

	if ( pMask == NULL ) {
		return;
	}
	xrtFree(pMask->sId);
	xrtFree(pMask->pRects);
	for ( i = 0; i < pMask->iShapeCount; i++ ) {
		xgeShapeExDestroy(pMask->pShapes[i].pShape);
	}
	xrtFree(pMask->pShapes);
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
	pSvg->pMasks[iIndex].iUnits = XGE_SVG_MASK_OBJECT_BOUNDING_BOX;
	pSvg->pMasks[iIndex].tRegion = (xge_rect_t){-0.1f, -0.1f, 1.2f, 1.2f};
	pSvg->pMasks[iIndex].iMaskType = XGE_SVG_MASK_TYPE_LUMINANCE;
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

static int __xgeSvgMaskAddShape(xge_svg_mask_t* pMask, xge_shape_ex pShape, float fOpacity)
{
	xge_svg_mask_shape_t* pShapes;
	int iCapacity;
	int iRet;

	if ( (pMask == NULL) || (pShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( fOpacity <= 0.0f ) {
		return XGE_OK;
	}
	if ( fOpacity > 1.0f ) {
		fOpacity = 1.0f;
	}
	if ( pMask->iShapeCount >= pMask->iShapeCapacity ) {
		iCapacity = pMask->iShapeCapacity > 0 ? pMask->iShapeCapacity * 2 : 4;
		pShapes = (xge_svg_mask_shape_t*)xrtRealloc(pMask->pShapes, (size_t)iCapacity * sizeof(*pShapes));
		if ( pShapes == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pMask->pShapes = pShapes;
		pMask->iShapeCapacity = iCapacity;
	}
	iRet = xgeShapeExAddRef(pShape);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pMask->pShapes[pMask->iShapeCount].pShape = pShape;
	pMask->pShapes[pMask->iShapeCount].fOpacity = fOpacity;
	pMask->iShapeCount++;
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

static int __xgeSvgPendingUseAdd(xge_svg pSvg, const char* sId, const xge_svg_style_t* pStyle, int iTargetDef, int iMainIdDefIndex, int iExtraMainIdDefIndex, float fX, float fY, float fW, float fH, int bHasWidth, int bHasHeight)
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
	pUse->iMainIdDefIndex = iMainIdDefIndex;
	pUse->iExtraMainIdDefIndex = iExtraMainIdDefIndex;
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
	pSvg->pLinearGradients[iIndex].tTransform = __xgeSvgMatrixIdentity();
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
	pSvg->pRadialGradients[iIndex].tTransform = __xgeSvgMatrixIdentity();
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

static int __xgeSvgLinearGradientInheritRadialCommon(xge_svg_linear_gradient_t* pGradient, const xge_svg_radial_gradient_t* pSource)
{
	int i;
	int iRet;

	if ( (pGradient == NULL) || (pSource == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
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
	return XGE_OK;
}

static int __xgeSvgRadialGradientInheritLinearCommon(xge_svg_radial_gradient_t* pGradient, const xge_svg_linear_gradient_t* pSource)
{
	int i;
	int iRet;

	if ( (pGradient == NULL) || (pSource == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
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
	return XGE_OK;
}

static int __xgeSvgResolveRadialGradient(xge_svg pSvg, int iIndex, int iDepth);

static int __xgeSvgResolveLinearGradient(xge_svg pSvg, int iIndex, int iDepth)
{
	xge_svg_linear_gradient_t* pGradient;
	xge_svg_linear_gradient_t* pSource;
	xge_svg_radial_gradient_t* pRadialSource;
	int iSource;
	int i;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iIndex < 0) || (iIndex >= pSvg->iLinearGradientCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iDepth > (pSvg->iLinearGradientCount + pSvg->iRadialGradientCount) ) {
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
		iSource = __xgeSvgRadialGradientFind(pSvg, pGradient->sHrefId);
		if ( iSource < 0 ) {
			return XGE_OK;
		}
		iRet = __xgeSvgResolveRadialGradient(pSvg, iSource, iDepth + 1);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		pRadialSource = &pSvg->pRadialGradients[iSource];
		iRet = __xgeSvgLinearGradientInheritRadialCommon(pGradient, pRadialSource);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		pGradient->iFlags |= XGE_SVG_GRADIENT_RESOLVED;
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
	xge_svg_linear_gradient_t* pLinearSource;
	int iSource;
	int i;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iIndex < 0) || (iIndex >= pSvg->iRadialGradientCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iDepth > (pSvg->iLinearGradientCount + pSvg->iRadialGradientCount) ) {
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
		iSource = __xgeSvgLinearGradientFind(pSvg, pGradient->sHrefId);
		if ( iSource < 0 ) {
			return XGE_OK;
		}
		iRet = __xgeSvgResolveLinearGradient(pSvg, iSource, iDepth + 1);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		pLinearSource = &pSvg->pLinearGradients[iSource];
		iRet = __xgeSvgRadialGradientInheritLinearCommon(pGradient, pLinearSource);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		pGradient->iFlags |= XGE_SVG_GRADIENT_RESOLVED;
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

static int __xgeSvgRangeHasNonSpace(const char* pStart, const char* pEnd)
{
	const char* p;

	if ( (pStart == NULL) || (pEnd == NULL) || (pEnd <= pStart) ) {
		return 0;
	}
	for ( p = pStart; p < pEnd; p++ ) {
		if ( (*p != ' ') && (*p != '\t') && (*p != '\r') && (*p != '\n') && (*p != '\f') ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeSvgElementNodesPreScan(xge_svg pSvg, const char* sText)
{
	const char* p;
	const char* pScan;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (sText == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeSvgElementNodesClear(pSvg);
	pSvg->iElementStackTop = 0;
	pScan = sText;
	while ( (p = strchr(pScan, '<')) != NULL ) {
		const char* pTagEnd;
		int bSelfClosing;
		int iNodeIndex;

		if ( (p > pScan) && (pSvg->iElementStackTop > 0) && __xgeSvgRangeHasNonSpace(pScan, p) ) {
			int iParentNode = pSvg->arrElementStack[pSvg->iElementStackTop - 1].iNodeIndex;
			if ( (iParentNode >= 0) && (iParentNode < pSvg->iElementNodeCount) ) {
				pSvg->pElementNodes[iParentNode].bHasTextContent = 1;
			}
		}
		pTagEnd = __xgeSvgFindTagEnd(p);
		if ( pTagEnd == NULL ) {
			break;
		}
		if ( (p[1] == '!') || (p[1] == '?') ) {
			pScan = pTagEnd + 1;
			continue;
		}
		if ( p[1] == '/' ) {
			__xgeSvgElementStackPop(pSvg, NULL);
			pScan = pTagEnd + 1;
			continue;
		}
		bSelfClosing = __xgeSvgIsSelfClosingTag(p, pTagEnd);
		iNodeIndex = -1;
		iRet = __xgeSvgElementNodeEnsure(pSvg, p, pTagEnd, &iNodeIndex);
		if ( iRet != XGE_OK ) {
			pSvg->iElementStackTop = 0;
			pSvg->iCurrentElementNode = -1;
			memset(pSvg->arrLastChildByDepth, 0xff, sizeof(pSvg->arrLastChildByDepth));
			return iRet;
		}
		if ( !bSelfClosing ) {
			iRet = __xgeSvgElementStackPush(pSvg, p, pTagEnd);
			if ( iRet != XGE_OK ) {
				pSvg->iElementStackTop = 0;
				pSvg->iCurrentElementNode = -1;
				memset(pSvg->arrLastChildByDepth, 0xff, sizeof(pSvg->arrLastChildByDepth));
				return iRet;
			}
			if ( __xgeSvgTagNameEquals(p, "style") || __xgeSvgTagNameEquals(p, "script") ) {
				const char* pClose = __xgeSvgTagNameEquals(p, "style") ? strstr(pTagEnd + 1, "</style") : strstr(pTagEnd + 1, "</script");
				if ( pClose != NULL ) {
					const char* pCloseEnd = __xgeSvgFindTagEnd(pClose);
					__xgeSvgElementStackPop(pSvg, NULL);
					if ( pCloseEnd != NULL ) {
						pScan = pCloseEnd + 1;
						continue;
					}
				}
			}
		}
		pScan = pTagEnd + 1;
	}
	if ( (pScan != NULL) && (pSvg->iElementStackTop > 0) && __xgeSvgRangeHasNonSpace(pScan, pScan + strlen(pScan)) ) {
		int iParentNode = pSvg->arrElementStack[pSvg->iElementStackTop - 1].iNodeIndex;
		if ( (iParentNode >= 0) && (iParentNode < pSvg->iElementNodeCount) ) {
			pSvg->pElementNodes[iParentNode].bHasTextContent = 1;
		}
	}
	pSvg->iElementStackTop = 0;
	pSvg->iCurrentElementNode = -1;
	memset(pSvg->arrLastChildByDepth, 0xff, sizeof(pSvg->arrLastChildByDepth));
	return XGE_OK;
}

static int __xgeSvgCssIsSpace(char c)
{
	return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n') || (c == '\f');
}

static int __xgeSvgCssAsciiLower(int c);
static int __xgeSvgCssRangeEqualsText(const char* pStart, const char* pEnd, const char* sText);

static int __xgeSvgCssRangeCopy(const char* pStart, const char* pEnd, char* sOut, int iOutCapacity)
{
	int iLen;

	if ( (pStart == NULL) || (pEnd == NULL) || (sOut == NULL) || (iOutCapacity <= 0) || (pEnd < pStart) ) {
		return 0;
	}
	__xgeSvgTrimRange(&pStart, &pEnd);
	iLen = (int)(pEnd - pStart);
	if ( (iLen <= 0) || (iLen >= iOutCapacity) ) {
		return 0;
	}
	memcpy(sOut, pStart, (size_t)iLen);
	sOut[iLen] = '\0';
	return 1;
}

static int __xgeSvgCssIdentChar(char c)
{
	return ((c >= 'a') && (c <= 'z')) ||
	       ((c >= 'A') && (c <= 'Z')) ||
	       ((c >= '0') && (c <= '9')) ||
	       (c == '-') || (c == '_');
}

static int __xgeSvgCssFunctionEnd(const char* pStart, const char* pEnd, const char** ppEnd)
{
	const char* p;
	int iDepth;
	int bInAttr;
	char cQuote;

	if ( (pStart == NULL) || (pEnd == NULL) || (ppEnd == NULL) || (pStart >= pEnd) || (*pStart != '(') ) {
		return 0;
	}
	p = pStart;
	iDepth = 0;
	bInAttr = 0;
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
		if ( bInAttr ) {
			if ( *p == ']' ) {
				bInAttr = 0;
			}
			p++;
			continue;
		}
		if ( *p == '[' ) {
			bInAttr = 1;
			p++;
			continue;
		}
		if ( *p == '(' ) {
			iDepth++;
			p++;
			continue;
		}
		if ( *p == ')' ) {
			iDepth--;
			if ( iDepth == 0 ) {
				*ppEnd = p + 1;
				return 1;
			}
			if ( iDepth < 0 ) {
				return 0;
			}
		}
		p++;
	}
	return 0;
}

static int __xgeSvgCssAttributeSelectorEnd(const char* pStart, const char* pEnd, const char** ppEnd)
{
	const char* p;
	char cQuote;

	if ( (pStart == NULL) || (pEnd == NULL) || (ppEnd == NULL) || (pStart >= pEnd) || (*pStart != '[') ) {
		return 0;
	}
	p = pStart + 1;
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
		if ( *p == ']' ) {
			*ppEnd = p + 1;
			return 1;
		}
		p++;
	}
	return 0;
}

static int __xgeSvgCssAttributeSelectorParse(const char* pStart, const char* pEnd, char* sName, int iNameCapacity, char* sOp, int iOpCapacity, char* sValue, int iValueCapacity)
{
	const char* p;
	const char* pClose;
	const char* pNameStart;
	const char* pNameEnd;
	const char* pValueStart;
	const char* pValueEnd;
	char cQuote;
	int iOpLen;

	if ( (pStart == NULL) || (pEnd == NULL) || (sName == NULL) || (iNameCapacity <= 0) ||
	     (sOp == NULL) || (iOpCapacity <= 0) || (sValue == NULL) || (iValueCapacity <= 0) ) {
		return 0;
	}
	sName[0] = '\0';
	sOp[0] = '\0';
	sValue[0] = '\0';
	if ( !__xgeSvgCssAttributeSelectorEnd(pStart, pEnd, &pClose) || (pClose != pEnd) ) {
		return 0;
	}
	p = pStart + 1;
	pClose--;
	while ( (p < pClose) && __xgeSvgCssIsSpace(*p) ) {
		p++;
	}
	pNameStart = p;
	while ( (p < pClose) && !__xgeSvgCssIsSpace(*p) && (*p != '=') && (*p != '~') &&
	        (*p != '|') && (*p != '^') && (*p != '$') && (*p != '*') && (*p != ']') ) {
		p++;
	}
	pNameEnd = p;
	if ( !__xgeSvgCssRangeCopy(pNameStart, pNameEnd, sName, iNameCapacity) ) {
		return 0;
	}
	while ( (p < pClose) && __xgeSvgCssIsSpace(*p) ) {
		p++;
	}
	if ( p >= pClose ) {
		return 1;
	}
	iOpLen = 0;
	if ( *p == '=' ) {
		if ( iOpCapacity < 2 ) return 0;
		sOp[iOpLen++] = *p++;
	} else if ( ((*p == '~') || (*p == '|') || (*p == '^') || (*p == '$') || (*p == '*')) &&
	            ((p + 1) < pClose) && (p[1] == '=') ) {
		if ( iOpCapacity < 3 ) return 0;
		sOp[iOpLen++] = *p++;
		sOp[iOpLen++] = *p++;
	} else {
		return 0;
	}
	sOp[iOpLen] = '\0';
	while ( (p < pClose) && __xgeSvgCssIsSpace(*p) ) {
		p++;
	}
	if ( p >= pClose ) {
		return 0;
	}
	cQuote = 0;
	if ( (*p == '"') || (*p == '\'') ) {
		cQuote = *p++;
	}
	pValueStart = p;
	if ( cQuote != 0 ) {
		while ( (p < pClose) && (*p != cQuote) ) {
			p++;
		}
		if ( p >= pClose ) {
			return 0;
		}
		pValueEnd = p++;
	} else {
		while ( (p < pClose) && !__xgeSvgCssIsSpace(*p) ) {
			p++;
		}
		pValueEnd = p;
	}
	if ( !__xgeSvgCssRangeCopy(pValueStart, pValueEnd, sValue, iValueCapacity) ) {
		return 0;
	}
	while ( (p < pClose) && __xgeSvgCssIsSpace(*p) ) {
		p++;
	}
	return p == pClose;
}

static int __xgeSvgCssNthParse(const char* pStart, const char* pEnd, int* pA, int* pB)
{
	char sExpr[128];
	char* pN;
	char* pParseEnd;
	int iWrite;
	long iValue;

	if ( (pStart == NULL) || (pEnd == NULL) || (pA == NULL) || (pB == NULL) || (pEnd < pStart) ) {
		return 0;
	}
	__xgeSvgTrimRange(&pStart, &pEnd);
	if ( pEnd <= pStart ) {
		return 0;
	}
	iWrite = 0;
	while ( pStart < pEnd ) {
		if ( !__xgeSvgCssIsSpace(*pStart) ) {
			if ( iWrite >= (int)sizeof(sExpr) - 1 ) {
				return 0;
			}
			sExpr[iWrite++] = (char)__xgeSvgCssAsciiLower((unsigned char)*pStart);
		}
		pStart++;
	}
	sExpr[iWrite] = '\0';
	if ( strcmp(sExpr, "odd") == 0 ) {
		*pA = 2;
		*pB = 1;
		return 1;
	}
	if ( strcmp(sExpr, "even") == 0 ) {
		*pA = 2;
		*pB = 0;
		return 1;
	}
	pN = strchr(sExpr, 'n');
	if ( pN == NULL ) {
		iValue = strtol(sExpr, &pParseEnd, 10);
		if ( (pParseEnd == sExpr) || (*pParseEnd != '\0') || (iValue < INT32_MIN) || (iValue > INT32_MAX) ) {
			return 0;
		}
		*pA = 0;
		*pB = (int)iValue;
		return 1;
	}
	if ( strchr(pN + 1, 'n') != NULL ) {
		return 0;
	}
	if ( pN == sExpr ) {
		*pA = 1;
	} else if ( ((pN - sExpr) == 1) && (sExpr[0] == '+') ) {
		*pA = 1;
	} else if ( ((pN - sExpr) == 1) && (sExpr[0] == '-') ) {
		*pA = -1;
	} else {
		char cSave = *pN;

		*pN = '\0';
		iValue = strtol(sExpr, &pParseEnd, 10);
		if ( (pParseEnd == sExpr) || (*pParseEnd != '\0') || (iValue < INT32_MIN) || (iValue > INT32_MAX) ) {
			*pN = cSave;
			return 0;
		}
		*pN = cSave;
		*pA = (int)iValue;
	}
	if ( pN[1] == '\0' ) {
		*pB = 0;
		return 1;
	}
	iValue = strtol(pN + 1, &pParseEnd, 10);
	if ( (pParseEnd == pN + 1) || (*pParseEnd != '\0') || (iValue < INT32_MIN) || (iValue > INT32_MAX) ) {
		return 0;
	}
	*pB = (int)iValue;
	return 1;
}

static int __xgeSvgCssNthMatches(int iIndex, int iA, int iB)
{
	int iDelta;

	if ( iIndex <= 0 ) {
		return 0;
	}
	if ( iA == 0 ) {
		return iIndex == iB;
	}
	iDelta = iIndex - iB;
	if ( iA > 0 ) {
		return (iDelta >= 0) && ((iDelta % iA) == 0);
	}
	return (iDelta <= 0) && ((iDelta % iA) == 0);
}

static int __xgeSvgCssSimpleSelectorSpecificity(const char* pStart, const char* pEnd, int* pSpecificity);
static int __xgeSvgCssSimpleSelectorListSpecificity(const char* pStart, const char* pEnd, int* pSpecificity);
static int __xgeSvgCssSelectorSpecificity(const char* pStart, const char* pEnd, int* pSpecificity);
static int __xgeSvgCssSelectorListSpecificity(const char* pStart, const char* pEnd, int* pSpecificity);
static int __xgeSvgCssRelativeSelectorListSpecificity(const char* pStart, const char* pEnd, int* pSpecificity);

static int __xgeSvgCssSimpleSelectorSpecificity(const char* pStart, const char* pEnd, int* pSpecificity)
{
	const char* p;
	int iSpecificity;

	if ( (pStart == NULL) || (pEnd == NULL) || (pSpecificity == NULL) ) {
		return 0;
	}
	__xgeSvgTrimRange(&pStart, &pEnd);
	if ( pEnd <= pStart ) {
		return 0;
	}
	p = pStart;
	iSpecificity = 0;
	if ( *p == '*' ) {
		p++;
		if ( (p < pEnd) && (*p != '.') && (*p != '#') && (*p != '[') && (*p != ':') ) {
			return 0;
		}
	} else if ( (*p != '.') && (*p != '#') && (*p != '[') && (*p != ':') ) {
		const char* pNameStart = p;
		while ( (p < pEnd) && (*p != '.') && (*p != '#') && (*p != '[') && (*p != ':') ) {
			if ( __xgeSvgCssIsSpace(*p) || (*p == '>') || (*p == '+') || (*p == '~') ||
			     (*p == ']') || (*p == ':') || (*p == '*') ) {
				return 0;
			}
			p++;
		}
		if ( p == pNameStart ) {
			return 0;
		}
		iSpecificity += 1;
	}
	while ( p < pEnd ) {
		if ( (*p != '.') && (*p != '#') && (*p != '[') && (*p != ':') ) {
			return 0;
		}
		if ( *p == '[' ) {
			const char* pAttrEnd;
			char sName[XGE_SVG_ATTR_MAX];
			char sOp[3];
			char sValue[XGE_SVG_ATTR_MAX];

			if ( !__xgeSvgCssAttributeSelectorEnd(p, pEnd, &pAttrEnd) ||
			     !__xgeSvgCssAttributeSelectorParse(p, pAttrEnd, sName, sizeof(sName), sOp, sizeof(sOp), sValue, sizeof(sValue)) ) {
				return 0;
			}
			iSpecificity += 10;
			p = pAttrEnd;
			continue;
		}
		if ( (*p == '.') || (*p == '#') ) {
			const char* pNameStart;
			const char* pNameEnd;

			if ( *p == '.' ) {
				iSpecificity += 10;
			} else {
				iSpecificity += 100;
			}
			p++;
			if ( (p >= pEnd) || (*p == '.') || (*p == '#') || (*p == '[') || (*p == ':') ) {
				return 0;
			}
			pNameStart = p;
			while ( (p < pEnd) && (*p != '.') && (*p != '#') && (*p != '[') && (*p != ':') ) {
				if ( __xgeSvgCssIsSpace(*p) || (*p == '>') || (*p == '+') || (*p == '~') ||
				     (*p == ']') || (*p == '*') ) {
					return 0;
				}
				p++;
			}
			pNameEnd = p;
			if ( pNameEnd <= pNameStart ) {
				return 0;
			}
			continue;
		}
		if ( *p == ':' ) {
			const char* pNameStart;
			const char* pNameEnd;

			p++;
			pNameStart = p;
			while ( (p < pEnd) && __xgeSvgCssIdentChar(*p) ) {
				p++;
			}
			pNameEnd = p;
			if ( pNameEnd <= pNameStart ) {
				return 0;
			}
			if ( __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "root") ) {
				iSpecificity += 10;
				continue;
			}
			if ( __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "first-child") ||
			     __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "first-of-type") ||
			     __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "last-child") ||
			     __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "last-of-type") ||
			     __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "only-child") ||
			     __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "only-of-type") ||
			     __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "empty") ) {
				iSpecificity += 10;
				continue;
			}
			if ( __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "nth-child") ||
			     __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "nth-of-type") ||
			     __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "nth-last-child") ||
			     __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "nth-last-of-type") ) {
				const char* pFunctionEnd;
				int iA;
				int iB;

				if ( (p >= pEnd) || (*p != '(') || !__xgeSvgCssFunctionEnd(p, pEnd, &pFunctionEnd) ||
				     !__xgeSvgCssNthParse(p + 1, pFunctionEnd - 1, &iA, &iB) ) {
					return 0;
				}
				iSpecificity += 10;
				p = pFunctionEnd;
				continue;
			}
			if ( __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "not") ||
			     __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "is") ||
			     __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "where") ||
			     __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "has") ) {
				const char* pFunctionEnd;
				const char* pArgStart;
				const char* pArgEnd;
				int iArgSpecificity;
				int bZeroSpecificity;

				if ( (p >= pEnd) || (*p != '(') || !__xgeSvgCssFunctionEnd(p, pEnd, &pFunctionEnd) ) {
					return 0;
				}
				pArgStart = p + 1;
				pArgEnd = pFunctionEnd - 1;
				if ( __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "has") ) {
					if ( !__xgeSvgCssRelativeSelectorListSpecificity(pArgStart, pArgEnd, &iArgSpecificity) ) {
						return 0;
					}
				} else if ( !__xgeSvgCssSelectorListSpecificity(pArgStart, pArgEnd, &iArgSpecificity) ) {
					return 0;
				}
				bZeroSpecificity = __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "where");
				if ( !bZeroSpecificity ) {
					iSpecificity += iArgSpecificity;
				}
				p = pFunctionEnd;
				continue;
			}
			return 0;
		}
	}
	*pSpecificity = iSpecificity;
	return 1;
}

static int __xgeSvgCssSimpleSelectorListSpecificity(const char* pStart, const char* pEnd, int* pSpecificity)
{
	const char* p;
	const char* pItemStart;
	int iBestSpecificity;
	int iItemCount;
	int iParenDepth;
	int bInAttr;
	char cQuote;

	if ( (pStart == NULL) || (pEnd == NULL) || (pSpecificity == NULL) ) {
		return 0;
	}
	__xgeSvgTrimRange(&pStart, &pEnd);
	if ( pEnd <= pStart ) {
		return 0;
	}
	p = pStart;
	pItemStart = pStart;
	iBestSpecificity = 0;
	iItemCount = 0;
	iParenDepth = 0;
	bInAttr = 0;
	cQuote = 0;
	while ( p <= pEnd ) {
		int bAtEnd;
		int bSplit;

		bAtEnd = (p == pEnd);
		bSplit = bAtEnd || ((cQuote == 0) && !bInAttr && (iParenDepth == 0) && (*p == ','));
		if ( bSplit ) {
			const char* pItemEnd;
			int iItemSpecificity;

			pItemEnd = p;
			__xgeSvgTrimRange(&pItemStart, &pItemEnd);
			if ( pItemEnd <= pItemStart ) {
				return 0;
			}
			if ( !__xgeSvgCssSimpleSelectorSpecificity(pItemStart, pItemEnd, &iItemSpecificity) ) {
				return 0;
			}
			if ( (iItemCount == 0) || (iItemSpecificity > iBestSpecificity) ) {
				iBestSpecificity = iItemSpecificity;
			}
			iItemCount++;
			if ( bAtEnd ) {
				break;
			}
			p++;
			pItemStart = p;
			continue;
		}
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
		if ( bInAttr ) {
			if ( *p == ']' ) {
				bInAttr = 0;
			}
			p++;
			continue;
		}
		if ( *p == '[' ) {
			bInAttr = 1;
			p++;
			continue;
		}
		if ( *p == '(' ) {
			iParenDepth++;
			p++;
			continue;
		}
		if ( *p == ')' ) {
			if ( iParenDepth <= 0 ) {
				return 0;
			}
			iParenDepth--;
			p++;
			continue;
		}
		p++;
	}
	if ( (iItemCount <= 0) || bInAttr || (cQuote != 0) || (iParenDepth != 0) ) {
		return 0;
	}
	*pSpecificity = iBestSpecificity;
	return 1;
}

static int __xgeSvgCssSelectorSpecificity(const char* pStart, const char* pEnd, int* pSpecificity)
{
	const char* p;
	int iSpecificity;
	int bNeedSimple;

	if ( (pStart == NULL) || (pEnd == NULL) || (pSpecificity == NULL) ) {
		return 0;
	}
	__xgeSvgTrimRange(&pStart, &pEnd);
	if ( pEnd <= pStart ) {
		return 0;
	}
	p = pStart;
	iSpecificity = 0;
	bNeedSimple = 1;
	while ( p < pEnd ) {
		const char* pSimpleStart;
		const char* pSimpleEnd;
		int iSimpleSpecificity;
		int bHadSpace;

		bHadSpace = 0;
		while ( (p < pEnd) && __xgeSvgCssIsSpace(*p) ) {
			bHadSpace = 1;
			p++;
		}
		if ( p >= pEnd ) {
			break;
		}
		if ( (*p == '>') || (*p == '+') || (*p == '~') ) {
			if ( bNeedSimple ) {
				return 0;
			}
			bNeedSimple = 1;
			p++;
			continue;
		}
		if ( !bNeedSimple && !bHadSpace ) {
			return 0;
		}
		pSimpleStart = p;
		{
			int bInAttr = 0;
			int iParenDepth = 0;
			char cQuote = 0;

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
				if ( bInAttr ) {
					if ( (*p == '"') || (*p == '\'') ) {
						cQuote = *p++;
						continue;
					}
					if ( *p == ']' ) {
						bInAttr = 0;
					}
					p++;
					continue;
				}
				if ( *p == '[' ) {
					bInAttr = 1;
					p++;
					continue;
				}
				if ( *p == '(' ) {
					iParenDepth++;
					p++;
					continue;
				}
				if ( *p == ')' ) {
					if ( iParenDepth <= 0 ) {
						return 0;
					}
					iParenDepth--;
					p++;
					continue;
				}
				if ( (iParenDepth == 0) && (__xgeSvgCssIsSpace(*p) || (*p == '>') || (*p == '+') || (*p == '~')) ) {
					break;
				}
				p++;
			}
			if ( bInAttr || (cQuote != 0) || (iParenDepth != 0) ) {
				return 0;
			}
		}
		pSimpleEnd = p;
		if ( !__xgeSvgCssSimpleSelectorSpecificity(pSimpleStart, pSimpleEnd, &iSimpleSpecificity) ) {
			return 0;
		}
		iSpecificity += iSimpleSpecificity;
		bNeedSimple = 0;
	}
	if ( bNeedSimple ) {
		return 0;
	}
	*pSpecificity = iSpecificity;
	return 1;
}

static int __xgeSvgCssRelativeSelectorSpecificity(const char* pStart, const char* pEnd, int* pSpecificity)
{
	if ( (pStart == NULL) || (pEnd == NULL) || (pSpecificity == NULL) ) {
		return 0;
	}
	__xgeSvgTrimRange(&pStart, &pEnd);
	if ( pEnd <= pStart ) {
		return 0;
	}
	if ( (*pStart == '>') || (*pStart == '+') || (*pStart == '~') ) {
		pStart++;
		__xgeSvgTrimRange(&pStart, &pEnd);
		if ( pEnd <= pStart ) {
			return 0;
		}
	}
	return __xgeSvgCssSelectorSpecificity(pStart, pEnd, pSpecificity);
}

static int __xgeSvgCssSelectorListSpecificityEx(const char* pStart, const char* pEnd, int* pSpecificity, int bRelative)
{
	const char* p;
	const char* pItemStart;
	int iBestSpecificity;
	int iItemCount;
	int iParenDepth;
	int bInAttr;
	char cQuote;

	if ( (pStart == NULL) || (pEnd == NULL) || (pSpecificity == NULL) ) {
		return 0;
	}
	__xgeSvgTrimRange(&pStart, &pEnd);
	if ( pEnd <= pStart ) {
		return 0;
	}
	p = pStart;
	pItemStart = pStart;
	iBestSpecificity = 0;
	iItemCount = 0;
	iParenDepth = 0;
	bInAttr = 0;
	cQuote = 0;
	while ( p <= pEnd ) {
		int bAtEnd;
		int bSplit;

		bAtEnd = (p == pEnd);
		bSplit = bAtEnd || ((cQuote == 0) && !bInAttr && (iParenDepth == 0) && (*p == ','));
		if ( bSplit ) {
			const char* pItemEnd;
			int iItemSpecificity;

			pItemEnd = p;
			__xgeSvgTrimRange(&pItemStart, &pItemEnd);
			if ( pItemEnd <= pItemStart ) {
				return 0;
			}
			if ( bRelative ) {
				if ( !__xgeSvgCssRelativeSelectorSpecificity(pItemStart, pItemEnd, &iItemSpecificity) ) {
					return 0;
				}
			} else if ( !__xgeSvgCssSelectorSpecificity(pItemStart, pItemEnd, &iItemSpecificity) ) {
				return 0;
			}
			if ( iItemSpecificity > iBestSpecificity ) {
				iBestSpecificity = iItemSpecificity;
			}
			iItemCount++;
			if ( bAtEnd ) {
				break;
			}
			p++;
			pItemStart = p;
			continue;
		}
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
		if ( bInAttr ) {
			if ( *p == ']' ) {
				bInAttr = 0;
			}
			p++;
			continue;
		}
		if ( *p == '[' ) {
			bInAttr = 1;
			p++;
			continue;
		}
		if ( *p == '(' ) {
			iParenDepth++;
			p++;
			continue;
		}
		if ( *p == ')' ) {
			if ( iParenDepth <= 0 ) {
				return 0;
			}
			iParenDepth--;
			p++;
			continue;
		}
		p++;
	}
	if ( (iItemCount <= 0) || bInAttr || (cQuote != 0) || (iParenDepth != 0) ) {
		return 0;
	}
	*pSpecificity = iBestSpecificity;
	return 1;
}

static int __xgeSvgCssSelectorListSpecificity(const char* pStart, const char* pEnd, int* pSpecificity)
{
	return __xgeSvgCssSelectorListSpecificityEx(pStart, pEnd, pSpecificity, 0);
}

static int __xgeSvgCssRelativeSelectorListSpecificity(const char* pStart, const char* pEnd, int* pSpecificity)
{
	return __xgeSvgCssSelectorListSpecificityEx(pStart, pEnd, pSpecificity, 1);
}

static int __xgeSvgCssSelectorSupported(const char* pStart, const char* pEnd)
{
	int iSpecificity;

	return __xgeSvgCssSelectorSpecificity(pStart, pEnd, &iSpecificity);
}

static int __xgeSvgStyleSelectorSpecificity(int iSelectorType, const char* sName)
{
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
	if ( !__xgeSvgCssSelectorSpecificity(sName, sName + strlen(sName), &iSpecificity) ) {
		return 0;
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
	int iParenDepth;
	int bInAttr;
	char cQuote;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pStart == NULL) || (pEnd == NULL) || (pStyleStart == NULL) || (pStyleEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	p = pStart;
	pItemStart = pStart;
	iParenDepth = 0;
	bInAttr = 0;
	cQuote = 0;
	while ( p <= pEnd ) {
		if ( p < pEnd ) {
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
			if ( bInAttr ) {
				if ( *p == ']' ) {
					bInAttr = 0;
				}
				p++;
				continue;
			}
			if ( *p == '[' ) {
				bInAttr = 1;
				p++;
				continue;
			}
			if ( *p == '(' ) {
				iParenDepth++;
				p++;
				continue;
			}
			if ( *p == ')' ) {
				if ( iParenDepth > 0 ) {
					iParenDepth--;
				}
				p++;
				continue;
			}
		}
		if ( (p == pEnd) || ((*p == ',') && (iParenDepth == 0) && !bInAttr && (cQuote == 0)) ) {
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

static void __xgeSvgAttrOutByte(char* sOut, int iOutCapacity, int* pWrite, char c)
{
	if ( (sOut == NULL) || (pWrite == NULL) || (iOutCapacity <= 0) ) {
		return;
	}
	if ( *pWrite < (iOutCapacity - 1) ) {
		sOut[*pWrite] = c;
	}
	(*pWrite)++;
}

static void __xgeSvgAttrOutUtf8(char* sOut, int iOutCapacity, int* pWrite, uint32_t iCodepoint)
{
	if ( iCodepoint <= 0x7Fu ) {
		__xgeSvgAttrOutByte(sOut, iOutCapacity, pWrite, (char)iCodepoint);
	} else if ( iCodepoint <= 0x7FFu ) {
		__xgeSvgAttrOutByte(sOut, iOutCapacity, pWrite, (char)(0xC0u | (iCodepoint >> 6)));
		__xgeSvgAttrOutByte(sOut, iOutCapacity, pWrite, (char)(0x80u | (iCodepoint & 0x3Fu)));
	} else if ( iCodepoint <= 0xFFFFu ) {
		if ( (iCodepoint >= 0xD800u) && (iCodepoint <= 0xDFFFu) ) {
			return;
		}
		__xgeSvgAttrOutByte(sOut, iOutCapacity, pWrite, (char)(0xE0u | (iCodepoint >> 12)));
		__xgeSvgAttrOutByte(sOut, iOutCapacity, pWrite, (char)(0x80u | ((iCodepoint >> 6) & 0x3Fu)));
		__xgeSvgAttrOutByte(sOut, iOutCapacity, pWrite, (char)(0x80u | (iCodepoint & 0x3Fu)));
	} else if ( iCodepoint <= 0x10FFFFu ) {
		__xgeSvgAttrOutByte(sOut, iOutCapacity, pWrite, (char)(0xF0u | (iCodepoint >> 18)));
		__xgeSvgAttrOutByte(sOut, iOutCapacity, pWrite, (char)(0x80u | ((iCodepoint >> 12) & 0x3Fu)));
		__xgeSvgAttrOutByte(sOut, iOutCapacity, pWrite, (char)(0x80u | ((iCodepoint >> 6) & 0x3Fu)));
		__xgeSvgAttrOutByte(sOut, iOutCapacity, pWrite, (char)(0x80u | (iCodepoint & 0x3Fu)));
	}
}

static int __xgeSvgAttrEntityCodepoint(const char* pStart, const char* pEnd, uint32_t* pCodepoint)
{
	uint32_t iValue;
	int iBase;
	int bAny;

	if ( (pStart == NULL) || (pEnd == NULL) || (pCodepoint == NULL) || (pEnd <= pStart) ) {
		return 0;
	}
	if ( ((pEnd - pStart) == 3) && (strncmp(pStart, "amp", 3) == 0) ) {
		*pCodepoint = '&';
		return 1;
	}
	if ( ((pEnd - pStart) == 2) && (strncmp(pStart, "lt", 2) == 0) ) {
		*pCodepoint = '<';
		return 1;
	}
	if ( ((pEnd - pStart) == 2) && (strncmp(pStart, "gt", 2) == 0) ) {
		*pCodepoint = '>';
		return 1;
	}
	if ( ((pEnd - pStart) == 4) && (strncmp(pStart, "quot", 4) == 0) ) {
		*pCodepoint = '"';
		return 1;
	}
	if ( ((pEnd - pStart) == 4) && (strncmp(pStart, "apos", 4) == 0) ) {
		*pCodepoint = '\'';
		return 1;
	}
	if ( ((pEnd - pStart) == 4) && (strncmp(pStart, "nbsp", 4) == 0) ) {
		*pCodepoint = 0xA0u;
		return 1;
	}
	if ( *pStart != '#' ) {
		return 0;
	}
	pStart++;
	iBase = 10;
	if ( (pStart < pEnd) && ((*pStart == 'x') || (*pStart == 'X')) ) {
		iBase = 16;
		pStart++;
	}
	iValue = 0;
	bAny = 0;
	while ( pStart < pEnd ) {
		int iDigit;
		if ( (*pStart >= '0') && (*pStart <= '9') ) {
			iDigit = *pStart - '0';
		} else if ( (*pStart >= 'a') && (*pStart <= 'f') ) {
			iDigit = *pStart - 'a' + 10;
		} else if ( (*pStart >= 'A') && (*pStart <= 'F') ) {
			iDigit = *pStart - 'A' + 10;
		} else {
			return 0;
		}
		if ( iDigit >= iBase ) {
			return 0;
		}
		if ( iValue > ((0x10FFFFu - (uint32_t)iDigit) / (uint32_t)iBase) ) {
			return 0;
		}
		iValue = (iValue * (uint32_t)iBase) + (uint32_t)iDigit;
		bAny = 1;
		pStart++;
	}
	if ( !bAny || (iValue == 0) || (iValue > 0x10FFFFu) ||
	     ((iValue >= 0xD800u) && (iValue <= 0xDFFFu)) ) {
		return 0;
	}
	*pCodepoint = iValue;
	return 1;
}

static void __xgeSvgAttrCopyDecoded(const char* pValue, const char* pValueEnd, char* sOut, int iOutCapacity)
{
	int iWrite;

	if ( (sOut == NULL) || (iOutCapacity <= 0) ) {
		return;
	}
	iWrite = 0;
	while ( (pValue != NULL) && (pValue < pValueEnd) ) {
		if ( *pValue == '&' ) {
			const char* pEntityStart = pValue + 1;
			const char* pSemi = pEntityStart;
			uint32_t iCodepoint;

			while ( (pSemi < pValueEnd) && (*pSemi != ';') && ((pSemi - pEntityStart) <= 16) ) {
				pSemi++;
			}
			if ( (pSemi < pValueEnd) && (*pSemi == ';') &&
			     __xgeSvgAttrEntityCodepoint(pEntityStart, pSemi, &iCodepoint) ) {
				__xgeSvgAttrOutUtf8(sOut, iOutCapacity, &iWrite, iCodepoint);
				pValue = pSemi + 1;
				continue;
			}
		}
		__xgeSvgAttrOutByte(sOut, iOutCapacity, &iWrite, *pValue);
		pValue++;
	}
	sOut[iWrite < iOutCapacity ? iWrite : (iOutCapacity - 1)] = '\0';
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
			__xgeSvgAttrCopyDecoded(pValue, pValueEnd, sOut, iOutCapacity);
			return 1;
		}
		p++;
	}
	return 0;
}

static int __xgeSvgHrefSpaceLen(const char* pText)
{
	if ( pText == NULL ) {
		return 0;
	}
	if ( (*pText == ' ') || (*pText == '\t') || (*pText == '\r') || (*pText == '\n') || (*pText == '\f') ) {
		return 1;
	}
	if ( ((unsigned char)pText[0] == 0xC2u) && ((unsigned char)pText[1] == 0xA0u) ) {
		return 2;
	}
	return 0;
}

static int __xgeSvgHrefCopyTrimmed(const char* pTag, const char* pTagEnd, char* sOut, int iOutCapacity)
{
	char sValue[XGE_SVG_ATTR_MAX];
	const char* pStart;
	const char* pEnd;
	char cQuote;
	int iLen;

	if ( (sOut == NULL) || (iOutCapacity <= 0) ) {
		return 0;
	}
	sOut[0] = '\0';
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "href", sValue, sizeof(sValue)) &&
	     !__xgeSvgAttrCopy(pTag, pTagEnd, "xlink:href", sValue, sizeof(sValue)) ) {
		return 0;
	}
	pStart = sValue;
	while ( __xgeSvgHrefSpaceLen(pStart) > 0 ) {
		pStart += __xgeSvgHrefSpaceLen(pStart);
	}
	pEnd = sValue + strlen(sValue);
	while ( pEnd > pStart ) {
		if ( __xgeSvgHrefSpaceLen(pEnd - 1) == 1 ) {
			pEnd--;
		} else if ( ((pEnd - pStart) >= 2) &&
		            ((unsigned char)pEnd[-2] == 0xC2u) && ((unsigned char)pEnd[-1] == 0xA0u) ) {
			pEnd -= 2;
		} else {
			break;
		}
	}
	if ( ((pEnd - pStart) >= 5) &&
	     (__xgeSvgCssAsciiLower((unsigned char)pStart[0]) == 'u') &&
	     (__xgeSvgCssAsciiLower((unsigned char)pStart[1]) == 'r') &&
	     (__xgeSvgCssAsciiLower((unsigned char)pStart[2]) == 'l') &&
	     (pStart[3] == '(') ) {
		pStart += 4;
		while ( __xgeSvgHrefSpaceLen(pStart) > 0 ) {
			pStart += __xgeSvgHrefSpaceLen(pStart);
		}
		while ( (pEnd > pStart) && __xgeSvgHrefSpaceLen(pEnd - 1) == 1 ) {
			pEnd--;
		}
		if ( (pEnd > pStart) && (pEnd[-1] == ')') ) {
			pEnd--;
		}
		while ( (pEnd > pStart) && __xgeSvgHrefSpaceLen(pEnd - 1) == 1 ) {
			pEnd--;
		}
		cQuote = 0;
		if ( (pStart < pEnd) && ((*pStart == '"') || (*pStart == '\'')) ) {
			cQuote = *pStart++;
		}
		if ( (cQuote != 0) && (pEnd > pStart) && (pEnd[-1] == cQuote) ) {
			pEnd--;
		}
	}
	iLen = (int)(pEnd - pStart);
	if ( iLen <= 0 ) {
		return 0;
	}
	if ( iLen >= iOutCapacity ) {
		iLen = iOutCapacity - 1;
	}
	memcpy(sOut, pStart, (size_t)iLen);
	sOut[iLen] = '\0';
	return 1;
}

static int __xgeSvgHrefIsLocalRef(const char* sHref)
{
	return (sHref != NULL) && (sHref[0] == '#');
}

static int __xgeSvgHrefHasFragment(const char* sHref)
{
	const char* p;

	if ( sHref == NULL ) {
		return 0;
	}
	for ( p = sHref; *p != '\0'; p++ ) {
		if ( *p == '#' ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeSvgHrefLooksExternalSvgRoot(const char* sHref)
{
	if ( (sHref == NULL) || (sHref[0] == '\0') || __xgeSvgHrefIsLocalRef(sHref) || __xgeSvgHrefHasFragment(sHref) ) {
		return 0;
	}
	if ( __xgeSvgUriHasExt(sHref, ".svg") ) {
		return 1;
	}
	return strncmp(sHref, "data:image/svg+xml", 18) == 0;
}

static int __xgeSvgHrefSplitExternalFragment(const char* sHref, char* sUriOut, int iUriOutCapacity, char* sIdOut, int iIdOutCapacity)
{
	const char* pHash;
	int iUriLen;
	int iIdLen;

	if ( (sHref == NULL) || (sUriOut == NULL) || (iUriOutCapacity <= 0) || (sIdOut == NULL) || (iIdOutCapacity <= 0) ) {
		return 0;
	}
	sUriOut[0] = '\0';
	sIdOut[0] = '\0';
	if ( __xgeSvgHrefIsLocalRef(sHref) ) {
		return 0;
	}
	pHash = strchr(sHref, '#');
	if ( (pHash == NULL) || (pHash == sHref) || (pHash[1] == '\0') ) {
		return 0;
	}
	iUriLen = (int)(pHash - sHref);
	iIdLen = (int)strlen(pHash + 1);
	if ( (iUriLen <= 0) || (iIdLen <= 0) ) {
		return 0;
	}
	if ( iUriLen >= iUriOutCapacity ) {
		iUriLen = iUriOutCapacity - 1;
	}
	if ( iIdLen >= iIdOutCapacity ) {
		iIdLen = iIdOutCapacity - 1;
	}
	memcpy(sUriOut, sHref, (size_t)iUriLen);
	sUriOut[iUriLen] = '\0';
	memcpy(sIdOut, pHash + 1, (size_t)iIdLen);
	sIdOut[iIdLen] = '\0';
	return 1;
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
	if ( !__xgeSvgHrefCopyTrimmed(pTag, pTagEnd, sValue, sizeof(sValue)) ) {
		return 0;
	}
	pValue = sValue;
	if ( pValue[0] == '#' ) {
		pValue++;
	}
	iLen = 0;
	while ( (pValue[iLen] != '\0') && (pValue[iLen] != ')') &&
	        (pValue[iLen] != ' ') && (pValue[iLen] != '\t') &&
	        (pValue[iLen] != '\r') && (pValue[iLen] != '\n') &&
	        !(((unsigned char)pValue[iLen] == 0xC2u) && ((unsigned char)pValue[iLen + 1] == 0xA0u)) ) {
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

static int __xgeSvgCssAsciiLower(int c)
{
	if ( (c >= 'A') && (c <= 'Z') ) {
		return c + ('a' - 'A');
	}
	return c;
}

static int __xgeSvgCssRangeEqualsText(const char* pStart, const char* pEnd, const char* sText)
{
	if ( (pStart == NULL) || (pEnd == NULL) || (sText == NULL) || (pEnd < pStart) ) {
		return 0;
	}
	while ( (pStart < pEnd) && (*sText != '\0') ) {
		if ( __xgeSvgCssAsciiLower((unsigned char)*pStart) != __xgeSvgCssAsciiLower((unsigned char)*sText) ) {
			return 0;
		}
		pStart++;
		sText++;
	}
	return (pStart == pEnd) && (*sText == '\0');
}

static int __xgeSvgStyleValueCleanCopy(const char* pValueStart, const char* pValueEnd, char* sOut, int iOutCapacity, int* pImportant)
{
	char sTemp[XGE_SVG_ATTR_MAX];
	int iLen;
	int iEnd;
	int iKeywordStart;
	int iBang;
	int iValueEnd;
	const char* p;

	if ( (pValueStart == NULL) || (pValueEnd == NULL) || (sOut == NULL) || (iOutCapacity <= 0) || (pImportant == NULL) ) {
		return 0;
	}
	iLen = 0;
	p = pValueStart;
	while ( p < pValueEnd ) {
		if ( ((p + 1) < pValueEnd) && (p[0] == '/') && (p[1] == '*') ) {
			p += 2;
			while ( ((p + 1) < pValueEnd) && !((p[0] == '*') && (p[1] == '/')) ) p++;
			if ( (p + 1) < pValueEnd ) p += 2;
			if ( (iLen > 0) && (iLen < (int)sizeof(sTemp) - 1) && (sTemp[iLen - 1] != ' ') ) {
				sTemp[iLen++] = ' ';
			}
			continue;
		}
		if ( (iLen == 0) && ((*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n')) ) {
			p++;
			continue;
		}
		if ( iLen < (int)sizeof(sTemp) - 1 ) {
			sTemp[iLen++] = *p;
		}
		p++;
	}
	while ( (iLen > 0) && ((sTemp[iLen - 1] == ' ') || (sTemp[iLen - 1] == '\t') || (sTemp[iLen - 1] == '\r') || (sTemp[iLen - 1] == '\n')) ) {
		iLen--;
	}
	sTemp[iLen] = '\0';
	*pImportant = 0;
	iEnd = iLen;
	while ( (iEnd > 0) && ((sTemp[iEnd - 1] == ' ') || (sTemp[iEnd - 1] == '\t') || (sTemp[iEnd - 1] == '\r') || (sTemp[iEnd - 1] == '\n')) ) {
		iEnd--;
	}
	iKeywordStart = iEnd - 9;
	if ( (iKeywordStart >= 0) && __xgeSvgCssRangeEqualsText(sTemp + iKeywordStart, sTemp + iEnd, "important") ) {
		iBang = iKeywordStart;
		while ( (iBang > 0) && ((sTemp[iBang - 1] == ' ') || (sTemp[iBang - 1] == '\t') || (sTemp[iBang - 1] == '\r') || (sTemp[iBang - 1] == '\n')) ) {
			iBang--;
		}
		if ( (iBang > 0) && (sTemp[iBang - 1] == '!') ) {
			*pImportant = 1;
			iLen = iBang - 1;
			while ( (iLen > 0) && ((sTemp[iLen - 1] == ' ') || (sTemp[iLen - 1] == '\t') || (sTemp[iLen - 1] == '\r') || (sTemp[iLen - 1] == '\n')) ) {
				iLen--;
			}
			sTemp[iLen] = '\0';
		}
	}
	iValueEnd = iLen;
	if ( iValueEnd >= iOutCapacity ) {
		iValueEnd = iOutCapacity - 1;
	}
	memcpy(sOut, sTemp, (size_t)iValueEnd);
	sOut[iValueEnd] = '\0';
	return 1;
}

static int __xgeSvgStyleCopyEx(const char* sStyle, const char* sName, char* sOut, int iOutCapacity, int iImportanceMode)
{
	const char* p;
	size_t iNameLen;
	int bFound;

	if ( (sStyle == NULL) || (sName == NULL) || (sOut == NULL) || (iOutCapacity <= 0) ) {
		return 0;
	}
	sOut[0] = '\0';
	iNameLen = strlen(sName);
	p = sStyle;
	bFound = 0;
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
				char sValue[XGE_SVG_ATTR_MAX];
				int bImportant;

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
				if ( __xgeSvgStyleValueCleanCopy(pValue, q, sValue, sizeof(sValue), &bImportant) &&
				     ((iImportanceMode == XGE_SVG_STYLE_IMPORTANCE_ANY) || (iImportanceMode == bImportant)) ) {
					snprintf(sOut, (size_t)iOutCapacity, "%s", sValue);
					sOut[iOutCapacity - 1] = '\0';
					bFound = 1;
				}
			}
		}
		while ( (*p != '\0') && (*p != ';') ) p++;
	}
	return bFound;
}

static int __xgeSvgStyleCopy(const char* sStyle, const char* sName, char* sOut, int iOutCapacity)
{
	return __xgeSvgStyleCopyEx(sStyle, sName, sOut, iOutCapacity, XGE_SVG_STYLE_IMPORTANCE_ANY);
}

static int __xgeSvgAttrOrStyleCopy(const char* pTag, const char* pTagEnd, const char* sStyle, const char* sName, char* sOut, int iOutCapacity)
{
	if ( __xgeSvgStyleCopy(sStyle, sName, sOut, iOutCapacity) ) {
		return 1;
	}
	return __xgeSvgAttrCopy(pTag, pTagEnd, sName, sOut, iOutCapacity);
}

static int __xgeSvgDoubleIsFiniteFloat(double fValue)
{
	if ( fValue != fValue ) {
		return 0;
	}
	if ( (fValue < -XGE_SVG_FLOAT_ABS_MAX) || (fValue > XGE_SVG_FLOAT_ABS_MAX) ) {
		return 0;
	}
	return 1;
}

static int __xgeSvgDoubleToFloat(double fValue, float* pValue)
{
	if ( (pValue == NULL) || !__xgeSvgDoubleIsFiniteFloat(fValue) ) {
		return 0;
	}
	*pValue = (float)fValue;
	return 1;
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
	return __xgeSvgDoubleToFloat(fValue, pValue);
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
	return __xgeSvgDoubleToFloat(fValue, pValue);
}

static float __xgeSvgParseStopOffset(const char* sText)
{
	const char* p;
	const char* pPercent;
	const char* pTextEnd;
	char* pEnd;
	double fValue;

	if ( sText == NULL ) {
		return 0.0f;
	}
	fValue = strtod(sText, &pEnd);
	if ( pEnd == sText ) {
		return 0.0f;
	}
	if ( !__xgeSvgDoubleIsFiniteFloat(fValue) ) {
		return 0.0f;
	}
	p = pEnd;
	__xgeSvgSkipSpaces(&p);
	pTextEnd = sText + strlen(sText);
	pPercent = strchr(sText, '%');
	if ( pPercent != NULL ) {
		if ( (p != pPercent) || ((pPercent + 1) != pTextEnd) ) {
			return 0.0f;
		}
		fValue /= 100.0;
	} else if ( p != pTextEnd ) {
		return 0.0f;
	}
	return (float)fValue;
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
		fValue = 1.0;
	} else if ( (pEnd[0] == '%') && (pEnd[1] == '\0') ) {
		fValue /= 100.0;
	} else if ( pEnd[0] != '\0' ) {
		fValue = 1.0;
	} else if ( !__xgeSvgDoubleIsFiniteFloat(fValue) ) {
		fValue = 1.0;
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

static int __xgeSvgParseLengthTokenEx(const char** ppText, float fPercentRef, float fFontSize, float* pValue)
{
	const char* p;
	char* pNumberEnd;
	double fValue;
	float fScale;

	if ( (ppText == NULL) || (*ppText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	if ( fFontSize <= 0.0f ) {
		fFontSize = 16.0f;
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
	} else if ( __xgeSvgUnitStartsWith(p, "em") ) {
		fScale = fFontSize;
		p += 2;
	} else if ( __xgeSvgUnitStartsWith(p, "ex") ) {
		fScale = fFontSize * 0.5f;
		p += 2;
	} else if ( ((*p >= 'a') && (*p <= 'z')) || ((*p >= 'A') && (*p <= 'Z')) ) {
		return 0;
	}
	fValue *= (double)fScale;
	if ( !__xgeSvgDoubleToFloat(fValue, pValue) ) {
		return 0;
	}
	*ppText = p;
	return 1;
}

static int __xgeSvgParseLengthToken(const char** ppText, float fPercentRef, float* pValue)
{
	return __xgeSvgParseLengthTokenEx(ppText, fPercentRef, 16.0f, pValue);
}

static int __xgeSvgParseLengthEx(const char* sText, float fPercentRef, float fFontSize, float* pValue)
{
	const char* p;

	if ( (sText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	p = sText;
	if ( !__xgeSvgParseLengthTokenEx(&p, fPercentRef, fFontSize, pValue) ) {
		return 0;
	}
	while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') ) p++;
	return *p == '\0';
}

static int __xgeSvgParseLength(const char* sText, float fPercentRef, float* pValue)
{
	return __xgeSvgParseLengthEx(sText, fPercentRef, 16.0f, pValue);
}

static int __xgeSvgParseFontSize(const char* sText, float fInheritedFontSize, float* pValue)
{
	const char* pStart;
	const char* pEnd;
	float fValue;

	if ( (sText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	if ( fInheritedFontSize <= 0.0f ) {
		fInheritedFontSize = 16.0f;
	}
	pStart = sText;
	while ( (*pStart == ' ') || (*pStart == '\t') || (*pStart == '\r') || (*pStart == '\n') ) pStart++;
	pEnd = pStart + strlen(pStart);
	while ( (pEnd > pStart) && ((pEnd[-1] == ' ') || (pEnd[-1] == '\t') || (pEnd[-1] == '\r') || (pEnd[-1] == '\n')) ) pEnd--;
	if ( pEnd == pStart ) {
		return 0;
	}
	if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "xx-small") ) fValue = 9.0f;
	else if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "x-small") ) fValue = 10.0f;
	else if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "small") ) fValue = 13.0f;
	else if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "medium") ) fValue = 16.0f;
	else if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "large") ) fValue = 18.0f;
	else if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "x-large") ) fValue = 24.0f;
	else if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "xx-large") ) fValue = 32.0f;
	else if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "xxx-large") ) fValue = 48.0f;
	else if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "smaller") ) fValue = fInheritedFontSize / 1.2f;
	else if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "larger") ) fValue = fInheritedFontSize * 1.2f;
	else if ( !__xgeSvgParseLengthEx(pStart, fInheritedFontSize, fInheritedFontSize, &fValue) ) return 0;
	if ( fValue <= 0.0f ) {
		return 0;
	}
	*pValue = fValue;
	return 1;
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

static float __xgeSvgAttrLengthRefEx(const char* pTag, const char* pTagEnd, const char* sName, float fPercentRef, float fFontSize, float fDefault)
{
	char sValue[XGE_SVG_ATTR_MAX];
	float fValue;

	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, sName, sValue, sizeof(sValue)) ) {
		return fDefault;
	}
	return __xgeSvgParseLengthEx(sValue, fPercentRef, fFontSize, &fValue) ? fValue : fDefault;
}

static float __xgeSvgAttrLengthRef(const char* pTag, const char* pTagEnd, const char* sName, float fPercentRef, float fDefault)
{
	return __xgeSvgAttrLengthRefEx(pTag, pTagEnd, sName, fPercentRef, 16.0f, fDefault);
}

static int __xgeSvgAttrLengthCopyEx(const char* pTag, const char* pTagEnd, const char* sName, float fPercentRef, float fFontSize, float* pValue)
{
	char sValue[XGE_SVG_ATTR_MAX];

	if ( (pValue == NULL) || !__xgeSvgAttrCopy(pTag, pTagEnd, sName, sValue, sizeof(sValue)) ) {
		return 0;
	}
	return __xgeSvgParseLengthEx(sValue, fPercentRef, fFontSize, pValue);
}

static int __xgeSvgAttrLengthCopy(const char* pTag, const char* pTagEnd, const char* sName, float fPercentRef, float* pValue)
{
	return __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, sName, fPercentRef, 16.0f, pValue);
}

static float __xgeSvgAttrLengthEx(xge_svg pSvg, const char* pTag, const char* pTagEnd, const char* sName, int iBasis, float fFontSize, float fDefault)
{
	return __xgeSvgAttrLengthRefEx(pTag, pTagEnd, sName, __xgeSvgLengthPercentRef(pSvg, iBasis), fFontSize, fDefault);
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

static int __xgeSvgUrlIdCopyFromValueEx(const char* sValue, char* sOut, int iOutCapacity, const char** ppAfter)
{
	const char* p;
	const char* pIdStart;
	char cQuote;
	int iLen;
	int iCopyLen;

	if ( ppAfter != NULL ) {
		*ppAfter = sValue;
	}
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
	pIdStart = p;
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
	p += iLen;
	iCopyLen = iLen;
	if ( iCopyLen >= iOutCapacity ) {
		iCopyLen = iOutCapacity - 1;
	}
	memcpy(sOut, pIdStart, (size_t)iCopyLen);
	sOut[iCopyLen] = '\0';
	if ( cQuote != 0 ) {
		if ( *p != cQuote ) {
			return 0;
		}
		p++;
		while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') ) p++;
	}
	if ( *p != ')' ) {
		return 0;
	}
	p++;
	while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') ) p++;
	if ( ppAfter != NULL ) {
		*ppAfter = p;
	}
	return 1;
}

static int __xgeSvgUrlIdCopyFromValue(const char* sValue, char* sOut, int iOutCapacity)
{
	return __xgeSvgUrlIdCopyFromValueEx(sValue, sOut, iOutCapacity, NULL);
}

static void __xgeSvgFilterReset(xge_svg_filter_t* pFilter)
{
	if ( pFilter == NULL ) {
		return;
	}
	xrtFree(pFilter->sId);
	memset(pFilter, 0, sizeof(*pFilter));
}

static void __xgeSvgFiltersClear(xge_svg pSvg)
{
	int i;

	if ( !__xgeSvgValid(pSvg) ) {
		return;
	}
	for ( i = 0; i < pSvg->iFilterCount; i++ ) {
		__xgeSvgFilterReset(&pSvg->pFilters[i]);
	}
	xrtFree(pSvg->pFilters);
	pSvg->pFilters = NULL;
	pSvg->iFilterCount = 0;
	pSvg->iFilterCapacity = 0;
}

static int __xgeSvgFilterFind(xge_svg pSvg, const char* sId)
{
	int i;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') ) {
		return -1;
	}
	for ( i = 0; i < pSvg->iFilterCount; i++ ) {
		if ( (pSvg->pFilters[i].sId != NULL) && (strcmp(pSvg->pFilters[i].sId, sId) == 0) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeSvgFilterGetOrCreate(xge_svg pSvg, const char* sId, int* pIndex)
{
	xge_svg_filter_t* pFilters;
	int iIndex;
	int iCapacity;

	if ( !__xgeSvgValid(pSvg) || (sId == NULL) || (sId[0] == '\0') || (pIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xgeSvgFilterFind(pSvg, sId);
	if ( iIndex >= 0 ) {
		*pIndex = iIndex;
		return XGE_OK;
	}
	if ( pSvg->iFilterCount >= pSvg->iFilterCapacity ) {
		iCapacity = pSvg->iFilterCapacity > 0 ? pSvg->iFilterCapacity * 2 : 8;
		pFilters = (xge_svg_filter_t*)xrtRealloc(pSvg->pFilters, (size_t)iCapacity * sizeof(*pFilters));
		if ( pFilters == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pFilters + pSvg->iFilterCapacity, 0, (size_t)(iCapacity - pSvg->iFilterCapacity) * sizeof(*pFilters));
		pSvg->pFilters = pFilters;
		pSvg->iFilterCapacity = iCapacity;
	}
	iIndex = pSvg->iFilterCount++;
	pSvg->pFilters[iIndex].sId = __xgeStrDup(sId);
	if ( pSvg->pFilters[iIndex].sId == NULL ) {
		pSvg->iFilterCount--;
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSvg->pFilters[iIndex].iUnits = XGE_SVG_FILTER_UNITS_USER_SPACE;
	pSvg->pFilters[iIndex].iFilterUnits = XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX;
	pSvg->pFilters[iIndex].tRegion = (xge_rect_t){-0.1f, -0.1f, 1.2f, 1.2f};
	*pIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgParseFilter(xge_svg pSvg, const char* pTag, const char* pTagEnd, int* pFilterIndex)
{
	char sId[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	xge_svg_filter_t* pFilter;
	int iIndex;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pFilterIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pFilterIndex = -1;
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "id", sId, sizeof(sId)) || (sId[0] == '\0') ) {
		return XGE_OK;
	}
	iRet = __xgeSvgFilterGetOrCreate(pSvg, sId, &iIndex);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pFilter = &pSvg->pFilters[iIndex];
	pFilter->fOffsetX = 0.0f;
	pFilter->fOffsetY = 0.0f;
	pFilter->bHasOffset = 0;
	pFilter->fDropShadowX = 2.0f;
	pFilter->fDropShadowY = 2.0f;
	pFilter->fDropShadowStdDeviation = 2.0f;
	pFilter->fDropShadowOpacity = 1.0f;
	pFilter->iDropShadowColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	pFilter->bHasDropShadow = 0;
	pFilter->iUnits = XGE_SVG_FILTER_UNITS_USER_SPACE;
	pFilter->iFilterUnits = XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX;
	pFilter->tRegion = (xge_rect_t){-0.1f, -0.1f, 1.2f, 1.2f};
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "filterUnits", sValue, sizeof(sValue)) ) {
		if ( strcmp(sValue, "userSpaceOnUse") == 0 ) {
			pFilter->iFilterUnits = XGE_SVG_FILTER_UNITS_USER_SPACE;
		} else if ( strcmp(sValue, "objectBoundingBox") == 0 ) {
			pFilter->iFilterUnits = XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX;
		}
	}
	if ( pFilter->iFilterUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX ) {
		pFilter->tRegion.fX = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "x", 1.0f, 16.0f, -0.1f);
		pFilter->tRegion.fY = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "y", 1.0f, 16.0f, -0.1f);
		pFilter->tRegion.fW = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "width", 1.0f, 16.0f, 1.2f);
		pFilter->tRegion.fH = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "height", 1.0f, 16.0f, 1.2f);
	} else {
		float fRefX = __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_X);
		float fRefY = __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_Y);

		pFilter->tRegion.fX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, 16.0f, -0.1f * fRefX);
		pFilter->tRegion.fY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, 16.0f, -0.1f * fRefY);
		pFilter->tRegion.fW = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, 16.0f, 1.2f * fRefX);
		pFilter->tRegion.fH = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, 16.0f, 1.2f * fRefY);
	}
	if ( pFilter->tRegion.fW < 0.0f ) pFilter->tRegion.fW = 0.0f;
	if ( pFilter->tRegion.fH < 0.0f ) pFilter->tRegion.fH = 0.0f;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "primitiveUnits", sValue, sizeof(sValue)) &&
	     (strcmp(sValue, "objectBoundingBox") == 0) ) {
		pFilter->iUnits = XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX;
	}
	*pFilterIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgParseFilterFeOffset(xge_svg pSvg, int iFilterIndex, const char* pTag, const char* pTagEnd)
{
	xge_svg_filter_t* pFilter;

	if ( !__xgeSvgValid(pSvg) || (iFilterIndex < 0) || (iFilterIndex >= pSvg->iFilterCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFilter = &pSvg->pFilters[iFilterIndex];
	if ( pFilter->iUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX ) {
		pFilter->fOffsetX = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "dx", 1.0f, 16.0f, 0.0f);
		pFilter->fOffsetY = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "dy", 1.0f, 16.0f, 0.0f);
	} else {
		pFilter->fOffsetX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "dx", XGE_SVG_LENGTH_BASIS_X, 16.0f, 0.0f);
		pFilter->fOffsetY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "dy", XGE_SVG_LENGTH_BASIS_Y, 16.0f, 0.0f);
	}
	pFilter->bHasOffset = 1;
	return XGE_OK;
}

static int __xgeSvgParseFilterFeDropShadow(xge_svg pSvg, int iFilterIndex, const char* pTag, const char* pTagEnd)
{
	char sStyle[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	const char* pValue;
	xge_svg_filter_t* pFilter;
	float fStdDeviation;

	if ( !__xgeSvgValid(pSvg) || (iFilterIndex < 0) || (iFilterIndex >= pSvg->iFilterCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFilter = &pSvg->pFilters[iFilterIndex];
	if ( pFilter->iUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX ) {
		pFilter->fDropShadowX = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "dx", 1.0f, 16.0f, 2.0f);
		pFilter->fDropShadowY = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "dy", 1.0f, 16.0f, 2.0f);
	} else {
		pFilter->fDropShadowX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "dx", XGE_SVG_LENGTH_BASIS_X, 16.0f, 2.0f);
		pFilter->fDropShadowY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "dy", XGE_SVG_LENGTH_BASIS_Y, 16.0f, 2.0f);
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "stdDeviation", sValue, sizeof(sValue)) ) {
		float fStdDeviationY;

		pValue = sValue;
		if ( __xgeSvgParseLengthTokenEx(&pValue, 1.0f, 16.0f, &fStdDeviation) && (fStdDeviation >= 0.0f) ) {
			fStdDeviationY = fStdDeviation;
			__xgeSvgSkipSpaces(&pValue);
			if ( __xgeSvgParseLengthTokenEx(&pValue, 1.0f, 16.0f, &fStdDeviationY) && (fStdDeviationY > fStdDeviation) ) {
				fStdDeviation = fStdDeviationY;
			}
			pFilter->fDropShadowStdDeviation = fStdDeviation;
		}
	}
	sStyle[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "style", sStyle, sizeof(sStyle));
	if ( __xgeSvgAttrOrStyleCopy(pTag, pTagEnd, sStyle, "flood-color", sValue, sizeof(sValue)) ) {
		if ( !__xgeSvgColorNameEquals(sValue, "currentColor") ) {
			__xgeSvgParseColor(sValue, &pFilter->iDropShadowColor);
		}
	}
	if ( __xgeSvgAttrOrStyleCopy(pTag, pTagEnd, sStyle, "flood-opacity", sValue, sizeof(sValue)) ) {
		__xgeSvgParseOpacity(sValue, &pFilter->fDropShadowOpacity);
	}
	pFilter->bHasDropShadow = 1;
	return XGE_OK;
}

static int __xgeSvgFilterOffsetForBounds(xge_svg pSvg, const char* sFilterId, xge_rect_t tBounds, float* pOffsetX, float* pOffsetY)
{
	xge_svg_filter_t* pFilter;
	int iFilter;

	if ( (pOffsetX == NULL) || (pOffsetY == NULL) ) {
		return 0;
	}
	*pOffsetX = 0.0f;
	*pOffsetY = 0.0f;
	if ( !__xgeSvgValid(pSvg) || (sFilterId == NULL) || (sFilterId[0] == '\0') ) {
		return 0;
	}
	iFilter = __xgeSvgFilterFind(pSvg, sFilterId);
	if ( iFilter < 0 ) {
		return 0;
	}
	pFilter = &pSvg->pFilters[iFilter];
	if ( !pFilter->bHasOffset ) {
		return 0;
	}
	*pOffsetX = pFilter->fOffsetX;
	*pOffsetY = pFilter->fOffsetY;
	if ( pFilter->iUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX ) {
		*pOffsetX *= tBounds.fW;
		*pOffsetY *= tBounds.fH;
	}
	return 1;
}

static int __xgeSvgFilterRegionForBounds(xge_svg pSvg, const char* sFilterId, xge_rect_t tBounds, xge_rect_t* pRegion)
{
	xge_svg_filter_t* pFilter;
	xge_rect_t tRegion;
	int iFilter;

	if ( pRegion == NULL ) {
		return 0;
	}
	memset(pRegion, 0, sizeof(*pRegion));
	if ( !__xgeSvgValid(pSvg) || (sFilterId == NULL) || (sFilterId[0] == '\0') ) {
		return 0;
	}
	iFilter = __xgeSvgFilterFind(pSvg, sFilterId);
	if ( iFilter < 0 ) {
		return 0;
	}
	pFilter = &pSvg->pFilters[iFilter];
	tRegion = pFilter->tRegion;
	if ( pFilter->iFilterUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX ) {
		tRegion = __xgeSvgObjectRectToBounds(tRegion, tBounds);
	}
	if ( (tRegion.fW <= 0.0f) || (tRegion.fH <= 0.0f) ) {
		return -1;
	}
	*pRegion = tRegion;
	return 1;
}

static int __xgeSvgFilterDropShadowForBounds(xge_svg pSvg, const char* sFilterId, xge_rect_t tBounds, float* pOffsetX, float* pOffsetY, uint32_t* pColor, float* pOpacity, float* pStdDeviation)
{
	xge_svg_filter_t* pFilter;
	int iFilter;

	if ( (pOffsetX == NULL) || (pOffsetY == NULL) || (pColor == NULL) || (pOpacity == NULL) || (pStdDeviation == NULL) ) {
		return 0;
	}
	*pOffsetX = 0.0f;
	*pOffsetY = 0.0f;
	*pColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	*pOpacity = 1.0f;
	*pStdDeviation = 0.0f;
	if ( !__xgeSvgValid(pSvg) || (sFilterId == NULL) || (sFilterId[0] == '\0') ) {
		return 0;
	}
	iFilter = __xgeSvgFilterFind(pSvg, sFilterId);
	if ( iFilter < 0 ) {
		return 0;
	}
	pFilter = &pSvg->pFilters[iFilter];
	if ( !pFilter->bHasDropShadow || (pFilter->fDropShadowOpacity <= 0.0f) || (XGE_COLOR_GET_A(pFilter->iDropShadowColor) <= 0) ) {
		return 0;
	}
	*pOffsetX = pFilter->fDropShadowX;
	*pOffsetY = pFilter->fDropShadowY;
	if ( pFilter->iUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX ) {
		*pOffsetX *= tBounds.fW;
		*pOffsetY *= tBounds.fH;
		*pStdDeviation = pFilter->fDropShadowStdDeviation * (tBounds.fW > tBounds.fH ? tBounds.fW : tBounds.fH);
	} else {
		*pStdDeviation = pFilter->fDropShadowStdDeviation;
	}
	*pColor = pFilter->iDropShadowColor;
	*pOpacity = pFilter->fDropShadowOpacity;
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

static uint32_t __xgeSvgFilterShadowColor(uint32_t iColor, float fOpacity, float fSourceOpacity)
{
	return __xgeSvgColorAlpha(iColor, fOpacity * fSourceOpacity);
}

static int __xgeSvgStyleHasPaintAlpha(const xge_svg_style_t* pStyle, int bStroke)
{
	if ( pStyle == NULL ) {
		return 0;
	}
	if ( bStroke ) {
		if ( pStyle->fStrokeWidth <= 0.0f || pStyle->fStrokeOpacity <= 0.0f ) {
			return 0;
		}
		return (pStyle->sStrokeGradientId[0] != '\0') || (XGE_COLOR_GET_A(pStyle->iStrokeColor) > 0);
	}
	if ( pStyle->fFillOpacity <= 0.0f ) {
		return 0;
	}
	return (pStyle->sFillGradientId[0] != '\0') || (XGE_COLOR_GET_A(pStyle->iFillColor) > 0);
}

static void __xgeSvgApplyDropShadowPaintToShape(xge_shape_ex pShape, const xge_svg_style_t* pStyle, uint32_t iColor, float fOpacity)
{
	uint32_t iFill;
	uint32_t iStroke;

	if ( (pShape == NULL) || (pStyle == NULL) ) {
		return;
	}
	iFill = __xgeSvgStyleHasPaintAlpha(pStyle, 0) ?
		__xgeSvgFilterShadowColor(iColor, fOpacity, pStyle->fOpacity * pStyle->fFillOpacity) :
		XGE_COLOR_RGBA(0, 0, 0, 0);
	iStroke = __xgeSvgStyleHasPaintAlpha(pStyle, 1) ?
		__xgeSvgFilterShadowColor(iColor, fOpacity, pStyle->fOpacity * pStyle->fStrokeOpacity) :
		XGE_COLOR_RGBA(0, 0, 0, 0);
	xgeShapeExFillColor(pShape, iFill);
	xgeShapeExStrokeColor(pShape, iStroke);
}

static void __xgeSvgApplyDropShadowPaintToText(xge_svg_text_item_t* pText, uint32_t iColor, float fOpacity)
{
	if ( pText == NULL ) {
		return;
	}
	pText->tStyle.iFillColor = __xgeSvgFilterShadowColor(iColor, fOpacity, 1.0f);
	pText->tStyle.sFillGradientId[0] = '\0';
	pText->tStyle.iStrokeColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	pText->tStyle.sStrokeGradientId[0] = '\0';
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

	if ( !__xgeSvgDoubleIsFiniteFloat(fValue) ) {
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
	if ( !__xgeSvgDoubleIsFiniteFloat(fValue) ) {
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
	if ( !__xgeSvgDoubleIsFiniteFloat(fValue) ) {
		return 0;
	}
	fValue = fmod(fValue, 360.0);
	if ( fValue < 0.0 ) fValue += 360.0;
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
	if ( !__xgeSvgDoubleIsFiniteFloat(fValue) ) {
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
	double fValue;
	float* arrValues[4];
	int i;

	if ( (sText == NULL) || (pA == NULL) || (pB == NULL) || (pC == NULL) || (pD == NULL) ) {
		return 0;
	}
	arrValues[0] = pA;
	arrValues[1] = pB;
	arrValues[2] = pC;
	arrValues[3] = pD;
	p = sText;
	for ( i = 0; i < 4; i++ ) {
		while ( __xgeSvgIsSpace((unsigned char)*p) || (*p == ',') ) p++;
		fValue = strtod(p, &pEnd);
		if ( pEnd == p ) {
			return 0;
		}
		if ( !__xgeSvgDoubleToFloat(fValue, arrValues[i]) ) {
			return 0;
		}
		p = pEnd;
	}
	while ( __xgeSvgIsSpace((unsigned char)*p) ) p++;
	if ( *p != '\0' ) {
		return 0;
	}
	return 1;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixIdentity(void)
{
	xge_shape_ex_matrix_t tMatrix;

	if ( xgeShapeExMatrixIdentity(&tMatrix) != XGE_OK ) {
		memset(&tMatrix, 0, sizeof(tMatrix));
		tMatrix.fA = 1.0f;
		tMatrix.fD = 1.0f;
	}
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixTranslate(float fX, float fY)
{
	xge_shape_ex_matrix_t tMatrix;

	xgeShapeExMatrixTranslate(&tMatrix, fX, fY);
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixScale(float fX, float fY)
{
	xge_shape_ex_matrix_t tMatrix;

	xgeShapeExMatrixScale(&tMatrix, fX, fY);
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixRotate(float fDegrees)
{
	xge_shape_ex_matrix_t tMatrix;
	float fRadians = fDegrees * (3.14159265358979323846f / 180.0f);

	xgeShapeExMatrixRotate(&tMatrix, fRadians);
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixSkewX(float fDegrees)
{
	xge_shape_ex_matrix_t tMatrix;
	float fRadians = fDegrees * (3.14159265358979323846f / 180.0f);

	xgeShapeExMatrixSkew(&tMatrix, fRadians, 0.0f);
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixSkewY(float fDegrees)
{
	xge_shape_ex_matrix_t tMatrix;
	float fRadians = fDegrees * (3.14159265358979323846f / 180.0f);

	xgeShapeExMatrixSkew(&tMatrix, 0.0f, fRadians);
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixMul(xge_shape_ex_matrix_t tParent, xge_shape_ex_matrix_t tLocal)
{
	xge_shape_ex_matrix_t tOut = tParent;

	xgeShapeExMatrixMultiply(&tOut, &tParent, &tLocal);
	return tOut;
}

static int __xgeSvgMatrixInvert(xge_shape_ex_matrix_t tMatrix, xge_shape_ex_matrix_t* pOut)
{
	return xgeShapeExMatrixInvert(pOut, &tMatrix) == XGE_OK;
}

static xge_vec2_t __xgeSvgMatrixPoint(xge_shape_ex_matrix_t tMatrix, xge_vec2_t tPoint)
{
	xge_vec2_t tOut;

	if ( xgeShapeExMatrixPoint(&tOut, &tMatrix, tPoint) != XGE_OK ) {
		return tPoint;
	}
	return tOut;
}

static xge_rect_t __xgeSvgMatrixRectBounds(xge_shape_ex_matrix_t tMatrix, xge_rect_t tRect)
{
	xge_rect_t tOut;

	if ( xgeShapeExMatrixRectBounds(&tOut, &tMatrix, tRect) != XGE_OK ) {
		return tRect;
	}
	return tOut;
}

static int __xgeSvgMatrixRectBoundsInverse(xge_shape_ex_matrix_t tMatrix, xge_rect_t tRect, xge_rect_t* pOut)
{
	xge_shape_ex_matrix_t tInverse;

	if ( pOut == NULL ) {
		return 0;
	}
	if ( xgeShapeExMatrixInvert(&tInverse, &tMatrix) != XGE_OK ) {
		return 0;
	}
	if ( xgeShapeExMatrixRectBounds(pOut, &tInverse, tRect) != XGE_OK ) {
		return 0;
	}
	return 1;
}

static int __xgeSvgRangeEqualsText(const char* pStart, const char* pEnd, const char* sText)
{
	size_t iLen;

	if ( (pStart == NULL) || (pEnd == NULL) || (sText == NULL) || (pEnd < pStart) ) {
		return 0;
	}
	iLen = strlen(sText);
	return ((size_t)(pEnd - pStart) == iLen) && (strncmp(pStart, sText, iLen) == 0);
}

static void __xgeSvgParsePreserveAspectRatioFields(const char* sValue, int* pAlignX, int* pAlignY, int* pMeetOrSlice)
{
	const char* p;
	const char* pValueStart;
	const char* pStart;
	const char* pEnd;
	int iAlignX;
	int iAlignY;

	if ( pAlignX != NULL ) *pAlignX = XGE_SVG_ASPECT_ALIGN_MID;
	if ( pAlignY != NULL ) *pAlignY = XGE_SVG_ASPECT_ALIGN_MID;
	if ( pMeetOrSlice != NULL ) *pMeetOrSlice = XGE_SVG_ASPECT_MEET;
	if ( (sValue == NULL) || (sValue[0] == '\0') ) {
		return;
	}
	p = sValue;
	__xgeSvgSkipSpaces(&p);
	pStart = p;
	pEnd = p + strlen(p);
	while ( (pEnd > pStart) && __xgeSvgIsSpace((unsigned char)pEnd[-1]) ) {
		pEnd--;
	}
	if ( __xgeSvgRangeEqualsText(pStart, pEnd, "none") ) {
		if ( pMeetOrSlice != NULL ) *pMeetOrSlice = XGE_SVG_ASPECT_NONE;
		return;
	}
	pValueStart = pStart;
	iAlignX = XGE_SVG_ASPECT_ALIGN_MID;
	iAlignY = XGE_SVG_ASPECT_ALIGN_MID;
	if ( strncmp(pStart, "xMinYMin", 8) == 0 ) {
		iAlignX = XGE_SVG_ASPECT_ALIGN_MIN;
		iAlignY = XGE_SVG_ASPECT_ALIGN_MIN;
		p = pStart + 8;
	} else if ( strncmp(pStart, "xMidYMin", 8) == 0 ) {
		iAlignX = XGE_SVG_ASPECT_ALIGN_MID;
		iAlignY = XGE_SVG_ASPECT_ALIGN_MIN;
		p = pStart + 8;
	} else if ( strncmp(pStart, "xMaxYMin", 8) == 0 ) {
		iAlignX = XGE_SVG_ASPECT_ALIGN_MAX;
		iAlignY = XGE_SVG_ASPECT_ALIGN_MIN;
		p = pStart + 8;
	} else if ( strncmp(pStart, "xMinYMid", 8) == 0 ) {
		iAlignX = XGE_SVG_ASPECT_ALIGN_MIN;
		iAlignY = XGE_SVG_ASPECT_ALIGN_MID;
		p = pStart + 8;
	} else if ( strncmp(pStart, "xMidYMid", 8) == 0 ) {
		iAlignX = XGE_SVG_ASPECT_ALIGN_MID;
		iAlignY = XGE_SVG_ASPECT_ALIGN_MID;
		p = pStart + 8;
	} else if ( strncmp(pStart, "xMaxYMid", 8) == 0 ) {
		iAlignX = XGE_SVG_ASPECT_ALIGN_MAX;
		iAlignY = XGE_SVG_ASPECT_ALIGN_MID;
		p = pStart + 8;
	} else if ( strncmp(pStart, "xMinYMax", 8) == 0 ) {
		iAlignX = XGE_SVG_ASPECT_ALIGN_MIN;
		iAlignY = XGE_SVG_ASPECT_ALIGN_MAX;
		p = pStart + 8;
	} else if ( strncmp(pStart, "xMidYMax", 8) == 0 ) {
		iAlignX = XGE_SVG_ASPECT_ALIGN_MID;
		iAlignY = XGE_SVG_ASPECT_ALIGN_MAX;
		p = pStart + 8;
	} else if ( strncmp(pStart, "xMaxYMax", 8) == 0 ) {
		iAlignX = XGE_SVG_ASPECT_ALIGN_MAX;
		iAlignY = XGE_SVG_ASPECT_ALIGN_MAX;
		p = pStart + 8;
	} else {
		iAlignX = XGE_SVG_ASPECT_ALIGN_MID;
		iAlignY = XGE_SVG_ASPECT_ALIGN_MID;
		p = pValueStart;
	}
	if ( pAlignX != NULL ) *pAlignX = iAlignX;
	if ( pAlignY != NULL ) *pAlignY = iAlignY;
	__xgeSvgSkipSpaces(&p);
	pStart = p;
	pEnd = p + strlen(p);
	while ( (pEnd > pStart) && __xgeSvgIsSpace((unsigned char)pEnd[-1]) ) {
		pEnd--;
	}
	if ( (pMeetOrSlice != NULL) && __xgeSvgRangeEqualsText(pStart, pEnd, "slice") ) {
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
	xge_shape_ex_matrix_t tMatrix = __xgeSvgMatrixIdentity();
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

static void __xgeSvgSkipTransformSeparators(const char** ppText)
{
	if ( ppText == NULL ) {
		return;
	}
	while ( __xgeSvgIsSpace((unsigned char)**ppText) || (**ppText == ',') ) {
		(*ppText)++;
	}
}

static int __xgeSvgParseTransformArgs(const char* sText, const char** ppNext, float* pValues, int iCapacity, int* pCount)
{
	const char* p;
	char* pEnd;
	double fValue;
	int iCount;

	if ( (sText == NULL) || (ppNext == NULL) || (pValues == NULL) || (iCapacity <= 0) || (pCount == NULL) ) {
		return 0;
	}
	p = sText;
	while ( __xgeSvgIsSpace((unsigned char)*p) ) p++;
	if ( *p != '(' ) {
		return 0;
	}
	p++;
	iCount = 0;
	for ( ;; ) {
		__xgeSvgSkipTransformSeparators(&p);
		if ( *p == ')' ) {
			p++;
			break;
		}
		if ( *p == '\0' || iCount >= iCapacity ) {
			return 0;
		}
		fValue = strtod(p, &pEnd);
		if ( pEnd == p ) {
			return 0;
		}
		if ( !__xgeSvgDoubleToFloat(fValue, &pValues[iCount]) ) {
			return 0;
		}
		iCount++;
		p = pEnd;
	}
	*ppNext = p;
	*pCount = iCount;
	return 1;
}

static int __xgeSvgParseTransform(const char* sText, xge_shape_ex_matrix_t* pMatrix)
{
	xge_shape_ex_matrix_t tOut;
	const char* p;

	if ( (sText == NULL) || (pMatrix == NULL) ) {
		return 0;
	}
	tOut = __xgeSvgMatrixIdentity();
	p = sText;
	while ( *p != '\0' ) {
		float arrArgs[6];
		const char* pNext;
		int iArgCount;

		__xgeSvgSkipTransformSeparators(&p);
		if ( *p == '\0' ) {
			break;
		}
		if ( strncmp(p, "matrix", 6) == 0 ) {
			xge_shape_ex_matrix_t tM;

			p += 6;
			if ( !__xgeSvgParseTransformArgs(p, &pNext, arrArgs, 6, &iArgCount) || (iArgCount != 6) ) return 0;
			tM.fA = arrArgs[0]; tM.fB = arrArgs[1]; tM.fC = arrArgs[2]; tM.fD = arrArgs[3]; tM.fE = arrArgs[4]; tM.fF = arrArgs[5];
			tOut = __xgeSvgMatrixMul(tOut, tM);
			p = pNext;
		} else if ( strncmp(p, "translate", 9) == 0 ) {
			p += 9;
			if ( !__xgeSvgParseTransformArgs(p, &pNext, arrArgs, 6, &iArgCount) || (iArgCount < 1) || (iArgCount > 2) ) return 0;
			tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixTranslate(arrArgs[0], iArgCount == 2 ? arrArgs[1] : 0.0f));
			p = pNext;
		} else if ( strncmp(p, "scale", 5) == 0 ) {
			p += 5;
			if ( !__xgeSvgParseTransformArgs(p, &pNext, arrArgs, 6, &iArgCount) || (iArgCount < 1) || (iArgCount > 2) ) return 0;
			tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixScale(arrArgs[0], iArgCount == 2 ? arrArgs[1] : arrArgs[0]));
			p = pNext;
		} else if ( strncmp(p, "rotate", 6) == 0 ) {
			p += 6;
			if ( !__xgeSvgParseTransformArgs(p, &pNext, arrArgs, 6, &iArgCount) || ((iArgCount != 1) && (iArgCount != 3)) ) return 0;
			if ( iArgCount == 3 ) {
				tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixTranslate(arrArgs[1], arrArgs[2]));
				tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixRotate(arrArgs[0]));
				tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixTranslate(-arrArgs[1], -arrArgs[2]));
			} else {
				tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixRotate(arrArgs[0]));
			}
			p = pNext;
		} else if ( strncmp(p, "skewX", 5) == 0 ) {
			p += 5;
			if ( !__xgeSvgParseTransformArgs(p, &pNext, arrArgs, 6, &iArgCount) || (iArgCount != 1) ) return 0;
			tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixSkewX(arrArgs[0]));
			p = pNext;
		} else if ( strncmp(p, "skewY", 5) == 0 ) {
			p += 5;
			if ( !__xgeSvgParseTransformArgs(p, &pNext, arrArgs, 6, &iArgCount) || (iArgCount != 1) ) return 0;
			tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixSkewY(arrArgs[0]));
			p = pNext;
		} else {
			return 0;
		}
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

static int __xgeSvgParseDashArrayEx(const char* sText, float fPercentRef, float fFontSize, float* pPattern, int* pCount)
{
	const char* p;
	float arrPattern[XGE_SVG_DASH_MAX];
	int iCount;
	float fTotal;
	int i;

	if ( (sText == NULL) || (pPattern == NULL) || (pCount == NULL) ) {
		return 0;
	}
	*pCount = 0;
	while ( (*sText == ' ') || (*sText == '\t') || (*sText == '\r') || (*sText == '\n') ) sText++;
	if ( __xgeSvgColorNameEquals(sText, "none") ) {
		return 1;
	}
	p = sText;
	iCount = 0;
	fTotal = 0.0f;
	while ( *p != '\0' ) {
		float fValue;

		while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') || (*p == ',') ) p++;
		if ( *p == '\0' ) {
			break;
		}
		if ( !__xgeSvgParseLengthTokenEx(&p, fPercentRef, fFontSize, &fValue) ) {
			break;
		}
		if ( fValue < 0.0f ) {
			return 1;
		}
		if ( iCount < XGE_SVG_DASH_MAX ) {
			arrPattern[iCount++] = fValue;
			fTotal += fValue;
		}
	}
	if ( fTotal <= 0.0f ) {
		*pCount = 0;
		return 1;
	}
	if ( (iCount & 1) != 0 ) {
		int iOriginalCount = iCount;
		int iCopyCount = XGE_SVG_DASH_MAX - iCount;

		if ( iCopyCount > iOriginalCount ) {
			iCopyCount = iOriginalCount;
		}
		for ( i = 0; i < iCopyCount; i++ ) {
			arrPattern[iCount++] = arrPattern[i];
		}
		if ( (iCount & 1) != 0 ) {
			iCount--;
		}
	}
	for ( i = 0; i < iCount; i++ ) {
		pPattern[i] = arrPattern[i];
	}
	*pCount = iCount;
	return 1;
}

static int __xgeSvgParseDashArray(const char* sText, float fPercentRef, float* pPattern, int* pCount)
{
	return __xgeSvgParseDashArrayEx(sText, fPercentRef, 16.0f, pPattern, pCount);
}

static int __xgeSvgPaintOrderTokenEquals(const char* pStart, const char* pEnd, const char* sToken)
{
	int iLen;

	if ( (pStart == NULL) || (pEnd == NULL) || (sToken == NULL) || (pEnd < pStart) ) {
		return 0;
	}
	iLen = (int)strlen(sToken);
	return ((int)(pEnd - pStart) == iLen) && (strncmp(pStart, sToken, (size_t)iLen) == 0);
}

static void __xgeSvgPaintOrderDefault(int pOrder[XGE_SVG_PAINT_COMPONENT_COUNT])
{
	if ( pOrder == NULL ) {
		return;
	}
	pOrder[0] = XGE_SVG_PAINT_FILL;
	pOrder[1] = XGE_SVG_PAINT_STROKE;
	pOrder[2] = XGE_SVG_PAINT_MARKERS;
}

static int __xgeSvgPaintOrderIndex(const int pOrder[XGE_SVG_PAINT_COMPONENT_COUNT], int iComponent)
{
	int i;

	if ( pOrder == NULL ) {
		return -1;
	}
	for ( i = 0; i < XGE_SVG_PAINT_COMPONENT_COUNT; i++ ) {
		if ( pOrder[i] == iComponent ) {
			return i;
		}
	}
	return -1;
}

static int __xgeSvgPaintOrderStrokeFirst(const int pOrder[XGE_SVG_PAINT_COMPONENT_COUNT])
{
	int iFill;
	int iStroke;

	iFill = __xgeSvgPaintOrderIndex(pOrder, XGE_SVG_PAINT_FILL);
	iStroke = __xgeSvgPaintOrderIndex(pOrder, XGE_SVG_PAINT_STROKE);
	if ( (iFill < 0) || (iStroke < 0) ) {
		return 0;
	}
	return iStroke < iFill;
}

static void __xgeSvgStyleUpdatePaintOrderDerived(xge_svg_style_t* pStyle)
{
	if ( pStyle == NULL ) {
		return;
	}
	pStyle->bStrokeFirst = __xgeSvgPaintOrderStrokeFirst(pStyle->iPaintOrder);
}

static int __xgeSvgParsePaintOrder(const char* sText, int pOrder[XGE_SVG_PAINT_COMPONENT_COUNT])
{
	const char* p;
	int arrSeen[XGE_SVG_PAINT_COMPONENT_COUNT];
	int arrParsed[XGE_SVG_PAINT_COMPONENT_COUNT];
	int iCount;
	int bSawToken;

	if ( (sText == NULL) || (pOrder == NULL) ) {
		return 0;
	}
	p = sText;
	memset(arrSeen, 0, sizeof(arrSeen));
	bSawToken = 0;
	iCount = 0;
	while ( *p != '\0' ) {
		const char* pStart;
		const char* pEnd;
		int iComponent;

		while ( __xgeSvgIsSpace((unsigned char)*p) || (*p == ',') ) {
			p++;
		}
		if ( *p == '\0' ) {
			break;
		}
		pStart = p;
		while ( (*p != '\0') && !__xgeSvgIsSpace((unsigned char)*p) && (*p != ',') ) {
			p++;
		}
		pEnd = p;
		if ( __xgeSvgPaintOrderTokenEquals(pStart, pEnd, "normal") ) {
			const char* q;

			if ( bSawToken ) {
				return 0;
			}
			q = p;
			while ( __xgeSvgIsSpace((unsigned char)*q) || (*q == ',') ) {
				q++;
			}
			if ( *q != '\0' ) {
				return 0;
			}
			__xgeSvgPaintOrderDefault(pOrder);
			return 1;
		}
		if ( __xgeSvgPaintOrderTokenEquals(pStart, pEnd, "fill") ) {
			iComponent = XGE_SVG_PAINT_FILL;
		} else if ( __xgeSvgPaintOrderTokenEquals(pStart, pEnd, "stroke") ) {
			iComponent = XGE_SVG_PAINT_STROKE;
		} else if ( __xgeSvgPaintOrderTokenEquals(pStart, pEnd, "markers") ) {
			iComponent = XGE_SVG_PAINT_MARKERS;
		} else {
			return 0;
		}
		if ( arrSeen[iComponent] || (iCount >= XGE_SVG_PAINT_COMPONENT_COUNT) ) {
			return 0;
		}
		arrSeen[iComponent] = 1;
		arrParsed[iCount++] = iComponent;
		bSawToken = 1;
	}
	if ( !bSawToken ) {
		return 0;
	}
	if ( !arrSeen[XGE_SVG_PAINT_FILL] ) {
		arrParsed[iCount++] = XGE_SVG_PAINT_FILL;
	}
	if ( !arrSeen[XGE_SVG_PAINT_STROKE] ) {
		arrParsed[iCount++] = XGE_SVG_PAINT_STROKE;
	}
	if ( !arrSeen[XGE_SVG_PAINT_MARKERS] ) {
		arrParsed[iCount++] = XGE_SVG_PAINT_MARKERS;
	}
	memcpy(pOrder, arrParsed, sizeof(arrParsed));
	return 1;
}

static int __xgeSvgParsePaintFallbackColor(const char* sText, uint32_t iCurrentColor, uint32_t* pColor)
{
	if ( (sText == NULL) || (pColor == NULL) ) {
		return 0;
	}
	__xgeSvgSkipSpaces(&sText);
	if ( *sText == '\0' ) {
		return 0;
	}
	if ( __xgeSvgColorNameEquals(sText, "currentColor") ) {
		*pColor = iCurrentColor;
		return 1;
	}
	return __xgeSvgParseColor(sText, pColor);
}

static int __xgeSvgParseTextBaseline(const char* sText, int* pBaseline)
{
	if ( (sText == NULL) || (pBaseline == NULL) ) {
		return 0;
	}
	if ( __xgeSvgColorNameEquals(sText, "auto") ||
	     __xgeSvgColorNameEquals(sText, "baseline") ||
	     __xgeSvgColorNameEquals(sText, "alphabetic") ) {
		*pBaseline = XGE_SVG_TEXT_BASELINE_ALPHABETIC;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "middle") ||
	     __xgeSvgColorNameEquals(sText, "central") ||
	     __xgeSvgColorNameEquals(sText, "mathematical") ) {
		*pBaseline = XGE_SVG_TEXT_BASELINE_MIDDLE;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "hanging") ) {
		*pBaseline = XGE_SVG_TEXT_BASELINE_HANGING;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "text-before-edge") ||
	     __xgeSvgColorNameEquals(sText, "before-edge") ||
	     __xgeSvgColorNameEquals(sText, "top") ) {
		*pBaseline = XGE_SVG_TEXT_BASELINE_TEXT_BEFORE_EDGE;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "text-after-edge") ||
	     __xgeSvgColorNameEquals(sText, "after-edge") ||
	     __xgeSvgColorNameEquals(sText, "ideographic") ||
	     __xgeSvgColorNameEquals(sText, "bottom") ) {
		*pBaseline = XGE_SVG_TEXT_BASELINE_TEXT_AFTER_EDGE;
		return 1;
	}
	return 0;
}

static int __xgeSvgParseTextDecoration(const char* sText, int* pDecoration)
{
	const char* p;
	const char* pTokenStart;
	const char* pTokenEnd;
	int iDecoration;
	int bFound;

	if ( (sText == NULL) || (pDecoration == NULL) ) {
		return 0;
	}
	p = sText;
	iDecoration = 0;
	bFound = 0;
	while ( *p != '\0' ) {
		while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') || (*p == ',') ) {
			p++;
		}
		if ( *p == '\0' ) {
			break;
		}
		pTokenStart = p;
		while ( (*p != '\0') && (*p != ' ') && (*p != '\t') && (*p != '\r') && (*p != '\n') && (*p != ',') ) {
			p++;
		}
		pTokenEnd = p;
		if ( __xgeSvgCssRangeEqualsText(pTokenStart, pTokenEnd, "none") ) {
			iDecoration = 0;
			bFound = 1;
			continue;
		}
		if ( __xgeSvgCssRangeEqualsText(pTokenStart, pTokenEnd, "underline") ) {
			iDecoration |= XGE_SVG_TEXT_DECORATION_UNDERLINE;
			bFound = 1;
		} else if ( __xgeSvgCssRangeEqualsText(pTokenStart, pTokenEnd, "overline") ) {
			iDecoration |= XGE_SVG_TEXT_DECORATION_OVERLINE;
			bFound = 1;
		} else if ( __xgeSvgCssRangeEqualsText(pTokenStart, pTokenEnd, "line-through") ) {
			iDecoration |= XGE_SVG_TEXT_DECORATION_LINE_THROUGH;
			bFound = 1;
		}
	}
	if ( !bFound ) {
		return 0;
	}
	*pDecoration = iDecoration;
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
	tStyle.fFontSize = 16.0f;
	tStyle.iFillRule = XGE_SHAPE_EX_FILL_NON_ZERO;
	tStyle.iClipRule = XGE_SHAPE_EX_FILL_NON_ZERO;
	tStyle.iLineCap = XGE_SHAPE_EX_CAP_BUTT;
	tStyle.iLineJoin = XGE_SHAPE_EX_JOIN_MITER;
	tStyle.iTextAnchor = XGE_SVG_TEXT_ANCHOR_START;
	tStyle.iTextSpace = XGE_SVG_TEXT_SPACE_DEFAULT;
	tStyle.iTextBaseline = XGE_SVG_TEXT_BASELINE_ALPHABETIC;
	__xgeSvgPaintOrderDefault(tStyle.iPaintOrder);
	__xgeSvgStyleUpdatePaintOrderDerived(&tStyle);
	tStyle.bVisible = 1;
	tStyle.bVisibility = 1;
	tStyle.tTransform = __xgeSvgMatrixIdentity();
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

static int __xgeSvgStylePropertyCopyEx(const char* pTag, const char* pTagEnd, const char* sStyle, const char* sName, char* sOut, int iOutCapacity, int bAllowAttrs, int iImportanceMode)
{
	if ( __xgeSvgStyleCopyEx(sStyle, sName, sOut, iOutCapacity, iImportanceMode) ) {
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

static int __xgeSvgStylePropertyCopy(const char* pTag, const char* pTagEnd, const char* sStyle, const char* sName, char* sOut, int iOutCapacity, int bAllowAttrs)
{
	return __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, sName, sOut, iOutCapacity, bAllowAttrs, XGE_SVG_STYLE_IMPORTANCE_ANY);
}

static void __xgeSvgStyleApplyProperties(xge_svg pSvg, xge_svg_style_t* pStyle, const char* pTag, const char* pTagEnd, const char* sStyle, int bAllowAttrs, int bAllowTransform, int iImportanceMode, xge_shape_ex_matrix_t tBaseTransform, float fBaseOpacity, int bBaseVisible, float fBaseFontSize, int iApplyMode)
{
	char sValue[XGE_SVG_ATTR_MAX];
	float fValue;
	float fLengthRef;
	int iBaseline;
	int iDecoration;

	if ( pStyle == NULL ) {
		return;
	}
	fLengthRef = __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_OTHER);
	if ( iApplyMode != XGE_SVG_STYLE_APPLY_EXCEPT_FONT_SIZE ) {
		if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "font-size", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) &&
		     __xgeSvgParseFontSize(sValue, fBaseFontSize, &fValue) ) {
			pStyle->fFontSize = fValue;
		}
		if ( iApplyMode == XGE_SVG_STYLE_APPLY_FONT_SIZE ) {
			return;
		}
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "color", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) &&
	     !__xgeSvgColorNameEquals(sValue, "currentColor") ) {
		if ( __xgeSvgParseColor(sValue, &pStyle->iCurrentColor) ) {
			pStyle->bColorSet = 1;
		}
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "display", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		pStyle->bVisible = (strcmp(sValue, "none") == 0) ? 0 : bBaseVisible;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "visibility", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( strcmp(sValue, "visible") == 0 ) {
			pStyle->bVisibility = 1;
		} else if ( (strcmp(sValue, "hidden") == 0) || (strcmp(sValue, "collapse") == 0) ) {
			pStyle->bVisibility = 0;
		}
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "text-anchor", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( strcmp(sValue, "middle") == 0 ) pStyle->iTextAnchor = XGE_SVG_TEXT_ANCHOR_MIDDLE;
		else if ( strcmp(sValue, "end") == 0 ) pStyle->iTextAnchor = XGE_SVG_TEXT_ANCHOR_END;
		else pStyle->iTextAnchor = XGE_SVG_TEXT_ANCHOR_START;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "dominant-baseline", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) &&
	     __xgeSvgParseTextBaseline(sValue, &iBaseline) ) {
		pStyle->iTextBaseline = iBaseline;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "alignment-baseline", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) &&
	     __xgeSvgParseTextBaseline(sValue, &iBaseline) ) {
		pStyle->iTextBaseline = iBaseline;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "text-decoration-line", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) &&
	     __xgeSvgParseTextDecoration(sValue, &iDecoration) ) {
		pStyle->iTextDecoration = iDecoration;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "text-decoration", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) &&
	     __xgeSvgParseTextDecoration(sValue, &iDecoration) ) {
		pStyle->iTextDecoration = iDecoration;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "letter-spacing", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( __xgeSvgColorNameEquals(sValue, "normal") ) {
			pStyle->fLetterSpacing = 0.0f;
		} else if ( __xgeSvgParseLengthEx(sValue, fLengthRef, pStyle->fFontSize, &fValue) ) {
			pStyle->fLetterSpacing = fValue;
		}
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "word-spacing", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( __xgeSvgColorNameEquals(sValue, "normal") ) {
			pStyle->fWordSpacing = 0.0f;
		} else if ( __xgeSvgParseLengthEx(sValue, fLengthRef, pStyle->fFontSize, &fValue) ) {
			pStyle->fWordSpacing = fValue;
		}
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "xml:space", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( strcmp(sValue, "preserve") == 0 ) pStyle->iTextSpace = XGE_SVG_TEXT_SPACE_PRESERVE;
		else if ( strcmp(sValue, "default") == 0 ) pStyle->iTextSpace = XGE_SVG_TEXT_SPACE_DEFAULT;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "white-space", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( (strcmp(sValue, "pre") == 0) || (strcmp(sValue, "pre-wrap") == 0) || (strcmp(sValue, "break-spaces") == 0) ) {
			pStyle->iTextSpace = XGE_SVG_TEXT_SPACE_PRESERVE;
		} else if ( (strcmp(sValue, "normal") == 0) || (strcmp(sValue, "nowrap") == 0) ) {
			pStyle->iTextSpace = XGE_SVG_TEXT_SPACE_DEFAULT;
		}
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "stop-color", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( __xgeSvgColorNameEquals(sValue, "currentColor") ) {
			pStyle->iStopColor = pStyle->iCurrentColor;
			pStyle->bStopColorSet = 1;
		} else if ( __xgeSvgParseColor(sValue, &pStyle->iStopColor) ) {
			pStyle->bStopColorSet = 1;
		}
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "stop-opacity", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) && __xgeSvgParseOpacity(sValue, &fValue) ) {
		pStyle->fStopOpacity = fValue;
		pStyle->bStopOpacitySet = 1;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "fill", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		const char* sFallback = NULL;
		pStyle->bFillSet = 1;
		pStyle->bFillCurrentColor = 0;
		if ( __xgeSvgUrlIdCopyFromValueEx(sValue, pStyle->sFillGradientId, sizeof(pStyle->sFillGradientId), &sFallback) ) {
			pStyle->iFillColor = XGE_COLOR_RGBA(0, 0, 0, 255);
			__xgeSvgParsePaintFallbackColor(sFallback, pStyle->iCurrentColor, &pStyle->iFillColor);
			pStyle->bFillCurrentColor = (sFallback != NULL) && __xgeSvgColorNameEquals(sFallback, "currentColor");
		} else if ( __xgeSvgColorNameEquals(sValue, "currentColor") ) {
			pStyle->sFillGradientId[0] = '\0';
			pStyle->iFillColor = pStyle->iCurrentColor;
			pStyle->bFillCurrentColor = 1;
		} else {
			pStyle->sFillGradientId[0] = '\0';
			pStyle->iFillColor = XGE_COLOR_RGBA(0, 0, 0, 255);
			__xgeSvgParseColor(sValue, &pStyle->iFillColor);
		}
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "stroke", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		const char* sFallback = NULL;
		pStyle->bStrokeSet = 1;
		pStyle->bStrokeCurrentColor = 0;
		if ( __xgeSvgUrlIdCopyFromValueEx(sValue, pStyle->sStrokeGradientId, sizeof(pStyle->sStrokeGradientId), &sFallback) ) {
			pStyle->iStrokeColor = XGE_COLOR_RGBA(0, 0, 0, 255);
			__xgeSvgParsePaintFallbackColor(sFallback, pStyle->iCurrentColor, &pStyle->iStrokeColor);
			pStyle->bStrokeCurrentColor = (sFallback != NULL) && __xgeSvgColorNameEquals(sFallback, "currentColor");
		} else if ( __xgeSvgColorNameEquals(sValue, "currentColor") ) {
			pStyle->sStrokeGradientId[0] = '\0';
			pStyle->iStrokeColor = pStyle->iCurrentColor;
			pStyle->bStrokeCurrentColor = 1;
		} else {
			pStyle->sStrokeGradientId[0] = '\0';
			pStyle->iStrokeColor = XGE_COLOR_RGBA(0, 0, 0, 0);
			__xgeSvgParseColor(sValue, &pStyle->iStrokeColor);
		}
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "opacity", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) && __xgeSvgParseOpacity(sValue, &fValue) ) {
		pStyle->fOpacity = fBaseOpacity * fValue;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "fill-opacity", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) && __xgeSvgParseOpacity(sValue, &fValue) ) {
		pStyle->fFillOpacity = fValue;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "stroke-opacity", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) && __xgeSvgParseOpacity(sValue, &fValue) ) {
		pStyle->fStrokeOpacity = fValue;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "stroke-width", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		pStyle->fStrokeWidth = __xgeSvgParseLengthEx(sValue, fLengthRef, pStyle->fFontSize, &fValue) ? fValue : 0.0f;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "stroke-miterlimit", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( __xgeSvgParseFloat(sValue, &fValue) ) {
			if ( fValue >= 0.0f ) {
				pStyle->fMiterLimit = fValue;
			}
		} else {
			pStyle->fMiterLimit = 0.0f;
		}
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "stroke-dasharray", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		__xgeSvgParseDashArrayEx(sValue, fLengthRef, pStyle->fFontSize, pStyle->fDashPattern, &pStyle->iDashCount);
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "stroke-dashoffset", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		pStyle->fDashOffset = __xgeSvgParseLengthEx(sValue, __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_X), pStyle->fFontSize, &fValue) ? fValue : 0.0f;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "clip-path", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( strcmp(sValue, "none") == 0 ) {
			pStyle->sClipId[0] = '\0';
		} else {
			__xgeSvgUrlIdCopyFromValue(sValue, pStyle->sClipId, sizeof(pStyle->sClipId));
		}
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "mask", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( strcmp(sValue, "none") == 0 ) {
			pStyle->sMaskId[0] = '\0';
		} else {
			__xgeSvgUrlIdCopyFromValue(sValue, pStyle->sMaskId, sizeof(pStyle->sMaskId));
		}
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "filter", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( strcmp(sValue, "none") == 0 ) {
			pStyle->sFilterId[0] = '\0';
		} else {
			__xgeSvgUrlIdCopyFromValue(sValue, pStyle->sFilterId, sizeof(pStyle->sFilterId));
		}
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "marker", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( strcmp(sValue, "none") == 0 ) {
			pStyle->sMarkerStartId[0] = '\0';
			pStyle->sMarkerMidId[0] = '\0';
			pStyle->sMarkerEndId[0] = '\0';
		} else if ( __xgeSvgUrlIdCopyFromValue(sValue, pStyle->sMarkerStartId, sizeof(pStyle->sMarkerStartId)) ) {
			strcpy(pStyle->sMarkerMidId, pStyle->sMarkerStartId);
			strcpy(pStyle->sMarkerEndId, pStyle->sMarkerStartId);
		}
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "marker-start", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( strcmp(sValue, "none") == 0 ) pStyle->sMarkerStartId[0] = '\0';
		else __xgeSvgUrlIdCopyFromValue(sValue, pStyle->sMarkerStartId, sizeof(pStyle->sMarkerStartId));
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "marker-mid", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( strcmp(sValue, "none") == 0 ) pStyle->sMarkerMidId[0] = '\0';
		else __xgeSvgUrlIdCopyFromValue(sValue, pStyle->sMarkerMidId, sizeof(pStyle->sMarkerMidId));
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "marker-end", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( strcmp(sValue, "none") == 0 ) pStyle->sMarkerEndId[0] = '\0';
		else __xgeSvgUrlIdCopyFromValue(sValue, pStyle->sMarkerEndId, sizeof(pStyle->sMarkerEndId));
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "fill-rule", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		pStyle->iFillRule = (strcmp(sValue, "evenodd") == 0) ? XGE_SHAPE_EX_FILL_EVEN_ODD : XGE_SHAPE_EX_FILL_NON_ZERO;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "clip-rule", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		pStyle->iClipRule = (strcmp(sValue, "evenodd") == 0) ? XGE_SHAPE_EX_FILL_EVEN_ODD : XGE_SHAPE_EX_FILL_NON_ZERO;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "paint-order", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		int arrPaintOrder[XGE_SVG_PAINT_COMPONENT_COUNT];

		if ( !__xgeSvgParsePaintOrder(sValue, arrPaintOrder) ) {
			__xgeSvgPaintOrderDefault(arrPaintOrder);
		}
		memcpy(pStyle->iPaintOrder, arrPaintOrder, sizeof(arrPaintOrder));
		__xgeSvgStyleUpdatePaintOrderDerived(pStyle);
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "stroke-linecap", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( strcmp(sValue, "round") == 0 ) pStyle->iLineCap = XGE_SHAPE_EX_CAP_ROUND;
		else if ( strcmp(sValue, "square") == 0 ) pStyle->iLineCap = XGE_SHAPE_EX_CAP_SQUARE;
		else pStyle->iLineCap = XGE_SHAPE_EX_CAP_BUTT;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "stroke-linejoin", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		if ( strcmp(sValue, "round") == 0 ) pStyle->iLineJoin = XGE_SHAPE_EX_JOIN_ROUND;
		else if ( strcmp(sValue, "bevel") == 0 ) pStyle->iLineJoin = XGE_SHAPE_EX_JOIN_BEVEL;
		else pStyle->iLineJoin = XGE_SHAPE_EX_JOIN_MITER;
	}
	if ( __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "vector-effect", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		pStyle->bStrokeNonScaling = (strcmp(sValue, "non-scaling-stroke") == 0);
	}
	if ( bAllowTransform && __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "transform", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		xge_shape_ex_matrix_t tTransform;
		if ( __xgeSvgParseTransform(sValue, &tTransform) ) {
			pStyle->tTransform = __xgeSvgMatrixMul(tBaseTransform, tTransform);
		}
	}
}

static void __xgeSvgStyleApplyDeclaration(xge_svg pSvg, xge_svg_style_t* pStyle, const char* sStyle, int iImportanceMode, xge_shape_ex_matrix_t tBaseTransform, float fBaseOpacity, int bBaseVisible, float fBaseFontSize, int iApplyMode)
{
	__xgeSvgStyleApplyProperties(pSvg, pStyle, NULL, NULL, sStyle, 0, 1, iImportanceMode, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, iApplyMode);
}

static void __xgeSvgStyleApplyPresentationAttrs(xge_svg pSvg, xge_svg_style_t* pStyle, const char* pTag, const char* pTagEnd, xge_shape_ex_matrix_t tBaseTransform, float fBaseOpacity, int bBaseVisible, float fBaseFontSize, int iApplyMode)
{
	if ( (pStyle == NULL) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return;
	}
	__xgeSvgStyleApplyProperties(pSvg, pStyle, pTag, pTagEnd, NULL, 1, 1, XGE_SVG_STYLE_IMPORTANCE_ANY, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, iApplyMode);
}

static void __xgeSvgStyleApplyInlineStyle(xge_svg pSvg, xge_svg_style_t* pStyle, const char* pTag, const char* pTagEnd, int iImportanceMode, xge_shape_ex_matrix_t tBaseTransform, float fBaseOpacity, int bBaseVisible, float fBaseFontSize, int iApplyMode)
{
	char sStyle[XGE_SVG_ATTR_MAX];

	if ( (pStyle == NULL) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return;
	}
	sStyle[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "style", sStyle, sizeof(sStyle));
	__xgeSvgStyleApplyProperties(pSvg, pStyle, NULL, NULL, sStyle, 0, 1, iImportanceMode, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, iApplyMode);
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

static int __xgeSvgCssAttributeWordContains(const char* sText, const char* sWord)
{
	const char* p;
	int iWordLen;

	if ( (sText == NULL) || (sWord == NULL) || (sWord[0] == '\0') ) {
		return 0;
	}
	iWordLen = (int)strlen(sWord);
	p = sText;
	while ( *p != '\0' ) {
		const char* pStart;
		const char* pEnd;

		while ( __xgeSvgCssIsSpace(*p) ) {
			p++;
		}
		pStart = p;
		while ( (*p != '\0') && !__xgeSvgCssIsSpace(*p) ) {
			p++;
		}
		pEnd = p;
		if ( ((int)(pEnd - pStart) == iWordLen) && (strncmp(pStart, sWord, (size_t)iWordLen) == 0) ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeSvgCssAttributeMatches(const char* pTag, const char* pTagEnd, const char* sName, const char* sOp, const char* sExpected)
{
	char sActual[XGE_SVG_ATTR_MAX];
	int iActualLen;
	int iExpectedLen;

	if ( (pTag == NULL) || (pTagEnd == NULL) || (sName == NULL) || (sOp == NULL) || (sExpected == NULL) ) {
		return 0;
	}
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, sName, sActual, sizeof(sActual)) ) {
		return 0;
	}
	if ( sOp[0] == '\0' ) {
		return 1;
	}
	iActualLen = (int)strlen(sActual);
	iExpectedLen = (int)strlen(sExpected);
	if ( strcmp(sOp, "=") == 0 ) {
		return strcmp(sActual, sExpected) == 0;
	}
	if ( strcmp(sOp, "~=") == 0 ) {
		return __xgeSvgCssAttributeWordContains(sActual, sExpected);
	}
	if ( strcmp(sOp, "|=") == 0 ) {
		return (strcmp(sActual, sExpected) == 0) ||
		       ((iActualLen > iExpectedLen) && (strncmp(sActual, sExpected, (size_t)iExpectedLen) == 0) && (sActual[iExpectedLen] == '-'));
	}
	if ( strcmp(sOp, "^=") == 0 ) {
		return (iExpectedLen > 0) && (iActualLen >= iExpectedLen) && (strncmp(sActual, sExpected, (size_t)iExpectedLen) == 0);
	}
	if ( strcmp(sOp, "$=") == 0 ) {
		return (iExpectedLen > 0) && (iActualLen >= iExpectedLen) && (strcmp(sActual + iActualLen - iExpectedLen, sExpected) == 0);
	}
	if ( strcmp(sOp, "*=") == 0 ) {
		return (iExpectedLen > 0) && (strstr(sActual, sExpected) != NULL);
	}
	return 0;
}

static int __xgeSvgElementNodeSameType(xge_svg pSvg, int iLeft, int iRight)
{
	char sLeft[XGE_SVG_ATTR_MAX];
	char sRight[XGE_SVG_ATTR_MAX];

	if ( !__xgeSvgValid(pSvg) || (iLeft < 0) || (iLeft >= pSvg->iElementNodeCount) ||
	     (iRight < 0) || (iRight >= pSvg->iElementNodeCount) ) {
		return 0;
	}
	if ( !__xgeSvgTagNameCopy(pSvg->pElementNodes[iLeft].pTag, sLeft, sizeof(sLeft)) ||
	     !__xgeSvgTagNameCopy(pSvg->pElementNodes[iRight].pTag, sRight, sizeof(sRight)) ) {
		return 0;
	}
	return strcmp(sLeft, sRight) == 0;
}

static int __xgeSvgElementNodeChildIndex(xge_svg pSvg, int iNodeIndex, int bOfType)
{
	int iIndex;
	int iSibling;

	if ( !__xgeSvgValid(pSvg) || (iNodeIndex < 0) || (iNodeIndex >= pSvg->iElementNodeCount) ) {
		return 0;
	}
	iIndex = 1;
	for ( iSibling = pSvg->pElementNodes[iNodeIndex].iPrevSiblingIndex; iSibling >= 0; iSibling = pSvg->pElementNodes[iSibling].iPrevSiblingIndex ) {
		if ( !bOfType || __xgeSvgElementNodeSameType(pSvg, iNodeIndex, iSibling) ) {
			iIndex++;
		}
	}
	return iIndex;
}

static int __xgeSvgElementNodeFirstChild(xge_svg pSvg, int iNodeIndex)
{
	return __xgeSvgValid(pSvg) && (iNodeIndex >= 0) && (iNodeIndex < pSvg->iElementNodeCount) &&
	       (pSvg->pElementNodes[iNodeIndex].iPrevSiblingIndex < 0);
}

static int __xgeSvgElementNodeFirstOfType(xge_svg pSvg, int iNodeIndex)
{
	int iSibling;

	if ( !__xgeSvgValid(pSvg) || (iNodeIndex < 0) || (iNodeIndex >= pSvg->iElementNodeCount) ) {
		return 0;
	}
	for ( iSibling = pSvg->pElementNodes[iNodeIndex].iPrevSiblingIndex; iSibling >= 0; iSibling = pSvg->pElementNodes[iSibling].iPrevSiblingIndex ) {
		if ( __xgeSvgElementNodeSameType(pSvg, iNodeIndex, iSibling) ) {
			return 0;
		}
	}
	return 1;
}

static int __xgeSvgElementNodeLastChild(xge_svg pSvg, int iNodeIndex)
{
	return __xgeSvgValid(pSvg) && (iNodeIndex >= 0) && (iNodeIndex < pSvg->iElementNodeCount) &&
	       (pSvg->pElementNodes[iNodeIndex].iNextSiblingIndex < 0);
}

static int __xgeSvgElementNodeLastOfType(xge_svg pSvg, int iNodeIndex)
{
	int iSibling;

	if ( !__xgeSvgValid(pSvg) || (iNodeIndex < 0) || (iNodeIndex >= pSvg->iElementNodeCount) ) {
		return 0;
	}
	for ( iSibling = pSvg->pElementNodes[iNodeIndex].iNextSiblingIndex; iSibling >= 0; iSibling = pSvg->pElementNodes[iSibling].iNextSiblingIndex ) {
		if ( __xgeSvgElementNodeSameType(pSvg, iNodeIndex, iSibling) ) {
			return 0;
		}
	}
	return 1;
}

static int __xgeSvgElementNodeOnlyChild(xge_svg pSvg, int iNodeIndex)
{
	return __xgeSvgElementNodeFirstChild(pSvg, iNodeIndex) &&
	       __xgeSvgElementNodeLastChild(pSvg, iNodeIndex);
}

static int __xgeSvgElementNodeOnlyOfType(xge_svg pSvg, int iNodeIndex)
{
	return __xgeSvgElementNodeFirstOfType(pSvg, iNodeIndex) &&
	       __xgeSvgElementNodeLastOfType(pSvg, iNodeIndex);
}

static int __xgeSvgElementNodeEmpty(xge_svg pSvg, int iNodeIndex)
{
	return __xgeSvgValid(pSvg) && (iNodeIndex >= 0) && (iNodeIndex < pSvg->iElementNodeCount) &&
	       (pSvg->pElementNodes[iNodeIndex].iChildCount <= 0) &&
	       (pSvg->pElementNodes[iNodeIndex].bHasTextContent == 0);
}

static int __xgeSvgElementNodeLastChildIndex(xge_svg pSvg, int iNodeIndex, int bOfType)
{
	int iIndex;
	int iSibling;

	if ( !__xgeSvgValid(pSvg) || (iNodeIndex < 0) || (iNodeIndex >= pSvg->iElementNodeCount) ) {
		return 0;
	}
	iIndex = 1;
	for ( iSibling = pSvg->pElementNodes[iNodeIndex].iNextSiblingIndex; iSibling >= 0; iSibling = pSvg->pElementNodes[iSibling].iNextSiblingIndex ) {
		if ( !bOfType || __xgeSvgElementNodeSameType(pSvg, iNodeIndex, iSibling) ) {
			iIndex++;
		}
	}
	return iIndex;
}

static int __xgeSvgSimpleSelectorRangeMatches(xge_svg pSvg, const char* pStart, const char* pEnd, int iNodeIndex);
static int __xgeSvgSelectorMatchesFromRight(xge_svg pSvg, const char* pSelectorStart, const char* pSelectorEnd, int iNodeIndex);
static int __xgeSvgSelectorListMatchesFromRight(xge_svg pSvg, const char* pStart, const char* pEnd, int iNodeIndex, int* pMatched);
static int __xgeSvgHasSelectorListMatches(xge_svg pSvg, const char* pStart, const char* pEnd, int iNodeIndex, int* pMatched);

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
	while ( (*p != '\0') && (*p != '.') && (*p != '#') && (*p != '[') && (*p != ':') ) {
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

static int __xgeSvgSimpleSelectorMatches(xge_svg pSvg, const char* sSelector, int iNodeIndex)
{
	const char* p;
	const char* pTag;
	const char* pTagEnd;
	const xge_svg_element_node_t* pNode;
	char sToken[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	char sTagName[XGE_SVG_ATTR_MAX];
	int bIsRoot;
	int bMatched;

	if ( !__xgeSvgValid(pSvg) || (sSelector == NULL) || (sSelector[0] == '\0') ||
	     (iNodeIndex < 0) || (iNodeIndex >= pSvg->iElementNodeCount) ) {
		return 0;
	}
	pNode = &pSvg->pElementNodes[iNodeIndex];
	pTag = pNode->pTag;
	pTagEnd = pNode->pTagEnd;
	bIsRoot = pNode->iParentIndex < 0;
	p = sSelector;
	bMatched = 0;
	if ( *p == '*' ) {
		p++;
		bMatched = 1;
	} else if ( (*p != '.') && (*p != '#') && (*p != '[') && (*p != ':') ) {
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
		} else if ( *p == '[' ) {
			const char* pAttrEnd;
			char sName[XGE_SVG_ATTR_MAX];
			char sOp[3];
			char sExpected[XGE_SVG_ATTR_MAX];

			if ( !__xgeSvgCssAttributeSelectorEnd(p, p + strlen(p), &pAttrEnd) ||
			     !__xgeSvgCssAttributeSelectorParse(p, pAttrEnd, sName, sizeof(sName), sOp, sizeof(sOp), sExpected, sizeof(sExpected)) ||
			     !__xgeSvgCssAttributeMatches(pTag, pTagEnd, sName, sOp, sExpected) ) {
				return 0;
			}
			p = pAttrEnd;
			bMatched = 1;
		} else if ( *p == ':' ) {
			const char* pNameStart;
			const char* pNameEnd;

			p++;
			pNameStart = p;
			while ( __xgeSvgCssIdentChar(*p) ) {
				p++;
			}
			pNameEnd = p;
			if ( !__xgeSvgCssRangeCopy(pNameStart, pNameEnd, sToken, sizeof(sToken)) ) {
				return 0;
			}
			if ( strcmp(sToken, "root") == 0 ) {
				if ( !bIsRoot ) {
					return 0;
				}
				bMatched = 1;
				continue;
			}
			if ( strcmp(sToken, "first-child") == 0 ) {
				if ( !__xgeSvgElementNodeFirstChild(pSvg, iNodeIndex) ) {
					return 0;
				}
				bMatched = 1;
				continue;
			}
			if ( strcmp(sToken, "first-of-type") == 0 ) {
				if ( !__xgeSvgElementNodeFirstOfType(pSvg, iNodeIndex) ) {
					return 0;
				}
				bMatched = 1;
				continue;
			}
			if ( strcmp(sToken, "last-child") == 0 ) {
				if ( !__xgeSvgElementNodeLastChild(pSvg, iNodeIndex) ) {
					return 0;
				}
				bMatched = 1;
				continue;
			}
			if ( strcmp(sToken, "last-of-type") == 0 ) {
				if ( !__xgeSvgElementNodeLastOfType(pSvg, iNodeIndex) ) {
					return 0;
				}
				bMatched = 1;
				continue;
			}
			if ( strcmp(sToken, "only-child") == 0 ) {
				if ( !__xgeSvgElementNodeOnlyChild(pSvg, iNodeIndex) ) {
					return 0;
				}
				bMatched = 1;
				continue;
			}
			if ( strcmp(sToken, "only-of-type") == 0 ) {
				if ( !__xgeSvgElementNodeOnlyOfType(pSvg, iNodeIndex) ) {
					return 0;
				}
				bMatched = 1;
				continue;
			}
			if ( strcmp(sToken, "empty") == 0 ) {
				if ( !__xgeSvgElementNodeEmpty(pSvg, iNodeIndex) ) {
					return 0;
				}
				bMatched = 1;
				continue;
			}
			if ( (strcmp(sToken, "nth-child") == 0) || (strcmp(sToken, "nth-of-type") == 0) ||
			     (strcmp(sToken, "nth-last-child") == 0) || (strcmp(sToken, "nth-last-of-type") == 0) ) {
				const char* pFunctionEnd;
				int iA;
				int iB;
				int iIndex;
				int bOfType;
				int bFromEnd;

				if ( (*p != '(') || !__xgeSvgCssFunctionEnd(p, p + strlen(p), &pFunctionEnd) ||
				     !__xgeSvgCssNthParse(p + 1, pFunctionEnd - 1, &iA, &iB) ) {
					return 0;
				}
				bOfType = (strcmp(sToken, "nth-of-type") == 0) || (strcmp(sToken, "nth-last-of-type") == 0);
				bFromEnd = (strcmp(sToken, "nth-last-child") == 0) || (strcmp(sToken, "nth-last-of-type") == 0);
				iIndex = bFromEnd ?
					__xgeSvgElementNodeLastChildIndex(pSvg, iNodeIndex, bOfType) :
					__xgeSvgElementNodeChildIndex(pSvg, iNodeIndex, bOfType);
				if ( !__xgeSvgCssNthMatches(iIndex, iA, iB) ) {
					return 0;
				}
				p = pFunctionEnd;
				bMatched = 1;
				continue;
			}
			if ( (strcmp(sToken, "not") == 0) || (strcmp(sToken, "is") == 0) ||
			     (strcmp(sToken, "where") == 0) || (strcmp(sToken, "has") == 0) ) {
				const char* pFunctionEnd;
				const char* pArgStart;
				const char* pArgEnd;
				int bArgMatched;

				if ( (*p != '(') || !__xgeSvgCssFunctionEnd(p, p + strlen(p), &pFunctionEnd) ) {
					return 0;
				}
				pArgStart = p + 1;
				pArgEnd = pFunctionEnd - 1;
				if ( strcmp(sToken, "has") == 0 ) {
					if ( !__xgeSvgHasSelectorListMatches(pSvg, pArgStart, pArgEnd, iNodeIndex, &bArgMatched) ) {
						return 0;
					}
					if ( !bArgMatched ) {
						return 0;
					}
				} else {
					if ( !__xgeSvgSelectorListMatchesFromRight(pSvg, pArgStart, pArgEnd, iNodeIndex, &bArgMatched) ) {
						return 0;
					}
					if ( strcmp(sToken, "not") == 0 ) {
						if ( bArgMatched ) {
							return 0;
						}
					} else if ( !bArgMatched ) {
						return 0;
					}
				}
				p = pFunctionEnd;
				bMatched = 1;
				continue;
			}
			return 0;
		} else {
			return 0;
		}
	}
	return bMatched;
}

static int __xgeSvgSimpleSelectorRangeMatches(xge_svg pSvg, const char* pStart, const char* pEnd, int iNodeIndex)
{
	char sSelector[XGE_SVG_ATTR_MAX];
	int iLen;

	if ( !__xgeSvgValid(pSvg) || (pStart == NULL) || (pEnd == NULL) ||
	     (iNodeIndex < 0) || (iNodeIndex >= pSvg->iElementNodeCount) ) {
		return 0;
	}
	__xgeSvgTrimRange(&pStart, &pEnd);
	if ( pEnd <= pStart ) {
		return 0;
	}
	iLen = (int)(pEnd - pStart);
	if ( iLen >= (int)sizeof(sSelector) ) {
		return 0;
	}
	memcpy(sSelector, pStart, (size_t)iLen);
	sSelector[iLen] = '\0';
	return __xgeSvgSimpleSelectorMatches(pSvg, sSelector, iNodeIndex);
}

static void __xgeSvgSelectorRightmost(const char* pStart, const char* pEnd, const char** ppSimpleStart, const char** ppSimpleEnd, const char** ppPrevEnd, int* pCombinator)
{
	const char* p;

	*ppSimpleStart = pEnd;
	*ppSimpleEnd = pEnd;
	*ppPrevEnd = pStart;
	*pCombinator = 0;
	__xgeSvgTrimRange(&pStart, &pEnd);
	p = pEnd;
	{
		const char* pCurrentStart = pStart;
		const char* pLastPrevEnd = pStart;
		const char* pLastSimpleStart = pStart;
		int iLastCombinator = 0;
		int bInAttr = 0;
		int iParenDepth = 0;
		char cQuote = 0;

		p = pStart;
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
			if ( bInAttr ) {
				if ( (*p == '"') || (*p == '\'') ) {
					cQuote = *p++;
					continue;
				}
				if ( *p == ']' ) {
					bInAttr = 0;
				}
				p++;
				continue;
			}
			if ( *p == '[' ) {
				bInAttr = 1;
				p++;
				continue;
			}
			if ( *p == '(' ) {
				iParenDepth++;
				p++;
				continue;
			}
			if ( *p == ')' ) {
				if ( iParenDepth > 0 ) {
					iParenDepth--;
				}
				p++;
				continue;
			}
			if ( (iParenDepth == 0) && ((*p == '>') || (*p == '+') || (*p == '~')) ) {
				const char* pSplit = p;
				char cCombinator = *p;
				while ( (pSplit > pStart) && __xgeSvgCssIsSpace(pSplit[-1]) ) {
					pSplit--;
				}
				p++;
				while ( (p < pEnd) && __xgeSvgCssIsSpace(*p) ) {
					p++;
				}
				if ( p < pEnd ) {
					pLastPrevEnd = pSplit;
					pLastSimpleStart = p;
					if ( cCombinator == '>' ) {
						iLastCombinator = XGE_SVG_SELECTOR_COMBINATOR_CHILD;
					} else if ( cCombinator == '+' ) {
						iLastCombinator = XGE_SVG_SELECTOR_COMBINATOR_ADJACENT;
					} else {
						iLastCombinator = XGE_SVG_SELECTOR_COMBINATOR_GENERAL_SIBLING;
					}
					pCurrentStart = p;
				}
				continue;
			}
			if ( (iParenDepth == 0) && __xgeSvgCssIsSpace(*p) ) {
				const char* pSplit = p;
				while ( (p < pEnd) && __xgeSvgCssIsSpace(*p) ) {
					p++;
				}
				if ( (p < pEnd) && ((*p == '>') || (*p == '+') || (*p == '~')) ) {
					continue;
				}
				if ( p < pEnd ) {
					pLastPrevEnd = pSplit;
					pLastSimpleStart = p;
					iLastCombinator = XGE_SVG_SELECTOR_COMBINATOR_DESCENDANT;
					pCurrentStart = p;
				}
				continue;
			}
			p++;
		}
		(void)pCurrentStart;
		*ppSimpleStart = pLastSimpleStart;
		*ppSimpleEnd = pEnd;
		*ppPrevEnd = pLastPrevEnd;
		*pCombinator = iLastCombinator;
	}
}

static int __xgeSvgSelectorMatchesFromRight(xge_svg pSvg, const char* pSelectorStart, const char* pSelectorEnd, int iNodeIndex)
{
	const char* pSimpleStart;
	const char* pSimpleEnd;
	const char* pPrevEnd;
	const xge_svg_element_node_t* pNode;
	int iCombinator;

	if ( !__xgeSvgValid(pSvg) || (pSelectorStart == NULL) || (pSelectorEnd == NULL) ||
	     (iNodeIndex < 0) || (iNodeIndex >= pSvg->iElementNodeCount) ) {
		return 0;
	}
	__xgeSvgTrimRange(&pSelectorStart, &pSelectorEnd);
	if ( pSelectorEnd <= pSelectorStart ) {
		return 0;
	}
	pNode = &pSvg->pElementNodes[iNodeIndex];
	__xgeSvgSelectorRightmost(pSelectorStart, pSelectorEnd, &pSimpleStart, &pSimpleEnd, &pPrevEnd, &iCombinator);
	if ( !__xgeSvgSimpleSelectorRangeMatches(pSvg, pSimpleStart, pSimpleEnd, iNodeIndex) ) {
		return 0;
	}
	if ( pPrevEnd <= pSelectorStart ) {
		return 1;
	}
	if ( iCombinator == XGE_SVG_SELECTOR_COMBINATOR_CHILD ) {
		if ( pNode->iParentIndex < 0 ) {
			return 0;
		}
		return __xgeSvgSelectorMatchesFromRight(pSvg, pSelectorStart, pPrevEnd, pNode->iParentIndex);
	}
	if ( iCombinator == XGE_SVG_SELECTOR_COMBINATOR_DESCENDANT ) {
		int iParent;

		for ( iParent = pNode->iParentIndex; iParent >= 0; iParent = pSvg->pElementNodes[iParent].iParentIndex ) {
			if ( __xgeSvgSelectorMatchesFromRight(pSvg, pSelectorStart, pPrevEnd, iParent) ) {
				return 1;
			}
		}
		return 0;
	}
	if ( iCombinator == XGE_SVG_SELECTOR_COMBINATOR_ADJACENT ) {
		if ( pNode->iPrevSiblingIndex < 0 ) {
			return 0;
		}
		return __xgeSvgSelectorMatchesFromRight(pSvg, pSelectorStart, pPrevEnd, pNode->iPrevSiblingIndex);
	}
	if ( iCombinator == XGE_SVG_SELECTOR_COMBINATOR_GENERAL_SIBLING ) {
		int iSibling;

		for ( iSibling = pNode->iPrevSiblingIndex; iSibling >= 0; iSibling = pSvg->pElementNodes[iSibling].iPrevSiblingIndex ) {
			if ( __xgeSvgSelectorMatchesFromRight(pSvg, pSelectorStart, pPrevEnd, iSibling) ) {
				return 1;
			}
		}
	}
	return 0;
}

static int __xgeSvgElementNodeIsDescendantOf(xge_svg pSvg, int iNodeIndex, int iAncestorIndex)
{
	int iParent;

	if ( !__xgeSvgValid(pSvg) || (iNodeIndex < 0) || (iNodeIndex >= pSvg->iElementNodeCount) ||
	     (iAncestorIndex < 0) || (iAncestorIndex >= pSvg->iElementNodeCount) ) {
		return 0;
	}
	for ( iParent = pSvg->pElementNodes[iNodeIndex].iParentIndex; iParent >= 0; iParent = pSvg->pElementNodes[iParent].iParentIndex ) {
		if ( iParent == iAncestorIndex ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeSvgSelectorListMatchesFromRight(xge_svg pSvg, const char* pStart, const char* pEnd, int iNodeIndex, int* pMatched)
{
	const char* p;
	const char* pItemStart;
	int iItemCount;
	int iParenDepth;
	int bInAttr;
	char cQuote;

	if ( !__xgeSvgValid(pSvg) || (pStart == NULL) || (pEnd == NULL) ||
	     (iNodeIndex < 0) || (iNodeIndex >= pSvg->iElementNodeCount) || (pMatched == NULL) ) {
		return 0;
	}
	__xgeSvgTrimRange(&pStart, &pEnd);
	if ( pEnd <= pStart ) {
		return 0;
	}
	*pMatched = 0;
	p = pStart;
	pItemStart = pStart;
	iItemCount = 0;
	iParenDepth = 0;
	bInAttr = 0;
	cQuote = 0;
	while ( p <= pEnd ) {
		int bAtEnd;
		int bSplit;

		bAtEnd = (p == pEnd);
		bSplit = bAtEnd || ((cQuote == 0) && !bInAttr && (iParenDepth == 0) && (*p == ','));
		if ( bSplit ) {
			const char* pItemEnd;
			int iSpecificity;

			pItemEnd = p;
			__xgeSvgTrimRange(&pItemStart, &pItemEnd);
			if ( pItemEnd <= pItemStart ) {
				return 0;
			}
			if ( !__xgeSvgCssSelectorSpecificity(pItemStart, pItemEnd, &iSpecificity) ) {
				return 0;
			}
			if ( __xgeSvgSelectorMatchesFromRight(pSvg, pItemStart, pItemEnd, iNodeIndex) ) {
				*pMatched = 1;
			}
			iItemCount++;
			if ( bAtEnd ) {
				break;
			}
			p++;
			pItemStart = p;
			continue;
		}
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
		if ( bInAttr ) {
			if ( *p == ']' ) {
				bInAttr = 0;
			}
			p++;
			continue;
		}
		if ( *p == '[' ) {
			bInAttr = 1;
			p++;
			continue;
		}
		if ( *p == '(' ) {
			iParenDepth++;
			p++;
			continue;
		}
		if ( *p == ')' ) {
			if ( iParenDepth <= 0 ) {
				return 0;
			}
			iParenDepth--;
			p++;
			continue;
		}
		p++;
	}
	if ( (iItemCount <= 0) || bInAttr || (cQuote != 0) || (iParenDepth != 0) ) {
		return 0;
	}
	return 1;
}

static int __xgeSvgHasSelectorMatches(xge_svg pSvg, const char* pStart, const char* pEnd, int iNodeIndex)
{
	int iCandidate;

	if ( !__xgeSvgValid(pSvg) || (pStart == NULL) || (pEnd == NULL) ||
	     (iNodeIndex < 0) || (iNodeIndex >= pSvg->iElementNodeCount) ) {
		return 0;
	}
	__xgeSvgTrimRange(&pStart, &pEnd);
	if ( pEnd <= pStart ) {
		return 0;
	}
	if ( (*pStart == '>') || (*pStart == '+') || (*pStart == '~') ) {
		char cCombinator;

		cCombinator = *pStart++;
		__xgeSvgTrimRange(&pStart, &pEnd);
		if ( pEnd <= pStart ) {
			return 0;
		}
		if ( cCombinator == '>' ) {
			for ( iCandidate = 0; iCandidate < pSvg->iElementNodeCount; iCandidate++ ) {
				if ( (pSvg->pElementNodes[iCandidate].iParentIndex == iNodeIndex) &&
				     __xgeSvgSelectorMatchesFromRight(pSvg, pStart, pEnd, iCandidate) ) {
					return 1;
				}
			}
			return 0;
		}
		if ( cCombinator == '+' ) {
			iCandidate = pSvg->pElementNodes[iNodeIndex].iNextSiblingIndex;
			return (iCandidate >= 0) && __xgeSvgSelectorMatchesFromRight(pSvg, pStart, pEnd, iCandidate);
		}
		for ( iCandidate = pSvg->pElementNodes[iNodeIndex].iNextSiblingIndex; iCandidate >= 0; iCandidate = pSvg->pElementNodes[iCandidate].iNextSiblingIndex ) {
			if ( __xgeSvgSelectorMatchesFromRight(pSvg, pStart, pEnd, iCandidate) ) {
				return 1;
			}
		}
		return 0;
	}
	for ( iCandidate = 0; iCandidate < pSvg->iElementNodeCount; iCandidate++ ) {
		if ( (iCandidate != iNodeIndex) &&
		     __xgeSvgElementNodeIsDescendantOf(pSvg, iCandidate, iNodeIndex) &&
		     __xgeSvgSelectorMatchesFromRight(pSvg, pStart, pEnd, iCandidate) ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeSvgHasSelectorListMatches(xge_svg pSvg, const char* pStart, const char* pEnd, int iNodeIndex, int* pMatched)
{
	const char* p;
	const char* pItemStart;
	int iItemCount;
	int iParenDepth;
	int bInAttr;
	char cQuote;

	if ( !__xgeSvgValid(pSvg) || (pStart == NULL) || (pEnd == NULL) ||
	     (iNodeIndex < 0) || (iNodeIndex >= pSvg->iElementNodeCount) || (pMatched == NULL) ) {
		return 0;
	}
	__xgeSvgTrimRange(&pStart, &pEnd);
	if ( pEnd <= pStart ) {
		return 0;
	}
	*pMatched = 0;
	p = pStart;
	pItemStart = pStart;
	iItemCount = 0;
	iParenDepth = 0;
	bInAttr = 0;
	cQuote = 0;
	while ( p <= pEnd ) {
		int bAtEnd;
		int bSplit;

		bAtEnd = (p == pEnd);
		bSplit = bAtEnd || ((cQuote == 0) && !bInAttr && (iParenDepth == 0) && (*p == ','));
		if ( bSplit ) {
			const char* pItemEnd;
			int iSpecificity;

			pItemEnd = p;
			__xgeSvgTrimRange(&pItemStart, &pItemEnd);
			if ( pItemEnd <= pItemStart ) {
				return 0;
			}
			if ( !__xgeSvgCssRelativeSelectorSpecificity(pItemStart, pItemEnd, &iSpecificity) ) {
				return 0;
			}
			if ( __xgeSvgHasSelectorMatches(pSvg, pItemStart, pItemEnd, iNodeIndex) ) {
				*pMatched = 1;
			}
			iItemCount++;
			if ( bAtEnd ) {
				break;
			}
			p++;
			pItemStart = p;
			continue;
		}
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
		if ( bInAttr ) {
			if ( *p == ']' ) {
				bInAttr = 0;
			}
			p++;
			continue;
		}
		if ( *p == '[' ) {
			bInAttr = 1;
			p++;
			continue;
		}
		if ( *p == '(' ) {
			iParenDepth++;
			p++;
			continue;
		}
		if ( *p == ')' ) {
			if ( iParenDepth <= 0 ) {
				return 0;
			}
			iParenDepth--;
			p++;
			continue;
		}
		p++;
	}
	if ( (iItemCount <= 0) || bInAttr || (cQuote != 0) || (iParenDepth != 0) ) {
		return 0;
	}
	return 1;
}

static int __xgeSvgSelectorMatches(xge_svg pSvg, const char* sSelector, const char* pTag, const char* pTagEnd)
{
	int iNodeIndex;
	int iRet;

	if ( (sSelector == NULL) || (sSelector[0] == '\0') ) {
		return 0;
	}
	iNodeIndex = -1;
	iRet = __xgeSvgElementNodeEnsure(pSvg, pTag, pTagEnd, &iNodeIndex);
	if ( (iRet != XGE_OK) || (iNodeIndex < 0) ) {
		return 0;
	}
	return __xgeSvgSelectorMatchesFromRight(pSvg, sSelector, sSelector + strlen(sSelector), iNodeIndex);
}

static int __xgeSvgStyleRuleMatches(xge_svg pSvg, const xge_svg_style_rule_t* pRule, const char* pTag, const char* pTagEnd)
{
	char sValue[XGE_SVG_ATTR_MAX];

	if ( !__xgeSvgValid(pSvg) || (pRule == NULL) || (pRule->sName == NULL) || (pRule->sStyle == NULL) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return 0;
	}
	if ( pRule->iSelectorType == XGE_SVG_STYLE_SELECTOR_SIMPLE ) {
		return __xgeSvgSelectorMatches(pSvg, pRule->sName, pTag, pTagEnd);
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

static void __xgeSvgStyleApplyCss(xge_svg pSvg, xge_svg_style_t* pStyle, const char* pTag, const char* pTagEnd, int iImportanceMode, xge_shape_ex_matrix_t tBaseTransform, float fBaseOpacity, int bBaseVisible, float fBaseFontSize, int iApplyMode)
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
			if ( __xgeSvgStyleRuleMatches(pSvg, &pSvg->pStyleRules[i], pTag, pTagEnd) ) {
				iBestIndex = i;
				iBestSpecificity = iSpecificity;
			}
		}
		if ( iBestIndex < 0 ) {
			break;
		}
		__xgeSvgStyleApplyDeclaration(pSvg, pStyle, pSvg->pStyleRules[iBestIndex].sStyle, iImportanceMode, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, iApplyMode);
		iLastSpecificity = iBestSpecificity;
		iLastIndex = iBestIndex;
	}
}

static xge_svg_style_t __xgeSvgStyleResolve(xge_svg pSvg, const xge_svg_style_t* pParentStyle, const char* pTag, const char* pTagEnd)
{
	xge_svg_style_t tStyle;
	xge_shape_ex_matrix_t tBaseTransform;
	float fBaseOpacity;
	float fBaseFontSize;
	int bBaseVisible;

	(void)__xgeSvgElementNodeEnsure(pSvg, pTag, pTagEnd, NULL);
	if ( pParentStyle != NULL ) {
		tStyle = *pParentStyle;
	} else {
		tStyle = __xgeSvgStyleDefault();
	}
	tStyle.bStrokeNonScaling = 0;
	tBaseTransform = tStyle.tTransform;
	fBaseOpacity = tStyle.fOpacity;
	fBaseFontSize = tStyle.fFontSize;
	bBaseVisible = tStyle.bVisible;
	__xgeSvgStyleApplyPresentationAttrs(pSvg, &tStyle, pTag, pTagEnd, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_FONT_SIZE);
	__xgeSvgStyleApplyCss(pSvg, &tStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_NORMAL, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_FONT_SIZE);
	__xgeSvgStyleApplyInlineStyle(pSvg, &tStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_NORMAL, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_FONT_SIZE);
	__xgeSvgStyleApplyCss(pSvg, &tStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_IMPORTANT, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_FONT_SIZE);
	__xgeSvgStyleApplyInlineStyle(pSvg, &tStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_IMPORTANT, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_FONT_SIZE);
	__xgeSvgStyleApplyPresentationAttrs(pSvg, &tStyle, pTag, pTagEnd, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_EXCEPT_FONT_SIZE);
	__xgeSvgStyleApplyCss(pSvg, &tStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_NORMAL, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_EXCEPT_FONT_SIZE);
	__xgeSvgStyleApplyInlineStyle(pSvg, &tStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_NORMAL, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_EXCEPT_FONT_SIZE);
	__xgeSvgStyleApplyCss(pSvg, &tStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_IMPORTANT, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_EXCEPT_FONT_SIZE);
	__xgeSvgStyleApplyInlineStyle(pSvg, &tStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_IMPORTANT, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_EXCEPT_FONT_SIZE);
	return tStyle;
}

static xge_svg_clip_path_t* __xgeSvgClipPathGetById(xge_svg pSvg, const char* sClipId)
{
	int iClip;

	if ( !__xgeSvgValid(pSvg) || (sClipId == NULL) || (sClipId[0] == '\0') ) {
		return NULL;
	}
	iClip = __xgeSvgClipPathFind(pSvg, sClipId);
	if ( (iClip < 0) ||
	     ((!pSvg->pClipPaths[iClip].bHasRect || (pSvg->pClipPaths[iClip].iRectCount <= 0)) &&
	      (!pSvg->pClipPaths[iClip].bHasShape || (pSvg->pClipPaths[iClip].iShapeCount <= 0))) ) {
		return NULL;
	}
	return &pSvg->pClipPaths[iClip];
}

static int __xgeSvgClipPathResolveRect(xge_svg_clip_path_t* pClip, int iRect, xge_shape_ex pShape, xge_rect_t* pRect)
{
	xge_rect_t tClip;

	if ( (pClip == NULL) || (pShape == NULL) || (pRect == NULL) || (iRect < 0) || (iRect >= pClip->iRectCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tClip = pClip->pRects[iRect];
	if ( pClip->iUnits == XGE_SVG_CLIP_OBJECT_BOUNDING_BOX ) {
		xge_rect_t tBounds;

		if ( xgeShapeExGetBounds(pShape, 0.0f, &tBounds) != XGE_OK ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		tClip.fX = tBounds.fX + tClip.fX * tBounds.fW;
		tClip.fY = tBounds.fY + tClip.fY * tBounds.fH;
		tClip.fW = tClip.fW * tBounds.fW;
		tClip.fH = tClip.fH * tBounds.fH;
	}
	*pRect = tClip;
	return XGE_OK;
}

static int __xgeSvgApplyClipShapes(xge_svg_clip_path_t* pClip, xge_shape_ex pShape)
{
	xge_rect_t tBounds;
	xge_shape_ex_matrix_t tBoundsMatrix;
	int bHasBounds;
	int i;
	int iRet;

	if ( (pClip == NULL) || (pShape == NULL) || (pClip->iShapeCount <= 0) ) {
		return XGE_OK;
	}
	bHasBounds = 0;
	memset(&tBounds, 0, sizeof(tBounds));
	tBoundsMatrix = __xgeSvgMatrixIdentity();
	if ( pClip->iUnits == XGE_SVG_CLIP_OBJECT_BOUNDING_BOX ) {
		iRet = xgeShapeExGetBounds(pShape, 0.0f, &tBounds);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( (tBounds.fW <= 0.0f) || (tBounds.fH <= 0.0f) ) {
			return XGE_OK;
		}
		tBoundsMatrix = __xgeSvgMatrixMul(__xgeSvgMatrixTranslate(tBounds.fX, tBounds.fY), __xgeSvgMatrixScale(tBounds.fW, tBounds.fH));
		bHasBounds = 1;
	}
	for ( i = 0; i < pClip->iShapeCount; i++ ) {
		xge_shape_ex pClipShape;

		if ( pClip->pShapes[i] == NULL ) {
			continue;
		}
		if ( bHasBounds ) {
			xge_shape_ex_matrix_t tTransform;

			iRet = xgeShapeExClone(pClip->pShapes[i], &pClipShape);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			tTransform = __xgeSvgMatrixMul(tBoundsMatrix, pClipShape->tTransform);
			xgeShapeExTransformSet(pClipShape, &tTransform);
			iRet = xgeShapeExClipShapeAdd(pShape, pClipShape);
			xgeShapeExDestroy(pClipShape);
		} else {
			iRet = xgeShapeExClipShapeAdd(pShape, pClip->pShapes[i]);
		}
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeSvgApplyClipPath(xge_svg pSvg, xge_shape_ex pShape, const char* sClipId)
{
	xge_svg_clip_path_t* pClip;
	xge_rect_t tClip;
	int iRet;

	pClip = __xgeSvgClipPathGetById(pSvg, sClipId);
	if ( (pClip == NULL) || (pShape == NULL) ) {
		return XGE_OK;
	}
	if ( pClip->iRectCount > 0 ) {
		if ( __xgeSvgClipPathResolveRect(pClip, 0, pShape, &tClip) != XGE_OK ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		xgeShapeExClipRectSet(pShape, tClip);
	}
	iRet = __xgeSvgApplyClipShapes(pClip, pShape);
	return iRet;
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
	xgeShapeExStrokeNonScaling(pShape, pStyle->bStrokeNonScaling);
	xgeShapeExStrokeDash(pShape, pStyle->fDashPattern, pStyle->iDashCount, pStyle->fDashOffset);
	xgeShapeExFillRule(pShape, pStyle->iFillRule);
	xgeShapeExStrokeCap(pShape, pStyle->iLineCap);
	xgeShapeExStrokeJoin(pShape, pStyle->iLineJoin);
	xgeShapeExPaintOrder(pShape, pStyle->bStrokeFirst);
	xgeShapeExVisible(pShape, pStyle->bVisible && pStyle->bVisibility);
	xgeShapeExTransformSet(pShape, &pStyle->tTransform);
}

static int __xgeSvgApplyFillGradientPaintToShape(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, float fPaintOpacity)
{
	int iGradient;

	if ( !__xgeSvgValid(pSvg) || (pShape == NULL) || (pStyle == NULL) || (pStyle->sFillGradientId[0] == '\0') ) {
		return 0;
	}
	__xgeSvgResolveGradients(pSvg);
	iGradient = __xgeSvgLinearGradientFind(pSvg, pStyle->sFillGradientId);
	if ( (iGradient >= 0) && (pSvg->pLinearGradients[iGradient].iStopCount > 0) ) {
		xge_svg_linear_gradient_t* pGradient = &pSvg->pLinearGradients[iGradient];
		xge_shape_ex_color_stop_t arrStops[64];
		xge_shape_ex_color_stop_t* pStops;
		int i;
		int bApplied;

		pStops = (pGradient->iStopCount <= (int)(sizeof(arrStops) / sizeof(arrStops[0]))) ? arrStops :
			(xge_shape_ex_color_stop_t*)xrtMalloc((size_t)pGradient->iStopCount * sizeof(*pStops));
		if ( pStops == NULL ) {
			return 0;
		}
		for ( i = 0; i < pGradient->iStopCount; i++ ) {
			pStops[i] = pGradient->pStops[i];
			pStops[i].iColor = __xgeSvgColorAlpha(pStops[i].iColor, fPaintOpacity);
		}
		bApplied = xgeShapeExFillLinearGradient(pShape, pGradient->fX1, pGradient->fY1, pGradient->fX2, pGradient->fY2, pGradient->iUnits, pStops, pGradient->iStopCount) == XGE_OK;
		if ( bApplied ) {
			xgeShapeExFillGradientSpread(pShape, pGradient->iSpread);
			xgeShapeExFillGradientTransformSet(pShape, &pGradient->tTransform);
		}
		if ( pStops != arrStops ) {
			xrtFree(pStops);
		}
		return bApplied;
	}
	iGradient = __xgeSvgRadialGradientFind(pSvg, pStyle->sFillGradientId);
	if ( (iGradient >= 0) && (pSvg->pRadialGradients[iGradient].iStopCount > 0) ) {
		xge_svg_radial_gradient_t* pGradient = &pSvg->pRadialGradients[iGradient];
		xge_shape_ex_color_stop_t arrStops[64];
		xge_shape_ex_color_stop_t* pStops;
		int i;
		int bApplied;

		pStops = (pGradient->iStopCount <= (int)(sizeof(arrStops) / sizeof(arrStops[0]))) ? arrStops :
			(xge_shape_ex_color_stop_t*)xrtMalloc((size_t)pGradient->iStopCount * sizeof(*pStops));
		if ( pStops == NULL ) {
			return 0;
		}
		for ( i = 0; i < pGradient->iStopCount; i++ ) {
			pStops[i] = pGradient->pStops[i];
			pStops[i].iColor = __xgeSvgColorAlpha(pStops[i].iColor, fPaintOpacity);
		}
		bApplied = xgeShapeExFillRadialGradient(pShape, pGradient->fCX, pGradient->fCY, pGradient->fR, pGradient->fFX, pGradient->fFY, pGradient->iUnits, pStops, pGradient->iStopCount) == XGE_OK;
		if ( bApplied ) {
			xgeShapeExFillGradientSpread(pShape, pGradient->iSpread);
			xgeShapeExFillGradientTransformSet(pShape, &pGradient->tTransform);
		}
		if ( pStops != arrStops ) {
			xrtFree(pStops);
		}
		return bApplied;
	}
	return 0;
}

static int __xgeSvgApplyStrokeGradientPaintToShape(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, float fPaintOpacity)
{
	int iGradient;

	if ( !__xgeSvgValid(pSvg) || (pShape == NULL) || (pStyle == NULL) || (pStyle->sStrokeGradientId[0] == '\0') ) {
		return 0;
	}
	__xgeSvgResolveGradients(pSvg);
	iGradient = __xgeSvgLinearGradientFind(pSvg, pStyle->sStrokeGradientId);
	if ( (iGradient >= 0) && (pSvg->pLinearGradients[iGradient].iStopCount > 0) ) {
		xge_svg_linear_gradient_t* pGradient = &pSvg->pLinearGradients[iGradient];
		xge_shape_ex_color_stop_t arrStops[64];
		xge_shape_ex_color_stop_t* pStops;
		int i;
		int bApplied;

		pStops = (pGradient->iStopCount <= (int)(sizeof(arrStops) / sizeof(arrStops[0]))) ? arrStops :
			(xge_shape_ex_color_stop_t*)xrtMalloc((size_t)pGradient->iStopCount * sizeof(*pStops));
		if ( pStops == NULL ) {
			return 0;
		}
		for ( i = 0; i < pGradient->iStopCount; i++ ) {
			pStops[i] = pGradient->pStops[i];
			pStops[i].iColor = __xgeSvgColorAlpha(pStops[i].iColor, fPaintOpacity);
		}
		bApplied = xgeShapeExStrokeLinearGradient(pShape, pGradient->fX1, pGradient->fY1, pGradient->fX2, pGradient->fY2, pGradient->iUnits, pStops, pGradient->iStopCount) == XGE_OK;
		if ( bApplied ) {
			xgeShapeExStrokeGradientSpread(pShape, pGradient->iSpread);
			xgeShapeExStrokeGradientTransformSet(pShape, &pGradient->tTransform);
		}
		if ( pStops != arrStops ) {
			xrtFree(pStops);
		}
		return bApplied;
	}
	iGradient = __xgeSvgRadialGradientFind(pSvg, pStyle->sStrokeGradientId);
	if ( (iGradient >= 0) && (pSvg->pRadialGradients[iGradient].iStopCount > 0) ) {
		xge_svg_radial_gradient_t* pGradient = &pSvg->pRadialGradients[iGradient];
		xge_shape_ex_color_stop_t arrStops[64];
		xge_shape_ex_color_stop_t* pStops;
		int i;
		int bApplied;

		pStops = (pGradient->iStopCount <= (int)(sizeof(arrStops) / sizeof(arrStops[0]))) ? arrStops :
			(xge_shape_ex_color_stop_t*)xrtMalloc((size_t)pGradient->iStopCount * sizeof(*pStops));
		if ( pStops == NULL ) {
			return 0;
		}
		for ( i = 0; i < pGradient->iStopCount; i++ ) {
			pStops[i] = pGradient->pStops[i];
			pStops[i].iColor = __xgeSvgColorAlpha(pStops[i].iColor, fPaintOpacity);
		}
		bApplied = xgeShapeExStrokeRadialGradient(pShape, pGradient->fCX, pGradient->fCY, pGradient->fR, pGradient->fFX, pGradient->fFY, pGradient->iUnits, pStops, pGradient->iStopCount) == XGE_OK;
		if ( bApplied ) {
			xgeShapeExStrokeGradientSpread(pShape, pGradient->iSpread);
			xgeShapeExStrokeGradientTransformSet(pShape, &pGradient->tTransform);
		}
		if ( pStops != arrStops ) {
			xrtFree(pStops);
		}
		return bApplied;
	}
	return 0;
}

static void __xgeSvgApplyPathLengthToStyle(xge_shape_ex pShape, const char* pTag, const char* pTagEnd, xge_svg_style_t* pStyle)
{
	float fDeclaredLength;
	float fActualLength;
	float fScale;
	int i;

	if ( (pShape == NULL) || (pTag == NULL) || (pTagEnd == NULL) || (pStyle == NULL) || (pStyle->iDashCount <= 0) ) {
		return;
	}
	fDeclaredLength = __xgeSvgAttrFloat(pTag, pTagEnd, "pathLength", 0.0f);
	if ( fDeclaredLength <= 0.0f ) {
		return;
	}
	if ( (xgeShapeExGetLength(pShape, 0.25f, &fActualLength) != XGE_OK) || (fActualLength <= 0.0f) ) {
		return;
	}
	fScale = fActualLength / fDeclaredLength;
	for ( i = 0; i < pStyle->iDashCount; i++ ) {
		pStyle->fDashPattern[i] *= fScale;
	}
	pStyle->fDashOffset *= fScale;
}

static int __xgeSvgParsePointList(const char* sPoints, xge_vec2_t** ppPoints, int* pCount)
{
	const char* p;
	char* pEnd;
	xge_vec2_t* pPoints;
	double fValue;
	int iCount;
	int iCapacity;
	float fX;
	float fY;

	if ( (sPoints == NULL) || (ppPoints == NULL) || (pCount == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	p = sPoints;
	pPoints = NULL;
	iCount = 0;
	iCapacity = 0;
	while ( *p != '\0' ) {
		while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') || (*p == ',') ) p++;
		if ( *p == '\0' ) break;
		fValue = strtod(p, &pEnd);
		if ( pEnd == p ) {
			xrtFree(pPoints);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( !__xgeSvgDoubleToFloat(fValue, &fX) ) {
			xrtFree(pPoints);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		p = pEnd;
		while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') || (*p == ',') ) p++;
		fValue = strtod(p, &pEnd);
		if ( pEnd == p ) {
			xrtFree(pPoints);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( !__xgeSvgDoubleToFloat(fValue, &fY) ) {
			xrtFree(pPoints);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		p = pEnd;
		if ( iCount >= iCapacity ) {
			int iNewCapacity = iCapacity > 0 ? iCapacity * 2 : 8;
			xge_vec2_t* pNewPoints = (xge_vec2_t*)xrtRealloc(pPoints, (size_t)iNewCapacity * sizeof(*pNewPoints));
			if ( pNewPoints == NULL ) {
				xrtFree(pPoints);
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			pPoints = pNewPoints;
			iCapacity = iNewCapacity;
		}
		pPoints[iCount].fX = fX;
		pPoints[iCount].fY = fY;
		iCount++;
	}
	*ppPoints = pPoints;
	*pCount = iCount;
	return XGE_OK;
}

static int __xgeSvgAppendPointList(xge_shape_ex pShape, const xge_vec2_t* pPoints, int iCount, int bClose)
{
	int i;

	if ( (pShape == NULL) || (pPoints == NULL) || (iCount <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeShapeExMoveTo(pShape, pPoints[0].fX, pPoints[0].fY);
	for ( i = 1; i < iCount; i++ ) {
		xgeShapeExLineTo(pShape, pPoints[i].fX, pPoints[i].fY);
	}
	if ( bClose ) {
		xgeShapeExClose(pShape);
	}
	return XGE_OK;
}

static int __xgeSvgParsePoints(xge_shape_ex pShape, const char* sPoints, int bClose)
{
	xge_vec2_t* pPoints;
	int iCount;
	int iRet;

	if ( (pShape == NULL) || (sPoints == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pPoints = NULL;
	iCount = 0;
	iRet = __xgeSvgParsePointList(sPoints, &pPoints, &iCount);
	if ( (iRet == XGE_OK) && (iCount > 0) ) {
		iRet = __xgeSvgAppendPointList(pShape, pPoints, iCount, bClose);
	}
	xrtFree(pPoints);
	return iRet;
}

static int __xgeSvgAddShapeStyled(xge_svg pSvg, xge_shape_ex pShape, int iDefIndex, const xge_svg_style_t* pStyle)
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
		iRet = __xgeSvgDefAddShape(&pSvg->pDefs[iDefIndex], pShape, pStyle);
	} else {
		iRet = __xgeSvgDrawItemReserve(pSvg);
		if ( iRet == XGE_OK ) {
			xge_svg_draw_item_t* pItem = &pSvg->pItems[pSvg->iItemCount++];

			pItem->iType = XGE_SVG_DRAW_ITEM_SHAPE;
			xgeShapeExAddRef(pShape);
			pItem->u.pShape = pShape;
		}
	}
	xgeShapeExDestroy(pShape);
	return iRet;
}

static int __xgeSvgAddShape(xge_svg pSvg, xge_shape_ex pShape, int iDefIndex)
{
	return __xgeSvgAddShapeStyled(pSvg, pShape, iDefIndex, NULL);
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

static xge_rect_t __xgeSvgMaskRegionResolveBounds(const xge_svg_mask_t* pMask, xge_rect_t tBounds)
{
	xge_rect_t tRegion;

	if ( pMask == NULL ) {
		return tBounds;
	}
	tRegion = pMask->tRegion;
	if ( pMask->iUnits == XGE_SVG_MASK_OBJECT_BOUNDING_BOX ) {
		tRegion = __xgeSvgObjectRectToBounds(tRegion, tBounds);
	}
	return tRegion;
}

static int __xgeSvgShapeClipRectIntersect(xge_shape_ex pShape, xge_rect_t tClip)
{
	if ( pShape == NULL ) {
		return 0;
	}
	if ( pShape->bClipRect ) {
		tClip = __xgeSvgRectIntersect(pShape->tClipRect, tClip);
	}
	if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
		return 0;
	}
	xgeShapeExClipRectSet(pShape, tClip);
	return 1;
}

static int __xgeSvgTextClipRectIntersect(xge_svg_text_item_t* pText, xge_rect_t tClip)
{
	if ( pText == NULL ) {
		return 0;
	}
	if ( pText->bClipRect ) {
		tClip = __xgeSvgRectIntersect(pText->tClipRect, tClip);
	}
	if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
		return 0;
	}
	pText->bClipRect = 1;
	pText->tClipRect = tClip;
	return 1;
}

static int __xgeSvgImageClipRectIntersect(xge_svg_image_item_t* pImage, xge_rect_t tClip)
{
	if ( pImage == NULL ) {
		return 0;
	}
	if ( pImage->bClipRect ) {
		tClip = __xgeSvgRectIntersect(pImage->tClipRect, tClip);
	}
	if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
		return 0;
	}
	pImage->bClipRect = 1;
	pImage->tClipRect = tClip;
	return 1;
}

static int __xgeSvgRasterClipRectIntersect(xge_svg_raster_item_t* pImage, xge_rect_t tClip)
{
	if ( pImage == NULL ) {
		return 0;
	}
	if ( pImage->bClipRect ) {
		tClip = __xgeSvgRectIntersect(pImage->tClipRect, tClip);
	}
	if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
		return 0;
	}
	pImage->bClipRect = 1;
	pImage->tClipRect = tClip;
	return 1;
}

static int __xgeSvgApplyMaskShapeToShape(const xge_svg_mask_t* pMask, const xge_svg_mask_shape_t* pMaskShape, xge_shape_ex pTarget);
static xge_rect_t __xgeSvgMaskRectResolveBounds(const xge_svg_mask_t* pMask, xge_svg_mask_rect_t tMaskRect, xge_rect_t tBounds);

static int __xgeSvgAddStyledShapeMasked(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iDefIndex)
{
	xge_svg_mask_t* pMask;
	xge_rect_t tBounds;
	xge_rect_t tRegion;
	int iMask;
	int i;
	int bAdded;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pStyle == NULL) || (pStyle->sMaskId[0] == '\0') ) {
		return __xgeSvgAddShapeStyled(pSvg, pShape, iDefIndex, pStyle);
	}
	iMask = __xgeSvgMaskFind(pSvg, pStyle->sMaskId);
	if ( iMask < 0 ) {
		return __xgeSvgAddShapeStyled(pSvg, pShape, iDefIndex, pStyle);
	}
	pMask = &pSvg->pMasks[iMask];
	if ( (pMask->iRectCount <= 0) && (pMask->iShapeCount <= 0) ) {
		xgeShapeExDestroy(pShape);
		return XGE_OK;
	}
	memset(&tBounds, 0, sizeof(tBounds));
	if ( (pMask->iUnits == XGE_SVG_MASK_OBJECT_BOUNDING_BOX) || (pMask->iContentUnits == XGE_SVG_MASK_OBJECT_BOUNDING_BOX) ) {
		iRet = xgeShapeExGetBounds(pShape, 0.0f, &tBounds);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		if ( (tBounds.fW <= 0.0f) || (tBounds.fH <= 0.0f) ) {
			xgeShapeExDestroy(pShape);
			return XGE_OK;
		}
	}
	tRegion = __xgeSvgMaskRegionResolveBounds(pMask, tBounds);
	if ( (tRegion.fW <= 0.0f) || (tRegion.fH <= 0.0f) ) {
		xgeShapeExDestroy(pShape);
		return XGE_OK;
	}
	bAdded = 0;
	for ( i = 0; i < pMask->iRectCount; i++ ) {
		xge_svg_mask_rect_t tMaskRect = pMask->pRects[i];
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
		tClip = __xgeSvgMaskRectResolveBounds(pMask, tMaskRect, tBounds);
		tClip = __xgeSvgRectIntersect(tClip, tRegion);
		if ( pClone->bClipRect ) {
			tClip = __xgeSvgRectIntersect(pClone->tClipRect, tClip);
		}
		if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
			xgeShapeExDestroy(pClone);
			continue;
		}
		xgeShapeExClipRectSet(pClone, tClip);
		xgeShapeExOpacity(pClone, pClone->fOpacity * tMaskRect.fOpacity);
		iRet = __xgeSvgAddShapeStyled(pSvg, pClone, iDefIndex, pStyle);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		bAdded = 1;
	}
	for ( i = 0; i < pMask->iShapeCount; i++ ) {
		xge_svg_mask_shape_t* pMaskShape = &pMask->pShapes[i];
		xge_shape_ex pClone;

		if ( (pMaskShape->pShape == NULL) || (pMaskShape->fOpacity <= 0.0f) ) {
			continue;
		}
		iRet = xgeShapeExClone(pShape, &pClone);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		iRet = __xgeSvgApplyMaskShapeToShape(pMask, pMaskShape, pClone);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pClone);
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		{
			xge_rect_t tClip = tRegion;

			if ( pClone->bClipRect ) {
				tClip = __xgeSvgRectIntersect(pClone->tClipRect, tClip);
			}
			if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
				xgeShapeExDestroy(pClone);
				continue;
			}
			xgeShapeExClipRectSet(pClone, tClip);
		}
		xgeShapeExOpacity(pClone, pClone->fOpacity * pMaskShape->fOpacity);
		iRet = __xgeSvgAddShapeStyled(pSvg, pClone, iDefIndex, pStyle);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		bAdded = 1;
	}
	xgeShapeExDestroy(pShape);
	return bAdded ? XGE_OK : XGE_OK;
}

static int __xgeSvgAddTextItemCopy(xge_svg pSvg, const xge_svg_text_item_t* pText, int iDefIndex)
{
	xge_svg_draw_item_t* pItem;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pText == NULL) || (pText->sText == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pText->sText[0] == '\0' ) {
		return XGE_OK;
	}
	if ( iDefIndex >= 0 ) {
		if ( iDefIndex >= pSvg->iDefCount ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		return __xgeSvgDefAddText(&pSvg->pDefs[iDefIndex], pText);
	}
	iRet = __xgeSvgDrawItemReserve(pSvg);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pItem = &pSvg->pItems[pSvg->iItemCount];
	memset(pItem, 0, sizeof(*pItem));
	pItem->iType = XGE_SVG_DRAW_ITEM_TEXT;
	pItem->u.tText = *pText;
	pItem->u.tText.sText = __xgeStrDup(pText->sText);
	if ( pItem->u.tText.sText == NULL ) {
		memset(pItem, 0, sizeof(*pItem));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( pItem->u.tText.pTextPath != NULL ) {
		iRet = xgeShapeExAddRef(pItem->u.tText.pTextPath);
		if ( iRet != XGE_OK ) {
			__xgeSvgDrawItemReset(pItem);
			return iRet;
		}
	}
	if ( pItem->u.tText.pMaskShape != NULL ) {
		iRet = xgeShapeExAddRef(pItem->u.tText.pMaskShape);
		if ( iRet != XGE_OK ) {
			__xgeSvgDrawItemReset(pItem);
			return iRet;
		}
	}
	pSvg->iItemCount++;
	return XGE_OK;
}

static int __xgeSvgAddImageItemCopy(xge_svg pSvg, const xge_svg_image_item_t* pImage, int iDefIndex)
{
	xge_svg_draw_item_t* pItem;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pImage == NULL) || !__xgeSvgValid(pImage->pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pImage->tRect.fW <= 0.0f) || (pImage->tRect.fH <= 0.0f) ) {
		return XGE_OK;
	}
	if ( iDefIndex >= 0 ) {
		if ( iDefIndex >= pSvg->iDefCount ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		return __xgeSvgDefAddImage(&pSvg->pDefs[iDefIndex], pImage);
	}
	iRet = __xgeSvgDrawItemReserve(pSvg);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pItem = &pSvg->pItems[pSvg->iItemCount];
	memset(pItem, 0, sizeof(*pItem));
	pItem->iType = XGE_SVG_DRAW_ITEM_SVG_IMAGE;
	pItem->u.tImage = *pImage;
	iRet = xgeSvgAddRef(pItem->u.tImage.pSvg);
	if ( iRet != XGE_OK ) {
		__xgeSvgDrawItemReset(pItem);
		return iRet;
	}
	if ( pItem->u.tImage.pMaskShape != NULL ) {
		iRet = xgeShapeExAddRef(pItem->u.tImage.pMaskShape);
		if ( iRet != XGE_OK ) {
			__xgeSvgDrawItemReset(pItem);
			return iRet;
		}
	}
	pSvg->iItemCount++;
	return XGE_OK;
}

static int __xgeSvgAddRasterItemCopy(xge_svg pSvg, const xge_svg_raster_item_t* pImage, int iDefIndex)
{
	xge_svg_draw_item_t* pItem;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pImage == NULL) || (pImage->pRaster == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pImage->tRect.fW <= 0.0f) || (pImage->tRect.fH <= 0.0f) ) {
		return XGE_OK;
	}
	if ( iDefIndex >= 0 ) {
		if ( iDefIndex >= pSvg->iDefCount ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		return __xgeSvgDefAddRasterImage(&pSvg->pDefs[iDefIndex], pImage);
	}
	iRet = __xgeSvgDrawItemReserve(pSvg);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pItem = &pSvg->pItems[pSvg->iItemCount];
	memset(pItem, 0, sizeof(*pItem));
	pItem->iType = XGE_SVG_DRAW_ITEM_RASTER_IMAGE;
	pItem->u.tRaster = *pImage;
	iRet = __xgeSvgRasterImageAddRef(pItem->u.tRaster.pRaster);
	if ( iRet <= 0 ) {
		__xgeSvgDrawItemReset(pItem);
		return iRet;
	}
	if ( pItem->u.tRaster.pMaskShape != NULL ) {
		iRet = xgeShapeExAddRef(pItem->u.tRaster.pMaskShape);
		if ( iRet != XGE_OK ) {
			__xgeSvgDrawItemReset(pItem);
			return iRet;
		}
	}
	pSvg->iItemCount++;
	return XGE_OK;
}

static xge_rect_t __xgeSvgTextMaskBounds(const xge_svg_text_item_t* pText)
{
	xge_rect_t tBounds;
	float fWidth;
	float fHeight;

	memset(&tBounds, 0, sizeof(tBounds));
	if ( pText == NULL ) {
		return tBounds;
	}
	fHeight = pText->tStyle.fFontSize > 0.0f ? pText->tStyle.fFontSize : 16.0f;
	fWidth = pText->fAnchorAdvance;
	if ( fWidth <= 0.0f && pText->sText != NULL ) {
		fWidth = (float)strlen(pText->sText) * fHeight * 0.5f;
	}
	tBounds.fX = pText->fX;
	if ( pText->tStyle.iTextAnchor == XGE_SVG_TEXT_ANCHOR_MIDDLE ) {
		tBounds.fX -= fWidth * 0.5f;
	} else if ( pText->tStyle.iTextAnchor == XGE_SVG_TEXT_ANCHOR_END ) {
		tBounds.fX -= fWidth;
	}
	tBounds.fY = pText->fY - fHeight;
	tBounds.fW = fWidth;
	tBounds.fH = fHeight;
	return tBounds;
}

static xge_rect_t __xgeSvgMaskRectResolveBounds(const xge_svg_mask_t* pMask, xge_svg_mask_rect_t tMaskRect, xge_rect_t tBounds)
{
	xge_rect_t tClip;

	tClip = tMaskRect.tRect;
	if ( (pMask != NULL) && (pMask->iContentUnits == XGE_SVG_MASK_OBJECT_BOUNDING_BOX) ) {
		tClip.fX = tBounds.fX + tClip.fX * tBounds.fW;
		tClip.fY = tBounds.fY + tClip.fY * tBounds.fH;
		tClip.fW *= tBounds.fW;
		tClip.fH *= tBounds.fH;
	}
	return tClip;
}

static int __xgeSvgApplyMaskShapeToShape(const xge_svg_mask_t* pMask, const xge_svg_mask_shape_t* pMaskShape, xge_shape_ex pTarget)
{
	xge_shape_ex pClipShape;
	int iRet;

	if ( (pMask == NULL) || (pMaskShape == NULL) || (pMaskShape->pShape == NULL) || (pTarget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pMask->iContentUnits == XGE_SVG_MASK_OBJECT_BOUNDING_BOX ) {
		xge_rect_t tBounds;
		xge_shape_ex_matrix_t tBoundsMatrix;
		xge_shape_ex_matrix_t tTransform;

		iRet = xgeShapeExGetBounds(pTarget, 0.0f, &tBounds);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( (tBounds.fW <= 0.0f) || (tBounds.fH <= 0.0f) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExClone(pMaskShape->pShape, &pClipShape);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		tBoundsMatrix = __xgeSvgMatrixMul(__xgeSvgMatrixTranslate(tBounds.fX, tBounds.fY), __xgeSvgMatrixScale(tBounds.fW, tBounds.fH));
		tTransform = __xgeSvgMatrixMul(tBoundsMatrix, pClipShape->tTransform);
		xgeShapeExTransformSet(pClipShape, &tTransform);
		iRet = xgeShapeExClipShapeAdd(pTarget, pClipShape);
		xgeShapeExDestroy(pClipShape);
		return iRet;
	}
	return xgeShapeExClipShapeAdd(pTarget, pMaskShape->pShape);
}

static int __xgeSvgMaskShapeResolveForBounds(const xge_svg_mask_t* pMask, const xge_svg_mask_shape_t* pMaskShape, xge_rect_t tBounds, xge_shape_ex* ppShape)
{
	xge_shape_ex pShape;
	int iRet;

	if ( ppShape == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppShape = NULL;
	if ( (pMask == NULL) || (pMaskShape == NULL) || (pMaskShape->pShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pMask->iContentUnits == XGE_SVG_MASK_OBJECT_BOUNDING_BOX ) {
		xge_shape_ex_matrix_t tBoundsMatrix;
		xge_shape_ex_matrix_t tTransform;

		if ( (tBounds.fW <= 0.0f) || (tBounds.fH <= 0.0f) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExClone(pMaskShape->pShape, &pShape);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		tBoundsMatrix = __xgeSvgMatrixMul(__xgeSvgMatrixTranslate(tBounds.fX, tBounds.fY), __xgeSvgMatrixScale(tBounds.fW, tBounds.fH));
		tTransform = __xgeSvgMatrixMul(tBoundsMatrix, pShape->tTransform);
		xgeShapeExTransformSet(pShape, &tTransform);
		*ppShape = pShape;
		return XGE_OK;
	}
	iRet = xgeShapeExAddRef(pMaskShape->pShape);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	*ppShape = pMaskShape->pShape;
	return XGE_OK;
}

static int __xgeSvgAddTextItemMasked(xge_svg pSvg, const xge_svg_text_item_t* pText, int iDefIndex)
{
	xge_svg_mask_t* pMask;
	xge_rect_t tBounds;
	xge_rect_t tRegion;
	int iMask;
	int i;
	int bAdded;
	int iRet;
	xge_svg_text_item_t tFilterOriginal;

	if ( !__xgeSvgValid(pSvg) || (pText == NULL) || (pText->sText == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pText->tStyle.sFilterId[0] != '\0' ) {
		xge_rect_t tFilterRegion;
		float fOffsetX;
		float fOffsetY;
		uint32_t iShadowColor;
		float fShadowOpacity;
		float fShadowStdDeviation;
		int iFilterRegion;

		tBounds = __xgeSvgTextMaskBounds(pText);
		iFilterRegion = __xgeSvgFilterRegionForBounds(pSvg, pText->tStyle.sFilterId, tBounds, &tFilterRegion);
		if ( iFilterRegion < 0 ) {
			return XGE_OK;
		}
		if ( __xgeSvgFilterDropShadowForBounds(pSvg, pText->tStyle.sFilterId, tBounds, &fOffsetX, &fOffsetY, &iShadowColor, &fShadowOpacity, &fShadowStdDeviation) ) {
			static const float arrTaps[9][3] = {
				{ 0.0f, 0.0f, 0.28f },
				{ -1.0f, 0.0f, 0.12f }, { 1.0f, 0.0f, 0.12f }, { 0.0f, -1.0f, 0.12f }, { 0.0f, 1.0f, 0.12f },
				{ -0.70710678f, -0.70710678f, 0.06f }, { 0.70710678f, -0.70710678f, 0.06f },
				{ -0.70710678f, 0.70710678f, 0.06f }, { 0.70710678f, 0.70710678f, 0.06f }
			};
			int iTapCount;
			int iTap;

			iTapCount = fShadowStdDeviation > 0.0001f ? 9 : 1;
			for ( iTap = 0; iTap < iTapCount; iTap++ ) {
				xge_svg_text_item_t tShadow;
				xge_shape_ex_matrix_t tOffset;
				float fTapOpacity;
				float fTapX;
				float fTapY;

				fTapOpacity = fShadowOpacity * (iTapCount > 1 ? arrTaps[iTap][2] : 1.0f);
				if ( fTapOpacity <= 0.0f ) {
					continue;
				}
				fTapX = fOffsetX + (iTapCount > 1 ? arrTaps[iTap][0] * fShadowStdDeviation : 0.0f);
				fTapY = fOffsetY + (iTapCount > 1 ? arrTaps[iTap][1] * fShadowStdDeviation : 0.0f);
				tShadow = *pText;
				tShadow.tStyle.sFilterId[0] = '\0';
				tOffset = __xgeSvgMatrixTranslate(fTapX, fTapY);
				tShadow.tStyle.tTransform = __xgeSvgMatrixMul(tOffset, tShadow.tStyle.tTransform);
				__xgeSvgApplyDropShadowPaintToText(&tShadow, iShadowColor, fTapOpacity);
				if ( (iFilterRegion > 0) && !__xgeSvgTextClipRectIntersect(&tShadow, tFilterRegion) ) {
					continue;
				}
				iRet = __xgeSvgAddTextItemMasked(pSvg, &tShadow, iDefIndex);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
			}

			tFilterOriginal = *pText;
			tFilterOriginal.tStyle.sFilterId[0] = '\0';
			if ( (iFilterRegion > 0) && !__xgeSvgTextClipRectIntersect(&tFilterOriginal, tFilterRegion) ) {
				return XGE_OK;
			}
			pText = &tFilterOriginal;
		}
		if ( __xgeSvgFilterOffsetForBounds(pSvg, pText->tStyle.sFilterId, tBounds, &fOffsetX, &fOffsetY) ) {
			xge_svg_text_item_t tCopy;
			xge_shape_ex_matrix_t tOffset;

			tCopy = *pText;
			tCopy.tStyle.sFilterId[0] = '\0';
			tOffset = __xgeSvgMatrixTranslate(fOffsetX, fOffsetY);
			tCopy.tStyle.tTransform = __xgeSvgMatrixMul(tOffset, tCopy.tStyle.tTransform);
			if ( (iFilterRegion > 0) && !__xgeSvgTextClipRectIntersect(&tCopy, tFilterRegion) ) {
				return XGE_OK;
			}
			return __xgeSvgAddTextItemMasked(pSvg, &tCopy, iDefIndex);
		}
	}
	if ( pText->tStyle.sMaskId[0] == '\0' ) {
		return __xgeSvgAddTextItemCopy(pSvg, pText, iDefIndex);
	}
	iMask = __xgeSvgMaskFind(pSvg, pText->tStyle.sMaskId);
	if ( iMask < 0 ) {
		return __xgeSvgAddTextItemCopy(pSvg, pText, iDefIndex);
	}
	pMask = &pSvg->pMasks[iMask];
	if ( (pMask->iRectCount <= 0) && (pMask->iShapeCount <= 0) ) {
		return XGE_OK;
	}
	tBounds = __xgeSvgTextMaskBounds(pText);
	tRegion = __xgeSvgMaskRegionResolveBounds(pMask, tBounds);
	if ( (tRegion.fW <= 0.0f) || (tRegion.fH <= 0.0f) ) {
		return XGE_OK;
	}
	bAdded = 0;
	for ( i = 0; i < pMask->iRectCount; i++ ) {
		xge_svg_mask_rect_t tMaskRect;
		xge_svg_text_item_t tCopy;
		xge_rect_t tClip;

		tMaskRect = pMask->pRects[i];
		if ( (tMaskRect.fOpacity <= 0.0f) || (tMaskRect.tRect.fW <= 0.0f) || (tMaskRect.tRect.fH <= 0.0f) ) {
			continue;
		}
		tCopy = *pText;
		tCopy.tStyle.fOpacity *= tMaskRect.fOpacity;
		tClip = __xgeSvgMaskRectResolveBounds(pMask, tMaskRect, tBounds);
		tClip = __xgeSvgRectIntersect(tClip, tRegion);
		if ( tCopy.bClipRect ) {
			tClip = __xgeSvgRectIntersect(tCopy.tClipRect, tClip);
		}
		if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
			continue;
		}
		tCopy.bClipRect = 1;
		tCopy.tClipRect = tClip;
		iRet = __xgeSvgAddTextItemCopy(pSvg, &tCopy, iDefIndex);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		bAdded = 1;
	}
	for ( i = 0; i < pMask->iShapeCount; i++ ) {
		xge_svg_mask_shape_t* pMaskShape;
		xge_svg_text_item_t tCopy;

		pMaskShape = &pMask->pShapes[i];
		if ( (pMaskShape->pShape == NULL) || (pMaskShape->fOpacity <= 0.0f) ) {
			continue;
		}
		tCopy = *pText;
		tCopy.tStyle.fOpacity *= pMaskShape->fOpacity;
		if ( tCopy.bClipRect ) {
			tCopy.tClipRect = __xgeSvgRectIntersect(tCopy.tClipRect, tRegion);
		} else {
			tCopy.bClipRect = 1;
			tCopy.tClipRect = tRegion;
		}
		if ( (tCopy.tClipRect.fW <= 0.0f) || (tCopy.tClipRect.fH <= 0.0f) ) {
			continue;
		}
		tCopy.pMaskShape = NULL;
		iRet = __xgeSvgMaskShapeResolveForBounds(pMask, pMaskShape, tBounds, &tCopy.pMaskShape);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( tCopy.pMaskShape == NULL ) {
			continue;
		}
		iRet = __xgeSvgAddTextItemCopy(pSvg, &tCopy, iDefIndex);
		xgeShapeExDestroy(tCopy.pMaskShape);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		bAdded = 1;
	}
	return bAdded ? XGE_OK : XGE_OK;
}

static int __xgeSvgAddImageItemMasked(xge_svg pSvg, const xge_svg_image_item_t* pImage, int iDefIndex)
{
	xge_svg_mask_t* pMask;
	xge_rect_t tRegion;
	int iMask;
	int i;
	int bAdded;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pImage == NULL) || !__xgeSvgValid(pImage->pSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pImage->tStyle.sFilterId[0] != '\0' ) {
		xge_rect_t tFilterRegion;
		float fOffsetX;
		float fOffsetY;
		int iFilterRegion;

		iFilterRegion = __xgeSvgFilterRegionForBounds(pSvg, pImage->tStyle.sFilterId, pImage->tRect, &tFilterRegion);
		if ( iFilterRegion < 0 ) {
			return XGE_OK;
		}
		if ( __xgeSvgFilterOffsetForBounds(pSvg, pImage->tStyle.sFilterId, pImage->tRect, &fOffsetX, &fOffsetY) ) {
			xge_svg_image_item_t tCopy;
			xge_shape_ex_matrix_t tOffset;

			tCopy = *pImage;
			tCopy.tStyle.sFilterId[0] = '\0';
			tOffset = __xgeSvgMatrixTranslate(fOffsetX, fOffsetY);
			tCopy.tStyle.tTransform = __xgeSvgMatrixMul(tOffset, tCopy.tStyle.tTransform);
			if ( (iFilterRegion > 0) && !__xgeSvgImageClipRectIntersect(&tCopy, tFilterRegion) ) {
				return XGE_OK;
			}
			return __xgeSvgAddImageItemMasked(pSvg, &tCopy, iDefIndex);
		}
	}
	if ( pImage->tStyle.sMaskId[0] == '\0' ) {
		return __xgeSvgAddImageItemCopy(pSvg, pImage, iDefIndex);
	}
	iMask = __xgeSvgMaskFind(pSvg, pImage->tStyle.sMaskId);
	if ( iMask < 0 ) {
		return __xgeSvgAddImageItemCopy(pSvg, pImage, iDefIndex);
	}
	pMask = &pSvg->pMasks[iMask];
	if ( (pMask->iRectCount <= 0) && (pMask->iShapeCount <= 0) ) {
		return XGE_OK;
	}
	tRegion = __xgeSvgMaskRegionResolveBounds(pMask, pImage->tRect);
	if ( (tRegion.fW <= 0.0f) || (tRegion.fH <= 0.0f) ) {
		return XGE_OK;
	}
	bAdded = 0;
	for ( i = 0; i < pMask->iRectCount; i++ ) {
		xge_svg_mask_rect_t tMaskRect;
		xge_svg_image_item_t tCopy;
		xge_rect_t tClip;

		tMaskRect = pMask->pRects[i];
		if ( (tMaskRect.fOpacity <= 0.0f) || (tMaskRect.tRect.fW <= 0.0f) || (tMaskRect.tRect.fH <= 0.0f) ) {
			continue;
		}
		tCopy = *pImage;
		tCopy.tStyle.fOpacity *= tMaskRect.fOpacity;
		tClip = __xgeSvgMaskRectResolveBounds(pMask, tMaskRect, pImage->tRect);
		tClip = __xgeSvgRectIntersect(tClip, tRegion);
		if ( tCopy.bClipRect ) {
			tClip = __xgeSvgRectIntersect(tCopy.tClipRect, tClip);
		}
		if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
			continue;
		}
		tCopy.bClipRect = 1;
		tCopy.tClipRect = tClip;
		iRet = __xgeSvgAddImageItemCopy(pSvg, &tCopy, iDefIndex);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		bAdded = 1;
	}
	for ( i = 0; i < pMask->iShapeCount; i++ ) {
		xge_svg_mask_shape_t* pMaskShape;
		xge_svg_image_item_t tCopy;

		pMaskShape = &pMask->pShapes[i];
		if ( (pMaskShape->pShape == NULL) || (pMaskShape->fOpacity <= 0.0f) ) {
			continue;
		}
		tCopy = *pImage;
		tCopy.tStyle.fOpacity *= pMaskShape->fOpacity;
		if ( tCopy.bClipRect ) {
			tCopy.tClipRect = __xgeSvgRectIntersect(tCopy.tClipRect, tRegion);
		} else {
			tCopy.bClipRect = 1;
			tCopy.tClipRect = tRegion;
		}
		if ( (tCopy.tClipRect.fW <= 0.0f) || (tCopy.tClipRect.fH <= 0.0f) ) {
			continue;
		}
		tCopy.pMaskShape = NULL;
		iRet = __xgeSvgMaskShapeResolveForBounds(pMask, pMaskShape, pImage->tRect, &tCopy.pMaskShape);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( tCopy.pMaskShape == NULL ) {
			continue;
		}
		iRet = __xgeSvgAddImageItemCopy(pSvg, &tCopy, iDefIndex);
		xgeShapeExDestroy(tCopy.pMaskShape);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		bAdded = 1;
	}
	return bAdded ? XGE_OK : XGE_OK;
}

static int __xgeSvgAddRasterItemMasked(xge_svg pSvg, const xge_svg_raster_item_t* pImage, int iDefIndex)
{
	xge_svg_mask_t* pMask;
	xge_svg_raster_item_t tFilterOriginal;
	xge_rect_t tRegion;
	int iMask;
	int i;
	int bAdded;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pImage == NULL) || (pImage->pRaster == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pImage->tStyle.sFilterId[0] != '\0' ) {
		xge_rect_t tFilterRegion;
		float fOffsetX;
		float fOffsetY;
		uint32_t iShadowColor;
		float fShadowOpacity;
		float fShadowStdDeviation;
		int iFilterRegion;

		iFilterRegion = __xgeSvgFilterRegionForBounds(pSvg, pImage->tStyle.sFilterId, pImage->tRect, &tFilterRegion);
		if ( iFilterRegion < 0 ) {
			return XGE_OK;
		}
		if ( __xgeSvgFilterDropShadowForBounds(pSvg, pImage->tStyle.sFilterId, pImage->tRect, &fOffsetX, &fOffsetY, &iShadowColor, &fShadowOpacity, &fShadowStdDeviation) ) {
			static const float arrTaps[9][3] = {
				{ 0.0f, 0.0f, 0.28f },
				{ -1.0f, 0.0f, 0.12f }, { 1.0f, 0.0f, 0.12f }, { 0.0f, -1.0f, 0.12f }, { 0.0f, 1.0f, 0.12f },
				{ -0.70710678f, -0.70710678f, 0.06f }, { 0.70710678f, -0.70710678f, 0.06f },
				{ -0.70710678f, 0.70710678f, 0.06f }, { 0.70710678f, 0.70710678f, 0.06f }
			};
			int iTapCount;
			int iTap;

			iTapCount = fShadowStdDeviation > 0.0001f ? 9 : 1;
			for ( iTap = 0; iTap < iTapCount; iTap++ ) {
				xge_svg_raster_item_t tShadow;
				xge_shape_ex_matrix_t tOffset;
				float fTapOpacity;
				float fTapX;
				float fTapY;

				fTapOpacity = fShadowOpacity * (iTapCount > 1 ? arrTaps[iTap][2] : 1.0f);
				if ( fTapOpacity <= 0.0f ) {
					continue;
				}
				fTapX = fOffsetX + (iTapCount > 1 ? arrTaps[iTap][0] * fShadowStdDeviation : 0.0f);
				fTapY = fOffsetY + (iTapCount > 1 ? arrTaps[iTap][1] * fShadowStdDeviation : 0.0f);
				tShadow = *pImage;
				tShadow.tStyle.sFilterId[0] = '\0';
				tShadow.iColor = __xgeSvgFilterShadowColor(iShadowColor, fTapOpacity, 1.0f);
				tOffset = __xgeSvgMatrixTranslate(fTapX, fTapY);
				tShadow.tStyle.tTransform = __xgeSvgMatrixMul(tOffset, tShadow.tStyle.tTransform);
				if ( (iFilterRegion > 0) && !__xgeSvgRasterClipRectIntersect(&tShadow, tFilterRegion) ) {
					continue;
				}
				iRet = __xgeSvgAddRasterItemMasked(pSvg, &tShadow, iDefIndex);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
			}

			tFilterOriginal = *pImage;
			tFilterOriginal.tStyle.sFilterId[0] = '\0';
			if ( (iFilterRegion > 0) && !__xgeSvgRasterClipRectIntersect(&tFilterOriginal, tFilterRegion) ) {
				return XGE_OK;
			}
			pImage = &tFilterOriginal;
		}

		if ( __xgeSvgFilterOffsetForBounds(pSvg, pImage->tStyle.sFilterId, pImage->tRect, &fOffsetX, &fOffsetY) ) {
			xge_svg_raster_item_t tCopy;
			xge_shape_ex_matrix_t tOffset;

			tCopy = *pImage;
			tCopy.tStyle.sFilterId[0] = '\0';
			tOffset = __xgeSvgMatrixTranslate(fOffsetX, fOffsetY);
			tCopy.tStyle.tTransform = __xgeSvgMatrixMul(tOffset, tCopy.tStyle.tTransform);
			if ( (iFilterRegion > 0) && !__xgeSvgRasterClipRectIntersect(&tCopy, tFilterRegion) ) {
				return XGE_OK;
			}
			return __xgeSvgAddRasterItemMasked(pSvg, &tCopy, iDefIndex);
		}
	}
	if ( pImage->tStyle.sMaskId[0] == '\0' ) {
		return __xgeSvgAddRasterItemCopy(pSvg, pImage, iDefIndex);
	}
	iMask = __xgeSvgMaskFind(pSvg, pImage->tStyle.sMaskId);
	if ( iMask < 0 ) {
		return __xgeSvgAddRasterItemCopy(pSvg, pImage, iDefIndex);
	}
	pMask = &pSvg->pMasks[iMask];
	if ( (pMask->iRectCount <= 0) && (pMask->iShapeCount <= 0) ) {
		return XGE_OK;
	}
	tRegion = __xgeSvgMaskRegionResolveBounds(pMask, pImage->tRect);
	if ( (tRegion.fW <= 0.0f) || (tRegion.fH <= 0.0f) ) {
		return XGE_OK;
	}
	bAdded = 0;
	for ( i = 0; i < pMask->iRectCount; i++ ) {
		xge_svg_mask_rect_t tMaskRect;
		xge_svg_raster_item_t tCopy;
		xge_rect_t tClip;

		tMaskRect = pMask->pRects[i];
		if ( (tMaskRect.fOpacity <= 0.0f) || (tMaskRect.tRect.fW <= 0.0f) || (tMaskRect.tRect.fH <= 0.0f) ) {
			continue;
		}
		tCopy = *pImage;
		tCopy.tStyle.fOpacity *= tMaskRect.fOpacity;
		tClip = __xgeSvgMaskRectResolveBounds(pMask, tMaskRect, pImage->tRect);
		tClip = __xgeSvgRectIntersect(tClip, tRegion);
		if ( tCopy.bClipRect ) {
			tClip = __xgeSvgRectIntersect(tCopy.tClipRect, tClip);
		}
		if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
			continue;
		}
		tCopy.bClipRect = 1;
		tCopy.tClipRect = tClip;
		iRet = __xgeSvgAddRasterItemCopy(pSvg, &tCopy, iDefIndex);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		bAdded = 1;
	}
	for ( i = 0; i < pMask->iShapeCount; i++ ) {
		xge_svg_mask_shape_t* pMaskShape;
		xge_svg_raster_item_t tCopy;

		pMaskShape = &pMask->pShapes[i];
		if ( (pMaskShape->pShape == NULL) || (pMaskShape->fOpacity <= 0.0f) ) {
			continue;
		}
		tCopy = *pImage;
		tCopy.tStyle.fOpacity *= pMaskShape->fOpacity;
		if ( tCopy.bClipRect ) {
			tCopy.tClipRect = __xgeSvgRectIntersect(tCopy.tClipRect, tRegion);
		} else {
			tCopy.bClipRect = 1;
			tCopy.tClipRect = tRegion;
		}
		if ( (tCopy.tClipRect.fW <= 0.0f) || (tCopy.tClipRect.fH <= 0.0f) ) {
			continue;
		}
		tCopy.pMaskShape = NULL;
		iRet = __xgeSvgMaskShapeResolveForBounds(pMask, pMaskShape, pImage->tRect, &tCopy.pMaskShape);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( tCopy.pMaskShape == NULL ) {
			continue;
		}
		iRet = __xgeSvgAddRasterItemCopy(pSvg, &tCopy, iDefIndex);
		xgeShapeExDestroy(tCopy.pMaskShape);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		bAdded = 1;
	}
	return bAdded ? XGE_OK : XGE_OK;
}

static xge_svg_pattern_t* __xgeSvgPatternGetForPaint(xge_svg pSvg, const char* sPaintId)
{
	int iPattern;

	if ( !__xgeSvgValid(pSvg) || (sPaintId == NULL) || (sPaintId[0] == '\0') ) {
		return NULL;
	}
	if ( (__xgeSvgLinearGradientFind(pSvg, sPaintId) >= 0) ||
	     (__xgeSvgRadialGradientFind(pSvg, sPaintId) >= 0) ) {
		return NULL;
	}
	iPattern = __xgeSvgPatternFind(pSvg, sPaintId);
	if ( iPattern >= 0 ) {
		if ( __xgeSvgResolvePattern(pSvg, iPattern, 0) != XGE_OK ) {
			return NULL;
		}
	}
	if ( (iPattern < 0) || (pSvg->pPatterns[iPattern].iDefIndex < 0) ||
	     (pSvg->pPatterns[iPattern].iDefIndex >= pSvg->iDefCount) ||
	     (pSvg->pPatterns[iPattern].tRect.fW <= 0.0f) ||
	     (pSvg->pPatterns[iPattern].tRect.fH <= 0.0f) ) {
		return NULL;
	}
	return &pSvg->pPatterns[iPattern];
}

static xge_rect_t __xgeSvgPatternRectToUser(const xge_svg_pattern_t* pPattern, xge_rect_t tBounds)
{
	xge_rect_t tRect;

	tRect = pPattern->tRect;
	if ( pPattern->iUnits == XGE_SVG_PATTERN_OBJECT_BOUNDING_BOX ) {
		tRect.fX = tBounds.fX + tRect.fX * tBounds.fW;
		tRect.fY = tBounds.fY + tRect.fY * tBounds.fH;
		tRect.fW = tRect.fW * tBounds.fW;
		tRect.fH = tRect.fH * tBounds.fH;
	}
	return tRect;
}

static float __xgeSvgPatternFirstTile(float fStart, float fSize, float fLimit)
{
	if ( fSize <= 0.0f ) {
		return fStart;
	}
	while ( fStart > fLimit ) {
		fStart -= fSize;
	}
	while ( (fStart + fSize) <= fLimit ) {
		fStart += fSize;
	}
	return fStart;
}

static xge_shape_ex_matrix_t __xgeSvgPatternTileMatrix(xge_svg pSvg, const xge_svg_pattern_t* pPattern, xge_rect_t tTile, xge_rect_t tObjectBounds)
{
	xge_shape_ex_matrix_t tBase;
	xge_svg_def_t* pDef;

	pDef = &pSvg->pDefs[pPattern->iDefIndex];
	if ( pDef->bHasViewBox ) {
		xge_rect_t tViewport = __xgeSvgAspectViewport(pDef->tViewBox, tTile, pDef->iAspectAlignX, pDef->iAspectAlignY, pDef->iAspectMeetOrSlice);
		tBase = __xgeSvgViewBoxMatrix(pDef->tViewBox, tViewport);
	} else {
		tBase = __xgeSvgMatrixTranslate(tTile.fX, tTile.fY);
		if ( pPattern->iContentUnits == XGE_SVG_PATTERN_OBJECT_BOUNDING_BOX ) {
			tBase = __xgeSvgMatrixMul(tBase, __xgeSvgMatrixScale(tObjectBounds.fW, tObjectBounds.fH));
		}
	}
	return __xgeSvgMatrixMul(pPattern->tTransform, tBase);
}

static xge_rect_t __xgeSvgRectInflate(xge_rect_t tRect, float fMargin)
{
	if ( fMargin <= 0.0f ) {
		return tRect;
	}
	tRect.fX -= fMargin;
	tRect.fY -= fMargin;
	tRect.fW += fMargin * 2.0f;
	tRect.fH += fMargin * 2.0f;
	return tRect;
}

static int __xgeSvgPatternTargetMaskCreate(xge_shape_ex pTargetShape, int bStroke, xge_shape_ex_matrix_t tItemTransform, xge_shape_ex* ppMaskShape)
{
	xge_shape_ex pMaskShape;
	xge_shape_ex_matrix_t tInverse;
	xge_shape_ex_matrix_t tRelative;
	int iRet;

	if ( (pTargetShape == NULL) || (ppMaskShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppMaskShape = NULL;
	if ( !__xgeSvgMatrixInvert(tItemTransform, &tInverse) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = xgeShapeExClone(pTargetShape, &pMaskShape);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tRelative = __xgeSvgMatrixMul(tInverse, pTargetShape->tTransform);
	xgeShapeExTransformSet(pMaskShape, &tRelative);
	if ( bStroke ) {
		xgeShapeExFillColor(pMaskShape, XGE_COLOR_RGBA(255, 255, 255, 0));
		xgeShapeExStrokeColor(pMaskShape, XGE_COLOR_RGBA(255, 255, 255, 255));
		pMaskShape->bStencilPaint = 1;
	} else {
		pMaskShape->bStencilPaint = 0;
	}
	*ppMaskShape = pMaskShape;
	return XGE_OK;
}

static int __xgeSvgPatternClipShapeToTarget(xge_shape_ex pShape, xge_shape_ex pTargetShape, int bStroke, xge_shape_ex_matrix_t tItemTransform)
{
	xge_shape_ex pMaskShape;
	int iRet;

	pMaskShape = NULL;
	iRet = __xgeSvgPatternTargetMaskCreate(pTargetShape, bStroke, tItemTransform, &pMaskShape);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeShapeExClipShapeAdd(pShape, pMaskShape);
	xgeShapeExDestroy(pMaskShape);
	return iRet;
}

static int __xgeSvgPatternItemMaskShapeCreate(xge_shape_ex pExistingMask, xge_shape_ex pTargetShape, int bStroke, xge_shape_ex_matrix_t tItemTransform, xge_shape_ex* ppMaskShape)
{
	xge_shape_ex pTargetMask;
	xge_shape_ex pCombined;
	int iRet;

	if ( ppMaskShape == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppMaskShape = NULL;
	pTargetMask = NULL;
	iRet = __xgeSvgPatternTargetMaskCreate(pTargetShape, bStroke, tItemTransform, &pTargetMask);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( pExistingMask == NULL ) {
		*ppMaskShape = pTargetMask;
		return XGE_OK;
	}
	iRet = xgeShapeExClone(pExistingMask, &pCombined);
	if ( iRet != XGE_OK ) {
		xgeShapeExDestroy(pTargetMask);
		return iRet;
	}
	iRet = xgeShapeExClipShapeAdd(pCombined, pTargetMask);
	xgeShapeExDestroy(pTargetMask);
	if ( iRet != XGE_OK ) {
		xgeShapeExDestroy(pCombined);
		return iRet;
	}
	*ppMaskShape = pCombined;
	return XGE_OK;
}

static int __xgeSvgAddPatternShapeTiles(xge_svg pSvg, const xge_svg_pattern_t* pPattern, xge_shape_ex pTargetShape, const xge_svg_style_t* pStyle, int iDefIndex, xge_rect_t tObjectBounds, xge_rect_t tTargetBounds, xge_rect_t tTileBase, float fPaintOpacity, int bStrokePattern)
{
	xge_svg_def_t* pDef;
	xge_rect_t tTargetClip;
	float fX;
	float fY;
	float fRight;
	float fBottom;
	int i;
	int iRet;

	if ( (tTileBase.fW <= 0.0f) || (tTileBase.fH <= 0.0f) ) {
		return XGE_OK;
	}
	pDef = &pSvg->pDefs[pPattern->iDefIndex];
	if ( (pDef->iShapeCount <= 0) && (pDef->iTextCount <= 0) && (pDef->iImageCount <= 0) && (pDef->iRasterCount <= 0) ) {
		return XGE_OK;
	}
	tTargetClip = tTargetBounds;
	if ( pTargetShape->bClipRect ) {
		tTargetClip = __xgeSvgRectIntersect(tTargetClip, pTargetShape->tClipRect);
	}
	if ( (tTargetClip.fW <= 0.0f) || (tTargetClip.fH <= 0.0f) ) {
		return XGE_OK;
	}
	fRight = tTargetClip.fX + tTargetClip.fW;
	fBottom = tTargetClip.fY + tTargetClip.fH;
	fY = __xgeSvgPatternFirstTile(tTileBase.fY, tTileBase.fH, tTargetClip.fY);
	while ( fY < fBottom ) {
		fX = __xgeSvgPatternFirstTile(tTileBase.fX, tTileBase.fW, tTargetClip.fX);
		while ( fX < fRight ) {
			xge_rect_t tTile;
			xge_rect_t tClip;
			xge_shape_ex_matrix_t tTileMatrix;
			xge_svg_style_t tMaskStyle;

			tTile.fX = fX;
			tTile.fY = fY;
			tTile.fW = tTileBase.fW;
			tTile.fH = tTileBase.fH;
			tClip = __xgeSvgRectIntersect(tTargetClip, tTile);
			if ( (tClip.fW > 0.0f) && (tClip.fH > 0.0f) ) {
				tTileMatrix = __xgeSvgPatternTileMatrix(pSvg, pPattern, tTile, tObjectBounds);
				tMaskStyle = *pStyle;
				tMaskStyle.sFillGradientId[0] = '\0';
				tMaskStyle.sStrokeGradientId[0] = '\0';
				for ( i = 0; i < pDef->iShapeCount; i++ ) {
					xge_shape_ex pClone;
					xge_shape_ex_matrix_t tShapeTransform;
					xge_rect_t tShapeClip;

					iRet = xgeShapeExClone(pDef->pShapes[i], &pClone);
					if ( iRet != XGE_OK ) {
						return iRet;
					}
					tShapeTransform = __xgeSvgMatrixMul(tTileMatrix, pClone->tTransform);
					if ( !__xgeSvgMatrixRectBoundsInverse(tShapeTransform, tClip, &tShapeClip) ) {
						xgeShapeExDestroy(pClone);
						continue;
					}
					xgeShapeExTransformSet(pClone, &tShapeTransform);
					xgeShapeExOpacity(pClone, pClone->fOpacity * pStyle->fOpacity * fPaintOpacity);
					if ( pClone->bClipRect ) {
						tShapeClip = __xgeSvgRectIntersect(tShapeClip, pClone->tClipRect);
					}
					xgeShapeExClipRectSet(pClone, tShapeClip);
					iRet = __xgeSvgPatternClipShapeToTarget(pClone, pTargetShape, bStrokePattern, tShapeTransform);
					if ( iRet != XGE_OK ) {
						xgeShapeExDestroy(pClone);
						return iRet;
					}
					iRet = __xgeSvgAddStyledShapeMasked(pSvg, pClone, &tMaskStyle, iDefIndex);
					if ( iRet != XGE_OK ) {
						return iRet;
					}
				}
				for ( i = 0; i < pDef->iTextCount; i++ ) {
					xge_svg_text_item_t tText;
					xge_rect_t tTextClip;
					xge_shape_ex pOwnedMaskShape;

					tText = pDef->pTexts[i];
					tText.tStyle.tTransform = __xgeSvgMatrixMul(tTileMatrix, tText.tStyle.tTransform);
					tText.tStyle.fOpacity *= pStyle->fOpacity * fPaintOpacity;
					if ( !pStyle->bVisible || !pStyle->bVisibility ) {
						tText.tStyle.bVisible = 0;
					}
					if ( !__xgeSvgMatrixRectBoundsInverse(tText.tStyle.tTransform, tClip, &tTextClip) ) {
						continue;
					}
					if ( tText.bClipRect ) {
						tTextClip = __xgeSvgRectIntersect(tTextClip, tText.tClipRect);
					}
					tText.bClipRect = 1;
					tText.tClipRect = tTextClip;
					pOwnedMaskShape = NULL;
					iRet = __xgeSvgPatternItemMaskShapeCreate(tText.pMaskShape, pTargetShape, bStrokePattern, tText.tStyle.tTransform, &pOwnedMaskShape);
					if ( iRet != XGE_OK ) {
						return iRet;
					}
					tText.pMaskShape = pOwnedMaskShape;
					iRet = __xgeSvgAddTextItemMasked(pSvg, &tText, iDefIndex);
					xgeShapeExDestroy(pOwnedMaskShape);
					if ( iRet != XGE_OK ) {
						return iRet;
					}
				}
				for ( i = 0; i < pDef->iImageCount; i++ ) {
					xge_svg_image_item_t tImage;
					xge_rect_t tImageClip;
					xge_shape_ex pOwnedMaskShape;

					tImage = pDef->pImages[i];
					tImage.tStyle.tTransform = __xgeSvgMatrixMul(tTileMatrix, tImage.tStyle.tTransform);
					tImage.tStyle.fOpacity *= pStyle->fOpacity * fPaintOpacity;
					if ( !pStyle->bVisible || !pStyle->bVisibility ) {
						tImage.tStyle.bVisible = 0;
					}
					if ( !__xgeSvgMatrixRectBoundsInverse(tImage.tStyle.tTransform, tClip, &tImageClip) ) {
						continue;
					}
					if ( tImage.bClipRect ) {
						tImageClip = __xgeSvgRectIntersect(tImageClip, tImage.tClipRect);
					}
					tImage.bClipRect = 1;
					tImage.tClipRect = tImageClip;
					pOwnedMaskShape = NULL;
					iRet = __xgeSvgPatternItemMaskShapeCreate(tImage.pMaskShape, pTargetShape, bStrokePattern, tImage.tStyle.tTransform, &pOwnedMaskShape);
					if ( iRet != XGE_OK ) {
						return iRet;
					}
					tImage.pMaskShape = pOwnedMaskShape;
					iRet = __xgeSvgAddImageItemMasked(pSvg, &tImage, iDefIndex);
					xgeShapeExDestroy(pOwnedMaskShape);
					if ( iRet != XGE_OK ) {
						return iRet;
					}
				}
				for ( i = 0; i < pDef->iRasterCount; i++ ) {
					xge_svg_raster_item_t tImage;
					xge_rect_t tImageClip;
					xge_shape_ex pOwnedMaskShape;

					tImage = pDef->pRasters[i];
					tImage.tStyle.tTransform = __xgeSvgMatrixMul(tTileMatrix, tImage.tStyle.tTransform);
					tImage.tStyle.fOpacity *= pStyle->fOpacity * fPaintOpacity;
					if ( !pStyle->bVisible || !pStyle->bVisibility ) {
						tImage.tStyle.bVisible = 0;
					}
					if ( !__xgeSvgMatrixRectBoundsInverse(tImage.tStyle.tTransform, tClip, &tImageClip) ) {
						continue;
					}
					if ( tImage.bClipRect ) {
						tImageClip = __xgeSvgRectIntersect(tImageClip, tImage.tClipRect);
					}
					tImage.bClipRect = 1;
					tImage.tClipRect = tImageClip;
					pOwnedMaskShape = NULL;
					iRet = __xgeSvgPatternItemMaskShapeCreate(tImage.pMaskShape, pTargetShape, bStrokePattern, tImage.tStyle.tTransform, &pOwnedMaskShape);
					if ( iRet != XGE_OK ) {
						return iRet;
					}
					tImage.pMaskShape = pOwnedMaskShape;
					iRet = __xgeSvgAddRasterItemMasked(pSvg, &tImage, iDefIndex);
					xgeShapeExDestroy(pOwnedMaskShape);
					if ( iRet != XGE_OK ) {
						return iRet;
					}
				}
			}
			fX += tTileBase.fW;
		}
		fY += tTileBase.fH;
	}
	return XGE_OK;
}

static int __xgeSvgAddPatternFill(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iDefIndex)
{
	xge_svg_pattern_t* pPattern;
	xge_rect_t tBounds;
	xge_rect_t tTileBase;
	int iRet;

	if ( (pStyle == NULL) || (pStyle->sFillGradientId[0] == '\0') ) {
		return XGE_OK;
	}
	if ( !pStyle->bVisible || !pStyle->bVisibility || ((pStyle->fOpacity * pStyle->fFillOpacity) <= 0.0f) ) {
		return XGE_OK;
	}
	pPattern = __xgeSvgPatternGetForPaint(pSvg, pStyle->sFillGradientId);
	if ( pPattern == NULL ) {
		return XGE_OK;
	}
	if ( (pSvg->pDefs[pPattern->iDefIndex].iShapeCount <= 0) &&
	     (pSvg->pDefs[pPattern->iDefIndex].iTextCount <= 0) &&
	     (pSvg->pDefs[pPattern->iDefIndex].iImageCount <= 0) &&
	     (pSvg->pDefs[pPattern->iDefIndex].iRasterCount <= 0) ) {
		return XGE_OK;
	}
	iRet = xgeShapeExGetBounds(pShape, 0.25f, &tBounds);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( (tBounds.fW <= 0.0f) || (tBounds.fH <= 0.0f) ) {
		xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(0, 0, 0, 0));
		return XGE_OK;
	}
	tTileBase = __xgeSvgPatternRectToUser(pPattern, tBounds);
	iRet = __xgeSvgAddPatternShapeTiles(pSvg, pPattern, pShape, pStyle, iDefIndex, tBounds, tBounds, tTileBase, pStyle->fFillOpacity, 0);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(0, 0, 0, 0));
	return XGE_OK;
}

static int __xgeSvgHasPatternStroke(xge_svg pSvg, const xge_svg_style_t* pStyle)
{
	xge_svg_pattern_t* pPattern;

	if ( (pStyle == NULL) || (pStyle->sStrokeGradientId[0] == '\0') ||
	     !pStyle->bVisible || !pStyle->bVisibility ||
	     (pStyle->fStrokeWidth <= 0.0f) || ((pStyle->fOpacity * pStyle->fStrokeOpacity) <= 0.0f) ) {
		return 0;
	}
	pPattern = __xgeSvgPatternGetForPaint(pSvg, pStyle->sStrokeGradientId);
	if ( pPattern == NULL ) {
		return 0;
	}
	if ( (pSvg->pDefs[pPattern->iDefIndex].iShapeCount <= 0) &&
	     (pSvg->pDefs[pPattern->iDefIndex].iTextCount <= 0) &&
	     (pSvg->pDefs[pPattern->iDefIndex].iImageCount <= 0) &&
	     (pSvg->pDefs[pPattern->iDefIndex].iRasterCount <= 0) ) {
		return 0;
	}
	return 1;
}

static int __xgeSvgAddPatternStroke(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iDefIndex)
{
	xge_svg_pattern_t* pPattern;
	xge_rect_t tBounds;
	xge_rect_t tStrokeBounds;
	xge_rect_t tTileBase;
	float fStrokeScale;
	float fMargin;
	int iRet;

	if ( !__xgeSvgHasPatternStroke(pSvg, pStyle) ) {
		return XGE_OK;
	}
	pPattern = __xgeSvgPatternGetForPaint(pSvg, pStyle->sStrokeGradientId);
	if ( pPattern == NULL ) {
		return XGE_OK;
	}
	iRet = xgeShapeExGetBounds(pShape, 0.25f, &tBounds);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( (tBounds.fW <= 0.0f) || (tBounds.fH <= 0.0f) ) {
		return XGE_OK;
	}
	if ( pShape->bStrokeNonScaling || (xgeShapeExMatrixStrokeScale(&fStrokeScale, &pShape->tTransform) != XGE_OK) ) {
		fStrokeScale = 1.0f;
	}
	fMargin = (pStyle->fStrokeWidth * fStrokeScale * 0.5f) + 1.0f;
	tStrokeBounds = __xgeSvgRectInflate(tBounds, fMargin);
	tTileBase = __xgeSvgPatternRectToUser(pPattern, tBounds);
	return __xgeSvgAddPatternShapeTiles(pSvg, pPattern, pShape, pStyle, iDefIndex, tBounds, tStrokeBounds, tTileBase, pStyle->fStrokeOpacity, 1);
}

static int __xgeSvgAddStyledShape(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iDefIndex)
{
	xge_svg_clip_path_t* pClip;
	xge_svg_style_t tFilterOriginalStyle;
	xge_shape_ex pStrokePatternShape;
	int bStrokePattern;
	int i;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pStyle != NULL) && (pStyle->sFilterId[0] != '\0') ) {
		xge_rect_t tBounds;
		xge_rect_t tFilterRegion;
		float fOffsetX;
		float fOffsetY;
		uint32_t iShadowColor;
		float fShadowOpacity;
		float fShadowStdDeviation;
		int iFilterRegion;
		int bHasFilterBounds;

		memset(&tBounds, 0, sizeof(tBounds));
		memset(&tFilterRegion, 0, sizeof(tFilterRegion));
		iFilterRegion = 0;
		bHasFilterBounds = 0;
		if ( xgeShapeExGetBounds(pShape, 0.0f, &tBounds) == XGE_OK ) {
			bHasFilterBounds = 1;
			iFilterRegion = __xgeSvgFilterRegionForBounds(pSvg, pStyle->sFilterId, tBounds, &tFilterRegion);
			if ( iFilterRegion < 0 ) {
				xgeShapeExDestroy(pShape);
				return XGE_OK;
			}
		}

		if ( bHasFilterBounds &&
		     __xgeSvgFilterDropShadowForBounds(pSvg, pStyle->sFilterId, tBounds, &fOffsetX, &fOffsetY, &iShadowColor, &fShadowOpacity, &fShadowStdDeviation) ) {
			static const float arrTaps[9][3] = {
				{ 0.0f, 0.0f, 0.28f },
				{ -1.0f, 0.0f, 0.12f }, { 1.0f, 0.0f, 0.12f }, { 0.0f, -1.0f, 0.12f }, { 0.0f, 1.0f, 0.12f },
				{ -0.70710678f, -0.70710678f, 0.06f }, { 0.70710678f, -0.70710678f, 0.06f },
				{ -0.70710678f, 0.70710678f, 0.06f }, { 0.70710678f, 0.70710678f, 0.06f }
			};
			xge_svg_style_t tShadowStyle;
			tShadowStyle = *pStyle;
			tShadowStyle.sFilterId[0] = '\0';
			tShadowStyle.sFillGradientId[0] = '\0';
			tShadowStyle.sStrokeGradientId[0] = '\0';
			{
				int iTapCount = fShadowStdDeviation > 0.0001f ? 9 : 1;
				int iTap;

				for ( iTap = 0; iTap < iTapCount; iTap++ ) {
					xge_shape_ex pShadow;
					xge_shape_ex_matrix_t tOffset;
					xge_shape_ex_matrix_t tTransform;
					float fTapOpacity;
					float fTapX;
					float fTapY;

					fTapOpacity = fShadowOpacity * (iTapCount > 1 ? arrTaps[iTap][2] : 1.0f);
					if ( fTapOpacity <= 0.0f ) {
						continue;
					}
					fTapX = fOffsetX + (iTapCount > 1 ? arrTaps[iTap][0] * fShadowStdDeviation : 0.0f);
					fTapY = fOffsetY + (iTapCount > 1 ? arrTaps[iTap][1] * fShadowStdDeviation : 0.0f);
					iRet = xgeShapeExClone(pShape, &pShadow);
					if ( iRet != XGE_OK ) {
						xgeShapeExDestroy(pShape);
						return iRet;
					}
					tOffset = __xgeSvgMatrixTranslate(fTapX, fTapY);
					tTransform = __xgeSvgMatrixMul(tOffset, pShadow->tTransform);
					iRet = xgeShapeExTransformSet(pShadow, &tTransform);
					if ( iRet != XGE_OK ) {
						xgeShapeExDestroy(pShadow);
						xgeShapeExDestroy(pShape);
						return iRet;
					}
					__xgeSvgApplyDropShadowPaintToShape(pShadow, pStyle, iShadowColor, fTapOpacity);
					if ( (iFilterRegion > 0) && !__xgeSvgShapeClipRectIntersect(pShadow, tFilterRegion) ) {
						xgeShapeExDestroy(pShadow);
						continue;
					}
					iRet = __xgeSvgAddStyledShape(pSvg, pShadow, &tShadowStyle, iDefIndex);
					if ( iRet != XGE_OK ) {
						xgeShapeExDestroy(pShape);
						return iRet;
					}
				}
			}
			tFilterOriginalStyle = *pStyle;
			tFilterOriginalStyle.sFilterId[0] = '\0';
			if ( (iFilterRegion > 0) && !__xgeSvgShapeClipRectIntersect(pShape, tFilterRegion) ) {
				xgeShapeExDestroy(pShape);
				return XGE_OK;
			}
			pStyle = &tFilterOriginalStyle;
		}
		if ( (pStyle->sFilterId[0] != '\0') &&
		     bHasFilterBounds &&
		     __xgeSvgFilterOffsetForBounds(pSvg, pStyle->sFilterId, tBounds, &fOffsetX, &fOffsetY) ) {
			xge_svg_style_t tFilterStyle;
			xge_shape_ex_matrix_t tOffset;
			xge_shape_ex_matrix_t tTransform;

			tFilterStyle = *pStyle;
			tFilterStyle.sFilterId[0] = '\0';
			tOffset = __xgeSvgMatrixTranslate(fOffsetX, fOffsetY);
			tTransform = __xgeSvgMatrixMul(tOffset, pShape->tTransform);
			iRet = xgeShapeExTransformSet(pShape, &tTransform);
			if ( iRet != XGE_OK ) {
				xgeShapeExDestroy(pShape);
				return iRet;
			}
			if ( (iFilterRegion > 0) && !__xgeSvgShapeClipRectIntersect(pShape, tFilterRegion) ) {
				xgeShapeExDestroy(pShape);
				return XGE_OK;
			}
			return __xgeSvgAddStyledShape(pSvg, pShape, &tFilterStyle, iDefIndex);
		}
	}
	pClip = (pStyle != NULL) ? __xgeSvgClipPathGetById(pSvg, pStyle->sClipId) : NULL;
	if ( (pClip != NULL) && (pClip->iRectCount > 1) ) {
		for ( i = 0; i < pClip->iRectCount; i++ ) {
			xge_shape_ex pClone;
			xge_rect_t tClip;

			iRet = __xgeSvgClipPathResolveRect(pClip, i, pShape, &tClip);
			if ( iRet != XGE_OK ) {
				xgeShapeExDestroy(pShape);
				return iRet;
			}
			if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
				continue;
			}
			iRet = xgeShapeExClone(pShape, &pClone);
			if ( iRet != XGE_OK ) {
				xgeShapeExDestroy(pShape);
				return iRet;
			}
			if ( pClone->bClipRect ) {
				tClip = __xgeSvgRectIntersect(pClone->tClipRect, tClip);
			}
			xgeShapeExClipRectSet(pClone, tClip);
			iRet = __xgeSvgApplyClipShapes(pClip, pClone);
			if ( iRet != XGE_OK ) {
				xgeShapeExDestroy(pShape);
				xgeShapeExDestroy(pClone);
				return iRet;
			}
			iRet = __xgeSvgAddStyledShapeMasked(pSvg, pClone, pStyle, iDefIndex);
			if ( iRet != XGE_OK ) {
				xgeShapeExDestroy(pShape);
				return iRet;
			}
		}
		xgeShapeExDestroy(pShape);
		return XGE_OK;
	}
	if ( pClip != NULL ) {
		iRet = __xgeSvgApplyClipPath(pSvg, pShape, pStyle->sClipId);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
	}
	pStrokePatternShape = NULL;
	bStrokePattern = __xgeSvgHasPatternStroke(pSvg, pStyle);
	if ( bStrokePattern ) {
		iRet = xgeShapeExClone(pShape, &pStrokePatternShape);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
	}
	if ( bStrokePattern && pStyle->bStrokeFirst ) {
		iRet = __xgeSvgAddPatternStroke(pSvg, pStrokePatternShape, pStyle, iDefIndex);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pStrokePatternShape);
			xgeShapeExDestroy(pShape);
			return iRet;
		}
	}
	iRet = __xgeSvgAddPatternFill(pSvg, pShape, pStyle, iDefIndex);
	if ( iRet != XGE_OK ) {
		xgeShapeExDestroy(pStrokePatternShape);
		xgeShapeExDestroy(pShape);
		return iRet;
	}
	if ( bStrokePattern ) {
		xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(0, 0, 0, 0));
	}
	iRet = __xgeSvgAddStyledShapeMasked(pSvg, pShape, pStyle, iDefIndex);
	if ( iRet != XGE_OK ) {
		xgeShapeExDestroy(pStrokePatternShape);
		return iRet;
	}
	if ( bStrokePattern && !pStyle->bStrokeFirst ) {
		iRet = __xgeSvgAddPatternStroke(pSvg, pStrokePatternShape, pStyle, iDefIndex);
	}
	xgeShapeExDestroy(pStrokePatternShape);
	return iRet;
}

static int __xgeSvgMainIdDefIndex(xge_svg pSvg, const char* pTag, const char* pTagEnd, int bInDefs, int iCurrentDef, int* pDefIndex)
{
	char sId[XGE_SVG_ATTR_MAX];

	if ( pDefIndex != NULL ) {
		*pDefIndex = -1;
	}
	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pDefIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( bInDefs || (iCurrentDef >= 0) ) {
		return XGE_OK;
	}
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "id", sId, sizeof(sId)) || (sId[0] == '\0') ) {
		return XGE_OK;
	}
	return __xgeSvgDefGetOrCreate(pSvg, sId, pDefIndex);
}

static int __xgeSvgAddStyledShapeWithMainIds(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iDefIndex, int iMainIdDefIndex, int iExtraMainIdDefIndex);

static int __xgeSvgAddStyledShapeWithMainId(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iDefIndex, int iMainIdDefIndex)
{
	return __xgeSvgAddStyledShapeWithMainIds(pSvg, pShape, pStyle, iDefIndex, iMainIdDefIndex, -1);
}

static int __xgeSvgAddStyledShapeWithMainIds(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iDefIndex, int iMainIdDefIndex, int iExtraMainIdDefIndex)
{
	xge_shape_ex pMirror;
	xge_shape_ex pExtraMirror;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pMirror = NULL;
	pExtraMirror = NULL;
	if ( (iDefIndex < 0) && (iMainIdDefIndex >= 0) ) {
		iRet = xgeShapeExClone(pShape, &pMirror);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
	}
	if ( (iDefIndex < 0) && (iExtraMainIdDefIndex >= 0) && (iExtraMainIdDefIndex != iMainIdDefIndex) ) {
		iRet = xgeShapeExClone(pShape, &pExtraMirror);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pMirror);
			xgeShapeExDestroy(pShape);
			return iRet;
		}
	}
	iRet = __xgeSvgAddStyledShape(pSvg, pShape, pStyle, iDefIndex);
	if ( iRet == XGE_OK && pMirror != NULL ) {
		iRet = __xgeSvgAddStyledShape(pSvg, pMirror, pStyle, iMainIdDefIndex);
		pMirror = NULL;
	}
	if ( iRet == XGE_OK && pExtraMirror != NULL ) {
		iRet = __xgeSvgAddStyledShape(pSvg, pExtraMirror, pStyle, iExtraMainIdDefIndex);
		pExtraMirror = NULL;
	}
	xgeShapeExDestroy(pMirror);
	xgeShapeExDestroy(pExtraMirror);
	return iRet;
}

static int __xgeSvgStyleHasMarkerPaint(const xge_svg_style_t* pStyle)
{
	return (pStyle != NULL) &&
		((pStyle->sMarkerStartId[0] != '\0') ||
		 (pStyle->sMarkerMidId[0] != '\0') ||
		 (pStyle->sMarkerEndId[0] != '\0'));
}

static void __xgeSvgStylePaintPass(const xge_svg_style_t* pStyle, int iPaint, xge_svg_style_t* pPassStyle)
{
	if ( (pStyle == NULL) || (pPassStyle == NULL) ) {
		return;
	}
	*pPassStyle = *pStyle;
	if ( iPaint == XGE_SVG_PAINT_FILL ) {
		pPassStyle->iStrokeColor = XGE_COLOR_RGBA(0, 0, 0, 0);
		pPassStyle->sStrokeGradientId[0] = '\0';
		pPassStyle->fStrokeOpacity = 0.0f;
		pPassStyle->fStrokeWidth = 0.0f;
		pPassStyle->iDashCount = 0;
	} else if ( iPaint == XGE_SVG_PAINT_STROKE ) {
		pPassStyle->iFillColor = XGE_COLOR_RGBA(0, 0, 0, 0);
		pPassStyle->sFillGradientId[0] = '\0';
		pPassStyle->fFillOpacity = 0.0f;
	}
	__xgeSvgPaintOrderDefault(pPassStyle->iPaintOrder);
	__xgeSvgStyleUpdatePaintOrderDerived(pPassStyle);
}

static int __xgeSvgAddShapePaintPassWithMainIds(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iDefIndex, int iMainIdDefIndex, int iExtraMainIdDefIndex, int iPaint);

static int __xgeSvgAddShapePaintPassWithMainId(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iDefIndex, int iMainIdDefIndex, int iPaint)
{
	return __xgeSvgAddShapePaintPassWithMainIds(pSvg, pShape, pStyle, iDefIndex, iMainIdDefIndex, -1, iPaint);
}

static int __xgeSvgAddShapePaintPassWithMainIds(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iDefIndex, int iMainIdDefIndex, int iExtraMainIdDefIndex, int iPaint)
{
	xge_svg_style_t tPassStyle;
	xge_shape_ex pPass;
	int iRet;

	if ( (pShape == NULL) || (pStyle == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iPaint == XGE_SVG_PAINT_FILL) && !__xgeSvgStyleHasPaintAlpha(pStyle, 0) ) {
		return XGE_OK;
	}
	if ( (iPaint == XGE_SVG_PAINT_STROKE) && !__xgeSvgStyleHasPaintAlpha(pStyle, 1) ) {
		return XGE_OK;
	}
	pPass = NULL;
	iRet = xgeShapeExClone(pShape, &pPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	__xgeSvgStylePaintPass(pStyle, iPaint, &tPassStyle);
	if ( iPaint == XGE_SVG_PAINT_FILL ) {
		xgeShapeExStrokeColor(pPass, XGE_COLOR_RGBA(0, 0, 0, 0));
		xgeShapeExStrokeWidth(pPass, 0.0f);
	} else if ( iPaint == XGE_SVG_PAINT_STROKE ) {
		xgeShapeExFillColor(pPass, XGE_COLOR_RGBA(0, 0, 0, 0));
	}
	return __xgeSvgAddStyledShapeWithMainIds(pSvg, pPass, &tPassStyle, iDefIndex, iMainIdDefIndex, iExtraMainIdDefIndex);
}

static int __xgeSvgApplyMarkersWithMainIds(xge_svg pSvg, const xge_svg_style_t* pStyle, int iDefIndex, int iMainIdDefIndex, int iExtraMainIdDefIndex, xge_svg_marker_apply_proc pApplyMarkers, void* pUser);

static int __xgeSvgApplyMarkersWithMainId(xge_svg pSvg, const xge_svg_style_t* pStyle, int iDefIndex, int iMainIdDefIndex, xge_svg_marker_apply_proc pApplyMarkers, void* pUser)
{
	return __xgeSvgApplyMarkersWithMainIds(pSvg, pStyle, iDefIndex, iMainIdDefIndex, -1, pApplyMarkers, pUser);
}

static int __xgeSvgApplyMarkersWithMainIds(xge_svg pSvg, const xge_svg_style_t* pStyle, int iDefIndex, int iMainIdDefIndex, int iExtraMainIdDefIndex, xge_svg_marker_apply_proc pApplyMarkers, void* pUser)
{
	int iRet;

	if ( pApplyMarkers == NULL ) {
		return XGE_OK;
	}
	iRet = pApplyMarkers(pSvg, pStyle, iDefIndex, pUser);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( (iDefIndex < 0) && (iMainIdDefIndex >= 0) ) {
		iRet = pApplyMarkers(pSvg, pStyle, iMainIdDefIndex, pUser);
	}
	if ( (iRet == XGE_OK) && (iDefIndex < 0) && (iExtraMainIdDefIndex >= 0) && (iExtraMainIdDefIndex != iMainIdDefIndex) ) {
		iRet = pApplyMarkers(pSvg, pStyle, iExtraMainIdDefIndex, pUser);
	}
	return iRet;
}

static int __xgeSvgAddMarkedShapeWithMainIds(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iDefIndex, int iMainIdDefIndex, int iExtraMainIdDefIndex, xge_svg_marker_apply_proc pApplyMarkers, void* pUser);

static int __xgeSvgAddMarkedShapeWithMainId(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iDefIndex, int iMainIdDefIndex, xge_svg_marker_apply_proc pApplyMarkers, void* pUser)
{
	return __xgeSvgAddMarkedShapeWithMainIds(pSvg, pShape, pStyle, iDefIndex, iMainIdDefIndex, -1, pApplyMarkers, pUser);
}

static int __xgeSvgAddMarkedShapeWithMainIds(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iDefIndex, int iMainIdDefIndex, int iExtraMainIdDefIndex, xge_svg_marker_apply_proc pApplyMarkers, void* pUser)
{
	int iMarkerOrder;
	int iRet;
	int i;

	if ( !__xgeSvgValid(pSvg) || (pShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeSvgStyleHasMarkerPaint(pStyle) || (pApplyMarkers == NULL) ) {
		return __xgeSvgAddStyledShapeWithMainIds(pSvg, pShape, pStyle, iDefIndex, iMainIdDefIndex, iExtraMainIdDefIndex);
	}
	iMarkerOrder = __xgeSvgPaintOrderIndex(pStyle->iPaintOrder, XGE_SVG_PAINT_MARKERS);
	if ( iMarkerOrder == (XGE_SVG_PAINT_COMPONENT_COUNT - 1) ) {
		xge_shape_ex pPaint;

		pPaint = NULL;
		iRet = xgeShapeExClone(pShape, &pPaint);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		iRet = __xgeSvgAddStyledShapeWithMainIds(pSvg, pPaint, pStyle, iDefIndex, iMainIdDefIndex, iExtraMainIdDefIndex);
		if ( iRet == XGE_OK ) {
			iRet = __xgeSvgApplyMarkersWithMainIds(pSvg, pStyle, iDefIndex, iMainIdDefIndex, iExtraMainIdDefIndex, pApplyMarkers, pUser);
		}
		xgeShapeExDestroy(pShape);
		return iRet;
	}
	for ( i = 0; i < XGE_SVG_PAINT_COMPONENT_COUNT; i++ ) {
		int iPaint = pStyle->iPaintOrder[i];

		if ( iPaint == XGE_SVG_PAINT_MARKERS ) {
			iRet = __xgeSvgApplyMarkersWithMainIds(pSvg, pStyle, iDefIndex, iMainIdDefIndex, iExtraMainIdDefIndex, pApplyMarkers, pUser);
		} else {
			iRet = __xgeSvgAddShapePaintPassWithMainIds(pSvg, pShape, pStyle, iDefIndex, iMainIdDefIndex, iExtraMainIdDefIndex, iPaint);
		}
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
	}
	xgeShapeExDestroy(pShape);
	return XGE_OK;
}

static float __xgeSvgMarkerSegmentAngle(xge_vec2_t tA, xge_vec2_t tB)
{
	return atan2f(tB.fY - tA.fY, tB.fX - tA.fX) * (180.0f / 3.14159265358979323846f);
}

static float __xgeSvgMarkerMidAngle(xge_vec2_t tPrev, xge_vec2_t tPoint, xge_vec2_t tNext)
{
	float fInX = tPoint.fX - tPrev.fX;
	float fInY = tPoint.fY - tPrev.fY;
	float fOutX = tNext.fX - tPoint.fX;
	float fOutY = tNext.fY - tPoint.fY;
	float fInLen = hypotf(fInX, fInY);
	float fOutLen = hypotf(fOutX, fOutY);
	float fX;
	float fY;

	if ( fInLen > 0.000001f ) {
		fInX /= fInLen;
		fInY /= fInLen;
	}
	if ( fOutLen > 0.000001f ) {
		fOutX /= fOutLen;
		fOutY /= fOutLen;
	}
	fX = fInX + fOutX;
	fY = fInY + fOutY;
	if ( hypotf(fX, fY) <= 0.000001f ) {
		if ( fOutLen > 0.000001f ) {
			fX = fOutX;
			fY = fOutY;
		} else {
			fX = fInX;
			fY = fInY;
		}
	}
	return atan2f(fY, fX) * (180.0f / 3.14159265358979323846f);
}

static int __xgeSvgApplyMarkerInstance(xge_svg pSvg, const char* sMarkerId, const xge_svg_style_t* pStyle, int iTargetDef, xge_vec2_t tPoint, float fAutoAngle, int iPosition)
{
	xge_svg_marker_t* pMarker;
	xge_svg_def_t* pDef;
	xge_shape_ex_matrix_t tMatrix;
	xge_shape_ex_matrix_t tViewportMatrix;
	xge_vec2_t tRefPoint;
	float fOrient;
	float fUnitsScale;
	int iMarkerIndex;
	int i;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (sMarkerId == NULL) || (sMarkerId[0] == '\0') || (pStyle == NULL) ) {
		return XGE_OK;
	}
	iMarkerIndex = __xgeSvgMarkerFind(pSvg, sMarkerId);
	if ( iMarkerIndex < 0 ) {
		return XGE_OK;
	}
	pMarker = &pSvg->pMarkers[iMarkerIndex];
	if ( (pMarker->iDefIndex < 0) || (pMarker->iDefIndex >= pSvg->iDefCount) || (pMarker->fWidth <= 0.0f) || (pMarker->fHeight <= 0.0f) ) {
		return XGE_OK;
	}
	pDef = &pSvg->pDefs[pMarker->iDefIndex];
	if ( (pDef->iShapeCount <= 0) && (pDef->iTextCount <= 0) && (pDef->iImageCount <= 0) && (pDef->iRasterCount <= 0) ) {
		return XGE_OK;
	}
	fUnitsScale = (pMarker->iUnits == XGE_SVG_MARKER_UNITS_STROKE_WIDTH) ? pStyle->fStrokeWidth : 1.0f;
	if ( fUnitsScale <= 0.000001f ) {
		return XGE_OK;
	}
	tViewportMatrix = __xgeSvgMatrixIdentity();
	if ( pDef->bHasViewBox ) {
		xge_rect_t tViewport;

		tViewport.fX = 0.0f;
		tViewport.fY = 0.0f;
		tViewport.fW = pMarker->fWidth;
		tViewport.fH = pMarker->fHeight;
		tViewport = __xgeSvgAspectViewport(pDef->tViewBox, tViewport, pDef->iAspectAlignX, pDef->iAspectAlignY, pDef->iAspectMeetOrSlice);
		tViewportMatrix = __xgeSvgViewBoxMatrix(pDef->tViewBox, tViewport);
	}
	tRefPoint.fX = pMarker->fRefX;
	tRefPoint.fY = pMarker->fRefY;
	tRefPoint = __xgeSvgMatrixPoint(tViewportMatrix, tRefPoint);
	fOrient = pMarker->bOrientAuto ? fAutoAngle : pMarker->fOrientDegrees;
	if ( pMarker->bOrientAutoStartReverse && (iPosition == XGE_SVG_MARKER_POSITION_START) ) {
		fOrient += 180.0f;
	}
	tMatrix = __xgeSvgMatrixTranslate(tPoint.fX, tPoint.fY);
	tMatrix = __xgeSvgMatrixMul(tMatrix, __xgeSvgMatrixRotate(fOrient));
	if ( pMarker->iUnits == XGE_SVG_MARKER_UNITS_STROKE_WIDTH ) {
		tMatrix = __xgeSvgMatrixMul(tMatrix, __xgeSvgMatrixScale(fUnitsScale, fUnitsScale));
	}
	tMatrix = __xgeSvgMatrixMul(tMatrix, __xgeSvgMatrixTranslate(-tRefPoint.fX, -tRefPoint.fY));
	tMatrix = __xgeSvgMatrixMul(tMatrix, tViewportMatrix);
	tMatrix = __xgeSvgMatrixMul(pStyle->tTransform, tMatrix);
	for ( i = 0; i < pDef->iShapeCount; i++ ) {
		xge_shape_ex pClone;
		xge_shape_ex_matrix_t tShapeTransform;

		iRet = xgeShapeExClone(pDef->pShapes[i], &pClone);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		tShapeTransform = __xgeSvgMatrixMul(tMatrix, pClone->tTransform);
		xgeShapeExTransformSet(pClone, &tShapeTransform);
		xgeShapeExOpacity(pClone, pClone->fOpacity * pStyle->fOpacity);
		if ( !pStyle->bVisible || !pStyle->bVisibility ) {
			xgeShapeExVisible(pClone, 0);
		}
		iRet = __xgeSvgAddStyledShape(pSvg, pClone, pStyle, iTargetDef);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	for ( i = 0; i < pDef->iTextCount; i++ ) {
		xge_svg_text_item_t tText;

		tText = pDef->pTexts[i];
		tText.tStyle.tTransform = __xgeSvgMatrixMul(tMatrix, tText.tStyle.tTransform);
		tText.tStyle.fOpacity *= pStyle->fOpacity;
		if ( !pStyle->bVisible || !pStyle->bVisibility ) {
			tText.tStyle.bVisible = 0;
		}
		iRet = __xgeSvgAddTextItemMasked(pSvg, &tText, iTargetDef);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	for ( i = 0; i < pDef->iImageCount; i++ ) {
		xge_svg_image_item_t tImage;

		tImage = pDef->pImages[i];
		tImage.tStyle.tTransform = __xgeSvgMatrixMul(tMatrix, tImage.tStyle.tTransform);
		tImage.tStyle.fOpacity *= pStyle->fOpacity;
		if ( !pStyle->bVisible || !pStyle->bVisibility ) {
			tImage.tStyle.bVisible = 0;
		}
		iRet = __xgeSvgAddImageItemMasked(pSvg, &tImage, iTargetDef);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	for ( i = 0; i < pDef->iRasterCount; i++ ) {
		xge_svg_raster_item_t tImage;

		tImage = pDef->pRasters[i];
		tImage.tStyle.tTransform = __xgeSvgMatrixMul(tMatrix, tImage.tStyle.tTransform);
		tImage.tStyle.fOpacity *= pStyle->fOpacity;
		if ( !pStyle->bVisible || !pStyle->bVisibility ) {
			tImage.tStyle.bVisible = 0;
		}
		iRet = __xgeSvgAddRasterItemMasked(pSvg, &tImage, iTargetDef);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeSvgApplyLineMarkers(xge_svg pSvg, const xge_svg_style_t* pStyle, int iTargetDef, xge_vec2_t tA, xge_vec2_t tB)
{
	float fAngle;
	int iRet;

	if ( (pStyle == NULL) || ((pStyle->sMarkerStartId[0] == '\0') && (pStyle->sMarkerEndId[0] == '\0')) ) {
		return XGE_OK;
	}
	fAngle = __xgeSvgMarkerSegmentAngle(tA, tB);
	iRet = __xgeSvgApplyMarkerInstance(pSvg, pStyle->sMarkerStartId, pStyle, iTargetDef, tA, fAngle, XGE_SVG_MARKER_POSITION_START);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	return __xgeSvgApplyMarkerInstance(pSvg, pStyle->sMarkerEndId, pStyle, iTargetDef, tB, fAngle, XGE_SVG_MARKER_POSITION_END);
}

static int __xgeSvgApplyPointListMarkers(xge_svg pSvg, const xge_svg_style_t* pStyle, int iTargetDef, const xge_vec2_t* pPoints, int iCount, int bClosed)
{
	int i;
	int iRet;

	if ( (pStyle == NULL) || (pPoints == NULL) || (iCount <= 0) ) {
		return XGE_OK;
	}
	if ( iCount == 1 ) {
		return __xgeSvgApplyMarkerInstance(pSvg, pStyle->sMarkerStartId, pStyle, iTargetDef, pPoints[0], 0.0f, XGE_SVG_MARKER_POSITION_START);
	}
	iRet = __xgeSvgApplyMarkerInstance(pSvg, pStyle->sMarkerStartId, pStyle, iTargetDef, pPoints[0], __xgeSvgMarkerSegmentAngle(pPoints[0], pPoints[1]), XGE_SVG_MARKER_POSITION_START);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	for ( i = 1; i < iCount - 1; i++ ) {
		iRet = __xgeSvgApplyMarkerInstance(pSvg, pStyle->sMarkerMidId, pStyle, iTargetDef, pPoints[i], __xgeSvgMarkerMidAngle(pPoints[i - 1], pPoints[i], pPoints[i + 1]), XGE_SVG_MARKER_POSITION_MID);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	if ( bClosed ) {
		iRet = __xgeSvgApplyMarkerInstance(pSvg, pStyle->sMarkerMidId, pStyle, iTargetDef, pPoints[iCount - 1], __xgeSvgMarkerMidAngle(pPoints[iCount - 2], pPoints[iCount - 1], pPoints[0]), XGE_SVG_MARKER_POSITION_MID);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		return __xgeSvgApplyMarkerInstance(pSvg, pStyle->sMarkerEndId, pStyle, iTargetDef, pPoints[0], __xgeSvgMarkerSegmentAngle(pPoints[iCount - 1], pPoints[0]), XGE_SVG_MARKER_POSITION_END);
	}
	return __xgeSvgApplyMarkerInstance(pSvg, pStyle->sMarkerEndId, pStyle, iTargetDef, pPoints[iCount - 1], __xgeSvgMarkerSegmentAngle(pPoints[iCount - 2], pPoints[iCount - 1]), XGE_SVG_MARKER_POSITION_END);
}

typedef struct xge_svg_marker_vertex_t {
	xge_vec2_t tPoint;
	float fInAngle;
	float fOutAngle;
	int bHasInAngle;
	int bHasOutAngle;
} xge_svg_marker_vertex_t;

static float __xgeSvgMarkerAngleFromDelta(float fDX, float fDY, float fFallback)
{
	if ( hypotf(fDX, fDY) <= 0.000001f ) {
		return fFallback;
	}
	return atan2f(fDY, fDX) * (180.0f / 3.14159265358979323846f);
}

static float __xgeSvgMarkerAngleBetween(xge_vec2_t tA, xge_vec2_t tB, float fFallback)
{
	return __xgeSvgMarkerAngleFromDelta(tB.fX - tA.fX, tB.fY - tA.fY, fFallback);
}

static float __xgeSvgMarkerCubicStartAngle(xge_vec2_t tP0, xge_vec2_t tP1, xge_vec2_t tP2, xge_vec2_t tP3, float fFallback)
{
	float fAngle = __xgeSvgMarkerAngleBetween(tP0, tP1, fFallback);
	if ( hypotf(tP1.fX - tP0.fX, tP1.fY - tP0.fY) > 0.000001f ) return fAngle;
	fAngle = __xgeSvgMarkerAngleBetween(tP0, tP2, fFallback);
	if ( hypotf(tP2.fX - tP0.fX, tP2.fY - tP0.fY) > 0.000001f ) return fAngle;
	return __xgeSvgMarkerAngleBetween(tP0, tP3, fFallback);
}

static float __xgeSvgMarkerCubicEndAngle(xge_vec2_t tP0, xge_vec2_t tP1, xge_vec2_t tP2, xge_vec2_t tP3, float fFallback)
{
	float fAngle = __xgeSvgMarkerAngleBetween(tP2, tP3, fFallback);
	if ( hypotf(tP3.fX - tP2.fX, tP3.fY - tP2.fY) > 0.000001f ) return fAngle;
	fAngle = __xgeSvgMarkerAngleBetween(tP1, tP3, fFallback);
	if ( hypotf(tP3.fX - tP1.fX, tP3.fY - tP1.fY) > 0.000001f ) return fAngle;
	return __xgeSvgMarkerAngleBetween(tP0, tP3, fFallback);
}

static float __xgeSvgMarkerAverageAngles(float fInAngle, int bHasInAngle, float fOutAngle, int bHasOutAngle)
{
	float fX;
	float fY;

	if ( !bHasInAngle && !bHasOutAngle ) {
		return 0.0f;
	}
	if ( !bHasInAngle ) {
		return fOutAngle;
	}
	if ( !bHasOutAngle ) {
		return fInAngle;
	}
	fX = cosf(fInAngle * (3.14159265358979323846f / 180.0f)) + cosf(fOutAngle * (3.14159265358979323846f / 180.0f));
	fY = sinf(fInAngle * (3.14159265358979323846f / 180.0f)) + sinf(fOutAngle * (3.14159265358979323846f / 180.0f));
	if ( hypotf(fX, fY) <= 0.000001f ) {
		return fOutAngle;
	}
	return atan2f(fY, fX) * (180.0f / 3.14159265358979323846f);
}

static int __xgeSvgMarkerVertexAdd(xge_svg_marker_vertex_t** ppVertices, int* pCount, int* pCapacity, xge_vec2_t tPoint)
{
	xge_svg_marker_vertex_t* pVertices;
	int iCapacity;

	if ( (ppVertices == NULL) || (pCount == NULL) || (pCapacity == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( *pCount >= *pCapacity ) {
		iCapacity = *pCapacity > 0 ? *pCapacity * 2 : 8;
		pVertices = (xge_svg_marker_vertex_t*)xrtRealloc(*ppVertices, (size_t)iCapacity * sizeof(*pVertices));
		if ( pVertices == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		*ppVertices = pVertices;
		*pCapacity = iCapacity;
	}
	memset(&(*ppVertices)[*pCount], 0, sizeof((*ppVertices)[*pCount]));
	(*ppVertices)[*pCount].tPoint = tPoint;
	(*pCount)++;
	return XGE_OK;
}

static int __xgeSvgApplyPathMarkerContour(xge_svg pSvg, const xge_svg_style_t* pStyle, int iTargetDef, const xge_svg_marker_vertex_t* pVertices, int iCount, int bClosed)
{
	float fAngle;
	int i;
	int iRet;

	if ( (pVertices == NULL) || (iCount <= 0) || (pStyle == NULL) ) {
		return XGE_OK;
	}
	if ( iCount == 1 ) {
		return __xgeSvgApplyMarkerInstance(pSvg, pStyle->sMarkerStartId, pStyle, iTargetDef, pVertices[0].tPoint,
			__xgeSvgMarkerAverageAngles(pVertices[0].fInAngle, pVertices[0].bHasInAngle, pVertices[0].fOutAngle, pVertices[0].bHasOutAngle),
			XGE_SVG_MARKER_POSITION_START);
	}
	fAngle = pVertices[0].bHasOutAngle ? pVertices[0].fOutAngle : pVertices[0].fInAngle;
	iRet = __xgeSvgApplyMarkerInstance(pSvg, pStyle->sMarkerStartId, pStyle, iTargetDef, pVertices[0].tPoint, fAngle, XGE_SVG_MARKER_POSITION_START);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	for ( i = 1; i < iCount - 1; i++ ) {
		fAngle = __xgeSvgMarkerAverageAngles(pVertices[i].fInAngle, pVertices[i].bHasInAngle, pVertices[i].fOutAngle, pVertices[i].bHasOutAngle);
		iRet = __xgeSvgApplyMarkerInstance(pSvg, pStyle->sMarkerMidId, pStyle, iTargetDef, pVertices[i].tPoint, fAngle, XGE_SVG_MARKER_POSITION_MID);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	if ( bClosed ) {
		fAngle = __xgeSvgMarkerAverageAngles(pVertices[iCount - 1].fInAngle, pVertices[iCount - 1].bHasInAngle, pVertices[iCount - 1].fOutAngle, pVertices[iCount - 1].bHasOutAngle);
		iRet = __xgeSvgApplyMarkerInstance(pSvg, pStyle->sMarkerMidId, pStyle, iTargetDef, pVertices[iCount - 1].tPoint, fAngle, XGE_SVG_MARKER_POSITION_MID);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		fAngle = pVertices[0].bHasInAngle ? pVertices[0].fInAngle : pVertices[0].fOutAngle;
		return __xgeSvgApplyMarkerInstance(pSvg, pStyle->sMarkerEndId, pStyle, iTargetDef, pVertices[0].tPoint, fAngle, XGE_SVG_MARKER_POSITION_END);
	}
	fAngle = pVertices[iCount - 1].bHasInAngle ? pVertices[iCount - 1].fInAngle : pVertices[iCount - 1].fOutAngle;
	return __xgeSvgApplyMarkerInstance(pSvg, pStyle->sMarkerEndId, pStyle, iTargetDef, pVertices[iCount - 1].tPoint, fAngle, XGE_SVG_MARKER_POSITION_END);
}

static int __xgeSvgApplyPathMarkers(xge_svg pSvg, const xge_svg_style_t* pStyle, int iTargetDef, xge_shape_ex pShape)
{
	xge_svg_marker_vertex_t* pVertices;
	int iCount;
	int iCapacity;
	int iPointRead;
	int bHasCurrent;
	int bHasStart;
	xge_vec2_t tCurrent;
	xge_vec2_t tStart;
	float fLastAngle;
	int i;
	int iRet;

	if ( (pStyle == NULL) || (pShape == NULL) ||
	     ((pStyle->sMarkerStartId[0] == '\0') && (pStyle->sMarkerMidId[0] == '\0') && (pStyle->sMarkerEndId[0] == '\0')) ) {
		return XGE_OK;
	}
	pVertices = NULL;
	iCount = 0;
	iCapacity = 0;
	iPointRead = 0;
	bHasCurrent = 0;
	bHasStart = 0;
	memset(&tCurrent, 0, sizeof(tCurrent));
	memset(&tStart, 0, sizeof(tStart));
	fLastAngle = 0.0f;
	for ( i = 0; i < pShape->iCommandCount; i++ ) {
		uint8_t iCommand = pShape->pCommands[i];

		if ( iCommand == XGE_SHAPE_EX_CMD_MOVE_TO ) {
			iRet = __xgeSvgApplyPathMarkerContour(pSvg, pStyle, iTargetDef, pVertices, iCount, 0);
			if ( iRet != XGE_OK ) {
				xrtFree(pVertices);
				return iRet;
			}
			iCount = 0;
			tCurrent = pShape->pPoints[iPointRead++];
			tStart = tCurrent;
			iRet = __xgeSvgMarkerVertexAdd(&pVertices, &iCount, &iCapacity, tCurrent);
			if ( iRet != XGE_OK ) {
				xrtFree(pVertices);
				return iRet;
			}
			bHasCurrent = 1;
			bHasStart = 1;
		} else if ( (iCommand == XGE_SHAPE_EX_CMD_LINE_TO) && bHasCurrent ) {
			xge_vec2_t tEnd = pShape->pPoints[iPointRead++];
			float fAngle;

			if ( iCount <= 0 ) {
				iRet = __xgeSvgMarkerVertexAdd(&pVertices, &iCount, &iCapacity, tCurrent);
				if ( iRet != XGE_OK ) {
					xrtFree(pVertices);
					return iRet;
				}
			}
			fAngle = __xgeSvgMarkerAngleBetween(tCurrent, tEnd, fLastAngle);
			pVertices[iCount - 1].fOutAngle = fAngle;
			pVertices[iCount - 1].bHasOutAngle = 1;
			iRet = __xgeSvgMarkerVertexAdd(&pVertices, &iCount, &iCapacity, tEnd);
			if ( iRet != XGE_OK ) {
				xrtFree(pVertices);
				return iRet;
			}
			pVertices[iCount - 1].fInAngle = fAngle;
			pVertices[iCount - 1].bHasInAngle = 1;
			tCurrent = tEnd;
			fLastAngle = fAngle;
		} else if ( (iCommand == XGE_SHAPE_EX_CMD_CUBIC_TO) && bHasCurrent ) {
			xge_vec2_t tP0 = tCurrent;
			xge_vec2_t tP1 = pShape->pPoints[iPointRead++];
			xge_vec2_t tP2 = pShape->pPoints[iPointRead++];
			xge_vec2_t tP3 = pShape->pPoints[iPointRead++];
			float fOutAngle;
			float fInAngle;

			if ( iCount <= 0 ) {
				iRet = __xgeSvgMarkerVertexAdd(&pVertices, &iCount, &iCapacity, tCurrent);
				if ( iRet != XGE_OK ) {
					xrtFree(pVertices);
					return iRet;
				}
			}
			fOutAngle = __xgeSvgMarkerCubicStartAngle(tP0, tP1, tP2, tP3, fLastAngle);
			fInAngle = __xgeSvgMarkerCubicEndAngle(tP0, tP1, tP2, tP3, fOutAngle);
			pVertices[iCount - 1].fOutAngle = fOutAngle;
			pVertices[iCount - 1].bHasOutAngle = 1;
			iRet = __xgeSvgMarkerVertexAdd(&pVertices, &iCount, &iCapacity, tP3);
			if ( iRet != XGE_OK ) {
				xrtFree(pVertices);
				return iRet;
			}
			pVertices[iCount - 1].fInAngle = fInAngle;
			pVertices[iCount - 1].bHasInAngle = 1;
			tCurrent = tP3;
			fLastAngle = fInAngle;
		} else if ( (iCommand == XGE_SHAPE_EX_CMD_CLOSE) && bHasCurrent && bHasStart ) {
			float fAngle;

			if ( iCount <= 0 ) {
				iRet = __xgeSvgMarkerVertexAdd(&pVertices, &iCount, &iCapacity, tCurrent);
				if ( iRet != XGE_OK ) {
					xrtFree(pVertices);
					return iRet;
				}
			}
			fAngle = __xgeSvgMarkerAngleBetween(tCurrent, tStart, fLastAngle);
			pVertices[iCount - 1].fOutAngle = fAngle;
			pVertices[iCount - 1].bHasOutAngle = 1;
			pVertices[0].fInAngle = fAngle;
			pVertices[0].bHasInAngle = 1;
			iRet = __xgeSvgApplyPathMarkerContour(pSvg, pStyle, iTargetDef, pVertices, iCount, 1);
			if ( iRet != XGE_OK ) {
				xrtFree(pVertices);
				return iRet;
			}
			iCount = 0;
			tCurrent = tStart;
			fLastAngle = fAngle;
		}
	}
	iRet = __xgeSvgApplyPathMarkerContour(pSvg, pStyle, iTargetDef, pVertices, iCount, 0);
	xrtFree(pVertices);
	return iRet;
}

typedef struct xge_svg_path_marker_data_t {
	xge_shape_ex pShape;
} xge_svg_path_marker_data_t;

typedef struct xge_svg_line_marker_data_t {
	xge_vec2_t tA;
	xge_vec2_t tB;
} xge_svg_line_marker_data_t;

typedef struct xge_svg_point_list_marker_data_t {
	const xge_vec2_t* pPoints;
	int iCount;
	int bClosed;
} xge_svg_point_list_marker_data_t;

static int __xgeSvgApplyPathMarkersFromData(xge_svg pSvg, const xge_svg_style_t* pStyle, int iTargetDef, void* pUser)
{
	xge_svg_path_marker_data_t* pData = (xge_svg_path_marker_data_t*)pUser;

	if ( pData == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeSvgApplyPathMarkers(pSvg, pStyle, iTargetDef, pData->pShape);
}

static int __xgeSvgApplyLineMarkersFromData(xge_svg pSvg, const xge_svg_style_t* pStyle, int iTargetDef, void* pUser)
{
	xge_svg_line_marker_data_t* pData = (xge_svg_line_marker_data_t*)pUser;

	if ( pData == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeSvgApplyLineMarkers(pSvg, pStyle, iTargetDef, pData->tA, pData->tB);
}

static int __xgeSvgApplyPointListMarkersFromData(xge_svg pSvg, const xge_svg_style_t* pStyle, int iTargetDef, void* pUser)
{
	xge_svg_point_list_marker_data_t* pData = (xge_svg_point_list_marker_data_t*)pUser;

	if ( pData == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeSvgApplyPointListMarkers(pSvg, pStyle, iTargetDef, pData->pPoints, pData->iCount, pData->bClosed);
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

static int __xgeSvgAddTextItem(xge_svg pSvg, const xge_svg_style_t* pStyle, const char* sText, float fX, float fY, float fAnchorAdvance, xge_shape_ex pTextPath, float fTextPathOffset, const xge_svg_text_layout_t* pLayout)
{
	xge_svg_draw_item_t* pItem;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pStyle == NULL) || (sText == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( sText[0] == '\0' ) {
		return XGE_OK;
	}
	iRet = __xgeSvgDrawItemReserve(pSvg);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pItem = &pSvg->pItems[pSvg->iItemCount];
	memset(pItem, 0, sizeof(*pItem));
	pItem->iType = XGE_SVG_DRAW_ITEM_TEXT;
	pItem->u.tText.sText = __xgeStrDup(sText);
	if ( pItem->u.tText.sText == NULL ) {
		memset(pItem, 0, sizeof(*pItem));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pItem->u.tText.tStyle = *pStyle;
	pItem->u.tText.fX = fX;
	pItem->u.tText.fY = fY;
	pItem->u.tText.fAnchorAdvance = fAnchorAdvance;
	pItem->u.tText.pTextPath = pTextPath;
	pItem->u.tText.fTextPathOffset = fTextPathOffset;
	pItem->u.tText.fTextScaleX = (pLayout != NULL && pLayout->fScaleX > 0.0001f) ? pLayout->fScaleX : 1.0f;
	pItem->u.tText.fTextExtraSpacing = (pLayout != NULL) ? pLayout->fExtraSpacing : 0.0f;
	if ( pTextPath != NULL ) {
		iRet = xgeShapeExAddRef(pTextPath);
		if ( iRet != XGE_OK ) {
			__xgeSvgDrawItemReset(pItem);
			return iRet;
		}
	}
	pSvg->iItemCount++;
	return XGE_OK;
}

static int __xgeSvgAddTextObject(xge_svg pSvg, const xge_svg_style_t* pStyle, const char* sText, float fX, float fY, float fAnchorAdvance, xge_shape_ex pTextPath, float fTextPathOffset, const xge_svg_text_layout_t* pLayout, int iDefIndex)
{
	xge_svg_text_item_t tText;

	if ( !__xgeSvgValid(pSvg) || (pStyle == NULL) || (sText == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( sText[0] == '\0' ) {
		return XGE_OK;
	}
	if ( iDefIndex >= pSvg->iDefCount ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tText, 0, sizeof(tText));
	tText.sText = (char*)sText;
	tText.tStyle = *pStyle;
	tText.fX = fX;
	tText.fY = fY;
	tText.fAnchorAdvance = fAnchorAdvance;
	tText.pTextPath = pTextPath;
	tText.fTextPathOffset = fTextPathOffset;
	tText.fTextScaleX = (pLayout != NULL && pLayout->fScaleX > 0.0001f) ? pLayout->fScaleX : 1.0f;
	tText.fTextExtraSpacing = (pLayout != NULL) ? pLayout->fExtraSpacing : 0.0f;
	return __xgeSvgAddTextItemMasked(pSvg, &tText, iDefIndex);
}

static char* __xgeSvgTextNormalize(char* sText, int iTextSpace)
{
	char* pRead;
	char* pWrite;
	int bSawSpace;
	int bPendingSpace;

	if ( sText == NULL ) {
		return NULL;
	}
	if ( iTextSpace == XGE_SVG_TEXT_SPACE_PRESERVE ) {
		for ( pWrite = sText; *pWrite != '\0'; pWrite++ ) {
			if ( (*pWrite == '\t') || (*pWrite == '\r') || (*pWrite == '\n') || (*pWrite == '\f') ) {
				*pWrite = ' ';
			}
		}
		return sText;
	}
	pRead = sText;
	pWrite = sText;
	bSawSpace = 0;
	bPendingSpace = 0;
	while ( *pRead != '\0' ) {
		unsigned char c = (unsigned char)*pRead++;

		if ( __xgeSvgIsSpace(c) ) {
			bPendingSpace = 1;
			continue;
		}
		if ( bPendingSpace && bSawSpace ) {
			*pWrite++ = ' ';
		}
		*pWrite++ = (char)c;
		bSawSpace = 1;
		bPendingSpace = 0;
	}
	*pWrite = '\0';
	return sText;
}

static char* __xgeSvgTextDecode(const char* pTextStart, const char* pTextEnd, int iTextSpace)
{
	char* sText;
	size_t iCapacity;

	if ( (pTextStart == NULL) || (pTextEnd == NULL) || (pTextEnd < pTextStart) ) {
		return NULL;
	}
	if ( iTextSpace != XGE_SVG_TEXT_SPACE_PRESERVE ) {
		__xgeSvgTrimRange(&pTextStart, &pTextEnd);
	}
	if ( pTextEnd <= pTextStart ) {
		return __xgeStrDup("");
	}
	iCapacity = (size_t)(pTextEnd - pTextStart) + 1u;
	sText = (char*)xrtMalloc(iCapacity);
	if ( sText == NULL ) {
		return NULL;
	}
	__xgeSvgAttrCopyDecoded(pTextStart, pTextEnd, sText, (int)iCapacity);
	return __xgeSvgTextNormalize(sText, iTextSpace);
}

static int __xgeSvgTextIsSpaceCodepoint(uint32_t iCodepoint)
{
	return (iCodepoint == ' ') || (iCodepoint == '\t') || (iCodepoint == '\n') ||
	       (iCodepoint == '\r') || (iCodepoint == '\f');
}

static xge_svg_text_layout_t __xgeSvgTextLayoutDefault(void)
{
	xge_svg_text_layout_t tLayout;

	memset(&tLayout, 0, sizeof(tLayout));
	tLayout.iLengthAdjust = XGE_SVG_TEXT_LENGTH_ADJUST_SPACING;
	tLayout.fScaleX = 1.0f;
	return tLayout;
}

static int __xgeSvgTextLayoutParse(xge_svg pSvg, const char* pTag, const char* pTagEnd, float fFontSize, xge_svg_text_layout_t* pLayout)
{
	char sValue[XGE_SVG_ATTR_MAX];
	float fValue;

	if ( pLayout == NULL ) {
		return 0;
	}
	*pLayout = __xgeSvgTextLayoutDefault();
	if ( (pTag == NULL) || (pTagEnd == NULL) ) {
		return 0;
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "lengthAdjust", sValue, sizeof(sValue)) &&
	     (strcmp(sValue, "spacingAndGlyphs") == 0) ) {
		pLayout->iLengthAdjust = XGE_SVG_TEXT_LENGTH_ADJUST_SPACING_AND_GLYPHS;
	}
	if ( __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "textLength", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_X), fFontSize, &fValue) &&
	     (fValue >= 0.0f) ) {
		pLayout->bHasTextLength = 1;
		pLayout->fTextLength = fValue;
		return 1;
	}
	return 0;
}

static void __xgeSvgTextLayoutResolve(xge_svg_text_layout_t* pLayout, float fNaturalAdvance, int iGapCount)
{
	float fTarget;

	if ( pLayout == NULL ) {
		return;
	}
	pLayout->fNaturalAdvance = fNaturalAdvance;
	pLayout->iGapCount = iGapCount;
	pLayout->fScaleX = 1.0f;
	pLayout->fExtraSpacing = 0.0f;
	if ( !pLayout->bHasTextLength ) {
		return;
	}
	fTarget = pLayout->fTextLength;
	if ( fTarget < 0.0f ) {
		fTarget = 0.0f;
	}
	if ( pLayout->iLengthAdjust == XGE_SVG_TEXT_LENGTH_ADJUST_SPACING_AND_GLYPHS ) {
		if ( fNaturalAdvance > 0.0001f ) {
			pLayout->fScaleX = fTarget / fNaturalAdvance;
		} else {
			pLayout->fScaleX = 1.0f;
		}
		return;
	}
	if ( iGapCount > 0 ) {
		pLayout->fExtraSpacing = (fTarget - fNaturalAdvance) / (float)iGapCount;
	}
}

static float __xgeSvgTextLayoutAdvance(const xge_svg_text_layout_t* pLayout, float fNaturalAdvance, int iGapCount)
{
	if ( pLayout == NULL ) {
		return fNaturalAdvance;
	}
	return fNaturalAdvance * pLayout->fScaleX + pLayout->fExtraSpacing * (float)iGapCount;
}

static float __xgeSvgTextSpacingForCodepoint(const xge_svg_style_t* pStyle, uint32_t iCodepoint, int bHasNext)
{
	float fSpacing;

	if ( (pStyle == NULL) || !bHasNext ) {
		return 0.0f;
	}
	fSpacing = pStyle->fLetterSpacing;
	if ( __xgeSvgTextIsSpaceCodepoint(iCodepoint) ) {
		fSpacing += pStyle->fWordSpacing;
	}
	return fSpacing;
}

static float __xgeSvgTextApproxAdvanceEx(const char* sText, const xge_svg_style_t* pStyle, int* pGapCount)
{
	const unsigned char* p;
	float fAdvance;
	float fFontSize;
	int iGapCount;

	if ( pGapCount != NULL ) {
		*pGapCount = 0;
	}
	if ( (sText == NULL) || (pStyle == NULL) ) {
		return 0.0f;
	}
	fFontSize = pStyle->fFontSize;
	if ( fFontSize <= 0.0f ) {
		fFontSize = 16.0f;
	}
	p = (const unsigned char*)sText;
	fAdvance = 0.0f;
	iGapCount = 0;
	while ( *p != '\0' ) {
		uint32_t iCodepoint;
		int bHasNext;

		if ( *p < 0x80u ) {
			iCodepoint = (uint32_t)*p;
			fAdvance += (*p == ' ' || *p == '\t') ? fFontSize * 0.32f : fFontSize * 0.56f;
			p++;
		} else {
			iCodepoint = 0x100u;
			fAdvance += fFontSize;
			if ( (*p & 0xE0u) == 0xC0u ) p += 2;
			else if ( (*p & 0xF0u) == 0xE0u ) p += 3;
			else if ( (*p & 0xF8u) == 0xF0u ) p += 4;
			else p++;
		}
		bHasNext = *p != '\0';
		fAdvance += __xgeSvgTextSpacingForCodepoint(pStyle, iCodepoint, bHasNext);
		if ( bHasNext ) {
			iGapCount++;
		}
	}
	if ( pGapCount != NULL ) {
		*pGapCount = iGapCount;
	}
	return fAdvance;
}

static int __xgeSvgAddDecodedTextRun(xge_svg pSvg, const xge_svg_style_t* pStyle, const char* pTextStart, const char* pTextEnd, float fX, float fY, float fAnchorAdvance, xge_shape_ex pTextPath, float fTextPathOffset, const xge_svg_text_layout_t* pLayout, int iTargetDef, int iMainIdDefIndex, int iExtraMainIdDefIndex, int bEmit, float* pAdvance, int* pGapCount)
{
	char* sText;
	float fAdvance;
	float fAdjustedAdvance;
	int iGapCount;
	int iRet;

	if ( pAdvance != NULL ) {
		*pAdvance = 0.0f;
	}
	if ( pGapCount != NULL ) {
		*pGapCount = 0;
	}
	if ( !__xgeSvgValid(pSvg) || (pStyle == NULL) || (pTextStart == NULL) || (pTextEnd == NULL) || (pTextEnd < pTextStart) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sText = __xgeSvgTextDecode(pTextStart, pTextEnd, pStyle->iTextSpace);
	if ( sText == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	fAdvance = __xgeSvgTextApproxAdvanceEx(sText, pStyle, &iGapCount);
	fAdjustedAdvance = __xgeSvgTextLayoutAdvance(pLayout, fAdvance, iGapCount);
	iRet = XGE_OK;
	if ( bEmit ) {
		iRet = __xgeSvgAddTextObject(pSvg, pStyle, sText, fX, fY, fAnchorAdvance, pTextPath, fTextPathOffset + fX, pLayout, iTargetDef);
		if ( (iRet == XGE_OK) && (iTargetDef < 0) && (iMainIdDefIndex >= 0) ) {
			iRet = __xgeSvgAddTextObject(pSvg, pStyle, sText, fX, fY, fAnchorAdvance, pTextPath, fTextPathOffset + fX, pLayout, iMainIdDefIndex);
		}
		if ( (iRet == XGE_OK) && (iTargetDef < 0) && (iExtraMainIdDefIndex >= 0) && (iExtraMainIdDefIndex != iMainIdDefIndex) ) {
			iRet = __xgeSvgAddTextObject(pSvg, pStyle, sText, fX, fY, fAnchorAdvance, pTextPath, fTextPathOffset + fX, pLayout, iExtraMainIdDefIndex);
		}
	}
	if ( (iRet == XGE_OK) && (pAdvance != NULL) ) {
		*pAdvance = fAdjustedAdvance;
	}
	if ( (iRet == XGE_OK) && (pGapCount != NULL) ) {
		*pGapCount = iGapCount;
	}
	xrtFree(sText);
	return iRet;
}

static int __xgeSvgTextPathClone(xge_svg pSvg, const char* pTag, const char* pTagEnd, xge_shape_ex* ppPath)
{
	char sId[XGE_SVG_ID_MAX];
	int iDefIndex;

	if ( ppPath != NULL ) {
		*ppPath = NULL;
	}
	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (ppPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeSvgHrefIdCopy(pTag, pTagEnd, sId, sizeof(sId)) ) {
		return XGE_OK;
	}
	iDefIndex = __xgeSvgDefFind(pSvg, sId);
	if ( (iDefIndex < 0) || (pSvg->pDefs[iDefIndex].iShapeCount <= 0) ) {
		return XGE_OK;
	}
	return xgeShapeExClone(pSvg->pDefs[iDefIndex].pShapes[0], ppPath);
}

static int __xgeSvgShapeLengthTransformed(xge_shape_ex pShape, float fTolerance, float* pLength)
{
	xge_shape_ex_path_measure pMeasure;
	int iRet;

	pMeasure = NULL;
	iRet = xgeShapeExPathMeasureCreate(&pMeasure, pShape, &pShape->tTransform, fTolerance);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeShapeExPathMeasureGetLength(pMeasure, pLength);
	xgeShapeExPathMeasureDestroy(pMeasure);
	return iRet;
}

static int __xgeSvgShapePointAtLengthTransformed(xge_shape_ex pShape, float fDistance, float fTolerance, xge_vec2_t* pPoint, xge_vec2_t* pTangent, float* pLength)
{
	xge_shape_ex_path_measure pMeasure;
	int iRet;

	pMeasure = NULL;
	iRet = xgeShapeExPathMeasureCreate(&pMeasure, pShape, &pShape->tTransform, fTolerance);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( pLength != NULL ) {
		iRet = xgeShapeExPathMeasureGetLength(pMeasure, pLength);
		if ( iRet != XGE_OK ) {
			xgeShapeExPathMeasureDestroy(pMeasure);
			return iRet;
		}
	}
	iRet = xgeShapeExPathMeasureGetPointAtLength(pMeasure, fDistance, pPoint, pTangent);
	xgeShapeExPathMeasureDestroy(pMeasure);
	return iRet;
}

static float __xgeSvgTextPathStartOffset(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_style_t* pStyle, xge_shape_ex pPath)
{
	char sValue[XGE_SVG_ATTR_MAX];
	float fPathLength;
	float fOffset;

	if ( (pStyle == NULL) || (pPath == NULL) ) {
		return 0.0f;
	}
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "startOffset", sValue, sizeof(sValue)) ) {
		return 0.0f;
	}
	fPathLength = 0.0f;
	if ( __xgeSvgShapeLengthTransformed(pPath, 0.0f, &fPathLength) != XGE_OK ) {
		fPathLength = __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_OTHER);
	}
	fOffset = 0.0f;
	return __xgeSvgParseLengthEx(sValue, fPathLength, pStyle->fFontSize, &fOffset) ? fOffset : 0.0f;
}

static const char* __xgeSvgFindCloseElement(const char* pStart, const char* sName, const char** ppTagEnd)
{
	const char* p;
	int iDepth;

	if ( ppTagEnd != NULL ) {
		*ppTagEnd = NULL;
	}
	if ( (pStart == NULL) || (sName == NULL) ) {
		return NULL;
	}
	p = pStart;
	iDepth = 0;
	while ( (p = strchr(p, '<')) != NULL ) {
		const char* pTagEnd = __xgeSvgFindTagEnd(p);

		if ( pTagEnd == NULL ) {
			return NULL;
		}
		if ( __xgeSvgIsCloseTagName(p, sName) ) {
			if ( iDepth <= 0 ) {
				if ( ppTagEnd != NULL ) {
					*ppTagEnd = pTagEnd;
				}
				return p;
			}
			iDepth--;
		} else if ( __xgeSvgTagNameEquals(p, sName) && !__xgeSvgIsSelfClosingTag(p, pTagEnd) ) {
			iDepth++;
		}
		p = pTagEnd + 1;
	}
	return NULL;
}

static int __xgeSvgParseTextChildrenPass(xge_svg pSvg, const char* pTextStart, const char* pTextEnd, const xge_svg_style_t* pTextStyle, float fTextX, float fTextY, float fAnchorAdvance, xge_shape_ex pTextPath, float fTextPathOffset, const xge_svg_text_layout_t* pLayout, int iTargetDef, int iMainIdDefIndex, int iExtraMainIdDefIndex, int bEmit, float* pTotalAdvance, int* pRunCount, int* pTotalGapCount)
{
	const char* p;
	float fCursorX;
	float fCursorY;
	int iRet;

	if ( pTotalAdvance != NULL ) {
		*pTotalAdvance = 0.0f;
	}
	if ( pRunCount != NULL ) {
		*pRunCount = 0;
	}
	if ( pTotalGapCount != NULL ) {
		*pTotalGapCount = 0;
	}
	if ( !__xgeSvgValid(pSvg) || (pTextStart == NULL) || (pTextEnd == NULL) || (pTextEnd < pTextStart) || (pTextStyle == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	p = pTextStart;
	fCursorX = fTextX;
	fCursorY = fTextY;
	while ( p < pTextEnd ) {
		const char* pTag;

		pTag = p;
		while ( (pTag < pTextEnd) && (*pTag != '<') ) {
			pTag++;
		}
		if ( pTag > p ) {
			float fAdvance;
			int iGapCount;

			iGapCount = 0;
			iRet = __xgeSvgAddDecodedTextRun(pSvg, pTextStyle, p, pTag, fCursorX, fCursorY, fAnchorAdvance, pTextPath, fTextPathOffset, pLayout, iTargetDef, iMainIdDefIndex, iExtraMainIdDefIndex, bEmit, &fAdvance, &iGapCount);
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			if ( (fAdvance > 0.0f) && (pRunCount != NULL) ) {
				(*pRunCount)++;
			}
			if ( pTotalGapCount != NULL ) {
				*pTotalGapCount += iGapCount;
			}
			fCursorX += fAdvance;
		}
		if ( pTag >= pTextEnd ) {
			break;
		}
		{
			const char* pTagEnd = __xgeSvgFindTagEnd(pTag);
			int bSelfClosing;

			if ( (pTagEnd == NULL) || (pTagEnd > pTextEnd) ) {
				break;
			}
			bSelfClosing = __xgeSvgIsSelfClosingTag(pTag, pTagEnd);
			if ( __xgeSvgTagNameEquals(pTag, "tspan") ) {
				const char* pTSpanTextStart;
				const char* pTSpanTextEnd;
				const char* pTSpanCloseEnd;
				xge_svg_style_t tSpanStyle;
				xge_svg_text_layout_t tSpanLayout;
				xge_svg_text_layout_t tSpanNaturalLayout;
				float fRunX;
				float fRunY;
				float fDelta;
				float fAdvance;
				int iGapCount;
				int iSpanRunCount;
				int bSpanHasTextLength;

				tSpanStyle = __xgeSvgStyleResolve(pSvg, pTextStyle, pTag, pTagEnd);
				bSpanHasTextLength = __xgeSvgTextLayoutParse(pSvg, pTag, pTagEnd, tSpanStyle.fFontSize, &tSpanLayout);
				fRunX = fCursorX;
				fRunY = fCursorY;
				fRunX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, tSpanStyle.fFontSize, fRunX);
				fRunY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, tSpanStyle.fFontSize, fRunY);
				if ( __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "dx", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_X), tSpanStyle.fFontSize, &fDelta) ) {
					fRunX += fDelta;
				}
				if ( __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "dy", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_Y), tSpanStyle.fFontSize, &fDelta) ) {
					fRunY += fDelta;
				}
				pTSpanTextStart = pTagEnd + 1;
				pTSpanTextEnd = pTSpanTextStart;
				pTSpanCloseEnd = NULL;
				fAdvance = 0.0f;
				iSpanRunCount = 0;
				if ( !bSelfClosing ) {
					const char* pClose = __xgeSvgFindCloseElement(pTagEnd + 1, "tspan", &pTSpanCloseEnd);
					if ( (pClose != NULL) && (pClose <= pTextEnd) ) {
						pTSpanTextEnd = pClose;
					}
				}
				if ( pTSpanTextEnd > pTSpanTextStart ) {
					iGapCount = 0;
					if ( bSpanHasTextLength ) {
						float fNaturalAdvance;
						int iNaturalGapCount;
						int iNaturalRunCount;

						tSpanNaturalLayout = __xgeSvgTextLayoutDefault();
						fNaturalAdvance = 0.0f;
						iNaturalGapCount = 0;
						iNaturalRunCount = 0;
						iRet = __xgeSvgParseTextChildrenPass(pSvg, pTSpanTextStart, pTSpanTextEnd, &tSpanStyle, fRunX, fRunY, 0.0f, pTextPath, fTextPathOffset, &tSpanNaturalLayout, iTargetDef, iMainIdDefIndex, iExtraMainIdDefIndex, 0, &fNaturalAdvance, &iNaturalRunCount, &iNaturalGapCount);
						if ( iRet != XGE_OK ) {
							return iRet;
						}
						__xgeSvgTextLayoutResolve(&tSpanLayout, fNaturalAdvance, iNaturalGapCount);
						iRet = __xgeSvgParseTextChildrenPass(pSvg, pTSpanTextStart, pTSpanTextEnd, &tSpanStyle, fRunX, fRunY, fAnchorAdvance, pTextPath, fTextPathOffset, &tSpanLayout, iTargetDef, iMainIdDefIndex, iExtraMainIdDefIndex, bEmit, &fAdvance, &iSpanRunCount, &iGapCount);
					} else {
						iRet = __xgeSvgParseTextChildrenPass(pSvg, pTSpanTextStart, pTSpanTextEnd, &tSpanStyle, fRunX, fRunY, fAnchorAdvance, pTextPath, fTextPathOffset, pLayout, iTargetDef, iMainIdDefIndex, iExtraMainIdDefIndex, bEmit, &fAdvance, &iSpanRunCount, &iGapCount);
					}
					if ( iRet != XGE_OK ) {
						return iRet;
					}
					if ( pRunCount != NULL ) {
						*pRunCount += iSpanRunCount;
					}
					if ( pTotalGapCount != NULL ) {
						*pTotalGapCount += iGapCount;
					}
				}
				fCursorX = fRunX + fAdvance;
				fCursorY = fRunY;
				p = (pTSpanCloseEnd != NULL) ? (pTSpanCloseEnd + 1) : (pTagEnd + 1);
				continue;
			}
			if ( __xgeSvgTagNameEquals(pTag, "textPath") ) {
				const char* pPathTextStart;
				const char* pPathTextEnd;
				const char* pPathCloseEnd;
				xge_svg_style_t tPathStyle;
				xge_svg_text_layout_t tPathLayout;
				xge_svg_text_layout_t tPathNaturalLayout;
				xge_shape_ex pPathShape;
				float fPathAdvance;
				float fPathOffset;
				int iPathGapCount;
				int iPathRunCount;
				int bPathHasTextLength;

				tPathStyle = __xgeSvgStyleResolve(pSvg, pTextStyle, pTag, pTagEnd);
				bPathHasTextLength = __xgeSvgTextLayoutParse(pSvg, pTag, pTagEnd, tPathStyle.fFontSize, &tPathLayout);
				pPathShape = NULL;
				iRet = __xgeSvgTextPathClone(pSvg, pTag, pTagEnd, &pPathShape);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
				pPathTextStart = pTagEnd + 1;
				pPathTextEnd = pPathTextStart;
				pPathCloseEnd = NULL;
				fPathAdvance = 0.0f;
				iPathGapCount = 0;
				iPathRunCount = 0;
				if ( !bSelfClosing ) {
					const char* pClose = __xgeSvgFindCloseElement(pTagEnd + 1, "textPath", &pPathCloseEnd);
					if ( (pClose != NULL) && (pClose <= pTextEnd) ) {
						pPathTextEnd = pClose;
					}
				}
				if ( (pPathShape != NULL) && (pPathTextEnd > pPathTextStart) ) {
					fPathOffset = fTextPathOffset + __xgeSvgTextPathStartOffset(pSvg, pTag, pTagEnd, &tPathStyle, pPathShape);
					if ( bPathHasTextLength ) {
						float fNaturalAdvance;
						int iNaturalGapCount;
						int iNaturalRunCount;

						tPathNaturalLayout = __xgeSvgTextLayoutDefault();
						fNaturalAdvance = 0.0f;
						iNaturalGapCount = 0;
						iNaturalRunCount = 0;
						iRet = __xgeSvgParseTextChildrenPass(pSvg, pPathTextStart, pPathTextEnd, &tPathStyle, 0.0f, fCursorY, 0.0f, pPathShape, fPathOffset, &tPathNaturalLayout, iTargetDef, iMainIdDefIndex, iExtraMainIdDefIndex, 0, &fNaturalAdvance, &iNaturalRunCount, &iNaturalGapCount);
						if ( iRet != XGE_OK ) {
							xgeShapeExDestroy(pPathShape);
							return iRet;
						}
						__xgeSvgTextLayoutResolve(&tPathLayout, fNaturalAdvance, iNaturalGapCount);
						iRet = __xgeSvgParseTextChildrenPass(pSvg, pPathTextStart, pPathTextEnd, &tPathStyle, 0.0f, fCursorY, fAnchorAdvance, pPathShape, fPathOffset, &tPathLayout, iTargetDef, iMainIdDefIndex, iExtraMainIdDefIndex, bEmit, &fPathAdvance, &iPathRunCount, &iPathGapCount);
					} else {
						iRet = __xgeSvgParseTextChildrenPass(pSvg, pPathTextStart, pPathTextEnd, &tPathStyle, 0.0f, fCursorY, fAnchorAdvance, pPathShape, fPathOffset, pLayout, iTargetDef, iMainIdDefIndex, iExtraMainIdDefIndex, bEmit, &fPathAdvance, &iPathRunCount, &iPathGapCount);
					}
					if ( iRet != XGE_OK ) {
						xgeShapeExDestroy(pPathShape);
						return iRet;
					}
					if ( pRunCount != NULL ) {
						*pRunCount += iPathRunCount;
					}
					if ( pTotalGapCount != NULL ) {
						*pTotalGapCount += iPathGapCount;
					}
				}
				xgeShapeExDestroy(pPathShape);
				fCursorX += fPathAdvance;
				p = (pPathCloseEnd != NULL) ? (pPathCloseEnd + 1) : (pTagEnd + 1);
				continue;
			}
			p = pTagEnd + 1;
		}
	}
	if ( pTotalAdvance != NULL ) {
		*pTotalAdvance = fCursorX - fTextX;
		if ( *pTotalAdvance < 0.0f ) {
			*pTotalAdvance = 0.0f;
		}
	}
	return XGE_OK;
}

static int __xgeSvgParseTextChildren(xge_svg pSvg, const char* pTextStart, const char* pTextEnd, const xge_svg_style_t* pTextStyle, float fTextX, float fTextY, const xge_svg_text_layout_t* pTextLayout, int iTargetDef, int iMainIdDefIndex, int iExtraMainIdDefIndex)
{
	xge_svg_text_layout_t tNaturalLayout;
	xge_svg_text_layout_t tResolvedLayout;
	float fTotalAdvance;
	float fAdjustedAdvance;
	float fAnchorAdvance;
	int iGapCount;
	int iRunCount;
	int iRet;

	tNaturalLayout = __xgeSvgTextLayoutDefault();
	iGapCount = 0;
	iRet = __xgeSvgParseTextChildrenPass(pSvg, pTextStart, pTextEnd, pTextStyle, fTextX, fTextY, 0.0f, NULL, 0.0f, &tNaturalLayout, iTargetDef, iMainIdDefIndex, iExtraMainIdDefIndex, 0, &fTotalAdvance, &iRunCount, &iGapCount);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tResolvedLayout = (pTextLayout != NULL) ? *pTextLayout : __xgeSvgTextLayoutDefault();
	__xgeSvgTextLayoutResolve(&tResolvedLayout, fTotalAdvance, iGapCount);
	fAdjustedAdvance = __xgeSvgTextLayoutAdvance(&tResolvedLayout, fTotalAdvance, iGapCount);
	fAnchorAdvance = (iRunCount > 1) ? fAdjustedAdvance : 0.0f;
	return __xgeSvgParseTextChildrenPass(pSvg, pTextStart, pTextEnd, pTextStyle, fTextX, fTextY, fAnchorAdvance, NULL, 0.0f, &tResolvedLayout, iTargetDef, iMainIdDefIndex, iExtraMainIdDefIndex, 1, NULL, NULL, NULL);
}

static int __xgeSvgParseText(xge_svg pSvg, const char* pTag, const char* pTagEnd, const char* pTextStart, const char* pTextEnd, const xge_svg_style_t* pParentStyle, int bInDefs, int iCurrentDef, int iGroupMainIdDefIndex)
{
	xge_svg_style_t tStyle;
	xge_svg_text_layout_t tTextLayout;
	float fX;
	float fY;
	float fDelta;
	int iTargetDef;
	int iOwnMainIdDefIndex;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pParentStyle == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pTextStart == NULL) || (pTextEnd == NULL) || (pTextEnd < pTextStart) ) {
		return XGE_OK;
	}
	iTargetDef = __xgeSvgShapeTargetDef(pSvg, pTag, pTagEnd, bInDefs, iCurrentDef, &iRet);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( iTargetDef == -2 ) {
		return XGE_OK;
	}
	iRet = __xgeSvgMainIdDefIndex(pSvg, pTag, pTagEnd, bInDefs, iCurrentDef, &iOwnMainIdDefIndex);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tStyle = __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd);
	__xgeSvgTextLayoutParse(pSvg, pTag, pTagEnd, tStyle.fFontSize, &tTextLayout);
	fX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
	fY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
	if ( __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "dx", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_X), tStyle.fFontSize, &fDelta) ) {
		fX += fDelta;
	}
	if ( __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "dy", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_Y), tStyle.fFontSize, &fDelta) ) {
		fY += fDelta;
	}
	return __xgeSvgParseTextChildren(pSvg, pTextStart, pTextEnd, &tStyle, fX, fY, &tTextLayout, iTargetDef, iOwnMainIdDefIndex, iGroupMainIdDefIndex);
}

static int __xgeSvgBase64Value(char c)
{
	if ( (c >= 'A') && (c <= 'Z') ) return c - 'A';
	if ( (c >= 'a') && (c <= 'z') ) return c - 'a' + 26;
	if ( (c >= '0') && (c <= '9') ) return c - '0' + 52;
	if ( c == '+' ) return 62;
	if ( c == '/' ) return 63;
	return -1;
}

static int __xgeSvgDecodeBase64(const char* sText, unsigned char** ppData, int* pSize)
{
	unsigned char* pData;
	size_t iLen;
	size_t iCapacity;
	int iValue;
	int iBits;
	int iCount;

	if ( (sText == NULL) || (ppData == NULL) || (pSize == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppData = NULL;
	*pSize = 0;
	iLen = strlen(sText);
	iCapacity = (iLen * 3u) / 4u + 4u;
	pData = (unsigned char*)xrtMalloc(iCapacity);
	if ( pData == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iValue = 0;
	iBits = 0;
	iCount = 0;
	while ( *sText != '\0' ) {
		int iDigit;

		if ( __xgeSvgIsSpace((unsigned char)*sText) ) {
			sText++;
			continue;
		}
		if ( *sText == '=' ) {
			break;
		}
		iDigit = __xgeSvgBase64Value(*sText);
		if ( iDigit < 0 ) {
			xrtFree(pData);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iValue = (iValue << 6) | iDigit;
		iBits += 6;
		if ( iBits >= 8 ) {
			iBits -= 8;
			pData[iCount++] = (unsigned char)((iValue >> iBits) & 0xFF);
		}
		sText++;
	}
	*ppData = pData;
	*pSize = iCount;
	return XGE_OK;
}

static int __xgeSvgDecodeUriData(const char* sText, char** ppText, int* pSize)
{
	char* sOut;
	int iLen;
	int iRead;
	int iWrite;

	if ( (sText == NULL) || (ppText == NULL) || (pSize == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppText = NULL;
	*pSize = 0;
	iLen = (int)strlen(sText);
	sOut = (char*)xrtMalloc((size_t)iLen + 1u);
	if ( sOut == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRead = 0;
	iWrite = 0;
	while ( iRead < iLen ) {
		if ( (sText[iRead] == '%') &&
		     ((iRead + 2) < iLen) &&
		     __xgeSvgIsHexChar((unsigned char)sText[iRead + 1]) &&
		     __xgeSvgIsHexChar((unsigned char)sText[iRead + 2]) ) {
			sOut[iWrite++] = (char)((__xgeSvgHexNibble(sText[iRead + 1]) << 4) |
			                        __xgeSvgHexNibble(sText[iRead + 2]));
			iRead += 3;
		} else {
			sOut[iWrite++] = sText[iRead++];
		}
	}
	sOut[iWrite] = '\0';
	*ppText = sOut;
	*pSize = iWrite;
	return XGE_OK;
}

static int __xgeSvgDecodeSvgDataUri(const char* sHref, char** ppSvgText, int* pSize)
{
	const char* sPrefix = "data:image/svg+xml";
	const char* pComma;
	const char* pMeta;
	int bBase64;
	size_t iPrefixLen;

	if ( (sHref == NULL) || (ppSvgText == NULL) || (pSize == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppSvgText = NULL;
	*pSize = 0;
	iPrefixLen = strlen(sPrefix);
	if ( strncmp(sHref, sPrefix, iPrefixLen) != 0 ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	pComma = strchr(sHref, ',');
	if ( pComma == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	bBase64 = 0;
	pMeta = sHref + iPrefixLen;
	while ( pMeta < pComma ) {
		if ( strncmp(pMeta, ";base64", 7) == 0 ) {
			bBase64 = 1;
			break;
		}
		pMeta++;
	}
	if ( bBase64 ) {
		unsigned char* pData;
		int iRet;

		pData = NULL;
		iRet = __xgeSvgDecodeBase64(pComma + 1, &pData, pSize);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		*ppSvgText = (char*)xrtMalloc((size_t)(*pSize) + 1u);
		if ( *ppSvgText == NULL ) {
			xrtFree(pData);
			*pSize = 0;
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memcpy(*ppSvgText, pData, (size_t)(*pSize));
		(*ppSvgText)[*pSize] = '\0';
		xrtFree(pData);
		return XGE_OK;
	}
	return __xgeSvgDecodeUriData(pComma + 1, ppSvgText, pSize);
}

static int __xgeSvgDecodeRasterImageDataUri(const char* sHref, unsigned char** ppData, int* pSize)
{
	const char* sPrefix = "data:image/";
	const char* sSvgPrefix = "data:image/svg+xml";
	const char* pComma;
	const char* pMeta;
	int bBase64;
	size_t iPrefixLen;
	size_t iSvgPrefixLen;

	if ( (sHref == NULL) || (ppData == NULL) || (pSize == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppData = NULL;
	*pSize = 0;
	iPrefixLen = strlen(sPrefix);
	if ( strncmp(sHref, sPrefix, iPrefixLen) != 0 ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	iSvgPrefixLen = strlen(sSvgPrefix);
	if ( strncmp(sHref, sSvgPrefix, iSvgPrefixLen) == 0 ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	pComma = strchr(sHref, ',');
	if ( pComma == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	bBase64 = 0;
	pMeta = sHref + iPrefixLen;
	while ( pMeta < pComma ) {
		if ( strncmp(pMeta, ";base64", 7) == 0 ) {
			bBase64 = 1;
			break;
		}
		pMeta++;
	}
	if ( bBase64 ) {
		return __xgeSvgDecodeBase64(pComma + 1, ppData, pSize);
	}
	return __xgeSvgDecodeUriData(pComma + 1, (char**)ppData, pSize);
}

static int __xgeSvgUriHasExt(const char* sURI, const char* sExt)
{
	const char* pEnd;
	const char* pDot;
	size_t iExtLen;
	size_t iNameLen;
	size_t i;

	if ( (sURI == NULL) || (sExt == NULL) ) {
		return 0;
	}
	pEnd = sURI + strlen(sURI);
	while ( (pEnd > sURI) && ((pEnd[-1] == '#') || (pEnd[-1] == '?')) ) {
		pEnd--;
	}
	for ( i = 0; sURI + i < pEnd; i++ ) {
		if ( (sURI[i] == '#') || (sURI[i] == '?') ) {
			pEnd = sURI + i;
			break;
		}
	}
	pDot = pEnd;
	while ( (pDot > sURI) && (pDot[-1] != '.') && (pDot[-1] != '/') && (pDot[-1] != '\\') ) {
		pDot--;
	}
	if ( (pDot <= sURI) || (pDot[-1] != '.') ) {
		return 0;
	}
	pDot--;
	iNameLen = (size_t)(pEnd - pDot);
	iExtLen = strlen(sExt);
	if ( iNameLen != iExtLen ) {
		return 0;
	}
	for ( i = 0; i < iExtLen; i++ ) {
		if ( __xgeSvgAsciiLower((unsigned char)pDot[i]) != __xgeSvgAsciiLower((unsigned char)sExt[i]) ) {
			return 0;
		}
	}
	return 1;
}

static int __xgeSvgMemoryLooksLikeSvg(const void* pData, int iSize)
{
	const unsigned char* p;
	int i;

	if ( (pData == NULL) || (iSize <= 0) ) {
		return 0;
	}
	p = (const unsigned char*)pData;
	i = 0;
	if ( (iSize >= 3) && (p[0] == 0xEFu) && (p[1] == 0xBBu) && (p[2] == 0xBFu) ) {
		i = 3;
	}
	while ( (i < iSize) &&
	        ((p[i] == ' ') || (p[i] == '\t') || (p[i] == '\r') || (p[i] == '\n') || (p[i] == '\f')) ) {
		i++;
	}
	return (i < iSize) && (p[i] == '<');
}

static int __xgeSvgLoadExternalSvgImage(xge_svg pParentSvg, const char* sResolved, const void* pData, int iSize, xge_svg* ppChildSvg)
{
	char* sBaseDir;
	xge_svg pChildSvg;
	int iRet;

	if ( !__xgeSvgValid(pParentSvg) || (sResolved == NULL) || (pData == NULL) || (iSize <= 0) || (ppChildSvg == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)pParentSvg;
	*ppChildSvg = NULL;
	sBaseDir = __xgeSvgUriBaseDir(sResolved);
	if ( sBaseDir == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pChildSvg = NULL;
	iRet = xgeSvgCreate(&pChildSvg);
	if ( iRet == XGE_OK ) {
		iRet = __xgeSvgLoadMemoryEx(pChildSvg, pData, iSize, sBaseDir);
	}
	xrtFree(sBaseDir);
	if ( iRet != XGE_OK ) {
		xgeSvgDestroy(pChildSvg);
		return iRet;
	}
	*ppChildSvg = pChildSvg;
	return XGE_OK;
}

static int __xgeSvgLoadExternalImage(xge_svg pSvg, const char* sHref, xge_svg* ppChildSvg, xge_svg_raster_image_t** ppRaster)
{
	xge_resource_t tResource;
	char* sResolved;
	char* sLoadUri;
	int bPreferSvg;
	int bLooksSvg;
	int iDecodedSize;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (sHref == NULL) || (ppChildSvg == NULL) || (ppRaster == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppChildSvg = NULL;
	*ppRaster = NULL;
	sResolved = __xgeSvgResolveHrefUri(pSvg, sHref);
	if ( sResolved == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( (sResolved[0] == '\0') || (sResolved[0] == '#') ) {
		xrtFree(sResolved);
		return XGE_OK;
	}
	sLoadUri = sResolved;
	if ( strchr(sResolved, '%') != NULL ) {
		iDecodedSize = 0;
		iRet = __xgeSvgDecodeUriData(sResolved, &sLoadUri, &iDecodedSize);
		if ( iRet != XGE_OK ) {
			xrtFree(sResolved);
			return iRet;
		}
	}
	memset(&tResource, 0, sizeof(tResource));
	iRet = xgeResourceLoad(sLoadUri, &tResource);
	if ( iRet != XGE_OK ) {
		if ( sLoadUri != sResolved ) {
			xrtFree(sLoadUri);
		}
		xrtFree(sResolved);
		return XGE_OK;
	}
	bPreferSvg = __xgeSvgUriHasExt(sLoadUri, ".svg");
	bLooksSvg = __xgeSvgMemoryLooksLikeSvg(tResource.pData, tResource.iSize);
	if ( bPreferSvg || bLooksSvg ) {
		iRet = __xgeSvgLoadExternalSvgImage(pSvg, sLoadUri, tResource.pData, tResource.iSize, ppChildSvg);
		if ( iRet == XGE_OK ) {
			xgeResourceFree(&tResource);
			if ( sLoadUri != sResolved ) {
				xrtFree(sLoadUri);
			}
			xrtFree(sResolved);
			return XGE_OK;
		}
	}
	iRet = __xgeSvgRasterImageCreateFromMemory(tResource.pData, tResource.iSize, ppRaster);
	if ( (iRet != XGE_OK) && !bPreferSvg && bLooksSvg ) {
		iRet = __xgeSvgLoadExternalSvgImage(pSvg, sLoadUri, tResource.pData, tResource.iSize, ppChildSvg);
	}
	xgeResourceFree(&tResource);
	if ( sLoadUri != sResolved ) {
		xrtFree(sLoadUri);
	}
	xrtFree(sResolved);
	if ( iRet != XGE_OK ) {
		__xgeSvgRasterImageDestroy(*ppRaster);
		*ppRaster = NULL;
		xgeSvgDestroy(*ppChildSvg);
		*ppChildSvg = NULL;
		return XGE_OK;
	}
	return XGE_OK;
}

static int __xgeSvgLoadExternalSvgDocument(xge_svg pSvg, const char* sHref, xge_svg* ppChildSvg)
{
	xge_resource_t tResource;
	char* sResolved;
	char* sLoadUri;
	int bPreferSvg;
	int bLooksSvg;
	int iDecodedSize;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (sHref == NULL) || (ppChildSvg == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppChildSvg = NULL;
	sResolved = __xgeSvgResolveHrefUri(pSvg, sHref);
	if ( sResolved == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( (sResolved[0] == '\0') || (sResolved[0] == '#') ) {
		xrtFree(sResolved);
		return XGE_OK;
	}
	sLoadUri = sResolved;
	if ( strchr(sResolved, '%') != NULL ) {
		iDecodedSize = 0;
		iRet = __xgeSvgDecodeUriData(sResolved, &sLoadUri, &iDecodedSize);
		if ( iRet != XGE_OK ) {
			xrtFree(sResolved);
			return iRet;
		}
	}
	memset(&tResource, 0, sizeof(tResource));
	iRet = xgeResourceLoad(sLoadUri, &tResource);
	if ( iRet != XGE_OK ) {
		if ( sLoadUri != sResolved ) {
			xrtFree(sLoadUri);
		}
		xrtFree(sResolved);
		return XGE_OK;
	}
	bPreferSvg = __xgeSvgUriHasExt(sLoadUri, ".svg");
	bLooksSvg = __xgeSvgMemoryLooksLikeSvg(tResource.pData, tResource.iSize);
	if ( bPreferSvg || bLooksSvg ) {
		iRet = __xgeSvgLoadExternalSvgImage(pSvg, sLoadUri, tResource.pData, tResource.iSize, ppChildSvg);
	} else {
		iRet = XGE_OK;
	}
	xgeResourceFree(&tResource);
	if ( sLoadUri != sResolved ) {
		xrtFree(sLoadUri);
	}
	xrtFree(sResolved);
	if ( iRet != XGE_OK ) {
		xgeSvgDestroy(*ppChildSvg);
		*ppChildSvg = NULL;
		return iRet;
	}
	return XGE_OK;
}

static int __xgeSvgAddImageItem(xge_svg pSvg, const xge_svg_style_t* pStyle, xge_svg pChildSvg, xge_rect_t tRect)
{
	xge_svg_draw_item_t* pItem;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pStyle == NULL) || !__xgeSvgValid(pChildSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_OK;
	}
	iRet = __xgeSvgDrawItemReserve(pSvg);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pItem = &pSvg->pItems[pSvg->iItemCount];
	memset(pItem, 0, sizeof(*pItem));
	pItem->iType = XGE_SVG_DRAW_ITEM_SVG_IMAGE;
	xgeSvgAddRef(pChildSvg);
	pItem->u.tImage.pSvg = pChildSvg;
	pItem->u.tImage.tStyle = *pStyle;
	pItem->u.tImage.tRect = tRect;
	pSvg->iItemCount++;
	return XGE_OK;
}

static int __xgeSvgAddSvgImage(xge_svg pSvg, const xge_svg_style_t* pStyle, xge_svg pChildSvg, xge_rect_t tRect, int iDefIndex)
{
	xge_svg_image_item_t tImage;

	if ( !__xgeSvgValid(pSvg) || (pStyle == NULL) || !__xgeSvgValid(pChildSvg) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_OK;
	}
	if ( iDefIndex >= pSvg->iDefCount ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tImage, 0, sizeof(tImage));
	tImage.pSvg = pChildSvg;
	tImage.tStyle = *pStyle;
	tImage.tRect = tRect;
	return __xgeSvgAddImageItemMasked(pSvg, &tImage, iDefIndex);
}

static int __xgeSvgAddRasterImage(xge_svg pSvg, const xge_svg_style_t* pStyle, xge_svg_raster_image_t* pRaster, xge_rect_t tRect, const char* sAspect, int iDefIndex)
{
	xge_svg_raster_item_t tImage;

	if ( !__xgeSvgValid(pSvg) || (pStyle == NULL) || (pRaster == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_OK;
	}
	if ( iDefIndex >= pSvg->iDefCount ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tImage, 0, sizeof(tImage));
	tImage.pRaster = pRaster;
	tImage.tStyle = *pStyle;
	tImage.tRect = tRect;
	tImage.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tImage.iAspectAlignX = XGE_SVG_ASPECT_ALIGN_MID;
	tImage.iAspectAlignY = XGE_SVG_ASPECT_ALIGN_MID;
	tImage.iAspectMeetOrSlice = XGE_SVG_ASPECT_MEET;
	if ( (sAspect != NULL) && (sAspect[0] != '\0') ) {
		__xgeSvgParsePreserveAspectRatioFields(sAspect, &tImage.iAspectAlignX, &tImage.iAspectAlignY, &tImage.iAspectMeetOrSlice);
	}
	return __xgeSvgAddRasterItemMasked(pSvg, &tImage, iDefIndex);
}

static int __xgeSvgParseImage(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_style_t* pParentStyle, int bInDefs, int iCurrentDef, int iGroupMainIdDefIndex)
{
	char sHref[XGE_SVG_ATTR_MAX];
	char sAspect[XGE_SVG_ATTR_MAX];
	char* sSvgText;
	unsigned char* pImageData;
	xge_svg pChildSvg;
	xge_svg_raster_image_t* pRaster;
	xge_svg_style_t tStyle;
	xge_rect_t tRect;
	int iSize;
	int iTargetDef;
	int iOwnMainIdDefIndex;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pParentStyle == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iTargetDef = __xgeSvgShapeTargetDef(pSvg, pTag, pTagEnd, bInDefs, iCurrentDef, &iRet);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( iTargetDef == -2 ) {
		return XGE_OK;
	}
	iRet = __xgeSvgMainIdDefIndex(pSvg, pTag, pTagEnd, bInDefs, iCurrentDef, &iOwnMainIdDefIndex);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "href", sHref, sizeof(sHref)) &&
	     !__xgeSvgAttrCopy(pTag, pTagEnd, "xlink:href", sHref, sizeof(sHref)) ) {
		return XGE_OK;
	}
	sAspect[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "preserveAspectRatio", sAspect, sizeof(sAspect));
	tStyle = __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd);
	tRect.fX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
	tRect.fY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
	tRect.fW = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
	tRect.fH = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
	sSvgText = NULL;
	iSize = 0;
	iRet = __xgeSvgDecodeSvgDataUri(sHref, &sSvgText, &iSize);
	if ( iRet == XGE_ERROR_UNSUPPORTED ) {
		pImageData = NULL;
		pRaster = NULL;
		iSize = 0;
		iRet = __xgeSvgDecodeRasterImageDataUri(sHref, &pImageData, &iSize);
		if ( iRet == XGE_ERROR_UNSUPPORTED ) {
			pChildSvg = NULL;
			pRaster = NULL;
			iRet = __xgeSvgLoadExternalImage(pSvg, sHref, &pChildSvg, &pRaster);
			if ( (iRet == XGE_OK) && (pRaster != NULL) ) {
				iRet = __xgeSvgAddRasterImage(pSvg, &tStyle, pRaster, tRect, sAspect, iTargetDef);
				if ( (iRet == XGE_OK) && (iTargetDef < 0) && (iOwnMainIdDefIndex >= 0) ) {
					iRet = __xgeSvgAddRasterImage(pSvg, &tStyle, pRaster, tRect, sAspect, iOwnMainIdDefIndex);
				}
				if ( (iRet == XGE_OK) && (iTargetDef < 0) && (iGroupMainIdDefIndex >= 0) && (iGroupMainIdDefIndex != iOwnMainIdDefIndex) ) {
					iRet = __xgeSvgAddRasterImage(pSvg, &tStyle, pRaster, tRect, sAspect, iGroupMainIdDefIndex);
				}
			} else if ( (iRet == XGE_OK) && (pChildSvg != NULL) ) {
				if ( sAspect[0] != '\0' ) {
					iRet = xgeSvgSetPreserveAspectRatio(pChildSvg, sAspect);
				}
				if ( iRet == XGE_OK ) {
					iRet = __xgeSvgAddSvgImage(pSvg, &tStyle, pChildSvg, tRect, iTargetDef);
				}
				if ( (iRet == XGE_OK) && (iTargetDef < 0) && (iOwnMainIdDefIndex >= 0) ) {
					iRet = __xgeSvgAddSvgImage(pSvg, &tStyle, pChildSvg, tRect, iOwnMainIdDefIndex);
				}
				if ( (iRet == XGE_OK) && (iTargetDef < 0) && (iGroupMainIdDefIndex >= 0) && (iGroupMainIdDefIndex != iOwnMainIdDefIndex) ) {
					iRet = __xgeSvgAddSvgImage(pSvg, &tStyle, pChildSvg, tRect, iGroupMainIdDefIndex);
				}
			}
			__xgeSvgRasterImageDestroy(pRaster);
			xgeSvgDestroy(pChildSvg);
			return iRet;
		}
		if ( iRet == XGE_OK ) {
			iRet = __xgeSvgRasterImageCreateFromMemory(pImageData, iSize, &pRaster);
		}
		if ( iRet == XGE_OK ) {
			iRet = __xgeSvgAddRasterImage(pSvg, &tStyle, pRaster, tRect, sAspect, iTargetDef);
		}
		if ( (iRet == XGE_OK) && (iTargetDef < 0) && (iOwnMainIdDefIndex >= 0) ) {
			iRet = __xgeSvgAddRasterImage(pSvg, &tStyle, pRaster, tRect, sAspect, iOwnMainIdDefIndex);
		}
		if ( (iRet == XGE_OK) && (iTargetDef < 0) && (iGroupMainIdDefIndex >= 0) && (iGroupMainIdDefIndex != iOwnMainIdDefIndex) ) {
			iRet = __xgeSvgAddRasterImage(pSvg, &tStyle, pRaster, tRect, sAspect, iGroupMainIdDefIndex);
		}
		__xgeSvgRasterImageDestroy(pRaster);
		xrtFree(pImageData);
		return iRet;
	}
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pChildSvg = NULL;
	iRet = xgeSvgCreate(&pChildSvg);
	if ( iRet == XGE_OK ) {
		iRet = __xgeSvgLoadMemoryEx(pChildSvg, sSvgText, iSize, pSvg->sBaseDir);
	}
	if ( (iRet == XGE_OK) && (sAspect[0] != '\0') ) {
		iRet = xgeSvgSetPreserveAspectRatio(pChildSvg, sAspect);
	}
	if ( iRet == XGE_OK ) {
		iRet = __xgeSvgAddSvgImage(pSvg, &tStyle, pChildSvg, tRect, iTargetDef);
	}
	if ( (iRet == XGE_OK) && (iTargetDef < 0) && (iOwnMainIdDefIndex >= 0) ) {
		iRet = __xgeSvgAddSvgImage(pSvg, &tStyle, pChildSvg, tRect, iOwnMainIdDefIndex);
	}
	if ( (iRet == XGE_OK) && (iTargetDef < 0) && (iGroupMainIdDefIndex >= 0) && (iGroupMainIdDefIndex != iOwnMainIdDefIndex) ) {
		iRet = __xgeSvgAddSvgImage(pSvg, &tStyle, pChildSvg, tRect, iGroupMainIdDefIndex);
	}
	xgeSvgDestroy(pChildSvg);
	xrtFree(sSvgText);
	return iRet;
}

static int __xgeSvgParseElement(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_style_t* pParentStyle, int bInDefs, int iCurrentDef, int iGroupMainIdDefIndex)
{
	char sValue[XGE_SVG_ATTR_MAX];
	xge_svg_style_t tStyle;
	xge_shape_ex pShape;
	int iTargetDef;
	int iOwnMainIdDefIndex;
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
	iRet = __xgeSvgMainIdDefIndex(pSvg, pTag, pTagEnd, bInDefs, iCurrentDef, &iOwnMainIdDefIndex);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tStyle = __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd);
	if ( __xgeSvgTagNameEquals(pTag, "path") ) {
		xge_svg_path_marker_data_t tMarkerData;

		if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "d", sValue, sizeof(sValue)) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = xgeShapeExAppendSvgPath(pShape, sValue);
		if ( iRet == XGE_OK ) {
			__xgeSvgApplyPathLengthToStyle(pShape, pTag, pTagEnd, &tStyle);
			__xgeSvgApplyShapeStyle(pSvg, pShape, &tStyle);
			tMarkerData.pShape = pShape;
			iRet = __xgeSvgAddMarkedShapeWithMainIds(pSvg, pShape, &tStyle, iTargetDef, iOwnMainIdDefIndex, iGroupMainIdDefIndex, __xgeSvgApplyPathMarkersFromData, &tMarkerData);
			return iRet;
		}
		xgeShapeExDestroy(pShape);
		return iRet;
	}
	if ( __xgeSvgTagNameEquals(pTag, "rect") ) {
		float fX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		float fW = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fH = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		float fRX = 0.0f;
		float fRY = 0.0f;
		int bHasRX = __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "rx", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_X), tStyle.fFontSize, &fRX);
		int bHasRY = __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "ry", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_Y), tStyle.fFontSize, &fRY);

		if ( bHasRX && !bHasRY ) {
			fRY = fRX;
		} else if ( !bHasRX && bHasRY ) {
			fRX = fRY;
		}
		if ( (fW <= 0.0f) || (fH <= 0.0f) ) {
			return XGE_OK;
		}

		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = xgeShapeExAppendRect(pShape, fX, fY, fW, fH, fRX, fRY, 1);
		if ( iRet == XGE_OK ) {
			__xgeSvgApplyPathLengthToStyle(pShape, pTag, pTagEnd, &tStyle);
			__xgeSvgApplyShapeStyle(pSvg, pShape, &tStyle);
			return __xgeSvgAddStyledShapeWithMainIds(pSvg, pShape, &tStyle, iTargetDef, iOwnMainIdDefIndex, iGroupMainIdDefIndex);
		}
		xgeShapeExDestroy(pShape);
		return iRet;
	}
	if ( __xgeSvgTagNameEquals(pTag, "circle") || __xgeSvgTagNameEquals(pTag, "ellipse") ) {
		float fCX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "cx", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fCY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "cy", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		float fR = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "r", XGE_SVG_LENGTH_BASIS_OTHER, tStyle.fFontSize, 0.0f);
		float fRX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "rx", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, fR);
		float fRY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "ry", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, fR);

		if ( (fRX <= 0.0f) || (fRY <= 0.0f) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = xgeShapeExAppendCircle(pShape, fCX, fCY, fRX, fRY, 1);
		if ( iRet == XGE_OK ) {
			__xgeSvgApplyPathLengthToStyle(pShape, pTag, pTagEnd, &tStyle);
			__xgeSvgApplyShapeStyle(pSvg, pShape, &tStyle);
			return __xgeSvgAddStyledShapeWithMainIds(pSvg, pShape, &tStyle, iTargetDef, iOwnMainIdDefIndex, iGroupMainIdDefIndex);
		}
		xgeShapeExDestroy(pShape);
		return iRet;
	}
	if ( __xgeSvgTagNameEquals(pTag, "line") ) {
		float fX1 = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x1", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fY1 = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y1", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		float fX2 = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x2", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fY2 = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y2", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		xge_svg_line_marker_data_t tMarkerData;

		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		xgeShapeExMoveTo(pShape, fX1, fY1);
		xgeShapeExLineTo(pShape, fX2, fY2);
		__xgeSvgApplyPathLengthToStyle(pShape, pTag, pTagEnd, &tStyle);
		__xgeSvgApplyShapeStyle(pSvg, pShape, &tStyle);
		tMarkerData.tA.fX = fX1;
		tMarkerData.tA.fY = fY1;
		tMarkerData.tB.fX = fX2;
		tMarkerData.tB.fY = fY2;
		return __xgeSvgAddMarkedShapeWithMainIds(pSvg, pShape, &tStyle, iTargetDef, iOwnMainIdDefIndex, iGroupMainIdDefIndex, __xgeSvgApplyLineMarkersFromData, &tMarkerData);
	}
	if ( __xgeSvgTagNameEquals(pTag, "polygon") || __xgeSvgTagNameEquals(pTag, "polyline") ) {
		int bClose = __xgeSvgTagNameEquals(pTag, "polygon");
		xge_vec2_t* pPoints;
		int iPointCount;

		if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "points", sValue, sizeof(sValue)) ) {
			return XGE_OK;
		}
		pPoints = NULL;
		iPointCount = 0;
		iRet = __xgeSvgParsePointList(sValue, &pPoints, &iPointCount);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( iPointCount <= 0 ) {
			xrtFree(pPoints);
			return XGE_OK;
		}
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) {
			xrtFree(pPoints);
			return iRet;
		}
		iRet = __xgeSvgAppendPointList(pShape, pPoints, iPointCount, bClose);
		if ( iRet == XGE_OK ) {
			xge_svg_point_list_marker_data_t tMarkerData;

			__xgeSvgApplyPathLengthToStyle(pShape, pTag, pTagEnd, &tStyle);
			__xgeSvgApplyShapeStyle(pSvg, pShape, &tStyle);
			tMarkerData.pPoints = pPoints;
			tMarkerData.iCount = iPointCount;
			tMarkerData.bClosed = bClose;
			iRet = __xgeSvgAddMarkedShapeWithMainIds(pSvg, pShape, &tStyle, iTargetDef, iOwnMainIdDefIndex, iGroupMainIdDefIndex, __xgeSvgApplyPointListMarkersFromData, &tMarkerData);
			pShape = NULL;
		}
		if ( (iRet != XGE_OK) && (pShape != NULL) ) {
			xgeShapeExDestroy(pShape);
		}
		xrtFree(pPoints);
		return iRet;
	}
	return XGE_OK;
}

static void __xgeSvgApplyUsePaintToShape(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pSourceStyle, const xge_svg_style_t* pUseStyle, xge_svg_style_t* pAddStyle)
{
	if ( pAddStyle != NULL ) {
		pAddStyle->sFillGradientId[0] = '\0';
		pAddStyle->sStrokeGradientId[0] = '\0';
	}
	if ( !__xgeSvgValid(pSvg) || (pShape == NULL) || (pSourceStyle == NULL) || (pUseStyle == NULL) || (pAddStyle == NULL) ) {
		return;
	}
	if ( !pSourceStyle->bFillSet && pUseStyle->bFillSet ) {
		if ( pUseStyle->sFillGradientId[0] == '\0' ) {
			xgeShapeExFillColor(pShape, __xgeSvgColorAlpha(pUseStyle->iFillColor, pUseStyle->fFillOpacity));
		} else if ( __xgeSvgPatternGetForPaint(pSvg, pUseStyle->sFillGradientId) != NULL ) {
			strcpy(pAddStyle->sFillGradientId, pUseStyle->sFillGradientId);
		} else if ( !__xgeSvgApplyFillGradientPaintToShape(pSvg, pShape, pUseStyle, pUseStyle->fFillOpacity) ) {
			xgeShapeExFillColor(pShape, __xgeSvgColorAlpha(pUseStyle->iFillColor, pUseStyle->fFillOpacity));
		}
	} else if ( pSourceStyle->bFillCurrentColor && !pSourceStyle->bColorSet && pUseStyle->bColorSet ) {
		xgeShapeExFillColor(pShape, __xgeSvgColorAlpha(pUseStyle->iCurrentColor, pSourceStyle->fFillOpacity));
	}
	if ( !pSourceStyle->bStrokeSet && pUseStyle->bStrokeSet ) {
		xgeShapeExStrokeWidth(pShape, pUseStyle->fStrokeWidth);
		if ( pUseStyle->sStrokeGradientId[0] == '\0' ) {
			xgeShapeExStrokeColor(pShape, __xgeSvgColorAlpha(pUseStyle->iStrokeColor, pUseStyle->fStrokeOpacity));
		} else if ( __xgeSvgPatternGetForPaint(pSvg, pUseStyle->sStrokeGradientId) != NULL ) {
			strcpy(pAddStyle->sStrokeGradientId, pUseStyle->sStrokeGradientId);
		} else if ( !__xgeSvgApplyStrokeGradientPaintToShape(pSvg, pShape, pUseStyle, pUseStyle->fStrokeOpacity) ) {
			xgeShapeExStrokeColor(pShape, __xgeSvgColorAlpha(pUseStyle->iStrokeColor, pUseStyle->fStrokeOpacity));
		}
	} else if ( pSourceStyle->bStrokeCurrentColor && !pSourceStyle->bColorSet && pUseStyle->bColorSet ) {
		xgeShapeExStrokeColor(pShape, __xgeSvgColorAlpha(pUseStyle->iCurrentColor, pSourceStyle->fStrokeOpacity));
	}
}

static void __xgeSvgApplyUsePaintToStyle(xge_svg_style_t* pTargetStyle, const xge_svg_style_t* pUseStyle)
{
	if ( (pTargetStyle == NULL) || (pUseStyle == NULL) ) {
		return;
	}
	if ( !pTargetStyle->bFillSet && pUseStyle->bFillSet && (pUseStyle->sFillGradientId[0] == '\0') ) {
		pTargetStyle->iFillColor = pUseStyle->iFillColor;
		pTargetStyle->sFillGradientId[0] = '\0';
		pTargetStyle->fFillOpacity = pUseStyle->fFillOpacity;
		pTargetStyle->bFillSet = 1;
		pTargetStyle->bFillCurrentColor = pUseStyle->bFillCurrentColor;
	} else if ( pTargetStyle->bFillCurrentColor && !pTargetStyle->bColorSet && pUseStyle->bColorSet ) {
		pTargetStyle->iFillColor = pUseStyle->iCurrentColor;
	}
	if ( !pTargetStyle->bStrokeSet && pUseStyle->bStrokeSet && (pUseStyle->sStrokeGradientId[0] == '\0') ) {
		pTargetStyle->iStrokeColor = pUseStyle->iStrokeColor;
		pTargetStyle->sStrokeGradientId[0] = '\0';
		pTargetStyle->fStrokeOpacity = pUseStyle->fStrokeOpacity;
		pTargetStyle->fStrokeWidth = pUseStyle->fStrokeWidth;
		pTargetStyle->bStrokeSet = 1;
		pTargetStyle->bStrokeCurrentColor = pUseStyle->bStrokeCurrentColor;
	} else if ( pTargetStyle->bStrokeCurrentColor && !pTargetStyle->bColorSet && pUseStyle->bColorSet ) {
		pTargetStyle->iStrokeColor = pUseStyle->iCurrentColor;
	}
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
		tStyle.tTransform = __xgeSvgMatrixMul(tStyle.tTransform, tUseTransform);
	} else if ( (fabsf(fX) > 0.0f) || (fabsf(fY) > 0.0f) ) {
		tUseTransform = __xgeSvgMatrixTranslate(fX, fY);
		tStyle.tTransform = __xgeSvgMatrixMul(tStyle.tTransform, tUseTransform);
	}
	iSourceShapeCount = pSvg->pDefs[iDefIndex].iShapeCount;
	for ( i = 0; i < iSourceShapeCount; i++ ) {
		xge_shape_ex pClone;
		xge_shape_ex_matrix_t tShapeTransform;
		const xge_svg_style_t* pSourceStyle;
		xge_svg_style_t tItemStyle;

		iRet = xgeShapeExClone(pSvg->pDefs[iDefIndex].pShapes[i], &pClone);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		pSourceStyle = (pSvg->pDefs[iDefIndex].pShapeStyles != NULL) ? &pSvg->pDefs[iDefIndex].pShapeStyles[i] : NULL;
		tItemStyle = tStyle;
		__xgeSvgApplyUsePaintToShape(pSvg, pClone, pSourceStyle, pUseStyle, &tItemStyle);
		tShapeTransform = __xgeSvgMatrixMul(tItemStyle.tTransform, pClone->tTransform);
		xgeShapeExTransformSet(pClone, &tShapeTransform);
		xgeShapeExOpacity(pClone, pClone->fOpacity * tItemStyle.fOpacity);
		if ( !tItemStyle.bVisible || !tItemStyle.bVisibility ) {
			xgeShapeExVisible(pClone, 0);
		}
		iRet = __xgeSvgAddStyledShape(pSvg, pClone, &tItemStyle, iTargetDef);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	for ( i = 0; i < pSvg->pDefs[iDefIndex].iTextCount; i++ ) {
		xge_svg_text_item_t tText;

		tText = pSvg->pDefs[iDefIndex].pTexts[i];
		__xgeSvgApplyUsePaintToStyle(&tText.tStyle, pUseStyle);
		tText.tStyle.tTransform = __xgeSvgMatrixMul(tStyle.tTransform, tText.tStyle.tTransform);
		tText.tStyle.fOpacity *= tStyle.fOpacity;
		if ( !tStyle.bVisible || !tStyle.bVisibility ) {
			tText.tStyle.bVisible = 0;
		}
		iRet = __xgeSvgAddTextItemMasked(pSvg, &tText, iTargetDef);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	for ( i = 0; i < pSvg->pDefs[iDefIndex].iImageCount; i++ ) {
		xge_svg_image_item_t tImage;

		tImage = pSvg->pDefs[iDefIndex].pImages[i];
		tImage.tStyle.tTransform = __xgeSvgMatrixMul(tStyle.tTransform, tImage.tStyle.tTransform);
		tImage.tStyle.fOpacity *= tStyle.fOpacity;
		if ( !tStyle.bVisible || !tStyle.bVisibility ) {
			tImage.tStyle.bVisible = 0;
		}
		iRet = __xgeSvgAddImageItemMasked(pSvg, &tImage, iTargetDef);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	for ( i = 0; i < pSvg->pDefs[iDefIndex].iRasterCount; i++ ) {
		xge_svg_raster_item_t tImage;

		tImage = pSvg->pDefs[iDefIndex].pRasters[i];
		tImage.tStyle.tTransform = __xgeSvgMatrixMul(tStyle.tTransform, tImage.tStyle.tTransform);
		tImage.tStyle.fOpacity *= tStyle.fOpacity;
		if ( !tStyle.bVisible || !tStyle.bVisibility ) {
			tImage.tStyle.bVisible = 0;
		}
		iRet = __xgeSvgAddRasterItemMasked(pSvg, &tImage, iTargetDef);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeSvgApplyUseInstanceWithMainIds(xge_svg pSvg, int iDefIndex, const xge_svg_style_t* pUseStyle, int iTargetDef, int iMainIdDefIndex, int iExtraMainIdDefIndex, float fX, float fY, float fW, float fH, int bHasWidth, int bHasHeight);

static int __xgeSvgApplyUseInstanceWithMainId(xge_svg pSvg, int iDefIndex, const xge_svg_style_t* pUseStyle, int iTargetDef, int iMainIdDefIndex, float fX, float fY, float fW, float fH, int bHasWidth, int bHasHeight)
{
	return __xgeSvgApplyUseInstanceWithMainIds(pSvg, iDefIndex, pUseStyle, iTargetDef, iMainIdDefIndex, -1, fX, fY, fW, fH, bHasWidth, bHasHeight);
}

static int __xgeSvgApplyUseInstanceWithMainIds(xge_svg pSvg, int iDefIndex, const xge_svg_style_t* pUseStyle, int iTargetDef, int iMainIdDefIndex, int iExtraMainIdDefIndex, float fX, float fY, float fW, float fH, int bHasWidth, int bHasHeight)
{
	int iRet;

	iRet = __xgeSvgApplyUseInstance(pSvg, iDefIndex, pUseStyle, iTargetDef, fX, fY, fW, fH, bHasWidth, bHasHeight);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( (iTargetDef < 0) && (iMainIdDefIndex >= 0) ) {
		iRet = __xgeSvgApplyUseInstance(pSvg, iDefIndex, pUseStyle, iMainIdDefIndex, fX, fY, fW, fH, bHasWidth, bHasHeight);
	}
	if ( (iRet == XGE_OK) && (iTargetDef < 0) && (iExtraMainIdDefIndex >= 0) && (iExtraMainIdDefIndex != iMainIdDefIndex) ) {
		iRet = __xgeSvgApplyUseInstance(pSvg, iDefIndex, pUseStyle, iExtraMainIdDefIndex, fX, fY, fW, fH, bHasWidth, bHasHeight);
	}
	return iRet;
}

static int __xgeSvgAddSvgImageToUseTargets(xge_svg pSvg, const xge_svg_style_t* pStyle, xge_svg pChildSvg, xge_rect_t tRect, int iTargetDef, int iOwnMainIdDefIndex, int iGroupMainIdDefIndex)
{
	int iRet;

	iRet = __xgeSvgAddSvgImage(pSvg, pStyle, pChildSvg, tRect, iTargetDef);
	if ( (iRet == XGE_OK) && (iTargetDef < 0) && (iOwnMainIdDefIndex >= 0) ) {
		iRet = __xgeSvgAddSvgImage(pSvg, pStyle, pChildSvg, tRect, iOwnMainIdDefIndex);
	}
	if ( (iRet == XGE_OK) && (iTargetDef < 0) && (iGroupMainIdDefIndex >= 0) && (iGroupMainIdDefIndex != iOwnMainIdDefIndex) ) {
		iRet = __xgeSvgAddSvgImage(pSvg, pStyle, pChildSvg, tRect, iGroupMainIdDefIndex);
	}
	return iRet;
}

static int __xgeSvgApplyExternalSvgUseRoot(xge_svg pSvg, const char* sHref, const char* pTag, const char* pTagEnd, const xge_svg_style_t* pUseStyle, int iTargetDef, int iOwnMainIdDefIndex, int iGroupMainIdDefIndex, float fX, float fY, float fW, float fH, int bHasWidth, int bHasHeight)
{
	char sAspect[XGE_SVG_ATTR_MAX];
	char* sSvgText;
	xge_svg pChildSvg;
	xge_svg_raster_image_t* pRaster;
	xge_rect_t tRect;
	int iSize;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (sHref == NULL) || (pTag == NULL) || (pTagEnd == NULL) || (pUseStyle == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sAspect[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "preserveAspectRatio", sAspect, sizeof(sAspect));
	sSvgText = NULL;
	pChildSvg = NULL;
	pRaster = NULL;
	iSize = 0;
	iRet = __xgeSvgDecodeSvgDataUri(sHref, &sSvgText, &iSize);
	if ( iRet == XGE_OK ) {
		iRet = xgeSvgCreate(&pChildSvg);
		if ( iRet == XGE_OK ) {
			iRet = __xgeSvgLoadMemoryEx(pChildSvg, sSvgText, iSize, pSvg->sBaseDir);
		}
		xrtFree(sSvgText);
	} else if ( iRet == XGE_ERROR_UNSUPPORTED ) {
		iRet = __xgeSvgLoadExternalImage(pSvg, sHref, &pChildSvg, &pRaster);
		__xgeSvgRasterImageDestroy(pRaster);
		pRaster = NULL;
	}
	if ( iRet != XGE_OK ) {
		xgeSvgDestroy(pChildSvg);
		return iRet;
	}
	if ( pChildSvg == NULL ) {
		return XGE_OK;
	}
	if ( sAspect[0] != '\0' ) {
		iRet = xgeSvgSetPreserveAspectRatio(pChildSvg, sAspect);
		if ( iRet != XGE_OK ) {
			xgeSvgDestroy(pChildSvg);
			return iRet;
		}
	}
	if ( !bHasWidth || (fW <= 0.0f) ) {
		if ( pChildSvg->fWidth > 0.0f ) {
			fW = pChildSvg->fWidth;
		} else if ( pChildSvg->bHasViewBox ) {
			fW = pChildSvg->tViewBox.fW;
		}
	}
	if ( !bHasHeight || (fH <= 0.0f) ) {
		if ( pChildSvg->fHeight > 0.0f ) {
			fH = pChildSvg->fHeight;
		} else if ( pChildSvg->bHasViewBox ) {
			fH = pChildSvg->tViewBox.fH;
		}
	}
	tRect.fX = fX;
	tRect.fY = fY;
	tRect.fW = fW;
	tRect.fH = fH;
	iRet = __xgeSvgAddSvgImageToUseTargets(pSvg, pUseStyle, pChildSvg, tRect, iTargetDef, iOwnMainIdDefIndex, iGroupMainIdDefIndex);
	xgeSvgDestroy(pChildSvg);
	return iRet;
}

static xge_rect_t __xgeSvgChildIntrinsicRect(xge_svg pChildSvg)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( !__xgeSvgValid(pChildSvg) ) {
		return tRect;
	}
	if ( pChildSvg->bHasViewBox && (pChildSvg->tViewBox.fW > 0.0f) && (pChildSvg->tViewBox.fH > 0.0f) ) {
		return pChildSvg->tViewBox;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = pChildSvg->fWidth;
	tRect.fH = pChildSvg->fHeight;
	if ( tRect.fW <= 0.0f ) tRect.fW = 1.0f;
	if ( tRect.fH <= 0.0f ) tRect.fH = 1.0f;
	return tRect;
}

static xge_svg_style_t __xgeSvgExternalUseInheritedStyle(const xge_svg_style_t* pUseStyle)
{
	xge_svg_style_t tStyle;

	tStyle = __xgeSvgStyleDefault();
	if ( pUseStyle == NULL ) {
		return tStyle;
	}
	if ( pUseStyle->bColorSet ) {
		tStyle.iCurrentColor = pUseStyle->iCurrentColor;
		tStyle.bColorSet = 1;
	}
	if ( pUseStyle->bFillSet && (pUseStyle->sFillGradientId[0] == '\0') ) {
		tStyle.iFillColor = pUseStyle->iFillColor;
		tStyle.fFillOpacity = pUseStyle->fFillOpacity;
		tStyle.bFillSet = 1;
		tStyle.bFillCurrentColor = pUseStyle->bFillCurrentColor;
	}
	if ( pUseStyle->bStrokeSet && (pUseStyle->sStrokeGradientId[0] == '\0') ) {
		tStyle.iStrokeColor = pUseStyle->iStrokeColor;
		tStyle.fStrokeOpacity = pUseStyle->fStrokeOpacity;
		tStyle.fStrokeWidth = pUseStyle->fStrokeWidth;
		tStyle.bStrokeSet = 1;
		tStyle.bStrokeCurrentColor = pUseStyle->bStrokeCurrentColor;
	}
	return tStyle;
}

static int __xgeSvgApplyExternalSvgUseFragment(xge_svg pSvg, const char* sHref, const char* pTag, const char* pTagEnd, const xge_svg_style_t* pUseStyle, int iTargetDef, int iOwnMainIdDefIndex, int iGroupMainIdDefIndex, float fX, float fY, float fW, float fH, int bHasWidth, int bHasHeight)
{
	char sUri[XGE_SVG_ATTR_MAX];
	char sId[XGE_SVG_ATTR_MAX];
	char sAspect[XGE_SVG_ATTR_MAX];
	xge_svg pChildSvg;
	xge_svg_style_t tFragmentStyle;
	xge_rect_t tIntrinsic;
	xge_rect_t tRect;
	int iDefIndex;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (sHref == NULL) || (pTag == NULL) || (pTagEnd == NULL) || (pUseStyle == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeSvgHrefSplitExternalFragment(sHref, sUri, sizeof(sUri), sId, sizeof(sId)) ) {
		return XGE_OK;
	}
	pChildSvg = NULL;
	iRet = __xgeSvgLoadExternalSvgDocument(pSvg, sUri, &pChildSvg);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( pChildSvg == NULL ) {
		return XGE_OK;
	}
	iDefIndex = __xgeSvgDefFind(pChildSvg, sId);
	if ( iDefIndex < 0 ) {
		xgeSvgDestroy(pChildSvg);
		return XGE_OK;
	}
	sAspect[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "preserveAspectRatio", sAspect, sizeof(sAspect));
	if ( sAspect[0] != '\0' ) {
		iRet = xgeSvgSetPreserveAspectRatio(pChildSvg, sAspect);
		if ( iRet != XGE_OK ) {
			xgeSvgDestroy(pChildSvg);
			return iRet;
		}
	}
	tIntrinsic = __xgeSvgChildIntrinsicRect(pChildSvg);
	if ( pChildSvg->pDefs[iDefIndex].bHasViewBox ) {
		tIntrinsic = pChildSvg->pDefs[iDefIndex].tViewBox;
	}
	pChildSvg->tViewBox = tIntrinsic;
	pChildSvg->bHasViewBox = ((tIntrinsic.fW > 0.0f) && (tIntrinsic.fH > 0.0f)) ? 1 : 0;
	pChildSvg->fWidth = tIntrinsic.fW;
	pChildSvg->fHeight = tIntrinsic.fH;
	if ( !bHasWidth || (fW <= 0.0f) ) {
		fW = tIntrinsic.fW;
	}
	if ( !bHasHeight || (fH <= 0.0f) ) {
		fH = tIntrinsic.fH;
	}
	__xgeSvgDrawItemsClear(pChildSvg);
	tFragmentStyle = __xgeSvgExternalUseInheritedStyle(pUseStyle);
	if ( pChildSvg->pDefs[iDefIndex].bHasViewBox ) {
		iRet = __xgeSvgApplyUseInstance(pChildSvg, iDefIndex, &tFragmentStyle, -1, tIntrinsic.fX, tIntrinsic.fY, tIntrinsic.fW, tIntrinsic.fH, 1, 1);
	} else {
		iRet = __xgeSvgApplyUseInstance(pChildSvg, iDefIndex, &tFragmentStyle, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0);
	}
	if ( iRet != XGE_OK ) {
		xgeSvgDestroy(pChildSvg);
		return iRet;
	}
	tRect.fX = fX;
	tRect.fY = fY;
	tRect.fW = fW;
	tRect.fH = fH;
	iRet = __xgeSvgAddSvgImageToUseTargets(pSvg, pUseStyle, pChildSvg, tRect, iTargetDef, iOwnMainIdDefIndex, iGroupMainIdDefIndex);
	xgeSvgDestroy(pChildSvg);
	return iRet;
}

static int __xgeSvgParseUse(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_style_t* pParentStyle, int bInDefs, int iCurrentDef, int iGroupMainIdDefIndex)
{
	char sHref[XGE_SVG_ATTR_MAX];
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
	int iOwnMainIdDefIndex;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pParentStyle == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeSvgHrefCopyTrimmed(pTag, pTagEnd, sHref, sizeof(sHref)) ) {
		return XGE_OK;
	}
	iTargetDef = __xgeSvgShapeTargetDef(pSvg, pTag, pTagEnd, bInDefs, iCurrentDef, &iRet);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( iTargetDef == -2 ) {
		return XGE_OK;
	}
	iRet = __xgeSvgMainIdDefIndex(pSvg, pTag, pTagEnd, bInDefs, iCurrentDef, &iOwnMainIdDefIndex);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tStyle = __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd);
	fX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
	fY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
	fW = 0.0f;
	fH = 0.0f;
	bHasWidth = __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "width", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_X), tStyle.fFontSize, &fW);
	bHasHeight = __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "height", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_Y), tStyle.fFontSize, &fH);
	if ( !__xgeSvgHrefIsLocalRef(sHref) ) {
		if ( __xgeSvgHrefHasFragment(sHref) ) {
			return __xgeSvgApplyExternalSvgUseFragment(pSvg, sHref, pTag, pTagEnd, &tStyle, iTargetDef, iOwnMainIdDefIndex, iGroupMainIdDefIndex, fX, fY, fW, fH, bHasWidth, bHasHeight);
		}
		if ( __xgeSvgHrefLooksExternalSvgRoot(sHref) ) {
			return __xgeSvgApplyExternalSvgUseRoot(pSvg, sHref, pTag, pTagEnd, &tStyle, iTargetDef, iOwnMainIdDefIndex, iGroupMainIdDefIndex, fX, fY, fW, fH, bHasWidth, bHasHeight);
		}
	}
	if ( !__xgeSvgHrefIdCopy(pTag, pTagEnd, sId, sizeof(sId)) ) {
		return XGE_OK;
	}
	iDefIndex = __xgeSvgDefFind(pSvg, sId);
	if ( iDefIndex < 0 ) {
		return __xgeSvgPendingUseAdd(pSvg, sId, &tStyle, iTargetDef, iOwnMainIdDefIndex, iGroupMainIdDefIndex, fX, fY, fW, fH, bHasWidth, bHasHeight);
	}
	return __xgeSvgApplyUseInstanceWithMainIds(pSvg, iDefIndex, &tStyle, iTargetDef, iOwnMainIdDefIndex, iGroupMainIdDefIndex, fX, fY, fW, fH, bHasWidth, bHasHeight);
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
		iRet = __xgeSvgApplyUseInstanceWithMainIds(pSvg, iSourceDef, &pUse->tStyle, pUse->iTargetDef, pUse->iMainIdDefIndex, pUse->iExtraMainIdDefIndex, pUse->fX, pUse->fY, pUse->fW, pUse->fH, pUse->bHasWidth, pUse->bHasHeight);
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
		iRet = __xgeSvgApplyUseInstanceWithMainIds(pSvg, iSourceDef, &pUse->tStyle, pUse->iTargetDef, pUse->iMainIdDefIndex, pUse->iExtraMainIdDefIndex, pUse->fX, pUse->fY, pUse->fW, pUse->fH, pUse->bHasWidth, pUse->bHasHeight);
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

static int __xgeSvgGradientLengthCopy(xge_svg pSvg, const char* pTag, const char* pTagEnd, const char* sName, int iUnits, int iBasis, float fFontSize, float* pValue)
{
	float fPercentRef;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (sName == NULL) || (pValue == NULL) ) {
		return 0;
	}
	fPercentRef = (iUnits == XGE_SHAPE_EX_GRADIENT_USER_SPACE) ? __xgeSvgLengthPercentRef(pSvg, iBasis) : 1.0f;
	return __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, sName, fPercentRef, fFontSize, pValue);
}

static int __xgeSvgPatternLengthCopy(xge_svg pSvg, const char* pTag, const char* pTagEnd, const char* sName, int iUnits, int iBasis, float fFontSize, float* pValue)
{
	float fPercentRef;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (sName == NULL) || (pValue == NULL) ) {
		return 0;
	}
	fPercentRef = (iUnits == XGE_SVG_PATTERN_USER_SPACE) ? __xgeSvgLengthPercentRef(pSvg, iBasis) : 1.0f;
	return __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, sName, fPercentRef, fFontSize, pValue);
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
	pSvg->pLinearGradients[iIndex].tTransform = __xgeSvgMatrixIdentity();
	pSvg->pLinearGradients[iIndex].iFlags = 0;
	pSvg->pLinearGradients[iIndex].iStopCount = 0;
	if ( __xgeSvgHrefIdCopy(pTag, pTagEnd, sValue, sizeof(sValue)) ) {
		pSvg->pLinearGradients[iIndex].sHrefId = __xgeStrDup(sValue);
		if ( pSvg->pLinearGradients[iIndex].sHrefId == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "gradientUnits", sValue, sizeof(sValue)) ) {
		pSvg->pLinearGradients[iIndex].iUnits = (strcmp(sValue, "userSpaceOnUse") == 0) ? XGE_SHAPE_EX_GRADIENT_USER_SPACE : XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX;
		pSvg->pLinearGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_UNITS;
	}
	if ( __xgeSvgGradientLengthCopy(pSvg, pTag, pTagEnd, "x1", pSvg->pLinearGradients[iIndex].iUnits, XGE_SVG_LENGTH_BASIS_X, pSvg->pLinearGradients[iIndex].tStyle.fFontSize, &pSvg->pLinearGradients[iIndex].fX1) ) {
		pSvg->pLinearGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_X1;
	}
	if ( __xgeSvgGradientLengthCopy(pSvg, pTag, pTagEnd, "y1", pSvg->pLinearGradients[iIndex].iUnits, XGE_SVG_LENGTH_BASIS_Y, pSvg->pLinearGradients[iIndex].tStyle.fFontSize, &pSvg->pLinearGradients[iIndex].fY1) ) {
		pSvg->pLinearGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_Y1;
	}
	if ( __xgeSvgGradientLengthCopy(pSvg, pTag, pTagEnd, "x2", pSvg->pLinearGradients[iIndex].iUnits, XGE_SVG_LENGTH_BASIS_X, pSvg->pLinearGradients[iIndex].tStyle.fFontSize, &pSvg->pLinearGradients[iIndex].fX2) ) {
		pSvg->pLinearGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_X2;
	}
	if ( __xgeSvgGradientLengthCopy(pSvg, pTag, pTagEnd, "y2", pSvg->pLinearGradients[iIndex].iUnits, XGE_SVG_LENGTH_BASIS_Y, pSvg->pLinearGradients[iIndex].tStyle.fFontSize, &pSvg->pLinearGradients[iIndex].fY2) ) {
		pSvg->pLinearGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_Y2;
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
	pSvg->pRadialGradients[iIndex].tTransform = __xgeSvgMatrixIdentity();
	pSvg->pRadialGradients[iIndex].iFlags = 0;
	pSvg->pRadialGradients[iIndex].iStopCount = 0;
	if ( __xgeSvgHrefIdCopy(pTag, pTagEnd, sValue, sizeof(sValue)) ) {
		pSvg->pRadialGradients[iIndex].sHrefId = __xgeStrDup(sValue);
		if ( pSvg->pRadialGradients[iIndex].sHrefId == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "gradientUnits", sValue, sizeof(sValue)) ) {
		pSvg->pRadialGradients[iIndex].iUnits = (strcmp(sValue, "userSpaceOnUse") == 0) ? XGE_SHAPE_EX_GRADIENT_USER_SPACE : XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX;
		pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_UNITS;
	}
	if ( __xgeSvgGradientLengthCopy(pSvg, pTag, pTagEnd, "cx", pSvg->pRadialGradients[iIndex].iUnits, XGE_SVG_LENGTH_BASIS_X, pSvg->pRadialGradients[iIndex].tStyle.fFontSize, &pSvg->pRadialGradients[iIndex].fCX) ) {
		pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_CX;
	}
	if ( __xgeSvgGradientLengthCopy(pSvg, pTag, pTagEnd, "cy", pSvg->pRadialGradients[iIndex].iUnits, XGE_SVG_LENGTH_BASIS_Y, pSvg->pRadialGradients[iIndex].tStyle.fFontSize, &pSvg->pRadialGradients[iIndex].fCY) ) {
		pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_CY;
	}
	if ( __xgeSvgGradientLengthCopy(pSvg, pTag, pTagEnd, "r", pSvg->pRadialGradients[iIndex].iUnits, XGE_SVG_LENGTH_BASIS_OTHER, pSvg->pRadialGradients[iIndex].tStyle.fFontSize, &pSvg->pRadialGradients[iIndex].fR) ) {
		pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_R;
	}
	if ( __xgeSvgGradientLengthCopy(pSvg, pTag, pTagEnd, "fx", pSvg->pRadialGradients[iIndex].iUnits, XGE_SVG_LENGTH_BASIS_X, pSvg->pRadialGradients[iIndex].tStyle.fFontSize, &pSvg->pRadialGradients[iIndex].fFX) ) {
		pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_FX;
	} else if ( pSvg->pRadialGradients[iIndex].sHrefId == NULL ) {
		pSvg->pRadialGradients[iIndex].fFX = pSvg->pRadialGradients[iIndex].fCX;
	}
	if ( __xgeSvgGradientLengthCopy(pSvg, pTag, pTagEnd, "fy", pSvg->pRadialGradients[iIndex].iUnits, XGE_SVG_LENGTH_BASIS_Y, pSvg->pRadialGradients[iIndex].tStyle.fFontSize, &pSvg->pRadialGradients[iIndex].fFY) ) {
		pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_FY;
	} else if ( pSvg->pRadialGradients[iIndex].sHrefId == NULL ) {
		pSvg->pRadialGradients[iIndex].fFY = pSvg->pRadialGradients[iIndex].fCY;
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
	char sValue[XGE_SVG_ATTR_MAX];

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pStop == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pStop->fOffset = __xgeSvgAttrCopy(pTag, pTagEnd, "offset", sValue, sizeof(sValue)) ? __xgeSvgParseStopOffset(sValue) : 0.0f;
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

static void __xgeSvgParseRoot(xge_svg pSvg, const char* pTag, const char* pTagEnd, float fFontSize)
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
	pSvg->fWidth = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, fFontSize, pSvg->bHasViewBox ? pSvg->tViewBox.fW : 0.0f);
	pSvg->fHeight = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, fFontSize, pSvg->bHasViewBox ? pSvg->tViewBox.fH : 0.0f);
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
	tDst.fX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, pStyle->fFontSize, 0.0f);
	tDst.fY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, pStyle->fFontSize, 0.0f);
	tDst.fW = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, pStyle->fFontSize, bHasViewBox ? tViewBox.fW : 0.0f);
	tDst.fH = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, pStyle->fFontSize, bHasViewBox ? tViewBox.fH : 0.0f);
	if ( bHasViewBox && (tDst.fW > 0.0f) && (tDst.fH > 0.0f) ) {
		iAlignX = XGE_SVG_ASPECT_ALIGN_MID;
		iAlignY = XGE_SVG_ASPECT_ALIGN_MID;
		iMeetOrSlice = XGE_SVG_ASPECT_MEET;
		if ( __xgeSvgAttrCopy(pTag, pTagEnd, "preserveAspectRatio", sValue, sizeof(sValue)) ) {
			__xgeSvgParsePreserveAspectRatioFields(sValue, &iAlignX, &iAlignY, &iMeetOrSlice);
		}
		tViewport = __xgeSvgAspectViewport(tViewBox, tDst, iAlignX, iAlignY, iMeetOrSlice);
		tViewportMatrix = __xgeSvgViewBoxMatrix(tViewBox, tViewport);
		pStyle->tTransform = __xgeSvgMatrixMul(pStyle->tTransform, tViewportMatrix);
	} else if ( (fabsf(tDst.fX) > 0.0f) || (fabsf(tDst.fY) > 0.0f) ) {
		tViewportMatrix = __xgeSvgMatrixTranslate(tDst.fX, tDst.fY);
		pStyle->tTransform = __xgeSvgMatrixMul(pStyle->tTransform, tViewportMatrix);
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
	} else if ( __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "width", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_X), 16.0f, &fW) &&
	            __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "height", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_Y), 16.0f, &fH) &&
	            (fW > 0.0f) && (fH > 0.0f) ) {
		pSvg->pDefs[iDefIndex].tViewBox.fX = 0.0f;
		pSvg->pDefs[iDefIndex].tViewBox.fY = 0.0f;
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

static int __xgeSvgParseMarker(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_style_t* pParentStyle, int* pDefIndex)
{
	char sId[XGE_SVG_ATTR_MAX];
	char sDefId[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	xge_svg_marker_t* pMarker;
	float fX;
	float fY;
	float fW;
	float fH;
	int iMarkerIndex;
	int iDefIndex;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pDefIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pDefIndex = -1;
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "id", sId, sizeof(sId)) || (sId[0] == '\0') ) {
		return XGE_OK;
	}
	{
		int iDefIdLen = snprintf(sDefId, sizeof(sDefId), "__marker__:%s", sId);
		if ( (iDefIdLen < 0) || (iDefIdLen >= (int)sizeof(sDefId)) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	iRet = __xgeSvgMarkerGetOrCreate(pSvg, sId, &iMarkerIndex);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeSvgDefGetOrCreate(pSvg, sDefId, &iDefIndex);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pMarker = &pSvg->pMarkers[iMarkerIndex];
	pMarker->iDefIndex = iDefIndex;
	pMarker->fRefX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "refX", XGE_SVG_LENGTH_BASIS_X, pParentStyle != NULL ? pParentStyle->fFontSize : 16.0f, 0.0f);
	pMarker->fRefY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "refY", XGE_SVG_LENGTH_BASIS_Y, pParentStyle != NULL ? pParentStyle->fFontSize : 16.0f, 0.0f);
	pMarker->fWidth = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "markerWidth", XGE_SVG_LENGTH_BASIS_X, pParentStyle != NULL ? pParentStyle->fFontSize : 16.0f, 3.0f);
	pMarker->fHeight = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "markerHeight", XGE_SVG_LENGTH_BASIS_Y, pParentStyle != NULL ? pParentStyle->fFontSize : 16.0f, 3.0f);
	if ( pMarker->fWidth <= 0.0f ) pMarker->fWidth = 3.0f;
	if ( pMarker->fHeight <= 0.0f ) pMarker->fHeight = 3.0f;
	pMarker->iUnits = XGE_SVG_MARKER_UNITS_STROKE_WIDTH;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "markerUnits", sValue, sizeof(sValue)) &&
	     (strcmp(sValue, "userSpaceOnUse") == 0) ) {
		pMarker->iUnits = XGE_SVG_MARKER_UNITS_USER_SPACE;
	}
	pMarker->bOrientAuto = 0;
	pMarker->bOrientAutoStartReverse = 0;
	pMarker->fOrientDegrees = 0.0f;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "orient", sValue, sizeof(sValue)) ) {
		if ( strcmp(sValue, "auto") == 0 ) {
			pMarker->bOrientAuto = 1;
		} else if ( strcmp(sValue, "auto-start-reverse") == 0 ) {
			pMarker->bOrientAuto = 1;
			pMarker->bOrientAutoStartReverse = 1;
		} else {
			__xgeSvgParseFloat(sValue, &pMarker->fOrientDegrees);
		}
	}
	__xgeSvgDefShapesClear(&pSvg->pDefs[iDefIndex]);
	pSvg->pDefs[iDefIndex].bHasViewBox = 0;
	pSvg->pDefs[iDefIndex].iAspectAlignX = XGE_SVG_ASPECT_ALIGN_MID;
	pSvg->pDefs[iDefIndex].iAspectAlignY = XGE_SVG_ASPECT_ALIGN_MID;
	pSvg->pDefs[iDefIndex].iAspectMeetOrSlice = XGE_SVG_ASPECT_MEET;
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
	*pDefIndex = iDefIndex;
	return XGE_OK;
}

static int __xgeSvgParsePattern(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_style_t* pParentStyle, int* pDefIndex)
{
	char sId[XGE_SVG_ATTR_MAX];
	char sDefId[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	xge_svg_pattern_t* pPattern;
	float fX;
	float fY;
	float fW;
	float fH;
	float fFontSize;
	int iPatternIndex;
	int iDefIndex;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pDefIndex == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pDefIndex = -1;
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "id", sId, sizeof(sId)) || (sId[0] == '\0') ) {
		return XGE_OK;
	}
	{
		int iDefIdLen = snprintf(sDefId, sizeof(sDefId), "__pattern__:%s", sId);
		if ( (iDefIdLen < 0) || (iDefIdLen >= (int)sizeof(sDefId)) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	iRet = __xgeSvgPatternGetOrCreate(pSvg, sId, &iPatternIndex);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeSvgDefGetOrCreate(pSvg, sDefId, &iDefIndex);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pPattern = &pSvg->pPatterns[iPatternIndex];
	pPattern->iDefIndex = iDefIndex;
	fFontSize = pParentStyle != NULL ? pParentStyle->fFontSize : 16.0f;
	xrtFree(pPattern->sHrefId);
	pPattern->sHrefId = NULL;
	pPattern->iFlags = 0;
	pPattern->tRect.fX = 0.0f;
	pPattern->tRect.fY = 0.0f;
	pPattern->tRect.fW = 0.0f;
	pPattern->tRect.fH = 0.0f;
	pPattern->iUnits = XGE_SVG_PATTERN_OBJECT_BOUNDING_BOX;
	pPattern->iContentUnits = XGE_SVG_PATTERN_USER_SPACE;
	if ( __xgeSvgHrefIdCopy(pTag, pTagEnd, sValue, sizeof(sValue)) ) {
		pPattern->sHrefId = __xgeStrDup(sValue);
		if ( pPattern->sHrefId == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "patternUnits", sValue, sizeof(sValue)) ) {
		pPattern->iUnits = (strcmp(sValue, "userSpaceOnUse") == 0) ? XGE_SVG_PATTERN_USER_SPACE : XGE_SVG_PATTERN_OBJECT_BOUNDING_BOX;
		pPattern->iFlags |= XGE_SVG_PATTERN_HAS_UNITS;
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "patternContentUnits", sValue, sizeof(sValue)) ) {
		pPattern->iContentUnits = (strcmp(sValue, "objectBoundingBox") == 0) ? XGE_SVG_PATTERN_OBJECT_BOUNDING_BOX : XGE_SVG_PATTERN_USER_SPACE;
		pPattern->iFlags |= XGE_SVG_PATTERN_HAS_CONTENT_UNITS;
	}
	if ( __xgeSvgPatternLengthCopy(pSvg, pTag, pTagEnd, "x", pPattern->iUnits, XGE_SVG_LENGTH_BASIS_X, fFontSize, &pPattern->tRect.fX) ) {
		pPattern->iFlags |= XGE_SVG_PATTERN_HAS_X;
	}
	if ( __xgeSvgPatternLengthCopy(pSvg, pTag, pTagEnd, "y", pPattern->iUnits, XGE_SVG_LENGTH_BASIS_Y, fFontSize, &pPattern->tRect.fY) ) {
		pPattern->iFlags |= XGE_SVG_PATTERN_HAS_Y;
	}
	if ( __xgeSvgPatternLengthCopy(pSvg, pTag, pTagEnd, "width", pPattern->iUnits, XGE_SVG_LENGTH_BASIS_X, fFontSize, &pPattern->tRect.fW) ) {
		pPattern->iFlags |= XGE_SVG_PATTERN_HAS_WIDTH;
	}
	if ( __xgeSvgPatternLengthCopy(pSvg, pTag, pTagEnd, "height", pPattern->iUnits, XGE_SVG_LENGTH_BASIS_Y, fFontSize, &pPattern->tRect.fH) ) {
		pPattern->iFlags |= XGE_SVG_PATTERN_HAS_HEIGHT;
	}
	pPattern->tTransform = __xgeSvgMatrixIdentity();
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "patternTransform", sValue, sizeof(sValue)) ) {
		if ( __xgeSvgParseTransform(sValue, &pPattern->tTransform) ) {
			pPattern->iFlags |= XGE_SVG_PATTERN_HAS_TRANSFORM;
		}
	}
	__xgeSvgDefShapesClear(&pSvg->pDefs[iDefIndex]);
	pSvg->pDefs[iDefIndex].bHasViewBox = 0;
	pSvg->pDefs[iDefIndex].iAspectAlignX = XGE_SVG_ASPECT_ALIGN_MID;
	pSvg->pDefs[iDefIndex].iAspectAlignY = XGE_SVG_ASPECT_ALIGN_MID;
	pSvg->pDefs[iDefIndex].iAspectMeetOrSlice = XGE_SVG_ASPECT_MEET;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "viewBox", sValue, sizeof(sValue)) &&
	     __xgeSvgParseFourFloats(sValue, &fX, &fY, &fW, &fH) && (fW > 0.0f) && (fH > 0.0f) ) {
		pSvg->pDefs[iDefIndex].tViewBox.fX = fX;
		pSvg->pDefs[iDefIndex].tViewBox.fY = fY;
		pSvg->pDefs[iDefIndex].tViewBox.fW = fW;
		pSvg->pDefs[iDefIndex].tViewBox.fH = fH;
		pSvg->pDefs[iDefIndex].bHasViewBox = 1;
		pPattern->iFlags |= XGE_SVG_PATTERN_HAS_VIEWBOX;
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "preserveAspectRatio", sValue, sizeof(sValue)) ) {
		__xgeSvgParsePreserveAspectRatioFields(sValue,
			&pSvg->pDefs[iDefIndex].iAspectAlignX,
			&pSvg->pDefs[iDefIndex].iAspectAlignY,
			&pSvg->pDefs[iDefIndex].iAspectMeetOrSlice);
		pPattern->iFlags |= XGE_SVG_PATTERN_HAS_ASPECT;
	}
	*pDefIndex = iDefIndex;
	return XGE_OK;
}

static int __xgeSvgParseClipPath(xge_svg pSvg, const char* pTag, const char* pTagEnd, int* pClipIndex)
{
	char sId[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	int iIndex;
	int iRet;
	int i;

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
	xrtFree(pSvg->pClipPaths[iIndex].pRects);
	pSvg->pClipPaths[iIndex].pRects = NULL;
	pSvg->pClipPaths[iIndex].iRectCount = 0;
	pSvg->pClipPaths[iIndex].iRectCapacity = 0;
	for ( i = 0; i < pSvg->pClipPaths[iIndex].iShapeCount; i++ ) {
		xgeShapeExDestroy(pSvg->pClipPaths[iIndex].pShapes[i]);
	}
	xrtFree(pSvg->pClipPaths[iIndex].pShapes);
	pSvg->pClipPaths[iIndex].pShapes = NULL;
	pSvg->pClipPaths[iIndex].iShapeCount = 0;
	pSvg->pClipPaths[iIndex].iShapeCapacity = 0;
	pSvg->pClipPaths[iIndex].iUnits = XGE_SVG_CLIP_USER_SPACE;
	pSvg->pClipPaths[iIndex].bHasRect = 0;
	pSvg->pClipPaths[iIndex].bHasShape = 0;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "clipPathUnits", sValue, sizeof(sValue)) &&
	     (strcmp(sValue, "objectBoundingBox") == 0) ) {
		pSvg->pClipPaths[iIndex].iUnits = XGE_SVG_CLIP_OBJECT_BOUNDING_BOX;
	}
	*pClipIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgParseClipRect(xge_svg pSvg, int iClipIndex, const char* pTag, const char* pTagEnd, xge_shape_ex_matrix_t tParentTransform, const xge_svg_style_t* pParentStyle)
{
	xge_rect_t tRect;
	char sValue[XGE_SVG_ATTR_MAX];
	xge_svg_style_t tStyle;
	xge_shape_ex_matrix_t tTransform;

	if ( !__xgeSvgValid(pSvg) || (iClipIndex < 0) || (iClipIndex >= pSvg->iClipPathCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeSvgTagNameEquals(pTag, "rect") ) {
		return XGE_OK;
	}
	tStyle = (pParentStyle != NULL) ? __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd) : __xgeSvgStyleDefault();
	if ( !tStyle.bVisible || !tStyle.bVisibility ) {
		return XGE_OK;
	}
	if ( pSvg->pClipPaths[iClipIndex].iUnits == XGE_SVG_CLIP_OBJECT_BOUNDING_BOX ) {
		tRect.fX = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "x", 1.0f, tStyle.fFontSize, 0.0f);
		tRect.fY = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "y", 1.0f, tStyle.fFontSize, 0.0f);
		tRect.fW = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "width", 1.0f, tStyle.fFontSize, 0.0f);
		tRect.fH = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "height", 1.0f, tStyle.fFontSize, 0.0f);
	} else {
		tRect.fX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		tRect.fY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		tRect.fW = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		tRect.fH = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
	}
	if ( tRect.fW < 0.0f ) tRect.fW = 0.0f;
	if ( tRect.fH < 0.0f ) tRect.fH = 0.0f;
	tTransform = tParentTransform;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "transform", sValue, sizeof(sValue)) ) {
		xge_shape_ex_matrix_t tLocalTransform;

		if ( __xgeSvgParseTransform(sValue, &tLocalTransform) ) {
			tTransform = __xgeSvgMatrixMul(tTransform, tLocalTransform);
		}
	}
	tRect = __xgeSvgMatrixRectBounds(tTransform, tRect);
	return __xgeSvgClipPathAddRect(&pSvg->pClipPaths[iClipIndex], tRect);
}

static int __xgeSvgParseClipShapeFinish(xge_svg pSvg, int iClipIndex, xge_shape_ex pShape, const xge_svg_style_t* pStyle, xge_shape_ex_matrix_t tTransform)
{
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iClipIndex < 0) || (iClipIndex >= pSvg->iClipPathCount) || (pShape == NULL) ) {
		xgeShapeExDestroy(pShape);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pStyle != NULL) && (!pStyle->bVisible || !pStyle->bVisibility) ) {
		xgeShapeExDestroy(pShape);
		return XGE_OK;
	}
	if ( pStyle != NULL ) {
		xgeShapeExFillRule(pShape, pStyle->iClipRule);
	}
	xgeShapeExTransformSet(pShape, &tTransform);
	iRet = __xgeSvgClipPathAddShape(&pSvg->pClipPaths[iClipIndex], pShape);
	xgeShapeExDestroy(pShape);
	return iRet;
}

static xge_shape_ex_matrix_t __xgeSvgParseClipShapeTransform(const char* pTag, const char* pTagEnd, xge_shape_ex_matrix_t tParentTransform)
{
	char sValue[XGE_SVG_ATTR_MAX];
	xge_shape_ex_matrix_t tTransform;

	tTransform = tParentTransform;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "transform", sValue, sizeof(sValue)) ) {
		xge_shape_ex_matrix_t tLocalTransform;

		if ( __xgeSvgParseTransform(sValue, &tLocalTransform) ) {
			tTransform = __xgeSvgMatrixMul(tTransform, tLocalTransform);
		}
	}
	return tTransform;
}

static float __xgeSvgMaskStyleOpacity(const xge_svg_style_t* pStyle, int iMaskType);
static int __xgeSvgPrepareStoredMaskShapePaint(xge_shape_ex pShape, int iMaskType, float* pOpacity);

static int __xgeSvgUseTransformForDef(xge_svg pSvg, int iDefIndex, const xge_svg_style_t* pStyle, const char* pTag, const char* pTagEnd, xge_shape_ex_matrix_t tBaseTransform, xge_shape_ex_matrix_t* pTransform, int* pSkip)
{
	xge_shape_ex_matrix_t tUseTransform;
	float fX;
	float fY;
	float fW;
	float fH;
	int bHasWidth;
	int bHasHeight;

	if ( (pTransform == NULL) || (pSkip == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pTransform = tBaseTransform;
	*pSkip = 0;
	if ( !__xgeSvgValid(pSvg) || (iDefIndex < 0) || (iDefIndex >= pSvg->iDefCount) || (pStyle == NULL) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, pStyle->fFontSize, 0.0f);
	fY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, pStyle->fFontSize, 0.0f);
	fW = 0.0f;
	fH = 0.0f;
	bHasWidth = __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "width", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_X), pStyle->fFontSize, &fW);
	bHasHeight = __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "height", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_Y), pStyle->fFontSize, &fH);
	if ( pSvg->pDefs[iDefIndex].bHasViewBox ) {
		xge_rect_t tDst;
		xge_rect_t tViewport;

		if ( !bHasWidth ) fW = pSvg->pDefs[iDefIndex].tViewBox.fW;
		if ( !bHasHeight ) fH = pSvg->pDefs[iDefIndex].tViewBox.fH;
		if ( (fW <= 0.0f) || (fH <= 0.0f) ) {
			*pSkip = 1;
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
		*pTransform = __xgeSvgMatrixMul(*pTransform, tUseTransform);
	} else if ( (fabsf(fX) > 0.0f) || (fabsf(fY) > 0.0f) ) {
		tUseTransform = __xgeSvgMatrixTranslate(fX, fY);
		*pTransform = __xgeSvgMatrixMul(*pTransform, tUseTransform);
	}
	return XGE_OK;
}

static int __xgeSvgAddUseShapesToClipPath(xge_svg pSvg, int iClipIndex, int iDefIndex, const xge_svg_style_t* pStyle, xge_shape_ex_matrix_t tUseTransform)
{
	int i;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iClipIndex < 0) || (iClipIndex >= pSvg->iClipPathCount) || (iDefIndex < 0) || (iDefIndex >= pSvg->iDefCount) || (pStyle == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pSvg->pDefs[iDefIndex].iShapeCount; i++ ) {
		xge_shape_ex pClone;
		xge_shape_ex_matrix_t tShapeTransform;

		iRet = xgeShapeExClone(pSvg->pDefs[iDefIndex].pShapes[i], &pClone);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		tShapeTransform = __xgeSvgMatrixMul(tUseTransform, pClone->tTransform);
		iRet = xgeShapeExTransformSet(pClone, &tShapeTransform);
		if ( iRet == XGE_OK ) {
			iRet = xgeShapeExFillRule(pClone, pStyle->iClipRule);
		}
		if ( iRet == XGE_OK ) {
			iRet = __xgeSvgClipPathAddShape(&pSvg->pClipPaths[iClipIndex], pClone);
		}
		xgeShapeExDestroy(pClone);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeSvgAddUseShapesToMask(xge_svg pSvg, int iMaskIndex, int iDefIndex, const xge_svg_style_t* pStyle, xge_shape_ex_matrix_t tUseTransform)
{
	float fUseOpacity;
	int i;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iMaskIndex < 0) || (iMaskIndex >= pSvg->iMaskCount) || (iDefIndex < 0) || (iDefIndex >= pSvg->iDefCount) || (pStyle == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fUseOpacity = __xgeSvgMaskStyleOpacity(pStyle, pSvg->pMasks[iMaskIndex].iMaskType);
	if ( fUseOpacity <= 0.0f ) {
		return XGE_OK;
	}
	for ( i = 0; i < pSvg->pDefs[iDefIndex].iShapeCount; i++ ) {
		xge_shape_ex pClone;
		xge_shape_ex_matrix_t tShapeTransform;
		float fShapeOpacity;
		float fOpacity;

		iRet = xgeShapeExClone(pSvg->pDefs[iDefIndex].pShapes[i], &pClone);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( !__xgeSvgPrepareStoredMaskShapePaint(pClone, pSvg->pMasks[iMaskIndex].iMaskType, &fShapeOpacity) ) {
			xgeShapeExDestroy(pClone);
			continue;
		}
		fOpacity = fUseOpacity * fShapeOpacity;
		tShapeTransform = __xgeSvgMatrixMul(tUseTransform, pClone->tTransform);
		iRet = xgeShapeExTransformSet(pClone, &tShapeTransform);
		if ( iRet == XGE_OK ) {
			iRet = xgeShapeExFillRule(pClone, pStyle->iFillRule);
		}
		if ( iRet == XGE_OK ) {
			iRet = __xgeSvgMaskAddShape(&pSvg->pMasks[iMaskIndex], pClone, fOpacity);
		}
		xgeShapeExDestroy(pClone);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeSvgParseClipShape(xge_svg pSvg, int iClipIndex, const char* pTag, const char* pTagEnd, xge_shape_ex_matrix_t tParentTransform, const xge_svg_style_t* pParentStyle)
{
	char sValue[XGE_SVG_ATTR_MAX];
	xge_svg_style_t tStyle;
	xge_shape_ex_matrix_t tTransform;
	xge_shape_ex pShape;
	int bObjectBounds;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iClipIndex < 0) || (iClipIndex >= pSvg->iClipPathCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeSvgTagNameEquals(pTag, "rect") ) {
		iRet = __xgeSvgParseClipRect(pSvg, iClipIndex, pTag, pTagEnd, tParentTransform, pParentStyle);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	tStyle = (pParentStyle != NULL) ? __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd) : __xgeSvgStyleDefault();
	if ( !tStyle.bVisible || !tStyle.bVisibility ) {
		return XGE_OK;
	}
	tTransform = __xgeSvgParseClipShapeTransform(pTag, pTagEnd, tParentTransform);
	bObjectBounds = (pSvg->pClipPaths[iClipIndex].iUnits == XGE_SVG_CLIP_OBJECT_BOUNDING_BOX);
	pShape = NULL;
	if ( __xgeSvgTagNameEquals(pTag, "use") ) {
		int iDefIndex;
		int bSkip;

		if ( !__xgeSvgHrefIdCopy(pTag, pTagEnd, sValue, sizeof(sValue)) ) {
			return XGE_OK;
		}
		iDefIndex = __xgeSvgDefFind(pSvg, sValue);
		if ( iDefIndex < 0 ) {
			return XGE_OK;
		}
		iRet = __xgeSvgUseTransformForDef(pSvg, iDefIndex, &tStyle, pTag, pTagEnd, tTransform, &tTransform, &bSkip);
		if ( (iRet != XGE_OK) || bSkip ) {
			return iRet;
		}
		return __xgeSvgAddUseShapesToClipPath(pSvg, iClipIndex, iDefIndex, &tStyle, tTransform);
	}
	if ( __xgeSvgTagNameEquals(pTag, "path") ) {
		if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "d", sValue, sizeof(sValue)) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = xgeShapeExAppendSvgPath(pShape, sValue);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		return __xgeSvgParseClipShapeFinish(pSvg, iClipIndex, pShape, &tStyle, tTransform);
	}
	if ( __xgeSvgTagNameEquals(pTag, "rect") ) {
		float fX = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "x", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fY = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "y", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		float fW = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "width", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fH = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "height", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		float fRX = 0.0f;
		float fRY = 0.0f;
		int bHasRX = bObjectBounds ? __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "rx", 1.0f, tStyle.fFontSize, &fRX) :
			__xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "rx", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_X), tStyle.fFontSize, &fRX);
		int bHasRY = bObjectBounds ? __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "ry", 1.0f, tStyle.fFontSize, &fRY) :
			__xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "ry", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_Y), tStyle.fFontSize, &fRY);

		if ( bHasRX && !bHasRY ) {
			fRY = fRX;
		} else if ( !bHasRX && bHasRY ) {
			fRX = fRY;
		}
		if ( (fW <= 0.0f) || (fH <= 0.0f) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = xgeShapeExAppendRect(pShape, fX, fY, fW, fH, fRX, fRY, 1);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		return __xgeSvgParseClipShapeFinish(pSvg, iClipIndex, pShape, &tStyle, tTransform);
	}
	if ( __xgeSvgTagNameEquals(pTag, "circle") || __xgeSvgTagNameEquals(pTag, "ellipse") ) {
		float fCX = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "cx", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "cx", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fCY = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "cy", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "cy", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		float fR = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "r", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "r", XGE_SVG_LENGTH_BASIS_OTHER, tStyle.fFontSize, 0.0f);
		float fRX = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "rx", 1.0f, tStyle.fFontSize, fR) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "rx", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, fR);
		float fRY = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "ry", 1.0f, tStyle.fFontSize, fR) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "ry", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, fR);

		if ( (fRX <= 0.0f) || (fRY <= 0.0f) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = xgeShapeExAppendCircle(pShape, fCX, fCY, fRX, fRY, 1);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		return __xgeSvgParseClipShapeFinish(pSvg, iClipIndex, pShape, &tStyle, tTransform);
	}
	if ( __xgeSvgTagNameEquals(pTag, "line") ) {
		float fX1 = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "x1", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x1", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fY1 = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "y1", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y1", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		float fX2 = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "x2", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x2", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fY2 = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "y2", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y2", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);

		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		xgeShapeExMoveTo(pShape, fX1, fY1);
		xgeShapeExLineTo(pShape, fX2, fY2);
		return __xgeSvgParseClipShapeFinish(pSvg, iClipIndex, pShape, &tStyle, tTransform);
	}
	if ( __xgeSvgTagNameEquals(pTag, "polygon") || __xgeSvgTagNameEquals(pTag, "polyline") ) {
		int bClose = __xgeSvgTagNameEquals(pTag, "polygon");

		if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "points", sValue, sizeof(sValue)) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = __xgeSvgParsePoints(pShape, sValue, bClose);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		return __xgeSvgParseClipShapeFinish(pSvg, iClipIndex, pShape, &tStyle, tTransform);
	}
	return XGE_OK;
}

static int __xgeSvgParseMask(xge_svg pSvg, const char* pTag, const char* pTagEnd, int* pMaskIndex)
{
	char sId[XGE_SVG_ATTR_MAX];
	char sStyle[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	int iIndex;
	int i;
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
	for ( i = 0; i < pSvg->pMasks[iIndex].iShapeCount; i++ ) {
		xgeShapeExDestroy(pSvg->pMasks[iIndex].pShapes[i].pShape);
	}
	xrtFree(pSvg->pMasks[iIndex].pShapes);
	pSvg->pMasks[iIndex].pShapes = NULL;
	pSvg->pMasks[iIndex].iShapeCount = 0;
	pSvg->pMasks[iIndex].iShapeCapacity = 0;
	pSvg->pMasks[iIndex].iUnits = XGE_SVG_MASK_OBJECT_BOUNDING_BOX;
	pSvg->pMasks[iIndex].iContentUnits = XGE_SVG_MASK_USER_SPACE;
	pSvg->pMasks[iIndex].tRegion = (xge_rect_t){-0.1f, -0.1f, 1.2f, 1.2f};
	pSvg->pMasks[iIndex].iMaskType = XGE_SVG_MASK_TYPE_LUMINANCE;
	sStyle[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "style", sStyle, sizeof(sStyle));
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "maskUnits", sValue, sizeof(sValue)) ) {
		if ( strcmp(sValue, "userSpaceOnUse") == 0 ) {
			pSvg->pMasks[iIndex].iUnits = XGE_SVG_MASK_USER_SPACE;
		} else if ( strcmp(sValue, "objectBoundingBox") == 0 ) {
			pSvg->pMasks[iIndex].iUnits = XGE_SVG_MASK_OBJECT_BOUNDING_BOX;
		}
	}
	if ( pSvg->pMasks[iIndex].iUnits == XGE_SVG_MASK_OBJECT_BOUNDING_BOX ) {
		xge_svg_style_t tMaskStyle = __xgeSvgStyleDefault();

		pSvg->pMasks[iIndex].tRegion.fX = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "x", 1.0f, tMaskStyle.fFontSize, -0.1f);
		pSvg->pMasks[iIndex].tRegion.fY = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "y", 1.0f, tMaskStyle.fFontSize, -0.1f);
		pSvg->pMasks[iIndex].tRegion.fW = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "width", 1.0f, tMaskStyle.fFontSize, 1.2f);
		pSvg->pMasks[iIndex].tRegion.fH = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "height", 1.0f, tMaskStyle.fFontSize, 1.2f);
	} else {
		xge_svg_style_t tMaskStyle = __xgeSvgStyleDefault();
		float fRefX = __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_X);
		float fRefY = __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_Y);

		pSvg->pMasks[iIndex].tRegion.fX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, tMaskStyle.fFontSize, -0.1f * fRefX);
		pSvg->pMasks[iIndex].tRegion.fY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, tMaskStyle.fFontSize, -0.1f * fRefY);
		pSvg->pMasks[iIndex].tRegion.fW = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, tMaskStyle.fFontSize, 1.2f * fRefX);
		pSvg->pMasks[iIndex].tRegion.fH = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, tMaskStyle.fFontSize, 1.2f * fRefY);
	}
	if ( pSvg->pMasks[iIndex].tRegion.fW < 0.0f ) pSvg->pMasks[iIndex].tRegion.fW = 0.0f;
	if ( pSvg->pMasks[iIndex].tRegion.fH < 0.0f ) pSvg->pMasks[iIndex].tRegion.fH = 0.0f;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "maskContentUnits", sValue, sizeof(sValue)) &&
	     (strcmp(sValue, "objectBoundingBox") == 0) ) {
		pSvg->pMasks[iIndex].iContentUnits = XGE_SVG_MASK_OBJECT_BOUNDING_BOX;
	}
	if ( __xgeSvgAttrOrStyleCopy(pTag, pTagEnd, sStyle, "mask-type", sValue, sizeof(sValue)) ) {
		if ( strcmp(sValue, "alpha") == 0 ) {
			pSvg->pMasks[iIndex].iMaskType = XGE_SVG_MASK_TYPE_ALPHA;
		} else if ( strcmp(sValue, "luminance") == 0 ) {
			pSvg->pMasks[iIndex].iMaskType = XGE_SVG_MASK_TYPE_LUMINANCE;
		}
	}
	*pMaskIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgParseMaskRect(xge_svg pSvg, int iMaskIndex, const char* pTag, const char* pTagEnd, xge_shape_ex_matrix_t tParentTransform, const xge_svg_style_t* pParentStyle)
{
	xge_svg_mask_rect_t tMaskRect;
	char sValue[XGE_SVG_ATTR_MAX];
	xge_svg_style_t tStyle;
	xge_shape_ex_matrix_t tTransform;

	if ( !__xgeSvgValid(pSvg) || (iMaskIndex < 0) || (iMaskIndex >= pSvg->iMaskCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeSvgTagNameEquals(pTag, "rect") ) {
		return XGE_OK;
	}
	tStyle = (pParentStyle != NULL) ? __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd) : __xgeSvgStyleDefault();
	if ( !tStyle.bVisible || !tStyle.bVisibility ) {
		return XGE_OK;
	}
	if ( pSvg->pMasks[iMaskIndex].iContentUnits == XGE_SVG_MASK_OBJECT_BOUNDING_BOX ) {
		tMaskRect.tRect.fX = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "x", 1.0f, tStyle.fFontSize, 0.0f);
		tMaskRect.tRect.fY = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "y", 1.0f, tStyle.fFontSize, 0.0f);
		tMaskRect.tRect.fW = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "width", 1.0f, tStyle.fFontSize, 0.0f);
		tMaskRect.tRect.fH = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "height", 1.0f, tStyle.fFontSize, 0.0f);
	} else {
		tMaskRect.tRect.fX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		tMaskRect.tRect.fY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		tMaskRect.tRect.fW = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		tMaskRect.tRect.fH = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
	}
	if ( tMaskRect.tRect.fW < 0.0f ) tMaskRect.tRect.fW = 0.0f;
	if ( tMaskRect.tRect.fH < 0.0f ) tMaskRect.tRect.fH = 0.0f;
	tTransform = tParentTransform;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "transform", sValue, sizeof(sValue)) ) {
		xge_shape_ex_matrix_t tLocalTransform;

		if ( __xgeSvgParseTransform(sValue, &tLocalTransform) ) {
			tTransform = __xgeSvgMatrixMul(tTransform, tLocalTransform);
		}
	}
	tMaskRect.tRect = __xgeSvgMatrixRectBounds(tTransform, tMaskRect.tRect);
	tMaskRect.fOpacity = __xgeSvgMaskStyleOpacity(&tStyle, pSvg->pMasks[iMaskIndex].iMaskType);
	return __xgeSvgMaskAddRect(&pSvg->pMasks[iMaskIndex], tMaskRect);
}

static float __xgeSvgMaskColorOpacity(uint32_t iColor, float fOpacity, int iMaskType);

static float __xgeSvgMaskStyleOpacity(const xge_svg_style_t* pStyle, int iMaskType)
{
	if ( pStyle == NULL ) {
		return 0.0f;
	}
	return __xgeSvgMaskColorOpacity(pStyle->iFillColor, pStyle->fOpacity * pStyle->fFillOpacity, iMaskType);
}

static float __xgeSvgMaskColorOpacity(uint32_t iColor, float fOpacity, int iMaskType)
{
	float fLuma;

	fOpacity *= (float)XGE_COLOR_GET_A(iColor) / 255.0f;
	if ( iMaskType != XGE_SVG_MASK_TYPE_ALPHA ) {
		fLuma = (0.2126f * (float)XGE_COLOR_GET_R(iColor) + 0.7152f * (float)XGE_COLOR_GET_G(iColor) + 0.0722f * (float)XGE_COLOR_GET_B(iColor)) / 255.0f;
		fOpacity *= fLuma;
	}
	if ( fOpacity < 0.0f ) fOpacity = 0.0f;
	if ( fOpacity > 1.0f ) fOpacity = 1.0f;
	return fOpacity;
}

static float __xgeSvgMaskStyleStrokeOpacity(const xge_svg_style_t* pStyle, int iMaskType)
{
	if ( (pStyle == NULL) || !__xgeSvgStyleHasPaintAlpha(pStyle, 1) ) {
		return 0.0f;
	}
	return __xgeSvgMaskColorOpacity(pStyle->iStrokeColor, pStyle->fOpacity * pStyle->fStrokeOpacity, iMaskType);
}

static int __xgeSvgPrepareStoredMaskShapePaint(xge_shape_ex pShape, int iMaskType, float* pOpacity)
{
	float fFillOpacity;
	float fStrokeOpacity;
	float fOpacity;

	if ( (pShape == NULL) || (pOpacity == NULL) ) {
		return 0;
	}
	fFillOpacity = __xgeSvgMaskColorOpacity(pShape->iFillColor, 1.0f, iMaskType);
	fStrokeOpacity = (pShape->fStrokeWidth > 0.0f) ? __xgeSvgMaskColorOpacity(pShape->iStrokeColor, 1.0f, iMaskType) : 0.0f;
	fOpacity = fFillOpacity > fStrokeOpacity ? fFillOpacity : fStrokeOpacity;
	fOpacity *= pShape->fOpacity;
	if ( fOpacity <= 0.0f ) {
		*pOpacity = 0.0f;
		return 0;
	}
	xgeShapeExFillColor(pShape, fFillOpacity > 0.0f ? XGE_COLOR_RGBA(255, 255, 255, 255) : XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeColor(pShape, fStrokeOpacity > 0.0f ? XGE_COLOR_RGBA(255, 255, 255, 255) : XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExOpacity(pShape, 1.0f);
	pShape->bStencilPaint = 1;
	*pOpacity = fOpacity > 1.0f ? 1.0f : fOpacity;
	return 1;
}

static int __xgeSvgPrepareMaskShapePaint(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iMaskType, float* pOpacity)
{
	if ( (pShape == NULL) || (pOpacity == NULL) ) {
		return 0;
	}
	if ( pStyle != NULL ) {
		__xgeSvgApplyShapeStyle(pSvg, pShape, pStyle);
	}
	return __xgeSvgPrepareStoredMaskShapePaint(pShape, iMaskType, pOpacity);
}

static int __xgeSvgParseMaskShapeFinish(xge_svg pSvg, int iMaskIndex, xge_shape_ex pShape, const xge_svg_style_t* pStyle, xge_shape_ex_matrix_t tTransform)
{
	float fOpacity;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iMaskIndex < 0) || (iMaskIndex >= pSvg->iMaskCount) || (pShape == NULL) ) {
		xgeShapeExDestroy(pShape);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pStyle != NULL) && (!pStyle->bVisible || !pStyle->bVisibility) ) {
		xgeShapeExDestroy(pShape);
		return XGE_OK;
	}
	if ( !__xgeSvgPrepareMaskShapePaint(pSvg, pShape, pStyle, pSvg->pMasks[iMaskIndex].iMaskType, &fOpacity) ) {
		xgeShapeExDestroy(pShape);
		return XGE_OK;
	}
	if ( pStyle != NULL ) {
		xgeShapeExFillRule(pShape, pStyle->iFillRule);
	}
	xgeShapeExTransformSet(pShape, &tTransform);
	iRet = __xgeSvgMaskAddShape(&pSvg->pMasks[iMaskIndex], pShape, fOpacity);
	xgeShapeExDestroy(pShape);
	return iRet;
}

static int __xgeSvgParseMaskShape(xge_svg pSvg, int iMaskIndex, const char* pTag, const char* pTagEnd, xge_shape_ex_matrix_t tParentTransform, const xge_svg_style_t* pParentStyle)
{
	char sValue[XGE_SVG_ATTR_MAX];
	xge_svg_style_t tStyle;
	xge_shape_ex_matrix_t tTransform;
	xge_shape_ex pShape;
	int bObjectBounds;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iMaskIndex < 0) || (iMaskIndex >= pSvg->iMaskCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tStyle = (pParentStyle != NULL) ? __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd) : __xgeSvgStyleDefault();
	if ( !tStyle.bVisible || !tStyle.bVisibility ) {
		return XGE_OK;
	}
	tTransform = __xgeSvgParseClipShapeTransform(pTag, pTagEnd, tParentTransform);
	bObjectBounds = (pSvg->pMasks[iMaskIndex].iContentUnits == XGE_SVG_MASK_OBJECT_BOUNDING_BOX);
	pShape = NULL;
	if ( __xgeSvgTagNameEquals(pTag, "use") ) {
		int iDefIndex;
		int bSkip;

		if ( !__xgeSvgHrefIdCopy(pTag, pTagEnd, sValue, sizeof(sValue)) ) {
			return XGE_OK;
		}
		iDefIndex = __xgeSvgDefFind(pSvg, sValue);
		if ( iDefIndex < 0 ) {
			return XGE_OK;
		}
		iRet = __xgeSvgUseTransformForDef(pSvg, iDefIndex, &tStyle, pTag, pTagEnd, tTransform, &tTransform, &bSkip);
		if ( (iRet != XGE_OK) || bSkip ) {
			return iRet;
		}
		return __xgeSvgAddUseShapesToMask(pSvg, iMaskIndex, iDefIndex, &tStyle, tTransform);
	}
	if ( __xgeSvgTagNameEquals(pTag, "rect") ) {
		float fRX = 0.0f;
		float fRY = 0.0f;
		int bHasRX = bObjectBounds ? __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "rx", 1.0f, tStyle.fFontSize, &fRX) :
			__xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "rx", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_X), tStyle.fFontSize, &fRX);
		int bHasRY = bObjectBounds ? __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "ry", 1.0f, tStyle.fFontSize, &fRY) :
			__xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "ry", __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_Y), tStyle.fFontSize, &fRY);

		if ( bHasRX && !bHasRY ) {
			fRY = fRX;
		} else if ( !bHasRX && bHasRY ) {
			fRX = fRY;
		}
		if ( ((fRX <= 0.0f) && (fRY <= 0.0f)) &&
		     (__xgeSvgMaskStyleStrokeOpacity(&tStyle, pSvg->pMasks[iMaskIndex].iMaskType) <= 0.0f) ) {
			return __xgeSvgParseMaskRect(pSvg, iMaskIndex, pTag, pTagEnd, tParentTransform, pParentStyle);
		}
		{
			float fX = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "x", 1.0f, tStyle.fFontSize, 0.0f) :
				__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
			float fY = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "y", 1.0f, tStyle.fFontSize, 0.0f) :
				__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
			float fW = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "width", 1.0f, tStyle.fFontSize, 0.0f) :
				__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
			float fH = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "height", 1.0f, tStyle.fFontSize, 0.0f) :
				__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);

			if ( (fW <= 0.0f) || (fH <= 0.0f) ) {
				return XGE_OK;
			}
			iRet = xgeShapeExCreate(&pShape);
			if ( iRet != XGE_OK ) return iRet;
			iRet = xgeShapeExAppendRect(pShape, fX, fY, fW, fH, fRX, fRY, 1);
		}
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		return __xgeSvgParseMaskShapeFinish(pSvg, iMaskIndex, pShape, &tStyle, tTransform);
	}
	if ( __xgeSvgTagNameEquals(pTag, "path") ) {
		if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "d", sValue, sizeof(sValue)) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = xgeShapeExAppendSvgPath(pShape, sValue);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		return __xgeSvgParseMaskShapeFinish(pSvg, iMaskIndex, pShape, &tStyle, tTransform);
	}
	if ( __xgeSvgTagNameEquals(pTag, "circle") || __xgeSvgTagNameEquals(pTag, "ellipse") ) {
		float fCX = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "cx", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "cx", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fCY = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "cy", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "cy", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		float fR = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "r", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "r", XGE_SVG_LENGTH_BASIS_OTHER, tStyle.fFontSize, 0.0f);
		float fRX = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "rx", 1.0f, tStyle.fFontSize, fR) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "rx", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, fR);
		float fRY = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "ry", 1.0f, tStyle.fFontSize, fR) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "ry", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, fR);

		if ( (fRX <= 0.0f) || (fRY <= 0.0f) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = xgeShapeExAppendCircle(pShape, fCX, fCY, fRX, fRY, 1);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		return __xgeSvgParseMaskShapeFinish(pSvg, iMaskIndex, pShape, &tStyle, tTransform);
	}
	if ( __xgeSvgTagNameEquals(pTag, "line") ) {
		float fX1 = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "x1", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x1", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fY1 = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "y1", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y1", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		float fX2 = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "x2", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x2", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fY2 = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "y2", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y2", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);

		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		xgeShapeExMoveTo(pShape, fX1, fY1);
		xgeShapeExLineTo(pShape, fX2, fY2);
		return __xgeSvgParseMaskShapeFinish(pSvg, iMaskIndex, pShape, &tStyle, tTransform);
	}
	if ( __xgeSvgTagNameEquals(pTag, "polygon") || __xgeSvgTagNameEquals(pTag, "polyline") ) {
		int bClose = __xgeSvgTagNameEquals(pTag, "polygon");

		if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "points", sValue, sizeof(sValue)) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = __xgeSvgParsePoints(pShape, sValue, bClose);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		return __xgeSvgParseMaskShapeFinish(pSvg, iMaskIndex, pShape, &tStyle, tTransform);
	}
	return XGE_OK;
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
	pSvg->iCurrentElementNode = -1;
	memset(pSvg->arrLastChildByDepth, 0xff, sizeof(pSvg->arrLastChildByDepth));
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
	__xgeSvgMarkersClear(pSvg);
	__xgeSvgPatternsClear(pSvg);
	__xgeSvgClipPathsClear(pSvg);
	__xgeSvgMasksClear(pSvg);
	__xgeSvgFiltersClear(pSvg);
	__xgeSvgLinearGradientsClear(pSvg);
	__xgeSvgRadialGradientsClear(pSvg);
	__xgeSvgStyleRulesClear(pSvg);
	__xgeSvgPendingUsesClear(pSvg);
	__xgeSvgDrawItemsClear(pSvg);
	__xgeSvgFontCacheClear(pSvg);
	__xgeSvgElementNodesClear(pSvg);
	xrtFree(pSvg->sBaseDir);
	pSvg->sBaseDir = NULL;
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
	__xgeSvgDrawItemsClear(pSvg);
	__xgeSvgDefsClear(pSvg);
	__xgeSvgMarkersClear(pSvg);
	__xgeSvgPatternsClear(pSvg);
	__xgeSvgClipPathsClear(pSvg);
	__xgeSvgMasksClear(pSvg);
	__xgeSvgFiltersClear(pSvg);
	__xgeSvgLinearGradientsClear(pSvg);
	__xgeSvgRadialGradientsClear(pSvg);
	__xgeSvgStyleRulesClear(pSvg);
	__xgeSvgPendingUsesClear(pSvg);
	__xgeSvgFontCacheClear(pSvg);
	__xgeSvgElementNodesClear(pSvg);
	xrtFree(pSvg->sBaseDir);
	pSvg->sBaseDir = NULL;
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
	pSvg->iElementStackTop = 0;
	pSvg->iCurrentElementNode = -1;
	memset(pSvg->arrLastChildByDepth, 0xff, sizeof(pSvg->arrLastChildByDepth));
	return XGE_OK;
}

static int __xgeSvgLoadMemoryEx(xge_svg pSvg, const void* pData, int iSize, const char* sBaseDir)
{
	char* sText;
	const char* p;
	xge_svg_style_t arrStack[XGE_SVG_STACK_MAX];
	int arrDefStack[XGE_SVG_STACK_MAX];
	int arrMainIdDefStack[XGE_SVG_STACK_MAX];
	int arrGroupCopyParentDefStack[XGE_SVG_STACK_MAX];
	xge_shape_ex_matrix_t arrMainIdNormalizeMatrixStack[XGE_SVG_STACK_MAX];
	int arrMainIdNormalizeStack[XGE_SVG_STACK_MAX];
	int arrMainIdNormalizeCloseStack[XGE_SVG_STACK_MAX];
	int arrMainIdNormalizeStartDefStack[XGE_SVG_STACK_MAX];
	int iStackTop;
	int iDefsDepth;
	int iSkipDepth;
	int iLinearGradientIndex;
	int iRadialGradientIndex;
	int iClipPathIndex;
	int iClipTransformTop;
	xge_shape_ex_matrix_t arrClipTransformStack[XGE_SVG_STACK_MAX];
	xge_svg_style_t arrClipStyleStack[XGE_SVG_STACK_MAX];
	int iClipStyleTop;
	int iMaskIndex;
	int iMaskTransformTop;
	xge_shape_ex_matrix_t arrMaskTransformStack[XGE_SVG_STACK_MAX];
	xge_svg_style_t arrMaskStyleStack[XGE_SVG_STACK_MAX];
	int iMaskStyleTop;
	int iFilterIndex;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pData == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = xgeSvgClear(pSvg);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeSvgBaseDirSet(pSvg, sBaseDir);
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
	arrMainIdDefStack[0] = -1;
	arrGroupCopyParentDefStack[0] = -1;
	arrMainIdNormalizeMatrixStack[0] = __xgeSvgMatrixIdentity();
	arrMainIdNormalizeStack[0] = 0;
	arrMainIdNormalizeCloseStack[0] = 0;
	arrMainIdNormalizeStartDefStack[0] = 0;
	iStackTop = 0;
	iDefsDepth = 0;
	iSkipDepth = 0;
	iLinearGradientIndex = -1;
	iRadialGradientIndex = -1;
	iClipPathIndex = -1;
	iClipTransformTop = 0;
	arrClipTransformStack[0] = __xgeSvgMatrixIdentity();
	iClipStyleTop = 0;
	arrClipStyleStack[0] = arrStack[0];
	iMaskIndex = -1;
	iMaskTransformTop = 0;
	arrMaskTransformStack[0] = __xgeSvgMatrixIdentity();
	iMaskStyleTop = 0;
	arrMaskStyleStack[0] = arrStack[0];
	iFilterIndex = -1;
	pSvg->iElementStackTop = 0;
	iRet = __xgeSvgParseStyleBlocks(pSvg, sText);
	if ( iRet != XGE_OK ) {
		__xgeSvgElementNodesClear(pSvg);
		xrtFree(sText);
		return iRet;
	}
	iRet = __xgeSvgElementNodesPreScan(pSvg, sText);
	if ( iRet != XGE_OK ) {
		__xgeSvgElementNodesClear(pSvg);
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
		bSkipTag = __xgeSvgTagNameEquals(p, "style");
		if ( __xgeSvgIsCloseTagName(p, "clipPath") ) {
			iClipPathIndex = -1;
			iClipTransformTop = 0;
			arrClipTransformStack[0] = __xgeSvgMatrixIdentity();
			iClipStyleTop = 0;
			arrClipStyleStack[0] = arrStack[iStackTop];
			p = pTagEnd + 1;
			continue;
		}
		if ( (iClipPathIndex >= 0) && __xgeSvgIsCloseTagName(p, "g") ) {
			if ( iClipTransformTop > 0 ) {
				iClipTransformTop--;
			}
			if ( iClipStyleTop > 0 ) {
				iClipStyleTop--;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgIsCloseTagName(p, "mask") ) {
			iMaskIndex = -1;
			iMaskTransformTop = 0;
			arrMaskTransformStack[0] = __xgeSvgMatrixIdentity();
			iMaskStyleTop = 0;
			arrMaskStyleStack[0] = arrStack[iStackTop];
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgIsCloseTagName(p, "filter") ) {
			if ( iFilterIndex >= 0 ) {
				__xgeSvgElementStackPop(pSvg, "filter");
			}
			iFilterIndex = -1;
			p = pTagEnd + 1;
			continue;
		}
		if ( (iMaskIndex >= 0) && __xgeSvgIsCloseTagName(p, "g") ) {
			if ( iMaskTransformTop > 0 ) {
				iMaskTransformTop--;
			}
			if ( iMaskStyleTop > 0 ) {
				iMaskStyleTop--;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgIsCloseTagName(p, "style") ) {
			if ( iSkipDepth > 0 ) {
				iSkipDepth--;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgIsCloseTagName(p, "linearGradient") ) {
			iLinearGradientIndex = -1;
			__xgeSvgElementStackPop(pSvg, "linearGradient");
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgIsCloseTagName(p, "radialGradient") ) {
			iRadialGradientIndex = -1;
			__xgeSvgElementStackPop(pSvg, "radialGradient");
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgIsCloseTagName(p, "defs") ) {
			if ( iDefsDepth > 0 ) {
				iDefsDepth--;
			}
			__xgeSvgElementStackPop(pSvg, "defs");
			p = pTagEnd + 1;
			continue;
		}
		if ( iMaskIndex >= 0 ) {
			if ( __xgeSvgTagNameEquals(p, "g") ) {
				if ( !bSelfClosing ) {
					xge_shape_ex_matrix_t tTransform = arrMaskTransformStack[iMaskTransformTop];
					char sTransform[XGE_SVG_ATTR_MAX];
					xge_svg_style_t tGroupStyle;

					if ( (iMaskTransformTop >= (XGE_SVG_STACK_MAX - 1)) || (iMaskStyleTop >= (XGE_SVG_STACK_MAX - 1)) ) {
						__xgeSvgElementNodesClear(pSvg);
						xrtFree(sText);
						return XGE_ERROR_OUT_OF_MEMORY;
					}
					if ( __xgeSvgAttrCopy(p, pTagEnd, "transform", sTransform, sizeof(sTransform)) ) {
						xge_shape_ex_matrix_t tLocalTransform;

						if ( __xgeSvgParseTransform(sTransform, &tLocalTransform) ) {
							tTransform = __xgeSvgMatrixMul(tTransform, tLocalTransform);
						}
					}
					arrMaskTransformStack[++iMaskTransformTop] = tTransform;
					tGroupStyle = __xgeSvgStyleResolve(pSvg, &arrMaskStyleStack[iMaskStyleTop], p, pTagEnd);
					arrMaskStyleStack[++iMaskStyleTop] = tGroupStyle;
				}
				p = pTagEnd + 1;
				continue;
			}
			iRet = __xgeSvgParseMaskShape(pSvg, iMaskIndex, p, pTagEnd, arrMaskTransformStack[iMaskTransformTop], &arrMaskStyleStack[iMaskStyleTop]);
			if ( iRet != XGE_OK ) {
				__xgeSvgElementNodesClear(pSvg);
				xrtFree(sText);
				return iRet;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( iFilterIndex >= 0 ) {
			if ( __xgeSvgTagNameEquals(p, "feOffset") ) {
				iRet = __xgeSvgParseFilterFeOffset(pSvg, iFilterIndex, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			} else if ( __xgeSvgTagNameEquals(p, "feDropShadow") ) {
				iRet = __xgeSvgParseFilterFeDropShadow(pSvg, iFilterIndex, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( iClipPathIndex >= 0 ) {
			if ( __xgeSvgTagNameEquals(p, "g") ) {
				if ( !bSelfClosing ) {
					xge_shape_ex_matrix_t tTransform = arrClipTransformStack[iClipTransformTop];
					char sTransform[XGE_SVG_ATTR_MAX];
					xge_svg_style_t tGroupStyle;

					if ( (iClipTransformTop >= (XGE_SVG_STACK_MAX - 1)) || (iClipStyleTop >= (XGE_SVG_STACK_MAX - 1)) ) {
						__xgeSvgElementNodesClear(pSvg);
						xrtFree(sText);
						return XGE_ERROR_OUT_OF_MEMORY;
					}
					if ( __xgeSvgAttrCopy(p, pTagEnd, "transform", sTransform, sizeof(sTransform)) ) {
						xge_shape_ex_matrix_t tLocalTransform;

						if ( __xgeSvgParseTransform(sTransform, &tLocalTransform) ) {
							tTransform = __xgeSvgMatrixMul(tTransform, tLocalTransform);
						}
					}
					arrClipTransformStack[++iClipTransformTop] = tTransform;
					tGroupStyle = __xgeSvgStyleResolve(pSvg, &arrClipStyleStack[iClipStyleTop], p, pTagEnd);
					arrClipStyleStack[++iClipStyleTop] = tGroupStyle;
				}
				p = pTagEnd + 1;
				continue;
			}
				iRet = __xgeSvgParseClipShape(pSvg, iClipPathIndex, p, pTagEnd, arrClipTransformStack[iClipTransformTop], &arrClipStyleStack[iClipStyleTop]);
			if ( iRet != XGE_OK ) {
				__xgeSvgElementNodesClear(pSvg);
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
				__xgeSvgElementNodesClear(pSvg);
				xrtFree(sText);
				return iRet;
			}
			iClipTransformTop = 0;
			arrClipTransformStack[0] = __xgeSvgMatrixIdentity();
			iClipStyleTop = 0;
			arrClipStyleStack[0] = __xgeSvgStyleResolve(pSvg, &arrStack[iStackTop], p, pTagEnd);
			if ( bSelfClosing ) {
				iClipPathIndex = -1;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgTagNameEquals(p, "mask") ) {
			iRet = __xgeSvgParseMask(pSvg, p, pTagEnd, &iMaskIndex);
			if ( iRet != XGE_OK ) {
				__xgeSvgElementNodesClear(pSvg);
				xrtFree(sText);
				return iRet;
			}
			iMaskTransformTop = 0;
			arrMaskTransformStack[0] = __xgeSvgMatrixIdentity();
			iMaskStyleTop = 0;
			arrMaskStyleStack[0] = __xgeSvgStyleResolve(pSvg, &arrStack[iStackTop], p, pTagEnd);
			if ( bSelfClosing ) {
				iMaskIndex = -1;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgTagNameEquals(p, "filter") ) {
			iRet = __xgeSvgParseFilter(pSvg, p, pTagEnd, &iFilterIndex);
			if ( iRet != XGE_OK ) {
				__xgeSvgElementNodesClear(pSvg);
				xrtFree(sText);
				return iRet;
			}
			if ( bSelfClosing ) {
				iFilterIndex = -1;
			} else if ( iFilterIndex >= 0 ) {
				iRet = __xgeSvgElementStackPush(pSvg, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( iLinearGradientIndex >= 0 ) {
			if ( __xgeSvgTagNameEquals(p, "stop") ) {
				iRet = __xgeSvgParseGradientStop(pSvg, iLinearGradientIndex, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
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
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgTagNameEquals(p, "defs") ) {
			if ( !bSelfClosing ) {
				iRet = __xgeSvgElementStackPush(pSvg, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
				iDefsDepth++;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgTagNameEquals(p, "linearGradient") ) {
			iRet = __xgeSvgParseLinearGradient(pSvg, p, pTagEnd, &arrStack[iStackTop], &iLinearGradientIndex);
			if ( iRet != XGE_OK ) {
				__xgeSvgElementNodesClear(pSvg);
				xrtFree(sText);
				return iRet;
			}
			if ( bSelfClosing ) {
				iLinearGradientIndex = -1;
			} else {
				iRet = __xgeSvgElementStackPush(pSvg, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgTagNameEquals(p, "radialGradient") ) {
			iRet = __xgeSvgParseRadialGradient(pSvg, p, pTagEnd, &arrStack[iStackTop], &iRadialGradientIndex);
			if ( iRet != XGE_OK ) {
				__xgeSvgElementNodesClear(pSvg);
				xrtFree(sText);
				return iRet;
			}
			if ( bSelfClosing ) {
				iRadialGradientIndex = -1;
			} else {
				iRet = __xgeSvgElementStackPush(pSvg, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgIsCloseTagName(p, "g") ||
		     __xgeSvgIsCloseTagName(p, "symbol") ||
		     __xgeSvgIsCloseTagName(p, "marker") ||
		     __xgeSvgIsCloseTagName(p, "pattern") ||
		     __xgeSvgIsCloseTagName(p, "svg") ) {
			if ( __xgeSvgIsCloseTagName(p, "g") && (iStackTop > 0) ) {
				int iChildDef = arrMainIdDefStack[iStackTop];
				int iParentDef = arrGroupCopyParentDefStack[iStackTop];

				if ( (iChildDef >= 0) && (iChildDef < pSvg->iDefCount) &&
				     (iParentDef >= 0) && (iParentDef < pSvg->iDefCount) &&
				     (iChildDef != iParentDef) ) {
					iRet = __xgeSvgDefCopyContent(&pSvg->pDefs[iParentDef], &pSvg->pDefs[iChildDef]);
					if ( iRet != XGE_OK ) {
						__xgeSvgElementNodesClear(pSvg);
						xrtFree(sText);
						return iRet;
					}
				}
			}
			if ( (iStackTop > 0) && arrMainIdNormalizeCloseStack[iStackTop] ) {
				int iNormalizeDef;
				int iStartDef = arrMainIdNormalizeStartDefStack[iStackTop];

				if ( iStartDef < 0 ) {
					iStartDef = 0;
				}
				if ( iStartDef > pSvg->iDefCount ) {
					iStartDef = pSvg->iDefCount;
				}
				for ( iNormalizeDef = iStartDef; iNormalizeDef < pSvg->iDefCount; iNormalizeDef++ ) {
					iRet = __xgeSvgDefPrependTransform(&pSvg->pDefs[iNormalizeDef], arrMainIdNormalizeMatrixStack[iStackTop]);
					if ( iRet != XGE_OK ) {
						__xgeSvgElementNodesClear(pSvg);
						xrtFree(sText);
						return iRet;
					}
				}
			}
			if ( iStackTop > 0 ) {
				iStackTop--;
			}
			if ( __xgeSvgIsCloseTagName(p, "g") ) {
				__xgeSvgElementStackPop(pSvg, "g");
			} else if ( __xgeSvgIsCloseTagName(p, "symbol") ) {
				__xgeSvgElementStackPop(pSvg, "symbol");
			} else if ( __xgeSvgIsCloseTagName(p, "marker") ) {
				__xgeSvgElementStackPop(pSvg, "marker");
			} else if ( __xgeSvgIsCloseTagName(p, "pattern") ) {
				__xgeSvgElementStackPop(pSvg, "pattern");
			} else {
				__xgeSvgElementStackPop(pSvg, "svg");
			}
		} else if ( __xgeSvgTagNameEquals(p, "svg") ) {
			xge_svg_style_t tStyle;
			int iSvgDefIndex = arrDefStack[iStackTop];
			int iSvgMainIdDefIndex = arrMainIdDefStack[iStackTop];
			int bSvgDefCapture = 0;
			xge_shape_ex_matrix_t tSvgNormalizeMatrix = __xgeSvgMatrixIdentity();
			int bSvgNormalize = 0;
			int iSvgNormalizeStartDef = arrMainIdNormalizeStartDefStack[iStackTop];
			char sId[XGE_SVG_ATTR_MAX];

			tStyle = __xgeSvgStyleResolve(pSvg, &arrStack[iStackTop], p, pTagEnd);
			if ( iStackTop == 0 ) {
				__xgeSvgParseRoot(pSvg, p, pTagEnd, tStyle.fFontSize);
			}
			if ( (iStackTop > 0) && ((iDefsDepth > 0) || (arrDefStack[iStackTop] >= 0)) &&
			     __xgeSvgAttrCopy(p, pTagEnd, "id", sId, sizeof(sId)) && (sId[0] != '\0') ) {
				iRet = __xgeSvgDefGetOrCreate(pSvg, sId, &iSvgDefIndex);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
				__xgeSvgParseSymbolMeta(pSvg, iSvgDefIndex, p, pTagEnd);
				bSvgDefCapture = 1;
				iSvgMainIdDefIndex = -1;
			}
			if ( (iStackTop > 0) && !bSvgDefCapture ) {
				__xgeSvgApplyNestedSvgViewport(pSvg, &tStyle, p, pTagEnd);
				if ( (iDefsDepth <= 0) && (arrDefStack[iStackTop] < 0) &&
				     __xgeSvgAttrCopy(p, pTagEnd, "id", sId, sizeof(sId)) && (sId[0] != '\0') ) {
					iSvgNormalizeStartDef = pSvg->iDefCount;
					iRet = __xgeSvgDefGetOrCreate(pSvg, sId, &iSvgMainIdDefIndex);
					if ( iRet != XGE_OK ) {
						__xgeSvgElementNodesClear(pSvg);
						xrtFree(sText);
						return iRet;
					}
					__xgeSvgParseSymbolMeta(pSvg, iSvgMainIdDefIndex, p, pTagEnd);
					if ( !__xgeSvgMatrixInvert(tStyle.tTransform, &tSvgNormalizeMatrix) ) {
						iSvgMainIdDefIndex = arrMainIdDefStack[iStackTop];
					} else {
						bSvgNormalize = 1;
					}
				}
			}
			if ( !bSelfClosing ) {
				if ( iStackTop >= (XGE_SVG_STACK_MAX - 1) ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return XGE_ERROR_OUT_OF_MEMORY;
				}
				arrStack[++iStackTop] = tStyle;
				arrDefStack[iStackTop] = iSvgDefIndex;
				arrMainIdDefStack[iStackTop] = iSvgMainIdDefIndex;
				arrGroupCopyParentDefStack[iStackTop] = -1;
				arrMainIdNormalizeMatrixStack[iStackTop] = bSvgNormalize ? tSvgNormalizeMatrix : arrMainIdNormalizeMatrixStack[iStackTop - 1];
				arrMainIdNormalizeStack[iStackTop] = bSvgNormalize ? 1 : arrMainIdNormalizeStack[iStackTop - 1];
				arrMainIdNormalizeCloseStack[iStackTop] = bSvgNormalize;
				arrMainIdNormalizeStartDefStack[iStackTop] = bSvgNormalize ? iSvgNormalizeStartDef : arrMainIdNormalizeStartDefStack[iStackTop - 1];
				iRet = __xgeSvgElementStackPush(pSvg, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			}
		} else if ( __xgeSvgTagNameEquals(p, "g") || __xgeSvgTagNameEquals(p, "symbol") ) {
			xge_svg_style_t tStyle;
			int iDefIndex = arrDefStack[iStackTop];
			int iGroupMainIdDefIndex = arrMainIdDefStack[iStackTop];
			int iGroupCopyParentDefIndex = -1;
			int bCapture = (iDefsDepth > 0) || (iDefIndex >= 0) || __xgeSvgTagNameEquals(p, "symbol");
			char sId[XGE_SVG_ATTR_MAX];
			tStyle = __xgeSvgStyleResolve(pSvg, &arrStack[iStackTop], p, pTagEnd);
			if ( bCapture && __xgeSvgAttrCopy(p, pTagEnd, "id", sId, sizeof(sId)) && (sId[0] != '\0') ) {
				iRet = __xgeSvgDefGetOrCreate(pSvg, sId, &iDefIndex);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
				if ( __xgeSvgTagNameEquals(p, "symbol") ) {
					__xgeSvgParseSymbolMeta(pSvg, iDefIndex, p, pTagEnd);
				}
			} else if ( !bCapture && __xgeSvgTagNameEquals(p, "g") && __xgeSvgAttrCopy(p, pTagEnd, "id", sId, sizeof(sId)) && (sId[0] != '\0') ) {
				int iParentGroupMainIdDefIndex = iGroupMainIdDefIndex;

				iRet = __xgeSvgDefGetOrCreate(pSvg, sId, &iGroupMainIdDefIndex);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
				if ( (iParentGroupMainIdDefIndex >= 0) && (iParentGroupMainIdDefIndex != iGroupMainIdDefIndex) ) {
					iGroupCopyParentDefIndex = iParentGroupMainIdDefIndex;
				}
			}
			if ( !bSelfClosing ) {
				if ( iStackTop >= (XGE_SVG_STACK_MAX - 1) ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return XGE_ERROR_OUT_OF_MEMORY;
				}
				arrStack[++iStackTop] = tStyle;
				arrDefStack[iStackTop] = iDefIndex;
				arrMainIdDefStack[iStackTop] = iGroupMainIdDefIndex;
				arrGroupCopyParentDefStack[iStackTop] = iGroupCopyParentDefIndex;
				arrMainIdNormalizeMatrixStack[iStackTop] = arrMainIdNormalizeMatrixStack[iStackTop - 1];
				arrMainIdNormalizeStack[iStackTop] = arrMainIdNormalizeStack[iStackTop - 1];
				arrMainIdNormalizeCloseStack[iStackTop] = 0;
				arrMainIdNormalizeStartDefStack[iStackTop] = arrMainIdNormalizeStartDefStack[iStackTop - 1];
				iRet = __xgeSvgElementStackPush(pSvg, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			}
		} else if ( __xgeSvgTagNameEquals(p, "marker") ) {
			xge_svg_style_t tStyle;
			int iDefIndex;

			tStyle = __xgeSvgStyleResolve(pSvg, &arrStack[iStackTop], p, pTagEnd);
			iDefIndex = -1;
			iRet = __xgeSvgParseMarker(pSvg, p, pTagEnd, &tStyle, &iDefIndex);
			if ( iRet != XGE_OK ) {
				__xgeSvgElementNodesClear(pSvg);
				xrtFree(sText);
				return iRet;
			}
			if ( !bSelfClosing ) {
				if ( iStackTop >= (XGE_SVG_STACK_MAX - 1) ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return XGE_ERROR_OUT_OF_MEMORY;
				}
				arrStack[++iStackTop] = tStyle;
				arrDefStack[iStackTop] = iDefIndex;
				arrMainIdDefStack[iStackTop] = arrMainIdDefStack[iStackTop - 1];
				arrGroupCopyParentDefStack[iStackTop] = -1;
				arrMainIdNormalizeMatrixStack[iStackTop] = arrMainIdNormalizeMatrixStack[iStackTop - 1];
				arrMainIdNormalizeStack[iStackTop] = 0;
				arrMainIdNormalizeCloseStack[iStackTop] = 0;
				arrMainIdNormalizeStartDefStack[iStackTop] = arrMainIdNormalizeStartDefStack[iStackTop - 1];
				iRet = __xgeSvgElementStackPush(pSvg, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			}
		} else if ( __xgeSvgTagNameEquals(p, "pattern") ) {
			xge_svg_style_t tStyle;
			int iDefIndex;

			tStyle = __xgeSvgStyleResolve(pSvg, &arrStack[iStackTop], p, pTagEnd);
			iDefIndex = -1;
			iRet = __xgeSvgParsePattern(pSvg, p, pTagEnd, &tStyle, &iDefIndex);
			if ( iRet != XGE_OK ) {
				__xgeSvgElementNodesClear(pSvg);
				xrtFree(sText);
				return iRet;
			}
			if ( !bSelfClosing ) {
				if ( iStackTop >= (XGE_SVG_STACK_MAX - 1) ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return XGE_ERROR_OUT_OF_MEMORY;
				}
				arrStack[++iStackTop] = tStyle;
				arrDefStack[iStackTop] = iDefIndex;
				arrMainIdDefStack[iStackTop] = arrMainIdDefStack[iStackTop - 1];
				arrGroupCopyParentDefStack[iStackTop] = -1;
				arrMainIdNormalizeMatrixStack[iStackTop] = arrMainIdNormalizeMatrixStack[iStackTop - 1];
				arrMainIdNormalizeStack[iStackTop] = 0;
				arrMainIdNormalizeCloseStack[iStackTop] = 0;
				arrMainIdNormalizeStartDefStack[iStackTop] = arrMainIdNormalizeStartDefStack[iStackTop - 1];
				iRet = __xgeSvgElementStackPush(pSvg, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			}
		} else if ( __xgeSvgTagNameEquals(p, "use") ) {
			iRet = __xgeSvgParseUse(pSvg, p, pTagEnd, &arrStack[iStackTop], (iDefsDepth > 0) || (arrDefStack[iStackTop] >= 0), arrDefStack[iStackTop], arrMainIdDefStack[iStackTop]);
			if ( iRet != XGE_OK ) {
				__xgeSvgElementNodesClear(pSvg);
				xrtFree(sText);
				return iRet;
			}
		} else if ( __xgeSvgTagNameEquals(p, "image") ) {
			iRet = __xgeSvgParseImage(pSvg, p, pTagEnd, &arrStack[iStackTop], (iDefsDepth > 0) || (arrDefStack[iStackTop] >= 0), arrDefStack[iStackTop], arrMainIdDefStack[iStackTop]);
			if ( iRet != XGE_OK ) {
				__xgeSvgElementNodesClear(pSvg);
				xrtFree(sText);
				return iRet;
			}
		} else if ( __xgeSvgTagNameEquals(p, "text") ) {
			const char* pCloseText;
			const char* pCloseTextEnd;
			const char* pTextEnd;

			pCloseText = NULL;
			pCloseTextEnd = NULL;
			pTextEnd = pTagEnd;
			if ( !bSelfClosing ) {
				pCloseText = __xgeSvgFindCloseElement(pTagEnd + 1, "text", &pCloseTextEnd);
				if ( pCloseText != NULL ) {
					pTextEnd = pCloseText;
				}
			}
			iRet = __xgeSvgParseText(pSvg, p, pTagEnd, pTagEnd + 1, pTextEnd, &arrStack[iStackTop], (iDefsDepth > 0) || (arrDefStack[iStackTop] >= 0), arrDefStack[iStackTop], arrMainIdDefStack[iStackTop]);
			if ( iRet != XGE_OK ) {
				__xgeSvgElementNodesClear(pSvg);
				xrtFree(sText);
				return iRet;
			}
			if ( (pCloseText != NULL) && (pCloseTextEnd != NULL) ) {
				p = pCloseTextEnd + 1;
				continue;
			}
		} else {
			iRet = __xgeSvgParseElement(pSvg, p, pTagEnd, &arrStack[iStackTop], (iDefsDepth > 0) || (arrDefStack[iStackTop] >= 0), arrDefStack[iStackTop], arrMainIdDefStack[iStackTop]);
			if ( iRet != XGE_OK ) {
				__xgeSvgElementNodesClear(pSvg);
				xrtFree(sText);
				return iRet;
			}
		}
		p = pTagEnd + 1;
	}
	iRet = __xgeSvgResolveGradients(pSvg);
	if ( iRet != XGE_OK ) {
		__xgeSvgElementNodesClear(pSvg);
		xrtFree(sText);
		return iRet;
	}
	iRet = __xgeSvgResolvePendingUses(pSvg);
	if ( iRet != XGE_OK ) {
		__xgeSvgElementNodesClear(pSvg);
		xrtFree(sText);
		return iRet;
	}
	__xgeSvgElementNodesClear(pSvg);
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

int xgeSvgLoadMemory(xge_svg pSvg, const void* pData, int iSize)
{
	return __xgeSvgLoadMemoryEx(pSvg, pData, iSize, NULL);
}

int xgeSvgLoad(xge_svg pSvg, const char* sURI)
{
	xge_resource_t tResource;
	char* sBaseDir;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (sURI == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sBaseDir = __xgeSvgUriBaseDir(sURI);
	if ( sBaseDir == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeResourceLoad(sURI, &tResource);
	if ( iRet != XGE_OK ) {
		xrtFree(sBaseDir);
		return iRet;
	}
	iRet = __xgeSvgLoadMemoryEx(pSvg, tResource.pData, tResource.iSize, sBaseDir);
	xgeResourceFree(&tResource);
	xrtFree(sBaseDir);
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

static int __xgeSvgDrawInternal(xge_svg pSvg, xge_rect_t tDst, float fTolerance, int bScreenSpace);

static xge_vec2_t __xgeSvgMatrixVector(xge_shape_ex_matrix_t tMatrix, xge_vec2_t tVector)
{
	xge_vec2_t tOut;

	if ( xgeShapeExMatrixVector(&tOut, &tMatrix, tVector) != XGE_OK ) {
		tOut = tVector;
	}
	return tOut;
}

static float __xgeSvgTextActualAdvance(xge_font pFont, const char* sText, const xge_svg_style_t* pStyle, float fScale, float fTextScaleX, float fTextExtraSpacing)
{
	const char* p;
	uint32_t iCodepoint;
	float fAdvance;

	if ( (pFont == NULL) || (sText == NULL) || (pStyle == NULL) ) {
		return 0.0f;
	}
	if ( fScale <= 0.0001f ) {
		fScale = 1.0f;
	}
	p = sText;
	fAdvance = 0.0f;
	if ( fTextScaleX <= 0.0001f ) {
		fTextScaleX = 1.0f;
	}
	while ( *p != '\0' ) {
		xge_glyph_metrics_t tMetrics;
		int bHasNext;

		if ( xgeTextUTF8Next(&p, &iCodepoint) != XGE_OK ) {
			break;
		}
		if ( iCodepoint == '\n' ) {
			break;
		}
		bHasNext = *p != '\0';
		if ( xgeFontGlyphGet(pFont, iCodepoint, &tMetrics) == XGE_OK ) {
			fAdvance += tMetrics.fAdvanceX * fTextScaleX;
		}
		fAdvance += (__xgeSvgTextSpacingForCodepoint(pStyle, iCodepoint, bHasNext) + (bHasNext ? fTextExtraSpacing : 0.0f)) * fScale;
	}
	return fAdvance;
}

static float __xgeSvgTextBaselineOffsetPx(xge_font pFont, int iBaseline)
{
	float fAscent;
	float fDescent;

	if ( pFont == NULL ) {
		return 0.0f;
	}
	fAscent = pFont->fAscent;
	fDescent = pFont->fDescent;
	switch ( iBaseline ) {
	case XGE_SVG_TEXT_BASELINE_MIDDLE:
		return (fAscent + fDescent) * 0.5f;
	case XGE_SVG_TEXT_BASELINE_HANGING:
	case XGE_SVG_TEXT_BASELINE_TEXT_BEFORE_EDGE:
		return fAscent;
	case XGE_SVG_TEXT_BASELINE_TEXT_AFTER_EDGE:
		return fDescent;
	case XGE_SVG_TEXT_BASELINE_ALPHABETIC:
	default:
		return 0.0f;
	}
}

static int __xgeSvgDrawTextDecorationLine(float fX0, float fX1, float fY, float fWidth, uint32_t iColor, xge_shape_ex_matrix_t tMatrix, int bScreenSpace)
{
	xge_shape_ex pLine;
	int iRet;

	if ( (fX1 <= fX0) || (fWidth <= 0.0f) || (XGE_COLOR_GET_A(iColor) <= 0) ) {
		return XGE_OK;
	}
	pLine = NULL;
	iRet = xgeShapeExCreate(&pLine);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( iRet == XGE_OK ) iRet = xgeShapeExMoveTo(pLine, fX0, fY);
	if ( iRet == XGE_OK ) iRet = xgeShapeExLineTo(pLine, fX1, fY);
	if ( iRet == XGE_OK ) iRet = xgeShapeExFillColor(pLine, XGE_COLOR_RGBA(0, 0, 0, 0));
	if ( iRet == XGE_OK ) iRet = xgeShapeExStrokeColor(pLine, iColor);
	if ( iRet == XGE_OK ) iRet = xgeShapeExStrokeWidth(pLine, fWidth);
	if ( iRet == XGE_OK ) iRet = xgeShapeExStrokeCap(pLine, XGE_SHAPE_EX_CAP_BUTT);
	if ( iRet == XGE_OK ) {
		iRet = bScreenSpace ? xgeShapeExDrawPxEx(pLine, 0.25f, &tMatrix, 1.0f) : xgeShapeExDrawEx(pLine, 0.25f, &tMatrix, 1.0f);
	}
	xgeShapeExDestroy(pLine);
	return iRet;
}

static int __xgeSvgDrawTextDecorations(xge_font pFont, const xge_svg_text_item_t* pText, xge_shape_ex_matrix_t tMatrix, float fScale, float fX0, float fX1, float fBaselineY, uint32_t iColor, int bScreenSpace)
{
	float fLinePx;
	float fLineWidth;
	int iRet;

	if ( (pFont == NULL) || (pText == NULL) || (pText->tStyle.iTextDecoration == 0) ) {
		return XGE_OK;
	}
	if ( fScale <= 0.0001f ) {
		fScale = 1.0f;
	}
	fLinePx = pFont->fLineHeight * 0.06f;
	if ( fLinePx < 1.0f ) {
		fLinePx = 1.0f;
	}
	fLineWidth = fLinePx / fScale;
	if ( pText->tStyle.iTextDecoration & XGE_SVG_TEXT_DECORATION_OVERLINE ) {
		iRet = __xgeSvgDrawTextDecorationLine(fX0, fX1, fBaselineY - (pFont->fAscent / fScale), fLineWidth, iColor, tMatrix, bScreenSpace);
		if ( iRet != XGE_OK ) return iRet;
	}
	if ( pText->tStyle.iTextDecoration & XGE_SVG_TEXT_DECORATION_LINE_THROUGH ) {
		iRet = __xgeSvgDrawTextDecorationLine(fX0, fX1, fBaselineY - ((pFont->fAscent * 0.35f) / fScale), fLineWidth, iColor, tMatrix, bScreenSpace);
		if ( iRet != XGE_OK ) return iRet;
	}
	if ( pText->tStyle.iTextDecoration & XGE_SVG_TEXT_DECORATION_UNDERLINE ) {
		iRet = __xgeSvgDrawTextDecorationLine(fX0, fX1, fBaselineY + fLineWidth, fLineWidth, iColor, tMatrix, bScreenSpace);
		if ( iRet != XGE_OK ) return iRet;
	}
	return XGE_OK;
}

static int __xgeSvgDrawTextPathDecorationLine(xge_shape_ex_path_measure pMeasure, float fPathLength, xge_shape_ex_matrix_t tMatrix, float fScale, float fStart, float fEnd, float fOffset, float fWidth, uint32_t iColor, int bScreenSpace)
{
	xge_shape_ex pLine;
	int iSegments;
	int i;
	int iRet;
	int bStarted;

	if ( (pMeasure == NULL) || (fPathLength <= 0.0001f) || (fWidth <= 0.0f) || (XGE_COLOR_GET_A(iColor) <= 0) ) {
		return XGE_OK;
	}
	if ( fStart < 0.0f ) fStart = 0.0f;
	if ( fEnd > fPathLength ) fEnd = fPathLength;
	if ( fEnd <= fStart ) {
		return XGE_OK;
	}
	if ( fScale <= 0.0001f ) {
		fScale = 1.0f;
	}
	iSegments = (int)ceilf(((fEnd - fStart) * fScale) / 3.0f);
	if ( iSegments < 2 ) iSegments = 2;
	if ( iSegments > 256 ) iSegments = 256;
	pLine = NULL;
	iRet = xgeShapeExCreate(&pLine);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	bStarted = 0;
	for ( i = 0; i <= iSegments; i++ ) {
		float fDistance;
		xge_vec2_t tPoint;
		xge_vec2_t tTangent;
		xge_vec2_t tNormal;
		float fTangentLen;

		fDistance = fStart + ((fEnd - fStart) * (float)i / (float)iSegments);
		if ( xgeShapeExPathMeasureGetPointAtLength(pMeasure, fDistance, &tPoint, &tTangent) != XGE_OK ) {
			continue;
		}
		tPoint = __xgeSvgMatrixPoint(tMatrix, tPoint);
		tTangent = __xgeSvgMatrixVector(tMatrix, tTangent);
		fTangentLen = hypotf(tTangent.fX, tTangent.fY);
		if ( fTangentLen <= 0.0001f ) {
			continue;
		}
		tTangent.fX /= fTangentLen;
		tTangent.fY /= fTangentLen;
		tNormal.fX = -tTangent.fY;
		tNormal.fY = tTangent.fX;
		tPoint.fX += tNormal.fX * fOffset;
		tPoint.fY += tNormal.fY * fOffset;
		if ( !bStarted ) {
			iRet = xgeShapeExMoveTo(pLine, tPoint.fX, tPoint.fY);
			bStarted = 1;
		} else {
			iRet = xgeShapeExLineTo(pLine, tPoint.fX, tPoint.fY);
		}
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pLine);
			return iRet;
		}
	}
	if ( !bStarted ) {
		xgeShapeExDestroy(pLine);
		return XGE_OK;
	}
	if ( iRet == XGE_OK ) iRet = xgeShapeExFillColor(pLine, XGE_COLOR_RGBA(0, 0, 0, 0));
	if ( iRet == XGE_OK ) iRet = xgeShapeExStrokeColor(pLine, iColor);
	if ( iRet == XGE_OK ) iRet = xgeShapeExStrokeWidth(pLine, fWidth);
	if ( iRet == XGE_OK ) iRet = xgeShapeExStrokeCap(pLine, XGE_SHAPE_EX_CAP_BUTT);
	if ( iRet == XGE_OK ) {
		xge_shape_ex_matrix_t tIdentity = __xgeSvgMatrixIdentity();

		iRet = bScreenSpace ? xgeShapeExDrawPxEx(pLine, 0.25f, &tIdentity, 1.0f) : xgeShapeExDrawEx(pLine, 0.25f, &tIdentity, 1.0f);
	}
	xgeShapeExDestroy(pLine);
	return iRet;
}

static int __xgeSvgDrawTextPathDecorations(xge_font pFont, const xge_svg_text_item_t* pText, xge_shape_ex_path_measure pMeasure, float fPathLength, xge_shape_ex_matrix_t tMatrix, float fScale, float fStart, float fEnd, float fBaselineOffset, uint32_t iColor, int bScreenSpace)
{
	float fLineWidth;
	int iRet;

	if ( (pFont == NULL) || (pText == NULL) || (pText->tStyle.iTextDecoration == 0) ) {
		return XGE_OK;
	}
	fLineWidth = pFont->fLineHeight * 0.06f;
	if ( fLineWidth < 1.0f ) {
		fLineWidth = 1.0f;
	}
	if ( pText->tStyle.iTextDecoration & XGE_SVG_TEXT_DECORATION_OVERLINE ) {
		iRet = __xgeSvgDrawTextPathDecorationLine(pMeasure, fPathLength, tMatrix, fScale, fStart, fEnd, fBaselineOffset - pFont->fAscent, fLineWidth, iColor, bScreenSpace);
		if ( iRet != XGE_OK ) return iRet;
	}
	if ( pText->tStyle.iTextDecoration & XGE_SVG_TEXT_DECORATION_LINE_THROUGH ) {
		iRet = __xgeSvgDrawTextPathDecorationLine(pMeasure, fPathLength, tMatrix, fScale, fStart, fEnd, fBaselineOffset - (pFont->fAscent * 0.35f), fLineWidth, iColor, bScreenSpace);
		if ( iRet != XGE_OK ) return iRet;
	}
	if ( pText->tStyle.iTextDecoration & XGE_SVG_TEXT_DECORATION_UNDERLINE ) {
		iRet = __xgeSvgDrawTextPathDecorationLine(pMeasure, fPathLength, tMatrix, fScale, fStart, fEnd, fBaselineOffset + fLineWidth, fLineWidth, iColor, bScreenSpace);
		if ( iRet != XGE_OK ) return iRet;
	}
	return XGE_OK;
}

static void __xgeSvgDrawTextPathGlyph(xge_font pFont, const xge_glyph_t* pGlyph, xge_vec2_t tBaseline, xge_vec2_t tTangent, float fTextScaleX, uint32_t iColor, uint32_t iDrawFlags)
{
	xge_glyph_atlas_page_t* pPages;
	xge_draw_t tDraw;
	xge_vec2_t tNormal;
	xge_vec2_t tTopLeft;

	if ( (pFont == NULL) || (pGlyph == NULL) || (pGlyph->iPage < 0) || (pGlyph->iWidth <= 0) || (pGlyph->iHeight <= 0) ) {
		return;
	}
	pPages = (xge_glyph_atlas_page_t*)pFont->tAtlas.pPages;
	if ( (pPages == NULL) || (__xgeFontAtlasUploadPage(pFont, pGlyph->iPage) != XGE_OK) ) {
		return;
	}
	if ( fTextScaleX <= 0.0001f ) {
		fTextScaleX = 1.0f;
	}
	tNormal.fX = -tTangent.fY;
	tNormal.fY = tTangent.fX;
	tTopLeft.fX = tBaseline.fX + (tTangent.fX * pGlyph->fOffsetX * fTextScaleX) + (tNormal.fX * pGlyph->fOffsetY);
	tTopLeft.fY = tBaseline.fY + (tTangent.fY * pGlyph->fOffsetX * fTextScaleX) + (tNormal.fY * pGlyph->fOffsetY);
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = &pPages[pGlyph->iPage].tTexture;
	tDraw.tSrc.fX = (float)pGlyph->iX;
	tDraw.tSrc.fY = (float)pGlyph->iY;
	tDraw.tSrc.fW = (float)pGlyph->iWidth;
	tDraw.tSrc.fH = (float)pGlyph->iHeight;
	tDraw.tDst.fX = tTopLeft.fX;
	tDraw.tDst.fY = tTopLeft.fY;
	tDraw.tDst.fW = (float)pGlyph->iWidth * fTextScaleX;
	tDraw.tDst.fH = (float)pGlyph->iHeight;
	tDraw.fRotation = atan2f(tTangent.fY, tTangent.fX);
	tDraw.iColor = iColor;
	tDraw.iFlags = iDrawFlags;
	xgeDrawEx(&tDraw);
}

static void __xgeSvgDrawGlyphAffine(xge_font pFont, const xge_glyph_t* pGlyph, xge_shape_ex_matrix_t tMatrix, float fScale, float fX, float fBaselineY, float fTextScaleX, uint32_t iColor, uint32_t iDrawFlags)
{
	xge_glyph_atlas_page_t* pPages;
	xge_texture pTexture;
	xge_vertex_t tVertices[4];
	xge_vec2_t tP[4];
	float fX0;
	float fY0;
	float fX1;
	float fY1;
	float fU0;
	float fV0;
	float fU1;
	float fV1;

	if ( (pFont == NULL) || (pGlyph == NULL) || (pGlyph->iPage < 0) || (pGlyph->iWidth <= 0) || (pGlyph->iHeight <= 0) ) {
		return;
	}
	if ( fScale <= 0.0001f ) {
		fScale = 1.0f;
	}
	pPages = (xge_glyph_atlas_page_t*)pFont->tAtlas.pPages;
	if ( (pPages == NULL) || (__xgeFontAtlasUploadPage(pFont, pGlyph->iPage) != XGE_OK) ) {
		return;
	}
	pTexture = &pPages[pGlyph->iPage].tTexture;
	if ( (pTexture == NULL) || (pTexture->iWidth <= 0) || (pTexture->iHeight <= 0) ) {
		return;
	}
	if ( fTextScaleX <= 0.0001f ) {
		fTextScaleX = 1.0f;
	}
	fX0 = fX + ((pGlyph->fOffsetX * fTextScaleX) / fScale);
	fY0 = fBaselineY + (pGlyph->fOffsetY / fScale);
	fX1 = fX0 + (((float)pGlyph->iWidth * fTextScaleX) / fScale);
	fY1 = fY0 + ((float)pGlyph->iHeight / fScale);
	tP[0].fX = fX0; tP[0].fY = fY0;
	tP[1].fX = fX1; tP[1].fY = fY0;
	tP[2].fX = fX0; tP[2].fY = fY1;
	tP[3].fX = fX1; tP[3].fY = fY1;
	tP[0] = __xgeSvgMatrixPoint(tMatrix, tP[0]);
	tP[1] = __xgeSvgMatrixPoint(tMatrix, tP[1]);
	tP[2] = __xgeSvgMatrixPoint(tMatrix, tP[2]);
	tP[3] = __xgeSvgMatrixPoint(tMatrix, tP[3]);
	fU0 = (float)pGlyph->iX / (float)pTexture->iWidth;
	fV0 = (float)pGlyph->iY / (float)pTexture->iHeight;
	fU1 = (float)(pGlyph->iX + pGlyph->iWidth) / (float)pTexture->iWidth;
	fV1 = (float)(pGlyph->iY + pGlyph->iHeight) / (float)pTexture->iHeight;
	tVertices[0] = (xge_vertex_t){tP[0].fX, tP[0].fY, 0.0f, 1.0f, fU0, fV0, iColor};
	tVertices[1] = (xge_vertex_t){tP[1].fX, tP[1].fY, 0.0f, 1.0f, fU1, fV0, iColor};
	tVertices[2] = (xge_vertex_t){tP[2].fX, tP[2].fY, 0.0f, 1.0f, fU0, fV1, iColor};
	tVertices[3] = (xge_vertex_t){tP[3].fX, tP[3].fY, 0.0f, 1.0f, fU1, fV1, iColor};
	xgeDrawQuad3D(pTexture, tVertices, iDrawFlags);
}

static int __xgeSvgDrawTextPathItem(xge_svg pSvg, const xge_svg_text_item_t* pText, xge_shape_ex_matrix_t tParent, int bScreenSpace)
{
	xge_shape_ex_path_measure pMeasure;
	xge_font pFont;
	xge_shape_ex_matrix_t tMatrix;
	const char* p;
	float fScaleX;
	float fScaleY;
	float fScale;
	float fFontSize;
	float fPathLength;
	float fTextAdvance;
	float fPathDistance;
	float fDecorStartDistance;
	float fBaselineOffset;
	float fTextScaleX;
	float fTextExtraSpacing;
	uint32_t iColor;
	uint32_t iDrawFlags;
	uint32_t iCodepoint;
	int iRet;

	if ( (pText == NULL) || (pText->sText == NULL) || (pText->sText[0] == '\0') || (pText->pTextPath == NULL) ) {
		return XGE_OK;
	}
	if ( !pText->tStyle.bVisible || !pText->tStyle.bVisibility ) {
		return XGE_OK;
	}
	iColor = __xgeSvgColorAlpha(pText->tStyle.iFillColor, pText->tStyle.fOpacity * pText->tStyle.fFillOpacity);
	if ( XGE_COLOR_GET_A(iColor) <= 0 ) {
		return XGE_OK;
	}
	tMatrix = __xgeSvgMatrixMul(tParent, pText->tStyle.tTransform);
	fScaleX = sqrtf(tMatrix.fA * tMatrix.fA + tMatrix.fB * tMatrix.fB);
	fScaleY = sqrtf(tMatrix.fC * tMatrix.fC + tMatrix.fD * tMatrix.fD);
	fScale = (fScaleX + fScaleY) * 0.5f;
	if ( fScale <= 0.0001f ) {
		fScale = 1.0f;
	}
	fFontSize = pText->tStyle.fFontSize * fScale;
	if ( fFontSize <= 0.25f ) {
		return XGE_OK;
	}
	pFont = NULL;
	iRet = __xgeSvgFontCacheGet(pSvg, fFontSize, &pFont);
	if ( iRet != XGE_OK ) {
		return XGE_OK;
	}
	fTextScaleX = (pText->fTextScaleX > 0.0001f) ? pText->fTextScaleX : 1.0f;
	fTextExtraSpacing = pText->fTextExtraSpacing;
	pMeasure = NULL;
	iRet = xgeShapeExPathMeasureCreate(&pMeasure, pText->pTextPath, &pText->pTextPath->tTransform, 0.25f);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeShapeExPathMeasureGetLength(pMeasure, &fPathLength);
	if ( iRet != XGE_OK ) {
		xgeShapeExPathMeasureDestroy(pMeasure);
		return iRet;
	}
	if ( fPathLength <= 0.0001f ) {
		xgeShapeExPathMeasureDestroy(pMeasure);
		return XGE_OK;
	}
	fTextAdvance = __xgeSvgTextActualAdvance(pFont, pText->sText, &pText->tStyle, fScale, fTextScaleX, fTextExtraSpacing) / fScale;
	fPathDistance = pText->fTextPathOffset;
	if ( pText->tStyle.iTextAnchor == XGE_SVG_TEXT_ANCHOR_MIDDLE ) {
		fPathDistance -= ((pText->fAnchorAdvance > 0.0f) ? pText->fAnchorAdvance : fTextAdvance) * 0.5f;
	} else if ( pText->tStyle.iTextAnchor == XGE_SVG_TEXT_ANCHOR_END ) {
		fPathDistance -= (pText->fAnchorAdvance > 0.0f) ? pText->fAnchorAdvance : fTextAdvance;
	}
	fDecorStartDistance = fPathDistance;
	fBaselineOffset = __xgeSvgTextBaselineOffsetPx(pFont, pText->tStyle.iTextBaseline);
	iDrawFlags = bScreenSpace ? XGE_DRAW_SCREEN_SPACE : 0u;
	p = pText->sText;
	while ( *p != '\0' ) {
		xge_glyph_t tGlyph;
		xge_glyph_metrics_t tMetrics;
		xge_vec2_t tPathPoint;
		xge_vec2_t tPathTangent;
		xge_vec2_t tPoint;
		xge_vec2_t tTangent;
		xge_vec2_t tBaseline;
		float fAdvance;
		float fGlyphAdvance;
		float fSpacing;
		float fCenterDistance;
		float fTangentLen;
		int bHasNext;

		if ( xgeTextUTF8Next(&p, &iCodepoint) != XGE_OK ) {
			break;
		}
		if ( iCodepoint == '\n' ) {
			break;
		}
		bHasNext = *p != '\0';
		if ( xgeFontGlyphAtlasGet(pFont, iCodepoint, &tGlyph) == XGE_OK ) {
			fGlyphAdvance = tGlyph.fAdvanceX / fScale;
		} else if ( xgeFontGlyphGet(pFont, iCodepoint, &tMetrics) == XGE_OK ) {
			fPathDistance += ((tMetrics.fAdvanceX / fScale) * fTextScaleX) + __xgeSvgTextSpacingForCodepoint(&pText->tStyle, iCodepoint, bHasNext) + (bHasNext ? fTextExtraSpacing : 0.0f);
			continue;
		} else {
			fPathDistance += __xgeSvgTextSpacingForCodepoint(&pText->tStyle, iCodepoint, bHasNext) + (bHasNext ? fTextExtraSpacing : 0.0f);
			continue;
		}
		fSpacing = __xgeSvgTextSpacingForCodepoint(&pText->tStyle, iCodepoint, bHasNext) + (bHasNext ? fTextExtraSpacing : 0.0f);
		fAdvance = (fGlyphAdvance * fTextScaleX) + fSpacing;
		fCenterDistance = fPathDistance + (fGlyphAdvance * fTextScaleX * 0.5f);
		if ( fCenterDistance > (fPathLength + 0.001f) ) {
			break;
		}
		if ( fCenterDistance < 0.0f ) {
			fPathDistance += fAdvance;
			continue;
		}
		if ( xgeShapeExPathMeasureGetPointAtLength(pMeasure, fCenterDistance, &tPathPoint, &tPathTangent) != XGE_OK ) {
			break;
		}
		tPoint = __xgeSvgMatrixPoint(tMatrix, tPathPoint);
		tTangent = __xgeSvgMatrixVector(tMatrix, tPathTangent);
		fTangentLen = hypotf(tTangent.fX, tTangent.fY);
		if ( fTangentLen <= 0.0001f ) {
			tTangent.fX = 1.0f;
			tTangent.fY = 0.0f;
		} else {
			tTangent.fX /= fTangentLen;
			tTangent.fY /= fTangentLen;
		}
		tBaseline.fX = tPoint.fX - (tTangent.fX * tGlyph.fAdvanceX * fTextScaleX * 0.5f);
		tBaseline.fY = tPoint.fY - (tTangent.fY * tGlyph.fAdvanceX * fTextScaleX * 0.5f);
		if ( fBaselineOffset != 0.0f ) {
			tBaseline.fX += -tTangent.fY * fBaselineOffset;
			tBaseline.fY += tTangent.fX * fBaselineOffset;
		}
		__xgeSvgDrawTextPathGlyph(pFont, &tGlyph, tBaseline, tTangent, fTextScaleX, iColor, iDrawFlags);
		fPathDistance += fAdvance;
	}
	iRet = __xgeSvgDrawTextPathDecorations(pFont, pText, pMeasure, fPathLength, tMatrix, fScale, fDecorStartDistance, fPathDistance, fBaselineOffset, iColor, bScreenSpace);
	if ( iRet != XGE_OK ) {
		xgeShapeExPathMeasureDestroy(pMeasure);
		return iRet;
	}
	xgeShapeExPathMeasureDestroy(pMeasure);
	return XGE_OK;
}

static int __xgeSvgDrawTextItem(xge_svg pSvg, const xge_svg_text_item_t* pText, xge_shape_ex_matrix_t tParent, int bScreenSpace)
{
	xge_font pFont;
	xge_shape_ex_matrix_t tMatrix;
	const char* p;
	xge_glyph_t tGlyph;
	uint32_t iColor;
	uint32_t iCodepoint;
	uint32_t iDrawFlags;
	float fScaleX;
	float fScaleY;
	float fScale;
	float fFontSize;
	float fAnchorWidth;
	float fTextAdvance;
	float fPenX;
	float fDecorX0;
	float fBaselineY;
	float fTextScaleX;
	float fTextExtraSpacing;
	int iRet;

	if ( (pText == NULL) || (pText->sText == NULL) || (pText->sText[0] == '\0') ) {
		return XGE_OK;
	}
	if ( pText->pTextPath != NULL ) {
		return __xgeSvgDrawTextPathItem(pSvg, pText, tParent, bScreenSpace);
	}
	if ( !pText->tStyle.bVisible || !pText->tStyle.bVisibility ) {
		return XGE_OK;
	}
	iColor = __xgeSvgColorAlpha(pText->tStyle.iFillColor, pText->tStyle.fOpacity * pText->tStyle.fFillOpacity);
	if ( XGE_COLOR_GET_A(iColor) <= 0 ) {
		return XGE_OK;
	}
	tMatrix = __xgeSvgMatrixMul(tParent, pText->tStyle.tTransform);
	fScaleX = sqrtf(tMatrix.fA * tMatrix.fA + tMatrix.fB * tMatrix.fB);
	fScaleY = sqrtf(tMatrix.fC * tMatrix.fC + tMatrix.fD * tMatrix.fD);
	fScale = (fScaleX + fScaleY) * 0.5f;
	if ( fScale <= 0.0001f ) {
		fScale = 1.0f;
	}
	fFontSize = pText->tStyle.fFontSize * fScale;
	if ( fFontSize <= 0.25f ) {
		return XGE_OK;
	}
	pFont = NULL;
	iRet = __xgeSvgFontCacheGet(pSvg, fFontSize, &pFont);
	if ( iRet != XGE_OK ) {
		return XGE_OK;
	}
	fTextScaleX = (pText->fTextScaleX > 0.0001f) ? pText->fTextScaleX : 1.0f;
	fTextExtraSpacing = pText->fTextExtraSpacing;
	fTextAdvance = __xgeSvgTextActualAdvance(pFont, pText->sText, &pText->tStyle, fScale, fTextScaleX, fTextExtraSpacing) / fScale;
	fAnchorWidth = (pText->fAnchorAdvance > 0.0f) ? pText->fAnchorAdvance : fTextAdvance;
	fPenX = pText->fX;
	fBaselineY = pText->fY;
	fBaselineY += __xgeSvgTextBaselineOffsetPx(pFont, pText->tStyle.iTextBaseline) / fScale;
	if ( pText->tStyle.iTextAnchor == XGE_SVG_TEXT_ANCHOR_MIDDLE ) {
		fPenX -= fAnchorWidth * 0.5f;
	} else if ( pText->tStyle.iTextAnchor == XGE_SVG_TEXT_ANCHOR_END ) {
		fPenX -= fAnchorWidth;
	}
	fDecorX0 = fPenX;
	iDrawFlags = bScreenSpace ? XGE_DRAW_SCREEN_SPACE : 0u;
	p = pText->sText;
	while ( *p != '\0' ) {
		xge_glyph_metrics_t tMetrics;
		int bHasNext;

		if ( xgeTextUTF8Next(&p, &iCodepoint) != XGE_OK ) {
			break;
		}
		if ( iCodepoint == '\n' ) {
			break;
		}
		bHasNext = *p != '\0';
		if ( xgeFontGlyphAtlasGet(pFont, iCodepoint, &tGlyph) != XGE_OK ) {
			if ( xgeFontGlyphGet(pFont, iCodepoint, &tMetrics) == XGE_OK ) {
				fPenX += ((tMetrics.fAdvanceX / fScale) * fTextScaleX) + __xgeSvgTextSpacingForCodepoint(&pText->tStyle, iCodepoint, bHasNext) + (bHasNext ? fTextExtraSpacing : 0.0f);
			} else {
				fPenX += __xgeSvgTextSpacingForCodepoint(&pText->tStyle, iCodepoint, bHasNext) + (bHasNext ? fTextExtraSpacing : 0.0f);
			}
			continue;
		}
		__xgeSvgDrawGlyphAffine(pFont, &tGlyph, tMatrix, fScale, fPenX, fBaselineY, fTextScaleX, iColor, iDrawFlags);
		fPenX += ((tGlyph.fAdvanceX / fScale) * fTextScaleX) + __xgeSvgTextSpacingForCodepoint(&pText->tStyle, iCodepoint, bHasNext) + (bHasNext ? fTextExtraSpacing : 0.0f);
	}
	iRet = __xgeSvgDrawTextDecorations(pFont, pText, tMatrix, fScale, fDecorX0, fPenX, fBaselineY, iColor, bScreenSpace);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	return XGE_OK;
}

static int __xgeSvgDrawImageItem(const xge_svg_image_item_t* pImage, xge_shape_ex_matrix_t tParent, float fTolerance, int bScreenSpace)
{
	xge_shape_ex_matrix_t tMatrix;
	xge_rect_t tDst;

	if ( (pImage == NULL) || !__xgeSvgValid(pImage->pSvg) ) {
		return XGE_OK;
	}
	if ( !pImage->tStyle.bVisible || !pImage->tStyle.bVisibility || (pImage->tStyle.fOpacity <= 0.0f) ) {
		return XGE_OK;
	}
	tMatrix = __xgeSvgMatrixMul(tParent, pImage->tStyle.tTransform);
	tDst = __xgeSvgMatrixRectBounds(tMatrix, pImage->tRect);
	if ( (tDst.fW <= 0.0f) || (tDst.fH <= 0.0f) ) {
		return XGE_OK;
	}
	return __xgeSvgDrawInternal(pImage->pSvg, tDst, fTolerance, bScreenSpace);
}

static int __xgeSvgDrawRasterImageItem(xge_svg_raster_item_t* pImage, xge_shape_ex_matrix_t tParent, int bScreenSpace)
{
	xge_shape_ex_matrix_t tMatrix;
	xge_draw_t tDraw;
	xge_rect_t tImageBox;
	xge_rect_t tLocalDst;
	xge_rect_t tDst;
	uint32_t iColor;
	int iAlpha;
	int iRet;

	if ( (pImage == NULL) || (pImage->pRaster == NULL) ) {
		return XGE_OK;
	}
	if ( !pImage->tStyle.bVisible || !pImage->tStyle.bVisibility || (pImage->tStyle.fOpacity <= 0.0f) ) {
		return XGE_OK;
	}
	if ( (pImage->pRaster->tImage.iWidth <= 0) || (pImage->pRaster->tImage.iHeight <= 0) ) {
		return XGE_OK;
	}
	iRet = __xgeSvgRasterImageEnsureTexture(pImage->pRaster);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tMatrix = __xgeSvgMatrixMul(tParent, pImage->tStyle.tTransform);
	tImageBox.fX = 0.0f;
	tImageBox.fY = 0.0f;
	tImageBox.fW = (float)pImage->pRaster->tImage.iWidth;
	tImageBox.fH = (float)pImage->pRaster->tImage.iHeight;
	tLocalDst = __xgeSvgAspectViewport(tImageBox, pImage->tRect, pImage->iAspectAlignX, pImage->iAspectAlignY, pImage->iAspectMeetOrSlice);
	tDst = __xgeSvgMatrixRectBounds(tMatrix, tLocalDst);
	if ( (tDst.fW <= 0.0f) || (tDst.fH <= 0.0f) ) {
		return XGE_OK;
	}
	iColor = pImage->iColor;
	iAlpha = (int)(pImage->tStyle.fOpacity * (float)XGE_COLOR_GET_A(iColor) + 0.5f);
	if ( iAlpha <= 0 ) {
		return XGE_OK;
	}
	if ( iAlpha > 255 ) {
		iAlpha = 255;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = &pImage->pRaster->tTexture;
	tDraw.tDst = tDst;
	tDraw.iColor = XGE_COLOR_RGBA(XGE_COLOR_GET_R(iColor), XGE_COLOR_GET_G(iColor), XGE_COLOR_GET_B(iColor), iAlpha);
	tDraw.iFlags = bScreenSpace ? XGE_DRAW_SCREEN_SPACE : 0;
	xgeDrawEx(&tDraw);
	return XGE_OK;
}

static int __xgeSvgDrawClipBegin(xge_rect_t tClip, xge_rect_t* pOldClip, int* pOldClipEnabled, int* pClipApplied)
{
	int iRet;

	if ( (pOldClip == NULL) || (pOldClipEnabled == NULL) || (pClipApplied == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pOldClipEnabled = 0;
	*pClipApplied = 0;
	if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
		return XGE_OK;
	}
	iRet = __xgeShapeAutoBatchFlush();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	*pOldClip = xgeClipGet();
	*pOldClipEnabled = (pOldClip->fW > 0.0f) && (pOldClip->fH > 0.0f);
	if ( *pOldClipEnabled ) {
		tClip = __xgeSvgRectIntersect(*pOldClip, tClip);
	}
	xgeClipSet(tClip);
	*pClipApplied = 1;
	return XGE_OK;
}

static int __xgeSvgDrawClipEnd(xge_rect_t tOldClip, int bOldClipEnabled, int bClipApplied, int iRet)
{
	if ( bClipApplied ) {
		int iFlushRet = __xgeShapeAutoBatchFlush();

		if ( iRet == XGE_OK ) {
			iRet = iFlushRet;
		}
		if ( bOldClipEnabled ) {
			xgeClipSet(tOldClip);
		} else {
			xgeClipClear();
		}
	}
	return iRet;
}

static int __xgeSvgDrawMaskShapeBegin(xge_shape_ex pMaskShape, xge_shape_ex_matrix_t tParent, float fTolerance, int bScreenSpace, int* pApplied)
{
	return bScreenSpace ?
		xgeShapeExStencilClipBeginPx(pMaskShape, fTolerance, &tParent, pApplied) :
		xgeShapeExStencilClipBegin(pMaskShape, fTolerance, &tParent, pApplied);
}

static int __xgeSvgDrawMaskShapeEnd(int bApplied, int iRet)
{
	return xgeShapeExStencilClipEnd(bApplied, iRet);
}

static int __xgeSvgDrawInternal(xge_svg pSvg, xge_rect_t tDst, float fTolerance, int bScreenSpace)
{
	xge_shape_ex_matrix_t tMatrix;
	xge_rect_t tViewport;
	xge_rect_t tViewBox;
	int iRet;
	int i;

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
	for ( i = 0; i < pSvg->iItemCount; i++ ) {
		if ( pSvg->pItems[i].iType == XGE_SVG_DRAW_ITEM_SHAPE ) {
			iRet = bScreenSpace ? xgeShapeExDrawPxEx(pSvg->pItems[i].u.pShape, fTolerance, &tMatrix, 1.0f) : xgeShapeExDrawEx(pSvg->pItems[i].u.pShape, fTolerance, &tMatrix, 1.0f);
		} else if ( pSvg->pItems[i].iType == XGE_SVG_DRAW_ITEM_TEXT ) {
			xge_svg_text_item_t* pText = &pSvg->pItems[i].u.tText;
			xge_rect_t tOldClip;
			xge_shape_ex_matrix_t tItemMatrix;
			int bOldClipEnabled;
			int bClipApplied;
			int bMaskApplied;

			iRet = __xgeShapeAutoBatchFlush();
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			tItemMatrix = __xgeSvgMatrixMul(tMatrix, pText->tStyle.tTransform);
			if ( pText->bClipRect ) {
				xge_rect_t tClip = __xgeSvgMatrixRectBounds(tItemMatrix, pText->tClipRect);

				iRet = __xgeSvgDrawClipBegin(tClip, &tOldClip, &bOldClipEnabled, &bClipApplied);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
			} else {
				memset(&tOldClip, 0, sizeof(tOldClip));
				bOldClipEnabled = 0;
				bClipApplied = 0;
			}
			bMaskApplied = 0;
			if ( pText->pMaskShape != NULL ) {
				iRet = __xgeSvgDrawMaskShapeBegin(pText->pMaskShape, tItemMatrix, fTolerance, bScreenSpace, &bMaskApplied);
				if ( iRet != XGE_OK ) {
					return __xgeSvgDrawClipEnd(tOldClip, bOldClipEnabled, bClipApplied, iRet);
				}
			}
			iRet = __xgeSvgDrawTextItem(pSvg, pText, tMatrix, bScreenSpace);
			iRet = __xgeSvgDrawMaskShapeEnd(bMaskApplied, iRet);
			iRet = __xgeSvgDrawClipEnd(tOldClip, bOldClipEnabled, bClipApplied, iRet);
		} else if ( pSvg->pItems[i].iType == XGE_SVG_DRAW_ITEM_SVG_IMAGE ) {
			xge_svg_image_item_t* pImage = &pSvg->pItems[i].u.tImage;
			xge_rect_t tOldClip;
			xge_shape_ex_matrix_t tItemMatrix;
			int bOldClipEnabled;
			int bClipApplied;
			int bMaskApplied;

			iRet = __xgeShapeAutoBatchFlush();
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			tItemMatrix = __xgeSvgMatrixMul(tMatrix, pImage->tStyle.tTransform);
			if ( pImage->bClipRect ) {
				xge_rect_t tClip = __xgeSvgMatrixRectBounds(tItemMatrix, pImage->tClipRect);

				iRet = __xgeSvgDrawClipBegin(tClip, &tOldClip, &bOldClipEnabled, &bClipApplied);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
			} else {
				memset(&tOldClip, 0, sizeof(tOldClip));
				bOldClipEnabled = 0;
				bClipApplied = 0;
			}
			bMaskApplied = 0;
			if ( pImage->pMaskShape != NULL ) {
				iRet = __xgeSvgDrawMaskShapeBegin(pImage->pMaskShape, tItemMatrix, fTolerance, bScreenSpace, &bMaskApplied);
				if ( iRet != XGE_OK ) {
					return __xgeSvgDrawClipEnd(tOldClip, bOldClipEnabled, bClipApplied, iRet);
				}
			}
			iRet = __xgeSvgDrawImageItem(pImage, tMatrix, fTolerance, bScreenSpace);
			iRet = __xgeSvgDrawMaskShapeEnd(bMaskApplied, iRet);
			iRet = __xgeSvgDrawClipEnd(tOldClip, bOldClipEnabled, bClipApplied, iRet);
		} else if ( pSvg->pItems[i].iType == XGE_SVG_DRAW_ITEM_RASTER_IMAGE ) {
			xge_svg_raster_item_t* pImage = &pSvg->pItems[i].u.tRaster;
			xge_rect_t tOldClip;
			xge_shape_ex_matrix_t tItemMatrix;
			int bOldClipEnabled;
			int bClipApplied;
			int bMaskApplied;

			iRet = __xgeShapeAutoBatchFlush();
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			tItemMatrix = __xgeSvgMatrixMul(tMatrix, pImage->tStyle.tTransform);
			if ( pImage->bClipRect ) {
				xge_rect_t tClip = __xgeSvgMatrixRectBounds(tItemMatrix, pImage->tClipRect);

				iRet = __xgeSvgDrawClipBegin(tClip, &tOldClip, &bOldClipEnabled, &bClipApplied);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
			} else {
				memset(&tOldClip, 0, sizeof(tOldClip));
				bOldClipEnabled = 0;
				bClipApplied = 0;
			}
			bMaskApplied = 0;
			if ( pImage->pMaskShape != NULL ) {
				iRet = __xgeSvgDrawMaskShapeBegin(pImage->pMaskShape, tItemMatrix, fTolerance, bScreenSpace, &bMaskApplied);
				if ( iRet != XGE_OK ) {
					return __xgeSvgDrawClipEnd(tOldClip, bOldClipEnabled, bClipApplied, iRet);
				}
			}
			iRet = __xgeSvgDrawRasterImageItem(pImage, tMatrix, bScreenSpace);
			iRet = __xgeSvgDrawMaskShapeEnd(bMaskApplied, iRet);
			iRet = __xgeSvgDrawClipEnd(tOldClip, bOldClipEnabled, bClipApplied, iRet);
		} else {
			iRet = XGE_OK;
		}
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return __xgeShapeAutoBatchFlush();
}

int xgeSvgDraw(xge_svg pSvg, xge_rect_t tDst, float fTolerance)
{
	return __xgeSvgDrawInternal(pSvg, tDst, fTolerance, 0);
}

int xgeSvgDrawPx(xge_svg pSvg, xge_rect_t tDst, float fTolerance)
{
	return __xgeSvgDrawInternal(pSvg, tDst, fTolerance, 1);
}

int xgeSvgRasterize(const char* sURI, int iWidth, int iHeight, void* pPixels, int iStride)
{
	xge_resource_t tResource;
	char* sBaseDir;
	int iRet;

	if ( (sURI == NULL) || (iWidth <= 0) || (iHeight <= 0) || (pPixels == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sBaseDir = __xgeSvgUriBaseDir(sURI);
	if ( sBaseDir == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeResourceLoad(sURI, &tResource);
	if ( iRet != XGE_OK ) {
		xrtFree(sBaseDir);
		return iRet;
	}
	iRet = __xgeSvgRasterizeMemoryEx(tResource.pData, tResource.iSize, iWidth, iHeight, pPixels, iStride, sBaseDir);
	xgeResourceFree(&tResource);
	xrtFree(sBaseDir);
	return iRet;
}

static int __xgeSvgRasterizeMemoryEx(const void* pData, int iSize, int iWidth, int iHeight, void* pPixels, int iStride, const char* sBaseDir)
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
		iRet = __xgeSvgLoadMemoryEx(pSvg, pData, iSize, sBaseDir);
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

int xgeSvgRasterizeMemory(const void* pData, int iSize, int iWidth, int iHeight, void* pPixels, int iStride)
{
	return __xgeSvgRasterizeMemoryEx(pData, iSize, iWidth, iHeight, pPixels, iStride, NULL);
}

int xgeSvgTextureLoad(xge_texture pTexture, const char* sURI, int iWidth, int iHeight)
{
	xge_resource_t tResource;
	char* sBaseDir;
	int iRet;

	if ( (pTexture == NULL) || (sURI == NULL) || (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sBaseDir = __xgeSvgUriBaseDir(sURI);
	if ( sBaseDir == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeResourceLoad(sURI, &tResource);
	if ( iRet != XGE_OK ) {
		xrtFree(sBaseDir);
		return iRet;
	}
	iRet = __xgeSvgTextureLoadMemoryEx(pTexture, tResource.pData, tResource.iSize, iWidth, iHeight, sBaseDir);
	xgeResourceFree(&tResource);
	xrtFree(sBaseDir);
	return iRet;
}

static int __xgeSvgTextureLoadMemoryEx(xge_texture pTexture, const void* pData, int iSize, int iWidth, int iHeight, const char* sBaseDir)
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
	iRet = __xgeSvgRasterizeMemoryEx(pData, iSize, iWidth, iHeight, pPixels, iStride, sBaseDir);
	if ( iRet == XGE_OK ) {
		iRet = xgeTextureCreateRGBA(pTexture, iWidth, iHeight, pPixels);
	}
	xrtFree(pPixels);
	return iRet;
}

int xgeSvgTextureLoadMemory(xge_texture pTexture, const void* pData, int iSize, int iWidth, int iHeight)
{
	return __xgeSvgTextureLoadMemoryEx(pTexture, pData, iSize, iWidth, iHeight, NULL);
}
