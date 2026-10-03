# XGE 轻量 3D 开发进度

更新日期 2026-10-03。执行依据：[IMPLEMENTATION.md](IMPLEMENTATION.md)。

## 当前状态

长期任务的本期范围已完成：P0 至 P6 必需工作包及交叉验收均已验证。工作区基线与原有修改保存在 `artifacts/xge-3d/baseline/`；历史 2.5D 能力不计为新 3D 工作包完成。交付、复现及限制见 [3D_RELEASE.md](../../docs/3D_RELEASE.md)。

- 当前工作包：P6 已完成，P5.4 应用几何与变换接入验收已关闭。
- 使用入口：构建 3d 运行库；运行 examples/xge_3d_integration 的 full-dev 综合范例，或按 docs/3D.md 在应用中使用公开 C API。本期没有待完成的必需项，后续游戏规则保持应用边界。
- 实现证据：P0 五种 profile、65 个控件及服务闭包链接检查、默认兼容及单控件增量通过。P1 的 3d、ui-min+3D、full-dev CPU/GPU 检查均通过；立方体遮挡、近裁剪、负缩放、超过 65535 的索引、资源引用/失败更新及离屏尺寸变更通过。真实 XUI 和失败后的 2D 像素结果正常，有限帧公开 API 范例通过。新增 TU 单文件增量 0.64 秒，另两对象复用；3d 去符号运行库 687,564 B。源码哈希、命令、构建信息和截图见 `artifacts/xge-3d/p1-evidence.json` 与 `p1-*.png`。
- P2 证据：固定 cgltf v1.15 MIT；glTF/GLB/外置与内嵌图像、无索引和 sparse、资源提供者和失败释放通过。材质按用途共享贴图，Mask/双面/基础透明、UV1 与变换、射线命中所属实例、天空方向与深度正确；full-dev 真实 XUI 前后绘制通过。P2 库 789,698 B，关闭模型为 704,522 B。P3.1 当前库 794,859 B，法线与金属/粗糙实际响应通过。
- P3 证据：直接光照/法线与 MR 响应、1～4 级太阳阴影及混合交界、微小移动像素一致、Alpha Mask/PCF、两个聚光投影上限、错误恢复通过。C 离线 IBL 生成与实际金属/粗糙/AO/LOD 响应通过；full-dev 与默认烟雾回归通过。当前 3d 为 814,345 B，IBL-off 为 812,297 B，lighting-off 为 790,722 B，model-off 为 729,169 B。光照范例有限帧通过，普通帧上传 0 B。记录源码与构建版本于 p3-evidence.json。
- P4 证据：glTF Skin/独立实例、稀疏绑定矩阵、多 primitive、权重截断报告和坏数据拒绝；STEP/LINEAR/CUBICSPLINE、双层/遮罩/挂点、循环事件、根运动及失败不发布通过。真实 GPU 独立动作、变形拾取与蒙皮阴影通过；3d/full-dev 全套回归通过。C FBX/BVH 转换与 JSON 人形映射完成，固定 CC0 KayKit 六类动作可重复转换并驱动 T/A 姿态、不同局部轴和骨长的两种角色；解析坐标、完整片段骨长与调色板验证通过。3d 为 837,385 B，关闭动画恢复 814,345 B；离线 ufbx/xrt JSON 不进入 DLL。p4-*.log、p4-walk-demo.png、p4-mix-demo.png、p4-evidence.json 记录证据。
- P5.1 与 P5.4 核心证据：静态/关节保守 AABB、更新失效、射线/重叠、容量及隐藏节点查询通过；1e9 坐标差分、原点往返及失败回滚通过。82 网格的屏幕外投影与动画进入视锥正确，原点改变前后阴影像素一致。120 静态实例含索引/无索引、Mask 和负缩放，主/太阳提交由 242 降至 8，像素一致、静止帧 0 B 上传。四级配置、资源持有、两级实际绘制/阴影与稳定拾取通过。3d/full-dev 全套回归通过，3d 为 856,329 B。p5-spatial.log、p5-culling.log、p5-instances.log、p5-scale-suite.log、p5-scale-full-dev.log 与 p5-scale-evidence.json；文档解释外部模拟/导航接入。
- P5.2 证据：新模块约 190 行 C，复制 uint16/float 行数据，固定块及 1/2/4/8 格网格、边缘裙边、普通节点/LOD/材质复用。细小 uint16 差分、不完整边块、复制/保留生命周期、实际对角三角形高度/法线与表面/裙边射线一致。强制混合 0/1/2 级时 7,221 内部采样像素无空洞，太阳/聚光地形阴影通过，P3 交叉验收关闭；3d/full-dev 全套回归通过。3d 为 865,545 B；Terrain-off 恢复 856,329 B，TU、八个声明/导出消失且实际静态 GPU 验证通过。p5-terrain-*.log、p5-terrain-lod-shadow.png 与 p5-terrain-evidence.json 保存证据。裙边遮挡空洞，近处强制粗级别仍可能出现台阶，文档明确限制。
- P5.3 证据：一个 xrt 工作线程、SlotMap/Array/Mutex/Cond/Atomic；CPU 读取解析和 GPU 提交分离。取消立即禁止发布，代次保护旧任务，100 次外部资源加载释放、错误与线程约束通过。8,940 B 模型在 32 次预算提交后与同步画面逐字节一致；纹理整行/像素、mip、VBO/IBO 续传，上传中取消、超限失败清理、共享字节/操作额度及 GL 解包/PBO 恢复通过。3d/full-dev 全套与当前默认声明/导出/烟雾回归通过；3d 为 873,737 B，关闭异步恢复 865,545 B 且 TU、八个声明/导出消失。p5-async-*.log/p5-async-upload.png/p5-async-evidence.json 保存证据。时间预算为操作间软限制，阻塞 IO 的取消清理需等返回；后续渲染器及姿态资源不计入模型上传额度，文档已说明。
- P6 证据：综合范例在 3d/full-dev 各完成 360 帧，十五项预算资源、六类动作/两骨架、真实 XUI、720 次应用地面查询、静态三角形导出及移动/删除/原点重定位通过。各 1,200 轮资源压力、3,320 个记录帧，后三百至一千二百轮 CRT 忙堆字节/块数不变；固定场景提交 401→74，记录真实 GPU elapsed query、CPU 提交、帧间隔、加载墙钟与峰值工作集。七个子模块逐项和全关的八种配置，六种顶层 profile、65 个控件/服务裁剪，旧默认 5,232 导出/17 结构布局/体积/烟雾回归及 0.59 s 单 TU 增量通过。3d/full-dev 最终全套通过；修复离线重定向测试入口变量覆盖并实际执行。p6-*.log、p6-pressure-*.json/csv、p6-integration-*.png、p6-default-compatibility.json、p6-incremental.json 与 p6-evidence.json 保存证据。
- 平台边界：Windows 原生必验完成；实测驱动报告 GL 4.3。其他原生平台、GLES/WebGL2 未执行，已在交付说明明确，不能宣称已验。物理与导航保持实际几何/变换/查询和应用范例，完整后端属于扩展范围。

## 阶段状态

| 阶段 | 工作包 | 状态 | 通过证据 |
| --- | --- | --- | --- |
| P0 裁剪与快速构建 | P0.1 至 P0.4 | 已验证 | p0-*-verification.json、control-matrix/summary.json、p0-incremental.json、p0-hot-build.json；默认烟雾回归通过 |
| P1 3D 底座 | P1.1 至 P1.3 | 已验证 | p1-evidence.json；CPU/GPU、full-dev 混合与默认烟雾回归通过 |
| P2 模型与静态场景 | P2.1 至 P2.3 | 已验证 | p2-complete-suite.log、p2-full-dev.log、p2-p3-cross-suite.log；共享材质/UV1、法线响应、拾取与天空六面像素通过，公开场景范例有限帧通过 |
| P3 光源与阴影 | P3.1 至 P3.4 | 已验证 | P3 静态证据及 p4-skin-animation-shadow.log、p5-instances.log、p5-terrain-suite.log；蒙皮/实例化/地形交叉通过 |
| P4 骨骼与动作素材 | P4.1 至 P4.4 | 已验证 | p4-complete-suite.log、p4-full-dev.log、p4-motion-validation.log、p4-retarget-validation.log、p4-retarget-runtime-fullclip.log、p4-animation-off-verification.log；两种实际不同骨架与六类外部动作通过 |
| P5 规模与功能支撑 | P5.1 至 P5.4 | 已验证 | p5-scale-*、p5-terrain-*、p5-async-*；p6-integration-* 关闭应用接入 |
| P6 整体验收与交付 | P6.1 至 P6.4 | 已验证 | p6-evidence.json、p6-*-suite.log、p6-pressure-*、p6-integration-*、p6-profile-regression.log、p6-control-matrix.log、p6-3d-trim-*、p6-default-*、p6-incremental.* |

## 构建与体积基线

由 P0.1 实测填写。记录编译器、优化参数、硬件、工作区版本和动态依赖；各次比较使用相同条件。

| 配置 | Release 去符号运行库 | 冷构建 | 热构建 | 单文件增量 | 预算与依据 |
| --- | --- | --- | --- | --- | --- |
| 当前默认 | 8,270,784 B | 基线 138.19 s（HB 已缓存） | 约 0.2 s | 聚合旧入口影响范围较大 | 兼容性基线，新增 3D 默认关闭 |
| core | 672,176 B | 12.90 s | 约 0.1 s | 旧核心仍为聚合 TU | 1 MiB |
| 2d | 723,404 B | 18.88 s | 约 0.1 s | 旧核心仍为聚合 TU | 1.5 MiB |
| 3d（P0 底座） | 672,176 B | 约 13 s | 约 0.1 s | P1 独立 TU 再验证 | 完成本期目标暂定 5 MiB；超过时分析贡献，不能静默提高 |
| ui-min | 3,230,247 B | 102.94 s（包含 HB） | 0.19 s | Button 单文件 1.02 s | 4 MiB，文本为主要成本 |
| full-dev（P0） | 8,270,784 B | 182.63 s（并行条件） | 约 0.3 s | 控件独立编译 | 原兼容体积加新增 3D 成本，P6 分项记录 |

以上冷构建条件含缓存和并发差异，不作为严格提速倍数。编译器 GCC 16.2.0、`-O2`；未去符号默认基线 10,153,102 B。动态依赖与编译参数见各 `p0-*-verification.json` 和 build-report。离线工具及测试资源不计入运行库。

物理/导航选型结论：本期不引入完整后端，避免新增重型依赖；P5 使用真实几何、变换与查询接口给出应用接入验证。完整模拟、角色控制和寻路保持扩展边界。

## 已确定的边界

- 裁剪与快速构建是前置门槛；新增 3D 优先独立编译。
- 新增代码与新依赖采用 C，通用数据结构复用 xrt；既存文本依赖按实际配置记录。
- 游戏世界策略、建造、物品、存档和联网留在应用项目。
- 运行库采用 glTF 与 GLB；FBX、BVH 和人形重定向进入独立离线工具。
- 不做完整物理、导航和高级渲染架构；本期必需范围以执行文档为准。
- 默认构建保留旧能力；3D 开发使用隔离的精简 profile。

## 每轮更新格式

用简短记录替换当前摘要，证据日志存入 `artifacts/xge-3d/`：

```text
工作包：
完成的实际行为：
涉及文件：
验证命令与配置：
退出码和证据路径：
对应源码版本：
未解决问题或交叉验收：
下一条可执行动作：
```

## 最终交付检查

- [x] P0 至 P6 必需工作包均已验证，交叉验收关闭。
- [x] 裁剪同时作用于源码、声明、导出、资源和第三方依赖。
- [x] 增量构建与相关测试路径可用，未混用不同 profile 的产物。
- [x] 综合范例实际运行并使用公开 API。
- [x] 外部动作独立加载与两种目标骨架的重定向通过。
- [x] 运行库、动态依赖、离线工具和资源大小分别记录。
- [x] 默认配置相关回归通过；既存失败与本任务回归分开记录。
- [x] 平台验证和功能限制如实记录，发布使用说明与代码一致。
