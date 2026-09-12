# 草稿包：xui.h / chart（71 条 API）

> 生成 2026-09-10 02:57 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiChartGetType
- 位置: xui.h:6585  已注释: 否
- 签名: `XUI_API xui_widget_type xuiChartGetType(xui_context pContext);`
- 实现: src/xui_chart.c:2543（体 33 行）
- 返回码: NULL

## xuiChartCreate
- 位置: xui.h:6586  已注释: 否
- 签名: `XUI_API int xuiChartCreate(xui_context pContext, xui_widget* ppWidget, const xui_chart_desc_t* pDesc);`
- 实现: src/xui_chart.c:2577（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch180_main1.c:14; examples/tutorial_capture/ch205_main1.c:27; examples/xui_chart/main.c:141

## xuiChartSetTitle
- 位置: xui.h:6587  已注释: 否
- 签名: `XUI_API int xuiChartSetTitle(xui_widget pWidget, const char* sTitle);`
- 实现: src/xui_chart.c:2595（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY

## xuiChartGetTitle
- 位置: xui.h:6588  已注释: 否
- 签名: `XUI_API const char* xuiChartGetTitle(xui_widget pWidget);`
- 实现: src/xui_chart.c:2609（体 5 行）
- 用法: test_xui/xui_chart_test.c:167

## xuiChartSetFont
- 位置: xui.h:6589  已注释: 否
- 签名: `XUI_API int xuiChartSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_chart.c:2615（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiChartGetFont
- 位置: xui.h:6590  已注释: 否
- 签名: `XUI_API xui_font xuiChartGetFont(xui_widget pWidget);`
- 实现: src/xui_chart.c:2623（体 5 行）

## xuiChartSetColors
- 位置: xui.h:6591  已注释: 否
- 签名: `XUI_API int xuiChartSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iPlot, uint32_t iGrid, uint32_t iAxis, uint32_t iText, uint32_t iTooltip, uint32_t iTooltipText);`
- 实现: src/xui_chart.c:2629（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiChartGetColors
- 位置: xui.h:6592  已注释: 否
- 签名: `XUI_API int xuiChartGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pPlot, uint32_t* pGrid, uint32_t* pAxis, uint32_t* pText, uint32_t* pTooltip, uint32_t* pTooltipText);`
- 实现: src/xui_chart.c:2644（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiChartSetXAxis
- 位置: xui.h:6593  已注释: 否
- 签名: `XUI_API int xuiChartSetXAxis(xui_widget pWidget, int iAxisType);`
- 实现: src/xui_chart.c:2659（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_chart/main.c:249; test_xui/xui_chart_test.c:499

## xuiChartSetYAxis
- 位置: xui.h:6594  已注释: 否
- 签名: `XUI_API int xuiChartSetYAxis(xui_widget pWidget, int iAxisType);`
- 实现: src/xui_chart.c:2669（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiChartGetXAxis
- 位置: xui.h:6595  已注释: 否
- 签名: `XUI_API int xuiChartGetXAxis(xui_widget pWidget);`
- 实现: src/xui_chart.c:2679（体 5 行）

## xuiChartGetYAxis
- 位置: xui.h:6596  已注释: 否
- 签名: `XUI_API int xuiChartGetYAxis(xui_widget pWidget);`
- 实现: src/xui_chart.c:2685（体 5 行）

## xuiChartSetBarMode
- 位置: xui.h:6597  已注释: 否
- 签名: `XUI_API int xuiChartSetBarMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_chart.c:2691（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_chart/main.c:233; test_xui/xui_chart_breadcrumb_keyboard_test.c:473; test_xui/xui_chart_test.c:469

## xuiChartGetBarMode
- 位置: xui.h:6598  已注释: 否
- 签名: `XUI_API int xuiChartGetBarMode(xui_widget pWidget);`
- 实现: src/xui_chart.c:2701（体 5 行）
- 用法: test_xui/xui_chart_test.c:470; test_xui/xui_chart_test.c:476

## xuiChartSetBarDirection
- 位置: xui.h:6599  已注释: 否
- 签名: `XUI_API int xuiChartSetBarDirection(xui_widget pWidget, int iDirection);`
- 实现: src/xui_chart.c:2707（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_chart/main.c:234; test_xui/xui_chart_breadcrumb_keyboard_test.c:472; test_xui/xui_chart_test.c:477

## xuiChartGetBarDirection
- 位置: xui.h:6600  已注释: 否
- 签名: `XUI_API int xuiChartGetBarDirection(xui_widget pWidget);`
- 实现: src/xui_chart.c:2717（体 5 行）
- 用法: test_xui/xui_chart_test.c:478; test_xui/xui_chart_test.c:490

## xuiChartSetLegendVisible
- 位置: xui.h:6601  已注释: 否
- 签名: `XUI_API int xuiChartSetLegendVisible(xui_widget pWidget, int bVisible);`
- 实现: src/xui_chart.c:2723（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:331; test_xui/xui_chart_breadcrumb_keyboard_test.c:367; test_xui/xui_chart_breadcrumb_keyboard_test.c:416

## xuiChartGetLegendVisible
- 位置: xui.h:6602  已注释: 否
- 签名: `XUI_API int xuiChartGetLegendVisible(xui_widget pWidget);`
- 实现: src/xui_chart.c:2732（体 5 行）

## xuiChartSetTooltipVisible
- 位置: xui.h:6603  已注释: 否
- 签名: `XUI_API int xuiChartSetTooltipVisible(xui_widget pWidget, int bVisible);`
- 实现: src/xui_chart.c:2738（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:332; test_xui/xui_chart_breadcrumb_keyboard_test.c:334; test_xui/xui_chart_breadcrumb_keyboard_test.c:526

## xuiChartGetTooltipVisible
- 位置: xui.h:6604  已注释: 否
- 签名: `XUI_API int xuiChartGetTooltipVisible(xui_widget pWidget);`
- 实现: src/xui_chart.c:2747（体 5 行）

## xuiChartSetTooltipCallback
- 位置: xui.h:6605  已注释: 否
- 签名: `XUI_API int xuiChartSetTooltipCallback(xui_widget pWidget, xui_chart_tooltip_proc onTooltip, void* pUser);`
- 实现: src/xui_chart.c:2753（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_test.c:202

## xuiChartSetContextMenu
- 位置: xui.h:6606  已注释: 否
- 签名: `XUI_API int xuiChartSetContextMenu(xui_widget pWidget, xui_chart_context_proc onContext, void* pUser);`
- 实现: src/xui_chart.c:2762（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:335; test_xui/xui_chart_breadcrumb_keyboard_test.c:527

## xuiChartSetPadding
- 位置: xui.h:6607  已注释: 否
- 签名: `XUI_API int xuiChartSetPadding(xui_widget pWidget, xui_thickness_t tPadding);`
- 实现: src/xui_chart.c:2772（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_test.c:317; test_xui/xui_chart_test.c:324

## xuiChartGetPadding
- 位置: xui.h:6608  已注释: 否
- 签名: `XUI_API xui_thickness_t xuiChartGetPadding(xui_widget pWidget);`
- 实现: src/xui_chart.c:2785（体 8 行）
- 用法: test_xui/xui_chart_test.c:319

## xuiChartGetDirtyFlags
- 位置: xui.h:6609  已注释: 否
- 签名: `XUI_API uint32_t xuiChartGetDirtyFlags(xui_widget pWidget);`
- 实现: src/xui_chart.c:2794（体 5 行）
- 用法: test_xui/xui_chart_test.c:204; test_xui/xui_chart_test.c:226; test_xui/xui_chart_test.c:321

## xuiChartSetViewRange
- 位置: xui.h:6610  已注释: 否
- 签名: `XUI_API int xuiChartSetViewRange(xui_widget pWidget, double fMinX, double fMaxX, double fMinY, double fMaxY);`
- 实现: src/xui_chart.c:2800（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:579; test_xui/xui_chart_test.c:347

## xuiChartGetViewRange
- 位置: xui.h:6611  已注释: 否
- 签名: `XUI_API int xuiChartGetViewRange(xui_widget pWidget, double* pMinX, double* pMaxX, double* pMinY, double* pMaxY);`
- 实现: src/xui_chart.c:2811（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:433; test_xui/xui_chart_breadcrumb_keyboard_test.c:439; test_xui/xui_chart_breadcrumb_keyboard_test.c:443

## xuiChartResetViewRange
- 位置: xui.h:6612  已注释: 否
- 签名: `XUI_API int xuiChartResetViewRange(xui_widget pWidget);`
- 实现: src/xui_chart.c:2823（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:465; test_xui/xui_chart_breadcrumb_keyboard_test.c:561; test_xui/xui_chart_test.c:351

## xuiChartSetBrushRange
- 位置: xui.h:6613  已注释: 否
- 签名: `XUI_API int xuiChartSetBrushRange(xui_widget pWidget, double fMinX, double fMaxX, double fMinY, double fMaxY);`
- 实现: src/xui_chart.c:2833（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_test.c:393; test_xui/xui_style_chrome_test.c:358

## xuiChartGetBrushRange
- 位置: xui.h:6614  已注释: 否
- 签名: `XUI_API int xuiChartGetBrushRange(xui_widget pWidget, double* pMinX, double* pMaxX, double* pMinY, double* pMaxY);`
- 实现: src/xui_chart.c:2843（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:452; test_xui/xui_chart_breadcrumb_keyboard_test.c:455; test_xui/xui_chart_breadcrumb_keyboard_test.c:571

## xuiChartClearBrushRange
- 位置: xui.h:6615  已注释: 否
- 签名: `XUI_API int xuiChartClearBrushRange(xui_widget pWidget);`
- 实现: src/xui_chart.c:2854（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_test.c:397

## xuiChartSetLodThreshold
- 位置: xui.h:6616  已注释: 否
- 签名: `XUI_API int xuiChartSetLodThreshold(xui_widget pWidget, int iThreshold);`
- 实现: src/xui_chart.c:2863（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_test.c:419; test_xui/xui_chart_test.c:423

## xuiChartGetLodThreshold
- 位置: xui.h:6617  已注释: 否
- 签名: `XUI_API int xuiChartGetLodThreshold(xui_widget pWidget);`
- 实现: src/xui_chart.c:2871（体 5 行）
- 用法: test_xui/xui_chart_test.c:420; test_xui/xui_chart_test.c:424

## xuiChartGetLastLodStride
- 位置: xui.h:6618  已注释: 否
- 签名: `XUI_API int xuiChartGetLastLodStride(xui_widget pWidget);`
- 实现: src/xui_chart.c:2877（体 5 行）
- 用法: test_xui/xui_chart_test.c:422

## xuiChartSetAnimation
- 位置: xui.h:6619  已注释: 否
- 签名: `XUI_API int xuiChartSetAnimation(xui_widget pWidget, int bEnabled, float fDuration);`
- 实现: src/xui_chart.c:2883（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_test.c:425; test_xui/xui_chart_test.c:439

## xuiChartGetAnimation
- 位置: xui.h:6620  已注释: 否
- 签名: `XUI_API int xuiChartGetAnimation(xui_widget pWidget, int* pEnabled, float* pDuration, float* pProgress);`
- 实现: src/xui_chart.c:2898（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_test.c:429; test_xui/xui_chart_test.c:433; test_xui/xui_chart_test.c:437

## xuiChartStepAnimation
- 位置: xui.h:6621  已注释: 否
- 签名: `XUI_API int xuiChartStepAnimation(xui_widget pWidget, float fDeltaSeconds);`
- 实现: src/xui_chart.c:2908（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_chart_test.c:431; test_xui/xui_chart_test.c:435

## xuiChartAddSeries
- 位置: xui.h:6622  已注释: 否
- 签名: `XUI_API int xuiChartAddSeries(xui_widget pWidget, int iType, const char* sName, int* pIndex);`
- 实现: src/xui_chart.c:2926（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/xui_chart/main.c:212; examples/xui_chart/main.c:217; examples/xui_chart/main.c:226

## xuiChartClearSeries
- 位置: xui.h:6623  已注释: 否
- 签名: `XUI_API int xuiChartClearSeries(xui_widget pWidget);`
- 实现: src/xui_chart.c:2951（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:414; test_xui/xui_chart_breadcrumb_keyboard_test.c:419; test_xui/xui_chart_breadcrumb_keyboard_test.c:471

## xuiChartGetSeriesCount
- 位置: xui.h:6624  已注释: 否
- 签名: `XUI_API int xuiChartGetSeriesCount(xui_widget pWidget);`
- 实现: src/xui_chart.c:2969（体 5 行）
- 用法: test_xui/xui_chart_test.c:168; test_xui/xui_chart_test.c:173; test_xui/xui_chart_test.c:529

## xuiChartGetSeriesType
- 位置: xui.h:6625  已注释: 否
- 签名: `XUI_API int xuiChartGetSeriesType(xui_widget pWidget, int iSeries);`
- 实现: src/xui_chart.c:2975（体 6 行）

## xuiChartSetSeriesData
- 位置: xui.h:6626  已注释: 否
- 签名: `XUI_API int xuiChartSetSeriesData(xui_widget pWidget, int iSeries, const xui_chart_point_t* pPoints, int iCount);`
- 实现: src/xui_chart.c:2982（体 27 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/xui_chart/main.c:216; examples/xui_chart/main.c:222; examples/xui_chart/main.c:228

## xuiChartSetSeriesColor
- 位置: xui.h:6627  已注释: 否
- 签名: `XUI_API int xuiChartSetSeriesColor(xui_widget pWidget, int iSeries, uint32_t iColor);`
- 实现: src/xui_chart.c:3010（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_chart/main.c:219; examples/xui_chart/main.c:231; test_xui/xui_style_chrome_test.c:354

## xuiChartGetSeriesColor
- 位置: xui.h:6628  已注释: 否
- 签名: `XUI_API uint32_t xuiChartGetSeriesColor(xui_widget pWidget, int iSeries);`
- 实现: src/xui_chart.c:3018（体 6 行）

## xuiChartSetSeriesAreaFill
- 位置: xui.h:6629  已注释: 否
- 签名: `XUI_API int xuiChartSetSeriesAreaFill(xui_widget pWidget, int iSeries, int bEnabled, uint32_t iColor);`
- 实现: src/xui_chart.c:3025（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: examples/xui_chart/main.c:214; test_xui/xui_chart_test.c:196

## xuiChartGetSeriesAreaFill
- 位置: xui.h:6630  已注释: 否
- 签名: `XUI_API int xuiChartGetSeriesAreaFill(xui_widget pWidget, int iSeries, uint32_t* pColor);`
- 实现: src/xui_chart.c:3038（体 13 行）
- 用法: test_xui/xui_chart_test.c:197

## xuiChartSetSeriesSmooth
- 位置: xui.h:6631  已注释: 否
- 签名: `XUI_API int xuiChartSetSeriesSmooth(xui_widget pWidget, int iSeries, int bSmooth);`
- 实现: src/xui_chart.c:3052（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: examples/xui_chart/main.c:215; test_xui/xui_chart_test.c:198

## xuiChartGetSeriesSmooth
- 位置: xui.h:6632  已注释: 否
- 签名: `XUI_API int xuiChartGetSeriesSmooth(xui_widget pWidget, int iSeries);`
- 实现: src/xui_chart.c:3064（体 7 行）
- 用法: test_xui/xui_chart_test.c:199

## xuiChartSetSeriesDash
- 位置: xui.h:6633  已注释: 否
- 签名: `XUI_API int xuiChartSetSeriesDash(xui_widget pWidget, int iSeries, const float* pDashPattern, int iDashCount);`
- 实现: src/xui_chart.c:3072（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: examples/xui_chart/main.c:221; test_xui/xui_chart_test.c:200

## xuiChartClearSeriesDash
- 位置: xui.h:6634  已注释: 否
- 签名: `XUI_API int xuiChartClearSeriesDash(xui_widget pWidget, int iSeries);`
- 实现: src/xui_chart.c:3090（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED

## xuiChartGetSeriesDash
- 位置: xui.h:6635  已注释: 否
- 签名: `XUI_API int xuiChartGetSeriesDash(xui_widget pWidget, int iSeries, float* pDashPattern, int iCapacity);`
- 实现: src/xui_chart.c:3103（体 16 行）
- 用法: test_xui/xui_chart_test.c:201

## xuiChartSetSeriesVisible
- 位置: xui.h:6636  已注释: 否
- 签名: `XUI_API int xuiChartSetSeriesVisible(xui_widget pWidget, int iSeries, int bVisible);`
- 实现: src/xui_chart.c:3120（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:338; test_xui/xui_chart_breadcrumb_keyboard_test.c:487; test_xui/xui_chart_breadcrumb_keyboard_test.c:503

## xuiChartGetSeriesVisible
- 位置: xui.h:6637  已注释: 否
- 签名: `XUI_API int xuiChartGetSeriesVisible(xui_widget pWidget, int iSeries);`
- 实现: src/xui_chart.c:3130（体 6 行）
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:374; test_xui/xui_chart_breadcrumb_keyboard_test.c:376; test_xui/xui_chart_breadcrumb_keyboard_test.c:380

## xuiChartSetSeriesSymbol
- 位置: xui.h:6638  已注释: 否
- 签名: `XUI_API int xuiChartSetSeriesSymbol(xui_widget pWidget, int iSeries, int iSymbol);`
- 实现: src/xui_chart.c:3137（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_chart/main.c:220; test_xui/xui_chart_test.c:192

## xuiChartGetSeriesSymbol
- 位置: xui.h:6639  已注释: 否
- 签名: `XUI_API int xuiChartGetSeriesSymbol(xui_widget pWidget, int iSeries);`
- 实现: src/xui_chart.c:3147（体 6 行）
- 用法: test_xui/xui_chart_test.c:193

## xuiChartSetSeriesSymbolSize
- 位置: xui.h:6640  已注释: 否
- 签名: `XUI_API int xuiChartSetSeriesSymbolSize(xui_widget pWidget, int iSeries, float fSize);`
- 实现: src/xui_chart.c:3154（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_chart/main.c:252; test_xui/xui_chart_test.c:194

## xuiChartGetSeriesSymbolSize
- 位置: xui.h:6641  已注释: 否
- 签名: `XUI_API float xuiChartGetSeriesSymbolSize(xui_widget pWidget, int iSeries);`
- 实现: src/xui_chart.c:3162（体 6 行）
- 用法: test_xui/xui_chart_test.c:195

## xuiChartSetSeriesValueRadius
- 位置: xui.h:6642  已注释: 否
- 签名: `XUI_API int xuiChartSetSeriesValueRadius(xui_widget pWidget, int iSeries, float fMinSize, float fMaxSize);`
- 实现: src/xui_chart.c:3169（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: examples/xui_chart/main.c:253; test_xui/xui_chart_test.c:505

## xuiChartClearSeriesValueRadius
- 位置: xui.h:6643  已注释: 否
- 签名: `XUI_API int xuiChartClearSeriesValueRadius(xui_widget pWidget, int iSeries);`
- 实现: src/xui_chart.c:3182（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_chart_test.c:517

## xuiChartGetSeriesValueRadius
- 位置: xui.h:6644  已注释: 否
- 签名: `XUI_API int xuiChartGetSeriesValueRadius(xui_widget pWidget, int iSeries, float* pMinSize, float* pMaxSize);`
- 实现: src/xui_chart.c:3193（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_chart_test.c:507; test_xui/xui_chart_test.c:518

## xuiChartSetSeriesValueColor
- 位置: xui.h:6645  已注释: 否
- 签名: `XUI_API int xuiChartSetSeriesValueColor(xui_widget pWidget, int iSeries, uint32_t iMinColor, uint32_t iMaxColor);`
- 实现: src/xui_chart.c:3205（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: examples/xui_chart/main.c:254; test_xui/xui_chart_test.c:509

## xuiChartClearSeriesValueColor
- 位置: xui.h:6646  已注释: 否
- 签名: `XUI_API int xuiChartClearSeriesValueColor(xui_widget pWidget, int iSeries);`
- 实现: src/xui_chart.c:3218（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_chart_test.c:519

## xuiChartGetSeriesValueColor
- 位置: xui.h:6647  已注释: 否
- 签名: `XUI_API int xuiChartGetSeriesValueColor(xui_widget pWidget, int iSeries, uint32_t* pMinColor, uint32_t* pMaxColor);`
- 实现: src/xui_chart.c:3229（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_chart_test.c:511; test_xui/xui_chart_test.c:520

## xuiChartSetPieMode
- 位置: xui.h:6648  已注释: 否
- 签名: `XUI_API int xuiChartSetPieMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_chart.c:3241（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_chart/main.c:245; test_xui/xui_chart_breadcrumb_keyboard_test.c:494; test_xui/xui_chart_test.c:545

## xuiChartGetPieMode
- 位置: xui.h:6649  已注释: 否
- 签名: `XUI_API int xuiChartGetPieMode(xui_widget pWidget);`
- 实现: src/xui_chart.c:3251（体 5 行）
- 用法: test_xui/xui_chart_test.c:546; test_xui/xui_chart_test.c:552

## xuiChartSetPieInnerRadius
- 位置: xui.h:6650  已注释: 否
- 签名: `XUI_API int xuiChartSetPieInnerRadius(xui_widget pWidget, float fRate);`
- 实现: src/xui_chart.c:3257（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_chart/main.c:244; test_xui/xui_chart_breadcrumb_keyboard_test.c:495; test_xui/xui_chart_test.c:538

## xuiChartGetPieInnerRadius
- 位置: xui.h:6651  已注释: 否
- 签名: `XUI_API float xuiChartGetPieInnerRadius(xui_widget pWidget);`
- 实现: src/xui_chart.c:3265（体 5 行）
- 用法: test_xui/xui_chart_test.c:539

## xuiChartGetPlotRect
- 位置: xui.h:6652  已注释: 否
- 签名: `XUI_API xui_rect_t xuiChartGetPlotRect(xui_widget pWidget);`
- 实现: src/xui_chart.c:3271（体 10 行）
- 用法: examples/xui_chart/main.c:330; test_xui/xui_chart_breadcrumb_keyboard_test.c:458; test_xui/xui_chart_breadcrumb_keyboard_test.c:543

## xuiChartDataToPixel
- 位置: xui.h:6653  已注释: 否
- 签名: `XUI_API int xuiChartDataToPixel(xui_widget pWidget, double fX, double fY, xui_vec2_t* pPoint);`
- 实现: src/xui_chart.c:3282（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_chart/main.c:67; test_xui/xui_chart_breadcrumb_keyboard_test.c:358; test_xui/xui_chart_breadcrumb_keyboard_test.c:530

## xuiChartPixelToData
- 位置: xui.h:6654  已注释: 否
- 签名: `XUI_API int xuiChartPixelToData(xui_widget pWidget, float fX, float fY, double* pDataX, double* pDataY);`
- 实现: src/xui_chart.c:3296（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_chart_test.c:222

## xuiChartHitTest
- 位置: xui.h:6655  已注释: 否
- 签名: `XUI_API int xuiChartHitTest(xui_widget pWidget, float fX, float fY, xui_chart_hit_t* pHit);`
- 实现: src/xui_chart.c:3310（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:427; test_xui/xui_chart_breadcrumb_keyboard_test.c:482; test_xui/xui_chart_breadcrumb_keyboard_test.c:499

