# 独立动作与两种角色骨架

此范例加载两种真实不同的测试骨架：A 为 T 姿态，B 为 A 姿态并使用不同骨长和局部轴。两个角色分别加载离线重定向后的独立动作，再通过公开 C API 驱动蒙皮及太阳阴影。模型骨长保持不变。

```bat
examples\xge_3d_animation\build.bat
build\3d\xge_3d_animation.exe --action walk --time 0.25 --frames 3 --capture artifacts\xge-3d\p4-walk-demo.png
build\3d\xge_3d_animation.exe --action run --mix --frames 185
```

action 可选 `idle/walk/run/jump/slash/gather`；time 指定开始时刻，mix 用三秒把层 1 的待机权重从 0 调至 1，展示连续混合。未指定帧数时持续运行，Escape 退出。固定 1/60 秒步长用于可复现验证，实际游戏应传真实 delta。

构建脚本首次下载已固定版本和 SHA-256 的 KayKit Knight FBX（约 20 MB，CC0），用独立 C 工具转换六类动作并重定向。缓存、导出动作、两种测试角色和映射均位于 `artifacts/xge-3d/`，不进入 DLL。源许可证和下载清单见 [测试资源](../../test/assets/3d_motion/README.md)。工作环境离线时可提前获取这些固定文件。

根运动、动作事件和挂点的 API 使用见 [3D 文档](../../docs/3D.md)。本例是通用动画调用范例，没有角色控制或游戏业务。
