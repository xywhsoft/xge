# XUI Layout Gallery

`examples/xui_layout_gallery` 用一个可自动退出的 XUI 画廊同时覆盖 `absolute`、`row`、`column`、`stack`、`grid` 布局，以及 `content/grow/percent/px` 尺寸模式、`gap/justify/align` 和 `layout batch + dirty layout` 状态流。

## 覆盖 API

- `xgeXuiLayoutBatchBegin`
- `xgeXuiLayoutBatchEnd`
- `xgeXuiWidgetSetLayout`
- `xgeXuiWidgetSetGrid`
- `xgeXuiWidgetSetGap`
- `xgeXuiWidgetSetJustify`
- `xgeXuiWidgetSetAlign`
- `xgeXuiWidgetSetSize`
- `xgeXuiWidgetSetRect`
- `xgeXuiWidgetGetRect`
- `xgeXuiWidgetGetDesiredSize`
- `xgeXuiUpdate`
- `xgeXuiPaint`

## 构建和运行

```bat
examples\xui_layout_gallery\build.bat
build\xge_xui_layout_gallery.exe --frames 5
```

## 验证点

- 控制台输出 `xui-layout-gallery final-summary`。
- `absolute=1 row=1 column=1 stack=1 grid=1` 表示五种布局路径都已进入并通过几何检查。
- `size=1` 表示 `content/grow/percent/px` 四种尺寸单位都进入了真实布局。
- `gja=1` 表示 `gap/justify/align` 组合生效。
- `dirty=1` 且 `gap=6->18` 表示 `layout batch` 期间不会立刻请求 refresh，结束后会触发 dirty layout，并在下一次 update 后反映出新间距。
