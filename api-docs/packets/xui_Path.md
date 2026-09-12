# 草稿包：xui.h / Path（10 条 API）

> 生成 2026-09-10 02:52 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiPathCreate
- 位置: xui.h:6090  已注释: 否
- 签名: `XUI_API int xuiPathCreate(xui_path* ppPath);`
- 实现: src/xui_core.c:2747（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_chart_test.c:252; test_xui/xui_vector_smoke_test.c:48; test_xui/xui_vector_xge_test.c:46

## xuiPathDestroy
- 位置: xui.h:6091  已注释: 否
- 签名: `XUI_API void xuiPathDestroy(xui_path pPath);`
- 实现: src/xui_core.c:2764（体 11 行）
- 用法: test_xui/xui_chart_test.c:314; test_xui/xui_chart_test.c:563; test_xui/xui_vector_smoke_test.c:111

## xuiPathClear
- 位置: xui.h:6092  已注释: 否
- 签名: `XUI_API int xuiPathClear(xui_path pPath);`
- 实现: src/xui_core.c:2776（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_vector_smoke_test.c:84

## xuiPathMoveTo
- 位置: xui.h:6093  已注释: 否
- 签名: `XUI_API int xuiPathMoveTo(xui_path pPath, float fX, float fY);`
- 实现: src/xui_core.c:2785（体 4 行）
- 用法: test_xui/xui_chart_test.c:254; test_xui/xui_vector_smoke_test.c:50; test_xui/xui_vector_smoke_test.c:86

## xuiPathLineTo
- 位置: xui.h:6094  已注释: 否
- 签名: `XUI_API int xuiPathLineTo(xui_path pPath, float fX, float fY);`
- 实现: src/xui_core.c:2790（体 4 行）
- 用法: test_xui/xui_chart_test.c:256; test_xui/xui_vector_xge_test.c:48; test_xui/xui_vector_xge_test.c:49

## xuiPathQuadTo
- 位置: xui.h:6095  已注释: 否
- 签名: `XUI_API int xuiPathQuadTo(xui_path pPath, float fCX, float fCY, float fX, float fY);`
- 实现: src/xui_core.c:2795（体 4 行）
- 用法: test_xui/xui_chart_test.c:258; test_xui/xui_vector_smoke_test.c:52

## xuiPathCubicTo
- 位置: xui.h:6096  已注释: 否
- 签名: `XUI_API int xuiPathCubicTo(xui_path pPath, float fC1X, float fC1Y, float fC2X, float fC2Y, float fX, float fY);`
- 实现: src/xui_core.c:2800（体 4 行）
- 用法: test_xui/xui_chart_test.c:260; test_xui/xui_vector_smoke_test.c:51; test_xui/xui_vector_smoke_test.c:88

## xuiPathClose
- 位置: xui.h:6097  已注释: 否
- 签名: `XUI_API int xuiPathClose(xui_path pPath);`
- 实现: src/xui_core.c:2805（体 4 行）
- 用法: test_xui/xui_chart_test.c:262; test_xui/xui_vector_smoke_test.c:53; test_xui/xui_vector_xge_test.c:51

## xuiPathGetCommandCount
- 位置: xui.h:6098  已注释: 否
- 签名: `XUI_API int xuiPathGetCommandCount(xui_path pPath);`
- 实现: src/xui_core.c:2810（体 4 行）
- 用法: test_xui/xui_chart_test.c:264; test_xui/xui_vector_smoke_test.c:54

## xuiPathGetCommand
- 位置: xui.h:6099  已注释: 否
- 签名: `XUI_API int xuiPathGetCommand(xui_path pPath, int iIndex, xui_path_command_t* pCommand);`
- 实现: src/xui_core.c:2815（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_chart_test.c:265

