# 线性颜色、FXAA 与阴影开关

仅使用公开 C API 和程序化网格，不需要外部素材。场景包含高对比细杆/斜边、半透明红色覆盖蓝色的面板，以及带太阳阴影的地面和立柱。

```bat
python examples/xge/xge_3d_render_quality/build.py --run
```

A 切换 FXAA，C 切换橙色立柱的阴影投射，R 切换地面的阴影接收，Esc 退出。当前状态显示在窗口标题中；范例默认 FXAA、投射和接收均开启。

固定状态截图：

```bat
python examples/xge/xge_3d_render_quality/build.py --run --frames 30 --aa none --capture artifacts/xge-render-quality/demo-none.png
python examples/xge/xge_3d_render_quality/build.py --run --frames 30 --aa fxaa --capture artifacts/xge-render-quality/demo-fxaa.png
python examples/xge/xge_3d_render_quality/build.py --run --frames 30 --cast 0 --capture artifacts/xge-render-quality/demo-no-cast.png
python examples/xge/xge_3d_render_quality/build.py --run --frames 30 --receive 0 --capture artifacts/xge-render-quality/demo-no-receive.png
```

输出在 `build/render-quality/demo/`，使用匹配的精简 SDK。关闭 MODEL、ANIMATION、ASYNC、IBL、TERRAIN、FOG、SCENE、COLLISION、CACHE，保留渲染底座、直接光、阴影和 FXAA；不覆盖其他构建。

接口、Alpha、GPU 内存开销和边界见 [渲染完整性指南](../../../docs/guide/3D_RENDER_QUALITY.md)。
