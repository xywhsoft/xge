# 草稿包：xui.h / Icon（53 条 API）

> 生成 2026-09-10 02:50 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiIconCategoryDescDefault
- 位置: xui.h:5903  已注释: 否
- 签名: `XUI_API void xuiIconCategoryDescDefault(xui_icon_category_desc_t* pDesc);`
- 实现: src/xui_icon.c:1007（体 11 行）
- 用法: examples/xui_icon/main.c:120; examples/xui_icon/main.c:159; examples/xui_iconpicker/main.c:175

## xuiIconDescDefault
- 位置: xui.h:5904  已注释: 否
- 签名: `XUI_API void xuiIconDescDefault(xui_icon_desc_t* pDesc);`
- 实现: src/xui_icon.c:1019（体 6 行）
- 用法: examples/xui_iconpicker/main.c:187; test_xui/xui_icon_picker_test.c:129; test_xui/xui_icon_test.c:110

## xuiIconDrawDescDefault
- 位置: xui.h:5905  已注释: 否
- 签名: `XUI_API void xuiIconDrawDescDefault(xui_icon_draw_desc_t* pDesc);`
- 实现: src/xui_icon.c:1026（体 8 行）
- 用法: examples/xui_icon/main.c:217; test_xui/xui_icon_test.c:308; test_xui/xui_image_icon_pixel_test.c:114

## xuiIconCategoryCreate
- 位置: xui.h:5906  已注释: 否
- 签名: `XUI_API int xuiIconCategoryCreate(xui_context pContext, const char* sName, const xui_icon_category_desc_t* pDesc, xui_icon_category* ppCategory);`
- 实现: src/xui_icon.c:1050（体 69 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_ALREADY_INITIALIZED, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/xui_icon/main.c:123; examples/xui_icon/main.c:164; examples/xui_iconpicker/main.c:179

## xuiIconCategoryFind
- 位置: xui.h:5907  已注释: 否
- 签名: `XUI_API xui_icon_category xuiIconCategoryFind(xui_context pContext, const char* sName);`
- 实现: src/xui_icon.c:1120（体 7 行）
- 返回码: NULL
- 用法: test_xui/xui_icon_test.c:120; test_xui/xui_icon_test.c:185

## xuiIconCategoryRemove
- 位置: xui.h:5908  已注释: 否
- 签名: `XUI_API int xuiIconCategoryRemove(xui_context pContext, const char* sName);`
- 实现: src/xui_icon.c:1128（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_icon_test.c:183

## xuiIconCategoryClear
- 位置: xui.h:5909  已注释: 否
- 签名: `XUI_API void xuiIconCategoryClear(xui_context pContext);`
- 实现: src/xui_icon.c:1153（体 26 行）

## xuiIconCategoryGetCount
- 位置: xui.h:5910  已注释: 否
- 签名: `XUI_API int xuiIconCategoryGetCount(xui_context pContext);`
- 实现: src/xui_icon.c:1180（体 12 行）
- 用法: test_xui/xui_icon_test.c:119

## xuiIconCategoryGetAt
- 位置: xui.h:5911  已注释: 否
- 签名: `XUI_API xui_icon_category xuiIconCategoryGetAt(xui_context pContext, int iIndex);`
- 实现: src/xui_icon.c:1193（体 18 行）
- 返回码: NULL
- 用法: test_xui/xui_icon_test.c:121

## xuiIconCategoryAddRef
- 位置: xui.h:5912  已注释: 否
- 签名: `XUI_API int xuiIconCategoryAddRef(xui_icon_category pCategory);`
- 实现: src/xui_icon.c:1212（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_icon_test.c:181

## xuiIconCategoryRelease
- 位置: xui.h:5913  已注释: 否
- 签名: `XUI_API int xuiIconCategoryRelease(xui_icon_category pCategory);`
- 实现: src/xui_icon.c:1223（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_icon_test.c:187; test_xui/xui_icon_test.c:192; test_xui/xui_style_pickers_icon_picker_test.c:127

## xuiIconCategoryGetRefCount
- 位置: xui.h:5914  已注释: 否
- 签名: `XUI_API int xuiIconCategoryGetRefCount(xui_icon_category pCategory);`
- 实现: src/xui_icon.c:1233（体 4 行）
- 用法: test_xui/xui_icon_picker_test.c:144; test_xui/xui_icon_picker_test.c:162; test_xui/xui_icon_picker_test.c:238

## xuiIconCategoryGetName
- 位置: xui.h:5915  已注释: 否
- 签名: `XUI_API const char* xuiIconCategoryGetName(xui_icon_category pCategory);`
- 实现: src/xui_icon.c:1238（体 4 行）
- 用法: test_xui/xui_icon_test.c:122

## xuiIconCategoryGetGeneration
- 位置: xui.h:5916  已注释: 否
- 签名: `XUI_API uint32_t xuiIconCategoryGetGeneration(xui_icon_category pCategory);`
- 实现: src/xui_icon.c:1243（体 4 行）
- 用法: test_xui/xui_icon_test.c:166; test_xui/xui_icon_test.c:170; test_xui/xui_icon_test.c:172

## xuiIconCategorySetDesc
- 位置: xui.h:5917  已注释: 否
- 签名: `XUI_API int xuiIconCategorySetDesc(xui_icon_category pCategory, const xui_icon_category_desc_t* pDesc);`
- 实现: src/xui_icon.c:1248（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_image_icon_pixel_test.c:141

## xuiIconCategoryGetDesc
- 位置: xui.h:5918  已注释: 否
- 签名: `XUI_API int xuiIconCategoryGetDesc(xui_icon_category pCategory, xui_icon_category_desc_t* pDesc);`
- 实现: src/xui_icon.c:1272（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiIconCategoryBeginUpdate
- 位置: xui.h:5919  已注释: 否
- 签名: `XUI_API int xuiIconCategoryBeginUpdate(xui_icon_category pCategory);`
- 实现: src/xui_icon.c:1281（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_iconpicker/main.c:188; test_xui/xui_icon_test.c:167

## xuiIconCategoryEndUpdate
- 位置: xui.h:5920  已注释: 否
- 签名: `XUI_API int xuiIconCategoryEndUpdate(xui_icon_category pCategory);`
- 实现: src/xui_icon.c:1288（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_iconpicker/main.c:208; test_xui/xui_icon_test.c:171

## xuiIconCategoryGetIconCount
- 位置: xui.h:5921  已注释: 否
- 签名: `XUI_API int xuiIconCategoryGetIconCount(xui_icon_category pCategory);`
- 实现: src/xui_icon.c:1301（体 4 行）
- 用法: test_xui/xui_icon_test.c:145; test_xui/xui_icon_test.c:175

## xuiIconCategoryGetIconSlotCount
- 位置: xui.h:5922  已注释: 否
- 签名: `XUI_API uint32_t xuiIconCategoryGetIconSlotCount(xui_icon_category pCategory);`
- 实现: src/xui_icon.c:1306（体 4 行）
- 用法: test_xui/xui_icon_test.c:155; test_xui/xui_icon_test.c:176

## xuiIconCategoryGetIconAt
- 位置: xui.h:5923  已注释: 否
- 签名: `XUI_API xui_icon xuiIconCategoryGetIconAt(xui_icon_category pCategory, int iIndex);`
- 实现: src/xui_icon.c:1311（体 18 行）
- 返回码: NULL
- 用法: test_xui/xui_icon_test.c:156; test_xui/xui_icon_test.c:157

## xuiIconCategoryPreload
- 位置: xui.h:5924  已注释: 否
- 签名: `XUI_API int xuiIconCategoryPreload(xui_icon_category pCategory, float fWidth, float fHeight);`
- 实现: src/xui_icon.c:1330（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiIconAddSvgPath
- 位置: xui.h:5926  已注释: 否
- 签名: `XUI_API int xuiIconAddSvgPath(xui_icon_category pCategory, const char* sName, const char* sPath, xui_rect_t tViewBox, const xui_path_style_t* pStyle, const xui_icon_desc_t* pDesc, xui_icon* ppIcon);`
- 实现: src/xui_icon.c:1348（体 48 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/xui_icon/main.c:133; examples/xui_icon/main.c:142; examples/xui_iconpicker/main.c:197

## xuiIconAddSvgFile
- 位置: xui.h:5927  已注释: 否
- 签名: `XUI_API int xuiIconAddSvgFile(xui_icon_category pCategory, const char* sName, const char* sPath, const xui_icon_desc_t* pDesc, xui_icon* ppIcon);`
- 实现: src/xui_icon.c:1397（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_icon_test.c:278

## xuiIconAddSvgMemory
- 位置: xui.h:5928  已注释: 否
- 签名: `XUI_API int xuiIconAddSvgMemory(xui_icon_category pCategory, const char* sName, const void* pData, int iSize, const xui_icon_desc_t* pDesc, xui_icon* ppIcon);`
- 实现: src/xui_icon.c:1432（体 4 行）
- 用法: examples/xui_icon/main.c:166; examples/xui_icon/main.c:174; test_xui/xui_icon_test.c:276

## xuiIconAddRasterFile
- 位置: xui.h:5929  已注释: 否
- 签名: `XUI_API int xuiIconAddRasterFile(xui_icon_category pCategory, const char* sName, const char* sPath, const xui_icon_desc_t* pDesc, xui_icon* ppIcon);`
- 实现: src/xui_icon.c:1437（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_icon_test.c:279

## xuiIconAddRasterMemory
- 位置: xui.h:5930  已注释: 否
- 签名: `XUI_API int xuiIconAddRasterMemory(xui_icon_category pCategory, const char* sName, const void* pData, int iSize, const xui_icon_desc_t* pDesc, xui_icon* ppIcon);`
- 实现: src/xui_icon.c:1454（体 4 行）
- 用法: test_xui/xui_icon_test.c:126; test_xui/xui_icon_test.c:151; test_xui/xui_icon_test.c:168

## xuiIconAddSurface
- 位置: xui.h:5931  已注释: 否
- 签名: `XUI_API int xuiIconAddSurface(xui_icon_category pCategory, const char* sName, xui_surface pSurface, xui_rect_t tSource, uint32_t iSurfaceFlags, const xui_icon_desc_t* pDesc, xui_icon* ppIcon);`
- 实现: src/xui_icon.c:1459（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_icon_test.c:373; test_xui/xui_image_icon_pixel_test.c:110; test_xui/xui_style_basic_content_test.c:59

## xuiIconAddResource
- 位置: xui.h:5932  已注释: 否
- 签名: `XUI_API int xuiIconAddResource(xui_icon_category pCategory, const char* sName, xui_resource pResource, xui_rect_t tSource, const xui_icon_desc_t* pDesc, xui_icon* ppIcon);`
- 实现: src/xui_icon.c:1474（体 25 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_icon_test.c:290

## xuiIconAddAlias
- 位置: xui.h:5933  已注释: 否
- 签名: `XUI_API int xuiIconAddAlias(xui_icon_category pCategory, const char* sName, xui_icon pTarget, const xui_icon_desc_t* pDesc, xui_icon* ppIcon);`
- 实现: src/xui_icon.c:1500（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_icon/main.c:151; examples/xui_icon/main.c:182; test_xui/xui_icon_test.c:159

## xuiIconAddCustom
- 位置: xui.h:5934  已注释: 否
- 签名: `XUI_API int xuiIconAddCustom(xui_icon_category pCategory, const char* sName, const xui_icon_custom_desc_t* pCustom, const xui_icon_desc_t* pDesc, xui_icon* ppIcon);`
- 实现: src/xui_icon.c:1521（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_icon/main.c:189; test_xui/xui_icon_test.c:305

## xuiIconFind
- 位置: xui.h:5935  已注释: 否
- 签名: `XUI_API xui_icon xuiIconFind(xui_icon_category pCategory, const char* sName);`
- 实现: src/xui_icon.c:1538（体 10 行）
- 返回码: NULL
- 用法: examples/xui_icon/main.c:154; test_xui/xui_icon_test.c:146

## xuiIconFindById
- 位置: xui.h:5936  已注释: 否
- 签名: `XUI_API xui_icon xuiIconFindById(xui_icon_category pCategory, xui_icon_id iId);`
- 实现: src/xui_icon.c:1549（体 11 行）
- 返回码: NULL
- 用法: test_xui/xui_icon_picker_test.c:172; test_xui/xui_icon_test.c:147; test_xui/xui_icon_test.c:150

## xuiIconRemove
- 位置: xui.h:5937  已注释: 否
- 签名: `XUI_API int xuiIconRemove(xui_icon_category pCategory, const char* sName);`
- 实现: src/xui_icon.c:1579（体 7 行）
- 用法: test_xui/xui_icon_test.c:162; test_xui/xui_icon_test.c:383; test_xui/xui_icon_test.c:387

## xuiIconRemoveById
- 位置: xui.h:5938  已注释: 否
- 签名: `XUI_API int xuiIconRemoveById(xui_icon_category pCategory, xui_icon_id iId);`
- 实现: src/xui_icon.c:1561（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_icon_picker_test.c:229; test_xui/xui_icon_test.c:149

## xuiIconClear
- 位置: xui.h:5939  已注释: 否
- 签名: `XUI_API void xuiIconClear(xui_icon_category pCategory);`
- 实现: src/xui_icon.c:1587（体 24 行）
- 用法: test_xui/xui_icon_test.c:174

## xuiIconAddRef
- 位置: xui.h:5940  已注释: 否
- 签名: `XUI_API int xuiIconAddRef(xui_icon pIcon);`
- 实现: src/xui_icon.c:1612（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK

## xuiIconRelease
- 位置: xui.h:5941  已注释: 否
- 签名: `XUI_API int xuiIconRelease(xui_icon pIcon);`
- 实现: src/xui_icon.c:1623（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiIconGetRefCount
- 位置: xui.h:5942  已注释: 否
- 签名: `XUI_API int xuiIconGetRefCount(xui_icon pIcon);`
- 实现: src/xui_icon.c:1633（体 4 行）
- 用法: test_xui/xui_icon_test.c:161; test_xui/xui_icon_test.c:164

## xuiIconGetId
- 位置: xui.h:5943  已注释: 否
- 签名: `XUI_API xui_icon_id xuiIconGetId(xui_icon pIcon);`
- 实现: src/xui_icon.c:1638（体 4 行）
- 用法: examples/xui_iconpicker/main.c:206; test_xui/xui_icon_picker_test.c:41; test_xui/xui_icon_picker_test.c:142

## xuiIconGetName
- 位置: xui.h:5944  已注释: 否
- 签名: `XUI_API const char* xuiIconGetName(xui_icon pIcon);`
- 实现: src/xui_icon.c:1643（体 4 行）
- 用法: examples/xui_iconpicker/main.c:233

## xuiIconGetDisplayName
- 位置: xui.h:5945  已注释: 否
- 签名: `XUI_API const char* xuiIconGetDisplayName(xui_icon pIcon);`
- 实现: src/xui_icon.c:1648（体 5 行）
- 返回码: NULL
- 用法: test_xui/xui_icon_test.c:130

## xuiIconGetTags
- 位置: xui.h:5946  已注释: 否
- 签名: `XUI_API const char* xuiIconGetTags(xui_icon pIcon);`
- 实现: src/xui_icon.c:1654（体 4 行）
- 用法: test_xui/xui_icon_test.c:131

## xuiIconGetSourceType
- 位置: xui.h:5947  已注释: 否
- 签名: `XUI_API int xuiIconGetSourceType(xui_icon pIcon);`
- 实现: src/xui_icon.c:1659（体 4 行）

## xuiIconGetGeneration
- 位置: xui.h:5948  已注释: 否
- 签名: `XUI_API uint32_t xuiIconGetGeneration(xui_icon pIcon);`
- 实现: src/xui_icon.c:1664（体 4 行）

## xuiIconGetUser
- 位置: xui.h:5949  已注释: 否
- 签名: `XUI_API void* xuiIconGetUser(xui_icon pIcon);`
- 实现: src/xui_icon.c:1669（体 4 行）

## xuiIconSetMetadata
- 位置: xui.h:5950  已注释: 否
- 签名: `XUI_API int xuiIconSetMetadata(xui_icon pIcon, const xui_icon_desc_t* pDesc);`
- 实现: src/xui_icon.c:1674（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK

## xuiIconTouch
- 位置: xui.h:5951  已注释: 否
- 签名: `XUI_API int xuiIconTouch(xui_icon pIcon);`
- 实现: src/xui_icon.c:1701（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_icon_test.c:346

## xuiIconGetIntrinsicSize
- 位置: xui.h:5952  已注释: 否
- 签名: `XUI_API int xuiIconGetIntrinsicSize(xui_icon pIcon, xui_vec2_t* pSize);`
- 实现: src/xui_icon.c:1708（体 4 行）
- 用法: test_xui/xui_icon_test.c:357; test_xui/xui_icon_test.c:362; test_xui/xui_image_icon_pixel_test.c:148

## xuiIconPrepare
- 位置: xui.h:5953  已注释: 否
- 签名: `XUI_API int xuiIconPrepare(xui_icon pIcon, float fWidth, float fHeight);`
- 实现: src/xui_icon.c:1713（体 4 行）
- 用法: test_xui/xui_icon_test.c:337; test_xui/xui_icon_test.c:349; test_xui/xui_icon_test.c:360

## xuiIconDraw
- 位置: xui.h:5954  已注释: 否
- 签名: `XUI_API int xuiIconDraw(xui_painter pPainter, xui_icon pIcon, xui_rect_t tRect, const xui_icon_draw_desc_t* pDesc);`
- 实现: src/xui_icon.c:1718（体 5 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_icon_test.c:310; test_xui/xui_icon_test.c:315; test_xui/xui_icon_test.c:317

## xuiIconDrawById
- 位置: xui.h:5955  已注释: 否
- 签名: `XUI_API int xuiIconDrawById(xui_painter pPainter, xui_icon_category pCategory, xui_icon_id iId, xui_rect_t tRect, const xui_icon_draw_desc_t* pDesc);`
- 实现: src/xui_icon.c:1724（体 7 行）

## xuiIconDrawByName
- 位置: xui.h:5956  已注释: 否
- 签名: `XUI_API int xuiIconDrawByName(xui_painter pPainter, xui_icon_category pCategory, const char* sName, xui_rect_t tRect, const xui_icon_draw_desc_t* pDesc);`
- 实现: src/xui_icon.c:1732（体 7 行）
- 用法: examples/xui_icon/main.c:224; examples/xui_icon/main.c:226; examples/xui_icon/main.c:228

