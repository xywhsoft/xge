# XGE API 注释任务 · 阶段提示词（M0-M4 + 续跑）

> 用法：在新的（或当前的）会话中，直接复制对应提示词发送。
> 每条提示词自带恢复机制：强制先读 SPEC.md + PROGRESS.md，再执行。
> 提示词内容若与 SPEC.md 冲突，以 SPEC.md 为准并指出冲突。

---

## 通用恢复头（每条提示词已内置，此处仅作说明）

所有提示词都要求助手：① 完整读取 `D:\GIT\xge\api-docs\SPEC.md` 与 `PROGRESS.md`；② 以 SPEC 为唯一事实源，与记忆冲突时以 SPEC 为准、存疑数字当场重测；③ 只加注释不改代码、不 commit；④ 每批跑门禁（coverage + verify_batch）并更新 PROGRESS.md 后才汇报。

---

## M0 提示词 · 规范落地 + 工具链 + 基线

```
【XGE-API-DOC M0】你在继续 XGE API 注释任务（为 xge.h 751 个与 xui.h 3,362 个 API 添加中文契约注释，只加注释不改代码）。

第一步（必做）：完整读取 D:\GIT\xge\api-docs\SPEC.md 与 D:\GIT\xge\api-docs\PROGRESS.md，以 SPEC 为唯一事实源；与你记忆冲突时以 SPEC 为准，存疑的数字用 SPEC 第 2 节命令当场重测。

本阶段任务（对照 SPEC 第 6/7/8 节）：
1. 创建 D:\GIT\xge\api-docs\tools\ 下四个工具：
   - api_comment_coverage.py：统计 xge.h/xui.h 分模块注释覆盖率（判定规则：API 声明行上方 3 行内存在 /* */ 块注释），输出 coverage.json（含总函数数、已注释数、按前缀模块分组明细）；
   - comment_packet.py：按模块生成草稿包到 api-docs\packets\<module>.md，每条 API 附：签名、实现文件定位、实现体中 grep 到的全部 return 错误码清单、examples/test_xui/tutorial 中的用法行号引用；
   - comment_lint.py：实现 SPEC 第 8 节四条门禁（新 API 无注释、覆盖率棘轮、注释与声明间距、格式检查），支持 --report 模式；
   - verify_batch.bat：用 MinGW gcc -fsyntax-only 编译一个包含 xge.h 与 xui.h 的最小 TU，验证注释未破坏编译。
2. 运行 coverage 脚本生成 coverage.json 基线；用实测结果回填 PROGRESS.md 第 3 节模块清单（xge.h 按前缀聚类、xui.h 框架族 + 核对 58 控件 API 前缀）与第 2 节覆盖率表。
3. comment_lint.py 以 --report 模式跑通一次；verify_batch.bat 对未改动的头文件跑通。

完成标准（SPEC 第 7 节 M0 全部满足）。完成后：更新 PROGRESS.md（勾选 M0、写批次日志），汇报工具用法与基线数字。禁止 git commit；禁止修改 src/ 与头文件内容。
```

---

## M1 提示词 · xge.h 全量注释

```
【XGE-API-DOC M1】你在继续 XGE API 注释任务。

第一步（必做）：完整读取 D:\GIT\xge\api-docs\SPEC.md 与 PROGRESS.md，以 SPEC 为唯一事实源，冲突时以 SPEC 为准。用 TodoWrite 建立本阶段任务清单。

本阶段任务：为 xge.h 的全部 751 个 XGE_API 声明添加中文契约注释，并覆盖公开配置结构体（xge_desc_t 等）字段与枚举值。严格按 SPEC 第 4 节三档模板（L1/L2/L3）与内容优先级；每条行为声明按 SPEC 第 5 节取证：错误码必须 grep 实现体、平台限制/桩状态按 SPEC 第 9 节清单如实标注（如 XGE_INIT_RESIZABLE 等占位 flag 必须写明"当前为占位"）。

执行方式：
1. 用 comment_packet.py 按 PROGRESS 第 3.1 节模块清单逐模块出草稿包，对包写注释，一个模块一批；
2. 第一批完成后立即做安全检查：git diff --numstat 确认改动行数与新增注释行数同量级（若删除行数接近全文件说明行尾被破坏，必须修复 CRLF 后继续）；file xge.h 确认编码仍合法；
3. 每批跑 verify_batch.bat + api_comment_coverage.py，在 PROGRESS.md 第 4 节追加批次日志；
4. 发现代码疑似问题只记 FINDINGS.md，不修改代码。

完成标准（SPEC 第 7 节 M1 全部满足，含抽 20 条证据核对）。完成后更新 PROGRESS.md 并汇报。禁止 git commit。若本会话未完成全部模块，在 PROGRESS.md 第 5 节写明中断点后正常收尾汇报。
```

---

## M2 提示词 · xui.h 框架层

```
【XGE-API-DOC M2】你在继续 XGE API 注释任务。

第一步（必做）：完整读取 D:\GIT\xge\api-docs\SPEC.md 与 PROGRESS.md，以 SPEC 为唯一事实源，冲突时以 SPEC 为准。确认 M1 已完成（未完成则先按 PROGRESS 第 5 节中断点收尾 M1）。

本阶段任务：为 xui.h 框架层全部 API（PROGRESS 第 3.2 节清单：context/widget/layout/style/input/focus/scroll/popup/window/accessibility/unicode/text/icon/assets/proxy 等非控件族）添加中文契约注释。重点契约内容：控件树所有权（父级/context 持有，无 widget 级 AddRef）、样式键字符串协议（<widget>.<part>.<state>_color）、事件三阶段（CAPTURE/TARGET/BUBBLE）与返回值语义、1-based 数组模型、错误码 15 个体系——以上术语以 SPEC 第 3 节与源码注释为准，先 grep 后写。

执行方式同 M1：逐模块草稿包 → 写注释 → 首批行尾安全检查 → 每批门禁 → 批次日志。Get/Set 中的三类异常（所有权转移/副作用/单位约束）必须升档审校（SPEC 第 4 节）。

完成标准（SPEC 第 7 节 M2）。完成后更新 PROGRESS.md 并汇报。禁止 git commit、禁止改代码。
```

---

## M3 提示词 · 控件层（可反复发送续跑）

```
【XGE-API-DOC M3】你在继续 XGE API 注释任务（控件层，可中断续跑）。

第一步（必做）：完整读取 D:\GIT\xge\api-docs\SPEC.md 与 PROGRESS.md，以 SPEC 为唯一事实源。查看 PROGRESS 第 3.3 节 58 控件勾选状态：若已有进行中/未完成控件，从断点续跑；若本阶段未开始，从清单头部开始。

本阶段任务（每批发送可处理 5-8 个控件）：
1. A 层模板：对当批控件的 Get/Set/Is/Has API 用模板生成注释（"获取/设置<控件><属性>。"），逐条审校三类异常（返回需释放的指针、有副作用的 Set、有单位/取值约束的值），异常条目升为 L2/L3；
2. C 层手写：当批控件的 Create/Destroy/Desc 结构体（逐字段）/事件注册/样式键族 API，按 SPEC 第 4/5 节写 L2/L3；Desc 字段含义从控件 data 结构体初始化代码与 examples\xui_<控件>\ 用法取证；
3. 每批：verify_batch + coverage 增量核对 → PROGRESS.md 第 3.3 节勾选当批控件 → 第 4 节批次日志。

完成标准（SPEC 第 7 节 M3：xui.h 函数注释覆盖率 100%）。单次会话未做完属正常，写清断点即可。禁止 git commit、禁止改代码。
```

---

## M4 提示词 · 类型/枚举/宏扫尾 + 棘轮 + CI + 终验

```
【XGE-API-DOC M4】你在继续 XGE API 注释任务的收官阶段。

第一步（必做）：完整读取 D:\GIT\xge\api-docs\SPEC.md 与 PROGRESS.md，以 SPEC 为唯一事实源。确认 M1-M3 均已完成；未完成则先收尾前序阶段。

本阶段任务（SPEC 第 7 节 M4）：
1. 扫尾：全部枚举值逐条注释；剩余 desc 结构体字段注释；样式键/常量宏按控件分组补分组注释；
2. 棘轮收紧：重跑 api_comment_coverage.py，把 coverage.json 更新为当前值（此后任何下降都会被 comment_lint.py 拦截）；
3. CI 挂载：在 .github/workflows/uidesign.yml 追加一个纯 Python 的 api-docs job（跑 api_comment_coverage.py + comment_lint.py，不依赖编译器），或接入本地 check_platform_all.bat——二选一，说明理由后实施；
4. 终验：从两头随机抽 50 条已写注释，逐条核对行为声明证据（错误码/用法/平台限制可回溯到实现或 SPEC 第 9 节），输出核对表；
5. 输出最终报告：总覆盖数、分阶段批次统计、FINDINGS.md 汇总、后续（官网集成）衔接建议。

完成标准（SPEC 第 7 节 M4 全部满足）。禁止 git commit、禁止改 src/ 与头文件代码。
```

---

## 续跑提示词 · 任意阶段中断后恢复（万能兜底）

```
【XGE-API-DOC 续跑】恢复 XGE API 注释任务。

第一步（必做）：完整读取 D:\GIT\xge\api-docs\SPEC.md 与 D:\GIT\xge\api-docs\PROGRESS.md。以 SPEC 为唯一事实源，与你记忆冲突时以 SPEC 为准；存疑数字用 SPEC 第 2 节命令重测。

第二步：根据 PROGRESS.md 第 1 节阶段状态、第 3 节勾选清单、第 5 节中断点，判断当前应执行的阶段与批次；向我复述一句话（当前阶段、断点位置、本批计划范围）确认后继续执行。

执行期间遵守 SPEC 第 10 节全部禁止事项（只加注释、不改代码、不 commit、不引用 docs/README/官网为据），每批门禁（verify_batch + coverage）通过并更新 PROGRESS.md 后才汇报。
```

---

## 使用备忘

- 顺序：M0 → M1 → M2 → M3（可多次发送）→ M4；中断后用"续跑"提示词。
- 用户审阅与 git 提交由用户本人完成，助手不代提交。
- 若任务范围/规范变更：先让助手更新 SPEC.md（附理由），再继续阶段提示词。
