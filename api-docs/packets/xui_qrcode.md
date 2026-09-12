# 草稿包：xui.h / qrcode（20 条 API）

> 生成 2026-09-10 03:03 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiQrCodeGetType
- 位置: xui.h:6515  已注释: 否
- 签名: `XUI_API xui_widget_type xuiQrCodeGetType(xui_context pContext);`
- 实现: src/xui_qrcode.c:1036（体 33 行）
- 返回码: NULL

## xuiQrCodeCreate
- 位置: xui.h:6516  已注释: 否
- 签名: `XUI_API int xuiQrCodeCreate(xui_context pContext, xui_widget* ppWidget, const xui_qrcode_desc_t* pDesc);`
- 实现: src/xui_qrcode.c:1070（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch189_main1.c:16; examples/xui_qrcode/main.c:168; test_xui/xui_qrcode_test.c:58

## xuiQrCodeSetValue
- 位置: xui.h:6517  已注释: 否
- 签名: `XUI_API int xuiQrCodeSetValue(xui_widget pWidget, const char* sValue);`
- 实现: src/xui_qrcode.c:1085（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_qrcode/main.c:216; test_xui/xui_qrcode_test.c:86

## xuiQrCodeGetValue
- 位置: xui.h:6518  已注释: 否
- 签名: `XUI_API const char* xuiQrCodeGetValue(xui_widget pWidget);`
- 实现: src/xui_qrcode.c:1103（体 7 行）
- 用法: test_xui/xui_qrcode_test.c:90

## xuiQrCodeSetColors
- 位置: xui.h:6519  已注释: 否
- 签名: `XUI_API int xuiQrCodeSetColors(xui_widget pWidget, uint32_t iForegroundColor, uint32_t iBackgroundColor);`
- 实现: src/xui_qrcode.c:1111（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_qrcode/main.c:219; test_xui/xui_qrcode_test.c:73

## xuiQrCodeSetForegroundColor
- 位置: xui.h:6520  已注释: 否
- 签名: `XUI_API int xuiQrCodeSetForegroundColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_qrcode.c:1122（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiQrCodeGetForegroundColor
- 位置: xui.h:6521  已注释: 否
- 签名: `XUI_API uint32_t xuiQrCodeGetForegroundColor(xui_widget pWidget);`
- 实现: src/xui_qrcode.c:1132（体 7 行）
- 用法: test_xui/xui_qrcode_test.c:75

## xuiQrCodeSetBackgroundColor
- 位置: xui.h:6522  已注释: 否
- 签名: `XUI_API int xuiQrCodeSetBackgroundColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_qrcode.c:1140（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiQrCodeGetBackgroundColor
- 位置: xui.h:6523  已注释: 否
- 签名: `XUI_API uint32_t xuiQrCodeGetBackgroundColor(xui_widget pWidget);`
- 实现: src/xui_qrcode.c:1150（体 7 行）
- 用法: test_xui/xui_qrcode_test.c:76

## xuiQrCodeSetPadding
- 位置: xui.h:6524  已注释: 否
- 签名: `XUI_API int xuiQrCodeSetPadding(xui_widget pWidget, float fPadding);`
- 实现: src/xui_qrcode.c:1158（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_qrcode_test.c:77

## xuiQrCodeGetPadding
- 位置: xui.h:6525  已注释: 否
- 签名: `XUI_API float xuiQrCodeGetPadding(xui_widget pWidget);`
- 实现: src/xui_qrcode.c:1168（体 7 行）
- 用法: test_xui/xui_qrcode_test.c:78

## xuiQrCodeSetIcon
- 位置: xui.h:6526  已注释: 否
- 签名: `XUI_API int xuiQrCodeSetIcon(xui_widget pWidget, xui_surface pIconSurface, xui_rect_t tIconSrc, float fIconSize);`
- 实现: src/xui_qrcode.c:1176（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_qrcode_test.c:82

## xuiQrCodeGetIcon
- 位置: xui.h:6527  已注释: 否
- 签名: `XUI_API xui_surface xuiQrCodeGetIcon(xui_widget pWidget);`
- 实现: src/xui_qrcode.c:1188（体 7 行）
- 用法: examples/xui_qrcode/main.c:212; test_xui/xui_qrcode_test.c:84

## xuiQrCodeGetIconSource
- 位置: xui.h:6528  已注释: 否
- 签名: `XUI_API xui_rect_t xuiQrCodeGetIconSource(xui_widget pWidget);`
- 实现: src/xui_qrcode.c:1196（体 7 行）

## xuiQrCodeGetIconSize
- 位置: xui.h:6529  已注释: 否
- 签名: `XUI_API float xuiQrCodeGetIconSize(xui_widget pWidget);`
- 实现: src/xui_qrcode.c:1204（体 7 行）
- 用法: test_xui/xui_qrcode_test.c:84

## xuiQrCodeSetVersionRange
- 位置: xui.h:6530  已注释: 否
- 签名: `XUI_API int xuiQrCodeSetVersionRange(xui_widget pWidget, int iMinVersion, int iMaxVersion);`
- 实现: src/xui_qrcode.c:1212（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_qrcode_test.c:88

## xuiQrCodeGetVersion
- 位置: xui.h:6531  已注释: 否
- 签名: `XUI_API int xuiQrCodeGetVersion(xui_widget pWidget);`
- 实现: src/xui_qrcode.c:1234（体 7 行）
- 用法: examples/xui_qrcode/main.c:323; test_xui/xui_qrcode_test.c:64; test_xui/xui_qrcode_test.c:89

## xuiQrCodeGetModuleCount
- 位置: xui.h:6532  已注释: 否
- 签名: `XUI_API int xuiQrCodeGetModuleCount(xui_widget pWidget);`
- 实现: src/xui_qrcode.c:1242（体 7 行）
- 用法: examples/xui_qrcode/main.c:209; examples/xui_qrcode/main.c:218; examples/xui_qrcode/main.c:323

## xuiQrCodeGetModule
- 位置: xui.h:6533  已注释: 否
- 签名: `XUI_API int xuiQrCodeGetModule(xui_widget pWidget, int iX, int iY);`
- 实现: src/xui_qrcode.c:1250（体 10 行）
- 用法: examples/xui_qrcode/main.c:210; examples/xui_qrcode/main.c:211; test_xui/xui_qrcode_test.c:68

## xuiQrCodeGetChangeCount
- 位置: xui.h:6534  已注释: 否
- 签名: `XUI_API int xuiQrCodeGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_qrcode.c:1261（体 7 行）
- 用法: examples/xui_qrcode/main.c:217; test_xui/xui_qrcode_test.c:91

