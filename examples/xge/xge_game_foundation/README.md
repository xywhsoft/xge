# 通用游戏基础范例

完全通过公开 C API 演示六项基础机制，不需要外部模型或贴图：

- WASD / 手柄左摇杆通过动作映射控制绿色方块。
- 地面和两堵墙使用独立盒碰撞体，库完成恢复与滑动；范例自己处理重力。
- F1 开关调试碰撞线框；调试绘制不会写深度。
- 棋盘纹理实时更新一个区域，场景中的屏幕直接采样另一个 3D 渲染目标。
- 蓝色柱体使用顶点摆动与表面着色扩展，主绘制与太阳阴影共用变形。
- 有限容量 CPU 3D 粒子导出 Billboard，使用加法混合并参与深度测试。

```bat
python examples/xge/xge_game_foundation/build.py --run
python examples/xge/xge_game_foundation/build.py --run --frames 240 --capture artifacts/xge-game-foundation/demo.png
```

`--frames` 启用固定输入路径，先向右、再向前，便于重现碰墙和墙角滑动；正常运行读取真实输入。Esc 退出。游戏规则仅在范例中。构建关闭模型、动画、异步、缓存、地形、IBL 和多数后处理，保留六项功能所需模块。

接口与边界见 [使用指南](../../../docs/guide/GAME_FOUNDATION.md)，验收见 [实施记录](../../../docs/plans/xge-game-foundation/PROGRESS.md)。
