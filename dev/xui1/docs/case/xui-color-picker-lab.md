# XUI Color Picker Lab

`examples/xui_color_picker_lab` 覆盖 `ColorPicker` 的颜色值、hex 文本、RGBA 字段、palette 和变更回调。

## 覆盖 API

- `xgeXuiColorPickerInit`
- `xgeXuiColorPickerSetChange`
- `xgeXuiColorPickerSetPalette`
- `xgeXuiColorPickerSetHex`
- `xgeXuiColorPickerGetPaletteCount`
- `xgeXuiColorPickerGetColor`
- `xgeXuiColorPickerGetHex`

## 构建和运行

```bat
examples\xui_color_picker_lab\build.bat
build\xge_xui_color_picker_lab.exe --frames 5
```

## 自动检查点

- `init=1`：palette 数量正确。
- `hex=1`：hex 文本可解析成 RGBA 值，读回文本保持一致。
- `palette=1`：点击 palette swatch 可更新颜色并触发回调。
- `changes`：输出颜色变更回调次数。

## 通过标准

程序自动退出，并打印 `xui-color-picker-lab final-summary`，其中 `init`、`hex`、`palette` 均为 `1`。

