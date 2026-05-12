# XUI Input

## 设计定位

Input 是单行文本输入控件。本轮重构保持原有输入、选择、IME、剪贴板、右键菜单、过滤器、错误提示等行为不变，只统一视觉层：

- 边框、背景、hover、focus、disabled 状态交由 widget 状态样式绘制。
- 文本、占位文字、选择区、光标、图标、清除按钮、错误文字仍由 Input 自己绘制。
- 不背兼容包袱，但不重写已稳定的输入流程。

默认风格应符合 XUI 当前浅蓝技术风：浅背景、清晰边框、蓝色焦点边框、低饱和禁用态。

## API

```c
int xgeXuiInputInit(xge_xui_input input, xge_xui_context ctx, xge_xui_widget widget, xge_font font);
void xgeXuiInputUnit(xge_xui_input input);

void xgeXuiInputSetText(xge_xui_input input, const char* text);
const char* xgeXuiInputGetText(xge_xui_input input);
void xgeXuiInputSetPlaceholder(xge_xui_input input, const char* text);
void xgeXuiInputSetPassword(xge_xui_input input, int enabled);
void xgeXuiInputSetReadonly(xge_xui_input input, int readonly);
void xgeXuiInputSetDisabled(xge_xui_input input, int disabled);
void xgeXuiInputSetMaxLength(xge_xui_input input, int maxLength);

void xgeXuiInputSetColors(xge_xui_input input, uint32_t text, uint32_t background, uint32_t focus, uint32_t cursor);
void xgeXuiInputSetFrameColors(xge_xui_input input, uint32_t background, uint32_t hoverBackground, uint32_t border, uint32_t hoverBorder, uint32_t focusBorder);
void xgeXuiInputSetDisabledColors(xge_xui_input input, uint32_t text, uint32_t background, uint32_t border);
void xgeXuiInputSetErrorColors(xge_xui_input input, uint32_t background, uint32_t border, uint32_t text);
```

## XSON

`type: "input"` 支持：

- 内容：`text` / `value`、`placeholder`、`password`、`readonly`、`disabled`、`selection`、`error`、`errorText`
- 文本色：`textColor`、`placeholderColor`、`cursorColor`、`selectionColor`
- 边框背景：`backgroundColor` / `background`、`hoverBackgroundColor`、`borderColor`、`hoverBorderColor`、`focusBorderColor`
- 禁用态：`disabledTextColor`、`disabledBackgroundColor`、`disabledBorderColor`
- 错误态：`errorBackgroundColor`、`errorBorderColor`、`errorTextColor`

`error` 设置初始错误状态，`errorText` 设置错误提示文本；运行时校验仍建议由代码调用 `xgeXuiInputSetError` 动态更新。
