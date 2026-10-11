# Markdown 删除后的空白保留

日期：2026-10-09。此项是首版常用编辑的正确性补充，不恢复HTML/CSS、网络动态资源、完整UIA或高级规模矩阵。

当前平台接续：同一产品源码已在Linux正常137全新/零复用构建后通过永久基础入口，删除594组合和8组分配预算扫描/折叠光标全部通过（240,512断言；少一次Windows专有模块路径检查）。见[Linux复验](XUI_DOCUMENT_LINUX_BASIC.md)。下文v1012—v1022未包含Linux的边界仍是原批次事实；物理IME、原生GPU和其他平台验收没有据此完成。

## 原问题与修复

对v1011接受运行库的12个公开可视Cut探针，有10个返回`XUI_DOC_ERROR_UNREPRESENTABLE`，原文保持不变。除了段首ASCII空格，删除还会暴露制表符、段尾空格和格式分隔符旁的Unicode空白；它们可能被Markdown解析为缩进、被裁掉或改变分隔符的结合规则。

| 原文与删除 | 正确保存的源码 | 保留的语义 |
| --- | --- | --- |
| `keep\ttail`删除`keep` | `&#9;tail` | 前导TAB与`tail` |
| `head keep`删除`keep` | `head&#32;` | `head`后的空格 |
| `**head keep**`删除`keep` | `**head&#32;**` | 原加粗与段尾空格 |
| 加粗文字中删除U+00A0前的`keep` | `**&#160;tail**` | 加粗、不可断空格与`tail` |
| `[head keep](url)`删除`keep` | `[head ](url)` | 链接及允许保留的普通空格 |

源码范围规划器现在处理删除两侧的完整UTF-8标量。只在首次私有候选解析不符合完整语义意图时，检查相邻标量的语义/来源映射、精确字节和空白分类，再将确实需要保护的边界写成实体。CR/LF仍是结构行尾，Code文字保留现有字面处理。二次解析必须再次通过完整语义与引用校验；未通过不发布。

修正保持已经计划的引用键补丁，不再为边界重建一个丢弃其他补丁的候选。带格式的隐式引用标签改变后，仍保留原定义键和未编辑定义的拼写。产品只修改`src/xui_document_markdown_edit.c`，使用同一Document、CST、事务与历史；公开API、代理ABI和文件格式未变。

## 永久回归与光标

```bat
call test_xui\build_document_boundary_delete_test.bat
call test_xui\build_document_basic_suite.bat --with-webview
```

删除入口现在覆盖33种源码、CommonMark/GFM/Extended三方言、可视Cut/TxnReplaceRange/TxnReplaceText三入口、正反选择，共594组。包含段首/段尾空白、TAB、U+00A0/U+3000、段中带格式文字、链接与显式/隐式引用键、Code、Quote/List/Heading、BOM/CRLF及已有实体。核对精确源码与字面语义、保留快照、保存点、Undo/Redo、模式切换和Cut后的折叠光标位置。

光标黄金比较原选区起点之前的完整语义前缀与删除后光标之前的语义前缀，兼容实体形成独立Text节点的情况。四种代表性编辑（ASCII段首、段尾、Unicode格式边界、带格式隐式引用）各扫描两个事务入口到首次成功。预算点分别为134/134、139/139、142/142、223/223，共1268次OOM失败、8次成功；失败保持revision、保存点、历史、通知、源码和快照，释放后无遗留分配。最终入口含分配器检查共240,513次断言。

## 实际证据

证据根目录为`build/source-document-footnote-local-reference-20261006`。

| 记录 | 实际结果 |
| --- | --- |
| `document-whitespace-delete-negative-v1012/proof.json` | 原接受库编译0/运行1，12探针中10失败，1712接受输入未变；冻结原库与原探针 |
| `document-whitespace-delete-fix-v1013/proof.json` | 正常构建实际0，1个Markdown单元重编译、136复用 |
| `document-whitespace-delete-positive-v1014/proof.json` | 同一12探针对新库编译0/运行0，失败为零 |
| `document-whitespace-delete-permanent-v1017/proof.json` | 永久33源码/594组合及8组分配预算扫描实际0 |
| `document-whitespace-basic-baseline-v1018/proof.json` | 当前全部18基础入口、3原控件回归及文件/关闭内部18条命令实际0，1712输入及主库/导入库/配置前后保持 |
| `document-whitespace-caret-permanent-v1020/proof.json` | 测试扩展误用节点内偏移推断整段光标，实际1；原夹具、可执行文件、库与日志保留 |
| `document-whitespace-caret-permanent-v1021/proof.json` | 仅修测试为完整语义前缀黄金，240,513断言实际0；主运行库未改动 |
| `document-v1-v1022-whitespace-acceptance.json` | 最终1713输入、实际日志、构建报告、运行产物与图片独立核对并冻结 |

完整基础验收继续覆盖现有Core/Editor、652样本、renderer、格式命令、剪贴板32场景、通用范围、MessageList、示例与四个原生检查、文件/关闭和298公开Document导出。Windows基础WebView重编译同一单元、136复用后通过，加载器仍自动打包，14内部浏览器符号未公开。本次不描述为新的137全新构建；测试新增光标断言后只重跑受影响永久入口，没有再次重跑未变化的全套产品。

首版可继续接入试用。真实物理IME、人工键鼠与真实系统剪贴板成功交互、其他机器/平台的新改动复验仍待完成。高级冻结范围保持，整体设计目标尚未完成。
