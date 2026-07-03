# Blend Depth Color Lab

`examples/blend_depth_color_lab` 覆盖颜色打包/解包、blend 状态和 depth test 状态。范例上半部分并排展示 `none`、`alpha`、`add` 三种混合模式的重叠纹理差异，下半部分展示 depth test 关闭与开启时，同一绘制顺序下的遮挡变化。

## 覆盖 API

- `xgeColorRGBA`
- `xgeColorUnpack`
- `xgeBlendSet`
- `xgeBlendGet`
- `xgeDepthTestSet`
- `xgeDepthTestGet`

## 构建和运行

```bat
examples\blend_depth_color_lab\build.bat
build\xge_blend_depth_color_lab.exe --frames 120
```

## 验证点

- 控制台输出 `blend-depth-color-lab init`。
- `color=1` 表示 `xgeColorRGBA` 与 `xgeColorUnpack` 的往返结果正确。
- `blend=1/1/1` 表示 `none`、`alpha`、`add` 三种模式都成功设置并读回。
- `depth=1/1` 表示 depth off/on 两种状态都成功设置并读回。
- 上半部分三个面板应能明显看出无混合、标准 alpha 混合和 additive 混合的差异。
- 下半部分左侧应主要看到后绘制的暖色块压住前景，右侧则应看到前景冷色块因 depth test 保持在上层。
