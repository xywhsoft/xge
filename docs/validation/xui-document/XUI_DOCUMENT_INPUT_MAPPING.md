# XUI Document 未发布输入的位置映射

更新：2026-10-09。内核协议和当前运行库已专项验收；View/Editor的Prepare文本范围接线仍待实现，产品候选未合入。

## 接口与位置域

`xuiDocumentPrepareMapSourcePosition(from,to,position,output,mapping)`只接受SOURCE位置，至少一个Prepare句柄非NULL。NULL表示该链共同的已提交base，非NULL表示该句柄的不可变输入。输入位置必须同时匹配Document identity、base revision、generation、root节点、UTF-8字节边界和affinity。输出可与输入别名，失败保持输出位置及mapping不变。

支持base与候选互转、同链跳代、逆向、同一generation及零操作generation。删除区内部映射返回DELETED；边界遵循已有BEFORE/AFTER规则。重新开始的输入、另一文档、另一base和独立预览分叉返回STALE，即使两份源码字节相同。EXACT表示按补丁/亲和性规则精确换算，不代表被替换文字可逆恢复。

## 存储、线程和发布

源码和累计操作日志仍使用已有不可变sequence。追加的链元数据为generation、操作前缀长度、源码长度，不增加第二套文档或历史；祖先定位二分搜索链记录，然后按顺序或逆序应用相关操作。查询没有分配，时间随跨越操作数增长，尚无固定机器时延验收。

调用方保留Prepare句柄期间，取消、被替代、owner释放和实际worker RUNNING不使这些不可变输入失效；查询不读取worker正在修改的draft/展平ops。

`xui_doc_change_info_t.iInputGeneration`仅标识Prepare发布的精确输入generation，在通知回调前已写入。对该generation的SOURCE位置调用`xuiDocumentMapPosition(change,...)`，直接保留其候选字节偏移、清除generation并推进revision，避免重复应用源码补丁。其他generation返回STALE；普通写入、Undo和Redo该字段为0。无操作发布不生成ChangeSet且revision不变，使用Prepare到NULL映射回base。

本次公共结构ABI改变，无旧API/ABI兼容包装；使用新的匹配运行库和头文件。

## 验证与交付边界

最新来源：`E:\codex-builds\xge-document-input-mapping-integrated-20261009-v847-01a09446`。专项接受记录：`build/source-document-footnote-local-reference-20261006/document-input-mapping-v850-paired-acceptance.json`。

- 两端普通/碰撞各28个Document源单元加XRT，共29单元全新编译、零复用；新映射与原完整Core通过。Linux保留ASan/UBSan与detect_leaks=1。
- 批量、跳代、正逆向、删除、UTF-8、affinity、别名、原子失败、重启/取消、流式半码点和独立预览分叉均有字面黄金。
- 初次创建12个、继续输入13个分配失败点逐点扫描；旧pending、revision和live内存不变，重试、无查询分配与零泄漏通过。
- 10MiB输入继续2048次，保留首段和中段句柄，新增保留内存1569184字节；worker实际阻塞在RUNNING时5000次查询无分配。
- 两端137正式单元全新/零复用，两端公开映射和完整Core、完整renderer、四配置范围、Windows基础UIA、原View sanitizer及原Windows范围bat通过，五份运行依赖匹配。
- 新测试接入永久CST入口：73私有、Windows53公开驱动。完整永久入口尚未对本候选取得终态。

原旧DLL负例编译成功、运行按指定位置断言退出1。控制器计数、收集器计数、夹具输出目录及对象目录校验失败记录保留；没有放宽产品断言或删除既有测试。

本次没有启用Prepare界面文本范围，也未完成一般语义候选位置、完整UIA TextPattern、全仓库、固定性能或平台矩阵。所有11个工作包仍有余项。
