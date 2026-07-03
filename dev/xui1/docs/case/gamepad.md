# 手柄输入

本案例展示手柄连接、按钮和轴输入读取。

[返回范例解析](README.md) | [输入教程](../guide/input-intro.md) | [Input API](../api/input.md)

## 查询连接

```c
if ( !xgeGamepadConnected(0) ) {
	return;
}
```

## 按钮

```c
if ( xgeGamepadButtonPressed(0, XGE_GAMEPAD_A) ) {
	Confirm();
}

if ( xgeGamepadButtonDown(0, XGE_GAMEPAD_DPAD_LEFT) ) {
	MoveMenu(-1);
}
```

## 摇杆

```c
float x;

x = xgeGamepadAxis(0, 0);
if ( x > -0.15f && x < 0.15f ) {
	x = 0.0f;
}
MovePlayer(x);
```

第一版轴编号保持轻量，后续可在文档中补充推荐映射。

## 常见失败原因

按钮无响应：平台后端没有把原生手柄映射到 XGE bitmask。

摇杆漂移：业务层没有设置 dead zone。

热插拔异常：没有处理 connected/disconnected 事件。
