# XUI Popup Owner Lab

`examples/xui_popup_owner_lab` 专门覆盖 `Popup` 的 owner 绑定和自动关闭策略。它把 `SetOwner/SetClose/SetOpen/IsOpen/SetAutoClose/SetBackground`，以及 `PopupEvent/EventProc/PaintProc` 的核心行为都压在了一个可自动退出的小实验台里。

## 覆盖 API

- `xgeXuiPopupInit`
- `xgeXuiPopupUnit`
- `xgeXuiPopupSetOwner`
- `xgeXuiPopupSetClose`
- `xgeXuiPopupSetOpen`
- `xgeXuiPopupIsOpen`
- `xgeXuiPopupSetAutoClose`
- `xgeXuiPopupSetBackground`
- `xgeXuiPopupEvent`
- `xgeXuiPopupEventProc`
- `xgeXuiPopupPaintProc`

## 构建和运行

```bat
examples\xui_popup_owner_lab\build.bat
build\xge_xui_popup_owner_lab.exe --frames 5
```

## 自动检查点

- `init=1`：popup 初始化后会把 event/paint proc 绑定到 popup widget，并保持 focusable + no-clip。
- `open=1`：`SetOpen(1)` 后 popup 可见且获得 focus。
- `owner=1`：点击 owner 不会把 popup 当作 outside click 关掉。
- `autoclose=1`：outside click 和 `ESC` 都能触发关闭，并调用 close callback。
- `proc=1`：直接走 `xgeXuiPopupEvent` / `xgeXuiPopupEventProc` 时，行为和正常分发路径一致。
- `bg=1`：背景色配置会落到 popup 状态里。

## 人工观察点

- 左侧应有一个蓝色 `Popup owner`，右侧应留着一个深色 popup 面板。
- popup 应明显不是裁剪在 owner 内部，而是独立浮在 overlay 上。
- 你手动点击 owner、popup 内部、窗口空白处时，三种行为应明显不同。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-popup-owner-lab final-summary`。
- `init`、`open`、`owner`、`autoclose`、`proc`、`bg` 均为 `1`。
