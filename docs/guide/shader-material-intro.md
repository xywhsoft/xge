# Shader / Material 入门

本教程介绍 XGE 中自定义 shader、material 和 shader 变体的基本使用方式。

[返回教程索引](README.md) | [Material API](../api/material.md) | [Shader 范例](../case/shader-material.md)

## 设计边界

XGE 的默认渲染 API 覆盖 sprite、shape、文本和 RenderTarget。自定义 shader 属于高级接口，适合局部效果，不建议把所有普通绘制都改成手写 shader。

Material 负责组合：

- shader
- texture
- color
- blend mode
- draw command

## Shader 版本

不同后端的 GLSL 头部不同。业务代码应优先通过 `xgeGraphicsShaderHeaderGet` 获取当前后端需要的头部，再拼接 shader 主体。

```c
char header[512];

if ( xgeGraphicsShaderHeaderGet(XGE_GRAPHICS_BACKEND_DEFAULT, header, sizeof(header)) > 0 ) {
	/* append custom shader body */
}
```

这样可以减少桌面 GL、GLES3、WebGL2 之间的语法分叉。

## 创建 shader

```c
xge_shader_t shader;

if ( xgeShaderCreate(&shader, sVertexSource, sFragmentSource) != XGE_OK ) {
	return XGE_ERROR_BACKEND;
}
```

如果只替换 fragment shader，可以先使用 XGE 默认顶点输入约定；后续复杂 mesh 再提供完整 vertex shader。

## 设置 uniform

```c
xgeShaderUniform1f(&shader, "u_time", xgeGetDelta());
xgeShaderUniform2f(&shader, "u_resolution", (float)xgeGetWidth(), (float)xgeGetHeight());
```

Uniform 名称查找不应成为每帧热点。后续实现如果暴露 uniform handle，应优先在资源加载阶段缓存。

## 使用 material 绘制

```c
xge_material_t material;
xge_draw_t draw;

xgeMaterialInit(&material);
xgeMaterialSetShader(&material, &shader);
xgeMaterialSetTexture(&material, &texture);
xgeMaterialSetBlend(&material, XGE_BLEND_ALPHA);

memset(&draw, 0, sizeof(draw));
draw.tDst.fX = 0.0f;
draw.tDst.fY = 0.0f;
draw.tDst.fW = 256.0f;
draw.tDst.fH = 256.0f;
draw.iColor = xgeColorRGBA(255, 255, 255, 255);
xgeMaterialDraw(&material, &draw);
```

## Shader 变体

当一个效果需要多个宏开关时，不要在每帧拼接源码。使用 variant set 管理组合：

```c
xge_shader_variant_set_t variants;
xge_shader shader_variant;

xgeShaderVariantSetInit(&variants, sVertexSource, sFragmentSource);
xgeShaderVariantGet(&variants, iKey, pDefines, iDefineCount, &shader_variant);
```

`iKey` 应由业务根据宏组合稳定生成，便于缓存命中。

## 释放资源

```c
xgeMaterialFree(&material);
xgeShaderFree(&shader);
xgeShaderVariantSetFree(&variants);
```

## 常见错误

不要在每个 sprite 上创建 shader。Shader 和 material 应该是资源对象，绘制时复用。

不要把后端差异写散在业务代码里。先通过图形后端映射和 shader header 处理差异。

不要在普通 UI 绘制中无节制切换 material。频繁切换会破坏批处理效果。

## 下一步

- 看完整例子用 [Shader / Material 效果](../case/shader-material.md)。
- 需要离屏后处理时继续读 [RenderTarget 入门](render-target-intro.md)。
