# 草稿包：xui.h / datepicker（73 条 API）

> 生成 2026-09-10 02:58 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiDatePickerGetType
- 位置: xui.h:9837  已注释: 否
- 签名: `XUI_API xui_widget_type xuiDatePickerGetType(xui_context pContext);`
- 实现: src/xui_date_picker.c:2713（体 30 行）
- 返回码: NULL

## xuiDatePickerCreate
- 位置: xui.h:9838  已注释: 否
- 签名: `XUI_API int xuiDatePickerCreate(xui_context pContext, xui_widget* ppWidget, const xui_date_picker_desc_t* pDesc);`
- 实现: src/xui_date_picker.c:2744（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch167_main1.c:17; examples/xui_datepicker/main.c:259; test_xui/xui_date_picker_pixel_test.c:152

## xuiDatePickerSetChanging
- 位置: xui.h:9839  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetChanging(xui_widget pWidget, xui_date_picker_proc onChanging, void* pUser);`
- 实现: src/xui_date_picker.c:2755（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_datepicker/main.c:261; test_xui/xui_date_picker_test.c:303

## xuiDatePickerSetChange
- 位置: xui.h:9840  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetChange(xui_widget pWidget, xui_date_picker_proc onChange, void* pUser);`
- 实现: src/xui_date_picker.c:2764（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_datepicker/main.c:262; test_xui/xui_date_picker_test.c:304

## xuiDatePickerSetCommit
- 位置: xui.h:9841  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetCommit(xui_widget pWidget, xui_date_picker_proc onCommit, void* pUser);`
- 实现: src/xui_date_picker.c:2773（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_datepicker/main.c:263; test_xui/xui_date_picker_test.c:305

## xuiDatePickerSetCancel
- 位置: xui.h:9842  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetCancel(xui_widget pWidget, xui_date_picker_proc onCancel, void* pUser);`
- 实现: src/xui_date_picker.c:2782（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_datepicker/main.c:264; test_xui/xui_date_picker_test.c:306

## xuiDatePickerSetClear
- 位置: xui.h:9843  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetClear(xui_widget pWidget, xui_date_picker_proc onClear, void* pUser);`
- 实现: src/xui_date_picker.c:2791（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_datepicker/main.c:265; test_xui/xui_date_picker_test.c:307

## xuiDatePickerSetMode
- 位置: xui.h:9844  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_date_picker.c:2800（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_date_picker_pixel_test.c:98; test_xui/xui_date_picker_pixel_test.c:161; test_xui/xui_date_picker_test.c:336

## xuiDatePickerGetMode
- 位置: xui.h:9845  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetMode(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:2815（体 5 行）
- 用法: examples/xui_datepicker/main.c:363; examples/xui_datepicker/main.c:364; examples/xui_datepicker/main.c:365

## xuiDatePickerSetNullable
- 位置: xui.h:9846  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetNullable(xui_widget pWidget, int bNullable);`
- 实现: src/xui_date_picker.c:2821（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_datepicker/main.c:321; test_xui/xui_date_picker_test.c:448; test_xui/xui_date_picker_test.c:452

## xuiDatePickerGetNullable
- 位置: xui.h:9847  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetNullable(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:2831（体 5 行）

## xuiDatePickerSetValue
- 位置: xui.h:9848  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetValue(xui_widget pWidget, xtime tValue);`
- 实现: src/xui_date_picker.c:2837（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_datepicker/main.c:310; examples/xui_datepicker/main.c:312; examples/xui_datepicker/main.c:324

## xuiDatePickerGetValue
- 位置: xui.h:9849  已注释: 否
- 签名: `XUI_API xtime xuiDatePickerGetValue(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:2850（体 7 行）
- 用法: examples/xui_datepicker/main.c:371; examples/xui_datepicker/main.c:372; examples/xui_datepicker/main.c:373

## xuiDatePickerHasValue
- 位置: xui.h:9850  已注释: 否
- 签名: `XUI_API int xuiDatePickerHasValue(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:2858（体 5 行）
- 用法: examples/xui_datepicker/main.c:379; examples/xui_datepicker/main.c:395; test_xui/xui_date_picker_test.c:451

## xuiDatePickerClearValue
- 位置: xui.h:9851  已注释: 否
- 签名: `XUI_API int xuiDatePickerClearValue(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:2864（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_datepicker/main.c:322; test_xui/xui_date_picker_test.c:450

## xuiDatePickerSetRangeValue
- 位置: xui.h:9852  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetRangeValue(xui_widget pWidget, xtime tStart, xtime tEnd);`
- 实现: src/xui_date_picker.c:2877（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_datepicker/main.c:313; examples/xui_datepicker/main.c:316; examples/xui_datepicker/main.c:320

## xuiDatePickerGetRangeValue
- 位置: xui.h:9853  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetRangeValue(xui_widget pWidget, xtime* pStart, xtime* pEnd);`
- 实现: src/xui_date_picker.c:2891（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_datepicker/main.c:374; examples/xui_datepicker/main.c:376; test_xui/xui_date_picker_test.c:411

## xuiDatePickerHasRangeValue
- 位置: xui.h:9854  已注释: 否
- 签名: `XUI_API int xuiDatePickerHasRangeValue(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:2901（体 5 行）

## xuiDatePickerSetLimits
- 位置: xui.h:9855  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetLimits(xui_widget pWidget, xtime tMin, xtime tMax);`
- 实现: src/xui_date_picker.c:2907（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_datepicker/main.c:323; test_xui/xui_date_picker_test.c:443

## xuiDatePickerSetLimitRange
- 位置: xui.h:9856  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetLimitRange(xui_widget pWidget, int bHasMin, xtime tMin, int bHasMax, xtime tMax);`
- 实现: src/xui_date_picker.c:2925（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiDatePickerGetLimitRange
- 位置: xui.h:9857  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetLimitRange(xui_widget pWidget, int* pHasMin, xtime* pMin, int* pHasMax, xtime* pMax);`
- 实现: src/xui_date_picker.c:2943（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiDatePickerClearLimits
- 位置: xui.h:9858  已注释: 否
- 签名: `XUI_API int xuiDatePickerClearLimits(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:2954（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_date_picker_test.c:454

## xuiDatePickerSetFormat
- 位置: xui.h:9859  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetFormat(xui_widget pWidget, const char* sFormat);`
- 实现: src/xui_date_picker.c:2963（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_datepicker/main.c:318; test_xui/xui_date_picker_test.c:428; test_xui/xui_date_picker_test.c:441

## xuiDatePickerGetFormat
- 位置: xui.h:9860  已注释: 否
- 签名: `XUI_API const char* xuiDatePickerGetFormat(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:2978（体 5 行）

## xuiDatePickerSetRangeSeparator
- 位置: xui.h:9861  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetRangeSeparator(xui_widget pWidget, const char* sSeparator);`
- 实现: src/xui_date_picker.c:2984（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_datepicker/main.c:319; test_xui/xui_date_picker_test.c:430

## xuiDatePickerGetRangeSeparator
- 位置: xui.h:9862  已注释: 否
- 签名: `XUI_API const char* xuiDatePickerGetRangeSeparator(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:2993（体 5 行）

## xuiDatePickerSetShowSecond
- 位置: xui.h:9863  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetShowSecond(xui_widget pWidget, int bShowSecond);`
- 实现: src/xui_date_picker.c:2999（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_datepicker/main.c:309; examples/xui_datepicker/main.c:311; examples/xui_datepicker/main.c:315

## xuiDatePickerGetShowSecond
- 位置: xui.h:9864  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetShowSecond(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3010（体 5 行）

## xuiDatePickerSetFirstDayOfWeek
- 位置: xui.h:9865  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetFirstDayOfWeek(xui_widget pWidget, int iFirstDayOfWeek);`
- 实现: src/xui_date_picker.c:3016（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_datepicker/main.c:314

## xuiDatePickerGetFirstDayOfWeek
- 位置: xui.h:9866  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetFirstDayOfWeek(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3024（体 5 行）

## xuiDatePickerSetDefaultRangeSpan
- 位置: xui.h:9867  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetDefaultRangeSpan(xui_widget pWidget, xtime tSpan);`
- 实现: src/xui_date_picker.c:3030（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiDatePickerGetDefaultRangeSpan
- 位置: xui.h:9868  已注释: 否
- 签名: `XUI_API xtime xuiDatePickerGetDefaultRangeSpan(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3038（体 5 行）

## xuiDatePickerOpen
- 位置: xui.h:9869  已注释: 否
- 签名: `XUI_API int xuiDatePickerOpen(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3044（体 5 行）
- 用法: examples/xui_datepicker/main.c:382; examples/xui_datepicker/main.c:392; test_xui/xui_date_picker_pixel_test.c:102

## xuiDatePickerClose
- 位置: xui.h:9870  已注释: 否
- 签名: `XUI_API int xuiDatePickerClose(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3050（体 5 行）
- 用法: test_xui/xui_date_picker_pixel_test.c:136

## xuiDatePickerToggle
- 位置: xui.h:9871  已注释: 否
- 签名: `XUI_API int xuiDatePickerToggle(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3056（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiDatePickerIsOpen
- 位置: xui.h:9872  已注释: 否
- 签名: `XUI_API int xuiDatePickerIsOpen(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3063（体 5 行）
- 用法: examples/xui_datepicker/main.c:389; test_xui/xui_date_picker_test.c:354; test_xui/xui_date_picker_test.c:371

## xuiDatePickerSetPopupSize
- 位置: xui.h:9873  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetPopupSize(xui_widget pWidget, float fWidth, float fHeight);`
- 实现: src/xui_date_picker.c:3069（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_date_picker_pixel_test.c:66; test_xui/xui_date_picker_pixel_test.c:99; test_xui/xui_date_picker_pixel_test.c:137

## xuiDatePickerGetPopupSize
- 位置: xui.h:9874  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetPopupSize(xui_widget pWidget, float* pWidth, float* pHeight);`
- 实现: src/xui_date_picker.c:3080（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_date_picker_test.c:220

## xuiDatePickerSetPopupPlacement
- 位置: xui.h:9875  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetPopupPlacement(xui_widget pWidget, int iPlacement);`
- 实现: src/xui_date_picker.c:3089（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiDatePickerGetPopupPlacement
- 位置: xui.h:9876  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetPopupPlacement(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3098（体 5 行）

## xuiDatePickerSetMetrics
- 位置: xui.h:9877  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetMetrics(xui_widget pWidget, float fBorderWidth);`
- 实现: src/xui_date_picker.c:3104（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiDatePickerGetMetrics
- 位置: xui.h:9878  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetMetrics(xui_widget pWidget, float* pBorderWidth);`
- 实现: src/xui_date_picker.c:3112（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_date_picker_test.c:324

## xuiDatePickerSetColors
- 位置: xui.h:9879  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetColors(xui_widget pWidget, uint32_t iText, uint32_t iDisabledText, uint32_t iBackground, uint32_t iHoverBackground, uint32_t iOpenBackground, uint32_t iDisabledBackground);`
- 实现: src/xui_date_picker.c:3120（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiDatePickerGetColors
- 位置: xui.h:9880  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetColors(xui_widget pWidget, uint32_t* pText, uint32_t* pDisabledText, uint32_t* pBackground, uint32_t* pHoverBackground, uint32_t* pOpenBackground, uint32_t* pDisabledBackground);`
- 实现: src/xui_date_picker.c:3133（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiDatePickerSetBorderColors
- 位置: xui.h:9881  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetBorderColors(xui_widget pWidget, uint32_t iBorder, uint32_t iHoverBorder, uint32_t iFocusBorder);`
- 实现: src/xui_date_picker.c:3146（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiDatePickerGetBorderColors
- 位置: xui.h:9882  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetBorderColors(xui_widget pWidget, uint32_t* pBorder, uint32_t* pHoverBorder, uint32_t* pFocusBorder);`
- 实现: src/xui_date_picker.c:3156（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiDatePickerSetArrowColors
- 位置: xui.h:9883  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetArrowColors(xui_widget pWidget, uint32_t iArrow, uint32_t iDisabledArrow);`
- 实现: src/xui_date_picker.c:3166（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiDatePickerGetArrowColors
- 位置: xui.h:9884  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetArrowColors(xui_widget pWidget, uint32_t* pArrow, uint32_t* pDisabledArrow);`
- 实现: src/xui_date_picker.c:3175（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiDatePickerSetPopupColors
- 位置: xui.h:9885  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetPopupColors(xui_widget pWidget, uint32_t iPanel, uint32_t iBorder, uint32_t iShadow, uint32_t iText, uint32_t iMutedText, uint32_t iAccent, uint32_t iField, uint32_t iFieldBorder, uint32_t iSelectedText, uint32_t iDisabledDay, uint32_t iSeparator);`
- 实现: src/xui_date_picker.c:3184（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_pickers_date_picker_test.c:118

## xuiDatePickerGetPopupColors
- 位置: xui.h:9886  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetPopupColors(xui_widget pWidget, uint32_t* pPanel, uint32_t* pBorder, uint32_t* pShadow, uint32_t* pText, uint32_t* pMutedText, uint32_t* pAccent, uint32_t* pField, uint32_t* pFieldBorder, uint32_t* pSelectedText, uint32_t* pDisabledDay, uint32_t* pSeparator);`
- 实现: src/xui_date_picker.c:3203（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiDatePickerSetFont
- 位置: xui.h:9887  已注释: 否
- 签名: `XUI_API int xuiDatePickerSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_date_picker.c:3221（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiDatePickerGetFont
- 位置: xui.h:9888  已注释: 否
- 签名: `XUI_API xui_font xuiDatePickerGetFont(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3229（体 5 行）

## xuiDatePickerGetText
- 位置: xui.h:9889  已注释: 否
- 签名: `XUI_API const char* xuiDatePickerGetText(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3235（体 5 行）
- 用法: test_xui/xui_date_picker_test.c:323; test_xui/xui_date_picker_test.c:342; test_xui/xui_date_picker_test.c:351

## xuiDatePickerGetPopupWidget
- 位置: xui.h:9890  已注释: 否
- 签名: `XUI_API xui_widget xuiDatePickerGetPopupWidget(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3241（体 5 行）
- 用法: examples/xui_datepicker/main.c:360; test_xui/xui_date_picker_test.c:463; test_xui/xui_grid_focus_test.c:254

## xuiDatePickerGetPanelWidget
- 位置: xui.h:9891  已注释: 否
- 签名: `XUI_API xui_widget xuiDatePickerGetPanelWidget(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3247（体 5 行）
- 用法: examples/xui_datepicker/main.c:335; test_xui/xui_date_picker_pixel_test.c:104; test_xui/xui_date_picker_test.c:357

## xuiDatePickerGetButtonRect
- 位置: xui.h:9892  已注释: 否
- 签名: `XUI_API xui_rect_t xuiDatePickerGetButtonRect(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3253（体 7 行）
- 用法: examples/xui_datepicker/main.c:361; test_xui/xui_date_picker_pixel_test.c:157; test_xui/xui_date_picker_test.c:333

## xuiDatePickerGetTextRect
- 位置: xui.h:9893  已注释: 否
- 签名: `XUI_API xui_rect_t xuiDatePickerGetTextRect(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3261（体 7 行）
- 用法: test_xui/xui_date_picker_pixel_test.c:158; test_xui/xui_date_picker_test.c:331

## xuiDatePickerGetCalendarPanelRect
- 位置: xui.h:9894  已注释: 否
- 签名: `XUI_API xui_rect_t xuiDatePickerGetCalendarPanelRect(xui_widget pWidget, int iPanel);`
- 实现: src/xui_date_picker.c:3269（体 7 行）
- 用法: test_xui/xui_date_picker_pixel_test.c:28; test_xui/xui_date_picker_pixel_test.c:81; test_xui/xui_date_picker_pixel_test.c:109

## xuiDatePickerGetTimePanelRect
- 位置: xui.h:9895  已注释: 否
- 签名: `XUI_API xui_rect_t xuiDatePickerGetTimePanelRect(xui_widget pWidget, int iPanel);`
- 实现: src/xui_date_picker.c:3277（体 7 行）
- 用法: test_xui/xui_date_picker_pixel_test.c:50; test_xui/xui_date_picker_pixel_test.c:82; test_xui/xui_date_picker_test.c:39

## xuiDatePickerGetPrevRect
- 位置: xui.h:9896  已注释: 否
- 签名: `XUI_API xui_rect_t xuiDatePickerGetPrevRect(xui_widget pWidget, int iPanel);`
- 实现: src/xui_date_picker.c:3285（体 7 行）
- 用法: test_xui/xui_date_picker_pixel_test.c:29

## xuiDatePickerGetNextRect
- 位置: xui.h:9897  已注释: 否
- 签名: `XUI_API xui_rect_t xuiDatePickerGetNextRect(xui_widget pWidget, int iPanel);`
- 实现: src/xui_date_picker.c:3293（体 7 行）
- 用法: test_xui/xui_date_picker_pixel_test.c:30

## xuiDatePickerGetDayRect
- 位置: xui.h:9898  已注释: 否
- 签名: `XUI_API xui_rect_t xuiDatePickerGetDayRect(xui_widget pWidget, int iPanel, int iIndex);`
- 实现: src/xui_date_picker.c:3301（体 7 行）
- 用法: examples/xui_datepicker/main.c:386; test_xui/xui_date_picker_pixel_test.c:33; test_xui/xui_date_picker_pixel_test.c:39

## xuiDatePickerGetDayValue
- 位置: xui.h:9899  已注释: 否
- 签名: `XUI_API xtime xuiDatePickerGetDayValue(xui_widget pWidget, int iPanel, int iIndex);`
- 实现: src/xui_date_picker.c:3309（体 7 行）
- 用法: examples/xui_datepicker/main.c:349; test_xui/xui_date_picker_test.c:236

## xuiDatePickerGetTimeRect
- 位置: xui.h:9900  已注释: 否
- 签名: `XUI_API xui_rect_t xuiDatePickerGetTimeRect(xui_widget pWidget, int iPanel, int iField);`
- 实现: src/xui_date_picker.c:3317（体 7 行）
- 用法: test_xui/xui_date_picker_pixel_test.c:55; test_xui/xui_date_picker_pixel_test.c:56; test_xui/xui_date_picker_test.c:42

## xuiDatePickerGetFooterRect
- 位置: xui.h:9901  已注释: 否
- 签名: `XUI_API xui_rect_t xuiDatePickerGetFooterRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_date_picker.c:3325（体 7 行）
- 用法: examples/xui_datepicker/main.c:387; examples/xui_datepicker/main.c:394; test_xui/xui_date_picker_pixel_test.c:84

## xuiDatePickerGetHoverPart
- 位置: xui.h:9902  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetHoverPart(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3333（体 5 行）

## xuiDatePickerGetActivePart
- 位置: xui.h:9903  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetActivePart(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3339（体 5 行）

## xuiDatePickerGetState
- 位置: xui.h:9904  已注释: 否
- 签名: `XUI_API uint32_t xuiDatePickerGetState(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3345（体 5 行）
- 用法: test_xui/xui_date_picker_test.c:466; test_xui/xui_date_picker_test.c:547

## xuiDatePickerGetChangingCount
- 位置: xui.h:9905  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetChangingCount(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3351（体 5 行）

## xuiDatePickerGetChangeCount
- 位置: xui.h:9906  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3357（体 5 行）

## xuiDatePickerGetCommitCount
- 位置: xui.h:9907  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetCommitCount(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3363（体 5 行）

## xuiDatePickerGetCancelCount
- 位置: xui.h:9908  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetCancelCount(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3369（体 5 行）
- 用法: test_xui/xui_grid_focus_test.c:326; test_xui/xui_grid_focus_test.c:330

## xuiDatePickerGetClearCount
- 位置: xui.h:9909  已注释: 否
- 签名: `XUI_API int xuiDatePickerGetClearCount(xui_widget pWidget);`
- 实现: src/xui_date_picker.c:3375（体 5 行）

