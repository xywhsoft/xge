#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"

#undef NDEBUG
#include <assert.h>
#include <stdio.h>

static int32 wrong_thread(void *raw)
{
    xge_particle_renderer r = raw;
    bool consumed = true;
    assert(xgeParticleRendererBind(r, NULL, 0, NULL, NULL) == XGE_ERROR_INVALID_STATE);
    assert(xgeParticleRendererReleaseOwnerChecked(r, &consumed) == XGE_ERROR_INVALID_STATE && !consumed);
    return 0;
}

static void allocation_failures(void)
{
    int failed = 0, succeeded = 0;
    for (int budget = 0; budget < 8; ++budget) {
        xmemdebugsnapshot before, after;
        xrtMemDebugSnapshot(&before);
        assert(xrtMemDebugFailAfter(budget));
        xge_particle_renderer renderer = (xge_particle_renderer)(uintptr_t)1;
        int status = xgeParticleRendererCreateOwned(&renderer, 16, 4);
        xrtMemDebugFailClear();
        if (status == XGE_OK) {
            assert(renderer && xgeGraphicsOwnedResourceCount() == 1);
            assert(xgeParticleRendererFreeChecked(renderer) == XGE_OK);
            ++succeeded;
        } else {
            assert(status == XGE_ERROR_OUT_OF_MEMORY && renderer == NULL);
            ++failed;
        }
        xrtClearError();
        assert(xgeGraphicsOwnedResourceCount() == 0);
        xrtMemDebugSnapshot(&after);
        assert(after.LiveCount == before.LiveCount && after.LiveBytes == before.LiveBytes);
    }
    assert(failed > 0 && succeeded > 0);
}

static void binding_failures(xge_particle_definition definition)
{
    const unsigned char pixel[] = {255, 0, 0, 255};
    int failed = 0, succeeded = 0;
    for (int budget = 0; budget < 12; ++budget) {
        xmemdebugsnapshot before, after;
        xrtMemDebugSnapshot(&before);
        xge_particle_renderer renderer;
        xge_texture texture;
        assert(xgeParticleRendererCreateOwned(&renderer, 16, 4) == XGE_OK);
        assert(xgeTextureCreateRGBAOwned(&texture, 1, 1, pixel, sizeof(pixel)) == XGE_OK);
        assert(xrtMemDebugFailAfter(budget));
        int status = xgeParticleRendererBind(renderer, definition, 0, texture, NULL);
        xrtMemDebugFailClear();
        if (status == XGE_OK) {
            assert(texture->iRefCount == 2);
            ++succeeded;
        } else {
            assert(status == XGE_ERROR_OUT_OF_MEMORY && texture->iRefCount == 1);
            ++failed;
        }
        xrtClearError();
        assert(xgeParticleRendererFreeChecked(renderer) == XGE_OK);
        assert(texture->iRefCount == 1);
        xgeTextureFree(texture);
        assert(xgeGraphicsOwnedResourceCount() == 0);
        xrtMemDebugSnapshot(&after);
        assert(after.LiveCount == before.LiveCount && after.LiveBytes == before.LiveBytes);
    }
    assert(failed > 0 && succeeded > 0);
}

int main(void)
{
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    allocation_failures();
    xge_particle_renderer renderer;
    assert(xgeParticleRendererCreateOwned(&renderer, 0, 4) == XGE_ERROR_INVALID_ARGUMENT && !renderer);
    assert(xgeParticleRendererCreateOwned(&renderer, 16, 4) == XGE_OK);
    assert(xgeGraphicsOwnedResourceCount() == 1 && xgeUnitChecked() == XGE_ERROR_BUSY);

    xthread *thread = xrtThreadCreate(wrong_thread, renderer, 0);
    assert(thread && xrtThreadWait(thread) == XWAIT_OK);
    xrtThreadDestroy(thread);
    assert(xgeGraphicsOwnedResourceCount() == 1);

    xge_particle_emitter_t emitter;
    xgeParticleEmitterInit(&emitter);
    xge_particle_definition definition;
    assert(xgeParticleDefinitionCreate(&definition, &emitter, 1) == XGE_OK);
    const unsigned char pixel[] = {255, 0, 0, 255};
    xge_texture_t callerTexture = {0};
    assert(xgeTextureCreateRGBA(&callerTexture, 1, 1, pixel) == XGE_OK);
    assert(xgeParticleRendererBind(renderer, definition, 0, &callerTexture, NULL) == XGE_ERROR_INVALID_ARGUMENT);
    assert(callerTexture.iRefCount == 1);
    xgeTextureFree(&callerTexture);

    xge_texture texture;
    assert(xgeTextureCreateRGBAOwned(&texture, 1, 1, pixel, sizeof(pixel)) == XGE_OK);
    assert(xgeParticleRendererBind(renderer, definition, 0, texture, NULL) == XGE_OK);
    assert(texture->iRefCount == 2);
    xge_shader_t callerShader = {0};
    callerShader.iRefCount = 1;
    xge_material_t callerMaterial;
    xgeMaterialInit(&callerMaterial);
    xgeMaterialSetShader(&callerMaterial, &callerShader);
    assert(xgeParticleRendererBind(renderer, definition, 0, texture, &callerMaterial) == XGE_ERROR_INVALID_ARGUMENT);
    assert(texture->iRefCount == 2 && callerShader.iRefCount == 1);
    xgeMaterialFree(&callerMaterial);
    xgeTextureFree(texture);
    assert(xgeGraphicsOwnedResourceCount() == 2);
    assert(xgeParticleRendererUnbind(renderer, definition) == XGE_OK);
    assert(xgeGraphicsOwnedResourceCount() == 1);

    bool consumed = false;
    assert(xgeParticleRendererReleaseOwnerChecked(renderer, &consumed) == XGE_OK && consumed);
    binding_failures(definition);
    xgeParticleDefinitionFree(definition);
    assert(xgeGraphicsOwnedResourceCount() == 0 && xgeUnitChecked() == XGE_OK);
    assert(xgeParticleRendererCreate(&renderer, 16, 4) == XGE_OK);
    assert(xgeGraphicsOwnedResourceCount() == 0);
    xgeParticleRendererFree(renderer);
    puts("PARTICLE_RENDERER_OWNED_NATIVE_OK");
    return 0;
}
