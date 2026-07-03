# Viewport、Clip 与 Camera

本案例展示如何组合 viewport、clip rect、正交/透视 camera，以及 world/screen 坐标转换。它延续旧版 XGE 中 SetView、SetCoord 一类示例的学习目标。

[返回范例解析](README.md) | [Graphics API](../api/graphics.md)

## 问题

你希望把画面分成多个区域，在每个区域内使用不同的相机和裁剪规则，并确认 resize 后坐标换算仍然合理。

## 运行方式

```bat
examples\viewport_clip_camera\build.bat
build\xge_viewport_clip_camera.exe --frames 180
```

Unix-like 平台：

```sh
./examples/viewport_clip_camera/build.sh
./build/xge_viewport_clip_camera --frames 180
```

## 覆盖 API

| API | 用途 |
| --- | --- |
| `xgeViewportSet` / `xgeViewportGet` / `xgeViewportClear` | 设置、读取和清除渲染 viewport |
| `xgeClipSet` / `xgeClipGet` / `xgeClipClear` | 设置、读取和清除 scissor clip |
| `xgeCameraDefault` | 创建默认正交 camera |
| `xgeCameraPerspective` | 创建透视 camera |
| `xgeCameraSet` / `xgeCameraGet` | 切换并读取当前 camera |
| `xgeWorldToScreen` / `xgeScreenToWorld` | 坐标双向转换 |

## 通过标准

- 构建成功并生成 `build\xge_viewport_clip_camera.exe`。
- 左侧 viewport 中能看到 world-space 网格、坐标轴和相机移动效果。
- 左侧右上角能看到一个小的透视 camera 区域。
- 右侧 viewport 中移动图形被白色 clip 矩形裁剪。
- 终端打印 `first`、`frame` 或 `final-summary`，其中 `world` 与 `roundtrip` 坐标应基本一致。
- 达到 `--frames` 或 `--seconds` 限制后自动退出；按 `ESC` 可提前退出。

## 可跳过条件

- 当前环境无法创建窗口或 GL context 时，可以记录构建通过，运行验证标记为跳过。
- 低精度平台上 world/screen 往返存在极小浮点误差不视为失败。
