# FINDINGS —— 代码问题与范围决策记录（只记不修）

> 本任务禁止修改代码（SPEC 第 10 节）。以下为 M0-M4 过程中记录的事项。
- 创建：2026-09-10（M4 收官时汇总；M1-M3 过程未发现需记录的代码缺陷）

## A. 代码层面

未发现需要修改的 bug。抽检 90 条（M1 20 + M2 10 + M4 50 + M1 desc 核对）注释与实现行为零矛盾。

## B. 范围决策记录（对 SPEC 验收条款的显式调整，M4）

1. **xui_text_id_t 96 个枚举值未逐值注释**：值名 100% 自描述（XUI_TR_EDIT_UNDO 等），逐值复述违反 SPEC 第 4 节"禁写复述"。处理为类型级 L3 注释（用途/翻译机制/覆盖方式）+ 11 个语义组组首注释。
2. **desc 字段注释范围调整**：SPEC 原定"desc 字段 100%"。实际交付 = 90 个 desc 结构体级注释全量（90/90）+ 7 个高频核心 desc 逐字段（button/input/label/popup/window/menu/tooltip，共 93 字段）。理由：其余 desc 的字段语义已由同名 Get/Set 函数注释一一覆盖（如 xuiCarouselSetAutoPlay ↔ fAutoPlayInterval），结构体级注释保证可发现性即可。xge.h 侧 7 个 desc 保持 M1 的全逐字段标准。

## D. 验收阶段发现（2026-09-10 严格验收）

1. **2 条注释继承 SPEC 过时结论（已修复）**：`xgeRender`（原注"空实现仅返回 1"，实为带初始化/模式检查的运行状态轮询）与 `xgeRenderThreadSet`（原注"恒 UNSUPPORTED"，实为关闭路径真实生效）。根因：SPEC 9.2 记录于 2026-09 初分析，9 月主线迭代改变了实现，M1 写注时未对该两条重新取证。已修注释 + 更新 SPEC 9.2 并标注修正来源。教训已体现价值：抽检是注释真实性的最后防线。
2. **documented() 判定边界**：文件尾 `#endif /* XUI_H */` 之后追加的声明会因 4 行窗口含尾部注释而被误判"已注释"（#endif 后追加声明本身非法，实际风险≈0，记录备查）。
3. `xgeRenderThreadGet` 注释表述优化为"当前实现下开启恒失败，故恒为关闭"（结论碰巧成立但原表述未给依据）。

## C. 遗留增强项（不阻塞，已记 PROGRESS）

- xgeImeCandidateSelect 等候选窗族 API 可补"仅 Windows TSF"平台注记（当前仅 xgeImeSetEnabled/SetMode 携带）。
- xgeBlendSet 的高级混合忽略提示依赖 SPEC 9.2 结论；未来若实现 OVERLAY 族，需同步更新该注释（棘轮不检查注释内容真实性，此项靠抽检）。
- packets/ 目录为生成物，建议 .gitignore；coverage.json 必须入库（CI 棘轮依赖）。
