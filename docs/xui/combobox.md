# XUI ComboBox

## 设计定位

ComboBox 是单选下拉控件，用于在有限选项集中选择一个值。它由主控件、内部 Popup 和内部 ListView 组成，但外部只把它当作一个控件使用。

本轮重构目标：

- 主控件负责当前选中项展示、hover/focus/disabled/open 状态和键盘打开。
- Popup/ListView 只作为内部实现细节，不要求业务代码直接操作。
- 条目支持简单字符串数组，也支持带 `value/enabled/separator/user` 的结构化条目。
- 弹出列表支持固定高度、最大高度和自动上下弹出，避免窗口底部空间不足时被截断。
- disabled item 不可选择，不触发 `Select`。

ComboBox 不承载搜索、过滤、多选、树形选择等复杂语义；这些后续应作为独立高级控件或 ext 组件处理。

## API

```c
int xgeXuiComboBoxInit(xge_xui_combo_box combo, xge_xui_context ctx, xge_xui_widget widget);
void xgeXuiComboBoxUnit(xge_xui_combo_box combo);

void xgeXuiComboBoxSetItems(xge_xui_combo_box combo, const char** items, int count);
void xgeXuiComboBoxSetItemData(xge_xui_combo_box combo, const xge_xui_combo_box_item_t* items, int count);
void xgeXuiComboBoxSetEnabledItems(xge_xui_combo_box combo, const int* enabled, int count);

void xgeXuiComboBoxSetSelected(xge_xui_combo_box combo, int index);
int xgeXuiComboBoxGetSelected(xge_xui_combo_box combo);
void xgeXuiComboBoxSetSelectedValue(xge_xui_combo_box combo, int value);
int xgeXuiComboBoxGetSelectedValue(xge_xui_combo_box combo);

void xgeXuiComboBoxSetSelect(xge_xui_combo_box combo, xge_xui_select_proc proc, void* user);
void xgeXuiComboBoxSetFont(xge_xui_combo_box combo, xge_font font);
void xgeXuiComboBoxSetPopupHeight(xge_xui_combo_box combo, float height);
void xgeXuiComboBoxSetPopupMaxHeight(xge_xui_combo_box combo, float height);
void xgeXuiComboBoxSetPopupPlacement(xge_xui_combo_box combo, int placement);
void xgeXuiComboBoxSetMetrics(xge_xui_combo_box combo, float itemHeight, float arrowWidth);
void xgeXuiComboBoxSetColors(xge_xui_combo_box combo, uint32_t normal, uint32_t hover, uint32_t active, uint32_t focus, uint32_t disabled, uint32_t text, uint32_t popup);
void xgeXuiComboBoxSetItemColors(xge_xui_combo_box combo, uint32_t hover, uint32_t selected, uint32_t disabled, uint32_t disabledText);
int xgeXuiComboBoxIsOpen(xge_xui_combo_box combo);
```

`xgeXuiComboBoxSetDropDownHeight` 保留为 `xgeXuiComboBoxSetPopupHeight` 的兼容别名，新代码使用 `popup` 口径。

## 条目

简单场景使用字符串数组：

```c
const char* items[] = { "Small", "Normal", "Large" };

xgeXuiComboBoxSetItems(combo, items, 3);
xgeXuiComboBoxSetSelected(combo, 1);
```

需要稳定业务值、禁用项或分隔项时使用结构化条目：

```c
const xge_xui_combo_box_item_t items[] = {
	{ "Draft", 10, 1, 0, 0, NULL },
	{ "Archived", 20, 0, 0, 0, NULL },
	{ "Published", 30, 1, 0, 0, NULL }
};

xgeXuiComboBoxSetItemData(combo, items, 3);
xgeXuiComboBoxSetSelectedValue(combo, 30);
```

`SetItemData` 只保存外部数组引用，调用方负责保证数组和字符串生命周期长于 ComboBox 使用期。

## 弹出策略

弹出高度：

- `popupHeight > 0`：使用固定高度。
- `popupHeight <= 0`：按 `itemHeight * itemCount` 计算，并被 `popupMaxHeight` 限制。
- 实际高度还会被窗口可用空间夹取，避免溢出窗口。

弹出方向：

- `XGE_XUI_COMBO_POPUP_AUTO`：默认。下方空间不足且上方空间更大时向上弹出。
- `XGE_XUI_COMBO_POPUP_BOTTOM`：优先向下。
- `XGE_XUI_COMBO_POPUP_TOP`：优先向上。

## 交互

- 鼠标点击主控件打开或关闭 Popup。
- 点击 enabled item 后提交选择、关闭 Popup 并触发 `Select`。
- disabled item 被消费但不提交选择。
- 主控件聚焦时，`Enter` / `Space` / `Up` / `Down` 打开 Popup。
- Popup 复用统一 overlay policy，支持 outside click 和 Escape 关闭，并恢复焦点。

## XSON

`type: "comboBox"` 支持：

- `items`：字符串数组，或对象数组。
- `enabledItems`：布尔数组，用于简单字符串条目。
- `selected`：按索引选择。
- `selectedValue`：按结构化条目 `value` 选择。
- `value`：兼容字段。结构化条目按 value 选择，字符串条目按 index 选择。
- `popupHeight` / `dropDownHeight`：固定弹出高度。
- `popupMaxHeight`：最大弹出高度。
- `popupPlacement`：`auto` / `top` / `bottom`。
- `itemHeight`、`arrowWidth`。
- 颜色：`color` / `background`、`hoverColor`、`activeColor`、`focusColor`、`disabledColor`、`textColor`、`disabledTextColor`、`borderColor`、`arrowColor`、`popupColor`、`itemHoverColor`、`itemSelectedColor`、`itemDisabledColor`。

结构化条目字段：

```json
{
  "text": "Published",
  "value": 30,
  "enabled": true,
  "separator": false
}
```

`label` 可作为 `text` 的别名。

## 范例

- `examples/xui_combobox`：C API 范例，覆盖简单条目、结构化条目、禁用项、固定高度、自动向上弹出和 disabled 状态。
- `examples/xui_combobox_xson`：XSON 范例，验证同等配置可通过声明式页面加载。
