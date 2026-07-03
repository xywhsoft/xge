# 输入系统入门

> 状态：中文初稿已生成，待审阅。

XGE 的输入系统提供两种使用方式：直接查询当前状态，或通过 scene/XUI 接收事件。游戏逻辑通常直接查询，GUI 和复杂状态机更适合事件派发。

## 输入从哪里来

```text
平台后端 poll
  -> XGE 输入状态
  -> xgeKeyDown / xgeMouseGet / xgeTouchGet / xgeGamepadGetState
  -> Scene / XUI 事件处理
```

在默认 `xgeRun` 中，平台事件会在每帧自动更新。手动模式下，宿主要调用 `xgeFrame`。

## 键盘

```c
if ( xgeKeyDown(XGE_KEY_LEFT) ) {
	player_x -= speed * xgeGetDelta();
}

if ( xgeKeyPressed(XGE_KEY_ENTER) ) {
	confirm();
}
```

区别：

- `Down`：当前是否按住。
- `Pressed`：本帧从未按下变为按下。
- `Released`：本帧从按下变为松开。

边沿状态只保留当前帧，建议在 update 里集中读取。

## 鼠标

```c
float x;
float y;
xgeMouseGet(&x, &y);

if ( xgeMouseDown(XGE_MOUSE_LEFT) ) {
	drag_to(x, y);
}
```

滚轮和移动增量：

```c
float dx;
float dy;
xgeMouseGetDelta(&dx, &dy);
xgeMouseGetWheel(&dx, &dy);
```

## 文本输入

```c
uint32_t cp = xgeTextGet();
if ( cp != 0 ) {
	append_codepoint(cp);
}
```

文本输入和按键输入不是一回事。输入中文、日文等复杂文本时，系统 IME 负责组合候选，XGE 接收提交后的 codepoint。第一版不内置完整 IME。

## 多点触控

触摸点有稳定 ID。不要长期依赖数组 index 跟踪手指。

```c
int count = xgeTouchGetCount();
for ( int i = 0; i < count; ++i ) {
	xge_touch_point_t p;
	if ( xgeTouchGet(i, &p) == XGE_OK ) {
		handle_touch(p.iId, p.fX, p.fY, p.iPhase);
	}
}
```

如果你已经保存了某根手指的 ID：

```c
xge_touch_point_t p;
if ( xgeTouchFind(active_id, &p) == XGE_OK ) {
	update_drag(p.fX, p.fY);
}
```

## 手柄

```c
if ( xgeGamepadConnected(0) ) {
	float x = xgeGamepadAxis(0, 0);
	if ( xgeGamepadButtonPressed(0, 0) ) {
		jump();
	}
}
```

测试或自定义宿主可以注入手柄状态：

```c
xge_gamepad_state_t state;
memset(&state, 0, sizeof(state));
state.bConnected = 1;
state.iButtons = 1;
xgeGamepadSetState(0, &state);
```

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| `Pressed` 偶尔读不到 | 在多个地方分散读取边沿状态 | 在 update 中集中消费。 |
| 多点触控拖拽错乱 | 用 index 追踪手指 | 用 `iId` 保存手指身份。 |
| 中文输入异常 | 把 key 当 text 处理 | 文本输入使用 `xgeTextGet` 或 XUI input。 |

## 下一步

- 查完整函数看 [Input API](../api/input.md)。
- GUI 输入看 [XUI 桥接入门](xui-bridge-intro.md)。

[返回教程入口](README.md)
