# XUI TableGrid Spec

本 spec 用于跟踪 `TableGrid` 重构。`TableGrid` 是 `TableView` 之上的可编辑表格层，不保留旧 TableGrid 兼容。

## 设计状态

- [x] 明确 `TableGrid` 不是电子表格。
- [x] 明确不支持公式、范围选择、范围复制粘贴和撤销栈。
- [x] 明确 `TableGrid` 复用 `TableView`，不重复实现表头、滚动、选择、合并单元格和绘制基础设施。
- [x] 明确数据所有权在业务层，TableGrid 通过 adapter 读，通过 set 回调写。
- [x] 明确第一阶段编辑器映射：text、int、float、bool、picker/file/image/custom。
- [x] 明确标准编辑器配置回调：高级编辑器不污染 `TableViewCell` 渲染字段，通过 `editor config` 按 cell 提供 enum/date/color 等配置。
- [x] 明确 `textarea` 使用摘要输入 + popup TextEdit。
- [x] 明确 date/time/datetime、enum、color 后续接入现有 DatePicker、ComboBox、ColorPicker。
- [x] 明确当前 `quick edit` 和未来 `immediate` 的区别。
- [x] 新增 `docs/xui/tablegrid.md`。
- [x] 新增 `docs/xui/tablegrid-spec.md`。

## 已实现任务

- [x] 新增 `xge_xui_table_grid_t` 数据结构。
- [x] 新增 TableGrid C API。
- [x] TableGrid 初始化时内部初始化 `TableView`。
- [x] TableGrid 对外暴露 `xgeXuiTableGridGetTableView`，允许继续配置 TableView 能力。
- [x] 支持 columns、rows、adapter 转发到 TableView。
- [x] 支持 `set` 回调写回业务模型。
- [x] 支持 `validate` 回调。
- [x] 支持 `change` 回调。
- [x] 支持 `editor` 回调，供 picker/custom 打开外部编辑器。
- [x] 支持 display/edit 模式：双击或键盘确认打开编辑器。
- [x] 支持 quick edit 模式：单击打开编辑器。
- [x] 支持 Input 覆盖编辑器定位到当前单元格。
- [x] Input 覆盖编辑器裁剪到 TableView viewport。
- [x] 支持 Enter 提交。
- [x] 支持 Escape 取消。
- [x] 支持焦点离开提交。
- [x] 支持提交失败保持编辑器打开并显示错误态。
- [x] 支持 text 单元格编辑。
- [x] 支持 int / float 单元格轻量过滤和提交校验。
- [x] 支持 bool 单元格激活切换。
- [x] 支持 picker/file/image/custom 业务回调。
- [x] 支持 picker 标准视觉：普通单元格文本 + 右侧 `...` 操作按钮；激活后只走 editor 回调，不创建输入框覆盖层。
- [x] 支持 TableView 默认类型显示：bool 勾选框、color 色块、textarea 摘要、picker/file/image 操作按钮。
- [x] 支持 `file` / `image` 语义类型：基于 picker 能力提供预设口径。
- [x] 支持 `editor config` 回调，提供 enum items、color palette、date/time 格式和限制配置。
- [x] 支持 `enum` 标准编辑器：复用 ComboBox。
- [x] 支持 `color` 标准编辑器：复用 ColorPicker。
- [x] 支持 `date` / `time` / `datetime` 标准编辑器：复用 DatePicker。
- [x] 支持 `textarea` 标准编辑器：普通单元格摘要 + Popup TextEdit + OK/Cancel。
- [x] 编辑中的单元格向 TableView cell state 标记 `editing`。
- [x] 滚轮或点击其他位置前先提交当前编辑。
- [x] 新增 `examples/xui_tablegrid`。
- [x] XSON `tableGrid` 类型。
- [x] `examples/xui_tablegrid_xson`。

## 待实现任务

- [ ] `XGE_XUI_TABLE_GRID_EDIT_IMMEDIATE`：直接渲染可见区域表单控件，使用编辑器池复用可见单元格控件。
- [ ] date/time/datetime 的 range 语义是否进入 TableGrid，需要等 Form/Binder 口径统一后再定。
- [ ] `custom` 编辑器生命周期：允许业务层提供 widget、布局、提交、取消和销毁逻辑。
- [ ] 表单系统完成后，复核 TableGrid 与未来 Form/Binder 的值提交口径。

## 验收标准

- TableGrid 不能重写 TableView 的滚动、表头、选择和绘制基础逻辑。
- 编辑器定位必须使用 TableView 暴露的 cell rect 和 viewport 能力。
- 编辑器不能绘制到滚动条区域。
- 校验失败必须保持编辑器打开。
- 取消编辑不能触发 set/change。
- 布尔编辑必须是一次激活一次提交，不留下隐藏编辑状态。
- picker/custom 回调不能破坏当前选择和滚动状态。
- 大表数据仍然必须通过 adapter 虚拟读取，不能为每个单元格创建 widget。
