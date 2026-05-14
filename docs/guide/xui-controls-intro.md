# XUI 控件入门

本教程介绍 XUI 第一版内置控件的使用方式。

[返回教程索引](README.md) | [XUI API](../api/xui.md) | [XUI 范例](../case/xui-controls.md)

> 当前 guide 描述第一版 API。维护者开发新控件或重构旧控件时，以 `dev/docs/XUI Widget V2基础设计.md` 为准；Widget V2 会统一 clip、Z、事件路由、焦点、TAB、滚动、IME 和盒模型。

## 控件模型

XUI 控件通常由两部分组成：

- `xge_xui_widget_t`：树节点、布局、状态、事件和 paint 挂载点。
- 控件结构：Button、Label、Input、Slider 等控件自己的状态。

控件初始化时会把事件和绘制回调绑定到 widget。

第一版 API 是“widget + 控件状态”的组合模型。Widget V2 仍保留这个方向，但每个 widget 必须有明确 role：Control、Container、Viewport 或 Overlay。普通 Control 默认不再被视为可任意承载子节点的容器；滚动、浮层、虚拟列表等能力由对应基础层统一处理。

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
xgeXuiLabelSetDisabledColor(&label, xgeColorRGBA(150, 156, 164, 255));
xgeXuiLabelSetAlign(&label, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
xgeXuiLabelSetUnderline(&label, 0);
```

Label 可提供内容测量，用于 content size 布局。默认透明、无边框、不可聚焦、IME disabled；背景、边框、圆角和 padding 直接使用 Widget 样式 API。Label 默认使用单个惰性渲染缓存，文本、字体、颜色、对齐、下划线、content 尺寸、DIP scale 或 enabled 状态变化时失效；无 render target 环境会自动回退到直接文本绘制。

## Input

```c
xge_xui_input_t input;

xgeXuiInputInit(&input, &ui, &input_widget, &font);
xgeXuiInputSetText(&input, "player");
```

Input 使用系统 IME 提交的文本事件，不在 XUI 内部实现完整输入法。

Widget V2 后，IME 由焦点系统和 `imeMode` 统一管理：普通非文本控件默认不申请 IME，文本输入控件显式申请 IME，Password 等场景可禁用 IME。

Input 的轻量装饰能力用于搜索图标、清空按钮、密码显示按钮、单位后缀和状态图标等内嵌元素。后续新范例统一使用 decoration API 表达这些效果；旧 clear button 和 prefix/suffix icon API 不再作为新代码口径。带 suggestion popup 的搜索框属于 ext/组合控件，不作为 core 控件提供；带 stepper 的 NumericInput 等复杂交互仍按专用控件处理，不把业务语义塞进 Input。

## NumericInput、ColorPicker、DatePicker

```c
xgeXuiNumericInputSetRange(&number, 0.0f, 100.0f);
xgeXuiColorPickerSetHex(&color, "#4E9FDCFF");
xgeXuiDatePickerSetDate(&date, 2026, 5, 8);
```

NumericInput 负责常见数值录入。ColorPicker 负责基础颜色选择，支持 swatch、RGBA 字段、hex 和 palette。DatePicker 负责基础日期选择，支持月历、范围限制、鼠标选择和键盘导航。

XSON 中分别使用 `type: "numericInput"`、`type: "colorPicker"`、`type: "datePicker"`。搜索输入使用 `type: "input"` 搭配 decoration API 或后续 ext 组合控件。ColorPicker 的 `color` 字段表示当前颜色值，文字颜色使用 `textColor`。

## Toggle、Slider、Progress

```c
xgeXuiCheckBoxSetChecked(&checkbox, 1);
xgeXuiSliderSetRange(&slider, 0.0f, 100.0f);
xgeXuiSliderSetValue(&slider, 50.0f);
xgeXuiProgressSetValue(&progress, 25.0f);
```

Toggle 表达开关，Slider 表达可交互数值，Progress 表达只读进度。

## Panel、ScrollView、ListView、Dialog

Panel 用于背景和标题区域。ScrollView 用于裁剪和滚动内容。ListView 用于大量同高度条目。Dialog 用于模态浮层。

Widget V2 后，ScrollView、ListView、TreeView 和 TableView 必须复用 ScrollViewBase / VirtualScrollViewBase；内容拖拽滚动默认关闭，由具体控件显式开启，避免干扰地图、画布、编辑器类控件的鼠标事件。

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
- 自定义控件和控件重构读 [XUI Widget 事件入门](xui-widget-events-intro.md)。
- 文本输入读 [XUI 文本输入与 IME 入门](xui-input-ime-intro.md)。
