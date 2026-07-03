# 清屏窗口

本案例展示最小清屏程序，用来确认窗口、GL context、frame callback 和 present 链路可用。

[返回范例解析](README.md) | [第一个程序](../guide/first-xge-program.md) | [Graphics API](../api/graphics.md)

## 问题

在接入新平台或新后端时，先不要急着加载资源。只清屏可以最快判断窗口和图形上下文是否正确创建。

## 最小帧函数

```c
static int MainFrame(void* pUser)
{
	(void)pUser;
	xgeClear(xgeColorRGBA(24, 32, 48, 255));
	return 0;
}
```

## 启动

```c
xge_desc_t desc;

memset(&desc, 0, sizeof(desc));
desc.iWidth = 800;
desc.iHeight = 600;
desc.sTitle = "XGE Clear";

if ( xgeInit(&desc) != XGE_OK ) {
	return 1;
}

xgeRun(MainFrame, NULL);
xgeUnit();
```

## 可观察结果

窗口应显示稳定的深色背景，关闭窗口后进程退出。

## 常见失败原因

窗口不出现：平台后端初始化失败。

窗口黑屏但 clear 颜色不对：GL context 未 current，或 clear/present 链路未执行。

关闭无响应：quit 事件没有进入 runtime，或 frame callback 阻塞。
