# XGE 性能建议

[返回文档中心](README.md)

## 总原则

XGE 的性能目标是让常见 2D 游戏和工具应用在多平台上保持稳定，而不是为某一个后端写死极限优化。优先减少状态切换、资源抖动、CPU/GPU 同步和无意义刷新。

## 绘制提交

优先使用批处理：

- 同纹理大量 sprite 使用 `xgeSpriteBatch*`。
- 同色大量 shape 使用 `xgeShapeBatch*`。
- UI 列表优先轻量行绘制，不要给每一行创建复杂控件树。

减少状态切换：

- 按 texture、blend、shader/material、render target 排序。
- 不要在普通 sprite 之间频繁切换自定义 shader。
- 2.5D pass 集中绘制，结束后恢复 2D 状态。

## 纹理和资源

加载阶段完成图片解码、premultiplied alpha 和格式转换。运行时避免重复加载同一资源。

XGE 第一版不内建全局资源缓存，项目可以在上层建立资源表：

```text
path -> texture/font/sound handle
```

异步资源只在后台做 IO 和解码，GPU 上传必须排队到拥有 context 的线程。

## RenderTarget 和读回

RenderTarget 适合缓存复杂 UI、小地图、动态头像和后处理。固定尺寸缓存不要跟随窗口 resize 重建。

`xgeRenderTargetReadPixels`、`xgeTextureReadPixels` 和 `xgeOffscreenReadPixels` 会导致 GPU/CPU 同步，只适合截图、测试、工具导出，不应每帧调用。

## 文本和字体

TTF 首次栅格化会产生开销。中文 UI 和常用文本建议：

- 使用 XRF 缓存常用字。
- 启动或切场景时预热常用 glyph。
- 长列表文本做可见区域裁剪。
- 不在每帧重复测量不变字符串。

## APP 模式

工具和 GUI APP 不应默认 60 FPS 连续刷新。使用：

- `xgeInvalidateRect`
- `xgeXuiRefreshRequest`
- `xgeXuiRefreshNeeded`
- 手动 `xgeBegin` / `xgeEnd` / `xgeFlush`

只在状态变化、输入、动画或异步资源完成时刷新。

## 音频

短音效使用 sound，背景音乐使用 music，长文件使用 stream。频繁播放的音效应常驻，不要每次播放前重新加载。

音量分组适合统一控制 UI、SFX、BGM，避免遍历所有音频对象。

## 调试统计

开发期用 `xgeDebugGetStats` 和 `xgeDebugDumpCaps` 检查：

- draw call
- batch count
- texture switch
- upload count
- framebuffer size
- GPU backend

发布版不依赖 debug 输出。

## 常见反模式

每帧加载纹理、字体或音频。

每帧重建 XUI 控件树。

每帧读回 framebuffer。

把普通 sprite 全部改成 2.5D quad。

在渲染线程之外直接调用底层 GL。
