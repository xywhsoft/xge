# 草稿包：xui.h / Widget（174 条 API）

> 生成 2026-09-10 02:48 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiWidgetGetBaseType
- 位置: xui.h:8959  已注释: 否
- 签名: `XUI_API xui_widget_type xuiWidgetGetBaseType(void);`
- 实现: src/xui_widget.c:2793（体 4 行）
- 用法: test_xui/xui_render_schedule_test.c:1588; test_xui/xui_style_test.c:161; test_xui/xui_widget_type_test.c:174

## xuiWidgetRegisterType
- 位置: xui.h:8960  已注释: 否
- 签名: `XUI_API int xuiWidgetRegisterType(xui_context pContext, xui_widget_type* ppType, const xui_widget_type_desc_t* pDesc);`
- 实现: src/xui_widget.c:2798（体 47 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_ERROR_ALREADY_INITIALIZED, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_callback_lifetime_test.c:281; test_xui/xui_callback_lifetime_test.c:308; test_xui/xui_prepare_paint_test.c:82

## xuiWidgetUnregisterType
- 位置: xui.h:8961  已注释: 否
- 签名: `XUI_API int xuiWidgetUnregisterType(xui_widget_type pType);`
- 实现: src/xui_widget.c:2846（体 35 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_ALREADY_INITIALIZED, XUI_OK
- 用法: test_xui/xui_callback_lifetime_test.c:287; test_xui/xui_render_schedule_test.c:1666; test_xui/xui_state_event_test.c:305

## xuiWidgetFindType
- 位置: xui.h:8962  已注释: 否
- 签名: `XUI_API xui_widget_type xuiWidgetFindType(xui_context pContext, const char* sName);`
- 实现: src/xui_widget.c:2882（体 7 行）
- 返回码: NULL
- 用法: test_xui/xui_widget_type_test.c:177; test_xui/xui_widget_type_test.c:200; test_xui/xui_widget_type_test.c:205

## xuiWidgetTypeGetName
- 位置: xui.h:8963  已注释: 否
- 签名: `XUI_API const char* xuiWidgetTypeGetName(xui_widget_type pType);`
- 实现: src/xui_widget.c:2890（体 4 行）
- 用法: test_xui/xui_code_edit_test.c:554; test_xui/xui_label_test.c:896; test_xui/xui_widget_type_test.c:175

## xuiWidgetTypeGetParent
- 位置: xui.h:8964  已注释: 否
- 签名: `XUI_API xui_widget_type xuiWidgetTypeGetParent(xui_widget_type pType);`
- 实现: src/xui_widget.c:2895（体 4 行）
- 用法: test_xui/xui_widget_type_test.c:211; test_xui/xui_widget_type_test.c:225

## xuiWidgetTypeGetUserData
- 位置: xui.h:8965  已注释: 否
- 签名: `XUI_API void* xuiWidgetTypeGetUserData(xui_widget_type pType);`
- 实现: src/xui_widget.c:2900（体 4 行）
- 用法: test_xui/xui_widget_type_test.c:212

## xuiWidgetCreate
- 位置: xui.h:8967  已注释: 否
- 签名: `XUI_API int xuiWidgetCreate(xui_context pContext, xui_widget* ppWidget);`
- 实现: src/xui_widget.c:3430（体 12 行）
- 用法: examples/audit_xui_cache/main.c:7; examples/audit_xui_complex/main.c:7; examples/audit_xui_container/main.c:9

## xuiWidgetCreateTyped
- 位置: xui.h:8968  已注释: 否
- 签名: `XUI_API int xuiWidgetCreateTyped(xui_context pContext, xui_widget_type pType, xui_widget* ppWidget, const void* pCreateData);`
- 实现: src/xui_widget.c:3443（体 12 行）
- 用法: test_xui/xui_callback_lifetime_test.c:283; test_xui/xui_callback_lifetime_test.c:310; test_xui/xui_prepare_paint_test.c:87

## xuiWidgetDestroy
- 位置: xui.h:8969  已注释: 否
- 签名: `XUI_API void xuiWidgetDestroy(xui_widget pWidget);`
- 实现: src/xui_widget.c:3694（体 36 行）
- 用法: examples/xui_accordion/main.c:152; examples/xui_accordion/main.c:174; examples/xui_breadcrumb/main.c:159

## xuiWidgetGetContext
- 位置: xui.h:8970  已注释: 否
- 签名: `XUI_API xui_context xuiWidgetGetContext(xui_widget pWidget);`
- 实现: src/xui_widget.c:3731（体 7 行）
- 返回码: NULL
- 用法: examples/xui_input/main.c:338; examples/xui_multitouch/main.c:228; test_xui/xui_input_test.c:220

## xuiWidgetGetType
- 位置: xui.h:8971  已注释: 否
- 签名: `XUI_API xui_widget_type xuiWidgetGetType(xui_widget pWidget);`
- 实现: src/xui_widget.c:3739（体 7 行）
- 返回码: NULL
- 用法: test_xui/xui_style_basic_canvas_test.c:56; test_xui/xui_style_basic_canvas_test.c:76; test_xui/xui_style_chrome_test.c:67

## xuiWidgetIsType
- 位置: xui.h:8972  已注释: 否
- 签名: `XUI_API int xuiWidgetIsType(xui_widget pWidget, xui_widget_type pType);`
- 实现: src/xui_widget.c:3747（体 14 行）
- 用法: test_xui/xui_code_edit_test.c:232; test_xui/xui_code_edit_test.c:247; test_xui/xui_code_edit_test.c:672

## xuiWidgetGetTypeData
- 位置: xui.h:8973  已注释: 否
- 签名: `XUI_API void* xuiWidgetGetTypeData(xui_widget pWidget);`
- 实现: src/xui_widget.c:3762（体 7 行）
- 返回码: NULL
- 用法: test_xui/xui_state_event_test.c:292; test_xui/xui_widget_type_test.c:231

## xuiWidgetSetUserData
- 位置: xui.h:8974  已注释: 否
- 签名: `XUI_API void xuiWidgetSetUserData(xui_widget pWidget, void* pUser);`
- 实现: src/xui_widget.c:3770（体 6 行）
- 用法: test_xui/xui_prepare_paint_test.c:88; test_xui/xui_widget_test.c:115

## xuiWidgetGetUserData
- 位置: xui.h:8975  已注释: 否
- 签名: `XUI_API void* xuiWidgetGetUserData(xui_widget pWidget);`
- 实现: src/xui_widget.c:3777（体 7 行）
- 返回码: NULL
- 用法: test_xui/xui_prepare_paint_test.c:26; test_xui/xui_widget_test.c:116

## xuiWidgetAddChild
- 位置: xui.h:8977  已注释: 否
- 签名: `XUI_API int xuiWidgetAddChild(xui_widget pParent, xui_widget pChild);`
- 实现: src/xui_widget.c:3785（体 4 行）
- 用法: examples/audit_xui_basic/main.c:18; examples/audit_xui_basic/main.c:21; examples/audit_xui_basic/main.c:23

## xuiWidgetInsertBefore
- 位置: xui.h:8978  已注释: 否
- 签名: `XUI_API int xuiWidgetInsertBefore(xui_widget pParent, xui_widget pChild, xui_widget pBefore);`
- 实现: src/xui_widget.c:3834（体 9 行）
- 用法: test_xui/xui_widget_test.c:132

## xuiWidgetRemoveFromParent
- 位置: xui.h:8979  已注释: 否
- 签名: `XUI_API int xuiWidgetRemoveFromParent(xui_widget pWidget);`
- 实现: src/xui_widget.c:4086（体 11 行）
- 用法: test_xui/xui_accessibility_test.c:414; test_xui/xui_cache_transition_test.c:73; test_xui/xui_render_schedule_test.c:991

## xuiWidgetGetParent
- 位置: xui.h:8980  已注释: 否
- 签名: `XUI_API xui_widget xuiWidgetGetParent(xui_widget pWidget);`
- 实现: src/xui_widget.c:3899（体 4 行）
- 用法: examples/xui_checkcard/main.c:381; examples/xui_checkcard/main.c:382; examples/xui_checkcard/main.c:383

## xuiWidgetGetFirstChild
- 位置: xui.h:8981  已注释: 否
- 签名: `XUI_API xui_widget xuiWidgetGetFirstChild(xui_widget pWidget);`
- 实现: src/xui_widget.c:3904（体 4 行）
- 用法: test_xui/xui_accessibility_test.c:412; test_xui/xui_code_edit_test.c:231; test_xui/xui_code_edit_test.c:246

## xuiWidgetGetLastChild
- 位置: xui.h:8982  已注释: 否
- 签名: `XUI_API xui_widget xuiWidgetGetLastChild(xui_widget pWidget);`
- 实现: src/xui_widget.c:3909（体 4 行）
- 用法: test_xui/xui_split_layout_pixel_test.c:75; test_xui/xui_widget_test.c:123; test_xui/xui_widget_test.c:135

## xuiWidgetGetPrevSibling
- 位置: xui.h:8983  已注释: 否
- 签名: `XUI_API xui_widget xuiWidgetGetPrevSibling(xui_widget pWidget);`
- 实现: src/xui_widget.c:3914（体 4 行）
- 用法: test_xui/xui_widget_test.c:137

## xuiWidgetGetNextSibling
- 位置: xui.h:8984  已注释: 否
- 签名: `XUI_API xui_widget xuiWidgetGetNextSibling(xui_widget pWidget);`
- 实现: src/xui_widget.c:3919（体 4 行）
- 用法: test_xui/xui_code_edit_test.c:231; test_xui/xui_code_edit_test.c:246; test_xui/xui_grid_focus_test.c:236

## xuiWidgetGetChildCount
- 位置: xui.h:8985  已注释: 否
- 签名: `XUI_API int xuiWidgetGetChildCount(xui_widget pWidget);`
- 实现: src/xui_widget.c:3924（体 4 行）
- 用法: test_xui/xui_callback_lifetime_test.c:171; test_xui/xui_dock_panel_test.c:1533; test_xui/xui_dock_panel_test.c:1553

## xuiWidgetSetRect
- 位置: xui.h:8987  已注释: 否
- 签名: `XUI_API int xuiWidgetSetRect(xui_widget pWidget, xui_rect_t tRect);`
- 实现: src/xui_widget.c:3957（体 11 行）
- 用法: examples/tutorial_capture/ch126_main1.c:21; examples/tutorial_capture/ch126_main1.c:27; examples/tutorial_capture/ch126_main1.c:35

## xuiWidgetGetRect
- 位置: xui.h:8988  已注释: 否
- 签名: `XUI_API xui_rect_t xuiWidgetGetRect(xui_widget pWidget);`
- 实现: src/xui_widget.c:3969（体 10 行）
- 用法: examples/xui_accordion/main.c:110; examples/xui_button/main.c:511; examples/xui_button/main.c:512

## xuiWidgetGetWorldRect
- 位置: xui.h:8989  已注释: 否
- 签名: `XUI_API xui_rect_t xuiWidgetGetWorldRect(xui_widget pWidget);`
- 实现: src/xui_widget.c:3980（体 16 行）
- 用法: examples/xui_accordion/main.c:246; examples/xui_breadcrumb/main.c:257; examples/xui_button/main.c:521

## xuiWidgetGetContentRect
- 位置: xui.h:8990  已注释: 否
- 签名: `XUI_API xui_rect_t xuiWidgetGetContentRect(xui_widget pWidget);`
- 实现: src/xui_widget.c:3997（体 10 行）
- 用法: examples/xui_breadcrumb/main.c:131; examples/xui_canvas/main.c:107; examples/xui_carousel/main.c:113

## xuiWidgetSetLayout
- 位置: xui.h:8991  已注释: 否
- 签名: `XUI_API int xuiWidgetSetLayout(xui_widget pWidget, const xui_layout_t* pLayout);`
- 实现: src/xui_widget.c:4008（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_render_schedule_test.c:1007; test_xui/xui_render_schedule_test.c:1246

## xuiWidgetGetLayout
- 位置: xui.h:8992  已注释: 否
- 签名: `XUI_API xui_layout_t xuiWidgetGetLayout(xui_widget pWidget);`
- 实现: src/xui_widget.c:4021（体 10 行）
- 用法: test_xui/xui_code_edit_test.c:579; test_xui/xui_render_schedule_test.c:1005; test_xui/xui_render_schedule_test.c:1244

## xuiWidgetSetContentMeasureCallback
- 位置: xui.h:8993  已注释: 否
- 签名: `XUI_API int xuiWidgetSetContentMeasureCallback(xui_widget pWidget, xui_widget_content_measure_proc onMeasure, void* pUser);`
- 实现: src/xui_widget.c:4032（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_layout_callback_test.c:180; test_xui/xui_layout_callback_test.c:205; test_xui/xui_layout_callback_test.c:240

## xuiWidgetGetContentMeasureCallback
- 位置: xui.h:8994  已注释: 否
- 签名: `XUI_API int xuiWidgetGetContentMeasureCallback(xui_widget pWidget, xui_widget_content_measure_proc* pMeasure, void** ppUser);`
- 实现: src/xui_widget.c:4045（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_layout_callback_test.c:200; test_xui/xui_layout_callback_test.c:209; test_xui/xui_widget_type_test.c:241

## xuiWidgetSetLayoutPrepareCallback
- 位置: xui.h:8995  已注释: 否
- 签名: `XUI_API int xuiWidgetSetLayoutPrepareCallback(xui_widget pWidget, xui_widget_layout_prepare_proc onPrepare, void* pUser);`
- 实现: src/xui_widget.c:4059（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWidgetGetLayoutPrepareCallback
- 位置: xui.h:8996  已注释: 否
- 签名: `XUI_API int xuiWidgetGetLayoutPrepareCallback(xui_widget pWidget, xui_widget_layout_prepare_proc* pPrepare, void** ppUser);`
- 实现: src/xui_widget.c:4072（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWidgetSetLayoutChildrenCallback
- 位置: xui.h:8997  已注释: 否
- 签名: `XUI_API int xuiWidgetSetLayoutChildrenCallback(xui_widget pWidget, xui_widget_layout_children_proc onChildren, void* pUser);`
- 实现: src/xui_widget.c:4098（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_layout_callback_test.c:221; test_xui/xui_layout_callback_test.c:247

## xuiWidgetGetLayoutChildrenCallback
- 位置: xui.h:8998  已注释: 否
- 签名: `XUI_API int xuiWidgetGetLayoutChildrenCallback(xui_widget pWidget, xui_widget_layout_children_proc* pChildren, void** ppUser);`
- 实现: src/xui_widget.c:4111（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_layout_callback_test.c:223; test_xui/xui_layout_callback_test.c:249

## xuiWidgetSetLayoutCompleteCallback
- 位置: xui.h:9001  已注释: 否
- 签名: `XUI_API int xuiWidgetSetLayoutCompleteCallback(xui_widget pWidget, xui_widget_layout_complete_proc onComplete, void* pUser);`
- 实现: src/xui_widget.c:4125（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_callback_lifetime_test.c:183; test_xui/xui_layout_callback_test.c:217; test_xui/xui_layout_callback_test.c:253

## xuiWidgetGetLayoutCompleteCallback
- 位置: xui.h:9002  已注释: 否
- 签名: `XUI_API int xuiWidgetGetLayoutCompleteCallback(xui_widget pWidget, xui_widget_layout_complete_proc* pComplete, void** ppUser);`
- 实现: src/xui_widget.c:4138（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_layout_callback_test.c:219; test_xui/xui_widget_type_test.c:243

## xuiWidgetMeasureContent
- 位置: xui.h:9003  已注释: 否
- 签名: `XUI_API int xuiWidgetMeasureContent(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pContentSize);`
- 实现: src/xui_widget.c:4163（体 11 行）
- 用法: test_xui/xui_hyperlink_test.c:131; test_xui/xui_image_icon_pixel_test.c:54; test_xui/xui_image_test.c:711

## xuiWidgetSetLayoutType
- 位置: xui.h:9004  已注释: 否
- 签名: `XUI_API int xuiWidgetSetLayoutType(xui_widget pWidget, int iLayoutType);`
- 实现: src/xui_widget.c:4175（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/audit_xui_cache/main.c:7; examples/audit_xui_complex/main.c:7; examples/audit_xui_container/main.c:10

## xuiWidgetGetLayoutType
- 位置: xui.h:9005  已注释: 否
- 签名: `XUI_API int xuiWidgetGetLayoutType(xui_widget pWidget);`
- 实现: src/xui_widget.c:4187（体 4 行）

## xuiWidgetSetSizeMode
- 位置: xui.h:9006  已注释: 否
- 签名: `XUI_API int xuiWidgetSetSizeMode(xui_widget pWidget, int iWidthMode, int iHeightMode);`
- 实现: src/xui_widget.c:4192（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/tutorial_capture/ch123_main1.c:14; examples/tutorial_capture/ch127_main1.c:18; examples/tutorial_capture/ch127_main1.c:37

## xuiWidgetGetSizeMode
- 位置: xui.h:9007  已注释: 否
- 签名: `XUI_API int xuiWidgetGetSizeMode(xui_widget pWidget, int* pWidthMode, int* pHeightMode);`
- 实现: src/xui_widget.c:4205（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_layout_test.c:155

## xuiWidgetSetFlowMode
- 位置: xui.h:9008  已注释: 否
- 签名: `XUI_API int xuiWidgetSetFlowMode(xui_widget pWidget, int iFlowMode);`
- 实现: src/xui_widget.c:4219（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_language/main.c:293; examples/xui_layout/main.c:75; test_xui/xui_layout_test.c:233

## xuiWidgetGetFlowMode
- 位置: xui.h:9009  已注释: 否
- 签名: `XUI_API int xuiWidgetGetFlowMode(xui_widget pWidget);`
- 实现: src/xui_widget.c:4231（体 4 行）
- 用法: test_xui/xui_layout_test.c:239; test_xui/xui_window_test.c:299

## xuiWidgetSetDock
- 位置: xui.h:9010  已注释: 否
- 签名: `XUI_API int xuiWidgetSetDock(xui_widget pWidget, int iDock);`
- 实现: src/xui_widget.c:4236（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_checkbox/main.c:112; examples/xui_input/main.c:143; examples/xui_radio/main.c:114

## xuiWidgetGetDock
- 位置: xui.h:9011  已注释: 否
- 签名: `XUI_API int xuiWidgetGetDock(xui_widget pWidget);`
- 实现: src/xui_widget.c:4251（体 4 行）
- 用法: test_xui/xui_layout_test.c:304

## xuiWidgetSetOverflow
- 位置: xui.h:9012  已注释: 否
- 签名: `XUI_API int xuiWidgetSetOverflow(xui_widget pWidget, int iOverflow);`
- 实现: src/xui_widget.c:4256（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_pixel_test.c:121; test_xui/xui_input_test.c:390; test_xui/xui_input_test.c:396

## xuiWidgetGetOverflow
- 位置: xui.h:9013  已注释: 否
- 签名: `XUI_API int xuiWidgetGetOverflow(xui_widget pWidget);`
- 实现: src/xui_widget.c:4269（体 4 行）
- 用法: examples/xui_panel/main.c:327; test_xui/xui_layout_test.c:126; test_xui/xui_panel_test.c:187

## xuiWidgetSetMeasureContainment
- 位置: xui.h:9014  已注释: 否
- 签名: `XUI_API int xuiWidgetSetMeasureContainment(xui_widget pWidget, int iContainment);`
- 实现: src/xui_widget.c:4274（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_layout_test.c:441; test_xui/xui_layout_test.c:444

## xuiWidgetGetMeasureContainment
- 位置: xui.h:9015  已注释: 否
- 签名: `XUI_API int xuiWidgetGetMeasureContainment(xui_widget pWidget);`
- 实现: src/xui_widget.c:4286（体 4 行）
- 用法: test_xui/xui_layout_test.c:442

## xuiWidgetSetGridMetrics
- 位置: xui.h:9016  已注释: 否
- 签名: `XUI_API int xuiWidgetSetGridMetrics(xui_widget pWidget, int iColumnCount, float fItemWidth, float fItemHeight);`
- 实现: src/xui_widget.c:4291（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_button/main.c:318; examples/xui_button/main.c:495; examples/xui_image/main.c:407

## xuiWidgetGetGridMetrics
- 位置: xui.h:9017  已注释: 否
- 签名: `XUI_API int xuiWidgetGetGridMetrics(xui_widget pWidget, int* pColumnCount, float* pItemWidth, float* pItemHeight);`
- 实现: src/xui_widget.c:4308（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_layout_test.c:331

## xuiWidgetSetLayer
- 位置: xui.h:9018  已注释: 否
- 签名: `XUI_API int xuiWidgetSetLayer(xui_widget pWidget, int iLayer, int iZIndex);`
- 实现: src/xui_widget.c:4325（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_input_test.c:369; test_xui/xui_input_test.c:371; test_xui/xui_input_test.c:373

## xuiWidgetGetLayer
- 位置: xui.h:9019  已注释: 否
- 签名: `XUI_API int xuiWidgetGetLayer(xui_widget pWidget, int* pLayer, int* pZIndex);`
- 实现: src/xui_widget.c:4339（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_window/main.c:350; examples/xui_window/main.c:351; test_xui/xui_dock_panel_test.c:965

## xuiWidgetSetPreferredSize
- 位置: xui.h:9020  已注释: 否
- 签名: `XUI_API int xuiWidgetSetPreferredSize(xui_widget pWidget, xui_vec2_t tSize);`
- 实现: src/xui_widget.c:4353（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/tutorial_capture/ch125_main1.c:19; examples/tutorial_capture/ch125_main1.c:27; examples/tutorial_capture/ch127_main1.c:28

## xuiWidgetGetPreferredSize
- 位置: xui.h:9021  已注释: 否
- 签名: `XUI_API xui_vec2_t xuiWidgetGetPreferredSize(xui_widget pWidget);`
- 实现: src/xui_widget.c:4366（体 7 行）

## xuiWidgetSetMinSize
- 位置: xui.h:9022  已注释: 否
- 签名: `XUI_API int xuiWidgetSetMinSize(xui_widget pWidget, xui_vec2_t tSize);`
- 实现: src/xui_widget.c:4374（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_layout_test.c:408; test_xui/xui_layout_test.c:410; test_xui/xui_layout_test.c:412

## xuiWidgetGetMinSize
- 位置: xui.h:9023  已注释: 否
- 签名: `XUI_API xui_vec2_t xuiWidgetGetMinSize(xui_widget pWidget);`
- 实现: src/xui_widget.c:4387（体 7 行）

## xuiWidgetSetMaxSize
- 位置: xui.h:9024  已注释: 否
- 签名: `XUI_API int xuiWidgetSetMaxSize(xui_widget pWidget, xui_vec2_t tSize);`
- 实现: src/xui_widget.c:4395（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiWidgetGetMaxSize
- 位置: xui.h:9025  已注释: 否
- 签名: `XUI_API xui_vec2_t xuiWidgetGetMaxSize(xui_widget pWidget);`
- 实现: src/xui_widget.c:4408（体 7 行）

## xuiWidgetSetMargin
- 位置: xui.h:9026  已注释: 否
- 签名: `XUI_API int xuiWidgetSetMargin(xui_widget pWidget, xui_thickness_t tMargin);`
- 实现: src/xui_widget.c:4416（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/tutorial_capture/ch121_main1.c:18; examples/tutorial_capture/ch122_main1.c:18; examples/tutorial_capture/ch122_main1.c:25

## xuiWidgetGetMargin
- 位置: xui.h:9027  已注释: 否
- 签名: `XUI_API xui_thickness_t xuiWidgetGetMargin(xui_widget pWidget);`
- 实现: src/xui_widget.c:4428（体 10 行）

## xuiWidgetSetPadding
- 位置: xui.h:9028  已注释: 否
- 签名: `XUI_API int xuiWidgetSetPadding(xui_widget pWidget, xui_thickness_t tPadding);`
- 实现: src/xui_widget.c:4439（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/tutorial_capture/ch123_main1.c:15; examples/tutorial_capture/ch127_main1.c:19; examples/tutorial_capture/ch127_main1.c:38

## xuiWidgetGetPadding
- 位置: xui.h:9029  已注释: 否
- 签名: `XUI_API xui_thickness_t xuiWidgetGetPadding(xui_widget pWidget);`
- 实现: src/xui_widget.c:4451（体 10 行）
- 用法: test_xui/xui_msgbox_test.c:268

## xuiWidgetSetGap
- 位置: xui.h:9030  已注释: 否
- 签名: `XUI_API int xuiWidgetSetGap(xui_widget pWidget, float fGap);`
- 实现: src/xui_widget.c:4462（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_button/main.c:317; examples/xui_button/main.c:405; examples/xui_checkbox/main.c:197

## xuiWidgetGetGap
- 位置: xui.h:9031  已注释: 否
- 签名: `XUI_API float xuiWidgetGetGap(xui_widget pWidget);`
- 实现: src/xui_widget.c:4474（体 4 行）

## xuiWidgetSetFlex
- 位置: xui.h:9032  已注释: 否
- 签名: `XUI_API int xuiWidgetSetFlex(xui_widget pWidget, float fGrow, float fShrink);`
- 实现: src/xui_widget.c:4479（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_ime_common/main.inc.c:168; examples/xui_layout/main.c:63; examples/xui_separator/main.c:354

## xuiWidgetGetFlex
- 位置: xui.h:9033  已注释: 否
- 签名: `XUI_API int xuiWidgetGetFlex(xui_widget pWidget, float* pGrow, float* pShrink);`
- 实现: src/xui_widget.c:4489（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWidgetSetAlign
- 位置: xui.h:9034  已注释: 否
- 签名: `XUI_API int xuiWidgetSetAlign(xui_widget pWidget, int iAlignX, int iAlignY);`
- 实现: src/xui_widget.c:4503（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/tutorial_capture/ch162_main1.c:16; examples/tutorial_capture/ch202_main1.c:41; examples/tutorial_capture/ch204_main1.c:44

## xuiWidgetGetAlign
- 位置: xui.h:9035  已注释: 否
- 签名: `XUI_API int xuiWidgetGetAlign(xui_widget pWidget, int* pAlignX, int* pAlignY);`
- 实现: src/xui_widget.c:4516（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWidgetSetTableSize
- 位置: xui.h:9036  已注释: 否
- 签名: `XUI_API int xuiWidgetSetTableSize(xui_widget pWidget, int iRows, int iColumns);`
- 实现: src/xui_widget.c:4530（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch128_main1.c:18; test_xui/xui_layout_test.c:257

## xuiWidgetGetTableSize
- 位置: xui.h:9037  已注释: 否
- 签名: `XUI_API int xuiWidgetGetTableSize(xui_widget pWidget, int* pRows, int* pColumns);`
- 实现: src/xui_widget.c:4548（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_layout_test.c:259

## xuiWidgetSetTableRow
- 位置: xui.h:9038  已注释: 否
- 签名: `XUI_API int xuiWidgetSetTableRow(xui_widget pWidget, int iRow, const xui_table_track_t* pTrack);`
- 实现: src/xui_widget.c:4562（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_layout_test.c:271

## xuiWidgetGetTableRow
- 位置: xui.h:9039  已注释: 否
- 签名: `XUI_API int xuiWidgetGetTableRow(xui_widget pWidget, int iRow, xui_table_track_t* pTrack);`
- 实现: src/xui_widget.c:4574（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWidgetSetTableColumn
- 位置: xui.h:9040  已注释: 否
- 签名: `XUI_API int xuiWidgetSetTableColumn(xui_widget pWidget, int iColumn, const xui_table_track_t* pTrack);`
- 实现: src/xui_widget.c:4583（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_layout_test.c:262; test_xui/xui_layout_test.c:265; test_xui/xui_layout_test.c:268

## xuiWidgetGetTableColumn
- 位置: xui.h:9041  已注释: 否
- 签名: `XUI_API int xuiWidgetGetTableColumn(xui_widget pWidget, int iColumn, xui_table_track_t* pTrack);`
- 实现: src/xui_widget.c:4595（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_layout_test.c:273

## xuiWidgetSetTableCell
- 位置: xui.h:9042  已注释: 否
- 签名: `XUI_API int xuiWidgetSetTableCell(xui_widget pWidget, int iRow, int iColumn, int iRowSpan, int iColumnSpan);`
- 实现: src/xui_widget.c:4604（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_layout_test.c:275; test_xui/xui_layout_test.c:277; test_xui/xui_layout_test.c:279

## xuiWidgetGetTableCell
- 位置: xui.h:9043  已注释: 否
- 签名: `XUI_API int xuiWidgetGetTableCell(xui_widget pWidget, int* pRow, int* pColumn, int* pRowSpan, int* pColumnSpan);`
- 实现: src/xui_widget.c:4616（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_layout_test.c:281

## xuiWidgetMeasure
- 位置: xui.h:9044  已注释: 否
- 签名: `XUI_API int xuiWidgetMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pMeasuredSize);`
- 实现: src/xui_widget.c:4646（体 11 行）
- 用法: test_xui/xui_canvas_test.c:93; test_xui/xui_canvas_test.c:145; test_xui/xui_checkbox_test.c:83

## xuiWidgetArrange
- 位置: xui.h:9046  已注释: 是
- 签名: `XUI_API int xuiWidgetArrange(xui_widget pWidget, xui_rect_t tRect);`
- 既有注释: /* Immediate root arrangement. Attached widgets must use xuiWidgetArrangeChild. */
- 实现: src/xui_widget.c:4681（体 11 行）
- 用法: test_xui/xui_layout_test.c:64; test_xui/xui_layout_test.c:80; test_xui/xui_layout_test.c:436

## xuiWidgetArrangeChild
- 位置: xui.h:9048  已注释: 是
- 签名: `XUI_API int xuiWidgetArrangeChild(xui_widget pParent, xui_widget pChild, xui_rect_t tRect);`
- 既有注释: /* Immediate subtree arrangement outside layout-children callbacks, in parent-local pixels. */
- 实现: src/xui_widget.c:4693（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_layout_test.c:66; test_xui/xui_layout_test.c:74; test_xui/xui_layout_test.c:438

## xuiWidgetSetVisible
- 位置: xui.h:9049  已注释: 否
- 签名: `XUI_API int xuiWidgetSetVisible(xui_widget pWidget, int bVisible);`
- 实现: src/xui_widget.c:4728（体 11 行）
- 用法: test_xui/xui_accessibility_test.c:86; test_xui/xui_accessibility_test.c:317; test_xui/xui_accessibility_test.c:324

## xuiWidgetGetVisible
- 位置: xui.h:9050  已注释: 否
- 签名: `XUI_API int xuiWidgetGetVisible(xui_widget pWidget);`
- 实现: src/xui_widget.c:4740（体 4 行）
- 用法: examples/xui_dockpanel/main.c:414; examples/xui_dockpanel/main.c:433; examples/xui_window/main.c:311

## xuiWidgetGetEffectiveVisible
- 位置: xui.h:9051  已注释: 否
- 签名: `XUI_API int xuiWidgetGetEffectiveVisible(xui_widget pWidget);`
- 实现: src/xui_widget.c:4745（体 4 行）
- 用法: test_xui/xui_state_event_test.c:144; test_xui/xui_state_event_test.c:156; test_xui/xui_state_event_test.c:159

## xuiWidgetSetEnabled
- 位置: xui.h:9052  已注释: 否
- 签名: `XUI_API int xuiWidgetSetEnabled(xui_widget pWidget, int bEnabled);`
- 实现: src/xui_widget.c:4774（体 11 行）
- 用法: examples/audit_xui_basic/main.c:23; examples/audit_xui_basic/main.c:36; examples/audit_xui_basic/main.c:47

## xuiWidgetGetEnabled
- 位置: xui.h:9053  已注释: 否
- 签名: `XUI_API int xuiWidgetGetEnabled(xui_widget pWidget);`
- 实现: src/xui_widget.c:4786（体 4 行）
- 用法: examples/xui_button/main.c:517; examples/xui_checkbox/main.c:247; examples/xui_datepicker/main.c:380

## xuiWidgetGetEffectiveEnabled
- 位置: xui.h:9054  已注释: 否
- 签名: `XUI_API int xuiWidgetGetEffectiveEnabled(xui_widget pWidget);`
- 实现: src/xui_widget.c:4791（体 4 行）
- 用法: test_xui/xui_state_event_test.c:145; test_xui/xui_state_event_test.c:149; test_xui/xui_state_event_test.c:153

## xuiWidgetGetInputState
- 位置: xui.h:9055  已注释: 否
- 签名: `XUI_API uint32_t xuiWidgetGetInputState(xui_widget pWidget);`
- 实现: src/xui_widget.c:4796（体 13 行）
- 用法: examples/xui_input_layer/main.c:217; test_xui/xui_input_test.c:470; test_xui/xui_input_test.c:477

## xuiWidgetSetHitTestVisible
- 位置: xui.h:9056  已注释: 否
- 签名: `XUI_API int xuiWidgetSetHitTestVisible(xui_widget pWidget, int bVisible);`
- 实现: src/xui_widget.c:4827（体 11 行）
- 用法: examples/xui_carousel/main.c:172; examples/xui_checkcard/main.c:214; examples/xui_checkcard/main.c:235

## xuiWidgetGetHitTestVisible
- 位置: xui.h:9057  已注释: 否
- 签名: `XUI_API int xuiWidgetGetHitTestVisible(xui_widget pWidget);`
- 实现: src/xui_widget.c:4839（体 4 行）
- 用法: test_xui/xui_check_card_test.c:178

## xuiWidgetGetEffectiveHitTestVisible
- 位置: xui.h:9058  已注释: 否
- 签名: `XUI_API int xuiWidgetGetEffectiveHitTestVisible(xui_widget pWidget);`
- 实现: src/xui_widget.c:4844（体 10 行）
- 用法: test_xui/xui_state_event_test.c:146; test_xui/xui_state_event_test.c:163

## xuiWidgetSetFocusable
- 位置: xui.h:9059  已注释: 否
- 签名: `XUI_API int xuiWidgetSetFocusable(xui_widget pWidget, int bFocusable);`
- 实现: src/xui_widget.c:4878（体 11 行）
- 用法: examples/xui_input_layer/main.c:358; examples/xui_input_layer/main.c:359; test_xui/xui_callback_lifetime_test.c:25

## xuiWidgetGetFocusable
- 位置: xui.h:9060  已注释: 否
- 签名: `XUI_API int xuiWidgetGetFocusable(xui_widget pWidget);`
- 实现: src/xui_widget.c:4890（体 4 行）
- 用法: test_xui/xui_check_card_test.c:129; test_xui/xui_grid_focus_test.c:207; test_xui/xui_page_test.c:133

## xuiWidgetGetEffectiveFocusable
- 位置: xui.h:9061  已注释: 否
- 签名: `XUI_API int xuiWidgetGetEffectiveFocusable(xui_widget pWidget);`
- 实现: src/xui_widget.c:4895（体 13 行）
- 用法: test_xui/xui_state_event_test.c:147; test_xui/xui_state_event_test.c:243

## xuiWidgetSetTabStop
- 位置: xui.h:9062  已注释: 否
- 签名: `XUI_API int xuiWidgetSetTabStop(xui_widget pWidget, int bTabStop);`
- 实现: src/xui_widget.c:4909（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:140; test_xui/xui_chart_breadcrumb_keyboard_test.c:574; test_xui/xui_dock_panel_test.c:465

## xuiWidgetGetTabStop
- 位置: xui.h:9063  已注释: 否
- 签名: `XUI_API int xuiWidgetGetTabStop(xui_widget pWidget);`
- 实现: src/xui_widget.c:4918（体 4 行）
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:186; test_xui/xui_chart_breadcrumb_keyboard_test.c:261; test_xui/xui_chart_breadcrumb_keyboard_test.c:265

## xuiWidgetSetTabIndex
- 位置: xui.h:9064  已注释: 否
- 签名: `XUI_API int xuiWidgetSetTabIndex(xui_widget pWidget, int iTabIndex);`
- 实现: src/xui_widget.c:4923（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_input_test.c:857; test_xui/xui_input_test.c:859

## xuiWidgetGetTabIndex
- 位置: xui.h:9065  已注释: 否
- 签名: `XUI_API int xuiWidgetGetTabIndex(xui_widget pWidget);`
- 实现: src/xui_widget.c:4932（体 4 行）

## xuiWidgetSetFocusScope
- 位置: xui.h:9066  已注释: 否
- 签名: `XUI_API int xuiWidgetSetFocusScope(xui_widget pWidget, int bFocusScope);`
- 实现: src/xui_widget.c:4937（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_input_test.c:875

## xuiWidgetGetFocusScope
- 位置: xui.h:9067  已注释: 否
- 签名: `XUI_API int xuiWidgetGetFocusScope(xui_widget pWidget);`
- 实现: src/xui_widget.c:4946（体 4 行）

## xuiWidgetSetDefaultAction
- 位置: xui.h:9068  已注释: 否
- 签名: `XUI_API int xuiWidgetSetDefaultAction(xui_widget pWidget, xui_widget_action_proc onAction, void* pUser);`
- 实现: src/xui_widget.c:4951（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:132; test_xui/xui_input_test.c:882

## xuiWidgetSetCancelAction
- 位置: xui.h:9069  已注释: 否
- 签名: `XUI_API int xuiWidgetSetCancelAction(xui_widget pWidget, xui_widget_action_proc onAction, void* pUser);`
- 实现: src/xui_widget.c:4961（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_input_test.c:884

## xuiWidgetSetDragEnabled
- 位置: xui.h:9070  已注释: 否
- 签名: `XUI_API int xuiWidgetSetDragEnabled(xui_widget pWidget, int bEnabled);`
- 实现: src/xui_widget.c:4971（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_drag_drop_test.c:100; test_xui/xui_input_test.c:796; test_xui/xui_state_event_test.c:209

## xuiWidgetGetDragEnabled
- 位置: xui.h:9071  已注释: 否
- 签名: `XUI_API int xuiWidgetGetDragEnabled(xui_widget pWidget);`
- 实现: src/xui_widget.c:4986（体 4 行）

## xuiWidgetSetDropEnabled
- 位置: xui.h:9072  已注释: 否
- 签名: `XUI_API int xuiWidgetSetDropEnabled(xui_widget pWidget, int bEnabled);`
- 实现: src/xui_widget.c:4991（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_drag_drop_reentry_test.c:230; test_xui/xui_drag_drop_test.c:101; test_xui/xui_proxy_drag_drop_test.c:71

## xuiWidgetGetDropEnabled
- 位置: xui.h:9073  已注释: 否
- 签名: `XUI_API int xuiWidgetGetDropEnabled(xui_widget pWidget);`
- 实现: src/xui_widget.c:5004（体 4 行）

## xuiWidgetSetImeMode
- 位置: xui.h:9074  已注释: 否
- 签名: `XUI_API int xuiWidgetSetImeMode(xui_widget pWidget, int iImeMode);`
- 实现: src/xui_widget.c:5097（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_input_test.c:895; test_xui/xui_layout_generation_test.c:40; test_xui/xui_popup_focus_test.c:230

## xuiWidgetGetImeMode
- 位置: xui.h:9075  已注释: 否
- 签名: `XUI_API int xuiWidgetGetImeMode(xui_widget pWidget);`
- 实现: src/xui_widget.c:5116（体 4 行）
- 用法: test_xui/xui_code_edit_test.c:737; test_xui/xui_input_widget_test.c:587; test_xui/xui_input_widget_test.c:609

## xuiWidgetSetImeCandidateRect
- 位置: xui.h:9076  已注释: 否
- 签名: `XUI_API int xuiWidgetSetImeCandidateRect(xui_widget pWidget, xui_widget_ime_rect_proc onRect, void* pUser);`
- 实现: src/xui_widget.c:5121（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_input_test.c:897; test_xui/xui_layout_generation_test.c:41; test_xui/xui_popup_focus_test.c:231

## xuiWidgetGetImeCandidateRect
- 位置: xui.h:9077  已注释: 否
- 签名: `XUI_API int xuiWidgetGetImeCandidateRect(xui_widget pWidget, xui_widget_ime_rect_proc* pRect, void** ppUser);`
- 实现: src/xui_widget.c:5134（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWidgetSetAccessibilityProvider
- 位置: xui.h:9078  已注释: 否
- 签名: `XUI_API int xuiWidgetSetAccessibilityProvider(xui_widget pWidget, xui_accessible_count_proc onCount, xui_accessible_get_proc onGet, xui_accessible_action_proc onAction, void* pUser);`
- 实现: src/xui_accessibility.c:82（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:120; test_xui/xui_accessibility_test.c:284; test_xui/xui_callback_lifetime_test.c:199

## xuiWidgetSetAccessibleName
- 位置: xui.h:9080  已注释: 否
- 签名: `XUI_API int xuiWidgetSetAccessibleName(xui_widget pWidget, const char* sName);`
- 实现: src/xui_accessibility.c:111（体 4 行）
- 用法: examples/xui_richedit/main.c:263; test_xui/xui_accessibility_test.c:127; test_xui/xui_accessibility_test.c:283

## xuiWidgetGetAccessibleName
- 位置: xui.h:9081  已注释: 否
- 签名: `XUI_API const char* xuiWidgetGetAccessibleName(xui_widget pWidget);`
- 实现: src/xui_accessibility.c:116（体 4 行）
- 用法: test_xui/xui_accessibility_test.c:126

## xuiWidgetSetAccessibleDescription
- 位置: xui.h:9082  已注释: 否
- 签名: `XUI_API int xuiWidgetSetAccessibleDescription(xui_widget pWidget, const char* sDescription);`
- 实现: src/xui_accessibility.c:121（体 5 行）

## xuiWidgetGetAccessibleDescription
- 位置: xui.h:9083  已注释: 否
- 签名: `XUI_API const char* xuiWidgetGetAccessibleDescription(xui_widget pWidget);`
- 实现: src/xui_accessibility.c:127（体 4 行）

## xuiWidgetGetAccessibleNodeCount
- 位置: xui.h:9084  已注释: 否
- 签名: `XUI_API int xuiWidgetGetAccessibleNodeCount(xui_widget pWidget);`
- 实现: src/xui_accessibility.c:132（体 23 行）
- 用法: test_xui/xui_accessibility_test.c:96; test_xui/xui_accessibility_test.c:291; test_xui/xui_accessibility_test.c:413

## xuiWidgetGetAccessibleNode
- 位置: xui.h:9093  已注释: 是
- 签名: `XUI_API int xuiWidgetGetAccessibleNode(xui_widget pWidget, int iIndex, xui_accessible_node_t* pNode);`
- 既有注释: /* Every ordinary widget exposes one local node (id 1). Enumerate the physical * tree with the Widget parent/child APIs; provider node ids remain widget-local. * Strings are borrowed until the next widget/provider mutation. Queries which * destroy or replace their provider fail and clear the result. Older node sizes * ending at iColumnCount are accepted. No native platform bridge is installed. * Automatic semantic notifications are coalesced at the next outer xuiUpdate; * explicit NotifyAccessibility remains synchronous. Repeated notifications of * the same change during a callback/drain are suppressed. */
- 实现: src/xui_accessibility.c:171（体 56 行）
- 用法: test_xui/xui_accessibility_test.c:116; test_xui/xui_accessibility_test.c:173; test_xui/xui_accessibility_test.c:306

## xuiWidgetPerformAccessibleAction
- 位置: xui.h:9094  已注释: 否
- 签名: `XUI_API int xuiWidgetPerformAccessibleAction(xui_widget pWidget, uint64_t iNodeId, int iAction, const void* pData);`
- 实现: src/xui_accessibility.c:228（体 39 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:46; test_xui/xui_accessibility_test.c:147; test_xui/xui_accessibility_test.c:299

## xuiWidgetGetAccessibilityRevision
- 位置: xui.h:9095  已注释: 否
- 签名: `XUI_API uint32_t xuiWidgetGetAccessibilityRevision(xui_widget pWidget);`
- 实现: src/xui_accessibility.c:268（体 4 行）
- 用法: test_xui/xui_rich_edit_test.c:566

## xuiWidgetNotifyAccessibility
- 位置: xui.h:9096  已注释: 否
- 签名: `XUI_API int xuiWidgetNotifyAccessibility(xui_widget pWidget, int iEventType, uint64_t iNodeId);`
- 实现: src/xui_accessibility.c:273（体 27 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_accessibility_test.c:196; test_xui/xui_accessibility_test.c:293; test_xui/xui_accessibility_test.c:376

## xuiWidgetSetEventCallback
- 位置: xui.h:9099  已注释: 否
- 签名: `XUI_API int xuiWidgetSetEventCallback(xui_widget pWidget, xui_widget_event_proc onEvent, void* pUser);`
- 实现: src/xui_widget.c:5164（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_multitouch/main.c:355; test_xui/xui_callback_lifetime_test.c:141; test_xui/xui_callback_lifetime_test.c:163

## xuiWidgetGetEventCallback
- 位置: xui.h:9100  已注释: 否
- 签名: `XUI_API int xuiWidgetGetEventCallback(xui_widget pWidget, xui_widget_event_proc* pEvent, void** ppUser);`
- 实现: src/xui_widget.c:5174（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_input_test.c:419

## xuiWidgetSetEventHandler
- 位置: xui.h:9101  已注释: 否
- 签名: `XUI_API int xuiWidgetSetEventHandler(xui_widget pWidget, int iEventType, xui_widget_event_proc onEvent, void* pUser);`
- 实现: src/xui_widget.c:5244（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_accessibility_test.c:365; test_xui/xui_dock_panel_test.c:471; test_xui/xui_input_test.c:500

## xuiWidgetGetEventHandler
- 位置: xui.h:9102  已注释: 否
- 签名: `XUI_API int xuiWidgetGetEventHandler(xui_widget pWidget, int iEventType, xui_widget_event_proc* pEvent, void** ppUser);`
- 实现: src/xui_widget.c:5267（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_basic_canvas_test.c:15; test_xui/xui_style_basic_input_test.c:14

## xuiWidgetSetEventInterest
- 位置: xui.h:9103  已注释: 否
- 签名: `XUI_API int xuiWidgetSetEventInterest(xui_widget pWidget, uint64_t iEventMask, int bEnabled);`
- 实现: src/xui_widget.c:5283（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_multitouch/main.c:356

## xuiWidgetGetEventMask
- 位置: xui.h:9104  已注释: 否
- 签名: `XUI_API uint64_t xuiWidgetGetEventMask(xui_widget pWidget);`
- 实现: src/xui_widget.c:5297（体 4 行）
- 用法: test_xui/xui_input_test.c:561; test_xui/xui_input_test.c:627; test_xui/xui_input_test.c:629

## xuiWidgetGetSubtreeEventMask
- 位置: xui.h:9105  已注释: 否
- 签名: `XUI_API uint64_t xuiWidgetGetSubtreeEventMask(xui_widget pWidget);`
- 实现: src/xui_widget.c:5302（体 4 行）
- 用法: test_xui/xui_input_test.c:761

## xuiWidgetSetCursorQueryCallback
- 位置: xui.h:9106  已注释: 否
- 签名: `XUI_API int xuiWidgetSetCursorQueryCallback(xui_widget pWidget, xui_widget_cursor_proc onQueryCursor, void* pUser);`
- 实现: src/xui_widget.c:5188（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWidgetGetCursorQueryCallback
- 位置: xui.h:9107  已注释: 否
- 签名: `XUI_API int xuiWidgetGetCursorQueryCallback(xui_widget pWidget, xui_widget_cursor_proc* pQueryCursor, void** ppUser);`
- 实现: src/xui_widget.c:5198（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWidgetSetTooltipText
- 位置: xui.h:9109  已注释: 否
- 签名: `XUI_API int xuiWidgetSetTooltipText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_widget.c:6147（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch158_main1.c:19; examples/tutorial_capture/ch158_main1.c:27; test_xui/xui_input_test.c:556

## xuiWidgetSetTooltip
- 位置: xui.h:9110  已注释: 否
- 签名: `XUI_API int xuiWidgetSetTooltip(xui_widget pWidget, const xui_tooltip_desc_t* pDesc);`
- 实现: src/xui_widget.c:6162（体 45 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/xui_tooltip/main.c:314; test_xui/xui_style_tooltip_test.c:74

## xuiWidgetSetTooltipResolver
- 位置: xui.h:9111  已注释: 否
- 签名: `XUI_API int xuiWidgetSetTooltipResolver(xui_widget pWidget, xui_tooltip_resolve_proc onResolve, void* pUser);`
- 实现: src/xui_widget.c:6208（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_tooltip/main.c:409; test_xui/xui_input_test.c:584

## xuiWidgetClearTooltip
- 位置: xui.h:9112  已注释: 否
- 签名: `XUI_API int xuiWidgetClearTooltip(xui_widget pWidget);`
- 实现: src/xui_widget.c:6230（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_input_test.c:625; test_xui/xui_input_test.c:628

## xuiWidgetGetTooltip
- 位置: xui.h:9113  已注释: 否
- 签名: `XUI_API const xui_tooltip_desc_t* xuiWidgetGetTooltip(xui_widget pWidget);`
- 实现: src/xui_widget.c:6251（体 4 行）
- 用法: examples/xui_tooltip/main.c:448; examples/xui_tooltip/main.c:449; examples/xui_tooltip/main.c:450

## xuiWidgetTooltipIsOpen
- 位置: xui.h:9114  已注释: 否
- 签名: `XUI_API int xuiWidgetTooltipIsOpen(xui_context pContext);`
- 实现: src/xui_widget.c:6256（体 4 行）
- 用法: examples/xui_tooltip/main.c:506; examples/xui_tooltip/main.c:523; test_xui/xui_dock_panel_test.c:775

## xuiWidgetTooltipGetOwner
- 位置: xui.h:9115  已注释: 否
- 签名: `XUI_API xui_widget xuiWidgetTooltipGetOwner(xui_context pContext);`
- 实现: src/xui_widget.c:6261（体 4 行）
- 用法: examples/xui_tooltip/main.c:467; examples/xui_tooltip/main.c:475; examples/xui_tooltip/main.c:483

## xuiWidgetTooltipGetRect
- 位置: xui.h:9116  已注释: 否
- 签名: `XUI_API xui_rect_t xuiWidgetTooltipGetRect(xui_context pContext);`
- 实现: src/xui_widget.c:6266（体 10 行）
- 用法: examples/xui_tooltip/main.c:466; examples/xui_tooltip/main.c:474; examples/xui_tooltip/main.c:482

## xuiWidgetInvalidate
- 位置: xui.h:9128  已注释: 否
- 签名: `XUI_API int xuiWidgetInvalidate(xui_widget pWidget, uint32_t iFlags);`
- 实现: src/xui_widget.c:6277（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_multitouch/main.c:212; examples/xui_multitouch/main.c:215; examples/xui_virtual_joystick/main.c:169

## xuiWidgetInvalidateRect
- 位置: xui.h:9129  已注释: 否
- 签名: `XUI_API int xuiWidgetInvalidateRect(xui_widget pWidget, xui_rect_t tRect, uint32_t iFlags);`
- 实现: src/xui_widget.c:6300（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiWidgetGetDirtyFlags
- 位置: xui.h:9130  已注释: 否
- 签名: `XUI_API uint32_t xuiWidgetGetDirtyFlags(xui_widget pWidget);`
- 实现: src/xui_widget.c:6315（体 4 行）
- 用法: test_xui/xui_render_schedule_test.c:1549; test_xui/xui_style_chrome_test.c:122; test_xui/xui_style_chrome_test.c:123

## xuiWidgetClearDirty
- 位置: xui.h:9131  已注释: 否
- 签名: `XUI_API void xuiWidgetClearDirty(xui_widget pWidget, uint32_t iFlags);`
- 实现: src/xui_widget.c:6320（体 11 行）
- 用法: test_xui/xui_style_chrome_test.c:119; test_xui/xui_style_test.c:234

## xuiWidgetSetStyleName
- 位置: xui.h:9133  已注释: 否
- 签名: `XUI_API int xuiWidgetSetStyleName(xui_widget pWidget, const char* sName);`
- 实现: src/xui_widget.c:6332（体 29 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_style_test.c:195

## xuiWidgetGetStyleName
- 位置: xui.h:9134  已注释: 否
- 签名: `XUI_API const char* xuiWidgetGetStyleName(xui_widget pWidget);`
- 实现: src/xui_widget.c:6362（体 4 行）
- 用法: test_xui/xui_style_test.c:197

## xuiWidgetAddStyleClass
- 位置: xui.h:9135  已注释: 否
- 签名: `XUI_API int xuiWidgetAddStyleClass(xui_widget pWidget, const char* sClass);`
- 实现: src/xui_widget.c:6367（体 28 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/xui_label/main.c:349; test_xui/xui_hyperlink_test.c:194; test_xui/xui_label_test.c:952

## xuiWidgetRemoveStyleClass
- 位置: xui.h:9136  已注释: 否
- 签名: `XUI_API int xuiWidgetRemoveStyleClass(xui_widget pWidget, const char* sClass);`
- 实现: src/xui_widget.c:6396（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_basic_canvas_test.c:74; test_xui/xui_style_chrome_test.c:144; test_xui/xui_style_test.c:274

## xuiWidgetClearStyleClasses
- 位置: xui.h:9137  已注释: 否
- 签名: `XUI_API void xuiWidgetClearStyleClasses(xui_widget pWidget);`
- 实现: src/xui_widget.c:6418（体 13 行）
- 用法: test_xui/xui_label_test.c:980; test_xui/xui_style_containers_test.c:200; test_xui/xui_style_containers_test.c:326

## xuiWidgetHasStyleClass
- 位置: xui.h:9138  已注释: 否
- 签名: `XUI_API int xuiWidgetHasStyleClass(xui_widget pWidget, const char* sClass);`
- 实现: src/xui_widget.c:6432（体 14 行）
- 用法: test_xui/xui_style_test.c:202; test_xui/xui_style_test.c:276

## xuiWidgetGetStyleClassCount
- 位置: xui.h:9139  已注释: 否
- 签名: `XUI_API int xuiWidgetGetStyleClassCount(xui_widget pWidget);`
- 实现: src/xui_widget.c:6447（体 4 行）
- 用法: test_xui/xui_style_test.c:200; test_xui/xui_style_test.c:280

## xuiWidgetGetStyleClass
- 位置: xui.h:9140  已注释: 否
- 签名: `XUI_API const char* xuiWidgetGetStyleClass(xui_widget pWidget, int iIndex);`
- 实现: src/xui_widget.c:6452（体 7 行）
- 返回码: NULL
- 用法: test_xui/xui_style_test.c:201

## xuiWidgetSetInlineStyle
- 位置: xui.h:9141  已注释: 否
- 签名: `XUI_API int xuiWidgetSetInlineStyle(xui_widget pWidget, const xui_style_property_t* pProperties, int iPropertyCount);`
- 实现: src/xui_widget.c:6460（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_test.c:215; test_xui/xui_code_edit_test.c:651; test_xui/xui_code_edit_test.c:661

## xuiWidgetGetInlineStyleProperty
- 位置: xui.h:9142  已注释: 否
- 签名: `XUI_API int xuiWidgetGetInlineStyleProperty(xui_widget pWidget, const char* sName, xui_style_property_t* pProperty);`
- 实现: src/xui_widget.c:6487（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_FILE_NOT_FOUND, XUI_OK
- 用法: test_xui/xui_style_test.c:208

## xuiWidgetResolveStyle
- 位置: xui.h:9143  已注释: 否
- 签名: `XUI_API int xuiWidgetResolveStyle(xui_widget pWidget);`
- 实现: src/xui_widget.c:6502（体 39 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWidgetGetStyleGeneration
- 位置: xui.h:9144  已注释: 否
- 签名: `XUI_API uint32_t xuiWidgetGetStyleGeneration(xui_widget pWidget);`
- 实现: src/xui_widget.c:6542（体 4 行）

## xuiWidgetGetStyleHash
- 位置: xui.h:9145  已注释: 否
- 签名: `XUI_API uint32_t xuiWidgetGetStyleHash(xui_widget pWidget);`
- 实现: src/xui_widget.c:6547（体 4 行）
- 用法: test_xui/xui_style_test.c:225; test_xui/xui_style_test.c:228; test_xui/xui_style_test.c:236

## xuiWidgetGetResolvedStyleProperty
- 位置: xui.h:9146  已注释: 否
- 签名: `XUI_API int xuiWidgetGetResolvedStyleProperty(xui_widget pWidget, const char* sName, xui_style_property_t* pProperty);`
- 实现: src/xui_widget.c:6552（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_FILE_NOT_FOUND, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1327; test_xui/xui_prepare_paint_test.c:40; test_xui/xui_style_test.c:212

## xuiWidgetGetResolvedStylePropertyCount
- 位置: xui.h:9147  已注释: 否
- 签名: `XUI_API int xuiWidgetGetResolvedStylePropertyCount(xui_widget pWidget);`
- 实现: src/xui_widget.c:6567（体 4 行）
- 用法: test_xui/xui_style_test.c:223

## xuiWidgetGetResolvedStylePropertyAt
- 位置: xui.h:9148  已注释: 否
- 签名: `XUI_API int xuiWidgetGetResolvedStylePropertyAt(xui_widget pWidget, int iIndex, xui_style_property_t* pProperty);`
- 实现: src/xui_widget.c:6572（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWidgetSetStateId
- 位置: xui.h:9150  已注释: 否
- 签名: `XUI_API int xuiWidgetSetStateId(xui_widget pWidget, uint32_t iStateId);`
- 实现: src/xui_widget.c:6581（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_cache_transition_test.c:47; test_xui/xui_cache_transition_test.c:70; test_xui/xui_cache_transition_test.c:75

## xuiWidgetGetStateId
- 位置: xui.h:9151  已注释: 否
- 签名: `XUI_API uint32_t xuiWidgetGetStateId(xui_widget pWidget);`
- 实现: src/xui_widget.c:6593（体 4 行）
- 用法: examples/xui_label/main.c:487; test_xui/xui_accordion_test.c:185; test_xui/xui_accordion_test.c:188

## xuiWidgetSetCachePolicy
- 位置: xui.h:9153  已注释: 是
- 签名: `XUI_API int xuiWidgetSetCachePolicy(xui_widget pWidget, const xui_cache_policy_t* pPolicy);`
- 既有注释: /* An unsupported policy leaves the current policy unchanged. */
- 实现: src/xui_widget.c:6598（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: examples/audit_xui_cache/main.c:8; examples/xui_dockpanel/main.c:220; test_xui/xui_cache_transition_test.c:45

## xuiWidgetGetCachePolicy
- 位置: xui.h:9154  已注释: 否
- 签名: `XUI_API xui_cache_policy_t xuiWidgetGetCachePolicy(xui_widget pWidget);`
- 实现: src/xui_widget.c:6621（体 10 行）
- 用法: test_xui/xui_cache_transition_test.c:42; test_xui/xui_code_edit_test.c:581; test_xui/xui_native_popup_matrix_test.c:98

## xuiWidgetSetCacheRenderCallback
- 位置: xui.h:9155  已注释: 否
- 签名: `XUI_API int xuiWidgetSetCacheRenderCallback(xui_widget pWidget, xui_widget_cache_render_proc onRender, void* pUser);`
- 实现: src/xui_widget.c:6632（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_accordion/main.c:188; examples/xui_breadcrumb/main.c:230; examples/xui_canvas/main.c:272

## xuiWidgetGetCacheRenderCallback
- 位置: xui.h:9156  已注释: 否
- 签名: `XUI_API int xuiWidgetGetCacheRenderCallback(xui_widget pWidget, xui_widget_cache_render_proc* pRender, void** ppUser);`
- 实现: src/xui_widget.c:6645（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_containers_test.c:152; test_xui/xui_style_containers_test.c:268; test_xui/xui_style_containers_test.c:270

## xuiWidgetSetCacheStateCount
- 位置: xui.h:9157  已注释: 否
- 签名: `XUI_API int xuiWidgetSetCacheStateCount(xui_widget pWidget, int iCount);`
- 实现: src/xui_widget.c:6659（体 45 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_ALREADY_INITIALIZED, XUI_OK, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_prepare_paint_test.c:99; test_xui/xui_render_lifetime_test.c:131; test_xui/xui_render_schedule_test.c:1369

## xuiWidgetGetCacheStateCount
- 位置: xui.h:9158  已注释: 否
- 签名: `XUI_API int xuiWidgetGetCacheStateCount(xui_widget pWidget);`
- 实现: src/xui_widget.c:6705（体 4 行）
- 用法: examples/xui_hyperlink/main.c:244; examples/xui_taginput/main.c:285; test_xui/xui_button_test.c:805

## xuiWidgetSetCacheStateId
- 位置: xui.h:9159  已注释: 否
- 签名: `XUI_API int xuiWidgetSetCacheStateId(xui_widget pWidget, int iIndex, uint32_t iStateId);`
- 实现: src/xui_widget.c:6710（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_ALREADY_INITIALIZED, XUI_OK
- 用法: test_xui/xui_prepare_paint_test.c:100; test_xui/xui_prepare_paint_test.c:101; test_xui/xui_prepare_paint_test.c:102

## xuiWidgetGetCacheStateId
- 位置: xui.h:9160  已注释: 否
- 签名: `XUI_API uint32_t xuiWidgetGetCacheStateId(xui_widget pWidget, int iIndex);`
- 实现: src/xui_widget.c:6729（体 7 行）
- 用法: test_xui/xui_hyperlink_test.c:121; test_xui/xui_hyperlink_test.c:122; test_xui/xui_hyperlink_test.c:123

## xuiWidgetGetCacheSurface
- 位置: xui.h:9161  已注释: 否
- 签名: `XUI_API xui_surface xuiWidgetGetCacheSurface(xui_widget pWidget, uint32_t iStateId);`
- 实现: src/xui_widget.c:6737（体 10 行）
- 返回码: NULL
- 用法: examples/xui_inventory_grid/main.c:664; examples/xui_inventory_grid/main.c:670; examples/xui_inventory_grid/main.c:671

## xuiWidgetUpdateBegin
- 位置: xui.h:9162  已注释: 否
- 签名: `XUI_API int xuiWidgetUpdateBegin(xui_widget pWidget, uint32_t iStateId, uint32_t iFlags, uint32_t iClearColor, xui_draw_context* ppDraw);`
- 实现: src/xui_widget.c:6750（体 59 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_ALREADY_INITIALIZED, XUI_ERROR_NOT_INITIALIZED, XUI_OK
- 用法: test_xui/xui_cache_transition_test.c:54; test_xui/xui_render_schedule_test.c:1390; test_xui/xui_widget_test.c:183

## xuiWidgetUpdateEnd
- 位置: xui.h:9163  已注释: 否
- 签名: `XUI_API int xuiWidgetUpdateEnd(xui_widget pWidget, uint32_t iStateId, xui_draw_context pDraw);`
- 实现: src/xui_widget.c:6810（体 32 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: test_xui/xui_cache_transition_test.c:57; test_xui/xui_cache_transition_test.c:84; test_xui/xui_render_schedule_test.c:1392

## xuiWidgetRenderCache
- 位置: xui.h:9164  已注释: 否
- 签名: `XUI_API int xuiWidgetRenderCache(xui_widget pWidget, xui_surface pTarget);`
- 实现: src/xui_widget.c:7360（体 36 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_NOT_INITIALIZED

## xuiWidgetRenderTree
- 位置: xui.h:9165  已注释: 否
- 签名: `XUI_API int xuiWidgetRenderTree(xui_widget pWidget, xui_surface pTarget);`
- 实现: src/xui_widget.c:7427（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_containers_test.c:126

