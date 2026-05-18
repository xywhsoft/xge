# XUI Scroll View Proc Lab

`examples/xui_scroll_view_proc_lab` 专门覆盖 `ScrollView` 的 setter/getter，以及直接 `Event / EventProc` 路径。它把 wheel、内容拖拽、竖向滚动条 page jump、横向 thumb drag 串成了一个可自动退出的自检页，同时在窗口里画出真正可滚动的内容块。

## 覆盖 API

- `xgeXuiScrollViewInit`
- `xgeXuiScrollViewUnit`
- `xgeXuiScrollViewSetContentSize`
- `xgeXuiScrollViewSetOffset`
- `xgeXuiScrollViewGetOffset`
- `xgeXuiScrollViewSetColors`
- `xgeXuiScrollViewEvent`
- `xgeXuiScrollViewEventProc`

## 构建和运行

```bat
examples\xui_scroll_view_proc_lab\build.bat
build\xge_xui_scroll_view_proc_lab.exe --frames 5
```

## 自动检查点

- `init=1`：scroll view 初始化成功，`procEvent/procPaint` 绑定正确，widget 默认可 focus 且开启 clip。
- `config=1`：content size、colors、offset clamp 都正确生效。
- `wheel=1`：wheel 同时推动 `scrollX/scrollY` 前进。
- `drag=1`：直接拖内容区域后，offset 会明显增大，drag 状态会在 mouse up 后恢复为 0。
- `vbar=1`：点击竖向 bar 中 thumb 下方区域后，Y offset 会按 page jump 推进。
- `hbar=1`：直接拖横向 thumb 后，X offset 会明显增大。
- `frame=1`：ScrollView 已正确创建内部 `ScrollFrame`、viewport widget 和 content widget。

## 人工观察点

- 视口里应能看到一块超出边界的卡片网格内容，不只是空白背景。
- 最终内容应整体向左滑过，说明横向滚动已经生效；竖向位置应回到接近顶部。
- 横向滚动条 thumb 应明显偏右，竖向 thumb 接近顶部。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-scroll-view-proc-lab final-summary`。
- `init`、`config`、`wheel`、`drag`、`vbar`、`hbar`、`frame` 均为 `1`。
