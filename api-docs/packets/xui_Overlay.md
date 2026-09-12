# 草稿包：xui.h / Overlay（6 条 API）

> 生成 2026-09-10 02:52 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiOverlayRoot
- 位置: xui.h:9658  已注释: 否
- 签名: `XUI_API xui_widget xuiOverlayRoot(xui_context pContext);`
- 实现: src/xui_widget.c:5307（体 26 行）
- 返回码: NULL
- 用法: examples/xui_window/main.c:344; test_xui/xui_dock_panel_test.c:1533; test_xui/xui_dock_panel_test.c:1553

## xuiOverlayAttach
- 位置: xui.h:9660  已注释: 是
- 签名: `XUI_API int xuiOverlayAttach(xui_context pContext, xui_widget pOwner, xui_widget pOverlay, int iLayer, int iZIndex);`
- 既有注释: /* Layer/z apply to the overlay root; descendants retain their own stacking order. */
- 实现: src/xui_widget.c:5334（体 33 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_input_test.c:957; test_xui/xui_label_test.c:1010; test_xui/xui_layout_generation_test.c:65

## xuiOverlayDetach
- 位置: xui.h:9661  已注释: 否
- 签名: `XUI_API int xuiOverlayDetach(xui_widget pOverlay);`
- 实现: src/xui_widget.c:5368（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_input_test.c:965; test_xui/xui_label_test.c:1012; test_xui/xui_window_pixel_test.c:131

## xuiOverlayBringToFront
- 位置: xui.h:9663  已注释: 是
- 签名: `XUI_API int xuiOverlayBringToFront(xui_widget pOverlay);`
- 既有注释: /* Raise within the existing layer/z group without changing explicit z values. */
- 实现: src/xui_widget.c:5377（体 37 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_input_test.c:963; test_xui/xui_overlay_stacking_test.c:39; test_xui/xui_overlay_stacking_test.c:50

## xuiOverlayGetOwner
- 位置: xui.h:9664  已注释: 否
- 签名: `XUI_API xui_widget xuiOverlayGetOwner(xui_widget pOverlay);`
- 实现: src/xui_widget.c:5415（体 4 行）
- 用法: test_xui/xui_dock_panel_test.c:538; test_xui/xui_dock_panel_test.c:569; test_xui/xui_dock_panel_test.c:603

## xuiOverlayTop
- 位置: xui.h:9666  已注释: 是
- 签名: `XUI_API xui_widget xuiOverlayTop(xui_context pContext);`
- 既有注释: /* Return the highest visible overlay in paint order, or NULL. */
- 实现: src/xui_widget.c:5420（体 15 行）
- 返回码: NULL
- 用法: test_xui/xui_dock_panel_test.c:536; test_xui/xui_dock_panel_test.c:567; test_xui/xui_dock_panel_test.c:599

