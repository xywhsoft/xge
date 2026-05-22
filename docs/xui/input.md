# XUI Input

## 设计定位

Input 是单行文本输入控件。本轮重构保持原有输入、选择、IME、剪贴板、右键菜单、过滤器、错误状态等行为，同时把输入框左右装饰区统一为轻量链表机制：

- 边框、背景、hover、focus、disabled 状态交由 widget 状态样式绘制。
- 文本、占位文字、选择区、光标、装饰区由 Input 自己绘制。
- 错误态只改变输入框自身背景和边框，并通过回调通知业务层；错误提示文字由业务层使用 Label、Toast 或任意布局自行展示。
- 普通 Input 不为装饰区支付固定节点内存，只保存 leading/trailing 链表指针和宽度缓存。
- 装饰区节点由 Input API 创建和释放，调用方不直接管理节点内存。
- 不在 Input 中引入 NumericInput、搜索框之类业务语义；搜索图标、清空按钮、密码眼睛按钮等通过装饰区组合完成。

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
void xgeXuiInputSetTextAlign(xge_xui_input input, int align);
int xgeXuiInputGetTextAlign(xge_xui_input input);

void xgeXuiInputSetColors(xge_xui_input input, uint32_t text, uint32_t background, uint32_t focus, uint32_t cursor);
void xgeXuiInputSetFrameColors(xge_xui_input input, uint32_t background, uint32_t hoverBackground, uint32_t border, uint32_t hoverBorder, uint32_t focusBorder);
void xgeXuiInputSetDisabledColors(xge_xui_input input, uint32_t text, uint32_t background, uint32_t border);
void xgeXuiInputSetErrorColors(xge_xui_input input, uint32_t background, uint32_t border);

void xgeXuiInputSetErrorChange(xge_xui_input input, xge_xui_input_error_proc procError, void* user);
void xgeXuiInputSetError(xge_xui_input input, int error);
int xgeXuiInputGetError(xge_xui_input input);

xge_xui_input_decoration xgeXuiInputDecorationAdd(xge_xui_input input, int side, const xge_xui_input_decoration_desc_t* desc);
void xgeXuiInputDecorationSet(xge_xui_input input, xge_xui_input_decoration decoration, const xge_xui_input_decoration_desc_t* desc);
void xgeXuiInputDecorationRemove(xge_xui_input input, xge_xui_input_decoration decoration);
void xgeXuiInputDecorationClear(xge_xui_input input, int side);
xge_rect_t xgeXuiInputDecorationGetRect(xge_xui_input input, xge_xui_input_decoration decoration);
```

旧的 `xgeXuiInputSetClearButton`、`xgeXuiInputSetClearColors`、`xgeXuiInputSetIcons`、`xgeXuiInputSetIconColor` 便捷 API 已删除；新代码和范例统一使用 decoration API。

## 装饰区

Input 提供两个装饰区链表：

- `XGE_XUI_INPUT_DECORATION_SIDE_LEADING`：文本区左侧。
- `XGE_XUI_INPUT_DECORATION_SIDE_TRAILING`：文本区右侧。

装饰区类型：

- `XGE_XUI_INPUT_DECORATION_ICON`：内置 bitmap mask 图标。
- `XGE_XUI_INPUT_DECORATION_TEXT`：短文本按钮或状态文本。
- `XGE_XUI_INPUT_DECORATION_TEXTURE`：纹理区域。
- `XGE_XUI_INPUT_DECORATION_CLEAR`：内置清空按钮，点击后清空文本并复用 change 流程。
- `XGE_XUI_INPUT_DECORATION_CUSTOM_PAINT`：由调用方自绘。

可见策略：

- `XGE_XUI_INPUT_DECORATION_VISIBLE_ALWAYS`
- `XGE_XUI_INPUT_DECORATION_VISIBLE_WHEN_NOT_EMPTY`
- `XGE_XUI_INPUT_DECORATION_VISIBLE_WHEN_FOCUSED`
- `XGE_XUI_INPUT_DECORATION_VISIBLE_WHEN_FOCUSED_NOT_EMPTY`

装饰区会自动参与文本 padding 计算，并随文本、焦点、布局变化延迟重算。事件上只处理可点击装饰区：`CLEAR` 默认可点击，其他类型设置 `procClick` 后可点击。

```c
xge_xui_input_decoration_desc_t desc;

memset(&desc, 0, sizeof(desc));
desc.iKind = XGE_XUI_INPUT_DECORATION_ICON;
desc.iIcon = XGE_XUI_INPUT_ICON_SEARCH;
desc.fWidth = 24.0f;
desc.fPadding = 6.0f;
desc.iColor = XGE_COLOR_RGBA(80, 112, 140, 255);
xgeXuiInputDecorationAdd(input, XGE_XUI_INPUT_DECORATION_SIDE_LEADING, &desc);

memset(&desc, 0, sizeof(desc));
desc.iKind = XGE_XUI_INPUT_DECORATION_CLEAR;
desc.iVisibleMode = XGE_XUI_INPUT_DECORATION_VISIBLE_WHEN_NOT_EMPTY;
desc.fWidth = 22.0f;
desc.fPadding = 6.0f;
desc.iColor = XGE_COLOR_RGBA(80, 112, 140, 255);
desc.iHoverColor = XGE_COLOR_RGBA(30, 112, 190, 255);
xgeXuiInputDecorationAdd(input, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, &desc);
```

密码框的“眼睛”按钮也使用普通 icon 装饰区实现，点击回调中切换 `xgeXuiInputSetPassword` 即可。

## 文本对齐

Input 支持文本水平对齐：

- `XGE_XUI_INPUT_TEXT_ALIGN_LEFT`：默认，普通文本输入。
- `XGE_XUI_INPUT_TEXT_ALIGN_CENTER`：居中输入框。
- `XGE_XUI_INPUT_TEXT_ALIGN_RIGHT`：数字、尺寸、百分比等值输入。

文本较短且不需要水平滚动时，文本、placeholder、selection、cursor、IME candidate 和鼠标命中会一起按对齐方式偏移；文本超过内容区后仍使用原有水平滚动逻辑，保证光标可见。

右对齐适合配合 trailing text decoration：

```c
xge_xui_input_decoration_desc_t unit;

xgeXuiInputSetTextAlign(input, XGE_XUI_INPUT_TEXT_ALIGN_RIGHT);

memset(&unit, 0, sizeof(unit));
unit.iKind = XGE_XUI_INPUT_DECORATION_TEXT;
unit.iVisibleMode = XGE_XUI_INPUT_DECORATION_VISIBLE_ALWAYS;
unit.sText = "px";
unit.fWidth = 24.0f;
unit.fPadding = 2.0f;
unit.iColor = XGE_COLOR_RGBA(80, 112, 140, 255);
xgeXuiInputDecorationAdd(input, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, &unit);
```

## XSON

`type: "input"` 支持：

- 内容：`text` / `value`、`placeholder`、`textAlign`、`password`、`readonly`、`disabled`、`selection`、`error`
- 文本色：`textColor`、`placeholderColor`、`cursorColor`、`selectionColor`
- 边框背景：`backgroundColor` / `background`、`hoverBackgroundColor`、`borderColor`、`hoverBorderColor`、`focusBorderColor`
- 禁用态：`disabledTextColor`、`disabledBackgroundColor`、`disabledBorderColor`
- 错误态：`errorBackgroundColor`、`errorBorderColor`

`error` 设置初始错误状态；运行时校验由代码调用 `xgeXuiInputSetError` 动态更新，并通过 `xgeXuiInputSetErrorChange` 通知业务层。设置为错误态时会通知；错误态清除时也会通知，便于隐藏业务层提示。
`textAlign` 支持 `left` / `start`、`center` / `middle`、`right` / `end`。

装饰区当前以 C API 为主。XSON 装饰区声明后续统一设计，不在本轮为 Input 单独增加一套临时字段。
