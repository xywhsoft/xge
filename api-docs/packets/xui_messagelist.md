# 草稿包：xui.h / messagelist（43 条 API）

> 生成 2026-09-10 03:02 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiMessageListGetType
- 位置: xui.h:9300  已注释: 否
- 签名: `XUI_API xui_widget_type xuiMessageListGetType(xui_context pContext);`
- 实现: src/xui_message_list.c:1652（体 25 行）
- 返回码: NULL

## xuiMessageListCreate
- 位置: xui.h:9301  已注释: 否
- 签名: `XUI_API int xuiMessageListCreate(xui_context pContext, xui_widget* ppWidget, const xui_message_list_desc_t* pDesc);`
- 实现: src/xui_message_list.c:1678（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch185_main1.c:22; examples/tutorial_capture/ch208_main1.c:26; examples/xui_messagelist/main.c:138

## xuiMessageListSetEvent
- 位置: xui.h:9302  已注释: 否
- 签名: `XUI_API int xuiMessageListSetEvent(xui_widget pWidget, xui_message_list_event_proc onEvent, void* pUser);`
- 实现: src/xui_message_list.c:1688（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_messagelist/main.c:141; test_xui/xui_message_list_audit_test.c:742; test_xui/xui_message_list_test.c:137

## xuiMessageListSetNodeRenderer
- 位置: xui.h:9303  已注释: 否
- 签名: `XUI_API int xuiMessageListSetNodeRenderer(xui_widget pWidget, xui_message_list_node_renderer_proc onRender, void* pUser);`
- 实现: src/xui_message_list.c:1697（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_collections_message_test.c:49

## xuiMessageListSetNodes
- 位置: xui.h:9304  已注释: 否
- 签名: `XUI_API int xuiMessageListSetNodes(xui_widget pWidget, const xui_message_node_t* pNodes, int iCount);`
- 实现: src/xui_message_list.c:1706（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_message_list_audit_test.c:155; test_xui/xui_message_list_audit_test.c:175; test_xui/xui_message_list_audit_test.c:185

## xuiMessageListAddNode
- 位置: xui.h:9305  已注释: 否
- 签名: `XUI_API int xuiMessageListAddNode(xui_widget pWidget, const xui_message_node_t* pNode);`
- 实现: src/xui_message_list.c:1730（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_message_list_audit_test.c:763; test_xui/xui_message_list_test.c:267

## xuiMessageListUpdateNodeText
- 位置: xui.h:9306  已注释: 否
- 签名: `XUI_API int xuiMessageListUpdateNodeText(xui_widget pWidget, const char* sId, const char* sText);`
- 实现: src/xui_message_list.c:1798（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_message_list_audit_test.c:198; test_xui/xui_message_list_audit_test.c:202; test_xui/xui_message_list_audit_test.c:527

## xuiMessageListAppendNodeText
- 位置: xui.h:9307  已注释: 否
- 签名: `XUI_API int xuiMessageListAppendNodeText(xui_widget pWidget, const char* sId, const char* sText);`
- 实现: src/xui_message_list.c:1813（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_message_list_audit_test.c:200; test_xui/xui_message_list_audit_test.c:530; test_xui/xui_message_list_audit_test.c:757

## xuiMessageListSetNodeTitle
- 位置: xui.h:9308  已注释: 否
- 签名: `XUI_API int xuiMessageListSetNodeTitle(xui_widget pWidget, const char* sId, const char* sTitle);`
- 实现: src/xui_message_list.c:1828（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_message_list_audit_test.c:770

## xuiMessageListSetNodeCollapsed
- 位置: xui.h:9309  已注释: 否
- 签名: `XUI_API int xuiMessageListSetNodeCollapsed(xui_widget pWidget, const char* sId, int bCollapsed);`
- 实现: src/xui_message_list.c:1842（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_message_list_audit_test.c:773; test_xui/xui_message_list_audit_test.c:774; test_xui/xui_message_list_test.c:287

## xuiMessageListGetNodeCollapsed
- 位置: xui.h:9310  已注释: 否
- 签名: `XUI_API int xuiMessageListGetNodeCollapsed(xui_widget pWidget, const char* sId);`
- 实现: src/xui_message_list.c:1854（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_message_list_test.c:279; test_xui/xui_message_list_test.c:285; test_xui/xui_message_list_test.c:288

## xuiMessageListClear
- 位置: xui.h:9311  已注释: 否
- 签名: `XUI_API int xuiMessageListClear(xui_widget pWidget);`
- 实现: src/xui_message_list.c:1864（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_message_list_audit_test.c:883; test_xui/xui_message_list_test.c:241; test_xui/xui_message_list_test.c:252

## xuiMessageListGetNodeCount
- 位置: xui.h:9312  已注释: 否
- 签名: `XUI_API int xuiMessageListGetNodeCount(xui_widget pWidget);`
- 实现: src/xui_message_list.c:1872（体 5 行）
- 用法: examples/xui_messagelist/main.c:208; examples/xui_messagelist/main.c:229; test_xui/xui_message_list_test.c:141

## xuiMessageListGetNode
- 位置: xui.h:9313  已注释: 否
- 签名: `XUI_API const xui_message_node_t* xuiMessageListGetNode(xui_widget pWidget, int iIndex);`
- 实现: src/xui_message_list.c:1878（体 8 行）
- 返回码: NULL
- 用法: test_xui/xui_message_list_test.c:142; test_xui/xui_message_list_test.c:143; test_xui/xui_message_list_test.c:144

## xuiMessageListSetSelected
- 位置: xui.h:9314  已注释: 否
- 签名: `XUI_API int xuiMessageListSetSelected(xui_widget pWidget, int iIndex);`
- 实现: src/xui_message_list.c:1887（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiMessageListGetSelected
- 位置: xui.h:9315  已注释: 否
- 签名: `XUI_API int xuiMessageListGetSelected(xui_widget pWidget);`
- 实现: src/xui_message_list.c:1897（体 5 行）
- 用法: test_xui/xui_message_list_test.c:237

## xuiMessageListGetHoverIndex
- 位置: xui.h:9316  已注释: 否
- 签名: `XUI_API int xuiMessageListGetHoverIndex(xui_widget pWidget);`
- 实现: src/xui_message_list.c:1903（体 5 行）

## xuiMessageListGetNodeAt
- 位置: xui.h:9317  已注释: 否
- 签名: `XUI_API int xuiMessageListGetNodeAt(xui_widget pWidget, float fX, float fY);`
- 实现: src/xui_message_list.c:1909（体 4 行）
- 用法: test_xui/xui_message_list_audit_test.c:723; test_xui/xui_message_list_audit_test.c:727; test_xui/xui_message_list_audit_test.c:885

## xuiMessageListGetNodeRect
- 位置: xui.h:9318  已注释: 否
- 签名: `XUI_API xui_rect_t xuiMessageListGetNodeRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_message_list.c:1914（体 8 行）
- 用法: test_xui/xui_message_list_audit_test.c:712; test_xui/xui_message_list_test.c:160; test_xui/xui_message_list_test.c:217

## xuiMessageListGetBubbleRect
- 位置: xui.h:9319  已注释: 否
- 签名: `XUI_API xui_rect_t xuiMessageListGetBubbleRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_message_list.c:1923（体 8 行）
- 用法: examples/xui_messagelist/main.c:208; test_xui/xui_message_list_audit_test.c:713; test_xui/xui_message_list_test.c:162

## xuiMessageListSetScroll
- 位置: xui.h:9320  已注释: 否
- 签名: `XUI_API int xuiMessageListSetScroll(xui_widget pWidget, float fOffsetY);`
- 实现: src/xui_message_list.c:1932（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_message_list_audit_test.c:581; test_xui/xui_message_list_audit_test.c:707; test_xui/xui_message_list_audit_test.c:749

## xuiMessageListGetScroll
- 位置: xui.h:9321  已注释: 否
- 签名: `XUI_API float xuiMessageListGetScroll(xui_widget pWidget);`
- 实现: src/xui_message_list.c:1945（体 5 行）
- 用法: test_xui/xui_message_list_test.c:235; test_xui/xui_message_list_test.c:277; test_xui/xui_message_list_test.c:283

## xuiMessageListScrollBy
- 位置: xui.h:9322  已注释: 否
- 签名: `XUI_API int xuiMessageListScrollBy(xui_widget pWidget, float fDeltaY);`
- 实现: src/xui_message_list.c:1951（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_message_list_audit_test.c:714

## xuiMessageListEnsureVisible
- 位置: xui.h:9323  已注释: 否
- 签名: `XUI_API int xuiMessageListEnsureVisible(xui_widget pWidget, int iIndex);`
- 实现: src/xui_message_list.c:1958（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_message_list_test.c:292

## xuiMessageListScrollToEnd
- 位置: xui.h:9324  已注释: 否
- 签名: `XUI_API int xuiMessageListScrollToEnd(xui_widget pWidget);`
- 实现: src/xui_message_list.c:1974（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_message_list_audit_test.c:807; test_xui/xui_message_list_test.c:344

## xuiMessageListSetAutoScroll
- 位置: xui.h:9325  已注释: 否
- 签名: `XUI_API int xuiMessageListSetAutoScroll(xui_widget pWidget, int bAutoScroll);`
- 实现: src/xui_message_list.c:1987（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_message_list_audit_test.c:147; test_xui/xui_message_list_audit_test.c:344; test_xui/xui_message_list_audit_test.c:615

## xuiMessageListGetAutoScroll
- 位置: xui.h:9326  已注释: 否
- 签名: `XUI_API int xuiMessageListGetAutoScroll(xui_widget pWidget);`
- 实现: src/xui_message_list.c:1995（体 5 行）

## xuiMessageListSetFont
- 位置: xui.h:9327  已注释: 否
- 签名: `XUI_API int xuiMessageListSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_message_list.c:2001（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_message_list_audit_test.c:616; test_xui/xui_message_list_audit_test.c:786; test_xui/xui_message_list_audit_test.c:869

## xuiMessageListGetFont
- 位置: xui.h:9328  已注释: 否
- 签名: `XUI_API xui_font xuiMessageListGetFont(xui_widget pWidget);`
- 实现: src/xui_message_list.c:2010（体 5 行）

## xuiMessageListSetMetrics
- 位置: xui.h:9329  已注释: 否
- 签名: `XUI_API int xuiMessageListSetMetrics(xui_widget pWidget, const xui_message_list_metrics_t* pMetrics);`
- 实现: src/xui_message_list.c:2016（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_message_list_audit_test.c:617; test_xui/xui_message_list_audit_test.c:795; test_xui/xui_message_list_audit_test.c:803

## xuiMessageListGetMetrics
- 位置: xui.h:9330  已注释: 否
- 签名: `XUI_API int xuiMessageListGetMetrics(xui_widget pWidget, xui_message_list_metrics_t* pMetrics);`
- 实现: src/xui_message_list.c:2025（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_message_list_test.c:159

## xuiMessageListSetColors
- 位置: xui.h:9331  已注释: 否
- 签名: `XUI_API int xuiMessageListSetColors(xui_widget pWidget, const xui_message_list_colors_t* pColors);`
- 实现: src/xui_message_list.c:2033（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_collections_message_test.c:67

## xuiMessageListGetColors
- 位置: xui.h:9332  已注释: 否
- 签名: `XUI_API int xuiMessageListGetColors(xui_widget pWidget, xui_message_list_colors_t* pColors);`
- 实现: src/xui_message_list.c:2041（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_collections_message_test.c:69

## xuiMessageListClearTextSelection
- 位置: xui.h:9333  已注释: 否
- 签名: `XUI_API int xuiMessageListClearTextSelection(xui_widget pWidget);`
- 实现: src/xui_message_list.c:2049（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_message_list_audit_test.c:748; test_xui/xui_message_list_audit_test.c:792

## xuiMessageListCopySelection
- 位置: xui.h:9334  已注释: 否
- 签名: `XUI_API int xuiMessageListCopySelection(xui_widget pWidget);`
- 实现: src/xui_message_list.c:2078（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_message_list_audit_test.c:165; test_xui/xui_message_list_audit_test.c:363; test_xui/xui_message_list_audit_test.c:411

## xuiMessageListGetSelectedText
- 位置: xui.h:9335  已注释: 否
- 签名: `XUI_API int xuiMessageListGetSelectedText(xui_widget pWidget, char* sBuffer, int iCapacity);`
- 实现: src/xui_message_list.c:2059（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_message_list_audit_test.c:163; test_xui/xui_message_list_audit_test.c:201; test_xui/xui_message_list_audit_test.c:203

## xuiMessageListExportText
- 位置: xui.h:9336  已注释: 否
- 签名: `XUI_API int xuiMessageListExportText(xui_widget pWidget, char* sBuffer, int iCapacity);`
- 实现: src/xui_message_list.c:2129（体 32 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_message_list_test.c:239

## xuiMessageListImportText
- 位置: xui.h:9337  已注释: 否
- 签名: `XUI_API int xuiMessageListImportText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_message_list.c:2222（体 117 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_message_list_test.c:243; test_xui/xui_message_list_test.c:247

## xuiMessageListSaveFile
- 位置: xui.h:9338  已注释: 否
- 签名: `XUI_API int xuiMessageListSaveFile(xui_widget pWidget, const char* sPath);`
- 实现: src/xui_message_list.c:2340（体 28 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_ERROR_FILE_NOT_FOUND, XUI_ERROR, XUI_OK
- 用法: test_xui/xui_message_list_test.c:250

## xuiMessageListLoadFile
- 位置: xui.h:9339  已注释: 否
- 签名: `XUI_API int xuiMessageListLoadFile(xui_widget pWidget, const char* sPath);`
- 实现: src/xui_message_list.c:2369（体 34 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_FILE_NOT_FOUND, XUI_ERROR, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_message_list_test.c:254

## xuiMessageListGetSelectCount
- 位置: xui.h:9340  已注释: 否
- 签名: `XUI_API int xuiMessageListGetSelectCount(xui_widget pWidget);`
- 实现: src/xui_message_list.c:2404（体 5 行）

## xuiMessageListGetClickCount
- 位置: xui.h:9341  已注释: 否
- 签名: `XUI_API int xuiMessageListGetClickCount(xui_widget pWidget);`
- 实现: src/xui_message_list.c:2410（体 5 行）

## xuiMessageListGetChangeCount
- 位置: xui.h:9342  已注释: 否
- 签名: `XUI_API int xuiMessageListGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_message_list.c:2416（体 5 行）

