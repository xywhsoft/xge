# 小程序 Hello

本案例展示小程序环境中最小 XGE 接入：初始化、触控输入、帧推进和请求下一帧。

[返回范例解析](README.md) | [小程序教程](../guide/miniprogram-intro.md) | [Miniprogram API](../api/miniprogram.md)

## 问题

小程序没有传统 native main loop。XGE 需要由宿主调用初始化和每帧函数，并通过桥接层把触控、文本、音频命令转发给宿主。

## 步骤 1：初始化

```c
int XgeMiniInit(int iWidth, int iHeight, float fDpr)
{
	xge_miniprogram_bridge_t bridge;

	memset(&bridge, 0, sizeof(bridge));
	bridge.procRequestFrame = HostRequestFrame;
	bridge.procAudioCommand = HostAudioCommand;

	if ( xgeMiniProgramInitSimple(iWidth, iHeight, fDpr) != XGE_OK ) {
		return 0;
	}

	xgeMiniProgramSetBridge(&bridge);
	return 1;
}
```

## 步骤 2：帧函数

```c
int XgeMiniFrame(double fTimeSeconds)
{
	xgeClear(xgeColorRGBA(20, 24, 32, 255));
	xgeShapeCircleFill(160.0f, 160.0f, 48.0f, xgeColorRGBA(80, 180, 120, 255));
	return xgeMiniProgramFrame(fTimeSeconds);
}
```

实际实现中可以把绘制放进 XGE frame callback；这里展示的是宿主驱动帧推进的核心关系。

## 步骤 3：转发触控

```c
void XgeMiniTouchBegin(int iId, float fX, float fY, float fForce)
{
	xgeMiniProgramTouchOne(XGE_TOUCH_BEGAN, iId, fX, fY, fForce);
}

void XgeMiniTouchMove(int iId, float fX, float fY, float fForce)
{
	xgeMiniProgramTouchOne(XGE_TOUCH_MOVED, iId, fX, fY, fForce);
}

void XgeMiniTouchEnd(int iId, float fX, float fY)
{
	xgeMiniProgramTouchOne(XGE_TOUCH_ENDED, iId, fX, fY, 0.0f);
}
```

## 步骤 4：尺寸变化

```c
void XgeMiniResize(int iWidth, int iHeight, float fDpr)
{
	xgeMiniProgramResize(iWidth, iHeight, fDpr);
}
```

## 关键 API

| API | 作用 |
| --- | --- |
| `xgeMiniProgramInitSimple` | 初始化小程序 runtime |
| `xgeMiniProgramSetBridge` | 设置宿主桥接 |
| `xgeMiniProgramFrame` | 推进一帧 |
| `xgeMiniProgramResize` | 更新 canvas 尺寸 |
| `xgeMiniProgramTouchOne` | 转发单个触控点 |
| `xgeMiniProgramRequestFrame` | 请求宿主调度下一帧 |

## 常见问题

如果只有第一帧，检查宿主是否响应了 `procRequestFrame` 并继续调用 `xgeMiniProgramFrame`。

如果触控坐标不准，确认传入的是逻辑 canvas 坐标，并且 DPR 与初始化一致。
