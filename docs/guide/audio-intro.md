# 音频入门

> 状态：中文初稿已生成，待审阅。

XGE 音频把对象分成 sound、music、stream 三类。先按用途选对象，再考虑音量组、循环、fade 和 3D 坐标。

## 选择 sound、music 还是 stream

| 类型 | 适合场景 |
| --- | --- |
| Sound | 短音效，例如点击、攻击、碰撞、提示音。 |
| Music | 背景音乐，通常循环播放和淡入淡出。 |
| Stream | 长音频、语音、边读边播的内容。 |

## 初始化音频

```c
if ( xgeAudioInit() != XGE_OK ) {
	return 1;
}
```

退出时：

```c
xgeAudioUnit();
```

可以用 `xgeAudioIsReady` 判断音频设备是否可用。

## 播放短音效

```c
xge_sound_t click;
memset(&click, 0, sizeof(click));

if ( xgeSoundLoad(&click, "assets/click.wav") == XGE_OK ) {
	xgeSoundPlay(&click);
}

/* 退出或不再需要时 */
xgeSoundFree(&click);
```

如果音效可能被多个系统持有，用 `xgeSoundAddRef` 增加引用，最终配对 `xgeSoundFree`。

## 播放音乐

```c
xge_music_t music;
memset(&music, 0, sizeof(music));

if ( xgeMusicLoad(&music, "assets/bgm.ogg") == XGE_OK ) {
	xgeMusicSetLoop(&music, 1);
	xgeMusicFade(&music, 0.0f, 1.0f, 1000);
	xgeMusicPlay(&music);
}
```

音乐通常使用 loop 和 fade，让场景切换更平滑。

## 音量组

音量组适合做全局分类控制，例如 master、music、sfx、ui。

```c
xge_audio_group_t sfx;
memset(&sfx, 0, sizeof(sfx));
xgeAudioGroupInit(&sfx);
xgeAudioGroupSetVolume(&sfx, 0.8f);

xgeSoundLoadGroup(&click, "assets/click.wav", &sfx);
```

需要整体淡出时：

```c
xgeAudioGroupFade(&sfx, 0.8f, 0.0f, 500);
```

## 3D 坐标播放

3D 声音由 listener 和 sound/stream position 决定。

```c
xge_audio_listener_t listener;
listener.tPosition = (xge_vec3_t){ 0.0f, 0.0f, 0.0f };
listener.tForward = (xge_vec3_t){ 0.0f, 0.0f, -1.0f };
listener.tUp = (xge_vec3_t){ 0.0f, 1.0f, 0.0f };
xgeAudioListenerSet(&listener);

xgeSoundSetPosition(&click, 4.0f, 0.0f, -2.0f);
```

普通 2D 游戏也可以用 X/Y 坐标映射到 3D 平面，得到简单的左右声像和距离衰减。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 没声音 | 音频系统没初始化或设备不可用 | 检查 `xgeAudioInit` 返回值和 `xgeAudioIsReady`。 |
| fade 没效果 | 对象没有绑定到 group，或直接设置音量覆盖 | 使用 `xgeSoundLoadGroup` / `xgeMusicLoadGroup`。 |
| 声音位置怪 | listener forward/up 没初始化 | 显式设置完整 `xge_audio_listener_t`。 |

## 下一步

- 查完整函数看 [Audio API](../api/audio.md)。
- 看完整例子用 [Audio 范例](../case/audio-playback.md)。

[返回教程入口](README.md)
