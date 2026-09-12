# 草稿包：xui.h / scrollview（40 条 API）

> 生成 2026-09-10 03:04 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiScrollViewGetType
- 位置: xui.h:9321  已注释: 否
- 签名: `XUI_API xui_widget_type xuiScrollViewGetType(xui_context pContext);`
- 实现: src/xui_scroll_view.c:342（体 35 行）
- 返回码: NULL
- 用法: test_xui/xui_style_containers_test.c:176; test_xui/xui_style_containers_test.c:201

## xuiScrollViewCreate
- 位置: xui.h:9322  已注释: 否
- 签名: `XUI_API int xuiScrollViewCreate(xui_context pContext, xui_widget* ppWidget, const xui_scroll_view_desc_t* pDesc);`
- 实现: src/xui_scroll_view.c:378（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch144_main1.c:13; examples/tutorial_capture/ch145_main1.c:14; examples/xui_scrollview/main.c:217

## xuiScrollViewSetChange
- 位置: xui.h:9323  已注释: 否
- 签名: `XUI_API int xuiScrollViewSetChange(xui_widget pWidget, xui_scroll_view_change_proc onChange, void* pUser);`
- 实现: src/xui_scroll_view.c:393（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_scrollview/main.c:225; test_xui/xui_scroll_view_test.c:129

## xuiScrollViewGetFrameWidget
- 位置: xui.h:9324  已注释: 否
- 签名: `XUI_API xui_widget xuiScrollViewGetFrameWidget(xui_widget pWidget);`
- 实现: src/xui_scroll_view.c:402（体 5 行）
- 用法: examples/xui_scrollview/main.c:298; examples/xui_scrollview/main.c:299; examples/xui_scrollview/main.c:310

## xuiScrollViewGetContentWidget
- 位置: xui.h:9325  已注释: 否
- 签名: `XUI_API xui_widget xuiScrollViewGetContentWidget(xui_widget pWidget);`
- 实现: src/xui_scroll_view.c:408（体 5 行）
- 用法: examples/xui_scrollview/main.c:249; examples/xui_scrollview/main.c:259; test_xui/xui_scroll_pixel_test.c:105

## xuiScrollViewGetViewportWidget
- 位置: xui.h:9326  已注释: 否
- 签名: `XUI_API xui_widget xuiScrollViewGetViewportWidget(xui_widget pWidget);`
- 实现: src/xui_scroll_view.c:414（体 5 行）
- 用法: examples/xui_scrollview/main.c:325; test_xui/xui_scroll_pixel_test.c:233; test_xui/xui_scroll_view_test.c:133

## xuiScrollViewGetModel
- 位置: xui.h:9327  已注释: 否
- 签名: `XUI_API xui_scroll_model_t* xuiScrollViewGetModel(xui_widget pWidget);`
- 实现: src/xui_scroll_view.c:420（体 5 行）
- 用法: examples/xui_scrollview/main.c:326

## xuiScrollViewSetContentSize
- 位置: xui.h:9328  已注释: 否
- 签名: `XUI_API int xuiScrollViewSetContentSize(xui_widget pWidget, float fWidth, float fHeight);`
- 实现: src/xui_scroll_view.c:426（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_scroll_pixel_test.c:151; test_xui/xui_scroll_pixel_test.c:237; test_xui/xui_scroll_pixel_test.c:242

## xuiScrollViewGetContentSize
- 位置: xui.h:9329  已注释: 否
- 签名: `XUI_API int xuiScrollViewGetContentSize(xui_widget pWidget, float* pWidth, float* pHeight);`
- 实现: src/xui_scroll_view.c:436（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_scroll_pixel_test.c:222; test_xui/xui_scroll_pixel_test.c:246

## xuiScrollViewSetViewportHint
- 位置: xui.h:9330  已注释: 否
- 签名: `XUI_API int xuiScrollViewSetViewportHint(xui_widget pWidget, float fWidth, float fHeight);`
- 实现: src/xui_scroll_view.c:443（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollViewGetViewportHint
- 位置: xui.h:9331  已注释: 否
- 签名: `XUI_API int xuiScrollViewGetViewportHint(xui_widget pWidget, float* pWidth, float* pHeight);`
- 实现: src/xui_scroll_view.c:460（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollViewSetOffset
- 位置: xui.h:9332  已注释: 否
- 签名: `XUI_API int xuiScrollViewSetOffset(xui_widget pWidget, float fOffsetX, float fOffsetY);`
- 实现: src/xui_scroll_view.c:469（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_scroll_pixel_test.c:119; test_xui/xui_scroll_pixel_test.c:217; test_xui/xui_scroll_pixel_test.c:238

## xuiScrollViewScrollBy
- 位置: xui.h:9333  已注释: 否
- 签名: `XUI_API int xuiScrollViewScrollBy(xui_widget pWidget, float fDeltaX, float fDeltaY);`
- 实现: src/xui_scroll_view.c:479（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollViewGetOffset
- 位置: xui.h:9334  已注释: 否
- 签名: `XUI_API int xuiScrollViewGetOffset(xui_widget pWidget, float* pOffsetX, float* pOffsetY);`
- 实现: src/xui_scroll_view.c:489（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_scrollview/main.c:334; test_xui/xui_scroll_pixel_test.c:120; test_xui/xui_scroll_pixel_test.c:150

## xuiScrollViewEnsureRectVisible
- 位置: xui.h:9335  已注释: 否
- 签名: `XUI_API int xuiScrollViewEnsureRectVisible(xui_widget pWidget, xui_rect_t tContentRect);`
- 实现: src/xui_scroll_view.c:496（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollViewEnsureChildVisible
- 位置: xui.h:9336  已注释: 否
- 签名: `XUI_API int xuiScrollViewEnsureChildVisible(xui_widget pWidget, xui_widget pChild);`
- 实现: src/xui_scroll_view.c:506（体 27 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_scrollview/main.c:331; test_xui/xui_scroll_view_test.c:162

## xuiScrollViewSetScrollbarPolicy
- 位置: xui.h:9337  已注释: 否
- 签名: `XUI_API int xuiScrollViewSetScrollbarPolicy(xui_widget pWidget, int iPolicyX, int iPolicyY);`
- 实现: src/xui_scroll_view.c:534（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_scroll_pixel_test.c:147

## xuiScrollViewGetScrollbarPolicy
- 位置: xui.h:9338  已注释: 否
- 签名: `XUI_API int xuiScrollViewGetScrollbarPolicy(xui_widget pWidget, int* pPolicyX, int* pPolicyY);`
- 实现: src/xui_scroll_view.c:541（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollViewSetScrollbarMode
- 位置: xui.h:9339  已注释: 否
- 签名: `XUI_API int xuiScrollViewSetScrollbarMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_scroll_view.c:548（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollViewGetScrollbarMode
- 位置: xui.h:9340  已注释: 否
- 签名: `XUI_API int xuiScrollViewGetScrollbarMode(xui_widget pWidget);`
- 实现: src/xui_scroll_view.c:555（体 5 行）
- 用法: examples/xui_scrollview/main.c:312; examples/xui_scrollview/main.c:313

## xuiScrollViewSetWheelAxis
- 位置: xui.h:9341  已注释: 否
- 签名: `XUI_API int xuiScrollViewSetWheelAxis(xui_widget pWidget, int iAxis);`
- 实现: src/xui_scroll_view.c:561（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollViewGetWheelAxis
- 位置: xui.h:9342  已注释: 否
- 签名: `XUI_API int xuiScrollViewGetWheelAxis(xui_widget pWidget);`
- 实现: src/xui_scroll_view.c:568（体 5 行）

## xuiScrollViewSetWheelStep
- 位置: xui.h:9343  已注释: 否
- 签名: `XUI_API int xuiScrollViewSetWheelStep(xui_widget pWidget, float fStep);`
- 实现: src/xui_scroll_view.c:574（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollViewGetWheelStep
- 位置: xui.h:9344  已注释: 否
- 签名: `XUI_API float xuiScrollViewGetWheelStep(xui_widget pWidget);`
- 实现: src/xui_scroll_view.c:581（体 5 行）

## xuiScrollViewSetContentDragEnabled
- 位置: xui.h:9345  已注释: 否
- 签名: `XUI_API int xuiScrollViewSetContentDragEnabled(xui_widget pWidget, int bEnabled);`
- 实现: src/xui_scroll_view.c:587（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollViewIsContentDragEnabled
- 位置: xui.h:9346  已注释: 否
- 签名: `XUI_API int xuiScrollViewIsContentDragEnabled(xui_widget pWidget);`
- 实现: src/xui_scroll_view.c:594（体 5 行）

## xuiScrollViewSetCornerMode
- 位置: xui.h:9347  已注释: 否
- 签名: `XUI_API int xuiScrollViewSetCornerMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_scroll_view.c:600（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollViewGetCornerMode
- 位置: xui.h:9348  已注释: 否
- 签名: `XUI_API int xuiScrollViewGetCornerMode(xui_widget pWidget);`
- 实现: src/xui_scroll_view.c:607（体 5 行）

## xuiScrollViewSetMetrics
- 位置: xui.h:9349  已注释: 否
- 签名: `XUI_API int xuiScrollViewSetMetrics(xui_widget pWidget, float fScrollbarSize, float fMinThumbSize, float fButtonSize);`
- 实现: src/xui_scroll_view.c:613（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollViewGetMetrics
- 位置: xui.h:9350  已注释: 否
- 签名: `XUI_API int xuiScrollViewGetMetrics(xui_widget pWidget, float* pScrollbarSize, float* pMinThumbSize, float* pButtonSize);`
- 实现: src/xui_scroll_view.c:620（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollViewSetBackgroundColor
- 位置: xui.h:9351  已注释: 否
- 签名: `XUI_API int xuiScrollViewSetBackgroundColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_scroll_view.c:627（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_containers_test.c:155

## xuiScrollViewGetBackgroundColor
- 位置: xui.h:9352  已注释: 否
- 签名: `XUI_API uint32_t xuiScrollViewGetBackgroundColor(xui_widget pWidget);`
- 实现: src/xui_scroll_view.c:634（体 5 行）
- 用法: test_xui/xui_style_containers_test.c:170

## xuiScrollViewSetColors
- 位置: xui.h:9353  已注释: 否
- 签名: `XUI_API int xuiScrollViewSetColors(xui_widget pWidget, uint32_t iTrack, uint32_t iThumb, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);`
- 实现: src/xui_scroll_view.c:640（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollViewGetColors
- 位置: xui.h:9354  已注释: 否
- 签名: `XUI_API int xuiScrollViewGetColors(xui_widget pWidget, uint32_t* pTrack, uint32_t* pThumb, uint32_t* pHover, uint32_t* pActive, uint32_t* pFocus, uint32_t* pDisabled);`
- 实现: src/xui_scroll_view.c:647（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollViewSetButtonColors
- 位置: xui.h:9355  已注释: 否
- 签名: `XUI_API int xuiScrollViewSetButtonColors(xui_widget pWidget, uint32_t iButton, uint32_t iIcon);`
- 实现: src/xui_scroll_view.c:654（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollViewGetButtonColors
- 位置: xui.h:9356  已注释: 否
- 签名: `XUI_API int xuiScrollViewGetButtonColors(xui_widget pWidget, uint32_t* pButton, uint32_t* pIcon);`
- 实现: src/xui_scroll_view.c:661（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollViewSetCornerColors
- 位置: xui.h:9357  已注释: 否
- 签名: `XUI_API int xuiScrollViewSetCornerColors(xui_widget pWidget, uint32_t iCorner, uint32_t iGrip);`
- 实现: src/xui_scroll_view.c:668（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_containers_test.c:156

## xuiScrollViewGetCornerColors
- 位置: xui.h:9358  已注释: 否
- 签名: `XUI_API int xuiScrollViewGetCornerColors(xui_widget pWidget, uint32_t* pCorner, uint32_t* pGrip);`
- 实现: src/xui_scroll_view.c:675（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollViewGetViewportRect
- 位置: xui.h:9359  已注释: 否
- 签名: `XUI_API xui_rect_t xuiScrollViewGetViewportRect(xui_widget pWidget);`
- 实现: src/xui_scroll_view.c:682（体 5 行）
- 用法: examples/xui_scrollview/main.c:296; examples/xui_scrollview/main.c:297; test_xui/xui_scroll_pixel_test.c:110

## xuiScrollViewGetChangeCount
- 位置: xui.h:9360  已注释: 否
- 签名: `XUI_API int xuiScrollViewGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_scroll_view.c:688（体 5 行）

