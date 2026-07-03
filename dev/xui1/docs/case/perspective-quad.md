# 2.5D 透视 quad

本案例展示使用 3D 顶点绘制一个带透视的 textured quad，用于卡牌翻转、地面投影、伪 3D 面片和图像扭曲。

[返回范例解析](README.md) | [Drawing API](../api/drawing.md) | [Material API](../api/material.md)

## 问题

XGE 是 2D 引擎，但允许高级 API 使用 3D 顶点。这个能力不把 XGE 变成 3D 引擎，只用于少量 2.5D 效果。

典型用途：

- 卡牌、菜单面板、地图格子的透视倾斜。
- 角色影子、地面贴花、伪 3D 背景层。
- 屏幕空间图像扭曲。
- 需要 z 排序的少量 quad。

## 步骤 1：设置透视相机

```c
xge_camera_t camera;

camera = xgeCameraPerspective((float)xgeGetWidth(), (float)xgeGetHeight(), 45.0f, 0.1f, 100.0f);
xgeCameraSet(&camera);
xgeDepthTestSet(1);
```

默认 2D 坐标仍然是 y 向下。进入 2.5D 绘制前应明确相机和 depth test，绘制完再恢复需要的状态。

## 步骤 2：准备四个顶点

```c
xge_vertex_t v[4];

memset(v, 0, sizeof(v));
v[0].fX = -1.0f; v[0].fY = -1.0f; v[0].fZ = -4.0f; v[0].fU = 0.0f; v[0].fV = 1.0f;
v[1].fX =  1.0f; v[1].fY = -1.0f; v[1].fZ = -4.0f; v[1].fU = 1.0f; v[1].fV = 1.0f;
v[2].fX =  1.2f; v[2].fY =  1.0f; v[2].fZ = -5.0f; v[2].fU = 1.0f; v[2].fV = 0.0f;
v[3].fX = -1.2f; v[3].fY =  1.0f; v[3].fZ = -5.0f; v[3].fU = 0.0f; v[3].fV = 0.0f;

v[0].iColor = xgeColorRGBA(255, 255, 255, 255);
v[1].iColor = v[0].iColor;
v[2].iColor = v[0].iColor;
v[3].iColor = v[0].iColor;
```

## 步骤 3：绘制 quad

```c
xgeDrawQuad3D(&g_texture, v, 0);
```

如果需要复用几何体，使用 `xgeMeshCreate` 和 `xgeMeshDraw`，避免每帧重复提交大量顶点。

## 步骤 4：恢复 2D 状态

```c
xgeDepthTestSet(0);
xgeCameraSet(NULL);
```

建议把 2.5D 绘制集中在一个明确阶段，避免影响普通 UI 或 sprite 批处理。

## 关键 API

| API | 作用 |
| --- | --- |
| `xgeCameraPerspective` | 创建透视相机 |
| `xgeCameraSet` | 设置当前相机 |
| `xgeDepthTestSet` | 开关深度测试 |
| `xgeDrawQuad3D` | 绘制 3D quad |
| `xgeMeshCreate` | 创建可复用 mesh |
| `xgeMeshDraw` | 绘制 mesh |

## 常见问题

如果 quad 完全消失，检查 z 值是否落在 near/far 之间，以及顶点绕序是否被剔除。

如果 2D UI 被深度挡住，确认 UI 绘制前已经关闭 depth test，并恢复默认 2D camera。
