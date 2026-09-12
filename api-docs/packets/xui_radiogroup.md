# 草稿包：xui.h / radiogroup（15 条 API）

> 生成 2026-09-10 03:04 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiRadioGroupGetType
- 位置: xui.h:8282  已注释: 否
- 签名: `XUI_API xui_widget_type xuiRadioGroupGetType(xui_context pContext);`
- 实现: src/xui_radio.c:1352（体 28 行）
- 返回码: NULL

## xuiRadioGroupCreate
- 位置: xui.h:8283  已注释: 否
- 签名: `XUI_API int xuiRadioGroupCreate(xui_context pContext, xui_widget* ppWidget, const xui_radio_group_desc_t* pDesc);`
- 实现: src/xui_radio.c:1381（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch135_main1.c:14; examples/xui_checkcard/main.c:306; examples/xui_radio/main.c:197

## xuiRadioGroupSetChange
- 位置: xui.h:8284  已注释: 否
- 签名: `XUI_API int xuiRadioGroupSetChange(xui_widget pGroup, xui_radio_group_change_proc onChange, void* pUser);`
- 实现: src/xui_radio.c:1397（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_checkcard/main.c:309; examples/xui_radio/main.c:200; test_xui/xui_check_card_test.c:137

## xuiRadioGroupAddRadio
- 位置: xui.h:8285  已注释: 否
- 签名: `XUI_API int xuiRadioGroupAddRadio(xui_widget pGroup, xui_widget pRadio);`
- 实现: src/xui_radio.c:1406（体 33 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiRadioGroupAddCheckCard
- 位置: xui.h:8286  已注释: 否
- 签名: `XUI_API int xuiRadioGroupAddCheckCard(xui_widget pGroup, xui_widget pCard);`
- 实现: src/xui_radio.c:1440（体 25 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_checkcard/main.c:268; test_xui/xui_check_card_test.c:150

## xuiRadioGroupAddOption
- 位置: xui.h:8287  已注释: 否
- 签名: `XUI_API int xuiRadioGroupAddOption(xui_widget pGroup, xui_widget* ppRadio, const xui_radio_desc_t* pDesc);`
- 实现: src/xui_radio.c:1466（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_radio/main.c:224; test_xui/xui_radio_test.c:133; test_xui/xui_radio_test.c:137

## xuiRadioGroupSetSelectedIndex
- 位置: xui.h:8288  已注释: 否
- 签名: `XUI_API int xuiRadioGroupSetSelectedIndex(xui_widget pGroup, int iIndex);`
- 实现: src/xui_radio.c:1488（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_check_card_test.c:195; test_xui/xui_radio_test.c:178

## xuiRadioGroupGetSelectedIndex
- 位置: xui.h:8289  已注释: 否
- 签名: `XUI_API int xuiRadioGroupGetSelectedIndex(xui_widget pGroup);`
- 实现: src/xui_radio.c:1505（体 5 行）
- 用法: examples/xui_checkcard/main.c:395; examples/xui_checkcard/main.c:400; examples/xui_radio/main.c:323

## xuiRadioGroupGetSelectedRadio
- 位置: xui.h:8290  已注释: 否
- 签名: `XUI_API xui_widget xuiRadioGroupGetSelectedRadio(xui_widget pGroup);`
- 实现: src/xui_radio.c:1511（体 9 行）
- 返回码: NULL
- 用法: test_xui/xui_radio_test.c:179

## xuiRadioGroupSetSelectedWidget
- 位置: xui.h:8291  已注释: 否
- 签名: `XUI_API int xuiRadioGroupSetSelectedWidget(xui_widget pGroup, xui_widget pOption);`
- 实现: src/xui_radio.c:1521（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiRadioGroupGetSelectedWidget
- 位置: xui.h:8292  已注释: 否
- 签名: `XUI_API xui_widget xuiRadioGroupGetSelectedWidget(xui_widget pGroup);`
- 实现: src/xui_radio.c:1532（体 5 行）
- 用法: test_xui/xui_check_card_test.c:152

## xuiRadioGroupSetOrientation
- 位置: xui.h:8293  已注释: 否
- 签名: `XUI_API int xuiRadioGroupSetOrientation(xui_widget pGroup, int iOrientation);`
- 实现: src/xui_radio.c:1538（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_radio_test.c:180

## xuiRadioGroupGetOrientation
- 位置: xui.h:8294  已注释: 否
- 签名: `XUI_API int xuiRadioGroupGetOrientation(xui_widget pGroup);`
- 实现: src/xui_radio.c:1549（体 5 行）
- 用法: examples/xui_radio/main.c:226; test_xui/xui_radio_test.c:181

## xuiRadioGroupSetGap
- 位置: xui.h:8295  已注释: 否
- 签名: `XUI_API int xuiRadioGroupSetGap(xui_widget pGroup, float fGap);`
- 实现: src/xui_radio.c:1555（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_radio_test.c:182

## xuiRadioGroupGetGap
- 位置: xui.h:8296  已注释: 否
- 签名: `XUI_API float xuiRadioGroupGetGap(xui_widget pGroup);`
- 实现: src/xui_radio.c:1564（体 5 行）
- 用法: test_xui/xui_radio_test.c:150; test_xui/xui_radio_test.c:183

