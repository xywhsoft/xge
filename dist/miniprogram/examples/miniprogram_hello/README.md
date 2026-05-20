# XGE 小程序 Hello 示例

本目录是 XGE 小程序后端的最小 hello 示例。它提供小程序工程侧的启动代码、资源加载策略和浏览器调试入口，用于后续接入真实 `xge.js` / `xge.wasm` 后做人机冒烟测试。

当前可用部分：

- C-facing 小程序 hooks 已在 `xge.h` 中定义。
- JS bridge 位于 `platform/miniprogram/xge_miniprogram.js`。
- `game.js` 提供小程序侧最小启动流程。
- `browser_debug.html` 可在浏览器中检查 JS bridge 接入路径。
- `examples\miniprogram_hello\build.bat` / `.sh` 会复制 bridge 和 hello 示例到 `dist/miniprogram`。

## 目录内容

```text
examples/miniprogram_hello/
  README.md
  game.js
  game.json
  project.config.json
  browser_debug.html
```

## 小程序接入形态

```js
const { XgeMiniProgramBridge } = require("../../platform/miniprogram/xge_miniprogram.js");

const bridge = new XgeMiniProgramBridge(Module, {
	canvas,
	resourceLoader(uri) {
		// Platform-specific file/package/cache loading goes here.
		throw new Error("resource loader is not wired yet: " + uri);
	}
});

bridge.init(canvas.width, canvas.height, devicePixelRatio || 1);
bridge.attachTouchEvents(canvas);
bridge.start();
```

`game.js` 默认会尝试加载同目录的 `xge.js`。后续 WASM 构建完成后，构建脚本应把 `xge.js` 和 `xge.wasm` 放入示例输出目录。

## 资源加载

示例内置 `res://` 资源加载策略：

- `res://foo.png` 映射到示例目录下的 `assets/foo.png`。
- 微信小游戏环境优先使用 `wx.getFileSystemManager().readFileSync`。
- 浏览器调试入口使用 `fetch` 加载资源。

## 仍未完成

- `xge.js` / `xge.wasm` 真实构建输出。
- WASM 侧资源与音频回调注册。
- 目标小程序运行时中的 WebGL2 draw call 人工验证。
- 将验证结果回填到 spec。
