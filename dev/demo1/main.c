/**
 * @file main.c
 * @brief XGE 游戏引擎主入口
 */

/* ============================================================================
 * 配置宏定义 (必须在包含头文件之前)
 * ============================================================================ */

#define XGE_DEBUG

/* Sokol 实现 */
#define SOKOL_IMPL
#if defined(_WIN32)
	#define SOKOL_WIN32_FORCE_MAIN
#endif

/* XGE GL 加载器实现 */
#define XGE_GL_IMPL

/* ============================================================================
 * 头文件包含
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* XGE API 声明 */
#include "include/xge/xge.h"

/* XGE GL 加载器 */
#include "include/xge/xge_gl.h"

/* 第三方: Sokol App */
#include "lib/sokol/sokol_app.h"

/* ============================================================================
 * 平台相关 GL 加载
 * ============================================================================ */

#if defined(_WIN32)
static HMODULE g_hOpenGL = NULL;

static void* WglGetProcAddress(const char* sName)
{
	void* pProc = (void*)wglGetProcAddress(sName);
	if ( (pProc == NULL) || (pProc == (void*)0x1) || (pProc == (void*)0x2) || (pProc == (void*)0x3) || (pProc == (void*)-1) ) {
		if ( !g_hOpenGL ) {
			g_hOpenGL = LoadLibraryA("opengl32.dll");
		}
		if ( g_hOpenGL ) {
			pProc = (void*)GetProcAddress(g_hOpenGL, sName);
		}
	}
	return pProc;
}
#define XGE_GL_GET_PROC WglGetProcAddress

#elif defined(__linux__)
#include <dlfcn.h>
static void* g_hOpenGL = NULL;

static void* GlxGetProcAddress(const char* sName)
{
	if ( !g_hOpenGL ) {
		g_hOpenGL = dlopen("libGL.so.1", RTLD_LAZY | RTLD_GLOBAL);
		if ( !g_hOpenGL ) {
			g_hOpenGL = dlopen("libGL.so", RTLD_LAZY | RTLD_GLOBAL);
		}
	}
	if ( g_hOpenGL ) {
		return dlsym(g_hOpenGL, sName);
	}
	return NULL;
}
#define XGE_GL_GET_PROC GlxGetProcAddress

#elif defined(__APPLE__)
#include <dlfcn.h>
static void* g_hOpenGL = NULL;

static void* NsglGetProcAddress(const char* sName)
{
	if ( !g_hOpenGL ) {
		g_hOpenGL = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);
	}
	if ( g_hOpenGL ) {
		return dlsym(g_hOpenGL, sName);
	}
	return NULL;
}
#define XGE_GL_GET_PROC NsglGetProcAddress
#endif

/* ============================================================================
 * XGE 模块实现 (包含顺序很重要)
 * ============================================================================ */

/* 核心模块 (定义 g_xge 全局状态) */
#include "include/xge/xge_core.h"

/* 画布模块 (依赖 g_xge) */
#include "include/xge/xge_surface.h"

/* 形状模块 (依赖 g_xge) */
#include "include/xge/xge_shape.h"

/* 输入模块 (依赖 g_xge) */
#include "include/xge/xge_input.h"

/* 场景模块 */
#include "include/xge/xge_scene.h"

/* ============================================================================
 * 测试示例: 形状绘制 + 输入处理 + 场景切换
 * ============================================================================ */

/* 前向声明场景 */
static int SceneMenu(int iMsg, XgeEvent* pEvent);
static int SceneDemo(int iMsg, XgeEvent* pEvent);

/* 可移动方块状态 */
static int s_iBoxX = 100;
static int s_iBoxY = 100;
static int s_iBoxSpeed = 5;

/* ----------------------------------------------------------------------------
 * 场景: 主菜单
 * ---------------------------------------------------------------------------- */
static int SceneMenu(int iMsg, XgeEvent* pEvent)
{
	(void)pEvent;
	
	switch ( iMsg ) {
		case XGE_MSG_INIT:
			XGE_LOG("Scene: Menu Init");
			break;
			
		case XGE_MSG_FRAME:
			/* 按空格进入演示场景 */
			if ( xgeGetKey(XGE_KEY_SPACE) ) {
				xgeSetScene(SceneDemo);
			}
			/* ESC 退出 */
			if ( xgeGetKey(XGE_KEY_ESCAPE) ) {
				xgeQuit();
			}
			break;
			
		case XGE_MSG_DRAW:
			{
				int iW = xgeGetWidth();
				int iH = xgeGetHeight();
				
				/* 清屏 */
				xgeClear(XGE_RGB(32, 32, 64));
				
				/* 标题 */
				xgeBoxFill(iW / 2 - 150, 50, 300, 60, XGE_COLOR_WHITE);
				xgeBox(iW / 2 - 150, 50, 300, 60, XGE_COLOR_CYAN);
				
				/* 装饰图形 */
				xgeCircleFill(iW / 2, iH / 2, 80, XGE_RGBA(255, 100, 100, 200));
				xgeCircle(iW / 2, iH / 2, 100, XGE_COLOR_YELLOW);
				xgeCircle(iW / 2, iH / 2, 120, XGE_COLOR_GREEN);
				
				/* 提示框 */
				xgeBoxFill(iW / 2 - 120, iH - 100, 240, 40, XGE_RGB(64, 64, 64));
				xgeBox(iW / 2 - 120, iH - 100, 240, 40, XGE_COLOR_WHITE);
				
				/* 三角形装饰 */
				xgeTriangleFill(50, 150, 100, 250, 150, 150, XGE_COLOR_RED);
				xgeTriangleFill(iW - 50, 150, iW - 100, 250, iW - 150, 150, XGE_COLOR_BLUE);
			}
			break;
			
		case XGE_MSG_EXIT:
			XGE_LOG("Scene: Menu Exit");
			break;
	}
	
	return 0;
}

/* ----------------------------------------------------------------------------
 * 场景: 演示 (可移动方块 + 绘图测试)
 * ---------------------------------------------------------------------------- */
static int SceneDemo(int iMsg, XgeEvent* pEvent)
{
	switch ( iMsg ) {
		case XGE_MSG_INIT:
			XGE_LOG("Scene: Demo Init");
			s_iBoxX = xgeGetWidth() / 2 - 25;
			s_iBoxY = xgeGetHeight() / 2 - 25;
			break;
			
		case XGE_MSG_FRAME:
			{
				/* 方向键移动 */
				if ( xgeGetKey(XGE_KEY_UP) || xgeGetKey(XGE_KEY_W) ) {
					s_iBoxY -= s_iBoxSpeed;
				}
				if ( xgeGetKey(XGE_KEY_DOWN) || xgeGetKey(XGE_KEY_S) ) {
					s_iBoxY += s_iBoxSpeed;
				}
				if ( xgeGetKey(XGE_KEY_LEFT) || xgeGetKey(XGE_KEY_A) ) {
					s_iBoxX -= s_iBoxSpeed;
				}
				if ( xgeGetKey(XGE_KEY_RIGHT) || xgeGetKey(XGE_KEY_D) ) {
					s_iBoxX += s_iBoxSpeed;
				}
				
				/* 边界检查 */
				if ( s_iBoxX < 0 ) s_iBoxX = 0;
				if ( s_iBoxY < 0 ) s_iBoxY = 0;
				if ( s_iBoxX > xgeGetWidth() - 50 ) s_iBoxX = xgeGetWidth() - 50;
				if ( s_iBoxY > xgeGetHeight() - 50 ) s_iBoxY = xgeGetHeight() - 50;
				
				/* 返回菜单 */
				if ( xgeGetKey(XGE_KEY_ESCAPE) ) {
					xgeSetScene(SceneMenu);
				}
			}
			break;
			
		case XGE_MSG_DRAW:
			{
				int iW = xgeGetWidth();
				int iH = xgeGetHeight();
				int iMouseX, iMouseY;
				
				xgeGetMouse(&iMouseX, &iMouseY);
				
				/* 清屏 - 深色背景 */
				xgeClear(XGE_RGB(24, 24, 32));
				
				/* 网格背景 */
				int i;
				for ( i = 0; i < iW; i += 50 ) {
					xgeLine(i, 0, i, iH, XGE_RGB(40, 40, 50));
				}
				for ( i = 0; i < iH; i += 50 ) {
					xgeLine(0, i, iW, i, XGE_RGB(40, 40, 50));
				}
				
				/* 左上角: 基本图形测试 */
				xgePset(20, 20, XGE_COLOR_WHITE);
				xgePset(22, 20, XGE_COLOR_WHITE);
				xgePset(24, 20, XGE_COLOR_WHITE);
				xgeLine(20, 30, 120, 30, XGE_COLOR_RED);
				xgeLine(20, 35, 120, 80, XGE_COLOR_GREEN);
				
				/* 矩形测试 */
				xgeBox(20, 100, 80, 60, XGE_COLOR_CYAN);
				xgeBoxFill(30, 110, 60, 40, XGE_RGBA(255, 255, 0, 128));
				
				/* 圆形测试 */
				xgeCircle(200, 100, 40, XGE_COLOR_MAGENTA);
				xgeCircleFill(200, 100, 25, XGE_RGBA(0, 255, 255, 150));
				
				/* 三角形测试 */
				xgeTriangle(300, 60, 340, 140, 260, 140, XGE_COLOR_YELLOW);
				xgeTriangleFill(300, 80, 330, 130, 270, 130, XGE_RGBA(255, 128, 0, 200));
				
				/* 可移动方块 */
				xgeBoxFill(s_iBoxX, s_iBoxY, 50, 50, XGE_COLOR_GREEN);
				xgeBox(s_iBoxX, s_iBoxY, 50, 50, XGE_COLOR_WHITE);
				
				/* 鼠标跟踪圆 */
				xgeCircleFill(iMouseX, iMouseY, 10, XGE_RGBA(255, 255, 255, 100));
				xgeCircle(iMouseX, iMouseY, 15, XGE_COLOR_WHITE);
				
				/* 鼠标按钮指示 */
				if ( xgeGetMouseButton(XGE_MOUSE_LEFT) ) {
					xgeCircleFill(iMouseX, iMouseY, 20, XGE_RGBA(255, 0, 0, 150));
				}
				if ( xgeGetMouseButton(XGE_MOUSE_RIGHT) ) {
					xgeCircleFill(iMouseX, iMouseY, 25, XGE_RGBA(0, 0, 255, 150));
				}
				
				/* FPS 显示区域 */
				xgeBoxFill(iW - 100, 10, 90, 25, XGE_RGB(0, 0, 0));
				xgeBox(iW - 100, 10, 90, 25, XGE_COLOR_GREEN);
				
				/* 控制提示区域 */
				xgeBoxFill(10, iH - 40, 200, 30, XGE_RGBA(0, 0, 0, 180));
				xgeBox(10, iH - 40, 200, 30, XGE_COLOR_GRAY);
			}
			break;
			
		case XGE_MSG_MOUSE_DOWN:
			XGE_LOG("Mouse Down: %d, %d (Button: %d)", 
				pEvent->mouse.iX, pEvent->mouse.iY, pEvent->mouse.iButton);
			break;
			
		case XGE_MSG_EXIT:
			XGE_LOG("Scene: Demo Exit");
			break;
	}
	
	return 0;
}

/* ============================================================================
 * Sokol 入口点
 * ============================================================================ */

sapp_desc sokol_main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
	
	/* 设置初始场景 */
	xgeRun(SceneMenu);
	
	return (sapp_desc){
		.init_cb = xge_OnInit,
		.frame_cb = xge_OnFrame,
		.cleanup_cb = xge_OnCleanup,
		.event_cb = xge_OnEvent,
		.width = 800,
		.height = 600,
		.window_title = "XGE Engine v2.0 - Demo",
		.icon.sokol_default = true,
		.sample_count = 4,
		.high_dpi = true,
	};
}
