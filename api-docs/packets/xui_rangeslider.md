# 草稿包：xui.h / rangeslider（30 条 API）

> 生成 2026-09-10 03:04 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiRangeSliderGetType
- 位置: xui.h:8397  已注释: 否
- 签名: `XUI_API xui_widget_type xuiRangeSliderGetType(xui_context pContext);`
- 实现: src/xui_range_slider.c:1120（体 33 行）
- 返回码: NULL

## xuiRangeSliderCreate
- 位置: xui.h:8398  已注释: 否
- 签名: `XUI_API int xuiRangeSliderCreate(xui_context pContext, xui_widget* ppWidget, const xui_range_slider_desc_t* pDesc);`
- 实现: src/xui_range_slider.c:1154（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch170_main1.c:20; examples/xui_rangeslider/main.c:164; test_xui/xui_range_slider_test.c:165

## xuiRangeSliderSetChange
- 位置: xui.h:8399  已注释: 否
- 签名: `XUI_API int xuiRangeSliderSetChange(xui_widget pWidget, xui_range_slider_change_proc onChange, void* pUser);`
- 实现: src/xui_range_slider.c:1169（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_rangeslider/main.c:166; test_xui/xui_range_slider_test.c:170

## xuiRangeSliderSetRange
- 位置: xui.h:8400  已注释: 否
- 签名: `XUI_API int xuiRangeSliderSetRange(xui_widget pWidget, float fMin, float fMax);`
- 实现: src/xui_range_slider.c:1178（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_range_slider_test.c:321

## xuiRangeSliderGetRange
- 位置: xui.h:8401  已注释: 否
- 签名: `XUI_API int xuiRangeSliderGetRange(xui_widget pWidget, float* pMin, float* pMax);`
- 实现: src/xui_range_slider.c:1191（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_range_slider_test.c:325

## xuiRangeSliderSetValues
- 位置: xui.h:8402  已注释: 否
- 签名: `XUI_API int xuiRangeSliderSetValues(xui_widget pWidget, float fStart, float fEnd);`
- 实现: src/xui_range_slider.c:1200（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_range_slider_test.c:223; test_xui/xui_range_slider_test.c:225; test_xui/xui_range_slider_test.c:241

## xuiRangeSliderGetValues
- 位置: xui.h:8403  已注释: 否
- 签名: `XUI_API int xuiRangeSliderGetValues(xui_widget pWidget, float* pStart, float* pEnd);`
- 实现: src/xui_range_slider.c:1207（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_range_slider_test.c:327

## xuiRangeSliderGetStart
- 位置: xui.h:8404  已注释: 否
- 签名: `XUI_API float xuiRangeSliderGetStart(xui_widget pWidget);`
- 实现: src/xui_range_slider.c:1216（体 5 行）
- 用法: examples/xui_rangeslider/main.c:261; test_xui/xui_range_slider_test.c:175; test_xui/xui_range_slider_test.c:224

## xuiRangeSliderGetEnd
- 位置: xui.h:8405  已注释: 否
- 签名: `XUI_API float xuiRangeSliderGetEnd(xui_widget pWidget);`
- 实现: src/xui_range_slider.c:1222（体 5 行）
- 用法: examples/xui_rangeslider/main.c:261; test_xui/xui_range_slider_test.c:176; test_xui/xui_range_slider_test.c:224

## xuiRangeSliderGetStartRate
- 位置: xui.h:8406  已注释: 否
- 签名: `XUI_API float xuiRangeSliderGetStartRate(xui_widget pWidget);`
- 实现: src/xui_range_slider.c:1228（体 5 行）
- 用法: test_xui/xui_range_slider_test.c:177

## xuiRangeSliderGetEndRate
- 位置: xui.h:8407  已注释: 否
- 签名: `XUI_API float xuiRangeSliderGetEndRate(xui_widget pWidget);`
- 实现: src/xui_range_slider.c:1234（体 5 行）
- 用法: test_xui/xui_range_slider_test.c:178

## xuiRangeSliderSetStep
- 位置: xui.h:8408  已注释: 否
- 签名: `XUI_API int xuiRangeSliderSetStep(xui_widget pWidget, float fStep, float fPageStep);`
- 实现: src/xui_range_slider.c:1240（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_range_slider_test.c:204

## xuiRangeSliderGetStep
- 位置: xui.h:8409  已注释: 否
- 签名: `XUI_API int xuiRangeSliderGetStep(xui_widget pWidget, float* pStep, float* pPageStep);`
- 实现: src/xui_range_slider.c:1249（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_range_slider_test.c:206

## xuiRangeSliderSetIntervalLimits
- 位置: xui.h:8410  已注释: 否
- 签名: `XUI_API int xuiRangeSliderSetIntervalLimits(xui_widget pWidget, float fMinInterval, float fMaxInterval);`
- 实现: src/xui_range_slider.c:1258（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_rangeslider/main.c:202; examples/xui_rangeslider/main.c:203; examples/xui_rangeslider/main.c:214

## xuiRangeSliderGetIntervalLimits
- 位置: xui.h:8411  已注释: 否
- 签名: `XUI_API int xuiRangeSliderGetIntervalLimits(xui_widget pWidget, float* pMinInterval, float* pMaxInterval);`
- 实现: src/xui_range_slider.c:1277（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_rangeslider/main.c:257; examples/xui_rangeslider/main.c:259; test_xui/xui_range_slider_test.c:208

## xuiRangeSliderSetOrientation
- 位置: xui.h:8412  已注释: 否
- 签名: `XUI_API int xuiRangeSliderSetOrientation(xui_widget pWidget, int iOrientation);`
- 实现: src/xui_range_slider.c:1288（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_range_slider_test.c:307; test_xui/xui_slider_pixel_test.c:53

## xuiRangeSliderGetOrientation
- 位置: xui.h:8413  已注释: 否
- 签名: `XUI_API int xuiRangeSliderGetOrientation(xui_widget pWidget);`
- 实现: src/xui_range_slider.c:1297（体 5 行）
- 用法: examples/xui_rangeslider/main.c:262; test_xui/xui_range_slider_test.c:174; test_xui/xui_range_slider_test.c:308

## xuiRangeSliderSetMetrics
- 位置: xui.h:8414  已注释: 否
- 签名: `XUI_API int xuiRangeSliderSetMetrics(xui_widget pWidget, float fTrackSize, float fKnobSize);`
- 实现: src/xui_range_slider.c:1303（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_rangeslider/main.c:204; examples/xui_rangeslider/main.c:213; test_xui/xui_range_slider_test.c:196

## xuiRangeSliderGetMetrics
- 位置: xui.h:8415  已注释: 否
- 签名: `XUI_API int xuiRangeSliderGetMetrics(xui_widget pWidget, float* pTrackSize, float* pKnobSize);`
- 实现: src/xui_range_slider.c:1312（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_range_slider_test.c:198

## xuiRangeSliderSetColors
- 位置: xui.h:8416  已注释: 否
- 签名: `XUI_API int xuiRangeSliderSetColors(xui_widget pWidget, uint32_t iTrack, uint32_t iFill, uint32_t iKnob, uint32_t iFocus, uint32_t iDisabled);`
- 实现: src/xui_range_slider.c:1321（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_rangeslider/main.c:205; test_xui/xui_range_slider_test.c:200

## xuiRangeSliderGetColors
- 位置: xui.h:8417  已注释: 否
- 签名: `XUI_API int xuiRangeSliderGetColors(xui_widget pWidget, uint32_t* pTrack, uint32_t* pFill, uint32_t* pKnob, uint32_t* pFocus, uint32_t* pDisabled);`
- 实现: src/xui_range_slider.c:1335（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiRangeSliderSetKnobBorderColor
- 位置: xui.h:8418  已注释: 否
- 签名: `XUI_API int xuiRangeSliderSetKnobBorderColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_range_slider.c:1347（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_rangeslider/main.c:211; test_xui/xui_range_slider_test.c:202

## xuiRangeSliderGetKnobBorderColor
- 位置: xui.h:8419  已注释: 否
- 签名: `XUI_API uint32_t xuiRangeSliderGetKnobBorderColor(xui_widget pWidget);`
- 实现: src/xui_range_slider.c:1355（体 5 行）
- 用法: test_xui/xui_range_slider_test.c:203

## xuiRangeSliderGetTrackRect
- 位置: xui.h:8420  已注释: 否
- 签名: `XUI_API xui_rect_t xuiRangeSliderGetTrackRect(xui_widget pWidget);`
- 实现: src/xui_range_slider.c:1379（体 4 行）
- 用法: examples/xui_rangeslider/main.c:251; examples/xui_rangeslider/main.c:263; examples/xui_rangeslider/main.c:266

## xuiRangeSliderGetFillRect
- 位置: xui.h:8421  已注释: 否
- 签名: `XUI_API xui_rect_t xuiRangeSliderGetFillRect(xui_widget pWidget);`
- 实现: src/xui_range_slider.c:1384（体 4 行）
- 用法: examples/xui_rangeslider/main.c:252; test_xui/xui_range_slider_test.c:189; test_xui/xui_range_slider_test.c:315

## xuiRangeSliderGetStartKnobRect
- 位置: xui.h:8422  已注释: 否
- 签名: `XUI_API xui_rect_t xuiRangeSliderGetStartKnobRect(xui_widget pWidget);`
- 实现: src/xui_range_slider.c:1389（体 4 行）
- 用法: examples/xui_rangeslider/main.c:253; test_xui/xui_range_slider_test.c:190; test_xui/xui_range_slider_test.c:256

## xuiRangeSliderGetEndKnobRect
- 位置: xui.h:8423  已注释: 否
- 签名: `XUI_API xui_rect_t xuiRangeSliderGetEndKnobRect(xui_widget pWidget);`
- 实现: src/xui_range_slider.c:1394（体 4 行）
- 用法: examples/xui_rangeslider/main.c:254; test_xui/xui_range_slider_test.c:191; test_xui/xui_slider_pixel_test.c:90

## xuiRangeSliderGetActiveThumb
- 位置: xui.h:8424  已注释: 否
- 签名: `XUI_API int xuiRangeSliderGetActiveThumb(xui_widget pWidget);`
- 实现: src/xui_range_slider.c:1399（体 5 行）
- 用法: test_xui/xui_range_slider_test.c:252

## xuiRangeSliderGetState
- 位置: xui.h:8425  已注释: 否
- 签名: `XUI_API uint32_t xuiRangeSliderGetState(xui_widget pWidget);`
- 实现: src/xui_range_slider.c:1405（体 10 行）
- 用法: test_xui/xui_range_slider_test.c:261; test_xui/xui_range_slider_test.c:303; test_xui/xui_range_slider_test.c:331

## xuiRangeSliderGetChangeCount
- 位置: xui.h:8426  已注释: 否
- 签名: `XUI_API int xuiRangeSliderGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_range_slider.c:1416（体 5 行）
- 用法: test_xui/xui_slider_pixel_test.c:75; test_xui/xui_slider_pixel_test.c:100

