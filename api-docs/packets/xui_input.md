# 草稿包：xui.h / input（78 条 API）

> 生成 2026-09-10 03:09 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiInputPointerMove
- 位置: xui.h:6194  已注释: 是
- 签名: `XUI_API int xuiInputPointerMove(xui_context pContext, int iX, int iY, uint32_t iButtons);`
- 既有注释: /* 注入主指针移动事件（入队，经派发管线处理）。 */
- 实现: src/xui_input.c:1852（体 4 行）
- 用法: examples/xui_accordion/main.c:249; examples/xui_breadcrumb/main.c:261; examples/xui_button/main.c:524

## xuiInputPointerDown
- 位置: xui.h:6196  已注释: 是
- 签名: `XUI_API int xuiInputPointerDown(xui_context pContext, int iX, int iY, int iButton, uint32_t iButtons);`
- 既有注释: /* 注入主指针按下事件。 */
- 实现: src/xui_input.c:1857（体 4 行）
- 用法: examples/xui_accordion/main.c:251; examples/xui_breadcrumb/main.c:265; examples/xui_button/main.c:526

## xuiInputPointerUp
- 位置: xui.h:6198  已注释: 是
- 签名: `XUI_API int xuiInputPointerUp(xui_context pContext, int iX, int iY, int iButton, uint32_t iButtons);`
- 既有注释: /* 注入主指针释放事件。 */
- 实现: src/xui_input.c:1862（体 4 行）
- 用法: examples/xui_accordion/main.c:253; examples/xui_breadcrumb/main.c:269; examples/xui_button/main.c:528

## xuiInputPointerWheel
- 位置: xui.h:6200  已注释: 是
- 签名: `XUI_API int xuiInputPointerWheel(xui_context pContext, int iX, int iY, float fWheelX, float fWheelY, uint32_t iButtons);`
- 既有注释: /* 注入滚轮事件（X/Y 双向）。 */
- 实现: src/xui_input.c:1867（体 4 行）
- 用法: examples/xui_numericinput/main.c:325; examples/xui_popup/main.c:301; examples/xui_scrollview/main.c:329

## xuiInputPointerLeave
- 位置: xui.h:6202  已注释: 是
- 签名: `XUI_API int xuiInputPointerLeave(xui_context pContext);`
- 既有注释: /* 注入主指针离开视口事件。 */
- 实现: src/xui_input.c:1890（体 8 行）
- 用法: test_xui/xui_input_test.c:637

## xuiInputPointerMoveEx
- 位置: xui.h:6204  已注释: 是
- 签名: `XUI_API int xuiInputPointerMoveEx(xui_context pContext, uint64_t iPointerId, int iPointerType, int iX, int iY, uint32_t iButtons);`
- 既有注释: /* 按指针 id 注入移动事件（多点/触控笔）。 */
- 实现: src/xui_input.c:1686（体 8 行）
- 用法: examples/xui_multitouch/main.c:375; examples/xui_multitouch/main.c:379; examples/xui_multitouch/main.c:486

## xuiInputPointerDownEx
- 位置: xui.h:6206  已注释: 是
- 签名: `XUI_API int xuiInputPointerDownEx(xui_context pContext, uint64_t iPointerId, int iPointerType, int iX, int iY, int iButton, uint32_t iButtons);`
- 既有注释: /* 按指针 id 注入按下事件。 */
- 实现: src/xui_input.c:1714（体 8 行）
- 用法: examples/xui_multitouch/main.c:376; examples/xui_multitouch/main.c:488; examples/xui_multitouch/main.c:493

## xuiInputPointerUpEx
- 位置: xui.h:6208  已注释: 是
- 签名: `XUI_API int xuiInputPointerUpEx(xui_context pContext, uint64_t iPointerId, int iPointerType, int iX, int iY, int iButton, uint32_t iButtons);`
- 既有注释: /* 按指针 id 注入释放事件。 */
- 实现: src/xui_input.c:1744（体 8 行）
- 用法: examples/xui_multitouch/main.c:381; examples/xui_multitouch/main.c:500; examples/xui_virtual_joystick/main.c:268

## xuiInputPointerWheelEx
- 位置: xui.h:6210  已注释: 是
- 签名: `XUI_API int xuiInputPointerWheelEx(xui_context pContext, uint64_t iPointerId, int iPointerType, int iX, int iY, float fWheelX, float fWheelY, uint32_t iButtons);`
- 既有注释: /* 按指针 id 注入滚轮事件。 */
- 实现: src/xui_input.c:1771（体 8 行）

## xuiInputPointerCancelEx
- 位置: xui.h:6212  已注释: 是
- 签名: `XUI_API int xuiInputPointerCancelEx(xui_context pContext, uint64_t iPointerId, int iPointerType);`
- 既有注释: /* 按指针 id 注入取消事件（系统打断）。 */
- 实现: src/xui_input.c:1810（体 8 行）
- 用法: examples/xui_multitouch/main.c:385; examples/xui_multitouch/main.c:505; examples/xui_virtual_joystick/main.c:270

## xuiInputCancelAllPointers
- 位置: xui.h:6214  已注释: 是
- 签名: `XUI_API int xuiInputCancelAllPointers(xui_context pContext);`
- 既有注释: /* 取消全部活跃指针（窗口失活时）。 */
- 实现: src/xui_input.c:1843（体 8 行）

## xuiInputSetModifiers
- 位置: xui.h:6216  已注释: 是
- 签名: `XUI_API int xuiInputSetModifiers(xui_context pContext, uint32_t iModifiers);`
- 既有注释: /* 覆盖当前键盘修饰位。 */
- 实现: src/xui_input.c:1899（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_input/main.c:492; examples/xui_input/main.c:502; test_xui/xui_chart_breadcrumb_keyboard_test.c:562

## xuiInputGetModifiers
- 位置: xui.h:6218  已注释: 是
- 签名: `XUI_API uint32_t xuiInputGetModifiers(xui_context pContext);`
- 既有注释: /* 读取当前修饰位。 */
- 实现: src/xui_input.c:1908（体 4 行）
- 用法: test_xui/xui_input_test.c:484; test_xui/xui_input_test.c:529

## xuiInputKeyDown
- 位置: xui.h:6220  已注释: 是
- 签名: `XUI_API int xuiInputKeyDown(xui_context pContext, int iKey, uint32_t iModifiers);`
- 既有注释: /* 注入主键盘按下（键码经热键与焦点链派发）。 */
- 实现: src/xui_input.c:2197（体 4 行）
- 用法: examples/xui_cascader/main.c:336; examples/xui_combobox/main.c:394; examples/xui_iconpicker/main.c:337

## xuiInputKeyUp
- 位置: xui.h:6222  已注释: 是
- 签名: `XUI_API int xuiInputKeyUp(xui_context pContext, int iKey, uint32_t iModifiers);`
- 既有注释: /* 注入主键盘释放。 */
- 实现: src/xui_input.c:2237（体 4 行）
- 用法: examples/xui_input/main.c:498; examples/xui_taginput/main.c:268; test_xui/xui_button_test.c:900

## xuiInputText
- 位置: xui.h:6224  已注释: 是
- 签名: `XUI_API int xuiInputText(xui_context pContext, uint32_t iCodepoint);`
- 既有注释: /* 注入文本输入（UTF-8 码点串）。 */
- 实现: src/xui_input.c:2275（体 4 行）
- 用法: examples/xui_codeedit/main.c:359; examples/xui_codeedit/main.c:360; examples/xui_input/main.c:566

## xuiInputKeyDownEx
- 位置: xui.h:6226  已注释: 是
- 签名: `XUI_API int xuiInputKeyDownEx(xui_context pContext, int iKey, uint32_t iModifiers, uint32_t* pResult);`
- 既有注释: /* 注入带设备修饰的键盘按下。 */
- 实现: src/xui_input.c:2188（体 8 行）
- 用法: test_xui/xui_input_test.c:506; test_xui/xui_input_test.c:516; test_xui/xui_input_test.c:838

## xuiInputKeyUpEx
- 位置: xui.h:6228  已注释: 是
- 签名: `XUI_API int xuiInputKeyUpEx(xui_context pContext, int iKey, uint32_t iModifiers, uint32_t* pResult);`
- 既有注释: /* 注入带设备修饰的键盘释放。 */
- 实现: src/xui_input.c:2228（体 8 行）

## xuiInputTextEx
- 位置: xui.h:6230  已注释: 是
- 签名: `XUI_API int xuiInputTextEx(xui_context pContext, uint32_t iCodepoint, uint32_t* pResult);`
- 既有注释: /* 注入带来源标志的文本输入。 */
- 实现: src/xui_input.c:2266（体 8 行）

## xuiInputImeComposition
- 位置: xui.h:6232  已注释: 是
- 签名: `XUI_API int xuiInputImeComposition(xui_context pContext, const char* sText, int iTextSize, int iCompositionStart, int iCompositionLength);`
- 既有注释: /* 注入输入法组合串事件（更新预编辑）。 */
- 实现: src/xui_input.c:2349（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_edit_test.c:132; test_xui/xui_input_test.c:906; test_xui/xui_input_widget_test.c:92

## xuiInputImeCompositionEx
- 位置: xui.h:6234  已注释: 是
- 签名: `XUI_API int xuiInputImeCompositionEx(xui_context pContext, const xui_ime_composition_t* pComposition);`
- 既有注释: /* 注入带光标/候选区间的组合事件。 */
- 实现: src/xui_input.c:2336（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_edit_test.c:154; test_xui/xui_input_test.c:925; test_xui/xui_input_test.c:941

## xuiInputViewport
- 位置: xui.h:6236  已注释: 是
- 签名: `XUI_API int xuiInputViewport(xui_context pContext, int iWidth, int iHeight);`
- 既有注释: /* 通知视口尺寸变更（派发 VIEWPORT 事件）。 */
- 实现: src/xui_input.c:2371（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_accordion/main.c:310; examples/xui_breadcrumb/main.c:289; examples/xui_button/main.c:552

## xuiInputDpi
- 位置: xui.h:6238  已注释: 是
- 签名: `XUI_API int xuiInputDpi(xui_context pContext, float fDpiScale);`
- 既有注释: /* 通知 DPI 变更（级联尺寸与缓存失效）。 */
- 实现: src/xui_input.c:2398（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_input_test.c:549; test_xui/xui_render_schedule_test.c:1632

## xuiInputGetType
- 位置: xui.h:7585  已注释: 否
- 签名: `XUI_API xui_widget_type xuiInputGetType(xui_context pContext);`
- 实现: src/xui_input_widget.c:2783（体 35 行）
- 返回码: NULL
- 用法: test_xui/xui_code_edit_test.c:1107

## xuiInputCreate
- 位置: xui.h:7586  已注释: 否
- 签名: `XUI_API int xuiInputCreate(xui_context pContext, xui_widget* ppWidget, const xui_input_desc_t* pDesc);`
- 实现: src/xui_input_widget.c:2819（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/audit_xui_text/main.c:16; examples/audit_xui_text/main.c:18; examples/audit_xui_text/main.c:20

## xuiInputSetChange
- 位置: xui.h:7587  已注释: 否
- 签名: `XUI_API int xuiInputSetChange(xui_widget pWidget, xui_input_change_proc onChange, void* pUser);`
- 实现: src/xui_input_widget.c:2834（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_input/main.c:246; test_xui/xui_accessibility_test.c:239; test_xui/xui_input_widget_test.c:301

## xuiInputSetText
- 位置: xui.h:7588  已注释: 否
- 签名: `XUI_API int xuiInputSetText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_input_widget.c:2843（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_input/main.c:576; test_xui/xui_accessibility_test.c:236; test_xui/xui_code_edit_test.c:1113

## xuiInputGetText
- 位置: xui.h:7589  已注释: 否
- 签名: `XUI_API const char* xuiInputGetText(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:2852（体 5 行）
- 用法: examples/xui_input/main.c:555; examples/xui_input/main.c:568; examples/xui_input/main.c:577

## xuiInputSetPlaceholder
- 位置: xui.h:7590  已注释: 否
- 签名: `XUI_API int xuiInputSetPlaceholder(xui_widget pWidget, const char* sText);`
- 实现: src/xui_input_widget.c:2858（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:237

## xuiInputGetPlaceholder
- 位置: xui.h:7591  已注释: 否
- 签名: `XUI_API const char* xuiInputGetPlaceholder(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:2869（体 5 行）
- 用法: test_xui/xui_input_widget_test.c:316

## xuiInputSetFont
- 位置: xui.h:7592  已注释: 否
- 签名: `XUI_API int xuiInputSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_input_widget.c:2875（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiInputGetFont
- 位置: xui.h:7593  已注释: 否
- 签名: `XUI_API xui_font xuiInputGetFont(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:2886（体 5 行）
- 用法: test_xui/xui_numeric_input_test.c:226

## xuiInputSetMaxLength
- 位置: xui.h:7594  已注释: 否
- 签名: `XUI_API int xuiInputSetMaxLength(xui_widget pWidget, int iMaxLength);`
- 实现: src/xui_input_widget.c:2892（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_input_widget_test.c:434; test_xui/xui_input_widget_test.c:443; test_xui/xui_input_widget_test.c:484

## xuiInputGetMaxLength
- 位置: xui.h:7595  已注释: 否
- 签名: `XUI_API int xuiInputGetMaxLength(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:2902（体 5 行）
- 用法: test_xui/xui_input_widget_test.c:317

## xuiInputSetTextAlign
- 位置: xui.h:7596  已注释: 否
- 签名: `XUI_API int xuiInputSetTextAlign(xui_widget pWidget, int iAlign);`
- 实现: src/xui_input_widget.c:2908（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_input/main.c:404; test_xui/xui_input_widget_test.c:613

## xuiInputGetTextAlign
- 位置: xui.h:7597  已注释: 否
- 签名: `XUI_API int xuiInputGetTextAlign(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:2916（体 5 行）
- 用法: examples/xui_input/main.c:544; test_xui/xui_input_widget_test.c:318; test_xui/xui_input_widget_test.c:614

## xuiInputSetPassword
- 位置: xui.h:7598  已注释: 否
- 签名: `XUI_API int xuiInputSetPassword(xui_widget pWidget, int bPassword);`
- 实现: src/xui_input_widget.c:2922（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_input/main.c:206; examples/xui_input/main.c:402; test_xui/xui_accessibility_test.c:238

## xuiInputIsPassword
- 位置: xui.h:7599  已注释: 否
- 签名: `XUI_API int xuiInputIsPassword(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:2934（体 5 行）
- 用法: examples/xui_input/main.c:205; examples/xui_input/main.c:542; test_xui/xui_input_widget_test.c:587

## xuiInputSetReadonly
- 位置: xui.h:7600  已注释: 否
- 签名: `XUI_API int xuiInputSetReadonly(xui_widget pWidget, int bReadonly);`
- 实现: src/xui_input_widget.c:2940（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_input/main.c:403; test_xui/xui_input_widget_test.c:683; test_xui/xui_input_widget_test.c:695

## xuiInputIsReadonly
- 位置: xui.h:7601  已注释: 否
- 签名: `XUI_API int xuiInputIsReadonly(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:2952（体 5 行）
- 用法: examples/xui_input/main.c:543; test_xui/xui_file_dialog_test.c:431; test_xui/xui_input_widget_test.c:684

## xuiInputSetError
- 位置: xui.h:7602  已注释: 否
- 签名: `XUI_API int xuiInputSetError(xui_widget pWidget, int bError);`
- 实现: src/xui_input_widget.c:2958（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_input/main.c:405; test_xui/xui_edit_contract_test.c:112; test_xui/xui_input_widget_test.c:320

## xuiInputGetError
- 位置: xui.h:7603  已注释: 否
- 签名: `XUI_API int xuiInputGetError(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:2971（体 5 行）
- 用法: examples/xui_input/main.c:545; test_xui/xui_file_dialog_test.c:285; test_xui/xui_file_dialog_test.c:306

## xuiInputSetSelection
- 位置: xui.h:7604  已注释: 否
- 签名: `XUI_API int xuiInputSetSelection(xui_widget pWidget, int iStart, int iEnd);`
- 实现: src/xui_input_widget.c:2977（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_input/main.c:568; examples/xui_input/main.c:577; test_xui/xui_input_widget_test.c:462

## xuiInputGetSelection
- 位置: xui.h:7605  已注释: 否
- 签名: `XUI_API int xuiInputGetSelection(xui_widget pWidget, int* pStart, int* pEnd);`
- 实现: src/xui_input_widget.c:2995（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_input/main.c:579; examples/xui_input/main.c:582; examples/xui_input/main.c:585

## xuiInputSelectAll
- 位置: xui.h:7606  已注释: 否
- 签名: `XUI_API int xuiInputSelectAll(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3003（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_input_widget_test.c:405; test_xui/xui_input_widget_test.c:421; test_xui/xui_input_widget_test.c:438

## xuiInputHasSelection
- 位置: xui.h:7607  已注释: 否
- 签名: `XUI_API int xuiInputHasSelection(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3010（体 4 行）
- 用法: test_xui/xui_file_dialog_test.c:365; test_xui/xui_input_widget_test.c:399; test_xui/xui_input_widget_test.c:406

## xuiInputCopy
- 位置: xui.h:7608  已注释: 否
- 签名: `XUI_API int xuiInputCopy(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3015（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_input_widget_test.c:407; test_xui/xui_input_widget_test.c:592

## xuiInputCut
- 位置: xui.h:7609  已注释: 否
- 签名: `XUI_API int xuiInputCut(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3022（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_input_widget_test.c:409; test_xui/xui_input_widget_test.c:644

## xuiInputPaste
- 位置: xui.h:7610  已注释: 否
- 签名: `XUI_API int xuiInputPaste(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3031（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_input_widget_test.c:423; test_xui/xui_input_widget_test.c:440

## xuiInputDeleteSelection
- 位置: xui.h:7611  已注释: 否
- 签名: `XUI_API int xuiInputDeleteSelection(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3038（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiInputUndo
- 位置: xui.h:7612  已注释: 否
- 签名: `XUI_API int xuiInputUndo(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3049（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_input_widget_test.c:412; test_xui/xui_input_widget_test.c:417; test_xui/xui_input_widget_test.c:426

## xuiInputRedo
- 位置: xui.h:7613  已注释: 否
- 签名: `XUI_API int xuiInputRedo(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3069（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_input_widget_test.c:415

## xuiInputCanUndo
- 位置: xui.h:7614  已注释: 否
- 签名: `XUI_API int xuiInputCanUndo(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3093（体 5 行）
- 用法: test_xui/xui_input_widget_test.c:411

## xuiInputCanRedo
- 位置: xui.h:7615  已注释: 否
- 签名: `XUI_API int xuiInputCanRedo(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3099（体 5 行）
- 用法: test_xui/xui_input_widget_test.c:414; test_xui/xui_input_widget_test.c:425; test_xui/xui_input_widget_test.c:647

## xuiInputSetColors
- 位置: xui.h:7616  已注释: 否
- 签名: `XUI_API int xuiInputSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iText, uint32_t iBorder, uint32_t iFocus);`
- 实现: src/xui_input_widget.c:3105（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_input/main.c:419

## xuiInputGetColors
- 位置: xui.h:7617  已注释: 否
- 签名: `XUI_API int xuiInputGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pText, uint32_t* pBorder, uint32_t* pFocus);`
- 实现: src/xui_input_widget.c:3118（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_basic_input_test.c:108; test_xui/xui_style_basic_input_test.c:160; test_xui/xui_style_pickers_combobox_test.c:94

## xuiInputSetErrorColors
- 位置: xui.h:7618  已注释: 否
- 签名: `XUI_API int xuiInputSetErrorColors(xui_widget pWidget, uint32_t iBackground, uint32_t iBorder);`
- 实现: src/xui_input_widget.c:3129（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiInputGetErrorColors
- 位置: xui.h:7619  已注释: 否
- 签名: `XUI_API int xuiInputGetErrorColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pBorder);`
- 实现: src/xui_input_widget.c:3138（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiInputSetExtendedColors
- 位置: xui.h:7620  已注释: 否
- 签名: `XUI_API int xuiInputSetExtendedColors(xui_widget pWidget, uint32_t iPlaceholder, uint32_t iDisabledText, uint32_t iHoverBackground, uint32_t iDisabledBackground, uint32_t iHoverBorder, uint32_t iSelection, uint32_t iCursor);`
- 实现: src/xui_input_widget.c:3147（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_chrome_test.c:451

## xuiInputGetExtendedColors
- 位置: xui.h:7621  已注释: 否
- 签名: `XUI_API int xuiInputGetExtendedColors(xui_widget pWidget, uint32_t* pPlaceholder, uint32_t* pDisabledText, uint32_t* pHoverBackground, uint32_t* pDisabledBackground, uint32_t* pHoverBorder, uint32_t* pSelection, uint32_t* pCursor);`
- 实现: src/xui_input_widget.c:3161（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_chrome_test.c:459; test_xui/xui_style_chrome_test.c:477; test_xui/xui_style_pickers_combobox_test.c:109

## xuiInputSetBorderWidth
- 位置: xui.h:7622  已注释: 否
- 签名: `XUI_API int xuiInputSetBorderWidth(xui_widget pWidget, float fBorderWidth);`
- 实现: src/xui_input_widget.c:3175（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiInputGetBorderWidth
- 位置: xui.h:7623  已注释: 否
- 签名: `XUI_API float xuiInputGetBorderWidth(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3185（体 5 行）

## xuiInputDecorationAdd
- 位置: xui.h:7624  已注释: 否
- 签名: `XUI_API int xuiInputDecorationAdd(xui_widget pWidget, int iSide, xui_input_decoration* ppDecoration, const xui_input_decoration_desc_t* pDesc);`
- 实现: src/xui_input_widget.c:3259（体 33 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/xui_input/main.c:272; examples/xui_input/main.c:288; examples/xui_input/main.c:304

## xuiInputDecorationSet
- 位置: xui.h:7625  已注释: 否
- 签名: `XUI_API int xuiInputDecorationSet(xui_widget pWidget, xui_input_decoration pDecoration, const xui_input_decoration_desc_t* pDesc);`
- 实现: src/xui_input_widget.c:3293（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_basic_input_test.c:87; test_xui/xui_style_basic_input_test.c:95

## xuiInputDecorationRemove
- 位置: xui.h:7626  已注释: 否
- 签名: `XUI_API int xuiInputDecorationRemove(xui_widget pWidget, xui_input_decoration pDecoration);`
- 实现: src/xui_input_widget.c:3310（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiInputDecorationClear
- 位置: xui.h:7627  已注释: 否
- 签名: `XUI_API int xuiInputDecorationClear(xui_widget pWidget, int iSide);`
- 实现: src/xui_input_widget.c:3334（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiInputDecorationGetCount
- 位置: xui.h:7628  已注释: 否
- 签名: `XUI_API int xuiInputDecorationGetCount(xui_widget pWidget, int iSide);`
- 实现: src/xui_input_widget.c:3355（体 21 行）

## xuiInputDecorationGetAt
- 位置: xui.h:7629  已注释: 否
- 签名: `XUI_API xui_input_decoration xuiInputDecorationGetAt(xui_widget pWidget, int iSide, int iIndex);`
- 实现: src/xui_input_widget.c:3377（体 24 行）
- 返回码: NULL

## xuiInputDecorationGetDesc
- 位置: xui.h:7630  已注释: 否
- 签名: `XUI_API int xuiInputDecorationGetDesc(xui_widget pWidget, xui_input_decoration pDecoration, xui_input_decoration_desc_t* pDesc);`
- 实现: src/xui_input_widget.c:3402（体 27 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiInputDecorationGetRect
- 位置: xui.h:7631  已注释: 否
- 签名: `XUI_API xui_rect_t xuiInputDecorationGetRect(xui_widget pWidget, xui_input_decoration pDecoration);`
- 实现: src/xui_input_widget.c:3430（体 12 行）
- 用法: examples/xui_input/main.c:547; examples/xui_input/main.c:548; examples/xui_input/main.c:549

## xuiInputSetMenuTitle
- 位置: xui.h:7632  已注释: 否
- 签名: `XUI_API int xuiInputSetMenuTitle(xui_widget pWidget, int iCommand, const char* sTitle);`
- 实现: src/xui_input_widget.c:3443（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/xui_input/main.c:417; examples/xui_input/main.c:418; test_xui/xui_input_widget_test.c:635

## xuiInputGetMenuTitle
- 位置: xui.h:7633  已注释: 否
- 签名: `XUI_API const char* xuiInputGetMenuTitle(xui_widget pWidget, int iCommand);`
- 实现: src/xui_input_widget.c:3464（体 8 行）
- 用法: examples/xui_input/main.c:546; test_xui/xui_combobox_test.c:285; test_xui/xui_combobox_test.c:289

## xuiInputOpenMenu
- 位置: xui.h:7634  已注释: 否
- 签名: `XUI_API int xuiInputOpenMenu(xui_widget pWidget, float fX, float fY);`
- 实现: src/xui_input_widget.c:3473（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_input_widget_test.c:600; test_xui/xui_input_widget_test.c:652; test_xui/xui_input_widget_test.c:685

## xuiInputGetMenuWidget
- 位置: xui.h:7635  已注释: 否
- 签名: `XUI_API xui_widget xuiInputGetMenuWidget(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3484（体 5 行）
- 用法: examples/xui_input/main.c:570; examples/xui_input/main.c:591; test_xui/xui_combobox_test.c:284

## xuiInputGetTextRect
- 位置: xui.h:7636  已注释: 否
- 签名: `XUI_API xui_rect_t xuiInputGetTextRect(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3490（体 5 行）
- 用法: test_xui/xui_combobox_test.c:305; test_xui/xui_combobox_test.c:319; test_xui/xui_input_widget_test.c:356

## xuiInputGetCursorRect
- 位置: xui.h:7637  已注释: 否
- 签名: `XUI_API xui_rect_t xuiInputGetCursorRect(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3496（体 5 行）

## xuiInputGetState
- 位置: xui.h:7638  已注释: 否
- 签名: `XUI_API uint32_t xuiInputGetState(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3502（体 14 行）
- 用法: test_xui/xui_input_widget_test.c:319; test_xui/xui_input_widget_test.c:321

## xuiInputGetChangeCount
- 位置: xui.h:7639  已注释: 否
- 签名: `XUI_API int xuiInputGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_input_widget.c:3517（体 5 行）

