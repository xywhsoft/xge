# Document 重构实施与验证记录

日期：2026-09-16。平台：本机 Windows x64，GCC 16.1.0，实际 XGE DLL 与 GPU 后端。

统一内核和原生显示、编辑基础已经实现。整个重构方案仍在实施，尚未达到方案规定的最终发布标准或成熟前端 Markdown 编辑器的全部功能。新实现已加入主 DLL；旧 RichDocument/RichEdit 仍在活动构建中，尚未完成仓库切换。

## 已落地内容

| 模块 | 已实现及验证的范围 |
| --- | --- |
| Document | RICH/MARKDOWN 共用节点树、64 位结构位置、持久化存储、不可变快照、根事务、共同历史、订阅与保存点 |
| 原子性 | 失败事务不发布内容、revision、历史或通知；过期位置拒绝；回调重入受控 |
| 表格内核 | Cell 属于根树，行列增删、矩形合并/拆分、跨度校验与根撤销 |
| Markdown | 固定版本 MD4C、CommonMark/GFM 及声明扩展的节点解析、原始源码保留、SOURCE 写入与有限 VISUAL 回写 |
| 三模式 | Source、Visual、块级 Live Markdown；模式切换共用 Document 和历史，不导出再导入 |
| Renderer/View | 可见区优先布局、块高度索引、嵌套内容、单元格递归布局、绘制/命中/光标、选择与多宽度视图 |
| Editor | 文本与段落输入、字素删除、导航、基础 marks、命令状态、只读、纯文本剪贴板、IME 候选投影和选区撤销 |
| 查找 | 区分大小写的 UTF-8 字面查找、跨样式片段查找、原子全部替换；多处源码替换只解析一次 |
| IO | 原生 schemaVersion 1 JSON、Markdown 保存、纯文本/转义 HTML 导出、UTF-8 路径与原子写入 |
| 宿主接口 | XUI 通用编辑协议、基础文本辅助技术、自动高度 View 的父级滚轮传递 |

API、线程、所有权、接入方法与详细行为见 [XUI_DOCUMENT.md](XUI_DOCUMENT.md)。

## 可重复运行的验证

仓库根目录执行，GCC/windres 必须位于 PATH：

```bat
call test_xui\build_document_suite.bat artifacts\xui-document-rebuild\commonmark-0.31.2.json
```

不传语料路径时明确跳过官方语料。文件取自 [CommonMark 0.31.2 官方样本](https://spec.commonmark.org/0.31.2/spec.json)，SHA256：

```text
d431b29d97b6f73e69d547109cf5081578fac931e72afe95639ebe766c1b2a20
```

| 验证目标 | 结果与含义 |
| --- | --- |
| 独立 core | 通过；不链接窗口/Renderer；3,000 次随机编辑、4 个快照读取线程、事务/位置/表格/IO/Markdown 回写 |
| 实际 DLL | 同一核心用例链接主 DLL 后通过，验证公开导出和运行时集成 |
| CommonMark 官方语料 | 652 项解析、原文保持、原生序列化重载通过；**不是 HTML 输出规范一致性测试** |
| LIVE 来源光标 | 652 项语料、15,470 个有效 UTF-8 边界通过；40 项使用明确报告的整篇源码回退，不能算作完整混合排版覆盖 |
| Renderer/View | 富文本/MD、嵌套单元格、宽窄多视图、绘制、命中、局部更新、10,000 段惰性布局与容器嵌入检查 |
| Editor | Source/Visual/Live 交错编辑和共同撤销、选区还原、输入/marks/剪贴板、中文预编辑/取消/确认、只读和通用编辑协议 |
| 真实 XGE | 三栏实际 GPU 渲染；Source、Live 各三帧；每栏有文本像素，模式切换产生不同显示结果；PNG 已人工查看 |
| 基础回归 | context、widget、layout、input、text、edit_contract、rich_edit 通过；accessibility 为 62 passed / 0 failed |

逐点分配失败扫描分别覆盖：普通文档编辑 12 轮、Markdown 解析 316 轮、表格结构 89 轮、Markdown 结构 93 轮、语义全部替换 79 轮、源码全部替换 77 轮。轮数包含首个成功预算；失败分支检查无半提交和分配平衡，不能将这些数字写成对应数量的实际失败调用。

文件用例包括正常保存/打开、UTF-8 路径、覆盖保存、不可写入目标失败后 dirty 不变，以及保存旧快照成功时新版本仍然 dirty。

## 性能样本

以下是 core 的本机测量：4 次预热、64 次同一位置的单字节交替替换并提交，保留历史；使用 XRT 单调微秒时钟。不含 IO、UI 排版、GPU 或 IME，不代表所有命令和文档的 P95。

| 固定样本 | P50 | P95 | 最大值 |
| --- | ---: | ---: | ---: |
| 10,000 段富文本，末段局部修改 | 0.002 ms | 0.003 ms | 0.004 ms |
| 102,480 字节 Markdown，首标题局部修改 | 20.306 ms | 24.367 ms | 28.692 ms |

富文本一次局部提交分配 15 次，增加 1,360 字节存活内存；本次构造 10,000 段约 31 ms。Markdown 加载约 10 ms，一次编辑仍走全量解析，分配 169,602 次。表中两个固定样本达到设计提出的 8 ms / 50 ms 初始目标，但 1/10 MiB 异步响应、内存预算和完整场景性能验收仍缺失。

## 原生运行与证据

```bat
call examples\xui_document\build.bat
build\xui_document.exe
build\xui_document.exe --verify
```

左栏为富文本 Editor，中栏为 Markdown Editor，右栏为同一 Markdown Document 的只读预览。Source/Visual/Live MD 按钮切换中栏模式。

本地输出：

- `build/xge.dll`、`build/xge.lib`。
- `build/xui_document.exe`。
- `artifacts/xui-document-rebuild/validation.log`。
- `artifacts/xui-document-rebuild/native-smoke.png`、`native-live-smoke.png`。

`build/` 和 `artifacts/` 被仓库忽略；本文件和测试脚本保存可提交的结果摘要及复现方式。原生截图验证了绘制，不能代替真实 Windows IME、物理键盘和读屏验收。

## 最终发布前的缺项

| 缺项 | 当前边界与需要补齐的内容 |
| --- | --- |
| 完整 Markdown 来源层 | 当前为语义范围及原文存储；需补完整来源/CST、实体与引用的精确映射，消除 LIVE 的 40 项回退 |
| 完整视觉结构编辑 | 顶层段落/标题及有限文本/marks 已用；嵌套列表、引用、表格、图片、脚注及复杂 delimiter 的通用回写仍缺失 |
| 解析调度 | 当前同步全量解析；需增量差分验证、异步 prepare、候选代次/取消/flush 协调、流式 UTF-8 追加缓冲 |
| 原生排版 | 当前按样式 run shaping，需完整段落 shaping/Bidi、精确范围几何、缓存预算、主题/DPI/资源变化锚点 |
| 完整编辑命令 | 需块/对象命令、矩形表格选择/矩阵粘贴/列宽拖动、自动连续键入合并、原生片段/HTML/图片剪贴板 |
| 资源和高级显示 | 当前只有 provider 契约；图片资源、公式排版、Mermaid 图形和 HTML 交互均未交付默认完整实现；通用 WebView 是独立前置依赖 |
| 可访问性 | 目前基础文本框适配；表格、链接、对象语义及真实读屏验证仍缺失 |
| 宿主与迁移 | 普通容器自动高度已测；MessageList 的高度、跨消息选择、滚动和链接集成未完成；旧 API/调用方/样式/行为测试尚未迁移移除 |
| 发布矩阵 | 需真实平台 IME、多 DPI/主题、其他平台及大文档验收；当前结果只覆盖本机 Windows x64 |

以上缺项是原方案的未完成验收项，没有因为基础测试通过而从交付范围中删除。
