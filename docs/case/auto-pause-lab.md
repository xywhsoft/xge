# Auto Pause Lab

`examples/auto_pause_lab` 是一个专门演示 scene `pause/resume` 语义的小范例。它参考了 V1 的 `206.Auto Pause`，但在 V2 里改成更稳定的 scene stack 版本：主场景先运动，随后自动 push 一个暂停场景，冻结底层状态并显示锁定遮罩，再自动 pop 回主场景继续运动。

## 覆盖 API

- `xgeSceneSet`
- `xgeScenePush`
- `xgeScenePop`
- `xgeSceneCurrent`
- `xgeSceneCount`
- `xge_scene_t.onPause`
- `xge_scene_t.onResume`

## 构建和运行

```bat
examples\auto_pause_lab\build.bat
build\xge_auto_pause_lab.exe --frames 5
```

## 验证点

- 控制台会输出 `auto-pause-lab final-summary ...`
- `flow=1 pause=1 resume=1 pop=1` 表示主场景进入暂停、恢复和叠层退出都成功。
- `freeze=1` 表示暂停阶段底层主场景没有继续 update，运动物体位置保持不变。
- `overlay=1` 表示暂停遮罩和锁定面板已经走到绘制路径。
- `stack=1` 表示 push 后 scene count 为 2、pop 后回到 1，且 current scene 切换正确。
- `game(... x=pause/snapshot/resume ...)` 中第三个位置值应大于前两个，说明恢复后运动重新继续。

## 人工观察建议

- 主场景里会有一个沿轨道移动的黄色圆点。
- 暂停时画面会转成灰暗版本，并出现中间锁定面板。
- 恢复后圆点会从暂停前的位置继续向右移动，而不是跳回初始位置。
