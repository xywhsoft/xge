# 深层空引用的跨块删除

更新：2026-10-08。新独立候选v335在v319之上修复一处来源前缀长度限制；v341接入永久内核测试，v355接入已实际通过的永久Editor及匹配运行库，产品源码均相同。尚未合入，完整K2和长期目标仍为active。

后续发现另一处深层列表段落前缀截断：v374在v355之上修改同一写入器，v382接入永久768/16与Editor432，默认62/公开42、计划永久Editor3306。两端完整768工作归档已实际0；新四配置内核、运行库及整体交付需独立验收。本页v355与v360/361/362证据保留原产品范围，不用于交付新修复。详见[深层列表来源前缀](XUI_DOCUMENT_NESTED_LIST_PREFIX_CST.md)。

## 已确认的产品问题

从根段落内部删除到列表内引用段落的末尾后，引用中未选中的隐藏链接定义仍需保留。统一语义事务保留列表及空引用结构，来源编辑必须保留其原标记、缩进、定义拼写和未编辑尾部。

`doc_md_nested_empty_quote_scaffold`原先使用256字节前缀数组；达到这个长度便跳过保留空引用的来源路径。后续重解析不能证明目标结构时，公开命令返回`XUI_DOC_ERROR_UNREPRESENTABLE`。这不是无法解析的输入：每个输入都先验证精确TEXT映射、引用层数和List/ListItem祖先；独立预期文档也验证完整空引用结构。

v319两端实际默认共享库在480个有效场景中各拒绝120个：宽缩进引用64/65/87/95/120层，每层24个LF/CRLF/CR、BOM、EOF、正反选区变体；第一条失败前缀为258字节，63层的254字节前缀成功。其余360个场景来源符合独立黄金。所有被拒绝场景仍保持版本、内容、保存点和历史原子。

## 测试预期修正

最初v331预期在删除选区内部多保留了一条空行，将360个成功场景误报为来源差异。v333仅将该已选中边界改为合并后的单个行结束符；输入、范围端点、未选中标记与定义字节、所有结构/重解析/历史断言及480个变体不变。原探针和原失败保留。修正后的两端实际程序均完整运行480个场景，结果为120拒绝、零来源差异，不能将最初360个差异当作产品破坏来源的证据。

## 独立修复

v335保持1629项源码与脚本输入，与父v319仅一个写入器不同。原栈数组改为既有受预算与溢出检查约束的动态来源输出；沿用原标记验证规则，错误和取消继续传播，所有出口释放临时存储。确认结构后通过既有分块来源读取保留原前缀字节，没有提高固定数组上限。

完整候选重解析、隐藏定义原拼写保护、SourceStore、统一事务和共同历史不变。父v319及其整库原54872、旧v289整库原21736和主1571项基线均保持不变。

## 当前实际验证

- 两端正常工作归档均实际退出0，每端12条归档/编译/运行检查；新480原字节、作用域、语义、完整重解析、快照、光标、Undo/Redo和零泄漏通过。
- 未修改的完整行内范围删除、单节点删除和容器前缀驱动在两端通过，原黄金及故障断言保留；Linux使用ASan/UBSan。
- 工作归档来自已验证的全新29单元普通内核，仅替换新写入器对象，实际复用28个归档成员；不视作全新内核或新共享库，也不视作碰撞配置验收。
- 新16组内核/8组公开故障用例保留原逐点OOM、合作取消、失败重试、旧快照、稳定尾部、共同历史及零泄漏断言，覆盖宽缩进63/64/95/120层、单步及复合事务。Windows工作归档原29304及Linux原2220/工具92cb3a均实际退出0，每端16组、35,720故障点通过。这个终态仍复用28归档成员，不标作全新内核。
- 永久v341新增黄金、共同辅助头、故障驱动与全新构建入口四文件，1633项输入；产品与v335相同。原480黄金主体和16/8故障断言不变，默认CST由58增至60、Windows公开由38增至40，原驱动及顺序保留。尚未合入。
- 新全新内核Windows原90472已实际退出0，两配置各29新单元/零复用、480黄金、16组OOM/取消及三个未修改删除/前缀驱动全部通过，每配置41条实际编译/归档/产品命令。Linux原13324已通过普通480，继续ASan/UBSan故障扫描，随后碰撞配置；不标为四配置完成。
- 新Editor探针覆盖432个外部单步/复合事务、三模式观察和VISUAL Delete/Backspace/空替换/继续输入的共同历史场景。旧Windows实际共享库编译0/运行1，先通过162个场景，在64层宽前缀258字节的第163个外部删除场景复现拒绝。相同断言在两端新实际共享库均已完整通过。
- 两端新默认共享库原21092/36045均已实际退出0，每端136单元全新编译/零复用。原52187/51398各14条公开/Editor编译运行检查实际0，公开480及原600/深层240/宽前缀528、新Editor432及原840通过。原52780/87626各两条公开8组OOM编译/运行检查实际0；Linux默认库的这些检查不标为sanitizer或真实IME。
- 新永久Editor原Windows直接工具1a8fd9及Linux原58191均实际退出0，每端两条编译/运行检查、432场景，原Editor C主体及全部断言不变。独立v355接入两文件及Windows整库调用，1635输入、78项变化，默认60/公开40、永久Editor总数1602+168+672+432=2874，五个匹配实际运行库文件已复制；仍未合入或完成60/2874整库执行。
- 新内部Web原38786已实际退出0，测试/生产两配置各136全新单元/零复用，15项实际检查及Loader打包、生产导出、HTML交互通过；公式、Mermaid、HTML三张500×350当前图像已逐张查看并接受，与先前已查看图像字节一致。使用同一产品的独立物理副本，避免与Linux默认构建共用HarfBuzz对象目录。
- 同一产品的新原生/Renderer/Editor/sanitizer26批次原88391/工具e842e7已实际0；32张当前实际图像与此前逐项查看的同平台图像字节一致并接受，v354 acceptance已写入。Linux窗口仍使用Xvfb/Mesa，Editor广域交互仍有代理入口，不标为物理IME/读屏。
- 新v359完整仓库17根已取得当前实际UI/Web/公开故障前置、冻结并启动原41783，官方Bidi根实际0，Windows完整套件继续。原16根全部保留，新增Linux永久Editor432；强制核对全部60个普通/碰撞CST、Windows40公开及两端2874个永久Editor场景和本批新原生图像。尚无整批终态或实际合入。

完整四配置全新内核、完整60/40/2874仓库及其新原生图像、实际合入及合入后验收仍需完成。父v319的已通过UI/Web和正在执行的整库回归不代表新修复验收；当前新共享库/Editor/UI/Web已经取得自身实际终态。

## 证据

最新78项及5运行库的v360合入计划已实际准备，保留备份/并发/回滚并保护六个原控制器身份；v361验收入口预检查0。合入后v362主工作区23根与Web15已实际准备，含两端136全新/零复用、各2874永久Editor及原生消费者。实际合入、冻结和执行仍未完成；缺少验收或合入时的两个入口均按预期拒绝，没有修改主1571或运行库。

证据目录：`build/source-document-footnote-local-reference-20261006/`。

- 修正探针与两端实际负例：`document-deep-quote-join-v333-preparation.json`、`document-deep-quote-join-v334-windows-observation.json`和`document-deep-quote-join-v334-linux-observation.json`。
- 新产品输入：`document-deep-quote-scaffold-v335-inputs.json`。
- 两端工作归档限定终态：`document-deep-quote-scoped-v340-observation.json`及`document-deep-quote-work-v336-*-observation.json`。
- 逐点故障用例：`document-deep-quote-fault-v338-inputs.json`；当前过程：`document-deep-quote-fault-v339-*-current.json`。
- Windows工作归档16组终态：`document-deep-quote-fault-v339-windows-observation.json`，原29304/工具f815d9实际0。
- Linux工作归档16组终态：`document-deep-quote-fault-v339-linux-observation.json`，原2220/工具92cb3a实际0；两端35,720点与最新78/5合入、23/Web15准备限定观察：`document-delivery-preparation-v363-observation.json`。
- 永久入口和两端全新普通480限定观察：`document-deep-quote-permanent-v341-inputs.json`、`document-deep-quote-progress-v347-observation.json`；本批过程：`document-deep-quote-kernel-v342-*-current.json`。
- 旧实际Editor负例：`document-deep-quote-editor-baseline-v346-observation.json`，原工具1c8091实际1；新两端实际默认构建及消费者：`document-deep-quote-runtime-v350-observation.json`。
- 当前整合与限定公开/永久Editor/Web接受：`document-deep-quote-repository-v355-inputs.json`、`document-deep-quote-scoped-v357-observation.json`。
- 当前Web15及三图接受：`document-deep-quote-web-v349-visual-acceptance.json`，原38786/工具05b3b2实际0。
- 当前Windows两配置内核终态：`document-deep-quote-kernel-v342-windows-terminal.json`，原90472/工具d4d759实际0；Linux过程为对应current文件，原13324。
- 当前UI26/32图接受：`document-deep-quote-ui-v354-acceptance.json`，原88391/工具e842e7实际0；当前整库冻结：`document-deep-quote-repository-v359-inputs.json`，原41783执行中。

整体范围以[剩余任务](XUI_DOCUMENT_REMAINING.md)为准。
