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

## XUI 基础设施与布局

| 范例 | 功能说明 | 适合查找 |
| --- | --- | --- |
| `xui_bridge` | 将 XGE scene/event/render 循环接入 XUI context 和 widgets。 | XGE + XUI 集成入口 |
| `xui_widget_tree_style_lab` | 验证 widget tree、style、rect/content rect、anchor 和 dirty 标记。 | widget 树、样式、布局刷新 |
| `xui_layout_validation` | 验证 XUI 布局基础行为。 | 布局回归、自检 |
| `xui_layout_gallery` | 展示 absolute/row/column/stack/grid、size unit 和 dirty layout。 | 布局画廊 |
| `xui_focus_capture_event_lab` | hit-test、event queue、focus/capture、Tab/Enter/Space/ESC 和右键上下文。 | 焦点、捕获、事件路由 |
| `xui_paint_host_lab` | 自定义 host、paint callback、clip set/clear 和手动刷新。 | 自绘 host、clip、manual refresh |
| `xui_theme_lab` | 设置默认 theme、DIP scale、styleFromTheme 和 light/dark 对照。 | 主题、缩放、样式 |

## XUI 新范式控件范例

| 控件/能力 | 范例 | 功能说明 |
| --- | --- | --- |
| Label | `xui_label`, `xui_label_xson` | 文本、对齐、字体、边框背景、禁用、下划线和缓存渲染。 |
| Tooltip | `xui_tooltip`, `xui_tooltip_xson` | Widget 内置 tooltip，覆盖静态文本、对象写法、跟随鼠标和 resolver。 |
| Image | `xui_image`, `xui_image_xson` | 图片显示、填充模式、透明度、裁剪和 XSON 加载。 |
| Separator | `xui_separator`, `xui_separator_xson` | 水平/垂直分割线、颜色、厚度和布局行为。 |
| Progress | `xui_progress`, `xui_progress_xson` | 进度显示、方向、文本、颜色和状态加载。 |
| Button | `xui_button`, `xui_button_xson` | 文本/图标按钮、状态色、选择态、贴图、九宫格和徽标。 |
| Choice | `xui_choice`, `xui_choice_xson` | CheckBox、Radio、Toggle 的 checked、disabled、自定义颜色、尺寸和缓存。 |
| Input | `xui_input`, `xui_input_xson` | 单行输入、placeholder、密码、装饰区、错误态、选择和对齐。 |
| TextEdit | `xui_textedit`, `xui_textedit_xson` | 多行文本、滚动、换行、选择、编辑状态和 XSON 加载。 |
| NumericInput | `xui_numericinput`, `xui_numericinput_xson` | 数值输入、范围、步进、spinner、滚轮、精度和错误态。 |
| Slider | `xui_slider`, `xui_slider_xson` | 水平/垂直滑块、禁用、自定义尺寸/颜色和缓存路径。 |
| ScrollBar | `xui_scrollbar`, `xui_scrollbar_xson` | 完整/精简滚动条、方向、拖动、按钮和 XSON 状态。 |
| ScrollView | `xui_scrollview`, `xui_scrollview_xson` | 基于 ScrollModel/ScrollFrame 的真实内容视口映射和两种滚动条样式。 |
| Panel | `xui_panel`, `xui_panel_xson` | 容器边框、背景、标题和布局承载。 |
| SplitLayout | `xui_split_layout`, `xui_split_layout_xson` | 分栏布局、拖拽调整、最小尺寸和 XSON 加载。 |
| Tabs | `xui_tabs`, `xui_tabs_xson` | 标签页、选中态、内容页、键鼠交互和 XSON 加载。 |
| Window | `xui_window`, `xui_window_xson` | 窗口控件、标题栏、关闭/折叠/最大化、拖动和内容区域。 |
| Popup | `xui_popup`, `xui_popup_xson` | 通用弹层、锚点/方向组合、回退定位、溢出滚动和遮罩策略。 |
| Menu | `xui_menu`, `xui_menu_xson` | 命令项、图标、勾选、radio、禁用、danger、子菜单和长菜单滚动。 |
| MenuBar | `xui_menubar`, `xui_menubar_xson` | 顶部菜单栏、菜单联动、键鼠交互和 XSON 加载。 |
| ComboBox | `xui_combobox`, `xui_combobox_xson` | 下拉单选、结构化条目、禁用项、固定高度和自动弹出方向。 |
| ColorPicker | `xui_colorpicker`, `xui_colorpicker_xson` | 颜色选择、输入框、滑块、预览和全局弹层复用。 |
| DatePicker | `xui_datepicker`, `xui_datepicker_xson` | 日期/时间/范围选择、格式化、min/max、秒显示和弹层表单。 |
| MsgBox/InputBox | `xui_msgbox_inputbox` | 消息框、输入框、按钮结果、modal/floating；便捷弹窗不作为 XSON 页面节点。 |
| ListView | `xui_listview`, `xui_listview_xson` | 基于 ScrollFrame 的列表、虚拟行、滚动条、单选/多选/Ctrl/Shift。 |
| TreeView | `xui_treeview`, `xui_treeview_xson` | 展开/折叠、选择、禁用、勾选、虚拟行、滚动条和自定义行绘制。 |
| TableView | `xui_tableview`, `xui_tableview_xson` | 静态表格、列宽、行/单元格选择、横纵滚动、合并单元格和自定义渲染。 |
| TableGrid | `xui_tablegrid`, `xui_tablegrid_xson` | 可编辑表格，覆盖文本/数字/bool、textarea、enum、color、date/time、picker/file/image/custom 回调、大表滚动和 XSON 静态数据。 |
| TimelineView | `xui_timelineview`, `xui_timelineview_xson` | 时间轴编辑控件，覆盖图层、显式帧、关键帧、span、锁定/隐藏、选择、滚动、XSON 加载和自定义渲染。 |
| Accordion | `xui_accordion`, `xui_accordion_xson` | section 展开/折叠、单开模式、禁用项和 XSON 加载。 |

## XUI 保留专项与回归范例

| 范例 | 功能说明 | 适合查找 |
| --- | --- | --- |
| `xui_input_proc_lab` | Input/TextEdit Event/EventProc/UpdateProc/PaintProc、selection、candidate rect 和默认菜单。 | 输入 proc 自检 |
| `xui_input_policy_lab`, `xui_input_ime_policy_lab`, `xui_input_nav_policy_lab` | placeholder、horizontal scroll、password、readonly、IME、candidate rect、Ctrl 导航和 selection collapse。 | Input 基础/IME/导航策略 |
| `xui_input_context_policy_lab`, `xui_input_blink_focus_lab`, `xui_input_pointer_select_lab` | 默认右键菜单、focus ownership、caret blink、click/drag/double-click selection。 | 输入上下文、焦点、鼠标选择 |
| `xui_input_replace_policy_lab`, `xui_input_delete_policy_lab`, `xui_input_clipboard_policy_lab`, `xui_input_password_policy_lab` | typing replace、Backspace/Delete、剪贴板、密码 mask 和安全 guard。 | 输入编辑策略 |
| `xui_text_buffer_lab`, `xui_text_select_lab`, `xui_text_delete_lab`, `xui_text_wrap_scroll_lab` | 文本缓冲、光标选择、删除/撤销、TextEdit wrap 和滚动策略。 | 文本基础能力 |
| `xui_text_undo_stack_lab`, `xui_text_history_limit_lab`, `xui_text_menu_detail_lab`, `xui_text_edit_lab` | undo/redo、历史上限、默认右键菜单和文本编辑综合验证。 | TextEdit 细节回归 |
| `xui_popup_owner_lab`, `xui_popup_policy_lab` | Popup owner 绑定、auto close、outside/ESC close、passthrough 和 focus 策略。 | Popup 旧策略回归 |
| `xui_visual_proc_lab`, `xui_tabs_dirty_rect_lab` | visual 控件 measure/paint/event proc、Tabs dirty rect、content size 和 z-order hit test。 | 自绘 proc、脏矩形 |
| `xui_toolbar_lab`, `xui_status_bar_lab`, `xui_toast_lab`, `xui_hyperlink_lab` | 尚未进入新范式但仍可构建、仍有覆盖价值的旧范例。 | 后续重构候选 |
| `xui_xson_app_layout_lab`, `xui_xson_toast_lab` | 仍有独立覆盖价值的 XSON app layout 和 toast 范例。 | XSON 专项覆盖 |
