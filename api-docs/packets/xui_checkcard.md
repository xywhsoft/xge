# 草稿包：xui.h / checkcard（17 条 API）

> 生成 2026-09-10 02:57 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiCheckCardGetType
- 位置: xui.h:7777  已注释: 否
- 签名: `XUI_API xui_widget_type xuiCheckCardGetType(xui_context pContext);`
- 实现: src/xui_check_card.c:469（体 28 行）
- 返回码: NULL

## xuiCheckCardCreate
- 位置: xui.h:7778  已注释: 否
- 签名: `XUI_API int xuiCheckCardCreate(xui_context pContext, xui_widget* ppWidget, const xui_check_card_desc_t* pDesc);`
- 实现: src/xui_check_card.c:498（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch137_main1.c:14; examples/tutorial_capture/ch137_main1.c:21; examples/xui_checkcard/main.c:261

## xuiCheckCardSetChange
- 位置: xui.h:7779  已注释: 否
- 签名: `XUI_API int xuiCheckCardSetChange(xui_widget pWidget, xui_check_card_change_proc onChange, void* pUser);`
- 实现: src/xui_check_card.c:509（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_checkcard/main.c:265; test_xui/xui_check_card_test.c:124

## xuiCheckCardSetChecked
- 位置: xui.h:7780  已注释: 否
- 签名: `XUI_API int xuiCheckCardSetChecked(xui_widget pWidget, int bChecked);`
- 实现: src/xui_check_card.c:518（体 5 行）

## xuiCheckCardSetCheckedFromGroup
- 位置: xui.h:7781  已注释: 否
- 签名: `XUI_API int xuiCheckCardSetCheckedFromGroup(xui_widget pWidget, int bChecked, int bNotify);`
- 实现: src/xui_check_card.c:524（体 5 行）

## xuiCheckCardGetChecked
- 位置: xui.h:7782  已注释: 否
- 签名: `XUI_API int xuiCheckCardGetChecked(xui_widget pWidget);`
- 实现: src/xui_check_card.c:530（体 5 行）
- 用法: examples/xui_checkcard/main.c:389; examples/xui_checkcard/main.c:390; examples/xui_checkcard/main.c:392

## xuiCheckCardSetRadioGroup
- 位置: xui.h:7785  已注释: 是
- 签名: `XUI_API int xuiCheckCardSetRadioGroup(xui_widget pWidget, xui_widget pGroup);`
- 既有注释: /* Assigning a radio group reparents the card to that group. Reparenting the * card elsewhere clears the association. */
- 实现: src/xui_check_card.c:536（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCheckCardGetRadioGroup
- 位置: xui.h:7786  已注释: 否
- 签名: `XUI_API xui_widget xuiCheckCardGetRadioGroup(xui_widget pWidget);`
- 实现: src/xui_check_card.c:556（体 9 行）

## xuiCheckCardSetMetrics
- 位置: xui.h:7787  已注释: 否
- 签名: `XUI_API int xuiCheckCardSetMetrics(xui_widget pWidget, float fBorderWidth, float fCheckedBorderWidth, float fCornerSize, float fFocusWidth);`
- 实现: src/xui_check_card.c:566（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_check_card_test.c:197

## xuiCheckCardGetMetrics
- 位置: xui.h:7788  已注释: 否
- 签名: `XUI_API int xuiCheckCardGetMetrics(xui_widget pWidget, float* pBorderWidth, float* pCheckedBorderWidth, float* pCornerSize, float* pFocusWidth);`
- 实现: src/xui_check_card.c:574（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_check_card_test.c:199

## xuiCheckCardSetColors
- 位置: xui.h:7789  已注释: 否
- 签名: `XUI_API int xuiCheckCardSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iHoverBackground, uint32_t iActiveBackground, uint32_t iCheckedBackground, uint32_t iBorder, uint32_t iHoverBorder, uint32_t iCheckedBorder, uint32_t iCorner, uint32_t iCheck);`
- 实现: src/xui_check_card.c:585（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_check_card_test.c:201

## xuiCheckCardGetColors
- 位置: xui.h:7790  已注释: 否
- 签名: `XUI_API int xuiCheckCardGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pHoverBackground, uint32_t* pActiveBackground, uint32_t* pCheckedBackground, uint32_t* pBorder, uint32_t* pHoverBorder, uint32_t* pCheckedBorder, uint32_t* pCorner, uint32_t* pCheck);`
- 实现: src/xui_check_card.c:594（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCheckCardSetStateColors
- 位置: xui.h:7791  已注释: 否
- 签名: `XUI_API int xuiCheckCardSetStateColors(xui_widget pWidget, uint32_t iDisabledBorder, uint32_t iFocus);`
- 实现: src/xui_check_card.c:610（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_basic_buttons_test.c:63

## xuiCheckCardGetStateColors
- 位置: xui.h:7792  已注释: 否
- 签名: `XUI_API int xuiCheckCardGetStateColors(xui_widget pWidget, uint32_t* pDisabledBorder, uint32_t* pFocus);`
- 实现: src/xui_check_card.c:619（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCheckCardGetCornerRect
- 位置: xui.h:7793  已注释: 否
- 签名: `XUI_API xui_rect_t xuiCheckCardGetCornerRect(xui_widget pWidget);`
- 实现: src/xui_check_card.c:628（体 5 行）
- 用法: test_xui/xui_check_card_test.c:180

## xuiCheckCardGetState
- 位置: xui.h:7794  已注释: 否
- 签名: `XUI_API uint32_t xuiCheckCardGetState(xui_widget pWidget);`
- 实现: src/xui_check_card.c:634（体 5 行）

## xuiCheckCardGetChangeCount
- 位置: xui.h:7795  已注释: 否
- 签名: `XUI_API int xuiCheckCardGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_check_card.c:640（体 5 行）

