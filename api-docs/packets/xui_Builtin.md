# 草稿包：xui.h / Builtin（6 条 API）

> 生成 2026-09-10 02:50 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiBuiltinAssetGetCount
- 位置: xui.h:5958  已注释: 否
- 签名: `XUI_API int xuiBuiltinAssetGetCount(void);`
- 实现: src/xui_assets.c:45（体 4 行）
- 返回码: XUI_BUILTIN_ASSET_COUNT
- 用法: test_xui/xui_builtin_asset_test.c:48

## xuiBuiltinAssetGetName
- 位置: xui.h:5959  已注释: 否
- 签名: `XUI_API const char* xuiBuiltinAssetGetName(int iIndex);`
- 实现: src/xui_assets.c:50（体 4 行）
- 用法: test_xui/xui_builtin_asset_test.c:49; test_xui/xui_builtin_asset_test.c:50; test_xui/xui_builtin_asset_test.c:51

## xuiBuiltinAssetGetAtlasSize
- 位置: xui.h:5960  已注释: 否
- 签名: `XUI_API int xuiBuiltinAssetGetAtlasSize(int* pWidth, int* pHeight);`
- 实现: src/xui_assets.c:55（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_builtin_asset_test.c:54; test_xui/xui_builtin_asset_test.c:56

## xuiBuiltinAssetGetRect
- 位置: xui.h:5961  已注释: 否
- 签名: `XUI_API int xuiBuiltinAssetGetRect(const char* sName, xui_rect_t* pRect);`
- 实现: src/xui_assets.c:81（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_FILE_NOT_FOUND
- 用法: test_xui/xui_builtin_asset_test.c:57; test_xui/xui_builtin_asset_test.c:59; test_xui/xui_builtin_asset_test.c:61

## xuiBuiltinAssetGetRectByIndex
- 位置: xui.h:5962  已注释: 否
- 签名: `XUI_API int xuiBuiltinAssetGetRectByIndex(int iIndex, xui_rect_t* pRect);`
- 实现: src/xui_assets.c:69（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_builtin_asset_test.c:67; test_xui/xui_builtin_asset_test.c:71

## xuiBuiltinAssetGetAtlas
- 位置: xui.h:5963  已注释: 否
- 签名: `XUI_API int xuiBuiltinAssetGetAtlas(xui_context pContext, xui_surface* ppSurface);`
- 实现: src/xui_assets.c:97（体 63 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED, XUI_ERROR_UNSUPPORTED, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_builtin_asset_test.c:72; test_xui/xui_builtin_asset_test.c:76; test_xui/xui_builtin_asset_test.c:82

