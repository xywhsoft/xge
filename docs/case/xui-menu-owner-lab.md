# XUI Menu Owner Lab

`examples/xui_menu_owner_lab` 专门覆盖 `Menu` 的 owner 绑定和弹出行为。它把 `SetItems/SetEnabledItems/SetFont/SetSelect/SetSize/SetColors`、`Open/Close/IsOpen`，以及 disabled row、outside close、focus return、窗口边界 clamp 串成了一个可自动退出的自检页。

## 覆盖 API

- `xgeXuiMenuInit`
- `xgeXuiMenuUnit`
- `xgeXuiMenuSetItems`
- `xgeXuiMenuSetEnabledItems`
- `xgeXuiMenuSetFont`
- `xgeXuiMenuSetSelect`
- `xgeXuiMenuSetSize`
- `xgeXuiMenuSetColors`
- `xgeXuiMenuOpen`
- `xgeXuiMenuClose`
- `xgeXuiMenuIsOpen`

## 构建和运行

```bat
examples\xui_menu_owner_lab\build.bat
build\xge_xui_menu_owner_lab.exe --frames 5
```

## 自动检查点

- `init=1`：menu 初始化后拥有 popup/list 两个内部 widget，并正确绑定 owner。
- `config=1`：items、enabled flags、font、select callback、size、colors 都能落到 menu 状态里。
- `clamp=1`：在窗口右下角附近 `Open` 时，popup rect 会被裁进窗口内。
- `disabled=1`：点击 disabled row 不会关闭 menu，也不会触发 select callback。
- `select=1`：点击 enabled row 会关闭 menu、触发 callback，并把 focus 还给 owner。
- `close=1`：outside click 和 `ESC` 都能把 menu 关掉。

## 人工观察点

- 主面板里应有一个蓝色 `Menu owner`。
- 自动检查结束后，owner 下方应留着一个展开的 menu。
- 第二项 `Disabled` 应明显偏灰，不应和可选项一个亮度。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-menu-owner-lab final-summary`。
- `init`、`config`、`clamp`、`disabled`、`select`、`close` 均为 `1`。
