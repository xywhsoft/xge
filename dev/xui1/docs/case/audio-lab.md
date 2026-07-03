# Audio Lab

`examples/audio_lab` 是音频 API 的覆盖型范例。它会在运行时生成一个短 WAV 文件，默认用这份文件覆盖 sound、music、stream、group、listener、fade、fallback 和 3D position；也支持传入外部音频文件。

## 覆盖 API

- `xgeAudioInit`
- `xgeAudioUnit`
- `xgeAudioIsReady`
- `xgeAudioSetVolume`
- `xgeAudioGetVolume`
- `xgeAudioListenerSet`
- `xgeAudioListenerGet`
- `xgeAudioGroupInit`
- `xgeAudioGroupFree`
- `xgeAudioGroupSetVolume`
- `xgeAudioGroupGetVolume`
- `xgeAudioGroupFade`
- `xgeSoundLoad`
- `xgeSoundLoadGroup`
- `xgeSoundFallbackSet`
- `xgeSoundFallbackGet`
- `xgeSoundFallbackClear`
- `xgeSoundPlay`
- `xgeSoundStop`
- `xgeSoundPause`
- `xgeSoundResume`
- `xgeSoundAddRef`
- `xgeSoundFree`
- `xgeSoundSetLoop`
- `xgeSoundSetVolume`
- `xgeSoundSetPosition`
- `xgeSoundFade`
- `xgeSoundIsPlaying`
- `xgeMusicLoad`
- `xgeMusicLoadGroup`
- `xgeMusicPlay`
- `xgeMusicStop`
- `xgeMusicPause`
- `xgeMusicResume`
- `xgeMusicFree`
- `xgeMusicSetLoop`
- `xgeMusicSetVolume`
- `xgeMusicFade`
- `xgeMusicIsPlaying`
- `xgeStreamOpen`
- `xgeStreamOpenGroup`
- `xgeStreamPlay`
- `xgeStreamStop`
- `xgeStreamPause`
- `xgeStreamResume`
- `xgeStreamClose`
- `xgeStreamSetLoop`
- `xgeStreamSetVolume`
- `xgeStreamSetPosition`
- `xgeStreamFade`
- `xgeStreamIsPlaying`

## 构建和运行

```bat
examples\audio_lab\build.bat
build\xge_audio_lab.exe --frames 120
```

可选参数：

```bat
build\xge_audio_lab.exe --sound path\to\click.wav --music path\to\music.ogg --stream path\to\voice.mp3
```

如果音频设备或后端不可用，程序会输出 `audio-lab skip` 并正常退出。

## 验证点

- 控制台输出 `audio-lab summary`。
- `ready=1` 表示音频引擎初始化成功。
- `fallback=(1,1,1)` 表示 fallback 设置、缺失文件回退和 fallback 复制都通过。
- `sound=(load=1 group=1 addref=1 play=1 pause=1 resume=1 stop=1)` 表示 sound 路径完整通过。
- `music=(load=1 group=1 play=1 pause=1 resume=1 stop=1)` 表示 music 路径完整通过。
- `stream=(load=1 group=1 play=1 pause=1 resume=1 stop=1)` 表示 stream 路径完整通过。
