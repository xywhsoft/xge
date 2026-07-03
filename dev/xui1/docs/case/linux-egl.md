# Linux EGL 窗口或离屏

本案例说明 Linux EGL 路径的接入和验证重点。

[返回范例解析](README.md) | [离屏教程](../guide/offscreen-intro.md) | [Offscreen API](../api/offscreen.md)

## 状态

该页面描述 EGL 接入策略，具体发行版和驱动组合需要单独记录验证结果。

## 目标

```text
EGL display
  -> window surface / pbuffer / surfaceless
  -> GLES3 context
  -> XGE render path
```

## 初始化

```c
xge_egl_context_t egl;
xge_egl_desc_t desc;

memset(&desc, 0, sizeof(desc));
desc.iWidth = 800;
desc.iHeight = 600;

if ( xgeEGLInit(&egl, &desc) != XGE_OK ) {
	return XGE_ERROR_BACKEND;
}

xgeEGLMakeCurrent(&egl);
```

## 验证重点

- EGL display/config/context 是否创建成功。
- GLES 版本是否达到 3.0。
- 当前线程是否拥有 context。
- FBO/RenderTarget 是否可用。
- read pixels 是否能读到非黑图像。

## 常见失败原因

系统有 OpenGL 但缺少 EGL/GLES 驱动。

使用了错误的 native display 或 surface。

context 在错误线程 current。
