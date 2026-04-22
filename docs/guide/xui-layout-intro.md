# XUI 布局入门

本教程介绍 XUI 第一版保留的高性能布局子集。

[返回教程索引](README.md) | [XUI API](../api/xui.md) | [XUI 桥接](xui-bridge-intro.md)

## 设计目标

XUI 布局不是 HTML/CSS 复刻。第一版只保留常用、可预测、性能稳定的布局方式：

- Absolute：手动设置矩形。
- Row：横向排列。
- Column：纵向排列。
- Stack：层叠排列。
- Grid：固定列网格。

复杂富文本和自适应布局可以后续扩展，但第一版先保证游戏 UI、工具面板和 APP 模式稳定。

## Size 类型

```c
xgeXuiSizePx(120.0f);       /* 固定像素 */
xgeXuiSizeDip(48.0f);       /* 设备无关尺寸 */
xgeXuiSizePercent(100.0f);  /* 父容器百分比 */
xgeXuiSizeGrow(1.0f);       /* 分配剩余空间 */
xgeXuiSizeContent();        /* 由内容测量决定 */
```

## Column 布局

```c
xge_xui_widget root;

root = xgeXuiRoot(&ui);
xgeXuiWidgetSetLayout(root, XGE_XUI_LAYOUT_COLUMN);
xgeXuiWidgetSetPaddingPx(root, 16.0f, 16.0f, 16.0f, 16.0f);
```

子控件按顺序从上到下排列。适合设置页、菜单、属性面板。

## Row 布局

```c
xgeXuiWidgetSetLayout(toolbar, XGE_XUI_LAYOUT_ROW);
xgeXuiWidgetSetAlign(toolbar, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_CENTER);
```

Row 适合工具栏、按钮组、状态栏。

## Grid 布局

```c
xgeXuiWidgetSetLayout(grid, XGE_XUI_LAYOUT_GRID);
xgeXuiWidgetSetGrid(grid, 4, 40.0f, 8.0f, 8.0f);
```

Grid 第一版使用固定列数和行高，避免复杂 auto placement 带来的性能和规则成本。

## Anchor 和 Absolute

Absolute 适合弹窗、拖拽面板和精确定位控件：

```c
xgeXuiWidgetSetLayout(root, XGE_XUI_LAYOUT_ABSOLUTE);
xgeXuiWidgetSetAnchorPx(dialog, XGE_XUI_ANCHOR_LEFT | XGE_XUI_ANCHOR_TOP, 210.0f, 140.0f, 0.0f, 0.0f);
xgeXuiWidgetSetSize(dialog, xgeXuiSizePx(360.0f), xgeXuiSizePx(220.0f));
```

## 批量修改

大量调整控件时使用布局批处理：

```c
xgeXuiLayoutBatchBegin(&ui);
/* add/remove/update many widgets */
xgeXuiLayoutBatchEnd(&ui);
```

这样可以减少重复 layout 和 repaint。

## 常见错误

不要用每帧重建控件树模拟 IMGUI。XUI 是 retained-mode GUI，应复用控件并更新状态。

不要把复杂文本排版塞进布局系统第一版。先用 label/input/list 等控件组合实现。

不要忘记 content size 依赖 measure 回调。自定义控件使用 `xgeXuiWidgetSetMeasure`。

## 下一步

- 学习 [XUI 控件入门](xui-controls-intro.md)。
- 需要 APP 模式刷新读 [XUI 渲染入门](xui-render-intro.md)。
