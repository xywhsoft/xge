# 草稿包：xge.h / Particle（39 条 API）

> 生成 2026-09-10 02:39 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeParticleEmitterInit
- 位置: xge.h:2723  已注释: 否
- 签名: `XGE_API void xgeParticleEmitterInit(xge_particle_emitter_t* pEmitter);`
- 实现: src/xge_particle.c:84（体 24 行）
- 用法: examples/xge_particles/confetti.c:16; examples/xge_particles/dust.c:11; examples/xge_particles/dust.c:28

## xgeParticleWorldDescInit
- 位置: xge.h:2724  已注释: 否
- 签名: `XGE_API void xgeParticleWorldDescInit(xge_particle_world_desc_t* pDesc);`
- 实现: src/xge_particle.c:108（体 12 行）
- 用法: examples/xge_particles/common.c:416; test/particle_bench.c:22; test/test_particle.c:44

## xgeParticlePlayInit
- 位置: xge.h:2725  已注释: 否
- 签名: `XGE_API void xgeParticlePlayInit(xge_particle_play_t* pPlay);`
- 实现: src/xge_particle.c:120（体 9 行）
- 用法: examples/xge_particles/common.c:55; test/test_particle.c:424; test/test_particle.c:483

## xgeParticleCurveEval
- 位置: xge.h:2726  已注释: 否
- 签名: `XGE_API float xgeParticleCurveEval(const xge_particle_curve_t* pCurve, float fTime, float fDefault);`
- 实现: src/xge_particle.c:144（体 24 行）
- 用法: test/test_particle.c:74; test/test_particle.c:78; test/test_particle.c:79

## xgeParticleGradientEval
- 位置: xge.h:2727  已注释: 否
- 签名: `XGE_API uint32_t xgeParticleGradientEval(const xge_particle_gradient_t* pGradient, float fTime, uint32_t iDefault);`
- 实现: src/xge_particle.c:179（体 15 行）
- 用法: test/test_particle.c:90

## xgeParticleDefinitionCreate
- 位置: xge.h:2728  已注释: 否
- 签名: `XGE_API int xgeParticleDefinitionCreate(xge_particle_definition* ppDefinition, const xge_particle_emitter_t* pEmitters, int iCount);`
- 实现: src/xge_particle.c:275（体 35 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/xge_particles/common.c:386; test/particle_bench.c:28; test/test_particle.c:54

## xgeParticleDefinitionAddRef
- 位置: xge.h:2730  已注释: 否
- 签名: `XGE_API void xgeParticleDefinitionAddRef(xge_particle_definition pDefinition);`
- 实现: src/xge_particle.c:310（体 5 行）

## xgeParticleDefinitionFree
- 位置: xge.h:2731  已注释: 否
- 签名: `XGE_API void xgeParticleDefinitionFree(xge_particle_definition pDefinition);`
- 实现: src/xge_particle.c:315（体 8 行）
- 用法: examples/xge_particles/common.c:436; test/particle_bench.c:46; test/test_particle.c:113

## xgeParticleDefinitionCount
- 位置: xge.h:2732  已注释: 否
- 签名: `XGE_API int xgeParticleDefinitionCount(xge_particle_definition pDefinition);`
- 实现: src/xge_particle.c:323（体 4 行）
- 用法: examples/xge_particles/common.c:384; test/test_particle.c:112

## xgeParticleDefinitionGet
- 位置: xge.h:2733  已注释: 否
- 签名: `XGE_API int xgeParticleDefinitionGet(xge_particle_definition pDefinition, int iEmitter, xge_particle_emitter_t* pOut);`
- 实现: src/xge_particle.c:327（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/xge_particles/common.c:85; test/test_particle.c:565

## xgeParticleDefinitionParse
- 位置: xge.h:2736  已注释: 是
- 签名: `XGE_API int xgeParticleDefinitionParse(xge_particle_definition* ppDefinition, const char* sText, size_t iSize, int bXson, char* sError, size_t iErrorSize);`
- 既有注释: /* JSON and XSON share schema version 1. Output is UTF-8, freed with xrtFree. * Invalid input leaves *ppDefinition NULL. Error text is optional and bounded. */
- 实现: src/xge_particle_io.c:372（体 57 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_RESOURCE_FAILED
- 用法: test/test_particle.c:564; test/test_particle.c:571; test/test_particle.c:580

## xgeParticleDefinitionLoad
- 位置: xge.h:2738  已注释: 否
- 签名: `XGE_API int xgeParticleDefinitionLoad(xge_particle_definition* ppDefinition, const char* sURI, int bXson, char* sError, size_t iErrorSize);`
- 实现: src/xge_particle_resource.c:1（体 23 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/xge_particles/common.c:379

## xgeParticleDefinitionStringify
- 位置: xge.h:2740  已注释: 否
- 签名: `XGE_API int xgeParticleDefinitionStringify(xge_particle_definition pDefinition, int bXson, char** ppText, size_t* pSize);`
- 实现: src/xge_particle_io.c:597（体 31 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/xge_particles/common.c:396; test/test_particle.c:570; test/test_particle.c:572

## xgeParticleWorldCreate
- 位置: xge.h:2741  已注释: 否
- 签名: `XGE_API int xgeParticleWorldCreate(xge_particle_world* ppWorld, const xge_particle_world_desc_t* pDesc);`
- 实现: src/xge_particle.c:335（体 32 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/xge_particles/common.c:421; test/particle_bench.c:29; test/test_particle.c:48

## xgeParticleWorldFree
- 位置: xge.h:2742  已注释: 否
- 签名: `XGE_API void xgeParticleWorldFree(xge_particle_world pWorld);`
- 实现: src/xge_particle.c:367（体 18 行）
- 用法: examples/xge_particles/common.c:435; test/particle_bench.c:45; test/test_particle.c:160

## xgeParticleWorldReserve
- 位置: xge.h:2744  已注释: 是
- 签名: `XGE_API int xgeParticleWorldReserve(xge_particle_world pWorld, xge_particle_definition pDefinition, uint32_t iCount);`
- 既有注释: /* Reserve caches inactive instances and their buffers; partial success is retained on OOM. */
- 实现: src/xge_particle.c:448（体 22 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/xge_particles/common.c:424; test/particle_bench.c:30; test/test_particle.c:129

## xgeParticlePlay
- 位置: xge.h:2745  已注释: 否
- 签名: `XGE_API int xgeParticlePlay(xge_particle_world pWorld, xge_particle_definition pDefinition, const xge_particle_play_t* pPlay, xge_particle_effect* pEffect);`
- 实现: src/xge_particle.c:482（体 30 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/xge_particles/common.c:60; test/particle_bench.c:31; test/test_particle.c:66

## xgeParticleRelease
- 位置: xge.h:2747  已注释: 否
- 签名: `XGE_API int xgeParticleRelease(xge_particle_world pWorld, xge_particle_effect iEffect);`
- 实现: src/xge_particle.c:512（体 13 行）
- 返回码: XGE_ERROR_NOT_FOUND, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/xge_particles/common.c:177; test/test_particle.c:154; test/test_particle.c:278

## xgeParticleRestart
- 位置: xge.h:2748  已注释: 否
- 签名: `XGE_API int xgeParticleRestart(xge_particle_world pWorld, xge_particle_effect iEffect, uint64_t iSeed);`
- 实现: src/xge_particle.c:525（体 10 行）
- 返回码: XGE_ERROR_NOT_FOUND, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: test/test_particle.c:147; test/test_particle.c:157; test/test_particle.c:270

## xgeParticlePause
- 位置: xge.h:2749  已注释: 否
- 签名: `XGE_API int xgeParticlePause(xge_particle_world pWorld, xge_particle_effect iEffect, int bPaused);`
- 实现: src/xge_particle.c:535（体 19 行）
- 返回码: XGE_ERROR_NOT_FOUND, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/xge_particles/common.c:172; test/test_particle.c:134; test/test_particle.c:138

## xgeParticleStop
- 位置: xge.h:2750  已注释: 否
- 签名: `XGE_API int xgeParticleStop(xge_particle_world pWorld, xge_particle_effect iEffect, int bClear);`
- 实现: src/xge_particle.c:554（体 18 行）
- 返回码: XGE_ERROR_NOT_FOUND, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/xge_particles/common.c:192; test/test_particle.c:139; test/test_particle.c:150

## xgeParticleSetTransform
- 位置: xge.h:2751  已注释: 否
- 签名: `XGE_API int xgeParticleSetTransform(xge_particle_world pWorld, xge_particle_effect iEffect, const xge_particle_transform_t* pTransform);`
- 实现: src/xge_particle.c:572（体 14 行）
- 返回码: XGE_ERROR_NOT_FOUND, XGE_ERROR_INVALID_STATE, XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/xge_particles/common.c:240; test/test_particle.c:302; test/test_particle.c:319

## xgeParticleSetParameters
- 位置: xge.h:2752  已注释: 否
- 签名: `XGE_API int xgeParticleSetParameters(xge_particle_world pWorld, xge_particle_effect iEffect, const xge_particle_parameters_t* pParameters);`
- 实现: src/xge_particle.c:586（体 12 行）
- 返回码: XGE_ERROR_NOT_FOUND, XGE_ERROR_INVALID_STATE, XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/xge_particles/common.c:210; test/test_particle.c:327

## xgeParticleSetVisible
- 位置: xge.h:2753  已注释: 否
- 签名: `XGE_API int xgeParticleSetVisible(xge_particle_world pWorld, xge_particle_effect iEffect, int bVisible);`
- 实现: src/xge_particle.c:598（体 10 行）
- 返回码: XGE_ERROR_NOT_FOUND, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/xge_particles/common.c:66; examples/xge_particles/common.c:202; test/test_particle.c:430

## xgeParticleState
- 位置: xge.h:2754  已注释: 否
- 签名: `XGE_API int xgeParticleState(xge_particle_world pWorld, xge_particle_effect iEffect);`
- 实现: src/xge_particle.c:608（体 5 行）
- 用法: examples/xge_particles/common.c:50; test/test_particle.c:141; test/test_particle.c:490

## xgeParticleBounds
- 位置: xge.h:2755  已注释: 否
- 签名: `XGE_API int xgeParticleBounds(xge_particle_world pWorld, xge_particle_effect iEffect, xge_rect_t* pBounds);`
- 实现: src/xge_particle.c:1249（体 27 行）
- 返回码: XGE_ERROR_NOT_FOUND, XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/xge_particles/common.c:271; test/test_particle.c:306; test/test_particle.c:526

## xgeParticleEmit
- 位置: xge.h:2757  已注释: 是
- 签名: `XGE_API int xgeParticleEmit(xge_particle_world pWorld, xge_particle_effect iEffect, int iEmitter, uint32_t iCount, uint32_t* pSpawned);`
- 既有注释: /* Manual emission returns actual count through pSpawned; capacity loss is reported in stats. */
- 实现: src/xge_particle.c:796（体 23 行）
- 返回码: XGE_ERROR_NOT_FOUND, XGE_ERROR_INVALID_STATE, XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/particle_bench.c:32; test/test_particle.c:148; test/test_particle.c:221

## xgeParticleUpdate
- 位置: xge.h:2758  已注释: 否
- 签名: `XGE_API int xgeParticleUpdate(xge_particle_world pWorld, double fDeltaSeconds);`
- 实现: src/xge_particle.c:1154（体 46 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/xge_particles/common.c:247; test/particle_bench.c:37; test/test_particle.c:61

## xgeParticlePrewarm
- 位置: xge.h:2760  已注释: 是
- 签名: `XGE_API int xgeParticlePrewarm(xge_particle_world pWorld, xge_particle_effect iEffect, float fSeconds);`
- 既有注释: /* Prewarm simulates this effect only; public events suppressed, internal subemitters still run. */
- 实现: src/xge_particle.c:1200（体 25 行）
- 返回码: XGE_ERROR_NOT_FOUND, XGE_ERROR_INVALID_STATE, XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/xge_particles/common.c:186; examples/xge_particles/common.c:428; test/test_particle.c:427

## xgeParticleVisit
- 位置: xge.h:2761  已注释: 否
- 签名: `XGE_API int xgeParticleVisit(xge_particle_world pWorld, xge_particle_visit_proc pVisit, void* pUser);`
- 实现: src/xge_particle.c:1225（体 24 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: test/test_particle.c:37; test/test_particle.c:548

## xgeParticleEventPoll
- 位置: xge.h:2762  已注释: 否
- 签名: `XGE_API int xgeParticleEventPoll(xge_particle_world pWorld, xge_particle_event_t* pEvent);`
- 实现: src/xge_particle.c:1276（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE
- 用法: examples/xge_particles/common.c:250; test/test_particle.c:143; test/test_particle.c:359

## xgeParticleStats
- 位置: xge.h:2763  已注释: 否
- 签名: `XGE_API xge_particle_stats_t xgeParticleStats(xge_particle_world pWorld);`
- 实现: src/xge_particle.c:1289（体 11 行）
- 用法: examples/xge_particles/common.c:279; test/particle_bench.c:40; test/test_particle.c:130

## xgeParticleCollidePlane
- 位置: xge.h:2765  已注释: 是
- 签名: `XGE_API int xgeParticleCollidePlane(xge_vec2_t tFrom, xge_vec2_t tTo, float fRadius, xge_vec2_t tNormal, float fOffset, xge_particle_hit_t* pHit);`
- 既有注释: /* Optional collision query helpers; plane's allowed side is dot(P, normal) >= offset. */
- 实现: src/xge_particle.c:841（体 27 行）
- 用法: examples/xge_particles/common.c:42; test/test_particle.c:371; test/test_particle.c:382

## xgeParticleCollideRect
- 位置: xge.h:2767  已注释: 否
- 签名: `XGE_API int xgeParticleCollideRect(xge_vec2_t tFrom, xge_vec2_t tTo, float fRadius, xge_rect_t tRect, xge_particle_hit_t* pHit);`
- 实现: src/xge_particle.c:868（体 63 行）
- 用法: examples/xge_particles/common.c:40; test/test_particle.c:385; test/test_particle.c:389

## xgeParticleRendererCreate
- 位置: xge.h:2772  已注释: 是
- 签名: `XGE_API int xgeParticleRendererCreate(xge_particle_renderer* ppRenderer, uint32_t iCapacity, uint32_t iBatchCapacity);`
- 既有注释: /* Renderer holds references to definitions, textures and shader resources. * Their caller-owned C structs must remain alive until unbound/renderer destruction. */
- 实现: src/xge_particle_render.c:57（体 24 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/xge_particles/common.c:422; test/test_particle_render.c:52; test/test_particle_render.c:53

## xgeParticleRendererFree
- 位置: xge.h:2773  已注释: 否
- 签名: `XGE_API void xgeParticleRendererFree(xge_particle_renderer pRenderer);`
- 实现: src/xge_particle_render.c:81（体 14 行）
- 用法: examples/xge_particles/common.c:434; test/test_particle_render.c:181; test/test_particle_render.c:182

## xgeParticleRendererBind
- 位置: xge.h:2774  已注释: 否
- 签名: `XGE_API int xgeParticleRendererBind(xge_particle_renderer pRenderer, xge_particle_definition pDefinition, int iEmitter, xge_texture pTexture, const xge_material_t* pMaterial);`
- 实现: src/xge_particle_render.c:95（体 68 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK, XGE_ERROR_OUT_OF_MEMORY
- 用法: examples/xge_particles/common.c:92; test/test_particle_render.c:82; test/test_particle_render.c:104

## xgeParticleRendererUnbind
- 位置: xge.h:2776  已注释: 否
- 签名: `XGE_API int xgeParticleRendererUnbind(xge_particle_renderer pRenderer, xge_particle_definition pDefinition);`
- 实现: src/xge_particle_render.c:163（体 19 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: test/test_particle_render.c:90; test/test_particle_render.c:125

## xgeParticleRender
- 位置: xge.h:2779  已注释: 是
- 签名: `XGE_API int xgeParticleRender(xge_particle_renderer pRenderer, xge_particle_world pWorld, const xge_rect_t* pView);`
- 既有注释: /* Explicit view rectangle uses the same coordinates as particles; NULL disables draw culling. * Missing bindings draw a white quad. Simulation culling is controlled via SetVisible. */
- 实现: src/xge_particle_render.c:256（体 81 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_UNSUPPORTED
- 用法: examples/xge_particles/common.c:264; test/test_particle_render.c:72; test/test_particle_render.c:86

