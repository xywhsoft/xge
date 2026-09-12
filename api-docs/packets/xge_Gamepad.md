# 草稿包：xge.h / Gamepad（8 条 API）

> 生成 2026-09-10 02:39 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeGamepadConnected
- 位置: xge.h:2631  已注释: 否
- 签名: `XGE_API int xgeGamepadConnected(int iGamepad);`
- 实现: src/xge_input.c:607（体 7 行）
- 用法: examples/tutorial_capture/ch119_main1.c:43

## xgeGamepadGetState
- 位置: xge.h:2632  已注释: 否
- 签名: `XGE_API int xgeGamepadGetState(int iGamepad, xge_gamepad_state_t* pState);`
- 实现: src/xge_input.c:615（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch119_main1.c:42

## xgeGamepadButtonDown
- 位置: xge.h:2633  已注释: 否
- 签名: `XGE_API int xgeGamepadButtonDown(int iGamepad, uint32_t iButton);`
- 实现: src/xge_input.c:624（体 7 行）
- 用法: examples/tutorial_capture/ch119_main1.c:44; examples/tutorial_capture/ch119_main1.c:83

## xgeGamepadButtonPressed
- 位置: xge.h:2634  已注释: 否
- 签名: `XGE_API int xgeGamepadButtonPressed(int iGamepad, uint32_t iButton);`
- 实现: src/xge_input.c:632（体 7 行）
- 用法: examples/tutorial_capture/ch119_main1.c:45

## xgeGamepadButtonReleased
- 位置: xge.h:2635  已注释: 否
- 签名: `XGE_API int xgeGamepadButtonReleased(int iGamepad, uint32_t iButton);`
- 实现: src/xge_input.c:640（体 7 行）

## xgeGamepadAxis
- 位置: xge.h:2636  已注释: 否
- 签名: `XGE_API float xgeGamepadAxis(int iGamepad, int iAxis);`
- 实现: src/xge_input.c:648（体 7 行）
- 用法: examples/tutorial_capture/ch119_main1.c:46; examples/tutorial_capture/ch119_main1.c:47

## xgeGamepadSetConnected
- 位置: xge.h:2637  已注释: 否
- 签名: `XGE_API int xgeGamepadSetConnected(int iGamepad, int bConnected);`
- 实现: src/xge_input.c:656（体 23 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeGamepadSetState
- 位置: xge.h:2638  已注释: 否
- 签名: `XGE_API int xgeGamepadSetState(int iGamepad, const xge_gamepad_state_t* pState);`
- 实现: src/xge_input.c:680（体 26 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch119_main1.c:41

