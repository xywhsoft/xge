# 草稿包：xui.h / toolbar（43 条 API）

> 生成 2026-09-10 03:08 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiToolbarGetType
- 位置: xui.h:10783  已注释: 否
- 签名: `XUI_API xui_widget_type xuiToolbarGetType(xui_context pContext);`
- 实现: src/xui_toolbar.c:1044（体 26 行）
- 返回码: NULL

## xuiToolbarCreate
- 位置: xui.h:10784  已注释: 否
- 签名: `XUI_API int xuiToolbarCreate(xui_context pContext, xui_widget* ppWidget, const xui_toolbar_desc_t* pDesc);`
- 实现: src/xui_toolbar.c:1071（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch157_main1.c:13; examples/tutorial_capture/ch203_main1.c:16; examples/tutorial_capture/ch207_main1.c:16

## xuiToolbarSetSelect
- 位置: xui.h:10785  已注释: 否
- 签名: `XUI_API int xuiToolbarSetSelect(xui_widget pWidget, xui_toolbar_select_proc onSelect, void* pUser);`
- 实现: src/xui_toolbar.c:1082（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_richedit/main.c:160; examples/xui_split_layout_dock_repro/main.c:298; examples/xui_toolbar/main.c:231

## xuiToolbarSetContextMenu
- 位置: xui.h:10786  已注释: 否
- 签名: `XUI_API int xuiToolbarSetContextMenu(xui_widget pWidget, xui_toolbar_context_proc onContext, void* pUser);`
- 实现: src/xui_toolbar.c:1091（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiToolbarSetOverflow
- 位置: xui.h:10787  已注释: 否
- 签名: `XUI_API int xuiToolbarSetOverflow(xui_widget pWidget, int bEnabled, float fButtonSize, xui_toolbar_overflow_proc onOverflow, void* pUser);`
- 实现: src/xui_toolbar.c:1100（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_richedit/main.c:157; examples/xui_toolbar/main.c:233; test_xui/xui_toolbar_pixel_test.c:203

## xuiToolbarIsOverflowEnabled
- 位置: xui.h:10788  已注释: 否
- 签名: `XUI_API int xuiToolbarIsOverflowEnabled(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1118（体 5 行）

## xuiToolbarSetItems
- 位置: xui.h:10789  已注释: 否
- 签名: `XUI_API int xuiToolbarSetItems(xui_widget pWidget, const xui_toolbar_item_t* pItems, int iCount);`
- 实现: src/xui_toolbar.c:1124（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_chrome_test.c:216; test_xui/xui_toolbar_pixel_test.c:189

## xuiToolbarAddItem
- 位置: xui.h:10790  已注释: 否
- 签名: `XUI_API int xuiToolbarAddItem(xui_widget pWidget, const char* sText, int iType, int iValue);`
- 实现: src/xui_toolbar.c:1151（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch157_main1.c:19; examples/tutorial_capture/ch157_main1.c:20; examples/tutorial_capture/ch157_main1.c:21

## xuiToolbarAddSeparator
- 位置: xui.h:10791  已注释: 否
- 签名: `XUI_API int xuiToolbarAddSeparator(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1169（体 4 行）
- 用法: examples/tutorial_capture/ch157_main1.c:22; examples/tutorial_capture/ch203_main1.c:23; examples/tutorial_capture/ch207_main1.c:22

## xuiToolbarClear
- 位置: xui.h:10792  已注释: 否
- 签名: `XUI_API int xuiToolbarClear(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1174（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiToolbarGetItemCount
- 位置: xui.h:10793  已注释: 否
- 签名: `XUI_API int xuiToolbarGetItemCount(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1189（体 5 行）
- 用法: examples/xui_toolbar/main.c:315; test_xui/xui_rich_edit_test.c:385; test_xui/xui_rich_edit_test.c:386

## xuiToolbarGetItem
- 位置: xui.h:10794  已注释: 否
- 签名: `XUI_API const xui_toolbar_item_t* xuiToolbarGetItem(xui_widget pWidget, int iIndex);`
- 实现: src/xui_toolbar.c:1195（体 6 行）
- 返回码: NULL
- 用法: test_xui/xui_rich_edit_test.c:387; test_xui/xui_toolbar_pixel_test.c:82

## xuiToolbarGetItemRect
- 位置: xui.h:10795  已注释: 否
- 签名: `XUI_API xui_rect_t xuiToolbarGetItemRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_toolbar.c:1202（体 12 行）
- 用法: examples/xui_toolbar/main.c:298; examples/xui_toolbar/main.c:316; examples/xui_toolbar/main.c:316

## xuiToolbarGetItemAt
- 位置: xui.h:10796  已注释: 否
- 签名: `XUI_API int xuiToolbarGetItemAt(xui_widget pWidget, float fX, float fY);`
- 实现: src/xui_toolbar.c:1215（体 9 行）
- 用法: test_xui/xui_toolbar_pixel_test.c:99; test_xui/xui_toolbar_pixel_test.c:100; test_xui/xui_toolbar_pixel_test.c:208

## xuiToolbarSetOrientation
- 位置: xui.h:10797  已注释: 否
- 签名: `XUI_API int xuiToolbarSetOrientation(xui_widget pWidget, int iOrientation);`
- 实现: src/xui_toolbar.c:1225（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_toolbar_pixel_test.c:254; test_xui/xui_toolbar_test.c:238; test_xui/xui_toolbar_test.c:245

## xuiToolbarGetOrientation
- 位置: xui.h:10798  已注释: 否
- 签名: `XUI_API int xuiToolbarGetOrientation(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1234（体 5 行）

## xuiToolbarSetItemSize
- 位置: xui.h:10799  已注释: 否
- 签名: `XUI_API int xuiToolbarSetItemSize(xui_widget pWidget, float fWidth, float fHeight, float fSeparatorSize);`
- 实现: src/xui_toolbar.c:1240（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_richedit/main.c:156; test_xui/xui_toolbar_pixel_test.c:250

## xuiToolbarSetItemGroup
- 位置: xui.h:10800  已注释: 否
- 签名: `XUI_API int xuiToolbarSetItemGroup(xui_widget pWidget, int iIndex, int iGroup);`
- 实现: src/xui_toolbar.c:1254（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_toolbar_pixel_test.c:258

## xuiToolbarGetItemGroup
- 位置: xui.h:10801  已注释: 否
- 签名: `XUI_API int xuiToolbarGetItemGroup(xui_widget pWidget, int iIndex);`
- 实现: src/xui_toolbar.c:1263（体 6 行）

## xuiToolbarSetItemTooltip
- 位置: xui.h:10802  已注释: 否
- 签名: `XUI_API int xuiToolbarSetItemTooltip(xui_widget pWidget, int iIndex, const char* sText);`
- 实现: src/xui_toolbar.c:1270（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiToolbarGetItemTooltip
- 位置: xui.h:10803  已注释: 否
- 签名: `XUI_API const char* xuiToolbarGetItemTooltip(xui_widget pWidget, int iIndex);`
- 实现: src/xui_toolbar.c:1281（体 6 行）
- 用法: test_xui/xui_rich_edit_test.c:410; test_xui/xui_toolbar_test.c:222

## xuiToolbarGetHoverTooltip
- 位置: xui.h:10804  已注释: 否
- 签名: `XUI_API const char* xuiToolbarGetHoverTooltip(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1288（体 6 行）
- 用法: test_xui/xui_toolbar_test.c:232

## xuiToolbarSetItemEnabled
- 位置: xui.h:10805  已注释: 否
- 签名: `XUI_API int xuiToolbarSetItemEnabled(xui_widget pWidget, int iIndex, int bEnabled);`
- 实现: src/xui_toolbar.c:1295（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiToolbarIsItemEnabled
- 位置: xui.h:10806  已注释: 否
- 签名: `XUI_API int xuiToolbarIsItemEnabled(xui_widget pWidget, int iIndex);`
- 实现: src/xui_toolbar.c:1313（体 6 行）
- 用法: test_xui/xui_rich_edit_test.c:405; test_xui/xui_rich_edit_test.c:408

## xuiToolbarSetItemChecked
- 位置: xui.h:10807  已注释: 否
- 签名: `XUI_API int xuiToolbarSetItemChecked(xui_widget pWidget, int iIndex, int bChecked);`
- 实现: src/xui_toolbar.c:1320（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiToolbarGetItemChecked
- 位置: xui.h:10808  已注释: 否
- 签名: `XUI_API int xuiToolbarGetItemChecked(xui_widget pWidget, int iIndex);`
- 实现: src/xui_toolbar.c:1331（体 6 行）
- 用法: examples/xui_toolbar/main.c:323; test_xui/xui_rich_edit_test.c:390; test_xui/xui_rich_edit_test.c:394

## xuiToolbarSetItemIcon
- 位置: xui.h:10809  已注释: 否
- 签名: `XUI_API int xuiToolbarSetItemIcon(xui_widget pWidget, int iIndex, xui_surface pIcon, xui_rect_t tSrc);`
- 实现: src/xui_toolbar.c:1338（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiToolbarGetHoverIndex
- 位置: xui.h:10810  已注释: 否
- 签名: `XUI_API int xuiToolbarGetHoverIndex(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1348（体 5 行）
- 用法: test_xui/xui_toolbar_test.c:236

## xuiToolbarSetHoverIndex
- 位置: xui.h:10811  已注释: 否
- 签名: `XUI_API int xuiToolbarSetHoverIndex(xui_widget pWidget, int iIndex);`
- 实现: src/xui_toolbar.c:1354（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_toolbar/main.c:328; test_xui/xui_toolbar_test.c:231

## xuiToolbarGetActiveIndex
- 位置: xui.h:10812  已注释: 否
- 签名: `XUI_API int xuiToolbarGetActiveIndex(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1361（体 5 行）
- 用法: test_xui/xui_toolbar_pixel_test.c:214

## xuiToolbarGetOverflowFirst
- 位置: xui.h:10813  已注释: 否
- 签名: `XUI_API int xuiToolbarGetOverflowFirst(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1367（体 9 行）
- 用法: test_xui/xui_toolbar_pixel_test.c:105; test_xui/xui_toolbar_pixel_test.c:231; test_xui/xui_toolbar_test.c:252

## xuiToolbarGetOverflowCount
- 位置: xui.h:10814  已注释: 否
- 签名: `XUI_API int xuiToolbarGetOverflowCount(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1377（体 9 行）
- 用法: examples/xui_toolbar/main.c:316; test_xui/xui_toolbar_pixel_test.c:106; test_xui/xui_toolbar_pixel_test.c:139

## xuiToolbarGetOverflowRect
- 位置: xui.h:10815  已注释: 否
- 签名: `XUI_API xui_rect_t xuiToolbarGetOverflowRect(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1387（体 11 行）
- 用法: examples/xui_toolbar/main.c:305; test_xui/xui_toolbar_pixel_test.c:110; test_xui/xui_toolbar_pixel_test.c:140

## xuiToolbarSetFont
- 位置: xui.h:10816  已注释: 否
- 签名: `XUI_API int xuiToolbarSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_toolbar.c:1399（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_toolbar_pixel_test.c:182

## xuiToolbarGetFont
- 位置: xui.h:10817  已注释: 否
- 签名: `XUI_API xui_font xuiToolbarGetFont(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1407（体 5 行）

## xuiToolbarSetMetrics
- 位置: xui.h:10818  已注释: 否
- 签名: `XUI_API int xuiToolbarSetMetrics(xui_widget pWidget, const xui_toolbar_metrics_t* pMetrics);`
- 实现: src/xui_toolbar.c:1413（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_toolbar_pixel_test.c:197; test_xui/xui_toolbar_pixel_test.c:245

## xuiToolbarGetMetrics
- 位置: xui.h:10819  已注释: 否
- 签名: `XUI_API int xuiToolbarGetMetrics(xui_widget pWidget, xui_toolbar_metrics_t* pMetrics);`
- 实现: src/xui_toolbar.c:1423（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_toolbar_pixel_test.c:192; test_xui/xui_toolbar_pixel_test.c:247; test_xui/xui_toolbar_test.c:262

## xuiToolbarSetColors
- 位置: xui.h:10820  已注释: 否
- 签名: `XUI_API int xuiToolbarSetColors(xui_widget pWidget, const xui_toolbar_colors_t* pColors);`
- 实现: src/xui_toolbar.c:1432（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_toolbar/main.c:258; test_xui/xui_toolbar_pixel_test.c:202

## xuiToolbarGetColors
- 位置: xui.h:10821  已注释: 否
- 签名: `XUI_API int xuiToolbarGetColors(xui_widget pWidget, xui_toolbar_colors_t* pColors);`
- 实现: src/xui_toolbar.c:1442（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_toolbar/main.c:251; test_xui/xui_toolbar_pixel_test.c:199; test_xui/xui_toolbar_test.c:264

## xuiToolbarGetState
- 位置: xui.h:10822  已注释: 否
- 签名: `XUI_API uint32_t xuiToolbarGetState(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1451（体 9 行）
- 返回码: XUI_WIDGET_STATE_DISABLED
- 用法: test_xui/xui_toolbar_test.c:258

## xuiToolbarGetSelectCount
- 位置: xui.h:10823  已注释: 否
- 签名: `XUI_API int xuiToolbarGetSelectCount(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1461（体 5 行）
- 用法: test_xui/xui_toolbar_test.c:266

## xuiToolbarGetOverflowSelectCount
- 位置: xui.h:10824  已注释: 否
- 签名: `XUI_API int xuiToolbarGetOverflowSelectCount(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1467（体 5 行）
- 用法: test_xui/xui_toolbar_test.c:267

## xuiToolbarGetChangeCount
- 位置: xui.h:10825  已注释: 否
- 签名: `XUI_API int xuiToolbarGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_toolbar.c:1473（体 5 行）
- 用法: test_xui/xui_toolbar_test.c:268

