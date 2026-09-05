# 可嵌入窗体设计器 0.1

这是独立的 C / XUI 组件和参考壳程序，不是把旧编辑器的 `main.c` 嵌进宿主。
它实现 WinForms / VB 式的基础工作流：画控件、修改 `(Name)` 和属性、双击默认事件、
编辑业务代码、编译并操作真正的运行界面。0.1 不迁移到 .NET，也不承诺完整复刻 Visual Studio。

## 构建和体验

从仓库根目录执行：

```powershell
./tools/UIDesign/build_component.ps1 -Test
./tools/UIDesign/release/component/uidesign_shell.exe
```

需要匹配的 `build/xge.dll`、`build/xge.lib`、公开 SDK 头文件和 64 位 MinGW GCC。
可用 `-Compiler C:/path/to/gcc.exe` 指定编译器。脚本只读根 SDK，输出到独立的
`tools/UIDesign/release/component/`；不会调用根构建或覆盖旧 `xui_uidesign.exe`。
SDK 更新后请重新执行本脚本，避免头文件、导入库和运行时不匹配。

产物：

- `libuidesign.a`：可嵌入组件静态库。
- `uidesign_shell.exe`：只使用公开接口的参考宿主。
- `component_test.exe`：无窗口的文档、组件和宿主回调测试。
- `exercise.uidproj`：端到端测试生成的完整示例，可在壳程序中 Open。
- `design.png`、`code.png`、`run.png`：真实渲染截图，不是设计稿。
- `generated/run-*/`：每次运行的生成代码、业务代码编译副本、DLL 和编译日志。

操作顺序：

1. 左侧选择 Button / Input / CheckBox 等，在白色窗体上拖出控件；Panel 内可以继续画子控件。
2. 右侧改 `(Name)`、Text、位置、尺寸、颜色、字体和行为属性。
3. 双击控件，或切换 Events，点击事件行的 `...` 创建／定位处理函数；也可输入已有函数名。
4. 在 Code 中修改 C 业务代码，按 F5 或 Run。点击按钮、输入文本、勾选复选框会执行编译后的函数。
5. Shift+F5 / Stop 回到设计。F7 切到代码，Shift+F7 切回设计，Ctrl+S 保存。

设计画布支持单选、Ctrl / Shift 多选、框选、拖动、右下角缩放、8 单位吸附、
左右/上下坐标辅助线、左对齐/顶对齐、方向键微调、Delete、Ctrl+Z / Ctrl+Y、Esc 取消。
画布缩放不写入文档。中文 UI 字体默认使用宿主提供的 Microsoft YaHei 18 px；组件不会修改宿主默认字体。

## 职责边界

| 层 | 负责什么 | 不负责什么 |
| --- | --- | --- |
| `uidesign.h` | 控件/属性/事件元数据、文档树、稳定 ID、命名、绑定、事务、撤销、快照、JSON | 窗口、消息循环、磁盘、编译器 |
| `uidesign_xui.h` | 嵌入视图、属性表、控件适配器、选择通知、自定义属性编辑器、预览事件分发 | IDE 文本缓冲区、语言语义、项目系统 |
| 可选 `uidGenerateC` | 从不可变快照生成 C 头文件和源文件；失败时不交付半成品 | 写文件、合并业务代码、执行编译 |
| 参考 shell | 窗口/帧循环、字体、文件对话框、代码编辑、处理函数、编译/加载/卸载、日志 | 访问组件私有结构 |

因此 IDE 可复用组件内核/视图，并把 shell 的代码服务替换成自身的编辑器、编译器和导航服务。
非 C 宿主也可以直接遍历快照生成自己的目标语言，而不调用内置 C 后端。

## 最小嵌入方式

宿主已有 XUI context、父容器和字体时，核心接入如下。省略部分统一错误处理；完整可运行实现见 `../shell/main.c`。

```c
#include "uidesign_xui.h"

uid_context *registry = NULL;
uid_document *document = NULL;
uid_editor *editor = NULL;

uidContextCreate(&registry);
uidXuiRegisterBuiltins(registry);
/* 在创建视图前注册宿主扩展控件、属性和事件。 */
uidDocumentCreate(registry, "MainForm", &document);

uid_editor_desc desc = {0};
desc.size = sizeof(desc);
desc.version = UID_API_VERSION;
desc.xui = host_xui_context;
desc.parent = host_designer_pane;
desc.font = host_ui_font;
desc.font_size = 18;
desc.user = host;
desc.invalidate = host_request_frame;
desc.ensure_handler = host_ensure_handler;
desc.show_code = host_goto_handler;
desc.validate_handler = host_validate_handler;
desc.resolve_font = host_resolve_font;
desc.selection_changed = host_selection_changed;
desc.edit_property = host_custom_property_picker;
uidEditorCreate(registry, document, &desc, &editor);

/* 宿主布局回调：组件根节点是普通 XUI widget。 */
xuiLayoutArrangeChild(host_designer_pane, uidEditorRoot(editor), available_rect);

/* 宿主每帧：统一输入、一次布局和渲染。组件不创建第二个帧循环。 */
xuiProxyXgePumpInput(host_xui_context); /* 使用其他 proxy 时由宿主转换输入 */
xuiDispatchPendingEvents(host_xui_context);
uidEditorUpdate(editor);
xuiLayout(host_xui_context);
/* 然后按宿主原有方式 update / render / present。 */

/* 宿主父容器、字体和 XUI context 必须比组件活得更久。 */
uidEditorDestroy(editor);
uidDocumentDestroy(document);
uidContextDestroy(registry);
```

公开句柄均为 opaque；宿主不应包含 `src/uid_internal.h` 或 `src/uid_xui_internal.h`。
`uidEditorRoot/Canvas/PropertyGrid/Tree/NodeWidget` 返回借用句柄，主要用于布局、查询和测试。
不要绕开模型直接编辑设计控件；这些控件会重新创建，`NodeWidget` 在下一次 `uidEditorUpdate` 后可能失效。

### 注册属性和事件

`uidRegisterControl` 深拷贝 `uid_control_desc`、属性表、事件表和字符串；不会写死特定宿主。
结构体的 `size/version` 必须匹配。类型 ID 不可重复替换；回调函数和 `user` 的生命周期由宿主管理。

属性由 `uid_property_desc` 描述：稳定 ID、显示名、分类、帮助、类型、默认值、范围和校验回调。
0.1 的值类型是 string / number / bool / RGBA color。

- `UID_MULTIEDIT`：允许同类型公共属性的多选批量修改；一次修改只有一条撤销记录。
- `UID_READONLY` / `UID_HIDDEN`：只读／不显示。
- `validate`：校验后才提交模型；失败不留半次修改。
- `UID_CUSTOM_EDITOR`：显示 `...`，调用宿主 `edit_property`；颜色/资源/表达式等选择器由宿主提供。
  返回 `UID_OK` 和 typed value 才提交，`kind=0` 表示取消。文档或选择已变化时拒绝应用过期结果。
- 默认值重置通过 Reset 或 `uidResetProperty`；控件名不作为普通默认值重置。

`uid_xui_adapter.create/apply/child_host` 把文档节点转成 XUI 控件及子容器。
自定义控件可用 `bind_events` 保存组件提供的 emitter，并通过
`emit("accepted", uidNumber(42), emitter)` 发出自己注册的事件。
组件核对事件和值类型，查找文档中的 handler，再向 `uid_runtime_host.dispatch` 发送消息。
设计模式不会绑定这些运行事件。

事件绑定持久化的是 `{ node_id, event_id, handler_identifier }`，不是函数指针或用户函数体。
双击默认事件通过 `ensure_handler` 让宿主创建或确认函数，再提交绑定并 `show_code`。
重复双击应幂等；解绑、删控件和撤销不会删除业务代码。
0.1 shell 以 `void Name(const uid_runtime_event *, const shell_services *)` 作为统一业务签名；
其他宿主可选自己的函数命名、签名、事件路由和代码组织方式。

### 通知、事务、保存与生成

- `uidSubscribe` / `uidUnsubscribe`：文档提交、Undo、Redo、保存基线通知，带 document ID、revision、dirty 和撤销可用状态。
- `selection_changed`：在 Update 合并发送，选择变化不会进入文档或污染 dirty。
- `uidBegin/Commit/Cancel`：宿主多步修改可组成一个事务。事务内发生错误时，调用方负责 Cancel。
- `uidEditorFlushPendingEdits`：保存、运行、切换文档前调用。非法属性保持编辑态，不会静默丢失。
- `uidSnapshotCreate`：深复制、只读的数据快照；进行中的拖动/事务不能创建快照。
- `uidNodeAt` / `uidGetPropertyAt` / `uidGetBindingAt`：宿主生成器完整遍历节点、持久化属性和绑定，包括未识别的扩展数据。
- `uidDocumentMarkSaved(document, saved_snapshot)`：只在实际写盘成功后设置已保存内容；保存旧快照不会错误清除较新修改的 dirty。
- `uidAcceptResult`：拒绝异步编译等任务返回的过期 document ID / revision。

除 XUI 自身 API 外，组件回调与函数统一采用 `UID_OK == 0` 的结果约定。
所有操作均在宿主 UI 线程执行；快照虽然不可变，0.1 API 仍要求 UI 线程调用。
跨线程工作请先在 UI 线程导出独立 JSON 或代码文本，在线程/子进程中处理，最后回 UI 线程校验 revision。
通知和校验回调中只查询，不直接修改文档；校验器应确定且无副作用，加载快照/撤销恢复时也会校验。
视图/运行回调中不要同步销毁正在回调的组件或宿主。
需要切换/销毁时，像参考 shell 一样排队到事件派发结束后执行。

字符串是 UTF-8。普通 getter 返回借用视图，在文档修改后失效；serialize 结果用 `uidFree` 释放，
artifacts 用 `uidCArtifactsFree` 释放。先销毁 editor / preview，再销毁 document / snapshot，最后 context。
存在依赖对象时 `uidContextDestroy` 返回 `UID_BUSY`。注册表可共享多个文档，文档的撤销、脏状态和绑定相互独立。
运行事件的字符串、sender 和 services 也是借用数据；如需跨帧使用文本，业务代码必须自行复制。
节点 ID 随现存节点保存，且在一个打开的文档生命周期内不会回收；不是跨项目的全局标识。

## 实际运行链路

```text
拖画 + 属性表 → 文档快照 → 生成 form.generated.h / .c
双击事件     → 宿主业务代码缓冲区 → business.c 编译副本
两份 C 代码  → GCC 生成 form.dll → 创建真实 XUI 控件
用户点击/输入 → 编译的 XUI 回调 → 宿主 dispatch → 编译的业务函数
```

生成头文件暴露 `form_controls` 中的命名控件句柄，以及 `form_create/destroy/find`。
生成代码只链接 XGE/XUI 运行时，不链接设计器库。壳程序的 `shell_services.find` 允许业务函数访问其他命名控件。
生成器不会覆盖用户函数；每次编译使用新目录，旧运行界面销毁后才卸载 DLL。
编译器由 Windows Job 管理，取消不会误杀别的构建任务。

注意：0.1 shell 在**当前进程**加载编译的 DLL，仅适用于可信的本地业务代码。
打开项目不会自动编译/运行，但按 Run 后代码拥有宿主进程权限，错误代码可能使壳程序崩溃。
不可信项目应由未来 IDE 宿主放到独立运行进程并增加权限隔离；这不是 0.1 的安全沙箱。

## 文件与边界

组件文档格式为 `xui.uidesign.component` v1 JSON。
壳程序 `.uidproj` 是单文件包：`UIDSHELL1\n`、十进制 JSON 字节数、业务代码字节数（各占一行），
随后拼接 UTF-8 JSON 和业务代码。单文件原子替换避免设计和业务代码保存一半；写盘失败不清 dirty。

本里程碑的明确边界：

- 内置 Form、Panel、Label、Input、Button、CheckBox；不是原编辑器所有控件的迁移版本。
- 一个文档同时最多一个编辑视图；可以有多个文档/编辑器实例和独立运行预览。
- 内置容器只有 `content` 插槽和固定坐标布局；高级布局、Tab、复杂集合属性等留给后续里程碑。
- 缺失扩展的节点和属性/事件数据保留，设计器给占位控件；内置 C 后端遇到不支持的内容明确失败。
  自定义适配器只有在语义完全等同于内置控件时才能用 `c_runtime_type`；其他扩展需宿主自己的生成后端。
- 内置属性表单行编辑缓冲区约 1 KiB，超长值以只读预览保留；可通过模型 API / 自定义编辑器修改最长 4096 字节的字符串。
- 画布暂不含复制粘贴、平移滚动、八方向缩放、同级智能基线吸附及跨容器拖放重父级。
  重父级模型 API 已有；旧工作台的部分成熟交互会在适配迁移阶段复用。
- 暂不做源码反向解析、全局重构、自动删除用户函数、事件候选函数的语义索引、撤销 UI + 源码的跨系统事务。
  重命名控件保持稳定 ID 和已有 handler 绑定，不擅自重命名用户函数。
- 旧 `xui.uidesign` 文件**不会被隐式转换或覆盖**。旧编辑器保留，迁移器将在覆盖足够控件后另行实现。
- 文件变更冲突检测、多文档壳、进程隔离预览、完整 IDE 文档/调试服务尚不在本壳内。

默认限额：每文档 256 节点、每控件 48 属性 / 8 事件、JSON 8 MiB、32 条撤销记录和约 32 MiB 历史预算；
超过限额返回错误。注册表最多 64 种控件，0.1 参考工具箱面向小型控件集；大型库可由宿主自建工具箱调用 `uidEditorSetTool`。

## 验证

`build_component.ps1 -Test` 同时执行：

1. 模型与无窗口 XUI 测试：动态元数据深拷贝、校验/自定义属性编辑回调、选择通知、命名冲突、事务、
   Undo/Redo、稳定 ID、保存基线、旧结果识别、文档编解码、未知扩展保留、代码生成、父子重排、
   实际属性编辑器提交/拒绝、多实例隔离、自定义 typed 事件、宿主所有权。
2. 窗口端到端测试：真实 XUI 输入链路拖画五个控件（覆盖六种内置类型）、嵌套、移动/缩放/撤销/Esc、
   缩放不污染模型、双击事件幂等创建、修改业务代码、保存重开、真实 F5、GCC 编译、按钮/输入/勾选调用、
   停止重启、编译失败日志、过期结果拒绝、编译进程取消，以及三张截图。

无窗口测试使用仓库已有的 `test_xui/xui_test_proxy.c`；端到端测试使用真实 XGE proxy 和实际 GCC，
不是模拟 handler 计数或直接调用运行按钮回调。
