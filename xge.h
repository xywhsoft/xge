#ifndef XGE_H
#define XGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define XGE_VERSION_MAJOR	2
#define XGE_VERSION_MINOR	0
#define XGE_VERSION_PATCH	0

#if !defined(XGE_DEBUG) && !defined(XGE_RELEASE)
	#if defined(NDEBUG)
		#define XGE_RELEASE	1
	#else
		#define XGE_DEBUG	1
	#endif
#endif

#if defined(_WIN32) || defined(_WIN64)
	#if defined(XGE_DLL)
		#if defined(XGE_BUILD_DLL)
			#define XGE_API __declspec(dllexport)
		#else
			#define XGE_API __declspec(dllimport)
		#endif
	#else
		#define XGE_API
	#endif
#else
	#define XGE_API
#endif

#define XGE_INIT_WINDOW		0x0000
#define XGE_INIT_FULLSCREEN	0x0001
#define XGE_INIT_NOFRAME	0x0002
#define XGE_INIT_RESIZABLE	0x0004
#define XGE_INIT_HIGHDPI	0x0008
#define XGE_INIT_VSYNC		0x0010
#define XGE_INIT_OFFSCREEN	0x0020

#define XGE_RUN_GAME_LOOP	0
#define XGE_RUN_MANUAL		1

#define XGE_KEY_COUNT		512
#define XGE_KEY_ENTER		257
#define XGE_KEY_TAB			258
#define XGE_KEY_BACKSPACE	259
#define XGE_KEY_DELETE		261
#define XGE_KEY_RIGHT		262
#define XGE_KEY_LEFT		263
#define XGE_TEXT_MAX		32
#define XGE_TOUCH_MAX		8
#define XGE_MOUSE_LEFT		0x01
#define XGE_MOUSE_RIGHT		0x02
#define XGE_MOUSE_MIDDLE	0x04

#define XGE_PIXEL_RGBA8		1
#define XGE_PIXEL_A8		2

#define XGE_IMAGE_PREMULTIPLIED		0x0001
#define XGE_IMAGE_STRAIGHT_ALPHA	0x0002

#define XGE_BLEND_NONE			0
#define XGE_BLEND_ALPHA			1
#define XGE_BLEND_ADD			2
#define XGE_BLEND_MULTIPLY		3
#define XGE_BLEND_SCREEN		4
#define XGE_BLEND_CUSTOM		5

#define XGE_COORD_Y_DOWN	0
#define XGE_COORD_CENTER	1

#define XGE_DRAW_FLIP_X		0x0001
#define XGE_DRAW_FLIP_Y		0x0002
#define XGE_DRAW_SCREEN_SPACE	0x0004

#define XGE_TEXT_ALIGN_LEFT		0x0000
#define XGE_TEXT_ALIGN_CENTER	0x0001
#define XGE_TEXT_ALIGN_RIGHT	0x0002
#define XGE_TEXT_ALIGN_TOP		0x0000
#define XGE_TEXT_ALIGN_MIDDLE	0x0010
#define XGE_TEXT_ALIGN_BOTTOM	0x0020
#define XGE_TEXT_CLIP			0x0100

#define XGE_XRF_MAGIC			0x32465258u
#define XGE_XRF_VERSION			1
#define XGE_XRF_PAGE_A8			1
#define XGE_XRF_PAGE_RGBA8		2
#define XGE_XRF_FLAG_KERNING	0x0001

#define XGE_AUDIO_SOUND		1
#define XGE_AUDIO_MUSIC		2
#define XGE_AUDIO_STREAM	3

#define XGE_EVENT_NONE			0
#define XGE_EVENT_KEY_DOWN		1
#define XGE_EVENT_KEY_UP		2
#define XGE_EVENT_TEXT			3
#define XGE_EVENT_MOUSE_DOWN	4
#define XGE_EVENT_MOUSE_UP		5
#define XGE_EVENT_MOUSE_MOVE	6
#define XGE_EVENT_MOUSE_WHEEL	7
#define XGE_EVENT_TOUCH_BEGIN	8
#define XGE_EVENT_TOUCH_MOVE	9
#define XGE_EVENT_TOUCH_END		10
#define XGE_EVENT_TOUCH_CANCEL	11
#define XGE_EVENT_RESIZE		12
#define XGE_EVENT_QUIT			13

#define XGE_XUI_LAYOUT_ABSOLUTE	0
#define XGE_XUI_LAYOUT_ROW		1
#define XGE_XUI_LAYOUT_COLUMN	2
#define XGE_XUI_LAYOUT_STACK	3
#define XGE_XUI_LAYOUT_GRID		4

#define XGE_XUI_SIZE_PX		0
#define XGE_XUI_SIZE_DIP	1
#define XGE_XUI_SIZE_PERCENT	2
#define XGE_XUI_SIZE_CONTENT	3
#define XGE_XUI_SIZE_GROW	4

#define XGE_XUI_WIDGET_VISIBLE		0x0001
#define XGE_XUI_WIDGET_ENABLED		0x0002
#define XGE_XUI_WIDGET_FOCUSABLE	0x0004
#define XGE_XUI_WIDGET_CLIP			0x0008
#define XGE_XUI_WIDGET_DIRTY_LAYOUT	0x0010
#define XGE_XUI_WIDGET_DIRTY_PAINT	0x0020

#define XGE_XUI_STATE_NORMAL	0x0000
#define XGE_XUI_STATE_HOVER		0x0001
#define XGE_XUI_STATE_ACTIVE	0x0002
#define XGE_XUI_STATE_FOCUS		0x0004
#define XGE_XUI_STATE_DISABLED	0x0008

#define XGE_XUI_IMAGE_STRETCH	0
#define XGE_XUI_IMAGE_FIT		1
#define XGE_XUI_IMAGE_CENTER	2

#define XGE_XUI_EVENT_CONTINUE	0
#define XGE_XUI_EVENT_CONSUMED	1

#define XGE_TOUCH_NONE			0
#define XGE_TOUCH_BEGIN			1
#define XGE_TOUCH_MOVE			2
#define XGE_TOUCH_STATIONARY	3
#define XGE_TOUCH_END			4
#define XGE_TOUCH_CANCEL		5

#define XGE_COLOR_RGBA(r, g, b, a)	((((uint32_t)(r) & 0xFFu) << 24) | (((uint32_t)(g) & 0xFFu) << 16) | (((uint32_t)(b) & 0xFFu) << 8) | ((uint32_t)(a) & 0xFFu))
#define XGE_COLOR_GET_R(c)			(((uint32_t)(c) >> 24) & 0xFFu)
#define XGE_COLOR_GET_G(c)			(((uint32_t)(c) >> 16) & 0xFFu)
#define XGE_COLOR_GET_B(c)			(((uint32_t)(c) >> 8) & 0xFFu)
#define XGE_COLOR_GET_A(c)			((uint32_t)(c) & 0xFFu)

typedef enum xge_result_t {
	XGE_OK = 0,
	XGE_ERROR = -1,
	XGE_ERROR_INVALID_ARGUMENT = -2,
	XGE_ERROR_NOT_INITIALIZED = -3,
	XGE_ERROR_ALREADY_INITIALIZED = -4,
	XGE_ERROR_OUT_OF_MEMORY = -5,
	XGE_ERROR_FILE_NOT_FOUND = -6,
	XGE_ERROR_UNSUPPORTED = -7,
	XGE_ERROR_BACKEND_FAILED = -8,
	XGE_ERROR_GPU_FAILED = -9,
	XGE_ERROR_RESOURCE_FAILED = -10,
	XGE_ERROR_AUDIO_FAILED = -11,
	XGE_ERROR_THREAD_FAILED = -12
} xge_result_t;

typedef struct xge_desc_t {
	int iWidth;
	int iHeight;
	const char* sTitle;
	int iFlags;
	int iRunMode;
	int iTargetFPS;
	void* pNativeWindow;
	void* pUser;
} xge_desc_t;

typedef struct xge_rect_t {
	float fX;
	float fY;
	float fW;
	float fH;
} xge_rect_t;

typedef struct xge_vec2_t {
	float fX;
	float fY;
} xge_vec2_t;

typedef struct xge_vec3_t {
	float fX;
	float fY;
	float fZ;
} xge_vec3_t;

typedef struct xge_mat3_t {
	float m[9];
} xge_mat3_t;

typedef struct xge_mat4_t {
	float m[16];
} xge_mat4_t;

typedef struct xge_color_t {
	float fR;
	float fG;
	float fB;
	float fA;
} xge_color_t;

typedef struct xge_camera_t {
	xge_vec2_t tPosition;
	xge_vec2_t tScale;
	float fRotation;
	xge_rect_t tViewport;
	int iCoordinateMode;
} xge_camera_t;

typedef struct xge_texture_t xge_texture_t;
typedef xge_texture_t* xge_texture;

typedef struct xge_draw_t {
	xge_texture pTexture;
	xge_rect_t tSrc;
	xge_rect_t tDst;
	xge_vec2_t tOrigin;
	float fRotation;
	uint32_t iColor;
	uint32_t iFlags;
} xge_draw_t;

struct xge_texture_t {
	int iWidth;
	int iHeight;
	int iFormat;
	int iRefCount;
	uint32_t iFlags;
	uint32_t iBackendId;
	void* pBackend;
};

typedef struct xge_image_t {
	int iWidth;
	int iHeight;
	int iFormat;
	int iStride;
	void* pPixels;
	uint32_t iFlags;
} xge_image_t, *xge_image;

typedef struct xge_resource_t {
	void* pData;
	int iSize;
	int iProvider;
	void (*free)(void* pData, void* pUser);
	void* pUser;
} xge_resource_t;

typedef struct xge_resource_provider_t {
	const char* sScheme;
	int (*load)(const char* sURI, void** ppData, int* pSize, void* pUser);
	void (*free)(void* pData, void* pUser);
	void* pUser;
} xge_resource_provider_t;

typedef struct xge_sound_t {
	int iRefCount;
	int iType;
	uint32_t iFlags;
	void* pBackend;
} xge_sound_t, *xge_sound;

typedef struct xge_music_t {
	int iRefCount;
	int iType;
	uint32_t iFlags;
	void* pBackend;
} xge_music_t, *xge_music;

typedef struct xge_stream_t {
	int iRefCount;
	int iType;
	uint32_t iFlags;
	void* pBackend;
} xge_stream_t, *xge_stream;

typedef struct xge_audio_group_t {
	float fVolume;
	void* pBackend;
} xge_audio_group_t, *xge_audio_group;

typedef struct xge_audio_listener_t {
	xge_vec3_t tPosition;
	xge_vec3_t tForward;
	xge_vec3_t tUp;
} xge_audio_listener_t;

typedef struct xge_font_t xge_font_t;
typedef xge_font_t* xge_font;

typedef struct xge_glyph_metrics_t {
	uint32_t iCodepoint;
	int iGlyph;
	float fAdvanceX;
	float fLeftSideBearing;
	float fX0;
	float fY0;
	float fX1;
	float fY1;
} xge_glyph_metrics_t;

typedef struct xge_glyph_bitmap_t {
	uint32_t iCodepoint;
	int iWidth;
	int iHeight;
	int iStride;
	int iOffsetX;
	int iOffsetY;
	int iFormat;
	void* pPixels;
} xge_glyph_bitmap_t;

typedef struct xge_glyph_t {
	uint32_t iCodepoint;
	int iGlyph;
	int iPage;
	int iX;
	int iY;
	int iWidth;
	int iHeight;
	float fOffsetX;
	float fOffsetY;
	float fAdvanceX;
} xge_glyph_t;

typedef struct xge_glyph_atlas_t {
	int iPageWidth;
	int iPageHeight;
	int iPageCount;
	int iFormat;
	void* pPages;
} xge_glyph_atlas_t;

struct xge_font_t {
	int iRefCount;
	float fSize;
	float fScale;
	float fAscent;
	float fDescent;
	float fLineGap;
	float fLineHeight;
	void* pData;
	int iDataSize;
	void* pBackend;
	xge_font pFallback;
	xge_glyph_atlas_t tAtlas;
	void* pGlyphs;
};

typedef struct xge_xrf_header_t {
	uint32_t iMagic;
	uint16_t iVersion;
	uint16_t iHeaderSize;
	uint32_t iFlags;
	uint32_t iGlyphCount;
	uint32_t iPageCount;
	uint32_t iRangeCount;
	uint32_t iKerningCount;
	float fAscent;
	float fDescent;
	float fLineGap;
	float fLineHeight;
	uint32_t iGlyphOffset;
	uint32_t iPageOffset;
	uint32_t iRangeOffset;
	uint32_t iKerningOffset;
	uint32_t iPixelOffset;
} xge_xrf_header_t;

typedef struct xge_xrf_range_t {
	uint32_t iFirstCodepoint;
	uint32_t iCount;
} xge_xrf_range_t;

typedef struct xge_xrf_glyph_t {
	uint32_t iCodepoint;
	uint16_t iPage;
	uint16_t iGlyph;
	uint16_t iX;
	uint16_t iY;
	uint16_t iWidth;
	uint16_t iHeight;
	float fOffsetX;
	float fOffsetY;
	float fAdvanceX;
} xge_xrf_glyph_t;

typedef struct xge_xrf_page_t {
	uint16_t iWidth;
	uint16_t iHeight;
	uint16_t iFormat;
	uint16_t iReserved;
	uint32_t iPixelOffset;
	uint32_t iPixelSize;
} xge_xrf_page_t;

typedef struct xge_xrf_kerning_t {
	uint32_t iLeftCodepoint;
	uint32_t iRightCodepoint;
	float fAdvanceX;
} xge_xrf_kerning_t;

typedef struct xge_event_t {
	int iType;
	int iParam1;
	int iParam2;
	float fX;
	float fY;
	float fDX;
	float fDY;
	uint32_t iCodepoint;
	void* pData;
} xge_event_t;

typedef struct xge_touch_point_t {
	uint64_t iId;
	float fX;
	float fY;
	float fDX;
	float fDY;
	int iPhase;
	int bChanged;
	int bDown;
} xge_touch_point_t;

typedef struct xge_touch_event_t {
	int iPhase;
	int iCount;
	xge_touch_point_t arrPoints[XGE_TOUCH_MAX];
} xge_touch_event_t;

typedef struct xge_xui_size_t {
	float fValue;
	int iUnit;
} xge_xui_size_t;

typedef struct xge_xui_edges_t {
	xge_xui_size_t tLeft;
	xge_xui_size_t tTop;
	xge_xui_size_t tRight;
	xge_xui_size_t tBottom;
} xge_xui_edges_t;

typedef struct xge_xui_style_t {
	int iLayout;
	xge_xui_size_t tWidth;
	xge_xui_size_t tHeight;
	xge_xui_size_t tMinWidth;
	xge_xui_size_t tMinHeight;
	xge_xui_size_t tMaxWidth;
	xge_xui_size_t tMaxHeight;
	xge_xui_edges_t tMargin;
	xge_xui_edges_t tPadding;
	int iAlignX;
	int iAlignY;
	int iZ;
	int iClip;
	uint32_t iBackgroundColor;
	float fRadius;
} xge_xui_style_t;

typedef struct xge_xui_widget_t xge_xui_widget_t;
typedef xge_xui_widget_t* xge_xui_widget;
typedef struct xge_xui_context_t xge_xui_context_t;
typedef xge_xui_context_t* xge_xui_context;
typedef struct xge_xui_button_t xge_xui_button_t;
typedef xge_xui_button_t* xge_xui_button;
typedef struct xge_xui_label_t xge_xui_label_t;
typedef xge_xui_label_t* xge_xui_label;
typedef struct xge_xui_image_t xge_xui_image_t;
typedef xge_xui_image_t* xge_xui_image;
typedef struct xge_xui_input_t xge_xui_input_t;
typedef xge_xui_input_t* xge_xui_input;
typedef struct xge_xui_toggle_t xge_xui_toggle_t;
typedef xge_xui_toggle_t* xge_xui_toggle;
typedef struct xge_xui_slider_t xge_xui_slider_t;
typedef xge_xui_slider_t* xge_xui_slider;
typedef struct xge_xui_progress_t xge_xui_progress_t;
typedef xge_xui_progress_t* xge_xui_progress;
typedef struct xge_xui_panel_t xge_xui_panel_t;
typedef xge_xui_panel_t* xge_xui_panel;
typedef struct xge_xui_scroll_view_t xge_xui_scroll_view_t;
typedef xge_xui_scroll_view_t* xge_xui_scroll_view;
typedef struct xge_xui_list_view_t xge_xui_list_view_t;
typedef xge_xui_list_view_t* xge_xui_list_view;

typedef struct xge_xui_text_t {
	char* sText;
	int iSize;
	int iCapacity;
	int iCursor;
	int iSelectStart;
	int iSelectEnd;
} xge_xui_text_t, *xge_xui_text;

typedef int (*xge_xui_event_proc)(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
typedef void (*xge_xui_paint_proc)(xge_xui_widget pWidget, void* pUser);
typedef void (*xge_xui_click_proc)(xge_xui_widget pWidget, void* pUser);
typedef void (*xge_xui_toggle_proc)(xge_xui_widget pWidget, int bChecked, void* pUser);
typedef void (*xge_xui_slider_proc)(xge_xui_widget pWidget, float fValue, void* pUser);
typedef void (*xge_xui_select_proc)(xge_xui_widget pWidget, int iIndex, void* pUser);

struct xge_xui_widget_t {
	xge_xui_widget pParent;
	xge_xui_widget pFirstChild;
	xge_xui_widget pLastChild;
	xge_xui_widget pPrevSibling;
	xge_xui_widget pNextSibling;
	xge_xui_style_t tStyle;
	xge_rect_t tRect;
	xge_rect_t tContentRect;
	uint32_t iFlags;
	void* pUser;
	xge_xui_event_proc procEvent;
	xge_xui_paint_proc procPaint;
	void* pInternal;
};

struct xge_xui_context_t {
	int bInitialized;
	xge_xui_widget pRoot;
	xge_xui_widget pFocus;
	xge_xui_widget pCapture;
	int iDirtyLayoutCount;
	int iDirtyPaintCount;
	int iPaintCommandCount;
	void* pUser;
	void* pInternal;
};

struct xge_xui_button_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_font pFont;
	const char* sText;
	xge_xui_click_proc procClick;
	void* pUser;
	uint32_t iTextColor;
	uint32_t iTextFlags;
	uint32_t iColorNormal;
	uint32_t iColorHover;
	uint32_t iColorActive;
	uint32_t iColorFocus;
	uint32_t iColorDisabled;
	int iState;
	int iClickCount;
};

struct xge_xui_label_t {
	xge_xui_widget pWidget;
	xge_font pFont;
	const char* sText;
	uint32_t iColor;
	uint32_t iTextFlags;
	xge_vec2_t tMeasuredSize;
};

struct xge_xui_image_t {
	xge_xui_widget pWidget;
	xge_texture pTexture;
	xge_rect_t tSrc;
	uint32_t iColor;
	int iMode;
};

struct xge_xui_input_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_text_t tText;
	xge_font pFont;
	uint32_t iTextColor;
	uint32_t iBackgroundColor;
	uint32_t iFocusColor;
	uint32_t iCursorColor;
	int bInitialized;
};

struct xge_xui_toggle_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_font pFont;
	const char* sText;
	xge_xui_toggle_proc procChange;
	void* pUser;
	uint32_t iTextColor;
	uint32_t iTextFlags;
	uint32_t iColorNormal;
	uint32_t iColorHover;
	uint32_t iColorActive;
	uint32_t iColorFocus;
	uint32_t iColorDisabled;
	uint32_t iColorChecked;
	int iState;
	int bChecked;
	int iChangeCount;
};

struct xge_xui_slider_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_slider_proc procChange;
	void* pUser;
	float fMin;
	float fMax;
	float fValue;
	uint32_t iColorTrack;
	uint32_t iColorFill;
	uint32_t iColorKnob;
	uint32_t iColorFocus;
	uint32_t iColorDisabled;
	int iState;
	int iChangeCount;
};

struct xge_xui_progress_t {
	xge_xui_widget pWidget;
	xge_font pFont;
	const char* sText;
	float fMin;
	float fMax;
	float fValue;
	uint32_t iColorTrack;
	uint32_t iColorFill;
	uint32_t iTextColor;
	uint32_t iTextFlags;
};

struct xge_xui_panel_t {
	xge_xui_widget pWidget;
	xge_font pFont;
	const char* sTitle;
	uint32_t iBackgroundColor;
	uint32_t iTitleColor;
	uint32_t iTitleFlags;
	int bClip;
};

struct xge_xui_scroll_view_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	float fContentW;
	float fContentH;
	float fScrollX;
	float fScrollY;
	float fDragX;
	float fDragY;
	uint32_t iBackgroundColor;
	uint32_t iBarColor;
	uint32_t iThumbColor;
	int bDragging;
};

struct xge_xui_list_view_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_font pFont;
	const char** arrItems;
	int iItemCount;
	int iSelected;
	float fItemHeight;
	float fScrollY;
	xge_xui_select_proc procSelect;
	void* pUser;
	uint32_t iBackgroundColor;
	uint32_t iRowColor;
	uint32_t iSelectedColor;
	uint32_t iTextColor;
	uint32_t iBarColor;
	uint32_t iThumbColor;
};

typedef int (*xge_scene_proc)(void* pUser);

XGE_API int xgeInit(const xge_desc_t* pDesc);
XGE_API void xgeUnit(void);
XGE_API int xgeRun(xge_scene_proc procFrame, void* pUser);
XGE_API void xgeQuit(void);
XGE_API int xgeFrame(void);

XGE_API int xgeGetWidth(void);
XGE_API int xgeGetHeight(void);
XGE_API float xgeGetDelta(void);
XGE_API int xgeGetFPS(void);
XGE_API void xgeSetTitle(const char* sTitle);

XGE_API void xgeClear(uint32_t iColor);
XGE_API void xgePresent(void);
XGE_API uint32_t xgeColorRGBA(int iR, int iG, int iB, int iA);
XGE_API xge_color_t xgeColorUnpack(uint32_t iColor);
XGE_API void xgeBlendSet(int iBlend);
XGE_API int xgeBlendGet(void);
XGE_API xge_camera_t xgeCameraDefault(float fWidth, float fHeight);
XGE_API void xgeCameraSet(const xge_camera_t* pCamera);
XGE_API xge_camera_t xgeCameraGet(void);
XGE_API xge_vec2_t xgeWorldToScreen(xge_vec2_t tPoint);
XGE_API xge_vec2_t xgeScreenToWorld(xge_vec2_t tPoint);
XGE_API int xgeResourceProviderAdd(const xge_resource_provider_t* pProvider);
XGE_API void xgeResourceProviderClear(void);
XGE_API int xgeResourceLoad(const char* sURI, xge_resource_t* pResource);
XGE_API int xgeResourceLoadMemory(const void* pData, int iSize, xge_resource_t* pResource);
XGE_API void xgeResourceFree(xge_resource_t* pResource);
XGE_API int xgeAudioInit(void);
XGE_API void xgeAudioUnit(void);
XGE_API int xgeAudioIsReady(void);
XGE_API void xgeAudioSetVolume(float fVolume);
XGE_API float xgeAudioGetVolume(void);
XGE_API void xgeAudioListenerSet(const xge_audio_listener_t* pListener);
XGE_API xge_audio_listener_t xgeAudioListenerGet(void);
XGE_API int xgeAudioGroupInit(xge_audio_group pGroup);
XGE_API void xgeAudioGroupFree(xge_audio_group pGroup);
XGE_API void xgeAudioGroupSetVolume(xge_audio_group pGroup, float fVolume);
XGE_API float xgeAudioGroupGetVolume(xge_audio_group pGroup);
XGE_API void xgeAudioGroupFade(xge_audio_group pGroup, float fFrom, float fTo, int iMilliseconds);
XGE_API int xgeSoundLoad(xge_sound pSound, const char* sPath);
XGE_API int xgeSoundLoadGroup(xge_sound pSound, const char* sPath, xge_audio_group pGroup);
XGE_API int xgeSoundPlay(xge_sound pSound);
XGE_API int xgeSoundStop(xge_sound pSound);
XGE_API int xgeSoundPause(xge_sound pSound);
XGE_API int xgeSoundResume(xge_sound pSound);
XGE_API int xgeSoundAddRef(xge_sound pSound);
XGE_API void xgeSoundFree(xge_sound pSound);
XGE_API void xgeSoundSetLoop(xge_sound pSound, int bLoop);
XGE_API void xgeSoundSetVolume(xge_sound pSound, float fVolume);
XGE_API void xgeSoundSetPosition(xge_sound pSound, float fX, float fY, float fZ);
XGE_API void xgeSoundFade(xge_sound pSound, float fFrom, float fTo, int iMilliseconds);
XGE_API int xgeSoundIsPlaying(xge_sound pSound);
XGE_API int xgeMusicLoad(xge_music pMusic, const char* sPath);
XGE_API int xgeMusicLoadGroup(xge_music pMusic, const char* sPath, xge_audio_group pGroup);
XGE_API int xgeMusicPlay(xge_music pMusic);
XGE_API int xgeMusicStop(xge_music pMusic);
XGE_API int xgeMusicPause(xge_music pMusic);
XGE_API int xgeMusicResume(xge_music pMusic);
XGE_API void xgeMusicFree(xge_music pMusic);
XGE_API void xgeMusicSetLoop(xge_music pMusic, int bLoop);
XGE_API void xgeMusicSetVolume(xge_music pMusic, float fVolume);
XGE_API void xgeMusicFade(xge_music pMusic, float fFrom, float fTo, int iMilliseconds);
XGE_API int xgeMusicIsPlaying(xge_music pMusic);
XGE_API int xgeStreamOpen(xge_stream pStream, const char* sPath);
XGE_API int xgeStreamOpenGroup(xge_stream pStream, const char* sPath, xge_audio_group pGroup);
XGE_API int xgeStreamPlay(xge_stream pStream);
XGE_API int xgeStreamStop(xge_stream pStream);
XGE_API int xgeStreamPause(xge_stream pStream);
XGE_API int xgeStreamResume(xge_stream pStream);
XGE_API void xgeStreamClose(xge_stream pStream);
XGE_API void xgeStreamSetLoop(xge_stream pStream, int bLoop);
XGE_API void xgeStreamSetVolume(xge_stream pStream, float fVolume);
XGE_API void xgeStreamSetPosition(xge_stream pStream, float fX, float fY, float fZ);
XGE_API void xgeStreamFade(xge_stream pStream, float fFrom, float fTo, int iMilliseconds);
XGE_API int xgeStreamIsPlaying(xge_stream pStream);
XGE_API int xgeTextUTF8Next(const char** psText, uint32_t* pCodepoint);
XGE_API int xgeFontLoad(xge_font pFont, const char* sPath, float fSize);
XGE_API int xgeFontLoadMemory(xge_font pFont, const void* pData, int iSize, float fSize);
XGE_API int xgeFontLoadXRF(xge_font pFont, const char* sPath);
XGE_API int xgeFontLoadXRFMemory(xge_font pFont, const void* pData, int iSize);
XGE_API int xgeFontAddRef(xge_font pFont);
XGE_API void xgeFontFree(xge_font pFont);
XGE_API void xgeFontSetFallback(xge_font pFont, xge_font pFallback);
XGE_API int xgeFontGlyphGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_metrics_t* pMetrics);
XGE_API int xgeFontGlyphRasterize(xge_font pFont, uint32_t iCodepoint, xge_glyph_bitmap_t* pBitmap);
XGE_API int xgeFontGlyphAtlasGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_t* pGlyph);
XGE_API void xgeGlyphBitmapFree(xge_glyph_bitmap_t* pBitmap);
XGE_API xge_vec2_t xgeTextMeasure(xge_font pFont, const char* sText);
XGE_API void xgeTextDraw(xge_font pFont, const char* sText, float fX, float fY, uint32_t iColor);
XGE_API void xgeTextDrawRect(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags);
XGE_API int xgeImageLoad(xge_image pImage, const char* sPath);
XGE_API int xgeImageLoadEx(xge_image pImage, const char* sPath, uint32_t iFlags);
XGE_API int xgeImageLoadMemory(xge_image pImage, const void* pData, int iSize);
XGE_API int xgeImageLoadMemoryEx(xge_image pImage, const void* pData, int iSize, uint32_t iFlags);
XGE_API void* xgeImageGetPixels(xge_image pImage);
XGE_API void xgeImagePremultiply(xge_image pImage);
XGE_API void xgeImageFree(xge_image pImage);
XGE_API int xgeTextureCreateRGBA(xge_texture pTexture, int iWidth, int iHeight, const void* pPixels);
XGE_API int xgeTextureCreateFromImage(xge_texture pTexture, const xge_image_t* pImage);
XGE_API int xgeTextureLoad(xge_texture pTexture, const char* sPath);
XGE_API int xgeTextureLoadEx(xge_texture pTexture, const char* sPath, uint32_t iFlags);
XGE_API int xgeTextureLoadMemory(xge_texture pTexture, const void* pData, int iSize);
XGE_API int xgeTextureLoadMemoryEx(xge_texture pTexture, const void* pData, int iSize, uint32_t iFlags);
XGE_API int xgeTextureAddRef(xge_texture pTexture);
XGE_API void xgeTextureFree(xge_texture pTexture);
XGE_API void xgeDraw(xge_texture pTexture, float fX, float fY);
XGE_API void xgeDrawEx(const xge_draw_t* pDraw);
XGE_API void xgeDrawPx(xge_texture pTexture, int iX, int iY);
XGE_API void xgeShapeRectFill(xge_rect_t tRect, uint32_t iColor);
XGE_API void xgeShapeRectFillPx(xge_rect_t tRect, uint32_t iColor);
XGE_API void xgeClipSet(xge_rect_t tRect);
XGE_API void xgeClipClear(void);

XGE_API int xgeKeyDown(int iKey);
XGE_API int xgeKeyPressed(int iKey);
XGE_API int xgeKeyReleased(int iKey);
XGE_API void xgeMouseGet(float* pX, float* pY);
XGE_API void xgeMouseGetDelta(float* pDX, float* pDY);
XGE_API void xgeMouseGetWheel(float* pX, float* pY);
XGE_API int xgeMouseDown(int iButton);
XGE_API uint32_t xgeTextGet(void);
XGE_API int xgeTouchGetCount(void);
XGE_API int xgeTouchGet(int iIndex, xge_touch_point_t* pPoint);
XGE_API int xgeTouchFind(uint64_t iId, xge_touch_point_t* pPoint);

XGE_API xge_xui_size_t xgeXuiSizePx(float fValue);
XGE_API xge_xui_size_t xgeXuiSizePercent(float fValue);
XGE_API xge_xui_size_t xgeXuiSizeGrow(float fValue);
XGE_API xge_xui_size_t xgeXuiSizeContent(void);
XGE_API int xgeXuiInit(xge_xui_context pContext);
XGE_API void xgeXuiUnit(xge_xui_context pContext);
XGE_API xge_xui_widget xgeXuiRoot(xge_xui_context pContext);
XGE_API xge_xui_widget xgeXuiWidgetCreate(void);
XGE_API void xgeXuiWidgetFree(xge_xui_widget pWidget);
XGE_API int xgeXuiWidgetAdd(xge_xui_widget pParent, xge_xui_widget pChild);
XGE_API void xgeXuiWidgetRemove(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetRect(xge_xui_widget pWidget, xge_rect_t tRect);
XGE_API xge_rect_t xgeXuiWidgetGetRect(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetLayout(xge_xui_widget pWidget, int iLayout);
XGE_API void xgeXuiWidgetSetSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
XGE_API void xgeXuiWidgetSetMarginPx(xge_xui_widget pWidget, float fLeft, float fTop, float fRight, float fBottom);
XGE_API void xgeXuiWidgetSetPaddingPx(xge_xui_widget pWidget, float fLeft, float fTop, float fRight, float fBottom);
XGE_API void xgeXuiWidgetSetBackground(xge_xui_widget pWidget, uint32_t iColor);
XGE_API uint32_t xgeXuiWidgetGetFlags(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetVisible(xge_xui_widget pWidget, int bVisible);
XGE_API void xgeXuiWidgetSetEnabled(xge_xui_widget pWidget, int bEnabled);
XGE_API void xgeXuiWidgetSetFocusable(xge_xui_widget pWidget, int bFocusable);
XGE_API void xgeXuiWidgetSetClip(xge_xui_widget pWidget, int bClip);
XGE_API int xgeXuiWidgetIsVisible(xge_xui_widget pWidget);
XGE_API int xgeXuiWidgetIsEnabled(xge_xui_widget pWidget);
XGE_API int xgeXuiWidgetIsFocusable(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetMarkLayout(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetMarkPaint(xge_xui_widget pWidget);
XGE_API xge_xui_widget xgeXuiHitTest(xge_xui_context pContext, float fX, float fY);
XGE_API void xgeXuiSetFocus(xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiSetCapture(xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API int xgeXuiDispatchEvent(xge_xui_context pContext, const xge_event_t* pEvent);
XGE_API int xgeXuiUpdate(xge_xui_context pContext, float fDelta);
XGE_API int xgeXuiPaint(xge_xui_context pContext);
XGE_API int xgeXuiTextInit(xge_xui_text pText);
XGE_API void xgeXuiTextUnit(xge_xui_text pText);
XGE_API int xgeXuiTextSet(xge_xui_text pText, const char* sText);
XGE_API int xgeXuiTextInsert(xge_xui_text pText, const char* sText);
XGE_API int xgeXuiTextInsertCodepoint(xge_xui_text pText, uint32_t iCodepoint);
XGE_API int xgeXuiTextDeleteBack(xge_xui_text pText);
XGE_API int xgeXuiTextInputEvent(xge_xui_text pText, const xge_event_t* pEvent);
XGE_API int xgeXuiButtonInit(xge_xui_button pButton, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiButtonUnit(xge_xui_button pButton);
XGE_API void xgeXuiButtonSetClick(xge_xui_button pButton, xge_xui_click_proc procClick, void* pUser);
XGE_API void xgeXuiButtonSetText(xge_xui_button pButton, xge_font pFont, const char* sText);
XGE_API void xgeXuiButtonSetTextColor(xge_xui_button pButton, uint32_t iColor);
XGE_API void xgeXuiButtonSetColors(xge_xui_button pButton, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);
XGE_API int xgeXuiButtonGetState(xge_xui_button pButton);
XGE_API int xgeXuiButtonEvent(xge_xui_button pButton, const xge_event_t* pEvent);
XGE_API int xgeXuiButtonEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiButtonPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiLabelInit(xge_xui_label pLabel, xge_xui_widget pWidget, xge_font pFont, const char* sText);
XGE_API void xgeXuiLabelUnit(xge_xui_label pLabel);
XGE_API void xgeXuiLabelSetText(xge_xui_label pLabel, const char* sText);
XGE_API void xgeXuiLabelSetFont(xge_xui_label pLabel, xge_font pFont);
XGE_API void xgeXuiLabelSetColor(xge_xui_label pLabel, uint32_t iColor);
XGE_API void xgeXuiLabelSetAlign(xge_xui_label pLabel, uint32_t iTextFlags);
XGE_API xge_vec2_t xgeXuiLabelMeasure(xge_xui_label pLabel);
XGE_API void xgeXuiLabelPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiImageInit(xge_xui_image pImage, xge_xui_widget pWidget, xge_texture pTexture);
XGE_API void xgeXuiImageUnit(xge_xui_image pImage);
XGE_API void xgeXuiImageSetTexture(xge_xui_image pImage, xge_texture pTexture);
XGE_API void xgeXuiImageSetSource(xge_xui_image pImage, xge_rect_t tSrc);
XGE_API void xgeXuiImageSetColor(xge_xui_image pImage, uint32_t iColor);
XGE_API void xgeXuiImageSetMode(xge_xui_image pImage, int iMode);
XGE_API void xgeXuiImagePaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiInputInit(xge_xui_input pInput, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont);
XGE_API void xgeXuiInputUnit(xge_xui_input pInput);
XGE_API void xgeXuiInputSetText(xge_xui_input pInput, const char* sText);
XGE_API const char* xgeXuiInputGetText(xge_xui_input pInput);
XGE_API void xgeXuiInputSetFont(xge_xui_input pInput, xge_font pFont);
XGE_API void xgeXuiInputSetColors(xge_xui_input pInput, uint32_t iText, uint32_t iBackground, uint32_t iFocus, uint32_t iCursor);
XGE_API int xgeXuiInputEvent(xge_xui_input pInput, const xge_event_t* pEvent);
XGE_API int xgeXuiInputEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiInputPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiToggleInit(xge_xui_toggle pToggle, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiToggleUnit(xge_xui_toggle pToggle);
XGE_API void xgeXuiToggleSetChange(xge_xui_toggle pToggle, xge_xui_toggle_proc procChange, void* pUser);
XGE_API void xgeXuiToggleSetText(xge_xui_toggle pToggle, xge_font pFont, const char* sText);
XGE_API void xgeXuiToggleSetChecked(xge_xui_toggle pToggle, int bChecked);
XGE_API int xgeXuiToggleGetChecked(xge_xui_toggle pToggle);
XGE_API void xgeXuiToggleSetTextColor(xge_xui_toggle pToggle, uint32_t iColor);
XGE_API void xgeXuiToggleSetColors(xge_xui_toggle pToggle, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iChecked);
XGE_API int xgeXuiToggleGetState(xge_xui_toggle pToggle);
XGE_API int xgeXuiToggleEvent(xge_xui_toggle pToggle, const xge_event_t* pEvent);
XGE_API int xgeXuiToggleEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiTogglePaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiSliderInit(xge_xui_slider pSlider, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiSliderUnit(xge_xui_slider pSlider);
XGE_API void xgeXuiSliderSetChange(xge_xui_slider pSlider, xge_xui_slider_proc procChange, void* pUser);
XGE_API void xgeXuiSliderSetRange(xge_xui_slider pSlider, float fMin, float fMax);
XGE_API void xgeXuiSliderSetValue(xge_xui_slider pSlider, float fValue);
XGE_API float xgeXuiSliderGetValue(xge_xui_slider pSlider);
XGE_API void xgeXuiSliderSetColors(xge_xui_slider pSlider, uint32_t iTrack, uint32_t iFill, uint32_t iKnob, uint32_t iFocus, uint32_t iDisabled);
XGE_API int xgeXuiSliderGetState(xge_xui_slider pSlider);
XGE_API int xgeXuiSliderEvent(xge_xui_slider pSlider, const xge_event_t* pEvent);
XGE_API int xgeXuiSliderEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiSliderPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiProgressInit(xge_xui_progress pProgress, xge_xui_widget pWidget);
XGE_API void xgeXuiProgressUnit(xge_xui_progress pProgress);
XGE_API void xgeXuiProgressSetRange(xge_xui_progress pProgress, float fMin, float fMax);
XGE_API void xgeXuiProgressSetValue(xge_xui_progress pProgress, float fValue);
XGE_API float xgeXuiProgressGetValue(xge_xui_progress pProgress);
XGE_API void xgeXuiProgressSetText(xge_xui_progress pProgress, xge_font pFont, const char* sText);
XGE_API void xgeXuiProgressSetTextColor(xge_xui_progress pProgress, uint32_t iColor);
XGE_API void xgeXuiProgressSetColors(xge_xui_progress pProgress, uint32_t iTrack, uint32_t iFill);
XGE_API void xgeXuiProgressPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiPanelInit(xge_xui_panel pPanel, xge_xui_widget pWidget);
XGE_API void xgeXuiPanelUnit(xge_xui_panel pPanel);
XGE_API void xgeXuiPanelSetBackground(xge_xui_panel pPanel, uint32_t iColor);
XGE_API void xgeXuiPanelSetTitle(xge_xui_panel pPanel, xge_font pFont, const char* sTitle);
XGE_API void xgeXuiPanelSetTitleColor(xge_xui_panel pPanel, uint32_t iColor);
XGE_API void xgeXuiPanelSetTitleAlign(xge_xui_panel pPanel, uint32_t iTextFlags);
XGE_API void xgeXuiPanelSetClip(xge_xui_panel pPanel, int bClip);
XGE_API void xgeXuiPanelPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiScrollViewInit(xge_xui_scroll_view pScroll, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiScrollViewUnit(xge_xui_scroll_view pScroll);
XGE_API void xgeXuiScrollViewSetContentSize(xge_xui_scroll_view pScroll, float fWidth, float fHeight);
XGE_API void xgeXuiScrollViewSetOffset(xge_xui_scroll_view pScroll, float fX, float fY);
XGE_API void xgeXuiScrollViewGetOffset(xge_xui_scroll_view pScroll, float* pX, float* pY);
XGE_API void xgeXuiScrollViewSetColors(xge_xui_scroll_view pScroll, uint32_t iBackground, uint32_t iBar, uint32_t iThumb);
XGE_API int xgeXuiScrollViewEvent(xge_xui_scroll_view pScroll, const xge_event_t* pEvent);
XGE_API int xgeXuiScrollViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiScrollViewPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiListViewInit(xge_xui_list_view pList, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiListViewUnit(xge_xui_list_view pList);
XGE_API void xgeXuiListViewSetItems(xge_xui_list_view pList, const char** arrItems, int iCount);
XGE_API void xgeXuiListViewSetFont(xge_xui_list_view pList, xge_font pFont);
XGE_API void xgeXuiListViewSetItemHeight(xge_xui_list_view pList, float fHeight);
XGE_API void xgeXuiListViewSetSelected(xge_xui_list_view pList, int iIndex);
XGE_API int xgeXuiListViewGetSelected(xge_xui_list_view pList);
XGE_API void xgeXuiListViewSetScroll(xge_xui_list_view pList, float fScrollY);
XGE_API float xgeXuiListViewGetScroll(xge_xui_list_view pList);
XGE_API void xgeXuiListViewSetSelect(xge_xui_list_view pList, xge_xui_select_proc procSelect, void* pUser);
XGE_API void xgeXuiListViewSetColors(xge_xui_list_view pList, uint32_t iBackground, uint32_t iRow, uint32_t iSelected, uint32_t iText, uint32_t iBar, uint32_t iThumb);
XGE_API int xgeXuiListViewEvent(xge_xui_list_view pList, const xge_event_t* pEvent);
XGE_API int xgeXuiListViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiListViewPaintProc(xge_xui_widget pWidget, void* pUser);

#ifdef __cplusplus
}
#endif

#endif /* XGE_H */
