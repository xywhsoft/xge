# XGE V2 规格与进度跟踪

本文件用于跟踪 XGE V2 的实现进度。后续开发时，请随着设计、实现、测试和文档完善同步更新勾选状态。

状态说明：

- `[ ]` 未开始
- `[~]` 进行中
- `[x]` 已完成

## 0. 项目搭建

- [x] 确认 XGE V2 的仓库目录结构。
- [x] 确认单头文件源码的最终路径。
- [x] 在根目录添加 `xge.h`。
- [x] 在根目录添加 `xge.c`。
- [x] 创建 `src/` 引擎源码目录。
- [x] 创建 `lib/` 第三方库目录。
- [x] 创建 `test/` 测试目录。
- [x] 创建 `singlehead/` 单头文件目录。
- [x] 创建 `examples/` 示例目录。
- [x] 添加 `xge.h` 开发头文件骨架。
- [x] 确认 `xge.h` 只保留公开 API 声明。
- [x] 确认实现代码按功能模块放入 `src/`。
- [x] 将 Image/Texture/Sampler 模块拆分为 `src/xge_texture.c`。
- [x] 将 Shader/Material 模块拆分为 `src/xge_material.c`。
- [x] 将 Buffer 模块拆分为 `src/xge_buffer.c`。
- [x] 将 RenderTarget/RenderPass 模块拆分为 `src/xge_render_target.c`。
- [x] 将 SpriteBatch 模块拆分为 `src/xge_sprite.c`。
- [x] 将 Mesh/2.5D 模块拆分为 `src/xge_mesh.c`。
- [x] 将 Shape 绘制模块拆分为 `src/xge_shape.c`。
- [x] 将 XUI 孵化模块拆分为 host/layout/core/text/controls/scroll/dialog 多个源码文件。
- [x] 确认单头文件由 `singlehead/` 工具从公开头和模块源码组装。
- [x] 添加第三方库 `lib/` 目录结构。
- [x] 添加 xrt 依赖到 `lib/`。
- [x] 添加 sokol 依赖到 `lib/`。
- [x] 添加 stb_image 依赖到 `lib/`。
- [x] 添加 miniaudio 依赖到 `lib/`。
- [x] 添加 stb_truetype 依赖到 `lib/`。
- [x] 添加 `examples\mvp\build.bat`。
- [x] 添加 `examples/mvp/build.sh`。
- [x] 添加 `build_dll.bat`。
- [x] 添加 `build_dll.sh`。
- [x] 添加测试构建脚本，命名带测试目标。
- [x] 添加最小测试框架入口。
- [x] 添加 MVP 示例目录。

## 0.1 开发约定

- [x] 确认代码风格遵守 `docs/代码风格规则.md`。
- [x] 确认缩进使用 Tab。
- [x] 确认类型命名使用 `xge_texture_t` 形式。
- [x] 确认内部函数使用两条前导下划线，例如 `__xgeRenderFlush`。
- [x] 确认第三方依赖直接复制到 `lib/`。
- [x] 确认测试策略采用纯 C 测试框架。
- [x] 确认测试流程采用人机协作反馈循环。
- [x] 确认实现偏离设计时先警报、再更新文档、最后改代码。
- [x] 确认每完成一个功能后更新本 spec。

## 0.2 第一阶段 MVP

- [x] 建立根目录结构。
- [x] 创建 `xge.h` / `xge.c` 骨架。
- [x] 创建 `examples\mvp\build.bat` / `examples/mvp/build.sh`。
- [x] 创建最小 example。
- [x] 实现 `xgeInit`。
- [x] 实现 `xgeUnit`。
- [x] 实现 `xgeRun`。
- [x] 实现 `xgeQuit`。
- [x] 接入 xrt allocator/file/time 最小路径。
- [x] 接入 Sokol 并打开窗口。
- [x] 创建 OpenGL/GLES/WebGL context。
- [x] 实现 clear/present。
- [x] 实现最小键盘输入。
- [x] 实现最小鼠标输入。
- [x] 实现最小纹理加载。
- [x] 实现 `xgeDraw` 绘制一张纹理。
- [x] 编写人工冒烟测试流程文档。
- [x] 完成人工冒烟测试（Windows/Sokol/OpenGL33 基线）。
- [x] 更新 MVP 相关 spec 状态。

## 1. 核心 API

- [x] 在 `xge.h` 中定义公开 API 命名规则。
- [x] 定义 `xge_result_t` 错误码枚举。
- [x] 定义 `xge_desc_t`。
- [x] 定义版本宏。
- [x] 定义编译期功能宏。
- [x] 定义 debug/release 配置宏。
- [x] 实现 `xgeInit`。
- [x] 实现 `xgeUnit`。
- [x] 实现 `xgeFrame`。
- [x] 实现 `xgeRun`。
- [x] 实现 `xgeQuit`。
- [x] 实现 `xgeGetWidth`。
- [x] 实现 `xgeGetHeight`。
- [x] 实现 `xgeGetDelta`。
- [x] 实现 `xgeGetFPS`。
- [x] 实现 `xgeSetTitle`。
- [x] 添加核心生命周期测试。

## 2. xrt 集成

- [x] 将所有内存分配接入 xrt allocator。
- [x] 添加 xrt 文件加载封装。
- [x] 添加 xrt 可执行文件根目录路径支持。
- [x] 添加用于异步资源的 xrt 线程/任务集成。
- [x] 编写 xrt logger 接入设计说明。
- [x] 在 xrt logger 可用后接入日志。
- [x] 添加 xrt 时间/计时器集成。
- [x] 按需接入 xrt 字符串/路径工具。
- [x] 添加 xrt 集成测试。

## 3. 平台后端

- [x] 定义平台后端接口。
- [x] 实现 Sokol 后端骨架。
- [x] 实现 Windows Sokol 启动路径。
- [x] 完成 Windows Sokol/OpenGL33 人工冒烟验证。
- [x] 编写 Sokol 跨平台冒烟测试计划。
- [x] 编写 Sokol 后端边界说明。
- [x] 添加平台后端验证结果模板。
- [x] 添加平台后端验证结果检查脚本。
- [x] 添加平台后端验证结果记录脚本。
- [x] 添加平台后端 scaffold 完整性检查脚本。
- [x] 添加平台后端工具链检查脚本。
- [x] 添加平台后端批量准备检查脚本。
- [x] 添加脚本文档引用一致性检查脚本。
- [x] 添加通用 platform smoke 示例。
- [x] 添加 Linux X11/OpenGL platform smoke 构建脚本。
- [x] 添加 Linux X11/EGL/GLES3 MVP 构建脚本。
- [ ] 实现 Linux Sokol X11 路径。
- [x] 实现 Linux Sokol Wayland 路径或能力检测。
- [x] 添加 macOS OpenGL MVP 构建脚本。
- [ ] 实现 macOS Sokol 路径。
- [x] 添加 Android NativeActivity/CMake scaffold。
- [x] 添加 Android NDK Windows 构建脚本。
- [x] 添加 Android Gradle APK scaffold。
- [x] 添加 Android adb 安装启动脚本。
- [ ] 实现 Android Sokol 路径。
- [x] 添加 iOS simulator app scaffold。
- [x] 添加 iOS simulator 安装启动脚本。
- [ ] 实现 iOS Sokol 路径。
- [x] 添加 Web/Emscripten MVP 构建脚本。
- [x] 添加 Web/Emscripten HTML shell scaffold。
- [x] 添加 Web/Emscripten 本地 serve 脚本。
- [ ] 实现 Web/Emscripten Sokol 路径。
- [x] 定义小程序后端接口。
- [x] 实现小程序 WebGL2 Canvas 后端骨架。
- [x] 实现小程序 JS bridge 骨架。
- [x] 实现 EGL 后端骨架。
- [x] 添加 EGL 编译期能力报告。
- [x] 添加 EGL 初始化失败阶段诊断。
- [x] 实现 EGL pbuffer 离屏上下文。
- [x] 添加 EGL surfaceless 冒烟示例和构建脚本。
- [x] 添加板卡 Linux EGL 一键冒烟脚本。
- [ ] 在平台支持时实现 EGL surfaceless 上下文。
- [ ] 实现板卡 Linux EGL 路径。
- [x] 添加平台能力报告。
- [x] 添加 Sokol 编译目标能力报告。
- [x] 添加 Sokol 编译图形后端能力报告。
- [x] 拆分平台输入能力报告中的 touch/mouse/high DPI 字段。
- [x] 添加平台运行态快照 API。
- [x] 在 platform smoke 中输出窗口、framebuffer、DPI 和平台事件计数。
- [x] 为 platform smoke 添加自动退出参数。
- [x] 添加 platform smoke 日志检查脚本。
- [x] 添加 platform smoke 构建、运行、日志检查包装脚本。
- [x] 添加 Linux X11、Linux X11/EGL、macOS platform smoke 一键验证脚本。

## 4. GL/GLES/WebGL 后端

- [x] 定义图形后端接口。
- [x] 定义 GPU 能力结构体。
- [x] 实现 OpenGL ES 3.0 函数加载。
- [x] 实现桌面 OpenGL 3.3 Core 映射。
- [x] 实现 WebGL2 映射层。
- [x] 实现 shader 编译。
- [x] 实现 shader 链接校验。
- [x] 实现 buffer 创建/更新/释放。
- [x] 实现 texture 创建/更新/释放。
- [x] 实现 sampler state。
- [x] 实现 render target/FBO 创建/释放。
- [x] 实现 viewport/scissor。
- [x] 实现 blend state。
- [x] 实现 2.5D 高级 API 所需 depth state。
- [x] 在 debug 模式实现 GL error 检查。
- [x] 在 debug 模式实现 GPU caps dump。
- [x] 添加图形后端测试。

## 5. 渲染命令系统

- [x] 设计渲染命令数据模型。
- [x] 实现 command queue。
- [x] 让命令提交具备线程安全性。
- [x] 实现 render thread 命令执行。
- [x] 添加无活动 GL context 时的 xrt worker 命令 drain 路径。
- [x] 添加 render thread 能力报告 API。
- [x] 编写拥有 GL context 的专用 render thread 设计说明。
- [x] 添加 EGL/offscreen render thread context owner 配置 API。
- [x] 添加 render thread 内创建 EGL/GLES3 context 并同步 flush 的执行路径。
- [ ] 实现拥有 GL context 的专用 render thread。
- [x] 实现 `xgeBegin`。
- [x] 实现 `xgeEnd`。
- [x] 实现 `xgeFlush`。
- [x] 实现 `xgePresent`。
- [x] 实现每帧命令重置。
- [x] 在 debug 模式实现 frame stats。
- [x] 在 debug 模式统计 draw call count。
- [x] 在 debug 模式统计 batch count。
- [x] 添加 command queue 测试。

## 6. RenderPass 与 RenderTarget

- [x] 定义 `XgeRenderTarget`。
- [x] 定义 `XgePass`。
- [x] 实现默认窗口 render target。
- [x] 实现离屏 render target。
- [x] 实现 `xgePassBegin`。
- [x] 实现 `xgePassEnd`。
- [x] 实现 clear flags。
- [x] 实现 viewport 绑定。
- [x] 实现 scissor 绑定。
- [x] 实现 render target resize。
- [x] 实现 screenshot/readback 慢路径。
- [x] 添加 render target 测试。

## 7. 坐标、Camera 与数学

- [x] 定义 `xge_vec2_t`。
- [x] 定义 `xge_vec3_t`。
- [x] 定义 `xge_mat3_t`。
- [x] 定义 `xge_mat4_t`。
- [x] 定义 `xge_rect_t`。
- [x] 定义 `xge_color_t`。
- [x] 定义 camera 结构体。
- [x] 实现默认 y-down 正交 camera。
- [x] 实现居中原点坐标模式。
- [x] 实现 camera transform。
- [x] 实现 viewport transform。
- [x] 实现 camera/viewport 相互独立。
- [x] 实现像素 API 包装。
- [x] 实现屏幕像素 bypass 路径。
- [x] 添加坐标映射测试。

## 8. 颜色与 Blend

- [x] 定义立即颜色格式 `0xRRGGBBAA`。
- [x] 定义图像像素内存格式为 RGBA bytes。
- [x] 实现颜色打包/解包辅助函数。
- [x] 实现 premultiplied alpha 纹理加载标志。
- [x] 实现 straight alpha 纹理加载标志。
- [x] 实现 `XGE_BLEND_NONE`。
- [x] 实现 `XGE_BLEND_ALPHA`。
- [x] 实现 `XGE_BLEND_ADD`。
- [x] 实现 `XGE_BLEND_MULTIPLY`。
- [x] 实现 `XGE_BLEND_SCREEN`。
- [x] 实现 `XGE_BLEND_CUSTOM`。
- [x] 添加 blend 测试。

## 9. Image 与 Texture

- [x] 定义 `xge_image_t`。
- [x] 定义 `xge_texture_t`。
- [x] 实现 `xgeImageLoad`。
- [x] 实现 `xgeImageLoadMemory`。
- [x] 实现 `xgeImageFree`。
- [x] 实现 `xgeImageGetPixels`。
- [x] 实现图像格式检测。
- [x] 实现 PNG/JPG/BMP/TGA 加载策略。
- [x] 实现 `xgeTextureCreateRGBA`。
- [x] 实现 `xgeTextureCreateFromImage`。
- [x] 实现 `xgeTextureUpdateRGBA`。
- [x] 实现 `xgeTextureLoad`。
- [x] 实现 `xgeTextureLoadMemory`。
- [x] 实现 `xgeTextureFree`。
- [x] 实现 texture 引用计数。
- [x] 实现 texture fallback resource。
- [x] 实现 `xgeTextureReadPixels`。
- [x] 添加 image/texture 测试。

## 10. 2D 绘制

- [x] 定义 sprite 绘制命令。
- [x] 实现 `xgeDraw`。
- [x] 实现 `xgeDrawEx`。
- [x] 实现 `xgeDrawPx`。
- [x] 实现 source rect。
- [x] 实现 destination rect。
- [x] 实现 rotation。
- [x] 实现 scale。
- [x] 实现 origin/pivot。
- [x] 实现 color modulation。
- [x] 实现 alpha。
- [x] 实现 flip/mirror。
- [x] 实现 sprite batching。
- [x] 实现 atlas-friendly batching。
- [x] 添加 sprite 绘制示例。
- [x] 添加 sprite 绘制测试。

## 11. Shape 绘制

- [x] 定义 shape batcher。
- [x] 实现点绘制。
- [x] 实现线绘制。
- [x] 实现矩形描边。
- [x] 实现矩形填充。
- [x] 实现圆形描边。
- [x] 实现圆形填充。
- [x] 实现弧线绘制。
- [x] 实现三角形绘制。
- [x] 实现多边形绘制。
- [x] 实现逻辑坐标 API。
- [x] 实现像素坐标 API。
- [x] 添加 shape 示例。
- [x] 添加 shape 测试。

## 12. Material 与 Shader

- [x] 定义 `XgeMaterial`。
- [x] 定义 `XgeShader`。
- [x] 定义 pipeline state 结构体。
- [x] 实现 material 创建/释放。
- [x] 实现 material texture 绑定。
- [x] 实现 material color/uniform 绑定。
- [x] 实现 material blend 绑定。
- [x] 实现 shader 创建/释放。
- [x] 实现 shader variant 策略。
- [x] 实现自定义 shader 绘制路径。
- [x] 实现自定义 uniform 上传。
- [x] 添加 gray shader/material effect。
- [x] 添加 distortion shader/material 示例。
- [x] 添加 material/shader 测试。

## 13. 高级 2.5D

- [x] 定义高级 vertex format。
- [x] 定义 mesh 对象。
- [x] 实现自定义 vertex 绘制。
- [x] 实现 indexed mesh 绘制。
- [x] 实现 z/order 支持。
- [x] 实现 depth test 开关。
- [x] 实现 mesh draw 的正交 camera。
- [x] 实现可选 perspective camera。
- [x] 实现 perspective quad/image warp 示例。
- [x] 实现 isometric depth-order 示例。
- [x] 添加 2.5D 测试。

## 14. 场景系统

- [x] 定义 `XgeScene`。
- [x] 实现场景 user data。
- [x] 实现 `onEnter`。
- [x] 实现 `onLeave`。
- [x] 实现 `onPause`。
- [x] 实现 `onResume`。
- [x] 实现 `onEvent`。
- [x] 实现 `onUpdate`。
- [x] 实现 `onDraw`。
- [x] 实现 `onFree`。
- [x] 实现 `xgeSceneSet`。
- [x] 实现 `xgeScenePush`。
- [x] 实现 `xgeScenePop`。
- [x] 实现 `xgeSceneReplace`。
- [x] 实现场景栈。
- [x] 实现 fixed update 策略。
- [x] 实现 variable update 策略。
- [x] 实现 multiple-update-per-draw 策略。
- [x] 添加场景示例。
- [x] 添加场景测试。

## 15. 运行模式与 APP 渲染

- [x] 定义 `XgeRunMode`。
- [x] 实现 `XGE_RUN_GAME_LOOP`。
- [x] 实现 `XGE_RUN_MANUAL`。
- [x] 实现手动 `xgeFrame`。
- [x] 实现手动 `xgeRender`。
- [x] 实现 `xgeInvalidateRect`。
- [x] 实现 dirty rect 跟踪。
- [x] 将 dirty rect 与 XUI app-mode 渲染集成。
- [x] 添加 app-mode 示例。
- [x] 添加运行模式测试。

## 16. 输入

- [x] 定义 `xge_event_t`。
- [x] 将 Sokol key code 映射为 XGE key code。
- [x] 实现 keyboard down 状态。
- [x] 实现 keyboard pressed 状态。
- [x] 实现 keyboard released 状态。
- [x] 实现 mouse position。
- [x] 实现 mouse button 状态。
- [x] 实现 mouse wheel。
- [x] 实现 text input 事件。
- [x] 实现 IME 事件路径。
- [x] 实现 gamepad 状态。
- [x] 实现 gamepad connection 事件。
- [x] 添加 keyboard/mouse 示例。
- [x] 添加 text input/IME 示例。
- [x] 添加 gamepad 示例。
- [x] 添加输入测试。

## 17. Touch

- [x] 定义 `XGE_TOUCH_MAX`。
- [x] 定义 `xge_touch_point_t`。
- [x] 定义 `xge_touch_event_t`。
- [x] 定义 touch phase enum。
- [x] 实现 touch begin。
- [x] 实现 touch move。
- [x] 实现 touch stationary。
- [x] 实现 touch end。
- [x] 实现 touch cancel。
- [x] 实现 `xgeTouchGetCount`。
- [x] 实现 `xgeTouchGet`。
- [x] 实现 `xgeTouchFind`。
- [x] 实现多点触控状态跟踪。
- [x] 添加 touch 示例。
- [x] 添加 touch 测试。

## 18. 异步资源

- [x] 定义异步资源状态枚举。
- [x] 定义异步请求对象。
- [x] 实现异步 image 加载。
- [x] 实现异步 texture 加载。
- [x] 实现异步 font 加载。
- [x] 实现异步 sound 加载。
- [x] 实现 fallback texture。
- [x] 实现 fallback font。
- [x] 实现 fallback sound。
- [x] 实现 GPU upload queue。
- [x] 实现完成回调。
- [x] 实现取消。
- [x] 添加异步加载示例。
- [x] 添加异步资源测试。

## 19. 资源协议

- [x] 定义资源协议接口。
- [x] 实现基于可执行文件根目录的文件路径加载。
- [x] 实现 memory resource 加载。
- [x] 实现 `res://` resolver。
- [x] 添加 xpack-backed `res://` provider hook。
- [x] 添加 custom provider 注册。
- [x] 添加协议测试。

## 20. 音频

- [x] 添加 miniaudio 依赖。
- [x] 定义 audio backend wrapper。
- [x] 定义 `xge_sound_t`。
- [x] 定义 `xge_music_t`。
- [x] 定义 `xge_stream_t`。
- [x] 定义 `xge_audio_group_t`。
- [x] 定义 `xge_audio_listener_t`。
- [x] 决定第一版音频文件格式。
- [x] 实现 audio device 初始化/关闭。
- [x] 实现 sound 加载/释放。
- [x] 实现 music 加载/释放。
- [x] 实现 stream 打开/关闭。
- [x] 实现 sound play/stop/pause/resume。
- [x] 实现 music play/stop/pause/resume。
- [x] 实现 loop 控制。
- [x] 实现 volume 控制。
- [x] 实现 group volume。
- [x] 实现 fade in/out。
- [x] 实现 3D position playback。
- [x] 实现 listener position/orientation。
- [x] 添加音频示例。
- [x] 添加音频测试。

## 21. 文本与字体

- [x] 添加 stb_truetype 依赖。
- [x] 定义 `xge_font_t`。
- [x] 定义 glyph metrics。
- [x] 定义 glyph atlas。
- [x] 实现 UTF-8 decoder。
- [x] 实现 UCS2 range 支持。
- [x] 实现从 TTF 加载字体。
- [x] 实现从 memory 加载字体。
- [x] 实现 fallback font chain。
- [x] 实现 glyph rasterization。
- [x] 实现 glyph atlas packing。
- [x] 实现 text measurement。
- [x] 实现单行文本绘制。
- [x] 实现多行文本绘制。
- [x] 实现 rect text draw。
- [x] 实现 text alignment。
- [x] 实现文本 clipping/scissor。
- [x] 添加中文文本示例。
- [x] 添加文本测试。

## 22. XRF 字体格式

- [x] 设计 XRF binary header。
- [x] 设计 XRF glyph record。
- [x] 设计 XRF atlas page record。
- [x] 支持 A8 atlas pages。
- [x] 支持 RGBA8 atlas pages。
- [x] 支持 UCS2 glyph ranges。
- [x] 支持 glyph metrics。
- [x] 支持 ascent/descent/line height。
- [x] 支持可选 kerning。
- [x] 实现 XRF loader。
- [x] 实现 XRF font draw path。
- [x] 实现 TTF-to-XRF cache path。
- [x] 添加 XRF generation tool plan。
- [x] 添加 XRF 测试。

## 23. XUI 默认桥接

> 2026-05-07 口径更新：本节历史 `[x]` 表示 XUI 孵化期 baseline，不表示当前 Widget 基础层已经成熟。clip、Z、事件、焦点、滚动、IME 和 box model 必须按 `XUI Widget V2基础SPEC.md` 重新验收。

- [x] 确认布局系统归属 XUI，不进入 XGE 内核。
- [x] 确认 XUI 是 XGE 的默认 RMGUI 接入方案。
- [x] 确认 XGE 不依赖 XUI。
- [x] 确认 XUI backend for XGE 依赖 XGE 和 XUI core。
- [x] 确认开发期采用 XGE 内部孵化 XUI，成熟后剥离到 XUI 仓库。
- [x] 创建 XGE 内部 XUI 孵化模块目录/文件。
- [x] 定义 `xge_xui_context_t`，避免把 XUI 状态塞进 XGE core context。
- [x] 定义孵化期 `xge_xui_*` API。
- [x] 实现孵化期 XUI event/layout/widget/paint 基础模块。
- [x] 实现孵化期 absolute/row/column/stack 基础布局。
- [x] 实现孵化期 grid-lite 基础布局。
- [x] 实现孵化期 min/max size 约束。
- [x] 实现孵化期 align 布局。
- [x] 实现孵化期 z/order。说明：这是历史 baseline；Widget V2 后必须迁移到统一 layer/zIndex/treeOrder。
- [x] 实现孵化期 anchor 布局。
- [x] 实现孵化期 DIP 布局单位。
- [x] 实现孵化期 content 布局尺寸。
- [x] 实现孵化期 layout batch update。
- [x] 实现孵化期 widget visible/enabled/focusable 状态 API。
- [x] 实现孵化期 widget id/name 与查找 API。
- [x] 实现孵化期 button 行为 helper。
- [x] 实现孵化期 button 视觉绘制 helper。
- [x] 实现孵化期 label 绘制 helper。
- [x] 实现孵化期 image 绘制 helper。
- [x] 实现孵化期 input baseline helper。
- [x] 实现孵化期 input selection baseline。
- [x] 实现孵化期 toggle baseline helper。
- [x] 实现孵化期 slider baseline helper。
- [x] 实现孵化期 progress baseline helper。
- [x] 实现孵化期 panel baseline helper。
- [x] 实现孵化期 scroll view baseline helper。
- [x] 实现孵化期 list view baseline helper。
- [x] 实现孵化期 dialog baseline helper。
- [x] 实现孵化期 widget clip flag 与 XGE scissor 接入。
- [x] 将孵化期 paint 集中映射到 XGE draw 能力。
- [x] 实现孵化期 paint dirty flag。
- [x] 添加孵化期 XUI rect/image/text/clip paint backend 测试。
- [x] 实现孵化期 XUI custom paint command。
- [x] 定义孵化期 XUI paint command 并实现立即 flush 路径。
- [x] 实现孵化期 XUI rounded rect command（当前渲染退化为 rect，等待 shape 圆角实现）。
- [x] 实现孵化期 XUI simple theme baseline。
- [x] 实现孵化期 XUI widget local style override。
- [x] 为孵化期 XUI 添加 XGE 侧回归测试。
- [x] 定义 XGE 提供给 XUI 的 host/backend 接口。
- [x] 定义 XGE 统一事件结构，用于向 XUI 传递 pointer/key/text/IME 事件。
- [x] 实现孵化期 XUI focus in/out 事件。
- [x] 实现孵化期 XUI event queue。
- [x] 实现孵化期 XUI capture phase。
- [x] 实现 pointer event adapter。
- [x] 实现 key event adapter。
- [x] 实现 text input event adapter。
- [x] 预留系统 IME composition event adapter。
- [x] 实现 XUI 未消费事件返回给游戏逻辑的路径。
- [x] 暴露 XUI 可用的 texture/draw/clip/text measure 能力。
- [x] 支持 XUI 在 app-mode 下触发手动刷新。
- [x] 添加 XGE + XUI bridge 示例。

## 24. XUI 协作任务（历史）

> 本节记录历史独立 XUI 仓库协作任务。2026-05-07 起，当前实现和设计源以 XGE 仓库内 Widget V2 文档为准；旧 XUI 仓库任务不再作为当前验收口径。

- [x] 在 XUI 仓库固化 core/backend 分层文档。
- [x] 在 XUI 仓库生成中文 spec。
- [x] 在 XUI 仓库削减第一版布局范围。
- [x] 在 XUI 仓库确认第一版只依靠系统 IME，不实现完整内建 IME。
- [x] 在 XUI 仓库定义 backend 接口。
- [x] 在 XUI 仓库生成技术实施计划。
- [x] 在 XUI 仓库定义 XGE backend。
- [x] 在 XUI 仓库创建最小源码骨架。
- [x] 在 XUI 仓库接入 xrt 单头文件版本。
- [x] 在 XUI 仓库添加纯 C 测试框架。
- [x] 在 XUI 仓库实现最小事件队列。
- [x] 在 XUI 仓库实现最小 widget tree。
- [x] 在 XUI 仓库实现最小 absolute/row/column/stack 布局。
- [x] 在 XUI 仓库实现最小 paint queue。
- [x] 在 XUI 仓库实现 widget hit test。
- [x] 在 XUI 仓库实现最小 widget event dispatch。
- [x] 在 XUI 仓库实现 pointer capture。
- [x] 在 XUI 仓库实现 panel/label/button 控件基线。
- [x] 在 XUI 仓库实现 icon button 控件基线。
- [x] 在 XUI 仓库实现 progress/toggle 控件基线。
- [x] 在 XUI 仓库实现 slider 控件基线。
- [x] 在 XUI 仓库实现 scroll view 控件基线。
- [x] 在 XUI 仓库实现 list view 控件基线。
- [x] 在 XUI 仓库实现 input 控件基线。
- [x] 在 XUI 仓库实现 image 控件基线。
- [x] 在 XUI 仓库实现 dialog 控件基线。
- [x] 在 XUI 仓库实现 checkbox 控件基线。
- [x] 在 XUI 仓库实现 radio 控件基线。
- [x] 在 XUI 仓库实现 select 控件基线。
- [x] 在 XUI 仓库实现 tabs 控件基线。
- [x] 在 XUI 仓库实现 splitter 控件基线。
- [x] 在 XUI 仓库实现 collapsible 控件基线。
- [x] 在 XUI 仓库实现 stepper 控件基线。
- [x] 在 XUI 仓库实现 separator 控件基线。
- [x] 在 XUI 仓库补完 backend 生命周期、texture、text、clip 基线接口。
- [x] 在 XUI 仓库添加 backend mock 测试。
- [x] 在 XUI 仓库添加 XGE backend 文件与最小事件桥接测试。
- [x] 在 XUI 仓库添加 hello XUI 示例。
- [x] 在 XUI 仓库添加布局示例。
- [x] 在 XUI 仓库添加控件示例。
- [x] 在 XUI 仓库添加 input/text 示例。
- [x] 在 XUI 仓库添加 scroll/list 示例。
- [x] 在 XUI 仓库添加 XGE backend 示例。
- [x] 在 XUI 仓库预留软键盘 backend 接口。
- [x] 在 XUI 仓库添加 system IME 示例。
- [x] 在 XUI 仓库添加 XGE backend 人机冒烟测试流程。
- [x] 在 XUI 仓库完成 XGE 孵化代码迁移审计。
- [x] 将 XGE 侧 XUI 回归测试意图迁移或复用到 XUI 仓库测试与示例。
- [x] 在 XUI 仓库实现最小 label/button/image/input/scroll 示例。

## 25. 小程序后端

- [x] 定义小程序 C-facing platform hooks。
- [x] 定义 JS bridge API。
- [x] 添加 WASM 友好的扁平小程序调用入口，避免 JS 直接打包 C 结构体。
- [x] 实现 WebGL2 Canvas context creation。
- [x] 实现 touch event bridge。
- [x] 在平台支持时实现 keyboard/text event bridge。
- [x] 实现 audio bridge 策略或 miniaudio 可行性检查。
- [x] 通过小程序 API 实现 resource loading。
- [x] 实现 game loop bridge。
- [x] 添加小程序构建脚本。
- [x] 添加小程序 hello 示例。

## 26. 离屏与板卡 Linux

- [x] 实现 offscreen EGL init。
- [x] 实现 offscreen render target creation。
- [x] 实现 offscreen readback。
- [x] 实现 board Linux EGL init。
- [x] 添加 board Linux EGL pbuffer smoke 示例。
- [x] 添加 board Linux EGL 构建脚本。
- [x] 添加 DRM/KMS/GBM 设计说明。
- [x] 添加 board Linux input strategy 说明。
- [x] 添加 offscreen 示例。
- [x] 添加 board Linux smoke test plan。

## 27. Debug 与诊断

- [x] 定义 `xge_debug_stats_t`。
- [x] 实现 `xgeDebugDumpCaps`。
- [x] 实现 `xgeDebugGetStats`。
- [x] 在 debug 模式实现 GL error checks。
- [x] 实现 frame time stats。
- [x] 实现 draw call stats。
- [x] 实现 batch stats。
- [x] 实现 texture memory stats。
- [x] 实现 resource count stats。
- [x] 编写日志接入设计说明。
- [x] 在 xrt logger 可用后集成日志。
- [x] 添加 debug overlay 示例。

## 28. 示例

- [x] Hello window。
- [x] Draw shape。
- [x] Draw texture。
- [x] Sprite batch。
- [x] Draw text。
- [x] Input keyboard/mouse。
- [x] Touch。
- [x] Gamepad。
- [x] Scene stack。
- [x] Render target。
- [x] Shader/material。
- [x] 2.5D perspective quad。
- [x] Audio sound/music/3D。
- [x] Async resource fallback。
- [x] XUI incubation basic。
- [x] XUI bridge basic。
- [x] Offscreen rendering。
- [x] Mini program hello。
- [x] Debug overlay。

## 29. 文档

- [x] 持续更新 `XGE_V2_DESIGN.md`。
- [x] 开发过程中持续更新本 spec。
- [x] 编写构建脚本文档。
- [x] 编写单头文件使用文档。
- [x] 编写 API 命名规则文档。
- [x] 编写平台支持矩阵文档。
- [x] 编写资源加载文档。
- [x] 编写渲染管线文档。
- [x] 编写场景系统文档。
- [x] 编写布局系统文档。
- [x] 编写 XRF 格式文档。
- [x] 编写小程序后端文档。
- [x] 编写人工冒烟测试流程文档。
- [x] 编写专用渲染线程设计文档。
- [x] 编写日志接入设计文档。
- [x] 编写 Sokol 跨平台冒烟测试计划。
