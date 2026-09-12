# 草稿包：xui.h / msgtip（27 条 API）

> 生成 2026-09-10 03:02 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiMsgTipCreate
- 位置: xui.h:8961  已注释: 否
- 签名: `XUI_API int xuiMsgTipCreate(xui_context pContext, xui_msgtip* ppTip, const xui_msgtip_desc_t* pDesc);`
- 实现: src/xui_msgtip.c:685（体 55 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/tutorial_capture/ch160_main1.c:16; examples/xui_msgtip/main.c:203; test_xui/xui_msgtip_test.c:173

## xuiMsgTipDestroy
- 位置: xui.h:8962  已注释: 否
- 签名: `XUI_API void xuiMsgTipDestroy(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:741（体 22 行）
- 用法: examples/xui_msgtip/main.c:287; test_xui/xui_msgtip_test.c:262

## xuiMsgTipShow
- 位置: xui.h:8963  已注释: 否
- 签名: `XUI_API int xuiMsgTipShow(xui_msgtip pTip, int iType, const char* sText, float fDuration);`
- 实现: src/xui_msgtip.c:764（体 29 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch160_main1.c:18; examples/xui_msgtip/main.c:150; examples/xui_msgtip/main.c:207

## xuiMsgTipClose
- 位置: xui.h:8964  已注释: 否
- 签名: `XUI_API int xuiMsgTipClose(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:794（体 4 行）

## xuiMsgTipIsOpen
- 位置: xui.h:8965  已注释: 否
- 签名: `XUI_API int xuiMsgTipIsOpen(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:799（体 4 行）
- 用法: examples/xui_msgtip/main.c:241; examples/xui_msgtip/main.c:247; test_xui/xui_msgtip_test.c:177

## xuiMsgTipSetText
- 位置: xui.h:8966  已注释: 否
- 签名: `XUI_API int xuiMsgTipSetText(xui_msgtip pTip, const char* sText);`
- 实现: src/xui_msgtip.c:804（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMsgTipGetText
- 位置: xui.h:8967  已注释: 否
- 签名: `XUI_API const char* xuiMsgTipGetText(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:817（体 4 行）
- 用法: test_xui/xui_msgtip_test.c:178

## xuiMsgTipSetType
- 位置: xui.h:8968  已注释: 否
- 签名: `XUI_API int xuiMsgTipSetType(xui_msgtip pTip, int iType);`
- 实现: src/xui_msgtip.c:822（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMsgTipGetType
- 位置: xui.h:8969  已注释: 否
- 签名: `XUI_API int xuiMsgTipGetType(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:835（体 4 行）
- 用法: test_xui/xui_msgtip_test.c:235

## xuiMsgTipSetDuration
- 位置: xui.h:8970  已注释: 否
- 签名: `XUI_API int xuiMsgTipSetDuration(xui_msgtip pTip, float fDuration);`
- 实现: src/xui_msgtip.c:840（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_msgtip_test.c:239

## xuiMsgTipGetDuration
- 位置: xui.h:8971  已注释: 否
- 签名: `XUI_API float xuiMsgTipGetDuration(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:850（体 4 行）
- 用法: test_xui/xui_msgtip_test.c:240

## xuiMsgTipSetIconSurface
- 位置: xui.h:8972  已注释: 否
- 签名: `XUI_API int xuiMsgTipSetIconSurface(xui_msgtip pTip, xui_surface pSurface, xui_rect_t tSrc);`
- 实现: src/xui_msgtip.c:855（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_msgtip_test.c:219

## xuiMsgTipUseBuiltinIcon
- 位置: xui.h:8973  已注释: 否
- 签名: `XUI_API int xuiMsgTipUseBuiltinIcon(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:867（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_msgtip_test.c:241

## xuiMsgTipSetMetrics
- 位置: xui.h:8974  已注释: 否
- 签名: `XUI_API int xuiMsgTipSetMetrics(xui_msgtip pTip, const xui_msgtip_metrics_t* pMetrics);`
- 实现: src/xui_msgtip.c:880（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_msgtip_test.c:226

## xuiMsgTipGetMetrics
- 位置: xui.h:8975  已注释: 否
- 签名: `XUI_API int xuiMsgTipGetMetrics(xui_msgtip pTip, xui_msgtip_metrics_t* pMetrics);`
- 实现: src/xui_msgtip.c:894（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_msgtip_test.c:221

## xuiMsgTipSetColors
- 位置: xui.h:8976  已注释: 否
- 签名: `XUI_API int xuiMsgTipSetColors(xui_msgtip pTip, const xui_msgtip_colors_t* pColors);`
- 实现: src/xui_msgtip.c:904（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_msgtip_test.c:232

## xuiMsgTipGetColors
- 位置: xui.h:8977  已注释: 否
- 签名: `XUI_API int xuiMsgTipGetColors(xui_msgtip pTip, xui_msgtip_colors_t* pColors);`
- 实现: src/xui_msgtip.c:915（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_msgtip_test.c:179; test_xui/xui_msgtip_test.c:228

## xuiMsgTipSetClose
- 位置: xui.h:8978  已注释: 否
- 签名: `XUI_API int xuiMsgTipSetClose(xui_msgtip pTip, xui_msgtip_close_proc onClose, void* pUser);`
- 实现: src/xui_msgtip.c:925（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_msgtip/main.c:205; test_xui/xui_msgtip_test.c:175

## xuiMsgTipGetWidget
- 位置: xui.h:8979  已注释: 否
- 签名: `XUI_API xui_widget xuiMsgTipGetWidget(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:935（体 4 行）
- 用法: test_xui/xui_msgtip_test.c:193; test_xui/xui_msgtip_test.c:197; test_xui/xui_msgtip_test.c:197

## xuiMsgTipGetTipRect
- 位置: xui.h:8980  已注释: 否
- 签名: `XUI_API xui_rect_t xuiMsgTipGetTipRect(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:940（体 4 行）
- 用法: examples/xui_msgtip/main.c:232; examples/xui_msgtip/main.c:244; test_xui/xui_msgtip_test.c:186

## xuiMsgTipGetIconRect
- 位置: xui.h:8981  已注释: 否
- 签名: `XUI_API xui_rect_t xuiMsgTipGetIconRect(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:945（体 4 行）
- 用法: examples/xui_msgtip/main.c:233; test_xui/xui_msgtip_test.c:187; test_xui/xui_msgtip_test.c:237

## xuiMsgTipGetTextRect
- 位置: xui.h:8982  已注释: 否
- 签名: `XUI_API xui_rect_t xuiMsgTipGetTextRect(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:950（体 4 行）
- 用法: examples/xui_msgtip/main.c:234; test_xui/xui_msgtip_test.c:188; test_xui/xui_msgtip_test.c:252

## xuiMsgTipGetWrapLineCount
- 位置: xui.h:8983  已注释: 否
- 签名: `XUI_API int xuiMsgTipGetWrapLineCount(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:955（体 4 行）
- 用法: examples/xui_msgtip/main.c:235; test_xui/xui_msgtip_test.c:192; test_xui/xui_msgtip_test.c:255

## xuiMsgTipGetShowCount
- 位置: xui.h:8984  已注释: 否
- 签名: `XUI_API int xuiMsgTipGetShowCount(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:960（体 4 行）
- 用法: examples/xui_msgtip/main.c:352; test_xui/xui_msgtip_test.c:243

## xuiMsgTipGetCloseCount
- 位置: xui.h:8985  已注释: 否
- 签名: `XUI_API int xuiMsgTipGetCloseCount(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:965（体 4 行）
- 用法: examples/xui_msgtip/main.c:245; examples/xui_msgtip/main.c:247; examples/xui_msgtip/main.c:352

## xuiMsgTipGetExpireCount
- 位置: xui.h:8986  已注释: 否
- 签名: `XUI_API int xuiMsgTipGetExpireCount(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:970（体 4 行）
- 用法: examples/xui_msgtip/main.c:237; examples/xui_msgtip/main.c:241; examples/xui_msgtip/main.c:352

## xuiMsgTipGetChangeCount
- 位置: xui.h:8987  已注释: 否
- 签名: `XUI_API int xuiMsgTipGetChangeCount(xui_msgtip pTip);`
- 实现: src/xui_msgtip.c:975（体 4 行）
- 用法: test_xui/xui_msgtip_test.c:244

