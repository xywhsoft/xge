# XUI Numeric Input Lab

`examples/xui_numericinput` 覆盖 `NumericInput` 的数值解析、范围限制、真实 spinner 按钮、键盘调整、滚轮调整、禁用/只读、边界状态和错误态。`examples/xui_numericinput_xson` 覆盖同一控件的 XSON 加载口径。

## 覆盖 API

- `xgeXuiNumericInputInit`
- `xgeXuiNumericInputSetInteger`
- `xgeXuiNumericInputSetRange`
- `xgeXuiNumericInputSetStep`
- `xgeXuiNumericInputSetPrecision`
- `xgeXuiNumericInputSetSpinnerVisible`
- `xgeXuiNumericInputSetSpinnerWidth`
- `xgeXuiNumericInputSetSpinnerColors`
- `xgeXuiNumericInputSetValue`
- `xgeXuiNumericInputSetChange`
- `xgeXuiNumericInputSetErrorChange`
- `xgeXuiNumericInputGetValue`
- `xgeXuiNumericInputEvent`

## 构建和运行

```bat
examples\xui_numericinput\build.bat
examples\xui_numericinput_xson\build.bat
build\xui_numericinput.exe --frames 5
build\xui_numericinput_xson.exe --frames 5
```

## 自动检查点

- `init=1`：整数输入、浮点输入、自定义 spinner 和隐藏 spinner 完成初始化。
- `keyboard=1`：聚焦后键盘调整能按 step 修改数值。
- `spinner=1`：上下按钮 MouseDown 进入按下态，MouseUp 在同一按钮释放后提交 step。
- `wheel=1`：控件区域内鼠标滚轮能按 step 修改数值。
- `error=1`：非法文本会进入错误态，并触发错误事件。
- `ro=1`：只读态不会被 spinner 修改。
- `dis=1`：禁用态不会被滚轮修改。
- `bound=1`：到达边界后对应方向按钮不可用。
- `noSpin=1`：隐藏 spinner 后右侧区域不会触发 spinner step。
- `capture=1`：spinner 按下时获取鼠标 capture，释放后自动归还。

## 通过标准

程序自动退出，并打印 `xui_numericinput final-summary`，其中 `init`、`key`、`spin`、`wheel`、`error`、`ro`、`dis`、`bound`、`noSpin`、`capture` 均为 `1`；XSON 范例打印 `xui_numericinput_xson final-summary`，其中 `create`、`layout`、`state`、`step` 均为 `1`。

