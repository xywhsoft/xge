# WebGL2 / Emscripten 入门

本教程说明 XGE 在 Web 目标上的基本策略。

[返回教程索引](README.md) | [Platform API](../api/platform.md) | [平台冒烟范例](../case/platform-smoke.md)

## 图形基线

Web 目标使用 WebGL2，对应 GLES3 风格能力。业务 shader 应通过 XGE 图形映射获取后端头部，避免直接写死桌面 GL 语法。

```c
xge_graphics_mapping_t mapping;

xgeGraphicsMappingGet(XGE_GPU_BACKEND_WEBGL2, &mapping);
```

## 主循环

Web 环境由浏览器调度帧。应用应使用 XGE 的 game loop 模式或由 Emscripten 回调驱动，不要写阻塞式 while loop。

```c
desc.iRunMode = XGE_RUN_GAME_LOOP;
xgeInit(&desc);
xgeRun(MainFrame, NULL);
```

## 资源

Web 资源需要在打包阶段进入虚拟文件系统，或由宿主下载后通过内存/provider 注入。

```c
xgeResourceLoad("res://ui/button.png", &resource);
```

## 音频

浏览器通常要求用户手势后才能启动音频。第一次播放应放在点击、触摸等用户事件之后。

## 常见错误

不要在 Web 目标使用桌面文件路径假设。资源必须由构建或宿主提供。

不要依赖同步阻塞加载大文件。Web 资源应优先异步。

不要假设所有浏览器都允许立即播放音频。

## 下一步

- 小程序环境读 [小程序接入入门](miniprogram-intro.md)。
- 异步资源读 [异步资源加载入门](async-intro.md)。
