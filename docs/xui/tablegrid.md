# XUI TableGrid

`TableGrid` 是可编辑表格控件。它不重新实现表格几何、表头、滚动、裁剪、选择、合并单元格和自定义渲染，这些能力全部复用 `TableView`；`TableGrid` 只在 `TableView` 之上增加编辑器生命周期、校验、提交和取消。

这不是电子表格控件，不做公式、范围选择、范围复制粘贴和撤销栈。

## 职责边界

- `TableView`：静态表格基础设施，负责显示和交互骨架。
- `TableGrid`：负责编辑器打开、定位、提交、取消、校验和变化通知。
- 业务层：持有真实数据，负责在 `set` 回调中写回模型。

`TableGrid` 已落地轻量编辑层和常用标准表单编辑器：文本、整数、浮点、布尔、textarea、enum、color、date/time/datetime、picker/file/image/custom。true immediate 模式继续按 spec 追加，但不改变 TableGrid 的基础边界。

## 编辑模型

当前实现支持两种激活模式：

- `XGE_XUI_TABLE_GRID_EDIT_DISPLAY`：默认展示模式，双击或键盘确认进入编辑。
- `XGE_XUI_TABLE_GRID_EDIT_QUICK`：快速编辑模式，单击单元格后立即打开当前单元格编辑器。

`XGE_XUI_TABLE_GRID_EDIT_IMMEDIATE` 被保留给后续“可见区域直接渲染表单控件”的模式。这个模式比 quick edit 重得多，需要按可见单元格复用编辑器池，不能用每个单元格创建 widget 的方式实现。

## 编辑器映射

第一阶段已实现：

- `text`：使用 `Input` 覆盖当前单元格。
- `int` / `float`：使用 `Input` 覆盖当前单元格，内置轻量数字过滤，提交前可走业务校验。
- `bool`：激活时直接在 `true` / `false` 之间切换并提交。
- `textarea`：使用只读摘要 `Input` 覆盖当前单元格，trailing button 打开 `Popup + TextEdit`，OK 后写回摘要/全文字符串，Cancel 仅关闭 popup。
- `enum`：复用 `ComboBox` 覆盖当前单元格。选项、禁用项和默认选中项由 `editor config` 回调提供；没有配置时退回普通 Input 或业务自定义 editor。
- `color`：复用 `ColorPicker` 覆盖当前单元格，可通过 `editor config` 配置 palette 和 alpha；提交值使用 hex 文本。
- `date` / `time` / `datetime`：复用 `DatePicker` 覆盖当前单元格，可通过 `editor config` 配置格式、秒显示、min/max 和默认跨度。
- `picker` / `file` / `image`：显示标准 Input + `...` trailing decoration，点击输入区、点击 `...` 或键盘 Enter 都会调用 `xge_xui_table_grid_editor_proc`，由业务层打开文件、图片或对象选择器。
- `custom`：调用 `xge_xui_table_grid_editor_proc`，由业务层接管自定义编辑器。

`TableGrid` 不把 enum 选项、日期格式、颜色 palette 等编辑器元数据塞进 `TableViewCell`。这些配置属于编辑器生命周期，使用 `xgeXuiTableGridSetEditorConfig` 按 row/column/type 动态提供。

## 提交流程

1. 用户双击、按 Enter/Space 或 quick edit 单击单元格。
2. TableGrid 通过 TableView 查询单元格矩形，确保单元格可见。
3. 对 Input 型编辑器，在单元格内显示覆盖编辑器并设置焦点。
4. 按 Enter、焦点离开或点击其他单元格时提交。
5. 按 Escape 取消。
6. 提交时先调用 validate；校验失败时编辑器保持打开并进入错误态。
7. 校验通过且值变化时，调用 set 写回业务模型，再调用 change 通知。

布尔单元格不显示覆盖编辑器，激活即切换并提交。

## C API

```c
int xgeXuiTableGridInit(xge_xui_table_grid grid, xge_xui_context context, xge_xui_widget widget);
void xgeXuiTableGridUnit(xge_xui_table_grid grid);

xge_xui_table_view xgeXuiTableGridGetTableView(xge_xui_table_grid grid);

void xgeXuiTableGridSetColumns(xge_xui_table_grid grid, const xge_xui_table_view_column_t* columns, int count);
void xgeXuiTableGridSetRows(xge_xui_table_grid grid, const xge_xui_table_view_row_t* rows, int count);
void xgeXuiTableGridSetAdapter(
    xge_xui_table_grid grid,
    xge_xui_table_view_count_proc count,
    xge_xui_table_view_cell_proc cell,
    xge_xui_table_grid_set_proc set,
    void* user);

void xgeXuiTableGridSetValidate(xge_xui_table_grid grid, xge_xui_table_grid_validate_proc proc, void* user);
void xgeXuiTableGridSetChange(xge_xui_table_grid grid, xge_xui_table_grid_change_proc proc, void* user);
void xgeXuiTableGridSetEditor(xge_xui_table_grid grid, xge_xui_table_grid_editor_proc proc, void* user);
void xgeXuiTableGridSetEditorConfig(xge_xui_table_grid grid, xge_xui_table_grid_editor_config_proc proc, void* user);

void xgeXuiTableGridSetEditMode(xge_xui_table_grid grid, int mode);
int xgeXuiTableGridGetEditMode(xge_xui_table_grid grid);

int xgeXuiTableGridBeginEdit(xge_xui_table_grid grid, int row, int column);
int xgeXuiTableGridEndEdit(xge_xui_table_grid grid, int commit);
int xgeXuiTableGridIsEditing(xge_xui_table_grid grid);
void xgeXuiTableGridGetEditingCell(xge_xui_table_grid grid, int* row, int* column);
```

常用 TableView 能力通过 `xgeXuiTableGridGetTableView` 继续使用，例如自定义 renderer、列宽拖动回调、排序回调、hover 回调、selection、ensure visible 和滚动条模式。

## XSON

XSON 使用 `type: "tableGrid"`。第一阶段支持和 `tableView` 一致的静态表格属性：

- `columns` / `rows` / `cells` / `merges`
- `defaultColumnWidth` / `defaultRowHeight` / `headerHeight`
- `selectionMode` / `selectedRow` / `selectedCell`
- `scrollX` / `scrollY` / `scrollbarMode`
- `editMode`: `display` 或 `quick`
- `backgroundColor` / `headerColor` / `headerTextColor` / `rowColor` / `selectedColor` / `gridColor` / `textColor` / `disabledTextColor` / `barColor` / `thumbColor` / `focusRingColor`

XSON 静态数据会接入 TableGrid 的最小 set 回写：text/int/float/bool 编辑提交后会更新页面内静态 cell 文本并标记 dirty。业务 model/binder 回写、标准 picker/date/enum/color 编辑器和 true immediate 模式仍在后续任务中。

`columns[].type` 支持 `text`、`int`、`float` / `number`、`bool` / `boolean`、`textarea`、`date`、`time`、`datetime` / `dateTime`、`enum`、`color`、`picker`、`file`、`image`、`custom`。当前内建编辑器已覆盖 `text`、`int`、`float`、`bool`、`textarea`、`enum`、`color`、`date`、`time`、`datetime`、`picker`、`file`、`image`；`custom` 仍由业务 editor 接管。

## 范例

- `examples/xui_tablegrid`
- `examples/xui_tablegrid_xson`

当前范例覆盖：

- display/edit 模式。
- quick edit 模式。
- 文本编辑。
- int/float 校验与错误态。
- bool 直接切换。
- textarea / enum / color / date / time 标准编辑器。
- picker/custom 回调。
- disabled / invalid / dirty 状态显示。
- 大表横纵滚动。
- 复用 TableView 的自定义单元格 renderer。
