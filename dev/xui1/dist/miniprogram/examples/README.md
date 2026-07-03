# XGE Mini Program Hello

This directory is the tracking point for the first real mini program smoke test.

Available pieces:

- C-facing mini program hooks are available in `xge.h`.
- JS bridge is in `platform/miniprogram/xge_miniprogram.js`.
- `build_miniprogram.bat` / `.sh` copies the bridge scaffold into `dist/miniprogram`.

Minimal integration shape:

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

Still pending before this is a runnable hello:

- Real platform project files.
- `xge.js` / `xge.wasm` build output.
- WASM-side bridge callback registration for resource/audio commands.
- One verified WebGL2 draw call in a target mini program runtime.
