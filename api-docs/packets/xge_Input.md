# 草稿包：xge.h / Input（6 条 API）

> 生成 2026-09-10 02:39 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeInputConsumeKey
- 位置: xge.h:2594  已注释: 否
- 签名: `XGE_API void xgeInputConsumeKey(int iKey);`
- 实现: src/xge_input.c:233（体 7 行）

## xgeInputKeyConsumed
- 位置: xge.h:2595  已注释: 否
- 签名: `XGE_API int xgeInputKeyConsumed(int iKey);`
- 实现: src/xge_input.c:241（体 7 行）

## xgeInputEventGet
- 位置: xge.h:2607  已注释: 是
- 签名: `XGE_API int xgeInputEventGet(xge_input_event_t* pEvent);`
- 既有注释: /* * Ordered input is the preferred API for UI integrations. The legacy key, * text and IME getters remain available as compatibility views; consumers * should choose one model and must not expect two independent deliveries. */
- 实现: src/xge_input.c:304（体 50 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch115_main1.c:25; examples/tutorial_capture/ch116_main1.c:10; examples/tutorial_capture/ch117_main1.c:91

## xgeInputEventPost
- 位置: xge.h:2608  已注释: 否
- 签名: `XGE_API int xgeInputEventPost(const xge_input_event_t* pEvent);`
- 实现: src/xge_input.c:355（体 37 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch115_main1.c:17; examples/tutorial_capture/ch116_main1.c:9; examples/tutorial_capture/ch117_main1.c:75

## xgeInputEventPendingCount
- 位置: xge.h:2609  已注释: 否
- 签名: `XGE_API int xgeInputEventPendingCount(void);`
- 实现: src/xge_input.c:393（体 4 行）
- 用法: test/test_ime_mode_contract.c:34; test/test_ime_mode_contract.c:37; test/test_input_event_queue.c:97

## xgeInputEventDroppedCount
- 位置: xge.h:2610  已注释: 否
- 签名: `XGE_API uint64_t xgeInputEventDroppedCount(void);`
- 实现: src/xge_input.c:398（体 4 行）
- 用法: test/test_input_event_queue.c:140

