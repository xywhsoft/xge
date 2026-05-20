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
| Auto Pause Lab | `examples/auto_pause_lab` | `examples\auto_pause_lab\build.bat` | scene pause/resume、freeze snapshot、pause overlay 和 auto resume |
| Render Target Lab | `examples/render_target_lab` | `examples\render_target_lab\build.bat` | render target、pass、texture 和 readback |
| Shader Variant Lab | `examples/shader_variant_lab` | `examples\shader_variant_lab\build.bat` | shader 创建、uniform 和 variant define 缓存 |
| Material Mesh Buffer Lab | `examples/material_mesh_buffer_lab` | `examples\material_mesh_buffer_lab\build.bat` | buffer、mesh 和 material 绘制路径 |
| Blend Depth Color Lab | `examples/blend_depth_color_lab` | `examples\blend_depth_color_lab\build.bat` | 颜色打包、blend 和 depth test 对照 |
| Shape Full Gallery | `examples/shape_full_gallery` | `examples\shape_full_gallery\build.bat` | 完整 shape API、auto batch 和 shape batch 画廊 |
| Async Assets Lab | `examples/async_assets_lab` | `examples\async_assets_lab\build.bat` | async request、threading、poll 和资源异步加载 |
| Platform Runtime Lab | `examples/platform_runtime_lab` | `examples\platform_runtime_lab\build.bat` | runtime counters、高 DPI 和平台 smoke 面板 |
| Offscreen EGL Lab | `examples/offscreen_egl_lab` | `examples\offscreen_egl_lab\build.bat` | EGL capability、offscreen target 和 readback/fallback |
| Render Thread Lab | `examples/render_thread_lab` | `examples\render_thread_lab\build.bat` | render thread caps、worker drain、EGL-owned context 和窗口拒绝路径 |
| MiniProgram Bridge Lab | `examples/miniprogram_bridge_lab` | `examples\miniprogram_bridge_lab\build.bat` | miniprogram bridge、touch/text、request-frame、audio 和 `res://` provider |
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
| XUI Bridge | `examples/xui_bridge` | `examples\xui_bridge\build.bat` | XGE 和 XUI bridge |
| XUI Widget Tree Style Lab | `examples/xui_widget_tree_style_lab` | `examples\xui_widget_tree_style_lab\build.bat` | widget tree、style、rect/content rect 和 refresh/mark 自检 |
| XUI Layout Gallery | `examples/xui_layout_gallery` | `examples\xui_layout_gallery\build.bat` | absolute/row/column/stack/grid、size unit 和 dirty layout 画廊 |
| XUI Focus Capture Event Lab | `examples/xui_focus_capture_event_lab` | `examples\xui_focus_capture_event_lab\build.bat` | hit-test、event queue、focus/capture、Tab/Enter/Space/ESC 和 right-click context 自检 |
| XUI Paint Host Lab | `examples/xui_paint_host_lab` | `examples\xui_paint_host_lab\build.bat` | custom host、paint callback、clip set/clear 和 manual refresh 自检 |
| XUI Theme Lab | `examples/xui_theme_lab` | `examples\xui_theme_lab\build.bat` | default theme、theme set/get、DIP scale、styleFromTheme 和 light/dark 自检 |
| XUI Label | `examples/xui_label` | `examples\xui_label\build.bat` | Label 九宫格对齐、边框、背景、字体、禁用、下划线和缓存模式视觉验证 |
| XUI Label XSON | `examples/xui_label_xson` | `examples\xui_label_xson\build.bat` | 使用 XSON 描述并加载 Label 九宫格对齐、边框、背景、字体、禁用、下划线和缓存模式 |
| XUI Tooltip | `examples/xui_tooltip` | `examples\xui_tooltip\build.bat` | Widget 内置 tooltip 的文本、锚点、延迟、跟随鼠标、动态 resolver 和装备自定义 tooltip |
| XUI Tooltip XSON | `examples/xui_tooltip_xson` | `examples\xui_tooltip_xson\build.bat` | 使用 XSON 的 widget `tooltip` 属性展示文本提示、锚点、延迟和跟随鼠标 |
| XUI Image | `examples/xui_image` | `examples\xui_image\build.bat` | Image 九宫格对齐、边框、背景、裁剪、缩放和 customRect 视觉验证 |
| XUI Image XSON | `examples/xui_image_xson` | `examples\xui_image_xson\build.bat` | 使用 XSON 描述并加载 Image 九宫格对齐、边框、背景、裁剪、缩放和 customRect |
| XUI Separator | `examples/xui_separator` | `examples\xui_separator\build.bat` | Separator 横向/纵向、线宽、颜色、对齐和四种线型混排验证 |
| XUI Separator XSON | `examples/xui_separator_xson` | `examples\xui_separator_xson\build.bat` | 使用 XSON 描述并加载 Separator 横向/纵向、线宽、颜色、对齐和四种线型 |
| XUI Progress | `examples/xui_progress` | `examples\xui_progress\build.bat` | Progress 模板文字、配色、边框/透明背景和自动递增循环验证 |
| XUI Progress XSON | `examples/xui_progress_xson` | `examples\xui_progress_xson\build.bat` | 使用 XSON 描述并加载 Progress 模板文字、配色和自动递增循环 |
| XUI Basic Controls Lab | `examples/xui_basic_controls_lab` | `examples\xui_basic_controls_lab\build.bat` | Label/Image/Button 图标模式/Panel/Separator 的 setter 和 state 自检 |
| XUI Choice Controls Lab | `examples/xui_choice_controls_lab` | `examples\xui_choice_controls_lab\build.bat` | Toggle/CheckBox/RadioGroup/Radio/Switch 的 getter、state 和交互自检 |
| XUI List Scroll Lab | `examples/xui_list_scroll_lab` | `examples\xui_list_scroll_lab\build.bat` | ScrollView/ListView、disabled rows、hover、PageUp/PageDown 和 scroll 自检 |
| XUI HyperLink Lab | `examples/xui_hyperlink_lab` | `examples\xui_hyperlink_lab\build.bat` | HyperLink 语义迁移、visited/custom/transient/disabled 和 mouse/keyboard 激活自检 |
| XUI Tabs Dirty Rect Lab | `examples/xui_tabs_dirty_rect_lab` | `examples\xui_tabs_dirty_rect_lab\build.bat` | Tabs、dirty rect、content size、min/max measure 和 z-order hit test 自检 |
| XUI Text Buffer Lab | `examples/xui_text_buffer_lab` | `examples\xui_text_buffer_lab\build.bat` | xgeXuiText*、TextInputEvent、Input/TextEdit font setter 和 candidate rect 自检 |
| XUI Visual Proc Lab | `examples/xui_visual_proc_lab` | `examples\xui_visual_proc_lab\build.bat` | Label/Image/Button/Separator/Panel 的 procMeasure/procPaint/procEvent 自检 |
| XUI List View Proc Lab | `examples/xui_list_view_proc_lab` | `examples\xui_list_view_proc_lab\build.bat` | ListView 的 setter/getter、Event/EventProc/PaintProc、disabled row、wheel、drag 和键盘导航自检 |
| XUI Scroll View Proc Lab | `examples/xui_scroll_view_proc_lab` | `examples\xui_scroll_view_proc_lab\build.bat` | ScrollView 的 setter/getter、Event/EventProc/PaintProc、wheel、内容拖拽、竖向 page jump 和横向 thumb drag 自检 |
| XUI Input Proc Lab | `examples/xui_input_proc_lab` | `examples\xui_input_proc_lab\build.bat` | Input/TextEdit 的 Event/EventProc/UpdateProc/PaintProc、selection、candidate rect 和默认菜单自检 |
| XUI Input Policy Lab | `examples/xui_input_policy_lab` | `examples\xui_input_policy_lab\build.bat` | 单行 Input 的 placeholder、horizontal scroll、password、readonly、disabled 和 focus 规则自检 |
| XUI Input IME Policy Lab | `examples/xui_input_ime_policy_lab` | `examples\xui_input_ime_policy_lab\build.bat` | 单行 Input 的 IME start/update/end、candidate rect 移动和 placeholder + composition 行为自检 |
| XUI Input Nav Policy Lab | `examples/xui_input_nav_policy_lab` | `examples\xui_input_nav_policy_lab\build.bat` | 单行 Input 的 Ctrl+Left/Right、Home/End、selection collapse 和 scroll 跟随自检 |
| XUI Input Context Policy Lab | `examples/xui_input_context_policy_lab` | `examples\xui_input_context_policy_lab\build.bat` | 单行 Input 默认右键菜单的 enabled 规则、action 路径和 readonly/password/disabled guard 自检 |
| XUI Input Blink Focus Lab | `examples/xui_input_blink_focus_lab` | `examples\xui_input_blink_focus_lab\build.bat` | 单行 Input 的 focus ownership、caret blink toggle、失焦复位和焦点切换自检 |
| XUI Input Pointer Select Lab | `examples/xui_input_pointer_select_lab` | `examples\xui_input_pointer_select_lab\build.bat` | 单行 Input 的 click caret、drag selection、double-click word select 和 focus route 自检 |
| XUI Input Replace Policy Lab | `examples/xui_input_replace_policy_lab` | `examples\xui_input_replace_policy_lab\build.bat` | 单行 Input 的 typing replace、Ctrl+V replace、Delete replace 和 readonly guard 自检 |
| XUI Input Delete Policy Lab | `examples/xui_input_delete_policy_lab` | `examples\xui_input_delete_policy_lab\build.bat` | 单行 Input 的 Backspace、Delete、selection delete 和 readonly guard 自检 |
| XUI Input Clipboard Policy Lab | `examples/xui_input_clipboard_policy_lab` | `examples\xui_input_clipboard_policy_lab\build.bat` | 单行 Input 的 Ctrl+A/C/X/V 和 readonly clipboard guard 自检 |
| XUI Input Password Policy Lab | `examples/xui_input_password_policy_lab` | `examples\xui_input_password_policy_lab\build.bat` | 单行 Input 的 password mask、copy/cut guard、paste、readonly 和 candidate rect 自检 |
| XUI Button State Lab | `examples/xui_button_state_lab` | `examples\xui_button_state_lab\build.bat` | Button 及其图标模式的 hover、active、focus、disabled、capture 和 keyboard click 状态自检 |
| XUI Text Select Lab | `examples/xui_text_select_lab` | `examples\xui_text_select_lab\build.bat` | Input/TextEdit 的 cursor navigation、selection expand、Ctrl+A 和鼠标拖选自检 |
| XUI Text Delete Lab | `examples/xui_text_delete_lab` | `examples\xui_text_delete_lab\build.bat` | xgeXuiText*、Input/TextEdit 的 selection replace、Backspace/Delete 和 undo/redo 自检 |
| XUI Text Wrap Scroll Lab | `examples/xui_text_wrap_scroll_lab` | `examples\xui_text_wrap_scroll_lab\build.bat` | TextEdit 的 wrap、scroll、PageUp/PageDown、Home/End 和 Ctrl+Home/Ctrl+End 自检 |
| XUI Text Undo Stack Lab | `examples/xui_text_undo_stack_lab` | `examples\xui_text_undo_stack_lab\build.bat` | TextEdit 的 empty stack、direct undo/redo、selection restore、Ctrl+Z/Ctrl+Y 和 redo clear 自检 |
| XUI Text History Limit Lab | `examples/xui_text_history_limit_lab` | `examples\xui_text_history_limit_lab\build.bat` | TextEdit 的 `iUndoLimit` 上限、oldest state drop、redo 上限同步和 setText 清栈自检 |
| XUI Text Menu Detail Lab | `examples/xui_text_menu_detail_lab` | `examples\xui_text_menu_detail_lab\build.bat` | Input/TextEdit 默认右键菜单的 enabled 规则、readonly/password guard 和 SelectAll/Cut/Copy/Paste/Delete 自检 |
| XUI Combo Detail Lab | `examples/xui_combo_detail_lab` | `examples\xui_combo_detail_lab\build.bat` | ComboBox 的 selected/open/state、disabled row、popup height 和 keyboard open 自检 |
| XUI Combo Policy Lab | `examples/xui_combo_policy_lab` | `examples\xui_combo_policy_lab\build.bat` | ComboBox 的 popup/list sync、live height update、items reset 和 wide-layout reopen 自检 |
| XUI List View Detail Lab | `examples/xui_list_view_detail_lab` | `examples\xui_list_view_detail_lab\build.bat` | ListView 的 selection clear on disable、page jump、thumb drag 和 keyboard skip disabled rows 自检 |
| XUI Menu | `examples/xui_menu` | `examples\xui_menu\build.bat` | 新 Menu 的命令项、图标、勾选、radio、禁用、danger、子菜单和长菜单滚动 |
| XUI Menu XSON | `examples/xui_menu_xson` | `examples\xui_menu_xson\build.bat` | 新 Menu 的 XSON 加载、对象菜单项、默认弹出和 Popup 集成 |
| XUI Popup Owner Lab | `examples/xui_popup_owner_lab` | `examples\xui_popup_owner_lab\build.bat` | Popup 的 owner 绑定、SetAutoClose/SetBackground/SetClose、PopupEvent/EventProc 和 outside/ESC close 自检 |
| XUI Popup Policy Lab | `examples/xui_popup_policy_lab` | `examples\xui_popup_policy_lab\build.bat` | Popup 的 no-auto-close、outside-only、escape-only、owner passthrough、visible/focus 和 close callback 自检 |
| XUI Text Edit Lab | `examples/xui_text_edit_lab` | `examples\xui_text_edit_lab\build.bat` | Input/TextEdit、selection、clipboard、context menu、undo/redo、wrap 和 IME 自检 |
| Game Login Lab | `examples/game_login_lab` | `examples\game_login_lab\build.bat` | 登录面板、password input、preset menu、failure MsgBox 和 scene switch 综合自检 |

## 平台和离屏

| 示例 | 路径 | 构建脚本 | 说明 |
| --- | --- | --- | --- |
| Platform Smoke | `examples/platform_smoke` | `check_platform_smoke.bat` | 平台能力、runtime 和基础绘制冒烟 |
| Offscreen | `examples/offscreen` | 平台相关 | EGL/offscreen 和 readback |
| Offscreen EGL Lab | `examples/offscreen_egl_lab` | `examples\offscreen_egl_lab\build.bat` | Windows/GCC 下的 EGL 探测、fallback 和 readback 验证 |
| Render Thread Lab | `examples/render_thread_lab` | `examples\render_thread_lab\build.bat` | render thread worker drain、EGL 配置和当前 context 限制 |
| MiniProgram Bridge Lab | `examples/miniprogram_bridge_lab` | `examples\miniprogram_bridge_lab\build.bat` | 小程序宿主 bridge API、资源桥和 frame pump 自检 |
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
examples\render_thread_lab\build.bat
examples\miniprogram_bridge_lab\build.bat
examples\xui_widget_tree_style_lab\build.bat
examples\xui_layout_gallery\build.bat
examples\xui_focus_capture_event_lab\build.bat
examples\xui_paint_host_lab\build.bat
examples\xui_theme_lab\build.bat
examples\xui_basic_controls_lab\build.bat
examples\xui_choice_controls_lab\build.bat
examples\xui_list_scroll_lab\build.bat
examples\xui_menu\build.bat
examples\xui_tooltip\build.bat
examples\xui_tooltip_xson\build.bat
examples\xui_popup_owner_lab\build.bat
examples\xui_text_undo_stack_lab\build.bat
examples\xui_text_history_limit_lab\build.bat
examples\xui_text_menu_detail_lab\build.bat
examples\xui_combo_detail_lab\build.bat
examples\xui_combo_policy_lab\build.bat
examples\xui_list_view_detail_lab\build.bat
examples\xui_menu_xson\build.bat
examples\xui_popup_policy_lab\build.bat
examples\xui_split_layout_lab\build.bat
examples\xui_window_lab\build.bat
examples\xui_text_edit_lab\build.bat
examples\game_login_lab\build.bat
examples\mvp\build.bat
examples\shape\build.bat
examples\texture\build.bat
examples\text\build.bat
examples\render_target\build.bat
examples\audio\build.bat
check_platform_smoke.bat
```

平台后端实机结果应记录到 `dev/docs/平台后端验证结果.md`。
