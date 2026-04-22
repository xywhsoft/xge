# 板卡 Linux EGL

本案例说明现代板卡 Linux 的 EGL/GLES3 验证方法。

[返回范例解析](README.md) | [板卡教程](../guide/board-linux-intro.md) | [离屏范例](offscreen-egl.md)

## 状态

板卡支持必须实机验证。该页面只定义目标路径和记录标准，不把未验证板卡写成已支持。

## 目标板卡

第一阶段面向近年板卡，例如 RK3566 级别设备，要求 GLES3/EGL 能力。

## 验证步骤

1. 查询 EGL/GLES 版本。
2. 创建 pbuffer 或 GBM/KMS surface。
3. 执行 clear + shape 绘制。
4. 如无显示输出，使用 readback 验证像素。
5. 验证触摸/键盘/鼠标输入来源。
6. 记录 GPU vendor、renderer、driver 和内核版本。

## 记录格式

```text
板卡:
系统:
GPU:
EGL:
GLES:
后端:
结果:
问题:
```

## 常见失败原因

镜像缺少用户态 GPU 驱动。

DRM 设备权限不足。

默认启动了不兼容的显示服务，占用了 KMS。
