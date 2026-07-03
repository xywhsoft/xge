# XUI Input Proc Lab

`examples/xui_input_proc_lab` 专门覆盖 `Input` 和 `TextEdit` 的直接 `Event / EventProc / UpdateProc / PaintProc` 路径。它把 selection、candidate rect、blink update、默认 context menu 这些偏底层的输入链路串成了一个可自动退出的自检页。

## 覆盖 API

- `xgeXuiInputInit`
- `xgeXuiInputSetColors`
- `xgeXuiInputSetPlaceholder`
- `xgeXuiInputSetText`
- `xgeXuiInputGetText`
- `xgeXuiInputEvent`
- `xgeXuiInputEventProc`
- `xgeXuiInputUpdateProc`
- `xgeXuiInputPaintProc`
- `xgeXuiInputSetSelection`
- `xgeXuiInputGetSelection`
- `xgeXuiInputGetCandidateRect`
- `xgeXuiTextEditInit`
- `xgeXuiTextEditSetColors`
- `xgeXuiTextEditSetText`
- `xgeXuiTextEditGetText`
- `xgeXuiTextEditSetWordWrap`
- `xgeXuiTextEditEvent`
- `xgeXuiTextEditEventProc`
- `xgeXuiTextEditUpdateProc`
- `xgeXuiTextEditPaintProc`
- `xgeXuiTextEditGetCandidateRect`

## 构建和运行

```bat
examples\xui_input_proc_lab\build.bat
build\xge_xui_input_proc_lab.exe --frames 5
```

## 自动检查点

- `input=1`：`Input` 的 `procEvent/procUpdate/procPaint` 绑定正确，直接 `Event` 可写入文本。
- `edit=1`：`TextEdit` 的 `procEvent/procUpdate/procPaint` 绑定正确，直接 `EventProc` 可修改多行文本。
- `select=1`：`Input` selection getter/setter 生效，`Input/TextEdit` 的 candidate rect 都返回有效高度。
- `update=1`：聚焦后直接调用 `UpdateProc`，光标 blink 状态会变化。
- `menu=1`：右键 context begin 能拉起 `Input` 和 `TextEdit` 默认菜单。
- `paint=1`：本例已直接调用过 `xgeXuiInputPaintProc` 和 `xgeXuiTextEditPaintProc`。

## 人工观察点

- 上方单行 `Input` 应显示 placeholder 风格和选区高亮，caret 高度要和字体匹配。
- 下方多行 `TextEdit` 应展示两行文本，插入 `Q` 后第一行末尾会更长。
- 右键时默认 context menu 应能弹出，随后自动关闭；状态栏里的 `cand` 数值应为正数。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-input-proc-lab final-summary`。
- `input`、`edit`、`select`、`update`、`menu`、`paint` 均为 `1`。
