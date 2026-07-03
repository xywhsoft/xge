# 文本输入

本案例展示 XUI Input 控件如何接收系统 IME 文本事件。

[返回范例解析](README.md) | [XUI IME 教程](../guide/xui-input-ime-intro.md) | [XUI API](../api/xui.md)

## 问题

文本输入不能只依赖 keydown。中文等输入需要系统 IME 提交文本和候选框位置。

## 创建 Input

```c
xge_xui_widget_t widget;
xge_xui_input_t input;

xgeXuiWidgetAdd(root, &widget);
xgeXuiWidgetSetSize(&widget, xgeXuiSizePx(240.0f), xgeXuiSizePx(36.0f));
xgeXuiInputInit(&input, &ui, &widget, &font);
xgeXuiInputSetText(&input, "");
```

## 事件流

```c
if ( xgeXuiDispatchEvent(&ui, &event) ) {
	return;
}
```

XUI 会把文本事件分发给当前焦点 Input。

## 候选框位置

```c
xge_rect_t rect;

rect = xgeXuiInputGetCandidateRect(&input);
```

平台后端将这个矩形交给系统 IME 或小程序宿主。

## 常见失败原因

只能输入英文：平台后端只转发了 key 事件，没有转发 text/IME 事件。

候选框位置错误：没有使用当前输入框的 candidate rect。

光标错位：字体测量、DPI 或文本索引规则不一致。
