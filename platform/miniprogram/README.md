# 小程序宿主桥接

用 Emscripten 构建单线程 wasm32 时为 XGE 定义 `XGE_ENABLE_MINIPROGRAM_JS=1`，并在链接时传入 `--js-library platform/miniprogram/xge_miniprogram_library.js`。将下列函数保留在 `EXPORTED_FUNCTIONS`，连同程序自己的入口一起导出。

```text
_malloc, _free
_xgeMiniProgramJsInstall, _xgeMiniProgramJsUnit, _xgeMiniProgramJsResourceReserve
_xgeMiniProgramFrame, _xgeMiniProgramResize, _xgeMiniProgramTouchOne
_xgeMiniProgramText, _xgeMiniProgramAudioCommand
```

WASM 模块就绪后，用 `xge_miniprogram.js` 中的 `XgeMiniProgramBridge` 将模块与宿主 canvas 关联。`init(width, height, dpr)` 注册帧调度和可选的资源、音频回调。宿主完成图形初始化后调用 `start()` 调度帧。`stop()` 暂停调度，`unit()` 释放本次桥接注册。引擎关闭也会释放注册。普通 Emscripten 构建可省略该编译定义和 JavaScript 库。

资源回调同步接收完整的 `res://` URI，并返回 `ArrayBuffer` 或 `Uint8Array`。宿主可提前读入异步资源，回调从已准备的数据返回字节。数据复制到由 XGE 接管的输出，重新注册或关闭后仍可使用已加载的原生资源。

`audioCommand(command, handle, data)` 接受 `ArrayBuffer`、`Uint8Array` 或按 UTF-8 编码的字符串。音频处理函数同步接收 WASM 内存上的 `Uint8Array`，返回一个 32 位整数状态。需要长期保存，或随后调用可能扩充 WASM 内存的操作时，先复制这段字节。

以下命令在源仓库根目录构建并运行无窗回归。它链接真实 XGE 核心版，使用 Node.js 驱动帧调度，canvas 由测试提供。微信客户端的图形与音频设备接入需要另行验收。

```sh
emcc -O2 @config/includes.rsp -DXGE_BUILD_PROFILE=1 -DXGE_DEBUGMODE=0 \
  -DXGE_ENABLE_MINIPROGRAM_JS=1 -DXRT_FEATURE_MEMORY_DEBUG \
  single/xge.c tests/xge/miniprogram_wasm_probe.c --no-entry \
  --js-library platform/miniprogram/xge_miniprogram_library.js \
  -sMODULARIZE=1 -sENVIRONMENT=node -sALLOW_MEMORY_GROWTH=1 -sABORTING_MALLOC=0 \
  -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sGL_ENABLE_GET_PROC_ADDRESS=1 \
  -sEXPORTED_FUNCTIONS=_malloc,_free,_xgeMiniProgramJsInstall,_xgeMiniProgramJsUnit,_xgeMiniProgramJsResourceReserve,_xgeMiniProgramRequestFrame,_xgeMiniProgramFrame,_xgeMiniProgramResize,_xgeMiniProgramTouchOne,_xgeMiniProgramText,_xgeMiniProgramAudioCommand,_xgeGetWidth,_testEngineInit,_testEngineUnit,_testMemoryFail,_testResourceLoad,_testResourceSize,_testResourceByte \
  -sEXPORTED_RUNTIME_METHODS=ccall -o /tmp/xge-mini-probe.cjs
node tests/xge/miniprogram_wasm_probe.cjs /tmp/xge-mini-probe.cjs
```
