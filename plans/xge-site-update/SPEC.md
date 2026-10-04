# XGE 官网内容更新 — SPEC

目标：把 2026-07-24 快照之后的引擎新能力（3D、可裁剪构建、xuiDocument、全局主题）更新到官网
（`D:\GIT\home\host\xge\wwwroot`，属 gitee.com/xywhsoft/home 仓库，**只改文件不 commit**），
并修正全部已核实的数字/API 失实，补防漂移校验脚本。

## 已拍板决策（2026-10-04）

- D1：download 页链接指向 GitHub 仓库/Releases 页面（`https://github.com/xywhsoft/xge`，
  Releases 为 `.../releases`，ZIP 用 `.../archive/refs/heads/master.zip`）；Gitee
  （`https://gitee.com/xywhsoft/xge`）与 GitHub 并列，标注"国内提速"。GitHub 是正式分发渠道。
- D2：教程本轮全做 —— Part 22「3D 图形」14 章（ch212-ch225）+ xuiDocument 扩展 2 章
  （ch226-ch227，挂在 Part 18 清单内，沿用 ch211 的"扩展"编号模式）。
- D3："总代码行"按全仓含内置库口径（src/ + lib/ + 根头文件，2026-10-04 实测 931,421 行 → 站点写 930K+）。

## 事实基准表（M0 校验脚本以此为准；引用前当场复测）

| 项 | 旧值 | 新值 | 口径 |
|---|---|---|---|
| XUI_PROXY_VERSION | 4 | 16 | xui.h `#define` |
| 版本 | v2.0.0 | v2.0.0（不变） | xge.h/xui.h 版本宏 |
| 总代码行 | 430K+ | 930K+ | src+lib+根头文件 wc -l |
| SVG 引擎行数 | 22,000 | 22,000（实测 22,221，保持） | src/xge_svg.c |
| shape_ex 行数 | 18,000 | 21,800 | src/xge_shape_ex*.c 合计 |
| 控件数 | 60+ | 80+（xui*Create 构造函数 80 个；可裁剪开关口径 65） | xui.h |
| 示例数 | 90+ | 114 | examples/ 目录数（audit_* 与 tutorial_capture 均含 build.bat，属可运行示例） |
| 控件测试文件 | 99 | 239 | test_xui/*.c |
| 3D 公开 API | — | 105（xge3d* 函数名去重） | xge.h |
| 3D 子模块开关 | — | 8（XGE3D_ENABLE_MODEL/LIGHTING/SHADOW/IBL/ANIMATION/TERRAIN/ASYNC/FOG） | xge_config.h |
| 3D 示例 | — | 6（xge_3d/_scene/_lighting/_animation/_integration/_walk） | examples/ |
| 构建档位 | — | 7（default/core/2d/3d/ui-min/full-dev/ide） | docs/BUILD_PROFILES.md |
| 控件及服务开关 | — | 65（xui_config.h XUI_ENABLE_*） | xui_config.h |
| 教程章数 | 210（实 211 页） | 227（211+14+2） | tutorial/ch*.html |
| xge.h / xui.h / xrt.h 行数（ch01） | ~2000/~7800/88K | 3,870 / 12,687 / 318,537 | wc -l |
| DLL 体积示例 | — | 3d 档 873,737 B；core 672,176 B | docs/3D_RELEASE.md（引用时注明日期口径） |

API 更正（docs 页与主页代码示例）：
- `xuiContextCreate(&ctx,&proxy)` → `xuiCreate(&ctx)` + `xuiSetProxy(ctx,&proxy)`（或 `xuiProxyXge()`）
- `xuiWidgetOn` → `xuiWidgetSetEventCallback`（xui.h:12238）
- `xuiInputDispatch*` → `xuiDispatchEvent` / `xuiDispatchPendingEvents`
- `xuiWidgetCreate(&w,ctx,NULL)` → `xuiWidgetCreate(ctx,&w)`（两参数，无 parent）
- 创建模式文案 → `xui<Name>Create(ctx, &widget, &desc)`

## 章节编号与挂载

- Part 22「3D 图形」：ch212 场景与节点 / ch213 相机 / ch214 glTF 模型 / ch215 材质 /
  ch216 直接光照 / ch217 阴影 / ch218 静态 IBL 与环境 / ch219 骨骼动画 / ch220 地形高度场 /
  ch221 距离雾与天空 / ch222 异步加载与预算 / ch223 拾取与射线 / ch224 实战：岛屿行走 /
  ch225 综合集成与可裁剪构建。
- ch226/ch227（xuiDocument：富文档与事务 / Markdown 三模式与修订）挂在 Part 18 清单尾，
  卡片标"扩展"。
- ch210.html 分页 next 由"返回教程首页"改为 ch212；ch225 next → ch226？否：ch225 是 3D 篇末，
  next → 返回教程首页；ch226 prev → ch185（Part 18 末），ch227 next → ch186（Part 19 首）。
  编号顺序与阅读顺序分离是既有 ch211 模式，允许。
- 截图：全部用示例 `--frames N --capture` 真实产物，重采样到宽 980（对齐 ch211 规格），存
  tutorial/img/chNNN_1.png。

## 里程碑

- M0 校验脚本 `tools/check_site_facts.py`（engine 仓库；--site 指向 wwwroot）：数字对源码实测、
  全站函数名/宏名对头文件存在性、构建脚本存在性；FAIL 退出码 1。
- M6 工程卫生：_preview_*.png 移到 home 仓 `xge/previews/`；favicon.svg + 全站 link 注入；
  robots.txt；全站去 `maximum-scale=1`；sitemap 生成脚本（域名未定，见 PROGRESS 待办）。
- M1 主页 / M2 docs / M3 download / M4 examples：见 prompts/。
- M5 教程：先截图后成文；API 名一律出自 xge.h 3D 节（3432-3870 行）与 xui.h xuiDocument 节。
- M7（2026-10-04 完成）：老教程 API 名勘误。精确扫描（词元集合 + 前缀家族规则）定债
  139 个真记号；经 apply_m7 批量变换（A 记号改名 / B 模板族重构）与 handfix_m7{,b,c}
  三轮手修清零；门禁已覆盖全部 227 章。工具与映射留档于 plans/xge-site-update/。

## 实测口径备注（2026-10-04）

- 总代码行 933,747（含 xui_document.h/xui_document_ui.h/两个 config 头）→ 站点写 930K+。
- 公开头文件实为 4 个：xge.h / xui.h / xui_document.h（1,154 行，154 个 API）/
  xui_document_ui.h；docs 页"两个头文件"的说法要改。
- 教程截图生产管线在 examples/tutorial_capture/（chNN_main1.c 逐章权威示例）。
- XGE 键盘：无字母/数字键宏（XGE_KEY_A 等不存在），可打印字符直接用 ASCII 字面值；
  修饰键用 XGE_KEY_MOD_CTRL 位掩码；手柄轴为纯索引 0..7 无语义宏（桥接层约定）。

## 验收

1. `python tools/check_site_facts.py --site <wwwroot>` 全绿。
2. 站内链接零死链（脚本扫描 href/src 本地目标存在性）。
3. 新教程 16 章齐全、索引 227 章口径一致（"210 章"字样清零）、截图 16 张为真实 capture。
4. 主页/docs/download/examples 无虚构 API 名（M0 脚本覆盖）。
5. 助手不执行任何 git commit（两个仓库都不 commit）。

## 环境备忘

- gcc：E:\software\w64devkit（PATH 注入 `/e/software/w64devkit/bin`）；Python 3.12 + PIL 可用。
- bat 内容 ASCII+CRLF；Python 控制台输出仅 ASCII。
- wwwroot HTML 均为 UTF-8；批量脚本须保留各文件原行尾。
