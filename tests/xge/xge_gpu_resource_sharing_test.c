#ifndef XRT_MODULE_ALL
#define XRT_MODULE_ALL
#endif
#include "xge.h"

#include <assert.h>
#include <stdio.h>

static int other_thread(void* raw)
{
    xge_texture texture = raw;
    xge_texture rejected = texture;
    assert(xgeTextureShareOwned(texture, &rejected) == XGE_ERROR_INVALID_STATE && rejected == NULL);
    return 0;
}

int main(void)
{
    const unsigned char pixel[] = {44, 18, 7, 255};
    xge_texture texture = NULL;
    assert(xgeTextureCreateRGBAOwned(&texture, 1, 1, pixel, sizeof(pixel)) == XGE_OK);
    assert(xgeGraphicsOwnedResourceCount() == 1);

    xge_texture_t raw_texture = {0};
    assert(xgeTextureCreateRGBA(&raw_texture, 1, 1, pixel) == XGE_OK);
    xge_texture output = texture;
    assert(xgeTextureShareOwned(NULL, &output) == XGE_ERROR_INVALID_ARGUMENT && !output);
    assert(xgeTextureShareOwned(&raw_texture, &output) == XGE_ERROR_INVALID_STATE && !output);
    assert(raw_texture.iRefCount == 1);
    xgeTextureFree(&raw_texture);

    xge_shader_t raw_shader = {.iRefCount = 1};
    xge_shader shader_output = &raw_shader;
    assert(xgeShaderShareOwned(&raw_shader, &shader_output) == XGE_ERROR_INVALID_STATE && !shader_output);
    assert(raw_shader.iRefCount == 1);
    xge_render_target_t raw_target = {0};
    xge_render_target target_output = &raw_target;
    assert(xgeRenderTargetShareOwned(&raw_target, &target_output) == XGE_ERROR_INVALID_STATE && !target_output);
    assert(raw_target._iOwnerReferences == 0);

    xthread* worker = xrtThreadCreate(other_thread, texture, 0);
    assert(worker && xrtThreadWaitFor(worker, 5000) == XWAIT_OK);
    xrtThreadDestroy(worker);
    assert(texture->iRefCount == 1);
    texture->iRefCount = INT32_MAX;
    assert(xgeTextureShareOwned(texture, &output) == XGE_ERROR_INVALID_STATE && !output);
    texture->iRefCount = 1;

    assert(xrtMemDebugFailAfter(0));
    assert(xgeTextureShareOwned(texture, &output) == XGE_OK && output == texture);
    assert(!xrtMemDebugFailTriggered() && texture->iRefCount == 2);
    xrtMemDebugFailClear();
    xgeTextureFree(texture);
    assert(output->iWidth == 1 && output->iRefCount == 1);
    assert(xgeGraphicsOwnedResourceCount() == 1 && xgeUnitChecked() == XGE_ERROR_BUSY);
    xgeTextureFree(output);
    assert(xgeGraphicsOwnedResourceCount() == 0);

    xge_desc_t desc = {.iWidth = 16, .iHeight = 12, .iRunMode = XGE_RUN_MANUAL};
    assert(xgeInit(&desc) == XGE_OK);
    xge_render_target target = NULL;
    assert(xgeRenderTargetWindowOwned(&target) == XGE_OK);
    assert(xgeRenderTargetShareOwned(target, &target_output) == XGE_OK && target_output == target);
    bool consumed = false;
    assert(xgeRenderTargetReleaseOwnerChecked(target, &consumed) == XGE_OK && consumed);
    assert(target_output->_iOwnerReferences == 1 && xgeGraphicsOwnedResourceCount() == 1);
    assert(xgeUnitChecked() == XGE_ERROR_BUSY);
    assert(xgeRenderTargetReleaseOwnerChecked(target_output, &consumed) == XGE_OK && consumed);
    assert(xgeGraphicsOwnedResourceCount() == 0 && xgeUnitChecked() == XGE_OK);
    puts("xge_gpu_resource_sharing_test passed");
    return 0;
}
