# 草稿包：xui.h / Style（21 条 API）

> 生成 2026-09-10 02:52 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiStyleBeginUpdate
- 位置: xui.h:5882  已注释: 否
- 签名: `XUI_API int xuiStyleBeginUpdate(xui_context pContext);`
- 实现: src/xui_widget.c:2905（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_test.c:237

## xuiStyleEndUpdate
- 位置: xui.h:5883  已注释: 否
- 签名: `XUI_API int xuiStyleEndUpdate(xui_context pContext);`
- 实现: src/xui_widget.c:2914（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_test.c:246

## xuiStyleGetGeneration
- 位置: xui.h:5884  已注释: 否
- 签名: `XUI_API uint32_t xuiStyleGetGeneration(xui_context pContext);`
- 实现: src/xui_widget.c:2928（体 7 行）
- 用法: test_xui/xui_style_test.c:235; test_xui/xui_style_test.c:243; test_xui/xui_style_test.c:248

## xuiStyleRefresh
- 位置: xui.h:5885  已注释: 否
- 签名: `XUI_API int xuiStyleRefresh(xui_context pContext);`
- 实现: src/xui_widget.c:2936（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiStyleSetNamed
- 位置: xui.h:5886  已注释: 否
- 签名: `XUI_API int xuiStyleSetNamed(xui_context pContext, const xui_style_desc_t* pStyle);`
- 实现: src/xui_widget.c:2950（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_test.c:166; test_xui/xui_style_test.c:171

## xuiStyleRemoveNamed
- 位置: xui.h:5887  已注释: 否
- 签名: `XUI_API int xuiStyleRemoveNamed(xui_context pContext, const char* sName);`
- 实现: src/xui_widget.c:2964（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiStyleSetClass
- 位置: xui.h:5888  已注释: 否
- 签名: `XUI_API int xuiStyleSetClass(xui_context pContext, const char* sClass, const xui_style_desc_t* pStyle);`
- 实现: src/xui_widget.c:2981（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_label/main.c:254; examples/xui_label/main.c:259; examples/xui_label/main.c:265

## xuiStyleRemoveClass
- 位置: xui.h:5889  已注释: 否
- 签名: `XUI_API int xuiStyleRemoveClass(xui_context pContext, const char* sClass);`
- 实现: src/xui_widget.c:2996（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_basic_canvas_test.c:75; test_xui/xui_style_chrome_test.c:143

## xuiStyleSetType
- 位置: xui.h:5890  已注释: 否
- 签名: `XUI_API int xuiStyleSetType(xui_context pContext, xui_widget_type pType, const xui_style_desc_t* pStyle);`
- 实现: src/xui_widget.c:3013（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_prepare_paint_test.c:122; test_xui/xui_prepare_paint_test.c:129; test_xui/xui_style_basic_canvas_test.c:56

## xuiStyleRemoveType
- 位置: xui.h:5891  已注释: 否
- 签名: `XUI_API int xuiStyleRemoveType(xui_context pContext, xui_widget_type pType);`
- 实现: src/xui_widget.c:3028（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_basic_canvas_test.c:76; test_xui/xui_style_chrome_test.c:146; test_xui/xui_style_collections_table_test.c:67

## xuiStyleSetStateClass
- 位置: xui.h:5892  已注释: 否
- 签名: `XUI_API int xuiStyleSetStateClass(xui_context pContext, const xui_state_style_desc_t* pStyle);`
- 实现: src/xui_widget.c:3045（体 25 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_test.c:261

## xuiStyleRemoveStateClass
- 位置: xui.h:5893  已注释: 否
- 签名: `XUI_API int xuiStyleRemoveStateClass(xui_context pContext, const char* sClass, uint32_t iStateMask);`
- 实现: src/xui_widget.c:3071（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_test.c:267

## xuiStyleSetDefault
- 位置: xui.h:5894  已注释: 否
- 签名: `XUI_API int xuiStyleSetDefault(xui_context pContext, const xui_style_property_t* pProperties, int iPropertyCount);`
- 实现: src/xui_widget.c:3091（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_prepare_paint_test.c:116; test_xui/xui_style_basic_canvas_test.c:53; test_xui/xui_style_basic_canvas_test.c:80

## xuiStyleClearDefault
- 位置: xui.h:5895  已注释: 否
- 签名: `XUI_API int xuiStyleClearDefault(xui_context pContext);`
- 实现: src/xui_widget.c:3114（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_prepare_paint_test.c:130; test_xui/xui_style_basic_canvas_test.c:77; test_xui/xui_style_basic_input_test.c:169

## xuiStyleRegisterProperty
- 位置: xui.h:5896  已注释: 否
- 签名: `XUI_API int xuiStyleRegisterProperty(xui_context pContext, const xui_style_property_info_t* pInfo, uint32_t* pPropertyId);`
- 实现: src/xui_widget.c:3128（体 69 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_prepare_paint_test.c:106; test_xui/xui_style_test.c:141

## xuiStyleFindProperty
- 位置: xui.h:5897  已注释: 否
- 签名: `XUI_API uint32_t xuiStyleFindProperty(xui_context pContext, const char* sName);`
- 实现: src/xui_widget.c:3198（体 10 行）
- 用法: test_xui/xui_code_edit_test.c:556; test_xui/xui_code_edit_test.c:557; test_xui/xui_style_chrome_test.c:106

## xuiStyleGetPropertyInfo
- 位置: xui.h:5898  已注释: 否
- 签名: `XUI_API int xuiStyleGetPropertyInfo(xui_context pContext, uint32_t iPropertyId, xui_style_property_info_t* pInfo);`
- 实现: src/xui_widget.c:3209（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_FILE_NOT_FOUND, XUI_OK
- 用法: test_xui/xui_style_chrome_test.c:106; test_xui/xui_style_test.c:145

## xuiStyleSetToken
- 位置: xui.h:5899  已注释: 否
- 签名: `XUI_API int xuiStyleSetToken(xui_context pContext, const char* sName, const xui_style_value_t* pValue);`
- 实现: src/xui_widget.c:3233（体 46 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/xui_label/main.c:275; examples/xui_label/main.c:457; test_xui/xui_label_test.c:974

## xuiStyleRemoveToken
- 位置: xui.h:5900  已注释: 否
- 签名: `XUI_API int xuiStyleRemoveToken(xui_context pContext, const char* sName);`
- 实现: src/xui_widget.c:3280（体 29 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_chrome_test.c:149; test_xui/xui_style_chrome_test.c:280

## xuiStyleGetToken
- 位置: xui.h:5901  已注释: 否
- 签名: `XUI_API int xuiStyleGetToken(xui_context pContext, const char* sName, xui_style_value_t* pValue);`
- 实现: src/xui_widget.c:3310（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_FILE_NOT_FOUND, XUI_OK
- 用法: test_xui/xui_style_test.c:154; test_xui/xui_style_test.c:298; test_xui/xui_style_test.c:311

## xuiStyleGetTokenGeneration
- 位置: xui.h:5902  已注释: 否
- 签名: `XUI_API uint32_t xuiStyleGetTokenGeneration(xui_context pContext);`
- 实现: src/xui_widget.c:3326（体 4 行）
- 用法: test_xui/xui_style_test.c:148; test_xui/xui_style_test.c:152

