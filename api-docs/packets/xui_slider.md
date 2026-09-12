# 草稿包：xui.h / slider（23 条 API）

> 生成 2026-09-10 03:05 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiSliderGetType
- 位置: xui.h:8415  已注释: 否
- 签名: `XUI_API xui_widget_type xuiSliderGetType(xui_context pContext);`
- 实现: src/xui_slider.c:1051（体 34 行）
- 返回码: NULL

## xuiSliderCreate
- 位置: xui.h:8416  已注释: 否
- 签名: `XUI_API int xuiSliderCreate(xui_context pContext, xui_widget* ppWidget, const xui_slider_desc_t* pDesc);`
- 实现: src/xui_slider.c:1086（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/audit_xui_basic/main.c:65; examples/audit_xui_basic/main.c:67; examples/audit_xui_basic/main.c:69

## xuiSliderSetChange
- 位置: xui.h:8417  已注释: 否
- 签名: `XUI_API int xuiSliderSetChange(xui_widget pWidget, xui_slider_change_proc onChange, void* pUser);`
- 实现: src/xui_slider.c:1101（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_slider/main.c:163; test_xui/xui_accessibility_test.c:258; test_xui/xui_slider_test.c:123

## xuiSliderSetRange
- 位置: xui.h:8418  已注释: 否
- 签名: `XUI_API int xuiSliderSetRange(xui_widget pWidget, float fMin, float fMax);`
- 实现: src/xui_slider.c:1110（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:255; test_xui/xui_slider_test.c:254

## xuiSliderGetRange
- 位置: xui.h:8419  已注释: 否
- 签名: `XUI_API int xuiSliderGetRange(xui_widget pWidget, float* pMin, float* pMax);`
- 实现: src/xui_slider.c:1124（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_slider_test.c:258

## xuiSliderSetValue
- 位置: xui.h:8420  已注释: 否
- 签名: `XUI_API int xuiSliderSetValue(xui_widget pWidget, float fValue);`
- 实现: src/xui_slider.c:1133（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:256; test_xui/xui_slider_pixel_test.c:72; test_xui/xui_slider_test.c:160

## xuiSliderGetValue
- 位置: xui.h:8421  已注释: 否
- 签名: `XUI_API float xuiSliderGetValue(xui_widget pWidget);`
- 实现: src/xui_slider.c:1140（体 5 行）
- 用法: test_xui/xui_accessibility_test.c:496; test_xui/xui_accessibility_test.c:498; test_xui/xui_accessibility_test.c:500

## xuiSliderGetRate
- 位置: xui.h:8422  已注释: 否
- 签名: `XUI_API float xuiSliderGetRate(xui_widget pWidget);`
- 实现: src/xui_slider.c:1146（体 5 行）
- 用法: test_xui/xui_slider_test.c:129

## xuiSliderSetStep
- 位置: xui.h:8423  已注释: 否
- 签名: `XUI_API int xuiSliderSetStep(xui_widget pWidget, float fStep, float fPageStep);`
- 实现: src/xui_slider.c:1152（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_accessibility_test.c:257; test_xui/xui_slider_test.c:154

## xuiSliderGetStep
- 位置: xui.h:8424  已注释: 否
- 签名: `XUI_API int xuiSliderGetStep(xui_widget pWidget, float* pStep, float* pPageStep);`
- 实现: src/xui_slider.c:1162（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_slider_test.c:156

## xuiSliderSetOrientation
- 位置: xui.h:8425  已注释: 否
- 签名: `XUI_API int xuiSliderSetOrientation(xui_widget pWidget, int iOrientation);`
- 实现: src/xui_slider.c:1171（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_slider_pixel_test.c:52; test_xui/xui_slider_test.c:240

## xuiSliderGetOrientation
- 位置: xui.h:8426  已注释: 否
- 签名: `XUI_API int xuiSliderGetOrientation(xui_widget pWidget);`
- 实现: src/xui_slider.c:1181（体 5 行）
- 用法: examples/xui_slider/main.c:254; test_xui/xui_slider_test.c:127; test_xui/xui_slider_test.c:241

## xuiSliderSetMetrics
- 位置: xui.h:8427  已注释: 否
- 签名: `XUI_API int xuiSliderSetMetrics(xui_widget pWidget, float fTrackSize, float fKnobSize, float fTrackRadius);`
- 实现: src/xui_slider.c:1187（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_slider/main.c:197; examples/xui_slider/main.c:206; test_xui/xui_slider_pixel_test.c:66

## xuiSliderGetMetrics
- 位置: xui.h:8428  已注释: 否
- 签名: `XUI_API int xuiSliderGetMetrics(xui_widget pWidget, float* pTrackSize, float* pKnobSize, float* pTrackRadius);`
- 实现: src/xui_slider.c:1197（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_slider_test.c:148

## xuiSliderSetColors
- 位置: xui.h:8429  已注释: 否
- 签名: `XUI_API int xuiSliderSetColors(xui_widget pWidget, uint32_t iTrack, uint32_t iFill, uint32_t iKnob, uint32_t iFocus, uint32_t iDisabled);`
- 实现: src/xui_slider.c:1207（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_slider/main.c:198; examples/xui_slider/main.c:207; test_xui/xui_slider_pixel_test.c:49

## xuiSliderGetColors
- 位置: xui.h:8430  已注释: 否
- 签名: `XUI_API int xuiSliderGetColors(xui_widget pWidget, uint32_t* pTrack, uint32_t* pFill, uint32_t* pKnob, uint32_t* pFocus, uint32_t* pDisabled);`
- 实现: src/xui_slider.c:1221（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiSliderSetKnobBorderColor
- 位置: xui.h:8431  已注释: 否
- 签名: `XUI_API int xuiSliderSetKnobBorderColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_slider.c:1233（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_slider/main.c:204; examples/xui_slider/main.c:213; test_xui/xui_slider_test.c:152

## xuiSliderGetKnobBorderColor
- 位置: xui.h:8432  已注释: 否
- 签名: `XUI_API uint32_t xuiSliderGetKnobBorderColor(xui_widget pWidget);`
- 实现: src/xui_slider.c:1241（体 5 行）
- 用法: test_xui/xui_slider_test.c:153

## xuiSliderGetTrackRect
- 位置: xui.h:8433  已注释: 否
- 签名: `XUI_API xui_rect_t xuiSliderGetTrackRect(xui_widget pWidget);`
- 实现: src/xui_slider.c:1264（体 4 行）
- 用法: examples/xui_slider/main.c:249; examples/xui_slider/main.c:254; examples/xui_slider/main.c:257

## xuiSliderGetFillRect
- 位置: xui.h:8434  已注释: 否
- 签名: `XUI_API xui_rect_t xuiSliderGetFillRect(xui_widget pWidget);`
- 实现: src/xui_slider.c:1269（体 4 行）
- 用法: examples/xui_slider/main.c:250; test_xui/xui_slider_pixel_test.c:80; test_xui/xui_slider_pixel_test.c:85

## xuiSliderGetKnobRect
- 位置: xui.h:8435  已注释: 否
- 签名: `XUI_API xui_rect_t xuiSliderGetKnobRect(xui_widget pWidget);`
- 实现: src/xui_slider.c:1274（体 4 行）
- 用法: examples/xui_slider/main.c:251; test_xui/xui_slider_pixel_test.c:88; test_xui/xui_slider_test.c:141

## xuiSliderGetState
- 位置: xui.h:8436  已注释: 否
- 签名: `XUI_API uint32_t xuiSliderGetState(xui_widget pWidget);`
- 实现: src/xui_slider.c:1279（体 10 行）
- 用法: test_xui/xui_slider_test.c:175; test_xui/xui_slider_test.c:236; test_xui/xui_slider_test.c:262

## xuiSliderGetChangeCount
- 位置: xui.h:8437  已注释: 否
- 签名: `XUI_API int xuiSliderGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_slider.c:1290（体 5 行）
- 用法: test_xui/xui_slider_pixel_test.c:74; test_xui/xui_slider_pixel_test.c:100

