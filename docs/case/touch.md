# 多点触控

本案例展示如何读取多点触控并跟踪同一根手指的生命周期。

[返回范例解析](README.md) | [输入教程](../guide/input-intro.md) | [Input API](../api/input.md)

## 问题

移动端、小程序和触摸屏板卡都需要多点触控。稳定的 touch id 比数组下标更重要。

## 遍历触控点

```c
int i;
int count;

count = xgeTouchGetCount();
for ( i = 0; i < count; ++i ) {
	xge_touch_point_t point;

	if ( xgeTouchGet(i, &point) == XGE_OK ) {
		DrawTouchPoint(point.fX, point.fY, point.iId);
	}
}
```

## 按 id 查找

```c
xge_touch_point_t point;

if ( xgeTouchFind(active_id, &point) == XGE_OK ) {
	UpdateDrag(point.fX, point.fY);
}
```

## 常见失败原因

手指跳变：业务用数组下标跟踪手指，而不是用 touch id。

高 DPI 偏移：宿主传入了 framebuffer 像素坐标，但 XGE 期望逻辑坐标。

结束事件丢失：平台后端没有在 cancel/end 时清理触控点。
