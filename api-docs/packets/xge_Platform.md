# 草稿包：xge.h / Platform（6 条 API）

> 生成 2026-09-10 02:41 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgePlatformBackendDefault
- 位置: xge.h:1611  已注释: 否
- 签名: `XGE_API xge_platform_backend_t xgePlatformBackendDefault(void);`
- 实现: src/xge_core.c:340（体 9 行）

## xgePlatformBackendSet
- 位置: xge.h:1612  已注释: 否
- 签名: `XGE_API int xgePlatformBackendSet(const xge_platform_backend_t* pBackend);`
- 实现: src/xge_core.c:350（体 11 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgePlatformBackendGet
- 位置: xge.h:1613  已注释: 否
- 签名: `XGE_API xge_platform_backend_t xgePlatformBackendGet(void);`
- 实现: src/xge_core.c:362（体 7 行）
- 用法: examples/tutorial_capture/ch10_main1.c:9

## xgePlatformCapsGet
- 位置: xge.h:1618  已注释: 否
- 签名: `XGE_API int xgePlatformCapsGet(xge_platform_caps_t* pCaps);`
- 实现: src/xge_core.c:432（体 65 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch09_main1.c:19

## xgePlatformRuntimeGet
- 位置: xge.h:1619  已注释: 否
- 签名: `XGE_API int xgePlatformRuntimeGet(xge_platform_runtime_t* pRuntime);`
- 实现: src/xge_core.c:498（体 18 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_INITIALIZED, XGE_OK

## xgePlatformNativeHandle
- 位置: xge.h:1622  已注释: 是
- 签名: `XGE_API void* xgePlatformNativeHandle(void);`
- 既有注释: /* Borrowed backend-native handle for the primary XGE window. The concrete * type is platform-specific; callers own all casts and native operations. */
- 实现: src/xge_core.c:517（体 25 行）
- 返回码: NULL
- 用法: test/test_platform_native_handle.c:18; test/test_platform_native_handle.c:26; test/test_platform_native_handle.c:28

