/**
 * @file xge.h
 * @brief XGE 游戏引擎主头文件
 * 
 * xywh Game Engine - 简单的 2D 游戏引擎
 * 支持平台: Windows, Linux, macOS, Android, iOS, Web
 */

#ifndef XGE_H
#define XGE_H

#include "xge_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 版本信息
 * ============================================================================ */

#define XGE_VERSION_MAJOR 2
#define XGE_VERSION_MINOR 0
#define XGE_VERSION_PATCH 0
#define XGE_VERSION_STRING "2.0.0"

/* ============================================================================
 * 初始化标志
 * ============================================================================ */

#define XGE_INIT_WINDOW      0x00    /* 窗口模式 */
#define XGE_INIT_FULLSCREEN  0x01    /* 全屏模式 */
#define XGE_INIT_NOFRAME     0x02    /* 无边框窗口 */
#define XGE_INIT_RESIZABLE   0x04    /* 可调整大小 */
#define XGE_INIT_VSYNC       0x08    /* 垂直同步 */
#define XGE_INIT_HIGHDPI     0x10    /* 高DPI支持 */

/* ============================================================================
 * 消息/事件类型
 * ============================================================================ */

#define XGE_MSG_NULL         0x00    /* 无消息 */
#define XGE_MSG_INIT         0x01    /* 初始化 */
#define XGE_MSG_EXIT         0x02    /* 退出 */
#define XGE_MSG_FRAME        0x03    /* 帧更新 */
#define XGE_MSG_DRAW         0x04    /* 绘制 */
#define XGE_MSG_RESIZE       0x05    /* 窗口大小改变 */
#define XGE_MSG_FOCUS        0x06    /* 获得焦点 */
#define XGE_MSG_BLUR         0x07    /* 失去焦点 */

/* 鼠标消息 */
#define XGE_MSG_MOUSE_MOVE   0x10    /* 鼠标移动 */
#define XGE_MSG_MOUSE_DOWN   0x11    /* 鼠标按下 */
#define XGE_MSG_MOUSE_UP     0x12    /* 鼠标弹起 */
#define XGE_MSG_MOUSE_WHEEL  0x13    /* 鼠标滚轮 */
#define XGE_MSG_MOUSE_ENTER  0x14    /* 鼠标进入 */
#define XGE_MSG_MOUSE_LEAVE  0x15    /* 鼠标离开 */

/* 键盘消息 */
#define XGE_MSG_KEY_DOWN     0x20    /* 按键按下 */
#define XGE_MSG_KEY_UP       0x21    /* 按键弹起 */
#define XGE_MSG_KEY_REPEAT   0x22    /* 按键重复 */
#define XGE_MSG_CHAR         0x23    /* 字符输入 */

/* ============================================================================
 * 鼠标按钮
 * ============================================================================ */

#define XGE_MOUSE_LEFT       0x01
#define XGE_MOUSE_RIGHT      0x02
#define XGE_MOUSE_MIDDLE     0x04

/* ============================================================================
 * 混合标志
 * ============================================================================ */

#define XGE_BLEND_NONE       0x00    /* 无混合 */
#define XGE_BLEND_ALPHA      0x01    /* Alpha 混合 */
#define XGE_BLEND_ADD        0x02    /* 加法混合 */
#define XGE_BLEND_MIRR_H     0x10    /* 水平镜像 */
#define XGE_BLEND_MIRR_V     0x20    /* 垂直镜像 */

/* ============================================================================
 * 颜色宏
 * ============================================================================ */

#define XGE_RGB(r, g, b)       (0xFF000000 | ((r) << 16) | ((g) << 8) | (b))
#define XGE_RGBA(r, g, b, a)   (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))
#define XGE_GETR(c)            (((c) >> 16) & 0xFF)
#define XGE_GETG(c)            (((c) >> 8) & 0xFF)
#define XGE_GETB(c)            ((c) & 0xFF)
#define XGE_GETA(c)            (((c) >> 24) & 0xFF)

/* 预定义颜色 */
#define XGE_COLOR_BLACK      0xFF000000
#define XGE_COLOR_WHITE      0xFFFFFFFF
#define XGE_COLOR_RED        0xFFFF0000
#define XGE_COLOR_GREEN      0xFF00FF00
#define XGE_COLOR_BLUE       0xFF0000FF
#define XGE_COLOR_YELLOW     0xFFFFFF00
#define XGE_COLOR_CYAN       0xFF00FFFF
#define XGE_COLOR_MAGENTA    0xFFFF00FF
#define XGE_COLOR_GRAY       0xFF808080

/* ============================================================================
 * 数据类型
 * ============================================================================ */

/* 事件结构 */
typedef struct XgeEvent {
	int iType;                  /* 事件类型 */
	union {
		/* 键盘事件 */
		struct {
			int iKeyCode;       /* 键码 */
			int iScanCode;      /* 扫描码 */
			int iModifiers;     /* 修饰键 */
			int iRepeat;        /* 重复标志 */
		} key;
		/* 鼠标事件 */
		struct {
			int iX;             /* X 坐标 */
			int iY;             /* Y 坐标 */
			int iDeltaX;        /* X 增量 */
			int iDeltaY;        /* Y 增量 */
			int iButton;        /* 按钮 */
			float fWheel;       /* 滚轮值 */
		} mouse;
		/* 窗口事件 */
		struct {
			int iWidth;         /* 宽度 */
			int iHeight;        /* 高度 */
		} window;
		/* 字符输入 */
		struct {
			unsigned int uChar; /* Unicode 字符 */
		} chr;
	};
} XgeEvent;

/* 画布句柄 */
typedef struct XgeSurface XgeSurface;

/* 场景回调类型 */
typedef int (*XgeSceneProc)(int iMsg, XgeEvent* pEvent);

/* ============================================================================
 * 核心 API
 * ============================================================================ */

/**
 * @brief 初始化引擎
 * @param iWidth 窗口宽度
 * @param iHeight 窗口高度
 * @param sTitle 窗口标题
 * @param iFlags 初始化标志
 * @return 成功返回 1，失败返回 0
 */
int xgeInit(int iWidth, int iHeight, const char* sTitle, int iFlags);

/**
 * @brief 退出引擎
 */
void xgeExit(void);

/**
 * @brief 运行主循环
 * @param procScene 场景回调函数
 */
void xgeRun(XgeSceneProc procScene);

/**
 * @brief 请求退出
 */
void xgeQuit(void);

/**
 * @brief 设置帧率
 * @param iFPS 目标帧率 (0 = 不限制)
 */
void xgeSetFPS(int iFPS);

/**
 * @brief 获取当前帧率
 * @return 当前帧率
 */
int xgeGetFPS(void);

/**
 * @brief 获取帧间隔时间
 * @return 帧间隔（秒）
 */
float xgeGetDelta(void);

/**
 * @brief 获取窗口宽度
 */
int xgeGetWidth(void);

/**
 * @brief 获取窗口高度
 */
int xgeGetHeight(void);

/* ============================================================================
 * 画布 API
 * ============================================================================ */

/**
 * @brief 创建离屏画布
 */
XgeSurface* xgeSurfaceCreate(int iWidth, int iHeight);

/**
 * @brief 释放画布
 */
void xgeSurfaceFree(XgeSurface* pSurface);

/**
 * @brief 设置绘制目标
 * @param pTarget 目标画布 (NULL = 屏幕)
 */
void xgeSetTarget(XgeSurface* pTarget);

/**
 * @brief 获取当前绘制目标
 */
XgeSurface* xgeGetTarget(void);

/**
 * @brief 清除画布
 * @param uColor 清除颜色
 */
void xgeClear(unsigned int uColor);

/* ============================================================================
 * 形状 API
 * ============================================================================ */

/**
 * @brief 画点
 */
void xgePset(int iX, int iY, unsigned int uColor);

/**
 * @brief 画线
 */
void xgeLine(int iX1, int iY1, int iX2, int iY2, unsigned int uColor);

/**
 * @brief 画矩形
 */
void xgeBox(int iX, int iY, int iW, int iH, unsigned int uColor);

/**
 * @brief 填充矩形
 */
void xgeBoxFill(int iX, int iY, int iW, int iH, unsigned int uColor);

/**
 * @brief 画圆
 */
void xgeCircle(int iX, int iY, int iRadius, unsigned int uColor);

/**
 * @brief 填充圆
 */
void xgeCircleFill(int iX, int iY, int iRadius, unsigned int uColor);

/**
 * @brief 画三角形
 */
void xgeTriangle(int iX1, int iY1, int iX2, int iY2, int iX3, int iY3, unsigned int uColor);

/**
 * @brief 填充三角形
 */
void xgeTriangleFill(int iX1, int iY1, int iX2, int iY2, int iX3, int iY3, unsigned int uColor);

/* ============================================================================
 * 输入 API
 * ============================================================================ */

/**
 * @brief 获取按键状态
 * @param iKeyCode 键码
 * @return 按下返回 1，否则返回 0
 */
int xgeGetKey(int iKeyCode);

/**
 * @brief 获取鼠标位置
 */
void xgeGetMouse(int* piX, int* piY);

/**
 * @brief 获取鼠标按钮状态
 * @param iButton 按钮 (XGE_MOUSE_LEFT 等)
 * @return 按下返回 1，否则返回 0
 */
int xgeGetMouseButton(int iButton);

/**
 * @brief 获取鼠标滚轮增量
 */
float xgeGetMouseWheel(void);

/* ============================================================================
 * 场景 API
 * ============================================================================ */

/**
 * @brief 设置当前场景
 */
void xgeSetScene(XgeSceneProc procScene);

/**
 * @brief 压入场景（保留当前场景）
 */
void xgePushScene(XgeSceneProc procScene);

/**
 * @brief 弹出场景（返回上一场景）
 */
void xgePopScene(void);

#ifdef __cplusplus
}
#endif

#endif /* XGE_H */
