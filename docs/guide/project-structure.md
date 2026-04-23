# 项目结构入门

本教程说明使用 XGE 开发项目时推荐的目录组织、资源路径和构建脚本约定。

[返回教程索引](README.md) | [第一个程序](first-xge-program.md) | [资源教程](resource-intro.md)

## 推荐结构

```text
my_game/
  examples\mvp\build.bat
  examples/mvp/build.sh
  build_dll.bat
  build_dll.sh
  xge.h
  xge.c
  src/
    main.c
    game_scene.c
    game_scene.h
  assets/
    image/
    audio/
    font/
  test/
    smoke.c
  lib/
    xrt/
    sokol/
    miniaudio/
    stb/
```

应用项目可以直接使用 XGE 的单头文件交付形态，也可以在引擎仓库中按 `src/` 分模块开发，再由生成器组装单头文件。

## 引擎仓库结构

XGE 自身使用可读性优先的分文件结构：

```text
xge/
  xge.h
  xge.c
  src/
  test/
  lib/
  singlehead/
  docs/
  dev/
```

`xge.h` 只保留公开 API、类型和常量声明。实现代码按模块放在 `src/`，单头文件由工具生成。

## 资源根目录

默认相对路径以 exe 所在目录为根：

```c
xgeTextureLoad(&texture, "assets/image/player.png");
xgeSoundLoad(&click, "assets/audio/click.wav");
```

`res://` 用于资源 provider，例如压缩包、内存资源、网络缓存或小程序宿主预加载资源。

```c
xgeTextureLoad(&texture, "res://ui/button.png");
```

## 构建脚本命名

脚本名带目标：

```text
examples\mvp\build.bat
examples/mvp/build.sh
build_dll.bat
build_dll.sh
build_test.bat
build_test.sh
```

脚本只负责测试和编译，不引入大型构建系统作为第一依赖。

## 单头文件集成

应用侧使用：

```c
#include "xge.h"
```

在一个 C 文件中启用实现：

```c
#define XGE_IMPLEMENTATION
#include "xge.h"
```

如果项目使用 `xge.c`，则不要再在其他文件定义 `XGE_IMPLEMENTATION`。

## 常见错误

不要把引擎开发结构和交付结构混在一起。仓库内部要分模块，交付给外部集成时可以是单头文件。

不要依赖当前工作目录加载资源。运行脚本、IDE 和双击 exe 的 cwd 可能不同，应以 exe path 为根。

不要把 xpack 当成 XGE 必需模块。XGE 只定义资源 provider，xpack 是可选 provider 之一。

## 下一步

- 继续读 [资源加载入门](resource-intro.md)。
- 需要调试运行状态读 [错误、日志与调试信息](error-and-log.md)。
