# XUI Tabs Dirty Rect Lab

`examples/xui_tabs_dirty_rect_lab` 把 XUI tabs、dirty rect 和 widget sizing 这几组还没被范例触达的公开 API 串成了一个可自动退出的小实验台。它会自动完成一次 tab hover/click/keyboard 切换，同时验证自定义 measure、content size、z-order hit test，以及 `xgeInvalidateRect` / `xgeDirtyRect*` 的读写路径。

## 覆盖 API

- `xgeInvalidateRect`
- `xgeDirtyRectCount`
- `xgeDirtyRectGet`
- `xgeDirtyRectClear`
- `xgeXuiSizeContent`
- `xgeXuiWidgetSetMinSize`
- `xgeXuiWidgetSetMaxSize`
- `xgeXuiWidgetSetMeasure`
- `xgeXuiWidgetGetZ`
- `xgeXuiTabsInit`
- `xgeXuiTabsUnit`
- `xgeXuiTabsSetItems`
- `xgeXuiTabsSetFont`
- `xgeXuiTabsSetSelect`
- `xgeXuiTabsSetSelected`
- `xgeXuiTabsGetSelected`
- `xgeXuiTabsSetTabSize`
- `xgeXuiTabsSetColors`
- `xgeXuiTabsGetState`
- `xgeXuiTabsEvent`
- `xgeXuiTabsEventProc`
- `xgeXuiTabsPaintProc`

## 构建和运行

```bat
examples\xui_tabs_dirty_rect_lab\build.bat
build\xge_xui_tabs_dirty_rect_lab.exe --frames 5
```

## 自动检查点

- `tabs=1/1/1/1`：分别表示 mouse select、keyboard select、state、proc binding 都通过。
- `measure=1`：`content + measure + min/max` 最终被 clamp 到预期尺寸。
- `dirty=1`：`InvalidateRect / DirtyRectCount / DirtyRectGet / DirtyRectClear` 都通过。
- `z=1`：重叠区 hit test 命中更高 `z` 的前景卡片。
- `content=1`：`xgeXuiSizeContent()` 和控件 style 上的 content unit 都生效。

## 人工观察点

- 顶部 tabs 条应有明显 hover / selected 差异。
- 中间左侧测量卡片尺寸应固定在一个被 clamp 后的盒子，不会因为内容测量值更大而继续膨胀。
- 右侧两张半透明卡片应明显重叠，蓝色前景卡片压在橙色背景卡片之上。

## 通过标准

- 程序能自动退出。
- 控制台打印 `xui-tabs-dirty-rect-lab final-summary`。
- `tabs`、`measure`、`dirty`、`z`、`content` 均为 `1`。

## 可跳过条件

- 如果当前机器没有可用字体，程序仍会运行，但 tabs 文本可能缺字；这不影响本例对 tabs 事件、dirty rect 和 layout 路径的验证。
