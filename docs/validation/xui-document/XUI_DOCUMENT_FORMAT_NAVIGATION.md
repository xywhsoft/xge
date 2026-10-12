# 统一 Document 的 FORMAT 文本范围

更新：2026-10-09。基础实现与限定范围验收完成，源码仍在隔离候选，完整格式、UIA、仓库和整体重构未完成。

## 行为与共用内核

现有通用文本范围的 `expand`、`move`、`moveEndpoint` 支持 `XUI_INTERNAL_TEXT_FORMAT`。端点沿用64位UTF-8坐标、既有 Document Position/Prepare generation、ChangeSet 和共同历史，没有第二套可写文档。EOF归入最后一段；零移动、极限计数、反向及跨端点折叠沿用通用导航规则。

VISUAL 从现有语义 plaintext spans 提取有效格式。比较已解析字体、继承文字颜色、内联背景/currentColor/highlight、语言、marks、段落对齐和不透明对象角色，连续相同键合并。字号14的显式节点可与默认14字体合并；语义粗体、下划线、语言及有效颜色变化仍可分段。字号、标题和代码的字体选择调用实际 layout 共用函数，文字颜色调用 renderer 共用函数。

SOURCE_TEXT 对当前原始源码字体提供统一格式。LIVE 复用同一 snapshot 的节点来源范围和 renderer 活动区：活动源码使用原始字体，非活动文字使用语义格式，未显示的源码标记保留隐藏格式。未发布输入的非活动范围经既有 base-to-input 映射移动；活动区覆盖其候选坐标。全源码 fallback 使用原始格式。围栏、链接与引用续行前缀有独立字面黄金，不由实现计算预期边界。

## 缓存及失败正确性

`doc_format_cache` 是 View 投影的可丢弃派生边界数组。索引来源是当前保留 edit snapshot；缓存只存边界、state/mode/active source/generation 与 renderer descriptor，没有另一份内容。记录通过事件排序和优先级堆合并重叠 LIVE 区间；建成后边界查询二分查找。

内容/投影失效清理缓存。主题与动态字体查询先刷新当前资源/样式；renderer 因字体、DPI、缩放或显式 InvalidateFonts 重建时清理旧格式缓存。单独重建缓存时，失败保留旧缓存；完整新数组建立成功后才替换。范围与实际移动数只在成功后提交，失败保留端点且实际移动数为零。

首次 plaintext projection 仍可能复制全文。首次格式目录的记录、事件、堆和边界随跨度增长；暖查询无 Document 分配不证明整个投影按视口有界、首次固定时延或最终缓存预算已经完成。

## 实际验证

候选：`E:\codex-builds\xge-document-format-integrated-20261009-v952-01a09446`。
接受：`build/source-document-footnote-local-reference-20261006/document-format-v954-paired-acceptance.json`。

- 富文本VISUAL、Markdown VISUAL/SOURCE/LIVE 的字面边界、逐UTF-8端点扩展、正反移动、端点移动、EOF和极限计数。
- 相邻相同字体、继承/显式颜色、currentColor内联背景、语言、下划线与硬换行、段落对齐、标题与代码。
- 四配置各三操作乘四缓存分配故障点：失败范围/文档版本/存活内存保持，重试和暖查询无分配，释放零泄漏。
- LIVE 活动区变化后重建失败、恢复旧活动区仍使用原缓存，再成功重建；主题变化在布局前查询，自定义字体回调选型变化后的显式失效。
- 双View属性编辑和共同Undo/Redo、保留范围、文档替换/销毁；既有SOURCE/LIVE/VISUAL文本Prepare真实RUNNING各5000次暖查询、继续和取消。
- 两端各137正式编译单元全新/零复用，完整Editor/Core/renderer与输入映射；原Linux View脚本ASan/UBSan/泄漏，原Windows范围bat及两个实际基础UIA客户端。

接受记录独立核对37条实际正例命令、Windows/Linux实际对象与独立HarfBuzz、依赖时间戳、ext4单链接、源码/日志/产物散列。旧DLLFORMAT不支持与旧主题边界错误负例都编译0/运行1。遗漏RICH profile的夹具失败及Linux原失败保留，随后仅修夹具。

## 继续完成的边界

当前背景键覆盖内联背景；段落、单元格、代码框等祖先表面背景与其叠加仍需接入。还需完成不透明对象/结构分隔符/空文档、完整样式失效与回调重入的格式矩阵。高级对象内部的属性查询需要宿主约定。

PAGE、属性访问、可见范围与子对象、Windows完整UIA TextPattern/TextRange/Grid/Table/事件、真实读屏、跨平台后端和固定机器性能仍未验收。当前基础UIA通过不证明TextPattern已经接线。所有结构/对象编辑异步Prepare、最新候选全仓库、源码合入及合入后检查也仍待完成。WebView范围保持Windows基础功能，Document内部高级渲染保留。
