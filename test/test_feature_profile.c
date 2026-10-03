#include "../xge.h"
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return 1; } } while (0)

int main(void)
{
    xge_desc_t desc = {0};
    xge_texture_t texture = {0};
    xge_buffer_t buffer = {0};
    uint32_t pixels[4] = {0xff0000ffu, 0xff00ff00u, 0xffff0000u, 0xffffffffu};
    int i;
    CHECK(xgeGetBuildFeatures() == XGE_FEATURES);
    desc.iRunMode = XGE_RUN_MANUAL;
    CHECK(xgeInit(&desc) == XGE_OK);
    CHECK(xgeInit(&desc) == XGE_ERROR_ALREADY_INITIALIZED);
    for (i = 0; i < 100; i++) {
        CHECK(xgeTextureCreateRGBA(&texture, 2, 2, pixels) == XGE_OK);
        CHECK(xgeTextureCreateRGBA(&texture, 2, 2, pixels) == XGE_ERROR_INVALID_STATE);
        CHECK(xgeBufferCreate(&buffer, XGE_BUFFER_VERTEX, XGE_BUFFER_DYNAMIC, pixels, sizeof(pixels)) == XGE_OK);
        CHECK(xgeBufferUpdate(&buffer, 0, pixels, sizeof(pixels)) == XGE_OK);
        xgeBufferFree(&buffer);
        xgeTextureFree(&texture);
        CHECK(texture.iWidth == 0 && texture.iHeight == 0);
    }
    CHECK(xgeBegin() == XGE_OK);
    CHECK(xgeFlush() == XGE_OK);
    xgePresent();
    xgeUnit();
    CHECK(xgeFlush() == XGE_ERROR_NOT_INITIALIZED);
    printf("feature profile passed: 0x%x\n", xgeGetBuildFeatures());
    return 0;
}
