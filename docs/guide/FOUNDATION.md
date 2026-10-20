# 通用 3D 基础增强（2026-10-18）

本轮保留 XGE 的图形与应用基础库边界，运行代码使用 C 和已有 XRT。没有加入服装槽位、房间/建造规则、伤害、导航解算或 MMO 服务。原有公开描述结构未增加字段；新增配置、统计和工作接口独立提供。

## SDK 身份

`xgeGetBuildInfo` 返回版本、基础接口修订、编译功能位和 SDK 指纹。结构需初始化 `size`。构建器根据公开头文件、功能配置和编译器版本产生 `XGE_BUILD_ABI_FINGERPRINT`，同时写入 DLL 和配套 `xge_build_config.h`。

```c
xge_build_info_t info = {.size = sizeof(info)};
int result = xgeGetBuildInfo(&info);
if (!result)
    result = xgeCheckBuildCompatibility(XGE_BUILD_ABI_FINGERPRINT, XGE_FEATURES);
```

这是一项主动检查，不会替调用方自动选择 DLL。使用生成的 SDK 配置并在启动时检查；未使用生成配置的源码构建指纹为零，只能做功能位检查。不同编译器、配置或公开头文件会产生不同指纹，即使其中部分组合实际上 ABI 兼容，也保守拒绝。

## 场景增量更新与空间查询

节点维护父子邻接关系和稠密数组索引。局部矩阵未变化时无操作；变化时只标记相关子树，世界矩阵按需计算。骨骼到蒙皮的依赖显式登记，关节变化只使其使用者的姿态和包围盒失效。不再用全场景版本使所有静态物体的缓存失效。

`SceneQueryAabb` 和 `SceneRaycast` 使用平衡 BVH；旧接口保留“仅可见物体”语义。`SceneQueryAabbEx`、`SceneRaycastEx` 可独立指定查询层及是否包含隐藏物体。层掩码零表示不匹配，NULL 过滤器表示所有层、排除隐藏。输出顺序未规定，不能依赖早先数组排列。

```c
xge3d_query_filter_t filter = {UINT32_MAX, 1};
xge3d_hit_t hit;
int result = xge3dSceneRaycastEx(scene, &ray, 100, &filter, &hit);
```

静态索引初建，以及增删节点/更换网格后的结构重建是同步 CPU 工作。可在进入交互前显式调用 `ScenePrepareSpatial`；否则首次查询准备。变换和姿态变化只排队重算相关叶节点并沿路径重拟合，不重排整树。共享网格 Update 会使场景在下次查询重建，以覆盖不经过节点接口的几何修改。当前渲染候选收集仍线性遍历，BVH 不意味着整条渲染路径变成次线性。

`SceneStats` 报告世界矩阵/包围盒计算数、空间重建/重拟合数、查询访问和销毁数；可读取后复位。`spatial_bytes` 是索引已用内容大小，不是分配器保留容量。

## 分步装配与回收

`ModelInstanceBegin/Step/Take/Free` 提供隐藏的暂存实例。Step 按 `xge3d_work_budget_t` 的节点操作数和软时间限制推进节点创建、图元接入、父子连接和蒙皮建立；Take 才整体发布。原 `ModelInstantiate` 沿用此实现并同步完成，旧调用者不必迁移。

```c
xge3d_instance_builder *builder = NULL;
int result = xge3dModelInstanceBegin(scene, model, parent, &builder);
/* Subsequent owner-thread frames: */
xge3d_work_budget_t work = {32, 1000};
int completed = 0;
if (!result) result = xge3dModelInstanceStep(builder, &work, &completed);
if (!result && completed) result = xge3dModelInstanceTake(builder, &root);
/* Call Free after Take, or to synchronously cancel. */
```

Builder 保留 model，借用 scene，必须在 scene 销毁前释放。失败不会发布半成品，但暂存节点保留到 Free；Free 适合取消收尾和退出，不是硬预算接口。初始映射数组分配、单个蒙皮建立、最后资源引用释放以及驱动调用不可抢占。预算限制每次操作数量，不保证操作一定在指定微秒内完成。

`NodeDestroyStep` 首次隐藏并锁定整棵子树，再按预算后序删除叶节点。子树的借用句柄从首次调用起不可操作；根句柄只用于继续 Step，直到 `completed=1`。预算节点数零暂停删除但仍锁定/隐藏。SceneFree 会清理未结束的回收。与已有 Destroy 一样，GPU 资源必须在所属上下文线程回收。

节点销毁不再为每个对象全场景搜索数组位置；模型源节点查询也改用实例映射。工坊仍负责文档及素材 ID、版本、缓存淘汰和跨文档共享政策。

## GPU 准备、资源共享与统计

已有 TexturePrepare/MeshPrepare/LoaderPump 继续负责受预算上传。启用 ASYNC 时，新增 RendererPrepare 可预建阴影图集，NodePrepareSkin 可预建/更新蒙皮矩阵纹理，避免把这些首次工作全部留给 Render。零操作或不足字节预算不会上传；已完成资源不会重复上传。所有操作恢复改动的 GL 状态。

Mesh/Texture/Material/Model 的 AddRef 允许宿主显式保留公共资源。它们不新增线程安全保证，不能跨独立 GL 上下文复用已上传资源。引用和缓存应由宿主管理，不依赖隐藏的全局资源库。

MeshResourceInfo、TextureResourceInfo、ModelResourceInfo 返回顶点/索引/图片的 CPU/GPU 内容字节及引用数。GPU 大小包含已分配内容和可估算的 mip，不代表真实显存驻留，也不含驱动保留、解析器和分配器开销。不同资源统计重叠时，宿主应按资源身份去重。

## 独立蒙皮与动画

`NodeBindSkin` 为网格节点复制显式关节顺序和 inverse-bind 矩阵，验证关节归属、权重和索引。NULL 解除蒙皮。失败保留旧绑定。多个独立衣服网格可绑定到同一组关节节点并共享一个 Animator，不需要逐件重放动作。

此接口不自动适配骨长、衣服版型或参考姿势。调用方完成兼容映射；关节被删除后，仍引用它的蒙皮返回 INVALID_STATE，调用方应先解绑/更换衣服。不同网格的局部空间和 inverse bind 可能不同，因此各自保留蒙皮矩阵纹理，不声称共享所有 GPU palette。

动画支持 0～7 层。默认仍为覆盖模式，旧 0/1 层切换语义保留。SetLayerMode 可选择相对目标资产参考 TRS 的加法模式：平移加参考差，旋转乘加权相对四元数，缩放乘相对比例。参考缩放零且动作缩放非零时失败。Mask、事件与根运动按层序工作；覆盖层会衰减更低层的根运动，加法层不衰减。

GetPose 读取已成功发布的本地 TRS；SetPose 用于 Update 后、Render 前的约束修正。下次 Update 重新采样。这样独立 IK/瞄准模块可接入，但 XGE 不决定脚接触、武器握持或战斗状态。Update 失败时已发布姿态、时间和事件保持原状。

## 灯光、视图与性能剖析

RendererOptionsDefault 初始化所有层、8 个直接光源预算和关闭 GPU 剖析。SetOptions 可分别设置相机与阴影层，以及 1～8 个直接光预算。NodeSetLayers 分别设置本节点的绘制、查询与投影层，父级普通 visibility 仍影响后代。

超过预算时不再整帧报错。方向光优先，其他灯按相机距离与强度估计选择；有限范围且不与视锥相交的灯先排除。不是逐物体/分簇光照，仍存在 8 个直接光着色槽。阴影对选中灯限制为一个太阳及两个聚光，其余继续照明但不投影。选择是确定性的，分数相同保留先遇到的灯；高密度灯光切换不做淡入淡出。

RenderProfile 返回 CPU 总计、候选收集、阴影和提交耗时，以及候选/选择/跳过灯数。启用 profiling 后，桌面 GL 3.3+ 可用时间戳环形查询返回较早一次 Render 的 GPU 命令区间时长，`gpu_sample` 标识对应调用。只查询已完成结果，队列忙时跳过采样，不等待 GPU，不做像素读回。不支持的后端返回 `gpu_available=0`，不能把零当作 GPU 耗时。

## 验证入口

```bat
python tools/build/build.py 3d build/foundation-20261018/3d
python tests/xge/test_foundation.py build/foundation-20261018/3d
python tests/xge/test_foundation.py build/foundation-20261018/3d foundation_gpu
```

专项覆盖随机空间查询与直接包围盒对照、静态缓存隔离、共享网格更新、深层级、部分回收、装配取消、蒙皮绑定回滚、跨文件动作、加法层、材质/视图隔离、40 灯筛选、非阻塞 GPU 样本与原三维 CPU/GPU 回归。实际执行结果见 `artifacts/xge-foundation/verification.json`；未验证的能力不得从接口存在推断完成。

大城市全帧预算、真实五万素材交互、大规模丧尸、完整 MMO、骨骼 LOD、布料、morph、HDR、多物理线程修改场景，以及 GLES/WebGL 实机支持，均不是本轮验收结论。

## 本机验收结果

实际固定 SDK 通过 57 组引擎回归，以及四组既有工坊程序兼容性回归（569 项检查）。完整和纯三维配置分别执行 22 组，另验证裸三维、关闭动画和关闭异步的 13 组。六种配置的声明/导出一致，保留基线 4427 个公开导出，17 个既有三维结构的 sizeof 未改变。

长循环执行 1200 轮：300 次立即取消、300 次纹理途中取消、300 次网格途中取消、300 次完整加载并创建/销毁两个动画实例。共记录 3320 帧；第 300～1200 轮 CRT 忙堆字节和块数均无增长。这不证明任意规模或驱动下无泄漏。

同一 6000 个合成静态物件的旧 API 负载各采样五次，中位数如下。这里的操作总耗时不是单帧或游戏 FPS，首次 BVH 构建不计入查询对照。

| CPU 操作 | 基线 | 本轮 |
| --- | ---: | ---: |
| 无关节点移动后，25 轮逐物件包围盒读取 | 4971.998 ms | 5.235 ms |
| 1500 次局部区域查询 | 220.089 ms | 1.473 ms |
| 整棵 6000 物件子树销毁 | 38.208 ms | 0.483 ms |

配套可复制开发包位于 `build/foundation-20261018/sdk`，含真实的配套公开头文件、配置、导入库、DLL 和 SDK 检查样例，不依赖源码目录联接。已在另一个含空格及中文的目录编译并运行样例。C 项目可将 XGE_ROOT 和 XGE_PROFILE 均指向该目录。

工作区同时存在其他开发。本次验收针对 `source-manifest.json` 固定的代码及 SDK，未覆盖随后发生的独立修改；报告记录了后续差异。完整 SDK 和当前工坊的正式发布目录没有被互相覆盖。
