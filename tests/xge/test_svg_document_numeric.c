#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"

#undef NDEBUG
#include <assert.h>
#include <float.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static const char document[] =
    "<svg xmlns='http://www.w3.org/2000/svg' width='3.25e1' height='16.25' viewBox='0,0,32.5,16.25'>"
    "<rect id='panel' x='.5' y='+1.25' width='20.5' height='8.25' fill='#00af80'/>"
    "<path id='outline' d='M2 2 L4 4 L6 2' stroke='red' fill='none' stroke-width='1.25' "
    "stroke-miterlimit='3.5' stroke-dasharray='2.5,1.25' stroke-dashoffset='.625'/>"
    "<path id='compat' d='M2 3 L4 5 L6 3' stroke='red' fill='none' stroke-miterlimit='+NaN'/>"
    "<path id='overflow' d='M2 4 L4 6 L6 4' stroke='red' fill='none' stroke-miterlimit='1e400'/>"
    "</svg>";

static void require_decimal_comma(void)
{
    const char* locales[] = {"de_DE.UTF-8", "German_Germany.1252", "German_Germany"};
    for (size_t i = 0; i < sizeof(locales) / sizeof(*locales); ++i) {
        if (setlocale(LC_NUMERIC, locales[i]) && strcmp(localeconv()->decimal_point, ",") == 0)
            return;
    }
    assert(!"a decimal-comma numeric locale must be available");
}

static void load(xge_svg svg)
{
    assert(xgeSvgLoadMemory(svg, document, sizeof(document) - 1) == XGE_OK);
}

static xge_shape_ex shape(xge_svg svg, const char* name)
{
    xge_svg_paint paint;
    xge_shape_ex result;
    assert(xgeSvgPaintGetByName(svg, name, &paint) == XGE_OK);
    assert(xgeSvgPaintShapeGet(paint, &result) == XGE_OK);
    return result;
}

static void measure(xge_svg svg)
{
    float width, height;
    assert(xgeSvgGetSize(svg, &width, &height) == XGE_OK);
    assert(width == 32.5f && height == 16.25f);
    xge_rect_t view_box;
    assert(xgeSvgGetViewBox(svg, &view_box) == XGE_OK);
    assert(view_box.fX == 0 && view_box.fY == 0 && view_box.fW == width && view_box.fH == height);

    xge_svg_paint paint;
    xge_rect_t bounds;
    assert(xgeSvgPaintGetByName(svg, "panel", &paint) == XGE_OK);
    assert(xgeSvgPaintGetBounds(paint, 0.25f, &bounds) == XGE_OK);
    assert(bounds.fX == .5f && bounds.fY == 1.25f && bounds.fW == 20.5f && bounds.fH == 8.25f);

    xge_shape_ex outline = shape(svg, "outline");
    float miter, offset;
    int dash_count;
    const float* dashes;
    assert(xgeShapeExStrokeMiterLimitGet(outline, &miter) == XGE_OK && miter == 3.5f);
    assert(xgeShapeExStrokeDashGet(outline, &dashes, &dash_count, &offset) == XGE_OK);
    assert(dash_count == 2 && dashes[0] == 2.5f && dashes[1] == 1.25f && offset == .625f);
    assert(xgeShapeExStrokeMiterLimitGet(shape(svg, "compat"), &miter) == XGE_OK && miter == FLT_MAX);
    assert(xgeShapeExStrokeMiterLimitGet(shape(svg, "overflow"), &miter) == XGE_OK && miter == FLT_MAX);
}

int main(void)
{
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xge_svg svg;
    assert(xgeSvgCreate(&svg) == XGE_OK);
    assert(setlocale(LC_NUMERIC, "C"));
    load(svg);
    measure(svg);

    for (int i = 0; i < 3; ++i) {
        clock_t start = clock();
        for (int count = 0; count < 512; ++count) load(svg);
        printf("SVG_NUMERIC_LOAD_SECONDS %.6f\n", (double)(clock() - start) / CLOCKS_PER_SEC);
    }
    require_decimal_comma();
    load(svg);
#if defined(XGE_TEST_SVG_NUMERIC_BASELINE)
    float width, height;
    assert(xgeSvgGetSize(svg, &width, &height) == XGE_OK);
    assert(width != 32.5f || height != 16.25f);
    puts("SVG_DOCUMENT_NUMERIC_BASELINE_REPRODUCED");
#else
    measure(svg);
    assert(!xrtGetError());
    int count;
    xge_svg alias = svg;
    assert(xgeSvgShareOwned(NULL, &alias) == XGE_ERROR_INVALID_ARGUMENT && alias == NULL);
    assert(xgeSvgShareOwned(svg, NULL) == XGE_ERROR_INVALID_ARGUMENT);
    assert(xgeSvgRefCountGet(svg, &count) == XGE_OK && count == 1);

    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    for (int i = 0; i < 1000; ++i) {
        assert(xgeSvgShareOwned(svg, &alias) == XGE_OK && alias == svg);
        xgeSvgDestroy(alias);
    }
    assert(xrtMemDebugFailAfter(-1));
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount == after.LiveCount && before.LiveBytes == after.LiveBytes);
    assert(xgeSvgShareOwned(svg, &alias) == XGE_OK);
    assert(xgeSvgRefCountGet(svg, &count) == XGE_OK && count == 2);
    xgeSvgDestroy(svg);
    svg = alias;
    measure(svg);
    assert(xgeSvgRefCountGet(svg, &count) == XGE_OK && count == 1);
    puts("SVG_DOCUMENT_NUMERIC_NATIVE_OK");
#endif
    xgeSvgDestroy(svg);
    return 0;
}
