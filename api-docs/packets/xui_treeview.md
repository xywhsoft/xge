# 草稿包：xui.h / treeview（53 条 API）

> 生成 2026-09-10 03:08 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiTreeViewGetType
- 位置: xui.h:10086  已注释: 否
- 签名: `XUI_API xui_widget_type xuiTreeViewGetType(xui_context pContext);`
- 实现: src/xui_tree_view.c:2003（体 39 行）
- 返回码: NULL

## xuiTreeViewCreate
- 位置: xui.h:10087  已注释: 否
- 签名: `XUI_API int xuiTreeViewCreate(xui_context pContext, xui_widget* ppWidget, const xui_tree_view_desc_t* pDesc);`
- 实现: src/xui_tree_view.c:2043（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch147_main1.c:13; examples/tutorial_capture/ch207_main1.c:29; examples/xui_treeview/main.c:250

## xuiTreeViewSetSelect
- 位置: xui.h:10088  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetSelect(xui_widget pWidget, xui_tree_view_select_proc onSelect, void* pUser);`
- 实现: src/xui_tree_view.c:2058（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_treeview/main.c:252; test_xui/xui_tree_view_test.c:375

## xuiTreeViewSetContextMenu
- 位置: xui.h:10089  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetContextMenu(xui_widget pWidget, xui_tree_view_context_proc onContext, void* pUser);`
- 实现: src/xui_tree_view.c:2067（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_tree_view_test.c:379

## xuiTreeViewSetItemRenderer
- 位置: xui.h:10090  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetItemRenderer(xui_widget pWidget, xui_tree_view_item_proc onRender, void* pUser);`
- 实现: src/xui_tree_view.c:2076（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_treeview/main.c:270; test_xui/xui_tree_view_test.c:381

## xuiTreeViewSetAdapter
- 位置: xui.h:10091  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetAdapter(xui_widget pWidget, xui_tree_view_count_proc onCount, xui_tree_view_node_proc onNode, void* pUser);`
- 实现: src/xui_tree_view.c:2085（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_INVALID_STATE
- 用法: test_xui/xui_tree_adapter_lifetime_test.c:25; test_xui/xui_tree_adapter_lifetime_test.c:78; test_xui/xui_tree_view_test.c:233

## xuiTreeViewRefreshAdapter
- 位置: xui.h:10094  已注释: 是
- 签名: `XUI_API int xuiTreeViewRefreshAdapter(xui_widget pWidget);`
- 既有注释: /* Adapter reads observe the old tree until commit. Reentrant refresh/rebind or * a callback invalidating the tree aborts with XUI_ERROR_INVALID_STATE. */
- 实现: src/xui_tree_view.c:2166（体 9 行）
- 用法: test_xui/xui_tree_adapter_lifetime_test.c:24; test_xui/xui_tree_view_test.c:238; test_xui/xui_tree_view_test.c:241

## xuiTreeViewClear
- 位置: xui.h:10095  已注释: 否
- 签名: `XUI_API int xuiTreeViewClear(xui_widget pWidget);`
- 实现: src/xui_tree_view.c:2176（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_tree_adapter_lifetime_test.c:23; test_xui/xui_tree_view_test.c:224

## xuiTreeViewSetNodes
- 位置: xui.h:10096  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetNodes(xui_widget pWidget, const xui_tree_view_node_t* pNodes, int iCount);`
- 实现: src/xui_tree_view.c:2191（体 40 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_tree_adapter_lifetime_test.c:22; test_xui/xui_tree_adapter_lifetime_test.c:77; test_xui/xui_tree_view_test.c:177

## xuiTreeViewAddNode
- 位置: xui.h:10097  已注释: 否
- 签名: `XUI_API int xuiTreeViewAddNode(xui_widget pWidget, int iId, int iParentId, const char* sText);`
- 实现: src/xui_tree_view.c:2232（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch147_main1.c:20; examples/tutorial_capture/ch147_main1.c:21; examples/tutorial_capture/ch147_main1.c:22

## xuiTreeViewGetNodeCount
- 位置: xui.h:10098  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetNodeCount(xui_widget pWidget);`
- 实现: src/xui_tree_view.c:2248（体 5 行）
- 用法: test_xui/xui_tree_adapter_lifetime_test.c:54; test_xui/xui_tree_adapter_lifetime_test.c:80; test_xui/xui_tree_adapter_lifetime_test.c:89

## xuiTreeViewGetNode
- 位置: xui.h:10099  已注释: 否
- 签名: `XUI_API const xui_tree_view_node_t* xuiTreeViewGetNode(xui_widget pWidget, int iIndex);`
- 实现: src/xui_tree_view.c:2254（体 6 行）
- 返回码: NULL
- 用法: test_xui/xui_tree_view_test.c:214

## xuiTreeViewGetNodeById
- 位置: xui.h:10100  已注释: 否
- 签名: `XUI_API const xui_tree_view_node_t* xuiTreeViewGetNodeById(xui_widget pWidget, int iNodeId);`
- 实现: src/xui_tree_view.c:2261（体 6 行）
- 用法: examples/xui_treeview/main.c:425; examples/xui_treeview/main.c:428; test_xui/xui_tree_adapter_lifetime_test.c:55

## xuiTreeViewFindNode
- 位置: xui.h:10101  已注释: 否
- 签名: `XUI_API int xuiTreeViewFindNode(xui_widget pWidget, int iNodeId);`
- 实现: src/xui_tree_view.c:2268（体 5 行）
- 用法: test_xui/xui_tree_view_test.c:180; test_xui/xui_tree_view_test.c:213; test_xui/xui_tree_view_test.c:215

## xuiTreeViewSetNodeExpanded
- 位置: xui.h:10102  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetNodeExpanded(xui_widget pWidget, int iNodeId, int bExpanded);`
- 实现: src/xui_tree_view.c:2274（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_treeview/main.c:293; examples/xui_treeview/main.c:294; examples/xui_treeview/main.c:312

## xuiTreeViewGetNodeExpanded
- 位置: xui.h:10103  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetNodeExpanded(xui_widget pWidget, int iNodeId);`
- 实现: src/xui_tree_view.c:2284（体 6 行）
- 用法: examples/xui_treeview/main.c:443; test_xui/xui_tree_view_test.c:491; test_xui/xui_tree_view_test.c:496

## xuiTreeViewToggleNode
- 位置: xui.h:10104  已注释: 否
- 签名: `XUI_API int xuiTreeViewToggleNode(xui_widget pWidget, int iNodeId);`
- 实现: src/xui_tree_view.c:2291（体 4 行）

## xuiTreeViewSetNodeEnabled
- 位置: xui.h:10105  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetNodeEnabled(xui_widget pWidget, int iNodeId, int bEnabled);`
- 实现: src/xui_tree_view.c:2296（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_treeview/main.c:295; examples/xui_treeview/main.c:317; test_xui/xui_tree_view_test.c:188

## xuiTreeViewGetNodeEnabled
- 位置: xui.h:10106  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetNodeEnabled(xui_widget pWidget, int iNodeId);`
- 实现: src/xui_tree_view.c:2308（体 6 行）
- 用法: examples/xui_treeview/main.c:420; test_xui/xui_tree_view_test.c:434

## xuiTreeViewSetNodeChecked
- 位置: xui.h:10107  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetNodeChecked(xui_widget pWidget, int iNodeId, int bChecked);`
- 实现: src/xui_tree_view.c:2315（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_treeview/main.c:296; examples/xui_treeview/main.c:314; examples/xui_treeview/main.c:319

## xuiTreeViewGetNodeChecked
- 位置: xui.h:10108  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetNodeChecked(xui_widget pWidget, int iNodeId);`
- 实现: src/xui_tree_view.c:2328（体 6 行）
- 用法: examples/xui_treeview/main.c:421; examples/xui_treeview/main.c:442; test_xui/xui_tree_view_test.c:435

## xuiTreeViewSetNodeDecorations
- 位置: xui.h:10109  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetNodeDecorations(xui_widget pWidget, int iNodeId, int bIcon, int bCheck);`
- 实现: src/xui_tree_view.c:2335（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_treeview/main.c:297; examples/xui_treeview/main.c:315; examples/xui_treeview/main.c:316

## xuiTreeViewSetSelected
- 位置: xui.h:10110  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetSelected(xui_widget pWidget, int iNodeId);`
- 实现: src/xui_tree_view.c:2348（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_treeview/main.c:298; examples/xui_treeview/main.c:320; examples/xui_treeview/main.c:350

## xuiTreeViewGetSelected
- 位置: xui.h:10111  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetSelected(xui_widget pWidget);`
- 实现: src/xui_tree_view.c:2357（体 5 行）
- 用法: examples/xui_treeview/main.c:419; examples/xui_treeview/main.c:441; examples/xui_treeview/main.c:446

## xuiTreeViewGetVisibleCount
- 位置: xui.h:10112  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetVisibleCount(xui_widget pWidget);`
- 实现: src/xui_tree_view.c:2363（体 5 行）
- 用法: examples/xui_treeview/main.c:417; test_xui/xui_tree_view_test.c:131; test_xui/xui_tree_view_test.c:272

## xuiTreeViewGetVisibleNodeId
- 位置: xui.h:10113  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetVisibleNodeId(xui_widget pWidget, int iVisible);`
- 实现: src/xui_tree_view.c:2369（体 6 行）
- 用法: examples/xui_treeview/main.c:418; test_xui/xui_tree_view_test.c:133; test_xui/xui_tree_view_test.c:432

## xuiTreeViewGetVisibleNode
- 位置: xui.h:10114  已注释: 否
- 签名: `XUI_API const xui_tree_view_node_t* xuiTreeViewGetVisibleNode(xui_widget pWidget, int iVisible);`
- 实现: src/xui_tree_view.c:2376（体 6 行）
- 返回码: NULL
- 用法: test_xui/xui_tree_view_test.c:135

## xuiTreeViewGetVisibleIndexOfId
- 位置: xui.h:10115  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetVisibleIndexOfId(xui_widget pWidget, int iNodeId);`
- 实现: src/xui_tree_view.c:2383（体 5 行）
- 用法: examples/xui_treeview/main.c:393; test_xui/xui_tree_view_test.c:134; test_xui/xui_tree_view_test.c:187

## xuiTreeViewSetFont
- 位置: xui.h:10116  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_tree_view.c:2389（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTreeViewGetFont
- 位置: xui.h:10117  已注释: 否
- 签名: `XUI_API xui_font xuiTreeViewGetFont(xui_widget pWidget);`
- 实现: src/xui_tree_view.c:2397（体 5 行）

## xuiTreeViewSetMetrics
- 位置: xui.h:10118  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetMetrics(xui_widget pWidget, float fItemHeight, float fIndent, float fPadding, float fBorderWidth);`
- 实现: src/xui_tree_view.c:2403（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTreeViewGetMetrics
- 位置: xui.h:10119  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetMetrics(xui_widget pWidget, float* pItemHeight, float* pIndent, float* pPadding, float* pBorderWidth);`
- 实现: src/xui_tree_view.c:2418（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTreeViewSetScroll
- 位置: xui.h:10120  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetScroll(xui_widget pWidget, float fOffsetY);`
- 实现: src/xui_tree_view.c:2429（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_treeview/main.c:351

## xuiTreeViewGetScroll
- 位置: xui.h:10121  已注释: 否
- 签名: `XUI_API float xuiTreeViewGetScroll(xui_widget pWidget);`
- 实现: src/xui_tree_view.c:2436（体 10 行）
- 用法: examples/xui_treeview/main.c:444; examples/xui_treeview/main.c:539; test_xui/xui_tree_view_test.c:280

## xuiTreeViewEnsureVisible
- 位置: xui.h:10122  已注释: 否
- 签名: `XUI_API int xuiTreeViewEnsureVisible(xui_widget pWidget, int iNodeId);`
- 实现: src/xui_tree_view.c:2447（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_treeview/main.c:431; test_xui/xui_tree_view_test.c:275; test_xui/xui_tree_view_test.c:526

## xuiTreeViewSetScrollbarMode
- 位置: xui.h:10123  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetScrollbarMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_tree_view.c:2456（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_treeview/main.c:332

## xuiTreeViewGetScrollbarMode
- 位置: xui.h:10124  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetScrollbarMode(xui_widget pWidget);`
- 实现: src/xui_tree_view.c:2463（体 5 行）

## xuiTreeViewSetColors
- 位置: xui.h:10125  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iBorder, uint32_t iFocus, uint32_t iRow, uint32_t iHover, uint32_t iSelected, uint32_t iText, uint32_t iDisabledText);`
- 实现: src/xui_tree_view.c:2469（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_treeview/main.c:254; test_xui/xui_tree_view_test.c:383

## xuiTreeViewGetColors
- 位置: xui.h:10126  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pBorder, uint32_t* pFocus, uint32_t* pRow, uint32_t* pHover, uint32_t* pSelected, uint32_t* pText, uint32_t* pDisabledText);`
- 实现: src/xui_tree_view.c:2484（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTreeViewSetDecorationColors
- 位置: xui.h:10127  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetDecorationColors(xui_widget pWidget, uint32_t iExpander, uint32_t iIcon, uint32_t iCheck);`
- 实现: src/xui_tree_view.c:2499（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTreeViewGetDecorationColors
- 位置: xui.h:10128  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetDecorationColors(xui_widget pWidget, uint32_t* pExpander, uint32_t* pIcon, uint32_t* pCheck);`
- 实现: src/xui_tree_view.c:2509（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTreeViewSetScrollbarColors
- 位置: xui.h:10129  已注释: 否
- 签名: `XUI_API int xuiTreeViewSetScrollbarColors(xui_widget pWidget, uint32_t iTrack, uint32_t iThumb, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);`
- 实现: src/xui_tree_view.c:2519（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_treeview/main.c:263; test_xui/xui_style_collections_tree_test.c:67

## xuiTreeViewGetScrollbarColors
- 位置: xui.h:10130  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetScrollbarColors(xui_widget pWidget, uint32_t* pTrack, uint32_t* pThumb, uint32_t* pHover, uint32_t* pActive, uint32_t* pFocus, uint32_t* pDisabled);`
- 实现: src/xui_tree_view.c:2537（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTreeViewGetFrameWidget
- 位置: xui.h:10131  已注释: 否
- 签名: `XUI_API xui_widget xuiTreeViewGetFrameWidget(xui_widget pWidget);`
- 实现: src/xui_tree_view.c:2550（体 5 行）
- 用法: examples/xui_treeview/main.c:412; examples/xui_treeview/main.c:413; examples/xui_treeview/main.c:414

## xuiTreeViewGetViewportWidget
- 位置: xui.h:10132  已注释: 否
- 签名: `XUI_API xui_widget xuiTreeViewGetViewportWidget(xui_widget pWidget);`
- 实现: src/xui_tree_view.c:2556（体 5 行）
- 用法: test_xui/xui_tree_view_test.c:437

## xuiTreeViewGetModel
- 位置: xui.h:10133  已注释: 否
- 签名: `XUI_API xui_scroll_model_t* xuiTreeViewGetModel(xui_widget pWidget);`
- 实现: src/xui_tree_view.c:2562（体 5 行）

## xuiTreeViewGetViewportRect
- 位置: xui.h:10134  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTreeViewGetViewportRect(xui_widget pWidget);`
- 实现: src/xui_tree_view.c:2568（体 5 行）
- 用法: test_xui/xui_tree_view_test.c:531; test_xui/xui_tree_view_test.c:531

## xuiTreeViewGetItemRect
- 位置: xui.h:10135  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTreeViewGetItemRect(xui_widget pWidget, int iVisible);`
- 实现: src/xui_tree_view.c:2574（体 15 行）
- 用法: examples/xui_treeview/main.c:396; test_xui/xui_tree_view_test.c:300; test_xui/xui_tree_view_test.c:309

## xuiTreeViewGetItemAt
- 位置: xui.h:10136  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetItemAt(xui_widget pWidget, float fX, float fY);`
- 实现: src/xui_tree_view.c:2590（体 18 行）

## xuiTreeViewGetHoverIndex
- 位置: xui.h:10137  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetHoverIndex(xui_widget pWidget);`
- 实现: src/xui_tree_view.c:2609（体 5 行）

## xuiTreeViewGetFocusIndex
- 位置: xui.h:10138  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetFocusIndex(xui_widget pWidget);`
- 实现: src/xui_tree_view.c:2615（体 5 行）
- 用法: test_xui/xui_tree_view_test.c:187; test_xui/xui_tree_view_test.c:239

## xuiTreeViewGetSelectCount
- 位置: xui.h:10139  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetSelectCount(xui_widget pWidget);`
- 实现: src/xui_tree_view.c:2621（体 5 行）

## xuiTreeViewGetChangeCount
- 位置: xui.h:10140  已注释: 否
- 签名: `XUI_API int xuiTreeViewGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_tree_view.c:2627（体 5 行）
- 用法: test_xui/xui_tree_view_test.c:201; test_xui/xui_tree_view_test.c:202

