# XGE 的 XRT 快照

`xrt.h` 是从 XRT 源码快照生成的单头文件，`xrt_config.h` 是 XGE 自己维护的裁剪配置。不要手工修改生成头；上游修复应进入 XRT，再重新同步。

本次来源为 `D:\GIT\xrt`，基础提交 `9bfc3c0f82e91dacc93d654f27fc72a21f1e784f`。同步时该仓库有未提交改动，因此这是**工作区源码快照，不是该提交的纯净发布包**。精确快照/生成头 SHA-256 记录在 [upstream.json](upstream.json)。

在 XGE 根目录执行：

```powershell
python tools/sync_xrt.py D:\GIT\xrt
```

工具把 include/src/tools/config/LICENSE 复制到隔离临时目录，核对源文件是否在复制时变化，调用上游 amalgamate 和 `--check`，然后更新本目录的头文件及记录。**不修改上游源码、single 目录或未提交改动**。不直接复制可能过期的上游 `single/xrt.h`；XGE 的裁剪配置也不会被上游配置覆盖。

粒子系统复用 RANDOM、ARRAY、POOL、SLOT_MAP、MAP、JSON、XSON、VALUE、原子引用和内存/计时设施。RANDOM 与 SLOT_MAP 现纳入 XGE 默认配置，基础噪声由粒子模块实现，不需要打开大型 MAPGEN 可选模块。

此版上游将正则模块合并到 XRT 自身：裁剪宏使用 `XRT_MODULE_REGEX_CORE` / `XRT_MODULE_REGEX_MATCH`，实现入口使用 `XRT_IMPLEMENTATION`，不再是 `XREGEX_*`。XGE 本体、XUI 的独立 XRT 测试实现及 deflate 基准入口只做了相应宏迁移，不改变其功能逻辑。发布/集成时应一并重编译 DLL 和使用该头文件的程序。

同步后的最小复验为 `build_dll.bat`、`build_test.bat` 和 `test/build_particle_test.bat`。若上游继续变化，重新同步会得到新的快照，应重新跑这些测试，而不是沿用旧验证结果。

已知上游问题：当前裁剪配置下，直接用 C++ 编译器包含 `xge.h` 会遇到部分可选网络头的 `XRT_EXTERN_C_BEGIN/END` 条件不配对。本次对同步前 HEAD 的旧头文件做了同样检查，也同样失败，并非粒子 API 新增导致。C 编译、DLL 和运行测试通过；此处没有修改上游源码或给生成头塞入临时修补。需要 C++ 集成时应先在 XRT 修正该条件编译问题后重新同步。
