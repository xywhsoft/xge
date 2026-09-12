# 草稿包：xui.h / cascader（55 条 API）

> 生成 2026-09-10 02:56 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiCascaderGetType
- 位置: xui.h:9474  已注释: 否
- 签名: `XUI_API xui_widget_type xuiCascaderGetType(xui_context pContext);`
- 实现: src/xui_cascader.c:1671（体 29 行）
- 返回码: NULL

## xuiCascaderCreate
- 位置: xui.h:9475  已注释: 否
- 签名: `XUI_API int xuiCascaderCreate(xui_context pContext, xui_widget* ppWidget, const xui_cascader_desc_t* pDesc);`
- 实现: src/xui_cascader.c:1701（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch165_main1.c:29; examples/xui_cascader/main.c:206; examples/xui_cascader/main.c:212

## xuiCascaderSetChange
- 位置: xui.h:9476  已注释: 否
- 签名: `XUI_API int xuiCascaderSetChange(xui_widget pWidget, xui_cascader_change_proc onChange, void* pUser);`
- 实现: src/xui_cascader.c:1712（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_cascader/main.c:208; examples/xui_cascader/main.c:214; examples/xui_cascader/main.c:223

## xuiCascaderSetItems
- 位置: xui.h:9477  已注释: 否
- 签名: `XUI_API int xuiCascaderSetItems(xui_widget pWidget, const xui_cascader_item_t* pItems, int iCount);`
- 实现: src/xui_cascader.c:1721（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCascaderGetItemCount
- 位置: xui.h:9478  已注释: 否
- 签名: `XUI_API int xuiCascaderGetItemCount(xui_widget pWidget);`
- 实现: src/xui_cascader.c:1728（体 5 行）
- 用法: test_xui/xui_cascader_test.c:160

## xuiCascaderGetItem
- 位置: xui.h:9479  已注释: 否
- 签名: `XUI_API const xui_cascader_item_t* xuiCascaderGetItem(xui_widget pWidget, int iIndex);`
- 实现: src/xui_cascader.c:1734（体 6 行）
- 返回码: NULL

## xuiCascaderSetSelectedPath
- 位置: xui.h:9480  已注释: 否
- 签名: `XUI_API int xuiCascaderSetSelectedPath(xui_widget pWidget, const int* arrValues, int iDepth);`
- 实现: src/xui_cascader.c:1741（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCascaderGetSelectedPath
- 位置: xui.h:9481  已注释: 否
- 签名: `XUI_API int xuiCascaderGetSelectedPath(xui_widget pWidget, int* arrValues, int iCapacity);`
- 实现: src/xui_cascader.c:1748（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_cascader_test.c:165; test_xui/xui_style_pickers_cascader_test.c:111

## xuiCascaderGetSelectedDepth
- 位置: xui.h:9482  已注释: 否
- 签名: `XUI_API int xuiCascaderGetSelectedDepth(xui_widget pWidget);`
- 实现: src/xui_cascader.c:1759（体 5 行）
- 用法: examples/xui_cascader/main.c:333; test_xui/xui_cascader_test.c:162; test_xui/xui_cascader_test.c:203

## xuiCascaderGetSelectedLeaf
- 位置: xui.h:9483  已注释: 否
- 签名: `XUI_API int xuiCascaderGetSelectedLeaf(xui_widget pWidget);`
- 实现: src/xui_cascader.c:1765（体 5 行）
- 用法: examples/xui_cascader/main.c:328; test_xui/xui_cascader_test.c:161; test_xui/xui_cascader_test.c:191

## xuiCascaderGetSelectedText
- 位置: xui.h:9484  已注释: 否
- 签名: `XUI_API const char* xuiCascaderGetSelectedText(xui_widget pWidget);`
- 实现: src/xui_cascader.c:1771（体 5 行）
- 用法: examples/xui_cascader/main.c:156; test_xui/xui_cascader_test.c:164; test_xui/xui_cascader_test.c:167

## xuiCascaderClear
- 位置: xui.h:9485  已注释: 否
- 签名: `XUI_API int xuiCascaderClear(xui_widget pWidget);`
- 实现: src/xui_cascader.c:1777（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_pickers_cascader_test.c:114

## xuiCascaderOpen
- 位置: xui.h:9486  已注释: 否
- 签名: `XUI_API int xuiCascaderOpen(xui_widget pWidget);`
- 实现: src/xui_cascader.c:1784（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_cascader/main.c:324; test_xui/xui_cascader_test.c:174; test_xui/xui_cascader_test.c:217

## xuiCascaderClose
- 位置: xui.h:9487  已注释: 否
- 签名: `XUI_API int xuiCascaderClose(xui_widget pWidget);`
- 实现: src/xui_cascader.c:1791（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_pickers_cascader_test.c:113

## xuiCascaderToggle
- 位置: xui.h:9488  已注释: 否
- 签名: `XUI_API int xuiCascaderToggle(xui_widget pWidget);`
- 实现: src/xui_cascader.c:1798（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCascaderIsOpen
- 位置: xui.h:9489  已注释: 否
- 签名: `XUI_API int xuiCascaderIsOpen(xui_widget pWidget);`
- 实现: src/xui_cascader.c:1805（体 5 行）
- 用法: examples/xui_cascader/main.c:326; examples/xui_cascader/main.c:338; test_xui/xui_cascader_test.c:175

## xuiCascaderSetShowAllLevels
- 位置: xui.h:9490  已注释: 否
- 签名: `XUI_API int xuiCascaderSetShowAllLevels(xui_widget pWidget, int bShowAllLevels);`
- 实现: src/xui_cascader.c:1811（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_cascader/main.c:215; test_xui/xui_cascader_test.c:166; test_xui/xui_cascader_test.c:168

## xuiCascaderGetShowAllLevels
- 位置: xui.h:9491  已注释: 否
- 签名: `XUI_API int xuiCascaderGetShowAllLevels(xui_widget pWidget);`
- 实现: src/xui_cascader.c:1820（体 5 行）

## xuiCascaderSetClearable
- 位置: xui.h:9492  已注释: 否
- 签名: `XUI_API int xuiCascaderSetClearable(xui_widget pWidget, int bClearable);`
- 实现: src/xui_cascader.c:1826（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCascaderGetClearable
- 位置: xui.h:9493  已注释: 否
- 签名: `XUI_API int xuiCascaderGetClearable(xui_widget pWidget);`
- 实现: src/xui_cascader.c:1834（体 5 行）

## xuiCascaderSetSelectAnyLevel
- 位置: xui.h:9494  已注释: 否
- 签名: `XUI_API int xuiCascaderSetSelectAnyLevel(xui_widget pWidget, int bSelectAnyLevel);`
- 实现: src/xui_cascader.c:1840（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCascaderGetSelectAnyLevel
- 位置: xui.h:9495  已注释: 否
- 签名: `XUI_API int xuiCascaderGetSelectAnyLevel(xui_widget pWidget);`
- 实现: src/xui_cascader.c:1848（体 5 行）

## xuiCascaderSetExpandTrigger
- 位置: xui.h:9496  已注释: 否
- 签名: `XUI_API int xuiCascaderSetExpandTrigger(xui_widget pWidget, int iExpandTrigger);`
- 实现: src/xui_cascader.c:1854（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCascaderGetExpandTrigger
- 位置: xui.h:9497  已注释: 否
- 签名: `XUI_API int xuiCascaderGetExpandTrigger(xui_widget pWidget);`
- 实现: src/xui_cascader.c:1862（体 5 行）

## xuiCascaderSetPopupSize
- 位置: xui.h:9498  已注释: 否
- 签名: `XUI_API int xuiCascaderSetPopupSize(xui_widget pWidget, float fColumnWidth, float fPopupHeight, float fPopupMaxHeight);`
- 实现: src/xui_cascader.c:1868（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCascaderGetPopupSize
- 位置: xui.h:9499  已注释: 否
- 签名: `XUI_API int xuiCascaderGetPopupSize(xui_widget pWidget, float* pColumnWidth, float* pPopupHeight, float* pPopupMaxHeight);`
- 实现: src/xui_cascader.c:1879（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCascaderSetPopupPlacement
- 位置: xui.h:9500  已注释: 否
- 签名: `XUI_API int xuiCascaderSetPopupPlacement(xui_widget pWidget, int iPlacement);`
- 实现: src/xui_cascader.c:1889（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_cascader/main.c:224

## xuiCascaderGetPopupPlacement
- 位置: xui.h:9501  已注释: 否
- 签名: `XUI_API int xuiCascaderGetPopupPlacement(xui_widget pWidget);`
- 实现: src/xui_cascader.c:1898（体 5 行）

## xuiCascaderSetMetrics
- 位置: xui.h:9502  已注释: 否
- 签名: `XUI_API int xuiCascaderSetMetrics(xui_widget pWidget, float fItemHeight, float fBorderWidth);`
- 实现: src/xui_cascader.c:1904（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCascaderGetMetrics
- 位置: xui.h:9503  已注释: 否
- 签名: `XUI_API int xuiCascaderGetMetrics(xui_widget pWidget, float* pItemHeight, float* pBorderWidth);`
- 实现: src/xui_cascader.c:1913（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCascaderSetColors
- 位置: xui.h:9504  已注释: 否
- 签名: `XUI_API int xuiCascaderSetColors(xui_widget pWidget, uint32_t iText, uint32_t iPlaceholder, uint32_t iDisabledText, uint32_t iBackground, uint32_t iHoverBackground, uint32_t iOpenBackground, uint32_t iDisabledBackground);`
- 实现: src/xui_cascader.c:1922（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_cascader/main.c:225

## xuiCascaderGetColors
- 位置: xui.h:9505  已注释: 否
- 签名: `XUI_API int xuiCascaderGetColors(xui_widget pWidget, uint32_t* pText, uint32_t* pPlaceholder, uint32_t* pDisabledText, uint32_t* pBackground, uint32_t* pHoverBackground, uint32_t* pOpenBackground, uint32_t* pDisabledBackground);`
- 实现: src/xui_cascader.c:1936（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCascaderSetBorderColors
- 位置: xui.h:9506  已注释: 否
- 签名: `XUI_API int xuiCascaderSetBorderColors(xui_widget pWidget, uint32_t iBorder, uint32_t iHoverBorder, uint32_t iFocusBorder);`
- 实现: src/xui_cascader.c:1950（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_cascader/main.c:233

## xuiCascaderGetBorderColors
- 位置: xui.h:9507  已注释: 否
- 签名: `XUI_API int xuiCascaderGetBorderColors(xui_widget pWidget, uint32_t* pBorder, uint32_t* pHoverBorder, uint32_t* pFocusBorder);`
- 实现: src/xui_cascader.c:1960（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCascaderSetArrowColors
- 位置: xui.h:9508  已注释: 否
- 签名: `XUI_API int xuiCascaderSetArrowColors(xui_widget pWidget, uint32_t iArrow, uint32_t iDisabledArrow);`
- 实现: src/xui_cascader.c:1970（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_cascader/main.c:237

## xuiCascaderGetArrowColors
- 位置: xui.h:9509  已注释: 否
- 签名: `XUI_API int xuiCascaderGetArrowColors(xui_widget pWidget, uint32_t* pArrow, uint32_t* pDisabledArrow);`
- 实现: src/xui_cascader.c:1979（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCascaderSetButtonColors
- 位置: xui.h:9510  已注释: 否
- 签名: `XUI_API int xuiCascaderSetButtonColors(xui_widget pWidget, uint32_t iButton, uint32_t iButtonHover, uint32_t iButtonOpen);`
- 实现: src/xui_cascader.c:1988（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_pickers_cascader_test.c:64

## xuiCascaderGetButtonColors
- 位置: xui.h:9511  已注释: 否
- 签名: `XUI_API int xuiCascaderGetButtonColors(xui_widget pWidget, uint32_t* pButton, uint32_t* pButtonHover, uint32_t* pButtonOpen);`
- 实现: src/xui_cascader.c:1998（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_pickers_cascader_test.c:67

## xuiCascaderSetPopupColors
- 位置: xui.h:9512  已注释: 否
- 签名: `XUI_API int xuiCascaderSetPopupColors(xui_widget pWidget, uint32_t iPanel, uint32_t iBorder, uint32_t iShadow, uint32_t iText, uint32_t iMutedText, uint32_t iHover, uint32_t iActive, uint32_t iSelected, uint32_t iActiveText, uint32_t iDisabledText, uint32_t iSeparator);`
- 实现: src/xui_cascader.c:2008（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_cascader/main.c:240

## xuiCascaderGetPopupColors
- 位置: xui.h:9513  已注释: 否
- 签名: `XUI_API int xuiCascaderGetPopupColors(xui_widget pWidget, uint32_t* pPanel, uint32_t* pBorder, uint32_t* pShadow, uint32_t* pText, uint32_t* pMutedText, uint32_t* pHover, uint32_t* pActive, uint32_t* pSelected, uint32_t* pActiveText, uint32_t* pDisabledText, uint32_t* pSeparator);`
- 实现: src/xui_cascader.c:2027（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCascaderSetFont
- 位置: xui.h:9514  已注释: 否
- 签名: `XUI_API int xuiCascaderSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_cascader.c:2045（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCascaderGetFont
- 位置: xui.h:9515  已注释: 否
- 签名: `XUI_API xui_font xuiCascaderGetFont(xui_widget pWidget);`
- 实现: src/xui_cascader.c:2053（体 5 行）

## xuiCascaderSetPlaceholder
- 位置: xui.h:9516  已注释: 否
- 签名: `XUI_API int xuiCascaderSetPlaceholder(xui_widget pWidget, const char* sPlaceholder);`
- 实现: src/xui_cascader.c:2059（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCascaderGetPlaceholder
- 位置: xui.h:9517  已注释: 否
- 签名: `XUI_API const char* xuiCascaderGetPlaceholder(xui_widget pWidget);`
- 实现: src/xui_cascader.c:2067（体 5 行）
- 用法: test_xui/xui_cascader_test.c:163

## xuiCascaderSetSeparator
- 位置: xui.h:9518  已注释: 否
- 签名: `XUI_API int xuiCascaderSetSeparator(xui_widget pWidget, const char* sSeparator);`
- 实现: src/xui_cascader.c:2073（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCascaderGetSeparator
- 位置: xui.h:9519  已注释: 否
- 签名: `XUI_API const char* xuiCascaderGetSeparator(xui_widget pWidget);`
- 实现: src/xui_cascader.c:2082（体 5 行）

## xuiCascaderGetPopupWidget
- 位置: xui.h:9520  已注释: 否
- 签名: `XUI_API xui_widget xuiCascaderGetPopupWidget(xui_widget pWidget);`
- 实现: src/xui_cascader.c:2088（体 5 行）
- 用法: examples/xui_cascader/main.c:339; test_xui/xui_cascader_test.c:181

## xuiCascaderGetPanelWidget
- 位置: xui.h:9521  已注释: 否
- 签名: `XUI_API xui_widget xuiCascaderGetPanelWidget(xui_widget pWidget);`
- 实现: src/xui_cascader.c:2094（体 5 行）
- 用法: examples/xui_cascader/main.c:294; examples/xui_cascader/main.c:322; test_xui/xui_cascader_test.c:61

## xuiCascaderGetColumnCount
- 位置: xui.h:9522  已注释: 否
- 签名: `XUI_API int xuiCascaderGetColumnCount(xui_widget pWidget);`
- 实现: src/xui_cascader.c:2100（体 5 行）
- 用法: examples/xui_cascader/main.c:326; test_xui/xui_cascader_test.c:183; test_xui/xui_cascader_test.c:211

## xuiCascaderGetItemRect
- 位置: xui.h:9523  已注释: 否
- 签名: `XUI_API xui_rect_t xuiCascaderGetItemRect(xui_widget pWidget, int iColumn, int iItem);`
- 实现: src/xui_cascader.c:2106（体 7 行）
- 用法: examples/xui_cascader/main.c:297; test_xui/xui_cascader_test.c:64

## xuiCascaderGetButtonRect
- 位置: xui.h:9524  已注释: 否
- 签名: `XUI_API xui_rect_t xuiCascaderGetButtonRect(xui_widget pWidget);`
- 实现: src/xui_cascader.c:2114（体 7 行）
- 用法: examples/xui_cascader/main.c:349; test_xui/xui_cascader_test.c:170

## xuiCascaderGetClearRect
- 位置: xui.h:9525  已注释: 否
- 签名: `XUI_API xui_rect_t xuiCascaderGetClearRect(xui_widget pWidget);`
- 实现: src/xui_cascader.c:2122（体 7 行）
- 用法: examples/xui_cascader/main.c:331; test_xui/xui_cascader_test.c:171

## xuiCascaderGetTextRect
- 位置: xui.h:9526  已注释: 否
- 签名: `XUI_API xui_rect_t xuiCascaderGetTextRect(xui_widget pWidget);`
- 实现: src/xui_cascader.c:2130（体 7 行）

## xuiCascaderGetState
- 位置: xui.h:9527  已注释: 否
- 签名: `XUI_API uint32_t xuiCascaderGetState(xui_widget pWidget);`
- 实现: src/xui_cascader.c:2138（体 5 行）
- 用法: test_xui/xui_cascader_test.c:184; test_xui/xui_cascader_test.c:219

## xuiCascaderGetChangeCount
- 位置: xui.h:9528  已注释: 否
- 签名: `XUI_API int xuiCascaderGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_cascader.c:2144（体 5 行）

