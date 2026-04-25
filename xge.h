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

#define XGE_MESH_DYNAMIC	0x0001

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
#define XGE_EVENT_XUI_FOCUS_IN	17
#define XGE_EVENT_XUI_FOCUS_OUT	18
#define XGE_EVENT_GAMEPAD_CONNECTED	19
#define XGE_EVENT_GAMEPAD_DISCONNECTED	20
#define XGE_EVENT_XUI_POINTER_ENTER	21
#define XGE_EVENT_XUI_POINTER_LEAVE	22
#define XGE_EVENT_XUI_CAPTURE_LOST	23
#define XGE_EVENT_XUI_CONTEXT_BEGIN	24
#define XGE_EVENT_XUI_CONTEXT_UPDATE	25
#define XGE_EVENT_XUI_CONTEXT_END		26
#define XGE_EVENT_XUI_CONTEXT_CANCEL	27

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

#define XGE_XUI_LAYOUT_ABSOLUTE	0
#define XGE_XUI_LAYOUT_ROW		1
#define XGE_XUI_LAYOUT_COLUMN	2
#define XGE_XUI_LAYOUT_STACK	3
#define XGE_XUI_LAYOUT_GRID		4

#define XGE_XUI_ALIGN_START		0
#define XGE_XUI_ALIGN_CENTER	1
#define XGE_XUI_ALIGN_END		2
#define XGE_XUI_ALIGN_STRETCH	3

#define XGE_XUI_JUSTIFY_START			0
#define XGE_XUI_JUSTIFY_CENTER			1
#define XGE_XUI_JUSTIFY_END			2
#define XGE_XUI_JUSTIFY_SPACE_BETWEEN	3

#define XGE_XUI_ANCHOR_LEFT		0x0001
#define XGE_XUI_ANCHOR_TOP		0x0002
#define XGE_XUI_ANCHOR_RIGHT	0x0004
#define XGE_XUI_ANCHOR_BOTTOM	0x0008

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
#define XGE_XUI_SEPARATOR_HORIZONTAL	0
#define XGE_XUI_SEPARATOR_VERTICAL		1

#define XGE_XUI_PAINT_RECT		1
#define XGE_XUI_PAINT_ROUNDED_RECT	2
#define XGE_XUI_PAINT_IMAGE		3
#define XGE_XUI_PAINT_TEXT		4
#define XGE_XUI_PAINT_CLIP_SET	5
#define XGE_XUI_PAINT_CLIP_CLEAR	6
#define XGE_XUI_PAINT_CUSTOM	7

#define XGE_XUI_EVENT_CONTINUE	0
#define XGE_XUI_EVENT_CONSUMED	1
#define XGE_XUI_EVENT_QUEUE_CAPACITY	64

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

typedef struct xge_debug_stats_t {
	xge_frame_stats_t tFrame;
	int iTextureCount;
	uint64_t iTextureMemoryBytes;
	int iFontCount;
	int iAudioCount;
	int iLastGLError;
} xge_debug_stats_t;

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

typedef struct xge_draw_t {
	xge_texture pTexture;
	xge_rect_t tSrc;
	xge_rect_t tDst;
	xge_vec2_t tOrigin;
	float fRotation;
	uint32_t iColor;
	uint32_t iFlags;
} xge_draw_t;

typedef struct xge_vertex_t {
	float fX;
	float fY;
	float fZ;
	float fW;
	float fU;
	float fV;
	uint32_t iColor;
} xge_vertex_t;

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
	uint32_t iCodepoint;
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

typedef struct xge_xui_paint_command_t {
	int iType;
	xge_rect_t tRect;
	xge_draw_t tDraw;
	xge_font pFont;
	const char* sText;
	uint32_t iColor;
	uint32_t iFlags;
	float fRadius;
	void* pUser;
} xge_xui_paint_command_t;

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
	xge_xui_edges_t tAnchor;
	int iGridColumns;
	float fGridRowHeight;
	float fGridColumnGap;
	float fGridRowGap;
	float fGap;
	int iAlignX;
	int iAlignY;
	int iJustify;
	int iZ;
	int iAnchor;
	int iClip;
	uint32_t iBackgroundColor;
	float fRadius;
} xge_xui_style_t;

typedef struct xge_xui_theme_t {
	xge_font pFont;
	uint32_t iTextColor;
	uint32_t iBackgroundColor;
	uint32_t iPanelColor;
	uint32_t iBorderColor;
	uint32_t iAccentColor;
	uint32_t iSelectionColor;
	uint32_t iStateNormal;
	uint32_t iStateHover;
	uint32_t iStateActive;
	uint32_t iStateFocus;
	uint32_t iStateDisabled;
	float fRadius;
	float fPadding;
	float fSpacing;
	float fBorderWidth;
} xge_xui_theme_t, *xge_xui_theme;

typedef struct xge_xui_widget_t xge_xui_widget_t;
typedef xge_xui_widget_t* xge_xui_widget;
typedef struct xge_xui_context_t xge_xui_context_t;
typedef xge_xui_context_t* xge_xui_context;
typedef struct xge_xui_button_t xge_xui_button_t;
typedef xge_xui_button_t* xge_xui_button;
typedef struct xge_xui_icon_button_t xge_xui_icon_button_t;
typedef xge_xui_icon_button_t* xge_xui_icon_button;
typedef struct xge_xui_label_t xge_xui_label_t;
typedef xge_xui_label_t* xge_xui_label;
typedef struct xge_xui_image_t xge_xui_image_t;
typedef xge_xui_image_t* xge_xui_image;
typedef struct xge_xui_input_t xge_xui_input_t;
typedef xge_xui_input_t* xge_xui_input;
typedef struct xge_xui_text_edit_t xge_xui_text_edit_t;
typedef xge_xui_text_edit_t* xge_xui_text_edit;
typedef struct xge_xui_toggle_t xge_xui_toggle_t;
typedef xge_xui_toggle_t* xge_xui_toggle;
typedef struct xge_xui_checkbox_t xge_xui_checkbox_t;
typedef xge_xui_checkbox_t* xge_xui_checkbox;
typedef struct xge_xui_radio_group_t xge_xui_radio_group_t;
typedef xge_xui_radio_group_t* xge_xui_radio_group;
typedef struct xge_xui_radio_t xge_xui_radio_t;
typedef xge_xui_radio_t* xge_xui_radio;
typedef struct xge_xui_switch_t xge_xui_switch_t;
typedef xge_xui_switch_t* xge_xui_switch;
typedef struct xge_xui_separator_t xge_xui_separator_t;
typedef xge_xui_separator_t* xge_xui_separator;
typedef struct xge_xui_splitter_t xge_xui_splitter_t;
typedef xge_xui_splitter_t* xge_xui_splitter;
typedef struct xge_xui_tabs_t xge_xui_tabs_t;
typedef xge_xui_tabs_t* xge_xui_tabs;
typedef struct xge_xui_scrollbar_t xge_xui_scrollbar_t;
typedef xge_xui_scrollbar_t* xge_xui_scrollbar;
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
typedef struct xge_xui_dialog_t xge_xui_dialog_t;
typedef xge_xui_dialog_t* xge_xui_dialog;
typedef struct xge_xui_popup_t xge_xui_popup_t;
typedef xge_xui_popup_t* xge_xui_popup;
typedef struct xge_xui_tooltip_t xge_xui_tooltip_t;
typedef xge_xui_tooltip_t* xge_xui_tooltip;
typedef struct xge_xui_combo_box_t xge_xui_combo_box_t;
typedef xge_xui_combo_box_t* xge_xui_combo_box;
typedef struct xge_xui_menu_t xge_xui_menu_t;
typedef xge_xui_menu_t* xge_xui_menu;
typedef struct xge_xui_split_layout_t xge_xui_split_layout_t;
typedef xge_xui_split_layout_t* xge_xui_split_layout;

typedef struct xge_xui_host_t {
	void (*draw_rect)(xge_rect_t tRect, uint32_t iColor, void* pUser);
	void (*draw_image)(const xge_draw_t* pDraw, void* pUser);
	void (*draw_text_rect)(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags, void* pUser);
	xge_vec2_t (*measure_text)(xge_font pFont, const char* sText, void* pUser);
	void (*clip_set)(xge_rect_t tRect, void* pUser);
	void (*clip_clear)(void* pUser);
	void (*request_refresh)(void* pUser);
	void* pUser;
} xge_xui_host_t, *xge_xui_host;

typedef struct xge_xui_text_t {
	char* sText;
	char* sComposition;
	int iSize;
	int iCapacity;
	int iCompositionSize;
	int iCompositionCapacity;
	int iCursor;
	int iSelectStart;
	int iSelectEnd;
} xge_xui_text_t, *xge_xui_text;

typedef int (*xge_xui_event_proc)(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
typedef void (*xge_xui_update_proc)(xge_xui_widget pWidget, float fDelta, void* pUser);
typedef xge_vec2_t (*xge_xui_measure_proc)(xge_xui_widget pWidget, void* pUser);
typedef void (*xge_xui_layout_proc)(xge_xui_widget pWidget, void* pUser);
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
	int iId;
	const char* sName;
	xge_xui_style_t tStyle;
	xge_rect_t tLocalRect;
	xge_rect_t tRect;
	xge_rect_t tContentRect;
	xge_vec2_t tDesiredSize;
	uint32_t iFlags;
	void* pUser;
	xge_xui_event_proc procCaptureEvent;
	void* pCaptureUser;
	xge_xui_event_proc procEvent;
	xge_xui_update_proc procUpdate;
	xge_xui_measure_proc procMeasure;
	xge_xui_layout_proc procLayout;
	void* pLayoutUser;
	xge_xui_paint_proc procPaint;
	void* pInternal;
};

struct xge_xui_context_t {
	int bInitialized;
	xge_xui_widget pRoot;
	xge_xui_widget pOverlayRoot;
	xge_xui_widget pFocus;
	xge_xui_widget pCapture;
	xge_xui_widget pHover;
	xge_event_t arrEventQueue[XGE_XUI_EVENT_QUEUE_CAPACITY];
	int iEventHead;
	int iEventTail;
	int iEventCount;
	float fDipScale;
	int iLayoutBatchDepth;
	int bLayoutBatchDirtyLayout;
	int bLayoutBatchDirtyPaint;
	int iDirtyLayoutCount;
	int iDirtyPaintCount;
	int iPaintCommandCount;
	int iPaintFlushCount;
	int bRefreshRequested;
	int bAutoDispatchProcFrameEvents;
	int bContextPressActive;
	int bContextPressMoved;
	int bContextPressFired;
	float fContextPressTime;
	float fContextPressStartX;
	float fContextPressStartY;
	float fContextPressLastX;
	float fContextPressLastY;
	xge_xui_widget pContextPressTarget;
	xge_xui_theme_t tTheme;
	const xge_xui_host_t* pHost;
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

struct xge_xui_icon_button_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_texture pTexture;
	xge_rect_t tSrc;
	xge_xui_click_proc procClick;
	void* pUser;
	uint32_t iColorNormal;
	uint32_t iColorHover;
	uint32_t iColorActive;
	uint32_t iColorFocus;
	uint32_t iColorDisabled;
	uint32_t iIconColor;
	int iMode;
	int iState;
	int iClickCount;
};

struct xge_xui_label_t {
	xge_xui_widget pWidget;
	xge_font pFont;
	const char* sText;
	char* sTextOwned;
	int iTextCapacity;
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
	xge_xui_menu pDefaultMenu;
	int arrDefaultMenuEnabled[5];
	const char* sPlaceholder;
	uint32_t iTextColor;
	uint32_t iPlaceholderColor;
	uint32_t iBackgroundColor;
	uint32_t iFocusColor;
	uint32_t iCursorColor;
	uint32_t iSelectionColor;
	uint32_t iDisabledTextColor;
	uint32_t iDisabledBackgroundColor;
	float fScrollX;
	double fLastClickTime;
	float fLastClickX;
	float fLastClickY;
	float fPressX;
	float fPressY;
	float fCursorBlinkTime;
	int iPressCursor;
	int bPressPending;
	int bPressInsideSelection;
	int bSelecting;
	int bPassword;
	int bImeManaged;
	int bImeEnabledPrev;
	int bReadonly;
	int bDisabled;
	int bCursorVisible;
	int bInitialized;
};

typedef struct xge_xui_text_edit_state_t {
	char* sText;
	int iCursor;
	int iSelectStart;
	int iSelectEnd;
} xge_xui_text_edit_state_t;

typedef struct xge_xui_text_edit_visual_line_t {
	int iStart;
	int iEnd;
} xge_xui_text_edit_visual_line_t;

struct xge_xui_text_edit_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_text_t tText;
	xge_font pFont;
	xge_xui_menu pDefaultMenu;
	int* arrLineStarts;
	xge_xui_text_edit_state_t* arrUndo;
	xge_xui_text_edit_state_t* arrRedo;
	xge_xui_text_edit_visual_line_t* arrVisualLines;
	int arrDefaultMenuEnabled[5];
	uint32_t iTextColor;
	uint32_t iBackgroundColor;
	uint32_t iFocusColor;
	uint32_t iCursorColor;
	uint32_t iSelectionColor;
	float fScrollX;
	float fScrollY;
	float fLineHeight;
	float fCursorBlinkTime;
	float fPreferredX;
	float fVisualCacheWidth;
	double fLastClickTime;
	float fLastClickX;
	float fLastClickY;
	float fPressX;
	float fPressY;
	int iLineCount;
	int iLineCapacity;
	int iUndoCount;
	int iUndoCapacity;
	int iRedoCount;
	int iRedoCapacity;
	int iUndoLimit;
	int iVisualLineCount;
	int iVisualLineCapacity;
	int iSelectionAnchor;
	int iPressCursor;
	int bPressPending;
	int bPressInsideSelection;
	int bSelecting;
	int bReadonly;
	int bWordWrap;
	int bLineCacheDirty;
	int bVisualCacheDirty;
	int bCursorVisible;
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

struct xge_xui_checkbox_t {
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
	uint32_t iColorBox;
	uint32_t iColorChecked;
	int iState;
	int bChecked;
	int iChangeCount;
};

struct xge_xui_radio_group_t {
	xge_xui_radio pFirst;
	xge_xui_select_proc procChange;
	void* pUser;
	int iSelectedValue;
	int iChangeCount;
};

struct xge_xui_radio_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_radio_group pGroup;
	xge_xui_radio pNextInGroup;
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
	uint32_t iColorRing;
	uint32_t iColorChecked;
	int iValue;
	int iState;
	int bChecked;
	int iChangeCount;
};

struct xge_xui_switch_t {
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
	uint32_t iColorTrack;
	uint32_t iColorChecked;
	uint32_t iColorKnob;
	int iState;
	int bChecked;
	int iChangeCount;
};

struct xge_xui_separator_t {
	xge_xui_widget pWidget;
	uint32_t iColor;
	float fThickness;
	int iOrientation;
};

struct xge_xui_splitter_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_slider_proc procChange;
	void* pUser;
	float fMin;
	float fMax;
	float fValue;
	float fDragStartMouse;
	float fDragStartValue;
	uint32_t iColorNormal;
	uint32_t iColorHover;
	uint32_t iColorActive;
	uint32_t iColorFocus;
	uint32_t iColorDisabled;
	int iOrientation;
	int iState;
	int iChangeCount;
};

struct xge_xui_tabs_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_font pFont;
	const char** arrItems;
	const int* arrEnabled;
	int iItemCount;
	int iEnabledCount;
	int iSelected;
	int iHover;
	float fTabWidth;
	float fTabHeight;
	xge_xui_select_proc procSelect;
	void* pUser;
	uint32_t iBackgroundColor;
	uint32_t iTabColor;
	uint32_t iHoverColor;
	uint32_t iActiveColor;
	uint32_t iFocusColor;
	uint32_t iDisabledColor;
	uint32_t iTextColor;
	uint32_t iActiveTextColor;
	int iState;
	int iChangeCount;
};

struct xge_xui_scrollbar_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_slider_proc procChange;
	void* pUser;
	float fMin;
	float fMax;
	float fPage;
	float fValue;
	float fDragStartMouse;
	float fDragStartValue;
	uint32_t iColorTrack;
	uint32_t iColorThumb;
	uint32_t iColorHover;
	uint32_t iColorActive;
	uint32_t iColorFocus;
	uint32_t iColorDisabled;
	int iOrientation;
	int iState;
	int bDraggingThumb;
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
	float fDragScrollX;
	float fDragScrollY;
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
	const int* arrEnabled;
	int iItemCount;
	int iEnabledCount;
	int iSelected;
	int iHover;
	float fItemHeight;
	float fScrollY;
	float fDragY;
	float fDragScrollY;
	xge_xui_select_proc procSelect;
	void* pUser;
	uint32_t iBackgroundColor;
	uint32_t iRowColor;
	uint32_t iHoverColor;
	uint32_t iSelectedColor;
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iBarColor;
	uint32_t iThumbColor;
	int bDraggingThumb;
};

struct xge_xui_dialog_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_font pFont;
	const char* sTitle;
	xge_xui_click_proc procClose;
	void* pUser;
	uint32_t iBackdropColor;
	uint32_t iBackgroundColor;
	uint32_t iTitleColor;
	uint32_t iCloseColor;
	xge_rect_t tCloseRect;
	int bOpen;
	int bModal;
	int bCloseOnEscape;
	int bShowClose;
	int iCloseCount;
};

struct xge_xui_popup_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_widget pOwner;
	xge_xui_click_proc procClose;
	void* pUser;
	uint32_t iBackgroundColor;
	int bOpen;
	int bCloseOnOutside;
	int bCloseOnEscape;
	int iCloseCount;
};

struct xge_xui_combo_box_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_widget pPopupWidget;
	xge_xui_widget pListWidget;
	xge_xui_popup_t tPopup;
	xge_xui_list_view_t tList;
	xge_font pFont;
	const char** arrItems;
	int iItemCount;
	int iSelected;
	float fDropDownHeight;
	xge_xui_select_proc procSelect;
	void* pUser;
	uint32_t iTextColor;
	uint32_t iColorNormal;
	uint32_t iColorHover;
	uint32_t iColorActive;
	uint32_t iColorFocus;
	uint32_t iColorDisabled;
	uint32_t iPopupColor;
	int iState;
	int iChangeCount;
};

struct xge_xui_menu_t {
	xge_xui_context pContext;
	xge_xui_widget pOwner;
	xge_xui_widget pPopupWidget;
	xge_xui_widget pListWidget;
	xge_xui_popup_t tPopup;
	xge_xui_list_view_t tList;
	xge_font pFont;
	const char** arrItems;
	const int* arrEnabled;
	int iItemCount;
	int iEnabledCount;
	float fWidth;
	float fMaxHeight;
	float fItemHeight;
	xge_xui_select_proc procSelect;
	void* pUser;
	uint32_t iBackgroundColor;
	uint32_t iRowColor;
	uint32_t iSelectedColor;
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	int iSelectCount;
};

struct xge_xui_tooltip_t {
	xge_xui_context pContext;
	xge_xui_widget pOwner;
	xge_xui_widget pPopupWidget;
	xge_xui_widget pLabelWidget;
	xge_xui_popup_t tPopup;
	xge_xui_label_t tLabel;
	xge_xui_event_proc procOldCapture;
	void* pOldCaptureUser;
	xge_font pFont;
	const char* sText;
	uint32_t iBackgroundColor;
	uint32_t iTextColor;
	float fOffsetX;
	float fOffsetY;
	int bEnabled;
};

struct xge_xui_split_layout_t {
	xge_xui_context pContext;
	xge_xui_widget pWidget;
	xge_xui_widget* arrPaneWidgets;
	xge_xui_widget* arrDividerWidgets;
	float* arrPaneWeights;
	float* arrPaneResolvedSizes;
	float* arrPaneMinSizes;
	xge_xui_widget pShadowWidget;
	int iPaneCount;
	int iOrientation;
	int iActiveDivider;
	int iHoverDivider;
	int bShadowDrag;
	float fDividerSize;
	float fDragStartMouse;
	float fDragCurrentMouse;
	float fDragStartBefore;
	float fDragStartAfter;
	float fResolvedAxis;
	uint32_t iDividerColor;
	uint32_t iDividerHoverColor;
	uint32_t iDividerActiveColor;
	uint32_t iShadowColor;
};

typedef int (*xge_scene_proc)(void* pUser);

XGE_API int xgeInit(const xge_desc_t* pDesc);
XGE_API void xgeUnit(void);
XGE_API void xgeMemoryFree(void* pData);
XGE_API int xgeRun(xge_scene_proc procFrame, void* pUser);
XGE_API void xgeQuit(void);
XGE_API int xgeFrame(void);
XGE_API int xgeRender(void);
XGE_API void xgeFrameStatsReset(void);
XGE_API xge_frame_stats_t xgeFrameStatsGet(void);
XGE_API int xgeDebugGetStats(xge_debug_stats_t* pStats);
XGE_API int xgeDebugDumpCaps(char* sBuffer, int iSize);
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
XGE_API int xgeDirtyRectCount(void);
XGE_API int xgeDirtyRectGet(int iIndex, xge_rect_t* pRect);
XGE_API void xgeDirtyRectClear(void);
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

XGE_API xge_xui_size_t xgeXuiSizePx(float fValue);
XGE_API xge_xui_size_t xgeXuiSizeDip(float fValue);
XGE_API xge_xui_size_t xgeXuiSizePercent(float fValue);
XGE_API xge_xui_size_t xgeXuiSizeGrow(float fValue);
XGE_API xge_xui_size_t xgeXuiSizeContent(void);
XGE_API int xgeXuiInit(xge_xui_context pContext);
XGE_API void xgeXuiUnit(xge_xui_context pContext);
XGE_API void xgeXuiSetProcFrameEventDispatch(xge_xui_context pContext, int bEnabled);
XGE_API xge_xui_widget xgeXuiRoot(xge_xui_context pContext);
XGE_API xge_xui_widget xgeXuiOverlayRoot(xge_xui_context pContext);
XGE_API void xgeXuiSetDipScale(xge_xui_context pContext, float fScale);
XGE_API float xgeXuiGetDipScale(xge_xui_context pContext);
XGE_API void xgeXuiThemeDefault(xge_xui_theme pTheme);
XGE_API void xgeXuiSetTheme(xge_xui_context pContext, const xge_xui_theme_t* pTheme);
XGE_API const xge_xui_theme_t* xgeXuiGetTheme(xge_xui_context pContext);
XGE_API void xgeXuiStyleDefault(xge_xui_style_t* pStyle);
XGE_API void xgeXuiStyleFromTheme(xge_xui_style_t* pStyle, const xge_xui_theme_t* pTheme);
XGE_API void xgeXuiRefreshRequest(xge_xui_context pContext);
XGE_API int xgeXuiRefreshNeeded(xge_xui_context pContext);
XGE_API void xgeXuiRefreshClear(xge_xui_context pContext);
XGE_API void xgeXuiLayoutBatchBegin(xge_xui_context pContext);
XGE_API void xgeXuiLayoutBatchEnd(xge_xui_context pContext);
XGE_API void xgeXuiSetHost(xge_xui_context pContext, const xge_xui_host_t* pHost);
XGE_API const xge_xui_host_t* xgeXuiGetHost(xge_xui_context pContext);
XGE_API xge_xui_widget xgeXuiWidgetCreate(void);
XGE_API void xgeXuiWidgetFree(xge_xui_widget pWidget);
XGE_API int xgeXuiWidgetAdd(xge_xui_widget pParent, xge_xui_widget pChild);
XGE_API void xgeXuiWidgetRemove(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetId(xge_xui_widget pWidget, int iId);
XGE_API int xgeXuiWidgetGetId(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetName(xge_xui_widget pWidget, const char* sName);
XGE_API const char* xgeXuiWidgetGetName(xge_xui_widget pWidget);
XGE_API xge_xui_widget xgeXuiWidgetFindById(xge_xui_widget pRoot, int iId);
XGE_API xge_xui_widget xgeXuiWidgetFindByName(xge_xui_widget pRoot, const char* sName);
XGE_API void xgeXuiWidgetSetRect(xge_xui_widget pWidget, xge_rect_t tRect);
XGE_API xge_rect_t xgeXuiWidgetGetRect(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetStyle(xge_xui_widget pWidget, const xge_xui_style_t* pStyle);
XGE_API const xge_xui_style_t* xgeXuiWidgetGetStyle(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetLayout(xge_xui_widget pWidget, int iLayout);
XGE_API void xgeXuiWidgetSetSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
XGE_API void xgeXuiWidgetSetMinSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
XGE_API void xgeXuiWidgetSetMaxSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
XGE_API void xgeXuiWidgetSetGrid(xge_xui_widget pWidget, int iColumns, float fRowHeight, float fColumnGap, float fRowGap);
XGE_API void xgeXuiWidgetSetGap(xge_xui_widget pWidget, float fGap);
XGE_API void xgeXuiWidgetSetAlign(xge_xui_widget pWidget, int iAlignX, int iAlignY);
XGE_API void xgeXuiWidgetSetJustify(xge_xui_widget pWidget, int iJustify);
XGE_API void xgeXuiWidgetSetZ(xge_xui_widget pWidget, int iZ);
XGE_API int xgeXuiWidgetGetZ(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetAnchorPx(xge_xui_widget pWidget, int iAnchor, float fLeft, float fTop, float fRight, float fBottom);
XGE_API int xgeXuiWidgetGetAnchor(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetMarginPx(xge_xui_widget pWidget, float fLeft, float fTop, float fRight, float fBottom);
XGE_API void xgeXuiWidgetSetPaddingPx(xge_xui_widget pWidget, float fLeft, float fTop, float fRight, float fBottom);
XGE_API void xgeXuiWidgetSetBackground(xge_xui_widget pWidget, uint32_t iColor);
XGE_API void xgeXuiWidgetSetRadius(xge_xui_widget pWidget, float fRadius);
XGE_API uint32_t xgeXuiWidgetGetFlags(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetSetVisible(xge_xui_widget pWidget, int bVisible);
XGE_API void xgeXuiWidgetSetEnabled(xge_xui_widget pWidget, int bEnabled);
XGE_API void xgeXuiWidgetSetFocusable(xge_xui_widget pWidget, int bFocusable);
XGE_API void xgeXuiWidgetSetClip(xge_xui_widget pWidget, int bClip);
XGE_API void xgeXuiWidgetSetCaptureEvent(xge_xui_widget pWidget, xge_xui_event_proc procEvent);
XGE_API void xgeXuiWidgetSetCaptureEventUser(xge_xui_widget pWidget, xge_xui_event_proc procEvent, void* pUser);
XGE_API void xgeXuiWidgetSetUpdate(xge_xui_widget pWidget, xge_xui_update_proc procUpdate, void* pUser);
XGE_API void xgeXuiWidgetSetMeasure(xge_xui_widget pWidget, xge_xui_measure_proc procMeasure);
XGE_API void xgeXuiWidgetSetLayoutProc(xge_xui_widget pWidget, xge_xui_layout_proc procLayout, void* pUser);
XGE_API void xgeXuiWidgetSetPaint(xge_xui_widget pWidget, xge_xui_paint_proc procPaint, void* pUser);
XGE_API xge_vec2_t xgeXuiWidgetGetDesiredSize(xge_xui_widget pWidget);
XGE_API int xgeXuiWidgetIsVisible(xge_xui_widget pWidget);
XGE_API int xgeXuiWidgetIsEnabled(xge_xui_widget pWidget);
XGE_API int xgeXuiWidgetIsFocusable(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetMarkLayout(xge_xui_widget pWidget);
XGE_API void xgeXuiWidgetMarkPaint(xge_xui_widget pWidget);
XGE_API xge_xui_widget xgeXuiHitTest(xge_xui_context pContext, float fX, float fY);
XGE_API void xgeXuiSetFocus(xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiSetCapture(xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API int xgeXuiDispatchEvent(xge_xui_context pContext, const xge_event_t* pEvent);
XGE_API int xgeXuiDispatchProcFrameEventAll(const xge_event_t* pEvent);
XGE_API int xgeXuiEventPush(xge_xui_context pContext, const xge_event_t* pEvent);
XGE_API int xgeXuiEventPop(xge_xui_context pContext, xge_event_t* pEvent);
XGE_API int xgeXuiEventCount(xge_xui_context pContext);
XGE_API int xgeXuiDispatchQueuedEvents(xge_xui_context pContext);
XGE_API int xgeXuiUpdate(xge_xui_context pContext, float fDelta);
XGE_API int xgeXuiPaint(xge_xui_context pContext);
XGE_API int xgeXuiTextInit(xge_xui_text pText);
XGE_API void xgeXuiTextUnit(xge_xui_text pText);
XGE_API int xgeXuiTextSet(xge_xui_text pText, const char* sText);
XGE_API int xgeXuiTextInsert(xge_xui_text pText, const char* sText);
XGE_API int xgeXuiTextInsertCodepoint(xge_xui_text pText, uint32_t iCodepoint);
XGE_API int xgeXuiTextDeleteBack(xge_xui_text pText);
XGE_API int xgeXuiTextDeleteForward(xge_xui_text pText);
XGE_API void xgeXuiTextSetCursor(xge_xui_text pText, int iCursor);
XGE_API int xgeXuiTextGetCursor(xge_xui_text pText);
XGE_API void xgeXuiTextSetSelection(xge_xui_text pText, int iStart, int iEnd);
XGE_API void xgeXuiTextGetSelection(xge_xui_text pText, int* pStart, int* pEnd);
XGE_API int xgeXuiTextSetComposition(xge_xui_text pText, const char* sText);
XGE_API void xgeXuiTextClearComposition(xge_xui_text pText);
XGE_API const char* xgeXuiTextGetComposition(xge_xui_text pText);
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
XGE_API int xgeXuiIconButtonInit(xge_xui_icon_button pButton, xge_xui_context pContext, xge_xui_widget pWidget, xge_texture pTexture);
XGE_API void xgeXuiIconButtonUnit(xge_xui_icon_button pButton);
XGE_API void xgeXuiIconButtonSetClick(xge_xui_icon_button pButton, xge_xui_click_proc procClick, void* pUser);
XGE_API void xgeXuiIconButtonSetTexture(xge_xui_icon_button pButton, xge_texture pTexture);
XGE_API void xgeXuiIconButtonSetSource(xge_xui_icon_button pButton, xge_rect_t tSrc);
XGE_API void xgeXuiIconButtonSetIconColor(xge_xui_icon_button pButton, uint32_t iColor);
XGE_API void xgeXuiIconButtonSetMode(xge_xui_icon_button pButton, int iMode);
XGE_API void xgeXuiIconButtonSetColors(xge_xui_icon_button pButton, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);
XGE_API int xgeXuiIconButtonGetState(xge_xui_icon_button pButton);
XGE_API int xgeXuiIconButtonEvent(xge_xui_icon_button pButton, const xge_event_t* pEvent);
XGE_API int xgeXuiIconButtonEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiIconButtonPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiLabelInit(xge_xui_label pLabel, xge_xui_widget pWidget, xge_font pFont, const char* sText);
XGE_API void xgeXuiLabelUnit(xge_xui_label pLabel);
XGE_API void xgeXuiLabelSetText(xge_xui_label pLabel, const char* sText);
XGE_API void xgeXuiLabelSetFont(xge_xui_label pLabel, xge_font pFont);
XGE_API void xgeXuiLabelSetColor(xge_xui_label pLabel, uint32_t iColor);
XGE_API void xgeXuiLabelSetAlign(xge_xui_label pLabel, uint32_t iTextFlags);
XGE_API xge_vec2_t xgeXuiLabelMeasure(xge_xui_label pLabel);
XGE_API xge_vec2_t xgeXuiLabelMeasureProc(xge_xui_widget pWidget, void* pUser);
XGE_API void xgeXuiLabelPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiImageInit(xge_xui_image pImage, xge_xui_widget pWidget, xge_texture pTexture);
XGE_API void xgeXuiImageUnit(xge_xui_image pImage);
XGE_API void xgeXuiImageSetTexture(xge_xui_image pImage, xge_texture pTexture);
XGE_API void xgeXuiImageSetSource(xge_xui_image pImage, xge_rect_t tSrc);
XGE_API void xgeXuiImageSetColor(xge_xui_image pImage, uint32_t iColor);
XGE_API void xgeXuiImageSetMode(xge_xui_image pImage, int iMode);
XGE_API xge_vec2_t xgeXuiImageMeasureProc(xge_xui_widget pWidget, void* pUser);
XGE_API void xgeXuiImagePaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiInputInit(xge_xui_input pInput, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont);
XGE_API void xgeXuiInputUnit(xge_xui_input pInput);
XGE_API void xgeXuiInputSetText(xge_xui_input pInput, const char* sText);
XGE_API const char* xgeXuiInputGetText(xge_xui_input pInput);
XGE_API void xgeXuiInputSetFont(xge_xui_input pInput, xge_font pFont);
XGE_API void xgeXuiInputSetColors(xge_xui_input pInput, uint32_t iText, uint32_t iBackground, uint32_t iFocus, uint32_t iCursor);
XGE_API void xgeXuiInputSetPlaceholder(xge_xui_input pInput, const char* sText);
XGE_API void xgeXuiInputSetPassword(xge_xui_input pInput, int bPassword);
XGE_API void xgeXuiInputSetReadonly(xge_xui_input pInput, int bReadonly);
XGE_API void xgeXuiInputSetDisabled(xge_xui_input pInput, int bDisabled);
XGE_API void xgeXuiInputSetSelection(xge_xui_input pInput, int iStart, int iEnd);
XGE_API void xgeXuiInputGetSelection(xge_xui_input pInput, int* pStart, int* pEnd);
XGE_API xge_rect_t xgeXuiInputGetCandidateRect(xge_xui_input pInput);
XGE_API int xgeXuiInputEvent(xge_xui_input pInput, const xge_event_t* pEvent);
XGE_API int xgeXuiInputEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiInputUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser);
XGE_API void xgeXuiInputPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiTextEditInit(xge_xui_text_edit pEdit, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont);
XGE_API void xgeXuiTextEditUnit(xge_xui_text_edit pEdit);
XGE_API void xgeXuiTextEditSetText(xge_xui_text_edit pEdit, const char* sText);
XGE_API const char* xgeXuiTextEditGetText(xge_xui_text_edit pEdit);
XGE_API void xgeXuiTextEditSetFont(xge_xui_text_edit pEdit, xge_font pFont);
XGE_API void xgeXuiTextEditSetColors(xge_xui_text_edit pEdit, uint32_t iText, uint32_t iBackground, uint32_t iFocus, uint32_t iCursor);
XGE_API void xgeXuiTextEditSetReadonly(xge_xui_text_edit pEdit, int bReadonly);
XGE_API void xgeXuiTextEditSetWordWrap(xge_xui_text_edit pEdit, int bWordWrap);
XGE_API void xgeXuiTextEditSetScroll(xge_xui_text_edit pEdit, float fX, float fY);
XGE_API int xgeXuiTextEditUndo(xge_xui_text_edit pEdit);
XGE_API int xgeXuiTextEditRedo(xge_xui_text_edit pEdit);
XGE_API xge_rect_t xgeXuiTextEditGetCandidateRect(xge_xui_text_edit pEdit);
XGE_API int xgeXuiTextEditEvent(xge_xui_text_edit pEdit, const xge_event_t* pEvent);
XGE_API int xgeXuiTextEditEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiTextEditUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser);
XGE_API void xgeXuiTextEditPaintProc(xge_xui_widget pWidget, void* pUser);
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
XGE_API int xgeXuiCheckBoxInit(xge_xui_checkbox pCheckBox, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiCheckBoxUnit(xge_xui_checkbox pCheckBox);
XGE_API void xgeXuiCheckBoxSetChange(xge_xui_checkbox pCheckBox, xge_xui_toggle_proc procChange, void* pUser);
XGE_API void xgeXuiCheckBoxSetText(xge_xui_checkbox pCheckBox, xge_font pFont, const char* sText);
XGE_API void xgeXuiCheckBoxSetChecked(xge_xui_checkbox pCheckBox, int bChecked);
XGE_API int xgeXuiCheckBoxGetChecked(xge_xui_checkbox pCheckBox);
XGE_API void xgeXuiCheckBoxSetTextColor(xge_xui_checkbox pCheckBox, uint32_t iColor);
XGE_API void xgeXuiCheckBoxSetColors(xge_xui_checkbox pCheckBox, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iBox, uint32_t iChecked);
XGE_API int xgeXuiCheckBoxGetState(xge_xui_checkbox pCheckBox);
XGE_API int xgeXuiCheckBoxEvent(xge_xui_checkbox pCheckBox, const xge_event_t* pEvent);
XGE_API int xgeXuiCheckBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiCheckBoxPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API void xgeXuiRadioGroupInit(xge_xui_radio_group pGroup);
XGE_API void xgeXuiRadioGroupUnit(xge_xui_radio_group pGroup);
XGE_API void xgeXuiRadioGroupSetChange(xge_xui_radio_group pGroup, xge_xui_select_proc procChange, void* pUser);
XGE_API void xgeXuiRadioGroupSetSelected(xge_xui_radio_group pGroup, int iValue);
XGE_API int xgeXuiRadioGroupGetSelected(xge_xui_radio_group pGroup);
XGE_API int xgeXuiRadioInit(xge_xui_radio pRadio, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiRadioUnit(xge_xui_radio pRadio);
XGE_API void xgeXuiRadioSetGroup(xge_xui_radio pRadio, xge_xui_radio_group pGroup, int iValue);
XGE_API void xgeXuiRadioSetChange(xge_xui_radio pRadio, xge_xui_toggle_proc procChange, void* pUser);
XGE_API void xgeXuiRadioSetText(xge_xui_radio pRadio, xge_font pFont, const char* sText);
XGE_API void xgeXuiRadioSetChecked(xge_xui_radio pRadio, int bChecked);
XGE_API int xgeXuiRadioGetChecked(xge_xui_radio pRadio);
XGE_API void xgeXuiRadioSetTextColor(xge_xui_radio pRadio, uint32_t iColor);
XGE_API void xgeXuiRadioSetColors(xge_xui_radio pRadio, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iRing, uint32_t iChecked);
XGE_API int xgeXuiRadioGetState(xge_xui_radio pRadio);
XGE_API int xgeXuiRadioEvent(xge_xui_radio pRadio, const xge_event_t* pEvent);
XGE_API int xgeXuiRadioEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiRadioPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiSwitchInit(xge_xui_switch pSwitch, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiSwitchUnit(xge_xui_switch pSwitch);
XGE_API void xgeXuiSwitchSetChange(xge_xui_switch pSwitch, xge_xui_toggle_proc procChange, void* pUser);
XGE_API void xgeXuiSwitchSetText(xge_xui_switch pSwitch, xge_font pFont, const char* sText);
XGE_API void xgeXuiSwitchSetChecked(xge_xui_switch pSwitch, int bChecked);
XGE_API int xgeXuiSwitchGetChecked(xge_xui_switch pSwitch);
XGE_API void xgeXuiSwitchSetTextColor(xge_xui_switch pSwitch, uint32_t iColor);
XGE_API void xgeXuiSwitchSetColors(xge_xui_switch pSwitch, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iTrack, uint32_t iChecked, uint32_t iKnob);
XGE_API int xgeXuiSwitchGetState(xge_xui_switch pSwitch);
XGE_API int xgeXuiSwitchEvent(xge_xui_switch pSwitch, const xge_event_t* pEvent);
XGE_API int xgeXuiSwitchEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiSwitchPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiSeparatorInit(xge_xui_separator pSeparator, xge_xui_widget pWidget);
XGE_API void xgeXuiSeparatorUnit(xge_xui_separator pSeparator);
XGE_API void xgeXuiSeparatorSetColor(xge_xui_separator pSeparator, uint32_t iColor);
XGE_API void xgeXuiSeparatorSetThickness(xge_xui_separator pSeparator, float fThickness);
XGE_API void xgeXuiSeparatorSetOrientation(xge_xui_separator pSeparator, int iOrientation);
XGE_API void xgeXuiSeparatorPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiSplitterInit(xge_xui_splitter pSplitter, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiSplitterUnit(xge_xui_splitter pSplitter);
XGE_API void xgeXuiSplitterSetChange(xge_xui_splitter pSplitter, xge_xui_slider_proc procChange, void* pUser);
XGE_API void xgeXuiSplitterSetRange(xge_xui_splitter pSplitter, float fMin, float fMax);
XGE_API void xgeXuiSplitterSetValue(xge_xui_splitter pSplitter, float fValue);
XGE_API float xgeXuiSplitterGetValue(xge_xui_splitter pSplitter);
XGE_API void xgeXuiSplitterSetOrientation(xge_xui_splitter pSplitter, int iOrientation);
XGE_API void xgeXuiSplitterSetColors(xge_xui_splitter pSplitter, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);
XGE_API int xgeXuiSplitterGetState(xge_xui_splitter pSplitter);
XGE_API int xgeXuiSplitterEvent(xge_xui_splitter pSplitter, const xge_event_t* pEvent);
XGE_API int xgeXuiSplitterEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiSplitterPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiSplitLayoutInit(xge_xui_split_layout pSplitLayout, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiSplitLayoutUnit(xge_xui_split_layout pSplitLayout);
XGE_API void xgeXuiSplitLayoutSetOrientation(xge_xui_split_layout pSplitLayout, int iOrientation);
XGE_API void xgeXuiSplitLayoutSetPaneCount(xge_xui_split_layout pSplitLayout, int iCount);
XGE_API int xgeXuiSplitLayoutGetPaneCount(xge_xui_split_layout pSplitLayout);
XGE_API xge_xui_widget xgeXuiSplitLayoutGetPaneWidget(xge_xui_split_layout pSplitLayout, int iIndex);
XGE_API void xgeXuiSplitLayoutSetPaneWeight(xge_xui_split_layout pSplitLayout, int iIndex, float fWeight);
XGE_API float xgeXuiSplitLayoutGetPaneWeight(xge_xui_split_layout pSplitLayout, int iIndex);
XGE_API void xgeXuiSplitLayoutSetPaneMinSize(xge_xui_split_layout pSplitLayout, int iIndex, float fMinSize);
XGE_API float xgeXuiSplitLayoutGetPaneSize(xge_xui_split_layout pSplitLayout, int iIndex);
XGE_API void xgeXuiSplitLayoutSetDividerSize(xge_xui_split_layout pSplitLayout, float fSize);
XGE_API void xgeXuiSplitLayoutSetShadowDrag(xge_xui_split_layout pSplitLayout, int bEnabled);
XGE_API void xgeXuiSplitLayoutSetColors(xge_xui_split_layout pSplitLayout, uint32_t iDivider, uint32_t iHover, uint32_t iActive, uint32_t iShadow);
XGE_API void xgeXuiSplitLayoutLayoutProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiSplitLayoutDividerEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiSplitLayoutDividerPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiTabsInit(xge_xui_tabs pTabs, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiTabsUnit(xge_xui_tabs pTabs);
XGE_API void xgeXuiTabsSetItems(xge_xui_tabs pTabs, const char** arrItems, int iCount);
XGE_API void xgeXuiTabsSetFont(xge_xui_tabs pTabs, xge_font pFont);
XGE_API void xgeXuiTabsSetSelect(xge_xui_tabs pTabs, xge_xui_select_proc procSelect, void* pUser);
XGE_API void xgeXuiTabsSetSelected(xge_xui_tabs pTabs, int iIndex);
XGE_API int xgeXuiTabsGetSelected(xge_xui_tabs pTabs);
XGE_API void xgeXuiTabsSetTabSize(xge_xui_tabs pTabs, float fWidth, float fHeight);
XGE_API void xgeXuiTabsSetColors(xge_xui_tabs pTabs, uint32_t iBackground, uint32_t iTab, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iText, uint32_t iActiveText);
XGE_API int xgeXuiTabsGetState(xge_xui_tabs pTabs);
XGE_API int xgeXuiTabsEvent(xge_xui_tabs pTabs, const xge_event_t* pEvent);
XGE_API int xgeXuiTabsEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiTabsPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiScrollBarInit(xge_xui_scrollbar pScrollBar, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiScrollBarUnit(xge_xui_scrollbar pScrollBar);
XGE_API void xgeXuiScrollBarSetChange(xge_xui_scrollbar pScrollBar, xge_xui_slider_proc procChange, void* pUser);
XGE_API void xgeXuiScrollBarSetRange(xge_xui_scrollbar pScrollBar, float fMin, float fMax, float fPage);
XGE_API void xgeXuiScrollBarSetPage(xge_xui_scrollbar pScrollBar, float fPage);
XGE_API void xgeXuiScrollBarSetValue(xge_xui_scrollbar pScrollBar, float fValue);
XGE_API float xgeXuiScrollBarGetValue(xge_xui_scrollbar pScrollBar);
XGE_API void xgeXuiScrollBarSetOrientation(xge_xui_scrollbar pScrollBar, int iOrientation);
XGE_API void xgeXuiScrollBarSetColors(xge_xui_scrollbar pScrollBar, uint32_t iTrack, uint32_t iThumb, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);
XGE_API int xgeXuiScrollBarGetState(xge_xui_scrollbar pScrollBar);
XGE_API int xgeXuiScrollBarEvent(xge_xui_scrollbar pScrollBar, const xge_event_t* pEvent);
XGE_API int xgeXuiScrollBarEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiScrollBarPaintProc(xge_xui_widget pWidget, void* pUser);
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
XGE_API void xgeXuiListViewSetEnabledItems(xge_xui_list_view pList, const int* arrEnabled, int iCount);
XGE_API void xgeXuiListViewSetFont(xge_xui_list_view pList, xge_font pFont);
XGE_API void xgeXuiListViewSetItemHeight(xge_xui_list_view pList, float fHeight);
XGE_API void xgeXuiListViewSetSelected(xge_xui_list_view pList, int iIndex);
XGE_API int xgeXuiListViewGetSelected(xge_xui_list_view pList);
XGE_API void xgeXuiListViewSetScroll(xge_xui_list_view pList, float fScrollY);
XGE_API float xgeXuiListViewGetScroll(xge_xui_list_view pList);
XGE_API void xgeXuiListViewSetSelect(xge_xui_list_view pList, xge_xui_select_proc procSelect, void* pUser);
XGE_API void xgeXuiListViewSetColors(xge_xui_list_view pList, uint32_t iBackground, uint32_t iRow, uint32_t iSelected, uint32_t iText, uint32_t iBar, uint32_t iThumb);
XGE_API void xgeXuiListViewSetDisabledTextColor(xge_xui_list_view pList, uint32_t iColor);
XGE_API int xgeXuiListViewEvent(xge_xui_list_view pList, const xge_event_t* pEvent);
XGE_API int xgeXuiListViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiListViewPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiDialogInit(xge_xui_dialog pDialog, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiDialogUnit(xge_xui_dialog pDialog);
XGE_API void xgeXuiDialogSetTitle(xge_xui_dialog pDialog, xge_font pFont, const char* sTitle);
XGE_API void xgeXuiDialogSetClose(xge_xui_dialog pDialog, xge_xui_click_proc procClose, void* pUser);
XGE_API void xgeXuiDialogSetOpen(xge_xui_dialog pDialog, int bOpen);
XGE_API int xgeXuiDialogIsOpen(xge_xui_dialog pDialog);
XGE_API void xgeXuiDialogSetModal(xge_xui_dialog pDialog, int bModal);
XGE_API void xgeXuiDialogSetCloseOnEscape(xge_xui_dialog pDialog, int bEnabled);
XGE_API void xgeXuiDialogSetShowClose(xge_xui_dialog pDialog, int bShow);
XGE_API void xgeXuiDialogSetColors(xge_xui_dialog pDialog, uint32_t iBackdrop, uint32_t iBackground, uint32_t iTitle, uint32_t iClose);
XGE_API int xgeXuiDialogEvent(xge_xui_dialog pDialog, const xge_event_t* pEvent);
XGE_API int xgeXuiDialogEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiDialogPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiPopupInit(xge_xui_popup pPopup, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiPopupUnit(xge_xui_popup pPopup);
XGE_API void xgeXuiPopupSetOwner(xge_xui_popup pPopup, xge_xui_widget pOwner);
XGE_API void xgeXuiPopupSetClose(xge_xui_popup pPopup, xge_xui_click_proc procClose, void* pUser);
XGE_API void xgeXuiPopupSetOpen(xge_xui_popup pPopup, int bOpen);
XGE_API int xgeXuiPopupIsOpen(xge_xui_popup pPopup);
XGE_API void xgeXuiPopupSetAutoClose(xge_xui_popup pPopup, int bOutside, int bEscape);
XGE_API void xgeXuiPopupSetBackground(xge_xui_popup pPopup, uint32_t iColor);
XGE_API int xgeXuiPopupEvent(xge_xui_popup pPopup, const xge_event_t* pEvent);
XGE_API int xgeXuiPopupEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiPopupPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiTooltipInit(xge_xui_tooltip pTooltip, xge_xui_context pContext, xge_xui_widget pOwner);
XGE_API void xgeXuiTooltipUnit(xge_xui_tooltip pTooltip);
XGE_API void xgeXuiTooltipSetText(xge_xui_tooltip pTooltip, xge_font pFont, const char* sText);
XGE_API void xgeXuiTooltipSetColors(xge_xui_tooltip pTooltip, uint32_t iBackground, uint32_t iText);
XGE_API void xgeXuiTooltipSetOffset(xge_xui_tooltip pTooltip, float fX, float fY);
XGE_API void xgeXuiTooltipSetEnabled(xge_xui_tooltip pTooltip, int bEnabled);
XGE_API void xgeXuiTooltipSetOpen(xge_xui_tooltip pTooltip, int bOpen);
XGE_API int xgeXuiTooltipIsOpen(xge_xui_tooltip pTooltip);
XGE_API int xgeXuiTooltipOwnerEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API int xgeXuiComboBoxInit(xge_xui_combo_box pCombo, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiComboBoxUnit(xge_xui_combo_box pCombo);
XGE_API void xgeXuiComboBoxSetItems(xge_xui_combo_box pCombo, const char** arrItems, int iCount);
XGE_API void xgeXuiComboBoxSetFont(xge_xui_combo_box pCombo, xge_font pFont);
XGE_API void xgeXuiComboBoxSetSelect(xge_xui_combo_box pCombo, xge_xui_select_proc procSelect, void* pUser);
XGE_API void xgeXuiComboBoxSetSelected(xge_xui_combo_box pCombo, int iIndex);
XGE_API int xgeXuiComboBoxGetSelected(xge_xui_combo_box pCombo);
XGE_API void xgeXuiComboBoxSetDropDownHeight(xge_xui_combo_box pCombo, float fHeight);
XGE_API void xgeXuiComboBoxSetColors(xge_xui_combo_box pCombo, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iText, uint32_t iPopup);
XGE_API int xgeXuiComboBoxIsOpen(xge_xui_combo_box pCombo);
XGE_API int xgeXuiComboBoxGetState(xge_xui_combo_box pCombo);
XGE_API int xgeXuiComboBoxEvent(xge_xui_combo_box pCombo, const xge_event_t* pEvent);
XGE_API int xgeXuiComboBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
XGE_API void xgeXuiComboBoxPaintProc(xge_xui_widget pWidget, void* pUser);
XGE_API int xgeXuiMenuInit(xge_xui_menu pMenu, xge_xui_context pContext, xge_xui_widget pOwner);
XGE_API void xgeXuiMenuUnit(xge_xui_menu pMenu);
XGE_API void xgeXuiMenuSetItems(xge_xui_menu pMenu, const char** arrItems, int iCount);
XGE_API void xgeXuiMenuSetEnabledItems(xge_xui_menu pMenu, const int* arrEnabled, int iCount);
XGE_API void xgeXuiMenuSetFont(xge_xui_menu pMenu, xge_font pFont);
XGE_API void xgeXuiMenuSetSelect(xge_xui_menu pMenu, xge_xui_select_proc procSelect, void* pUser);
XGE_API void xgeXuiMenuSetSize(xge_xui_menu pMenu, float fWidth, float fMaxHeight);
XGE_API void xgeXuiMenuSetColors(xge_xui_menu pMenu, uint32_t iBackground, uint32_t iRow, uint32_t iSelected, uint32_t iText, uint32_t iDisabledText);
XGE_API void xgeXuiMenuOpen(xge_xui_menu pMenu, float fX, float fY);
XGE_API void xgeXuiMenuClose(xge_xui_menu pMenu);
XGE_API int xgeXuiMenuIsOpen(xge_xui_menu pMenu);

#ifdef __cplusplus
}
#endif

#endif /* XGE_H */
