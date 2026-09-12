# 草稿包：xui.h / Drag（6 条 API）

> 生成 2026-09-10 02:52 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiDragBegin
- 位置: xui.h:5837  已注释: 否
- 签名: `XUI_API int xuiDragBegin(xui_context pContext, xui_widget pSource, xui_data_object pData, uint32_t iAllowedEffects, uint32_t iSuggestedEffect);`
- 实现: src/xui_drag_drop.c:401（体 9 行）
- 用法: test_xui/xui_drag_drop_reentry_test.c:104; test_xui/xui_drag_drop_reentry_test.c:201; test_xui/xui_drag_drop_reentry_test.c:260

## xuiDragAccept
- 位置: xui.h:5839  已注释: 否
- 签名: `XUI_API int xuiDragAccept(xui_context pContext, uint32_t iEffect);`
- 实现: src/xui_drag_drop.c:411（体 16 行）
- 返回码: XUI_ERROR_INVALID_STATE, XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_drag_drop_reentry_test.c:133; test_xui/xui_drag_drop_reentry_test.c:143; test_xui/xui_drag_drop_reentry_test.c:160

## xuiDragCancel
- 位置: xui.h:5840  已注释: 否
- 签名: `XUI_API int xuiDragCancel(xui_context pContext);`
- 实现: src/xui_drag_drop.c:532（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_drag_drop_reentry_test.c:154; test_xui/xui_drag_drop_reentry_test.c:158; test_xui/xui_drag_drop_reentry_test.c:280

## xuiDragIsActive
- 位置: xui.h:5841  已注释: 否
- 签名: `XUI_API int xuiDragIsActive(xui_context pContext);`
- 实现: src/xui_drag_drop.c:539（体 4 行）
- 用法: test_xui/xui_drag_drop_reentry_test.c:307; test_xui/xui_drag_drop_reentry_test.c:319; test_xui/xui_drag_drop_reentry_test.c:321

## xuiDragGetEffect
- 位置: xui.h:5842  已注释: 否
- 签名: `XUI_API uint32_t xuiDragGetEffect(xui_context pContext);`
- 实现: src/xui_drag_drop.c:544（体 6 行）
- 返回码: XUI_DRAG_EFFECT_NONE
- 用法: test_xui/xui_drag_drop_reentry_test.c:310; test_xui/xui_drag_drop_reentry_test.c:316; test_xui/xui_drag_drop_reentry_test.c:338

## xuiDragExternalEvent
- 位置: xui.h:5843  已注释: 否
- 签名: `XUI_API int xuiDragExternalEvent(xui_context pContext, int iType, int iX, int iY, uint32_t iModifiers, xui_data_object pData, uint32_t iAllowedEffects, uint32_t iSuggestedEffect, uint32_t* pEffect);`
- 实现: src/xui_drag_drop.c:610（体 11 行）
- 用法: test_xui/xui_drag_drop_reentry_test.c:84; test_xui/xui_drag_drop_reentry_test.c:263; test_xui/xui_drag_drop_reentry_test.c:282

