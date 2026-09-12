# 草稿包：xge.h / Data（9 条 API）

> 生成 2026-09-10 02:40 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeDataObjectCreate
- 位置: xge.h:1624  已注释: 否
- 签名: `XGE_API int xgeDataObjectCreate(xge_data_object* ppData);`
- 实现: src/xge_drag_drop.c:138（体 14 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: test/test_drag_data_object.c:45; test_xui/xui_proxy_drag_drop_test.c:76

## xgeDataObjectAddRef
- 位置: xge.h:1625  已注释: 否
- 签名: `XGE_API int xgeDataObjectAddRef(xge_data_object pData);`
- 实现: src/xge_drag_drop.c:153（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeDataObjectRelease
- 位置: xge.h:1626  已注释: 否
- 签名: `XGE_API void xgeDataObjectRelease(xge_data_object pData);`
- 实现: src/xge_drag_drop.c:162（体 12 行）
- 用法: test/test_drag_data_object.c:63; test_xui/xui_proxy_drag_drop_test.c:97

## xgeDataObjectSet
- 位置: xge.h:1627  已注释: 否
- 签名: `XGE_API int xgeDataObjectSet(xge_data_object pData, const char* sFormat, const void* pValue, size_t iSize);`
- 实现: src/xge_drag_drop.c:175（体 23 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: test/test_drag_data_object.c:46; test_xui/xui_proxy_drag_drop_test.c:77

## xgeDataObjectSetProvider
- 位置: xge.h:1629  已注释: 否
- 签名: `XGE_API int xgeDataObjectSetProvider(xge_data_object pData, const char* sFormat, xge_data_provider_proc onRead, xge_data_provider_free_proc onFree, void* pUser);`
- 实现: src/xge_drag_drop.c:199（体 14 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_drag_data_object.c:47

## xgeDataObjectFormatCount
- 位置: xge.h:1632  已注释: 否
- 签名: `XGE_API int xgeDataObjectFormatCount(xge_data_object pData);`
- 实现: src/xge_drag_drop.c:214（体 4 行）
- 用法: test/test_drag_data_object.c:49

## xgeDataObjectFormatAt
- 位置: xge.h:1633  已注释: 否
- 签名: `XGE_API const char* xgeDataObjectFormatAt(xge_data_object pData, int iIndex);`
- 实现: src/xge_drag_drop.c:219（体 5 行）
- 返回码: NULL

## xgeDataObjectHas
- 位置: xge.h:1634  已注释: 否
- 签名: `XGE_API int xgeDataObjectHas(xge_data_object pData, const char* sFormat);`
- 实现: src/xge_drag_drop.c:225（体 4 行）
- 用法: test/test_drag_data_object.c:50

## xgeDataObjectGet
- 位置: xge.h:1635  已注释: 否
- 签名: `XGE_API int xgeDataObjectGet(xge_data_object pData, const char* sFormat, void* pOutput, size_t iCapacity, size_t* pOutputSize);`
- 实现: src/xge_drag_drop.c:230（体 22 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK, XGE_ERROR_BUFFER_TOO_SMALL
- 用法: test/test_drag_data_object.c:52; test/test_drag_data_object.c:54

