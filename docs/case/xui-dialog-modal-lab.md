# XUI Dialog Modal Lab

`examples/xui_dialog_modal_lab` 专门覆盖 `Dialog` 的 modal 行为和可见策略。它把 `SetTitle/SetClose/SetOpen/IsOpen/SetModal/SetCloseOnEscape/SetShowClose/SetColors`，以及 `DialogEvent/EventProc/PaintProc` 压成了一个可自动退出的自检页。

## 覆盖 API

- `xgeXuiDialogInit`
- `xgeXuiDialogUnit`
- `xgeXuiDialogSetTitle`
- `xgeXuiDialogSetClose`
- `xgeXuiDialogSetOpen`
- `xgeXuiDialogIsOpen`
- `xgeXuiDialogSetModal`
- `xgeXuiDialogSetCloseOnEscape`
- `xgeXuiDialogSetShowClose`
- `xgeXuiDialogSetColors`
- `xgeXuiDialogEvent`
- `xgeXuiDialogEventProc`
- `xgeXuiDialogPaintProc`

## 构建和运行

```bat
examples\xui_dialog_modal_lab\build.bat
build\xge_xui_dialog_modal_lab.exe --frames 5
```

## 自动检查点

- `init=1`：dialog 初始化后带默认 `modal + closeOnEscape + showClose`，并绑定 event/paint proc。
- `config=1`：标题、颜色、open 状态和焦点都能落到 dialog 上。
- `modal=1`：modal 状态下，点击对话框外部会被消费，但不会关闭。
- `closebtn=1`：点击右上角关闭块会关闭 dialog，并触发 close callback。
- `escape=1`：关闭 `closeOnEscape` 后按 `ESC` 不会关闭；重新打开后恢复 `ESC` 可关闭。
- `proc=1`：直接走 `DialogEvent` / `DialogEventProc` 时，`modal` 和 `showClose` 行为都一致。

## 人工观察点

- 左侧应有一个 `Dialog owner`，右侧应有一个带标题和关闭块的对话框。
- 对话框外部应有一层半透明 backdrop。
- 关闭块隐藏时，右上角不应再有红色小块。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-dialog-modal-lab final-summary`。
- `init`、`config`、`modal`、`closebtn`、`escape`、`proc` 均为 `1`。
