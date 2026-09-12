# 草稿包：xui.h / iconpicker（53 条 API）

> 生成 2026-09-10 03:00 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiIconPickerGetType
- 位置: xui.h:9994  已注释: 否
- 签名: `XUI_API xui_widget_type xuiIconPickerGetType(xui_context pContext);`
- 实现: src/xui_icon_picker.c:1441（体 29 行）
- 返回码: NULL

## xuiIconPickerCreate
- 位置: xui.h:9995  已注释: 否
- 签名: `XUI_API int xuiIconPickerCreate(xui_context pContext, xui_widget* ppWidget, const xui_icon_picker_desc_t* pDesc);`
- 实现: src/xui_icon_picker.c:1471（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/xui_iconpicker/main.c:257; examples/xui_iconpicker/main.c:266; examples/xui_iconpicker/main.c:273

## xuiIconPickerSetChange
- 位置: xui.h:9996  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetChange(xui_widget pWidget, xui_icon_picker_change_proc onChange, void* pUser);`
- 实现: src/xui_icon_picker.c:1482（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_iconpicker/main.c:280; test_xui/xui_icon_picker_test.c:163

## xuiIconPickerSetFormatter
- 位置: xui.h:9997  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetFormatter(xui_widget pWidget, xui_icon_picker_format_proc onFormat, void* pUser);`
- 实现: src/xui_icon_picker.c:1491（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_icon_picker_test.c:222

## xuiIconPickerSetCategory
- 位置: xui.h:9998  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetCategory(xui_widget pWidget, xui_icon_category pCategory);`
- 实现: src/xui_icon_picker.c:1500（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_icon_picker_test.c:237

## xuiIconPickerGetCategory
- 位置: xui.h:9999  已注释: 否
- 签名: `XUI_API xui_icon_category xuiIconPickerGetCategory(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1519（体 5 行）

## xuiIconPickerSetSelectedId
- 位置: xui.h:10000  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetSelectedId(xui_widget pWidget, xui_icon_id iId);`
- 实现: src/xui_icon_picker.c:1525（体 5 行）
- 用法: test_xui/xui_icon_picker_test.c:228; test_xui/xui_style_pickers_icon_picker_test.c:87

## xuiIconPickerGetSelectedId
- 位置: xui.h:10001  已注释: 否
- 签名: `XUI_API xui_icon_id xuiIconPickerGetSelectedId(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1531（体 5 行）
- 用法: examples/xui_iconpicker/main.c:341; examples/xui_iconpicker/main.c:440; test_xui/xui_icon_picker_test.c:171

## xuiIconPickerGetSelectedIcon
- 位置: xui.h:10002  已注释: 否
- 签名: `XUI_API xui_icon xuiIconPickerGetSelectedIcon(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1537（体 6 行）
- 返回码: NULL
- 用法: examples/xui_iconpicker/main.c:225; test_xui/xui_icon_picker_test.c:172

## xuiIconPickerClearSelection
- 位置: xui.h:10003  已注释: 否
- 签名: `XUI_API int xuiIconPickerClearSelection(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1544（体 4 行）
- 用法: test_xui/xui_style_pickers_icon_picker_test.c:69

## xuiIconPickerSetTextMode
- 位置: xui.h:10004  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetTextMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_icon_picker.c:1549（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_icon_picker_test.c:223; test_xui/xui_icon_picker_test.c:225

## xuiIconPickerGetTextMode
- 位置: xui.h:10005  已注释: 否
- 签名: `XUI_API int xuiIconPickerGetTextMode(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1557（体 5 行）
- 用法: test_xui/xui_icon_picker_test.c:173; test_xui/xui_icon_picker_test.c:226

## xuiIconPickerSetPlaceholder
- 位置: xui.h:10006  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetPlaceholder(xui_widget pWidget, const char* sText);`
- 实现: src/xui_icon_picker.c:1563（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiIconPickerGetPlaceholder
- 位置: xui.h:10007  已注释: 否
- 签名: `XUI_API const char* xuiIconPickerGetPlaceholder(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1573（体 5 行）
- 用法: test_xui/xui_icon_picker_test.c:174

## xuiIconPickerSetGrid
- 位置: xui.h:10008  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetGrid(xui_widget pWidget, int iRows, int iColumns);`
- 实现: src/xui_icon_picker.c:1579（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_icon_picker_test.c:235

## xuiIconPickerGetGrid
- 位置: xui.h:10009  已注释: 否
- 签名: `XUI_API int xuiIconPickerGetGrid(xui_widget pWidget, int* pRows, int* pColumns);`
- 实现: src/xui_icon_picker.c:1590（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_icon_picker_test.c:236

## xuiIconPickerSetCellSize
- 位置: xui.h:10010  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetCellSize(xui_widget pWidget, float fWidth, float fHeight);`
- 实现: src/xui_icon_picker.c:1599（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiIconPickerGetCellSize
- 位置: xui.h:10011  已注释: 否
- 签名: `XUI_API int xuiIconPickerGetCellSize(xui_widget pWidget, float* pWidth, float* pHeight);`
- 实现: src/xui_icon_picker.c:1611（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiIconPickerSetSpacing
- 位置: xui.h:10012  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetSpacing(xui_widget pWidget, float fGapX, float fGapY, float fIconPadding);`
- 实现: src/xui_icon_picker.c:1620（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiIconPickerGetSpacing
- 位置: xui.h:10013  已注释: 否
- 签名: `XUI_API int xuiIconPickerGetSpacing(xui_widget pWidget, float* pGapX, float* pGapY, float* pIconPadding);`
- 实现: src/xui_icon_picker.c:1634（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiIconPickerSetValuePadding
- 位置: xui.h:10014  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetValuePadding(xui_widget pWidget, xui_thickness_t tPadding);`
- 实现: src/xui_icon_picker.c:1644（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiIconPickerGetValuePadding
- 位置: xui.h:10015  已注释: 否
- 签名: `XUI_API xui_thickness_t xuiIconPickerGetValuePadding(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1653（体 5 行）

## xuiIconPickerSetPopupPlacement
- 位置: xui.h:10016  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetPopupPlacement(xui_widget pWidget, int iPlacement);`
- 实现: src/xui_icon_picker.c:1659（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiIconPickerGetPopupPlacement
- 位置: xui.h:10017  已注释: 否
- 签名: `XUI_API int xuiIconPickerGetPopupPlacement(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1669（体 5 行）

## xuiIconPickerOpen
- 位置: xui.h:10018  已注释: 否
- 签名: `XUI_API int xuiIconPickerOpen(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1675（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_iconpicker/main.c:326; test_xui/xui_icon_picker_test.c:180; test_xui/xui_icon_picker_test.c:208

## xuiIconPickerClose
- 位置: xui.h:10019  已注释: 否
- 签名: `XUI_API int xuiIconPickerClose(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1700（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_icon_picker_test.c:234; test_xui/xui_style_pickers_icon_picker_test.c:85

## xuiIconPickerToggle
- 位置: xui.h:10020  已注释: 否
- 签名: `XUI_API int xuiIconPickerToggle(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1713（体 4 行）

## xuiIconPickerIsOpen
- 位置: xui.h:10021  已注释: 否
- 签名: `XUI_API int xuiIconPickerIsOpen(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1718（体 5 行）
- 用法: examples/xui_iconpicker/main.c:342; test_xui/xui_icon_picker_test.c:180; test_xui/xui_icon_picker_test.c:204

## xuiIconPickerEnsureVisible
- 位置: xui.h:10022  已注释: 否
- 签名: `XUI_API int xuiIconPickerEnsureVisible(xui_widget pWidget, xui_icon_id iId);`
- 实现: src/xui_icon_picker.c:1724（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_iconpicker/main.c:333; test_xui/xui_icon_picker_test.c:218

## xuiIconPickerSetMetrics
- 位置: xui.h:10023  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetMetrics(xui_widget pWidget, float fBorderWidth, float fScrollbarSize);`
- 实现: src/xui_icon_picker.c:1736（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiIconPickerGetMetrics
- 位置: xui.h:10024  已注释: 否
- 签名: `XUI_API int xuiIconPickerGetMetrics(xui_widget pWidget, float* pBorderWidth, float* pScrollbarSize);`
- 实现: src/xui_icon_picker.c:1748（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiIconPickerSetColors
- 位置: xui.h:10025  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetColors(xui_widget pWidget, uint32_t iText, uint32_t iPlaceholder, uint32_t iDisabledText, uint32_t iBackground, uint32_t iHoverBackground, uint32_t iOpenBackground, uint32_t iDisabledBackground);`
- 实现: src/xui_icon_picker.c:1757（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiIconPickerGetColors
- 位置: xui.h:10026  已注释: 否
- 签名: `XUI_API int xuiIconPickerGetColors(xui_widget pWidget, uint32_t* pText, uint32_t* pPlaceholder, uint32_t* pDisabledText, uint32_t* pBackground, uint32_t* pHoverBackground, uint32_t* pOpenBackground, uint32_t* pDisabledBackground);`
- 实现: src/xui_icon_picker.c:1771（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiIconPickerSetBorderColors
- 位置: xui.h:10027  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetBorderColors(xui_widget pWidget, uint32_t iBorder, uint32_t iHoverBorder, uint32_t iFocusBorder);`
- 实现: src/xui_icon_picker.c:1785（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiIconPickerGetBorderColors
- 位置: xui.h:10028  已注释: 否
- 签名: `XUI_API int xuiIconPickerGetBorderColors(xui_widget pWidget, uint32_t* pBorder, uint32_t* pHoverBorder, uint32_t* pFocusBorder);`
- 实现: src/xui_icon_picker.c:1795（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiIconPickerSetArrowColors
- 位置: xui.h:10029  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetArrowColors(xui_widget pWidget, uint32_t iArrow, uint32_t iDisabledArrow);`
- 实现: src/xui_icon_picker.c:1805（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiIconPickerGetArrowColors
- 位置: xui.h:10030  已注释: 否
- 签名: `XUI_API int xuiIconPickerGetArrowColors(xui_widget pWidget, uint32_t* pArrow, uint32_t* pDisabledArrow);`
- 实现: src/xui_icon_picker.c:1814（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiIconPickerSetButtonColors
- 位置: xui.h:10031  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetButtonColors(xui_widget pWidget, uint32_t iButton, uint32_t iHover, uint32_t iOpen);`
- 实现: src/xui_icon_picker.c:1823（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiIconPickerGetButtonColors
- 位置: xui.h:10032  已注释: 否
- 签名: `XUI_API int xuiIconPickerGetButtonColors(xui_widget pWidget, uint32_t* pButton, uint32_t* pHover, uint32_t* pOpen);`
- 实现: src/xui_icon_picker.c:1833（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiIconPickerSetPopupColors
- 位置: xui.h:10033  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetPopupColors(xui_widget pWidget, uint32_t iPanel, uint32_t iBorder, uint32_t iShadow, uint32_t iHover, uint32_t iSelected, uint32_t iFocus);`
- 实现: src/xui_icon_picker.c:1843（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_pickers_icon_picker_test.c:118

## xuiIconPickerGetPopupColors
- 位置: xui.h:10034  已注释: 否
- 签名: `XUI_API int xuiIconPickerGetPopupColors(xui_widget pWidget, uint32_t* pPanel, uint32_t* pBorder, uint32_t* pShadow, uint32_t* pHover, uint32_t* pSelected, uint32_t* pFocus);`
- 实现: src/xui_icon_picker.c:1857（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiIconPickerSetFont
- 位置: xui.h:10035  已注释: 否
- 签名: `XUI_API int xuiIconPickerSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_icon_picker.c:1870（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiIconPickerGetFont
- 位置: xui.h:10036  已注释: 否
- 签名: `XUI_API xui_font xuiIconPickerGetFont(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1878（体 5 行）

## xuiIconPickerGetPopupWidget
- 位置: xui.h:10037  已注释: 否
- 签名: `XUI_API xui_widget xuiIconPickerGetPopupWidget(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1884（体 5 行）
- 用法: examples/xui_iconpicker/main.c:324; examples/xui_iconpicker/main.c:328; test_xui/xui_icon_picker_test.c:183

## xuiIconPickerGetFrameWidget
- 位置: xui.h:10038  已注释: 否
- 签名: `XUI_API xui_widget xuiIconPickerGetFrameWidget(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1890（体 5 行）
- 用法: examples/xui_iconpicker/main.c:329; test_xui/xui_icon_picker_test.c:184

## xuiIconPickerGetViewportWidget
- 位置: xui.h:10039  已注释: 否
- 签名: `XUI_API xui_widget xuiIconPickerGetViewportWidget(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1896（体 5 行）
- 用法: test_xui/xui_icon_picker_test.c:185

## xuiIconPickerGetButtonRect
- 位置: xui.h:10040  已注释: 否
- 签名: `XUI_API xui_rect_t xuiIconPickerGetButtonRect(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1902（体 7 行）
- 用法: test_xui/xui_icon_picker_test.c:177

## xuiIconPickerGetValueRect
- 位置: xui.h:10041  已注释: 否
- 签名: `XUI_API xui_rect_t xuiIconPickerGetValueRect(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1910（体 7 行）
- 用法: test_xui/xui_icon_picker_test.c:175

## xuiIconPickerGetItemRect
- 位置: xui.h:10042  已注释: 否
- 签名: `XUI_API xui_rect_t xuiIconPickerGetItemRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_icon_picker.c:1918（体 16 行）
- 用法: test_xui/xui_icon_picker_test.c:196; test_xui/xui_icon_picker_test.c:210

## xuiIconPickerGetHoverIndex
- 位置: xui.h:10043  已注释: 否
- 签名: `XUI_API int xuiIconPickerGetHoverIndex(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1935（体 5 行）

## xuiIconPickerGetFocusIndex
- 位置: xui.h:10044  已注释: 否
- 签名: `XUI_API int xuiIconPickerGetFocusIndex(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1941（体 5 行）
- 用法: test_xui/xui_icon_picker_test.c:200; test_xui/xui_icon_picker_test.c:202

## xuiIconPickerGetState
- 位置: xui.h:10045  已注释: 否
- 签名: `XUI_API uint32_t xuiIconPickerGetState(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1947（体 5 行）
- 用法: test_xui/xui_icon_picker_test.c:188

## xuiIconPickerGetChangeCount
- 位置: xui.h:10046  已注释: 否
- 签名: `XUI_API int xuiIconPickerGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_icon_picker.c:1953（体 5 行）

