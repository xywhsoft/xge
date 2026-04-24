# XUI Text Delete Lab

`examples/xui_text_delete_lab` 专门覆盖文本删除和选区替换这条线。它把底层 `xgeXuiTextInsert/DeleteBack/DeleteForward`，以及 `Input/TextEdit` 里的 `Backspace/Delete/selection replace`、多行合并和 `undo/redo` 串成了一个可自动退出的自检页。

## 覆盖 API

- `xgeXuiTextSet`
- `xgeXuiTextInsert`
- `xgeXuiTextDeleteBack`
- `xgeXuiTextDeleteForward`
- `xgeXuiTextSetCursor`
- `xgeXuiTextGetCursor`
- `xgeXuiTextSetSelection`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetSelection`
- `xgeXuiInputGetSelection`
- `xgeXuiTextEditSetText`
- `xgeXuiTextEditGetText`
- `xgeXuiTextEditUndo`
- `xgeXuiTextEditRedo`
- `xgeXuiDispatchEvent`

## 构建和运行

```bat
examples\xui_text_delete_lab\build.bat
build\xge_xui_text_delete_lab.exe --frames 5
```

## 自动检查点

- `text=1`：底层文本对象的 selection replace、forward delete、UTF-8 backspace 都能得到预期结果。
- `input=1`：`Input` 的 selection replace、Delete、Backspace 都能正确更新文本和 cursor。
- `edit=1`：`TextEdit` 的 selection replace、跨行 Backspace 合并、Delete 都能正确更新文本。
- `undo=1`：`TextEdit` 的删除相关修改会正确记入 undo/redo 历史。

## 人工观察点

- 上方 `Input` 最终应该只剩一个字符。
- 下方 `TextEdit` 最终应该保留三行文本，最后一行是 `e`。
- 顶部状态行里的 `text / input / edit / undo` 应全部为 `1`。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-text-delete-lab final-summary`。
- `text`、`input`、`edit`、`undo` 均为 `1`。
