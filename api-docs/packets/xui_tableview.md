# 草稿包：xui.h / tableview（60 条 API）

> 生成 2026-09-10 03:06 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiTableViewGetType
- 位置: xui.h:9925  已注释: 否
- 签名: `XUI_API xui_widget_type xuiTableViewGetType(xui_context pContext);`
- 实现: src/xui_table_view.c:2728（体 40 行）
- 返回码: NULL
- 用法: test_xui/xui_code_edit_test.c:1043; test_xui/xui_code_edit_test.c:1108; test_xui/xui_style_collections_table_test.c:53

## xuiTableViewCreate
- 位置: xui.h:9926  已注释: 否
- 签名: `XUI_API int xuiTableViewCreate(xui_context pContext, xui_widget* ppWidget, const xui_table_view_desc_t* pDesc);`
- 实现: src/xui_table_view.c:2769（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch148_main1.c:13; examples/xui_tableview/main.c:273; test_xui/xui_table_provider_lifetime_test.c:167

## xuiTableViewSetColumns
- 位置: xui.h:9927  已注释: 否
- 签名: `XUI_API int xuiTableViewSetColumns(xui_widget pWidget, const xui_table_view_column_t* arrColumns, int iCount);`
- 实现: src/xui_table_view.c:3454（体 9 行）
- 用法: examples/tutorial_capture/ch148_main1.c:26; test_xui/xui_table_provider_lifetime_test.c:58; test_xui/xui_table_provider_lifetime_test.c:184

## xuiTableViewSetRows
- 位置: xui.h:9928  已注释: 否
- 签名: `XUI_API int xuiTableViewSetRows(xui_widget pWidget, const xui_table_view_row_t* arrRows, int iCount);`
- 实现: src/xui_table_view.c:3464（体 9 行）
- 用法: test_xui/xui_table_provider_lifetime_test.c:62; test_xui/xui_table_provider_lifetime_test.c:185; test_xui/xui_table_provider_lifetime_test.c:218

## xuiTableViewSetAdapter
- 位置: xui.h:9929  已注释: 否
- 签名: `XUI_API int xuiTableViewSetAdapter(xui_widget pWidget, xui_table_view_count_proc onCount, xui_table_view_cell_proc onCell, void* pUser);`
- 实现: src/xui_table_view.c:3474（体 9 行）
- 用法: test_xui/xui_table_provider_lifetime_test.c:64; test_xui/xui_table_provider_lifetime_test.c:186; test_xui/xui_table_provider_lifetime_test.c:219

## xuiTableViewRefreshAdapter
- 位置: xui.h:9930  已注释: 否
- 签名: `XUI_API int xuiTableViewRefreshAdapter(xui_widget pWidget);`
- 实现: src/xui_table_view.c:2885（体 4 行）
- 用法: test_xui/xui_table_provider_lifetime_test.c:222; test_xui/xui_table_view_index_test.c:213; test_xui/xui_table_view_index_test.c:637

## xuiTableViewSetSort
- 位置: xui.h:9931  已注释: 否
- 签名: `XUI_API int xuiTableViewSetSort(xui_widget pWidget, xui_table_view_sort_proc onSort, void* pUser);`
- 实现: src/xui_table_view.c:2890（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_table_view_index_test.c:573; test_xui/xui_table_view_test.c:297

## xuiTableViewSetSelect
- 位置: xui.h:9932  已注释: 否
- 签名: `XUI_API int xuiTableViewSetSelect(xui_widget pWidget, xui_table_view_select_proc onSelect, void* pUser);`
- 实现: src/xui_table_view.c:2899（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_tableview/main.c:275; test_xui/xui_grid_focus_test.c:185; test_xui/xui_table_provider_lifetime_test.c:317

## xuiTableViewSetColumnResize
- 位置: xui.h:9933  已注释: 否
- 签名: `XUI_API int xuiTableViewSetColumnResize(xui_widget pWidget, xui_table_view_column_resize_proc onResize, void* pUser);`
- 实现: src/xui_table_view.c:2908（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_table_provider_lifetime_test.c:319; test_xui/xui_table_view_index_test.c:574; test_xui/xui_table_view_test.c:295

## xuiTableViewSetHover
- 位置: xui.h:9934  已注释: 否
- 签名: `XUI_API int xuiTableViewSetHover(xui_widget pWidget, xui_table_view_hover_proc onHover, void* pUser);`
- 实现: src/xui_table_view.c:2917（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_table_provider_lifetime_test.c:318; test_xui/xui_table_view_test.c:293

## xuiTableViewSetContextMenu
- 位置: xui.h:9935  已注释: 否
- 签名: `XUI_API int xuiTableViewSetContextMenu(xui_widget pWidget, xui_table_view_context_proc onContext, void* pUser);`
- 实现: src/xui_table_view.c:2926（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_table_provider_lifetime_test.c:320; test_xui/xui_table_view_test.c:299

## xuiTableViewSetMergeProvider
- 位置: xui.h:9936  已注释: 否
- 签名: `XUI_API int xuiTableViewSetMergeProvider(xui_widget pWidget, xui_table_view_merge_proc onMerge, void* pUser);`
- 实现: src/xui_table_view.c:2935（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_tableview/main.c:276; test_xui/xui_table_provider_lifetime_test.c:187; test_xui/xui_table_view_index_test.c:119

## xuiTableViewSetHeaderRenderer
- 位置: xui.h:9937  已注释: 否
- 签名: `XUI_API int xuiTableViewSetHeaderRenderer(xui_widget pWidget, xui_table_view_header_renderer_proc onRender, void* pUser);`
- 实现: src/xui_table_view.c:2945（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_table_provider_lifetime_test.c:189; test_xui/xui_table_view_test.c:303

## xuiTableViewSetCellRenderer
- 位置: xui.h:9938  已注释: 否
- 签名: `XUI_API int xuiTableViewSetCellRenderer(xui_widget pWidget, xui_table_view_cell_renderer_proc onRender, void* pUser);`
- 实现: src/xui_table_view.c:2955（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_tableview/main.c:280; test_xui/xui_table_provider_lifetime_test.c:66; test_xui/xui_table_provider_lifetime_test.c:188

## xuiTableViewSetColumnFormatter
- 位置: xui.h:9939  已注释: 否
- 签名: `XUI_API int xuiTableViewSetColumnFormatter(xui_widget pWidget, int iColumn, xui_table_view_format_proc onFormat, void* pUser);`
- 实现: src/xui_table_view.c:2965（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_table_provider_lifetime_test.c:67

## xuiTableViewSetFont
- 位置: xui.h:9940  已注释: 否
- 签名: `XUI_API int xuiTableViewSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_table_view.c:2975（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTableViewGetFont
- 位置: xui.h:9941  已注释: 否
- 签名: `XUI_API xui_font xuiTableViewGetFont(xui_widget pWidget);`
- 实现: src/xui_table_view.c:2983（体 5 行）
- 用法: test_xui/xui_property_grid_test.c:480

## xuiTableViewSetDefaultMetrics
- 位置: xui.h:9942  已注释: 否
- 签名: `XUI_API int xuiTableViewSetDefaultMetrics(xui_widget pWidget, float fColumnWidth, float fRowHeight, float fHeaderHeight);`
- 实现: src/xui_table_view.c:3484（体 9 行）
- 用法: test_xui/xui_table_provider_lifetime_test.c:220; test_xui/xui_table_view_index_test.c:308; test_xui/xui_table_view_index_test.c:619

## xuiTableViewGetDefaultMetrics
- 位置: xui.h:9943  已注释: 否
- 签名: `XUI_API int xuiTableViewGetDefaultMetrics(xui_widget pWidget, float* pColumnWidth, float* pRowHeight, float* pHeaderHeight);`
- 实现: src/xui_table_view.c:3010（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTableViewSetSelectionMode
- 位置: xui.h:9944  已注释: 否
- 签名: `XUI_API int xuiTableViewSetSelectionMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_table_view.c:3020（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_tableview/main.c:278

## xuiTableViewGetSelectionMode
- 位置: xui.h:9945  已注释: 否
- 签名: `XUI_API int xuiTableViewGetSelectionMode(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3032（体 5 行）
- 用法: test_xui/xui_table_view_test.c:377

## xuiTableViewSetSelectedRow
- 位置: xui.h:9946  已注释: 否
- 签名: `XUI_API int xuiTableViewSetSelectedRow(xui_widget pWidget, int iRow);`
- 实现: src/xui_table_view.c:3494（体 9 行）
- 用法: test_xui/xui_table_provider_lifetime_test.c:227; test_xui/xui_table_view_test.c:376

## xuiTableViewGetSelectedRow
- 位置: xui.h:9947  已注释: 否
- 签名: `XUI_API int xuiTableViewGetSelectedRow(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3046（体 5 行）
- 用法: examples/xui_tableview/main.c:423; test_xui/xui_table_provider_lifetime_test.c:82; test_xui/xui_table_provider_lifetime_test.c:351

## xuiTableViewSetSelectedCell
- 位置: xui.h:9948  已注释: 否
- 签名: `XUI_API int xuiTableViewSetSelectedCell(xui_widget pWidget, int iRow, int iColumn);`
- 实现: src/xui_table_view.c:3504（体 9 行）
- 用法: test_xui/xui_grid_focus_test.c:228; test_xui/xui_grid_focus_test.c:268; test_xui/xui_grid_focus_test.c:303

## xuiTableViewGetSelectedCell
- 位置: xui.h:9949  已注释: 否
- 签名: `XUI_API int xuiTableViewGetSelectedCell(xui_widget pWidget, int* pRow, int* pColumn);`
- 实现: src/xui_table_view.c:3060（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_grid_focus_test.c:270; test_xui/xui_grid_focus_test.c:274; test_xui/xui_grid_focus_test.c:276

## xuiTableViewSetOffset
- 位置: xui.h:9950  已注释: 否
- 签名: `XUI_API int xuiTableViewSetOffset(xui_widget pWidget, float fOffsetX, float fOffsetY);`
- 实现: src/xui_table_view.c:3069（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_table_view_index_test.c:486; test_xui/xui_table_view_index_test.c:496; test_xui/xui_table_view_index_test.c:575

## xuiTableViewGetOffset
- 位置: xui.h:9951  已注释: 否
- 签名: `XUI_API int xuiTableViewGetOffset(xui_widget pWidget, float* pOffsetX, float* pOffsetY);`
- 实现: src/xui_table_view.c:3076（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_tablegrid/main.c:564; examples/xui_tableview/main.c:429; test_xui/xui_table_view_index_test.c:527

## xuiTableViewSetScrollbarMode
- 位置: xui.h:9952  已注释: 否
- 签名: `XUI_API int xuiTableViewSetScrollbarMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_table_view.c:3083（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTableViewGetScrollbarMode
- 位置: xui.h:9953  已注释: 否
- 签名: `XUI_API int xuiTableViewGetScrollbarMode(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3090（体 5 行）

## xuiTableViewGetColumnCount
- 位置: xui.h:9954  已注释: 否
- 签名: `XUI_API int xuiTableViewGetColumnCount(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3096（体 5 行）
- 用法: examples/xui_propertygrid/main.c:506; examples/xui_tablegrid/main.c:543; test_xui/xui_code_edit_test.c:1044

## xuiTableViewGetColumn
- 位置: xui.h:9955  已注释: 否
- 签名: `XUI_API const xui_table_view_column_t* xuiTableViewGetColumn(xui_widget pWidget, int iColumn);`
- 实现: src/xui_table_view.c:3102（体 6 行）
- 返回码: NULL

## xuiTableViewGetColumnWidth
- 位置: xui.h:9956  已注释: 否
- 签名: `XUI_API float xuiTableViewGetColumnWidth(xui_widget pWidget, int iColumn);`
- 实现: src/xui_table_view.c:3109（体 5 行）
- 用法: examples/xui_tableview/main.c:433; examples/xui_tableview/main.c:438; test_xui/xui_table_provider_lifetime_test.c:257

## xuiTableViewSetColumnWidth
- 位置: xui.h:9957  已注释: 否
- 签名: `XUI_API int xuiTableViewSetColumnWidth(xui_widget pWidget, int iColumn, float fWidth);`
- 实现: src/xui_table_view.c:3514（体 9 行）
- 用法: test_xui/xui_table_provider_lifetime_test.c:221; test_xui/xui_table_view_index_test.c:562

## xuiTableViewGetRowCount
- 位置: xui.h:9958  已注释: 否
- 签名: `XUI_API int xuiTableViewGetRowCount(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3524（体 9 行）
- 用法: test_xui/xui_code_edit_test.c:1124; test_xui/xui_table_grid_test.c:535; test_xui/xui_table_provider_lifetime_test.c:83

## xuiTableViewGetFirstVisible
- 位置: xui.h:9959  已注释: 否
- 签名: `XUI_API int xuiTableViewGetFirstVisible(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3143（体 5 行）
- 用法: test_xui/xui_table_view_index_test.c:499; test_xui/xui_table_view_index_test.c:639; test_xui/xui_table_view_test.c:390

## xuiTableViewGetPaintVisibleCount
- 位置: xui.h:9960  已注释: 否
- 签名: `XUI_API int xuiTableViewGetPaintVisibleCount(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3149（体 5 行）
- 用法: test_xui/xui_table_view_index_test.c:500; test_xui/xui_table_view_index_test.c:500; test_xui/xui_table_view_index_test.c:631

## xuiTableViewGetActiveCell
- 位置: xui.h:9961  已注释: 否
- 签名: `XUI_API int xuiTableViewGetActiveCell(xui_widget pWidget, int* pRow, int* pColumn);`
- 实现: src/xui_table_view.c:3155（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTableViewGetHoverCell
- 位置: xui.h:9962  已注释: 否
- 签名: `XUI_API int xuiTableViewGetHoverCell(xui_widget pWidget, int* pRow, int* pColumn);`
- 实现: src/xui_table_view.c:3164（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_table_view_index_test.c:522

## xuiTableViewGetCellContentRect
- 位置: xui.h:9963  已注释: 否
- 签名: `XUI_API int xuiTableViewGetCellContentRect(xui_widget pWidget, int iRow, int iColumn, xui_rect_t* pRect);`
- 实现: src/xui_table_view.c:3534（体 9 行）
- 用法: examples/xui_tableview/main.c:432; examples/xui_tableview/main.c:437; test_xui/xui_table_provider_lifetime_test.c:68

## xuiTableViewGetCellRect
- 位置: xui.h:9964  已注释: 否
- 签名: `XUI_API int xuiTableViewGetCellRect(xui_widget pWidget, int iRow, int iColumn, xui_rect_t* pRect);`
- 实现: src/xui_table_view.c:3544（体 9 行）
- 用法: examples/xui_tablegrid/main.c:526; examples/xui_tableview/main.c:382; test_xui/xui_property_grid_test.c:505

## xuiTableViewGetFrameWidget
- 位置: xui.h:9965  已注释: 否
- 签名: `XUI_API xui_widget xuiTableViewGetFrameWidget(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3216（体 5 行）
- 用法: examples/xui_tablegrid/main.c:543; examples/xui_tableview/main.c:418; examples/xui_tableview/main.c:419

## xuiTableViewGetViewportWidget
- 位置: xui.h:9966  已注释: 否
- 签名: `XUI_API xui_widget xuiTableViewGetViewportWidget(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3222（体 5 行）
- 用法: test_xui/xui_table_provider_lifetime_test.c:191; test_xui/xui_table_provider_lifetime_test.c:193; test_xui/xui_table_provider_lifetime_test.c:231

## xuiTableViewGetBodyWidget
- 位置: xui.h:9967  已注释: 否
- 签名: `XUI_API xui_widget xuiTableViewGetBodyWidget(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3228（体 5 行）

## xuiTableViewGetModel
- 位置: xui.h:9968  已注释: 否
- 签名: `XUI_API xui_scroll_model_t* xuiTableViewGetModel(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3234（体 5 行）
- 用法: test_xui/xui_table_provider_lifetime_test.c:260

## xuiTableViewGetViewportRect
- 位置: xui.h:9969  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTableViewGetViewportRect(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3240（体 5 行）
- 用法: examples/xui_tableview/main.c:401; test_xui/xui_table_grid_test.c:538; test_xui/xui_table_provider_lifetime_test.c:213

## xuiTableViewGetItemAt
- 位置: xui.h:9970  已注释: 否
- 签名: `XUI_API int xuiTableViewGetItemAt(xui_widget pWidget, float fX, float fY, int* pRow, int* pColumn);`
- 实现: src/xui_table_view.c:3554（体 9 行）
- 用法: test_xui/xui_table_provider_lifetime_test.c:225; test_xui/xui_table_view_index_test.c:516

## xuiTableViewSetColors
- 位置: xui.h:9971  已注释: 否
- 签名: `XUI_API int xuiTableViewSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iHeader, uint32_t iRow, uint32_t iSelected, uint32_t iGrid, uint32_t iText);`
- 实现: src/xui_table_view.c:3282（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_tableview/main.c:281

## xuiTableViewSetColorStyle
- 位置: xui.h:9972  已注释: 否
- 签名: `XUI_API int xuiTableViewSetColorStyle(xui_widget pWidget, const xui_table_view_colors_t* pColors);`
- 实现: src/xui_table_view.c:3298（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_collections_table_test.c:81

## xuiTableViewGetColors
- 位置: xui.h:9973  已注释: 否
- 签名: `XUI_API int xuiTableViewGetColors(xui_widget pWidget, xui_table_view_colors_t* pColors);`
- 实现: src/xui_table_view.c:3330（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_collections_table_test.c:80

## xuiTableViewSetDisabledTextColor
- 位置: xui.h:9974  已注释: 否
- 签名: `XUI_API int xuiTableViewSetDisabledTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_table_view.c:3352（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTableViewSetScrollbarColors
- 位置: xui.h:9975  已注释: 否
- 签名: `XUI_API int xuiTableViewSetScrollbarColors(xui_widget pWidget, uint32_t iTrack, uint32_t iThumb, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);`
- 实现: src/xui_table_view.c:3360（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_tableview/main.c:288

## xuiTableViewGetScrollbarColors
- 位置: xui.h:9976  已注释: 否
- 签名: `XUI_API int xuiTableViewGetScrollbarColors(xui_widget pWidget, uint32_t* pTrack, uint32_t* pThumb, uint32_t* pHover, uint32_t* pActive, uint32_t* pFocus, uint32_t* pDisabled);`
- 实现: src/xui_table_view.c:3380（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTableViewRefresh
- 位置: xui.h:9977  已注释: 否
- 签名: `XUI_API int xuiTableViewRefresh(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3564（体 9 行）
- 用法: test_xui/xui_grid_focus_test.c:432; test_xui/xui_grid_focus_test.c:480; test_xui/xui_table_provider_lifetime_test.c:65

## xuiTableViewEnsureCellVisible
- 位置: xui.h:9978  已注释: 否
- 签名: `XUI_API int xuiTableViewEnsureCellVisible(xui_widget pWidget, int iRow, int iColumn);`
- 实现: src/xui_table_view.c:3574（体 9 行）
- 用法: examples/xui_propertygrid/main.c:532; examples/xui_tablegrid/main.c:561; examples/xui_tableview/main.c:426

## xuiTableViewGetSortColumn
- 位置: xui.h:9979  已注释: 否
- 签名: `XUI_API int xuiTableViewGetSortColumn(xui_widget pWidget, int* pDescending);`
- 实现: src/xui_table_view.c:3414（体 7 行）
- 用法: examples/xui_tableview/main.c:425; test_xui/xui_table_view_test.c:365

## xuiTableViewGetSelectCount
- 位置: xui.h:9980  已注释: 否
- 签名: `XUI_API int xuiTableViewGetSelectCount(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3422（体 5 行）

## xuiTableViewGetSortCount
- 位置: xui.h:9981  已注释: 否
- 签名: `XUI_API int xuiTableViewGetSortCount(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3428（体 5 行）

## xuiTableViewGetHoverCount
- 位置: xui.h:9982  已注释: 否
- 签名: `XUI_API int xuiTableViewGetHoverCount(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3434（体 5 行）

## xuiTableViewGetColumnResizeCount
- 位置: xui.h:9983  已注释: 否
- 签名: `XUI_API int xuiTableViewGetColumnResizeCount(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3440（体 5 行）

## xuiTableViewGetChangeCount
- 位置: xui.h:9984  已注释: 否
- 签名: `XUI_API int xuiTableViewGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_table_view.c:3446（体 5 行）

