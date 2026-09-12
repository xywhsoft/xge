# 任务进度状态（可变文件 · 每批必须更新）

> 本文件记录"现在做到哪了"。与 SPEC.md（稳定事实）配套使用。
> 每完成一个批次：勾选对应项、追加批次日志、更新覆盖率数字。
> 任何会话开始时先读本文件确定续跑位置。

- 最后更新：2026-09-10（**任务收官：M0-M4 全部完成**）

## 1. 阶段状态

| 阶段 | 状态 | 备注 |
|---|---|---|
| M0 规范+工具+基线 | ✅ 完成（2026-09-10） | 4 工具就绪；基线已生成 |
| M1 xge.h 全量 747 条声明 | ✅ 完成（2026-09-10） | 747/747 = 100%；含 desc 字段与枚举值；抽 20 条核对通过 |
| M2 xui.h 框架层 | ✅ 完成（2026-09-10） | 框架 717/717 = 100%（余 148 条为 M3 组合族：Dock 75/Msg 37/File 36 含 Open/Save/Select 三个便捷入口）；抽 10 条核对通过 |
| M3 控件层（2,493 条 + 组合族 148） | ✅ 完成（2026-09-10） | 2,641/2,641 = 100%（含 input 控件簇内 23 条框架泵于 M2 完成）；58 控件全勾选 |
| M4 类型/枚举/宏 + 棘轮 + CI | ✅ 完成（2026-09-10） | 扫尾 222 处（枚举 22 值逐值 + text_id 类型级/11 组 + 90 desc 结构体级 + 7 核心 desc 93 字段 + 3 宏组）；CI api-docs job 已挂载；终验抽 50 条全通过；范围决策见 FINDINGS.md B 节 |

## 2. 覆盖率基线（棘轮依据 = api-docs/coverage.json）

| 文件 | 函数注释覆盖 | desc 字段 | 枚举值 | 测量日期 |
|---|---|---|---|---|
| xge.h | **747 / 747**（100%） | ✅ 7 个 desc_t 全部逐字段（xge_desc/miniprogram/egl/font_face/font_instance/text_shape/particle_world） | ✅ xge_result_t 16 值全注释 | 2026-09-10 |
| xui.h | **3,358 / 3,358 = 100%**（控件族 2,493 + 框架族 865 全完成） | ✅ 90/90 结构体级 + 7 核心 desc 逐字段（93 字段，FINDINGS B-2） | ✅ result 15 值 + stage 7 值逐值；text_id 类型级 + 11 组（FINDINGS B-1） | 2026-09-10 |

基线 revision：81f1054（工作区含注释改动，未提交）。棘轮基线已随 M1 刷新至 747/15。

## 3. 模块清单（canonical 来源 = coverage.json 的 files.*.modules；下文为 M0 实测整理）

### 3.1 xge.h 模块与 M1 批次建议（747 条，簇名为首词聚类，数量以 coverage.json 为准）

| 批 | 范围（簇 → 条数） | 对应实现 |
|---|---|---|
| 1 | core/窗口长尾（Init/Run/Unit/Quit/Get5/Set4/Log4/Platform6/Graphics6/Timer/Time/Sleep/Begin/End/Flush/Clear/Present/Invalidate/Frame2/Debug2/dbg3 及全部单条簇） | xge_core.c、xge_impl.c |
| 2 | ShapeEx 205 | xge_shape_ex*.c |
| 3 | Svg 66 | xge_svg.c |
| 4 | Shape 58 + Nine(Patch) 5 + Draw 4 + Blend 2 + Depth 2 + Clip 3 | xge_shape.c、xge_render.c |
| 5 | Font 37 + Glyph 8 + Text 6 | xge_font.c、xge_text_run.c |
| 6 | Texture 19 + Image 9 + Render 14 + Pass 3 + Viewport 3 + Frame 2 + Offscreen 3 | xge_texture.c、xge_render_target.c |
| 7 | Particle 39 | xge_particle*.c |
| 8 | Audio 12 + Sound 16 + Music 11 + Stream 12 | xge_audio.c |
| 9 | Ime 14 + Input 6 + Key 4 + Mouse 4 + Touch 3 + Gamepad 8 | xge_input.c、xge_ime_win32_tsf.c |
| 10 | Data 9（xgeDataObject*）+ Drag 6 + Clipboard 4 | xge_drag_drop.c、xge_render.c |
| 11 | Resource 6 + Async 10 + Emoji 9 + Scene 9 + Sprite 7 + Zstd 1 | xge_resource.c、xge_async.c、xge_emoji.c、xge_sprite.c |
| 12 | Mesh 4 + Material 9 + Shader 11 + Buffer 4 | xge_mesh.c、xge_material.c、xge_buffer.c |
| 13 | EGL(E 簇) 4 + Mini(MiniProgram) 11 | xge_egl.c、xge_miniprogram.c |

### 3.2 xui.h 框架族清单（865 条；M2 范围）

大族（≥10 条）：Code 191（xuiCode* 代码文档/词法框架族，与 codeedit 控件互补）、Widget 174、Rich 58（xuiRichDocument* 富文本文档族，与 RichEdit 控件互补）、Icon 53、Get 30、Set 16、Style 21、Resource 15、Painter 15、Scroll 15、Text 12、Path 10。

小族（<10 条）：Data 9、Input 9（框架输入泵 xuiInputPointer*/Key*/Text* 等——注意与"input"输入框控件簇 69 条区分，控件部分归 M3）、Drag 6、Find 6、Builtin 6、Overlay 6、Debug 5、Hot 4、Proxy 4、Clear 3，及 core 单条簇（Create2/Destroy/Poll/Dispatch2/Render2/Translate/Update/Layout2/Invalidate2/Has2/Vector2/Release2/Theme/Register/Build/Report/Rect/Interaction/Purge/Hit/Focus/Command/Open/Save/Select/Query 等）。

### 3.3 控件清单（58 个注册类型，2026-09-10 实测 `grep -h "tDesc.sName" src/xui_*.c`）

M3 按此清单逐控件勾选（状态：⬜ 未做 / 🔧 进行中 / ✅ 完成）。各控件 API 条数见 coverage.json `files["xui.h"].modules`（如 codeedit 102、timelineview 85、datepicker 73…）。

- [x] RichEdit　- [x] accordion　- [x] breadcrumb　- [x] button　- [x] canvas
- [x] carousel　- [x] cascader　- [x] chart　- [x] checkbox　- [x] checkcard
- [x] codeedit　- [x] colorpicker　- [x] combobox　- [x] datepicker　- [x] flowgraph
- [x] hyperlink　- [x] iconpicker　- [x] image　- [x] input　- [x] inventorygrid
- [x] label　- [x] listview　- [x] menu　- [x] menubar　- [x] messagelist
- [x] msgtip　- [x] numeric_input　- [x] page　- [x] panel　- [x] popup
- [x] progress　- [x] propertygrid　- [x] qrcode　- [x] radio　- [x] radiogroup
- [x] rangeslider　- [x] scrollbar　- [x] scrollframe　- [x] scrollview　- [x] separator
- [x] slider　- [x] splitlayout　- [x] statusbar　- [x] stepbar　- [x] tablegrid
- [x] tableview　- [x] tabs　- [x] taginput　- [x] terminal　- [x] textedit
- [x] timelineview　- [x] toast-item　- [x] toggle　- [x] toolbar　- [x] treeview
- [x] virtual_joystick　- [x] window　- [x] workflow

**M0 前缀核对结论**（2026-09-10）：

- 控件 API 前缀与类型名 PascalCase 对应；例外：`toast-item → xuiToast*`、`virtual_joystick → xuiVirtualJoystick*`、`numeric_input → xuiNumericInput*`（下划线名在 API 中为驼峰）。
- **dockpanel 与 msgbox 不注册控件类型**（grep tDesc.sName 无此二者）：是组合式 API 家族——`xuiDock*` 75 条（xui_dock_panel.c，7037 行）、`xuiMsg*` 37 条（xui_msgbox.c）。同类的还有 `xuiFileDialog*` 36 条（File 簇）。**这三个组合族随 M3 一并处理**（不进 3.3 勾选清单，批次日志单独记录）。

## 4. 批次日志（每批一行：日期 | 阶段 | 范围 | 结果）

- 2026-09-10 | M0 | 工具链 + 基线 | ✅ 4 工具就绪（coverage/packet/lint/verify_batch）；基线 xge.h 25/747、xui.h 15/3,358（revision 81f1054）；lint --report 通过、verify_batch 通过；修复工具三类缺陷（名称提取误取返回类型、控件匹配 k 循环 break 错用 continue、bat 中文 GBK 乱码改 ASCII+CRLF）；冒烟草稿包 packets/xge_Svg.md（66 条）
- 2026-09-10 | M1 | xge.h 全部 13+1 批（Svg 66 / Shape 族 76 / Font+Glyph+Text 51 / 纹理渲染 54 / 粒子 39 / 音频 51 / 输入 39 / 数据拖放剪贴板 19 / 资源异步 emoji 场景精灵 60 / 网格材质着色缓冲 28 / EGL+小程序+平台图形 55 / core 长尾 47 / ShapeEx 205）+ 批N（7 个 desc 结构体字段 + xge_result_t 枚举 16 值） | ✅ **747/747 = 100%**；verify_batch 全程通过；git diff +780/-0（零既有行改动）；抽 20 条证据核对通过（无注释性谎言；xgeImeCandidateSelect 平台注记列为后续增强项）；过程中修复 apply_comments 混合行尾合并行 bug 与一处多行注释提前闭合笔误；24 条既有英文注释保留（其中 xgeParticleEmit 等 7+17 条分布在各批 skipped）

- 2026-09-10 | M2 | xui.h 框架层 9 批（Widget 174 / Code 191 / Rich+Text 70 / Icon+Builtin+Resource 74 / Edit+Get+Set 68 / Style+Painter+Path+Vector+Theme 49 / ScrollModel+Data+Find 30 / Drag+Overlay+Hot+Proxy+Clear+Debug 34 / 输入泵 23 + core 单条族 30 + 对话框入口 3） | ✅ **框架 717/717 = 100%**（组合族 Dock 75/Msg 37/File 36 归 M3）；verify_batch 全程通过；xui.h +731/-0；抽 10 条核对通过；关键契约已落注释：控件树所有权（无 widget 级 AddRef、Destroy 延迟销毁）、事件三阶段与 DISPATCH_STOP、xuiSetProxy 绑定时序、Serialize/Find 族输出所有权（FreeSerialized/FindFreeText）、xuiSetTheme 同帧生效、xuiRenderPrepare 三阶段；发现并注记聚类事实：xuiInputPointer* 等 23 条框架输入泵因首词撞 input 控件名而计入控件簇（M3 补 input 控件时只需处理其余 55 条）

- 2026-09-10 | M3 | 控件层 13 批（RichEdit/accordion/breadcrumb/button/canvas 212 → carousel/cascader/chart/checkbox/checkcard 201 → codeedit 100 → colorpicker+combobox 111 → datepicker+flowgraph 144 → hyperlink/iconpicker/image/inventorygrid 164 → label/listview/menu/menubar 124 → messagelist/msgtip/numeric_input/page 144 → panel/popup/progress/qrcode/radio 180 → radiogroup/rangeslider/scrollbar/scrollframe/scrollview 162 → separator/slider/splitlayout/statusbar/stepbar 131 → tablegrid/tableview/tabs 141 → taginput/terminal 104 → textedit/timelineview 146 → toast/toggle/toolbar/treeview 160 → virtual_joystick/window/workflow 130 → propertygrid+input 113 → Dock+MsgBox+File 组合族 148） | ✅ **xui.h 3,358/3,358 = 100%**；verify_batch 全程通过；git diff xge.h +780/-0、xui.h +3,345/-0（零既有行改动）；58 控件全勾选；所有权类契约（bOwnDocument/bOwnGraph/bOwnWorkflow/FreeSerialized/StateFree）与测试计数（GetChangeCount 族）均有注记

- 2026-09-10 | M4 | 收官扫尾 + CI + 终验 | ✅ 222 处扫尾插入（verify 通过、覆盖率保持 100%、git xui.h +3,568/-0）；CI 挂载 .github/workflows/uidesign.yml 追加 api-docs job（纯 Python：coverage + lint 棘轮，选择 CI 而非本地脚本的理由=强制执行优于人工纪律，此为项目此前短板），触发路径补 api-docs/**；CI 两步本地模拟通过；终验随机抽 50 条（固定种子，xge/xui 混合）50/50 零矛盾；FINDINGS.md 建立（无代码 bug；2 项范围决策；增强项）；任务全部完成，最终统计见下

**任务总账（2026-09-10 收官）**：函数注释 xge.h 747/747 + xui.h 3,358/3,358 = 4,105/4,105（100%）；类型层：xge 7 desc 逐字段 + 枚举 16 值，xui 90 desc 结构体级 + 7 核心 93 字段 + 枚举 22 值逐值 + text_id 类型/分组；宏组注释 3 处 + 既有键码区说明保留；门禁体系：4 工具 + CI job；git 合计 xge.h +780/-0、xui.h +3,568/-0，零既有行改动，未提交。

## 5. 待办与中断点

- **任务完成，无待办。** 后续衔接：①官网/API 参考集成（注释现为唯一语义源，可用 xrt 官网管线提取，见任务前讨论的方案）；②建议提交时将 api-docs/（含 coverage.json，不含 packets/）与两头注释一并入库。
- 后续增强项（不阻塞）：xgeImeCandidateSelect 等候选窗 API 可补平台注记（当前仅 ImeSetEnabled/Mode 带注记）。
- 遗留小项（不阻塞）：packets/ 与 coverage.json 是否入库由用户决定（coverage.json 必须入库，棘轮依赖它；packets/ 可 gitignore）。
