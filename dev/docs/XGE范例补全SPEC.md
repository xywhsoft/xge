# XGE 范例补全 SPEC

本文档用于跟踪 XGE 范例程序补全工程。范例不仅是学习 XGE 的入口，也必须作为可运行的测试文档存在，用来持续验证公开 API 的真实使用路径。

## 1. 背景

当前 `examples/` 已覆盖窗口、基础绘制、纹理、文本、输入、场景、异步、音频、渲染管线、平台 smoke、小程序 scaffold 和 XUI 验证程序，但覆盖粒度偏粗。

以 `xge.h` 中 `XGE_API` 为粗略统计口径：

- 公开 API 约 671 个。
- 当前 examples 直接调用约 245 个。
- 当前 examples 未直接调用约 426 个。
- 缺口主要集中在 XUI、音频、字体/资源、底层渲染、平台后端、小程序桥接和离屏/EGL。

旧版 XGE_V1 可作为范例体系参考：

```text
D:\git\XGE_V1\release\examples\freebasic
```

其中 `000-014` 覆盖窗口、形状、文本、图像、截图、视图和坐标；`100-103` 覆盖声音；`200-281` 覆盖场景、鼠标、键盘、暂停和场景栈；`300-380` 覆盖 XUI；`900-902` 覆盖 pack/resource 相关能力。

## 2. 目标

- 建立一套覆盖 XGE 主要公开 API 的范例程序。
- 每个范例都能独立构建、运行、阅读和验证。
- 每个范例都附带对应 case 文档和构建脚本。
- 范例要尽量覆盖真实使用路径，而不是只为调用 API 而调用 API。
- 可自动退出的范例应提供 `--frames`、`--seconds` 或 `--bench` 等参数，便于 CI/冒烟测试。
- 需要人工观察的范例应在窗口内清晰展示状态、操作提示和通过标准。
- 范例补全进度必须可跟踪，可分阶段交付。

## 3. 非目标

- 不要求“一 API 一个范例”。应优先用功能簇覆盖一组 API。
- 不复刻 XGE_V1 的 FreeBasic 绑定和旧网络 API。
- 不把旧版 xPack 工具链原样迁移到当前核心；当前只覆盖 V2 已公开的 resource provider / xpack provider API。
- 不把平台专用能力都纳入 Windows 必过标准；缺设备或缺平台时可以标记为跳过。
- 不为内部 helper 或非公开函数建立范例。

## 4. 范例规范

每个新增范例应包含：

- `examples/<name>/main.c`
- `examples/<name>/build.bat`
- `examples/<name>/build.sh`，如果该范例适合非 Windows 平台。
- `docs/case/<name>.md`
- `docs/case/<name>.en.md`，如果英文文档同步维护。
- 在 `dev/docs/人工冒烟测试流程.md` 中追加验证步骤。
- 如果范例可自动验证，追加到对应聚合检查脚本或新增聚合脚本。

范例窗口应遵守：

- 第一屏展示真实功能，不做说明型 landing 页面。
- 窗口内可以有简短状态行，但不要堆叠教程文本。
- 需要交互时，展示当前输入状态、关键计数或结果值。
- 按 `ESC` 可退出。
- 支持自动退出的范例应在退出时打印 summary。

范例代码应遵守：

- 优先使用当前公开 API，不直接依赖内部源码。
- 保持单文件示例可读性；复杂范例可以拆少量本地 helper。
- 避免把测试断言塞满窗口范例；逻辑断言仍放在 `test/test_main.c` 或专门验证程序中。
- 每个范例应覆盖一组明确 API，并在 case 文档列出“覆盖 API”。

## 5. 覆盖统计规则

覆盖状态按以下层级记录：

- `直接调用`：范例源码直接调用该 API。
- `行为覆盖`：范例通过更高层 API 间接覆盖该行为，并在 case 文档说明。
- `测试覆盖`：自动测试覆盖，但 examples 不直接覆盖。
- `暂不覆盖`：平台、设备或非当前目标原因导致暂不做范例。

建议后续维护一个生成文件：

```text
build/api_example_coverage.csv
```

该文件可由脚本扫描 `xge.h` 和 `examples/` 得到，不纳入源码提交，只作为本地分析产物。

## 6. 优先级定义

- P0：学习路径核心和最大 API 缺口，优先补齐。
- P1：底层渲染、平台、资源和可自动验证能力。
- P2：XUI 细粒度控件与复杂交互，用于完善文档型范例。
- P3：可选高级案例、综合案例或特定平台案例。

## 7. 阶段 A：P0 核心范例

### 7.1 `core_lifecycle`

目标：覆盖 XGE 程序生命周期、窗口与运行模式。

覆盖 API：

- `xgeInit`
- `xgeRun`
- `xgeFrame`
- `xgeRender`
- `xgeQuit`
- `xgeUnit`
- `xgeSetTitle`
- `xgeGetWidth`
- `xgeGetHeight`
- `xgeGetDelta`
- `xgeGetFPS`
- `xgeTimer`
- `xgeTimeNow`
- `xgeSleep`
- `xgeFrameStatsReset`
- `xgeFrameStatsGet`

任务：

- [x] 新增 `examples/core_lifecycle/main.c`。
- [x] 新增 `examples/core_lifecycle/build.bat`。
- [x] 支持 `--frames` 自动退出。
- [x] 退出时打印 frame/fps/delta summary。
- [x] 新增 `docs/case/core-lifecycle.md`。
- [x] 更新人工冒烟测试流程。

### 7.2 `diagnostics_caps_log`

目标：覆盖诊断、日志、平台能力和图形后端查询。

覆盖 API：

- `xgeDebugGetStats`
- `xgeDebugDumpCaps`
- `xgeLogSetLevel`
- `xgeLogGetLevel`
- `xgeLogWrite`
- `xgeLogFlush`
- `xgePlatformBackendDefault`
- `xgePlatformBackendSet`
- `xgePlatformBackendGet`
- `xgeGraphicsBackendDefault`
- `xgeGraphicsBackendSet`
- `xgeGraphicsBackendGet`
- `xgeGpuCapsGet`
- `xgePlatformCapsGet`
- `xgePlatformRuntimeGet`
- `xgeGraphicsShaderHeaderGet`
- `xgeGraphicsLibraryNameGet`
- `xgeGraphicsMappingGet`

任务：

- [x] 新增 `examples/diagnostics_caps_log/main.c`。
- [x] 新增构建脚本。
- [x] 输出 caps/log/backend summary。
- [x] 支持无窗口或短窗口自动退出。
- [x] 新增 case 文档。

### 7.3 `viewport_clip_camera`

目标：迁移 XGE_V1 `013.SetView`、`014.SetCoodr` 思路，覆盖视口、裁剪和坐标转换。

覆盖 API：

- `xgeViewportSet`
- `xgeViewportGet`
- `xgeViewportClear`
- `xgeClipSet`
- `xgeClipGet`
- `xgeClipClear`
- `xgeCameraDefault`
- `xgeCameraPerspective`
- `xgeCameraSet`
- `xgeCameraGet`
- `xgeWorldToScreen`
- `xgeScreenToWorld`

任务：

- [x] 新增 `examples/viewport_clip_camera/main.c`。
- [x] 窗口中展示多个 viewport 与 clip 区域。
- [x] 展示 world/screen 坐标转换数值。
- [x] 支持 resize 后验证坐标仍合理。
- [x] 新增 case 文档。

### 7.4 `image_texture_lab`

目标：覆盖图像加载、内存加载、纹理创建、更新、采样和读回。

覆盖 API：

- `xgeImageLoad`
- `xgeImageLoadEx`
- `xgeImageLoadMemory`
- `xgeImageLoadMemoryEx`
- `xgeImageGetPixels`
- `xgeImagePremultiply`
- `xgeImageFree`
- `xgeTextureCreateRGBA`
- `xgeTextureCreateFromImage`
- `xgeTextureLoad`
- `xgeTextureLoadEx`
- `xgeTextureLoadMemory`
- `xgeTextureLoadMemoryEx`
- `xgeTextureUpdateRGBA`
- `xgeSamplerDefault`
- `xgeTextureSetSampler`
- `xgeTextureGetSampler`
- `xgeTextureFallbackSetRGBA`
- `xgeTextureFallbackGet`
- `xgeTextureFallbackClear`
- `xgeTextureUploadQueue`
- `xgeTextureUploadFlush`
- `xgeTextureReadPixels`
- `xgeTextureAddRef`
- `xgeTextureFree`

任务：

- [x] 新增 `examples/image_texture_lab/main.c`。
- [x] 使用文件纹理、内存纹理和程序生成纹理。
- [x] 展示 sampler 对比和 update 区域。
- [x] 打印 readback 校验 summary。
- [x] 新增 case 文档。

### 7.5 `font_text_lab`

目标：覆盖字体加载、XRF、fallback、glyph 和文本绘制。

覆盖 API：

- `xgeTextUTF8Next`
- `xgeFontLoad`
- `xgeFontLoadMemory`
- `xgeFontLoadXRF`
- `xgeFontLoadXRFMemory`
- `xgeFontBuildXRFMemory`
- `xgeFontSaveXRF`
- `xgeFontLoadCached`
- `xgeFontAddRef`
- `xgeFontFree`
- `xgeFontSetFallback`
- `xgeFontFallbackSet`
- `xgeFontFallbackSetMemory`
- `xgeFontFallbackGet`
- `xgeFontFallbackClear`
- `xgeFontGlyphGet`
- `xgeFontGlyphRasterize`
- `xgeFontGlyphAtlasGet`
- `xgeGlyphBitmapFree`
- `xgeTextMeasure`
- `xgeTextDraw`
- `xgeTextDrawRect`

任务：

- [x] 新增 `examples/font_text_lab/main.c`。
- [x] 展示英文、中文或 fallback 字符。
- [x] 展示 measure、align、clip、glyph metrics。
- [x] 生成并加载一份 XRF 缓存。
- [x] 新增 case 文档。

### 7.6 `resource_provider_lab`

目标：迁移 XGE_V1 `900.xPack5`、`901.ResManage` 中与 V2 对应的资源访问思想。

覆盖 API：

- `xgeResourceProviderAdd`
- `xgeResourceProviderClear`
- `xgeResourceXPackProviderAdd`
- `xgeResourceLoad`
- `xgeResourceLoadMemory`
- `xgeResourceFree`
- `xgeMemoryFree`

任务：

- [x] 新增 `examples/resource_provider_lab/main.c`。
- [x] 实现一个 `mem://` provider。
- [x] 实现一个模拟 xpack provider。
- [x] 展示 texture/font/resource 从 provider 加载。
- [x] 打印 provider 命中与 free 计数。
- [x] 新增 case 文档。

### 7.7 `input_state_lab`

目标：覆盖键盘、鼠标、触摸、剪贴板和手柄状态。

覆盖 API：

- `xgeKeyDown`
- `xgeKeyPressed`
- `xgeKeyReleased`
- `xgeMouseGet`
- `xgeMouseGetDelta`
- `xgeMouseGetWheel`
- `xgeMouseDown`
- `xgeClipboardSetText`
- `xgeClipboardGetText`
- `xgeTouchGetCount`
- `xgeTouchGet`
- `xgeTouchFind`
- `xgeGamepadConnected`
- `xgeGamepadGetState`
- `xgeGamepadButtonDown`
- `xgeGamepadButtonPressed`
- `xgeGamepadButtonReleased`
- `xgeGamepadAxis`
- `xgeGamepadSetConnected`
- `xgeGamepadSetState`

任务：

- [x] 新增 `examples/input_state_lab/main.c`。
- [x] 显示键盘、鼠标、触摸、剪贴板、手柄状态面板。
- [x] 支持模拟 gamepad 状态，避免必须依赖真实手柄。
- [x] 支持复制/粘贴验证。
- [x] 新增 case 文档。

### 7.8 `audio_lab`

目标：扩展现有 `audio`，覆盖 sound/music/stream/group/listener/fade/fallback。

覆盖 API：

- `xgeAudioInit`
- `xgeAudioUnit`
- `xgeAudioIsReady`
- `xgeAudioSetVolume`
- `xgeAudioGetVolume`
- `xgeAudioListenerSet`
- `xgeAudioListenerGet`
- `xgeAudioGroupInit`
- `xgeAudioGroupFree`
- `xgeAudioGroupSetVolume`
- `xgeAudioGroupGetVolume`
- `xgeAudioGroupFade`
- `xgeSoundLoad`
- `xgeSoundLoadGroup`
- `xgeSoundFallbackSet`
- `xgeSoundFallbackGet`
- `xgeSoundFallbackClear`
- `xgeSoundPlay`
- `xgeSoundStop`
- `xgeSoundPause`
- `xgeSoundResume`
- `xgeSoundAddRef`
- `xgeSoundFree`
- `xgeSoundSetLoop`
- `xgeSoundSetVolume`
- `xgeSoundSetPosition`
- `xgeSoundFade`
- `xgeSoundIsPlaying`
- `xgeMusicLoad`
- `xgeMusicLoadGroup`
- `xgeMusicPlay`
- `xgeMusicStop`
- `xgeMusicPause`
- `xgeMusicResume`
- `xgeMusicFree`
- `xgeMusicSetLoop`
- `xgeMusicSetVolume`
- `xgeMusicFade`
- `xgeMusicIsPlaying`
- `xgeStreamOpen`
- `xgeStreamOpenGroup`
- `xgeStreamPlay`
- `xgeStreamStop`
- `xgeStreamPause`
- `xgeStreamResume`
- `xgeStreamClose`
- `xgeStreamSetLoop`
- `xgeStreamSetVolume`
- `xgeStreamSetPosition`
- `xgeStreamFade`
- `xgeStreamIsPlaying`

任务：

- [x] 新增 `examples/audio_lab/main.c`。
- [x] 支持命令行传入 sound/music/stream 文件。
- [x] 支持无音频文件时走 fallback 或标记跳过。
- [x] 展示 group volume、fade、loop、3D position。
- [x] 新增 case 文档。

### 7.9 `scene_lifecycle_lab`

目标：扩展现有 `scene`，覆盖完整场景生命周期和更新策略。

覆盖 API：

- `xgeSceneSet`
- `xgeScenePush`
- `xgeScenePop`
- `xgeSceneReplace`
- `xgeSceneCurrent`
- `xgeSceneCount`
- `xgeSceneDispatchEvent`
- `xgeSceneUpdateStrategySet`
- `xgeSceneUpdateStrategyGet`

任务：

- [x] 新增 `examples/scene_lifecycle_lab/main.c`。
- [x] 展示 enter/leave/pause/resume/event/update/draw/free 计数。
- [x] 支持固定步长和可变步长切换。
- [x] 支持 `--frames` 自动退出。
- [x] 新增 case 文档。

## 8. 阶段 B：P1 渲染、平台与异步范例

### 8.1 `render_target_lab`

任务：

- [x] 扩展或新增范例覆盖 `xgeRenderTargetWindow`。
- [x] 覆盖 `xgeRenderTargetCreate`。
- [x] 覆盖 `xgeRenderTargetResize`。
- [x] 覆盖 `xgeRenderTargetReadPixels`。
- [x] 覆盖 `xgeRenderTargetTexture`。
- [x] 覆盖 `xgeRenderTargetFree`。
- [x] 覆盖 `xgePassInit`、`xgePassBegin`、`xgePassEnd`。
- [x] 新增 readback summary。

### 8.2 `shader_variant_lab`

任务：

- [x] 新增 shader create/free 基础示例。
- [x] 覆盖 `xgeShaderUniform1f/2f/3f/4f`。
- [x] 覆盖 `xgeShaderAddRef`。
- [x] 覆盖 `xgeShaderVariantSetInit`。
- [x] 覆盖 `xgeShaderVariantGet`。
- [x] 覆盖 `xgeShaderVariantSetFree`。
- [x] 展示不同 define key 的视觉差异。

### 8.3 `material_mesh_buffer_lab`

任务：

- [x] 覆盖 `xgeBufferCreate`。
- [x] 覆盖 `xgeBufferUpdate`。
- [x] 覆盖 `xgeBufferUpload`。
- [x] 覆盖 `xgeBufferFree`。
- [x] 覆盖 `xgeMeshCreate`。
- [x] 覆盖 `xgeMeshUpdate`。
- [x] 覆盖 `xgeMeshDraw`。
- [x] 覆盖 `xgeMeshFree`。
- [x] 覆盖 `xgeMaterialInit`。
- [x] 覆盖 `xgeMaterialSetShader`。
- [x] 覆盖 `xgeMaterialSetTexture`。
- [x] 覆盖 `xgeMaterialSetColor`。
- [x] 覆盖 `xgeMaterialSetBlend`。
- [x] 覆盖 `xgeMaterialDraw`。
- [x] 覆盖 `xgeMaterialFree`。

### 8.4 `blend_depth_color_lab`

任务：

- [x] 覆盖 `xgeColorRGBA`。
- [x] 覆盖 `xgeColorUnpack`。
- [x] 覆盖 `xgeBlendSet`。
- [x] 覆盖 `xgeBlendGet`。
- [x] 覆盖 `xgeDepthTestSet`。
- [x] 覆盖 `xgeDepthTestGet`。
- [x] 展示 alpha/additive/none 等混合差异。
- [x] 展示 depth test 对遮挡顺序的影响。

### 8.5 `shape_full_gallery`

任务：

- [x] 扩展所有 shape world-space API。
- [x] 扩展所有 shape pixel-space API。
- [x] 覆盖 point/line/rect/circle/arc/poly/triangle。
- [x] 覆盖 fill/stroke。
- [x] 覆盖 shape auto batch 相关路径。
- [x] 新增 case 文档中的视觉检查表。

### 8.6 `async_assets_lab`

任务：

- [x] 覆盖 `xgeAsyncRequestInit`。
- [x] 覆盖 `xgeAsyncRequestFree`。
- [x] 覆盖 `xgeAsyncRequestCancel`。
- [x] 覆盖 `xgeAsyncThreadingSet`。
- [x] 覆盖 `xgeAsyncThreadingGet`。
- [x] 覆盖 `xgeAsyncPoll`。
- [x] 覆盖 `xgeAsyncImageLoad`。
- [x] 覆盖 `xgeAsyncTextureLoad`。
- [x] 覆盖 `xgeAsyncFontLoad`。
- [x] 覆盖 `xgeAsyncSoundLoad`。
- [x] 展示 threaded/on-main-thread 两种模式。

### 8.7 `offscreen_egl_lab`

任务：

- [ ] 覆盖 `xgeEGLCapsGet`。
- [ ] 覆盖 `xgeEGLInit`。
- [ ] 覆盖 `xgeEGLMakeCurrent`。
- [ ] 覆盖 `xgeEGLUnit`。
- [ ] 覆盖 `xgeOffscreenInit`。
- [ ] 覆盖 `xgeOffscreenRenderTarget`。
- [ ] 覆盖 `xgeOffscreenReadPixels`。
- [ ] 覆盖 `xgeOffscreenUnit`。
- [ ] 对无 EGL 环境输出明确跳过原因。

### 8.8 `render_thread_lab`

任务：

- [ ] 覆盖 `xgeRenderThreadCapsGet`。
- [ ] 覆盖 `xgeRenderThreadSet`。
- [ ] 覆盖 `xgeRenderThreadGet`。
- [ ] 覆盖 `xgeRenderThreadEGLSet`。
- [ ] 覆盖 threaded flush 的可见绘制路径。
- [ ] 对不支持平台输出明确跳过原因。

### 8.9 `platform_runtime_lab`

任务：

- [x] 扩展现有 `platform_smoke` 或新增独立范例。
- [x] 展示 runtime counters。
- [x] 展示 resize/high-dpi/mouse/key/text/gamepad 计数。
- [x] 支持 `--frames` 和 `--seconds`。
- [x] 可作为 CI 平台 smoke 的标准入口。

### 8.10 `miniprogram_bridge_lab`

任务：

- [ ] 覆盖 `xgeMiniProgramInit`。
- [ ] 覆盖 `xgeMiniProgramInitSimple`。
- [ ] 覆盖 `xgeMiniProgramUnit`。
- [ ] 覆盖 `xgeMiniProgramSetBridge`。
- [ ] 覆盖 `xgeMiniProgramFrame`。
- [ ] 覆盖 `xgeMiniProgramResize`。
- [ ] 覆盖 `xgeMiniProgramTouch`。
- [ ] 覆盖 `xgeMiniProgramTouchOne`。
- [ ] 覆盖 `xgeMiniProgramText`。
- [ ] 覆盖 `xgeMiniProgramRequestFrame`。
- [ ] 覆盖 `xgeMiniProgramAudioCommand`。
- [ ] 与 `platform/miniprogram` 文档互相引用。

## 9. 阶段 C：P2 XUI 细粒度范例

### 9.1 `xui_widget_tree_style_lab`

任务：

- [ ] 覆盖 widget create/free/add/remove。
- [ ] 覆盖 id/name/find。
- [ ] 覆盖 rect/local rect/content rect。
- [ ] 覆盖 visible/enabled/focusable/clip。
- [ ] 覆盖 background/style/margin/padding/anchor。
- [ ] 覆盖 mark layout/paint/refresh。

### 9.2 `xui_layout_gallery`

任务：

- [ ] 覆盖 absolute。
- [ ] 覆盖 row。
- [ ] 覆盖 column。
- [ ] 覆盖 stack。
- [ ] 覆盖 grid-lite。
- [ ] 覆盖 content/grow/percent/px。
- [ ] 覆盖 gap/justify/align。
- [ ] 覆盖 dirty layout 可视化或 summary。

### 9.3 `xui_focus_capture_event_lab`

任务：

- [ ] 覆盖 dispatch event。
- [ ] 覆盖 event queue push/process。
- [ ] 覆盖 focus in/out。
- [ ] 覆盖 pointer enter/leave。
- [ ] 覆盖 capture/capture lost。
- [ ] 覆盖 Tab/Shift+Tab/Enter/Space/ESC。
- [ ] 覆盖 context press/right click。

### 9.4 `xui_paint_host_lab`

任务：

- [ ] 覆盖 XUI host 设置。
- [ ] 覆盖 paint command 输出。
- [ ] 覆盖 clip set/clear command。
- [ ] 覆盖 custom paint callback。
- [ ] 覆盖 manual refresh request。

### 9.5 `xui_theme_lab`

任务：

- [ ] 覆盖 default theme。
- [ ] 覆盖 theme set/get。
- [ ] 覆盖 DIP scale。
- [ ] 覆盖控件级颜色覆盖。
- [ ] 展示 light/dark 或两套主题切换。

### 9.6 `xui_basic_controls_lab`

- [x] completed: Label / Image / Button / IconButton / Panel / Separator / init-unit-setter-getter-state
任务：

- [ ] 覆盖 Label。
- [ ] 覆盖 Image。
- [ ] 覆盖 Button。
- [ ] 覆盖 IconButton。
- [ ] 覆盖 Panel。
- [ ] 覆盖 Separator。
- [ ] 每个控件展示 init/unit/setter/getter/state。

### 9.7 `xui_choice_controls_lab`

- [x] completed: Toggle / CheckBox / RadioGroup / Radio / Switch / keyboard-mouse-callback-state
任务：

- [ ] 覆盖 Toggle。
- [ ] 覆盖 CheckBox。
- [ ] 覆盖 RadioGroup。
- [ ] 覆盖 Radio。
- [ ] 覆盖 Switch。
- [ ] 展示 keyboard/mouse 状态变化和回调计数。

### 9.8 `xui_value_controls_lab`

任务：

- [x] 覆盖 Slider。
- [x] 覆盖 Progress。
- [x] 覆盖 Splitter。
- [x] 覆盖 ScrollBar。
- [x] 覆盖 range/page/value/orientation/colors/state。

### 9.9 `xui_list_scroll_lab`

任务：

- [x] 覆盖 ScrollView。
- [x] 覆盖 ListView。
- [x] 覆盖 disabled items。
- [x] 覆盖 hover/selected。
- [x] 覆盖 keyboard up/down。
- [x] 覆盖 PageUp/PageDown。
- [x] 覆盖 scroll get/set。

### 9.10 `xui_overlay_menu_lab`

任务：

- [x] 覆盖 Popup。
- [x] 覆盖 Tooltip。
- [x] 覆盖 ComboBox。
- [x] 覆盖 Menu。
- [x] 覆盖 Dialog。
- [x] 覆盖 modal/outside close/ESC close/disabled item。

### 9.11 `xui_text_edit_lab`

任务：

- [x] 覆盖 Input。
- [x] 覆盖 TextEdit。
- [x] 覆盖 cursor/selection。
- [x] 覆盖 readonly/disabled/password。
- [x] 覆盖 clipboard/context menu。
- [x] 覆盖 undo/redo。
- [x] 覆盖 wrap。
- [x] 覆盖 IME composition/candidate rect。

### 9.12 `xui_hyperlink_lab`

任务：

- [x] 用现有 `xgeXuiButton` primitive 组合出 hyperlink 语义。
- [x] 覆盖 visited/custom/transient/disabled 四种状态。
- [x] 覆盖 mouse 激活路径。
- [x] 覆盖 keyboard 激活路径。
- [x] 作为 V1 `323.XUI HyperLink Element` 的迁移对照范例。

### 9.13 `xui_tabs_dirty_rect_lab`

任务：

- [x] 覆盖 `xgeXuiTabs*` 的 init/select/state/event/paint 主路径。
- [x] 覆盖 `xgeInvalidateRect`、`xgeDirtyRectCount/Get/Clear`。
- [x] 覆盖 `xgeXuiSizeContent`、`xgeXuiWidgetSetMinSize`、`xgeXuiWidgetSetMaxSize`、`xgeXuiWidgetSetMeasure`。
- [x] 覆盖 `xgeXuiWidgetGetZ` 的重叠命中场景。
- [x] 做成可自动退出并输出 summary 的 XUI 自检范例。

### 9.14 `xui_text_buffer_lab`

任务：

- [x] 覆盖 `xgeXuiTextInit/Unit/Set/Insert/InsertCodepoint/DeleteBack/DeleteForward`。
- [x] 覆盖 `xgeXuiTextSetCursor/GetCursor` 与 `xgeXuiTextSetSelection/GetSelection`。
- [x] 覆盖 `xgeXuiTextSetComposition/ClearComposition/GetComposition` 与 `xgeXuiTextInputEvent`。
- [x] 覆盖 `xgeXuiInputSetFont` 与 `xgeXuiTextEditSetFont`。
- [x] 用真实事件覆盖 `xgeXuiTextEditEvent` / `xgeXuiTextEditEventProc` 的文本输入、退格、回车和 IME 路径。
- [x] 做成可自动退出并输出 summary 的 XUI 自检范例。

### 9.15 `xui_visual_proc_lab`

任务：

- [x] 覆盖 `xgeXuiLabelMeasure`、`xgeXuiLabelMeasureProc`、`xgeXuiLabelPaintProc`。
- [x] 覆盖 `xgeXuiImageMeasureProc`、`xgeXuiImagePaintProc`。
- [x] 覆盖 `xgeXuiSeparatorPaintProc` 与 `xgeXuiPanelPaintProc`。
- [x] 覆盖 `xgeXuiButtonEventProc`、`xgeXuiButtonPaintProc`。
- [x] 覆盖 `xgeXuiIconButtonEventProc`、`xgeXuiIconButtonPaintProc`。
- [x] 做成可自动退出并输出 summary 的 XUI 自检范例。

### 9.16 `xui_control_proc_lab`

任务：

- [x] 覆盖 `xgeXuiToggleEventProc`、`xgeXuiTogglePaintProc`。
- [x] 覆盖 `xgeXuiCheckBoxEventProc`、`xgeXuiCheckBoxPaintProc`。
- [x] 覆盖 `xgeXuiRadioEventProc`、`xgeXuiRadioPaintProc`、`xgeXuiRadioGetState`。
- [x] 覆盖 `xgeXuiSliderEventProc`、`xgeXuiSliderPaintProc`。
- [x] 覆盖 `xgeXuiScrollBarEventProc`、`xgeXuiScrollBarPaintProc`。
- [x] 覆盖 `xgeXuiSplitterEventProc`、`xgeXuiSplitterPaintProc`。
- [x] 覆盖 `xgeXuiProgressPaintProc`。
- [x] 覆盖 `xgeXuiSwitchEventProc`、`xgeXuiSwitchPaintProc`。
- [x] 做成可自动退出并输出 summary 的 XUI 自检范例。

### 9.17 `xui_overlay_proc_lab`

任务：

- [x] 覆盖 `xgeXuiPopupEvent`、`xgeXuiPopupEventProc`、`xgeXuiPopupPaintProc`。
- [x] 覆盖 `xgeXuiComboBoxEvent`、`xgeXuiComboBoxEventProc`、`xgeXuiComboBoxPaintProc`。
- [x] 覆盖 `xgeXuiComboBoxGetSelected`、`xgeXuiComboBoxIsOpen`、`xgeXuiComboBoxGetState`。
- [x] 覆盖 `xgeXuiListViewEventProc`、`xgeXuiListViewPaintProc`、`xgeXuiListViewGetSelected`、`xgeXuiListViewGetScroll`。
- [x] 覆盖 `xgeXuiDialogEvent`、`xgeXuiDialogEventProc`、`xgeXuiDialogPaintProc`、`xgeXuiDialogIsOpen`。
- [x] 做成可自动退出并输出 summary 的 XUI 自检范例。

### 9.18 `xui_list_view_proc_lab`

任务：

- [x] 覆盖 `xgeXuiListViewInit`、`xgeXuiListViewUnit`。
- [x] 覆盖 `xgeXuiListViewSetItems`、`xgeXuiListViewSetEnabledItems`、`xgeXuiListViewSetFont`、`xgeXuiListViewSetItemHeight`。
- [x] 覆盖 `xgeXuiListViewSetSelected`、`xgeXuiListViewGetSelected`、`xgeXuiListViewSetScroll`、`xgeXuiListViewGetScroll`。
- [x] 覆盖 `xgeXuiListViewSetSelect`、`xgeXuiListViewSetColors`、`xgeXuiListViewSetDisabledTextColor`。
- [x] 覆盖 `xgeXuiListViewEvent`、`xgeXuiListViewEventProc`、`xgeXuiListViewPaintProc`。
- [x] 做成可自动退出并输出 summary 的 XUI 自检范例。

### 9.19 `xui_scroll_view_proc_lab`

任务：

- [x] 覆盖 `xgeXuiScrollViewInit`、`xgeXuiScrollViewUnit`。
- [x] 覆盖 `xgeXuiScrollViewSetContentSize`、`xgeXuiScrollViewSetOffset`、`xgeXuiScrollViewGetOffset`、`xgeXuiScrollViewSetColors`。
- [x] 覆盖 `xgeXuiScrollViewEvent`、`xgeXuiScrollViewEventProc`、`xgeXuiScrollViewPaintProc`。
- [x] 覆盖 wheel、内容拖拽、竖向 page jump、横向 thumb drag 的真实交互路径。
- [x] 做成可自动退出并输出 summary 的 XUI 自检范例。

## 10. 阶段 D：P3 综合与迁移案例

### 9.20 `xui_input_proc_lab`

浠诲姟锛?
- [x] 瑕嗙洊 `xgeXuiInputEvent`銆乣xgeXuiInputEventProc`銆乣xgeXuiInputUpdateProc`銆乣xgeXuiInputPaintProc`銆?
- [x] 瑕嗙洊 `xgeXuiInputSetSelection`銆乣xgeXuiInputGetSelection`銆乣xgeXuiInputGetCandidateRect`銆?
- [x] 瑕嗙洊 `xgeXuiTextEditEvent`銆乣xgeXuiTextEditEventProc`銆乣xgeXuiTextEditUpdateProc`銆乣xgeXuiTextEditPaintProc`銆乣xgeXuiTextEditGetCandidateRect`銆?
- [x] 瑕嗙洊 default context menu銆乥link update銆乧andidate rect 绛変綆灞傝緭鍏ヨ矾寰勩€?
- [x] 鍋氭垚鍙嚜鍔ㄩ€€鍑哄苟杈撳嚭 summary 鐨?XUI 鑷鑼冧緥銆?

### 9.21 `xui_button_state_lab`

浠诲姟锛?
- [x] 瑕嗙洊 `xgeXuiButtonInit`銆乣xgeXuiButtonSetClick`銆乣xgeXuiButtonSetText`銆乣xgeXuiButtonSetTextColor`銆乣xgeXuiButtonSetColors`銆乣xgeXuiButtonGetState`銆乣xgeXuiButtonEvent`銆?
- [x] 瑕嗙洊 `xgeXuiIconButtonInit`銆乣xgeXuiIconButtonSetClick`銆乣xgeXuiIconButtonSetTexture`銆乣xgeXuiIconButtonSetSource`銆乣xgeXuiIconButtonSetIconColor`銆乣xgeXuiIconButtonSetMode`銆乣xgeXuiIconButtonSetColors`銆乣xgeXuiIconButtonGetState`銆乣xgeXuiIconButtonEvent`銆?
- [x] 瑕嗙洊 hover銆乤ctive銆乫ocus銆乨isabled銆乧apture銆乲eyboard click 鐨勭姸鎬佹祦銆?
- [x] 瑕嗙洊 `xgeXuiDispatchEvent` 璺緞锛屽仛鍒版枃鏈?button 鍜?icon button 鍚勮嚜鏈夎嚜妫€鐐广€?
- [x] 鍋氭垚鍙嚜鍔ㄩ€€鍑哄苟杈撳嚭 summary 鐨?XUI 鑷鑼冧緥銆?

### 9.22 `xui_text_select_lab`

浠诲姟锛?
- [x] 瑕嗙洊 `Input` 鐨?Ctrl+Left / Ctrl+Right / Home / End cursor navigation 銆?
- [x] 瑕嗙洊 `Input` 鐨?Ctrl+A selection 鍜岄紶鏍囨嫋閫夈€?
- [x] 瑕嗙洊 `TextEdit` 鐨?Home / Up / Ctrl+End / Ctrl+Home / PageDown navigation 銆?
- [x] 瑕嗙洊 `TextEdit` 鐨?Shift selection銆乧trl+A 鍜岃法琛屾嫋閫夈€?
- [x] 鍋氭垚鍙嚜鍔ㄩ€€鍑哄苟杈撳嚭 summary 鐨?XUI 鑷鑼冧緥銆?

### 9.23 `xui_text_delete_lab`

浠诲姟锛?
- [x] 瑕嗙洊 `xgeXuiTextInsert`銆乣xgeXuiTextDeleteBack`銆乣xgeXuiTextDeleteForward` 鐨勫簳灞傛枃鏈涓恒€?
- [x] 瑕嗙洊 `Input` 鐨?selection replace銆乣Delete`銆乣Backspace` 鐨勭紪杈戣矾寰勩€?
- [x] 瑕嗙洊 `TextEdit` 鐨?selection replace銆佽法琛屽悎骞剁殑 `Backspace`銆乣Delete` 鐨勭紪杈戣矾寰勩€?
- [x] 瑕嗙洊 `TextEditUndo/Redo` 鍦ㄥ垹闄ょ浉鍏虫搷浣滃悗鐨勬仮澶嶈矾寰勩€?
- [x] 鍋氭垚鍙嚜鍔ㄩ€€鍑哄苟杈撳嚭 summary 鐨?XUI 鑷鑼冧緥銆?

### 9.24 `xui_text_wrap_scroll_lab`

浠诲姟锛?
- [x] 瑕嗙洊 `TextEdit` 鐨?SetWordWrap` 鍜?visual line cache 鐨勮嚜妫€銆?
- [x] 瑕嗙洊 `TextEdit` 鐨?SetScroll` 鍜岄暱鏂囨湰鑷姩 scroll 鐨勮嚜妫€銆?
- [x] 瑕嗙洊 `PageUp/PageDown` 鍜?Home/End/Ctrl+Home/Ctrl+End` 鐨勫厜鏍囧鑸矾寰勩€?
- [x] 鍋氭垚鍙嚜鍔ㄩ€€鍑哄苟杈撳嚭 summary 鐨?TextEdit 鑷鑼冧緥銆?

### 9.25 `xui_text_undo_stack_lab`

浠诲姟锛?
- [x] 瑕嗙洊 `xgeXuiTextEditUndo` 鍜?`xgeXuiTextEditRedo` 鐨勭洿鎺ヨ皟鐢ㄨ矾寰勩€?
- [x] 瑕嗙洊 empty stack 鍜?redo clear 鐨勮竟鐣岃涓恒€?
- [x] 瑕嗙洊 selection replace 涔嬪悗鐨?selection restore 鐘舵€佸洖婧€?
- [x] 瑕嗙洊 `Ctrl+Z` 鍜?`Ctrl+Y` 鐨勪簨浠惰矾寰勶紝骞朵笌 direct undo/redo 瀵规瘮銆?
- [x] 鍋氭垚鍙嚜鍔ㄩ€€鍑哄苟杈撳嚭 summary 鐨?TextEdit history stack 鑷鑼冧緥銆?

### 9.27 `xui_menu_owner_lab`

浠诲姟锛?
- [x] 瑕嗙洊 `xgeXuiMenuInit` / `xgeXuiMenuUnit` / `xgeXuiMenuIsOpen` 鐨?owner 绑定鍜屽脊鍑虹姸鎬併€?
- [x] 瑕嗙洊 `xgeXuiMenuSetItems` / `SetEnabledItems` / `SetFont` / `SetSelect` / `SetSize` / `SetColors`銆?
- [x] 瑕嗙洊 `xgeXuiMenuOpen` 鐨?window clamp 鍜?focus 切鎹㈣矾寰勩€?
- [x] 瑕嗙洊 disabled item 涓?enabled item 鐨勭偣鍑诲樊寮傘€?
- [x] 瑕嗙洊 outside close 鍜?ESC close 鐨勫叧闂矾寰勩€?
- [x] 鍋氭垚鍙嚜鍔ㄩ€€鍑哄苟杈撳嚭 summary 鐨?Menu 鑷鑼冧緥銆?

### 9.28 `xui_popup_owner_lab`

浠诲姟锛?
- [x] 瑕嗙洊 `xgeXuiPopupInit` / `xgeXuiPopupUnit` / `xgeXuiPopupIsOpen` 鐨勫熀纭€鐘舵€佽矾寰勩€?
- [x] 瑕嗙洊 `xgeXuiPopupSetOwner` / `SetClose` / `SetOpen` / `SetAutoClose` / `SetBackground`銆?
- [x] 瑕嗙洊 owner click 涓?outside click 鐨勮涓哄樊寮傘€?
- [x] 瑕嗙洊 `ESC` close 鍜?close callback 璺緞銆?
- [x] 瑕嗙洊 `xgeXuiPopupEvent` / `xgeXuiPopupEventProc` / `xgeXuiPopupPaintProc` 鐨勭洿鎺ュ彲瑙佽矾寰勩€?
- [x] 鍋氭垚鍙嚜鍔ㄩ€€鍑哄苟杈撳嚭 summary 鐨?Popup 鑷鑼冧緥銆?

### 9.29 `xui_dialog_modal_lab`

- [x] completed: covered `xgeXuiDialogInit` / `Unit` / `SetTitle` / `SetClose` / `SetOpen` / `IsOpen`
- [x] completed: covered `SetModal` / `SetCloseOnEscape` / `SetShowClose` / `SetColors`
- [x] completed: covered modal outside block, close button callback, `ESC` policy toggle
- [x] completed: covered direct `xgeXuiDialogEvent` / `xgeXuiDialogEventProc` / `xgeXuiDialogPaintProc`
- [x] completed: added example, build scripts, case doc, and auto-exit final summary

### 9.30 `xui_text_history_limit_lab`

- [x] completed: covered public `xge_xui_text_edit_t.iUndoLimit` configuration path
- [x] completed: verified oldest undo state drop when the limit is exceeded
- [x] completed: verified redo stack follows the same effective bound
- [x] completed: verified `xgeXuiTextEditSetText` clears history stacks
- [x] completed: verified `Ctrl+Z` / `Ctrl+Y` path still respects the configured limit

### 9.31 `xui_text_menu_detail_lab`

- [x] completed: covered Input default menu enabled-state rules
- [x] completed: covered TextEdit default menu enabled-state rules
- [x] completed: covered `Select All` / `Cut` / `Copy` / `Paste` / `Delete` action path through default menu callback
- [x] completed: covered `readonly` guard and Input `password` no-menu guard
- [x] completed: added example, build scripts, case doc, and auto-exit final summary

### 9.32 `xui_combo_detail_lab`

- [x] completed: covered ComboBox init, selected getter/setter, dropdown height, colors, and callback path
- [x] completed: covered open/close state via mouse and keyboard
- [x] completed: covered disabled row behavior through underlying list enabled-items path
- [x] completed: covered disabled widget state through `xgeXuiComboBoxGetState`
- [x] completed: added example, build scripts, case doc, and auto-exit final summary

### 9.33 `xui_list_view_detail_lab`

- [x] completed: covered ListView enabled-items update clearing an invalid selected row
- [x] completed: covered page jump and thumb drag through direct scrollbar interaction
- [x] completed: covered keyboard Home/PageDown/End/Up traversal while skipping disabled rows
- [x] completed: covered item height, scroll clamp, disabled text color, and callback wiring
- [x] completed: added example, build scripts, case doc, and auto-exit final summary

### 9.34 `xui_popup_policy_lab`

- [x] completed: covered popup default close policy flags, focusable/visible state, and background setter
- [x] completed: covered no-auto-close mode for both outside click and escape key
- [x] completed: covered owner passthrough while popup stays open
- [x] completed: covered outside-only and escape-only policy combinations plus close callback count
- [x] completed: added example, build scripts, case doc, and auto-exit final summary

### 9.35 `xui_menu_policy_lab`

- [x] completed: covered MenuOpen resetting stale selected/scroll state
- [x] completed: covered repeated reopen with updated width/max-height
- [x] completed: covered popup clamp near window edges plus visible reopened menu state
- [x] completed: covered MenuClose focus return and idempotent close path
- [x] completed: covered enabled-items update clearing an invalid current selection
- [x] completed: added example, build scripts, case doc, and auto-exit final summary

### 9.36 `xui_combo_policy_lab`

- [x] completed: covered popup/list selected sync on open
- [x] completed: covered live drop-down height update while popup stays open
- [x] completed: covered `SetItems` shrinking count resetting invalid selected state
- [x] completed: covered wide-layout reopen with popup width following widget width
- [x] completed: covered outside close returning focus to combo widget
- [x] completed: added example, build scripts, case doc, and auto-exit final summary


- [x] completed: covered `SetOpen` gating by enabled flag and non-empty text
- [x] completed: covered live text relayout and offset relayout while tooltip stays open
- [x] completed: covered owner event open/close rules for pointer enter, touch move, mouse down, and pointer leave
- [x] completed: covered legacy capture handler chaining through tooltip owner proc
- [x] completed: added example, build scripts, case doc, and auto-exit final summary

### 9.38 `xui_dialog_policy_lab`

- [x] completed: covered modal vs non-modal event consumption through direct dialog event path
- [x] completed: covered `closeOnEscape` policy toggling and callback count
- [x] completed: covered `showClose` gating for close button hit path
- [x] completed: covered reopen with updated title and persistent dialog visuals
- [x] completed: added example, build scripts, case doc, and auto-exit final summary

### 10.1 `game_login_lab`

参考 XGE_V1 `380.GameLogin`，作为 XUI + input + resource + scene 的综合范例。

任务：

- [x] 登录面板布局。
- [x] Input password。
- [x] Button/CheckBox。
- [x] Dialog/Menu feedback。
- [x] 场景切换。

### 10.2 `v1_gallery_migration`

目标：建立一个 V1 到 V2 范例对照索引，不一定新增单独代码。

任务：

- [x] 建立 V1 `000-014` 到 V2 范例映射。
- [x] 建立 V1 `100-103` 到 V2 音频范例映射。
- [x] 建立 V1 `200-281` 到 V2 场景/输入范例映射。
- [x] 建立 V1 `300-380` 到 V2 XUI 范例映射。
- [x] 建立 V1 `900-902` 到 V2 资源范例映射。

## 11. 构建脚本任务

- [ ] 为每个新增范例增加 Windows 构建脚本。
- [ ] 为适合跨平台的范例增加 `.sh` 构建脚本。
- [x] 新增或扩展 `build_examples_all.bat`。
- [ ] 新增或扩展 `build_examples_all_parallel.ps1`。
- [x] 新增 `run_examples_smoke.bat`，只运行支持自动退出的范例。
- [ ] 所有新增脚本写入 `dev/docs/构建脚本说明.md`。

## 12. 文档任务

- [ ] 每个范例新增 `docs/case/*.md`。
- [ ] 每个范例 case 文档包含“覆盖 API”。
- [ ] 每个范例 case 文档包含“运行方式”。
- [ ] 每个范例 case 文档包含“通过标准”。
- [ ] 每个范例 case 文档包含“可跳过条件”。
- [ ] 更新 `docs/case/README.md`。
- [ ] 更新 `docs/EXAMPLES.md`。
- [ ] 更新 `dev/docs/人工冒烟测试流程.md`。

已完成批次：

- [x] `core_lifecycle` 已补齐源码、构建脚本、case 文档、索引和人工冒烟流程。
- [x] `diagnostics_caps_log` 已补齐源码、构建脚本、case 文档、索引和自动 smoke。
- [x] `viewport_clip_camera` 已补齐源码、构建脚本、case 文档、索引和自动 smoke。
- [x] `image_texture_lab` 已补齐源码、构建脚本、case 文档、索引和自动 smoke。
- [x] `font_text_lab` 已补齐源码、构建脚本、case 文档、索引和自动 smoke。
- [x] `resource_provider_lab` 已补齐源码、构建脚本、case 文档、索引和自动 smoke。
- [x] `input_state_lab` 已补齐源码、构建脚本、case 文档、索引和自动 smoke。
- [x] `audio_lab` 已补齐源码、构建脚本、case 文档、索引和自动 smoke。
- [x] `scene_lifecycle_lab` 已补齐源码、构建脚本、case 文档、索引和自动 smoke。
- [x] `render_target_lab` 已补齐源码、构建脚本、case 文档、索引和自动 smoke。
- [x] `shader_variant_lab` 已补齐源码、构建脚本、case 文档、索引和自动 smoke。
- [x] `material_mesh_buffer_lab` 已补齐源码、构建脚本、case 文档、索引和自动 smoke。
- [x] `blend_depth_color_lab` 已补齐源码、构建脚本、case 文档、索引和自动 smoke。
- [x] `shape_full_gallery` 已补齐源码、构建脚本、case 文档、索引和自动 smoke。
- [x] `async_assets_lab` 已补齐源码、构建脚本、case 文档、索引和自动 smoke。
- [x] `platform_runtime_lab` 已补齐源码、构建脚本、case 文档、索引和自动 smoke。

- [x] xui_basic_controls_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_choice_controls_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_value_controls_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_list_scroll_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_overlay_menu_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_hyperlink_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_tabs_dirty_rect_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_text_buffer_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_visual_proc_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_control_proc_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_overlay_proc_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_list_view_proc_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_scroll_view_proc_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_input_proc_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_input_policy_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_input_ime_policy_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_input_nav_policy_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_input_context_policy_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_input_blink_focus_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_input_pointer_select_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_input_replace_policy_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_input_delete_policy_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_input_clipboard_policy_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_input_password_policy_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_button_state_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_text_select_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_text_delete_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_text_wrap_scroll_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_menu_owner_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_popup_owner_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_dialog_modal_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_text_undo_stack_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_text_history_limit_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_text_menu_detail_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_combo_detail_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_list_view_detail_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_menu_policy_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_combo_policy_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_dialog_policy_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_popup_policy_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] xui_text_edit_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] game_login_lab completed with example, build scripts, case doc, and smoke coverage.
- [x] v1_gallery_migration completed with mapping doc and index updates.

## 13. 验证任务

- [ ] 所有新增范例构建通过。
- [ ] 自动退出范例可在 `run_examples_smoke.bat` 中通过。
- [ ] 窗口范例人工观察无黑屏、崩溃、明显闪烁。
- [ ] 资源/音频/平台缺依赖时输出明确跳过原因。
- [ ] `build_test.bat` 通过。
- [ ] `check_docs.bat` 通过。
- [ ] `check_script_docs.bat` 通过。
- [ ] `git diff --check` 通过。

## 14. 任务跟踪规则

本节用于跟踪范例补全工程进度。每完成一个范例、脚本或文档，都应同步更新本 SPEC 对应勾选项。

勾选规则：

- `[ ]` 未开始或尚未完成。
- `[x]` 已完成，并且至少通过构建验证。
- 如果任务包含人工验证，必须在人工验证通过后才能勾选。
- 如果范例因为平台或设备原因无法在当前机器验证，应保留未勾选，并在 case 文档中说明跳过条件。
- 如果某个范例拆分出新子任务，应追加到对应阶段，不只在聊天中记录。

## 15. 总体进度

- [ ] 完成 P0 核心范例。
- [ ] 完成 P1 渲染、平台与异步范例。
- [ ] 完成 P2 XUI 细粒度范例。
- [ ] 完成 P3 综合与迁移案例。
- [ ] 建立 API 覆盖统计脚本或本地统计流程。
- [ ] 建立范例聚合构建脚本。
- [ ] 建立范例自动 smoke 脚本。
- [ ] 完成范例文档索引更新。

### 7.10 auto_pause_lab

- [x] completed: scene pause/resume, freeze snapshot, pause overlay, auto resume
- [x] auto_pause_lab completed with example, build scripts, case doc, and smoke coverage.

### 10.3 `xbsmm_migration`

- [x] completed: added `docs/case/xbsmm-migration.md` for V1 `902.xBsmm`
- [x] completed: mapped V1 fixed-size struct storage guidance to V2 caller-owned arrays/pools
- [x] completed: linked migration note to `resource_provider_lab` and resource docs
- [x] completed: updated `docs/case/README.md` and `docs/case/v1-gallery-migration.md`
