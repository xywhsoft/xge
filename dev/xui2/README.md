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
test_xui\build_text_test.bat
test_xui\build_render_schedule_test.bat
test_xui\build_proxy_xge_test.bat
test_xui\build_label_test.bat
test_xui\build_button_test.bat
test_xui\build_image_test.bat
test_xui\build_separator_test.bat
test_xui\build_progress_test.bat
examples\xui_proxy_surface\build.bat
examples\xui_proxy_shape\build.bat
examples\xui_proxy_font\build.bat
examples\xui_input\build.bat
examples\xui_label\build.bat
examples\xui_tooltip\build.bat
examples\xui_button\build.bat
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
build\xui_input.exe --frames 360
build\xui_label.exe --frames 360
build\xui_tooltip.exe --frames 360
build\xui_button.exe --frames 360
build\xui_image.exe --frames 360
build\xui_separator.exe --frames 360
build\xui_progress.exe --frames 360
```

