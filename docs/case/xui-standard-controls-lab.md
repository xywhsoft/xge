# XUI Standard Controls Lab

`examples/xui_standard_controls_lab` 是标准控件总览示例，用于把基础控件、选择控件、数值控件、工具条和状态条放在同一个淡蓝色界面里做 smoke 验证。单控件的交互细节仍由各自的 `xui-*-lab` 覆盖。

## 覆盖范围

- 基础控件：`Panel`、`Label`、`Button`。
- 选择控件：`CheckBox`、`RadioGroup`、`Radio`、`Switch`。
- 数值控件：`Slider`、`Progress`、`NumericInput`。
- 工具控件：`Toolbar`、`StatusBar`。

## 构建和运行

```bat
examples\xui_standard_controls_lab\build.bat
build\xge_xui_standard_controls_lab.exe --frames 5
```

## 自动检查点

- `basic=1`：基础控件初始化成功。
- `choice=1`：CheckBox、RadioGroup、Switch 的选中状态正确。
- `value=1`：Slider、Progress、NumericInput 的数值正确。
- `toolbar=1`：Toolbar 项数量、toggle 状态和禁用项配置正确。
- `status=1`：StatusBar 项数量和进度项配置正确。

## 通过标准

程序自动退出，并打印 `xui-standard-controls-lab final-summary`，其中 `basic`、`choice`、`value`、`toolbar`、`status` 均为 `1`。

