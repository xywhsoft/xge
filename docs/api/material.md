# Shader / Material / Mesh API

> Shader / Material / Mesh API 提供自定义 shader、uniform、material 状态和自定义网格绘制能力。

[返回 API 索引](README.md) | [Shader 教程](../guide/shader-material-intro.md) | [Shader 范例](../case/shader-material.md)

---

## 模块定位

普通 2D 绘制优先使用 [Drawing API](drawing.md)。当默认 sprite/shape 无法表达效果时，再使用 Shader、Material 和 Mesh：

- Shader：编译自定义 vertex/fragment shader。
- Shader Variant：按 define key 缓存多个 shader 变体。
- Material：绑定 shader、纹理、颜色和 blend 状态。
- Mesh：提交自定义顶点和索引，适合 2.5D、扭曲、网格特效和特殊批处理。

Shader 源码需要考虑 OpenGL 3.3、GLES3 和 WebGL2 的差异。推荐通过 `xgeGraphicsShaderHeaderGet` 获取当前后端 header。

## 标准调用顺序

```text
xgeShaderCreate
  -> xgeMaterialInit
  -> xgeMaterialSetShader / xgeMaterialSetTexture / xgeMaterialSetColor
  -> xgeMaterialDraw 或 xgeMeshDraw
  -> xgeMaterialFree
  -> xgeShaderFree
```

## 常量与宏

| 名称 | 值 | 说明 |
| --- | --- | --- |
| `XGE_MATERIAL_DEFAULT_BLEND` | `-1` | Material 使用全局默认 blend。 |
| `XGE_SHADER_DEFINE_MAX` | `8` | 单个 shader variant 的 define 数量上限。 |
| `XGE_SHADER_DEFINE_NAME_MAX` | `32` | define 名称长度上限。 |
| `XGE_MESH_DYNAMIC` | `0x0001` | 动态 mesh。 |

## 公共类型

### `xge_shader_t`

| 字段 | 说明 |
| --- | --- |
| `iRefCount` | 引用计数。 |
| `iProgram` | 后端 shader program id。 |
| `iLocResolution` / `iLocTexture` / `iLocColor` | 常用 uniform location。 |
| `pBackend` | 后端私有状态。 |

### `xge_shader_define_t`

| 字段 | 说明 |
| --- | --- |
| `sName` | define 名称。 |
| `iValue` | define 值。 |

### `xge_material_t`

| 字段 | 说明 |
| --- | --- |
| `pShader` | 绑定 shader。 |
| `pTexture` | 绑定纹理。 |
| `iColor` | 颜色调制。 |
| `tPipeline` | blend/depth 等 pipeline 状态。 |

### `xge_mesh_t`

| 字段 | 说明 |
| --- | --- |
| `iVertexCount` / `iIndexCount` | 顶点和索引数量。 |
| `iFlags` | `XGE_MESH_*`。 |
| `iVertexBufferId` / `iIndexBufferId` | 后端 buffer id。 |
| `pVertices` / `pIndices` | CPU 侧顶点和索引副本。 |

## API 参考

### xgeShaderCreate

编译并链接 shader program。

**功能：**

你可以用它创建自定义绘制效果，例如扭曲、颜色变换、特殊采样或 2.5D 网格效果。

**函数原型：**

```c
XGE_API int xgeShaderCreate(xge_shader pShader, const char* sVertexSource, const char* sFragmentSource);
```

**参数：**

- `pShader`：输出 shader 对象，不能为 `NULL`。
- `sVertexSource`：vertex shader 源码，不能为 `NULL`。
- `sFragmentSource`：fragment shader 源码，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 渲染后端未运行返回 `XGE_ERROR_NOT_INITIALIZED`。
- 编译或链接失败返回 `XGE_ERROR_GPU_FAILED`。

**资源归属：**

创建成功后，`pShader` 持有后端 program。调用者必须使用 `xgeShaderFree` 释放。

**补充说明：**

- 失败时，编译或链接错误会写入 XRT 错误信息和 XGE 日志。
- 创建成功后引用计数为 1。
- 实现会缓存 `uResolution`、`uTexture`、`uColor` 的 location，material 绘制会自动设置这些常用 uniform。

**范例代码：**

```c
xge_shader_t shader;
int ret = xgeShaderCreate(&shader, vertex_source, fragment_source);
if (ret != XGE_OK) {
    return ret;
}
```

**相关 API：**

- `xgeShaderFree`
- `xgeMaterialSetShader`
- `xgeGraphicsShaderHeaderGet`

---

### xgeShaderAddRef

增加 shader 引用计数。

**功能：**

你可以在多个对象共享同一个 shader 时调用它，避免某个持有者提前释放 GPU program。

**函数原型：**

```c
XGE_API int xgeShaderAddRef(xge_shader pShader);
```

**参数：**

- `pShader`：shader 对象，可以为 `NULL`。

**返回值：**

- `pShader` 有效时返回新的引用计数。
- `pShader` 为 `NULL` 时返回 `0`。

**资源归属：**

每次成功增加引用后，都需要在不再使用时调用一次 `xgeShaderFree`。

**补充说明：**

- 引用计数达到 `INT32_MAX` 后不再继续增加。
- `xgeMaterialSetShader` 当前只保存借用指针，不会自动调用该函数。

**范例代码：**

```c
if (xgeShaderAddRef(&shader) > 0) {
    /* 共享给另一个持有者 */
}
```

**相关 API：**

- `xgeShaderFree`
- `xgeMaterialSetShader`

---

### xgeShaderFree

释放 shader 引用。

**功能：**

你可以用它减少引用计数，并在引用归零时释放后端 shader program。

**函数原型：**

```c
XGE_API void xgeShaderFree(xge_shader pShader);
```

**参数：**

- `pShader`：shader 对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `pShader` 持有的后端 program，并在最终释放时清零对象。

**补充说明：**

- 对 `NULL` 调用是安全的。
- 如果引用计数大于 1，只减少引用计数，不释放 program。

**范例代码：**

```c
xgeShaderFree(&shader);
```

**相关 API：**

- `xgeShaderCreate`
- `xgeShaderAddRef`

---

### xgeShaderUniform1f

设置 float uniform。

**功能：**

你可以用它向自定义 shader 传入一个浮点参数。

**函数原型：**

```c
XGE_API int xgeShaderUniform1f(xge_shader pShader, const char* sName, float fX);
```

**参数：**

- `pShader`：shader 对象，必须已经创建成功。
- `sName`：uniform 名称，不能为 `NULL`。
- `fX`：float 值。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 后端未初始化返回 `XGE_ERROR_NOT_INITIALIZED`。
- 找不到 uniform 返回 `XGE_ERROR_RESOURCE_FAILED`。

**资源归属：**

函数不接管任何资源。`sName` 只需在调用期间有效。

**补充说明：**

- 函数内部会临时 `glUseProgram`，设置完成后恢复为 0。
- 被编译器优化掉的 uniform 可能无法找到。

**范例代码：**

```c
xgeShaderUniform1f(&shader, "uTime", time_seconds);
```

**相关 API：**

- `xgeShaderUniform2f`
- `xgeShaderUniform4f`

---

### xgeShaderUniform2f

设置 vec2 uniform。

**功能：**

你可以用它传入分辨率、偏移、方向或 UV 参数。

**函数原型：**

```c
XGE_API int xgeShaderUniform2f(xge_shader pShader, const char* sName, float fX, float fY);
```

**参数：**

- `pShader`：shader 对象。
- `sName`：uniform 名称，不能为 `NULL`。
- `fX`：第一个 float 值。
- `fY`：第二个 float 值。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 后端未初始化返回 `XGE_ERROR_NOT_INITIALIZED`。
- 找不到 uniform 返回 `XGE_ERROR_RESOURCE_FAILED`。

**资源归属：**

函数不接管任何资源。

**补充说明：**

该函数适合设置 `vec2`，也适合设置两个相关的标量参数。

**范例代码：**

```c
xgeShaderUniform2f(&shader, "uOffset", 4.0f, 8.0f);
```

**相关 API：**

- `xgeShaderUniform1f`
- `xgeShaderUniform3f`

---

### xgeShaderUniform3f

设置 vec3 uniform。

**功能：**

你可以用它传入三维方向、颜色、位置或自定义参数。

**函数原型：**

```c
XGE_API int xgeShaderUniform3f(xge_shader pShader, const char* sName, float fX, float fY, float fZ);
```

**参数：**

- `pShader`：shader 对象。
- `sName`：uniform 名称，不能为 `NULL`。
- `fX`：第一个 float 值。
- `fY`：第二个 float 值。
- `fZ`：第三个 float 值。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 后端未初始化返回 `XGE_ERROR_NOT_INITIALIZED`。
- 找不到 uniform 返回 `XGE_ERROR_RESOURCE_FAILED`。

**资源归属：**

函数不接管任何资源。

**补充说明：**

颜色仍建议优先使用 XGE 的 RGBA 工具生成，只有 shader 明确需要 vec3 时再使用该函数。

**范例代码：**

```c
xgeShaderUniform3f(&shader, "uLightDir", 0.0f, 0.0f, 1.0f);
```

**相关 API：**

- `xgeShaderUniform2f`
- `xgeShaderUniform4f`

---

### xgeShaderUniform4f

设置 vec4 uniform。

**功能：**

你可以用它传入颜色、矩形、四元参数或其他四分量数据。

**函数原型：**

```c
XGE_API int xgeShaderUniform4f(xge_shader pShader, const char* sName, float fX, float fY, float fZ, float fW);
```

**参数：**

- `pShader`：shader 对象。
- `sName`：uniform 名称，不能为 `NULL`。
- `fX`：第一个 float 值。
- `fY`：第二个 float 值。
- `fZ`：第三个 float 值。
- `fW`：第四个 float 值。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 后端未初始化返回 `XGE_ERROR_NOT_INITIALIZED`。
- 找不到 uniform 返回 `XGE_ERROR_RESOURCE_FAILED`。

**资源归属：**

函数不接管任何资源。

**补充说明：**

如果 uniform 由 material 自动设置，例如 `uColor`，通常不需要手动设置。

**范例代码：**

```c
xgeShaderUniform4f(&shader, "uTint", 1.0f, 1.0f, 1.0f, 0.5f);
```

**相关 API：**

- `xgeShaderUniform1f`
- `xgeMaterialSetColor`

---

### xgeShaderVariantSetInit

初始化 shader variant set。

**功能：**

你可以用它保存一组原始 shader 源码，后续按 key 和 define 自动生成、编译并缓存不同变体。

**函数原型：**

```c
XGE_API int xgeShaderVariantSetInit(xge_shader_variant_set pSet, const char* sVertexSource, const char* sFragmentSource);
```

**参数：**

- `pSet`：输出 variant set，不能为 `NULL`。
- `sVertexSource`：vertex shader 原始源码，不能为 `NULL`。
- `sFragmentSource`：fragment shader 原始源码，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 复制源码失败返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

函数会复制 shader 源码。调用者仍持有原始字符串，variant set 使用 `xgeShaderVariantSetFree` 释放。

**补充说明：**

- 该函数本身不编译 shader。
- 变体在 `xgeShaderVariantGet` 首次请求时创建。

**范例代码：**

```c
xge_shader_variant_set_t variants;
xgeShaderVariantSetInit(&variants, vertex_source, fragment_source);
```

**相关 API：**

- `xgeShaderVariantGet`
- `xgeShaderVariantSetFree`

---

### xgeShaderVariantSetFree

释放 shader variant set。

**功能：**

你可以用它释放所有已缓存变体、复制的源码和内部链表节点。

**函数原型：**

```c
XGE_API void xgeShaderVariantSetFree(xge_shader_variant_set pSet);
```

**参数：**

- `pSet`：variant set，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `pSet` 持有的所有 shader 变体和源码副本，并清零对象。

**补充说明：**

- 对 `NULL` 调用是安全的。
- 从该 set 返回过的 shader 指针会在释放后失效。

**范例代码：**

```c
xgeShaderVariantSetFree(&variants);
```

**相关 API：**

- `xgeShaderVariantSetInit`
- `xgeShaderVariantGet`

---

### xgeShaderVariantGet

获取或创建 shader 变体。

**功能：**

你可以按 `iKey` 和 define 列表获取 shader。已有 key 会直接返回缓存 shader；没有则在源码中插入 `#define` 后创建新 shader。

**函数原型：**

```c
XGE_API int xgeShaderVariantGet(xge_shader_variant_set pSet, uint32_t iKey, const xge_shader_define_t* pDefines, int iDefineCount, xge_shader* ppShader);
```

**参数：**

- `pSet`：已初始化的 variant set。
- `iKey`：变体 key。
- `pDefines`：define 数组。`iDefineCount > 0` 时不能为 `NULL`。
- `iDefineCount`：define 数量，范围为 0 到 `XGE_SHADER_DEFINE_MAX`。
- `ppShader`：输出 shader 指针，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法或 define 名称非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 内存不足返回 `XGE_ERROR_OUT_OF_MEMORY`。
- shader 创建失败时返回 `xgeShaderCreate` 的错误码。

**资源归属：**

返回的 `*ppShader` 是 variant set 内部 shader 的借用指针，生命周期到 `xgeShaderVariantSetFree`。

**补充说明：**

- define 名称必须是合法 C/GLSL 标识符，长度受 `XGE_SHADER_DEFINE_NAME_MAX` 限制。
- 实现会自动插入 `#define XGE_VARIANT_KEY <key>`。
- 如果源码以 `#version` 开头，define 会插入到 version 行之后。

**范例代码：**

```c
xge_shader_define_t defs[1] = {{"USE_WAVE", 1}};
xge_shader shader;
xgeShaderVariantGet(&variants, 1, defs, 1, &shader);
```

**相关 API：**

- `xgeShaderVariantSetInit`
- `xgeShaderCreate`

---

### xgeMaterialInit

初始化 material。

**功能：**

你可以用它准备一个 material 对象，用于绑定 shader、texture、颜色和 blend 状态。

**函数原型：**

```c
XGE_API void xgeMaterialInit(xge_material pMaterial);
```

**参数：**

- `pMaterial`：输出 material，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数不分配外部资源，只初始化结构体字段。

**补充说明：**

- 默认颜色为白色不透明。
- 默认 blend 为 `XGE_MATERIAL_DEFAULT_BLEND`，表示使用当前全局 blend。

**范例代码：**

```c
xge_material_t material;
xgeMaterialInit(&material);
```

**相关 API：**

- `xgeMaterialFree`
- `xgeMaterialSetShader`

---

### xgeMaterialFree

释放 material 持有引用。

**功能：**

你可以用它释放 material 当前持有的 texture 引用，并清零对象。

**函数原型：**

```c
XGE_API void xgeMaterialFree(xge_material pMaterial);
```

**参数：**

- `pMaterial`：material 对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数会释放 material 对 texture 增加的引用。当前实现不会释放 `pShader`，shader 是借用指针。

**补充说明：**

- 对 `NULL` 调用是安全的。
- 如果调用者手动给 shader 加过引用，需要自行调用 `xgeShaderFree`。

**范例代码：**

```c
xgeMaterialFree(&material);
```

**相关 API：**

- `xgeMaterialSetTexture`
- `xgeShaderFree`

---

### xgeMaterialSetShader

设置 material 使用的 shader。

**功能：**

你可以用它把自定义 shader 绑定到 material，后续 `xgeMaterialDraw` 会使用该 shader 绘制。

**函数原型：**

```c
XGE_API void xgeMaterialSetShader(xge_material pMaterial, xge_shader pShader);
```

**参数：**

- `pMaterial`：material 对象。
- `pShader`：shader 对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

`pShader` 是借用指针。函数不会自动增加 shader 引用计数，也不会释放旧 shader。

**补充说明：**

- 绘制期间 shader 必须保持有效。
- 如果需要跨对象共享生命周期，请调用者自行使用 `xgeShaderAddRef` / `xgeShaderFree`。

**范例代码：**

```c
xgeMaterialSetShader(&material, &shader);
```

**相关 API：**

- `xgeShaderCreate`
- `xgeMaterialDraw`

---

### xgeMaterialSetTexture

设置 material 使用的 texture。

**功能：**

你可以用它为 material 指定默认纹理；绘制时会优先使用 material texture，没有设置时再使用 `xge_draw_t` 中的 texture。

**函数原型：**

```c
XGE_API void xgeMaterialSetTexture(xge_material pMaterial, xge_texture pTexture);
```

**参数：**

- `pMaterial`：material 对象。
- `pTexture`：texture 对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

函数会释放旧 texture 引用，并对新 texture 调用 `xgeTextureAddRef`。material 释放时会对应 `xgeTextureFree`。

**补充说明：**

- 多次设置同一个 texture 不会重复增加引用。
- 传入 `NULL` 可清除 material texture。

**范例代码：**

```c
xgeMaterialSetTexture(&material, &texture);
```

**相关 API：**

- `xgeTextureAddRef`
- `xgeMaterialFree`

---

### xgeMaterialSetColor

设置 material 颜色。

**功能：**

你可以用它设置 tint 色或传给 shader 的 `uColor`。

**函数原型：**

```c
XGE_API void xgeMaterialSetColor(xge_material pMaterial, uint32_t iColor);
```

**参数：**

- `pMaterial`：material 对象。
- `iColor`：XGE 颜色值。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

material 绘制会把颜色拆成浮点 RGBA，并尝试设置 `uColor`。

**范例代码：**

```c
xgeMaterialSetColor(&material, XGE_COLOR_RGBA(255, 255, 255, 180));
```

**相关 API：**

- `xgeMaterialDraw`
- `xgeShaderUniform4f`

---

### xgeMaterialSetBlend

设置 material blend 模式。

**功能：**

你可以用它让某个 material 使用独立 blend 状态，而不是继承当前全局 blend。

**函数原型：**

```c
XGE_API void xgeMaterialSetBlend(xge_material pMaterial, int iBlend);
```

**参数：**

- `pMaterial`：material 对象。
- `iBlend`：`XGE_BLEND_*` 或 `XGE_MATERIAL_DEFAULT_BLEND`。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

`xgeMaterialDraw` 会在绘制前切换 blend，绘制后恢复原全局 blend。

**范例代码：**

```c
xgeMaterialSetBlend(&material, XGE_BLEND_ADD);
```

**相关 API：**

- `xgeBlendSet`
- `xgeBlendGet`

---

### xgeMaterialDraw

使用 material 绘制一个 quad。

**功能：**

你可以用它把 `xge_draw_t` 描述的 sprite 交给自定义 shader/material 绘制。

**函数原型：**

```c
XGE_API void xgeMaterialDraw(xge_material pMaterial, const xge_draw_t* pDraw);
```

**参数：**

- `pMaterial`：material 对象，必须设置有效 shader。
- `pDraw`：绘制描述，不能为 `NULL`。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。绘制期间 shader 和 texture 必须保持有效。

**补充说明：**

- 如果 material 设置了 texture，优先使用 material texture；否则使用 `pDraw->pTexture`。
- shader 需要可用的后端 program，texture 需要已上传到 GPU。
- 该函数无返回值，参数或后端状态无效时会直接跳过绘制。

**范例代码：**

```c
xge_draw_t draw;
memset(&draw, 0, sizeof(draw));
draw.pTexture = &texture;
draw.tDst = xgeRect(0.0f, 0.0f, 128.0f, 128.0f);
xgeMaterialDraw(&material, &draw);
```

**相关 API：**

- `xgeMaterialSetShader`
- `xgeMaterialSetTexture`
- `xgeDrawTexture`

---

### xgeMeshCreate

创建自定义 mesh。

**功能：**

你可以用它创建自定义顶点和索引网格，用于 2.5D、透视四边形、图像扭曲或特殊批处理。

**函数原型：**

```c
XGE_API int xgeMeshCreate(xge_mesh pMesh, const xge_vertex_t* pVertices, int iVertexCount, const uint16_t* pIndices, int iIndexCount, uint32_t iFlags);
```

**参数：**

- `pMesh`：输出 mesh 对象，不能为 `NULL`。
- `pVertices`：顶点数组，不能为 `NULL`。
- `iVertexCount`：顶点数量，必须大于 0。
- `pIndices`：索引数组，不能为 `NULL`。
- `iIndexCount`：索引数量，必须大于 0。
- `iFlags`：mesh 标志，例如 `XGE_MESH_DYNAMIC`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 内存不足返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

函数会复制顶点和索引数据。创建成功后调用者必须使用 `xgeMeshFree` 释放。

**补充说明：**

- 创建时不要求 GL 后端已经运行。
- 后端 buffer 会在绘制时按需创建或更新。

**范例代码：**

```c
xge_mesh_t mesh;
int ret = xgeMeshCreate(&mesh, vertices, vertex_count, indices, index_count, XGE_MESH_DYNAMIC);
```

**相关 API：**

- `xgeMeshUpdate`
- `xgeMeshDraw`
- `xgeMeshFree`

---

### xgeMeshUpdate

更新 mesh 顶点和索引数据。

**功能：**

你可以用它替换 mesh 的 CPU 数据，并保留原有 flags 和后端 buffer id。

**函数原型：**

```c
XGE_API int xgeMeshUpdate(xge_mesh pMesh, const xge_vertex_t* pVertices, int iVertexCount, const uint16_t* pIndices, int iIndexCount);
```

**参数：**

- `pMesh`：mesh 对象，不能为 `NULL`。
- `pVertices`：新顶点数组，不能为 `NULL`。
- `iVertexCount`：新顶点数量，必须大于 0。
- `pIndices`：新索引数组，不能为 `NULL`。
- `iIndexCount`：新索引数量，必须大于 0。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 内存不足返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

函数会复制输入数据。调用返回后输入数组可以释放或复用。

**补充说明：**

- 适合动态网格逐帧更新。
- 后端 buffer 会在下一次 `xgeMeshDraw` 时上传新数据。

**范例代码：**

```c
xgeMeshUpdate(&mesh, vertices, vertex_count, indices, index_count);
```

**相关 API：**

- `xgeMeshCreate`
- `xgeMeshDraw`

---

### xgeMeshFree

释放 mesh。

**功能：**

你可以用它释放 mesh 的 CPU 数据和后端 buffer。

**函数原型：**

```c
XGE_API void xgeMeshFree(xge_mesh pMesh);
```

**参数：**

- `pMesh`：mesh 对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `pMesh` 内部顶点、索引和 GL buffer，并清零对象。

**补充说明：**

- 对 `NULL` 调用是安全的。
- 如果 GL 后端已经关闭，只释放 CPU 数据。

**范例代码：**

```c
xgeMeshFree(&mesh);
```

**相关 API：**

- `xgeMeshCreate`
- `xgeMeshUpdate`

---

### xgeMeshDraw

绘制 mesh。

**功能：**

你可以用它使用 XGE 默认 2.5D mesh shader 绘制自定义顶点网格。

**函数原型：**

```c
XGE_API void xgeMeshDraw(xge_mesh pMesh, xge_texture pTexture, uint32_t iFlags);
```

**参数：**

- `pMesh`：mesh 对象。
- `pTexture`：绘制使用的纹理。
- `iFlags`：绘制标志，传给顶点投影逻辑。

**返回值：**

无。

**资源归属：**

函数不接管 mesh 或 texture 所有权。绘制期间两者必须保持有效。

**补充说明：**

- mesh 和 texture 无效时函数会直接跳过绘制。
- 当前实现会在绘制前上传 mesh 数据；动态 mesh 应控制顶点数量和更新频率。
- 顶点会经过 XGE camera 投影，除非绘制标志指定屏幕空间路径。

**范例代码：**

```c
xgeMeshDraw(&mesh, &texture, 0);
```

**相关 API：**

- `xgeMeshCreate`
- `xgeDrawQuad3D`
- `xgeCameraSet`

---

## Shader 兼容性

XGE 的图形基线覆盖 OpenGL 3.3、GLES3 和 WebGL2。不同后端 shader 头、precision、版本号和内建变量写法可能不同。建议：

- 用 `xgeGraphicsShaderHeaderGet` 获取当前后端 header。
- 避免依赖桌面 GL 独有扩展。
- WebGL2/GLES3 路径显式处理 precision。
- 将常用宏放进 Shader Variant，减少运行时字符串拼接。

## 生命周期与所有权

Shader、material、shader variant set、buffer 和 mesh 都由调用者显式管理。创建成功后使用对应 `Free` 函数释放。

Material 会引用 texture 并管理这份引用，但 shader 当前是借用指针。被 material 使用的 shader/texture 必须在绘制期间保持有效。

## 线程约束

Shader 编译、buffer 上传、mesh 绘制和 material draw 都涉及 GPU 状态，必须在 render owner 线程执行或通过命令队列提交。

业务线程可以准备 shader 源码、define 列表和 CPU 顶点数据，但不能直接跨线程调用底层 GL。

## 后端差异

自定义 shader 是后端差异最敏感的部分。OpenGL 3.3、GLES3 和 WebGL2 的 shader header、precision、内置变量和扩展支持不同。

Material API 只提供统一提交入口，不保证同一段 shader 源码在所有后端无需修改。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| shader 在桌面可用、Web 不可用 | GLSL 版本和 precision 不兼容 | 使用 `xgeGraphicsShaderHeaderGet` 并按后端写兼容代码。 |
| material 绘制后 shader 悬空 | `xgeMaterialSetShader` 不接管 shader 所有权 | 确保 shader 生命周期覆盖 material 绘制期。 |
| 动态 mesh 更新慢 | 顶点数过大或每帧重建静态 mesh | 使用 `XGE_MESH_DYNAMIC` 并控制更新规模。 |

## 相关示例

- `examples/shader_material`
- `examples/shader_distortion`
- `examples/perspective_quad`
- `build_shader_material_exe.bat`
