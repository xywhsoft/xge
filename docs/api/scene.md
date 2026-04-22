# Scene API

> Scene API 负责场景栈、生命周期回调、事件派发和 update 策略。

[返回 API 索引](README.md) | [场景教程](../guide/scene-intro.md) | [Scene Stack 范例](../case/scene-stack.md)

---

## 模块定位

Scene 是 XGE 的基础游戏结构层。它适合表达主菜单、游戏关卡、暂停层、弹窗式子场景和临时交互流程。Scene 不内建 entity/node 系统，也不限制用户的数据组织方式；用户可以把 ECS、对象树或脚本系统挂在 `pUser` 上。

场景栈用于“进入子场景后保持主场景状态”。例如主战斗场景 push 一个暂停菜单，暂停菜单 pop 后回到战斗场景。

## 公共类型

### `xge_scene_t`

透明 scene 结构，包含用户数据和生命周期回调。XGE 保存 scene 指针，不接管 scene 内存。

### 回调类型

```c
typedef int (*xge_scene_lifecycle_proc)(xge_scene pScene);
typedef int (*xge_scene_event_proc)(xge_scene pScene, const xge_event_t* pEvent);
typedef int (*xge_scene_update_proc)(xge_scene pScene, float fDelta);
typedef int (*xge_scene_draw_proc)(xge_scene pScene);
```

## API 列表

- `xgeSceneSet`
- `xgeScenePush`
- `xgeScenePop`
- `xgeSceneReplace`
- `xgeSceneCurrent`
- `xgeSceneCount`
- `xgeSceneDispatchEvent`
- `xgeSceneUpdateStrategySet`
- `xgeSceneUpdateStrategyGet`

## 场景栈

### xgeSceneSet

清空当前场景栈并设置根场景。

**功能：**

你可以用它进入主场景、切换到另一个完整流程，或传入 `NULL` 清空场景栈。

**函数原型：**

```c
XGE_API int xgeSceneSet(xge_scene pScene);
```

**参数：**

- `pScene`：输入参数，可以为 `NULL`。非 `NULL` 时必须在场景栈持有期间保持有效。

**返回值：**

- 成功返回 `XGE_OK`。
- XGE 尚未初始化时返回 `XGE_ERROR_NOT_INITIALIZED`。
- 场景 `onEnter` 回调失败时返回对应错误码。

**资源归属：**

XGE 保存 scene 指针，不接管 scene 内存。清空旧栈时会调用旧 scene 的 leave/free 回调，但不释放 scene 本身。

**补充说明：**

- 调用成功后会重置固定步长 accumulator。

**范例代码：**

```c
xge_scene_t scene;
memset(&scene, 0, sizeof(scene));
scene.onUpdate = GameUpdate;
scene.onDraw = GameDraw;
xgeSceneSet(&scene);
```

**相关 API：**

- `xgeScenePush`
- `xgeSceneReplace`
- `xgeSceneCurrent`

---

### xgeScenePush

压入子场景。

**功能：**

你可以在保留当前场景状态的同时进入暂停菜单、弹窗、对话框或临时小游戏。

**函数原型：**

```c
XGE_API int xgeScenePush(xge_scene pScene);
```

**参数：**

- `pScene`：输入参数，不能为 `NULL`，且必须在压栈期间保持有效。

**返回值：**

- 成功返回 `XGE_OK`。
- XGE 尚未初始化时返回 `XGE_ERROR_NOT_INITIALIZED`。
- `pScene == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 场景栈满时返回 `XGE_ERROR_OUT_OF_MEMORY`。
- 当前场景 pause 或新场景 enter 失败时返回对应错误码。

**资源归属：**

XGE 保存 scene 指针，不接管 scene 内存。

**补充说明：**

- push 前会暂停当前栈顶场景。
- push 成功后新场景会收到 enter。

**范例代码：**

```c
xgeScenePush(&pause_scene);
```

**相关 API：**

- `xgeScenePop`
- `xgeSceneSet`
- `xgeSceneCount`

---

### xgeScenePop

弹出当前场景。

**功能：**

你可以结束当前子场景，并返回下方场景。

**函数原型：**

```c
XGE_API int xgeScenePop(void);
```

**参数：**

无。

**返回值：**

- 成功返回 `XGE_OK`。
- XGE 尚未初始化时返回 `XGE_ERROR_NOT_INITIALIZED`。
- 场景栈为空时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- leave/free/resume 回调失败时返回对应错误码。

**资源归属：**

XGE 会调用弹出场景的 leave/free 回调，但不释放 scene 指针本身。

**补充说明：**

- 弹出后，如果下方还有场景，会调用其 resume 回调。

**范例代码：**

```c
xgeScenePop();
```

**相关 API：**

- `xgeScenePush`
- `xgeSceneCurrent`
- `xgeSceneCount`

---

### xgeSceneReplace

替换当前栈顶场景。

**功能：**

你可以把当前场景替换为另一个场景，而不保留被替换场景。

**函数原型：**

```c
XGE_API int xgeSceneReplace(xge_scene pScene);
```

**参数：**

- `pScene`：输入参数，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- XGE 尚未初始化时返回 `XGE_ERROR_NOT_INITIALIZED`。
- `pScene == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 旧场景 leave/free 或新场景 enter 失败时返回对应错误码。

**资源归属：**

XGE 保存新 scene 指针，不接管 scene 内存。旧 scene 只会收到回调，不会被 `free()`。

**补充说明：**

- 如果当前栈为空，当前实现退化为 `xgeScenePush(pScene)`。

**范例代码：**

```c
xgeSceneReplace(&game_over_scene);
```

**相关 API：**

- `xgeSceneSet`
- `xgeScenePush`
- `xgeScenePop`

---

### xgeSceneCurrent

获取当前栈顶场景。

**功能：**

你可以读取当前正在接收 update/draw/event 的场景。

**函数原型：**

```c
XGE_API xge_scene xgeSceneCurrent(void);
```

**参数：**

无。

**返回值：**

- 返回当前栈顶 scene。
- XGE 尚未初始化或场景栈为空时返回 `NULL`。

**资源归属：**

返回值是借用指针，调用者不能释放。

**补充说明：**

- 不建议通过返回指针直接改写内部状态字段，业务状态应放在 `pUser`。

**范例代码：**

```c
xge_scene current = xgeSceneCurrent();
```

**相关 API：**

- `xgeSceneCount`
- `xgeSceneSet`
- `xgeScenePush`

---

### xgeSceneCount

获取场景栈数量。

**功能：**

你可以判断当前是否有场景，以及是否处于子场景状态。

**函数原型：**

```c
XGE_API int xgeSceneCount(void);
```

**参数：**

无。

**返回值：**

- 返回场景栈数量。
- XGE 尚未初始化时返回 `0`。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 最大值为 `XGE_SCENE_STACK_MAX`。

**范例代码：**

```c
if (xgeSceneCount() > 1) {
	/* in sub scene */
}
```

**相关 API：**

- `xgeSceneCurrent`
- `xgeScenePush`
- `xgeScenePop`

---

## 事件与更新策略

### xgeSceneDispatchEvent

向当前场景派发事件。

**功能：**

你可以把输入、窗口、手柄、小程序或自定义事件派发给当前栈顶场景。

**函数原型：**

```c
XGE_API int xgeSceneDispatchEvent(const xge_event_t* pEvent);
```

**参数：**

- `pEvent`：输入参数，不能为 `NULL`。调用期间必须有效。

**返回值：**

- 成功派发后返回场景 `onEvent` 的返回值。
- XGE 尚未初始化时返回 `XGE_ERROR_NOT_INITIALIZED`。
- `pEvent == NULL` 时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 没有当前场景或没有 `onEvent` 回调时返回 `XGE_OK`。

**资源归属：**

XGE 不保存 `pEvent` 指针。事件附带的 `pData` 所有权由事件来源管理。

**补充说明：**

- 事件派发只发给当前栈顶场景。

**范例代码：**

```c
xge_event_t event;
memset(&event, 0, sizeof(event));
event.iType = XGE_EVENT_CUSTOM;
xgeSceneDispatchEvent(&event);
```

**相关 API：**

- `xgeSceneCurrent`
- `xgeGamepadSetState`
- `xgeMiniProgramTouch`

---

### xgeSceneUpdateStrategySet

设置 scene update 策略。

**功能：**

你可以选择可变步长或固定步长 update，并限制单帧最多补偿次数。

**函数原型：**

```c
XGE_API int xgeSceneUpdateStrategySet(int iMode, float fFixedStep, int iMaxUpdates);
```

**参数：**

- `iMode`：输入参数，`XGE_UPDATE_VARIABLE` 或 `XGE_UPDATE_FIXED`。
- `fFixedStep`：输入参数，固定步长秒数，必须大于 `0.0f`。
- `iMaxUpdates`：输入参数，单帧最大 update 次数，必须大于 `0`。

**返回值：**

- 成功返回 `XGE_OK`。
- XGE 尚未初始化时返回 `XGE_ERROR_NOT_INITIALIZED`。
- 参数非法时返回 `XGE_ERROR_INVALID_ARGUMENT`。

**资源归属：**

不创建资源，不转移所有权。

**补充说明：**

- 设置成功后会清空内部 accumulator。
- 固定步长适合物理、动作和确定性逻辑；可变步长适合 UI 和工具。

**范例代码：**

```c
xgeSceneUpdateStrategySet(XGE_UPDATE_FIXED, 1.0f / 60.0f, 5);
```

**相关 API：**

- `xgeSceneUpdateStrategyGet`
- `xgeFrame`
- `xgeGetDelta`

---

### xgeSceneUpdateStrategyGet

读取 scene update 策略。

**功能：**

你可以查询当前 update 模式、固定步长和最大补偿次数。

**函数原型：**

```c
XGE_API void xgeSceneUpdateStrategyGet(int* pMode, float* pFixedStep, int* pMaxUpdates);
```

**参数：**

- `pMode`：输出参数，可以为 `NULL`。
- `pFixedStep`：输出参数，可以为 `NULL`。
- `pMaxUpdates`：输出参数，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

调用者持有输出指针。XGE 不保存这些指针。

**补充说明：**

- 可以只传入需要读取的字段。

**范例代码：**

```c
int mode;
float step;
int max_updates;
xgeSceneUpdateStrategyGet(&mode, &step, &max_updates);
```

**相关 API：**

- `xgeSceneUpdateStrategySet`
- `xgeGetDelta`
- `xgeRun`

---

## 生命周期与所有权

Scene 对象由调用者持有。XGE scene stack 保存 scene 指针，不接管 scene 内存所有权。调用者必须保证被设置或压栈的 scene 在出栈或替换前保持有效。

Scene user data 由调用者管理，XGE 只透传指针。

## 线程约束

Scene API 应在主逻辑线程调用。Scene update、draw 和 event dispatch 的顺序由 XGE runtime 或调用者手动驱动，不应跨线程同时修改 scene stack。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| pop 后主场景状态丢失 | 使用 replace 而不是 push/pop | 需要返回原状态时使用 `xgeScenePush`。 |
| 固定步长卡顿时 update 爆炸 | `iMaxUpdates` 设置过大 | 给固定步长设置合理最大 update 次数。 |
| scene 指针失效 | 场景对象在栈上创建后函数返回 | 场景对象生命周期必须覆盖其在场景栈中的时间。 |

## 相关示例

- `examples/scene`
- `examples/app_mode`
- `build_scene_exe.bat`
