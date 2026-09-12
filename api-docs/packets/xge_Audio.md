# 草稿包：xge.h / Audio（12 条 API）

> 生成 2026-09-10 02:39 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeAudioInit
- 位置: xge.h:1755  已注释: 否
- 签名: `XGE_API int xgeAudioInit(void);`
- 实现: src/xge_audio.c:102（体 32 行）
- 返回码: XGE_OK, XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_OUT_OF_MEMORY, XGE_ERROR_AUDIO_FAILED
- 用法: examples/tutorial_capture/ch109_main1.c:57; examples/tutorial_capture/ch110_main1.c:32; examples/tutorial_capture/ch111_main1.c:5

## xgeAudioUnit
- 位置: xge.h:1756  已注释: 否
- 签名: `XGE_API void xgeAudioUnit(void);`
- 实现: src/xge_audio.c:135（体 10 行）

## xgeAudioIsReady
- 位置: xge.h:1757  已注释: 否
- 签名: `XGE_API int xgeAudioIsReady(void);`
- 实现: src/xge_audio.c:146（体 4 行）
- 用法: examples/tutorial_capture/ch109_main1.c:68

## xgeAudioSetVolume
- 位置: xge.h:1758  已注释: 否
- 签名: `XGE_API void xgeAudioSetVolume(float fVolume);`
- 实现: src/xge_audio.c:151（体 6 行）
- 用法: examples/tutorial_capture/ch109_main1.c:59

## xgeAudioGetVolume
- 位置: xge.h:1759  已注释: 否
- 签名: `XGE_API float xgeAudioGetVolume(void);`
- 实现: src/xge_audio.c:158（体 7 行）
- 用法: examples/tutorial_capture/ch109_main1.c:68

## xgeAudioListenerSet
- 位置: xge.h:1760  已注释: 否
- 签名: `XGE_API void xgeAudioListenerSet(const xge_audio_listener_t* pListener);`
- 实现: src/xge_audio.c:166（体 12 行）
- 用法: examples/tutorial_capture/ch109_main1.c:60

## xgeAudioListenerGet
- 位置: xge.h:1761  已注释: 否
- 签名: `XGE_API xge_audio_listener_t xgeAudioListenerGet(void);`
- 实现: src/xge_audio.c:179（体 4 行）

## xgeAudioGroupInit
- 位置: xge.h:1762  已注释: 否
- 签名: `XGE_API int xgeAudioGroupInit(xge_audio_group pGroup);`
- 实现: src/xge_audio.c:184（体 23 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_OUT_OF_MEMORY, XGE_ERROR_AUDIO_FAILED, XGE_OK
- 用法: examples/tutorial_capture/ch113_main1.c:5

## xgeAudioGroupFree
- 位置: xge.h:1763  已注释: 否
- 签名: `XGE_API void xgeAudioGroupFree(xge_audio_group pGroup);`
- 实现: src/xge_audio.c:208（体 11 行）
- 用法: examples/tutorial_capture/ch113_main1.c:5

## xgeAudioGroupSetVolume
- 位置: xge.h:1764  已注释: 否
- 签名: `XGE_API void xgeAudioGroupSetVolume(xge_audio_group pGroup, float fVolume);`
- 实现: src/xge_audio.c:220（体 10 行）
- 用法: examples/tutorial_capture/ch113_main1.c:5

## xgeAudioGroupGetVolume
- 位置: xge.h:1765  已注释: 否
- 签名: `XGE_API float xgeAudioGroupGetVolume(xge_audio_group pGroup);`
- 实现: src/xge_audio.c:231（体 10 行）
- 用法: examples/tutorial_capture/ch113_main1.c:5

## xgeAudioGroupFade
- 位置: xge.h:1766  已注释: 否
- 签名: `XGE_API void xgeAudioGroupFade(xge_audio_group pGroup, float fFrom, float fTo, int iMilliseconds);`
- 实现: src/xge_audio.c:242（体 7 行）

