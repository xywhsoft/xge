# XUI 单元格编辑器与属性表增强 SPEC

本文档跟踪 `PropertyGrid` 向通用单元格渲染/编辑框架演进的设计与实现进度。短期先在 `PropertyGrid` 中验证，稳定后再抽出给 `TableView` 复用。

## 进度维护规则

状态含义：

- `[ ]` 未开始
- `[~]` 进行中
- `[x]` 已完成，并通过必要验证
- `[!]` 放弃、替代或暂缓，必须说明原因

完成口径：

- 设计任务：文档写清边界、阶段和兼容策略。
- 渲染任务：至少覆盖普通、选中、只读、错误/变更标记并不破坏现有绘制。
- 编辑任务：至少覆盖鼠标、键盘、焦点丢失、提交/取消路径。
- Overlay 任务：覆盖打开/关闭、焦点恢复、外部点击、Escape。
- 复用任务：确认 `PropertyGrid` 与 `TableView` 的共同模型，而不是只复制代码。

## 总体目标

- [~] 在 `PropertyGrid` 内建立可演进的 cell renderer/editor 设计。
- [ ] 抽象通用 `xge_xui_cell_editor` / `xge_xui_cell_renderer` 内部模型。
- [ ] 让 `TableView` 逐步支持同一套单元格格式和编辑器。
- [ ] 在 XSON 中声明属性表/表格的编辑器类型、按钮和选择器元数据。

## 阶段 A：PropertyGrid 现有编辑器补强

- [x] 文本/数字编辑器改为内嵌真实 `xgeXuiInput`，支持选择、复制粘贴和右键菜单。
- [x] `BOOL` 编辑器增加 checkbox 渲染，避免只显示 `true/false` 文本。
- [x] `BOOL` 支持键盘 Space/Enter 切换时与 checkbox 语义一致。
- [ ] `ENUM` 下拉编辑器补齐打开状态箭头、关闭策略和键盘操作细节。
- [ ] 颜色编辑器显示色块，并为后续颜色选择器弹窗预留动作入口。

## 阶段 B：尾部按钮与动作事件

- [x] 属性项增加尾部按钮元数据：按钮文本、action id、可用状态。
- [x] 增加 `xgeXuiPropertyGridSetActionButton(...)` API。
- [x] 增加 `procAction(widget, propIndex, actionId, user)` 回调。
- [ ] 尾部按钮支持 `...` 文件选择、资源选择、对象引用选择等通用入口。
- [x] 绘制时保证 value 文本避让尾部按钮，按钮不覆盖标记点。

## 阶段 C：多行输入弹窗

- [ ] 增加 `XGE_XUI_PROPERTY_GRID_EDITOR_MULTILINE`。
- [ ] 复用 `xgeXuiDialog + xgeXuiTextEdit` 实现大文本编辑弹窗。
- [ ] 支持提交/取消、外部关闭策略和焦点恢复。
- [ ] 支持只读多行查看模式。
- [ ] 未来可与 JSON、脚本、备注类属性共用。

## 阶段 D：选择器族

- [ ] 文件/目录选择器：路径输入 + 尾部按钮 + 回调接入宿主选择逻辑。
- [ ] 资源选择器：选择图块、图集、对象定义等项目资源。
- [ ] 对象引用选择器：支持显示名称和值 id 分离。
- [ ] 数字范围选择器：数字输入、滑条或 spinner 的组合策略。
- [ ] 自定义选择器：允许用户自定义绘制与编辑 overlay。

## 阶段 E：抽象到通用 Cell 系统

- [ ] 定义 `cell kind`：text、number、bool、enum、color、multiline、file、object、custom。
- [ ] 定义 `cell state`：normal、hover、selected、editing、readonly、disabled、error、dirty。
- [ ] 定义 renderer 输入：rect、value、display text、state、metadata。
- [ ] 定义 editor 生命周期：begin、layout、event、commit、cancel、end。
- [ ] `PropertyGrid` 改为两列表格的特化包装。
- [ ] `TableView` 支持列格式、单元格编辑器和提交回调。

## 设计约束

- `PropertyGrid` 的公开 API 需要保持向后兼容；新增能力优先用可选 setter。
- 现阶段不要一次性引入过大的公共抽象，先在 `PropertyGrid` 内验证真实交互。
- 单元格编辑器应复用已有 XUI 控件能力，例如 `Input`、`TextEdit`、`Popup`、`Dialog`、`ListView`。
- 表格和属性表最终应共享编辑器模型，但可以保留不同的布局、分组和显示策略。

## 验证记录

- [x] `xui_property_grid_lab` 覆盖 bool checkbox 渲染和点击切换。
- [x] `xui_property_grid_lab` 覆盖尾部 action button 回调。
- [x] mapedit 属性表启动与基础编辑 smoke 通过。
