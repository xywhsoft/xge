#ifdef XGE_XRT_HOST
#define XRT_MODULE_ALL
#ifdef _WIN32
#define XRT_USE_SHARED
#endif
#endif
#include "xge.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef XGE_XRT_HOST
#include <xge_host_xrt_contract.h>
static xge_host_xrt_proc resolve(void* user, const char* name)
{
    (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if (!strcmp(name, #symbol)) return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif

typedef struct mask_fixture_t {
    xge_shape_ex_scene mask;
    xge_shape_ex source;
    xge_pass nested;
    int fault;
    int calls;
} mask_fixture_t;

static int draw(void* raw, const xge_shape_ex_matrix_t* matrix)
{
    mask_fixture_t* fixture = raw;
    ++fixture->calls;
    assert(matrix && matrix->fE <= -7.0f && matrix->fF <= -3.0f);
    assert(xgeShapeExSceneClear(fixture->mask) == XGE_ERROR_BUSY);
    if (fixture->fault == 3) {
        xgeShapeExSceneDestroy(fixture->mask);
        fixture->mask = NULL;
    } else {
        assert(xgePassBegin(fixture->nested) == XGE_OK);
        if (fixture->fault == 1) return XGE_ERROR_CALLBACK_FAILED;
        if (fixture->fault == 2) return XGE_OK;
        if (fixture->fault == 4) {
            assert(xrtMemDebugFailAfter(0));
            void* allocation = xrtMalloc(64);
            xrtMemDebugFailClear();
            assert(allocation == NULL);
            return XGE_ERROR_OUT_OF_MEMORY;
        }
        assert(xgePassEnd(fixture->nested) == XGE_OK);
    }
    return xgeShapeExDrawPxEx(fixture->source, 0.25f, matrix, 1.0f);
}

static xge_shape_ex_scene make_mask(void)
{
    xge_shape_ex_scene result = NULL;
    xge_shape_ex shape = NULL;
    assert(xgeShapeExSceneCreate(&result) == XGE_OK);
    assert(xgeShapeExCreate(&shape) == XGE_OK);
    assert(xgeShapeExAppendRect(shape, 8, 4, 8, 16, 0, 0, 1) == XGE_OK);
    assert(xgeShapeExFillColor(shape, XGE_COLOR_RGBA(255,255,255,255)) == XGE_OK);
    assert(xgeShapeExSceneAdd(result, shape) == XGE_OK);
    xgeShapeExDestroy(shape);
    return result;
}

static int wrong_thread(void* raw)
{
    mask_fixture_t* fixture = raw;
    assert(xgeShapeExMaskCompositeScene(fixture->mask, XGE_SHAPE_EX_MASK_ALPHA,
        XGE_BLEND_ALPHA, (xge_rect_t){0,0,32,24}, draw, fixture, 0.25f, NULL, 1) == XGE_ERROR_INVALID_STATE);
    return 0;
}

static int frame(void* raw)
{
    (void)raw;
    xge_render_target target = NULL, nestedTarget = NULL;
    xge_pass outer = NULL;
    mask_fixture_t fixture = {0};
    fixture.mask = make_mask();
    assert(xgeShapeExCreate(&fixture.source) == XGE_OK);
    assert(xgeShapeExAppendRect(fixture.source, 0, 0, 32, 24, 0, 0, 1) == XGE_OK);
    assert(xgeShapeExFillColor(fixture.source, XGE_COLOR_RGBA(192,32,16,255)) == XGE_OK);
    assert(xgeRenderTargetCreateOwned(&target, 32, 24) == XGE_OK);
    assert(xgeRenderTargetCreateOwned(&nestedTarget, 4, 4) == XGE_OK);
    assert(xgePassCreateOwned(&outer, target, XGE_PASS_CLEAR_COLOR, 0x000000ff) == XGE_OK);
    assert(xgePassCreateOwned(&fixture.nested, nestedTarget, 0, 0) == XGE_OK);
    assert(xgePassBegin(outer) == XGE_OK);
    const xge_rect_t bounds = {0,0,32,24};
    xgeClipSet((xge_rect_t){2,2,28,20});

    xthread* worker = xrtThreadCreate(wrong_thread, &fixture, 0);
    assert(worker && xrtThreadWait(worker) == XWAIT_OK && fixture.calls == 0);
    xrtThreadDestroy(worker);
    const xge_rect_t outside = {40,40,2,2};
    assert(xgeShapeExMaskCompositeScene(fixture.mask, XGE_SHAPE_EX_MASK_ALPHA,
        XGE_BLEND_ALPHA, outside, draw, &fixture, 0.25f, NULL, 1) == XGE_OK);
    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);
    for (int i = 0; i < 1000; ++i) {
        assert(xgeShapeExMaskCompositeScene(fixture.mask, XGE_SHAPE_EX_MASK_ALPHA,
            XGE_BLEND_ALPHA, outside, draw, &fixture, 0.25f, NULL, 1) == XGE_OK);
    }
    xrtMemDebugSnapshot(&after);
    assert(fixture.calls == 0 && before.LiveBytes == after.LiveBytes);
    printf("native clipped masks per 1000: heap %llu/%llu/%llu\n",
        (unsigned long long)(after.AllocCount-before.AllocCount),
        (unsigned long long)(after.FreeCount-before.FreeCount),
        (unsigned long long)(after.ReallocCount-before.ReallocCount));
    const int faults[] = {4, 1, 2, 3};
    const int expected[] = {XGE_ERROR_OUT_OF_MEMORY, XGE_ERROR_CALLBACK_FAILED, XGE_ERROR_INVALID_STATE, XGE_OK};
    for (size_t f = 0; f < sizeof(faults)/sizeof(faults[0]); ++f) {
        fixture.fault = faults[f];
        assert(xgeShapeExMaskCompositeScene(fixture.mask, XGE_SHAPE_EX_MASK_ALPHA,
            XGE_BLEND_ALPHA, bounds, draw, &fixture, 0.25f, NULL, 1) == expected[f]);
        xrtClearError();
        assert(xgePassIsCurrent(outer) && !xgePassIsCurrent(fixture.nested));
        assert(xgeClipGet().fX == 2.0f && xgeClipGet().fW == 28.0f);
    }
    assert(fixture.calls == 4 && fixture.mask == NULL);
    assert(xgePassEnd(outer) == XGE_OK && xgePassIsIdle());

    unsigned char pixels[32*24*4];
    assert(xgeRenderTargetReadPixels(target, pixels, 32*4) == XGE_OK);
    const unsigned char* in = pixels + (12*32+12)*4;
    const unsigned char* out = pixels + (12*32+24)*4;
    assert(in[0] == 192 && in[1] == 32 && in[2] == 16 && in[3] == 255);
    assert(out[0] == 0 && out[1] == 0 && out[2] == 0 && out[3] == 255);
    xgePassFree(fixture.nested);
    xgeRenderTargetFree(nestedTarget);
    xgePassFree(outer);
    xgeRenderTargetFree(target);
    xgeShapeExDestroy(fixture.source);
    puts("native mask: pixels, callback failure and pass restoration passed");
    return 1;
}

int main(void)
{
#ifdef XGE_XRT_HOST
    const xge_host_xrt_contract_t binding = XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&binding, resolve, NULL, NULL) == XGE_HOST_XRT_OK);
#endif
    assert(xrtMemDebugEnable(true));
    const xge_desc_t desc = {.iWidth=96,.iHeight=72,.sTitle="Shape mask callback"};
    assert(xgeInit(&desc) == XGE_OK);
    assert(xgeRun(frame, NULL) == XGE_OK);
    assert(xgeUnitChecked() == XGE_OK);
    return 0;
}
