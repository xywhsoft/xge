# XGE V2 规格与进度跟踪

本文件用于跟踪 XGE V2 的实现进度。后续开发时，请随着设计、实现、测试和文档完善同步更新勾选状态。

状态说明：

- `[ ]` 未开始
- `[~]` 进行中
- `[x]` 已完成

## 0. 项目搭建

- [ ] 确认 XGE V2 的仓库目录结构。
- [ ] 确认单头文件源码的最终路径。
- [ ] 在根目录添加 `xge.h`。
- [ ] 在根目录添加 `xge.c`。
- [ ] 创建 `src/` 引擎源码目录。
- [ ] 创建 `lib/` 第三方库目录。
- [ ] 创建 `test/` 测试目录。
- [ ] 创建 `singlehead/` 单头文件目录。
- [ ] 创建 `examples/` 示例目录。
- [ ] 添加 `xge.h` 开发头文件骨架。
- [ ] 添加 `XGE_IMPL` 实现区规划。
- [ ] 添加第三方库 `lib/` 目录结构。
- [ ] 添加 xrt 依赖到 `lib/`。
- [ ] 添加 sokol 依赖到 `lib/`。
- [ ] 添加 miniaudio 依赖到 `lib/`。
- [ ] 添加 stb_truetype 依赖到 `lib/`。
- [ ] 添加 `build_exe.bat`。
- [ ] 添加 `build_exe.sh`。
- [ ] 添加 `build_dll.bat`。
- [ ] 添加 `build_dll.sh`。
- [ ] 添加测试构建脚本，命名带测试目标。
- [ ] 添加最小测试框架入口。
- [ ] 添加 MVP 示例目录。

## 0.1 开发约定

- [ ] 确认代码风格遵守 `docs/代码风格规则.md`。
- [ ] 确认缩进使用 Tab。
- [ ] 确认类型命名使用 `xge_texture_t` 形式。
- [ ] 确认内部函数使用两条前导下划线，例如 `__xgeRenderFlush`。
- [ ] 确认第三方依赖直接复制到 `lib/`。
- [ ] 确认测试策略采用纯 C 测试框架。
- [ ] 确认测试流程采用人机协作反馈循环。
- [ ] 确认实现偏离设计时先警报、再更新文档、最后改代码。
- [ ] 确认每完成一个功能后更新本 spec。

## 0.2 第一阶段 MVP

- [ ] 建立根目录结构。
- [ ] 创建 `xge.h` / `xge.c` 骨架。
- [ ] 创建 `build_exe.bat` / `build_exe.sh`。
- [ ] 创建最小 example。
- [ ] 实现 `xgeInit`。
- [ ] 实现 `xgeUnit`。
- [ ] 实现 `xgeRun`。
- [ ] 实现 `xgeQuit`。
- [ ] 接入 xrt allocator/file/time 最小路径。
- [ ] 接入 Sokol 并打开窗口。
- [ ] 创建 OpenGL/GLES/WebGL context。
- [ ] 实现 clear/present。
- [ ] 实现最小键盘输入。
- [ ] 实现最小鼠标输入。
- [ ] 实现最小纹理加载。
- [ ] 实现 `xgeDraw` 绘制一张纹理。
- [ ] 完成人工冒烟测试。
- [ ] 更新 MVP 相关 spec 状态。

## 1. 核心 API

- [ ] 在 `xge.h` 中定义公开 API 命名规则。
- [ ] 定义 `XgeResult` 错误码枚举。
- [ ] 定义 `XgeDesc`。
- [ ] 定义版本宏。
- [ ] 定义编译期功能宏。
- [ ] 定义 debug/release 配置宏。
- [ ] 实现 `xgeInit`。
- [ ] 实现 `xgeUnit`。
- [ ] 实现 `xgeFrame`。
- [ ] 实现 `xgeRun`。
- [ ] 实现 `xgeQuit`。
- [ ] 实现 `xgeGetWidth`。
- [ ] 实现 `xgeGetHeight`。
- [ ] 实现 `xgeGetDelta`。
- [ ] 实现 `xgeGetFPS`。
- [ ] 实现 `xgeSetTitle`。
- [ ] 添加核心生命周期测试。

## 2. xrt 集成

- [ ] 将所有内存分配接入 xrt allocator。
- [ ] 添加 xrt 文件加载封装。
- [ ] 添加 xrt 可执行文件根目录路径支持。
- [ ] 添加用于异步资源的 xrt 线程/任务集成。
- [ ] 在 xrt logger 可用后接入日志。
- [ ] 添加 xrt 时间/计时器集成。
- [ ] 按需接入 xrt 字符串/路径工具。
- [ ] 添加 xrt 集成测试。

## 3. 平台后端

- [ ] 定义平台后端接口。
- [ ] 实现 Sokol 后端骨架。
- [ ] 实现 Windows Sokol 启动路径。
- [ ] 实现 Linux Sokol X11 路径。
- [ ] 实现 Linux Sokol Wayland 路径或能力检测。
- [ ] 实现 macOS Sokol 路径。
- [ ] 实现 Android Sokol 路径。
- [ ] 实现 iOS Sokol 路径。
- [ ] 实现 Web/Emscripten Sokol 路径。
- [ ] 定义小程序后端接口。
- [ ] 实现小程序 WebGL2 Canvas 后端骨架。
- [ ] 实现小程序 JS bridge 骨架。
- [ ] 实现 EGL 后端骨架。
- [ ] 实现 EGL pbuffer 离屏上下文。
- [ ] 在平台支持时实现 EGL surfaceless 上下文。
- [ ] 实现板卡 Linux EGL 路径。
- [ ] 添加平台能力报告。

## 4. GL/GLES/WebGL 后端

- [ ] 定义图形后端接口。
- [ ] 定义 GPU 能力结构体。
- [ ] 实现 OpenGL ES 3.0 函数加载。
- [ ] 实现桌面 OpenGL 3.3 Core 映射。
- [ ] 实现 WebGL2 映射层。
- [ ] 实现 shader 编译。
- [ ] 实现 shader 链接校验。
- [ ] 实现 buffer 创建/更新/释放。
- [ ] 实现 texture 创建/更新/释放。
- [ ] 实现 sampler state。
- [ ] 实现 render target/FBO 创建/释放。
- [ ] 实现 viewport/scissor。
- [ ] 实现 blend state。
- [ ] 实现 2.5D 高级 API 所需 depth state。
- [ ] 在 debug 模式实现 GL error 检查。
- [ ] 在 debug 模式实现 GPU caps dump。
- [ ] 添加图形后端测试。

## 5. 渲染命令系统

- [ ] 设计渲染命令数据模型。
- [ ] 实现 command queue。
- [ ] 让命令提交具备线程安全性。
- [ ] 实现 render thread 命令执行。
- [ ] 实现 `xgeBegin`。
- [ ] 实现 `xgeEnd`。
- [ ] 实现 `xgeFlush`。
- [ ] 实现 `xgePresent`。
- [ ] 实现每帧命令重置。
- [ ] 在 debug 模式实现 frame stats。
- [ ] 在 debug 模式统计 draw call count。
- [ ] 在 debug 模式统计 batch count。
- [ ] 添加 command queue 测试。

## 6. RenderPass 与 RenderTarget

- [ ] 定义 `XgeRenderTarget`。
- [ ] 定义 `XgePass`。
- [ ] 实现默认窗口 render target。
- [ ] 实现离屏 render target。
- [ ] 实现 `xgePassBegin`。
- [ ] 实现 `xgePassEnd`。
- [ ] 实现 clear flags。
- [ ] 实现 viewport 绑定。
- [ ] 实现 scissor 绑定。
- [ ] 实现 render target resize。
- [ ] 实现 screenshot/readback 慢路径。
- [ ] 添加 render target 测试。

## 7. 坐标、Camera 与数学

- [ ] 定义 `XgeVec2`。
- [ ] 定义 `XgeVec3`。
- [ ] 定义 `XgeMat3`。
- [ ] 定义 `XgeMat4`。
- [ ] 定义 `XgeRect`。
- [ ] 定义 `XgeColor`。
- [ ] 定义 camera 结构体。
- [ ] 实现默认 y-down 正交 camera。
- [ ] 实现居中原点坐标模式。
- [ ] 实现 camera transform。
- [ ] 实现 viewport transform。
- [ ] 实现 camera/viewport 相互独立。
- [ ] 实现像素 API 包装。
- [ ] 实现屏幕像素 bypass 路径。
- [ ] 添加坐标映射测试。

## 8. 颜色与 Blend

- [ ] 定义立即颜色格式 `0xRRGGBBAA`。
- [ ] 定义图像像素内存格式为 RGBA bytes。
- [ ] 实现颜色打包/解包辅助函数。
- [ ] 实现 premultiplied alpha 纹理加载标志。
- [ ] 实现 straight alpha 纹理加载标志。
- [ ] 实现 `XGE_BLEND_NONE`。
- [ ] 实现 `XGE_BLEND_ALPHA`。
- [ ] 实现 `XGE_BLEND_ADD`。
- [ ] 实现 `XGE_BLEND_MULTIPLY`。
- [ ] 实现 `XGE_BLEND_SCREEN`。
- [ ] 实现 `XGE_BLEND_CUSTOM`。
- [ ] 添加 blend 测试。

## 9. Image 与 Texture

- [ ] 定义 `XgeImage`。
- [ ] 定义 `XgeTexture`。
- [ ] 实现 `xgeImageLoad`。
- [ ] 实现 `xgeImageLoadMemory`。
- [ ] 实现 `xgeImageFree`。
- [ ] 实现 `xgeImageGetPixels`。
- [ ] 实现图像格式检测。
- [ ] 实现 PNG/JPG/BMP/TGA 加载策略。
- [ ] 实现 `xgeTextureCreate`。
- [ ] 实现 `xgeTextureCreateFromImage`。
- [ ] 实现 `xgeTextureLoad`。
- [ ] 实现 `xgeTextureLoadMemory`。
- [ ] 实现 `xgeTextureFree`。
- [ ] 实现 texture 引用计数。
- [ ] 实现 texture fallback resource。
- [ ] 实现 `xgeTextureReadPixels`。
- [ ] 添加 image/texture 测试。

## 10. 2D 绘制

- [ ] 定义 sprite 绘制命令。
- [ ] 实现 `xgeDraw`。
- [ ] 实现 `xgeDrawEx`。
- [ ] 实现 `xgeDrawPx`。
- [ ] 实现 source rect。
- [ ] 实现 destination rect。
- [ ] 实现 rotation。
- [ ] 实现 scale。
- [ ] 实现 origin/pivot。
- [ ] 实现 color modulation。
- [ ] 实现 alpha。
- [ ] 实现 flip/mirror。
- [ ] 实现 sprite batching。
- [ ] 实现 atlas-friendly batching。
- [ ] 添加 sprite 绘制示例。
- [ ] 添加 sprite 绘制测试。

## 11. Shape 绘制

- [ ] 定义 shape batcher。
- [ ] 实现点绘制。
- [ ] 实现线绘制。
- [ ] 实现矩形描边。
- [ ] 实现矩形填充。
- [ ] 实现圆形描边。
- [ ] 实现圆形填充。
- [ ] 实现弧线绘制。
- [ ] 实现三角形绘制。
- [ ] 实现多边形绘制。
- [ ] 实现逻辑坐标 API。
- [ ] 实现像素坐标 API。
- [ ] 添加 shape 示例。
- [ ] 添加 shape 测试。

## 12. Material 与 Shader

- [ ] 定义 `XgeMaterial`。
- [ ] 定义 `XgeShader`。
- [ ] 定义 pipeline state 结构体。
- [ ] 实现 material 创建/释放。
- [ ] 实现 material texture 绑定。
- [ ] 实现 material color/uniform 绑定。
- [ ] 实现 material blend 绑定。
- [ ] 实现 shader 创建/释放。
- [ ] 实现 shader variant 策略。
- [ ] 实现自定义 shader 绘制路径。
- [ ] 实现自定义 uniform 上传。
- [ ] 添加 gray shader/material effect。
- [ ] 添加 distortion shader/material 示例。
- [ ] 添加 material/shader 测试。

## 13. 高级 2.5D

- [ ] 定义高级 vertex format。
- [ ] 定义 mesh 对象。
- [ ] 实现自定义 vertex 绘制。
- [ ] 实现 indexed mesh 绘制。
- [ ] 实现 z/order 支持。
- [ ] 实现 depth test 开关。
- [ ] 实现 mesh draw 的正交 camera。
- [ ] 实现可选 perspective camera。
- [ ] 实现 perspective quad/image warp 示例。
- [ ] 实现 isometric depth-order 示例。
- [ ] 添加 2.5D 测试。

## 14. 场景系统

- [ ] 定义 `XgeScene`。
- [ ] 实现场景 user data。
- [ ] 实现 `onEnter`。
- [ ] 实现 `onLeave`。
- [ ] 实现 `onPause`。
- [ ] 实现 `onResume`。
- [ ] 实现 `onEvent`。
- [ ] 实现 `onUpdate`。
- [ ] 实现 `onDraw`。
- [ ] 实现 `onFree`。
- [ ] 实现 `xgeSceneSet`。
- [ ] 实现 `xgeScenePush`。
- [ ] 实现 `xgeScenePop`。
- [ ] 实现 `xgeSceneReplace`。
- [ ] 实现场景栈。
- [ ] 实现 fixed update 策略。
- [ ] 实现 variable update 策略。
- [ ] 实现 multiple-update-per-draw 策略。
- [ ] 添加场景示例。
- [ ] 添加场景测试。

## 15. 运行模式与 APP 渲染

- [ ] 定义 `XgeRunMode`。
- [ ] 实现 `XGE_RUN_GAME_LOOP`。
- [ ] 实现 `XGE_RUN_MANUAL`。
- [ ] 实现手动 `xgeFrame`。
- [ ] 实现手动 `xgeRender`。
- [ ] 实现 `xgeInvalidateRect`。
- [ ] 实现 dirty rect 跟踪。
- [ ] 将 dirty rect 与 RMGUI/layout 渲染集成。
- [ ] 添加 app-mode 示例。
- [ ] 添加运行模式测试。

## 16. 输入

- [ ] 定义 `XgeEvent`。
- [ ] 将 Sokol key code 映射为 XGE key code。
- [ ] 实现 keyboard down 状态。
- [ ] 实现 keyboard pressed 状态。
- [ ] 实现 keyboard released 状态。
- [ ] 实现 mouse position。
- [ ] 实现 mouse button 状态。
- [ ] 实现 mouse wheel。
- [ ] 实现 text input 事件。
- [ ] 实现 IME 事件路径。
- [ ] 实现 gamepad 状态。
- [ ] 实现 gamepad connection 事件。
- [ ] 添加 keyboard/mouse 示例。
- [ ] 添加 text input/IME 示例。
- [ ] 添加 gamepad 示例。
- [ ] 添加输入测试。

## 17. Touch

- [ ] 定义 `XGE_TOUCH_MAX`。
- [ ] 定义 `XgeTouchPoint`。
- [ ] 定义 `XgeTouchEvent`。
- [ ] 定义 touch phase enum。
- [ ] 实现 touch begin。
- [ ] 实现 touch move。
- [ ] 实现 touch stationary。
- [ ] 实现 touch end。
- [ ] 实现 touch cancel。
- [ ] 实现 `xgeTouchGetCount`。
- [ ] 实现 `xgeTouchGet`。
- [ ] 实现 `xgeTouchFind`。
- [ ] 实现多点触控状态跟踪。
- [ ] 添加 touch 示例。
- [ ] 添加 touch 测试。

## 18. 异步资源

- [ ] 定义异步资源状态枚举。
- [ ] 定义异步请求对象。
- [ ] 实现异步 image 加载。
- [ ] 实现异步 texture 加载。
- [ ] 实现异步 font 加载。
- [ ] 实现异步 sound 加载。
- [ ] 实现 fallback texture。
- [ ] 实现 fallback font。
- [ ] 实现 fallback sound。
- [ ] 实现 GPU upload queue。
- [ ] 实现完成回调。
- [ ] 实现取消。
- [ ] 添加异步加载示例。
- [ ] 添加异步资源测试。

## 19. 资源协议

- [ ] 定义资源协议接口。
- [ ] 实现基于可执行文件根目录的文件路径加载。
- [ ] 实现 memory resource 加载。
- [ ] 实现 `res://` resolver。
- [ ] 添加 xpack-backed `res://` provider hook。
- [ ] 添加 custom provider 注册。
- [ ] 添加协议测试。

## 20. 音频

- [ ] 添加 miniaudio 依赖。
- [ ] 定义 audio backend wrapper。
- [ ] 定义 `XgeSound`。
- [ ] 定义 `XgeMusic`。
- [ ] 定义 `XgeStream`。
- [ ] 定义 `XgeAudioGroup`。
- [ ] 定义 `XgeAudioListener`。
- [ ] 决定第一版音频文件格式。
- [ ] 实现 audio device 初始化/关闭。
- [ ] 实现 sound 加载/释放。
- [ ] 实现 music 加载/释放。
- [ ] 实现 stream 打开/关闭。
- [ ] 实现 sound play/stop/pause/resume。
- [ ] 实现 music play/stop/pause/resume。
- [ ] 实现 loop 控制。
- [ ] 实现 volume 控制。
- [ ] 实现 group volume。
- [ ] 实现 fade in/out。
- [ ] 实现 3D position playback。
- [ ] 实现 listener position/orientation。
- [ ] 添加音频示例。
- [ ] 添加音频测试。

## 21. 文本与字体

- [ ] 添加 stb_truetype 依赖。
- [ ] 定义 `XgeFont`。
- [ ] 定义 glyph metrics。
- [ ] 定义 glyph atlas。
- [ ] 实现 UTF-8 decoder。
- [ ] 实现 UCS2 range 支持。
- [ ] 实现从 TTF 加载字体。
- [ ] 实现从 memory 加载字体。
- [ ] 实现 fallback font chain。
- [ ] 实现 glyph rasterization。
- [ ] 实现 glyph atlas packing。
- [ ] 实现 text measurement。
- [ ] 实现单行文本绘制。
- [ ] 实现多行文本绘制。
- [ ] 实现 rect text draw。
- [ ] 实现 text alignment。
- [ ] 实现文本 clipping/scissor。
- [ ] 添加中文文本示例。
- [ ] 添加文本测试。

## 22. XRF 字体格式

- [ ] 设计 XRF binary header。
- [ ] 设计 XRF glyph record。
- [ ] 设计 XRF atlas page record。
- [ ] 支持 A8 atlas pages。
- [ ] 支持 RGBA8 atlas pages。
- [ ] 支持 UCS2 glyph ranges。
- [ ] 支持 glyph metrics。
- [ ] 支持 ascent/descent/line height。
- [ ] 支持可选 kerning。
- [ ] 实现 XRF loader。
- [ ] 实现 XRF font draw path。
- [ ] 实现 TTF-to-XRF cache path。
- [ ] 添加 XRF generation tool plan。
- [ ] 添加 XRF 测试。

## 23. 布局系统

- [ ] 定义 layout node。
- [ ] 定义 layout style。
- [ ] 定义 layout rect/result。
- [ ] 实现 retained layout tree。
- [ ] 实现 absolute layout。
- [ ] 实现 horizontal flow layout。
- [ ] 实现 vertical flow layout。
- [ ] 实现 fixed size。
- [ ] 实现 ratio size。
- [ ] 实现 min size。
- [ ] 实现 max size。
- [ ] 实现 padding。
- [ ] 实现 margin。
- [ ] 实现 alignment。
- [ ] 实现 anchor。
- [ ] 实现 z/order。
- [ ] 实现 clip/scissor。
- [ ] 实现 dirty layout propagation。
- [ ] 实现 cached layout result rects。
- [ ] 实现 dirty rect output。
- [ ] 添加布局示例。
- [ ] 添加布局测试。

## 24. RMGUI 集成

- [ ] 定义 RMGUI/XGE 边界。
- [ ] 定义 UI 绘制接口。
- [ ] 将布局输出与 XGE render batch 集成。
- [ ] 将输入事件与 UI hit testing 集成。
- [ ] 将文本测量与布局集成。
- [ ] 集成 dirty rect rendering。
- [ ] 添加基础 label widget。
- [ ] 添加基础 button widget。
- [ ] 添加基础 image widget。
- [ ] 添加 scroll view baseline。
- [ ] 添加 RMGUI 示例。

## 25. 小程序后端

- [ ] 定义小程序 C-facing platform hooks。
- [ ] 定义 JS bridge API。
- [ ] 实现 WebGL2 Canvas context creation。
- [ ] 实现 touch event bridge。
- [ ] 在平台支持时实现 keyboard/text event bridge。
- [ ] 实现 audio bridge 策略或 miniaudio 可行性检查。
- [ ] 通过小程序 API 实现 resource loading。
- [ ] 实现 game loop bridge。
- [ ] 添加小程序构建脚本。
- [ ] 添加小程序 hello 示例。

## 26. 离屏与板卡 Linux

- [ ] 实现 offscreen EGL init。
- [ ] 实现 offscreen render target creation。
- [ ] 实现 offscreen readback。
- [ ] 实现 board Linux EGL init。
- [ ] 添加 DRM/KMS/GBM 设计说明。
- [ ] 添加 board Linux input strategy 说明。
- [ ] 添加 offscreen 示例。
- [ ] 添加 board Linux smoke test plan。

## 27. Debug 与诊断

- [ ] 定义 `XgeDebugStats`。
- [ ] 实现 `xgeDebugDumpCaps`。
- [ ] 实现 `xgeDebugGetStats`。
- [ ] 在 debug 模式实现 GL error checks。
- [ ] 实现 frame time stats。
- [ ] 实现 draw call stats。
- [ ] 实现 batch stats。
- [ ] 实现 texture memory stats。
- [ ] 实现 resource count stats。
- [ ] 在 xrt logger 可用后集成日志。
- [ ] 添加 debug overlay 示例。

## 28. 示例

- [ ] Hello window。
- [ ] Draw shape。
- [ ] Draw texture。
- [ ] Draw text。
- [ ] Input keyboard/mouse。
- [ ] Touch。
- [ ] Gamepad。
- [ ] Scene stack。
- [ ] Render target。
- [ ] Shader/material。
- [ ] 2.5D perspective quad。
- [ ] Audio sound/music/3D。
- [ ] Async resource fallback。
- [ ] Layout basic。
- [ ] RMGUI basic。
- [ ] Offscreen rendering。
- [ ] Mini program hello。

## 29. 文档

- [ ] 持续更新 `XGE_V2_DESIGN.md`。
- [ ] 开发过程中持续更新本 spec。
- [ ] 编写构建脚本文档。
- [ ] 编写单头文件使用文档。
- [ ] 编写 API 命名规则文档。
- [ ] 编写平台支持矩阵文档。
- [ ] 编写资源加载文档。
- [ ] 编写渲染管线文档。
- [ ] 编写场景系统文档。
- [ ] 编写布局系统文档。
- [ ] 编写 XRF 格式文档。
- [ ] 编写小程序后端文档。
