# XGE Examples Index

本索引按能力域整理 `examples/` 下的所有范例，方便按“想看什么功能”快速定位目录。多数范例可在各自目录中运行 `build.bat` 构建；有 `.sh` 的目录也提供 Unix-like 平台构建入口。

## 基础运行与诊断

| 范例 | 功能说明 | 适合查找 |
| --- | --- | --- |
| `mvp` | 最小窗口示例，初始化 XGE、创建纹理、清屏并绘制。 | 第一个可运行程序、最小绘制闭环 |
| `core_lifecycle` | 演示初始化、手动帧、游戏循环、帧统计和退出条件。 | runtime 生命周期、manual/game loop 对照 |
| `app_mode` | 使用手动刷新模式驱动 XUI host，统计 refresh/paint/dirty。 | 工具型 App、按需刷新、手动 present |
| `platform_runtime_lab` | 展示平台 caps、runtime counters、高 DPI 和输入状态面板。 | 平台运行时信息、DPI、runtime 诊断 |
| `diagnostics_caps_log` | 输出平台、GPU、graphics mapping、debug stats 和日志信息。 | 能力报告、日志、GPU caps |
| `platform_smoke` | 跨平台基础冒烟，运行若干帧并输出 runtime/caps。 | 后端 smoke、平台构建验证 |
| `manual_validation` | 人工验证综合面板，覆盖输入、平台 runtime、帧统计和基础绘制。 | 手工回归、综合验证入口 |
| `debug_overlay` | 加载字体并绘制 debug stats 覆盖层。 | FPS/debug stats、调试面板 |

## 输入、场景与交互

| 范例 | 功能说明 | 适合查找 |
| --- | --- | --- |
| `input` | 读取键盘、鼠标、滚轮、按下/触发状态并绘制反馈。 | 基础键鼠输入 |
| `input_state_lab` | 系统性验证键盘、鼠标、触摸、剪贴板和模拟手柄状态。 | 输入状态 API、自检型输入范例 |
| `text_input` | 处理文本输入、按键、鼠标选择和 IME 事件。 | 文本输入、IME、编辑框基础 |
| `touch` | 展示多点触控 begin/move/end/cancel 状态。 | 触摸点、移动端输入 |
| `gamepad` | 读取手柄连接、按钮和摇杆轴状态。 | gamepad 输入 |
| `scene` | 使用 scene 栈、固定更新和鼠标/键盘切换状态。 | 场景栈、scene update/render |
| `scene_lifecycle_lab` | 验证 scene set/push/pop/replace 生命周期和 update policy。 | 场景生命周期自检 |
| `auto_pause_lab` | 演示 scene pause/resume、暂停覆盖层、freeze snapshot 和自动恢复。 | 暂停流程、后台/前台状态 |
| `game_login_lab` | 构建登录界面，包含密码输入、预设菜单、失败弹窗和场景切换。 | 综合 UI 流程、登录面板 |

## 渲染、纹理、文本与资源

| 范例 | 功能说明 | 适合查找 |
| --- | --- | --- |
| `shape` | 绘制点、线、矩形、圆、弧线、多边形，并演示 camera 坐标。 | shape API、2D 基础图元 |
| `shape_full_gallery` | 覆盖完整 shape API、shape batch、auto batch 和帧统计。 | shape 全量能力、批处理 |
| `texture` | 创建/加载纹理并使用不同 draw API、翻转和像素坐标绘制。 | texture 绘制、draw flags |
| `image_texture_lab` | 验证 image/texture 加载、更新、采样器、readback 和 fallback。 | image、texture、sampler |
| `text` | 加载字体并演示文本颜色、对齐、裁剪和多行绘制。 | 字体绘制、文本对齐 |
| `font_text_lab` | 覆盖 TTF/TTC/XRF/fallback、glyph metrics、bitmap 和中文范围。 | 字体系统、glyph 细节 |
| `sprite_batch` | 使用 sprite batch 批量绘制大量 sprite 并查看帧统计。 | 批量绘制、sprite 性能 |
| `render_target` | 将内容绘制到 render target，再作为纹理显示到窗口。 | 离屏渲染、render-to-texture |
| `render_target_lab` | 验证 render target/pass/window pass/readback/fallback。 | render pass、自检型 RT |
| `shader_material` | 创建自定义 shader 和 material，对纹理进行着色绘制。 | shader/material 基础 |
| `shader_distortion` | 使用自定义 shader 做动态扭曲效果。 | shader 特效、时间 uniform |
| `shader_variant_lab` | 验证 shader define、variant cache、uniform 和 material 组合。 | shader variant、宏变体 |
| `material_mesh_buffer_lab` | 创建 buffer、mesh、material 并绘制自定义几何。 | vertex/index buffer、mesh |
| `blend_depth_color_lab` | 对照颜色打包、blend mode、depth test 和 mesh 绘制结果。 | 混合、深度、颜色格式 |
| `perspective_quad` | 用 3D 顶点绘制带透视变换的 quad。 | 2.5D、透视贴图 |
| `isometric_depth` | 绘制等距 tile 并使用 depth test 控制遮挡顺序。 | 等距视角、depth ordering |
| `viewport_clip_camera` | 演示 viewport、clip rect、camera、坐标转换和透视 camera。 | 视口、裁剪、相机 |
| `resource_provider_lab` | 演示 memory/provider/mock xpack、resource/texture/font 加载。 | 自定义资源 provider、`res://` |
| `async` | 异步加载纹理并绘制进度、完成状态和 fallback 状态。 | async request、资源异步 |
| `async_assets_lab` | 异步加载 image/texture/font/sound，测试线程开关、poll、cancel。 | 异步资源全流程 |
| `audio` | 初始化音频，加载 sound，演示 group、loop、position 和播放停止。 | sound/music 基础 |
| `audio_lab` | 覆盖 sound/music/stream/group/listener/fade/fallback。 | 音频系统自检 |

## 平台、离屏与小程序

| 范例 | 功能说明 | 适合查找 |
| --- | --- | --- |
| `offscreen` | 板卡 Linux/EGL pbuffer 离屏渲染与 readback smoke。 | offscreen、EGL pbuffer |
| `offscreen_egl_lab` | Windows/GCC 下探测 EGL 能力，验证 offscreen target、readback 和 fallback。 | EGL/offscreen 自检 |
| `egl_surfaceless` | 创建 surfaceless EGL context 并验证目标驱动支持情况。 | EGL_KHR_surfaceless_context |
| `render_thread_lab` | 探测 render thread caps、worker drain、EGL-owned context 和窗口拒绝路径。 | 渲染线程、context 约束 |
| `miniprogram_bridge_lab` | 验证小程序 bridge、touch/text/request-frame/audio/resource provider。 | 小程序宿主桥、自检 |
| `miniprogram_hello` | 小程序工程 scaffold，含 `game.js`、`game.json` 和浏览器调试入口。 | 微信小游戏接入、JS bridge |

## XUI 基础、布局与标准控件

| 范例 | 功能说明 | 适合查找 |
| --- | --- | --- |
| `xui_bridge` | 将 XGE scene/event/render 循环接入 XUI context 和 widgets。 | XGE + XUI 集成入口 |
| `xui_incubation` | 早期孵化控件集合，展示基础控件组合方式。 | XUI 历史/试验控件 |
| `xui_widget_tree_style_lab` | 验证 widget tree、style、rect/content rect、anchor 和 dirty 标记。 | widget 树、样式、布局刷新 |
| `xui_layout_validation` | 验证 XUI 布局基础行为。 | 布局回归、自检 |
| `xui_layout_gallery` | 展示 absolute/row/column/stack/grid、size unit 和 dirty layout。 | 布局画廊 |
| `xui_split_layout_lab` | 构建可 resize 的分栏布局，处理 resize/quit 和文本区域。 | split layout、响应窗口尺寸 |
| `xui_window_lab` | 演示 XUI window/container 风格布局和可调整窗口内容。 | 窗口式 UI、容器 |
| `xui_paint_host_lab` | 自定义 host、paint callback、clip set/clear 和手动刷新。 | 自绘 host、clip、manual refresh |
| `xui_theme_lab` | 设置默认 theme、DIP scale、styleFromTheme 和 light/dark 对照。 | 主题、缩放、样式 |
| `xui_basic_controls_lab` | Label/Image/Button 图标模式/Panel/Separator 的 setter 和状态自检。 | 基础控件 API |
| `xui_standard_controls_lab` | 标准控件组合展示，覆盖按钮、基础交互和 scene 集成。 | 标准控件总览 |
| `xui_all_controls_gallery` | 完整 XUI 控件画廊，集中展示基础、输入、选择、数值、数据、布局和浮层控件。 | 检查所有控件样式与交互 |
| `xui_button_state_lab` | Button 及其图标模式的 hover、active、focus、disabled、capture 和键盘点击。 | 按钮状态机 |
| `xui_button_standard_lab` | 标准 Button 行为，覆盖鼠标、键盘、触摸、图标混排和选择状态。 | 标准按钮控件 |
| `xui_choice_controls_lab` | Toggle/CheckBox/RadioGroup/Radio/Switch 的 getter、state 和交互。 | 选择类控件 |
| `xui_choice_standard_lab` | 标准选择控件行为验证。 | checkbox/radio/switch 标准路径 |
| `xui_value_controls_lab` | Slider/Progress/Splitter/ScrollBar 的 range/page/value/orientation/state。 | 数值控件、拖拽控件 |
| `xui_data_controls_lab` | 数据类控件集合展示。 | 数据展示控件入口 |
| `xui_numeric_input_lab` | 数字输入控件，处理范围、步进和文本到数值转换。 | numeric input、数值编辑 |
| `xui_color_picker_lab` | 颜色选择控件，展示颜色值编辑和预览。 | color picker |
| `xui_toolbar_lab` | 工具栏按钮、状态、鼠标/键盘交互和布局。 | toolbar |
| `xui_status_bar_lab` | 状态栏控件，展示状态项、点击和更新。 | status bar |
| `xui_message_box_lab` | 消息框控件，展示按钮、结果和关闭流程。 | message box |
| `xui_toast_lab` | Toast 通知控件，展示类型、显示位置和生命周期。 | toast 通知 |
| `xui_hyperlink_lab` | HyperLink 的 visited/custom/transient/disabled 和鼠标/键盘激活。 | 超链接控件 |
| `xui_breadcrumb_lab` | 面包屑导航控件，处理 segment、点击和键盘行为。 | breadcrumb |
| `xui_accordion_lab` | 手风琴控件，展示 section 展开/折叠和选择模式。 | accordion |
| `xui_property_grid_lab` | 属性表控件，展示属性项、编辑器和可见项。 | property grid |
| `xui_table_view_lab` | 表格控件，展示行列、选择、滚动和事件。 | table view |
| `xui_tree_view_lab` | 树控件，展示展开/折叠、选择和键盘导航。 | tree view |

## XUI 列表、滚动、菜单、弹层与对话框

| 范例 | 功能说明 | 适合查找 |
| --- | --- | --- |
| `xui_list_scroll_lab` | ScrollView/ListView、disabled rows、hover、PageUp/PageDown 和 scroll。 | 列表滚动 |
| `xui_list_standard_lab` | ListView 标准选择行为，覆盖 multi/range、Ctrl/Shift 和 renderer。 | 标准列表选择 |
| `xui_virtual_list_standard_lab` | VirtualList 可见槽位、first visible、键盘滚动和 clipping。 | 虚拟列表 |
| `xui_list_view_proc_lab` | ListView setter/getter、Event/EventProc/PaintProc、wheel、drag 和键盘导航。 | ListView proc 自检 |
| `xui_list_view_detail_lab` | ListView 选择清理、page jump、thumb drag 和跳过 disabled rows。 | ListView 细节策略 |
| `xui_scroll_standard_lab` | ScrollView/ScrollBar 标准嵌套滚动策略、隐藏/自动 policy。 | 标准滚动行为 |
| `xui_scroll_view_proc_lab` | ScrollView setter/getter、proc、wheel、内容拖拽和 thumb drag。 | ScrollView proc 自检 |
| `xui_overlay_menu_lab` | Popup/Tooltip/ComboBox/Menu/Dialog 的 modal、outside close、ESC 和 disabled item。 | 弹层菜单综合 |
| `xui_overlay_proc_lab` | Popup/ComboBox/Dialog 以及内部 ListView 的 Event/EventProc/PaintProc。 | overlay proc 自检 |
| `xui_overlay_policy_lab` | Overlay 关闭、焦点、可见状态和策略行为。 | overlay 策略 |
| `xui_menu_owner_lab` | Menu owner 绑定、items/enabled/size/colors、disabled row、outside close 和 focus return。 | Menu owner |
| `xui_menu_policy_lab` | Menu open reset、重复 reopen、size update、clamp、focus return 和 close 幂等。 | Menu 策略 |
| `xui_popup_owner_lab` | Popup owner 绑定、auto close、background、close callback 和 outside/ESC close。 | Popup owner |
| `xui_popup_policy_lab` | Popup no-auto-close、outside-only、escape-only、owner passthrough 和 focus。 | Popup 策略 |
| `xui_tooltip_owner_lab` | Tooltip owner capture 链、text/offset/enabled/open 和 OwnerEventProc。 | Tooltip owner |
| `xui_tooltip_policy_lab` | Tooltip open 门控、live text/offset relayout、owner event open-close 和 capture。 | Tooltip 策略 |
| `xui_dialog_modal_lab` | Dialog title/modal/escape/showClose/colors、关闭按钮和 modal block。 | Modal dialog |
| `xui_dialog_policy_lab` | Dialog modal/non-modal、closeOnEscape、showClose、事件消费和 reopen。 | Dialog 策略 |
| `xui_combo_detail_lab` | ComboBox selected/open/state、disabled row、dropdown height 和 keyboard open。 | ComboBox 细节 |
| `xui_combo_policy_lab` | ComboBox popup/list sync、live height update、items reset 和 wide-layout reopen。 | ComboBox 策略 |

## XUI 输入与文本编辑

| 范例 | 功能说明 | 适合查找 |
| --- | --- | --- |
| `xui_input_validation` | XUI Input 基础验证示例。 | 输入控件回归 |
| `xui_input_proc_lab` | Input/TextEdit Event/EventProc/UpdateProc/PaintProc、selection、candidate rect 和默认菜单。 | 输入 proc 自检 |
| `xui_input_standard_lab` | 单行 Input 标准行为验证。 | 标准 input |
| `xui_input_policy_lab` | 单行 Input placeholder、horizontal scroll、password、readonly、disabled 和 focus。 | Input 基础策略 |
| `xui_input_ime_policy_lab` | 单行 Input IME start/update/end、candidate rect 和 composition 行为。 | IME 策略 |
| `xui_input_nav_policy_lab` | Ctrl+Left/Right、Home/End、selection collapse 和 scroll 跟随。 | 光标导航 |
| `xui_input_context_policy_lab` | 默认右键菜单 enabled 规则、action 路径和 readonly/password/disabled guard。 | 输入上下文菜单 |
| `xui_input_blink_focus_lab` | focus ownership、caret blink toggle、失焦复位和焦点切换。 | 光标闪烁、焦点 |
| `xui_input_pointer_select_lab` | click caret、drag selection、double-click word select 和 focus route。 | 鼠标选择 |
| `xui_input_replace_policy_lab` | typing replace、Ctrl+V replace、Delete replace 和 readonly guard。 | 替换策略 |
| `xui_input_delete_policy_lab` | Backspace、Delete、selection delete 和 readonly guard。 | 删除策略 |
| `xui_input_clipboard_policy_lab` | Ctrl+A/C/X/V 和 readonly clipboard guard。 | 剪贴板策略 |
| `xui_input_password_policy_lab` | password mask、copy/cut guard、paste、readonly 和 candidate rect。 | 密码输入 |
| `xui_text_buffer_lab` | xgeXuiText*、TextInputEvent、font setter 和 candidate rect。 | 文本缓冲层 |
| `xui_text_select_lab` | Input/TextEdit 光标导航、selection expand、Ctrl+A 和鼠标拖选。 | 文本选择 |
| `xui_text_delete_lab` | selection replace、Backspace/Delete 和 undo/redo。 | 文本删除、撤销 |
| `xui_text_wrap_scroll_lab` | TextEdit wrap、scroll、PageUp/PageDown、Home/End 和 Ctrl+Home/End。 | 多行滚动文本 |
| `xui_text_undo_stack_lab` | empty stack、direct undo/redo、selection restore、Ctrl+Z/Ctrl+Y 和 redo clear。 | undo stack |
| `xui_text_history_limit_lab` | `iUndoLimit` 上限、oldest state drop、redo 上限同步和 setText 清栈。 | 撤销历史限制 |
| `xui_text_menu_detail_lab` | Input/TextEdit 默认右键菜单 enabled 规则、readonly/password guard 和编辑动作。 | 文本右键菜单 |
| `xui_text_edit_lab` | Input/TextEdit、selection、clipboard、context menu、undo/redo、wrap 和 IME 综合验证。 | 文本编辑综合 |
| `xui_text_edit_standard_lab` | TextEdit 标准文本、Tab、Ctrl 快捷键和高亮行为。 | 标准 TextEdit |
| `xui_text_edit_validation` | TextEdit 验证场景，覆盖基本编辑和快捷键。 | TextEdit 回归 |

## XUI Proc 与事件路由

| 范例 | 功能说明 | 适合查找 |
| --- | --- | --- |
| `xui_focus_capture_event_lab` | hit-test、event queue、focus/capture、Tab/Enter/Space/ESC 和右键上下文。 | 焦点、捕获、事件路由 |
| `xui_visual_proc_lab` | Label/Image/Button/Separator/Panel 的 measure/paint/event proc。 | visual 控件 proc |
| `xui_control_proc_lab` | Toggle/CheckBox/Radio/Switch/Slider/ScrollBar/Splitter/Progress 的 proc。 | control proc |
| `xui_tabs_dirty_rect_lab` | Tabs、dirty rect、content size、min/max measure 和 z-order hit test。 | Tabs、dirty rect |
| `xui_tabs_standard_lab` | Tabs 标准鼠标、键盘、滚轮和焦点行为。 | 标准 Tabs |

## XUI XSON 声明式页面

| 范例 | 功能说明 | 适合查找 |
| --- | --- | --- |
| `xui_xson_page_lab` | 从内存加载 XSON page，查找 root/widget 并验证错误状态。 | XSON page 基础 |
| `xui_xson_style_lab` | XSON 中声明 row/column、justify、按钮样式和控件查找。 | XSON style |
| `xui_xson_app_layout_lab` | 使用 XSON dock layout 组织 top/left/bottom/fill 和 scroll view。 | XSON app layout |
| `xui_xson_layout_gallery_lab` | XSON size px/percent/grow/content 和布局更新。 | XSON 布局画廊 |
| `xui_xson_controls_lab` | XSON 加载多类控件并读取 accordion/breadcrumb/table/property/toast/tree 状态。 | XSON 控件总览 |
| `xui_xson_accordion_lab` | XSON Accordion section 和 single 模式。 | XSON accordion |
| `xui_xson_breadcrumb_lab` | XSON Breadcrumb segment 和场景集成。 | XSON breadcrumb |
| `xui_xson_property_grid_lab` | XSON PropertyGrid、颜色编辑器和 visible count。 | XSON property grid |
| `xui_xson_table_view_lab` | XSON TableView 行列、对齐和页面加载。 | XSON table view |
| `xui_xson_toast_lab` | XSON Toast placement/type/count 和通知显示。 | XSON toast |
| `xui_xson_virtual_list_lab` | XSON VirtualList 可见项、slot widget 和滚动更新。 | XSON virtual list |
