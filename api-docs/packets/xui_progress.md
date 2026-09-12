# 草稿包：xui.h / progress（35 条 API）

> 生成 2026-09-10 03:03 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiProgressGetType
- 位置: xui.h:6593  已注释: 否
- 签名: `XUI_API xui_widget_type xuiProgressGetType(xui_context pContext);`
- 实现: src/xui_progress.c:951（体 33 行）
- 返回码: NULL

## xuiProgressCreate
- 位置: xui.h:6594  已注释: 否
- 签名: `XUI_API int xuiProgressCreate(xui_context pContext, xui_widget* ppWidget, const xui_progress_desc_t* pDesc);`
- 实现: src/xui_progress.c:985（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/audit_xui_basic/main.c:76; examples/audit_xui_basic/main.c:78; examples/audit_xui_basic/main.c:80

## xuiProgressSetRange
- 位置: xui.h:6595  已注释: 否
- 签名: `XUI_API int xuiProgressSetRange(xui_widget pWidget, float fMin, float fMax);`
- 实现: src/xui_progress.c:1000（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_progress_test.c:129

## xuiProgressGetRange
- 位置: xui.h:6596  已注释: 否
- 签名: `XUI_API int xuiProgressGetRange(xui_widget pWidget, float* pMin, float* pMax);`
- 实现: src/xui_progress.c:1020（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_progress_test.c:131

## xuiProgressSetValue
- 位置: xui.h:6597  已注释: 否
- 签名: `XUI_API int xuiProgressSetValue(xui_widget pWidget, float fValue);`
- 实现: src/xui_progress.c:1037（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_progress/main.c:447; test_xui/xui_progress_test.c:133; test_xui/xui_progress_test.c:141

## xuiProgressGetValue
- 位置: xui.h:6598  已注释: 否
- 签名: `XUI_API float xuiProgressGetValue(xui_widget pWidget);`
- 实现: src/xui_progress.c:1058（体 7 行）
- 用法: test_xui/xui_progress_test.c:115; test_xui/xui_progress_test.c:134

## xuiProgressGetRate
- 位置: xui.h:6599  已注释: 否
- 签名: `XUI_API float xuiProgressGetRate(xui_widget pWidget);`
- 实现: src/xui_progress.c:1066（体 7 行）
- 用法: test_xui/xui_progress_test.c:116; test_xui/xui_progress_test.c:134

## xuiProgressSetText
- 位置: xui.h:6600  已注释: 否
- 签名: `XUI_API int xuiProgressSetText(xui_widget pWidget, xui_font pFont, const char* sText);`
- 实现: src/xui_progress.c:1074（体 10 行）

## xuiProgressSetTextTemplate
- 位置: xui.h:6601  已注释: 否
- 签名: `XUI_API int xuiProgressSetTextTemplate(xui_widget pWidget, const char* sTextTemplate);`
- 实现: src/xui_progress.c:1085（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_progress_test.c:161; test_xui/xui_progress_test.c:163; test_xui/xui_progress_test.c:165

## xuiProgressGetTextTemplate
- 位置: xui.h:6602  已注释: 否
- 签名: `XUI_API const char* xuiProgressGetTextTemplate(xui_widget pWidget);`
- 实现: src/xui_progress.c:1105（体 7 行）
- 用法: examples/xui_progress/main.c:455; test_xui/xui_progress_test.c:166

## xuiProgressGetDisplayText
- 位置: xui.h:6603  已注释: 否
- 签名: `XUI_API const char* xuiProgressGetDisplayText(xui_widget pWidget);`
- 实现: src/xui_progress.c:1113（体 7 行）
- 用法: examples/xui_progress/main.c:456; examples/xui_progress/main.c:457; test_xui/xui_progress_test.c:117

## xuiProgressSetFont
- 位置: xui.h:6604  已注释: 否
- 签名: `XUI_API int xuiProgressSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_progress.c:1121（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiProgressGetFont
- 位置: xui.h:6605  已注释: 否
- 签名: `XUI_API xui_font xuiProgressGetFont(xui_widget pWidget);`
- 实现: src/xui_progress.c:1136（体 7 行）

## xuiProgressSetTextColor
- 位置: xui.h:6606  已注释: 否
- 签名: `XUI_API int xuiProgressSetTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_progress.c:1144（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiProgressGetTextColor
- 位置: xui.h:6607  已注释: 否
- 签名: `XUI_API uint32_t xuiProgressGetTextColor(xui_widget pWidget);`
- 实现: src/xui_progress.c:1159（体 7 行）

## xuiProgressSetFillTextColor
- 位置: xui.h:6608  已注释: 否
- 签名: `XUI_API int xuiProgressSetFillTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_progress.c:1167（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiProgressGetFillTextColor
- 位置: xui.h:6609  已注释: 否
- 签名: `XUI_API uint32_t xuiProgressGetFillTextColor(xui_widget pWidget);`
- 实现: src/xui_progress.c:1182（体 7 行）

## xuiProgressSetTextFlags
- 位置: xui.h:6610  已注释: 否
- 签名: `XUI_API int xuiProgressSetTextFlags(xui_widget pWidget, uint32_t iTextFlags);`
- 实现: src/xui_progress.c:1190（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiProgressGetTextFlags
- 位置: xui.h:6611  已注释: 否
- 签名: `XUI_API uint32_t xuiProgressGetTextFlags(xui_widget pWidget);`
- 实现: src/xui_progress.c:1206（体 7 行）

## xuiProgressSetColors
- 位置: xui.h:6612  已注释: 否
- 签名: `XUI_API int xuiProgressSetColors(xui_widget pWidget, uint32_t iTrack, uint32_t iFill);`
- 实现: src/xui_progress.c:1214（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiProgressGetTrackColor
- 位置: xui.h:6613  已注释: 否
- 签名: `XUI_API uint32_t xuiProgressGetTrackColor(xui_widget pWidget);`
- 实现: src/xui_progress.c:1230（体 7 行）

## xuiProgressGetFillColor
- 位置: xui.h:6614  已注释: 否
- 签名: `XUI_API uint32_t xuiProgressGetFillColor(xui_widget pWidget);`
- 实现: src/xui_progress.c:1238（体 7 行）

## xuiProgressSetFillDirection
- 位置: xui.h:6615  已注释: 否
- 签名: `XUI_API int xuiProgressSetFillDirection(xui_widget pWidget, int iFillDirection);`
- 实现: src/xui_progress.c:1246（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_progress_test.c:144; test_xui/xui_progress_test.c:148; test_xui/xui_progress_test.c:156

## xuiProgressGetFillDirection
- 位置: xui.h:6616  已注释: 否
- 签名: `XUI_API int xuiProgressGetFillDirection(xui_widget pWidget);`
- 实现: src/xui_progress.c:1264（体 7 行）

## xuiProgressSetTrackPatch
- 位置: xui.h:6617  已注释: 否
- 签名: `XUI_API int xuiProgressSetTrackPatch(xui_widget pWidget, const xui_nine_patch_t* pPatch);`
- 实现: src/xui_progress.c:1272（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_progress/main.c:355; examples/xui_progress/main.c:373; test_xui/xui_progress_test.c:190

## xuiProgressClearTrackPatch
- 位置: xui.h:6618  已注释: 否
- 签名: `XUI_API int xuiProgressClearTrackPatch(xui_widget pWidget);`
- 实现: src/xui_progress.c:1291（体 4 行）

## xuiProgressHasTrackPatch
- 位置: xui.h:6619  已注释: 否
- 签名: `XUI_API int xuiProgressHasTrackPatch(xui_widget pWidget);`
- 实现: src/xui_progress.c:1296（体 7 行）
- 用法: examples/xui_progress/main.c:460; test_xui/xui_progress_test.c:191

## xuiProgressGetTrackPatch
- 位置: xui.h:6620  已注释: 否
- 签名: `XUI_API int xuiProgressGetTrackPatch(xui_widget pWidget, xui_nine_patch_t* pPatch);`
- 实现: src/xui_progress.c:1304（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiProgressSetFillPatch
- 位置: xui.h:6621  已注释: 否
- 签名: `XUI_API int xuiProgressSetFillPatch(xui_widget pWidget, const xui_nine_patch_t* pPatch);`
- 实现: src/xui_progress.c:1320（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_progress/main.c:356; examples/xui_progress/main.c:374; test_xui/xui_progress_test.c:192

## xuiProgressClearFillPatch
- 位置: xui.h:6622  已注释: 否
- 签名: `XUI_API int xuiProgressClearFillPatch(xui_widget pWidget);`
- 实现: src/xui_progress.c:1339（体 4 行）

## xuiProgressHasFillPatch
- 位置: xui.h:6623  已注释: 否
- 签名: `XUI_API int xuiProgressHasFillPatch(xui_widget pWidget);`
- 实现: src/xui_progress.c:1344（体 7 行）
- 用法: examples/xui_progress/main.c:458; test_xui/xui_progress_test.c:193

## xuiProgressGetFillPatch
- 位置: xui.h:6624  已注释: 否
- 签名: `XUI_API int xuiProgressGetFillPatch(xui_widget pWidget, xui_nine_patch_t* pPatch);`
- 实现: src/xui_progress.c:1352（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiProgressSetFillPatchMode
- 位置: xui.h:6625  已注释: 否
- 签名: `XUI_API int xuiProgressSetFillPatchMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_progress.c:1368（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_progress/main.c:357; test_xui/xui_progress_test.c:194; test_xui/xui_progress_test.c:208

## xuiProgressGetFillPatchMode
- 位置: xui.h:6626  已注释: 否
- 签名: `XUI_API int xuiProgressGetFillPatchMode(xui_widget pWidget);`
- 实现: src/xui_progress.c:1386（体 7 行）
- 用法: examples/xui_progress/main.c:459; test_xui/xui_progress_test.c:195

## xuiProgressGetFillRect
- 位置: xui.h:6627  已注释: 否
- 签名: `XUI_API xui_rect_t xuiProgressGetFillRect(xui_widget pWidget);`
- 实现: src/xui_progress.c:1394（体 12 行）
- 用法: test_xui/xui_progress_test.c:118; test_xui/xui_progress_test.c:146; test_xui/xui_progress_test.c:154

