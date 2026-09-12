# 草稿包：xui.h / image（18 条 API）

> 生成 2026-09-10 03:00 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiImageGetType
- 位置: xui.h:6428  已注释: 否
- 签名: `XUI_API xui_widget_type xuiImageGetType(xui_context pContext);`
- 实现: src/xui_image.c:408（体 33 行）
- 返回码: NULL

## xuiImageCreate
- 位置: xui.h:6429  已注释: 否
- 签名: `XUI_API int xuiImageCreate(xui_context pContext, xui_widget* ppWidget, const xui_image_desc_t* pDesc);`
- 实现: src/xui_image.c:442（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch141_main1.c:39; examples/xui_image/main.c:377; test_xui/xui_image_icon_pixel_test.c:46

## xuiImageSetSurface
- 位置: xui.h:6430  已注释: 否
- 签名: `XUI_API int xuiImageSetSurface(xui_widget pWidget, xui_surface pSurface);`
- 实现: src/xui_image.c:457（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_image/main.c:429; test_xui/xui_image_test.c:782

## xuiImageGetSurface
- 位置: xui.h:6431  已注释: 否
- 签名: `XUI_API xui_surface xuiImageGetSurface(xui_widget pWidget);`
- 实现: src/xui_image.c:472（体 7 行）
- 用法: examples/xui_image/main.c:485; examples/xui_image/main.c:486; examples/xui_image/main.c:491

## xuiImageSetSource
- 位置: xui.h:6432  已注释: 否
- 签名: `XUI_API int xuiImageSetSource(xui_widget pWidget, xui_rect_t tSrc);`
- 实现: src/xui_image.c:480（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiImageSetSourceRect
- 位置: xui.h:6433  已注释: 否
- 签名: `XUI_API int xuiImageSetSourceRect(xui_widget pWidget, float fX1, float fY1, float fX2, float fY2);`
- 实现: src/xui_image.c:495（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_image_icon_pixel_test.c:52; test_xui/xui_image_icon_pixel_test.c:88; test_xui/xui_image_icon_pixel_test.c:90

## xuiImageClearSource
- 位置: xui.h:6434  已注释: 否
- 签名: `XUI_API int xuiImageClearSource(xui_widget pWidget);`
- 实现: src/xui_image.c:507（体 4 行）
- 用法: examples/xui_image/main.c:426; test_xui/xui_image_icon_pixel_test.c:81; test_xui/xui_image_test.c:732

## xuiImageGetSource
- 位置: xui.h:6435  已注释: 否
- 签名: `XUI_API xui_rect_t xuiImageGetSource(xui_widget pWidget);`
- 实现: src/xui_image.c:512（体 10 行）
- 用法: test_xui/xui_image_icon_pixel_test.c:53

## xuiImageSetColor
- 位置: xui.h:6436  已注释: 否
- 签名: `XUI_API int xuiImageSetColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_image.c:523（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiImageSetTint
- 位置: xui.h:6437  已注释: 否
- 签名: `XUI_API int xuiImageSetTint(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_image.c:538（体 4 行）
- 用法: examples/xui_image/main.c:427; test_xui/xui_image_test.c:774

## xuiImageGetColor
- 位置: xui.h:6438  已注释: 否
- 签名: `XUI_API uint32_t xuiImageGetColor(xui_widget pWidget);`
- 实现: src/xui_image.c:543（体 7 行）
- 用法: examples/xui_image/main.c:490; test_xui/xui_image_test.c:775; test_xui/xui_style_basic_content_test.c:57

## xuiImageSetMode
- 位置: xui.h:6439  已注释: 否
- 签名: `XUI_API int xuiImageSetMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_image.c:551（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_image/main.c:421; examples/xui_image/main.c:422; examples/xui_image/main.c:423

## xuiImageGetMode
- 位置: xui.h:6440  已注释: 否
- 签名: `XUI_API int xuiImageGetMode(xui_widget pWidget);`
- 实现: src/xui_image.c:569（体 7 行）
- 用法: examples/xui_image/main.c:480; examples/xui_image/main.c:487; examples/xui_image/main.c:488

## xuiImageSetAlign
- 位置: xui.h:6441  已注释: 否
- 签名: `XUI_API int xuiImageSetAlign(xui_widget pWidget, int iAlignX, int iAlignY);`
- 实现: src/xui_image.c:577（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_image_icon_pixel_test.c:62; test_xui/xui_image_test.c:758

## xuiImageGetAlign
- 位置: xui.h:6442  已注释: 否
- 签名: `XUI_API int xuiImageGetAlign(xui_widget pWidget, int* pAlignX, int* pAlignY);`
- 实现: src/xui_image.c:596（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_image/main.c:476; test_xui/xui_image_test.c:708; test_xui/xui_image_test.c:760

## xuiImageSetCustomRect
- 位置: xui.h:6443  已注释: 否
- 签名: `XUI_API int xuiImageSetCustomRect(xui_widget pWidget, float fX1, float fY1, float fX2, float fY2);`
- 实现: src/xui_image.c:613（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_image/main.c:425; test_xui/xui_image_icon_pixel_test.c:82; test_xui/xui_image_icon_pixel_test.c:87

## xuiImageGetCustomRect
- 位置: xui.h:6444  已注释: 否
- 签名: `XUI_API xui_rect_t xuiImageGetCustomRect(xui_widget pWidget);`
- 实现: src/xui_image.c:631（体 10 行）
- 用法: test_xui/xui_image_icon_pixel_test.c:83; test_xui/xui_image_icon_pixel_test.c:91; test_xui/xui_image_test.c:768

## xuiImageGetDrawRect
- 位置: xui.h:6445  已注释: 否
- 签名: `XUI_API xui_rect_t xuiImageGetDrawRect(xui_widget pWidget);`
- 实现: src/xui_image.c:642（体 14 行）
- 用法: examples/xui_image/main.c:477; examples/xui_image/main.c:478; test_xui/xui_image_icon_pixel_test.c:63

