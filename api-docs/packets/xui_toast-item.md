# 草稿包：xui.h / toast-item（27 条 API）

> 生成 2026-09-10 03:08 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiToastCreate
- 位置: xui.h:9602  已注释: 否
- 签名: `XUI_API int xuiToastCreate(xui_context pContext, xui_toast* ppToast, const xui_toast_desc_t* pDesc);`
- 实现: src/xui_toast.c:1082（体 38 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/tutorial_capture/ch161_main1.c:15; examples/xui_toast/main.c:250; test_xui/xui_toast_test.c:146

## xuiToastDestroy
- 位置: xui.h:9603  已注释: 否
- 签名: `XUI_API void xuiToastDestroy(xui_toast pToast);`
- 实现: src/xui_toast.c:1121（体 20 行）
- 用法: examples/xui_toast/main.c:344; test_xui/xui_toast_test.c:231

## xuiToastShow
- 位置: xui.h:9604  已注释: 否
- 签名: `XUI_API int xuiToastShow(xui_toast pToast, int iType, const char* sTitle, const char* sMessage, float fDuration, xui_toast_click_proc onClick, void* pUser);`
- 实现: src/xui_toast.c:1142（体 38 行）
- 用法: examples/tutorial_capture/ch161_main1.c:17; examples/tutorial_capture/ch161_main1.c:18; examples/xui_toast/main.c:166

## xuiToastClose
- 位置: xui.h:9605  已注释: 否
- 签名: `XUI_API int xuiToastClose(xui_toast pToast, int iToastId);`
- 实现: src/xui_toast.c:1181（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR
- 用法: test_xui/xui_toast_test.c:221

## xuiToastClear
- 位置: xui.h:9606  已注释: 否
- 签名: `XUI_API int xuiToastClear(xui_toast pToast);`
- 实现: src/xui_toast.c:1199（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_toast/main.c:177; examples/xui_toast/main.c:284; test_xui/xui_toast_test.c:224

## xuiToastGetActiveCount
- 位置: xui.h:9607  已注释: 否
- 签名: `XUI_API int xuiToastGetActiveCount(xui_toast pToast);`
- 实现: src/xui_toast.c:1231（体 4 行）
- 用法: examples/xui_toast/main.c:280; examples/xui_toast/main.c:289; examples/xui_toast/main.c:409

## xuiToastGetPendingCount
- 位置: xui.h:9608  已注释: 否
- 签名: `XUI_API int xuiToastGetPendingCount(xui_toast pToast);`
- 实现: src/xui_toast.c:1236（体 4 行）
- 用法: examples/xui_toast/main.c:409; test_xui/xui_toast_test.c:150; test_xui/xui_toast_test.c:156

## xuiToastSetPlacement
- 位置: xui.h:9609  已注释: 否
- 签名: `XUI_API int xuiToastSetPlacement(xui_toast pToast, int iPlacement);`
- 实现: src/xui_toast.c:1241（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_toast_test.c:213

## xuiToastGetPlacement
- 位置: xui.h:9610  已注释: 否
- 签名: `XUI_API int xuiToastGetPlacement(xui_toast pToast);`
- 实现: src/xui_toast.c:1251（体 4 行）
- 用法: test_xui/xui_toast_test.c:214

## xuiToastSetDirection
- 位置: xui.h:9611  已注释: 否
- 签名: `XUI_API int xuiToastSetDirection(xui_toast pToast, int iDirection);`
- 实现: src/xui_toast.c:1256（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_toast_test.c:215

## xuiToastGetDirection
- 位置: xui.h:9612  已注释: 否
- 签名: `XUI_API int xuiToastGetDirection(xui_toast pToast);`
- 实现: src/xui_toast.c:1266（体 4 行）
- 用法: test_xui/xui_toast_test.c:216

## xuiToastSetMetrics
- 位置: xui.h:9613  已注释: 否
- 签名: `XUI_API int xuiToastSetMetrics(xui_toast pToast, const xui_toast_metrics_t* pMetrics);`
- 实现: src/xui_toast.c:1271（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_toast_test.c:206

## xuiToastGetMetrics
- 位置: xui.h:9614  已注释: 否
- 签名: `XUI_API int xuiToastGetMetrics(xui_toast pToast, xui_toast_metrics_t* pMetrics);`
- 实现: src/xui_toast.c:1286（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_toast_test.c:164; test_xui/xui_toast_test.c:203; test_xui/xui_toast_test.c:207

## xuiToastSetColors
- 位置: xui.h:9615  已注释: 否
- 签名: `XUI_API int xuiToastSetColors(xui_toast pToast, const xui_toast_colors_t* pColors);`
- 实现: src/xui_toast.c:1296（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_toast_test.c:211

## xuiToastGetColors
- 位置: xui.h:9616  已注释: 否
- 签名: `XUI_API int xuiToastGetColors(xui_toast pToast, xui_toast_colors_t* pColors);`
- 实现: src/xui_toast.c:1307（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_toast_test.c:208

## xuiToastSetFont
- 位置: xui.h:9617  已注释: 否
- 签名: `XUI_API int xuiToastSetFont(xui_toast pToast, xui_font pFont);`
- 实现: src/xui_toast.c:1317（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiToastGetFont
- 位置: xui.h:9618  已注释: 否
- 签名: `XUI_API xui_font xuiToastGetFont(xui_toast pToast);`
- 实现: src/xui_toast.c:1327（体 4 行）

## xuiToastSetClose
- 位置: xui.h:9619  已注释: 否
- 签名: `XUI_API int xuiToastSetClose(xui_toast pToast, xui_toast_close_proc onClose, void* pUser);`
- 实现: src/xui_toast.c:1332（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_toast/main.c:252; test_xui/xui_toast_test.c:148

## xuiToastGetItemWidget
- 位置: xui.h:9620  已注释: 否
- 签名: `XUI_API xui_widget xuiToastGetItemWidget(xui_toast pToast, int iSlot);`
- 实现: src/xui_toast.c:1342（体 7 行）
- 返回码: NULL
- 用法: test_xui/xui_toast_test.c:168; test_xui/xui_toast_test.c:175; test_xui/xui_toast_test.c:175

## xuiToastGetItemRect
- 位置: xui.h:9621  已注释: 否
- 签名: `XUI_API xui_rect_t xuiToastGetItemRect(xui_toast pToast, int iSlot);`
- 实现: src/xui_toast.c:1350（体 7 行）
- 用法: examples/xui_toast/main.c:279; examples/xui_toast/main.c:293; test_xui/xui_toast_test.c:161

## xuiToastGetIconRect
- 位置: xui.h:9622  已注释: 否
- 签名: `XUI_API xui_rect_t xuiToastGetIconRect(xui_toast pToast, int iSlot);`
- 实现: src/xui_toast.c:1358（体 12 行）

## xuiToastGetCloseRect
- 位置: xui.h:9623  已注释: 否
- 签名: `XUI_API xui_rect_t xuiToastGetCloseRect(xui_toast pToast, int iSlot);`
- 实现: src/xui_toast.c:1371（体 12 行）
- 用法: examples/xui_toast/main.c:299; test_xui/xui_toast_test.c:187

## xuiToastGetShowCount
- 位置: xui.h:9624  已注释: 否
- 签名: `XUI_API int xuiToastGetShowCount(xui_toast pToast);`
- 实现: src/xui_toast.c:1384（体 4 行）
- 用法: examples/xui_toast/main.c:409; test_xui/xui_toast_test.c:157

## xuiToastGetCloseCount
- 位置: xui.h:9625  已注释: 否
- 签名: `XUI_API int xuiToastGetCloseCount(xui_toast pToast);`
- 实现: src/xui_toast.c:1389（体 4 行）
- 用法: examples/xui_toast/main.c:410; test_xui/xui_toast_test.c:227

## xuiToastGetExpireCount
- 位置: xui.h:9626  已注释: 否
- 签名: `XUI_API int xuiToastGetExpireCount(xui_toast pToast);`
- 实现: src/xui_toast.c:1394（体 4 行）
- 用法: examples/xui_toast/main.c:285; examples/xui_toast/main.c:289; examples/xui_toast/main.c:410

## xuiToastGetDropCount
- 位置: xui.h:9627  已注释: 否
- 签名: `XUI_API int xuiToastGetDropCount(xui_toast pToast);`
- 实现: src/xui_toast.c:1399（体 4 行）

## xuiToastGetChangeCount
- 位置: xui.h:9628  已注释: 否
- 签名: `XUI_API int xuiToastGetChangeCount(xui_toast pToast);`
- 实现: src/xui_toast.c:1404（体 4 行）
- 用法: test_xui/xui_toast_test.c:227

