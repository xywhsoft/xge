# XUI Text Buffer Lab

`examples/xui_text_buffer_lab` 专门补 XUI 底层文本缓冲这条线。它会先直接对 `xgeXuiText*` 做字节级自检，再用一个 `Input + TextEdit` 页面补 `xgeXuiInputSetFont`、`xgeXuiTextEditSetFont` 和 `xgeXuiTextEditEvent` 的真实事件路径。

## 覆盖 API

- `xgeXuiTextInit`
- `xgeXuiTextUnit`
- `xgeXuiTextSet`
- `xgeXuiTextInsert`
- `xgeXuiTextInsertCodepoint`
- `xgeXuiTextDeleteBack`
- `xgeXuiTextDeleteForward`
- `xgeXuiTextSetCursor`
- `xgeXuiTextGetCursor`
- `xgeXuiTextSetSelection`
- `xgeXuiTextGetSelection`
- `xgeXuiTextSetComposition`
- `xgeXuiTextClearComposition`
- `xgeXuiTextGetComposition`
- `xgeXuiTextInputEvent`
- `xgeXuiInputSetFont`
- `xgeXuiTextEditSetFont`
- `xgeXuiTextEditEvent`
- `xgeXuiTextEditEventProc`
- `xgeXuiInputGetCandidateRect`
- `xgeXuiTextEditGetCandidateRect`

## 构建和运行

```bat
examples\xui_text_buffer_lab\build.bat
build\xge_xui_text_buffer_lab.exe --frames 5
```

## 自动检查点

- `text=1`：直接 `xgeXuiText*` 缓冲操作通过，包括 selection、cursor clamp、insert / delete。
- `event=1`：`xgeXuiTextInputEvent` 的 `TEXT / IME_START / IME_UPDATE / IME_END` 路径通过。
- `font=1`：`xgeXuiInputSetFont` 和 `xgeXuiTextEditSetFont` 都已切到替换字体。
- `edit=1`：`xgeXuiTextEditEvent` / `xgeXuiTextEditEventProc` 能真实处理文本输入、退格和回车。
- `ime=1`：`TextEdit` 的 composition 更新和清空通过。
- `candidate=1`：`Input` 与 `TextEdit` 的 candidate rect 都是有效矩形。

## 人工观察点

- 上方单行 `Input` 的字形尺寸应明显比 caption 更大，说明 font setter 已生效。
- 下方 `TextEdit` 的光标高度和行距应更接近大字号字体。
- 自动检查结束前，你可以观察单行和多行输入框里 caret 的高度是否一致地跟着新字体变化。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-text-buffer-lab final-summary`。
- `text`、`event`、`font`、`edit`、`ime`、`candidate` 均为 `1`。

## 可跳过条件

- 如果当前机器没有可用系统字体，这个范例无法完成 font setter 验证，应视为环境缺字体验证，不算逻辑回归。
