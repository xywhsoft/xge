# 碰撞、缓存与节点综合范例

演示同一个 glTF 模型缓存加载与多实例放置、C 描述表装配、按名称查找节点、借用用户数据、胶囊扫掠阻挡及带碰撞绑定的墙体克隆。全部调用公开 C API；角色移动响应留在范例。

在仓库根目录执行：

```bat
examples\xge\xge_3d_core\build.bat --run
```

WASD 移动，Esc 退出。运行到第 120 帧时克隆第二面墙。角色移动到障碍前停止，可绕行；当前示例为平面水平移动，没有阶梯、跳跃或刚体解算。

自动运行与截图：

```bat
python examples/xge/xge_3d_core/build.py --run --automatic --frames 360 --capture artifacts/xge-core-next/demo.png
```

自动路线先碰墙、再绕行到另一侧；退出时检查阻挡次数、最终位置及一次加载/三次缓存命中。输出程序为 `build/core-next/demo/xge_3d_core.exe`，SDK、资产和程序同目录，不覆盖其他构建。

该配置启用 SCENE/COLLISION/CACHE，关闭 ASYNC、IBL 和 TERRAIN，验证同步缓存无需工作线程。模型复用仓库内 `xge_3d_walk/assets/explorer.gltf`，构建时复制到输出目录，无需网络下载。完整接口、所有权与能力边界见 [通用 3D 指南](../../../docs/guide/3D_CORE.md)。
