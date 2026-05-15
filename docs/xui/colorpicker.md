# XUI ColorPicker

`ColorPicker` 是表单化颜色选择控件。它的主控件外观看起来像普通输入框，点击后打开颜色选择弹层。

## 设计思路

- 主控件使用 `widget` 的边框、背景、悬停、焦点和禁用状态。
- 左侧表单区域显示当前颜色，色块距离边框 2px，中央叠加 `#RRGGBB` 文本。
- 右侧是和 `ComboBox` 一致的 V 型按钮区域，不单独绘制按钮边框。
- 弹层使用 XUI `Popup` 基础设施承载；ColorPicker 只申请面板内容尺寸，弹出位置、方向回退、外部点击关闭和 Esc 关闭都由 `Popup` 处理。
- 色相条是 ColorPicker 的私有区域，不复用通用 `Slider`，避免把普通滑块扩展成渐变轨道、无 fill、特殊指示器的复杂控件。
- 弹层只显示 `Old` 和 `New` 两个色块。`Old` 是打开弹层时的颜色，`New` 是当前颜色。
- 默认以 RGB 为主，公开显示 `#RRGGBB`。
- 可通过 `alphaEnabled` / `xgeXuiColorPickerSetAlphaEnabled` 开启 Alpha 通道。开启后显示和编辑 `#RRGGBBAA`，弹层增加 A 通道输入和滑条。

## 交互

- 点击主控件打开或关闭弹层。
- 弹层默认锚定主控件左下角向右下弹出；窗口边缘空间不足时，使用 `Popup` 的统一回退策略。
- 在明度/饱和度矩形中点击或拖动，更新当前颜色。
- 在色相条中点击或拖动，更新色相。
- 点击 RGB/A 数值框可以输入数值。
- 拖动 RGB/A 行右侧的小滑条可以调整单个通道。
- 点击常用颜色直接应用到当前颜色。
- 所有颜色修改即时触发 `onChange` 回调。

## API

```c
int xgeXuiColorPickerInit(xge_xui_color_picker picker, xge_xui_context context, xge_xui_widget widget, xge_font font);
void xgeXuiColorPickerUnit(xge_xui_color_picker picker);

void xgeXuiColorPickerSetChange(xge_xui_color_picker picker, xge_xui_color_proc proc, void* user);

void xgeXuiColorPickerSetColor(xge_xui_color_picker picker, uint32_t color);
uint32_t xgeXuiColorPickerGetColor(xge_xui_color_picker picker);

void xgeXuiColorPickerSetRGBA(xge_xui_color_picker picker, int r, int g, int b, int a);
void xgeXuiColorPickerGetRGBA(xge_xui_color_picker picker, int* r, int* g, int* b, int* a);

void xgeXuiColorPickerSetAlphaEnabled(xge_xui_color_picker picker, int enabled);
int xgeXuiColorPickerGetAlphaEnabled(xge_xui_color_picker picker);

int xgeXuiColorPickerSetHex(xge_xui_color_picker picker, const char* hex);
const char* xgeXuiColorPickerGetHex(xge_xui_color_picker picker);

void xgeXuiColorPickerSetPalette(xge_xui_color_picker picker, const uint32_t* colors, int count);
int xgeXuiColorPickerGetPaletteCount(xge_xui_color_picker picker);

void xgeXuiColorPickerSetColors(
    xge_xui_color_picker picker,
    uint32_t background,
    uint32_t panel,
    uint32_t border,
    uint32_t text,
    uint32_t accent,
    uint32_t field);
```

## XSON

```json
{
  "type": "colorPicker",
  "rect": [144, 40, 240, 30],
  "value": "#82B737FF",
  "alphaEnabled": true,
  "palette": [
    "#00000000", "#000000", "#ffffff", "#949ea8",
    "#e13a46", "#e67e22", "#f4c936", "#4aa55b",
    "#2bb8cb", "#2e7cd6", "#8456d1"
  ],
  "backgroundColor": "#f8fcff",
  "panelColor": "#1f2c38",
  "borderColor": "#7ea6c8",
  "textColor": "#18384f",
  "accentColor": "#35aeea",
  "fieldColor": "#f6fbff"
}
```

## 测试覆盖

专属范例 `xui_colorpicker` 与 `xui_colorpicker_xson` 覆盖初始化、Hex 读写、Alpha 开关、弹层打开、常用颜色点击、变更回调和主控件绘制。
