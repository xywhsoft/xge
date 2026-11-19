#ifdef XGE_XRT_HOST
#define XRT_MODULE_ALL
#ifdef _WIN32
#define XRT_USE_SHARED
#endif
#endif
#include "xui.h"
#include "xui_test_proxy.h"
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

static int wrong_thread(void* raw)
{
    xui_code_theme theme = raw;
    assert(xuiCodeThemeSetDefault(theme) == XUI_ERROR_BUSY);
    return 0;
}

static void invalidated(void* user)
{
    ++*(int*)user;
}

typedef struct paint_probe_t {
    xui_code_theme theme;
    int calls;
} paint_probe_t;

static xui_draw_text_spans_proc draw_spans;
static uint32_t token_color;

static int record_spans(xui_proxy proxy,xui_draw_context draw,const xui_text_item_t* text,
    xui_rect_t rect,uint32_t color,uint32_t flags,const xui_text_paint_span_t* spans,int count)
{
    if (count > 0 && text->iTextSize == 6 && !memcmp(text->sText, "return", 6))
        token_color = spans[0].iColor;
    return draw_spans(proxy,draw,text,rect,color,flags,spans,count);
}

static int token_style(xui_code_document document, const xui_code_token_t* token, int* style, void* user)
{
    (void)document;
    (void)token;
    paint_probe_t* probe = user;
    ++probe->calls;
    assert(xuiCodeThemeSetDefault(probe->theme) == XUI_ERROR_BUSY);
    *style = XUI_CODE_STYLE_KEYWORD;
    return XUI_OK;
}

static void check_copy_and_hot_write(xui_widget editor, xui_code_theme live)
{
    const int kind = XUI_CODE_TOKEN_CUSTOM_BASE, id = XUI_CODE_STYLE_CUSTOM_BASE;
    const uint32_t dirty = XUI_WIDGET_DIRTY_STYLE | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
    xui_code_style_t base, actual;
    assert(xuiCodeThemeGetStyle(live, XUI_CODE_STYLE_KEYWORD, &base) == XUI_OK);
    xui_code_theme other = NULL;
    assert(xuiCodeThemeCreate(&other) == XUI_OK);
    assert(xuiCodeThemeSetStyle(other, id, &base) == XUI_OK);
    assert(xuiCodeThemeMapTokenKind(other, kind, id) == XUI_OK);
    for (int budget = 0; budget < 2; ++budget) {
        xuiWidgetClearDirty(editor, dirty);
        xmemdebugsnapshot before, after;
        xrtMemDebugSnapshot(&before);
        assert(xrtMemDebugFailAfter(budget));
        assert(xuiCodeThemeCopy(live, other) == XUI_ERROR_OUT_OF_MEMORY);
        xrtMemDebugFailClear();
        xrtClearError();
        xrtMemDebugSnapshot(&after);
        assert(after.LiveBytes == before.LiveBytes && after.LiveCount == before.LiveCount);
        assert(xuiCodeThemeGetStyle(live, id, &actual) == XUI_ERROR_NOT_FOUND);
        assert((xuiWidgetGetDirtyFlags(editor) & dirty) == 0);
    }
    assert(xuiCodeThemeCopy(live, other) == XUI_OK);
    assert((xuiWidgetGetDirtyFlags(editor) & dirty) == dirty);
    assert(xuiCodeThemeGetTokenStyle(live, kind, &actual) == XUI_OK && actual.iForeground == base.iForeground);
    xuiCodeThemeDestroy(other);
    for (int budget = 0; budget < 3; ++budget) {
        xui_code_theme clone = live;
        xmemdebugsnapshot before, after;
        xrtMemDebugSnapshot(&before);
        assert(xrtMemDebugFailAfter(budget));
        assert(xuiCodeThemeClone(live, &clone) == XUI_ERROR_OUT_OF_MEMORY && !clone);
        xrtMemDebugFailClear();
        xrtClearError();
        xrtMemDebugSnapshot(&after);
        assert(after.LiveBytes == before.LiveBytes && after.LiveCount == before.LiveCount);
    }
    xui_code_theme clone = NULL;
    assert(xuiCodeThemeClone(live, &clone) == XUI_OK && clone != live);
    assert(xuiCodeThemeGetTokenStyle(clone, kind, &actual) == XUI_OK && actual.iForeground == base.iForeground);
    assert(xuiCodeThemeSetDefault(clone) == XUI_OK);
    assert(xuiCodeThemeGetStyle(live, id, &actual) == XUI_OK);
    xuiCodeThemeDestroy(clone);

    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    for (int i = 0; i < 1000; ++i) {
        assert(xuiCodeThemeSetStyle(live, id, &base) == XUI_OK);
        assert(xuiCodeThemeMapTokenKind(live, kind, id) == XUI_OK);
    }
    xrtMemDebugFailClear();
    xrtMemDebugSnapshot(&after);
    assert(after.AllocCount == before.AllocCount && after.FreeCount == before.FreeCount &&
        after.ReallocCount == before.ReallocCount && after.LiveBytes == before.LiveBytes);
    puts("theme warmed writes: 1000 style/map pairs, heap 0/0/0");
}

static void check_theme(void)
{
    xui_test_proxy_state_t proxy;
    xuiTestProxyInit(&proxy);
    draw_spans = proxy.tProxy.drawTextSpans;
    proxy.tProxy.drawTextSpans = record_spans;
    xui_context_desc_t desc;
    xuiContextDescDefault(&desc);
    desc.iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED;
    xui_context context = NULL;
    assert(xuiCreateEx(&desc, &context) == XUI_OK);
    assert(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    xui_font font = NULL;
    assert(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "theme.ttf", 14, 0) == XUI_OK);
    assert(xuiSetDefaultFont(context, font) == XUI_OK);
    xui_widget editor = NULL;
    assert(xuiCodeEditCreate(context, &editor, NULL) == XUI_OK);
    assert(xuiSetRootWidget(context, editor) == XUI_OK);
    assert(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 160}) == XUI_OK);
    assert(xuiInputViewport(context, 320, 160) == XUI_OK);
    assert(xuiCodeEditSetText(editor, "return") == XUI_OK);
    assert(xuiCodeEditSetLanguage(editor, "c") == XUI_OK);
    assert(xuiCodeEditSetDefaultMargins(editor, 0, 0, 0, 0) == XUI_OK);
    xui_surface target = NULL;
    assert(xuiTestSurfaceCreate(&proxy, &target, 320, 160, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    xui_code_theme theme = xuiCodeEditGetTheme(editor);
    assert(theme);
    xuiCodeThemeDestroy(theme);
    assert(xuiCodeEditGetTheme(editor) == theme);
    check_copy_and_hot_write(editor, theme);

    xui_code_style_t style;
    assert(xuiCodeThemeGetStyle(theme, XUI_CODE_STYLE_KEYWORD, &style) == XUI_OK);
    const uint32_t color = XUI_COLOR_RGBA(19, 181, 137, 255);
    paint_probe_t probe = {theme, 0};
    assert(xuiCodeEditSetTokenStyleResolver(editor, token_style, &probe) == XUI_OK);
    assert(xuiLayout(context) == XUI_OK && xuiRender(context, target, NULL, 0) == XUI_OK);
    assert(probe.calls > 0 && token_color != 0 && token_color != color);
    style.iForeground = color;
    assert(xuiCodeThemeSetStyle(theme, XUI_CODE_STYLE_KEYWORD, &style) == XUI_OK);
    assert(xuiRender(context, target, NULL, 0) == XUI_OK);
    assert(token_color == color);

    xthread* worker = xrtThreadCreate(wrong_thread, theme, 0);
    assert(worker && xrtThreadWait(worker) == XWAIT_OK);
    xrtThreadDestroy(worker);
    xui_lifetime_link_t watch = {0}, removed = {0};
    int count = 0;
    assert(xuiCodeThemeEditorLifetimeWatch(theme, &watch, invalidated, &count) == XUI_OK);
    assert(xuiCodeThemeEditorLifetimeWatch(theme, &removed, invalidated, &count) == XUI_OK);
    xuiLifetimeUnwatch(&removed);
    xui_call_scope_t scope = {0};
    assert(xuiCallScopeBegin(context, &scope) == XUI_OK);
    assert(xuiWidgetDestroyChecked(editor) == XUI_OK && count == 1);
    assert(xuiCodeThemeSetDefault(theme) == XUI_ERROR_INVALID_STATE);
    assert(xuiCallScopeEnd(&scope) == XUI_OK);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
    xuiDestroy(context);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
}

int main(void)
{
#ifdef XGE_XRT_HOST
    const xge_host_xrt_contract_t binding = XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&binding, resolve, NULL, NULL) == XGE_HOST_XRT_OK);
#endif
    assert(xrtMemDebugEnable(true));
    check_theme();
    puts("code_edit_theme: live colors, copy rollback and observation passed");
    return 0;
}
