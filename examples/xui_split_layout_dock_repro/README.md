# XUI SplitLayout Dock Repro

Minimal interactive reproduction for SplitLayout pointer-coordinate errors in a realistic application shell.

The widget hierarchy is:

```text
Root
+- MenuBar
+- Toolbar
+- DockPanel
|  +- left Toolbox pane
|  +- center document pane
|  |  +- Split Repro tab
|  |  |  +- SplitLayout
|  |  +- Readme tab
|  +- right Inspector pane
|  +- bottom Output pane
+- StatusBar
```

Build and run:

```bat
examples\xui_split_layout_dock_repro\build.bat
build\xui_split_layout_dock_repro.exe
```

Drag the vertical divider in the `Split Repro` tab. The 3 px cyan drag shadow must stay under the pointer, and the committed divider position must match the shadow after mouse release.

Use `--high-dpi` to exercise framebuffer-to-XUI mapping on a high-DPI display. The program uses XGE on-demand rendering in interactive mode and the ordered `xuiProxyXgePumpInput()` path without application coordinate compensation.

Automated smoke mode:

```bat
build\xui_split_layout_dock_repro.exe --frames 4
```
