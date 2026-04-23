# Async Assets Lab

`examples/async_assets_lab` 覆盖 async request 生命周期、threading 开关、poll、image/texture/font/sound 四类异步加载，以及取消请求路径。范例启动时会在 `build/` 下生成一张临时 TGA 和一份临时 WAV，再用这些文件发起异步请求。

## 覆盖 API

- `xgeAsyncRequestInit`
- `xgeAsyncRequestFree`
- `xgeAsyncRequestCancel`
- `xgeAsyncThreadingSet`
- `xgeAsyncThreadingGet`
- `xgeAsyncPoll`
- `xgeAsyncImageLoad`
- `xgeAsyncTextureLoad`
- `xgeAsyncFontLoad`
- `xgeAsyncSoundLoad`

## 构建和运行

```bat
examples\async_assets_lab\build.bat
build\xge_async_assets_lab.exe --frames 120
```

## 验证点

- 控制台输出 `async-assets-lab init`。
- `threading=1/1` 表示 threaded off/on 两次切换和读取都成功。
- `modes=1/1` 表示至少有一个请求走主线程完成、且 texture/font/sound 走了 threaded 路径。
- `loads=1/1/1/1 poll=1` 表示四类异步加载和 poll 都进入成功路径。
- `cancel=1` 且最后一个 `status` 为 `4`，表示取消请求进入 `XGE_ASYNC_CANCELLED`。
- 右侧预览区域能看到异步加载成功后的纹理；字体加载成功后会显示 `Async Font Ready`。

## 可跳过条件

- 如果当前机器没有可用音频后端，`sound` 可能进入 failed，但示例仍会输出 summary 并继续验证其余 async 路径。
