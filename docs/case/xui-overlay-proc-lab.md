# XUI Overlay Proc Lab

`examples/xui_overlay_proc_lab` 专门覆盖 XUI overlay 相关控件的直接 `Event / EventProc / PaintProc` 路径。它把 `Popup / ComboBox / Dialog` 放进一个可自动退出的页面里，同时借用 combo 的下拉列表补上 `ListViewEventProc / ListViewPaintProc` 这条公开 proc 路径。

## 覆盖 API

- `xgeXuiPopupEvent`
- `xgeXuiPopupEventProc`
- `xgeXuiPopupPaintProc`
- `xgeXuiComboBoxEvent`
- `xgeXuiComboBoxEventProc`
- `xgeXuiComboBoxPaintProc`
- `xgeXuiComboBoxGetSelected`
- `xgeXuiComboBoxIsOpen`
- `xgeXuiComboBoxGetState`
- `xgeXuiListViewEventProc`
- `xgeXuiListViewPaintProc`
- `xgeXuiListViewGetSelected`
- `xgeXuiListViewGetScroll`
- `xgeXuiDialogEvent`
- `xgeXuiDialogEventProc`
- `xgeXuiDialogPaintProc`
- `xgeXuiDialogIsOpen`

## 构建和运行

```bat
examples\xui_overlay_proc_lab\build.bat
build\xge_xui_overlay_proc_lab.exe --frames 5
```

## 自动检查点

- `popup=1`：popup 的 `Event/EventProc/PaintProc` 绑定正确，outside close 和 ESC close 都能触发。
- `combo=1`：combo 的 `Event/EventProc/PaintProc` 绑定正确，mouse open、keyboard open、disabled state 和 selected state 正常。
- `list=1`：combo 下拉里的 list view `EventProc/PaintProc` 绑定正确，点击条目后会驱动 combo 选中并关闭下拉。
- `dialog=1`：dialog 的 `Event/EventProc/PaintProc` 绑定正确，modal、close glyph、close-on-escape、show-close 策略都按预期工作。
- `paint=1`：本例已直接调用过 popup/combo/list/dialog 的公开 paint proc。

## 人工观察点

- 左侧应有一块蓝色 owner 区，下面是展开状态的 popup。
- 中间 combo 应保持展开，下拉列表里高亮停在第三项。
- 右侧 dialog 应显示标题和关闭块，但不应再有整屏遮罩。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-overlay-proc-lab final-summary`。
- `popup`、`combo`、`list`、`dialog`、`paint` 均为 `1`。
