# APP 模式刷新

本案例展示不依赖连续游戏循环的手动刷新方式。

[返回范例解析](README.md) | [运行模式教程](../guide/run-mode-intro.md) | [Graphics API](../api/graphics.md)

## 问题

工具、编辑器和普通 APP 不适合永远 60 FPS 渲染。APP 模式应在状态变化时刷新。

## 标记刷新

```c
xgeInvalidateRect((xge_rect_t){ 0.0f, 0.0f, 400.0f, 240.0f });
xgeXuiRefreshRequest(&ui);
```

## 手动提交

```c
if ( xgeXuiRefreshNeeded(&ui) || xgeDirtyRectCount() > 0 ) {
	xgeBegin();
	xgeClear(xgeColorRGBA(18, 20, 26, 255));
	xgeXuiPaint(&ui);
	xgeEnd();
	xgeFlush();

	xgeDirtyRectClear();
	xgeXuiRefreshClear(&ui);
}
```

## 常见失败原因

界面不刷新：控件状态变化后没有请求 refresh。

CPU 占用高：仍然无条件每帧 flush。

局部刷新残影：dirty 区域没有完整重绘。
