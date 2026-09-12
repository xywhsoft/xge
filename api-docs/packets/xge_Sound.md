# 草稿包：xge.h / Sound（16 条 API）

> 生成 2026-09-10 02:39 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeSoundLoad
- 位置: xge.h:1767  已注释: 否
- 签名: `XGE_API int xgeSoundLoad(xge_sound pSound, const char* sPath);`
- 实现: src/xge_audio.c:250（体 4 行）
- 用法: examples/tutorial_capture/ch109_main1.c:62; examples/tutorial_capture/ch110_main1.c:32

## xgeSoundLoadGroup
- 位置: xge.h:1768  已注释: 否
- 签名: `XGE_API int xgeSoundLoadGroup(xge_sound pSound, const char* sPath, xge_audio_group pGroup);`
- 实现: src/xge_audio.c:255（体 4 行）
- 用法: examples/tutorial_capture/ch113_main1.c:5

## xgeSoundFallbackSet
- 位置: xge.h:1769  已注释: 否
- 签名: `XGE_API int xgeSoundFallbackSet(const char* sPath);`
- 实现: src/xge_audio.c:260（体 15 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK

## xgeSoundFallbackGet
- 位置: xge.h:1770  已注释: 否
- 签名: `XGE_API int xgeSoundFallbackGet(xge_sound pSound);`
- 实现: src/xge_audio.c:276（体 17 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_RESOURCE_FAILED, XGE_OK

## xgeSoundFallbackClear
- 位置: xge.h:1771  已注释: 否
- 签名: `XGE_API void xgeSoundFallbackClear(void);`
- 实现: src/xge_audio.c:294（体 7 行）

## xgeSoundPlay
- 位置: xge.h:1772  已注释: 否
- 签名: `XGE_API int xgeSoundPlay(xge_sound pSound);`
- 实现: src/xge_audio.c:302（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch109_main1.c:62; examples/tutorial_capture/ch110_main1.c:32; examples/tutorial_capture/ch113_main1.c:5

## xgeSoundStop
- 位置: xge.h:1773  已注释: 否
- 签名: `XGE_API int xgeSoundStop(xge_sound pSound);`
- 实现: src/xge_audio.c:310（体 11 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_AUDIO_FAILED, XGE_OK
- 用法: examples/tutorial_capture/ch109_main1.c:64; examples/tutorial_capture/ch110_main1.c:32; examples/tutorial_capture/ch113_main1.c:5

## xgeSoundPause
- 位置: xge.h:1774  已注释: 否
- 签名: `XGE_API int xgeSoundPause(xge_sound pSound);`
- 实现: src/xge_audio.c:322（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch110_main1.c:32

## xgeSoundResume
- 位置: xge.h:1775  已注释: 否
- 签名: `XGE_API int xgeSoundResume(xge_sound pSound);`
- 实现: src/xge_audio.c:330（体 4 行）
- 用法: examples/tutorial_capture/ch110_main1.c:32

## xgeSoundAddRef
- 位置: xge.h:1776  已注释: 否
- 签名: `XGE_API int xgeSoundAddRef(xge_sound pSound);`
- 实现: src/xge_audio.c:335（体 8 行）

## xgeSoundFree
- 位置: xge.h:1777  已注释: 否
- 签名: `XGE_API void xgeSoundFree(xge_sound pSound);`
- 实现: src/xge_audio.c:344（体 18 行）
- 用法: examples/tutorial_capture/ch109_main1.c:65; examples/tutorial_capture/ch110_main1.c:32; examples/tutorial_capture/ch113_main1.c:5

## xgeSoundSetLoop
- 位置: xge.h:1778  已注释: 否
- 签名: `XGE_API void xgeSoundSetLoop(xge_sound pSound, int bLoop);`
- 实现: src/xge_audio.c:363（体 6 行）
- 用法: examples/tutorial_capture/ch110_main1.c:32

## xgeSoundSetVolume
- 位置: xge.h:1779  已注释: 否
- 签名: `XGE_API void xgeSoundSetVolume(xge_sound pSound, float fVolume);`
- 实现: src/xge_audio.c:370（体 6 行）
- 用法: examples/tutorial_capture/ch110_main1.c:32

## xgeSoundSetPosition
- 位置: xge.h:1780  已注释: 否
- 签名: `XGE_API void xgeSoundSetPosition(xge_sound pSound, float fX, float fY, float fZ);`
- 实现: src/xge_audio.c:377（体 7 行）

## xgeSoundFade
- 位置: xge.h:1781  已注释: 否
- 签名: `XGE_API void xgeSoundFade(xge_sound pSound, float fFrom, float fTo, int iMilliseconds);`
- 实现: src/xge_audio.c:385（体 6 行）

## xgeSoundIsPlaying
- 位置: xge.h:1782  已注释: 否
- 签名: `XGE_API int xgeSoundIsPlaying(xge_sound pSound);`
- 实现: src/xge_audio.c:392（体 7 行）
- 用法: examples/tutorial_capture/ch109_main1.c:63; examples/tutorial_capture/ch110_main1.c:32; examples/tutorial_capture/ch110_main1.c:32

