# 输入事件

本案例展示键盘、鼠标、触控、文本输入和手柄状态的基础读取方式。

[返回范例解析](README.md) | [输入教程](../guide/input-intro.md) | [Input API](../api/input.md)

## 问题

XGE 面向多平台，输入系统需要同时覆盖桌面、移动、小程序和板卡。业务层应尽量使用 XGE 的统一 key、mouse、touch、gamepad API，减少平台分支。

## 键盘

```c
if ( xgeKeyPressed(XGE_KEY_SPACE) ) {
	Jump();
}

if ( xgeKeyDown(XGE_KEY_LEFT) ) {
	Move(-1.0f);
}
```

`Pressed` 表示当前帧按下，`Down` 表示持续按住，`Released` 表示当前帧释放。

## 鼠标

```c
float x;
float y;
float wheel_x;
float wheel_y;

xgeMouseGet(&x, &y);
xgeMouseGetWheel(&wheel_x, &wheel_y);

if ( xgeMouseDown(XGE_MOUSE_LEFT) ) {
	DragAt(x, y);
}
```

鼠标坐标使用当前逻辑坐标映射后的值，像素级工具可以结合 viewport/camera 映射自行转换。

## 触控

```c
int i;
int count;

count = xgeTouchGetCount();
for ( i = 0; i < count; ++i ) {
	xge_touch_point_t point;

	if ( xgeTouchGet(i, &point) == XGE_OK ) {
		HandleTouch(point.iId, point.fX, point.fY, point.iPhase);
	}
}
```

多点触控依赖稳定 touch id。不要用数组下标表达同一根手指的生命周期。

## 文本输入

```c
uint32_t cp;

cp = xgeTextGet();
if ( cp != 0 ) {
	AppendCodepoint(cp);
}
```

文本输入来自系统 IME 或平台桥接，适合输入框；游戏控制仍应使用 key API。

## 手柄

```c
xge_gamepad_state_t state;

if ( xgeGamepadConnected(0) && xgeGamepadGetState(0, &state) == XGE_OK ) {
	if ( xgeGamepadButtonPressed(0, XGE_GAMEPAD_A) ) {
		Confirm();
	}
	MoveByAxis(xgeGamepadAxis(0, 0));
}
```

## 关键 API

| API | 作用 |
| --- | --- |
| `xgeKeyDown` | 查询按键是否按住 |
| `xgeKeyPressed` | 查询当前帧是否按下 |
| `xgeMouseGet` | 获取鼠标位置 |
| `xgeTouchGet` | 获取触控点 |
| `xgeTextGet` | 获取文本输入 codepoint |
| `xgeGamepadGetState` | 获取手柄状态 |

## 常见问题

如果输入框无法输入中文，确认平台后端已接入系统 IME 文本事件，而不是只处理 keydown。

如果触控在高 DPI 设备上偏移，检查设备像素比和逻辑坐标映射。
