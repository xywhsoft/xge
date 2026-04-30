# XUI Numeric Input Lab

`examples/xui_numeric_input_lab` 覆盖 `NumericInput` 的数值解析、范围限制、步进按钮、键盘调整、格式化和错误态。

## 覆盖 API

- `xgeXuiNumericInputInit`
- `xgeXuiNumericInputSetInteger`
- `xgeXuiNumericInputSetRange`
- `xgeXuiNumericInputSetStep`
- `xgeXuiNumericInputSetValue`
- `xgeXuiNumericInputSetFormatter`
- `xgeXuiNumericInputSetChange`
- `xgeXuiNumericInputGetValue`
- `xgeXuiNumericInputEvent`

## 构建和运行

```bat
examples\xui_numeric_input_lab\build.bat
build\xge_xui_numeric_input_lab.exe --frames 5
```

## 自动检查点

- `init=1`：整数输入和浮点输入完成初始化，默认值和格式化文本正确。
- `keyboard=1`：聚焦后键盘调整能按 step 修改数值。
- `spinner=1`：上下按钮点击能修改数值并触发回调。
- `error=1`：非法文本会进入错误态。
- `blur=1`：失焦提交合法文本后清除错误态并更新数值。

## 通过标准

程序自动退出，并打印 `xui-numeric-input-lab final-summary`，其中 `init`、`keyboard`、`spinner`、`error`、`blur` 均为 `1`。

