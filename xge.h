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

#ifdef __cplusplus
}
#endif

#ifdef XGE_IMPL

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <string.h>
#include "lib/xrt/xrt.h"

#ifndef SOKOL_NO_ENTRY
	#define SOKOL_NO_ENTRY
#endif
#ifndef SOKOL_GLCORE
	#define SOKOL_GLCORE
#endif
#ifndef SOKOL_IMPL
	#define SOKOL_IMPL
#endif
#include "lib/sokol/sokol_app.h"

#ifndef XGE_GL_IMPL
	#define XGE_GL_IMPL
#endif
#include "src/xge_gl.h"

#ifndef GL_DST_COLOR
	#define GL_DST_COLOR 0x0306
#endif
#ifndef GL_ONE_MINUS_SRC_COLOR
	#define GL_ONE_MINUS_SRC_COLOR 0x0301
#endif

#define XGE_RESOURCE_PROVIDER_MAX	8

#define STBI_MALLOC(sz) xrtMalloc(sz)
#define STBI_REALLOC(p, sz) xrtRealloc((p), (sz))
#define STBI_FREE(p) xrtFree(p)
#include "lib/stb/stb_image.h"

#ifndef XGE_NO_TEXT
	#define STBTT_malloc(x, u) ((void)(u), xrtMalloc(x))
	#define STBTT_free(x, u) ((void)(u), xrtFree(x))
	#include "lib/stb/stb_truetype.h"
#endif

#ifndef XGE_NO_AUDIO
	#ifndef MINIAUDIO_IMPLEMENTATION
		#define MINIAUDIO_IMPLEMENTATION
	#endif
	#include "lib/miniaudio/miniaudio.h"
#endif

#if defined(__linux__) || defined(__APPLE__)
	#include <dlfcn.h>
#endif

// XGE 全局运行状态
typedef struct xge_context_t {
	int bInitialized;
	int bRunning;
	int bSokolRunning;
	int iWidth;
	int iHeight;
	int iFPS;
	int iBlend;
	float fDelta;
	uint32_t iClearColor;
	xge_desc_t objDesc;
	xge_scene_proc procFrame;
	void* pFrameUser;
	unsigned char arrKeyDown[XGE_KEY_COUNT];
	unsigned char arrKeyPressed[XGE_KEY_COUNT];
	unsigned char arrKeyReleased[XGE_KEY_COUNT];
	float fMouseX;
	float fMouseY;
	float fMouseDX;
	float fMouseDY;
	float fMouseWheelX;
	float fMouseWheelY;
	uint32_t iTextCodepoint;
	unsigned int iMouseButtons;
	xge_touch_point_t arrTouches[XGE_TOUCH_MAX];
	int iTouchCount;
	xge_camera_t tCamera;
	xge_resource_provider_t arrResourceProviders[XGE_RESOURCE_PROVIDER_MAX];
	int iResourceProviderCount;
#ifndef XGE_NO_AUDIO
	int bAudioInitialized;
	void* pAudioEngine;
	xge_audio_listener_t tAudioListener;
#endif
} xge_context_t;

// MVP 纹理渲染器
typedef struct xge_texture_renderer_t {
	int bInitialized;
	GLuint iProgram;
	GLuint iVAO;
	GLuint iVBO;
	GLint iLocResolution;
	GLint iLocTexture;
	GLint iLocColor;
} xge_texture_renderer_t;

static xge_context_t g_xge;
static xge_texture_renderer_t g_xgeTextureRenderer;

// Sokol 分配函数接入 xrt
static void* __xgeSokolAlloc(size_t iSize, void* pUser)
{
	(void)pUser;
	return xrtMalloc(iSize);
}

// Sokol 释放函数接入 xrt
static void __xgeSokolFree(void* pData, void* pUser)
{
	(void)pUser;
	xrtFree(pData);
}

static int __xgeUriSchemeLen(const char* sURI);
static int __xgeSchemeEqual(const char* sA, int iASize, const char* sB);

#ifndef XGE_NO_AUDIO
static void* __xgeMaMalloc(size_t iSize, void* pUser)
{
	(void)pUser;
	return xrtMalloc(iSize);
}

static void* __xgeMaRealloc(void* pData, size_t iSize, void* pUser)
{
	(void)pUser;
	return xrtRealloc(pData, iSize);
}

static void __xgeMaFree(void* pData, void* pUser)
{
	(void)pUser;
	xrtFree(pData);
}
#endif

static char* __xgeStrDup(const char* sText)
{
	size_t iSize;
	char* sCopy;

	if ( sText == NULL ) {
		return NULL;
	}
	iSize = strlen(sText) + 1;
	sCopy = (char*)xrtMalloc(iSize);
	if ( sCopy == NULL ) {
		return NULL;
	}
	memcpy(sCopy, sText, iSize);
	return sCopy;
}

static char* __xgePathResolve(const char* sPath)
{
	if ( sPath == NULL ) {
		return NULL;
	}
	if ( __xgeUriSchemeLen(sPath) > 0 ) {
		if ( __xgeSchemeEqual(sPath, __xgeUriSchemeLen(sPath), "res") || __xgeSchemeEqual(sPath, __xgeUriSchemeLen(sPath), "file") ) {
			sPath += __xgeUriSchemeLen(sPath) + 3;
		} else {
			return NULL;
		}
	}
	if ( xrtPathIsAbs((str)sPath, 0) ) {
		return __xgeStrDup(sPath);
	}
	if ( xCore.AppPath != NULL ) {
		return (char*)xrtPathJoin(2, xCore.AppPath, (str)sPath);
	}
	return __xgeStrDup(sPath);
}

// 使用 xrt 读取完整文件，优先基于 AppPath 处理相对路径
static void* __xgeFileGetAll(const char* sPath, size_t* pSize)
{
	void* pData;
	str sFullPath;

	if ( pSize != NULL ) {
		*pSize = 0;
	}
	if ( sPath == NULL ) {
		return NULL;
	}

	if ( xrtPathIsAbs((str)sPath, 0) ) {
		return xrtFileGetAll((str)sPath, pSize);
	}

	pData = NULL;
	sFullPath = NULL;
	if ( xCore.AppPath != NULL ) {
		sFullPath = xrtPathJoin(2, xCore.AppPath, (str)sPath);
		if ( sFullPath != NULL ) {
			pData = xrtFileGetAll(sFullPath, pSize);
			xrtFree(sFullPath);
			if ( pData != NULL ) {
				return pData;
			}
		}
	}

	return xrtFileGetAll((str)sPath, pSize);
}

static int __xgeUriSchemeLen(const char* sURI)
{
	int i;

	if ( sURI == NULL ) {
		return 0;
	}
	for ( i = 0; sURI[i] != '\0'; i++ ) {
		if ( (sURI[i] == ':') && (sURI[i + 1] == '/') && (sURI[i + 2] == '/') ) {
			return i;
		}
		if ( (sURI[i] == '/') || (sURI[i] == '\\') ) {
			return 0;
		}
	}
	return 0;
}

static int __xgeSchemeEqual(const char* sA, int iASize, const char* sB)
{
	int i;

	if ( (sA == NULL) || (sB == NULL) || (iASize <= 0) ) {
		return 0;
	}
	for ( i = 0; i < iASize; i++ ) {
		if ( sA[i] != sB[i] ) {
			return 0;
		}
	}
	return sB[iASize] == '\0';
}

static int __xgeResourceFindProvider(const char* sURI, int iSchemeLen)
{
	int i;

	for ( i = g_xge.iResourceProviderCount - 1; i >= 0; i-- ) {
		if ( __xgeSchemeEqual(sURI, iSchemeLen, g_xge.arrResourceProviders[i].sScheme) ) {
			return i;
		}
	}
	return -1;
}

#if defined(_WIN32) || defined(_WIN64)
static HMODULE g_xgeOpenGL = NULL;

// windows OpenGL 函数加载
static void* __xgeGLGetProc(const char* sName)
{
	void* pProc;

	pProc = (void*)wglGetProcAddress(sName);
	if ( (pProc == NULL) || (pProc == (void*)0x1) || (pProc == (void*)0x2) || (pProc == (void*)0x3) || (pProc == (void*)-1) ) {
		if ( g_xgeOpenGL == NULL ) {
			g_xgeOpenGL = LoadLibraryA("opengl32.dll");
		}
		if ( g_xgeOpenGL != NULL ) {
			pProc = (void*)GetProcAddress(g_xgeOpenGL, sName);
		}
	}
	return pProc;
}
#elif defined(__APPLE__)
static void* g_xgeOpenGL = NULL;

// macOS OpenGL 函数加载
static void* __xgeGLGetProc(const char* sName)
{
	if ( g_xgeOpenGL == NULL ) {
		g_xgeOpenGL = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);
	}
	if ( g_xgeOpenGL != NULL ) {
		return dlsym(g_xgeOpenGL, sName);
	}
	return NULL;
}
#else
static void* g_xgeOpenGL = NULL;

// Linux OpenGL 函数加载
static void* __xgeGLGetProc(const char* sName)
{
	if ( g_xgeOpenGL == NULL ) {
		g_xgeOpenGL = dlopen("libGL.so.1", RTLD_LAZY | RTLD_GLOBAL);
		if ( g_xgeOpenGL == NULL ) {
			g_xgeOpenGL = dlopen("libGL.so", RTLD_LAZY | RTLD_GLOBAL);
		}
	}
	if ( g_xgeOpenGL != NULL ) {
		return dlsym(g_xgeOpenGL, sName);
	}
	return NULL;
}
#endif

// 编译 shader
static GLuint __xgeCompileShader(GLenum iType, const char* sSource)
{
	GLuint iShader;
	GLint bSuccess;
	char arrLog[512];

	iShader = glCreateShader(iType);
	glShaderSource(iShader, 1, &sSource, NULL);
	glCompileShader(iShader);
	glGetShaderiv(iShader, GL_COMPILE_STATUS, &bSuccess);
	if ( bSuccess == 0 ) {
		glGetShaderInfoLog(iShader, 512, NULL, arrLog);
		xrtSetError(arrLog, false);
		glDeleteShader(iShader);
		return 0;
	}
	return iShader;
}

// 初始化 MVP 纹理渲染器
static int __xgeTextureRendererInit(void)
{
	GLuint iVS;
	GLuint iFS;
	GLint bSuccess;
	char arrLog[512];
	const char* sVS =
		"#version 330 core\n"
		"layout (location = 0) in vec2 aPos;\n"
		"layout (location = 1) in vec2 aUV;\n"
		"uniform vec2 uResolution;\n"
		"out vec2 vUV;\n"
		"void main() {\n"
		"	vec2 pos = (aPos / uResolution) * 2.0 - 1.0;\n"
		"	pos.y = -pos.y;\n"
		"	gl_Position = vec4(pos, 0.0, 1.0);\n"
		"	vUV = aUV;\n"
		"}\n";
	const char* sFS =
		"#version 330 core\n"
		"in vec2 vUV;\n"
		"uniform vec4 uColor;\n"
		"uniform sampler2D uTexture;\n"
		"out vec4 FragColor;\n"
		"void main() {\n"
		"	FragColor = texture(uTexture, vUV) * uColor;\n"
		"}\n";

	if ( g_xgeTextureRenderer.bInitialized ) {
		return XGE_OK;
	}

	iVS = __xgeCompileShader(GL_VERTEX_SHADER, sVS);
	iFS = __xgeCompileShader(GL_FRAGMENT_SHADER, sFS);
	if ( (iVS == 0) || (iFS == 0) ) {
		return XGE_ERROR_GPU_FAILED;
	}

	g_xgeTextureRenderer.iProgram = glCreateProgram();
	glAttachShader(g_xgeTextureRenderer.iProgram, iVS);
	glAttachShader(g_xgeTextureRenderer.iProgram, iFS);
	glLinkProgram(g_xgeTextureRenderer.iProgram);
	glGetProgramiv(g_xgeTextureRenderer.iProgram, GL_LINK_STATUS, &bSuccess);
	glDeleteShader(iVS);
	glDeleteShader(iFS);
	if ( bSuccess == 0 ) {
		glGetProgramInfoLog(g_xgeTextureRenderer.iProgram, 512, NULL, arrLog);
		xrtSetError(arrLog, false);
		return XGE_ERROR_GPU_FAILED;
	}

	g_xgeTextureRenderer.iLocResolution = glGetUniformLocation(g_xgeTextureRenderer.iProgram, "uResolution");
	g_xgeTextureRenderer.iLocTexture = glGetUniformLocation(g_xgeTextureRenderer.iProgram, "uTexture");
	g_xgeTextureRenderer.iLocColor = glGetUniformLocation(g_xgeTextureRenderer.iProgram, "uColor");
	glGenVertexArrays(1, &g_xgeTextureRenderer.iVAO);
	glGenBuffers(1, &g_xgeTextureRenderer.iVBO);
	glBindVertexArray(g_xgeTextureRenderer.iVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_xgeTextureRenderer.iVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	g_xgeTextureRenderer.bInitialized = 1;
	return XGE_OK;
}

// RGBA 颜色转换为 OpenGL 浮点颜色
static void __xgeColorToFloat(uint32_t iColor, float* pR, float* pG, float* pB, float* pA)
{
	*pR = (float)XGE_COLOR_GET_R(iColor) / 255.0f;
	*pG = (float)XGE_COLOR_GET_G(iColor) / 255.0f;
	*pB = (float)XGE_COLOR_GET_B(iColor) / 255.0f;
	*pA = (float)XGE_COLOR_GET_A(iColor) / 255.0f;
}

static xge_vec2_t __xgeVec2Make(float fX, float fY)
{
	xge_vec2_t tPoint;

	tPoint.fX = fX;
	tPoint.fY = fY;
	return tPoint;
}

static void __xgeBlendApply(int iBlend)
{
	if ( g_xge.bSokolRunning == 0 ) {
		return;
	}

	switch ( iBlend ) {
		case XGE_BLEND_NONE:
			glDisable(GL_BLEND);
			break;

		case XGE_BLEND_ADD:
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			break;

		case XGE_BLEND_MULTIPLY:
			glEnable(GL_BLEND);
			glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
			break;

		case XGE_BLEND_SCREEN:
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
			break;

		case XGE_BLEND_CUSTOM:
			break;

		case XGE_BLEND_ALPHA:
		default:
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;
	}
}

static void __xgeTouchResetStationary(void);
static void __xgeTouchRemoveEnded(void);

// 每帧开始时清理瞬时输入状态
static void __xgeInputBeginFrame(void)
{
	memset(g_xge.arrKeyPressed, 0, sizeof(g_xge.arrKeyPressed));
	memset(g_xge.arrKeyReleased, 0, sizeof(g_xge.arrKeyReleased));
	g_xge.fMouseDX = 0.0f;
	g_xge.fMouseDY = 0.0f;
	g_xge.fMouseWheelX = 0.0f;
	g_xge.fMouseWheelY = 0.0f;
	g_xge.iTextCodepoint = 0;
	__xgeTouchRemoveEnded();
	__xgeTouchResetStationary();
}

// Sokol 初始化回调
static void __xgeSokolInit(void)
{
	g_xge.bSokolRunning = 1;
	g_xge.iWidth = sapp_width();
	g_xge.iHeight = sapp_height();
	g_xge.tCamera.tViewport.fW = (float)g_xge.iWidth;
	g_xge.tCamera.tViewport.fH = (float)g_xge.iHeight;
	if ( xge_gl_load((XgeGLLoadProc)__xgeGLGetProc) == 0 ) {
		xrtSetError("OpenGL function load failed.", false);
		sapp_quit();
		return;
	}
	__xgeBlendApply(g_xge.iBlend);
}

// Sokol 帧回调
static void __xgeSokolFrame(void)
{
	float fR;
	float fG;
	float fB;
	float fA;

	if ( g_xge.bRunning == 0 ) {
		sapp_quit();
		return;
	}

	g_xge.fDelta = (float)sapp_frame_duration();
	g_xge.iWidth = sapp_width();
	g_xge.iHeight = sapp_height();
	g_xge.tCamera.tViewport.fW = (float)g_xge.iWidth;
	g_xge.tCamera.tViewport.fH = (float)g_xge.iHeight;

	__xgeColorToFloat(g_xge.iClearColor, &fR, &fG, &fB, &fA);
	glViewport(0, 0, g_xge.iWidth, g_xge.iHeight);
	glClearColor(fR, fG, fB, fA);
	glClear(GL_COLOR_BUFFER_BIT);

	if ( g_xge.procFrame != NULL ) {
		if ( g_xge.procFrame(g_xge.pFrameUser) != 0 ) {
			xgeQuit();
		}
	}

	__xgeInputBeginFrame();
}

// Sokol 清理回调
static void __xgeSokolCleanup(void)
{
	g_xge.bSokolRunning = 0;
}

// 映射 Sokol 鼠标按钮
static unsigned int __xgeMouseButtonMask(sapp_mousebutton iButton)
{
	if ( iButton == SAPP_MOUSEBUTTON_LEFT ) {
		return XGE_MOUSE_LEFT;
	} else if ( iButton == SAPP_MOUSEBUTTON_RIGHT ) {
		return XGE_MOUSE_RIGHT;
	} else if ( iButton == SAPP_MOUSEBUTTON_MIDDLE ) {
		return XGE_MOUSE_MIDDLE;
	}
	return 0;
}

static int __xgeTouchPhaseFromSokol(sapp_event_type iType)
{
	switch ( iType ) {
		case SAPP_EVENTTYPE_TOUCHES_BEGAN:
			return XGE_TOUCH_BEGIN;
		case SAPP_EVENTTYPE_TOUCHES_MOVED:
			return XGE_TOUCH_MOVE;
		case SAPP_EVENTTYPE_TOUCHES_ENDED:
			return XGE_TOUCH_END;
		case SAPP_EVENTTYPE_TOUCHES_CANCELLED:
			return XGE_TOUCH_CANCEL;
		default:
			return XGE_TOUCH_NONE;
	}
}

static void __xgeTouchResetStationary(void)
{
	int i;

	for ( i = 0; i < g_xge.iTouchCount; i++ ) {
		if ( g_xge.arrTouches[i].bDown ) {
			g_xge.arrTouches[i].iPhase = XGE_TOUCH_STATIONARY;
			g_xge.arrTouches[i].bChanged = 0;
			g_xge.arrTouches[i].fDX = 0.0f;
			g_xge.arrTouches[i].fDY = 0.0f;
		}
	}
}

static int __xgeTouchFindIndex(uint64_t iId)
{
	int i;

	for ( i = 0; i < g_xge.iTouchCount; i++ ) {
		if ( (g_xge.arrTouches[i].bDown != 0) && (g_xge.arrTouches[i].iId == iId) ) {
			return i;
		}
	}
	return -1;
}

static void __xgeTouchRemoveEnded(void)
{
	int i;

	for ( i = 0; i < g_xge.iTouchCount; ) {
		if ( g_xge.arrTouches[i].bDown == 0 ) {
			g_xge.arrTouches[i] = g_xge.arrTouches[g_xge.iTouchCount - 1];
			g_xge.iTouchCount--;
		} else {
			i++;
		}
	}
}

static void __xgeTouchUpdate(const sapp_event* pEvent)
{
	int i;
	int iIndex;
	int iPhase;
	uint64_t iId;
	xge_touch_point_t* pPoint;
	const sapp_touchpoint* pSokolPoint;

	iPhase = __xgeTouchPhaseFromSokol(pEvent->type);
	__xgeTouchResetStationary();

	for ( i = 0; (i < pEvent->num_touches) && (i < XGE_TOUCH_MAX); i++ ) {
		pSokolPoint = &pEvent->touches[i];
		iId = (uint64_t)pSokolPoint->identifier;
		iIndex = __xgeTouchFindIndex(iId);
		if ( iIndex < 0 ) {
			if ( g_xge.iTouchCount >= XGE_TOUCH_MAX ) {
				continue;
			}
			iIndex = g_xge.iTouchCount++;
			memset(&g_xge.arrTouches[iIndex], 0, sizeof(g_xge.arrTouches[iIndex]));
			g_xge.arrTouches[iIndex].iId = iId;
		}

		pPoint = &g_xge.arrTouches[iIndex];
		pPoint->fDX = pSokolPoint->pos_x - pPoint->fX;
		pPoint->fDY = pSokolPoint->pos_y - pPoint->fY;
		pPoint->fX = pSokolPoint->pos_x;
		pPoint->fY = pSokolPoint->pos_y;
		pPoint->iPhase = iPhase;
		pPoint->bChanged = pSokolPoint->changed ? 1 : 0;
		pPoint->bDown = ((iPhase == XGE_TOUCH_END) || (iPhase == XGE_TOUCH_CANCEL)) ? 0 : 1;
	}
	(void)iPhase;
}

// Sokol 事件回调
static void __xgeSokolEvent(const sapp_event* pEvent)
{
	int iKey;
	unsigned int iButton;

	if ( pEvent == NULL ) {
		return;
	}

	switch ( pEvent->type ) {
		case SAPP_EVENTTYPE_KEY_DOWN:
			iKey = (int)pEvent->key_code;
			if ( (iKey >= 0) && (iKey < XGE_KEY_COUNT) ) {
				if ( g_xge.arrKeyDown[iKey] == 0 ) {
					g_xge.arrKeyPressed[iKey] = 1;
				}
				g_xge.arrKeyDown[iKey] = 1;
			}
			break;

		case SAPP_EVENTTYPE_KEY_UP:
			iKey = (int)pEvent->key_code;
			if ( (iKey >= 0) && (iKey < XGE_KEY_COUNT) ) {
				g_xge.arrKeyDown[iKey] = 0;
				g_xge.arrKeyReleased[iKey] = 1;
			}
			break;

		case SAPP_EVENTTYPE_CHAR:
			g_xge.iTextCodepoint = pEvent->char_code;
			break;

		case SAPP_EVENTTYPE_MOUSE_MOVE:
			g_xge.fMouseDX += pEvent->mouse_dx;
			g_xge.fMouseDY += pEvent->mouse_dy;
			g_xge.fMouseX = pEvent->mouse_x;
			g_xge.fMouseY = pEvent->mouse_y;
			break;

		case SAPP_EVENTTYPE_MOUSE_SCROLL:
			g_xge.fMouseWheelX += pEvent->scroll_x;
			g_xge.fMouseWheelY += pEvent->scroll_y;
			break;

		case SAPP_EVENTTYPE_MOUSE_DOWN:
			iButton = __xgeMouseButtonMask(pEvent->mouse_button);
			g_xge.iMouseButtons |= iButton;
			g_xge.fMouseX = pEvent->mouse_x;
			g_xge.fMouseY = pEvent->mouse_y;
			break;

		case SAPP_EVENTTYPE_MOUSE_UP:
			iButton = __xgeMouseButtonMask(pEvent->mouse_button);
			g_xge.iMouseButtons &= ~iButton;
			g_xge.fMouseX = pEvent->mouse_x;
			g_xge.fMouseY = pEvent->mouse_y;
			break;

		case SAPP_EVENTTYPE_TOUCHES_BEGAN:
		case SAPP_EVENTTYPE_TOUCHES_MOVED:
		case SAPP_EVENTTYPE_TOUCHES_ENDED:
		case SAPP_EVENTTYPE_TOUCHES_CANCELLED:
			__xgeTouchUpdate(pEvent);
			break;

		case SAPP_EVENTTYPE_RESIZED:
			g_xge.iWidth = pEvent->window_width;
			g_xge.iHeight = pEvent->window_height;
			break;

		case SAPP_EVENTTYPE_QUIT_REQUESTED:
			g_xge.bRunning = 0;
			break;

		default:
			break;
	}
}

// 构建 Sokol 描述信息
static sapp_desc __xgeMakeSokolDesc(void)
{
	sapp_desc objDesc;

	memset(&objDesc, 0, sizeof(objDesc));
	objDesc.init_cb = __xgeSokolInit;
	objDesc.frame_cb = __xgeSokolFrame;
	objDesc.cleanup_cb = __xgeSokolCleanup;
	objDesc.event_cb = __xgeSokolEvent;
	objDesc.width = g_xge.objDesc.iWidth;
	objDesc.height = g_xge.objDesc.iHeight;
	objDesc.window_title = g_xge.objDesc.sTitle ? g_xge.objDesc.sTitle : "XGE";
	objDesc.high_dpi = ((g_xge.objDesc.iFlags & XGE_INIT_HIGHDPI) != 0);
	objDesc.sample_count = 1;
	objDesc.swap_interval = ((g_xge.objDesc.iFlags & XGE_INIT_VSYNC) != 0) ? 1 : 0;
	objDesc.enable_clipboard = true;
	objDesc.icon.sokol_default = true;
	objDesc.allocator.alloc_fn = __xgeSokolAlloc;
	objDesc.allocator.free_fn = __xgeSokolFree;
	return objDesc;
}

int xgeInit(const xge_desc_t* pDesc)
{
	xge_desc_t objDesc;

	if ( g_xge.bInitialized ) {
		return XGE_ERROR_ALREADY_INITIALIZED;
	}

	memset(&g_xge, 0, sizeof(g_xge));
	memset(&objDesc, 0, sizeof(objDesc));

	if ( pDesc != NULL ) {
		objDesc = *pDesc;
	}
	if ( objDesc.iWidth <= 0 ) {
		objDesc.iWidth = 800;
	}
	if ( objDesc.iHeight <= 0 ) {
		objDesc.iHeight = 600;
	}
	if ( objDesc.sTitle == NULL ) {
		objDesc.sTitle = "XGE";
	}

	if ( xrtInit() == NULL ) {
		return XGE_ERROR_BACKEND_FAILED;
	}

	g_xge.objDesc = objDesc;
	g_xge.iWidth = objDesc.iWidth;
	g_xge.iHeight = objDesc.iHeight;
	g_xge.iClearColor = XGE_COLOR_RGBA(0, 0, 0, 255);
	g_xge.iBlend = XGE_BLEND_ALPHA;
	g_xge.fDelta = 1.0f / 60.0f;
	g_xge.tCamera = xgeCameraDefault((float)objDesc.iWidth, (float)objDesc.iHeight);
#ifndef XGE_NO_AUDIO
	g_xge.tAudioListener.tForward.fZ = -1.0f;
	g_xge.tAudioListener.tUp.fY = 1.0f;
#endif
	g_xge.bInitialized = 1;
	g_xge.bRunning = 1;
	return XGE_OK;
}

void xgeUnit(void)
{
	if ( g_xge.bInitialized == 0 ) {
		return;
	}
	g_xge.bInitialized = 0;
	g_xge.bRunning = 0;
	xgeAudioUnit();
	xrtUnit();
}

int xgeRun(xge_scene_proc procFrame, void* pUser)
{
	sapp_desc objDesc;

	if ( g_xge.bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}

	g_xge.procFrame = procFrame;
	g_xge.pFrameUser = pUser;
	objDesc = __xgeMakeSokolDesc();
	sapp_run(&objDesc);
	return XGE_OK;
}

void xgeQuit(void)
{
	g_xge.bRunning = 0;
	if ( g_xge.bSokolRunning ) {
		sapp_quit();
	}
}

int xgeFrame(void)
{
	if ( g_xge.bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	return XGE_ERROR_UNSUPPORTED;
}

int xgeGetWidth(void)
{
	return g_xge.iWidth;
}

int xgeGetHeight(void)
{
	return g_xge.iHeight;
}

float xgeGetDelta(void)
{
	return g_xge.fDelta;
}

int xgeGetFPS(void)
{
	return g_xge.iFPS;
}

void xgeSetTitle(const char* sTitle)
{
	if ( sTitle != NULL ) {
		sapp_set_window_title(sTitle);
	}
}

void xgeClear(uint32_t iColor)
{
	float fR;
	float fG;
	float fB;
	float fA;

	g_xge.iClearColor = iColor;
	if ( g_xge.bSokolRunning ) {
		__xgeColorToFloat(iColor, &fR, &fG, &fB, &fA);
		glClearColor(fR, fG, fB, fA);
		glClear(GL_COLOR_BUFFER_BIT);
	}
}

void xgePresent(void)
{
}

uint32_t xgeColorRGBA(int iR, int iG, int iB, int iA)
{
	return XGE_COLOR_RGBA(iR, iG, iB, iA);
}

xge_color_t xgeColorUnpack(uint32_t iColor)
{
	xge_color_t tColor;

	tColor.fR = (float)XGE_COLOR_GET_R(iColor) / 255.0f;
	tColor.fG = (float)XGE_COLOR_GET_G(iColor) / 255.0f;
	tColor.fB = (float)XGE_COLOR_GET_B(iColor) / 255.0f;
	tColor.fA = (float)XGE_COLOR_GET_A(iColor) / 255.0f;
	return tColor;
}

void xgeBlendSet(int iBlend)
{
	g_xge.iBlend = iBlend;
	__xgeBlendApply(iBlend);
}

int xgeBlendGet(void)
{
	return g_xge.iBlend;
}

xge_camera_t xgeCameraDefault(float fWidth, float fHeight)
{
	xge_camera_t tCamera;

	memset(&tCamera, 0, sizeof(tCamera));
	tCamera.tScale.fX = 1.0f;
	tCamera.tScale.fY = 1.0f;
	tCamera.tViewport.fX = 0.0f;
	tCamera.tViewport.fY = 0.0f;
	tCamera.tViewport.fW = fWidth;
	tCamera.tViewport.fH = fHeight;
	tCamera.iCoordinateMode = XGE_COORD_Y_DOWN;
	return tCamera;
}

void xgeCameraSet(const xge_camera_t* pCamera)
{
	if ( pCamera == NULL ) {
		return;
	}
	g_xge.tCamera = *pCamera;
	if ( g_xge.tCamera.tScale.fX == 0.0f ) {
		g_xge.tCamera.tScale.fX = 1.0f;
	}
	if ( g_xge.tCamera.tScale.fY == 0.0f ) {
		g_xge.tCamera.tScale.fY = 1.0f;
	}
}

xge_camera_t xgeCameraGet(void)
{
	return g_xge.tCamera;
}

xge_vec2_t xgeWorldToScreen(xge_vec2_t tPoint)
{
	xge_vec2_t tRet;
	xge_camera_t* pCamera;

	pCamera = &g_xge.tCamera;
	tRet.fX = (tPoint.fX - pCamera->tPosition.fX) * pCamera->tScale.fX;
	tRet.fY = (tPoint.fY - pCamera->tPosition.fY) * pCamera->tScale.fY;
	if ( pCamera->iCoordinateMode == XGE_COORD_CENTER ) {
		tRet.fX += pCamera->tViewport.fX + (pCamera->tViewport.fW * 0.5f);
		tRet.fY += pCamera->tViewport.fY + (pCamera->tViewport.fH * 0.5f);
	} else {
		tRet.fX += pCamera->tViewport.fX;
		tRet.fY += pCamera->tViewport.fY;
	}
	return tRet;
}

xge_vec2_t xgeScreenToWorld(xge_vec2_t tPoint)
{
	xge_vec2_t tRet;
	xge_camera_t* pCamera;
	float fX;
	float fY;

	pCamera = &g_xge.tCamera;
	fX = tPoint.fX - pCamera->tViewport.fX;
	fY = tPoint.fY - pCamera->tViewport.fY;
	if ( pCamera->iCoordinateMode == XGE_COORD_CENTER ) {
		fX -= pCamera->tViewport.fW * 0.5f;
		fY -= pCamera->tViewport.fH * 0.5f;
	}
	tRet.fX = (fX / pCamera->tScale.fX) + pCamera->tPosition.fX;
	tRet.fY = (fY / pCamera->tScale.fY) + pCamera->tPosition.fY;
	return tRet;
}

int xgeResourceProviderAdd(const xge_resource_provider_t* pProvider)
{
	if ( (pProvider == NULL) || (pProvider->sScheme == NULL) || (pProvider->load == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xge.iResourceProviderCount >= XGE_RESOURCE_PROVIDER_MAX ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	g_xge.arrResourceProviders[g_xge.iResourceProviderCount] = *pProvider;
	g_xge.iResourceProviderCount++;
	return XGE_OK;
}

void xgeResourceProviderClear(void)
{
	memset(g_xge.arrResourceProviders, 0, sizeof(g_xge.arrResourceProviders));
	g_xge.iResourceProviderCount = 0;
}

int xgeResourceLoad(const char* sURI, xge_resource_t* pResource)
{
	int iSchemeLen;
	int iProvider;
	int iRet;
	size_t iSize;
	void* pData;

	if ( (sURI == NULL) || (pResource == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pResource, 0, sizeof(*pResource));

	iSchemeLen = __xgeUriSchemeLen(sURI);
	if ( iSchemeLen > 0 ) {
		iProvider = __xgeResourceFindProvider(sURI, iSchemeLen);
		if ( iProvider >= 0 ) {
			iRet = g_xge.arrResourceProviders[iProvider].load(sURI, &pResource->pData, &pResource->iSize, g_xge.arrResourceProviders[iProvider].pUser);
			if ( iRet != XGE_OK ) {
				memset(pResource, 0, sizeof(*pResource));
				return iRet;
			}
			pResource->iProvider = iProvider + 1;
			pResource->free = g_xge.arrResourceProviders[iProvider].free;
			pResource->pUser = g_xge.arrResourceProviders[iProvider].pUser;
			return XGE_OK;
		}
		if ( __xgeSchemeEqual(sURI, iSchemeLen, "res") ) {
			sURI += iSchemeLen + 3;
		} else if ( __xgeSchemeEqual(sURI, iSchemeLen, "file") ) {
			sURI += iSchemeLen + 3;
		} else {
			return XGE_ERROR_UNSUPPORTED;
		}
	}

	pData = __xgeFileGetAll(sURI, &iSize);
	if ( pData == NULL ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	if ( iSize > (size_t)INT32_MAX ) {
		xrtFree(pData);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pResource->pData = pData;
	pResource->iSize = (int)iSize;
	pResource->iProvider = 0;
	return XGE_OK;
}

int xgeResourceLoadMemory(const void* pData, int iSize, xge_resource_t* pResource)
{
	void* pCopy;

	if ( (pData == NULL) || (iSize < 0) || (pResource == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pResource, 0, sizeof(*pResource));
	if ( iSize == 0 ) {
		return XGE_OK;
	}
	pCopy = xrtMalloc((size_t)iSize);
	if ( pCopy == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(pCopy, pData, (size_t)iSize);
	pResource->pData = pCopy;
	pResource->iSize = iSize;
	pResource->iProvider = 0;
	return XGE_OK;
}

void xgeResourceFree(xge_resource_t* pResource)
{
	if ( (pResource == NULL) || (pResource->pData == NULL) ) {
		return;
	}
	if ( pResource->free != NULL ) {
		pResource->free(pResource->pData, pResource->pUser);
	} else {
		xrtFree(pResource->pData);
	}
	memset(pResource, 0, sizeof(*pResource));
}

#ifndef XGE_NO_AUDIO
static xge_sound __xgeAudioAsSound(void* pObject)
{
	return (xge_sound)pObject;
}

static int __xgeAudioLoad(void* pObject, const char* sPath, int iType, uint32_t iFlags, xge_audio_group pGroup)
{
	xge_sound pSound;
	ma_sound* pMaSound;
	ma_sound_group* pMaGroup;
	char* sFullPath;
	ma_uint32 iMaFlags;

	if ( (pObject == NULL) || (sPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeAudioIsReady() == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	pSound = __xgeAudioAsSound(pObject);
	memset(pSound, 0, sizeof(*pSound));
	pMaSound = (ma_sound*)xrtMalloc(sizeof(*pMaSound));
	if ( pMaSound == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	sFullPath = __xgePathResolve(sPath);
	if ( sFullPath == NULL ) {
		xrtFree(pMaSound);
		return XGE_ERROR_UNSUPPORTED;
	}
	iMaFlags = iFlags;
	pMaGroup = NULL;
	if ( pGroup != NULL ) {
		pMaGroup = (ma_sound_group*)pGroup->pBackend;
	}
	if ( ma_sound_init_from_file((ma_engine*)g_xge.pAudioEngine, sFullPath, iMaFlags, pMaGroup, NULL, pMaSound) != MA_SUCCESS ) {
		xrtFree(sFullPath);
		xrtFree(pMaSound);
		return XGE_ERROR_AUDIO_FAILED;
	}
	xrtFree(sFullPath);
	pSound->iRefCount = 1;
	pSound->iType = iType;
	pSound->iFlags = iFlags;
	pSound->pBackend = pMaSound;
	return XGE_OK;
}

int xgeAudioInit(void)
{
	ma_allocation_callbacks tAlloc;
	ma_engine_config tConfig;
	ma_engine* pEngine;

	if ( g_xge.bAudioInitialized ) {
		return XGE_OK;
	}
	if ( g_xge.bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	pEngine = (ma_engine*)xrtMalloc(sizeof(*pEngine));
	if ( pEngine == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(&tAlloc, 0, sizeof(tAlloc));
	tAlloc.onMalloc = __xgeMaMalloc;
	tAlloc.onRealloc = __xgeMaRealloc;
	tAlloc.onFree = __xgeMaFree;
	tConfig = ma_engine_config_init();
	tConfig.allocationCallbacks = tAlloc;
	if ( ma_engine_init(&tConfig, pEngine) != MA_SUCCESS ) {
		xrtFree(pEngine);
		return XGE_ERROR_AUDIO_FAILED;
	}
	g_xge.pAudioEngine = pEngine;
	g_xge.bAudioInitialized = 1;
	xgeAudioListenerSet(&g_xge.tAudioListener);
	return XGE_OK;
}

void xgeAudioUnit(void)
{
	if ( g_xge.bAudioInitialized == 0 ) {
		return;
	}
	ma_engine_uninit((ma_engine*)g_xge.pAudioEngine);
	xrtFree(g_xge.pAudioEngine);
	g_xge.pAudioEngine = NULL;
	g_xge.bAudioInitialized = 0;
}

int xgeAudioIsReady(void)
{
	return g_xge.bAudioInitialized;
}

void xgeAudioSetVolume(float fVolume)
{
	if ( g_xge.bAudioInitialized ) {
		ma_engine_set_volume((ma_engine*)g_xge.pAudioEngine, fVolume);
	}
}

float xgeAudioGetVolume(void)
{
	if ( g_xge.bAudioInitialized == 0 ) {
		return 0.0f;
	}
	return ma_engine_get_volume((ma_engine*)g_xge.pAudioEngine);
}

void xgeAudioListenerSet(const xge_audio_listener_t* pListener)
{
	if ( pListener == NULL ) {
		return;
	}
	g_xge.tAudioListener = *pListener;
	if ( g_xge.bAudioInitialized ) {
		ma_engine_listener_set_position((ma_engine*)g_xge.pAudioEngine, 0, pListener->tPosition.fX, pListener->tPosition.fY, pListener->tPosition.fZ);
		ma_engine_listener_set_direction((ma_engine*)g_xge.pAudioEngine, 0, pListener->tForward.fX, pListener->tForward.fY, pListener->tForward.fZ);
		ma_engine_listener_set_world_up((ma_engine*)g_xge.pAudioEngine, 0, pListener->tUp.fX, pListener->tUp.fY, pListener->tUp.fZ);
	}
}

xge_audio_listener_t xgeAudioListenerGet(void)
{
	return g_xge.tAudioListener;
}

int xgeAudioGroupInit(xge_audio_group pGroup)
{
	ma_sound_group* pMaGroup;

	if ( pGroup == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeAudioIsReady() == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	memset(pGroup, 0, sizeof(*pGroup));
	pMaGroup = (ma_sound_group*)xrtMalloc(sizeof(*pMaGroup));
	if ( pMaGroup == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( ma_sound_group_init((ma_engine*)g_xge.pAudioEngine, MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, pMaGroup) != MA_SUCCESS ) {
		xrtFree(pMaGroup);
		return XGE_ERROR_AUDIO_FAILED;
	}
	pGroup->fVolume = 1.0f;
	pGroup->pBackend = pMaGroup;
	return XGE_OK;
}

void xgeAudioGroupFree(xge_audio_group pGroup)
{
	if ( pGroup == NULL ) {
		return;
	}
	if ( pGroup->pBackend != NULL ) {
		ma_sound_group_uninit((ma_sound_group*)pGroup->pBackend);
		xrtFree(pGroup->pBackend);
	}
	memset(pGroup, 0, sizeof(*pGroup));
}

void xgeAudioGroupSetVolume(xge_audio_group pGroup, float fVolume)
{
	if ( pGroup == NULL ) {
		return;
	}
	pGroup->fVolume = fVolume;
	if ( pGroup->pBackend != NULL ) {
		ma_sound_group_set_volume((ma_sound_group*)pGroup->pBackend, fVolume);
	}
}

float xgeAudioGroupGetVolume(xge_audio_group pGroup)
{
	if ( pGroup == NULL ) {
		return 0.0f;
	}
	if ( pGroup->pBackend != NULL ) {
		pGroup->fVolume = ma_sound_group_get_volume((ma_sound_group*)pGroup->pBackend);
	}
	return pGroup->fVolume;
}

void xgeAudioGroupFade(xge_audio_group pGroup, float fFrom, float fTo, int iMilliseconds)
{
	if ( (pGroup != NULL) && (pGroup->pBackend != NULL) ) {
		pGroup->fVolume = fTo;
		ma_sound_group_set_fade_in_milliseconds((ma_sound_group*)pGroup->pBackend, fFrom, fTo, (ma_uint64)iMilliseconds);
	}
}

int xgeSoundLoad(xge_sound pSound, const char* sPath)
{
	return xgeSoundLoadGroup(pSound, sPath, NULL);
}

int xgeSoundLoadGroup(xge_sound pSound, const char* sPath, xge_audio_group pGroup)
{
	return __xgeAudioLoad(pSound, sPath, XGE_AUDIO_SOUND, 0, pGroup);
}

int xgeSoundPlay(xge_sound pSound)
{
	if ( (pSound == NULL) || (pSound->pBackend == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return ma_sound_start((ma_sound*)pSound->pBackend) == MA_SUCCESS ? XGE_OK : XGE_ERROR_AUDIO_FAILED;
}

int xgeSoundStop(xge_sound pSound)
{
	if ( (pSound == NULL) || (pSound->pBackend == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( ma_sound_stop((ma_sound*)pSound->pBackend) != MA_SUCCESS ) {
		return XGE_ERROR_AUDIO_FAILED;
	}
	ma_sound_seek_to_pcm_frame((ma_sound*)pSound->pBackend, 0);
	return XGE_OK;
}

int xgeSoundPause(xge_sound pSound)
{
	if ( (pSound == NULL) || (pSound->pBackend == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return ma_sound_stop((ma_sound*)pSound->pBackend) == MA_SUCCESS ? XGE_OK : XGE_ERROR_AUDIO_FAILED;
}

int xgeSoundResume(xge_sound pSound)
{
	return xgeSoundPlay(pSound);
}

int xgeSoundAddRef(xge_sound pSound)
{
	if ( pSound == NULL ) {
		return 0;
	}
	pSound->iRefCount++;
	return pSound->iRefCount;
}

void xgeSoundFree(xge_sound pSound)
{
	if ( pSound == NULL ) {
		return;
	}
	if ( pSound->iRefCount > 1 ) {
		pSound->iRefCount--;
		return;
	}
	if ( pSound->pBackend != NULL ) {
		ma_sound_uninit((ma_sound*)pSound->pBackend);
		xrtFree(pSound->pBackend);
	}
	memset(pSound, 0, sizeof(*pSound));
}

void xgeSoundSetLoop(xge_sound pSound, int bLoop)
{
	if ( (pSound != NULL) && (pSound->pBackend != NULL) ) {
		ma_sound_set_looping((ma_sound*)pSound->pBackend, bLoop ? MA_TRUE : MA_FALSE);
	}
}

void xgeSoundSetVolume(xge_sound pSound, float fVolume)
{
	if ( (pSound != NULL) && (pSound->pBackend != NULL) ) {
		ma_sound_set_volume((ma_sound*)pSound->pBackend, fVolume);
	}
}

void xgeSoundSetPosition(xge_sound pSound, float fX, float fY, float fZ)
{
	if ( (pSound != NULL) && (pSound->pBackend != NULL) ) {
		ma_sound_set_positioning((ma_sound*)pSound->pBackend, ma_positioning_absolute);
		ma_sound_set_position((ma_sound*)pSound->pBackend, fX, fY, fZ);
	}
}

void xgeSoundFade(xge_sound pSound, float fFrom, float fTo, int iMilliseconds)
{
	if ( (pSound != NULL) && (pSound->pBackend != NULL) ) {
		ma_sound_set_fade_in_milliseconds((ma_sound*)pSound->pBackend, fFrom, fTo, (ma_uint64)iMilliseconds);
	}
}

int xgeSoundIsPlaying(xge_sound pSound)
{
	if ( (pSound == NULL) || (pSound->pBackend == NULL) ) {
		return 0;
	}
	return ma_sound_is_playing((ma_sound*)pSound->pBackend) ? 1 : 0;
}

int xgeMusicLoad(xge_music pMusic, const char* sPath)
{
	return xgeMusicLoadGroup(pMusic, sPath, NULL);
}

int xgeMusicLoadGroup(xge_music pMusic, const char* sPath, xge_audio_group pGroup)
{
	return __xgeAudioLoad(pMusic, sPath, XGE_AUDIO_MUSIC, MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION, pGroup);
}

int xgeMusicPlay(xge_music pMusic)
{
	return xgeSoundPlay((xge_sound)pMusic);
}

int xgeMusicStop(xge_music pMusic)
{
	return xgeSoundStop((xge_sound)pMusic);
}

int xgeMusicPause(xge_music pMusic)
{
	return xgeSoundPause((xge_sound)pMusic);
}

int xgeMusicResume(xge_music pMusic)
{
	return xgeSoundResume((xge_sound)pMusic);
}

void xgeMusicFree(xge_music pMusic)
{
	xgeSoundFree((xge_sound)pMusic);
}

void xgeMusicSetLoop(xge_music pMusic, int bLoop)
{
	xgeSoundSetLoop((xge_sound)pMusic, bLoop);
}

void xgeMusicSetVolume(xge_music pMusic, float fVolume)
{
	xgeSoundSetVolume((xge_sound)pMusic, fVolume);
}

void xgeMusicFade(xge_music pMusic, float fFrom, float fTo, int iMilliseconds)
{
	xgeSoundFade((xge_sound)pMusic, fFrom, fTo, iMilliseconds);
}

int xgeMusicIsPlaying(xge_music pMusic)
{
	return xgeSoundIsPlaying((xge_sound)pMusic);
}

int xgeStreamOpen(xge_stream pStream, const char* sPath)
{
	return xgeStreamOpenGroup(pStream, sPath, NULL);
}

int xgeStreamOpenGroup(xge_stream pStream, const char* sPath, xge_audio_group pGroup)
{
	return __xgeAudioLoad(pStream, sPath, XGE_AUDIO_STREAM, MA_SOUND_FLAG_STREAM, pGroup);
}

int xgeStreamPlay(xge_stream pStream)
{
	return xgeSoundPlay((xge_sound)pStream);
}

int xgeStreamStop(xge_stream pStream)
{
	return xgeSoundStop((xge_sound)pStream);
}

int xgeStreamPause(xge_stream pStream)
{
	return xgeSoundPause((xge_sound)pStream);
}

int xgeStreamResume(xge_stream pStream)
{
	return xgeSoundResume((xge_sound)pStream);
}

void xgeStreamClose(xge_stream pStream)
{
	xgeSoundFree((xge_sound)pStream);
}

void xgeStreamSetLoop(xge_stream pStream, int bLoop)
{
	xgeSoundSetLoop((xge_sound)pStream, bLoop);
}

void xgeStreamSetVolume(xge_stream pStream, float fVolume)
{
	xgeSoundSetVolume((xge_sound)pStream, fVolume);
}

void xgeStreamSetPosition(xge_stream pStream, float fX, float fY, float fZ)
{
	xgeSoundSetPosition((xge_sound)pStream, fX, fY, fZ);
}

void xgeStreamFade(xge_stream pStream, float fFrom, float fTo, int iMilliseconds)
{
	xgeSoundFade((xge_sound)pStream, fFrom, fTo, iMilliseconds);
}

int xgeStreamIsPlaying(xge_stream pStream)
{
	return xgeSoundIsPlaying((xge_sound)pStream);
}
#else
int xgeAudioInit(void) { return XGE_ERROR_UNSUPPORTED; }
void xgeAudioUnit(void) {}
int xgeAudioIsReady(void) { return 0; }
void xgeAudioSetVolume(float fVolume) { (void)fVolume; }
float xgeAudioGetVolume(void) { return 0.0f; }
void xgeAudioListenerSet(const xge_audio_listener_t* pListener) { (void)pListener; }
xge_audio_listener_t xgeAudioListenerGet(void) { xge_audio_listener_t tListener; memset(&tListener, 0, sizeof(tListener)); return tListener; }
int xgeAudioGroupInit(xge_audio_group pGroup) { (void)pGroup; return XGE_ERROR_UNSUPPORTED; }
void xgeAudioGroupFree(xge_audio_group pGroup) { (void)pGroup; }
void xgeAudioGroupSetVolume(xge_audio_group pGroup, float fVolume) { (void)pGroup; (void)fVolume; }
float xgeAudioGroupGetVolume(xge_audio_group pGroup) { (void)pGroup; return 0.0f; }
void xgeAudioGroupFade(xge_audio_group pGroup, float fFrom, float fTo, int iMilliseconds) { (void)pGroup; (void)fFrom; (void)fTo; (void)iMilliseconds; }
int xgeSoundLoad(xge_sound pSound, const char* sPath) { (void)pSound; (void)sPath; return XGE_ERROR_UNSUPPORTED; }
int xgeSoundLoadGroup(xge_sound pSound, const char* sPath, xge_audio_group pGroup) { (void)pSound; (void)sPath; (void)pGroup; return XGE_ERROR_UNSUPPORTED; }
int xgeSoundPlay(xge_sound pSound) { (void)pSound; return XGE_ERROR_UNSUPPORTED; }
int xgeSoundStop(xge_sound pSound) { (void)pSound; return XGE_ERROR_UNSUPPORTED; }
int xgeSoundPause(xge_sound pSound) { (void)pSound; return XGE_ERROR_UNSUPPORTED; }
int xgeSoundResume(xge_sound pSound) { (void)pSound; return XGE_ERROR_UNSUPPORTED; }
int xgeSoundAddRef(xge_sound pSound) { (void)pSound; return 0; }
void xgeSoundFree(xge_sound pSound) { (void)pSound; }
void xgeSoundSetLoop(xge_sound pSound, int bLoop) { (void)pSound; (void)bLoop; }
void xgeSoundSetVolume(xge_sound pSound, float fVolume) { (void)pSound; (void)fVolume; }
void xgeSoundSetPosition(xge_sound pSound, float fX, float fY, float fZ) { (void)pSound; (void)fX; (void)fY; (void)fZ; }
void xgeSoundFade(xge_sound pSound, float fFrom, float fTo, int iMilliseconds) { (void)pSound; (void)fFrom; (void)fTo; (void)iMilliseconds; }
int xgeSoundIsPlaying(xge_sound pSound) { (void)pSound; return 0; }
int xgeMusicLoad(xge_music pMusic, const char* sPath) { (void)pMusic; (void)sPath; return XGE_ERROR_UNSUPPORTED; }
int xgeMusicLoadGroup(xge_music pMusic, const char* sPath, xge_audio_group pGroup) { (void)pMusic; (void)sPath; (void)pGroup; return XGE_ERROR_UNSUPPORTED; }
int xgeMusicPlay(xge_music pMusic) { (void)pMusic; return XGE_ERROR_UNSUPPORTED; }
int xgeMusicStop(xge_music pMusic) { (void)pMusic; return XGE_ERROR_UNSUPPORTED; }
int xgeMusicPause(xge_music pMusic) { (void)pMusic; return XGE_ERROR_UNSUPPORTED; }
int xgeMusicResume(xge_music pMusic) { (void)pMusic; return XGE_ERROR_UNSUPPORTED; }
void xgeMusicFree(xge_music pMusic) { (void)pMusic; }
void xgeMusicSetLoop(xge_music pMusic, int bLoop) { (void)pMusic; (void)bLoop; }
void xgeMusicSetVolume(xge_music pMusic, float fVolume) { (void)pMusic; (void)fVolume; }
void xgeMusicFade(xge_music pMusic, float fFrom, float fTo, int iMilliseconds) { (void)pMusic; (void)fFrom; (void)fTo; (void)iMilliseconds; }
int xgeMusicIsPlaying(xge_music pMusic) { (void)pMusic; return 0; }
int xgeStreamOpen(xge_stream pStream, const char* sPath) { (void)pStream; (void)sPath; return XGE_ERROR_UNSUPPORTED; }
int xgeStreamOpenGroup(xge_stream pStream, const char* sPath, xge_audio_group pGroup) { (void)pStream; (void)sPath; (void)pGroup; return XGE_ERROR_UNSUPPORTED; }
int xgeStreamPlay(xge_stream pStream) { (void)pStream; return XGE_ERROR_UNSUPPORTED; }
int xgeStreamStop(xge_stream pStream) { (void)pStream; return XGE_ERROR_UNSUPPORTED; }
int xgeStreamPause(xge_stream pStream) { (void)pStream; return XGE_ERROR_UNSUPPORTED; }
int xgeStreamResume(xge_stream pStream) { (void)pStream; return XGE_ERROR_UNSUPPORTED; }
void xgeStreamClose(xge_stream pStream) { (void)pStream; }
void xgeStreamSetLoop(xge_stream pStream, int bLoop) { (void)pStream; (void)bLoop; }
void xgeStreamSetVolume(xge_stream pStream, float fVolume) { (void)pStream; (void)fVolume; }
void xgeStreamSetPosition(xge_stream pStream, float fX, float fY, float fZ) { (void)pStream; (void)fX; (void)fY; (void)fZ; }
void xgeStreamFade(xge_stream pStream, float fFrom, float fTo, int iMilliseconds) { (void)pStream; (void)fFrom; (void)fTo; (void)iMilliseconds; }
int xgeStreamIsPlaying(xge_stream pStream) { (void)pStream; return 0; }
#endif

#ifndef XGE_NO_TEXT
typedef struct xge_glyph_cache_t {
	xge_glyph_t tGlyph;
	struct xge_glyph_cache_t* pNext;
} xge_glyph_cache_t;

typedef struct xge_glyph_atlas_page_t {
	int iCursorX;
	int iCursorY;
	int iRowHeight;
	int bDirty;
	unsigned char* pPixels;
	xge_texture_t tTexture;
} xge_glyph_atlas_page_t;

static stbtt_fontinfo* __xgeFontInfo(xge_font pFont)
{
	if ( pFont == NULL ) {
		return NULL;
	}
	return (stbtt_fontinfo*)pFont->pBackend;
}

static void __xgeFontAtlasFree(xge_font pFont)
{
	xge_glyph_atlas_page_t* pPages;
	xge_glyph_cache_t* pGlyph;
	xge_glyph_cache_t* pNext;
	int i;

	if ( pFont == NULL ) {
		return;
	}
	pPages = (xge_glyph_atlas_page_t*)pFont->tAtlas.pPages;
	if ( pPages != NULL ) {
		for ( i = 0; i < pFont->tAtlas.iPageCount; i++ ) {
			xgeTextureFree(&pPages[i].tTexture);
			xrtFree(pPages[i].pPixels);
		}
		xrtFree(pPages);
	}
	pGlyph = (xge_glyph_cache_t*)pFont->pGlyphs;
	while ( pGlyph != NULL ) {
		pNext = pGlyph->pNext;
		xrtFree(pGlyph);
		pGlyph = pNext;
	}
	pFont->tAtlas.pPages = NULL;
	pFont->tAtlas.iPageCount = 0;
	pFont->pGlyphs = NULL;
}

static xge_glyph_cache_t* __xgeFontGlyphFind(xge_font pFont, uint32_t iCodepoint)
{
	xge_glyph_cache_t* pGlyph;

	pGlyph = (xge_glyph_cache_t*)pFont->pGlyphs;
	while ( pGlyph != NULL ) {
		if ( pGlyph->tGlyph.iCodepoint == iCodepoint ) {
			return pGlyph;
		}
		pGlyph = pGlyph->pNext;
	}
	return NULL;
}

static xge_glyph_atlas_page_t* __xgeFontAtlasAddPage(xge_font pFont)
{
	xge_glyph_atlas_page_t* pPages;
	xge_glyph_atlas_page_t* pPage;
	int iNewCount;
	int iPixelSize;

	iNewCount = pFont->tAtlas.iPageCount + 1;
	pPages = (xge_glyph_atlas_page_t*)xrtRealloc(pFont->tAtlas.pPages, sizeof(*pPages) * (size_t)iNewCount);
	if ( pPages == NULL ) {
		return NULL;
	}
	pFont->tAtlas.pPages = pPages;
	pPage = &pPages[iNewCount - 1];
	memset(pPage, 0, sizeof(*pPage));
	iPixelSize = pFont->tAtlas.iPageWidth * pFont->tAtlas.iPageHeight * 4;
	pPage->pPixels = (unsigned char*)xrtMalloc((size_t)iPixelSize);
	if ( pPage->pPixels == NULL ) {
		return NULL;
	}
	memset(pPage->pPixels, 0, (size_t)iPixelSize);
	pPage->bDirty = 1;
	pFont->tAtlas.iPageCount = iNewCount;
	return pPage;
}

static xge_glyph_atlas_page_t* __xgeFontAtlasPlace(xge_font pFont, int iWidth, int iHeight, int* pPageIndex, int* pX, int* pY)
{
	xge_glyph_atlas_page_t* pPages;
	xge_glyph_atlas_page_t* pPage;
	int i;

	if ( (iWidth > pFont->tAtlas.iPageWidth) || (iHeight > pFont->tAtlas.iPageHeight) ) {
		return NULL;
	}
	pPages = (xge_glyph_atlas_page_t*)pFont->tAtlas.pPages;
	for ( i = 0; i < pFont->tAtlas.iPageCount; i++ ) {
		pPage = &pPages[i];
		if ( pPage->iCursorX + iWidth > pFont->tAtlas.iPageWidth ) {
			pPage->iCursorX = 0;
			pPage->iCursorY += pPage->iRowHeight + 1;
			pPage->iRowHeight = 0;
		}
		if ( pPage->iCursorY + iHeight <= pFont->tAtlas.iPageHeight ) {
			*pPageIndex = i;
			*pX = pPage->iCursorX;
			*pY = pPage->iCursorY;
			pPage->iCursorX += iWidth + 1;
			if ( iHeight > pPage->iRowHeight ) {
				pPage->iRowHeight = iHeight;
			}
			return pPage;
		}
	}
	pPage = __xgeFontAtlasAddPage(pFont);
	if ( pPage == NULL ) {
		return NULL;
	}
	*pPageIndex = pFont->tAtlas.iPageCount - 1;
	*pX = 0;
	*pY = 0;
	pPage->iCursorX = iWidth + 1;
	pPage->iCursorY = 0;
	pPage->iRowHeight = iHeight;
	return pPage;
}

static void __xgeFontAtlasBlit(xge_font pFont, xge_glyph_atlas_page_t* pPage, int iX, int iY, const xge_glyph_bitmap_t* pBitmap)
{
	unsigned char* pDst;
	unsigned char* pSrc;
	int iRow;
	int iCol;
	int iDstPos;

	for ( iRow = 0; iRow < pBitmap->iHeight; iRow++ ) {
		for ( iCol = 0; iCol < pBitmap->iWidth; iCol++ ) {
			pSrc = ((unsigned char*)pBitmap->pPixels) + iRow * pBitmap->iStride + iCol;
			iDstPos = (((iY + iRow) * pFont->tAtlas.iPageWidth) + (iX + iCol)) * 4;
			pDst = pPage->pPixels + iDstPos;
			pDst[0] = 255;
			pDst[1] = 255;
			pDst[2] = 255;
			pDst[3] = *pSrc;
		}
	}
	pPage->bDirty = 1;
}

static int __xgeFontAtlasUploadPage(xge_font pFont, int iPage)
{
	xge_glyph_atlas_page_t* pPages;
	xge_glyph_atlas_page_t* pPage;

	if ( (pFont == NULL) || (iPage < 0) || (iPage >= pFont->tAtlas.iPageCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	pPages = (xge_glyph_atlas_page_t*)pFont->tAtlas.pPages;
	pPage = &pPages[iPage];
	if ( pPage->tTexture.iBackendId == 0 ) {
		if ( xgeTextureCreateRGBA(&pPage->tTexture, pFont->tAtlas.iPageWidth, pFont->tAtlas.iPageHeight, pPage->pPixels) != XGE_OK ) {
			return XGE_ERROR_GPU_FAILED;
		}
		pPage->bDirty = 0;
		return XGE_OK;
	}
	if ( pPage->bDirty ) {
		xgeTextureFree(&pPage->tTexture);
		if ( xgeTextureCreateRGBA(&pPage->tTexture, pFont->tAtlas.iPageWidth, pFont->tAtlas.iPageHeight, pPage->pPixels) != XGE_OK ) {
			return XGE_ERROR_GPU_FAILED;
		}
		pPage->bDirty = 0;
	}
	return XGE_OK;
}

int xgeTextUTF8Next(const char** psText, uint32_t* pCodepoint)
{
	const unsigned char* pText;
	uint32_t iCodepoint;
	int iCount;

	if ( (psText == NULL) || (*psText == NULL) || (pCodepoint == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pText = (const unsigned char*)*psText;
	if ( pText[0] == 0 ) {
		return XGE_ERROR;
	}
	if ( pText[0] < 0x80 ) {
		iCodepoint = pText[0];
		iCount = 1;
	} else if ( (pText[0] & 0xE0) == 0xC0 ) {
		if ( (pText[1] & 0xC0) != 0x80 ) {
			return XGE_ERROR_UNSUPPORTED;
		}
		iCodepoint = ((uint32_t)(pText[0] & 0x1F) << 6) | (uint32_t)(pText[1] & 0x3F);
		iCount = 2;
		if ( iCodepoint < 0x80 ) {
			return XGE_ERROR_UNSUPPORTED;
		}
	} else if ( (pText[0] & 0xF0) == 0xE0 ) {
		if ( ((pText[1] & 0xC0) != 0x80) || ((pText[2] & 0xC0) != 0x80) ) {
			return XGE_ERROR_UNSUPPORTED;
		}
		iCodepoint = ((uint32_t)(pText[0] & 0x0F) << 12) | ((uint32_t)(pText[1] & 0x3F) << 6) | (uint32_t)(pText[2] & 0x3F);
		iCount = 3;
		if ( iCodepoint < 0x800 ) {
			return XGE_ERROR_UNSUPPORTED;
		}
	} else {
		return XGE_ERROR_UNSUPPORTED;
	}
	if ( (iCodepoint >= 0xD800 && iCodepoint <= 0xDFFF) || (iCodepoint > 0xFFFF) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	*pCodepoint = iCodepoint;
	*psText += iCount;
	return XGE_OK;
}

int xgeFontLoad(xge_font pFont, const char* sPath, float fSize)
{
	xge_resource_t tResource;
	int iRet;

	if ( (pFont == NULL) || (sPath == NULL) || (fSize <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = xgeResourceLoad(sPath, &tResource);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeFontLoadMemory(pFont, tResource.pData, tResource.iSize, fSize);
	xgeResourceFree(&tResource);
	return iRet;
}

int xgeFontLoadMemory(xge_font pFont, const void* pData, int iSize, float fSize)
{
	stbtt_fontinfo* pInfo;
	void* pCopy;
	int iOffset;
	int iAscent;
	int iDescent;
	int iLineGap;

	if ( (pFont == NULL) || (pData == NULL) || (iSize <= 0) || (fSize <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pFont, 0, sizeof(*pFont));
	pCopy = xrtMalloc((size_t)iSize);
	if ( pCopy == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(pCopy, pData, (size_t)iSize);
	pInfo = (stbtt_fontinfo*)xrtMalloc(sizeof(*pInfo));
	if ( pInfo == NULL ) {
		xrtFree(pCopy);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iOffset = stbtt_GetFontOffsetForIndex((const unsigned char*)pCopy, 0);
	if ( (iOffset < 0) || (stbtt_InitFont(pInfo, (const unsigned char*)pCopy, iOffset) == 0) ) {
		xrtFree(pInfo);
		xrtFree(pCopy);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	stbtt_GetFontVMetrics(pInfo, &iAscent, &iDescent, &iLineGap);
	pFont->iRefCount = 1;
	pFont->fSize = fSize;
	pFont->fScale = stbtt_ScaleForPixelHeight(pInfo, fSize);
	pFont->fAscent = (float)iAscent * pFont->fScale;
	pFont->fDescent = (float)iDescent * pFont->fScale;
	pFont->fLineGap = (float)iLineGap * pFont->fScale;
	pFont->fLineHeight = pFont->fAscent - pFont->fDescent + pFont->fLineGap;
	pFont->pData = pCopy;
	pFont->iDataSize = iSize;
	pFont->pBackend = pInfo;
	pFont->tAtlas.iPageWidth = 512;
	pFont->tAtlas.iPageHeight = 512;
	pFont->tAtlas.iFormat = XGE_PIXEL_RGBA8;
	return XGE_OK;
}

int xgeFontAddRef(xge_font pFont)
{
	if ( pFont == NULL ) {
		return 0;
	}
	if ( pFont->iRefCount < INT32_MAX ) {
		pFont->iRefCount++;
	}
	return pFont->iRefCount;
}

void xgeFontFree(xge_font pFont)
{
	if ( pFont == NULL ) {
		return;
	}
	if ( pFont->iRefCount > 1 ) {
		pFont->iRefCount--;
		return;
	}
	if ( pFont->pBackend != NULL ) {
		xrtFree(pFont->pBackend);
	}
	if ( pFont->pData != NULL ) {
		xrtFree(pFont->pData);
	}
	__xgeFontAtlasFree(pFont);
	memset(pFont, 0, sizeof(*pFont));
}

void xgeFontSetFallback(xge_font pFont, xge_font pFallback)
{
	if ( pFont != NULL ) {
		pFont->pFallback = pFallback;
	}
}

int xgeFontGlyphGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_metrics_t* pMetrics)
{
	stbtt_fontinfo* pInfo;
	int iGlyph;
	int iAdvance;
	int iBearing;
	int iX0;
	int iY0;
	int iX1;
	int iY1;

	if ( (pFont == NULL) || (pMetrics == NULL) || (iCodepoint > 0xFFFF) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pInfo = __xgeFontInfo(pFont);
	if ( pInfo == NULL ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	iGlyph = stbtt_FindGlyphIndex(pInfo, (int)iCodepoint);
	if ( (iGlyph == 0) && (pFont->pFallback != NULL) ) {
		return xgeFontGlyphGet(pFont->pFallback, iCodepoint, pMetrics);
	}
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iCodepoint = iCodepoint;
	pMetrics->iGlyph = iGlyph;
	stbtt_GetGlyphHMetrics(pInfo, iGlyph, &iAdvance, &iBearing);
	stbtt_GetGlyphBitmapBox(pInfo, iGlyph, pFont->fScale, pFont->fScale, &iX0, &iY0, &iX1, &iY1);
	pMetrics->fAdvanceX = (float)iAdvance * pFont->fScale;
	pMetrics->fLeftSideBearing = (float)iBearing * pFont->fScale;
	pMetrics->fX0 = (float)iX0;
	pMetrics->fY0 = (float)iY0;
	pMetrics->fX1 = (float)iX1;
	pMetrics->fY1 = (float)iY1;
	return XGE_OK;
}

int xgeFontGlyphRasterize(xge_font pFont, uint32_t iCodepoint, xge_glyph_bitmap_t* pBitmap)
{
	stbtt_fontinfo* pInfo;
	unsigned char* pPixels;
	int iGlyph;
	int iWidth;
	int iHeight;
	int iOffsetX;
	int iOffsetY;

	if ( (pFont == NULL) || (pBitmap == NULL) || (iCodepoint > 0xFFFF) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pInfo = __xgeFontInfo(pFont);
	if ( pInfo == NULL ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	iGlyph = stbtt_FindGlyphIndex(pInfo, (int)iCodepoint);
	if ( (iGlyph == 0) && (pFont->pFallback != NULL) ) {
		return xgeFontGlyphRasterize(pFont->pFallback, iCodepoint, pBitmap);
	}
	memset(pBitmap, 0, sizeof(*pBitmap));
	pPixels = stbtt_GetCodepointBitmap(pInfo, pFont->fScale, pFont->fScale, (int)iCodepoint, &iWidth, &iHeight, &iOffsetX, &iOffsetY);
	if ( (pPixels == NULL) && (iWidth > 0) && (iHeight > 0) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pBitmap->iCodepoint = iCodepoint;
	pBitmap->iWidth = iWidth;
	pBitmap->iHeight = iHeight;
	pBitmap->iStride = iWidth;
	pBitmap->iOffsetX = iOffsetX;
	pBitmap->iOffsetY = iOffsetY;
	pBitmap->iFormat = XGE_PIXEL_A8;
	pBitmap->pPixels = pPixels;
	return XGE_OK;
}

int xgeFontGlyphAtlasGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_t* pGlyph)
{
	xge_glyph_cache_t* pCache;
	xge_glyph_bitmap_t tBitmap;
	xge_glyph_metrics_t tMetrics;
	xge_glyph_atlas_page_t* pPage;
	int iPage;
	int iX;
	int iY;
	int iRet;

	if ( (pFont == NULL) || (pGlyph == NULL) || (iCodepoint > 0xFFFF) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pCache = __xgeFontGlyphFind(pFont, iCodepoint);
	if ( pCache != NULL ) {
		*pGlyph = pCache->tGlyph;
		return XGE_OK;
	}
	iRet = xgeFontGlyphGet(pFont, iCodepoint, &tMetrics);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeFontGlyphRasterize(pFont, iCodepoint, &tBitmap);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pCache = (xge_glyph_cache_t*)xrtMalloc(sizeof(*pCache));
	if ( pCache == NULL ) {
		xgeGlyphBitmapFree(&tBitmap);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pCache, 0, sizeof(*pCache));
	pCache->tGlyph.iCodepoint = iCodepoint;
	pCache->tGlyph.iGlyph = tMetrics.iGlyph;
	pCache->tGlyph.fAdvanceX = tMetrics.fAdvanceX;
	pCache->tGlyph.fOffsetX = (float)tBitmap.iOffsetX;
	pCache->tGlyph.fOffsetY = (float)tBitmap.iOffsetY;
	pCache->tGlyph.iWidth = tBitmap.iWidth;
	pCache->tGlyph.iHeight = tBitmap.iHeight;
	pCache->tGlyph.iPage = -1;
	if ( (tBitmap.iWidth > 0) && (tBitmap.iHeight > 0) ) {
		pPage = __xgeFontAtlasPlace(pFont, tBitmap.iWidth, tBitmap.iHeight, &iPage, &iX, &iY);
		if ( pPage == NULL ) {
			xrtFree(pCache);
			xgeGlyphBitmapFree(&tBitmap);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		__xgeFontAtlasBlit(pFont, pPage, iX, iY, &tBitmap);
		pCache->tGlyph.iPage = iPage;
		pCache->tGlyph.iX = iX;
		pCache->tGlyph.iY = iY;
	}
	pCache->pNext = (xge_glyph_cache_t*)pFont->pGlyphs;
	pFont->pGlyphs = pCache;
	*pGlyph = pCache->tGlyph;
	xgeGlyphBitmapFree(&tBitmap);
	return XGE_OK;
}

void xgeGlyphBitmapFree(xge_glyph_bitmap_t* pBitmap)
{
	if ( pBitmap == NULL ) {
		return;
	}
	if ( pBitmap->pPixels != NULL ) {
		xrtFree(pBitmap->pPixels);
	}
	memset(pBitmap, 0, sizeof(*pBitmap));
}

xge_vec2_t xgeTextMeasure(xge_font pFont, const char* sText)
{
	xge_vec2_t tSize;
	xge_glyph_metrics_t tMetrics;
	const char* sScan;
	uint32_t iCodepoint;
	float fLineWidth;
	int iRet;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( (pFont == NULL) || (sText == NULL) ) {
		return tSize;
	}
	sScan = sText;
	fLineWidth = 0.0f;
	tSize.fY = pFont->fLineHeight;
	while ( *sScan != 0 ) {
		iRet = xgeTextUTF8Next(&sScan, &iCodepoint);
		if ( iRet != XGE_OK ) {
			break;
		}
		if ( iCodepoint == '\n' ) {
			if ( fLineWidth > tSize.fX ) {
				tSize.fX = fLineWidth;
			}
			fLineWidth = 0.0f;
			tSize.fY += pFont->fLineHeight;
			continue;
		}
		if ( xgeFontGlyphGet(pFont, iCodepoint, &tMetrics) == XGE_OK ) {
			fLineWidth += tMetrics.fAdvanceX;
		}
	}
	if ( fLineWidth > tSize.fX ) {
		tSize.fX = fLineWidth;
	}
	return tSize;
}

static float __xgeTextLineMeasure(xge_font pFont, const char* sText, int iSize)
{
	xge_glyph_metrics_t tMetrics;
	const char* sScan;
	const char* sEnd;
	uint32_t iCodepoint;
	float fWidth;

	if ( (pFont == NULL) || (sText == NULL) || (iSize <= 0) ) {
		return 0.0f;
	}
	sScan = sText;
	sEnd = sText + iSize;
	fWidth = 0.0f;
	while ( sScan < sEnd ) {
		if ( xgeTextUTF8Next(&sScan, &iCodepoint) != XGE_OK ) {
			break;
		}
		if ( iCodepoint == '\n' ) {
			break;
		}
		if ( xgeFontGlyphGet(pFont, iCodepoint, &tMetrics) == XGE_OK ) {
			fWidth += tMetrics.fAdvanceX;
		}
	}
	return fWidth;
}

static const char* __xgeTextLineEnd(const char* sText)
{
	const char* sScan;

	sScan = sText;
	while ( (*sScan != 0) && (*sScan != '\n') ) {
		sScan++;
	}
	return sScan;
}

static void __xgeTextDrawRange(xge_font pFont, const char* sText, int iSize, float fX, float fY, uint32_t iColor)
{
	const char* sScan;
	const char* sEnd;
	uint32_t iCodepoint;
	xge_glyph_t tGlyph;
	xge_draw_t tDraw;
	xge_glyph_atlas_page_t* pPages;
	float fPenX;
	float fPenY;
	int iRet;

	if ( (pFont == NULL) || (sText == NULL) || (iSize <= 0) ) {
		return;
	}
	pPages = (xge_glyph_atlas_page_t*)pFont->tAtlas.pPages;
	sScan = sText;
	sEnd = sText + iSize;
	fPenX = fX;
	fPenY = fY + pFont->fAscent;
	while ( sScan < sEnd ) {
		iRet = xgeTextUTF8Next(&sScan, &iCodepoint);
		if ( iRet != XGE_OK ) {
			break;
		}
		if ( iCodepoint == '\n' ) {
			fPenX = fX;
			fPenY += pFont->fLineHeight;
			continue;
		}
		if ( xgeFontGlyphAtlasGet(pFont, iCodepoint, &tGlyph) != XGE_OK ) {
			continue;
		}
		if ( (tGlyph.iPage >= 0) && (tGlyph.iWidth > 0) && (tGlyph.iHeight > 0) ) {
			pPages = (xge_glyph_atlas_page_t*)pFont->tAtlas.pPages;
			if ( __xgeFontAtlasUploadPage(pFont, tGlyph.iPage) == XGE_OK ) {
				memset(&tDraw, 0, sizeof(tDraw));
				tDraw.pTexture = &pPages[tGlyph.iPage].tTexture;
				tDraw.tSrc.fX = (float)tGlyph.iX;
				tDraw.tSrc.fY = (float)tGlyph.iY;
				tDraw.tSrc.fW = (float)tGlyph.iWidth;
				tDraw.tSrc.fH = (float)tGlyph.iHeight;
				tDraw.tDst.fX = fPenX + tGlyph.fOffsetX;
				tDraw.tDst.fY = fPenY + tGlyph.fOffsetY;
				tDraw.tDst.fW = (float)tGlyph.iWidth;
				tDraw.tDst.fH = (float)tGlyph.iHeight;
				tDraw.iColor = iColor;
				xgeDrawEx(&tDraw);
			}
		}
		fPenX += tGlyph.fAdvanceX;
	}
}

void xgeTextDraw(xge_font pFont, const char* sText, float fX, float fY, uint32_t iColor)
{
	const char* sLine;
	const char* sEnd;
	float fPenY;

	if ( (pFont == NULL) || (sText == NULL) ) {
		return;
	}
	sLine = sText;
	fPenY = fY;
	while ( *sLine != 0 ) {
		sEnd = __xgeTextLineEnd(sLine);
		__xgeTextDrawRange(pFont, sLine, (int)(sEnd - sLine), fX, fPenY, iColor);
		if ( *sEnd == '\n' ) {
			sLine = sEnd + 1;
			fPenY += pFont->fLineHeight;
		} else {
			break;
		}
	}
}

void xgeTextDrawRect(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	const char* sLine;
	const char* sEnd;
	xge_vec2_t tSize;
	float fPenY;
	float fLineX;
	float fLineWidth;
	int iLineSize;
	int bClip;

	if ( (pFont == NULL) || (sText == NULL) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return;
	}
	tSize = xgeTextMeasure(pFont, sText);
	fPenY = tRect.fY;
	if ( (iFlags & XGE_TEXT_ALIGN_BOTTOM) == XGE_TEXT_ALIGN_BOTTOM ) {
		fPenY = tRect.fY + tRect.fH - tSize.fY;
	} else if ( (iFlags & XGE_TEXT_ALIGN_MIDDLE) == XGE_TEXT_ALIGN_MIDDLE ) {
		fPenY = tRect.fY + (tRect.fH - tSize.fY) * 0.5f;
	}
	bClip = ((iFlags & XGE_TEXT_CLIP) != 0) && (glScissor != NULL);
	if ( bClip ) {
		glEnable(GL_SCISSOR_TEST);
		glScissor((GLint)tRect.fX, (GLint)((float)g_xge.iHeight - tRect.fY - tRect.fH), (GLsizei)tRect.fW, (GLsizei)tRect.fH);
	}
	sLine = sText;
	while ( *sLine != 0 ) {
		sEnd = __xgeTextLineEnd(sLine);
		iLineSize = (int)(sEnd - sLine);
		fLineWidth = __xgeTextLineMeasure(pFont, sLine, iLineSize);
		fLineX = tRect.fX;
		if ( (iFlags & XGE_TEXT_ALIGN_RIGHT) == XGE_TEXT_ALIGN_RIGHT ) {
			fLineX = tRect.fX + tRect.fW - fLineWidth;
		} else if ( (iFlags & XGE_TEXT_ALIGN_CENTER) == XGE_TEXT_ALIGN_CENTER ) {
			fLineX = tRect.fX + (tRect.fW - fLineWidth) * 0.5f;
		}
		__xgeTextDrawRange(pFont, sLine, iLineSize, fLineX, fPenY, iColor);
		if ( *sEnd == '\n' ) {
			sLine = sEnd + 1;
			fPenY += pFont->fLineHeight;
		} else {
			break;
		}
	}
	if ( bClip ) {
		glDisable(GL_SCISSOR_TEST);
	}
}
#else
int xgeTextUTF8Next(const char** psText, uint32_t* pCodepoint) { (void)psText; (void)pCodepoint; return XGE_ERROR_UNSUPPORTED; }
int xgeFontLoad(xge_font pFont, const char* sPath, float fSize) { (void)pFont; (void)sPath; (void)fSize; return XGE_ERROR_UNSUPPORTED; }
int xgeFontLoadMemory(xge_font pFont, const void* pData, int iSize, float fSize) { (void)pFont; (void)pData; (void)iSize; (void)fSize; return XGE_ERROR_UNSUPPORTED; }
int xgeFontAddRef(xge_font pFont) { (void)pFont; return 0; }
void xgeFontFree(xge_font pFont) { (void)pFont; }
void xgeFontSetFallback(xge_font pFont, xge_font pFallback) { (void)pFont; (void)pFallback; }
int xgeFontGlyphGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_metrics_t* pMetrics) { (void)pFont; (void)iCodepoint; (void)pMetrics; return XGE_ERROR_UNSUPPORTED; }
int xgeFontGlyphRasterize(xge_font pFont, uint32_t iCodepoint, xge_glyph_bitmap_t* pBitmap) { (void)pFont; (void)iCodepoint; (void)pBitmap; return XGE_ERROR_UNSUPPORTED; }
int xgeFontGlyphAtlasGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_t* pGlyph) { (void)pFont; (void)iCodepoint; (void)pGlyph; return XGE_ERROR_UNSUPPORTED; }
void xgeGlyphBitmapFree(xge_glyph_bitmap_t* pBitmap) { (void)pBitmap; }
xge_vec2_t xgeTextMeasure(xge_font pFont, const char* sText) { xge_vec2_t tSize; (void)pFont; (void)sText; tSize.fX = 0.0f; tSize.fY = 0.0f; return tSize; }
void xgeTextDraw(xge_font pFont, const char* sText, float fX, float fY, uint32_t iColor) { (void)pFont; (void)sText; (void)fX; (void)fY; (void)iColor; }
void xgeTextDrawRect(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags) { (void)pFont; (void)sText; (void)tRect; (void)iColor; (void)iFlags; }
#endif

int xgeTextureCreateRGBA(xge_texture pTexture, int iWidth, int iHeight, const void* pPixels)
{
	GLuint iTexture;

	if ( (pTexture == NULL) || (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}

	memset(pTexture, 0, sizeof(*pTexture));
	glGenTextures(1, &iTexture);
	glBindTexture(GL_TEXTURE_2D, iTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, iWidth, iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pPixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	pTexture->iWidth = iWidth;
	pTexture->iHeight = iHeight;
	pTexture->iFormat = XGE_PIXEL_RGBA8;
	pTexture->iRefCount = 1;
	pTexture->iBackendId = iTexture;
	return XGE_OK;
}

int xgeImageLoad(xge_image pImage, const char* sPath)
{
	return xgeImageLoadEx(pImage, sPath, XGE_IMAGE_PREMULTIPLIED);
}

int xgeImageLoadEx(xge_image pImage, const char* sPath, uint32_t iFlags)
{
	xge_resource_t tResource;
	int iRet;

	if ( (pImage == NULL) || (sPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}

	iRet = xgeResourceLoad(sPath, &tResource);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeImageLoadMemoryEx(pImage, tResource.pData, tResource.iSize, iFlags);
	xgeResourceFree(&tResource);
	return iRet;
}

int xgeImageLoadMemory(xge_image pImage, const void* pData, int iSize)
{
	return xgeImageLoadMemoryEx(pImage, pData, iSize, XGE_IMAGE_PREMULTIPLIED);
}

int xgeImageLoadMemoryEx(xge_image pImage, const void* pData, int iSize, uint32_t iFlags)
{
	int iWidth;
	int iHeight;
	int iChannels;
	unsigned char* pPixels;

	if ( (pImage == NULL) || (pData == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}

	pPixels = stbi_load_from_memory((const stbi_uc*)pData, iSize, &iWidth, &iHeight, &iChannels, 4);
	if ( pPixels == NULL ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	memset(pImage, 0, sizeof(*pImage));
	pImage->iWidth = iWidth;
	pImage->iHeight = iHeight;
	pImage->iFormat = XGE_PIXEL_RGBA8;
	pImage->iStride = iWidth * 4;
	pImage->pPixels = pPixels;
	pImage->iFlags = ((iFlags & XGE_IMAGE_STRAIGHT_ALPHA) != 0) ? XGE_IMAGE_STRAIGHT_ALPHA : XGE_IMAGE_PREMULTIPLIED;
	if ( (pImage->iFlags & XGE_IMAGE_PREMULTIPLIED) != 0 ) {
		xgeImagePremultiply(pImage);
	}
	return XGE_OK;
}

void* xgeImageGetPixels(xge_image pImage)
{
	if ( pImage == NULL ) {
		return NULL;
	}
	return pImage->pPixels;
}

void xgeImagePremultiply(xge_image pImage)
{
	unsigned char* pPixels;
	int i;
	int iCount;
	unsigned int iA;

	if ( (pImage == NULL) || (pImage->pPixels == NULL) || (pImage->iFormat != XGE_PIXEL_RGBA8) ) {
		return;
	}
	pPixels = (unsigned char*)pImage->pPixels;
	iCount = pImage->iWidth * pImage->iHeight;
	for ( i = 0; i < iCount; i++ ) {
		iA = (unsigned int)pPixels[(i * 4) + 3];
		pPixels[(i * 4) + 0] = (unsigned char)(((unsigned int)pPixels[(i * 4) + 0] * iA + 127u) / 255u);
		pPixels[(i * 4) + 1] = (unsigned char)(((unsigned int)pPixels[(i * 4) + 1] * iA + 127u) / 255u);
		pPixels[(i * 4) + 2] = (unsigned char)(((unsigned int)pPixels[(i * 4) + 2] * iA + 127u) / 255u);
	}
	pImage->iFlags &= ~XGE_IMAGE_STRAIGHT_ALPHA;
	pImage->iFlags |= XGE_IMAGE_PREMULTIPLIED;
}

void xgeImageFree(xge_image pImage)
{
	if ( pImage == NULL ) {
		return;
	}
	if ( pImage->pPixels != NULL ) {
		stbi_image_free(pImage->pPixels);
	}
	memset(pImage, 0, sizeof(*pImage));
}

int xgeTextureCreateFromImage(xge_texture pTexture, const xge_image_t* pImage)
{
	if ( (pTexture == NULL) || (pImage == NULL) || (pImage->pPixels == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return xgeTextureCreateRGBA(pTexture, pImage->iWidth, pImage->iHeight, pImage->pPixels);
}

int xgeTextureLoad(xge_texture pTexture, const char* sPath)
{
	return xgeTextureLoadEx(pTexture, sPath, XGE_IMAGE_PREMULTIPLIED);
}

int xgeTextureLoadEx(xge_texture pTexture, const char* sPath, uint32_t iFlags)
{
	xge_image_t objImage;
	int iRet;

	iRet = xgeImageLoadEx(&objImage, sPath, iFlags);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeTextureCreateFromImage(pTexture, &objImage);
	xgeImageFree(&objImage);
	return iRet;
}

int xgeTextureLoadMemory(xge_texture pTexture, const void* pData, int iSize)
{
	return xgeTextureLoadMemoryEx(pTexture, pData, iSize, XGE_IMAGE_PREMULTIPLIED);
}

int xgeTextureLoadMemoryEx(xge_texture pTexture, const void* pData, int iSize, uint32_t iFlags)
{
	xge_image_t objImage;
	int iRet;

	iRet = xgeImageLoadMemoryEx(&objImage, pData, iSize, iFlags);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeTextureCreateFromImage(pTexture, &objImage);
	xgeImageFree(&objImage);
	return iRet;
}

int xgeTextureAddRef(xge_texture pTexture)
{
	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pTexture->iRefCount < INT32_MAX ) {
		pTexture->iRefCount++;
	}
	return pTexture->iRefCount;
}

void xgeTextureFree(xge_texture pTexture)
{
	GLuint iTexture;

	if ( pTexture == NULL ) {
		return;
	}
	if ( pTexture->iRefCount > 1 ) {
		pTexture->iRefCount--;
		return;
	}
	iTexture = (GLuint)pTexture->iBackendId;
	if ( iTexture != 0 ) {
		glDeleteTextures(1, &iTexture);
	}
	memset(pTexture, 0, sizeof(*pTexture));
}

void xgeDraw(xge_texture pTexture, float fX, float fY)
{
	xge_draw_t tDraw;

	if ( pTexture == NULL ) {
		return;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tDst.fX = fX;
	tDraw.tDst.fY = fY;
	tDraw.tDst.fW = (float)pTexture->iWidth;
	tDraw.tDst.fH = (float)pTexture->iHeight;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	xgeDrawEx(&tDraw);
}

void xgeDrawPx(xge_texture pTexture, int iX, int iY)
{
	xge_draw_t tDraw;

	if ( pTexture == NULL ) {
		return;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tDst.fX = (float)iX;
	tDraw.tDst.fY = (float)iY;
	tDraw.tDst.fW = (float)pTexture->iWidth;
	tDraw.tDst.fH = (float)pTexture->iHeight;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
}

void xgeDrawEx(const xge_draw_t* pDraw)
{
	float fSrcX;
	float fSrcY;
	float fSrcW;
	float fSrcH;
	float fU0;
	float fV0;
	float fU1;
	float fV1;
	float fLocalX[4];
	float fLocalY[4];
	float fCos;
	float fSin;
	float fR;
	float fG;
	float fB;
	float fA;
	float arrVertices[16];
	int i;
	xge_texture pTexture;
	xge_vec2_t tScreen;

	if ( (pDraw == NULL) || (pDraw->pTexture == NULL) || (pDraw->pTexture->iBackendId == 0) ) {
		return;
	}
	pTexture = pDraw->pTexture;
	if ( __xgeTextureRendererInit() != XGE_OK ) {
		return;
	}

	fSrcX = pDraw->tSrc.fX;
	fSrcY = pDraw->tSrc.fY;
	fSrcW = pDraw->tSrc.fW;
	fSrcH = pDraw->tSrc.fH;
	if ( fSrcW == 0.0f ) {
		fSrcW = (float)pTexture->iWidth;
	}
	if ( fSrcH == 0.0f ) {
		fSrcH = (float)pTexture->iHeight;
	}
	fU0 = fSrcX / (float)pTexture->iWidth;
	fV0 = fSrcY / (float)pTexture->iHeight;
	fU1 = (fSrcX + fSrcW) / (float)pTexture->iWidth;
	fV1 = (fSrcY + fSrcH) / (float)pTexture->iHeight;
	if ( (pDraw->iFlags & XGE_DRAW_FLIP_X) != 0 ) {
		float fTmp = fU0;
		fU0 = fU1;
		fU1 = fTmp;
	}
	if ( (pDraw->iFlags & XGE_DRAW_FLIP_Y) != 0 ) {
		float fTmp = fV0;
		fV0 = fV1;
		fV1 = fTmp;
	}

	fLocalX[0] = -pDraw->tOrigin.fX;
	fLocalY[0] = -pDraw->tOrigin.fY;
	fLocalX[1] = pDraw->tDst.fW - pDraw->tOrigin.fX;
	fLocalY[1] = -pDraw->tOrigin.fY;
	fLocalX[2] = -pDraw->tOrigin.fX;
	fLocalY[2] = pDraw->tDst.fH - pDraw->tOrigin.fY;
	fLocalX[3] = pDraw->tDst.fW - pDraw->tOrigin.fX;
	fLocalY[3] = pDraw->tDst.fH - pDraw->tOrigin.fY;
	fCos = cosf(pDraw->fRotation);
	fSin = sinf(pDraw->fRotation);

	for ( i = 0; i < 4; i++ ) {
		xge_vec2_t tWorld;
		tWorld.fX = pDraw->tDst.fX + (fLocalX[i] * fCos) - (fLocalY[i] * fSin);
		tWorld.fY = pDraw->tDst.fY + (fLocalX[i] * fSin) + (fLocalY[i] * fCos);
		if ( (pDraw->iFlags & XGE_DRAW_SCREEN_SPACE) != 0 ) {
			tScreen = tWorld;
		} else {
			tScreen = xgeWorldToScreen(tWorld);
		}
		arrVertices[(i * 4) + 0] = tScreen.fX;
		arrVertices[(i * 4) + 1] = tScreen.fY;
	}
	arrVertices[2] = fU0; arrVertices[3] = fV0;
	arrVertices[6] = fU1; arrVertices[7] = fV0;
	arrVertices[10] = fU0; arrVertices[11] = fV1;
	arrVertices[14] = fU1; arrVertices[15] = fV1;

	__xgeColorToFloat(pDraw->iColor, &fR, &fG, &fB, &fA);

	glUseProgram(g_xgeTextureRenderer.iProgram);
	glUniform2f(g_xgeTextureRenderer.iLocResolution, (float)g_xge.iWidth, (float)g_xge.iHeight);
	glUniform1i(g_xgeTextureRenderer.iLocTexture, 0);
	glUniform4f(g_xgeTextureRenderer.iLocColor, fR, fG, fB, fA);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, (GLuint)pTexture->iBackendId);
	glBindVertexArray(g_xgeTextureRenderer.iVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_xgeTextureRenderer.iVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(arrVertices), arrVertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

int xgeKeyDown(int iKey)
{
	if ( (iKey < 0) || (iKey >= XGE_KEY_COUNT) ) {
		return 0;
	}
	return g_xge.arrKeyDown[iKey] ? 1 : 0;
}

int xgeKeyPressed(int iKey)
{
	if ( (iKey < 0) || (iKey >= XGE_KEY_COUNT) ) {
		return 0;
	}
	return g_xge.arrKeyPressed[iKey] ? 1 : 0;
}

int xgeKeyReleased(int iKey)
{
	if ( (iKey < 0) || (iKey >= XGE_KEY_COUNT) ) {
		return 0;
	}
	return g_xge.arrKeyReleased[iKey] ? 1 : 0;
}

void xgeMouseGet(float* pX, float* pY)
{
	if ( pX != NULL ) {
		*pX = g_xge.fMouseX;
	}
	if ( pY != NULL ) {
		*pY = g_xge.fMouseY;
	}
}

void xgeMouseGetDelta(float* pDX, float* pDY)
{
	if ( pDX != NULL ) {
		*pDX = g_xge.fMouseDX;
	}
	if ( pDY != NULL ) {
		*pDY = g_xge.fMouseDY;
	}
}

void xgeMouseGetWheel(float* pX, float* pY)
{
	if ( pX != NULL ) {
		*pX = g_xge.fMouseWheelX;
	}
	if ( pY != NULL ) {
		*pY = g_xge.fMouseWheelY;
	}
}

int xgeMouseDown(int iButton)
{
	return (g_xge.iMouseButtons & (unsigned int)iButton) ? 1 : 0;
}

uint32_t xgeTextGet(void)
{
	return g_xge.iTextCodepoint;
}

int xgeTouchGetCount(void)
{
	return g_xge.iTouchCount;
}

int xgeTouchGet(int iIndex, xge_touch_point_t* pPoint)
{
	if ( (iIndex < 0) || (iIndex >= g_xge.iTouchCount) || (pPoint == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pPoint = g_xge.arrTouches[iIndex];
	return XGE_OK;
}

int xgeTouchFind(uint64_t iId, xge_touch_point_t* pPoint)
{
	int iIndex;

	if ( pPoint == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xgeTouchFindIndex(iId);
	if ( iIndex < 0 ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	*pPoint = g_xge.arrTouches[iIndex];
	return XGE_OK;
}

#endif /* XGE_IMPL */

#endif /* XGE_H */
