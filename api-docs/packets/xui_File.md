# 草稿包：xui.h / File（36 条 API）

> 生成 2026-09-10 03:09 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiFileDialogCreate
- 位置: xui.h:9631  已注释: 否
- 签名: `XUI_API int xuiFileDialogCreate(xui_context pContext, xui_file_dialog* ppDialog, const xui_file_dialog_desc_t* pDesc);`
- 实现: src/xui_file_dialog.c:1565（体 247 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK

## xuiFileDialogDestroy
- 位置: xui.h:9632  已注释: 否
- 签名: `XUI_API void xuiFileDialogDestroy(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:1813（体 30 行）
- 用法: examples/xui_file_dialog/main.c:188; examples/xui_file_dialog/main.c:321; test_xui/xui_file_dialog_test.c:324

## xuiFileDialogSetOpen
- 位置: xui.h:9639  已注释: 否
- 签名: `XUI_API int xuiFileDialogSetOpen(xui_file_dialog pDialog, int bOpen);`
- 实现: src/xui_file_dialog.c:1884（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiFileDialogIsOpen
- 位置: xui.h:9640  已注释: 否
- 签名: `XUI_API int xuiFileDialogIsOpen(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:1904（体 4 行）
- 用法: test_xui/xui_file_dialog_test.c:202; test_xui/xui_file_dialog_test.c:306; test_xui/xui_file_dialog_test.c:321

## xuiFileDialogSetResult
- 位置: xui.h:9641  已注释: 否
- 签名: `XUI_API int xuiFileDialogSetResult(xui_file_dialog pDialog, xui_file_dialog_result_proc onResult, void* pUser);`
- 实现: src/xui_file_dialog.c:1909（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiFileDialogSetContextMenu
- 位置: xui.h:9642  已注释: 否
- 签名: `XUI_API int xuiFileDialogSetContextMenu(xui_file_dialog pDialog, xui_file_dialog_context_proc onContext, void* pUser);`
- 实现: src/xui_file_dialog.c:1919（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiFileDialogSetFilter
- 位置: xui.h:9643  已注释: 否
- 签名: `XUI_API int xuiFileDialogSetFilter(xui_file_dialog pDialog, const char* sFilter);`
- 实现: src/xui_file_dialog.c:1927（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_file_dialog_test.c:310; test_xui/xui_file_dialog_test.c:312; test_xui/xui_file_dialog_test.c:314

## xuiFileDialogGetFilter
- 位置: xui.h:9644  已注释: 否
- 签名: `XUI_API const char* xuiFileDialogGetFilter(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:1943（体 4 行）
- 用法: test_xui/xui_file_dialog_test.c:213; test_xui/xui_file_dialog_test.c:311

## xuiFileDialogSetDirectory
- 位置: xui.h:9645  已注释: 否
- 签名: `XUI_API int xuiFileDialogSetDirectory(xui_file_dialog pDialog, const char* sDir);`
- 实现: src/xui_file_dialog.c:1948（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_file_dialog_test.c:268

## xuiFileDialogGetDirectory
- 位置: xui.h:9646  已注释: 否
- 签名: `XUI_API const char* xuiFileDialogGetDirectory(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:1971（体 4 行）
- 用法: test_xui/xui_file_dialog_test.c:255; test_xui/xui_file_dialog_test.c:257; test_xui/xui_file_dialog_test.c:261

## xuiFileDialogGoUp
- 位置: xui.h:9647  已注释: 否
- 签名: `XUI_API int xuiFileDialogGoUp(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:1976（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_file_dialog_test.c:256; test_xui/xui_file_dialog_test.c:294; test_xui/xui_file_dialog_test.c:301

## xuiFileDialogRefresh
- 位置: xui.h:9648  已注释: 否
- 签名: `XUI_API int xuiFileDialogRefresh(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:1996（体 59 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiFileDialogSelectIndex
- 位置: xui.h:9649  已注释: 否
- 签名: `XUI_API int xuiFileDialogSelectIndex(xui_file_dialog pDialog, int iIndex);`
- 实现: src/xui_file_dialog.c:2056（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_file_dialog/main.c:282; test_xui/xui_file_dialog_test.c:318; test_xui/xui_file_dialog_test.c:338

## xuiFileDialogCommit
- 位置: xui.h:9650  已注释: 否
- 签名: `XUI_API int xuiFileDialogCommit(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2066（体 45 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_file_dialog/main.c:283; test_xui/xui_file_dialog_test.c:320; test_xui/xui_file_dialog_test.c:340

## xuiFileDialogCancel
- 位置: xui.h:9651  已注释: 否
- 签名: `XUI_API int xuiFileDialogCancel(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2112（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiFileDialogGetResult
- 位置: xui.h:9652  已注释: 否
- 签名: `XUI_API int xuiFileDialogGetResult(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2121（体 4 行）
- 用法: examples/xui_file_dialog/main.c:286; test_xui/xui_file_dialog_test.c:493

## xuiFileDialogGetResultPath
- 位置: xui.h:9653  已注释: 否
- 签名: `XUI_API const char* xuiFileDialogGetResultPath(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2126（体 4 行）
- 用法: test_xui/xui_file_dialog_test.c:323; test_xui/xui_file_dialog_test.c:350; test_xui/xui_file_dialog_test.c:373

## xuiFileDialogGetWindowWidget
- 位置: xui.h:9654  已注释: 否
- 签名: `XUI_API xui_widget xuiFileDialogGetWindowWidget(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2131（体 19 行）
- 用法: test_xui/xui_file_dialog_test.c:203; test_xui/xui_file_dialog_test.c:226; test_xui/xui_file_dialog_test.c:231

## xuiFileDialogGetRootListWidget
- 位置: xui.h:9655  已注释: 否
- 签名: `XUI_API xui_widget xuiFileDialogGetRootListWidget(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2132（体 18 行）
- 用法: test_xui/xui_file_dialog_test.c:204; test_xui/xui_file_dialog_test.c:228

## xuiFileDialogGetFileListWidget
- 位置: xui.h:9656  已注释: 否
- 签名: `XUI_API xui_widget xuiFileDialogGetFileListWidget(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2133（体 17 行）
- 用法: test_xui/xui_file_dialog_test.c:74; test_xui/xui_file_dialog_test.c:205; test_xui/xui_file_dialog_test.c:229

## xuiFileDialogGetPathBreadcrumbWidget
- 位置: xui.h:9657  已注释: 否
- 签名: `XUI_API xui_widget xuiFileDialogGetPathBreadcrumbWidget(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2134（体 16 行）
- 用法: test_xui/xui_file_dialog_test.c:206; test_xui/xui_file_dialog_test.c:214; test_xui/xui_file_dialog_test.c:247

## xuiFileDialogGetPathInputWidget
- 位置: xui.h:9658  已注释: 否
- 签名: `XUI_API xui_widget xuiFileDialogGetPathInputWidget(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2135（体 15 行）
- 用法: test_xui/xui_file_dialog_test.c:214; test_xui/xui_file_dialog_test.c:251; test_xui/xui_file_dialog_test.c:251

## xuiFileDialogGetNameInputWidget
- 位置: xui.h:9659  已注释: 否
- 签名: `XUI_API xui_widget xuiFileDialogGetNameInputWidget(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2136（体 14 行）
- 用法: test_xui/xui_file_dialog_test.c:219; test_xui/xui_file_dialog_test.c:236; test_xui/xui_file_dialog_test.c:290

## xuiFileDialogGetFilterComboWidget
- 位置: xui.h:9660  已注释: 否
- 签名: `XUI_API xui_widget xuiFileDialogGetFilterComboWidget(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2137（体 13 行）
- 用法: test_xui/xui_file_dialog_test.c:207; test_xui/xui_file_dialog_test.c:220; test_xui/xui_file_dialog_test.c:237

## xuiFileDialogGetUpButtonWidget
- 位置: xui.h:9661  已注释: 否
- 签名: `XUI_API xui_widget xuiFileDialogGetUpButtonWidget(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2138（体 12 行）
- 用法: test_xui/xui_file_dialog_test.c:215; test_xui/xui_file_dialog_test.c:262

## xuiFileDialogGetRefreshButtonWidget
- 位置: xui.h:9662  已注释: 否
- 签名: `XUI_API xui_widget xuiFileDialogGetRefreshButtonWidget(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2139（体 11 行）
- 用法: test_xui/xui_file_dialog_test.c:216; test_xui/xui_file_dialog_test.c:266

## xuiFileDialogGetOkButtonWidget
- 位置: xui.h:9663  已注释: 否
- 签名: `XUI_API xui_widget xuiFileDialogGetOkButtonWidget(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2140（体 10 行）
- 用法: test_xui/xui_file_dialog_test.c:208; test_xui/xui_file_dialog_test.c:222; test_xui/xui_file_dialog_test.c:223

## xuiFileDialogGetCancelButtonWidget
- 位置: xui.h:9664  已注释: 否
- 签名: `XUI_API xui_widget xuiFileDialogGetCancelButtonWidget(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2141（体 9 行）
- 用法: test_xui/xui_file_dialog_test.c:209

## xuiFileDialogGetOverwriteMsgBox
- 位置: xui.h:9665  已注释: 否
- 签名: `XUI_API xui_msgbox xuiFileDialogGetOverwriteMsgBox(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2142（体 8 行）
- 用法: test_xui/xui_file_dialog_test.c:412; test_xui/xui_file_dialog_test.c:413; test_xui/xui_file_dialog_test.c:416

## xuiFileDialogGetFilterCount
- 位置: xui.h:9666  已注释: 否
- 签名: `XUI_API int xuiFileDialogGetFilterCount(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2143（体 7 行）
- 用法: test_xui/xui_file_dialog_test.c:210

## xuiFileDialogGetFilterName
- 位置: xui.h:9667  已注释: 否
- 签名: `XUI_API const char* xuiFileDialogGetFilterName(xui_file_dialog pDialog, int iIndex);`
- 实现: src/xui_file_dialog.c:2145（体 5 行）
- 用法: test_xui/xui_file_dialog_test.c:211

## xuiFileDialogGetFilterPattern
- 位置: xui.h:9668  已注释: 否
- 签名: `XUI_API const char* xuiFileDialogGetFilterPattern(xui_file_dialog pDialog, int iIndex);`
- 实现: src/xui_file_dialog.c:2151（体 5 行）
- 用法: test_xui/xui_file_dialog_test.c:212

## xuiFileDialogGetEntryCount
- 位置: xui.h:9669  已注释: 否
- 签名: `XUI_API int xuiFileDialogGetEntryCount(xui_file_dialog pDialog);`
- 实现: src/xui_file_dialog.c:2157（体 7 行）
- 用法: examples/xui_file_dialog/main.c:280; test_xui/xui_file_dialog_test.c:39; test_xui/xui_file_dialog_test.c:490

## xuiFileDialogGetEntryName
- 位置: xui.h:9670  已注释: 否
- 签名: `XUI_API const char* xuiFileDialogGetEntryName(xui_file_dialog pDialog, int iIndex);`
- 实现: src/xui_file_dialog.c:2159（体 5 行）
- 用法: test_xui/xui_file_dialog_test.c:41

## xuiFileDialogGetEntryPath
- 位置: xui.h:9671  已注释: 否
- 签名: `XUI_API const char* xuiFileDialogGetEntryPath(xui_file_dialog pDialog, int iIndex);`
- 实现: src/xui_file_dialog.c:2165（体 5 行）

## xuiFileDialogEntryIsDir
- 位置: xui.h:9672  已注释: 否
- 签名: `XUI_API int xuiFileDialogEntryIsDir(xui_file_dialog pDialog, int iIndex);`
- 实现: src/xui_file_dialog.c:2171（体 5 行）
- 用法: test_xui/xui_file_dialog_test.c:447

