# XUI Tooltip Owner Lab

`examples/xui_tooltip_owner_lab` 覆盖新的 tooltip 机制：tooltip 是 widget 属性，由 XUI context 内部服务负责命中、延迟、定位、显示和关闭。

## 覆盖 API

- `xgeXuiWidgetSetTooltip`
- `xgeXuiWidgetSetTooltipText`
- `xgeXuiWidgetClearTooltip`
- `xgeXuiWidgetGetTooltip`
- `xgeXuiTooltipIsOpen`
- `xgeXuiTooltipGetOwner`
- `xgeXuiTooltipGetRect`

## 构建和运行

```bat
examples\xui_tooltip_owner_lab\build.bat
build\xge_xui_tooltip_owner_lab.exe --frames 5
```

## 自动检查点

- `init=1`：owner widget 成功保存文本 tooltip 描述。
- `open=1`：鼠标移动到 owner 后，context 内部 tooltip 服务打开提示并记录 owner。
- `clear=1`：清除 owner tooltip 后，描述类型恢复为 `XGE_XUI_TOOLTIP_NONE`。
- `context=1`：鼠标按下会关闭当前 tooltip。
