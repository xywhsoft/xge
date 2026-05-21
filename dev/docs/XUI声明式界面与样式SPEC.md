# XUI 声明式界面与样式 SPEC

本文档跟踪 XUI 声明式界面、XSON 加载、XValue 样式继承和 APP/游戏 UI 布局能力的开发进度。

关联设计文档：

- `dev/docs/XUI Widget V2基础设计.md`
- `dev/docs/XUI Widget V2基础SPEC.md`
- `dev/docs/XUI声明式界面与样式设计.md`
- `dev/docs/XGE_XGEDBG构建分离设计.md`

> 2026-05-07 口径更新：本文历史 `[x]` 只代表第一版 XSON / loader 实现；`type` 到 Widget role、children 规则、overflow、z、scroll、IME 等基础行为必须按 Widget V2 重新同步和验收。

> 2026-05-09 口径更新：Widget 阶段 E2 已完成，XSON 事件绑定扩展必须映射到 Widget 基础语义事件。当前 `onClick`、Mouse、Key、TextInput、HotKey、Command 等已实现项继续有效；Drag 等声明式字段进入后续任务，不再以 E2 作为阻塞理由。

## 进度维护规则

每次开始相关开发前，必须先更新本文档进度：

1. 将即将开发的任务从 `[ ]` 改为 `[~]`。
2. 如果任务范围变化，先调整任务描述或新增子任务。
3. 如果发现阻塞，追加到“阻塞与决策记录”。
4. 开发完成、构建通过、示例或测试通过后，才能改为 `[x]`。
5. 如果任务被放弃或替代，标记为 `[!]` 并写明原因。

状态含义：

- `[ ]` 未开始。
- `[~]` 进行中。
- `[x]` 已完成，并通过必要验证。
- `[!]` 放弃、替代或暂缓，必须说明原因。

完成口径：

- 设计任务：文档更新完成，并明确边界。
- API 任务：公共声明、实现和最小调用示例齐备。
- 加载任务：错误路径、资源释放和失败回滚覆盖。
- 布局任务：自动验证或 layout snapshot 覆盖主要行为。
- 控件任务：至少覆盖创建、属性、事件、绘制、销毁。
- 性能任务：说明热路径不做重复字符串查找或全树重算。

## 总体里程碑

- [x] 确认 XUI 声明式界面使用 XSON/XValue，不新增解析器。
- [x] 确认样式继承使用 XValue 父表链，不复制完整字段。
- [x] 确认 XSON 不内嵌脚本，事件只绑定 C 侧注册名。
- [x] 确认 retained widget tree，不每帧重建。
- [x] 确认 XUI 调试能力进入 `xgedbg`，由 `XGE_DEBUGMODE` 控制。
- [x] 确认 style parent 字段固定为 `@parent`。
- [x] 确认 XSON imports 路径解析和资源 provider 第一版规则。
- [!] 确认 XValue 父表生命周期安全机制。原因：当前没有安全机制，先记录为缺陷，通过 page/style arena 保证统一生命周期和统一释放。
- [x] 完成布局核心语义修正。
- [x] 完成 XUI page/loader/binder 基础 API。
- [x] 完成 XSON 基础 widget tree 加载。
- [x] 完成 style/theme/token 继承与 cache。
- [x] 完成常用控件 XSON 化。
- [x] 完成 APP 容器能力。
- [x] 完成 model 数据绑定。
- [x] 完成 debug overlay 和 layout snapshot。
- [x] 完成示例迁移和文档补充。

## 阶段 0：Widget V2 XSON 口径同步

- [x] 建立 `type -> Widget role` 映射：Control、Container、Viewport、Overlay。说明：Page Loader 在 `__xgeXuiPageTypeToRole` 中建立映射，构建 widget 时立即写入 role，测试覆盖 Container、Control、Viewport。
- [x] Control 默认拒绝普通 `children`，只允许明确声明的控件 slot 字段。说明：role 为 Control 的节点出现 `children` 时加载失败，错误路径指向 `tree.children` 或对应子路径。
- [x] Container 允许 `children` 并参与子布局。说明：`column/row/grid` 等容器节点继续通过普通 `children` 构建并参与布局。
- [x] `virtualized ListView` 拒绝普通 `children`，只允许 `itemTemplate`。说明：`virtualized ListView.children` 会加载失败，错误路径指向 `tree.children`，item 内容只能来自 `itemTemplate`。
- [x] 同步 Widget V2 字段：`overflow`、`zIndex`、`layer`、`hitTestVisible`、`inputTransparent`、`tabStop`、`tabIndex`、`imeMode`、`borderColor`、`borderWidth`、`focusRingColor`、`focusRingWidth`、`disabledOverlay`、`debugOutlineColor`、`debugOutlineWidth` 已接入 widget/style/XSON。
- [x] 同步 ScrollViewBase 字段：ScrollView 已支持 `wheelAxis`、`dragMode`/`contentDrag`、`scrollbarDrag`、`nestedScroll`。
- [x] XSON 加载错误必须指出违反 role/children/slot 规则的字段路径。说明：新增 Control children 和 virtualized ListView children 回归测试，错误文本包含具体字段路径。
- [x] XSON 示例按 Widget V2 口径更新，避免继续展示控件可任意挂普通子节点的写法。说明：现有 XSON guide 示例只在 Container/itemTemplate 内使用普通 `children`，Control 示例不再展示任意子节点。

## 阶段 A：布局核心修正

- [x] 明确 `ALIGN_STRETCH` 在 row/column/grid/stack/absolute 中的语义。
- [x] 修正 `ALIGN_STRETCH`：交叉轴默认填满 slot。
- [x] 明确 `grow` 与 `min/max` 的执行顺序。
- [x] 修正 grow 分配后未重新 clamp min/max 的问题。
- [x] 明确空间不足时 Row/Column 的 overflow 策略。
- [x] 增加 `overflow` 策略设计：visible/clip/hidden/scroll；普通 widget 不因 `scroll` 自动创建滚动容器。
- [x] 修正 batch dirty 只标 root 可能跳过深层子树的问题。
- [x] 补充 batch dirty 回归测试。
- [x] 明确 Grid `rowHeight=0` 行为。
- [x] 修正文档与 Grid 实现不一致的问题。
- [x] 评估轻量 span 是否进入第一版 Grid。
- [x] 增加 root/safe area 自动填充策略设计。
- [x] 减少示例中重复 `LayoutRoot + SetRect` 的必要性。

## 阶段 B：XUI Page 与 Loader API

- [x] 定义 `xge_xui_page_t`。
- [x] 定义 `xge_xui_loader_t` 或内部 loader 状态。
- [x] 定义 `xge_xui_binder_t`。
- [x] 定义 `xge_xui_model_t`。
- [x] 增加 `xgeXuiPageLoad` API。
- [x] 增加 `xgeXuiPageLoadMemory` API。
- [x] 增加 `xgeXuiPageUnload` API。
- [x] 增加 `xgeXuiPageRoot` API。
- [x] 增加 `xgeXuiPageFind` API。
- [x] 增加 `xgeXuiPageGetError` API。
- [x] Page 内维护 id/name 索引。
- [x] Page unload 释放由 loader 创建的 widget tree。
- [x] Page load 失败时完整回滚并释放已创建资源。
- [x] Page 支持从 `xgeResourceLoad` 读取 XSON 资源。

## 阶段 C：XSON 基础格式

- [x] 定义顶层字段：`xui/name/theme/imports/styles/tree`。
- [x] 支持 `imports` 加载并合并资源。
- [x] imports 支持带 scheme URI 原样走 `xgeResourceLoad`。
- [x] imports 支持相对路径按当前 XSON 所在目录解析。
- [x] imports 路径规范化，避免重复加载同一资源。
- [x] imports 按数组顺序合并，后加载声明覆盖先加载声明。
- [x] 当前 XSON 本地声明优先于 imports。
- [x] imports 只导入 styles/tokens/templates，不导入 tree。
- [x] 检测 import 循环并输出 URI 链。
- [x] 支持 `tree.type`。
- [x] 支持 `tree.id`。
- [x] 支持 `tree.name`。
- [x] 支持 `tree.children`。
- [x] 支持 `style` 字段引用命名 style。
- [x] 支持 widget inline 字段覆盖 style 字段。
- [x] XSON 错误信息包含资源 URI。
- [x] XSON 错误信息包含字段路径。
- [x] 增加最小 XSON 页面示例。

## 阶段 D：尺寸、边距与布局字段解析

- [x] 支持数字尺寸，默认 px。
- [x] 支持 `"content"`。
- [x] 支持 `"grow"`。
- [x] 支持 `"grow:N"`。
- [x] 支持 `"N%"`。
- [x] 支持 `"Ndip"`。
- [x] 支持对象尺寸 `{ "unit": "...", "value": N }`。
- [x] 支持 `padding` 数字。
- [x] 支持 `padding` 二元数组。
- [x] 支持 `padding` 四元数组。
- [x] 支持 `margin` 数字。
- [x] 支持 `margin` 二元数组。
- [x] 支持 `margin` 四元数组。
- [x] 支持 `layout` 字段。
- [x] 支持 `gap` 字段。
- [x] 支持 `alignX/alignY` 字段。
- [x] 支持 `justify` 字段。
- [x] 支持 `grid.columns` 或 `columns` 字段。
- [x] 支持 `grid.rowHeight` 或 `rowHeight` 字段。
- [x] 支持 `grid.columnGap/rowGap` 字段。
- [x] 支持 `grid.columnSpan` 或 `columnSpan` 子元素字段。
- [x] 支持 `anchor` 对象。

## 阶段 E：XValue 样式继承

- [x] 确认 XValue 字典父表 API：`xvoTableSetParent`。
- [x] 确认 XValue 列表父表 API：`xvoListSetParent`。
- [x] 定义命名 style 的父表标记字段：`@parent`。
- [x] 加载 style 时建立父表链。
- [x] 检测 style 父表循环。
- [x] 样式表、theme table、widget inline table 由 page/style arena 统一持有。
- [x] Page unload 时统一释放样式相关 XValue 表。
- [x] 禁止单独释放仍可能作为父表被查询的 XValue 表。
- [!] 支持 style 继承 theme style。原因：第一版已存在 C 侧 `xgeXuiStyleFromTheme` 默认样式路径；XSON theme/token 与 style cache 未完成前，不引入额外 XValue 父表链，避免生命周期和 cache 失效规则提前固化。
- [!] 支持 engine default style 作为最终父表。原因：当前 widget 创建与 C 侧 theme/style API 已提供 engine default；XValue 形式的默认表推迟到 token/theme/cache 阶段统一设计。
- [x] 定义 widget inline table 到 named style 的覆盖关系：第一版采用显式 overlay lookup，不直接给 widget node 设置父表，避免 `children` 等结构字段被样式继承。
- [x] 禁止或忽略 `children` 参与样式继承。
- [x] 增加 style lookup 单元测试。
- [x] 增加 style parent chain 共享验证。

## 阶段 F：Style Cache 与版本

- [x] 定义 page style version。
- [x] 定义 theme version。
- [x] 定义 widget style cache dirty 标记。
- [x] 将布局高频字段解析到 layout style cache。
- [x] 将绘制高频字段解析到 visual style cache。
- [x] style cache dirty 时触发布局或绘制 dirty。
- [x] theme token 改变时使相关 page cache 失效。说明：page 记录 context theme version，并提供显式 `xgeXuiPageRefreshStyle` / `xgeXuiPageSyncStyle` 路径；不在 layout/paint 热路径重新解析 XSON。
- [x] 避免 layout/paint 热路径重复字符串查找。说明：解析发生在 load/refresh/sync 阶段，layout/paint 继续读取 `xge_xui_style_t` cache。
- [x] 增加 style cache 失效测试。说明：已覆盖 context token 改变后 page style cache sync 的布局/视觉字段更新。

## 阶段 G：Theme Token

- [x] 定义颜色 token。
- [x] 定义 spacing token。
- [x] 定义 font token。
- [x] 支持 XSON 样式值引用 `@token`。
- [x] 支持 C 侧注册 token。
- [x] 支持 page load 时解析 token。
- [x] 字体 token 只引用外部生命周期字体，不由 XSON 直接释放。
- [x] token 缺失时提供明确错误或 fallback 策略。

## 阶段 H：事件绑定

- [x] 定义 binder 初始化 API。
- [x] 支持 `onClick` 绑定。
- [x] 扩展 `onMouseEnter/onMouseLeave/onMouseMove/onMouseDown/onMouseUp/onMouseWheel/onDoubleClick/onContextMenu`。说明：已通过 `xgeXuiBinderSetEvent` 注册事件名，并在 page load 阶段固化到 Widget 分类型事件槽；不绑定到控件私有 raw event 补丁。
- [x] 扩展 `onKeyDown/onKeyUp/onTextInput`。说明：已通过 Widget 键盘事件与文本输入事件槽分层绑定，IME composition 提交只进入文本输入路径。
- [x] 扩展 `hotkey` / `command` 字段。说明：已接入 Widget HotKey 注册表和 Command 分发；声明式层在 page load 阶段注册热键，按键热路径不扫描 XSON tree 或字符串。
- [!] 支持 `onChange` 绑定。原因：依赖 input/value 控件 XSON 化和状态模型，随阶段 I/K 落地。
- [!] 支持 `onSelect` 绑定。原因：依赖 choice/list/select 控件 XSON 化和选择状态模型，随阶段 I/K 落地。
- [!] 支持 `onSubmit` 绑定。原因：依赖 input/form 类控件 XSON 化和提交语义，随阶段 I/K 落地。
- [x] 未注册事件名给出加载警告或错误策略。
- [x] 事件回调携带 widget 和用户指针。
- [x] XSON 内禁止脚本字段。
- [x] 增加 button onClick 示例。

## 阶段 I：基础控件 XSON 化

- [x] `panel`。
- [x] `row`。
- [x] `column`。
- [x] `stack`。
- [x] `grid`。
- [x] `label`。
- [x] `button`。
- [x] `input`。说明：已接入 page control arena、`text/value`、`placeholder`、字体/颜色、`password/readonly/disabled`、selection 和销毁路径；`onChange/onSubmit` 不做空占位，留到阶段 K 数据绑定与状态模型统一实现。
- [x] `image`。
- [x] `separator`。
- [x] 每个控件支持 id/name/style。说明：`panel/absolute/row/column/stack/grid/button/image/input/label/separator` 已支持。
- [x] 每个控件支持必要尺寸与布局字段。说明：`panel/absolute/row/column/stack/grid/button/image/input/label/separator` 已支持。
- [x] 每个控件支持销毁路径。说明：结构型控件由 page widget tree 统一销毁；`button/image/input/label/separator` 已由 page 级固定容量 control arena 持有并在 `xgeXuiPageUnload` 中调用 Unit。

## 阶段 J：APP 容器能力

- [x] 设计 `DockLayout`。说明：第一版作为轻量 APP 容器布局，子项支持 `dock: top/bottom/left/right/fill/center`，按声明顺序扣减剩余 rect，`fill/center` 占用当前剩余区域。
- [x] 实现 `DockLayout`。说明：新增 `XGE_XUI_LAYOUT_DOCK`、子项 dock side style 字段和布局回归测试。
- [x] 支持 Dock XSON 描述。说明：新增 `type:"dock"` 容器和子项 `dock` 字段解析。
- [x] 设计 `SafeArea`。说明：第一版沿用既有决策，safe area 表达为 XUI root padding，page 可通过顶层 `safeArea` 声明应用到 context root。
- [x] 实现 `SafeArea`。说明：加载 page 时保存原 root padding，声明 `safeArea` 后应用，page unload 时恢复。
- [x] 支持 SafeArea XSON 描述。说明：顶层 `safeArea` 支持数字、二元数组、四元数组和 spacing token。
- [x] 完善 `ScrollView` 布局与 hit test 语义。说明：滚动视口命中边界改为 content rect；layout 完成后对 ScrollView 子树应用 offset 偏移，hit test 可命中滚动后的可见子节点。
- [x] 支持 ScrollView XSON 描述。说明：新增 `type:"scrollView"`/`type:"scroll"` 控件声明，page 固定容量 control arena 持有 `xge_xui_scroll_view_t`，支持 content size、offset 和颜色字段。
- [x] 设计 `virtualized ListView`。说明：第一阶段采用固定 item height、可见槽位复用、C adapter 创建/绑定 item widget 的轻量方案；XSON `itemTemplate` 后续复用同一槽位机制。
- [x] 实现固定 item height virtualized ListView。说明：新增 `xge_xui_list_view_t`，根据 content rect 和 scrollY 计算可见区，最多复用 `xge_xui_list_view_SLOT_CAPACITY` 个 slot widget。
- [x] 支持 `itemTemplate`。说明：XSON `type:"virtualized ListView"` 支持 `itemTemplate` 内联对象或顶层 `templates` 名称引用；slot 创建时复用 page widget builder 构建模板子树。
- [x] 支持 C adapter 绑定数据。说明：`SetAdapter(count/create/bind,user)` 负责数据数量、slot 创建和 index 绑定；选择回调使用独立 user 指针，避免覆盖 adapter user。
- [x] 避免为不可见 item 创建 widget。说明：只创建当前可见范围所需 slot，滚动时复用并重新 bind，超出可见范围的既有 slot 标记 invisible。

## 阶段 K：数据绑定

- [x] 定义 model key/value 存储。说明：第一版为固定容量 key/value 文本表，`SetInt/SetFloat` 格式化为文本，成功变更递增 model version。
- [x] 支持 `${key}` 文本绑定。说明：当前只接受字段值整体为 `${key}`，key 仅允许字母、数字、`_`、`.`、`-`。
- [x] 支持 image src 绑定。说明：`xgeXuiPageApplyModel` 根据绑定 key 装载/释放 page 持有的 image texture；空路径表示清空贴图。
- [x] 支持 input value 绑定。
- [x] 支持 model 更新后局部 dirty。说明：apply 阶段调用 label/input/image setter，只标记对应 widget 的 layout/paint dirty，不在 layout/paint 热路径查 model。
- [x] 不支持复杂表达式。说明：`Hello ${name}`、`${a+b}` 等不会被识别为 model binding。
- [x] 不支持 XSON 内脚本。
- [x] 增加数据绑定示例。说明：`test/test_main.c::__testXuiPageApi` 覆盖 label/input/image 三类绑定、model version 和缺失 key 错误。

## 阶段 L：调试与测试

- [x] XSON 加载错误路径测试。
- [x] Page load/unload 泄漏检查。
- [x] Style 继承测试。
- [x] Style cache 失效测试。
- [x] Layout snapshot 输出格式设计。说明：输出 context 摘要、widget tree、layout、flags、dirty、rect/content、margin/padding、desired size。
- [x] Layout snapshot 测试工具。说明：`test/debug_xui_snapshot.c` 覆盖 snapshot、inspect、hit inspect。
- [x] Debug overlay 显示 widget id/type。说明：label 模式输出 id/name/layout/size。
- [x] Debug overlay 显示 rect/content rect。
- [x] Debug overlay 显示 margin/padding。说明：新增 `XGEDBG_XUI_OVERLAY_MARGIN/PADDING`。
- [x] Debug overlay 显示 dirty 状态。说明：新增 `XGEDBG_XUI_OVERLAY_DIRTY`，label 中显示 S/L/P。
- [x] Debug overlay 显示 hit/focus/capture。说明：inspect-at 覆盖 hit；overlay 保留 focus/hover/capture 高亮，snapshot 测试覆盖 focus/capture inspect。
- [x] Layout snapshot API 只在 `XGE_DEBUGMODE` 下声明和实现。
- [x] Debug overlay API 只在 `XGE_DEBUGMODE` 下声明和实现。
- [x] Widget inspector 只在 `XGE_DEBUGMODE` 下声明和实现。
- [x] XSON 详细 trace 只进入 `xgedbg`。说明：新增 `xgedbgXuiPageTrace`，输出 page URI、文档/资源、imports、index、control 和 model binding 状态。
- [x] `xge` 仅保留 XSON 最小错误码和必要错误字符串。说明：运行版仅保留 `xgeXuiPageLoad*` 返回码与 `xgeXuiPageGetError` 必要错误字符串，不声明/导出 trace API。

## 阶段 M：示例与文档

- [x] 新增 `examples/xui_xson_page_lab`。
- [x] 新增 `examples/xui_xson_style_lab`。
- [x] 新增 `examples/xui_xson_app_layout_lab`。
- [x] 新增 `examples/xui_listview_xson`。
- [x] 将一个现有手写 layout 示例迁移为 XSON。
- [x] 更新 `docs/guide/xui-layout-intro.md`。
- [x] 新增 XSON UI 入门文档。
- [x] 新增样式继承说明文档。
- [x] 新增 APP UI 开发范例文档。

## 阻塞与决策记录

- [x] 决策：XUI 声明式描述使用 XSON/XValue，不新增解析器。
- [x] 决策：样式继承使用 XValue 父表链，字段共享，不默认 flatten。
- [x] 决策：运行期需要 style cache，避免热路径重复查找。
- [x] 决策：XSON 不支持脚本，事件只绑定 C 侧注册名。
- [x] 决策：XUI debug overlay、layout snapshot、widget inspector 属于 `xgedbg`，不进入正式 `xge`。
- [!] 待确认：XValue 父表 API 的准确函数名和生命周期约束。结论：当前没有父表所有权安全机制，记录为缺陷；第一版通过 page/style arena 保证样式表生命周期一致并统一释放。
- [x] 待确认：XSON imports 的路径解析与资源 provider 规则。结论：带 scheme URI 原样走 `xgeResourceLoad`，相对路径按当前 XSON 所在目录解析，imports 只合并 styles/tokens/templates。
- [x] 待确认：style parent 字段最终命名。结论：固定为 `@parent`。
- [x] 验证：2026-04-29，`xgedbgXuiLayoutSnapshot`、`xgedbgXuiDebugOverlayPaint`、`xgedbgXuiWidgetInspect`、`xgedbgXuiWidgetInspectAt` 已在构建分离阶段落地，并通过 `build_debug_api_compile_test.bat` 与 `build_verify_xge_split.bat` 验证。
- [x] 决策：2026-04-29，`ALIGN_STRETCH` 只覆盖可伸缩尺寸类型，即 `content` 和 `grow`；显式 `px`、`dip`、`percent` 尺寸不被 stretch 强行覆盖。
- [x] 修复：2026-04-29，Row/Column 的交叉轴 stretch、Stack/Grid 的双轴 stretch 会把 `content`/`grow` 子项填满 slot，并在 grow/stretch 后重新执行 min/max clamp。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 measured content widget 的 Row/Column/Stack/Grid stretch 回归测试；`build_test.bat` 通过。
- [x] 验证：2026-04-29，布局 stretch 修复后，`build_verify_xge_split.bat` 通过。
- [x] 决策：2026-04-29，Row/Column grow 主轴按 `fixed -> remaining -> grow weight -> min/max clamp -> 释放空间重分配` 执行；当 min 约束超出可用空间时允许进入 overflow，具体 overflow 策略留给后续任务。
- [x] 修复：2026-04-29，Row/Column grow 分配增加 min/max 约束重分配，避免某个 grow 子项被 max 限制后留下未使用空间。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 row grow max、row grow min、column grow max 回归测试；`build_test.bat` 与 `build_dbg_test.bat` 均通过。
- [x] 验证：2026-04-29，grow/min/max 修复后，`build_verify_xge_split.bat` 通过。
- [x] 决策：2026-04-29，Grid `rowHeight <= 0` 表示使用单元格宽度作为行高，形成方格网格；不表示 0 高度，也不在第一版做内容推导行高。
- [x] 修复：2026-04-29，修正 `docs/api/xui.md`、`docs/api/xui.en.md` 与 `dev/docs/布局系统说明.md` 中关于 Grid `rowHeight=0` 的描述，使其与实现一致。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 Grid `rowHeight=0` 方格单元回归测试；`build_test.bat` 与 `build_dbg_test.bat` 均通过。
- [x] 验证：2026-04-29，Grid 文档修正后，`build_verify_xge_split.bat` 通过。
- [x] 修复：2026-04-29，`xgeXuiLayoutBatchEnd` 在 batch layout dirty 时标记整棵 layout tree，避免只标 root 导致深层子树因父节点未 dirty 且 rect 未变而被跳过。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 batch 中修改 grandchild 尺寸的回归测试，确认 `xgeXuiUpdate` 后深层子节点完成重新布局。
- [x] 验证：2026-04-29，batch dirty 修复后，`build_test.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 决策：2026-04-29，XUI root 与 overlay root 在 `xgeXuiUpdate` 中自动跟随窗口尺寸；safe area 表达为 root padding，overlay root 保持全窗口，便于弹窗、菜单和调试层覆盖。
- [x] 修复：2026-04-29，新增 `xgeXuiSetSafeAreaPx`，用于设置 root content rect 的安全区内边距，减少业务示例为外边距反复手写 `LayoutRoot + SetRect` 的必要性。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 root 自动尺寸与 safe area content rect 回归测试；`build_test.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 决策：2026-04-29，Row/Column 空间不足时不自动创建滚动容器；第一版 overflow 映射为默认 `visible`、widget `clip`、显式 `ScrollView/ListView` 三类能力，避免引入完整 CSS overflow 模型。
- [x] 修复：2026-04-29，`xgeXuiWidgetSetClip` 的语义从仅绘制裁剪扩展为绘制和子节点命中裁剪：命中点位于 widget rect 内但在 `tContentRect` 外时返回该 widget，不继续命中溢出的子节点。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 clip hit-test 回归测试；`build_test.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-04-29，ScrollView 的事件命中改为 content rect；布局完成后对 ScrollView 子树应用滚动 offset；page XSON 新增 `type:"scrollView"`/`type:"scroll"`、content size、offset 和颜色字段。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 ScrollView 子内容滚动偏移、content rect 命中和 XSON scrollView 回归测试；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-04-29，新增轻量 `virtualized ListView` C API：固定 item height、content rect 命中、滚轮/拖动滚动条/键盘选择、slot widget 复用、adapter count/create/bind 绑定。
- [x] 验证：2026-04-29，`test/test_main.c` 已增加 virtualized ListView 可见 slot 数、滚动复用、content rect 命中、选择回调和 Unit 清理回归测试；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-04-29，page XSON 新增 `virtualized ListView`/`itemTemplate` 支持：page 固定容量 arena 持有 `xge_xui_list_view_t`，模板 slot 由 virtualized ListView 可见范围驱动创建；`xgeXuiPageRefreshStyle` 会跳过运行时 slot 子树长度校验。
- [x] 验证：2026-04-29，`test/test_main.c` 已增加 XSON virtualized ListView 模板实例化、slot 复用、模板控件 arena 计数和 refresh style 回归；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 决策：2026-04-29，Grid 第一版加入轻量 `columnSpan`，不加入 row span、命名区域或占用矩阵回填，保持单次遍历和 O(N) 布局。
- [x] 修复：2026-04-29，新增 `xgeXuiWidgetSetGridColumnSpan` 和 `xge_xui_style_t.iGridColumnSpan`；Grid 子元素横跨多列时，如果当前行剩余列不足会换到下一行再放置。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 Grid column span 回归测试；`build_test.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 决策：2026-04-29，阶段 B 先落 XUI page/loader/binder/model 公共结构和最小 API；XSON widget tree 构建进入阶段 C，第一版先支持 retained widget tree，不在阶段 B 展开 style/imports/control 细节。
- [x] 修复：2026-04-29，新增 `xge_xui_page_t`、`xge_xui_loader_t`、`xge_xui_binder_t`、`xge_xui_model_t`，以及 `xgeXuiPageLoad`、`xgeXuiPageLoadMemory`、`xgeXuiPageUnload`、`xgeXuiPageRoot`、`xgeXuiPageFind`、`xgeXuiPageGetError`、`xgeXuiBinderInit`、`xgeXuiBinderSetClick`。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 Page/Binder API 回归测试，覆盖 memory load、URI resource load、error、unload、binder 注册替换、PageFind；`build_test.bat` 与 `build_dbg_test.bat` 均通过。
- [x] 修复：2026-04-29，阶段 C 最小 XSON tree 已支持 `tree.type/id/name/children`，`PageLoad*` 可创建 retained widget tree 并挂到 XUI root。
- [x] 验证：2026-04-29，`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-04-29，阶段 C 支持顶层 `styles`、widget `style` 命名引用、style `@parent` 继承，以及 widget inline 字段覆盖 style 字段。当前解析 `layout/width/height/minWidth/minHeight/maxWidth/maxHeight/gap/align/alignX/alignY/justify/background/radius/borderColor/borderWidth/focusRingColor/focusRingWidth/disabledOverlay/debugOutlineColor/debugOutlineWidth/clip/overflow/layer/z/zIndex/hitTestVisible/inputTransparent/tabStop/tabIndex/imeMode/gridColumns/gridColumnSpan`。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 style 继承、inline 覆盖、百分比尺寸、clip 标记回归测试；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-04-29，`PageLoad*` 的 XSON 加载错误信息统一包含资源 URI 与字段路径，例如 `xui://bad: tree.children: expected array`。
- [x] 修复：2026-04-29，新增 `examples/xui_xson_page_lab` 最小 XSON 页面示例，并纳入 `build_verify_xge_split.bat`。
- [x] 验证：2026-04-29，新增 XSON 错误 URI/路径回归测试；`examples/xui_xson_page_lab/build.bat` 与示例 exe 运行通过；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat`、`build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-04-29，XSON `imports` 第一版落地：支持 URI/相对路径解析、导入去重、循环检测、按顺序合并 `styles/tokens/templates`，并保证本地声明覆盖 imports。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 imports 相对路径、覆盖顺序、本地优先、忽略 imported tree、循环检测回归测试；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-04-29，阶段 D 布局字段解析落地：支持 `grow:N`、对象尺寸、margin/padding 数字与数组、anchor 对象、grid 嵌套字段和顶层别名。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 Phase D XSON 字段解析回归测试；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-04-29，style `@parent` 改为显式解析栈循环检测；自继承和多节点循环都会使 page load 失败并返回 `style parent cycle`。
- [x] 文档：2026-04-29，补充 XValue 父表生命周期约束：`xvoTableSetParent` 不持有父表，参与继承的 imported styles、merged styles 和当前 page styles 必须由 `xge_xui_page_t` 统一持有并由 `xgeXuiPageUnload` 释放。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 style parent cycle 回归测试；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [!] 决策：2026-04-29，theme style 与 engine default style 暂不作为 XValue 父表接入第一版 style 继承链；当前使用 widget 默认值与 `xgeXuiStyleFromTheme`，后续在 Theme Token 与 Style Cache 阶段统一处理默认表、token 引用和 cache 失效。
- [x] 修复：2026-04-29，Page load 为 XSON 创建出的 widget 建立固定容量 id/name 索引；`xgeXuiPageFind` 优先查索引，索引溢出或手工填充 page root 时回退递归查找。
- [x] 修复：2026-04-29，Page load 失败时回滚 loader 已创建的 resource、document、imports、merged tables 和 widget tree，同时保留 `xgeXuiPageGetError` 可读取的错误字符串。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 Page 索引与失败回滚断言；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-04-29，阶段 F 第一版版本与 dirty 边界落地：新增 context theme version、page style version、widget style version 和 `XGE_XUI_WIDGET_DIRTY_STYLE`；`xge_xui_style_t` 作为轻量 layout/visual style cache。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 theme version、page style version、widget style dirty 回归断言；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-04-29，阶段 G 第一版 token 解析落地：支持 `tokens.colors`、`tokens.spacing`、`tokens.fonts` 命名空间，样式值支持 `@colors.name`、`@spacing.name`、`@fonts.name` 和未限定 `@name` 查找。
- [x] 决策：2026-04-29，font token 第一版只保留命名空间和生命周期规则，不由 XSON 创建或释放字体对象；待文本类 XSON 控件落地后再接入具体字体字段。
- [x] 验证：2026-04-29，`test/test_main.c` 增加颜色 token、spacing token、数组 spacing token、缺失 token 错误路径回归断言；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-04-29，新增 C 侧 context token 注册 API：`xgeXuiTokenSetColor`、`xgeXuiTokenSetSpacing`、`xgeXuiTokenSetFont`。XSON/import token 优先，context token 作为 fallback；注册成功递增 theme version 并标记 root style/layout/paint dirty。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 C 侧 token 注册、theme version 增量、XSON fallback 解析回归断言；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-04-29，阶段 H 第一版 `onClick` 绑定落地：XSON `onClick` 引用 C 侧 `xgeXuiBinderSetClick` 注册名，绑定到 page 创建 widget 的通用事件过程，触发时回调携带当前 widget 和 binder 用户指针。
- [x] 修复：2026-04-29，XSON 事件错误策略落地：未注册 `onClick` 名称会使 page load 失败；`script`、`onClickScript` 被拒绝并返回字段路径错误。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 onClick 正常回调、未注册事件、脚本字段拒绝回归断言；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-05-09，阶段 H 语义事件绑定扩展落地：新增 `xgeXuiBinderSetEvent`，XSON `onMouseEnter/onMouseLeave/onMouseMove/onMouseDown/onMouseUp/onMouseWheel/onDoubleClick/onContextMenu/onKeyDown/onKeyUp/onTextInput` 在 page load 阶段固化到 Widget 分类型事件槽。
- [x] 验证：2026-05-09，`test/test_main.c` 增加通用事件 binder 更新、XSON 语义事件回调、未注册事件路径错误回归断言；`build_dll.bat` 与 `build_test.bat` 已通过。
- [x] 修复：2026-05-09，阶段 H `hotkey` / `command` 声明式绑定落地：`hotkey` 通过 C 侧事件 binder 注册回调，`command` 注册到 Widget HotKey Command 路径，`onCommand` 接收 `XGE_EVENT_XUI_COMMAND`。
- [x] 验证：2026-05-09，`test/test_main.c` 增加声明式 hotkey 触发、command 分发、数组形式、Page unload 清理热键和错误 key 回归断言；`build_dll.bat` 与 `build_test.bat` 已通过。
- [!] 决策：2026-04-29，`onChange/onSelect/onSubmit` 不提供空占位绑定；等待 input/select/form 语义落地后随具体控件实现。
- [x] 修复：2026-04-29，阶段 I 第一版结构型控件类型收紧：`panel/absolute/row/column/stack/grid` 明确支持，未知 `tree.type` 或非字符串 `type` 会使 page load 失败，避免尚未实现控件静默降级。
- [x] 验证：2026-04-29，`test/test_main.c` 增加未知 type 和非字符串 type 错误路径回归断言；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-04-29，阶段 I 接入 `label/separator` XSON 控件：page 内新增固定容量 `label/separator` control arena，`xgeXuiPageUnload` 与失败回滚会在释放 widget tree 前调用对应 Unit。
- [x] 修复：2026-04-29，`label` 支持 `text/font/textColor/color/textAlign/textVAlign`，`separator` 支持 `orientation/thickness/color/background`；font 通过 C 侧 font token 引用，不由 XSON 创建或释放字体。
- [x] 修复：2026-04-29，通用 `onClick` 暂不绑定已接管 `pUser` 或事件过程的状态控件，避免覆盖控件自身状态指针；后续 `button/input` 由具体控件语义接入事件。
- [x] 修复：2026-04-29，阶段 I 接入 `button` XSON 控件：page 内新增固定容量 button arena，支持 `text/font/textColor/textAlign/textVAlign/color/background/hoverColor/activeColor/focusColor/disabledColor/onClick`。
- [x] 修复：2026-04-29，`button.onClick` 通过 `xgeXuiButtonSetClick` 绑定 C 侧注册事件名，复用按钮自身事件过程和状态机，不覆盖控件 `pUser`。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 XSON button 字段解析、font token、颜色状态和 onClick 触发回归断言；`build_dll.bat` 与 `build_test.bat` 已通过。
- [x] 修复：2026-04-29，阶段 I 接入 `image` XSON 控件：page 内新增固定容量 image arena，支持 `texture/src/source/srcRect/color/tint/mode`。
- [x] 修复：2026-04-29，新增 C 侧 `xgeXuiTokenSetTexture` 和 `tokens.textures` 解析；`image.texture` 使用 borrowed texture token，`image.src` 使用 page 自有 texture 并在 unload 中释放。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 XSON image texture token、source rect、color 和 mode 回归断言；`build_dll.bat` 与 `build_test.bat` 已通过。
- [x] 修复：2026-04-29，阶段 I 接入 `input` XSON 控件：page 内新增固定容量 input arena，支持 `text/value/placeholder/font/textColor/background/backgroundColor/focusColor/cursorColor/placeholderColor/selectionColor/disabledTextColor/disabledBackgroundColor/password/readonly/disabled/selection`。
- [x] 决策：2026-04-29，`input.onChange/onSubmit` 不提供空占位绑定；当前解析时严格拒绝，后续随阶段 K model/value 状态模型统一实现。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 XSON input 文本、placeholder、font、颜色、password/readonly/disabled、selection 和 unload 回归断言；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-04-29，阶段 F style cache 失效补齐：page 记录加载时的 context theme version，新增 `xgeXuiPageRefreshStyle` 强制重算 XSON layout/visual cache，新增 `xgeXuiPageSyncStyle` 在 theme/token version 落后时重算。
- [x] 决策：2026-04-29，context token 改变只递增 theme version 并标记 root dirty；已加载 page 通过显式 sync/refresh 更新 cache，不在 layout/paint 热路径重新查 XSON 字符串字段。
- [x] 验证：2026-04-29，`test/test_main.c` 增加 context token 改变后 page style cache sync 回归断言，覆盖 style version 增量、theme version 同步、布局尺寸 token 和背景颜色 token 更新；`build_dll.bat`、`build_test.bat`、`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过。
- [x] 修复：2026-04-29，阶段 K 接入轻量 model/data binding：新增固定容量 `xge_xui_model_t` key/value 存储、`xgeXuiModelSetText/SetInt/SetFloat/GetText/Version` 与 `xgeXuiPageApplyModel`；XSON 支持 label `text`、input `value/text`、image `src` 的 `${key}` 绑定。
- [x] 决策：2026-04-29，第一版 binding 不做表达式、不做字符串插值、不在 XSON 内执行脚本；page load 只登记绑定，model apply 阶段显式同步并局部标记 dirty。
- [x] 修复：2026-04-29，阶段 L 补齐 xgedbg XUI snapshot/overlay 输出：snapshot 增加 layout、dirty、margin/padding；inspector 增加 dirty 与 margin/padding 字段；overlay 增加 margin/padding/dirty flags 与更完整 label。
- [x] 验证：2026-04-29，`build_dbg_dll.bat`、`build_dbg_test.bat` 与 `build_verify_xge_split.bat` 均通过，确认 XUI 调试 API 仍只存在于 `xgedbg`。
- [x] 修复：2026-04-29，阶段 L 新增 `xgedbgXuiPageTrace`，作为 XSON page 详细 trace 的 debug-only API；普通 `xge` 不声明、不导出该接口。
- [x] 验证：2026-04-29，`test/debug_api_release_fail.c` 增加 `xgedbgXuiPageTrace` release 编译边界断言，`test/debug_xui_snapshot.c` 增加 page trace 内容断言；`build_verify_xge_split.bat` 已通过。
- [x] 修复：2026-04-29，阶段 M 新增 `examples/xui_xson_style_lab`，覆盖 XSON tokens、style `@parent` 继承、inline 覆盖和 button/row 样式验证。
- [x] 验证：2026-04-29，`examples\xui_xson_style_lab\build.bat` 构建成功，`build\xge_xui_xson_style_lab.exe` 运行通过；`build_verify_xge_split.bat` 已纳入该示例。
- [x] 修复：2026-04-29，阶段 M 新增 `examples/xui_xson_app_layout_lab`，覆盖 APP 风格 dock shell、header、左侧导航、scroll content、grid cards 与 status bar 的声明式布局。
- [x] 验证：2026-04-29，`examples\xui_xson_app_layout_lab\build.bat` 构建成功，`build\xge_xui_xson_app_layout_lab.exe` 运行通过；`build_verify_xge_split.bat` 已纳入该示例。
- [x] 修复：2026-04-29，阶段 M 新增 `examples/xui_listview_xson`，覆盖 XSON `virtualized ListView`、`itemTemplate`、可见窗口计算和 slot 复用。
- [x] 验证：2026-04-29，`examples\xui_listview_xson\build.bat` 构建成功，`build\xge_xui_listview_xson.exe` 运行通过；`build_verify_xge_split.bat` 已纳入该示例。
- [x] 修复：2026-04-29，阶段 M 新增 `examples/xui_xson_layout_gallery_lab`，将手写 `examples/xui_layout_gallery` 的核心 layout gallery 迁移为 XSON 页面，覆盖 absolute/row/column/stack/grid/justify/content/grow/percent/px 几何验证。
- [x] 验证：2026-04-29，`examples\xui_xson_layout_gallery_lab\build.bat` 构建成功，`build\xge_xui_xson_layout_gallery_lab.exe` 运行通过；`build_verify_xge_split.bat` 已纳入该示例。
- [x] 文档：2026-04-29，更新 `docs/guide/xui-layout-intro.md` 与 `docs/guide/xui-layout-intro.en.md`，补齐当前布局能力、XSON 声明式布局、safe area、Dock、ScrollView、virtualized ListView、dirty/batch 和 xgedbg 隔离说明。
- [x] 验证：2026-04-29，已核对布局入门文档引用的公开 API 名称存在于 `xge.h`，并执行 `git diff --check`。
- [x] 文档：2026-04-29，新增 `docs/guide/xui-xson-intro.md` 与 `docs/guide/xui-xson-intro.en.md`，覆盖 XSON 页面结构、tokens/styles/tree、imports、事件绑定、model binding、virtualized ListView 模板、错误路径和 xgedbg trace 边界，并加入 guide 索引。
- [x] 验证：2026-04-29，已核对 XSON 入门文档引用的公开 API 名称存在于 `xge.h`，并确认引用示例目录存在。
- [x] 文档：2026-04-29，新增 `docs/guide/xui-style-inheritance-intro.md` 与 `docs/guide/xui-style-inheritance-intro.en.md`，覆盖 `@parent`、inline 覆盖、tokens、imports 合并顺序、XValue 父表生命周期、style cache refresh/sync 和当前限制，并加入 guide 索引。
- [x] 验证：2026-04-29，已核对样式继承文档引用的公开 API 名称存在于 `xge.h`，并确认引用示例/测试路径存在。
- [x] 文档：2026-04-29，新增 `docs/guide/xui-app-ui-intro.md` 与 `docs/guide/xui-app-ui-intro.en.md`，覆盖 APP shell 推荐结构、Dock/SafeArea/ScrollView/virtualized ListView 选型、C 侧加载、事件/model 更新、style sync、运行循环和 xgedbg 隔离，并加入 guide 索引。
- [x] 验证：2026-04-29，已核对 APP UI 文档引用的公开 API 名称存在于 `xge.h`，并确认引用示例目录存在。
- [x] 验证：2026-04-29，阶段 L 测试清单收口：`test/test_main.c` 已覆盖 XSON URI/字段路径错误、失败回滚、provider free 计数、style parent cycle、style 继承、context token fallback、style cache sync/refresh；将阶段 L 遗留测试项标记完成。

## 当前开发指针

下一次开发前请先更新本节：

- 当前任务：阶段 0 XSON role/children 规则收口。
- 当前状态：`type -> Widget role` 映射已接入 Page Loader；Control 普通 `children`、`virtualized ListView.children` 会被拒绝并返回字段路径错误；Container 普通 `children` 保持可用。
- 最近更新时间：2026-05-08。
