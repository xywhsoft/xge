# XUI DockPanelSuite Reference Assets

This folder contains DockPanelSuite assets collected for the planned XUI
docklayout / dockwindow implementation.

Source:
- Repository: https://github.com/dockpanelsuite/dockpanelsuite
- Source snapshot used locally: `WinFormsUI/ThemeVS2005/Resources`
- License: MIT, copied as `LICENSE.dockpanelsuite.txt`

## vs2005

Ready-to-use PNG indicators. The pane diamond images in this folder have been
post-processed so the original WinForms edge key color is real alpha
transparency, which avoids white/red matte artifacts when drawn by XUI.

- `DockIndicator_PaneDiamond*.png`: the 5-way pane drop diamond used when the
  cursor is over an existing dock pane.
- `DockIndicator_Panel*.png`: global docklayout indicators for left, right,
  top, bottom, and fill targets.
- `DockIndicator_Panel*_Active.png`: active/highlighted state for those global
  indicators.
- `DockPane_Close.png`: dock pane close button.
- `DockPane_AutoHide.png`: dock pane auto-hide / pin toggle button.
- `DockPane_Dock.png`: dock pane dock / unpin toggle button.
- `DockPane_Option.png`: dock pane option menu button.
- `DockPane_OptionOverflow.png`: dock pane overflow menu button.

Useful XUI mapping:
- `PaneDiamond` -> local `dockpane` target overlay.
- `PanelLeft/Right/Top/Bottom` -> main `docklayout` edge targets.
- `PanelFill` -> document/center target.
- `DockPane_*` -> dock pane title/tab chrome buttons.

The DockPanelSuite hotspot images are not stored here because they are
WinForms-specific pixel-hit-test helpers. XUI should use explicit dock target
rects/regions instead.

## Related DockPanelSuite Code

- `WinFormsUI/Docking/DockPanel.DockDragHandler.cs`
- `WinFormsUI/ThemeVS2005/VS2005PaneIndicatorFactory.cs`
- `WinFormsUI/ThemeVS2005/VS2005PanelIndicatorFactory.cs`
