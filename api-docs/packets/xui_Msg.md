# 草稿包：xui.h / Msg（37 条 API）

> 生成 2026-09-10 03:09 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiMsgBoxCreate
- 位置: xui.h:9593  已注释: 否
- 签名: `XUI_API int xuiMsgBoxCreate(xui_context pContext, xui_msgbox* ppBox, const xui_msgbox_desc_t* pDesc);`
- 实现: src/xui_msgbox.c:989（体 148 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/tutorial_capture/ch159_main1.c:18; examples/xui_msgbox/main.c:151; test_xui/xui_msgbox_test.c:147

## xuiMsgBoxDestroy
- 位置: xui.h:9594  已注释: 否
- 签名: `XUI_API void xuiMsgBoxDestroy(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1138（体 33 行）
- 用法: examples/xui_msgbox/main.c:255; test_xui/xui_msgbox_test.c:302

## xuiMsgBoxSetText
- 位置: xui.h:9595  已注释: 否
- 签名: `XUI_API int xuiMsgBoxSetText(xui_msgbox pBox, const char* sTitle, const char* sMessage);`
- 实现: src/xui_msgbox.c:1172（体 8 行）

## xuiMsgBoxSetTitle
- 位置: xui.h:9596  已注释: 否
- 签名: `XUI_API int xuiMsgBoxSetTitle(xui_msgbox pBox, const char* sTitle);`
- 实现: src/xui_msgbox.c:1181（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiMsgBoxGetTitle
- 位置: xui.h:9597  已注释: 否
- 签名: `XUI_API const char* xuiMsgBoxGetTitle(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1196（体 4 行）
- 用法: test_xui/xui_msgbox_test.c:151; test_xui/xui_text_consumer_display_test.c:344

## xuiMsgBoxSetMessage
- 位置: xui.h:9598  已注释: 否
- 签名: `XUI_API int xuiMsgBoxSetMessage(xui_msgbox pBox, const char* sMessage);`
- 实现: src/xui_msgbox.c:1201（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMsgBoxGetMessage
- 位置: xui.h:9599  已注释: 否
- 签名: `XUI_API const char* xuiMsgBoxGetMessage(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1214（体 4 行）

## xuiMsgBoxSetType
- 位置: xui.h:9600  已注释: 否
- 签名: `XUI_API int xuiMsgBoxSetType(xui_msgbox pBox, int iType);`
- 实现: src/xui_msgbox.c:1219（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_msgbox/main.c:207; test_xui/xui_msgbox_test.c:253

## xuiMsgBoxGetType
- 位置: xui.h:9601  已注释: 否
- 签名: `XUI_API int xuiMsgBoxGetType(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1233（体 4 行）
- 用法: test_xui/xui_msgbox_test.c:152

## xuiMsgBoxSetIconSurface
- 位置: xui.h:9602  已注释: 否
- 签名: `XUI_API int xuiMsgBoxSetIconSurface(xui_msgbox pBox, xui_surface pSurface, xui_rect_t tSrc);`
- 实现: src/xui_msgbox.c:1238（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMsgBoxUseBuiltinIcon
- 位置: xui.h:9603  已注释: 否
- 签名: `XUI_API int xuiMsgBoxUseBuiltinIcon(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1254（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMsgBoxSetButtons
- 位置: xui.h:9604  已注释: 否
- 签名: `XUI_API int xuiMsgBoxSetButtons(xui_msgbox pBox, int iButtons);`
- 实现: src/xui_msgbox.c:1266（体 10 行）
- 用法: test_xui/xui_msgbox_test.c:282

## xuiMsgBoxSetCustomButtons
- 位置: xui.h:9605  已注释: 否
- 签名: `XUI_API int xuiMsgBoxSetCustomButtons(xui_msgbox pBox, const xui_msgbox_button_t* pButtons, int iCount);`
- 实现: src/xui_msgbox.c:1277（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_msgbox/main.c:208; test_xui/xui_msgbox_test.c:255

## xuiMsgBoxSetPresetButtonTitle
- 位置: xui.h:9606  已注释: 否
- 签名: `XUI_API int xuiMsgBoxSetPresetButtonTitle(xui_msgbox pBox, int iTitle, const char* sText);`
- 实现: src/xui_msgbox.c:1296（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiMsgBoxGetPresetButtonTitle
- 位置: xui.h:9607  已注释: 否
- 签名: `XUI_API const char* xuiMsgBoxGetPresetButtonTitle(xui_msgbox pBox, int iTitle);`
- 实现: src/xui_msgbox.c:1328（体 7 行）
- 返回码: NULL

## xuiMsgBoxSetResult
- 位置: xui.h:9608  已注释: 否
- 签名: `XUI_API int xuiMsgBoxSetResult(xui_msgbox pBox, xui_msgbox_result_proc onResult, void* pUser);`
- 实现: src/xui_msgbox.c:1336（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_msgbox/main.c:153; test_xui/xui_msgbox_test.c:149

## xuiMsgBoxSetModal
- 位置: xui.h:9609  已注释: 否
- 签名: `XUI_API int xuiMsgBoxSetModal(xui_msgbox pBox, int bModal);`
- 实现: src/xui_msgbox.c:1346（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMsgBoxIsModal
- 位置: xui.h:9610  已注释: 否
- 签名: `XUI_API int xuiMsgBoxIsModal(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1356（体 4 行）

## xuiMsgBoxSetOpen
- 位置: xui.h:9611  已注释: 否
- 签名: `XUI_API int xuiMsgBoxSetOpen(xui_msgbox pBox, int bOpen);`
- 实现: src/xui_msgbox.c:1361（体 38 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/tutorial_capture/ch159_main1.c:20; examples/xui_msgbox/main.c:155; examples/xui_msgbox/main.c:209

## xuiMsgBoxIsOpen
- 位置: xui.h:9612  已注释: 否
- 签名: `XUI_API int xuiMsgBoxIsOpen(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1400（体 4 行）
- 用法: examples/xui_msgbox/main.c:200; examples/xui_msgbox/main.c:215; test_xui/xui_file_dialog_test.c:412

## xuiMsgBoxGetResult
- 位置: xui.h:9613  已注释: 否
- 签名: `XUI_API int xuiMsgBoxGetResult(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1405（体 4 行）
- 用法: test_xui/xui_msgbox_test.c:233; test_xui/xui_msgbox_test.c:244; test_xui/xui_msgbox_test.c:279

## xuiMsgBoxSetMetrics
- 位置: xui.h:9614  已注释: 否
- 签名: `XUI_API int xuiMsgBoxSetMetrics(xui_msgbox pBox, const xui_msgbox_metrics_t* pMetrics);`
- 实现: src/xui_msgbox.c:1410（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMsgBoxGetMetrics
- 位置: xui.h:9615  已注释: 否
- 签名: `XUI_API int xuiMsgBoxGetMetrics(xui_msgbox pBox, xui_msgbox_metrics_t* pMetrics);`
- 实现: src/xui_msgbox.c:1421（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_msgbox_test.c:263

## xuiMsgBoxSetColors
- 位置: xui.h:9616  已注释: 否
- 签名: `XUI_API int xuiMsgBoxSetColors(xui_msgbox pBox, const xui_msgbox_colors_t* pColors);`
- 实现: src/xui_msgbox.c:1431（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMsgBoxGetColors
- 位置: xui.h:9617  已注释: 否
- 签名: `XUI_API int xuiMsgBoxGetColors(xui_msgbox pBox, xui_msgbox_colors_t* pColors);`
- 实现: src/xui_msgbox.c:1452（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiMsgBoxGetWindowWidget
- 位置: xui.h:9618  已注释: 否
- 签名: `XUI_API xui_widget xuiMsgBoxGetWindowWidget(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1462（体 4 行）
- 用法: examples/xui_msgbox/main.c:196; test_xui/xui_msgbox_test.c:154; test_xui/xui_msgbox_test.c:164

## xuiMsgBoxGetContentWidget
- 位置: xui.h:9619  已注释: 否
- 签名: `XUI_API xui_widget xuiMsgBoxGetContentWidget(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1467（体 4 行）
- 用法: examples/xui_msgbox/main.c:212; test_xui/xui_msgbox_test.c:155; test_xui/xui_msgbox_test.c:171

## xuiMsgBoxGetBackdropWidget
- 位置: xui.h:9620  已注释: 否
- 签名: `XUI_API xui_widget xuiMsgBoxGetBackdropWidget(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1472（体 4 行）
- 用法: test_xui/xui_msgbox_test.c:156; test_xui/xui_msgbox_test.c:175

## xuiMsgBoxGetButtonWidget
- 位置: xui.h:9621  已注释: 否
- 签名: `XUI_API xui_widget xuiMsgBoxGetButtonWidget(xui_msgbox pBox, int iIndex);`
- 实现: src/xui_msgbox.c:1477（体 7 行）
- 返回码: NULL
- 用法: test_xui/xui_file_dialog_test.c:413; test_xui/xui_file_dialog_test.c:417; test_xui/xui_msgbox_test.c:173

## xuiMsgBoxGetButtonCount
- 位置: xui.h:9622  已注释: 否
- 签名: `XUI_API int xuiMsgBoxGetButtonCount(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1485（体 4 行）
- 用法: examples/xui_msgbox/main.c:191; test_xui/xui_msgbox_test.c:153; test_xui/xui_msgbox_test.c:256

## xuiMsgBoxGetIconRect
- 位置: xui.h:9623  已注释: 否
- 签名: `XUI_API xui_rect_t xuiMsgBoxGetIconRect(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1490（体 4 行）
- 用法: examples/xui_msgbox/main.c:192; test_xui/xui_msgbox_test.c:177

## xuiMsgBoxGetMessageRect
- 位置: xui.h:9624  已注释: 否
- 签名: `XUI_API xui_rect_t xuiMsgBoxGetMessageRect(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1495（体 4 行）
- 用法: examples/xui_msgbox/main.c:193; test_xui/xui_msgbox_test.c:178

## xuiMsgBoxGetButtonRect
- 位置: xui.h:9625  已注释: 否
- 签名: `XUI_API xui_rect_t xuiMsgBoxGetButtonRect(xui_msgbox pBox, int iIndex);`
- 实现: src/xui_msgbox.c:1500（体 7 行）
- 用法: examples/xui_msgbox/main.c:211; test_xui/xui_msgbox_test.c:240; test_xui/xui_msgbox_test.c:261

## xuiMsgBoxGetBackdropRect
- 位置: xui.h:9626  已注释: 否
- 签名: `XUI_API xui_rect_t xuiMsgBoxGetBackdropRect(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1508（体 4 行）
- 用法: test_xui/xui_msgbox_test.c:162; test_xui/xui_msgbox_test.c:195

## xuiMsgBoxGetWrapLineCount
- 位置: xui.h:9627  已注释: 否
- 签名: `XUI_API int xuiMsgBoxGetWrapLineCount(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1513（体 4 行）
- 用法: examples/xui_msgbox/main.c:194; test_xui/xui_msgbox_test.c:181

## xuiMsgBoxGetResultCount
- 位置: xui.h:9628  已注释: 否
- 签名: `XUI_API int xuiMsgBoxGetResultCount(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1518（体 4 行）
- 用法: test_xui/xui_msgbox_test.c:294

## xuiMsgBoxGetChangeCount
- 位置: xui.h:9629  已注释: 否
- 签名: `XUI_API int xuiMsgBoxGetChangeCount(xui_msgbox pBox);`
- 实现: src/xui_msgbox.c:1523（体 4 行）
- 用法: test_xui/xui_msgbox_test.c:295

