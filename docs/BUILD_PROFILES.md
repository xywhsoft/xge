# 可裁剪构建与增量编译

Windows 使用原有入口；Linux 使用 `./build_dll.sh <profile>`，Linux 路径尚未实机验证。构建脚本需要 Python 3 和 GCC 工具链，既存可选 HarfBuzz 需要 G++。

```bat
build_dll.bat 3d
build_dll.bat ui-min
python tools/build_profile.py ui-min --define XUI_ENABLE_IMAGE=0
```

| 配置 | 能力 | 输出 |
| --- | --- | --- |
| default（省略参数） | 保留旧默认能力，3D 关闭 | build/ |
| core | 窗口、输入、资源和 GPU 底座 | build/core/ |
| 2d | 基础二维，关闭文本和高级绘图 | build/2d/ |
| 3d | 三维及底座，关闭二维、文本、音频和 XUI | build/3d/ |
| ui-min | 二维、文本、XUI 核心与 Panel、Label、Button、Image | build/ui-min/ |
| full-dev | 所有已实现功能，包含三维 | build/full-dev/ |
| ide | 保留原有 IDE 的 xrt 扩展配置 | build/ide/ |

`xge_config.h` 定义 XGE 与 3D 模块，`xui_config.h` 定义 65 个控件及服务开关。开关值必须为数字 0 或 1。默认关闭顶层模块时，其子模块随之关闭；显式冲突由编译器报错。控件有真实依赖，例如关闭 ListView 时须同时关闭使用它的控件。错误信息列出缺少的开关；依赖清单见 `tools/features.json`。不要用 `#ifdef` 判断数字功能宏。

库和消费者必须使用相同配置。构建产物包含完整的 `xge_build_config.h`，编译应用时强制包含它，使用该目录的导入库并从同一目录运行，以避免加载其他配置的 DLL：

3D 子模块可用 `XGE3D_ENABLE_MODEL/LIGHTING/SHADOW/IBL/ANIMATION/TERRAIN/ASYNC/FOG` 单独关闭。异步依赖 MODEL，动画依赖 MODEL，阴影依赖 LIGHTING，IBL 依赖 LIGHTING；FOG 仅依赖 3D，可用于无光照配置。显式冲突会报错。配置位掩码表示顶层模块，子模块列表与源码裁剪结果见 build-report.json。

```bat
gcc -O2 -DXGE_DLL -include build/3d/xge_build_config.h application.c build/3d/xge.lib -lm -o build/3d/application.exe
```

`xgeGetBuildFeatures()` 返回 `XGE_FEATURES` 位掩码，可检查库与消费者是否一致；它表示编译配置，具体能力以当前公开 API 和使用说明为准。旧 `XGE_NO_TEXT`、`XGE_NO_AUDIO` 继续可用，不能与相反的正向宏同时设置。

源码、声明、导出、内置资源和第三方依赖按配置选择。无文本配置跳过 HarfBuzz；`ui-min` 不加入 Document、CodeEdit、WebView 等模块。配置隔离对象和链接产物；相同配置下缓存键包含编译器、参数以及编译器生成的包含依赖。新增三维文件独立编译；HarfBuzz 使用共享对象缓存。`build-report.json` 记录复用、编译、链接和大小，DLL 保存去符号版本，另有 `xge.unstripped.dll` 供诊断。

验证入口：

```bat
test\build_feature_profiles.bat
python test/check_control_trimming.py
```

第二条使用当前 `full-dev` 对象，逐项关闭控件及其依赖后实际链接 DLL，检查声明与导出。修改对应源码后应先重新构建 `full-dev`；已有矩阵产物的快速复核可用 `--verify-existing`，它拒绝比输入对象更旧的 DLL。
