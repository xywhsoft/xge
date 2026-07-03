# XUI 控件范例

本案例展示在一个 XUI 页面中组合 Button、Label、Input、Toggle、Slider、Progress、ListView 和 MsgBox。

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
  msgbox
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

## ListView 和 MsgBox

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

xgeXuiMsgBoxInit(&msgbox, &ui, &msgbox_widget);
xgeXuiMsgBoxSetText(&msgbox, &font, "Confirm", "Apply changes?");
xgeXuiMsgBoxSetButtons(&msgbox, XGE_XUI_MSG_BOX_BUTTON_OK_CANCEL);
xgeXuiMsgBoxSetOpen(&msgbox, 0);
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
| `xgeXuiMsgBoxInit` | 创建消息弹窗 |

## 扩展控件示例

新范式控件采用独立 lab 维护，便于按控件验证绘制、输入、状态、回调和 XSON 接入。PropertyGrid 和 DockPanel/DockLayout 也归入这个口径：前者是属性表专用复合控件，后者是工作台级停靠面板系统。

| 示例 | 覆盖内容 |
| --- | --- |
| `examples/xui_numericinput` | 数值输入、范围、步进、spinner、格式化和错误态 |
| `examples/xui_numericinput_xson` | NumericInput XSON 加载、样式和状态字段 |
| `examples/xui_combobox` | ComboBox 字符串条目、结构化条目、禁用项、选择、固定高度和自动向上弹出 |
| `examples/xui_combobox_xson` | ComboBox XSON 加载、value、popup 高度、条目状态和弹出方向 |
| `examples/xui_input_standard_lab` | Input Change、Submit、MaxLength 和 readonly 标准契约 |
| `examples/xui_msgbox_inputbox` | MsgBox/InputBox 打开、按钮结果、Escape、Enter、阻塞和非阻塞窗口 |
| `examples/xui_msgtip` | MsgTip 类型、自动过期、点击关闭、长文本换行和自定义颜色 |
| `examples/xui_toolbar_lab` | 工具按钮、toggle、分隔项、禁用态、键盘和垂直布局 |
| `examples/xui_status_bar_lab` | 状态项布局、进度项、点击项和禁用态 |
| `examples/xui_treeview` / `examples/xui_treeview_xson` | 树节点展开折叠、选择、禁用、勾选、滚动条模式、自定义行绘制和 XSON 加载 |
| `examples/xui_tableview` / `examples/xui_tableview_xson` | 表格列、行/单元格选择、横纵滚动、合并单元格、自定义渲染和 XSON 加载 |
| `examples/xui_property_grid_lab` | 分类、属性行、只读、已改动、错误和编辑器类型 |
| `examples/xui_dockpanel_lab` / `examples/xui_dockpanel_xson` | DockLayout region、split tree、pane tab、floating dockwindow、indicator、preview、auto-hide 和 XSON 初始布局 |
| `examples/xui_accordion_lab` | 折叠面板、single/multiple 模式和选择回调 |
| `examples/xui_colorpicker` / `examples/xui_colorpicker_xson` | swatch、RGBA 字段、hex 输入、Alpha 开关、调色板和 XSON 加载 |
| `examples/xui_toast` | Context 级通知队列、类型、点击回调、关闭原因、过期和位置 |

对应的 XSON 示例位于 `examples/xui_xson_*_lab`，覆盖页面声明加载、字段解析、样式字段和未接入事件报错路径。

## 常见问题

如果控件不显示，检查 widget 是否被添加到 root，尺寸是否为 0，父控件是否 visible。

如果控件不响应，检查事件是否进入 `xgeXuiDispatchEvent`，以及控件是否 enabled。

如果 APP 模式下修改控件不刷新，调用 `xgeXuiRefreshRequest` 或对应 widget dirty 标记。
