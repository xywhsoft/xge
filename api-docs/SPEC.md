# XGE API 注释任务 SPEC —— 防漂移事实锚点

- 创建日期：2026-09-10
- 本文件是本任务的**唯一事实源**。任何会话中，记忆与本文冲突时，以本文为准；本文数字存疑时，用第 2 节命令重新实测并更新本文（附命令与日期）。
- 修改本文仅限：重新实测后的数字更正、用户明确做出的决策记录、任务范围变更。禁止凭记忆增删事实。
- 配套文件：`PROGRESS.md`（可变状态，每批必须更新）、`prompts.md`（M0-M4 阶段提示词）。

---

## 0. 任务定义（一句话）

为 `xge.h`（751 个 XGE_API）与 `xui.h`（3,362 个 XUI_API）的全部公开声明添加**一行中文契约注释**（`/* ... */` 格式），并覆盖 desc 配置结构体字段、枚举值、样式键宏；**只加注释，不改任何代码**。

## 1. 事实源层级与防幻觉守则

事实可信度从高到低：

1. 当前源码（用 grep/读文件当场验证）
2. 本 SPEC 的已测量事实表（附命令，可复验）
3. 本 SPEC 第 9 节"代码真话"清单（来自 2026-09 源码深度分析，带文件:行号）
4. 其他一切（包括本助手的历史记忆、docs/、README.md、wwwroot 官网）——**一律不可作为事实依据**

硬性守则：

- **不信任记忆中的行号**。编辑任何声明前，先重新 grep 定位。
- 注释中的每条**行为声明必须有证据**：错误码来自实现体 grep、用法来自 examples/test/tutorial、平台限制来自第 9 节清单。没有证据就只写语义，不猜行为。
- 不确定时重新测量；测量结果与 SPEC 不符时，以新测量为准并更新 SPEC 第 2 节。
- 发现代码疑似 bug 或文档失实：只记录到 `FINDINGS.md`，**不修改代码**。

## 2. 已测量事实表（2026-09-10，工作目录 D:\GIT\xge）

| 事实 | 数值 | 测量命令 |
|---|---|---|
| xge.h 函数声明 | 751 | `grep -c "XGE_API" xge.h`（含 4 行 `#define XGE_API` 宏定义） |
| xui.h 函数声明 | 3,362 | `grep -c "XUI_API" xui.h`（含 4 行 `#define XUI_API` 宏定义） |
| 声明行数（M0 实测，排除 #define 行；=注释任务工作总量） | xge.h 747 / xui.h 3,358 | `python api-docs/tools/api_comment_coverage.py` 输出的 total_marker_lines（coverage.json 为准；函数名提取规则：取 `(` 前的前缀标识符，防止把返回类型当函数名） |
| xui.h Get/Set 模式 | 2,457 | `grep -E "XUI_API" xui.h \| grep -cE "(Get\|Set)[A-Z]"` |
| xui.h Is/Has 模式 | 79 | `grep -E "XUI_API" xui.h \| grep -cE "(Is\|Has)[A-Z]"` |
| xge.h Get/Set 模式 | 121 | `grep -E "XGE_API" xge.h \| grep -cE "(Get\|Set)[A-Z]"` |
| xge.h typedef struct | 127 | `grep -c "^typedef struct" xge.h` |
| xui.h typedef struct | 289 | `grep -c "^typedef struct" xui.h` |
| xge.h enum / xui.h enum | 1 / 3 | `grep -cE "^(typedef )?enum" xge.h` / `xui.h` |
| XGE_/XUI_ 宏定义 | 387 / 1,390 | `grep -cE "^#define (XGE_\|XUI_)" xge.h` / `xui.h` |
| xge.h 行数 / xui.h 行数 | 2,541 / 9,298 | `wc -l` |
| xge.h 行首块注释 | 32 | `grep -c "^/\*" xge.h`（现状≈零文档注释） |
| xui.h 行首块注释 | 29 | `grep -c "^/\*" xui.h` |
| 控件类型注册数 | 58 | `grep -h "tDesc.sName" src/xui_*.c \| wc -l` = 59，其中 1 条为 `XUI_BUILTIN_ATLAS_RESOURCE_NAME`（资源名，非控件）→ 58 |
| 控件类型名单 | 见 PROGRESS.md | `grep -h "tDesc.sName" src/xui_*.c \| sed 's/.*= *//'` |

### 2.1 编码与格式事实（编辑头文件必读）

| 事实 | 说明 |
|---|---|
| xge.h / xui.h 当前编码 | **纯 ASCII + CRLF**（`file xge.h xui.h` 实测） |
| 中文注释先例 | `src/xge_impl.c`、`src/xge_gl.h` 为 **UTF-8 + CRLF**，含中文注释（如 `src/xge_impl.c:176 // XGE 全局运行状态`），同一 MinGW gcc 无 charset 参数编译通过 |
| **编辑规则** | 新注释行按 **UTF-8 中文**写入（ASCII 文件追加 UTF-8 行不改变既有字节），**保持 CRLF 行尾**；Edit 工具默认行为需验证不破坏 CRLF |
| 缩进 | 头文件使用 **Tab** 缩进 |
| 注释格式 | API 声明注释一律 `/* ... */`（继承 xrt.h 风格）；头文件虽有既有 `//` 注释（xge.h 含 `//` 的行 74、xui.h 60），新增 API 文档注释不用 `//` |

### 2.2 风格先例（xrt.h，同作者的成熟库）

`D:\GIT\xrt\single\xrt.h`：5,922 个 API 配 14,636 条中文块注释全覆盖（`grep -c "^/\*"` / `grep -c "XRT_API\|XRT_INLINE"`），中位长度 30 字符、P90 52、最长 174，**零 doxygen 标签**（`grep -c "@param\|@return"` = 0）。示例：

```c
/* 使用全局堆的默认对齐初始化空数组。 */
XRT_API bool xrtArrayInit(xarray* pArray, size_t iItemSize);
```

## 3. 环境与仓库事实

| 项 | 值 |
|---|---|
| 仓库 | `D:\GIT\xge`，远端 `https://gitee.com/xywhsoft/xge`，主分支 master，提交人 xLeaves（用户本人） |
| 构建形态 | unity build：`xge.c`（5 行）聚合 `src/xge_impl.c` → 全部 `src/xge_*.c`；产物 `build/xge.dll + xge.lib`（MinGW gcc，`build_dll.bat`） |
| XUI 源清单 | `xui_sources.bat`（仅被 `build_dll.bat` 消费——`build_dll.sh`/`build_dbg_dll.bat` 不含 XUI，与注释任务无关但为已知事实） |
| 语言纯度 | 纯 C，`tools/check_c_language_boundary.py` 强制（禁 C++ 语法） |
| 版本宏 | xge.h:19-21 与 xui.h:20-22 均为 2.0.0 |
| 匈牙利记法 | i=int、f=float、s=char*、p=指针、b=bool、on=回调、t=结构实例、arr=数组、pp=输出句柄指针（如 `xge_svg* ppSvg`） |
| XUI 数组模型 | 1-based（xui_xrt_port.h:27-37 注释） |
| 错误码体系 | xui.h:24-40（XUI_OK..XUI_ERROR_INVALID_STATE 共 15 个）；xge.h:486-503 镜像 15 个；实现体以 `return XUI_ERROR_*` / `return XGE_*` 返回 |

## 4. 注释规范

**格式**：`/* 一句话中文。 */` 紧贴声明上方（≤1 空行），不用任何 doxygen 标签。

**三档模板**：

```c
/* L1：简单访问器（Get/Set 主体） */
/* 获取按钮文本。 */

/* L2：语义 + 契约（标准档，多数 API） */
/* 加载 SVG 文档；sURI 支持 res:// 打包路径。失败时句柄保持原状态。 */

/* L3：多行块（复杂/危险 API：错误码、所有权、生命周期、线程/平台） */
/* 异步加载并解码图片。回调统一在 xgeAsyncPoll（调用线程）触发；回调内可
 * 释放请求本身，此后不得再访问请求句柄。
 * 返回 XGE_OK / XGE_ERROR_INVALID_ARGUMENT / XGE_ERROR_UNSUPPORTED（仅图片类走线程）。 */
```

**内容优先级（契约优先）**：语义 → 约束（参数方向/所有权/单位/生命周期）→ 失败行为（返回哪些错误码、输出参数是否被触碰）→ 平台/线程限制。

**禁写**：签名能看出的复述（如"pContext：上下文"）、实现细节、变更历史、营销语言。

**Get/Set 三类异常必须升档审校**（A 层模板批量时的审校重点）：

1. 返回值需调用方 Free/Destroy 的（所有权转移，必须写明）；
2. Set 有副作用的（触发失效/重排/重绘/异步）；
3. 值有单位或取值约束的（ms/像素/DPI 档/枚举范围）。

**结构体**：58 个 `*_desc_t` 配置结构体逐字段注释优先；内部结构体只注类型本身。**枚举**：类型一句话 + 每值一句话。**样式键宏**（约千余个 `#define XUI_STYLE_KEY_*` 类，以 M0 实测清单为准）按控件分组一句话。

## 5. 证据规则（四大矿，写准不写猜）

1. **错误码自动提取**：定位 API 实现函数体，grep 其 `return XGE_.../XUI_ERROR_...` 得失败行为清单。例：`grep -n "return XUI_ERROR_\|return XGE_ERROR_\|return XGE_OK" src/xui_combobox.c`。
2. **用法实证**：`examples/`（110 个目录）、`examples/tutorial_capture/`（211 章）、`test/`（18 .c）+ `test_xui/`（174 .c）中的真实调用。
3. **内部契约注释搬迁**：src 内既有英文契约注释（生命周期/线程/所有权）翻译并入对应公开 API 的 L3。已知锚点：`src/xge_async.c:41-42`（回调可释放请求）、`src/xui_internal.h:337-338`（onPreparePaint 不得绘制/跑布局）。
4. **第 9 节"代码真话"清单**：平台限制与桩状态以此为准，不得凭印象写。

**草稿包**：`tools/comment_packet.py` 按模块生成 `packets/<module>.md`（签名 + 错误码提取 + 用法行号），人工/AI 对包写注释。

## 6. 目录与工具布局

```
D:\GIT\xge\api-docs\
├── SPEC.md            # 本文件（稳定锚点）
├── PROGRESS.md        # 可变状态：阶段/模块勾选/覆盖率基线（每批必更）
├── prompts.md         # M0-M4 提示词
├── FINDINGS.md        # 代码问题记录（只记不修）
├── coverage.json      # 覆盖率基线（M0 生成，棘轮依据）
├── tools\
│   ├── api_comment_coverage.py   # 覆盖率统计（声明前 3 行内有无 /* */）
│   ├── comment_packet.py         # 草稿包生成（签名+错误码+用法引用）
│   ├── comment_lint.py           # 门禁（见第 8 节）
│   └── verify_batch.bat          # 批次语法验证（gcc -fsyntax-only 含两头的 TU）
└── packets\           # 草稿包输出
```

## 7. 阶段定义与验收标准

| 期 | 任务 | 验收标准（全部满足才算完成） |
|---|---|---|
| **M0** | 规范落地 + 3 脚本 + 基线 | ① 3 个脚本 + verify_batch.bat 可运行；② coverage.json 生成（分模块数字）；③ PROGRESS.md 回填实测模块清单（含 58 控件名单）；④ verify_batch 对未改动头文件通过；⑤ 门禁以 report 模式跑通 |
| **M1** | xge.h 全量（751 API + 公开类型/枚举） | ① xge.h 函数注释覆盖率 100%；② `xge_desc_t` 等配置结构体字段 100%；③ verify_batch 通过；④ 抽 20 条核对注释行为声明有证据 |
| **M2** | xui.h 框架层（context/widget/layout/style/input/focus/scroll/popup/window/accessibility/unicode/icon/assets/proxy 等非控件族，清单以 M0 实测前缀分布为准） | 上述族函数覆盖率 100% + verify_batch 通过 |
| **M3** | 控件层：A 层 2,457 条 Get/Set 模板 + 58 控件 Create/Desc/事件族；按控件分批（每批 1-N 个控件），PROGRESS.md 勾选推进，**可中断续跑** | xui.h 函数注释覆盖率 100% + verify_batch 通过 |
| **M4** | 类型/枚举/宏扫尾 + 棘轮收紧 + CI | ① 枚举值 100%、desc 字段 100%、样式宏分组注释；② coverage.json 棘轮收紧至当前值；③ CI 挂载（uidesign.yml 追加纯 Python job 或本地 check 挂载点）；④ 随机抽 50 条全量核对；⑤ 最终报告 |

## 8. 门禁规则（comment_lint.py）

1. **新增 API 无注释 → fail**（对比 git 或对比 coverage 基线增量）；
2. **覆盖率棘轮**：低于 coverage.json 记录值 → fail；
3. 注释与声明间隔 >1 空行 → 格式告警；
4. verify_batch.bat 语法检查失败 → fail（防注释破坏编译）。

## 9. "代码真话"清单（写注释时必须如实标注，2026-09 源码分析结论）

### 9.1 占位 flag（定义了但 src/ 零引用——注释必须写"当前为占位"）

- `XGE_INIT_FULLSCREEN / NOFRAME / RESIZABLE / OFFSCREEN`（xge.h:55-62 定义；仅 VSYNC/HIGHDPI 等真实生效）

### 9.2 桩/受限 API（注释必须写明限制）

| API/能力 | 真话 | 证据 |
|---|---|---|
| 渲染线程族（xgeRenderThreadSet 等） | 开启路径恒 UNSUPPORTED；关闭路径真实生效（Join+返回 OK）【2026-09-10 验收实测修正，原"恒返回 UNSUPPORTED"过时】 | xge_core.c 实测 |
| 高级混合 XGE_BLEND_OVERLAY..LUMINOSITY | 仅告警忽略 | xge_impl.c:1327-1331 |
| 手柄（xgeGamepad*） | 仅注入接口，无事件源；caps 却报支持 | xge_core.c:464 |
| UBO（XGE_BUFFER_UNIFORM） | 不支持 | xge_buffer.c:77 |
| xgeRender（手动模式） | 轮询运行状态（1 运行/0 退出；含初始化与模式检查）【2026-09-10 验收实测修正，原"空壳"过时】 | xge_core.c 实测 |
| 引擎级脏矩形（xgeInvalidateRect） | 只记录+触发按需渲染，不驱动局部重绘 | xge_core.c:1368-1386 |
| xgeMeshDraw | 每次调用全量重传顶点 | xge_mesh.c:231 |
| 异步加载（xgeAsync*） | 仅 IMAGE/TEXTURE 走线程；FONT/SOUND 同步 | xge_async.c:102-114 |
| xgeMusic*/xgeStream* | 与 xgeSound* 同实现（仅 flag 差异） | xge_audio.c:460-463 |
| EGL 离屏（xge_egl.c） | 代码在，但无构建脚本定义 XGE_HAS_EGL | 全仓 grep |
| 音频 | 需手动 xgeAudioInit 惰性初始化 | xge_audio.c |

### 9.3 Win32 专属能力（非 Windows 无实现，注释必须带平台标注）

- TSF 输入法全家族（xge_ime_win32_tsf.c，1928 行；xge_input.c:185-187 条件 include）
- OLE 拖出/多格式拖放（xge_drag_drop.c；非 Win32 仅文件拖入）
- 多格式剪贴板（xge_render.c:1138-1213；非 Win32 纯文本）
- on-demand 空闲唤醒（xge_impl.c:427-434 MsgWaitForMultipleObjects，仅 Win32）

### 9.4 引擎架构事实（描述行为时用）

- 单线程契约：UI/引擎对象非线程安全；例外：渲染命令队列自旋锁（xge_command.c:18-33）、emoji 缓存 mutex、async 回调固定在调用线程（xgeAsyncPoll）
- 帧流程：整屏 glClear 全帧重绘；XUI 层才有真正的损伤矩形渲染（xuiRender damage 列表）
- 纹理 CPU shadow 永久保留（xgeTextureReadPixels 从 CPU 读）
- 资源对象生命周期：调用方持有结构 + int 引用计数，Free 后 memset 归零，复用前必须 zero-init（xge.h:1861-1862 注释约定）

### 9.5 文档侧已知错误（本任务**不做**官网修复，仅防误导）

- 官网存在不存在的 API 名：xuiContextCreate / xuiWidgetOn / xuiInputDispatch（真实 API：xuiCreate + xuiSetProxy、xuiWidgetSetEventCallback/EventHandler、xuiInputPointer* 族）
- XUI_PROXY_VERSION 实际 = 10（xui.h:24），官网写 4
- 官网统计数字过时（430K 行等），不得引用

## 10. 禁止事项

1. **禁止修改任何 API 声明、实现、宏值**——只允许在声明上方插入注释行。发现代码问题 → FINDINGS.md。
2. **禁止 git commit / push**——由用户审阅后自行提交。
3. 禁止引用 docs/、README.md、wwwroot 作为事实依据（已证严重失实）。
4. 禁止照抄 xrt.h 的注释语义（只继承格式；xge/xui 语义必须来自本仓库证据）。
5. 禁止为凑覆盖率写无信息量注释（"设置值。"这类复述签名的注释视为未完成）。
6. 每批完成必须：跑 coverage + verify_batch，更新 PROGRESS.md，然后才汇报。

## 11. 决策记录

| # | 决策 | 状态 |
|---|---|---|
| D1 | 注释语言 = 中文 | 已定（xrt.h 先例 + 用户语境） |
| D2 | 不用 doxygen 标签，一行式 /* */ | 已定（xrt.h 先例：0 标签） |
| D3 | A 层草稿生产 = 规则模板打底 + AI 按草稿包起草、用户审校 | 默认采用，用户可改 |
| D4 | 脚本放 api-docs/tools/（不进仓库 tools/ 子目录体系） | 已定（自包含，避免与现有工具混淆） |
| D5 | CI 挂载推迟到 M4 | 已定 |

## 12. 路径索引

| 资源 | 路径 |
|---|---|
| 仓库 | D:\GIT\xge |
| 官网（后续阶段，本任务不动） | D:\GIT\home\host\xge\wwwroot |
| xrt 官网参考实现（后续阶段） | D:\GIT\home\host\xrt\wwwroot 与 book-work\ |
| xrt.h 风格先例 | D:\GIT\xrt\single\xrt.h |
| 示例 | D:\GIT\xge\examples\（110 目录，含 tutorial_capture 211 章） |
| 测试 | D:\GIT\xge\test\（18 .c）、test_xui\（174 .c） |
