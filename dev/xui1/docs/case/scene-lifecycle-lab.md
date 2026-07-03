# Scene Lifecycle Lab

`examples/scene_lifecycle_lab` 用一个自动执行的小剧本覆盖 XGE scene 栈。流程是：设置 base scene，push overlay，pop overlay 回到 base，再 replace 到 replacement scene，最后清空场景栈并打印每个 lifecycle 回调计数。

## 覆盖 API

- `xgeSceneSet`
- `xgeScenePush`
- `xgeScenePop`
- `xgeSceneReplace`
- `xgeSceneCurrent`
- `xgeSceneCount`
- `xgeSceneDispatchEvent`
- `xgeSceneUpdateStrategySet`
- `xgeSceneUpdateStrategyGet`

## 构建和运行

```bat
examples\scene_lifecycle_lab\build.bat
build\xge_scene_lifecycle_lab.exe --frames 12
```

`--frames` 是安全上限。正常情况下范例会在完成生命周期剧本后自动退出。

## 验证点

- 控制台输出三行 `scene-lifecycle-lab counts`，分别对应 base、overlay、replacement。
- base 应出现 enter、pause、resume、leave、free。
- overlay 应出现 enter、leave、free。
- replacement 应出现 enter、leave、free。
- `final-summary ok=1` 表示场景栈、事件分发和 update strategy 检查全部通过。
