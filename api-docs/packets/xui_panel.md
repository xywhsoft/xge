# 草稿包：xui.h / panel（42 条 API）

> 生成 2026-09-10 03:03 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiPanelGetType
- 位置: xui.h:6536  已注释: 否
- 签名: `XUI_API xui_widget_type xuiPanelGetType(xui_context pContext);`
- 实现: src/xui_panel.c:587（体 34 行）
- 返回码: NULL

## xuiPanelCreate
- 位置: xui.h:6537  已注释: 否
- 签名: `XUI_API int xuiPanelCreate(xui_context pContext, xui_widget* ppWidget, const xui_panel_desc_t* pDesc);`
- 实现: src/xui_panel.c:622（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch143_main1.c:15; examples/tutorial_capture/ch191_main1.c:14; examples/tutorial_capture/ch195_main1.c:14

## xuiPanelGetHeaderWidget
- 位置: xui.h:6538  已注释: 否
- 签名: `XUI_API xui_widget xuiPanelGetHeaderWidget(xui_widget pWidget);`
- 实现: src/xui_panel.c:637（体 7 行）
- 用法: test_xui/xui_panel_test.c:116; test_xui/xui_panel_test.c:183

## xuiPanelGetIconWidget
- 位置: xui.h:6539  已注释: 否
- 签名: `XUI_API xui_widget xuiPanelGetIconWidget(xui_widget pWidget);`
- 实现: src/xui_panel.c:645（体 7 行）
- 用法: test_xui/xui_panel_test.c:117

## xuiPanelGetTitleWidget
- 位置: xui.h:6540  已注释: 否
- 签名: `XUI_API xui_widget xuiPanelGetTitleWidget(xui_widget pWidget);`
- 实现: src/xui_panel.c:653（体 7 行）
- 用法: examples/xui_panel/main.c:319; test_xui/xui_panel_test.c:118; test_xui/xui_style_chrome_test.c:492

## xuiPanelGetClientWidget
- 位置: xui.h:6541  已注释: 否
- 签名: `XUI_API xui_widget xuiPanelGetClientWidget(xui_widget pWidget);`
- 实现: src/xui_panel.c:661（体 7 行）
- 用法: examples/xui_panel/main.c:246; examples/xui_panel/main.c:314; examples/xui_panel/main.c:324

## xuiPanelAddChild
- 位置: xui.h:6542  已注释: 否
- 签名: `XUI_API int xuiPanelAddChild(xui_widget pWidget, xui_widget pChild);`
- 实现: src/xui_panel.c:669（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_panel/main.c:177; test_xui/xui_code_edit_test.c:1468; test_xui/xui_panel_test.c:132

## xuiPanelInsertBefore
- 位置: xui.h:6543  已注释: 否
- 签名: `XUI_API int xuiPanelInsertBefore(xui_widget pWidget, xui_widget pChild, xui_widget pBefore);`
- 实现: src/xui_panel.c:680（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPanelSetTitle
- 位置: xui.h:6544  已注释: 否
- 签名: `XUI_API int xuiPanelSetTitle(xui_widget pWidget, const char* sTitle);`
- 实现: src/xui_panel.c:691（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_chrome_test.c:488

## xuiPanelGetTitle
- 位置: xui.h:6545  已注释: 否
- 签名: `XUI_API const char* xuiPanelGetTitle(xui_widget pWidget);`
- 实现: src/xui_panel.c:707（体 7 行）
- 用法: examples/xui_panel/main.c:318; test_xui/xui_panel_test.c:121

## xuiPanelSetFont
- 位置: xui.h:6546  已注释: 否
- 签名: `XUI_API int xuiPanelSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_panel.c:715（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPanelGetFont
- 位置: xui.h:6547  已注释: 否
- 签名: `XUI_API xui_font xuiPanelGetFont(xui_widget pWidget);`
- 实现: src/xui_panel.c:728（体 7 行）

## xuiPanelSetTitleColor
- 位置: xui.h:6548  已注释: 否
- 签名: `XUI_API int xuiPanelSetTitleColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_panel.c:736（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_panel/main.c:234; test_xui/xui_style_chrome_test.c:489

## xuiPanelGetTitleColor
- 位置: xui.h:6549  已注释: 否
- 签名: `XUI_API uint32_t xuiPanelGetTitleColor(xui_widget pWidget);`
- 实现: src/xui_panel.c:749（体 7 行）
- 用法: test_xui/xui_panel_test.c:122; test_xui/xui_style_chrome_test.c:505

## xuiPanelSetDisabledTitleColor
- 位置: xui.h:6550  已注释: 否
- 签名: `XUI_API int xuiPanelSetDisabledTitleColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_panel.c:757（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_chrome_test.c:490

## xuiPanelGetDisabledTitleColor
- 位置: xui.h:6551  已注释: 否
- 签名: `XUI_API uint32_t xuiPanelGetDisabledTitleColor(xui_widget pWidget);`
- 实现: src/xui_panel.c:770（体 7 行）
- 用法: test_xui/xui_style_chrome_test.c:506

## xuiPanelSetTitleAlign
- 位置: xui.h:6552  已注释: 否
- 签名: `XUI_API int xuiPanelSetTitleAlign(xui_widget pWidget, uint32_t iTextFlags);`
- 实现: src/xui_panel.c:778（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPanelGetTitleAlign
- 位置: xui.h:6553  已注释: 否
- 签名: `XUI_API uint32_t xuiPanelGetTitleAlign(xui_widget pWidget);`
- 实现: src/xui_panel.c:791（体 7 行）

## xuiPanelSetIcon
- 位置: xui.h:6554  已注释: 否
- 签名: `XUI_API int xuiPanelSetIcon(xui_widget pWidget, xui_surface pSurface, xui_rect_t tSrc);`
- 实现: src/xui_panel.c:799（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_panel/main.c:230; test_xui/xui_panel_test.c:142

## xuiPanelGetIconSurface
- 位置: xui.h:6555  已注释: 否
- 签名: `XUI_API xui_surface xuiPanelGetIconSurface(xui_widget pWidget);`
- 实现: src/xui_panel.c:817（体 7 行）
- 用法: examples/xui_panel/main.c:320; test_xui/xui_panel_test.c:143

## xuiPanelGetIconSource
- 位置: xui.h:6556  已注释: 否
- 签名: `XUI_API xui_rect_t xuiPanelGetIconSource(xui_widget pWidget);`
- 实现: src/xui_panel.c:825（体 9 行）

## xuiPanelSetIconSize
- 位置: xui.h:6557  已注释: 否
- 签名: `XUI_API int xuiPanelSetIconSize(xui_widget pWidget, float fSize);`
- 实现: src/xui_panel.c:835（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_panel/main.c:231; test_xui/xui_panel_test.c:144

## xuiPanelGetIconSize
- 位置: xui.h:6558  已注释: 否
- 签名: `XUI_API float xuiPanelGetIconSize(xui_widget pWidget);`
- 实现: src/xui_panel.c:851（体 7 行）
- 用法: test_xui/xui_panel_test.c:145

## xuiPanelSetBackgroundColor
- 位置: xui.h:6559  已注释: 否
- 签名: `XUI_API int xuiPanelSetBackgroundColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_panel.c:859（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPanelGetBackgroundColor
- 位置: xui.h:6560  已注释: 否
- 签名: `XUI_API uint32_t xuiPanelGetBackgroundColor(xui_widget pWidget);`
- 实现: src/xui_panel.c:869（体 7 行）

## xuiPanelSetHeaderColor
- 位置: xui.h:6561  已注释: 否
- 签名: `XUI_API int xuiPanelSetHeaderColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_panel.c:877（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_panel/main.c:232; examples/xui_panel/main.c:248; test_xui/xui_panel_test.c:151

## xuiPanelGetHeaderColor
- 位置: xui.h:6562  已注释: 否
- 签名: `XUI_API uint32_t xuiPanelGetHeaderColor(xui_widget pWidget);`
- 实现: src/xui_panel.c:887（体 7 行）
- 用法: examples/xui_panel/main.c:322

## xuiPanelSetClientColor
- 位置: xui.h:6563  已注释: 否
- 签名: `XUI_API int xuiPanelSetClientColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_panel.c:895（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_panel/main.c:233; examples/xui_panel/main.c:240; examples/xui_panel/main.c:249

## xuiPanelGetClientColor
- 位置: xui.h:6564  已注释: 否
- 签名: `XUI_API uint32_t xuiPanelGetClientColor(xui_widget pWidget);`
- 实现: src/xui_panel.c:905（体 7 行）
- 用法: examples/xui_panel/main.c:323

## xuiPanelSetBorder
- 位置: xui.h:6565  已注释: 否
- 签名: `XUI_API int xuiPanelSetBorder(xui_widget pWidget, float fBorderWidth, uint32_t iBorderColor);`
- 实现: src/xui_panel.c:913（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_panel/main.c:235; examples/xui_panel/main.c:241; examples/xui_panel/main.c:250

## xuiPanelGetBorder
- 位置: xui.h:6566  已注释: 否
- 签名: `XUI_API int xuiPanelGetBorder(xui_widget pWidget, float* pBorderWidth, uint32_t* pBorderColor);`
- 实现: src/xui_panel.c:924（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_panel_test.c:157

## xuiPanelSetHeaderHeight
- 位置: xui.h:6567  已注释: 否
- 签名: `XUI_API int xuiPanelSetHeaderHeight(xui_widget pWidget, float fHeight);`
- 实现: src/xui_panel.c:935（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_panel/main.c:239; test_xui/xui_panel_test.c:177

## xuiPanelGetHeaderHeight
- 位置: xui.h:6568  已注释: 否
- 签名: `XUI_API float xuiPanelGetHeaderHeight(xui_widget pWidget);`
- 实现: src/xui_panel.c:947（体 7 行）
- 用法: test_xui/xui_panel_test.c:123

## xuiPanelSetHeaderGap
- 位置: xui.h:6569  已注释: 否
- 签名: `XUI_API int xuiPanelSetHeaderGap(xui_widget pWidget, float fGap);`
- 实现: src/xui_panel.c:955（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPanelGetHeaderGap
- 位置: xui.h:6570  已注释: 否
- 签名: `XUI_API float xuiPanelGetHeaderGap(xui_widget pWidget);`
- 实现: src/xui_panel.c:966（体 7 行）

## xuiPanelSetClientClip
- 位置: xui.h:6571  已注释: 否
- 签名: `XUI_API int xuiPanelSetClientClip(xui_widget pWidget, int bClip);`
- 实现: src/xui_panel.c:974（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_panel/main.c:247; test_xui/xui_panel_test.c:186; test_xui/xui_panel_test.c:188

## xuiPanelGetClientClip
- 位置: xui.h:6572  已注释: 否
- 签名: `XUI_API int xuiPanelGetClientClip(xui_widget pWidget);`
- 实现: src/xui_panel.c:985（体 7 行）
- 用法: examples/xui_panel/main.c:326; test_xui/xui_panel_test.c:187; test_xui/xui_panel_test.c:189

## xuiPanelGetHeaderRect
- 位置: xui.h:6573  已注释: 否
- 签名: `XUI_API xui_rect_t xuiPanelGetHeaderRect(xui_widget pWidget);`
- 实现: src/xui_panel.c:993（体 9 行）
- 用法: examples/xui_panel/main.c:315; test_xui/xui_panel_test.c:137; test_xui/xui_panel_test.c:174

## xuiPanelGetIconRect
- 位置: xui.h:6574  已注释: 否
- 签名: `XUI_API xui_rect_t xuiPanelGetIconRect(xui_widget pWidget);`
- 实现: src/xui_panel.c:1003（体 9 行）
- 用法: examples/xui_panel/main.c:321; test_xui/xui_panel_test.c:148; test_xui/xui_panel_test.c:175

## xuiPanelGetTitleRect
- 位置: xui.h:6575  已注释: 否
- 签名: `XUI_API xui_rect_t xuiPanelGetTitleRect(xui_widget pWidget);`
- 实现: src/xui_panel.c:1013（体 9 行）

## xuiPanelGetClientRect
- 位置: xui.h:6576  已注释: 否
- 签名: `XUI_API xui_rect_t xuiPanelGetClientRect(xui_widget pWidget);`
- 实现: src/xui_panel.c:1023（体 9 行）
- 用法: examples/xui_panel/main.c:316; examples/xui_panel/main.c:325; test_xui/xui_panel_test.c:138

## xuiPanelGetState
- 位置: xui.h:6577  已注释: 否
- 签名: `XUI_API uint32_t xuiPanelGetState(xui_widget pWidget);`
- 实现: src/xui_panel.c:1033（体 7 行）

