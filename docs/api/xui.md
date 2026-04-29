# XUI API

> XUI API 是 XGE 内部孵化期 GUI 接口，负责布局、控件、事件响应和通过 XGE host 渲染。

[返回 API 索引](README.md) | [XUI 教程](../guide/xui-bridge-intro.md) | [XUI 范例](../case/xui-bridge.md)

---

## 模块定位

XUI 的长期目标是成为可独立工作的 DUI 系统。当前阶段先在 XGE 内孵化，方便调试和回归测试；成熟后再剥离到 XUI 仓库，并通过高性能 bridge 接入 XGE。

边界：

- XGE 提供时间、输入、渲染能力和资源对象。
- XUI 负责布局、控件树、事件响应、焦点/捕获、dirty 标记和 paint。
- XUI 不内置完整 IME，第一版依赖系统 IME，XUI 处理提交文本和候选区域。

## 标准调用顺序

```text
xgeXuiInit
  -> xgeXuiSetHost 可选
  -> 创建 widget 树和控件
  -> xgeXuiDispatchEvent / xgeXuiEventPush
  -> xgeXuiUpdate
  -> xgeXuiPaint
  -> xgeXuiUnit
```

## 公共类型

### `xge_xui_context_t`

XUI 上下文，保存 root、focus、capture、事件队列、主题、dirty 状态和 host。调用者可以透明读取状态，但不应直接破坏链表或队列字段。

### `xge_xui_widget_t`

基础 widget 节点，包含父子兄弟链表、ID、Name、style、rect、content rect、flags、事件回调、measure 回调和 paint 回调。

### `xge_xui_host_t`

XUI 到宿主的绘制桥。

| 回调 | 说明 |
| --- | --- |
| `draw_rect` | 绘制矩形。 |
| `draw_image` | 绘制图片。 |
| `draw_text_rect` | 绘制文本。 |
| `measure_text` | 测量文本。 |
| `clip_set` / `clip_clear` | 裁剪。 |
| `request_refresh` | 请求宿主刷新。 |

### `xge_xui_style_t` / `xge_xui_theme_t`

Style 保存布局、dock、尺寸、margin、padding、anchor、grid、align、z、clip、background 和 radius。Theme 保存默认字体、颜色、状态色、圆角、padding、spacing 和 border width。

## 函数分组

| 分组 | 函数 |
| --- | --- |
| Size | `xgeXuiSizePx`、`xgeXuiSizeDip`、`xgeXuiSizePercent`、`xgeXuiSizeGrow`、`xgeXuiSizeContent` |
| Context | `xgeXuiInit`、`xgeXuiUnit`、`xgeXuiRoot` |
| DIP / Safe Area | `xgeXuiSetDipScale`、`xgeXuiGetDipScale`、`xgeXuiSetSafeAreaPx` |
| Theme/Style | `xgeXuiThemeDefault`、`xgeXuiSetTheme`、`xgeXuiGetTheme`、`xgeXuiGetThemeVersion`、`xgeXuiTokenSetColor`、`xgeXuiTokenSetSpacing`、`xgeXuiTokenSetFont`、`xgeXuiStyleDefault`、`xgeXuiStyleFromTheme` |
| Refresh/Layout Batch | `xgeXuiRefreshRequest`、`xgeXuiRefreshNeeded`、`xgeXuiRefreshClear`、`xgeXuiLayoutBatchBegin`、`xgeXuiLayoutBatchEnd` |
| Host | `xgeXuiSetHost`、`xgeXuiGetHost` |
| Page / Loader | `xgeXuiPageLoad`、`xgeXuiPageLoadMemory`、`xgeXuiPageUnload`、`xgeXuiPageRoot`、`xgeXuiPageFind`、`xgeXuiPageStyleVersion`、`xgeXuiPageGetError` |
| Binder | `xgeXuiBinderInit`、`xgeXuiBinderSetClick` |
| Widget | `xgeXuiWidget*`、`xgeXuiHitTest`、`xgeXuiSetFocus`、`xgeXuiSetCapture` |
| Event | `xgeXuiDispatchEvent`、`xgeXuiEventPush`、`xgeXuiEventPop`、`xgeXuiEventCount`、`xgeXuiDispatchQueuedEvents` |
| Frame | `xgeXuiUpdate`、`xgeXuiPaint` |
| Text | `xgeXuiText*` |
| Controls | `xgeXuiButton*`、`xgeXuiLabel*`、`xgeXuiImage*`、`xgeXuiInput*`、`xgeXuiToggle*`、`xgeXuiSlider*`、`xgeXuiProgress*`、`xgeXuiPanel*`、`xgeXuiScrollView*`、`xgeXuiListView*`、`xgeXuiVirtualList*`、`xgeXuiDialog*` |

> 本页 API 数量较多，采用分批展开。当前已展开 Size / Context / Theme / Host 核心函数；Widget、Event、Text 和 Controls 会在后续批次继续补齐。

## API 参考：Size / Context / Host

### xgeXuiSizePx

创建像素尺寸描述。

**功能：**

你可以用它声明一个不受 DIP 缩放影响的固定像素尺寸。

**函数原型：**

```c
XGE_API xge_xui_size_t xgeXuiSizePx(float fValue);
```

**参数：**

- `fValue`：像素值。

**返回值：**

返回 `iUnit = XGE_XUI_SIZE_PX`、`fValue = fValue` 的尺寸描述。

**资源归属：**

返回值是结构体值，不需要释放。

**补充说明：**

布局解析时该值直接作为像素使用。

**范例代码：**

```c
style.tWidth = xgeXuiSizePx(240.0f);
```

**相关 API：**

- `xgeXuiSizeDip`
- `xgeXuiWidgetSetSize`

---

### xgeXuiSizeDip

创建 DIP 尺寸描述。

**功能：**

你可以用它声明随 `xgeXuiSetDipScale` 缩放的界面尺寸。

**函数原型：**

```c
XGE_API xge_xui_size_t xgeXuiSizeDip(float fValue);
```

**参数：**

- `fValue`：DIP 值。

**返回值：**

返回 `iUnit = XGE_XUI_SIZE_DIP` 的尺寸描述。

**资源归属：**

返回值是结构体值，不需要释放。

**补充说明：**

布局解析时会乘以当前 context 的 DIP scale。

**范例代码：**

```c
style.tHeight = xgeXuiSizeDip(44.0f);
```

**相关 API：**

- `xgeXuiSetDipScale`
- `xgeXuiGetDipScale`

---

### xgeXuiSizePercent

创建百分比尺寸描述。

**功能：**

你可以用它按父容器尺寸计算控件宽高。

**函数原型：**

```c
XGE_API xge_xui_size_t xgeXuiSizePercent(float fValue);
```

**参数：**

- `fValue`：百分比数值，例如 `50.0f` 表示 50%。

**返回值：**

返回 `iUnit = XGE_XUI_SIZE_PERCENT` 的尺寸描述。

**资源归属：**

返回值是结构体值，不需要释放。

**补充说明：**

布局解析公式为 `parent * fValue * 0.01f`。

**范例代码：**

```c
style.tWidth = xgeXuiSizePercent(100.0f);
```

**相关 API：**

- `xgeXuiWidgetSetSize`

---

### xgeXuiSizeGrow

创建 grow 尺寸描述。

**功能：**

你可以用它让控件在 row/column/grid 等布局中使用剩余空间。

**函数原型：**

```c
XGE_API xge_xui_size_t xgeXuiSizeGrow(float fValue);
```

**参数：**

- `fValue`：grow 权重。

**返回值：**

返回 `iUnit = XGE_XUI_SIZE_GROW` 的尺寸描述。

**资源归属：**

返回值是结构体值，不需要释放。

**补充说明：**

当前底层 size resolve 会把 grow 交给布局提供的 fallback 值。

**范例代码：**

```c
style.tWidth = xgeXuiSizeGrow(1.0f);
```

**相关 API：**

- `xgeXuiWidgetSetLayout`
- `xgeXuiWidgetSetSize`

---

### xgeXuiSizeContent

创建内容自适应尺寸描述。

**功能：**

你可以用它让控件宽高优先来自 measure 回调或控件自身内容。

**函数原型：**

```c
XGE_API xge_xui_size_t xgeXuiSizeContent(void);
```

**参数：**

无。

**返回值：**

返回 `iUnit = XGE_XUI_SIZE_CONTENT`、`fValue = 0` 的尺寸描述。

**资源归属：**

返回值是结构体值，不需要释放。

**补充说明：**

如果内容测量值不可用，布局会使用 fallback 尺寸。

**范例代码：**

```c
style.tWidth = xgeXuiSizeContent();
```

**相关 API：**

- `xgeXuiWidgetSetMeasure`
- `xgeXuiStyleDefault`

---

### xgeXuiInit

初始化 XUI context。

**功能：**

你可以用它创建 XUI root widget、默认主题、默认 host 和初始 dirty 状态。

**函数原型：**

```c
XGE_API int xgeXuiInit(xge_xui_context pContext);
```

**参数：**

- `pContext`：输出 XUI context，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- context 已初始化返回 `XGE_ERROR_ALREADY_INITIALIZED`。
- root widget 分配失败返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

成功后 `pContext` 持有 root widget tree，调用者必须使用 `xgeXuiUnit` 释放。

**补充说明：**

- root 默认尺寸来自 `xgeGetWidth()` / `xgeGetHeight()`。
- 默认 DIP scale 为 `1.0f`。
- 默认 host 为 XGE 内置 host。

**范例代码：**

```c
xge_xui_context_t ui;
if (xgeXuiInit(&ui) == XGE_OK) {
    /* build UI */
}
```

**相关 API：**

- `xgeXuiUnit`
- `xgeXuiRoot`
- `xgeXuiSetHost`

---

### xgeXuiUnit

释放 XUI context。

**功能：**

你可以用它释放 root widget tree，并清零 context。

**函数原型：**

```c
XGE_API void xgeXuiUnit(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `xgeXuiInit` 创建的 root widget tree，不释放 context 结构体本身。

**补充说明：**

对 `NULL` 或未初始化 context 调用是安全的。

**范例代码：**

```c
xgeXuiUnit(&ui);
```

**相关 API：**

- `xgeXuiInit`

---

### xgeXuiRoot

获取 root widget。

**功能：**

你可以用它取得 context 的根节点，并把业务 widget 添加到 root 下。

**函数原型：**

```c
XGE_API xge_xui_widget xgeXuiRoot(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

- context 有效时返回 root widget。
- `pContext` 为 `NULL` 时返回 `NULL`。

**资源归属：**

返回值是借用指针，所有权属于 context，不能单独释放。

**补充说明：**

root 会随 `xgeXuiUnit` 释放。

**范例代码：**

```c
xge_xui_widget root = xgeXuiRoot(&ui);
```

**相关 API：**

- `xgeXuiWidgetAdd`
- `xgeXuiUnit`

---

### xgeXuiSetDipScale

设置 DIP 缩放。

**功能：**

你可以用它根据窗口缩放、DPR 或 UI 缩放设置 DIP 到像素的转换比例。

**函数原型：**

```c
XGE_API void xgeXuiSetDipScale(xge_xui_context pContext, float fScale);
```

**参数：**

- `pContext`：XUI context。
- `fScale`：缩放值，小于等于 0 时按 `1.0f` 处理。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

scale 变化时会标记 root layout 和 paint dirty。

**范例代码：**

```c
xgeXuiSetDipScale(&ui, dpr);
```

**相关 API：**

- `xgeXuiGetDipScale`
- `xgeXuiSizeDip`

---

### xgeXuiGetDipScale

获取 DIP 缩放。

**功能：**

你可以用它读取当前 context 的 DIP scale。

**函数原型：**

```c
XGE_API float xgeXuiGetDipScale(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

- context 有效且 scale 大于 0 时返回当前 scale。
- context 无效时返回 `1.0f`。

**资源归属：**

函数不分配资源。

**补充说明：**

布局和 paint 期间内部也会使用该值解析 DIP 尺寸。

**范例代码：**

```c
float scale = xgeXuiGetDipScale(&ui);
```

**相关 API：**

- `xgeXuiSetDipScale`

---

### xgeXuiSetSafeAreaPx

设置 XUI root 的安全区内边距。

**功能：**

root 和 overlay root 会自动跟随窗口尺寸。你可以用本函数让 root 的 content rect 避开刘海、系统栏或业务边距；overlay root 仍保持全窗口尺寸。

**函数原型：**

```c
XGE_API void xgeXuiSetSafeAreaPx(xge_xui_context pContext, float fLeft, float fTop, float fRight, float fBottom);
```

**参数：**

- `pContext`：XUI context。
- `fLeft` / `fTop` / `fRight` / `fBottom`：安全区内边距，单位为像素。

**返回值：**

无。

**相关 API：**

- `xgeXuiRoot`
- `xgeXuiWidgetSetPaddingPx`

**XSON：**

Page 顶层可声明 `safeArea`，加载时应用到当前 context root，卸载该 page 时恢复加载前的 root padding。`safeArea` 支持数字、二元数组 `[x, y]`、四元数组 `[left, top, right, bottom]`，并支持 spacing token：

```json
{
  "xui": 1,
  "tokens": { "spacing": { "safeX": 12, "safeY": 24 } },
  "safeArea": [ "@spacing.safeX", "@spacing.safeY" ],
  "tree": { "id": "root" }
}
```

---

### xgeXuiThemeDefault

生成默认主题。

**功能：**

你可以用它初始化一个主题结构，再按项目需要修改颜色、字体、圆角和间距。

**函数原型：**

```c
XGE_API void xgeXuiThemeDefault(xge_xui_theme pTheme);
```

**参数：**

- `pTheme`：输出主题，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数只写入结构体字段，不接管字体资源。

**补充说明：**

默认主题使用白色文本、深色 panel、绿色 accent、4px 圆角、4px padding/spacing。

**范例代码：**

```c
xge_xui_theme_t theme;
xgeXuiThemeDefault(&theme);
```

**相关 API：**

- `xgeXuiSetTheme`
- `xgeXuiStyleFromTheme`

---

### xgeXuiSetTheme

设置 context 主题。

**功能：**

你可以用它替换 XUI context 当前主题，并触发重新布局和重绘。

**函数原型：**

```c
XGE_API void xgeXuiSetTheme(xge_xui_context pContext, const xge_xui_theme_t* pTheme);
XGE_API uint32_t xgeXuiGetThemeVersion(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。
- `pTheme`：主题输入；传 `NULL` 时恢复默认主题。

**返回值：**

无。

**资源归属：**

函数复制主题结构体，不接管 `pTheme` 或主题里的字体对象。

**补充说明：**

context 未初始化时调用无效果。`xgeXuiSetTheme` 每次成功设置都会递增 theme version，并标记 root layout/paint dirty；`xgeXuiGetThemeVersion` 返回当前版本。theme version 第一版用于后续 style/token cache 失效判断。

`xgeXuiTokenSetColor`、`xgeXuiTokenSetSpacing`、`xgeXuiTokenSetFont` 分别注册 context 级颜色、间距和字体 token。注册成功后会递增 theme version，并标记 root style/layout/paint dirty。字体 token 只保存外部字体指针，不接管字体生命周期。

**范例代码：**

```c
xgeXuiSetTheme(&ui, &theme);
```

**相关 API：**

- `xgeXuiGetTheme`
- `xgeXuiThemeDefault`

---

### xgeXuiGetTheme

获取当前主题。

**功能：**

你可以用它读取 context 当前主题；context 无效时仍能取得一个静态默认主题。

**函数原型：**

```c
XGE_API const xge_xui_theme_t* xgeXuiGetTheme(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context，可以为 `NULL`。

**返回值：**

- context 有效时返回 `&pContext->tTheme`。
- context 无效时返回静态默认主题指针。

**资源归属：**

返回值是借用指针，调用者不能释放。

**补充说明：**

静态默认主题会在首次需要时初始化。

**范例代码：**

```c
const xge_xui_theme_t* theme = xgeXuiGetTheme(&ui);
```

**相关 API：**

- `xgeXuiSetTheme`
- `xgeXuiThemeDefault`

---

### xgeXuiStyleDefault

生成默认 style。

**功能：**

你可以用它初始化一个 widget style，再设置布局、尺寸、padding、颜色等字段。

**函数原型：**

```c
XGE_API void xgeXuiStyleDefault(xge_xui_style_t* pStyle);
```

**参数：**

- `pStyle`：输出 style，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数只写入结构体字段，不分配资源。

**补充说明：**

默认 layout 为 absolute，dock 为 fill，width/height 为 content，align 为 stretch，背景透明，grid columns 为 1。

**范例代码：**

```c
xge_xui_style_t style;
xgeXuiStyleDefault(&style);
```

**相关 API：**

- `xgeXuiStyleFromTheme`
- `xgeXuiWidgetSetStyle`

---

### xgeXuiStyleFromTheme

从主题生成 style。

**功能：**

你可以用它基于主题快速生成 panel 风格的默认 style。

**函数原型：**

```c
XGE_API void xgeXuiStyleFromTheme(xge_xui_style_t* pStyle, const xge_xui_theme_t* pTheme);
```

**参数：**

- `pStyle`：输出 style，可以为 `NULL`。
- `pTheme`：输入主题；传 `NULL` 时使用默认主题。

**返回值：**

无。

**资源归属：**

函数只复制数值字段，不接管主题资源。

**补充说明：**

生成的 style 会使用主题 panel 色、圆角、padding 和 spacing。

**范例代码：**

```c
xge_xui_style_t style;
xgeXuiStyleFromTheme(&style, xgeXuiGetTheme(&ui));
```

**相关 API：**

- `xgeXuiThemeDefault`
- `xgeXuiWidgetSetStyle`

---

### xgeXuiRefreshRequest

请求刷新。

**功能：**

你可以用它标记 context 需要刷新，并通知 host 调度重绘。

**函数原型：**

```c
XGE_API void xgeXuiRefreshRequest(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

如果已经请求过刷新，函数会直接返回。host 提供 `request_refresh` 时会被调用一次。

**范例代码：**

```c
xgeXuiRefreshRequest(&ui);
```

**相关 API：**

- `xgeXuiRefreshNeeded`
- `xgeXuiRefreshClear`

---

### xgeXuiRefreshNeeded

查询是否需要刷新。

**功能：**

你可以在 APP 模式或手动刷新模式下判断是否需要调用 update/paint。

**函数原型：**

```c
XGE_API int xgeXuiRefreshNeeded(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

- 需要刷新返回 1。
- context 无效或无需刷新返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

`xgeXuiPaint` 完成后会清除 refresh 请求。

**范例代码：**

```c
if (xgeXuiRefreshNeeded(&ui)) {
    xgeXuiUpdate(&ui, 0.0f);
    xgeXuiPaint(&ui);
}
```

**相关 API：**

- `xgeXuiRefreshRequest`
- `xgeXuiPaint`

---

### xgeXuiRefreshClear

清除刷新请求。

**功能：**

你可以手动清除 context 的 refresh 标记。

**函数原型：**

```c
XGE_API void xgeXuiRefreshClear(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

context 无效时调用无效果。

**范例代码：**

```c
xgeXuiRefreshClear(&ui);
```

**相关 API：**

- `xgeXuiRefreshNeeded`

---

### xgeXuiLayoutBatchBegin

开始布局批处理。

**功能：**

你可以在批量修改 widget 时调用它，延迟 dirty 传播和 refresh 请求，减少重复刷新。

**函数原型：**

```c
XGE_API void xgeXuiLayoutBatchBegin(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

无。

**资源归属：**

函数不分配资源。

**补充说明：**

批处理支持嵌套，内部通过 `iLayoutBatchDepth` 计数。

**范例代码：**

```c
xgeXuiLayoutBatchBegin(&ui);
```

**相关 API：**

- `xgeXuiLayoutBatchEnd`

---

### xgeXuiLayoutBatchEnd

结束布局批处理。

**功能：**

你可以用它结束一段批量修改；当批处理深度回到 0 时，XUI 会把累积 dirty 应用到 root 并请求刷新。

**函数原型：**

```c
XGE_API void xgeXuiLayoutBatchEnd(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

如果没有对应 begin 或 context 无效，函数直接返回。

**范例代码：**

```c
xgeXuiLayoutBatchEnd(&ui);
```

**相关 API：**

- `xgeXuiLayoutBatchBegin`
- `xgeXuiRefreshRequest`

---

### xgeXuiSetHost

设置 XUI host。

**功能：**

你可以用它替换 XUI 的绘制桥，接入 XGE 默认 host 或其他宿主框架。

**函数原型：**

```c
XGE_API void xgeXuiSetHost(xge_xui_context pContext, const xge_xui_host_t* pHost);
```

**参数：**

- `pContext`：XUI context。
- `pHost`：host 描述；传 `NULL` 时恢复默认 host。

**返回值：**

无。

**资源归属：**

函数只保存 host 借用指针，不复制 host，也不接管 `pUser`。

**补充说明：**

设置 host 后会标记 root paint dirty。

**范例代码：**

```c
xgeXuiSetHost(&ui, &host);
```

**相关 API：**

- `xgeXuiGetHost`
- `xgeXuiPaint`

---

### xgeXuiGetHost

获取当前 XUI host。

**功能：**

你可以用它取得当前绘制桥；context 无效或未设置 host 时返回默认 host。

**函数原型：**

```c
XGE_API const xge_xui_host_t* xgeXuiGetHost(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context，可以为 `NULL`。

**返回值：**

返回当前 host 或默认 host 的借用指针。

**资源归属：**

返回值不转移所有权，调用者不能释放。

**补充说明：**

XUI update/paint 期间会把该 host 设置为当前 active host。

**范例代码：**

```c
const xge_xui_host_t* host = xgeXuiGetHost(&ui);
```

**相关 API：**

- `xgeXuiSetHost`

---

## API 参考：Page / Loader / Binder

### xgeXuiPageLoad

从资源 URI 加载 XUI 页面。

**函数原型：**

```c
XGE_API int xgeXuiPageLoad(xge_xui_context pContext, const char* sURI, const xge_xui_binder_t* pBinder, xge_xui_page_t* pPage);
```

**说明：**

- 资源读取走 `xgeResourceLoad`。
- 当前阶段支持从 XSON 创建 retained widget tree，并挂到当前 XUI root。
- 已支持 `tree.type/id/name/children`、顶层 `styles`、widget `style` 引用、style `@parent` 继承，以及基础 layout/size/spacing/alignment/visual inline 覆盖字段。
- `tree.type` 第一版支持结构型控件：`panel`、`absolute`、`row`、`column`、`stack`、`grid`、`dock`、`scrollView`/`scroll`，并支持轻量状态控件：`button`、`image`、`input`、`label`、`separator`。未知类型或非字符串类型会使 page load 失败。
- `scrollView`/`scroll` 会绑定 `xge_xui_scroll_view_t`，默认开启 `clip` 和 focusable；支持 `contentSize`、`contentWidth`、`contentHeight`、`offset`/`scrollOffset`/`contentOffset`、`scrollX`、`scrollY`、`backgroundColor`/`background`、`barColor`、`thumbColor`。滚动偏移会作用到子 widget 布局结果；命中和滚轮只在 content rect 内生效，padding 区不会启动滚动。`button` 支持 `text`、`font`、`textColor`、`textAlign`、`textVAlign`、`color`/`background`、`hoverColor`、`activeColor`、`focusColor`、`disabledColor` 和 `onClick`。`button.onClick` 使用控件自身 `xgeXuiButtonSetClick`，不会覆盖控件 `pUser`。`image` 支持 `texture`、`src`、`source`/`srcRect`、`color`/`tint`、`mode`；`texture` 引用 C 侧注册的 texture token，`src` 由 page 同步加载并在 unload 时释放。`input` 支持 `text`/`value`、`placeholder`、`font`、`textColor`、`background`/`backgroundColor`、`focusColor`、`cursorColor`、`placeholderColor`、`selectionColor`、`disabledTextColor`、`disabledBackgroundColor`、`password`、`readonly`、`disabled`、`selection`；文本缓冲、默认菜单和 IME 状态由 `xge_xui_input_t` 持有，随 page unload 调用 `xgeXuiInputUnit`。`label` 支持 `text`、`font`、`textColor`/`color`、`textAlign`、`textVAlign`，其中 `font` 引用 C 侧注册的 font token，例如 `"@fonts.body"`。`separator` 支持 `orientation`、`thickness`、`color`/`background`。这些状态控件由 page 内固定容量 control arena 持有，并在 `xgeXuiPageUnload` 中调用对应 `Unit`。
- style 表直接使用 XValue table 父表链共享字段；父表不被单独持有，生命周期随 page document 统一释放。`@parent` 循环会导致 page load 失败，并通过 `xgeXuiPageGetError` 返回 `style parent cycle`。
- `imports` 第一版支持导入其它 XSON 资源中的 `styles`、`tokens`、`templates`，不会导入 `tree`。带 scheme 的 URI 原样走 `xgeResourceLoad`；相对路径按当前 XSON URI 所在目录解析。imports 按数组顺序合并，后导入覆盖先导入，当前 XSON 本地声明最终覆盖 imports。
- `tokens.colors` 可被颜色字段引用，例如 `"background": "@colors.panel"`；`tokens.spacing` 可被尺寸、间距和半径字段引用，例如 `"gap": "@spacing.md"`。未带 section 的 `@name` 会依次查找顶层 token、`colors`、`spacing`、`fonts`、`textures`。缺失 token 会使 page load 失败，并在错误中包含字段路径。C 侧可通过 `xgeXuiTokenSetColor`、`xgeXuiTokenSetSpacing`、`xgeXuiTokenSetFont`、`xgeXuiTokenSetTexture` 注册 context 级 token；XSON/import token 优先，context token 作为 fallback。font/texture token 都只引用外部对象，不由 XSON 或 token 表释放。
- 尺寸字段支持数字 px、`"content"`、`"grow"`、`"grow:N"`、`"N%"`、`"Ndip"` 和 `{ "unit": "...", "value": N }`。`padding`/`margin` 支持数字、二元数组 `[x, y]`、四元数组 `[left, top, right, bottom]`。顶层 `safeArea` 使用同样的数字/数组格式，并在 page unload 时恢复加载前的 root padding。`anchor` 支持 `{ "left": N, "right": "5%" }` 这类对象，出现哪个边就启用哪个 anchor flag。Grid 支持 `grid.columns/rowHeight/columnGap/rowGap/columnSpan`，也支持顶层别名 `columns/rowHeight/columnGap/rowGap/columnSpan`。DockLayout 支持容器 `type:"dock"` 或 `layout:"dock"`，子元素 `dock` 可为 `top`、`bottom`、`left`、`right`、`fill`、`center`；布局按声明顺序扣减剩余区域，`fill/center` 使用当前剩余区域。ScrollView 示例：

```json
{
  "type": "scrollView",
  "width": 320,
  "height": 240,
  "padding": 8,
  "contentSize": [ 320, 800 ],
  "offset": { "x": 0, "y": 120 },
  "children": [
    { "type": "column", "children": [] }
  ]
}
```
- `onClick` 支持绑定 C 侧 `xgeXuiBinderSetClick` 注册的事件名，例如 `"onClick": "ok"`。未注册事件名会使 page load 失败并报告字段路径。XSON 不支持脚本字段；`script`、`onClickScript` 会作为加载错误处理。结构型 widget 的 `onClick` 绑定到 page 创建的通用事件过程；`button` 的 `onClick` 绑定到按钮控件自身点击回调。`input.onChange/onSubmit` 当前严格拒绝，不提供空占位，后续随 model/value 绑定统一实现。

---

### xgeXuiPageLoadMemory

从内存加载 XUI 页面。

```c
XGE_API int xgeXuiPageLoadMemory(xge_xui_context pContext, const void* pData, int iSize, const xge_xui_binder_t* pBinder, xge_xui_page_t* pPage);
```

内存内容会复制进 page loader 持有的 resource，卸载时统一释放。

最小 XSON 示例：

```json
{
  "xui": 1,
  "imports": [ "shared/styles.xson" ],
  "styles": {
    "base": { "width": 120, "height": "grow:2", "padding": [8, 4] },
    "panel": { "@parent": "base", "layout": "row", "gap": 8 }
  },
  "tree": {
    "type": "column",
    "id": "root",
    "style": "panel",
    "children": [
      { "type": "row", "name": "child", "width": "50%" }
    ]
  }
}
```

---

### xgeXuiPageUnload / Root / Find / GetError

```c
XGE_API void xgeXuiPageUnload(xge_xui_page_t* pPage);
XGE_API xge_xui_widget xgeXuiPageRoot(xge_xui_page_t* pPage);
XGE_API xge_xui_widget xgeXuiPageFind(xge_xui_page_t* pPage, const char* sId);
XGE_API uint32_t xgeXuiPageStyleVersion(xge_xui_page_t* pPage);
XGE_API const char* xgeXuiPageGetError(xge_xui_page_t* pPage);
```

`xgeXuiPageUnload` 释放 loader 创建的 root、XSON document、imports、合并后的 style/token/template 表、page control arena 中的状态控件和 resource。`xgeXuiPageFind` 优先使用 page load 时建立的固定容量 id/name 索引；索引溢出或手工填充 `pRoot` 且未建立索引时，会回退到 page root 下递归查找。page load 失败会回滚 loader 已创建资源、状态控件和 widget tree，并保留 `xgeXuiPageGetError` 可读取的错误字符串。`xgeXuiPageStyleVersion` 返回 page 当前 style version；第一版 page load 成功后为 `1`，用于后续 style/token cache 失效扩展。

context 级 token 变更会递增 theme version，但不会在布局或绘制热路径重新解析 XSON。需要让已加载 page 使用新的 token 值时，调用 `xgeXuiPageSyncStyle`；它只在 page 记录的 theme version 落后于 context 时调用 `xgeXuiPageRefreshStyle`。`xgeXuiPageRefreshStyle` 会重新遍历 page 的 XSON tree，把 layout/visual 字段解析回 widget 的 `xge_xui_style_t` cache，递增 page style version，并标记受影响 widget 的 layout/paint dirty。

```c
XGE_API int xgeXuiPageRefreshStyle(xge_xui_page_t* pPage);
XGE_API int xgeXuiPageSyncStyle(xge_xui_page_t* pPage);
```

---

### xgeXuiBinderInit / xgeXuiBinderSetClick

```c
XGE_API void xgeXuiBinderInit(xge_xui_binder_t* pBinder);
XGE_API int xgeXuiBinderSetClick(xge_xui_binder_t* pBinder, const char* sName, xge_xui_click_proc procClick, void* pUser);
```

Binder 第一版只提供事件名到 C 回调的注册容器。XSON 解析阶段会使用这些注册项绑定 `onClick`。

---

### XSON style 生命周期约束

`styles`、imports 中导入的 `styles`、以及 loader 内部合并后的 style table 都由 `xge_xui_page_t` 持有。`xvoTableSetParent` 只设置底层父表指针，不增加父表引用计数，也不接管父表所有权。因此第一版规则是：所有参与 `@parent` 查询的 XValue table 必须和 page 保持相同生命周期，并只能由 `xgeXuiPageUnload` 统一释放。

---

## API 参考：Widget / Event / Frame

### xgeXuiWidgetCreate

创建一个 widget。

**功能：**

你可以用它分配独立 widget 节点，之后再添加到 context root 或其他父节点下。

**函数原型：**

```c
XGE_API xge_xui_widget xgeXuiWidgetCreate(void);
```

**参数：**

无。

**返回值：**

- 成功返回新 widget。
- 内存不足返回 `NULL`。

**资源归属：**

返回 widget 由调用者持有，使用 `xgeXuiWidgetFree` 释放。

**补充说明：**

新 widget 默认 visible、enabled，并带 layout/paint dirty 标志。

**范例代码：**

```c
xge_xui_widget w = xgeXuiWidgetCreate();
```

**相关 API：**

- `xgeXuiWidgetAdd`
- `xgeXuiWidgetFree`

---

### xgeXuiWidgetFree

释放 widget 子树。

**功能：**

你可以用它把 widget 从父节点移除，并递归释放其所有子节点。

**函数原型：**

```c
XGE_API void xgeXuiWidgetFree(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：要释放的 widget，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 widget 树内由 XUI 分配的节点，不释放控件对象、font、texture 或用户数据。

**补充说明：**

对 `NULL` 调用安全。释放前会调用 `xgeXuiWidgetRemove`。

**范例代码：**

```c
xgeXuiWidgetFree(w);
```

**相关 API：**

- `xgeXuiWidgetCreate`
- `xgeXuiWidgetRemove`

---

### xgeXuiWidgetAdd

添加子 widget。

**功能：**

你可以用它把 child 挂到 parent 的子链表末尾。

**函数原型：**

```c
XGE_API int xgeXuiWidgetAdd(xge_xui_widget pParent, xge_xui_widget pChild);
```

**参数：**

- `pParent`：父 widget，不能为 `NULL`。
- `pChild`：子 widget，不能为 `NULL`，且不能等于 parent。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

父节点只维护树关系，不接管业务控件对象。

**补充说明：**

如果 child 已有父节点，会先从旧父节点移除。添加后 parent 会标记 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetAdd(xgeXuiRoot(&ui), w);
```

**相关 API：**

- `xgeXuiWidgetRemove`
- `xgeXuiWidgetMarkLayout`

---

### xgeXuiWidgetRemove

从父节点移除 widget。

**功能：**

你可以用它断开 widget 与父节点的关系，但不释放 widget 本身。

**函数原型：**

```c
XGE_API void xgeXuiWidgetRemove(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：要移除的 widget。

**返回值：**

无。

**资源归属：**

函数不释放 widget，调用者仍持有该节点。

**补充说明：**

移除后会清空 parent/prev/next sibling，并标记原父节点 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetRemove(w);
```

**相关 API：**

- `xgeXuiWidgetAdd`
- `xgeXuiWidgetFree`

---

### xgeXuiWidgetSetId

设置 widget ID。

**功能：**

你可以用它给 widget 设置整数 ID，便于后续查找。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetId(xge_xui_widget pWidget, int iId);
```

**参数：**

- `pWidget`：widget 对象。
- `iId`：整数 ID。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

`pWidget` 为 `NULL` 时无效果。

**范例代码：**

```c
xgeXuiWidgetSetId(w, 1001);
```

**相关 API：**

- `xgeXuiWidgetGetId`
- `xgeXuiWidgetFindById`

---

### xgeXuiWidgetGetId

获取 widget ID。

**功能：**

你可以用它读取 widget 当前 ID。

**函数原型：**

```c
XGE_API int xgeXuiWidgetGetId(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

- widget 有效时返回 ID。
- `pWidget` 为 `NULL` 时返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

ID 是否唯一由调用者维护。

**范例代码：**

```c
int id = xgeXuiWidgetGetId(w);
```

**相关 API：**

- `xgeXuiWidgetSetId`

---

### xgeXuiWidgetSetName

设置 widget 名称。

**功能：**

你可以用它给 widget 设置名称，便于调试或按名称查找。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetName(xge_xui_widget pWidget, const char* sName);
```

**参数：**

- `pWidget`：widget 对象。
- `sName`：名称字符串，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数只保存借用指针，不复制字符串。

**补充说明：**

`sName == NULL` 时名称被设置为空字符串。调用者必须保证非空名称指针生命周期覆盖使用期。

**范例代码：**

```c
xgeXuiWidgetSetName(w, "main_panel");
```

**相关 API：**

- `xgeXuiWidgetGetName`
- `xgeXuiWidgetFindByName`

---

### xgeXuiWidgetGetName

获取 widget 名称。

**功能：**

你可以用它读取 widget 当前名称。

**函数原型：**

```c
XGE_API const char* xgeXuiWidgetGetName(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

- 返回名称字符串。
- widget 无效或名称为空时返回空字符串。

**资源归属：**

返回值是借用指针，调用者不能释放。

**补充说明：**

名称指针由 `xgeXuiWidgetSetName` 的调用者负责保持有效。

**范例代码：**

```c
const char* name = xgeXuiWidgetGetName(w);
```

**相关 API：**

- `xgeXuiWidgetSetName`

---

### xgeXuiWidgetFindById

按 ID 递归查找 widget。

**功能：**

你可以从 root 或任意子树开始查找第一个匹配 ID 的 widget。

**函数原型：**

```c
XGE_API xge_xui_widget xgeXuiWidgetFindById(xge_xui_widget pRoot, int iId);
```

**参数：**

- `pRoot`：查找起点。
- `iId`：目标 ID。

**返回值：**

- 找到时返回 widget。
- 未找到或 `pRoot == NULL` 时返回 `NULL`。

**资源归属：**

返回值是借用指针，不转移所有权。

**补充说明：**

查找使用深度优先遍历。

**范例代码：**

```c
xge_xui_widget button = xgeXuiWidgetFindById(xgeXuiRoot(&ui), 1001);
```

**相关 API：**

- `xgeXuiWidgetSetId`

---

### xgeXuiWidgetFindByName

按名称递归查找 widget。

**功能：**

你可以从 root 或任意子树开始查找第一个匹配名称的 widget。

**函数原型：**

```c
XGE_API xge_xui_widget xgeXuiWidgetFindByName(xge_xui_widget pRoot, const char* sName);
```

**参数：**

- `pRoot`：查找起点。
- `sName`：目标名称，不能为 `NULL`。

**返回值：**

- 找到时返回 widget。
- 未找到或参数非法时返回 `NULL`。

**资源归属：**

返回值是借用指针。

**补充说明：**

名称比较使用 `strcmp`。

**范例代码：**

```c
xge_xui_widget panel = xgeXuiWidgetFindByName(xgeXuiRoot(&ui), "main_panel");
```

**相关 API：**

- `xgeXuiWidgetSetName`

---

### xgeXuiWidgetSetRect

设置 widget 矩形。

**功能：**

你可以用它直接设置 widget 的绝对矩形，常用于 absolute layout 或 root 尺寸更新。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetRect(xge_xui_widget pWidget, xge_rect_t tRect);
```

**参数：**

- `pWidget`：widget 对象。
- `tRect`：目标矩形。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后会标记该 widget layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetRect(root, xgeRect(0.0f, 0.0f, 800.0f, 600.0f));
```

**相关 API：**

- `xgeXuiWidgetGetRect`
- `xgeXuiUpdate`

---

### xgeXuiWidgetGetRect

获取 widget 矩形。

**功能：**

你可以用它读取 widget 当前布局矩形。

**函数原型：**

```c
XGE_API xge_rect_t xgeXuiWidgetGetRect(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

- widget 有效时返回其矩形。
- widget 无效时返回清零矩形。

**资源归属：**

返回值是结构体值，不需要释放。

**补充说明：**

布局更新后读取更有意义。

**范例代码：**

```c
xge_rect_t rect = xgeXuiWidgetGetRect(w);
```

**相关 API：**

- `xgeXuiWidgetSetRect`
- `xgeXuiUpdate`

---

### xgeXuiWidgetSetStyle

设置 widget style。

**功能：**

你可以一次性替换 widget 的布局和外观样式。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetStyle(xge_xui_widget pWidget, const xge_xui_style_t* pStyle);
```

**参数：**

- `pWidget`：widget 对象。
- `pStyle`：style 输入，不能为 `NULL`。

**返回值：**

无。

**资源归属：**

函数复制 style 结构体，不接管输入指针。

**补充说明：**

grid columns 会被修正到至少 1，align 会被 clamp，负 radius 会被修正为 0。设置后标记 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetStyle(w, &style);
```

**相关 API：**

- `xgeXuiWidgetGetStyle`
- `xgeXuiStyleDefault`

---

### xgeXuiWidgetGetStyle

获取 widget style。

**功能：**

你可以读取 widget 当前 style。

**函数原型：**

```c
XGE_API const xge_xui_style_t* xgeXuiWidgetGetStyle(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

- widget 有效时返回内部 style 指针。
- widget 无效时返回 `NULL`。

**资源归属：**

返回值是借用指针，不能释放。

**补充说明：**

建议通过 setter 修改 style，避免绕过 dirty 标记。

**范例代码：**

```c
const xge_xui_style_t* style = xgeXuiWidgetGetStyle(w);
```

**相关 API：**

- `xgeXuiWidgetSetStyle`

---

### xgeXuiWidgetSetLayout

设置布局模式。

**功能：**

你可以设置 widget 子节点布局方式，例如 absolute、row、column、stack、grid 或 dock。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetLayout(xge_xui_widget pWidget, int iLayout);
```

**参数：**

- `pWidget`：widget 对象。
- `iLayout`：`XGE_XUI_LAYOUT_*`。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后标记 layout dirty。

**范例代码：**

```c
xgeXuiWidgetSetLayout(panel, XGE_XUI_LAYOUT_DOCK);
```

**相关 API：**

- `xgeXuiWidgetSetGrid`
- `xgeXuiWidgetSetDock`
- `xgeXuiUpdate`

---

### xgeXuiWidgetSetDock

设置 widget 在父 DockLayout 中的停靠边。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetDock(xge_xui_widget pWidget, int iDock);
XGE_API int xgeXuiWidgetGetDock(xge_xui_widget pWidget);
```

**补充说明：**

`iDock` 支持 `XGE_XUI_DOCK_TOP/BOTTOM/LEFT/RIGHT/FILL`，`XGE_XUI_DOCK_CENTER` 是 `FILL` 的别名。父布局不是 `XGE_XUI_LAYOUT_DOCK` 时该字段不会影响布局。

---

### xgeXuiWidgetSetSize

设置 widget 宽高尺寸描述。

**功能：**

你可以设置 widget 的宽高解析规则。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
```

**参数：**

- `pWidget`：widget 对象。
- `tWidth`：宽度描述。
- `tHeight`：高度描述。

**返回值：**

无。

**资源归属：**

函数复制尺寸结构体。

**补充说明：**

设置后标记 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetSize(w, xgeXuiSizePercent(100.0f), xgeXuiSizeDip(40.0f));
```

**相关 API：**

- `xgeXuiSizePx`
- `xgeXuiSizeGrow`

---

### xgeXuiWidgetSetMinSize

设置最小尺寸。

**功能：**

你可以限制 widget 布局结果的最小宽高。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetMinSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
```

**参数：**

- `pWidget`：widget 对象。
- `tWidth`：最小宽度。
- `tHeight`：最小高度。

**返回值：**

无。

**资源归属：**

函数复制尺寸描述。

**补充说明：**

设置后标记 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetMinSize(w, xgeXuiSizePx(80.0f), xgeXuiSizePx(24.0f));
```

**相关 API：**

- `xgeXuiWidgetSetMaxSize`
- `xgeXuiWidgetSetSize`

---

### xgeXuiWidgetSetMaxSize

设置最大尺寸。

**功能：**

你可以限制 widget 布局结果的最大宽高。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetMaxSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight);
```

**参数：**

- `pWidget`：widget 对象。
- `tWidth`：最大宽度。
- `tHeight`：最大高度。

**返回值：**

无。

**资源归属：**

函数复制尺寸描述。

**补充说明：**

max 值解析为大于 0 时才作为上限。

**范例代码：**

```c
xgeXuiWidgetSetMaxSize(w, xgeXuiSizePx(320.0f), xgeXuiSizePx(0.0f));
```

**相关 API：**

- `xgeXuiWidgetSetMinSize`

---

### xgeXuiWidgetSetGrid

设置 grid 布局参数。

**功能：**

你可以配置 grid 列数、行高和行列间距。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetGrid(xge_xui_widget pWidget, int iColumns, float fRowHeight, float fColumnGap, float fRowGap);
```

**参数：**

- `pWidget`：widget 对象。
- `iColumns`：列数，小于等于 0 时按 1。
- `fRowHeight`：行高，小于等于 0 时使用单元格宽度，形成方格网格。
- `fColumnGap`：列间距，小于等于 0 时按 0。
- `fRowGap`：行间距，小于等于 0 时按 0。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后标记 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetGrid(panel, 3, 32.0f, 8.0f, 8.0f);
```

**相关 API：**

- `xgeXuiWidgetSetLayout`
- `xgeXuiWidgetSetGridColumnSpan`

---

### xgeXuiWidgetSetGridColumnSpan

设置 grid 子元素横跨列数。

**功能：**

你可以让 grid 布局中的某个子元素横跨多列，用于表单标题、宽按钮、卡片头部等场景。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetGridColumnSpan(xge_xui_widget pWidget, int iColumnSpan);
```

**参数：**

- `pWidget`：grid 子元素 widget。
- `iColumnSpan`：横跨列数，小于等于 1 时按 1；大于父 grid 列数时按父 grid 列数裁剪。

**返回值：**

无。

**补充说明：**

- 第一版只支持 column span，不支持 row span 或 CSS Grid 式占用矩阵。
- 如果当前行剩余列数不足，子元素会换到下一行再放置。

**范例代码：**

```c
xgeXuiWidgetSetGridColumnSpan(title, 2);
```

**相关 API：**

- `xgeXuiWidgetSetGrid`

---

### xgeXuiWidgetSetAlign

设置子项对齐方式。

**功能：**

你可以设置 widget 在父布局槽位中的水平和垂直对齐。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetAlign(xge_xui_widget pWidget, int iAlignX, int iAlignY);
```

**参数：**

- `pWidget`：widget 对象。
- `iAlignX`：水平对齐，使用 `XGE_XUI_ALIGN_*`。
- `iAlignY`：垂直对齐，使用 `XGE_XUI_ALIGN_*`。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

非法 align 会被 clamp 为 stretch。

**范例代码：**

```c
xgeXuiWidgetSetAlign(w, XGE_XUI_ALIGN_CENTER, XGE_XUI_ALIGN_CENTER);
```

**相关 API：**

- `xgeXuiWidgetSetSize`

---

### xgeXuiWidgetSetZ

设置 widget z 顺序值。

**功能：**

你可以用它影响同级 widget 的绘制排序。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetZ(xge_xui_widget pWidget, int iZ);
```

**参数：**

- `pWidget`：widget 对象。
- `iZ`：z 值。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后会标记父节点 paint dirty；没有父节点时标记自身。

**范例代码：**

```c
xgeXuiWidgetSetZ(dialog, 100);
```

**相关 API：**

- `xgeXuiWidgetGetZ`

---

### xgeXuiWidgetGetZ

获取 widget z 顺序值。

**功能：**

你可以读取 widget 当前 z 值。

**函数原型：**

```c
XGE_API int xgeXuiWidgetGetZ(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

- widget 有效时返回 z 值。
- widget 无效时返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

z 值只在 XUI 绘制排序中使用。

**范例代码：**

```c
int z = xgeXuiWidgetGetZ(dialog);
```

**相关 API：**

- `xgeXuiWidgetSetZ`

---

### xgeXuiWidgetSetAnchorPx

设置像素锚点。

**功能：**

你可以用像素值配置 absolute/anchor 布局中的 left/top/right/bottom 约束。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetAnchorPx(xge_xui_widget pWidget, int iAnchor, float fLeft, float fTop, float fRight, float fBottom);
```

**参数：**

- `pWidget`：widget 对象。
- `iAnchor`：`XGE_XUI_ANCHOR_*` 组合。
- `fLeft`：左锚点像素。
- `fTop`：上锚点像素。
- `fRight`：右锚点像素。
- `fBottom`：下锚点像素。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

anchor 标志会被过滤到合法位；四个值会转为 px size。

**范例代码：**

```c
xgeXuiWidgetSetAnchorPx(w, XGE_XUI_ANCHOR_LEFT | XGE_XUI_ANCHOR_TOP, 8.0f, 8.0f, 0.0f, 0.0f);
```

**相关 API：**

- `xgeXuiWidgetGetAnchor`

---

### xgeXuiWidgetGetAnchor

获取 anchor 标志。

**功能：**

你可以读取 widget 当前 anchor 标志。

**函数原型：**

```c
XGE_API int xgeXuiWidgetGetAnchor(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

- widget 有效时返回 anchor 标志。
- widget 无效时返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

该函数只返回标志，不返回具体边距值。

**范例代码：**

```c
int anchor = xgeXuiWidgetGetAnchor(w);
```

**相关 API：**

- `xgeXuiWidgetSetAnchorPx`

---

### xgeXuiWidgetSetMarginPx

设置像素 margin。

**功能：**

你可以设置 widget 四边外边距。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetMarginPx(xge_xui_widget pWidget, float fLeft, float fTop, float fRight, float fBottom);
```

**参数：**

- `pWidget`：widget 对象。
- `fLeft` / `fTop` / `fRight` / `fBottom`：四边 margin 像素值。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后标记 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetMarginPx(w, 4.0f, 4.0f, 4.0f, 4.0f);
```

**相关 API：**

- `xgeXuiWidgetSetPaddingPx`

---

### xgeXuiWidgetSetPaddingPx

设置像素 padding。

**功能：**

你可以设置 widget 四边内边距，影响 content rect 和内容测量。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetPaddingPx(xge_xui_widget pWidget, float fLeft, float fTop, float fRight, float fBottom);
```

**参数：**

- `pWidget`：widget 对象。
- `fLeft` / `fTop` / `fRight` / `fBottom`：四边 padding 像素值。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后标记 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetPaddingPx(panel, 8.0f, 8.0f, 8.0f, 8.0f);
```

**相关 API：**

- `xgeXuiWidgetSetMarginPx`

---

### xgeXuiWidgetSetBackground

设置背景色。

**功能：**

你可以设置 widget 默认绘制背景颜色。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetBackground(xge_xui_widget pWidget, uint32_t iColor);
```

**参数：**

- `pWidget`：widget 对象。
- `iColor`：背景色。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后只标记 paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetBackground(panel, XGE_COLOR_RGBA(32, 38, 46, 255));
```

**相关 API：**

- `xgeXuiWidgetSetRadius`

---

### xgeXuiWidgetSetRadius

设置圆角半径。

**功能：**

你可以设置 widget 背景绘制的圆角半径。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetRadius(xge_xui_widget pWidget, float fRadius);
```

**参数：**

- `pWidget`：widget 对象。
- `fRadius`：圆角半径，小于 0 时按 0。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

设置后标记 paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetRadius(panel, 4.0f);
```

**相关 API：**

- `xgeXuiWidgetSetBackground`

---

### xgeXuiWidgetGetFlags

获取 widget flags。

**功能：**

你可以读取 widget 当前可见、启用、可聚焦、裁剪和 dirty 状态。

**函数原型：**

```c
XGE_API uint32_t xgeXuiWidgetGetFlags(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

- widget 有效时返回 flags。
- widget 无效时返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

flags 使用 `XGE_XUI_WIDGET_*`。

**范例代码：**

```c
uint32_t flags = xgeXuiWidgetGetFlags(w);
```

**相关 API：**

- `xgeXuiWidgetSetVisible`
- `xgeXuiWidgetSetEnabled`

---

### xgeXuiWidgetSetVisible

设置可见状态。

**功能：**

你可以显示或隐藏 widget。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetVisible(xge_xui_widget pWidget, int bVisible);
```

**参数：**

- `pWidget`：widget 对象。
- `bVisible`：非 0 可见，0 隐藏。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

状态变化会标记 layout/paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetVisible(dialog, 1);
```

**相关 API：**

- `xgeXuiWidgetIsVisible`

---

### xgeXuiWidgetSetEnabled

设置启用状态。

**功能：**

你可以启用或禁用 widget。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetEnabled(xge_xui_widget pWidget, int bEnabled);
```

**参数：**

- `pWidget`：widget 对象。
- `bEnabled`：非 0 启用，0 禁用。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

禁用状态会影响命中、交互和可聚焦判断。

**范例代码：**

```c
xgeXuiWidgetSetEnabled(button, 0);
```

**相关 API：**

- `xgeXuiWidgetIsEnabled`

---

### xgeXuiWidgetSetFocusable

设置可聚焦状态。

**功能：**

你可以允许或禁止 widget 获得焦点。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetFocusable(xge_xui_widget pWidget, int bFocusable);
```

**参数：**

- `pWidget`：widget 对象。
- `bFocusable`：非 0 可聚焦，0 不可聚焦。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

实际可聚焦还要求 widget 可见且启用。

**范例代码：**

```c
xgeXuiWidgetSetFocusable(input, 1);
```

**相关 API：**

- `xgeXuiWidgetIsFocusable`
- `xgeXuiSetFocus`

---

### xgeXuiWidgetSetClip

设置裁剪状态。

**功能：**

你可以让 widget 绘制时裁剪到自身 content rect，并阻止命中测试继续进入 content rect 外的子节点。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetClip(xge_xui_widget pWidget, int bClip);
```

**参数：**

- `pWidget`：widget 对象。
- `bClip`：非 0 启用裁剪，0 关闭裁剪。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

- 状态变化会触发布局和绘制刷新。
- `bClip` 为 0 时，子节点可以按布局结果溢出显示和命中。
- `bClip` 非 0 时，绘制裁剪到 `tContentRect`；命中点如果仍在 widget 矩形内但落在 `tContentRect` 外，会命中该 widget 本身，不会命中溢出的子节点。

**范例代码：**

```c
xgeXuiWidgetSetClip(scroll, 1);
```

**相关 API：**

- `xgeXuiPaint`

---

### xgeXuiWidgetSetCaptureEvent

设置捕获阶段事件回调。

**功能：**

你可以用它在事件冒泡前处理事件。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetCaptureEvent(xge_xui_widget pWidget, xge_xui_event_proc procEvent);
```

**参数：**

- `pWidget`：widget 对象。
- `procEvent`：捕获事件回调，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数只保存函数指针，不接管用户数据。

**补充说明：**

捕获回调返回 `XGE_XUI_EVENT_CONSUMED` 时会阻止后续普通派发。

**范例代码：**

```c
xgeXuiWidgetSetCaptureEvent(w, on_capture);
```

**相关 API：**

- `xgeXuiDispatchEvent`

---

### xgeXuiWidgetSetMeasure

设置测量回调。

**功能：**

你可以用它为自定义 widget 提供内容尺寸。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetMeasure(xge_xui_widget pWidget, xge_xui_measure_proc procMeasure);
```

**参数：**

- `pWidget`：widget 对象。
- `procMeasure`：测量回调，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数只保存回调指针。

**补充说明：**

设置后标记 layout dirty。测量时会把 widget 的 `pUser` 传给回调。

**范例代码：**

```c
xgeXuiWidgetSetMeasure(label_widget, measure_label);
```

**相关 API：**

- `xgeXuiSizeContent`
- `xgeXuiUpdate`

---

### xgeXuiWidgetSetPaint

设置绘制回调。

**功能：**

你可以用它为自定义 widget 提供绘制逻辑和用户数据。

**函数原型：**

```c
XGE_API void xgeXuiWidgetSetPaint(xge_xui_widget pWidget, xge_xui_paint_proc procPaint, void* pUser);
```

**参数：**

- `pWidget`：widget 对象。
- `procPaint`：绘制回调，可以为 `NULL`。
- `pUser`：用户数据指针。

**返回值：**

无。

**资源归属：**

函数只保存回调和 `pUser` 借用指针。

**补充说明：**

设置后标记 paint dirty。

**范例代码：**

```c
xgeXuiWidgetSetPaint(w, paint_custom, user);
```

**相关 API：**

- `xgeXuiPaint`

---

### xgeXuiWidgetIsVisible

判断 widget 是否可见。

**功能：**

你可以用它读取 visible 标志。

**函数原型：**

```c
XGE_API int xgeXuiWidgetIsVisible(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

可见返回非 0，否则返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

`pWidget == NULL` 时返回 0。

**范例代码：**

```c
if (xgeXuiWidgetIsVisible(w)) {
}
```

**相关 API：**

- `xgeXuiWidgetSetVisible`

---

### xgeXuiWidgetIsEnabled

判断 widget 是否启用。

**功能：**

你可以用它读取 enabled 标志。

**函数原型：**

```c
XGE_API int xgeXuiWidgetIsEnabled(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

启用返回非 0，否则返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

`pWidget == NULL` 时返回 0。

**范例代码：**

```c
int enabled = xgeXuiWidgetIsEnabled(w);
```

**相关 API：**

- `xgeXuiWidgetSetEnabled`

---

### xgeXuiWidgetIsFocusable

判断 widget 是否可聚焦。

**功能：**

你可以用它判断 widget 是否满足可见、启用、focusable 三个条件。

**函数原型：**

```c
XGE_API int xgeXuiWidgetIsFocusable(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

可聚焦返回非 0，否则返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

该函数用于 `xgeXuiSetFocus` 的合法性判断。

**范例代码：**

```c
if (xgeXuiWidgetIsFocusable(input)) {
    xgeXuiSetFocus(&ui, input);
}
```

**相关 API：**

- `xgeXuiWidgetSetFocusable`
- `xgeXuiSetFocus`

---

### xgeXuiWidgetMarkLayout

标记 layout dirty。

**功能：**

你可以用它通知 XUI 某个 widget 的布局需要重新计算。

**函数原型：**

```c
XGE_API void xgeXuiWidgetMarkLayout(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

非批处理状态下会向父链传播 dirty 并请求刷新；批处理状态下只记录批处理 dirty。

**范例代码：**

```c
xgeXuiWidgetMarkLayout(w);
```

**相关 API：**

- `xgeXuiLayoutBatchBegin`
- `xgeXuiUpdate`

---

### xgeXuiWidgetMarkPaint

标记 paint dirty。

**功能：**

你可以用它通知 XUI 某个 widget 需要重绘。

**函数原型：**

```c
XGE_API void xgeXuiWidgetMarkPaint(xge_xui_widget pWidget);
```

**参数：**

- `pWidget`：widget 对象。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

非批处理状态下会向父链传播 dirty、尝试 invalidate widget 矩形并请求刷新。

**范例代码：**

```c
xgeXuiWidgetMarkPaint(w);
```

**相关 API：**

- `xgeXuiPaint`
- `xgeXuiRefreshRequest`

---

### xgeXuiWidgetMarkStyle

标记 style cache dirty。

```c
XGE_API void xgeXuiWidgetMarkStyle(xge_xui_widget pWidget);
```

第一版 XSON loader 会把 layout/visual 高频字段解析到 widget 的 `xge_xui_style_t`，这个结构就是轻量 style cache。`xgeXuiWidgetMarkStyle` 设置 `XGE_XUI_WIDGET_DIRTY_STYLE`，递增 widget style version，并同时触发布局与绘制 dirty。

---

### xgeXuiHitTest

命中测试。

**功能：**

你可以用它根据坐标找到当前最合适的 widget 目标。

**函数原型：**

```c
XGE_API xge_xui_widget xgeXuiHitTest(xge_xui_context pContext, float fX, float fY);
```

**参数：**

- `pContext`：XUI context。
- `fX`：X 坐标。
- `fY`：Y 坐标。

**返回值：**

- 命中时返回 widget。
- context 无效或无命中时返回 `NULL`。

**资源归属：**

返回值是借用指针。

**补充说明：**

事件派发中有坐标的事件会使用该函数寻找目标。

**范例代码：**

```c
xge_xui_widget hit = xgeXuiHitTest(&ui, x, y);
```

**相关 API：**

- `xgeXuiDispatchEvent`

---

### xgeXuiSetFocus

设置焦点 widget。

**功能：**

你可以用它改变当前键盘/文本事件焦点。

**函数原型：**

```c
XGE_API void xgeXuiSetFocus(xge_xui_context pContext, xge_xui_widget pWidget);
```

**参数：**

- `pContext`：XUI context。
- `pWidget`：目标 widget，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数不接管 widget 所有权。

**补充说明：**

不可聚焦 widget 会被忽略。焦点变化会派发 focus out/in 事件并标记旧/新焦点重绘。

**范例代码：**

```c
xgeXuiSetFocus(&ui, input);
```

**相关 API：**

- `xgeXuiWidgetSetFocusable`
- `xgeXuiWidgetIsFocusable`

---

### xgeXuiSetCapture

设置事件捕获 widget。

**功能：**

你可以用它让后续事件优先派发给指定 widget，常用于拖动和按下捕获。

**函数原型：**

```c
XGE_API void xgeXuiSetCapture(xge_xui_context pContext, xge_xui_widget pWidget);
```

**参数：**

- `pContext`：XUI context。
- `pWidget`：捕获 widget；传 `NULL` 释放捕获。

**返回值：**

无。

**资源归属：**

函数只保存借用指针。

**补充说明：**

context 无效时无效果。

**范例代码：**

```c
xgeXuiSetCapture(&ui, slider);
```

**相关 API：**

- `xgeXuiDispatchEvent`

---

### xgeXuiDispatchEvent

派发单个事件。

**功能：**

你可以用它把 XGE 输入事件派发到 XUI widget tree。

**函数原型：**

```c
XGE_API int xgeXuiDispatchEvent(xge_xui_context pContext, const xge_event_t* pEvent);
```

**参数：**

- `pContext`：XUI context。
- `pEvent`：事件指针，不能为 `NULL` 才能生效。

**返回值：**

- 事件被消费返回 `XGE_XUI_EVENT_CONSUMED`。
- 继续传播返回 `XGE_XUI_EVENT_CONTINUE`。

**资源归属：**

函数不保存事件指针。

**补充说明：**

有 capture 时优先给 capture；有坐标事件走 hit test；其他事件走 focus，focus 为空时走 root。

**范例代码：**

```c
xgeXuiDispatchEvent(&ui, &event);
```

**相关 API：**

- `xgeXuiEventPush`
- `xgeXuiDispatchQueuedEvents`

---

### xgeXuiEventPush

压入事件队列。

**功能：**

你可以把事件复制到 XUI 队列，稍后统一派发。

**函数原型：**

```c
XGE_API int xgeXuiEventPush(xge_xui_context pContext, const xge_event_t* pEvent);
```

**参数：**

- `pContext`：XUI context。
- `pEvent`：事件指针，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 队列已满返回 `XGE_ERROR`。

**资源归属：**

函数复制事件结构体，不持有 `pEvent` 指针。

**补充说明：**

队列容量为 `XGE_XUI_EVENT_QUEUE_CAPACITY`。

**范例代码：**

```c
xgeXuiEventPush(&ui, &event);
```

**相关 API：**

- `xgeXuiEventPop`
- `xgeXuiDispatchQueuedEvents`

---

### xgeXuiEventPop

弹出事件队列。

**功能：**

你可以从 XUI 队列取出最早进入的事件。

**函数原型：**

```c
XGE_API int xgeXuiEventPop(xge_xui_context pContext, xge_event_t* pEvent);
```

**参数：**

- `pContext`：XUI context。
- `pEvent`：输出事件，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 队列为空返回 `XGE_ERROR`。

**资源归属：**

函数写入调用者提供的事件结构体。

**补充说明：**

队列使用环形 head/tail。

**范例代码：**

```c
xge_event_t event;
xgeXuiEventPop(&ui, &event);
```

**相关 API：**

- `xgeXuiEventPush`
- `xgeXuiEventCount`

---

### xgeXuiEventCount

获取事件队列长度。

**功能：**

你可以用它查询当前排队事件数量。

**函数原型：**

```c
XGE_API int xgeXuiEventCount(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

- context 有效时返回事件数量。
- context 无效时返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

该数量不会超过 `XGE_XUI_EVENT_QUEUE_CAPACITY`。

**范例代码：**

```c
int count = xgeXuiEventCount(&ui);
```

**相关 API：**

- `xgeXuiEventPush`
- `xgeXuiEventPop`

---

### xgeXuiDispatchQueuedEvents

派发当前队列中的事件。

**功能：**

你可以用它一次处理当前已经排队的事件。

**函数原型：**

```c
XGE_API int xgeXuiDispatchQueuedEvents(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

- 成功返回已处理事件数量。
- context 无效返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

函数不接管任何资源。

**补充说明：**

函数只处理调用开始时的队列长度；派发过程中新增事件会留到后续处理。

**范例代码：**

```c
int processed = xgeXuiDispatchQueuedEvents(&ui);
```

**相关 API：**

- `xgeXuiDispatchEvent`
- `xgeXuiEventPush`

---

### xgeXuiUpdate

更新 XUI 布局。

**功能：**

你可以在绘制前调用它，让 XUI 根据 root rect、style 和 measure 回调计算布局。

**函数原型：**

```c
XGE_API int xgeXuiUpdate(xge_xui_context pContext, float fDelta);
```

**参数：**

- `pContext`：XUI context。
- `fDelta`：帧间隔。当前实现暂未使用。

**返回值：**

- 成功返回 `XGE_OK`。
- context 无效返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

函数不接管任何资源。

**补充说明：**

root 宽高小于等于 0 时会使用当前 XGE 窗口宽高。更新后清空 dirty layout 计数。

**范例代码：**

```c
xgeXuiUpdate(&ui, dt);
```

**相关 API：**

- `xgeXuiPaint`
- `xgeXuiWidgetMarkLayout`

---

### xgeXuiPaint

绘制 XUI。

**功能：**

你可以用它遍历 widget tree，调用 host 绘制背景、裁剪、控件内容和自定义 paint。

**函数原型：**

```c
XGE_API int xgeXuiPaint(xge_xui_context pContext);
```

**参数：**

- `pContext`：XUI context。

**返回值：**

- 成功绘制时返回 paint command 数量。
- 无需重绘时返回 0。
- context 无效返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

函数不接管任何资源。

**补充说明：**

如果 root 没有 paint dirty，会清空 paint/refresh 状态并返回 0。绘制后会恢复 active host/context。

**范例代码：**

```c
xgeXuiUpdate(&ui, dt);
xgeXuiPaint(&ui);
```

**相关 API：**

- `xgeXuiUpdate`
- `xgeXuiWidgetMarkPaint`

---

## API 参考：Text

### xgeXuiTextInit

初始化 XUI 文本缓冲。

**功能：**

为 `xge_xui_text_t` 分配初始 UTF-8 文本缓冲和 IME composition 缓冲，作为输入框、文本编辑控件或自定义文本状态的基础结构。

**函数原型：**

```c
XGE_API int xgeXuiTextInit(xge_xui_text pText);
```

**参数：**

- `pText`：要初始化的文本对象。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 内存分配失败返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

函数会通过 xrt allocator 为文本对象内部缓冲分配内存。调用者必须在不再使用时调用 `xgeXuiTextUnit` 释放。

**补充说明：**

- 初始化后 `sText` 和 `sComposition` 都是空字符串。
- `xge_xui_text_t` 是透明结构，但不要直接释放内部指针。

**范例代码：**

```c
xge_xui_text_t text;
if (xgeXuiTextInit(&text) == XGE_OK) {
	xgeXuiTextSet(&text, "hello");
	xgeXuiTextUnit(&text);
}
```

**相关 API：**

- `xgeXuiTextUnit`
- `xgeXuiTextSet`

---

### xgeXuiTextUnit

释放 XUI 文本缓冲。

**功能：**

释放 `xge_xui_text_t` 内部持有的主文本缓冲和 IME composition 缓冲，并将结构清零。

**函数原型：**

```c
XGE_API void xgeXuiTextUnit(xge_xui_text pText);
```

**参数：**

- `pText`：要释放的文本对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

只释放 `xgeXuiTextInit` 或后续扩容创建的内部缓冲，不释放 `pText` 结构本身。

**补充说明：**

- 释放后文本对象回到全零状态。
- 如果文本对象嵌在控件结构内，由控件的 `Unit` 函数负责调用。

**范例代码：**

```c
xge_xui_text_t text;
xgeXuiTextInit(&text);
xgeXuiTextUnit(&text);
```

**相关 API：**

- `xgeXuiTextInit`

---

### xgeXuiTextSet

设置完整文本内容。

**功能：**

用新的 UTF-8 字符串替换文本缓冲内容，并把游标和选择区移动到文本末尾。

**函数原型：**

```c
XGE_API int xgeXuiTextSet(xge_xui_text pText, const char* sText);
```

**参数：**

- `pText`：文本对象。
- `sText`：新的 UTF-8 文本；传入 `NULL` 等同于空字符串。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 扩容失败返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

函数会复制 `sText` 内容，调用者可以在返回后释放或修改原始字符串。

**补充说明：**

- 文本长度以 UTF-8 字节数保存。
- 设置主文本会清空 composition。

**范例代码：**

```c
xgeXuiTextSet(&text, "player name");
```

**相关 API：**

- `xgeXuiTextInsert`
- `xgeXuiTextClearComposition`

---

### xgeXuiTextInsert

在游标位置插入文本。

**功能：**

将 UTF-8 字符串插入当前游标位置。如果存在选择区，会先删除选择区，再插入新文本。

**函数原型：**

```c
XGE_API int xgeXuiTextInsert(xge_xui_text pText, const char* sText);
```

**参数：**

- `pText`：文本对象。
- `sText`：要插入的 UTF-8 字符串，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 初始化或扩容失败返回对应错误码。

**资源归属：**

函数复制插入内容，不持有 `sText` 指针。

**补充说明：**

- 如果文本对象尚未初始化，当前实现会尝试自动初始化。
- 插入后游标移动到插入文本之后，选择区折叠到游标处。

**范例代码：**

```c
xgeXuiTextSetCursor(&text, 0);
xgeXuiTextInsert(&text, "XGE ");
```

**相关 API：**

- `xgeXuiTextSetCursor`
- `xgeXuiTextDeleteBack`

---

### xgeXuiTextInsertCodepoint

插入一个 Unicode codepoint。

**功能：**

将 `uint32_t` codepoint 编码为 UTF-8，并插入当前游标位置。

**函数原型：**

```c
XGE_API int xgeXuiTextInsertCodepoint(xge_xui_text pText, uint32_t iCodepoint);
```

**参数：**

- `pText`：文本对象。
- `iCodepoint`：要插入的 Unicode codepoint。

**返回值：**

- 成功返回 `XGE_OK`。
- 不支持的 codepoint 返回 `XGE_ERROR_UNSUPPORTED`。
- 插入失败返回 `xgeXuiTextInsert` 的错误码。

**资源归属：**

不接管外部资源。

**补充说明：**

- 当前实现支持 1 到 3 字节 UTF-8 编码范围，并排除 surrogate 区间。
- 超出当前支持范围的字符会返回不支持。

**范例代码：**

```c
xgeXuiTextInsertCodepoint(&text, 0x4E2D);
```

**相关 API：**

- `xgeXuiTextInsert`
- `xgeXuiTextInputEvent`

---

### xgeXuiTextDeleteBack

删除游标前一个字符。

**功能：**

实现退格行为。如果存在选择区，则删除选择区；否则删除游标前一个 UTF-8 字符。

**函数原型：**

```c
XGE_API int xgeXuiTextDeleteBack(xge_xui_text pText);
```

**参数：**

- `pText`：文本对象。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空或文本未初始化返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

不释放文本对象，只在内部缓冲中移动内容。

**补充说明：**

- 游标位于开头时返回成功但不修改内容。
- 删除后会清空 composition。

**范例代码：**

```c
xgeXuiTextDeleteBack(&text);
```

**相关 API：**

- `xgeXuiTextDeleteForward`
- `xgeXuiTextSetSelection`

---

### xgeXuiTextDeleteForward

删除游标后一个字符。

**功能：**

实现 Delete 键行为。如果存在选择区，则删除选择区；否则删除游标后的一个 UTF-8 字符。

**函数原型：**

```c
XGE_API int xgeXuiTextDeleteForward(xge_xui_text pText);
```

**参数：**

- `pText`：文本对象。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空或文本未初始化返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

不释放文本对象，只在内部缓冲中移动内容。

**补充说明：**

- 游标位于末尾时返回成功但不修改内容。
- 删除后会清空 composition。

**范例代码：**

```c
xgeXuiTextDeleteForward(&text);
```

**相关 API：**

- `xgeXuiTextDeleteBack`
- `xgeXuiTextGetCursor`

---

### xgeXuiTextSetCursor

设置文本游标位置。

**功能：**

把游标移动到指定 UTF-8 字节偏移，并自动夹到合法字符边界，同时清空选择区。

**函数原型：**

```c
XGE_API void xgeXuiTextSetCursor(xge_xui_text pText, int iCursor);
```

**参数：**

- `pText`：文本对象，可以为 `NULL`。
- `iCursor`：目标字节偏移。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 位置小于 0 会夹到 0。
- 位置大于文本长度会夹到文本末尾。
- 如果落在 UTF-8 continuation byte 上，会向前移动到字符起点。

**范例代码：**

```c
xgeXuiTextSetCursor(&text, text.iSize);
```

**相关 API：**

- `xgeXuiTextGetCursor`
- `xgeXuiTextSetSelection`

---

### xgeXuiTextGetCursor

获取文本游标位置。

**功能：**

返回当前游标的 UTF-8 字节偏移，并在返回前把游标修正到合法字符边界。

**函数原型：**

```c
XGE_API int xgeXuiTextGetCursor(xge_xui_text pText);
```

**参数：**

- `pText`：文本对象。

**返回值：**

- 返回当前游标字节偏移。
- `pText` 为 `NULL` 时返回 0。

**资源归属：**

不接管外部资源。

**补充说明：**

- 返回值是字节偏移，不是 Unicode 字符序号。

**范例代码：**

```c
int cursor = xgeXuiTextGetCursor(&text);
```

**相关 API：**

- `xgeXuiTextSetCursor`

---

### xgeXuiTextSetSelection

设置文本选择区。

**功能：**

设置选择区起点和终点，并把游标移动到选择区终点。

**函数原型：**

```c
XGE_API void xgeXuiTextSetSelection(xge_xui_text pText, int iStart, int iEnd);
```

**参数：**

- `pText`：文本对象，可以为 `NULL`。
- `iStart`：选择起点字节偏移。
- `iEnd`：选择终点字节偏移。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 起点和终点都会被夹到合法 UTF-8 字符边界。
- 可以传入反向选择，读取时会自动归一化为小值到大值。

**范例代码：**

```c
xgeXuiTextSetSelection(&text, 0, text.iSize);
```

**相关 API：**

- `xgeXuiTextGetSelection`
- `xgeXuiTextInsert`

---

### xgeXuiTextGetSelection

获取文本选择区。

**功能：**

读取当前选择区，并将选择区归一化为 `[start, end]` 顺序。

**函数原型：**

```c
XGE_API void xgeXuiTextGetSelection(xge_xui_text pText, int* pStart, int* pEnd);
```

**参数：**

- `pText`：文本对象，可以为 `NULL`。
- `pStart`：用于接收选择起点，可为 `NULL`。
- `pEnd`：用于接收选择终点，可为 `NULL`。

**返回值：**

无。

**资源归属：**

输出参数由调用者持有。

**补充说明：**

- `pText` 为 `NULL` 时起点和终点按 0 返回。
- 输出值是 UTF-8 字节偏移。

**范例代码：**

```c
int start;
int end;
xgeXuiTextGetSelection(&text, &start, &end);
```

**相关 API：**

- `xgeXuiTextSetSelection`

---

### xgeXuiTextSetComposition

设置 IME 组合文本。

**功能：**

保存系统 IME 正在组合但尚未提交的 UTF-8 文本，供输入框绘制候选状态或预编辑文本。

**函数原型：**

```c
XGE_API int xgeXuiTextSetComposition(xge_xui_text pText, const char* sText);
```

**参数：**

- `pText`：文本对象。
- `sText`：composition 文本；传入 `NULL` 等同于空字符串。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 扩容失败返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

函数复制 `sText` 内容，不持有外部字符串指针。

**补充说明：**

- composition 不会立刻写入主文本缓冲。
- 系统提交文本后应通过 `XGE_EVENT_TEXT` 或直接调用插入 API 写入主文本。

**范例代码：**

```c
xgeXuiTextSetComposition(&text, "zhong");
```

**相关 API：**

- `xgeXuiTextClearComposition`
- `xgeXuiTextGetComposition`

---

### xgeXuiTextClearComposition

清空 IME 组合文本。

**功能：**

将 composition 缓冲重置为空字符串。

**函数原型：**

```c
XGE_API void xgeXuiTextClearComposition(xge_xui_text pText);
```

**参数：**

- `pText`：文本对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 composition 缓冲，只清空内容。

**补充说明：**

- 设置主文本、插入文本、删除文本时会自动清空 composition。

**范例代码：**

```c
xgeXuiTextClearComposition(&text);
```

**相关 API：**

- `xgeXuiTextSetComposition`
- `xgeXuiTextGetComposition`

---

### xgeXuiTextGetComposition

获取 IME 组合文本。

**功能：**

返回当前 composition 字符串，用于输入框绘制预编辑文本。

**函数原型：**

```c
XGE_API const char* xgeXuiTextGetComposition(xge_xui_text pText);
```

**参数：**

- `pText`：文本对象。

**返回值：**

- 返回 composition 字符串。
- 文本对象为空或未初始化时返回空字符串。

**资源归属：**

返回值是借用指针，由文本对象内部持有。调用者不能释放，也不应长期保存。

**补充说明：**

- 后续 `xgeXuiTextSetComposition`、`xgeXuiTextClearComposition` 或 `xgeXuiTextUnit` 可能让该指针失效或内容变化。

**范例代码：**

```c
const char* composing = xgeXuiTextGetComposition(&text);
```

**相关 API：**

- `xgeXuiTextSetComposition`
- `xgeXuiTextClearComposition`

---

### xgeXuiTextInputEvent

处理文本输入事件。

**功能：**

把 XGE 文本事件和 IME 事件应用到 `xge_xui_text_t`：文本事件插入 codepoint，IME start/update/end 维护 composition 状态。

**函数原型：**

```c
XGE_API int xgeXuiTextInputEvent(xge_xui_text pText, const xge_event_t* pEvent);
```

**参数：**

- `pText`：文本对象。
- `pEvent`：输入事件。

**返回值：**

- 已处理事件返回 `XGE_XUI_EVENT_CONSUMED`。
- 不相关事件或插入失败返回 `XGE_XUI_EVENT_CONTINUE`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

不接管事件对象。事件数据由事件生产方管理。

**补充说明：**

- `XGE_EVENT_TEXT` 使用 `pEvent->iCodepoint`。
- `XGE_EVENT_IME_UPDATE` 会读取 `pEvent->pData` 中的 `xge_ime_event_t`。
- 该函数只处理文本缓冲，不负责标记控件重绘；控件事件过程应在内容变化后标记 paint。

**范例代码：**

```c
int handled = xgeXuiTextInputEvent(&text, event);
if (handled == XGE_XUI_EVENT_CONSUMED) {
	xgeXuiWidgetMarkPaint(widget);
}
```

**相关 API：**

- `xgeXuiInputEvent`
- `xgeXuiTextInsertCodepoint`

---

## API 参考：Button

### xgeXuiButtonInit

初始化按钮控件。

**功能：**

把 `xge_xui_button_t` 绑定到指定 widget，并安装按钮默认事件处理、绘制回调、主题颜色和可聚焦状态。

**函数原型：**

```c
XGE_API int xgeXuiButtonInit(xge_xui_button pButton, xge_xui_context pContext, xge_xui_widget pWidget);
```

**参数：**

- `pButton`：按钮对象。
- `pContext`：所属 XUI context。
- `pWidget`：承载按钮的 widget。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

按钮对象和 widget 都由调用者持有。函数不会分配按钮对象，也不会接管 widget 生命周期。

**补充说明：**

- 初始化会把 `pWidget->procEvent` 设置为 `xgeXuiButtonEventProc`。
- 初始化会把 `pWidget->procPaint` 设置为 `xgeXuiButtonPaintProc`。
- 默认字体和颜色来自 context theme。

**范例代码：**

```c
xge_xui_widget buttonWidget = xgeXuiWidgetCreate(&ui);
xge_xui_button_t button;
xgeXuiButtonInit(&button, &ui, buttonWidget);
```

**相关 API：**

- `xgeXuiButtonUnit`
- `xgeXuiButtonSetClick`

---

### xgeXuiButtonUnit

释放按钮控件绑定。

**功能：**

解除按钮对 widget 的事件和绘制回调绑定，并清空按钮对象。

**函数原型：**

```c
XGE_API void xgeXuiButtonUnit(xge_xui_button pButton);
```

**参数：**

- `pButton`：按钮对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放按钮结构本身，也不释放绑定的 widget、字体或文本字符串。

**补充说明：**

- 只有当 widget 的 `pUser` 仍指向该按钮时，才会清理 widget 回调，避免误清其他控件接管后的回调。

**范例代码：**

```c
xgeXuiButtonUnit(&button);
xgeXuiWidgetFree(buttonWidget);
```

**相关 API：**

- `xgeXuiButtonInit`

---

### xgeXuiButtonSetClick

设置按钮点击回调。

**功能：**

注册按钮点击处理函数和用户数据。按钮在按下后于控件内部释放并仍位于按钮范围内时触发点击。

**函数原型：**

```c
XGE_API void xgeXuiButtonSetClick(xge_xui_button pButton, xge_xui_click_proc procClick, void* pUser);
```

**参数：**

- `pButton`：按钮对象，可以为 `NULL`。
- `procClick`：点击回调，可为 `NULL`。
- `pUser`：传给点击回调的用户数据。

**返回值：**

无。

**资源归属：**

`pUser` 由调用者管理，XUI 只保存指针并在回调时原样传回。

**补充说明：**

- 清空回调可传入 `procClick = NULL`。
- 点击计数会记录到按钮内部 `iClickCount`。

**范例代码：**

```c
static void on_ok(xge_xui_widget widget, void* user) {
	(void)widget;
	(void)user;
}

xgeXuiButtonSetClick(&button, on_ok, NULL);
```

**相关 API：**

- `xgeXuiButtonEvent`

---

### xgeXuiButtonSetText

设置按钮文字。

**功能：**

设置按钮使用的字体和显示文本，并标记承载 widget 需要重新布局和重绘。

**函数原型：**

```c
XGE_API void xgeXuiButtonSetText(xge_xui_button pButton, xge_font pFont, const char* sText);
```

**参数：**

- `pButton`：按钮对象，可以为 `NULL`。
- `pFont`：用于绘制按钮文字的字体，可以为 `NULL`。
- `sText`：按钮文字；传入 `NULL` 等同于空字符串。

**返回值：**

无。

**资源归属：**

按钮保存 `sText` 的借用指针，不复制字符串。字体资源由调用者或资源系统管理。

**补充说明：**

- 文本必须在按钮绘制期间保持有效。
- 默认文本对齐为水平居中、垂直居中并裁剪。

**范例代码：**

```c
xgeXuiButtonSetText(&button, font, "Start");
```

**相关 API：**

- `xgeXuiButtonSetTextColor`
- `xgeXuiWidgetMarkLayout`

---

### xgeXuiButtonSetTextColor

设置按钮文字颜色。

**功能：**

修改按钮文本颜色，并标记承载 widget 需要重绘。

**函数原型：**

```c
XGE_API void xgeXuiButtonSetTextColor(xge_xui_button pButton, uint32_t iColor);
```

**参数：**

- `pButton`：按钮对象，可以为 `NULL`。
- `iColor`：文本颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 颜色使用 XGE 当前统一颜色格式。

**范例代码：**

```c
xgeXuiButtonSetTextColor(&button, XGE_COLOR_RGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeXuiButtonSetText`
- `xgeXuiButtonSetColors`

---

### xgeXuiButtonSetColors

设置按钮各状态背景颜色。

**功能：**

设置按钮 normal、hover、active、focus、disabled 状态对应的背景颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiButtonSetColors(xge_xui_button pButton, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);
```

**参数：**

- `pButton`：按钮对象，可以为 `NULL`。
- `iNormal`：普通状态颜色。
- `iHover`：悬停状态颜色。
- `iActive`：按下状态颜色。
- `iFocus`：焦点状态颜色。
- `iDisabled`：禁用状态颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 透明度为 0 的颜色不会绘制背景。
- 实际绘制颜色由当前状态决定。

**范例代码：**

```c
xgeXuiButtonSetColors(&button,
	XGE_COLOR_RGBA(40, 40, 44, 255),
	XGE_COLOR_RGBA(56, 56, 62, 255),
	XGE_COLOR_RGBA(28, 28, 32, 255),
	XGE_COLOR_RGBA(64, 96, 160, 255),
	XGE_COLOR_RGBA(24, 24, 26, 180));
```

**相关 API：**

- `xgeXuiButtonGetState`

---

### xgeXuiButtonGetState

获取按钮当前状态。

**功能：**

返回按钮的当前交互状态，并根据 widget 可见和启用状态刷新 disabled 标记。

**函数原型：**

```c
XGE_API int xgeXuiButtonGetState(xge_xui_button pButton);
```

**参数：**

- `pButton`：按钮对象。

**返回值：**

- 返回 `XGE_XUI_STATE_*` 状态位组合。
- `pButton` 为 `NULL` 时返回 `XGE_XUI_STATE_DISABLED`。

**资源归属：**

不接管外部资源。

**补充说明：**

- 状态可能包含 normal、hover、active、focus、disabled 等位。

**范例代码：**

```c
int state = xgeXuiButtonGetState(&button);
```

**相关 API：**

- `xgeXuiButtonEvent`

---

### xgeXuiButtonEvent

处理按钮事件。

**功能：**

处理鼠标和触控输入，维护按钮 hover/active 状态、焦点、捕获和点击回调。

**函数原型：**

```c
XGE_API int xgeXuiButtonEvent(xge_xui_button pButton, const xge_event_t* pEvent);
```

**参数：**

- `pButton`：按钮对象。
- `pEvent`：输入事件。

**返回值：**

- 事件被按钮处理时返回 `XGE_XUI_EVENT_CONSUMED`。
- 不相关事件或不在按钮范围内时返回 `XGE_XUI_EVENT_CONTINUE`。

**资源归属：**

不接管事件对象。

**补充说明：**

- 按下时会请求 focus 和 pointer capture。
- 释放时如果仍在按钮范围内，会触发点击回调。
- 控件不可见或禁用时不会处理点击。

**范例代码：**

```c
return xgeXuiButtonEvent(&button, event);
```

**相关 API：**

- `xgeXuiButtonSetClick`
- `xgeXuiSetCapture`

---

### xgeXuiButtonEventProc

按钮 widget 事件适配器。

**功能：**

把 widget 事件回调参数转发给 `xgeXuiButtonEvent`，用于挂接到 `xge_xui_widget_t::procEvent`。

**函数原型：**

```c
XGE_API int xgeXuiButtonEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
```

**参数：**

- `pWidget`：触发事件的 widget。
- `pEvent`：输入事件。
- `pUser`：按钮对象指针。

**返回值：**

返回 `xgeXuiButtonEvent` 的处理结果。

**资源归属：**

不接管任何参数。

**补充说明：**

- 当前实现不直接使用 `pWidget`，按钮对象会通过自身保存的 `pWidget` 工作。
- 普通用户通常不需要手动调用该函数。

**范例代码：**

```c
widget->procEvent = xgeXuiButtonEventProc;
widget->pUser = &button;
```

**相关 API：**

- `xgeXuiButtonInit`
- `xgeXuiWidgetSetCaptureEvent`

---

### xgeXuiButtonPaintProc

绘制按钮控件。

**功能：**

根据按钮当前状态绘制背景，并在内容区域中绘制按钮文本。

**函数原型：**

```c
XGE_API void xgeXuiButtonPaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载按钮的 widget。
- `pUser`：按钮对象指针。

**返回值：**

无。

**资源归属：**

不接管 widget、按钮、字体或文本资源。

**补充说明：**

- 背景颜色 alpha 为 0 时不会绘制背景。
- 如果字体为空、文本为空或文本首字符为 0，不绘制文字。
- 绘制通过当前 XUI host 完成。

**范例代码：**

```c
xgeXuiButtonPaintProc(button.pWidget, &button);
```

**相关 API：**

- `xgeXuiButtonSetColors`
- `xgeXuiButtonSetText`

---

## API 参考：Label / Image / Input

### xgeXuiLabelInit

初始化文本标签控件。

**功能：**

把 `xge_xui_label_t` 绑定到 widget，设置字体、文本、默认颜色和文本测量/绘制回调。

**函数原型：**

```c
XGE_API int xgeXuiLabelInit(xge_xui_label pLabel, xge_xui_widget pWidget, xge_font pFont, const char* sText);
```

**参数：**

- `pLabel`：标签对象。
- `pWidget`：承载标签的 widget。
- `pFont`：用于测量和绘制的字体，可以为 `NULL`。
- `sText`：标签文本；传入 `NULL` 等同于空字符串。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

标签对象、widget、字体和文本字符串都由调用者管理。标签保存 `sText` 的借用指针。

**补充说明：**

- 初始化会安装 `xgeXuiLabelMeasureProc` 和 `xgeXuiLabelPaintProc`。
- 默认颜色为白色，默认文本标志为左上对齐并裁剪。

**范例代码：**

```c
xge_xui_label_t label;
xgeXuiLabelInit(&label, widget, font, "Score");
```

**相关 API：**

- `xgeXuiLabelUnit`
- `xgeXuiLabelSetText`

---

### xgeXuiLabelUnit

释放标签控件绑定。

**功能：**

解除标签对 widget 测量和绘制回调的占用，并清空标签对象。

**函数原型：**

```c
XGE_API void xgeXuiLabelUnit(xge_xui_label pLabel);
```

**参数：**

- `pLabel`：标签对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 widget、字体或文本字符串。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该标签时才会清理回调。

**范例代码：**

```c
xgeXuiLabelUnit(&label);
```

**相关 API：**

- `xgeXuiLabelInit`

---

### xgeXuiLabelSetText

设置标签文本。

**功能：**

替换标签显示文本，重新测量文本尺寸，并标记布局和绘制 dirty。

**函数原型：**

```c
XGE_API void xgeXuiLabelSetText(xge_xui_label pLabel, const char* sText);
```

**参数：**

- `pLabel`：标签对象，可以为 `NULL`。
- `sText`：新文本；传入 `NULL` 等同于空字符串。

**返回值：**

无。

**资源归属：**

标签保存文本借用指针，不复制字符串。

**补充说明：**

- 文本生命周期必须覆盖标签绘制期。

**范例代码：**

```c
xgeXuiLabelSetText(&label, "Ready");
```

**相关 API：**

- `xgeXuiLabelMeasure`
- `xgeXuiWidgetMarkLayout`

---

### xgeXuiLabelSetFont

设置标签字体。

**功能：**

替换标签字体，重新测量文本尺寸，并标记布局和绘制 dirty。

**函数原型：**

```c
XGE_API void xgeXuiLabelSetFont(xge_xui_label pLabel, xge_font pFont);
```

**参数：**

- `pLabel`：标签对象，可以为 `NULL`。
- `pFont`：新字体，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

字体资源由调用者或资源系统管理，标签只保存指针。

**补充说明：**

- 字体为空时测量结果为 0，绘制阶段不会输出文本。

**范例代码：**

```c
xgeXuiLabelSetFont(&label, font);
```

**相关 API：**

- `xgeXuiLabelSetText`
- `xgeXuiLabelMeasure`

---

### xgeXuiLabelSetColor

设置标签文本颜色。

**功能：**

修改标签文本颜色，并标记 widget 需要重绘。

**函数原型：**

```c
XGE_API void xgeXuiLabelSetColor(xge_xui_label pLabel, uint32_t iColor);
```

**参数：**

- `pLabel`：标签对象，可以为 `NULL`。
- `iColor`：文本颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 颜色 alpha 为 0 时文本由后端按透明色处理。

**范例代码：**

```c
xgeXuiLabelSetColor(&label, XGE_COLOR_RGBA(220, 220, 220, 255));
```

**相关 API：**

- `xgeXuiLabelSetAlign`

---

### xgeXuiLabelSetAlign

设置标签文本对齐方式。

**功能：**

设置标签绘制时传给文本绘制接口的 flags，并强制附加 `XGE_TEXT_CLIP`。

**函数原型：**

```c
XGE_API void xgeXuiLabelSetAlign(xge_xui_label pLabel, uint32_t iTextFlags);
```

**参数：**

- `pLabel`：标签对象，可以为 `NULL`。
- `iTextFlags`：文本对齐和布局标志。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 当前实现会始终裁剪到 widget content rect。

**范例代码：**

```c
xgeXuiLabelSetAlign(&label, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE);
```

**相关 API：**

- `xgeXuiLabelPaintProc`

---

### xgeXuiLabelMeasure

测量标签文本尺寸。

**功能：**

通过 XUI host 测量标签当前字体和文本的尺寸。

**函数原型：**

```c
XGE_API xge_vec2_t xgeXuiLabelMeasure(xge_xui_label pLabel);
```

**参数：**

- `pLabel`：标签对象。

**返回值：**

- 返回文本尺寸。
- 标签、字体或文本为空时返回 `{0, 0}`。

**资源归属：**

不接管外部资源。

**补充说明：**

- 测量能力依赖当前 XUI host 的 `measure_text` 实现。

**范例代码：**

```c
xge_vec2_t size = xgeXuiLabelMeasure(&label);
```

**相关 API：**

- `xgeXuiLabelMeasureProc`

---

### xgeXuiLabelMeasureProc

标签 widget 测量适配器。

**功能：**

把 widget 测量回调转发到 `xgeXuiLabelMeasure`。

**函数原型：**

```c
XGE_API xge_vec2_t xgeXuiLabelMeasureProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：触发测量的 widget。
- `pUser`：标签对象指针。

**返回值：**

返回标签测量尺寸。

**资源归属：**

不接管任何参数。

**补充说明：**

- 当前实现不直接使用 `pWidget`。
- 普通用户通常不需要手动调用。

**范例代码：**

```c
widget->procMeasure = xgeXuiLabelMeasureProc;
```

**相关 API：**

- `xgeXuiLabelInit`

---

### xgeXuiLabelPaintProc

绘制标签控件。

**功能：**

把标签文本绘制到 widget content rect 中。

**函数原型：**

```c
XGE_API void xgeXuiLabelPaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载标签的 widget。
- `pUser`：标签对象指针。

**返回值：**

无。

**资源归属：**

不接管 widget、字体或文本资源。

**补充说明：**

- widget、标签、字体或文本为空时直接返回。
- 绘制通过当前 XUI host 完成。

**范例代码：**

```c
xgeXuiLabelPaintProc(widget, &label);
```

**相关 API：**

- `xgeXuiLabelSetColor`
- `xgeXuiLabelSetAlign`

---

### xgeXuiImageInit

初始化图片控件。

**功能：**

把 `xge_xui_image_t` 绑定到 widget，设置纹理、默认白色 tint 和默认拉伸模式。

**函数原型：**

```c
XGE_API int xgeXuiImageInit(xge_xui_image pImage, xge_xui_widget pWidget, xge_texture pTexture);
```

**参数：**

- `pImage`：图片控件对象。
- `pWidget`：承载图片的 widget。
- `pTexture`：要显示的纹理，可以为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

图片控件不接管纹理生命周期，只保存纹理指针。

**补充说明：**

- 初始化会安装图片测量和绘制回调。
- 默认模式为 `XGE_XUI_IMAGE_STRETCH`。

**范例代码：**

```c
xge_xui_image_t image;
xgeXuiImageInit(&image, widget, texture);
```

**相关 API：**

- `xgeXuiImageUnit`
- `xgeXuiImageSetTexture`

---

### xgeXuiImageUnit

释放图片控件绑定。

**功能：**

解除图片控件对 widget 测量和绘制回调的占用，并清空图片对象。

**函数原型：**

```c
XGE_API void xgeXuiImageUnit(xge_xui_image pImage);
```

**参数：**

- `pImage`：图片控件对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 widget 或纹理资源。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该图片对象时才会清理回调。

**范例代码：**

```c
xgeXuiImageUnit(&image);
```

**相关 API：**

- `xgeXuiImageInit`

---

### xgeXuiImageSetTexture

设置图片纹理。

**功能：**

替换图片控件显示的纹理，并标记布局和绘制 dirty。

**函数原型：**

```c
XGE_API void xgeXuiImageSetTexture(xge_xui_image pImage, xge_texture pTexture);
```

**参数：**

- `pImage`：图片控件对象，可以为 `NULL`。
- `pTexture`：新纹理，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

图片控件不持有纹理所有权。

**补充说明：**

- 没有 source rect 时，测量尺寸来自纹理宽高。

**范例代码：**

```c
xgeXuiImageSetTexture(&image, texture);
```

**相关 API：**

- `xgeXuiImageSetSource`

---

### xgeXuiImageSetSource

设置图片源区域。

**功能：**

设置纹理源矩形，并标记布局和绘制 dirty。

**函数原型：**

```c
XGE_API void xgeXuiImageSetSource(xge_xui_image pImage, xge_rect_t tSrc);
```

**参数：**

- `pImage`：图片控件对象，可以为 `NULL`。
- `tSrc`：纹理源区域。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 当 `tSrc.fW` 和 `tSrc.fH` 大于 0 时，测量尺寸优先使用源区域尺寸。

**范例代码：**

```c
xge_rect_t src = {0.0f, 0.0f, 64.0f, 64.0f};
xgeXuiImageSetSource(&image, src);
```

**相关 API：**

- `xgeXuiImageMeasureProc`

---

### xgeXuiImageSetColor

设置图片 tint 颜色。

**功能：**

设置图片绘制颜色，并标记 widget 重绘。

**函数原型：**

```c
XGE_API void xgeXuiImageSetColor(xge_xui_image pImage, uint32_t iColor);
```

**参数：**

- `pImage`：图片控件对象，可以为 `NULL`。
- `iColor`：绘制颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- alpha 为 0 时图片不会绘制。

**范例代码：**

```c
xgeXuiImageSetColor(&image, XGE_COLOR_RGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeXuiImagePaintProc`

---

### xgeXuiImageSetMode

设置图片布局模式。

**功能：**

设置图片在 widget content rect 中的显示方式，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiImageSetMode(xge_xui_image pImage, int iMode);
```

**参数：**

- `pImage`：图片控件对象，可以为 `NULL`。
- `iMode`：图片模式，例如 stretch、center、fit。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- `XGE_XUI_IMAGE_CENTER` 使用源尺寸居中显示。
- `XGE_XUI_IMAGE_FIT` 保持比例适配 content rect。
- 其他模式按默认拉伸到 content rect 处理。

**范例代码：**

```c
xgeXuiImageSetMode(&image, XGE_XUI_IMAGE_FIT);
```

**相关 API：**

- `xgeXuiImagePaintProc`

---

### xgeXuiImageMeasureProc

图片 widget 测量适配器。

**功能：**

返回图片控件的期望尺寸，优先使用 source rect，其次使用纹理尺寸。

**函数原型：**

```c
XGE_API xge_vec2_t xgeXuiImageMeasureProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：触发测量的 widget。
- `pUser`：图片控件对象指针。

**返回值：**

- 返回图片期望尺寸。
- 图片对象或纹理为空且没有 source rect 时返回 `{0, 0}`。

**资源归属：**

不接管任何参数。

**补充说明：**

- 当前实现不直接使用 `pWidget`。

**范例代码：**

```c
xge_vec2_t size = xgeXuiImageMeasureProc(widget, &image);
```

**相关 API：**

- `xgeXuiImageSetSource`
- `xgeXuiImageSetTexture`

---

### xgeXuiImagePaintProc

绘制图片控件。

**功能：**

根据图片模式计算目标矩形，并通过 XUI host 绘制纹理。

**函数原型：**

```c
XGE_API void xgeXuiImagePaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载图片的 widget。
- `pUser`：图片控件对象指针。

**返回值：**

无。

**资源归属：**

不接管 widget 或纹理资源。

**补充说明：**

- widget、图片、纹理为空或 tint alpha 为 0 时直接返回。
- 绘制命令使用 `XGE_DRAW_SCREEN_SPACE`。

**范例代码：**

```c
xgeXuiImagePaintProc(widget, &image);
```

**相关 API：**

- `xgeXuiImageSetMode`
- `xgeXuiImageSetColor`

---

### xgeXuiInputInit

初始化单行输入框控件。

**功能：**

初始化输入框内部文本缓冲，绑定 widget，安装事件/绘制回调，并从主题中读取默认字体和颜色。

**函数原型：**

```c
XGE_API int xgeXuiInputInit(xge_xui_input pInput, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont);
```

**参数：**

- `pInput`：输入框对象。
- `pContext`：所属 XUI context。
- `pWidget`：承载输入框的 widget。
- `pFont`：输入框字体；传入 `NULL` 时使用主题字体。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 文本缓冲初始化失败返回对应错误码。

**资源归属：**

输入框内部持有 `xge_xui_text_t` 的缓冲资源，必须用 `xgeXuiInputUnit` 释放。

**补充说明：**

- 初始化会让 widget 可聚焦并启用 clip。
- 输入框依赖系统 IME 事件，不内建完整 IME。

**范例代码：**

```c
xge_xui_input_t input;
xgeXuiInputInit(&input, &ui, widget, font);
```

**相关 API：**

- `xgeXuiInputUnit`
- `xgeXuiTextInit`

---

### xgeXuiInputUnit

释放输入框控件。

**功能：**

解除输入框对 widget 事件和绘制回调的占用，释放内部文本缓冲，并清空输入框对象。

**函数原型：**

```c
XGE_API void xgeXuiInputUnit(xge_xui_input pInput);
```

**参数：**

- `pInput`：输入框对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放内部文本缓冲；不释放 widget、字体或输入框结构本身。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该输入框时才会清理回调。

**范例代码：**

```c
xgeXuiInputUnit(&input);
```

**相关 API：**

- `xgeXuiInputInit`

---

### xgeXuiInputSetText

设置输入框文本。

**功能：**

用新文本替换输入框内容，并标记布局和绘制 dirty。

**函数原型：**

```c
XGE_API void xgeXuiInputSetText(xge_xui_input pInput, const char* sText);
```

**参数：**

- `pInput`：输入框对象，可以为 `NULL`。
- `sText`：新文本；传入 `NULL` 等同于空字符串。

**返回值：**

无。

**资源归属：**

输入框会复制文本到内部缓冲，不持有 `sText` 指针。

**补充说明：**

- 未初始化输入框会直接返回。

**范例代码：**

```c
xgeXuiInputSetText(&input, "guest");
```

**相关 API：**

- `xgeXuiInputGetText`
- `xgeXuiTextSet`

---

### xgeXuiInputGetText

获取输入框文本。

**功能：**

返回输入框当前 UTF-8 文本内容。

**函数原型：**

```c
XGE_API const char* xgeXuiInputGetText(xge_xui_input pInput);
```

**参数：**

- `pInput`：输入框对象。

**返回值：**

- 返回输入框内部文本指针。
- 输入框为空或文本未初始化时返回空字符串。

**资源归属：**

返回值是借用指针，调用者不能释放，也不应长期保存。

**补充说明：**

- 后续修改输入框文本可能使返回指针失效。

**范例代码：**

```c
const char* value = xgeXuiInputGetText(&input);
```

**相关 API：**

- `xgeXuiInputSetText`

---

### xgeXuiInputSetFont

设置输入框字体。

**功能：**

替换输入框字体，并标记布局和绘制 dirty。

**函数原型：**

```c
XGE_API void xgeXuiInputSetFont(xge_xui_input pInput, xge_font pFont);
```

**参数：**

- `pInput`：输入框对象，可以为 `NULL`。
- `pFont`：新字体，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

字体由调用者或资源系统管理，输入框只保存指针。

**补充说明：**

- 候选框位置和游标位置依赖字体测量结果。

**范例代码：**

```c
xgeXuiInputSetFont(&input, font);
```

**相关 API：**

- `xgeXuiInputGetCandidateRect`

---

### xgeXuiInputSetColors

设置输入框颜色。

**功能：**

设置输入框文本、背景、焦点背景和游标颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiInputSetColors(xge_xui_input pInput, uint32_t iText, uint32_t iBackground, uint32_t iFocus, uint32_t iCursor);
```

**参数：**

- `pInput`：输入框对象，可以为 `NULL`。
- `iText`：文本颜色。
- `iBackground`：普通背景颜色。
- `iFocus`：获得焦点时的背景颜色。
- `iCursor`：游标颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- selection 颜色来自主题初始化值，当前函数不修改 selection 颜色。

**范例代码：**

```c
xgeXuiInputSetColors(&input, textColor, bgColor, focusColor, cursorColor);
```

**相关 API：**

- `xgeXuiInputPaintProc`

---

### xgeXuiInputSetSelection

设置输入框选择区。

**功能：**

设置输入框内部文本选择区，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiInputSetSelection(xge_xui_input pInput, int iStart, int iEnd);
```

**参数：**

- `pInput`：输入框对象，可以为 `NULL`。
- `iStart`：选择起点 UTF-8 字节偏移。
- `iEnd`：选择终点 UTF-8 字节偏移。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 未初始化输入框会直接返回。
- 偏移会被底层文本对象夹到合法 UTF-8 边界。

**范例代码：**

```c
xgeXuiInputSetSelection(&input, 0, 4);
```

**相关 API：**

- `xgeXuiInputGetSelection`
- `xgeXuiTextSetSelection`

---

### xgeXuiInputGetSelection

获取输入框选择区。

**功能：**

读取输入框当前选择区，输出 UTF-8 字节偏移范围。

**函数原型：**

```c
XGE_API void xgeXuiInputGetSelection(xge_xui_input pInput, int* pStart, int* pEnd);
```

**参数：**

- `pInput`：输入框对象，可以为 `NULL`。
- `pStart`：接收选择起点，可为 `NULL`。
- `pEnd`：接收选择终点，可为 `NULL`。

**返回值：**

无。

**资源归属：**

输出参数由调用者持有。

**补充说明：**

- 输入框为空时输出 0 到 0。

**范例代码：**

```c
int start;
int end;
xgeXuiInputGetSelection(&input, &start, &end);
```

**相关 API：**

- `xgeXuiInputSetSelection`

---

### xgeXuiInputGetCandidateRect

获取 IME 候选框参考区域。

**功能：**

根据输入框内容、字体测量和当前游标位置，计算系统 IME 候选框应锚定的矩形。

**函数原型：**

```c
XGE_API xge_rect_t xgeXuiInputGetCandidateRect(xge_xui_input pInput);
```

**参数：**

- `pInput`：输入框对象。

**返回值：**

- 返回候选框参考矩形。
- 输入框或 widget 为空时返回全 0 矩形。

**资源归属：**

不接管外部资源。

**补充说明：**

- 返回矩形宽度固定为 1，高度为 widget content 高度。
- 平台后端可把该矩形传给系统 IME。

**范例代码：**

```c
xge_rect_t imeRect = xgeXuiInputGetCandidateRect(&input);
```

**相关 API：**

- `xgeXuiTextGetCursor`
- `xgeXuiInputEvent`

---

### xgeXuiInputEvent

处理输入框事件。

**功能：**

处理鼠标、触控、文本、IME 和键盘事件，维护焦点、选择区、游标和文本内容。

**函数原型：**

```c
XGE_API int xgeXuiInputEvent(xge_xui_input pInput, const xge_event_t* pEvent);
```

**参数：**

- `pInput`：输入框对象。
- `pEvent`：输入事件。

**返回值：**

- 已处理事件返回 `XGE_XUI_EVENT_CONSUMED`。
- 不相关事件返回 `XGE_XUI_EVENT_CONTINUE`。

**资源归属：**

不接管事件对象。

**补充说明：**

- 鼠标/触控按下会设置焦点、计算游标并启动选择。
- 文本和 IME 事件只在输入框获得焦点后处理。
- 支持 Backspace、Delete、Left、Right 的基础编辑行为。

**范例代码：**

```c
int handled = xgeXuiInputEvent(&input, event);
```

**相关 API：**

- `xgeXuiTextInputEvent`
- `xgeXuiInputGetCandidateRect`

---

### xgeXuiInputEventProc

输入框 widget 事件适配器。

**功能：**

把 widget 事件回调转发给 `xgeXuiInputEvent`。

**函数原型：**

```c
XGE_API int xgeXuiInputEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
```

**参数：**

- `pWidget`：触发事件的 widget。
- `pEvent`：输入事件。
- `pUser`：输入框对象指针。

**返回值：**

返回 `xgeXuiInputEvent` 的处理结果。

**资源归属：**

不接管任何参数。

**补充说明：**

- 当前实现不直接使用 `pWidget`。
- 普通用户通常不需要手动调用。

**范例代码：**

```c
widget->procEvent = xgeXuiInputEventProc;
```

**相关 API：**

- `xgeXuiInputInit`

---

### xgeXuiInputPaintProc

绘制输入框控件。

**功能：**

绘制输入框背景、选择区、文本、IME composition 和焦点游标。

**函数原型：**

```c
XGE_API void xgeXuiInputPaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载输入框的 widget。
- `pUser`：输入框对象指针。

**返回值：**

无。

**资源归属：**

不接管 widget、输入框或字体资源。

**补充说明：**

- 背景颜色根据输入框是否获得焦点切换。
- composition 会绘制在候选框参考位置之后。
- 游标只在输入框获得焦点且游标颜色非透明时绘制。

**范例代码：**

```c
xgeXuiInputPaintProc(widget, &input);
```

**相关 API：**

- `xgeXuiInputSetColors`
- `xgeXuiInputGetCandidateRect`

---

## API 参考：Toggle / Slider / Progress

### xgeXuiToggleInit

初始化开关控件。

**功能：**

把 `xge_xui_toggle_t` 绑定到 widget，安装事件和绘制回调，并从主题中读取默认字体、状态颜色和选中颜色。

**函数原型：**

```c
XGE_API int xgeXuiToggleInit(xge_xui_toggle pToggle, xge_xui_context pContext, xge_xui_widget pWidget);
```

**参数：**

- `pToggle`：开关控件对象。
- `pContext`：所属 XUI context。
- `pWidget`：承载开关的 widget。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

控件对象、context 和 widget 均由调用者持有，XUI 不接管其生命周期。

**补充说明：**

- 初始化会让 widget 可聚焦。
- 初始化会安装 `xgeXuiToggleEventProc` 和 `xgeXuiTogglePaintProc`。

**范例代码：**

```c
xge_xui_toggle_t toggle;
xgeXuiToggleInit(&toggle, &ui, widget);
```

**相关 API：**

- `xgeXuiToggleUnit`
- `xgeXuiToggleSetChange`

---

### xgeXuiToggleUnit

释放开关控件绑定。

**功能：**

解除开关对 widget 事件和绘制回调的占用，并清空控件对象。

**函数原型：**

```c
XGE_API void xgeXuiToggleUnit(xge_xui_toggle pToggle);
```

**参数：**

- `pToggle`：开关控件对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 widget、字体或文本字符串。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该控件时才会清理回调。

**范例代码：**

```c
xgeXuiToggleUnit(&toggle);
```

**相关 API：**

- `xgeXuiToggleInit`

---

### xgeXuiToggleSetChange

设置开关变更回调。

**功能：**

注册开关选中状态变化回调和用户数据。

**函数原型：**

```c
XGE_API void xgeXuiToggleSetChange(xge_xui_toggle pToggle, xge_xui_toggle_proc procChange, void* pUser);
```

**参数：**

- `pToggle`：开关控件对象，可以为 `NULL`。
- `procChange`：状态变化回调，可以为 `NULL`。
- `pUser`：传给回调的用户数据。

**返回值：**

无。

**资源归属：**

`pUser` 由调用者管理，XUI 只保存并回传指针。

**补充说明：**

- 用户点击并成功切换状态时会触发该回调。

**范例代码：**

```c
xgeXuiToggleSetChange(&toggle, on_toggle, user);
```

**相关 API：**

- `xgeXuiToggleEvent`

---

### xgeXuiToggleSetText

设置开关文本。

**功能：**

设置开关显示文本和字体，并标记布局和绘制 dirty。

**函数原型：**

```c
XGE_API void xgeXuiToggleSetText(xge_xui_toggle pToggle, xge_font pFont, const char* sText);
```

**参数：**

- `pToggle`：开关控件对象，可以为 `NULL`。
- `pFont`：文本字体，可以为 `NULL`。
- `sText`：显示文本；传入 `NULL` 等同于空字符串。

**返回值：**

无。

**资源归属：**

控件保存 `sText` 的借用指针，不复制字符串。

**补充说明：**

- 文本绘制在勾选框右侧。

**范例代码：**

```c
xgeXuiToggleSetText(&toggle, font, "Enable sound");
```

**相关 API：**

- `xgeXuiToggleSetTextColor`

---

### xgeXuiToggleSetChecked

设置开关选中状态。

**功能：**

直接设置开关是否选中，并在状态变化时标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiToggleSetChecked(xge_xui_toggle pToggle, int bChecked);
```

**参数：**

- `pToggle`：开关控件对象，可以为 `NULL`。
- `bChecked`：非 0 表示选中，0 表示未选中。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 该函数不会主动触发 change 回调。

**范例代码：**

```c
xgeXuiToggleSetChecked(&toggle, 1);
```

**相关 API：**

- `xgeXuiToggleGetChecked`

---

### xgeXuiToggleGetChecked

获取开关选中状态。

**功能：**

返回开关当前是否选中。

**函数原型：**

```c
XGE_API int xgeXuiToggleGetChecked(xge_xui_toggle pToggle);
```

**参数：**

- `pToggle`：开关控件对象。

**返回值：**

- 选中返回 1，未选中返回 0。
- `pToggle` 为 `NULL` 时返回 0。

**资源归属：**

不接管外部资源。

**补充说明：**

- 返回值已经归一化为 0 或 1。

**范例代码：**

```c
int checked = xgeXuiToggleGetChecked(&toggle);
```

**相关 API：**

- `xgeXuiToggleSetChecked`

---

### xgeXuiToggleSetTextColor

设置开关文本颜色。

**功能：**

设置开关文本绘制颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiToggleSetTextColor(xge_xui_toggle pToggle, uint32_t iColor);
```

**参数：**

- `pToggle`：开关控件对象，可以为 `NULL`。
- `iColor`：文本颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 文本颜色只影响文字，不影响勾选框和背景。

**范例代码：**

```c
xgeXuiToggleSetTextColor(&toggle, XGE_COLOR_RGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeXuiToggleSetColors`

---

### xgeXuiToggleSetColors

设置开关状态颜色。

**功能：**

设置开关 normal、hover、active、focus、disabled 和 checked 状态颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiToggleSetColors(xge_xui_toggle pToggle, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iChecked);
```

**参数：**

- `pToggle`：开关控件对象，可以为 `NULL`。
- `iNormal`：普通背景颜色。
- `iHover`：悬停背景颜色。
- `iActive`：按下背景颜色。
- `iFocus`：焦点背景颜色。
- `iDisabled`：禁用背景颜色。
- `iChecked`：选中框颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- checked 颜色用于选中方框，背景颜色仍按交互状态决定。

**范例代码：**

```c
xgeXuiToggleSetColors(&toggle, normal, hover, active, focus, disabled, checked);
```

**相关 API：**

- `xgeXuiToggleGetState`

---

### xgeXuiToggleGetState

获取开关交互状态。

**功能：**

返回开关当前交互状态，并根据 widget 可见和启用状态刷新 disabled 标记。

**函数原型：**

```c
XGE_API int xgeXuiToggleGetState(xge_xui_toggle pToggle);
```

**参数：**

- `pToggle`：开关控件对象。

**返回值：**

- 返回 `XGE_XUI_STATE_*` 状态位组合。
- `pToggle` 为 `NULL` 时返回 `XGE_XUI_STATE_DISABLED`。

**资源归属：**

不接管外部资源。

**补充说明：**

- 选中状态不属于 state 位，需用 `xgeXuiToggleGetChecked` 查询。

**范例代码：**

```c
int state = xgeXuiToggleGetState(&toggle);
```

**相关 API：**

- `xgeXuiToggleGetChecked`

---

### xgeXuiToggleEvent

处理开关事件。

**功能：**

处理鼠标和触控事件，维护 hover/active 状态、焦点、捕获，并在释放时切换选中状态。

**函数原型：**

```c
XGE_API int xgeXuiToggleEvent(xge_xui_toggle pToggle, const xge_event_t* pEvent);
```

**参数：**

- `pToggle`：开关控件对象。
- `pEvent`：输入事件。

**返回值：**

- 已处理事件返回 `XGE_XUI_EVENT_CONSUMED`。
- 不相关事件返回 `XGE_XUI_EVENT_CONTINUE`。

**资源归属：**

不接管事件对象。

**补充说明：**

- 鼠标或触控释放时，如果之前处于 active 且仍在控件内，会切换 checked 并触发 change 回调。

**范例代码：**

```c
return xgeXuiToggleEvent(&toggle, event);
```

**相关 API：**

- `xgeXuiToggleSetChange`
- `xgeXuiSetCapture`

---

### xgeXuiToggleEventProc

开关 widget 事件适配器。

**功能：**

把 widget 事件回调转发给 `xgeXuiToggleEvent`。

**函数原型：**

```c
XGE_API int xgeXuiToggleEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
```

**参数：**

- `pWidget`：触发事件的 widget。
- `pEvent`：输入事件。
- `pUser`：开关控件对象指针。

**返回值：**

返回 `xgeXuiToggleEvent` 的处理结果。

**资源归属：**

不接管任何参数。

**补充说明：**

- 普通用户通常不需要手动调用该函数。

**范例代码：**

```c
widget->procEvent = xgeXuiToggleEventProc;
```

**相关 API：**

- `xgeXuiToggleInit`

---

### xgeXuiTogglePaintProc

绘制开关控件。

**功能：**

绘制开关背景、勾选框、选中标记和右侧文本。

**函数原型：**

```c
XGE_API void xgeXuiTogglePaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载开关的 widget。
- `pUser`：开关控件对象指针。

**返回值：**

无。

**资源归属：**

不接管 widget、字体或文本资源。

**补充说明：**

- 勾选框大小取 content 高度并限制到最多 18 像素。

**范例代码：**

```c
xgeXuiTogglePaintProc(widget, &toggle);
```

**相关 API：**

- `xgeXuiToggleSetColors`
- `xgeXuiToggleSetText`

---

### xgeXuiSliderInit

初始化滑块控件。

**功能：**

把 `xge_xui_slider_t` 绑定到 widget，设置默认范围、颜色、事件回调和绘制回调。

**函数原型：**

```c
XGE_API int xgeXuiSliderInit(xge_xui_slider pSlider, xge_xui_context pContext, xge_xui_widget pWidget);
```

**参数：**

- `pSlider`：滑块控件对象。
- `pContext`：所属 XUI context。
- `pWidget`：承载滑块的 widget。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

控件对象、context 和 widget 均由调用者持有。

**补充说明：**

- 默认范围为 0 到 1，默认值为 0。
- 初始化会让 widget 可聚焦。

**范例代码：**

```c
xge_xui_slider_t slider;
xgeXuiSliderInit(&slider, &ui, widget);
```

**相关 API：**

- `xgeXuiSliderUnit`
- `xgeXuiSliderSetRange`

---

### xgeXuiSliderUnit

释放滑块控件绑定。

**功能：**

解除滑块对 widget 事件和绘制回调的占用，并清空控件对象。

**函数原型：**

```c
XGE_API void xgeXuiSliderUnit(xge_xui_slider pSlider);
```

**参数：**

- `pSlider`：滑块控件对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 widget 或外部资源。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该控件时才会清理回调。

**范例代码：**

```c
xgeXuiSliderUnit(&slider);
```

**相关 API：**

- `xgeXuiSliderInit`

---

### xgeXuiSliderSetChange

设置滑块变化回调。

**功能：**

注册滑块值变化回调和用户数据。

**函数原型：**

```c
XGE_API void xgeXuiSliderSetChange(xge_xui_slider pSlider, xge_xui_slider_proc procChange, void* pUser);
```

**参数：**

- `pSlider`：滑块控件对象，可以为 `NULL`。
- `procChange`：值变化回调，可以为 `NULL`。
- `pUser`：传给回调的用户数据。

**返回值：**

无。

**资源归属：**

`pUser` 由调用者管理，XUI 只保存并回传指针。

**补充说明：**

- 拖动或点击导致值变化时会触发回调。

**范例代码：**

```c
xgeXuiSliderSetChange(&slider, on_slider, user);
```

**相关 API：**

- `xgeXuiSliderEvent`

---

### xgeXuiSliderSetRange

设置滑块范围。

**功能：**

设置滑块最小值和最大值，并把当前值夹到新范围内。

**函数原型：**

```c
XGE_API void xgeXuiSliderSetRange(xge_xui_slider pSlider, float fMin, float fMax);
```

**参数：**

- `pSlider`：滑块控件对象，可以为 `NULL`。
- `fMin`：范围最小值。
- `fMax`：范围最大值。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 如果 `fMax < fMin` 会自动交换。
- 如果两者相等，最大值会调整为 `fMin + 1.0f`。

**范例代码：**

```c
xgeXuiSliderSetRange(&slider, 0.0f, 100.0f);
```

**相关 API：**

- `xgeXuiSliderSetValue`

---

### xgeXuiSliderSetValue

设置滑块当前值。

**功能：**

设置滑块值，并自动夹到当前范围内。

**函数原型：**

```c
XGE_API void xgeXuiSliderSetValue(xge_xui_slider pSlider, float fValue);
```

**参数：**

- `pSlider`：滑块控件对象，可以为 `NULL`。
- `fValue`：目标值。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 程序主动设置值不会触发 change 回调。

**范例代码：**

```c
xgeXuiSliderSetValue(&slider, 50.0f);
```

**相关 API：**

- `xgeXuiSliderGetValue`

---

### xgeXuiSliderGetValue

获取滑块当前值。

**功能：**

返回滑块当前数值。

**函数原型：**

```c
XGE_API float xgeXuiSliderGetValue(xge_xui_slider pSlider);
```

**参数：**

- `pSlider`：滑块控件对象。

**返回值：**

- 返回当前值。
- `pSlider` 为 `NULL` 时返回 0。

**资源归属：**

不接管外部资源。

**补充说明：**

- 返回值位于当前范围内。

**范例代码：**

```c
float value = xgeXuiSliderGetValue(&slider);
```

**相关 API：**

- `xgeXuiSliderSetValue`

---

### xgeXuiSliderSetColors

设置滑块颜色。

**功能：**

设置轨道、填充、滑块、焦点和禁用颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiSliderSetColors(xge_xui_slider pSlider, uint32_t iTrack, uint32_t iFill, uint32_t iKnob, uint32_t iFocus, uint32_t iDisabled);
```

**参数：**

- `pSlider`：滑块控件对象，可以为 `NULL`。
- `iTrack`：轨道颜色。
- `iFill`：已填充轨道颜色。
- `iKnob`：滑块颜色。
- `iFocus`：焦点背景颜色。
- `iDisabled`：禁用轨道颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 禁用状态下只绘制禁用轨道。

**范例代码：**

```c
xgeXuiSliderSetColors(&slider, track, fill, knob, focus, disabled);
```

**相关 API：**

- `xgeXuiSliderPaintProc`

---

### xgeXuiSliderGetState

获取滑块交互状态。

**功能：**

返回滑块当前交互状态，并根据 widget 可见和启用状态刷新 disabled 标记。

**函数原型：**

```c
XGE_API int xgeXuiSliderGetState(xge_xui_slider pSlider);
```

**参数：**

- `pSlider`：滑块控件对象。

**返回值：**

- 返回 `XGE_XUI_STATE_*` 状态位组合。
- `pSlider` 为 `NULL` 时返回 `XGE_XUI_STATE_DISABLED`。

**资源归属：**

不接管外部资源。

**补充说明：**

- active 状态通常表示正在拖动。

**范例代码：**

```c
int state = xgeXuiSliderGetState(&slider);
```

**相关 API：**

- `xgeXuiSliderEvent`

---

### xgeXuiSliderEvent

处理滑块事件。

**功能：**

处理鼠标和触控事件，维护 hover/active 状态、焦点、捕获，并根据指针位置更新滑块值。

**函数原型：**

```c
XGE_API int xgeXuiSliderEvent(xge_xui_slider pSlider, const xge_event_t* pEvent);
```

**参数：**

- `pSlider`：滑块控件对象。
- `pEvent`：输入事件。

**返回值：**

- 已处理事件返回 `XGE_XUI_EVENT_CONSUMED`。
- 不相关事件返回 `XGE_XUI_EVENT_CONTINUE`。

**资源归属：**

不接管事件对象。

**补充说明：**

- 指针按下和拖动时会根据 X 坐标更新值。
- 用户交互导致值变化时会触发 change 回调。

**范例代码：**

```c
return xgeXuiSliderEvent(&slider, event);
```

**相关 API：**

- `xgeXuiSliderSetChange`
- `xgeXuiSetCapture`

---

### xgeXuiSliderEventProc

滑块 widget 事件适配器。

**功能：**

把 widget 事件回调转发给 `xgeXuiSliderEvent`。

**函数原型：**

```c
XGE_API int xgeXuiSliderEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
```

**参数：**

- `pWidget`：触发事件的 widget。
- `pEvent`：输入事件。
- `pUser`：滑块控件对象指针。

**返回值：**

返回 `xgeXuiSliderEvent` 的处理结果。

**资源归属：**

不接管任何参数。

**补充说明：**

- 普通用户通常不需要手动调用该函数。

**范例代码：**

```c
widget->procEvent = xgeXuiSliderEventProc;
```

**相关 API：**

- `xgeXuiSliderInit`

---

### xgeXuiSliderPaintProc

绘制滑块控件。

**功能：**

绘制滑块轨道、填充段、焦点背景和滑块按钮。

**函数原型：**

```c
XGE_API void xgeXuiSliderPaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载滑块的 widget。
- `pUser`：滑块控件对象指针。

**返回值：**

无。

**资源归属：**

不接管任何参数。

**补充说明：**

- 轨道高度最多 6 像素。
- 滑块尺寸取 content 高度并限制到 6 到 18 像素。

**范例代码：**

```c
xgeXuiSliderPaintProc(widget, &slider);
```

**相关 API：**

- `xgeXuiSliderSetColors`

---

### xgeXuiProgressInit

初始化进度条控件。

**功能：**

把 `xge_xui_progress_t` 绑定到 widget，设置默认范围、颜色、文本标志和绘制回调。

**函数原型：**

```c
XGE_API int xgeXuiProgressInit(xge_xui_progress pProgress, xge_xui_widget pWidget);
```

**参数：**

- `pProgress`：进度条控件对象。
- `pWidget`：承载进度条的 widget。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

控件对象和 widget 均由调用者持有。

**补充说明：**

- 默认范围为 0 到 1。
- 默认文本为空，设置文本后才绘制文字。

**范例代码：**

```c
xge_xui_progress_t progress;
xgeXuiProgressInit(&progress, widget);
```

**相关 API：**

- `xgeXuiProgressUnit`
- `xgeXuiProgressSetValue`

---

### xgeXuiProgressUnit

释放进度条控件绑定。

**功能：**

解除进度条对 widget 绘制回调的占用，并清空控件对象。

**函数原型：**

```c
XGE_API void xgeXuiProgressUnit(xge_xui_progress pProgress);
```

**参数：**

- `pProgress`：进度条控件对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 widget、字体或文本字符串。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该控件时才会清理回调。

**范例代码：**

```c
xgeXuiProgressUnit(&progress);
```

**相关 API：**

- `xgeXuiProgressInit`

---

### xgeXuiProgressSetRange

设置进度条范围。

**功能：**

设置进度条最小值和最大值，并把当前值夹到新范围内。

**函数原型：**

```c
XGE_API void xgeXuiProgressSetRange(xge_xui_progress pProgress, float fMin, float fMax);
```

**参数：**

- `pProgress`：进度条控件对象，可以为 `NULL`。
- `fMin`：范围最小值。
- `fMax`：范围最大值。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 如果 `fMax < fMin` 会自动交换。
- 如果两者相等，最大值会调整为 `fMin + 1.0f`。

**范例代码：**

```c
xgeXuiProgressSetRange(&progress, 0.0f, 100.0f);
```

**相关 API：**

- `xgeXuiProgressSetValue`

---

### xgeXuiProgressSetValue

设置进度条当前值。

**功能：**

设置进度值，并自动夹到当前范围内。

**函数原型：**

```c
XGE_API void xgeXuiProgressSetValue(xge_xui_progress pProgress, float fValue);
```

**参数：**

- `pProgress`：进度条控件对象，可以为 `NULL`。
- `fValue`：目标值。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 值变化后会标记 widget 重绘。

**范例代码：**

```c
xgeXuiProgressSetValue(&progress, 75.0f);
```

**相关 API：**

- `xgeXuiProgressGetValue`

---

### xgeXuiProgressGetValue

获取进度条当前值。

**功能：**

返回进度条当前数值。

**函数原型：**

```c
XGE_API float xgeXuiProgressGetValue(xge_xui_progress pProgress);
```

**参数：**

- `pProgress`：进度条控件对象。

**返回值：**

- 返回当前值。
- `pProgress` 为 `NULL` 时返回 0。

**资源归属：**

不接管外部资源。

**补充说明：**

- 返回值位于当前范围内。

**范例代码：**

```c
float value = xgeXuiProgressGetValue(&progress);
```

**相关 API：**

- `xgeXuiProgressSetValue`

---

### xgeXuiProgressSetText

设置进度条文本。

**功能：**

设置进度条中央显示的文本和字体，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiProgressSetText(xge_xui_progress pProgress, xge_font pFont, const char* sText);
```

**参数：**

- `pProgress`：进度条控件对象，可以为 `NULL`。
- `pFont`：文本字体，可以为 `NULL`。
- `sText`：显示文本；传入 `NULL` 等同于空字符串。

**返回值：**

无。

**资源归属：**

控件保存 `sText` 的借用指针，不复制字符串。

**补充说明：**

- 文本默认居中绘制并裁剪。

**范例代码：**

```c
xgeXuiProgressSetText(&progress, font, "75%");
```

**相关 API：**

- `xgeXuiProgressSetTextColor`

---

### xgeXuiProgressSetTextColor

设置进度条文本颜色。

**功能：**

设置进度条文本绘制颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiProgressSetTextColor(xge_xui_progress pProgress, uint32_t iColor);
```

**参数：**

- `pProgress`：进度条控件对象，可以为 `NULL`。
- `iColor`：文本颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 仅影响进度条文本，不影响轨道和填充颜色。

**范例代码：**

```c
xgeXuiProgressSetTextColor(&progress, XGE_COLOR_RGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeXuiProgressSetText`

---

### xgeXuiProgressSetColors

设置进度条颜色。

**功能：**

设置进度条轨道颜色和填充颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiProgressSetColors(xge_xui_progress pProgress, uint32_t iTrack, uint32_t iFill);
```

**参数：**

- `pProgress`：进度条控件对象，可以为 `NULL`。
- `iTrack`：轨道颜色。
- `iFill`：填充颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- alpha 为 0 的轨道或填充不会绘制。

**范例代码：**

```c
xgeXuiProgressSetColors(&progress, trackColor, fillColor);
```

**相关 API：**

- `xgeXuiProgressPaintProc`

---

### xgeXuiProgressPaintProc

绘制进度条控件。

**功能：**

绘制进度条轨道、按当前进度裁剪的填充区域，以及可选居中文本。

**函数原型：**

```c
XGE_API void xgeXuiProgressPaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载进度条的 widget。
- `pUser`：进度条控件对象指针。

**返回值：**

无。

**资源归属：**

不接管 widget、字体或文本资源。

**补充说明：**

- 填充宽度按 `(value - min) / (max - min)` 计算。

**范例代码：**

```c
xgeXuiProgressPaintProc(widget, &progress);
```

**相关 API：**

- `xgeXuiProgressSetValue`
- `xgeXuiProgressSetColors`

---

## API 参考：Panel / ScrollView / ListView / Dialog

### xgeXuiPanelInit

初始化面板控件。

**功能：**

把 `xge_xui_panel_t` 绑定到 widget，设置默认背景、标题颜色和绘制回调。

**函数原型：**

```c
XGE_API int xgeXuiPanelInit(xge_xui_panel pPanel, xge_xui_widget pWidget);
```

**参数：**

- `pPanel`：面板对象。
- `pWidget`：承载面板的 widget。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

面板对象和 widget 均由调用者持有。

**补充说明：**

- 初始化后标题为空，只绘制背景。
- 面板不主动管理子控件，只提供容器背景和可选标题。

**范例代码：**

```c
xge_xui_panel_t panel;
xgeXuiPanelInit(&panel, widget);
```

**相关 API：**

- `xgeXuiPanelUnit`
- `xgeXuiPanelSetTitle`

---

### xgeXuiPanelUnit

释放面板控件绑定。

**功能：**

解除面板对 widget 绘制回调的占用，并清空面板对象。

**函数原型：**

```c
XGE_API void xgeXuiPanelUnit(xge_xui_panel pPanel);
```

**参数：**

- `pPanel`：面板对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 widget、字体或标题字符串。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该面板时才会清理回调。

**范例代码：**

```c
xgeXuiPanelUnit(&panel);
```

**相关 API：**

- `xgeXuiPanelInit`

---

### xgeXuiPanelSetBackground

设置面板背景颜色。

**功能：**

修改面板背景颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiPanelSetBackground(xge_xui_panel pPanel, uint32_t iColor);
```

**参数：**

- `pPanel`：面板对象，可以为 `NULL`。
- `iColor`：背景颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- alpha 为 0 时不会绘制背景。

**范例代码：**

```c
xgeXuiPanelSetBackground(&panel, XGE_COLOR_RGBA(32, 38, 46, 255));
```

**相关 API：**

- `xgeXuiPanelPaintProc`

---

### xgeXuiPanelSetTitle

设置面板标题。

**功能：**

设置面板标题字体和标题文本，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiPanelSetTitle(xge_xui_panel pPanel, xge_font pFont, const char* sTitle);
```

**参数：**

- `pPanel`：面板对象，可以为 `NULL`。
- `pFont`：标题字体，可以为 `NULL`。
- `sTitle`：标题文本；传入 `NULL` 等同于空字符串。

**返回值：**

无。

**资源归属：**

面板保存标题字符串借用指针，不复制字符串。

**补充说明：**

- 标题绘制在 widget content rect 中。

**范例代码：**

```c
xgeXuiPanelSetTitle(&panel, font, "Settings");
```

**相关 API：**

- `xgeXuiPanelSetTitleColor`
- `xgeXuiPanelSetTitleAlign`

---

### xgeXuiPanelSetTitleColor

设置面板标题颜色。

**功能：**

修改标题文本颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiPanelSetTitleColor(xge_xui_panel pPanel, uint32_t iColor);
```

**参数：**

- `pPanel`：面板对象，可以为 `NULL`。
- `iColor`：标题颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 仅影响标题文本，不影响背景。

**范例代码：**

```c
xgeXuiPanelSetTitleColor(&panel, XGE_COLOR_RGBA(255, 255, 255, 255));
```

**相关 API：**

- `xgeXuiPanelSetTitle`

---

### xgeXuiPanelSetTitleAlign

设置面板标题对齐方式。

**功能：**

设置标题文本绘制 flags，并强制附加 `XGE_TEXT_CLIP`。

**函数原型：**

```c
XGE_API void xgeXuiPanelSetTitleAlign(xge_xui_panel pPanel, uint32_t iTextFlags);
```

**参数：**

- `pPanel`：面板对象，可以为 `NULL`。
- `iTextFlags`：标题文本 flags。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 标题始终裁剪在 content rect 内。

**范例代码：**

```c
xgeXuiPanelSetTitleAlign(&panel, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_TOP);
```

**相关 API：**

- `xgeXuiPanelPaintProc`

---

### xgeXuiPanelSetClip

设置面板裁剪。

**功能：**

设置面板自身的 clip 标记，并同步到承载 widget。

**函数原型：**

```c
XGE_API void xgeXuiPanelSetClip(xge_xui_panel pPanel, int bClip);
```

**参数：**

- `pPanel`：面板对象，可以为 `NULL`。
- `bClip`：非 0 启用裁剪，0 禁用裁剪。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 裁剪行为最终由 widget 的 `XGE_XUI_WIDGET_CLIP` 标志控制。

**范例代码：**

```c
xgeXuiPanelSetClip(&panel, 1);
```

**相关 API：**

- `xgeXuiWidgetSetClip`

---

### xgeXuiPanelPaintProc

绘制面板控件。

**功能：**

绘制面板背景和可选标题文本。

**函数原型：**

```c
XGE_API void xgeXuiPanelPaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载面板的 widget。
- `pUser`：面板对象指针。

**返回值：**

无。

**资源归属：**

不接管 widget、字体或标题字符串。

**补充说明：**

- widget 或面板为空时直接返回。

**范例代码：**

```c
xgeXuiPanelPaintProc(widget, &panel);
```

**相关 API：**

- `xgeXuiPanelSetBackground`
- `xgeXuiPanelSetTitle`

---

### xgeXuiScrollViewInit

初始化滚动视图。

**功能：**

把 `xge_xui_scroll_view_t` 绑定到 widget，设置默认内容尺寸、颜色、事件回调和绘制回调。

**函数原型：**

```c
XGE_API int xgeXuiScrollViewInit(xge_xui_scroll_view pScroll, xge_xui_context pContext, xge_xui_widget pWidget);
```

**参数：**

- `pScroll`：滚动视图对象。
- `pContext`：所属 XUI context。
- `pWidget`：承载滚动视图的 widget。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

滚动视图对象、context 和 widget 均由调用者持有。

**补充说明：**

- 初始化会让 widget 可聚焦并启用 clip。

**范例代码：**

```c
xge_xui_scroll_view_t scroll;
xgeXuiScrollViewInit(&scroll, &ui, widget);
```

**相关 API：**

- `xgeXuiScrollViewUnit`
- `xgeXuiScrollViewSetContentSize`

---

### xgeXuiScrollViewUnit

释放滚动视图绑定。

**功能：**

解除滚动视图对 widget 事件和绘制回调的占用，并清空对象。

**函数原型：**

```c
XGE_API void xgeXuiScrollViewUnit(xge_xui_scroll_view pScroll);
```

**参数：**

- `pScroll`：滚动视图对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 widget 或外部资源。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该对象时才会清理回调。

**范例代码：**

```c
xgeXuiScrollViewUnit(&scroll);
```

**相关 API：**

- `xgeXuiScrollViewInit`

---

### xgeXuiScrollViewSetContentSize

设置滚动内容尺寸。

**功能：**

设置滚动视图的虚拟内容宽高，并夹紧当前滚动偏移。

**函数原型：**

```c
XGE_API void xgeXuiScrollViewSetContentSize(xge_xui_scroll_view pScroll, float fWidth, float fHeight);
```

**参数：**

- `pScroll`：滚动视图对象，可以为 `NULL`。
- `fWidth`：内容宽度。
- `fHeight`：内容高度。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 负数宽高会被夹到 0。

**范例代码：**

```c
xgeXuiScrollViewSetContentSize(&scroll, 800.0f, 1200.0f);
```

**相关 API：**

- `xgeXuiScrollViewSetOffset`

---

### xgeXuiScrollViewSetOffset

设置滚动偏移。

**功能：**

设置滚动视图的水平和垂直偏移，并自动夹到可滚动范围内。

**函数原型：**

```c
XGE_API void xgeXuiScrollViewSetOffset(xge_xui_scroll_view pScroll, float fX, float fY);
```

**参数：**

- `pScroll`：滚动视图对象，可以为 `NULL`。
- `fX`：水平滚动偏移。
- `fY`：垂直滚动偏移。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 偏移变化时会标记重绘。

**范例代码：**

```c
xgeXuiScrollViewSetOffset(&scroll, 0.0f, 120.0f);
```

**相关 API：**

- `xgeXuiScrollViewGetOffset`

---

### xgeXuiScrollViewGetOffset

获取滚动偏移。

**功能：**

读取滚动视图当前水平和垂直偏移。

**函数原型：**

```c
XGE_API void xgeXuiScrollViewGetOffset(xge_xui_scroll_view pScroll, float* pX, float* pY);
```

**参数：**

- `pScroll`：滚动视图对象，可以为 `NULL`。
- `pX`：接收水平偏移，可为 `NULL`。
- `pY`：接收垂直偏移，可为 `NULL`。

**返回值：**

无。

**资源归属：**

输出参数由调用者持有。

**补充说明：**

- `pScroll` 为 `NULL` 时输出 0。

**范例代码：**

```c
float x;
float y;
xgeXuiScrollViewGetOffset(&scroll, &x, &y);
```

**相关 API：**

- `xgeXuiScrollViewSetOffset`

---

### xgeXuiScrollViewSetColors

设置滚动视图颜色。

**功能：**

设置背景、滚动条轨道和滚动条滑块颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiScrollViewSetColors(xge_xui_scroll_view pScroll, uint32_t iBackground, uint32_t iBar, uint32_t iThumb);
```

**参数：**

- `pScroll`：滚动视图对象，可以为 `NULL`。
- `iBackground`：背景颜色。
- `iBar`：滚动条轨道颜色。
- `iThumb`：滚动条滑块颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 只有内容尺寸大于可见尺寸时才绘制对应方向滚动条。

**范例代码：**

```c
xgeXuiScrollViewSetColors(&scroll, bg, bar, thumb);
```

**相关 API：**

- `xgeXuiScrollViewPaintProc`

---

### xgeXuiScrollViewEvent

处理滚动视图事件。

**功能：**

处理鼠标滚轮、鼠标拖动和触控拖动，更新滚动偏移并维护焦点和捕获。

**函数原型：**

```c
XGE_API int xgeXuiScrollViewEvent(xge_xui_scroll_view pScroll, const xge_event_t* pEvent);
```

**参数：**

- `pScroll`：滚动视图对象。
- `pEvent`：输入事件。

**返回值：**

- 已处理事件返回 `XGE_XUI_EVENT_CONSUMED`。
- 不相关事件返回 `XGE_XUI_EVENT_CONTINUE`。

**资源归属：**

不接管事件对象。

**补充说明：**

- 滚轮步长按 `32.0f` 像素缩放。
- 拖动时会设置 pointer capture。

**范例代码：**

```c
return xgeXuiScrollViewEvent(&scroll, event);
```

**相关 API：**

- `xgeXuiScrollViewSetOffset`
- `xgeXuiSetCapture`

---

### xgeXuiScrollViewEventProc

滚动视图 widget 事件适配器。

**功能：**

把 widget 事件回调转发给 `xgeXuiScrollViewEvent`。

**函数原型：**

```c
XGE_API int xgeXuiScrollViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
```

**参数：**

- `pWidget`：触发事件的 widget。
- `pEvent`：输入事件。
- `pUser`：滚动视图对象指针。

**返回值：**

返回 `xgeXuiScrollViewEvent` 的处理结果。

**资源归属：**

不接管任何参数。

**补充说明：**

- 普通用户通常不需要手动调用该函数。

**范例代码：**

```c
widget->procEvent = xgeXuiScrollViewEventProc;
```

**相关 API：**

- `xgeXuiScrollViewInit`

---

### xgeXuiScrollViewPaintProc

绘制滚动视图。

**功能：**

绘制滚动视图背景，以及按内容比例计算的水平和垂直滚动条。

**函数原型：**

```c
XGE_API void xgeXuiScrollViewPaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载滚动视图的 widget。
- `pUser`：滚动视图对象指针。

**返回值：**

无。

**资源归属：**

不接管任何参数。

**补充说明：**

- 滚动条厚度为 4 像素，滑块最小尺寸为 8 像素。

**范例代码：**

```c
xgeXuiScrollViewPaintProc(widget, &scroll);
```

**相关 API：**

- `xgeXuiScrollViewSetColors`

---

### xgeXuiListViewInit

初始化列表控件。

**功能：**

把 `xge_xui_list_view_t` 绑定到 widget，设置默认行高、选择状态、颜色、事件回调和绘制回调。

**函数原型：**

```c
XGE_API int xgeXuiListViewInit(xge_xui_list_view pList, xge_xui_context pContext, xge_xui_widget pWidget);
```

**参数：**

- `pList`：列表对象。
- `pContext`：所属 XUI context。
- `pWidget`：承载列表的 widget。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

列表对象、context 和 widget 均由调用者持有。

**补充说明：**

- 默认选中项为 -1。
- 初始化会让 widget 可聚焦并启用 clip。

**范例代码：**

```c
xge_xui_list_view_t list;
xgeXuiListViewInit(&list, &ui, widget);
```

**相关 API：**

- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`

---

### xgeXuiListViewUnit

释放列表控件绑定。

**功能：**

解除列表对 widget 事件和绘制回调的占用，并清空列表对象。

**函数原型：**

```c
XGE_API void xgeXuiListViewUnit(xge_xui_list_view pList);
```

**参数：**

- `pList`：列表对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 widget、字体或列表项字符串。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该列表时才会清理回调。

**范例代码：**

```c
xgeXuiListViewUnit(&list);
```

**相关 API：**

- `xgeXuiListViewInit`

---

### xgeXuiListViewSetItems

设置列表项。

**功能：**

设置列表项字符串数组和数量，并夹紧选中项和滚动位置。

**函数原型：**

```c
XGE_API void xgeXuiListViewSetItems(xge_xui_list_view pList, const char** arrItems, int iCount);
```

**参数：**

- `pList`：列表对象，可以为 `NULL`。
- `arrItems`：字符串指针数组。
- `iCount`：列表项数量。

**返回值：**

无。

**资源归属：**

列表保存数组和字符串的借用指针，不复制内容。

**补充说明：**

- `iCount` 小于 0 时按 0 处理。
- 如果原选中项越界，会重置为 -1。

**范例代码：**

```c
const char* items[] = {"A", "B", "C"};
xgeXuiListViewSetItems(&list, items, 3);
```

**相关 API：**

- `xgeXuiListViewSetSelected`

---

### xgeXuiListViewSetFont

设置列表字体。

**功能：**

设置列表项绘制字体，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiListViewSetFont(xge_xui_list_view pList, xge_font pFont);
```

**参数：**

- `pList`：列表对象，可以为 `NULL`。
- `pFont`：字体，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

字体由调用者或资源系统管理。

**补充说明：**

- 字体为空时仍绘制行背景和选中背景，但不绘制文本。

**范例代码：**

```c
xgeXuiListViewSetFont(&list, font);
```

**相关 API：**

- `xgeXuiListViewPaintProc`

---

### xgeXuiListViewSetItemHeight

设置列表行高。

**功能：**

设置每个列表项的行高，并夹紧滚动位置。

**函数原型：**

```c
XGE_API void xgeXuiListViewSetItemHeight(xge_xui_list_view pList, float fHeight);
```

**参数：**

- `pList`：列表对象，可以为 `NULL`。
- `fHeight`：行高。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 行高小于 1 时会夹到 1。

**范例代码：**

```c
xgeXuiListViewSetItemHeight(&list, 28.0f);
```

**相关 API：**

- `xgeXuiListViewSetScroll`

---

### xgeXuiListViewSetSelected

设置列表选中项。

**功能：**

设置当前选中行索引，并在变化时标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiListViewSetSelected(xge_xui_list_view pList, int iIndex);
```

**参数：**

- `pList`：列表对象，可以为 `NULL`。
- `iIndex`：目标索引。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 越界索引会被设置为 -1。
- 程序主动设置选中项不会触发 select 回调。

**范例代码：**

```c
xgeXuiListViewSetSelected(&list, 0);
```

**相关 API：**

- `xgeXuiListViewGetSelected`

---

### xgeXuiListViewGetSelected

获取列表选中项。

**功能：**

返回当前选中行索引。

**函数原型：**

```c
XGE_API int xgeXuiListViewGetSelected(xge_xui_list_view pList);
```

**参数：**

- `pList`：列表对象。

**返回值：**

- 返回当前选中索引。
- 无选中项或 `pList` 为 `NULL` 时返回 -1。

**资源归属：**

不接管外部资源。

**补充说明：**

- 返回值可直接用于访问调用者持有的 item 数组。

**范例代码：**

```c
int index = xgeXuiListViewGetSelected(&list);
```

**相关 API：**

- `xgeXuiListViewSetSelected`

---

### xgeXuiListViewSetScroll

设置列表滚动位置。

**功能：**

设置列表垂直滚动偏移，并夹到合法范围。

**函数原型：**

```c
XGE_API void xgeXuiListViewSetScroll(xge_xui_list_view pList, float fScrollY);
```

**参数：**

- `pList`：列表对象，可以为 `NULL`。
- `fScrollY`：垂直滚动偏移。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 只有偏移实际变化时才标记重绘。

**范例代码：**

```c
xgeXuiListViewSetScroll(&list, 64.0f);
```

**相关 API：**

- `xgeXuiListViewGetScroll`

---

### xgeXuiListViewGetScroll

获取列表滚动位置。

**功能：**

返回列表当前垂直滚动偏移。

**函数原型：**

```c
XGE_API float xgeXuiListViewGetScroll(xge_xui_list_view pList);
```

**参数：**

- `pList`：列表对象。

**返回值：**

- 返回当前垂直滚动偏移。
- `pList` 为 `NULL` 时返回 0。

**资源归属：**

不接管外部资源。

**补充说明：**

- 该值以像素为单位。

**范例代码：**

```c
float y = xgeXuiListViewGetScroll(&list);
```

**相关 API：**

- `xgeXuiListViewSetScroll`

---

### xgeXuiListViewSetSelect

设置列表选择回调。

**功能：**

注册用户点击选择列表项时触发的回调和用户数据。

**函数原型：**

```c
XGE_API void xgeXuiListViewSetSelect(xge_xui_list_view pList, xge_xui_select_proc procSelect, void* pUser);
```

**参数：**

- `pList`：列表对象，可以为 `NULL`。
- `procSelect`：选择回调，可以为 `NULL`。
- `pUser`：传给回调的用户数据。

**返回值：**

无。

**资源归属：**

`pUser` 由调用者管理，XUI 只保存并回传指针。

**补充说明：**

- 回调只由用户事件触发，程序调用 `xgeXuiListViewSetSelected` 不触发。

**范例代码：**

```c
xgeXuiListViewSetSelect(&list, on_select, user);
```

**相关 API：**

- `xgeXuiListViewEvent`

---

### xgeXuiListViewSetColors

设置列表颜色。

**功能：**

设置列表背景、普通行、选中行、文本、滚动条轨道和滚动条滑块颜色。

**函数原型：**

```c
XGE_API void xgeXuiListViewSetColors(xge_xui_list_view pList, uint32_t iBackground, uint32_t iRow, uint32_t iSelected, uint32_t iText, uint32_t iBar, uint32_t iThumb);
```

**参数：**

- `pList`：列表对象，可以为 `NULL`。
- `iBackground`：背景颜色。
- `iRow`：普通行颜色。
- `iSelected`：选中行颜色。
- `iText`：文本颜色。
- `iBar`：滚动条轨道颜色。
- `iThumb`：滚动条滑块颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 设置后会标记重绘。

**范例代码：**

```c
xgeXuiListViewSetColors(&list, bg, row, selected, text, bar, thumb);
```

**相关 API：**

- `xgeXuiListViewPaintProc`

---

### xgeXuiListViewEvent

处理列表事件。

**功能：**

处理鼠标滚轮滚动和鼠标/触控点击选择列表项。

**函数原型：**

```c
XGE_API int xgeXuiListViewEvent(xge_xui_list_view pList, const xge_event_t* pEvent);
```

**参数：**

- `pList`：列表对象。
- `pEvent`：输入事件。

**返回值：**

- 已处理事件返回 `XGE_XUI_EVENT_CONSUMED`。
- 不相关事件返回 `XGE_XUI_EVENT_CONTINUE`。

**资源归属：**

不接管事件对象。

**补充说明：**

- 事件必须发生在列表 widget rect 内才会处理。
- 滚轮步长按行高缩放。

**范例代码：**

```c
return xgeXuiListViewEvent(&list, event);
```

**相关 API：**

- `xgeXuiListViewSetSelect`
- `xgeXuiListViewSetScroll`

---

### xgeXuiListViewEventProc

列表 widget 事件适配器。

**功能：**

把 widget 事件回调转发给 `xgeXuiListViewEvent`。

**函数原型：**

```c
XGE_API int xgeXuiListViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
```

**参数：**

- `pWidget`：触发事件的 widget。
- `pEvent`：输入事件。
- `pUser`：列表对象指针。

**返回值：**

返回 `xgeXuiListViewEvent` 的处理结果。

**资源归属：**

不接管任何参数。

**补充说明：**

- 普通用户通常不需要手动调用该函数。

**范例代码：**

```c
widget->procEvent = xgeXuiListViewEventProc;
```

**相关 API：**

- `xgeXuiListViewInit`

---

### xgeXuiListViewPaintProc

绘制列表控件。

**功能：**

绘制列表背景、可见行、选中行、文本和垂直滚动条。

**函数原型：**

```c
XGE_API void xgeXuiListViewPaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载列表的 widget。
- `pUser`：列表对象指针。

**返回值：**

无。

**资源归属：**

不接管 widget、字体或 item 字符串。

**补充说明：**

- 只绘制当前可见范围内的行，适合较长列表。
- 滚动条滑块最小高度为 8 像素。

**范例代码：**

```c
xgeXuiListViewPaintProc(widget, &list);
```

**相关 API：**

- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetColors`

---

### xgeXuiVirtualListInit

初始化轻量虚拟列表控件。VirtualList 使用固定 item height 和可见 slot widget 复用，适合大量数据的 APP 列表或游戏内嵌界面列表。

**核心 API：**

```c
XGE_API int xgeXuiVirtualListInit(xge_xui_virtual_list pList, xge_xui_context pContext, xge_xui_widget pWidget);
XGE_API void xgeXuiVirtualListUnit(xge_xui_virtual_list pList);
XGE_API void xgeXuiVirtualListSetAdapter(xge_xui_virtual_list pList, xge_xui_virtual_list_count_proc procCount, xge_xui_virtual_list_create_proc procCreate, xge_xui_virtual_list_bind_proc procBind, void* pUser);
XGE_API void xgeXuiVirtualListSetItemHeight(xge_xui_virtual_list pList, float fHeight);
XGE_API void xgeXuiVirtualListSetScroll(xge_xui_virtual_list pList, float fScrollY);
XGE_API int xgeXuiVirtualListGetFirstVisible(xge_xui_virtual_list pList);
XGE_API int xgeXuiVirtualListGetVisibleCount(xge_xui_virtual_list pList);
XGE_API xge_xui_widget xgeXuiVirtualListGetSlotWidget(xge_xui_virtual_list pList, int iSlot);
```

`SetAdapter` 的 `count/create/bind` 分别负责返回 item 数量、创建 slot widget、把数据 index 绑定到 slot。滚动时只移动和重绑可见 slot；不可见 item 不会创建 widget。事件命中使用 content rect，支持滚轮、滚动条拖动和键盘选择。

XSON `type:"virtualList"` 支持 `itemCount`、`itemHeight`、`scrollY`、`backgroundColor`、`barColor`、`thumbColor` 和 `itemTemplate`。`itemTemplate` 可为内联对象，或引用顶层 `templates` 中的模板名；slot 创建复用 page widget builder，因此模板内可继续使用已有控件、样式和 token。

---

### xgeXuiDialogInit

初始化对话框控件。

**功能：**

把 `xge_xui_dialog_t` 绑定到 widget，设置默认颜色、打开状态、事件回调和绘制回调。

**函数原型：**

```c
XGE_API int xgeXuiDialogInit(xge_xui_dialog pDialog, xge_xui_context pContext, xge_xui_widget pWidget);
```

**参数：**

- `pDialog`：对话框对象。
- `pContext`：所属 XUI context。
- `pWidget`：承载对话框的 widget。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数为空返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

对话框对象、context 和 widget 均由调用者持有。

**补充说明：**

- 初始化后对话框默认打开。
- 初始化会让 widget 可聚焦并启用 clip。

**范例代码：**

```c
xge_xui_dialog_t dialog;
xgeXuiDialogInit(&dialog, &ui, widget);
```

**相关 API：**

- `xgeXuiDialogUnit`
- `xgeXuiDialogSetOpen`

---

### xgeXuiDialogUnit

释放对话框控件绑定。

**功能：**

解除对话框对 widget 事件和绘制回调的占用，并清空对象。

**函数原型：**

```c
XGE_API void xgeXuiDialogUnit(xge_xui_dialog pDialog);
```

**参数：**

- `pDialog`：对话框对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

不释放 widget、字体或标题字符串。

**补充说明：**

- 只有 widget 的 `pUser` 仍指向该对话框时才会清理回调。

**范例代码：**

```c
xgeXuiDialogUnit(&dialog);
```

**相关 API：**

- `xgeXuiDialogInit`

---

### xgeXuiDialogSetTitle

设置对话框标题。

**功能：**

设置对话框标题字体和标题文本，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiDialogSetTitle(xge_xui_dialog pDialog, xge_font pFont, const char* sTitle);
```

**参数：**

- `pDialog`：对话框对象，可以为 `NULL`。
- `pFont`：标题字体，可以为 `NULL`。
- `sTitle`：标题文本；传入 `NULL` 等同于空字符串。

**返回值：**

无。

**资源归属：**

对话框保存标题字符串借用指针，不复制字符串。

**补充说明：**

- 标题绘制在顶部 24 像素标题区域内，右侧预留关闭按钮空间。

**范例代码：**

```c
xgeXuiDialogSetTitle(&dialog, font, "Confirm");
```

**相关 API：**

- `xgeXuiDialogPaintProc`

---

### xgeXuiDialogSetClose

设置对话框关闭回调。

**功能：**

注册用户点击关闭按钮时触发的回调和用户数据。

**函数原型：**

```c
XGE_API void xgeXuiDialogSetClose(xge_xui_dialog pDialog, xge_xui_click_proc procClose, void* pUser);
```

**参数：**

- `pDialog`：对话框对象，可以为 `NULL`。
- `procClose`：关闭回调，可以为 `NULL`。
- `pUser`：传给回调的用户数据。

**返回值：**

无。

**资源归属：**

`pUser` 由调用者管理，XUI 只保存并回传指针。

**补充说明：**

- 点击关闭按钮时会先关闭对话框，再触发回调。

**范例代码：**

```c
xgeXuiDialogSetClose(&dialog, on_close, user);
```

**相关 API：**

- `xgeXuiDialogEvent`

---

### xgeXuiDialogSetOpen

设置对话框打开状态。

**功能：**

打开或关闭对话框，并同步设置 widget 可见状态。

**函数原型：**

```c
XGE_API void xgeXuiDialogSetOpen(xge_xui_dialog pDialog, int bOpen);
```

**参数：**

- `pDialog`：对话框对象，可以为 `NULL`。
- `bOpen`：非 0 打开，0 关闭。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 状态变化时会标记重绘。

**范例代码：**

```c
xgeXuiDialogSetOpen(&dialog, 1);
```

**相关 API：**

- `xgeXuiDialogIsOpen`

---

### xgeXuiDialogIsOpen

获取对话框打开状态。

**功能：**

返回对话框当前是否打开。

**函数原型：**

```c
XGE_API int xgeXuiDialogIsOpen(xge_xui_dialog pDialog);
```

**参数：**

- `pDialog`：对话框对象。

**返回值：**

- 打开返回 1，关闭返回 0。
- `pDialog` 为 `NULL` 时返回 0。

**资源归属：**

不接管外部资源。

**补充说明：**

- 返回值已经归一化为 0 或 1。

**范例代码：**

```c
if (xgeXuiDialogIsOpen(&dialog)) {
	/* dialog is visible */
}
```

**相关 API：**

- `xgeXuiDialogSetOpen`

---

### xgeXuiDialogSetColors

设置对话框颜色。

**功能：**

设置遮罩、背景、标题和关闭按钮颜色，并标记重绘。

**函数原型：**

```c
XGE_API void xgeXuiDialogSetColors(xge_xui_dialog pDialog, uint32_t iBackdrop, uint32_t iBackground, uint32_t iTitle, uint32_t iClose);
```

**参数：**

- `pDialog`：对话框对象，可以为 `NULL`。
- `iBackdrop`：遮罩颜色。
- `iBackground`：对话框背景颜色。
- `iTitle`：标题文本颜色。
- `iClose`：关闭按钮颜色。

**返回值：**

无。

**资源归属：**

不接管外部资源。

**补充说明：**

- 遮罩绘制在父 widget rect 上；没有父级时绘制在自身 rect 上。

**范例代码：**

```c
xgeXuiDialogSetColors(&dialog, backdrop, background, title, close);
```

**相关 API：**

- `xgeXuiDialogPaintProc`

---

### xgeXuiDialogEvent

处理对话框事件。

**功能：**

在对话框打开时拦截常见输入事件，处理焦点和关闭按钮点击。

**函数原型：**

```c
XGE_API int xgeXuiDialogEvent(xge_xui_dialog pDialog, const xge_event_t* pEvent);
```

**参数：**

- `pDialog`：对话框对象。
- `pEvent`：输入事件。

**返回值：**

- 已拦截或处理事件返回 `XGE_XUI_EVENT_CONSUMED`。
- 对话框关闭、参数无效或不相关事件返回 `XGE_XUI_EVENT_CONTINUE`。

**资源归属：**

不接管事件对象。

**补充说明：**

- 点击对话框外部也会被消费，用于实现模态遮罩。
- 点击关闭按钮会关闭对话框并触发 close 回调。

**范例代码：**

```c
return xgeXuiDialogEvent(&dialog, event);
```

**相关 API：**

- `xgeXuiDialogSetClose`
- `xgeXuiDialogSetOpen`

---

### xgeXuiDialogEventProc

对话框 widget 事件适配器。

**功能：**

把 widget 事件回调转发给 `xgeXuiDialogEvent`。

**函数原型：**

```c
XGE_API int xgeXuiDialogEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
```

**参数：**

- `pWidget`：触发事件的 widget。
- `pEvent`：输入事件。
- `pUser`：对话框对象指针。

**返回值：**

返回 `xgeXuiDialogEvent` 的处理结果。

**资源归属：**

不接管任何参数。

**补充说明：**

- 普通用户通常不需要手动调用该函数。

**范例代码：**

```c
widget->procEvent = xgeXuiDialogEventProc;
```

**相关 API：**

- `xgeXuiDialogInit`

---

### xgeXuiDialogPaintProc

绘制对话框控件。

**功能：**

绘制模态遮罩、对话框背景、关闭按钮和可选标题文本。

**函数原型：**

```c
XGE_API void xgeXuiDialogPaintProc(xge_xui_widget pWidget, void* pUser);
```

**参数：**

- `pWidget`：承载对话框的 widget。
- `pUser`：对话框对象指针。

**返回值：**

无。

**资源归属：**

不接管 widget、字体或标题字符串。

**补充说明：**

- 对话框关闭时不绘制。
- 绘制时会更新 `tCloseRect`，供事件阶段判断关闭按钮命中。

**范例代码：**

```c
xgeXuiDialogPaintProc(widget, &dialog);
```

**相关 API：**

- `xgeXuiDialogSetColors`
- `xgeXuiDialogEvent`

## 生命周期与所有权

XUI context 由调用者持有，使用 `xgeXuiInit` / `xgeXuiUnit` 管理。Widget 和控件对象也由调用者持有，XUI 只维护树关系和回调。

Host 回调和 `pUser` 由宿主管理，XUI 不接管其生命周期。控件引用的 font、texture 等 XGE 资源必须在控件绘制期间保持有效。

## 线程约束

XUI 是 retained-mode GUI，建议在主 UI 线程进行事件派发、布局更新和 paint。不要多个线程同时修改同一个 widget tree。

异步资源完成后应回到 UI 线程更新控件状态，并请求 refresh。

## 后端差异

XUI 本身不绑定 GL、Sokol 或 XGE 私有状态。当前 XGE 内孵化版本使用 XGE host 绘制；未来剥离到 XUI 仓库后，可以通过其他 host 接入。

IME、触控、鼠标和候选框位置依赖平台后端能力。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 控件修改后不刷新 | 没有标记 dirty 或请求刷新 | 调用 `xgeXuiWidgetMarkPaint` 或 `xgeXuiRefreshRequest`。 |
| 输入框 IME 候选框位置不对 | 宿主没有读取 candidate rect | 使用 `xgeXuiInputGetCandidateRect` 给系统 IME。 |
| 布局频繁抖动 | 多次修改 widget 未批处理 | 用 `xgeXuiLayoutBatchBegin` / `xgeXuiLayoutBatchEnd` 包住批量修改。 |

## 相关示例

- `examples/xui_incubation`
- `examples/xui_bridge`
- `examples\xui_bridge\build.bat`
