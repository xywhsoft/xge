# 草稿包：xui.h / numeric_input（48 条 API）

> 生成 2026-09-10 03:02 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiNumericInputGetType
- 位置: xui.h:7542  已注释: 否
- 签名: `XUI_API xui_widget_type xuiNumericInputGetType(xui_context pContext);`
- 实现: src/xui_numeric_input.c:1243（体 38 行）
- 返回码: NULL
- 用法: test_xui/xui_property_grid_test.c:133; test_xui/xui_table_grid_test.c:258

## xuiNumericInputCreate
- 位置: xui.h:7543  已注释: 否
- 签名: `XUI_API int xuiNumericInputCreate(xui_context pContext, xui_widget* ppWidget, const xui_numeric_input_desc_t* pDesc);`
- 实现: src/xui_numeric_input.c:1282（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch139_main1.c:21; examples/tutorial_capture/ch139_main1.c:34; examples/xui_numericinput/main.c:184

## xuiNumericInputSetChange
- 位置: xui.h:7544  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetChange(xui_widget pWidget, xui_numeric_input_change_proc onChange, void* pUser);`
- 实现: src/xui_numeric_input.c:1297（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_numericinput/main.c:186; test_xui/xui_numeric_input_test.c:195

## xuiNumericInputSetErrorChange
- 位置: xui.h:7545  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetErrorChange(xui_widget pWidget, xui_numeric_input_error_proc onError, void* pUser);`
- 实现: src/xui_numeric_input.c:1306（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_numeric_input_test.c:197

## xuiNumericInputSetFormatter
- 位置: xui.h:7546  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetFormatter(xui_widget pWidget, xui_numeric_input_format_proc onFormat, void* pUser);`
- 实现: src/xui_numeric_input.c:1315（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiNumericInputSetRange
- 位置: xui.h:7547  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetRange(xui_widget pWidget, float fMin, float fMax);`
- 实现: src/xui_numeric_input.c:1324（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_numeric_input_test.c:270; test_xui/xui_numeric_input_test.c:291

## xuiNumericInputGetRange
- 位置: xui.h:7548  已注释: 否
- 签名: `XUI_API int xuiNumericInputGetRange(xui_widget pWidget, float* pMin, float* pMax);`
- 实现: src/xui_numeric_input.c:1334（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_numeric_input_test.c:234

## xuiNumericInputSetStep
- 位置: xui.h:7549  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetStep(xui_widget pWidget, float fStep);`
- 实现: src/xui_numeric_input.c:1343（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_numeric_input_test.c:289

## xuiNumericInputGetStep
- 位置: xui.h:7550  已注释: 否
- 签名: `XUI_API float xuiNumericInputGetStep(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1351（体 5 行）
- 用法: test_xui/xui_numeric_input_test.c:231

## xuiNumericInputSetInteger
- 位置: xui.h:7551  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetInteger(xui_widget pWidget, int bInteger);`
- 实现: src/xui_numeric_input.c:1357（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_numeric_input_test.c:264; test_xui/xui_numeric_input_test.c:285

## xuiNumericInputIsInteger
- 位置: xui.h:7552  已注释: 否
- 签名: `XUI_API int xuiNumericInputIsInteger(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1365（体 5 行）
- 用法: test_xui/xui_numeric_input_test.c:265

## xuiNumericInputSetPrecision
- 位置: xui.h:7553  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetPrecision(xui_widget pWidget, int iPrecision);`
- 实现: src/xui_numeric_input.c:1371（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_numeric_input_test.c:287

## xuiNumericInputGetPrecision
- 位置: xui.h:7554  已注释: 否
- 签名: `XUI_API int xuiNumericInputGetPrecision(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1379（体 5 行）
- 用法: test_xui/xui_numeric_input_test.c:232

## xuiNumericInputSetFont
- 位置: xui.h:7555  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_numeric_input.c:1385（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiNumericInputGetFont
- 位置: xui.h:7556  已注释: 否
- 签名: `XUI_API xui_font xuiNumericInputGetFont(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1394（体 5 行）

## xuiNumericInputSetReadonly
- 位置: xui.h:7557  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetReadonly(xui_widget pWidget, int bReadonly);`
- 实现: src/xui_numeric_input.c:1400（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_numericinput/main.c:225; test_xui/xui_numeric_input_test.c:277; test_xui/xui_numeric_input_test.c:282

## xuiNumericInputIsReadonly
- 位置: xui.h:7558  已注释: 否
- 签名: `XUI_API int xuiNumericInputIsReadonly(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1409（体 5 行）
- 用法: test_xui/xui_numeric_input_test.c:278

## xuiNumericInputSetSpinnerVisible
- 位置: xui.h:7559  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetSpinnerVisible(xui_widget pWidget, int bVisible);`
- 实现: src/xui_numeric_input.c:1415（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_numericinput/main.c:226; test_xui/xui_numeric_input_test.c:413

## xuiNumericInputGetSpinnerVisible
- 位置: xui.h:7560  已注释: 否
- 签名: `XUI_API int xuiNumericInputGetSpinnerVisible(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1425（体 5 行）
- 用法: test_xui/xui_numeric_input_test.c:233; test_xui/xui_numeric_input_test.c:414

## xuiNumericInputSetSpinnerWidth
- 位置: xui.h:7561  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetSpinnerWidth(xui_widget pWidget, float fWidth);`
- 实现: src/xui_numeric_input.c:1431（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiNumericInputGetSpinnerWidth
- 位置: xui.h:7562  已注释: 否
- 签名: `XUI_API float xuiNumericInputGetSpinnerWidth(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1439（体 5 行）

## xuiNumericInputSetColors
- 位置: xui.h:7563  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iText, uint32_t iBorder, uint32_t iFocus);`
- 实现: src/xui_numeric_input.c:1445（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiNumericInputGetColors
- 位置: xui.h:7564  已注释: 否
- 签名: `XUI_API int xuiNumericInputGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pText, uint32_t* pBorder, uint32_t* pFocus);`
- 实现: src/xui_numeric_input.c:1460（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiNumericInputSetExtendedColors
- 位置: xui.h:7565  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetExtendedColors(xui_widget pWidget, uint32_t iPlaceholder, uint32_t iDisabledText, uint32_t iHoverBackground, uint32_t iDisabledBackground, uint32_t iHoverBorder, uint32_t iErrorBackground, uint32_t iErrorBorder, uint32_t iSelection, uint32_t iCursor);`
- 实现: src/xui_numeric_input.c:1471（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiNumericInputGetExtendedColors
- 位置: xui.h:7566  已注释: 否
- 签名: `XUI_API int xuiNumericInputGetExtendedColors(xui_widget pWidget, uint32_t* pPlaceholder, uint32_t* pDisabledText, uint32_t* pHoverBackground, uint32_t* pDisabledBackground, uint32_t* pHoverBorder, uint32_t* pErrorBackground, uint32_t* pErrorBorder, uint32_t* pSelection, uint32_t* pCursor);`
- 实现: src/xui_numeric_input.c:1491（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiNumericInputSetBorderWidth
- 位置: xui.h:7567  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetBorderWidth(xui_widget pWidget, float fBorderWidth);`
- 实现: src/xui_numeric_input.c:1507（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiNumericInputGetBorderWidth
- 位置: xui.h:7568  已注释: 否
- 签名: `XUI_API float xuiNumericInputGetBorderWidth(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1516（体 5 行）

## xuiNumericInputSetSpinnerColors
- 位置: xui.h:7569  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetSpinnerColors(xui_widget pWidget, uint32_t iColor, uint32_t iHover, uint32_t iActive, uint32_t iBorder, uint32_t iIcon, uint32_t iDisabledIcon);`
- 实现: src/xui_numeric_input.c:1522（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_numericinput/main.c:218

## xuiNumericInputGetSpinnerColors
- 位置: xui.h:7570  已注释: 否
- 签名: `XUI_API int xuiNumericInputGetSpinnerColors(xui_widget pWidget, uint32_t* pColor, uint32_t* pHover, uint32_t* pActive, uint32_t* pBorder, uint32_t* pIcon, uint32_t* pDisabledIcon);`
- 实现: src/xui_numeric_input.c:1535（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiNumericInputSetValue
- 位置: xui.h:7571  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetValue(xui_widget pWidget, float fValue);`
- 实现: src/xui_numeric_input.c:1548（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_inventory_grid_test.c:529; test_xui/xui_numeric_input_test.c:266; test_xui/xui_numeric_input_test.c:272

## xuiNumericInputGetValue
- 位置: xui.h:7572  已注释: 否
- 签名: `XUI_API float xuiNumericInputGetValue(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1555（体 5 行）
- 用法: examples/xui_numericinput/main.c:304; examples/xui_numericinput/main.c:313; examples/xui_numericinput/main.c:316

## xuiNumericInputCommit
- 位置: xui.h:7573  已注释: 否
- 签名: `XUI_API int xuiNumericInputCommit(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1561（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_numericinput/main.c:330; examples/xui_numericinput/main.c:333; test_xui/xui_numeric_input_test.c:251

## xuiNumericInputStep
- 位置: xui.h:7574  已注释: 否
- 签名: `XUI_API int xuiNumericInputStep(xui_widget pWidget, int iDirection);`
- 实现: src/xui_numeric_input.c:1568（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_numeric_input_test.c:244; test_xui/xui_numeric_input_test.c:280; test_xui/xui_numeric_input_test.c:381

## xuiNumericInputSetText
- 位置: xui.h:7575  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_numeric_input.c:1575（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_numericinput/main.c:329; examples/xui_numericinput/main.c:332; test_xui/xui_numeric_input_test.c:249

## xuiNumericInputGetText
- 位置: xui.h:7576  已注释: 否
- 签名: `XUI_API const char* xuiNumericInputGetText(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1582（体 5 行）
- 用法: test_xui/xui_numeric_input_test.c:229; test_xui/xui_numeric_input_test.c:247; test_xui/xui_numeric_input_test.c:253

## xuiNumericInputSetMenuTitle
- 位置: xui.h:7577  已注释: 否
- 签名: `XUI_API int xuiNumericInputSetMenuTitle(xui_widget pWidget, int iCommand, const char* sTitle);`
- 实现: src/xui_numeric_input.c:1588（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiNumericInputGetMenuTitle
- 位置: xui.h:7578  已注释: 否
- 签名: `XUI_API const char* xuiNumericInputGetMenuTitle(xui_widget pWidget, int iCommand);`
- 实现: src/xui_numeric_input.c:1595（体 5 行）
- 用法: examples/xui_numericinput/main.c:309; test_xui/xui_numeric_input_test.c:388; test_xui/xui_numeric_input_test.c:390

## xuiNumericInputOpenMenu
- 位置: xui.h:7579  已注释: 否
- 签名: `XUI_API int xuiNumericInputOpenMenu(xui_widget pWidget, float fX, float fY);`
- 实现: src/xui_numeric_input.c:1601（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_numeric_input_test.c:393

## xuiNumericInputGetMenuWidget
- 位置: xui.h:7580  已注释: 否
- 签名: `XUI_API xui_widget xuiNumericInputGetMenuWidget(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1611（体 5 行）
- 用法: examples/xui_numericinput/main.c:307; test_xui/xui_numeric_input_test.c:227; test_xui/xui_numeric_input_test.c:386

## xuiNumericInputGetInputWidget
- 位置: xui.h:7581  已注释: 否
- 签名: `XUI_API xui_widget xuiNumericInputGetInputWidget(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1617（体 5 行）
- 用法: examples/xui_numericinput/main.c:311; test_xui/xui_numeric_input_test.c:221; test_xui/xui_numeric_input_test.c:224

## xuiNumericInputGetSpinnerRect
- 位置: xui.h:7582  已注释: 否
- 签名: `XUI_API xui_rect_t xuiNumericInputGetSpinnerRect(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1623（体 9 行）
- 用法: examples/xui_numericinput/main.c:302; examples/xui_numericinput/main.c:306; test_xui/xui_numeric_input_test.c:236

## xuiNumericInputGetButtonRect
- 位置: xui.h:7583  已注释: 否
- 签名: `XUI_API xui_rect_t xuiNumericInputGetButtonRect(xui_widget pWidget, int iButton);`
- 实现: src/xui_numeric_input.c:1633（体 11 行）
- 用法: examples/xui_numericinput/main.c:319; test_xui/xui_numeric_input_test.c:339; test_xui/xui_style_basic_input_test.c:128

## xuiNumericInputGetHoverButton
- 位置: xui.h:7584  已注释: 否
- 签名: `XUI_API int xuiNumericInputGetHoverButton(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1645（体 5 行）
- 用法: test_xui/xui_numeric_input_test.c:344

## xuiNumericInputGetActiveButton
- 位置: xui.h:7585  已注释: 否
- 签名: `XUI_API int xuiNumericInputGetActiveButton(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1651（体 5 行）
- 用法: test_xui/xui_numeric_input_test.c:348

## xuiNumericInputIsButtonEnabled
- 位置: xui.h:7586  已注释: 否
- 签名: `XUI_API int xuiNumericInputIsButtonEnabled(xui_widget pWidget, int iButton);`
- 实现: src/xui_numeric_input.c:1657（体 6 行）
- 用法: test_xui/xui_numeric_input_test.c:274; test_xui/xui_numeric_input_test.c:275

## xuiNumericInputGetError
- 位置: xui.h:7587  已注释: 否
- 签名: `XUI_API int xuiNumericInputGetError(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1664（体 5 行）
- 用法: examples/xui_numericinput/main.c:331; test_xui/xui_numeric_input_test.c:252; test_xui/xui_numeric_input_test.c:259

## xuiNumericInputGetState
- 位置: xui.h:7588  已注释: 否
- 签名: `XUI_API uint32_t xuiNumericInputGetState(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1670（体 5 行）

## xuiNumericInputGetChangeCount
- 位置: xui.h:7589  已注释: 否
- 签名: `XUI_API int xuiNumericInputGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_numeric_input.c:1676（体 5 行）
- 用法: examples/xui_numericinput/main.c:305

