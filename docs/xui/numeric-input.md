# XUI NumericInput

`NumericInput` 是数值录入控件，用于整数、浮点数、范围限制、步进调整和常见参数编辑场景。它复用 `Input` 的文本编辑、右对齐、错误态、禁用/只读、焦点和基础事件能力，在右侧提供内建上下调节按钮。

## 设计口径

- `Input` 只负责通用文本输入；数值语义、范围、step、spinner 和滚轮调整由 `NumericInput` 承担。
- 默认文本右对齐，便于数字列和带单位场景阅读。
- 默认显示右侧上下 spinner。spinner 使用真实按钮态：normal、hover、active、disabled，按下时只改变视觉状态，释放在同一按钮上才提交 step。
- 鼠标滚轮在控件区域内调整 step；滚轮向上增加，向下减少。
- `readonly` 和 `disabled` 状态不会响应 spinner、键盘 step 或滚轮 step。
- 超出范围的 step 会被拦截；到达最小值时 down 按钮禁用，到达最大值时 up 按钮禁用。
- 非法文本在提交时进入错误态，并保留用户输入，便于修正。

## API

```c
xgeXuiNumericInputInit(&number, &xui, widget, font);
xgeXuiNumericInputSetRange(&number, 0.0f, 100.0f);
xgeXuiNumericInputSetStep(&number, 0.5f);
xgeXuiNumericInputSetPrecision(&number, 2);
xgeXuiNumericInputSetValue(&number, 42.5f);
```

常用配置：

- `xgeXuiNumericInputSetInteger`：整数模式，格式化和提交时会取整。
- `xgeXuiNumericInputSetPrecision`：浮点默认显示精度，默认 `3`，范围限制为 `0..8`。
- `xgeXuiNumericInputSetSpinnerVisible`：隐藏或显示 spinner。
- `xgeXuiNumericInputSetSpinnerWidth`：调整 spinner 区宽度，最小有效宽度为 `12px`。
- `xgeXuiNumericInputSetSpinnerColors`：设置 spinner 普通、悬停、按下、禁用、边框、图标和禁用图标颜色。
- `xgeXuiNumericInputSetFormatter`：自定义显示格式。带单位的格式更适合只读展示；如果格式化文本不是纯数字，编辑提交会进入错误态。
- `xgeXuiNumericInputSetChange`：数值真正变化时触发。

## XSON

`numericInput` 支持：

```json
{
  "type": "numericInput",
  "min": 0,
  "max": 100,
  "step": 0.5,
  "precision": 2,
  "spinner": true,
  "spinnerWidth": 24,
  "value": 42.5
}
```

同时继承 Input 的文本颜色、背景色、焦点色、占位文本、禁用、只读等基础样式字段。

## 范例

`examples/xui_numericinput` 覆盖 C API 创建与交互，`examples/xui_numericinput_xson` 覆盖 XSON 加载：

- 整数和浮点显示。
- spinner normal、hover、active、disabled 状态。
- 键盘上下键 step。
- 鼠标滚轮 step。
- 禁用、只读、边界值和隐藏 spinner。
- 非法文本提交错误态。
- spinner capture 获取和释放。
