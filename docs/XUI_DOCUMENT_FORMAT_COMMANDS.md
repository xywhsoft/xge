# 基础格式命令的可用状态与 Markdown 回写

2026-10-09。本轮修复属于首个可用版本的常用编辑正确性，不恢复冻结的 HTML/CSS、网络动态资源、完整 UIA 或高级规模矩阵。

## 已修复的问题

1. 在代码块正文或图片 alt 字段内选择、放置光标时，Bold/Italic/Strike/Code 等命令曾显示可用，执行却返回不支持。现在 `QueryCommand`、`CanExecute`、`Execute` 与直接 `SetMarks` 使用相同的正文格式范围约束。正文 Text、SoftBreak、HardBreak 可参与格式修改；仅包含代码块正文或图片字段的范围禁用行内格式。空富文本根与可插入文本的段落间隙仍支持待输入格式。
2. GFM 的单词内删除线曾错误要求标记外侧为单词边界。例如对 `and` 中的 `a` 加删除线无法成功。解析器现在采用左右 flanking 规则，与 [GFM 官方实现](https://github.com/github/cmark-gfm/blob/master/extensions/strikethrough.c) 一致；支持相同长度的单/双波浪线，内部空白与标点仍受规则约束，三条及以上的行内波浪线保持字面文本。Extended 的单波浪线下标规则保留。
3. Markdown 格式回写的快捷路径曾直接修改正式事务，再做语义核对。现在先在私有候选中插入分隔符，重新解析并验证与目标语义一致后再采用；无法表达时进入已有的源码规划路径。例如选中普通文本 `a\`b` 加 Code，使用双反引号得到正确代码内容。失败仍使整个事务无法提交，不能留下部分修改。

修改集中在 `src/xui_document_editor.c`、`src/xui_document_markdown.c` 与 `lib/md4c/md4c.c`。统一 Document、来源保护与最终语义等价检查保留，没有增加公开 API、旧兼容接口或版本变更。

能力查询提供当前模式、方言、范围与只读状态的可用信息，不等同于完整试执行任意 Markdown 结构组合。宿主仍必须检查执行结果；无法无损表达的内容可保留原文并切换源码编辑。

## 永久回归与实际结果

从仓库根目录运行：

```bat
call test_xui\build_document_format_command_test.bat
call test_xui\build_document_basic_suite.bat --with-webview
```

`xui_document_format_command_test.c` 仅使用公开 Document/UI API 与既有测试代理，覆盖四种文档配置、字段选区与光标、只读、空根/段落插入、GFM 中文和标点边界、前后向选择、精确来源、Undo/Redo 与模式切换。该入口已加入基础套件。

新增两条小规模分配失败回归：单词内部 Strike 的直接回写，以及内容含反引号的 Code 候选回退。分别通过 109、219 个分配预算点（含首个成功点），共 326 次失败与两次成功。失败必须返回 OOM，版本、来源、保存点、历史和通知保持不变；成功只发布一次，Undo/Redo 与保留快照正确；释放全部对象后无遗留分配。上限每条 1024 点，不属于被冻结的完整复杂组合矩阵。

证据目录均位于 `build/source-document-footnote-local-reference-20261006`：

- `document-command-probe-v978`：旧运行库的九个可用状态/执行不一致；`document-command-positive-v980` 保留解析器修正前的最后一个删除线失败。
- `document-command-fix-v979/proof.json`：正常入口重编译两个修改单元、复用 135 个；保留原运行库副本。`document-command-parser-v981/build.log`：重编译解析适配单元、复用 136 个；原探针通过 133 次可执行命令，无不一致。
- `document-command-baseline-v985/proof.json`：当前主运行库的 16 个基础构建/验证入口全部实际退出 0，包括完整现有 Core/Editor、652 个 CommonMark 样本、renderer、MessageList、三模式文件与关闭流程、298 个公开 Document 导出以及 Windows 基础 WebView。1701 个输入与主 DLL/导入库/配置在运行前后保持不变。Windows WebView 产品本轮重编译三个修改单元、复用 134 个。
- `document-command-atomic-v987/proof.json`：仅扩展永久测试后的独立入口通过；运行库和产品源码与 v985 相同，不将本次独立检查记为重复运行整套。原 v986 测试编译失败保留，原因是把返回 void 的取消订阅接口当作有返回值；修正夹具后通过。
- `document-v1-v988-format-acceptance.json`：最终输入、日志、产品与图像的独立核对记录。

本轮是 Windows 主工作区验收。此前 Linux 构建与 sanitizer 证据仍属于此前接受的输入，不能当作本轮三个代码修改的 Linux 复验。真实物理 IME、人工键鼠、其他机器和全平台发布仍未验收；首版可试用不等于整体高级方案完成。
