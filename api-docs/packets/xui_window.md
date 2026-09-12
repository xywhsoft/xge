# 草稿包：xui.h / window（65 条 API）

> 生成 2026-09-10 03:09 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiWindowGetType
- 位置: xui.h:9362  已注释: 否
- 签名: `XUI_API xui_widget_type xuiWindowGetType(xui_context pContext);`
- 实现: src/xui_window.c:1573（体 37 行）
- 返回码: NULL

## xuiWindowCreate
- 位置: xui.h:9363  已注释: 否
- 签名: `XUI_API int xuiWindowCreate(xui_context pContext, xui_widget* ppWidget, const xui_window_desc_t* pDesc);`
- 实现: src/xui_window.c:1611（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch150_main1.c:15; examples/xui_window/main.c:169; examples/xui_window/main.c:212

## xuiWindowSetClose
- 位置: xui.h:9364  已注释: 否
- 签名: `XUI_API int xuiWindowSetClose(xui_widget pWidget, xui_window_close_proc onClose, void* pUser);`
- 实现: src/xui_window.c:1626（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_window/main.c:188; test_xui/xui_accessibility_test.c:269; test_xui/xui_window_test.c:166

## xuiWindowSetContextMenu
- 位置: xui.h:9365  已注释: 否
- 签名: `XUI_API int xuiWindowSetContextMenu(xui_widget pWidget, xui_window_context_proc onContext, void* pUser);`
- 实现: src/xui_window.c:1635（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWindowGetClientWidget
- 位置: xui.h:9366  已注释: 否
- 签名: `XUI_API xui_widget xuiWindowGetClientWidget(xui_widget pWidget);`
- 实现: src/xui_window.c:1644（体 5 行）
- 用法: examples/xui_window/main.c:311; examples/xui_window/main.c:343; test_xui/xui_code_edit_test.c:1042

## xuiWindowGetCollapseButtonWidget
- 位置: xui.h:9367  已注释: 否
- 签名: `XUI_API xui_widget xuiWindowGetCollapseButtonWidget(xui_widget pWidget);`
- 实现: src/xui_window.c:1650（体 5 行）

## xuiWindowGetMaximizeButtonWidget
- 位置: xui.h:9368  已注释: 否
- 签名: `XUI_API xui_widget xuiWindowGetMaximizeButtonWidget(xui_widget pWidget);`
- 实现: src/xui_window.c:1656（体 5 行）

## xuiWindowGetCloseButtonWidget
- 位置: xui.h:9369  已注释: 否
- 签名: `XUI_API xui_widget xuiWindowGetCloseButtonWidget(xui_widget pWidget);`
- 实现: src/xui_window.c:1662（体 5 行）
- 用法: test_xui/xui_window_test.c:210

## xuiWindowAddChild
- 位置: xui.h:9370  已注释: 否
- 签名: `XUI_API int xuiWindowAddChild(xui_widget pWidget, xui_widget pChild);`
- 实现: src/xui_window.c:1668（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_window/main.c:150; test_xui/xui_window_test.c:88; test_xui/xui_window_test.c:251

## xuiWindowInsertBefore
- 位置: xui.h:9371  已注释: 否
- 签名: `XUI_API int xuiWindowInsertBefore(xui_widget pWidget, xui_widget pChild, xui_widget pBefore);`
- 实现: src/xui_window.c:1690（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiWindowSetTitle
- 位置: xui.h:9372  已注释: 否
- 签名: `XUI_API int xuiWindowSetTitle(xui_widget pWidget, const char* sTitle);`
- 实现: src/xui_window.c:1712（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:268

## xuiWindowGetTitle
- 位置: xui.h:9373  已注释: 否
- 签名: `XUI_API const char* xuiWindowGetTitle(xui_widget pWidget);`
- 实现: src/xui_window.c:1723（体 5 行）
- 用法: examples/xui_language/main.c:419; test_xui/xui_code_edit_test.c:1046; test_xui/xui_code_edit_test.c:1048

## xuiWindowSetFont
- 位置: xui.h:9374  已注释: 否
- 签名: `XUI_API int xuiWindowSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_window.c:1729（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiWindowGetFont
- 位置: xui.h:9375  已注释: 否
- 签名: `XUI_API xui_font xuiWindowGetFont(xui_widget pWidget);`
- 实现: src/xui_window.c:1737（体 5 行）

## xuiWindowSetIcon
- 位置: xui.h:9376  已注释: 否
- 签名: `XUI_API int xuiWindowSetIcon(xui_widget pWidget, xui_surface pSurface, xui_rect_t tSrc);`
- 实现: src/xui_window.c:1743（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiWindowGetIconSurface
- 位置: xui.h:9377  已注释: 否
- 签名: `XUI_API xui_surface xuiWindowGetIconSurface(xui_widget pWidget);`
- 实现: src/xui_window.c:1753（体 5 行）

## xuiWindowGetIconSource
- 位置: xui.h:9378  已注释: 否
- 签名: `XUI_API xui_rect_t xuiWindowGetIconSource(xui_widget pWidget);`
- 实现: src/xui_window.c:1759（体 7 行）

## xuiWindowSetOpen
- 位置: xui.h:9379  已注释: 否
- 签名: `XUI_API int xuiWindowSetOpen(xui_widget pWidget, int bOpen);`
- 实现: src/xui_window.c:1801（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_codeedit/main.c:376; examples/xui_textedit/main.c:321; examples/xui_window/main.c:327

## xuiWindowIsOpen
- 位置: xui.h:9380  已注释: 否
- 签名: `XUI_API int xuiWindowIsOpen(xui_widget pWidget);`
- 实现: src/xui_window.c:1808（体 5 行）
- 用法: examples/xui_codeedit/main.c:375; examples/xui_textedit/main.c:320; examples/xui_window/main.c:326

## xuiWindowSetShowTitleBar
- 位置: xui.h:9381  已注释: 否
- 签名: `XUI_API int xuiWindowSetShowTitleBar(xui_widget pWidget, int bShow);`
- 实现: src/xui_window.c:1814（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWindowGetShowTitleBar
- 位置: xui.h:9382  已注释: 否
- 签名: `XUI_API int xuiWindowGetShowTitleBar(xui_widget pWidget);`
- 实现: src/xui_window.c:1826（体 5 行）

## xuiWindowSetMovable
- 位置: xui.h:9383  已注释: 否
- 签名: `XUI_API int xuiWindowSetMovable(xui_widget pWidget, int bEnabled);`
- 实现: src/xui_window.c:1832（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWindowIsMovable
- 位置: xui.h:9384  已注释: 否
- 签名: `XUI_API int xuiWindowIsMovable(xui_widget pWidget);`
- 实现: src/xui_window.c:1840（体 5 行）

## xuiWindowSetDragAnywhere
- 位置: xui.h:9385  已注释: 否
- 签名: `XUI_API int xuiWindowSetDragAnywhere(xui_widget pWidget, int bEnabled);`
- 实现: src/xui_window.c:1846（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWindowIsDragAnywhere
- 位置: xui.h:9386  已注释: 否
- 签名: `XUI_API int xuiWindowIsDragAnywhere(xui_widget pWidget);`
- 实现: src/xui_window.c:1855（体 5 行）

## xuiWindowSetResizable
- 位置: xui.h:9387  已注释: 否
- 签名: `XUI_API int xuiWindowSetResizable(xui_widget pWidget, int bEnabled);`
- 实现: src/xui_window.c:1861（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWindowIsResizable
- 位置: xui.h:9388  已注释: 否
- 签名: `XUI_API int xuiWindowIsResizable(xui_widget pWidget);`
- 实现: src/xui_window.c:1869（体 5 行）

## xuiWindowSetResizeEdges
- 位置: xui.h:9389  已注释: 否
- 签名: `XUI_API int xuiWindowSetResizeEdges(xui_widget pWidget, uint32_t iEdges);`
- 实现: src/xui_window.c:1875（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWindowGetResizeEdges
- 位置: xui.h:9390  已注释: 否
- 签名: `XUI_API uint32_t xuiWindowGetResizeEdges(xui_widget pWidget);`
- 实现: src/xui_window.c:1884（体 5 行）
- 用法: test_xui/xui_window_test.c:356

## xuiWindowSetShowCollapse
- 位置: xui.h:9391  已注释: 否
- 签名: `XUI_API int xuiWindowSetShowCollapse(xui_widget pWidget, int bShow);`
- 实现: src/xui_window.c:1890（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiWindowGetShowCollapse
- 位置: xui.h:9392  已注释: 否
- 签名: `XUI_API int xuiWindowGetShowCollapse(xui_widget pWidget);`
- 实现: src/xui_window.c:1898（体 5 行）

## xuiWindowSetShowMaximize
- 位置: xui.h:9393  已注释: 否
- 签名: `XUI_API int xuiWindowSetShowMaximize(xui_widget pWidget, int bShow);`
- 实现: src/xui_window.c:1904（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiWindowGetShowMaximize
- 位置: xui.h:9394  已注释: 否
- 签名: `XUI_API int xuiWindowGetShowMaximize(xui_widget pWidget);`
- 实现: src/xui_window.c:1912（体 5 行）

## xuiWindowSetShowClose
- 位置: xui.h:9395  已注释: 否
- 签名: `XUI_API int xuiWindowSetShowClose(xui_widget pWidget, int bShow);`
- 实现: src/xui_window.c:1918（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:309

## xuiWindowGetShowClose
- 位置: xui.h:9396  已注释: 否
- 签名: `XUI_API int xuiWindowGetShowClose(xui_widget pWidget);`
- 实现: src/xui_window.c:1927（体 5 行）

## xuiWindowSetCollapsed
- 位置: xui.h:9397  已注释: 否
- 签名: `XUI_API int xuiWindowSetCollapsed(xui_widget pWidget, int bCollapsed);`
- 实现: src/xui_window.c:1933（体 25 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_window/main.c:309; examples/xui_window/main.c:312; test_xui/xui_window_pixel_test.c:176

## xuiWindowIsCollapsed
- 位置: xui.h:9398  已注释: 否
- 签名: `XUI_API int xuiWindowIsCollapsed(xui_widget pWidget);`
- 实现: src/xui_window.c:1959（体 5 行）
- 用法: examples/xui_window/main.c:311; examples/xui_window/main.c:314; test_xui/xui_window_test.c:221

## xuiWindowSetMaximized
- 位置: xui.h:9399  已注释: 否
- 签名: `XUI_API int xuiWindowSetMaximized(xui_widget pWidget, int bMaximized);`
- 实现: src/xui_window.c:1965（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_window/main.c:316; examples/xui_window/main.c:320; test_xui/xui_window_test.c:348

## xuiWindowIsMaximized
- 位置: xui.h:9400  已注释: 否
- 签名: `XUI_API int xuiWindowIsMaximized(xui_widget pWidget);`
- 实现: src/xui_window.c:1997（体 5 行）
- 用法: examples/xui_window/main.c:318; test_xui/xui_window_test.c:349; test_xui/xui_window_test.c:353

## xuiWindowBringToFront
- 位置: xui.h:9401  已注释: 否
- 签名: `XUI_API int xuiWindowBringToFront(xui_widget pWidget);`
- 实现: src/xui_window.c:2003（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_window_pixel_test.c:157; test_xui/xui_window_test.c:257

## xuiWindowSetTopMost
- 位置: xui.h:9402  已注释: 否
- 签名: `XUI_API int xuiWindowSetTopMost(xui_widget pWidget, int bTopMost);`
- 实现: src/xui_window.c:2010（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_chrome_test.c:306; test_xui/xui_window_test.c:264

## xuiWindowIsTopMost
- 位置: xui.h:9403  已注释: 否
- 签名: `XUI_API int xuiWindowIsTopMost(xui_widget pWidget);`
- 实现: src/xui_window.c:2025（体 5 行）
- 用法: examples/xui_window/main.c:356; test_xui/xui_window_test.c:220

## xuiWindowIsActive
- 位置: xui.h:9404  已注释: 否
- 签名: `XUI_API int xuiWindowIsActive(xui_widget pWidget);`
- 实现: src/xui_window.c:2031（体 5 行）
- 用法: examples/xui_window/main.c:301; test_xui/xui_window_test.c:310

## xuiWindowGetActive
- 位置: xui.h:9405  已注释: 否
- 签名: `XUI_API xui_widget xuiWindowGetActive(xui_context pContext);`
- 实现: src/xui_window.c:2037（体 8 行）
- 返回码: NULL
- 用法: examples/xui_window/main.c:463; test_xui/xui_window_test.c:220; test_xui/xui_window_test.c:310

## xuiWindowSetChrome
- 位置: xui.h:9406  已注释: 否
- 签名: `XUI_API int xuiWindowSetChrome(xui_widget pWidget, float fTitleBarHeight, float fBorderWidth, float fResizeGrip, float fButtonSize);`
- 实现: src/xui_window.c:2046（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiWindowGetChrome
- 位置: xui.h:9407  已注释: 否
- 签名: `XUI_API int xuiWindowGetChrome(xui_widget pWidget, float* pTitleBarHeight, float* pBorderWidth, float* pResizeGrip, float* pButtonSize);`
- 实现: src/xui_window.c:2057（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWindowSetIconSize
- 位置: xui.h:9408  已注释: 否
- 签名: `XUI_API int xuiWindowSetIconSize(xui_widget pWidget, float fIconSize);`
- 实现: src/xui_window.c:2068（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiWindowGetIconSize
- 位置: xui.h:9409  已注释: 否
- 签名: `XUI_API float xuiWindowGetIconSize(xui_widget pWidget);`
- 实现: src/xui_window.c:2076（体 5 行）

## xuiWindowSetMinSize
- 位置: xui.h:9410  已注释: 否
- 签名: `XUI_API int xuiWindowSetMinSize(xui_widget pWidget, float fWidth, float fHeight);`
- 实现: src/xui_window.c:2082（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiWindowGetMinSize
- 位置: xui.h:9411  已注释: 否
- 签名: `XUI_API int xuiWindowGetMinSize(xui_widget pWidget, float* pWidth, float* pHeight);`
- 实现: src/xui_window.c:2092（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWindowSetColors
- 位置: xui.h:9412  已注释: 否
- 签名: `XUI_API int xuiWindowSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iClient, uint32_t iTitleBar, uint32_t iTitleText, uint32_t iBorder, uint32_t iButtonNormal, uint32_t iButtonHover, uint32_t iButtonActive);`
- 实现: src/xui_window.c:2101（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_window/main.c:195; examples/xui_window/main.c:229

## xuiWindowGetColors
- 位置: xui.h:9413  已注释: 否
- 签名: `XUI_API int xuiWindowGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pClient, uint32_t* pTitleBar, uint32_t* pTitleText, uint32_t* pBorder, uint32_t* pButtonNormal, uint32_t* pButtonHover, uint32_t* pButtonActive);`
- 实现: src/xui_window.c:2117（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWindowSetStateColors
- 位置: xui.h:9414  已注释: 否
- 签名: `XUI_API int xuiWindowSetStateColors(xui_widget pWidget, uint32_t iInactiveTitleBar, uint32_t iInactiveTitleText, uint32_t iActiveBorder, uint32_t iCloseHover, uint32_t iCloseActive);`
- 实现: src/xui_window.c:2132（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiWindowGetStateColors
- 位置: xui.h:9415  已注释: 否
- 签名: `XUI_API int xuiWindowGetStateColors(xui_widget pWidget, uint32_t* pInactiveTitleBar, uint32_t* pInactiveTitleText, uint32_t* pActiveBorder, uint32_t* pCloseHover, uint32_t* pCloseActive);`
- 实现: src/xui_window.c:2144（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiWindowGetTitleBarRect
- 位置: xui.h:9416  已注释: 否
- 签名: `XUI_API xui_rect_t xuiWindowGetTitleBarRect(xui_widget pWidget);`
- 实现: src/xui_window.c:2156（体 7 行）
- 用法: test_xui/xui_msgbox_test.c:164; test_xui/xui_window_pixel_test.c:141

## xuiWindowGetClientRect
- 位置: xui.h:9417  已注释: 否
- 签名: `XUI_API xui_rect_t xuiWindowGetClientRect(xui_widget pWidget);`
- 实现: src/xui_window.c:2164（体 7 行）
- 用法: test_xui/xui_window_pixel_test.c:142; test_xui/xui_window_pixel_test.c:179

## xuiWindowGetCollapseButtonRect
- 位置: xui.h:9418  已注释: 否
- 签名: `XUI_API xui_rect_t xuiWindowGetCollapseButtonRect(xui_widget pWidget);`
- 实现: src/xui_window.c:2172（体 7 行）
- 用法: test_xui/xui_window_test.c:342

## xuiWindowGetMaximizeButtonRect
- 位置: xui.h:9419  已注释: 否
- 签名: `XUI_API xui_rect_t xuiWindowGetMaximizeButtonRect(xui_widget pWidget);`
- 实现: src/xui_window.c:2180（体 7 行）

## xuiWindowGetCloseButtonRect
- 位置: xui.h:9420  已注释: 否
- 签名: `XUI_API xui_rect_t xuiWindowGetCloseButtonRect(xui_widget pWidget);`
- 实现: src/xui_window.c:2188（体 7 行）
- 用法: examples/xui_window/main.c:324; test_xui/xui_msgbox_test.c:165; test_xui/xui_window_test.c:379

## xuiWindowGetResizeEdgesAt
- 位置: xui.h:9421  已注释: 否
- 签名: `XUI_API uint32_t xuiWindowGetResizeEdgesAt(xui_widget pWidget, float fX, float fY);`
- 实现: src/xui_window.c:2196（体 8 行）
- 用法: test_xui/xui_window_test.c:357

## xuiWindowGetHoverPart
- 位置: xui.h:9422  已注释: 否
- 签名: `XUI_API int xuiWindowGetHoverPart(xui_widget pWidget);`
- 实现: src/xui_window.c:2205（体 5 行）

## xuiWindowGetActivePart
- 位置: xui.h:9423  已注释: 否
- 签名: `XUI_API int xuiWindowGetActivePart(xui_widget pWidget);`
- 实现: src/xui_window.c:2211（体 5 行）

## xuiWindowGetState
- 位置: xui.h:9424  已注释: 否
- 签名: `XUI_API uint32_t xuiWindowGetState(xui_widget pWidget);`
- 实现: src/xui_window.c:2217（体 15 行）
- 用法: test_xui/xui_window_test.c:222

## xuiWindowGetChangeCount
- 位置: xui.h:9425  已注释: 否
- 签名: `XUI_API int xuiWindowGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_window.c:2233（体 5 行）
- 用法: test_xui/xui_window_test.c:384

## xuiWindowGetCloseCount
- 位置: xui.h:9426  已注释: 否
- 签名: `XUI_API int xuiWindowGetCloseCount(xui_widget pWidget);`
- 实现: src/xui_window.c:2239（体 5 行）
- 用法: examples/xui_window/main.c:326; test_xui/xui_window_test.c:381

