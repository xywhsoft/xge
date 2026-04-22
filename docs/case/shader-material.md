# Shader / Material 效果

本案例展示如何创建自定义 shader，并用 material 绘制带参数的 sprite。

[返回范例解析](README.md) | [Shader 教程](../guide/shader-material-intro.md) | [Material API](../api/material.md)

## 问题

默认 sprite 和 shape 能覆盖大部分 2D 绘制，但描边、溶解、扭曲、换色、灰度等效果需要自定义 shader。XGE 用 material 封装 shader、纹理、颜色和 blend，避免每个效果都暴露成独立绘制函数。

## 最小结构

```text
shader source
  -> xgeShaderCreate
material
  -> set shader
  -> set texture
  -> set uniforms
  -> xgeMaterialDraw
```

## 步骤 1：准备 shader

```c
static const char* s_frag =
	"uniform sampler2D u_texture;\n"
	"uniform float u_amount;\n"
	"varying vec2 v_uv;\n"
	"void main() {\n"
	"	vec4 c = texture2D(u_texture, v_uv);\n"
	"	float g = dot(c.rgb, vec3(0.299, 0.587, 0.114));\n"
	"	gl_FragColor = vec4(mix(c.rgb, vec3(g), u_amount), c.a);\n"
	"}\n";
```

实际项目中应使用 `xgeGraphicsShaderHeaderGet` 根据后端拼接 shader 头部，避免 GL、GLES、WebGL 的语法差异散落在业务代码中。

## 步骤 2：创建 shader 和 material

```c
static xge_shader_t g_shader;
static xge_material_t g_material;
static xge_texture_t g_texture;

static int InitEffect(void)
{
	if ( xgeShaderCreate(&g_shader, NULL, s_frag) != XGE_OK ) {
		return XGE_ERROR_BACKEND;
	}

	xgeMaterialInit(&g_material);
	xgeMaterialSetShader(&g_material, &g_shader);
	xgeMaterialSetTexture(&g_material, &g_texture);
	xgeMaterialSetColor(&g_material, xgeColorRGBA(255, 255, 255, 255));
	xgeMaterialSetBlend(&g_material, XGE_BLEND_ALPHA);
	return XGE_OK;
}
```

## 步骤 3：更新 uniform 并绘制

```c
static void DrawGray(float fAmount)
{
	xge_draw_t draw;

	xgeShaderUniform1f(&g_shader, "u_amount", fAmount);

	memset(&draw, 0, sizeof(draw));
	draw.tDst.fX = 80.0f;
	draw.tDst.fY = 80.0f;
	draw.tDst.fW = 256.0f;
	draw.tDst.fH = 256.0f;
	draw.iColor = xgeColorRGBA(255, 255, 255, 255);
	xgeMaterialDraw(&g_material, &draw);
}
```

## Shader 变体

需要根据宏生成多个版本时，用 `xgeShaderVariantSetInit` 和 `xgeShaderVariantGet` 管理变体。变体 key 应由业务稳定生成，不要用字符串拼接作为每帧路径。

## 关键 API

| API | 作用 |
| --- | --- |
| `xgeGraphicsShaderHeaderGet` | 获取当前后端 shader 头 |
| `xgeShaderCreate` | 创建 shader |
| `xgeShaderUniform1f` | 设置 float uniform |
| `xgeShaderVariantGet` | 获取或创建 shader 变体 |
| `xgeMaterialInit` | 初始化 material |
| `xgeMaterialSetTexture` | 绑定纹理 |
| `xgeMaterialDraw` | 使用 material 绘制 |

## 常见问题

如果 shader 在桌面可用但 WebGL 不可用，优先检查 precision、attribute/varying、sampler 和 GLSL 版本声明。

如果设置 uniform 没有效果，确认当前 material 使用的 shader 与设置 uniform 的 shader 是同一个对象。
