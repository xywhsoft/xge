# 草稿包：xge.h / Drag（6 条 API）

> 生成 2026-09-10 02:40 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeDragDropCapsGet
- 位置: xge.h:1623  已注释: 否
- 签名: `XGE_API int xgeDragDropCapsGet(xge_drag_drop_caps_t* pCaps);`
- 实现: src/xge_drag_drop.c:253（体 19 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_drag_data_object.c:57

## xgeDragEventCallbackSet
- 位置: xge.h:1637  已注释: 否
- 签名: `XGE_API int xgeDragEventCallbackSet(xge_drag_event_proc onEvent, void* pUser);`
- 实现: src/xge_drag_drop.c:273（体 6 行）
- 返回码: XGE_OK
- 用法: test/test_drag_data_object.c:58

## xgeDragEventDispatch
- 位置: xge.h:1640  已注释: 是
- 签名: `XGE_API uint32_t xgeDragEventDispatch(const xge_drag_event_t* pEvent);`
- 既有注释: /* Dispatches a platform drag event synchronously. Custom window backends use * this entry point so effect negotiation completes before returning to the OS. */
- 实现: src/xge_drag_drop.c:280（体 32 行）
- 返回码: XGE_DRAG_EFFECT_NONE
- 用法: test_xui/xui_proxy_drag_drop_test.c:87; test_xui/xui_proxy_drag_drop_test.c:91

## xgeDragBegin
- 位置: xge.h:1641  已注释: 否
- 签名: `XGE_API int xgeDragBegin(xge_data_object pData, uint32_t iAllowedEffects, uint32_t iSuggestedEffect);`
- 实现: src/xge_drag_drop.c:313（体 20 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: test/test_drag_data_object.c:59

## xgeDragCancel
- 位置: xge.h:1643  已注释: 否
- 签名: `XGE_API int xgeDragCancel(void);`
- 实现: src/xge_drag_drop.c:334（体 17 行）
- 返回码: XGE_OK
- 用法: test/test_drag_data_object.c:62

## xgeDragIsActive
- 位置: xge.h:1644  已注释: 否
- 签名: `XGE_API int xgeDragIsActive(void);`
- 实现: src/xge_drag_drop.c:352（体 4 行）
- 用法: test/test_drag_data_object.c:61; test/test_drag_data_object.c:62

