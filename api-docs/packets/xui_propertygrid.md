# 草稿包：xui.h / propertygrid（58 条 API）

> 生成 2026-09-10 03:09 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiPropertyGridGetType
- 位置: xui.h:10699  已注释: 否
- 签名: `XUI_API xui_widget_type xuiPropertyGridGetType(xui_context pContext);`
- 实现: src/xui_property_grid.c:1358（体 30 行）
- 返回码: NULL

## xuiPropertyGridCreate
- 位置: xui.h:10700  已注释: 否
- 签名: `XUI_API int xuiPropertyGridCreate(xui_context pContext, xui_widget* ppWidget, const xui_property_grid_desc_t* pDesc);`
- 实现: src/xui_property_grid.c:1389（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch184_main1.c:16; examples/xui_propertygrid/main.c:393; test_xui/xui_grid_focus_test.c:146

## xuiPropertyGridGetTableGrid
- 位置: xui.h:10701  已注释: 否
- 签名: `XUI_API xui_widget xuiPropertyGridGetTableGrid(xui_widget pWidget);`
- 实现: src/xui_property_grid.c:1404（体 5 行）
- 用法: examples/xui_propertygrid/main.c:402; test_xui/xui_grid_focus_test.c:147; test_xui/xui_property_grid_test.c:365

## xuiPropertyGridGetTableView
- 位置: xui.h:10702  已注释: 否
- 签名: `XUI_API xui_widget xuiPropertyGridGetTableView(xui_widget pWidget);`
- 实现: src/xui_property_grid.c:1410（体 5 行）
- 用法: examples/xui_propertygrid/main.c:403; test_xui/xui_grid_focus_test.c:148; test_xui/xui_property_grid_test.c:366

## xuiPropertyGridClear
- 位置: xui.h:10703  已注释: 否
- 签名: `XUI_API int xuiPropertyGridClear(xui_widget pWidget);`
- 实现: src/xui_property_grid.c:1416（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPropertyGridAddCategory
- 位置: xui.h:10704  已注释: 否
- 签名: `XUI_API int xuiPropertyGridAddCategory(xui_widget pWidget, const char* sId, const char* sName, int bExpanded);`
- 实现: src/xui_property_grid.c:1434（体 22 行）
- 用法: examples/xui_propertygrid/main.c:414; examples/xui_propertygrid/main.c:415; examples/xui_propertygrid/main.c:416

## xuiPropertyGridAddProperty
- 位置: xui.h:10705  已注释: 否
- 签名: `XUI_API int xuiPropertyGridAddProperty(xui_widget pWidget, int iCategory, const xui_property_desc_t* pDesc);`
- 实现: src/xui_property_grid.c:1457（体 33 行）
- 用法: examples/xui_propertygrid/main.c:328; test_xui/xui_grid_focus_test.c:156; test_xui/xui_property_grid_test.c:393

## xuiPropertyGridFindCategory
- 位置: xui.h:10706  已注释: 否
- 签名: `XUI_API int xuiPropertyGridFindCategory(xui_widget pWidget, const char* sId);`
- 实现: src/xui_property_grid.c:1491（体 16 行）

## xuiPropertyGridFindProperty
- 位置: xui.h:10707  已注释: 否
- 签名: `XUI_API int xuiPropertyGridFindProperty(xui_widget pWidget, const char* sId);`
- 实现: src/xui_property_grid.c:1508（体 16 行）
- 用法: examples/xui_propertygrid/main.c:507; test_xui/xui_property_grid_test.c:475; test_xui/xui_property_grid_test.c:476

## xuiPropertyGridSetCategoryExpanded
- 位置: xui.h:10708  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetCategoryExpanded(xui_widget pWidget, int iCategory, int bExpanded);`
- 实现: src/xui_property_grid.c:1525（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_propertygrid/main.c:528; examples/xui_propertygrid/main.c:531; test_xui/xui_property_grid_test.c:519

## xuiPropertyGridGetCategoryExpanded
- 位置: xui.h:10709  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetCategoryExpanded(xui_widget pWidget, int iCategory);`
- 实现: src/xui_property_grid.c:1540（体 5 行）
- 用法: test_xui/xui_grid_focus_test.c:446; test_xui/xui_grid_focus_test.c:449; test_xui/xui_grid_focus_test.c:451

## xuiPropertyGridSetSelected
- 位置: xui.h:10710  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetSelected(xui_widget pWidget, int iProperty);`
- 实现: src/xui_property_grid.c:1546（体 25 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_propertygrid/main.c:542

## xuiPropertyGridGetSelected
- 位置: xui.h:10711  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetSelected(xui_widget pWidget);`
- 实现: src/xui_property_grid.c:1572（体 5 行）
- 用法: test_xui/xui_property_grid_test.c:526

## xuiPropertyGridGetCategoryCount
- 位置: xui.h:10712  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetCategoryCount(xui_widget pWidget);`
- 实现: src/xui_property_grid.c:1578（体 5 行）
- 用法: examples/xui_propertygrid/main.c:504; test_xui/xui_property_grid_test.c:473

## xuiPropertyGridGetPropertyCount
- 位置: xui.h:10713  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetPropertyCount(xui_widget pWidget);`
- 实现: src/xui_property_grid.c:1584（体 5 行）
- 用法: examples/xui_propertygrid/main.c:505; test_xui/xui_property_grid_test.c:474

## xuiPropertyGridGetVisibleCount
- 位置: xui.h:10714  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetVisibleCount(xui_widget pWidget);`
- 实现: src/xui_property_grid.c:1590（体 5 行）
- 用法: examples/xui_propertygrid/main.c:508; examples/xui_propertygrid/main.c:527; examples/xui_propertygrid/main.c:530

## xuiPropertyGridGetVisibleProperty
- 位置: xui.h:10715  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetVisibleProperty(xui_widget pWidget, int iVisible);`
- 实现: src/xui_property_grid.c:1596（体 5 行）
- 用法: examples/xui_propertygrid/main.c:508; test_xui/xui_property_grid_test.c:477; test_xui/xui_property_grid_test.c:490

## xuiPropertyGridSetValue
- 位置: xui.h:10716  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetValue(xui_widget pWidget, int iProperty, const char* sValue);`
- 实现: src/xui_property_grid.c:1602（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_propertygrid/main.c:263; examples/xui_propertygrid/main.c:265; test_xui/xui_property_grid_test.c:259

## xuiPropertyGridGetValue
- 位置: xui.h:10717  已注释: 否
- 签名: `XUI_API const char* xuiPropertyGridGetValue(xui_widget pWidget, int iProperty);`
- 实现: src/xui_property_grid.c:1612（体 5 行）
- 用法: examples/xui_propertygrid/main.c:535; examples/xui_propertygrid/main.c:538; examples/xui_propertygrid/main.c:541

## xuiPropertyGridGetBool
- 位置: xui.h:10718  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetBool(xui_widget pWidget, int iProperty, int bDefault);`
- 实现: src/xui_property_grid.c:1618（体 5 行）
- 用法: examples/xui_propertygrid/main.c:537; test_xui/xui_property_grid_test.c:535

## xuiPropertyGridSetBool
- 位置: xui.h:10719  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetBool(xui_widget pWidget, int iProperty, int bValue);`
- 实现: src/xui_property_grid.c:1624（体 4 行）

## xuiPropertyGridGetInt
- 位置: xui.h:10720  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetInt(xui_widget pWidget, int iProperty, int iDefault);`
- 实现: src/xui_property_grid.c:1629（体 5 行）
- 用法: examples/xui_propertygrid/main.c:536; test_xui/xui_property_grid_test.c:627

## xuiPropertyGridSetInt
- 位置: xui.h:10721  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetInt(xui_widget pWidget, int iProperty, int iValue);`
- 实现: src/xui_property_grid.c:1635（体 6 行）

## xuiPropertyGridGetFloat
- 位置: xui.h:10722  已注释: 否
- 签名: `XUI_API float xuiPropertyGridGetFloat(xui_widget pWidget, int iProperty, float fDefault);`
- 实现: src/xui_property_grid.c:1642（体 5 行）

## xuiPropertyGridSetFloat
- 位置: xui.h:10723  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetFloat(xui_widget pWidget, int iProperty, float fValue);`
- 实现: src/xui_property_grid.c:1648（体 6 行）

## xuiPropertyGridGetColor
- 位置: xui.h:10724  已注释: 否
- 签名: `XUI_API uint32_t xuiPropertyGridGetColor(xui_widget pWidget, int iProperty, uint32_t iDefault);`
- 实现: src/xui_property_grid.c:1655（体 11 行）
- 用法: test_xui/xui_property_grid_test.c:651

## xuiPropertyGridSetColor
- 位置: xui.h:10725  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetColor(xui_widget pWidget, int iProperty, uint32_t iColor);`
- 实现: src/xui_property_grid.c:1667（体 6 行）
- 用法: examples/xui_propertygrid/main.c:543; test_xui/xui_property_grid_test.c:650

## xuiPropertyGridSetPropertyFlags
- 位置: xui.h:10726  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetPropertyFlags(xui_widget pWidget, int iProperty, int iFlags);`
- 实现: src/xui_property_grid.c:1674（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_propertygrid/main.c:544; test_xui/xui_property_grid_test.c:652

## xuiPropertyGridGetPropertyFlags
- 位置: xui.h:10727  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetPropertyFlags(xui_widget pWidget, int iProperty);`
- 实现: src/xui_property_grid.c:1683（体 5 行）
- 用法: test_xui/xui_property_grid_test.c:653

## xuiPropertyGridSetEditorConfig
- 位置: xui.h:10728  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetEditorConfig(xui_widget pWidget, int iProperty, const xui_table_grid_editor_config_t* pConfig);`
- 实现: src/xui_property_grid.c:1689（体 63 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_propertygrid/main.c:340; examples/xui_propertygrid/main.c:346; examples/xui_propertygrid/main.c:353

## xuiPropertyGridSetRenderer
- 位置: xui.h:10729  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetRenderer(xui_widget pWidget, int iProperty, xui_property_grid_render_proc onRender, void* pUser);`
- 实现: src/xui_property_grid.c:1753（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPropertyGridSetAction
- 位置: xui.h:10730  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetAction(xui_widget pWidget, int iProperty, xui_property_grid_action_proc onAction, void* pUser);`
- 实现: src/xui_property_grid.c:1763（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_propertygrid/main.c:367; examples/xui_propertygrid/main.c:369

## xuiPropertyGridIsEditing
- 位置: xui.h:10731  已注释: 否
- 签名: `XUI_API int xuiPropertyGridIsEditing(xui_widget pWidget);`
- 实现: src/xui_property_grid.c:1772（体 5 行）
- 用法: test_xui/xui_property_grid_test.c:527; test_xui/xui_property_grid_test.c:535; test_xui/xui_property_grid_test.c:549

## xuiPropertyGridBeginEdit
- 位置: xui.h:10732  已注释: 否
- 签名: `XUI_API int xuiPropertyGridBeginEdit(xui_widget pWidget, int iProperty);`
- 实现: src/xui_property_grid.c:1778（体 12 行）
- 用法: examples/xui_propertygrid/main.c:511; examples/xui_propertygrid/main.c:514; examples/xui_propertygrid/main.c:520

## xuiPropertyGridEndEdit
- 位置: xui.h:10733  已注释: 否
- 签名: `XUI_API int xuiPropertyGridEndEdit(xui_widget pWidget, int bCommit);`
- 实现: src/xui_property_grid.c:1791（体 5 行）
- 用法: examples/xui_propertygrid/main.c:513; examples/xui_propertygrid/main.c:516; examples/xui_propertygrid/main.c:519

## xuiPropertyGridSetFont
- 位置: xui.h:10734  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_property_grid.c:1797（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPropertyGridGetFont
- 位置: xui.h:10735  已注释: 否
- 签名: `XUI_API xui_font xuiPropertyGridGetFont(xui_widget pWidget);`
- 实现: src/xui_property_grid.c:1807（体 5 行）
- 用法: test_xui/xui_property_grid_test.c:480

## xuiPropertyGridSetMetrics
- 位置: xui.h:10736  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetMetrics(xui_widget pWidget, float fNameWidth, float fRowHeight, float fCategoryHeight);`
- 实现: src/xui_property_grid.c:1813（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPropertyGridGetMetrics
- 位置: xui.h:10737  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetMetrics(xui_widget pWidget, float* pNameWidth, float* pRowHeight, float* pCategoryHeight);`
- 实现: src/xui_property_grid.c:1826（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPropertyGridSetDescriptionMode
- 位置: xui.h:10738  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetDescriptionMode(xui_widget pWidget, int iMode, float fPanelHeight);`
- 实现: src/xui_property_grid.c:1836（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_property_grid_test.c:656

## xuiPropertyGridGetDescriptionMode
- 位置: xui.h:10739  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetDescriptionMode(xui_widget pWidget);`
- 实现: src/xui_property_grid.c:1847（体 5 行）
- 用法: test_xui/xui_property_grid_test.c:657

## xuiPropertyGridSetEditMode
- 位置: xui.h:10740  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetEditMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_property_grid.c:1853（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPropertyGridGetEditMode
- 位置: xui.h:10741  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetEditMode(xui_widget pWidget);`
- 实现: src/xui_property_grid.c:1860（体 5 行）

## xuiPropertyGridSetScroll
- 位置: xui.h:10742  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetScroll(xui_widget pWidget, float fScrollY);`
- 实现: src/xui_property_grid.c:1866（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_property_grid_test.c:654

## xuiPropertyGridGetScroll
- 位置: xui.h:10743  已注释: 否
- 签名: `XUI_API float xuiPropertyGridGetScroll(xui_widget pWidget);`
- 实现: src/xui_property_grid.c:1878（体 12 行）
- 用法: examples/xui_propertygrid/main.c:533; test_xui/xui_property_grid_test.c:655

## xuiPropertyGridSetScrollbarMode
- 位置: xui.h:10744  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetScrollbarMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_property_grid.c:1891（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPropertyGridGetScrollbarMode
- 位置: xui.h:10745  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetScrollbarMode(xui_widget pWidget);`
- 实现: src/xui_property_grid.c:1898（体 5 行）

## xuiPropertyGridSetSelect
- 位置: xui.h:10746  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetSelect(xui_widget pWidget, xui_property_grid_select_proc onSelect, void* pUser);`
- 实现: src/xui_property_grid.c:1904（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_propertygrid/main.c:404; test_xui/xui_property_grid_test.c:368

## xuiPropertyGridSetValidate
- 位置: xui.h:10747  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetValidate(xui_widget pWidget, xui_property_grid_validate_proc onValidate, void* pUser);`
- 实现: src/xui_property_grid.c:1913（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_propertygrid/main.c:405; test_xui/xui_property_grid_test.c:370

## xuiPropertyGridSetChange
- 位置: xui.h:10748  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetChange(xui_widget pWidget, xui_property_grid_change_proc onChange, void* pUser);`
- 实现: src/xui_property_grid.c:1922（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_propertygrid/main.c:406; test_xui/xui_grid_focus_test.c:162; test_xui/xui_property_grid_test.c:372

## xuiPropertyGridSetGlobalAction
- 位置: xui.h:10749  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetGlobalAction(xui_widget pWidget, xui_property_grid_action_proc onAction, void* pUser);`
- 实现: src/xui_property_grid.c:1931（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_property_grid_test.c:374

## xuiPropertyGridSetGlobalRenderer
- 位置: xui.h:10750  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetGlobalRenderer(xui_widget pWidget, xui_property_grid_render_proc onRender, void* pUser);`
- 实现: src/xui_property_grid.c:1940（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_propertygrid/main.c:407; test_xui/xui_property_grid_test.c:376

## xuiPropertyGridSetContextMenu
- 位置: xui.h:10751  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetContextMenu(xui_widget pWidget, xui_property_grid_context_proc onContext, void* pUser);`
- 实现: src/xui_property_grid.c:1950（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_property_grid_test.c:378

## xuiPropertyGridSetStyle
- 位置: xui.h:10752  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetStyle(xui_widget pWidget, const xui_property_grid_style_t* pStyle);`
- 实现: src/xui_property_grid.c:1959（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_propertygrid/main.c:412

## xuiPropertyGridGetStyle
- 位置: xui.h:10753  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetStyle(xui_widget pWidget, xui_property_grid_style_t* pStyle);`
- 实现: src/xui_property_grid.c:1969（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_propertygrid/main.c:408

## xuiPropertyGridSetColors
- 位置: xui.h:10754  已注释: 否
- 签名: `XUI_API int xuiPropertyGridSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iCategory, uint32_t iRow, uint32_t iSelected, uint32_t iGrid, uint32_t iText);`
- 实现: src/xui_property_grid.c:1977（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPropertyGridGetSelectCount
- 位置: xui.h:10755  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetSelectCount(xui_widget pWidget);`
- 实现: src/xui_property_grid.c:1993（体 5 行）

## xuiPropertyGridGetToggleCount
- 位置: xui.h:10756  已注释: 否
- 签名: `XUI_API int xuiPropertyGridGetToggleCount(xui_widget pWidget);`
- 实现: src/xui_property_grid.c:1999（体 5 行）
- 用法: examples/xui_propertygrid/main.c:529; test_xui/xui_property_grid_test.c:517

