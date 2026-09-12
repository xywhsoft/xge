# 草稿包：xui.h / Scroll（15 条 API）

> 生成 2026-09-10 02:52 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiScrollModelInit
- 位置: xui.h:8336  已注释: 否
- 签名: `XUI_API void xuiScrollModelInit(xui_scroll_model_t* pModel);`
- 实现: src/xui_scroll_model.c:73（体 8 行）
- 用法: test_xui/xui_scroll_model_test.c:36

## xuiScrollModelSetViewport
- 位置: xui.h:8337  已注释: 否
- 签名: `XUI_API int xuiScrollModelSetViewport(xui_scroll_model_t* pModel, xui_rect_t tViewport);`
- 实现: src/xui_scroll_model.c:82（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_scroll_model_test.c:39

## xuiScrollModelGetViewport
- 位置: xui.h:8338  已注释: 否
- 签名: `XUI_API xui_rect_t xuiScrollModelGetViewport(const xui_scroll_model_t* pModel);`
- 实现: src/xui_scroll_model.c:93（体 10 行）
- 用法: test_xui/xui_scroll_model_test.c:41

## xuiScrollModelSetContentSize
- 位置: xui.h:8339  已注释: 否
- 签名: `XUI_API int xuiScrollModelSetContentSize(xui_scroll_model_t* pModel, float fWidth, float fHeight);`
- 实现: src/xui_scroll_model.c:104（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_scroll_model_test.c:44

## xuiScrollModelGetContentSize
- 位置: xui.h:8340  已注释: 否
- 签名: `XUI_API int xuiScrollModelGetContentSize(const xui_scroll_model_t* pModel, float* pWidth, float* pHeight);`
- 实现: src/xui_scroll_model.c:116（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1630; test_xui/xui_code_edit_test.c:1649; test_xui/xui_scroll_model_test.c:46

## xuiScrollModelSetOffset
- 位置: xui.h:8341  已注释: 否
- 签名: `XUI_API int xuiScrollModelSetOffset(xui_scroll_model_t* pModel, float fOffsetX, float fOffsetY);`
- 实现: src/xui_scroll_model.c:126（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_scroll_model_test.c:51; test_xui/xui_terminal_test.c:412; test_xui/xui_terminal_test.c:422

## xuiScrollModelScrollBy
- 位置: xui.h:8342  已注释: 否
- 签名: `XUI_API int xuiScrollModelScrollBy(xui_scroll_model_t* pModel, float fDeltaX, float fDeltaY);`
- 实现: src/xui_scroll_model.c:138（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_scroll_model_test.c:56

## xuiScrollModelGetOffset
- 位置: xui.h:8343  已注释: 否
- 签名: `XUI_API int xuiScrollModelGetOffset(const xui_scroll_model_t* pModel, float* pOffsetX, float* pOffsetY);`
- 实现: src/xui_scroll_model.c:146（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_inventory_grid/main.c:560; test_xui/xui_code_edit_test.c:1256; test_xui/xui_inventory_grid_test.c:590

## xuiScrollModelGetMaxOffset
- 位置: xui.h:8344  已注释: 否
- 签名: `XUI_API int xuiScrollModelGetMaxOffset(const xui_scroll_model_t* pModel, float* pMaxX, float* pMaxY);`
- 实现: src/xui_scroll_model.c:156（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_test.c:576; test_xui/xui_scroll_frame_test.c:178; test_xui/xui_scroll_model_test.c:48

## xuiScrollModelEnsureRectVisible
- 位置: xui.h:8345  已注释: 否
- 签名: `XUI_API int xuiScrollModelEnsureRectVisible(xui_scroll_model_t* pModel, xui_rect_t tContentRect);`
- 实现: src/xui_scroll_model.c:165（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_scroll_model_test.c:61

## xuiScrollModelScreenToViewport
- 位置: xui.h:8346  已注释: 否
- 签名: `XUI_API xui_vec2_t xuiScrollModelScreenToViewport(const xui_scroll_model_t* pModel, float fX, float fY);`
- 实现: src/xui_scroll_model.c:188（体 12 行）
- 用法: test_xui/xui_scroll_model_test.c:66

## xuiScrollModelViewportToContent
- 位置: xui.h:8347  已注释: 否
- 签名: `XUI_API xui_vec2_t xuiScrollModelViewportToContent(const xui_scroll_model_t* pModel, float fX, float fY);`
- 实现: src/xui_scroll_model.c:201（体 12 行）
- 用法: test_xui/xui_scroll_model_test.c:68

## xuiScrollModelScreenToContent
- 位置: xui.h:8348  已注释: 否
- 签名: `XUI_API xui_vec2_t xuiScrollModelScreenToContent(const xui_scroll_model_t* pModel, float fX, float fY);`
- 实现: src/xui_scroll_model.c:214（体 7 行）
- 用法: examples/xui_scrollview/main.c:326

## xuiScrollModelContentToViewport
- 位置: xui.h:8349  已注释: 否
- 签名: `XUI_API xui_vec2_t xuiScrollModelContentToViewport(const xui_scroll_model_t* pModel, float fX, float fY);`
- 实现: src/xui_scroll_model.c:222（体 12 行）

## xuiScrollModelContentToScreen
- 位置: xui.h:8350  已注释: 否
- 签名: `XUI_API xui_vec2_t xuiScrollModelContentToScreen(const xui_scroll_model_t* pModel, float fX, float fY);`
- 实现: src/xui_scroll_model.c:235（体 11 行）
- 用法: test_xui/xui_scroll_model_test.c:70

