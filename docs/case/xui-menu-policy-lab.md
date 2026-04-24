# XUI Menu Policy Lab

`xui_menu_policy_lab` 把 `Menu` 的打开/关闭语义单独拆出来，重点覆盖 `MenuOpen` 在重复打开时对 `selected/scroll` 的重置、尺寸变化后的 reopen、生效中的 clamp，以及 `MenuClose` 的 focus return 和幂等行为。

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

## 运行方式

```bat
examples\xui_menu_policy_lab\build.bat
build\xge_xui_menu_policy_lab.exe --frames 5
```

## 自动检查

- 初始化后检查 owner、popup/list widget 和默认尺寸字段。
- 配置 items、enabled flags、font、callback、size、colors。
- 在打开前故意写入旧的 `selected/scroll`，再调用 `MenuOpen`，验证它会 reset 到 `-1/0`。
- 先用小尺寸在右下角打开，验证 popup 会 clamp 到窗口内。
- 调用 `MenuClose` 两次，验证 close 幂等且 focus 会回到 owner。
- 禁用当前选中项，验证底层 list selection 会被自动清空。
- 改成更大的尺寸重新打开，验证新的宽高会生效，而且 reset 行为仍成立。
- 最后点击第二项，验证 callback 收到正确索引；随后再次打开，让程序退出前保留一个可见菜单。

## 人工观察

- 顶部状态条里的 `init`、`config`、`reset`、`clamp`、`reopen`、`focus`、`enabled` 都应为 `1`。
- 结束前菜单应保持打开，宽度明显比第一次更宽。
- 列表应从顶部开始显示，滚动条不应停在旧位置。

## 可跳过条件

- 若当前机器没有可用字体，示例会初始化失败；先切换到可加载字体后再测。
