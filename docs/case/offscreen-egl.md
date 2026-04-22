# 离屏与板卡 EGL 范例

本案例展示在没有传统窗口的环境中创建 EGL context，执行离屏绘制，并读回像素。

[返回范例解析](README.md) | [离屏教程](../guide/offscreen-intro.md) | [Offscreen API](../api/offscreen.md)

## 问题

板卡 Linux、自动化测试和资源生成工具可能没有桌面窗口系统。XGE 需要支持 EGL pbuffer、GBM/DRM 或宿主提供 surface 的离屏路径。

## 步骤 1：查询 EGL 能力

```c
xge_egl_caps_t caps;

if ( xgeEGLCapsGet(&caps) != XGE_OK ) {
	return XGE_ERROR_BACKEND;
}
```

能力查询用于决定是否支持 pbuffer、surfaceless、GLES3 和目标像素格式。

## 步骤 2：创建 context

```c
xge_egl_context_t egl;
xge_egl_desc_t desc;

memset(&desc, 0, sizeof(desc));
desc.iWidth = 512;
desc.iHeight = 512;
desc.iFlags = XGE_EGL_PBUFFER;

if ( xgeEGLInit(&egl, &desc) != XGE_OK ) {
	return XGE_ERROR_BACKEND;
}
xgeEGLMakeCurrent(&egl);
```

板卡 GBM/DRM 路径可以把 native display/surface 放入 desc，由平台后端解释。

## 步骤 3：创建 offscreen

```c
xge_offscreen_t offscreen;
xge_render_target target;

xgeOffscreenInit(&offscreen, 512, 512);
target = xgeOffscreenRenderTarget(&offscreen);
```

之后使用 RenderTarget/pass API 绘制即可。

## 步骤 4：绘制并读回

```c
xge_pass_t pass;
uint32_t pixels[512 * 512];

xgePassInit(&pass, target, XGE_CLEAR_COLOR, xgeColorRGBA(0, 0, 0, 255));
xgePassBegin(&pass);
xgeShapeRectFill((xge_rect_t){ 64.0f, 64.0f, 256.0f, 128.0f }, xgeColorRGBA(240, 180, 64, 255));
xgePassEnd(&pass);

xgeOffscreenReadPixels(&offscreen, pixels, 512 * 4);
```

读回后的像素可以保存成 PNG，也可以做自动化像素检查。

## 步骤 5：释放

```c
xgeOffscreenUnit(&offscreen);
xgeEGLUnit(&egl);
```

## 关键 API

| API | 作用 |
| --- | --- |
| `xgeEGLCapsGet` | 查询 EGL 能力 |
| `xgeEGLInit` | 创建 EGL context |
| `xgeEGLMakeCurrent` | 绑定当前线程 |
| `xgeOffscreenInit` | 创建离屏目标 |
| `xgeOffscreenRenderTarget` | 获取 RenderTarget |
| `xgeOffscreenReadPixels` | 读回像素 |

## 常见问题

如果 `xgeEGLInit` 失败，先检查板卡驱动是否提供 EGL/GLES3，而不是只安装了桌面 OpenGL 包。

如果读回全黑，确认 pass begin/end 成功，并且当前线程的 EGL context 没有被其他线程抢占。
