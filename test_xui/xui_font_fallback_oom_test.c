#include "../xge.h"
#include "../xui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(e) do { if (!(e)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #e); exit(1); } } while (0)

static void same_live(const xmemdebugsnapshot* before)
{
    xmemdebugsnapshot after;
    xrtMemDebugSnapshot(&after);
    if (after.LiveCount != before->LiveCount || after.LiveBytes != before->LiveBytes)
        fprintf(stderr, "live before=%zu/%zu after=%zu/%zu\n", before->LiveCount, before->LiveBytes, after.LiveCount, after.LiveBytes);
    CHECK(after.LiveCount == before->LiveCount && after.LiveBytes == before->LiveBytes);
    CHECK(after.InvalidFreeCount == before->InvalidFreeCount &&
        after.DoubleFreeCount == before->DoubleFreeCount);
}

int main(void)
{
    xui_proxy_t proxy = xuiProxyXge();
    xui_font source, full, sized;
    xmemdebugsnapshot before;
    xui_vec2_t expected, actual;
    unsigned i, failures = 0;
    CHECK(xrtMemDebugEnable(true));
    CHECK(xgeFontFallbackSet("test/data/xge_opentype_fixture.ttf", 40) == XGE_OK);
    CHECK(proxy.fontLoadFile(&proxy, &source, "test/data/xge_fallback_no_i.ttf", 40, 0) == XUI_OK);
    CHECK(proxy.fontLoadFile(&proxy, &full, "test/data/xge_opentype_fixture.ttf", 60, 0) == XUI_OK);
    CHECK(proxy.textMeasure(&proxy, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=full, .sText="ffi", .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &expected) == XUI_OK);
    /* Warm the existing source's shaping cache before the allocation baseline. */
    CHECK(proxy.textMeasure(&proxy, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=source, .sText="ffi", .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &actual) == XUI_OK);
    for (i = 0; i < 32; i++) {
        int result;
        bool fired;
        sized = NULL;
        xrtClearError();
        xrtMemDebugSnapshot(&before);
        CHECK(xrtMemDebugFailAfter(i));
        result = proxy.fontCreateSized(&proxy, &sized, source, 60);
        fired = xrtMemDebugFailTriggered();
        xrtMemDebugFailClear();
        xrtClearError();
        if (fired) {
            CHECK(result == XGE_ERROR_OUT_OF_MEMORY && sized == NULL);
            same_live(&before);
            failures++;
        } else {
            CHECK(result == XUI_OK && sized != NULL);
            CHECK(proxy.textMeasure(&proxy, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=sized, .sText="ffi", .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &actual) == XUI_OK);
            CHECK(fabsf(actual.fX - expected.fX) < .001f);
            proxy.fontDestroy(&proxy, sized);
            same_live(&before);
            break;
        }
        /* A failed construction must leave the caller's source usable. */
        CHECK(proxy.textMeasure(&proxy, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=source, .sText="ffi", .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &actual) == XUI_OK);
        CHECK(actual.fX > 0 && actual.fX < expected.fX);
    }
    CHECK(i < 32 && failures == 3);
    proxy.fontDestroy(&proxy, source);
    proxy.fontDestroy(&proxy, full);
    xgeFontFallbackClear();
    xrtClearError();
    xrtMemDebugSnapshot(&before);
    CHECK(before.LiveCount == 0 && before.LiveBytes == 0 &&
        before.InvalidFreeCount == 0 && before.DoubleFreeCount == 0);
    printf("Native XUI sized-font fallback: %u allocation failures, rollback, retry, complete fallback metrics and zero live allocations passed\n", failures);
    return 0;
}
