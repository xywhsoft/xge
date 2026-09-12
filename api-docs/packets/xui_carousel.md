# 草稿包：xui.h / carousel（30 条 API）

> 生成 2026-09-10 02:56 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiCarouselGetType
- 位置: xui.h:8006  已注释: 否
- 签名: `XUI_API xui_widget_type xuiCarouselGetType(xui_context pContext);`
- 实现: src/xui_carousel.c:940（体 32 行）
- 返回码: NULL

## xuiCarouselCreate
- 位置: xui.h:8007  已注释: 否
- 签名: `XUI_API int xuiCarouselCreate(xui_context pContext, xui_widget* ppWidget, const xui_carousel_desc_t* pDesc);`
- 实现: src/xui_carousel.c:973（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch163_main1.c:18; examples/xui_carousel/main.c:238; test_xui/xui_carousel_test.c:85

## xuiCarouselSetChange
- 位置: xui.h:8008  已注释: 否
- 签名: `XUI_API int xuiCarouselSetChange(xui_widget pWidget, xui_carousel_change_proc onChange, void* pUser);`
- 实现: src/xui_carousel.c:984（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_carousel/main.c:241; test_xui/xui_carousel_test.c:88

## xuiCarouselSetPageCount
- 位置: xui.h:8009  已注释: 否
- 签名: `XUI_API int xuiCarouselSetPageCount(xui_widget pWidget, int iPageCount);`
- 实现: src/xui_carousel.c:993（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_chrome_test.c:303

## xuiCarouselGetPageCount
- 位置: xui.h:8010  已注释: 否
- 签名: `XUI_API int xuiCarouselGetPageCount(xui_widget pWidget);`
- 实现: src/xui_carousel.c:1000（体 5 行）
- 用法: test_xui/xui_carousel_test.c:93

## xuiCarouselGetPageWidget
- 位置: xui.h:8011  已注释: 否
- 签名: `XUI_API xui_widget xuiCarouselGetPageWidget(xui_widget pWidget, int iIndex);`
- 实现: src/xui_carousel.c:1006（体 6 行）
- 返回码: NULL
- 用法: examples/xui_carousel/main.c:194; examples/xui_carousel/main.c:354; test_xui/xui_carousel_test.c:95

## xuiCarouselAddPageChild
- 位置: xui.h:8012  已注释: 否
- 签名: `XUI_API int xuiCarouselAddPageChild(xui_widget pWidget, int iIndex, xui_widget pChild);`
- 实现: src/xui_carousel.c:1013（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_carousel_test.c:102

## xuiCarouselSetCurrent
- 位置: xui.h:8013  已注释: 否
- 签名: `XUI_API int xuiCarouselSetCurrent(xui_widget pWidget, int iIndex, int bNotify);`
- 实现: src/xui_carousel.c:1023（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_carousel/main.c:272; test_xui/xui_carousel_test.c:119

## xuiCarouselGetCurrent
- 位置: xui.h:8014  已注释: 否
- 签名: `XUI_API int xuiCarouselGetCurrent(xui_widget pWidget);`
- 实现: src/xui_carousel.c:1030（体 5 行）
- 用法: examples/xui_carousel/main.c:257; examples/xui_carousel/main.c:274; examples/xui_carousel/main.c:354

## xuiCarouselNext
- 位置: xui.h:8015  已注释: 否
- 签名: `XUI_API int xuiCarouselNext(xui_widget pWidget, int bNotify);`
- 实现: src/xui_carousel.c:1036（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_carousel/main.c:273; test_xui/xui_carousel_test.c:106; test_xui/xui_carousel_test.c:109

## xuiCarouselPrev
- 位置: xui.h:8016  已注释: 否
- 签名: `XUI_API int xuiCarouselPrev(xui_widget pWidget, int bNotify);`
- 实现: src/xui_carousel.c:1043（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_carousel_test.c:111

## xuiCarouselSetAutoPlay
- 位置: xui.h:8017  已注释: 否
- 签名: `XUI_API int xuiCarouselSetAutoPlay(xui_widget pWidget, int bEnabled, float fInterval);`
- 实现: src/xui_carousel.c:1050（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_carousel/main.c:275; test_xui/xui_carousel_test.c:121; test_xui/xui_style_chrome_test.c:304

## xuiCarouselGetAutoPlay
- 位置: xui.h:8018  已注释: 否
- 签名: `XUI_API int xuiCarouselGetAutoPlay(xui_widget pWidget, int* pEnabled, float* pInterval);`
- 实现: src/xui_carousel.c:1061（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_carousel_test.c:123

## xuiCarouselSetLoop
- 位置: xui.h:8019  已注释: 否
- 签名: `XUI_API int xuiCarouselSetLoop(xui_widget pWidget, int bLoop);`
- 实现: src/xui_carousel.c:1070（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_carousel_test.c:113; test_xui/xui_carousel_test.c:117

## xuiCarouselGetLoop
- 位置: xui.h:8020  已注释: 否
- 签名: `XUI_API int xuiCarouselGetLoop(xui_widget pWidget);`
- 实现: src/xui_carousel.c:1078（体 5 行）
- 用法: examples/xui_carousel/main.c:261; test_xui/xui_carousel_test.c:114

## xuiCarouselSetIndicatorsVisible
- 位置: xui.h:8021  已注释: 否
- 签名: `XUI_API int xuiCarouselSetIndicatorsVisible(xui_widget pWidget, int bVisible);`
- 实现: src/xui_carousel.c:1084（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCarouselGetIndicatorsVisible
- 位置: xui.h:8022  已注释: 否
- 签名: `XUI_API int xuiCarouselGetIndicatorsVisible(xui_widget pWidget);`
- 实现: src/xui_carousel.c:1094（体 5 行）

## xuiCarouselSetFont
- 位置: xui.h:8023  已注释: 否
- 签名: `XUI_API int xuiCarouselSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_carousel.c:1100（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCarouselGetFont
- 位置: xui.h:8024  已注释: 否
- 签名: `XUI_API xui_font xuiCarouselGetFont(xui_widget pWidget);`
- 实现: src/xui_carousel.c:1108（体 5 行）

## xuiCarouselSetArrowsOnHover
- 位置: xui.h:8025  已注释: 否
- 签名: `XUI_API int xuiCarouselSetArrowsOnHover(xui_widget pWidget, int bEnabled);`
- 实现: src/xui_carousel.c:1114（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCarouselGetArrowsOnHover
- 位置: xui.h:8026  已注释: 否
- 签名: `XUI_API int xuiCarouselGetArrowsOnHover(xui_widget pWidget);`
- 实现: src/xui_carousel.c:1124（体 5 行）

## xuiCarouselSetMetrics
- 位置: xui.h:8027  已注释: 否
- 签名: `XUI_API int xuiCarouselSetMetrics(xui_widget pWidget, float fArrowSize, float fIndicatorSize, float fIndicatorGap, float fIndicatorBottom);`
- 实现: src/xui_carousel.c:1130（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_carousel_test.c:127

## xuiCarouselGetMetrics
- 位置: xui.h:8028  已注释: 否
- 签名: `XUI_API int xuiCarouselGetMetrics(xui_widget pWidget, float* pArrowSize, float* pIndicatorSize, float* pIndicatorGap, float* pIndicatorBottom);`
- 实现: src/xui_carousel.c:1141（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCarouselSetColors
- 位置: xui.h:8029  已注释: 否
- 签名: `XUI_API int xuiCarouselSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iArrow, uint32_t iArrowHover, uint32_t iArrowText, uint32_t iIndicator, uint32_t iIndicatorActive, uint32_t iIndicatorHover);`
- 实现: src/xui_carousel.c:1152（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_carousel_test.c:129

## xuiCarouselGetColors
- 位置: xui.h:8030  已注释: 否
- 签名: `XUI_API int xuiCarouselGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pArrow, uint32_t* pArrowHover, uint32_t* pArrowText, uint32_t* pIndicator, uint32_t* pIndicatorActive, uint32_t* pIndicatorHover);`
- 实现: src/xui_carousel.c:1166（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCarouselSetFocusColor
- 位置: xui.h:8031  已注释: 否
- 签名: `XUI_API int xuiCarouselSetFocusColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_carousel.c:1180（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCarouselGetFocusColor
- 位置: xui.h:8032  已注释: 否
- 签名: `XUI_API uint32_t xuiCarouselGetFocusColor(xui_widget pWidget);`
- 实现: src/xui_carousel.c:1188（体 5 行）

## xuiCarouselGetHoverIndicator
- 位置: xui.h:8033  已注释: 否
- 签名: `XUI_API int xuiCarouselGetHoverIndicator(xui_widget pWidget);`
- 实现: src/xui_carousel.c:1194（体 5 行）

## xuiCarouselGetHoverArrow
- 位置: xui.h:8034  已注释: 否
- 签名: `XUI_API int xuiCarouselGetHoverArrow(xui_widget pWidget);`
- 实现: src/xui_carousel.c:1200（体 5 行）

## xuiCarouselGetChangeCount
- 位置: xui.h:8035  已注释: 否
- 签名: `XUI_API int xuiCarouselGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_carousel.c:1206（体 5 行）
- 用法: examples/xui_carousel/main.c:378; test_xui/xui_carousel_test.c:138

