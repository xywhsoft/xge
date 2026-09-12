# 草稿包：xge.h / Music（11 条 API）

> 生成 2026-09-10 02:39 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeMusicLoad
- 位置: xge.h:1783  已注释: 否
- 签名: `XGE_API int xgeMusicLoad(xge_music pMusic, const char* sPath);`
- 实现: src/xge_audio.c:400（体 4 行）
- 用法: examples/tutorial_capture/ch111_main1.c:5

## xgeMusicLoadGroup
- 位置: xge.h:1784  已注释: 否
- 签名: `XGE_API int xgeMusicLoadGroup(xge_music pMusic, const char* sPath, xge_audio_group pGroup);`
- 实现: src/xge_audio.c:405（体 4 行）

## xgeMusicPlay
- 位置: xge.h:1785  已注释: 否
- 签名: `XGE_API int xgeMusicPlay(xge_music pMusic);`
- 实现: src/xge_audio.c:410（体 4 行）
- 用法: examples/tutorial_capture/ch111_main1.c:5

## xgeMusicStop
- 位置: xge.h:1786  已注释: 否
- 签名: `XGE_API int xgeMusicStop(xge_music pMusic);`
- 实现: src/xge_audio.c:415（体 4 行）
- 用法: examples/tutorial_capture/ch111_main1.c:5

## xgeMusicPause
- 位置: xge.h:1787  已注释: 否
- 签名: `XGE_API int xgeMusicPause(xge_music pMusic);`
- 实现: src/xge_audio.c:420（体 4 行）
- 用法: examples/tutorial_capture/ch111_main1.c:5

## xgeMusicResume
- 位置: xge.h:1788  已注释: 否
- 签名: `XGE_API int xgeMusicResume(xge_music pMusic);`
- 实现: src/xge_audio.c:425（体 4 行）
- 用法: examples/tutorial_capture/ch111_main1.c:5

## xgeMusicFree
- 位置: xge.h:1789  已注释: 否
- 签名: `XGE_API void xgeMusicFree(xge_music pMusic);`
- 实现: src/xge_audio.c:430（体 4 行）
- 用法: examples/tutorial_capture/ch111_main1.c:5

## xgeMusicSetLoop
- 位置: xge.h:1790  已注释: 否
- 签名: `XGE_API void xgeMusicSetLoop(xge_music pMusic, int bLoop);`
- 实现: src/xge_audio.c:435（体 4 行）
- 用法: examples/tutorial_capture/ch111_main1.c:5

## xgeMusicSetVolume
- 位置: xge.h:1791  已注释: 否
- 签名: `XGE_API void xgeMusicSetVolume(xge_music pMusic, float fVolume);`
- 实现: src/xge_audio.c:440（体 4 行）
- 用法: examples/tutorial_capture/ch111_main1.c:5

## xgeMusicFade
- 位置: xge.h:1792  已注释: 否
- 签名: `XGE_API void xgeMusicFade(xge_music pMusic, float fFrom, float fTo, int iMilliseconds);`
- 实现: src/xge_audio.c:445（体 4 行）

## xgeMusicIsPlaying
- 位置: xge.h:1793  已注释: 否
- 签名: `XGE_API int xgeMusicIsPlaying(xge_music pMusic);`
- 实现: src/xge_audio.c:450（体 4 行）
- 用法: examples/tutorial_capture/ch111_main1.c:5; examples/tutorial_capture/ch111_main1.c:5

