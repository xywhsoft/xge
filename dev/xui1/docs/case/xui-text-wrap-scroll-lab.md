# XUI Text Wrap Scroll Lab

`examples/xui_text_wrap_scroll_lab` 专门覆盖 `TextEdit` 的自动换行、滚动和分页导航。它把 `SetWordWrap`、`SetScroll`、`PageDown/PageUp`、`Home/End/Ctrl+Home/Ctrl+End` 串成了一个可自动退出的自检页。

## 覆盖 API

- `xgeXuiTextEditSetText`
- `xgeXuiTextEditGetText`
- `xgeXuiTextEditSetWordWrap`
- `xgeXuiTextEditSetScroll`
- `xgeXuiDispatchEvent`
- `xgeXuiTextGetCursor`

## 构建和运行

```bat
examples\xui_text_wrap_scroll_lab\build.bat
build\xge_xui_text_wrap_scroll_lab.exe --frames 5
```

## 自动检查点

- `wrap=1`：打开 word wrap 后会生成 visual lines，`visual line count` 不小于 `line count`。
- `scroll=1`：长文本会自动产生正的 `scrollY`，手动 `SetScroll(4,5)` 会被正确记录。
- `page=1`：`PageDown/PageUp` 都能消费事件，并让 cursor 在可视页之间跳动。
- `homeend=1`：`Home`、`Ctrl+Home`、`Ctrl+End` 都能把 cursor 带到预期边界。

## 人工观察点

- 编辑区里应该能看到一段明显被折行的长文本。
- 顶部状态和编辑区标题里应能看到 `scroll`、`lines`、`visual` 的实时数值。
- 光标最终不应落在不可见区域外。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-text-wrap-scroll-lab final-summary`。
- `wrap`、`scroll`、`page`、`homeend` 均为 `1`。
