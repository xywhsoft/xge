# PROGRESS

- 2026-10-04 方案交付，D1/D2/D3 已拍板（见 SPEC）。
- 2026-10-04 侦察完成：站点在 gitee.com/xywhsoft/home 仓（工作区干净）；gcc16.2/Py3.12/PIL 可用；
  教程图 800x600 为主、ch211 为 980x640；各 3D 示例 build.py 自带 profile 构建与 --frames/--capture/--seed。
- [x] SPEC 建立
- [x] M0 check_site_facts.py（19 项检查；legacy 章节 ch02-ch211 名字门禁隔离，债 137 个虚构标识符见 SPEC M7）
- [x] M6 工程卫生：8 张 _preview_*.png → home 仓 xge/previews/；favicon.svg 注入 232 页；
  viewport 去 maximum-scale=1 全站；robots.txt；xge/tools/gen_sitemap.py（域名待定）
- [x] M1 主页：meta/hero/tags/ticker/Bento(+3D 大格+地形雾天空中格+可裁剪构建中格)/代码Tab(+3D)/
  画廊(+Document，51)/架构图(+3D Core，114 示例，80+ 控件)/统计 6 列(930K+/80+/105/8/22K/239)/
  下载卡(+build_dll.bat 3d)；xge.js +Demo3d/DemoTerrain；CSS StatsGrid 6 列 + RvD5
- [x] M2 docs：三头文件口径、构建档位表、3D API 节(14 行)、Document 节(154 API)、
  XUI 表勘误(xuiCreate/SetProxy/SetEventCallback/DispatchEvent/AddStyleClass/SetInlineStyle/
  SetLayoutType/SetDock/SetAlign/Theme)、changelog 2026-10
- [x] M3 download：PROXY 16、GitHub 仓库/archive zip/Releases + Gitee(国内提速)、档位注记
- [x] M4 examples：3D 分类 + 6 张 3D 卡 + xui_document 卡（threeD/terrain 缩略动画）
- [x] M5a 截图：16 张全部真实产物（3D 12 张 --capture；walk 不同 seed/frames/first-person 出 6 张；
  xui_document --verify 失败(-6 FILE_NOT_FOUND 原因未明)→改交互开窗+PowerShell 截图/点击 Visual 再截）；
  PIL 重采样宽≤980 → tutorial/img/ch212..227_1.png；黑帧方差检测通过；ch224/ch226 视觉目检通过
- [x] M5b Part22 14 章（ch212-214 手写，ch215-227 由 plans/xge-site-update/gen_chapters.py 生成）
- [x] M5c：索引 227 口径 + Part22 区块 + Part18 两张扩展卡；ch01 行数修正(+xui_document.h 行)；
  ch210 next→ch212
- [x] 收尾：check_site_facts.py 19/19 ALL GREEN；232 页本地链接 0 死链；教程 227 页

## 交付物清单

- home 仓（未提交，262 项变更）：wwwroot 五页更新 + 16 新章 + 16 图 + favicon/robots/CSS/JS；
  previews/（移出的设计稿）；tools/gen_sitemap.py
- engine 仓（未提交）：tools/check_site_facts.py；plans/xge-site-update/{SPEC,PROGRESS,prompts,
  apply_hygiene,capture_shots,capture_doc.ps1,gen_chapters}
- 截图原始产物：artifacts/site-shots/（16 张）

## 待办/悬而未决（下轮）

1. ~~sitemap.xml~~ 已完成（2026-10-04）：域名 xge.xywhsoft.com，232 URL 已生成，
   robots.txt 已补 Sitemap 行。内容变化后重跑
   `python D:/GIT/home/host/xge/tools/gen_sitemap.py --domain https://xge.xywhsoft.com`。
2. ~~M7 老教程 API 名勘误~~ 已完成（2026-10-04 第二轮）：
   - 精确清单：词元集合匹配（非子串）+ 前缀家族规则（下划线尾/下划线续/小写续/显式家族集），
     真债 139 记号（m7_inventory/m7_context 报告留档）。
   - apply_m7.py：A 记号改名 40+ 条（xgeGetDeltaTime→xgeGetDelta、XGE_PI→字面量、
     键盘字母/数字→ASCII 字面值、BLEND/PASS/DECO/TOUCH 系列等）+ B 模板族重构
     （代理惯用法 xuiProxyXge 185 处、三参 WidgetCreate→per-widget Create+AddChild、
     SetSize/SetPos→SetRect 合并 100+ 处、SetText→Label/ButtonSetText、ColumnCreate→
     LayoutType、xgeDrawText 参数重排 60 处、AppendRoundRect 参数补齐、
     5/6 参矩形→RectFill/RectStroke 结构体化、鼠标索引→XGE_MOUSE_* 位）。
   - handfix_m7{,b,c}.py + 补丁：28 个语义簇手写重写（矩阵栈→xge_draw_t、字形位图→纹理、
     图表序列、状态栏 AddText、触摸点类型/相位、i18n 文本槽枚举、手柄轴索引、
     事件掩码/停传返回值、焦点/使能反转、代理 caps/ops、模态按实例、TimelineView 大写 L 更正——
     原章节此处本是对的，被第一轮误改后回滚）。
   - 教训：教程 HTML 为 CRLF，多行模式必须 \r?\n；章节有"片段+完整代码"双副本。
   - 门禁升级：check_site_facts.py 改词元集合匹配 + 家族规则，legacy 排除已移除，
     名字门禁覆盖全部 227 章；全站 232 页链接 0 死链；ALL GREEN。
3. check_site_facts.py 挂 CI ratchet（脚本退出码已就绪）。
4. Google Fonts 自托管（可选，涉及部署环境；本轮未做，站点仍引 fonts.googleapis.com）。
5. xui_document --verify 的 -6 FILE_NOT_FOUND 待查（不影响站点；交互模式正常）。
