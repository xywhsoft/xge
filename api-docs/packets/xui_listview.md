# 草稿包：xui.h / listview（45 条 API）

> 生成 2026-09-10 03:01 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiListViewGetType
- 位置: xui.h:9189  已注释: 否
- 签名: `XUI_API xui_widget_type xuiListViewGetType(xui_context pContext);`
- 实现: src/xui_list_view.c:1428（体 39 行）
- 返回码: NULL

## xuiListViewCreate
- 位置: xui.h:9190  已注释: 否
- 签名: `XUI_API int xuiListViewCreate(xui_context pContext, xui_widget* ppWidget, const xui_list_view_desc_t* pDesc);`
- 实现: src/xui_list_view.c:1468（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch146_main1.c:13; examples/tutorial_capture/ch207_main1.c:39; examples/xui_listview/main.c:190

## xuiListViewSetSelect
- 位置: xui.h:9191  已注释: 否
- 签名: `XUI_API int xuiListViewSetSelect(xui_widget pWidget, xui_list_view_select_proc onSelect, void* pUser);`
- 实现: src/xui_list_view.c:1483（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_listview/main.c:192; test_xui/xui_list_view_test.c:172

## xuiListViewSetContextMenu
- 位置: xui.h:9192  已注释: 否
- 签名: `XUI_API int xuiListViewSetContextMenu(xui_widget pWidget, xui_list_view_context_proc onContext, void* pUser);`
- 实现: src/xui_list_view.c:1492（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_list_view_test.c:176

## xuiListViewSetItemRenderer
- 位置: xui.h:9193  已注释: 否
- 签名: `XUI_API int xuiListViewSetItemRenderer(xui_widget pWidget, xui_list_view_item_proc onRender, void* pUser);`
- 实现: src/xui_list_view.c:1501（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_list_view_test.c:178

## xuiListViewSetItems
- 位置: xui.h:9194  已注释: 否
- 签名: `XUI_API int xuiListViewSetItems(xui_widget pWidget, const char** arrItems, int iCount);`
- 实现: src/xui_list_view.c:1510（体 67 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/tutorial_capture/ch146_main1.c:25

## xuiListViewSetEnabledItems
- 位置: xui.h:9195  已注释: 否
- 签名: `XUI_API int xuiListViewSetEnabledItems(xui_widget pWidget, const int* arrEnabled, int iCount);`
- 实现: src/xui_list_view.c:1578（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiListViewGetItemCount
- 位置: xui.h:9196  已注释: 否
- 签名: `XUI_API int xuiListViewGetItemCount(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1603（体 5 行）
- 用法: test_xui/xui_list_view_test.c:200

## xuiListViewGetItemText
- 位置: xui.h:9197  已注释: 否
- 签名: `XUI_API const char* xuiListViewGetItemText(xui_widget pWidget, int iIndex);`
- 实现: src/xui_list_view.c:1609（体 6 行）
- 返回码: NULL

## xuiListViewIsItemEnabled
- 位置: xui.h:9198  已注释: 否
- 签名: `XUI_API int xuiListViewIsItemEnabled(xui_widget pWidget, int iIndex);`
- 实现: src/xui_list_view.c:1616（体 5 行）
- 用法: test_xui/xui_list_view_test.c:202

## xuiListViewSetFont
- 位置: xui.h:9199  已注释: 否
- 签名: `XUI_API int xuiListViewSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_list_view.c:1622（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiListViewGetFont
- 位置: xui.h:9200  已注释: 否
- 签名: `XUI_API xui_font xuiListViewGetFont(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1630（体 5 行）

## xuiListViewSetItemHeight
- 位置: xui.h:9201  已注释: 否
- 签名: `XUI_API int xuiListViewSetItemHeight(xui_widget pWidget, float fHeight);`
- 实现: src/xui_list_view.c:1636（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiListViewGetItemHeight
- 位置: xui.h:9202  已注释: 否
- 签名: `XUI_API float xuiListViewGetItemHeight(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1647（体 5 行）

## xuiListViewSetMetrics
- 位置: xui.h:9203  已注释: 否
- 签名: `XUI_API int xuiListViewSetMetrics(xui_widget pWidget, float fItemHeight, float fPadding, float fBorderWidth);`
- 实现: src/xui_list_view.c:1653（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiListViewGetMetrics
- 位置: xui.h:9204  已注释: 否
- 签名: `XUI_API int xuiListViewGetMetrics(xui_widget pWidget, float* pItemHeight, float* pPadding, float* pBorderWidth);`
- 实现: src/xui_list_view.c:1666（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiListViewSetSelected
- 位置: xui.h:9205  已注释: 否
- 签名: `XUI_API int xuiListViewSetSelected(xui_widget pWidget, int iIndex);`
- 实现: src/xui_list_view.c:1676（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_listview/main.c:281; test_xui/xui_list_view_test.c:260

## xuiListViewGetSelected
- 位置: xui.h:9206  已注释: 否
- 签名: `XUI_API int xuiListViewGetSelected(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1683（体 5 行）
- 用法: examples/xui_listview/main.c:279; test_xui/xui_list_view_test.c:201; test_xui/xui_list_view_test.c:212

## xuiListViewSetSelectionMode
- 位置: xui.h:9207  已注释: 否
- 签名: `XUI_API int xuiListViewSetSelectionMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_list_view.c:1689（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_list_view_test.c:256

## xuiListViewGetSelectionMode
- 位置: xui.h:9208  已注释: 否
- 签名: `XUI_API int xuiListViewGetSelectionMode(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1707（体 5 行）

## xuiListViewSetSelectionBuffer
- 位置: xui.h:9209  已注释: 否
- 签名: `XUI_API int xuiListViewSetSelectionBuffer(xui_widget pWidget, int* arrSelected, int iCount);`
- 实现: src/xui_list_view.c:1713（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_list_view_test.c:258

## xuiListViewClearSelection
- 位置: xui.h:9210  已注释: 否
- 签名: `XUI_API int xuiListViewClearSelection(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1734（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiListViewSetItemSelected
- 位置: xui.h:9211  已注释: 否
- 签名: `XUI_API int xuiListViewSetItemSelected(xui_widget pWidget, int iIndex, int bSelected);`
- 实现: src/xui_list_view.c:1746（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiListViewIsItemSelected
- 位置: xui.h:9212  已注释: 否
- 签名: `XUI_API int xuiListViewIsItemSelected(xui_widget pWidget, int iIndex);`
- 实现: src/xui_list_view.c:1762（体 5 行）

## xuiListViewSetNotifyRepeatSelect
- 位置: xui.h:9213  已注释: 否
- 签名: `XUI_API int xuiListViewSetNotifyRepeatSelect(xui_widget pWidget, int bNotify);`
- 实现: src/xui_list_view.c:1768（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiListViewGetNotifyRepeatSelect
- 位置: xui.h:9214  已注释: 否
- 签名: `XUI_API int xuiListViewGetNotifyRepeatSelect(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1776（体 5 行）

## xuiListViewSetScroll
- 位置: xui.h:9215  已注释: 否
- 签名: `XUI_API int xuiListViewSetScroll(xui_widget pWidget, float fOffsetY);`
- 实现: src/xui_list_view.c:1782（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_list_view_test.c:266

## xuiListViewGetScroll
- 位置: xui.h:9216  已注释: 否
- 签名: `XUI_API float xuiListViewGetScroll(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1789（体 10 行）
- 用法: examples/xui_listview/main.c:286; test_xui/xui_list_view_test.c:251; test_xui/xui_list_view_test.c:267

## xuiListViewEnsureVisible
- 位置: xui.h:9217  已注释: 否
- 签名: `XUI_API int xuiListViewEnsureVisible(xui_widget pWidget, int iIndex);`
- 实现: src/xui_list_view.c:1800（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_listview/main.c:285; test_xui/xui_list_view_test.c:249

## xuiListViewSetScrollbarMode
- 位置: xui.h:9218  已注释: 否
- 签名: `XUI_API int xuiListViewSetScrollbarMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_list_view.c:1813（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiListViewGetScrollbarMode
- 位置: xui.h:9219  已注释: 否
- 签名: `XUI_API int xuiListViewGetScrollbarMode(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1820（体 5 行）

## xuiListViewSetColors
- 位置: xui.h:9220  已注释: 否
- 签名: `XUI_API int xuiListViewSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iBorder, uint32_t iFocus, uint32_t iRow, uint32_t iHover, uint32_t iSelected, uint32_t iText, uint32_t iDisabledText);`
- 实现: src/xui_list_view.c:1826（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_listview/main.c:194; test_xui/xui_list_view_test.c:180

## xuiListViewGetColors
- 位置: xui.h:9221  已注释: 否
- 签名: `XUI_API int xuiListViewGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pBorder, uint32_t* pFocus, uint32_t* pRow, uint32_t* pHover, uint32_t* pSelected, uint32_t* pText, uint32_t* pDisabledText);`
- 实现: src/xui_list_view.c:1841（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiListViewSetScrollbarColors
- 位置: xui.h:9222  已注释: 否
- 签名: `XUI_API int xuiListViewSetScrollbarColors(xui_widget pWidget, uint32_t iTrack, uint32_t iThumb, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);`
- 实现: src/xui_list_view.c:1856（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_listview/main.c:203; test_xui/xui_style_collections_list_test.c:27

## xuiListViewGetScrollbarColors
- 位置: xui.h:9223  已注释: 否
- 签名: `XUI_API int xuiListViewGetScrollbarColors(xui_widget pWidget, uint32_t* pTrack, uint32_t* pThumb, uint32_t* pHover, uint32_t* pActive, uint32_t* pFocus, uint32_t* pDisabled);`
- 实现: src/xui_list_view.c:1874（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiListViewGetFrameWidget
- 位置: xui.h:9224  已注释: 否
- 签名: `XUI_API xui_widget xuiListViewGetFrameWidget(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1887（体 5 行）
- 用法: examples/xui_listview/main.c:274; examples/xui_listview/main.c:275; examples/xui_listview/main.c:276

## xuiListViewGetViewportWidget
- 位置: xui.h:9225  已注释: 否
- 签名: `XUI_API xui_widget xuiListViewGetViewportWidget(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1893（体 5 行）
- 用法: test_xui/xui_list_view_test.c:204

## xuiListViewGetModel
- 位置: xui.h:9226  已注释: 否
- 签名: `XUI_API xui_scroll_model_t* xuiListViewGetModel(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1899（体 5 行）

## xuiListViewGetViewportRect
- 位置: xui.h:9227  已注释: 否
- 签名: `XUI_API xui_rect_t xuiListViewGetViewportRect(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1905（体 5 行）
- 用法: test_xui/xui_list_view_test.c:254; test_xui/xui_list_view_test.c:254

## xuiListViewGetItemRect
- 位置: xui.h:9228  已注释: 否
- 签名: `XUI_API xui_rect_t xuiListViewGetItemRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_list_view.c:1911（体 15 行）
- 用法: examples/xui_listview/main.c:262; test_xui/xui_file_dialog_test.c:81; test_xui/xui_list_view_test.c:209

## xuiListViewGetItemAt
- 位置: xui.h:9229  已注释: 否
- 签名: `XUI_API int xuiListViewGetItemAt(xui_widget pWidget, float fX, float fY);`
- 实现: src/xui_list_view.c:1927（体 18 行）

## xuiListViewGetHoverIndex
- 位置: xui.h:9230  已注释: 否
- 签名: `XUI_API int xuiListViewGetHoverIndex(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1946（体 5 行）

## xuiListViewGetFocusIndex
- 位置: xui.h:9231  已注释: 否
- 签名: `XUI_API int xuiListViewGetFocusIndex(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1952（体 5 行）

## xuiListViewGetSelectCount
- 位置: xui.h:9232  已注释: 否
- 签名: `XUI_API int xuiListViewGetSelectCount(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1958（体 5 行）

## xuiListViewGetChangeCount
- 位置: xui.h:9233  已注释: 否
- 签名: `XUI_API int xuiListViewGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_list_view.c:1964（体 5 行）

