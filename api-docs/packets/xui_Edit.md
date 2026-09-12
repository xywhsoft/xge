# 草稿包：xui.h / Edit（22 条 API）

> 生成 2026-09-10 02:50 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiEditGetCapabilities
- 位置: xui.h:6881  已注释: 是
- 签名: `XUI_API uint32_t xuiEditGetCapabilities(xui_widget pWidget);`
- 既有注释: /* * Common editing surface contract. Text-control offsets are UTF-8 byte offsets; * Terminal selection offsets encode line * columns + column. Caret rectangles * are widget-local. These APIs do not replace type-specific structured data APIs. */
- 实现: src/xui_edit.c:137（体 5 行）
- 用法: test_xui/xui_edit_contract_test.c:86; test_xui/xui_edit_contract_test.c:123; test_xui/xui_edit_contract_test.c:132

## xuiEditSetEvent
- 位置: xui.h:6882  已注释: 否
- 签名: `XUI_API int xuiEditSetEvent(xui_widget pWidget, xui_edit_event_proc onEvent, void* pUser);`
- 实现: src/xui_edit.c:143（体 7 行）
- 返回码: XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_edit_contract_test.c:92; test_xui/xui_edit_contract_test.c:181

## xuiEditSetBehavior
- 位置: xui.h:6883  已注释: 否
- 签名: `XUI_API int xuiEditSetBehavior(xui_widget pWidget, const xui_edit_behavior_t* pBehavior);`
- 实现: src/xui_edit.c:151（体 8 行）
- 返回码: XUI_ERROR_UNSUPPORTED, XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_edit_contract_test.c:107

## xuiEditGetBehavior
- 位置: xui.h:6884  已注释: 否
- 签名: `XUI_API int xuiEditGetBehavior(xui_widget pWidget, xui_edit_behavior_t* pBehavior);`
- 实现: src/xui_edit.c:160（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_edit_contract_test.c:110

## xuiEditSetText
- 位置: xui.h:6885  已注释: 否
- 签名: `XUI_API int xuiEditSetText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_edit.c:174（体 6 行）
- 返回码: XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_edit_contract_test.c:93; test_xui/xui_edit_contract_test.c:115; test_xui/xui_edit_contract_test.c:134

## xuiEditGetText
- 位置: xui.h:6886  已注释: 否
- 签名: `XUI_API const char* xuiEditGetText(xui_widget pWidget);`
- 实现: src/xui_edit.c:181（体 5 行）
- 用法: test_xui/xui_accessibility_test.c:516; test_xui/xui_edit_contract_test.c:93; test_xui/xui_edit_contract_test.c:134

## xuiEditSetSelection
- 位置: xui.h:6887  已注释: 否
- 签名: `XUI_API int xuiEditSetSelection(xui_widget pWidget, int iStart, int iEnd);`
- 实现: src/xui_edit.c:187（体 6 行）
- 返回码: XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_edit_contract_test.c:96; test_xui/xui_edit_contract_test.c:124; test_xui/xui_edit_contract_test.c:159

## xuiEditGetSelection
- 位置: xui.h:6888  已注释: 否
- 签名: `XUI_API int xuiEditGetSelection(xui_widget pWidget, int* pStart, int* pEnd);`
- 实现: src/xui_edit.c:194（体 6 行）
- 返回码: XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_edit_contract_test.c:97

## xuiEditHasSelection
- 位置: xui.h:6889  已注释: 否
- 签名: `XUI_API int xuiEditHasSelection(xui_widget pWidget);`
- 实现: src/xui_edit.c:201（体 5 行）
- 用法: test_xui/xui_edit_contract_test.c:124; test_xui/xui_edit_contract_test.c:159

## xuiEditSelectAll
- 位置: xui.h:6890  已注释: 否
- 签名: `XUI_API int xuiEditSelectAll(xui_widget pWidget);`
- 实现: src/xui_edit.c:207（体 13 行）

## xuiEditCopy
- 位置: xui.h:6891  已注释: 否
- 签名: `XUI_API int xuiEditCopy(xui_widget pWidget);`
- 实现: src/xui_edit.c:208（体 12 行）

## xuiEditCut
- 位置: xui.h:6892  已注释: 否
- 签名: `XUI_API int xuiEditCut(xui_widget pWidget);`
- 实现: src/xui_edit.c:209（体 11 行）

## xuiEditPaste
- 位置: xui.h:6893  已注释: 否
- 签名: `XUI_API int xuiEditPaste(xui_widget pWidget);`
- 实现: src/xui_edit.c:210（体 10 行）

## xuiEditDeleteSelection
- 位置: xui.h:6894  已注释: 否
- 签名: `XUI_API int xuiEditDeleteSelection(xui_widget pWidget);`
- 实现: src/xui_edit.c:211（体 9 行）

## xuiEditUndo
- 位置: xui.h:6895  已注释: 否
- 签名: `XUI_API int xuiEditUndo(xui_widget pWidget);`
- 实现: src/xui_edit.c:212（体 8 行）

## xuiEditRedo
- 位置: xui.h:6896  已注释: 否
- 签名: `XUI_API int xuiEditRedo(xui_widget pWidget);`
- 实现: src/xui_edit.c:213（体 7 行）

## xuiEditCanUndo
- 位置: xui.h:6897  已注释: 否
- 签名: `XUI_API int xuiEditCanUndo(xui_widget pWidget);`
- 实现: src/xui_edit.c:215（体 5 行）

## xuiEditCanRedo
- 位置: xui.h:6898  已注释: 否
- 签名: `XUI_API int xuiEditCanRedo(xui_widget pWidget);`
- 实现: src/xui_edit.c:221（体 5 行）

## xuiEditSetReadonly
- 位置: xui.h:6899  已注释: 否
- 签名: `XUI_API int xuiEditSetReadonly(xui_widget pWidget, int bReadonly);`
- 实现: src/xui_edit.c:227（体 6 行）
- 返回码: XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_accessibility_test.c:519; test_xui/xui_edit_contract_test.c:114; test_xui/xui_edit_contract_test.c:145

## xuiEditIsReadonly
- 位置: xui.h:6900  已注释: 否
- 签名: `XUI_API int xuiEditIsReadonly(xui_widget pWidget);`
- 实现: src/xui_edit.c:234（体 5 行）
- 用法: test_xui/xui_edit_contract_test.c:114; test_xui/xui_edit_contract_test.c:172

## xuiEditGetCaretRect
- 位置: xui.h:6901  已注释: 否
- 签名: `XUI_API xui_rect_t xuiEditGetCaretRect(xui_widget pWidget);`
- 实现: src/xui_edit.c:240（体 6 行）
- 用法: test_xui/xui_terminal_test.c:1112

## xuiEditOpenContextMenu
- 位置: xui.h:6902  已注释: 否
- 签名: `XUI_API int xuiEditOpenContextMenu(xui_widget pWidget, float fX, float fY);`
- 实现: src/xui_edit.c:247（体 6 行）
- 返回码: XUI_ERROR_UNSUPPORTED

