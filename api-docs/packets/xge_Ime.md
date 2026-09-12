# 草稿包：xge.h / Ime（14 条 API）

> 生成 2026-09-10 02:39 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeImeGetEnabled
- 位置: xge.h:2611  已注释: 否
- 签名: `XGE_API int xgeImeGetEnabled(void);`
- 实现: src/xge_input.c:437（体 8 行）

## xgeImeSetEnabled
- 位置: xge.h:2612  已注释: 否
- 签名: `XGE_API int xgeImeSetEnabled(int bEnabled);`
- 实现: src/xge_input.c:446（体 14 行）
- 返回码: XGE_ERROR_NOT_INITIALIZED, XGE_OK, XGE_ERROR_UNSUPPORTED
- 用法: examples/tutorial_capture/ch117_main1.c:127

## xgeImeGetMode
- 位置: xge.h:2613  已注释: 否
- 签名: `XGE_API int xgeImeGetMode(void);`
- 实现: src/xge_input.c:461（体 4 行）

## xgeImeSetMode
- 位置: xge.h:2614  已注释: 否
- 签名: `XGE_API int xgeImeSetMode(int iMode);`
- 实现: src/xge_input.c:466（体 17 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_INITIALIZED, XGE_OK
- 用法: examples/xui_ime_common/main.inc.c:312

## xgeImeSetCandidatePresenterReady
- 位置: xge.h:2616  已注释: 是
- 签名: `XGE_API int xgeImeSetCandidatePresenterReady(int bReady);`
- 既有注释: /* FULL mode hides native candidate UI only while a complete presenter is ready. */
- 实现: src/xge_input.c:484（体 9 行）
- 返回码: XGE_ERROR_UNSUPPORTED

## xgeImeSetCandidateRect
- 位置: xge.h:2617  已注释: 否
- 签名: `XGE_API int xgeImeSetCandidateRect(xge_rect_t tRect);`
- 实现: src/xge_input.c:494（体 10 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_UNSUPPORTED

## xgeImeGetCandidateRect
- 位置: xge.h:2618  已注释: 否
- 签名: `XGE_API int xgeImeGetCandidateRect(xge_rect_t* pRect);`
- 实现: src/xge_input.c:505（体 12 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_FILE_NOT_FOUND, XGE_OK, XGE_ERROR_UNSUPPORTED

## xgeImeSetTextClient
- 位置: xge.h:2619  已注释: 否
- 签名: `XGE_API int xgeImeSetTextClient(const xge_ime_text_client_t* pClient);`
- 实现: src/xge_input.c:518（体 9 行）
- 返回码: XGE_ERROR_UNSUPPORTED

## xgeImeRefreshTextClient
- 位置: xge.h:2620  已注释: 否
- 签名: `XGE_API int xgeImeRefreshTextClient(void);`
- 实现: src/xge_input.c:528（体 8 行）
- 返回码: XGE_ERROR_UNSUPPORTED

## xgeImeCandidateGetInfo
- 位置: xge.h:2621  已注释: 否
- 签名: `XGE_API int xgeImeCandidateGetInfo(xge_ime_candidate_info_t* pInfo);`
- 实现: src/xge_input.c:537（体 11 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_UNSUPPORTED
- 用法: examples/xui_ime_common/main.inc.c:236; test/test_tsf_candidate_ui.c:310

## xgeImeCandidateGetText
- 位置: xge.h:2623  已注释: 是
- 签名: `XGE_API int xgeImeCandidateGetText(int iIndex, char* sText, int iCapacity);`
- 既有注释: /* Returns the complete UTF-8 byte length, excluding the terminator. */
- 实现: src/xge_input.c:549（体 9 行）
- 返回码: XGE_ERROR_UNSUPPORTED
- 用法: test/test_tsf_candidate_ui.c:315

## xgeImeCandidateSelect
- 位置: xge.h:2624  已注释: 否
- 签名: `XGE_API int xgeImeCandidateSelect(int iIndex);`
- 实现: src/xge_input.c:559（体 9 行）
- 返回码: XGE_ERROR_UNSUPPORTED
- 用法: test/test_tsf_candidate_ui.c:317

## xgeImeCandidateFinalize
- 位置: xge.h:2625  已注释: 否
- 签名: `XGE_API int xgeImeCandidateFinalize(void);`
- 实现: src/xge_input.c:569（体 8 行）
- 返回码: XGE_ERROR_UNSUPPORTED
- 用法: test/test_tsf_candidate_ui.c:319

## xgeImeEventGet
- 位置: xge.h:2627  已注释: 是
- 签名: `XGE_API int xgeImeEventGet(xge_ime_event_t* pEvent);`
- 既有注释: /* sText remains valid until the next xgeImeEventGet call or xgeUnit. */
- 实现: src/xge_input.c:403（体 33 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT

