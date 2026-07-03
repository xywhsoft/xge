# Input API

> Input API 负责键盘、鼠标、文本输入、多点触控和手柄状态查询。

[返回 API 索引](README.md) | [输入教程](../guide/input-intro.md) | [Input 范例](../case/input.md)

---

## 模块定位

Input API 提供当前帧输入状态的查询接口。平台后端负责把系统事件转换为 XGE 内部状态，用户在 update、scene event 或 frame callback 中读取这些状态。

`Pressed` / `Released` 类函数只表示当前帧边沿状态，应该在同一帧内读取。XUI 会消费 XGE 输入事件并处理 focus、capture、控件响应。

## 标准调用顺序

```text
xgeFrame / xgeRun 内部 poll
  -> 平台后端更新输入状态
  -> xgeKeyDown / xgeMouseGet / xgeTouchGet / xgeGamepadGetState
```

## 常量与宏

| 名称 | 说明 |
| --- | --- |
| `XGE_KEY_COUNT` | 键盘按键状态数组容量。 |
| `XGE_TEXT_MAX` | 单帧文本输入缓存容量。 |
| `XGE_TOUCH_MAX` | 最大触点数。 |
| `XGE_GAMEPAD_MAX` | 最大手柄数。 |
| `XGE_GAMEPAD_BUTTON_COUNT` | 手柄按钮数。 |
| `XGE_GAMEPAD_AXIS_COUNT` | 手柄轴数。 |
| `XGE_MOUSE_LEFT` / `XGE_MOUSE_RIGHT` / `XGE_MOUSE_MIDDLE` | 鼠标按钮位。 |

Key code 当前映射 Sokol key code。跨平台业务层建议优先使用 XGE 暴露的 `XGE_KEY_*` 常量。

## 公共类型

### `xge_touch_point_t`

触点快照。`iId` 是稳定触点 ID，同一根手指从 begin 到 end 应保持一致；`fX/fY` 是当前位置；`fDX/fDY` 是本帧位移；`iPhase` 是触摸阶段；`bChanged` 表示本帧是否变化；`bDown` 表示是否仍按下。

### `xge_gamepad_state_t`

手柄状态快照。`bConnected` 表示连接状态；`iButtons` 是当前按钮位集；`iButtonsPressed` 和 `iButtonsReleased` 是当前帧边沿位集；`arrAxes` 保存轴值，通常范围为 `-1.0f` 到 `1.0f`。

## Keyboard

### xgeKeyDown

查询按键当前是否按下。

**功能：**

你可以在 update 或 frame callback 中读取持续按住状态，例如移动、蓄力或快捷键保持。

**函数原型：**

```c
XGE_API int xgeKeyDown(int iKey);
```

**参数：**

- `iKey`：输入参数，XGE key code。合法范围是 `0` 到 `XGE_KEY_COUNT - 1`。

**返回值：**

- 返回 `1` 表示当前按下。
- 返回 `0` 表示未按下，或 `iKey` 越界。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 该函数读取当前状态，不消费状态。
- 对文本输入不要使用 keydown 拼字符，应使用 `xgeTextGet`。

**范例代码：**

```c
if (xgeKeyDown(XGE_KEY_LEFT)) {
	/* move left */
}
```

**相关 API：**

- `xgeKeyPressed`
- `xgeKeyReleased`
- `xgeTextGet`

---

### xgeKeyPressed

查询按键当前帧是否按下。

**功能：**

你可以读取从未按下到按下的边沿状态，例如菜单确认、跳跃或一次性快捷键。

**函数原型：**

```c
XGE_API int xgeKeyPressed(int iKey);
```

**参数：**

- `iKey`：输入参数，XGE key code。合法范围是 `0` 到 `XGE_KEY_COUNT - 1`。

**返回值：**

- 返回 `1` 表示当前帧刚按下。
- 返回 `0` 表示没有刚按下，或 `iKey` 越界。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 该状态只在当前帧有效。
- 多处读取同一边沿状态不会被函数本身消费，但业务上应避免多处重复响应。

**范例代码：**

```c
if (xgeKeyPressed(XGE_KEY_ENTER)) {
	/* confirm */
}
```

**相关 API：**

- `xgeKeyDown`
- `xgeKeyReleased`
- `xgeSceneDispatchEvent`

---

### xgeKeyReleased

查询按键当前帧是否释放。

**功能：**

你可以读取从按下到未按下的边沿状态，例如松开蓄力、结束拖拽或快捷键释放。

**函数原型：**

```c
XGE_API int xgeKeyReleased(int iKey);
```

**参数：**

- `iKey`：输入参数，XGE key code。合法范围是 `0` 到 `XGE_KEY_COUNT - 1`。

**返回值：**

- 返回 `1` 表示当前帧刚释放。
- 返回 `0` 表示没有刚释放，或 `iKey` 越界。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 该状态只在当前帧有效。

**范例代码：**

```c
if (xgeKeyReleased(XGE_KEY_ENTER)) {
	/* stop confirm hold */
}
```

**相关 API：**

- `xgeKeyDown`
- `xgeKeyPressed`
- `xgeFrame`

---

## Mouse

### xgeMouseGet

读取鼠标当前位置。

**功能：**

你可以获取当前鼠标坐标，用于按钮 hover、拖拽、拾取或编辑器工具。

**函数原型：**

```c
XGE_API void xgeMouseGet(float* pX, float* pY);
```

**参数：**

- `pX`：输出参数，可以为 `NULL`。非 `NULL` 时写入 X 坐标。
- `pY`：输出参数，可以为 `NULL`。非 `NULL` 时写入 Y 坐标。

**返回值：**

无。

**资源归属：**

调用者持有输出指针。XGE 不保存指针。

**补充说明：**

- 坐标单位由平台后端输入映射决定，通常是 XGE 逻辑坐标。

**范例代码：**

```c
float x;
float y;
xgeMouseGet(&x, &y);
```

**相关 API：**

- `xgeMouseGetDelta`
- `xgeMouseDown`
- `xgeScreenToWorld`

---

### xgeMouseGetDelta

读取本帧鼠标移动量。

**功能：**

你可以获取鼠标相对移动，用于拖拽、摄像机移动或编辑器手柄。

**函数原型：**

```c
XGE_API void xgeMouseGetDelta(float* pDX, float* pDY);
```

**参数：**

- `pDX`：输出参数，可以为 `NULL`。非 `NULL` 时写入 X 方向位移。
- `pDY`：输出参数，可以为 `NULL`。非 `NULL` 时写入 Y 方向位移。

**返回值：**

无。

**资源归属：**

调用者持有输出指针。XGE 不保存指针。

**补充说明：**

- delta 是当前帧输入快照的一部分，应在同一帧内使用。

**范例代码：**

```c
float dx;
float dy;
xgeMouseGetDelta(&dx, &dy);
```

**相关 API：**

- `xgeMouseGet`
- `xgeMouseDown`
- `xgeFrame`

---

### xgeMouseGetWheel

读取本帧鼠标滚轮量。

**功能：**

你可以获取滚轮输入，用于缩放、列表滚动或工具参数调节。

**函数原型：**

```c
XGE_API void xgeMouseGetWheel(float* pX, float* pY);
```

**参数：**

- `pX`：输出参数，可以为 `NULL`。非 `NULL` 时写入水平滚轮量。
- `pY`：输出参数，可以为 `NULL`。非 `NULL` 时写入垂直滚轮量。

**返回值：**

无。

**资源归属：**

调用者持有输出指针。XGE 不保存指针。

**补充说明：**

- 不同平台的滚轮单位可能不同，上层应按体验做归一化。

**范例代码：**

```c
float wheel_y;
xgeMouseGetWheel(NULL, &wheel_y);
```

**相关 API：**

- `xgeMouseGet`
- `xgeXuiWidgetDispatchEvent`
- `xgePlatformCapsGet`

---

### xgeMouseDown

查询鼠标按钮是否按下。

**功能：**

你可以读取鼠标按钮持续状态，用于拖拽、选择或绘制工具。

**函数原型：**

```c
XGE_API int xgeMouseDown(int iButton);
```

**参数：**

- `iButton`：输入参数，鼠标按钮位，例如 `XGE_MOUSE_LEFT`。

**返回值：**

- 返回 `1` 表示指定按钮当前按下。
- 返回 `0` 表示未按下。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- `iButton` 是位标志，不是数组索引。

**范例代码：**

```c
if (xgeMouseDown(XGE_MOUSE_LEFT)) {
	/* drag */
}
```

**相关 API：**

- `xgeMouseGet`
- `xgeMouseGetDelta`
- `xgeKeyDown`

---

## Text Input

### xgeTextGet

读取当前帧文本输入 codepoint。

**功能：**

你可以获取平台输入法提交后的 Unicode codepoint，用于文本框、命令行或调试控制台。

**函数原型：**

```c
XGE_API uint32_t xgeTextGet(void);
```

**参数：**

无。

**返回值：**

- 返回当前帧文本输入 codepoint。
- 没有文本输入时通常返回 `0`。

**资源归属：**

返回值是普通整数，不涉及资源所有权。

**补充说明：**

- XGE 第一版依赖系统 IME 提交文本，不实现完整候选框。
- 多字符提交的完整缓冲策略以后续 Text/Input 设计为准。

**范例代码：**

```c
uint32_t cp = xgeTextGet();
if (cp != 0) {
	/* append codepoint */
}
```

**相关 API：**

- `xgeKeyDown`
- `xgeTextUTF8Next`
- `xgeXuiInputSetText`

---

## Touch

### xgeTouchGetCount

获取当前触点数量。

**功能：**

你可以判断当前有多少个触点处于活动状态。

**函数原型：**

```c
XGE_API int xgeTouchGetCount(void);
```

**参数：**

无。

**返回值：**

- 返回当前触点数量，范围通常为 `0` 到 `XGE_TOUCH_MAX`。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 多点触控应使用 `iId` 追踪手指，不应长期依赖索引稳定。

**范例代码：**

```c
int count = xgeTouchGetCount();
```

**相关 API：**

- `xgeTouchGet`
- `xgeTouchFind`
- `xgeMiniProgramTouch`

---

### xgeTouchGet

按索引读取触点快照。

**功能：**

你可以遍历当前触点列表，读取每个触点的位置、delta、阶段和按下状态。

**函数原型：**

```c
XGE_API int xgeTouchGet(int iIndex, xge_touch_point_t* pPoint);
```

**参数：**

- `iIndex`：输入参数，触点索引，从 `0` 开始。
- `pPoint`：输出参数，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 索引越界或 `pPoint == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

调用者持有 `pPoint` 指向的结构体。XGE 不保存该指针。

**补充说明：**

- 返回的是当前触点快照副本。
- 索引只适合遍历，跨帧追踪应使用 `iId`。

**范例代码：**

```c
int i;
for (i = 0; i < xgeTouchGetCount(); i++) {
	xge_touch_point_t touch;
	if (xgeTouchGet(i, &touch) == XGE_OK) {
		/* handle touch */
	}
}
```

**相关 API：**

- `xgeTouchGetCount`
- `xgeTouchFind`
- `xgeMouseGet`

---

### xgeTouchFind

按稳定触点 ID 查找触点。

**功能：**

你可以跨帧追踪同一根手指，避免多点触控时因为索引变化导致手指错乱。

**函数原型：**

```c
XGE_API int xgeTouchFind(uint64_t iId, xge_touch_point_t* pPoint);
```

**参数：**

- `iId`：输入参数，稳定触点 ID。
- `pPoint`：输出参数，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- `pPoint == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 未找到该触点时返回 `XGE_ERROR_FILE_NOT_FOUND`。

**资源归属：**

调用者持有 `pPoint` 指向的结构体。XGE 不保存该指针。

**补充说明：**

- `iId` 由平台后端或小程序桥接提供。

**范例代码：**

```c
xge_touch_point_t touch;
if (xgeTouchFind(active_id, &touch) == XGE_OK) {
	/* continue gesture */
}
```

**相关 API：**

- `xgeTouchGet`
- `xgeTouchGetCount`
- `xgeMiniProgramTouchOne`

---

## Gamepad

### xgeGamepadConnected

查询手柄是否连接。

**功能：**

你可以判断指定手柄槽位是否可用。

**函数原型：**

```c
XGE_API int xgeGamepadConnected(int iGamepad);
```

**参数：**

- `iGamepad`：输入参数，手柄索引，合法范围是 `0` 到 `XGE_GAMEPAD_MAX - 1`。

**返回值：**

- 返回 `1` 表示连接。
- 返回 `0` 表示未连接或索引越界。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 平台后端能力不同，使用前可通过 `xgePlatformCapsGet` 查看 `bGamepad`。

**范例代码：**

```c
if (xgeGamepadConnected(0)) {
	/* use gamepad 0 */
}
```

**相关 API：**

- `xgeGamepadGetState`
- `xgeGamepadSetConnected`
- `xgePlatformCapsGet`

---

### xgeGamepadGetState

读取完整手柄状态。

**功能：**

你可以一次性获取连接、按钮、边沿按钮和轴状态，减少多次查询。

**函数原型：**

```c
XGE_API int xgeGamepadGetState(int iGamepad, xge_gamepad_state_t* pState);
```

**参数：**

- `iGamepad`：输入参数，手柄索引。
- `pState`：输出参数，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 索引越界或 `pState == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

调用者持有 `pState` 指向的结构体。XGE 不保存该指针。

**补充说明：**

- 返回的是当前状态快照副本。

**范例代码：**

```c
xge_gamepad_state_t state;
if (xgeGamepadGetState(0, &state) == XGE_OK && state.bConnected) {
	/* read state.arrAxes */
}
```

**相关 API：**

- `xgeGamepadConnected`
- `xgeGamepadAxis`
- `xgeGamepadSetState`

---

### xgeGamepadButtonDown

查询手柄按钮是否按下。

**功能：**

你可以读取手柄按钮持续状态。

**函数原型：**

```c
XGE_API int xgeGamepadButtonDown(int iGamepad, uint32_t iButton);
```

**参数：**

- `iGamepad`：输入参数，手柄索引。
- `iButton`：输入参数，按钮位。

**返回值：**

- 返回 `1` 表示按钮按下。
- 返回 `0` 表示未按下或索引越界。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- `iButton` 是位标志，不是按钮数组索引。

**范例代码：**

```c
if (xgeGamepadButtonDown(0, 1u << 0)) {
	/* button 0 held */
}
```

**相关 API：**

- `xgeGamepadButtonPressed`
- `xgeGamepadButtonReleased`
- `xgeGamepadGetState`

---

### xgeGamepadButtonPressed

查询手柄按钮当前帧是否按下。

**功能：**

你可以读取手柄按钮按下边沿，例如确认、跳跃或菜单打开。

**函数原型：**

```c
XGE_API int xgeGamepadButtonPressed(int iGamepad, uint32_t iButton);
```

**参数：**

- `iGamepad`：输入参数，手柄索引。
- `iButton`：输入参数，按钮位。

**返回值：**

- 返回 `1` 表示当前帧刚按下。
- 返回 `0` 表示没有刚按下或索引越界。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 该状态只在当前帧有效。

**范例代码：**

```c
if (xgeGamepadButtonPressed(0, 1u << 0)) {
	/* confirm */
}
```

**相关 API：**

- `xgeGamepadButtonDown`
- `xgeGamepadButtonReleased`
- `xgeGamepadSetState`

---

### xgeGamepadButtonReleased

查询手柄按钮当前帧是否释放。

**功能：**

你可以读取手柄按钮释放边沿。

**函数原型：**

```c
XGE_API int xgeGamepadButtonReleased(int iGamepad, uint32_t iButton);
```

**参数：**

- `iGamepad`：输入参数，手柄索引。
- `iButton`：输入参数，按钮位。

**返回值：**

- 返回 `1` 表示当前帧刚释放。
- 返回 `0` 表示没有刚释放或索引越界。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 该状态只在当前帧有效。

**范例代码：**

```c
if (xgeGamepadButtonReleased(0, 1u << 0)) {
	/* release action */
}
```

**相关 API：**

- `xgeGamepadButtonPressed`
- `xgeGamepadButtonDown`
- `xgeGamepadGetState`

---

### xgeGamepadAxis

读取手柄轴值。

**功能：**

你可以读取摇杆、扳机或其他模拟轴输入。

**函数原型：**

```c
XGE_API float xgeGamepadAxis(int iGamepad, int iAxis);
```

**参数：**

- `iGamepad`：输入参数，手柄索引。
- `iAxis`：输入参数，轴索引，合法范围是 `0` 到 `XGE_GAMEPAD_AXIS_COUNT - 1`。

**返回值：**

- 返回轴值，通常范围为 `-1.0f` 到 `1.0f`。
- 索引越界时返回 `0.0f`。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 不同平台的轴映射可能不同，上层应建立自己的语义映射。

**范例代码：**

```c
float lx = xgeGamepadAxis(0, 0);
```

**相关 API：**

- `xgeGamepadGetState`
- `xgeGamepadConnected`
- `xgeGamepadSetState`

---

### xgeGamepadSetConnected

注入手柄连接状态。

**功能：**

测试代码、宿主平台或非 Sokol 后端可以用它向 XGE 注入手柄连接和断开事件。

**函数原型：**

```c
XGE_API int xgeGamepadSetConnected(int iGamepad, int bConnected);
```

**参数：**

- `iGamepad`：输入参数，手柄索引。
- `bConnected`：输入参数，`0` 表示断开，非 `0` 表示连接。

**返回值：**

- 成功返回 `XGE_OK`。
- 索引越界时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

不接管外部资源。函数会修改 XGE runtime 内部手柄状态。

**补充说明：**

- 状态变化时会增加平台运行时手柄事件计数，并派发 scene event。
- 状态不变时直接返回 `XGE_OK`。

**范例代码：**

```c
xgeGamepadSetConnected(0, 1);
```

**相关 API：**

- `xgeGamepadSetState`
- `xgeGamepadConnected`
- `xgeSceneDispatchEvent`

---

### xgeGamepadSetState

注入完整手柄状态。

**功能：**

测试代码、宿主平台或非 Sokol 后端可以用它写入手柄按钮和轴状态，并让 XGE 计算 pressed/released 边沿。

**函数原型：**

```c
XGE_API int xgeGamepadSetState(int iGamepad, const xge_gamepad_state_t* pState);
```

**参数：**

- `iGamepad`：输入参数，手柄索引。
- `pState`：输入参数，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 索引越界或 `pState == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

XGE 会复制状态内容，不保存 `pState` 指针。

**补充说明：**

- `pState->bConnected == 0` 时会转为断开该手柄。
- 如果原状态未连接，函数会先设置为连接。

**范例代码：**

```c
xge_gamepad_state_t state;
memset(&state, 0, sizeof(state));
state.bConnected = 1;
state.iButtons = 1u << 0;
xgeGamepadSetState(0, &state);
```

**相关 API：**

- `xgeGamepadSetConnected`
- `xgeGamepadGetState`
- `xgeGamepadButtonPressed`

---

## 生命周期与所有权

Input 状态属于当前 XGE runtime，由平台后端在事件循环中更新。调用者只读取状态，不获得内部事件队列所有权。

输出结构体都是快照副本，生命周期由调用者管理。

## 线程约束

输入状态建议在主线程或 scene update 阶段读取。平台事件派发、XUI 事件消费和游戏逻辑读取应保持确定顺序。

不要在多个线程同时消费同一帧的 pressed/released 状态，否则会造成语义不明确。

## 后端差异

不同平台的 keyboard、mouse、touch、gamepad、text input 能力不同。文本输入依赖系统 IME 或宿主桥接，不等同于 keydown。

小程序、Web 和移动端的触控坐标需要结合 DPR 和逻辑坐标映射。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| `Pressed` 状态偶尔丢失 | 在错误帧读取边沿状态 | 在 update/frame 中集中读取，并避免多处响应同一状态。 |
| 多点触控手指错乱 | 用 index 追踪手指 | 用 `iId` 和 `xgeTouchFind`。 |
| 文本输入拿不到组合中内容 | XGE 第一版依赖系统 IME 提交文本 | 由系统处理组合候选，XUI 只接收提交文本和候选区域。 |

## 相关示例

- `examples/input`
- `examples/touch`
- `examples/gamepad`
- `examples/text_input`
