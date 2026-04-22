# 场景栈

本案例展示如何使用 scene stack 实现主场景、暂停层、弹窗子场景和返回流程。

[返回范例解析](README.md) | [场景教程](../guide/scene-intro.md) | [Scene API](../api/scene.md)

## 问题

游戏常见结构不是只有一个全局 update/draw，而是存在菜单、关卡、暂停、结算、设置等多个状态。scene stack 用栈表达这些状态，避免所有逻辑挤进一个巨大 switch。

## 场景职责

```text
主菜单 scene
  -> replace 游戏 scene
游戏 scene
  -> push 暂停 scene
暂停 scene
  -> pop 返回游戏
游戏 scene
  -> push 结算 scene
结算 scene
  -> replace 主菜单 scene
```

下层场景是否继续 update/draw 由场景自身和栈策略决定。第一版建议默认只更新栈顶，绘制时允许暂停层绘制半透明遮罩后继续显示下层画面。

## 步骤 1：定义场景对象

```c
typedef struct game_scene_t {
	xge_scene_t base;
	int iPaused;
	float fTime;
} game_scene_t;
```

公开 API 使用透明结构，便于业务读取必要状态；内部运行时状态仍由 XGE 管理。

## 步骤 2：实现 update/draw

```c
static int GameUpdate(xge_scene pScene, float fDelta)
{
	game_scene_t* pGame;

	pGame = (game_scene_t*)pScene;
	pGame->fTime += fDelta;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeScenePush(&g_pause.base);
	}
	return 0;
}

static int GameDraw(xge_scene pScene)
{
	(void)pScene;

	xgeClear(xgeColorRGBA(18, 22, 30, 255));
	xgeShapeRectFill((xge_rect_t){ 64.0f, 64.0f, 160.0f, 80.0f }, xgeColorRGBA(60, 130, 220, 255));
	return 0;
}
```

## 步骤 3：暂停层拦截输入

```c
static int PauseUpdate(xge_scene pScene, float fDelta)
{
	(void)pScene;
	(void)fDelta;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) || xgeKeyPressed(XGE_KEY_ENTER) ) {
		xgeScenePop();
	}
	return 0;
}

static int PauseDraw(xge_scene pScene)
{
	(void)pScene;

	xgeShapeRectFill((xge_rect_t){ 0.0f, 0.0f, 800.0f, 600.0f }, xgeColorRGBA(0, 0, 0, 128));
	return 0;
}
```

## 步骤 4：安装初始场景

```c
xgeSceneSet(&g_game.base);
xgeRun(NULL, NULL);
```

如果使用 `xgeRun` 的默认场景调度，XGE 会在每帧调用当前场景的 update/draw。手动模式下也可以自己调用 `xgeFrame` 和 `xgeRender`。

## 关键 API

| API | 作用 |
| --- | --- |
| `xgeSceneSet` | 设置根场景 |
| `xgeScenePush` | 压入子场景 |
| `xgeScenePop` | 弹出当前场景 |
| `xgeSceneReplace` | 替换当前场景 |
| `xgeSceneCurrent` | 获取当前场景 |
| `xgeSceneUpdateStrategySet` | 设置 update/draw 调度策略 |

## 常见问题

如果暂停层无法返回，确认暂停场景收到输入事件，且没有被 XUI 或其他系统提前捕获。

如果底层场景在暂停期间仍然移动，需要将 update 策略设为只更新栈顶，或在业务场景中检测 `iPaused`。
