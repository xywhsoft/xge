# 草稿包：xge.h / Blend（2 条 API）

> 生成 2026-09-10 02:33 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeBlendSet
- 位置: xge.h:1688  已注释: 否
- 签名: `XGE_API void xgeBlendSet(int iBlend);`
- 实现: src/xge_core.c:1404（体 5 行）
- 用法: examples/tutorial_capture/ch12_main1.c:9; examples/tutorial_capture/ch12_main1.c:13; examples/tutorial_capture/ch12_main1.c:18

## xgeBlendGet
- 位置: xge.h:1689  已注释: 否
- 签名: `XGE_API int xgeBlendGet(void);`
- 实现: src/xge_core.c:1410（体 4 行）
- 用法: test/test_particle_render.c:73

