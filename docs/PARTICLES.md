# XGE 2D 粒子系统

粒子系统由独立的 CPU 模拟器和 XGE 绘制适配器组成。它不依赖 Sprite 对象、动画播放器、XUI 或布局系统。公开 API 在 [`xge.h`](../xge.h) 中；七个完整程序见 [粒子范例](../examples/xge_particles/README.md)。

## 已有能力与边界

| 类别 | 当前支持 |
| --- | --- |
| 发射 | 按秒、按移动距离、定时 Burst、手动 Emit；延迟、有限时长、循环、多发射器组合。 |
| 形状 | 点、线段、矩形、圆盘、圆环、扇形 Cone；圆盘/圆环按面积均匀采样。 |
| 初始属性 | 生命周期、速度、尺寸、旋转、角速度、起始帧的随机范围，以及两种颜色间随机插值。 |
| 生命周期属性 | 尺寸、移动速度、透明度曲线；颜色渐变；线性、阶梯、Hermite 曲线。 |
| 运动 | 重力、指数阻力、径向/切向加速度、平滑二维噪声、发射器速度继承。 |
| 空间与外观 | 世界/本地空间、速度朝向与拉伸、宽高比、纹理子矩形、网格序列帧、屏幕空间、混合模式、层级。 |
| 实例管理 | 不可变定义、多实例、随机种子、倍率覆盖、暂停/重播/排空/清空、预热、包围盒、可见性策略、实例缓存。 |
| 事件与碰撞 | 出生/死亡/碰撞/结束事件；前三种事件可触发子发射器；平面、矩形或应用碰撞回调；反弹/销毁。 |
| 工程支持 | JSON/XSON 配置、资源 URI 加载、容量上限、统计、XGE 批绘制、自定义材质、无窗口测试和像素回归测试。 |

这是单线程 CPU 粒子实现，不包括 GPU 模拟、粒子间碰撞、刚体求解、连续拖尾网格、Ribbon、粒子编辑器或 Sprite/Animation 资源模型。纹理网格只负责粒子的 UV 变化，不是另一个动画播放器。

## 最小 CPU 用法

模拟、定义解析和曲线求值不需要调用 `xgeInit`：

```c
#include "xge.h"

int main(void)
{
    xge_particle_emitter_t emitter;
    xge_particle_definition definition = NULL;
    xge_particle_world world = NULL;
    xge_particle_effect effect = 0;
    int result = 1;

    xgeParticleEmitterInit(&emitter);  /* 不要只将结构体清零。 */
    emitter.fRate = 80;
    emitter.tGravity = (xge_vec2_t){0, 180};
    if (xgeParticleDefinitionCreate(&definition, &emitter, 1) != XGE_OK) goto done;
    if (xgeParticleWorldCreate(&world, NULL) != XGE_OK) goto done;
    if (xgeParticleWorldReserve(world, definition, 4) != XGE_OK) goto done;
    if (xgeParticlePlay(world, definition, NULL, &effect) != XGE_OK) goto done;
    if (xgeParticleUpdate(world, 1.0 / 60.0) != XGE_OK) goto done;
    result = 0;
done:
    xgeParticleWorldFree(world);
    xgeParticleDefinitionFree(definition);
    return result;
}
```

在已有 XGE 帧循环中，另外创建 `xgeParticleRendererCreate(&renderer, capacity, batchCapacity)`，在 GPU 可用后绑定纹理/材质，并在目标 pass 中调用 `xgeParticleRender(renderer, world, view)`。GPU 纹理和批缓冲在第一次渲染时初始化；完整的初始化、绘制与错误清理参照 `examples/xge_particles/common.c`。

## 定义、实例与资源所有权

- `DefinitionCreate` 校验并复制所有发射器。创建后定义不可变，`DefinitionGet` 返回副本。修改副本不会修改已有定义；需要新建定义。
- World 保存实例、粒子、事件和内部子发射任务。每个实例持有定义引用；`DefinitionFree` 只释放调用者的一份引用。
- Effect 是 **仅在所属 World 内有效** 的代际句柄，不能传给另一 World。`Release` 后旧句柄失效；`Restart` 保留句柄，清空粒子和发射时钟，重置随机流。
- `WorldReserve(world, definition, count)` 预留 `count` 个空闲实例和完整粒子容量；可复用其他定义的空闲槽。分配失败时已经预留的部分保留。正在使用的槽也计入 World 的实例上限。
- `Release` / 自动释放使实例回到缓存，不立即归还粒子缓冲；`WorldFree` 才释放整个 World 的缓存。定义也可能因此被缓存继续引用。
- Renderer 单独持有绑定定义、纹理、Shader 的引用。绑定会复制材质的颜色/管线设置，并引用附加纹理；修改原材质设置后需重新绑定。Shader 自定义 uniform 仍属于共享 Shader 对象。
- **纹理和 Shader 的 C 结构体由应用提供，必须一直存活到解绑或 Renderer 销毁。** 引用计数保护 GPU 资源，不会复制这两个结构体。可以提前释放应用的一份资源引用，但不能销毁其结构体存储。
- 推荐销毁顺序：Renderer → World → 调用者持有的 Definition → 应用纹理/Shader → `xgeUnit`。释放后的指针和句柄不要再次使用。

World/Renderer 及其回调按单线程使用。Visit 回调和碰撞回调中禁止更新、释放、发射或修改 World；相关可报告错误的操作返回 `XGE_ERROR_INVALID_STATE`。事件通过队列在 Update 后读取，不在模拟中直接调用游戏事件处理代码。

## 时间与播放状态

先调用 `WorldDescInit`、`PlayInit` 和 `EmitterInit`，再覆盖字段。默认固定步长为 `1/120` 秒，每次 Update 最多 64 个子步。Update 累积不足一个子步的余量；超出追赶上限的完整子步被丢弃，并累加到 `fDroppedTime`，不会留下无限追赶债务。

每个子步先更新已有粒子，再处理自动发射及延迟队列中的子发射器。新粒子在该子步末产生，年龄从 0 开始；Burst 和生命周期结束按固定步长量化，不提供子步内的精确出生时间积分。

| 操作/状态 | 语义 |
| --- | --- |
| `PLAYING` | 更新已有粒子，并允许自动和手动发射。 |
| `Pause(..., 1)` | 暂停粒子年龄和发射时钟；恢复时回到原来的 PLAYING/DRAINING 状态。 |
| `Stop(..., 0)` | DRAINING：停止根发射，已有粒子继续运动、死亡和触发子发射器。 |
| `Stop(..., 1)` | 立即清空粒子，不为清掉的粒子产生死亡事件；下一个子步进入 FINISHED。 |
| `FINISHED` | 无粒子且无未来自动发射；只产生一次结束事件。可 Restart，不能直接 Emit。 |
| `Release` | 立即使句柄失效并缓存实例，不产生结束/死亡事件。 |
| `bAutoRelease` | Update 结束时自动 Release 已完成的实例；队列中的结束事件仍包含其旧句柄。 |

只有手动发射的定义，在空实例经历一次 Update 后也会完成。可以在 Play 后立即 Emit，或者需要时 Restart 再 Emit。`State` 返回状态常量或负数错误；`EventPoll` 返回 1（有事件）、0（队列空）或负数错误。

`fDelay` 只发生在最开始；`fDuration == 0` 表示没有有限发射窗口。正 duration 配合 loop 重复窗口中的 Burst，但不重复初始 delay。Burst 时间相对 delay，必须小于正 duration。`bAutomatic == 0` 的发射器仅响应 Emit 或子发射器链接。

`Prewarm` 只推进一个实例，不推进其他实例或 World 的固定步长余量。公开事件被抑制，内部子发射器照常运行，已有的公开待取事件不被清空。上限为 60 秒且不超过 100,000 步。预热完成后的自动释放由随后一次 Update 处理。

### 倍率覆盖

`PlayInit` 将四种倍率设为 1，Tint 设为白色：

- `fRateScale`：按秒/按距离发射倍率，不改变 Burst 或手动 Emit 的数量。
- `fSpeedScale`：新粒子的初始速度倍率，不重写已经出生的粒子速度。
- `fSizeScale`、`iTint`：实时作用于已有和新粒子的输出外观。
- `fTimeScale`：整个实例的模拟时间倍率，包括发射时钟。0 冻结时间，但不等同于 PAUSED；外部变换仍可移动本地粒子。

前三个倍率范围分别为 `[0,1000]`、`[0,1000]`、`[0,1000]`，时间倍率为 `[0,16]`。

固定种子、相同定义、固定步输入与操作顺序可用于复现效果；每个发射器使用独立 XRT 随机流。**不承诺跨 CPU、编译器或数学库的逐位浮点一致性。** 容量竞争也会影响实际出生数量，复现时要保持容量和实例顺序一致。

## 空间、形状和运动单位

长度使用 XGE 世界坐标单位（通常按像素配置），时间为秒，角度/角速度为弧度/弧度每秒。默认屏幕轴向为 X 向右、Y 向下；`fDirection = -π/2` 朝上。Spread 表示完整扇角，范围 `[0, 2π]`。

Point 的位置是 offset；Line 从 `-shapeSize/2` 到 `+shapeSize/2`；Rect 以 offset 为中心。Circle 使用 radius，Ring 使用 innerRadius 到 radius；Cone 是沿 direction、半径为 radius 的扇形位置分布。**初始运动方向独立按 direction/spread 采样**，不强制与采样位置的径向一致。

- WORLD：出生时应用实例变换，之后已有粒子不会随发射器平移/旋转；尺寸保留出生时缩放。适合尾尘、爆炸、火花。
- LOCAL：在本地模拟，输出位置/速度/尺寸再应用当前实例变换。适合附着光环。实例 scale 必须正值，范围 `[0.0001,10000]`。
- 世界粒子的 gravity 等力按世界轴解释；本地粒子的力按本地轴解释。径向/切向力相对各粒子的出生发射中心，不追踪之后移动的发射器。
- `SetTransform` 在 Update 的多个子步间线性插值。距离发射根据变换前后 offset 点的位移均匀分布新粒子；延迟/结束只覆盖对应的有效线段。它不是曲线路径重建；瞬移也会留下距离发射，必要时先暂停、重播或临时关闭距离密度。
- `fInheritVelocity` 添加发射器位移除以实例模拟步长得到的速度；时间倍率会影响这个速度。手动 Emit / 静止预热没有位移速度继承。
- 阻力为 `exp(-drag * dt)`；噪声是平滑二维 value noise 加速度，不是流体或 curl-noise 求解器。

尺寸曲线是初始 size 的倍率，Y 再乘 aspect；alignVelocity 沿输出速度朝向，stretch 将 X 尺寸乘以 `1 + stretch * speed`。速度曲线调制位移量，不反复缩放内部速度。返回的速度快照是内部模拟速度经空间变换后的值，不包含这个位移倍率或本地根节点运动速度。

曲线时间是归一化年龄 `[0,1]`，最多 8 个严格递增键；范围外保持首末键值。Hermite 的 in/out tangent 是相对于归一化年龄的导数。未配置尺寸/速度/透明度曲线时使用 1，未配置渐变时使用白色。最终颜色为随机出生色 × 生命周期渐变 × 实例 Tint，透明度再乘 alpha 曲线。透明度限制在 `[0,1]`，负尺寸/速度曲线输出按 0 处理。

输入拒绝 NaN、Inf、非法枚举、逆序范围和循环子发射图。尺寸/初速不超过 `1e6`，生命周期为 `[0.0001,3600]` 秒，多数其他浮点字段绝对值不超过 `1e12`；这些是防御性上限，不是建议效果尺度。极端组合的输出尺寸被限制为 `1e12`；模拟产生无效位置/速度时移除该粒子，不把无效状态转发给子发射器。

## 碰撞、子发射与游戏事件

在 World 配置 `pCollision` 回调。每个启用碰撞的粒子每子步最多做一次扫掠查询；回调接收世界坐标（屏幕空间粒子则是屏幕坐标）的起终点，以及 `collisionRadius * max(输出宽, 输出高)` 的半径。0 表示点粒子。

回调返回非 0 时填写命中中心、朝外法线、`[0,1]` 命中比例。运行时校验并归一化法线：

- Plane 允许区域为 `dot(P, normal) >= offset`；函数支持非单位输入法线。
- Rect 使用半径扩展 AABB 的线段检测，角部是**保守方角扩张**，不是精确的圆角扫掠。
- Bounce 使用 restitution 和 friction，继续走完剩余步长；同一步不会再做第二次碰撞。
- Kill 先报告碰撞，再报告死亡，两种子发射链接都可能触发。

多个障碍物由应用查询并选择最近命中；范例中仅用平面和一个目标矩形。这不是游戏刚体系统，不能用于精确命中判定、复杂狭缝或高速多次反弹求解。

`arrSubEmitters[EVENT_BIRTH/DEATH/COLLISION]` 指向**同一定义内**的发射器下标。子粒子使用事件位置为发射中心，并可继承事件速度；目标的 shape 仍适用，其 offset 不再叠加。建议子发射器 `bAutomatic = 0`。链接图必须无环，内部按延迟 FIFO 队列处理，受同一出生预算约束。

游戏逻辑在 Update 后循环 `EventPoll`，可根据事件决定声音、伤害提示等。公开事件队列满时丢弃新事件并累计 `iDroppedEvents`，不影响内部子发射。`iMaxEvents = 0` 可禁用公开事件。手动清空、Release 和离屏清空不产生逐粒子死亡事件。

## 渲染、排序与裁剪

CPU 定义只保存逻辑资源名称；调用者通过 `RendererBind` 将每个发射器绑定到 XGE Texture/Material。未绑定时绘制白色四边形，可用于彩纸/火花。`DefinitionLoad` 复用 `xgeResourceLoad`，但不自动加载定义里的 texture/material 名称。

纹理 rect 使用像素单位，宽/高为 0 时分别取纹理宽/高。columns/rows 均为 `[1,256]`；起始帧可以随机。fps > 0 时按秒循环网格，fps == 0 时按生命周期覆盖整个网格，再叠加 startFrame 并取模。它不会建立 Sprite 或动画对象。

Renderer 先按 layer → order → 出生序号排序，再合并相邻兼容批次；不会为减少纹理切换而打乱透明绘制顺序。当前不同绑定记录也会分批，即使资源相同；批容量满时同样分批。每粒子提交一个四边形，不进行逐粒子 GPU DrawCall。自定义材质使用同一批接口，材质显式 blend 覆盖发射器 blend；默认材质 blend 则沿用发射器设置。

XGE 混合使用预乘颜色：粒子适配器会将最终顶点颜色预乘 alpha。输入纹理也应为预乘 RGBA；自定义 Shader 应保持相同约定。材质自定义 uniform 由应用设置，标准位置/UV/颜色布局与现有 Material API 相同。

`Render` 开始时先 Flush 已排队的 XGE 绘制，结束时恢复 blend。它是在当前 pass 的明确绘制点，并不参与其他系统的全局层级调度。HUD/前景应在其后绘制。第一版适配当前非渲染线程的 GL/GLES 路径；开启 XGE 渲染线程会返回 `XGE_ERROR_UNSUPPORTED`。

传入的 view 仅是粒子坐标系中的**绘制剔除矩形，不是裁剪框，也不自动转换相机坐标**。需要切掉跨边缘粒子的像素时另用 `xgeClipSet`。混合世界与屏幕空间时可传 NULL，或分别组织渲染；不要拿屏幕矩形直接剔除未经相机变换的世界坐标。

`SetVisible(false)` 的行为由 Play 的 cullPolicy 决定，与 Render(view) 的剔除互不替代：

| 策略 | 不可见时 |
| --- | --- |
| DRAW（默认） | 继续模拟，只不绘制。 |
| PAUSE | 暂停模拟/发射时钟，恢复可见后继续，不补发离屏移动段。 |
| CLEAR | 清空现有粒子、推进发射时钟并丢弃离屏发射，恢复后不补发；有限效果可在离屏期间结束。 |

`Bounds` 返回当前粒子旋转矩形的联合 AABB，含尺寸曲线和拉伸；没有粒子时返回当前位置的零尺寸框。它不是未来运动范围，不能仅因一个空发射器的 bounds 不可见就永久停止检查其可见性。

## 容量与性能

每个定义最多 16 个发射器，各自最多 16 个 Burst、8 个曲线/渐变键；定义内粒子容量之和最多 1,000,000。World 的实例、全局粒子、事件、每步出生预算均可配置。已有粒子与较早处理的实例优先；超额新粒子被丢弃，不覆盖存活粒子。不要把防御上限当作实际性能保证。

Play/Reserve 完成粒子缓冲预留，Update/Emit 使用稠密数组和 swap-remove；事件环、内部任务队列以及句柄表预先预留。正常模拟更新不分配堆内存，测试用 XRT 分配统计验证此合同。Renderer 在创建时预留排序数组，首次渲染时建立 GPU/批资源；配置解析、绑定、Play 缓存扩容以及驱动内部开销不包含在“模拟无分配”承诺中。

Renderer capacity 小于本帧可绘制粒子数时返回 `XGE_ERROR_BUFFER_TOO_SMALL`，并在绘制任何粒子之前中止，不能忽略该返回值。`Stats` 给出存活/峰值、实例与缓存、累计出生/丢弃/碰撞/子步、事件积压、丢弃时间与最近 Update 耗时。世界不会自动重置这些统计；每次新建 World 从零开始。

## JSON / XSON 配置

两种格式共用 `version: 1` 模型，顶层只有 `version` 和 `emitters`。文本最大 1 MiB；UTF-8；省略字段使用 `EmitterInit` 默认值，未知字段、非法类型和非法范围报错。枚举使用头文件对应的整数，不接受字符串枚举名。Stringify 输出包含全部配置，不保存运行时粒子、随机流进度、实例变换或材质绑定。

```json
{
  "version": 1,
  "emitters": [{
    "name": "sparks",
    "maxParticles": 256,
    "rate": 0,
    "loop": false,
    "duration": 0.25,
    "shape": 5,
    "direction": -1.5707963,
    "spread": 1.8,
    "life": [0.25, 0.8],
    "speed": [100, 260],
    "size": [2, 5],
    "gravity": [0, 400],
    "colorMin": "#FFB344FF",
    "colorMax": "#FFF5CCFF",
    "alphaOverLife": {"interpolation": 0, "keys": [[0, 1], [1, 0]]},
    "bursts": [[0, 80]]
  }]
}
```

标量字段按 C 名称去掉类型前缀，使用 lowerCamelCase，例如 `fRateOverDistance` → `rateOverDistance`、`bAlignVelocity` → `alignVelocity`、`iMaxParticles` → `maxParticles`。下列组合字段有专门形式：

| 字段 | 数据形式 |
| --- | --- |
| `name`, `texture`, `material` | 字符串，最多分别 63/127/63 个 UTF-8 字节。 |
| `offset`, `shapeSize`, `gravity` | `[x, y]`。 |
| `life`, `speed`, `size`, `rotation`, `angularVelocity`, `startFrame` | `[min, max]`。 |
| `textureRect` | `[x, y, width, height]`。 |
| `colorMin`, `colorMax` | `"#RRGGBBAA"` 或对应 uint32 整数。 |
| `sizeOverLife`, `speedOverLife`, `alphaOverLife` | `{ "interpolation": 0, "keys": [[time, value], ...] }`；键也可为 `[time, value, inTangent, outTangent]`。 |
| `colorOverLife` | `[[time, "#RRGGBBAA"], ...]`。 |
| `bursts` | `[[time, count], ...]`。 |
| `subEmitters` | `{ "death": { "emitter": 1, "count": 2, "inheritVelocity": 0.3 } }`，事件名也可为 birth/collision。 |

完整键名以 `src/xge_particle_io.c` 的字段表和范例 `--export` 输出为准。Parse 失败时输出定义为 NULL；可选错误缓冲指明字段或定义约束，底层语法/分配失败也会返回错误，不能只检查字符串。Stringify 返回的 UTF-8 文本用 `xrtFree` 释放；长度不含结尾 NUL。

## 复用与实现入口

| 现有设施 | 粒子层复用方式 |
| --- | --- |
| XRT `xrng` | 每发射器独立、有种子的随机流。 |
| XRT `xarray` / `xpool` / `xslotmap` | 稠密粒子数组、稳定实例存储、代际句柄和缓存。 |
| XRT `xmap` | Renderer 的定义/发射器资源绑定。 |
| XRT JSON/XSON / `xvalue` | 配置解析、所有权、序列化，不再写另一套文本解析器。 |
| XRT 原子引用/分配/计时/内存调试 | 定义引用、分配失败处理、统计与测试。 |
| XGE Resource | 现有资源加载与释放合同。 |
| XGE SpriteBatch/Material | 四边形、相机、UV、GPU 提交；新增空批换纹理与材质批提交两个通用入口。 |
| ShapeEx 的数学操作 | 仿射点变换、RGBA 插值提取为共享私有助手，未引入布局依赖。 |

核心为 `src/xge_particle.c`，配置为 `xge_particle_io.c`，资源桥接为 `xge_particle_resource.c`，绘制为 `xge_particle_render.c`。它们由现有 `xge.c` 单元集成，不需要在应用项目里再独立添加这些 `.c` 文件。

## 验证入口

Windows 完整复验可直接执行 `test\build_particle_suite.bat`：重建 DLL，运行 CPU/DLL/GPU 与现有 XGE smoke 测试，构建七个范例并逐个导出、重载 JSON/XSON。GPU 和范例测试会短暂创建窗口。也可按需分别运行：

```bat
build_dll.bat
test\build_particle_test.bat
test\build_particle_dll_test.bat
test\build_particle_render_test.bat
test\build_particle_bench.bat
examples\xge_particles\build.bat
build_test.bat
```

独立 CPU 测试覆盖曲线/形状、随机复现、生命周期、固定步长、预算、世界/本地/距离发射、倍率、碰撞、子发射、预热、可见性、失效句柄、配置往返、回调重入保护；内存调试逐个注入解析/创建/播放/序列化分配失败，并检查泄漏及更新无分配。DLL 测试复用同一组公开 API 用例；GPU 测试读回像素，验证预乘透明度、排序、材质批次、容量错误、资源引用、序列帧 UV 和屏幕空间，以及材质批提交后原有 MaterialDraw 仍可正常使用。

本轮验证环境为 Windows x64 / MinGW-w64 / OpenGL。提供 Linux shell 构建入口，但没有把未运行的 Linux、移动端或 Web 构建写成已验证；这些平台仍须在目标环境执行相同回归。

另有同步前后均存在的 XRT 裁剪头 C++ `extern "C"` 配对问题；当前交付验证的是 C 接入，详情见 [XRT 快照说明](../lib/xrt/README.md)。

`build_particle_bench.bat` 仅测 CPU 模拟（无公开事件、无碰撞、无渲染），分别运行 1,000/10,000/50,000 粒子、有/无噪声，每组 240 个 `1/120` 秒子步。它检查粒子没有丢失，但不设置机器相关的耗时阈值。本次本机 10,000 粒子均值约 0.29 ms/子步，开启噪声约 0.86 ms/子步；不能等同于整个游戏帧耗时或其他平台保证。
