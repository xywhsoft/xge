# 草稿包：xge.h / Scene（9 条 API）

> 生成 2026-09-10 02:40 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeSceneSet
- 位置: xge.h:1650  已注释: 否
- 签名: `XGE_API int xgeSceneSet(xge_scene pScene);`
- 实现: src/xge_core.c:879（体 21 行）
- 返回码: XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_UNSUPPORTED, XGE_OK
- 用法: examples/xui_multitouch/main.c:605; examples/xui_virtual_joystick/main.c:426; test/test_native_input_coordinates.c:331

## xgeScenePush
- 位置: xge.h:1651  已注释: 否
- 签名: `XGE_API int xgeScenePush(xge_scene pScene);`
- 实现: src/xge_core.c:901（体 35 行）
- 返回码: XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_UNSUPPORTED, XGE_ERROR_OUT_OF_MEMORY, XGE_OK

## xgeScenePop
- 位置: xge.h:1652  已注释: 否
- 签名: `XGE_API int xgeScenePop(void);`
- 实现: src/xge_core.c:937（体 31 行）
- 返回码: XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_UNSUPPORTED

## xgeSceneReplace
- 位置: xge.h:1653  已注释: 否
- 签名: `XGE_API int xgeSceneReplace(xge_scene pScene);`
- 实现: src/xge_core.c:969（体 36 行）
- 返回码: XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_UNSUPPORTED

## xgeSceneCurrent
- 位置: xge.h:1654  已注释: 否
- 签名: `XGE_API xge_scene xgeSceneCurrent(void);`
- 实现: src/xge_core.c:1006（体 7 行）
- 返回码: NULL

## xgeSceneCount
- 位置: xge.h:1655  已注释: 否
- 签名: `XGE_API int xgeSceneCount(void);`
- 实现: src/xge_core.c:1014（体 7 行）

## xgeSceneDispatchEvent
- 位置: xge.h:1656  已注释: 否
- 签名: `XGE_API int xgeSceneDispatchEvent(const xge_event_t* pEvent);`
- 实现: src/xge_core.c:1022（体 16 行）
- 返回码: XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSceneUpdateStrategySet
- 位置: xge.h:1657  已注释: 否
- 签名: `XGE_API int xgeSceneUpdateStrategySet(int iMode, float fFixedStep, int iMaxUpdates);`
- 实现: src/xge_core.c:1039（体 20 行）
- 返回码: XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSceneUpdateStrategyGet
- 位置: xge.h:1658  已注释: 否
- 签名: `XGE_API void xgeSceneUpdateStrategyGet(int* pMode, float* pFixedStep, int* pMaxUpdates);`
- 实现: src/xge_core.c:1060（体 12 行）

