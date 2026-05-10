# XUI Button

`xge_xui_button_t` 是基础交互按钮控件。它建立在 Widget 的背景、边框、圆角、焦点环、禁用遮罩和状态样式基础设施上，自身负责按钮语义：标题、图标混排、点击、选择状态、五状态图片/九宫格按钮、Badge 通知点和五状态渲染缓存。

## 设计思路

- 不保留 loading 机制。等待态可以由业务修改标题并禁用按钮实现，复杂动画交给后续专门控件。
- 普通文字按钮、图标按钮、图标+文字按钮都由 Button 表达。
- 背景、边框、圆角、焦点环和禁用遮罩默认由 Widget 绘制；Button 通过 Widget 状态样式 API 获得 hover、active、disabled、selected 的视觉状态。
- Button 的视觉状态固定为五种：普通、选中、hover、按下、禁用。状态优先级为 `disabled > active > selected > hover > normal`。
- `selected` 是持久状态位，不是 hover、active、disabled 的互斥替代。Widget 样式可以在 selected 基础上继续叠加 hover、active、focus、disabled；图片 patch 无法叠加时按优先级选择单个 patch。
- 图片按钮使用九宫格对象。普通图片按钮可以使用 simple nine patch；圆角、游戏 UI 或复杂素材按钮可以使用 stretch/tile 九宫格，并可为五种状态分别提供素材或 tint。
- Button 内部为五种视觉状态各保留一个渲染缓存槽。缓存只覆盖 Button 自己绘制的图片/九宫格状态层，文字、图标和 Badge 保持实时绘制。
- Badge 默认锚定在内容组右上角并完整放在锚点内侧，这样无论按钮只有文字、只有图标、还是图标文字混排，小红点都跟随“用户真正要看的内容”。
- Badge 默认使用内置渐变红点纹理，不使用纯色矩形；需要强风格时可替换为自定义纹理。

## API

```c
int xgeXuiButtonInit(xge_xui_button button, xge_xui_context ctx, xge_xui_widget widget);
void xgeXuiButtonUnit(xge_xui_button button);

void xgeXuiButtonSetClick(xge_xui_button button, xge_xui_click_proc proc, void* user);
void xgeXuiButtonSetText(xge_xui_button button, xge_font font, const char* text);
void xgeXuiButtonSetTextColor(xge_xui_button button, uint32_t color);

void xgeXuiButtonSetSelectable(xge_xui_button button, int selectable);
void xgeXuiButtonSetSelected(xge_xui_button button, int selected);
int xgeXuiButtonIsSelected(xge_xui_button button);

void xgeXuiButtonSetIcon(xge_xui_button button, xge_texture texture, xge_rect_t src);
void xgeXuiButtonSetIconColor(xge_xui_button button, uint32_t color);
void xgeXuiButtonSetIconLayout(xge_xui_button button, int placement, float iconSize, float gap);

void xgeXuiButtonSetColors(xge_xui_button button, uint32_t normal, uint32_t hover, uint32_t active, uint32_t focus, uint32_t disabled);
void xgeXuiButtonSetPatch(xge_xui_button button, int state, const xge_nine_patch_t* patch);
void xgeXuiButtonClearPatch(xge_xui_button button, int state);
void xgeXuiButtonSetCacheMode(xge_xui_button button, int mode);

void xgeXuiButtonSetBadgeVisible(xge_xui_button button, int visible);
void xgeXuiButtonSetBadgeAnchor(xge_xui_button button, int anchor);
void xgeXuiButtonSetBadgeOffset(xge_xui_button button, float x, float y);
void xgeXuiButtonSetBadgeSize(xge_xui_button button, float size);
void xgeXuiButtonSetBadgeTexture(xge_xui_button button, xge_texture texture, xge_rect_t src);
```

## 状态

五个视觉状态：

- `normal`：默认状态。
- `selected`：可选择按钮的持久选中状态，对应 `XGE_XUI_STATE_CHECKED`。
- `hover`：指针进入按钮。
- `active`：按下状态。
- `disabled`：Widget 被禁用。

图片/九宫格 patch 选择优先级：

```text
disabled > active > selected > hover > normal
```

如果某个高优先级状态没有设置 patch，会回退到较低优先级状态。`active` 没有 patch 但 `hover` 有 patch 时，会使用 hover patch；最终回退到 normal patch。

## 图标与文字

图标位置：

- `XGE_XUI_BUTTON_ICON_LEFT`
- `XGE_XUI_BUTTON_ICON_RIGHT`
- `XGE_XUI_BUTTON_ICON_TOP`
- `XGE_XUI_BUTTON_ICON_BOTTOM`

`xgeXuiButtonSetIconLayout(button, placement, iconSize, gap)` 同时控制图标位置、图标显示尺寸和图标文字间距。按钮会把图标和文字视为一个内容组并整体居中，避免只按文字或只按图标对齐导致视觉偏移。

## 选择逻辑

`xgeXuiButtonSetSelectable(button, 1)` 启用选择逻辑。启用后，鼠标或键盘触发按钮会切换 `selected` 状态；也可以使用 `xgeXuiButtonSetSelected` 主动设置。

选中态既影响 Widget 状态样式，也影响图片/九宫格 patch 选择。纯色按钮可以通过 `selectedColor` 或 `xgeXuiWidgetSetStateBackground(widget, XGE_XUI_STATE_CHECKED, color)` 定义选中背景；图片按钮可以设置 `selectedPatch`。

XSON 还支持 `selectedBorderColor` / `checkedBorderColor` 和 `selectedBorderWidth` / `checkedBorderWidth`，用于让选中态有更明确的边框反馈。

## 图片与九宫格

`xgeXuiButtonSetPatch(button, state, patch)` 用来设置某个状态的图片/九宫格外观。可传入的状态通常为：

- `XGE_XUI_STATE_NORMAL`
- `XGE_XUI_STATE_CHECKED`
- `XGE_XUI_STATE_HOVER`
- `XGE_XUI_STATE_ACTIVE`
- `XGE_XUI_STATE_DISABLED`

普通图片按钮可以使用 `xgeNinePatchInitSimple` 创建 simple patch。复杂按钮建议使用 `xgeNinePatchInit` 设置中心区域，并根据素材选择 `XGE_NINE_PATCH_STRETCH` 或 `XGE_NINE_PATCH_TILE`。

## Badge

Badge 是按钮右上角的通知点能力，适合游戏、工具栏、背包、任务入口等“需要提醒用户点击”的场景。

Badge 锚点：

- `XGE_XUI_BUTTON_BADGE_CONTENT_TOP_RIGHT`
- `XGE_XUI_BUTTON_BADGE_WIDGET_TOP_RIGHT`
- `XGE_XUI_BUTTON_BADGE_ICON_TOP_RIGHT`
- `XGE_XUI_BUTTON_BADGE_TEXT_TOP_RIGHT`

默认锚点是 `CONTENT_TOP_RIGHT`，即跟随图标+文字内容组。这样不同按钮布局下，小红点仍然贴近真正表达功能的区域。需要固定在按钮外框右上角时，使用 `WIDGET_TOP_RIGHT`。

默认 Badge 尺寸是 12px，并且默认完整绘制在锚点内侧，避免 Widget 裁剪后只露出很小的一部分。`offset` 是在这个默认内侧位置基础上的偏移。

## 缓存策略

Button 内部创建 5 个缓存槽，对应：

```text
0 normal
1 selected
2 hover
3 active
4 disabled
```

缓存只覆盖图片/九宫格状态层。文字、图标和 Badge 不进入缓存，原因是这些内容更可能动态变化，并且绘制成本低于九宫格拆分绘制。

缓存模式：

- `XGE_XUI_CACHE_AUTO`：默认模式，有 patch 时尝试缓存，缓存失败时直接绘制 patch。
- `XGE_XUI_CACHE_OFF`：关闭 Button patch 缓存，始终直接绘制。
- `XGE_XUI_CACHE_FORCE`：强制走缓存路径；如果底层渲染目标不可用，仍会回退到直接绘制，保证控件可显示。

缓存失效规则：

- 调用 `xgeXuiButtonSetPatch` 或 `xgeXuiButtonClearPatch` 会失效所有五状态缓存。
- 调用 `xgeXuiButtonSetCacheMode` 会失效所有五状态缓存。
- 控件尺寸变化、DIP scale 变化由通用缓存基础设施自动重建。
- 切换 hover、active、selected、disabled 状态不会重建全部缓存，只会选择对应状态缓存槽。

## XSON

```json
{
  "type": "button",
  "text": "Open",
  "font": "@fonts.body",
  "textColor": "#FFFFFFFF",
  "color": "#367EDAFF",
  "hoverColor": "#5A95E0FF",
  "activeColor": "#2A65AEFF",
  "disabledColor": "#969CA680",
  "selectedColor": "#2F8E5AFF",
  "selectable": true,
  "selected": false,
  "cacheMode": "auto",
  "icon": "@textures.icon",
  "iconSrc": [0, 0, 32, 32],
  "iconPlacement": "left",
  "iconSize": 18,
  "iconGap": 5,
  "badge": {
    "visible": true,
    "anchor": "contentTopRight",
    "size": 12,
    "offset": [0, 0]
  },
  "normalPatch": {
    "texture": "@textures.button",
    "src": [0, 0, 64, 32],
    "center": [0.25, 0.25, 0.75, 0.75],
    "mode": "stretch"
  },
  "hoverPatch": {
    "texture": "@textures.button",
    "src": [0, 32, 64, 32],
    "center": [0.25, 0.25, 0.75, 0.75],
    "mode": "stretch"
  },
  "activePatch": {
    "texture": "@textures.button",
    "src": [0, 64, 64, 32],
    "center": [0.25, 0.25, 0.75, 0.75],
    "mode": "stretch"
  },
  "selectedPatch": {
    "texture": "@textures.button",
    "src": [0, 96, 64, 32],
    "center": [0.25, 0.25, 0.75, 0.75],
    "mode": "stretch"
  },
  "disabledPatch": {
    "texture": "@textures.button",
    "src": [0, 128, 64, 32],
    "center": [0.25, 0.25, 0.75, 0.75],
    "mode": "stretch"
  }
}
```

支持字段：

- `text`、`font`、`textColor`：标题、字体和文字颜色。
- `color` / `hoverColor` / `activeColor` / `disabledColor` / `selectedColor`：状态背景色。
- `selectedBorderColor` / `checkedBorderColor`、`selectedBorderWidth` / `checkedBorderWidth`：选中态边框。
- `selectable`、`selected`：启用选择逻辑及初始选中状态。
- `icon` / `iconTexture`、`iconSrc`、`iconPlacement`、`iconSize`、`iconGap`：图标纹理、源区域和混排布局。
- `patch` / `normalPatch` / `hoverPatch` / `activePatch` / `disabledPatch` / `selectedPatch`：五状态图片/九宫格外观。
- `cacheMode`：`"auto"`、`"off"`、`"force"` 或对应整数。
- `badge`：布尔值表示是否显示默认 Badge；对象形式可设置 `visible`、`anchor`、`size`、`offset`、`texture`、`src`。
- Widget 通用字段同样适用，例如 `width`、`height`、`padding`、`borderColor`、`borderWidth`、`radius`、`enabled`、`tabIndex`。

## 用户流程建议

纯色按钮优先使用 Widget 状态样式：`color`、`hoverColor`、`activeColor`、`disabledColor`、`selectedColor`、`borderColor`、`borderWidth`、`radius`。

图片按钮优先使用五状态 patch：至少提供 `normalPatch`，再按需要补充 `hoverPatch`、`activePatch`、`selectedPatch`、`disabledPatch`。如果希望按钮看起来像完整游戏 UI 素材，推荐五个状态都提供。

只需要文字按钮时，不要设置 icon 和 patch。只需要图标按钮时，设置 icon 并把 text 设为空字符串。需要通知点时，优先使用默认 Badge，再根据项目美术风格替换纹理。

## Examples

- `examples/xui_button`：C API 版本。
- `examples/xui_button_xson`：XSON 描述版本。

两个范例都展示 7 种配色方案、图标与文字四方位混排、选中/未选中切换、禁用按钮、普通图片按钮、九宫格按钮和 Badge 按钮。
