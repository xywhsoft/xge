# XUI Text History Limit Lab

`xui_text_history_limit_lab` 用一个 `TextEdit` 小实验台专门验证历史栈上限行为。它不引入新的 UI 概念，重点是把 `xge_xui_text_edit_t.iUndoLimit` 与 `xgeXuiTextEditUndo/Redo` 的边界语义讲清楚，也顺手覆盖 `xgeXuiTextEditSetText` 清空历史栈的行为。

## 覆盖 API

- `xgeXuiTextEditInit`
- `xgeXuiTextEditUnit`
- `xgeXuiTextEditSetText`
- `xgeXuiTextEditGetText`
- `xgeXuiTextEditSetWordWrap`
- `xgeXuiTextEditSetColors`
- `xgeXuiTextEditUndo`
- `xgeXuiTextEditRedo`
- `xgeXuiTextEditEvent`
- `xgeXuiTextEditEventProc`
- `xgeXuiTextEditUpdateProc`
- `xgeXuiTextEditPaintProc`
- `xgeXuiSetFocus`

说明：当前 XGE 还没有单独公开 `SetHistoryLimit` 风格函数，这个范例通过公开结构体字段 `xge_xui_text_edit_t.iUndoLimit` 来配置历史栈上限，并配合公开的 `Undo/Redo` 行为验证效果。

## 运行方式

```bat
examples\xui_text_history_limit_lab\build.bat
build\xge_xui_text_history_limit_lab.exe --frames 5
```

## 自动检查

- 先把 `iUndoLimit` 设为 `2`，连续输入三次，确认 `undoCount` 只保留两条历史。
- 连续 `Undo` 两次后，第三次 `Undo` 应失败，文本停在 `AB`，说明最老的一条历史已被丢弃。
- 连续 `Redo` 两次后，第三次 `Redo` 应失败，文本回到 `ABCD`。
- 调用 `xgeXuiTextEditSetText("xy")` 后，`undo/redo` 计数都应清零。
- 再把 `iUndoLimit` 设为 `1`，通过 `Ctrl+Z / Ctrl+Y` 路径确认键盘触发也受同一上限约束。

程序退出前会打印一行 `xui-text-history-limit-lab final-summary`。

## 人工观察

- 编辑区最终应显示 `xy12`。
- 顶部状态条里的 `limit`、`undo`、`redo`、`reset`、`keyboard` 都应为 `1`。
- 中部说明行里的 `limit=1 undo=1 redo=0` 应与最后一轮键盘测试一致。

## 可跳过条件

- 若当前机器缺少可用字体，示例会初始化失败；先安装或切换到可加载的系统字体后再测。
