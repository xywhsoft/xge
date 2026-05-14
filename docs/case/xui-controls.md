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
| `xgeXuiSliderInit` | 创建滑条 |
| `xgeXuiProgressInit` | 创建进度条 |
| `xgeXuiListViewInit` | 创建列表 |
| `xgeXuiDialogInit` | 创建对话框 |

## 扩展控件示例

新增控件采用独立 lab 维护，便于按控件验证绘制、输入、状态和回调：

| 示例 | 覆盖内容 |
| --- | --- |
| `examples/xui_numeric_input_lab` | 数值输入、范围、步进、spinner、格式化和错误态 |
| `examples/xui_input_standard_lab` | Input Change、Submit、MaxLength 和 readonly 标准契约 |
| `examples/xui_message_box_lab` | 消息框打开、按钮结果、Escape、Enter 和一次性回调 |
| `examples/xui_toolbar_lab` | 工具按钮、toggle、分隔项、禁用态、键盘和垂直布局 |
| `examples/xui_status_bar_lab` | 状态项布局、进度项、点击项和禁用态 |
| `examples/xui_tree_view_lab` | 树节点展开折叠、选择、键盘导航和回调 |
| `examples/xui_table_view_lab` | 表格列、虚拟行数据、选择、排序和列宽 |
| `examples/xui_property_grid_lab` | 分类、属性行、只读、已改动、错误和编辑器类型 |
| `examples/xui_breadcrumb_lab` | 路径段、分隔符、点击选择和折叠显示 |
| `examples/xui_accordion_lab` | 折叠面板、single/multiple 模式和选择回调 |
| `examples/xui_color_picker_lab` | swatch、RGBA 字段、hex 输入和调色板 |
| `examples/xui_toast_lab` | 通知队列、类型、关闭、过期和位置 |

对应的 XSON 示例位于 `examples/xui_xson_*_lab`，覆盖页面声明加载、字段解析、样式字段和未接入事件报错路径。

## 常见问题

如果控件不显示，检查 widget 是否被添加到 root，尺寸是否为 0，父控件是否 visible。

如果控件不响应，检查事件是否进入 `xgeXuiDispatchEvent`，以及控件是否 enabled。

如果 APP 模式下修改控件不刷新，调用 `xgeXuiRefreshRequest` 或对应 widget dirty 标记。
