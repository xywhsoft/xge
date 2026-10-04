# 阶段提示词（多会话续作时逐段投喂）

## M0 校验脚本
在 D:\GIT\xge 写 tools/check_site_facts.py：--site 指向 wwwroot（默认 D:/GIT/home/host/xge/wwwroot）。
检查组：① 数字类（按 SPEC 事实基准表，源码实测对比站点字符串）；② API 名存在性（提取站点
index.html 代码面板与 docs/index.html 表格中的 xge*/xui*/XGE_*/XUI_* 调用名与宏，对 xge.h+xui.h
grep）；③ 构建脚本存在性。FAIL 输出 ASCII，退出码 1。禁止修改两个仓库的 git 状态。

## M6 工程卫生
_preview_*.png(8) 移至 home 仓 xge/previews/；wwwroot/res/img/favicon.svg（复用 BrandMark）；
批量脚本为全部 HTML 注入 favicon link 并移除 maximum-scale=1（保留原行尾）；robots.txt；
xge/tools/gen_sitemap.py（参数化域名）。

## M1 主页
index.html：meta/hero/ticker/Bento(+3D 大格+可裁剪构建格)/代码Tab(+3D)/控件画廊(+Document, 52)/
统计(930K+,80+,117,239,22K,+105 3D API 视 CSS 栏数而定)/架构图引擎层+3D/下载区 build_dll.bat 3d。
xge.js：Demo3D（旋转线框+渐变着色）加入 data-demo 与 data-ex 映射。

## M2 docs
构建系统节+档位表(7档+65开关+features.json，Linux 未实机验证注明)；新增 3D API 节与
Document 节（函数名全部现场 grep xge.h/xui.h 核实）；修正 SPEC 所列 5 处 API 错误；
changelog 增"2026-10 更新"块。

## M3 download
PROXY_VERSION 16；GitHub 仓库/Releases/archive-zip 三链 + Gitee 并列(国内提速)；构建表加档位
说明；版本说明块更新。

## M4 examples
过滤加"3D"；7 张新卡（xge_3d/_scene/_lighting/_animation/_integration/_walk/xui_document），
data-ex 用 threeD 或既有 kind；文案取自各 README。

## M5 教程
先 M5a 截图：从仓库根 python examples/<name>/build.py --run --frames N --capture out.png
（xui_document 先 build_dll.bat 默认档）；PIL 重采样宽 980 存 img/chNNN_1.png。
M5b/5c 按 SPEC 章节表写 ch212-ch227（模板对齐 ch01/ch211：Nav/PageHero/TutGrid/TutSide/
TutMain/API速查/要点/练习/TutPager/Footer）；更新 tutorial/index.html（Part22 区块+Part18 两张
扩展卡+全部"210 章"→"227 章"+meta）、ch01 目录树行数、ch210 分页 next→ch212。

## 收尾
gen_sitemap 待域名；跑 check_site_facts.py 全绿；链接检查脚本零死链；PROGRESS 勾项；不 commit。
