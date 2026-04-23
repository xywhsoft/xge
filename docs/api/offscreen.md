# Offscreen / EGL API

> Offscreen / EGL API 负责 EGL 能力查询、EGL context 生命周期、pbuffer/surfaceless 和 XGE 离屏渲染对象。

[返回 API 索引](README.md) | [离屏教程](../guide/offscreen-intro.md) | [离屏范例](../case/offscreen-egl.md)

---

## 模块定位

Offscreen / EGL 用于没有常规窗口的渲染场景，例如板卡 Linux、服务端截图、自动化测试、资源生成、CI/headless 环境和工具链离屏输出。

它和 [Render Target API](render-target.md) 的关系：

- EGL 负责创建和管理 GL/GLES context。
- Offscreen 负责把 EGL context 和 XGE render target 组合成可绘制对象。
- RenderTarget/Pass 负责实际绘制目标切换、清屏、读回和纹理访问。

## 标准调用顺序

使用 XGE offscreen 封装：

```text
xgeOffscreenInit
  -> xgeOffscreenRenderTarget
  -> xgePassBegin / 绘制 / xgePassEnd
  -> xgeOffscreenReadPixels
  -> xgeOffscreenUnit
```

直接使用 EGL：

```text
xgeEGLCapsGet
  -> xgeEGLInit
  -> xgeEGLMakeCurrent
  -> 创建 RT / 绘制 / 读回
  -> xgeEGLUnit
```

## 公共类型

### `xge_egl_desc_t`

| 字段 | 说明 |
| --- | --- |
| `pNativeDisplay` | 原生 display，平台相关。 |
| `pNativeWindow` | 原生 window，可为空。 |
| `iWidth` / `iHeight` | pbuffer/offscreen 尺寸，必须大于 0。 |
| `bPBuffer` | 请求 pbuffer surface。 |
| `bSurfaceless` | 请求 surfaceless context。 |
| `bBoardLinux` | 板卡 Linux 路径标记。 |
| `pUser` | 用户数据。 |

### `xge_egl_caps_t`

| 字段 | 说明 |
| --- | --- |
| `bCompiled` | 当前构建是否编译 EGL 支持。 |
| `bOffscreen` | 是否支持 offscreen。 |
| `bPBuffer` | 是否支持 pbuffer。 |
| `bSurfaceless` | 是否支持 surfaceless。 |
| `bNativeWindow` | 是否支持 native window surface。 |
| `bBoardLinux` | 是否为板卡 Linux 能力路径。 |
| `iLastError` | 最近错误码。 |
| `sBackendName` | 后端名称。 |
| `sLastStage` | 最近阶段说明。 |

### `xge_egl_context_t`

EGL context 状态。字段透明用于诊断，但用户不应直接修改 `pDisplay`、`pConfig`、`pSurface`、`pContext`。

### `xge_offscreen_t`

| 字段 | 说明 |
| --- | --- |
| `iWidth` / `iHeight` | 离屏尺寸。 |
| `tEGL` | 内部 EGL context。 |
| `tTarget` | 内部 render target。 |
| `bActive` | 是否初始化完成。 |
| `bEGLContext` | 是否持有 EGL context。 |
| `bFallbackRenderTarget` | 是否使用 fallback RT。 |

## API 参考

### xgeEGLCapsGet

查询 EGL/offscreen 编译能力。

**功能：**

你可以用它在运行前检查当前构建是否编译了 EGL、pbuffer、surfaceless、native window 或板卡 Linux 支持。

**函数原型：**

```c
XGE_API int xgeEGLCapsGet(xge_egl_caps_t* pCaps);
```

**参数：**

- `pCaps`：输出能力结构，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- `pCaps` 为 `NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

函数只写入 `pCaps`，不分配资源。

**补充说明：**

- 未编译 EGL 时，`bCompiled` 为 0，`sBackendName` 为 `egl-unavailable`。
- 该函数反映编译能力，不等价于目标机器驱动一定可用。

**范例代码：**

```c
xge_egl_caps_t caps;
if (xgeEGLCapsGet(&caps) == XGE_OK && caps.bPBuffer) {
    /* 可以尝试 pbuffer */
}
```

**相关 API：**

- `xgeEGLInit`

---

### xgeEGLInit

初始化 EGL context。

**功能：**

你可以用它创建 GLES3 EGL context，并按描述创建 pbuffer、surfaceless 或 native window surface。

**函数原型：**

```c
XGE_API int xgeEGLInit(xge_egl_context_t* pContext, const xge_egl_desc_t* pDesc);
```

**参数：**

- `pContext`：输出 EGL context，不能为 `NULL`。
- `pDesc`：初始化描述，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 当前构建或请求 surface 类型不支持返回 `XGE_ERROR_UNSUPPORTED`。
- EGL display、config、surface、context 或 make current 失败返回 `XGE_ERROR_BACKEND_FAILED`。

**资源归属：**

创建成功后 `pContext` 持有 EGL display、surface 和 context，调用者必须使用 `xgeEGLUnit` 释放。

**补充说明：**

- `iWidth` / `iHeight` 必须大于 0。
- 函数会把平台后端设置为 EGL，把图形后端设置为 GLES30。
- 失败阶段会写入 `pContext->sLastStage`，EGL 错误会写入 `iLastError` 和 XRT 错误信息。

**范例代码：**

```c
xge_egl_desc_t desc;
xge_egl_context_t egl;
memset(&desc, 0, sizeof(desc));
desc.iWidth = 512;
desc.iHeight = 512;
desc.bPBuffer = 1;
xgeEGLInit(&egl, &desc);
```

**相关 API：**

- `xgeEGLCapsGet`
- `xgeEGLMakeCurrent`
- `xgeEGLUnit`

---

### xgeEGLUnit

销毁 EGL context。

**功能：**

你可以用它释放 `xgeEGLInit` 创建的 EGL context、surface 和 display。

**函数原型：**

```c
XGE_API void xgeEGLUnit(xge_egl_context_t* pContext);
```

**参数：**

- `pContext`：EGL context，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `pContext` 内部 EGL 资源，并清零结构体。

**补充说明：**

- 对 `NULL` 调用是安全的。
- 释放时会先把当前线程 EGL context 解绑。

**范例代码：**

```c
xgeEGLUnit(&egl);
```

**相关 API：**

- `xgeEGLInit`

---

### xgeEGLMakeCurrent

将 EGL context 绑定到当前线程。

**功能：**

你可以在专用 render thread 或线程切换后调用它，让当前线程拥有该 GL context。

**函数原型：**

```c
XGE_API int xgeEGLMakeCurrent(xge_egl_context_t* pContext);
```

**参数：**

- `pContext`：已初始化的 EGL context。

**返回值：**

- 成功返回 `XGE_OK`。
- `pContext` 为 `NULL` 返回 `XGE_ERROR_INVALID_ARGUMENT`。
- context 未初始化返回 `XGE_ERROR_NOT_INITIALIZED`。
- 后端 make current 失败返回 `XGE_ERROR_BACKEND_FAILED`。

**资源归属：**

函数不接管资源。

**补充说明：**

不要在多个线程同时 make current 同一个 EGL context。

**范例代码：**

```c
xgeEGLMakeCurrent(&egl);
```

**相关 API：**

- `xgeEGLInit`
- `xgeEGLUnit`

---

### xgeOffscreenInit

初始化离屏渲染对象。

**功能：**

你可以用它创建一个 XGE offscreen 对象。函数会优先创建 EGL pbuffer context；如果 EGL 不支持，会退回仅创建 render target 的 fallback 路径。

**函数原型：**

```c
XGE_API int xgeOffscreenInit(xge_offscreen pOffscreen, int iWidth, int iHeight);
```

**参数：**

- `pOffscreen`：输出离屏对象，不能为 `NULL`。
- `iWidth`：离屏宽度，必须大于 0。
- `iHeight`：离屏高度，必须大于 0。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- EGL 初始化发生非 unsupported 错误时返回对应错误码。
- render target 创建失败时返回对应错误码。

**资源归属：**

创建成功后 `pOffscreen` 持有内部 EGL context 和 render target，调用者必须使用 `xgeOffscreenUnit` 释放。

**补充说明：**

- `bEGLContext` 表示是否成功持有 EGL context。
- `bFallbackRenderTarget` 表示 EGL 不支持时使用了 fallback RT。
- fallback 路径仍依赖已有可用渲染上下文才能真正绘制。

**范例代码：**

```c
xge_offscreen_t offscreen;
if (xgeOffscreenInit(&offscreen, 512, 512) == XGE_OK) {
    /* draw */
}
```

**相关 API：**

- `xgeOffscreenRenderTarget`
- `xgeOffscreenReadPixels`
- `xgeOffscreenUnit`

---

### xgeOffscreenUnit

释放离屏对象。

**功能：**

你可以用它释放 offscreen 内部 render target 和 EGL context，并清零对象。

**函数原型：**

```c
XGE_API void xgeOffscreenUnit(xge_offscreen pOffscreen);
```

**参数：**

- `pOffscreen`：离屏对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `pOffscreen` 内部资源，不释放结构体本身。

**补充说明：**

对 `NULL` 调用是安全的。

**范例代码：**

```c
xgeOffscreenUnit(&offscreen);
```

**相关 API：**

- `xgeOffscreenInit`

---

### xgeOffscreenRenderTarget

获取离屏 render target。

**功能：**

你可以用它取得 offscreen 内部 render target，然后通过 `xgePassBegin` / `xgePassEnd` 绘制到离屏目标。

**函数原型：**

```c
XGE_API xge_render_target xgeOffscreenRenderTarget(xge_offscreen pOffscreen);
```

**参数：**

- `pOffscreen`：离屏对象。

**返回值：**

- 有效且 active 时返回内部 render target。
- 参数非法或未初始化时返回 `NULL`。

**资源归属：**

返回值是借用指针，所有权仍属于 `pOffscreen`，不能单独释放。

**补充说明：**

返回目标随 `xgeOffscreenUnit` 失效。

**范例代码：**

```c
xge_render_target target = xgeOffscreenRenderTarget(&offscreen);
```

**相关 API：**

- `xgePassInit`
- `xgePassBegin`
- `xgeOffscreenReadPixels`

---

### xgeOffscreenReadPixels

读回离屏像素。

**功能：**

你可以用它把 offscreen render target 的 RGBA8 像素读回 CPU，用于截图、测试或资源生成。

**函数原型：**

```c
XGE_API int xgeOffscreenReadPixels(xge_offscreen pOffscreen, void* pPixels, int iStride);
```

**参数：**

- `pOffscreen`：离屏对象。
- `pPixels`：输出像素缓冲。
- `iStride`：每行字节数，小于等于 0 时由 render target 读回逻辑使用默认 stride。

**返回值：**

- 成功返回 `XGE_OK`。
- offscreen 无效或未初始化返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 其他错误返回 `xgeRenderTargetReadPixels` 的错误码。

**资源归属：**

调用者提供并持有 `pPixels`。

**补充说明：**

读回会触发 GPU/CPU 同步，不适合高频运行时路径。

**范例代码：**

```c
unsigned char pixels[512 * 512 * 4];
xgeOffscreenReadPixels(&offscreen, pixels, 512 * 4);
```

**相关 API：**

- `xgeRenderTargetReadPixels`
- `xgeOffscreenRenderTarget`

---

## pbuffer、surfaceless 与板卡 Linux

| 模式 | 说明 | 适用场景 |
| --- | --- | --- |
| pbuffer | 创建不可见 EGL surface | 通用离屏渲染，兼容性较好。 |
| surfaceless | 无 surface context | 支持该扩展的平台可减少 surface 管理。 |
| native window | 绑定原生窗口 surface | 板卡或嵌入式窗口系统。 |
| board linux | 面向 RK3566 等近年板卡的 EGL/GLES3 路径 | 无桌面环境或轻量窗口系统。 |

是否实际可用必须看目标机器驱动和 EGL 扩展。正式验证结果记录在 `dev/docs/平台后端验证结果.md`。

## 生命周期与所有权

`xgeEGLInit` 成功后，调用者必须用 `xgeEGLUnit` 释放 context。`xgeOffscreenInit` 成功后，调用者必须用 `xgeOffscreenUnit` 释放 offscreen 目标。

`xgeOffscreenRenderTarget` 返回的是 offscreen 内部 RenderTarget 句柄，不转移所有权，不应单独释放。

## 线程约束

EGL context 必须在明确 current 的线程使用。Offscreen 创建、绘制和 readback 都应在 context owner 线程执行，或由专用 render thread 统一处理。

不要在多个线程同时 make current 同一个 context。

## 后端差异

EGL pbuffer、surfaceless、GBM/KMS 和 native window surface 的支持取决于驱动。桌面开发机通过不代表板卡通过。

不同驱动的 framebuffer 格式、readback 行对齐和扩展能力可能不同。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| `xgeEGLInit` 失败 | 驱动不支持请求的 surface 类型 | 先调用 `xgeEGLCapsGet`，在 pbuffer/surfaceless/native window 间切换。 |
| render thread 中 GL 错误 | context 绑定在线程不一致 | 在拥有 context 的线程调用 `xgeEGLMakeCurrent`。 |
| 读回图像为空 | 没有绘制到 offscreen RT 或没有结束 pass | 获取 `xgeOffscreenRenderTarget` 后用 pass 正确绘制。 |

## 相关示例

- `examples/offscreen`
- `examples/egl_surfaceless`
- `examples/egl_surfaceless/build.sh`
- `examples/offscreen/build_board_linux_egl.sh`
