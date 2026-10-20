# XGE 3 图片坐标与迁移

XGE 3.0 将普通图片、二维离屏目标和三维离屏结果统一为逻辑图片：左上角是原点，X 向右，Y 向下。`xge_draw_t.tSrc` 始终使用这套像素坐标；公开的网格、四边形输入 UV 的 `(0,0)` 始终表示图片左上角。世界空间、相机和三维 Y-up 规则不变。

## 普通绘制

```c
xge_draw_t draw = {0};
draw.pTexture = xgeRenderTargetTexture(&target);
draw.tSrc = (xge_rect_t){10, 20, 100, 80}; // 从逻辑左上角取区域
draw.tDst = (xge_rect_t){0, 0, 100, 80};
draw.iColor = 0xffffffffu;
draw.iFlags = XGE_DRAW_SCREEN_SPACE;
xgeDrawEx(&draw); // 不需要 FLIP_Y
```

`XGE_DRAW_FLIP_X/Y` 表示对选中的逻辑图片区域主动镜像。源区域先确定，再镜像，最后映射存储方向。对于自定义网格/四边形，翻转标志作用于归一化的完整图片 UV；若要镜像一个自定义 UV 子区域，应交换该区域的顶点 UV 端点。

`xgeDrawEx`、`xgeMaterialDraw`、精灵批次、九宫格、四边形、网格和 XUI 都消费统一的方向元数据。创建二维或三维渲染目标、目标 resize、窗口捕获都会保留正确的标记。XUI 不再按目标类型二次补偿。

底层方向使用 `XGE_TEXTURE_ORIGIN_BOTTOM_LEFT` 记录；该标记由引擎维护。正常绘制只调整 UV，不翻转 CPU 图片、不读回显存、不增加全屏绘制。显式 CPU 上传到目标时会转换行顺序，输入和局部更新区域仍按 top-down 定义。

## 原始 GPU / 自定义着色器

`xgeTextureUVTransform(texture)` 返回 `native = logical * scale + offset`；`xgeTextureMapUV` 转换一个归一化坐标。不应在调用普通绘制前手工调用这些函数，否则会重复转换。

`XGE_DRAW_RAW_UV` 是明确的高级/旧代码入口，禁用自动方向映射。原始源矩形和 UV 按后端纹理坐标解释；可与主动翻转组合。正常 UI、缓存、预览不应使用它，也不存在会影响所有模块的全局兼容开关。

自定义 GLSL 保留对原始 `texture()` 采样的控制。材质收到的主纹理 UV 已转换为主纹理的原生采样坐标，单纹理着色器继续直接使用 `texture(uTexture, vUV)`。多纹理的存储方向可能不同：在版本/精度声明之后加入 `XGE_GLSL_MATERIAL_UV`，并通过对应辅助函数从主纹理采样坐标映射到其他槽位：

```c
const char *fragment =
    "#version 330 core\n"
    "in vec2 vUV; out vec4 FragColor;\n"
    "uniform sampler2D uTexture; uniform sampler2D uTexture2;\n"
    XGE_GLSL_MATERIAL_UV
    "void main(){\n"
    "  vec4 a = texture(uTexture, vUV);\n"
    "  vec4 b = texture(uTexture2, xgeMaterialUV2(vUV));\n"
    "  FragColor = (a+b)*0.5;\n"
    "}\n";
```

三个 `uTexture[2/3]UVTransform` 由材质提交自动绑定、位置缓存于 shader；无需应用逐帧查询/设置。辅助函数的输入是主纹理采样坐标。独立生成的查表 UV（例如渐变表）以及直接访问 GPU 句柄的代码，应按其自身的坐标约定处理。任意用户 GLSL 中手写的 `texture()` 调用不会被引擎字符串改写。

## 读回

`xgeRenderTargetReadPixels` 返回 top-down、straight RGBA8；`xge3dTargetReadPixels` 返回 top-down RGBA8，保留三维渲染结果的 alpha 语义。PNG 的输入行顺序也为 top-down。要检查最终显示，应读回窗口目标，不能用离屏截图代替窗口合成验证。

`xgeTextureReadPixels` 只读取 CPU 影子副本。已进入 GPU 环境的渲染目标会返回 `XGE_ERROR_INVALID_STATE`，避免将没有更新的影子副本当作截图。二维和三维目标分别使用各自的目标读回接口。

## 从 XGE 2 迁移

1. 配套升级 XGE 3 的头文件、DLL、导入库和 `xge_build_config.h`，重新编译调用方。`xge_shader_t` 增加缓存字段，不能混用旧 ABI。
2. 删除只为渲染目标补偿方向的 `FLIP_Y`；保留确实需要的图像镜像。
3. 删除源区域中的 `height - y - h` 补偿，恢复为逻辑图片坐标；删除四边形/网格中只为目标来源补偿的 `1-v`。
4. 旧代码若暂时无法迁移，可逐次绘制显式启用 `XGE_DRAW_RAW_UV`，并保留原来的补偿，待迁移后移除。
5. 多纹理自定义材质按上述辅助函数处理不同方向；标准引擎效果和 XUI 已同步迁移。

新 SDK 配置记录 `XGE_BUILD_API_MAJOR`，公开头文件会拒绝已声明但不匹配的 SDK 主版本。运行时 `xgeTextureCoordinateContract()` 应等于编译期 `XGE_TEXTURE_COORDINATE_CONTRACT`（当前为 2），也可用于调用方启动检查；这不是完整的第三方 DLL 身份校验。

## 验证

在 Windows 的真实 GPU 环境运行：

```bat
tests\xge\build_texture_orientation_test.bat
```

该测试覆盖非对称图片、裁剪与镜像矩阵、普通/材质/批次/网格/九宫格、三个材质槽的方向组合、嵌套目标、局部上传、目标 resize、XUI 裁剪和四边形、三维输出、最终窗口、窗口区域捕获、PNG 编码解码再上传。缺少 GPU 时失败，不以 unsupported 作为通过。

脚本还会故意移除生产端方向标记：必须在渲染像素比较中检测到倒置。最终窗口样本保存在 `build/full-dev/orientation-window.png`。现有纹理存储 GPU 测试和 3D GPU 套件用于验证其他渲染行为。
