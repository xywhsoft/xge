# 音频播放和 3D 声源

本案例展示初始化音频系统，播放短音效、流式音乐，并设置 3D 声源位置。

[返回范例解析](README.md) | [音频教程](../guide/audio-intro.md) | [Audio API](../api/audio.md)

## 问题

游戏音频通常分为短音效、背景音乐和长流式音频。XGE 使用 miniaudio 后端，提供 sound、music、stream 三类对象，并支持 volume group、loop、fade 和 3D position。

## 步骤 1：初始化音频

```c
if ( xgeAudioInit() != XGE_OK ) {
	return XGE_ERROR_BACKEND;
}
```

如果目标平台暂时没有可用音频设备，`xgeAudioIsReady` 可用于降级判断。

## 步骤 2：创建分组

```c
static xge_audio_group_t g_sfx_group;
static xge_audio_group_t g_music_group;

xgeAudioGroupInit(&g_sfx_group);
xgeAudioGroupInit(&g_music_group);
xgeAudioGroupSetVolume(&g_sfx_group, 0.8f);
xgeAudioGroupSetVolume(&g_music_group, 0.5f);
```

分组适合做设置界面的总音量、静音、淡入淡出和场景切换。

## 步骤 3：加载并播放音效

```c
static xge_sound_t g_click;

if ( xgeSoundLoadGroup(&g_click, "assets/click.wav", &g_sfx_group) == XGE_OK ) {
	xgeSoundSetPosition(&g_click, 1.0f, 0.0f, 0.0f);
	xgeSoundPlay(&g_click);
}
```

短音效适合常驻内存。频繁播放的音效不要每次重新加载。

## 步骤 4：播放音乐

```c
static xge_music_t g_bgm;

if ( xgeMusicLoadGroup(&g_bgm, "assets/bgm.ogg", &g_music_group) == XGE_OK ) {
	xgeMusicSetLoop(&g_bgm, 1);
	xgeMusicFade(&g_bgm, 0.0f, 1.0f, 1200);
	xgeMusicPlay(&g_bgm);
}
```

长背景音乐应优先使用 music 或 stream，不要按短音效整段解码到内存。

## 步骤 5：设置监听器

```c
xge_audio_listener_t listener;

memset(&listener, 0, sizeof(listener));
listener.fX = 0.0f;
listener.fY = 0.0f;
listener.fZ = 0.0f;
listener.fForwardZ = -1.0f;
listener.fUpY = 1.0f;
xgeAudioListenerSet(&listener);
```

2D 游戏可以把 z 固定为 0，仅用 x/y 表达左右和远近；2.5D 场景可使用 z 增强空间感。

## 关键 API

| API | 作用 |
| --- | --- |
| `xgeAudioInit` | 初始化音频系统 |
| `xgeAudioGroupInit` | 创建音量分组 |
| `xgeSoundLoadGroup` | 加载短音效并绑定分组 |
| `xgeSoundPlay` | 播放短音效 |
| `xgeMusicLoadGroup` | 加载音乐并绑定分组 |
| `xgeMusicFade` | 音乐淡入淡出 |
| `xgeStreamOpen` | 打开流式音频 |
| `xgeAudioListenerSet` | 设置 3D 监听器 |

## 常见问题

如果没有声音，先检查 `xgeAudioIsReady`，再确认资源路径和平台音频权限。

如果移动端第一次播放失败，通常是平台要求用户手势后才能启动音频，应在按钮点击后调用播放接口。
