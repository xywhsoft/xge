# Shader Variant Lab

`examples/shader_variant_lab` 覆盖 shader 创建、uniform 设置、引用计数和 variant set。范例创建一张程序纹理，并用基础 shader、暖色 variant、冷色 variant 三种路径绘制三块图像。

## 覆盖 API

- `xgeShaderCreate`
- `xgeShaderAddRef`
- `xgeShaderFree`
- `xgeShaderUniform1f`
- `xgeShaderUniform2f`
- `xgeShaderUniform3f`
- `xgeShaderUniform4f`
- `xgeShaderVariantSetInit`
- `xgeShaderVariantGet`
- `xgeShaderVariantSetFree`

## 构建和运行

```bat
examples\shader_variant_lab\build.bat
build\xge_shader_variant_lab.exe --frames 120
```

## 验证点

- 控制台输出 `shader-variant-lab init`。
- `uniforms=1/1/1/1` 表示 1f/2f/3f/4f uniform 都设置成功。
- `warm=1 cool=1 cache=1 count=2` 表示两个 variant 编译成功，同 key 二次获取命中缓存。
- 窗口中三块图像应呈现基础、暖色和冷色三种视觉差异。
