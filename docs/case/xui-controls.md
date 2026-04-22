# XUI 控件范例

本案例展示在一个 XUI 页面中组合 Button、Label、Input、Toggle、Slider、Progress、ListView 和 Dialog。

[返回范例解析](README.md) | [XUI 控件教程](../guide/xui-controls-intro.md) | [XUI API](../api/xui.md)

## 问题

工具、编辑器和游戏设置页通常需要一组基础控件，而不是每个项目重复手写 GUI。XUI 第一版提供轻量控件集合，并通过 XGE host 绘制。

## 页面结构

```text
root column
  panel title
  row: label + input
  toggle
  slider
  progress
  list view
  dialog
```

## 初始化控件

```c
xge_xui_context_t ui;
xge_xui_widget root;

xgeXuiInit(&ui);
root = xgeXuiRoot(&ui);
xgeXuiWidgetSetLayout(root, XGE_XUI_LAYOUT_COLUMN);
xgeXuiWidgetSetPaddingPx(root, 16.0f, 16.0f, 16.0f, 16.0f);
```

## Button

```c
xge_xui_widget_t button_widget;
xge_xui_button_t button;

xgeXuiWidgetAdd(root, &button_widget);
xgeXuiButtonInit(&button, &ui, &button_widget);
xgeXuiButtonSetText(&button, &font, "Apply");
xgeXuiButtonSetClick(&button, OnApply, NULL);
```

## Input 和 Slider

```c
xgeXuiInputInit(&input, &ui, &input_widget, &font);
xgeXuiInputSetText(&input, "player");

xgeXuiSliderInit(&slider, &ui, &slider_widget);
xgeXuiSliderSetRange(&slider, 0.0f, 100.0f);
xgeXuiSliderSetValue(&slider, 50.0f);
```

## ListView 和 Dialog

```c
static const char* items[] = {
	"Window",
	"Renderer",
	"Audio",
	"Input"
};

xgeXuiListViewInit(&list, &ui, &list_widget);
xgeXuiListViewSetItems(&list, items, 4);
xgeXuiListViewSetSelect(&list, OnSelect, NULL);

xgeXuiDialogInit(&dialog, &ui, &dialog_widget);
xgeXuiDialogSetTitle(&dialog, &font, "Confirm");
xgeXuiDialogSetOpen(&dialog, 0);
```

## 帧流程

```c
xgeXuiUpdate(&ui, xgeGetDelta());
xgeXuiPaint(&ui);
```

输入事件先交给 XUI：

```c
if ( xgeXuiDispatchEvent(&ui, &event) ) {
	return;
}
```

## 关键 API

| API | 作用 |
| --- | --- |
| `xgeXuiButtonInit` | 创建按钮控件 |
| `xgeXuiInputInit` | 创建输入框 |
| `xgeXuiToggleInit` | 创建开关 |
| `xgeXuiSliderInit` | 创建滑条 |
| `xgeXuiProgressInit` | 创建进度条 |
| `xgeXuiListViewInit` | 创建列表 |
| `xgeXuiDialogInit` | 创建对话框 |

## 常见问题

如果控件不显示，检查 widget 是否被添加到 root，尺寸是否为 0，父控件是否 visible。

如果控件不响应，检查事件是否进入 `xgeXuiDispatchEvent`，以及控件是否 enabled。

如果 APP 模式下修改控件不刷新，调用 `xgeXuiRefreshRequest` 或对应 widget dirty 标记。
