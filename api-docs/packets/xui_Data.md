# 草稿包：xui.h / Data（9 条 API）

> 生成 2026-09-10 02:52 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiDataObjectCreate
- 位置: xui.h:5824  已注释: 否
- 签名: `XUI_API int xuiDataObjectCreate(xui_data_object* ppData);`
- 实现: src/xui_drag_drop.c:119（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_drag_drop_reentry_test.c:233; test_xui/xui_drag_drop_reentry_test.c:237; test_xui/xui_drag_drop_test.c:106

## xuiDataObjectAddRef
- 位置: xui.h:5825  已注释: 否
- 签名: `XUI_API int xuiDataObjectAddRef(xui_data_object pData);`
- 实现: src/xui_drag_drop.c:134（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiDataObjectRelease
- 位置: xui.h:5826  已注释: 否
- 签名: `XUI_API void xuiDataObjectRelease(xui_data_object pData);`
- 实现: src/xui_drag_drop.c:143（体 12 行）
- 用法: test_xui/xui_drag_drop_reentry_test.c:268; test_xui/xui_drag_drop_reentry_test.c:343; test_xui/xui_drag_drop_reentry_test.c:344

## xuiDataObjectSet
- 位置: xui.h:5827  已注释: 否
- 签名: `XUI_API int xuiDataObjectSet(xui_data_object pData, const char* sFormat, const void* pValue, size_t iSize);`
- 实现: src/xui_drag_drop.c:156（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_drag_drop_test.c:107; test_xui/xui_drag_drop_test.c:132

## xuiDataObjectSetProvider
- 位置: xui.h:5829  已注释: 否
- 签名: `XUI_API int xuiDataObjectSetProvider(xui_data_object pData, const char* sFormat, xui_data_provider_proc onRead, xui_data_provider_free_proc onFree, void* pUser);`
- 实现: src/xui_drag_drop.c:179（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_drag_drop_reentry_test.c:235; test_xui/xui_drag_drop_reentry_test.c:238

## xuiDataObjectFormatCount
- 位置: xui.h:5832  已注释: 否
- 签名: `XUI_API int xuiDataObjectFormatCount(xui_data_object pData);`
- 实现: src/xui_drag_drop.c:195（体 4 行）

## xuiDataObjectFormatAt
- 位置: xui.h:5833  已注释: 否
- 签名: `XUI_API const char* xuiDataObjectFormatAt(xui_data_object pData, int iIndex);`
- 实现: src/xui_drag_drop.c:200（体 6 行）
- 返回码: NULL

## xuiDataObjectHas
- 位置: xui.h:5834  已注释: 否
- 签名: `XUI_API int xuiDataObjectHas(xui_data_object pData, const char* sFormat);`
- 实现: src/xui_drag_drop.c:207（体 4 行）

## xuiDataObjectGet
- 位置: xui.h:5835  已注释: 否
- 签名: `XUI_API int xuiDataObjectGet(xui_data_object pData, const char* sFormat, void* pOutput, size_t iCapacity, size_t* pOutputSize);`
- 实现: src/xui_drag_drop.c:212（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_FOUND, XUI_OK, XUI_ERROR_BUFFER_TOO_SMALL
- 用法: test_xui/xui_drag_drop_reentry_test.c:75; test_xui/xui_drag_drop_test.c:60; test_xui/xui_proxy_drag_drop_test.c:25

