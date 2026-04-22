
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
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
#ifndef GL_MAX_TEXTURE_SIZE
	#define GL_MAX_TEXTURE_SIZE 0x0D33
#endif

#define XGE_RESOURCE_PROVIDER_MAX	8
#define XGE_XPACK_PROVIDER_MAX		4

#define XGE_RENDER_COMMAND_DRAW		1

typedef struct xge_texture_upload_node_t {
	xge_texture pTexture;
	struct xge_texture_upload_node_t* pNext;
} xge_texture_upload_node_t;

typedef struct xge_render_command_t {
	int iType;
	union {
		xge_draw_t tDraw;
	} u;
} xge_render_command_t;

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
	int iFrameCount;
	int iBlend;
	int bDepthTestEnabled;
	uint32_t iCurrentFramebufferId;
	int iTextureCount;
	uint64_t iTextureMemoryBytes;
	int iFontCount;
	int iAudioCount;
	int iLastGLError;
	float fDelta;
	double fStartTime;
	uint32_t iClearColor;
	xge_frame_stats_t tFrameStats;
	xge_desc_t objDesc;
	xge_scene_proc procFrame;
	void* pFrameUser;
	xge_scene arrSceneStack[XGE_SCENE_STACK_MAX];
	int iSceneStackCount;
	xge_platform_backend_t tPlatformBackend;
	xge_graphics_backend_t tGraphicsBackend;
	xge_gpu_caps_t tGpuCaps;
	xge_miniprogram_desc_t tMiniProgramDesc;
	xge_miniprogram_bridge_t tMiniProgramBridge;
	int bMiniProgramInitialized;
	int iSceneUpdateMode;
	int iSceneMaxUpdates;
	float fSceneFixedStep;
	float fSceneAccumulator;
	xge_texture_t tFallbackTexture;
	xge_font_t tFallbackFont;
	char* sFallbackSoundPath;
	xge_texture_upload_node_t* pTextureUploadHead;
	xge_texture_upload_node_t* pTextureUploadTail;
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
	xge_gamepad_state_t arrGamepads[XGE_GAMEPAD_MAX];
	int bClipEnabled;
	xge_rect_t tClipRect;
	int bViewportEnabled;
	xge_rect_t tViewportRect;
	xge_rect_t arrDirtyRects[XGE_DIRTY_RECT_MAX];
	int iDirtyRectCount;
	int bRenderActive;
	xge_render_command_t* pRenderCommands;
	int iRenderCommandCount;
	int iRenderCommandCapacity;
	volatile int iRenderCommandLock;
	xge_camera_t tCamera;
	xge_resource_provider_t arrResourceProviders[XGE_RESOURCE_PROVIDER_MAX];
	int iResourceProviderCount;
	xge_xpack_provider_t arrXPackProviders[XGE_XPACK_PROVIDER_MAX];
	int iXPackProviderCount;
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

// 2.5D 透视四边形渲染器
typedef struct xge_quad3d_renderer_t {
	int bInitialized;
	GLuint iProgram;
	GLuint iVAO;
	GLuint iVBO;
	GLint iLocResolution;
	GLint iLocTexture;
} xge_quad3d_renderer_t;

static xge_quad3d_renderer_t g_xgeQuad3DRenderer;

// MVP Shape 渲染器
typedef struct xge_shape_renderer_t {
	int bInitialized;
	GLuint iProgram;
	GLuint iVAO;
	GLuint iVBO;
	GLint iLocResolution;
	GLint iLocColor;
} xge_shape_renderer_t;

static xge_shape_renderer_t g_xgeShapeRenderer;

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
static int __xgeSceneFrame(void);
static void __xgeFrameStatsRecordTime(double fSeconds);
static void __xgeCameraProjectVertex(float fX, float fY, float fZ, uint32_t iFlags, float* pOutX, float* pOutY);
static void __xgeRenderCommandReset(void);
static void __xgeRenderCommandUnit(void);
static int __xgeRenderCommandFlush(void);
static int __xgeRenderCommandDraw(const xge_draw_t* pDraw);
static void __xgeDrawExImmediate(const xge_draw_t* pDraw);

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
static int g_xgeOpenGLLibraryBackend = XGE_GPU_BACKEND_NONE;

static const char* __xgeGraphicsLibraryName(int iBackend, int iIndex)
{
	if ( (iBackend == XGE_GPU_BACKEND_GLES30) || (iBackend == XGE_GPU_BACKEND_WEBGL2) ) {
		if ( iIndex == 0 ) {
			return "libGLESv2.dll";
		}
		if ( iIndex == 1 ) {
			return "GLESv2.dll";
		}
		return NULL;
	}
	if ( iIndex == 0 ) {
		return "opengl32.dll";
	}
	return NULL;
}

// windows OpenGL 函数加载
static void* __xgeGLGetProc(const char* sName)
{
	void* pProc;
	int iBackend;
	int i;
	const char* sLibrary;

	iBackend = xgeGraphicsBackendGet().iType;
	pProc = NULL;
	if ( iBackend == XGE_GPU_BACKEND_OPENGL33 ) {
		pProc = (void*)wglGetProcAddress(sName);
	}
	if ( (pProc == NULL) || (pProc == (void*)0x1) || (pProc == (void*)0x2) || (pProc == (void*)0x3) || (pProc == (void*)-1) ) {
		if ( g_xgeOpenGLLibraryBackend != iBackend ) {
			g_xgeOpenGL = NULL;
			g_xgeOpenGLLibraryBackend = iBackend;
		}
		if ( g_xgeOpenGL == NULL ) {
			for ( i = 0; (sLibrary = __xgeGraphicsLibraryName(iBackend, i)) != NULL; i++ ) {
				g_xgeOpenGL = LoadLibraryA(sLibrary);
				if ( g_xgeOpenGL != NULL ) {
					break;
				}
			}
		}
		if ( g_xgeOpenGL != NULL ) {
			pProc = (void*)GetProcAddress(g_xgeOpenGL, sName);
		}
	}
	return pProc;
}
#elif defined(__APPLE__)
static void* g_xgeOpenGL = NULL;
static int g_xgeOpenGLLibraryBackend = XGE_GPU_BACKEND_NONE;

static const char* __xgeGraphicsLibraryName(int iBackend, int iIndex)
{
	if ( (iBackend == XGE_GPU_BACKEND_GLES30) || (iBackend == XGE_GPU_BACKEND_WEBGL2) ) {
		if ( iIndex == 0 ) {
			return "/System/Library/Frameworks/OpenGLES.framework/Versions/Current/OpenGLES";
		}
		return NULL;
	}
	if ( iIndex == 0 ) {
		return "/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL";
	}
	return NULL;
}

// macOS OpenGL 函数加载
static void* __xgeGLGetProc(const char* sName)
{
	int iBackend;
	int i;
	const char* sLibrary;

	iBackend = xgeGraphicsBackendGet().iType;
	if ( g_xgeOpenGLLibraryBackend != iBackend ) {
		g_xgeOpenGL = NULL;
		g_xgeOpenGLLibraryBackend = iBackend;
	}
	if ( g_xgeOpenGL == NULL ) {
		for ( i = 0; (sLibrary = __xgeGraphicsLibraryName(iBackend, i)) != NULL; i++ ) {
			g_xgeOpenGL = dlopen(sLibrary, RTLD_LAZY);
			if ( g_xgeOpenGL != NULL ) {
				break;
			}
		}
	}
	if ( g_xgeOpenGL != NULL ) {
		return dlsym(g_xgeOpenGL, sName);
	}
	return NULL;
}
#else
static void* g_xgeOpenGL = NULL;
static int g_xgeOpenGLLibraryBackend = XGE_GPU_BACKEND_NONE;

static const char* __xgeGraphicsLibraryName(int iBackend, int iIndex)
{
	if ( (iBackend == XGE_GPU_BACKEND_GLES30) || (iBackend == XGE_GPU_BACKEND_WEBGL2) ) {
		if ( iIndex == 0 ) {
			return "libGLESv2.so.2";
		}
		if ( iIndex == 1 ) {
			return "libGLESv2.so";
		}
		return NULL;
	}
	if ( iIndex == 0 ) {
		return "libGL.so.1";
	}
	if ( iIndex == 1 ) {
		return "libGL.so";
	}
	return NULL;
}

// Linux OpenGL 函数加载
static void* __xgeGLGetProc(const char* sName)
{
	int iBackend;
	int i;
	const char* sLibrary;

	iBackend = xgeGraphicsBackendGet().iType;
	if ( g_xgeOpenGLLibraryBackend != iBackend ) {
		g_xgeOpenGL = NULL;
		g_xgeOpenGLLibraryBackend = iBackend;
	}
	if ( g_xgeOpenGL == NULL ) {
		for ( i = 0; (sLibrary = __xgeGraphicsLibraryName(iBackend, i)) != NULL; i++ ) {
			g_xgeOpenGL = dlopen(sLibrary, RTLD_LAZY | RTLD_GLOBAL);
			if ( g_xgeOpenGL != NULL ) {
				break;
			}
		}
	}
	if ( g_xgeOpenGL != NULL ) {
		return dlsym(g_xgeOpenGL, sName);
	}
	return NULL;
}
#endif

// 编译 shader
static int __xgeGraphicsShaderHeaderGet(int iBackend, char* sBuffer, int iSize)
{
	const char* sHeader;

	if ( (sBuffer == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iBackend == XGE_GPU_BACKEND_NONE ) {
		iBackend = xgeGraphicsBackendGet().iType;
	}
	if ( (iBackend == XGE_GPU_BACKEND_GLES30) || (iBackend == XGE_GPU_BACKEND_WEBGL2) ) {
		sHeader = "#version 300 es\nprecision mediump float;\nprecision mediump sampler2D;\n";
	} else {
		sHeader = "#version 330 core\n";
	}
	return snprintf(sBuffer, (size_t)iSize, "%s", sHeader);
}

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
	char sHeader[128];
	char sVS[1024];
	char sFS[1024];

	if ( g_xgeTextureRenderer.bInitialized ) {
		return XGE_OK;
	}
	if ( __xgeGraphicsShaderHeaderGet(XGE_GPU_BACKEND_NONE, sHeader, (int)sizeof(sHeader)) < 0 ) {
		return XGE_ERROR_GPU_FAILED;
	}
	snprintf(sVS, sizeof(sVS),
		"%s"
		"layout (location = 0) in vec2 aPos;\n"
		"layout (location = 1) in vec2 aUV;\n"
		"uniform vec2 uResolution;\n"
		"out vec2 vUV;\n"
		"void main() {\n"
		"	vec2 pos = (aPos / uResolution) * 2.0 - 1.0;\n"
		"	pos.y = -pos.y;\n"
		"	gl_Position = vec4(pos, 0.0, 1.0);\n"
		"	vUV = aUV;\n"
		"}\n",
		sHeader);
	snprintf(sFS, sizeof(sFS),
		"%s"
		"in vec2 vUV;\n"
		"uniform vec4 uColor;\n"
		"uniform sampler2D uTexture;\n"
		"out vec4 FragColor;\n"
		"void main() {\n"
		"	FragColor = texture(uTexture, vUV) * uColor;\n"
		"}\n",
		sHeader);

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

// 初始化 MVP Shape 渲染器
static int __xgeShapeRendererInit(void)
{
	GLuint iVS;
	GLuint iFS;
	GLint bSuccess;
	char arrLog[512];
	char sHeader[128];
	char sVS[1024];
	char sFS[1024];

	if ( g_xgeShapeRenderer.bInitialized ) {
		return XGE_OK;
	}
	if ( __xgeGraphicsShaderHeaderGet(XGE_GPU_BACKEND_NONE, sHeader, (int)sizeof(sHeader)) < 0 ) {
		return XGE_ERROR_GPU_FAILED;
	}
	snprintf(sVS, sizeof(sVS),
		"%s"
		"layout (location = 0) in vec2 aPos;\n"
		"uniform vec2 uResolution;\n"
		"void main() {\n"
		"	vec2 pos = (aPos / uResolution) * 2.0 - 1.0;\n"
		"	pos.y = -pos.y;\n"
		"	gl_Position = vec4(pos, 0.0, 1.0);\n"
		"}\n",
		sHeader);
	snprintf(sFS, sizeof(sFS),
		"%s"
		"uniform vec4 uColor;\n"
		"out vec4 FragColor;\n"
		"void main() {\n"
		"	FragColor = uColor;\n"
		"}\n",
		sHeader);

	iVS = __xgeCompileShader(GL_VERTEX_SHADER, sVS);
	iFS = __xgeCompileShader(GL_FRAGMENT_SHADER, sFS);
	if ( (iVS == 0) || (iFS == 0) ) {
		return XGE_ERROR_GPU_FAILED;
	}

	g_xgeShapeRenderer.iProgram = glCreateProgram();
	glAttachShader(g_xgeShapeRenderer.iProgram, iVS);
	glAttachShader(g_xgeShapeRenderer.iProgram, iFS);
	glLinkProgram(g_xgeShapeRenderer.iProgram);
	glGetProgramiv(g_xgeShapeRenderer.iProgram, GL_LINK_STATUS, &bSuccess);
	glDeleteShader(iVS);
	glDeleteShader(iFS);
	if ( bSuccess == 0 ) {
		glGetProgramInfoLog(g_xgeShapeRenderer.iProgram, 512, NULL, arrLog);
		xrtSetError(arrLog, false);
		return XGE_ERROR_GPU_FAILED;
	}

	g_xgeShapeRenderer.iLocResolution = glGetUniformLocation(g_xgeShapeRenderer.iProgram, "uResolution");
	g_xgeShapeRenderer.iLocColor = glGetUniformLocation(g_xgeShapeRenderer.iProgram, "uColor");
	glGenVertexArrays(1, &g_xgeShapeRenderer.iVAO);
	glGenBuffers(1, &g_xgeShapeRenderer.iVBO);
	glBindVertexArray(g_xgeShapeRenderer.iVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_xgeShapeRenderer.iVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	g_xgeShapeRenderer.bInitialized = 1;
	return XGE_OK;
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

static void __xgeDepthTestApply(void)
{
	if ( g_xge.bSokolRunning == 0 ) {
		return;
	}
	if ( g_xge.bDepthTestEnabled ) {
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}
}

// 初始化 2.5D 透视四边形渲染器
static int __xgeQuad3DRendererInit(void)
{
	GLuint iVS;
	GLuint iFS;
	GLint bSuccess;
	char arrLog[512];
	char sHeader[128];
	char sVS[1400];
	char sFS[1024];

	if ( g_xgeQuad3DRenderer.bInitialized ) {
		return XGE_OK;
	}
	if ( __xgeGraphicsShaderHeaderGet(XGE_GPU_BACKEND_NONE, sHeader, (int)sizeof(sHeader)) < 0 ) {
		return XGE_ERROR_GPU_FAILED;
	}
	snprintf(sVS, sizeof(sVS),
		"%s"
		"layout (location = 0) in vec4 aPos;\n"
		"layout (location = 1) in vec2 aUV;\n"
		"layout (location = 2) in vec4 aColor;\n"
		"uniform vec2 uResolution;\n"
		"out vec2 vUV;\n"
		"out vec4 vColor;\n"
		"void main() {\n"
		"	vec2 pos = (aPos.xy / uResolution) * 2.0 - 1.0;\n"
		"	pos.y = -pos.y;\n"
		"	gl_Position = vec4(pos * aPos.w, aPos.z * aPos.w, aPos.w);\n"
		"	vUV = aUV;\n"
		"	vColor = aColor;\n"
		"}\n",
		sHeader);
	snprintf(sFS, sizeof(sFS),
		"%s"
		"in vec2 vUV;\n"
		"in vec4 vColor;\n"
		"uniform sampler2D uTexture;\n"
		"out vec4 FragColor;\n"
		"void main() {\n"
		"	FragColor = texture(uTexture, vUV) * vColor;\n"
		"}\n",
		sHeader);

	iVS = __xgeCompileShader(GL_VERTEX_SHADER, sVS);
	iFS = __xgeCompileShader(GL_FRAGMENT_SHADER, sFS);
	if ( (iVS == 0) || (iFS == 0) ) {
		return XGE_ERROR_GPU_FAILED;
	}

	g_xgeQuad3DRenderer.iProgram = glCreateProgram();
	glAttachShader(g_xgeQuad3DRenderer.iProgram, iVS);
	glAttachShader(g_xgeQuad3DRenderer.iProgram, iFS);
	glLinkProgram(g_xgeQuad3DRenderer.iProgram);
	glGetProgramiv(g_xgeQuad3DRenderer.iProgram, GL_LINK_STATUS, &bSuccess);
	glDeleteShader(iVS);
	glDeleteShader(iFS);
	if ( bSuccess == 0 ) {
		glGetProgramInfoLog(g_xgeQuad3DRenderer.iProgram, 512, NULL, arrLog);
		xrtSetError(arrLog, false);
		return XGE_ERROR_GPU_FAILED;
	}

	g_xgeQuad3DRenderer.iLocResolution = glGetUniformLocation(g_xgeQuad3DRenderer.iProgram, "uResolution");
	g_xgeQuad3DRenderer.iLocTexture = glGetUniformLocation(g_xgeQuad3DRenderer.iProgram, "uTexture");
	glGenVertexArrays(1, &g_xgeQuad3DRenderer.iVAO);
	glGenBuffers(1, &g_xgeQuad3DRenderer.iVBO);
	glBindVertexArray(g_xgeQuad3DRenderer.iVAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_xgeQuad3DRenderer.iVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 40, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(4 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	g_xgeQuad3DRenderer.bInitialized = 1;
	return XGE_OK;
}

static void __xgeTouchResetStationary(void);
static void __xgeTouchRemoveEnded(void);

// 每帧开始时清理瞬时输入状态
static void __xgeInputBeginFrame(void)
{
	int i;

	memset(g_xge.arrKeyPressed, 0, sizeof(g_xge.arrKeyPressed));
	memset(g_xge.arrKeyReleased, 0, sizeof(g_xge.arrKeyReleased));
	g_xge.fMouseDX = 0.0f;
	g_xge.fMouseDY = 0.0f;
	g_xge.fMouseWheelX = 0.0f;
	g_xge.fMouseWheelY = 0.0f;
	g_xge.iTextCodepoint = 0;
	__xgeTouchRemoveEnded();
	__xgeTouchResetStationary();
	for ( i = 0; i < XGE_GAMEPAD_MAX; i++ ) {
		g_xge.arrGamepads[i].iButtonsPressed = 0;
		g_xge.arrGamepads[i].iButtonsReleased = 0;
	}
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
	__xgeDepthTestApply();
}

// Sokol 帧回调
static void __xgeSokolFrame(void)
{
	float fR;
	float fG;
	float fB;
	float fA;
	double fFrameStart;

	if ( g_xge.bRunning == 0 ) {
		sapp_quit();
		return;
	}

	fFrameStart = xrtTimer();
	__xgeRenderCommandReset();
	g_xge.fDelta = (float)sapp_frame_duration();
	g_xge.iFrameCount++;
	g_xge.tFrameStats.iFrameCount++;
	g_xge.iWidth = sapp_width();
	g_xge.iHeight = sapp_height();
	g_xge.tCamera.tViewport.fW = (float)g_xge.iWidth;
	g_xge.tCamera.tViewport.fH = (float)g_xge.iHeight;

	__xgeColorToFloat(g_xge.iClearColor, &fR, &fG, &fB, &fA);
	glViewport(0, 0, g_xge.iWidth, g_xge.iHeight);
	glClearColor(fR, fG, fB, fA);
	glClear(GL_COLOR_BUFFER_BIT);
	xgeTextureUploadFlush();

	if ( g_xge.procFrame != NULL ) {
		if ( g_xge.procFrame(g_xge.pFrameUser) != 0 ) {
			xgeQuit();
		}
	} else {
		if ( __xgeSceneFrame() != 0 ) {
			xgeQuit();
		}
	}

	__xgeInputBeginFrame();
	__xgeFrameStatsRecordTime(xrtTimer() - fFrameStart);
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

static void __xgeSokolDispatchSceneEvent(const sapp_event* pEvent)
{
	xge_event_t tEvent;
	xge_touch_event_t tTouch;
	int i;

	memset(&tEvent, 0, sizeof(tEvent));
	memset(&tTouch, 0, sizeof(tTouch));
	switch ( pEvent->type ) {
		case SAPP_EVENTTYPE_KEY_DOWN:
			tEvent.iType = XGE_EVENT_KEY_DOWN;
			tEvent.iParam1 = (int)pEvent->key_code;
			break;

		case SAPP_EVENTTYPE_KEY_UP:
			tEvent.iType = XGE_EVENT_KEY_UP;
			tEvent.iParam1 = (int)pEvent->key_code;
			break;

		case SAPP_EVENTTYPE_CHAR:
			tEvent.iType = XGE_EVENT_TEXT;
			tEvent.iCodepoint = pEvent->char_code;
			break;

		case SAPP_EVENTTYPE_MOUSE_MOVE:
			tEvent.iType = XGE_EVENT_MOUSE_MOVE;
			tEvent.fX = pEvent->mouse_x;
			tEvent.fY = pEvent->mouse_y;
			tEvent.fDX = pEvent->mouse_dx;
			tEvent.fDY = pEvent->mouse_dy;
			break;

		case SAPP_EVENTTYPE_MOUSE_SCROLL:
			tEvent.iType = XGE_EVENT_MOUSE_WHEEL;
			tEvent.fDX = pEvent->scroll_x;
			tEvent.fDY = pEvent->scroll_y;
			break;

		case SAPP_EVENTTYPE_MOUSE_DOWN:
			tEvent.iType = XGE_EVENT_MOUSE_DOWN;
			tEvent.iParam1 = (int)__xgeMouseButtonMask(pEvent->mouse_button);
			tEvent.fX = pEvent->mouse_x;
			tEvent.fY = pEvent->mouse_y;
			break;

		case SAPP_EVENTTYPE_MOUSE_UP:
			tEvent.iType = XGE_EVENT_MOUSE_UP;
			tEvent.iParam1 = (int)__xgeMouseButtonMask(pEvent->mouse_button);
			tEvent.fX = pEvent->mouse_x;
			tEvent.fY = pEvent->mouse_y;
			break;

		case SAPP_EVENTTYPE_TOUCHES_BEGAN:
		case SAPP_EVENTTYPE_TOUCHES_MOVED:
		case SAPP_EVENTTYPE_TOUCHES_ENDED:
		case SAPP_EVENTTYPE_TOUCHES_CANCELLED:
			if ( pEvent->type == SAPP_EVENTTYPE_TOUCHES_BEGAN ) {
				tEvent.iType = XGE_EVENT_TOUCH_BEGIN;
				tTouch.iPhase = XGE_TOUCH_BEGIN;
			} else if ( pEvent->type == SAPP_EVENTTYPE_TOUCHES_MOVED ) {
				tEvent.iType = XGE_EVENT_TOUCH_MOVE;
				tTouch.iPhase = XGE_TOUCH_MOVE;
			} else if ( pEvent->type == SAPP_EVENTTYPE_TOUCHES_ENDED ) {
				tEvent.iType = XGE_EVENT_TOUCH_END;
				tTouch.iPhase = XGE_TOUCH_END;
			} else {
				tEvent.iType = XGE_EVENT_TOUCH_CANCEL;
				tTouch.iPhase = XGE_TOUCH_CANCEL;
			}
			tTouch.iCount = g_xge.iTouchCount;
			if ( tTouch.iCount > XGE_TOUCH_MAX ) {
				tTouch.iCount = XGE_TOUCH_MAX;
			}
			for ( i = 0; i < tTouch.iCount; i++ ) {
				tTouch.arrPoints[i] = g_xge.arrTouches[i];
			}
			tEvent.pData = &tTouch;
			break;

		case SAPP_EVENTTYPE_RESIZED:
			tEvent.iType = XGE_EVENT_RESIZE;
			tEvent.iParam1 = pEvent->window_width;
			tEvent.iParam2 = pEvent->window_height;
			break;

		case SAPP_EVENTTYPE_QUIT_REQUESTED:
			tEvent.iType = XGE_EVENT_QUIT;
			break;

		default:
			return;
	}
	if ( xgeSceneDispatchEvent(&tEvent) != 0 ) {
		xgeQuit();
	}
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
	__xgeSokolDispatchSceneEvent(pEvent);
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


#include "xge_core.c"
#include "xge_resource.c"
#include "xge_miniprogram.c"
#include "xge_egl.c"
#include "xge_audio.c"
#include "xge_font.c"
#include "xge_texture.c"
#include "xge_command.c"
#include "xge_render.c"
#include "xge_shape.c"
#include "xge_mesh.c"
#include "xge_sprite.c"
#include "xge_material.c"
#include "xge_render_target.c"
#include "xge_buffer.c"
#include "xge_async.c"
#include "xge_input.c"
#include "xge_xui_host.c"
#include "xge_xui_layout.c"
#include "xge_xui_core.c"
#include "xge_xui_text.c"
#include "xge_xui_controls.c"
#include "xge_xui_controls_extra.c"
#include "xge_xui_scroll.c"
#include "xge_xui.c"
