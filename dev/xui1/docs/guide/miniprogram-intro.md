# 小程序接入入门

本教程介绍小程序环境下 XGE 的接入方式：由宿主提供 canvas、触控、文本和音频桥接，XGE 负责帧推进和渲染命令。

[返回教程索引](README.md) | [Miniprogram API](../api/miniprogram.md) | [小程序范例](../case/miniprogram-hello.md)

## 接入模型

小程序不是传统桌面窗口模型。XGE 不直接创建系统窗口，而是通过桥接层接收宿主事件。

```text
小程序 JS
  -> canvas/context
  -> touch/text/audio bridge
  -> xgeMiniProgramFrame
XGE
  -> update/render
  -> request next frame
```

## 初始化

```c
if ( xgeMiniProgramInitSimple(750, 1334, 2.0f) != XGE_OK ) {
	return XGE_ERROR_BACKEND;
}
```

宽高使用逻辑 canvas 尺寸，`fDevicePixelRatio` 用于映射 framebuffer 和输入坐标。

## 设置桥接

```c
xge_miniprogram_bridge_t bridge;

memset(&bridge, 0, sizeof(bridge));
bridge.procRequestFrame = RequestFrameFromHost;
bridge.procAudioCommand = SendAudioCommandToHost;
xgeMiniProgramSetBridge(&bridge);
```

桥接函数应保持薄层，不要在回调里做大量业务逻辑。

## 帧推进

```c
int MiniProgramTick(double fTimeSeconds)
{
	return xgeMiniProgramFrame(fTimeSeconds);
}
```

小程序的帧调度由宿主控制。XGE 可以通过 `xgeMiniProgramRequestFrame` 请求下一帧，但最终是否执行由平台决定。

## 触控输入

```c
xgeMiniProgramTouchOne(XGE_TOUCH_BEGAN, iTouchId, fX, fY, fForce);
xgeMiniProgramTouchOne(XGE_TOUCH_MOVED, iTouchId, fX, fY, fForce);
xgeMiniProgramTouchOne(XGE_TOUCH_ENDED, iTouchId, fX, fY, 0.0f);
```

多点触控用稳定 touch id 表示同一根手指的生命周期。

## 常见错误

不要假设小程序有完整 POSIX 文件系统。资源加载应通过普通路径、`res://` provider 或宿主预加载内存。

不要假设音频命令立即生效。小程序音频通常需要走宿主异步接口。

## 下一步

- 看完整例子用 [小程序 Hello 范例](../case/miniprogram-hello.md)。
- 资源接入读 [资源加载入门](resource-intro.md)。
