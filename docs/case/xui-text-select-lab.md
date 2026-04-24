# XUI Text Select Lab

`examples/xui_text_select_lab` 专门覆盖 `Input` 和 `TextEdit` 的光标移动、选区扩展和鼠标拖选。它把单行输入的词级跳转、Home/End、Ctrl+A 和拖选，以及多行编辑器的 Home/End、Ctrl+Home/Ctrl+End、PageDown、Shift 扩选和跨行拖选拆成了一个可自动退出的自检页。

## 覆盖 API

- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputSetSelection`
- `xgeXuiInputGetSelection`
- `xgeXuiTextGetCursor`
- `xgeXuiTextGetSelection`
- `xgeXuiDispatchEvent`
- `xgeXuiTextEditSetText`
- `xgeXuiTextEditGetText`
- `xgeXuiTextEditSetWordWrap`

## 构建和运行

```bat
examples\xui_text_select_lab\build.bat
build\xge_xui_text_select_lab.exe --frames 5
```

## 自动检查点

- `input_nav=1`：`Input` 的 Ctrl+Left / Ctrl+Right / Home / End 路径都能把 cursor 移到预期位置。
- `input_select=1`：`Input` 的 Ctrl+A 和鼠标拖选都会产生非空 selection。
- `edit_nav=1`：`TextEdit` 的 Home、Up、Ctrl+End、Ctrl+Home、PageDown 路径都能正确消费事件并更新 cursor。
- `edit_select=1`：`TextEdit` 的 Shift+Right、Ctrl+A 和跨行鼠标拖选都会产生非空 selection。

## 人工观察点

- 上方 `Input` 最终应保留一段蓝色选区高亮。
- 下方 `TextEdit` 最终应出现跨行选区，caret 不应跑到可视区域外。
- 顶部状态行里的 `input_nav / input_select / edit_nav / edit_select` 应全部为 `1`。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-text-select-lab final-summary`。
- `input_nav`、`input_select`、`edit_nav`、`edit_select` 均为 `1`。
