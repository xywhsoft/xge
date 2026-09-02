# 范例索引

每个范例目录都包含构建脚本和源码。Windows 下在仓库根目录执行相应范例的构建脚本；带可视界面的范例可使用 `--frames N` 或 `--seconds N` 自动退出（以范例自身帮助为准）。

## XGE

| 目录 | 主题 |
| --- | --- |
| `examples/xge_shape` | 基础形状绘制。 |
| `examples/xge_shape_ex`、`xge_shape_ex_*` | 路径、混合、特效、渐变、裁剪和绘制顺序。 |
| `examples/xge_svg` | SVG 加载与绘制。 |
| `examples/xge_emoji` | 文本中的 Emoji 显示。 |
| `examples/yuv420p_texture_lab` | YUV420P 纹理。 |
| `examples/tutorial_capture` | 教程用的最小渲染场景。 |

## XUI

`examples/xui_proxy_surface`、`examples/xui_proxy_shape` 与 `examples/xui_proxy_font` 演示 XGE/XUI 的接入层。其余 `examples/xui_*` 按控件命名，例如：

- 输入与编辑：`xui_input`、`xui_numericinput`、`xui_textedit`、`xui_codeedit`；
- 布局与容器：`xui_layout`、`xui_scrollview`、`xui_split_layout`、`xui_window`、`xui_dockpanel`；
- 数据控件：`xui_listview`、`xui_treeview`、`xui_tableview`、`xui_propertygrid`、`xui_chart`；
- 弹出与反馈：`xui_popup`、`xui_menu`、`xui_combobox`、`xui_msgbox`、`xui_toast`；
- 专用功能：`xui_flowgraph`、`xui_workflow_editor`、`xui_terminal`、`xui_qrcode`。

以范例中的公开 API 调用为准；`test_xui/` 适合用于验证行为和边界条件，而不是替代应用代码。

## 视觉审计程序

`examples/audit_*` 是固定场景的视觉审计程序，用于人工检查渲染质量以及生成截图回归素材。它们覆盖基础形状、ShapeEx、SVG、文字、相机、九宫格和多组 XUI 控件，不是对应单个 API 的入门范例。

审计程序默认创建可见窗口并持续运行，按 `Esc` 或关闭窗口退出。仍可使用原有自动审计参数：

```bat
build\audit_shape_basic.exe --frames 120
build\audit_shape_basic.exe --capture build\audit_shape_basic.png
```

指定 `--capture` 后，程序在成功保存离屏渲染结果后自动退出；截图内容与窗口预览使用同一个渲染目标。
