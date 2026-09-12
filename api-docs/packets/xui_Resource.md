# 草稿包：xui.h / Resource（15 条 API）

> 生成 2026-09-10 02:50 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiResourceSet
- 位置: xui.h:5887  已注释: 否
- 签名: `XUI_API int xuiResourceSet(xui_context pContext, xui_resource* ppResource, const xui_resource_desc_t* pDesc);`
- 实现: src/xui_core.c:2090（体 56 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_icon_test.c:288; test_xui/xui_icon_test.c:333; test_xui/xui_render_schedule_test.c:1426

## xuiResourceFind
- 位置: xui.h:5888  已注释: 否
- 签名: `XUI_API xui_resource xuiResourceFind(xui_context pContext, const char* sName);`
- 实现: src/xui_core.c:2147（体 14 行）
- 返回码: NULL
- 用法: test_xui/xui_render_schedule_test.c:1428; test_xui/xui_render_schedule_test.c:1461

## xuiResourceRemove
- 位置: xui.h:5889  已注释: 否
- 签名: `XUI_API int xuiResourceRemove(xui_resource pResource);`
- 实现: src/xui_core.c:2162（体 30 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_icon_test.c:328; test_xui/xui_render_schedule_test.c:1458; test_xui/xui_render_schedule_test.c:1676

## xuiResourceTouch
- 位置: xui.h:5890  已注释: 否
- 签名: `XUI_API int xuiResourceTouch(xui_resource pResource);`
- 实现: src/xui_core.c:2193（体 34 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_render_schedule_test.c:1449; test_xui/xui_render_schedule_test.c:1456

## xuiResourceGetName
- 位置: xui.h:5891  已注释: 否
- 签名: `XUI_API const char* xuiResourceGetName(xui_resource pResource);`
- 实现: src/xui_core.c:2228（体 4 行）

## xuiResourceGetKind
- 位置: xui.h:5892  已注释: 否
- 签名: `XUI_API int xuiResourceGetKind(xui_resource pResource);`
- 实现: src/xui_core.c:2233（体 4 行）
- 用法: test_xui/xui_render_schedule_test.c:1429

## xuiResourceGetHandle
- 位置: xui.h:5893  已注释: 否
- 签名: `XUI_API void* xuiResourceGetHandle(xui_resource pResource);`
- 实现: src/xui_core.c:2238（体 4 行）
- 用法: test_xui/xui_render_schedule_test.c:1430

## xuiResourceGetGeneration
- 位置: xui.h:5894  已注释: 否
- 签名: `XUI_API uint32_t xuiResourceGetGeneration(xui_resource pResource);`
- 实现: src/xui_core.c:2243（体 4 行）
- 用法: test_xui/xui_render_schedule_test.c:1431; test_xui/xui_render_schedule_test.c:1448; test_xui/xui_render_schedule_test.c:1450

## xuiResourceAddRef
- 位置: xui.h:5895  已注释: 否
- 签名: `XUI_API int xuiResourceAddRef(xui_resource pResource);`
- 实现: src/xui_core.c:2248（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_render_schedule_test.c:1441

## xuiResourceRelease
- 位置: xui.h:5896  已注释: 否
- 签名: `XUI_API int xuiResourceRelease(xui_resource pResource);`
- 实现: src/xui_core.c:2261（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_render_schedule_test.c:1443; test_xui/xui_render_schedule_test.c:1453

## xuiResourceGetRefCount
- 位置: xui.h:5897  已注释: 否
- 签名: `XUI_API int xuiResourceGetRefCount(xui_resource pResource);`
- 实现: src/xui_core.c:2273（体 4 行）
- 用法: test_xui/xui_render_schedule_test.c:1442; test_xui/xui_render_schedule_test.c:1444

## xuiResourceAddDependency
- 位置: xui.h:5898  已注释: 否
- 签名: `XUI_API int xuiResourceAddDependency(xui_resource pResource, xui_resource pDependency);`
- 实现: src/xui_core.c:2278（体 25 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_render_schedule_test.c:1445

## xuiResourceClearDependencies
- 位置: xui.h:5899  已注释: 否
- 签名: `XUI_API void xuiResourceClearDependencies(xui_resource pResource);`
- 实现: src/xui_core.c:2304（体 17 行）
- 用法: test_xui/xui_render_schedule_test.c:1451

## xuiResourceGetDependencyCount
- 位置: xui.h:5900  已注释: 否
- 签名: `XUI_API int xuiResourceGetDependencyCount(xui_resource pResource);`
- 实现: src/xui_core.c:2322（体 14 行）
- 用法: test_xui/xui_render_schedule_test.c:1446; test_xui/xui_render_schedule_test.c:1452

## xuiResourceGetDependency
- 位置: xui.h:5901  已注释: 否
- 签名: `XUI_API xui_resource xuiResourceGetDependency(xui_resource pResource, int iIndex);`
- 实现: src/xui_core.c:2337（体 17 行）
- 返回码: NULL
- 用法: test_xui/xui_render_schedule_test.c:1447

