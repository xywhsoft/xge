# 轻量画质增强范例

```bat
python examples/xge/xge_3d_light_effects/build.py --run
python examples/xge/xge_3d_light_effects/build.py --run --frames 60 --effects all --capture artifacts/xge-light-effects/demo-all.png
python examples/xge/xge_3d_light_effects/build.py --run --frames 60 --effects none --capture artifacts/xge-light-effects/demo-none.png
```

`T` 色调映射、`G` 基础调色、`B` Bloom、`H` 高度雾；`Esc` 退出。`--levels 1..6` 控制 Bloom 的空间扩散层数和处理开销，默认 4。场景、材质和灯具均为程序化数据，不下载资源。灯具为发光材质，Bloom 不会让它自动照亮附近建筑。

`none` 关闭这次新增的四项效果，保留同样的太阳阴影和 FXAA，便于同场景对照。自动退出时输出 Bloom 额外 Pass、纹理字节数和渲染器采样的 GPU 时间；GPU 时间会受分辨率、设备和其他负载影响，不作为固定性能承诺。

独立构建到 `build/light-effects/demo`，关闭 2D/XUI/ShapeEx 和无关 3D 模块。旧距离雾关闭，高度雾仍能独立使用。Renderer/Target 在帧回调有效上下文中创建和释放。效果设置采用公开 C API，默认全开仅是本范例的选择，库默认全关。

接口与边界见[轻量画质指南](../../../docs/guide/3D_LIGHT_EFFECTS.md)。
