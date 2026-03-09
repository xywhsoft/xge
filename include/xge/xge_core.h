/**
 * @file xge_core.h
 * @brief XGE 核心模块实现（初始化/主循环/帧控制）
 */

#ifndef XGE_CORE_H
#define XGE_CORE_H

/* ============================================================================
 * 全局状态结构
 * ============================================================================ */

typedef struct XgeState {
	/* 窗口状态 */
	int iWidth;
	int iHeight;
	int iFlags;
	int bRunning;
	int bInitialized;
	
	/* 帧控制 */
	int iTargetFPS;
	int iCurrentFPS;
	float fDeltaTime;
	float fFrameTime;
	double dLastTime;
	int iFrameCount;
	double dFPSTime;
	
	/* 场景 */
	XgeSceneProc procCurrentScene;
	XgeSceneProc arrSceneStack[32];
	int iSceneStackTop;
	XgeSceneProc procPendingScene;
	int iPendingAction;  /* 0=none, 1=set, 2=push, 3=pop */
	
	/* 输入状态 */
	int arrKeyState[512];
	int iMouseX;
	int iMouseY;
	int iMouseButtons;
	float fMouseWheel;
	
	/* 绘制状态 */
	XgeSurface* pCurrentTarget;
	
} XgeState;

static XgeState g_xge = {0};

/* ============================================================================
 * 前向声明
 * ============================================================================ */

static void xge_ProcessPendingScene(void);

/* ============================================================================
 * 核心 API 实现
 * ============================================================================ */

int xgeInit(int iWidth, int iHeight, const char* sTitle, int iFlags)
{
	(void)sTitle;
	(void)iFlags;
	
	g_xge.iWidth = iWidth;
	g_xge.iHeight = iHeight;
	g_xge.iFlags = iFlags;
	g_xge.bRunning = XGE_TRUE;
	g_xge.bInitialized = XGE_TRUE;
	g_xge.iTargetFPS = 60;
	g_xge.iCurrentFPS = 0;
	g_xge.fDeltaTime = 1.0f / 60.0f;
	g_xge.iSceneStackTop = -1;
	g_xge.procCurrentScene = NULL;
	g_xge.iPendingAction = 0;
	
	return XGE_TRUE;
}

void xgeExit(void)
{
	g_xge.bRunning = XGE_FALSE;
}

void xgeQuit(void)
{
	g_xge.bRunning = XGE_FALSE;
	sapp_quit();
}

void xgeSetFPS(int iFPS)
{
	g_xge.iTargetFPS = iFPS;
	if ( iFPS > 0 ) {
		g_xge.fFrameTime = 1.0f / (float)iFPS;
	} else {
		g_xge.fFrameTime = 0.0f;
	}
}

int xgeGetFPS(void)
{
	return g_xge.iCurrentFPS;
}

float xgeGetDelta(void)
{
	return g_xge.fDeltaTime;
}

int xgeGetWidth(void)
{
	return g_xge.iWidth;
}

int xgeGetHeight(void)
{
	return g_xge.iHeight;
}

/* ============================================================================
 * 场景 API 实现
 * ============================================================================ */

void xgeSetScene(XgeSceneProc procScene)
{
	g_xge.procPendingScene = procScene;
	g_xge.iPendingAction = 1;
}

void xgePushScene(XgeSceneProc procScene)
{
	g_xge.procPendingScene = procScene;
	g_xge.iPendingAction = 2;
}

void xgePopScene(void)
{
	g_xge.procPendingScene = NULL;
	g_xge.iPendingAction = 3;
}

static void xge_ProcessPendingScene(void)
{
	XgeEvent evt = {0};
	
	if ( g_xge.iPendingAction == 0 ) {
		return;
	}
	
	switch ( g_xge.iPendingAction ) {
		case 1: /* set */
			/* 退出当前场景 */
			if ( g_xge.procCurrentScene ) {
				evt.iType = XGE_MSG_EXIT;
				g_xge.procCurrentScene(XGE_MSG_EXIT, &evt);
			}
			/* 设置新场景 */
			g_xge.procCurrentScene = g_xge.procPendingScene;
			/* 初始化新场景 */
			if ( g_xge.procCurrentScene ) {
				evt.iType = XGE_MSG_INIT;
				g_xge.procCurrentScene(XGE_MSG_INIT, &evt);
			}
			break;
			
		case 2: /* push */
			/* 保存当前场景到栈 */
			if ( g_xge.iSceneStackTop < 31 ) {
				g_xge.iSceneStackTop++;
				g_xge.arrSceneStack[g_xge.iSceneStackTop] = g_xge.procCurrentScene;
			}
			/* 设置新场景 */
			g_xge.procCurrentScene = g_xge.procPendingScene;
			/* 初始化新场景 */
			if ( g_xge.procCurrentScene ) {
				evt.iType = XGE_MSG_INIT;
				g_xge.procCurrentScene(XGE_MSG_INIT, &evt);
			}
			break;
			
		case 3: /* pop */
			/* 退出当前场景 */
			if ( g_xge.procCurrentScene ) {
				evt.iType = XGE_MSG_EXIT;
				g_xge.procCurrentScene(XGE_MSG_EXIT, &evt);
			}
			/* 从栈恢复场景 */
			if ( g_xge.iSceneStackTop >= 0 ) {
				g_xge.procCurrentScene = g_xge.arrSceneStack[g_xge.iSceneStackTop];
				g_xge.iSceneStackTop--;
			} else {
				g_xge.procCurrentScene = NULL;
			}
			break;
	}
	
	g_xge.iPendingAction = 0;
}

/* ============================================================================
 * 输入 API 实现
 * ============================================================================ */

int xgeGetKey(int iKeyCode)
{
	if ( (iKeyCode >= 0) && (iKeyCode < 512) ) {
		return g_xge.arrKeyState[iKeyCode];
	}
	return 0;
}

void xgeGetMouse(int* piX, int* piY)
{
	if ( piX ) *piX = g_xge.iMouseX;
	if ( piY ) *piY = g_xge.iMouseY;
}

int xgeGetMouseButton(int iButton)
{
	return (g_xge.iMouseButtons & iButton) ? 1 : 0;
}

float xgeGetMouseWheel(void)
{
	float fWheel = g_xge.fMouseWheel;
	g_xge.fMouseWheel = 0.0f;
	return fWheel;
}

/* ============================================================================
 * Sokol 回调处理
 * ============================================================================ */

static void xge_OnInit(void)
{
	XgeEvent evt = {0};
	
	XGE_LOG("XGE Engine v%s initializing...", XGE_VERSION_STRING);
	XGE_LOG("Platform: %s", XGE_PLATFORM_NAME);
	
	/* 加载 GL */
	if ( !xge_gl_load((XgeGLLoadProc)XGE_GL_GET_PROC) ) {
		XGE_ERROR("Failed to load OpenGL functions");
		sapp_quit();
		return;
	}
	
	XGE_LOG("OpenGL: %s", glGetString(GL_VERSION));
	XGE_LOG("Renderer: %s", glGetString(GL_RENDERER));
	
	/* 初始化引擎状态 */
	g_xge.iWidth = sapp_width();
	g_xge.iHeight = sapp_height();
	g_xge.bInitialized = XGE_TRUE;
	g_xge.bRunning = XGE_TRUE;
	g_xge.iTargetFPS = 60;
	g_xge.fDeltaTime = 1.0f / 60.0f;
	
	/* 启用混合 */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	/* 初始化当前场景 */
	if ( g_xge.procCurrentScene ) {
		evt.iType = XGE_MSG_INIT;
		g_xge.procCurrentScene(XGE_MSG_INIT, &evt);
	}
	
	XGE_LOG("XGE initialized successfully");
}

static void xge_OnFrame(void)
{
	XgeEvent evt = {0};
	
	if ( !g_xge.bInitialized ) {
		return;
	}
	
	/* 处理待定场景切换 */
	xge_ProcessPendingScene();
	
	/* 更新窗口尺寸 */
	g_xge.iWidth = sapp_width();
	g_xge.iHeight = sapp_height();
	
	/* 计算帧时间 */
	double dCurrentTime = sapp_frame_count() / 60.0;
	g_xge.fDeltaTime = (float)sapp_frame_duration();
	
	/* FPS 统计 */
	g_xge.iFrameCount++;
	g_xge.dFPSTime += g_xge.fDeltaTime;
	if ( g_xge.dFPSTime >= 1.0 ) {
		g_xge.iCurrentFPS = g_xge.iFrameCount;
		g_xge.iFrameCount = 0;
		g_xge.dFPSTime = 0.0;
	}
	
	/* 设置视口 */
	glViewport(0, 0, g_xge.iWidth, g_xge.iHeight);
	
	/* 调用场景帧更新 */
	if ( g_xge.procCurrentScene ) {
		evt.iType = XGE_MSG_FRAME;
		g_xge.procCurrentScene(XGE_MSG_FRAME, &evt);
	}
	
	/* 调用场景绘制 */
	if ( g_xge.procCurrentScene ) {
		evt.iType = XGE_MSG_DRAW;
		g_xge.procCurrentScene(XGE_MSG_DRAW, &evt);
	}
}

static void xge_OnCleanup(void)
{
	XgeEvent evt = {0};
	
	XGE_LOG("XGE cleanup...");
	
	/* 退出当前场景 */
	if ( g_xge.procCurrentScene ) {
		evt.iType = XGE_MSG_EXIT;
		g_xge.procCurrentScene(XGE_MSG_EXIT, &evt);
	}
	
	g_xge.bInitialized = XGE_FALSE;
}

static void xge_OnEvent(const sapp_event* pSokolEvent)
{
	XgeEvent evt = {0};
	
	if ( !g_xge.bInitialized || !g_xge.procCurrentScene ) {
		return;
	}
	
	switch ( pSokolEvent->type ) {
		case SAPP_EVENTTYPE_KEY_DOWN:
			if ( (pSokolEvent->key_code >= 0) && (pSokolEvent->key_code < 512) ) {
				g_xge.arrKeyState[pSokolEvent->key_code] = 1;
			}
			evt.iType = pSokolEvent->key_repeat ? XGE_MSG_KEY_REPEAT : XGE_MSG_KEY_DOWN;
			evt.key.iKeyCode = pSokolEvent->key_code;
			evt.key.iScanCode = pSokolEvent->key_code;
			evt.key.iModifiers = pSokolEvent->modifiers;
			evt.key.iRepeat = pSokolEvent->key_repeat;
			g_xge.procCurrentScene(evt.iType, &evt);
			break;
			
		case SAPP_EVENTTYPE_KEY_UP:
			if ( (pSokolEvent->key_code >= 0) && (pSokolEvent->key_code < 512) ) {
				g_xge.arrKeyState[pSokolEvent->key_code] = 0;
			}
			evt.iType = XGE_MSG_KEY_UP;
			evt.key.iKeyCode = pSokolEvent->key_code;
			evt.key.iScanCode = pSokolEvent->key_code;
			evt.key.iModifiers = pSokolEvent->modifiers;
			g_xge.procCurrentScene(evt.iType, &evt);
			break;
			
		case SAPP_EVENTTYPE_CHAR:
			evt.iType = XGE_MSG_CHAR;
			evt.chr.uChar = pSokolEvent->char_code;
			g_xge.procCurrentScene(evt.iType, &evt);
			break;
			
		case SAPP_EVENTTYPE_MOUSE_DOWN:
			if ( pSokolEvent->mouse_button == SAPP_MOUSEBUTTON_LEFT ) {
				g_xge.iMouseButtons |= XGE_MOUSE_LEFT;
			} else if ( pSokolEvent->mouse_button == SAPP_MOUSEBUTTON_RIGHT ) {
				g_xge.iMouseButtons |= XGE_MOUSE_RIGHT;
			} else if ( pSokolEvent->mouse_button == SAPP_MOUSEBUTTON_MIDDLE ) {
				g_xge.iMouseButtons |= XGE_MOUSE_MIDDLE;
			}
			evt.iType = XGE_MSG_MOUSE_DOWN;
			evt.mouse.iX = (int)pSokolEvent->mouse_x;
			evt.mouse.iY = (int)pSokolEvent->mouse_y;
			evt.mouse.iButton = pSokolEvent->mouse_button + 1;
			g_xge.procCurrentScene(evt.iType, &evt);
			break;
			
		case SAPP_EVENTTYPE_MOUSE_UP:
			if ( pSokolEvent->mouse_button == SAPP_MOUSEBUTTON_LEFT ) {
				g_xge.iMouseButtons &= ~XGE_MOUSE_LEFT;
			} else if ( pSokolEvent->mouse_button == SAPP_MOUSEBUTTON_RIGHT ) {
				g_xge.iMouseButtons &= ~XGE_MOUSE_RIGHT;
			} else if ( pSokolEvent->mouse_button == SAPP_MOUSEBUTTON_MIDDLE ) {
				g_xge.iMouseButtons &= ~XGE_MOUSE_MIDDLE;
			}
			evt.iType = XGE_MSG_MOUSE_UP;
			evt.mouse.iX = (int)pSokolEvent->mouse_x;
			evt.mouse.iY = (int)pSokolEvent->mouse_y;
			evt.mouse.iButton = pSokolEvent->mouse_button + 1;
			g_xge.procCurrentScene(evt.iType, &evt);
			break;
			
		case SAPP_EVENTTYPE_MOUSE_MOVE:
			evt.iType = XGE_MSG_MOUSE_MOVE;
			evt.mouse.iDeltaX = (int)pSokolEvent->mouse_x - g_xge.iMouseX;
			evt.mouse.iDeltaY = (int)pSokolEvent->mouse_y - g_xge.iMouseY;
			g_xge.iMouseX = (int)pSokolEvent->mouse_x;
			g_xge.iMouseY = (int)pSokolEvent->mouse_y;
			evt.mouse.iX = g_xge.iMouseX;
			evt.mouse.iY = g_xge.iMouseY;
			g_xge.procCurrentScene(evt.iType, &evt);
			break;
			
		case SAPP_EVENTTYPE_MOUSE_SCROLL:
			g_xge.fMouseWheel = pSokolEvent->scroll_y;
			evt.iType = XGE_MSG_MOUSE_WHEEL;
			evt.mouse.fWheel = pSokolEvent->scroll_y;
			evt.mouse.iX = g_xge.iMouseX;
			evt.mouse.iY = g_xge.iMouseY;
			g_xge.procCurrentScene(evt.iType, &evt);
			break;
			
		case SAPP_EVENTTYPE_MOUSE_ENTER:
			evt.iType = XGE_MSG_MOUSE_ENTER;
			g_xge.procCurrentScene(evt.iType, &evt);
			break;
			
		case SAPP_EVENTTYPE_MOUSE_LEAVE:
			evt.iType = XGE_MSG_MOUSE_LEAVE;
			g_xge.procCurrentScene(evt.iType, &evt);
			break;
			
		case SAPP_EVENTTYPE_RESIZED:
			g_xge.iWidth = pSokolEvent->window_width;
			g_xge.iHeight = pSokolEvent->window_height;
			evt.iType = XGE_MSG_RESIZE;
			evt.window.iWidth = g_xge.iWidth;
			evt.window.iHeight = g_xge.iHeight;
			g_xge.procCurrentScene(evt.iType, &evt);
			break;
			
		case SAPP_EVENTTYPE_FOCUSED:
			evt.iType = XGE_MSG_FOCUS;
			g_xge.procCurrentScene(evt.iType, &evt);
			break;
			
		case SAPP_EVENTTYPE_UNFOCUSED:
			evt.iType = XGE_MSG_BLUR;
			g_xge.procCurrentScene(evt.iType, &evt);
			break;
			
		default:
			break;
	}
}

/* ============================================================================
 * 运行入口 (由 main.c 调用)
 * ============================================================================ */

void xgeRun(XgeSceneProc procScene)
{
	g_xge.procCurrentScene = procScene;
}

#endif /* XGE_CORE_H */
