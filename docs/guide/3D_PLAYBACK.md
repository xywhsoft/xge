# 3D 动画播放

复用现有八层动画、TRS 覆盖/加法、骨骼遮罩、glTF 动作导入、事件及根运动。新增五个 C 接口：`AnimatorPlay`、`AnimatorFadeLayer`、`AnimatorSetPaused`、`AnimatorSetLayerPaused`、`AnimatorGetPlayback`，均使用 `xge3d` 前缀。没有动画图、状态机或游戏规则。

## 基本接入

```c
xge3d_animation_layer_t layer = xge3dAnimationLayerDefault();
int result = xge3dAnimatorPlay(animator, 0, walk, &layer, .2f);
/* Animator retains walk; the caller can release its own reference. */
if (!result) result = xge3dAnimatorUpdate(animator, dt);

xge3d_playback_info_t state = {.size = sizeof(state)};
if (!result) result = xge3dAnimatorGetPlayback(animator, 0, &state);
if (!result && state.finished) {
    /* Application decides whether/when to replace the held final pose. */
}
```

Animator 借用场景，必须先于 Scene 释放。所有操作在拥有场景的线程调用。正常每帧推进一次，动画更新先于渲染。加载和绑定失败返回错误，应用应检查返回值；旧播放状态保留。

## 播放、过渡与停止

| 操作 | 行为 |
| --- | --- |
| `Play(a, layer, clip, desc, seconds)` | 从当前层过渡至新剪辑。空层从零权重淡入；`desc == NULL` 使用默认值。 |
| `Play(a, layer, NULL, NULL, seconds)` | 从当前权重淡出，停止产生新事件；成功推进至结束后释放该层引用。已排队事件保留。 |
| `FadeLayer(a, layer, weight, seconds)` | 保持剪辑、遮罩和绑定，逐渐修改总体权重。空层返回 NOT_FOUND。 |
| `SetLayerWeight(a, layer, weight)` | 立即设置总体权重，取消权重淡化/停止；进行中的剪辑过渡保留。 |
| `SetLayer(a, layer, clip, desc)` | 立即替换，清除过渡、权重淡化及该层暂停；原有用法保持。 |

`seconds` 是非负有限秒数；零秒的 Play 等同 SetLayer。目标权重范围 `[0,1]`，从当前有效权重开始淡化，因此反向淡化不会跳变。淡化使用 Update 的实际秒数，与 `desc.speed` 独立：speed 为零仍能淡入淡出。Play 保留该层暂停标记；整体暂停始终独立保留。描述中的绑定和遮罩复制，剪辑引用由 Animator 持有。

普通切换期间，新旧两个剪辑都推进，按目标骨骼的平移、旋转、缩放通道与遮罩分别混合。新动作缺少的通道逐渐退回下层姿态，而不是突然恢复参考姿态。一次过渡尚未完成又收到 Play 时，保存这一刻混合出的通道姿态，作为新的静态过渡源；新的目标剪辑继续推进。这保证连续快速切换不跳姿态，并把每层存储限制在一个移动的旧剪辑或一个姿态快照，不累计剪辑链。它保证姿态连续，不保证过渡速度连续。

权重降到零的 FadeLayer 与旧 SetLayerWeight 一致：剪辑仍推进并发事件；需要停止和释放时使用 Play(NULL)。暂停中的停止请求会等到恢复后继续淡出。全局或分层暂停不会自动取消过渡。配置操作只改播放控制，姿态在下一次成功 Update 发布；全局暂停期间不重新发布姿态。

## 暂停、完成与状态

```c
xge3dAnimatorSetPaused(animator, 1);          /* 全体暂停 */
xge3dAnimatorSetPaused(animator, 0);          /* 恢复，不追赶已忽略的 dt */
xge3dAnimatorSetLayerPaused(animator, 1, 1);  /* 仅暂停第 1 层 */
```

暂停标记只接受 0/1。整体暂停冻结已发布姿态、所有剪辑、过渡、淡化、事件及根运动；可保留 SetPose 的程序姿态。分层暂停冻结本层新旧剪辑及淡化/事件，其当前姿态仍参与覆盖/加法混合，其他层继续工作。两种暂停独立，恢复整体播放不会清除分层暂停。暂停仍检查场景节点是否有效。

查询前设置 `size = sizeof(state)`。`active` 表示持有剪辑；`time/duration/speed/loop` 描述目标剪辑；`weight/target_weight` 描述总体权重；`fade_remaining` 是本层权重淡化和剪辑过渡剩余秒数的较大值，暂停时不减少。`transitioning` 表示还有剪辑姿态过渡，`stopping` 表示正淡出停止，`paused` 合并整体和分层标记，`mode` 返回覆盖或加法模式。

`finished` 是“非循环目标剪辑已到末尾”的持续状态，空层和循环层为假；动作末帧继续参与混合，应用决定后续行为。目标 time 设置到末尾或零时长剪辑可以立即 finished；finished 与是否仍在淡入/过渡互相独立。没有自动完成回调；应用可比较前后状态，或通过现有 SetEvents 注册末尾事件。完成后继续 Update 不重复发末尾事件。

## 叠加、事件和根运动

用层 0 播放基础动作，用层 1～7 和 `SetLayerMode(..., XGE3D_ANIMATION_ADDITIVE)` 添加相对资产参考 TRS 的动作差，再以目标节点索引排列的遮罩限制影响范围。Play/FadeLayer 同样适用于叠加层。加法参考姿态仍是模型资产参考姿态，不自动以剪辑首帧校正；素材应预先匹配骨架与参考姿态。不同骨架的 FBX/BVH 等资源继续使用现有[离线转换与重定向](3D.md#c-离线转换与人形重定向)。

切换清除旧剪辑尚未触发的标记，新事件只来自目标剪辑；旧的移动剪辑不再发事件。Play 之后用 SetEvents 为新剪辑注册标记。已经排队的事件始终可读。暂停不消耗首次更新的起点标记；停止请求抑制本层后续标记。原有 1024 标记/层、4096 待消费事件、循环边界规则不变。

根运动继续返回父坐标增量，库不移动角色。过渡会混合新旧剪辑的运动增量，使用该步内淡化权重的平均值，避免混合绝对骨骼位置引入偏移跳跃。连续打断时冻结的快照不产生运动；暂停层不产生新运动，但仍按其权重影响下层覆盖。对于非线性轨道、STEP 跳变或跨越片段末尾的大 dt，平均权重乘运动增量是近似值；需要精确移动时用固定的小步长（例如 1/60 或 1/120 秒）更新和取出根运动，再交给应用碰撞检查。

Update 的姿态、两条剪辑时钟、淡化/过渡时钟、事件与累计根运动一起提交。队列溢出、采样非法值或分配失败时，已发布结果保留。重试使用同一步 dt；失败后不会偷偷推进一部分动画。

## 体积、构建与验证

数据结构继续使用 xrt。没有新依赖或新模块：`XGE3D_ENABLE_ANIMATION=0` 移除整个动画播放实现和声明，`XGE3D_ENABLE_MODEL=0` 自动关闭动画。只在第一次混合时申请一份可复用通道工作区，每个模型节点约 52 字节；打断过渡才申请一份同样大小的层快照，过渡结束释放。正常 Update 无新增逐帧分配；事件队列扩容仍按原机制按需分配。

```bat
python examples/xge/xge_3d_playback/build.py --run
python examples/xge/xge_3d_playback/build.py --run --frames 420 --capture artifacts/xge-animation-playback/demo.png
tests\xge\build_3d_suite.bat playback 3d
```

[范例源码](../../examples/xge/xge_3d_playback/main.c)复用自有 Explorer 模型，演示待机/行走/跑步/一次动作、连续打断、手臂加法遮罩、两种暂停、事件、完成状态和淡出停止。完成与验收记录见[实施进度](../plans/xge-animation-playback/PROGRESS.md)。
