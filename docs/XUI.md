# XUI 使用指南

XUI 是 XGE 附带的保留式 UI 库。它提供 context、控件树、布局、输入事件、文本编辑、弹出层、窗口和标准控件。应用从 `xui.h` 引入 API，并先初始化 XGE。

## 工作方式

1. 以 `xgeInit` 创建窗口和图形运行时。
2. 创建 XUI context，并为它提供 XGE proxy/host。
3. 创建根控件与标准控件，按照父子关系组成控件树。
4. 每帧把 XGE 输入送入 XUI，执行 update/layout/paint，并在状态变化后请求刷新。
5. 销毁 context 前，先停止向它派发事件和绘制请求。

XUI 的精确初始化顺序、结构体字段和回调签名以 `xui.h` 与 `examples/xui_proxy_*` 为准；这些范例同时演示 proxy、字体、纹理与输入桥接。

## 控件选择

| 类别 | 代表控件 |
| --- | --- |
| 基础内容 | Label、Button、Image、Hyperlink、Separator、Progress。 |
| 输入和编辑 | Input、NumericInput、TextEdit、CodeEdit、TagInput、ColorPicker、DatePicker。 |
| 容器和布局 | Panel、Page、Canvas、ScrollView、SplitLayout、Tabs、Accordion、Window、DockPanel。 |
| 数据展示 | ListView、TreeView、TableView、TableGrid、PropertyGrid、Chart、TimelineView。 |
| 弹出与反馈 | Popup、Menu、MenuBar、ComboBox、Cascader、MsgBox、MsgTip、Toast、FileDialog。 |
| 专用控件 | FlowGraph、Workflow、Terminal、QRCode、VirtualJoystick、InventoryGrid。 |

每种控件都有同名的 `examples/xui_*` 可运行范例。范例比开发期设计文档更适合作为组合控件、样式和事件处理的参考。

Terminal 的缓冲区、VT/TUI 输入、ConPTY、查找和性能约定见 [XUI Terminal 使用指南](TERMINAL.md)。

## 布局、事件与绘制

- 使用公开 layout、style 和 widget API 描述控件尺寸、间距、对齐和可见性；不要直接写入私有控件数据。
- 事件通过 context 分发，并遵守焦点、捕获、弹出层与模态层的规则。输入控件需使用 XUI 的文本、剪贴板和 IME 入口，而非绕过控件状态直接修改内部文本。
- Window、Popup、Menu、Toast 等需要浮层语义的组件应通过其专用创建 API 使用，而不是作为普通 child 模拟。
- 自绘、proxy 和 surface 接口适合接入现有图形资源；调用者仍负责遵守 `xui.h` 中的资源所有权约定。

## 建议的入门顺序

1. `examples/xui_proxy_surface`：最小 XGE/XUI 图形桥接。
2. `examples/xui_button`、`examples/xui_layout`：基础控件和布局。
3. `examples/xui_input`、`examples/xui_textedit`：输入、剪贴板和 IME。
4. `examples/xui_window`、`examples/xui_dockpanel`：浮层和桌面式布局。
5. 选择业务控件范例并结合 `test_xui/` 的相应回归测试验证集成。
