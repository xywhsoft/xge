# XUI 使用指南

XUI 是 XGE 附带的保留式 UI 库。它提供 context、控件树、布局、输入事件、文本编辑、弹出层、窗口和标准控件。应用从 `xui.h` 引入 API，并先初始化 XGE。

## 工作方式

1. 以 `xgeInit` 创建窗口和图形运行时。
2. 创建 XUI context，并为它提供 XGE proxy/host。
3. 创建根控件与标准控件，按照父子关系组成控件树。
4. 每帧把 XGE 输入送入 XUI，执行 update/layout/paint，并在状态变化后请求刷新。
5. 销毁 context 前，先停止向它派发事件和绘制请求。

XUI 的精确初始化顺序、结构体字段和回调签名以 `xui.h` 与 `examples/xui/xui_proxy_*` 为准；这些范例同时演示 proxy、字体、纹理与输入桥接。

## 控件选择

| 类别 | 代表控件 |
| --- | --- |
| 基础内容 | Label、Button、Image、Hyperlink、Separator、Progress。 |
| 输入和编辑 | Input、NumericInput、TextEdit、CodeEdit、TagInput、ColorPicker、DatePicker。 |
| 容器和布局 | Panel、Page、Canvas、ScrollView、SplitLayout、Tabs、Accordion、Window、DockPanel。 |
| 数据展示 | ListView、TreeView、TableView、TableGrid、PropertyGrid、Chart、TimelineView。 |
| 弹出与反馈 | Popup、Menu、MenuBar、ComboBox、Cascader、MsgBox、MsgTip、Toast、FileDialog。 |
| 专用控件 | FlowGraph、Workflow、Terminal、QRCode、VirtualJoystick、InventoryGrid。 |

每种控件都有同名的 `examples/xui/xui_*` 可运行范例。范例比开发期设计文档更适合作为组合控件、样式和事件处理的参考。

Terminal 的缓冲区、VT/TUI 输入、ConPTY、查找和性能约定见 [XUI Terminal 使用指南](TERMINAL.md)。

## 布局、事件与绘制

- 使用公开 layout、style 和 widget API 描述控件尺寸、间距、对齐和可见性；不要直接写入私有控件数据。
- `ROW`/`COLUMN` 使用刚性 Stack 模型：固定和内容尺寸不因父空间不足而压缩，`FILL` 只分配正的剩余空间，空间为负时该 Fill 项在本次布局中折叠。基础容器默认裁剪溢出内容。
- `FLOW` 是内容排版模型，负责换行，并仅按显式 `grow`/`shrink` 权重进行行内伸缩；需要弹性变形时应使用 Flow，而不是依赖 Row/Column 改写控件申请尺寸。
- 滚动控件的外部期望尺寸由 viewport 决定，滚动内容范围只参与内部滚动模型和滚动条计算。`ScrollFrame`、`ScrollView` 与 `Canvas` 可通过 `SetViewportHint` 设置内容尺寸未被父布局约束时的默认视口；自定义复合控件可用 `xuiWidgetSetMeasureContainment` 按轴阻止内部子树尺寸向父布局传播。
- `MessageList` 的消息总高度属于内部虚拟内容范围，不参与父布局测量；追加、折叠或清空消息不会改变 MessageList 的外部申请尺寸。
- 事件通过 context 分发，并遵守焦点、捕获、弹出层与模态层的规则。输入控件需使用 XUI 的文本、剪贴板和 IME 入口，而非绕过控件状态直接修改内部文本。
- Window、Popup、Menu、Toast 等需要浮层语义的组件应通过其专用创建 API 使用，而不是作为普通 child 模拟。
- 自绘、proxy 和 surface 接口适合接入现有图形资源；调用者仍负责遵守 `xui.h` 中的资源所有权约定。

### 测量缓存与交互失效

Label 在控件内保留最近四组约束的尺寸结果，避免父布局和实际排版宽度交替时重复整形。缓存只保存尺寸，不持有额外的文本布局或绘制表面；文本、字体、换行、行段间距、DPI 或显式测量失效都会重新测量，失败结果不会进入缓存。颜色变化只需要重绘。若自定义字体后端在句柄不变时修改字体度量，应调用 `xuiWidgetInvalidate(label, XUI_WIDGET_DIRTY_LAYOUT)`。

CodeEdit 的光标和选择更新刷新绘制及 IME 锚点；文本编辑和实际几何变化仍通过原有路径触发布局。拥有布局回调的容器平移等尺寸子控件时，不把排版位置重新当作测量输入，滚动仍更新世界坐标、命中测试和损伤区域。

`tests/xui/build_help_interaction_test.bat` 验证多约束缓存、字体与 DPI 失效、失败重试、回调重入、滚动、光标、IME 和宽度重排；应与 Label、CodeEdit、ScrollView 及布局测试一并运行。

## 建议的入门顺序

1. `examples/xui/xui_proxy_surface`：最小 XGE/XUI 图形桥接。
2. `examples/xui/xui_button`、`examples/xui/xui_layout`：基础控件和布局。
3. `examples/xui/xui_input`、`examples/xui/xui_textedit`：输入、剪贴板和 IME。
4. `examples/xui/xui_window`、`examples/xui/xui_dockpanel`：浮层和桌面式布局。
5. 选择业务控件范例并结合 `tests/xui/` 的相应回归测试验证集成。
