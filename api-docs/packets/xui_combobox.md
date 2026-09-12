# 草稿包：xui.h / combobox（59 条 API）

> 生成 2026-09-10 02:58 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiComboBoxGetType
- 位置: xui.h:9559  已注释: 否
- 签名: `XUI_API xui_widget_type xuiComboBoxGetType(xui_context pContext);`
- 实现: src/xui_combobox.c:1493（体 38 行）
- 返回码: NULL
- 用法: test_xui/xui_property_grid_test.c:571; test_xui/xui_table_grid_test.c:667

## xuiComboBoxCreate
- 位置: xui.h:9560  已注释: 否
- 签名: `XUI_API int xuiComboBoxCreate(xui_context pContext, xui_widget* ppWidget, const xui_combobox_desc_t* pDesc);`
- 实现: src/xui_combobox.c:1532（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch164_main1.c:21; examples/xui_combobox/main.c:208; examples/xui_combobox/main.c:237

## xuiComboBoxSetSelect
- 位置: xui.h:9561  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetSelect(xui_widget pWidget, xui_combobox_select_proc onSelect, void* pUser);`
- 实现: src/xui_combobox.c:1547（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_combobox/main.c:210; examples/xui_combobox/main.c:239; examples/xui_combobox/main.c:243

## xuiComboBoxSetTextChange
- 位置: xui.h:9562  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetTextChange(xui_widget pWidget, xui_combobox_text_proc onChange, void* pUser);`
- 实现: src/xui_combobox.c:1556（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_combobox/main.c:279; test_xui/xui_combobox_test.c:246

## xuiComboBoxSetItems
- 位置: xui.h:9563  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetItems(xui_widget pWidget, const char** arrItems, int iCount);`
- 实现: src/xui_combobox.c:1565（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:262

## xuiComboBoxSetItemData
- 位置: xui.h:9564  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetItemData(xui_widget pWidget, const xui_combobox_item_t* pItems, int iCount);`
- 实现: src/xui_combobox.c:1572（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiComboBoxSetEnabledItems
- 位置: xui.h:9565  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetEnabledItems(xui_widget pWidget, const int* arrEnabled, int iCount);`
- 实现: src/xui_combobox.c:1579（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_combobox_test.c:404; test_xui/xui_grid_focus_test.c:381; test_xui/xui_grid_focus_test.c:385

## xuiComboBoxGetItemCount
- 位置: xui.h:9566  已注释: 否
- 签名: `XUI_API int xuiComboBoxGetItemCount(xui_widget pWidget);`
- 实现: src/xui_combobox.c:1598（体 5 行）
- 用法: test_xui/xui_combobox_test.c:261

## xuiComboBoxGetItem
- 位置: xui.h:9567  已注释: 否
- 签名: `XUI_API const xui_combobox_item_t* xuiComboBoxGetItem(xui_widget pWidget, int iIndex);`
- 实现: src/xui_combobox.c:1604（体 6 行）
- 返回码: NULL

## xuiComboBoxGetItemText
- 位置: xui.h:9568  已注释: 否
- 签名: `XUI_API const char* xuiComboBoxGetItemText(xui_widget pWidget, int iIndex);`
- 实现: src/xui_combobox.c:1611（体 5 行）

## xuiComboBoxGetItemValue
- 位置: xui.h:9569  已注释: 否
- 签名: `XUI_API int xuiComboBoxGetItemValue(xui_widget pWidget, int iIndex);`
- 实现: src/xui_combobox.c:1617（体 5 行）

## xuiComboBoxIsItemEnabled
- 位置: xui.h:9570  已注释: 否
- 签名: `XUI_API int xuiComboBoxIsItemEnabled(xui_widget pWidget, int iIndex);`
- 实现: src/xui_combobox.c:1623（体 5 行）
- 用法: test_xui/xui_combobox_test.c:263; test_xui/xui_combobox_test.c:263; test_xui/xui_combobox_test.c:405

## xuiComboBoxSetSelected
- 位置: xui.h:9571  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetSelected(xui_widget pWidget, int iIndex);`
- 实现: src/xui_combobox.c:1629（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:263

## xuiComboBoxGetSelected
- 位置: xui.h:9572  已注释: 否
- 签名: `XUI_API int xuiComboBoxGetSelected(xui_widget pWidget);`
- 实现: src/xui_combobox.c:1636（体 5 行）
- 用法: examples/xui_combobox/main.c:364; examples/xui_combobox/main.c:368; examples/xui_combobox/main.c:371

## xuiComboBoxSetSelectedValue
- 位置: xui.h:9573  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetSelectedValue(xui_widget pWidget, int iValue);`
- 实现: src/xui_combobox.c:1642（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_combobox_test.c:388

## xuiComboBoxGetSelectedValue
- 位置: xui.h:9574  已注释: 否
- 签名: `XUI_API int xuiComboBoxGetSelectedValue(xui_widget pWidget);`
- 实现: src/xui_combobox.c:1649（体 8 行）
- 用法: test_xui/xui_combobox_test.c:262; test_xui/xui_combobox_test.c:377

## xuiComboBoxSetMode
- 位置: xui.h:9575  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_combobox.c:1658（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_accessibility_test.c:265; test_xui/xui_accessibility_test.c:347; test_xui/xui_style_pickers_combobox_test.c:90

## xuiComboBoxGetMode
- 位置: xui.h:9576  已注释: 否
- 签名: `XUI_API int xuiComboBoxGetMode(xui_widget pWidget);`
- 实现: src/xui_combobox.c:1690（体 5 行）
- 用法: test_xui/xui_combobox_test.c:290

## xuiComboBoxSetText
- 位置: xui.h:9577  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_combobox.c:1696（体 33 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: examples/xui_combobox/main.c:374; test_xui/xui_combobox_test.c:292; test_xui/xui_combobox_test.c:312

## xuiComboBoxGetText
- 位置: xui.h:9578  已注释: 否
- 签名: `XUI_API const char* xuiComboBoxGetText(xui_widget pWidget);`
- 实现: src/xui_combobox.c:1730（体 9 行）
- 用法: examples/xui_combobox/main.c:376; examples/xui_combobox/main.c:384; test_xui/xui_accessibility_test.c:544

## xuiComboBoxSetPlaceholder
- 位置: xui.h:9579  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetPlaceholder(xui_widget pWidget, const char* sText);`
- 实现: src/xui_combobox.c:1740（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiComboBoxGetPlaceholder
- 位置: xui.h:9580  已注释: 否
- 签名: `XUI_API const char* xuiComboBoxGetPlaceholder(xui_widget pWidget);`
- 实现: src/xui_combobox.c:1750（体 5 行）

## xuiComboBoxSetMaxLength
- 位置: xui.h:9581  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetMaxLength(xui_widget pWidget, int iMaxLength);`
- 实现: src/xui_combobox.c:1756（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiComboBoxGetMaxLength
- 位置: xui.h:9582  已注释: 否
- 签名: `XUI_API int xuiComboBoxGetMaxLength(xui_widget pWidget);`
- 实现: src/xui_combobox.c:1770（体 5 行）

## xuiComboBoxSetInputMenuTitle
- 位置: xui.h:9583  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetInputMenuTitle(xui_widget pWidget, int iCommand, const char* sTitle);`
- 实现: src/xui_combobox.c:1776（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_combobox_test.c:286; test_xui/xui_combobox_test.c:288

## xuiComboBoxGetInputMenuTitle
- 位置: xui.h:9584  已注释: 否
- 签名: `XUI_API const char* xuiComboBoxGetInputMenuTitle(xui_widget pWidget, int iCommand);`
- 实现: src/xui_combobox.c:1783（体 5 行）
- 用法: test_xui/xui_combobox_test.c:285; test_xui/xui_combobox_test.c:287; test_xui/xui_combobox_test.c:289

## xuiComboBoxOpenInputMenu
- 位置: xui.h:9585  已注释: 否
- 签名: `XUI_API int xuiComboBoxOpenInputMenu(xui_widget pWidget, float fX, float fY);`
- 实现: src/xui_combobox.c:1789（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: examples/xui_combobox/main.c:388; test_xui/xui_combobox_test.c:328; test_xui/xui_combobox_test.c:353

## xuiComboBoxGetInputMenuWidget
- 位置: xui.h:9586  已注释: 否
- 签名: `XUI_API xui_widget xuiComboBoxGetInputMenuWidget(xui_widget pWidget);`
- 实现: src/xui_combobox.c:1801（体 5 行）
- 用法: examples/xui_combobox/main.c:385; test_xui/xui_combobox_test.c:283

## xuiComboBoxOpen
- 位置: xui.h:9587  已注释: 否
- 签名: `XUI_API int xuiComboBoxOpen(xui_widget pWidget);`
- 实现: src/xui_combobox.c:1807（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_combobox/main.c:360; examples/xui_combobox/main.c:366; examples/xui_combobox/main.c:378

## xuiComboBoxClose
- 位置: xui.h:9588  已注释: 否
- 签名: `XUI_API int xuiComboBoxClose(xui_widget pWidget);`
- 实现: src/xui_combobox.c:1814（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_combobox/main.c:372; test_xui/xui_combobox_test.c:396; test_xui/xui_property_grid_test.c:577

## xuiComboBoxToggle
- 位置: xui.h:9589  已注释: 否
- 签名: `XUI_API int xuiComboBoxToggle(xui_widget pWidget);`
- 实现: src/xui_combobox.c:1828（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiComboBoxIsOpen
- 位置: xui.h:9590  已注释: 否
- 签名: `XUI_API int xuiComboBoxIsOpen(xui_widget pWidget);`
- 实现: src/xui_combobox.c:1835（体 5 行）
- 用法: examples/xui_combobox/main.c:371; examples/xui_combobox/main.c:396; test_xui/xui_accessibility_test.c:537

## xuiComboBoxSetPopupHeight
- 位置: xui.h:9591  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetPopupHeight(xui_widget pWidget, float fHeight);`
- 实现: src/xui_combobox.c:1841（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiComboBoxGetPopupHeight
- 位置: xui.h:9592  已注释: 否
- 签名: `XUI_API float xuiComboBoxGetPopupHeight(xui_widget pWidget);`
- 实现: src/xui_combobox.c:1850（体 5 行）

## xuiComboBoxSetPopupMaxHeight
- 位置: xui.h:9593  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetPopupMaxHeight(xui_widget pWidget, float fMaxHeight);`
- 实现: src/xui_combobox.c:1856（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiComboBoxGetPopupMaxHeight
- 位置: xui.h:9594  已注释: 否
- 签名: `XUI_API float xuiComboBoxGetPopupMaxHeight(xui_widget pWidget);`
- 实现: src/xui_combobox.c:1865（体 5 行）

## xuiComboBoxSetPopupPlacement
- 位置: xui.h:9595  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetPopupPlacement(xui_widget pWidget, int iPlacement);`
- 实现: src/xui_combobox.c:1871（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_combobox/main.c:244

## xuiComboBoxGetPopupPlacement
- 位置: xui.h:9596  已注释: 否
- 签名: `XUI_API int xuiComboBoxGetPopupPlacement(xui_widget pWidget);`
- 实现: src/xui_combobox.c:1882（体 5 行）

## xuiComboBoxSetMetrics
- 位置: xui.h:9597  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetMetrics(xui_widget pWidget, float fItemHeight, float fBorderWidth);`
- 实现: src/xui_combobox.c:1888（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiComboBoxGetMetrics
- 位置: xui.h:9598  已注释: 否
- 签名: `XUI_API int xuiComboBoxGetMetrics(xui_widget pWidget, float* pItemHeight, float* pBorderWidth);`
- 实现: src/xui_combobox.c:1897（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiComboBoxSetColors
- 位置: xui.h:9599  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetColors(xui_widget pWidget, uint32_t iText, uint32_t iDisabledText, uint32_t iBackground, uint32_t iHoverBackground, uint32_t iOpenBackground, uint32_t iDisabledBackground);`
- 实现: src/xui_combobox.c:1906（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_combobox/main.c:245; test_xui/xui_style_pickers_combobox_test.c:101

## xuiComboBoxGetColors
- 位置: xui.h:9600  已注释: 否
- 签名: `XUI_API int xuiComboBoxGetColors(xui_widget pWidget, uint32_t* pText, uint32_t* pDisabledText, uint32_t* pBackground, uint32_t* pHoverBackground, uint32_t* pOpenBackground, uint32_t* pDisabledBackground);`
- 实现: src/xui_combobox.c:1919（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiComboBoxSetBorderColors
- 位置: xui.h:9601  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetBorderColors(xui_widget pWidget, uint32_t iBorder, uint32_t iHoverBorder, uint32_t iFocusBorder);`
- 实现: src/xui_combobox.c:1932（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_combobox/main.c:252

## xuiComboBoxGetBorderColors
- 位置: xui.h:9602  已注释: 否
- 签名: `XUI_API int xuiComboBoxGetBorderColors(xui_widget pWidget, uint32_t* pBorder, uint32_t* pHoverBorder, uint32_t* pFocusBorder);`
- 实现: src/xui_combobox.c:1942（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiComboBoxSetArrowColors
- 位置: xui.h:9603  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetArrowColors(xui_widget pWidget, uint32_t iArrow, uint32_t iDisabledArrow);`
- 实现: src/xui_combobox.c:1952（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_combobox/main.c:256

## xuiComboBoxGetArrowColors
- 位置: xui.h:9604  已注释: 否
- 签名: `XUI_API int xuiComboBoxGetArrowColors(xui_widget pWidget, uint32_t* pArrow, uint32_t* pDisabledArrow);`
- 实现: src/xui_combobox.c:1961（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiComboBoxSetButtonColors
- 位置: xui.h:9605  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetButtonColors(xui_widget pWidget, uint32_t iButton, uint32_t iHover, uint32_t iOpen);`
- 实现: src/xui_combobox.c:1970（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_pickers_combobox_test.c:44

## xuiComboBoxGetButtonColors
- 位置: xui.h:9606  已注释: 否
- 签名: `XUI_API int xuiComboBoxGetButtonColors(xui_widget pWidget, uint32_t* pButton, uint32_t* pHover, uint32_t* pOpen);`
- 实现: src/xui_combobox.c:1980（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_pickers_combobox_test.c:47

## xuiComboBoxSetPopupColors
- 位置: xui.h:9607  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetPopupColors(xui_widget pWidget, uint32_t iPanel, uint32_t iBorder, uint32_t iShadow, uint32_t iHover, uint32_t iText, uint32_t iHoverText, uint32_t iDisabledText, uint32_t iSeparator);`
- 实现: src/xui_combobox.c:1990（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_combobox/main.c:259

## xuiComboBoxGetPopupColors
- 位置: xui.h:9608  已注释: 否
- 签名: `XUI_API int xuiComboBoxGetPopupColors(xui_widget pWidget, uint32_t* pPanel, uint32_t* pBorder, uint32_t* pShadow, uint32_t* pHover, uint32_t* pText, uint32_t* pHoverText, uint32_t* pDisabledText, uint32_t* pSeparator);`
- 实现: src/xui_combobox.c:2006（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiComboBoxSetFont
- 位置: xui.h:9609  已注释: 否
- 签名: `XUI_API int xuiComboBoxSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_combobox.c:2021（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiComboBoxGetFont
- 位置: xui.h:9610  已注释: 否
- 签名: `XUI_API xui_font xuiComboBoxGetFont(xui_widget pWidget);`
- 实现: src/xui_combobox.c:2031（体 5 行）

## xuiComboBoxGetMenuWidget
- 位置: xui.h:9611  已注释: 否
- 签名: `XUI_API xui_widget xuiComboBoxGetMenuWidget(xui_widget pWidget);`
- 实现: src/xui_combobox.c:2037（体 5 行）
- 用法: examples/xui_combobox/main.c:358; examples/xui_combobox/main.c:362; examples/xui_combobox/main.c:369

## xuiComboBoxGetPopupWidget
- 位置: xui.h:9612  已注释: 否
- 签名: `XUI_API xui_widget xuiComboBoxGetPopupWidget(xui_widget pWidget);`
- 实现: src/xui_combobox.c:2043（体 5 行）
- 用法: examples/xui_combobox/main.c:397; test_xui/xui_combobox_test.c:363; test_xui/xui_grid_focus_test.c:248

## xuiComboBoxGetInputWidget
- 位置: xui.h:9613  已注释: 否
- 签名: `XUI_API xui_widget xuiComboBoxGetInputWidget(xui_widget pWidget);`
- 实现: src/xui_combobox.c:2049（体 5 行）
- 用法: examples/xui_combobox/main.c:377; test_xui/xui_accessibility_test.c:546; test_xui/xui_combobox_test.c:266

## xuiComboBoxGetButtonRect
- 位置: xui.h:9614  已注释: 否
- 签名: `XUI_API xui_rect_t xuiComboBoxGetButtonRect(xui_widget pWidget);`
- 实现: src/xui_combobox.c:2055（体 7 行）
- 用法: examples/xui_combobox/main.c:407; test_xui/xui_combobox_test.c:264; test_xui/xui_combobox_test.c:268

## xuiComboBoxGetTextRect
- 位置: xui.h:9615  已注释: 否
- 签名: `XUI_API xui_rect_t xuiComboBoxGetTextRect(xui_widget pWidget);`
- 实现: src/xui_combobox.c:2063（体 7 行）

## xuiComboBoxGetState
- 位置: xui.h:9616  已注释: 否
- 签名: `XUI_API uint32_t xuiComboBoxGetState(xui_widget pWidget);`
- 实现: src/xui_combobox.c:2071（体 4 行）
- 用法: test_xui/xui_combobox_test.c:366; test_xui/xui_combobox_test.c:410

## xuiComboBoxGetChangeCount
- 位置: xui.h:9617  已注释: 否
- 签名: `XUI_API int xuiComboBoxGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_combobox.c:2076（体 5 行）

