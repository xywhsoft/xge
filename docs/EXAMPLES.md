# XGE 示例说明

> 状态：中文初稿已生成，待审阅。

本页按学习目标整理 `examples/` 目录。每个示例都有对应构建脚本，Windows 使用 `.bat`，Unix-like 平台使用 `.sh`。

## 基础示例

| 示例 | 路径 | 构建脚本 | 说明 |
| --- | --- | --- | --- |
| MVP | `examples/mvp` | `build_exe.bat` | 最小窗口、清屏和基础绘制 |
| Shape | `examples/shape` | `build_shape_exe.bat` | 点、线、矩形、圆、弧线、多边形 |
| Texture | `examples/texture` | `build_texture_exe.bat` | 纹理加载和绘制 |
| Text | `examples/text` | `build_text_exe.bat` | TTF/XRF 字体和文本绘制 |
| Sprite Batch | `examples/sprite_batch` | `build_sprite_batch_exe.bat` | 批量 sprite 绘制 |

## 渲染管线

| 示例 | 路径 | 构建脚本 | 说明 |
| --- | --- | --- | --- |
| Render Target | `examples/render_target` | `build_render_target_exe.bat` | 离屏绘制再显示 |
| Shader Material | `examples/shader_material` | `build_shader_material_exe.bat` | 自定义 shader/material |
| Shader Distortion | `examples/shader_distortion` | `build_shader_distortion_exe.bat` | 扭曲效果 |
| Perspective Quad | `examples/perspective_quad` | `build_perspective_quad_exe.bat` | 2.5D 透视 quad |
| Isometric Depth | `examples/isometric_depth` | `build_isometric_depth_exe.bat` | 等距视角 depth/order |

## 输入、场景和 APP 模式

| 示例 | 路径 | 构建脚本 | 说明 |
| --- | --- | --- | --- |
| Input | `examples/input` | `build_input_exe.bat` | 键盘、鼠标、滚轮 |
| Text Input | `examples/text_input` | `build_text_input_exe.bat` | 文本输入和系统 IME 结果 |
| Touch | `examples/touch` | `build_touch_exe.bat` | 多点触控状态 |
| Gamepad | `examples/gamepad` | `build_gamepad_exe.bat` | 手柄状态 |
| Scene | `examples/scene` | `build_scene_exe.bat` | 场景栈和生命周期 |
| App Mode | `examples/app_mode` | `build_app_mode_exe.bat` | 手动刷新和 APP 模式 |

## 资源、音频、调试和 GUI

| 示例 | 路径 | 构建脚本 | 说明 |
| --- | --- | --- | --- |
| Async | `examples/async` | `build_async_exe.bat` | 异步资源和 fallback |
| Audio | `examples/audio` | `build_audio_exe.bat` | sound/music/3D audio |
| Debug Overlay | `examples/debug_overlay` | `build_debug_overlay_exe.bat` | debug stats |
| XUI Incubation | `examples/xui_incubation` | `build_xui_incubation_exe.bat` | 孵化期 XUI 控件 |
| XUI Bridge | `examples/xui_bridge` | `build_xui_bridge_exe.bat` | XGE 和 XUI bridge |

## 平台和离屏

| 示例 | 路径 | 构建脚本 | 说明 |
| --- | --- | --- | --- |
| Platform Smoke | `examples/platform_smoke` | `check_platform_smoke.bat` | 平台能力、runtime 和基础绘制冒烟 |
| Offscreen | `examples/offscreen` | 平台相关 | EGL/offscreen 和 readback |
| EGL Surfaceless | `examples/egl_surfaceless` | `build_egl_surfaceless_exe.sh` | EGL surfaceless 冒烟 |
| Mini Program Hello | `examples/miniprogram_hello` | `build_miniprogram.bat` | 小程序 scaffold |

## 建议验证顺序

```bat
build_test.bat
build_exe.bat
build_shape_exe.bat
build_texture_exe.bat
build_text_exe.bat
build_render_target_exe.bat
build_audio_exe.bat
check_platform_smoke.bat
```

平台后端实机结果应记录到 `dev/docs/平台后端验证结果.md`。
