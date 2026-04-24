# XUI Visual Proc Lab

`examples/xui_visual_proc_lab` 专门补 XUI 里一批公开的 `procMeasure / procPaint / procEvent` 接口。它把 `Panel / Label / Image / Separator / Button / IconButton` 组合成一个可自动退出的页签面板，同时直接调用这些公开 proc API 做自检。

## 覆盖 API

- `xgeXuiLabelMeasure`
- `xgeXuiLabelMeasureProc`
- `xgeXuiLabelPaintProc`
- `xgeXuiImageMeasureProc`
- `xgeXuiImagePaintProc`
- `xgeXuiSeparatorPaintProc`
- `xgeXuiPanelPaintProc`
- `xgeXuiButtonEventProc`
- `xgeXuiButtonPaintProc`
- `xgeXuiIconButtonEventProc`
- `xgeXuiIconButtonPaintProc`

## 构建和运行

```bat
examples\xui_visual_proc_lab\build.bat
build\xge_xui_visual_proc_lab.exe --frames 5
```

## 自动检查点

- `label=1`：`xgeXuiLabelMeasure` 与 `xgeXuiLabelMeasureProc` 一致，且 label paint proc 已绑定。
- `image=1`：`xgeXuiImageMeasureProc` 能返回 source rect 的尺寸，image paint proc 已绑定。
- `button=1`：`xgeXuiButtonEventProc` 能处理 mouse + keyboard 激活，button paint proc 已绑定。
- `icon=1`：`xgeXuiIconButtonEventProc` 能处理 mouse + keyboard 激活，icon button paint proc 已绑定。
- `panel=1`：panel paint proc 已绑定，标题和 clip 配置已落到控件上。
- `separator=1`：separator paint proc 已绑定，方向和 thickness 设置正确。
- `paint=1`：本例已经直接调用过这些公开 paint proc 一次。

## 人工观察点

- 左侧 panel 应该有标题和深色底板。
- 左侧 image 应该是一个小的彩色图标块，不是纯色空白。
- 中间竖向 separator 应该是橙色细条。
- 右侧按钮和图标按钮应都有明显的 hover/active/focus 视觉变化。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-visual-proc-lab final-summary`。
- `label`、`image`、`button`、`icon`、`panel`、`separator`、`paint` 均为 `1`。

## 可跳过条件

- 如果当前机器没有可用系统字体，本例会失去 panel title 和 label 文本测量依据，应按环境缺字处理。
