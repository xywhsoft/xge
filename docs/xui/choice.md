# XUI Choice Controls

本文档覆盖 `CheckBox`、`Radio` / `RadioGroup`、`Toggle` 三个选择控件。

## 设计思路

- 三个控件的选中状态统一由 `Widget` 的 `XGE_XUI_STATE_CHECKED` 托管，控件本体不再保存第二份 checked 状态。
- 默认关闭 widget 背景和边框。需要 hover、active、focus、checked、disabled 外框样式时，使用 widget 状态样式显式配置。
- `CheckBox` 和 `Radio` 是 `indicator + text` 模型，文本与指示器共同组成一行盒，按 `max(indicatorHeight, textHeight)` 垂直居中，保证自定义素材高度变化时文本仍然对齐。
- `Toggle` 只表示开关本体，不附带外部标题。需要标题时使用 `Label + Toggle` 的布局组合。
- `Toggle` 支持可选内嵌状态文本。未设置文本时不占用文本空间，保持紧凑开关尺寸。
- 三个控件都支持双状态纹理。纹理存在时优先绘制纹理，缺失时回退到内置矢量绘制。
- `Toggle` 使用纹理后不绘制内嵌文本，避免把简单游戏素材升级成九宫格或复杂布局模型。
- 三个控件都支持懒缓存。状态、尺寸、DPI、文本、字体、颜色、纹理或 widget visual state 变化时缓存失效。

## CheckBox API

```c
int xgeXuiCheckBoxInit(xge_xui_checkbox checkbox, xge_xui_context ctx, xge_xui_widget widget);
void xgeXuiCheckBoxUnit(xge_xui_checkbox checkbox);

void xgeXuiCheckBoxSetChange(xge_xui_checkbox checkbox, xge_xui_checked_proc proc, void* user);
void xgeXuiCheckBoxSetText(xge_xui_checkbox checkbox, xge_font font, const char* text);
void xgeXuiCheckBoxSetTextColor(xge_xui_checkbox checkbox, uint32_t color);

void xgeXuiCheckBoxSetChecked(xge_xui_checkbox checkbox, int checked);
int xgeXuiCheckBoxGetChecked(xge_xui_checkbox checkbox);

void xgeXuiCheckBoxSetColors(xge_xui_checkbox checkbox, uint32_t boxColor, uint32_t checkedColor);
void xgeXuiCheckBoxSetTextures(xge_xui_checkbox checkbox, xge_texture uncheckedTexture, xge_rect_t uncheckedSrc, xge_texture checkedTexture, xge_rect_t checkedSrc);
void xgeXuiCheckBoxSetIndicatorSize(xge_xui_checkbox checkbox, float size);
void xgeXuiCheckBoxSetGap(xge_xui_checkbox checkbox, float gap);
void xgeXuiCheckBoxSetCacheMode(xge_xui_checkbox checkbox, int mode);
```

## Radio API

```c
void xgeXuiRadioGroupInit(xge_xui_radio_group group);
void xgeXuiRadioGroupUnit(xge_xui_radio_group group);
void xgeXuiRadioGroupSetChange(xge_xui_radio_group group, xge_xui_select_proc proc, void* user);
void xgeXuiRadioGroupSetSelected(xge_xui_radio_group group, int value);
int xgeXuiRadioGroupGetSelected(xge_xui_radio_group group);

int xgeXuiRadioInit(xge_xui_radio radio, xge_xui_context ctx, xge_xui_widget widget);
void xgeXuiRadioUnit(xge_xui_radio radio);
void xgeXuiRadioSetGroup(xge_xui_radio radio, xge_xui_radio_group group, int value);
void xgeXuiRadioSetChange(xge_xui_radio radio, xge_xui_checked_proc proc, void* user);
void xgeXuiRadioSetText(xge_xui_radio radio, xge_font font, const char* text);
void xgeXuiRadioSetTextColor(xge_xui_radio radio, uint32_t color);

void xgeXuiRadioSetChecked(xge_xui_radio radio, int checked);
int xgeXuiRadioGetChecked(xge_xui_radio radio);

void xgeXuiRadioSetColors(xge_xui_radio radio, uint32_t ringColor, uint32_t checkedColor);
void xgeXuiRadioSetTextures(xge_xui_radio radio, xge_texture uncheckedTexture, xge_rect_t uncheckedSrc, xge_texture checkedTexture, xge_rect_t checkedSrc);
void xgeXuiRadioSetIndicatorSize(xge_xui_radio radio, float size);
void xgeXuiRadioSetGap(xge_xui_radio radio, float gap);
void xgeXuiRadioSetCacheMode(xge_xui_radio radio, int mode);
```

`RadioGroup` 负责互斥选择和键盘方向键导航。分组内再次选择已选中的 radio 不取消选择；需要清空选择时调用 `xgeXuiRadioGroupSetSelected(group, -1)`。

## Toggle API

```c
int xgeXuiToggleInit(xge_xui_toggle toggle, xge_xui_context ctx, xge_xui_widget widget);
void xgeXuiToggleUnit(xge_xui_toggle toggle);

void xgeXuiToggleSetChange(xge_xui_toggle toggle, xge_xui_checked_proc proc, void* user);
void xgeXuiToggleSetChecked(xge_xui_toggle toggle, int checked);
int xgeXuiToggleGetChecked(xge_xui_toggle toggle);

void xgeXuiToggleSetColors(xge_xui_toggle toggle, uint32_t trackColor, uint32_t checkedColor, uint32_t knobColor, uint32_t trackBorderColor);
void xgeXuiToggleSetInnerText(xge_xui_toggle toggle, xge_font font, const char* uncheckedText, const char* checkedText);
void xgeXuiToggleSetInnerTextColor(xge_xui_toggle toggle, uint32_t uncheckedColor, uint32_t checkedColor);
void xgeXuiToggleSetTextures(xge_xui_toggle toggle, xge_texture uncheckedTexture, xge_rect_t uncheckedSrc, xge_texture checkedTexture, xge_rect_t checkedSrc);
void xgeXuiToggleSetMetrics(xge_xui_toggle toggle, float trackWidth, float trackHeight, float knobInset, float textPadding, float textGap);
void xgeXuiToggleSetCacheMode(xge_xui_toggle toggle, int mode);
```

## XSON

```json
{
  "type": "toggle",
  "checked": true,
  "uncheckedText": "OFF",
  "checkedText": "ON",
  "trackColor": "#DCE7F0FF",
  "checkedColor": "#35AEEAFF",
  "knobColor": "#FFFFFFFF",
  "cacheMode": "auto"
}
```

`checkbox` 和 `radio` 支持：

```json
{
  "type": "checkbox",
  "text": "Remember",
  "checked": true,
  "indicatorSize": 28,
  "uncheckedTexture": "@textures.checkOff",
  "checkedTexture": "@textures.checkOn",
  "uncheckedSrc": [0, 0, 28, 28],
  "checkedSrc": [0, 0, 28, 28]
}
```

## 示例

`examples/xui_choice` 展示三种控件的未选中、选中、禁用、自定义纹理状态，`Toggle` 同时展示带内嵌文字和不带内嵌文字两种默认样式，并包含一个 `RadioGroup` 分组选择示例。
