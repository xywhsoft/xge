#include "xge.h"

#include <assert.h>
#include <stdio.h>

typedef struct {
    xge_image image;
    xge_image other;
    int calls;
} pixel_visit_fixture;

static int visit_nested(const xbytesview* view, void* user)
{
    pixel_visit_fixture* fixture = user;
    assert(view->Data == xgeImageGetPixels(fixture->image) && view->Size == 4);
    assert(xgeImagePremultiplyManaged(fixture->image) == XGE_ERROR_BUSY);
    ++fixture->calls;
    return XGE_ERROR_RESOURCE_FAILED;
}

static int visit(const xbytesview* view, void* user)
{
    pixel_visit_fixture* fixture = user;
    assert(view->Data == xgeImageGetPixels(fixture->image) && view->Size == 4);
    unsigned char output = 0;
    assert(xgeImagePixelViewReadByte(view, 0, &output) == XGE_OK && output == 100);
    assert(xgeImagePixelViewReadByte(view, 4, &output) == XGE_ERROR_INVALID_ARGUMENT && output == 100);
    assert(xgeImagePremultiplyManaged(fixture->image) == XGE_ERROR_BUSY);
    assert(xgeImagePremultiplyManaged(fixture->other) == XGE_OK);
    assert(xgeImageVisitPixels(fixture->image, visit_nested, fixture) == XGE_ERROR_RESOURCE_FAILED);
    assert(xgeImagePremultiplyManaged(fixture->image) == XGE_ERROR_BUSY);
    assert(xgeImagePixelViewReadByte(view, 0, &output) == XGE_OK && output == 100);
    ++fixture->calls;
    return XGE_ERROR_RESOURCE_FAILED;
}

int main(void)
{
    unsigned char pixels[] = {100, 50, 0, 128};
    unsigned char other[] = {100, 50, 0, 128};
    xge_image_t image = {.iWidth=1,.iHeight=1,.iFormat=XGE_PIXEL_RGBA8,
        .iStride=4,.pPixels=pixels,.iFlags=XGE_IMAGE_STRAIGHT_ALPHA};
    xge_image_t independent = image;
    independent.pPixels = other;
    pixel_visit_fixture fixture = {&image, &independent, 0};

    assert(xgeImageVisitPixels(NULL, visit, &fixture) == XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeImageVisitPixels(&image, NULL, &fixture) == XGE_ERROR_INVALID_ARGUMENT);
    assert(fixture.calls == 0);
    assert(xgeImageVisitPixels(&image, visit, &fixture) == XGE_ERROR_RESOURCE_FAILED);
    assert(fixture.calls == 2 && pixels[0] == 100 && other[0] == 50);
    assert(xgeImagePremultiplyManaged(&image) == XGE_OK && pixels[0] == 50);
    puts("XGE_IMAGE_PIXEL_VISIT_NATIVE_OK");
    return 0;
}
