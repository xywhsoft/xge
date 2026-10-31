# 通用游戏基础：六项轻量机制

这组接口补齐调试、碰撞响应、动态资源、着色扩展、3D 特效和输入动作。它们都是 C API，使用 xrt 存储并由调用线程串行操作。旧公开结构没有增加字段；新增描述和查询结构使用 `size = sizeof(...)`。游戏负责玩法、世界管理和物理策略。

公开接口以 `include/xge.h` 为准。[完整范例](../../examples/xge/xge_game_foundation/README.md)不需要外部素材。实施和实际验收见[进度记录](../plans/xge-game-foundation/PROGRESS.md)。

## 编译与运行成本

| 数字宏 | 默认及依赖 | 裁剪内容 |
| --- | --- | --- |
| `XGE3D_ENABLE_DEBUG_DRAW` | 跟随 3D | 调试队列、线段/形状/骨骼 API |
| `XGE3D_ENABLE_MOTION` | 跟随 COLLISION，要求 COLLISION | 穿透接触、恢复与滑动；旧碰撞查询仍保留 |
| `XGE3D_ENABLE_DYNAMIC` | 跟随 3D | 材质修改、纹理更新和目标纹理视图 |
| `XGE3D_ENABLE_SHADER` | 跟随 3D | 着色扩展对象、参数和程序变体 |
| `XGE3D_ENABLE_BILLBOARD` | 跟随 3D | Billboard 队列和批绘制 |
| `XGE3D_ENABLE_PARTICLES` | 跟随 BILLBOARD，要求 BILLBOARD | 有限容量 CPU 3D 粒子独立编译单元 |
| `XGE_ENABLE_INPUT_ACTIONS` | core 默认关闭，其他配置默认开启 | 通用输入动作；与 2D/3D 相互独立 |

全部取值为 0/1。关闭 3D 自动关闭相应模块，关闭 COLLISION/BILLBOARD 自动关闭其默认依赖。显式开启不满足依赖的模块会编译报错。库与消费者使用同一个生成的 `xge_build_config.h`。

调试和 Billboard 队列默认空，GPU 程序/VBO/VAO 首次绘制时才创建；空队列不产生额外绘制。队列清空后保留已分配容量，RendererFree 释放。自定义 Shader 只有显式创建时才编译。动作和粒子只在创建时分配容量，正常更新不分配；Billboard 的双缓冲队列预热后复用存储。目标纹理视图共享 GPU 存储，不读回、不重新上传图像。

```bat
python tools/build/build.py 3d build/my-game --define XGE3D_ENABLE_DEBUG_DRAW=0 --define XGE3D_ENABLE_PARTICLES=0
tests\xge\build_3d_suite.bat game-foundation 3d
python examples/xge/xge_game_foundation/build.py --run
```

## 调试绘制

`DebugLine`、`DebugBox`、`DebugAxes` 的完整前缀为 `xge3d`。Box 接受局部 AABB 和仿射矩阵。`DebugShape` 跟随 COLLISION，绘制球、盒、胶囊或静态网格；球/胶囊使用 32 段近似。`DebugSkeleton` 跟随 ANIMATION，传入有蒙皮绑定的网格节点，绘制所属关节与最近祖先关节之间的连线。

```c
float green[4] = {0, 1, 0, 1}; /* Straight linear RGBA. */
xge3dDebugClear(renderer);
int result = xge3dDebugLine(renderer, hit.position,
    (xge3d_vec3_t){hit.position.x + hit.normal.x,
                   hit.position.y + hit.normal.y,
                   hit.position.z + hit.normal.z}, green, 0);
```

队列持久保存，反复 Render 不会隐式清空。每帧重建时先 Clear。坐标相对当前场景原点；重定位后应用重新生成调试几何。`depth_test` 只接受 0/1，全部不写深度；测试开启的线先绘制，忽略深度的线后绘制。颜色不受灯光和雾影响，但经过曝光及最终后处理。线宽为后端标准 1 像素，每队列最多 1,048,576 条线。调用失败时形状/骨骼复合命令撤销其已追加部分。

调试绘制在三维几何和 Billboard 之后、后处理之前执行。Render 统计包含实际绘制调用，线段不计入三角形数；GL 状态通过现有 Render 保存/恢复机制处理。

## 接触、恢复与滑动

`SceneContactShape` 复用现有场景碰撞 BVH、三角形 BVH 和形状仿射变换。移动查询形状为球、盒、胶囊，目标可以再包含静态三角网格。每个碰撞体输出最深的一条接触，网格返回对应三角形索引。`count` 返回总数；NULL/0 只查询数量，短输出保留前缀并返回 BUFFER_TOO_SMALL。

`contact.depth` 沿 `contact.hit.normal` 给出将查询形状推出目标的距离，法线指向查询形状。接触容差约 1e-5 米，接触但没有穿透时深度为 0。均匀缩放球对球用解析解，其余使用双精度 GJK/EPA；EPA 存储和迭代有限，达到内部存储上限时使用保守的分离支撑平面，因此深度不承诺始终是最小平移距离。退化、零体积形状可能返回 UNSUPPORTED。静态网格仍是双面表面，不判断封闭网格内部，不跟随蒙皮变形。

```c
xge3d_motion_settings_t settings = xge3dMotionDefault();
xge3d_motion_result_t motion = {.size = sizeof(motion)};
int result = xge3dSceneMoveShape(scene, &shape, &world, desired_delta,
                                &filter, &settings, &motion);
if (result == XGE_OK) {
    /* Apply motion.displacement to the application's transform. */
}
```

MoveShape 是查询，不修改节点。先沿接触法线恢复初始穿透，再扫掠移动、对接触约束投影剩余位移。它保存最多 8 个不同接触法线，并处理墙面滑动、墙角和多平面约束。默认恢复/移动各最多 8 次迭代，允许设置 1～32；默认 margin 为 0.001 米，范围 `[0.0001,1]`，恢复路径总距离上限默认 1 米，范围 `(0,1000]`。

结果 `displacement` 包含恢复和实际移动，`recovery` 单独给出恢复量，`remainder = desired_delta - (displacement - recovery)` 包含被阻挡及未完成的原始请求。BLOCKED、RECOVERED 分别标记阻挡与恢复；LIMITED 表示恢复距离/接触数/迭代预算不足，需要应用处理剩余工作。错误返回保留旧结果；首次构建碰撞索引可能分配，应在加载阶段预热。后续静态索引查询复用已有容量。

重力、上下坡规则、自动上阶梯、移动平台、跳跃和刚体求解都由应用或外部模块决定。根运动同样可以作为 `desired_delta` 输入。推荐用小固定步长更新高速移动；普通 SweepShape 的旧行为保持。

## 动态材质、纹理与目标视图

`MaterialSetDesc` 原位修改已有材质，先验证并取得新贴图引用，全部成功后才替换旧描述。材质的 Shader 与参数保持。失败保留旧描述和引用；修改共享材质影响所有使用者。若要独立外观，先 GetDesc，再创建一个新材质绑定到目标节点。

`TextureUpdate(texture,x,y,width,height,pixels,stride)` 更新 straight RGBA8 子区域，坐标/行方向与 TextureCreate 一致。CPU 像素在成功后复制；已上传纹理在所属上下文线程做子图像更新，并重新生成需要的 mip。未上传纹理只更新 CPU 副本。更新不新增长期内存，恢复纹理绑定、PBO 和 unpack 状态；部分预热中的纹理返回 INVALID_STATE，目标视图返回 UNSUPPORTED。GPU 驱动错误可能已影响部分 GPU 内容，应用应重建该资源；不承诺驱动写入的事务回滚。

```c
xge3d_texture *view = NULL;
int result = xge3dTextureFromTarget(source_target, &view);
if (!result) {
    xge3d_material_desc_t m = xge3dMaterialDefault();
    m.unlit = 1;
    m.maps[XGE3D_MAP_BASE_COLOR].texture = view;
    result = xge3dMaterialCreate(&m, &screen_material);
    xge3dTextureFree(view); /* Material retains the view. */
}
```

视图保留 **3D Target**，跟随其 Resize，调用者释放自己的 Target 引用后仍可采样。它没有独立 CPU/GPU 图像存储，ResourceInfo 的这两个载荷为 0。TexturePrepare 对有效视图报告已完成，无上传操作。只接受 base color/emissive 两个颜色槽；该目标输出的显示编码预乘颜色会在采样时去预乘并转换到线性空间，同时处理存储方向。用 AlphaBlend 材质可保留透明内容。

Render 前检查直接目标和借用 FBO：采样正在绘制的目标返回 INVALID_STATE，避免读写反馈。采用两个目标轮换可以实现应用自己的前后帧效果。创建、Resize、采样和最终释放要求同一个上下文；没有跨上下文或任意外部 GL 纹理的隐式接管。普通 2D 纹理/2D RenderTarget 不属于这个保留型视图接口。

## 受控着色器扩展

ShaderCreate 接受两个 GLSL 函数字符串；NULL 表示对应恒等函数。每段最多 64 KiB。函数不包含版本头，库根据后端补充 GL 3.3/GLES 3.0 头与标准声明：

```glsl
void xge3dVertex(inout vec3 p, inout vec3 n, vec2 uv) {
    p.x += sin(p.y * 2.0 + xge3dParams[0].x) * xge3dParams[0].y;
}
void xge3dSurface(inout vec4 base, inout vec3 emission, vec2 uv) {
    base.rgb *= xge3dParams[1].rgb;
}
```

顶点函数输入为模型局部位置/法线和 UV0，运行在蒙皮/实例/世界变换之前。应用负责正确修改法线。表面函数取得已采样 base color 与节点颜色的乘积、材质发光因子和 UV0；执行后进行 AlphaMask，再应用发光贴图和标准 PBR/Unlit、雾、曝光。函数可以使用自己的纯函数和常量，以及四个 `vec4 xge3dParams[4]`；不通过未公开的引擎 uniform 或全局变量取数据。

```c
xge3d_shader_desc_t desc = {.size = sizeof(desc),
    .vertex = vertex_function, .surface = surface_function,
    .bounds_margin = .2f};
xge3d_shader *shader = NULL;
int result = xge3dShaderCreate(&desc, &shader);
float parameters[16] = {0};
if (!result) result = xge3dMaterialSetShader(material, shader, parameters);
xge3dShaderFree(shader); /* Material retains it. */
```

SetShader 复制 16 个参数，NULL 参数写零，NULL Shader 恢复标准程序；非有限参数或引用获取失败保留旧绑定。更新参数不重新编译、不分配。ShaderCreate 在上下文线程编译主通道和阴影变体；失败清空输出，通过现有日志输出编译诊断。

相同顶点函数和参数用于主通道及阴影，同一表面函数用于 AlphaMask，蒙皮/实例化继续使用现有路径。`bounds_margin` 是每轴局部位移的绝对上界；库结合世界和蒙皮变换扩大渲染/阴影裁剪边界。上界不足仍可能错误裁剪。几何拾取和物理查询继续使用 CPU 原始几何；需要变形碰撞时由应用更新碰撞几何。

这是一组顶点/表面扩展，不是可替换所有阶段的材质图、额外纹理绑定系统或任意后处理框架。它覆盖常见摆动、颜色变化、程序化表面和溶解；保留现有管线的透明、灯光、阴影和合成约定。

## Billboard 与 CPU 3D 粒子

BillboardDefault 返回 1×1 米、白色、完整 UV、启用深度测试的透明混合四边形。宽高为米，rotation 为面内弧度，color 为 straight 线性 RGBA，uv 为 `(u,v,width,height)`。贴图 NULL 使用纯色，目标视图同样支持。队列最多 65,536 项。

```c
xge3d_billboard_t b = xge3dBillboardDefault();
b.position = (xge3d_vec3_t){0, 2, 0};
b.color[3] = .5f;
b.blend = XGE3D_BILLBOARD_ADD;
int result = xge3dBillboardsSet(renderer, &b, 1);
```

Set 复制整张队列并保留贴图引用，失败保留旧队列；NULL/0 清空。每次 Render 按当前相机朝向生成四边形，按相机深度从远到近排序，同深度保持提交次序。相邻的相同贴图、混合和深度设置合并提交；不为每个粒子建节点或网格。Alpha 和 Add 两种模式均不写深度，Add 的 RGB 累加、覆盖 alpha 按 source-over 合成。Billboard 接受曝光、距离雾和高度雾，并进入后处理；不投射/接收阴影。

队列在普通网格透明绘制之后执行，因此与单个透明网格的相交面不做全局排序；普通不透明物体可以正常遮挡粒子。渲染顺序为网格 → Billboard → 调试线 → 后处理 → 后续 2D/UI。

ParticlesCreate 只做 CPU 分配，无需 Init/窗口。描述控制容量、随机种子、发射率、寿命/速度区间、圆锥方向、重力、阻力、首尾颜色/大小和图集序列帧。位置是应用选择的局部模拟坐标，移动发射器只影响新粒子。容量 1～1,048,576，默认 256；默认发射 30 个/秒。寿命为 `(0,3600]` 秒，spread 为 `[0,pi]`，columns/rows 各 1～256。

`Emit` 手动发射，满额时只增加 dropped，不扩大容量。Update 接受 `[0,1]` 秒，按最多 1/60 秒的小步推进，emit=0 只排空已出生粒子，dt=0 可暂停。阻力使用指数衰减，大小和颜色按寿命线性插值；相同种子、调用顺序和步长产生相同结果。没有 GPU 模拟、粒子碰撞或子发射器树。

`ParticlesBillboards` 导出到调用者缓冲，返回所需数量。输出临时借用 texture；随后 BillboardsSet 才保留引用。多个粒子系统可导出到同一应用数组后一次提交。数组的 UV 序列帧按年龄/fps 选择并循环，Info 返回 alive/emitted/dropped。模拟原点由应用选择；SceneSetOrigin 后可将导出的 Billboard 坐标转换到新场景原点，SetPosition 仍传入模拟坐标。库不自动遍历外部粒子系统。

## 输入动作

动作 ID 是应用提供的 `[0,action_count)` 索引，没有内置“移动/攻击”等语义。创建时固定动作数量和绑定容量，单个动作可绑定多个键、鼠标按钮或手柄按钮/轴。device 为手柄索引 0～3，-1 读取已连接手柄中绝对值最大的输入。

```c
xge_actions *actions = NULL;
int result = xgeActionsCreate(2, 8, &actions);
xge_action_binding_t left = {XGE_ACTION_KEY, 0, 'A', -1};
xge_action_binding_t right = {XGE_ACTION_KEY, 0, 'D', 1};
if (!result) result = xgeActionsBind(actions, 0, &left);
if (!result) result = xgeActionsBind(actions, 0, &right);
/* Once per input frame, outside a fixed-physics substep loop: */
if (!result) result = xgeActionsUpdate(actions, NULL, NULL);
xge_action_state_t move;
if (!result) result = xgeActionsGet(actions, 0, &move);
```

输入值乘 scale 后相加，抵消相反输入，限幅到 `[-1,1]`，再应用动作死区。deadzone 默认 0，范围 `[0,1)`；超出死区部分重新映射到完整幅度。down 表示值非零，pressed/released 分别表示进入/离开非零状态，不以正负方向切换作为按下/释放。

Update 的默认读取器轮询现有 XGE 输入，并尊重已消费的键。自定义读取回调可接入 UI 路由、回放或测试，必须返回有限的 `[-1,1]`；任何非法值使本次更新失败并保留所有旧动作状态。回调不应重入或修改本动作集合。需要屏蔽 UI 占用的鼠标/手柄时通过回调处理。

ClearBindings 用于重绑，会清除该动作的值与边沿；SetEnabled(0) 同样清空，重新启用后按下一次实际输入生成状态。窗口失焦时可 Reset 清空所有值/边沿，不制造释放事件。配置、Update 和 Get 在应用输入线程串行进行。动作创建上限 65,536，绑定容量上限 1,048,576；满额返回 BUFFER_TOO_SMALL，不自动扩大容量。
