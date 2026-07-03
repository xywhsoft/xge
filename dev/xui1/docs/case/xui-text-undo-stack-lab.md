# XUI Text Undo Stack Lab

`examples/xui_text_undo_stack_lab` 专门覆盖 `TextEdit` 的 undo/redo 历史栈行为。它把空栈返回、直接 `Undo/Redo`、`Ctrl+Z/Ctrl+Y`、selection restore，以及“undo 之后新编辑会清空 redo 栈”串成了一个可自动退出的自检页。

## 覆盖 API

- `xgeXuiTextEditSetText`
- `xgeXuiTextEditGetText`
- `xgeXuiTextEditUndo`
- `xgeXuiTextEditRedo`
- `xgeXuiDispatchEvent`
- `xgeXuiSetFocus`
- `xgeXuiTextGetCursor`
- `xgeXuiTextGetSelection`

## 构建和运行

```bat
examples\xui_text_undo_stack_lab\build.bat
build\xge_xui_text_undo_stack_lab.exe --frames 5
```

## 自动检查点

- `empty=1`：初始 history 为空时，直接 `Undo/Redo` 会返回非成功状态，不会伪造栈内容。
- `direct=1`：`Enter` 和文本输入生成两个 undo snapshot，直接 `xgeXuiTextEditUndo/Redo` 能正确往返。
- `selection=1`：`Ctrl+A` 后替换文本，再 `Undo` 会恢复原文本以及整段 selection。
- `keyboard=1`：`Ctrl+Y` / `Ctrl+Z` 事件路径和直接 API 路径一致。
- `clear=1`：在 undo 之后进行新输入，会清空 redo 栈，后续 `Redo` 不再成功。

## 人工观察点

- 顶部状态条里的 `undo` / `redo` 计数应和编辑区实际状态一致。
- 编辑区最终应停在三行文本 `ab / cd / e!`，而不是回到单字符替换态。
- 如果你手动继续按 `Ctrl+Z` / `Ctrl+Y`，文本应能在追加 `!` 前后的状态之间切换，不应跳回旧的 redo 分支。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-text-undo-stack-lab final-summary`。
- `empty`、`direct`、`selection`、`keyboard`、`clear` 均为 `1`。
