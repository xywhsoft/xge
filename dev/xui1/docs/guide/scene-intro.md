# 场景系统入门

> 状态：中文初稿已生成，待审阅。

Scene 用来组织游戏流程。它不是 entity 系统，也不是对象树；它更像“当前游戏状态”的生命周期容器，例如主菜单、关卡、暂停菜单、设置窗口和结算界面。

## 什么时候使用 Scene

适合使用 Scene 的情况：

- 游戏有多个清晰状态，例如主菜单、游戏中、暂停、结算。
- 需要进入一个子流程，结束后回到原流程。
- 想把 update、draw、event 按状态拆开。

不适合让 Scene 承担的事情：

- 每个角色或子弹都做成 scene。
- 用 scene 代替 ECS、node 或 UI 树。
- 把所有资源都交给 scene 自动管理。

## 最小 scene

```c
static int GameUpdate(xge_scene scene, float delta)
{
	(void)scene;
	(void)delta;
	return 0;
}

static int GameDraw(xge_scene scene)
{
	(void)scene;
	xgeClear(xgeColorRGBA(20, 24, 32, 255));
	return 0;
}

xge_scene_t game;
memset(&game, 0, sizeof(game));
game.onUpdate = GameUpdate;
game.onDraw = GameDraw;
xgeSceneSet(&game);
```

调用顺序：

```text
准备 xge_scene_t
  -> 设置回调
  -> xgeSceneSet
  -> xgeRun / xgeFrame 自动驱动
```

## 场景栈

场景栈解决“临时进入子场景，退出后回到原场景”的问题。

```c
xgeSceneSet(&game_scene);
xgeScenePush(&pause_scene);

/* 暂停菜单完成后 */
xgeScenePop();
```

典型行为：

- push 子场景时，原场景暂停，子场景进入。
- pop 子场景时，子场景离开，原场景恢复。
- replace 当前场景时，不保留当前栈顶状态。

## update 策略

XGE 支持可变步长和固定步长。

```c
xgeSceneUpdateStrategySet(XGE_UPDATE_FIXED, 1.0f / 60.0f, 4);
```

选择建议：

- 动作、物理、战斗判定：固定步长。
- UI、菜单、工具窗口：可变步长。
- 固定步长一定要设置最大 update 次数，避免卡顿后补帧过多。

## user data

`xge_scene_t` 有 `pUser` 字段，建议把业务状态放在这里。

```c
typedef struct game_state_t {
	int iScore;
	float fPlayerX;
} game_state_t;

game_state_t state;
game_scene.pUser = &state;
```

回调中再取回：

```c
game_state_t* state = (game_state_t*)scene->pUser;
```

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 返回暂停菜单后游戏状态丢失 | 用了 `xgeSceneReplace` | 需要返回原状态时用 `xgeScenePush` / `xgeScenePop`。 |
| scene 指针失效 | scene 是局部变量，函数返回后仍在栈中 | scene 生命周期必须覆盖其在场景栈中的时间。 |
| 固定步长卡住 | 最大 update 次数过大 | 给 `iMaxUpdates` 设置合理上限。 |

## 下一步

- 查函数细节看 [Scene API](../api/scene.md)。
- 看完整例子用 [Scene Stack 范例](../case/scene-stack.md)。

[返回教程入口](README.md)
