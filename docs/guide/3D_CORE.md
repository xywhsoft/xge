# 通用 3D：碰撞、缓存与节点

这三项补充常见游戏都会使用的机制，保持 C API、XRT 数据结构和独立功能裁剪。接口声明见 `include/xge.h`，可运行范例见 [xge_3d_core](../../examples/xge/xge_3d_core/README.md)，本轮进度与验收见 [交付记录](../plans/xge-3d-core/PROGRESS.md)。

## 构建与裁剪

| 数字宏 | 默认值 | 作用与依赖 |
| --- | --- | --- |
| `XGE3D_ENABLE_SCENE` | 跟随 3D | 名称、用户数据、层级读取、TRS 读取、子树克隆和描述表装配；基础场景/节点 API 仍属于 3D 底座。 |
| `XGE3D_ENABLE_COLLISION` | 跟随 3D | 独立碰撞体、重叠/扫掠/射线、碰撞 BVH。 |
| `XGE3D_ENABLE_CACHE` | 跟随 MODEL | 显式模型与纹理缓存，依赖 MODEL。 |

三个模块分别编译，设为 0 时去除对应源码、公开声明和导出。关闭 ASYNC 仍可使用同步缓存，不创建工作线程。关闭 MODEL 时，SCENE 与 COLLISION 仍可处理程序化网格。

```bat
python tools/build/build.py 3d build/my-3d --define XGE3D_ENABLE_ASYNC=0
python tools/build/build.py 3d build/my-renderer --define XGE3D_ENABLE_SCENE=0 --define XGE3D_ENABLE_COLLISION=0 --define XGE3D_ENABLE_CACHE=0
```

消费者必须强制包含对应 SDK 的 `xge_build_config.h`，并链接、运行同目录库。坐标、上下文线程、引用和错误码约定沿用 [3D C API](3D.md)。

## 独立碰撞体与查询

`xge3dNodeSetCollider` 在节点绑定一个球、盒、Y 轴胶囊或静态三角网格；传 NULL 移除。绑定复制形状参数并保留网格引用，失败保留原绑定。它独立于可见网格，隐藏物体仍参与碰撞；使用 `NodeSetLayers` 的 query layers，过滤器可排除整个自身子树。模型、地形实例不会自动生成碰撞绑定。

```c
xge3d_shape_t wall = xge3dShapeDefault(XGE3D_SHAPE_BOX);
wall.half_extents = (xge3d_vec3_t){2, 1, .25f};
int result = xge3dNodeSetCollider(scene, wall_node, &wall);

xge3d_shape_t body = xge3dShapeDefault(XGE3D_SHAPE_CAPSULE);
body.center.y = 1;
body.radius = .25f;
body.half_height = .7f; /* Segment half-length, excluding the two caps. */
xge3d_mat4_t world;
if (result == XGE_OK)
    result = xge3dNodeGetWorldMatrix(scene, player, &world);
xge3d_collision_filter_t filter = {UINT32_MAX, player};
xge3d_collision_hit_t hit;
if (result == XGE_OK)
    result = xge3dSceneSweepShape(scene, &body, &world,
        (xge3d_vec3_t){.1f, 0, 0}, &filter, &hit);
/* OK: hit.fraction is the first contact in [0,1]. NOT_FOUND: full motion is clear. */
```

球/盒/胶囊均可作为移动查询形状，与全部四类碰撞体测试。查询矩阵和节点世界矩阵必须为有限、非奇异仿射矩阵；旋转、反射、非均匀缩放和剪切应用于完整形状，例如非均匀缩放球得到椭球。形状尺寸在自身局部空间，查询位置、命中位置和移动向量相对当前场景原点，距离单位米。

- `SceneOverlapShape` 输出重叠节点；NULL/0 查询数量，容量不足返回前缀、总数与 BUFFER_TOO_SMALL，顺序未指定。
- `SceneSweepShape` 返回最近接触；`fraction` 是移动比例，`distance` 是已移动的米数，`normal` 朝向查询形状，`triangle` 对网格是三角形序号，其他类型为 SIZE_MAX。
- `SceneCollisionRaycast` 测试显式碰撞体，方向无需归一化，最大距离须为有限正数。原有 `SceneRaycast` 继续拾取可见渲染几何。
- `NodeGetCollider` 的网格为借用引用；`SceneCollisionStats` 报告碰撞体数量、去重后的三角 BVH 内容字节与累计三角测试次数。

场景碰撞 BVH 随节点变换局部重拟合，结构变化重建；静态三角 BVH 在网格第一次查询时建立，同一网格的实例共享。MeshUpdate 使旧索引失效，下次查询重建。首次构建同步执行，较大场景应在加载阶段用查询预热。

当前边界：扫掠只处理线性平移；接触容差约 `1e-5` 米。初始重叠或接触返回比例 0、`started_overlapping=1`，此时法线可能是回退值，不提供穿透深度。静态网格按双面三角形表面处理，不判断封闭体内部，也不跟随骨骼变形。查询错误和 NOT_FOUND 应分别处理；移动停止、滑动、阶梯、重力与刚体求解由应用或外部物理库完成。范例保留 1 mm 移动余量，避免下一帧从接触状态开始。

## 显式资源缓存

`xge3dCacheCreate` 创建由调用线程拥有的缓存。没有隐藏全局缓存；同步调用不创建线程，第一次 CacheRequest 才创建已有 Loader 工作线程。GPU 发布、释放还要求拥有原上下文，CacheFree 应在上下文销毁前执行。

```c
xge3d_cache_desc_t desc = xge3dCacheDefault();
desc.max_models = 32;
desc.cpu_bytes = 64u * 1024 * 1024;
desc.gpu_bytes = 128u * 1024 * 1024;
xge3d_cache *cache = NULL;
xge3d_model *model = NULL;
int result = xge3dCacheCreate(&desc, &cache);
if (result == XGE_OK) result = xge3dCacheLoad(cache, "assets/actor.glb", &model);
if (result == XGE_OK) result = xge3dModelInstantiate(scene, model, parent, &root);
xge3dModelFree(model); /* The scene and cache still hold their own references. */
/* Later, on the owner/context thread: xge3dCacheFree(cache). */
```

CacheLoad 返回调用者拥有的模型引用，相同键复用同一模型。共享模型、材质和纹理按只读资产使用；节点颜色、材质替换、变换和骨骼姿态独立。CacheFree 只释放缓存持有的引用，场景和调用者仍保留的模型继续有效。

普通路径及 `file://` 采用绝对路径加词法清理作为键，能合并 `./`、`../` 等写法；不承诺识别符号链接或文件名大小写等价。自定义提供者需设置 `XGE3D_CACHE_RESOURCE_PROVIDERS`，键逐字节匹配。提供者注册表在 CacheFree 完成前须保持稳定；回调线程规则沿用 Loader，提供者用户数据还须存活到所有外部模型引用释放。

开启 ASYNC 后，使用 CacheRequest → 每帧 CachePump → CacheStatus → CacheTake。参数与 Loader 的预算、状态结构共用；Take 得到拥有引用并使客户端句柄失效。每个客户端有独立代次句柄，即使同键也可以独立取消和释放；取消一个不会破坏其他订阅者。最后一个有效订阅者取消底层工作，键立即允许重新请求，旧工作不会发布到新请求。失败也可重试，旧失败句柄保留终态。失败、取消或不用的句柄须 CacheRelease；继续 Pump 以清理后台工作，或在退出时 CacheFree 等待回收。

默认最多 64 个缓存模型、256 个客户端句柄；max_requests 上限 65,536，底层同时未回收的独立加载工作最多 1,024。相同 URI 请求合并 CPU 解码和 GPU 上传；CacheLoad 遇到正在加载的同键会同步等待并驱动它完成，帧循环宜使用异步接口。

缓存发布模型时按像素、尺寸、sRGB 角色和采样器去重纹理，并进行内容比较确认。不同 URI 的文件仍分别解析；它们的纹理在发布时共享，异步上传可能已发生重复传输。去重哈希、发布和清理是同步 CPU 工作，不计入 Loader 的上传时间/操作预算。

`CacheTrim` 按 LRU 清理未使用模型直到满足软目标；`CacheClear` 清理所有未使用模型。模型被场景、调用者或请求持有时不强制驱逐，因此预算可暂时超出，`CacheStats.over_budget` 明确报告。cpu_bytes/gpu_bytes 为 0 表示该字节目标不限；max_models 为 0 选默认值。需要硬性加载配额时传入已有的 `decode_budget`，缓存保留该预算对象。

`CacheStats` 统计去重后的保留资产存储、GPU 所需字节估计、命中/合并/驱逐次数；不等于进程内存或显存驻留，不包含缓存索引、正在解码的峰值、场景实例、碰撞 BVH、分配器及驱动开销。

## 节点组织、克隆与描述表

`NodeGetParent`、`NodeChildCount/At` 读取层级，零父句柄表示场景根集合；遍历顺序未指定，结构修改后应重新遍历。原有 `NodeGetLocalMatrix` 保留完整局部矩阵；新增 `NodeGetTransform` 返回等价 TRS，遇到剪切或奇异矩阵返回 UNSUPPORTED 且不修改输出。反射可能改以另一组等价旋转/缩放表示。

`NodeSetName` 复制字符串，NULL/空串清除；`NodeName` 返回借用字符串，在改名或删除后失效。NodeFind 在指定子树中包含根节点查找，零根表示整个场景，按 UTF-8 字节匹配；重名时不承诺选中顺序。启用 SCENE 后导入 glTF 节点也复制源名称。名称上限含结尾零字节 1 MiB。

`NodeSetUserData/NodeUserData` 仅存取应用借用指针，不释放其内容。

```c
xge3d_node_t copy;
int result = xge3dNodeClone(scene, source, new_parent, 0, &copy);
if (result == XGE_OK) result = xge3dNodeSetName(scene, copy, "second-actor");
```

Clone 复制层级、名称、局部矩阵、可见性、层、颜色、灯光、LOD、碰撞绑定和模型/蒙皮绑定，共享不可变网格、材质和模型；失败删除全部新节点。默认清空用户数据，显式传 `XGE3D_CLONE_USER_DATA` 才复制指针。新父节点下保留局部矩阵；根到根克隆保留双精度全局位置。

蒙皮关节必须全部位于克隆子树内，否则返回 UNSUPPORTED 并回滚。新蒙皮与姿态独立，应用为克隆模型新建 Animator；不会复制 Animator 的播放状态或事件。源子树删除后，克隆仍持有资产和自己的关节。

`SceneLoad` 接受轻量 C 描述表，应用可以从自己选择的文件格式填表，不增加引擎专用磁盘格式或解析器。每项包含名称、TRS、父索引、网格/材质、隐藏标志，以及可选的 glTF 模型 URI。最多 65,536 项；父索引必须小于当前索引，SIZE_MAX 指向返回的包装根。mesh/material 作用于该项节点，材质不会递归覆盖模型后代。

```c
static int resolve(const char *uri, xge3d_model **out, void *user)
{ return xge3dCacheLoad(user, uri, out); }

/* Inside a function; initialize TRS/default parent correctly. */
xge3d_scene_node_desc_t items[2];
items[0] = xge3dSceneNodeDefault();
items[0].name = "group";
items[1] = xge3dSceneNodeDefault();
items[1].name = "actor";
items[1].parent = 0;
items[1].model_uri = "assets/actor.glb";
items[1].transform.position.x = 3;
xge3d_scene_desc_t desc = {items, 2, resolve, cache};
int result = xge3dSceneLoad(scene, (xge3d_node_t){0}, &desc, &root);
```

模型解析器须返回拥有引用；未指定回调时直接 ModelLoad。先验证父索引/变换并解析模型，再装配不可见子树，全部成功后发布；错误回滚子树及临时模型引用。调用是同步的，大模型可先通过缓存异步加载，READY 后再装配；非常大的层级继续使用已有分步实例化 API。

## 验证与范围

```bat
python tools/build/build.py 3d build/core-next/3d
python tests/xge/make_3d_fixtures.py
python tests/xge/test_core_next.py build/core-next/3d
python tests/xge/test_core_next.py build/core-next/3d --gpu
python tests/xge/test_foundation.py build/core-next/3d cpu spatial terrain import animation async gpu import_gpu material_gpu sky_gpu light_gpu shadow_gpu ibl_gpu skin_gpu culling_gpu instancing_gpu terrain_gpu async_gpu fog_gpu foundation_base foundation foundation_gpu local_matrix mesh_prepare_gpu
python tests/xge/check_3d_trimming.py SCENE COLLISION CACHE ASYNC MODEL BARE --output-root build/core-next/trimming --evidence-root artifacts/xge-core-next --evidence-prefix trim
python examples/xge/xge_3d_core/build.py --run --automatic --frames 360 --capture artifacts/xge-core-next/demo.png
```

2026-10-25，Windows 原生 GL：3d/full-dev 各 28 组 CPU/GPU 回归通过，六种裁剪审计通过；保留本轮基线全部 916 个导出，新增 32 个，27 个代表性旧公开结构 sizeof 相同。后者是局部 ABI 检查，不是所有结构偏移和二进制兼容性的完整证明。扩充后的 CPU 用例在两种配置及适用裁剪配置再次通过。

同工具链纯 3D 去符号 DLL 从 925,675 B 增至 966,123 B，增加 40,448 B（约 39.5 KiB）。包含已有所有模块的 full-dev 为 8,985,745 B；本轮不以它推断 XUI 的增量。结果与日志在 `artifacts/xge-core-next/verification.json`，构建输出隔离在 `build/core-next/`。未验证 GLES/WebGL2 和其他操作系统，未运行本轮全部 XUI/default 配置回归。
