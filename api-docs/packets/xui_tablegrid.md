# 草稿包：xui.h / tablegrid（29 条 API）

> 生成 2026-09-10 03:06 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiTableGridGetType
- 位置: xui.h:9986  已注释: 否
- 签名: `XUI_API xui_widget_type xuiTableGridGetType(xui_context pContext);`
- 实现: src/xui_table_grid.c:1507（体 27 行）
- 返回码: NULL

## xuiTableGridCreate
- 位置: xui.h:9987  已注释: 否
- 签名: `XUI_API int xuiTableGridCreate(xui_context pContext, xui_widget* ppWidget, const xui_table_grid_desc_t* pDesc);`
- 实现: src/xui_table_grid.c:1535（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch149_main1.c:13; examples/xui_tablegrid/main.c:368; test_xui/xui_grid_focus_test.c:182

## xuiTableGridGetTableView
- 位置: xui.h:9988  已注释: 否
- 签名: `XUI_API xui_widget xuiTableGridGetTableView(xui_widget pWidget);`
- 实现: src/xui_table_grid.c:1550（体 5 行）
- 用法: examples/xui_tablegrid/main.c:386; test_xui/xui_grid_focus_test.c:184; test_xui/xui_table_grid_test.c:521

## xuiTableGridSetColumns
- 位置: xui.h:9989  已注释: 否
- 签名: `XUI_API int xuiTableGridSetColumns(xui_widget pWidget, const xui_table_view_column_t* arrColumns, int iCount);`
- 实现: src/xui_table_grid.c:1556（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTableGridSetRows
- 位置: xui.h:9990  已注释: 否
- 签名: `XUI_API int xuiTableGridSetRows(xui_widget pWidget, const xui_table_view_row_t* arrRows, int iCount);`
- 实现: src/xui_table_grid.c:1563（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTableGridSetAdapter
- 位置: xui.h:9991  已注释: 否
- 签名: `XUI_API int xuiTableGridSetAdapter(xui_widget pWidget, xui_table_view_count_proc onCount, xui_table_view_cell_proc onCell, xui_table_grid_set_proc onSet, void* pUser);`
- 实现: src/xui_table_grid.c:1570（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTableGridSetValidate
- 位置: xui.h:9992  已注释: 否
- 签名: `XUI_API int xuiTableGridSetValidate(xui_widget pWidget, xui_table_grid_validate_proc onValidate, void* pUser);`
- 实现: src/xui_table_grid.c:1582（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTableGridSetChange
- 位置: xui.h:9993  已注释: 否
- 签名: `XUI_API int xuiTableGridSetChange(xui_widget pWidget, xui_table_grid_change_proc onChange, void* pUser);`
- 实现: src/xui_table_grid.c:1591（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTableGridSetEditor
- 位置: xui.h:9994  已注释: 否
- 签名: `XUI_API int xuiTableGridSetEditor(xui_widget pWidget, xui_table_grid_editor_proc onEditor, void* pUser);`
- 实现: src/xui_table_grid.c:1600（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTableGridSetEditorConfig
- 位置: xui.h:9995  已注释: 否
- 签名: `XUI_API int xuiTableGridSetEditorConfig(xui_widget pWidget, xui_table_grid_editor_config_proc onConfig, void* pUser);`
- 实现: src/xui_table_grid.c:1609（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTableGridSetContextMenu
- 位置: xui.h:9996  已注释: 否
- 签名: `XUI_API int xuiTableGridSetContextMenu(xui_widget pWidget, xui_table_grid_context_proc onContext, void* pUser);`
- 实现: src/xui_table_grid.c:1618（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_table_grid_test.c:523

## xuiTableGridSetEditMode
- 位置: xui.h:9997  已注释: 否
- 签名: `XUI_API int xuiTableGridSetEditMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_table_grid.c:1627（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_table_grid_test.c:627

## xuiTableGridGetEditMode
- 位置: xui.h:9998  已注释: 否
- 签名: `XUI_API int xuiTableGridGetEditMode(xui_widget pWidget);`
- 实现: src/xui_table_grid.c:1635（体 5 行）
- 用法: test_xui/xui_table_grid_test.c:628

## xuiTableGridSetFont
- 位置: xui.h:9999  已注释: 否
- 签名: `XUI_API int xuiTableGridSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_table_grid.c:1641（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTableGridGetFont
- 位置: xui.h:10000  已注释: 否
- 签名: `XUI_API xui_font xuiTableGridGetFont(xui_widget pWidget);`
- 实现: src/xui_table_grid.c:1657（体 5 行）
- 用法: test_xui/xui_property_grid_test.c:480

## xuiTableGridSetDefaultMetrics
- 位置: xui.h:10001  已注释: 否
- 签名: `XUI_API int xuiTableGridSetDefaultMetrics(xui_widget pWidget, float fColumnWidth, float fRowHeight, float fHeaderHeight);`
- 实现: src/xui_table_grid.c:1663（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTableGridSetSelectionMode
- 位置: xui.h:10002  已注释: 否
- 签名: `XUI_API int xuiTableGridSetSelectionMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_table_grid.c:1670（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTableGridSetScrollbarMode
- 位置: xui.h:10003  已注释: 否
- 签名: `XUI_API int xuiTableGridSetScrollbarMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_table_grid.c:1677（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTableGridSetColors
- 位置: xui.h:10004  已注释: 否
- 签名: `XUI_API int xuiTableGridSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iHeader, uint32_t iRow, uint32_t iSelected, uint32_t iGrid, uint32_t iText);`
- 实现: src/xui_table_grid.c:1684（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_tablegrid/main.c:372

## xuiTableGridSetColorStyle
- 位置: xui.h:10005  已注释: 否
- 签名: `XUI_API int xuiTableGridSetColorStyle(xui_widget pWidget, const xui_table_view_colors_t* pColors);`
- 实现: src/xui_table_grid.c:1694（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTableGridBeginEdit
- 位置: xui.h:10006  已注释: 否
- 签名: `XUI_API int xuiTableGridBeginEdit(xui_widget pWidget, int iRow, int iColumn);`
- 实现: src/xui_table_grid.c:1704（体 118 行）
- 用法: examples/xui_tablegrid/main.c:545; examples/xui_tablegrid/main.c:548; examples/xui_tablegrid/main.c:552

## xuiTableGridEndEdit
- 位置: xui.h:10007  已注释: 否
- 签名: `XUI_API int xuiTableGridEndEdit(xui_widget pWidget, int bCommit);`
- 实现: src/xui_table_grid.c:1823（体 33 行）
- 用法: examples/xui_tablegrid/main.c:547; examples/xui_tablegrid/main.c:550; examples/xui_tablegrid/main.c:551

## xuiTableGridIsEditing
- 位置: xui.h:10008  已注释: 否
- 签名: `XUI_API int xuiTableGridIsEditing(xui_widget pWidget);`
- 实现: src/xui_table_grid.c:1857（体 5 行）
- 用法: examples/xui_tablegrid/main.c:567; test_xui/xui_grid_focus_test.c:284; test_xui/xui_grid_focus_test.c:286

## xuiTableGridGetEditingCell
- 位置: xui.h:10009  已注释: 否
- 签名: `XUI_API int xuiTableGridGetEditingCell(xui_widget pWidget, int* pRow, int* pColumn);`
- 实现: src/xui_table_grid.c:1863（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_table_grid_test.c:659

## xuiTableGridGetCommitCount
- 位置: xui.h:10010  已注释: 否
- 签名: `XUI_API int xuiTableGridGetCommitCount(xui_widget pWidget);`
- 实现: src/xui_table_grid.c:1872（体 5 行）
- 用法: examples/xui_propertygrid/main.c:539; examples/xui_propertygrid/main.c:656; examples/xui_tablegrid/main.c:202

## xuiTableGridGetRejectCount
- 位置: xui.h:10011  已注释: 否
- 签名: `XUI_API int xuiTableGridGetRejectCount(xui_widget pWidget);`
- 实现: src/xui_table_grid.c:1878（体 5 行）
- 用法: examples/xui_propertygrid/main.c:517; examples/xui_propertygrid/main.c:655; examples/xui_tablegrid/main.c:203

## xuiTableGridGetCancelCount
- 位置: xui.h:10012  已注释: 否
- 签名: `XUI_API int xuiTableGridGetCancelCount(xui_widget pWidget);`
- 实现: src/xui_table_grid.c:1884（体 5 行）
- 用法: examples/xui_tablegrid/main.c:204; examples/xui_tablegrid/main.c:678; test_xui/xui_table_grid_test.c:576

## xuiTableGridGetPickerCount
- 位置: xui.h:10013  已注释: 否
- 签名: `XUI_API int xuiTableGridGetPickerCount(xui_widget pWidget);`
- 实现: src/xui_table_grid.c:1890（体 5 行）
- 用法: test_xui/xui_property_grid_test.c:642; test_xui/xui_table_grid_test.c:623

## xuiTableGridGetChangeCount
- 位置: xui.h:10014  已注释: 否
- 签名: `XUI_API int xuiTableGridGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_table_grid.c:1896（体 5 行）

