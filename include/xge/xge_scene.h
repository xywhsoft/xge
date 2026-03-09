/**
 * @file xge_scene.h
 * @brief XGE 场景管理模块
 * 
 * 场景核心实现在 xge_core.h 中
 * 此文件提供场景编写辅助宏和工具
 */

#ifndef XGE_SCENE_H
#define XGE_SCENE_H

/* ============================================================================
 * 场景宏
 * ============================================================================ */

/**
 * @brief 定义场景函数
 * @example
 * XGE_SCENE_DEF(SceneMenu)
 * {
 *     XGE_SCENE_BEGIN
 *         XGE_ON_INIT:
 *             // 初始化代码
 *             break;
 *         XGE_ON_FRAME:
 *             // 帧更新代码
 *             break;
 *         XGE_ON_DRAW:
 *             // 绘制代码
 *             break;
 *         XGE_ON_EXIT:
 *             // 清理代码
 *             break;
 *     XGE_SCENE_END
 * }
 */

#define XGE_SCENE_DEF(name) \
	int name(int iMsg, XgeEvent* pEvent)

#define XGE_SCENE_BEGIN \
	switch (iMsg) {

#define XGE_SCENE_END \
	} \
	return 0;

#define XGE_ON_INIT     case XGE_MSG_INIT
#define XGE_ON_EXIT     case XGE_MSG_EXIT
#define XGE_ON_FRAME    case XGE_MSG_FRAME
#define XGE_ON_DRAW     case XGE_MSG_DRAW
#define XGE_ON_RESIZE   case XGE_MSG_RESIZE
#define XGE_ON_FOCUS    case XGE_MSG_FOCUS
#define XGE_ON_BLUR     case XGE_MSG_BLUR

#define XGE_ON_KEY_DOWN     case XGE_MSG_KEY_DOWN
#define XGE_ON_KEY_UP       case XGE_MSG_KEY_UP
#define XGE_ON_KEY_REPEAT   case XGE_MSG_KEY_REPEAT
#define XGE_ON_CHAR         case XGE_MSG_CHAR

#define XGE_ON_MOUSE_MOVE   case XGE_MSG_MOUSE_MOVE
#define XGE_ON_MOUSE_DOWN   case XGE_MSG_MOUSE_DOWN
#define XGE_ON_MOUSE_UP     case XGE_MSG_MOUSE_UP
#define XGE_ON_MOUSE_WHEEL  case XGE_MSG_MOUSE_WHEEL
#define XGE_ON_MOUSE_ENTER  case XGE_MSG_MOUSE_ENTER
#define XGE_ON_MOUSE_LEAVE  case XGE_MSG_MOUSE_LEAVE

/* ============================================================================
 * 简化写法场景模板宏
 * ============================================================================ */

/**
 * @brief 简单场景模板
 * @example
 * XGE_SIMPLE_SCENE(SceneGame, 
 *     // 初始化
 *     int score = 0;
 * ,
 *     // 更新
 *     if (xgeGetKey(XGE_KEY_SPACE)) score++;
 * ,
 *     // 绘制
 *     xgeClear(XGE_COLOR_BLACK);
 *     xgeBoxFill(100, 100, 50, 50, XGE_COLOR_RED);
 * ,
 *     // 退出
 * )
 */

#define XGE_SIMPLE_SCENE(name, init_code, update_code, draw_code, exit_code) \
	XGE_SCENE_DEF(name) \
	{ \
		XGE_SCENE_BEGIN \
			XGE_ON_INIT: \
				init_code \
				break; \
			XGE_ON_FRAME: \
				update_code \
				break; \
			XGE_ON_DRAW: \
				draw_code \
				break; \
			XGE_ON_EXIT: \
				exit_code \
				break; \
		XGE_SCENE_END \
	}

#endif /* XGE_SCENE_H */
