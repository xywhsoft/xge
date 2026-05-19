# XUI Viewport / Scroll

本文是 XUI 滚动和 viewport 系列的当前唯一设计口径。

本轮重构不保留 `ScrollViewBase` / `VirtualScrollViewBase` 旧分层。旧代码已从编译入口隔离，后续实现只按这里的边界推进。

## 名词边界

| 名称 | 定位 | 是否是控件 | 是否绘制滚动条 |
| --- | --- | --- | --- |
| `ScrollModel` | 纯滚动状态模型，负责 content size、viewport size、offset、clamp、坐标转换 | 否 | 否 |
| `ScrollFrame` | 滚动 UI 基础设施，负责 viewport、横向滚动条、纵向滚动条、右下角 grip 的 2x2 框架 | 是基础设施 | 是 |
| `ScrollView` | 真实内容滚动视图，把一个真实 content widget 放进可滚动虚拟画布 | 是 | 通过 ScrollFrame |
| `VirtualView` | 虚拟数据滚动视图，提供可见范围、slot/adapter、虚拟命中和选择基础 | 是/控件基类 | 通过 ScrollFrame |

旧口径中的 `ScrollViewBase` 删除。旧口径中的 `VirtualScrollViewBase` 改为 `VirtualView`，不再作为所有滚动控件的泛化基类。

## ScrollModel

`ScrollModel` 是纯状态对象，不持有 widget，不绘制，不处理事件。

职责：

- 记录 content width/height。
- 记录 viewport width/height。
- 记录 scroll offset。
- 根据 content/viewport 自动计算 max offset。
- clamp offset。
- 提供 `screen <-> viewport <-> content` 坐标转换。
- 提供 `ensureRectVisible` 这类纯几何能力。

`ScrollModel` 不知道滚动条、拖拽、滚轮、颜色、焦点、capture，也不保存 widget 指针。凡是涉及交互或绘制的状态都不进入 `ScrollModel`。

当前已落地的公开能力：

- `xgeXuiScrollModelInit`
- `xgeXuiScrollModelSetViewport` / `xgeXuiScrollModelGetViewport`
- `xgeXuiScrollModelSetContentSize`
- `xgeXuiScrollModelSetOffset` / `xgeXuiScrollModelScrollBy` / `xgeXuiScrollModelGetOffset`
- `xgeXuiScrollModelGetMaxOffset`
- `xgeXuiScrollModelEnsureRectVisible`
- `xgeXuiScrollModelScreenToViewport`
- `xgeXuiScrollModelViewportToContent`
- `xgeXuiScrollModelScreenToContent`
- `xgeXuiScrollModelContentToViewport`
- `xgeXuiScrollModelContentToScreen`

## ScrollFrame

`ScrollFrame` 是所有“内部空间映射到外部 viewport”的 UI 基础设施。

它的结构固定为：

```text
+-------------------------+-------------+
| viewport                | v scrollbar |
+-------------------------+-------------+
| h scrollbar             | corner grip  |
+-------------------------+-------------+
```

职责：

- 根据 widget content rect 和 `ScrollModel` 状态计算 viewport rect。
- 根据 overflow 情况显示或隐藏横向/纵向 `ScrollBar`。
- 同步 `ScrollBar` range、page、value。
- 处理滚轮，按配置支持横向、纵向或双向。
- 处理滚动条按钮、轨道、thumb 拖拽。
- 保证内容绘制和命中不会溢出到滚动条区域。
- 需要右下角 grip 时，提供独立 corner 区域。

`ScrollFrame` 不负责业务内容。它只负责滚动框架、滚动条和坐标映射。

当前已落地的公开能力：

- `xgeXuiScrollFrameInit` / `xgeXuiScrollFrameUnit`
- `xgeXuiScrollFrameGetViewportWidget`
- `xgeXuiScrollFrameGetHScrollBarWidget`
- `xgeXuiScrollFrameGetVScrollBarWidget`
- `xgeXuiScrollFrameGetCornerWidget`
- `xgeXuiScrollFrameGetViewportRect`
- `xgeXuiScrollFrameSetChange`
- `xgeXuiScrollFrameSetContentSize`
- `xgeXuiScrollFrameSetOffset` / `xgeXuiScrollFrameScrollBy` / `xgeXuiScrollFrameGetOffset`
- `xgeXuiScrollFrameSetScrollbarPolicy`
- `xgeXuiScrollFrameSetScrollbarMode` / `xgeXuiScrollFrameGetScrollbarMode`
- `xgeXuiScrollFrameSetWheelAxis` / `xgeXuiScrollFrameGetWheelAxis`
- `xgeXuiScrollFrameSetWheelStep`
- `xgeXuiScrollFrameSetContentDragEnabled` / `xgeXuiScrollFrameIsContentDragEnabled`
- `xgeXuiScrollFrameSetCornerMode`
- `xgeXuiScrollFrameSetMetrics`
- `xgeXuiScrollFrameSetColors`
- `xgeXuiScrollFrameSetButtonColors`
- `xgeXuiScrollFrameSetCornerColors`
- `xgeXuiScrollFrameLayout`
- `xgeXuiScrollFrameEvent` / `xgeXuiScrollFrameEventProc`
- `xgeXuiScrollFrameLayoutProc`
- `xgeXuiScrollFramePaintProc`

`ScrollFrame` 当前只作为基础设施落地。业务控件不应直接把它当成最终控件暴露给 XSON；恢复 XSON 时仍通过 `ScrollView`、`Popup`、`VirtualView` 等具体控件进入。

默认滚动条模式为 `XGE_XUI_SCROLLBAR_MODE_COMPACT`。需要传统完整滚动条时，由具体控件或调用方显式设置 `XGE_XUI_SCROLLBAR_MODE_FULL`。

## ScrollView

`ScrollView` 用于真实内容空间。

适合：

- 大画布。
- 地图编辑器。
- 图片或节点编辑区域。
- 包含真实子 widget 的滚动表单。
- Popup 内部 content 大于窗口时的承载容器。

`ScrollView` 拥有：

- 一个 `ScrollModel`。
- 一个 `ScrollFrame`。
- 一个真实 `contentWidget`。

子 widget 的布局坐标属于 content 空间。`ScrollView` 负责把 content 空间映射到 viewport，并通过 widget 变换/布局结果让事件命中仍按最终屏幕矩形工作。控件作者不应在业务控件里重复扣减 scroll offset。

内容拖拽默认关闭。地图、画布、编辑器类控件通常需要完整掌控鼠标事件，只有明确需要“拖动内容平移”时才打开。

当前已落地：

- `ScrollView` 基于 `ScrollModel + ScrollFrame` 实现，不再手写滚动条和偏移算法。
- `ScrollView` 创建内部 `contentWidget`，业务子 widget 必须挂到 `contentWidget` 下。
- `xgeXuiScrollViewGetContentWidget` 返回真实内容容器。
- `xgeXuiScrollViewGetViewportWidget` 返回裁剪 viewport。
- `xgeXuiScrollViewGetModel` / `xgeXuiScrollViewGetFrame` 返回内部基础设施对象。
- `xgeXuiScrollViewSetContentSize` 设置虚拟画布尺寸。
- `xgeXuiScrollViewSetOffset` / `xgeXuiScrollViewScrollBy` / `xgeXuiScrollViewGetOffset` 管理滚动偏移。
- `xgeXuiScrollViewEnsureRectVisible` / `xgeXuiScrollViewEnsureChildVisible` 负责滚动到可见区域。
- `xgeXuiScrollViewSetScrollbarPolicy` 同时设置横纵策略，`xgeXuiScrollViewSetScrollbarPolicyXY` 分别设置横纵策略。
- `xgeXuiScrollViewSetScrollbarMode` / `xgeXuiScrollViewGetScrollbarMode` 控制 full/compact 滚动条。
- `xgeXuiScrollViewSetWheelAxis` / `xgeXuiScrollViewGetWheelAxis` / `xgeXuiScrollViewSetWheelStep` 控制滚轮。
- `xgeXuiScrollViewSetContentDragEnabled` / `xgeXuiScrollViewIsContentDragEnabled` 控制内容拖拽平移。
- `xgeXuiScrollViewSetMetrics` / `xgeXuiScrollViewSetColors` 调整滚动条尺寸和颜色。

## VirtualView

`VirtualView` 用于虚拟数据空间，不是一个真实 content widget 容器。

适合：

- 大量行的 list。
- tree。
- table。
- property grid。
- 其他只创建可见项的虚拟化控件。

`VirtualView` 拥有：

- 一个 `ScrollModel`。
- 一个 `ScrollFrame`。
- 虚拟数据数量、可见范围、item/row 高度策略。
- adapter 或 slot 管理能力。

`VirtualView` 告诉控件“当前 viewport 对应哪些数据项”，由控件决定这些数据项如何绘制、命中和提交业务事件。

## Popup 关系

Popup 始终使用 `ScrollView` 承载内容，不再有“内容能放下时用普通 widget，放不下时换成 ScrollView”的双状态。

原因：

- ScrollView 本来就应该在内容不溢出时隐藏滚动条。
- Popup 只有一种承载路径，事件、裁剪、坐标和焦点逻辑更稳定。
- Menu、ComboBox、ColorPicker 等弹层控件不需要关心内容是否超过窗口。

Popup 作为动态尺寸容器，会把可见滚动条的预留尺寸计入弹层外框。也就是说，只有纵向溢出时，Popup 会优先增加纵向滚动条宽度，而不是压缩原本足够的横向 viewport；只有横向溢出时同理增加横向滚动条高度。只有窗口硬限制导致加上必要滚动条后仍放不下时，才允许另一轴滚动条出现。

## 坐标原则

- 原始事件坐标不被随意改写。
- ScrollModel 提供显式坐标转换。
- ScrollView 的真实子树最终应拥有正确的屏幕 rect，所以普通 widget 命中不需要业务代码手动换算。
- VirtualView 不移动真实子树，它以 viewport/content 坐标推导可见 item。
- Popup 内部滚动也必须沿用 ScrollView 的坐标模型，不能在 Popup 中另写一套偏移算法。

## XSON

Viewport 系列在重构期间仍暂时隔离的类型：

- `virtualList`
- `treeView`
- `tableView`
- `propertyGrid`
- `menu`
- `comboBox`
- `colorPicker`
- `textEdit`

加载这些类型应返回明确错误，不能回落到旧实现。

`scroll` / `scrollView` 已恢复 XSON 支持。`children` 会自动挂到内部 `contentWidget`，而不是挂到 ScrollView 外层框架同级。已支持字段：

- `contentSize` / `contentWidth` / `contentHeight`
- `offset` / `scrollOffset` / `contentOffset` / `scrollX` / `scrollY`
- `wheelAxis` / `wheelStep`
- `dragMode` / `contentDrag`
- `scrollbarPolicy` / `scrollbarPolicyX` / `scrollbarPolicyY`
- `scrollbarMode`
- `scrollbarSize` / `minThumbSize` / `thumbRadius` / `scrollbarButtonSize`
- `backgroundColor` / `background` / `barColor` / `thumbColor`

`popup` 也已恢复 XSON 支持。`children` 会自动挂到 Popup 内部 `contentWidget`；Popup 字段描述 anchor、direction、fallback、close policy、focus policy 和 content size，滚动能力来自内部 `ScrollView`。

`listView` 已恢复 XSON 支持。它不再依赖旧 `VirtualViewBase`，内部直接持有 `ScrollModel + ScrollFrame`，滚动条、滚轮和 viewport 裁剪由 ScrollFrame 处理，行选择和绘制由 ListView 处理。

后续恢复其它 viewport 控件的 XSON 支持时，字段以新分层为准：

- ScrollView 字段只描述真实内容滚动。
- VirtualView 字段只描述虚拟数据滚动。
- Popup 字段描述 anchor、direction、fallback、close policy、focus policy 和 content size；滚动能力来自内部 ScrollView。

## 迁移原则

- 不保留旧 `ScrollViewBase` / `VirtualScrollViewBase` API 作为新实现入口。
- 不让 ListView、TreeView、TableView、PropertyGrid 各自手写滚动条。
- 不让 Popup、Menu、ComboBox、ColorPicker 各自实现坐标偏移。
- 不通过补丁遮盖裁剪、Z 序、事件穿透和滚动条区域溢出问题。
- 所有 viewport 控件必须先走 `ScrollModel + ScrollFrame`，再落到具体业务控件。
