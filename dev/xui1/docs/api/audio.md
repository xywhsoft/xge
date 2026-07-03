# Audio API

> Audio API 负责音频系统、全局音量、listener、音量组、sound、music 和 stream。

[返回 API 索引](README.md) | [音频教程](../guide/audio-intro.md) | [Audio 范例](../case/audio-playback.md)

---

## 模块定位

XGE 音频层区分三类对象：

- Sound：短音效，适合按钮、攻击、碰撞等低延迟播放。
- Music：背景音乐，当前实现为流式播放，并关闭空间化。
- Stream：流式音频，适合较长音频、语音或需要边读边播的内容，可设置 3D 坐标。

音频组用于批量控制音量和 fade。3D 坐标播放由 listener 与 sound/stream position 共同决定。当前实现基于 miniaudio；如果构建关闭音频后端，相关函数会返回 `XGE_ERROR_UNSUPPORTED` 或空结果。

## 标准调用顺序

```text
xgeAudioInit
  -> xgeAudioGroupInit 可选
  -> xgeSoundLoad / xgeMusicLoad / xgeStreamOpen
  -> xgeSoundPlay / xgeMusicPlay / xgeStreamPlay
  -> xgeSoundFree / xgeMusicFree / xgeStreamClose
  -> xgeAudioGroupFree
  -> xgeAudioUnit
```

## 常量与宏

| 名称 | 值 | 说明 |
| --- | --- | --- |
| `XGE_AUDIO_SOUND` | `1` | sound 类型。 |
| `XGE_AUDIO_MUSIC` | `2` | music 类型。 |
| `XGE_AUDIO_STREAM` | `3` | stream 类型。 |
| `XGE_SOUND_FALLBACK` | `0x80000000u` | sound fallback 标志。 |

## 公共类型

### `xge_sound_t` / `xge_music_t` / `xge_stream_t`

三者结构相近。

| 字段 | 说明 |
| --- | --- |
| `iRefCount` | 引用计数。 |
| `iType` | `XGE_AUDIO_*`。 |
| `iFlags` | 运行标志。 |
| `pBackend` | 音频后端私有状态。 |

`pBackend` 不应由用户修改。

### `xge_audio_group_t`

| 字段 | 说明 |
| --- | --- |
| `fVolume` | 组音量。 |
| `pBackend` | 后端私有状态。 |

### `xge_audio_listener_t`

| 字段 | 说明 |
| --- | --- |
| `tPosition` | listener 位置。 |
| `tForward` | 朝向。 |
| `tUp` | up 向量。 |

## API 参考

### xgeAudioInit

初始化音频后端。

**功能：**

你可以用它显式启动音频系统，让后续 sound、music、stream 和 audio group 能够创建并播放。

**函数原型：**

```c
XGE_API int xgeAudioInit(void);
```

**参数：**

无。

**返回值：**

- 成功返回 `XGE_OK`。
- XGE 核心尚未初始化时返回 `XGE_ERROR_NOT_INITIALIZED`。
- 内存不足返回 `XGE_ERROR_OUT_OF_MEMORY`。
- 后端设备或 engine 初始化失败返回 `XGE_ERROR_AUDIO_FAILED`。
- 音频后端关闭构建时返回 `XGE_ERROR_UNSUPPORTED`。

**资源归属：**

成功后 XGE 全局状态持有音频 engine，使用 `xgeAudioUnit` 释放。

**补充说明：**

- 重复调用是安全的；已经初始化时直接返回 `XGE_OK`。
- 初始化成功后会把当前 listener 状态应用到后端。

**范例代码：**

```c
if (xgeAudioInit() != XGE_OK) {
    return 1;
}
```

**相关 API：**

- `xgeAudioUnit`
- `xgeAudioIsReady`

---

### xgeAudioUnit

关闭音频后端。

**功能：**

你可以用它释放音频 engine，并让音频系统回到未初始化状态。

**函数原型：**

```c
XGE_API void xgeAudioUnit(void);
```

**参数：**

无。

**返回值：**

无。

**资源归属：**

释放 `xgeAudioInit` 创建的全局音频后端资源。

**补充说明：**

- 未初始化时调用会直接返回。
- 调用前应先释放仍由业务持有的 sound、music、stream 和 group。

**范例代码：**

```c
xgeAudioUnit();
```

**相关 API：**

- `xgeAudioInit`
- `xgeSoundFree`
- `xgeAudioGroupFree`

---

### xgeAudioIsReady

查询音频系统是否可用。

**功能：**

你可以用它判断当前是否已经成功初始化音频后端。

**函数原型：**

```c
XGE_API int xgeAudioIsReady(void);
```

**参数：**

无。

**返回值：**

- 已初始化返回非 0。
- 未初始化或音频后端关闭构建时返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

该函数只反映 XGE 音频 engine 状态，不保证平台当前一定允许立即发声。Web 和移动端可能还有用户手势限制。

**范例代码：**

```c
if (!xgeAudioIsReady()) {
    xgeAudioInit();
}
```

**相关 API：**

- `xgeAudioInit`

---

### xgeAudioSetVolume

设置全局音量。

**功能：**

你可以用它控制整个音频 engine 的主音量。

**函数原型：**

```c
XGE_API void xgeAudioSetVolume(float fVolume);
```

**参数：**

- `fVolume`：主音量。通常使用 0 到 1，具体范围由后端解释。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

音频后端未初始化时调用无效果。

**范例代码：**

```c
xgeAudioSetVolume(0.8f);
```

**相关 API：**

- `xgeAudioGetVolume`
- `xgeAudioGroupSetVolume`

---

### xgeAudioGetVolume

获取全局音量。

**功能：**

你可以用它读取当前音频 engine 的主音量。

**函数原型：**

```c
XGE_API float xgeAudioGetVolume(void);
```

**参数：**

无。

**返回值：**

- 音频后端已初始化时返回当前主音量。
- 未初始化时返回 `0.0f`。

**资源归属：**

函数不分配资源。

**补充说明：**

返回值来自后端 engine。

**范例代码：**

```c
float volume = xgeAudioGetVolume();
```

**相关 API：**

- `xgeAudioSetVolume`

---

### xgeAudioListenerSet

设置 3D audio listener。

**功能：**

你可以用它设置听者位置、朝向和 up 向量，影响 sound/stream 的 3D 空间化。

**函数原型：**

```c
XGE_API void xgeAudioListenerSet(const xge_audio_listener_t* pListener);
```

**参数：**

- `pListener`：listener 描述，不能为 `NULL` 才会生效。

**返回值：**

无。

**资源归属：**

函数复制 listener 值，不持有传入指针。

**补充说明：**

- 后端未初始化时，也会先保存 listener；后续 `xgeAudioInit` 成功后会应用。
- `pListener` 为 `NULL` 时直接返回。

**范例代码：**

```c
xge_audio_listener_t listener;
memset(&listener, 0, sizeof(listener));
listener.tForward.fZ = -1.0f;
listener.tUp.fY = 1.0f;
xgeAudioListenerSet(&listener);
```

**相关 API：**

- `xgeAudioListenerGet`
- `xgeSoundSetPosition`
- `xgeStreamSetPosition`

---

### xgeAudioListenerGet

获取当前 listener。

**功能：**

你可以用它读取 XGE 保存的 3D audio listener 状态。

**函数原型：**

```c
XGE_API xge_audio_listener_t xgeAudioListenerGet(void);
```

**参数：**

无。

**返回值：**

返回 listener 值副本。音频后端关闭构建时返回清零结构体。

**资源归属：**

返回值是结构体副本，不需要释放。

**补充说明：**

该函数返回 XGE 保存的状态，不返回后端内部指针。

**范例代码：**

```c
xge_audio_listener_t listener = xgeAudioListenerGet();
```

**相关 API：**

- `xgeAudioListenerSet`

---

### xgeAudioGroupInit

初始化音量组。

**功能：**

你可以用它创建一个混音组，用于批量控制绑定到该组的 sound、music 或 stream。

**函数原型：**

```c
XGE_API int xgeAudioGroupInit(xge_audio_group pGroup);
```

**参数：**

- `pGroup`：输出音量组对象，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 音频后端未初始化返回 `XGE_ERROR_NOT_INITIALIZED`。
- 内存不足返回 `XGE_ERROR_OUT_OF_MEMORY`。
- 后端 group 创建失败返回 `XGE_ERROR_AUDIO_FAILED`。
- 音频后端关闭构建时返回 `XGE_ERROR_UNSUPPORTED`。

**资源归属：**

创建成功后，`pGroup` 持有后端 group，调用者必须使用 `xgeAudioGroupFree` 释放。

**补充说明：**

默认组音量为 `1.0f`。当前 group 创建时关闭空间化，用于混音控制。

**范例代码：**

```c
xge_audio_group_t ui_group;
xgeAudioGroupInit(&ui_group);
```

**相关 API：**

- `xgeAudioGroupFree`
- `xgeSoundLoadGroup`

---

### xgeAudioGroupFree

释放音量组。

**功能：**

你可以用它释放 group 后端资源，并清零 group 对象。

**函数原型：**

```c
XGE_API void xgeAudioGroupFree(xge_audio_group pGroup);
```

**参数：**

- `pGroup`：音量组对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `pGroup` 内部后端资源，不释放结构体本身。

**补充说明：**

对 `NULL` 调用是安全的。释放 group 前应确保不再依赖绑定到该 group 的音频对象。

**范例代码：**

```c
xgeAudioGroupFree(&ui_group);
```

**相关 API：**

- `xgeAudioGroupInit`

---

### xgeAudioGroupSetVolume

设置音量组音量。

**功能：**

你可以用它统一调整一组音频对象的音量。

**函数原型：**

```c
XGE_API void xgeAudioGroupSetVolume(xge_audio_group pGroup, float fVolume);
```

**参数：**

- `pGroup`：音量组对象。
- `fVolume`：组音量。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

如果后端 group 已创建，会同步设置到后端；无后端时只更新结构体中的 `fVolume`。

**范例代码：**

```c
xgeAudioGroupSetVolume(&ui_group, 0.5f);
```

**相关 API：**

- `xgeAudioGroupGetVolume`
- `xgeAudioGroupFade`

---

### xgeAudioGroupGetVolume

获取音量组音量。

**功能：**

你可以用它读取 group 当前音量。

**函数原型：**

```c
XGE_API float xgeAudioGroupGetVolume(xge_audio_group pGroup);
```

**参数：**

- `pGroup`：音量组对象。

**返回值：**

- `pGroup` 有效时返回当前组音量。
- `pGroup` 为 `NULL` 时返回 `0.0f`。

**资源归属：**

函数不分配资源。

**补充说明：**

如果后端 group 存在，函数会先从后端刷新 `fVolume`。

**范例代码：**

```c
float group_volume = xgeAudioGroupGetVolume(&ui_group);
```

**相关 API：**

- `xgeAudioGroupSetVolume`

---

### xgeAudioGroupFade

对音量组执行淡变。

**功能：**

你可以用它实现整组声音的淡入、淡出或过渡。

**函数原型：**

```c
XGE_API void xgeAudioGroupFade(xge_audio_group pGroup, float fFrom, float fTo, int iMilliseconds);
```

**参数：**

- `pGroup`：音量组对象。
- `fFrom`：起始音量。
- `fTo`：目标音量。
- `iMilliseconds`：淡变毫秒数。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

只有 `pGroup` 和后端 group 都有效时才会生效。函数会把 `pGroup->fVolume` 更新为 `fTo`。

**范例代码：**

```c
xgeAudioGroupFade(&music_group, 0.0f, 1.0f, 500);
```

**相关 API：**

- `xgeAudioGroupSetVolume`

---

### xgeSoundLoad

加载短音效。

**功能：**

你可以用它加载按钮、碰撞、攻击等短音效，不绑定音量组。

**函数原型：**

```c
XGE_API int xgeSoundLoad(xge_sound pSound, const char* sPath);
```

**参数：**

- `pSound`：输出 sound 对象，不能为 `NULL`。
- `sPath`：音频文件路径，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 音频后端未初始化返回 `XGE_ERROR_NOT_INITIALIZED`。
- 路径解析失败返回 `XGE_ERROR_UNSUPPORTED`。
- 内存不足返回 `XGE_ERROR_OUT_OF_MEMORY`。
- 后端加载失败返回 `XGE_ERROR_AUDIO_FAILED`。

**资源归属：**

创建成功后 `pSound` 持有后端 sound，调用者必须使用 `xgeSoundFree` 释放。

**补充说明：**

加载失败且已设置 sound fallback 时，会自动尝试 fallback，并设置 `XGE_SOUND_FALLBACK` 标志。

**范例代码：**

```c
xge_sound_t click;
if (xgeSoundLoad(&click, "assets/click.wav") == XGE_OK) {
    xgeSoundPlay(&click);
}
```

**相关 API：**

- `xgeSoundLoadGroup`
- `xgeSoundFree`
- `xgeSoundFallbackSet`

---

### xgeSoundLoadGroup

加载短音效并绑定音量组。

**功能：**

你可以用它让 sound 接入指定 group，后续由 group 统一控制音量和 fade。

**函数原型：**

```c
XGE_API int xgeSoundLoadGroup(xge_sound pSound, const char* sPath, xge_audio_group pGroup);
```

**参数：**

- `pSound`：输出 sound 对象，不能为 `NULL`。
- `sPath`：音频文件路径，不能为 `NULL`。
- `pGroup`：音量组，可以为 `NULL`。

**返回值：**

同 `xgeSoundLoad`。

**资源归属：**

sound 由调用者持有并使用 `xgeSoundFree` 释放。group 由调用者单独管理。

**补充说明：**

`pGroup` 为 `NULL` 时等价于不绑定音量组。

**范例代码：**

```c
xgeSoundLoadGroup(&click, "assets/click.wav", &ui_group);
```

**相关 API：**

- `xgeAudioGroupInit`
- `xgeSoundLoad`

---

### xgeSoundFallbackSet

设置 sound fallback 路径。

**功能：**

你可以配置一个默认音效文件，当普通 sound 加载失败时自动加载该 fallback。

**函数原型：**

```c
XGE_API int xgeSoundFallbackSet(const char* sPath);
```

**参数：**

- `sPath`：fallback 音频路径，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 内存不足返回 `XGE_ERROR_OUT_OF_MEMORY`。
- 音频后端关闭构建时返回 `XGE_ERROR_UNSUPPORTED`。

**资源归属：**

函数会复制路径字符串。全局 fallback 路径由 XGE 持有，使用 `xgeSoundFallbackClear` 释放。

**补充说明：**

设置新 fallback 前会清除旧路径。

**范例代码：**

```c
xgeSoundFallbackSet("assets/silence.wav");
```

**相关 API：**

- `xgeSoundFallbackGet`
- `xgeSoundFallbackClear`

---

### xgeSoundFallbackGet

加载 fallback sound。

**功能：**

你可以显式把当前 fallback 音效加载到指定 sound 对象中。

**函数原型：**

```c
XGE_API int xgeSoundFallbackGet(xge_sound pSound);
```

**参数：**

- `pSound`：输出 sound 对象，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 未设置 fallback 路径返回 `XGE_ERROR_RESOURCE_FAILED`。
- 加载失败时返回底层加载错误码。

**资源归属：**

成功后 `pSound` 由调用者持有，使用 `xgeSoundFree` 释放。

**补充说明：**

成功后会设置 `XGE_SOUND_FALLBACK` 标志。

**范例代码：**

```c
xge_sound_t fallback;
xgeSoundFallbackGet(&fallback);
```

**相关 API：**

- `xgeSoundFallbackSet`
- `xgeSoundFree`

---

### xgeSoundFallbackClear

清除 sound fallback 路径。

**功能：**

你可以用它释放全局 fallback 路径，并停止后续自动 fallback。

**函数原型：**

```c
XGE_API void xgeSoundFallbackClear(void);
```

**参数：**

无。

**返回值：**

无。

**资源归属：**

释放 XGE 保存的 fallback 路径字符串。

**补充说明：**

不会影响已经加载出来的 fallback sound 对象。

**范例代码：**

```c
xgeSoundFallbackClear();
```

**相关 API：**

- `xgeSoundFallbackSet`

---

### xgeSoundPlay

播放 sound。

**功能：**

你可以用它启动一个已加载 sound。

**函数原型：**

```c
XGE_API int xgeSoundPlay(xge_sound pSound);
```

**参数：**

- `pSound`：已加载的 sound。

**返回值：**

- 成功返回 `XGE_OK`。
- sound 无效返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 后端播放失败返回 `XGE_ERROR_AUDIO_FAILED`。

**资源归属：**

函数不接管 sound 所有权。

**补充说明：**

调用前应确保 sound 已加载成功。

**范例代码：**

```c
xgeSoundPlay(&click);
```

**相关 API：**

- `xgeSoundStop`
- `xgeSoundIsPlaying`

---

### xgeSoundStop

停止 sound 并回到开头。

**功能：**

你可以用它停止播放，并把播放位置 seek 到起始帧。

**函数原型：**

```c
XGE_API int xgeSoundStop(xge_sound pSound);
```

**参数：**

- `pSound`：sound 对象。

**返回值：**

- 成功返回 `XGE_OK`。
- sound 无效返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 后端停止失败返回 `XGE_ERROR_AUDIO_FAILED`。

**资源归属：**

函数不释放 sound。

**补充说明：**

与 `xgeSoundPause` 不同，stop 会把播放位置重置到 0。

**范例代码：**

```c
xgeSoundStop(&click);
```

**相关 API：**

- `xgeSoundPause`
- `xgeSoundPlay`

---

### xgeSoundPause

暂停 sound。

**功能：**

你可以用它停止播放但保留当前播放位置。

**函数原型：**

```c
XGE_API int xgeSoundPause(xge_sound pSound);
```

**参数：**

- `pSound`：sound 对象。

**返回值：**

- 成功返回 `XGE_OK`。
- sound 无效返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 后端暂停失败返回 `XGE_ERROR_AUDIO_FAILED`。

**资源归属：**

函数不释放 sound。

**补充说明：**

当前实现通过后端 stop 实现暂停，但不会 seek 到起点。

**范例代码：**

```c
xgeSoundPause(&click);
```

**相关 API：**

- `xgeSoundResume`
- `xgeSoundStop`

---

### xgeSoundResume

恢复 sound 播放。

**功能：**

你可以用它从当前位置继续播放 sound。

**函数原型：**

```c
XGE_API int xgeSoundResume(xge_sound pSound);
```

**参数：**

- `pSound`：sound 对象。

**返回值：**

返回 `xgeSoundPlay` 的结果。

**资源归属：**

函数不接管 sound 所有权。

**补充说明：**

当前实现等价于调用 `xgeSoundPlay`。

**范例代码：**

```c
xgeSoundResume(&click);
```

**相关 API：**

- `xgeSoundPause`
- `xgeSoundPlay`

---

### xgeSoundAddRef

增加 sound 引用计数。

**功能：**

你可以在多个对象共享同一个 sound 时调用它，避免提前释放。

**函数原型：**

```c
XGE_API int xgeSoundAddRef(xge_sound pSound);
```

**参数：**

- `pSound`：sound 对象，可以为 `NULL`。

**返回值：**

- sound 有效时返回新的引用计数。
- `pSound` 为 `NULL` 时返回 0。

**资源归属：**

每次增加引用后，都需要对应调用一次 `xgeSoundFree`。

**补充说明：**

当前实现直接递增引用计数。

**范例代码：**

```c
xgeSoundAddRef(&click);
```

**相关 API：**

- `xgeSoundFree`

---

### xgeSoundFree

释放 sound。

**功能：**

你可以用它减少 sound 引用计数，并在归零时释放后端 sound。

**函数原型：**

```c
XGE_API void xgeSoundFree(xge_sound pSound);
```

**参数：**

- `pSound`：sound 对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `pSound` 内部后端资源，不释放结构体本身。

**补充说明：**

- 对 `NULL` 调用是安全的。
- 引用计数大于 1 时只递减引用，不释放实际资源。

**范例代码：**

```c
xgeSoundFree(&click);
```

**相关 API：**

- `xgeSoundLoad`
- `xgeSoundAddRef`

---

### xgeSoundSetLoop

设置 sound 循环播放。

**功能：**

你可以用它控制 sound 播放到末尾后是否自动循环。

**函数原型：**

```c
XGE_API void xgeSoundSetLoop(xge_sound pSound, int bLoop);
```

**参数：**

- `pSound`：sound 对象。
- `bLoop`：非 0 表示循环，0 表示不循环。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

sound 或后端无效时无效果。

**范例代码：**

```c
xgeSoundSetLoop(&click, 1);
```

**相关 API：**

- `xgeSoundPlay`

---

### xgeSoundSetVolume

设置 sound 音量。

**功能：**

你可以单独调整某个 sound 的音量。

**函数原型：**

```c
XGE_API void xgeSoundSetVolume(xge_sound pSound, float fVolume);
```

**参数：**

- `pSound`：sound 对象。
- `fVolume`：音量。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

最终音量还会受到全局音量和 group 音量影响。

**范例代码：**

```c
xgeSoundSetVolume(&click, 0.7f);
```

**相关 API：**

- `xgeAudioSetVolume`
- `xgeAudioGroupSetVolume`

---

### xgeSoundSetPosition

设置 sound 的 3D 位置。

**功能：**

你可以用它让 sound 进入绝对 3D 定位模式，并设置空间坐标。

**函数原型：**

```c
XGE_API void xgeSoundSetPosition(xge_sound pSound, float fX, float fY, float fZ);
```

**参数：**

- `pSound`：sound 对象。
- `fX`：X 坐标。
- `fY`：Y 坐标。
- `fZ`：Z 坐标。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

3D 效果还取决于 listener 设置和后端平台能力。

**范例代码：**

```c
xgeSoundSetPosition(&explosion, 10.0f, 0.0f, 0.0f);
```

**相关 API：**

- `xgeAudioListenerSet`
- `xgeStreamSetPosition`

---

### xgeSoundFade

设置 sound 音量淡变。

**功能：**

你可以用它对单个 sound 执行淡入、淡出或音量过渡。

**函数原型：**

```c
XGE_API void xgeSoundFade(xge_sound pSound, float fFrom, float fTo, int iMilliseconds);
```

**参数：**

- `pSound`：sound 对象。
- `fFrom`：起始音量。
- `fTo`：目标音量。
- `iMilliseconds`：淡变毫秒数。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

sound 或后端无效时无效果。

**范例代码：**

```c
xgeSoundFade(&click, 1.0f, 0.0f, 250);
```

**相关 API：**

- `xgeAudioGroupFade`
- `xgeMusicFade`

---

### xgeSoundIsPlaying

查询 sound 是否正在播放。

**功能：**

你可以用它检查音效当前播放状态。

**函数原型：**

```c
XGE_API int xgeSoundIsPlaying(xge_sound pSound);
```

**参数：**

- `pSound`：sound 对象。

**返回值：**

- 正在播放返回 1。
- 未播放、对象无效或音频后端关闭构建时返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

该函数直接查询后端播放状态。

**范例代码：**

```c
if (!xgeSoundIsPlaying(&click)) {
    xgeSoundPlay(&click);
}
```

**相关 API：**

- `xgeSoundPlay`
- `xgeSoundPause`

---

### xgeMusicLoad

加载背景音乐。

**功能：**

你可以用它加载较长音乐文件。当前实现使用 stream 标志并关闭空间化。

**函数原型：**

```c
XGE_API int xgeMusicLoad(xge_music pMusic, const char* sPath);
```

**参数：**

- `pMusic`：输出 music 对象，不能为 `NULL`。
- `sPath`：音乐文件路径，不能为 `NULL`。

**返回值：**

同 `xgeSoundLoad`，但 music 不使用 sound fallback。

**资源归属：**

创建成功后 `pMusic` 持有后端资源，调用者使用 `xgeMusicFree` 释放。

**补充说明：**

Music 适合 BGM，不支持 position API。

**范例代码：**

```c
xge_music_t bgm;
xgeMusicLoad(&bgm, "assets/bgm.ogg");
```

**相关 API：**

- `xgeMusicLoadGroup`
- `xgeMusicPlay`
- `xgeMusicFree`

---

### xgeMusicLoadGroup

加载背景音乐并绑定音量组。

**功能：**

你可以把 BGM 绑定到 music group，统一控制音乐音量和 fade。

**函数原型：**

```c
XGE_API int xgeMusicLoadGroup(xge_music pMusic, const char* sPath, xge_audio_group pGroup);
```

**参数：**

- `pMusic`：输出 music 对象。
- `sPath`：音乐路径。
- `pGroup`：音量组，可以为 `NULL`。

**返回值：**

同 `xgeMusicLoad`。

**资源归属：**

music 和 group 生命周期分别由调用者管理。

**补充说明：**

当前实现底层调用统一音频加载逻辑，flags 为 stream + no spatialization。

**范例代码：**

```c
xgeMusicLoadGroup(&bgm, "assets/bgm.ogg", &music_group);
```

**相关 API：**

- `xgeAudioGroupInit`
- `xgeMusicLoad`

---

### xgeMusicPlay

播放 music。

**功能：**

你可以用它启动已加载的背景音乐。

**函数原型：**

```c
XGE_API int xgeMusicPlay(xge_music pMusic);
```

**参数：**

- `pMusic`：music 对象。

**返回值：**

返回底层播放控制结果：成功 `XGE_OK`，对象非法或后端失败返回对应错误码。

**资源归属：**

函数不接管 music 所有权。

**补充说明：**

当前实现复用 sound 播放逻辑。

**范例代码：**

```c
xgeMusicPlay(&bgm);
```

**相关 API：**

- `xgeMusicStop`
- `xgeMusicIsPlaying`

---

### xgeMusicStop

停止 music 并回到开头。

**功能：**

你可以用它停止背景音乐，并把播放位置重置到起点。

**函数原型：**

```c
XGE_API int xgeMusicStop(xge_music pMusic);
```

**参数：**

- `pMusic`：music 对象。

**返回值：**

返回底层停止控制结果。

**资源归属：**

函数不释放 music。

**补充说明：**

当前实现复用 `xgeSoundStop`。

**范例代码：**

```c
xgeMusicStop(&bgm);
```

**相关 API：**

- `xgeMusicPause`
- `xgeMusicPlay`

---

### xgeMusicPause

暂停 music。

**功能：**

你可以用它暂停背景音乐，后续可恢复播放。

**函数原型：**

```c
XGE_API int xgeMusicPause(xge_music pMusic);
```

**参数：**

- `pMusic`：music 对象。

**返回值：**

返回底层暂停控制结果。

**资源归属：**

函数不释放 music。

**补充说明：**

当前实现复用 `xgeSoundPause`。

**范例代码：**

```c
xgeMusicPause(&bgm);
```

**相关 API：**

- `xgeMusicResume`

---

### xgeMusicResume

恢复 music。

**功能：**

你可以用它继续播放此前暂停的背景音乐。

**函数原型：**

```c
XGE_API int xgeMusicResume(xge_music pMusic);
```

**参数：**

- `pMusic`：music 对象。

**返回值：**

返回底层播放控制结果。

**资源归属：**

函数不接管 music 所有权。

**补充说明：**

当前实现复用 `xgeSoundResume`。

**范例代码：**

```c
xgeMusicResume(&bgm);
```

**相关 API：**

- `xgeMusicPause`
- `xgeMusicPlay`

---

### xgeMusicFree

释放 music。

**功能：**

你可以用它释放背景音乐后端资源。

**函数原型：**

```c
XGE_API void xgeMusicFree(xge_music pMusic);
```

**参数：**

- `pMusic`：music 对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `pMusic` 内部资源，不释放结构体本身。

**补充说明：**

当前实现复用 sound 释放逻辑。

**范例代码：**

```c
xgeMusicFree(&bgm);
```

**相关 API：**

- `xgeMusicLoad`

---

### xgeMusicSetLoop

设置 music 循环播放。

**功能：**

你可以用它控制背景音乐是否循环。

**函数原型：**

```c
XGE_API void xgeMusicSetLoop(xge_music pMusic, int bLoop);
```

**参数：**

- `pMusic`：music 对象。
- `bLoop`：非 0 循环，0 不循环。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

当前实现复用 sound loop 设置。

**范例代码：**

```c
xgeMusicSetLoop(&bgm, 1);
```

**相关 API：**

- `xgeMusicPlay`

---

### xgeMusicSetVolume

设置 music 音量。

**功能：**

你可以单独调整背景音乐音量。

**函数原型：**

```c
XGE_API void xgeMusicSetVolume(xge_music pMusic, float fVolume);
```

**参数：**

- `pMusic`：music 对象。
- `fVolume`：音量。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

最终音量仍受全局音量和 group 音量影响。

**范例代码：**

```c
xgeMusicSetVolume(&bgm, 0.6f);
```

**相关 API：**

- `xgeAudioSetVolume`
- `xgeAudioGroupSetVolume`

---

### xgeMusicFade

设置 music 音量淡变。

**功能：**

你可以用它实现背景音乐淡入淡出。

**函数原型：**

```c
XGE_API void xgeMusicFade(xge_music pMusic, float fFrom, float fTo, int iMilliseconds);
```

**参数：**

- `pMusic`：music 对象。
- `fFrom`：起始音量。
- `fTo`：目标音量。
- `iMilliseconds`：淡变毫秒数。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

当前实现复用 sound fade。

**范例代码：**

```c
xgeMusicFade(&bgm, 0.0f, 1.0f, 1000);
```

**相关 API：**

- `xgeAudioGroupFade`

---

### xgeMusicIsPlaying

查询 music 是否正在播放。

**功能：**

你可以用它判断背景音乐当前是否处于播放状态。

**函数原型：**

```c
XGE_API int xgeMusicIsPlaying(xge_music pMusic);
```

**参数：**

- `pMusic`：music 对象。

**返回值：**

- 正在播放返回 1。
- 未播放或对象无效返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

当前实现复用 sound 播放状态查询。

**范例代码：**

```c
if (!xgeMusicIsPlaying(&bgm)) {
    xgeMusicPlay(&bgm);
}
```

**相关 API：**

- `xgeMusicPlay`
- `xgeMusicPause`

---

### xgeStreamOpen

打开流式音频。

**功能：**

你可以用它打开较长音频、语音或需要边读边播的音频文件。

**函数原型：**

```c
XGE_API int xgeStreamOpen(xge_stream pStream, const char* sPath);
```

**参数：**

- `pStream`：输出 stream 对象，不能为 `NULL`。
- `sPath`：音频路径，不能为 `NULL`。

**返回值：**

同 `xgeSoundLoad`，但 stream 不使用 sound fallback。

**资源归属：**

打开成功后 `pStream` 持有后端资源，调用者使用 `xgeStreamClose` 释放。

**补充说明：**

Stream 使用后端 stream 标志，并保留空间化能力。

**范例代码：**

```c
xge_stream_t voice;
xgeStreamOpen(&voice, "assets/voice.ogg");
```

**相关 API：**

- `xgeStreamOpenGroup`
- `xgeStreamClose`

---

### xgeStreamOpenGroup

打开流式音频并绑定音量组。

**功能：**

你可以让 stream 接入指定 group，以便统一控制音量和淡变。

**函数原型：**

```c
XGE_API int xgeStreamOpenGroup(xge_stream pStream, const char* sPath, xge_audio_group pGroup);
```

**参数：**

- `pStream`：输出 stream 对象。
- `sPath`：音频路径。
- `pGroup`：音量组，可以为 `NULL`。

**返回值：**

同 `xgeStreamOpen`。

**资源归属：**

stream 和 group 生命周期分别由调用者管理。

**补充说明：**

`pGroup` 为 `NULL` 时不绑定音量组。

**范例代码：**

```c
xgeStreamOpenGroup(&voice, "assets/voice.ogg", &voice_group);
```

**相关 API：**

- `xgeAudioGroupInit`
- `xgeStreamOpen`

---

### xgeStreamPlay

播放 stream。

**功能：**

你可以用它启动已打开的流式音频。

**函数原型：**

```c
XGE_API int xgeStreamPlay(xge_stream pStream);
```

**参数：**

- `pStream`：stream 对象。

**返回值：**

返回底层播放控制结果。

**资源归属：**

函数不接管 stream 所有权。

**补充说明：**

当前实现复用 sound 播放逻辑。

**范例代码：**

```c
xgeStreamPlay(&voice);
```

**相关 API：**

- `xgeStreamStop`
- `xgeStreamIsPlaying`

---

### xgeStreamStop

停止 stream 并回到开头。

**功能：**

你可以用它停止流式音频并重置播放位置。

**函数原型：**

```c
XGE_API int xgeStreamStop(xge_stream pStream);
```

**参数：**

- `pStream`：stream 对象。

**返回值：**

返回底层停止控制结果。

**资源归属：**

函数不释放 stream。

**补充说明：**

当前实现复用 `xgeSoundStop`。

**范例代码：**

```c
xgeStreamStop(&voice);
```

**相关 API：**

- `xgeStreamPause`
- `xgeStreamPlay`

---

### xgeStreamPause

暂停 stream。

**功能：**

你可以用它暂停流式音频，后续可恢复播放。

**函数原型：**

```c
XGE_API int xgeStreamPause(xge_stream pStream);
```

**参数：**

- `pStream`：stream 对象。

**返回值：**

返回底层暂停控制结果。

**资源归属：**

函数不释放 stream。

**补充说明：**

当前实现复用 `xgeSoundPause`。

**范例代码：**

```c
xgeStreamPause(&voice);
```

**相关 API：**

- `xgeStreamResume`

---

### xgeStreamResume

恢复 stream。

**功能：**

你可以用它继续播放此前暂停的流式音频。

**函数原型：**

```c
XGE_API int xgeStreamResume(xge_stream pStream);
```

**参数：**

- `pStream`：stream 对象。

**返回值：**

返回底层播放控制结果。

**资源归属：**

函数不接管 stream 所有权。

**补充说明：**

当前实现复用 `xgeSoundResume`。

**范例代码：**

```c
xgeStreamResume(&voice);
```

**相关 API：**

- `xgeStreamPause`
- `xgeStreamPlay`

---

### xgeStreamClose

关闭 stream。

**功能：**

你可以用它释放流式音频后端资源。

**函数原型：**

```c
XGE_API void xgeStreamClose(xge_stream pStream);
```

**参数：**

- `pStream`：stream 对象，可以为 `NULL`。

**返回值：**

无。

**资源归属：**

释放 `pStream` 内部资源，不释放结构体本身。

**补充说明：**

当前实现复用 sound 释放逻辑。

**范例代码：**

```c
xgeStreamClose(&voice);
```

**相关 API：**

- `xgeStreamOpen`

---

### xgeStreamSetLoop

设置 stream 循环播放。

**功能：**

你可以控制流式音频是否在结尾循环。

**函数原型：**

```c
XGE_API void xgeStreamSetLoop(xge_stream pStream, int bLoop);
```

**参数：**

- `pStream`：stream 对象。
- `bLoop`：非 0 循环，0 不循环。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

当前实现复用 sound loop 设置。

**范例代码：**

```c
xgeStreamSetLoop(&voice, 1);
```

**相关 API：**

- `xgeStreamPlay`

---

### xgeStreamSetVolume

设置 stream 音量。

**功能：**

你可以单独调整流式音频音量。

**函数原型：**

```c
XGE_API void xgeStreamSetVolume(xge_stream pStream, float fVolume);
```

**参数：**

- `pStream`：stream 对象。
- `fVolume`：音量。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

最终音量仍受全局音量和 group 音量影响。

**范例代码：**

```c
xgeStreamSetVolume(&voice, 0.8f);
```

**相关 API：**

- `xgeAudioSetVolume`
- `xgeAudioGroupSetVolume`

---

### xgeStreamSetPosition

设置 stream 的 3D 位置。

**功能：**

你可以用它让流式音频进入绝对 3D 定位模式，并设置空间坐标。

**函数原型：**

```c
XGE_API void xgeStreamSetPosition(xge_stream pStream, float fX, float fY, float fZ);
```

**参数：**

- `pStream`：stream 对象。
- `fX`：X 坐标。
- `fY`：Y 坐标。
- `fZ`：Z 坐标。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

Music 没有 position API；需要空间化长音频时使用 stream。

**范例代码：**

```c
xgeStreamSetPosition(&voice, -2.0f, 0.0f, 0.0f);
```

**相关 API：**

- `xgeAudioListenerSet`
- `xgeSoundSetPosition`

---

### xgeStreamFade

设置 stream 音量淡变。

**功能：**

你可以对流式音频执行淡入淡出。

**函数原型：**

```c
XGE_API void xgeStreamFade(xge_stream pStream, float fFrom, float fTo, int iMilliseconds);
```

**参数：**

- `pStream`：stream 对象。
- `fFrom`：起始音量。
- `fTo`：目标音量。
- `iMilliseconds`：淡变毫秒数。

**返回值：**

无。

**资源归属：**

函数不接管任何资源。

**补充说明：**

当前实现复用 sound fade。

**范例代码：**

```c
xgeStreamFade(&voice, 1.0f, 0.0f, 300);
```

**相关 API：**

- `xgeAudioGroupFade`

---

### xgeStreamIsPlaying

查询 stream 是否正在播放。

**功能：**

你可以用它检查流式音频当前播放状态。

**函数原型：**

```c
XGE_API int xgeStreamIsPlaying(xge_stream pStream);
```

**参数：**

- `pStream`：stream 对象。

**返回值：**

- 正在播放返回 1。
- 未播放或对象无效返回 0。

**资源归属：**

函数不分配资源。

**补充说明：**

当前实现复用 sound 播放状态查询。

**范例代码：**

```c
if (xgeStreamIsPlaying(&voice)) {
    xgeStreamPause(&voice);
}
```

**相关 API：**

- `xgeStreamPlay`
- `xgeStreamPause`

---

## 生命周期与所有权

`xgeAudioInit` 和 `xgeAudioUnit` 管理音频系统生命周期。Sound、music、stream 和 group 对象由调用者持有，并分别用对应 `Free`、`Close` 或 `Unit` 函数释放。

Sound 支持引用计数，`xgeSoundAddRef` 后需要对应 `xgeSoundFree`。Music 和 stream 当前复用 sound 的底层释放逻辑。Group 不接管 sound/music/stream 所有权，只作为混音控制对象。

## 线程约束

音频控制 API 可从主线程调用。实时音频回调中不能分配内存、写复杂日志或调用可能阻塞的 API。

3D position、volume、fade 等运行时控制应避免多个线程并发修改同一对象。

## 后端差异

桌面、移动、Web 和小程序的音频启动策略不同。Web 和移动端可能要求用户手势后播放，小程序可能需要通过宿主音频命令桥接。

设备采样率、声道布局和 3D audio 能力由 miniaudio 后端和平台共同决定。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 播放无声 | 音频后端未初始化或设备不可用 | 检查 `xgeAudioInit` 返回值和 `xgeAudioIsReady`。 |
| 音量组 fade 没效果 | 对象没有绑定到 group | 使用 `xgeSoundLoadGroup`、`xgeMusicLoadGroup` 或 `xgeStreamOpenGroup`。 |
| 3D 声音位置异常 | listener 朝向或 up 向量未设置 | 初始化 `xge_audio_listener_t` 并调用 `xgeAudioListenerSet`。 |

## 相关示例

- `examples/audio`
- `examples\audio\build.bat`
