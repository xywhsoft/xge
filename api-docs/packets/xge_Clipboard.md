# 草稿包：xge.h / Clipboard（4 条 API）

> 生成 2026-09-10 02:40 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeClipboardSetItems
- 位置: xge.h:2585  已注释: 否
- 签名: `XGE_API int xgeClipboardSetItems(const xge_clipboard_item_t* pItems, int iItemCount);`
- 实现: src/xge_render.c:1138（体 23 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK, XGE_ERROR_UNSUPPORTED

## xgeClipboardGetData
- 位置: xge.h:2586  已注释: 否
- 签名: `XGE_API int xgeClipboardGetData(const char* sFormat, void* pData, size_t iCapacity);`
- 实现: src/xge_render.c:1162（体 15 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_BUFFER_TOO_SMALL, XGE_ERROR_UNSUPPORTED

## xgeClipboardSetText
- 位置: xge.h:2587  已注释: 否
- 签名: `XGE_API void xgeClipboardSetText(const char* sText);`
- 实现: src/xge_render.c:1178（体 21 行）
- 用法: examples/tutorial_capture/ch120_main1.c:36; test/test_clipboard_win32.c:116; test/test_clipboard_win32.c:122

## xgeClipboardGetText
- 位置: xge.h:2588  已注释: 否
- 签名: `XGE_API const char* xgeClipboardGetText(void);`
- 实现: src/xge_render.c:1200（体 14 行）
- 用法: examples/tutorial_capture/ch120_main1.c:37; examples/tutorial_capture/ch120_main1.c:66; test/test_clipboard_win32.c:91

