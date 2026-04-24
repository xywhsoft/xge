# V1 Gallery Migration

这份索引用来把 `XGE_V1/release/examples/freebasic` 中仍有参考价值的示例，映射到当前 XGE V2 的 `examples/` 和 `docs/case/`。

说明：

- V1 和 V2 不是一一翻译关系，V2 更倾向把大而散的功能拆成多份 lab。
- 一条 V1 记录可能对应多个 V2 范例或 case 文档。
- `最接近` 表示当前 V2 能覆盖主要学习目标，但不是逐像素复刻。
- `暂无专门范例` 表示当前没有单独的 V2 对应物，通常建议从最接近的 lab 或 bridge/custom 路线进入。

## 000-014 基础绘制

| V1 | 主题 | V2 对应 | 说明 |
| --- | --- | --- | --- |
| `000.Hello World` | 最小窗口 | `examples/mvp` / [最小窗口和 shape 绘制](minimal-window.md) | 直接对应最小启动路径。 |
| `001.Draw Shape` | 线框 shape | `examples/shape` / [Shape 绘制](shape-drawing.md) | 基础图元直接对应。 |
| `002.Fill Area` | 填充图元 | `examples/shape` / `examples/shape_full_gallery` | V2 把 fill/outline 一并放进 shape 系列。 |
| `003.Draw Text` | 基本文本绘制 | `examples/text` / [文本渲染和 XRF 缓存](text-rendering.md) | 基础文本路径。 |
| `004.Draw Text` | 字体/文本变体 | `examples/text` / `examples/font_text_lab` | 更适合看字体加载与 fallback。 |
| `005.Draw Text` | 多文本样式 | `examples/font_text_lab` | V2 把文本进阶能力集中到 lab。 |
| `006.Font Driver` | 字体驱动/字体文件 | `examples/font_text_lab` | 包含 TTF/TTC/XRF、多字体和 glyph 路径。 |
| `007.Draw Image` | 图片绘制 | `examples/texture` / [纹理和 Sprite 绘制](texture-sprite.md) | 基础图片显示。 |
| `008.Draw Image` | 图片裁切/变体 | `examples/texture` / `examples/image_texture_lab` | 裁切、采样、读回都在 V2 中拆开了。 |
| `009.Draw Image` | 图片绘制进阶 | `examples/image_texture_lab` | 更适合看加载、更新和 readback。 |
| `010.Memory Load Image` | 内存加载图片 | `examples/image_texture_lab` / `examples/resource_provider_lab` | V2 用 resource/memory/provider 路径统一表达。 |
| `011.GDI Draw Image` | 平台图像路径 | `examples/image_texture_lab` / [平台后端冒烟](platform-smoke.md) | V2 不再强调 GDI 专门路径，重点转到后端无关绘制。 |
| `012.ScreenShot` | 截屏/读回 | `examples/image_texture_lab` / `examples/render_target_lab` | 主要看 texture readback。 |
| `013.SetView` | viewport/view | `examples/viewport_clip_camera` | 直接对应 viewport、clip、camera。 |
| `014.SetCoodr` | 坐标系/坐标变换 | `examples/viewport_clip_camera` / `examples/perspective_quad` | V2 把 view、camera 和 world/screen 转换拆开了。 |

## 100-103 音频

| V1 | 主题 | V2 对应 | 说明 |
| --- | --- | --- | --- |
| `100.Play Sound` | 播放音频 | `examples/audio` / `examples/audio_lab` / [音频播放和 3D 声源](audio-playback.md) | 从最小播放到完整音频状态都能覆盖。 |
| `101.Play URL Sound` | URL/流式音频 | `examples/audio_lab` | V2 把 stream/fallback 收到 lab 中，没有单独 URL 小样例。 |
| `102.Play Memory Sound` | 内存音频 | `examples/audio_lab` | 覆盖 memory sound / sound load 路径。 |
| `103.Play SE` | 音效/SE | `examples/audio` / `examples/audio_lab` | 最小音效触发和完整音频控制都可对照。 |

## 200-281 输入、场景与运行时

| V1 | 主题 | V2 对应 | 说明 |
| --- | --- | --- | --- |
| `200.Scene` | 单场景模板 | `examples/scene` / `examples/scene_lifecycle_lab` | 一个偏最小，一个偏生命周期与策略验证。 |
| `201.Mouse Event` | 鼠标事件 | `examples/input` / `examples/input_state_lab` / [输入事件](input.md) | V2 把事件示例和状态示例拆开。 |
| `202.Mouse Event` | 鼠标事件变体 | `examples/input` / `examples/input_state_lab` | 最接近。 |
| `203.Mouse Event` | 双击/滚轮等 | `examples/input` / `examples/input_state_lab` | 同属输入路径。 |
| `204.Keyboard Event` | 键盘事件 | `examples/input` / `examples/input_state_lab` | 键盘状态与事件都已覆盖。 |
| `205.Earthquake` | 震屏/相机偏移 | `examples/viewport_clip_camera` | V2 当前没有专门 `earthquake` 范例，最接近的是 camera/view 变换。 |
| `206.Auto Pause` | auto pause | `examples/auto_pause_lab` / `examples/scene_lifecycle_lab` / `examples/platform_runtime_lab` | `auto_pause_lab` covers pause/resume/freeze directly; scene/runtime examples provide scene-stack and platform context. |
| `280.Scene Stack` | 场景栈 | `examples/scene` / [场景栈](scene-stack.md) / `examples/scene_lifecycle_lab` | 直接对应 push/pop 结构。 |
| `281.Scene Cut` | 场景切换/切场 | `examples/scene_lifecycle_lab` / `examples/game_login_lab` | `replace/cut` 行为在 lifecycle lab 和登录综合例里都能看到。 |

## 300-380 XUI

| V1 | 主题 | V2 对应 | 说明 |
| --- | --- | --- | --- |
| `300.XUI Layout` | 基础布局 | `examples/xui_layout_gallery` / `examples/xui_widget_tree_style_lab` | V2 用 gallery + tree/style 拆开布局基础。 |
| `301.XUI Layout` | 布局变体 | `examples/xui_layout_gallery` / `examples/xui_layout_validation` | 最接近。 |
| `302.XUI Layout` | 布局组合 | `examples/xui_layout_gallery` | 行、列、stack、grid 都集中在这里。 |
| `303.XUI Layout` | 复杂布局 | `examples/xui_layout_gallery` / `examples/xui_layout_validation` | V2 更强调可验证布局状态。 |
| `304.XUI ClassEvent` | 类事件/控件事件 | `examples/xui_focus_capture_event_lab` | 对应 focus、capture、event queue、hook。 |
| `305.XUI OnDraw` | 自定义绘制 | `examples/xui_paint_host_lab` / `examples/xui_bridge` | V2 走 host/custom paint 路线。 |
| `320.XUI Button Element` | Button | `examples/xui_basic_controls_lab` | 直接对应。 |
| `321.XUI Check Element` | CheckBox | `examples/xui_choice_controls_lab` | 直接对应。 |
| `322.XUI Radio Element` | Radio | `examples/xui_choice_controls_lab` | 直接对应。 |
| `323.XUI HyperLink Element` | HyperLink | `examples/xui_hyperlink_lab` | V2 ? `xgeXuiButton` primitive ??? hyperlink ????? visited/custom/transient/disabled? |
| `324.XUI Static Element` | Label/Static | `examples/xui_basic_controls_lab` | 对应 Label、Image 等静态控件。 |
| `325.XUI ScrollBar Element` | ScrollBar | `examples/xui_value_controls_lab` | 直接对应。 |
| `326.XUI ListBox Element` | ListBox | `examples/xui_list_scroll_lab` / [XUI 滚动列表](xui-scroll-list.md) | V2 用 ListView 表达列表选择。 |
| `327.XUI ScrollView Element` | ScrollView | `examples/xui_list_scroll_lab` / [XUI 滚动列表](xui-scroll-list.md) | 直接对应滚动容器。 |
| `328.XUI ProgressBar Element` | ProgressBar | `examples/xui_value_controls_lab` | 直接对应。 |
| `329.XUI LineEdit Element` | LineEdit/Password | `examples/xui_text_edit_lab` / `examples/xui_input_validation` / `examples/xui_text_edit_validation` | V2 把单行输入和多行编辑拆成多份验证例。 |
| `330.XUI Window Element` | Window/Dialog | `examples/xui_overlay_menu_lab` / [XUI 对话框](xui-dialog.md) | V2 用 Dialog/Panel/Overlay 路径表达。 |
| `380.GameLogin` | 登录综合例 | `examples/game_login_lab` | 这是目前最直接的 V2 对照项。 |

## 900-902 资源与底层容器

| V1 | 主题 | V2 对应 | 说明 |
| --- | --- | --- | --- |
| `900.xPack5` | pack 创建、增删、索引、读写 | `examples/resource_provider_lab` / `examples/async_assets_lab` | V2 不再提供同名 xPack 教学样例，资源 provider 和异步资源例最接近。 |
| `901.ResManage` | 多 pack 资源索引与查找 | `examples/resource_provider_lab` | 这是最接近的 V2 资源索引/统一加载入口。 |
| `902.xBsmm` | 结构体块内存管理 | [xBsmm Migration](xbsmm-migration.md) / `examples/resource_provider_lab` | V2 不再提供对等的公开 xBsmm API；迁移方式是把固定大小结构存储下沉到业务模块，资源型数据则转到 provider 路径。 |

## 迁移建议

1. 先按 [docs/case/README.md](README.md) 的主线顺序把 V2 基础能力跑通。
2. 如果你是从 V1 某个编号跳过来的，优先找本页中的 `直接对应` 或 `最接近`。
3. 若一条 V1 映射到了多个 V2 lab，建议先看更小的单功能 lab，再回到综合例：
   - `329.XUI LineEdit Element` -> 先 `xui_input_validation` / `xui_text_edit_lab`，再看 `game_login_lab`
   - `281.Scene Cut` -> 先 `scene_lifecycle_lab`，再看 `game_login_lab`
   - `900/901` -> 先 `resource_provider_lab`，再看 `async_assets_lab`

## 当前缺口

- 暂无明确缺口；剩余项以迁移说明或组合范例形式覆盖。
