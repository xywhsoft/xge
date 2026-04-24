# XUI Dialog Policy Lab

`xui_dialog_policy_lab` 把 `Dialog` 的事件策略单独拆出来，重点覆盖 `modal / non-modal / closeOnEscape / showClose` 组合下的事件消费行为，以及 reopen 后 title 与视觉状态的恢复。

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

## 运行方式

```bat
examples\xui_dialog_policy_lab\build.bat
build\xge_xui_dialog_policy_lab.exe --frames 5
```

## 自动检查

- 初始化后检查默认 `modal/closeOnEscape/showClose` 状态，以及 `procEvent/procPaint` 绑定。
- 设置 title、close callback、colors 并打开 dialog，验证 focus 落到 dialog。
- `modal=1` 时，outside `mouse wheel` 应被消费。
- 切到 `modal=0` 后，同样的事件应继续向下传递。
- `closeOnEscape=0` 时按 `ESC` 不应关闭；恢复为 `1` 后按 `ESC` 应关闭并触发 close callback。
- `showClose=0` 时点击 close 区域不应关闭；恢复为 `1` 后点击 close block 应关闭。
- 最后重新打开并改 title，验证 reopen 后视觉和策略状态恢复。

## 人工观察

- 顶部状态条里的 `init`、`visual`、`event`、`policy`、`closebtn`、`reopen` 都应为 `1`。
- 结束前 dialog 应保持打开，标题应显示 `Policy dialog reopened`。
- close button 可见，背景遮罩明显但不过深。

## 可跳过条件

- 若当前机器没有可用字体，示例会初始化失败；先切换到可加载字体后再测。
