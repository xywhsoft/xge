# XUI 控件入门

本教程介绍 XUI 第一版内置控件的使用方式。

[返回教程索引](README.md) | [XUI API](../api/xui.md) | [XUI 范例](../case/xui-controls.md)

## 控件模型

XUI 控件通常由两部分组成：

- `xge_xui_widget_t`：树节点、布局、状态、事件和 paint 挂载点。
- 控件结构：Button、Label、Input、Slider 等控件自己的状态。

控件初始化时会把事件和绘制回调绑定到 widget。

## Button

```c
xge_xui_widget_t button_widget;
xge_xui_button_t button;

xgeXuiWidgetAdd(parent, &button_widget);
xgeXuiButtonInit(&button, &ui, &button_widget);
xgeXuiButtonSetText(&button, &font, "Start");
xgeXuiButtonSetClick(&button, OnStartClick, NULL);
```

Button 适合命令触发，内部处理 hover、active、focus、disabled 状态。

## Label

```c
xge_xui_label_t label;

xgeXuiLabelInit(&label, &label_widget, &font, "Score: 0");
xgeXuiLabelSetColor(&label, xgeColorRGBA(240, 240, 240, 255));
xgeXuiLabelSetAlign(&label, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
```

Label 可提供内容测量，用于 content size 布局。

## Input

```c
xge_xui_input_t input;

xgeXuiInputInit(&input, &ui, &input_widget, &font);
xgeXuiInputSetText(&input, "player");
```

Input 使用系统 IME 提交的文本事件，不在 XUI 内部实现完整输入法。

## Toggle、Slider、Progress

```c
xgeXuiToggleSetChecked(&toggle, 1);
xgeXuiSliderSetRange(&slider, 0.0f, 100.0f);
xgeXuiSliderSetValue(&slider, 50.0f);
xgeXuiProgressSetValue(&progress, 25.0f);
```

Toggle 表达开关，Slider 表达可交互数值，Progress 表达只读进度。

## Panel、ScrollView、ListView、Dialog

Panel 用于背景和标题区域。ScrollView 用于裁剪和滚动内容。ListView 用于大量同高度条目。Dialog 用于模态浮层。

```c
xgeXuiScrollViewSetContentSize(&scroll, 640.0f, 1200.0f);
xgeXuiListViewSetItems(&list, items, item_count);
xgeXuiDialogSetOpen(&dialog, 1);
```

## 状态控制

```c
xgeXuiWidgetSetVisible(widget, 1);
xgeXuiWidgetSetEnabled(widget, 0);
xgeXuiWidgetSetFocusable(widget, 1);
```

状态变化后，控件通常会请求 repaint。自定义控件需要主动调用 `xgeXuiWidgetMarkPaint`。

## 常见错误

不要直接修改控件内部字段绕过 setter。setter 会维护 dirty、事件和派生状态。

不要在 disabled 控件上依赖点击回调。

不要让 ListView 的每一行都创建复杂树。大量列表应优先使用轻量行绘制。

## 下一步

- 看完整例子用 [XUI 控件范例](../case/xui-controls.md)。
- 文本输入读 [XUI 文本输入与 IME 入门](xui-input-ime-intro.md)。
