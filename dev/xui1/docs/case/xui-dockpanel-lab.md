# XUI DockPanel Lab

`examples/xui_dockpanel_lab` 展示新范式 DockPanel 工作台控件：一个 `DockLayout` 管理 document、left、right、bottom 等 region，并通过显式 split tree 组织 dock pane、tab 和 dockwindow。

[返回范例解析](README.md) | [XUI 控件教程](../guide/xui-controls-intro.md) | [XUI API](../api/xui.md)

## 覆盖内容

- `DockLayout` region 和 split tree 布局。
- `DockWindow` 组合现有 `Window`，停靠时由 DockPane 绘制 tab/title/client。
- XUI root 内 floating dockwindow，不创建 OS/native 子窗口。
- DockPanelSuite VS2005 风格 indicator、pane diamond 和 preview rect。
- pane tab 激活、关闭、overflow menu、tooltip 和 tab reorder preview。
- auto-hide strip、overlay pane 展开和 dock restore。
- XSON 初始布局：`examples/xui_dockpanel_xson` 覆盖 docked、floating、hidden 和 autoHide。

## 验证入口

```bat
examples\xui_dockpanel_lab\build.bat
build\xui_dockpanel_lab.exe --frames 3
build\xui_dockpanel_lab.exe --drag-preview --frames 3
build\xui_dockpanel_lab.exe --float-dock-preview --frames 3
build\xui_dockpanel_lab.exe --global-dock-preview --frames 3
build\xui_dockpanel_lab.exe --auto-hide-preview --frames 3
```

XSON 版本：

```bat
examples\xui_dockpanel_xson\build.bat
build\xui_dockpanel_xson.exe --frames 3
```

## 设计口径

DockPanel 不是旧式普通控件拼接，而是 Widget V2 上的工作台级复合控件。它必须复用 XUI 的 container、overlay、pointer capture、focus、tooltip、host 和 XSON 机制；内部模型采用 `docklayout -> dock_region -> dock_node(split|pane) -> dock_pane -> dockwindow tabs`，不照搬 DockPanelSuite 的 `previousPane + alignment + proportion` 数据模型。
