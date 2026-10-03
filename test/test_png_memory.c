#include "../xge.h"
#include <stdio.h>
#include <string.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "PNG memory test failed at %d: %s\n", __LINE__, #c); return 1; } } while (0)

int main(void)
{
    const unsigned char straight[] = {255, 128, 64, 64, 0, 0, 0, 0};
    const unsigned char premultiplied[] = {64, 32, 16, 64, 0, 0, 0, 0};
    const unsigned char signature[] = {137, 80, 78, 71, 13, 10, 26, 10};
    xge_image_t image = {0};
    void* png = (void*)1;
    size_t bytes = 123;
    int result;
    result = xgeImageEncodePNG(2, 1, straight, 8, &png, &bytes);
    CHECK(result == XGE_OK && bytes > sizeof(signature));
    CHECK(memcmp(png, signature, sizeof(signature)) == 0);
    CHECK(xgeImageLoadMemoryEx(&image, png, (int)bytes, XGE_IMAGE_STRAIGHT_ALPHA) == XGE_OK);
    CHECK(image.iWidth == 2 && image.iHeight == 1);
    CHECK(memcmp(image.pPixels, straight, sizeof(straight)) == 0);
    xgeImageFree(&image); xrtFree(png);

    {
        const unsigned char padded[24] = {
            255, 128, 64, 64, 0, 0, 0, 0, 9, 9, 9, 9,
            10, 20, 30, 255, 40, 50, 60, 255, 8, 8, 8, 8
        };
        CHECK(xgeImageEncodePNG(2, 2, padded, 12, &png, &bytes) == XGE_OK);
        CHECK(xgeImageLoadMemoryEx(&image, png, (int)bytes, XGE_IMAGE_STRAIGHT_ALPHA) == XGE_OK);
        CHECK(image.iWidth == 2 && image.iHeight == 2);
        CHECK(memcmp(image.pPixels, padded, 8) == 0);
        CHECK(memcmp((unsigned char*)image.pPixels + 8, padded + 12, 8) == 0);
        xgeImageFree(&image); xrtFree(png);
    }

    CHECK(xgeImageEncodePNGEx(2, 1, premultiplied, 8,
        XGE_IMAGE_PREMULTIPLIED, &png, &bytes) == XGE_OK);
    CHECK(xgeImageLoadMemoryEx(&image, png, (int)bytes, XGE_IMAGE_STRAIGHT_ALPHA) == XGE_OK);
    CHECK(((unsigned char*)image.pPixels)[0] >= 254);
    CHECK(((unsigned char*)image.pPixels)[1] >= 126 && ((unsigned char*)image.pPixels)[1] <= 129);
    CHECK(((unsigned char*)image.pPixels)[2] >= 62 && ((unsigned char*)image.pPixels)[2] <= 66);
    CHECK(((unsigned char*)image.pPixels)[3] == 64);
    xgeImageFree(&image); xrtFree(png);

    CHECK(xgeImageEncodePNGEx(0, 1, straight, 8, 0, &png, &bytes) == XGE_ERROR_INVALID_ARGUMENT);
    CHECK(png == NULL && bytes == 0);
    CHECK(xgeImageEncodePNGEx(2, 1, straight, 8,
        XGE_IMAGE_PREMULTIPLIED | XGE_IMAGE_STRAIGHT_ALPHA,
        &png, &bytes) == XGE_ERROR_INVALID_ARGUMENT);
    CHECK(png == NULL && bytes == 0);
    puts("PNG memory encode/decode and alpha conversion passed");
    return 0;
}
