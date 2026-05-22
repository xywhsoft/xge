# XUI DockPanel

`DockPanel` 由 `DockLayout`、`DockPane` 和 `DockWindow` 组成，用于构建 IDE、编辑器和工具工作台式界面。它属于 Widget V2 新范式复合控件，必须复用 XUI 的布局、overlay、pointer capture、focus、tooltip、host 绘制和 XSON 生命周期，而不是在控件内部另建一套窗口系统。

## 结构

```text
docklayout
  dock_region
    dock_node(split|pane)
      dock_pane
        dockwindow tabs
```

`dock_node` 使用显式二叉 split tree，便于 C 结构维护、调试和序列化。`DockWindow` 组合现有 `xge_xui_window_t`：停靠时隐藏普通 Window 外框，由 DockPane 绘制 tab、title、pane button 和 client；浮动时恢复特殊 dockwindow 外观，但仍限制在 XUI root 内，不创建 OS/native 子窗口。

## 能力

- document、left、right、top、bottom region。
- pane tab、active tab、close、overflow menu 和 tooltip。
- split tree 布局和 splitter 调整。
- XUI root 内 floating dockwindow。
- DockPanelSuite VS2005 风格 indicator、pane diamond 和 preview rect。
- tab reorder、tab float、floating dock-back 和 Ctrl 抑制 docking。
- auto-hide strip、overlay pane 展开和 dock restore。
- save/load runtime state。
- XSON 初始布局。

## C API 入口

```c
xgeXuiDockLayoutInit(&layout, &xui, layoutWidget);
xgeXuiDockWindowInit(&doc, &xui);
xgeXuiDockWindowSetTitle(&doc, "Document.c");
xgeXuiDockWindowSetClientWidget(&doc, editorWidget);
xgeXuiDockLayoutDockWindow(
    &layout,
    &doc,
    XGE_XUI_DOCK_REGION_DOCUMENT,
    XGE_XUI_DOCK_SIDE_FILL,
    0.0f);
```

常用 API：

- `xgeXuiDockLayoutDockWindow`
- `xgeXuiDockLayoutFloatWindow`
- `xgeXuiDockLayoutHideWindow`
- `xgeXuiDockLayoutAutoHideWindow`
- `xgeXuiDockLayoutDockAutoHideWindow`
- `xgeXuiDockLayoutSaveState`
- `xgeXuiDockLayoutLoadState`
- `xgeXuiDockWindowSetClientWidget`
- `xgeXuiDockPaneSetActiveIndex`

## XSON

```json
{
  "type": "dockLayout",
  "id": "mainDock",
  "regions": { "left": 0.22, "right": 0.2, "bottom": 0.25 },
  "dockWindows": [
    {
      "id": "document",
      "title": "Document",
      "region": "document",
      "side": "fill",
      "children": [
        { "type": "label", "text": "Document client" }
      ]
    }
  ]
}
```

`dockWindows` 支持 `state: "docked"`、`"floating"`、`"hidden"` 和 `"autoHide"`。声明式 children 属于 dockwindow client，随 `xgeXuiPageUnload` 释放。

## 范例

- `examples/xui_dockpanel_lab`
- `examples/xui_dockpanel_xson`

详细实现跟踪见 `dev/docs/XUI DockPanel停靠面板SPEC.md`。
