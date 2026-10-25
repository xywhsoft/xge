# 动画播放范例

```bat
python examples/xge/xge_3d_playback/build.py --run
python examples/xge/xge_3d_playback/build.py --run --frames 420 --capture artifacts/xge-animation-playback/demo.png
```

按 `1/2/3/4` 切换待机、行走、跑步、一次性跳跃；`P` 整体暂停/恢复，`O` 淡入/淡出手臂加法动作，`L` 暂停/恢复叠加层，`Esc` 退出。标题显示播放时间、权重与剩余过渡时间。跳跃完成状态由应用查询并切回待机。

`--frames` 使用固定 60 Hz 自动流程，包含连续打断过渡、叠加、两种暂停、完成状态、事件、淡出停止。默认交互模式使用实际帧间隔并限制单帧 50 ms。复用仓库自有 Explorer glTF，不复制资产，也不依赖外部下载。示例中的跳跃只展示骨骼动作，没有角色移动或游戏规则。

构建输出 `build/animation-playback/demo`，关闭 2D、XUI、ShapeEx 和不需要的 3D 模块；仅用公开 C API。CPU 资产与 Renderer 的生命周期不同，Renderer 在首个帧回调中创建，确保 OpenGL 上下文有效。

接口及事件/根运动约定见 [动画播放指南](../../../docs/guide/3D_PLAYBACK.md)。
