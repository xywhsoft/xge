# XUI Chrome Style

`xge_xui_chrome_style_t` 是窗口框架类控件的局部全局样式表。当前只覆盖 5 类控件：

- `Window`
- `Menu`
- `MenuBar`
- `Toolbar`
- `StatusBar`

这不是完整应用级样式系统。普通表单控件仍然优先使用自己的重构样式和 Widget 状态样式，避免一次性扩大影响面。

## 设计目标

窗口通常由标题栏、菜单栏、工具栏、状态栏和客户区组成。前四者属于 chrome 区域，应使用统一配色；客户区承载真实业务内容，应使用更安静的内容背景。

因此 chrome style 将样式拆成三组：

- `tWindow`: 窗口框架、标题栏、标题按钮、客户区背景。
- `tBarColors`: MenuBar、Toolbar、StatusBar 共享的条状控件颜色。
- `tMenuColors` / `tMenuMetrics`: 弹出菜单的颜色和布局指标。

## API

```c
void xgeXuiChromeStyleDefault(xge_xui_chrome_style pStyle, const xge_xui_theme_t* pTheme);
void xgeXuiSetChromeStyle(xge_xui_context pContext, const xge_xui_chrome_style_t* pStyle);
const xge_xui_chrome_style_t* xgeXuiGetChromeStyle(xge_xui_context pContext);
```

`xgeXuiInit` 会根据当前 theme 初始化 chrome style；`xgeXuiSetTheme` 会同步刷新 chrome style。若项目需要自己的窗口框架风格，应在设置 theme 之后调用 `xgeXuiSetChromeStyle`。

## 使用约束

- Chrome style 是默认值来源，控件仍然允许局部覆盖颜色和指标。
- `Widget` 继续负责背景、边框、盒模型和状态基础设施。
- `MenuBar`、`Toolbar`、`StatusBar` 的默认背景、边框和状态色来自 `tBarColors`。
- `Menu` 的默认布局和颜色来自 `tMenuMetrics` / `tMenuColors`。
- `Window` 的框架和客户区颜色来自 `tWindow`。

## 示例

```c
xge_xui_chrome_style_t chrome;

xgeXuiChromeStyleDefault(&chrome, xgeXuiGetTheme(&xui));
chrome.tBarColors.iBackground = XGE_COLOR_RGBA(224, 235, 246, 255);
chrome.tWindow.iClientBackground = XGE_COLOR_RGBA(242, 247, 252, 255);
xgeXuiSetChromeStyle(&xui, &chrome);
```

注意 `xgeXuiWidgetSetBorder(widget, width, color)` 的参数顺序是宽度在前、颜色在后。Chrome 控件默认通过 Widget 绘制边框，不能把颜色值误传给宽度参数。
