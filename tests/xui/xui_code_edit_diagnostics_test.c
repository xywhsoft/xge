#ifdef XGE_XRT_HOST
#define XRT_MODULE_ALL
#ifdef _WIN32
#define XRT_USE_SHARED
#endif
#endif
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <math.h>
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

static xui_widget activeEditor;
static int (*originalMeasure)(xui_proxy, const xui_text_item_t*, xui_vec2_t*);
static bool destroyOnMeasure;
static bool destroyed;

static int measure(xui_proxy proxy, const xui_text_item_t* item, xui_vec2_t* size)
{
    if (destroyOnMeasure) {
        destroyOnMeasure = false;
        assert(xuiWidgetDestroyChecked(activeEditor) == XUI_OK);
        destroyed = true;
    }
    return originalMeasure(proxy, item, size);
}

static void checkDamageRollback(xui_context context, uint32_t version)
{
    assert(xuiInputViewport(context, 640, 480) == XUI_OK);
    xuiClearDamage(context);
    int count = 0;
    for (; count < 256; ++count) {
        assert(xrtMemDebugFailAfter(0));
        int status = xuiInvalidateRect(context, (xui_rect_i_t){200 + count%16*3, 200 + count/16*3, 1, 1});
        xrtMemDebugFailClear();
        if (status == XUI_ERROR_OUT_OF_MEMORY) break;
        assert(status == XUI_OK);
    }
    assert(count > 0 && count < 256 && xuiGetDamageRects(context, NULL, 0) == count);
    xui_rect_t editorRect = xuiWidgetGetWorldRect(activeEditor);
    assert(editorRect.fX + editorRect.fW < 200.0f && editorRect.fY + editorRect.fH < 200.0f);

    xui_code_annotation_store sample = NULL;
    xui_code_diagnostic_t candidate = {.tRange={23,26}, .iSeverity=XUI_CODE_DIAGNOSTIC_ERROR};
    xmemdebugsnapshot before, after;
    assert(xuiCodeAnnotationStoreCreate(&sample) == XUI_OK);
    xrtMemDebugSnapshot(&before);
    assert(xuiCodeAnnotationSetDiagnostics(sample, &candidate, 1) == XUI_OK);
    xrtMemDebugSnapshot(&after);
    xuiCodeAnnotationStoreDestroy(sample);
    size_t budget = (size_t)(after.AllocCount - before.AllocCount + after.ReallocCount - before.ReallocCount);
    assert(budget > 0 && xrtMemDebugFailAfter(budget));
    assert(xuiCodeEditApplyDiagnostics(activeEditor, version, &candidate, 1) == XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();
    assert(xuiCodeEditGetDiagnosticCount(activeEditor) == 40 && xuiGetDamageRects(context, NULL, 0) == count);
    xuiClearDamage(context);
    assert(xuiInputViewport(context, 112, 140) == XUI_OK);
}

int main(void)
{
#ifdef XGE_XRT_HOST
    const xge_host_xrt_contract_t binding = XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&binding, resolve, NULL, NULL) == XGE_HOST_XRT_OK);
#endif
    xui_context context = NULL;
    xui_font font = NULL;
    xui_test_proxy_state_t state;
    xuiTestProxyInit(&state);
    originalMeasure = state.tProxy.textMeasure;
    assert(originalMeasure != NULL);
    state.tProxy.textMeasure = measure;
    xui_context_desc_t configuration;
    xuiContextDescDefault(&configuration);
    configuration.iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED;
    assert(xuiCreateEx(&configuration, &context) == XUI_OK);
    assert(xuiSetProxy(context, &state.tProxy) == XUI_OK);
    assert(state.tProxy.fontLoadFile(&state.tProxy, &font, "diagnostics.ttf", 14, 0) == XUI_OK);
    assert(xuiSetDefaultFont(context, font) == XUI_OK);
    xui_widget root = NULL;
    assert(xuiWidgetCreate(context, &root) == XUI_OK && xuiSetRootWidget(context, root) == XUI_OK);
    xui_code_edit_desc_t desc = {.iSize=sizeof(desc), .sText="one two three four five six seven eight\n"};
    assert(xuiCodeEditCreate(context, &activeEditor, &desc) == XUI_OK);
    assert(xuiWidgetAddChild(root, activeEditor) == XUI_OK);
    assert(xuiInputViewport(context, 112, 140) == XUI_OK);
    assert(xuiWidgetSetRect(activeEditor, (xui_rect_t){0,0,112,140}) == XUI_OK);
    assert(xuiCodeEditSetDefaultMargins(activeEditor, 0, 0, 0, 0) == XUI_OK);
    assert(xuiCodeEditSetWordWrap(activeEditor, 1) == XUI_OK);

    uint32_t version;
    assert(xuiCodeEditGetDocumentVersion(activeEditor, &version) == XUI_OK);
    xui_code_diagnostic_hit_t hit = {0};
    assert(xuiCodeEditFindDiagnostic(activeEditor, NAN, 10, &hit) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiCodeEditFindDiagnostic(activeEditor, 30, INFINITY, &hit) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiCodeEditFindDiagnostic(activeEditor, 5, 5, &hit) == XUI_OK && hit.iIndex == -1);
    assert(xuiCodeEditHitTestDiagnostic(activeEditor, 5, 5, &hit) == XUI_ERROR_UNSUPPORTED && !hit.iSize);

    xui_code_diagnostic_t diagnostics[40];
    for (int i = 0; i < 40; ++i) {
        diagnostics[i] = (xui_code_diagnostic_t){.tRange={23,26}, .iSeverity=XUI_CODE_DIAGNOSTIC_WARNING,
            .sCode="W", .sMessage="old", .iUserData=(uintptr_t)i};
    }
    diagnostics[39].iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
    diagnostics[39].sCode = "";
    assert(xuiCodeEditApplyDiagnostics(activeEditor, version, diagnostics, 40) == XUI_OK);
    assert(xuiCodeEditGetDiagnosticCount(activeEditor) == 40);
    assert(xuiLayout(context) == XUI_OK);
    xui_rect_t first, wrapped;
    assert(xuiCodeEditGetOffsetRect(activeEditor, 0, &first) == XUI_OK);
    assert(xuiCodeEditGetOffsetRect(activeEditor, 23, &wrapped) == XUI_OK && wrapped.fY > first.fY);
    float x = wrapped.fX + 1.0f;
    float y = wrapped.fY + 1.0f;
    assert(xuiCodeEditFindDiagnostic(activeEditor, x, y, &hit) == XUI_OK);
    assert(hit.iIndex == 39 && hit.tRect.fY == wrapped.fY && hit.tDiagnostic.iUserData == 39);
    assert(hit.tDiagnostic.sCode && !*hit.tDiagnostic.sCode && !hit.tDiagnostic.sSource);
    assert(xuiCodeEditHitTestDiagnostic(activeEditor, x, y, &hit) == XUI_OK && hit.iIndex == 39);

    diagnostics[39].sMessage = "new";
    assert(xuiCodeEditApplyDiagnostics(activeEditor, version + 1, diagnostics, 40) == XUI_ERROR_INVALID_STATE);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    assert(xrtMemDebugFailAfter(0));
    assert(xuiCodeEditApplyDiagnostics(activeEditor, version, diagnostics, 40) == XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();
    xui_code_diagnostic_t current = {0};
    assert(xuiCodeEditGetDiagnostic(activeEditor, 39, &current) == XUI_OK && !strcmp(current.sMessage, "old"));
    checkDamageRollback(context, version);

    assert(xuiCodeEditSetText(activeEditor, "one two three four five six even longer content\n") == XUI_OK);
    assert(xuiCodeEditGetDocumentVersion(activeEditor, &version) == XUI_OK);
    assert(xuiCodeEditApplyDiagnostics(activeEditor, version, diagnostics, 40) == XUI_OK);
    destroyOnMeasure = true;
    int status = xuiCodeEditFindDiagnostic(activeEditor, x, y, &hit);
    assert(destroyed && status != XUI_OK && hit.iIndex == -1);
    xuiDestroy(context);
    state.tProxy.fontDestroy(&state.tProxy, font);
    puts("code_edit_diagnostics passed");
    return 0;
}
