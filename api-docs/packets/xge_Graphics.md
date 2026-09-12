# 草稿包：xge.h / Graphics（6 条 API）

> 生成 2026-09-10 02:41 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeGraphicsBackendDefault
- 位置: xge.h:1614  已注释: 否
- 签名: `XGE_API xge_graphics_backend_t xgeGraphicsBackendDefault(void);`
- 实现: src/xge_core.c:543（体 9 行）

## xgeGraphicsBackendSet
- 位置: xge.h:1615  已注释: 否
- 签名: `XGE_API int xgeGraphicsBackendSet(const xge_graphics_backend_t* pBackend);`
- 实现: src/xge_core.c:553（体 12 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeGraphicsBackendGet
- 位置: xge.h:1616  已注释: 否
- 签名: `XGE_API xge_graphics_backend_t xgeGraphicsBackendGet(void);`
- 实现: src/xge_core.c:566（体 7 行）
- 用法: examples/tutorial_capture/ch10_main1.c:10

## xgeGraphicsShaderHeaderGet
- 位置: xge.h:1661  已注释: 是
- 签名: `XGE_API int xgeGraphicsShaderHeaderGet(int iBackend, char* sBuffer, int iSize);`
- 既有注释: /* Writes a NUL-terminated string. Returns XGE_OK, or XGE_ERROR_BUFFER_TOO_SMALL * after writing a truncated NUL-terminated value. */
- 实现: src/xge_core.c:632（体 6 行）
- 用法: test/test_input_event_queue.c:77; test/test_input_event_queue.c:79

## xgeGraphicsLibraryNameGet
- 位置: xge.h:1662  已注释: 否
- 签名: `XGE_API int xgeGraphicsLibraryNameGet(int iBackend, int iIndex, char* sBuffer, int iSize);`
- 实现: src/xge_core.c:639（体 20 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_FILE_NOT_FOUND, XGE_ERROR_BUFFER_TOO_SMALL, XGE_OK

## xgeGraphicsMappingGet
- 位置: xge.h:1663  已注释: 否
- 签名: `XGE_API int xgeGraphicsMappingGet(int iBackend, xge_graphics_mapping_t* pMapping);`
- 实现: src/xge_core.c:660（体 28 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT

