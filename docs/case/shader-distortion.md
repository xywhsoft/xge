# Shader 扭曲效果

本案例展示使用 fragment shader 做简单的 UV 扭曲。

[返回范例解析](README.md) | [Shader 教程](../guide/shader-material-intro.md) | [Material API](../api/material.md)

## 问题

水波、热浪、传送门和受击波纹这类效果通常不需要修改图片本身，只需要在 shader 中扰动采样坐标。

## Fragment Shader 思路

```glsl
uniform sampler2D u_texture;
uniform float u_time;
varying vec2 v_uv;

void main() {
	vec2 uv = v_uv;
	uv.x += sin((uv.y + u_time) * 30.0) * 0.01;
	gl_FragColor = texture2D(u_texture, uv);
}
```

实际代码应拼接当前图形后端的 shader header。

## 绘制

```c
xgeShaderUniform1f(&shader, "u_time", (float)xgeTimer());
xgeMaterialDraw(&material, &draw);
```

## 扩展

可以把扭曲强度、方向、中心点、半径做成 uniform，也可以先绘制到 RenderTarget，再对整屏纹理做后处理。

## 常见失败原因

WebGL 不通过：检查 precision 和 GLSL 版本语法。

边缘采样错误：设置合适的 wrap 模式，或在 shader 中 clamp UV。

效果影响 UI：把后处理限制在游戏画面 RenderTarget，UI 最后单独绘制。
