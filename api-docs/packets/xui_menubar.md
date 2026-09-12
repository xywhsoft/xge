# 草稿包：xui.h / menubar（29 条 API）

> 生成 2026-09-10 03:01 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiMenuBarGetType
- 位置: xui.h:9658  已注释: 否
- 签名: `XUI_API xui_widget_type xuiMenuBarGetType(xui_context pContext);`
- 实现: src/xui_menubar.c:905（体 27 行）
- 返回码: NULL

## xuiMenuBarCreate
- 位置: xui.h:9659  已注释: 否
- 签名: `XUI_API int xuiMenuBarCreate(xui_context pContext, xui_widget* ppWidget, const xui_menubar_desc_t* pDesc);`
- 实现: src/xui_menubar.c:933（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch156_main1.c:13; examples/tutorial_capture/ch202_main1.c:16; examples/tutorial_capture/ch204_main1.c:16

## xuiMenuBarSetItems
- 位置: xui.h:9660  已注释: 否
- 签名: `XUI_API int xuiMenuBarSetItems(xui_widget pWidget, const xui_menubar_item_t* pItems, int iCount);`
- 实现: src/xui_menubar.c:944（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_menubar/main.c:232; examples/xui_split_layout_dock_repro/main.c:258; test_xui/xui_menubar_pixel_test.c:117

## xuiMenuBarAddItem
- 位置: xui.h:9661  已注释: 否
- 签名: `XUI_API int xuiMenuBarAddItem(xui_widget pWidget, const char* sText, xui_widget pMenu, int iValue);`
- 实现: src/xui_menubar.c:969（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch156_main1.c:19; examples/tutorial_capture/ch156_main1.c:20; examples/tutorial_capture/ch156_main1.c:21

## xuiMenuBarClear
- 位置: xui.h:9662  已注释: 否
- 签名: `XUI_API int xuiMenuBarClear(xui_widget pWidget);`
- 实现: src/xui_menubar.c:989（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_menubar_pixel_test.c:165

## xuiMenuBarGetItemCount
- 位置: xui.h:9663  已注释: 否
- 签名: `XUI_API int xuiMenuBarGetItemCount(xui_widget pWidget);`
- 实现: src/xui_menubar.c:1004（体 5 行）
- 用法: examples/xui_menubar/main.c:284; test_xui/xui_menubar_pixel_test.c:166; test_xui/xui_menubar_test.c:212

## xuiMenuBarGetItem
- 位置: xui.h:9664  已注释: 否
- 签名: `XUI_API const xui_menubar_item_t* xuiMenuBarGetItem(xui_widget pWidget, int iIndex);`
- 实现: src/xui_menubar.c:1010（体 6 行）
- 返回码: NULL

## xuiMenuBarGetItemRect
- 位置: xui.h:9665  已注释: 否
- 签名: `XUI_API xui_rect_t xuiMenuBarGetItemRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_menubar.c:1017（体 12 行）
- 用法: examples/xui_menubar/main.c:269; examples/xui_menubar/main.c:276; examples/xui_menubar/main.c:285

## xuiMenuBarSetItemMenu
- 位置: xui.h:9666  已注释: 否
- 签名: `XUI_API int xuiMenuBarSetItemMenu(xui_widget pWidget, int iIndex, xui_widget pMenu);`
- 实现: src/xui_menubar.c:1030（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMenuBarSetItemEnabled
- 位置: xui.h:9667  已注释: 否
- 签名: `XUI_API int xuiMenuBarSetItemEnabled(xui_widget pWidget, int iIndex, int bEnabled);`
- 实现: src/xui_menubar.c:1039（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_menubar_pixel_test.c:163; test_xui/xui_menubar_test.c:258

## xuiMenuBarIsItemEnabled
- 位置: xui.h:9668  已注释: 否
- 签名: `XUI_API int xuiMenuBarIsItemEnabled(xui_widget pWidget, int iIndex);`
- 实现: src/xui_menubar.c:1055（体 6 行）
- 用法: test_xui/xui_menubar_pixel_test.c:164; test_xui/xui_menubar_test.c:213; test_xui/xui_menubar_test.c:259

## xuiMenuBarSetItemMnemonic
- 位置: xui.h:9669  已注释: 否
- 签名: `XUI_API int xuiMenuBarSetItemMnemonic(xui_widget pWidget, int iIndex, int iMnemonic);`
- 实现: src/xui_menubar.c:1062（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_menubar_test.c:260

## xuiMenuBarGetHoverIndex
- 位置: xui.h:9670  已注释: 否
- 签名: `XUI_API int xuiMenuBarGetHoverIndex(xui_widget pWidget);`
- 实现: src/xui_menubar.c:1073（体 5 行）
- 用法: test_xui/xui_menubar_pixel_test.c:130; test_xui/xui_menubar_pixel_test.c:140; test_xui/xui_menubar_test.c:257

## xuiMenuBarSetHoverIndex
- 位置: xui.h:9671  已注释: 否
- 签名: `XUI_API int xuiMenuBarSetHoverIndex(xui_widget pWidget, int iIndex);`
- 实现: src/xui_menubar.c:1079（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMenuBarGetActiveIndex
- 位置: xui.h:9672  已注释: 否
- 签名: `XUI_API int xuiMenuBarGetActiveIndex(xui_widget pWidget);`
- 实现: src/xui_menubar.c:1086（体 5 行）

## xuiMenuBarGetOpenIndex
- 位置: xui.h:9673  已注释: 否
- 签名: `XUI_API int xuiMenuBarGetOpenIndex(xui_widget pWidget);`
- 实现: src/xui_menubar.c:1092（体 7 行）
- 用法: examples/xui_menubar/main.c:288; examples/xui_menubar/main.c:290; examples/xui_menubar/main.c:293

## xuiMenuBarOpenItem
- 位置: xui.h:9674  已注释: 否
- 签名: `XUI_API int xuiMenuBarOpenItem(xui_widget pWidget, int iIndex);`
- 实现: src/xui_menubar.c:1100（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMenuBarClose
- 位置: xui.h:9675  已注释: 否
- 签名: `XUI_API int xuiMenuBarClose(xui_widget pWidget);`
- 实现: src/xui_menubar.c:1107（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMenuBarIsOpen
- 位置: xui.h:9676  已注释: 否
- 签名: `XUI_API int xuiMenuBarIsOpen(xui_widget pWidget);`
- 实现: src/xui_menubar.c:1114（体 4 行）
- 用法: examples/xui_menubar/main.c:288; examples/xui_menubar/main.c:303; test_xui/xui_menubar_test.c:228

## xuiMenuBarGetOpenMenu
- 位置: xui.h:9677  已注释: 否
- 签名: `XUI_API xui_widget xuiMenuBarGetOpenMenu(xui_widget pWidget);`
- 实现: src/xui_menubar.c:1119（体 7 行）
- 返回码: NULL

## xuiMenuBarSetSelect
- 位置: xui.h:9678  已注释: 否
- 签名: `XUI_API int xuiMenuBarSetSelect(xui_widget pWidget, xui_menu_select_proc onSelect, void* pUser);`
- 实现: src/xui_menubar.c:1127（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_menubar/main.c:224; examples/xui_split_layout_dock_repro/main.c:259; test_xui/xui_menubar_pixel_test.c:118

## xuiMenuBarSetFont
- 位置: xui.h:9679  已注释: 否
- 签名: `XUI_API int xuiMenuBarSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_menubar.c:1136（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_menubar_pixel_test.c:113; test_xui/xui_menubar_pixel_test.c:158

## xuiMenuBarGetFont
- 位置: xui.h:9680  已注释: 否
- 签名: `XUI_API xui_font xuiMenuBarGetFont(xui_widget pWidget);`
- 实现: src/xui_menubar.c:1144（体 5 行）

## xuiMenuBarSetMetrics
- 位置: xui.h:9681  已注释: 否
- 签名: `XUI_API int xuiMenuBarSetMetrics(xui_widget pWidget, const xui_menubar_metrics_t* pMetrics);`
- 实现: src/xui_menubar.c:1150（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_menubar_pixel_test.c:123; test_xui/xui_menubar_pixel_test.c:153

## xuiMenuBarGetMetrics
- 位置: xui.h:9682  已注释: 否
- 签名: `XUI_API int xuiMenuBarGetMetrics(xui_widget pWidget, xui_menubar_metrics_t* pMetrics);`
- 实现: src/xui_menubar.c:1159（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_menubar_pixel_test.c:120; test_xui/xui_menubar_pixel_test.c:155; test_xui/xui_menubar_test.c:264

## xuiMenuBarSetColors
- 位置: xui.h:9683  已注释: 否
- 签名: `XUI_API int xuiMenuBarSetColors(xui_widget pWidget, const xui_menubar_colors_t* pColors);`
- 实现: src/xui_menubar.c:1168（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMenuBarGetColors
- 位置: xui.h:9684  已注释: 否
- 签名: `XUI_API int xuiMenuBarGetColors(xui_widget pWidget, xui_menubar_colors_t* pColors);`
- 实现: src/xui_menubar.c:1177（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_menubar_test.c:266

## xuiMenuBarGetState
- 位置: xui.h:9685  已注释: 否
- 签名: `XUI_API int xuiMenuBarGetState(xui_widget pWidget);`
- 实现: src/xui_menubar.c:1186（体 10 行）

## xuiMenuBarGetChangeCount
- 位置: xui.h:9686  已注释: 否
- 签名: `XUI_API int xuiMenuBarGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_menubar.c:1197（体 5 行）
- 用法: test_xui/xui_menubar_test.c:268

