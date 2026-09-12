# 草稿包：xge.h / Buffer（4 条 API）

> 生成 2026-09-10 02:41 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeBufferCreate
- 位置: xge.h:2086  已注释: 否
- 签名: `XGE_API int xgeBufferCreate(xge_buffer pBuffer, int iType, int iUsage, const void* pData, int iSize);`
- 实现: src/xge_buffer.c:51（体 35 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/tutorial_capture/ch100_main1.c:17; examples/tutorial_capture/ch100_main1.c:21

## xgeBufferUpdate
- 位置: xge.h:2087  已注释: 否
- 签名: `XGE_API int xgeBufferUpdate(xge_buffer pBuffer, int iOffset, const void* pData, int iSize);`
- 实现: src/xge_buffer.c:87（体 28 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_UNSUPPORTED, XGE_ERROR_GPU_FAILED, XGE_OK
- 用法: examples/tutorial_capture/ch100_main1.c:41

## xgeBufferUpload
- 位置: xge.h:2088  已注释: 否
- 签名: `XGE_API int xgeBufferUpload(xge_buffer pBuffer);`
- 实现: src/xge_buffer.c:20（体 30 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_UNSUPPORTED, XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_GPU_FAILED, XGE_OK
- 用法: examples/tutorial_capture/ch100_main1.c:25; examples/tutorial_capture/ch100_main1.c:26; examples/tutorial_capture/ch100_main1.c:42

## xgeBufferFree
- 位置: xge.h:2089  已注释: 否
- 签名: `XGE_API void xgeBufferFree(xge_buffer pBuffer);`
- 实现: src/xge_buffer.c:116（体 16 行）
- 用法: examples/tutorial_capture/ch100_main1.c:22; examples/tutorial_capture/ch100_main1.c:55; examples/tutorial_capture/ch100_main1.c:56

