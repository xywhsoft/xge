# 草稿包：xui.h / scrollframe（46 条 API）

> 生成 2026-09-10 03:04 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiScrollFrameGetType
- 位置: xui.h:9274  已注释: 否
- 签名: `XUI_API xui_widget_type xuiScrollFrameGetType(xui_context pContext);`
- 实现: src/xui_scroll_frame.c:746（体 35 行）
- 返回码: NULL

## xuiScrollFrameCreate
- 位置: xui.h:9275  已注释: 否
- 签名: `XUI_API int xuiScrollFrameCreate(xui_context pContext, xui_widget* ppWidget, const xui_scroll_frame_desc_t* pDesc);`
- 实现: src/xui_scroll_frame.c:782（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: test_xui/xui_scroll_frame_test.c:141

## xuiScrollFrameSetChange
- 位置: xui.h:9276  已注释: 否
- 签名: `XUI_API int xuiScrollFrameSetChange(xui_widget pWidget, xui_scroll_frame_change_proc onChange, void* pUser);`
- 实现: src/xui_scroll_frame.c:797（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_scroll_frame_test.c:156

## xuiScrollFrameGetModel
- 位置: xui.h:9277  已注释: 否
- 签名: `XUI_API xui_scroll_model_t* xuiScrollFrameGetModel(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:806（体 5 行）
- 用法: test_xui/xui_scroll_frame_test.c:178

## xuiScrollFrameGetViewportWidget
- 位置: xui.h:9278  已注释: 否
- 签名: `XUI_API xui_widget xuiScrollFrameGetViewportWidget(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:812（体 5 行）
- 用法: test_xui/xui_scroll_frame_test.c:181; test_xui/xui_scroll_pixel_test.c:116

## xuiScrollFrameGetHScrollBarWidget
- 位置: xui.h:9279  已注释: 否
- 签名: `XUI_API xui_widget xuiScrollFrameGetHScrollBarWidget(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:818（体 5 行）
- 用法: examples/xui_scrollview/main.c:300; examples/xui_scrollview/main.c:302

## xuiScrollFrameGetVScrollBarWidget
- 位置: xui.h:9280  已注释: 否
- 签名: `XUI_API xui_widget xuiScrollFrameGetVScrollBarWidget(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:824（体 5 行）
- 用法: examples/xui_scrollview/main.c:301; examples/xui_scrollview/main.c:303; test_xui/xui_scroll_frame_test.c:186

## xuiScrollFrameGetCornerWidget
- 位置: xui.h:9281  已注释: 否
- 签名: `XUI_API xui_widget xuiScrollFrameGetCornerWidget(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:830（体 5 行）
- 用法: test_xui/xui_scroll_pixel_test.c:117

## xuiScrollFrameSetContentSize
- 位置: xui.h:9282  已注释: 否
- 签名: `XUI_API int xuiScrollFrameSetContentSize(xui_widget pWidget, float fWidth, float fHeight);`
- 实现: src/xui_scroll_frame.c:836（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_scroll_frame_test.c:147; test_xui/xui_scroll_frame_test.c:151; test_xui/xui_scroll_frame_test.c:199

## xuiScrollFrameGetContentSize
- 位置: xui.h:9283  已注释: 否
- 签名: `XUI_API int xuiScrollFrameGetContentSize(xui_widget pWidget, float* pWidth, float* pHeight);`
- 实现: src/xui_scroll_frame.c:847（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollFrameSetViewportHint
- 位置: xui.h:9284  已注释: 否
- 签名: `XUI_API int xuiScrollFrameSetViewportHint(xui_widget pWidget, float fWidth, float fHeight);`
- 实现: src/xui_scroll_frame.c:854（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollFrameGetViewportHint
- 位置: xui.h:9285  已注释: 否
- 签名: `XUI_API int xuiScrollFrameGetViewportHint(xui_widget pWidget, float* pWidth, float* pHeight);`
- 实现: src/xui_scroll_frame.c:868（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_scroll_frame_test.c:145

## xuiScrollFrameSetOffset
- 位置: xui.h:9286  已注释: 否
- 签名: `XUI_API int xuiScrollFrameSetOffset(xui_widget pWidget, float fOffsetX, float fOffsetY);`
- 实现: src/xui_scroll_frame.c:877（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollFrameScrollBy
- 位置: xui.h:9287  已注释: 否
- 签名: `XUI_API int xuiScrollFrameScrollBy(xui_widget pWidget, float fDeltaX, float fDeltaY);`
- 实现: src/xui_scroll_frame.c:884（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollFrameGetOffset
- 位置: xui.h:9288  已注释: 否
- 签名: `XUI_API int xuiScrollFrameGetOffset(xui_widget pWidget, float* pOffsetX, float* pOffsetY);`
- 实现: src/xui_scroll_frame.c:891（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_iconpicker/main.c:335; test_xui/xui_icon_picker_test.c:220; test_xui/xui_scroll_frame_test.c:184

## xuiScrollFrameEnsureRectVisible
- 位置: xui.h:9289  已注释: 否
- 签名: `XUI_API int xuiScrollFrameEnsureRectVisible(xui_widget pWidget, xui_rect_t tContentRect);`
- 实现: src/xui_scroll_frame.c:898（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiScrollFrameSetScrollbarPolicy
- 位置: xui.h:9290  已注释: 否
- 签名: `XUI_API int xuiScrollFrameSetScrollbarPolicy(xui_widget pWidget, int iPolicyX, int iPolicyY);`
- 实现: src/xui_scroll_frame.c:916（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_scroll_frame_test.c:193; test_xui/xui_scroll_frame_test.c:201

## xuiScrollFrameGetScrollbarPolicy
- 位置: xui.h:9291  已注释: 否
- 签名: `XUI_API int xuiScrollFrameGetScrollbarPolicy(xui_widget pWidget, int* pPolicyX, int* pPolicyY);`
- 实现: src/xui_scroll_frame.c:926（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_icon_picker_test.c:189

## xuiScrollFrameSetScrollbarMode
- 位置: xui.h:9292  已注释: 否
- 签名: `XUI_API int xuiScrollFrameSetScrollbarMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_scroll_frame.c:935（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollFrameGetScrollbarMode
- 位置: xui.h:9293  已注释: 否
- 签名: `XUI_API int xuiScrollFrameGetScrollbarMode(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:950（体 5 行）
- 用法: examples/xui_scrollview/main.c:314; examples/xui_scrollview/main.c:315

## xuiScrollFrameSetWheelAxis
- 位置: xui.h:9294  已注释: 否
- 签名: `XUI_API int xuiScrollFrameSetWheelAxis(xui_widget pWidget, int iAxis);`
- 实现: src/xui_scroll_frame.c:956（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollFrameGetWheelAxis
- 位置: xui.h:9295  已注释: 否
- 签名: `XUI_API int xuiScrollFrameGetWheelAxis(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:965（体 5 行）

## xuiScrollFrameSetWheelStep
- 位置: xui.h:9296  已注释: 否
- 签名: `XUI_API int xuiScrollFrameSetWheelStep(xui_widget pWidget, float fStep);`
- 实现: src/xui_scroll_frame.c:971（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollFrameGetWheelStep
- 位置: xui.h:9297  已注释: 否
- 签名: `XUI_API float xuiScrollFrameGetWheelStep(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:980（体 5 行）

## xuiScrollFrameSetContentDragEnabled
- 位置: xui.h:9298  已注释: 否
- 签名: `XUI_API int xuiScrollFrameSetContentDragEnabled(xui_widget pWidget, int bEnabled);`
- 实现: src/xui_scroll_frame.c:986（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollFrameIsContentDragEnabled
- 位置: xui.h:9299  已注释: 否
- 签名: `XUI_API int xuiScrollFrameIsContentDragEnabled(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:997（体 5 行）

## xuiScrollFrameSetCornerMode
- 位置: xui.h:9300  已注释: 否
- 签名: `XUI_API int xuiScrollFrameSetCornerMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_scroll_frame.c:1003（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollFrameGetCornerMode
- 位置: xui.h:9301  已注释: 否
- 签名: `XUI_API int xuiScrollFrameGetCornerMode(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:1012（体 5 行）

## xuiScrollFrameSetMetrics
- 位置: xui.h:9302  已注释: 否
- 签名: `XUI_API int xuiScrollFrameSetMetrics(xui_widget pWidget, float fScrollbarSize, float fMinThumbSize, float fButtonSize);`
- 实现: src/xui_scroll_frame.c:1018（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollFrameGetMetrics
- 位置: xui.h:9303  已注释: 否
- 签名: `XUI_API int xuiScrollFrameGetMetrics(xui_widget pWidget, float* pScrollbarSize, float* pMinThumbSize, float* pButtonSize);`
- 实现: src/xui_scroll_frame.c:1035（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollFrameSetBackgroundColor
- 位置: xui.h:9304  已注释: 否
- 签名: `XUI_API int xuiScrollFrameSetBackgroundColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_scroll_frame.c:1045（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_containers_test.c:227

## xuiScrollFrameGetBackgroundColor
- 位置: xui.h:9305  已注释: 否
- 签名: `XUI_API uint32_t xuiScrollFrameGetBackgroundColor(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:1054（体 5 行）
- 用法: test_xui/xui_style_containers_test.c:197; test_xui/xui_style_pickers_icon_picker_test.c:105; test_xui/xui_style_pickers_icon_picker_test.c:108

## xuiScrollFrameSetColors
- 位置: xui.h:9306  已注释: 否
- 签名: `XUI_API int xuiScrollFrameSetColors(xui_widget pWidget, uint32_t iTrack, uint32_t iThumb, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);`
- 实现: src/xui_scroll_frame.c:1060（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollFrameGetColors
- 位置: xui.h:9307  已注释: 否
- 签名: `XUI_API int xuiScrollFrameGetColors(xui_widget pWidget, uint32_t* pTrack, uint32_t* pThumb, uint32_t* pHover, uint32_t* pActive, uint32_t* pFocus, uint32_t* pDisabled);`
- 实现: src/xui_scroll_frame.c:1077（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_collections_table_test.c:82

## xuiScrollFrameSetButtonColors
- 位置: xui.h:9308  已注释: 否
- 签名: `XUI_API int xuiScrollFrameSetButtonColors(xui_widget pWidget, uint32_t iButton, uint32_t iIcon);`
- 实现: src/xui_scroll_frame.c:1090（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollFrameGetButtonColors
- 位置: xui.h:9309  已注释: 否
- 签名: `XUI_API int xuiScrollFrameGetButtonColors(xui_widget pWidget, uint32_t* pButton, uint32_t* pIcon);`
- 实现: src/xui_scroll_frame.c:1101（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollFrameSetCornerColors
- 位置: xui.h:9310  已注释: 否
- 签名: `XUI_API int xuiScrollFrameSetCornerColors(xui_widget pWidget, uint32_t iCorner, uint32_t iGrip);`
- 实现: src/xui_scroll_frame.c:1110（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollFrameGetCornerColors
- 位置: xui.h:9311  已注释: 否
- 签名: `XUI_API int xuiScrollFrameGetCornerColors(xui_widget pWidget, uint32_t* pCorner, uint32_t* pGrip);`
- 实现: src/xui_scroll_frame.c:1120（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollFrameGetViewportRect
- 位置: xui.h:9312  已注释: 否
- 签名: `XUI_API xui_rect_t xuiScrollFrameGetViewportRect(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:1129（体 5 行）
- 用法: examples/xui_iconpicker/main.c:331; test_xui/xui_icon_picker_test.c:192; test_xui/xui_scroll_frame_test.c:171

## xuiScrollFrameGetHScrollBarRect
- 位置: xui.h:9313  已注释: 否
- 签名: `XUI_API xui_rect_t xuiScrollFrameGetHScrollBarRect(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:1135（体 5 行）
- 用法: test_xui/xui_scroll_frame_test.c:172; test_xui/xui_scroll_pixel_test.c:111

## xuiScrollFrameGetVScrollBarRect
- 位置: xui.h:9314  已注释: 否
- 签名: `XUI_API xui_rect_t xuiScrollFrameGetVScrollBarRect(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:1141（体 5 行）
- 用法: test_xui/xui_scroll_frame_test.c:173; test_xui/xui_scroll_pixel_test.c:112

## xuiScrollFrameGetCornerRect
- 位置: xui.h:9315  已注释: 否
- 签名: `XUI_API xui_rect_t xuiScrollFrameGetCornerRect(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:1147（体 5 行）
- 用法: test_xui/xui_scroll_frame_test.c:174; test_xui/xui_scroll_pixel_test.c:113

## xuiScrollFrameIsHScrollBarVisible
- 位置: xui.h:9316  已注释: 否
- 签名: `XUI_API int xuiScrollFrameIsHScrollBarVisible(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:1153（体 5 行）
- 用法: examples/xui_listview/main.c:276; examples/xui_popup/main.c:315; examples/xui_scrollview/main.c:310

## xuiScrollFrameIsVScrollBarVisible
- 位置: xui.h:9317  已注释: 否
- 签名: `XUI_API int xuiScrollFrameIsVScrollBarVisible(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:1159（体 5 行）
- 用法: examples/xui_listview/main.c:275; examples/xui_popup/main.c:314; examples/xui_scrollview/main.c:311

## xuiScrollFrameIsCornerVisible
- 位置: xui.h:9318  已注释: 否
- 签名: `XUI_API int xuiScrollFrameIsCornerVisible(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:1165（体 5 行）
- 用法: test_xui/xui_scroll_frame_test.c:170

## xuiScrollFrameGetChangeCount
- 位置: xui.h:9319  已注释: 否
- 签名: `XUI_API int xuiScrollFrameGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_scroll_frame.c:1171（体 5 行）

