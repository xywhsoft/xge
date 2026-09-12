# 草稿包：xui.h / Find（6 条 API）

> 生成 2026-09-10 02:52 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiFindFont
- 位置: xui.h:5917  已注释: 否
- 签名: `XUI_API xui_font xuiFindFont(xui_context pContext, const char* sName);`
- 实现: src/xui_core.c:2015（体 14 行）
- 返回码: NULL
- 用法: test_xui/xui_style_test.c:318; test_xui/xui_style_test.c:322

## xuiFindText
- 位置: xui.h:6583  已注释: 否
- 签名: `XUI_API int xuiFindText(const char* sText, int iTextLength, const char* sPattern, int iStartOffset, int iRangeStart, int iRangeEnd, uint32_t iFlags, xui_find_result_t* pResult, char* sError, int iErrorCapacity);`
- 实现: src/xui_find.c:537（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_find_test.c:32; test_xui/xui_find_test.c:34; test_xui/xui_find_test.c:36

## xuiFindCollectText
- 位置: xui.h:6584  已注释: 否
- 签名: `XUI_API int xuiFindCollectText(const char* sText, int iTextLength, const char* sPattern, int iRangeStart, int iRangeEnd, uint32_t iFlags, xui_find_result_t* pResults, int iResultCapacity, int* pResultCount, char* sError, int iErrorCapacity);`
- 实现: src/xui_find.c:554（体 30 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_find_test.c:53

## xuiFindReplaceAllText
- 位置: xui.h:6585  已注释: 否
- 签名: `XUI_API int xuiFindReplaceAllText(const char* sText, int iTextLength, const char* sPattern, const char* sReplacement, int iRangeStart, int iRangeEnd, uint32_t iFlags, char** psOutput, int* pOutputLength, int* pReplaceCount, char* sError, int iErrorCapacity);`
- 实现: src/xui_find.c:585（体 63 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_find_test.c:63; test_xui/xui_find_test.c:70

## xuiFindBuildReplacement
- 位置: xui.h:6586  已注释: 否
- 签名: `XUI_API int xuiFindBuildReplacement(const char* sText, int iTextLength, const char* sPattern, const char* sReplacement, uint32_t iFlags, const xui_find_result_t* pResult, char** psOutput, int* pOutputLength, char* sError, int iErrorCapacity);`
- 实现: src/xui_find.c:649（体 28 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiFindFreeText
- 位置: xui.h:6589  已注释: 否
- 签名: `XUI_API void xuiFindFreeText(char* sText);`
- 实现: src/xui_find.c:678（体 4 行）
- 用法: test_xui/xui_find_test.c:67; test_xui/xui_find_test.c:75

