# 草稿包：xui.h / button（46 条 API）

> 生成 2026-09-10 02:55 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiButtonGetType
- 位置: xui.h:7562  已注释: 否
- 签名: `XUI_API xui_widget_type xuiButtonGetType(xui_context pContext);`
- 实现: src/xui_button.c:1314（体 34 行）
- 返回码: NULL

## xuiButtonCreate
- 位置: xui.h:7563  已注释: 否
- 签名: `XUI_API int xuiButtonCreate(xui_context pContext, xui_widget* ppWidget, const xui_button_desc_t* pDesc);`
- 实现: src/xui_button.c:1349（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/audit_xui_basic/main.c:21; examples/audit_xui_basic/main.c:23; examples/audit_xui_basic/main.c:25

## xuiButtonSetClick
- 位置: xui.h:7564  已注释: 否
- 签名: `XUI_API int xuiButtonSetClick(xui_widget pWidget, xui_button_click_proc onClick, void* pUser);`
- 实现: src/xui_button.c:1364（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_button/main.c:347; examples/xui_file_dialog/main.c:164; examples/xui_language/main.c:307

## xuiButtonSetText
- 位置: xui.h:7565  已注释: 否
- 签名: `XUI_API int xuiButtonSetText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_button.c:1377（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_accessibility_test.c:123; test_xui/xui_accessibility_test.c:164; test_xui/xui_accessibility_test.c:203

## xuiButtonGetText
- 位置: xui.h:7566  已注释: 否
- 签名: `XUI_API const char* xuiButtonGetText(xui_widget pWidget);`
- 实现: src/xui_button.c:1400（体 7 行）
- 用法: test_xui/xui_accessibility_test.c:122; test_xui/xui_button_test.c:804

## xuiButtonSetFont
- 位置: xui.h:7567  已注释: 否
- 签名: `XUI_API int xuiButtonSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_button.c:1408（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiButtonGetFont
- 位置: xui.h:7568  已注释: 否
- 签名: `XUI_API xui_font xuiButtonGetFont(xui_widget pWidget);`
- 实现: src/xui_button.c:1423（体 7 行）

## xuiButtonSetTextFlags
- 位置: xui.h:7569  已注释: 否
- 签名: `XUI_API int xuiButtonSetTextFlags(xui_widget pWidget, uint32_t iTextFlags);`
- 实现: src/xui_button.c:1431（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiButtonGetTextFlags
- 位置: xui.h:7570  已注释: 否
- 签名: `XUI_API uint32_t xuiButtonGetTextFlags(xui_widget pWidget);`
- 实现: src/xui_button.c:1443（体 7 行）

## xuiButtonSetTextColor
- 位置: xui.h:7571  已注释: 否
- 签名: `XUI_API int xuiButtonSetTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_button.c:1451（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_button/main.c:341; examples/xui_button/main.c:443; examples/xui_language/main.c:173

## xuiButtonGetTextColor
- 位置: xui.h:7572  已注释: 否
- 签名: `XUI_API uint32_t xuiButtonGetTextColor(xui_widget pWidget);`
- 实现: src/xui_button.c:1467（体 7 行）
- 用法: test_xui/xui_style_basic_buttons_test.c:27

## xuiButtonSetDisabledTextColor
- 位置: xui.h:7573  已注释: 否
- 签名: `XUI_API int xuiButtonSetDisabledTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_button.c:1475（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_button/main.c:444

## xuiButtonGetDisabledTextColor
- 位置: xui.h:7574  已注释: 否
- 签名: `XUI_API uint32_t xuiButtonGetDisabledTextColor(xui_widget pWidget);`
- 实现: src/xui_button.c:1490（体 7 行）

## xuiButtonSetSelectable
- 位置: xui.h:7575  已注释: 否
- 签名: `XUI_API int xuiButtonSetSelectable(xui_widget pWidget, int bSelectable);`
- 实现: src/xui_button.c:1498（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/tutorial_capture/ch133_main1.c:28; examples/xui_button/main.c:434; examples/xui_button/main.c:439

## xuiButtonGetSelectable
- 位置: xui.h:7576  已注释: 否
- 签名: `XUI_API int xuiButtonGetSelectable(xui_widget pWidget);`
- 实现: src/xui_button.c:1519（体 7 行）

## xuiButtonSetSelected
- 位置: xui.h:7577  已注释: 否
- 签名: `XUI_API int xuiButtonSetSelected(xui_widget pWidget, int bSelected);`
- 实现: src/xui_button.c:1527（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/tutorial_capture/ch133_main1.c:29; examples/xui_button/main.c:436; test_xui/xui_accessibility_test.c:296

## xuiButtonIsSelected
- 位置: xui.h:7578  已注释: 否
- 签名: `XUI_API int xuiButtonIsSelected(xui_widget pWidget);`
- 实现: src/xui_button.c:1546（体 7 行）
- 用法: examples/xui_button/main.c:516; test_xui/xui_button_test.c:890

## xuiButtonSetSemantic
- 位置: xui.h:7579  已注释: 否
- 签名: `XUI_API int xuiButtonSetSemantic(xui_widget pWidget, int iSemantic);`
- 实现: src/xui_button.c:1554（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiButtonGetSemantic
- 位置: xui.h:7580  已注释: 否
- 签名: `XUI_API int xuiButtonGetSemantic(xui_widget pWidget);`
- 实现: src/xui_button.c:1586（体 7 行）

## xuiButtonSetIcon
- 位置: xui.h:7581  已注释: 否
- 签名: `XUI_API int xuiButtonSetIcon(xui_widget pWidget, xui_surface pSurface, xui_rect_t tSrc);`
- 实现: src/xui_button.c:1594（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_button/main.c:429; test_xui/xui_button_pixel_test.c:112; test_xui/xui_button_pixel_test.c:124

## xuiButtonGetIcon
- 位置: xui.h:7582  已注释: 否
- 签名: `XUI_API int xuiButtonGetIcon(xui_widget pWidget, xui_surface* ppSurface, xui_rect_t* pSrc);`
- 实现: src/xui_button.c:1607（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiButtonGetIconSurface
- 位置: xui.h:7583  已注释: 否
- 签名: `XUI_API xui_surface xuiButtonGetIconSurface(xui_widget pWidget);`
- 实现: src/xui_button.c:1620（体 7 行）
- 用法: examples/xui_button/main.c:515

## xuiButtonSetIconColor
- 位置: xui.h:7584  已注释: 否
- 签名: `XUI_API int xuiButtonSetIconColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_button.c:1628（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_basic_buttons_test.c:24

## xuiButtonGetIconColor
- 位置: xui.h:7585  已注释: 否
- 签名: `XUI_API uint32_t xuiButtonGetIconColor(xui_widget pWidget);`
- 实现: src/xui_button.c:1643（体 7 行）
- 用法: test_xui/xui_style_basic_buttons_test.c:27

## xuiButtonSetIconLayout
- 位置: xui.h:7586  已注释: 否
- 签名: `XUI_API int xuiButtonSetIconLayout(xui_widget pWidget, int iPlacement, float fIconSize, float fGap);`
- 实现: src/xui_button.c:1651（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_button/main.c:430; test_xui/xui_button_pixel_test.c:57; test_xui/xui_button_test.c:822

## xuiButtonGetIconLayout
- 位置: xui.h:7587  已注释: 否
- 签名: `XUI_API int xuiButtonGetIconLayout(xui_widget pWidget, int* pPlacement, float* pIconSize, float* pGap);`
- 实现: src/xui_button.c:1668（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiButtonSetColors
- 位置: xui.h:7588  已注释: 否
- 签名: `XUI_API int xuiButtonSetColors(xui_widget pWidget, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);`
- 实现: src/xui_button.c:1682（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_button/main.c:342; examples/xui_language/main.c:174; test_xui/xui_button_test.c:865

## xuiButtonSetStateVisual
- 位置: xui.h:7589  已注释: 否
- 签名: `XUI_API int xuiButtonSetStateVisual(xui_widget pWidget, uint32_t iState, uint32_t iFill, float fBorderWidth, uint32_t iBorderColor);`
- 实现: src/xui_button.c:1701（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_button/main.c:358; examples/xui_button/main.c:445; test_xui/xui_native_state_matrix_test.c:99

## xuiButtonGetStateVisual
- 位置: xui.h:7590  已注释: 否
- 签名: `XUI_API int xuiButtonGetStateVisual(xui_widget pWidget, uint32_t iState, uint32_t* pFill, float* pBorderWidth, uint32_t* pBorderColor);`
- 实现: src/xui_button.c:1720（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiButtonSetBorder
- 位置: xui.h:7591  已注释: 否
- 签名: `XUI_API int xuiButtonSetBorder(xui_widget pWidget, float fBorderWidth, uint32_t iBorderColor);`
- 实现: src/xui_button.c:1736（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_button/main.c:343

## xuiButtonSetPatch
- 位置: xui.h:7592  已注释: 否
- 签名: `XUI_API int xuiButtonSetPatch(xui_widget pWidget, uint32_t iState, const xui_nine_patch_t* pPatch);`
- 实现: src/xui_button.c:1758（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_button/main.c:370; test_xui/xui_button_pixel_test.c:133; test_xui/xui_button_test.c:831

## xuiButtonGetPatch
- 位置: xui.h:7593  已注释: 否
- 签名: `XUI_API int xuiButtonGetPatch(xui_widget pWidget, uint32_t iState, xui_nine_patch_t* pPatch);`
- 实现: src/xui_button.c:1785（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR, XUI_OK

## xuiButtonClearPatch
- 位置: xui.h:7594  已注释: 否
- 签名: `XUI_API int xuiButtonClearPatch(xui_widget pWidget, uint32_t iState);`
- 实现: src/xui_button.c:1807（体 4 行）
- 用法: test_xui/xui_button_test.c:861

## xuiButtonHasPatch
- 位置: xui.h:7595  已注释: 否
- 签名: `XUI_API int xuiButtonHasPatch(xui_widget pWidget, uint32_t iState);`
- 实现: src/xui_button.c:1812（体 10 行）
- 用法: examples/xui_button/main.c:518; test_xui/xui_button_test.c:832

## xuiButtonSetBadgeVisible
- 位置: xui.h:7596  已注释: 否
- 签名: `XUI_API int xuiButtonSetBadgeVisible(xui_widget pWidget, int bVisible);`
- 实现: src/xui_button.c:1823（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_button/main.c:476; test_xui/xui_button_pixel_test.c:114; test_xui/xui_button_pixel_test.c:126

## xuiButtonGetBadgeVisible
- 位置: xui.h:7597  已注释: 否
- 签名: `XUI_API int xuiButtonGetBadgeVisible(xui_widget pWidget);`
- 实现: src/xui_button.c:1839（体 7 行）
- 用法: examples/xui_button/main.c:519; test_xui/xui_button_test.c:834

## xuiButtonSetBadgeAnchor
- 位置: xui.h:7598  已注释: 否
- 签名: `XUI_API int xuiButtonSetBadgeAnchor(xui_widget pWidget, int iAnchor);`
- 实现: src/xui_button.c:1847（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_button_pixel_test.c:83

## xuiButtonGetBadgeAnchor
- 位置: xui.h:7599  已注释: 否
- 签名: `XUI_API int xuiButtonGetBadgeAnchor(xui_widget pWidget);`
- 实现: src/xui_button.c:1862（体 7 行）

## xuiButtonSetBadgeOffset
- 位置: xui.h:7600  已注释: 否
- 签名: `XUI_API int xuiButtonSetBadgeOffset(xui_widget pWidget, float fX, float fY);`
- 实现: src/xui_button.c:1870（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_button_pixel_test.c:116

## xuiButtonGetBadgeOffset
- 位置: xui.h:7601  已注释: 否
- 签名: `XUI_API int xuiButtonGetBadgeOffset(xui_widget pWidget, float* pX, float* pY);`
- 实现: src/xui_button.c:1883（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiButtonSetBadgeSize
- 位置: xui.h:7602  已注释: 否
- 签名: `XUI_API int xuiButtonSetBadgeSize(xui_widget pWidget, float fSize);`
- 实现: src/xui_button.c:1896（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_button/main.c:477; test_xui/xui_button_pixel_test.c:115

## xuiButtonGetBadgeSize
- 位置: xui.h:7603  已注释: 否
- 签名: `XUI_API float xuiButtonGetBadgeSize(xui_widget pWidget);`
- 实现: src/xui_button.c:1911（体 7 行）

## xuiButtonSetBadgeSurface
- 位置: xui.h:7604  已注释: 否
- 签名: `XUI_API int xuiButtonSetBadgeSurface(xui_widget pWidget, xui_surface pSurface, xui_rect_t tSrc);`
- 实现: src/xui_button.c:1919（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_button_pixel_test.c:113; test_xui/xui_style_basic_buttons_test.c:37

## xuiButtonGetBadgeSurface
- 位置: xui.h:7605  已注释: 否
- 签名: `XUI_API int xuiButtonGetBadgeSurface(xui_widget pWidget, xui_surface* ppSurface, xui_rect_t* pSrc);`
- 实现: src/xui_button.c:1932（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiButtonGetState
- 位置: xui.h:7606  已注释: 否
- 签名: `XUI_API uint32_t xuiButtonGetState(xui_widget pWidget);`
- 实现: src/xui_button.c:1945（体 11 行）
- 用法: test_xui/xui_button_test.c:844; test_xui/xui_button_test.c:850; test_xui/xui_button_test.c:877

## xuiButtonGetClickCount
- 位置: xui.h:7607  已注释: 否
- 签名: `XUI_API int xuiButtonGetClickCount(xui_widget pWidget);`
- 实现: src/xui_button.c:1957（体 7 行）
- 用法: test_xui/xui_button_test.c:860; test_xui/xui_button_test.c:904; test_xui/xui_native_state_matrix_test.c:122

