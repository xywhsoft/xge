#ifndef XUI_H
#define XUI_H

#include <stddef.h>
#include <stdint.h>
#include "lib/xrt/xrt.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XUI_VERSION_MAJOR	2
#define XUI_VERSION_MINOR	0
#define XUI_VERSION_PATCH	0

#define XUI_PROXY_VERSION	2

typedef enum xui_result_t {
	XUI_OK = 0,
	XUI_ERROR = -1,
	XUI_ERROR_INVALID_ARGUMENT = -2,
	XUI_ERROR_NOT_INITIALIZED = -3,
	XUI_ERROR_ALREADY_INITIALIZED = -4,
	XUI_ERROR_OUT_OF_MEMORY = -5,
	XUI_ERROR_FILE_NOT_FOUND = -6,
	XUI_ERROR_UNSUPPORTED = -7,
	XUI_ERROR_BACKEND_FAILED = -8,
	XUI_ERROR_GPU_FAILED = -9,
	XUI_ERROR_RESOURCE_FAILED = -10
} xui_result_t;

#define XUI_COLOR_RGBA(r, g, b, a)	((((uint32_t)(r) & 0xFFu) << 24) | (((uint32_t)(g) & 0xFFu) << 16) | (((uint32_t)(b) & 0xFFu) << 8) | ((uint32_t)(a) & 0xFFu))
#define XUI_COLOR_WHITE			XUI_COLOR_RGBA(255, 255, 255, 255)

#define XUI_SURFACE_KIND_TEXTURE	1
#define XUI_SURFACE_FORMAT_RGBA8	1

#define XUI_SURFACE_ALPHA_PREMULTIPLIED	0x0001
#define XUI_SURFACE_ALPHA_STRAIGHT	0x0002
#define XUI_SURFACE_USAGE_TARGET	0x0100

#define XUI_SURFACE_DRAW_FLIP_X		0x0001
#define XUI_SURFACE_DRAW_FLIP_Y		0x0002
#define XUI_SURFACE_DRAW_SCREEN_SPACE	0x0004

#define XUI_SURFACE_FILTER_NEAREST	1
#define XUI_SURFACE_FILTER_LINEAR	2

#define XUI_SURFACE_WRAP_CLAMP		1
#define XUI_SURFACE_WRAP_REPEAT		2

#define XUI_PROXY_CAP_SURFACE_TARGET	0x00000001u
#define XUI_PROXY_CAP_SURFACE_READ	0x00000002u
#define XUI_PROXY_CAP_SURFACE_QUAD	0x00000004u
#define XUI_PROXY_CAP_SURFACE_CLEAR_RECT	0x00000008u
#define XUI_PROXY_CAP_SURFACE_SAMPLER	0x00000010u
#define XUI_PROXY_CAP_DRAW_CONTEXT	0x00000020u
#define XUI_PROXY_CAP_SHAPE		0x00000040u
#define XUI_PROXY_CAP_FONT_TTF		0x00000080u
#define XUI_PROXY_CAP_FONT_XRF		0x00000100u
#define XUI_PROXY_CAP_TEXT		0x00000200u

#define XUI_FONT_FORMAT_TTF		0x0001
#define XUI_FONT_FORMAT_XRF		0x0002

#define XUI_TEXT_ALIGN_LEFT		0x0000
#define XUI_TEXT_ALIGN_CENTER		0x0001
#define XUI_TEXT_ALIGN_RIGHT		0x0002
#define XUI_TEXT_ALIGN_TOP		0x0000
#define XUI_TEXT_ALIGN_MIDDLE		0x0010
#define XUI_TEXT_ALIGN_BOTTOM		0x0020
#define XUI_TEXT_CLIP			0x0100
#define XUI_TEXT_UNDERLINE		0x0200

#define XUI_TEXT_WRAP_NONE		0
#define XUI_TEXT_WRAP_WORD		1
#define XUI_TEXT_WRAP_CHAR		2

#define XUI_TEXT_BREAK_NONE		0
#define XUI_TEXT_BREAK_WRAP		1
#define XUI_TEXT_BREAK_NEWLINE		2
#define XUI_TEXT_BREAK_END		3

#define XUI_WIDGET_DIRTY_LAYOUT		0x00000001u
#define XUI_WIDGET_DIRTY_STYLE		0x00000002u
#define XUI_WIDGET_DIRTY_CACHE		0x00000004u
#define XUI_WIDGET_DIRTY_RENDER		0x00000008u
#define XUI_WIDGET_DIRTY_TREE		0x00000010u
#define XUI_WIDGET_DIRTY_ALL		0x0000001Fu

#define XUI_WIDGET_UPDATE_CLEAR		0x00000001u

#define XUI_CACHE_POLICY_NONE		0
#define XUI_CACHE_POLICY_SELF		1
#define XUI_CACHE_POLICY_SUBTREE	2
#define XUI_CACHE_POLICY_SUBTREE_TILED	3
#define XUI_CACHE_POLICY_DISPLAY_LIST	4
#define XUI_CACHE_POLICY_AUTO		5

#define XUI_CACHE_UPDATE_ALL_STATES	0x00000001u
#define XUI_CACHE_CLEAR_ON_UPDATE	0x00000002u
#define XUI_CACHE_PINNED		0x00000004u

#define XUI_WIDGET_TYPE_DEFAULT_LAYOUT		0x00000001u
#define XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY	0x00000002u

#define XUI_RESOURCE_SURFACE		1
#define XUI_RESOURCE_FONT		2
#define XUI_RESOURCE_STYLE		3
#define XUI_RESOURCE_THEME		4
#define XUI_RESOURCE_USER		1024

#define XUI_STYLE_VALUE_NONE		0
#define XUI_STYLE_VALUE_INT		1
#define XUI_STYLE_VALUE_FLOAT		2
#define XUI_STYLE_VALUE_COLOR		3
#define XUI_STYLE_VALUE_BOOL		4
#define XUI_STYLE_VALUE_STRING		5
#define XUI_STYLE_VALUE_TOKEN		6

#define XUI_STYLE_DIRTY_DEFAULT \
	(XUI_WIDGET_DIRTY_STYLE | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER)

#define XUI_STYLE_PROPERTY_INHERITED	0x00000001u
#define XUI_STYLE_PROPERTY_PRIVATE	0x00000002u

#define XUI_WIDGET_STATE_HOVER		0x00000001u
#define XUI_WIDGET_STATE_ACTIVE		0x00000002u
#define XUI_WIDGET_STATE_FOCUS		0x00000004u
#define XUI_WIDGET_STATE_DISABLED	0x00000008u

#define XUI_WIDGET_HIT_SELF		0x00000001u
#define XUI_WIDGET_HIT_CHILDREN		0x00000002u
#define XUI_WIDGET_HIT_VISIBLE		0x00000004u
#define XUI_WIDGET_HIT_ENABLED		0x00000008u
#define XUI_WIDGET_HIT_DEFAULT		(XUI_WIDGET_HIT_SELF | XUI_WIDGET_HIT_CHILDREN | XUI_WIDGET_HIT_VISIBLE | XUI_WIDGET_HIT_ENABLED)

#define XUI_POINTER_BUTTON_LEFT		0x00000001u
#define XUI_POINTER_BUTTON_RIGHT	0x00000002u
#define XUI_POINTER_BUTTON_MIDDLE	0x00000004u

#define XUI_MOD_SHIFT			0x00000001u
#define XUI_MOD_CTRL			0x00000002u
#define XUI_MOD_ALT			0x00000004u
#define XUI_MOD_SUPER			0x00000008u

#define XUI_EVENT_NONE			0
#define XUI_EVENT_POINTER_ENTER		1
#define XUI_EVENT_POINTER_LEAVE		2
#define XUI_EVENT_POINTER_MOVE		3
#define XUI_EVENT_POINTER_DOWN		4
#define XUI_EVENT_POINTER_UP		5
#define XUI_EVENT_POINTER_CLICK		6
#define XUI_EVENT_POINTER_WHEEL		7
#define XUI_EVENT_KEY_DOWN		8
#define XUI_EVENT_KEY_UP		9
#define XUI_EVENT_TEXT			10
#define XUI_EVENT_FOCUS		11
#define XUI_EVENT_BLUR			12
#define XUI_EVENT_VIEWPORT		13
#define XUI_EVENT_DPI			14
#define XUI_EVENT_POINTER_CAPTURE_LOST	15
#define XUI_EVENT_IME_COMPOSITION	16
#define XUI_EVENT_POINTER_DOUBLE_CLICK	17
#define XUI_EVENT_CONTEXT_MENU		18
#define XUI_EVENT_DRAG_BEGIN		19
#define XUI_EVENT_DRAG_MOVE		20
#define XUI_EVENT_DRAG_END		21
#define XUI_EVENT_DRAG_CANCEL		22
#define XUI_EVENT_HOTKEY		23
#define XUI_EVENT_COMMAND		24
#define XUI_EVENT_BOUNDS_CHANGED	25
#define XUI_EVENT_VISIBLE_CHANGED	26
#define XUI_EVENT_ENABLED_CHANGED	27

#define XUI_EVENT_PHASE_CAPTURE		1
#define XUI_EVENT_PHASE_TARGET		2
#define XUI_EVENT_PHASE_BUBBLE		3

#define XUI_EVENT_DISPATCH_STOP		0x00000001u

#define XUI_EVENT_MASK_POINTER_ENTER		0x0000000000000001ull
#define XUI_EVENT_MASK_POINTER_LEAVE		0x0000000000000002ull
#define XUI_EVENT_MASK_POINTER_MOVE		0x0000000000000004ull
#define XUI_EVENT_MASK_POINTER_DOWN		0x0000000000000008ull
#define XUI_EVENT_MASK_POINTER_UP		0x0000000000000010ull
#define XUI_EVENT_MASK_POINTER_CLICK		0x0000000000000020ull
#define XUI_EVENT_MASK_POINTER_WHEEL		0x0000000000000040ull
#define XUI_EVENT_MASK_DOUBLE_CLICK		0x0000000000000080ull
#define XUI_EVENT_MASK_CONTEXT_MENU		0x0000000000000100ull
#define XUI_EVENT_MASK_KEY_DOWN			0x0000000000000200ull
#define XUI_EVENT_MASK_KEY_UP			0x0000000000000400ull
#define XUI_EVENT_MASK_TEXT			0x0000000000000800ull
#define XUI_EVENT_MASK_HOTKEY			0x0000000000001000ull
#define XUI_EVENT_MASK_COMMAND			0x0000000000002000ull
#define XUI_EVENT_MASK_FOCUS			0x0000000000004000ull
#define XUI_EVENT_MASK_CAPTURE			0x0000000000008000ull
#define XUI_EVENT_MASK_DRAG			0x0000000000010000ull
#define XUI_EVENT_MASK_STATE			0x0000000000020000ull
#define XUI_EVENT_MASK_IME			0x0000000000040000ull
#define XUI_EVENT_MASK_TOOLTIP			0x0000000000080000ull
#define XUI_EVENT_MASK_POINTER			(XUI_EVENT_MASK_POINTER_ENTER | XUI_EVENT_MASK_POINTER_LEAVE | XUI_EVENT_MASK_POINTER_MOVE | XUI_EVENT_MASK_POINTER_DOWN | XUI_EVENT_MASK_POINTER_UP | XUI_EVENT_MASK_POINTER_CLICK | XUI_EVENT_MASK_POINTER_WHEEL | XUI_EVENT_MASK_DOUBLE_CLICK | XUI_EVENT_MASK_CONTEXT_MENU)
#define XUI_EVENT_MASK_KEYBOARD			(XUI_EVENT_MASK_KEY_DOWN | XUI_EVENT_MASK_KEY_UP | XUI_EVENT_MASK_TEXT | XUI_EVENT_MASK_HOTKEY | XUI_EVENT_MASK_COMMAND)

#define XUI_KEY_TAB			9
#define XUI_KEY_SPACE			32
#define XUI_KEY_ENTER			13
#define XUI_KEY_ESCAPE			27
#define XUI_KEY_CONTEXT_MENU		93

#define XUI_IME_DISABLED		0
#define XUI_IME_ENABLED			1
#define XUI_IME_AUTO			2

#define XUI_LAYER_NORMAL		0
#define XUI_LAYER_FLOATING		1
#define XUI_LAYER_POPUP			2
#define XUI_LAYER_MODAL			3
#define XUI_LAYER_TOOLTIP		4
#define XUI_LAYER_DRAG			5
#define XUI_LAYER_DEBUG			6

#define XUI_TOOLTIP_NONE			0
#define XUI_TOOLTIP_TEXT			1
#define XUI_TOOLTIP_CUSTOM			2
#define XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM	0
#define XUI_TOOLTIP_ANCHOR_WIDGET_TOP		1
#define XUI_TOOLTIP_ANCHOR_WIDGET_RIGHT		2
#define XUI_TOOLTIP_ANCHOR_WIDGET_LEFT		3
#define XUI_TOOLTIP_ANCHOR_CURSOR		4

#define XUI_NINE_PATCH_STRETCH		0
#define XUI_NINE_PATCH_TILE		1

#define XUI_BUTTON_STATE_CHECKED	0x00000010u

#define XUI_BUTTON_SEMANTIC_DEFAULT	0
#define XUI_BUTTON_SEMANTIC_PRIMARY	1
#define XUI_BUTTON_SEMANTIC_DANGER	2

#define XUI_BUTTON_ICON_LEFT		0
#define XUI_BUTTON_ICON_RIGHT		1
#define XUI_BUTTON_ICON_TOP		2
#define XUI_BUTTON_ICON_BOTTOM		3

#define XUI_BUTTON_BADGE_CONTENT_TOP_RIGHT	0
#define XUI_BUTTON_BADGE_WIDGET_TOP_RIGHT	1
#define XUI_BUTTON_BADGE_ICON_TOP_RIGHT		2
#define XUI_BUTTON_BADGE_TEXT_TOP_RIGHT		3

#define XUI_IMAGE_NATURAL		0
#define XUI_IMAGE_STRETCH		1
#define XUI_IMAGE_CONTAIN		2
#define XUI_IMAGE_FIT			XUI_IMAGE_CONTAIN
#define XUI_IMAGE_COVER		3
#define XUI_IMAGE_SCALE_DOWN		4
#define XUI_IMAGE_CUSTOM		5
#define XUI_IMAGE_CENTER		XUI_IMAGE_NATURAL

#define XUI_SEPARATOR_HORIZONTAL	0
#define XUI_SEPARATOR_VERTICAL		1
#define XUI_ORIENTATION_HORIZONTAL	XUI_SEPARATOR_HORIZONTAL
#define XUI_ORIENTATION_VERTICAL	XUI_SEPARATOR_VERTICAL
#define XUI_SEPARATOR_SOLID		0
#define XUI_SEPARATOR_DOT		1
#define XUI_SEPARATOR_DASH		2
#define XUI_SEPARATOR_DASH_DOT		3

#define XUI_PROGRESS_LEFT_TO_RIGHT	0
#define XUI_PROGRESS_RIGHT_TO_LEFT	1
#define XUI_PROGRESS_BOTTOM_TO_TOP	2
#define XUI_PROGRESS_TOP_TO_BOTTOM	3
#define XUI_PROGRESS_FILL_STRETCH	0
#define XUI_PROGRESS_FILL_REVEAL	1

#define XUI_WIDGET_EVENT_SLOT_COUNT	48

#define XUI_LAYOUT_UNBOUNDED		1073741824.0f

#define XUI_LAYOUT_MANUAL		0
#define XUI_LAYOUT_OVERLAY		1
#define XUI_LAYOUT_ROW			2
#define XUI_LAYOUT_COLUMN		3
#define XUI_LAYOUT_FLOW			4
#define XUI_LAYOUT_TABLE		5
#define XUI_LAYOUT_DOCK		6
#define XUI_LAYOUT_GRID		7

#define XUI_SIZE_FIXED			0
#define XUI_SIZE_CONTENT		1
#define XUI_SIZE_FILL			2

#define XUI_FLOW_BLOCK			0
#define XUI_FLOW_INLINE			1
#define XUI_FLOW_INLINE_BLOCK		2
#define XUI_FLOW_NONE			3
#define XUI_FLOW_ABSOLUTE		4

#define XUI_ALIGN_START			0
#define XUI_ALIGN_CENTER		1
#define XUI_ALIGN_END			2
#define XUI_ALIGN_STRETCH		3

#define XUI_DOCK_LEFT			1
#define XUI_DOCK_TOP			2
#define XUI_DOCK_RIGHT			3
#define XUI_DOCK_BOTTOM		4
#define XUI_DOCK_FILL			5

#define XUI_OVERFLOW_VISIBLE		0
#define XUI_OVERFLOW_HIDDEN		1
#define XUI_OVERFLOW_CLIP		2
#define XUI_OVERFLOW_REPORT		3

#if defined(_WIN32) || defined(_WIN64)
	#if defined(XUI_DLL)
		#if defined(XUI_BUILD_DLL)
			#define XUI_API __declspec(dllexport)
		#else
			#define XUI_API __declspec(dllimport)
		#endif
	#else
		#define XUI_API
	#endif
#else
	#define XUI_API
#endif

typedef struct xui_vec2_t {
	float fX;
	float fY;
} xui_vec2_t;

typedef struct xui_rect_t {
	float fX;
	float fY;
	float fW;
	float fH;
} xui_rect_t;

typedef struct xui_rect_i_t {
	int iX;
	int iY;
	int iW;
	int iH;
} xui_rect_i_t;

typedef struct xui_thickness_t {
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;
} xui_thickness_t;

typedef struct xui_layout_t {
	int iLayoutType;
	int iWidthMode;
	int iHeightMode;
	int iFlowMode;
	int iDock;
	int iOverflow;
	int iAlignX;
	int iAlignY;
	int iTableRow;
	int iTableColumn;
	int iTableRowSpan;
	int iTableColumnSpan;
	int iGridColumnCount;
	int iLayer;
	int iZIndex;
	float fPreferredWidth;
	float fPreferredHeight;
	float fMinWidth;
	float fMinHeight;
	float fMaxWidth;
	float fMaxHeight;
	float fGrow;
	float fShrink;
	float fGap;
	float fGridItemWidth;
	float fGridItemHeight;
	float fBaseline;
	xui_thickness_t tMargin;
	xui_thickness_t tPadding;
} xui_layout_t;

typedef struct xui_table_track_t {
	int iSizeMode;
	float fValue;
	float fMin;
	float fMax;
	float fWeight;
} xui_table_track_t;

typedef struct xui_surface_desc_t {
	int iKind;
	int iFormat;
	int iWidth;
	int iHeight;
	uint32_t iFlags;
} xui_surface_desc_t;

typedef struct xui_surface_vertex_t {
	float fX;
	float fY;
	float fZ;
	float fW;
	float fU;
	float fV;
	uint32_t iColor;
} xui_surface_vertex_t;

typedef struct xui_surface_sampler_t {
	int iMinFilter;
	int iMagFilter;
	int iWrapS;
	int iWrapT;
} xui_surface_sampler_t;

typedef struct xui_proxy_caps_t {
	uint32_t iSize;
	uint32_t iCaps;
	int iSurfaceFormat;
	int iInternalAlpha;
	xui_surface_sampler_t tDefaultSampler;
} xui_proxy_caps_t;

typedef struct xui_font_metrics_t {
	float fSize;
	float fAscent;
	float fDescent;
	float fLineGap;
	float fLineHeight;
} xui_font_metrics_t;

typedef struct xui_text_layout_desc_t {
	uint32_t iSize;
	const char* sText;
	int iTextSize;
	struct xui_font_t* pFont;
	float fMaxWidth;
	float fMaxHeight;
	int iWrapMode;
	uint32_t iFlags;
	float fLineGap;
	float fParagraphGap;
} xui_text_layout_desc_t;

typedef struct xui_text_line_t {
	uint32_t iSize;
	int iTextOffset;
	int iTextSize;
	int iBreakType;
	float fX;
	float fY;
	float fW;
	float fH;
	float fBaseline;
} xui_text_line_t;

typedef struct xui_label_desc_t {
	uint32_t iSize;
	const char* sText;
	struct xui_font_t* pFont;
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iTextFlags;
	int iWrapMode;
	int bUnderline;
	float fLineGap;
	float fParagraphGap;
} xui_label_desc_t;

typedef struct xui_image_desc_t {
	uint32_t iSize;
	struct xui_surface_t* pSurface;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	uint32_t iColor;
	int iMode;
	int iAlignX;
	int iAlignY;
} xui_image_desc_t;

typedef struct xui_separator_desc_t {
	uint32_t iSize;
	uint32_t iColor;
	float fThickness;
	int iOrientation;
	int iAlign;
	int iLineStyle;
} xui_separator_desc_t;

typedef struct xui_context_t xui_context_t;
typedef struct xui_widget_t xui_widget_t;
typedef struct xui_widget_type_t xui_widget_type_t;

typedef struct xui_nine_patch_t {
	uint32_t iSize;
	struct xui_surface_t* pSurface;
	xui_rect_t tSrc;
	xui_thickness_t tSlice;
	uint32_t iColor;
	int iMode;
} xui_nine_patch_t;

typedef struct xui_progress_desc_t {
	uint32_t iSize;
	struct xui_font_t* pFont;
	const char* sText;
	float fMin;
	float fMax;
	float fValue;
	uint32_t iTrackColor;
	uint32_t iFillColor;
	uint32_t iTextColor;
	uint32_t iFillTextColor;
	uint32_t iTextFlags;
	int iFillDirection;
	int iFillPatchMode;
	xui_nine_patch_t tTrackPatch;
	xui_nine_patch_t tFillPatch;
	int bHasTrackPatch;
	int bHasFillPatch;
} xui_progress_desc_t;

typedef void (*xui_button_click_proc)(xui_widget_t* pWidget, void* pUser);

typedef struct xui_button_desc_t {
	uint32_t iSize;
	const char* sText;
	struct xui_font_t* pFont;
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iTextFlags;
	uint32_t iNormalColor;
	uint32_t iHoverColor;
	uint32_t iActiveColor;
	uint32_t iFocusColor;
	uint32_t iDisabledColor;
	uint32_t iCheckedColor;
	float fRadius;
	float fBorderWidth;
	uint32_t iBorderColor;
} xui_button_desc_t;

typedef struct xui_cache_policy_t {
	uint32_t iSize;
	int iPolicy;
	uint32_t iFlags;
	uint32_t iClearColor;
	int iTileWidth;
	int iTileHeight;
	size_t iMaxBytes;
} xui_cache_policy_t;

typedef struct xui_cache_stats_t {
	uint32_t iSize;
	size_t iBudgetBytes;
	size_t iUsedBytes;
	int iSurfaceCount;
	uint32_t iGeneration;
} xui_cache_stats_t;

typedef struct xui_render_stats_t {
	uint32_t iSize;
	uint32_t iGeneration;
	int iUpdatedWidgets;
	int iUpdatedCaches;
	int iDrawnCaches;
	int iSkippedWidgets;
} xui_render_stats_t;

typedef struct xui_style_value_t {
	uint32_t iSize;
	int iType;
	int iInt;
	float fFloat;
	uint32_t iColor;
	const char* sText;
} xui_style_value_t;

typedef struct xui_style_property_t {
	uint32_t iSize;
	uint32_t iPropertyId;
	const char* sName;
	uint32_t iDirtyFlags;
	xui_style_value_t tValue;
} xui_style_property_t;

typedef struct xui_state_style_desc_t {
	uint32_t iSize;
	const char* sClass;
	uint32_t iStateMask;
	const xui_style_property_t* pProperties;
	int iPropertyCount;
} xui_state_style_desc_t;

typedef struct xui_style_desc_t {
	uint32_t iSize;
	const char* sName;
	const char* sParent;
	const xui_style_property_t* pProperties;
	int iPropertyCount;
} xui_style_desc_t;

typedef struct xui_style_property_info_t {
	uint32_t iSize;
	uint32_t iPropertyId;
	const char* sName;
	int iValueType;
	uint32_t iDirtyFlags;
	uint32_t iFlags;
	xui_widget_type_t* pWidgetType;
	xui_style_value_t tDefaultValue;
} xui_style_property_info_t;

typedef struct xui_render_node_t {
	uint32_t iSize;
	xui_widget_t* pWidget;
	xui_rect_t tWorldRect;
	xui_rect_t tPaintRect;
	int iLayer;
	int iZIndex;
	uint32_t iStateId;
	int iCachePolicy;
	uint32_t iFlags;
} xui_render_node_t;

typedef struct xui_resource_desc_t {
	uint32_t iSize;
	const char* sName;
	int iKind;
	void* pHandle;
	void* pUser;
	void (*onDestroy)(xui_context_t* pContext, void* pHandle, void* pUser);
} xui_resource_desc_t;

typedef struct xui_surface_t xui_surface_t;
typedef struct xui_font_t xui_font_t;
typedef struct xui_draw_context_t xui_draw_context_t;
typedef struct xui_proxy_t xui_proxy_t;
typedef struct xui_text_layout_t xui_text_layout_t;
typedef struct xui_painter_t xui_painter_t;
typedef struct xui_resource_t xui_resource_t;
typedef struct xui_event_t xui_event_t;

typedef xui_context_t* xui_context;
typedef xui_widget_t* xui_widget;
typedef xui_widget_type_t* xui_widget_type;
typedef xui_surface_t* xui_surface;
typedef xui_font_t* xui_font;
typedef xui_draw_context_t* xui_draw_context;
typedef xui_proxy_t* xui_proxy;
typedef xui_text_layout_t* xui_text_layout;
typedef xui_painter_t* xui_painter;
typedef xui_resource_t* xui_resource;

typedef int (*xui_widget_content_measure_proc)(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser);
typedef int (*xui_widget_layout_measure_proc)(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser);
typedef int (*xui_widget_layout_arrange_proc)(xui_widget pWidget, xui_rect_t tContentRect, void* pUser);
typedef int (*xui_widget_cache_render_proc)(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser);
typedef int (*xui_widget_event_proc)(xui_widget pWidget, const xui_event_t* pEvent, void* pUser);
typedef void (*xui_widget_action_proc)(xui_widget pWidget, void* pUser);
typedef xui_rect_t (*xui_widget_ime_rect_proc)(xui_widget pWidget, void* pUser);
typedef xui_vec2_t (*xui_tooltip_measure_proc)(xui_context pContext, xui_widget pOwner, void* pUser);
typedef int (*xui_tooltip_paint_proc)(xui_context pContext, xui_widget pOwner, xui_draw_context pDraw, xui_rect_t tRect, void* pUser);

typedef struct xui_tooltip_desc_t {
	uint32_t iSize;
	int iType;
	const char* sText;
	int iAnchor;
	int bCustomAnchorRect;
	xui_rect_t tAnchorRect;
	float fOffsetX;
	float fOffsetY;
	float fDelay;
	int bFollowCursor;
	xui_tooltip_measure_proc onMeasure;
	xui_tooltip_paint_proc onPaint;
	void* pUser;
} xui_tooltip_desc_t;

typedef int (*xui_tooltip_resolve_proc)(xui_context pContext, xui_widget pWidget, xui_tooltip_desc_t* pDesc, void* pUser);

struct xui_event_t {
	uint32_t iSize;
	int iType;
	int iPhase;
	uint32_t iFlags;
	xui_widget pTarget;
	xui_widget pCurrentTarget;
	xui_widget pRelated;
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	int iButton;
	uint32_t iButtons;
	int iKey;
	uint32_t iCodepoint;
	uint32_t iModifiers;
	int iCommand;
	const char* sCommand;
	void* pData;
	char sText[256];
	int iTextSize;
	int iCompositionStart;
	int iCompositionLength;
};

typedef struct xui_theme_t {
	uint32_t iSize;
	xui_font pFont;
	uint32_t iTextColor;
	uint32_t iBackgroundColor;
	uint32_t iPanelColor;
	uint32_t iBorderColor;
	uint32_t iAccentColor;
	uint32_t iSelectionColor;
	uint32_t iStateNormalColor;
	uint32_t iStateHoverColor;
	uint32_t iStateActiveColor;
	uint32_t iStateFocusColor;
	uint32_t iStateDisabledColor;
	float fRadius;
	float fPadding;
	float fSpacing;
	float fBorderWidth;
} xui_theme_t;

typedef struct xui_chrome_style_t {
	uint32_t iSize;
	uint32_t iBackdropColor;
	uint32_t iPopupColor;
	uint32_t iPopupBorderColor;
	uint32_t iTooltipColor;
	uint32_t iTooltipTextColor;
	uint32_t iModalOverlayColor;
	float fRadius;
	float fBorderWidth;
	float fShadowSize;
} xui_chrome_style_t;

typedef struct xui_debug_widget_info_t {
	uint32_t iSize;
	xui_widget pWidget;
	xui_widget pParent;
	int iDepth;
	int iChildCount;
	int bVisible;
	int bEnabled;
	int bFocusable;
	int bTabStop;
	int bFocusScope;
	int bHitTestVisible;
	int bDragEnabled;
	int iImeMode;
	int iLayer;
	int iZIndex;
	uint32_t iDirtyFlags;
	uint64_t iEventMask;
	uint64_t iSubtreeEventMask;
	xui_rect_t tRect;
	xui_rect_t tWorldRect;
} xui_debug_widget_info_t;

typedef int (*xui_widget_type_init_proc)(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser);
typedef void (*xui_widget_type_destroy_proc)(xui_widget pWidget, void* pTypeData, void* pUser);

typedef struct xui_widget_type_desc_t {
	uint32_t iSize;
	const char* sName;
	xui_widget_type pParent;
	uint32_t iFlags;
	size_t iTypeDataSize;
	void* pUser;
	xui_widget_type_init_proc onInit;
	xui_widget_type_destroy_proc onDestroy;
	xui_widget_content_measure_proc onContentMeasure;
	xui_widget_layout_measure_proc onLayoutMeasure;
	xui_widget_layout_arrange_proc onLayoutArrange;
	xui_widget_cache_render_proc onCacheRender;
	xui_layout_t tLayout;
	xui_cache_policy_t tCachePolicy;
} xui_widget_type_desc_t;

typedef int (*xui_proxy_get_caps_proc)(xui_proxy pProxy, xui_proxy_caps_t* pCaps);
typedef int (*xui_clipboard_set_text_proc)(xui_proxy pProxy, const char* sText);
typedef int (*xui_clipboard_get_text_proc)(xui_proxy pProxy, char* sText, int iCapacity);
typedef int (*xui_ime_get_enabled_proc)(xui_proxy pProxy);
typedef int (*xui_ime_set_enabled_proc)(xui_proxy pProxy, int bEnabled);
typedef int (*xui_ime_set_candidate_rect_proc)(xui_proxy pProxy, xui_rect_t tRect);
typedef int (*xui_surface_create_proc)(xui_proxy pProxy, xui_surface* ppSurface, const xui_surface_desc_t* pDesc);
typedef int (*xui_surface_create_rgba_proc)(xui_proxy pProxy, xui_surface* ppSurface, int iWidth, int iHeight, const void* pPixels, int iStride, uint32_t iFlags);
typedef int (*xui_surface_load_file_proc)(xui_proxy pProxy, xui_surface* ppSurface, const char* sPath, uint32_t iFlags);
typedef int (*xui_surface_load_memory_proc)(xui_proxy pProxy, xui_surface* ppSurface, const void* pData, int iSize, uint32_t iFlags);
typedef int (*xui_surface_update_rgba_proc)(xui_proxy pProxy, xui_surface pSurface, xui_rect_i_t tRect, const void* pPixels, int iStride);
typedef int (*xui_surface_read_rgba_proc)(xui_proxy pProxy, xui_surface pSurface, void* pPixels, int iStride);
typedef int (*xui_surface_get_desc_proc)(xui_proxy pProxy, xui_surface pSurface, xui_surface_desc_t* pDesc);
typedef int (*xui_surface_draw_proc)(xui_proxy pProxy, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags);
typedef int (*xui_surface_clear_proc)(xui_proxy pProxy, xui_surface pTarget, uint32_t iColor);
typedef int (*xui_surface_clear_rect_proc)(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor);
typedef int (*xui_surface_draw_to_proc)(xui_proxy pProxy, xui_surface pTarget, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags);
typedef int (*xui_surface_draw_quad_proc)(xui_proxy pProxy, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags);
typedef int (*xui_surface_draw_quad_to_proc)(xui_proxy pProxy, xui_surface pTarget, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags);
typedef int (*xui_surface_get_sampler_proc)(xui_proxy pProxy, xui_surface pSurface, xui_surface_sampler_t* pSampler);
typedef int (*xui_surface_set_sampler_proc)(xui_proxy pProxy, xui_surface pSurface, const xui_surface_sampler_t* pSampler);
typedef int (*xui_surface_get_generation_proc)(xui_proxy pProxy, xui_surface pSurface, uint32_t* pGeneration);
typedef void (*xui_surface_destroy_proc)(xui_proxy pProxy, xui_surface pSurface);
typedef int (*xui_shape_point_proc)(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fSize, uint32_t iColor);
typedef int (*xui_shape_line_proc)(xui_proxy pProxy, xui_surface pTarget, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor);
typedef int (*xui_shape_triangle_fill_proc)(xui_proxy pProxy, xui_surface pTarget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor);
typedef int (*xui_shape_triangle_stroke_proc)(xui_proxy pProxy, xui_surface pTarget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor);
typedef int (*xui_shape_rect_fill_proc)(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor);
typedef int (*xui_shape_rect_stroke_proc)(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, float fWidth, uint32_t iColor);
typedef int (*xui_shape_circle_fill_proc)(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fRadius, uint32_t iColor);
typedef int (*xui_shape_circle_stroke_proc)(xui_proxy pProxy, xui_surface pTarget, float fX, float fY, float fRadius, float fWidth, uint32_t iColor);
typedef int (*xui_shape_round_rect_fill_proc)(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, float fRadius, uint32_t iColor);
typedef int (*xui_shape_round_rect_stroke_proc)(xui_proxy pProxy, xui_surface pTarget, xui_rect_t tRect, float fRadius, float fWidth, uint32_t iColor);
typedef int (*xui_font_load_file_proc)(xui_proxy pProxy, xui_font* ppFont, const char* sPath, float fSize, uint32_t iFlags);
typedef int (*xui_font_load_memory_proc)(xui_proxy pProxy, xui_font* ppFont, const void* pData, int iSize, float fSize, uint32_t iFlags);
typedef int (*xui_font_get_metrics_proc)(xui_proxy pProxy, xui_font pFont, xui_font_metrics_t* pMetrics);
typedef void (*xui_font_destroy_proc)(xui_proxy pProxy, xui_font pFont);
typedef int (*xui_text_measure_proc)(xui_proxy pProxy, xui_font pFont, const char* sText, xui_vec2_t* pSize);
typedef int (*xui_text_draw_proc)(xui_proxy pProxy, xui_surface pTarget, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags);
typedef int (*xui_draw_begin_proc)(xui_proxy pProxy, xui_draw_context* ppDraw, xui_surface pTarget);
typedef int (*xui_draw_end_proc)(xui_proxy pProxy, xui_draw_context pDraw);
typedef int (*xui_draw_clear_rect_proc)(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor);
typedef int (*xui_draw_surface_proc)(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags);
typedef int (*xui_draw_surface_quad_proc)(xui_proxy pProxy, xui_draw_context pDraw, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags);
typedef int (*xui_draw_point_proc)(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fSize, uint32_t iColor);
typedef int (*xui_draw_line_proc)(xui_proxy pProxy, xui_draw_context pDraw, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor);
typedef int (*xui_draw_triangle_fill_proc)(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor);
typedef int (*xui_draw_triangle_stroke_proc)(xui_proxy pProxy, xui_draw_context pDraw, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor);
typedef int (*xui_draw_rect_fill_proc)(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor);
typedef int (*xui_draw_rect_stroke_proc)(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor);
typedef int (*xui_draw_circle_fill_proc)(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, uint32_t iColor);
typedef int (*xui_draw_circle_stroke_proc)(xui_proxy pProxy, xui_draw_context pDraw, float fX, float fY, float fRadius, float fWidth, uint32_t iColor);
typedef int (*xui_draw_round_rect_fill_proc)(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fRadius, uint32_t iColor);
typedef int (*xui_draw_round_rect_stroke_proc)(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fRadius, float fWidth, uint32_t iColor);
typedef int (*xui_draw_text_proc)(xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags);

struct xui_proxy_t {
	uint32_t iSize;
	uint32_t iVersion;
	void* pUser;
	xui_proxy_get_caps_proc getCaps;
	xui_clipboard_set_text_proc clipboardSetText;
	xui_clipboard_get_text_proc clipboardGetText;
	xui_ime_get_enabled_proc imeGetEnabled;
	xui_ime_set_enabled_proc imeSetEnabled;
	xui_ime_set_candidate_rect_proc imeSetCandidateRect;
	xui_surface_create_proc surfaceCreate;
	xui_surface_create_rgba_proc surfaceCreateRGBA;
	xui_surface_load_file_proc surfaceLoadFile;
	xui_surface_load_memory_proc surfaceLoadMemory;
	xui_surface_update_rgba_proc surfaceUpdateRGBA;
	xui_surface_read_rgba_proc surfaceReadRGBA;
	xui_surface_get_desc_proc surfaceGetDesc;
	xui_surface_draw_proc surfaceDraw;
	xui_surface_clear_proc surfaceClear;
	xui_surface_clear_rect_proc surfaceClearRect;
	xui_surface_draw_to_proc surfaceDrawTo;
	xui_surface_draw_quad_proc surfaceDrawQuad;
	xui_surface_draw_quad_to_proc surfaceDrawQuadTo;
	xui_surface_get_sampler_proc surfaceGetSampler;
	xui_surface_set_sampler_proc surfaceSetSampler;
	xui_surface_get_generation_proc surfaceGetGeneration;
	xui_surface_destroy_proc surfaceDestroy;
	xui_shape_point_proc shapePoint;
	xui_shape_line_proc shapeLine;
	xui_shape_triangle_fill_proc shapeTriangleFill;
	xui_shape_triangle_stroke_proc shapeTriangleStroke;
	xui_shape_rect_fill_proc shapeRectFill;
	xui_shape_rect_stroke_proc shapeRectStroke;
	xui_shape_circle_fill_proc shapeCircleFill;
	xui_shape_circle_stroke_proc shapeCircleStroke;
	xui_shape_round_rect_fill_proc shapeRoundRectFill;
	xui_shape_round_rect_stroke_proc shapeRoundRectStroke;
	xui_font_load_file_proc fontLoadFile;
	xui_font_load_memory_proc fontLoadMemory;
	xui_font_get_metrics_proc fontGetMetrics;
	xui_font_destroy_proc fontDestroy;
	xui_text_measure_proc textMeasure;
	xui_text_draw_proc textDraw;
	xui_draw_begin_proc drawBegin;
	xui_draw_end_proc drawEnd;
	xui_draw_clear_rect_proc drawClearRect;
	xui_draw_surface_proc drawSurface;
	xui_draw_surface_quad_proc drawSurfaceQuad;
	xui_draw_point_proc drawPoint;
	xui_draw_line_proc drawLine;
	xui_draw_triangle_fill_proc drawTriangleFill;
	xui_draw_triangle_stroke_proc drawTriangleStroke;
	xui_draw_rect_fill_proc drawRectFill;
	xui_draw_rect_stroke_proc drawRectStroke;
	xui_draw_circle_fill_proc drawCircleFill;
	xui_draw_circle_stroke_proc drawCircleStroke;
	xui_draw_round_rect_fill_proc drawRoundRectFill;
	xui_draw_round_rect_stroke_proc drawRoundRectStroke;
	xui_draw_text_proc drawText;
};

XUI_API int xuiCreate(xui_context* ppContext);
XUI_API void xuiDestroy(xui_context pContext);
XUI_API int xuiSetProxy(xui_context pContext, const xui_proxy_t* pProxy);
XUI_API int xuiGetProxy(xui_context pContext, xui_proxy_t* pProxy);
XUI_API int xuiGetProxyCaps(xui_context pContext, xui_proxy_caps_t* pCaps);
XUI_API int xuiUpdate(xui_context pContext, float fDelta);
XUI_API int xuiSetViewportSize(xui_context pContext, float fWidth, float fHeight);
XUI_API xui_vec2_t xuiGetViewportSize(xui_context pContext);
XUI_API int xuiSetVirtualDpi(xui_context pContext, float fDpiScale);
XUI_API float xuiGetVirtualDpi(xui_context pContext);
XUI_API int xuiInvalidateRect(xui_context pContext, xui_rect_i_t tRect);
XUI_API int xuiInvalidateAll(xui_context pContext);
XUI_API int xuiHasDamage(xui_context pContext);
XUI_API int xuiGetDamageRects(xui_context pContext, xui_rect_i_t* pRects, int iCapacity);
XUI_API void xuiClearDamage(xui_context pContext);
XUI_API int xuiRenderPrepare(xui_context pContext);
XUI_API int xuiGetRenderStats(xui_context pContext, xui_render_stats_t* pStats);
XUI_API int xuiRender(xui_context pContext, xui_surface pTarget, const xui_rect_i_t* pRects, int iRectCount);
XUI_API int xuiLayout(xui_context pContext);
XUI_API int xuiSetCacheBudget(xui_context pContext, size_t iBudgetBytes);
XUI_API size_t xuiGetCacheBudget(xui_context pContext);
XUI_API int xuiGetCacheStats(xui_context pContext, xui_cache_stats_t* pStats);
XUI_API int xuiPurgeCaches(xui_context pContext, size_t iTargetBytes);

XUI_API int xuiStyleBeginUpdate(xui_context pContext);
XUI_API int xuiStyleEndUpdate(xui_context pContext);
XUI_API uint32_t xuiStyleGetGeneration(xui_context pContext);
XUI_API int xuiStyleRefresh(xui_context pContext);
XUI_API int xuiStyleSetNamed(xui_context pContext, const xui_style_desc_t* pStyle);
XUI_API int xuiStyleRemoveNamed(xui_context pContext, const char* sName);
XUI_API int xuiStyleSetClass(xui_context pContext, const char* sClass, const xui_style_desc_t* pStyle);
XUI_API int xuiStyleRemoveClass(xui_context pContext, const char* sClass);
XUI_API int xuiStyleSetType(xui_context pContext, xui_widget_type pType, const xui_style_desc_t* pStyle);
XUI_API int xuiStyleRemoveType(xui_context pContext, xui_widget_type pType);
XUI_API int xuiStyleSetStateClass(xui_context pContext, const xui_state_style_desc_t* pStyle);
XUI_API int xuiStyleRemoveStateClass(xui_context pContext, const char* sClass, uint32_t iStateMask);
XUI_API int xuiStyleSetDefault(xui_context pContext, const xui_style_property_t* pProperties, int iPropertyCount);
XUI_API int xuiStyleClearDefault(xui_context pContext);
XUI_API int xuiStyleRegisterProperty(xui_context pContext, const xui_style_property_info_t* pInfo, uint32_t* pPropertyId);
XUI_API uint32_t xuiStyleFindProperty(xui_context pContext, const char* sName);
XUI_API int xuiStyleGetPropertyInfo(xui_context pContext, uint32_t iPropertyId, xui_style_property_info_t* pInfo);
XUI_API int xuiStyleSetToken(xui_context pContext, const char* sName, const xui_style_value_t* pValue);
XUI_API int xuiStyleRemoveToken(xui_context pContext, const char* sName);
XUI_API int xuiStyleGetToken(xui_context pContext, const char* sName, xui_style_value_t* pValue);
XUI_API uint32_t xuiStyleGetTokenGeneration(xui_context pContext);
XUI_API void xuiThemeDefault(xui_theme_t* pTheme);
XUI_API int xuiSetTheme(xui_context pContext, const xui_theme_t* pTheme);
XUI_API int xuiGetTheme(xui_context pContext, xui_theme_t* pTheme);
XUI_API int xuiSetChromeStyle(xui_context pContext, const xui_chrome_style_t* pChrome);
XUI_API int xuiGetChromeStyle(xui_context pContext, xui_chrome_style_t* pChrome);
XUI_API int xuiSetDefaultFont(xui_context pContext, xui_font pFont);
XUI_API xui_font xuiGetDefaultFont(xui_context pContext);
XUI_API int xuiRegisterFont(xui_context pContext, const char* sName, xui_font pFont);
XUI_API xui_font xuiFindFont(xui_context pContext, const char* sName);
XUI_API void xuiClearFonts(xui_context pContext);

XUI_API int xuiResourceSet(xui_context pContext, xui_resource* ppResource, const xui_resource_desc_t* pDesc);
XUI_API xui_resource xuiResourceFind(xui_context pContext, const char* sName);
XUI_API int xuiResourceRemove(xui_resource pResource);
XUI_API int xuiResourceTouch(xui_resource pResource);
XUI_API const char* xuiResourceGetName(xui_resource pResource);
XUI_API int xuiResourceGetKind(xui_resource pResource);
XUI_API void* xuiResourceGetHandle(xui_resource pResource);
XUI_API uint32_t xuiResourceGetGeneration(xui_resource pResource);
XUI_API int xuiResourceAddRef(xui_resource pResource);
XUI_API int xuiResourceRelease(xui_resource pResource);
XUI_API int xuiResourceGetRefCount(xui_resource pResource);
XUI_API int xuiResourceAddDependency(xui_resource pResource, xui_resource pDependency);
XUI_API void xuiResourceClearDependencies(xui_resource pResource);
XUI_API int xuiResourceGetDependencyCount(xui_resource pResource);
XUI_API xui_resource xuiResourceGetDependency(xui_resource pResource, int iIndex);

XUI_API int xuiPainterBegin(xui_context pContext, xui_surface pTarget, xui_painter* ppPainter);
XUI_API int xuiPainterEnd(xui_painter pPainter);
XUI_API xui_draw_context xuiPainterGetDrawContext(xui_painter pPainter);
XUI_API int xuiPainterClearRect(xui_painter pPainter, xui_rect_t tRect, uint32_t iColor);
XUI_API int xuiPainterDrawSurface(xui_painter pPainter, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags);
XUI_API int xuiPainterDrawSurfaceQuad(xui_painter pPainter, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags);
XUI_API int xuiPainterFillRect(xui_painter pPainter, xui_rect_t tRect, uint32_t iColor);
XUI_API int xuiPainterStrokeRect(xui_painter pPainter, xui_rect_t tRect, float fWidth, uint32_t iColor);
XUI_API int xuiPainterFillRoundRect(xui_painter pPainter, xui_rect_t tRect, float fRadius, uint32_t iColor);
XUI_API int xuiPainterStrokeRoundRect(xui_painter pPainter, xui_rect_t tRect, float fRadius, float fWidth, uint32_t iColor);
XUI_API int xuiPainterDrawText(xui_painter pPainter, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags);
XUI_API int xuiPainterDrawNinePatch(xui_painter pPainter, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, xui_thickness_t tSlice, uint32_t iColor, uint32_t iFlags);

XUI_API int xuiBuildRenderTree(xui_context pContext);
XUI_API int xuiGetRenderNodeCount(xui_context pContext);
XUI_API int xuiGetRenderNode(xui_context pContext, int iIndex, xui_render_node_t* pNode);

XUI_API int xuiInputPointerMove(xui_context pContext, float fX, float fY, uint32_t iButtons);
XUI_API int xuiInputPointerDown(xui_context pContext, float fX, float fY, int iButton, uint32_t iButtons);
XUI_API int xuiInputPointerUp(xui_context pContext, float fX, float fY, int iButton, uint32_t iButtons);
XUI_API int xuiInputPointerWheel(xui_context pContext, float fX, float fY, float fWheelX, float fWheelY, uint32_t iButtons);
XUI_API int xuiInputPointerLeave(xui_context pContext);
XUI_API int xuiInputKeyDown(xui_context pContext, int iKey, uint32_t iModifiers);
XUI_API int xuiInputKeyUp(xui_context pContext, int iKey, uint32_t iModifiers);
XUI_API int xuiInputText(xui_context pContext, uint32_t iCodepoint);
XUI_API int xuiInputImeComposition(xui_context pContext, const char* sText, int iTextSize, int iCompositionStart, int iCompositionLength);
XUI_API int xuiInputViewport(xui_context pContext, float fWidth, float fHeight);
XUI_API int xuiInputDpi(xui_context pContext, float fDpiScale);
XUI_API int xuiPollEvent(xui_context pContext, xui_event_t* pEvent);
XUI_API void xuiClearEvents(xui_context pContext);
XUI_API int xuiDispatchEvent(xui_context pContext, const xui_event_t* pEvent);
XUI_API int xuiDispatchPendingEvents(xui_context pContext);
XUI_API xui_widget xuiHitTest(xui_context pContext, float fX, float fY, uint32_t iFlags);
XUI_API xui_widget xuiGetHoverWidget(xui_context pContext);
XUI_API xui_widget xuiGetActiveWidget(xui_context pContext);
XUI_API xui_widget xuiGetFocusWidget(xui_context pContext);
XUI_API int xuiSetFocusWidget(xui_context pContext, xui_widget pWidget);
XUI_API int xuiSetPointerCapture(xui_context pContext, xui_widget pWidget);
XUI_API int xuiReleasePointerCapture(xui_context pContext, xui_widget pWidget);
XUI_API xui_widget xuiGetPointerCapture(xui_context pContext);
XUI_API int xuiFocusNext(xui_context pContext, int iForward);
XUI_API int xuiHotKeyRegister(xui_context pContext, xui_widget pWidget, int iKey, uint32_t iModifiers, xui_widget_event_proc onEvent, void* pUser);
XUI_API int xuiHotKeyRegisterCommand(xui_context pContext, xui_widget pWidget, int iKey, uint32_t iModifiers, int iCommand, const char* sCommand, void* pData);
XUI_API int xuiHotKeyUnregister(xui_context pContext, xui_widget pWidget, int iKey, uint32_t iModifiers);
XUI_API int xuiHotKeyClearWidget(xui_widget pWidget);
XUI_API int xuiCommandDispatch(xui_context pContext, xui_widget pTarget, int iCommand, const char* sCommand, void* pData);

XUI_API int xuiTextLayoutCreate(xui_context pContext, xui_text_layout* ppLayout, const xui_text_layout_desc_t* pDesc);
XUI_API void xuiTextLayoutDestroy(xui_text_layout pLayout);
XUI_API int xuiTextLayoutReset(xui_text_layout pLayout, const xui_text_layout_desc_t* pDesc);
XUI_API xui_vec2_t xuiTextLayoutGetSize(xui_text_layout pLayout);
XUI_API int xuiTextLayoutGetLineCount(xui_text_layout pLayout);
XUI_API int xuiTextLayoutGetLine(xui_text_layout pLayout, int iIndex, xui_text_line_t* pLine);
XUI_API const char* xuiTextLayoutGetText(xui_text_layout pLayout);
XUI_API int xuiTextLayoutGetTruncated(xui_text_layout pLayout);
XUI_API int xuiTextLayoutDraw(xui_text_layout pLayout, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags);
XUI_API int xuiTextMeasureLayout(xui_context pContext, const xui_text_layout_desc_t* pDesc, xui_vec2_t* pSize);

XUI_API xui_widget_type xuiLabelGetType(xui_context pContext);
XUI_API int xuiLabelCreate(xui_context pContext, xui_widget* ppWidget, const xui_label_desc_t* pDesc);
XUI_API int xuiLabelSetText(xui_widget pWidget, const char* sText);
XUI_API const char* xuiLabelGetText(xui_widget pWidget);
XUI_API int xuiLabelSetFont(xui_widget pWidget, xui_font pFont);
XUI_API xui_font xuiLabelGetFont(xui_widget pWidget);
XUI_API int xuiLabelSetTextColor(xui_widget pWidget, uint32_t iColor);
XUI_API uint32_t xuiLabelGetTextColor(xui_widget pWidget);
XUI_API int xuiLabelSetDisabledTextColor(xui_widget pWidget, uint32_t iColor);
XUI_API uint32_t xuiLabelGetDisabledTextColor(xui_widget pWidget);
XUI_API int xuiLabelSetTextFlags(xui_widget pWidget, uint32_t iTextFlags);
XUI_API uint32_t xuiLabelGetTextFlags(xui_widget pWidget);
XUI_API int xuiLabelSetWrapMode(xui_widget pWidget, int iWrapMode);
XUI_API int xuiLabelGetWrapMode(xui_widget pWidget);
XUI_API int xuiLabelSetUnderline(xui_widget pWidget, int bUnderline);
XUI_API int xuiLabelGetUnderline(xui_widget pWidget);
XUI_API int xuiLabelSetLineGap(xui_widget pWidget, float fLineGap);
XUI_API float xuiLabelGetLineGap(xui_widget pWidget);
XUI_API int xuiLabelSetParagraphGap(xui_widget pWidget, float fParagraphGap);
XUI_API float xuiLabelGetParagraphGap(xui_widget pWidget);

XUI_API xui_widget_type xuiImageGetType(xui_context pContext);
XUI_API int xuiImageCreate(xui_context pContext, xui_widget* ppWidget, const xui_image_desc_t* pDesc);
XUI_API int xuiImageSetSurface(xui_widget pWidget, xui_surface pSurface);
XUI_API xui_surface xuiImageGetSurface(xui_widget pWidget);
XUI_API int xuiImageSetSource(xui_widget pWidget, xui_rect_t tSrc);
XUI_API int xuiImageSetSourceRect(xui_widget pWidget, float fX1, float fY1, float fX2, float fY2);
XUI_API int xuiImageClearSource(xui_widget pWidget);
XUI_API xui_rect_t xuiImageGetSource(xui_widget pWidget);
XUI_API int xuiImageSetColor(xui_widget pWidget, uint32_t iColor);
XUI_API int xuiImageSetTint(xui_widget pWidget, uint32_t iColor);
XUI_API uint32_t xuiImageGetColor(xui_widget pWidget);
XUI_API int xuiImageSetMode(xui_widget pWidget, int iMode);
XUI_API int xuiImageGetMode(xui_widget pWidget);
XUI_API int xuiImageSetAlign(xui_widget pWidget, int iAlignX, int iAlignY);
XUI_API int xuiImageGetAlign(xui_widget pWidget, int* pAlignX, int* pAlignY);
XUI_API int xuiImageSetCustomRect(xui_widget pWidget, float fX1, float fY1, float fX2, float fY2);
XUI_API xui_rect_t xuiImageGetCustomRect(xui_widget pWidget);
XUI_API xui_rect_t xuiImageGetDrawRect(xui_widget pWidget);

XUI_API xui_widget_type xuiSeparatorGetType(xui_context pContext);
XUI_API int xuiSeparatorCreate(xui_context pContext, xui_widget* ppWidget, const xui_separator_desc_t* pDesc);
XUI_API int xuiSeparatorSetColor(xui_widget pWidget, uint32_t iColor);
XUI_API uint32_t xuiSeparatorGetColor(xui_widget pWidget);
XUI_API int xuiSeparatorSetThickness(xui_widget pWidget, float fThickness);
XUI_API float xuiSeparatorGetThickness(xui_widget pWidget);
XUI_API int xuiSeparatorSetOrientation(xui_widget pWidget, int iOrientation);
XUI_API int xuiSeparatorGetOrientation(xui_widget pWidget);
XUI_API int xuiSeparatorSetAlign(xui_widget pWidget, int iAlign);
XUI_API int xuiSeparatorGetAlign(xui_widget pWidget);
XUI_API int xuiSeparatorSetLineStyle(xui_widget pWidget, int iLineStyle);
XUI_API int xuiSeparatorGetLineStyle(xui_widget pWidget);
XUI_API xui_rect_t xuiSeparatorGetLineRect(xui_widget pWidget);

XUI_API xui_widget_type xuiProgressGetType(xui_context pContext);
XUI_API int xuiProgressCreate(xui_context pContext, xui_widget* ppWidget, const xui_progress_desc_t* pDesc);
XUI_API int xuiProgressSetRange(xui_widget pWidget, float fMin, float fMax);
XUI_API int xuiProgressGetRange(xui_widget pWidget, float* pMin, float* pMax);
XUI_API int xuiProgressSetValue(xui_widget pWidget, float fValue);
XUI_API float xuiProgressGetValue(xui_widget pWidget);
XUI_API float xuiProgressGetRate(xui_widget pWidget);
XUI_API int xuiProgressSetText(xui_widget pWidget, xui_font pFont, const char* sText);
XUI_API int xuiProgressSetTextTemplate(xui_widget pWidget, const char* sTextTemplate);
XUI_API const char* xuiProgressGetTextTemplate(xui_widget pWidget);
XUI_API const char* xuiProgressGetDisplayText(xui_widget pWidget);
XUI_API int xuiProgressSetFont(xui_widget pWidget, xui_font pFont);
XUI_API xui_font xuiProgressGetFont(xui_widget pWidget);
XUI_API int xuiProgressSetTextColor(xui_widget pWidget, uint32_t iColor);
XUI_API uint32_t xuiProgressGetTextColor(xui_widget pWidget);
XUI_API int xuiProgressSetFillTextColor(xui_widget pWidget, uint32_t iColor);
XUI_API uint32_t xuiProgressGetFillTextColor(xui_widget pWidget);
XUI_API int xuiProgressSetTextFlags(xui_widget pWidget, uint32_t iTextFlags);
XUI_API uint32_t xuiProgressGetTextFlags(xui_widget pWidget);
XUI_API int xuiProgressSetColors(xui_widget pWidget, uint32_t iTrack, uint32_t iFill);
XUI_API uint32_t xuiProgressGetTrackColor(xui_widget pWidget);
XUI_API uint32_t xuiProgressGetFillColor(xui_widget pWidget);
XUI_API int xuiProgressSetFillDirection(xui_widget pWidget, int iFillDirection);
XUI_API int xuiProgressGetFillDirection(xui_widget pWidget);
XUI_API int xuiProgressSetTrackPatch(xui_widget pWidget, const xui_nine_patch_t* pPatch);
XUI_API int xuiProgressClearTrackPatch(xui_widget pWidget);
XUI_API int xuiProgressHasTrackPatch(xui_widget pWidget);
XUI_API int xuiProgressSetFillPatch(xui_widget pWidget, const xui_nine_patch_t* pPatch);
XUI_API int xuiProgressClearFillPatch(xui_widget pWidget);
XUI_API int xuiProgressHasFillPatch(xui_widget pWidget);
XUI_API int xuiProgressSetFillPatchMode(xui_widget pWidget, int iMode);
XUI_API int xuiProgressGetFillPatchMode(xui_widget pWidget);
XUI_API xui_rect_t xuiProgressGetFillRect(xui_widget pWidget);

XUI_API xui_widget_type xuiButtonGetType(xui_context pContext);
XUI_API int xuiButtonCreate(xui_context pContext, xui_widget* ppWidget, const xui_button_desc_t* pDesc);
XUI_API int xuiButtonSetClick(xui_widget pWidget, xui_button_click_proc onClick, void* pUser);
XUI_API int xuiButtonSetText(xui_widget pWidget, const char* sText);
XUI_API const char* xuiButtonGetText(xui_widget pWidget);
XUI_API int xuiButtonSetFont(xui_widget pWidget, xui_font pFont);
XUI_API xui_font xuiButtonGetFont(xui_widget pWidget);
XUI_API int xuiButtonSetTextColor(xui_widget pWidget, uint32_t iColor);
XUI_API uint32_t xuiButtonGetTextColor(xui_widget pWidget);
XUI_API int xuiButtonSetDisabledTextColor(xui_widget pWidget, uint32_t iColor);
XUI_API uint32_t xuiButtonGetDisabledTextColor(xui_widget pWidget);
XUI_API int xuiButtonSetSelectable(xui_widget pWidget, int bSelectable);
XUI_API int xuiButtonSetSelected(xui_widget pWidget, int bSelected);
XUI_API int xuiButtonIsSelected(xui_widget pWidget);
XUI_API int xuiButtonSetSemantic(xui_widget pWidget, int iSemantic);
XUI_API int xuiButtonGetSemantic(xui_widget pWidget);
XUI_API int xuiButtonSetIcon(xui_widget pWidget, xui_surface pSurface, xui_rect_t tSrc);
XUI_API xui_surface xuiButtonGetIconSurface(xui_widget pWidget);
XUI_API int xuiButtonSetIconColor(xui_widget pWidget, uint32_t iColor);
XUI_API int xuiButtonSetIconLayout(xui_widget pWidget, int iPlacement, float fIconSize, float fGap);
XUI_API int xuiButtonSetColors(xui_widget pWidget, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);
XUI_API int xuiButtonSetStateVisual(xui_widget pWidget, uint32_t iState, uint32_t iFill, float fBorderWidth, uint32_t iBorderColor);
XUI_API int xuiButtonSetBorder(xui_widget pWidget, float fBorderWidth, uint32_t iBorderColor);
XUI_API int xuiButtonSetRadius(xui_widget pWidget, float fRadius);
XUI_API int xuiButtonSetPatch(xui_widget pWidget, uint32_t iState, const xui_nine_patch_t* pPatch);
XUI_API int xuiButtonClearPatch(xui_widget pWidget, uint32_t iState);
XUI_API int xuiButtonHasPatch(xui_widget pWidget, uint32_t iState);
XUI_API int xuiButtonSetBadgeVisible(xui_widget pWidget, int bVisible);
XUI_API int xuiButtonGetBadgeVisible(xui_widget pWidget);
XUI_API int xuiButtonSetBadgeAnchor(xui_widget pWidget, int iAnchor);
XUI_API int xuiButtonSetBadgeOffset(xui_widget pWidget, float fX, float fY);
XUI_API int xuiButtonSetBadgeSize(xui_widget pWidget, float fSize);
XUI_API int xuiButtonSetBadgeSurface(xui_widget pWidget, xui_surface pSurface, xui_rect_t tSrc);
XUI_API uint32_t xuiButtonGetState(xui_widget pWidget);
XUI_API int xuiButtonGetClickCount(xui_widget pWidget);

XUI_API int xuiSetRootWidget(xui_context pContext, xui_widget pWidget);
XUI_API xui_widget xuiGetRootWidget(xui_context pContext);

XUI_API xui_widget_type xuiWidgetGetBaseType(void);
XUI_API int xuiWidgetRegisterType(xui_context pContext, xui_widget_type* ppType, const xui_widget_type_desc_t* pDesc);
XUI_API int xuiWidgetUnregisterType(xui_widget_type pType);
XUI_API xui_widget_type xuiWidgetFindType(xui_context pContext, const char* sName);
XUI_API const char* xuiWidgetTypeGetName(xui_widget_type pType);
XUI_API xui_widget_type xuiWidgetTypeGetParent(xui_widget_type pType);
XUI_API void* xuiWidgetTypeGetUserData(xui_widget_type pType);

XUI_API int xuiWidgetCreate(xui_context pContext, xui_widget* ppWidget);
XUI_API int xuiWidgetCreateTyped(xui_context pContext, xui_widget_type pType, xui_widget* ppWidget, const void* pCreateData);
XUI_API void xuiWidgetDestroy(xui_widget pWidget);
XUI_API xui_context xuiWidgetGetContext(xui_widget pWidget);
XUI_API xui_widget_type xuiWidgetGetType(xui_widget pWidget);
XUI_API int xuiWidgetIsType(xui_widget pWidget, xui_widget_type pType);
XUI_API void* xuiWidgetGetTypeData(xui_widget pWidget);
XUI_API void xuiWidgetSetUserData(xui_widget pWidget, void* pUser);
XUI_API void* xuiWidgetGetUserData(xui_widget pWidget);

XUI_API int xuiWidgetAddChild(xui_widget pParent, xui_widget pChild);
XUI_API int xuiWidgetInsertBefore(xui_widget pParent, xui_widget pChild, xui_widget pBefore);
XUI_API int xuiWidgetRemoveFromParent(xui_widget pWidget);
XUI_API xui_widget xuiWidgetGetParent(xui_widget pWidget);
XUI_API xui_widget xuiWidgetGetFirstChild(xui_widget pWidget);
XUI_API xui_widget xuiWidgetGetLastChild(xui_widget pWidget);
XUI_API xui_widget xuiWidgetGetPrevSibling(xui_widget pWidget);
XUI_API xui_widget xuiWidgetGetNextSibling(xui_widget pWidget);
XUI_API int xuiWidgetGetChildCount(xui_widget pWidget);

XUI_API int xuiWidgetSetRect(xui_widget pWidget, xui_rect_t tRect);
XUI_API xui_rect_t xuiWidgetGetRect(xui_widget pWidget);
XUI_API xui_rect_t xuiWidgetGetWorldRect(xui_widget pWidget);
XUI_API xui_rect_t xuiWidgetGetContentRect(xui_widget pWidget);
XUI_API int xuiWidgetSetLayout(xui_widget pWidget, const xui_layout_t* pLayout);
XUI_API xui_layout_t xuiWidgetGetLayout(xui_widget pWidget);
XUI_API int xuiWidgetSetContentMeasureCallback(xui_widget pWidget, xui_widget_content_measure_proc onMeasure, void* pUser);
XUI_API int xuiWidgetGetContentMeasureCallback(xui_widget pWidget, xui_widget_content_measure_proc* pMeasure, void** ppUser);
XUI_API int xuiWidgetSetLayoutCallbacks(xui_widget pWidget, xui_widget_layout_measure_proc onMeasure, xui_widget_layout_arrange_proc onArrange, void* pUser);
XUI_API int xuiWidgetGetLayoutCallbacks(xui_widget pWidget, xui_widget_layout_measure_proc* pMeasure, xui_widget_layout_arrange_proc* pArrange, void** ppUser);
XUI_API int xuiWidgetMeasureContent(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pContentSize);
XUI_API int xuiWidgetSetLayoutType(xui_widget pWidget, int iLayoutType);
XUI_API int xuiWidgetGetLayoutType(xui_widget pWidget);
XUI_API int xuiWidgetSetSizeMode(xui_widget pWidget, int iWidthMode, int iHeightMode);
XUI_API int xuiWidgetGetSizeMode(xui_widget pWidget, int* pWidthMode, int* pHeightMode);
XUI_API int xuiWidgetSetFlowMode(xui_widget pWidget, int iFlowMode);
XUI_API int xuiWidgetGetFlowMode(xui_widget pWidget);
XUI_API int xuiWidgetSetDock(xui_widget pWidget, int iDock);
XUI_API int xuiWidgetGetDock(xui_widget pWidget);
XUI_API int xuiWidgetSetOverflow(xui_widget pWidget, int iOverflow);
XUI_API int xuiWidgetGetOverflow(xui_widget pWidget);
XUI_API int xuiWidgetSetGridMetrics(xui_widget pWidget, int iColumnCount, float fItemWidth, float fItemHeight);
XUI_API int xuiWidgetGetGridMetrics(xui_widget pWidget, int* pColumnCount, float* pItemWidth, float* pItemHeight);
XUI_API int xuiWidgetSetLayer(xui_widget pWidget, int iLayer, int iZIndex);
XUI_API int xuiWidgetGetLayer(xui_widget pWidget, int* pLayer, int* pZIndex);
XUI_API int xuiWidgetSetPreferredSize(xui_widget pWidget, xui_vec2_t tSize);
XUI_API xui_vec2_t xuiWidgetGetPreferredSize(xui_widget pWidget);
XUI_API int xuiWidgetSetMinSize(xui_widget pWidget, xui_vec2_t tSize);
XUI_API xui_vec2_t xuiWidgetGetMinSize(xui_widget pWidget);
XUI_API int xuiWidgetSetMaxSize(xui_widget pWidget, xui_vec2_t tSize);
XUI_API xui_vec2_t xuiWidgetGetMaxSize(xui_widget pWidget);
XUI_API int xuiWidgetSetMargin(xui_widget pWidget, xui_thickness_t tMargin);
XUI_API xui_thickness_t xuiWidgetGetMargin(xui_widget pWidget);
XUI_API int xuiWidgetSetPadding(xui_widget pWidget, xui_thickness_t tPadding);
XUI_API xui_thickness_t xuiWidgetGetPadding(xui_widget pWidget);
XUI_API int xuiWidgetSetGap(xui_widget pWidget, float fGap);
XUI_API float xuiWidgetGetGap(xui_widget pWidget);
XUI_API int xuiWidgetSetFlex(xui_widget pWidget, float fGrow, float fShrink);
XUI_API int xuiWidgetGetFlex(xui_widget pWidget, float* pGrow, float* pShrink);
XUI_API int xuiWidgetSetAlign(xui_widget pWidget, int iAlignX, int iAlignY);
XUI_API int xuiWidgetGetAlign(xui_widget pWidget, int* pAlignX, int* pAlignY);
XUI_API int xuiWidgetSetTableSize(xui_widget pWidget, int iRows, int iColumns);
XUI_API int xuiWidgetGetTableSize(xui_widget pWidget, int* pRows, int* pColumns);
XUI_API int xuiWidgetSetTableRow(xui_widget pWidget, int iRow, const xui_table_track_t* pTrack);
XUI_API int xuiWidgetGetTableRow(xui_widget pWidget, int iRow, xui_table_track_t* pTrack);
XUI_API int xuiWidgetSetTableColumn(xui_widget pWidget, int iColumn, const xui_table_track_t* pTrack);
XUI_API int xuiWidgetGetTableColumn(xui_widget pWidget, int iColumn, xui_table_track_t* pTrack);
XUI_API int xuiWidgetSetTableCell(xui_widget pWidget, int iRow, int iColumn, int iRowSpan, int iColumnSpan);
XUI_API int xuiWidgetGetTableCell(xui_widget pWidget, int* pRow, int* pColumn, int* pRowSpan, int* pColumnSpan);
XUI_API int xuiWidgetMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pMeasuredSize);
XUI_API int xuiWidgetArrange(xui_widget pWidget, xui_rect_t tRect);
XUI_API int xuiWidgetSetVisible(xui_widget pWidget, int bVisible);
XUI_API int xuiWidgetGetVisible(xui_widget pWidget);
XUI_API int xuiWidgetSetEnabled(xui_widget pWidget, int bEnabled);
XUI_API int xuiWidgetGetEnabled(xui_widget pWidget);
XUI_API uint32_t xuiWidgetGetInputState(xui_widget pWidget);
XUI_API int xuiWidgetSetHitTestVisible(xui_widget pWidget, int bVisible);
XUI_API int xuiWidgetGetHitTestVisible(xui_widget pWidget);
XUI_API int xuiWidgetSetFocusable(xui_widget pWidget, int bFocusable);
XUI_API int xuiWidgetGetFocusable(xui_widget pWidget);
XUI_API int xuiWidgetSetTabStop(xui_widget pWidget, int bTabStop);
XUI_API int xuiWidgetGetTabStop(xui_widget pWidget);
XUI_API int xuiWidgetSetTabIndex(xui_widget pWidget, int iTabIndex);
XUI_API int xuiWidgetGetTabIndex(xui_widget pWidget);
XUI_API int xuiWidgetSetFocusScope(xui_widget pWidget, int bFocusScope);
XUI_API int xuiWidgetGetFocusScope(xui_widget pWidget);
XUI_API int xuiWidgetSetDefaultAction(xui_widget pWidget, xui_widget_action_proc onAction, void* pUser);
XUI_API int xuiWidgetSetCancelAction(xui_widget pWidget, xui_widget_action_proc onAction, void* pUser);
XUI_API int xuiWidgetSetDragEnabled(xui_widget pWidget, int bEnabled);
XUI_API int xuiWidgetGetDragEnabled(xui_widget pWidget);
XUI_API int xuiWidgetSetImeMode(xui_widget pWidget, int iImeMode);
XUI_API int xuiWidgetGetImeMode(xui_widget pWidget);
XUI_API int xuiWidgetSetImeCandidateRect(xui_widget pWidget, xui_widget_ime_rect_proc onRect, void* pUser);
XUI_API int xuiWidgetGetImeCandidateRect(xui_widget pWidget, xui_widget_ime_rect_proc* pRect, void** ppUser);
XUI_API int xuiHasImeCandidateRect(xui_context pContext);
XUI_API xui_rect_t xuiGetImeCandidateRect(xui_context pContext);
XUI_API int xuiWidgetSetEventCallback(xui_widget pWidget, xui_widget_event_proc onEvent, void* pUser);
XUI_API int xuiWidgetGetEventCallback(xui_widget pWidget, xui_widget_event_proc* pEvent, void** ppUser);
XUI_API int xuiWidgetSetEventHandler(xui_widget pWidget, int iEventType, xui_widget_event_proc onEvent, void* pUser);
XUI_API int xuiWidgetGetEventHandler(xui_widget pWidget, int iEventType, xui_widget_event_proc* pEvent, void** ppUser);
XUI_API int xuiWidgetSetEventInterest(xui_widget pWidget, uint64_t iEventMask, int bEnabled);
XUI_API uint64_t xuiWidgetGetEventMask(xui_widget pWidget);
XUI_API uint64_t xuiWidgetGetSubtreeEventMask(xui_widget pWidget);
XUI_API int xuiWidgetSetTooltipText(xui_widget pWidget, const char* sText);
XUI_API int xuiWidgetSetTooltip(xui_widget pWidget, const xui_tooltip_desc_t* pDesc);
XUI_API int xuiWidgetSetTooltipResolver(xui_widget pWidget, xui_tooltip_resolve_proc onResolve, void* pUser);
XUI_API int xuiWidgetClearTooltip(xui_widget pWidget);
XUI_API const xui_tooltip_desc_t* xuiWidgetGetTooltip(xui_widget pWidget);
XUI_API int xuiWidgetTooltipIsOpen(xui_context pContext);
XUI_API xui_widget xuiWidgetTooltipGetOwner(xui_context pContext);
XUI_API xui_rect_t xuiWidgetTooltipGetRect(xui_context pContext);

XUI_API xui_widget xuiOverlayRoot(xui_context pContext);
XUI_API int xuiOverlayAttach(xui_context pContext, xui_widget pOwner, xui_widget pOverlay, int iLayer, int iZIndex);
XUI_API int xuiOverlayDetach(xui_widget pOverlay);
XUI_API int xuiOverlayBringToFront(xui_widget pOverlay);
XUI_API xui_widget xuiOverlayGetOwner(xui_widget pOverlay);
XUI_API xui_widget xuiOverlayTop(xui_context pContext);

XUI_API int xuiWidgetInvalidate(xui_widget pWidget, uint32_t iFlags);
XUI_API int xuiWidgetInvalidateRect(xui_widget pWidget, xui_rect_t tRect, uint32_t iFlags);
XUI_API uint32_t xuiWidgetGetDirtyFlags(xui_widget pWidget);
XUI_API void xuiWidgetClearDirty(xui_widget pWidget, uint32_t iFlags);

XUI_API int xuiWidgetSetStyleName(xui_widget pWidget, const char* sName);
XUI_API const char* xuiWidgetGetStyleName(xui_widget pWidget);
XUI_API int xuiWidgetAddStyleClass(xui_widget pWidget, const char* sClass);
XUI_API int xuiWidgetRemoveStyleClass(xui_widget pWidget, const char* sClass);
XUI_API void xuiWidgetClearStyleClasses(xui_widget pWidget);
XUI_API int xuiWidgetHasStyleClass(xui_widget pWidget, const char* sClass);
XUI_API int xuiWidgetGetStyleClassCount(xui_widget pWidget);
XUI_API const char* xuiWidgetGetStyleClass(xui_widget pWidget, int iIndex);
XUI_API int xuiWidgetSetInlineStyle(xui_widget pWidget, const xui_style_property_t* pProperties, int iPropertyCount);
XUI_API int xuiWidgetGetInlineStyleProperty(xui_widget pWidget, const char* sName, xui_style_property_t* pProperty);
XUI_API int xuiWidgetResolveStyle(xui_widget pWidget);
XUI_API uint32_t xuiWidgetGetStyleGeneration(xui_widget pWidget);
XUI_API uint32_t xuiWidgetGetStyleHash(xui_widget pWidget);
XUI_API int xuiWidgetGetResolvedStyleProperty(xui_widget pWidget, const char* sName, xui_style_property_t* pProperty);
XUI_API int xuiWidgetGetResolvedStylePropertyCount(xui_widget pWidget);
XUI_API int xuiWidgetGetResolvedStylePropertyAt(xui_widget pWidget, int iIndex, xui_style_property_t* pProperty);

XUI_API int xuiWidgetSetStateId(xui_widget pWidget, uint32_t iStateId);
XUI_API uint32_t xuiWidgetGetStateId(xui_widget pWidget);
XUI_API int xuiWidgetSetCachePolicy(xui_widget pWidget, const xui_cache_policy_t* pPolicy);
XUI_API xui_cache_policy_t xuiWidgetGetCachePolicy(xui_widget pWidget);
XUI_API int xuiWidgetSetCacheRenderCallback(xui_widget pWidget, xui_widget_cache_render_proc onRender, void* pUser);
XUI_API int xuiWidgetGetCacheRenderCallback(xui_widget pWidget, xui_widget_cache_render_proc* pRender, void** ppUser);
XUI_API int xuiWidgetSetCacheStateCount(xui_widget pWidget, int iCount);
XUI_API int xuiWidgetGetCacheStateCount(xui_widget pWidget);
XUI_API int xuiWidgetSetCacheStateId(xui_widget pWidget, int iIndex, uint32_t iStateId);
XUI_API uint32_t xuiWidgetGetCacheStateId(xui_widget pWidget, int iIndex);
XUI_API xui_surface xuiWidgetGetCacheSurface(xui_widget pWidget, uint32_t iStateId);
XUI_API int xuiWidgetUpdateBegin(xui_widget pWidget, uint32_t iStateId, uint32_t iFlags, uint32_t iClearColor, xui_draw_context* ppDraw);
XUI_API int xuiWidgetUpdateEnd(xui_widget pWidget, uint32_t iStateId, xui_draw_context pDraw);
XUI_API int xuiWidgetRenderCache(xui_widget pWidget, xui_surface pTarget);
XUI_API int xuiWidgetRenderTree(xui_widget pWidget, xui_surface pTarget);

XUI_API int xuiDebugWidgetInspect(xui_widget pWidget, xui_debug_widget_info_t* pInfo);
XUI_API xui_widget xuiDebugWidgetInspectAt(xui_context pContext, float fX, float fY, xui_debug_widget_info_t* pInfo);
XUI_API int xuiDebugWidgetTreeDump(xui_context pContext, xui_widget pRoot, char* sBuffer, int iCapacity);
XUI_API int xuiDebugLayoutSnapshot(xui_context pContext, char* sBuffer, int iCapacity);
XUI_API int xuiDebugEventTrace(xui_context pContext, const xui_event_t* pEvent, char* sBuffer, int iCapacity);

XUI_API xui_proxy_t xuiProxyXge(void);

#ifdef __cplusplus
}
#endif

#endif /* XUI_H */
