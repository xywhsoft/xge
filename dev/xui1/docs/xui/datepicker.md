# XUI DatePicker

`DatePicker` 是表单化日期时间选择控件。底层只有一个控件实现，通过 `mode` 组合日期、时间和范围选择能力；未来表单层可以提供 `TimePicker`、`DatePicker`、`DateTimePicker`、`TimeRangePicker`、`DateRangePicker`、`DateTimeRangePicker` 等语义入口。

## 设计口径

- 对外值读写统一使用 `xtime`。
- `xtime == 0` 是合法时间值，不能作为空值。
- 默认 `nullable = false`，控件必须持有有效值；开启 `nullable` 后才允许清空。
- 输入框只负责展示和打开 popup，不支持手输，避免日期解析、错误态和 IME 逻辑进入第一版。
- 范围选择不允许提交半开值；popup 内可以临时只选一端，提交时按默认跨度补齐。
- `min/max` 使用 `xtime`。`time` / `timeRange` 模式只比较一天内的 `xrtTimePart()`，日期类模式比较完整 `xtime`。
- `showSecond = false` 时 UI 不显示秒，提交时秒固定为 `0`。
- 日期计算全部使用 XRT time 库，控件内部不维护独立闰年、月份天数和星期计算逻辑。

## Mode

| mode | 值模型 | popup 组成 |
| --- | --- | --- |
| `date` | `value` | `CalendarPanel + Footer` |
| `time` | `value` | `TimePanel + Footer` |
| `dateTime` | `value` | `CalendarPanel + TimePanel + Footer` |
| `dateRange` | `startValue/endValue` | `CalendarPanel(start) + CalendarPanel(end) + RangeController + Footer` |
| `timeRange` | `startValue/endValue` | `TimePanel(start) + TimePanel(end) + RangeController + Footer` |
| `dateTimeRange` | `startValue/endValue` | 双 `CalendarPanel`，各自下方带 `TimePanel`，再加 `Footer` |

## 默认值

当 `nullable = false` 且用户没有显式设置值时：

- `date`：今天。
- `time`：当前时间。
- `dateTime`：当前日期时间。
- `dateRange`：今天到明天，默认跨度 1 天。
- `timeRange`：当前时间到当前时间 + 默认时间差，默认 4 小时。
- `dateTimeRange`：当前日期时间到当前日期时间 + 默认时间差，默认 4 小时。

默认跨度可通过 API / XSON 修改。

## Range 规则

- 第一次选择设置临时 anchor。
- 第二次选择补齐另一端。
- 如果 end 小于 start，自动交换。
- 用户只选一端就提交时，根据 `defaultRangeSpan` 自动补齐另一端。
- `nullable = false` 时隐藏 Clear。
- `nullable = true` 时 Clear 清空单值或范围值，并触发 `onClear`。

## Popup

Popup 使用通用 `Popup + ScrollFrame` 基础设施，不在 DatePicker 内重复处理弹层坐标、回退、窗口溢出和滚动条。

DatePicker 的六种 mode 复用 context 级共享 popup host：每个 `xge_xui_context` 只保留 6 个 DatePicker popup panel，控件打开时绑定当前 DatePicker、同步 draft 和布局，关闭时解除绑定。单个 DatePicker 实例不持有独立 popup 副本，避免每个表单字段都创建一整套弹层控件。

弹层内复用标准控件：

- `Button`：上一月、下一月、Today/Now、Clear、Cancel、OK。
- `Input`：年份编辑。
- `ComboBox`：月份选择。
- `NumericInput`：时、分、秒输入和 spinner。
- 日历日期网格仍由 DatePicker 自绘，因为当前 core 里还没有独立 Calendar 控件。

日期类 header：

- 上一月按钮。
- 年份字段：标准 Input，变更后同步当前面板年份。
- 月份字段：标准 ComboBox，选项为 1-12 月。
- 下一月按钮。

日历网格固定 6 行，避免 popup 高度随月份变化跳动。`firstDayOfWeek` 默认 `0`，即周日开始；可设置为 `1` 使用周一开始。

时间面板：

- 内部按秒存储。
- `showSecond = false` 时只显示时、分，秒提交为 0。
- 每个字段提供上下调节按钮，滚轮在字段上调整对应值。

Footer：

- `Today` / `Now`：按当前 mode 写入当前日期或时间。
- `Clear`：仅 `nullable = true` 时显示。
- `Cancel` / Esc / 外部关闭：放弃 draft，触发 `onCancel`。
- `OK` / Enter：提交 draft，触发 `onCommit`，值变化时触发 `onChange`。

## API

```c
int xgeXuiDatePickerInit(xge_xui_date_picker picker, xge_xui_context context, xge_xui_widget widget, xge_font font);
void xgeXuiDatePickerUnit(xge_xui_date_picker picker);

void xgeXuiDatePickerSetMode(xge_xui_date_picker picker, int mode);
int xgeXuiDatePickerGetMode(xge_xui_date_picker picker);

void xgeXuiDatePickerSetNullable(xge_xui_date_picker picker, int nullable);
int xgeXuiDatePickerGetNullable(xge_xui_date_picker picker);

void xgeXuiDatePickerSetValue(xge_xui_date_picker picker, xtime value);
xtime xgeXuiDatePickerGetValue(xge_xui_date_picker picker);
int xgeXuiDatePickerHasValue(xge_xui_date_picker picker);
void xgeXuiDatePickerClearValue(xge_xui_date_picker picker);

void xgeXuiDatePickerSetRangeValue(xge_xui_date_picker picker, xtime start, xtime end);
void xgeXuiDatePickerGetRangeValue(xge_xui_date_picker picker, xtime* start, xtime* end);
int xgeXuiDatePickerHasRangeValue(xge_xui_date_picker picker);

void xgeXuiDatePickerSetLimits(xge_xui_date_picker picker, xtime min, xtime max);
void xgeXuiDatePickerClearLimits(xge_xui_date_picker picker);

void xgeXuiDatePickerSetFormat(xge_xui_date_picker picker, const char* format);
const char* xgeXuiDatePickerGetFormat(xge_xui_date_picker picker);
void xgeXuiDatePickerSetRangeSeparator(xge_xui_date_picker picker, const char* separator);

void xgeXuiDatePickerSetShowSecond(xge_xui_date_picker picker, int showSecond);
void xgeXuiDatePickerSetFirstDayOfWeek(xge_xui_date_picker picker, int firstDayOfWeek);
void xgeXuiDatePickerSetDefaultRangeSpan(xge_xui_date_picker picker, xtime span);

void xgeXuiDatePickerSetChanging(xge_xui_date_picker picker, xge_xui_date_proc proc, void* user);
void xgeXuiDatePickerSetChange(xge_xui_date_picker picker, xge_xui_date_proc proc, void* user);
void xgeXuiDatePickerSetCommit(xge_xui_date_picker picker, xge_xui_date_proc proc, void* user);
void xgeXuiDatePickerSetCancel(xge_xui_date_picker picker, xge_xui_date_proc proc, void* user);
void xgeXuiDatePickerSetClear(xge_xui_date_picker picker, xge_xui_date_proc proc, void* user);
int xgeXuiDatePickerIsPopupOpen(xge_xui_date_picker picker);
```

`xge_xui_date_proc` 使用统一签名：

```c
void proc(xge_xui_widget widget, xtime start, xtime end, int mode, void* user);
```

单值模式只使用 `start`，范围模式使用 `start/end`。

## XSON

XSON 支持直接存储 `xtime`。文档示例使用整数值表达：

```json
{
  "type": "datePicker",
  "mode": "dateTimeRange",
  "rect": [40, 40, 260, 30],
  "startValue": 63930618000,
  "endValue": 63930632400,
  "min": 63928026000,
  "max": 63956509199,
  "nullable": false,
  "format": "yyyy-mm-dd hh:nn:ss",
  "rangeSeparator": " - ",
  "showSecond": true,
  "firstDayOfWeek": 0,
  "defaultRangeSpan": 14400
}
```

当 XSON 设置 `nullable: true` 且没有提供 `value` / `startValue` / `endValue` 时，控件初始为空；`nullable: false` 或未设置时按默认值规则生成有效值。

## 测试覆盖

专属范例：

- `xui_datepicker`
- `xui_datepicker_xson`

覆盖项：

- 六种 mode。
- 默认值、显式值、范围自动交换。
- `nullable` 与 Clear。
- `showSecond` 开关。
- `firstDayOfWeek`。
- `min/max`。
- `onChanging`、`onChange`、`onCommit`、`onCancel`、`onClear`。
- popup 打开、取消、提交、清空。
