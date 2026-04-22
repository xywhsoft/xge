# Premultiplied Alpha 与 Blend 入门

本教程说明 XGE 的颜色格式、premultiplied alpha 和内置 blend 模式。

[返回教程索引](README.md) | [Graphics API](../api/graphics.md) | [纹理教程](texture-intro.md)

## 颜色格式

XGE 的颜色以 RGBA8 方式打包，尽量贴近 OpenGL/WebGL 常见输入顺序。

```c
uint32_t white = xgeColorRGBA(255, 255, 255, 255);
uint32_t half_black = xgeColorRGBA(0, 0, 0, 128);
```

需要读取分量时使用：

```c
xge_color_t c;

c = xgeColorUnpack(white);
```

## Premultiplied Alpha

XGE 默认按 premultiplied alpha 设计透明混合路径。也就是说，透明像素的 RGB 通常已经乘过 alpha。

优点：

- 边缘插值更稳定。
- UI、字体、粒子边缘更少黑边。
- 与现代 2D 渲染管线更一致。

需要注意：

- 同一张图不要重复预乘。
- 直 alpha 资源进入 XGE 前应在加载阶段统一转换。
- 自定义 shader 输出颜色时要和当前 blend 约定一致。

## 默认 Alpha Blend

```c
xgeBlendSet(XGE_BLEND_ALPHA);
```

这是普通 sprite、UI、字体最常用模式。大多数情况下不需要手动设置，因为默认绘制路径会使用它。

## 内置 Blend 模式

| 模式 | 用途 |
| --- | --- |
| `XGE_BLEND_NONE` | 不透明绘制或写入 mask。 |
| `XGE_BLEND_ALPHA` | 普通透明、UI、字体、sprite。 |
| `XGE_BLEND_ADD` | 光效、火焰、魔法、粒子叠加。 |
| `XGE_BLEND_MULTIPLY` | 阴影、压暗、颜色调制。 |
| `XGE_BLEND_SCREEN` | 柔和提亮、辉光。 |
| `XGE_BLEND_CUSTOM` | 后续扩展或后端自定义。 |

## 绘制局部光效

```c
xgeBlendSet(XGE_BLEND_ADD);
xgeDraw(&glow_texture, 120.0f, 80.0f);
xgeBlendSet(XGE_BLEND_ALPHA);
```

切换 blend 会影响批处理。大量对象使用不同 blend 时，应按 blend 和纹理排序。

## 自定义 shader 的注意点

Fragment shader 输出 premultiplied 颜色时，RGB 需要与 alpha 匹配：

```glsl
vec4 c = texture2D(u_texture, v_uv);
gl_FragColor = vec4(c.rgb * tint.rgb, c.a * tint.a);
```

如果 shader 输出直 alpha，却使用 premultiplied blend，边缘会出现错误。

## 常见错误

透明边缘发黑：通常是直 alpha 图片被当作 premultiplied 使用，或线性插值采样到了未处理的透明边缘 RGB。

光效越叠越白：加法 blend 正常会累加亮度，需要控制颜色和 alpha。

UI 颜色不对：检查材质、纹理、顶点色是否重复乘 alpha。

## 下一步

- 需要自定义效果时读 [Shader / Material 入门](shader-material-intro.md)。
- 需要读写图片像素时读 [纹理入门](texture-intro.md)。
