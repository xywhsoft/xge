# XUI 滚动列表

本案例展示 ScrollView 和 ListView 的典型使用。

[返回范例解析](README.md) | [XUI 控件教程](../guide/xui-controls-intro.md) | [XUI API](../api/xui.md)

## ScrollView

```c
xgeXuiScrollViewInit(&scroll, &ui, &scroll_widget);
xgeXuiScrollViewSetContentSize(&scroll, 640.0f, 1800.0f);
```

ScrollView 适合包含任意子控件的滚动区域。

## ListView

```c
static const char* items[] = {
	"assets",
	"render",
	"audio",
	"input"
};

xgeXuiListViewInit(&list, &ui, &list_widget);
xgeXuiListViewSetItems(&list, items, 4);
xgeXuiListViewSetItemHeight(&list, 28.0f);
```

ListView 适合同高度、轻量行项目。

## 常见失败原因

内容画出边界：host clip 实现不正确。

滚动卡顿：每一行都创建复杂控件树，应使用轻量行绘制。

选择状态丢失：更新 items 后没有同步 selected index。
