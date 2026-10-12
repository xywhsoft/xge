# 嵌套图片 alt 投影与 CST 命令归属

核对日期：2026-10-05。正式源码、DLL、Windows/Linux 和浏览器验收通过；整体重构目标 active，原 13 个工作包保持。

## 实际问题与独立证据

CommonMark 0.31.2 第 574 例 `![foo ![bar](/url)](/url2)` 的预期是一张图片，alt 为 `foo bar`。当前文字归一化 v6 的实际 DLL 通过公共 Snapshot API 返回两张图片：外层 alt 为 `foo `，内层 alt 为 `bar`。这不是 HTML 输出格式或转义写法的差异。

新增独立语义检查直接读取规范语料的预期 HTML，提取代码、行内内容、格式、链接、图片 alt、硬换行及分隔线，再与公共 Document 快照比较。578 例中只有上述嵌套图片例存在语义差异；72 个含原始 HTML 的例子和两个需要额外链接图片元数据的例子明确排除。普通非代码空白及块结构未由此检查覆盖，不能宣称完整 CommonMark HTML 一致。另一项精确代码内容检查覆盖 580 例，没有差异，另 72 例排除。

检查器初版把缺省 title 与空 title、URI 中方括号与百分号编码当作差异，已保留初版记录并修正比较规则；这些比较器差异不作为产品缺陷。URI 比较保留百分号编码和保留字符，不把 `%2F` 与 `/` 混为一谈。

候选第一版修正渲染树后，继续发现 `![![bar](../../in)](out)` 和嵌套空 alt 的图片命令仍有来源归属歧义。修改资源会走整段重写路径，将操作范围外的 `__outside__` 改成 `**outside**`。该失败及对应输入已归档。

## 实现

候选位于 `build/nested-image-alt-20261005`，以已验证、尚未合入的多行空白 v3 的 188 项输入为基线。

- 解析投影保留最外层图片对象。嵌套图片的文本、实体、转义、格式及代码回调继续汇入外层 alt，不生成额外图片，不覆盖外层资源、标题或包围链接。
- 保留嵌套图片的完整行内语法和引用定义关系。语义对象被投影为一个，不删除原始 Markdown 的嵌套语法。
- 图片更新依据解析器记录的 CST 父子关系排除处于图片 alt 内的图片语法，绑定真正可见的外层图片。空 alt 来自嵌套空图片时也按此规则处理。
- 外层和内层属性的分配/借用关系保持明确。未增加公共 API、ABI 或持久格式版本。

## 已执行的验证

最终 v4 主驱动编译、运行均实际退出 0：

| 范围 | 结果 |
| --- | --- |
| 独立原字节黄金 | 3,024 场景：14 种结构，LF/CRLF/CR，普通/引用/列表，三种方言，原生往返，Load/SOURCE/Prepare/Continue |
| 图片结构命令 | 144 次 alt/资源修改，含包围链接、引用/列表、原生前缀和三种方言；周围原字节保持，Undo/Redo 身份恢复 |
| 边界差分 | 10,884 场景，其中 10,818 次增量解析；删除、Tab、反引号、二进制 NUL，与完整重解析及历史比较 |
| 失败与取消 | 2,861 个 SOURCE/Prepare OOM、1,392 个 Prepare 取消点；内容、版本、身份、历史、保存点和分配平衡保持 |
| 独立规范语义 | 最终 v4 新编译的公共 Snapshot 驱动读取 652 个二进制输入；578 例实际比较，零差异，74 例明确排除 |
| 精确负例 | 最终 v4 驱动先在 v6 实际 DLL 上编译 0/运行 1；加入共用配方后，精确 190 项最终输入在新多行空白正式 DLL 上再次编译 0/运行 1，首先报出应一张却有两张图片 |

普通/碰撞 sanitizer、Core、多行空白、文字来源、图片 alt 来源回归，以及两个模块静态检查均已通过。候选主驱动 2 条、上述检查 14 条和最终独立语义驱动 2 条，共 18 条全新命令实际退出 0；197 项实际验证输入已归档。190 项最终输入/6 项拟合入已冻结，共用 suite/sanitizer 和新批处理入口属于验证后补齐的未使用构建配方，须在正式合入后执行。原完整 CST 与其他工作包继续，正式合入后的 DLL/Windows/Linux/浏览器验收尚未完成。

证据入口：

- `build/document-semantic-oracle-20261005/expected-inline-v3-report.json` 与 `expected-inline-v3-failures.json`。
- `build/nested-image-alt-20261005/candidate-v4-exits.json`、`candidate-v4.log`、`expected-inline-v4-report.json`。
- `build/nested-image-alt-20261005/candidate-accepted-v4.json`、`candidate-final-v4-frozen.json`，以及 `before-final-v5-exits.json`、`before-final-v5-inputs.json`。
- `build/nested-image-alt-20261005/before-final-v4-exits.json`、`before-final-v4-inputs.json`、`before-final-v4.log`。
- `build/nested-image-alt-20261005/archived-v1` 保留整段重写失败的输入；`archived-v2`、`archived-v3` 保留后续版本。

## 剩余边界

该修复解决图片投影与图片命令的 CST 归属，不代表完整 CST、所有对象的 alt 投影、所有结构命令或完整 HTML 输出一致。原始 HTML、其他高级对象、复杂无损编辑、三模式、排版、真实平台/IME/读屏/DPI 和固定机器性能仍按整体设计推进。

文字归一化 v6 已完成 22 条完整命令验收，多行空白 v3 已完成 21 条完整命令验收。嵌套图片 v4 已合入 6 项输入，190 项正式输入保持冻结；实际 DLL 25 条、两个模块分析、两种 Web 构建和串行 Web/生产边界 13 条全部实际退出 0，公式、Mermaid、HTML 三张实际截图已查看。Windows 完整套件和 Linux Core/Renderer/View 原进程继续运行。通用 WebView 仍仅要求 Windows 基础功能；其他四平台后端及通用通信暂缓，Document 内部渲染保留。

嵌套图片 alt 本批已正式落地：统一 Document 投影为一个外层图片，保留内部语法和引用，并按 CST 父子关系绑定图片命令。3,024 黄金、144 图片命令、10,884 差分（10,818 增量）、2,861 OOM/1,392 取消点及独立规范语义 578 例零差异通过；实际 DLL 25 条，Windows/Linux、静态检查、两种 Web 构建和串行浏览器/生产边界共 22 条完整命令实际退出 0，190 项正式输入不变，三张实际截图已核对。完整 CST、其他图片对象/字段无损编辑、模式、渲染、迁移和真实平台验收继续，原 13 包和整体目标 active。
