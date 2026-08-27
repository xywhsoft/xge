# XGE Web

此目录提供 XGE 的 Emscripten/WebGL2 HTML shell。它为 Sokol 提供稳定的 `#canvas`，并把标准输出和错误输出显示在页面日志面板中。完整的发布前验证要求见根目录 [平台说明](../../docs/PLATFORM.md)。

## 构建与运行

使用带 Web 构建脚本的范例生成产物：

```sh
./examples/platform_smoke/build_web.sh
```

Windows：

```bat
examples\platform_smoke\build_web.bat
```

使用本地 HTTP 服务打开输出：

```sh
./serve_web_exe.sh
```

Windows：

```bat
serve_web_exe.bat
```

然后访问 `http://localhost:8000/xge_web.html`。交付前应使用目标浏览器完成 WebGL2 启动、绘制和输入验证。
