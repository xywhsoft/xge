# XUI List View Detail Lab

`xui_list_view_detail_lab` 把 `ListView` 再单独拆细一层，重点覆盖配置细节和行为边界：`SetEnabledItems` 使当前选中项失效后的清空、滚动条 page jump、thumb drag，以及键盘导航跨过 disabled row。

## 覆盖 API

- `xgeXuiListViewInit`
- `xgeXuiListViewUnit`
- `xgeXuiListViewSetItems`
- `xgeXuiListViewSetEnabledItems`
- `xgeXuiListViewSetFont`
- `xgeXuiListViewSetItemHeight`
- `xgeXuiListViewSetSelected`
- `xgeXuiListViewGetSelected`
- `xgeXuiListViewSetScroll`
- `xgeXuiListViewGetScroll`
- `xgeXuiListViewSetSelect`
- `xgeXuiListViewSetColors`
- `xgeXuiListViewSetDisabledTextColor`
- `xgeXuiListViewEvent`
- `xgeXuiListViewEventProc`

## 运行方式

```bat
examples\xui_list_view_detail_lab\build.bat
build\xge_xui_list_view_detail_lab.exe --frames 5
```

## 自动检查

- 初始化后检查 `procEvent/procPaint`、focusable 和 clip 标记。
- 配置 items、enabled flags、font、item height、colors 和 callback。
- 先选中 `Delta Relay`，再把它标成 disabled，验证 `selected` 会被自动清空。
- 点击 enabled row 应切到 `Echo Lab` 并触发 callback；点击 disabled row 不应改选中项。
- 点击滚动条 track 应触发 page jump；拖动 thumb 应改变 `scroll`。
- 键盘 `Home/PageDown/End/Up/Enter` 应能跳过 disabled row，并最终把选中项停在 `India Core`。
- 退出前打印 `xui-list-view-detail-lab final-summary`。

## 人工观察

- 顶部状态条里的 `init`、`config`、`clear`、`mouse`、`disabled`、`page`、`drag`、`keys` 都应为 `1`。
- 列表里 `Charlie Depot (Disabled)` 和 `Hotel Annex (Disabled)` 应明显是灰态。
- 程序结束前高亮项应停在 `India Core`，滚动条 thumb 应比顶部明显更靠下。

## 可跳过条件

- 若当前机器没有可用字体，示例会初始化失败；先切换到可加载字体后再测。
