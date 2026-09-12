# 草稿包：xui.h / menu（30 条 API）

> 生成 2026-09-10 03:01 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiMenuGetType
- 位置: xui.h:9627  已注释: 否
- 签名: `XUI_API xui_widget_type xuiMenuGetType(xui_context pContext);`
- 实现: src/xui_menu.c:1208（体 34 行）
- 返回码: NULL

## xuiMenuCreate
- 位置: xui.h:9628  已注释: 否
- 签名: `XUI_API int xuiMenuCreate(xui_context pContext, xui_widget* ppWidget, const xui_menu_desc_t* pDesc);`
- 实现: src/xui_menu.c:1243（体 27 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED, XUI_OK
- 用法: examples/tutorial_capture/ch155_main1.c:13; examples/xui_menu/main.c:179; examples/xui_menu/main.c:199

## xuiMenuSetItems
- 位置: xui.h:9629  已注释: 否
- 签名: `XUI_API int xuiMenuSetItems(xui_widget pWidget, const xui_menu_item_t* pItems, int iCount);`
- 实现: src/xui_menu.c:1271（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_menu/main.c:196; examples/xui_menu/main.c:237; examples/xui_menubar/main.c:178

## xuiMenuAddItem
- 位置: xui.h:9630  已注释: 否
- 签名: `XUI_API int xuiMenuAddItem(xui_widget pWidget, const xui_menu_item_t* pItem);`
- 实现: src/xui_menu.c:1296（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch155_main1.c:21; examples/tutorial_capture/ch155_main1.c:22; examples/tutorial_capture/ch155_main1.c:23

## xuiMenuAddSeparator
- 位置: xui.h:9631  已注释: 否
- 签名: `XUI_API int xuiMenuAddSeparator(xui_widget pWidget);`
- 实现: src/xui_menu.c:1312（体 8 行）
- 用法: examples/tutorial_capture/ch155_main1.c:24

## xuiMenuClear
- 位置: xui.h:9632  已注释: 否
- 签名: `XUI_API int xuiMenuClear(xui_widget pWidget);`
- 实现: src/xui_menu.c:1321（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMenuGetItemCount
- 位置: xui.h:9633  已注释: 否
- 签名: `XUI_API int xuiMenuGetItemCount(xui_widget pWidget);`
- 实现: src/xui_menu.c:1336（体 5 行）
- 用法: examples/xui_combobox/main.c:390; examples/xui_dockpanel/main.c:396; examples/xui_dockpanel/main.c:400

## xuiMenuGetItem
- 位置: xui.h:9634  已注释: 否
- 签名: `XUI_API const xui_menu_item_t* xuiMenuGetItem(xui_widget pWidget, int iIndex);`
- 实现: src/xui_menu.c:1342（体 8 行）
- 返回码: NULL
- 用法: examples/xui_input/main.c:592; examples/xui_input/main.c:596; examples/xui_input/main.c:597

## xuiMenuGetItemRect
- 位置: xui.h:9635  已注释: 否
- 签名: `XUI_API xui_rect_t xuiMenuGetItemRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_menu.c:1351（体 12 行）
- 用法: examples/xui_combobox/main.c:344; examples/xui_menu/main.c:284; examples/xui_menu/main.c:298

## xuiMenuSetItemState
- 位置: xui.h:9636  已注释: 否
- 签名: `XUI_API int xuiMenuSetItemState(xui_widget pWidget, int iIndex, uint32_t iMask, uint32_t iState);`
- 实现: src/xui_menu.c:1364（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMenuGetItemState
- 位置: xui.h:9637  已注释: 否
- 签名: `XUI_API uint32_t xuiMenuGetItemState(xui_widget pWidget, int iIndex);`
- 实现: src/xui_menu.c:1377（体 8 行）
- 用法: examples/xui_menu/main.c:315; test_xui/xui_combobox_test.c:331; test_xui/xui_combobox_test.c:365

## xuiMenuGetHoverIndex
- 位置: xui.h:9638  已注释: 否
- 签名: `XUI_API int xuiMenuGetHoverIndex(xui_widget pWidget);`
- 实现: src/xui_menu.c:1386（体 5 行）
- 用法: test_xui/xui_menubar_test.c:248

## xuiMenuSetHoverIndex
- 位置: xui.h:9639  已注释: 否
- 签名: `XUI_API int xuiMenuSetHoverIndex(xui_widget pWidget, int iIndex);`
- 实现: src/xui_menu.c:1392（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_workflow_editor/main.c:764; test_xui/xui_dock_panel_test.c:824; test_xui/xui_dock_panel_test.c:853

## xuiMenuCommitHover
- 位置: xui.h:9640  已注释: 否
- 签名: `XUI_API int xuiMenuCommitHover(xui_widget pWidget);`
- 实现: src/xui_menu.c:1399（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_workflow_editor/main.c:765; test_xui/xui_dock_panel_test.c:826; test_xui/xui_dock_panel_test.c:855

## xuiMenuSetSelect
- 位置: xui.h:9641  已注释: 否
- 签名: `XUI_API int xuiMenuSetSelect(xui_widget pWidget, xui_menu_select_proc onSelect, void* pUser);`
- 实现: src/xui_menu.c:1406（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_menu/main.c:201; examples/xui_split_layout_dock_repro/main.c:246; examples/xui_split_layout_dock_repro/main.c:252

## xuiMenuGetSelectCount
- 位置: xui.h:9642  已注释: 否
- 签名: `XUI_API int xuiMenuGetSelectCount(xui_widget pWidget);`
- 实现: src/xui_menu.c:1415（体 5 行）

## xuiMenuSetFont
- 位置: xui.h:9643  已注释: 否
- 签名: `XUI_API int xuiMenuSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_menu.c:1421（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMenuGetFont
- 位置: xui.h:9644  已注释: 否
- 签名: `XUI_API xui_font xuiMenuGetFont(xui_widget pWidget);`
- 实现: src/xui_menu.c:1429（体 5 行）
- 用法: test_xui/xui_numeric_input_test.c:228

## xuiMenuSetMetrics
- 位置: xui.h:9645  已注释: 否
- 签名: `XUI_API int xuiMenuSetMetrics(xui_widget pWidget, const xui_menu_metrics_t* pMetrics);`
- 实现: src/xui_menu.c:1435（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMenuGetMetrics
- 位置: xui.h:9646  已注释: 否
- 签名: `XUI_API int xuiMenuGetMetrics(xui_widget pWidget, xui_menu_metrics_t* pMetrics);`
- 实现: src/xui_menu.c:1444（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_menu_test.c:307

## xuiMenuSetColors
- 位置: xui.h:9647  已注释: 否
- 签名: `XUI_API int xuiMenuSetColors(xui_widget pWidget, const xui_menu_colors_t* pColors);`
- 实现: src/xui_menu.c:1453（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMenuGetColors
- 位置: xui.h:9648  已注释: 否
- 签名: `XUI_API int xuiMenuGetColors(xui_widget pWidget, xui_menu_colors_t* pColors);`
- 实现: src/xui_menu.c:1462（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_pickers_combobox_test.c:76

## xuiMenuMeasure
- 位置: xui.h:9649  已注释: 否
- 签名: `XUI_API int xuiMenuMeasure(xui_widget pWidget, float* pWidth, float* pHeight);`
- 实现: src/xui_menu.c:1471（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_menu_test.c:232; test_xui/xui_menu_test.c:309

## xuiMenuOpenAt
- 位置: xui.h:9650  已注释: 否
- 签名: `XUI_API int xuiMenuOpenAt(xui_widget pWidget, xui_widget pOwner, float fX, float fY);`
- 实现: src/xui_menu.c:1484（体 30 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch155_main1.c:29; examples/xui_workflow_editor/main.c:595; test_xui/xui_menu_test.c:258

## xuiMenuOpenForOwner
- 位置: xui.h:9651  已注释: 否
- 签名: `XUI_API int xuiMenuOpenForOwner(xui_widget pWidget, xui_widget pOwner);`
- 实现: src/xui_menu.c:1515（体 28 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_menu/main.c:147; examples/xui_menu/main.c:311; examples/xui_menu/main.c:316

## xuiMenuClose
- 位置: xui.h:9652  已注释: 否
- 签名: `XUI_API int xuiMenuClose(xui_widget pWidget);`
- 实现: src/xui_menu.c:1544（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_combobox/main.c:391; examples/xui_dockpanel/main.c:397; examples/xui_dockpanel/main.c:401

## xuiMenuIsOpen
- 位置: xui.h:9653  已注释: 否
- 签名: `XUI_API int xuiMenuIsOpen(xui_widget pWidget);`
- 实现: src/xui_menu.c:1566（体 5 行）
- 用法: examples/xui_combobox/main.c:389; examples/xui_dockpanel/main.c:396; examples/xui_dockpanel/main.c:400

## xuiMenuGetPopupWidget
- 位置: xui.h:9654  已注释: 否
- 签名: `XUI_API xui_widget xuiMenuGetPopupWidget(xui_widget pWidget);`
- 实现: src/xui_menu.c:1572（体 5 行）
- 用法: examples/xui_menu/main.c:309; examples/xui_menu/main.c:327; test_xui/xui_menu_test.c:234

## xuiMenuGetContentWidget
- 位置: xui.h:9655  已注释: 否
- 签名: `XUI_API xui_widget xuiMenuGetContentWidget(xui_widget pWidget);`
- 实现: src/xui_menu.c:1578（体 4 行）
- 用法: examples/xui_menu/main.c:309; test_xui/xui_menu_test.c:236

## xuiMenuGetOwner
- 位置: xui.h:9656  已注释: 否
- 签名: `XUI_API xui_widget xuiMenuGetOwner(xui_widget pWidget);`
- 实现: src/xui_menu.c:1583（体 8 行）
- 返回码: NULL
- 用法: test_xui/xui_menu_test.c:236

