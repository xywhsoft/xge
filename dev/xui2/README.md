# XUI 重构工作区

这个目录用于准备 XUI 的独立重构环境。当前阶段只复制 XGE 引擎核心、平台 scaffold、第三方依赖和构建脚本，不包含旧版 XUI 实现代码。

## 当前内容

| 路径 | 说明 |
| --- | --- |
| `xge.h` | 已剥离旧版 XUI 声明的 XGE 公开头文件副本。 |
| `xui.h` | XUI 公开 API 入口；后续 UI API 只声明在这里，不进入 `xge.h`。 |
| `xge.c` | XGE 聚合编译入口。 |
| `src/` | XGE 核心实现副本，不包含 `xge_xui*` 文件和旧 XUI 聚合入口。 |
| `lib/` | xrt、sokol、stb、miniaudio 等依赖。 |
| `platform/` | Android、iOS、Web、小程序等平台 scaffold。 |
| `res/` | 构建 DLL 所需的基础图标资源。 |
| `test/` | 当前仅包含不依赖旧 XUI 的 XGE smoke test。 |
| `test_xui/` | 专门的 XUI 测试代码。 |
| `examples/` | XUI 代理层范例。 |
| `docs/` | XUI 架构设计文档。 |

## 构建验证

Windows / MinGW-w64 环境：

```bat
build_dll.bat
build_dbg_dll.bat
build_test.bat
build_dbg_test.bat
test_xui\build_context_test.bat
test_xui\build_widget_test.bat
test_xui\build_widget_type_test.bat
test_xui\build_style_test.bat
test_xui\build_layout_test.bat
test_xui\build_layout_callback_test.bat
test_xui\build_input_test.bat
test_xui\build_input_widget_test.bat
test_xui\build_numeric_input_test.bat
test_xui\build_text_test.bat
test_xui\build_render_schedule_test.bat
test_xui\build_proxy_xge_test.bat
test_xui\build_builtin_asset_test.bat
test_xui\build_checkbox_test.bat
test_xui\build_radio_test.bat
test_xui\build_toggle_test.bat
test_xui\build_scrollbar_test.bat
test_xui\build_slider_test.bat
test_xui\build_scroll_model_test.bat
test_xui\build_scroll_frame_test.bat
test_xui\build_scroll_view_test.bat
test_xui\build_list_view_test.bat
test_xui\build_tree_view_test.bat
test_xui\build_table_view_test.bat
test_xui\build_popup_test.bat
test_xui\build_menu_test.bat
test_xui\build_menubar_test.bat
test_xui\build_toolbar_test.bat
test_xui\build_statusbar_test.bat
test_xui\build_msgbox_test.bat
test_xui\build_msgtip_test.bat
test_xui\build_toast_test.bat
test_xui\build_combobox_test.bat
test_xui\build_color_picker_test.bat
test_xui\build_date_picker_test.bat
test_xui\build_panel_test.bat
test_xui\build_window_test.bat
test_xui\build_text_edit_test.bat
test_xui\build_label_test.bat
test_xui\build_button_test.bat
test_xui\build_image_test.bat
test_xui\build_separator_test.bat
test_xui\build_progress_test.bat
test_xui\build_timeline_view_test.bat
test_xui\build_dock_panel_test.bat
test_xui\build_chart_test.bat
examples\xui_proxy_surface\build.bat
examples\xui_proxy_shape\build.bat
examples\xui_proxy_font\build.bat
examples\xui_input_layer\build.bat
examples\xui_input\build.bat
examples\xui_numericinput\build.bat
examples\xui_label\build.bat
examples\xui_tooltip\build.bat
examples\xui_button\build.bat
examples\xui_checkbox\build.bat
examples\xui_radio\build.bat
examples\xui_toggle\build.bat
examples\xui_scrollbar\build.bat
examples\xui_slider\build.bat
examples\xui_scrollview\build.bat
examples\xui_listview\build.bat
examples\xui_treeview\build.bat
examples\xui_tableview\build.bat
examples\xui_tablegrid\build.bat
examples\xui_propertygrid\build.bat
examples\xui_timelineview\build.bat
examples\xui_dockpanel\build.bat
examples\xui_chart\build.bat
examples\xui_popup\build.bat
examples\xui_menu\build.bat
examples\xui_menubar\build.bat
examples\xui_toolbar\build.bat
examples\xui_statusbar\build.bat
examples\xui_msgbox\build.bat
examples\xui_msgtip\build.bat
examples\xui_toast\build.bat
examples\xui_combobox\build.bat
examples\xui_colorpicker\build.bat
examples\xui_datepicker\build.bat
examples\xui_panel\build.bat
examples\xui_window\build.bat
examples\xui_textedit\build.bat
examples\xui_image\build.bat
examples\xui_separator\build.bat
examples\xui_progress\build.bat
```

`build/` 目录为构建输出，已由根目录 `.gitignore` 忽略。

## 范例运行约定

XUI 范例默认不自动退出，适合手动运行和观察。需要自动化运行时，通过命令行参数指定时长：

```bat
build\xui_proxy_surface.exe --frames 360
build\xui_proxy_surface.exe --seconds 6
build\xui_proxy_shape.exe --frames 360
build\xui_proxy_font.exe --frames 360
build\xui_input_layer.exe --frames 360
build\xui_input.exe --frames 360
build\xui_numericinput.exe --frames 360
build\xui_label.exe --frames 360
build\xui_tooltip.exe --frames 360
build\xui_button.exe --frames 360
build\xui_checkbox.exe --frames 360
build\xui_radio.exe --frames 360
build\xui_toggle.exe --frames 360
build\xui_scrollbar.exe --frames 360
build\xui_slider.exe --frames 360
build\xui_scrollview.exe --frames 360
build\xui_listview.exe --frames 360
build\xui_treeview.exe --frames 360
build\xui_tableview.exe --frames 360
build\xui_tablegrid.exe --frames 360
build\xui_propertygrid.exe --frames 360
build\xui_timelineview.exe --frames 360
build\xui_dockpanel.exe --frames 360
build\xui_chart.exe --frames 360
build\xui_popup.exe --frames 360
build\xui_menu.exe --frames 360
build\xui_menubar.exe --frames 360
build\xui_toolbar.exe --frames 360
build\xui_statusbar.exe --frames 360
build\xui_msgbox.exe --frames 360
build\xui_msgtip.exe --frames 360
build\xui_toast.exe --frames 360
build\xui_combobox.exe --frames 360
build\xui_colorpicker.exe --frames 360
build\xui_datepicker.exe --frames 360
build\xui_panel.exe --frames 360
build\xui_window.exe --frames 360
build\xui_textedit.exe --frames 360
build\xui_image.exe --frames 360
build\xui_separator.exe --frames 360
build\xui_progress.exe --frames 360
```

