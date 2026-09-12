# 草稿包：xui.h / Dock（75 条 API）

> 生成 2026-09-10 03:09 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiDockPanelGetType
- 位置: xui.h:9517  已注释: 否
- 签名: `XUI_API xui_widget_type xuiDockPanelGetType(xui_context pContext);`
- 实现: src/xui_dock_panel.c:5889（体 35 行）
- 返回码: NULL
- 用法: test_xui/xui_style_containers_test.c:286; test_xui/xui_style_containers_test.c:327

## xuiDockPanelCreate
- 位置: xui.h:9518  已注释: 否
- 签名: `XUI_API int xuiDockPanelCreate(xui_context pContext, xui_widget* ppWidget, const xui_dock_panel_desc_t* pDesc);`
- 实现: src/xui_dock_panel.c:5925（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch181_main1.c:13; examples/xui_dockpanel/main.c:284; examples/xui_split_layout_dock_repro/main.c:381

## xuiDockPanelClear
- 位置: xui.h:9519  已注释: 否
- 签名: `XUI_API int xuiDockPanelClear(xui_widget pWidget);`
- 实现: src/xui_dock_panel.c:5935（体 75 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiDockPanelAddWindow
- 位置: xui.h:9520  已注释: 否
- 签名: `XUI_API int xuiDockPanelAddWindow(xui_widget pWidget, const char* sTitle, xui_widget pClient, int* pWindow);`
- 实现: src/xui_dock_panel.c:6011（体 121 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/xui_dockpanel/main.c:303; examples/xui_dockpanel/main.c:304; examples/xui_dockpanel/main.c:305

## xuiDockPanelSetWindowClient
- 位置: xui.h:9521  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetWindowClient(xui_widget pWidget, int iWindow, xui_widget pClient);`
- 实现: src/xui_dock_panel.c:6133（体 52 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_test.c:667

## xuiDockPanelGetWindowClient
- 位置: xui.h:9522  已注释: 否
- 签名: `XUI_API xui_widget xuiDockPanelGetWindowClient(xui_widget pWidget, int iWindow);`
- 实现: src/xui_dock_panel.c:6186（体 6 行）
- 用法: test_xui/xui_dock_panel_test.c:670

## xuiDockPanelSetWindowTitle
- 位置: xui.h:9523  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetWindowTitle(xui_widget pWidget, int iWindow, const char* sTitle);`
- 实现: src/xui_dock_panel.c:6193（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiDockPanelGetWindowTitle
- 位置: xui.h:9524  已注释: 否
- 签名: `XUI_API const char* xuiDockPanelGetWindowTitle(xui_widget pWidget, int iWindow);`
- 实现: src/xui_dock_panel.c:6205（体 6 行）

## xuiDockPanelSetWindowFlags
- 位置: xui.h:9525  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetWindowFlags(xui_widget pWidget, int iWindow, int bClosable, int bDockable);`
- 实现: src/xui_dock_panel.c:6212（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_dockpanel/main.c:402; examples/xui_dockpanel/main.c:409; test_xui/xui_dock_panel_test.c:683

## xuiDockPanelSetWindowMovable
- 位置: xui.h:9526  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetWindowMovable(xui_widget pWidget, int iWindow, int bMovable);`
- 实现: src/xui_dock_panel.c:6227（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_test.c:1496; test_xui/xui_dock_panel_test.c:1509

## xuiDockPanelGetWindowMovable
- 位置: xui.h:9527  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetWindowMovable(xui_widget pWidget, int iWindow);`
- 实现: src/xui_dock_panel.c:6235（体 5 行）

## xuiDockPanelSetWindowResizable
- 位置: xui.h:9528  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetWindowResizable(xui_widget pWidget, int iWindow, int bResizable);`
- 实现: src/xui_dock_panel.c:6241（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_test.c:1510

## xuiDockPanelGetWindowResizable
- 位置: xui.h:9529  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetWindowResizable(xui_widget pWidget, int iWindow);`
- 实现: src/xui_dock_panel.c:6249（体 5 行）

## xuiDockPanelSetWindowResizeEdges
- 位置: xui.h:9530  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetWindowResizeEdges(xui_widget pWidget, int iWindow, uint32_t iEdges);`
- 实现: src/xui_dock_panel.c:6255（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_dockpanel/main.c:315; test_xui/xui_dock_panel_test.c:1463

## xuiDockPanelGetWindowResizeEdges
- 位置: xui.h:9531  已注释: 否
- 签名: `XUI_API uint32_t xuiDockPanelGetWindowResizeEdges(xui_widget pWidget, int iWindow);`
- 实现: src/xui_dock_panel.c:6263（体 5 行）

## xuiDockPanelSetWindowMinSize
- 位置: xui.h:9532  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetWindowMinSize(xui_widget pWidget, int iWindow, float fMinWidth, float fMinHeight);`
- 实现: src/xui_dock_panel.c:6269（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_dockpanel/main.c:314; test_xui/xui_dock_panel_pixel_test.c:187; test_xui/xui_dock_panel_test.c:1462

## xuiDockPanelGetWindowMinSize
- 位置: xui.h:9533  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetWindowMinSize(xui_widget pWidget, int iWindow, float* pMinWidth, float* pMinHeight);`
- 实现: src/xui_dock_panel.c:6284（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiDockPanelSetWindowIcon
- 位置: xui.h:9534  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetWindowIcon(xui_widget pWidget, int iWindow, xui_surface pSurface, xui_rect_t tSrc);`
- 实现: src/xui_dock_panel.c:6294（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_containers_test.c:260

## xuiDockPanelGetWindowIconSurface
- 位置: xui.h:9535  已注释: 否
- 签名: `XUI_API xui_surface xuiDockPanelGetWindowIconSurface(xui_widget pWidget, int iWindow);`
- 实现: src/xui_dock_panel.c:6308（体 5 行）

## xuiDockPanelGetWindowIconSource
- 位置: xui.h:9536  已注释: 否
- 签名: `XUI_API xui_rect_t xuiDockPanelGetWindowIconSource(xui_widget pWidget, int iWindow);`
- 实现: src/xui_dock_panel.c:6314（体 5 行）

## xuiDockPanelGetWindowCloseButtonWidget
- 位置: xui.h:9537  已注释: 否
- 签名: `XUI_API xui_widget xuiDockPanelGetWindowCloseButtonWidget(xui_widget pWidget, int iWindow);`
- 实现: src/xui_dock_panel.c:6320（体 5 行）
- 用法: test_xui/xui_dock_panel_test.c:1474; test_xui/xui_dock_panel_test.c:1520

## xuiDockPanelGetWindowPinButtonWidget
- 位置: xui.h:9538  已注释: 否
- 签名: `XUI_API xui_widget xuiDockPanelGetWindowPinButtonWidget(xui_widget pWidget, int iWindow);`
- 实现: src/xui_dock_panel.c:6326（体 5 行）

## xuiDockPanelSetWindowUserData
- 位置: xui.h:9539  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetWindowUserData(xui_widget pWidget, int iWindow, void* pUser);`
- 实现: src/xui_dock_panel.c:6332（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiDockPanelGetWindowUserData
- 位置: xui.h:9540  已注释: 否
- 签名: `XUI_API void* xuiDockPanelGetWindowUserData(xui_widget pWidget, int iWindow);`
- 实现: src/xui_dock_panel.c:6341（体 6 行）

## xuiDockPanelSetFont
- 位置: xui.h:9541  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_dock_panel.c:6348（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_containers_test.c:252

## xuiDockPanelGetFont
- 位置: xui.h:9542  已注释: 否
- 签名: `XUI_API xui_font xuiDockPanelGetFont(xui_widget pWidget);`
- 实现: src/xui_dock_panel.c:6359（体 5 行）

## xuiDockPanelDockWindow
- 位置: xui.h:9543  已注释: 否
- 签名: `XUI_API int xuiDockPanelDockWindow(xui_widget pWidget, int iWindow, int iRegion, int iSide, float fRatio, int* pPane);`
- 实现: src/xui_dock_panel.c:6365（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_dockpanel/main.c:317; examples/xui_dockpanel/main.c:326; examples/xui_dockpanel/main.c:327

## xuiDockPanelDockWindowToPane
- 位置: xui.h:9544  已注释: 否
- 签名: `XUI_API int xuiDockPanelDockWindowToPane(xui_widget pWidget, int iWindow, int iPane);`
- 实现: src/xui_dock_panel.c:6373（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_dockpanel/main.c:318; examples/xui_dockpanel/main.c:319; examples/xui_dockpanel/main.c:320

## xuiDockPanelDockWindowToPaneSide
- 位置: xui.h:9545  已注释: 否
- 签名: `XUI_API int xuiDockPanelDockWindowToPaneSide(xui_widget pWidget, int iWindow, int iPane, int iSide, float fRatio, int* pPane);`
- 实现: src/xui_dock_panel.c:6381（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_dockpanel/main.c:420; test_xui/xui_dock_panel_pixel_test.c:167; test_xui/xui_dock_panel_test.c:699

## xuiDockPanelFloatWindow
- 位置: xui.h:9546  已注释: 否
- 签名: `XUI_API int xuiDockPanelFloatWindow(xui_widget pWidget, int iWindow, xui_rect_t tRect);`
- 实现: src/xui_dock_panel.c:6389（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_dockpanel/main.c:407; examples/xui_dockpanel/main.c:410; test_xui/xui_dock_panel_test.c:693

## xuiDockPanelHideWindow
- 位置: xui.h:9547  已注释: 否
- 签名: `XUI_API int xuiDockPanelHideWindow(xui_widget pWidget, int iWindow);`
- 实现: src/xui_dock_panel.c:6410（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_test.c:744; test_xui/xui_dock_panel_test.c:878; test_xui/xui_dock_panel_test.c:1248

## xuiDockPanelAutoHideWindow
- 位置: xui.h:9548  已注释: 否
- 签名: `XUI_API int xuiDockPanelAutoHideWindow(xui_widget pWidget, int iWindow);`
- 实现: src/xui_dock_panel.c:6427（体 28 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_dockpanel/main.c:408; examples/xui_dockpanel/main.c:425; test_xui/xui_dock_panel_pixel_test.c:176

## xuiDockPanelExpandAutoHideWindow
- 位置: xui.h:9549  已注释: 否
- 签名: `XUI_API int xuiDockPanelExpandAutoHideWindow(xui_widget pWidget, int iWindow);`
- 实现: src/xui_dock_panel.c:6456（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_dockpanel/main.c:427; test_xui/xui_dock_panel_pixel_test.c:188; test_xui/xui_dock_panel_test.c:986

## xuiDockPanelCollapseAutoHide
- 位置: xui.h:9550  已注释: 否
- 签名: `XUI_API int xuiDockPanelCollapseAutoHide(xui_widget pWidget);`
- 实现: src/xui_dock_panel.c:6470（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_pixel_test.c:194; test_xui/xui_dock_panel_test.c:982

## xuiDockPanelGetAutoHideExpandedWindow
- 位置: xui.h:9551  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetAutoHideExpandedWindow(xui_widget pWidget);`
- 实现: src/xui_dock_panel.c:6481（体 5 行）
- 用法: examples/xui_dockpanel/main.c:432; test_xui/xui_dock_panel_test.c:963; test_xui/xui_dock_panel_test.c:985

## xuiDockPanelGetAutoHideExpandRect
- 位置: xui.h:9552  已注释: 否
- 签名: `XUI_API xui_rect_t xuiDockPanelGetAutoHideExpandRect(xui_widget pWidget);`
- 实现: src/xui_dock_panel.c:6487（体 5 行）
- 用法: examples/xui_dockpanel/main.c:430; test_xui/xui_dock_panel_pixel_test.c:82; test_xui/xui_dock_panel_pixel_test.c:191

## xuiDockPanelDockAutoHideWindow
- 位置: xui.h:9553  已注释: 否
- 签名: `XUI_API int xuiDockPanelDockAutoHideWindow(xui_widget pWidget, int iWindow);`
- 实现: src/xui_dock_panel.c:6493（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_dockpanel/main.c:434; test_xui/xui_dock_panel_test.c:990; test_xui/xui_dock_panel_test.c:1087

## xuiDockPanelSetPaneActiveWindow
- 位置: xui.h:9554  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetPaneActiveWindow(xui_widget pWidget, int iPane, int iWindow);`
- 实现: src/xui_dock_panel.c:6509（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_dockpanel/main.c:325; examples/xui_dockpanel/main.c:392; examples/xui_split_layout_dock_repro/main.c:405

## xuiDockPanelGetPaneActiveWindow
- 位置: xui.h:9555  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetPaneActiveWindow(xui_widget pWidget, int iPane);`
- 实现: src/xui_dock_panel.c:6518（体 5 行）
- 用法: examples/xui_dockpanel/main.c:261; examples/xui_dockpanel/main.c:393; test_xui/xui_dock_panel_test.c:657

## xuiDockPanelGetPaneWindowCount
- 位置: xui.h:9556  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetPaneWindowCount(xui_widget pWidget, int iPane);`
- 实现: src/xui_dock_panel.c:6524（体 6 行）
- 用法: examples/xui_dockpanel/main.c:396; test_xui/xui_dock_panel_test.c:656; test_xui/xui_dock_panel_test.c:662

## xuiDockPanelGetPaneWindow
- 位置: xui.h:9557  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetPaneWindow(xui_widget pWidget, int iPane, int iIndex);`
- 实现: src/xui_dock_panel.c:6531（体 7 行）
- 用法: test_xui/xui_dock_panel_test.c:788; test_xui/xui_dock_panel_test.c:789; test_xui/xui_dock_panel_test.c:874

## xuiDockPanelGetWindowCount
- 位置: xui.h:9558  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetWindowCount(xui_widget pWidget);`
- 实现: src/xui_dock_panel.c:6539（体 5 行）
- 用法: examples/xui_dockpanel/main.c:259; examples/xui_dockpanel/main.c:389; test_xui/xui_dock_panel_test.c:654

## xuiDockPanelGetPaneCount
- 位置: xui.h:9559  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetPaneCount(xui_widget pWidget);`
- 实现: src/xui_dock_panel.c:6545（体 5 行）
- 用法: examples/xui_dockpanel/main.c:260; examples/xui_dockpanel/main.c:491; examples/xui_split_layout_dock_repro/main.c:507

## xuiDockPanelGetWindowInfo
- 位置: xui.h:9560  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetWindowInfo(xui_widget pWidget, int iWindow, xui_dock_window_info_t* pInfo);`
- 实现: src/xui_dock_panel.c:6551（体 35 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_dockpanel/main.c:411; examples/xui_dockpanel/main.c:422; examples/xui_dockpanel/main.c:429

## xuiDockPanelGetPaneInfo
- 位置: xui.h:9561  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetPaneInfo(xui_widget pWidget, int iPane, xui_dock_pane_info_t* pInfo);`
- 实现: src/xui_dock_panel.c:6587（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_dockpanel/main.c:390; examples/xui_dockpanel/main.c:404; examples/xui_dockpanel/main.c:415

## xuiDockPanelSetRegionSize
- 位置: xui.h:9562  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetRegionSize(xui_widget pWidget, int iRegion, int iSizeMode, float fValue);`
- 实现: src/xui_dock_panel.c:6611（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_pixel_test.c:136; test_xui/xui_dock_panel_pixel_test.c:212

## xuiDockPanelGetRegionSize
- 位置: xui.h:9563  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetRegionSize(xui_widget pWidget, int iRegion, int* pSizeMode, float* pValue);`
- 实现: src/xui_dock_panel.c:6624（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_pixel_test.c:164; test_xui/xui_dock_panel_pixel_test.c:217; test_xui/xui_dock_panel_test.c:715

## xuiDockPanelSetMetrics
- 位置: xui.h:9564  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetMetrics(xui_widget pWidget, const xui_dock_panel_metrics_t* pMetrics);`
- 实现: src/xui_dock_panel.c:6633（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_pixel_test.c:131

## xuiDockPanelGetMetrics
- 位置: xui.h:9565  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetMetrics(xui_widget pWidget, xui_dock_panel_metrics_t* pMetrics);`
- 实现: src/xui_dock_panel.c:6643（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_pixel_test.c:127; test_xui/xui_dock_panel_test.c:521

## xuiDockPanelSetColors
- 位置: xui.h:9566  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetColors(xui_widget pWidget, const xui_dock_panel_colors_t* pColors);`
- 实现: src/xui_dock_panel.c:6652（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_containers_test.c:323

## xuiDockPanelGetColors
- 位置: xui.h:9567  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetColors(xui_widget pWidget, xui_dock_panel_colors_t* pColors);`
- 实现: src/xui_dock_panel.c:6662（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_test.c:524; test_xui/xui_style_containers_test.c:263; test_xui/xui_style_containers_test.c:299

## xuiDockPanelSetWindowStateChanged
- 位置: xui.h:9568  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetWindowStateChanged(xui_widget pWidget, xui_dock_window_state_proc onState, void* pUser);`
- 实现: src/xui_dock_panel.c:6670（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_dockpanel/main.c:289; test_xui/xui_dock_panel_test.c:453

## xuiDockPanelSetActiveChanged
- 位置: xui.h:9569  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetActiveChanged(xui_widget pWidget, xui_dock_active_proc onActive, void* pUser);`
- 实现: src/xui_dock_panel.c:6679（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_dockpanel/main.c:290; test_xui/xui_dock_panel_test.c:455

## xuiDockPanelSetWindowClose
- 位置: xui.h:9570  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetWindowClose(xui_widget pWidget, xui_dock_window_close_proc onClose, void* pUser);`
- 实现: src/xui_dock_panel.c:6688（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_dockpanel/main.c:291; test_xui/xui_dock_panel_test.c:457; test_xui/xui_dock_panel_test.c:1461

## xuiDockPanelSetContextMenu
- 位置: xui.h:9571  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetContextMenu(xui_widget pWidget, xui_dock_context_proc onContext, void* pUser);`
- 实现: src/xui_dock_panel.c:6697（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiDockPanelHitTest
- 位置: xui.h:9572  已注释: 否
- 签名: `XUI_API int xuiDockPanelHitTest(xui_widget pWidget, float fX, float fY, xui_dock_hit_t* pHit);`
- 实现: src/xui_dock_panel.c:6706（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_pixel_test.c:57; test_xui/xui_dock_panel_pixel_test.c:153; test_xui/xui_dock_panel_pixel_test.c:157

## xuiDockPanelFindDropTarget
- 位置: xui.h:9573  已注释: 否
- 签名: `XUI_API int xuiDockPanelFindDropTarget(xui_widget pWidget, int iWindow, float fX, float fY, xui_dock_drop_info_t* pInfo);`
- 实现: src/xui_dock_panel.c:6714（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_dockpanel/main.c:405; examples/xui_dockpanel/main.c:417; test_xui/xui_dock_panel_test.c:289

## xuiDockPanelGetDragPreview
- 位置: xui.h:9574  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetDragPreview(xui_widget pWidget, xui_dock_drop_info_t* pInfo);`
- 实现: src/xui_dock_panel.c:6721（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_test.c:1161; test_xui/xui_dock_panel_test.c:1170; test_xui/xui_dock_panel_test.c:1180

## xuiDockPanelGetOptionMenu
- 位置: xui.h:9575  已注释: 否
- 签名: `XUI_API xui_widget xuiDockPanelGetOptionMenu(xui_widget pWidget);`
- 实现: src/xui_dock_panel.c:6730（体 5 行）
- 用法: examples/xui_dockpanel/main.c:399; test_xui/xui_dock_panel_test.c:687; test_xui/xui_dock_panel_test.c:794

## xuiDockPanelGetOverflowMenu
- 位置: xui.h:9576  已注释: 否
- 签名: `XUI_API xui_widget xuiDockPanelGetOverflowMenu(xui_widget pWidget);`
- 实现: src/xui_dock_panel.c:6736（体 5 行）
- 用法: examples/xui_dockpanel/main.c:395; test_xui/xui_dock_panel_test.c:818

## xuiDockPanelSetMenuTitle
- 位置: xui.h:9577  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetMenuTitle(xui_widget pWidget, int iCommand, const char* sTitle);`
- 实现: src/xui_dock_panel.c:6742（体 27 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK

## xuiDockPanelGetMenuTitle
- 位置: xui.h:9578  已注释: 否
- 签名: `XUI_API const char* xuiDockPanelGetMenuTitle(xui_widget pWidget, int iCommand);`
- 实现: src/xui_dock_panel.c:6770（体 5 行）

## xuiDockPanelSetTooltipText
- 位置: xui.h:9579  已注释: 否
- 签名: `XUI_API int xuiDockPanelSetTooltipText(xui_widget pWidget, int iTooltip, const char* sText);`
- 实现: src/xui_dock_panel.c:6776（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_OUT_OF_MEMORY

## xuiDockPanelGetTooltipText
- 位置: xui.h:9580  已注释: 否
- 签名: `XUI_API const char* xuiDockPanelGetTooltipText(xui_widget pWidget, int iTooltip);`
- 实现: src/xui_dock_panel.c:6798（体 6 行）

## xuiDockPanelOpenPaneMenu
- 位置: xui.h:9581  已注释: 否
- 签名: `XUI_API int xuiDockPanelOpenPaneMenu(xui_widget pWidget, int iPane);`
- 实现: src/xui_dock_panel.c:6805（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_dockpanel/main.c:398; test_xui/xui_dock_panel_test.c:685; test_xui/xui_dock_panel_test.c:792

## xuiDockPanelOpenOverflowMenu
- 位置: xui.h:9582  已注释: 否
- 签名: `XUI_API int xuiDockPanelOpenOverflowMenu(xui_widget pWidget, int iPane);`
- 实现: src/xui_dock_panel.c:6812（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_dockpanel/main.c:394; test_xui/xui_dock_panel_test.c:816

## xuiDockPanelSaveState
- 位置: xui.h:9583  已注释: 否
- 签名: `XUI_API int xuiDockPanelSaveState(xui_widget pWidget, xvalue** ppState);`
- 实现: src/xui_dock_panel.c:6819（体 74 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_dock_panel_test.c:519; test_xui/xui_dock_panel_test.c:1253; test_xui/xui_dock_panel_test.c:1415

## xuiDockPanelLoadState
- 位置: xui.h:9584  已注释: 否
- 签名: `XUI_API int xuiDockPanelLoadState(xui_widget pWidget, xvalue* pState);`
- 实现: src/xui_dock_panel.c:6894（体 74 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_test.c:639; test_xui/xui_dock_panel_test.c:1263; test_xui/xui_dock_panel_test.c:1421

## xuiDockPanelStateFree
- 位置: xui.h:9585  已注释: 否
- 签名: `XUI_API void xuiDockPanelStateFree(xvalue* pState);`
- 实现: src/xui_dock_panel.c:6969（体 4 行）
- 用法: test_xui/xui_dock_panel_test.c:640; test_xui/xui_dock_panel_test.c:1265; test_xui/xui_dock_panel_test.c:1423

## xuiDockPanelStateGetCounts
- 位置: xui.h:9586  已注释: 否
- 签名: `XUI_API int xuiDockPanelStateGetCounts(xvalue* pState, int* pRegionCount, int* pWindowCount, int* pFloatingCount);`
- 实现: src/xui_dock_panel.c:6974（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_test.c:1255; test_xui/xui_dock_panel_test.c:1417

## xuiDockPanelSaveXSONFile
- 位置: xui.h:9587  已注释: 否
- 签名: `XUI_API int xuiDockPanelSaveXSONFile(xui_widget pWidget, const char* sPath);`
- 实现: src/xui_dock_panel.c:6994（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_dock_panel_test.c:1277

## xuiDockPanelLoadXSONFile
- 位置: xui.h:9588  已注释: 否
- 签名: `XUI_API int xuiDockPanelLoadXSONFile(xui_widget pWidget, const char* sPath);`
- 实现: src/xui_dock_panel.c:7006（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_RESOURCE_FAILED
- 用法: test_xui/xui_dock_panel_test.c:1281

## xuiDockPanelGetChangeCount
- 位置: xui.h:9589  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_dock_panel.c:7021（体 5 行）
- 用法: examples/xui_dockpanel/main.c:262; examples/xui_dockpanel/main.c:445; examples/xui_dockpanel/main.c:492

## xuiDockPanelGetLayoutChangeCount
- 位置: xui.h:9590  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetLayoutChangeCount(xui_widget pWidget);`
- 实现: src/xui_dock_panel.c:7027（体 5 行）
- 用法: test_xui/xui_dock_panel_test.c:940

## xuiDockPanelGetWindowChangeCount
- 位置: xui.h:9591  已注释: 否
- 签名: `XUI_API int xuiDockPanelGetWindowChangeCount(xui_widget pWidget);`
- 实现: src/xui_dock_panel.c:7033（体 5 行）

