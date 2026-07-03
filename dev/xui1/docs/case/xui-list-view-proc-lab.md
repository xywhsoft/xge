# XUI List View Proc Lab

`examples/xui_list_view_proc_lab` 专门覆盖 `ListView` 的公开 setter/getter，以及直接 `Event / EventProc / PaintProc` 路径。它把 mouse select、disabled item、wheel、thumb drag、keyboard navigation 都做成了一个可自动退出的自检页。

## 覆盖 API

- `xgeXuiListViewInit`
- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetEnabledItems`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewSetSelected`
- `xgeXuiListViewGetSelected`
- `xgeXuiListViewSetScroll`
- `xgeXuiListViewGetScroll`
- `xgeXuiListViewSetSelect`
- `xgeXuiListViewSetColors`
- `xgeXuiListViewSetDisabledTextColor`
- `xgeXuiListViewEvent`
- `xgeXuiListViewEventProc`
- `xgeXuiListViewPaintProc`

## 构建和运行

```bat
examples\xui_list_view_proc_lab\build.bat
build\xge_xui_list_view_proc_lab.exe --frames 5
```

## 自动检查点

- `init=1`：list view 初始化成功，`procEvent/procPaint` 绑定正确，widget 默认可 focus 且开启 clip。
- `config=1`：items、enabled items、font、item height、selected、scroll、colors 配置都落到控件状态里。
- `mouse=1`：直接 `xgeXuiListViewEvent` 处理 mouse select 后，selected 正确切换并触发 callback。
- `disabled=1`：点击 disabled row 时事件会被消费，但 selected 不会变化。
- `wheel=1`：wheel 会推动 scroll 值前进。
- `drag=1`：直接拖 scrollbar thumb 后，thumb 位置和 scroll 值都会明显变化。
- `keys=1`：`End / Up / Enter` 这些键盘路径会更新 selected 并再次触发 callback。
- `paint=1`：本例已直接调用过 `xgeXuiListViewPaintProc`。

## 人工观察点

- 列表应只有一列，行高较大，第二、第五、第九项是 disabled 色。
- 最终高亮应停在倒数第二项附近，滚动条 thumb 明显不在顶部。
- 状态栏里的 `sel / scroll / cb / last` 应和列表视觉状态一致。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-list-view-proc-lab final-summary`。
- `init`、`config`、`mouse`、`wheel`、`drag`、`keys`、`disabled`、`paint` 均为 `1`。
