#if XGE_XRT_HOST && !defined(XL6_XGE_STATIC)
#define XRT_MODULE_ALL
#ifdef _WIN32
#define XRT_USE_SHARED
#endif
#endif
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if XGE_XRT_HOST && !defined(XL6_XGE_STATIC)
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

typedef struct owned_language_probe_t {
    xui_code_language_registry registry;
    size_t references;
    int visits;
    int calls;
    bool failDrop;
} owned_language_probe_t;

static bool owned_language_count(const void* raw, size_t* count)
{ *count = ((const owned_language_probe_t*)raw)->references; return true; }

static bool owned_language_trace(const void* raw, xrtownershipvisitor visit, void* user)
{ (void)raw; (void)visit; (void)user; return true; }

static bool owned_language_hold(const void* raw)
{ ++((owned_language_probe_t*)raw)->references; return true; }

static bool owned_language_drop(const void* raw)
{
    owned_language_probe_t* probe = (owned_language_probe_t*)raw;
    assert(probe->references != 0);
    assert(xuiCodeLanguageRegistryClearChecked(probe->registry) == XUI_ERROR_BUSY);
    --probe->references;
    return !probe->failDrop;
}

static bool visit_owned_language(xrtownershipref reference, void* user)
{
    owned_language_probe_t* probe = user;
    if ( reference.Data == probe ) ++probe->visits;
    return true;
}

static int owned_language_lex(xui_code_document document, int first, int last,
    xui_code_token_output_t* output, void* raw)
{
    (void)first;
    (void)last;
    owned_language_probe_t* probe = raw;
    ++probe->calls;
    assert(xuiCodeLanguageRegistryOwnershipBusy(probe->registry));
    assert(xuiCodeLanguageRegistryClearChecked(probe->registry) == XUI_ERROR_BUSY);
    assert(xuiCodeDocumentGetLength(document) >= 3 && xuiCodeTokenOutputGetCapacity(output) >= 0);
    xui_code_token_t token = {.iStartOffset=0, .iEndOffset=3,
        .iKind=XUI_CODE_TOKEN_KEYWORD, .iStyle=XUI_CODE_STYLE_KEYWORD};
    return xuiCodeTokenOutputAppend(output, token);
}

static int owned_language_fold(xui_code_document document, xui_code_fold_output_t* output, void* raw)
{
    (void)document;
    owned_language_probe_t* probe = raw;
    assert(xuiCodeLanguageRegistryClearChecked(probe->registry) == XUI_ERROR_BUSY);
    xui_code_fold_range_t range = {.iStartLine=0, .iEndLine=2};
    return xuiCodeFoldOutputAppend(output, range);
}

static void install_owned_language(owned_language_probe_t* probe)
{
    static const xrtownershipops ops = {owned_language_count, owned_language_trace};
    const xui_callback_owner_t owner = {{probe, &ops}, owned_language_hold, owned_language_drop};
    const xui_code_language_extension_t extensions[] = {{"owned"}};
    const xui_code_callback_language_t language = {.iSize=sizeof(language), .sId="owned-test",
        .pExtensions=extensions, .iExtensionCount=1, .onLex=owned_language_lex,
        .onFold=owned_language_fold, .pUser=probe};
    assert(xuiCodeLanguageRegistryRegisterCallbacks(probe->registry, &language, &owner) == XUI_OK);
}

typedef struct nested_language_probe_t {
    xui_code_document other;
    xui_code_language_t managed;
} nested_language_probe_t;

static int nested_legacy_lex(xui_code_document document, int first, int last,
    xui_code_token_t* tokens, int capacity, int* count, void* raw)
{
    nested_language_probe_t* probe = raw;
    int rejected = 99;
    assert(xuiCodeLanguageLex(&probe->managed,probe->other,first,last,NULL,0,&rejected,NULL,0) == XUI_ERROR_BUSY);
    assert(rejected == 0);
    return xuiCodeLanguageLex(&probe->managed,document,first,last,tokens,capacity,count,NULL,0);
}

static void check_owned_registry(void)
{
    xui_code_document document = NULL;
    owned_language_probe_t probe = {0};
    assert(xuiCodeLanguageRegistryCreate(&probe.registry) == XUI_OK);
    assert(xuiCodeDocumentCreate(&document) == XUI_OK);
    assert(xuiCodeDocumentSetText(document,"abc\nnext\nend") == XUI_OK);
    install_owned_language(&probe);
    assert(probe.references == 1);
    assert(xuiCodeLanguageRegistryTraceOwnership(probe.registry,visit_owned_language,&probe) && probe.visits == 1);
    xui_code_token_t token = {0};
    int count = 0;
    assert(xuiCodeLanguageRegistryLex(probe.registry,"owned-test",document,0,-1,&token,1,&count,NULL,0) == XUI_OK);
    assert(count == 1 && token.iEndOffset == 3);
    assert(xuiCodeLanguageRegistryFold(probe.registry,"owned-test",document,NULL,0,&count) == XUI_OK && count == 1);
    xui_code_language_t borrowed = {0};
    assert(xuiCodeLanguageRegistryFind(probe.registry,"owned-test",&borrowed) == XUI_OK);
    assert(xuiCodeLanguageLex(&borrowed,document,0,-1,NULL,0,&count,NULL,0) == XUI_OK && count == 1);
    nested_language_probe_t nested = {.managed=borrowed};
    assert(xuiCodeDocumentCreate(&nested.other) == XUI_OK);
    assert(xuiCodeDocumentSetText(nested.other,"abc") == XUI_OK);
    const xui_code_language_t legacy = {.iSize=sizeof(legacy), .sId="nested",
        .onLex=nested_legacy_lex, .pUser=&nested};
    assert(xuiCodeLanguageRegistryRegister(probe.registry,&legacy) == XUI_OK);
    assert(xuiCodeLanguageRegistryLex(probe.registry,"nested",document,0,-1,NULL,0,&count,NULL,0) == XUI_OK);
    assert(count == 1);
    xuiCodeDocumentDestroy(nested.other);
    install_owned_language(&probe);
    assert(probe.references == 1 && probe.calls == 3);
    probe.failDrop = true;
    bool consumed = false;
    assert(xuiCodeLanguageRegistryDestroyOwnerChecked(probe.registry,&consumed) == XUI_ERROR_CALLBACK_FAILED);
    assert(consumed && probe.references == 0);
    xrtClearError();
    xuiCodeDocumentDestroy(document);
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

static void check_mixed_registry(xui_context context, xui_widget editor, xui_surface target)
{
    static const xrtownershipops ops = {owned_language_count, owned_language_trace};
    owned_language_probe_t probe = {.registry=xuiCodeEditGetLanguageRegistry(editor)};
    const xui_callback_owner_t owner = {{&probe, &ops}, owned_language_hold, owned_language_drop};
    char pattern[] = "abc";
    const xui_code_regex_rule_t rule = {.iSize=sizeof(rule), .sPattern=pattern,
        .iTokenKind=XUI_CODE_TOKEN_COMMENT, .iStyle=XUI_CODE_STYLE_COMMENT};
    xui_code_callback_language_t language = {.iSize=sizeof(language), .sId="mixed",
        .onFold=owned_language_fold, .pUser=&probe, .pRegexRules=&rule, .iRegexRuleCount=1};
    assert(xuiCodeEditSetText(editor, "abc\nnext\nend") == XUI_OK);
    assert(xuiCodeLanguageRegistryRegisterRegexCallbacks(probe.registry, &language, &owner) == XUI_OK);
    pattern[0] = 'x';
    assert(xuiCodeEditSetLanguage(editor, "mixed") == XUI_OK);
    require_first_token(context, editor, target, XUI_CODE_STYLE_COMMENT);
    xui_code_document document = xuiCodeEditGetDocument(editor);
    xui_code_fold_range_t range = {0};
    int count = 0;
    assert(xuiCodeLanguageRegistryFold(probe.registry, "mixed", document, &range, 1, &count) == XUI_OK);
    assert(count == 1 && range.iStartLine == 0 && range.iEndLine == 2 && probe.references == 1);
    assert(xuiCodeLanguageRegistryTraceOwnership(probe.registry, visit_owned_language, &probe) && probe.visits == 1);

    // The custom lexer takes precedence over the regex rules on the same definition.
    language.onLex = owned_language_lex;
    language.iSize = offsetof(xui_code_callback_language_t, pRegexRules) + 1;
    assert(xuiCodeLanguageRegistryRegisterCallbacks(probe.registry, &language, &owner) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiCodeLanguageRegistryRegisterRegexCallbacks(probe.registry, &language, &owner) == XUI_ERROR_INVALID_ARGUMENT);
    language.iSize = sizeof(language);
    int failures = 0;
    for ( int allowance = 0; allowance < 32; ++allowance ) {
        xmemdebugsnapshot before, after;
        xrtMemDebugSnapshot(&before);
        assert(xrtMemDebugFailAfter(allowance));
        int status = xuiCodeLanguageRegistryRegisterRegexCallbacks(probe.registry, &language, &owner);
        xrtMemDebugFailClear();
        xrtClearError();
        if ( status == XUI_OK ) break;
        assert(status == XUI_ERROR_OUT_OF_MEMORY && probe.references == 1);
        ++failures;
        xrtMemDebugSnapshot(&after);
        assert(after.LiveCount == before.LiveCount && after.LiveBytes == before.LiveBytes);
        require_first_token(context, editor, target, XUI_CODE_STYLE_COMMENT);
    }
    assert(failures > 0 && failures < 32 && probe.references == 1);
    require_first_token(context, editor, target, XUI_CODE_STYLE_KEYWORD);
    assert(probe.calls > 0 && probe.references == 1);

    // The original descriptor length must hide the tail even if bytes follow it.
    language.iSize = offsetof(xui_code_callback_language_t, pRegexRules);
    language.iRegexRuleCount = -1;
    assert(xuiCodeLanguageRegistryRegisterCallbacks(probe.registry, &language, &owner) == XUI_OK);
    xui_code_language_t stored = {0};
    assert(xuiCodeLanguageRegistryFind(probe.registry, "mixed", &stored) == XUI_OK);
    assert(stored.iLexerType == XUI_CODE_LEXER_CALLBACK && stored.iRegexRuleCount == 0);

    language.iSize = sizeof(language);
    language.onLex = NULL;
    language.onFold = NULL;
    language.iRegexRuleCount = 0;
    assert(xuiCodeLanguageRegistryRegisterRegexCallbacks(probe.registry, &language, NULL) == XUI_ERROR_INVALID_ARGUMENT);
    language.iRegexRuleCount = 1;
    assert(xuiCodeLanguageRegistryRegisterRegexCallbacks(probe.registry, &language, &owner) == XUI_ERROR_INVALID_ARGUMENT);
    pattern[0] = 'a';
    assert(xuiCodeLanguageRegistryRegisterRegexCallbacks(probe.registry, &language, NULL) == XUI_OK);
    assert(probe.references == 0);
    require_first_token(context, editor, target, XUI_CODE_STYLE_COMMENT);
    assert(xuiCodeLanguageRegistryFold(probe.registry, "mixed", document, NULL, 0, &count) == XUI_ERROR_UNSUPPORTED);
    puts("code_edit_language_registry: mixed regex and callbacks passed");
}

int main(void)
{
#if XGE_XRT_HOST && !defined(XL6_XGE_STATIC)
    const xge_host_xrt_contract_t binding = XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&binding, resolve, NULL, NULL) == XGE_HOST_XRT_OK);
#endif
    check_owned_registry();
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

    owned_language_probe_t owned = {.registry=registry};
    install_owned_language(&owned);
    assert(xuiCodeEditSetLanguage(editor,"owned-test") == XUI_OK);
    require_first_token(context, editor, target, XUI_CODE_STYLE_KEYWORD);
    assert(owned.references == 1 && owned.calls > 0);
    check_mixed_registry(context, editor, target);
    assert(xuiCodeLanguageRegistryClearChecked(registry) == XUI_OK);
    assert(xuiCodeLanguageRegistryGetCount(registry) == 0 && owned.references == 0);

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
