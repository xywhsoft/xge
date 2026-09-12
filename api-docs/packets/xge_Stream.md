# 草稿包：xge.h / Stream（12 条 API）

> 生成 2026-09-10 02:39 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeStreamOpen
- 位置: xge.h:1794  已注释: 否
- 签名: `XGE_API int xgeStreamOpen(xge_stream pStream, const char* sPath);`
- 实现: src/xge_audio.c:455（体 4 行）
- 用法: examples/tutorial_capture/ch112_main1.c:5

## xgeStreamOpenGroup
- 位置: xge.h:1795  已注释: 否
- 签名: `XGE_API int xgeStreamOpenGroup(xge_stream pStream, const char* sPath, xge_audio_group pGroup);`
- 实现: src/xge_audio.c:460（体 4 行）

## xgeStreamPlay
- 位置: xge.h:1796  已注释: 否
- 签名: `XGE_API int xgeStreamPlay(xge_stream pStream);`
- 实现: src/xge_audio.c:465（体 4 行）
- 用法: examples/tutorial_capture/ch112_main1.c:5

## xgeStreamStop
- 位置: xge.h:1797  已注释: 否
- 签名: `XGE_API int xgeStreamStop(xge_stream pStream);`
- 实现: src/xge_audio.c:470（体 4 行）
- 用法: examples/tutorial_capture/ch112_main1.c:5

## xgeStreamPause
- 位置: xge.h:1798  已注释: 否
- 签名: `XGE_API int xgeStreamPause(xge_stream pStream);`
- 实现: src/xge_audio.c:475（体 4 行）
- 用法: examples/tutorial_capture/ch112_main1.c:5

## xgeStreamResume
- 位置: xge.h:1799  已注释: 否
- 签名: `XGE_API int xgeStreamResume(xge_stream pStream);`
- 实现: src/xge_audio.c:480（体 4 行）
- 用法: examples/tutorial_capture/ch112_main1.c:5

## xgeStreamClose
- 位置: xge.h:1800  已注释: 否
- 签名: `XGE_API void xgeStreamClose(xge_stream pStream);`
- 实现: src/xge_audio.c:485（体 4 行）
- 用法: examples/tutorial_capture/ch112_main1.c:5

## xgeStreamSetLoop
- 位置: xge.h:1801  已注释: 否
- 签名: `XGE_API void xgeStreamSetLoop(xge_stream pStream, int bLoop);`
- 实现: src/xge_audio.c:490（体 4 行）
- 用法: examples/tutorial_capture/ch112_main1.c:5

## xgeStreamSetVolume
- 位置: xge.h:1802  已注释: 否
- 签名: `XGE_API void xgeStreamSetVolume(xge_stream pStream, float fVolume);`
- 实现: src/xge_audio.c:495（体 4 行）
- 用法: examples/tutorial_capture/ch112_main1.c:5

## xgeStreamSetPosition
- 位置: xge.h:1803  已注释: 否
- 签名: `XGE_API void xgeStreamSetPosition(xge_stream pStream, float fX, float fY, float fZ);`
- 实现: src/xge_audio.c:500（体 4 行）
- 用法: examples/tutorial_capture/ch112_main1.c:5

## xgeStreamFade
- 位置: xge.h:1804  已注释: 否
- 签名: `XGE_API void xgeStreamFade(xge_stream pStream, float fFrom, float fTo, int iMilliseconds);`
- 实现: src/xge_audio.c:505（体 4 行）

## xgeStreamIsPlaying
- 位置: xge.h:1805  已注释: 否
- 签名: `XGE_API int xgeStreamIsPlaying(xge_stream pStream);`
- 实现: src/xge_audio.c:510（体 4 行）
- 用法: examples/tutorial_capture/ch112_main1.c:5; examples/tutorial_capture/ch112_main1.c:5

