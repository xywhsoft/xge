# Render Target API

> Render Target API 负责窗口渲染目标、离屏纹理目标、buffer 和 render pass。

[返回 API 索引](README.md) | [RenderTarget 教程](../guide/render-target-intro.md) | [RenderTarget 范例](../case/render-target.md)

---

## 模块定位

RenderTarget 是 XGE 渲染输出的目标对象。窗口本身可以包装成 render target，离屏纹理也可以创建为 render target。Pass 用于切换输出目标，并在结束时恢复 framebuffer、窗口尺寸、viewport 和 camera 状态。

EGL context、pbuffer、surfaceless 和完整 offscreen 生命周期不在本页展开，后续放在 `offscreen.md`。本页只描述通用 RT、buffer 和 pass。

## 标准调用顺序

```text
xgeRenderTargetCreate 或 xgeRenderTargetWindow
  -> xgePassInit
  -> xgePassBegin
    -> 绘制命令
  -> xgePassEnd
  -> 可选：xgeRenderTargetTexture / xgeRenderTargetReadPixels
  -> xgeRenderTargetFree
```

窗口 RT 通常由默认渲染流程管理。自建 RT 适合后处理、缩略图、动态贴图、UI 缓存和工具截图。

## 常量与宏

| 名称 | 值 | 说明 |
| --- | --- | --- |
| `XGE_RENDER_TARGET_WINDOW` | `0x0001` | 窗口 render target。 |
| `XGE_RENDER_TARGET_TEXTURE` | `0x0002` | 纹理 render target。 |
| `XGE_PASS_CLEAR_COLOR` | `0x0001` | pass 开始时清颜色。 |
| `XGE_BUFFER_VERTEX` | `1` | 顶点 buffer。 |
| `XGE_BUFFER_INDEX` | `2` | 索引 buffer。 |
| `XGE_BUFFER_UNIFORM` | `3` | uniform buffer，当前实现作为 CPU 暂存数据。 |
| `XGE_BUFFER_STATIC` | `1` | 静态 buffer。 |
| `XGE_BUFFER_DYNAMIC` | `2` | 动态 buffer。 |

## 公共类型

### `xge_render_target_t`

| 字段 | 说明 |
| --- | --- |
| `iWidth` / `iHeight` | render target 尺寸。 |
| `iFlags` | `XGE_RENDER_TARGET_*`。 |
| `iFramebufferId` | 后端 framebuffer id。 |
| `tTexture` | 纹理目标持有的纹理。 |

### `xge_pass_t`

| 字段 | 说明 |
| --- | --- |
| `pTarget` | pass 输出目标。 |
| `iClearFlags` | `XGE_PASS_*`。 |
| `iClearColor` | clear color。 |
| `bActive` | pass 是否正在执行。 |

其他 `iPrev*` 字段用于保存前一个 framebuffer、viewport 和 camera 状态，用户不应直接修改。

### `xge_buffer_t`

| 字段 | 说明 |
| --- | --- |
| `iType` | `XGE_BUFFER_VERTEX`、`XGE_BUFFER_INDEX` 或 `XGE_BUFFER_UNIFORM`。 |
| `iUsage` | `XGE_BUFFER_STATIC` 或 `XGE_BUFFER_DYNAMIC`。 |
| `iSize` | buffer 大小。 |
| `iBackendId` | 后端 buffer id。 |
| `pData` | CPU 侧数据。 |

## API 参考

### xgeRenderTargetWindow

将当前窗口 framebuffer 包装为 render target。

**功能：**

你可以用它把默认窗口作为 `xge_pass_t` 的目标，从而复用 pass API 管理窗口绘制流程。

**函数原型：**

```c
XGE_API int xgeRenderTargetWindow(xge_render_target pTarget);
```

**参数：**

- `pTarget`：输出 render target 对象，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- `pTarget` 为 `NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

该函数不创建新的 GPU 资源。`pTarget` 由调用者持有，可以调用 `xgeRenderTargetFree` 清空，但不会释放窗口本身。

**补充说明：**

- 宽高来自当前 XGE 窗口尺寸。
- 该目标标记为 `XGE_RENDER_TARGET_WINDOW`。

**范例代码：**

```c
xge_render_target_t window_target;
if (xgeRenderTargetWindow(&window_target) == XGE_OK) {
    /* window_target 可用于 xgePassInit */
}
```

**相关 API：**

- `xgePassInit`
- `xgePassBegin`
- `xgePassEnd`

---

### xgeRenderTargetCreate

创建一个纹理 render target。

**功能：**

你可以用它创建离屏绘制目标，用于后处理、动态贴图、UI 缓存、截图或测试读回。

**函数原型：**

```c
XGE_API int xgeRenderTargetCreate(xge_render_target pTarget, int iWidth, int iHeight);
```

**参数：**

- `pTarget`：输出 render target 对象，不能为 `NULL`。
- `iWidth`：目标宽度，必须大于 0。
- `iHeight`：目标高度，必须大于 0。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 纹理创建、上传或 FBO 创建失败时返回对应错误码。

**资源归属：**

创建成功后，`pTarget` 持有内部纹理和可能的 framebuffer。调用者必须使用 `xgeRenderTargetFree` 释放。

**补充说明：**

- 如果渲染后端已经运行，会立即确保 framebuffer 可用。
- 如果后端尚未运行，目标仍可创建，后续进入渲染线程时再创建 GPU 侧资源。

**范例代码：**

```c
xge_render_target_t target;
int ret = xgeRenderTargetCreate(&target, 512, 512);
if (ret != XGE_OK) {
    return ret;
}
```

**相关 API：**

- `xgeRenderTargetFree`
- `xgeRenderTargetTexture`
- `xgePassBegin`

---

### xgeRenderTargetResize

调整 render target 尺寸。

**功能：**

你可以在窗口大小变化、动态贴图尺寸变化或 UI 缓存重建时调用它，让目标对象保持有效。

**函数原型：**

```c
XGE_API int xgeRenderTargetResize(xge_render_target pTarget, int iWidth, int iHeight);
```

**参数：**

- `pTarget`：要调整的 render target，不能为 `NULL`。
- `iWidth`：新宽度，必须大于 0。
- `iHeight`：新高度，必须大于 0。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 纹理目标重建失败时返回对应错误码。

**资源归属：**

窗口目标只更新宽高。纹理目标会释放旧资源并重新创建，调用者继续持有同一个 `pTarget` 对象。

**补充说明：**

- 不要直接修改 `iWidth` / `iHeight` 来 resize 纹理目标。
- resize 会丢弃旧纹理内容。

**范例代码：**

```c
if (xgeRenderTargetResize(&target, new_width, new_height) != XGE_OK) {
    /* 处理重建失败 */
}
```

**相关 API：**

- `xgeRenderTargetCreate`
- `xgeRenderTargetFree`

---

### xgeRenderTargetReadPixels

从 render target 读回 RGBA 像素。

**功能：**

你可以用它实现截图、离屏渲染验证、工具导出或自动化冒烟测试。

**函数原型：**

```c
XGE_API int xgeRenderTargetReadPixels(xge_render_target pTarget, void* pPixels, int iStride);
```

**参数：**

- `pTarget`：读取来源，不能为 `NULL`。
- `pPixels`：输出像素缓冲，不能为 `NULL`。
- `iStride`：每行字节数。传入小于等于 0 时使用 `width * 4`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 后端未初始化且无法从 CPU 纹理读取时返回 `XGE_ERROR_NOT_INITIALIZED`。
- 当前 GL 后端不支持读回时返回 `XGE_ERROR_UNSUPPORTED`。
- 临时内存不足时返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

调用者提供并继续持有 `pPixels`。函数内部临时内存会在返回前释放。

**补充说明：**

- 输出格式为 RGBA8。
- GL 路径会做垂直翻转，使输出按 XGE 的 2D 坐标习惯排列。
- 读回会同步 GPU/CPU，不适合每帧高频调用。

**范例代码：**

```c
unsigned char pixels[256 * 256 * 4];
if (xgeRenderTargetReadPixels(&target, pixels, 256 * 4) == XGE_OK) {
    /* pixels 中是 RGBA8 数据 */
}
```

**相关 API：**

- `xgeTextureReadPixels`
- `xgePassEnd`

---

### xgeRenderTargetTexture

获取纹理 render target 内部纹理。

**功能：**

你可以把离屏渲染结果作为普通 texture 继续绘制到窗口或其他目标。

**函数原型：**

```c
XGE_API xge_texture xgeRenderTargetTexture(xge_render_target pTarget);
```

**参数：**

- `pTarget`：render target 对象。

**返回值：**

- 纹理目标返回内部 `xge_texture` 句柄。
- `pTarget` 为 `NULL` 或不是纹理目标时返回 `NULL`。

**资源归属：**

返回值是借用指针，所有权仍属于 `pTarget`。调用者不能单独释放该纹理。

**补充说明：**

- 返回纹理会随 `xgeRenderTargetFree` 一起失效。
- 如果需要长期保存结果，应复制或重新绘制到独立 texture。

**范例代码：**

```c
xge_texture texture = xgeRenderTargetTexture(&target);
if (texture != NULL) {
    xgeDrawTexture(texture, 0.0f, 0.0f);
}
```

**相关 API：**

- `xgeRenderTargetCreate`
- `xgeDrawTexture`

---

### xgeRenderTargetFree

释放 render target 持有的资源。

**功能：**

你可以用它释放离屏纹理、framebuffer，并把对象清零为未初始化状态。

**函数原型：**

```c
XGE_API void xgeRenderTargetFree(xge_render_target pTarget);
```

**参数：**

- `pTarget`：要释放的 render target，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `pTarget` 内部持有的 texture 和 framebuffer。不会释放 `pTarget` 结构体本身。

**补充说明：**

- 对 `NULL` 调用是安全的。
- 如果当前 framebuffer 正是该目标，释放时会尝试绑定回默认 framebuffer。

**范例代码：**

```c
xgeRenderTargetFree(&target);
```

**相关 API：**

- `xgeRenderTargetCreate`
- `xgeRenderTargetResize`

---

### xgeBufferCreate

创建通用 buffer。

**功能：**

你可以用它为高级渲染扩展准备 vertex、index 或 uniform 数据。普通 sprite/shape 绘制通常不需要直接使用该 API。

**函数原型：**

```c
XGE_API int xgeBufferCreate(xge_buffer pBuffer, int iType, int iUsage, const void* pData, int iSize);
```

**参数：**

- `pBuffer`：输出 buffer 对象，不能为 `NULL`。
- `iType`：`XGE_BUFFER_VERTEX`、`XGE_BUFFER_INDEX` 或 `XGE_BUFFER_UNIFORM`。
- `iUsage`：`XGE_BUFFER_STATIC` 或 `XGE_BUFFER_DYNAMIC`。
- `pData`：初始数据，可以为 `NULL`。
- `iSize`：buffer 字节数，必须大于 0。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 内存不足返回 `XGE_ERROR_OUT_OF_MEMORY`。
- 上传失败时返回 `xgeBufferUpload` 的错误码。

**资源归属：**

创建成功后，`pBuffer` 持有 CPU 数据副本，并可能持有 GPU buffer。调用者必须使用 `xgeBufferFree` 释放。

**补充说明：**

- `pData` 为 `NULL` 时，CPU 数据会被清零。
- 当前实现会在 GL 已运行且类型不是 `XGE_BUFFER_UNIFORM` 时立即上传。
- `XGE_BUFFER_UNIFORM` 当前保留为 CPU 数据路径。

**范例代码：**

```c
float vertices[8] = {0};
xge_buffer_t buffer;
int ret = xgeBufferCreate(&buffer, XGE_BUFFER_VERTEX, XGE_BUFFER_DYNAMIC, vertices, sizeof(vertices));
```

**相关 API：**

- `xgeBufferUpdate`
- `xgeBufferUpload`
- `xgeBufferFree`

---

### xgeBufferUpdate

更新 buffer 的一段数据。

**功能：**

你可以用它更新动态顶点、索引或 uniform 暂存数据，并在 GL 后端已运行时同步更新 GPU buffer。

**函数原型：**

```c
XGE_API int xgeBufferUpdate(xge_buffer pBuffer, int iOffset, const void* pData, int iSize);
```

**参数：**

- `pBuffer`：要更新的 buffer。
- `iOffset`：写入偏移，必须大于等于 0。
- `pData`：输入数据，不能为 `NULL`。
- `iSize`：写入字节数，必须大于 0。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法、范围越界或 buffer 未创建时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 后端不支持该 buffer 类型时返回 `XGE_ERROR_UNSUPPORTED`。
- GPU 更新失败返回 `XGE_ERROR_GPU_FAILED`。

**资源归属：**

函数会复制 `pData` 内容，调用返回后输入数据可以释放或复用。

**补充说明：**

- `iOffset + iSize` 不能超过 `pBuffer->iSize`。
- GL 已运行且类型不是 uniform 时，会调用后端子数据更新。

**范例代码：**

```c
float value[4] = {1.0f, 0.0f, 0.0f, 1.0f};
xgeBufferUpdate(&buffer, 0, value, sizeof(value));
```

**相关 API：**

- `xgeBufferCreate`
- `xgeBufferUpload`

---

### xgeBufferUpload

上传 buffer CPU 数据到 GPU。

**功能：**

你可以在延迟创建 GL 资源、后端刚启动或需要强制重建 GPU buffer 时调用它。

**函数原型：**

```c
XGE_API int xgeBufferUpload(xge_buffer pBuffer);
```

**参数：**

- `pBuffer`：要上传的 buffer。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 当前 buffer 类型不支持上传时返回 `XGE_ERROR_UNSUPPORTED`。
- GL 后端未初始化返回 `XGE_ERROR_NOT_INITIALIZED`。
- GPU 创建或上传失败返回 `XGE_ERROR_GPU_FAILED`。

**资源归属：**

上传成功后，`pBuffer` 持有后端 buffer id。仍由 `xgeBufferFree` 统一释放。

**补充说明：**

- 当前只有 vertex/index buffer 映射到 GL buffer target。
- 上传使用 `XGE_BUFFER_STATIC` 或 `XGE_BUFFER_DYNAMIC` 决定后端 usage。

**范例代码：**

```c
int ret = xgeBufferUpload(&buffer);
if (ret != XGE_OK) {
    /* 后端可能尚未初始化 */
}
```

**相关 API：**

- `xgeBufferCreate`
- `xgeBufferUpdate`

---

### xgeBufferFree

释放 buffer。

**功能：**

你可以用它释放 CPU 数据和后端 buffer id，并将对象清零。

**函数原型：**

```c
XGE_API void xgeBufferFree(xge_buffer pBuffer);
```

**参数：**

- `pBuffer`：要释放的 buffer，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `pBuffer` 内部资源，不释放结构体本身。

**补充说明：**

- 对 `NULL` 调用是安全的。
- 如果 GL 后端已经关闭，函数只释放 CPU 数据并清零对象。

**范例代码：**

```c
xgeBufferFree(&buffer);
```

**相关 API：**

- `xgeBufferCreate`

---

### xgePassInit

初始化 render pass 描述。

**功能：**

你可以用它把目标、clear 标志和 clear color 组合成一个可执行 pass。

**函数原型：**

```c
XGE_API void xgePassInit(xge_pass pPass, xge_render_target pTarget, uint32_t iClearFlags, uint32_t iClearColor);
```

**参数：**

- `pPass`：输出 pass 对象，可以为 `NULL`。
- `pTarget`：输出目标。
- `iClearFlags`：clear 标志，例如 `XGE_PASS_CLEAR_COLOR`。
- `iClearColor`：clear 颜色，使用 XGE 颜色格式。

**返回值：**

无。

**资源归属：**

`pPass` 借用 `pTarget` 指针，不接管目标所有权。目标必须在 pass begin/end 期间保持有效。

**补充说明：**

- `pPass` 为 `NULL` 时函数直接返回。
- 该函数不创建 GPU 资源。

**范例代码：**

```c
xge_pass_t pass;
xgePassInit(&pass, &target, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(0, 0, 0, 255));
```

**相关 API：**

- `xgePassBegin`
- `xgePassEnd`

---

### xgePassBegin

开始 render pass。

**功能：**

你可以用它切换到指定 render target，并让后续绘制命令输出到该目标。

**函数原型：**

```c
XGE_API int xgePassBegin(xge_pass pPass);
```

**参数：**

- `pPass`：已初始化的 pass，不能为 `NULL`，且必须包含有效 target。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- pass 已经处于 active 状态时返回 `XGE_ERROR_ALREADY_INITIALIZED`。
- 渲染后端未运行返回 `XGE_ERROR_NOT_INITIALIZED`。
- framebuffer 创建失败返回 `XGE_ERROR_GPU_FAILED`。

**资源归属：**

函数不会接管目标所有权。它会在 `pPass` 中保存前一个 framebuffer、尺寸、viewport 和 camera 状态。

**补充说明：**

- `xgePassBegin` 必须与 `xgePassEnd` 配对。
- 开始 pass 后，XGE 会把 camera 重置为目标尺寸对应的默认相机。
- 如果设置了 `XGE_PASS_CLEAR_COLOR`，会立即清除颜色缓冲。

**范例代码：**

```c
if (xgePassBegin(&pass) == XGE_OK) {
    /* draw into target */
    xgePassEnd(&pass);
}
```

**相关 API：**

- `xgePassInit`
- `xgePassEnd`
- `xgeClear`

---

### xgePassEnd

结束 render pass。

**功能：**

你可以用它恢复 `xgePassBegin` 前保存的 framebuffer、窗口尺寸、viewport 和 camera 状态。

**函数原型：**

```c
XGE_API int xgePassEnd(xge_pass pPass);
```

**参数：**

- `pPass`：当前 active 的 pass。

**返回值：**

- 成功返回 `XGE_OK`。
- `pPass` 为 `NULL` 或未处于 active 状态时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 渲染后端未运行返回 `XGE_ERROR_NOT_INITIALIZED`。

**资源归属：**

函数不释放目标或 pass，只恢复全局渲染状态并把 `bActive` 置为 0。

**补充说明：**

- 每次成功 `xgePassBegin` 后都应调用一次 `xgePassEnd`。
- 嵌套 pass 需要非常谨慎，确保状态恢复顺序严格匹配。

**范例代码：**

```c
int ret = xgePassEnd(&pass);
if (ret != XGE_OK) {
    /* pass 状态不匹配 */
}
```

**相关 API：**

- `xgePassBegin`
- `xgeRenderTargetTexture`

---

## 线程约束

RenderTarget 创建、resize、readback、buffer 上传和 pass begin/end 都可能涉及 GPU 状态，必须在 render owner 线程执行或通过命令队列提交。

Readback 会同步 GPU/CPU，不应在性能关键线程频繁调用。

## 后端差异

FBO、颜色附件、readback 和 MSAA 等能力在 OpenGL/GLES/WebGL 后端存在差异。Web 和移动端 readback 成本更高。

窗口 RenderTarget 与离屏 RenderTarget 的 present 行为不同，离屏目标不会自动显示到窗口。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 离屏纹理为空 | pass 没有 begin/end 或绘制目标错误 | 确认 `xgePassBegin` 成功并成对调用 `xgePassEnd`。 |
| resize 后崩溃或显示旧内容 | 旧 framebuffer/texture 未重建 | 使用 `xgeRenderTargetResize`，不要直接改字段。 |
| readback 卡顿 | GPU 同步 | 避免每帧读回，必要时降低频率或只在工具路径使用。 |

## 相关示例

- `examples/render_target`
- `examples/offscreen`
- `examples\render_target\build.bat`
