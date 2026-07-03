# XUI Widget Tree Style Lab

`examples/xui_widget_tree_style_lab` 用一个可自动退出的 XUI 小面板覆盖 widget tree 和基础 style API。范例会创建一组 widget，验证 add/remove/free、id/name/find、rect/local/content rect、visible/enabled/focusable/clip、style/margin/padding/anchor，以及 `mark layout`、`mark paint`、`refresh` 的联动。

## 覆盖 API

- `xgeXuiInit`
- `xgeXuiUnit`
- `xgeXuiRoot`
- `xgeXuiStyleDefault`
- `xgeXuiStyleFromTheme`
- `xgeXuiRefreshRequest`
- `xgeXuiRefreshNeeded`
- `xgeXuiWidgetCreate`
- `xgeXuiWidgetFree`
- `xgeXuiWidgetAdd`
- `xgeXuiWidgetRemove`
- `xgeXuiWidgetSetId`
- `xgeXuiWidgetGetId`
- `xgeXuiWidgetSetName`
- `xgeXuiWidgetGetName`
- `xgeXuiWidgetFindById`
- `xgeXuiWidgetFindByName`
- `xgeXuiWidgetSetRect`
- `xgeXuiWidgetGetRect`
- `xgeXuiWidgetSetStyle`
- `xgeXuiWidgetGetStyle`
- `xgeXuiWidgetSetAnchorPx`
- `xgeXuiWidgetGetAnchor`
- `xgeXuiWidgetSetMarginPx`
- `xgeXuiWidgetSetPaddingPx`
- `xgeXuiWidgetSetBackground`
- `xgeXuiWidgetSetRadius`
- `xgeXuiWidgetGetFlags`
- `xgeXuiWidgetSetVisible`
- `xgeXuiWidgetSetEnabled`
- `xgeXuiWidgetSetFocusable`
- `xgeXuiWidgetSetClip`
- `xgeXuiWidgetSetUpdate`
- `xgeXuiWidgetSetPaint`
- `xgeXuiWidgetIsVisible`
- `xgeXuiWidgetIsEnabled`
- `xgeXuiWidgetIsFocusable`
- `xgeXuiWidgetMarkLayout`
- `xgeXuiWidgetMarkPaint`
- `xgeXuiUpdate`
- `xgeXuiPaint`

## 构建和运行

```bat
examples\xui_widget_tree_style_lab\build.bat
build\xge_xui_widget_tree_style_lab.exe --frames 5
```

## 验证点

- 控制台输出 `xui-widget-tree-style-lab final-summary`。
- `tree=1` 表示 create/add/remove/free 路径都跑通了。
- `find=1` 表示 `id/name/find` 和 `getId/getName` 都能对上。
- `rect=1` 表示 `rect/local rect/content rect` 与 anchor/stretch 的布局结果正确。
- `flags=1` 表示 visible/enabled/focusable/clip 的设置和查询正确。
- `style=1` 表示 `style default/from theme`、`set/get style`、background/radius/margin/padding/anchor 都生效。
- `marks=1/1` 且 `refresh=1` 表示 `mark layout`、`mark paint`、`refresh request/needed` 的状态流正确。
- `paint=1` 且 `paint_calls`、`update_calls` 为正，表示自定义 update/paint 回调进入成功。
