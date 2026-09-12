# 草稿包：xui.h / popup（55 条 API）

> 生成 2026-09-10 03:03 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiPopupGetType
- 位置: xui.h:9780  已注释: 否
- 签名: `XUI_API xui_widget_type xuiPopupGetType(xui_context pContext);`
- 实现: src/xui_popup.c:1208（体 31 行）
- 返回码: NULL
- 用法: test_xui/xui_grid_focus_test.c:257; test_xui/xui_property_grid_test.c:98; test_xui/xui_table_grid_test.c:314

## xuiPopupCreate
- 位置: xui.h:9781  已注释: 否
- 签名: `XUI_API int xuiPopupCreate(xui_context pContext, xui_widget* ppWidget, const xui_popup_desc_t* pDesc);`
- 实现: src/xui_popup.c:1240（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch154_main1.c:26; examples/xui_popup/main.c:268; test_xui/xui_accessibility_test.c:271

## xuiPopupSetChange
- 位置: xui.h:9782  已注释: 否
- 签名: `XUI_API int xuiPopupSetChange(xui_widget pWidget, xui_popup_change_proc onChange, void* pUser);`
- 实现: src/xui_popup.c:1255（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_popup/main.c:270; test_xui/xui_accessibility_test.c:273; test_xui/xui_popup_focus_test.c:226

## xuiPopupSetOpen
- 位置: xui.h:9783  已注释: 否
- 签名: `XUI_API int xuiPopupSetOpen(xui_widget pWidget, int bOpen);`
- 实现: src/xui_popup.c:1264（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch154_main1.c:42; examples/xui_popup/main.c:299; examples/xui_popup/main.c:308

## xuiPopupIsOpen
- 位置: xui.h:9784  已注释: 否
- 签名: `XUI_API int xuiPopupIsOpen(xui_widget pWidget);`
- 实现: src/xui_popup.c:1271（体 5 行）
- 用法: examples/xui_popup/main.c:307; examples/xui_popup/main.c:313; test_xui/xui_grid_focus_test.c:259

## xuiPopupToggle
- 位置: xui.h:9785  已注释: 否
- 签名: `XUI_API int xuiPopupToggle(xui_widget pWidget);`
- 实现: src/xui_popup.c:1277（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_popup/main.c:149

## xuiPopupApplyPlacement
- 位置: xui.h:9786  已注释: 否
- 签名: `XUI_API int xuiPopupApplyPlacement(xui_widget pWidget);`
- 实现: src/xui_popup.c:1284（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPopupSetOwner
- 位置: xui.h:9787  已注释: 否
- 签名: `XUI_API int xuiPopupSetOwner(xui_widget pWidget, xui_widget pOwner);`
- 实现: src/xui_popup.c:1298（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch154_main1.c:41; test_xui/xui_popup_focus_test.c:187; test_xui/xui_popup_focus_test.c:382

## xuiPopupGetOwner
- 位置: xui.h:9788  已注释: 否
- 签名: `XUI_API xui_widget xuiPopupGetOwner(xui_widget pWidget);`
- 实现: src/xui_popup.c:1311（体 5 行）
- 用法: test_xui/xui_property_grid_test.c:102; test_xui/xui_table_grid_test.c:318

## xuiPopupSetAnchorRect
- 位置: xui.h:9789  已注释: 否
- 签名: `XUI_API int xuiPopupSetAnchorRect(xui_widget pWidget, xui_rect_t tAnchor);`
- 实现: src/xui_popup.c:1317（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_native_popup_matrix_test.c:116; test_xui/xui_native_popup_matrix_test.c:151; test_xui/xui_popup_test.c:208

## xuiPopupClearAnchorRect
- 位置: xui.h:9790  已注释: 否
- 签名: `XUI_API int xuiPopupClearAnchorRect(xui_widget pWidget);`
- 实现: src/xui_popup.c:1328（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPopupGetAnchorRect
- 位置: xui.h:9791  已注释: 否
- 签名: `XUI_API xui_rect_t xuiPopupGetAnchorRect(xui_widget pWidget);`
- 实现: src/xui_popup.c:1337（体 5 行）

## xuiPopupSetAnchor
- 位置: xui.h:9792  已注释: 否
- 签名: `XUI_API int xuiPopupSetAnchor(xui_widget pWidget, int iAnchor);`
- 实现: src/xui_popup.c:1343（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPopupGetAnchor
- 位置: xui.h:9793  已注释: 否
- 签名: `XUI_API int xuiPopupGetAnchor(xui_widget pWidget);`
- 实现: src/xui_popup.c:1351（体 5 行）

## xuiPopupSetDirection
- 位置: xui.h:9794  已注释: 否
- 签名: `XUI_API int xuiPopupSetDirection(xui_widget pWidget, int iDirection);`
- 实现: src/xui_popup.c:1357（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_popup_test.c:210

## xuiPopupGetDirection
- 位置: xui.h:9795  已注释: 否
- 签名: `XUI_API int xuiPopupGetDirection(xui_widget pWidget);`
- 实现: src/xui_popup.c:1365（体 5 行）

## xuiPopupSetGap
- 位置: xui.h:9796  已注释: 否
- 签名: `XUI_API int xuiPopupSetGap(xui_widget pWidget, float fGap);`
- 实现: src/xui_popup.c:1371（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPopupGetGap
- 位置: xui.h:9797  已注释: 否
- 签名: `XUI_API float xuiPopupGetGap(xui_widget pWidget);`
- 实现: src/xui_popup.c:1379（体 5 行）

## xuiPopupSetOffset
- 位置: xui.h:9798  已注释: 否
- 签名: `XUI_API int xuiPopupSetOffset(xui_widget pWidget, float fOffsetX, float fOffsetY);`
- 实现: src/xui_popup.c:1385（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPopupGetOffset
- 位置: xui.h:9799  已注释: 否
- 签名: `XUI_API int xuiPopupGetOffset(xui_widget pWidget, float* pOffsetX, float* pOffsetY);`
- 实现: src/xui_popup.c:1394（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPopupSetMargin
- 位置: xui.h:9800  已注释: 否
- 签名: `XUI_API int xuiPopupSetMargin(xui_widget pWidget, float fMargin);`
- 实现: src/xui_popup.c:1403（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPopupGetMargin
- 位置: xui.h:9801  已注释: 否
- 签名: `XUI_API float xuiPopupGetMargin(xui_widget pWidget);`
- 实现: src/xui_popup.c:1411（体 5 行）

## xuiPopupSetContentSize
- 位置: xui.h:9802  已注释: 否
- 签名: `XUI_API int xuiPopupSetContentSize(xui_widget pWidget, float fWidth, float fHeight);`
- 实现: src/xui_popup.c:1417（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_popup_focus_test.c:249; test_xui/xui_popup_test.c:204

## xuiPopupGetContentSize
- 位置: xui.h:9803  已注释: 否
- 签名: `XUI_API int xuiPopupGetContentSize(xui_widget pWidget, float* pWidth, float* pHeight);`
- 实现: src/xui_popup.c:1441（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_menu_test.c:234

## xuiPopupSetMaxSize
- 位置: xui.h:9804  已注释: 否
- 签名: `XUI_API int xuiPopupSetMaxSize(xui_widget pWidget, float fMaxWidth, float fMaxHeight);`
- 实现: src/xui_popup.c:1450（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_popup_test.c:206

## xuiPopupGetMaxSize
- 位置: xui.h:9805  已注释: 否
- 签名: `XUI_API int xuiPopupGetMaxSize(xui_widget pWidget, float* pMaxWidth, float* pMaxHeight);`
- 实现: src/xui_popup.c:1461（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPopupSetMatchOwnerWidth
- 位置: xui.h:9806  已注释: 否
- 签名: `XUI_API int xuiPopupSetMatchOwnerWidth(xui_widget pWidget, int bEnabled);`
- 实现: src/xui_popup.c:1470（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPopupGetMatchOwnerWidth
- 位置: xui.h:9807  已注释: 否
- 签名: `XUI_API int xuiPopupGetMatchOwnerWidth(xui_widget pWidget);`
- 实现: src/xui_popup.c:1478（体 5 行）

## xuiPopupSetClosePolicy
- 位置: xui.h:9808  已注释: 否
- 签名: `XUI_API int xuiPopupSetClosePolicy(xui_widget pWidget, int iOutsidePolicy, int iOwnerPolicy, int iEscapePolicy);`
- 实现: src/xui_popup.c:1484（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_popup_test.c:194; test_xui/xui_popup_test.c:202

## xuiPopupGetClosePolicy
- 位置: xui.h:9809  已注释: 否
- 签名: `XUI_API int xuiPopupGetClosePolicy(xui_widget pWidget, int* pOutsidePolicy, int* pOwnerPolicy, int* pEscapePolicy);`
- 实现: src/xui_popup.c:1496（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPopupSetModal
- 位置: xui.h:9810  已注释: 否
- 签名: `XUI_API int xuiPopupSetModal(xui_widget pWidget, int bModal);`
- 实现: src/xui_popup.c:1506（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:272; test_xui/xui_accessibility_test.c:420; test_xui/xui_popup_focus_test.c:215

## xuiPopupIsModal
- 位置: xui.h:9811  已注释: 否
- 签名: `XUI_API int xuiPopupIsModal(xui_widget pWidget);`
- 实现: src/xui_popup.c:1516（体 5 行）

## xuiPopupSetConsumeInside
- 位置: xui.h:9812  已注释: 否
- 签名: `XUI_API int xuiPopupSetConsumeInside(xui_widget pWidget, int bEnabled);`
- 实现: src/xui_popup.c:1522（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPopupGetConsumeInside
- 位置: xui.h:9813  已注释: 否
- 签名: `XUI_API int xuiPopupGetConsumeInside(xui_widget pWidget);`
- 实现: src/xui_popup.c:1530（体 5 行）

## xuiPopupSetFocusPolicy
- 位置: xui.h:9814  已注释: 否
- 签名: `XUI_API int xuiPopupSetFocusPolicy(xui_widget pWidget, int iFocusPolicy, xui_widget pCustomFocus);`
- 实现: src/xui_popup.c:1536（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_popup_focus_test.c:189; test_xui/xui_popup_focus_test.c:212; test_xui/xui_popup_focus_test.c:218

## xuiPopupGetFocusPolicy
- 位置: xui.h:9815  已注释: 否
- 签名: `XUI_API int xuiPopupGetFocusPolicy(xui_widget pWidget);`
- 实现: src/xui_popup.c:1548（体 5 行）

## xuiPopupSetFocusRestore
- 位置: xui.h:9816  已注释: 否
- 签名: `XUI_API int xuiPopupSetFocusRestore(xui_widget pWidget, xui_widget pRestore);`
- 实现: src/xui_popup.c:1554（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_popup_focus_test.c:188; test_xui/xui_popup_focus_test.c:222; test_xui/xui_popup_focus_test.c:369

## xuiPopupSetScroll
- 位置: xui.h:9817  已注释: 否
- 签名: `XUI_API int xuiPopupSetScroll(xui_widget pWidget, float fOffsetX, float fOffsetY);`
- 实现: src/xui_popup.c:1566（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPopupGetScroll
- 位置: xui.h:9818  已注释: 否
- 签名: `XUI_API int xuiPopupGetScroll(xui_widget pWidget, float* pOffsetX, float* pOffsetY);`
- 实现: src/xui_popup.c:1579（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_popup/main.c:303; examples/xui_popup/main.c:319; test_xui/xui_popup_test.c:179

## xuiPopupSetScrollbarStyle
- 位置: xui.h:9819  已注释: 否
- 签名: `XUI_API int xuiPopupSetScrollbarStyle(xui_widget pWidget, int iMode, float fScrollbarSize);`
- 实现: src/xui_popup.c:1586（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPopupGetScrollbarStyle
- 位置: xui.h:9820  已注释: 否
- 签名: `XUI_API int xuiPopupGetScrollbarStyle(xui_widget pWidget, int* pMode, float* pScrollbarSize);`
- 实现: src/xui_popup.c:1608（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPopupGetPanelWidget
- 位置: xui.h:9821  已注释: 否
- 签名: `XUI_API xui_widget xuiPopupGetPanelWidget(xui_widget pWidget);`
- 实现: src/xui_popup.c:1617（体 5 行）
- 用法: test_xui/xui_native_popup_matrix_test.c:123; test_xui/xui_popup_test.c:137

## xuiPopupGetScrollViewWidget
- 位置: xui.h:9822  已注释: 否
- 签名: `XUI_API xui_widget xuiPopupGetScrollViewWidget(xui_widget pWidget);`
- 实现: src/xui_popup.c:1623（体 5 行）
- 用法: test_xui/xui_popup_test.c:138

## xuiPopupGetFrameWidget
- 位置: xui.h:9823  已注释: 否
- 签名: `XUI_API xui_widget xuiPopupGetFrameWidget(xui_widget pWidget);`
- 实现: src/xui_popup.c:1629（体 5 行）
- 用法: examples/xui_popup/main.c:314; examples/xui_popup/main.c:315; test_xui/xui_popup_test.c:139

## xuiPopupGetViewportWidget
- 位置: xui.h:9824  已注释: 否
- 签名: `XUI_API xui_widget xuiPopupGetViewportWidget(xui_widget pWidget);`
- 实现: src/xui_popup.c:1635（体 5 行）
- 用法: test_xui/xui_popup_test.c:142

## xuiPopupGetContentWidget
- 位置: xui.h:9825  已注释: 否
- 签名: `XUI_API xui_widget xuiPopupGetContentWidget(xui_widget pWidget);`
- 实现: src/xui_popup.c:1641（体 5 行）
- 用法: examples/xui_popup/main.c:273; examples/xui_popup/main.c:297; test_xui/xui_popup_focus_test.c:82

## xuiPopupGetModel
- 位置: xui.h:9826  已注释: 否
- 签名: `XUI_API xui_scroll_model_t* xuiPopupGetModel(xui_widget pWidget);`
- 实现: src/xui_popup.c:1647（体 5 行）

## xuiPopupGetPopupRect
- 位置: xui.h:9827  已注释: 否
- 签名: `XUI_API xui_rect_t xuiPopupGetPopupRect(xui_widget pWidget);`
- 实现: src/xui_popup.c:1653（体 5 行）
- 用法: examples/xui_cascader/main.c:339; examples/xui_combobox/main.c:397; examples/xui_iconpicker/main.c:330

## xuiPopupGetViewportRect
- 位置: xui.h:9828  已注释: 否
- 签名: `XUI_API xui_rect_t xuiPopupGetViewportRect(xui_widget pWidget);`
- 实现: src/xui_popup.c:1659（体 5 行）
- 用法: examples/xui_popup/main.c:312; test_xui/xui_popup_test.c:169

## xuiPopupGetContentRect
- 位置: xui.h:9829  已注释: 否
- 签名: `XUI_API xui_rect_t xuiPopupGetContentRect(xui_widget pWidget);`
- 实现: src/xui_popup.c:1665（体 22 行）

## xuiPopupSetColors
- 位置: xui.h:9830  已注释: 否
- 签名: `XUI_API int xuiPopupSetColors(xui_widget pWidget, uint32_t iPanel, uint32_t iBorder, uint32_t iShadow, uint32_t iBackdrop);`
- 实现: src/xui_popup.c:1688（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_native_popup_matrix_test.c:153; test_xui/xui_popup_focus_test.c:245

## xuiPopupGetColors
- 位置: xui.h:9831  已注释: 否
- 签名: `XUI_API int xuiPopupGetColors(xui_widget pWidget, uint32_t* pPanel, uint32_t* pBorder, uint32_t* pShadow, uint32_t* pBackdrop);`
- 实现: src/xui_popup.c:1710（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPopupSetMetrics
- 位置: xui.h:9832  已注释: 否
- 签名: `XUI_API int xuiPopupSetMetrics(xui_widget pWidget, float fPadding, float fBorderWidth, float fShadowSize);`
- 实现: src/xui_popup.c:1721（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPopupGetMetrics
- 位置: xui.h:9833  已注释: 否
- 签名: `XUI_API int xuiPopupGetMetrics(xui_widget pWidget, float* pPadding, float* pBorderWidth, float* pShadowSize);`
- 实现: src/xui_popup.c:1734（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPopupGetChangeCount
- 位置: xui.h:9834  已注释: 否
- 签名: `XUI_API int xuiPopupGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_popup.c:1744（体 5 行）

