# 草稿包：xui.h / accordion（37 条 API）

> 生成 2026-09-10 02:55 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiAccordionGetType
- 位置: xui.h:8148  已注释: 否
- 签名: `XUI_API xui_widget_type xuiAccordionGetType(xui_context pContext);`
- 实现: src/xui_accordion.c:1232（体 37 行）
- 返回码: NULL

## xuiAccordionCreate
- 位置: xui.h:8149  已注释: 否
- 签名: `XUI_API int xuiAccordionCreate(xui_context pContext, xui_widget* ppWidget, const xui_accordion_desc_t* pDesc);`
- 实现: src/xui_accordion.c:1270（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch152_main1.c:13; examples/xui_accordion/main.c:169; test_xui/xui_accordion_test.c:146

## xuiAccordionSetSelect
- 位置: xui.h:8150  已注释: 否
- 签名: `XUI_API int xuiAccordionSetSelect(xui_widget pWidget, xui_accordion_select_proc onSelect, void* pUser);`
- 实现: src/xui_accordion.c:1285（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_accordion/main.c:171; test_xui/xui_accordion_test.c:151

## xuiAccordionClear
- 位置: xui.h:8151  已注释: 否
- 签名: `XUI_API int xuiAccordionClear(xui_widget pWidget);`
- 实现: src/xui_accordion.c:1296（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accordion_test.c:250

## xuiAccordionAddSection
- 位置: xui.h:8152  已注释: 否
- 签名: `XUI_API int xuiAccordionAddSection(xui_widget pWidget, const char* sTitle, int iId, int bExpanded, xui_widget* ppClient);`
- 实现: src/xui_accordion.c:1319（体 28 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/tutorial_capture/ch152_main1.c:20; examples/tutorial_capture/ch152_main1.c:21; examples/tutorial_capture/ch152_main1.c:22

## xuiAccordionGetSectionCount
- 位置: xui.h:8153  已注释: 否
- 签名: `XUI_API int xuiAccordionGetSectionCount(xui_widget pWidget);`
- 实现: src/xui_accordion.c:1348（体 7 行）
- 用法: examples/xui_accordion/main.c:267; test_xui/xui_accordion_test.c:174; test_xui/xui_accordion_test.c:251

## xuiAccordionGetSectionWidget
- 位置: xui.h:8154  已注释: 否
- 签名: `XUI_API xui_widget xuiAccordionGetSectionWidget(xui_widget pWidget, int iIndex);`
- 实现: src/xui_accordion.c:1356（体 8 行）
- 返回码: NULL
- 用法: test_xui/xui_accordion_test.c:177

## xuiAccordionGetHeaderWidget
- 位置: xui.h:8155  已注释: 否
- 签名: `XUI_API xui_widget xuiAccordionGetHeaderWidget(xui_widget pWidget, int iIndex);`
- 实现: src/xui_accordion.c:1365（体 8 行）
- 返回码: NULL
- 用法: test_xui/xui_accordion_test.c:178; test_xui/xui_accordion_test.c:179; test_xui/xui_accordion_test.c:185

## xuiAccordionGetButtonWidget
- 位置: xui.h:8156  已注释: 否
- 签名: `XUI_API xui_widget xuiAccordionGetButtonWidget(xui_widget pWidget, int iIndex);`
- 实现: src/xui_accordion.c:1374（体 4 行）
- 用法: test_xui/xui_accordion_test.c:179

## xuiAccordionGetClientWidget
- 位置: xui.h:8157  已注释: 否
- 签名: `XUI_API xui_widget xuiAccordionGetClientWidget(xui_widget pWidget, int iIndex);`
- 实现: src/xui_accordion.c:1379（体 8 行）
- 返回码: NULL
- 用法: test_xui/xui_accordion_test.c:180

## xuiAccordionAddSectionChild
- 位置: xui.h:8158  已注释: 否
- 签名: `XUI_API int xuiAccordionAddSectionChild(xui_widget pWidget, int iIndex, xui_widget pChild);`
- 实现: src/xui_accordion.c:1388（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_accordion/main.c:150; test_xui/xui_accordion_test.c:84

## xuiAccordionSetSectionTitle
- 位置: xui.h:8159  已注释: 否
- 签名: `XUI_API int xuiAccordionSetSectionTitle(xui_widget pWidget, int iIndex, const char* sTitle);`
- 实现: src/xui_accordion.c:1398（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiAccordionGetSectionTitle
- 位置: xui.h:8160  已注释: 否
- 签名: `XUI_API const char* xuiAccordionGetSectionTitle(xui_widget pWidget, int iIndex);`
- 实现: src/xui_accordion.c:1408（体 8 行）
- 返回码: NULL
- 用法: test_xui/xui_accordion_test.c:175

## xuiAccordionSetSectionId
- 位置: xui.h:8161  已注释: 否
- 签名: `XUI_API int xuiAccordionSetSectionId(xui_widget pWidget, int iIndex, int iId);`
- 实现: src/xui_accordion.c:1417（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiAccordionGetSectionId
- 位置: xui.h:8162  已注释: 否
- 签名: `XUI_API int xuiAccordionGetSectionId(xui_widget pWidget, int iIndex);`
- 实现: src/xui_accordion.c:1427（体 8 行）
- 用法: test_xui/xui_accordion_test.c:176

## xuiAccordionSetExpanded
- 位置: xui.h:8163  已注释: 否
- 签名: `XUI_API int xuiAccordionSetExpanded(xui_widget pWidget, int iIndex, int bExpanded);`
- 实现: src/xui_accordion.c:1436（体 7 行）
- 用法: test_xui/xui_accordion_test.c:213

## xuiAccordionIsExpanded
- 位置: xui.h:8164  已注释: 否
- 签名: `XUI_API int xuiAccordionIsExpanded(xui_widget pWidget, int iIndex);`
- 实现: src/xui_accordion.c:1444（体 8 行）
- 用法: examples/xui_accordion/main.c:278; examples/xui_accordion/main.c:279; examples/xui_accordion/main.c:280

## xuiAccordionSetSectionEnabled
- 位置: xui.h:8165  已注释: 否
- 签名: `XUI_API int xuiAccordionSetSectionEnabled(xui_widget pWidget, int iIndex, int bEnabled);`
- 实现: src/xui_accordion.c:1453（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_accordion/main.c:200; test_xui/xui_accordion_test.c:160

## xuiAccordionIsSectionEnabled
- 位置: xui.h:8166  已注释: 否
- 签名: `XUI_API int xuiAccordionIsSectionEnabled(xui_widget pWidget, int iIndex);`
- 实现: src/xui_accordion.c:1467（体 8 行）
- 用法: test_xui/xui_accordion_test.c:184

## xuiAccordionSetMode
- 位置: xui.h:8167  已注释: 否
- 签名: `XUI_API int xuiAccordionSetMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_accordion.c:1476（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accordion_test.c:211

## xuiAccordionGetMode
- 位置: xui.h:8168  已注释: 否
- 签名: `XUI_API int xuiAccordionGetMode(xui_widget pWidget);`
- 实现: src/xui_accordion.c:1501（体 7 行）
- 用法: examples/xui_accordion/main.c:274; test_xui/xui_accordion_test.c:212

## xuiAccordionSetFont
- 位置: xui.h:8169  已注释: 否
- 签名: `XUI_API int xuiAccordionSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_accordion.c:1509（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiAccordionGetFont
- 位置: xui.h:8170  已注释: 否
- 签名: `XUI_API xui_font xuiAccordionGetFont(xui_widget pWidget);`
- 实现: src/xui_accordion.c:1519（体 7 行）

## xuiAccordionSetMetrics
- 位置: xui.h:8171  已注释: 否
- 签名: `XUI_API int xuiAccordionSetMetrics(xui_widget pWidget, float fHeaderHeight, float fSpacing, float fContentPadding);`
- 实现: src/xui_accordion.c:1527（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_accordion/main.c:215; test_xui/xui_accordion_test.c:219

## xuiAccordionGetMetrics
- 位置: xui.h:8172  已注释: 否
- 签名: `XUI_API int xuiAccordionGetMetrics(xui_widget pWidget, float* pHeaderHeight, float* pSpacing, float* pContentPadding);`
- 实现: src/xui_accordion.c:1540（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_accordion_test.c:221

## xuiAccordionSetColors
- 位置: xui.h:8173  已注释: 否
- 签名: `XUI_API int xuiAccordionSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iHeader, uint32_t iHover, uint32_t iExpanded, uint32_t iContent, uint32_t iBorder, uint32_t iText, uint32_t iActiveText, uint32_t iDisabledText);`
- 实现: src/xui_accordion.c:1552（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_accordion/main.c:216; test_xui/xui_accordion_test.c:223

## xuiAccordionGetColors
- 位置: xui.h:8174  已注释: 否
- 签名: `XUI_API int xuiAccordionGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pHeader, uint32_t* pHover, uint32_t* pExpanded, uint32_t* pContent, uint32_t* pBorder, uint32_t* pText, uint32_t* pActiveText, uint32_t* pDisabledText);`
- 实现: src/xui_accordion.c:1570（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_accordion_test.c:234

## xuiAccordionGetContentHeight
- 位置: xui.h:8175  已注释: 否
- 签名: `XUI_API float xuiAccordionGetContentHeight(xui_widget pWidget);`
- 实现: src/xui_accordion.c:1588（体 7 行）
- 用法: examples/xui_accordion/main.c:273; test_xui/xui_accordion_test.c:197

## xuiAccordionGetSectionRect
- 位置: xui.h:8176  已注释: 否
- 签名: `XUI_API xui_rect_t xuiAccordionGetSectionRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_accordion.c:1596（体 8 行）

## xuiAccordionGetHeaderRect
- 位置: xui.h:8177  已注释: 否
- 签名: `XUI_API xui_rect_t xuiAccordionGetHeaderRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_accordion.c:1605（体 8 行）
- 用法: examples/xui_accordion/main.c:277; examples/xui_accordion/main.c:284; examples/xui_accordion/main.c:286

## xuiAccordionGetClientRect
- 位置: xui.h:8178  已注释: 否
- 签名: `XUI_API xui_rect_t xuiAccordionGetClientRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_accordion.c:1614（体 8 行）
- 用法: examples/xui_accordion/main.c:268; examples/xui_accordion/main.c:269; test_xui/xui_accordion_test.c:191

## xuiAccordionGetArrowRect
- 位置: xui.h:8179  已注释: 否
- 签名: `XUI_API xui_rect_t xuiAccordionGetArrowRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_accordion.c:1623（体 8 行）
- 用法: examples/xui_accordion/main.c:272; test_xui/xui_accordion_test.c:193

## xuiAccordionGetSelected
- 位置: xui.h:8180  已注释: 否
- 签名: `XUI_API int xuiAccordionGetSelected(xui_widget pWidget);`
- 实现: src/xui_accordion.c:1632（体 7 行）
- 用法: test_xui/xui_accordion_test.c:242

## xuiAccordionGetHoverIndex
- 位置: xui.h:8181  已注释: 否
- 签名: `XUI_API int xuiAccordionGetHoverIndex(xui_widget pWidget);`
- 实现: src/xui_accordion.c:1640（体 7 行）

## xuiAccordionGetActiveIndex
- 位置: xui.h:8182  已注释: 否
- 签名: `XUI_API int xuiAccordionGetActiveIndex(xui_widget pWidget);`
- 实现: src/xui_accordion.c:1648（体 7 行）

## xuiAccordionGetState
- 位置: xui.h:8183  已注释: 否
- 签名: `XUI_API uint32_t xuiAccordionGetState(xui_widget pWidget);`
- 实现: src/xui_accordion.c:1656（体 14 行）
- 用法: test_xui/xui_accordion_test.c:247

## xuiAccordionGetChangeCount
- 位置: xui.h:8184  已注释: 否
- 签名: `XUI_API int xuiAccordionGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_accordion.c:1671（体 7 行）
- 用法: test_xui/xui_accordion_test.c:248

