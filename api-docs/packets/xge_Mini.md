# 草稿包：xge.h / Mini（11 条 API）

> 生成 2026-09-10 02:41 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeMiniProgramInit
- 位置: xge.h:1746  已注释: 否
- 签名: `XGE_API int xgeMiniProgramInit(const xge_miniprogram_desc_t* pDesc);`
- 实现: src/xge_miniprogram.c:166（体 39 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeMiniProgramInitSimple
- 位置: xge.h:1747  已注释: 否
- 签名: `XGE_API int xgeMiniProgramInitSimple(int iWidth, int iHeight, float fDevicePixelRatio);`
- 实现: src/xge_miniprogram.c:206（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: test/test_native_input_coordinates.c:261

## xgeMiniProgramUnit
- 位置: xge.h:1748  已注释: 否
- 签名: `XGE_API void xgeMiniProgramUnit(void);`
- 实现: src/xge_miniprogram.c:220（体 8 行）
- 用法: test/test_native_input_coordinates.c:288

## xgeMiniProgramSetBridge
- 位置: xge.h:1749  已注释: 否
- 签名: `XGE_API int xgeMiniProgramSetBridge(const xge_miniprogram_bridge_t* pBridge);`
- 实现: src/xge_miniprogram.c:229（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_native_input_coordinates.c:260; test/test_native_input_coordinates.c:275

## xgeMiniProgramFrame
- 位置: xge.h:1751  已注释: 是
- 签名: `XGE_API int xgeMiniProgramFrame(double fTimeSeconds);`
- 既有注释: /* fTimeSeconds is a finite, monotonic host timestamp in seconds. */
- 实现: src/xge_miniprogram.c:243（体 14 行）
- 返回码: XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_INVALID_ARGUMENT
- 用法: test/test_native_input_coordinates.c:286; test/test_native_input_coordinates.c:286

## xgeMiniProgramResize
- 位置: xge.h:1752  已注释: 否
- 签名: `XGE_API int xgeMiniProgramResize(int iWidth, int iHeight, float fDevicePixelRatio);`
- 实现: src/xge_miniprogram.c:258（体 18 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_native_input_coordinates.c:284

## xgeMiniProgramTouch
- 位置: xge.h:1753  已注释: 否
- 签名: `XGE_API int xgeMiniProgramTouch(int iPhase, const xge_miniprogram_touch_t* pTouches, int iCount);`
- 实现: src/xge_miniprogram.c:277（体 87 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeMiniProgramTouchOne
- 位置: xge.h:1754  已注释: 否
- 签名: `XGE_API int xgeMiniProgramTouchOne(int iPhase, int iId, float fX, float fY, float fForce);`
- 实现: src/xge_miniprogram.c:365（体 14 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: test/test_native_input_coordinates.c:265

## xgeMiniProgramText
- 位置: xge.h:1755  已注释: 否
- 签名: `XGE_API int xgeMiniProgramText(uint32_t iCodepoint);`
- 实现: src/xge_miniprogram.c:380（体 20 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: test_xui/xui_proxy_xge_test.c:687; test_xui/xui_proxy_xge_test.c:688

## xgeMiniProgramRequestFrame
- 位置: xge.h:1756  已注释: 否
- 签名: `XGE_API int xgeMiniProgramRequestFrame(void);`
- 实现: src/xge_miniprogram.c:401（体 10 行）
- 返回码: XGE_ERROR_UNSUPPORTED

## xgeMiniProgramAudioCommand
- 位置: xge.h:1757  已注释: 否
- 签名: `XGE_API int xgeMiniProgramAudioCommand(int iCommand, int iHandle, const void* pData, int iSize);`
- 实现: src/xge_miniprogram.c:412（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_UNSUPPORTED

