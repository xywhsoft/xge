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
#define XGE_SVG_COLOR_SEPARATOR_NONE 0
#define XGE_SVG_COLOR_SEPARATOR_COMMA 1
#define XGE_SVG_COLOR_SEPARATOR_SPACE 2
#define XGE_SVG_GRADIENT_HAS_X1			0x00000001
#define XGE_SVG_GRADIENT_HAS_Y1			0x00000002
#define XGE_SVG_GRADIENT_HAS_X2			0x00000004
#define XGE_SVG_GRADIENT_HAS_Y2			0x00000008
#define XGE_SVG_GRADIENT_HAS_CX			0x00000010
#define XGE_SVG_GRADIENT_HAS_CY			0x00000020
#define XGE_SVG_GRADIENT_HAS_R			0x00000040
#define XGE_SVG_GRADIENT_HAS_FX			0x00000080
#define XGE_SVG_GRADIENT_HAS_FY			0x00000100
#define XGE_SVG_GRADIENT_HAS_FR			0x00004000
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
#define XGE_SVG_PATTERN_HAS_OVERFLOW		0x00000400
#define XGE_SVG_DATA_IMAGE_UTF8			1
#define XGE_SVG_DATA_IMAGE_BASE64		2
#define XGE_SVG_FILTER_PRIMITIVE_MAX 16
#define XGE_SVG_FILTER_PRIMITIVE_OFFSET 1
#define XGE_SVG_FILTER_PRIMITIVE_DROP_SHADOW 2
#define XGE_SVG_FILTER_PRIMITIVE_FLOOD 3
#define XGE_SVG_FILTER_PRIMITIVE_GAUSSIAN_BLUR 4
#define XGE_SVG_FILTER_PRIMITIVE_COLOR_MATRIX 5
#define XGE_SVG_FILTER_PRIMITIVE_COMPONENT_TRANSFER 6
#define XGE_SVG_FILTER_PRIMITIVE_COMPOSITE 7
#define XGE_SVG_FILTER_PRIMITIVE_MERGE 8
#define XGE_SVG_FILTER_PRIMITIVE_BLEND 9
#define XGE_SVG_FILTER_PRIMITIVE_MORPHOLOGY 10
#define XGE_SVG_FILTER_PRIMITIVE_TILE 11
#define XGE_SVG_FILTER_PRIMITIVE_IMAGE 12
#define XGE_SVG_FILTER_MERGE_NODE_MAX 16
#define XGE_SVG_COMPONENT_TRANSFER_TABLE_MAX 64
#define XGE_SVG_COMPONENT_TRANSFER_IDENTITY 0
#define XGE_SVG_COMPONENT_TRANSFER_TABLE 1
#define XGE_SVG_COMPONENT_TRANSFER_DISCRETE 2
#define XGE_SVG_COMPONENT_TRANSFER_LINEAR 3
#define XGE_SVG_COMPONENT_TRANSFER_GAMMA 4
#define XGE_SVG_FILTER_COMPOSITE_OVER 0
#define XGE_SVG_FILTER_COMPOSITE_IN 1
#define XGE_SVG_FILTER_COMPOSITE_OUT 2
#define XGE_SVG_FILTER_COMPOSITE_ATOP 3
#define XGE_SVG_FILTER_COMPOSITE_XOR 4
#define XGE_SVG_FILTER_COMPOSITE_LIGHTER 5
#define XGE_SVG_FILTER_COMPOSITE_ARITHMETIC 6
#define XGE_SVG_FILTER_BLEND_NORMAL 0
#define XGE_SVG_FILTER_BLEND_MULTIPLY 1
#define XGE_SVG_FILTER_BLEND_SCREEN 2
#define XGE_SVG_FILTER_BLEND_DARKEN 3
#define XGE_SVG_FILTER_BLEND_LIGHTEN 4
#define XGE_SVG_FILTER_BLEND_OVERLAY 5
#define XGE_SVG_FILTER_BLEND_COLOR_DODGE 6
#define XGE_SVG_FILTER_BLEND_COLOR_BURN 7
#define XGE_SVG_FILTER_BLEND_HARD_LIGHT 8
#define XGE_SVG_FILTER_BLEND_SOFT_LIGHT 9
#define XGE_SVG_FILTER_BLEND_DIFFERENCE 10
#define XGE_SVG_FILTER_BLEND_EXCLUSION 11
#define XGE_SVG_FILTER_BLEND_HUE 12
#define XGE_SVG_FILTER_BLEND_SATURATION 13
#define XGE_SVG_FILTER_BLEND_COLOR 14
#define XGE_SVG_FILTER_BLEND_LUMINOSITY 15
#define XGE_SVG_FILTER_MORPHOLOGY_ERODE 0
#define XGE_SVG_FILTER_MORPHOLOGY_DILATE 1
#define XGE_SVG_FILTER_PRIMITIVE_REGION_X 0x01
#define XGE_SVG_FILTER_PRIMITIVE_REGION_Y 0x02
#define XGE_SVG_FILTER_PRIMITIVE_REGION_W 0x04
#define XGE_SVG_FILTER_PRIMITIVE_REGION_H 0x08
#define XGE_SVG_BOUNDS_MAX_DEPTH 32
#define XGE_SVG_DRAW_ITEM_SHAPE 1
#define XGE_SVG_DRAW_ITEM_TEXT 2
#define XGE_SVG_DRAW_ITEM_SVG_IMAGE 3
#define XGE_SVG_DRAW_ITEM_RASTER_IMAGE 4
#define XGE_SVG_DRAW_ITEM_SCENE 5
#define XGE_SVG_FLOAT_ABS_MAX 3.40282346638528860e+38

typedef int (*xge_shape_ex_svg_path_event_proc)(void* pUser, uint8_t iCommand, xge_vec2_t tStart, xge_vec2_t tControl1, xge_vec2_t tControl2, xge_vec2_t tEnd);

int xgeShapeExInternalAppendSvgPathWithEvents(xge_shape_ex pShape, const char* sPath, xge_shape_ex_svg_path_event_proc pEvent, void* pUser);

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
	float fBaselineShift;
	float fLetterSpacing;
	float fWordSpacing;
	float fPathLength;
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
	int bFillRuleSet;
	int bClipRuleSet;
	int iLineCap;
	int iLineJoin;
	int iTextAnchor;
	int iTextSpace;
	int iTextBaseline;
	int iTextDecoration;
	int iBlend;
	int iPaintOrder[XGE_SVG_PAINT_COMPONENT_COUNT];
	int bStrokeFirst;
	int bVisible;
	int bVisibility;
	int bBlendSet;
	int bStopColorSet;
	int bStopOpacitySet;
	int bColorSet;
	int bFillSet;
	int bFillCurrentColor;
	int bStrokeSet;
	int bStrokeCurrentColor;
	int bStrokeNonScaling;
	int bViewportClip;
	xge_rect_t tViewportClip;
	xge_shape_ex_matrix_t tTransform;
} xge_svg_style_t;

typedef struct xge_svg_style_length_valid_t {
	float fPercentRef;
	float fFontSize;
	float fMinValue;
	int bUseMinValue;
} xge_svg_style_length_valid_t;

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
	int iBlend;
	int bBlendSet;
	union {
		xge_shape_ex pShape;
		xge_shape_ex_scene pScene;
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
	int bOverflowVisible;
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

typedef struct xge_svg_component_transfer_func_t {
	int iType;
	float fSlope;
	float fIntercept;
	float fAmplitude;
	float fExponent;
	float fOffset;
	float arrTableValues[XGE_SVG_COMPONENT_TRANSFER_TABLE_MAX];
	int iTableValueCount;
} xge_svg_component_transfer_func_t;

typedef struct xge_svg_filter_primitive_t {
	int iType;
	xge_rect_t tRegion;
	float fOffsetX;
	float fOffsetY;
	float fGaussianBlurX;
	float fGaussianBlurY;
	float fMorphologyRadiusX;
	float fMorphologyRadiusY;
	float fDropShadowX;
	float fDropShadowY;
	float fDropShadowStdDeviation;
	float fDropShadowOpacity;
	float arrColorMatrix[20];
	xge_svg_component_transfer_func_t arrComponentTransfer[4];
	uint32_t iDropShadowColor;
	uint32_t iFloodColor;
	float fFloodOpacity;
	int iBlendMode;
	int iMorphologyOperator;
	int iCompositeOperator;
	float fCompositeK1;
	float fCompositeK2;
	float fCompositeK3;
	float fCompositeK4;
	int bDropShadowCurrentColor;
	int bFloodCurrentColor;
	char sIn[XGE_SVG_ID_MAX];
	char sIn2[XGE_SVG_ID_MAX];
	char sResult[XGE_SVG_ID_MAX];
	char sOperator[XGE_SVG_ID_MAX];
	char* sHref;
	char* sAspect;
	char arrMergeInputs[XGE_SVG_FILTER_MERGE_NODE_MAX][XGE_SVG_ID_MAX];
	int iMergeNodeCount;
	int iRegionFlags;
} xge_svg_filter_primitive_t;

typedef struct xge_svg_filter_t {
	char* sId;
	xge_rect_t tRegion;
	int iFilterUnits;
	int iUnits;
	int bHasOffset;
	int bHasDropShadow;
	int bHasGaussianBlur;
	int bHasFlood;
	int bHasColorMatrix;
	int bHasComponentTransfer;
	int bHasCompositeIn;
	int bHasMerge;
	int bHasBlend;
	int bMergeIncludesSourceGraphic;
	int bMergeIncludesFilterOutput;
	int bHasUnsupportedPrimitive;
	int iPrimitiveCount;
	int arrPrimitiveTypes[XGE_SVG_FILTER_PRIMITIVE_MAX];
	xge_svg_filter_primitive_t arrPrimitives[XGE_SVG_FILTER_PRIMITIVE_MAX];
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
	float fFR;
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
static xge_rect_t __xgeSvgRectIntersect(xge_rect_t a, xge_rect_t b);
static xge_rect_t __xgeSvgRectUnion(xge_rect_t a, xge_rect_t b);

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
	} else if ( pItem->iType == XGE_SVG_DRAW_ITEM_SCENE ) {
		xgeShapeExSceneDestroy(pItem->u.pScene);
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
		if ( (pPattern->iFlags & XGE_SVG_PATTERN_HAS_OVERFLOW) == 0 ) {
			pDef->bOverflowVisible = pSourceDef->bOverflowVisible;
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
	pSvg->pRadialGradients[iIndex].fFR = 0.0f;
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
	if ( (pGradient->iFlags & XGE_SVG_GRADIENT_HAS_FR) == 0 ) pGradient->fFR = pSource->fFR;
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
	int iBracketDepth;
	int bUseBracketDepth;

	if ( pText == NULL ) {
		return NULL;
	}
	if ( strncmp(pText, "<!--", 4) == 0 ) {
		const char* pEnd = strstr(pText + 4, "-->");
		return pEnd != NULL ? pEnd + 2 : NULL;
	}
	if ( strncmp(pText, "<![CDATA[", 9) == 0 ) {
		const char* pEnd = strstr(pText + 9, "]]>");
		return pEnd != NULL ? pEnd + 2 : NULL;
	}
	if ( strncmp(pText, "<?", 2) == 0 ) {
		const char* pEnd = strstr(pText + 2, "?>");
		return pEnd != NULL ? pEnd + 1 : NULL;
	}
	cQuote = 0;
	iBracketDepth = 0;
	bUseBracketDepth = (pText[0] == '<') && (pText[1] == '!');
	while ( *pText != '\0' ) {
		if ( cQuote != 0 ) {
			if ( *pText == cQuote ) {
				cQuote = 0;
			}
		} else if ( (*pText == '"') || (*pText == '\'') ) {
			cQuote = *pText;
		} else if ( bUseBracketDepth && (pText[0] == '[') ) {
			iBracketDepth++;
		} else if ( bUseBracketDepth && (pText[0] == ']') && (iBracketDepth > 0) ) {
			iBracketDepth--;
		} else if ( (*pText == '>') && (iBracketDepth <= 0) ) {
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

static int __xgeSvgTagNameIsGroupContainer(const char* pTag)
{
	return __xgeSvgTagNameEquals(pTag, "g") || __xgeSvgTagNameEquals(pTag, "a");
}

static int __xgeSvgIsCloseGroupContainer(const char* pTag)
{
	return __xgeSvgIsCloseTagName(pTag, "g") || __xgeSvgIsCloseTagName(pTag, "a");
}

static int __xgeSvgTagNameIsClipMaskContainer(const char* pTag)
{
	return __xgeSvgTagNameIsGroupContainer(pTag) || __xgeSvgTagNameEquals(pTag, "svg");
}

static int __xgeSvgIsCloseClipMaskContainer(const char* pTag)
{
	return __xgeSvgIsCloseGroupContainer(pTag) || __xgeSvgIsCloseTagName(pTag, "svg");
}

static int __xgeSvgTagNameIsNonRenderingContent(const char* pTag)
{
	return __xgeSvgTagNameEquals(pTag, "script") ||
	       __xgeSvgTagNameEquals(pTag, "title") ||
	       __xgeSvgTagNameEquals(pTag, "desc") ||
	       __xgeSvgTagNameEquals(pTag, "metadata");
}

static int __xgeSvgTagNameIsRawContentSkip(const char* pTag)
{
	return __xgeSvgTagNameEquals(pTag, "style") ||
	       __xgeSvgTagNameIsNonRenderingContent(pTag);
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

static int __xgeSvgElementRangeEnd(const char* pTag, const char* pTagEnd, const char* pLimit, const char** ppElementEnd);
static int __xgeSvgSwitchSelectRange(const char* pSwitchTag, const char* pSwitchTagEnd, const char** ppSwitchEnd, const char** ppSelectedStart, const char** ppSelectedEnd);
static int __xgeSvgConditionalElementMatches(const char* pTag, const char* pTagEnd);

static int __xgeSvgElementNodesPreScan(xge_svg pSvg, const char* sText)
{
	const char* p;
	const char* pScan;
	const char* pTextEnd;
	const char* arrSwitchEndStack[XGE_SVG_STACK_MAX];
	const char* arrSwitchSelectedStartStack[XGE_SVG_STACK_MAX];
	const char* arrSwitchSelectedEndStack[XGE_SVG_STACK_MAX];
	int iSwitchTop;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (sText == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeSvgElementNodesClear(pSvg);
	pSvg->iElementStackTop = 0;
	pTextEnd = sText + strlen(sText);
	iSwitchTop = 0;
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
		bSelfClosing = __xgeSvgIsSelfClosingTag(p, pTagEnd);
		while ( (iSwitchTop > 0) && (p >= arrSwitchEndStack[iSwitchTop - 1]) ) {
			iSwitchTop--;
		}
		if ( iSwitchTop > 0 ) {
			const char* pSelectedStart = arrSwitchSelectedStartStack[iSwitchTop - 1];
			const char* pSelectedEnd = arrSwitchSelectedEndStack[iSwitchTop - 1];

			if ( (pSelectedStart == NULL) || (pSelectedEnd == NULL) || (p < pSelectedStart) || (p >= pSelectedEnd) ) {
				const char* pElementEnd;

				if ( __xgeSvgElementRangeEnd(p, pTagEnd, arrSwitchEndStack[iSwitchTop - 1], &pElementEnd) ) {
					pScan = pElementEnd;
				} else {
					pScan = pTagEnd + 1;
				}
				continue;
			}
		}
		if ( __xgeSvgIsCloseTagName(p, "switch") ) {
			__xgeSvgElementStackPop(pSvg, "switch");
			pScan = pTagEnd + 1;
			continue;
		}
		if ( (p[1] != '/') && !__xgeSvgConditionalElementMatches(p, pTagEnd) ) {
			const char* pElementEnd;

			if ( __xgeSvgElementRangeEnd(p, pTagEnd, pTextEnd, &pElementEnd) ) {
				pScan = pElementEnd;
			} else {
				pScan = pTagEnd + 1;
			}
			continue;
		}
		if ( (p[1] != '/') && __xgeSvgTagNameIsRawContentSkip(p) ) {
			const char* pElementEnd;

			if ( __xgeSvgElementRangeEnd(p, pTagEnd, pTextEnd, &pElementEnd) ) {
				pScan = pElementEnd;
			} else {
				pScan = pTagEnd + 1;
			}
			continue;
		}
		if ( __xgeSvgTagNameEquals(p, "switch") ) {
			int bHasSwitchRange;

			bHasSwitchRange = 0;
			if ( !bSelfClosing ) {
				const char* pSwitchEnd;
				const char* pSelectedStart;
				const char* pSelectedEnd;

				if ( iSwitchTop >= XGE_SVG_STACK_MAX ) {
					pSvg->iElementStackTop = 0;
					pSvg->iCurrentElementNode = -1;
					memset(pSvg->arrLastChildByDepth, 0xff, sizeof(pSvg->arrLastChildByDepth));
					return XGE_ERROR_OUT_OF_MEMORY;
				}
				if ( __xgeSvgSwitchSelectRange(p, pTagEnd, &pSwitchEnd, &pSelectedStart, &pSelectedEnd) ) {
					arrSwitchEndStack[iSwitchTop] = pSwitchEnd;
					arrSwitchSelectedStartStack[iSwitchTop] = pSelectedStart;
					arrSwitchSelectedEndStack[iSwitchTop] = pSelectedEnd;
					iSwitchTop++;
					bHasSwitchRange = 1;
				}
			}
			iNodeIndex = -1;
			iRet = __xgeSvgElementNodeEnsure(pSvg, p, pTagEnd, &iNodeIndex);
			if ( iRet != XGE_OK ) {
				pSvg->iElementStackTop = 0;
				pSvg->iCurrentElementNode = -1;
				memset(pSvg->arrLastChildByDepth, 0xff, sizeof(pSvg->arrLastChildByDepth));
				return iRet;
			}
			if ( bHasSwitchRange ) {
				iRet = __xgeSvgElementStackPush(pSvg, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					pSvg->iElementStackTop = 0;
					pSvg->iCurrentElementNode = -1;
					memset(pSvg->arrLastChildByDepth, 0xff, sizeof(pSvg->arrLastChildByDepth));
					return iRet;
				}
			}
			pScan = pTagEnd + 1;
			continue;
		}
		if ( p[1] == '/' ) {
			__xgeSvgElementStackPop(pSvg, NULL);
			pScan = pTagEnd + 1;
			continue;
		}
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

static int __xgeSvgCssHexValue(char c)
{
	if ( (c >= '0') && (c <= '9') ) {
		return c - '0';
	}
	if ( (c >= 'a') && (c <= 'f') ) {
		return c - 'a' + 10;
	}
	if ( (c >= 'A') && (c <= 'F') ) {
		return c - 'A' + 10;
	}
	return -1;
}

static void __xgeSvgCssOutByte(char* sOut, int iOutCapacity, int* pWrite, char c)
{
	if ( (sOut == NULL) || (pWrite == NULL) || (iOutCapacity <= 0) ) {
		return;
	}
	if ( *pWrite < iOutCapacity - 1 ) {
		sOut[*pWrite] = c;
		(*pWrite)++;
	}
}

static void __xgeSvgCssOutUtf8(char* sOut, int iOutCapacity, int* pWrite, uint32_t iCodepoint)
{
	if ( iCodepoint <= 0x7Fu ) {
		__xgeSvgCssOutByte(sOut, iOutCapacity, pWrite, (char)iCodepoint);
	} else if ( iCodepoint <= 0x7FFu ) {
		__xgeSvgCssOutByte(sOut, iOutCapacity, pWrite, (char)(0xC0u | (iCodepoint >> 6)));
		__xgeSvgCssOutByte(sOut, iOutCapacity, pWrite, (char)(0x80u | (iCodepoint & 0x3Fu)));
	} else if ( iCodepoint <= 0xFFFFu ) {
		if ( (iCodepoint >= 0xD800u) && (iCodepoint <= 0xDFFFu) ) {
			return;
		}
		__xgeSvgCssOutByte(sOut, iOutCapacity, pWrite, (char)(0xE0u | (iCodepoint >> 12)));
		__xgeSvgCssOutByte(sOut, iOutCapacity, pWrite, (char)(0x80u | ((iCodepoint >> 6) & 0x3Fu)));
		__xgeSvgCssOutByte(sOut, iOutCapacity, pWrite, (char)(0x80u | (iCodepoint & 0x3Fu)));
	} else if ( iCodepoint <= 0x10FFFFu ) {
		__xgeSvgCssOutByte(sOut, iOutCapacity, pWrite, (char)(0xF0u | (iCodepoint >> 18)));
		__xgeSvgCssOutByte(sOut, iOutCapacity, pWrite, (char)(0x80u | ((iCodepoint >> 12) & 0x3Fu)));
		__xgeSvgCssOutByte(sOut, iOutCapacity, pWrite, (char)(0x80u | ((iCodepoint >> 6) & 0x3Fu)));
		__xgeSvgCssOutByte(sOut, iOutCapacity, pWrite, (char)(0x80u | (iCodepoint & 0x3Fu)));
	}
}

static int __xgeSvgCssEscapeNext(const char** ppText, const char* pEnd, uint32_t* pCodepoint)
{
	const char* p;
	int iHexCount;
	uint32_t iValue;

	if ( (ppText == NULL) || (*ppText == NULL) || (pEnd == NULL) || (pCodepoint == NULL) ||
	     (*ppText >= pEnd) || (**ppText != '\\') ) {
		return 0;
	}
	p = *ppText + 1;
	if ( p >= pEnd ) {
		return 0;
	}
	if ( (*p == '\n') || (*p == '\r') || (*p == '\f') ) {
		return 0;
	}
	iValue = 0;
	iHexCount = 0;
	while ( (p < pEnd) && (iHexCount < 6) ) {
		int iHex = __xgeSvgCssHexValue(*p);
		if ( iHex < 0 ) {
			break;
		}
		iValue = (iValue << 4) | (uint32_t)iHex;
		iHexCount++;
		p++;
	}
	if ( iHexCount > 0 ) {
		if ( (p < pEnd) && __xgeSvgCssIsSpace(*p) ) {
			p++;
		}
		if ( (iValue == 0) || (iValue > 0x10FFFFu) ||
		     ((iValue >= 0xD800u) && (iValue <= 0xDFFFu)) ) {
			iValue = 0xFFFDu;
		}
		*pCodepoint = iValue;
		*ppText = p;
		return 1;
	}
	*pCodepoint = (unsigned char)*p;
	*ppText = p + 1;
	return 1;
}

static int __xgeSvgCssIsSelectorTokenDelimiter(char c)
{
	return (c == '.') || (c == '#') || (c == '[') || (c == ':') ||
	       (c == '>') || (c == '+') || (c == '~') || (c == ']') ||
	       (c == '*') || __xgeSvgCssIsSpace(c);
}

static int __xgeSvgCssIdentifierTokenEnd(const char* pStart, const char* pEnd, const char** ppTokenEnd)
{
	const char* p;
	int bAny;

	if ( (pStart == NULL) || (pEnd == NULL) || (ppTokenEnd == NULL) || (pStart >= pEnd) ) {
		return 0;
	}
	p = pStart;
	bAny = 0;
	while ( p < pEnd ) {
		if ( *p == '\\' ) {
			uint32_t iCodepoint;
			const char* pNext = p;

			if ( !__xgeSvgCssEscapeNext(&pNext, pEnd, &iCodepoint) ) {
				return 0;
			}
			p = pNext;
			bAny = 1;
			continue;
		}
		if ( __xgeSvgCssIsSelectorTokenDelimiter(*p) ) {
			break;
		}
		p++;
		bAny = 1;
	}
	if ( !bAny ) {
		return 0;
	}
	*ppTokenEnd = p;
	return 1;
}

static int __xgeSvgCssIdentifierTokenCopyDecoded(const char* pStart, const char* pEnd, char* sOut, int iOutCapacity)
{
	int iWrite;

	if ( (pStart == NULL) || (pEnd == NULL) || (sOut == NULL) || (iOutCapacity <= 0) || (pEnd <= pStart) ) {
		return 0;
	}
	iWrite = 0;
	while ( pStart < pEnd ) {
		if ( *pStart == '\\' ) {
			uint32_t iCodepoint;
			if ( !__xgeSvgCssEscapeNext(&pStart, pEnd, &iCodepoint) ) {
				return 0;
			}
			__xgeSvgCssOutUtf8(sOut, iOutCapacity, &iWrite, iCodepoint);
			continue;
		}
		__xgeSvgCssOutByte(sOut, iOutCapacity, &iWrite, *pStart);
		pStart++;
	}
	sOut[iWrite < iOutCapacity ? iWrite : (iOutCapacity - 1)] = '\0';
	return iWrite < iOutCapacity;
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

static int __xgeSvgCssAttributeSelectorParse(const char* pStart, const char* pEnd, char* sName, int iNameCapacity, char* sOp, int iOpCapacity, char* sValue, int iValueCapacity, int* pCaseInsensitive)
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
	if ( pCaseInsensitive != NULL ) {
		*pCaseInsensitive = 0;
	}
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
	if ( !__xgeSvgCssIdentifierTokenCopyDecoded(pNameStart, pNameEnd, sName, iNameCapacity) ) {
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
		while ( p < pClose ) {
			if ( *p == '\\' ) {
				const char* pNext = p;
				uint32_t iCodepoint;

				if ( !__xgeSvgCssEscapeNext(&pNext, pClose, &iCodepoint) ) {
					return 0;
				}
				p = pNext;
				continue;
			}
			if ( *p == cQuote ) {
				break;
			}
			p++;
		}
		if ( p >= pClose ) {
			return 0;
		}
		pValueEnd = p++;
	} else {
		while ( (p < pClose) && !__xgeSvgCssIsSpace(*p) ) {
			if ( *p == '\\' ) {
				const char* pNext = p;
				uint32_t iCodepoint;

				if ( !__xgeSvgCssEscapeNext(&pNext, pClose, &iCodepoint) ) {
					return 0;
				}
				p = pNext;
				continue;
			}
			if ( (*p == '"') || (*p == '\'') ) {
				return 0;
			}
			p++;
		}
		pValueEnd = p;
	}
	if ( !__xgeSvgCssIdentifierTokenCopyDecoded(pValueStart, pValueEnd, sValue, iValueCapacity) ) {
		return 0;
	}
	while ( (p < pClose) && __xgeSvgCssIsSpace(*p) ) {
		p++;
	}
	if ( p < pClose ) {
		char cFlag;

		cFlag = (char)__xgeSvgCssAsciiLower((unsigned char)*p);
		if ( (cFlag != 'i') && (cFlag != 's') ) {
			return 0;
		}
		if ( pCaseInsensitive != NULL ) {
			*pCaseInsensitive = (cFlag == 'i');
		}
		p++;
		while ( (p < pClose) && __xgeSvgCssIsSpace(*p) ) {
			p++;
		}
	}
	return p == pClose;
}

static void __xgeSvgCssSkipSpacesRange(const char** ppText, const char* pEnd)
{
	const char* p;

	if ( (ppText == NULL) || (*ppText == NULL) || (pEnd == NULL) ) {
		return;
	}
	p = *ppText;
	while ( (p < pEnd) && __xgeSvgCssIsSpace(*p) ) {
		p++;
	}
	*ppText = p;
}

static int __xgeSvgCssNthReadUnsigned(const char** ppText, const char* pEnd, int* pValue)
{
	const char* p;
	int iValue;
	int bAny;

	if ( (ppText == NULL) || (*ppText == NULL) || (pEnd == NULL) || (pValue == NULL) ) {
		return 0;
	}
	p = *ppText;
	iValue = 0;
	bAny = 0;
	while ( (p < pEnd) && (*p >= '0') && (*p <= '9') ) {
		int iDigit = *p - '0';

		if ( iValue > ((INT32_MAX - iDigit) / 10) ) {
			return 0;
		}
		iValue = iValue * 10 + iDigit;
		bAny = 1;
		p++;
	}
	if ( !bAny ) {
		return 0;
	}
	*ppText = p;
	*pValue = iValue;
	return 1;
}

static int __xgeSvgCssNthParse(const char* pStart, const char* pEnd, int* pA, int* pB)
{
	const char* p;
	int iSign;
	int iDigits;
	int bHasDigits;

	if ( (pStart == NULL) || (pEnd == NULL) || (pA == NULL) || (pB == NULL) || (pEnd < pStart) ) {
		return 0;
	}
	__xgeSvgTrimRange(&pStart, &pEnd);
	if ( pEnd <= pStart ) {
		return 0;
	}
	if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "odd") ) {
		*pA = 2;
		*pB = 1;
		return 1;
	}
	if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "even") ) {
		*pA = 2;
		*pB = 0;
		return 1;
	}
	p = pStart;
	iSign = 1;
	if ( (p < pEnd) && ((*p == '+') || (*p == '-')) ) {
		if ( *p == '-' ) {
			iSign = -1;
		}
		p++;
	}
	iDigits = 0;
	bHasDigits = __xgeSvgCssNthReadUnsigned(&p, pEnd, &iDigits);
	if ( (p < pEnd) && (__xgeSvgCssAsciiLower((unsigned char)*p) == 'n') ) {
		p++;
		if ( bHasDigits ) {
			*pA = iSign * iDigits;
		} else {
			*pA = iSign;
		}
		__xgeSvgCssSkipSpacesRange(&p, pEnd);
		if ( p >= pEnd ) {
			*pB = 0;
			return 1;
		}
		if ( (*p != '+') && (*p != '-') ) {
			return 0;
		}
		iSign = (*p == '-') ? -1 : 1;
		p++;
		__xgeSvgCssSkipSpacesRange(&p, pEnd);
		if ( !__xgeSvgCssNthReadUnsigned(&p, pEnd, &iDigits) ) {
			return 0;
		}
		__xgeSvgCssSkipSpacesRange(&p, pEnd);
		if ( p != pEnd ) {
			return 0;
		}
		*pB = iSign * iDigits;
		return 1;
	}
	if ( !bHasDigits ) {
		return 0;
	}
	__xgeSvgCssSkipSpacesRange(&p, pEnd);
	if ( p != pEnd ) {
		return 0;
	}
	*pA = 0;
	*pB = iSign * iDigits;
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
		const char* pNameEnd;

		if ( !__xgeSvgCssIdentifierTokenEnd(pNameStart, pEnd, &pNameEnd) ) {
			return 0;
		}
		p = pNameEnd;
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
			int bCaseInsensitive;

			if ( !__xgeSvgCssAttributeSelectorEnd(p, pEnd, &pAttrEnd) ||
			     !__xgeSvgCssAttributeSelectorParse(p, pAttrEnd, sName, sizeof(sName), sOp, sizeof(sOp), sValue, sizeof(sValue), &bCaseInsensitive) ) {
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
			if ( !__xgeSvgCssIdentifierTokenEnd(pNameStart, pEnd, &pNameEnd) ) {
				return 0;
			}
			p = pNameEnd;
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
		if ( (p < pEnd) && (cQuote == 0) && !bInAttr && (*p == '\\') ) {
			const char* pNext = p;
			uint32_t iCodepoint;

			if ( !__xgeSvgCssEscapeNext(&pNext, pEnd, &iCodepoint) ) {
				return 0;
			}
			p = pNext;
			continue;
		}
		if ( cQuote != 0 ) {
			if ( *p == '\\' ) {
				const char* pNext = p;
				uint32_t iCodepoint;

				if ( __xgeSvgCssEscapeNext(&pNext, pEnd, &iCodepoint) ) {
					p = pNext;
					continue;
				}
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
						const char* pNext = p;
						uint32_t iCodepoint;

						if ( __xgeSvgCssEscapeNext(&pNext, pEnd, &iCodepoint) ) {
							p = pNext;
							continue;
						}
					}
					if ( *p == cQuote ) {
						cQuote = 0;
					}
					p++;
					continue;
				}
				if ( *p == '\\' ) {
					const char* pNext = p;
					uint32_t iCodepoint;

					if ( !__xgeSvgCssEscapeNext(&pNext, pEnd, &iCodepoint) ) {
						return 0;
					}
					p = pNext;
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
		if ( (p < pEnd) && (cQuote == 0) && !bInAttr && (*p == '\\') ) {
			const char* pNext = p;
			uint32_t iCodepoint;

			if ( !__xgeSvgCssEscapeNext(&pNext, pEnd, &iCodepoint) ) {
				return 0;
			}
			p = pNext;
			continue;
		}
		if ( cQuote != 0 ) {
			if ( *p == '\\' ) {
				const char* pNext = p;
				uint32_t iCodepoint;

				if ( __xgeSvgCssEscapeNext(&pNext, pEnd, &iCodepoint) ) {
					p = pNext;
					continue;
				}
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

static int __xgeSvgParseStyleText(xge_svg pSvg, const char* pStart, const char* pEnd);
static int __xgeSvgElementRangeEnd(const char* pTag, const char* pTagEnd, const char* pLimit, const char** ppElementEnd);
static int __xgeSvgSwitchSelectRange(const char* pSwitchTag, const char* pSwitchTagEnd, const char** ppSwitchEnd, const char** ppSelectedStart, const char** ppSelectedEnd);
static int __xgeSvgConditionalElementMatches(const char* pTag, const char* pTagEnd);

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

static int __xgeSvgCssAtRuleBlock(const char* pStart, const char* pEnd, const char** ppPreludeStart, const char** ppPreludeEnd, const char** ppBlockStart, const char** ppBlockEnd, const char** ppAfter)
{
	const char* p;
	const char* pBlockStart;
	int iDepth;
	char cQuote;

	if ( (pStart == NULL) || (pEnd == NULL) || (ppPreludeStart == NULL) || (ppPreludeEnd == NULL) ||
	     (ppBlockStart == NULL) || (ppBlockEnd == NULL) || (ppAfter == NULL) || (pStart >= pEnd) || (*pStart != '@') ) {
		return 0;
	}
	p = pStart + 1;
	while ( (p < pEnd) && __xgeSvgCssIdentChar(*p) ) {
		p++;
	}
	if ( p >= pEnd ) {
		return 0;
	}
	*ppPreludeStart = p;
	iDepth = 0;
	cQuote = 0;
	pBlockStart = NULL;
	while ( p < pEnd ) {
		if ( cQuote != 0 ) {
			if ( *p == '\\' ) {
				const char* pNext = p;
				uint32_t iCodepoint;

				if ( __xgeSvgCssEscapeNext(&pNext, pEnd, &iCodepoint) ) {
					p = pNext;
					continue;
				}
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
			return 0;
		}
		if ( *p == '{' ) {
			if ( iDepth == 0 ) {
				*ppPreludeEnd = p;
				pBlockStart = p + 1;
			}
			iDepth++;
			p++;
			continue;
		}
		if ( *p == '}' ) {
			if ( iDepth <= 0 ) {
				return 0;
			}
			iDepth--;
			if ( iDepth == 0 ) {
				*ppBlockStart = pBlockStart;
				*ppBlockEnd = p;
				*ppAfter = p + 1;
				return 1;
			}
			p++;
			continue;
		}
		p++;
	}
	return 0;
}

static int __xgeSvgCssMediaQueryMatches(const char* pStart, const char* pEnd)
{
	int bNot;
	int bOnly;
	const char* pTypeStart;
	const char* pTypeEnd;

	if ( (pStart == NULL) || (pEnd == NULL) || (pEnd < pStart) ) {
		return 0;
	}
	__xgeSvgTrimRange(&pStart, &pEnd);
	if ( pStart >= pEnd ) {
		return 0;
	}
	bNot = 0;
	bOnly = 0;
	while ( pStart < pEnd ) {
		const char* pTokenStart = pStart;
		const char* pTokenEnd;

		while ( (pStart < pEnd) && !__xgeSvgCssIsSpace(*pStart) && (*pStart != '(') && (*pStart != ',') ) {
			pStart++;
		}
		pTokenEnd = pStart;
		if ( pTokenEnd <= pTokenStart ) {
			break;
		}
		if ( !bNot && __xgeSvgCssRangeEqualsText(pTokenStart, pTokenEnd, "not") ) {
			bNot = 1;
		} else if ( !bOnly && __xgeSvgCssRangeEqualsText(pTokenStart, pTokenEnd, "only") ) {
			bOnly = 1;
		} else {
			pTypeStart = pTokenStart;
			pTypeEnd = pTokenEnd;
			if ( __xgeSvgCssRangeEqualsText(pTypeStart, pTypeEnd, "all") ||
			     __xgeSvgCssRangeEqualsText(pTypeStart, pTypeEnd, "screen") ) {
				(void)bOnly;
				return !bNot;
			}
			if ( __xgeSvgCssRangeEqualsText(pTypeStart, pTypeEnd, "print") ) {
				return bNot;
			}
			return 0;
		}
		while ( (pStart < pEnd) && __xgeSvgCssIsSpace(*pStart) ) {
			pStart++;
		}
	}
	return 0;
}

static int __xgeSvgCssMediaMatches(const char* pStart, const char* pEnd)
{
	const char* p;
	const char* pItemStart;
	int iParenDepth;
	char cQuote;

	if ( (pStart == NULL) || (pEnd == NULL) || (pEnd < pStart) ) {
		return 0;
	}
	p = pStart;
	pItemStart = pStart;
	iParenDepth = 0;
	cQuote = 0;
	while ( p <= pEnd ) {
		int bAtEnd;
		int bSplit;

		bAtEnd = (p == pEnd);
		bSplit = bAtEnd || ((cQuote == 0) && (iParenDepth == 0) && (*p == ','));
		if ( bSplit ) {
			if ( __xgeSvgCssMediaQueryMatches(pItemStart, p) ) {
				return 1;
			}
			if ( bAtEnd ) {
				break;
			}
			p++;
			pItemStart = p;
			continue;
		}
		if ( cQuote != 0 ) {
			if ( *p == '\\' ) {
				const char* pNext = p;
				uint32_t iCodepoint;

				if ( __xgeSvgCssEscapeNext(&pNext, pEnd, &iCodepoint) ) {
					p = pNext;
					continue;
				}
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
		p++;
	}
	return 0;
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
			const char* pNameStart = p + 1;
			const char* pNameEnd = pNameStart;

			while ( (pNameEnd < pEnd) && __xgeSvgCssIdentChar(*pNameEnd) ) {
				pNameEnd++;
			}
			if ( __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "media") ) {
				const char* pPreludeStart;
				const char* pPreludeEnd;
				const char* pBlockStart;
				const char* pBlockEnd;
				const char* pAfter;

				if ( __xgeSvgCssAtRuleBlock(p, pEnd, &pPreludeStart, &pPreludeEnd, &pBlockStart, &pBlockEnd, &pAfter) ) {
					if ( __xgeSvgCssMediaMatches(pPreludeStart, pPreludeEnd) ) {
						iRet = __xgeSvgParseStyleText(pSvg, pBlockStart, pBlockEnd);
						if ( iRet != XGE_OK ) {
							return iRet;
						}
					}
					p = pAfter;
					continue;
				}
			}
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
	const char* pTextEnd;
	const char* arrSwitchEndStack[XGE_SVG_STACK_MAX];
	const char* arrSwitchSelectedStartStack[XGE_SVG_STACK_MAX];
	const char* arrSwitchSelectedEndStack[XGE_SVG_STACK_MAX];
	int iSwitchTop;

	if ( !__xgeSvgValid(pSvg) || (sText == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iSwitchTop = 0;
	p = sText;
	pTextEnd = sText + strlen(sText);
	while ( (p = strchr(p, '<')) != NULL ) {
		const char* pTagEnd = __xgeSvgFindTagEnd(p);
		int bSelfClosing;

		if ( pTagEnd == NULL ) {
			break;
		}
		bSelfClosing = __xgeSvgIsSelfClosingTag(p, pTagEnd);
		while ( (iSwitchTop > 0) && (p >= arrSwitchEndStack[iSwitchTop - 1]) ) {
			iSwitchTop--;
		}
		if ( iSwitchTop > 0 ) {
			const char* pSelectedStart = arrSwitchSelectedStartStack[iSwitchTop - 1];
			const char* pSelectedEnd = arrSwitchSelectedEndStack[iSwitchTop - 1];

			if ( (pSelectedStart == NULL) || (pSelectedEnd == NULL) || (p < pSelectedStart) || (p >= pSelectedEnd) ) {
				p = pTagEnd + 1;
				continue;
			}
		}
		if ( (p[1] != '/') && !__xgeSvgConditionalElementMatches(p, pTagEnd) ) {
			const char* pElementEnd;

			if ( __xgeSvgElementRangeEnd(p, pTagEnd, pTextEnd, &pElementEnd) ) {
				p = pElementEnd;
			} else {
				p = pTagEnd + 1;
			}
			continue;
		}
		if ( (p[1] != '/') && __xgeSvgTagNameIsNonRenderingContent(p) ) {
			const char* pElementEnd;

			if ( __xgeSvgElementRangeEnd(p, pTagEnd, pTextEnd, &pElementEnd) ) {
				p = pElementEnd;
			} else {
				p = pTagEnd + 1;
			}
			continue;
		}
		if ( __xgeSvgTagNameEquals(p, "switch") ) {
			if ( !bSelfClosing ) {
				const char* pSwitchEnd;
				const char* pSelectedStart;
				const char* pSelectedEnd;

				if ( iSwitchTop >= XGE_SVG_STACK_MAX ) {
					return XGE_ERROR_OUT_OF_MEMORY;
				}
				if ( __xgeSvgSwitchSelectRange(p, pTagEnd, &pSwitchEnd, &pSelectedStart, &pSelectedEnd) ) {
					arrSwitchEndStack[iSwitchTop] = pSwitchEnd;
					arrSwitchSelectedStartStack[iSwitchTop] = pSelectedStart;
					arrSwitchSelectedEndStack[iSwitchTop] = pSelectedEnd;
					iSwitchTop++;
				}
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( !__xgeSvgTagNameEquals(p, "style") ) {
			p = pTagEnd + 1;
			continue;
		}
		{
			const char* pClose = strstr(pTagEnd + 1, "</style");
			const char* pCloseEnd;
			const char* pStyleStart = pTagEnd + 1;
			const char* pStyleEnd;
			int iRet;

			if ( pClose == NULL ) {
				break;
			}
			pCloseEnd = __xgeSvgFindTagEnd(pClose);
			if ( pCloseEnd == NULL ) {
				break;
			}
			pStyleEnd = pClose;

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
			if ( iRet != XGE_OK ) {
				return iRet;
			}
			p = pCloseEnd + 1;
		}
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

static int __xgeSvgAttrNext(const char** ppCursor, const char* pTagEnd, char* sName, int iNameCapacity, char* sValue, int iValueCapacity)
{
	const char* p;
	const char* pNameStart;
	const char* pNameEnd;
	const char* pValueStart;
	const char* pValueEnd;
	char cQuote;
	int iNameLen;

	if ( (ppCursor == NULL) || (*ppCursor == NULL) || (pTagEnd == NULL) ||
	     (sName == NULL) || (iNameCapacity <= 0) || (sValue == NULL) || (iValueCapacity <= 0) ) {
		return 0;
	}
	sName[0] = '\0';
	sValue[0] = '\0';
	p = *ppCursor;
	if ( p >= pTagEnd ) {
		return 0;
	}
	if ( *p == '<' ) {
		p++;
		if ( (p < pTagEnd) && (*p == '/') ) {
			p++;
		}
		while ( (p < pTagEnd) && !__xgeSvgCssIsSpace(*p) && (*p != '/') && (*p != '>') ) {
			p++;
		}
	}
	for ( ;; ) {
		while ( (p < pTagEnd) && __xgeSvgCssIsSpace(*p) ) {
			p++;
		}
		if ( (p >= pTagEnd) || (*p == '/') || (*p == '>') ) {
			*ppCursor = p;
			return 0;
		}
		pNameStart = p;
		while ( (p < pTagEnd) && !__xgeSvgCssIsSpace(*p) && (*p != '=') && (*p != '/') && (*p != '>') ) {
			p++;
		}
		pNameEnd = p;
		while ( (p < pTagEnd) && __xgeSvgCssIsSpace(*p) ) {
			p++;
		}
		if ( (p >= pTagEnd) || (*p != '=') || (pNameEnd <= pNameStart) ) {
			while ( (p < pTagEnd) && !__xgeSvgCssIsSpace(*p) && (*p != '>') ) {
				p++;
			}
			continue;
		}
		p++;
		while ( (p < pTagEnd) && __xgeSvgCssIsSpace(*p) ) {
			p++;
		}
		if ( p >= pTagEnd ) {
			*ppCursor = p;
			return 0;
		}
		cQuote = 0;
		if ( (*p == '"') || (*p == '\'') ) {
			cQuote = *p++;
		}
		pValueStart = p;
		if ( cQuote != 0 ) {
			while ( (p < pTagEnd) && (*p != cQuote) ) {
				p++;
			}
			pValueEnd = p;
			if ( p < pTagEnd ) {
				p++;
			}
		} else {
			while ( (p < pTagEnd) && !__xgeSvgCssIsSpace(*p) && (*p != '>') ) {
				p++;
			}
			pValueEnd = p;
		}
		iNameLen = (int)(pNameEnd - pNameStart);
		if ( iNameLen >= iNameCapacity ) {
			iNameLen = iNameCapacity - 1;
		}
		memcpy(sName, pNameStart, (size_t)iNameLen);
		sName[iNameLen] = '\0';
		__xgeSvgAttrCopyDecoded(pValueStart, pValueEnd, sValue, iValueCapacity);
		*ppCursor = p;
		return 1;
	}
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

static int __xgeSvgHrefStartsWithNoCase(const char* sText, const char* sPrefix)
{
	if ( (sText == NULL) || (sPrefix == NULL) ) {
		return 0;
	}
	while ( *sPrefix != '\0' ) {
		if ( *sText == '\0' ) {
			return 0;
		}
		if ( __xgeSvgCssAsciiLower((unsigned char)*sText) != __xgeSvgCssAsciiLower((unsigned char)*sPrefix) ) {
			return 0;
		}
		sText++;
		sPrefix++;
	}
	return 1;
}

static int __xgeSvgHrefLooksSvgDataImageUri(const char* sHref)
{
	const char* sPrefix = "data:image/svg+xml";
	const char* pAfterMime;

	if ( !__xgeSvgHrefStartsWithNoCase(sHref, sPrefix) ) {
		return 0;
	}
	pAfterMime = sHref + strlen(sPrefix);
	return (*pAfterMime == ';') || (*pAfterMime == ',');
}

static int __xgeSvgHrefLooksExternalSvgRoot(const char* sHref)
{
	if ( (sHref == NULL) || (sHref[0] == '\0') || __xgeSvgHrefIsLocalRef(sHref) || __xgeSvgHrefHasFragment(sHref) ) {
		return 0;
	}
	if ( __xgeSvgUriHasExt(sHref, ".svg") ) {
		return 1;
	}
	return __xgeSvgHrefLooksSvgDataImageUri(sHref);
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

static int __xgeSvgHrefIdCopyFromString(const char* sHref, char* sOut, int iOutCapacity);

static int __xgeSvgHrefIdCopy(const char* pTag, const char* pTagEnd, char* sOut, int iOutCapacity)
{
	char sValue[XGE_SVG_ATTR_MAX];

	if ( (sOut == NULL) || (iOutCapacity <= 0) ) {
		return 0;
	}
	sOut[0] = '\0';
	if ( !__xgeSvgHrefCopyTrimmed(pTag, pTagEnd, sValue, sizeof(sValue)) ) {
		return 0;
	}
	return __xgeSvgHrefIdCopyFromString(sValue, sOut, iOutCapacity);
}

static int __xgeSvgHrefIdCopyFromString(const char* sHref, char* sOut, int iOutCapacity)
{
	const char* pValue;
	int iLen;

	if ( (sOut == NULL) || (iOutCapacity <= 0) ) {
		return 0;
	}
	sOut[0] = '\0';
	if ( sHref == NULL ) {
		return 0;
	}
	pValue = sHref;
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

static void __xgeSvgCssAsciiLowerString(char* sText)
{
	if ( sText == NULL ) {
		return;
	}
	while ( *sText != '\0' ) {
		*sText = (char)__xgeSvgCssAsciiLower((unsigned char)*sText);
		sText++;
	}
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

typedef int (*xge_svg_style_value_valid_fn)(const char* sValue, void* pUser);

static int __xgeSvgCssPropertyNameMatches(const char* pText, size_t iNameLen, const char* sName)
{
	size_t i;

	if ( (pText == NULL) || (sName == NULL) ) {
		return 0;
	}
	for ( i = 0; i < iNameLen; i++ ) {
		if ( __xgeSvgCssAsciiLower((unsigned char)pText[i]) != __xgeSvgCssAsciiLower((unsigned char)sName[i]) ) {
			return 0;
		}
	}
	return sName[iNameLen] == '\0';
}

static int __xgeSvgStyleCopyValidEx(const char* sStyle, const char* sName, char* sOut, int iOutCapacity, int iImportanceMode, xge_svg_style_value_valid_fn pfnValid, void* pUser)
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
		if ( __xgeSvgCssPropertyNameMatches(p, iNameLen, sName) ) {
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
				     ((iImportanceMode == XGE_SVG_STYLE_IMPORTANCE_ANY) || (iImportanceMode == bImportant)) &&
				     ((pfnValid == NULL) || pfnValid(sValue, pUser)) ) {
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

static int __xgeSvgStyleCopyEx(const char* sStyle, const char* sName, char* sOut, int iOutCapacity, int iImportanceMode)
{
	return __xgeSvgStyleCopyValidEx(sStyle, sName, sOut, iOutCapacity, iImportanceMode, NULL, NULL);
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

static int __xgeSvgFloatFinite(float fValue)
{
	return (fValue == fValue) && (fValue >= -XGE_SVG_FLOAT_ABS_MAX) && (fValue <= XGE_SVG_FLOAT_ABS_MAX);
}

static int __xgeSvgNumberIsDigit(char c)
{
	return (c >= '0') && (c <= '9');
}

static int __xgeSvgScanNumberEnd(const char* pText, const char** ppEnd)
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
	while ( __xgeSvgNumberIsDigit(*p) ) {
		iDigitCount++;
		p++;
	}
	iFracDigitCount = 0;
	if ( *p == '.' ) {
		p++;
		while ( __xgeSvgNumberIsDigit(*p) ) {
			iFracDigitCount++;
			p++;
		}
	}
	if ( (iDigitCount + iFracDigitCount) <= 0 ) {
		return 0;
	}
	if ( (*p == 'e') || (*p == 'E') ) {
		const char* pExp;

		pExp = p;
		p++;
		if ( (*p == '+') || (*p == '-') ) {
			p++;
		}
		iExpDigitCount = 0;
		while ( __xgeSvgNumberIsDigit(*p) ) {
			iExpDigitCount++;
			p++;
		}
		if ( iExpDigitCount <= 0 ) {
			p = pExp;
		}
	}
	*ppEnd = p;
	return 1;
}

static int __xgeSvgParseNumberAt(const char* pText, const char** ppEnd, double* pValue)
{
	const char* pNumberEnd;
	char* pEnd;
	double fValue;

	if ( (pText == NULL) || (ppEnd == NULL) || (pValue == NULL) ) {
		return 0;
	}
	if ( !__xgeSvgScanNumberEnd(pText, &pNumberEnd) ) {
		return 0;
	}
	fValue = strtod(pText, &pEnd);
	if ( (pEnd == pText) || (pEnd != pNumberEnd) || !__xgeSvgDoubleIsFiniteFloat(fValue) ) {
		return 0;
	}
	*ppEnd = pNumberEnd;
	*pValue = fValue;
	return 1;
}

static int __xgeSvgParseFloat(const char* sText, float* pValue)
{
	const char* pEnd;
	double fValue;

	if ( (sText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	if ( !__xgeSvgParseNumberAt(sText, &pEnd, &fValue) ) {
		return 0;
	}
	return __xgeSvgDoubleToFloat(fValue, pValue);
}

static int __xgeSvgParseFloatStrict(const char* sText, float* pValue)
{
	const char* p;
	const char* pEnd;
	double fValue;

	if ( (sText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	p = sText;
	while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') ) p++;
	if ( !__xgeSvgParseNumberAt(p, &pEnd, &fValue) ) {
		return 0;
	}
	if ( !__xgeSvgDoubleToFloat(fValue, pValue) ) {
		return 0;
	}
	p = pEnd;
	while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') ) p++;
	return *p == '\0';
}

static void __xgeSvgSkipSpaces(const char** ppText);

static int __xgeSvgParseFloatOrPercent(const char* sText, float* pValue)
{
	const char* pEnd;
	double fValue;

	if ( (sText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	while ( (*sText == ' ') || (*sText == '\t') || (*sText == '\r') || (*sText == '\n') ) sText++;
	if ( !__xgeSvgParseNumberAt(sText, &pEnd, &fValue) ) {
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
	const char* pEnd;
	double fValue;

	if ( sText == NULL ) {
		return 0.0f;
	}
	if ( !__xgeSvgParseNumberAt(sText, &pEnd, &fValue) ) {
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
	const char* pEnd;
	double fValue;

	if ( (sText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	p = sText;
	__xgeSvgSkipSpaces(&p);
	if ( !__xgeSvgParseNumberAt(p, &pEnd, &fValue) ) {
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

static int __xgeSvgParseLengthTokenEx(const char** ppText, float fPercentRef, float fFontSize, float* pValue)
{
	const char* p;
	const char* pNumberEnd;
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
	if ( !__xgeSvgParseNumberAt(p, &pNumberEnd, &fValue) ) {
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

static int __xgeSvgStringHasUnit(const char* sText, const char* sUnit)
{
	const char* p;
	int i;

	if ( (sText == NULL) || (sUnit == NULL) || (sUnit[0] == '\0') ) {
		return 0;
	}
	for ( p = sText; *p != '\0'; p++ ) {
		for ( i = 0; sUnit[i] != '\0'; i++ ) {
			char a = p[i];
			char b = sUnit[i];
			if ( (a >= 'A') && (a <= 'Z') ) a = (char)(a - 'A' + 'a');
			if ( a != b ) {
				break;
			}
		}
		if ( sUnit[i] == '\0' ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeSvgParseLengthLooseEx(const char* sText, float fPercentRef, float fFontSize, float* pValue)
{
	const char* p;
	const char* pUnit;
	const char* pNumberEnd;
	double fValue;
	float fScale;

	if ( (sText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	if ( fFontSize <= 0.0f ) {
		fFontSize = 16.0f;
	}
	p = sText;
	while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') || (*p == ',') ) p++;
	if ( !__xgeSvgParseNumberAt(p, &pNumberEnd, &fValue) ) {
		*pValue = 0.0f;
		return 1;
	}
	pUnit = pNumberEnd;
	while ( (*pUnit == ' ') || (*pUnit == '\t') || (*pUnit == '\r') || (*pUnit == '\n') ) pUnit++;
	if ( *pUnit == '%' ) {
		fValue = fValue * (double)fPercentRef / 100.0;
	} else {
		fScale = 1.0f;
		if ( __xgeSvgStringHasUnit(sText, "em") ) fScale = fFontSize;
		else if ( __xgeSvgStringHasUnit(sText, "ex") ) fScale = fFontSize * 0.5f;
		else if ( __xgeSvgStringHasUnit(sText, "cm") ) fScale = 96.0f / 2.54f;
		else if ( __xgeSvgStringHasUnit(sText, "mm") ) fScale = 96.0f / 25.4f;
		else if ( __xgeSvgStringHasUnit(sText, "pt") ) fScale = 96.0f / 72.0f;
		else if ( __xgeSvgStringHasUnit(sText, "pc") ) fScale = 16.0f;
		else if ( __xgeSvgStringHasUnit(sText, "in") ) fScale = 96.0f;
		fValue *= (double)fScale;
	}
	if ( !__xgeSvgDoubleToFloat(fValue, pValue) ) {
		*pValue = 0.0f;
	}
	return 1;
}

static int __xgeSvgParseLengthToken(const char** ppText, float fPercentRef, float* pValue)
{
	return __xgeSvgParseLengthTokenEx(ppText, fPercentRef, 16.0f, pValue);
}

static int __xgeSvgParseLengthStrictEx(const char* sText, float fPercentRef, float fFontSize, float* pValue)
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

static int __xgeSvgParseLengthEx(const char* sText, float fPercentRef, float fFontSize, float* pValue)
{
	if ( (sText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	if ( __xgeSvgParseLengthStrictEx(sText, fPercentRef, fFontSize, pValue) ) {
		return 1;
	}
	return __xgeSvgParseLengthLooseEx(sText, fPercentRef, fFontSize, pValue);
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
	else if ( !__xgeSvgParseLengthStrictEx(pStart, fInheritedFontSize, fInheritedFontSize, &fValue) ) {
		return 0;
	}
	if ( fValue < 0.0f ) {
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

static float __xgeSvgMarkerRefKeywordValue(int iBasis, xge_rect_t tViewBox, int bHasViewBox, float fViewportSize, float fFactor)
{
	float fStart;
	float fSize;

	fStart = 0.0f;
	fSize = fViewportSize;
	if ( bHasViewBox ) {
		if ( iBasis == XGE_SVG_LENGTH_BASIS_Y ) {
			fStart = tViewBox.fY;
			fSize = tViewBox.fH;
		} else {
			fStart = tViewBox.fX;
			fSize = tViewBox.fW;
		}
	}
	return fStart + (fSize * fFactor);
}

static float __xgeSvgAttrMarkerRef(const char* pTag, const char* pTagEnd, const char* sName, int iBasis, xge_rect_t tViewBox, int bHasViewBox, float fViewportSize, float fFontSize, float fDefault)
{
	char sValue[XGE_SVG_ATTR_MAX];
	float fValue;
	float fPercentRef;

	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, sName, sValue, sizeof(sValue)) ) {
		return fDefault;
	}
	if ( iBasis == XGE_SVG_LENGTH_BASIS_Y ) {
		if ( __xgeSvgColorNameEquals(sValue, "top") ) {
			return __xgeSvgMarkerRefKeywordValue(iBasis, tViewBox, bHasViewBox, fViewportSize, 0.0f);
		}
		if ( __xgeSvgColorNameEquals(sValue, "center") ) {
			return __xgeSvgMarkerRefKeywordValue(iBasis, tViewBox, bHasViewBox, fViewportSize, 0.5f);
		}
		if ( __xgeSvgColorNameEquals(sValue, "bottom") ) {
			return __xgeSvgMarkerRefKeywordValue(iBasis, tViewBox, bHasViewBox, fViewportSize, 1.0f);
		}
	} else {
		if ( __xgeSvgColorNameEquals(sValue, "left") ) {
			return __xgeSvgMarkerRefKeywordValue(iBasis, tViewBox, bHasViewBox, fViewportSize, 0.0f);
		}
		if ( __xgeSvgColorNameEquals(sValue, "center") ) {
			return __xgeSvgMarkerRefKeywordValue(iBasis, tViewBox, bHasViewBox, fViewportSize, 0.5f);
		}
		if ( __xgeSvgColorNameEquals(sValue, "right") ) {
			return __xgeSvgMarkerRefKeywordValue(iBasis, tViewBox, bHasViewBox, fViewportSize, 1.0f);
		}
	}
	fPercentRef = fViewportSize;
	if ( bHasViewBox ) {
		fPercentRef = (iBasis == XGE_SVG_LENGTH_BASIS_Y) ? tViewBox.fH : tViewBox.fW;
	}
	return __xgeSvgParseLengthEx(sValue, fPercentRef, fFontSize, &fValue) ? fValue : fDefault;
}

static void __xgeSvgNormalizeRectRadii(float* pRX, float* pRY, int* pHasRX, int* pHasRY)
{
	if ( (pRX == NULL) || (pRY == NULL) || (pHasRX == NULL) || (pHasRY == NULL) ) {
		return;
	}
	if ( *pHasRX && (*pRX < 0.0f) ) {
		*pRX = 0.0f;
		*pHasRX = 0;
	}
	if ( *pHasRY && (*pRY < 0.0f) ) {
		*pRY = 0.0f;
		*pHasRY = 0;
	}
	if ( *pHasRX && !*pHasRY ) {
		*pRY = *pRX;
		*pHasRY = 1;
	} else if ( !*pHasRX && *pHasRY ) {
		*pRX = *pRY;
		*pHasRX = 1;
	}
	if ( !*pHasRX ) {
		*pRX = 0.0f;
	}
	if ( !*pHasRY ) {
		*pRY = 0.0f;
	}
}

static void __xgeSvgCircleEllipseRadii(xge_svg pSvg, const char* pTag, const char* pTagEnd, int bObjectBounds, float fFontSize, float* pRX, float* pRY)
{
	float fR;

	if ( (pRX == NULL) || (pRY == NULL) ) {
		return;
	}
	if ( __xgeSvgTagNameEquals(pTag, "circle") ) {
		fR = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "r", 1.0f, fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "r", XGE_SVG_LENGTH_BASIS_OTHER, fFontSize, 0.0f);
		*pRX = fR;
		*pRY = fR;
		return;
	}
	*pRX = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "rx", 1.0f, fFontSize, 0.0f) :
		__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "rx", XGE_SVG_LENGTH_BASIS_X, fFontSize, 0.0f);
	*pRY = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "ry", 1.0f, fFontSize, 0.0f) :
		__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "ry", XGE_SVG_LENGTH_BASIS_Y, fFontSize, 0.0f);
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
	const char* pIdEnd;
	char cQuote;

	if ( ppAfter != NULL ) {
		*ppAfter = sValue;
	}
	if ( (sValue == NULL) || (sOut == NULL) || (iOutCapacity <= 0) ) {
		return 0;
	}
	sOut[0] = '\0';
	while ( (*sValue == ' ') || (*sValue == '\t') || (*sValue == '\r') || (*sValue == '\n') ) sValue++;
	if ( (sValue[0] == '\0') || (__xgeSvgCssAsciiLower((unsigned char)sValue[0]) != 'u') ||
	     (sValue[1] == '\0') || (__xgeSvgCssAsciiLower((unsigned char)sValue[1]) != 'r') ||
	     (sValue[2] == '\0') || (__xgeSvgCssAsciiLower((unsigned char)sValue[2]) != 'l') ||
	     (sValue[3] != '(') ) {
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
	while ( *p != '\0' ) {
		if ( *p == '\\' ) {
			uint32_t iCodepoint;
			const char* pNext = p;

			if ( !__xgeSvgCssEscapeNext(&pNext, sValue + strlen(sValue), &iCodepoint) ) {
				return 0;
			}
			p = pNext;
			continue;
		}
		if ( cQuote != 0 ) {
			if ( *p == cQuote ) {
				break;
			}
		} else if ( (*p == ')') || (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') ) {
			break;
		}
		p++;
	}
	pIdEnd = p;
	if ( pIdEnd <= pIdStart ) {
		return 0;
	}
	if ( !__xgeSvgCssIdentifierTokenCopyDecoded(pIdStart, pIdEnd, sOut, iOutCapacity) ) {
		return 0;
	}
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

static int __xgeSvgParseUserObjectUnits(const char* sText, int iUserSpaceValue, int iObjectBoundingBoxValue, int* pUnits)
{
	if ( (sText == NULL) || (pUnits == NULL) ) {
		return 0;
	}
	if ( strcmp(sText, "userSpaceOnUse") == 0 ) {
		*pUnits = iUserSpaceValue;
		return 1;
	}
	if ( strcmp(sText, "objectBoundingBox") == 0 ) {
		*pUnits = iObjectBoundingBoxValue;
		return 1;
	}
	return 0;
}

static int __xgeSvgParseFilterUnits(const char* sText, int* pUnits)
{
	return __xgeSvgParseUserObjectUnits(sText, XGE_SVG_FILTER_UNITS_USER_SPACE, XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX, pUnits);
}

static void __xgeSvgFilterPrimitivesReset(xge_svg_filter_t* pFilter);

static void __xgeSvgFilterPrimitiveParseRegionAttrs(xge_svg_filter_primitive_t* pPrimitive, const char* pTag, const char* pTagEnd)
{
	float fValue;

	if ( (pPrimitive == NULL) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return;
	}
	if ( __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "x", 1.0f, 16.0f, &fValue) ) {
		pPrimitive->tRegion.fX = fValue;
		pPrimitive->iRegionFlags |= XGE_SVG_FILTER_PRIMITIVE_REGION_X;
	}
	if ( __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "y", 1.0f, 16.0f, &fValue) ) {
		pPrimitive->tRegion.fY = fValue;
		pPrimitive->iRegionFlags |= XGE_SVG_FILTER_PRIMITIVE_REGION_Y;
	}
	if ( __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "width", 1.0f, 16.0f, &fValue) ) {
		pPrimitive->tRegion.fW = fValue;
		pPrimitive->iRegionFlags |= XGE_SVG_FILTER_PRIMITIVE_REGION_W;
	}
	if ( __xgeSvgAttrLengthCopyEx(pTag, pTagEnd, "height", 1.0f, 16.0f, &fValue) ) {
		pPrimitive->tRegion.fH = fValue;
		pPrimitive->iRegionFlags |= XGE_SVG_FILTER_PRIMITIVE_REGION_H;
	}
}

static xge_svg_filter_primitive_t* __xgeSvgFilterPrimitiveAppend(xge_svg_filter_t* pFilter, int iType, const char* pTag, const char* pTagEnd)
{
	xge_svg_filter_primitive_t* pPrimitive;

	if ( pFilter == NULL ) {
		return NULL;
	}
	if ( pFilter->iPrimitiveCount >= XGE_SVG_FILTER_PRIMITIVE_MAX ) {
		pFilter->bHasUnsupportedPrimitive = 1;
		return NULL;
	}
	pFilter->arrPrimitiveTypes[pFilter->iPrimitiveCount] = iType;
	pPrimitive = &pFilter->arrPrimitives[pFilter->iPrimitiveCount];
	memset(pPrimitive, 0, sizeof(*pPrimitive));
	pPrimitive->iType = iType;
	if ( (pTag != NULL) && (pTagEnd != NULL) ) {
		__xgeSvgAttrCopy(pTag, pTagEnd, "in", pPrimitive->sIn, sizeof(pPrimitive->sIn));
		__xgeSvgAttrCopy(pTag, pTagEnd, "in2", pPrimitive->sIn2, sizeof(pPrimitive->sIn2));
		__xgeSvgAttrCopy(pTag, pTagEnd, "result", pPrimitive->sResult, sizeof(pPrimitive->sResult));
		__xgeSvgAttrCopy(pTag, pTagEnd, "operator", pPrimitive->sOperator, sizeof(pPrimitive->sOperator));
		__xgeSvgFilterPrimitiveParseRegionAttrs(pPrimitive, pTag, pTagEnd);
	}
	pFilter->iPrimitiveCount++;
	return pPrimitive;
}

static int __xgeSvgFilterPrimitiveMatch1(const xge_svg_filter_t* pFilter, int iType0)
{
	return (pFilter != NULL) && !pFilter->bHasUnsupportedPrimitive &&
	       (pFilter->iPrimitiveCount == 1) && (pFilter->arrPrimitiveTypes[0] == iType0);
}

static int __xgeSvgFilterPrimitiveMatch2(const xge_svg_filter_t* pFilter, int iType0, int iType1)
{
	return (pFilter != NULL) && !pFilter->bHasUnsupportedPrimitive &&
	       (pFilter->iPrimitiveCount == 2) &&
	       (pFilter->arrPrimitiveTypes[0] == iType0) &&
	       (pFilter->arrPrimitiveTypes[1] == iType1);
}

static int __xgeSvgFilterPrimitiveInputIsSource(const xge_svg_filter_t* pFilter, int iPrimitive, const char* sBuiltIn)
{
	const xge_svg_filter_primitive_t* pPrimitive;

	if ( (pFilter == NULL) || (iPrimitive < 0) || (iPrimitive >= pFilter->iPrimitiveCount) || (sBuiltIn == NULL) ) {
		return 0;
	}
	pPrimitive = &pFilter->arrPrimitives[iPrimitive];
	if ( pPrimitive->sIn[0] == '\0' ) {
		return iPrimitive == 0;
	}
	return __xgeSvgColorNameEquals(pPrimitive->sIn, sBuiltIn);
}

static int __xgeSvgFilterPrimitiveInputIsBuiltIn(const xge_svg_filter_t* pFilter, int iPrimitive, int bIn2, const char* sBuiltIn)
{
	const xge_svg_filter_primitive_t* pPrimitive;
	const char* sInput;

	if ( (pFilter == NULL) || (iPrimitive < 0) || (iPrimitive >= pFilter->iPrimitiveCount) || (sBuiltIn == NULL) ) {
		return 0;
	}
	pPrimitive = &pFilter->arrPrimitives[iPrimitive];
	sInput = bIn2 ? pPrimitive->sIn2 : pPrimitive->sIn;
	if ( sInput[0] == '\0' ) {
		return !bIn2 && (iPrimitive == 0) && __xgeSvgColorNameEquals(sBuiltIn, "SourceGraphic");
	}
	return __xgeSvgColorNameEquals(sInput, sBuiltIn);
}

static int __xgeSvgFilterPrimitiveInputMatchesOutput(const xge_svg_filter_t* pFilter, int iConsumer, int bIn2, int iProducer)
{
	const xge_svg_filter_primitive_t* pConsumer;
	const xge_svg_filter_primitive_t* pProducer;
	const char* sInput;

	if ( (pFilter == NULL) || (iConsumer < 0) || (iConsumer >= pFilter->iPrimitiveCount) ||
	     (iProducer < 0) || (iProducer >= pFilter->iPrimitiveCount) || (iConsumer == iProducer) ) {
		return 0;
	}
	pConsumer = &pFilter->arrPrimitives[iConsumer];
	pProducer = &pFilter->arrPrimitives[iProducer];
	sInput = bIn2 ? pConsumer->sIn2 : pConsumer->sIn;
	if ( sInput[0] == '\0' ) {
		return !bIn2 && (iConsumer == (iProducer + 1));
	}
	return (pProducer->sResult[0] != '\0') && (strcmp(sInput, pProducer->sResult) == 0);
}

static int __xgeSvgFilterResultExistsBefore(const xge_svg_filter_t* pFilter, int iPrimitive, const char* sResult)
{
	int i;

	if ( (pFilter == NULL) || (sResult == NULL) || (sResult[0] == '\0') || (iPrimitive <= 0) ) {
		return 0;
	}
	if ( iPrimitive > pFilter->iPrimitiveCount ) {
		iPrimitive = pFilter->iPrimitiveCount;
	}
	for ( i = iPrimitive - 1; i >= 0; i-- ) {
		if ( (pFilter->arrPrimitives[i].sResult[0] != '\0') && (strcmp(sResult, pFilter->arrPrimitives[i].sResult) == 0) ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeSvgFilterPrimitiveInputIsLinearPrevious(const xge_svg_filter_t* pFilter, int iPrimitive)
{
	const xge_svg_filter_primitive_t* pPrimitive;

	if ( (pFilter == NULL) || (iPrimitive < 0) || (iPrimitive >= pFilter->iPrimitiveCount) ) {
		return 0;
	}
	pPrimitive = &pFilter->arrPrimitives[iPrimitive];
	if ( pPrimitive->sIn[0] == '\0' ) {
		return 1;
	}
	if ( iPrimitive == 0 ) {
		return __xgeSvgColorNameEquals(pPrimitive->sIn, "SourceGraphic");
	}
	return __xgeSvgFilterPrimitiveInputMatchesOutput(pFilter, iPrimitive, 0, iPrimitive - 1);
}

static int __xgeSvgFilterPrimitiveMatchComposedDropShadow(const xge_svg_filter_t* pFilter)
{
	int i;
	int iBlur;
	int iOffset;
	int iFlood;
	int iComposite;
	int iMerge;

	if ( (pFilter == NULL) || pFilter->bHasUnsupportedPrimitive ||
	     !pFilter->bHasGaussianBlur || !pFilter->bHasOffset || !pFilter->bHasFlood ||
	     !pFilter->bHasCompositeIn || !pFilter->bHasMerge || !pFilter->bMergeIncludesSourceGraphic ||
	     (pFilter->iPrimitiveCount != 5) ) {
		return 0;
	}
	iBlur = -1;
	iOffset = -1;
	iFlood = -1;
	iComposite = -1;
	iMerge = -1;
	for ( i = 0; i < pFilter->iPrimitiveCount; i++ ) {
		int* pSlot = NULL;

		if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_GAUSSIAN_BLUR ) pSlot = &iBlur;
		else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_OFFSET ) pSlot = &iOffset;
		else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_FLOOD ) pSlot = &iFlood;
		else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_COMPOSITE ) pSlot = &iComposite;
		else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_MERGE ) pSlot = &iMerge;
		else return 0;
		if ( pSlot == NULL || *pSlot >= 0 ) {
			return 0;
		}
		*pSlot = i;
	}
	if ( (iBlur < 0) || (iOffset < 0) || (iFlood < 0) || (iComposite < 0) || (iMerge < 0) ) {
		return 0;
	}
	if ( !__xgeSvgFilterPrimitiveInputIsSource(pFilter, iBlur, "SourceAlpha") &&
	     !__xgeSvgFilterPrimitiveInputIsSource(pFilter, iBlur, "SourceGraphic") ) {
		return 0;
	}
	if ( !__xgeSvgFilterPrimitiveInputMatchesOutput(pFilter, iOffset, 0, iBlur) ) {
		return 0;
	}
	if ( !__xgeSvgFilterPrimitiveInputMatchesOutput(pFilter, iComposite, 0, iFlood) ||
	     !__xgeSvgFilterPrimitiveInputMatchesOutput(pFilter, iComposite, 1, iOffset) ) {
		return 0;
	}
	return (iMerge > iComposite) && pFilter->bMergeIncludesFilterOutput;
}

static void __xgeSvgFilterReset(xge_svg_filter_t* pFilter)
{
	if ( pFilter == NULL ) {
		return;
	}
	__xgeSvgFilterPrimitivesReset(pFilter);
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
	pFilter->bHasOffset = 0;
	pFilter->bHasDropShadow = 0;
	pFilter->bHasGaussianBlur = 0;
	pFilter->bHasColorMatrix = 0;
	pFilter->bHasComponentTransfer = 0;
	pFilter->bHasCompositeIn = 0;
	pFilter->bHasMerge = 0;
	pFilter->bHasBlend = 0;
	pFilter->bMergeIncludesSourceGraphic = 0;
	pFilter->bMergeIncludesFilterOutput = 0;
	pFilter->bHasFlood = 0;
	pFilter->bHasUnsupportedPrimitive = 0;
	__xgeSvgFilterPrimitivesReset(pFilter);
	pFilter->iUnits = XGE_SVG_FILTER_UNITS_USER_SPACE;
	pFilter->iFilterUnits = XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX;
	pFilter->tRegion = (xge_rect_t){-0.1f, -0.1f, 1.2f, 1.2f};
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "filterUnits", sValue, sizeof(sValue)) ) {
		int iUnits;
		if ( __xgeSvgParseFilterUnits(sValue, &iUnits) ) {
			pFilter->iFilterUnits = iUnits;
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
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "primitiveUnits", sValue, sizeof(sValue)) ) {
		int iUnits;
		if ( __xgeSvgParseFilterUnits(sValue, &iUnits) ) {
			pFilter->iUnits = iUnits;
		}
	}
	*pFilterIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgParseFilterFeOffset(xge_svg pSvg, int iFilterIndex, const char* pTag, const char* pTagEnd)
{
	xge_svg_filter_t* pFilter;
	xge_svg_filter_primitive_t* pPrimitive;
	float fOffsetX;
	float fOffsetY;

	if ( !__xgeSvgValid(pSvg) || (iFilterIndex < 0) || (iFilterIndex >= pSvg->iFilterCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFilter = &pSvg->pFilters[iFilterIndex];
	if ( pFilter->iUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX ) {
		fOffsetX = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "dx", 1.0f, 16.0f, 0.0f);
		fOffsetY = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "dy", 1.0f, 16.0f, 0.0f);
	} else {
		fOffsetX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "dx", XGE_SVG_LENGTH_BASIS_X, 16.0f, 0.0f);
		fOffsetY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "dy", XGE_SVG_LENGTH_BASIS_Y, 16.0f, 0.0f);
	}
	pFilter->bHasOffset = 1;
	pPrimitive = __xgeSvgFilterPrimitiveAppend(pFilter, XGE_SVG_FILTER_PRIMITIVE_OFFSET, pTag, pTagEnd);
	if ( pPrimitive != NULL ) {
		pPrimitive->fOffsetX = fOffsetX;
		pPrimitive->fOffsetY = fOffsetY;
	}
	return XGE_OK;
}

static int __xgeSvgParseFilterFeDropShadow(xge_svg pSvg, int iFilterIndex, const char* pTag, const char* pTagEnd)
{
	char sStyle[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	const char* pValue;
	xge_svg_filter_t* pFilter;
	xge_svg_filter_primitive_t* pPrimitive;
	float fStdDeviation;
	float fDropShadowX;
	float fDropShadowY;
	float fDropShadowStdDeviation;
	float fDropShadowOpacity;
	uint32_t iDropShadowColor;
	int bDropShadowCurrentColor;

	if ( !__xgeSvgValid(pSvg) || (iFilterIndex < 0) || (iFilterIndex >= pSvg->iFilterCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFilter = &pSvg->pFilters[iFilterIndex];
	fDropShadowStdDeviation = 2.0f;
	fDropShadowOpacity = 1.0f;
	iDropShadowColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	bDropShadowCurrentColor = 0;
	if ( pFilter->iUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX ) {
		fDropShadowX = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "dx", 1.0f, 16.0f, 2.0f);
		fDropShadowY = __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "dy", 1.0f, 16.0f, 2.0f);
	} else {
		fDropShadowX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "dx", XGE_SVG_LENGTH_BASIS_X, 16.0f, 2.0f);
		fDropShadowY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "dy", XGE_SVG_LENGTH_BASIS_Y, 16.0f, 2.0f);
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
			fDropShadowStdDeviation = fStdDeviation;
		}
	}
	sStyle[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "style", sStyle, sizeof(sStyle));
	if ( __xgeSvgAttrOrStyleCopy(pTag, pTagEnd, sStyle, "flood-color", sValue, sizeof(sValue)) ) {
		if ( __xgeSvgColorNameEquals(sValue, "currentColor") ) {
			bDropShadowCurrentColor = 1;
		} else {
			bDropShadowCurrentColor = 0;
			__xgeSvgParseColor(sValue, &iDropShadowColor);
		}
	}
	if ( __xgeSvgAttrOrStyleCopy(pTag, pTagEnd, sStyle, "flood-opacity", sValue, sizeof(sValue)) ) {
		__xgeSvgParseOpacity(sValue, &fDropShadowOpacity);
	}
	pFilter->bHasDropShadow = 1;
	pPrimitive = __xgeSvgFilterPrimitiveAppend(pFilter, XGE_SVG_FILTER_PRIMITIVE_DROP_SHADOW, pTag, pTagEnd);
	if ( pPrimitive != NULL ) {
		pPrimitive->fDropShadowX = fDropShadowX;
		pPrimitive->fDropShadowY = fDropShadowY;
		pPrimitive->fDropShadowStdDeviation = fDropShadowStdDeviation;
		pPrimitive->fDropShadowOpacity = fDropShadowOpacity;
		pPrimitive->iDropShadowColor = iDropShadowColor;
		pPrimitive->bDropShadowCurrentColor = bDropShadowCurrentColor;
	}
	return XGE_OK;
}

static int __xgeSvgParseFilterFeFlood(xge_svg pSvg, int iFilterIndex, const char* pTag, const char* pTagEnd)
{
	char sStyle[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	xge_svg_filter_t* pFilter;
	xge_svg_filter_primitive_t* pPrimitive;
	uint32_t iFloodColor;
	float fFloodOpacity;
	int bFloodCurrentColor;

	if ( !__xgeSvgValid(pSvg) || (iFilterIndex < 0) || (iFilterIndex >= pSvg->iFilterCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFilter = &pSvg->pFilters[iFilterIndex];
	iFloodColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	fFloodOpacity = 1.0f;
	bFloodCurrentColor = 0;
	sStyle[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "style", sStyle, sizeof(sStyle));
	if ( __xgeSvgAttrOrStyleCopy(pTag, pTagEnd, sStyle, "flood-color", sValue, sizeof(sValue)) ) {
		if ( __xgeSvgColorNameEquals(sValue, "currentColor") ) {
			bFloodCurrentColor = 1;
		} else {
			bFloodCurrentColor = 0;
			__xgeSvgParseColor(sValue, &iFloodColor);
		}
	}
	if ( __xgeSvgAttrOrStyleCopy(pTag, pTagEnd, sStyle, "flood-opacity", sValue, sizeof(sValue)) ) {
		__xgeSvgParseOpacity(sValue, &fFloodOpacity);
	}
	pFilter->bHasFlood = 1;
	pPrimitive = __xgeSvgFilterPrimitiveAppend(pFilter, XGE_SVG_FILTER_PRIMITIVE_FLOOD, pTag, pTagEnd);
	if ( pPrimitive != NULL ) {
		pPrimitive->iFloodColor = iFloodColor;
		pPrimitive->fFloodOpacity = fFloodOpacity;
		pPrimitive->bFloodCurrentColor = bFloodCurrentColor;
	}
	return XGE_OK;
}

static int __xgeSvgParseGaussianBlurStdDeviation(const char* sText, float* pStdX, float* pStdY)
{
	const char* p;
	float arrValues[2];
	int iCount;

	if ( (sText == NULL) || (pStdX == NULL) || (pStdY == NULL) ) {
		return 0;
	}
	p = sText;
	arrValues[0] = 0.0f;
	arrValues[1] = 0.0f;
	iCount = 0;
	while ( (*p != '\0') && (iCount < 2) ) {
		float fValue;

		__xgeSvgSkipSpaces(&p);
		if ( *p == ',' ) {
			p++;
			continue;
		}
		if ( !__xgeSvgParseLengthTokenEx(&p, 1.0f, 16.0f, &fValue) ) {
			break;
		}
		if ( fValue < 0.0f ) {
			break;
		}
		arrValues[iCount++] = fValue;
	}
	if ( iCount <= 0 ) {
		*pStdX = 0.0f;
		*pStdY = 0.0f;
		return 0;
	}
	*pStdX = arrValues[0];
	*pStdY = (iCount == 1) ? arrValues[0] : arrValues[1];
	return 1;
}

static int __xgeSvgParseFilterFeGaussianBlur(xge_svg pSvg, int iFilterIndex, const char* pTag, const char* pTagEnd)
{
	char sValue[XGE_SVG_ATTR_MAX];
	xge_svg_filter_t* pFilter;
	xge_svg_filter_primitive_t* pPrimitive;
	float fStdX;
	float fStdY;

	if ( !__xgeSvgValid(pSvg) || (iFilterIndex < 0) || (iFilterIndex >= pSvg->iFilterCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFilter = &pSvg->pFilters[iFilterIndex];
	fStdX = 0.0f;
	fStdY = 0.0f;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "stdDeviation", sValue, sizeof(sValue)) ) {
		__xgeSvgParseGaussianBlurStdDeviation(sValue, &fStdX, &fStdY);
	}
	pFilter->bHasGaussianBlur = 1;
	pPrimitive = __xgeSvgFilterPrimitiveAppend(pFilter, XGE_SVG_FILTER_PRIMITIVE_GAUSSIAN_BLUR, pTag, pTagEnd);
	if ( pPrimitive != NULL ) {
		pPrimitive->fGaussianBlurX = fStdX;
		pPrimitive->fGaussianBlurY = fStdY;
	}
	return XGE_OK;
}

static int __xgeSvgParseMorphologyRadius(const char* sText, float* pRadiusX, float* pRadiusY)
{
	const char* p;
	float arrValues[2];
	int iCount;

	if ( (sText == NULL) || (pRadiusX == NULL) || (pRadiusY == NULL) ) {
		return 0;
	}
	p = sText;
	arrValues[0] = 0.0f;
	arrValues[1] = 0.0f;
	iCount = 0;
	while ( (*p != '\0') && (iCount < 2) ) {
		float fValue;

		__xgeSvgSkipSpaces(&p);
		if ( *p == ',' ) {
			p++;
			continue;
		}
		if ( !__xgeSvgParseLengthTokenEx(&p, 1.0f, 16.0f, &fValue) ) {
			break;
		}
		if ( fValue < 0.0f ) {
			break;
		}
		arrValues[iCount++] = fValue;
	}
	if ( iCount <= 0 ) {
		*pRadiusX = 0.0f;
		*pRadiusY = 0.0f;
		return 0;
	}
	*pRadiusX = arrValues[0];
	*pRadiusY = (iCount == 1) ? arrValues[0] : arrValues[1];
	return 1;
}

static int __xgeSvgParseFilterFeMorphology(xge_svg pSvg, int iFilterIndex, const char* pTag, const char* pTagEnd)
{
	char sValue[XGE_SVG_ATTR_MAX];
	xge_svg_filter_t* pFilter;
	xge_svg_filter_primitive_t* pPrimitive;
	float fRadiusX;
	float fRadiusY;
	int iOperator;

	if ( !__xgeSvgValid(pSvg) || (iFilterIndex < 0) || (iFilterIndex >= pSvg->iFilterCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFilter = &pSvg->pFilters[iFilterIndex];
	fRadiusX = 0.0f;
	fRadiusY = 0.0f;
	iOperator = XGE_SVG_FILTER_MORPHOLOGY_ERODE;
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "operator", sValue, sizeof(sValue)) ) {
		if ( __xgeSvgColorNameEquals(sValue, "erode") ) {
			iOperator = XGE_SVG_FILTER_MORPHOLOGY_ERODE;
		} else if ( __xgeSvgColorNameEquals(sValue, "dilate") ) {
			iOperator = XGE_SVG_FILTER_MORPHOLOGY_DILATE;
		} else {
			pFilter->bHasUnsupportedPrimitive = 1;
		}
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "radius", sValue, sizeof(sValue)) ) {
		if ( !__xgeSvgParseMorphologyRadius(sValue, &fRadiusX, &fRadiusY) ) {
			pFilter->bHasUnsupportedPrimitive = 1;
		}
	}
	pPrimitive = __xgeSvgFilterPrimitiveAppend(pFilter, XGE_SVG_FILTER_PRIMITIVE_MORPHOLOGY, pTag, pTagEnd);
	if ( pPrimitive != NULL ) {
		pPrimitive->fMorphologyRadiusX = fRadiusX;
		pPrimitive->fMorphologyRadiusY = fRadiusY;
		pPrimitive->iMorphologyOperator = iOperator;
	}
	return XGE_OK;
}

static int __xgeSvgParseFilterFeTile(xge_svg pSvg, int iFilterIndex, const char* pTag, const char* pTagEnd)
{
	xge_svg_filter_t* pFilter;

	if ( !__xgeSvgValid(pSvg) || (iFilterIndex < 0) || (iFilterIndex >= pSvg->iFilterCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFilter = &pSvg->pFilters[iFilterIndex];
	(void)__xgeSvgFilterPrimitiveAppend(pFilter, XGE_SVG_FILTER_PRIMITIVE_TILE, pTag, pTagEnd);
	return XGE_OK;
}

static int __xgeSvgParseFilterFeImage(xge_svg pSvg, int iFilterIndex, const char* pTag, const char* pTagEnd)
{
	char sHref[XGE_SVG_ATTR_MAX];
	char sAspect[XGE_SVG_ATTR_MAX];
	xge_svg_filter_t* pFilter;
	xge_svg_filter_primitive_t* pPrimitive;

	if ( !__xgeSvgValid(pSvg) || (iFilterIndex < 0) || (iFilterIndex >= pSvg->iFilterCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFilter = &pSvg->pFilters[iFilterIndex];
	sHref[0] = '\0';
	sAspect[0] = '\0';
	__xgeSvgHrefCopyTrimmed(pTag, pTagEnd, sHref, sizeof(sHref));
	__xgeSvgAttrCopy(pTag, pTagEnd, "preserveAspectRatio", sAspect, sizeof(sAspect));
	pPrimitive = __xgeSvgFilterPrimitiveAppend(pFilter, XGE_SVG_FILTER_PRIMITIVE_IMAGE, pTag, pTagEnd);
	if ( pPrimitive != NULL ) {
		if ( sHref[0] != '\0' ) {
			pPrimitive->sHref = __xgeStrDup(sHref);
			if ( pPrimitive->sHref == NULL ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
		}
		if ( sAspect[0] != '\0' ) {
			pPrimitive->sAspect = __xgeStrDup(sAspect);
			if ( pPrimitive->sAspect == NULL ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
		}
	}
	return XGE_OK;
}

static void __xgeSvgFilterColorMatrixIdentity(float* pMatrix)
{
	int i;

	if ( pMatrix == NULL ) {
		return;
	}
	for ( i = 0; i < 20; i++ ) {
		pMatrix[i] = 0.0f;
	}
	pMatrix[0] = 1.0f;
	pMatrix[6] = 1.0f;
	pMatrix[12] = 1.0f;
	pMatrix[18] = 1.0f;
}

static int __xgeSvgParseFloatList(const char* sText, float* pValues, int iMaxCount, int* pCount)
{
	const char* p;
	int iCount;

	if ( (sText == NULL) || (pValues == NULL) || (iMaxCount <= 0) || (pCount == NULL) ) {
		return 0;
	}
	p = sText;
	iCount = 0;
	while ( *p != '\0' ) {
		const char* pEnd;
		double fValue;

		__xgeSvgSkipSpaces(&p);
		if ( *p == ',' ) {
			p++;
			continue;
		}
		if ( *p == '\0' ) {
			break;
		}
		if ( iCount >= iMaxCount || !__xgeSvgParseNumberAt(p, &pEnd, &fValue) ) {
			return 0;
		}
		if ( !__xgeSvgDoubleToFloat(fValue, &pValues[iCount]) ) {
			return 0;
		}
		iCount++;
		p = pEnd;
	}
	*pCount = iCount;
	return 1;
}

static void __xgeSvgFilterColorMatrixSaturate(float fS, float* pMatrix)
{
	__xgeSvgFilterColorMatrixIdentity(pMatrix);
	pMatrix[0] = 0.213f + 0.787f * fS;
	pMatrix[1] = 0.715f - 0.715f * fS;
	pMatrix[2] = 0.072f - 0.072f * fS;
	pMatrix[5] = 0.213f - 0.213f * fS;
	pMatrix[6] = 0.715f + 0.285f * fS;
	pMatrix[7] = 0.072f - 0.072f * fS;
	pMatrix[10] = 0.213f - 0.213f * fS;
	pMatrix[11] = 0.715f - 0.715f * fS;
	pMatrix[12] = 0.072f + 0.928f * fS;
}

static void __xgeSvgFilterColorMatrixHueRotate(float fDegrees, float* pMatrix)
{
	float fRadians;
	float fCos;
	float fSin;

	fRadians = fDegrees * 0.01745329251994329577f;
	fCos = cosf(fRadians);
	fSin = sinf(fRadians);
	__xgeSvgFilterColorMatrixIdentity(pMatrix);
	pMatrix[0] = 0.213f + fCos * 0.787f - fSin * 0.213f;
	pMatrix[1] = 0.715f - fCos * 0.715f - fSin * 0.715f;
	pMatrix[2] = 0.072f - fCos * 0.072f + fSin * 0.928f;
	pMatrix[5] = 0.213f - fCos * 0.213f + fSin * 0.143f;
	pMatrix[6] = 0.715f + fCos * 0.285f + fSin * 0.140f;
	pMatrix[7] = 0.072f - fCos * 0.072f - fSin * 0.283f;
	pMatrix[10] = 0.213f - fCos * 0.213f - fSin * 0.787f;
	pMatrix[11] = 0.715f - fCos * 0.715f + fSin * 0.715f;
	pMatrix[12] = 0.072f + fCos * 0.928f + fSin * 0.072f;
}

static void __xgeSvgFilterColorMatrixLuminanceToAlpha(float* pMatrix)
{
	int i;

	if ( pMatrix == NULL ) {
		return;
	}
	for ( i = 0; i < 20; i++ ) {
		pMatrix[i] = 0.0f;
	}
	pMatrix[15] = 0.2125f;
	pMatrix[16] = 0.7154f;
	pMatrix[17] = 0.0721f;
}

static int __xgeSvgParseFilterFeColorMatrix(xge_svg pSvg, int iFilterIndex, const char* pTag, const char* pTagEnd)
{
	char sType[XGE_SVG_ATTR_MAX];
	char sValues[XGE_SVG_ATTR_MAX];
	float arrValues[20];
	float arrColorMatrix[20];
	int iCount;
	xge_svg_filter_t* pFilter;
	xge_svg_filter_primitive_t* pPrimitive;

	if ( !__xgeSvgValid(pSvg) || (iFilterIndex < 0) || (iFilterIndex >= pSvg->iFilterCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFilter = &pSvg->pFilters[iFilterIndex];
	sType[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "type", sType, sizeof(sType));
	sValues[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "values", sValues, sizeof(sValues));
	__xgeSvgFilterColorMatrixIdentity(arrColorMatrix);
	if ( (sType[0] == '\0') || __xgeSvgColorNameEquals(sType, "matrix") ) {
		if ( sValues[0] != '\0' ) {
			iCount = 0;
			if ( __xgeSvgParseFloatList(sValues, arrValues, 20, &iCount) && (iCount == 20) ) {
				memcpy(arrColorMatrix, arrValues, sizeof(arrValues));
			} else {
				pFilter->bHasUnsupportedPrimitive = 1;
			}
		}
	} else if ( __xgeSvgColorNameEquals(sType, "saturate") ) {
		float fS = 1.0f;
		if ( sValues[0] != '\0' ) {
			iCount = 0;
			if ( !__xgeSvgParseFloatList(sValues, arrValues, 1, &iCount) || (iCount != 1) ) {
				pFilter->bHasUnsupportedPrimitive = 1;
			} else {
				fS = arrValues[0];
			}
		}
		__xgeSvgFilterColorMatrixSaturate(fS, arrColorMatrix);
	} else if ( __xgeSvgColorNameEquals(sType, "hueRotate") ) {
		float fDegrees = 0.0f;
		if ( sValues[0] != '\0' ) {
			iCount = 0;
			if ( !__xgeSvgParseFloatList(sValues, arrValues, 1, &iCount) || (iCount != 1) ) {
				pFilter->bHasUnsupportedPrimitive = 1;
			} else {
				fDegrees = arrValues[0];
			}
		}
		__xgeSvgFilterColorMatrixHueRotate(fDegrees, arrColorMatrix);
	} else if ( __xgeSvgColorNameEquals(sType, "luminanceToAlpha") ) {
		__xgeSvgFilterColorMatrixLuminanceToAlpha(arrColorMatrix);
	} else {
		pFilter->bHasUnsupportedPrimitive = 1;
	}
	pFilter->bHasColorMatrix = 1;
	pPrimitive = __xgeSvgFilterPrimitiveAppend(pFilter, XGE_SVG_FILTER_PRIMITIVE_COLOR_MATRIX, pTag, pTagEnd);
	if ( pPrimitive != NULL ) {
		memcpy(pPrimitive->arrColorMatrix, arrColorMatrix, sizeof(pPrimitive->arrColorMatrix));
	}
	return XGE_OK;
}

static void __xgeSvgComponentTransferFuncReset(xge_svg_component_transfer_func_t* pFunc)
{
	if ( pFunc == NULL ) {
		return;
	}
	memset(pFunc, 0, sizeof(*pFunc));
	pFunc->iType = XGE_SVG_COMPONENT_TRANSFER_IDENTITY;
	pFunc->fSlope = 1.0f;
	pFunc->fIntercept = 0.0f;
	pFunc->fAmplitude = 1.0f;
	pFunc->fExponent = 1.0f;
	pFunc->fOffset = 0.0f;
}

static void __xgeSvgComponentTransferFuncsReset(xge_svg_component_transfer_func_t* pFuncs)
{
	int i;

	if ( pFuncs == NULL ) {
		return;
	}
	for ( i = 0; i < 4; i++ ) {
		__xgeSvgComponentTransferFuncReset(&pFuncs[i]);
	}
}

static int __xgeSvgAttrFloatStrict(const char* pTag, const char* pTagEnd, const char* sName, float* pValue)
{
	char sValue[XGE_SVG_ATTR_MAX];
	float arrValues[1];
	int iCount;

	if ( (pTag == NULL) || (pTagEnd == NULL) || (sName == NULL) || (pValue == NULL) ) {
		return 0;
	}
	if ( !__xgeSvgAttrCopy(pTag, pTagEnd, sName, sValue, sizeof(sValue)) ) {
		return 0;
	}
	iCount = 0;
	if ( !__xgeSvgParseFloatList(sValue, arrValues, 1, &iCount) || (iCount != 1) ) {
		return 0;
	}
	*pValue = arrValues[0];
	return 1;
}

static int __xgeSvgParseFilterComponentTransferFunc(const char* pTag, const char* pTagEnd, xge_svg_component_transfer_func_t* pFunc)
{
	char sType[XGE_SVG_ATTR_MAX];
	char sValues[XGE_SVG_ATTR_MAX];
	float arrValues[XGE_SVG_COMPONENT_TRANSFER_TABLE_MAX];
	int iCount;
	int i;

	if ( (pTag == NULL) || (pTagEnd == NULL) || (pFunc == NULL) ) {
		return 0;
	}
	__xgeSvgComponentTransferFuncReset(pFunc);
	sType[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "type", sType, sizeof(sType));
	if ( (sType[0] == '\0') || __xgeSvgColorNameEquals(sType, "identity") ) {
		pFunc->iType = XGE_SVG_COMPONENT_TRANSFER_IDENTITY;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sType, "table") || __xgeSvgColorNameEquals(sType, "discrete") ) {
		pFunc->iType = __xgeSvgColorNameEquals(sType, "table") ? XGE_SVG_COMPONENT_TRANSFER_TABLE : XGE_SVG_COMPONENT_TRANSFER_DISCRETE;
		sValues[0] = '\0';
		__xgeSvgAttrCopy(pTag, pTagEnd, "tableValues", sValues, sizeof(sValues));
		iCount = 0;
		if ( sValues[0] == '\0' ) {
			pFunc->iType = XGE_SVG_COMPONENT_TRANSFER_IDENTITY;
			return 1;
		}
		if ( !__xgeSvgParseFloatList(sValues, arrValues, XGE_SVG_COMPONENT_TRANSFER_TABLE_MAX, &iCount) || (iCount <= 0) ) {
			return 0;
		}
		pFunc->iTableValueCount = iCount;
		for ( i = 0; i < iCount; i++ ) {
			pFunc->arrTableValues[i] = arrValues[i];
		}
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sType, "linear") ) {
		pFunc->iType = XGE_SVG_COMPONENT_TRANSFER_LINEAR;
		__xgeSvgAttrFloatStrict(pTag, pTagEnd, "slope", &pFunc->fSlope);
		__xgeSvgAttrFloatStrict(pTag, pTagEnd, "intercept", &pFunc->fIntercept);
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sType, "gamma") ) {
		pFunc->iType = XGE_SVG_COMPONENT_TRANSFER_GAMMA;
		__xgeSvgAttrFloatStrict(pTag, pTagEnd, "amplitude", &pFunc->fAmplitude);
		__xgeSvgAttrFloatStrict(pTag, pTagEnd, "exponent", &pFunc->fExponent);
		__xgeSvgAttrFloatStrict(pTag, pTagEnd, "offset", &pFunc->fOffset);
		return 1;
	}
	return 0;
}

static int __xgeSvgParseFilterFeComponentTransfer(xge_svg pSvg, int iFilterIndex, const char* pTag, const char* pTagEnd, const char* pElementEnd)
{
	xge_svg_filter_t* pFilter;
	xge_svg_filter_primitive_t* pPrimitive;
	xge_svg_component_transfer_func_t arrComponentTransfer[4];
	const char* pChild;

	if ( !__xgeSvgValid(pSvg) || (iFilterIndex < 0) || (iFilterIndex >= pSvg->iFilterCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFilter = &pSvg->pFilters[iFilterIndex];
	__xgeSvgComponentTransferFuncsReset(arrComponentTransfer);
	pFilter->bHasComponentTransfer = 1;
	if ( pElementEnd != NULL ) {
		pChild = pTagEnd + 1;
		while ( (pChild < pElementEnd) && ((pChild = strchr(pChild, '<')) != NULL) && (pChild < pElementEnd) ) {
			const char* pChildEnd = __xgeSvgFindTagEnd(pChild);
			xge_svg_component_transfer_func_t* pFunc = NULL;

			if ( (pChildEnd == NULL) || (pChildEnd > pElementEnd) ) {
				break;
			}
			if ( pChild[1] == '/' ) {
				pChild = pChildEnd + 1;
				continue;
			}
			if ( __xgeSvgTagNameEquals(pChild, "feFuncR") ) {
				pFunc = &arrComponentTransfer[0];
			} else if ( __xgeSvgTagNameEquals(pChild, "feFuncG") ) {
				pFunc = &arrComponentTransfer[1];
			} else if ( __xgeSvgTagNameEquals(pChild, "feFuncB") ) {
				pFunc = &arrComponentTransfer[2];
			} else if ( __xgeSvgTagNameEquals(pChild, "feFuncA") ) {
				pFunc = &arrComponentTransfer[3];
			} else if ( (pChild[0] == '<') && (pChild[1] == 'f') && (pChild[2] == 'e') ) {
				pFilter->bHasUnsupportedPrimitive = 1;
			}
			if ( (pFunc != NULL) && !__xgeSvgParseFilterComponentTransferFunc(pChild, pChildEnd, pFunc) ) {
				pFilter->bHasUnsupportedPrimitive = 1;
			}
			pChild = pChildEnd + 1;
		}
	}
	pPrimitive = __xgeSvgFilterPrimitiveAppend(pFilter, XGE_SVG_FILTER_PRIMITIVE_COMPONENT_TRANSFER, pTag, pTagEnd);
	if ( pPrimitive != NULL ) {
		memcpy(pPrimitive->arrComponentTransfer, arrComponentTransfer, sizeof(pPrimitive->arrComponentTransfer));
	}
	return XGE_OK;
}

static int __xgeSvgParseFilterCompositeOperator(const char* sText, int* pOperator)
{
	if ( pOperator == NULL ) {
		return 0;
	}
	*pOperator = XGE_SVG_FILTER_COMPOSITE_OVER;
	if ( (sText == NULL) || (sText[0] == '\0') || __xgeSvgColorNameEquals(sText, "over") ) {
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "in") ) {
		*pOperator = XGE_SVG_FILTER_COMPOSITE_IN;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "out") ) {
		*pOperator = XGE_SVG_FILTER_COMPOSITE_OUT;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "atop") ) {
		*pOperator = XGE_SVG_FILTER_COMPOSITE_ATOP;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "xor") ) {
		*pOperator = XGE_SVG_FILTER_COMPOSITE_XOR;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "lighter") ) {
		*pOperator = XGE_SVG_FILTER_COMPOSITE_LIGHTER;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "arithmetic") ) {
		*pOperator = XGE_SVG_FILTER_COMPOSITE_ARITHMETIC;
		return 1;
	}
	return 0;
}

static int __xgeSvgParseFilterFeComposite(xge_svg pSvg, int iFilterIndex, const char* pTag, const char* pTagEnd)
{
	char sOperator[XGE_SVG_ATTR_MAX];
	xge_svg_filter_t* pFilter;
	xge_svg_filter_primitive_t* pPrimitive;
	int iOperator;
	float fK1;
	float fK2;
	float fK3;
	float fK4;

	if ( !__xgeSvgValid(pSvg) || (iFilterIndex < 0) || (iFilterIndex >= pSvg->iFilterCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFilter = &pSvg->pFilters[iFilterIndex];
	sOperator[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "operator", sOperator, sizeof(sOperator));
	if ( !__xgeSvgParseFilterCompositeOperator(sOperator, &iOperator) ) {
		pFilter->bHasUnsupportedPrimitive = 1;
		iOperator = XGE_SVG_FILTER_COMPOSITE_OVER;
	}
	if ( iOperator == XGE_SVG_FILTER_COMPOSITE_IN ) {
		pFilter->bHasCompositeIn = 1;
	}
	fK1 = 0.0f;
	fK2 = 0.0f;
	fK3 = 0.0f;
	fK4 = 0.0f;
	if ( iOperator == XGE_SVG_FILTER_COMPOSITE_ARITHMETIC ) {
		__xgeSvgAttrFloatStrict(pTag, pTagEnd, "k1", &fK1);
		__xgeSvgAttrFloatStrict(pTag, pTagEnd, "k2", &fK2);
		__xgeSvgAttrFloatStrict(pTag, pTagEnd, "k3", &fK3);
		__xgeSvgAttrFloatStrict(pTag, pTagEnd, "k4", &fK4);
	}
	pPrimitive = __xgeSvgFilterPrimitiveAppend(pFilter, XGE_SVG_FILTER_PRIMITIVE_COMPOSITE, pTag, pTagEnd);
	if ( pPrimitive != NULL ) {
		pPrimitive->iCompositeOperator = iOperator;
		pPrimitive->fCompositeK1 = fK1;
		pPrimitive->fCompositeK2 = fK2;
		pPrimitive->fCompositeK3 = fK3;
		pPrimitive->fCompositeK4 = fK4;
	}
	return XGE_OK;
}

static int __xgeSvgParseFilterFeMerge(xge_svg pSvg, int iFilterIndex, const char* pTag, const char* pTagEnd, const char* pElementEnd)
{
	xge_svg_filter_t* pFilter;
	xge_svg_filter_primitive_t* pPrimitive;
	const char* pChild;
	char arrMergeInputs[XGE_SVG_FILTER_MERGE_NODE_MAX][XGE_SVG_ID_MAX];
	int iPreviousPrimitive;
	int iMergeNodeCount;

	if ( !__xgeSvgValid(pSvg) || (iFilterIndex < 0) || (iFilterIndex >= pSvg->iFilterCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFilter = &pSvg->pFilters[iFilterIndex];
	pFilter->bHasMerge = 1;
	iPreviousPrimitive = pFilter->iPrimitiveCount - 1;
	iMergeNodeCount = 0;
	memset(arrMergeInputs, 0, sizeof(arrMergeInputs));
	if ( pElementEnd != NULL ) {
		pChild = pTagEnd + 1;
		while ( (pChild < pElementEnd) && ((pChild = strchr(pChild, '<')) != NULL) && (pChild < pElementEnd) ) {
			const char* pChildEnd = __xgeSvgFindTagEnd(pChild);

			if ( (pChildEnd == NULL) || (pChildEnd > pElementEnd) ) {
				break;
			}
			if ( pChild[1] == '/' ) {
				pChild = pChildEnd + 1;
				continue;
			}
			if ( __xgeSvgTagNameEquals(pChild, "feMergeNode") ) {
				char sInput[XGE_SVG_ID_MAX];

				sInput[0] = '\0';
				__xgeSvgAttrCopy(pChild, pChildEnd, "in", sInput, sizeof(sInput));
				if ( iMergeNodeCount >= XGE_SVG_FILTER_MERGE_NODE_MAX ) {
					pFilter->bHasUnsupportedPrimitive = 1;
				} else {
					snprintf(arrMergeInputs[iMergeNodeCount], sizeof(arrMergeInputs[iMergeNodeCount]), "%s", sInput);
					arrMergeInputs[iMergeNodeCount][sizeof(arrMergeInputs[iMergeNodeCount]) - 1] = '\0';
					iMergeNodeCount++;
				}
				if ( __xgeSvgColorNameEquals(sInput, "SourceGraphic") ) {
					pFilter->bMergeIncludesSourceGraphic = 1;
				} else if ( sInput[0] == '\0' ) {
					if ( iPreviousPrimitive >= 0 ) {
						pFilter->bMergeIncludesFilterOutput = 1;
					}
				} else if ( __xgeSvgColorNameEquals(sInput, "SourceAlpha") ||
				            __xgeSvgColorNameEquals(sInput, "FillPaint") ||
				            __xgeSvgColorNameEquals(sInput, "StrokePaint") ) {
					/* Built-in filter inputs are resolved by the graph evaluator. */
				} else if ( (iPreviousPrimitive >= 0) &&
				            (pFilter->arrPrimitives[iPreviousPrimitive].sResult[0] != '\0') &&
				            (strcmp(sInput, pFilter->arrPrimitives[iPreviousPrimitive].sResult) == 0) ) {
					pFilter->bMergeIncludesFilterOutput = 1;
				} else if ( !__xgeSvgFilterResultExistsBefore(pFilter, pFilter->iPrimitiveCount, sInput) ) {
					pFilter->bHasUnsupportedPrimitive = 1;
				} else {
					/* Named earlier primitive result. */
				}
			} else if ( (pChild[0] == '<') && (pChild[1] == 'f') && (pChild[2] == 'e') ) {
				pFilter->bHasUnsupportedPrimitive = 1;
			}
			pChild = pChildEnd + 1;
		}
	}
	pPrimitive = __xgeSvgFilterPrimitiveAppend(pFilter, XGE_SVG_FILTER_PRIMITIVE_MERGE, pTag, pTagEnd);
	if ( pPrimitive != NULL ) {
		int i;

		pPrimitive->iMergeNodeCount = iMergeNodeCount;
		for ( i = 0; i < iMergeNodeCount; i++ ) {
			memcpy(pPrimitive->arrMergeInputs[i], arrMergeInputs[i], sizeof(pPrimitive->arrMergeInputs[i]));
			pPrimitive->arrMergeInputs[i][sizeof(pPrimitive->arrMergeInputs[i]) - 1] = '\0';
		}
	}
	return XGE_OK;
}

static int __xgeSvgParseFilterBlendMode(const char* sText, int* pBlend)
{
	if ( pBlend == NULL ) {
		return 0;
	}
	*pBlend = XGE_SVG_FILTER_BLEND_NORMAL;
	if ( (sText == NULL) || (sText[0] == '\0') || __xgeSvgColorNameEquals(sText, "normal") ) {
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "multiply") ) {
		*pBlend = XGE_SVG_FILTER_BLEND_MULTIPLY;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "screen") ) {
		*pBlend = XGE_SVG_FILTER_BLEND_SCREEN;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "darken") ) {
		*pBlend = XGE_SVG_FILTER_BLEND_DARKEN;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "lighten") ) {
		*pBlend = XGE_SVG_FILTER_BLEND_LIGHTEN;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "overlay") ) {
		*pBlend = XGE_SVG_FILTER_BLEND_OVERLAY;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "color-dodge") ) {
		*pBlend = XGE_SVG_FILTER_BLEND_COLOR_DODGE;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "color-burn") ) {
		*pBlend = XGE_SVG_FILTER_BLEND_COLOR_BURN;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "hard-light") ) {
		*pBlend = XGE_SVG_FILTER_BLEND_HARD_LIGHT;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "soft-light") ) {
		*pBlend = XGE_SVG_FILTER_BLEND_SOFT_LIGHT;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "difference") ) {
		*pBlend = XGE_SVG_FILTER_BLEND_DIFFERENCE;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "exclusion") ) {
		*pBlend = XGE_SVG_FILTER_BLEND_EXCLUSION;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "hue") ) {
		*pBlend = XGE_SVG_FILTER_BLEND_HUE;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "saturation") ) {
		*pBlend = XGE_SVG_FILTER_BLEND_SATURATION;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "color") ) {
		*pBlend = XGE_SVG_FILTER_BLEND_COLOR;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "luminosity") ) {
		*pBlend = XGE_SVG_FILTER_BLEND_LUMINOSITY;
		return 1;
	}
	return 0;
}

static int __xgeSvgParseFilterFeBlend(xge_svg pSvg, int iFilterIndex, const char* pTag, const char* pTagEnd)
{
	char sMode[XGE_SVG_ATTR_MAX];
	xge_svg_filter_t* pFilter;
	xge_svg_filter_primitive_t* pPrimitive;
	int iBlend;

	if ( !__xgeSvgValid(pSvg) || (iFilterIndex < 0) || (iFilterIndex >= pSvg->iFilterCount) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFilter = &pSvg->pFilters[iFilterIndex];
	sMode[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "mode", sMode, sizeof(sMode));
	if ( !__xgeSvgParseFilterBlendMode(sMode, &iBlend) ) {
		pFilter->bHasUnsupportedPrimitive = 1;
		iBlend = XGE_SVG_FILTER_BLEND_NORMAL;
	}
	pFilter->bHasBlend = 1;
	pPrimitive = __xgeSvgFilterPrimitiveAppend(pFilter, XGE_SVG_FILTER_PRIMITIVE_BLEND, pTag, pTagEnd);
	if ( pPrimitive != NULL ) {
		pPrimitive->iBlendMode = iBlend;
	}
	return XGE_OK;
}

static void __xgeSvgFilterPrimitiveOffsetForBounds(const xge_svg_filter_t* pFilter, const xge_svg_filter_primitive_t* pPrimitive, xge_rect_t tBounds, float* pOffsetX, float* pOffsetY)
{
	float fOffsetX;
	float fOffsetY;

	if ( (pOffsetX == NULL) || (pOffsetY == NULL) ) {
		return;
	}
	fOffsetX = (pPrimitive != NULL) ? pPrimitive->fOffsetX : 0.0f;
	fOffsetY = (pPrimitive != NULL) ? pPrimitive->fOffsetY : 0.0f;
	if ( (pFilter != NULL) && (pFilter->iUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX) ) {
		fOffsetX *= tBounds.fW;
		fOffsetY *= tBounds.fH;
	}
	*pOffsetX = fOffsetX;
	*pOffsetY = fOffsetY;
}

static void __xgeSvgFilterPrimitiveBlurForBounds(const xge_svg_filter_t* pFilter, const xge_svg_filter_primitive_t* pPrimitive, xge_rect_t tBounds, float* pStdX, float* pStdY)
{
	float fStdX;
	float fStdY;

	if ( (pStdX == NULL) || (pStdY == NULL) ) {
		return;
	}
	fStdX = (pPrimitive != NULL) ? pPrimitive->fGaussianBlurX : 0.0f;
	fStdY = (pPrimitive != NULL) ? pPrimitive->fGaussianBlurY : 0.0f;
	if ( (pFilter != NULL) && (pFilter->iUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX) ) {
		fStdX *= tBounds.fW;
		fStdY *= tBounds.fH;
	}
	*pStdX = fStdX;
	*pStdY = fStdY;
}

static int __xgeSvgFilterPrimitiveIsZeroMorphology(const xge_svg_filter_t* pFilter, int iPrimitive)
{
	const xge_svg_filter_primitive_t* pPrimitive;

	if ( (pFilter == NULL) || (iPrimitive < 0) || (iPrimitive >= pFilter->iPrimitiveCount) ||
	     (pFilter->arrPrimitiveTypes[iPrimitive] != XGE_SVG_FILTER_PRIMITIVE_MORPHOLOGY) ) {
		return 0;
	}
	pPrimitive = &pFilter->arrPrimitives[iPrimitive];
	return (fabsf(pPrimitive->fMorphologyRadiusX) <= 0.0001f) &&
	       (fabsf(pPrimitive->fMorphologyRadiusY) <= 0.0001f);
}

static void __xgeSvgFilterPrimitiveMorphologyRadiusForBounds(const xge_svg_filter_t* pFilter, const xge_svg_filter_primitive_t* pPrimitive, xge_rect_t tBounds, float* pRadiusX, float* pRadiusY)
{
	float fRadiusX;
	float fRadiusY;

	if ( (pRadiusX == NULL) || (pRadiusY == NULL) ) {
		return;
	}
	fRadiusX = (pPrimitive != NULL) ? pPrimitive->fMorphologyRadiusX : 0.0f;
	fRadiusY = (pPrimitive != NULL) ? pPrimitive->fMorphologyRadiusY : 0.0f;
	if ( (pFilter != NULL) && (pFilter->iUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX) ) {
		fRadiusX *= tBounds.fW;
		fRadiusY *= tBounds.fH;
	}
	*pRadiusX = fRadiusX;
	*pRadiusY = fRadiusY;
}

static void __xgeSvgFilterPrimitiveDropShadowForBounds(const xge_svg_filter_t* pFilter, const xge_svg_filter_primitive_t* pPrimitive, xge_rect_t tBounds, float* pOffsetX, float* pOffsetY, float* pStdDeviation)
{
	float fOffsetX;
	float fOffsetY;
	float fStdDeviation;

	if ( (pOffsetX == NULL) || (pOffsetY == NULL) || (pStdDeviation == NULL) ) {
		return;
	}
	fOffsetX = (pPrimitive != NULL) ? pPrimitive->fDropShadowX : 0.0f;
	fOffsetY = (pPrimitive != NULL) ? pPrimitive->fDropShadowY : 0.0f;
	fStdDeviation = (pPrimitive != NULL) ? pPrimitive->fDropShadowStdDeviation : 0.0f;
	if ( (pFilter != NULL) && (pFilter->iUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX) ) {
		fOffsetX *= tBounds.fW;
		fOffsetY *= tBounds.fH;
		fStdDeviation *= (tBounds.fW > tBounds.fH) ? tBounds.fW : tBounds.fH;
	}
	*pOffsetX = fOffsetX;
	*pOffsetY = fOffsetY;
	*pStdDeviation = fStdDeviation;
}

static void __xgeSvgFilterPrimitiveReset(xge_svg_filter_primitive_t* pPrimitive)
{
	if ( pPrimitive == NULL ) {
		return;
	}
	xrtFree(pPrimitive->sHref);
	xrtFree(pPrimitive->sAspect);
	memset(pPrimitive, 0, sizeof(*pPrimitive));
}

static void __xgeSvgFilterPrimitivesReset(xge_svg_filter_t* pFilter)
{
	int i;

	if ( pFilter == NULL ) {
		return;
	}
	for ( i = 0; i < XGE_SVG_FILTER_PRIMITIVE_MAX; i++ ) {
		__xgeSvgFilterPrimitiveReset(&pFilter->arrPrimitives[i]);
	}
	pFilter->iPrimitiveCount = 0;
	memset(pFilter->arrPrimitiveTypes, 0, sizeof(pFilter->arrPrimitiveTypes));
}

static int __xgeSvgFilterOffsetForBoundsEx(xge_svg pSvg, const char* sFilterId, xge_rect_t tBounds, float* pOffsetX, float* pOffsetY, int* pSourceAlpha)
{
	xge_svg_filter_t* pFilter;
	int iFilter;
	int i;

	if ( (pOffsetX == NULL) || (pOffsetY == NULL) ) {
		return 0;
	}
	*pOffsetX = 0.0f;
	*pOffsetY = 0.0f;
	if ( pSourceAlpha != NULL ) {
		*pSourceAlpha = 0;
	}
	if ( !__xgeSvgValid(pSvg) || (sFilterId == NULL) || (sFilterId[0] == '\0') ) {
		return 0;
	}
	iFilter = __xgeSvgFilterFind(pSvg, sFilterId);
	if ( iFilter < 0 ) {
		return 0;
	}
	pFilter = &pSvg->pFilters[iFilter];
	if ( pFilter->bHasUnsupportedPrimitive || (pFilter->iPrimitiveCount <= 0) ) {
		return 0;
	}
	for ( i = 0; i < pFilter->iPrimitiveCount; i++ ) {
		float fOffsetX;
		float fOffsetY;

		if ( !__xgeSvgFilterPrimitiveInputIsLinearPrevious(pFilter, i) ) {
			if ( (i == 0) && (pSourceAlpha != NULL) && __xgeSvgColorNameEquals(pFilter->arrPrimitives[i].sIn, "SourceAlpha") ) {
				if ( pSourceAlpha != NULL ) {
					*pSourceAlpha = 1;
				}
			} else {
				*pOffsetX = 0.0f;
				*pOffsetY = 0.0f;
				if ( pSourceAlpha != NULL ) {
					*pSourceAlpha = 0;
				}
				return 0;
			}
		}
		if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_OFFSET ) {
			__xgeSvgFilterPrimitiveOffsetForBounds(pFilter, &pFilter->arrPrimitives[i], tBounds, &fOffsetX, &fOffsetY);
			*pOffsetX += fOffsetX;
			*pOffsetY += fOffsetY;
		} else if ( !__xgeSvgFilterPrimitiveIsZeroMorphology(pFilter, i) ) {
			*pOffsetX = 0.0f;
			*pOffsetY = 0.0f;
			if ( pSourceAlpha != NULL ) {
				*pSourceAlpha = 0;
			}
			return 0;
		}
	}
	return 1;
}

static int __xgeSvgFilterOffsetForBounds(xge_svg pSvg, const char* sFilterId, xge_rect_t tBounds, float* pOffsetX, float* pOffsetY)
{
	return __xgeSvgFilterOffsetForBoundsEx(pSvg, sFilterId, tBounds, pOffsetX, pOffsetY, NULL);
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

static int __xgeSvgFilterPrimitiveRegionForBounds(const xge_svg_filter_t* pFilter, const xge_svg_filter_primitive_t* pPrimitive, xge_rect_t tBounds, xge_rect_t tFallback, xge_rect_t* pRegion)
{
	xge_rect_t tRegion;

	if ( (pFilter == NULL) || (pPrimitive == NULL) || (pRegion == NULL) ) {
		return 0;
	}
	tRegion = tFallback;
	if ( pPrimitive->iRegionFlags & XGE_SVG_FILTER_PRIMITIVE_REGION_X ) {
		tRegion.fX = (pFilter->iUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX) ? (tBounds.fX + pPrimitive->tRegion.fX * tBounds.fW) : pPrimitive->tRegion.fX;
	}
	if ( pPrimitive->iRegionFlags & XGE_SVG_FILTER_PRIMITIVE_REGION_Y ) {
		tRegion.fY = (pFilter->iUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX) ? (tBounds.fY + pPrimitive->tRegion.fY * tBounds.fH) : pPrimitive->tRegion.fY;
	}
	if ( pPrimitive->iRegionFlags & XGE_SVG_FILTER_PRIMITIVE_REGION_W ) {
		tRegion.fW = (pFilter->iUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX) ? (pPrimitive->tRegion.fW * tBounds.fW) : pPrimitive->tRegion.fW;
	}
	if ( pPrimitive->iRegionFlags & XGE_SVG_FILTER_PRIMITIVE_REGION_H ) {
		tRegion.fH = (pFilter->iUnits == XGE_SVG_FILTER_UNITS_OBJECT_BOUNDING_BOX) ? (pPrimitive->tRegion.fH * tBounds.fH) : pPrimitive->tRegion.fH;
	}
	if ( (tRegion.fW <= 0.0f) || (tRegion.fH <= 0.0f) ) {
		memset(pRegion, 0, sizeof(*pRegion));
		return -1;
	}
	*pRegion = tRegion;
	return 1;
}

static int __xgeSvgFilterDropShadowForBounds(xge_svg pSvg, const char* sFilterId, const xge_svg_style_t* pSourceStyle, xge_rect_t tBounds, float* pOffsetX, float* pOffsetY, uint32_t* pColor, float* pOpacity, float* pStdDeviation)
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
	if ( __xgeSvgFilterPrimitiveMatch1(pFilter, XGE_SVG_FILTER_PRIMITIVE_DROP_SHADOW) ) {
		const xge_svg_filter_primitive_t* pPrimitive = &pFilter->arrPrimitives[0];

		if ( (pPrimitive->fDropShadowOpacity <= 0.0f) ||
		     (XGE_COLOR_GET_A(pPrimitive->bDropShadowCurrentColor && (pSourceStyle != NULL) ? pSourceStyle->iCurrentColor : pPrimitive->iDropShadowColor) <= 0) ) {
			return 0;
		}
		__xgeSvgFilterPrimitiveDropShadowForBounds(pFilter, pPrimitive, tBounds, pOffsetX, pOffsetY, pStdDeviation);
		*pColor = (pPrimitive->bDropShadowCurrentColor && (pSourceStyle != NULL)) ? pSourceStyle->iCurrentColor : pPrimitive->iDropShadowColor;
		*pOpacity = pPrimitive->fDropShadowOpacity;
		return 1;
	}
	if ( !__xgeSvgFilterPrimitiveMatchComposedDropShadow(pFilter) ||
	     (pFilter->iPrimitiveCount != 5) ) {
		return 0;
	}
	{
		const xge_svg_filter_primitive_t* pBlur = NULL;
		const xge_svg_filter_primitive_t* pOffset = NULL;
		const xge_svg_filter_primitive_t* pFlood = NULL;
		float fStdX;
		float fStdY;
		int i;

		for ( i = 0; i < pFilter->iPrimitiveCount; i++ ) {
			if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_GAUSSIAN_BLUR ) pBlur = &pFilter->arrPrimitives[i];
			else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_OFFSET ) pOffset = &pFilter->arrPrimitives[i];
			else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_FLOOD ) pFlood = &pFilter->arrPrimitives[i];
		}
		if ( (pBlur == NULL) || (pOffset == NULL) || (pFlood == NULL) ||
		     (pFlood->fFloodOpacity <= 0.0f) ||
		     (XGE_COLOR_GET_A(pFlood->bFloodCurrentColor && (pSourceStyle != NULL) ? pSourceStyle->iCurrentColor : pFlood->iFloodColor) <= 0) ) {
			return 0;
		}
		__xgeSvgFilterPrimitiveOffsetForBounds(pFilter, pOffset, tBounds, pOffsetX, pOffsetY);
		__xgeSvgFilterPrimitiveBlurForBounds(pFilter, pBlur, tBounds, &fStdX, &fStdY);
		*pStdDeviation = fStdX > fStdY ? fStdX : fStdY;
		*pColor = (pFlood->bFloodCurrentColor && (pSourceStyle != NULL)) ? pSourceStyle->iCurrentColor : pFlood->iFloodColor;
		*pOpacity = pFlood->fFloodOpacity;
	}
	return 1;
}

static int __xgeSvgFilterFloodForBounds(xge_svg pSvg, const char* sFilterId, const xge_svg_style_t* pSourceStyle, xge_rect_t tBounds, xge_rect_t tFilterRegion, uint32_t* pColor, float* pOpacity, xge_rect_t* pOutRegion)
{
	xge_svg_filter_t* pFilter;
	int iFilter;

	if ( (pColor == NULL) || (pOpacity == NULL) ) {
		return 0;
	}
	*pColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	*pOpacity = 1.0f;
	if ( !__xgeSvgValid(pSvg) || (sFilterId == NULL) || (sFilterId[0] == '\0') ) {
		return 0;
	}
	iFilter = __xgeSvgFilterFind(pSvg, sFilterId);
	if ( iFilter < 0 ) {
		return 0;
	}
	pFilter = &pSvg->pFilters[iFilter];
	if ( !__xgeSvgFilterPrimitiveMatch1(pFilter, XGE_SVG_FILTER_PRIMITIVE_FLOOD) ) {
		return 0;
	}
	{
		const xge_svg_filter_primitive_t* pPrimitive = &pFilter->arrPrimitives[0];
		xge_rect_t tRegion;

		*pColor = (pPrimitive->bFloodCurrentColor && (pSourceStyle != NULL)) ? pSourceStyle->iCurrentColor : pPrimitive->iFloodColor;
		*pOpacity = pPrimitive->fFloodOpacity;
		if ( pOutRegion != NULL ) {
			if ( __xgeSvgFilterPrimitiveRegionForBounds(pFilter, pPrimitive, tBounds, tFilterRegion, &tRegion) < 0 ) {
				return 0;
			}
			*pOutRegion = tRegion;
		}
	}
	return 1;
}

static int __xgeSvgFilterImageForBounds(xge_svg pSvg, const char* sFilterId, xge_rect_t tBounds, xge_rect_t tFilterRegion, const char** ppHref, const char** ppAspect, xge_rect_t* pOutRegion)
{
	xge_svg_filter_t* pFilter;
	const xge_svg_filter_primitive_t* pPrimitive;
	int iFilter;

	if ( ppHref == NULL ) {
		return 0;
	}
	*ppHref = NULL;
	if ( ppAspect != NULL ) {
		*ppAspect = NULL;
	}
	if ( !__xgeSvgValid(pSvg) || (sFilterId == NULL) || (sFilterId[0] == '\0') ) {
		return 0;
	}
	iFilter = __xgeSvgFilterFind(pSvg, sFilterId);
	if ( iFilter < 0 ) {
		return 0;
	}
	pFilter = &pSvg->pFilters[iFilter];
	if ( !__xgeSvgFilterPrimitiveMatch1(pFilter, XGE_SVG_FILTER_PRIMITIVE_IMAGE) ) {
		return 0;
	}
	pPrimitive = &pFilter->arrPrimitives[0];
	if ( (pPrimitive->sHref == NULL) || (pPrimitive->sHref[0] == '\0') ) {
		return 0;
	}
	if ( pOutRegion != NULL ) {
		xge_rect_t tRegion;

		if ( __xgeSvgFilterPrimitiveRegionForBounds(pFilter, pPrimitive, tBounds, tFilterRegion, &tRegion) < 0 ) {
			return 0;
		}
		*pOutRegion = tRegion;
	}
	*ppHref = pPrimitive->sHref;
	if ( ppAspect != NULL ) {
		*ppAspect = pPrimitive->sAspect;
	}
	return 1;
}

static float __xgeSvgClampUnit(float fValue);

static float __xgeSvgBlendLum(float fR, float fG, float fB)
{
	return 0.3f * fR + 0.59f * fG + 0.11f * fB;
}

static float __xgeSvgBlendSat(float fR, float fG, float fB)
{
	float fMin;
	float fMax;

	fMin = fR;
	if ( fG < fMin ) fMin = fG;
	if ( fB < fMin ) fMin = fB;
	fMax = fR;
	if ( fG > fMax ) fMax = fG;
	if ( fB > fMax ) fMax = fB;
	return fMax - fMin;
}

static void __xgeSvgBlendClipColor(float* pR, float* pG, float* pB)
{
	float fLum;
	float fMin;
	float fMax;
	float fDenom;

	fLum = __xgeSvgBlendLum(*pR, *pG, *pB);
	fMin = *pR;
	if ( *pG < fMin ) fMin = *pG;
	if ( *pB < fMin ) fMin = *pB;
	if ( fMin < 0.0f ) {
		fDenom = fLum - fMin;
		if ( fDenom > 0.000001f ) {
			*pR = fLum + ((*pR - fLum) * fLum) / fDenom;
			*pG = fLum + ((*pG - fLum) * fLum) / fDenom;
			*pB = fLum + ((*pB - fLum) * fLum) / fDenom;
		} else {
			*pR = fLum;
			*pG = fLum;
			*pB = fLum;
		}
	}
	fMax = *pR;
	if ( *pG > fMax ) fMax = *pG;
	if ( *pB > fMax ) fMax = *pB;
	if ( fMax > 1.0f ) {
		fDenom = fMax - fLum;
		if ( fDenom > 0.000001f ) {
			*pR = fLum + ((*pR - fLum) * (1.0f - fLum)) / fDenom;
			*pG = fLum + ((*pG - fLum) * (1.0f - fLum)) / fDenom;
			*pB = fLum + ((*pB - fLum) * (1.0f - fLum)) / fDenom;
		} else {
			*pR = fLum;
			*pG = fLum;
			*pB = fLum;
		}
	}
	*pR = __xgeSvgClampUnit(*pR);
	*pG = __xgeSvgClampUnit(*pG);
	*pB = __xgeSvgClampUnit(*pB);
}

static void __xgeSvgBlendSetLum(float* pR, float* pG, float* pB, float fLum)
{
	float fDelta;

	fDelta = fLum - __xgeSvgBlendLum(*pR, *pG, *pB);
	*pR += fDelta;
	*pG += fDelta;
	*pB += fDelta;
	__xgeSvgBlendClipColor(pR, pG, pB);
}

static void __xgeSvgBlendSetSat(float* pR, float* pG, float* pB, float fSat)
{
	float arrColor[3];
	int iMin;
	int iMid;
	int iMax;
	int iSwap;

	arrColor[0] = *pR;
	arrColor[1] = *pG;
	arrColor[2] = *pB;
	iMin = 0;
	iMid = 1;
	iMax = 2;
	if ( arrColor[iMin] > arrColor[iMid] ) {
		iSwap = iMin;
		iMin = iMid;
		iMid = iSwap;
	}
	if ( arrColor[iMid] > arrColor[iMax] ) {
		iSwap = iMid;
		iMid = iMax;
		iMax = iSwap;
	}
	if ( arrColor[iMin] > arrColor[iMid] ) {
		iSwap = iMin;
		iMin = iMid;
		iMid = iSwap;
	}
	fSat = __xgeSvgClampUnit(fSat);
	if ( arrColor[iMax] > arrColor[iMin] ) {
		arrColor[iMid] = ((arrColor[iMid] - arrColor[iMin]) * fSat) / (arrColor[iMax] - arrColor[iMin]);
		arrColor[iMax] = fSat;
	} else {
		arrColor[iMid] = 0.0f;
		arrColor[iMax] = 0.0f;
	}
	arrColor[iMin] = 0.0f;
	*pR = arrColor[0];
	*pG = arrColor[1];
	*pB = arrColor[2];
}

static float __xgeSvgBlendHardLightUnit(float fSource, float fBackdrop)
{
	if ( fSource <= 0.5f ) {
		return 2.0f * fSource * fBackdrop;
	}
	return 1.0f - 2.0f * (1.0f - fSource) * (1.0f - fBackdrop);
}

static float __xgeSvgBlendSoftLightD(float fBackdrop)
{
	if ( fBackdrop <= 0.25f ) {
		return ((16.0f * fBackdrop - 12.0f) * fBackdrop + 4.0f) * fBackdrop;
	}
	return sqrtf(fBackdrop);
}

static float __xgeSvgBlendComponentUnit(int iBlend, float fSource, float fBackdrop)
{
	float fOut;

	fSource = __xgeSvgClampUnit(fSource);
	fBackdrop = __xgeSvgClampUnit(fBackdrop);
	if ( iBlend == XGE_SVG_FILTER_BLEND_MULTIPLY ) {
		fOut = fSource * fBackdrop;
	} else if ( iBlend == XGE_SVG_FILTER_BLEND_SCREEN ) {
		fOut = fSource + fBackdrop - fSource * fBackdrop;
	} else if ( iBlend == XGE_SVG_FILTER_BLEND_DARKEN ) {
		fOut = fSource < fBackdrop ? fSource : fBackdrop;
	} else if ( iBlend == XGE_SVG_FILTER_BLEND_LIGHTEN ) {
		fOut = fSource > fBackdrop ? fSource : fBackdrop;
	} else if ( iBlend == XGE_SVG_FILTER_BLEND_OVERLAY ) {
		fOut = __xgeSvgBlendHardLightUnit(fBackdrop, fSource);
	} else if ( iBlend == XGE_SVG_FILTER_BLEND_COLOR_DODGE ) {
		if ( fBackdrop <= 0.0f ) fOut = 0.0f;
		else if ( fSource >= 1.0f ) fOut = 1.0f;
		else fOut = fBackdrop / (1.0f - fSource);
	} else if ( iBlend == XGE_SVG_FILTER_BLEND_COLOR_BURN ) {
		if ( fBackdrop >= 1.0f ) fOut = 1.0f;
		else if ( fSource <= 0.0f ) fOut = 0.0f;
		else fOut = 1.0f - (1.0f - fBackdrop) / fSource;
	} else if ( iBlend == XGE_SVG_FILTER_BLEND_HARD_LIGHT ) {
		fOut = __xgeSvgBlendHardLightUnit(fSource, fBackdrop);
	} else if ( iBlend == XGE_SVG_FILTER_BLEND_SOFT_LIGHT ) {
		if ( fSource <= 0.5f ) {
			fOut = fBackdrop - (1.0f - 2.0f * fSource) * fBackdrop * (1.0f - fBackdrop);
		} else {
			fOut = fBackdrop + (2.0f * fSource - 1.0f) * (__xgeSvgBlendSoftLightD(fBackdrop) - fBackdrop);
		}
	} else if ( iBlend == XGE_SVG_FILTER_BLEND_DIFFERENCE ) {
		fOut = fBackdrop - fSource;
		if ( fOut < 0.0f ) fOut = -fOut;
	} else if ( iBlend == XGE_SVG_FILTER_BLEND_EXCLUSION ) {
		fOut = fBackdrop + fSource - 2.0f * fBackdrop * fSource;
	} else {
		fOut = fSource;
	}
	return __xgeSvgClampUnit(fOut);
}

static void __xgeSvgBlendColorTriplet(int iBlend, float fSR, float fSG, float fSB, float fBR, float fBG, float fBB, float* pR, float* pG, float* pB)
{
	float fR;
	float fG;
	float fB;

	if ( iBlend == XGE_SVG_FILTER_BLEND_HUE ) {
		fR = fSR;
		fG = fSG;
		fB = fSB;
		__xgeSvgBlendSetSat(&fR, &fG, &fB, __xgeSvgBlendSat(fBR, fBG, fBB));
		__xgeSvgBlendSetLum(&fR, &fG, &fB, __xgeSvgBlendLum(fBR, fBG, fBB));
	} else if ( iBlend == XGE_SVG_FILTER_BLEND_SATURATION ) {
		fR = fBR;
		fG = fBG;
		fB = fBB;
		__xgeSvgBlendSetSat(&fR, &fG, &fB, __xgeSvgBlendSat(fSR, fSG, fSB));
		__xgeSvgBlendSetLum(&fR, &fG, &fB, __xgeSvgBlendLum(fBR, fBG, fBB));
	} else if ( iBlend == XGE_SVG_FILTER_BLEND_COLOR ) {
		fR = fSR;
		fG = fSG;
		fB = fSB;
		__xgeSvgBlendSetLum(&fR, &fG, &fB, __xgeSvgBlendLum(fBR, fBG, fBB));
	} else if ( iBlend == XGE_SVG_FILTER_BLEND_LUMINOSITY ) {
		fR = fBR;
		fG = fBG;
		fB = fBB;
		__xgeSvgBlendSetLum(&fR, &fG, &fB, __xgeSvgBlendLum(fSR, fSG, fSB));
	} else {
		fR = __xgeSvgBlendComponentUnit(iBlend, fSR, fBR);
		fG = __xgeSvgBlendComponentUnit(iBlend, fSG, fBG);
		fB = __xgeSvgBlendComponentUnit(iBlend, fSB, fBB);
	}
	*pR = __xgeSvgClampUnit(fR);
	*pG = __xgeSvgClampUnit(fG);
	*pB = __xgeSvgClampUnit(fB);
}

static uint32_t __xgeSvgFilterColorMatrixApplyColor(const float* pMatrix, uint32_t iColor);
static uint32_t __xgeSvgFilterComponentTransferApplyColor(const xge_svg_component_transfer_func_t* pFuncs, uint32_t iColor);
static uint32_t __xgeSvgColorAlpha(uint32_t iColor, float fAlpha);
static float __xgeSvgUnitByte(int iValue);
static uint32_t __xgeSvgCompositePremulColor(float fR, float fG, float fB, float fA);

typedef struct xge_svg_filter_color_value_t {
	int bValid;
	int bDependsOnSource;
	int bHasRegion;
	xge_rect_t tRegion;
	uint32_t iColor;
	char sName[XGE_SVG_ID_MAX];
} xge_svg_filter_color_value_t;

static int __xgeSvgFilterColorInputResolveName(const xge_svg_filter_t* pFilter, const xge_svg_filter_color_value_t* pValues, int iPrimitive, const char* sInput, int bAllowDefaultPrevious, xge_svg_filter_color_value_t tSource, xge_svg_filter_color_value_t tFillPaint, xge_svg_filter_color_value_t tStrokePaint, xge_svg_filter_color_value_t* pOut)
{
	int i;

	if ( (pOut == NULL) || (pFilter == NULL) || (pValues == NULL) || (iPrimitive < 0) || (iPrimitive >= pFilter->iPrimitiveCount) ) {
		return 0;
	}
	memset(pOut, 0, sizeof(*pOut));
	if ( (sInput == NULL) || (sInput[0] == '\0') ) {
		if ( !bAllowDefaultPrevious ) return 0;
		if ( iPrimitive == 0 ) {
			*pOut = tSource;
			return 1;
		}
		if ( pValues[iPrimitive - 1].bValid ) {
			*pOut = pValues[iPrimitive - 1];
			return 1;
		}
		return 0;
	}
	if ( __xgeSvgColorNameEquals(sInput, "SourceGraphic") ) {
		*pOut = tSource;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sInput, "SourceAlpha") ) {
		*pOut = tSource;
		pOut->iColor = XGE_COLOR_RGBA(0, 0, 0, XGE_COLOR_GET_A(tSource.iColor));
		strcpy(pOut->sName, "SourceAlpha");
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sInput, "FillPaint") ) {
		*pOut = tFillPaint;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sInput, "StrokePaint") ) {
		*pOut = tStrokePaint;
		return 1;
	}
	for ( i = iPrimitive - 1; i >= 0; i-- ) {
		if ( pValues[i].bValid && (pValues[i].sName[0] != '\0') && (strcmp(sInput, pValues[i].sName) == 0) ) {
			*pOut = pValues[i];
			return 1;
		}
	}
	return 0;
}

static int __xgeSvgFilterColorInputResolve(const xge_svg_filter_t* pFilter, const xge_svg_filter_color_value_t* pValues, int iPrimitive, int bIn2, xge_svg_filter_color_value_t tSource, xge_svg_filter_color_value_t tFillPaint, xge_svg_filter_color_value_t tStrokePaint, xge_svg_filter_color_value_t* pOut)
{
	const xge_svg_filter_primitive_t* pPrimitive;
	const char* sInput;

	if ( (pFilter == NULL) || (iPrimitive < 0) || (iPrimitive >= pFilter->iPrimitiveCount) ) {
		return 0;
	}
	pPrimitive = &pFilter->arrPrimitives[iPrimitive];
	sInput = bIn2 ? pPrimitive->sIn2 : pPrimitive->sIn;
	return __xgeSvgFilterColorInputResolveName(pFilter, pValues, iPrimitive, sInput, !bIn2, tSource, tFillPaint, tStrokePaint, pOut);
}

static uint32_t __xgeSvgFilterBlendApplyColor(uint32_t iSourceColor, uint32_t iBackdropColor, int iBlend)
{
	float fSA;
	float fBA;
	float fSR;
	float fSG;
	float fSB;
	float fBR;
	float fBG;
	float fBB;
	float fBlendR;
	float fBlendG;
	float fBlendB;
	float fOR;
	float fOG;
	float fOB;
	float fOA;

	fSA = __xgeSvgUnitByte(XGE_COLOR_GET_A(iSourceColor));
	fBA = __xgeSvgUnitByte(XGE_COLOR_GET_A(iBackdropColor));
	fSR = __xgeSvgUnitByte(XGE_COLOR_GET_R(iSourceColor));
	fSG = __xgeSvgUnitByte(XGE_COLOR_GET_G(iSourceColor));
	fSB = __xgeSvgUnitByte(XGE_COLOR_GET_B(iSourceColor));
	fBR = __xgeSvgUnitByte(XGE_COLOR_GET_R(iBackdropColor));
	fBG = __xgeSvgUnitByte(XGE_COLOR_GET_G(iBackdropColor));
	fBB = __xgeSvgUnitByte(XGE_COLOR_GET_B(iBackdropColor));
	__xgeSvgBlendColorTriplet(iBlend, fSR, fSG, fSB, fBR, fBG, fBB, &fBlendR, &fBlendG, &fBlendB);
	fOR = fSR * fSA * (1.0f - fBA) + fBlendR * fSA * fBA + fBR * fBA * (1.0f - fSA);
	fOG = fSG * fSA * (1.0f - fBA) + fBlendG * fSA * fBA + fBG * fBA * (1.0f - fSA);
	fOB = fSB * fSA * (1.0f - fBA) + fBlendB * fSA * fBA + fBB * fBA * (1.0f - fSA);
	fOA = fSA + fBA * (1.0f - fSA);
	return __xgeSvgCompositePremulColor(fOR, fOG, fOB, fOA);
}

static float __xgeSvgUnitByte(int iValue)
{
	if ( iValue < 0 ) iValue = 0;
	else if ( iValue > 255 ) iValue = 255;
	return (float)iValue / 255.0f;
}

static int __xgeSvgCompositeByte(float fValue)
{
	int iValue;

	if ( fValue < 0.0f ) fValue = 0.0f;
	else if ( fValue > 1.0f ) fValue = 1.0f;
	iValue = (int)(fValue * 255.0f + 0.5f);
	if ( iValue < 0 ) iValue = 0;
	else if ( iValue > 255 ) iValue = 255;
	return iValue;
}

static uint32_t __xgeSvgCompositePremulColor(float fR, float fG, float fB, float fA)
{
	float fInvA;

	if ( fA <= 0.00001f ) {
		return XGE_COLOR_RGBA(0, 0, 0, 0);
	}
	if ( fA > 1.0f ) fA = 1.0f;
	fInvA = 1.0f / fA;
	return XGE_COLOR_RGBA(
		__xgeSvgCompositeByte(fR * fInvA),
		__xgeSvgCompositeByte(fG * fInvA),
		__xgeSvgCompositeByte(fB * fInvA),
		__xgeSvgCompositeByte(fA));
}

static uint32_t __xgeSvgCompositeOverColor(uint32_t iSourceColor, uint32_t iBackdropColor)
{
	float fSA;
	float fBA;
	float fSR;
	float fSG;
	float fSB;
	float fBR;
	float fBG;
	float fBB;

	fSA = __xgeSvgUnitByte(XGE_COLOR_GET_A(iSourceColor));
	fBA = __xgeSvgUnitByte(XGE_COLOR_GET_A(iBackdropColor));
	fSR = __xgeSvgUnitByte(XGE_COLOR_GET_R(iSourceColor)) * fSA;
	fSG = __xgeSvgUnitByte(XGE_COLOR_GET_G(iSourceColor)) * fSA;
	fSB = __xgeSvgUnitByte(XGE_COLOR_GET_B(iSourceColor)) * fSA;
	fBR = __xgeSvgUnitByte(XGE_COLOR_GET_R(iBackdropColor)) * fBA;
	fBG = __xgeSvgUnitByte(XGE_COLOR_GET_G(iBackdropColor)) * fBA;
	fBB = __xgeSvgUnitByte(XGE_COLOR_GET_B(iBackdropColor)) * fBA;
	return __xgeSvgCompositePremulColor(
		fSR + fBR * (1.0f - fSA),
		fSG + fBG * (1.0f - fSA),
		fSB + fBB * (1.0f - fSA),
		fSA + fBA * (1.0f - fSA));
}

static uint32_t __xgeSvgFilterCompositeApplyColor(const xge_svg_filter_primitive_t* pPrimitive, uint32_t iInColor, uint32_t iIn2Color)
{
	float fSA;
	float fBA;
	float fSR;
	float fSG;
	float fSB;
	float fBR;
	float fBG;
	float fBB;
	float fOR;
	float fOG;
	float fOB;
	float fOA;

	if ( pPrimitive == NULL ) {
		return iInColor;
	}
	fSA = __xgeSvgUnitByte(XGE_COLOR_GET_A(iInColor));
	fBA = __xgeSvgUnitByte(XGE_COLOR_GET_A(iIn2Color));
	fSR = __xgeSvgUnitByte(XGE_COLOR_GET_R(iInColor));
	fSG = __xgeSvgUnitByte(XGE_COLOR_GET_G(iInColor));
	fSB = __xgeSvgUnitByte(XGE_COLOR_GET_B(iInColor));
	fBR = __xgeSvgUnitByte(XGE_COLOR_GET_R(iIn2Color));
	fBG = __xgeSvgUnitByte(XGE_COLOR_GET_G(iIn2Color));
	fBB = __xgeSvgUnitByte(XGE_COLOR_GET_B(iIn2Color));
	if ( pPrimitive->iCompositeOperator == XGE_SVG_FILTER_COMPOSITE_ARITHMETIC ) {
		fOR = pPrimitive->fCompositeK1 * fSR * fBR + pPrimitive->fCompositeK2 * fSR + pPrimitive->fCompositeK3 * fBR + pPrimitive->fCompositeK4;
		fOG = pPrimitive->fCompositeK1 * fSG * fBG + pPrimitive->fCompositeK2 * fSG + pPrimitive->fCompositeK3 * fBG + pPrimitive->fCompositeK4;
		fOB = pPrimitive->fCompositeK1 * fSB * fBB + pPrimitive->fCompositeK2 * fSB + pPrimitive->fCompositeK3 * fBB + pPrimitive->fCompositeK4;
		fOA = pPrimitive->fCompositeK1 * fSA * fBA + pPrimitive->fCompositeK2 * fSA + pPrimitive->fCompositeK3 * fBA + pPrimitive->fCompositeK4;
		return XGE_COLOR_RGBA(__xgeSvgCompositeByte(fOR), __xgeSvgCompositeByte(fOG), __xgeSvgCompositeByte(fOB), __xgeSvgCompositeByte(fOA));
	}
	fSR *= fSA;
	fSG *= fSA;
	fSB *= fSA;
	fBR *= fBA;
	fBG *= fBA;
	fBB *= fBA;
	if ( pPrimitive->iCompositeOperator == XGE_SVG_FILTER_COMPOSITE_IN ) {
		fOR = fSR * fBA;
		fOG = fSG * fBA;
		fOB = fSB * fBA;
		fOA = fSA * fBA;
	} else if ( pPrimitive->iCompositeOperator == XGE_SVG_FILTER_COMPOSITE_OUT ) {
		fOR = fSR * (1.0f - fBA);
		fOG = fSG * (1.0f - fBA);
		fOB = fSB * (1.0f - fBA);
		fOA = fSA * (1.0f - fBA);
	} else if ( pPrimitive->iCompositeOperator == XGE_SVG_FILTER_COMPOSITE_ATOP ) {
		fOR = fSR * fBA + fBR * (1.0f - fSA);
		fOG = fSG * fBA + fBG * (1.0f - fSA);
		fOB = fSB * fBA + fBB * (1.0f - fSA);
		fOA = fBA;
	} else if ( pPrimitive->iCompositeOperator == XGE_SVG_FILTER_COMPOSITE_XOR ) {
		fOR = fSR * (1.0f - fBA) + fBR * (1.0f - fSA);
		fOG = fSG * (1.0f - fBA) + fBG * (1.0f - fSA);
		fOB = fSB * (1.0f - fBA) + fBB * (1.0f - fSA);
		fOA = fSA * (1.0f - fBA) + fBA * (1.0f - fSA);
	} else if ( pPrimitive->iCompositeOperator == XGE_SVG_FILTER_COMPOSITE_LIGHTER ) {
		fOR = fSR + fBR;
		fOG = fSG + fBG;
		fOB = fSB + fBB;
		fOA = fSA + fBA;
		if ( fOR > 1.0f ) fOR = 1.0f;
		if ( fOG > 1.0f ) fOG = 1.0f;
		if ( fOB > 1.0f ) fOB = 1.0f;
		if ( fOA > 1.0f ) fOA = 1.0f;
	} else {
		fOR = fSR + fBR * (1.0f - fSA);
		fOG = fSG + fBG * (1.0f - fSA);
		fOB = fSB + fBB * (1.0f - fSA);
		fOA = fSA + fBA * (1.0f - fSA);
	}
	return __xgeSvgCompositePremulColor(fOR, fOG, fOB, fOA);
}

static void __xgeSvgFilterCompositeRegion(const xge_svg_filter_primitive_t* pPrimitive, xge_svg_filter_color_value_t tIn, xge_svg_filter_color_value_t tIn2, xge_svg_filter_color_value_t* pOut)
{
	if ( pOut == NULL ) {
		return;
	}
	pOut->bHasRegion = tIn.bHasRegion || tIn2.bHasRegion;
	if ( !tIn.bHasRegion && !tIn2.bHasRegion ) {
		return;
	}
	if ( tIn.bHasRegion && !tIn2.bHasRegion ) {
		pOut->tRegion = tIn.tRegion;
		return;
	}
	if ( !tIn.bHasRegion && tIn2.bHasRegion ) {
		pOut->tRegion = tIn2.tRegion;
		return;
	}
	if ( (pPrimitive != NULL) && (pPrimitive->iCompositeOperator == XGE_SVG_FILTER_COMPOSITE_IN) ) {
		pOut->tRegion = __xgeSvgRectIntersect(tIn.tRegion, tIn2.tRegion);
	} else if ( (pPrimitive != NULL) && (pPrimitive->iCompositeOperator == XGE_SVG_FILTER_COMPOSITE_ATOP) ) {
		pOut->tRegion = tIn2.tRegion;
	} else if ( (pPrimitive != NULL) && (pPrimitive->iCompositeOperator == XGE_SVG_FILTER_COMPOSITE_OUT) ) {
		pOut->tRegion = tIn.tRegion;
	} else {
		pOut->tRegion = __xgeSvgRectUnion(tIn.tRegion, tIn2.tRegion);
	}
}

static int __xgeSvgFilterSolidColorGraphEvaluateEx(const xge_svg_filter_t* pFilter, const xge_svg_style_t* pStyle, uint32_t iSourceColor, int iDependencyMode, xge_rect_t tBounds, xge_rect_t tFilterRegion, uint32_t* pOutColor, xge_rect_t* pOutRegion)
{
	xge_svg_filter_color_value_t arrValues[XGE_SVG_FILTER_PRIMITIVE_MAX];
	xge_svg_filter_color_value_t tSource;
	xge_svg_filter_color_value_t tFillPaint;
	xge_svg_filter_color_value_t tStrokePaint;
	xge_svg_filter_color_value_t tFinal;
	int i;

	if ( (pFilter == NULL) || (pStyle == NULL) || (pOutColor == NULL) || pFilter->bHasUnsupportedPrimitive ||
	     (pFilter->iPrimitiveCount <= 0) || (pFilter->iPrimitiveCount > XGE_SVG_FILTER_PRIMITIVE_MAX) ) {
		return 0;
	}
	memset(arrValues, 0, sizeof(arrValues));
	memset(&tSource, 0, sizeof(tSource));
	tSource.bValid = 1;
	tSource.bDependsOnSource = 1;
	tSource.bHasRegion = 1;
	tSource.tRegion = tBounds;
	tSource.iColor = iSourceColor;
	strcpy(tSource.sName, "SourceGraphic");
	memset(&tFillPaint, 0, sizeof(tFillPaint));
	tFillPaint.bValid = 1;
	tFillPaint.bDependsOnSource = 0;
	tFillPaint.bHasRegion = 1;
	tFillPaint.tRegion = tFilterRegion;
	tFillPaint.iColor = pStyle->iFillColor;
	strcpy(tFillPaint.sName, "FillPaint");
	memset(&tStrokePaint, 0, sizeof(tStrokePaint));
	tStrokePaint.bValid = 1;
	tStrokePaint.bDependsOnSource = 0;
	tStrokePaint.bHasRegion = 1;
	tStrokePaint.tRegion = tFilterRegion;
	tStrokePaint.iColor = pStyle->iStrokeColor;
	strcpy(tStrokePaint.sName, "StrokePaint");
	for ( i = 0; i < pFilter->iPrimitiveCount; i++ ) {
		xge_svg_filter_color_value_t tOut;
		xge_svg_filter_color_value_t tIn;
		xge_svg_filter_color_value_t tIn2;
		const xge_svg_filter_primitive_t* pPrimitive = &pFilter->arrPrimitives[i];

		memset(&tOut, 0, sizeof(tOut));
		if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_FLOOD ) {
			uint32_t iFloodColor = pPrimitive->bFloodCurrentColor ? pStyle->iCurrentColor : pPrimitive->iFloodColor;
			xge_rect_t tRegion;

			tOut.bValid = 1;
			tOut.bDependsOnSource = 0;
			if ( __xgeSvgFilterPrimitiveRegionForBounds(pFilter, pPrimitive, tBounds, tFilterRegion, &tRegion) < 0 ) {
				return 0;
			}
			tOut.bHasRegion = 1;
			tOut.tRegion = tRegion;
			tOut.iColor = __xgeSvgColorAlpha(iFloodColor, pPrimitive->fFloodOpacity);
		} else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_COLOR_MATRIX ) {
			if ( !__xgeSvgFilterColorInputResolve(pFilter, arrValues, i, 0, tSource, tFillPaint, tStrokePaint, &tIn) ) {
				return 0;
			}
			tOut = tIn;
			tOut.iColor = __xgeSvgFilterColorMatrixApplyColor(pPrimitive->arrColorMatrix, tIn.iColor);
		} else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_COMPONENT_TRANSFER ) {
			if ( !__xgeSvgFilterColorInputResolve(pFilter, arrValues, i, 0, tSource, tFillPaint, tStrokePaint, &tIn) ) {
				return 0;
			}
			tOut = tIn;
			tOut.iColor = __xgeSvgFilterComponentTransferApplyColor(pPrimitive->arrComponentTransfer, tIn.iColor);
		} else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_COMPOSITE ) {
			if ( !__xgeSvgFilterColorInputResolve(pFilter, arrValues, i, 0, tSource, tFillPaint, tStrokePaint, &tIn) ||
			     !__xgeSvgFilterColorInputResolve(pFilter, arrValues, i, 1, tSource, tFillPaint, tStrokePaint, &tIn2) ) {
				return 0;
			}
			tOut.bValid = 1;
			tOut.bDependsOnSource = tIn.bDependsOnSource || tIn2.bDependsOnSource;
			__xgeSvgFilterCompositeRegion(pPrimitive, tIn, tIn2, &tOut);
			tOut.iColor = __xgeSvgFilterCompositeApplyColor(pPrimitive, tIn.iColor, tIn2.iColor);
		} else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_MERGE ) {
			int iNode;

			if ( pPrimitive->iMergeNodeCount <= 0 ) {
				return 0;
			}
			for ( iNode = 0; iNode < pPrimitive->iMergeNodeCount; iNode++ ) {
				if ( !__xgeSvgFilterColorInputResolveName(pFilter, arrValues, i, pPrimitive->arrMergeInputs[iNode], 1, tSource, tFillPaint, tStrokePaint, &tIn) ) {
					return 0;
				}
				if ( iNode == 0 ) {
					tOut = tIn;
				} else {
					tOut.bValid = 1;
					tOut.bDependsOnSource = tOut.bDependsOnSource || tIn.bDependsOnSource;
					if ( tOut.bHasRegion && tIn.bHasRegion ) tOut.tRegion = __xgeSvgRectUnion(tOut.tRegion, tIn.tRegion);
					else if ( tIn.bHasRegion ) {
						tOut.bHasRegion = 1;
						tOut.tRegion = tIn.tRegion;
					}
					tOut.iColor = __xgeSvgCompositeOverColor(tIn.iColor, tOut.iColor);
				}
			}
		} else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_BLEND ) {
			if ( !__xgeSvgFilterColorInputResolve(pFilter, arrValues, i, 0, tSource, tFillPaint, tStrokePaint, &tIn) ||
			     !__xgeSvgFilterColorInputResolve(pFilter, arrValues, i, 1, tSource, tFillPaint, tStrokePaint, &tIn2) ) {
				return 0;
			}
			tOut.bValid = 1;
			tOut.bDependsOnSource = tIn.bDependsOnSource || tIn2.bDependsOnSource;
			tOut.bHasRegion = tIn.bHasRegion || tIn2.bHasRegion;
			if ( tIn.bHasRegion && tIn2.bHasRegion ) tOut.tRegion = __xgeSvgRectUnion(tIn.tRegion, tIn2.tRegion);
			else if ( tIn.bHasRegion ) tOut.tRegion = tIn.tRegion;
			else if ( tIn2.bHasRegion ) tOut.tRegion = tIn2.tRegion;
			tOut.iColor = __xgeSvgFilterBlendApplyColor(tIn.iColor, tIn2.iColor, pPrimitive->iBlendMode);
		} else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_OFFSET ) {
			float fOffsetX;
			float fOffsetY;

			if ( !__xgeSvgFilterColorInputResolve(pFilter, arrValues, i, 0, tSource, tFillPaint, tStrokePaint, &tIn) ) {
				return 0;
			}
			__xgeSvgFilterPrimitiveOffsetForBounds(pFilter, pPrimitive, tBounds, &fOffsetX, &fOffsetY);
			if ( ((fabsf(fOffsetX) > 0.0001f) || (fabsf(fOffsetY) > 0.0001f)) && tIn.bDependsOnSource ) {
				return 0;
			}
			tOut = tIn;
			if ( tOut.bHasRegion ) {
				tOut.tRegion.fX += fOffsetX;
				tOut.tRegion.fY += fOffsetY;
			}
		} else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_GAUSSIAN_BLUR ) {
			if ( (fabsf(pPrimitive->fGaussianBlurX) > 0.0001f) || (fabsf(pPrimitive->fGaussianBlurY) > 0.0001f) ||
			     !__xgeSvgFilterColorInputResolve(pFilter, arrValues, i, 0, tSource, tFillPaint, tStrokePaint, &tIn) ) {
				return 0;
			}
			tOut = tIn;
		} else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_MORPHOLOGY ) {
			float fRadiusX;
			float fRadiusY;

			if ( !__xgeSvgFilterColorInputResolve(pFilter, arrValues, i, 0, tSource, tFillPaint, tStrokePaint, &tIn) ) {
				return 0;
			}
			__xgeSvgFilterPrimitiveMorphologyRadiusForBounds(pFilter, pPrimitive, tBounds, &fRadiusX, &fRadiusY);
			if ( ((fabsf(fRadiusX) > 0.0001f) || (fabsf(fRadiusY) > 0.0001f)) && tIn.bDependsOnSource ) {
				return 0;
			}
			tOut = tIn;
			if ( tOut.bHasRegion && ((fabsf(fRadiusX) > 0.0001f) || (fabsf(fRadiusY) > 0.0001f)) ) {
				xge_rect_t tPrimitiveRegion;

				if ( pPrimitive->iMorphologyOperator == XGE_SVG_FILTER_MORPHOLOGY_DILATE ) {
					tOut.tRegion.fX -= fRadiusX;
					tOut.tRegion.fY -= fRadiusY;
					tOut.tRegion.fW += fRadiusX * 2.0f;
					tOut.tRegion.fH += fRadiusY * 2.0f;
				} else {
					tOut.tRegion.fX += fRadiusX;
					tOut.tRegion.fY += fRadiusY;
					tOut.tRegion.fW -= fRadiusX * 2.0f;
					tOut.tRegion.fH -= fRadiusY * 2.0f;
					if ( tOut.tRegion.fW < 0.0f ) tOut.tRegion.fW = 0.0f;
					if ( tOut.tRegion.fH < 0.0f ) tOut.tRegion.fH = 0.0f;
				}
				if ( __xgeSvgFilterPrimitiveRegionForBounds(pFilter, pPrimitive, tBounds, tFilterRegion, &tPrimitiveRegion) < 0 ) {
					return 0;
				}
				tOut.tRegion = __xgeSvgRectIntersect(tOut.tRegion, tPrimitiveRegion);
			}
		} else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_TILE ) {
			xge_rect_t tRegion;

			if ( !__xgeSvgFilterColorInputResolve(pFilter, arrValues, i, 0, tSource, tFillPaint, tStrokePaint, &tIn) ||
			     tIn.bDependsOnSource ) {
				return 0;
			}
			tOut = tIn;
			if ( __xgeSvgFilterPrimitiveRegionForBounds(pFilter, pPrimitive, tBounds, tFilterRegion, &tRegion) < 0 ) {
				return 0;
			}
			tOut.bHasRegion = 1;
			tOut.tRegion = tRegion;
		} else {
			return 0;
		}
		if ( pPrimitive->iRegionFlags != 0 ) {
			xge_rect_t tRegion;
			int bHadRegion = tOut.bHasRegion;
			xge_rect_t tFallback = tOut.bHasRegion ? tOut.tRegion : tFilterRegion;

			if ( __xgeSvgFilterPrimitiveRegionForBounds(pFilter, pPrimitive, tBounds, tFallback, &tRegion) < 0 ) {
				return 0;
			}
			tOut.bHasRegion = 1;
			tOut.tRegion = bHadRegion ? __xgeSvgRectIntersect(tFallback, tRegion) : tRegion;
		}
		if ( pPrimitive->sResult[0] != '\0' ) {
			snprintf(tOut.sName, sizeof(tOut.sName), "%s", pPrimitive->sResult);
			tOut.sName[sizeof(tOut.sName) - 1] = '\0';
		} else {
			tOut.sName[0] = '\0';
		}
		arrValues[i] = tOut;
	}
	tFinal = arrValues[pFilter->iPrimitiveCount - 1];
	if ( !tFinal.bValid ) {
		return 0;
	}
	if ( (iDependencyMode > 0) && !tFinal.bDependsOnSource ) {
		return 0;
	}
	if ( (iDependencyMode < 0) && tFinal.bDependsOnSource ) {
		return 0;
	}
	*pOutColor = tFinal.iColor;
	if ( pOutRegion != NULL ) {
		*pOutRegion = tFinal.bHasRegion ? tFinal.tRegion : tFilterRegion;
	}
	return 1;
}

static int __xgeSvgFilterSolidColorGraphEvaluate(const xge_svg_filter_t* pFilter, const xge_svg_style_t* pStyle, uint32_t iSourceColor, uint32_t* pOutColor)
{
	xge_rect_t tZero;

	memset(&tZero, 0, sizeof(tZero));
	return __xgeSvgFilterSolidColorGraphEvaluateEx(pFilter, pStyle, iSourceColor, 1, tZero, tZero, pOutColor, NULL);
}

static int __xgeSvgFilterInputIsPaint(const char* sInput)
{
	return (sInput != NULL) &&
	       (__xgeSvgColorNameEquals(sInput, "FillPaint") || __xgeSvgColorNameEquals(sInput, "StrokePaint"));
}

static int __xgeSvgFilterColorGraphReferencesPaintInput(const xge_svg_filter_t* pFilter)
{
	int i;

	if ( pFilter == NULL ) {
		return 0;
	}
	for ( i = 0; i < pFilter->iPrimitiveCount; i++ ) {
		const xge_svg_filter_primitive_t* pPrimitive = &pFilter->arrPrimitives[i];
		int iNode;

		if ( __xgeSvgFilterInputIsPaint(pPrimitive->sIn) || __xgeSvgFilterInputIsPaint(pPrimitive->sIn2) ) {
			return 1;
		}
		for ( iNode = 0; iNode < pPrimitive->iMergeNodeCount; iNode++ ) {
			if ( __xgeSvgFilterInputIsPaint(pPrimitive->arrMergeInputs[iNode]) ) {
				return 1;
			}
		}
	}
	return 0;
}

static int __xgeSvgFilterIndependentColorGraphForStyle(xge_svg pSvg, const char* sFilterId, const xge_svg_style_t* pStyle, xge_rect_t tBounds, xge_rect_t tFilterRegion, uint32_t* pOutColor, xge_rect_t* pOutRegion)
{
	xge_svg_filter_t* pFilter;
	int iFilter;

	if ( !__xgeSvgValid(pSvg) || (sFilterId == NULL) || (sFilterId[0] == '\0') || (pStyle == NULL) || (pOutColor == NULL) ) {
		return 0;
	}
	iFilter = __xgeSvgFilterFind(pSvg, sFilterId);
	if ( iFilter < 0 ) {
		return 0;
	}
	pFilter = &pSvg->pFilters[iFilter];
	if ( __xgeSvgFilterColorGraphReferencesPaintInput(pFilter) &&
	     ((pStyle->sFillGradientId[0] != '\0') || (pStyle->sStrokeGradientId[0] != '\0')) ) {
		return 0;
	}
	return __xgeSvgFilterSolidColorGraphEvaluateEx(pFilter, pStyle, pStyle->iFillColor, -1, tBounds, tFilterRegion, pOutColor, pOutRegion);
}

static int __xgeSvgFilterSolidPaintGraphForStyle(xge_svg pSvg, const char* sFilterId, xge_svg_style_t* pStyle, int bIncludeStroke)
{
	xge_svg_filter_t* pFilter;
	uint32_t iFillColor;
	uint32_t iStrokeColor;
	int iFilter;

	if ( !__xgeSvgValid(pSvg) || (sFilterId == NULL) || (sFilterId[0] == '\0') || (pStyle == NULL) ) {
		return 0;
	}
	if ( (pStyle->sFillGradientId[0] != '\0') || (bIncludeStroke && (pStyle->sStrokeGradientId[0] != '\0')) ) {
		return 0;
	}
	iFilter = __xgeSvgFilterFind(pSvg, sFilterId);
	if ( iFilter < 0 ) {
		return 0;
	}
	pFilter = &pSvg->pFilters[iFilter];
	if ( !__xgeSvgFilterSolidColorGraphEvaluate(pFilter, pStyle, pStyle->iFillColor, &iFillColor) ) {
		return 0;
	}
	if ( bIncludeStroke ) {
		if ( !__xgeSvgFilterSolidColorGraphEvaluate(pFilter, pStyle, pStyle->iStrokeColor, &iStrokeColor) ) {
			return 0;
		}
		pStyle->iStrokeColor = iStrokeColor;
	}
	pStyle->iFillColor = iFillColor;
	return 1;
}

static int __xgeSvgFilterGaussianBlurOffsetForBoundsEx(xge_svg pSvg, const char* sFilterId, xge_rect_t tBounds, float* pStdX, float* pStdY, float* pOffsetX, float* pOffsetY, int* pSourceAlpha)
{
	xge_svg_filter_t* pFilter;
	int iFilter;
	int i;
	int iBlurCount;

	if ( (pStdX == NULL) || (pStdY == NULL) ) {
		return 0;
	}
	*pStdX = 0.0f;
	*pStdY = 0.0f;
	if ( pOffsetX != NULL ) *pOffsetX = 0.0f;
	if ( pOffsetY != NULL ) *pOffsetY = 0.0f;
	if ( pSourceAlpha != NULL ) {
		*pSourceAlpha = 0;
	}
	if ( !__xgeSvgValid(pSvg) || (sFilterId == NULL) || (sFilterId[0] == '\0') ) {
		return 0;
	}
	iFilter = __xgeSvgFilterFind(pSvg, sFilterId);
	if ( iFilter < 0 ) {
		return 0;
	}
	pFilter = &pSvg->pFilters[iFilter];
	if ( pFilter->bHasUnsupportedPrimitive || (pFilter->iPrimitiveCount <= 0) ) {
		return 0;
	}
	iBlurCount = 0;
	for ( i = 0; i < pFilter->iPrimitiveCount; i++ ) {
		float fValueX;
		float fValueY;

		if ( !__xgeSvgFilterPrimitiveInputIsLinearPrevious(pFilter, i) ) {
			if ( (i == 0) && (pSourceAlpha != NULL) && __xgeSvgColorNameEquals(pFilter->arrPrimitives[i].sIn, "SourceAlpha") ) {
				*pSourceAlpha = 1;
			} else {
				*pStdX = 0.0f;
				*pStdY = 0.0f;
				if ( pOffsetX != NULL ) *pOffsetX = 0.0f;
				if ( pOffsetY != NULL ) *pOffsetY = 0.0f;
				if ( pSourceAlpha != NULL ) *pSourceAlpha = 0;
				return 0;
			}
		}
		if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_OFFSET ) {
			__xgeSvgFilterPrimitiveOffsetForBounds(pFilter, &pFilter->arrPrimitives[i], tBounds, &fValueX, &fValueY);
			if ( pOffsetX != NULL ) *pOffsetX += fValueX;
			if ( pOffsetY != NULL ) *pOffsetY += fValueY;
		} else if ( pFilter->arrPrimitiveTypes[i] == XGE_SVG_FILTER_PRIMITIVE_GAUSSIAN_BLUR ) {
			if ( iBlurCount > 0 ) {
				*pStdX = 0.0f;
				*pStdY = 0.0f;
				if ( pOffsetX != NULL ) *pOffsetX = 0.0f;
				if ( pOffsetY != NULL ) *pOffsetY = 0.0f;
				if ( pSourceAlpha != NULL ) *pSourceAlpha = 0;
				return 0;
			}
			__xgeSvgFilterPrimitiveBlurForBounds(pFilter, &pFilter->arrPrimitives[i], tBounds, pStdX, pStdY);
			iBlurCount++;
		} else if ( __xgeSvgFilterPrimitiveIsZeroMorphology(pFilter, i) ) {
			continue;
		} else {
			*pStdX = 0.0f;
			*pStdY = 0.0f;
			if ( pOffsetX != NULL ) *pOffsetX = 0.0f;
			if ( pOffsetY != NULL ) *pOffsetY = 0.0f;
			if ( pSourceAlpha != NULL ) *pSourceAlpha = 0;
			return 0;
		}
	}
	return (iBlurCount == 1) && ((*pStdX > 0.0001f) || (*pStdY > 0.0001f));
}

static int __xgeSvgFilterGaussianBlurOffsetForBounds(xge_svg pSvg, const char* sFilterId, xge_rect_t tBounds, float* pStdX, float* pStdY, float* pOffsetX, float* pOffsetY)
{
	return __xgeSvgFilterGaussianBlurOffsetForBoundsEx(pSvg, sFilterId, tBounds, pStdX, pStdY, pOffsetX, pOffsetY, NULL);
}

static int __xgeSvgFilterMorphologyForBounds(xge_svg pSvg, const char* sFilterId, xge_rect_t tBounds, int iOperator, float* pRadiusX, float* pRadiusY, int* pSourceAlpha)
{
	xge_svg_filter_t* pFilter;
	xge_svg_filter_primitive_t* pPrimitive;
	int iFilter;

	if ( (pRadiusX == NULL) || (pRadiusY == NULL) ) {
		return 0;
	}
	*pRadiusX = 0.0f;
	*pRadiusY = 0.0f;
	if ( pSourceAlpha != NULL ) {
		*pSourceAlpha = 0;
	}
	if ( !__xgeSvgValid(pSvg) || (sFilterId == NULL) || (sFilterId[0] == '\0') ) {
		return 0;
	}
	iFilter = __xgeSvgFilterFind(pSvg, sFilterId);
	if ( iFilter < 0 ) {
		return 0;
	}
	pFilter = &pSvg->pFilters[iFilter];
	if ( pFilter->bHasUnsupportedPrimitive || (pFilter->iPrimitiveCount != 1) ||
	     (pFilter->arrPrimitiveTypes[0] != XGE_SVG_FILTER_PRIMITIVE_MORPHOLOGY) ) {
		return 0;
	}
	pPrimitive = &pFilter->arrPrimitives[0];
	if ( pPrimitive->iMorphologyOperator != iOperator ) {
		return 0;
	}
	if ( pPrimitive->sIn[0] == '\0' ) {
		if ( pSourceAlpha != NULL ) {
			*pSourceAlpha = 0;
		}
	} else if ( __xgeSvgColorNameEquals(pPrimitive->sIn, "SourceGraphic") ) {
		if ( pSourceAlpha != NULL ) {
			*pSourceAlpha = 0;
		}
	} else if ( (pSourceAlpha != NULL) && __xgeSvgColorNameEquals(pPrimitive->sIn, "SourceAlpha") ) {
		*pSourceAlpha = 1;
	} else {
		return 0;
	}
	__xgeSvgFilterPrimitiveMorphologyRadiusForBounds(pFilter, pPrimitive, tBounds, pRadiusX, pRadiusY);
	return (fabsf(*pRadiusX) > 0.0001f) || (fabsf(*pRadiusY) > 0.0001f);
}

static int __xgeSvgFilterMorphologyDilateForBounds(xge_svg pSvg, const char* sFilterId, xge_rect_t tBounds, float* pRadiusX, float* pRadiusY, int* pSourceAlpha)
{
	return __xgeSvgFilterMorphologyForBounds(pSvg, sFilterId, tBounds, XGE_SVG_FILTER_MORPHOLOGY_DILATE, pRadiusX, pRadiusY, pSourceAlpha);
}

static int __xgeSvgFilterMorphologyErodeForBounds(xge_svg pSvg, const char* sFilterId, xge_rect_t tBounds, float* pRadiusX, float* pRadiusY, int* pSourceAlpha)
{
	return __xgeSvgFilterMorphologyForBounds(pSvg, sFilterId, tBounds, XGE_SVG_FILTER_MORPHOLOGY_ERODE, pRadiusX, pRadiusY, pSourceAlpha);
}

static float __xgeSvgClampUnit(float fValue)
{
	if ( fValue < 0.0f ) return 0.0f;
	if ( fValue > 1.0f ) return 1.0f;
	return fValue;
}

static uint32_t __xgeSvgFilterColorMatrixApplyColor(const float* pMatrix, uint32_t iColor)
{
	float r;
	float g;
	float b;
	float a;
	float nr;
	float ng;
	float nb;
	float na;
	int ir;
	int ig;
	int ib;
	int ia;

	if ( pMatrix == NULL ) {
		return iColor;
	}
	r = (float)XGE_COLOR_GET_R(iColor) / 255.0f;
	g = (float)XGE_COLOR_GET_G(iColor) / 255.0f;
	b = (float)XGE_COLOR_GET_B(iColor) / 255.0f;
	a = (float)XGE_COLOR_GET_A(iColor) / 255.0f;
	nr = __xgeSvgClampUnit(pMatrix[0] * r + pMatrix[1] * g + pMatrix[2] * b + pMatrix[3] * a + pMatrix[4]);
	ng = __xgeSvgClampUnit(pMatrix[5] * r + pMatrix[6] * g + pMatrix[7] * b + pMatrix[8] * a + pMatrix[9]);
	nb = __xgeSvgClampUnit(pMatrix[10] * r + pMatrix[11] * g + pMatrix[12] * b + pMatrix[13] * a + pMatrix[14]);
	na = __xgeSvgClampUnit(pMatrix[15] * r + pMatrix[16] * g + pMatrix[17] * b + pMatrix[18] * a + pMatrix[19]);
	ir = (int)(nr * 255.0f + 0.5f);
	ig = (int)(ng * 255.0f + 0.5f);
	ib = (int)(nb * 255.0f + 0.5f);
	ia = (int)(na * 255.0f + 0.5f);
	if ( ir < 0 ) ir = 0; else if ( ir > 255 ) ir = 255;
	if ( ig < 0 ) ig = 0; else if ( ig > 255 ) ig = 255;
	if ( ib < 0 ) ib = 0; else if ( ib > 255 ) ib = 255;
	if ( ia < 0 ) ia = 0; else if ( ia > 255 ) ia = 255;
	return XGE_COLOR_RGBA(ir, ig, ib, ia);
}

static float __xgeSvgComponentTransferApplyValue(const xge_svg_component_transfer_func_t* pFunc, float fValue)
{
	float fResult;

	if ( pFunc == NULL ) {
		return __xgeSvgClampUnit(fValue);
	}
	fValue = __xgeSvgClampUnit(fValue);
	fResult = fValue;
	if ( pFunc->iType == XGE_SVG_COMPONENT_TRANSFER_TABLE ) {
		int iCount = pFunc->iTableValueCount;
		if ( iCount <= 0 ) {
			fResult = fValue;
		} else if ( iCount == 1 ) {
			fResult = pFunc->arrTableValues[0];
		} else {
			float fScaled = fValue * (float)(iCount - 1);
			int iIndex = (int)floorf(fScaled);
			float fT;

			if ( iIndex < 0 ) iIndex = 0;
			if ( iIndex >= (iCount - 1) ) {
				iIndex = iCount - 2;
				fT = 1.0f;
			} else {
				fT = fScaled - (float)iIndex;
			}
			fResult = pFunc->arrTableValues[iIndex] * (1.0f - fT) + pFunc->arrTableValues[iIndex + 1] * fT;
		}
	} else if ( pFunc->iType == XGE_SVG_COMPONENT_TRANSFER_DISCRETE ) {
		int iCount = pFunc->iTableValueCount;
		if ( iCount <= 0 ) {
			fResult = fValue;
		} else {
			int iIndex = (int)floorf(fValue * (float)iCount);
			if ( iIndex < 0 ) iIndex = 0;
			if ( iIndex >= iCount ) iIndex = iCount - 1;
			fResult = pFunc->arrTableValues[iIndex];
		}
	} else if ( pFunc->iType == XGE_SVG_COMPONENT_TRANSFER_LINEAR ) {
		fResult = pFunc->fSlope * fValue + pFunc->fIntercept;
	} else if ( pFunc->iType == XGE_SVG_COMPONENT_TRANSFER_GAMMA ) {
		fResult = pFunc->fAmplitude * powf(fValue, pFunc->fExponent) + pFunc->fOffset;
	}
	if ( !((fResult >= 0.0f) || (fResult < 0.0f)) ) {
		fResult = 0.0f;
	}
	return __xgeSvgClampUnit(fResult);
}

static uint32_t __xgeSvgFilterComponentTransferApplyColor(const xge_svg_component_transfer_func_t* pFuncs, uint32_t iColor)
{
	float r;
	float g;
	float b;
	float a;
	int ir;
	int ig;
	int ib;
	int ia;

	if ( pFuncs == NULL ) {
		return iColor;
	}
	r = __xgeSvgComponentTransferApplyValue(&pFuncs[0], (float)XGE_COLOR_GET_R(iColor) / 255.0f);
	g = __xgeSvgComponentTransferApplyValue(&pFuncs[1], (float)XGE_COLOR_GET_G(iColor) / 255.0f);
	b = __xgeSvgComponentTransferApplyValue(&pFuncs[2], (float)XGE_COLOR_GET_B(iColor) / 255.0f);
	a = __xgeSvgComponentTransferApplyValue(&pFuncs[3], (float)XGE_COLOR_GET_A(iColor) / 255.0f);
	ir = (int)(r * 255.0f + 0.5f);
	ig = (int)(g * 255.0f + 0.5f);
	ib = (int)(b * 255.0f + 0.5f);
	ia = (int)(a * 255.0f + 0.5f);
	if ( ir < 0 ) ir = 0; else if ( ir > 255 ) ir = 255;
	if ( ig < 0 ) ig = 0; else if ( ig > 255 ) ig = 255;
	if ( ib < 0 ) ib = 0; else if ( ib > 255 ) ib = 255;
	if ( ia < 0 ) ia = 0; else if ( ia > 255 ) ia = 255;
	return XGE_COLOR_RGBA(ir, ig, ib, ia);
}

static int __xgeSvgGaussianBlurTap(int iTap, float fStdX, float fStdY, float* pX, float* pY, float* pWeight)
{
	static const float arrBoth[9][3] = {
		{ 0.0f, 0.0f, 0.28f },
		{ -1.0f, 0.0f, 0.12f }, { 1.0f, 0.0f, 0.12f }, { 0.0f, -1.0f, 0.12f }, { 0.0f, 1.0f, 0.12f },
		{ -0.70710678f, -0.70710678f, 0.06f }, { 0.70710678f, -0.70710678f, 0.06f },
		{ -0.70710678f, 0.70710678f, 0.06f }, { 0.70710678f, 0.70710678f, 0.06f }
	};
	static const float arrOne[5][2] = {
		{ 0.0f, 0.40f }, { -1.0f, 0.24f }, { 1.0f, 0.24f }, { -2.0f, 0.06f }, { 2.0f, 0.06f }
	};
	float fScale;

	if ( (pX == NULL) || (pY == NULL) || (pWeight == NULL) ) {
		return 0;
	}
	*pX = 0.0f;
	*pY = 0.0f;
	*pWeight = 0.0f;
	if ( (fStdX <= 0.0001f) && (fStdY <= 0.0001f) ) {
		return 0;
	}
	fScale = 1.25f;
	if ( (fStdX > 0.0001f) && (fStdY > 0.0001f) ) {
		if ( (iTap < 0) || (iTap >= 9) ) {
			return 0;
		}
		*pX = arrBoth[iTap][0] * fStdX * fScale;
		*pY = arrBoth[iTap][1] * fStdY * fScale;
		*pWeight = arrBoth[iTap][2];
		return 1;
	}
	if ( (iTap < 0) || (iTap >= 5) ) {
		return 0;
	}
	if ( fStdX > 0.0001f ) {
		*pX = arrOne[iTap][0] * fStdX * fScale;
	} else {
		*pY = arrOne[iTap][0] * fStdY * fScale;
	}
	*pWeight = arrOne[iTap][1];
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

static void __xgeSvgApplyShapeStyle(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle);
static int __xgeSvgAddStyledShape(xge_svg pSvg, xge_shape_ex pShape, const xge_svg_style_t* pStyle, int iDefIndex);
static int __xgeSvgApplyUseInstance(xge_svg pSvg, int iDefIndex, const xge_svg_style_t* pUseStyle, int iTargetDef, float fX, float fY, float fW, float fH, int bHasWidth, int bHasHeight);
static int __xgeSvgApplyUseInstanceWithAspect(xge_svg pSvg, int iDefIndex, const xge_svg_style_t* pUseStyle, int iTargetDef, float fX, float fY, float fW, float fH, int bHasWidth, int bHasHeight, const char* sAspect);
static int __xgeSvgAddSvgFragmentImage(xge_svg pSvg, const xge_svg_style_t* pStyle, const char* sHref, xge_rect_t tRect, const char* sAspect, int iDefIndex);
static int __xgeSvgAddHrefImage(xge_svg pSvg, const xge_svg_style_t* pStyle, const char* sHref, xge_rect_t tRect, const char* sAspect, int iDefIndex);

static int __xgeSvgAddFilterFloodOutput(xge_svg pSvg, const xge_svg_style_t* pSourceStyle, xge_rect_t tFilterRegion, uint32_t iFloodColor, float fFloodOpacity, int iDefIndex)
{
	xge_svg_style_t tFloodStyle;
	xge_shape_ex pFlood;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pSourceStyle == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (tFilterRegion.fW <= 0.0f) || (tFilterRegion.fH <= 0.0f) ) {
		return XGE_OK;
	}
	if ( (fFloodOpacity <= 0.0f) || (XGE_COLOR_GET_A(iFloodColor) <= 0) ) {
		return XGE_OK;
	}
	pFlood = NULL;
	iRet = xgeShapeExCreate(&pFlood);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeShapeExAppendRect(pFlood, tFilterRegion.fX, tFilterRegion.fY, tFilterRegion.fW, tFilterRegion.fH, 0.0f, 0.0f, 1);
	if ( iRet != XGE_OK ) {
		xgeShapeExDestroy(pFlood);
		return iRet;
	}
	tFloodStyle = *pSourceStyle;
	tFloodStyle.sFilterId[0] = '\0';
	tFloodStyle.sFillGradientId[0] = '\0';
	tFloodStyle.sStrokeGradientId[0] = '\0';
	tFloodStyle.iFillColor = iFloodColor;
	tFloodStyle.iStrokeColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	tFloodStyle.fFillOpacity = fFloodOpacity;
	tFloodStyle.fStrokeOpacity = 0.0f;
	tFloodStyle.fStrokeWidth = 0.0f;
	tFloodStyle.iDashCount = 0;
	tFloodStyle.fDashOffset = 0.0f;
	tFloodStyle.tTransform = __xgeSvgMatrixIdentity();
	__xgeSvgApplyShapeStyle(pSvg, pFlood, &tFloodStyle);
	return __xgeSvgAddStyledShape(pSvg, pFlood, &tFloodStyle, iDefIndex);
}

static int __xgeSvgAddFilterImageOutput(xge_svg pSvg, const xge_svg_style_t* pSourceStyle, const char* sHref, const char* sAspect, xge_rect_t tFilterRegion, int iDefIndex)
{
	xge_svg_style_t tImageStyle;

	if ( !__xgeSvgValid(pSvg) || (pSourceStyle == NULL) || (sHref == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (sHref[0] == '\0') || (tFilterRegion.fW <= 0.0f) || (tFilterRegion.fH <= 0.0f) ) {
		return XGE_OK;
	}
	tImageStyle = *pSourceStyle;
	tImageStyle.sFilterId[0] = '\0';
	tImageStyle.sFillGradientId[0] = '\0';
	tImageStyle.sStrokeGradientId[0] = '\0';
	tImageStyle.tTransform = __xgeSvgMatrixIdentity();
	return __xgeSvgAddHrefImage(pSvg, &tImageStyle, sHref, tFilterRegion, sAspect, iDefIndex);
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

static int __xgeSvgApplyMorphologyDilatePaintToShape(xge_shape_ex pShape, const xge_svg_style_t* pStyle, int bSourceAlpha, float fRadius)
{
	uint32_t iFill;
	uint32_t iEdge;
	float fStrokeWidth;
	int bFillVisible;
	int bStrokeVisible;

	if ( (pShape == NULL) || (pStyle == NULL) || (fRadius <= 0.0f) ) {
		return 0;
	}
	if ( !bSourceAlpha && ((pStyle->sFillGradientId[0] != '\0') || (pStyle->sStrokeGradientId[0] != '\0')) ) {
		return 0;
	}
	bFillVisible = __xgeSvgStyleHasPaintAlpha(pStyle, 0);
	bStrokeVisible = __xgeSvgStyleHasPaintAlpha(pStyle, 1);
	if ( !bFillVisible && !bStrokeVisible ) {
		return 0;
	}
	if ( bSourceAlpha ) {
		iFill = bFillVisible ? __xgeSvgFilterShadowColor(XGE_COLOR_RGBA(0, 0, 0, 255), 1.0f, pStyle->fOpacity * pStyle->fFillOpacity) : XGE_COLOR_RGBA(0, 0, 0, 0);
		iEdge = (bFillVisible || bStrokeVisible) ? __xgeSvgFilterShadowColor(XGE_COLOR_RGBA(0, 0, 0, 255), 1.0f, pStyle->fOpacity * (bFillVisible ? pStyle->fFillOpacity : pStyle->fStrokeOpacity)) : XGE_COLOR_RGBA(0, 0, 0, 0);
	} else {
		iFill = pShape->iFillColor;
		iEdge = bFillVisible ? pShape->iFillColor : pShape->iStrokeColor;
	}
	fStrokeWidth = bStrokeVisible ? pShape->fStrokeWidth : 0.0f;
	fStrokeWidth += fRadius * 2.0f;
	xgeShapeExFillColor(pShape, iFill);
	xgeShapeExStrokeColor(pShape, iEdge);
	xgeShapeExStrokeWidth(pShape, fStrokeWidth);
	xgeShapeExStrokeDash(pShape, NULL, 0, 0.0f);
	return 1;
}

static int __xgeSvgMatrixNearlyIdentity(xge_shape_ex_matrix_t tMatrix)
{
	return (fabsf(tMatrix.fA - 1.0f) <= 0.0001f) &&
	       (fabsf(tMatrix.fB) <= 0.0001f) &&
	       (fabsf(tMatrix.fC) <= 0.0001f) &&
	       (fabsf(tMatrix.fD - 1.0f) <= 0.0001f) &&
	       (fabsf(tMatrix.fE) <= 0.0001f) &&
	       (fabsf(tMatrix.fF) <= 0.0001f);
}

static int __xgeSvgPointNear(float fA, float fB)
{
	return fabsf(fA - fB) <= 0.0001f;
}

static int __xgeSvgShapeSimpleRectBounds(xge_shape_ex pShape, xge_rect_t* pRect)
{
	float fMinX;
	float fMaxX;
	float fMinY;
	float fMaxY;
	int iMask;
	int i;

	if ( (pShape == NULL) || (pRect == NULL) ||
	     (pShape->iCommandCount != 5) || (pShape->iPointCount != 4) ||
	     (pShape->pCommands[0] != XGE_SHAPE_EX_CMD_MOVE_TO) ||
	     (pShape->pCommands[1] != XGE_SHAPE_EX_CMD_LINE_TO) ||
	     (pShape->pCommands[2] != XGE_SHAPE_EX_CMD_LINE_TO) ||
	     (pShape->pCommands[3] != XGE_SHAPE_EX_CMD_LINE_TO) ||
	     (pShape->pCommands[4] != XGE_SHAPE_EX_CMD_CLOSE) ) {
		return 0;
	}
	fMinX = pShape->pPoints[0].fX;
	fMaxX = pShape->pPoints[0].fX;
	fMinY = pShape->pPoints[0].fY;
	fMaxY = pShape->pPoints[0].fY;
	for ( i = 1; i < 4; i++ ) {
		if ( pShape->pPoints[i].fX < fMinX ) fMinX = pShape->pPoints[i].fX;
		if ( pShape->pPoints[i].fX > fMaxX ) fMaxX = pShape->pPoints[i].fX;
		if ( pShape->pPoints[i].fY < fMinY ) fMinY = pShape->pPoints[i].fY;
		if ( pShape->pPoints[i].fY > fMaxY ) fMaxY = pShape->pPoints[i].fY;
	}
	if ( (fMaxX - fMinX <= 0.0001f) || (fMaxY - fMinY <= 0.0001f) ) {
		return 0;
	}
	iMask = 0;
	for ( i = 0; i < 4; i++ ) {
		int iX;
		int iY;

		if ( __xgeSvgPointNear(pShape->pPoints[i].fX, fMinX) ) {
			iX = 0;
		} else if ( __xgeSvgPointNear(pShape->pPoints[i].fX, fMaxX) ) {
			iX = 1;
		} else {
			return 0;
		}
		if ( __xgeSvgPointNear(pShape->pPoints[i].fY, fMinY) ) {
			iY = 0;
		} else if ( __xgeSvgPointNear(pShape->pPoints[i].fY, fMaxY) ) {
			iY = 1;
		} else {
			return 0;
		}
		iMask |= 1 << ((iY * 2) + iX);
	}
	if ( iMask != 0x0f ) {
		return 0;
	}
	pRect->fX = fMinX;
	pRect->fY = fMinY;
	pRect->fW = fMaxX - fMinX;
	pRect->fH = fMaxY - fMinY;
	return 1;
}

static int __xgeSvgEllipseEndpointMask(xge_vec2_t tPoint, float fCX, float fCY, float fMinX, float fMaxX, float fMinY, float fMaxY)
{
	if ( __xgeSvgPointNear(tPoint.fX, fCX) && __xgeSvgPointNear(tPoint.fY, fMinY) ) {
		return 0x01;
	}
	if ( __xgeSvgPointNear(tPoint.fX, fMaxX) && __xgeSvgPointNear(tPoint.fY, fCY) ) {
		return 0x02;
	}
	if ( __xgeSvgPointNear(tPoint.fX, fCX) && __xgeSvgPointNear(tPoint.fY, fMaxY) ) {
		return 0x04;
	}
	if ( __xgeSvgPointNear(tPoint.fX, fMinX) && __xgeSvgPointNear(tPoint.fY, fCY) ) {
		return 0x08;
	}
	return 0;
}

static int __xgeSvgShapeSimpleEllipseBounds(xge_shape_ex pShape, xge_rect_t* pRect)
{
	float fMinX;
	float fMaxX;
	float fMinY;
	float fMaxY;
	float fCX;
	float fCY;
	int iMask;
	int i;

	if ( (pShape == NULL) || (pRect == NULL) ||
	     (pShape->iCommandCount != 6) || (pShape->iPointCount != 13) ||
	     (pShape->pCommands[0] != XGE_SHAPE_EX_CMD_MOVE_TO) ||
	     (pShape->pCommands[1] != XGE_SHAPE_EX_CMD_CUBIC_TO) ||
	     (pShape->pCommands[2] != XGE_SHAPE_EX_CMD_CUBIC_TO) ||
	     (pShape->pCommands[3] != XGE_SHAPE_EX_CMD_CUBIC_TO) ||
	     (pShape->pCommands[4] != XGE_SHAPE_EX_CMD_CUBIC_TO) ||
	     (pShape->pCommands[5] != XGE_SHAPE_EX_CMD_CLOSE) ||
	     !__xgeSvgPointNear(pShape->pPoints[0].fX, pShape->pPoints[12].fX) ||
	     !__xgeSvgPointNear(pShape->pPoints[0].fY, pShape->pPoints[12].fY) ) {
		return 0;
	}
	fMinX = pShape->pPoints[0].fX;
	fMaxX = pShape->pPoints[0].fX;
	fMinY = pShape->pPoints[0].fY;
	fMaxY = pShape->pPoints[0].fY;
	for ( i = 1; i < 13; i++ ) {
		if ( pShape->pPoints[i].fX < fMinX ) fMinX = pShape->pPoints[i].fX;
		if ( pShape->pPoints[i].fX > fMaxX ) fMaxX = pShape->pPoints[i].fX;
		if ( pShape->pPoints[i].fY < fMinY ) fMinY = pShape->pPoints[i].fY;
		if ( pShape->pPoints[i].fY > fMaxY ) fMaxY = pShape->pPoints[i].fY;
	}
	if ( (fMaxX - fMinX <= 0.0001f) || (fMaxY - fMinY <= 0.0001f) ) {
		return 0;
	}
	fCX = (fMinX + fMaxX) * 0.5f;
	fCY = (fMinY + fMaxY) * 0.5f;
	iMask = __xgeSvgEllipseEndpointMask(pShape->pPoints[0], fCX, fCY, fMinX, fMaxX, fMinY, fMaxY) |
	        __xgeSvgEllipseEndpointMask(pShape->pPoints[3], fCX, fCY, fMinX, fMaxX, fMinY, fMaxY) |
	        __xgeSvgEllipseEndpointMask(pShape->pPoints[6], fCX, fCY, fMinX, fMaxX, fMinY, fMaxY) |
	        __xgeSvgEllipseEndpointMask(pShape->pPoints[9], fCX, fCY, fMinX, fMaxX, fMinY, fMaxY);
	if ( iMask != 0x0f ) {
		return 0;
	}
	pRect->fX = fMinX;
	pRect->fY = fMinY;
	pRect->fW = fMaxX - fMinX;
	pRect->fH = fMaxY - fMinY;
	return 1;
}

static int __xgeSvgAppendErodedEllipseShape(xge_shape_ex pShape, xge_rect_t tBounds, float fRadiusX, float fRadiusY)
{
	xge_vec2_t arrPoints[68];
	float fCX;
	float fCY;
	float fRX;
	float fRY;
	float fRXNorm;
	float fRYNorm;
	float fStart;
	float fEnd;
	float fT;
	int iSegments;
	int iCount;
	int i;

	if ( (pShape == NULL) || (tBounds.fW <= 0.0f) || (tBounds.fH <= 0.0f) || (fRadiusX < 0.0f) || (fRadiusY < 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fCX = tBounds.fX + tBounds.fW * 0.5f;
	fCY = tBounds.fY + tBounds.fH * 0.5f;
	fRX = tBounds.fW * 0.5f;
	fRY = tBounds.fH * 0.5f;
	if ( (fRX <= 0.0001f) || (fRY <= 0.0001f) || (fRadiusX >= fRX) || (fRadiusY >= fRY) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fRXNorm = fRadiusX / fRX;
	fRYNorm = fRadiusY / fRY;
	if ( (fRXNorm * fRXNorm + fRYNorm * fRYNorm) >= 1.0f ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fStart = asinf(fRYNorm);
	fEnd = acosf(fRXNorm);
	if ( fStart >= fEnd ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iSegments = 16;
	iCount = 0;
	for ( i = 0; i <= iSegments; i++ ) {
		fT = fEnd + (fStart - fEnd) * ((float)i / (float)iSegments);
		arrPoints[iCount].fX = fCX + fRX * cosf(fT) - fRadiusX;
		arrPoints[iCount].fY = fCY - (fRY * sinf(fT) - fRadiusY);
		iCount++;
	}
	for ( i = 0; i <= iSegments; i++ ) {
		fT = fStart + (fEnd - fStart) * ((float)i / (float)iSegments);
		arrPoints[iCount].fX = fCX + fRX * cosf(fT) - fRadiusX;
		arrPoints[iCount].fY = fCY + fRY * sinf(fT) - fRadiusY;
		iCount++;
	}
	for ( i = 0; i <= iSegments; i++ ) {
		fT = fEnd + (fStart - fEnd) * ((float)i / (float)iSegments);
		arrPoints[iCount].fX = fCX - (fRX * cosf(fT) - fRadiusX);
		arrPoints[iCount].fY = fCY + fRY * sinf(fT) - fRadiusY;
		iCount++;
	}
	for ( i = 0; i <= iSegments; i++ ) {
		fT = fStart + (fEnd - fStart) * ((float)i / (float)iSegments);
		arrPoints[iCount].fX = fCX - (fRX * cosf(fT) - fRadiusX);
		arrPoints[iCount].fY = fCY - (fRY * sinf(fT) - fRadiusY);
		iCount++;
	}
	return xgeShapeExAppendPolygon(pShape, arrPoints, iCount);
}

static int __xgeSvgApplyMorphologyErodePaintToSimpleShape(xge_shape_ex pShape, const xge_svg_style_t* pStyle, int bSourceAlpha, float fRadiusX, float fRadiusY)
{
	xge_shape_ex_path_snapshot_t tSnapshot = {0};
	xge_rect_t tBounds;
	uint32_t iFill;
	int iShapeType;
	int iRet;

	if ( (pShape == NULL) || (pStyle == NULL) || (fRadiusX < 0.0f) || (fRadiusY < 0.0f) ||
	     ((fRadiusX <= 0.0001f) && (fRadiusY <= 0.0001f)) ) {
		return 0;
	}
	if ( !__xgeSvgMatrixNearlyIdentity(pShape->tTransform) || pShape->bTrimPath ) {
		return 0;
	}
	if ( !__xgeSvgStyleHasPaintAlpha(pStyle, 0) || __xgeSvgStyleHasPaintAlpha(pStyle, 1) ) {
		return 0;
	}
	if ( !bSourceAlpha && ((pStyle->sFillGradientId[0] != '\0') || (pStyle->sStrokeGradientId[0] != '\0')) ) {
		return 0;
	}
	if ( __xgeSvgShapeSimpleRectBounds(pShape, &tBounds) ) {
		iShapeType = 1;
	} else if ( __xgeSvgShapeSimpleEllipseBounds(pShape, &tBounds) ) {
		iShapeType = 2;
	} else {
		return 0;
	}
	__xgeShapeExPathSnapshot(pShape, &tSnapshot);
	if ( xgeShapeExReset(pShape) != XGE_OK ) {
		return 0;
	}
	if ( iShapeType == 1 ) {
		tBounds.fX += fRadiusX;
		tBounds.fY += fRadiusY;
		tBounds.fW -= fRadiusX * 2.0f;
		tBounds.fH -= fRadiusY * 2.0f;
	}
	if ( (tBounds.fW <= 0.0001f) || (tBounds.fH <= 0.0001f) ) {
		xgeShapeExFillColor(pShape, XGE_COLOR_RGBA(0, 0, 0, 0));
		xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(0, 0, 0, 0));
		xgeShapeExStrokeWidth(pShape, 0.0f);
		xgeShapeExStrokeDash(pShape, NULL, 0, 0.0f);
		return 1;
	}
	if ( iShapeType == 1 ) {
		iRet = xgeShapeExAppendRect(pShape, tBounds.fX, tBounds.fY, tBounds.fW, tBounds.fH, 0.0f, 0.0f, 1);
	} else {
		iRet = __xgeSvgAppendErodedEllipseShape(pShape, tBounds, fRadiusX, fRadiusY);
	}
	if ( iRet != XGE_OK ) {
		__xgeShapeExPathRestore(pShape, &tSnapshot);
		return 0;
	}
	iFill = bSourceAlpha ?
		__xgeSvgFilterShadowColor(XGE_COLOR_RGBA(0, 0, 0, 255), 1.0f, pStyle->fOpacity * pStyle->fFillOpacity) :
		pShape->iFillColor;
	xgeShapeExFillColor(pShape, iFill);
	xgeShapeExStrokeColor(pShape, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeShapeExStrokeWidth(pShape, 0.0f);
	xgeShapeExStrokeDash(pShape, NULL, 0, 0.0f);
	return 1;
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

static int __xgeSvgAsciiStartsWithNoCase(const char* sText, const char* sPrefix)
{
	if ( (sText == NULL) || (sPrefix == NULL) ) {
		return 0;
	}
	while ( *sPrefix != '\0' ) {
		if ( __xgeSvgAsciiLower((unsigned char)*sText) != __xgeSvgAsciiLower((unsigned char)*sPrefix) ) {
			return 0;
		}
		sText++;
		sPrefix++;
	}
	return 1;
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
	const char* pEnd;
	double fValue;
	int bPercent;
	int iValue;

	if ( (ppText == NULL) || (*ppText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	p = *ppText;
	__xgeSvgSkipSpaces(&p);
	if ( !__xgeSvgParseNumberAt(p, &pEnd, &fValue) ) {
		return 0;
	}
	p = pEnd;
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

static int __xgeSvgConsumeColorSeparator(const char** ppText, int* pMode)
{
	const char* p;
	int bHadSpace;

	if ( (ppText == NULL) || (*ppText == NULL) || (pMode == NULL) ) {
		return 0;
	}
	p = *ppText;
	bHadSpace = 0;
	while ( __xgeSvgIsSpace(*p) ) {
		bHadSpace = 1;
		p++;
	}
	if ( *p == ',' ) {
		if ( *pMode == XGE_SVG_COLOR_SEPARATOR_SPACE ) {
			return 0;
		}
		*pMode = XGE_SVG_COLOR_SEPARATOR_COMMA;
		p++;
		__xgeSvgSkipSpaces(&p);
		*ppText = p;
		return 1;
	}
	if ( bHadSpace ) {
		if ( *pMode == XGE_SVG_COLOR_SEPARATOR_COMMA ) {
			return 0;
		}
		*pMode = XGE_SVG_COLOR_SEPARATOR_SPACE;
		*ppText = p;
		return 1;
	}
	return 0;
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
	int iMode;

	if ( !__xgeSvgColorFunctionArgs(sText, "rgb", &p) &&
	     !__xgeSvgColorFunctionArgs(sText, "rgba", &p) ) {
		return 0;
	}
	iMode = XGE_SVG_COLOR_SEPARATOR_NONE;
	if ( !__xgeSvgParseColorComponent(&p, 0, &r) ) return 0;
	if ( !__xgeSvgConsumeColorSeparator(&p, &iMode) ) return 0;
	if ( !__xgeSvgParseColorComponent(&p, 0, &g) ) return 0;
	if ( !__xgeSvgConsumeColorSeparator(&p, &iMode) ) return 0;
	if ( !__xgeSvgParseColorComponent(&p, 0, &b) ) return 0;
	a = 255;
	__xgeSvgSkipSpaces(&p);
	if ( *p == '/' ) {
		if ( iMode == XGE_SVG_COLOR_SEPARATOR_COMMA ) {
			return 0;
		}
		p++;
		if ( !__xgeSvgParseColorComponent(&p, 1, &a) ) return 0;
	} else if ( *p == ',' ) {
		if ( iMode == XGE_SVG_COLOR_SEPARATOR_SPACE ) {
			return 0;
		}
		p++;
		if ( !__xgeSvgParseColorComponent(&p, 1, &a) ) return 0;
	} else if ( *p != ')' ) {
		return 0;
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
	const char* pEnd;
	double fValue;

	if ( (ppText == NULL) || (*ppText == NULL) || (pHue == NULL) ) {
		return 0;
	}
	p = *ppText;
	__xgeSvgSkipSpaces(&p);
	if ( !__xgeSvgParseNumberAt(p, &pEnd, &fValue) ) {
		return 0;
	}
	p = pEnd;
	if ( __xgeSvgAsciiStartsWithNoCase(p, "turn") ) {
		fValue *= 360.0;
		p += 4;
	} else if ( __xgeSvgAsciiStartsWithNoCase(p, "rad") ) {
		fValue *= 57.2957795130823208768;
		p += 3;
	} else if ( __xgeSvgAsciiStartsWithNoCase(p, "grad") ) {
		fValue *= 0.9;
		p += 4;
	} else if ( __xgeSvgAsciiStartsWithNoCase(p, "deg") ) {
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
	const char* pEnd;
	double fValue;
	int bPercent;

	if ( (ppText == NULL) || (*ppText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	p = *ppText;
	__xgeSvgSkipSpaces(&p);
	if ( !__xgeSvgParseNumberAt(p, &pEnd, &fValue) ) {
		return 0;
	}
	p = pEnd;
	bPercent = 0;
	if ( *p == '%' ) {
		bPercent = 1;
		p++;
	}
	if ( !bPercent ) {
		return 0;
	}
	fValue /= 100.0;
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
	int iMode;

	if ( !__xgeSvgColorFunctionArgs(sText, "hsl", &p) &&
	     !__xgeSvgColorFunctionArgs(sText, "hsla", &p) ) {
		return 0;
	}
	iMode = XGE_SVG_COLOR_SEPARATOR_NONE;
	if ( !__xgeSvgParseHueComponent(&p, &h) ) return 0;
	if ( !__xgeSvgConsumeColorSeparator(&p, &iMode) ) return 0;
	if ( !__xgeSvgParseHslPercentComponent(&p, &s) ) return 0;
	if ( !__xgeSvgConsumeColorSeparator(&p, &iMode) ) return 0;
	if ( !__xgeSvgParseHslPercentComponent(&p, &l) ) return 0;
	a = 255;
	__xgeSvgSkipSpaces(&p);
	if ( *p == '/' ) {
		if ( iMode == XGE_SVG_COLOR_SEPARATOR_COMMA ) {
			return 0;
		}
		p++;
		if ( !__xgeSvgParseColorComponent(&p, 1, &a) ) return 0;
	} else if ( *p == ',' ) {
		if ( iMode == XGE_SVG_COLOR_SEPARATOR_SPACE ) {
			return 0;
		}
		p++;
		if ( !__xgeSvgParseColorComponent(&p, 1, &a) ) return 0;
	} else if ( *p != ')' ) {
		return 0;
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
	const char* pEnd;
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
		if ( !__xgeSvgParseNumberAt(p, &pEnd, &fValue) ) {
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

	if ( xgeShapeExMatrixTranslate(&tMatrix, fX, fY) != XGE_OK ) {
		return __xgeSvgMatrixIdentity();
	}
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixScale(float fX, float fY)
{
	xge_shape_ex_matrix_t tMatrix;

	if ( xgeShapeExMatrixScale(&tMatrix, fX, fY) != XGE_OK ) {
		return __xgeSvgMatrixIdentity();
	}
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixRotate(float fDegrees)
{
	xge_shape_ex_matrix_t tMatrix;
	float fRadians = fDegrees * (3.14159265358979323846f / 180.0f);

	if ( xgeShapeExMatrixRotate(&tMatrix, fRadians) != XGE_OK ) {
		return __xgeSvgMatrixIdentity();
	}
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixSkewX(float fDegrees)
{
	xge_shape_ex_matrix_t tMatrix;
	float fRadians = fDegrees * (3.14159265358979323846f / 180.0f);

	if ( xgeShapeExMatrixSkew(&tMatrix, fRadians, 0.0f) != XGE_OK ) {
		return __xgeSvgMatrixIdentity();
	}
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixSkewY(float fDegrees)
{
	xge_shape_ex_matrix_t tMatrix;
	float fRadians = fDegrees * (3.14159265358979323846f / 180.0f);

	if ( xgeShapeExMatrixSkew(&tMatrix, 0.0f, fRadians) != XGE_OK ) {
		return __xgeSvgMatrixIdentity();
	}
	return tMatrix;
}

static xge_shape_ex_matrix_t __xgeSvgMatrixMul(xge_shape_ex_matrix_t tParent, xge_shape_ex_matrix_t tLocal)
{
	xge_shape_ex_matrix_t tOut = tParent;

	if ( xgeShapeExMatrixMultiply(&tOut, &tParent, &tLocal) != XGE_OK ) {
		return __xgeSvgMatrixIdentity();
	}
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

static int __xgeSvgClipRectMapToItem(xge_shape_ex_matrix_t tItemTransform, xge_rect_t tClip, int bHasExistingClip, xge_rect_t tExistingClip, xge_rect_t* pOut)
{
	xge_rect_t tLocalClip;

	if ( pOut == NULL ) {
		return 0;
	}
	if ( !__xgeSvgMatrixRectBoundsInverse(tItemTransform, tClip, &tLocalClip) ) {
		return 0;
	}
	if ( bHasExistingClip ) {
		tLocalClip = __xgeSvgRectIntersect(tLocalClip, tExistingClip);
	}
	if ( (tLocalClip.fW <= 0.0f) || (tLocalClip.fH <= 0.0f) ) {
		return 0;
	}
	*pOut = tLocalClip;
	return 1;
}

static int __xgeSvgApplyViewportClipToShape(xge_shape_ex pShape, const xge_svg_style_t* pStyle)
{
	xge_rect_t tClip;

	if ( (pShape == NULL) || (pStyle == NULL) || !pStyle->bViewportClip ) {
		return 1;
	}
	if ( !__xgeSvgClipRectMapToItem(pShape->tTransform, pStyle->tViewportClip, pShape->bClipRect, pShape->tClipRect, &tClip) ) {
		return 0;
	}
	xgeShapeExClipRectSet(pShape, tClip);
	return 1;
}

static int __xgeSvgApplyViewportClipToText(xge_svg_text_item_t* pText)
{
	xge_rect_t tClip;

	if ( (pText == NULL) || !pText->tStyle.bViewportClip ) {
		return 1;
	}
	if ( !__xgeSvgClipRectMapToItem(pText->tStyle.tTransform, pText->tStyle.tViewportClip, pText->bClipRect, pText->tClipRect, &tClip) ) {
		return 0;
	}
	pText->bClipRect = 1;
	pText->tClipRect = tClip;
	pText->tStyle.bViewportClip = 0;
	return 1;
}

static int __xgeSvgApplyViewportClipToImage(xge_svg_image_item_t* pImage)
{
	xge_rect_t tClip;

	if ( (pImage == NULL) || !pImage->tStyle.bViewportClip ) {
		return 1;
	}
	if ( !__xgeSvgClipRectMapToItem(pImage->tStyle.tTransform, pImage->tStyle.tViewportClip, pImage->bClipRect, pImage->tClipRect, &tClip) ) {
		return 0;
	}
	pImage->bClipRect = 1;
	pImage->tClipRect = tClip;
	pImage->tStyle.bViewportClip = 0;
	return 1;
}

static int __xgeSvgApplyViewportClipToRaster(xge_svg_raster_item_t* pImage)
{
	xge_rect_t tClip;

	if ( (pImage == NULL) || !pImage->tStyle.bViewportClip ) {
		return 1;
	}
	if ( !__xgeSvgClipRectMapToItem(pImage->tStyle.tTransform, pImage->tStyle.tViewportClip, pImage->bClipRect, pImage->tClipRect, &tClip) ) {
		return 0;
	}
	pImage->bClipRect = 1;
	pImage->tClipRect = tClip;
	pImage->tStyle.bViewportClip = 0;
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

static int __xgeSvgTransformNameEquals(const char* pText, const char* sName)
{
	if ( (pText == NULL) || (sName == NULL) ) {
		return 0;
	}
	while ( *sName != '\0' ) {
		if ( __xgeSvgCssAsciiLower((unsigned char)*pText) != __xgeSvgCssAsciiLower((unsigned char)*sName) ) {
			return 0;
		}
		pText++;
		sName++;
	}
	return 1;
}

static int __xgeSvgParseTransformAngleUnit(const char** ppText, float* pValue)
{
	const char* p;

	if ( (ppText == NULL) || (*ppText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	p = *ppText;
	if ( (__xgeSvgCssAsciiLower((unsigned char)p[0]) == 'd') &&
	     (__xgeSvgCssAsciiLower((unsigned char)p[1]) == 'e') &&
	     (__xgeSvgCssAsciiLower((unsigned char)p[2]) == 'g') ) {
		*ppText = p + 3;
		return 1;
	}
	if ( (__xgeSvgCssAsciiLower((unsigned char)p[0]) == 'r') &&
	     (__xgeSvgCssAsciiLower((unsigned char)p[1]) == 'a') &&
	     (__xgeSvgCssAsciiLower((unsigned char)p[2]) == 'd') ) {
		*pValue = *pValue * (180.0f / 3.14159265358979323846f);
		*ppText = p + 3;
		return 1;
	}
	if ( (__xgeSvgCssAsciiLower((unsigned char)p[0]) == 'g') &&
	     (__xgeSvgCssAsciiLower((unsigned char)p[1]) == 'r') &&
	     (__xgeSvgCssAsciiLower((unsigned char)p[2]) == 'a') &&
	     (__xgeSvgCssAsciiLower((unsigned char)p[3]) == 'd') ) {
		*pValue = *pValue * 0.9f;
		*ppText = p + 4;
		return 1;
	}
	if ( (__xgeSvgCssAsciiLower((unsigned char)p[0]) == 't') &&
	     (__xgeSvgCssAsciiLower((unsigned char)p[1]) == 'u') &&
	     (__xgeSvgCssAsciiLower((unsigned char)p[2]) == 'r') &&
	     (__xgeSvgCssAsciiLower((unsigned char)p[3]) == 'n') ) {
		*pValue = *pValue * 360.0f;
		*ppText = p + 4;
		return 1;
	}
	return 0;
}

static int __xgeSvgParseAngleDegreesStrict(const char* sText, float* pValue)
{
	const char* p;
	const char* pEnd;
	double fValue;

	if ( (sText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	p = sText;
	while ( __xgeSvgIsSpace((unsigned char)*p) ) p++;
	if ( !__xgeSvgParseNumberAt(p, &pEnd, &fValue) ) {
		return 0;
	}
	if ( !__xgeSvgDoubleToFloat(fValue, pValue) ) {
		return 0;
	}
	p = pEnd;
	while ( __xgeSvgIsSpace((unsigned char)*p) ) p++;
	if ( (*p != '\0') && !__xgeSvgParseTransformAngleUnit(&p, pValue) ) {
		return 0;
	}
	while ( __xgeSvgIsSpace((unsigned char)*p) ) p++;
	return *p == '\0';
}

static int __xgeSvgParseTransformArgs(const char* sText, const char** ppNext, float* pValues, int iCapacity, int* pCount, unsigned int iAngleMask, unsigned int iLengthMask)
{
	const char* p;
	const char* pEnd;
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
		const char* pValueStart;

		__xgeSvgSkipTransformSeparators(&p);
		if ( *p == ')' ) {
			p++;
			break;
		}
		if ( *p == '\0' || iCount >= iCapacity ) {
			return 0;
		}
		pValueStart = p;
		if ( !__xgeSvgParseNumberAt(p, &pEnd, &fValue) ) {
			return 0;
		}
		if ( !__xgeSvgDoubleToFloat(fValue, &pValues[iCount]) ) {
			return 0;
		}
		p = pEnd;
		if ( ((iAngleMask & (1u << iCount)) != 0) && !__xgeSvgIsSpace((unsigned char)*p) && (*p != ',') && (*p != ')') && (*p != '\0') ) {
			const char* pUnit = p;

			if ( __xgeSvgParseTransformAngleUnit(&pUnit, &pValues[iCount]) ) {
				p = pUnit;
			}
		} else if ( ((iLengthMask & (1u << iCount)) != 0) && !__xgeSvgIsSpace((unsigned char)*p) && (*p != ',') && (*p != ')') && (*p != '\0') ) {
			const char* pLength = pValueStart;

			if ( *p == '%' ) {
				return 0;
			}
			if ( !__xgeSvgParseLengthTokenEx(&pLength, 0.0f, 16.0f, &pValues[iCount]) ) {
				return 0;
			}
			p = pLength;
		}
		iCount++;
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
		if ( __xgeSvgTransformNameEquals(p, "matrix") ) {
			xge_shape_ex_matrix_t tM;

			p += 6;
			if ( !__xgeSvgParseTransformArgs(p, &pNext, arrArgs, 6, &iArgCount, 0u, 0u) || (iArgCount != 6) ) return 0;
			tM.fA = arrArgs[0]; tM.fB = arrArgs[1]; tM.fC = arrArgs[2]; tM.fD = arrArgs[3]; tM.fE = arrArgs[4]; tM.fF = arrArgs[5];
			tOut = __xgeSvgMatrixMul(tOut, tM);
			p = pNext;
		} else if ( __xgeSvgTransformNameEquals(p, "translate") ) {
			p += 9;
			if ( !__xgeSvgParseTransformArgs(p, &pNext, arrArgs, 6, &iArgCount, 0u, 3u) || (iArgCount < 1) || (iArgCount > 2) ) return 0;
			tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixTranslate(arrArgs[0], iArgCount == 2 ? arrArgs[1] : 0.0f));
			p = pNext;
		} else if ( __xgeSvgTransformNameEquals(p, "scale") ) {
			p += 5;
			if ( !__xgeSvgParseTransformArgs(p, &pNext, arrArgs, 6, &iArgCount, 0u, 0u) || (iArgCount < 1) || (iArgCount > 2) ) return 0;
			tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixScale(arrArgs[0], iArgCount == 2 ? arrArgs[1] : arrArgs[0]));
			p = pNext;
		} else if ( __xgeSvgTransformNameEquals(p, "rotate") ) {
			p += 6;
			if ( !__xgeSvgParseTransformArgs(p, &pNext, arrArgs, 6, &iArgCount, 1u, 6u) || ((iArgCount != 1) && (iArgCount != 3)) ) return 0;
			if ( iArgCount == 3 ) {
				tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixTranslate(arrArgs[1], arrArgs[2]));
				tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixRotate(arrArgs[0]));
				tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixTranslate(-arrArgs[1], -arrArgs[2]));
			} else {
				tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixRotate(arrArgs[0]));
			}
			p = pNext;
		} else if ( __xgeSvgTransformNameEquals(p, "skewX") ) {
			p += 5;
			if ( !__xgeSvgParseTransformArgs(p, &pNext, arrArgs, 6, &iArgCount, 1u, 0u) || (iArgCount != 1) ) return 0;
			tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixSkewX(arrArgs[0]));
			p = pNext;
		} else if ( __xgeSvgTransformNameEquals(p, "skewY") ) {
			p += 5;
			if ( !__xgeSvgParseTransformArgs(p, &pNext, arrArgs, 6, &iArgCount, 1u, 0u) || (iArgCount != 1) ) return 0;
			tOut = __xgeSvgMatrixMul(tOut, __xgeSvgMatrixSkewY(arrArgs[0]));
			p = pNext;
		} else {
			return 0;
		}
	}
	*pMatrix = tOut;
	return 1;
}

static int __xgeSvgParseGradientSpread(const char* sText, int* pSpread)
{
	if ( (sText == NULL) || (pSpread == NULL) ) {
		return 0;
	}
	if ( strcmp(sText, "reflect") == 0 ) {
		*pSpread = XGE_SHAPE_EX_GRADIENT_SPREAD_REFLECT;
		return 1;
	}
	if ( strcmp(sText, "repeat") == 0 ) {
		*pSpread = XGE_SHAPE_EX_GRADIENT_SPREAD_REPEAT;
		return 1;
	}
	if ( strcmp(sText, "pad") == 0 ) {
		*pSpread = XGE_SHAPE_EX_GRADIENT_SPREAD_PAD;
		return 1;
	}
	return 0;
}

static int __xgeSvgParseGradientUnits(const char* sText, int* pUnits)
{
	return __xgeSvgParseUserObjectUnits(sText, XGE_SHAPE_EX_GRADIENT_USER_SPACE, XGE_SHAPE_EX_GRADIENT_OBJECT_BOUNDING_BOX, pUnits);
}

static int __xgeSvgParsePatternUnits(const char* sText, int* pUnits)
{
	return __xgeSvgParseUserObjectUnits(sText, XGE_SVG_PATTERN_USER_SPACE, XGE_SVG_PATTERN_OBJECT_BOUNDING_BOX, pUnits);
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
	while ( (*sText == ' ') || (*sText == '\t') || (*sText == '\r') || (*sText == '\n') ) sText++;
	if ( __xgeSvgColorNameEquals(sText, "none") ) {
		*pCount = 0;
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
			return 0;
		}
		if ( fValue < 0.0f ) {
			return 0;
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
	if ( (pStart == NULL) || (pEnd == NULL) || (sToken == NULL) || (pEnd < pStart) ) {
		return 0;
	}
	return __xgeSvgCssRangeEqualsText(pStart, pEnd, sToken);
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

static int __xgeSvgParseBlendMode(const char* sText, int* pBlend)
{
	const char* pStart;
	const char* pEnd;

	if ( (sText == NULL) || (pBlend == NULL) ) {
		return 0;
	}
	pStart = sText;
	while ( __xgeSvgIsSpace((unsigned char)*pStart) ) pStart++;
	pEnd = pStart + strlen(pStart);
	while ( (pEnd > pStart) && __xgeSvgIsSpace((unsigned char)pEnd[-1]) ) pEnd--;
	*pBlend = XGE_BLEND_ALPHA;
	if ( pStart == pEnd ) {
		return 0;
	}
	if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "normal") ) {
		return 1;
	}
	if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "multiply") ) {
		*pBlend = XGE_BLEND_MULTIPLY;
		return 1;
	}
	if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "screen") ) {
		*pBlend = XGE_BLEND_SCREEN;
		return 1;
	}
	if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "darken") ) {
		*pBlend = XGE_BLEND_DARKEN;
		return 1;
	}
	if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "lighten") ) {
		*pBlend = XGE_BLEND_LIGHTEN;
		return 1;
	}
	if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "plus-lighter") ||
	     __xgeSvgCssRangeEqualsText(pStart, pEnd, "lighter") ||
	     __xgeSvgCssRangeEqualsText(pStart, pEnd, "add") ) {
		*pBlend = XGE_BLEND_ADD;
		return 1;
	}
	return 0;
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

static int __xgeSvgPaintFallbackValid(const char* sText)
{
	uint32_t iColor;

	if ( sText == NULL ) {
		return 1;
	}
	__xgeSvgSkipSpaces(&sText);
	if ( *sText == '\0' ) {
		return 1;
	}
	return __xgeSvgColorNameEquals(sText, "currentColor") || __xgeSvgParseColor(sText, &iColor);
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

static int __xgeSvgParseBaselineShift(const char* sText, float fFontSize, float* pShift)
{
	float fValue;

	if ( (sText == NULL) || (pShift == NULL) ) {
		return 0;
	}
	if ( fFontSize <= 0.0f ) {
		fFontSize = 16.0f;
	}
	if ( __xgeSvgColorNameEquals(sText, "baseline") ) {
		*pShift = 0.0f;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "sub") ) {
		*pShift = fFontSize * 0.35f;
		return 1;
	}
	if ( __xgeSvgColorNameEquals(sText, "super") ) {
		*pShift = -fFontSize * 0.60f;
		return 1;
	}
	if ( !__xgeSvgParseLengthStrictEx(sText, fFontSize, fFontSize, &fValue) ) {
		return 0;
	}
	*pShift = -fValue;
	return 1;
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
	tStyle.fBaselineShift = 0.0f;
	tStyle.iFillRule = XGE_SHAPE_EX_FILL_NON_ZERO;
	tStyle.iClipRule = XGE_SHAPE_EX_FILL_NON_ZERO;
	tStyle.iLineCap = XGE_SHAPE_EX_CAP_BUTT;
	tStyle.iLineJoin = XGE_SHAPE_EX_JOIN_MITER;
	tStyle.iTextAnchor = XGE_SVG_TEXT_ANCHOR_START;
	tStyle.iTextSpace = XGE_SVG_TEXT_SPACE_DEFAULT;
	tStyle.iTextBaseline = XGE_SVG_TEXT_BASELINE_ALPHABETIC;
	tStyle.iBlend = XGE_BLEND_ALPHA;
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

static int __xgeSvgStylePropertyCopyValidEx(const char* pTag, const char* pTagEnd, const char* sStyle, const char* sName, char* sOut, int iOutCapacity, int bAllowAttrs, int iImportanceMode, xge_svg_style_value_valid_fn pfnValid, void* pUser)
{
	if ( __xgeSvgStyleCopyValidEx(sStyle, sName, sOut, iOutCapacity, iImportanceMode, pfnValid, pUser) ) {
		return 1;
	}
	if ( bAllowAttrs && (pTag != NULL) && (pTagEnd != NULL) ) {
		if ( __xgeSvgAttrCopy(pTag, pTagEnd, sName, sOut, iOutCapacity) ) {
			if ( (pfnValid == NULL) || pfnValid(sOut, pUser) ) {
				return 1;
			}
			if ( (sOut != NULL) && (iOutCapacity > 0) ) {
				sOut[0] = '\0';
			}
		}
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

static int __xgeSvgStyleValueIsInherit(const char* sValue)
{
	return __xgeSvgColorNameEquals(sValue, "inherit");
}

static int __xgeSvgStyleValueEquals(const char* sValue, const char* sKeyword)
{
	return __xgeSvgColorNameEquals(sValue, sKeyword);
}

static int __xgeSvgDisplayValueIsVisible(const char* sValue)
{
	if ( sValue == NULL ) {
		return 0;
	}
	return __xgeSvgStyleValueEquals(sValue, "inline") ||
		__xgeSvgStyleValueEquals(sValue, "block") ||
		__xgeSvgStyleValueEquals(sValue, "list-item") ||
		__xgeSvgStyleValueEquals(sValue, "inline-block") ||
		__xgeSvgStyleValueEquals(sValue, "table") ||
		__xgeSvgStyleValueEquals(sValue, "inline-table") ||
		__xgeSvgStyleValueEquals(sValue, "table-row-group") ||
		__xgeSvgStyleValueEquals(sValue, "table-header-group") ||
		__xgeSvgStyleValueEquals(sValue, "table-footer-group") ||
		__xgeSvgStyleValueEquals(sValue, "table-row") ||
		__xgeSvgStyleValueEquals(sValue, "table-column-group") ||
		__xgeSvgStyleValueEquals(sValue, "table-column") ||
		__xgeSvgStyleValueEquals(sValue, "table-cell") ||
		__xgeSvgStyleValueEquals(sValue, "table-caption") ||
		__xgeSvgStyleValueEquals(sValue, "flex") ||
		__xgeSvgStyleValueEquals(sValue, "inline-flex") ||
		__xgeSvgStyleValueEquals(sValue, "grid") ||
		__xgeSvgStyleValueEquals(sValue, "inline-grid");
}

static int __xgeSvgStyleValueValidDisplay(const char* sValue, void* pUser)
{
	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) || __xgeSvgStyleValueEquals(sValue, "none") || __xgeSvgDisplayValueIsVisible(sValue);
}

static int __xgeSvgStyleValueValidTextAnchor(const char* sValue, void* pUser)
{
	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) ||
		__xgeSvgStyleValueEquals(sValue, "start") ||
		__xgeSvgStyleValueEquals(sValue, "middle") ||
		__xgeSvgStyleValueEquals(sValue, "end");
}

static int __xgeSvgStyleValueValidPaint(const char* sValue, void* pUser)
{
	const char* sFallback;
	uint32_t iColor;
	char sId[XGE_SVG_ID_MAX];

	(void)pUser;
	sFallback = NULL;
	if ( __xgeSvgStyleValueIsInherit(sValue) ||
	     __xgeSvgColorNameEquals(sValue, "currentColor") ||
	     __xgeSvgParseColor(sValue, &iColor) ) {
		return 1;
	}
	if ( !__xgeSvgUrlIdCopyFromValueEx(sValue, sId, sizeof(sId), &sFallback) ) {
		return 0;
	}
	return __xgeSvgPaintFallbackValid(sFallback);
}

static int __xgeSvgStyleValueValidColor(const char* sValue, void* pUser)
{
	uint32_t iColor;

	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) ||
		__xgeSvgColorNameEquals(sValue, "currentColor") ||
		__xgeSvgParseColor(sValue, &iColor);
}

static int __xgeSvgStyleValueValidOpacity(const char* sValue, void* pUser)
{
	float fValue;

	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) || __xgeSvgParseOpacity(sValue, &fValue);
}

static int __xgeSvgStyleValueValidFontSize(const char* sValue, void* pUser)
{
	float fValue;
	float fBaseFontSize;

	fBaseFontSize = pUser != NULL ? *((float*)pUser) : 16.0f;
	return __xgeSvgStyleValueIsInherit(sValue) || __xgeSvgParseFontSize(sValue, fBaseFontSize, &fValue);
}

static int __xgeSvgStyleValueValidLength(const char* sValue, void* pUser)
{
	const xge_svg_style_length_valid_t* pRule;
	float fValue;
	float fPercentRef;
	float fFontSize;

	if ( __xgeSvgStyleValueIsInherit(sValue) ) {
		return 1;
	}
	pRule = (const xge_svg_style_length_valid_t*)pUser;
	fPercentRef = pRule != NULL ? pRule->fPercentRef : 1.0f;
	fFontSize = pRule != NULL ? pRule->fFontSize : 16.0f;
	if ( !__xgeSvgParseLengthStrictEx(sValue, fPercentRef, fFontSize, &fValue) ) {
		return 0;
	}
	return (pRule == NULL) || !pRule->bUseMinValue || (fValue >= pRule->fMinValue);
}

static int __xgeSvgStyleValueValidMiterLimit(const char* sValue, void* pUser)
{
	float fValue;

	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) || (__xgeSvgParseFloatStrict(sValue, &fValue) && (fValue >= 1.0f));
}

static int __xgeSvgStyleValueValidDashArray(const char* sValue, void* pUser)
{
	const xge_svg_style_length_valid_t* pRule;
	float arrPattern[XGE_SVG_DASH_MAX];
	int iCount;

	if ( __xgeSvgStyleValueIsInherit(sValue) ) {
		return 1;
	}
	pRule = (const xge_svg_style_length_valid_t*)pUser;
	return __xgeSvgParseDashArrayEx(sValue, pRule != NULL ? pRule->fPercentRef : 1.0f, pRule != NULL ? pRule->fFontSize : 16.0f, arrPattern, &iCount);
}

static int __xgeSvgStyleValueValidVisibility(const char* sValue, void* pUser)
{
	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) ||
		__xgeSvgStyleValueEquals(sValue, "visible") ||
		__xgeSvgStyleValueEquals(sValue, "hidden") ||
		__xgeSvgStyleValueEquals(sValue, "collapse");
}

static int __xgeSvgStyleValueValidBlendMode(const char* sValue, void* pUser)
{
	int iBlend;

	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) || __xgeSvgParseBlendMode(sValue, &iBlend);
}

static int __xgeSvgStyleValueValidTextBaseline(const char* sValue, void* pUser)
{
	int iBaseline;

	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) || __xgeSvgParseTextBaseline(sValue, &iBaseline);
}

static int __xgeSvgStyleValueValidBaselineShift(const char* sValue, void* pUser)
{
	float fValue;
	float fFontSize;

	fFontSize = pUser != NULL ? *((float*)pUser) : 16.0f;
	return __xgeSvgStyleValueIsInherit(sValue) || __xgeSvgParseBaselineShift(sValue, fFontSize, &fValue);
}

static int __xgeSvgStyleValueValidTextDecoration(const char* sValue, void* pUser)
{
	int iDecoration;

	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) || __xgeSvgParseTextDecoration(sValue, &iDecoration);
}

static int __xgeSvgStyleValueValidTextSpacing(const char* sValue, void* pUser)
{
	const xge_svg_style_length_valid_t* pRule;
	float fValue;

	if ( __xgeSvgStyleValueIsInherit(sValue) || __xgeSvgColorNameEquals(sValue, "normal") ) {
		return 1;
	}
	pRule = (const xge_svg_style_length_valid_t*)pUser;
	return __xgeSvgParseLengthStrictEx(sValue, pRule != NULL ? pRule->fPercentRef : 1.0f, pRule != NULL ? pRule->fFontSize : 16.0f, &fValue);
}

static int __xgeSvgStyleValueValidXmlSpace(const char* sValue, void* pUser)
{
	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) ||
		__xgeSvgStyleValueEquals(sValue, "preserve") ||
		__xgeSvgStyleValueEquals(sValue, "default");
}

static int __xgeSvgStyleValueValidWhiteSpace(const char* sValue, void* pUser)
{
	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) ||
		__xgeSvgStyleValueEquals(sValue, "pre") ||
		__xgeSvgStyleValueEquals(sValue, "pre-wrap") ||
		__xgeSvgStyleValueEquals(sValue, "break-spaces") ||
		__xgeSvgStyleValueEquals(sValue, "normal") ||
		__xgeSvgStyleValueEquals(sValue, "nowrap");
}

static int __xgeSvgStyleValueValidUrlNone(const char* sValue, void* pUser)
{
	char sId[XGE_SVG_ID_MAX];

	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) ||
		__xgeSvgStyleValueEquals(sValue, "none") ||
		__xgeSvgUrlIdCopyFromValue(sValue, sId, sizeof(sId));
}

static int __xgeSvgStyleValueValidLineCap(const char* sValue, void* pUser)
{
	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) ||
		__xgeSvgStyleValueEquals(sValue, "butt") ||
		__xgeSvgStyleValueEquals(sValue, "round") ||
		__xgeSvgStyleValueEquals(sValue, "square");
}

static int __xgeSvgStyleValueValidLineJoin(const char* sValue, void* pUser)
{
	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) ||
		__xgeSvgStyleValueEquals(sValue, "miter") ||
		__xgeSvgStyleValueEquals(sValue, "round") ||
		__xgeSvgStyleValueEquals(sValue, "bevel");
}

static int __xgeSvgStyleValueValidFillRule(const char* sValue, void* pUser)
{
	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) ||
		__xgeSvgStyleValueEquals(sValue, "evenodd") ||
		__xgeSvgStyleValueEquals(sValue, "nonzero");
}

static int __xgeSvgStyleValueValidPaintOrder(const char* sValue, void* pUser)
{
	int arrPaintOrder[XGE_SVG_PAINT_COMPONENT_COUNT];

	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) || __xgeSvgParsePaintOrder(sValue, arrPaintOrder);
}

static int __xgeSvgStyleValueValidVectorEffect(const char* sValue, void* pUser)
{
	(void)pUser;
	return __xgeSvgStyleValueIsInherit(sValue) ||
		__xgeSvgStyleValueEquals(sValue, "none") ||
		__xgeSvgStyleValueEquals(sValue, "non-scaling-stroke");
}

static void __xgeSvgStyleApplyProperties(xge_svg pSvg, xge_svg_style_t* pStyle, const xge_svg_style_t* pInheritedStyle, const char* pTag, const char* pTagEnd, const char* sStyle, int bAllowAttrs, int bAllowTransform, int iImportanceMode, xge_shape_ex_matrix_t tBaseTransform, float fBaseOpacity, int bBaseVisible, float fBaseFontSize, int iApplyMode)
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
		if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "font-size", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidFontSize, &fBaseFontSize) ) {
			if ( __xgeSvgStyleValueIsInherit(sValue) ) {
				pStyle->fFontSize = pInheritedStyle != NULL ? pInheritedStyle->fFontSize : 16.0f;
			} else if ( __xgeSvgParseFontSize(sValue, fBaseFontSize, &fValue) ) {
				pStyle->fFontSize = fValue;
			}
		}
		if ( iApplyMode == XGE_SVG_STYLE_APPLY_FONT_SIZE ) {
			return;
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "color", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidColor, NULL) &&
	     !__xgeSvgColorNameEquals(sValue, "currentColor") ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			pStyle->iCurrentColor = pInheritedStyle != NULL ? pInheritedStyle->iCurrentColor : XGE_COLOR_RGBA(0, 0, 0, 255);
			pStyle->bColorSet = pInheritedStyle != NULL ? pInheritedStyle->bColorSet : 0;
		} else if ( __xgeSvgParseColor(sValue, &pStyle->iCurrentColor) ) {
			pStyle->bColorSet = 1;
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "display", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidDisplay, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			pStyle->bVisible = pInheritedStyle != NULL ? pInheritedStyle->bVisible : 1;
		} else if ( __xgeSvgStyleValueEquals(sValue, "none") ) {
			pStyle->bVisible = 0;
		} else if ( __xgeSvgDisplayValueIsVisible(sValue) ) {
			pStyle->bVisible = bBaseVisible;
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "visibility", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidVisibility, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			pStyle->bVisibility = pInheritedStyle != NULL ? pInheritedStyle->bVisibility : 1;
		} else if ( __xgeSvgStyleValueEquals(sValue, "visible") ) {
			pStyle->bVisibility = 1;
		} else if ( __xgeSvgStyleValueEquals(sValue, "hidden") || __xgeSvgStyleValueEquals(sValue, "collapse") ) {
			pStyle->bVisibility = 0;
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "mix-blend-mode", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidBlendMode, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			pStyle->iBlend = pInheritedStyle != NULL ? pInheritedStyle->iBlend : XGE_BLEND_ALPHA;
			pStyle->bBlendSet = pInheritedStyle != NULL ? pInheritedStyle->bBlendSet : 0;
		} else if ( __xgeSvgParseBlendMode(sValue, &pStyle->iBlend) ) {
			pStyle->bBlendSet = 1;
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "text-anchor", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidTextAnchor, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->iTextAnchor = pInheritedStyle != NULL ? pInheritedStyle->iTextAnchor : XGE_SVG_TEXT_ANCHOR_START;
		else if ( __xgeSvgStyleValueEquals(sValue, "start") ) pStyle->iTextAnchor = XGE_SVG_TEXT_ANCHOR_START;
		else if ( __xgeSvgStyleValueEquals(sValue, "middle") ) pStyle->iTextAnchor = XGE_SVG_TEXT_ANCHOR_MIDDLE;
		else if ( __xgeSvgStyleValueEquals(sValue, "end") ) pStyle->iTextAnchor = XGE_SVG_TEXT_ANCHOR_END;
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "dominant-baseline", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidTextBaseline, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->iTextBaseline = pInheritedStyle != NULL ? pInheritedStyle->iTextBaseline : XGE_SVG_TEXT_BASELINE_ALPHABETIC;
		else if ( __xgeSvgParseTextBaseline(sValue, &iBaseline) ) pStyle->iTextBaseline = iBaseline;
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "alignment-baseline", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidTextBaseline, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->iTextBaseline = pInheritedStyle != NULL ? pInheritedStyle->iTextBaseline : XGE_SVG_TEXT_BASELINE_ALPHABETIC;
		else if ( __xgeSvgParseTextBaseline(sValue, &iBaseline) ) pStyle->iTextBaseline = iBaseline;
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "baseline-shift", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidBaselineShift, &pStyle->fFontSize) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			pStyle->fBaselineShift = pInheritedStyle != NULL ? pInheritedStyle->fBaselineShift : 0.0f;
		} else if ( __xgeSvgParseBaselineShift(sValue, pStyle->fFontSize, &fValue) ) {
			pStyle->fBaselineShift = fValue;
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "text-decoration-line", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidTextDecoration, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->iTextDecoration = pInheritedStyle != NULL ? pInheritedStyle->iTextDecoration : 0;
		else if ( __xgeSvgParseTextDecoration(sValue, &iDecoration) ) pStyle->iTextDecoration = iDecoration;
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "text-decoration", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidTextDecoration, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->iTextDecoration = pInheritedStyle != NULL ? pInheritedStyle->iTextDecoration : 0;
		else if ( __xgeSvgParseTextDecoration(sValue, &iDecoration) ) pStyle->iTextDecoration = iDecoration;
	}
	{
		xge_svg_style_length_valid_t tTextSpacingRule;
		tTextSpacingRule.fPercentRef = fLengthRef;
		tTextSpacingRule.fFontSize = pStyle->fFontSize;
		tTextSpacingRule.fMinValue = 0.0f;
		tTextSpacingRule.bUseMinValue = 0;
		if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "letter-spacing", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidTextSpacing, &tTextSpacingRule) ) {
			if ( __xgeSvgStyleValueIsInherit(sValue) ) {
				pStyle->fLetterSpacing = pInheritedStyle != NULL ? pInheritedStyle->fLetterSpacing : 0.0f;
			} else if ( __xgeSvgColorNameEquals(sValue, "normal") ) {
				pStyle->fLetterSpacing = 0.0f;
			} else if ( __xgeSvgParseLengthStrictEx(sValue, fLengthRef, pStyle->fFontSize, &fValue) ) {
				pStyle->fLetterSpacing = fValue;
			}
		}
	}
	{
		xge_svg_style_length_valid_t tTextSpacingRule;
		tTextSpacingRule.fPercentRef = fLengthRef;
		tTextSpacingRule.fFontSize = pStyle->fFontSize;
		tTextSpacingRule.fMinValue = 0.0f;
		tTextSpacingRule.bUseMinValue = 0;
		if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "word-spacing", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidTextSpacing, &tTextSpacingRule) ) {
			if ( __xgeSvgStyleValueIsInherit(sValue) ) {
				pStyle->fWordSpacing = pInheritedStyle != NULL ? pInheritedStyle->fWordSpacing : 0.0f;
			} else if ( __xgeSvgColorNameEquals(sValue, "normal") ) {
				pStyle->fWordSpacing = 0.0f;
			} else if ( __xgeSvgParseLengthStrictEx(sValue, fLengthRef, pStyle->fFontSize, &fValue) ) {
				pStyle->fWordSpacing = fValue;
			}
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "xml:space", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidXmlSpace, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->iTextSpace = pInheritedStyle != NULL ? pInheritedStyle->iTextSpace : XGE_SVG_TEXT_SPACE_DEFAULT;
		else if ( __xgeSvgStyleValueEquals(sValue, "preserve") ) pStyle->iTextSpace = XGE_SVG_TEXT_SPACE_PRESERVE;
		else if ( __xgeSvgStyleValueEquals(sValue, "default") ) pStyle->iTextSpace = XGE_SVG_TEXT_SPACE_DEFAULT;
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "white-space", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidWhiteSpace, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			pStyle->iTextSpace = pInheritedStyle != NULL ? pInheritedStyle->iTextSpace : XGE_SVG_TEXT_SPACE_DEFAULT;
		} else if ( __xgeSvgStyleValueEquals(sValue, "pre") || __xgeSvgStyleValueEquals(sValue, "pre-wrap") || __xgeSvgStyleValueEquals(sValue, "break-spaces") ) {
			pStyle->iTextSpace = XGE_SVG_TEXT_SPACE_PRESERVE;
		} else if ( __xgeSvgStyleValueEquals(sValue, "normal") || __xgeSvgStyleValueEquals(sValue, "nowrap") ) {
			pStyle->iTextSpace = XGE_SVG_TEXT_SPACE_DEFAULT;
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "stop-color", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidColor, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			pStyle->iStopColor = pInheritedStyle != NULL ? pInheritedStyle->iStopColor : XGE_COLOR_RGBA(0, 0, 0, 255);
			pStyle->bStopColorSet = pInheritedStyle != NULL ? pInheritedStyle->bStopColorSet : 0;
		} else if ( __xgeSvgColorNameEquals(sValue, "currentColor") ) {
			pStyle->iStopColor = pStyle->iCurrentColor;
			pStyle->bStopColorSet = 1;
		} else if ( __xgeSvgParseColor(sValue, &pStyle->iStopColor) ) {
			pStyle->bStopColorSet = 1;
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "stop-opacity", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidOpacity, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			pStyle->fStopOpacity = pInheritedStyle != NULL ? pInheritedStyle->fStopOpacity : 1.0f;
			pStyle->bStopOpacitySet = pInheritedStyle != NULL ? pInheritedStyle->bStopOpacitySet : 0;
		} else if ( __xgeSvgParseOpacity(sValue, &fValue) ) {
			pStyle->fStopOpacity = fValue;
			pStyle->bStopOpacitySet = 1;
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "fill", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidPaint, NULL) ) {
		const char* sFallback = NULL;
		uint32_t iColor;
		char sPaintId[XGE_SVG_ID_MAX];
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			pStyle->iFillColor = pInheritedStyle != NULL ? pInheritedStyle->iFillColor : XGE_COLOR_RGBA(0, 0, 0, 255);
			pStyle->bFillSet = pInheritedStyle != NULL ? pInheritedStyle->bFillSet : 0;
			pStyle->bFillCurrentColor = pInheritedStyle != NULL ? pInheritedStyle->bFillCurrentColor : 0;
			if ( pInheritedStyle != NULL ) strcpy(pStyle->sFillGradientId, pInheritedStyle->sFillGradientId);
			else pStyle->sFillGradientId[0] = '\0';
		} else if ( __xgeSvgUrlIdCopyFromValueEx(sValue, sPaintId, sizeof(sPaintId), &sFallback) ) {
			pStyle->bFillSet = 1;
			pStyle->bFillCurrentColor = 0;
			strcpy(pStyle->sFillGradientId, sPaintId);
			pStyle->iFillColor = XGE_COLOR_RGBA(0, 0, 0, 255);
			__xgeSvgParsePaintFallbackColor(sFallback, pStyle->iCurrentColor, &pStyle->iFillColor);
			pStyle->bFillCurrentColor = (sFallback != NULL) && __xgeSvgColorNameEquals(sFallback, "currentColor");
		} else if ( __xgeSvgColorNameEquals(sValue, "currentColor") ) {
			pStyle->bFillSet = 1;
			pStyle->sFillGradientId[0] = '\0';
			pStyle->iFillColor = pStyle->iCurrentColor;
			pStyle->bFillCurrentColor = 1;
		} else if ( __xgeSvgParseColor(sValue, &iColor) ) {
			pStyle->bFillSet = 1;
			pStyle->bFillCurrentColor = 0;
			pStyle->sFillGradientId[0] = '\0';
			pStyle->iFillColor = iColor;
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "stroke", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidPaint, NULL) ) {
		const char* sFallback = NULL;
		uint32_t iColor;
		char sPaintId[XGE_SVG_ID_MAX];
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			pStyle->iStrokeColor = pInheritedStyle != NULL ? pInheritedStyle->iStrokeColor : XGE_COLOR_RGBA(0, 0, 0, 0);
			pStyle->bStrokeSet = pInheritedStyle != NULL ? pInheritedStyle->bStrokeSet : 0;
			pStyle->bStrokeCurrentColor = pInheritedStyle != NULL ? pInheritedStyle->bStrokeCurrentColor : 0;
			if ( pInheritedStyle != NULL ) strcpy(pStyle->sStrokeGradientId, pInheritedStyle->sStrokeGradientId);
			else pStyle->sStrokeGradientId[0] = '\0';
		} else if ( __xgeSvgUrlIdCopyFromValueEx(sValue, sPaintId, sizeof(sPaintId), &sFallback) ) {
			pStyle->bStrokeSet = 1;
			pStyle->bStrokeCurrentColor = 0;
			strcpy(pStyle->sStrokeGradientId, sPaintId);
			pStyle->iStrokeColor = XGE_COLOR_RGBA(0, 0, 0, 255);
			__xgeSvgParsePaintFallbackColor(sFallback, pStyle->iCurrentColor, &pStyle->iStrokeColor);
			pStyle->bStrokeCurrentColor = (sFallback != NULL) && __xgeSvgColorNameEquals(sFallback, "currentColor");
		} else if ( __xgeSvgColorNameEquals(sValue, "currentColor") ) {
			pStyle->bStrokeSet = 1;
			pStyle->sStrokeGradientId[0] = '\0';
			pStyle->iStrokeColor = pStyle->iCurrentColor;
			pStyle->bStrokeCurrentColor = 1;
		} else if ( __xgeSvgParseColor(sValue, &iColor) ) {
			pStyle->bStrokeSet = 1;
			pStyle->bStrokeCurrentColor = 0;
			pStyle->sStrokeGradientId[0] = '\0';
			pStyle->iStrokeColor = iColor;
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "opacity", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidOpacity, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->fOpacity = pInheritedStyle != NULL ? pInheritedStyle->fOpacity : 1.0f;
		else if ( __xgeSvgParseOpacity(sValue, &fValue) ) pStyle->fOpacity = fBaseOpacity * fValue;
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "fill-opacity", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidOpacity, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->fFillOpacity = pInheritedStyle != NULL ? pInheritedStyle->fFillOpacity : 1.0f;
		else if ( __xgeSvgParseOpacity(sValue, &fValue) ) pStyle->fFillOpacity = fValue;
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "stroke-opacity", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidOpacity, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->fStrokeOpacity = pInheritedStyle != NULL ? pInheritedStyle->fStrokeOpacity : 1.0f;
		else if ( __xgeSvgParseOpacity(sValue, &fValue) ) pStyle->fStrokeOpacity = fValue;
	}
	{
		xge_svg_style_length_valid_t tStrokeWidthRule;
		tStrokeWidthRule.fPercentRef = fLengthRef;
		tStrokeWidthRule.fFontSize = pStyle->fFontSize;
		tStrokeWidthRule.fMinValue = 0.0f;
		tStrokeWidthRule.bUseMinValue = 1;
		if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "stroke-width", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidLength, &tStrokeWidthRule) ) {
			if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->fStrokeWidth = pInheritedStyle != NULL ? pInheritedStyle->fStrokeWidth : 1.0f;
			else if ( __xgeSvgParseLengthStrictEx(sValue, fLengthRef, pStyle->fFontSize, &fValue) ) pStyle->fStrokeWidth = fValue;
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "stroke-miterlimit", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidMiterLimit, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			pStyle->fMiterLimit = pInheritedStyle != NULL ? pInheritedStyle->fMiterLimit : 4.0f;
		} else if ( __xgeSvgParseFloatStrict(sValue, &fValue) ) {
			pStyle->fMiterLimit = fValue;
		}
	}
	{
		xge_svg_style_length_valid_t tDashArrayRule;
		tDashArrayRule.fPercentRef = fLengthRef;
		tDashArrayRule.fFontSize = pStyle->fFontSize;
		tDashArrayRule.fMinValue = 0.0f;
		tDashArrayRule.bUseMinValue = 0;
		if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "stroke-dasharray", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidDashArray, &tDashArrayRule) ) {
			if ( __xgeSvgStyleValueIsInherit(sValue) ) {
				if ( pInheritedStyle != NULL ) {
					memcpy(pStyle->fDashPattern, pInheritedStyle->fDashPattern, sizeof(pStyle->fDashPattern));
					pStyle->iDashCount = pInheritedStyle->iDashCount;
				}
			} else {
				__xgeSvgParseDashArrayEx(sValue, fLengthRef, pStyle->fFontSize, pStyle->fDashPattern, &pStyle->iDashCount);
			}
		}
	}
	{
		float fDashOffsetRef;
		xge_svg_style_length_valid_t tDashOffsetRule;
		fDashOffsetRef = __xgeSvgLengthPercentRef(pSvg, XGE_SVG_LENGTH_BASIS_OTHER);
		tDashOffsetRule.fPercentRef = fDashOffsetRef;
		tDashOffsetRule.fFontSize = pStyle->fFontSize;
		tDashOffsetRule.fMinValue = 0.0f;
		tDashOffsetRule.bUseMinValue = 0;
		if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "stroke-dashoffset", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidLength, &tDashOffsetRule) ) {
			if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->fDashOffset = pInheritedStyle != NULL ? pInheritedStyle->fDashOffset : 0.0f;
			else if ( __xgeSvgParseLengthStrictEx(sValue, fDashOffsetRef, pStyle->fFontSize, &fValue) ) pStyle->fDashOffset = fValue;
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "clip-path", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidUrlNone, NULL) ) {
		char sClipId[XGE_SVG_ID_MAX];
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			if ( pInheritedStyle != NULL ) strcpy(pStyle->sClipId, pInheritedStyle->sClipId);
			else pStyle->sClipId[0] = '\0';
		} else if ( __xgeSvgStyleValueEquals(sValue, "none") ) {
			pStyle->sClipId[0] = '\0';
		} else if ( __xgeSvgUrlIdCopyFromValue(sValue, sClipId, sizeof(sClipId)) ) {
			strcpy(pStyle->sClipId, sClipId);
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "mask", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidUrlNone, NULL) ) {
		char sMaskId[XGE_SVG_ID_MAX];
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			if ( pInheritedStyle != NULL ) strcpy(pStyle->sMaskId, pInheritedStyle->sMaskId);
			else pStyle->sMaskId[0] = '\0';
		} else if ( __xgeSvgStyleValueEquals(sValue, "none") ) {
			pStyle->sMaskId[0] = '\0';
		} else if ( __xgeSvgUrlIdCopyFromValue(sValue, sMaskId, sizeof(sMaskId)) ) {
			strcpy(pStyle->sMaskId, sMaskId);
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "filter", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidUrlNone, NULL) ) {
		char sFilterId[XGE_SVG_ID_MAX];
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			if ( pInheritedStyle != NULL ) strcpy(pStyle->sFilterId, pInheritedStyle->sFilterId);
			else pStyle->sFilterId[0] = '\0';
		} else if ( __xgeSvgStyleValueEquals(sValue, "none") ) {
			pStyle->sFilterId[0] = '\0';
		} else if ( __xgeSvgUrlIdCopyFromValue(sValue, sFilterId, sizeof(sFilterId)) ) {
			strcpy(pStyle->sFilterId, sFilterId);
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "marker", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidUrlNone, NULL) ) {
		char sMarkerId[XGE_SVG_ID_MAX];
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			if ( pInheritedStyle != NULL ) {
				strcpy(pStyle->sMarkerStartId, pInheritedStyle->sMarkerStartId);
				strcpy(pStyle->sMarkerMidId, pInheritedStyle->sMarkerMidId);
				strcpy(pStyle->sMarkerEndId, pInheritedStyle->sMarkerEndId);
			} else {
				pStyle->sMarkerStartId[0] = '\0';
				pStyle->sMarkerMidId[0] = '\0';
				pStyle->sMarkerEndId[0] = '\0';
			}
		} else if ( __xgeSvgStyleValueEquals(sValue, "none") ) {
			pStyle->sMarkerStartId[0] = '\0';
			pStyle->sMarkerMidId[0] = '\0';
			pStyle->sMarkerEndId[0] = '\0';
		} else if ( __xgeSvgUrlIdCopyFromValue(sValue, sMarkerId, sizeof(sMarkerId)) ) {
			strcpy(pStyle->sMarkerStartId, sMarkerId);
			strcpy(pStyle->sMarkerMidId, sMarkerId);
			strcpy(pStyle->sMarkerEndId, sMarkerId);
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "marker-start", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidUrlNone, NULL) ) {
		char sMarkerId[XGE_SVG_ID_MAX];
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			if ( pInheritedStyle != NULL ) strcpy(pStyle->sMarkerStartId, pInheritedStyle->sMarkerStartId);
			else pStyle->sMarkerStartId[0] = '\0';
		} else if ( __xgeSvgStyleValueEquals(sValue, "none") ) pStyle->sMarkerStartId[0] = '\0';
		else if ( __xgeSvgUrlIdCopyFromValue(sValue, sMarkerId, sizeof(sMarkerId)) ) strcpy(pStyle->sMarkerStartId, sMarkerId);
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "marker-mid", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidUrlNone, NULL) ) {
		char sMarkerId[XGE_SVG_ID_MAX];
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			if ( pInheritedStyle != NULL ) strcpy(pStyle->sMarkerMidId, pInheritedStyle->sMarkerMidId);
			else pStyle->sMarkerMidId[0] = '\0';
		} else if ( __xgeSvgStyleValueEquals(sValue, "none") ) pStyle->sMarkerMidId[0] = '\0';
		else if ( __xgeSvgUrlIdCopyFromValue(sValue, sMarkerId, sizeof(sMarkerId)) ) strcpy(pStyle->sMarkerMidId, sMarkerId);
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "marker-end", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidUrlNone, NULL) ) {
		char sMarkerId[XGE_SVG_ID_MAX];
		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			if ( pInheritedStyle != NULL ) strcpy(pStyle->sMarkerEndId, pInheritedStyle->sMarkerEndId);
			else pStyle->sMarkerEndId[0] = '\0';
		} else if ( __xgeSvgStyleValueEquals(sValue, "none") ) pStyle->sMarkerEndId[0] = '\0';
		else if ( __xgeSvgUrlIdCopyFromValue(sValue, sMarkerId, sizeof(sMarkerId)) ) strcpy(pStyle->sMarkerEndId, sMarkerId);
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "fill-rule", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidFillRule, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->iFillRule = pInheritedStyle != NULL ? pInheritedStyle->iFillRule : XGE_SHAPE_EX_FILL_NON_ZERO;
		else if ( __xgeSvgStyleValueEquals(sValue, "evenodd") ) pStyle->iFillRule = XGE_SHAPE_EX_FILL_EVEN_ODD;
		else if ( __xgeSvgStyleValueEquals(sValue, "nonzero") ) pStyle->iFillRule = XGE_SHAPE_EX_FILL_NON_ZERO;
		pStyle->bFillRuleSet = 1;
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "clip-rule", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidFillRule, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->iClipRule = pInheritedStyle != NULL ? pInheritedStyle->iClipRule : XGE_SHAPE_EX_FILL_NON_ZERO;
		else if ( __xgeSvgStyleValueEquals(sValue, "evenodd") ) pStyle->iClipRule = XGE_SHAPE_EX_FILL_EVEN_ODD;
		else if ( __xgeSvgStyleValueEquals(sValue, "nonzero") ) pStyle->iClipRule = XGE_SHAPE_EX_FILL_NON_ZERO;
		pStyle->bClipRuleSet = 1;
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "paint-order", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidPaintOrder, NULL) ) {
		int arrPaintOrder[XGE_SVG_PAINT_COMPONENT_COUNT];

		if ( __xgeSvgStyleValueIsInherit(sValue) ) {
			if ( pInheritedStyle != NULL ) {
				memcpy(pStyle->iPaintOrder, pInheritedStyle->iPaintOrder, sizeof(pStyle->iPaintOrder));
				__xgeSvgStyleUpdatePaintOrderDerived(pStyle);
			}
		} else {
			if ( __xgeSvgParsePaintOrder(sValue, arrPaintOrder) ) {
				memcpy(pStyle->iPaintOrder, arrPaintOrder, sizeof(arrPaintOrder));
				__xgeSvgStyleUpdatePaintOrderDerived(pStyle);
			}
		}
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "stroke-linecap", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidLineCap, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->iLineCap = pInheritedStyle != NULL ? pInheritedStyle->iLineCap : XGE_SHAPE_EX_CAP_BUTT;
		else if ( __xgeSvgStyleValueEquals(sValue, "butt") ) pStyle->iLineCap = XGE_SHAPE_EX_CAP_BUTT;
		else if ( __xgeSvgStyleValueEquals(sValue, "round") ) pStyle->iLineCap = XGE_SHAPE_EX_CAP_ROUND;
		else if ( __xgeSvgStyleValueEquals(sValue, "square") ) pStyle->iLineCap = XGE_SHAPE_EX_CAP_SQUARE;
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "stroke-linejoin", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidLineJoin, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->iLineJoin = pInheritedStyle != NULL ? pInheritedStyle->iLineJoin : XGE_SHAPE_EX_JOIN_MITER;
		else if ( __xgeSvgStyleValueEquals(sValue, "miter") ) pStyle->iLineJoin = XGE_SHAPE_EX_JOIN_MITER;
		else if ( __xgeSvgStyleValueEquals(sValue, "round") ) pStyle->iLineJoin = XGE_SHAPE_EX_JOIN_ROUND;
		else if ( __xgeSvgStyleValueEquals(sValue, "bevel") ) pStyle->iLineJoin = XGE_SHAPE_EX_JOIN_BEVEL;
	}
	if ( __xgeSvgStylePropertyCopyValidEx(pTag, pTagEnd, sStyle, "vector-effect", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode, __xgeSvgStyleValueValidVectorEffect, NULL) ) {
		if ( __xgeSvgStyleValueIsInherit(sValue) ) pStyle->bStrokeNonScaling = pInheritedStyle != NULL ? pInheritedStyle->bStrokeNonScaling : 0;
		else if ( __xgeSvgStyleValueEquals(sValue, "none") ) pStyle->bStrokeNonScaling = 0;
		else if ( __xgeSvgStyleValueEquals(sValue, "non-scaling-stroke") ) pStyle->bStrokeNonScaling = 1;
	}
	if ( bAllowTransform && __xgeSvgStylePropertyCopyEx(pTag, pTagEnd, sStyle, "transform", sValue, sizeof(sValue), bAllowAttrs, iImportanceMode) ) {
		xge_shape_ex_matrix_t tTransform;
		if ( __xgeSvgParseTransform(sValue, &tTransform) ) {
			pStyle->tTransform = __xgeSvgMatrixMul(tBaseTransform, tTransform);
		}
	}
}

static void __xgeSvgStyleApplyDeclaration(xge_svg pSvg, xge_svg_style_t* pStyle, const xge_svg_style_t* pInheritedStyle, const char* sStyle, int iImportanceMode, xge_shape_ex_matrix_t tBaseTransform, float fBaseOpacity, int bBaseVisible, float fBaseFontSize, int iApplyMode)
{
	__xgeSvgStyleApplyProperties(pSvg, pStyle, pInheritedStyle, NULL, NULL, sStyle, 0, 1, iImportanceMode, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, iApplyMode);
}

static void __xgeSvgStyleApplyPresentationAttrs(xge_svg pSvg, xge_svg_style_t* pStyle, const xge_svg_style_t* pInheritedStyle, const char* pTag, const char* pTagEnd, xge_shape_ex_matrix_t tBaseTransform, float fBaseOpacity, int bBaseVisible, float fBaseFontSize, int iApplyMode)
{
	if ( (pStyle == NULL) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return;
	}
	__xgeSvgStyleApplyProperties(pSvg, pStyle, pInheritedStyle, pTag, pTagEnd, NULL, 1, 1, XGE_SVG_STYLE_IMPORTANCE_ANY, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, iApplyMode);
}

static void __xgeSvgStyleApplyInlineStyle(xge_svg pSvg, xge_svg_style_t* pStyle, const xge_svg_style_t* pInheritedStyle, const char* pTag, const char* pTagEnd, int iImportanceMode, xge_shape_ex_matrix_t tBaseTransform, float fBaseOpacity, int bBaseVisible, float fBaseFontSize, int iApplyMode)
{
	char sStyle[XGE_SVG_ATTR_MAX];

	if ( (pStyle == NULL) || (pTag == NULL) || (pTagEnd == NULL) ) {
		return;
	}
	sStyle[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "style", sStyle, sizeof(sStyle));
	__xgeSvgStyleApplyProperties(pSvg, pStyle, pInheritedStyle, NULL, NULL, sStyle, 0, 1, iImportanceMode, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, iApplyMode);
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

static int __xgeSvgCssCharEquals(char a, char b, int bCaseInsensitive)
{
	if ( bCaseInsensitive ) {
		a = (char)__xgeSvgCssAsciiLower((unsigned char)a);
		b = (char)__xgeSvgCssAsciiLower((unsigned char)b);
	}
	return a == b;
}

static int __xgeSvgCssStringEqualsEx(const char* sA, const char* sB, int bCaseInsensitive)
{
	int i;

	if ( (sA == NULL) || (sB == NULL) ) {
		return 0;
	}
	for ( i = 0; sA[i] != '\0' || sB[i] != '\0'; i++ ) {
		if ( !__xgeSvgCssCharEquals(sA[i], sB[i], bCaseInsensitive) ) {
			return 0;
		}
	}
	return 1;
}

static int __xgeSvgCssRangeEqualsStringEx(const char* pStart, const char* pEnd, const char* sText, int bCaseInsensitive)
{
	int i;
	int iLen;

	if ( (pStart == NULL) || (pEnd == NULL) || (sText == NULL) || (pEnd < pStart) ) {
		return 0;
	}
	iLen = (int)(pEnd - pStart);
	for ( i = 0; i < iLen; i++ ) {
		if ( sText[i] == '\0' || !__xgeSvgCssCharEquals(pStart[i], sText[i], bCaseInsensitive) ) {
			return 0;
		}
	}
	return sText[iLen] == '\0';
}

static int __xgeSvgCssStringStartsWithEx(const char* sText, const char* sPrefix, int bCaseInsensitive)
{
	int i;

	if ( (sText == NULL) || (sPrefix == NULL) || (sPrefix[0] == '\0') ) {
		return 0;
	}
	for ( i = 0; sPrefix[i] != '\0'; i++ ) {
		if ( sText[i] == '\0' || !__xgeSvgCssCharEquals(sText[i], sPrefix[i], bCaseInsensitive) ) {
			return 0;
		}
	}
	return 1;
}

static int __xgeSvgCssStringEndsWithEx(const char* sText, const char* sSuffix, int bCaseInsensitive)
{
	int iTextLen;
	int iSuffixLen;

	if ( (sText == NULL) || (sSuffix == NULL) || (sSuffix[0] == '\0') ) {
		return 0;
	}
	iTextLen = (int)strlen(sText);
	iSuffixLen = (int)strlen(sSuffix);
	if ( iTextLen < iSuffixLen ) {
		return 0;
	}
	return __xgeSvgCssRangeEqualsStringEx(sText + iTextLen - iSuffixLen, sText + iTextLen, sSuffix, bCaseInsensitive);
}

static int __xgeSvgCssStringContainsEx(const char* sText, const char* sNeedle, int bCaseInsensitive)
{
	int iNeedleLen;
	const char* p;

	if ( (sText == NULL) || (sNeedle == NULL) || (sNeedle[0] == '\0') ) {
		return 0;
	}
	iNeedleLen = (int)strlen(sNeedle);
	for ( p = sText; *p != '\0'; p++ ) {
		if ( (int)strlen(p) < iNeedleLen ) {
			break;
		}
		if ( __xgeSvgCssRangeEqualsStringEx(p, p + iNeedleLen, sNeedle, bCaseInsensitive) ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeSvgCssAttributeWordContains(const char* sText, const char* sWord, int bCaseInsensitive)
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
		if ( ((int)(pEnd - pStart) == iWordLen) && __xgeSvgCssRangeEqualsStringEx(pStart, pEnd, sWord, bCaseInsensitive) ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeSvgCssAttributeMatches(const char* pTag, const char* pTagEnd, const char* sName, const char* sOp, const char* sExpected, int bCaseInsensitive)
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
		return __xgeSvgCssStringEqualsEx(sActual, sExpected, bCaseInsensitive);
	}
	if ( strcmp(sOp, "~=") == 0 ) {
		return __xgeSvgCssAttributeWordContains(sActual, sExpected, bCaseInsensitive);
	}
	if ( strcmp(sOp, "|=") == 0 ) {
		return __xgeSvgCssStringEqualsEx(sActual, sExpected, bCaseInsensitive) ||
		       ((iActualLen > iExpectedLen) && __xgeSvgCssRangeEqualsStringEx(sActual, sActual + iExpectedLen, sExpected, bCaseInsensitive) && (sActual[iExpectedLen] == '-'));
	}
	if ( strcmp(sOp, "^=") == 0 ) {
		return (iExpectedLen > 0) && (iActualLen >= iExpectedLen) && __xgeSvgCssStringStartsWithEx(sActual, sExpected, bCaseInsensitive);
	}
	if ( strcmp(sOp, "$=") == 0 ) {
		return (iExpectedLen > 0) && (iActualLen >= iExpectedLen) && __xgeSvgCssStringEndsWithEx(sActual, sExpected, bCaseInsensitive);
	}
	if ( strcmp(sOp, "*=") == 0 ) {
		return (iExpectedLen > 0) && __xgeSvgCssStringContainsEx(sActual, sExpected, bCaseInsensitive);
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

static int __xgeSvgSimpleSelectorReadToken(const char** ppText, const char* pEnd, char* sOut, int iOutCapacity)
{
	const char* p;
	const char* pTokenEnd;

	if ( (ppText == NULL) || (*ppText == NULL) || (pEnd == NULL) || (sOut == NULL) || (iOutCapacity <= 0) ) {
		return 0;
	}
	sOut[0] = '\0';
	p = *ppText;
	if ( !__xgeSvgCssIdentifierTokenEnd(p, pEnd, &pTokenEnd) ||
	     !__xgeSvgCssIdentifierTokenCopyDecoded(p, pTokenEnd, sOut, iOutCapacity) ) {
		return 0;
	}
	*ppText = pTokenEnd;
	return 1;
}

static int __xgeSvgSimpleSelectorMatches(xge_svg pSvg, const char* sSelector, int iNodeIndex)
{
	const char* p;
	const char* pEnd;
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
	pEnd = sSelector + strlen(sSelector);
	bMatched = 0;
	if ( *p == '*' ) {
		p++;
		bMatched = 1;
	} else if ( (*p != '.') && (*p != '#') && (*p != '[') && (*p != ':') ) {
		if ( !__xgeSvgSimpleSelectorReadToken(&p, pEnd, sToken, sizeof(sToken)) ) {
			return 0;
		}
		if ( !__xgeSvgTagNameCopy(pTag, sTagName, sizeof(sTagName)) || (strcmp(sTagName, sToken) != 0) ) {
			return 0;
		}
		bMatched = 1;
	}
	while ( p < pEnd ) {
		if ( *p == '.' ) {
			p++;
			if ( !__xgeSvgSimpleSelectorReadToken(&p, pEnd, sToken, sizeof(sToken)) ) {
				return 0;
			}
			if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "class", sValue, sizeof(sValue)) ||
			     !__xgeSvgClassListContains(sValue, sToken) ) {
				return 0;
			}
			bMatched = 1;
		} else if ( *p == '#' ) {
			p++;
			if ( !__xgeSvgSimpleSelectorReadToken(&p, pEnd, sToken, sizeof(sToken)) ) {
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
			int bCaseInsensitive;

			if ( !__xgeSvgCssAttributeSelectorEnd(p, pEnd, &pAttrEnd) ||
			     !__xgeSvgCssAttributeSelectorParse(p, pAttrEnd, sName, sizeof(sName), sOp, sizeof(sOp), sExpected, sizeof(sExpected), &bCaseInsensitive) ||
			     !__xgeSvgCssAttributeMatches(pTag, pTagEnd, sName, sOp, sExpected, bCaseInsensitive) ) {
				return 0;
			}
			p = pAttrEnd;
			bMatched = 1;
		} else if ( *p == ':' ) {
			const char* pNameStart;
			const char* pNameEnd;

			p++;
			pNameStart = p;
			while ( (p < pEnd) && __xgeSvgCssIdentChar(*p) ) {
				p++;
			}
			pNameEnd = p;
			if ( !__xgeSvgCssRangeCopy(pNameStart, pNameEnd, sToken, sizeof(sToken)) ) {
				return 0;
			}
			__xgeSvgCssAsciiLowerString(sToken);
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

				if ( (p >= pEnd) || (*p != '(') || !__xgeSvgCssFunctionEnd(p, pEnd, &pFunctionEnd) ||
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

				if ( (p >= pEnd) || (*p != '(') || !__xgeSvgCssFunctionEnd(p, pEnd, &pFunctionEnd) ) {
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
					const char* pNext = p;
					uint32_t iCodepoint;

					if ( __xgeSvgCssEscapeNext(&pNext, pEnd, &iCodepoint) ) {
						p = pNext;
						continue;
					}
				}
				if ( *p == cQuote ) {
					cQuote = 0;
				}
				p++;
				continue;
			}
			if ( *p == '\\' ) {
				const char* pNext = p;
				uint32_t iCodepoint;

				if ( !__xgeSvgCssEscapeNext(&pNext, pEnd, &iCodepoint) ) {
					break;
				}
				p = pNext;
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
		if ( (p < pEnd) && (cQuote == 0) && !bInAttr && (*p == '\\') ) {
			const char* pNext = p;
			uint32_t iCodepoint;

			if ( !__xgeSvgCssEscapeNext(&pNext, pEnd, &iCodepoint) ) {
				return 0;
			}
			p = pNext;
			continue;
		}
		if ( cQuote != 0 ) {
			if ( *p == '\\' ) {
				const char* pNext = p;
				uint32_t iCodepoint;

				if ( __xgeSvgCssEscapeNext(&pNext, pEnd, &iCodepoint) ) {
					p = pNext;
					continue;
				}
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
		if ( (p < pEnd) && (cQuote == 0) && !bInAttr && (*p == '\\') ) {
			const char* pNext = p;
			uint32_t iCodepoint;

			if ( !__xgeSvgCssEscapeNext(&pNext, pEnd, &iCodepoint) ) {
				return 0;
			}
			p = pNext;
			continue;
		}
		if ( cQuote != 0 ) {
			if ( *p == '\\' ) {
				const char* pNext = p;
				uint32_t iCodepoint;

				if ( __xgeSvgCssEscapeNext(&pNext, pEnd, &iCodepoint) ) {
					p = pNext;
					continue;
				}
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

static void __xgeSvgStyleApplyCss(xge_svg pSvg, xge_svg_style_t* pStyle, const xge_svg_style_t* pInheritedStyle, const char* pTag, const char* pTagEnd, int iImportanceMode, xge_shape_ex_matrix_t tBaseTransform, float fBaseOpacity, int bBaseVisible, float fBaseFontSize, int iApplyMode)
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
		__xgeSvgStyleApplyDeclaration(pSvg, pStyle, pInheritedStyle, pSvg->pStyleRules[iBestIndex].sStyle, iImportanceMode, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, iApplyMode);
		iLastSpecificity = iBestSpecificity;
		iLastIndex = iBestIndex;
	}
}

static xge_svg_style_t __xgeSvgStyleResolve(xge_svg pSvg, const xge_svg_style_t* pParentStyle, const char* pTag, const char* pTagEnd)
{
	xge_svg_style_t tStyle;
	xge_svg_style_t tDefaultStyle;
	const xge_svg_style_t* pInheritedStyle;
	xge_shape_ex_matrix_t tBaseTransform;
	float fBaseOpacity;
	float fBaseFontSize;
	int bBaseVisible;

	(void)__xgeSvgElementNodeEnsure(pSvg, pTag, pTagEnd, NULL);
	if ( pParentStyle != NULL ) {
		tStyle = *pParentStyle;
		pInheritedStyle = pParentStyle;
	} else {
		tDefaultStyle = __xgeSvgStyleDefault();
		tStyle = tDefaultStyle;
		pInheritedStyle = &tDefaultStyle;
	}
	tStyle.bStrokeNonScaling = 0;
	tStyle.fPathLength = 0.0f;
	tBaseTransform = tStyle.tTransform;
	fBaseOpacity = tStyle.fOpacity;
	fBaseFontSize = tStyle.fFontSize;
	bBaseVisible = tStyle.bVisible;
	__xgeSvgStyleApplyPresentationAttrs(pSvg, &tStyle, pInheritedStyle, pTag, pTagEnd, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_FONT_SIZE);
	__xgeSvgStyleApplyCss(pSvg, &tStyle, pInheritedStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_NORMAL, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_FONT_SIZE);
	__xgeSvgStyleApplyInlineStyle(pSvg, &tStyle, pInheritedStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_NORMAL, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_FONT_SIZE);
	__xgeSvgStyleApplyCss(pSvg, &tStyle, pInheritedStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_IMPORTANT, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_FONT_SIZE);
	__xgeSvgStyleApplyInlineStyle(pSvg, &tStyle, pInheritedStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_IMPORTANT, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_FONT_SIZE);
	__xgeSvgStyleApplyPresentationAttrs(pSvg, &tStyle, pInheritedStyle, pTag, pTagEnd, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_EXCEPT_FONT_SIZE);
	__xgeSvgStyleApplyCss(pSvg, &tStyle, pInheritedStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_NORMAL, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_EXCEPT_FONT_SIZE);
	__xgeSvgStyleApplyInlineStyle(pSvg, &tStyle, pInheritedStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_NORMAL, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_EXCEPT_FONT_SIZE);
	__xgeSvgStyleApplyCss(pSvg, &tStyle, pInheritedStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_IMPORTANT, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_EXCEPT_FONT_SIZE);
	__xgeSvgStyleApplyInlineStyle(pSvg, &tStyle, pInheritedStyle, pTag, pTagEnd, XGE_SVG_STYLE_IMPORTANCE_IMPORTANT, tBaseTransform, fBaseOpacity, bBaseVisible, fBaseFontSize, XGE_SVG_STYLE_APPLY_EXCEPT_FONT_SIZE);
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

static int __xgeSvgApplyClipRectsAsShapes(xge_svg_clip_path_t* pClip, xge_shape_ex pShape)
{
	int i;
	int iRet;

	if ( (pClip == NULL) || (pShape == NULL) || (pClip->iRectCount <= 0) ) {
		return XGE_OK;
	}
	for ( i = 0; i < pClip->iRectCount; i++ ) {
		xge_shape_ex pClipShape;
		xge_rect_t tClip;

		iRet = __xgeSvgClipPathResolveRect(pClip, i, pShape, &tClip);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( (tClip.fW <= 0.0f) || (tClip.fH <= 0.0f) ) {
			continue;
		}
		pClipShape = NULL;
		iRet = xgeShapeExCreate(&pClipShape);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		iRet = xgeShapeExAppendRect(pClipShape, tClip.fX, tClip.fY, tClip.fW, tClip.fH, 0.0f, 0.0f, 1);
		if ( iRet == XGE_OK ) {
			iRet = xgeShapeExClipShapeAdd(pShape, pClipShape);
		}
		xgeShapeExDestroy(pClipShape);
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
	if ( (pClip->iShapeCount <= 0) && (pClip->iRectCount == 1) ) {
		if ( __xgeSvgClipPathResolveRect(pClip, 0, pShape, &tClip) != XGE_OK ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		xgeShapeExClipRectSet(pShape, tClip);
	} else if ( (pClip->iShapeCount <= 0) && (pClip->iRectCount > 1) ) {
		iRet = __xgeSvgApplyClipRectsAsShapes(pClip, pShape);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
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
					if ( xgeShapeExFillRadialGradientEx(pShape, pGradient->fCX, pGradient->fCY, pGradient->fR, pGradient->fFX, pGradient->fFY, pGradient->fFR, pGradient->iUnits, pStops, pGradient->iStopCount) == XGE_OK ) {
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
					if ( xgeShapeExStrokeRadialGradientEx(pShape, pGradient->fCX, pGradient->fCY, pGradient->fR, pGradient->fFX, pGradient->fFY, pGradient->fFR, pGradient->iUnits, pStops, pGradient->iStopCount) == XGE_OK ) {
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
	if ( pStyle->bBlendSet ) {
		xgeShapeExBlend(pShape, pStyle->iBlend);
	} else {
		xgeShapeExBlendClear(pShape);
	}
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
		bApplied = xgeShapeExFillRadialGradientEx(pShape, pGradient->fCX, pGradient->fCY, pGradient->fR, pGradient->fFX, pGradient->fFY, pGradient->fFR, pGradient->iUnits, pStops, pGradient->iStopCount) == XGE_OK;
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
		bApplied = xgeShapeExStrokeRadialGradientEx(pShape, pGradient->fCX, pGradient->fCY, pGradient->fR, pGradient->fFX, pGradient->fFY, pGradient->fFR, pGradient->iUnits, pStops, pGradient->iStopCount) == XGE_OK;
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

static int __xgeSvgPathLengthAttr(const char* pTag, const char* pTagEnd, float* pLength)
{
	float fLength;

	if ( pLength != NULL ) {
		*pLength = 0.0f;
	}
	if ( !__xgeSvgAttrFloatStrict(pTag, pTagEnd, "pathLength", &fLength) || (fLength <= 0.0f) ) {
		return 0;
	}
	if ( pLength != NULL ) {
		*pLength = fLength;
	}
	return 1;
}

static void __xgeSvgApplyPathLengthToStyle(xge_shape_ex pShape, const char* pTag, const char* pTagEnd, xge_svg_style_t* pStyle)
{
	float fDeclaredLength;
	float fActualLength;
	float fScale;
	int i;

	if ( (pShape == NULL) || (pTag == NULL) || (pTagEnd == NULL) || (pStyle == NULL) ) {
		return;
	}
	pStyle->fPathLength = 0.0f;
	if ( !__xgeSvgPathLengthAttr(pTag, pTagEnd, &fDeclaredLength) ) {
		return;
	}
	pStyle->fPathLength = fDeclaredLength;
	if ( pStyle->iDashCount <= 0 ) {
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
	const char* pEnd;
	xge_vec2_t* pPoints;
	double fValue;
	int iCount;
	int iCapacity;
	float fX;
	float fY;

	if ( (sPoints == NULL) || (ppPoints == NULL) || (pCount == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppPoints = NULL;
	*pCount = 0;
	p = sPoints;
	pPoints = NULL;
	iCount = 0;
	iCapacity = 0;
	while ( *p != '\0' ) {
		while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') || (*p == ',') ) p++;
		if ( *p == '\0' ) break;
		if ( !__xgeSvgParseNumberAt(p, &pEnd, &fValue) ) {
			xrtFree(pPoints);
			return XGE_OK;
		}
		if ( !__xgeSvgDoubleToFloat(fValue, &fX) ) {
			xrtFree(pPoints);
			return XGE_OK;
		}
		p = pEnd;
		while ( (*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') || (*p == ',') ) p++;
		if ( !__xgeSvgParseNumberAt(p, &pEnd, &fValue) ) {
			xrtFree(pPoints);
			return XGE_OK;
		}
		if ( !__xgeSvgDoubleToFloat(fValue, &fY) ) {
			xrtFree(pPoints);
			return XGE_OK;
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
	if ( (pShape == NULL) || (pPoints == NULL) || (iCount <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return bClose ? xgeShapeExAppendPolygon(pShape, pPoints, iCount) :
	                xgeShapeExAppendPolyline(pShape, pPoints, iCount);
}

static int __xgeSvgCreatePointListShape(const char* sPoints, int bClose, xge_shape_ex* ppShape)
{
	xge_shape_ex pShape;
	xge_vec2_t* pPoints;
	int iCount;
	int iRet;

	if ( (sPoints == NULL) || (ppShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppShape = NULL;
	pPoints = NULL;
	iCount = 0;
	iRet = __xgeSvgParsePointList(sPoints, &pPoints, &iCount);
	if ( iRet != XGE_OK ) {
		xrtFree(pPoints);
		return iRet;
	}
	if ( iCount <= 0 ) {
		xrtFree(pPoints);
		return XGE_OK;
	}
	iRet = xgeShapeExCreate(&pShape);
	if ( iRet != XGE_OK ) {
		xrtFree(pPoints);
		return iRet;
	}
	iRet = __xgeSvgAppendPointList(pShape, pPoints, iCount, bClose);
	xrtFree(pPoints);
	if ( iRet != XGE_OK ) {
		xgeShapeExDestroy(pShape);
		return iRet;
	}
	*ppShape = pShape;
	return iRet;
}

static int __xgeSvgAddShapeStyled(xge_svg pSvg, xge_shape_ex pShape, int iDefIndex, const xge_svg_style_t* pStyle)
{
	xge_svg_draw_item_t* pItem;
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
			pItem = &pSvg->pItems[pSvg->iItemCount];
			memset(pItem, 0, sizeof(*pItem));
			pItem->iType = XGE_SVG_DRAW_ITEM_SCENE;
			pItem->iBlend = pStyle != NULL ? pStyle->iBlend : XGE_BLEND_ALPHA;
			pItem->bBlendSet = pStyle != NULL ? pStyle->bBlendSet : 0;
			iRet = xgeShapeExSceneCreate(&pItem->u.pScene);
			if ( iRet == XGE_OK && pItem->bBlendSet ) {
				iRet = xgeShapeExSceneBlend(pItem->u.pScene, pItem->iBlend);
			}
			if ( iRet == XGE_OK ) {
				iRet = xgeShapeExSceneAdd(pItem->u.pScene, pShape);
			}
			if ( iRet == XGE_OK ) {
				pSvg->iItemCount++;
			} else {
				__xgeSvgDrawItemReset(pItem);
			}
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
	tBounds.fY = pText->fY + pText->tStyle.fBaselineShift - fHeight;
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
	iRet = xgeShapeExClone(pMaskShape->pShape, &pClipShape);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeShapeExClipShapeAdd(pTarget, pClipShape);
	xgeShapeExDestroy(pClipShape);
	return iRet;
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
	iRet = xgeShapeExClone(pMaskShape->pShape, &pShape);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	*ppShape = pShape;
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
	xge_svg_text_item_t tSolidPaint;
	xge_svg_text_item_t tFilterOriginal;
	xge_svg_text_item_t tViewportClipped;

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
		float fBlurX;
		float fBlurY;
		float fBlurOffsetX;
		float fBlurOffsetY;
		uint32_t iFloodColor;
		float fFloodOpacity;
		xge_rect_t tFilterOutputRegion;
		const char* sFilterImageHref;
		const char* sFilterImageAspect;
		int iFilterRegion;
		int bBlurSourceAlpha;

		tBounds = __xgeSvgTextMaskBounds(pText);
		iFilterRegion = __xgeSvgFilterRegionForBounds(pSvg, pText->tStyle.sFilterId, tBounds, &tFilterRegion);
		if ( iFilterRegion < 0 ) {
			return XGE_OK;
		}
		tSolidPaint = *pText;
		tSolidPaint.tStyle.sFilterId[0] = '\0';
		if ( __xgeSvgFilterSolidPaintGraphForStyle(pSvg, pText->tStyle.sFilterId, &tSolidPaint.tStyle, 0) ) {
			if ( (iFilterRegion > 0) && !__xgeSvgTextClipRectIntersect(&tSolidPaint, tFilterRegion) ) {
				return XGE_OK;
			}
			return __xgeSvgAddTextItemMasked(pSvg, &tSolidPaint, iDefIndex);
		}
		if ( __xgeSvgFilterFloodForBounds(pSvg, pText->tStyle.sFilterId, &pText->tStyle, tBounds, tFilterRegion, &iFloodColor, &fFloodOpacity, &tFilterOutputRegion) ) {
			return __xgeSvgAddFilterFloodOutput(pSvg, &pText->tStyle, tFilterOutputRegion, iFloodColor, fFloodOpacity, iDefIndex);
		}
		if ( __xgeSvgFilterImageForBounds(pSvg, pText->tStyle.sFilterId, tBounds, tFilterRegion, &sFilterImageHref, &sFilterImageAspect, &tFilterOutputRegion) ) {
			return __xgeSvgAddFilterImageOutput(pSvg, &pText->tStyle, sFilterImageHref, sFilterImageAspect, tFilterOutputRegion, iDefIndex);
		}
		if ( __xgeSvgFilterIndependentColorGraphForStyle(pSvg, pText->tStyle.sFilterId, &pText->tStyle, tBounds, tFilterRegion, &iFloodColor, &tFilterOutputRegion) ) {
			return __xgeSvgAddFilterFloodOutput(pSvg, &pText->tStyle, tFilterOutputRegion, iFloodColor, 1.0f, iDefIndex);
		}
		bBlurSourceAlpha = 0;
		if ( __xgeSvgFilterGaussianBlurOffsetForBoundsEx(pSvg, pText->tStyle.sFilterId, tBounds, &fBlurX, &fBlurY, &fBlurOffsetX, &fBlurOffsetY, &bBlurSourceAlpha) ) {
			int iTap;

			for ( iTap = 0; iTap < 9; iTap++ ) {
				xge_svg_text_item_t tBlur;
				xge_shape_ex_matrix_t tOffset;
				float fTapX;
				float fTapY;
				float fTapWeight;

				if ( !__xgeSvgGaussianBlurTap(iTap, fBlurX, fBlurY, &fTapX, &fTapY, &fTapWeight) || (fTapWeight <= 0.0f) ) {
					continue;
				}
				tBlur = *pText;
				tBlur.tStyle.sFilterId[0] = '\0';
				tBlur.tStyle.fOpacity *= fTapWeight;
				if ( bBlurSourceAlpha ) {
					__xgeSvgApplyDropShadowPaintToText(&tBlur, XGE_COLOR_RGBA(0, 0, 0, 255), 1.0f);
				}
				tOffset = __xgeSvgMatrixTranslate(fBlurOffsetX + fTapX, fBlurOffsetY + fTapY);
				tBlur.tStyle.tTransform = __xgeSvgMatrixMul(tOffset, tBlur.tStyle.tTransform);
				if ( (iFilterRegion > 0) && !__xgeSvgTextClipRectIntersect(&tBlur, tFilterRegion) ) {
					continue;
				}
				iRet = __xgeSvgAddTextItemMasked(pSvg, &tBlur, iDefIndex);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
			}
			return XGE_OK;
		}
		if ( __xgeSvgFilterDropShadowForBounds(pSvg, pText->tStyle.sFilterId, &pText->tStyle, tBounds, &fOffsetX, &fOffsetY, &iShadowColor, &fShadowOpacity, &fShadowStdDeviation) ) {
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
		{
			xge_svg_text_item_t tCopy;
			xge_shape_ex_matrix_t tOffset;
			int bOffsetSourceAlpha;

			bOffsetSourceAlpha = 0;
			if ( __xgeSvgFilterOffsetForBoundsEx(pSvg, pText->tStyle.sFilterId, tBounds, &fOffsetX, &fOffsetY, &bOffsetSourceAlpha) ) {
				tCopy = *pText;
				tCopy.tStyle.sFilterId[0] = '\0';
				tOffset = __xgeSvgMatrixTranslate(fOffsetX, fOffsetY);
				tCopy.tStyle.tTransform = __xgeSvgMatrixMul(tOffset, tCopy.tStyle.tTransform);
				if ( bOffsetSourceAlpha ) {
					__xgeSvgApplyDropShadowPaintToText(&tCopy, XGE_COLOR_RGBA(0, 0, 0, 255), 1.0f);
				}
				if ( (iFilterRegion > 0) && !__xgeSvgTextClipRectIntersect(&tCopy, tFilterRegion) ) {
					return XGE_OK;
				}
				return __xgeSvgAddTextItemMasked(pSvg, &tCopy, iDefIndex);
			}
		}
	}
	if ( pText->tStyle.bViewportClip ) {
		tViewportClipped = *pText;
		if ( !__xgeSvgApplyViewportClipToText(&tViewportClipped) ) {
			return XGE_OK;
		}
		pText = &tViewportClipped;
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
	xge_svg_image_item_t tViewportClipped;
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
		float fBlurX;
		float fBlurY;
		float fBlurOffsetX;
		float fBlurOffsetY;
		uint32_t iFloodColor;
		float fFloodOpacity;
		xge_rect_t tFilterOutputRegion;
		const char* sFilterImageHref;
		const char* sFilterImageAspect;
		int iFilterRegion;

		iFilterRegion = __xgeSvgFilterRegionForBounds(pSvg, pImage->tStyle.sFilterId, pImage->tRect, &tFilterRegion);
		if ( iFilterRegion < 0 ) {
			return XGE_OK;
		}
		if ( __xgeSvgFilterFloodForBounds(pSvg, pImage->tStyle.sFilterId, &pImage->tStyle, pImage->tRect, tFilterRegion, &iFloodColor, &fFloodOpacity, &tFilterOutputRegion) ) {
			return __xgeSvgAddFilterFloodOutput(pSvg, &pImage->tStyle, tFilterOutputRegion, iFloodColor, fFloodOpacity, iDefIndex);
		}
		if ( __xgeSvgFilterImageForBounds(pSvg, pImage->tStyle.sFilterId, pImage->tRect, tFilterRegion, &sFilterImageHref, &sFilterImageAspect, &tFilterOutputRegion) ) {
			return __xgeSvgAddFilterImageOutput(pSvg, &pImage->tStyle, sFilterImageHref, sFilterImageAspect, tFilterOutputRegion, iDefIndex);
		}
		if ( __xgeSvgFilterIndependentColorGraphForStyle(pSvg, pImage->tStyle.sFilterId, &pImage->tStyle, pImage->tRect, tFilterRegion, &iFloodColor, &tFilterOutputRegion) ) {
			return __xgeSvgAddFilterFloodOutput(pSvg, &pImage->tStyle, tFilterOutputRegion, iFloodColor, 1.0f, iDefIndex);
		}
		if ( __xgeSvgFilterGaussianBlurOffsetForBounds(pSvg, pImage->tStyle.sFilterId, pImage->tRect, &fBlurX, &fBlurY, &fBlurOffsetX, &fBlurOffsetY) ) {
			int iTap;

			for ( iTap = 0; iTap < 9; iTap++ ) {
				xge_svg_image_item_t tBlur;
				xge_shape_ex_matrix_t tOffset;
				float fTapX;
				float fTapY;
				float fTapWeight;

				if ( !__xgeSvgGaussianBlurTap(iTap, fBlurX, fBlurY, &fTapX, &fTapY, &fTapWeight) || (fTapWeight <= 0.0f) ) {
					continue;
				}
				tBlur = *pImage;
				tBlur.tStyle.sFilterId[0] = '\0';
				tBlur.tStyle.fOpacity *= fTapWeight;
				tOffset = __xgeSvgMatrixTranslate(fBlurOffsetX + fTapX, fBlurOffsetY + fTapY);
				tBlur.tStyle.tTransform = __xgeSvgMatrixMul(tOffset, tBlur.tStyle.tTransform);
				if ( (iFilterRegion > 0) && !__xgeSvgImageClipRectIntersect(&tBlur, tFilterRegion) ) {
					continue;
				}
				iRet = __xgeSvgAddImageItemMasked(pSvg, &tBlur, iDefIndex);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
			}
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
	if ( pImage->tStyle.bViewportClip ) {
		tViewportClipped = *pImage;
		if ( !__xgeSvgApplyViewportClipToImage(&tViewportClipped) ) {
			return XGE_OK;
		}
		pImage = &tViewportClipped;
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
	xge_svg_raster_item_t tViewportClipped;
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
		float fBlurX;
		float fBlurY;
		float fBlurOffsetX;
		float fBlurOffsetY;
		uint32_t iFloodColor;
		float fFloodOpacity;
		xge_rect_t tFilterOutputRegion;
		const char* sFilterImageHref;
		const char* sFilterImageAspect;
		int iFilterRegion;

		iFilterRegion = __xgeSvgFilterRegionForBounds(pSvg, pImage->tStyle.sFilterId, pImage->tRect, &tFilterRegion);
		if ( iFilterRegion < 0 ) {
			return XGE_OK;
		}
		if ( __xgeSvgFilterFloodForBounds(pSvg, pImage->tStyle.sFilterId, &pImage->tStyle, pImage->tRect, tFilterRegion, &iFloodColor, &fFloodOpacity, &tFilterOutputRegion) ) {
			return __xgeSvgAddFilterFloodOutput(pSvg, &pImage->tStyle, tFilterOutputRegion, iFloodColor, fFloodOpacity, iDefIndex);
		}
		if ( __xgeSvgFilterImageForBounds(pSvg, pImage->tStyle.sFilterId, pImage->tRect, tFilterRegion, &sFilterImageHref, &sFilterImageAspect, &tFilterOutputRegion) ) {
			return __xgeSvgAddFilterImageOutput(pSvg, &pImage->tStyle, sFilterImageHref, sFilterImageAspect, tFilterOutputRegion, iDefIndex);
		}
		if ( __xgeSvgFilterIndependentColorGraphForStyle(pSvg, pImage->tStyle.sFilterId, &pImage->tStyle, pImage->tRect, tFilterRegion, &iFloodColor, &tFilterOutputRegion) ) {
			return __xgeSvgAddFilterFloodOutput(pSvg, &pImage->tStyle, tFilterOutputRegion, iFloodColor, 1.0f, iDefIndex);
		}
		if ( __xgeSvgFilterGaussianBlurOffsetForBounds(pSvg, pImage->tStyle.sFilterId, pImage->tRect, &fBlurX, &fBlurY, &fBlurOffsetX, &fBlurOffsetY) ) {
			int iTap;

			for ( iTap = 0; iTap < 9; iTap++ ) {
				xge_svg_raster_item_t tBlur;
				xge_shape_ex_matrix_t tOffset;
				float fTapX;
				float fTapY;
				float fTapWeight;

				if ( !__xgeSvgGaussianBlurTap(iTap, fBlurX, fBlurY, &fTapX, &fTapY, &fTapWeight) || (fTapWeight <= 0.0f) ) {
					continue;
				}
				tBlur = *pImage;
				tBlur.tStyle.sFilterId[0] = '\0';
				tBlur.tStyle.fOpacity *= fTapWeight;
				tOffset = __xgeSvgMatrixTranslate(fBlurOffsetX + fTapX, fBlurOffsetY + fTapY);
				tBlur.tStyle.tTransform = __xgeSvgMatrixMul(tOffset, tBlur.tStyle.tTransform);
				if ( (iFilterRegion > 0) && !__xgeSvgRasterClipRectIntersect(&tBlur, tFilterRegion) ) {
					continue;
				}
				iRet = __xgeSvgAddRasterItemMasked(pSvg, &tBlur, iDefIndex);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
			}
			return XGE_OK;
		}
		if ( __xgeSvgFilterDropShadowForBounds(pSvg, pImage->tStyle.sFilterId, &pImage->tStyle, pImage->tRect, &fOffsetX, &fOffsetY, &iShadowColor, &fShadowOpacity, &fShadowStdDeviation) ) {
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
	if ( pImage->tStyle.bViewportClip ) {
		tViewportClipped = *pImage;
		if ( !__xgeSvgApplyViewportClipToRaster(&tViewportClipped) ) {
			return XGE_OK;
		}
		pImage = &tViewportClipped;
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

static void __xgeSvgPatternContentBoundsUnion(xge_rect_t* pOut, int* pHasBounds, xge_rect_t tBounds)
{
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;

	if ( (pOut == NULL) || (pHasBounds == NULL) || (tBounds.fW <= 0.0f) || (tBounds.fH <= 0.0f) ) {
		return;
	}
	if ( !*pHasBounds ) {
		*pOut = tBounds;
		*pHasBounds = 1;
		return;
	}
	fLeft = fminf(pOut->fX, tBounds.fX);
	fTop = fminf(pOut->fY, tBounds.fY);
	fRight = fmaxf(pOut->fX + pOut->fW, tBounds.fX + tBounds.fW);
	fBottom = fmaxf(pOut->fY + pOut->fH, tBounds.fY + tBounds.fH);
	pOut->fX = fLeft;
	pOut->fY = fTop;
	pOut->fW = fRight - fLeft;
	pOut->fH = fBottom - fTop;
}

static int __xgeSvgPatternContentBounds(xge_svg pSvg, const xge_svg_pattern_t* pPattern, xge_rect_t tObjectBounds, xge_rect_t tTileBase, xge_rect_t* pBounds)
{
	xge_svg_def_t* pDef;
	xge_shape_ex_matrix_t tTileMatrix;
	xge_rect_t tOut;
	int bHasBounds;
	int i;

	if ( !__xgeSvgValid(pSvg) || (pPattern == NULL) || (pBounds == NULL) ||
	     (pPattern->iDefIndex < 0) || (pPattern->iDefIndex >= pSvg->iDefCount) ) {
		return 0;
	}
	pDef = &pSvg->pDefs[pPattern->iDefIndex];
	tTileMatrix = __xgeSvgPatternTileMatrix(pSvg, pPattern, tTileBase, tObjectBounds);
	memset(&tOut, 0, sizeof(tOut));
	bHasBounds = 0;
	for ( i = 0; i < pDef->iShapeCount; i++ ) {
		xge_rect_t tBounds;

		if ( (pDef->pShapes[i] != NULL) && (xgeShapeExGetBounds(pDef->pShapes[i], 0.25f, &tBounds) == XGE_OK) ) {
			tBounds = __xgeSvgMatrixRectBounds(tTileMatrix, tBounds);
			__xgeSvgPatternContentBoundsUnion(&tOut, &bHasBounds, tBounds);
		}
	}
	for ( i = 0; i < pDef->iTextCount; i++ ) {
		xge_rect_t tBounds;
		xge_shape_ex_matrix_t tMatrix;

		tBounds = __xgeSvgTextMaskBounds(&pDef->pTexts[i]);
		if ( pDef->pTexts[i].bClipRect ) {
			tBounds = __xgeSvgRectIntersect(tBounds, pDef->pTexts[i].tClipRect);
		}
		tMatrix = __xgeSvgMatrixMul(tTileMatrix, pDef->pTexts[i].tStyle.tTransform);
		tBounds = __xgeSvgMatrixRectBounds(tMatrix, tBounds);
		__xgeSvgPatternContentBoundsUnion(&tOut, &bHasBounds, tBounds);
	}
	for ( i = 0; i < pDef->iImageCount; i++ ) {
		xge_rect_t tBounds;
		xge_shape_ex_matrix_t tMatrix;

		tBounds = pDef->pImages[i].tRect;
		if ( pDef->pImages[i].bClipRect ) {
			tBounds = __xgeSvgRectIntersect(tBounds, pDef->pImages[i].tClipRect);
		}
		tMatrix = __xgeSvgMatrixMul(tTileMatrix, pDef->pImages[i].tStyle.tTransform);
		tBounds = __xgeSvgMatrixRectBounds(tMatrix, tBounds);
		__xgeSvgPatternContentBoundsUnion(&tOut, &bHasBounds, tBounds);
	}
	for ( i = 0; i < pDef->iRasterCount; i++ ) {
		xge_rect_t tBounds;
		xge_shape_ex_matrix_t tMatrix;

		tBounds = pDef->pRasters[i].tRect;
		if ( pDef->pRasters[i].bClipRect ) {
			tBounds = __xgeSvgRectIntersect(tBounds, pDef->pRasters[i].tClipRect);
		}
		tMatrix = __xgeSvgMatrixMul(tTileMatrix, pDef->pRasters[i].tStyle.tTransform);
		tBounds = __xgeSvgMatrixRectBounds(tMatrix, tBounds);
		__xgeSvgPatternContentBoundsUnion(&tOut, &bHasBounds, tBounds);
	}
	*pBounds = tOut;
	return bHasBounds;
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
	if ( pDef->bOverflowVisible ) {
		xge_rect_t tContentBounds;
		if ( __xgeSvgPatternContentBounds(pSvg, pPattern, tObjectBounds, tTileBase, &tContentBounds) ) {
			float fTileRight = tTileBase.fX + tTileBase.fW;
			float fTileBottom = tTileBase.fY + tTileBase.fH;
			float fOverflowLeft = fmaxf(0.0f, tTileBase.fX - tContentBounds.fX);
			float fOverflowTop = fmaxf(0.0f, tTileBase.fY - tContentBounds.fY);
			float fOverflowRight = fmaxf(0.0f, (tContentBounds.fX + tContentBounds.fW) - fTileRight);
			float fOverflowBottom = fmaxf(0.0f, (tContentBounds.fY + tContentBounds.fH) - fTileBottom);

			fRight += fOverflowLeft;
			fBottom += fOverflowTop;
			tTargetClip.fX -= fOverflowRight;
			tTargetClip.fY -= fOverflowBottom;
			tTargetClip.fW += fOverflowRight + fOverflowLeft;
			tTargetClip.fH += fOverflowBottom + fOverflowTop;
		}
	}
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
			tClip = pDef->bOverflowVisible ? tTargetClip : __xgeSvgRectIntersect(tTargetClip, tTile);
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
	xge_svg_style_t tViewportClippedStyle;
	xge_shape_ex pStrokePatternShape;
	int bStrokePattern;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pShape == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pStyle != NULL) && (pStyle->sFilterId[0] != '\0') ) {
		xge_rect_t tBounds;
		xge_rect_t tFilterRegion;
		xge_svg_style_t tSolidPaintStyle;
		float fOffsetX;
		float fOffsetY;
		uint32_t iShadowColor;
		float fShadowOpacity;
		float fShadowStdDeviation;
		float fBlurX;
		float fBlurY;
		float fBlurOffsetX;
		float fBlurOffsetY;
		float fMorphRadiusX;
		float fMorphRadiusY;
		uint32_t iFloodColor;
		float fFloodOpacity;
		xge_rect_t tFilterOutputRegion;
		const char* sFilterImageHref;
		const char* sFilterImageAspect;
		int iFilterRegion;
		int bHasFilterBounds;
		int bOffsetSourceAlpha;
		int bBlurSourceAlpha;
		int bMorphSourceAlpha;

		memset(&tBounds, 0, sizeof(tBounds));
		memset(&tFilterRegion, 0, sizeof(tFilterRegion));
		iFilterRegion = 0;
		bHasFilterBounds = 0;
		bOffsetSourceAlpha = 0;
		bBlurSourceAlpha = 0;
		bMorphSourceAlpha = 0;
		if ( xgeShapeExGetBounds(pShape, 0.0f, &tBounds) == XGE_OK ) {
			bHasFilterBounds = 1;
			iFilterRegion = __xgeSvgFilterRegionForBounds(pSvg, pStyle->sFilterId, tBounds, &tFilterRegion);
			if ( iFilterRegion < 0 ) {
				xgeShapeExDestroy(pShape);
				return XGE_OK;
			}
		}

		if ( bHasFilterBounds ) {
			tSolidPaintStyle = *pStyle;
			tSolidPaintStyle.sFilterId[0] = '\0';
			if ( __xgeSvgFilterSolidPaintGraphForStyle(pSvg, pStyle->sFilterId, &tSolidPaintStyle, 1) ) {
				__xgeSvgApplyShapeStyle(pSvg, pShape, &tSolidPaintStyle);
				if ( (iFilterRegion > 0) && !__xgeSvgShapeClipRectIntersect(pShape, tFilterRegion) ) {
					xgeShapeExDestroy(pShape);
					return XGE_OK;
				}
				return __xgeSvgAddStyledShape(pSvg, pShape, &tSolidPaintStyle, iDefIndex);
			}
		}
		if ( bHasFilterBounds &&
		     __xgeSvgFilterFloodForBounds(pSvg, pStyle->sFilterId, pStyle, tBounds, tFilterRegion, &iFloodColor, &fFloodOpacity, &tFilterOutputRegion) ) {
			iRet = __xgeSvgAddFilterFloodOutput(pSvg, pStyle, tFilterOutputRegion, iFloodColor, fFloodOpacity, iDefIndex);
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		if ( bHasFilterBounds &&
		     __xgeSvgFilterImageForBounds(pSvg, pStyle->sFilterId, tBounds, tFilterRegion, &sFilterImageHref, &sFilterImageAspect, &tFilterOutputRegion) ) {
			iRet = __xgeSvgAddFilterImageOutput(pSvg, pStyle, sFilterImageHref, sFilterImageAspect, tFilterOutputRegion, iDefIndex);
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		if ( bHasFilterBounds &&
		     __xgeSvgFilterIndependentColorGraphForStyle(pSvg, pStyle->sFilterId, pStyle, tBounds, tFilterRegion, &iFloodColor, &tFilterOutputRegion) ) {
			iRet = __xgeSvgAddFilterFloodOutput(pSvg, pStyle, tFilterOutputRegion, iFloodColor, 1.0f, iDefIndex);
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		if ( bHasFilterBounds &&
		     __xgeSvgFilterMorphologyDilateForBounds(pSvg, pStyle->sFilterId, tBounds, &fMorphRadiusX, &fMorphRadiusY, &bMorphSourceAlpha) ) {
			xge_svg_style_t tMorphStyle;
			float fMorphRadius;

			fMorphRadius = fMorphRadiusX > fMorphRadiusY ? fMorphRadiusX : fMorphRadiusY;
			if ( bMorphSourceAlpha || ((pStyle->sFillGradientId[0] == '\0') && (pStyle->sStrokeGradientId[0] == '\0')) ) {
				tMorphStyle = *pStyle;
				tMorphStyle.sFilterId[0] = '\0';
				tMorphStyle.sFillGradientId[0] = '\0';
				tMorphStyle.sStrokeGradientId[0] = '\0';
				tMorphStyle.iDashCount = 0;
				tMorphStyle.fDashOffset = 0.0f;
				if ( __xgeSvgApplyMorphologyDilatePaintToShape(pShape, pStyle, bMorphSourceAlpha, fMorphRadius) ) {
					if ( (iFilterRegion > 0) && !__xgeSvgShapeClipRectIntersect(pShape, tFilterRegion) ) {
						xgeShapeExDestroy(pShape);
						return XGE_OK;
					}
					return __xgeSvgAddStyledShape(pSvg, pShape, &tMorphStyle, iDefIndex);
				}
			}
		}
		if ( bHasFilterBounds &&
		     __xgeSvgFilterMorphologyErodeForBounds(pSvg, pStyle->sFilterId, tBounds, &fMorphRadiusX, &fMorphRadiusY, &bMorphSourceAlpha) ) {
			xge_svg_style_t tMorphStyle;

			if ( bMorphSourceAlpha || ((pStyle->sFillGradientId[0] == '\0') && (pStyle->sStrokeGradientId[0] == '\0')) ) {
				tMorphStyle = *pStyle;
				tMorphStyle.sFilterId[0] = '\0';
				tMorphStyle.sFillGradientId[0] = '\0';
				tMorphStyle.sStrokeGradientId[0] = '\0';
				tMorphStyle.iDashCount = 0;
				tMorphStyle.fDashOffset = 0.0f;
				if ( __xgeSvgApplyMorphologyErodePaintToSimpleShape(pShape, pStyle, bMorphSourceAlpha, fMorphRadiusX, fMorphRadiusY) ) {
					if ( (iFilterRegion > 0) && !__xgeSvgShapeClipRectIntersect(pShape, tFilterRegion) ) {
						xgeShapeExDestroy(pShape);
						return XGE_OK;
					}
					return __xgeSvgAddStyledShape(pSvg, pShape, &tMorphStyle, iDefIndex);
				}
			}
		}
		if ( bHasFilterBounds &&
		     __xgeSvgFilterGaussianBlurOffsetForBoundsEx(pSvg, pStyle->sFilterId, tBounds, &fBlurX, &fBlurY, &fBlurOffsetX, &fBlurOffsetY, &bBlurSourceAlpha) ) {
			xge_svg_style_t tBlurStyle;
			int iTap;

			tBlurStyle = *pStyle;
			tBlurStyle.sFilterId[0] = '\0';
			if ( bBlurSourceAlpha ) {
				tBlurStyle.sFillGradientId[0] = '\0';
				tBlurStyle.sStrokeGradientId[0] = '\0';
			}
			for ( iTap = 0; iTap < 9; iTap++ ) {
				xge_shape_ex pBlur;
				xge_shape_ex_matrix_t tOffset;
				xge_shape_ex_matrix_t tTransform;
				float fTapX;
				float fTapY;
				float fTapWeight;

				if ( !__xgeSvgGaussianBlurTap(iTap, fBlurX, fBlurY, &fTapX, &fTapY, &fTapWeight) || (fTapWeight <= 0.0f) ) {
					continue;
				}
				iRet = xgeShapeExClone(pShape, &pBlur);
				if ( iRet != XGE_OK ) {
					xgeShapeExDestroy(pShape);
					return iRet;
				}
				xgeShapeExOpacity(pBlur, pBlur->fOpacity * fTapWeight);
				if ( bBlurSourceAlpha ) {
					__xgeSvgApplyDropShadowPaintToShape(pBlur, pStyle, XGE_COLOR_RGBA(0, 0, 0, 255), 1.0f);
				}
				tOffset = __xgeSvgMatrixTranslate(fBlurOffsetX + fTapX, fBlurOffsetY + fTapY);
				tTransform = __xgeSvgMatrixMul(tOffset, pBlur->tTransform);
				iRet = xgeShapeExTransformSet(pBlur, &tTransform);
				if ( iRet != XGE_OK ) {
					xgeShapeExDestroy(pBlur);
					xgeShapeExDestroy(pShape);
					return iRet;
				}
				if ( (iFilterRegion > 0) && !__xgeSvgShapeClipRectIntersect(pBlur, tFilterRegion) ) {
					xgeShapeExDestroy(pBlur);
					continue;
				}
				iRet = __xgeSvgAddStyledShape(pSvg, pBlur, &tBlurStyle, iDefIndex);
				if ( iRet != XGE_OK ) {
					xgeShapeExDestroy(pShape);
					return iRet;
				}
			}
			xgeShapeExDestroy(pShape);
			return XGE_OK;
		}
		if ( bHasFilterBounds &&
		     __xgeSvgFilterDropShadowForBounds(pSvg, pStyle->sFilterId, pStyle, tBounds, &fOffsetX, &fOffsetY, &iShadowColor, &fShadowOpacity, &fShadowStdDeviation) ) {
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
		     __xgeSvgFilterOffsetForBoundsEx(pSvg, pStyle->sFilterId, tBounds, &fOffsetX, &fOffsetY, &bOffsetSourceAlpha) ) {
			xge_svg_style_t tFilterStyle;
			xge_shape_ex_matrix_t tOffset;
			xge_shape_ex_matrix_t tTransform;

			tFilterStyle = *pStyle;
			tFilterStyle.sFilterId[0] = '\0';
			if ( bOffsetSourceAlpha ) {
				tFilterStyle.sFillGradientId[0] = '\0';
				tFilterStyle.sStrokeGradientId[0] = '\0';
			}
			tOffset = __xgeSvgMatrixTranslate(fOffsetX, fOffsetY);
			tTransform = __xgeSvgMatrixMul(tOffset, pShape->tTransform);
			iRet = xgeShapeExTransformSet(pShape, &tTransform);
			if ( iRet != XGE_OK ) {
				xgeShapeExDestroy(pShape);
				return iRet;
			}
			if ( bOffsetSourceAlpha ) {
				__xgeSvgApplyDropShadowPaintToShape(pShape, pStyle, XGE_COLOR_RGBA(0, 0, 0, 255), 1.0f);
			}
			if ( (iFilterRegion > 0) && !__xgeSvgShapeClipRectIntersect(pShape, tFilterRegion) ) {
				xgeShapeExDestroy(pShape);
				return XGE_OK;
			}
			return __xgeSvgAddStyledShape(pSvg, pShape, &tFilterStyle, iDefIndex);
		}
	}
	if ( !__xgeSvgApplyViewportClipToShape(pShape, pStyle) ) {
		xgeShapeExDestroy(pShape);
		return XGE_OK;
	}
	if ( (pStyle != NULL) && pStyle->bViewportClip ) {
		tViewportClippedStyle = *pStyle;
		tViewportClippedStyle.bViewportClip = 0;
		pStyle = &tViewportClippedStyle;
	}
	pClip = (pStyle != NULL) ? __xgeSvgClipPathGetById(pSvg, pStyle->sClipId) : NULL;
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
	xge_shape_ex_matrix_t tClipMatrix;
	xge_shape_ex_matrix_t tViewportMatrix;
	xge_rect_t tMarkerClip;
	xge_vec2_t tRefPoint;
	float fOrient;
	float fUnitsScale;
	int bUseClip;
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
	tClipMatrix = __xgeSvgMatrixTranslate(tPoint.fX, tPoint.fY);
	tClipMatrix = __xgeSvgMatrixMul(tClipMatrix, __xgeSvgMatrixRotate(fOrient));
	if ( pMarker->iUnits == XGE_SVG_MARKER_UNITS_STROKE_WIDTH ) {
		tClipMatrix = __xgeSvgMatrixMul(tClipMatrix, __xgeSvgMatrixScale(fUnitsScale, fUnitsScale));
	}
	tClipMatrix = __xgeSvgMatrixMul(tClipMatrix, __xgeSvgMatrixTranslate(-tRefPoint.fX, -tRefPoint.fY));
	tClipMatrix = __xgeSvgMatrixMul(pStyle->tTransform, tClipMatrix);
	tMarkerClip.fX = 0.0f;
	tMarkerClip.fY = 0.0f;
	tMarkerClip.fW = pMarker->fWidth;
	tMarkerClip.fH = pMarker->fHeight;
	tMarkerClip = __xgeSvgMatrixRectBounds(tClipMatrix, tMarkerClip);
	bUseClip = !pDef->bOverflowVisible && (tMarkerClip.fW > 0.0f) && (tMarkerClip.fH > 0.0f);
	tMatrix = __xgeSvgMatrixMul(tClipMatrix, tViewportMatrix);
	for ( i = 0; i < pDef->iShapeCount; i++ ) {
		xge_shape_ex pClone;
		xge_shape_ex_matrix_t tShapeTransform;

		iRet = xgeShapeExClone(pDef->pShapes[i], &pClone);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		tShapeTransform = __xgeSvgMatrixMul(tMatrix, pClone->tTransform);
		if ( bUseClip ) {
			xge_rect_t tShapeClip;

			if ( !__xgeSvgClipRectMapToItem(tShapeTransform, tMarkerClip, pClone->bClipRect, pClone->tClipRect, &tShapeClip) ) {
				xgeShapeExDestroy(pClone);
				continue;
			}
			xgeShapeExClipRectSet(pClone, tShapeClip);
		}
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
		if ( bUseClip ) {
			xge_rect_t tTextClip;

			if ( !__xgeSvgClipRectMapToItem(tText.tStyle.tTransform, tMarkerClip, tText.bClipRect, tText.tClipRect, &tTextClip) ) {
				continue;
			}
			tText.bClipRect = 1;
			tText.tClipRect = tTextClip;
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
		if ( bUseClip ) {
			xge_rect_t tImageClip;

			if ( !__xgeSvgClipRectMapToItem(tImage.tStyle.tTransform, tMarkerClip, tImage.bClipRect, tImage.tClipRect, &tImageClip) ) {
				continue;
			}
			tImage.bClipRect = 1;
			tImage.tClipRect = tImageClip;
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
		if ( bUseClip ) {
			xge_rect_t tImageClip;

			if ( !__xgeSvgClipRectMapToItem(tImage.tStyle.tTransform, tMarkerClip, tImage.bClipRect, tImage.tClipRect, &tImageClip) ) {
				continue;
			}
			tImage.bClipRect = 1;
			tImage.tClipRect = tImageClip;
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

static float __xgeSvgMarkerQuadStartAngle(xge_vec2_t tP0, xge_vec2_t tP1, xge_vec2_t tP2, float fFallback)
{
	float fAngle = __xgeSvgMarkerAngleBetween(tP0, tP1, fFallback);
	if ( hypotf(tP1.fX - tP0.fX, tP1.fY - tP0.fY) > 0.000001f ) return fAngle;
	return __xgeSvgMarkerAngleBetween(tP0, tP2, fFallback);
}

static float __xgeSvgMarkerQuadEndAngle(xge_vec2_t tP0, xge_vec2_t tP1, xge_vec2_t tP2, float fFallback)
{
	float fAngle = __xgeSvgMarkerAngleBetween(tP1, tP2, fFallback);
	if ( hypotf(tP2.fX - tP1.fX, tP2.fY - tP1.fY) > 0.000001f ) return fAngle;
	return __xgeSvgMarkerAngleBetween(tP0, tP2, fFallback);
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
		} else if ( (iCommand == XGE_SHAPE_EX_CMD_QUAD_TO) && bHasCurrent ) {
			xge_vec2_t tP0 = tCurrent;
			xge_vec2_t tP1 = pShape->pPoints[iPointRead++];
			xge_vec2_t tP2 = pShape->pPoints[iPointRead++];
			float fOutAngle;
			float fInAngle;

			if ( iCount <= 0 ) {
				iRet = __xgeSvgMarkerVertexAdd(&pVertices, &iCount, &iCapacity, tCurrent);
				if ( iRet != XGE_OK ) {
					xrtFree(pVertices);
					return iRet;
				}
			}
			fOutAngle = __xgeSvgMarkerQuadStartAngle(tP0, tP1, tP2, fLastAngle);
			fInAngle = __xgeSvgMarkerQuadEndAngle(tP0, tP1, tP2, fOutAngle);
			pVertices[iCount - 1].fOutAngle = fOutAngle;
			pVertices[iCount - 1].bHasOutAngle = 1;
			iRet = __xgeSvgMarkerVertexAdd(&pVertices, &iCount, &iCapacity, tP2);
			if ( iRet != XGE_OK ) {
				xrtFree(pVertices);
				return iRet;
			}
			pVertices[iCount - 1].fInAngle = fInAngle;
			pVertices[iCount - 1].bHasInAngle = 1;
			tCurrent = tP2;
			fLastAngle = fInAngle;
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

static int __xgeSvgPathMarkerShapeEvent(void* pUser, uint8_t iCommand, xge_vec2_t tStart, xge_vec2_t tControl1, xge_vec2_t tControl2, xge_vec2_t tEnd)
{
	xge_shape_ex pShape = (xge_shape_ex)pUser;

	(void)tStart;
	if ( pShape == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iCommand == XGE_SHAPE_EX_CMD_MOVE_TO ) {
		return xgeShapeExMoveTo(pShape, tEnd.fX, tEnd.fY);
	}
	if ( iCommand == XGE_SHAPE_EX_CMD_LINE_TO ) {
		return xgeShapeExLineTo(pShape, tEnd.fX, tEnd.fY);
	}
	if ( iCommand == XGE_SHAPE_EX_CMD_QUAD_TO ) {
		return xgeShapeExQuadTo(pShape, tControl1.fX, tControl1.fY, tEnd.fX, tEnd.fY);
	}
	if ( iCommand == XGE_SHAPE_EX_CMD_CUBIC_TO ) {
		return xgeShapeExCubicTo(pShape, tControl1.fX, tControl1.fY, tControl2.fX, tControl2.fY, tEnd.fX, tEnd.fY);
	}
	if ( iCommand == XGE_SHAPE_EX_CMD_CLOSE ) {
		return xgeShapeExClose(pShape);
	}
	return XGE_ERROR_INVALID_ARGUMENT;
}

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

static int __xgeSvgTextPathClone(xge_svg pSvg, const char* pTag, const char* pTagEnd, xge_shape_ex* ppPath, float* pDeclaredPathLength)
{
	char sId[XGE_SVG_ID_MAX];
	int iDefIndex;

	if ( ppPath != NULL ) {
		*ppPath = NULL;
	}
	if ( pDeclaredPathLength != NULL ) {
		*pDeclaredPathLength = 0.0f;
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
	if ( (pDeclaredPathLength != NULL) && (pSvg->pDefs[iDefIndex].pShapeStyles != NULL) ) {
		*pDeclaredPathLength = pSvg->pDefs[iDefIndex].pShapeStyles[0].fPathLength;
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

static float __xgeSvgTextPathStartOffset(xge_svg pSvg, const char* pTag, const char* pTagEnd, const xge_svg_style_t* pStyle, xge_shape_ex pPath, float fDeclaredPathLength)
{
	char sValue[XGE_SVG_ATTR_MAX];
	float fPathLength;
	float fParseLength;
	float fScale;
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
	fParseLength = fPathLength;
	fScale = 1.0f;
	if ( (fDeclaredPathLength > 0.0f) && (fPathLength > 0.0f) ) {
		fParseLength = fDeclaredPathLength;
		fScale = fPathLength / fDeclaredPathLength;
	}
	fOffset = 0.0f;
	return __xgeSvgParseLengthEx(sValue, fParseLength, pStyle->fFontSize, &fOffset) ? (fOffset * fScale) : 0.0f;
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

static int __xgeSvgConditionalRequiredFeatureSupported(const char* pStart, const char* pEnd)
{
	const char* pHash;

	__xgeSvgTrimRange(&pStart, &pEnd);
	if ( pEnd <= pStart ) {
		return 1;
	}
	pHash = pEnd;
	while ( (pHash > pStart) && (pHash[-1] != '#') ) {
		pHash--;
	}
	if ( pHash <= pStart ) {
		pHash = pStart;
	}
	return __xgeSvgCssRangeEqualsText(pHash, pEnd, "SVG") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "SVG-static") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "CoreAttribute") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "Structure") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "BasicStructure") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "ContainerAttribute") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "ConditionalProcessing") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "Image") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "Style") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "Shape") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "Text") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "BasicText") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "PaintAttribute") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "OpacityAttribute") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "GraphicsAttribute") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "Gradient") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "Clip") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "BasicClip") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "Mask") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "Filter") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "Marker") ||
	       __xgeSvgCssRangeEqualsText(pHash, pEnd, "View");
}

static int __xgeSvgConditionalRequiredFeaturesMatch(const char* sValue)
{
	const char* p;

	if ( (sValue == NULL) || (sValue[0] == '\0') ) {
		return 1;
	}
	p = sValue;
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
		if ( (pEnd > pStart) && !__xgeSvgConditionalRequiredFeatureSupported(pStart, pEnd) ) {
			return 0;
		}
	}
	return 1;
}

static int __xgeSvgConditionalSystemLanguageMatches(const char* sValue)
{
	const char* p;

	if ( (sValue == NULL) || (sValue[0] == '\0') ) {
		return 1;
	}
	p = sValue;
	while ( *p != '\0' ) {
		const char* pStart;
		const char* pEnd;

		while ( __xgeSvgCssIsSpace(*p) || (*p == ',') ) {
			p++;
		}
		pStart = p;
		while ( (*p != '\0') && !__xgeSvgCssIsSpace(*p) && (*p != ',') ) {
			p++;
		}
		pEnd = p;
		if ( __xgeSvgCssRangeEqualsText(pStart, pEnd, "en") ||
		     __xgeSvgCssRangeEqualsText(pStart, pEnd, "en-US") ||
		     __xgeSvgCssRangeEqualsText(pStart, pEnd, "zh") ||
		     __xgeSvgCssRangeEqualsText(pStart, pEnd, "zh-CN") ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeSvgConditionalElementMatches(const char* pTag, const char* pTagEnd)
{
	char sValue[XGE_SVG_ATTR_MAX];

	if ( (pTag == NULL) || (pTagEnd == NULL) || (pTag >= pTagEnd) ) {
		return 0;
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "requiredExtensions", sValue, sizeof(sValue)) ) {
		const char* pValueStart = sValue;
		const char* pValueEnd = sValue + strlen(sValue);

		__xgeSvgTrimRange(&pValueStart, &pValueEnd);
		if ( pValueEnd > pValueStart ) {
			return 0;
		}
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "requiredFeatures", sValue, sizeof(sValue)) &&
	     !__xgeSvgConditionalRequiredFeaturesMatch(sValue) ) {
		return 0;
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "systemLanguage", sValue, sizeof(sValue)) &&
	     !__xgeSvgConditionalSystemLanguageMatches(sValue) ) {
		return 0;
	}
	return 1;
}

static int __xgeSvgElementRangeEnd(const char* pTag, const char* pTagEnd, const char* pLimit, const char** ppElementEnd)
{
	char sName[XGE_SVG_ATTR_MAX];

	if ( ppElementEnd != NULL ) {
		*ppElementEnd = NULL;
	}
	if ( (pTag == NULL) || (pTagEnd == NULL) || (pLimit == NULL) || (ppElementEnd == NULL) || (pTag >= pLimit) ) {
		return 0;
	}
	if ( __xgeSvgIsSelfClosingTag(pTag, pTagEnd) ) {
		*ppElementEnd = pTagEnd + 1;
		return 1;
	}
	if ( !__xgeSvgTagNameCopy(pTag, sName, sizeof(sName)) ) {
		return 0;
	}
	{
		const char* pCloseEnd;
		const char* pClose = __xgeSvgFindCloseElement(pTagEnd + 1, sName, &pCloseEnd);

		if ( (pClose == NULL) || (pClose >= pLimit) || (pCloseEnd == NULL) ) {
			return 0;
		}
		*ppElementEnd = pCloseEnd + 1;
		return 1;
	}
}

static int __xgeSvgSwitchSelectRange(const char* pSwitchTag, const char* pSwitchTagEnd, const char** ppSwitchEnd, const char** ppSelectedStart, const char** ppSelectedEnd)
{
	const char* pCloseEnd;
	const char* pSwitchClose;
	const char* p;

	if ( ppSwitchEnd != NULL ) *ppSwitchEnd = NULL;
	if ( ppSelectedStart != NULL ) *ppSelectedStart = NULL;
	if ( ppSelectedEnd != NULL ) *ppSelectedEnd = NULL;
	if ( (pSwitchTag == NULL) || (pSwitchTagEnd == NULL) || (ppSwitchEnd == NULL) || (ppSelectedStart == NULL) || (ppSelectedEnd == NULL) ) {
		return 0;
	}
	pSwitchClose = __xgeSvgFindCloseElement(pSwitchTagEnd + 1, "switch", &pCloseEnd);
	if ( (pSwitchClose == NULL) || (pCloseEnd == NULL) ) {
		return 0;
	}
	*ppSwitchEnd = pSwitchClose;
	p = pSwitchTagEnd + 1;
	while ( (p = strchr(p, '<')) != NULL && (p < pSwitchClose) ) {
		const char* pTagEnd = __xgeSvgFindTagEnd(p);
		const char* pElementEnd;

		if ( (pTagEnd == NULL) || (pTagEnd >= pSwitchClose) ) {
			return 0;
		}
		if ( (p[1] == '/') || (p[1] == '!') || (p[1] == '?') ) {
			p = pTagEnd + 1;
			continue;
		}
		if ( !__xgeSvgElementRangeEnd(p, pTagEnd, pSwitchClose, &pElementEnd) ) {
			pElementEnd = pTagEnd + 1;
		}
		if ( __xgeSvgConditionalElementMatches(p, pTagEnd) ) {
			*ppSelectedStart = p;
			*ppSelectedEnd = pElementEnd;
			return 1;
		}
		p = pElementEnd;
	}
	return 1;
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
				float fDeclaredPathLength;
				float fPathAdvance;
				float fPathOffset;
				int iPathGapCount;
				int iPathRunCount;
				int bPathHasTextLength;

				tPathStyle = __xgeSvgStyleResolve(pSvg, pTextStyle, pTag, pTagEnd);
				bPathHasTextLength = __xgeSvgTextLayoutParse(pSvg, pTag, pTagEnd, tPathStyle.fFontSize, &tPathLayout);
				pPathShape = NULL;
				fDeclaredPathLength = 0.0f;
				iRet = __xgeSvgTextPathClone(pSvg, pTag, pTagEnd, &pPathShape, &fDeclaredPathLength);
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
					fPathOffset = fTextPathOffset + __xgeSvgTextPathStartOffset(pSvg, pTag, pTagEnd, &tPathStyle, pPathShape, fDeclaredPathLength);
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

static int __xgeSvgRangeStartsWithTextNoCase(const char* pStart, const char* pEnd, const char* sText)
{
	while ( *sText != '\0' ) {
		if ( pStart >= pEnd ) {
			return 0;
		}
		if ( __xgeSvgAsciiLower((unsigned char)*pStart) != __xgeSvgAsciiLower((unsigned char)*sText) ) {
			return 0;
		}
		pStart++;
		sText++;
	}
	return 1;
}

static int __xgeSvgDataImagePayload(const char* sHref, const char* sMime, int bAllowUtf8, int bAllowBase64, const char** ppPayload, int* pEncoding)
{
	const char* sPrefix = "data:image/";
	const char* pEnd;
	const char* pMimeStart;
	const char* pMimeEnd;
	const char* pComma;
	const char* p;
	size_t iPrefixLen;
	int bHasBase64;

	if ( (sHref == NULL) || (sMime == NULL) || (ppPayload == NULL) || (pEncoding == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppPayload = NULL;
	*pEncoding = 0;
	pEnd = sHref + strlen(sHref);
	iPrefixLen = strlen(sPrefix);
	if ( !__xgeSvgRangeStartsWithTextNoCase(sHref, pEnd, sPrefix) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	pMimeStart = sHref + iPrefixLen;
	pMimeEnd = pMimeStart;
	while ( (pMimeEnd < pEnd) && (*pMimeEnd != ';') && (*pMimeEnd != ',') ) {
		pMimeEnd++;
	}
	if ( !__xgeSvgCssRangeEqualsText(pMimeStart, pMimeEnd, sMime) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	pComma = strchr(pMimeEnd, ',');
	if ( pComma == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pMimeEnd == pComma ) {
		if ( !bAllowUtf8 ) {
			return XGE_ERROR_UNSUPPORTED;
		}
		*ppPayload = pComma + 1;
		*pEncoding = XGE_SVG_DATA_IMAGE_UTF8;
		return XGE_OK;
	}
	bHasBase64 = 0;
	p = pMimeEnd;
	while ( p < pComma ) {
		const char* pTokenStart;
		const char* pTokenEnd;

		if ( *p != ';' ) {
			return XGE_ERROR_UNSUPPORTED;
		}
		p++;
		pTokenStart = p;
		while ( (p < pComma) && (*p != ';') ) {
			p++;
		}
		pTokenEnd = p;
		if ( bAllowBase64 && __xgeSvgCssRangeEqualsText(pTokenStart, pTokenEnd, "base64") ) {
			bHasBase64 = 1;
		}
	}
	if ( bHasBase64 ) {
		*ppPayload = pComma + 1;
		*pEncoding = XGE_SVG_DATA_IMAGE_BASE64;
		return XGE_OK;
	}
	if ( bAllowUtf8 ) {
		*ppPayload = pComma + 1;
		*pEncoding = XGE_SVG_DATA_IMAGE_UTF8;
		return XGE_OK;
	}
	return XGE_ERROR_UNSUPPORTED;
}

static int __xgeSvgIsDataImageUri(const char* sHref)
{
	if ( sHref == NULL ) {
		return 0;
	}
	return __xgeSvgRangeStartsWithTextNoCase(sHref, sHref + strlen(sHref), "data:image/");
}

static int __xgeSvgDecodeSvgDataUri(const char* sHref, char** ppSvgText, int* pSize)
{
	const char* pPayload;
	int iEncoding;
	int iRet;

	if ( (sHref == NULL) || (ppSvgText == NULL) || (pSize == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppSvgText = NULL;
	*pSize = 0;
	iEncoding = 0;
	pPayload = NULL;
	iRet = __xgeSvgDataImagePayload(sHref, "svg+xml", 1, 1, &pPayload, &iEncoding);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( iEncoding == XGE_SVG_DATA_IMAGE_BASE64 ) {
		unsigned char* pData;
		int iRet;

		pData = NULL;
		iRet = __xgeSvgDecodeBase64(pPayload, &pData, pSize);
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
	return __xgeSvgDecodeUriData(pPayload, ppSvgText, pSize);
}

static int __xgeSvgDecodeRasterImageDataUri(const char* sHref, unsigned char** ppData, int* pSize)
{
	const char* pPayload;
	int iEncoding;
	int iRet;

	if ( (sHref == NULL) || (ppData == NULL) || (pSize == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppData = NULL;
	*pSize = 0;
	pPayload = NULL;
	iEncoding = 0;
	iRet = __xgeSvgDataImagePayload(sHref, "png", 0, 1, &pPayload, &iEncoding);
	if ( iRet == XGE_ERROR_UNSUPPORTED ) iRet = __xgeSvgDataImagePayload(sHref, "jpeg", 0, 1, &pPayload, &iEncoding);
	if ( iRet == XGE_ERROR_UNSUPPORTED ) iRet = __xgeSvgDataImagePayload(sHref, "jpg", 0, 1, &pPayload, &iEncoding);
	if ( iRet == XGE_ERROR_UNSUPPORTED ) iRet = __xgeSvgDataImagePayload(sHref, "webp", 0, 1, &pPayload, &iEncoding);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	return __xgeSvgDecodeBase64(pPayload, ppData, pSize);
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
	char* sSvgText;
	int bPreferSvg;
	int bLooksSvg;
	int iDecodedSize;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (sHref == NULL) || (ppChildSvg == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppChildSvg = NULL;
	sSvgText = NULL;
	iDecodedSize = 0;
	iRet = __xgeSvgDecodeSvgDataUri(sHref, &sSvgText, &iDecodedSize);
	if ( iRet == XGE_OK ) {
		iRet = xgeSvgCreate(ppChildSvg);
		if ( iRet == XGE_OK ) {
			iRet = __xgeSvgLoadMemoryEx(*ppChildSvg, sSvgText, iDecodedSize, pSvg->sBaseDir);
		}
		xrtFree(sSvgText);
		if ( iRet != XGE_OK ) {
			xgeSvgDestroy(*ppChildSvg);
			*ppChildSvg = NULL;
		}
		return iRet;
	}
	if ( iRet != XGE_ERROR_UNSUPPORTED ) {
		if ( (iRet != XGE_ERROR_OUT_OF_MEMORY) && __xgeSvgIsDataImageUri(sHref) ) {
			return XGE_OK;
		}
		return iRet;
	}
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

static int __xgeSvgAddHrefImage(xge_svg pSvg, const xge_svg_style_t* pStyle, const char* sHref, xge_rect_t tRect, const char* sAspect, int iDefIndex)
{
	char* sSvgText;
	unsigned char* pImageData;
	xge_svg pChildSvg;
	xge_svg_raster_image_t* pRaster;
	int iSize;
	int bDataImage;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pStyle == NULL) || (sHref == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (sHref[0] == '\0') || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_OK;
	}
	if ( __xgeSvgHrefIsLocalRef(sHref) ) {
		char sId[XGE_SVG_ID_MAX];
		int iSourceDef;

		if ( !__xgeSvgHrefIdCopyFromString(sHref, sId, sizeof(sId)) ) {
			return XGE_OK;
		}
		iSourceDef = __xgeSvgDefFind(pSvg, sId);
		if ( iSourceDef < 0 ) {
			return XGE_OK;
		}
		return __xgeSvgApplyUseInstanceWithAspect(pSvg, iSourceDef, pStyle, iDefIndex, tRect.fX, tRect.fY, tRect.fW, tRect.fH, 1, 1, sAspect);
	}
	if ( __xgeSvgHrefHasFragment(sHref) ) {
		return __xgeSvgAddSvgFragmentImage(pSvg, pStyle, sHref, tRect, sAspect, iDefIndex);
	}
	bDataImage = __xgeSvgIsDataImageUri(sHref);
	sSvgText = NULL;
	iSize = 0;
	iRet = __xgeSvgDecodeSvgDataUri(sHref, &sSvgText, &iSize);
	if ( iRet == XGE_ERROR_UNSUPPORTED ) {
		pImageData = NULL;
		pRaster = NULL;
		iSize = 0;
		iRet = __xgeSvgDecodeRasterImageDataUri(sHref, &pImageData, &iSize);
		if ( iRet == XGE_ERROR_UNSUPPORTED ) {
			if ( bDataImage ) {
				return XGE_OK;
			}
			pChildSvg = NULL;
			pRaster = NULL;
			iRet = __xgeSvgLoadExternalImage(pSvg, sHref, &pChildSvg, &pRaster);
			if ( (iRet == XGE_OK) && (pRaster != NULL) ) {
				iRet = __xgeSvgAddRasterImage(pSvg, pStyle, pRaster, tRect, sAspect, iDefIndex);
			} else if ( (iRet == XGE_OK) && (pChildSvg != NULL) ) {
				if ( (sAspect != NULL) && (sAspect[0] != '\0') ) {
					iRet = xgeSvgSetPreserveAspectRatio(pChildSvg, sAspect);
				}
				if ( iRet == XGE_OK ) {
					iRet = __xgeSvgAddSvgImage(pSvg, pStyle, pChildSvg, tRect, iDefIndex);
				}
			}
			__xgeSvgRasterImageDestroy(pRaster);
			xgeSvgDestroy(pChildSvg);
			return iRet;
		}
		if ( iRet == XGE_OK ) {
			iRet = __xgeSvgRasterImageCreateFromMemory(pImageData, iSize, &pRaster);
		}
		if ( iRet != XGE_OK ) {
			xrtFree(pImageData);
			if ( bDataImage && (iRet != XGE_ERROR_OUT_OF_MEMORY) ) {
				return XGE_OK;
			}
			return iRet;
		}
		iRet = __xgeSvgAddRasterImage(pSvg, pStyle, pRaster, tRect, sAspect, iDefIndex);
		__xgeSvgRasterImageDestroy(pRaster);
		xrtFree(pImageData);
		return iRet;
	}
	if ( iRet != XGE_OK ) {
		if ( bDataImage && (iRet != XGE_ERROR_OUT_OF_MEMORY) ) {
			return XGE_OK;
		}
		return iRet;
	}
	pChildSvg = NULL;
	iRet = xgeSvgCreate(&pChildSvg);
	if ( iRet == XGE_OK ) {
		iRet = __xgeSvgLoadMemoryEx(pChildSvg, sSvgText, iSize, pSvg->sBaseDir);
	}
	if ( (iRet == XGE_OK) && (sAspect != NULL) && (sAspect[0] != '\0') ) {
		iRet = xgeSvgSetPreserveAspectRatio(pChildSvg, sAspect);
	}
	if ( iRet == XGE_OK ) {
		iRet = __xgeSvgAddSvgImage(pSvg, pStyle, pChildSvg, tRect, iDefIndex);
	}
	xgeSvgDestroy(pChildSvg);
	xrtFree(sSvgText);
	return iRet;
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
	int bDataImage;
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
	if ( !__xgeSvgHrefCopyTrimmed(pTag, pTagEnd, sHref, sizeof(sHref)) ) {
		return XGE_OK;
	}
	bDataImage = __xgeSvgIsDataImageUri(sHref);
	sAspect[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "preserveAspectRatio", sAspect, sizeof(sAspect));
	tStyle = __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd);
	tRect.fX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "x", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
	tRect.fY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "y", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
	tRect.fW = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "width", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
	tRect.fH = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "height", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
	if ( !__xgeSvgHrefIsLocalRef(sHref) && __xgeSvgHrefHasFragment(sHref) ) {
		iRet = __xgeSvgAddSvgFragmentImage(pSvg, &tStyle, sHref, tRect, sAspect, iTargetDef);
		if ( (iRet == XGE_OK) && (iTargetDef < 0) && (iOwnMainIdDefIndex >= 0) ) {
			iRet = __xgeSvgAddSvgFragmentImage(pSvg, &tStyle, sHref, tRect, sAspect, iOwnMainIdDefIndex);
		}
		if ( (iRet == XGE_OK) && (iTargetDef < 0) && (iGroupMainIdDefIndex >= 0) && (iGroupMainIdDefIndex != iOwnMainIdDefIndex) ) {
			iRet = __xgeSvgAddSvgFragmentImage(pSvg, &tStyle, sHref, tRect, sAspect, iGroupMainIdDefIndex);
		}
		return iRet;
	}
	sSvgText = NULL;
	iSize = 0;
	iRet = __xgeSvgDecodeSvgDataUri(sHref, &sSvgText, &iSize);
	if ( iRet == XGE_ERROR_UNSUPPORTED ) {
		pImageData = NULL;
		pRaster = NULL;
		iSize = 0;
		iRet = __xgeSvgDecodeRasterImageDataUri(sHref, &pImageData, &iSize);
		if ( iRet == XGE_ERROR_UNSUPPORTED ) {
			if ( bDataImage ) {
				return XGE_OK;
			}
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
		if ( iRet != XGE_OK ) {
			xrtFree(pImageData);
			if ( bDataImage && (iRet != XGE_ERROR_OUT_OF_MEMORY) ) {
				return XGE_OK;
			}
			return iRet;
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
		if ( bDataImage && (iRet != XGE_ERROR_OUT_OF_MEMORY) ) {
			return XGE_OK;
		}
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
		xge_shape_ex pMarkerShape;
		int bUseMarkerShape;

		if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "d", sValue, sizeof(sValue)) ) {
			return XGE_OK;
		}
		pMarkerShape = NULL;
		bUseMarkerShape = __xgeSvgStyleHasMarkerPaint(&tStyle);
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		if ( bUseMarkerShape ) {
			iRet = xgeShapeExCreate(&pMarkerShape);
			if ( iRet != XGE_OK ) {
				xgeShapeExDestroy(pShape);
				return iRet;
			}
			iRet = xgeShapeExInternalAppendSvgPathWithEvents(pShape, sValue, __xgeSvgPathMarkerShapeEvent, pMarkerShape);
		} else {
			iRet = xgeShapeExAppendSvgPath(pShape, sValue);
		}
		if ( iRet == XGE_OK ) {
			__xgeSvgApplyPathLengthToStyle(pShape, pTag, pTagEnd, &tStyle);
			__xgeSvgApplyShapeStyle(pSvg, pShape, &tStyle);
			tMarkerData.pShape = (pMarkerShape != NULL) ? pMarkerShape : pShape;
			iRet = __xgeSvgAddMarkedShapeWithMainIds(pSvg, pShape, &tStyle, iTargetDef, iOwnMainIdDefIndex, iGroupMainIdDefIndex, __xgeSvgApplyPathMarkersFromData, &tMarkerData);
			xgeShapeExDestroy(pMarkerShape);
			return iRet;
		}
		xgeShapeExDestroy(pMarkerShape);
		xgeShapeExDestroy(pShape);
		return (iRet == XGE_ERROR_INVALID_ARGUMENT) ? XGE_OK : iRet;
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

		__xgeSvgNormalizeRectRadii(&fRX, &fRY, &bHasRX, &bHasRY);
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
		int bEllipse = __xgeSvgTagNameEquals(pTag, "ellipse");
		float fCX = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "cx", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fCY = __xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "cy", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		float fRX;
		float fRY;

		__xgeSvgCircleEllipseRadii(pSvg, pTag, pTagEnd, 0, tStyle.fFontSize, &fRX, &fRY);
		if ( (fRX <= 0.0f) || (fRY <= 0.0f) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = bEllipse ? xgeShapeExAppendEllipse(pShape, fCX, fCY, fRX, fRY, 1) :
		                   xgeShapeExAppendCircle(pShape, fCX, fCY, fRX, fRY, 1);
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
		iRet = xgeShapeExAppendLine(pShape, fX1, fY1, fX2, fY2);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
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
	if ( pSourceStyle->bBlendSet ) {
		pAddStyle->iBlend = pSourceStyle->iBlend;
		pAddStyle->bBlendSet = 1;
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
	if ( !pTargetStyle->bBlendSet && pUseStyle->bBlendSet ) {
		pTargetStyle->iBlend = pUseStyle->iBlend;
		pTargetStyle->bBlendSet = 1;
	}
}

static int __xgeSvgApplyUseInstance(xge_svg pSvg, int iDefIndex, const xge_svg_style_t* pUseStyle, int iTargetDef, float fX, float fY, float fW, float fH, int bHasWidth, int bHasHeight)
{
	xge_svg_style_t tStyle;
	xge_shape_ex_matrix_t tUseTransform;
	xge_rect_t tUseClip;
	int iSourceShapeCount;
	int bUseClip;
	int i;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pUseStyle == NULL) || (iDefIndex < 0) || (iDefIndex >= pSvg->iDefCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tStyle = *pUseStyle;
	bUseClip = 0;
	memset(&tUseClip, 0, sizeof(tUseClip));
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
		if ( !pSvg->pDefs[iDefIndex].bOverflowVisible ) {
			tUseClip = __xgeSvgMatrixRectBounds(pUseStyle->tTransform, tDst);
			bUseClip = (tUseClip.fW > 0.0f) && (tUseClip.fH > 0.0f);
		}
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
		if ( bUseClip ) {
			xge_rect_t tShapeClip;

			if ( !__xgeSvgClipRectMapToItem(tShapeTransform, tUseClip, pClone->bClipRect, pClone->tClipRect, &tShapeClip) ) {
				xgeShapeExDestroy(pClone);
				continue;
			}
			xgeShapeExClipRectSet(pClone, tShapeClip);
		}
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
		if ( bUseClip ) {
			xge_rect_t tTextClip;

			if ( !__xgeSvgClipRectMapToItem(tText.tStyle.tTransform, tUseClip, tText.bClipRect, tText.tClipRect, &tTextClip) ) {
				continue;
			}
			tText.bClipRect = 1;
			tText.tClipRect = tTextClip;
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
		if ( bUseClip ) {
			xge_rect_t tImageClip;

			if ( !__xgeSvgClipRectMapToItem(tImage.tStyle.tTransform, tUseClip, tImage.bClipRect, tImage.tClipRect, &tImageClip) ) {
				continue;
			}
			tImage.bClipRect = 1;
			tImage.tClipRect = tImageClip;
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
		if ( bUseClip ) {
			xge_rect_t tImageClip;

			if ( !__xgeSvgClipRectMapToItem(tImage.tStyle.tTransform, tUseClip, tImage.bClipRect, tImage.tClipRect, &tImageClip) ) {
				continue;
			}
			tImage.bClipRect = 1;
			tImage.tClipRect = tImageClip;
		}
		iRet = __xgeSvgAddRasterItemMasked(pSvg, &tImage, iTargetDef);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	return XGE_OK;
}

static int __xgeSvgApplyUseInstanceWithAspect(xge_svg pSvg, int iDefIndex, const xge_svg_style_t* pUseStyle, int iTargetDef, float fX, float fY, float fW, float fH, int bHasWidth, int bHasHeight, const char* sAspect)
{
	int iOldAspectAlignX;
	int iOldAspectAlignY;
	int iOldAspectMeetOrSlice;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (iDefIndex < 0) || (iDefIndex >= pSvg->iDefCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (sAspect == NULL) || (sAspect[0] == '\0') || !pSvg->pDefs[iDefIndex].bHasViewBox ) {
		return __xgeSvgApplyUseInstance(pSvg, iDefIndex, pUseStyle, iTargetDef, fX, fY, fW, fH, bHasWidth, bHasHeight);
	}
	iOldAspectAlignX = pSvg->pDefs[iDefIndex].iAspectAlignX;
	iOldAspectAlignY = pSvg->pDefs[iDefIndex].iAspectAlignY;
	iOldAspectMeetOrSlice = pSvg->pDefs[iDefIndex].iAspectMeetOrSlice;
	__xgeSvgParsePreserveAspectRatioFields(sAspect,
		&pSvg->pDefs[iDefIndex].iAspectAlignX,
		&pSvg->pDefs[iDefIndex].iAspectAlignY,
		&pSvg->pDefs[iDefIndex].iAspectMeetOrSlice);
	iRet = __xgeSvgApplyUseInstance(pSvg, iDefIndex, pUseStyle, iTargetDef, fX, fY, fW, fH, bHasWidth, bHasHeight);
	pSvg->pDefs[iDefIndex].iAspectAlignX = iOldAspectAlignX;
	pSvg->pDefs[iDefIndex].iAspectAlignY = iOldAspectAlignY;
	pSvg->pDefs[iDefIndex].iAspectMeetOrSlice = iOldAspectMeetOrSlice;
	return iRet;
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
	int bDataImage;
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
	bDataImage = __xgeSvgIsDataImageUri(sHref);
	iRet = __xgeSvgDecodeSvgDataUri(sHref, &sSvgText, &iSize);
	if ( iRet == XGE_OK ) {
		iRet = xgeSvgCreate(&pChildSvg);
		if ( iRet == XGE_OK ) {
			iRet = __xgeSvgLoadMemoryEx(pChildSvg, sSvgText, iSize, pSvg->sBaseDir);
		}
		xrtFree(sSvgText);
	} else if ( iRet == XGE_ERROR_UNSUPPORTED ) {
		if ( bDataImage ) {
			return XGE_OK;
		}
		iRet = __xgeSvgLoadExternalImage(pSvg, sHref, &pChildSvg, &pRaster);
		__xgeSvgRasterImageDestroy(pRaster);
		pRaster = NULL;
	} else if ( bDataImage && (iRet != XGE_ERROR_OUT_OF_MEMORY) ) {
		iRet = XGE_OK;
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

static int __xgeSvgAddSvgFragmentImage(xge_svg pSvg, const xge_svg_style_t* pStyle, const char* sHref, xge_rect_t tRect, const char* sAspect, int iDefIndex)
{
	char sUri[XGE_SVG_ATTR_MAX];
	char sId[XGE_SVG_ATTR_MAX];
	xge_svg pChildSvg;
	xge_svg_style_t tFragmentStyle;
	xge_rect_t tIntrinsic;
	int iSourceDef;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pStyle == NULL) || (sHref == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_OK;
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
	iSourceDef = __xgeSvgDefFind(pChildSvg, sId);
	if ( iSourceDef < 0 ) {
		xgeSvgDestroy(pChildSvg);
		return XGE_OK;
	}
	if ( (sAspect != NULL) && (sAspect[0] != '\0') ) {
		iRet = xgeSvgSetPreserveAspectRatio(pChildSvg, sAspect);
		if ( iRet != XGE_OK ) {
			xgeSvgDestroy(pChildSvg);
			return iRet;
		}
	}
	tIntrinsic = __xgeSvgChildIntrinsicRect(pChildSvg);
	if ( pChildSvg->pDefs[iSourceDef].bHasViewBox ) {
		tIntrinsic = pChildSvg->pDefs[iSourceDef].tViewBox;
	}
	pChildSvg->tViewBox = tIntrinsic;
	pChildSvg->bHasViewBox = ((tIntrinsic.fW > 0.0f) && (tIntrinsic.fH > 0.0f)) ? 1 : 0;
	pChildSvg->fWidth = tIntrinsic.fW;
	pChildSvg->fHeight = tIntrinsic.fH;
	__xgeSvgDrawItemsClear(pChildSvg);
	tFragmentStyle = __xgeSvgStyleDefault();
	if ( pChildSvg->pDefs[iSourceDef].bHasViewBox ) {
		iRet = __xgeSvgApplyUseInstanceWithAspect(pChildSvg, iSourceDef, &tFragmentStyle, -1, tIntrinsic.fX, tIntrinsic.fY, tIntrinsic.fW, tIntrinsic.fH, 1, 1, sAspect);
	} else {
		iRet = __xgeSvgApplyUseInstance(pChildSvg, iSourceDef, &tFragmentStyle, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0);
	}
	if ( iRet == XGE_OK ) {
		iRet = __xgeSvgAddSvgImage(pSvg, pStyle, pChildSvg, tRect, iDefIndex);
	}
	xgeSvgDestroy(pChildSvg);
	return iRet;
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
		return XGE_OK;
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
		int iUnits;
		if ( __xgeSvgParseGradientUnits(sValue, &iUnits) ) {
			pSvg->pLinearGradients[iIndex].iUnits = iUnits;
			pSvg->pLinearGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_UNITS;
		}
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
		int iSpread;
		if ( __xgeSvgParseGradientSpread(sValue, &iSpread) ) {
			pSvg->pLinearGradients[iIndex].iSpread = iSpread;
			pSvg->pLinearGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_SPREAD;
		}
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
	pSvg->pRadialGradients[iIndex].fFR = 0.0f;
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
		int iUnits;
		if ( __xgeSvgParseGradientUnits(sValue, &iUnits) ) {
			pSvg->pRadialGradients[iIndex].iUnits = iUnits;
			pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_UNITS;
		}
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
	if ( __xgeSvgGradientLengthCopy(pSvg, pTag, pTagEnd, "fr", pSvg->pRadialGradients[iIndex].iUnits, XGE_SVG_LENGTH_BASIS_OTHER, pSvg->pRadialGradients[iIndex].tStyle.fFontSize, &pSvg->pRadialGradients[iIndex].fFR) ) {
		pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_FR;
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
		int iSpread;
		if ( __xgeSvgParseGradientSpread(sValue, &iSpread) ) {
			pSvg->pRadialGradients[iIndex].iSpread = iSpread;
			pSvg->pRadialGradients[iIndex].iFlags |= XGE_SVG_GRADIENT_HAS_SPREAD;
		}
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
	typedef struct xge_svg_stop_parse_state_t {
		float fOffset;
		uint32_t iColor;
		float fOpacity;
		uint32_t iCurrentColor;
		int bStyleStopColorSet;
		int bStyleStopOpacitySet;
	} xge_svg_stop_parse_state_t;
	xge_svg_stop_parse_state_t tState;
	xge_svg_style_t tBaseStyle;
	const char* pCursor;
	char sName[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	char sStyleValue[XGE_SVG_ATTR_MAX];
	float fValue;
	uint32_t iColor;

	if ( !__xgeSvgValid(pSvg) || (pTag == NULL) || (pTagEnd == NULL) || (pStop == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tBaseStyle = pGradientStyle != NULL ? *pGradientStyle : __xgeSvgStyleDefault();
	__xgeSvgStyleResetStopPaint(&tBaseStyle);
	tState.fOffset = 0.0f;
	tState.iColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	tState.fOpacity = 1.0f;
	tState.iCurrentColor = tBaseStyle.iCurrentColor;
	tState.bStyleStopColorSet = 0;
	tState.bStyleStopOpacitySet = 0;
	pCursor = pTag;
	while ( __xgeSvgAttrNext(&pCursor, pTagEnd, sName, sizeof(sName), sValue, sizeof(sValue)) ) {
		if ( strcmp(sName, "offset") == 0 ) {
			tState.fOffset = __xgeSvgParseStopOffset(sValue);
		} else if ( strcmp(sName, "stop-opacity") == 0 ) {
			if ( !tState.bStyleStopOpacitySet && __xgeSvgParseOpacity(sValue, &fValue) ) {
				tState.fOpacity = fValue;
			}
		} else if ( strcmp(sName, "stop-color") == 0 ) {
			if ( __xgeSvgColorNameEquals(sValue, "currentColor") ) {
				tState.iColor = tState.iCurrentColor;
			} else if ( !tState.bStyleStopColorSet ) {
				iColor = tState.iColor;
				if ( __xgeSvgParseColor(sValue, &iColor) ) {
					tState.iColor = iColor;
				}
			}
		} else if ( strcmp(sName, "style") == 0 ) {
			const char* pStyle;

			pStyle = sValue;
			while ( *pStyle != '\0' ) {
				const char* pNameStart;
				const char* pNameEnd;
				const char* pValueStart;
				const char* pValueEnd;
				int bImportant;

				for ( ;; ) {
					while ( __xgeSvgCssIsSpace(*pStyle) || (*pStyle == ';') ) pStyle++;
					if ( (pStyle[0] == '/') && (pStyle[1] == '*') ) {
						pStyle += 2;
						while ( (pStyle[0] != '\0') && !((pStyle[0] == '*') && (pStyle[1] == '/')) ) pStyle++;
						if ( pStyle[0] != '\0' ) pStyle += 2;
						continue;
					}
					break;
				}
				if ( *pStyle == '\0' ) {
					break;
				}
				pNameStart = pStyle;
				while ( (*pStyle != '\0') && (*pStyle != ':') && (*pStyle != ';') ) pStyle++;
				pNameEnd = pStyle;
				__xgeSvgTrimRange(&pNameStart, &pNameEnd);
				if ( *pStyle != ':' ) {
					while ( (*pStyle != '\0') && (*pStyle != ';') ) pStyle++;
					continue;
				}
				pStyle++;
				pValueStart = pStyle;
				while ( (*pStyle != '\0') && (*pStyle != ';') ) pStyle++;
				pValueEnd = pStyle;
				if ( !__xgeSvgStyleValueCleanCopy(pValueStart, pValueEnd, sStyleValue, sizeof(sStyleValue), &bImportant) ) {
					continue;
				}
				(void)bImportant;
				if ( __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "stop-opacity") ) {
					if ( __xgeSvgParseOpacity(sStyleValue, &fValue) ) {
						tState.fOpacity = fValue;
						tState.bStyleStopOpacitySet = 1;
					}
				} else if ( __xgeSvgCssRangeEqualsText(pNameStart, pNameEnd, "stop-color") ) {
					if ( __xgeSvgColorNameEquals(sStyleValue, "currentColor") ) {
						tState.iColor = tState.iCurrentColor;
					} else {
						iColor = tState.iColor;
						if ( __xgeSvgParseColor(sStyleValue, &iColor) ) {
							tState.iColor = iColor;
							tState.bStyleStopColorSet = 1;
						}
					}
				}
			}
		}
	}
	pStop->fOffset = tState.fOffset;
	pStop->iColor = __xgeSvgColorAlpha(tState.iColor, tState.fOpacity);
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

static int __xgeSvgOverflowIsVisible(const char* pTag, const char* pTagEnd)
{
	char sStyle[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];
	const char* pStart;
	const char* pEnd;

	if ( (pTag == NULL) || (pTagEnd == NULL) ) {
		return 0;
	}
	sStyle[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "style", sStyle, sizeof(sStyle));
	if ( !__xgeSvgAttrOrStyleCopy(pTag, pTagEnd, sStyle, "overflow", sValue, sizeof(sValue)) ) {
		return 0;
	}
	pStart = sValue;
	pEnd = sValue + strlen(sValue);
	while ( (pStart < pEnd) && __xgeSvgIsSpace((unsigned char)*pStart) ) pStart++;
	while ( (pEnd > pStart) && __xgeSvgIsSpace((unsigned char)pEnd[-1]) ) pEnd--;
	return __xgeSvgCssRangeEqualsText(pStart, pEnd, "visible");
}

static int __xgeSvgOverflowIsSpecified(const char* pTag, const char* pTagEnd)
{
	char sStyle[XGE_SVG_ATTR_MAX];
	char sValue[XGE_SVG_ATTR_MAX];

	if ( (pTag == NULL) || (pTagEnd == NULL) ) {
		return 0;
	}
	sStyle[0] = '\0';
	__xgeSvgAttrCopy(pTag, pTagEnd, "style", sStyle, sizeof(sStyle));
	return __xgeSvgAttrOrStyleCopy(pTag, pTagEnd, sStyle, "overflow", sValue, sizeof(sValue));
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
	if ( (tDst.fW > 0.0f) && (tDst.fH > 0.0f) && !__xgeSvgOverflowIsVisible(pTag, pTagEnd) ) {
		xge_rect_t tClip;

		tClip = __xgeSvgMatrixRectBounds(pStyle->tTransform, tDst);
		if ( pStyle->bViewportClip ) {
			tClip = __xgeSvgRectIntersect(pStyle->tViewportClip, tClip);
		}
		pStyle->bViewportClip = 1;
		pStyle->tViewportClip = tClip;
	}
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

static xge_svg_style_t __xgeSvgResolveClipMaskContainerStyle(xge_svg pSvg, const xge_svg_style_t* pParentStyle, const char* pTag, const char* pTagEnd)
{
	xge_svg_style_t tStyle;

	tStyle = __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd);
	if ( __xgeSvgTagNameEquals(pTag, "svg") ) {
		__xgeSvgApplyNestedSvgViewport(pSvg, &tStyle, pTag, pTagEnd);
	}
	return tStyle;
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
	pSvg->pDefs[iDefIndex].bOverflowVisible = 0;
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
	if ( __xgeSvgOverflowIsVisible(pTag, pTagEnd) ) {
		pSvg->pDefs[iDefIndex].bOverflowVisible = 1;
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
		if ( __xgeSvgColorNameEquals(sValue, "auto") ) {
			pMarker->bOrientAuto = 1;
		} else if ( __xgeSvgColorNameEquals(sValue, "auto-start-reverse") ) {
			pMarker->bOrientAuto = 1;
			pMarker->bOrientAutoStartReverse = 1;
		} else {
			__xgeSvgParseAngleDegreesStrict(sValue, &pMarker->fOrientDegrees);
		}
	}
	__xgeSvgDefShapesClear(&pSvg->pDefs[iDefIndex]);
	pSvg->pDefs[iDefIndex].bOverflowVisible = 0;
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
	if ( __xgeSvgOverflowIsVisible(pTag, pTagEnd) ) {
		pSvg->pDefs[iDefIndex].bOverflowVisible = 1;
	}
	pMarker->fRefX = __xgeSvgAttrMarkerRef(pTag, pTagEnd, "refX", XGE_SVG_LENGTH_BASIS_X,
		pSvg->pDefs[iDefIndex].tViewBox, pSvg->pDefs[iDefIndex].bHasViewBox,
		pMarker->fWidth, pParentStyle != NULL ? pParentStyle->fFontSize : 16.0f, 0.0f);
	pMarker->fRefY = __xgeSvgAttrMarkerRef(pTag, pTagEnd, "refY", XGE_SVG_LENGTH_BASIS_Y,
		pSvg->pDefs[iDefIndex].tViewBox, pSvg->pDefs[iDefIndex].bHasViewBox,
		pMarker->fHeight, pParentStyle != NULL ? pParentStyle->fFontSize : 16.0f, 0.0f);
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
		int iUnits;
		if ( __xgeSvgParsePatternUnits(sValue, &iUnits) ) {
			pPattern->iUnits = iUnits;
			pPattern->iFlags |= XGE_SVG_PATTERN_HAS_UNITS;
		}
	}
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "patternContentUnits", sValue, sizeof(sValue)) ) {
		int iUnits;
		if ( __xgeSvgParsePatternUnits(sValue, &iUnits) ) {
			pPattern->iContentUnits = iUnits;
			pPattern->iFlags |= XGE_SVG_PATTERN_HAS_CONTENT_UNITS;
		}
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
	pSvg->pDefs[iDefIndex].bOverflowVisible = 0;
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
	if ( __xgeSvgOverflowIsSpecified(pTag, pTagEnd) ) {
		pSvg->pDefs[iDefIndex].bOverflowVisible = __xgeSvgOverflowIsVisible(pTag, pTagEnd) ? 1 : 0;
		pPattern->iFlags |= XGE_SVG_PATTERN_HAS_OVERFLOW;
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
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "clipPathUnits", sValue, sizeof(sValue)) ) {
		int iUnits;
		if ( __xgeSvgParseUserObjectUnits(sValue, XGE_SVG_CLIP_USER_SPACE, XGE_SVG_CLIP_OBJECT_BOUNDING_BOX, &iUnits) ) {
			pSvg->pClipPaths[iIndex].iUnits = iUnits;
		}
	}
	*pClipIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgParseClipRect(xge_svg pSvg, int iClipIndex, const char* pTag, const char* pTagEnd, xge_shape_ex_matrix_t tParentTransform, const xge_svg_style_t* pParentStyle)
{
	xge_rect_t tRect;
	xge_svg_style_t tStyle;

	(void)tParentTransform;
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
	tRect = __xgeSvgMatrixRectBounds(tStyle.tTransform, tRect);
	if ( tStyle.bViewportClip ) {
		tRect = __xgeSvgRectIntersect(tRect, tStyle.tViewportClip);
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XGE_OK;
	}
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
	if ( !__xgeSvgApplyViewportClipToShape(pShape, pStyle) ) {
		xgeShapeExDestroy(pShape);
		return XGE_OK;
	}
	if ( (pStyle != NULL) && (pStyle->sClipId[0] != '\0') ) {
		iRet = __xgeSvgApplyClipPath(pSvg, pShape, pStyle->sClipId);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
	}
	iRet = __xgeSvgClipPathAddShape(&pSvg->pClipPaths[iClipIndex], pShape);
	xgeShapeExDestroy(pShape);
	return iRet;
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
		if ( (iRet == XGE_OK) && pStyle->bClipRuleSet ) {
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
		if ( (iRet == XGE_OK) && pStyle->bFillRuleSet ) {
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
	(void)tParentTransform;
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
	tTransform = tStyle.tTransform;
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
			return (iRet == XGE_ERROR_INVALID_ARGUMENT) ? XGE_OK : iRet;
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

		__xgeSvgNormalizeRectRadii(&fRX, &fRY, &bHasRX, &bHasRY);
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
		int bEllipse = __xgeSvgTagNameEquals(pTag, "ellipse");
		float fCX = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "cx", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "cx", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fCY = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "cy", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "cy", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		float fRX;
		float fRY;

		__xgeSvgCircleEllipseRadii(pSvg, pTag, pTagEnd, bObjectBounds, tStyle.fFontSize, &fRX, &fRY);
		if ( (fRX <= 0.0f) || (fRY <= 0.0f) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = bEllipse ? xgeShapeExAppendEllipse(pShape, fCX, fCY, fRX, fRY, 1) :
		                   xgeShapeExAppendCircle(pShape, fCX, fCY, fRX, fRY, 1);
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
		iRet = xgeShapeExAppendLine(pShape, fX1, fY1, fX2, fY2);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		return __xgeSvgParseClipShapeFinish(pSvg, iClipIndex, pShape, &tStyle, tTransform);
	}
	if ( __xgeSvgTagNameEquals(pTag, "polygon") || __xgeSvgTagNameEquals(pTag, "polyline") ) {
		int bClose = __xgeSvgTagNameEquals(pTag, "polygon");

		if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "points", sValue, sizeof(sValue)) ) {
			return XGE_OK;
		}
		iRet = __xgeSvgCreatePointListShape(sValue, bClose, &pShape);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( pShape == NULL ) {
			return XGE_OK;
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
		int iUnits;
		if ( __xgeSvgParseUserObjectUnits(sValue, XGE_SVG_MASK_USER_SPACE, XGE_SVG_MASK_OBJECT_BOUNDING_BOX, &iUnits) ) {
			pSvg->pMasks[iIndex].iUnits = iUnits;
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
	if ( __xgeSvgAttrCopy(pTag, pTagEnd, "maskContentUnits", sValue, sizeof(sValue)) ) {
		int iUnits;
		if ( __xgeSvgParseUserObjectUnits(sValue, XGE_SVG_MASK_USER_SPACE, XGE_SVG_MASK_OBJECT_BOUNDING_BOX, &iUnits) ) {
			pSvg->pMasks[iIndex].iContentUnits = iUnits;
		}
	}
	if ( __xgeSvgAttrOrStyleCopy(pTag, pTagEnd, sStyle, "mask-type", sValue, sizeof(sValue)) ) {
		if ( __xgeSvgStyleValueEquals(sValue, "alpha") ) {
			pSvg->pMasks[iIndex].iMaskType = XGE_SVG_MASK_TYPE_ALPHA;
		} else if ( __xgeSvgStyleValueEquals(sValue, "luminance") ) {
			pSvg->pMasks[iIndex].iMaskType = XGE_SVG_MASK_TYPE_LUMINANCE;
		}
	}
	*pMaskIndex = iIndex;
	return XGE_OK;
}

static int __xgeSvgParseMaskRect(xge_svg pSvg, int iMaskIndex, const char* pTag, const char* pTagEnd, xge_shape_ex_matrix_t tParentTransform, const xge_svg_style_t* pParentStyle)
{
	xge_svg_mask_rect_t tMaskRect;
	xge_svg_style_t tStyle;

	(void)tParentTransform;
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
	tMaskRect.tRect = __xgeSvgMatrixRectBounds(tStyle.tTransform, tMaskRect.tRect);
	if ( tStyle.bViewportClip ) {
		tMaskRect.tRect = __xgeSvgRectIntersect(tMaskRect.tRect, tStyle.tViewportClip);
	}
	if ( (tMaskRect.tRect.fW <= 0.0f) || (tMaskRect.tRect.fH <= 0.0f) ) {
		return XGE_OK;
	}
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
	if ( !__xgeSvgApplyViewportClipToShape(pShape, pStyle) ) {
		xgeShapeExDestroy(pShape);
		return XGE_OK;
	}
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
	(void)tParentTransform;
	tStyle = (pParentStyle != NULL) ? __xgeSvgStyleResolve(pSvg, pParentStyle, pTag, pTagEnd) : __xgeSvgStyleDefault();
	if ( !tStyle.bVisible || !tStyle.bVisibility ) {
		return XGE_OK;
	}
	tTransform = tStyle.tTransform;
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

		__xgeSvgNormalizeRectRadii(&fRX, &fRY, &bHasRX, &bHasRY);
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
			return (iRet == XGE_ERROR_INVALID_ARGUMENT) ? XGE_OK : iRet;
		}
		return __xgeSvgParseMaskShapeFinish(pSvg, iMaskIndex, pShape, &tStyle, tTransform);
	}
	if ( __xgeSvgTagNameEquals(pTag, "circle") || __xgeSvgTagNameEquals(pTag, "ellipse") ) {
		int bEllipse = __xgeSvgTagNameEquals(pTag, "ellipse");
		float fCX = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "cx", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "cx", XGE_SVG_LENGTH_BASIS_X, tStyle.fFontSize, 0.0f);
		float fCY = bObjectBounds ? __xgeSvgAttrLengthRefEx(pTag, pTagEnd, "cy", 1.0f, tStyle.fFontSize, 0.0f) :
			__xgeSvgAttrLengthEx(pSvg, pTag, pTagEnd, "cy", XGE_SVG_LENGTH_BASIS_Y, tStyle.fFontSize, 0.0f);
		float fRX;
		float fRY;

		__xgeSvgCircleEllipseRadii(pSvg, pTag, pTagEnd, bObjectBounds, tStyle.fFontSize, &fRX, &fRY);
		if ( (fRX <= 0.0f) || (fRY <= 0.0f) ) {
			return XGE_OK;
		}
		iRet = xgeShapeExCreate(&pShape);
		if ( iRet != XGE_OK ) return iRet;
		iRet = bEllipse ? xgeShapeExAppendEllipse(pShape, fCX, fCY, fRX, fRY, 1) :
		                   xgeShapeExAppendCircle(pShape, fCX, fCY, fRX, fRY, 1);
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
		iRet = xgeShapeExAppendLine(pShape, fX1, fY1, fX2, fY2);
		if ( iRet != XGE_OK ) {
			xgeShapeExDestroy(pShape);
			return iRet;
		}
		return __xgeSvgParseMaskShapeFinish(pSvg, iMaskIndex, pShape, &tStyle, tTransform);
	}
	if ( __xgeSvgTagNameEquals(pTag, "polygon") || __xgeSvgTagNameEquals(pTag, "polyline") ) {
		int bClose = __xgeSvgTagNameEquals(pTag, "polygon");

		if ( !__xgeSvgAttrCopy(pTag, pTagEnd, "points", sValue, sizeof(sValue)) ) {
			return XGE_OK;
		}
		iRet = __xgeSvgCreatePointListShape(sValue, bClose, &pShape);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( pShape == NULL ) {
			return XGE_OK;
		}
		return __xgeSvgParseMaskShapeFinish(pSvg, iMaskIndex, pShape, &tStyle, tTransform);
	}
	return XGE_OK;
}

int xgeSvgCreate(xge_svg* ppSvg)
{
	xge_svg pSvg;

	if ( ppSvg == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pSvg = (xge_svg)xrtCalloc(1, sizeof(*pSvg));
	if ( pSvg == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
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
	const char* arrSwitchEndStack[XGE_SVG_STACK_MAX];
	const char* arrSwitchSelectedStartStack[XGE_SVG_STACK_MAX];
	const char* arrSwitchSelectedEndStack[XGE_SVG_STACK_MAX];
	int iSwitchTop;
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
	iSwitchTop = 0;
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
		if ( pTagEnd == NULL ) {
			break;
		}
		bSelfClosing = __xgeSvgIsSelfClosingTag(p, pTagEnd);
		while ( (iSwitchTop > 0) && (p >= arrSwitchEndStack[iSwitchTop - 1]) ) {
			iSwitchTop--;
		}
		if ( iSwitchTop > 0 ) {
			const char* pSelectedStart = arrSwitchSelectedStartStack[iSwitchTop - 1];
			const char* pSelectedEnd = arrSwitchSelectedEndStack[iSwitchTop - 1];

			if ( (pSelectedStart == NULL) || (pSelectedEnd == NULL) || (p < pSelectedStart) || (p >= pSelectedEnd) ) {
				p = pTagEnd + 1;
				continue;
			}
		}
		if ( __xgeSvgIsCloseTagName(p, "switch") ) {
			if ( iStackTop > 0 ) {
				iStackTop--;
			}
			if ( (iMaskIndex >= 0) && (iMaskTransformTop > 0) ) {
				iMaskTransformTop--;
			}
			if ( (iMaskIndex >= 0) && (iMaskStyleTop > 0) ) {
				iMaskStyleTop--;
			}
			if ( (iClipPathIndex >= 0) && (iClipTransformTop > 0) ) {
				iClipTransformTop--;
			}
			if ( (iClipPathIndex >= 0) && (iClipStyleTop > 0) ) {
				iClipStyleTop--;
			}
			__xgeSvgElementStackPop(pSvg, "switch");
			p = pTagEnd + 1;
			continue;
		}
		if ( (p[1] != '/') && !__xgeSvgConditionalElementMatches(p, pTagEnd) ) {
			const char* pElementEnd;

			if ( __xgeSvgElementRangeEnd(p, pTagEnd, sText + iSize, &pElementEnd) ) {
				p = pElementEnd;
			} else {
				p = pTagEnd + 1;
			}
			continue;
		}
		if ( (p[1] != '/') && __xgeSvgTagNameIsRawContentSkip(p) ) {
			const char* pElementEnd;

			if ( __xgeSvgElementRangeEnd(p, pTagEnd, sText + iSize, &pElementEnd) ) {
				p = pElementEnd;
			} else {
				p = pTagEnd + 1;
			}
			continue;
		}
		if ( __xgeSvgTagNameEquals(p, "switch") ) {
			xge_svg_style_t tStyle;
			int bHasSwitchRange;

			tStyle = __xgeSvgStyleResolve(pSvg, &arrStack[iStackTop], p, pTagEnd);
			bHasSwitchRange = 0;
			if ( !bSelfClosing ) {
				const char* pSwitchEnd;
				const char* pSelectedStart;
				const char* pSelectedEnd;

				if ( (iSwitchTop >= XGE_SVG_STACK_MAX) ||
				     (iStackTop >= (XGE_SVG_STACK_MAX - 1)) ||
				     ((iMaskIndex >= 0) && ((iMaskTransformTop >= (XGE_SVG_STACK_MAX - 1)) || (iMaskStyleTop >= (XGE_SVG_STACK_MAX - 1)))) ||
				     ((iClipPathIndex >= 0) && ((iClipTransformTop >= (XGE_SVG_STACK_MAX - 1)) || (iClipStyleTop >= (XGE_SVG_STACK_MAX - 1)))) ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return XGE_ERROR_OUT_OF_MEMORY;
				}
				if ( __xgeSvgSwitchSelectRange(p, pTagEnd, &pSwitchEnd, &pSelectedStart, &pSelectedEnd) ) {
					arrSwitchEndStack[iSwitchTop] = pSwitchEnd;
					arrSwitchSelectedStartStack[iSwitchTop] = pSelectedStart;
					arrSwitchSelectedEndStack[iSwitchTop] = pSelectedEnd;
					iSwitchTop++;
					bHasSwitchRange = 1;
				}
			}
			if ( bHasSwitchRange ) {
				arrStack[++iStackTop] = tStyle;
				arrDefStack[iStackTop] = arrDefStack[iStackTop - 1];
				arrMainIdDefStack[iStackTop] = arrMainIdDefStack[iStackTop - 1];
				arrGroupCopyParentDefStack[iStackTop] = -1;
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
				if ( iMaskIndex >= 0 ) {
					xge_svg_style_t tSwitchStyle;

					tSwitchStyle = __xgeSvgStyleResolve(pSvg, &arrMaskStyleStack[iMaskStyleTop], p, pTagEnd);
					arrMaskTransformStack[++iMaskTransformTop] = tSwitchStyle.tTransform;
					arrMaskStyleStack[++iMaskStyleTop] = tSwitchStyle;
				}
				if ( iClipPathIndex >= 0 ) {
					xge_svg_style_t tSwitchStyle;

					tSwitchStyle = __xgeSvgStyleResolve(pSvg, &arrClipStyleStack[iClipStyleTop], p, pTagEnd);
					arrClipTransformStack[++iClipTransformTop] = tSwitchStyle.tTransform;
					arrClipStyleStack[++iClipStyleTop] = tSwitchStyle;
				}
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( __xgeSvgIsCloseTagName(p, "clipPath") ) {
			iClipPathIndex = -1;
			iClipTransformTop = 0;
			arrClipTransformStack[0] = __xgeSvgMatrixIdentity();
			iClipStyleTop = 0;
			arrClipStyleStack[0] = arrStack[iStackTop];
			p = pTagEnd + 1;
			continue;
		}
		if ( (iClipPathIndex >= 0) && __xgeSvgIsCloseClipMaskContainer(p) ) {
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
		if ( (iMaskIndex >= 0) && __xgeSvgIsCloseClipMaskContainer(p) ) {
			if ( iMaskTransformTop > 0 ) {
				iMaskTransformTop--;
			}
			if ( iMaskStyleTop > 0 ) {
				iMaskStyleTop--;
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
			if ( __xgeSvgTagNameIsClipMaskContainer(p) ) {
				if ( !bSelfClosing ) {
					xge_svg_style_t tGroupStyle;

					if ( (iMaskTransformTop >= (XGE_SVG_STACK_MAX - 1)) || (iMaskStyleTop >= (XGE_SVG_STACK_MAX - 1)) ) {
						__xgeSvgElementNodesClear(pSvg);
						xrtFree(sText);
						return XGE_ERROR_OUT_OF_MEMORY;
					}
					tGroupStyle = __xgeSvgResolveClipMaskContainerStyle(pSvg, &arrMaskStyleStack[iMaskStyleTop], p, pTagEnd);
					arrMaskTransformStack[++iMaskTransformTop] = tGroupStyle.tTransform;
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
			} else if ( __xgeSvgTagNameEquals(p, "feFlood") ) {
				iRet = __xgeSvgParseFilterFeFlood(pSvg, iFilterIndex, p, pTagEnd);
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
			} else if ( __xgeSvgTagNameEquals(p, "feGaussianBlur") ) {
				iRet = __xgeSvgParseFilterFeGaussianBlur(pSvg, iFilterIndex, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			} else if ( __xgeSvgTagNameEquals(p, "feMorphology") ) {
				iRet = __xgeSvgParseFilterFeMorphology(pSvg, iFilterIndex, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			} else if ( __xgeSvgTagNameEquals(p, "feTile") ) {
				iRet = __xgeSvgParseFilterFeTile(pSvg, iFilterIndex, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			} else if ( __xgeSvgTagNameEquals(p, "feImage") ) {
				iRet = __xgeSvgParseFilterFeImage(pSvg, iFilterIndex, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			} else if ( __xgeSvgTagNameEquals(p, "feColorMatrix") ) {
				iRet = __xgeSvgParseFilterFeColorMatrix(pSvg, iFilterIndex, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			} else if ( __xgeSvgTagNameEquals(p, "feComponentTransfer") ) {
				const char* pElementEnd = pTagEnd + 1;
				int bHasElementEnd;

				bHasElementEnd = __xgeSvgElementRangeEnd(p, pTagEnd, sText + iSize, &pElementEnd);
				if ( !bHasElementEnd ) {
					pSvg->pFilters[iFilterIndex].bHasUnsupportedPrimitive = 1;
				}
				iRet = __xgeSvgParseFilterFeComponentTransfer(pSvg, iFilterIndex, p, pTagEnd, pElementEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
				p = pElementEnd;
				continue;
			} else if ( __xgeSvgTagNameEquals(p, "feComposite") ) {
				iRet = __xgeSvgParseFilterFeComposite(pSvg, iFilterIndex, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			} else if ( __xgeSvgTagNameEquals(p, "feMerge") ) {
				const char* pElementEnd = pTagEnd + 1;
				int bHasElementEnd;

				bHasElementEnd = __xgeSvgElementRangeEnd(p, pTagEnd, sText + iSize, &pElementEnd);
				if ( !bHasElementEnd ) {
					pSvg->pFilters[iFilterIndex].bHasUnsupportedPrimitive = 1;
				}
				iRet = __xgeSvgParseFilterFeMerge(pSvg, iFilterIndex, p, pTagEnd, pElementEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
				p = pElementEnd;
				continue;
			} else if ( __xgeSvgTagNameEquals(p, "feBlend") ) {
				iRet = __xgeSvgParseFilterFeBlend(pSvg, iFilterIndex, p, pTagEnd);
				if ( iRet != XGE_OK ) {
					__xgeSvgElementNodesClear(pSvg);
					xrtFree(sText);
					return iRet;
				}
			} else if ( (p[0] == '<') && (p[1] == 'f') && (p[2] == 'e') ) {
				pSvg->pFilters[iFilterIndex].bHasUnsupportedPrimitive = 1;
			}
			p = pTagEnd + 1;
			continue;
		}
		if ( iClipPathIndex >= 0 ) {
			if ( __xgeSvgTagNameIsClipMaskContainer(p) ) {
				if ( !bSelfClosing ) {
					xge_svg_style_t tGroupStyle;

					if ( (iClipTransformTop >= (XGE_SVG_STACK_MAX - 1)) || (iClipStyleTop >= (XGE_SVG_STACK_MAX - 1)) ) {
						__xgeSvgElementNodesClear(pSvg);
						xrtFree(sText);
						return XGE_ERROR_OUT_OF_MEMORY;
					}
					tGroupStyle = __xgeSvgResolveClipMaskContainerStyle(pSvg, &arrClipStyleStack[iClipStyleTop], p, pTagEnd);
					arrClipTransformStack[++iClipTransformTop] = tGroupStyle.tTransform;
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
		if ( __xgeSvgIsCloseGroupContainer(p) ||
		     __xgeSvgIsCloseTagName(p, "symbol") ||
		     __xgeSvgIsCloseTagName(p, "marker") ||
		     __xgeSvgIsCloseTagName(p, "pattern") ||
		     __xgeSvgIsCloseTagName(p, "svg") ) {
			if ( __xgeSvgIsCloseGroupContainer(p) && (iStackTop > 0) ) {
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
			} else if ( __xgeSvgIsCloseTagName(p, "a") ) {
				__xgeSvgElementStackPop(pSvg, "a");
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
		} else if ( __xgeSvgTagNameIsGroupContainer(p) || __xgeSvgTagNameEquals(p, "symbol") ) {
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
			} else if ( !bCapture && __xgeSvgTagNameIsGroupContainer(p) && __xgeSvgAttrCopy(p, pTagEnd, "id", sId, sizeof(sId)) && (sId[0] != '\0') ) {
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
static int __xgeSvgGetDrawBoundsInternal(xge_svg pSvg, xge_rect_t tDst, float fTolerance, xge_rect_t* pBounds, int iDepth);
static int __xgeSvgContainsPointWithParent(xge_svg pSvg, xge_shape_ex_matrix_t tParent, xge_vec2_t tPoint, float fTolerance, int* pContains, int iDepth);
static int __xgeSvgDrawContainsPointInternal(xge_svg pSvg, xge_rect_t tDst, xge_vec2_t tPoint, float fTolerance, int* pContains, int iDepth);

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
	if ( iRet == XGE_OK ) iRet = xgeShapeExAppendLine(pLine, fX0, fY, fX1, fY);
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
	xge_vec2_t arrPoints[257];
	int iPointCount;

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
	iPointCount = 0;
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
		if ( iPointCount < (int)(sizeof(arrPoints) / sizeof(arrPoints[0])) ) {
			arrPoints[iPointCount++] = tPoint;
		}
	}
	if ( iPointCount <= 0 ) {
		xgeShapeExDestroy(pLine);
		return XGE_OK;
	}
	iRet = xgeShapeExAppendPolyline(pLine, arrPoints, iPointCount);
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
	fBaselineOffset = __xgeSvgTextBaselineOffsetPx(pFont, pText->tStyle.iTextBaseline) + (pText->tStyle.fBaselineShift * fScale);
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
	fBaselineY += pText->tStyle.fBaselineShift;
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

static xge_rect_t __xgeSvgRectUnion(xge_rect_t a, xge_rect_t b)
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

static int __xgeSvgRectContainsPoint(xge_rect_t tRect, xge_vec2_t tPoint)
{
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return 0;
	}
	return (tPoint.fX >= tRect.fX) && (tPoint.fX <= (tRect.fX + tRect.fW)) &&
	       (tPoint.fY >= tRect.fY) && (tPoint.fY <= (tRect.fY + tRect.fH));
}

static int __xgeSvgBoundsUnion(xge_rect_t* pOut, int* pHasBounds, xge_rect_t tBounds)
{
	if ( (pOut == NULL) || (pHasBounds == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (tBounds.fW <= 0.0f) || (tBounds.fH <= 0.0f) ) {
		return XGE_OK;
	}
	if ( !*pHasBounds ) {
		*pOut = tBounds;
		*pHasBounds = 1;
	} else {
		*pOut = __xgeSvgRectUnion(*pOut, tBounds);
	}
	return XGE_OK;
}

static xge_rect_t __xgeSvgRectExpand(xge_rect_t tBounds, float fAmount)
{
	if ( fAmount <= 0.0f ) {
		return tBounds;
	}
	tBounds.fX -= fAmount;
	tBounds.fY -= fAmount;
	tBounds.fW += fAmount * 2.0f;
	tBounds.fH += fAmount * 2.0f;
	if ( tBounds.fW < 0.0f ) tBounds.fW = 0.0f;
	if ( tBounds.fH < 0.0f ) tBounds.fH = 0.0f;
	return tBounds;
}

static int __xgeSvgShapeStopsVisible(const xge_shape_ex_color_stop_t* pStops, int iStopCount)
{
	int i;

	if ( (pStops == NULL) || (iStopCount <= 0) ) {
		return 0;
	}
	for ( i = 0; i < iStopCount; i++ ) {
		if ( XGE_COLOR_GET_A(pStops[i].iColor) != 0u ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeSvgShapeFillPaintVisible(xge_shape_ex pShape)
{
	const xge_shape_ex_color_stop_t* pStops;
	uint32_t iColor;
	float fX1;
	float fY1;
	float fX2;
	float fY2;
	float fR;
	int iStopCount;
	int iUnits;
	int iType;

	if ( xgeShapeExFillTypeGet(pShape, &iType) != XGE_OK ) {
		return 1;
	}
	if ( iType == XGE_SHAPE_EX_PAINT_SOLID ) {
		if ( xgeShapeExFillColorGet(pShape, &iColor) != XGE_OK ) {
			return 1;
		}
		return XGE_COLOR_GET_A(iColor) != 0u;
	}
	if ( iType == XGE_SHAPE_EX_PAINT_LINEAR_GRADIENT ) {
		pStops = NULL;
		iStopCount = 0;
		if ( xgeShapeExFillLinearGradientGet(pShape, &fX1, &fY1, &fX2, &fY2, &iUnits, &pStops, &iStopCount) != XGE_OK ) {
			return 1;
		}
		return __xgeSvgShapeStopsVisible(pStops, iStopCount);
	}
	if ( iType == XGE_SHAPE_EX_PAINT_RADIAL_GRADIENT ) {
		pStops = NULL;
		iStopCount = 0;
		if ( xgeShapeExFillRadialGradientGet(pShape, &fX1, &fY1, &fR, &fX2, &fY2, &iUnits, &pStops, &iStopCount) != XGE_OK ) {
			return 1;
		}
		return __xgeSvgShapeStopsVisible(pStops, iStopCount);
	}
	return 1;
}

static int __xgeSvgShapeStrokePaintVisible(xge_shape_ex pShape)
{
	const xge_shape_ex_color_stop_t* pStops;
	uint32_t iColor;
	float fX1;
	float fY1;
	float fX2;
	float fY2;
	float fR;
	float fStrokeWidth;
	int iStopCount;
	int iUnits;
	int iType;

	if ( xgeShapeExStrokeWidthGet(pShape, &fStrokeWidth) != XGE_OK ) {
		return 1;
	}
	if ( fStrokeWidth <= 0.0f ) {
		return 0;
	}
	if ( xgeShapeExStrokeTypeGet(pShape, &iType) != XGE_OK ) {
		return 1;
	}
	if ( iType == XGE_SHAPE_EX_PAINT_SOLID ) {
		if ( xgeShapeExStrokeColorGet(pShape, &iColor) != XGE_OK ) {
			return 1;
		}
		return XGE_COLOR_GET_A(iColor) != 0u;
	}
	if ( iType == XGE_SHAPE_EX_PAINT_LINEAR_GRADIENT ) {
		pStops = NULL;
		iStopCount = 0;
		if ( xgeShapeExStrokeLinearGradientGet(pShape, &fX1, &fY1, &fX2, &fY2, &iUnits, &pStops, &iStopCount) != XGE_OK ) {
			return 1;
		}
		return __xgeSvgShapeStopsVisible(pStops, iStopCount);
	}
	if ( iType == XGE_SHAPE_EX_PAINT_RADIAL_GRADIENT ) {
		pStops = NULL;
		iStopCount = 0;
		if ( xgeShapeExStrokeRadialGradientGet(pShape, &fX1, &fY1, &fR, &fX2, &fY2, &iUnits, &pStops, &iStopCount) != XGE_OK ) {
			return 1;
		}
		return __xgeSvgShapeStopsVisible(pStops, iStopCount);
	}
	return 1;
}

static int __xgeSvgShapePaintVisible(xge_shape_ex pShape)
{
	return __xgeSvgShapeFillPaintVisible(pShape) || __xgeSvgShapeStrokePaintVisible(pShape);
}

static int __xgeSvgShapeBoundsTransformed(xge_shape_ex pShape, float fTolerance, xge_shape_ex_matrix_t tParent, int bRespectPaint, xge_rect_t* pBounds)
{
	xge_rect_t tBounds;
	int bVisible;
	float fOpacity;
	int iRet;

	if ( (pBounds == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pBounds, 0, sizeof(*pBounds));
	if ( pShape == NULL ) {
		return XGE_OK;
	}
	bVisible = 1;
	fOpacity = 1.0f;
	(void)xgeShapeExVisibleGet(pShape, &bVisible);
	(void)xgeShapeExOpacityGet(pShape, &fOpacity);
	if ( !bVisible || (fOpacity <= 0.0f) ) {
		return XGE_OK;
	}
	if ( bRespectPaint && !__xgeSvgShapePaintVisible(pShape) ) {
		return XGE_OK;
	}
	iRet = xgeShapeExGetBounds(pShape, fTolerance, &tBounds);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	*pBounds = __xgeSvgMatrixRectBounds(tParent, tBounds);
	return XGE_OK;
}

static int __xgeSvgSceneBoundsTransformed(xge_shape_ex_scene pScene, float fTolerance, xge_shape_ex_matrix_t tParent, xge_rect_t* pBounds)
{
	xge_shape_ex_matrix_t tSceneMatrix;
	xge_shape_ex_matrix_t tMatrix;
	xge_rect_t tOut;
	int bHasBounds;
	int bVisible;
	float fOpacity;
	int iCount;
	int iRet;
	int i;

	if ( (pBounds == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pBounds, 0, sizeof(*pBounds));
	if ( pScene == NULL ) {
		return XGE_OK;
	}
	bVisible = 1;
	fOpacity = 1.0f;
	if ( xgeShapeExSceneVisibleGet(pScene, &bVisible) != XGE_OK || !bVisible ) {
		return XGE_OK;
	}
	if ( xgeShapeExSceneOpacityGet(pScene, &fOpacity) != XGE_OK || (fOpacity <= 0.0f) ) {
		return XGE_OK;
	}
	iRet = xgeShapeExSceneTransformGet(pScene, &tSceneMatrix);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tMatrix = __xgeSvgMatrixMul(tParent, tSceneMatrix);
	iRet = xgeShapeExSceneGetCount(pScene, &iCount);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	memset(&tOut, 0, sizeof(tOut));
	bHasBounds = 0;
	for ( i = 0; i < iCount; i++ ) {
		xge_shape_ex pShape;
		xge_rect_t tShapeBounds;

		pShape = NULL;
		iRet = xgeShapeExSceneGetAt(pScene, i, &pShape);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		iRet = __xgeSvgShapeBoundsTransformed(pShape, fTolerance, tMatrix, 1, &tShapeBounds);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		iRet = __xgeSvgBoundsUnion(&tOut, &bHasBounds, tShapeBounds);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	if ( bHasBounds ) {
		xge_rect_t tClipRect;
		int bClipRect;
		int iClipCount;

		bClipRect = 0;
		if ( xgeShapeExSceneClipRectGet(pScene, &tClipRect, &bClipRect) == XGE_OK && bClipRect ) {
			tOut = __xgeSvgRectIntersect(tOut, __xgeSvgMatrixRectBounds(tMatrix, tClipRect));
		}
		if ( xgeShapeExSceneClipShapeGetCount(pScene, &iClipCount) == XGE_OK ) {
			xge_rect_t tClipUnion;
			int bHasClipBounds;

			memset(&tClipUnion, 0, sizeof(tClipUnion));
			bHasClipBounds = 0;
			for ( i = 0; i < iClipCount; i++ ) {
				xge_shape_ex pClipShape;
				xge_rect_t tClipBounds;

				pClipShape = NULL;
				iRet = xgeShapeExSceneClipShapeGetAt(pScene, i, &pClipShape);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
				iRet = __xgeSvgShapeBoundsTransformed(pClipShape, fTolerance, tMatrix, 0, &tClipBounds);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
				iRet = __xgeSvgBoundsUnion(&tClipUnion, &bHasClipBounds, tClipBounds);
				if ( iRet != XGE_OK ) {
					return iRet;
				}
			}
			if ( iClipCount > 0 ) {
				if ( bHasClipBounds ) {
					tOut = __xgeSvgRectIntersect(tOut, tClipUnion);
				} else {
					memset(&tOut, 0, sizeof(tOut));
				}
			}
		}
	}
	*pBounds = bHasBounds ? tOut : (xge_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return XGE_OK;
}

static int __xgeSvgApplyItemClipAndMask(xge_rect_t* pBounds, xge_shape_ex_matrix_t tMatrix, int bClipRect, xge_rect_t tClipRect, xge_shape_ex pMaskShape, float fTolerance)
{
	int iRet;

	if ( pBounds == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pBounds->fW <= 0.0f) || (pBounds->fH <= 0.0f) ) {
		return XGE_OK;
	}
	if ( bClipRect ) {
		*pBounds = __xgeSvgRectIntersect(*pBounds, __xgeSvgMatrixRectBounds(tMatrix, tClipRect));
	}
	if ( pMaskShape != NULL ) {
		xge_rect_t tMaskBounds;

		iRet = __xgeSvgShapeBoundsTransformed(pMaskShape, fTolerance, tMatrix, 1, &tMaskBounds);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		*pBounds = __xgeSvgRectIntersect(*pBounds, tMaskBounds);
	}
	return XGE_OK;
}

static int __xgeSvgTextPathBounds(xge_shape_ex pTextPath, float fTolerance, float fFontSize, xge_rect_t* pBounds)
{
	int iRet;

	if ( pBounds == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pBounds, 0, sizeof(*pBounds));
	if ( pTextPath == NULL ) {
		return XGE_OK;
	}
	iRet = xgeShapeExGetBounds(pTextPath, fTolerance, pBounds);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	*pBounds = __xgeSvgRectExpand(*pBounds, fFontSize);
	return XGE_OK;
}

static int __xgeSvgTextLineBounds(xge_svg pSvg, const xge_svg_text_item_t* pText, xge_rect_t* pBounds)
{
	xge_font pFont;
	xge_rect_t tOut;
	const char* p;
	uint32_t iCodepoint;
	float fFontSize;
	float fTextScaleX;
	float fTextExtraSpacing;
	float fTextAdvance;
	float fAnchorWidth;
	float fPenX;
	float fBaselineY;
	float fAscent;
	float fDescent;
	int bHasBounds;

	if ( !__xgeSvgValid(pSvg) || (pText == NULL) || (pBounds == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pBounds, 0, sizeof(*pBounds));
	fFontSize = pText->tStyle.fFontSize > 0.0f ? pText->tStyle.fFontSize : 16.0f;
	pFont = NULL;
	if ( __xgeSvgFontCacheGet(pSvg, fFontSize, &pFont) != XGE_OK || pFont == NULL ) {
		*pBounds = __xgeSvgTextMaskBounds(pText);
		return XGE_OK;
	}
	fTextScaleX = (pText->fTextScaleX > 0.0001f) ? pText->fTextScaleX : 1.0f;
	fTextExtraSpacing = pText->fTextExtraSpacing;
	fTextAdvance = __xgeSvgTextActualAdvance(pFont, pText->sText, &pText->tStyle, 1.0f, fTextScaleX, fTextExtraSpacing);
	fAnchorWidth = (pText->fAnchorAdvance > 0.0f) ? pText->fAnchorAdvance : fTextAdvance;
	fPenX = pText->fX;
	fBaselineY = pText->fY + __xgeSvgTextBaselineOffsetPx(pFont, pText->tStyle.iTextBaseline) + pText->tStyle.fBaselineShift;
	if ( pText->tStyle.iTextAnchor == XGE_SVG_TEXT_ANCHOR_MIDDLE ) {
		fPenX -= fAnchorWidth * 0.5f;
	} else if ( pText->tStyle.iTextAnchor == XGE_SVG_TEXT_ANCHOR_END ) {
		fPenX -= fAnchorWidth;
	}
	fAscent = pFont->fAscent > 0.0f ? pFont->fAscent : fFontSize;
	fDescent = pFont->fDescent < 0.0f ? pFont->fDescent : 0.0f;
	memset(&tOut, 0, sizeof(tOut));
	bHasBounds = 0;
	p = pText->sText;
	while ( p != NULL && *p != '\0' ) {
		xge_glyph_metrics_t tMetrics;
		xge_rect_t tGlyphBounds;
		int bHasNext;

		if ( xgeTextUTF8Next(&p, &iCodepoint) != XGE_OK ) {
			break;
		}
		if ( iCodepoint == '\n' ) {
			break;
		}
		bHasNext = *p != '\0';
		if ( xgeFontGlyphGet(pFont, iCodepoint, &tMetrics) == XGE_OK ) {
			tGlyphBounds.fX = fPenX + tMetrics.fX0 * fTextScaleX;
			tGlyphBounds.fY = fBaselineY + tMetrics.fY0;
			tGlyphBounds.fW = (tMetrics.fX1 - tMetrics.fX0) * fTextScaleX;
			tGlyphBounds.fH = tMetrics.fY1 - tMetrics.fY0;
			(void)__xgeSvgBoundsUnion(&tOut, &bHasBounds, tGlyphBounds);
			fPenX += (tMetrics.fAdvanceX * fTextScaleX) + __xgeSvgTextSpacingForCodepoint(&pText->tStyle, iCodepoint, bHasNext) + (bHasNext ? fTextExtraSpacing : 0.0f);
		} else {
			fPenX += __xgeSvgTextSpacingForCodepoint(&pText->tStyle, iCodepoint, bHasNext) + (bHasNext ? fTextExtraSpacing : 0.0f);
		}
	}
	if ( !bHasBounds && fTextAdvance > 0.0f ) {
		tOut.fX = (pText->tStyle.iTextAnchor == XGE_SVG_TEXT_ANCHOR_MIDDLE) ? pText->fX - fAnchorWidth * 0.5f :
		          (pText->tStyle.iTextAnchor == XGE_SVG_TEXT_ANCHOR_END) ? pText->fX - fAnchorWidth : pText->fX;
		tOut.fY = fBaselineY - fAscent;
		tOut.fW = fTextAdvance;
		tOut.fH = fAscent - fDescent;
		bHasBounds = 1;
	}
	*pBounds = bHasBounds ? tOut : (xge_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return XGE_OK;
}

static int __xgeSvgTextItemBounds(xge_svg pSvg, const xge_svg_text_item_t* pText, xge_shape_ex_matrix_t tParent, float fTolerance, xge_rect_t* pBounds)
{
	xge_shape_ex_matrix_t tMatrix;
	xge_rect_t tBounds;
	uint32_t iColor;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pText == NULL) || (pBounds == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pBounds, 0, sizeof(*pBounds));
	if ( (pText->sText == NULL) || (pText->sText[0] == '\0') || !pText->tStyle.bVisible || !pText->tStyle.bVisibility ) {
		return XGE_OK;
	}
	iColor = __xgeSvgColorAlpha(pText->tStyle.iFillColor, pText->tStyle.fOpacity * pText->tStyle.fFillOpacity);
	if ( XGE_COLOR_GET_A(iColor) <= 0 ) {
		return XGE_OK;
	}
	if ( pText->pTextPath != NULL ) {
		iRet = __xgeSvgTextPathBounds(pText->pTextPath, fTolerance, pText->tStyle.fFontSize, &tBounds);
	} else {
		iRet = __xgeSvgTextLineBounds(pSvg, pText, &tBounds);
	}
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tMatrix = __xgeSvgMatrixMul(tParent, pText->tStyle.tTransform);
	tBounds = __xgeSvgMatrixRectBounds(tMatrix, tBounds);
	iRet = __xgeSvgApplyItemClipAndMask(&tBounds, tMatrix, pText->bClipRect, pText->tClipRect, pText->pMaskShape, fTolerance);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	*pBounds = tBounds;
	return XGE_OK;
}

static int __xgeSvgImageItemBounds(const xge_svg_image_item_t* pImage, xge_shape_ex_matrix_t tParent, float fTolerance, xge_rect_t* pBounds, int iDepth)
{
	xge_shape_ex_matrix_t tMatrix;
	xge_rect_t tDst;
	int iRet;

	if ( pBounds == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pBounds, 0, sizeof(*pBounds));
	if ( (pImage == NULL) || !__xgeSvgValid(pImage->pSvg) ) {
		return XGE_OK;
	}
	if ( !pImage->tStyle.bVisible || !pImage->tStyle.bVisibility || (pImage->tStyle.fOpacity <= 0.0f) ) {
		return XGE_OK;
	}
	if ( iDepth > XGE_SVG_BOUNDS_MAX_DEPTH ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tMatrix = __xgeSvgMatrixMul(tParent, pImage->tStyle.tTransform);
	tDst = __xgeSvgMatrixRectBounds(tMatrix, pImage->tRect);
	iRet = __xgeSvgGetDrawBoundsInternal(pImage->pSvg, tDst, fTolerance, pBounds, iDepth + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	return __xgeSvgApplyItemClipAndMask(pBounds, tMatrix, pImage->bClipRect, pImage->tClipRect, pImage->pMaskShape, fTolerance);
}

static int __xgeSvgRasterImageItemBounds(const xge_svg_raster_item_t* pImage, xge_shape_ex_matrix_t tParent, float fTolerance, xge_rect_t* pBounds)
{
	xge_shape_ex_matrix_t tMatrix;
	xge_rect_t tImageBox;
	xge_rect_t tLocalDst;
	xge_rect_t tDst;
	uint32_t iColor;
	int iAlpha;
	int iRet;

	if ( pBounds == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pBounds, 0, sizeof(*pBounds));
	if ( (pImage == NULL) || (pImage->pRaster == NULL) ) {
		return XGE_OK;
	}
	if ( !pImage->tStyle.bVisible || !pImage->tStyle.bVisibility || (pImage->tStyle.fOpacity <= 0.0f) ) {
		return XGE_OK;
	}
	if ( (pImage->pRaster->tImage.iWidth <= 0) || (pImage->pRaster->tImage.iHeight <= 0) ) {
		return XGE_OK;
	}
	iColor = pImage->iColor;
	iAlpha = (int)(pImage->tStyle.fOpacity * (float)XGE_COLOR_GET_A(iColor) + 0.5f);
	if ( iAlpha <= 0 ) {
		return XGE_OK;
	}
	tMatrix = __xgeSvgMatrixMul(tParent, pImage->tStyle.tTransform);
	tImageBox.fX = 0.0f;
	tImageBox.fY = 0.0f;
	tImageBox.fW = (float)pImage->pRaster->tImage.iWidth;
	tImageBox.fH = (float)pImage->pRaster->tImage.iHeight;
	tLocalDst = __xgeSvgAspectViewport(tImageBox, pImage->tRect, pImage->iAspectAlignX, pImage->iAspectAlignY, pImage->iAspectMeetOrSlice);
	tDst = __xgeSvgMatrixRectBounds(tMatrix, tLocalDst);
	iRet = __xgeSvgApplyItemClipAndMask(&tDst, tMatrix, pImage->bClipRect, pImage->tClipRect, pImage->pMaskShape, fTolerance);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	*pBounds = tDst;
	return XGE_OK;
}

static int __xgeSvgShapeContainsPointTransformed(xge_shape_ex pShape, xge_shape_ex_matrix_t tParent, xge_vec2_t tPoint, float fTolerance, int* pContains)
{
	int bVisible;
	int bFillVisible;
	int bStrokeVisible;
	float fOpacity;

	if ( pContains == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pContains = 0;
	if ( pShape == NULL ) {
		return XGE_OK;
	}
	bVisible = 1;
	fOpacity = 1.0f;
	(void)xgeShapeExVisibleGet(pShape, &bVisible);
	(void)xgeShapeExOpacityGet(pShape, &fOpacity);
	if ( !bVisible || (fOpacity <= 0.0f) ) {
		return XGE_OK;
	}
	bFillVisible = __xgeSvgShapeFillPaintVisible(pShape);
	bStrokeVisible = __xgeSvgShapeStrokePaintVisible(pShape);
	if ( !bFillVisible && !bStrokeVisible ) {
		return XGE_OK;
	}
	return xgeShapeExContainsPointEx(pShape, tPoint.fX, tPoint.fY, fTolerance, &tParent, pContains);
}

static int __xgeSvgSceneContainsPointTransformed(xge_shape_ex_scene pScene, xge_shape_ex_matrix_t tParent, xge_vec2_t tPoint, float fTolerance, int* pContains)
{
	xge_shape_ex pHitShape;
	int iRet;

	if ( pContains == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pContains = 0;
	if ( pScene == NULL ) {
		return XGE_OK;
	}
	pHitShape = NULL;
	iRet = xgeShapeExSceneHitTestEx(pScene, tPoint.fX, tPoint.fY, fTolerance, &tParent, &pHitShape);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	*pContains = pHitShape != NULL;
	return XGE_OK;
}

static int __xgeSvgItemClipAndMaskContainsPoint(xge_shape_ex_matrix_t tMatrix, int bClipRect, xge_rect_t tClipRect, xge_shape_ex pMaskShape, xge_vec2_t tPoint, float fTolerance, int* pContains)
{
	int iRet;
	int bMaskContains;

	if ( pContains == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pContains = 1;
	if ( bClipRect && !__xgeSvgRectContainsPoint(__xgeSvgMatrixRectBounds(tMatrix, tClipRect), tPoint) ) {
		*pContains = 0;
		return XGE_OK;
	}
	if ( pMaskShape != NULL ) {
		bMaskContains = 0;
		iRet = __xgeSvgShapeContainsPointTransformed(pMaskShape, tMatrix, tPoint, fTolerance, &bMaskContains);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( !bMaskContains ) {
			*pContains = 0;
		}
	}
	return XGE_OK;
}

static int __xgeSvgImageItemContainsPoint(const xge_svg_image_item_t* pImage, xge_shape_ex_matrix_t tParent, xge_vec2_t tPoint, float fTolerance, int* pContains, int iDepth)
{
	xge_shape_ex_matrix_t tMatrix;
	xge_rect_t tDst;
	int bClipContains;
	int iRet;

	if ( pContains == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pContains = 0;
	if ( (pImage == NULL) || !__xgeSvgValid(pImage->pSvg) ) {
		return XGE_OK;
	}
	if ( !pImage->tStyle.bVisible || !pImage->tStyle.bVisibility || (pImage->tStyle.fOpacity <= 0.0f) ) {
		return XGE_OK;
	}
	if ( iDepth > XGE_SVG_BOUNDS_MAX_DEPTH ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tMatrix = __xgeSvgMatrixMul(tParent, pImage->tStyle.tTransform);
	tDst = __xgeSvgMatrixRectBounds(tMatrix, pImage->tRect);
	if ( !__xgeSvgRectContainsPoint(tDst, tPoint) ) {
		return XGE_OK;
	}
	bClipContains = 0;
	iRet = __xgeSvgItemClipAndMaskContainsPoint(tMatrix, pImage->bClipRect, pImage->tClipRect, pImage->pMaskShape, tPoint, fTolerance, &bClipContains);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( !bClipContains ) {
		return XGE_OK;
	}
	return __xgeSvgDrawContainsPointInternal(pImage->pSvg, tDst, tPoint, fTolerance, pContains, iDepth + 1);
}

static int __xgeSvgRasterImageItemContainsPoint(const xge_svg_raster_item_t* pImage, xge_shape_ex_matrix_t tParent, xge_vec2_t tPoint, float fTolerance, int* pContains)
{
	const xge_image_t* pSource;
	const unsigned char* pPixel;
	xge_shape_ex_matrix_t tMatrix;
	xge_shape_ex_matrix_t tInverse;
	xge_rect_t tImageBox;
	xge_rect_t tLocalDst;
	xge_rect_t tDst;
	xge_vec2_t tLocalPoint;
	uint32_t iColor;
	float fU;
	float fV;
	int iAlpha;
	int iX;
	int iY;
	int bClipContains;
	int iRet;

	if ( pContains == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pContains = 0;
	if ( (pImage == NULL) || (pImage->pRaster == NULL) ) {
		return XGE_OK;
	}
	pSource = &pImage->pRaster->tImage;
	if ( !pImage->tStyle.bVisible || !pImage->tStyle.bVisibility || (pImage->tStyle.fOpacity <= 0.0f) ) {
		return XGE_OK;
	}
	if ( (pSource->iWidth <= 0) || (pSource->iHeight <= 0) || (pSource->iFormat != XGE_PIXEL_RGBA8) || (pSource->pPixels == NULL) || (pSource->iStride < pSource->iWidth * 4) ) {
		return XGE_OK;
	}
	iColor = pImage->iColor;
	iAlpha = (int)(pImage->tStyle.fOpacity * (float)XGE_COLOR_GET_A(iColor) + 0.5f);
	if ( iAlpha <= 0 ) {
		return XGE_OK;
	}
	tMatrix = __xgeSvgMatrixMul(tParent, pImage->tStyle.tTransform);
	tImageBox.fX = 0.0f;
	tImageBox.fY = 0.0f;
	tImageBox.fW = (float)pSource->iWidth;
	tImageBox.fH = (float)pSource->iHeight;
	tLocalDst = __xgeSvgAspectViewport(tImageBox, pImage->tRect, pImage->iAspectAlignX, pImage->iAspectAlignY, pImage->iAspectMeetOrSlice);
	if ( (tLocalDst.fW <= 0.0f) || (tLocalDst.fH <= 0.0f) ) {
		return XGE_OK;
	}
	tDst = __xgeSvgMatrixRectBounds(tMatrix, tLocalDst);
	if ( !__xgeSvgRectContainsPoint(tDst, tPoint) ) {
		return XGE_OK;
	}
	bClipContains = 0;
	iRet = __xgeSvgItemClipAndMaskContainsPoint(tMatrix, pImage->bClipRect, pImage->tClipRect, pImage->pMaskShape, tPoint, fTolerance, &bClipContains);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( !bClipContains ) {
		return XGE_OK;
	}
	if ( !__xgeSvgMatrixInvert(tMatrix, &tInverse) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tLocalPoint = __xgeSvgMatrixPoint(tInverse, tPoint);
	if ( !__xgeSvgRectContainsPoint(tLocalDst, tLocalPoint) ) {
		return XGE_OK;
	}
	fU = (tLocalPoint.fX - tLocalDst.fX) / tLocalDst.fW;
	fV = (tLocalPoint.fY - tLocalDst.fY) / tLocalDst.fH;
	iX = (int)floorf(fU * (float)pSource->iWidth);
	iY = (int)floorf(fV * (float)pSource->iHeight);
	if ( iX < 0 ) iX = 0;
	if ( iY < 0 ) iY = 0;
	if ( iX >= pSource->iWidth ) iX = pSource->iWidth - 1;
	if ( iY >= pSource->iHeight ) iY = pSource->iHeight - 1;
	pPixel = (const unsigned char*)pSource->pPixels + ((size_t)iY * (size_t)pSource->iStride) + ((size_t)iX * 4u);
	*pContains = pPixel[3] > 0;
	return XGE_OK;
}

static int __xgeSvgBoundsItemContainsPoint(xge_rect_t tBounds, xge_vec2_t tPoint, int* pContains)
{
	if ( pContains == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pContains = __xgeSvgRectContainsPoint(tBounds, tPoint);
	return XGE_OK;
}

static int __xgeSvgItemContainsPoint(xge_svg pSvg, const xge_svg_draw_item_t* pItem, xge_shape_ex_matrix_t tParent, xge_vec2_t tPoint, float fTolerance, int* pContains, int iDepth)
{
	xge_rect_t tBounds;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pItem == NULL) || (pContains == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pContains = 0;
	memset(&tBounds, 0, sizeof(tBounds));
	if ( pItem->iType == XGE_SVG_DRAW_ITEM_SCENE ) {
		return __xgeSvgSceneContainsPointTransformed(pItem->u.pScene, tParent, tPoint, fTolerance, pContains);
	}
	if ( pItem->iType == XGE_SVG_DRAW_ITEM_SHAPE ) {
		return __xgeSvgShapeContainsPointTransformed(pItem->u.pShape, tParent, tPoint, fTolerance, pContains);
	}
	if ( pItem->iType == XGE_SVG_DRAW_ITEM_TEXT ) {
		iRet = __xgeSvgTextItemBounds(pSvg, &pItem->u.tText, tParent, fTolerance, &tBounds);
	} else if ( pItem->iType == XGE_SVG_DRAW_ITEM_SVG_IMAGE ) {
		return __xgeSvgImageItemContainsPoint(&pItem->u.tImage, tParent, tPoint, fTolerance, pContains, iDepth + 1);
	} else if ( pItem->iType == XGE_SVG_DRAW_ITEM_RASTER_IMAGE ) {
		return __xgeSvgRasterImageItemContainsPoint(&pItem->u.tRaster, tParent, tPoint, fTolerance, pContains);
	} else {
		iRet = XGE_OK;
	}
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	return __xgeSvgBoundsItemContainsPoint(tBounds, tPoint, pContains);
}

static int __xgeSvgContainsPointWithParent(xge_svg pSvg, xge_shape_ex_matrix_t tParent, xge_vec2_t tPoint, float fTolerance, int* pContains, int iDepth)
{
	int i;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pContains == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iDepth > XGE_SVG_BOUNDS_MAX_DEPTH ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pContains = 0;
	for ( i = pSvg->iItemCount - 1; i >= 0; i-- ) {
		int bItemContains;

		iRet = __xgeSvgItemContainsPoint(pSvg, &pSvg->pItems[i], tParent, tPoint, fTolerance, &bItemContains, iDepth + 1);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( bItemContains ) {
			*pContains = 1;
			return XGE_OK;
		}
	}
	return XGE_OK;
}

static int __xgeSvgDrawContainsPointInternal(xge_svg pSvg, xge_rect_t tDst, xge_vec2_t tPoint, float fTolerance, int* pContains, int iDepth)
{
	xge_shape_ex_matrix_t tMatrix;
	xge_rect_t tViewport;
	xge_rect_t tViewBox;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pContains == NULL) || !__xgeSvgFloatFinite(tPoint.fX) || !__xgeSvgFloatFinite(tPoint.fY) || !__xgeSvgFloatFinite(fTolerance) ||
	     !__xgeSvgFloatFinite(tDst.fX) || !__xgeSvgFloatFinite(tDst.fY) || !__xgeSvgFloatFinite(tDst.fW) || !__xgeSvgFloatFinite(tDst.fH) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pContains = 0;
	if ( iDepth > XGE_SVG_BOUNDS_MAX_DEPTH ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tViewBox = pSvg->tViewBox;
	if ( (tViewBox.fW <= 0.0f) || (tViewBox.fH <= 0.0f) || (tDst.fW <= 0.0f) || (tDst.fH <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeSvgRectContainsPoint(tDst, tPoint) ) {
		return XGE_OK;
	}
	iRet = xgeSvgGetDrawViewport(pSvg, tDst, &tViewport);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tMatrix = __xgeSvgViewBoxMatrix(tViewBox, tViewport);
	return __xgeSvgContainsPointWithParent(pSvg, tMatrix, tPoint, fTolerance, pContains, iDepth + 1);
}

static int __xgeSvgGetBoundsWithParent(xge_svg pSvg, xge_shape_ex_matrix_t tParent, float fTolerance, xge_rect_t* pBounds, int iDepth)
{
	xge_rect_t tOut;
	int bHasBounds;
	int iRet;
	int i;

	if ( !__xgeSvgValid(pSvg) || (pBounds == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iDepth > XGE_SVG_BOUNDS_MAX_DEPTH ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tOut, 0, sizeof(tOut));
	bHasBounds = 0;
	for ( i = 0; i < pSvg->iItemCount; i++ ) {
		xge_svg_draw_item_t* pItem = &pSvg->pItems[i];
		xge_rect_t tItemBounds;

		memset(&tItemBounds, 0, sizeof(tItemBounds));
		if ( pItem->iType == XGE_SVG_DRAW_ITEM_SCENE ) {
			iRet = __xgeSvgSceneBoundsTransformed(pItem->u.pScene, fTolerance, tParent, &tItemBounds);
		} else if ( pItem->iType == XGE_SVG_DRAW_ITEM_SHAPE ) {
			iRet = __xgeSvgShapeBoundsTransformed(pItem->u.pShape, fTolerance, tParent, 1, &tItemBounds);
		} else if ( pItem->iType == XGE_SVG_DRAW_ITEM_TEXT ) {
			iRet = __xgeSvgTextItemBounds(pSvg, &pItem->u.tText, tParent, fTolerance, &tItemBounds);
		} else if ( pItem->iType == XGE_SVG_DRAW_ITEM_SVG_IMAGE ) {
			iRet = __xgeSvgImageItemBounds(&pItem->u.tImage, tParent, fTolerance, &tItemBounds, iDepth + 1);
		} else if ( pItem->iType == XGE_SVG_DRAW_ITEM_RASTER_IMAGE ) {
			iRet = __xgeSvgRasterImageItemBounds(&pItem->u.tRaster, tParent, fTolerance, &tItemBounds);
		} else {
			iRet = XGE_OK;
		}
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		iRet = __xgeSvgBoundsUnion(&tOut, &bHasBounds, tItemBounds);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	*pBounds = bHasBounds ? tOut : (xge_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return XGE_OK;
}

static int __xgeSvgGetDrawBoundsInternal(xge_svg pSvg, xge_rect_t tDst, float fTolerance, xge_rect_t* pBounds, int iDepth)
{
	xge_shape_ex_matrix_t tMatrix;
	xge_rect_t tViewport;
	xge_rect_t tViewBox;
	int iRet;

	if ( !__xgeSvgValid(pSvg) || (pBounds == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pBounds, 0, sizeof(*pBounds));
	if ( iDepth > XGE_SVG_BOUNDS_MAX_DEPTH ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tViewBox = pSvg->tViewBox;
	if ( (tViewBox.fW <= 0.0f) || (tViewBox.fH <= 0.0f) || (tDst.fW <= 0.0f) || (tDst.fH <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = xgeSvgGetDrawViewport(pSvg, tDst, &tViewport);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tMatrix = __xgeSvgViewBoxMatrix(tViewBox, tViewport);
	iRet = __xgeSvgGetBoundsWithParent(pSvg, tMatrix, fTolerance, pBounds, iDepth + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	*pBounds = __xgeSvgRectIntersect(*pBounds, tDst);
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

static int __xgeSvgDrawBlendBegin(int bBlendSet, int iBlend, int* pOldBlend, int* pBlendApplied)
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
	if ( (iBlend < XGE_BLEND_NONE) || (iBlend > XGE_BLEND_LIGHTEN) ) {
		iBlend = XGE_BLEND_ALPHA;
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

static int __xgeSvgDrawBlendEnd(int iOldBlend, int bBlendApplied, int iRet)
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

static int __xgeSvgDrawInternal(xge_svg pSvg, xge_rect_t tDst, float fTolerance, int bScreenSpace)
{
	xge_shape_ex_matrix_t tMatrix;
	xge_rect_t tViewport;
	xge_rect_t tViewBox;
	xge_rect_t tOldRootClip;
	int bOldRootClipEnabled;
	int bRootClipApplied;
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
	iRet = __xgeSvgDrawClipBegin(tDst, &tOldRootClip, &bOldRootClipEnabled, &bRootClipApplied);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	for ( i = 0; i < pSvg->iItemCount; i++ ) {
		if ( pSvg->pItems[i].iType == XGE_SVG_DRAW_ITEM_SCENE ) {
			iRet = bScreenSpace ? xgeShapeExSceneDrawPxEx(pSvg->pItems[i].u.pScene, fTolerance, &tMatrix, 1.0f) : xgeShapeExSceneDrawEx(pSvg->pItems[i].u.pScene, fTolerance, &tMatrix, 1.0f);
		} else if ( pSvg->pItems[i].iType == XGE_SVG_DRAW_ITEM_SHAPE ) {
			int iOldBlend;
			int bBlendApplied;

			iRet = __xgeSvgDrawBlendBegin(pSvg->pItems[i].bBlendSet, pSvg->pItems[i].iBlend, &iOldBlend, &bBlendApplied);
			if ( iRet == XGE_OK ) {
				iRet = bScreenSpace ? xgeShapeExDrawPxEx(pSvg->pItems[i].u.pShape, fTolerance, &tMatrix, 1.0f) : xgeShapeExDrawEx(pSvg->pItems[i].u.pShape, fTolerance, &tMatrix, 1.0f);
				iRet = __xgeSvgDrawBlendEnd(iOldBlend, bBlendApplied, iRet);
			}
		} else if ( pSvg->pItems[i].iType == XGE_SVG_DRAW_ITEM_TEXT ) {
			xge_svg_text_item_t* pText = &pSvg->pItems[i].u.tText;
			xge_rect_t tOldClip;
			xge_shape_ex_matrix_t tItemMatrix;
			int bOldClipEnabled;
			int bClipApplied;
			int bMaskApplied;
			int iOldBlend;
			int bBlendApplied;

			iRet = __xgeShapeAutoBatchFlush();
			if ( iRet != XGE_OK ) {
				return __xgeSvgDrawClipEnd(tOldRootClip, bOldRootClipEnabled, bRootClipApplied, iRet);
			}
			tItemMatrix = __xgeSvgMatrixMul(tMatrix, pText->tStyle.tTransform);
			if ( pText->bClipRect ) {
				xge_rect_t tClip = __xgeSvgMatrixRectBounds(tItemMatrix, pText->tClipRect);

				iRet = __xgeSvgDrawClipBegin(tClip, &tOldClip, &bOldClipEnabled, &bClipApplied);
				if ( iRet != XGE_OK ) {
					return __xgeSvgDrawClipEnd(tOldRootClip, bOldRootClipEnabled, bRootClipApplied, iRet);
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
					iRet = __xgeSvgDrawClipEnd(tOldClip, bOldClipEnabled, bClipApplied, iRet);
					return __xgeSvgDrawClipEnd(tOldRootClip, bOldRootClipEnabled, bRootClipApplied, iRet);
				}
			}
			iRet = __xgeSvgDrawBlendBegin(pText->tStyle.bBlendSet, pText->tStyle.iBlend, &iOldBlend, &bBlendApplied);
			if ( iRet == XGE_OK ) {
				iRet = __xgeSvgDrawTextItem(pSvg, pText, tMatrix, bScreenSpace);
				iRet = __xgeSvgDrawBlendEnd(iOldBlend, bBlendApplied, iRet);
			}
			iRet = __xgeSvgDrawMaskShapeEnd(bMaskApplied, iRet);
			iRet = __xgeSvgDrawClipEnd(tOldClip, bOldClipEnabled, bClipApplied, iRet);
		} else if ( pSvg->pItems[i].iType == XGE_SVG_DRAW_ITEM_SVG_IMAGE ) {
			xge_svg_image_item_t* pImage = &pSvg->pItems[i].u.tImage;
			xge_rect_t tOldClip;
			xge_shape_ex_matrix_t tItemMatrix;
			int bOldClipEnabled;
			int bClipApplied;
			int bMaskApplied;
			int iOldBlend;
			int bBlendApplied;

			iRet = __xgeShapeAutoBatchFlush();
			if ( iRet != XGE_OK ) {
				return __xgeSvgDrawClipEnd(tOldRootClip, bOldRootClipEnabled, bRootClipApplied, iRet);
			}
			tItemMatrix = __xgeSvgMatrixMul(tMatrix, pImage->tStyle.tTransform);
			if ( pImage->bClipRect ) {
				xge_rect_t tClip = __xgeSvgMatrixRectBounds(tItemMatrix, pImage->tClipRect);

				iRet = __xgeSvgDrawClipBegin(tClip, &tOldClip, &bOldClipEnabled, &bClipApplied);
				if ( iRet != XGE_OK ) {
					return __xgeSvgDrawClipEnd(tOldRootClip, bOldRootClipEnabled, bRootClipApplied, iRet);
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
					iRet = __xgeSvgDrawClipEnd(tOldClip, bOldClipEnabled, bClipApplied, iRet);
					return __xgeSvgDrawClipEnd(tOldRootClip, bOldRootClipEnabled, bRootClipApplied, iRet);
				}
			}
			iRet = __xgeSvgDrawBlendBegin(pImage->tStyle.bBlendSet, pImage->tStyle.iBlend, &iOldBlend, &bBlendApplied);
			if ( iRet == XGE_OK ) {
				iRet = __xgeSvgDrawImageItem(pImage, tMatrix, fTolerance, bScreenSpace);
				iRet = __xgeSvgDrawBlendEnd(iOldBlend, bBlendApplied, iRet);
			}
			iRet = __xgeSvgDrawMaskShapeEnd(bMaskApplied, iRet);
			iRet = __xgeSvgDrawClipEnd(tOldClip, bOldClipEnabled, bClipApplied, iRet);
		} else if ( pSvg->pItems[i].iType == XGE_SVG_DRAW_ITEM_RASTER_IMAGE ) {
			xge_svg_raster_item_t* pImage = &pSvg->pItems[i].u.tRaster;
			xge_rect_t tOldClip;
			xge_shape_ex_matrix_t tItemMatrix;
			int bOldClipEnabled;
			int bClipApplied;
			int bMaskApplied;
			int iOldBlend;
			int bBlendApplied;

			iRet = __xgeShapeAutoBatchFlush();
			if ( iRet != XGE_OK ) {
				return __xgeSvgDrawClipEnd(tOldRootClip, bOldRootClipEnabled, bRootClipApplied, iRet);
			}
			tItemMatrix = __xgeSvgMatrixMul(tMatrix, pImage->tStyle.tTransform);
			if ( pImage->bClipRect ) {
				xge_rect_t tClip = __xgeSvgMatrixRectBounds(tItemMatrix, pImage->tClipRect);

				iRet = __xgeSvgDrawClipBegin(tClip, &tOldClip, &bOldClipEnabled, &bClipApplied);
				if ( iRet != XGE_OK ) {
					return __xgeSvgDrawClipEnd(tOldRootClip, bOldRootClipEnabled, bRootClipApplied, iRet);
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
					iRet = __xgeSvgDrawClipEnd(tOldClip, bOldClipEnabled, bClipApplied, iRet);
					return __xgeSvgDrawClipEnd(tOldRootClip, bOldRootClipEnabled, bRootClipApplied, iRet);
				}
			}
			iRet = __xgeSvgDrawBlendBegin(pImage->tStyle.bBlendSet, pImage->tStyle.iBlend, &iOldBlend, &bBlendApplied);
			if ( iRet == XGE_OK ) {
				iRet = __xgeSvgDrawRasterImageItem(pImage, tMatrix, bScreenSpace);
				iRet = __xgeSvgDrawBlendEnd(iOldBlend, bBlendApplied, iRet);
			}
			iRet = __xgeSvgDrawMaskShapeEnd(bMaskApplied, iRet);
			iRet = __xgeSvgDrawClipEnd(tOldClip, bOldClipEnabled, bClipApplied, iRet);
		} else {
			iRet = XGE_OK;
		}
		if ( iRet != XGE_OK ) {
			return __xgeSvgDrawClipEnd(tOldRootClip, bOldRootClipEnabled, bRootClipApplied, iRet);
		}
	}
	iRet = __xgeShapeAutoBatchFlush();
	return __xgeSvgDrawClipEnd(tOldRootClip, bOldRootClipEnabled, bRootClipApplied, iRet);
}

int xgeSvgGetBounds(xge_svg pSvg, float fTolerance, xge_rect_t* pBounds)
{
	if ( !__xgeSvgValid(pSvg) || (pBounds == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return __xgeSvgGetBoundsWithParent(pSvg, __xgeSvgMatrixIdentity(), fTolerance, pBounds, 0);
}

int xgeSvgGetDrawBounds(xge_svg pSvg, xge_rect_t tDst, float fTolerance, xge_rect_t* pBounds)
{
	return __xgeSvgGetDrawBoundsInternal(pSvg, tDst, fTolerance, pBounds, 0);
}

int xgeSvgContainsPoint(xge_svg pSvg, float fX, float fY, float fTolerance, int* pContains)
{
	xge_vec2_t tPoint;

	if ( !__xgeSvgValid(pSvg) || (pContains == NULL) || !__xgeSvgFloatFinite(fX) || !__xgeSvgFloatFinite(fY) || !__xgeSvgFloatFinite(fTolerance) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tPoint.fX = fX;
	tPoint.fY = fY;
	return __xgeSvgContainsPointWithParent(pSvg, __xgeSvgMatrixIdentity(), tPoint, fTolerance, pContains, 0);
}

int xgeSvgDrawContainsPoint(xge_svg pSvg, xge_rect_t tDst, float fX, float fY, float fTolerance, int* pContains)
{
	xge_vec2_t tPoint;

	if ( !__xgeSvgValid(pSvg) || (pContains == NULL) || !__xgeSvgFloatFinite(fX) || !__xgeSvgFloatFinite(fY) || !__xgeSvgFloatFinite(fTolerance) ||
	     !__xgeSvgFloatFinite(tDst.fX) || !__xgeSvgFloatFinite(tDst.fY) || !__xgeSvgFloatFinite(tDst.fW) || !__xgeSvgFloatFinite(tDst.fH) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pContains = 0;
	tPoint.fX = fX;
	tPoint.fY = fY;
	return __xgeSvgDrawContainsPointInternal(pSvg, tDst, tPoint, fTolerance, pContains, 0);
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
