#ifndef XGE_H
#define XGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "lib/xrt/xrt.h"

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

#if !defined(XGE_DEBUGMODE)
	#define XGE_DEBUGMODE	0
#endif

#if XGE_DEBUGMODE
	#define XGE_HAS_DEBUGMODE	1
#else
	#define XGE_HAS_DEBUGMODE	0
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
#define XGE_INIT_ON_DEMAND	0x0040

#define XGE_RUN_GAME_LOOP	0
#define XGE_RUN_MANUAL		1

#define XGE_PLATFORM_BACKEND_NONE	0
#define XGE_PLATFORM_BACKEND_SOKOL	1
#define XGE_PLATFORM_BACKEND_MINIPROGRAM	2
#define XGE_PLATFORM_BACKEND_EGL	3
#define XGE_PLATFORM_BACKEND_CUSTOM	100

#define XGE_GPU_BACKEND_NONE		0
#define XGE_GPU_BACKEND_OPENGL33	1
#define XGE_GPU_BACKEND_GLES30		2
#define XGE_GPU_BACKEND_WEBGL2		3
#define XGE_GPU_BACKEND_CUSTOM		100

#define XGE_UPDATE_VARIABLE	0
#define XGE_UPDATE_FIXED	1

#define XGE_LOG_TRACE	0
#define XGE_LOG_DEBUG	1
#define XGE_LOG_INFO	2
#define XGE_LOG_WARN	3
#define XGE_LOG_ERROR	4
#define XGE_LOG_FATAL	5
#define XGE_LOG_OFF	6

#define XGE_KEY_COUNT		512
#define XGE_KEY_SPACE		32
#define XGE_KEY_ESCAPE		256
#define XGE_KEY_ENTER		257
#define XGE_KEY_TAB			258
#define XGE_KEY_BACKSPACE	259
#define XGE_KEY_DELETE		261
#define XGE_KEY_RIGHT		262
#define XGE_KEY_LEFT		263
#define XGE_KEY_DOWN		264
#define XGE_KEY_UP			265
#define XGE_KEY_PAGE_UP		266
#define XGE_KEY_PAGE_DOWN	267
#define XGE_KEY_HOME		268
#define XGE_KEY_END			269
#define XGE_KEY_F10			299
#define XGE_KEY_MENU		348
#define XGE_TEXT_MAX		32
#define XGE_TOUCH_MAX		8
#define XGE_GAMEPAD_MAX		4
#define XGE_GAMEPAD_BUTTON_COUNT	32
#define XGE_GAMEPAD_AXIS_COUNT	8
#define XGE_MOUSE_LEFT		0x01
#define XGE_MOUSE_RIGHT		0x02
#define XGE_MOUSE_MIDDLE	0x04
#define XGE_DIRTY_RECT_MAX	64
#define XGE_SCENE_STACK_MAX	16

#define XGE_PIXEL_RGBA8		1
#define XGE_PIXEL_A8		2

#define XGE_IMAGE_PREMULTIPLIED		0x0001
#define XGE_IMAGE_STRAIGHT_ALPHA	0x0002

#define XGE_TEXTURE_FALLBACK		0x0001
#define XGE_TEXTURE_UPLOAD_QUEUED	0x0002
#define XGE_FONT_FALLBACK			0x0001
#define XGE_SOUND_FALLBACK			0x80000000u

#define XGE_RENDER_TARGET_WINDOW	0x0001
#define XGE_RENDER_TARGET_TEXTURE	0x0002
#define XGE_PASS_CLEAR_COLOR		0x0001

#define XGE_BUFFER_VERTEX		1
#define XGE_BUFFER_INDEX		2
#define XGE_BUFFER_UNIFORM		3

#define XGE_BUFFER_STATIC		1
#define XGE_BUFFER_DYNAMIC		2

#define XGE_FILTER_NEAREST		1
#define XGE_FILTER_LINEAR		2

#define XGE_WRAP_CLAMP			1
#define XGE_WRAP_REPEAT		2

#define XGE_BLEND_NONE			0
#define XGE_BLEND_ALPHA			1
#define XGE_BLEND_ADD			2
#define XGE_BLEND_MULTIPLY		3
#define XGE_BLEND_SCREEN		4
#define XGE_BLEND_CUSTOM		5

#define XGE_MATERIAL_DEFAULT_BLEND	-1
#define XGE_SHADER_DEFINE_MAX		8
#define XGE_SHADER_DEFINE_NAME_MAX	32

#define XGE_COORD_Y_DOWN	0
#define XGE_COORD_CENTER	1

#define XGE_CAMERA_ORTHO		0
#define XGE_CAMERA_PERSPECTIVE	1

#define XGE_DRAW_FLIP_X		0x0001
#define XGE_DRAW_FLIP_Y		0x0002
#define XGE_DRAW_SCREEN_SPACE	0x0004
#define XGE_NINE_PATCH_STRETCH	0
#define XGE_NINE_PATCH_TILE		1

#define XGE_MESH_DYNAMIC	0x0001

#define XGE_TEXT_ALIGN_LEFT		0x0000
#define XGE_TEXT_ALIGN_CENTER	0x0001
#define XGE_TEXT_ALIGN_RIGHT	0x0002
#define XGE_TEXT_ALIGN_TOP		0x0000
#define XGE_TEXT_ALIGN_MIDDLE	0x0010
#define XGE_TEXT_ALIGN_BOTTOM	0x0020
#define XGE_TEXT_CLIP			0x0100
#define XGE_TEXT_UNDERLINE		0x0200
#define XGE_TEXT_SCREEN_SPACE	0x0400

#define XGE_XRF_MAGIC			0x32465258u
#define XGE_XRF_VERSION			1
#define XGE_XRF_PAGE_A8			1
#define XGE_XRF_PAGE_RGBA8		2
#define XGE_XRF_FLAG_KERNING	0x0001

#define XGE_AUDIO_SOUND		1
#define XGE_AUDIO_MUSIC		2
#define XGE_AUDIO_STREAM	3

#define XGE_ASYNC_PENDING		0
#define XGE_ASYNC_LOADING		1
#define XGE_ASYNC_READY		2
#define XGE_ASYNC_FAILED		3
#define XGE_ASYNC_CANCELLED	4

#define XGE_ASYNC_IMAGE		1
#define XGE_ASYNC_TEXTURE		2
#define XGE_ASYNC_FONT			3
#define XGE_ASYNC_SOUND		4

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
#define XGE_EVENT_IME_START		14
#define XGE_EVENT_IME_UPDATE	15
#define XGE_EVENT_IME_END		16
#define XGE_EVENT_GAMEPAD_CONNECTED	19
#define XGE_EVENT_GAMEPAD_DISCONNECTED	20



#define XGE_KEY_MOD_SHIFT	0x0001
#define XGE_KEY_MOD_CTRL	0x0002
#define XGE_KEY_MOD_ALT		0x0004
#define XGE_KEY_MOD_SUPER	0x0008

#define XGE_GAMEPAD_A				0x00000001u
#define XGE_GAMEPAD_B				0x00000002u
#define XGE_GAMEPAD_X				0x00000004u
#define XGE_GAMEPAD_Y				0x00000008u
#define XGE_GAMEPAD_LEFT_SHOULDER	0x00000010u
#define XGE_GAMEPAD_RIGHT_SHOULDER	0x00000020u
#define XGE_GAMEPAD_BACK			0x00000040u
#define XGE_GAMEPAD_START			0x00000080u
#define XGE_GAMEPAD_LEFT_STICK		0x00000100u
#define XGE_GAMEPAD_RIGHT_STICK		0x00000200u
#define XGE_GAMEPAD_DPAD_UP		0x00000400u
#define XGE_GAMEPAD_DPAD_DOWN		0x00000800u
#define XGE_GAMEPAD_DPAD_LEFT		0x00001000u
#define XGE_GAMEPAD_DPAD_RIGHT		0x00002000u
#define XGE_GAMEPAD_GUIDE			0x00004000u


























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

typedef struct xge_frame_stats_t {
	int iFrameCount;
	int iDrawCallCount;
	int iBatchCount;
	int iDirtyRectCount;
	float fFrameTimeMs;
	float fFrameTimeAvgMs;
	float fFrameTimeMaxMs;
} xge_frame_stats_t;

#if XGE_HAS_DEBUGMODE
typedef struct xge_debug_stats_t {
	xge_frame_stats_t tFrame;
	int iTextureCount;
	uint64_t iTextureMemoryBytes;
	int iFontCount;
	int iAudioCount;
	int iLastGLError;
} xge_debug_stats_t;
#endif

typedef struct xge_platform_backend_t {
	int iType;
	const char* sName;
	int (*init)(void* pUser);
	void (*unit)(void* pUser);
	int (*poll)(void* pUser);
	void (*present)(void* pUser);
	void* pUser;
} xge_platform_backend_t;

typedef struct xge_graphics_backend_t {
	int iType;
	const char* sName;
	int (*init)(void* pUser);
	void (*unit)(void* pUser);
	int (*begin)(void* pUser);
	int (*end)(void* pUser);
	void* pUser;
} xge_graphics_backend_t;

typedef struct xge_gpu_caps_t {
	int iBackend;
	int iMajorVersion;
	int iMinorVersion;
	int iMaxTextureSize;
	char sVendor[64];
	char sRenderer[128];
	char sVersion[64];
	char sShadingLanguage[64];
} xge_gpu_caps_t;

typedef struct xge_graphics_mapping_t {
	int iBackend;
	int bOpenGLCore;
	int bGLES;
	int bWebGL;
	int bVAO;
	int bFramebufferObject;
	int bDepthTexture;
	int bClientSideVertexArray;
	int iRGBA8InternalFormat;
	int iRGBAFormat;
	int iUnsignedByteType;
	char sShaderHeader[128];
} xge_graphics_mapping_t;

typedef struct xge_platform_caps_t {
	int iPlatformBackend;
	int iGraphicsBackend;
	char sPlatformName[64];
	char sGraphicsName[64];
	char sSokolTargetName[64];
	int bWindow;
	int bOffscreen;
	int bRenderTarget;
	int bPBuffer;
	int bSurfaceless;
	int bBoardLinux;
	int bMiniProgram;
	int bWASM;
	int bTouch;
	int bMouse;
	int bKeyboard;
	int bTextInput;
	int bGamepad;
	int bAudio;
	int bResourceProvider;
	int bThreadSafeSubmit;
	int bHighDPI;
	int bSokol;
	int bSokolWindows;
	int bSokolLinuxX11;
	int bSokolLinuxWayland;
	int bSokolMacOS;
	int bSokolAndroid;
	int bSokolIOS;
	int bSokolWeb;
	int bSokolGLCore;
	int bSokolGLES3;
	int bSokolD3D11;
	int bSokolMetal;
	int bSokolDummy;
} xge_platform_caps_t;

typedef struct xge_platform_runtime_t {
	int bRunning;
	int iWindowWidth;
	int iWindowHeight;
	int iFramebufferWidth;
	int iFramebufferHeight;
	float fDpiScale;
	int iKeyEventCount;
	int iTextEventCount;
	int iMouseEventCount;
	int iTouchEventCount;
	int iGamepadEventCount;
	int iResizeEventCount;
	int iQuitEventCount;
} xge_platform_runtime_t;

typedef struct xge_render_thread_caps_t {
	int bSupported;
	int bEnabled;
	int bWorkerDrain;
	int bGLContextOwned;
	int bAsyncFlush;
	int bCanUseWithCurrentContext;
} xge_render_thread_caps_t;

typedef struct xge_camera_t {
	xge_vec2_t tPosition;
	xge_vec2_t tScale;
	float fRotation;
	xge_rect_t tViewport;
	int iCoordinateMode;
	int iProjectionMode;
	float fFovY;
	float fNearZ;
	float fFarZ;
	float fPerspectiveDistance;
} xge_camera_t;

typedef struct xge_texture_t xge_texture_t;
typedef xge_texture_t* xge_texture;
typedef struct xge_render_target_t xge_render_target_t;
typedef xge_render_target_t* xge_render_target;
typedef struct xge_buffer_t xge_buffer_t;
typedef xge_buffer_t* xge_buffer;
typedef struct xge_pass_t xge_pass_t;
typedef xge_pass_t* xge_pass;
typedef struct xge_sprite_batch_t xge_sprite_batch_t;
typedef xge_sprite_batch_t* xge_sprite_batch;
typedef struct xge_shape_batch_t xge_shape_batch_t;
typedef xge_shape_batch_t* xge_shape_batch;
typedef struct xge_shader_t xge_shader_t;
typedef xge_shader_t* xge_shader;
typedef struct xge_material_t xge_material_t;
typedef xge_material_t* xge_material;
typedef struct xge_shader_variant_t xge_shader_variant_t;
typedef xge_shader_variant_t* xge_shader_variant;
typedef struct xge_shader_variant_set_t xge_shader_variant_set_t;
typedef xge_shader_variant_set_t* xge_shader_variant_set;
typedef struct xge_mesh_t xge_mesh_t;
typedef xge_mesh_t* xge_mesh;
typedef struct xge_nine_patch_t xge_nine_patch_t;
typedef xge_nine_patch_t* xge_nine_patch;

typedef struct xge_draw_t {
	xge_texture pTexture;
	xge_rect_t tSrc;
	xge_rect_t tDst;
	xge_vec2_t tOrigin;
	float fRotation;
	uint32_t iColor;
	uint32_t iFlags;
} xge_draw_t;

struct xge_nine_patch_t {
	xge_texture pTexture;
	xge_rect_t tSrc;
	float fX1;
	float fY1;
	float fX2;
	float fY2;
	uint32_t iColor;
	int iMode;
	int bEasyMode;
};

typedef struct xge_vertex_t {
	float fX;
	float fY;
	float fZ;
	float fW;
	float fU;
	float fV;
	uint32_t iColor;
} xge_vertex_t;

typedef struct xge_shape_vertex_t {
	float fX;
	float fY;
	uint32_t iColor;
} xge_shape_vertex_t;

typedef struct xge_sampler_t {
	int iMinFilter;
	int iMagFilter;
	int iWrapS;
	int iWrapT;
} xge_sampler_t;

struct xge_sprite_batch_t {
	xge_texture pTexture;
	int iCapacity;
	int iCount;
	uint32_t iFlags;
	void* pVertices;
};

typedef struct xge_pipeline_state_t {
	int iBlend;
	int bDepthTest;
	uint32_t iFlags;
} xge_pipeline_state_t;

struct xge_shader_t {
	int iRefCount;
	uint32_t iProgram;
	int iLocResolution;
	int iLocTexture;
	int iLocColor;
	void* pBackend;
};

typedef struct xge_shader_define_t {
	char sName[XGE_SHADER_DEFINE_NAME_MAX];
	int iValue;
} xge_shader_define_t;

struct xge_shader_variant_t {
	uint32_t iKey;
	xge_shader_t tShader;
	int iDefineCount;
	xge_shader_define_t arrDefines[XGE_SHADER_DEFINE_MAX];
	struct xge_shader_variant_t* pNext;
};

struct xge_shader_variant_set_t {
	char* sVertexSource;
	char* sFragmentSource;
	xge_shader_variant pVariants;
	int iCount;
};

struct xge_buffer_t {
	int iType;
	int iUsage;
	int iSize;
	uint32_t iBackendId;
	void* pData;
};

struct xge_material_t {
	xge_shader pShader;
	xge_texture pTexture;
	uint32_t iColor;
	xge_pipeline_state_t tPipeline;
};

struct xge_mesh_t {
	int iVertexCount;
	int iIndexCount;
	uint32_t iFlags;
	uint32_t iVertexBufferId;
	uint32_t iIndexBufferId;
	void* pVertices;
	void* pIndices;
};

struct xge_shape_batch_t {
	uint32_t iColor;
	uint32_t iFlags;
	int iTriangleCount;
	int iTriangleCapacity;
	void* pVertices;
};

struct xge_texture_t {
	int iWidth;
	int iHeight;
	int iFormat;
	int iRefCount;
	uint32_t iFlags;
	uint32_t iBackendId;
	xge_sampler_t tSampler;
	void* pBackend;
};

struct xge_render_target_t {
	int iWidth;
	int iHeight;
	uint32_t iFlags;
	uint32_t iFramebufferId;
	xge_texture_t tTexture;
};

struct xge_pass_t {
	xge_render_target pTarget;
	uint32_t iClearFlags;
	uint32_t iClearColor;
	int iPrevWidth;
	int iPrevHeight;
	uint32_t iPrevFramebufferId;
	xge_camera_t tPrevCamera;
	int bPrevViewportEnabled;
	xge_rect_t tPrevViewportRect;
	int bActive;
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

typedef void* (*xge_xpack_read_proc)(void* pPack, const char* sPath, uint64_t* pSize, void* pUser);
typedef void (*xge_xpack_free_proc)(void* pData, void* pUser);

typedef struct xge_xpack_provider_t {
	void* pPack;
	const char* sRoot;
	xge_xpack_read_proc read;
	xge_xpack_free_proc free;
	void* pUser;
} xge_xpack_provider_t;

typedef struct xge_miniprogram_desc_t {
	void* pCanvas;
	void* pWebGLContext;
	int iWidth;
	int iHeight;
	float fDevicePixelRatio;
	void* pUser;
} xge_miniprogram_desc_t;

typedef struct xge_miniprogram_touch_t {
	uint64_t iId;
	float fX;
	float fY;
	float fForce;
} xge_miniprogram_touch_t;

typedef struct xge_miniprogram_bridge_t {
	int (*request_frame)(void* pUser);
	int (*load_resource)(const char* sURI, void** ppData, int* pSize, void* pUser);
	void (*free_resource)(void* pData, void* pUser);
	int (*audio_command)(int iCommand, int iHandle, const void* pData, int iSize, void* pUser);
	void* pUser;
} xge_miniprogram_bridge_t;

typedef struct xge_egl_desc_t {
	void* pNativeDisplay;
	void* pNativeWindow;
	int iWidth;
	int iHeight;
	int bPBuffer;
	int bSurfaceless;
	int bBoardLinux;
	void* pUser;
} xge_egl_desc_t;

typedef struct xge_egl_caps_t {
	int bCompiled;
	int bOffscreen;
	int bPBuffer;
	int bSurfaceless;
	int bNativeWindow;
	int bBoardLinux;
	int iLastError;
	char sBackendName[64];
	char sLastStage[64];
} xge_egl_caps_t;

typedef struct xge_egl_context_t {
	int bInitialized;
	int bPBuffer;
	int bSurfaceless;
	int bBoardLinux;
	int iWidth;
	int iHeight;
	void* pDisplay;
	void* pConfig;
	void* pSurface;
	void* pContext;
	void* pUser;
	int iLastError;
	char sLastStage[64];
} xge_egl_context_t;

typedef struct xge_offscreen_t {
	int iWidth;
	int iHeight;
	xge_egl_context_t tEGL;
	xge_render_target_t tTarget;
	int bActive;
	int bEGLContext;
	int bFallbackRenderTarget;
} xge_offscreen_t, *xge_offscreen;

typedef struct xge_async_request_t xge_async_request_t;
typedef xge_async_request_t* xge_async_request;
typedef void (*xge_async_proc)(xge_async_request pRequest, void* pUser);

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

struct xge_async_request_t {
	int iType;
	int iStatus;
	int iResult;
	float fProgress;
	void* pTarget;
	char* sURI;
	xge_async_proc onComplete;
	void* pUser;
	void* pThread;
	uint32_t iFlags;
	float fSize;
	int bCancel;
	int bThreaded;
	int bCallbackPending;
};

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
	uint32_t iFlags;
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
	uint64_t iPointerId;
	uint32_t iCodepoint;
	double fTime;
	void* pData;
} xge_event_t;

typedef struct xge_scene_t xge_scene_t;
typedef xge_scene_t* xge_scene;
typedef int (*xge_scene_lifecycle_proc)(xge_scene pScene);
typedef int (*xge_scene_event_proc)(xge_scene pScene, const xge_event_t* pEvent);
typedef int (*xge_scene_update_proc)(xge_scene pScene, float fDelta);
typedef int (*xge_scene_draw_proc)(xge_scene pScene);

struct xge_scene_t {
	void* pUser;
	xge_scene_lifecycle_proc onEnter;
	xge_scene_lifecycle_proc onLeave;
	xge_scene_lifecycle_proc onPause;
	xge_scene_lifecycle_proc onResume;
	xge_scene_event_proc onEvent;
	xge_scene_update_proc onUpdate;
	xge_scene_draw_proc onDraw;
	xge_scene_lifecycle_proc onFree;
	int bEntered;
	int bPaused;
};

typedef struct xge_ime_event_t {
	const char* sText;
	int iCursor;
	int iSelectStart;
	int iSelectEnd;
} xge_ime_event_t;

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

typedef struct xge_gamepad_state_t {
	int bConnected;
	uint32_t iButtons;
	uint32_t iButtonsPressed;
	uint32_t iButtonsReleased;
	float arrAxes[XGE_GAMEPAD_AXIS_COUNT];
} xge_gamepad_state_t;

typedef int (*xge_scene_proc)(void* pUser);

XGE_API int xgeInit(const xge_desc_t* pDesc);
XGE_API void xgeUnit(void);
XGE_API void xgeMemoryFree(void* pData);
XGE_API int xgeRun(xge_scene_proc procFrame, void* pUser);
XGE_API void xgeQuit(void);
XGE_API void xgeRenderRequest(void);
XGE_API int xgeFrame(void);
XGE_API int xgeRender(void);
XGE_API void xgeFrameStatsReset(void);
XGE_API xge_frame_stats_t xgeFrameStatsGet(void);
#if XGE_HAS_DEBUGMODE
/* Debug APIs are only declared for xgedbg builds.
 * Build with -DXGE_DEBUGMODE=1 and link xgedbg instead of xge. */
XGE_API int xgeDebugGetStats(xge_debug_stats_t* pStats);
XGE_API int xgeDebugDumpCaps(char* sBuffer, int iSize);
#endif
XGE_API int xgeLogSetLevel(int iLevel);
XGE_API int xgeLogGetLevel(void);
XGE_API int xgeLogWrite(int iLevel, const char* sTag, const char* sMessage);
XGE_API int xgeLogFlush(void);
XGE_API xge_platform_backend_t xgePlatformBackendDefault(void);
XGE_API int xgePlatformBackendSet(const xge_platform_backend_t* pBackend);
XGE_API xge_platform_backend_t xgePlatformBackendGet(void);
XGE_API xge_graphics_backend_t xgeGraphicsBackendDefault(void);
XGE_API int xgeGraphicsBackendSet(const xge_graphics_backend_t* pBackend);
XGE_API xge_graphics_backend_t xgeGraphicsBackendGet(void);
XGE_API int xgeGpuCapsGet(xge_gpu_caps_t* pCaps);
XGE_API int xgePlatformCapsGet(xge_platform_caps_t* pCaps);
XGE_API int xgePlatformRuntimeGet(xge_platform_runtime_t* pRuntime);
XGE_API int xgeGraphicsShaderHeaderGet(int iBackend, char* sBuffer, int iSize);
XGE_API int xgeGraphicsLibraryNameGet(int iBackend, int iIndex, char* sBuffer, int iSize);
XGE_API int xgeGraphicsMappingGet(int iBackend, xge_graphics_mapping_t* pMapping);
XGE_API int xgeSceneSet(xge_scene pScene);
XGE_API int xgeScenePush(xge_scene pScene);
XGE_API int xgeScenePop(void);
XGE_API int xgeSceneReplace(xge_scene pScene);
XGE_API xge_scene xgeSceneCurrent(void);
XGE_API int xgeSceneCount(void);
XGE_API int xgeSceneDispatchEvent(const xge_event_t* pEvent);
XGE_API int xgeSceneUpdateStrategySet(int iMode, float fFixedStep, int iMaxUpdates);
XGE_API void xgeSceneUpdateStrategyGet(int* pMode, float* pFixedStep, int* pMaxUpdates);

XGE_API int xgeGetWidth(void);
XGE_API int xgeGetHeight(void);
XGE_API float xgeGetDelta(void);
XGE_API int xgeGetFPS(void);
XGE_API double xgeTimer(void);
XGE_API int64_t xgeTimeNow(void);
XGE_API void xgeSleep(uint32_t iMilliseconds);
XGE_API void xgeSetTitle(const char* sTitle);

XGE_API int xgeBegin(void);
XGE_API int xgeEnd(void);
XGE_API int xgeFlush(void);
XGE_API int xgeRenderThreadCapsGet(xge_render_thread_caps_t* pCaps);
XGE_API int xgeRenderThreadEGLSet(const xge_egl_desc_t* pDesc);
XGE_API int xgeRenderThreadSet(int bEnabled);
XGE_API int xgeRenderThreadGet(void);
XGE_API void xgeClear(uint32_t iColor);
XGE_API void xgePresent(void);
XGE_API void xgeInvalidateRect(xge_rect_t tRect);
#if XGE_HAS_DEBUGMODE
/* Dirty rect inspection is a debug feature; xge keeps only the runtime
 * invalidation mechanism. Build with -DXGE_DEBUGMODE=1 and link xgedbg. */
XGE_API int xgedbgDirtyRectCount(void);
XGE_API int xgedbgDirtyRectGet(int iIndex, xge_rect_t* pRect);
XGE_API void xgedbgDirtyRectClear(void);
#endif
XGE_API uint32_t xgeColorRGBA(int iR, int iG, int iB, int iA);
XGE_API xge_color_t xgeColorUnpack(uint32_t iColor);
XGE_API void xgeBlendSet(int iBlend);
XGE_API int xgeBlendGet(void);
XGE_API void xgeDepthTestSet(int bEnabled);
XGE_API int xgeDepthTestGet(void);
XGE_API xge_camera_t xgeCameraDefault(float fWidth, float fHeight);
XGE_API xge_camera_t xgeCameraPerspective(float fWidth, float fHeight, float fFovY, float fNearZ, float fFarZ);
XGE_API void xgeCameraSet(const xge_camera_t* pCamera);
XGE_API xge_camera_t xgeCameraGet(void);
XGE_API xge_vec2_t xgeWorldToScreen(xge_vec2_t tPoint);
XGE_API xge_vec2_t xgeScreenToWorld(xge_vec2_t tPoint);
XGE_API int xgeResourceProviderAdd(const xge_resource_provider_t* pProvider);
XGE_API void xgeResourceProviderClear(void);
XGE_API int xgeResourceXPackProviderAdd(const xge_xpack_provider_t* pProvider);
XGE_API int xgeResourceLoad(const char* sURI, xge_resource_t* pResource);
XGE_API int xgeResourceLoadMemory(const void* pData, int iSize, xge_resource_t* pResource);
XGE_API void xgeResourceFree(xge_resource_t* pResource);
XGE_API int xgeMiniProgramInit(const xge_miniprogram_desc_t* pDesc);
XGE_API int xgeMiniProgramInitSimple(int iWidth, int iHeight, float fDevicePixelRatio);
XGE_API void xgeMiniProgramUnit(void);
XGE_API int xgeMiniProgramSetBridge(const xge_miniprogram_bridge_t* pBridge);
XGE_API int xgeMiniProgramFrame(double fTimeSeconds);
XGE_API int xgeMiniProgramResize(int iWidth, int iHeight, float fDevicePixelRatio);
XGE_API int xgeMiniProgramTouch(int iPhase, const xge_miniprogram_touch_t* pTouches, int iCount);
XGE_API int xgeMiniProgramTouchOne(int iPhase, int iId, float fX, float fY, float fForce);
XGE_API int xgeMiniProgramText(uint32_t iCodepoint);
XGE_API int xgeMiniProgramRequestFrame(void);
XGE_API int xgeMiniProgramAudioCommand(int iCommand, int iHandle, const void* pData, int iSize);
XGE_API int xgeEGLCapsGet(xge_egl_caps_t* pCaps);
XGE_API int xgeEGLInit(xge_egl_context_t* pContext, const xge_egl_desc_t* pDesc);
XGE_API void xgeEGLUnit(xge_egl_context_t* pContext);
XGE_API int xgeEGLMakeCurrent(xge_egl_context_t* pContext);
XGE_API int xgeOffscreenInit(xge_offscreen pOffscreen, int iWidth, int iHeight);
XGE_API void xgeOffscreenUnit(xge_offscreen pOffscreen);
XGE_API xge_render_target xgeOffscreenRenderTarget(xge_offscreen pOffscreen);
XGE_API int xgeOffscreenReadPixels(xge_offscreen pOffscreen, void* pPixels, int iStride);
XGE_API void xgeAsyncRequestInit(xge_async_request pRequest);
XGE_API void xgeAsyncRequestFree(xge_async_request pRequest);
XGE_API int xgeAsyncRequestCancel(xge_async_request pRequest);
XGE_API int xgeAsyncThreadingSet(int bEnabled);
XGE_API int xgeAsyncThreadingGet(void);
XGE_API int xgeAsyncPoll(xge_async_request pRequest);
XGE_API int xgeAsyncImageLoad(xge_async_request pRequest, xge_image pImage, const char* sPath, uint32_t iFlags, xge_async_proc onComplete, void* pUser);
XGE_API int xgeAsyncTextureLoad(xge_async_request pRequest, xge_texture pTexture, const char* sPath, uint32_t iFlags, xge_async_proc onComplete, void* pUser);
XGE_API int xgeAsyncFontLoad(xge_async_request pRequest, xge_font pFont, const char* sPath, float fSize, xge_async_proc onComplete, void* pUser);
XGE_API int xgeAsyncSoundLoad(xge_async_request pRequest, xge_sound pSound, const char* sPath, xge_async_proc onComplete, void* pUser);
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
XGE_API int xgeSoundFallbackSet(const char* sPath);
XGE_API int xgeSoundFallbackGet(xge_sound pSound);
XGE_API void xgeSoundFallbackClear(void);
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
XGE_API int xgeFontBuildXRFMemory(xge_font pFont, uint32_t iFirstCodepoint, uint32_t iCount, void** ppData, int* pSize);
XGE_API int xgeFontSaveXRF(xge_font pFont, const char* sPath, uint32_t iFirstCodepoint, uint32_t iCount);
XGE_API int xgeFontLoadCached(xge_font pFont, const char* sTTFPath, const char* sXRFPath, float fSize, uint32_t iFirstCodepoint, uint32_t iCount);
XGE_API int xgeFontAddRef(xge_font pFont);
XGE_API void xgeFontFree(xge_font pFont);
XGE_API void xgeFontSetFallback(xge_font pFont, xge_font pFallback);
XGE_API int xgeFontFallbackSet(const char* sPath, float fSize);
XGE_API int xgeFontFallbackSetMemory(const void* pData, int iSize, float fSize);
XGE_API int xgeFontFallbackGet(xge_font pFont, float fSize);
XGE_API void xgeFontFallbackClear(void);
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
XGE_API int xgeImageSavePNG(const char* sPath, int iWidth, int iHeight, const void* pPixels, int iStride);
XGE_API void xgeImageFree(xge_image pImage);
XGE_API int xgeTextureCreateRGBA(xge_texture pTexture, int iWidth, int iHeight, const void* pPixels);
XGE_API int xgeTextureCreateFromImage(xge_texture pTexture, const xge_image_t* pImage);
XGE_API int xgeTextureLoad(xge_texture pTexture, const char* sPath);
XGE_API int xgeTextureLoadEx(xge_texture pTexture, const char* sPath, uint32_t iFlags);
XGE_API int xgeTextureLoadMemory(xge_texture pTexture, const void* pData, int iSize);
XGE_API int xgeTextureLoadMemoryEx(xge_texture pTexture, const void* pData, int iSize, uint32_t iFlags);
XGE_API int xgeTextureAddRef(xge_texture pTexture);
XGE_API int xgeTextureUpdateRGBA(xge_texture pTexture, int iX, int iY, int iWidth, int iHeight, const void* pPixels, int iStride);
XGE_API xge_sampler_t xgeSamplerDefault(void);
XGE_API int xgeTextureSetSampler(xge_texture pTexture, const xge_sampler_t* pSampler);
XGE_API xge_sampler_t xgeTextureGetSampler(xge_texture pTexture);
XGE_API int xgeTextureFallbackSetRGBA(int iWidth, int iHeight, const void* pPixels);
XGE_API int xgeTextureFallbackGet(xge_texture pTexture);
XGE_API void xgeTextureFallbackClear(void);
XGE_API int xgeTextureUploadQueue(xge_texture pTexture);
XGE_API int xgeTextureUploadFlush(void);
XGE_API int xgeTextureReadPixels(xge_texture pTexture, void* pPixels, int iStride);
XGE_API void xgeTextureFree(xge_texture pTexture);
XGE_API int xgeRenderTargetWindow(xge_render_target pTarget);
XGE_API int xgeRenderTargetCreate(xge_render_target pTarget, int iWidth, int iHeight);
XGE_API int xgeRenderTargetResize(xge_render_target pTarget, int iWidth, int iHeight);
XGE_API int xgeRenderTargetReadPixels(xge_render_target pTarget, void* pPixels, int iStride);
XGE_API xge_texture xgeRenderTargetTexture(xge_render_target pTarget);
XGE_API void xgeRenderTargetFree(xge_render_target pTarget);
XGE_API int xgeBufferCreate(xge_buffer pBuffer, int iType, int iUsage, const void* pData, int iSize);
XGE_API int xgeBufferUpdate(xge_buffer pBuffer, int iOffset, const void* pData, int iSize);
XGE_API int xgeBufferUpload(xge_buffer pBuffer);
XGE_API void xgeBufferFree(xge_buffer pBuffer);
XGE_API void xgePassInit(xge_pass pPass, xge_render_target pTarget, uint32_t iClearFlags, uint32_t iClearColor);
XGE_API int xgePassBegin(xge_pass pPass);
XGE_API int xgePassEnd(xge_pass pPass);
XGE_API int xgeShaderCreate(xge_shader pShader, const char* sVertexSource, const char* sFragmentSource);
XGE_API int xgeShaderAddRef(xge_shader pShader);
XGE_API void xgeShaderFree(xge_shader pShader);
XGE_API int xgeShaderUniform1f(xge_shader pShader, const char* sName, float fX);
XGE_API int xgeShaderUniform2f(xge_shader pShader, const char* sName, float fX, float fY);
XGE_API int xgeShaderUniform3f(xge_shader pShader, const char* sName, float fX, float fY, float fZ);
XGE_API int xgeShaderUniform4f(xge_shader pShader, const char* sName, float fX, float fY, float fZ, float fW);
XGE_API int xgeShaderVariantSetInit(xge_shader_variant_set pSet, const char* sVertexSource, const char* sFragmentSource);
XGE_API void xgeShaderVariantSetFree(xge_shader_variant_set pSet);
XGE_API int xgeShaderVariantGet(xge_shader_variant_set pSet, uint32_t iKey, const xge_shader_define_t* pDefines, int iDefineCount, xge_shader* ppShader);
XGE_API void xgeMaterialInit(xge_material pMaterial);
XGE_API void xgeMaterialFree(xge_material pMaterial);
XGE_API void xgeMaterialSetShader(xge_material pMaterial, xge_shader pShader);
XGE_API void xgeMaterialSetTexture(xge_material pMaterial, xge_texture pTexture);
XGE_API void xgeMaterialSetColor(xge_material pMaterial, uint32_t iColor);
XGE_API void xgeMaterialSetBlend(xge_material pMaterial, int iBlend);
XGE_API void xgeMaterialDraw(xge_material pMaterial, const xge_draw_t* pDraw);
XGE_API int xgeMeshCreate(xge_mesh pMesh, const xge_vertex_t* pVertices, int iVertexCount, const uint16_t* pIndices, int iIndexCount, uint32_t iFlags);
XGE_API int xgeMeshUpdate(xge_mesh pMesh, const xge_vertex_t* pVertices, int iVertexCount, const uint16_t* pIndices, int iIndexCount);
XGE_API void xgeMeshFree(xge_mesh pMesh);
XGE_API void xgeMeshDraw(xge_mesh pMesh, xge_texture pTexture, uint32_t iFlags);
XGE_API void xgeDraw(xge_texture pTexture, float fX, float fY);
XGE_API void xgeDrawEx(const xge_draw_t* pDraw);
XGE_API void xgeDrawQuad3D(xge_texture pTexture, const xge_vertex_t* pVertices, uint32_t iFlags);
XGE_API void xgeDrawPx(xge_texture pTexture, int iX, int iY);
XGE_API void xgeNinePatchInitSimple(xge_nine_patch pPatch, xge_texture pTexture, xge_rect_t tSrc);
XGE_API void xgeNinePatchInit(xge_nine_patch pPatch, xge_texture pTexture, xge_rect_t tSrc, float fX1, float fY1, float fX2, float fY2);
XGE_API void xgeNinePatchSetMode(xge_nine_patch pPatch, int iMode);
XGE_API void xgeNinePatchSetColor(xge_nine_patch pPatch, uint32_t iColor);
XGE_API void xgeNinePatchDraw(const xge_nine_patch_t* pPatch, xge_rect_t tDst, uint32_t iFlags);
XGE_API int xgeSpriteBatchInit(xge_sprite_batch pBatch, xge_texture pTexture, int iCapacity, uint32_t iFlags);
XGE_API void xgeSpriteBatchFree(xge_sprite_batch pBatch);
XGE_API void xgeSpriteBatchClear(xge_sprite_batch pBatch);
XGE_API int xgeSpriteBatchAdd(xge_sprite_batch pBatch, const xge_draw_t* pDraw);
XGE_API int xgeSpriteBatchFlush(xge_sprite_batch pBatch);
XGE_API void xgeShapePoint(float fX, float fY, float fSize, uint32_t iColor);
XGE_API void xgeShapePointPx(float fX, float fY, float fSize, uint32_t iColor);
XGE_API void xgeShapeLine(float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor);
XGE_API void xgeShapeLinePx(float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor);
XGE_API void xgeShapeRectFill(xge_rect_t tRect, uint32_t iColor);
XGE_API void xgeShapeRectFillPx(xge_rect_t tRect, uint32_t iColor);
XGE_API void xgeShapeRectStroke(xge_rect_t tRect, float fWidth, uint32_t iColor);
XGE_API void xgeShapeRectStrokePx(xge_rect_t tRect, float fWidth, uint32_t iColor);
XGE_API void xgeShapeCircleFill(float fX, float fY, float fRadius, uint32_t iColor);
XGE_API void xgeShapeCircleFillPx(float fX, float fY, float fRadius, uint32_t iColor);
XGE_API void xgeShapeCircleStroke(float fX, float fY, float fRadius, float fWidth, uint32_t iColor);
XGE_API void xgeShapeCircleStrokePx(float fX, float fY, float fRadius, float fWidth, uint32_t iColor);
XGE_API void xgeShapeArc(float fX, float fY, float fRadius, float fStartRadians, float fEndRadians, float fWidth, uint32_t iColor);
XGE_API void xgeShapeArcPx(float fX, float fY, float fRadius, float fStartRadians, float fEndRadians, float fWidth, uint32_t iColor);
XGE_API void xgeShapeTriangleFill(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor);
XGE_API void xgeShapeTriangleFillPx(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor);
XGE_API void xgeShapePolygonFill(const xge_vec2_t* pPoints, int iCount, uint32_t iColor);
XGE_API void xgeShapePolygonFillPx(const xge_vec2_t* pPoints, int iCount, uint32_t iColor);
XGE_API int xgeShapeMeshFill(const xge_shape_vertex_t* pVertices, int iVertexCount, const uint32_t* pIndices, int iIndexCount);
XGE_API int xgeShapeMeshFillPx(const xge_shape_vertex_t* pVertices, int iVertexCount, const uint32_t* pIndices, int iIndexCount);
XGE_API int xgeShapeBatchInit(xge_shape_batch pBatch, uint32_t iColor, int iTriangleCapacity, uint32_t iFlags);
XGE_API void xgeShapeBatchFree(xge_shape_batch pBatch);
XGE_API void xgeShapeBatchClear(xge_shape_batch pBatch);
XGE_API int xgeShapeBatchTriangleFill(xge_shape_batch pBatch, xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC);
XGE_API int xgeShapeBatchRectFill(xge_shape_batch pBatch, xge_rect_t tRect);
XGE_API int xgeShapeBatchFlush(xge_shape_batch pBatch);
XGE_API void xgeViewportSet(xge_rect_t tRect);
XGE_API xge_rect_t xgeViewportGet(void);
XGE_API void xgeViewportClear(void);
XGE_API void xgeClipSet(xge_rect_t tRect);
XGE_API xge_rect_t xgeClipGet(void);
XGE_API void xgeClipClear(void);
XGE_API void xgeClipboardSetText(const char* sText);
XGE_API const char* xgeClipboardGetText(void);

XGE_API int xgeKeyDown(int iKey);
XGE_API int xgeKeyPressed(int iKey);
XGE_API int xgeKeyReleased(int iKey);
XGE_API void xgeMouseGet(float* pX, float* pY);
XGE_API void xgeMouseGetDelta(float* pDX, float* pDY);
XGE_API void xgeMouseGetWheel(float* pX, float* pY);
XGE_API int xgeMouseDown(int iButton);
XGE_API uint32_t xgeTextGet(void);
XGE_API int xgeImeGetEnabled(void);
XGE_API int xgeImeSetEnabled(int bEnabled);
XGE_API int xgeTouchGetCount(void);
XGE_API int xgeTouchGet(int iIndex, xge_touch_point_t* pPoint);
XGE_API int xgeTouchFind(uint64_t iId, xge_touch_point_t* pPoint);
XGE_API int xgeGamepadConnected(int iGamepad);
XGE_API int xgeGamepadGetState(int iGamepad, xge_gamepad_state_t* pState);
XGE_API int xgeGamepadButtonDown(int iGamepad, uint32_t iButton);
XGE_API int xgeGamepadButtonPressed(int iGamepad, uint32_t iButton);
XGE_API int xgeGamepadButtonReleased(int iGamepad, uint32_t iButton);
XGE_API float xgeGamepadAxis(int iGamepad, int iAxis);
XGE_API int xgeGamepadSetConnected(int iGamepad, int bConnected);
XGE_API int xgeGamepadSetState(int iGamepad, const xge_gamepad_state_t* pState);


#ifdef __cplusplus
}
#endif

#endif /* XGE_H */
