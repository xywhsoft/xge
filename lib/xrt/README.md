# XGE 的 XRT 快照

`xrt.h` 是从 XRT 源码快照生成的单头文件，`xrt_config.h` 是 XGE 自己维护的裁剪配置。不要手工修改生成头；上游修复应进入 XRT，再重新同步。

来源为 XRT 主线工作区。当前提交、分支、未提交状态及精确快照/生成头 SHA-256
记录在 [upstream.json](upstream.json)。存在未提交改动时，这是**工作区源码快照，
不是该提交的纯净发布包**。

在 XGE 根目录执行：

```powershell
python tools/sync_xrt.py D:\GIT\xrt
```

工具把 include/src/tools/config/LICENSE 复制到隔离临时目录，核对源文件是否在复制时变化，调用上游 amalgamate 和 `--check`，然后更新本目录的头文件及记录。**不修改上游源码、single 目录或未提交改动**。不直接复制可能过期的上游 `single/xrt.h`；XGE 的裁剪配置也不会被上游配置覆盖。

同步还生成 `xrt_decl.h`，供匹配的 SDK 作为声明头发布。它与 `xrt.h` 来自同一次
生成，不应从另一个版本拼接。`XGE_XRT_PROFILE_IDE` 在默认模块之外启用 IDE / AI
所需的 HTTP/TLS、系统证书、进程、信号量、XID、临时文件及编解码；通过
`build_dll.bat ide` 构建，所有消费者使用同一配置。

粒子系统复用 RANDOM、ARRAY、POOL、SLOT_MAP、MAP、JSON、XSON、VALUE、原子引用和内存/计时设施。RANDOM 与 SLOT_MAP 现纳入 XGE 默认配置，基础噪声由粒子模块实现，不需要打开大型 MAPGEN 可选模块。

此版上游将正则模块合并到 XRT 自身：裁剪宏使用 `XRT_MODULE_REGEX_CORE` / `XRT_MODULE_REGEX_MATCH`，实现入口使用 `XRT_IMPLEMENTATION`，不再是 `XREGEX_*`。XGE 本体、XUI 的独立 XRT 测试实现及 deflate 基准入口只做了相应宏迁移，不改变其功能逻辑。发布/集成时应一并重编译 DLL 和使用该头文件的程序。

同步后的最小复验为 `build_dll.bat`、`build_test.bat` 和 `test/build_particle_test.bat`。若上游继续变化，重新同步会得到新的快照，应重新跑这些测试，而不是沿用旧验证结果。

历史验证中，直接用 C++ 编译器包含 `xge.h` 曾遇到可选网络头的
`XRT_EXTERN_C_BEGIN/END` 条件不配对。IDE 使用 C 工具链；本轮不据旧记录断言当前
C++ 兼容性，也不修改生成头规避问题。需要 C++ 集成时应针对所选快照重新验证，
若发现问题，应在 XRT 主线修复后重新同步。
