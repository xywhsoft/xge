#include "src/xui_internal.h"
#include "src/xui_text_internal.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>

#define SHY "\xc2\xad"
#define ZWSP "\xe2\x80\x8b"
#define WJ "\xe2\x81\xa0"
#define FEFF "\xef\xbb\xbf"
static int checks, failures, ownedAllocs, liveLayouts, displayFail = -1;
static int scaleRuns, scaleAllocations;
static const char *phase = "setup";
#define CHECK(e) do { ++checks; if (!(e)) { if (failures < 24) { printf("FAIL %s:%d: %s\n", phase, __LINE__, #e); } ++failures; } } while (0)
static void *owned_malloc(size_t n) { ++ownedAllocs; return xrtMalloc(n); }
static void *owned_realloc(void *p, size_t n) { ++ownedAllocs; return xrtRealloc(p, n); }
static struct {
    xui_text_layout layout;
    const char *source, *borrowed;
    xui_text_line_t lines[128];
    int count;
} snapshots[8];
static int layout_create(xui_context c, xui_text_layout *out, const xui_text_layout_desc_t *d)
{
    int i, ret = xuiTextLayoutCreate(c, out, d);
    if (ret != XUI_OK) return ret;
    CHECK(liveLayouts < 8);
    snapshots[liveLayouts].layout = *out;
    snapshots[liveLayouts].source = d->sText;
    snapshots[liveLayouts].borrowed = xuiTextLayoutGetText(*out);
    snapshots[liveLayouts].count = xuiTextLayoutGetLineCount(*out);
    CHECK(snapshots[liveLayouts].count <= 128);
    for (i = 0; i < snapshots[liveLayouts].count && i < 128; ++i)
        CHECK(xuiTextLayoutGetLine(*out, i, &snapshots[liveLayouts].lines[i]) == XUI_OK);
    ++liveLayouts;
    return ret;
}
static void layout_destroy(xui_text_layout layout)
{
    int i;
    xui_text_line_t line;
    CHECK(liveLayouts > 0);
    --liveLayouts;
    CHECK(snapshots[liveLayouts].layout == layout);
    CHECK(xuiTextLayoutGetText(layout) == snapshots[liveLayouts].borrowed);
    CHECK(strcmp(xuiTextLayoutGetText(layout), snapshots[liveLayouts].source) == 0);
    for (i = 0; i < snapshots[liveLayouts].count && i < 128; ++i) {
        CHECK(xuiTextLayoutGetLine(layout, i, &line) == XUI_OK);
        CHECK(line.iTextOffset == snapshots[liveLayouts].lines[i].iTextOffset);
        CHECK(line.iTextSize == snapshots[liveLayouts].lines[i].iTextSize);
    }
    xuiTextLayoutDestroy(layout);
}
static int display_line(xui_text_layout layout, int index, const char **text, int *size)
{
    if (displayFail >= 0 && displayFail-- == 0) return XUI_ERROR_OUT_OF_MEMORY;
    return xuiInternalTextLayoutGetDisplayLine(layout, index, text, size);
}
/* Count these consumers' allocations, not the separately compiled shared layout. */
#define xrtMalloc owned_malloc
#define xrtRealloc owned_realloc
#define xuiTextLayoutCreate layout_create
#define xuiTextLayoutDestroy layout_destroy
#define xuiInternalTextLayoutGetDisplayLine display_line
#include "../src/xui_label.c"
#include "../src/xui_hyperlink.c"
#include "../src/xui_msgbox.c"
#include "../src/xui_msgtip.c"
#include "../src/xui_toast.c"
#undef xrtMalloc
#undef xrtRealloc
#undef xuiTextLayoutCreate
#undef xuiTextLayoutDestroy
#undef xuiInternalTextLayoutGetDisplayLine

typedef struct test_case_t {
    const char *name, *raw;
    int width, mode, count;
    const char *expected[4];
} test_case_t;
static const test_case_t cases[] = {
    {"SHY wrap", "ab" SHY "cd", 30, XUI_TEXT_WRAP_WORD, 2, {"ab-", "cd"}},
    {"SHY fits", "ab" SHY "cd", 100, XUI_TEXT_WRAP_WORD, 1, {"abcd"}},
    {"SHY no wrap", "ab" SHY "cd", 20, XUI_TEXT_WRAP_NONE, 1, {"abcd"}},
    {"zero width", "A" WJ "B" FEFF "C" ZWSP "D" SHY "E", 100, XUI_TEXT_WRAP_WORD, 1, {"ABCDE"}},
    {"ZWSP wrap", "ab" ZWSP "cd", 20, XUI_TEXT_WRAP_WORD, 2, {"ab", "cd"}},
    {"WJ/FEFF protect", "A" WJ "B" FEFF "C", 10, XUI_TEXT_WRAP_WORD, 1, {"ABC"}},
    {"invisible only", SHY ZWSP WJ FEFF, 100, XUI_TEXT_WRAP_WORD, 0, {NULL}},
    {"CHAR SHY", "ab" SHY "cd", 20, XUI_TEXT_WRAP_CHAR, 2, {"ab", "cd"}},
    {"CHAR ZWSP", "ab" ZWSP "cd", 20, XUI_TEXT_WRAP_CHAR, 2, {"ab", "cd"}},
    {"CHAR zero width", "A" WJ "B" FEFF "C" ZWSP "D" SHY "E", 100, XUI_TEXT_WRAP_CHAR, 1, {"ABCDE"}},
    {"CHAR invisible", SHY ZWSP WJ FEFF, 100, XUI_TEXT_WRAP_CHAR, 0, {NULL}},
    {"CHAR SHY fits", "ab" SHY "cd", 100, XUI_TEXT_WRAP_CHAR, 1, {"abcd"}}
};
static struct { char text[128]; xui_rect_t rect; uint32_t color, flags; } draws[128];
static int drawCount, injectedDrawError;
static int capture(xui_proxy p, xui_draw_context draw, xui_font font,
    const char *text, xui_rect_t rect, uint32_t color, uint32_t flags)
{
    (void)p; (void)draw; (void)font;
    CHECK(drawCount < 128 && strlen(text) < 128);
    if (drawCount < 128) {
        snprintf(draws[drawCount].text, sizeof(draws[drawCount].text), "%s", text);
        draws[drawCount].rect = rect; draws[drawCount].color = color; draws[drawCount].flags = flags;
    }
    ++drawCount;
    return injectedDrawError ? XUI_ERROR_BACKEND_FAILED : XUI_OK;
}
static void verify(const test_case_t *tc, uint32_t flags, uint32_t color, int middle, int bottom)
{
    int i, y = 7;
    if (middle) y += (60 - (tc->count * 20 + (tc->count - 1) * 2)) / 2;
    if (bottom) y += 60 - (tc->count * 20 + (tc->count - 1) * 2);
    CHECK(drawCount == tc->count);
    for (i = 0; i < tc->count && i < drawCount; ++i) {
        CHECK(strcmp(draws[i].text, tc->expected[i]) == 0);
        CHECK(draws[i].flags == flags);
        CHECK(draws[i].color == color);
        CHECK(draws[i].rect.fX == 5 && draws[i].rect.fW == tc->width);
        CHECK(draws[i].rect.fY == y + i * 22 && draws[i].rect.fH == 20);
    }
    CHECK(liveLayouts == 0);
    CHECK(ownedAllocs == 0);
}
static int render_widget(xui_widget w, xui_draw_context draw, int link, uint32_t state)
{
    return link ? __xuiHyperlinkCacheRender(w, draw, state, NULL) : __xuiLabelCacheRender(w, draw, state, NULL);
}
static void widget_cases(xui_context c, xui_draw_context draw, xui_font font, int link)
{
    xui_widget w;
    xui_label_desc_t ld;
    xui_hyperlink_desc_t hd;
    xui_style_property_t props[3];
    int i, state;
    uint32_t normal = XUI_COLOR_RGBA(11, 31, 61, 255), active = XUI_COLOR_RGBA(83, 17, 41, 255);
    uint32_t disabled = XUI_COLOR_RGBA(79, 79, 79, 255);
    memset(&ld, 0, sizeof(ld)); ld.iSize = sizeof(ld); ld.pFont = font;
    ld.iTextColor = normal; ld.iDisabledTextColor = disabled; ld.fLineGap = 2;
    ld.bUnderline = 1; ld.iTextFlags = XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE;
    memset(&hd, 0, sizeof(hd)); hd.iSize = sizeof(hd); hd.pFont = font;
    hd.iTextColor = normal; hd.iActiveTextColor = active; hd.iHoverTextColor = active;
    hd.iDisabledTextColor = disabled; hd.fLineGap = 2; hd.bUnderline = 1; hd.bActiveUnderline = 1;
    hd.iTextFlags = XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_BOTTOM;
    CHECK((link ? xuiHyperlinkCreate(c, &w, &hd) : xuiLabelCreate(c, &w, &ld)) == XUI_OK);
    CHECK(xuiWidgetSetPadding(w, (xui_thickness_t){5, 7, 3, 4}) == XUI_OK);
    for (state = 0; state < 3; ++state) {
        CHECK(xuiWidgetSetEnabled(w, state != 2) == XUI_OK);
        for (i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); ++i) {
            const test_case_t *tc = &cases[i];
            uint32_t color = state == 2 ? disabled : (link && state == 1 ? active : normal);
            uint32_t flags = (link ? XUI_TEXT_ALIGN_RIGHT : XUI_TEXT_ALIGN_CENTER) | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP;
            phase = tc->name;
            if (!link || state != 2) flags |= XUI_TEXT_UNDERLINE;
            CHECK(xuiWidgetSetRect(w, (xui_rect_t){17, -13, tc->width + 8, 71}) == XUI_OK);
            CHECK((link ? xuiHyperlinkSetText(w, tc->raw) : xuiLabelSetText(w, tc->raw)) == XUI_OK);
            CHECK((link ? xuiHyperlinkSetWrapMode(w, tc->mode) : xuiLabelSetWrapMode(w, tc->mode)) == XUI_OK);
            drawCount = ownedAllocs = 0;
            CHECK(render_widget(w, draw, link, state == 1 ? XUI_WIDGET_STATE_ACTIVE : 0) == XUI_OK);
            verify(tc, flags, color, !link, link);
            CHECK(strcmp(link ? xuiHyperlinkGetText(w) : xuiLabelGetText(w), tc->raw) == 0);
        }
    }
    CHECK(xuiWidgetSetEnabled(w, 1) == XUI_OK);
    CHECK((link ? xuiHyperlinkSetText(w, "ab" SHY "cd") : xuiLabelSetText(w, "ab" SHY "cd")) == XUI_OK);
    CHECK(xuiWidgetSetRect(w, (xui_rect_t){0, 0, 108, 71}) == XUI_OK);
    phase = "display failure"; drawCount = ownedAllocs = 0; displayFail = 0;
    CHECK(render_widget(w, draw, link, 0) == XUI_ERROR_OUT_OF_MEMORY);
    CHECK(drawCount == 0 && liveLayouts == 0); displayFail = -1;
    phase = "draw failure"; injectedDrawError = 1;
    CHECK(render_widget(w, draw, link, 0) == XUI_ERROR_BACKEND_FAILED);
    CHECK(liveLayouts == 0); injectedDrawError = 0;
    phase = "recovery"; drawCount = ownedAllocs = 0;
    CHECK(render_widget(w, draw, link, 0) == XUI_OK);
    CHECK(drawCount == 1 && strcmp(draws[0].text, "abcd") == 0 && liveLayouts == 0);
    phase = "inline style";
    memset(props, 0, sizeof(props));
    for (i = 0; i < 3; ++i) { props[i].iSize = sizeof(props[i]); props[i].tValue.iSize = sizeof(props[i].tValue); }
    props[0].sName = link ? "hyperlink.text.color" : "text.color";
    props[0].tValue.iType = XUI_STYLE_VALUE_COLOR; props[0].tValue.iColor = active;
    props[1].sName = "text.flags"; props[1].tValue.iType = XUI_STYLE_VALUE_INT;
    props[1].tValue.iInt = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP;
    props[2].sName = "text.underline"; props[2].tValue.iType = XUI_STYLE_VALUE_BOOL;
    CHECK(xuiWidgetSetInlineStyle(w, props, 3) == XUI_OK);
    drawCount = ownedAllocs = 0;
    CHECK(render_widget(w, draw, link, 0) == XUI_OK);
    CHECK(drawCount == 1 && strcmp(draws[0].text, "abcd") == 0);
    CHECK(draws[0].color == active && draws[0].flags == (XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP));
    CHECK(draws[0].rect.fX == 5 && draws[0].rect.fY == 7 && draws[0].rect.fW == 100 && draws[0].rect.fH == 20);
    CHECK(ownedAllocs == 0 && liveLayouts == 0);
    phase = "clipped height";
    CHECK((link ? xuiHyperlinkSetWrapMode(w, XUI_TEXT_WRAP_WORD) : xuiLabelSetWrapMode(w, XUI_TEXT_WRAP_WORD)) == XUI_OK);
    for (i = 0; i < 4; ++i) {
        static const int heights[] = {19, 20, 41, 42};
        static const int counts[] = {0, 1, 1, 2};
        CHECK(xuiWidgetSetRect(w, (xui_rect_t){-17, 13, 38, heights[i] + 11}) == XUI_OK);
        drawCount = ownedAllocs = 0;
        CHECK(render_widget(w, draw, link, 0) == XUI_OK);
        CHECK(drawCount == counts[i] && ownedAllocs == 0 && liveLayouts == 0);
        if (drawCount > 0) {
            CHECK(strcmp(draws[0].text, "ab-") == 0);
            CHECK(draws[0].rect.fY == 7 && draws[0].rect.fH == 20);
            CHECK(draws[0].flags == (XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP));
        }
        if (drawCount > 1) {
            CHECK(strcmp(draws[1].text, "cd") == 0);
            CHECK(draws[1].rect.fY == 29 && draws[1].rect.fH == 20);
        }
    }
    phase = "second display failure"; displayFail = 1; drawCount = ownedAllocs = 0;
    CHECK(render_widget(w, draw, link, 0) == XUI_ERROR_OUT_OF_MEMORY);
    CHECK(drawCount == 1 && strcmp(draws[0].text, "ab-") == 0 && liveLayouts == 0);
    displayFail = -1; drawCount = ownedAllocs = 0;
    CHECK(render_widget(w, draw, link, 0) == XUI_OK);
    CHECK(drawCount == 2 && strcmp(draws[1].text, "cd") == 0 && liveLayouts == 0 && ownedAllocs == 0);
    xuiWidgetDestroy(w);
}
static void message_cases(xui_context c, xui_draw_context draw, xui_font font, int consumer)
{
    struct xui_msgbox_t box;
    struct xui_msgtip_t tip;
    struct xui_toast_t toast;
    xui_toast_item_view_t view;
    int i, ret;
    uint32_t color = XUI_COLOR_RGBA(23, 47, 91, 255);
    memset(&box, 0, sizeof(box)); box.iMagic = XUI_MSGBOX_MAGIC; box.pContext = c; box.pFont = font;
    box.tColors.iMessageColor = color;
    memset(&tip, 0, sizeof(tip)); tip.iMagic = XUI_MSGTIP_MAGIC; tip.pContext = c; tip.pFont = font;
    tip.tColors.iTextColor = color;
    memset(&toast, 0, sizeof(toast)); toast.iMagic = XUI_TOAST_MAGIC; toast.pContext = c; toast.pFont = font;
    toast.tColors.iTextColor = toast.tColors.iMutedTextColor = color; toast.iActiveCount = 1;
    view.pToast = &toast; view.iSlot = 0;
    for (i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); ++i) {
        const test_case_t *tc = &cases[i];
        if (consumer == 1 ? tc->mode != XUI_TEXT_WRAP_CHAR : tc->mode != XUI_TEXT_WRAP_WORD) continue;
        phase = tc->name;
        box.sMessage = tip.sText = toast.arrActive[0].sMessage = (char*)tc->raw;
        box.tMessageRect = tip.tTextRect = toast.arrActive[0].tMessageRect = (xui_rect_t){5, 7, tc->width, 60};
        drawCount = ownedAllocs = 0;
        ret = consumer == 0 ? __xuiMsgBoxDrawTextLayout(&box, draw) : consumer == 1 ?
            __xuiMsgTipDrawTextLayout(&tip, draw) : __xuiToastRender(NULL, draw, 0, &view);
        CHECK(ret == XUI_OK);
        verify(tc, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP, color, 0, 0);
        CHECK(strcmp(box.sMessage, tc->raw) == 0 && strcmp(tip.sText, tc->raw) == 0 && strcmp(toast.arrActive[0].sMessage, tc->raw) == 0);
    }
    if (consumer == 2) {
        toast.arrActive[0].sMessage = NULL;
        for (i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); ++i) {
            const test_case_t *tc = &cases[i];
            if (tc->mode != XUI_TEXT_WRAP_WORD) continue;
            phase = "Toast title"; toast.arrActive[0].sTitle = (char*)tc->raw;
            toast.arrActive[0].tTitleRect = (xui_rect_t){5, 7, tc->width, 60};
            drawCount = ownedAllocs = 0;
            CHECK(__xuiToastRender(NULL, draw, 0, &view) == XUI_OK);
            verify(tc, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP, color, 1, 0);
            CHECK(strcmp(toast.arrActive[0].sTitle, tc->raw) == 0);
        }
    }
    box.sMessage = tip.sText = toast.arrActive[0].sMessage = "ab" SHY "cd";
    box.tMessageRect = tip.tTextRect = toast.arrActive[0].tMessageRect = (xui_rect_t){5, 7, consumer == 1 ? 20 : 30, 60};
    toast.arrActive[0].sTitle = NULL;
    for (i = 0; i < 2; ++i) {
        phase = "message display failure"; displayFail = i; drawCount = ownedAllocs = 0;
        ret = consumer == 0 ? __xuiMsgBoxDrawTextLayout(&box, draw) : consumer == 1 ?
            __xuiMsgTipDrawTextLayout(&tip, draw) : __xuiToastRender(NULL, draw, 0, &view);
        CHECK(ret == (consumer == 2 ? XUI_OK : XUI_ERROR_OUT_OF_MEMORY));
        CHECK(drawCount == i && liveLayouts == 0 && ownedAllocs == 0);
        if (i) CHECK(strcmp(draws[0].text, consumer == 1 ? "ab" : "ab-") == 0);
        displayFail = -1;
    }
    phase = "message recovery"; drawCount = ownedAllocs = 0;
    ret = consumer == 0 ? __xuiMsgBoxDrawTextLayout(&box, draw) : consumer == 1 ?
        __xuiMsgTipDrawTextLayout(&tip, draw) : __xuiToastRender(NULL, draw, 0, &view);
    CHECK(ret == XUI_OK);
    verify(&cases[consumer == 1 ? 7 : 0], XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP, color, 0, 0);
    if (consumer == 2) {
        phase = "title display failure"; displayFail = 1; drawCount = ownedAllocs = 0;
        toast.arrActive[0].sTitle = "ab" SHY "cd"; toast.arrActive[0].sMessage = NULL;
        toast.arrActive[0].tTitleRect = (xui_rect_t){5, 7, 30, 60};
        CHECK(__xuiToastRender(NULL, draw, 0, &view) == XUI_OK);
        CHECK(drawCount == 1 && strcmp(draws[0].text, "ab-") == 0 && liveLayouts == 0 && ownedAllocs == 0);
        displayFail = -1; drawCount = ownedAllocs = 0;
        CHECK(__xuiToastRender(NULL, draw, 0, &view) == XUI_OK);
        verify(&cases[0], XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP, color, 1, 0);
    }
}
static void scale_case(xui_context c, xui_draw_context draw, xui_font font, int consumer)
{
    char raw[256];
    size_t n = 0;
    xui_widget w = NULL;
    xui_label_desc_t ld;
    xui_hyperlink_desc_t hd;
    struct xui_msgbox_t box;
    struct xui_msgtip_t tip;
    struct xui_toast_t toast;
    xui_toast_item_view_t view;
    xui_rect_t rect = {5, 7, consumer == 3 ? 20 : 30, 1500};
    uint32_t color = XUI_COLOR_RGBA(23, 47, 91, 255);
    uint32_t flags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_CLIP | (consumer == 5 ? XUI_TEXT_ALIGN_MIDDLE : XUI_TEXT_ALIGN_TOP);
    int i, ret;
    for (i = 0; i < 32; ++i) {
        memcpy(raw + n, "ab" SHY "cd", 6); n += 6;
        if (i != 31) raw[n++] = '\n';
    }
    raw[n] = 0;
    phase = "64 display lines";
    if (consumer == 0) {
        memset(&ld, 0, sizeof(ld)); ld.iSize = sizeof(ld); ld.pFont = font; ld.sText = raw;
        ld.iWrapMode = XUI_TEXT_WRAP_WORD; ld.iTextColor = color; ld.iTextFlags = flags; ld.fLineGap = 2;
        CHECK(xuiLabelCreate(c, &w, &ld) == XUI_OK);
    } else if (consumer == 1) {
        memset(&hd, 0, sizeof(hd)); hd.iSize = sizeof(hd); hd.pFont = font; hd.sText = raw;
        hd.iWrapMode = XUI_TEXT_WRAP_WORD; hd.iTextColor = color; hd.iTextFlags = flags; hd.fLineGap = 2;
        CHECK(xuiHyperlinkCreate(c, &w, &hd) == XUI_OK);
    }
    if (w != NULL) {
        CHECK(xuiWidgetSetPadding(w, (xui_thickness_t){5, 7, 3, 4}) == XUI_OK);
        CHECK(xuiWidgetSetRect(w, (xui_rect_t){0, 0, rect.fW + 8, 1511}) == XUI_OK);
    }
    memset(&box, 0, sizeof(box)); box.iMagic = XUI_MSGBOX_MAGIC; box.pContext = c;
    box.pFont = font; box.sMessage = raw; box.sTitle = raw; box.tMessageRect = rect; box.tColors.iMessageColor = color;
    memset(&tip, 0, sizeof(tip)); tip.iMagic = XUI_MSGTIP_MAGIC; tip.pContext = c;
    tip.pFont = font; tip.sText = raw; tip.tTextRect = rect; tip.tColors.iTextColor = color;
    memset(&toast, 0, sizeof(toast)); toast.iMagic = XUI_TOAST_MAGIC; toast.pContext = c;
    toast.pFont = font; toast.iActiveCount = 1; toast.tColors.iTextColor = toast.tColors.iMutedTextColor = color;
    if (consumer == 5) { toast.arrActive[0].sTitle = raw; toast.arrActive[0].tTitleRect = rect; }
    else { toast.arrActive[0].sMessage = raw; toast.arrActive[0].tMessageRect = rect; }
    view.pToast = &toast; view.iSlot = 0;
    drawCount = ownedAllocs = 0;
    if (consumer < 2) ret = render_widget(w, draw, consumer, 0);
    else if (consumer == 2) ret = __xuiMsgBoxDrawTextLayout(&box, draw);
    else if (consumer == 3) ret = __xuiMsgTipDrawTextLayout(&tip, draw);
    else ret = __xuiToastRender(NULL, draw, 0, &view);
    CHECK(ret == XUI_OK && drawCount == 64);
    for (i = 0; i < drawCount && i < 64; ++i) {
        CHECK(strcmp(draws[i].text, i % 2 ? "cd" : (consumer == 3 ? "ab" : "ab-")) == 0);
        CHECK(draws[i].color == color && draws[i].flags == flags);
        CHECK(draws[i].rect.fX == 5 && draws[i].rect.fW == rect.fW && draws[i].rect.fH == 20);
        /* Soft wraps use line gap 2; hard newlines use paragraph gap 0. */
        CHECK(draws[i].rect.fY == 7 + (i / 2) * 42 + (i % 2) * 22 + (consumer == 5 ? 78 : 0));
    }
    CHECK(ownedAllocs == 0 && liveLayouts == 0);
    CHECK(strcmp(xuiMsgBoxGetTitle(&box), raw) == 0 && raw[n] == 0 && strlen(raw) == n);
    ++scaleRuns; scaleAllocations += ownedAllocs;
    if (w != NULL) xuiWidgetDestroy(w);
}
int main(void)
{
    xui_test_proxy_state_t p;
    xui_context c = NULL;
    xui_font font = NULL;
    xui_surface target = NULL;
    xui_draw_context draw = NULL;
    int backend, consumer;
    for (backend = 0; backend < 2; ++backend) {
        xuiTestProxyInit(&p); p.tProxy.drawText = capture;
        if (backend) p.tProxy.textShape = NULL;
        CHECK(xuiCreate(&c) == XUI_OK && xuiSetProxy(c, &p.tProxy) == XUI_OK);
        CHECK(p.tProxy.fontLoadMemory(&p.tProxy, &font, "body", 4, 20, XUI_FONT_FORMAT_TTF) == XUI_OK);
        CHECK(xuiTestSurfaceCreate(&p, &target, 256, 256, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
        CHECK(p.tProxy.drawBegin(&p.tProxy, &draw, target) == XUI_OK);
        widget_cases(c, draw, font, 0); widget_cases(c, draw, font, 1);
        for (consumer = 0; consumer < 3; ++consumer) message_cases(c, draw, font, consumer);
        for (consumer = 0; consumer < 6; ++consumer) scale_case(c, draw, font, consumer);
        CHECK(p.tProxy.drawEnd(&p.tProxy, draw) == XUI_OK);
        xuiDestroy(c); p.tProxy.surfaceDestroy(&p.tProxy, target); p.tProxy.fontDestroy(&p.tProxy, font);
    }
    printf("xui_text_consumer_display_test: %d checks, %d failures; both proxy shaping paths\n", checks, failures);
    printf("64-line runs: %d, consumer-owned drawing allocations: %d\n", scaleRuns, scaleAllocations);
    return failures != 0;
}
