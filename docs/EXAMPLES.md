# XGE 示例说明

> 状态：中文初稿已生成，待审阅。

本页按学习目标整理 `examples/` 目录。每个示例都有对应构建脚本，Windows 使用 `.bat`，Unix-like 平台使用 `.sh`。
Windows 下可从根目录运行 `build_examples_all.bat` 一次性编译全部本地示例；单个示例也可以运行对应目录内的 `build.bat`。

## 基础示例

| 示例 | 路径 | 构建脚本 | 说明 |
| --- | --- | --- | --- |
| Core Lifecycle | `examples/core_lifecycle` | `examples\core_lifecycle\build.bat` | 初始化、手动帧、窗口主循环和 frame stats |
| Diagnostics Caps Log | `examples/diagnostics_caps_log` | `examples\diagnostics_caps_log\build.bat` | 日志、平台能力、GPU caps 和 runtime counters |
| Viewport Clip Camera | `examples/viewport_clip_camera` | `examples\viewport_clip_camera\build.bat` | viewport、clip、camera 和坐标转换 |
| Image Texture Lab | `examples/image_texture_lab` | `examples\image_texture_lab\build.bat` | image/texture 加载、更新、采样和 readback |
| Font Text Lab | `examples/font_text_lab` | `examples\font_text_lab\build.bat` | TTF/TTC、XRF、fallback、glyph 和文本绘制 |
| Resource Provider Lab | `examples/resource_provider_lab` | `examples\resource_provider_lab\build.bat` | mem/provider、mock xpack、resource/texture/font 加载 |
| Input State Lab | `examples/input_state_lab` | `examples\input_state_lab\build.bat` | 键盘、鼠标、剪贴板、触摸和模拟手柄状态 |
| Audio Lab | `examples/audio_lab` | `examples\audio_lab\build.bat` | sound/music/stream、group、listener、fade 和 fallback |
| Scene Lifecycle Lab | `examples/scene_lifecycle_lab` | `examples\scene_lifecycle_lab\build.bat` | scene set/push/pop/replace、生命周期计数和更新策略 |
| Render Target Lab | `examples/render_target_lab` | `examples\render_target_lab\build.bat` | render target、pass、texture 和 readback |
| Shader Variant Lab | `examples/shader_variant_lab` | `examples\shader_variant_lab\build.bat` | shader 创建、uniform 和 variant define 缓存 |
| Material Mesh Buffer Lab | `examples/material_mesh_buffer_lab` | `examples\material_mesh_buffer_lab\build.bat` | buffer、mesh 和 material 绘制路径 |
| Blend Depth Color Lab | `examples/blend_depth_color_lab` | `examples\blend_depth_color_lab\build.bat` | 颜色打包、blend 和 depth test 对照 |
| Shape Full Gallery | `examples/shape_full_gallery` | `examples\shape_full_gallery\build.bat` | 完整 shape API、auto batch 和 shape batch 画廊 |
| Async Assets Lab | `examples/async_assets_lab` | `examples\async_assets_lab\build.bat` | async request、threading、poll 和资源异步加载 |
| Platform Runtime Lab | `examples/platform_runtime_lab` | `examples\platform_runtime_lab\build.bat` | runtime counters、高 DPI 和平台 smoke 面板 |
| Offscreen EGL Lab | `examples/offscreen_egl_lab` | `examples\offscreen_egl_lab\build.bat` | EGL capability、offscreen target 和 readback/fallback |
| MVP | `examples/mvp` | `examples\mvp\build.bat` | 最小窗口、清屏和基础绘制 |
| Shape | `examples/shape` | `examples\shape\build.bat` | 点、线、矩形、圆、弧线、多边形 |
| Texture | `examples/texture` | `examples\texture\build.bat` | 纹理加载和绘制 |
| Text | `examples/text` | `examples\text\build.bat` | TTF/XRF 字体和文本绘制 |
| Sprite Batch | `examples/sprite_batch` | `examples\sprite_batch\build.bat` | 批量 sprite 绘制 |

## 渲染管线

| 示例 | 路径 | 构建脚本 | 说明 |
| --- | --- | --- | --- |
| Render Target | `examples/render_target` | `examples\render_target\build.bat` | 离屏绘制再显示 |
| Shader Material | `examples/shader_material` | `examples\shader_material\build.bat` | 自定义 shader/material |
| Shader Distortion | `examples/shader_distortion` | `examples\shader_distortion\build.bat` | 扭曲效果 |
| Perspective Quad | `examples/perspective_quad` | `examples\perspective_quad\build.bat` | 2.5D 透视 quad |
| Isometric Depth | `examples/isometric_depth` | `examples\isometric_depth\build.bat` | 等距视角 depth/order |

## 输入、场景和 APP 模式

| 示例 | 路径 | 构建脚本 | 说明 |
| --- | --- | --- | --- |
| Input | `examples/input` | `examples\input\build.bat` | 键盘、鼠标、滚轮 |
| Text Input | `examples/text_input` | `examples\text_input\build.bat` | 文本输入和系统 IME 结果 |
| Touch | `examples/touch` | `examples\touch\build.bat` | 多点触控状态 |
| Gamepad | `examples/gamepad` | `examples\gamepad\build.bat` | 手柄状态 |
| Scene | `examples/scene` | `examples\scene\build.bat` | 场景栈和生命周期 |
| App Mode | `examples/app_mode` | `examples\app_mode\build.bat` | 手动刷新和 APP 模式 |

## 资源、音频、调试和 GUI

| 示例 | 路径 | 构建脚本 | 说明 |
| --- | --- | --- | --- |
| Async | `examples/async` | `examples\async\build.bat` | 异步资源和 fallback |
| Audio | `examples/audio` | `examples\audio\build.bat` | sound/music/3D audio |
| Debug Overlay | `examples/debug_overlay` | `examples\debug_overlay\build.bat` | debug stats |
| XUI Incubation | `examples/xui_incubation` | `examples\xui_incubation\build.bat` | 孵化期 XUI 控件 |
| XUI Bridge | `examples/xui_bridge` | `examples\xui_bridge\build.bat` | XGE 和 XUI bridge |

## 平台和离屏

| 示例 | 路径 | 构建脚本 | 说明 |
| --- | --- | --- | --- |
| Platform Smoke | `examples/platform_smoke` | `check_platform_smoke.bat` | 平台能力、runtime 和基础绘制冒烟 |
| Offscreen | `examples/offscreen` | 平台相关 | EGL/offscreen 和 readback |
| Offscreen EGL Lab | `examples/offscreen_egl_lab` | `examples\offscreen_egl_lab\build.bat` | Windows/GCC 下的 EGL 探测、fallback 和 readback 验证 |
| EGL Surfaceless | `examples/egl_surfaceless` | `examples/egl_surfaceless/build.sh` | EGL surfaceless 冒烟 |
| Mini Program Hello | `examples/miniprogram_hello` | `examples\miniprogram_hello\build.bat` | 小程序 scaffold |

## 建议验证顺序

```bat
build_test.bat
build_examples_all.bat
examples\core_lifecycle\build.bat
examples\diagnostics_caps_log\build.bat
examples\viewport_clip_camera\build.bat
examples\image_texture_lab\build.bat
examples\font_text_lab\build.bat
examples\resource_provider_lab\build.bat
examples\input_state_lab\build.bat
examples\audio_lab\build.bat
examples\scene_lifecycle_lab\build.bat
examples\render_target_lab\build.bat
examples\shader_variant_lab\build.bat
examples\material_mesh_buffer_lab\build.bat
examples\blend_depth_color_lab\build.bat
examples\shape_full_gallery\build.bat
examples\async_assets_lab\build.bat
examples\platform_runtime_lab\build.bat
examples\offscreen_egl_lab\build.bat
examples\mvp\build.bat
examples\shape\build.bat
examples\texture\build.bat
examples\text\build.bat
examples\render_target\build.bat
examples\audio\build.bat
check_platform_smoke.bat
```

平台后端实机结果应记录到 `dev/docs/平台后端验证结果.md`。
