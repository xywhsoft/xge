# XUI Viewport Refactor Spec

本 spec 用于跟踪第三次 viewport 重构。实现期间不维护旧 viewport 控件兼容。

## 当前状态

- [x] 统一设计口径：`ScrollModel`、`ScrollFrame`、`ScrollView`、`VirtualView`。
- [x] 删除文档口径中的 `ScrollViewBase` / `VirtualScrollViewBase` 作为新设计入口。
- [x] 旧 viewport 依赖控件从 `xge_impl.c` 编译入口隔离。
- [x] XSON 对隔离控件返回明确不可用错误。
- [x] 删除旧 `xge_xui_scroll_view.c` / `xge_xui_virtual_list.c` 实现文件。
- [x] Input 默认右键菜单临时断开，不再把 Menu 旧实现拉回编译链。
- [x] `build_dll.bat` 通过，主库可继续构建。
- [x] 清理 `ScrollModel` / `ScrollFrame` 相关旧结构体字段，替换为新结构。
- [x] 实现纯状态 `ScrollModel`。
- [x] 实现 `ScrollFrame`。
- [x] 重写 `ScrollView`。
- [x] 恢复 `scroll` / `scrollView` XSON 字段解析和 children 挂载到内部 content widget。
- [x] 重写 `Popup`，始终通过 `ScrollView` 承载内容。
- [ ] 逐个恢复依赖控件：Menu、ComboBox、ColorPicker、ListView、VirtualView 系列。
- [ ] 为后续恢复的控件补齐 XSON 字段解析和范例。

## 已落地文件

- `src/xge_xui_scroll_model.c`：纯状态滚动模型，不持有 widget，不绘制，不处理事件。
- `src/xge_xui_scroll_frame.c`：viewport + 横向滚动条 + 纵向滚动条 + corner 的滚动框架基础设施。
- `src/xge_xui_scroll_view.c`：真实内容滚动视图，持有内部 `contentWidget`，复用 `ScrollFrame` 处理滚动条、滚轮和内容拖拽。
- `src/xge_xui_popup.c`：通用弹层基础设施，挂载到 overlay root，复用内部 `ScrollView` 承载业务内容和溢出滚动。
- `xge.h`：新增 `xge_xui_scroll_frame_t`、`xge_xui_scroll_frame`、`xge_xui_scroll_frame_change_proc` 以及 ScrollModel/ScrollFrame 公开 API。
- `src/xge_impl.c`：纳入 `xge_xui_scroll_model.c`、`xge_xui_scrollbar.c`、`xge_xui_scroll_frame.c`、`xge_xui_scroll_view.c`、`xge_xui_popup.c` 编译入口。

## 当前限制

- `ScrollFrame` 已可作为 C 层基础设施使用，但不是 XSON 业务控件入口。
- `ScrollView` 和 `Popup` 已恢复，旧 `VirtualView` / List 系列控件仍处于隔离状态。
- ComboBox、ColorPicker、Menu 等弹层控件继续断链，恢复时必须接入新的 Popup/ScrollView 路径。

## 隔离范围

当前隔离控件：

- `TextEdit`
- `ColorPicker`
- `ComboBox`
- `Menu`
- `VirtualList`
- `ListView`
- `TreeView`
- `TableView`
- `PropertyGrid`

这些控件旧实现不参与编译。旧源码不作为新实现的边界依据。

## 实现顺序

1. `ScrollModel`
2. `ScrollBar` 对接检查
3. `ScrollFrame`
4. `ScrollView`
5. `Popup`
6. `Menu`
7. `ComboBox`
8. `ColorPicker`
9. `VirtualView`
10. `ListView`
11. `TreeView`
12. `TableView`
13. `PropertyGrid`
14. `TextEdit`

## 验收标准

- 内容不能绘制到滚动条区域。
- 文字不能溢出控件本体裁剪范围。
- 滚动条 thumb 拖拽必须跟手。
- 滚轮支持纵向、横向和双向配置。
- 内容拖拽默认关闭。
- ScrollView 和 Popup 不重复实现滚动偏移算法。
- Popup 内容大于窗口时，content size 不缩放，viewport 使用 ScrollView 显示滚动条。
- XSON 范例恢复后，`xui_*` 与 `xui_*_xson` 成对存在。
