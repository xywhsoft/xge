# 草稿包：xui.h / colorpicker（52 条 API）

> 生成 2026-09-10 02:58 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiColorPickerGetType
- 位置: xui.h:9730  已注释: 否
- 签名: `XUI_API xui_widget_type xuiColorPickerGetType(xui_context pContext);`
- 实现: src/xui_color_picker.c:2174（体 33 行）
- 返回码: NULL
- 用法: test_xui/xui_property_grid_test.c:594; test_xui/xui_table_grid_test.c:691

## xuiColorPickerCreate
- 位置: xui.h:9731  已注释: 否
- 签名: `XUI_API int xuiColorPickerCreate(xui_context pContext, xui_widget* ppWidget, const xui_color_picker_desc_t* pDesc);`
- 实现: src/xui_color_picker.c:2208（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch166_main1.c:22; examples/xui_colorpicker/main.c:197; examples/xui_colorpicker/main.c:207

## xuiColorPickerSetChange
- 位置: xui.h:9732  已注释: 否
- 签名: `XUI_API int xuiColorPickerSetChange(xui_widget pWidget, xui_color_picker_change_proc onChange, void* pUser);`
- 实现: src/xui_color_picker.c:2223（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_colorpicker/main.c:235; test_xui/xui_color_picker_test.c:183

## xuiColorPickerSetColor
- 位置: xui.h:9733  已注释: 否
- 签名: `XUI_API int xuiColorPickerSetColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_color_picker.c:2232（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiColorPickerGetColor
- 位置: xui.h:9734  已注释: 否
- 签名: `XUI_API uint32_t xuiColorPickerGetColor(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2239（体 5 行）
- 用法: examples/xui_colorpicker/main.c:327; examples/xui_colorpicker/main.c:334; test_xui/xui_color_picker_test.c:235

## xuiColorPickerSetRGBA
- 位置: xui.h:9735  已注释: 否
- 签名: `XUI_API int xuiColorPickerSetRGBA(xui_widget pWidget, int iR, int iG, int iB, int iA);`
- 实现: src/xui_color_picker.c:2245（体 8 行）

## xuiColorPickerGetRGBA
- 位置: xui.h:9736  已注释: 否
- 签名: `XUI_API int xuiColorPickerGetRGBA(xui_widget pWidget, int* pR, int* pG, int* pB, int* pA);`
- 实现: src/xui_color_picker.c:2254（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_color_picker_test.c:201; test_xui/xui_color_picker_test.c:271

## xuiColorPickerSetAlphaEnabled
- 位置: xui.h:9737  已注释: 否
- 签名: `XUI_API int xuiColorPickerSetAlphaEnabled(xui_widget pWidget, int bEnabled);`
- 实现: src/xui_color_picker.c:2267（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiColorPickerGetAlphaEnabled
- 位置: xui.h:9738  已注释: 否
- 签名: `XUI_API int xuiColorPickerGetAlphaEnabled(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2281（体 5 行）
- 用法: test_xui/xui_color_picker_test.c:199

## xuiColorPickerSetHex
- 位置: xui.h:9739  已注释: 否
- 签名: `XUI_API int xuiColorPickerSetHex(xui_widget pWidget, const char* sHex);`
- 实现: src/xui_color_picker.c:2287（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_colorpicker/main.c:318; examples/xui_colorpicker/main.c:319; test_xui/xui_color_picker_test.c:203

## xuiColorPickerGetHex
- 位置: xui.h:9740  已注释: 否
- 签名: `XUI_API const char* xuiColorPickerGetHex(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2299（体 5 行）
- 用法: examples/xui_colorpicker/main.c:320; examples/xui_colorpicker/main.c:321; test_xui/xui_color_picker_test.c:200

## xuiColorPickerSetPalette
- 位置: xui.h:9741  已注释: 否
- 签名: `XUI_API int xuiColorPickerSetPalette(xui_widget pWidget, const uint32_t* arrColors, int iCount);`
- 实现: src/xui_color_picker.c:2305（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_pickers_color_picker_test.c:55

## xuiColorPickerGetPaletteCount
- 位置: xui.h:9742  已注释: 否
- 签名: `XUI_API int xuiColorPickerGetPaletteCount(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2320（体 5 行）
- 用法: test_xui/xui_color_picker_test.c:198

## xuiColorPickerGetPaletteColor
- 位置: xui.h:9743  已注释: 否
- 签名: `XUI_API uint32_t xuiColorPickerGetPaletteColor(xui_widget pWidget, int iIndex);`
- 实现: src/xui_color_picker.c:2326（体 6 行）
- 用法: examples/xui_colorpicker/main.c:325; test_xui/xui_style_pickers_color_picker_test.c:117

## xuiColorPickerOpen
- 位置: xui.h:9744  已注释: 否
- 签名: `XUI_API int xuiColorPickerOpen(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2333（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_colorpicker/main.c:323; examples/xui_colorpicker/main.c:330; examples/xui_colorpicker/main.c:338

## xuiColorPickerClose
- 位置: xui.h:9745  已注释: 否
- 签名: `XUI_API int xuiColorPickerClose(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2340（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_colorpicker/main.c:328; examples/xui_colorpicker/main.c:336; test_xui/xui_property_grid_test.c:600

## xuiColorPickerToggle
- 位置: xui.h:9746  已注释: 否
- 签名: `XUI_API int xuiColorPickerToggle(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2354（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiColorPickerIsOpen
- 位置: xui.h:9747  已注释: 否
- 签名: `XUI_API int xuiColorPickerIsOpen(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2361（体 5 行）
- 用法: examples/xui_colorpicker/main.c:339; test_xui/xui_color_picker_test.c:211; test_xui/xui_color_picker_test.c:234

## xuiColorPickerSetPopupSize
- 位置: xui.h:9748  已注释: 否
- 签名: `XUI_API int xuiColorPickerSetPopupSize(xui_widget pWidget, float fWidth, float fHeight);`
- 实现: src/xui_color_picker.c:2367（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiColorPickerGetPopupSize
- 位置: xui.h:9749  已注释: 否
- 签名: `XUI_API int xuiColorPickerGetPopupSize(xui_widget pWidget, float* pWidth, float* pHeight);`
- 实现: src/xui_color_picker.c:2377（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiColorPickerSetPopupPlacement
- 位置: xui.h:9750  已注释: 否
- 签名: `XUI_API int xuiColorPickerSetPopupPlacement(xui_widget pWidget, int iPlacement);`
- 实现: src/xui_color_picker.c:2386（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiColorPickerGetPopupPlacement
- 位置: xui.h:9751  已注释: 否
- 签名: `XUI_API int xuiColorPickerGetPopupPlacement(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2397（体 5 行）

## xuiColorPickerSetMetrics
- 位置: xui.h:9752  已注释: 否
- 签名: `XUI_API int xuiColorPickerSetMetrics(xui_widget pWidget, float fBorderWidth);`
- 实现: src/xui_color_picker.c:2403（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiColorPickerGetMetrics
- 位置: xui.h:9753  已注释: 否
- 签名: `XUI_API int xuiColorPickerGetMetrics(xui_widget pWidget, float* pBorderWidth);`
- 实现: src/xui_color_picker.c:2411（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiColorPickerSetColors
- 位置: xui.h:9754  已注释: 否
- 签名: `XUI_API int xuiColorPickerSetColors(xui_widget pWidget, uint32_t iText, uint32_t iDisabledText, uint32_t iBackground, uint32_t iHoverBackground, uint32_t iOpenBackground, uint32_t iDisabledBackground);`
- 实现: src/xui_color_picker.c:2419（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_colorpicker/main.c:209

## xuiColorPickerGetColors
- 位置: xui.h:9755  已注释: 否
- 签名: `XUI_API int xuiColorPickerGetColors(xui_widget pWidget, uint32_t* pText, uint32_t* pDisabledText, uint32_t* pBackground, uint32_t* pHoverBackground, uint32_t* pOpenBackground, uint32_t* pDisabledBackground);`
- 实现: src/xui_color_picker.c:2432（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiColorPickerSetBorderColors
- 位置: xui.h:9756  已注释: 否
- 签名: `XUI_API int xuiColorPickerSetBorderColors(xui_widget pWidget, uint32_t iBorder, uint32_t iHoverBorder, uint32_t iFocusBorder);`
- 实现: src/xui_color_picker.c:2445（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_colorpicker/main.c:216

## xuiColorPickerGetBorderColors
- 位置: xui.h:9757  已注释: 否
- 签名: `XUI_API int xuiColorPickerGetBorderColors(xui_widget pWidget, uint32_t* pBorder, uint32_t* pHoverBorder, uint32_t* pFocusBorder);`
- 实现: src/xui_color_picker.c:2455（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiColorPickerSetArrowColors
- 位置: xui.h:9758  已注释: 否
- 签名: `XUI_API int xuiColorPickerSetArrowColors(xui_widget pWidget, uint32_t iArrow, uint32_t iDisabledArrow);`
- 实现: src/xui_color_picker.c:2465（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiColorPickerGetArrowColors
- 位置: xui.h:9759  已注释: 否
- 签名: `XUI_API int xuiColorPickerGetArrowColors(xui_widget pWidget, uint32_t* pArrow, uint32_t* pDisabledArrow);`
- 实现: src/xui_color_picker.c:2474（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiColorPickerSetButtonColors
- 位置: xui.h:9760  已注释: 否
- 签名: `XUI_API int xuiColorPickerSetButtonColors(xui_widget pWidget, uint32_t iButton, uint32_t iButtonHover, uint32_t iButtonOpen);`
- 实现: src/xui_color_picker.c:2483（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_pickers_color_picker_test.c:61

## xuiColorPickerGetButtonColors
- 位置: xui.h:9761  已注释: 否
- 签名: `XUI_API int xuiColorPickerGetButtonColors(xui_widget pWidget, uint32_t* pButton, uint32_t* pButtonHover, uint32_t* pButtonOpen);`
- 实现: src/xui_color_picker.c:2493（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_pickers_color_picker_test.c:64

## xuiColorPickerSetPopupColors
- 位置: xui.h:9762  已注释: 否
- 签名: `XUI_API int xuiColorPickerSetPopupColors(xui_widget pWidget, uint32_t iPanel, uint32_t iBorder, uint32_t iShadow, uint32_t iText, uint32_t iMutedText, uint32_t iAccent, uint32_t iField, uint32_t iFieldBorder, uint32_t iSeparator);`
- 实现: src/xui_color_picker.c:2503（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_colorpicker/main.c:220

## xuiColorPickerGetPopupColors
- 位置: xui.h:9763  已注释: 否
- 签名: `XUI_API int xuiColorPickerGetPopupColors(xui_widget pWidget, uint32_t* pPanel, uint32_t* pBorder, uint32_t* pShadow, uint32_t* pText, uint32_t* pMutedText, uint32_t* pAccent, uint32_t* pField, uint32_t* pFieldBorder, uint32_t* pSeparator);`
- 实现: src/xui_color_picker.c:2520（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiColorPickerSetFont
- 位置: xui.h:9764  已注释: 否
- 签名: `XUI_API int xuiColorPickerSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_color_picker.c:2536（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiColorPickerGetFont
- 位置: xui.h:9765  已注释: 否
- 签名: `XUI_API xui_font xuiColorPickerGetFont(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2544（体 5 行）

## xuiColorPickerGetPopupWidget
- 位置: xui.h:9766  已注释: 否
- 签名: `XUI_API xui_widget xuiColorPickerGetPopupWidget(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2550（体 5 行）
- 用法: examples/xui_colorpicker/main.c:315; test_xui/xui_color_picker_test.c:217; test_xui/xui_grid_focus_test.c:251

## xuiColorPickerGetPanelWidget
- 位置: xui.h:9767  已注释: 否
- 签名: `XUI_API xui_widget xuiColorPickerGetPanelWidget(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2556（体 5 行）
- 用法: examples/xui_colorpicker/main.c:278; examples/xui_colorpicker/main.c:293; test_xui/xui_color_picker_test.c:218

## xuiColorPickerGetSwatchRect
- 位置: xui.h:9768  已注释: 否
- 签名: `XUI_API xui_rect_t xuiColorPickerGetSwatchRect(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2562（体 7 行）

## xuiColorPickerGetButtonRect
- 位置: xui.h:9769  已注释: 否
- 签名: `XUI_API xui_rect_t xuiColorPickerGetButtonRect(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2570（体 7 行）
- 用法: examples/xui_colorpicker/main.c:316; test_xui/xui_color_picker_test.c:208

## xuiColorPickerGetTextRect
- 位置: xui.h:9770  已注释: 否
- 签名: `XUI_API xui_rect_t xuiColorPickerGetTextRect(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2578（体 7 行）

## xuiColorPickerGetSvRect
- 位置: xui.h:9771  已注释: 否
- 签名: `XUI_API xui_rect_t xuiColorPickerGetSvRect(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2586（体 7 行）
- 用法: test_xui/xui_color_picker_test.c:221

## xuiColorPickerGetHueRect
- 位置: xui.h:9772  已注释: 否
- 签名: `XUI_API xui_rect_t xuiColorPickerGetHueRect(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2594（体 7 行）

## xuiColorPickerGetAlphaRect
- 位置: xui.h:9773  已注释: 否
- 签名: `XUI_API xui_rect_t xuiColorPickerGetAlphaRect(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2602（体 7 行）
- 用法: examples/xui_colorpicker/main.c:332; test_xui/xui_color_picker_test.c:239

## xuiColorPickerGetHexRect
- 位置: xui.h:9774  已注释: 否
- 签名: `XUI_API xui_rect_t xuiColorPickerGetHexRect(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2610（体 7 行）
- 用法: test_xui/xui_color_picker_test.c:222; test_xui/xui_color_picker_test.c:251; test_xui/xui_grid_focus_test.c:389

## xuiColorPickerGetOldRect
- 位置: xui.h:9775  已注释: 否
- 签名: `XUI_API xui_rect_t xuiColorPickerGetOldRect(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2618（体 7 行）
- 用法: test_xui/xui_color_picker_test.c:223; test_xui/xui_color_picker_test.c:236; test_xui/xui_color_picker_test.c:261

## xuiColorPickerGetNewRect
- 位置: xui.h:9776  已注释: 否
- 签名: `XUI_API xui_rect_t xuiColorPickerGetNewRect(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2626（体 7 行）
- 用法: test_xui/xui_color_picker_test.c:224

## xuiColorPickerGetPaletteRect
- 位置: xui.h:9777  已注释: 否
- 签名: `XUI_API xui_rect_t xuiColorPickerGetPaletteRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_color_picker.c:2634（体 7 行）
- 用法: examples/xui_colorpicker/main.c:326; test_xui/xui_color_picker_test.c:233

## xuiColorPickerGetHoverPart
- 位置: xui.h:9778  已注释: 否
- 签名: `XUI_API int xuiColorPickerGetHoverPart(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2642（体 5 行）

## xuiColorPickerGetActivePart
- 位置: xui.h:9779  已注释: 否
- 签名: `XUI_API int xuiColorPickerGetActivePart(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2648（体 5 行）

## xuiColorPickerGetState
- 位置: xui.h:9780  已注释: 否
- 签名: `XUI_API uint32_t xuiColorPickerGetState(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2654（体 5 行）
- 用法: test_xui/xui_color_picker_test.c:228; test_xui/xui_color_picker_test.c:283

## xuiColorPickerGetChangeCount
- 位置: xui.h:9781  已注释: 否
- 签名: `XUI_API int xuiColorPickerGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_color_picker.c:2660（体 5 行）

