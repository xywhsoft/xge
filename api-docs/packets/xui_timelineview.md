# 草稿包：xui.h / timelineview（85 条 API）

> 生成 2026-09-10 03:07 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiTimeLineViewGetType
- 位置: xui.h:10261  已注释: 否
- 签名: `XUI_API xui_widget_type xuiTimeLineViewGetType(xui_context pContext);`
- 实现: src/xui_timeline_view.c:2148（体 32 行）
- 返回码: NULL

## xuiTimeLineViewCreate
- 位置: xui.h:10262  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewCreate(xui_context pContext, xui_widget* ppWidget, const xui_timeline_view_desc_t* pDesc);`
- 实现: src/xui_timeline_view.c:2181（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch183_main1.c:19; examples/tutorial_capture/ch203_main1.c:44; examples/xui_timelineview/main.c:365

## xuiTimeLineViewGetFrameWidget
- 位置: xui.h:10263  已注释: 否
- 签名: `XUI_API xui_widget xuiTimeLineViewGetFrameWidget(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:2191（体 5 行）
- 用法: examples/xui_timelineview/main.c:451; examples/xui_timelineview/main.c:453

## xuiTimeLineViewGetViewportWidget
- 位置: xui.h:10264  已注释: 否
- 签名: `XUI_API xui_widget xuiTimeLineViewGetViewportWidget(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:2197（体 5 行）

## xuiTimeLineViewGetModel
- 位置: xui.h:10265  已注释: 否
- 签名: `XUI_API xui_scroll_model_t* xuiTimeLineViewGetModel(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:2203（体 5 行）

## xuiTimeLineViewClear
- 位置: xui.h:10266  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewClear(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:2209（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTimeLineViewSetFont
- 位置: xui.h:10267  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_timeline_view.c:2232（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTimeLineViewGetFont
- 位置: xui.h:10268  已注释: 否
- 签名: `XUI_API xui_font xuiTimeLineViewGetFont(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:2240（体 5 行）

## xuiTimeLineViewSetFrameCount
- 位置: xui.h:10269  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetFrameCount(xui_widget pWidget, int iFrameCount);`
- 实现: src/xui_timeline_view.c:2246（体 29 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_collections_timeline_test.c:44; test_xui/xui_style_collections_timeline_test.c:97

## xuiTimeLineViewGetFrameCount
- 位置: xui.h:10270  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetFrameCount(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:2276（体 5 行）

## xuiTimeLineViewSetFrameRate
- 位置: xui.h:10271  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetFrameRate(xui_widget pWidget, float fFrameRate);`
- 实现: src/xui_timeline_view.c:2282（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTimeLineViewGetFrameRate
- 位置: xui.h:10272  已注释: 否
- 签名: `XUI_API float xuiTimeLineViewGetFrameRate(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:2291（体 5 行）

## xuiTimeLineViewSetCurrentFrame
- 位置: xui.h:10273  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetCurrentFrame(xui_widget pWidget, int iFrame);`
- 实现: src/xui_timeline_view.c:2297（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_timelineview/main.c:402; test_xui/xui_timeline_view_test.c:390

## xuiTimeLineViewGetCurrentFrame
- 位置: xui.h:10274  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetCurrentFrame(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:2304（体 5 行）
- 用法: examples/xui_timelineview/main.c:144; examples/xui_timelineview/main.c:460; examples/xui_timelineview/main.c:558

## xuiTimeLineViewSetMetrics
- 位置: xui.h:10275  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetMetrics(xui_widget pWidget, float fLayerHeaderWidth, float fFrameWidth, float fRowHeight, float fRulerHeight);`
- 实现: src/xui_timeline_view.c:2310（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTimeLineViewGetMetrics
- 位置: xui.h:10276  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetMetrics(xui_widget pWidget, float* pLayerHeaderWidth, float* pFrameWidth, float* pRowHeight, float* pRulerHeight);`
- 实现: src/xui_timeline_view.c:2322（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_timelineview/main.c:439

## xuiTimeLineViewSetFrameWidthRange
- 位置: xui.h:10277  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetFrameWidthRange(xui_widget pWidget, float fMinFrameWidth, float fMaxFrameWidth);`
- 实现: src/xui_timeline_view.c:2333（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTimeLineViewGetFrameWidthRange
- 位置: xui.h:10278  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetFrameWidthRange(xui_widget pWidget, float* pMinFrameWidth, float* pMaxFrameWidth);`
- 实现: src/xui_timeline_view.c:2342（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTimeLineViewSetFrameWidth
- 位置: xui.h:10279  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetFrameWidth(xui_widget pWidget, float fFrameWidth);`
- 实现: src/xui_timeline_view.c:2351（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTimeLineViewGetFrameWidth
- 位置: xui.h:10280  已注释: 否
- 签名: `XUI_API float xuiTimeLineViewGetFrameWidth(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:2360（体 5 行）

## xuiTimeLineViewSetFeatureFlags
- 位置: xui.h:10281  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetFeatureFlags(xui_widget pWidget, int bShowVisibility, int bShowLock);`
- 实现: src/xui_timeline_view.c:2366（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTimeLineViewGetFeatureFlags
- 位置: xui.h:10282  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetFeatureFlags(xui_widget pWidget, int* pShowVisibility, int* pShowLock);`
- 实现: src/xui_timeline_view.c:2375（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTimeLineViewSetScrollbarMode
- 位置: xui.h:10283  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetScrollbarMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_timeline_view.c:2384（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTimeLineViewGetScrollbarMode
- 位置: xui.h:10284  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetScrollbarMode(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:2393（体 5 行）

## xuiTimeLineViewAddLayer
- 位置: xui.h:10285  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewAddLayer(xui_widget pWidget, const char* sName, int* pLayer);`
- 实现: src/xui_timeline_view.c:2399（体 25 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/xui_timelineview/main.c:384; examples/xui_timelineview/main.c:385; examples/xui_timelineview/main.c:386

## xuiTimeLineViewRemoveLayer
- 位置: xui.h:10286  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewRemoveLayer(xui_widget pWidget, int iLayer);`
- 实现: src/xui_timeline_view.c:2425（体 41 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_collections_timeline_test.c:86

## xuiTimeLineViewMoveLayer
- 位置: xui.h:10287  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewMoveLayer(xui_widget pWidget, int iLayer, int iToLayer);`
- 实现: src/xui_timeline_view.c:2467（体 34 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_collections_timeline_test.c:84; test_xui/xui_style_collections_timeline_test.c:85

## xuiTimeLineViewGetLayerCount
- 位置: xui.h:10288  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetLayerCount(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:2502（体 5 行）
- 用法: examples/xui_timelineview/main.c:452; test_xui/xui_timeline_view_test.c:361

## xuiTimeLineViewGetLayer
- 位置: xui.h:10289  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetLayer(xui_widget pWidget, int iLayer, xui_timeline_layer_t* pLayer);`
- 实现: src/xui_timeline_view.c:2508（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_timeline_view_test.c:369

## xuiTimeLineViewSetLayerName
- 位置: xui.h:10290  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetLayerName(xui_widget pWidget, int iLayer, const char* sName);`
- 实现: src/xui_timeline_view.c:2516（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTimeLineViewGetLayerName
- 位置: xui.h:10291  已注释: 否
- 签名: `XUI_API const char* xuiTimeLineViewGetLayerName(xui_widget pWidget, int iLayer);`
- 实现: src/xui_timeline_view.c:2528（体 5 行）

## xuiTimeLineViewSetLayerVisible
- 位置: xui.h:10292  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetLayerVisible(xui_widget pWidget, int iLayer, int bVisible);`
- 实现: src/xui_timeline_view.c:2534（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_collections_timeline_test.c:48; test_xui/xui_timeline_view_test.c:365

## xuiTimeLineViewGetLayerVisible
- 位置: xui.h:10293  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetLayerVisible(xui_widget pWidget, int iLayer);`
- 实现: src/xui_timeline_view.c:2548（体 5 行）
- 用法: test_xui/xui_timeline_view_test.c:366

## xuiTimeLineViewSetLayerLocked
- 位置: xui.h:10294  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetLayerLocked(xui_widget pWidget, int iLayer, int bLocked);`
- 实现: src/xui_timeline_view.c:2554（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_collections_timeline_test.c:49; test_xui/xui_timeline_view_test.c:367

## xuiTimeLineViewGetLayerLocked
- 位置: xui.h:10295  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetLayerLocked(xui_widget pWidget, int iLayer);`
- 实现: src/xui_timeline_view.c:2568（体 5 行）
- 用法: test_xui/xui_timeline_view_test.c:368

## xuiTimeLineViewSetLayerHeight
- 位置: xui.h:10296  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetLayerHeight(xui_widget pWidget, int iLayer, float fHeight);`
- 实现: src/xui_timeline_view.c:2574（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTimeLineViewGetLayerHeight
- 位置: xui.h:10297  已注释: 否
- 签名: `XUI_API float xuiTimeLineViewGetLayerHeight(xui_widget pWidget, int iLayer);`
- 实现: src/xui_timeline_view.c:2583（体 5 行）

## xuiTimeLineViewSetLayerColor
- 位置: xui.h:10298  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetLayerColor(xui_widget pWidget, int iLayer, uint32_t iColor);`
- 实现: src/xui_timeline_view.c:2589（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_timelineview/main.c:388; examples/xui_timelineview/main.c:389; test_xui/xui_style_collections_timeline_test.c:46

## xuiTimeLineViewGetLayerColor
- 位置: xui.h:10299  已注释: 否
- 签名: `XUI_API uint32_t xuiTimeLineViewGetLayerColor(xui_widget pWidget, int iLayer);`
- 实现: src/xui_timeline_view.c:2598（体 5 行）

## xuiTimeLineViewSetLayerUserData
- 位置: xui.h:10300  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetLayerUserData(xui_widget pWidget, int iLayer, void* pUser);`
- 实现: src/xui_timeline_view.c:2604（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTimeLineViewGetLayerUserData
- 位置: xui.h:10301  已注释: 否
- 签名: `XUI_API void* xuiTimeLineViewGetLayerUserData(xui_widget pWidget, int iLayer);`
- 实现: src/xui_timeline_view.c:2612（体 5 行）

## xuiTimeLineViewSetFrame
- 位置: xui.h:10302  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetFrame(xui_widget pWidget, int iLayer, int iFrame, int iType, void* pUser);`
- 实现: src/xui_timeline_view.c:2618（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/xui_timelineview/main.c:390; examples/xui_timelineview/main.c:391; examples/xui_timelineview/main.c:392

## xuiTimeLineViewGetFrame
- 位置: xui.h:10303  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetFrame(xui_widget pWidget, int iLayer, int iFrame, xui_timeline_frame_t* pFrame);`
- 实现: src/xui_timeline_view.c:2643（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_timeline_view_test.c:378; test_xui/xui_timeline_view_test.c:423; test_xui/xui_timeline_view_test.c:434

## xuiTimeLineViewClearFrame
- 位置: xui.h:10304  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewClearFrame(xui_widget pWidget, int iLayer, int iFrame);`
- 实现: src/xui_timeline_view.c:2659（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTimeLineViewSetFrameUserData
- 位置: xui.h:10305  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetFrameUserData(xui_widget pWidget, int iLayer, int iFrame, void* pUser);`
- 实现: src/xui_timeline_view.c:2676（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTimeLineViewGetFrameUserData
- 位置: xui.h:10306  已注释: 否
- 签名: `XUI_API void* xuiTimeLineViewGetFrameUserData(xui_widget pWidget, int iLayer, int iFrame);`
- 实现: src/xui_timeline_view.c:2687（体 8 行）
- 返回码: NULL

## xuiTimeLineViewAddSpan
- 位置: xui.h:10307  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewAddSpan(xui_widget pWidget, int iLayer, int iStartFrame, int iEndFrame, int iType, const char* sLabel, int* pSpanId);`
- 实现: src/xui_timeline_view.c:2696（体 27 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/xui_timelineview/main.c:398; examples/xui_timelineview/main.c:399; examples/xui_timelineview/main.c:400

## xuiTimeLineViewRemoveSpan
- 位置: xui.h:10308  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewRemoveSpan(xui_widget pWidget, int iSpanId);`
- 实现: src/xui_timeline_view.c:2724（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_collections_timeline_test.c:92

## xuiTimeLineViewSetSpan
- 位置: xui.h:10309  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetSpan(xui_widget pWidget, int iSpanId, int iStartFrame, int iEndFrame, int iType, const char* sLabel);`
- 实现: src/xui_timeline_view.c:2740（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTimeLineViewGetSpan
- 位置: xui.h:10310  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetSpan(xui_widget pWidget, int iSpanId, xui_timeline_span_t* pSpan);`
- 实现: src/xui_timeline_view.c:2764（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_collections_timeline_test.c:100; test_xui/xui_timeline_view_test.c:387

## xuiTimeLineViewSetSpanColor
- 位置: xui.h:10311  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetSpanColor(xui_widget pWidget, int iSpanId, uint32_t iColor);`
- 实现: src/xui_timeline_view.c:2775（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_collections_timeline_test.c:55; test_xui/xui_style_collections_timeline_test.c:91; test_xui/xui_style_collections_timeline_test.c:96

## xuiTimeLineViewSetSpanCustomType
- 位置: xui.h:10312  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetSpanCustomType(xui_widget pWidget, int iSpanId, const char* sCustomType);`
- 实现: src/xui_timeline_view.c:2787（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_timeline_view_test.c:385

## xuiTimeLineViewSetSpanUserData
- 位置: xui.h:10313  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetSpanUserData(xui_widget pWidget, int iSpanId, void* pUser);`
- 实现: src/xui_timeline_view.c:2798（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTimeLineViewGetSpanUserData
- 位置: xui.h:10314  已注释: 否
- 签名: `XUI_API void* xuiTimeLineViewGetSpanUserData(xui_widget pWidget, int iSpanId);`
- 实现: src/xui_timeline_view.c:2809（体 8 行）
- 返回码: NULL

## xuiTimeLineViewClearSelection
- 位置: xui.h:10315  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewClearSelection(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:2818（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTimeLineViewSelectFrame
- 位置: xui.h:10316  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSelectFrame(xui_widget pWidget, int iLayer, int iFrame, int bSelected);`
- 实现: src/xui_timeline_view.c:2826（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_collections_timeline_test.c:56

## xuiTimeLineViewSelectRange
- 位置: xui.h:10317  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSelectRange(xui_widget pWidget, int iLayer0, int iFrame0, int iLayer1, int iFrame1, int bSelected);`
- 实现: src/xui_timeline_view.c:2834（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_timelineview/main.c:401; test_xui/xui_timeline_view_test.c:392

## xuiTimeLineViewIsFrameSelected
- 位置: xui.h:10318  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewIsFrameSelected(xui_widget pWidget, int iLayer, int iFrame);`
- 实现: src/xui_timeline_view.c:2842（体 5 行）
- 用法: test_xui/xui_timeline_view_test.c:396; test_xui/xui_timeline_view_test.c:442; test_xui/xui_timeline_view_test.c:443

## xuiTimeLineViewGetSelectionCount
- 位置: xui.h:10319  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetSelectionCount(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:2848（体 5 行）
- 用法: examples/xui_timelineview/main.c:145; test_xui/xui_timeline_view_test.c:393; test_xui/xui_timeline_view_test.c:441

## xuiTimeLineViewGetSelection
- 位置: xui.h:10320  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetSelection(xui_widget pWidget, int iIndex, xui_timeline_selection_t* pSelection);`
- 实现: src/xui_timeline_view.c:2854（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_timeline_view_test.c:394

## xuiTimeLineViewSetCurrentFrameCallbacks
- 位置: xui.h:10321  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetCurrentFrameCallbacks(xui_widget pWidget, xui_timeline_current_frame_changing_proc onChanging, xui_timeline_current_frame_changed_proc onChanged, void* pUser);`
- 实现: src/xui_timeline_view.c:2862（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_timelineview/main.c:375; test_xui/xui_timeline_view_test.c:336

## xuiTimeLineViewSetLayerCallbacks
- 位置: xui.h:10322  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetLayerCallbacks(xui_widget pWidget, xui_timeline_layer_changing_proc onChanging, xui_timeline_layer_changed_proc onChanged, void* pUser);`
- 实现: src/xui_timeline_view.c:2872（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_timelineview/main.c:376; test_xui/xui_timeline_view_test.c:338

## xuiTimeLineViewSetFrameCallbacks
- 位置: xui.h:10323  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetFrameCallbacks(xui_widget pWidget, xui_timeline_frame_changing_proc onChanging, xui_timeline_frame_changed_proc onChanged, void* pUser);`
- 实现: src/xui_timeline_view.c:2882（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_timelineview/main.c:377; test_xui/xui_timeline_view_test.c:340

## xuiTimeLineViewSetSpanCallbacks
- 位置: xui.h:10324  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetSpanCallbacks(xui_widget pWidget, xui_timeline_span_changing_proc onChanging, xui_timeline_span_changed_proc onChanged, void* pUser);`
- 实现: src/xui_timeline_view.c:2892（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_timelineview/main.c:378; test_xui/xui_timeline_view_test.c:342

## xuiTimeLineViewSetLayerSelected
- 位置: xui.h:10325  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetLayerSelected(xui_widget pWidget, xui_timeline_layer_selected_proc onSelected, void* pUser);`
- 实现: src/xui_timeline_view.c:2902（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_timelineview/main.c:379; test_xui/xui_timeline_view_test.c:344

## xuiTimeLineViewSetContextMenu
- 位置: xui.h:10326  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetContextMenu(xui_widget pWidget, xui_timeline_context_opening_proc onOpening, xui_timeline_context_command_proc onCommand, void* pUser);`
- 实现: src/xui_timeline_view.c:2911（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_timelineview/main.c:380; test_xui/xui_timeline_view_test.c:346

## xuiTimeLineViewSetMenuTitle
- 位置: xui.h:10327  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetMenuTitle(xui_widget pWidget, int iCommand, const char* sTitle);`
- 实现: src/xui_timeline_view.c:2921（体 27 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK

## xuiTimeLineViewGetMenuTitle
- 位置: xui.h:10328  已注释: 否
- 签名: `XUI_API const char* xuiTimeLineViewGetMenuTitle(xui_widget pWidget, int iCommand);`
- 实现: src/xui_timeline_view.c:2949（体 5 行）

## xuiTimeLineViewRunContextCommand
- 位置: xui.h:10329  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewRunContextCommand(xui_widget pWidget, int iCommand);`
- 实现: src/xui_timeline_view.c:2955（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_timelineview/main.c:458; test_xui/xui_timeline_view_test.c:421; test_xui/xui_timeline_view_test.c:432

## xuiTimeLineViewSetFrameClick
- 位置: xui.h:10330  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetFrameClick(xui_widget pWidget, xui_timeline_frame_click_proc onClick, xui_timeline_frame_click_proc onDoubleClick, void* pUser);`
- 实现: src/xui_timeline_view.c:2962（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_timelineview/main.c:381; test_xui/xui_timeline_view_test.c:348

## xuiTimeLineViewSetSelectionChange
- 位置: xui.h:10331  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetSelectionChange(xui_widget pWidget, xui_timeline_selection_proc onSelection, void* pUser);`
- 实现: src/xui_timeline_view.c:2972（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_timelineview/main.c:382; test_xui/xui_timeline_view_test.c:350

## xuiTimeLineViewSetRenderers
- 位置: xui.h:10332  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetRenderers(xui_widget pWidget, xui_timeline_layer_renderer_proc onLayer, xui_timeline_ruler_renderer_proc onRuler, xui_timeline_frame_renderer_proc onFrame, xui_timeline_span_renderer_proc onSpan, void* pUser);`
- 实现: src/xui_timeline_view.c:2981（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_timelineview/main.c:383; test_xui/xui_timeline_view_test.c:352

## xuiTimeLineViewSetColors
- 位置: xui.h:10333  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetColors(xui_widget pWidget, const xui_timeline_view_colors_t* pColors);`
- 实现: src/xui_timeline_view.c:2993（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_timelineview/main.c:374; test_xui/xui_style_collections_timeline_test.c:77

## xuiTimeLineViewGetColors
- 位置: xui.h:10334  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetColors(xui_widget pWidget, xui_timeline_view_colors_t* pColors);`
- 实现: src/xui_timeline_view.c:3002（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_timelineview/main.c:370; test_xui/xui_style_collections_timeline_test.c:79

## xuiTimeLineViewEnsureFrameVisible
- 位置: xui.h:10335  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewEnsureFrameVisible(xui_widget pWidget, int iLayer, int iFrame);`
- 实现: src/xui_timeline_view.c:3010（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_timelineview/main.c:459; test_xui/xui_timeline_view_test.c:465

## xuiTimeLineViewHitTest
- 位置: xui.h:10336  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewHitTest(xui_widget pWidget, float fX, float fY, xui_timeline_hit_t* pHit);`
- 实现: src/xui_timeline_view.c:3022（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_timeline_view_test.c:407; test_xui/xui_timeline_view_test.c:409

## xuiTimeLineViewSetOffset
- 位置: xui.h:10337  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewSetOffset(xui_widget pWidget, float fOffsetX, float fOffsetY);`
- 实现: src/xui_timeline_view.c:3029（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_timeline_view_test.c:467

## xuiTimeLineViewGetOffset
- 位置: xui.h:10338  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetOffset(xui_widget pWidget, float* pOffsetX, float* pOffsetY);`
- 实现: src/xui_timeline_view.c:3036（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_timeline_view_test.c:469

## xuiTimeLineViewGetChangeCount
- 位置: xui.h:10339  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:3043（体 5 行）
- 用法: examples/xui_timelineview/main.c:462; test_xui/xui_timeline_view_test.c:477

## xuiTimeLineViewGetCurrentFrameChangeCount
- 位置: xui.h:10340  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetCurrentFrameChangeCount(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:3049（体 5 行）

## xuiTimeLineViewGetLayerChangeCount
- 位置: xui.h:10341  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetLayerChangeCount(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:3055（体 5 行）

## xuiTimeLineViewGetFrameChangeCount
- 位置: xui.h:10342  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetFrameChangeCount(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:3061（体 5 行）

## xuiTimeLineViewGetSpanChangeCount
- 位置: xui.h:10343  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetSpanChangeCount(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:3067（体 5 行）

## xuiTimeLineViewGetSelectionChangeCount
- 位置: xui.h:10344  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetSelectionChangeCount(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:3073（体 5 行）

## xuiTimeLineViewGetClickCount
- 位置: xui.h:10345  已注释: 否
- 签名: `XUI_API int xuiTimeLineViewGetClickCount(xui_widget pWidget);`
- 实现: src/xui_timeline_view.c:3079（体 5 行）

