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

static void invalidate(void* user)
{
    ++*(int*)user;
}

static int wrong_thread(void* user)
{
    xui_code_language_registry registry = user;
    xui_lifetime_link_t link = {0};
    int calls = 0;
    assert(xuiCodeLanguageRegistryGetCount(registry) == XUI_ERROR_BUSY);
    assert(xuiCodeLanguageRegistryClearChecked(registry) == XUI_ERROR_BUSY);
    assert(xuiCodeLanguageRegistryEditorLifetimeWatch(registry, &link, invalidate, &calls) == XUI_ERROR_BUSY);
    assert(calls == 0);
    return 0;
}

static int guarded_lex(xui_code_document doc, int first, int last, xui_code_token_t* tokens,
    int capacity, int* count, void* user)
{
    (void)doc;
    (void)first;
    (void)last;
    (void)tokens;
    (void)capacity;
    xui_code_language_registry registry = user;
    assert(xuiCodeLanguageRegistryGetCount(registry) == 2);
    assert(xuiCodeLanguageRegistryClearChecked(registry) == XUI_ERROR_BUSY);
    *count = 0;
    return XUI_OK;
}

static int close_editor_lex(xui_code_document document, int first, int last,
    xui_code_token_t* tokens, int capacity, int* count, void* user)
{
    (void)document;
    (void)first;
    (void)last;
    (void)tokens;
    (void)capacity;
    *count = 0;
    return xuiWidgetDestroyChecked(user);
}

static void require_first_token(xui_context context, xui_widget editor, xui_surface target, int style)
{
    assert(xuiLayout(context) == XUI_OK);
    assert(xuiUpdate(context, 0.016f) == XUI_OK);
    assert(xuiRender(context, target, NULL, 0) == XUI_OK);
    xui_code_token_t token;
    int count = 0;
    uint32_t version = xuiCodeDocumentGetVersion(xuiCodeEditGetDocument(editor));
    assert(xuiCodeTokenBufferGetTokens(xuiCodeEditGetTokenBuffer(editor), version, &token, 1, &count) == XUI_OK);
    assert(count > 0 && token.iStartOffset == 0 && token.iEndOffset == 3 && token.iStyle == style);
}

static void check_rollback(xui_code_language_registry registry, xui_code_regex_language_t* language)
{
    int failures = 0;
    language->sName = "Replacement";
    for ( int allowance = 0; allowance < 32; ++allowance ) {
        xmemdebugsnapshot before, after;
        xrtMemDebugSnapshot(&before);
        assert(xrtMemDebugFailAfter(allowance));
        int status = xuiCodeLanguageRegistryRegisterRegex(registry, language);
        xrtMemDebugFailClear();
        xrtClearError();
        if ( status == XUI_OK ) break;
        assert(status == XUI_ERROR_OUT_OF_MEMORY);
        ++failures;
        xrtMemDebugSnapshot(&after);
        assert(after.LiveCount == before.LiveCount && after.LiveBytes == before.LiveBytes);
        xui_code_language_t value;
        assert(xuiCodeLanguageRegistryFind(registry, "NOTES", &value) == XUI_OK);
        assert(!strcmp(value.sName, "Notes") && value.iRegexRuleCount == 2);
    }
    assert(failures >= 10 && failures < 32);
    printf("language registry replaced only after %d allocation failures\n", failures);
}

int main(void)
{
#ifdef XGE_XRT_HOST
    const xge_host_xrt_contract_t binding = XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&binding, resolve, NULL, NULL) == XGE_HOST_XRT_OK);
#endif
    xui_test_proxy_state_t proxy;
    xuiTestProxyInit(&proxy);
    xui_context_desc_t settings;
    xuiContextDescDefault(&settings);
    settings.iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED;
    xui_context context = NULL;
    assert(xuiCreateEx(&settings, &context) == XUI_OK);
    assert(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    xui_font font = NULL;
    assert(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "registry.ttf", 14, 0) == XUI_OK);
    assert(xuiSetDefaultFont(context, font) == XUI_OK);
    xui_widget editor = NULL;
    assert(xuiCodeEditCreate(context, &editor, NULL) == XUI_OK);
    assert(xuiSetRootWidget(context, editor) == XUI_OK);
    assert(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 160}) == XUI_OK);
    assert(xuiInputViewport(context, 320, 160) == XUI_OK);
    xui_surface target = NULL;
    assert(xuiTestSurfaceCreate(&proxy, &target, 320, 160, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    assert(xuiCodeEditSetText(editor, "abc 42") == XUI_OK);
    xui_code_language_registry registry = xuiCodeEditGetLanguageRegistry(editor);
    assert(xuiCodeLanguageRegistryGetCount(registry) == 1);
    assert(xuiCodeLanguageRegistryDestroyChecked(registry) == XUI_ERROR_BUSY);

    char extension[] = ".note";
    char pattern[] = "abc";
    const xui_code_language_extension_t extensions[] = {{extension}};
    xui_code_regex_rule_t rules[] = {
        {.iSize = sizeof(rules[0]), .sName = "word", .sPattern = pattern,
            .iTokenKind = XUI_CODE_TOKEN_KEYWORD, .iStyle = XUI_CODE_STYLE_KEYWORD},
        {.iSize = sizeof(rules[0]), .sName = "number", .sPattern = "[0-9]+",
            .iTokenKind = XUI_CODE_TOKEN_NUMBER, .iStyle = XUI_CODE_STYLE_NUMBER}
    };
    xui_code_regex_language_t language = {
        .iSize = sizeof(language), .sId = "notes", .sName = "Notes", .sLineComment = "//",
        .pExtensions = extensions, .iExtensionCount = 1, .pRegexRules = rules, .iRegexRuleCount = 2
    };
    assert(xuiCodeLanguageRegistryRegisterRegex(registry, &language) == XUI_OK);
    extension[1] = 'X';
    pattern[0] = 'X';
    assert(xuiCodeEditSetLanguage(editor, "NOTES") == XUI_OK);
    const char* stored = NULL;
    assert(xuiCodeLanguageRegistryGetExtension(registry, 1, 0, &stored) == XUI_OK && !strcmp(stored, "note"));
    require_first_token(context, editor, target, XUI_CODE_STYLE_KEYWORD);

    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    extension[1] = 'n';
    pattern[0] = 'a';
    rules[0].iTokenKind = XUI_CODE_TOKEN_COMMENT;
    rules[0].iStyle = XUI_CODE_STYLE_COMMENT;
    check_rollback(registry, &language);
    uint32_t dirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
    assert((xuiWidgetGetDirtyFlags(editor) & dirty) == dirty);
    require_first_token(context, editor, target, XUI_CODE_STYLE_COMMENT);
    xuiWidgetClearDirty(editor, dirty);
    assert(xuiCodeLanguageRegistryLoadDefaults(registry) == XUI_OK);
    assert(!(xuiWidgetGetDirtyFlags(editor) & dirty));

    xui_code_language_t guard = {
        .iSize = sizeof(guard), .sId = "notes", .onLex = guarded_lex, .pUser = registry
    };
    assert(xuiCodeLanguageRegistryRegister(registry, &guard) == XUI_OK);
    assert(xuiLayout(context) == XUI_OK && xuiRender(context, target, NULL, 0) == XUI_OK);
    assert(xuiCodeLanguageRegistryClearChecked(registry) == XUI_OK);
    assert(xuiCodeLanguageRegistryGetCount(registry) == 0);

    xthread* worker = xrtThreadCreate(wrong_thread, registry, 0);
    assert(worker && xrtThreadWait(worker) == XWAIT_OK);
    xrtThreadDestroy(worker);
    xui_lifetime_link_t link = {0};
    int calls = 0;
    assert(xuiCodeLanguageRegistryEditorLifetimeWatch(registry, &link, invalidate, &calls) == XUI_OK);
    xui_call_scope_t scope = {0};
    assert(xuiCallScopeBegin(context, &scope) == XUI_OK);
    assert(xuiWidgetDestroyChecked(editor) == XUI_OK && calls == 1);
    assert(xuiCodeLanguageRegistryGetCount(registry) == XUI_ERROR_INVALID_STATE);
    assert(xuiCallScopeEnd(&scope) == XUI_OK);

    assert(xuiCodeEditCreate(context, &editor, NULL) == XUI_OK);
    assert(xuiSetRootWidget(context, editor) == XUI_OK);
    registry = xuiCodeEditGetLanguageRegistry(editor);
    xui_lifetime_link_t closeLink = {0};
    assert(xuiCodeLanguageRegistryEditorLifetimeWatch(registry, &closeLink, invalidate, &calls) == XUI_OK);
    xui_code_language_t closer = {
        .iSize = sizeof(closer), .sId = "close-editor", .onLex = close_editor_lex, .pUser = editor
    };
    assert(xuiCodeLanguageRegistryRegister(registry, &closer) == XUI_OK);
    int count = 99;
    assert(xuiCodeLanguageRegistryLex(registry, "close-editor", xuiCodeEditGetDocument(editor),
        0, -1, NULL, 0, &count, NULL, 0) == XUI_OK);
    assert(count == 0 && calls == 2);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
    xuiDestroy(context);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("code_edit_language_registry: refreshed rendered tokens, guarded mutation and lifetime passed");
    return 0;
}
