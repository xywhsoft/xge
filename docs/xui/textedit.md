# XUI TextEdit

## 设计定位

TextEdit 是多行文本编辑控件。本轮重构是保守重构：保留已有文本缓冲、撤销重做、自动换行、行号、查找高亮、滚动、IME、右键菜单等能力，主要统一外观和配色。

- widget 负责背景、边框、hover、focus、disabled 状态。
- TextEdit 负责正文、选择区、当前行、查找高亮、行号、内置滚动条和光标。
- 颜色字段被显式拆开，避免硬编码颜色继续扩散。

## API

```c
int xgeXuiTextEditInit(xge_xui_text_edit edit, xge_xui_context ctx, xge_xui_widget widget, xge_font font);
void xgeXuiTextEditUnit(xge_xui_text_edit edit);

void xgeXuiTextEditSetText(xge_xui_text_edit edit, const char* text);
const char* xgeXuiTextEditGetText(xge_xui_text_edit edit);
void xgeXuiTextEditSetReadonly(xge_xui_text_edit edit, int readonly);
void xgeXuiTextEditSetWordWrap(xge_xui_text_edit edit, int wordWrap);
void xgeXuiTextEditSetLineNumbers(xge_xui_text_edit edit, int enabled, float width);
void xgeXuiTextEditSetScrollbarMode(xge_xui_text_edit edit, int mode);

void xgeXuiTextEditSetColors(xge_xui_text_edit edit, uint32_t text, uint32_t background, uint32_t focus, uint32_t cursor);
void xgeXuiTextEditSetFrameColors(xge_xui_text_edit edit, uint32_t background, uint32_t hoverBackground, uint32_t border, uint32_t hoverBorder, uint32_t focusBorder);
void xgeXuiTextEditSetDisabledColors(xge_xui_text_edit edit, uint32_t text, uint32_t background, uint32_t border);
void xgeXuiTextEditSetReserveColors(xge_xui_text_edit edit, uint32_t findHighlight, uint32_t lineNumberText, uint32_t lineNumberBackground);
void xgeXuiTextEditSetCurrentLineColor(xge_xui_text_edit edit, uint32_t color);
void xgeXuiTextEditSetScrollbarColors(xge_xui_text_edit edit, uint32_t track, uint32_t border, uint32_t thumb);
```

## XSON

`type: "textEdit"` / `"textedit"` 支持：

- 内容：`text` / `value`
- 行为：`readonly`、`wordWrap`、`lineNumbers`、`lineNumberWidth`、`scrollbarMode`
- 文本色：`textColor`、`cursorColor`、`selectionColor`
- 边框背景：`backgroundColor` / `background`、`hoverBackgroundColor`、`borderColor`、`hoverBorderColor`、`focusBorderColor`
- 编辑辅助色：`currentLineColor`、`findHighlightColor`、`lineNumberTextColor`、`lineNumberBackgroundColor`
- 滚动条色：`scrollbarTrackColor`、`scrollbarBorderColor`、`scrollbarThumbColor`
- 禁用态：`disabledTextColor`、`disabledBackgroundColor`、`disabledBorderColor`

TextEdit 的 XSON 只描述初始内容和视觉配置。动态编辑、撤销重做、查找高亮数据仍由代码侧控制。

