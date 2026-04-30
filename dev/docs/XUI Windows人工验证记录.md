# XUI Windows 人工验证记录

本文用于记录 XUI 在 Windows 上的人工验证结果。每完成一项验证，应同步更新 `dev/docs/XUI轻量控件与布局SPEC.md`。

## 环境

- Windows：Microsoft Windows 10 专业版 10.0.19045 build 19045
- GPU：AMD Radeon RX 6600
- GCC：gcc.exe (GCC) 15.2.0
- XGE commit/分支：2725a43 / master

## 1. XUI Bridge

命令：

```bat
build\xge_xui_bridge.exe
```

结果：

- [x] 通过
- [ ] 失败
- [ ] 跳过

现象：

```text
初始版本只显示右侧空进度条，左侧 XUI 内容未正常绘制。修复 xui_bridge 示例后复测通过：左侧面板、标题、状态行、按钮和输入框显示正常；按钮可点击，点击后右侧进度条更新，进度条满后重置；输入框可正常输入、选择，Ctrl+A/C/X、删除等编辑快捷键正常。
```

日志：

```text
xui bridge font loaded; xui clicks/game-events 正常输出。复测截图确认通过。
```

## 2. XUI Incubation

命令：

```bat
build\xge_xui_incubation.exe
```

结果：

- [x] 通过
- [ ] 失败
- [ ] 跳过

检查项：

- [x] Button normal/hover/active 状态恢复正常。
- [x] Popup 打开、关闭、点击外部关闭正常。
- [x] IconButton 和 Tooltip 正常。
- [x] Tabs 切换正常。
- [x] Progress 文本和绿色进度条正常。
- [x] CheckBox、Radio、Switch 正常。
- [x] ComboBox 展开和选择正常。
- [x] Splitter 拖动正常。
- [x] ScrollBar/ScrollView 正常。

日志：

```text
xui incubation summary:
初测发现 popup/ComboBox 下拉默认可见、布局过紧且 ComboBox 右侧命中区与 splitter 重叠；修复 popup 默认可见、扩大示例布局后复测。随后发现 popup overlay 绘制顺序不正确，root 层文字/图标穿到 popup 之上；修复 root 与 overlay 之间的 flush 后复测通过。checkbox、radio、combobox、switch、tabs、popup 打开关闭均人工确认可正常交互。
```

## 3. XUI Layout Validation

命令：

```bat
build\xge_xui_layout_validation.exe
```

结果：

- [x] 通过
- [ ] 失败
- [ ] 跳过

检查项：

- [x] 初始布局无错位、重叠、闪烁。
- [x] 调整窗口大小时布局稳定。
- [x] grow、gap、space-between 显示合理。
- [x] content desired 与 resize tracking 文本随窗口变化。

日志：

```text
xui layout summary:
初始状态和多轮窗口缩放回放均通过。resize tracking 文本跟随窗口尺寸更新，布局块、grow、space-between 和 resize panel 显示稳定，无明显错位、重叠或闪烁。
```

## 4. XUI Input Validation

命令：

```bat
build\xge_xui_input_validation.exe
```

结果：

- [x] 通过
- [ ] 失败
- [ ] 跳过

检查项：

- [x] 英文输入正常。
- [x] 中文 IME 正常。
- [x] 长文本横向滚动正常。
- [x] Home/End、Ctrl+Left/Ctrl+Right 正常。
- [x] Ctrl+A/C/X/V 正常。
- [x] 双击选词正常。
- [x] readonly 可选择但不可修改。
- [x] disabled 不响应编辑。

日志：

```text
xui input summary:
初测窗口空白、UI 偶尔闪现；修复示例每帧稳定重绘后复测。随后发现长文本未按输入框 content rect 裁剪，修复 clipped widget 批处理 flush 与 Input 文本绘制裁剪后复测通过。英文/中文输入、选择、Ctrl+A/C/X/V、删除、Home/End、Ctrl+Left/Right、readonly 可选择不可修改、disabled 不响应编辑均人工确认通过。
```

## 5. XUI TextEdit Validation

命令：

```bat
build\xge_xui_text_edit_validation.exe
```

结果：

- [x] 通过
- [ ] 失败
- [ ] 跳过

检查项：

- [x] 基础输入、换行、删除能力正常。
- [x] 中文 IME 正常。
- [x] 光标移动对标记事本基础能力。
- [x] PageUp/PageDown、Home/End、Ctrl+Home/Ctrl+End 正常。
- [x] 鼠标点击定位、双击选词和拖拽选择正常。
- [x] Ctrl+A/C/X/V 正常。
- [x] Ctrl+Z/Ctrl+Y 正常。
- [x] Ctrl+W 自动换行切换正常。
- [x] 大段文本滚动和选择无明显卡顿、错位。
- [x] 右键/长按默认上下文菜单正常。
- [x] 菜单项与选区、readonly、剪贴板状态联动置灰正常。

日志：

```text
xui textedit summary:
初测发现光标和行高错位，拖选会出现无关选区高亮；修复行高计算和选区绘制后，拖拽选择、PageUp/PageDown、基础编辑操作复测通过。随后为 TextEdit 接入默认右键菜单，菜单包含 Select All/Cut/Copy/Paste/Delete，剪切、粘贴、删除进入 undo 栈，右键/长按弹出菜单不清空选区；修复菜单接入后双击选词缺失问题。最终人工确认多行输入框基础输入、换行、删除、中文 IME、点击定位、拖拽选择、双击选词、Ctrl+A/C/X/V、Ctrl+Z/Y、Ctrl+W、PageUp/PageDown、Home/End、Ctrl+Home/End、大段文本滚动和默认右键菜单均正常。
```

## 复跑记录

- 2026-04-23：执行 `build_examples_all.bat --xui`，Bridge、Incubation、Layout、Input、TextEdit 五个验证程序均构建通过。
- 2026-04-23：执行 `run_xui_validation_manual.bat`，Bridge、Incubation、Layout、Input、TextEdit 五个验证窗口均按顺序退出，退出码均为 0。
- 2026-04-30：执行 `build_examples_all.bat --xui`，新增控件与 XSON lab 均纳入 XUI 构建列表并构建通过；覆盖 `NumericInput`、`MessageBox`、`Toolbar`、`StatusBar`、`TreeView`、`TableView`、`PropertyGrid`、`Breadcrumb`、`Accordion`、`SearchBox`、`ColorPicker`、`Toast` 以及 `xui_xson_*` 示例。
- 2026-04-30：执行新增 XUI/XSON lab 子集 smoke：`build\xge_xui_numeric_input_lab.exe`、`build\xge_xui_message_box_lab.exe`、`build\xge_xui_toolbar_lab.exe`、`build\xge_xui_status_bar_lab.exe`、`build\xge_xui_tree_view_lab.exe`、`build\xge_xui_table_view_lab.exe`、`build\xge_xui_property_grid_lab.exe`、`build\xge_xui_breadcrumb_lab.exe`、`build\xge_xui_accordion_lab.exe`、`build\xge_xui_search_box_lab.exe`、`build\xge_xui_color_picker_lab.exe`、`build\xge_xui_toast_lab.exe`、`build\xge_xui_xson_page_lab.exe`、`build\xge_xui_xson_style_lab.exe`、`build\xge_xui_xson_layout_gallery_lab.exe`、`build\xge_xui_xson_app_layout_lab.exe`、`build\xge_xui_xson_virtual_list_lab.exe`、`build\xge_xui_xson_breadcrumb_lab.exe`、`build\xge_xui_xson_accordion_lab.exe`、`build\xge_xui_xson_toast_lab.exe`、`build\xge_xui_xson_property_grid_lab.exe`、`build\xge_xui_xson_table_view_lab.exe` 均以 `--frames 2` 通过。
- 2026-04-30：复跑 `build\xge_xui_toolbar_lab.exe --frames 2`，输出 `xui-toolbar-lab final-summary frames=2 init=1 toggle=1 disabled=1 keyboard=1 vertical=1 group=1 tooltip=1 overflow=1 last=1 cb=2 of=1`，Toolbar group、tooltip metadata 与 overflow callback smoke 通过。
- 2026-04-30：新增并执行 `build\xge_xui_standard_controls_lab.exe --frames 2`，输出 `xui-standard-controls-lab final-summary frames=2 basic=1 choice=1 value=1 toolbar=1 status=1`，标准控件总览 smoke 通过。
- 2026-04-30：复跑 `build\xge_xui_status_bar_lab.exe --frames 2`，输出 `xui-status-bar-lab final-summary frames=2 init=1 layout=1 flex=1 progress=1 click=1 disabled=1 last=0 cb=1`，StatusBar flexible spacer smoke 通过。
- 2026-04-30：新增并执行 `build\xge_xui_data_controls_lab.exe --frames 2`，输出 `xui-data-controls-lab final-summary frames=2 tree=1 table=1 grid=1 adapter=1 selection=1 cells=66`，数据控件总览 smoke 通过。
- 2026-04-30：新增并执行 `build\xge_xui_xson_controls_lab.exe`，输出 `xui-xson-controls-lab final-summary load=1 basic=1 choice=1 range=1 bars=1 data=1 experience=1 index=19`，XSON 控件聚合 smoke 通过。
- 2026-04-30：新增并执行 `build\xge_xui_overlay_policy_lab.exe`，输出 `xui-overlay-policy-lab final-summary create=1 popup=1 dialog=1 menu=1 tooltip=1 top_escape=1 popup_close=2 dialog_close=1 legacy=2`，Overlay Policy 聚合 smoke 通过。
- 2026-04-30：增强并执行 `build\xge_xui_overlay_policy_lab.exe`，输出 `xui-overlay-policy-lab final-summary create=1 popup=1 dialog=1 menu=1 tooltip=1 top_escape=1 policy=1 popup_close=3 dialog_close=2 legacy=2`，新增覆盖 Popup placement/anchor/offset、screen clamp、z base/top overlay、modal outside consume、owner disabled 自动关闭，Dialog outside close/focus restore/z base，以及 Menu/ComboBox/Tooltip 复用 Popup policy 的路径。
- 2026-04-30：新增并执行 `build\xge_xui_choice_standard_lab.exe`，输出 `xui-choice-standard-lab final-summary create=1 checkbox=1 radio=1 switch=1 toggle=1 focus=1 callbacks=2/4/4/2/2`，Choice 标准交互 smoke 通过；RadioGroup 方向键导航验证通过；Toggle 对齐 CheckBox checked/change 命名、键盘切换、disabled 策略和 bitmap check mark 绘制。
- 2026-04-30：新增并执行 `build\xge_xui_button_standard_lab.exe`，输出 `xui-button-standard-lab final-summary create=1 button=1 icon=1 focus=1 options=1 clicks=5/3 callbacks=5/3 checked=1 loading=0 semantic=2 iconText=1`，Button 标准交互 smoke 通过；覆盖 checked、loading、primary/danger semantic 和 icon + text 左右布局。
- 2026-04-30：新增并执行 `build\xge_xui_input_standard_lab.exe`，输出 `xui-input-standard-lab final-summary create=1 standard=1 change=9/9 submit=1/1 filter=2/1 clear=2 icons=1/3 max=0 error=0 text=abcd lastChange=abcd lastSubmit=Z`，Input 标准交互 smoke 通过；覆盖 Change、Submit、Filter、MaxLength、error 状态与错误文本、clear button、prefix/suffix icon 预留、readonly 可选择但不修改、password 复制/IME 策略、IME candidate rect 和 Backspace Change 路径。
- 2026-04-30：复跑 `build\xge_xui_input_password_policy_lab.exe --frames 2`、`build\xge_xui_input_clipboard_policy_lab.exe --frames 2`、`build\xge_xui_input_ime_policy_lab.exe --frames 2`，均通过；输出分别确认 password mask/copy/cut/paste/readonly/IME/candidate、clipboard copy/cut/paste/readonly、IME start/update/move/end/candidate 策略。运行期间仅出现 libpng iCCP warning。
- 2026-04-30：新增并执行 `build\xge_xui_text_edit_standard_lab.exe`，输出 `xui-text-edit-standard-lab final-summary create=1 standard=1 readonlyOK=1 undoOK=1 tabOK=1 reserveOK=1 readonly=0 undo=1 redo=1 highlights=2 lineNumbers=1 size=11`，TextEdit 标准交互 smoke 通过；覆盖 readonly selection、undo/redo 单步策略、redo clear、Tab 输入为字面量 `\t`、find highlight 预留和 line number 预留。
- 2026-04-30：更新 `docs/api/xui.md` 的“控件标准契约”章节，明确基础状态、扩展状态保留策略、事件命名、键盘焦点、readonly/disabled 差异、setter dirty 标记和 Init/Unit 生命周期所有权规则；Stage A 文档项完成。
- 2026-04-30：更新 `docs/api/xui.md` 的“主题与内置 Tech Blue 风格”章节，明确 Tech Blue token 映射、尺寸/字体 token、状态色、focus ring、error input、overlay 层级色块和无 texture host 时的几何/bitmap mask fallback；增强并执行 `build\xge_xui_theme_lab.exe --frames 2`，输出 `xui-theme-lab final-summary frames=2 default=1 setget=1 dip=1 style=1 lightdark=1 button=1 states=1`，主题默认值、主题切换和状态色 swatch smoke 通过。
- 2026-04-30：新增并执行 `build\xge_xui_list_standard_lab.exe`，输出 `xui-list-standard-lab final-summary create=1 keyboard=1 disabled=1 focus=1 selection=1 renderer=1 selected=3 scroll=60.00 callbacks=11 last=3 renderCalls=5`，ListView 标准交互 smoke 通过；覆盖 disabled item、keyboard navigation、ensure visible、single/multi/range selection mode 和 item renderer 预留。
- 2026-04-30：新增并执行 `build\xge_xui_scroll_standard_lab.exe`，输出 `xui-scroll-standard-lab final-summary create=1 clamp=1 wheel=1 drag=1 bars=1 ensure=1 policy=1 focus=1 offset=20.00/20.00`，ScrollView 标准交互 smoke 通过；覆盖 auto/hide scrollbar policy、ensure rect/child visible、nested scroll edge pass-through 和 horizontal wheel。
- 2026-04-30：新增并执行 `build\xge_xui_tabs_standard_lab.exe`，输出 `xui-tabs-standard-lab final-summary create=1 disabled=1 keyboard=1 close=1 scroll=1 meta=1 selected=3 scrollX=210.00 callbacks=3/1 last=3/0`，Tabs 标准交互 smoke 通过；覆盖 disabled tab、close button、dirty mark、scrollable tab strip 和 icon 预留。
- 2026-04-30：新增并执行 `build\xge_xui_virtual_list_standard_lab.exe`，输出 `xui-virtual-list-standard-lab final-summary create=1 ensure=1 reuse=1 selection=1 variable=1 clamp=1 focus=1 first=2 visible=6 selected=6 scroll=1728.00 create=6 bind=30 select=3 last=6`，VirtualList 标准交互 smoke 通过；覆盖 ensure index visible、slot reuse 生命周期、selected persistent 和 variable item height API。
- 2026-04-30：执行 `run_examples_smoke.bat 2`，脚本已构建新增 XUI 示例；全量运行在进入新增 XUI smoke 前被既有 `scene_lifecycle_lab` 阻断，输出 `scene-lifecycle-lab final-summary ok=0`，因此本次不把全量 smoke 标为通过。
- 2026-04-30：复跑 XUI 聚合 lab：`build\xge_xui_standard_controls_lab.exe --frames 2` 输出 `basic=1 choice=1 value=1 toolbar=1 status=1`；`build\xge_xui_data_controls_lab.exe --frames 2` 输出 `tree=1 table=1 grid=1 adapter=1 selection=1`；`build\xge_xui_xson_controls_lab.exe` 输出 `load=1 basic=1 choice=1 range=1 bars=1 data=1 experience=1`；`build\xge_xui_overlay_policy_lab.exe` 输出 `create=1 popup=1 dialog=1 menu=1 tooltip=1 top_escape=1 policy=1`。据此确认旧控件第一轮、新增控件、数据控件、XSON 控件扩展、Overlay Policy 和示例文档聚合验证通过。
- 2026-04-30：复跑 Overlay/输入细分 policy lab：`xge_xui_input_policy_lab.exe`、`xge_xui_input_ime_policy_lab.exe`、`xge_xui_input_nav_policy_lab.exe`、`xge_xui_input_context_policy_lab.exe`、`xge_xui_input_replace_policy_lab.exe`、`xge_xui_input_delete_policy_lab.exe`、`xge_xui_input_clipboard_policy_lab.exe`、`xge_xui_input_password_policy_lab.exe` 均以 `--frames 2` 通过；`build\xge_xui_menu_policy_lab.exe --frames 2` 输出 `xui-menu-policy-lab final-summary frames=2 init=1 config=1 reset=1 clamp=1 reopen=1 focus=1 enabled=1 open=1`，同步验证 Menu popup owner/focus restore/cursor placement/z base 的新 Overlay Policy 断言；Combo/Popup/Tooltip/Dialog policy lab 也以 `--frames 2` 通过。
- 2026-04-30：复跑第一批新增控件与数据/体验控件 smoke：`xge_xui_numeric_input_lab.exe` 输出 `init=1 keyboard=1 spinner=1 error=1 blur=1`；`xge_xui_message_box_lab.exe` 输出 `init=1 click=1 escape=1 enter=1 once=1`；`xge_xui_toolbar_lab.exe` 输出 `init=1 toggle=1 disabled=1 keyboard=1 vertical=1 group=1 tooltip=1 overflow=1`；`xge_xui_status_bar_lab.exe` 输出 `init=1 layout=1 flex=1 progress=1 click=1 disabled=1`；`xge_xui_tree_view_lab.exe` 输出 `init=1 expand=1 select=1 keyboard=1 collapse=1`；`xge_xui_table_view_lab.exe` 输出 `init=1 virtual=1 select=1 sort=1 resize=1`；`xge_xui_property_grid_lab.exe` 输出 `init=1 flags=1 select=1 collapse=1 editors=1`；`xge_xui_breadcrumb_lab.exe` 输出 `init=1 overflow=1 select=1`；`xge_xui_accordion_lab.exe` 输出 `init=1 multi=1 single=1`；`xge_xui_search_box_lab.exe` 输出 `init=1 submit=1 clear=1`；`xge_xui_color_picker_lab.exe` 输出 `init=1 hex=1 palette=1`；修正并复跑 `xge_xui_toast_lab.exe --frames 2`，输出 `init=1 close=1 expire=1`，退出码绑定 init/close/expire 检查。
- 2026-04-30：复跑 XSON 细分 lab：`xge_xui_xson_page_lab.exe`、`xge_xui_xson_style_lab.exe`、`xge_xui_xson_layout_gallery_lab.exe`、`xge_xui_xson_app_layout_lab.exe`、`xge_xui_xson_virtual_list_lab.exe` 均输出 `ok`；`xge_xui_xson_breadcrumb_lab.exe --frames 2` 输出 `load=1 segments=1 selected=1`；`xge_xui_xson_accordion_lab.exe --frames 2` 输出 `load=1 sections=1 mode=1 style=1`；`xge_xui_xson_toast_lab.exe --frames 2` 输出 `load=1 items=1 placement=1 style=1`；`xge_xui_xson_property_grid_lab.exe --frames 2` 输出 `load=1 items=1 flags=1 style=1`；`xge_xui_xson_table_view_lab.exe --frames 2` 输出 `load=1 columns=1 rows=1 style=1`；`xge_xui_xson_controls_lab.exe --frames 2` 输出 `load=1 basic=1 choice=1 range=1 bars=1 data=1 experience=1`。
- 2026-04-30：复跑旧基础 XUI lab 并修正旧断言/退出码绑定：`xge_xui_basic_controls_lab.exe --frames 2` 输出 `label=1 image=1 button=1 icon=1 panel=1 separator=1 state=1`；`xge_xui_choice_controls_lab.exe --frames 2` 输出 `toggle=1 checkbox=1 group=1 radio=1 switch=1 state=1`；`xge_xui_value_controls_lab.exe --frames 2` 输出 `slider=1 progress=1 splitter=1 scrollbar=1 state=1`；同时 `xge_xui_list_scroll_lab.exe`、`xge_xui_button_state_lab.exe`、`xge_xui_combo_detail_lab.exe`、`xge_xui_list_view_detail_lab.exe`、`xge_xui_theme_lab.exe` 均以 `--frames 2` 通过。`basic/choice/value` 现在在 summary 检查失败时返回非 0，避免旧 smoke 漏报。
- 2026-04-30：新增并执行 `run_examples_smoke.bat --xui 2`，脚本会先执行 `build_examples_all.bat --xui`，再直接进入 XUI smoke 段，避开既有非 XUI `scene_lifecycle_lab ok=0` 阻断；本轮完整通过，末尾输出 `Example smoke checks finished`。过程中修正 `xui_tabs_dirty_rect_lab` 的点击/键盘事件验证点、`xui_input_blink_focus_lab` 的焦点切换 blink 断言，并让 `xui_tabs_dirty_rect_lab\build.bat` 每次强制重建 `xgedbg.dll/xgedbg.lib`，避免 debug lab 链接陈旧库。
- 2026-04-30：补齐 `xui_xson_*_lab` 细分 case 文档，并更新 `docs/case/README.md` 主线与 XUI 分组索引；脚本比对确认所有 `examples/xui_*_lab` 均存在对应 `docs/case/xui-*-lab.md`，README 主线编号连续到 106。`git diff --check` 对新增 XSON case 文档和 README 无实际格式错误，仅有 CRLF 转换提示。
- 2026-04-30：修正 `xui_xson_accordion_lab`、`xui_xson_breadcrumb_lab`、`xui_xson_property_grid_lab`、`xui_xson_table_view_lab`、`xui_xson_toast_lab` 的退出码绑定，确保 `final-summary` 中的 `load/sections/items/style` 等检查失败时返回非 0；重建并以 `--frames 2` 复跑 5 个 lab，输出检查位均为 `1`，退出码均为 0。
- 2026-04-30：继续收紧新增控件 lab 的退出码绑定，`xui_accordion_lab`、`xui_breadcrumb_lab`、`xui_color_picker_lab`、`xui_data_controls_lab`、`xui_message_box_lab`、`xui_numeric_input_lab`、`xui_property_grid_lab`、`xui_search_box_lab`、`xui_standard_controls_lab`、`xui_status_bar_lab`、`xui_table_view_lab`、`xui_toolbar_lab`、`xui_tree_view_lab` 现在都会在 summary 检查位失败时返回非 0；静态扫描确认 XUI lab 中已无 `final-summary` 但退出码只看 `xgeRun` 的模式。复跑 `run_examples_smoke.bat --xui 2` 完整通过，末尾输出 `Example smoke checks finished`。同时关闭 `xui_value_controls_lab` 默认调试日志，仅在 `XGE_XUI_VALUE_CONTROLS_DEBUG` 显式设置非 `0` 时输出；单独复跑 value lab 通过。
- 2026-04-30：补齐 `run_examples_smoke.bat` 的 XUI 入口覆盖：full smoke 构建段显式加入输入 policy、owner/policy 细分、split/window/text edit 等 XUI lab；XUI smoke 运行段加入 `xui_split_layout_lab`。脚本比对确认所有 `examples/xui_*_lab` 均被 `build_examples_all.bat --xui`、`run_examples_smoke.bat` 构建段和 `run_examples_smoke.bat` 运行段覆盖。复跑 `run_examples_smoke.bat --xui 2` 完整通过，包含 `xui-split-layout-lab final-summary frames=2 init=1 vertical=1 horizontal=1 shadow=1`，末尾输出 `Example smoke checks finished`。
- 2026-04-30：继续收紧窗口式 lab 退出码绑定，`xui_split_layout_lab`、`xui_window_lab`、`xui_text_edit_lab`、`xui_focus_capture_event_lab`、`xui_list_scroll_lab`、`xui_overlay_menu_lab`、`xui_paint_host_lab`、`xui_theme_lab` 现在都会在对应 summary 检查位失败时返回非 0。单独重建并以 `--frames 2` 复跑 8 个 lab 均通过；静态扫描确认 XUI `final-summary` lab 中已无运行后直接 `return 0` 的模式。复跑 `run_examples_smoke.bat --xui 2` 完整通过，末尾输出 `Example smoke checks finished`。
- 2026-04-30：执行 XUI 文档和源码入口覆盖检查：`docs/case/xui-*.md` 未发现真实乱码或 TODO/FIXME 残留；case 文档中的 `examples\...\build.bat` 和 `build\*.exe` 命令均指向真实路径；`src/xge_impl.c` 已 include 当前全部 `src/xge_xui_*.c` 源文件。
- 2026-04-30：执行新增控件 API 暴露面检查：比对 `src/xge_xui_numeric_input.c`、`message_box`、`toolbar`、`status_bar`、`tree_view`、`table_view`、`property_grid`、`breadcrumb`、`accordion`、`search_box`、`color_picker`、`toast` 中的公开 `xgeXui*` 函数，确认均已在 `xge.h` 声明；相关 typedef、callback、item/column/segment/section 结构也已在头文件公开。同步在 `docs/api/xui.md` 增加“新增控件 API 速查”表。
- 2026-04-30：重建 `build\xge.dll/build\xge.lib` 并执行 `build_test.bat`。修正 Tabs 在布局尺寸变化后未重新 clamp `fScrollX` 的问题；修正 Menu/Popup 的显式点 anchor 与无效窗口尺寸下的 clamp 策略；RadioGroup 单控件 change 仅通知新选中项；Popup 单测断言更新为验证默认 owner bottom-left placement。复跑 `build_test.bat` 通过，直接执行 `build\xge_test.exe` 退出码为 0。
- 2026-04-30：修正 XUI 脏绘制与每帧清屏的冲突：Sokol 帧清屏和显式 `xgeClear()` 会推进 surface dirty generation，`xgeXuiPaint()` 在 surface 已更新但控件自身不脏时仍重绘一次，避免 `*.exe 0` 无限运行时界面被清屏后空白、只在交互时闪现。复跑 `build_dll.bat`、`build_test.bat`、`run_examples_smoke.bat --xui 2` 均通过，末尾输出 `Example smoke checks finished`。

## 关闭条件

- 全部可执行验证项通过，或失败项已修复。
- 如果某项跳过，必须写明原因。
- 通过后可勾选 SPEC 中对应人工验证项和 `Windows 人工验证通过`。
