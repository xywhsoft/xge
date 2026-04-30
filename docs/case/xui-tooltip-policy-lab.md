# XUI Tooltip Policy Lab

`examples/xui_tooltip_policy_lab` 覆盖 tooltip 的策略行为：文本 tooltip、自定义 tooltip、相对光标定位、尺寸测量和关闭规则。

## 覆盖 API

- `xgeXuiWidgetSetTooltip`
- `xge_xui_tooltip_desc_t`
- `xge_xui_tooltip_measure_proc`
- `xge_xui_tooltip_paint_proc`
- `xgeXuiTooltipIsOpen`
- `xgeXuiTooltipGetOwner`
- `xgeXuiTooltipGetRect`

## 运行方式

```bat
examples\xui_tooltip_policy_lab\build.bat
build\xge_xui_tooltip_policy_lab.exe --frames 5
```

## 自动检查

- `text=1`：文本 tooltip 能被 owner hover 打开并得到有效尺寸。
- `cursor=1`：光标锚点会按鼠标位置和 offset 计算位置。
- `custom=1`：自定义 tooltip 使用 `measure/paint` 回调描述尺寸和内容。
- `close=1`：滚轮、按下、触摸结束等非 hover 事件会关闭 tooltip。
