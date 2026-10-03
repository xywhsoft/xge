# Document 与共享文本的 Unicode 17 行边界

更新：2026-10-02。整体重构仍进行中。本批属于 R1/K1 文本正确性；统一 Document、Rich 和 Markdown 使用相同行边界入口。通用 WebView 仍只承诺 Windows 基础网页承载；Document 的公式、Mermaid 和 HTML 私有渲染通道保留。

## 实现

将私有 libunibreak 从 7.0 更新到固定的 8.0 发布提交，保留原始许可证和代码，只规范化行尾。新增 General_Category 辅助表，与 line/EAW/emoji 数据一同更新到 Unicode 17。固定提交、归档 SHA 与语料 SHA 见 `lib/libunibreak/README.xui.md` 和 `test_xui/data/unicode-17/README.md`。

上游移除了增量公开 API。XUI 没有补兼容包装；适配器和硬换行扫描共同调用新的默认语言、strict LB1 分类入口。原始 UTF-8、每个非法字节替换为 U+FFFD 的分析策略、逻辑 byte offset、SHY 的显示投影、Unicode 17 字素交集与 XRT 所有权保持原合同。public ABI16、Document 原生格式7不变。

算法在调用方输出缓冲区上工作，状态固定大小，无堆分配；新 lookahead 规则最多保存待回填的位置，不重复扫描文本。XUI 的 2B 临时边界工作缓冲、B+1保留边界图及 WORD 的显式应急切分策略保持不变。

## 可观察变化

- Unicode 17 的 AK/AP/AS/VF/VI/HH 类及正字法音节规则已接入；SA 中 Mn/Mc 标记按 CM 处理。
- 旧 LB30b 的 `U+1F02C U+1F3FF` 已恢复禁止断行，不再接受已知偏差。
- 单词起始 HY/HH 不能单独成为一行；`-ab` 在10像素宽、每 glyph 10像素的确定字体下保留首个 `-a` 单元，允许它溢出，再换行显示 `b`。
- 希伯来 LB21a 保护 HY/HH，不再把 BA 软连字符一起禁止。可容纳的终端 glyph/连字符候选会使用断点，宽终端仍被拒绝。SHY 后有空格时可在空格后断行，SHY 不显示；独立40像素字体中首行末端 caret 为保留空格的10像素。

## 验证

官方文件从 Unicode 网站获取，与固定发布自带语料 SHA 完全相同。隔离旧引擎在相同19,338条语料上有1,026条不匹配，新引擎无不匹配；逐条检查解码恰好为码点数加一及输出前后哨兵。真实 XUI 测试检查19,338条全部 UTF-8 内部字节、766条字素记录、完整 line/grapheme 交集，以及官方 Line_Break 文件的1,114,112码点属性。

17种规模输入从256增长到65,536次重复，覆盖最长1,048,576 clusters、长组合/RI/ZWJ、引号、数字回填、AK/AP/VI/VF、起始连字符和强制换行。测试约束实际解码、映射、断行和二分探测次数，包含9个真实边界构建分配失败与可重试、非法 UTF-8、内嵌NUL和八类硬换行。Windows GCC 与 Linux ASan/UBSan 的同一共享文本测试已通过。

新增自有确定字体和字面 PUA glyph 参照，核对360个完整RGBA：Rich整段/分片、Markdown VISUAL/SOURCE/LIVE、六条缓存/基础范围/仅drawText/无paint路径、文字位置、caret/hit、改宽与字体失效、Document释放后的Snapshot。旧封存DLL在相同程序上实际退出1。已有SHY测试保持字面glyph、advance、候选接受/拒绝与全部像素检查，仅按新规则更新希伯来断点预期。

完整 Windows 发布套件（含652项CommonMark、Editor/MessageList/原生示例）、真实无HB九程序、两条Document分配失败扫描、Linux共享文本/Renderer/View/Editor/实际C字体回退的ASan/UBSan，以及可选WebView五项回归均退出0。13个共享文本/Label/MessageList/调度/Context基础程序通过；过时投影夹具改为本地基本能力提供者后，保留11个构造失败与9个DPI恢复点。生产14个内部浏览器/请求符号不导出，三张公式/Mermaid/HTML截图逐张检查。默认、Document-local和隔离Native DLL一致为 `d6bb51e0af3910a1ebb0f320078ba27a10f0380f024156ca07f42066a95ca1ac`；37个运行时源码指纹全过程不变。最终真实退出码、失败记录和指纹写入独立前缀 `artifacts/xui-document-rebuild/validation-2026-10-02-line17-`。旧批次日志、DLL和清单不重写。

## 剩余边界

这里保证共享行算法按默认/strict的Unicode17官方语料。WORD应急切分和CHAR是显式产品策略，不把它们宣称为未裁剪的UAX14输出。仍无字典型泰文/老挝文/高棉文分词或自动语言连字符；语言定制、PS编辑规则、其余控制投影、任意跨字体GSUB、复杂shaping/caret、完整CST、结构命令、预算性能与实际多平台验收继续按整体设计推进。
