# XUI 布局入门

本教程介绍当前 XUI 布局系统的能力边界。XUI 是 retained-mode GUI，不是 HTML/CSS 复刻；它优先保证 APP 界面、游戏内嵌界面和工具面板的布局可预测、内存可控、运行时开销稳定。

[返回教程索引](README.md) | [XUI API](../api/xui.md) | [XUI 控件](xui-controls-intro.md) | [XUI 渲染](xui-render-intro.md)

> 当前 guide 描述第一版布局 API。XUI 基础层统一 Widget role、box model、overflow、clip、Z 序、事件、焦点、TAB、IME，并按 ScrollModel / ScrollFrame / ScrollView / VirtualView 重构 viewport 系列。

## 布局模型

XUI 使用 widget tree。每个 widget 有 `outerRect`、`borderRect`、`paddingRect`、`contentRect` 四层盒模型，以及一份轻量 `xge_xui_style_t`；`xgeXuiWidgetGetRect` 返回当前 `borderRect`。布局更新只在 dirty 时执行，业务应复用 widget，而不是每帧重建 UI 树。

常用流程：

```c
xge_xui_context_t ui;
xge_xui_widget root;

xgeXuiInit(&ui);
root = xgeXuiRoot(&ui);
xgeXuiWidgetSetLayout(root, XGE_XUI_LAYOUT_COLUMN);
xgeXuiWidgetSetPaddingPx(root, 16.0f, 16.0f, 16.0f, 16.0f);
xgeXuiUpdate(&ui, delta);
xgeXuiPaint(&ui);
xgeXuiUnit(&ui);
```

根节点默认跟随窗口尺寸。APP 或全面屏界面可以通过 safe area 把根节点的 content 区域向内收缩：

```c
xgeXuiSetSafeAreaPx(&ui, left, top, right, bottom);
```

## Size 类型

XUI 的尺寸是显式类型，不做 CSS 式复杂约束求解。

```c
xgeXuiSizePx(120.0f);       /* 固定像素 */
xgeXuiSizeDip(48.0f);       /* 设备无关尺寸 */
xgeXuiSizePercent(100.0f);  /* 父容器 content 尺寸百分比 */
xgeXuiSizeGrow(1.0f);       /* 分配主轴剩余空间 */
xgeXuiSizeContent();        /* 由 measure 回调或控件内容决定 */
```

`minWidth/minHeight/maxWidth/maxHeight` 会在布局分配后 clamp。Row/Column 的 grow 分配会处理 min/max 重分配；空间不足时允许 overflow，由上层选择 `visible`、`clip`、`hidden` 或显式 ScrollView/VirtualList。

## 布局类型

`Absolute` 适合弹窗、拖拽面板、游戏 HUD 和精确定位控件。子节点可以直接设置 rect，也可以使用 anchor。

```c
xgeXuiWidgetSetLayout(panel, XGE_XUI_LAYOUT_ABSOLUTE);
xgeXuiWidgetSetAnchorPx(dialog, XGE_XUI_ANCHOR_LEFT | XGE_XUI_ANCHOR_TOP, 210.0f, 140.0f, 0.0f, 0.0f);
```

`Row` 横向排列子节点，主轴从左到右，`gap` 控制间距，`justify` 控制剩余空间分布，`alignY` 控制交叉轴对齐。

```c
xgeXuiWidgetSetLayout(toolbar, XGE_XUI_LAYOUT_ROW);
xgeXuiWidgetSetGap(toolbar, 8.0f);
xgeXuiWidgetSetJustify(toolbar, XGE_XUI_JUSTIFY_SPACE_BETWEEN);
```

`Column` 纵向排列子节点，适合设置页、菜单、表单和面板主体。

```c
xgeXuiWidgetSetLayout(panel, XGE_XUI_LAYOUT_COLUMN);
xgeXuiWidgetSetGap(panel, 10.0f);
```

`Stack` 把子节点放在同一个 slot 中，适合背景、遮罩、徽标和叠层状态。子节点可以通过 `alignX/alignY` 定位。

`Grid` 使用固定列数与行高，保持 O(N) 排布。第一版支持 `columnSpan`，不支持 row span、命名区域或自动回填。

```c
xgeXuiWidgetSetLayout(grid, XGE_XUI_LAYOUT_GRID);
xgeXuiWidgetSetGrid(grid, 4, 40.0f, 8.0f, 8.0f);
xgeXuiWidgetSetGridColumnSpan(card, 2);
```

`Dock` 适合 APP shell：header、footer、left/right rail 和 fill content。子节点用 dock role 声明占位顺序。

```c
xgeXuiWidgetSetLayout(shell, XGE_XUI_LAYOUT_DOCK);
xgeXuiWidgetSetDock(header, XGE_XUI_DOCK_TOP);
xgeXuiWidgetSetDock(content, XGE_XUI_DOCK_FILL);
```

## 滚动和长列表

普通 widget 的 `overflow: scroll` 不会自动变成滚动容器。需要滚动时显式使用 ScrollView 或 VirtualView 系列控件；这些控件会通过 ScrollFrame 启用 viewport 裁剪和滚动条区域布局。

ScrollView 适合中等规模内容树，会对子树应用滚动 offset，并按 content rect 做命中：

```c
xgeXuiScrollViewInit(&scroll, &ui, widget);
xgeXuiScrollViewSetContentSize(&scroll, 800.0f, 1200.0f);
xgeXuiScrollViewSetOffset(&scroll, 0.0f, 160.0f);
```

ScrollView 的滚轮方向通过 `wheelAxis` 显式控制，默认纵向；内容拖拽滚动默认关闭，避免干扰地图、画布和编辑器类控件；滚动条交互由 ScrollFrame 统一处理。

VirtualList 适合大量同高列表项。它复用可见 slot，不为所有 item 创建 widget：

```c
xgeXuiVirtualListInit(&list, &ui, widget);
xgeXuiVirtualListSetItemCount(&list, 10000);
xgeXuiVirtualListSetItemHeight(&list, 28.0f);
```

VirtualList、TreeView、TableView 后续共享 VirtualView，而不是各自实现可见范围、slot 复用和滚动边界。

## XSON 声明式布局

XUI 可以通过 XSON 快速描述页面，不需要 UI 设计器。XSON 会创建 retained widget tree，样式在 load/refresh/sync 阶段解析到轻量 style cache，运行时热路径不反复查字符串字段。

```json
{
  "xui": 1,
  "tokens": {
    "colors": { "panel": "#202631", "accent": "#3277FF" },
    "spacing": { "pad": 10, "gap": 8, "headerH": 48 }
  },
  "styles": {
    "shell": { "layout": "dock", "width": "100%", "height": "100%", "padding": "@spacing.pad", "gap": "@spacing.gap" },
    "header": { "dock": "top", "height": "@spacing.headerH", "background": "@colors.panel" },
    "content": { "dock": "fill", "width": "grow", "height": "grow" }
  },
  "tree": {
    "type": "dock",
    "id": "app",
    "style": "shell",
    "children": [
      { "type": "row", "id": "header", "style": "header" },
      { "type": "column", "id": "content", "style": "content" }
    ]
  }
}
```

样式支持 `@parent` 继承、tokens 引用和 widget inline 字段覆盖。XValue 父表只共享数据，不持有父表生命周期；由 `xge_xui_page_t` 统一持有 imported styles、merged styles 和当前 page styles，并在 `xgeXuiPageUnload` 释放。

## Dirty 和批量修改

布局、样式、绘制都有 dirty 标记。大量调整控件时使用布局批处理：

```c
xgeXuiLayoutBatchBegin(&ui);
/* add/remove/update many widgets */
xgeXuiLayoutBatchEnd(&ui);
```

batch 期间不会立即触发 refresh；结束时会标记需要重新布局的树。

## 调试

运行版 `xge` 不包含调试 API。调试相关能力在 `xgedbg` 中，通过 `XGE_DEBUGMODE` 编译宏隔离。调试版可使用 layout snapshot、overlay 和 page trace 查看 widget tree、dirty、rect/content、margin/padding、layout 类型和 XSON 加载信息。

## 常见错误

- 不要每帧重建控件树模拟 IMGUI。更新数据、样式或状态即可。
- 不要期待 Row/Column 空间不足时自动出现滚动条。显式选择 clip、ScrollView 或 VirtualList。
- 不要把大列表做成几千个普通子节点。使用 VirtualList。
- 不要在 release 代码里调用 xgedbg API。调试调用应使用 `#if XGE_DEBUGMODE` 隔离。
- `content` 尺寸依赖 measure 回调或控件自身测量；自定义控件需要设置 `xgeXuiWidgetSetMeasure`。

## 示例

- `examples/xui_layout_gallery`：手写 C API layout gallery。
- `examples/xui_xson_layout_gallery_lab`：上述 layout gallery 的 XSON 迁移验证。
- `examples/xui_xson_app_layout_lab`：APP shell、Dock、ScrollView 和 Grid 组合。
- `examples/xui_xson_virtual_list_lab`：XSON VirtualList 与 itemTemplate。
- `examples/xui_xson_style_lab`：tokens、style `@parent` 继承和 inline override。

## 下一步

- 学习 [XUI 控件入门](xui-controls-intro.md)。
- 需要 APP 模式刷新读 [XUI 渲染入门](xui-render-intro.md)。
