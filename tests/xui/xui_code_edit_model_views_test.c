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

typedef struct models {
    xui_code_document document;
    xui_code_selection_model selection;
} models;

static void notified(void* user)
{
    ++*(int*)user;
}

static void changed(xui_code_document document, xui_code_range_t range, uint32_t version, void* user)
{
    assert(version == xuiCodeDocumentGetVersion(document));
    assert(range.iStart <= range.iEnd);
    notified(user);
}

static int other_thread(void* user)
{
    models* source = user;
    xui_lifetime_link_t document = {0}, selection = {0};
    int calls = 0;
    assert(xuiCodeDocumentEditorLifetimeWatch(source->document, &document, notified, &calls) == XUI_ERROR_BUSY);
    assert(xuiCodeSelectionEditorLifetimeWatch(source->selection, &selection, notified, &calls) == XUI_ERROR_BUSY);
    assert(xuiCodeSelectionSetRange(source->selection, source->document, 0, 1) == XUI_ERROR_BUSY);
    assert(calls == 0);
    return 0;
}

int main(void)
{
#ifdef XGE_XRT_HOST
    const xge_host_xrt_contract_t binding = XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&binding, resolve, NULL, NULL) == XGE_HOST_XRT_OK);
#endif
    xui_test_proxy_state_t proxy;
    xuiTestProxyInit(&proxy);
    xui_context_desc_t desc;
    xuiContextDescDefault(&desc);
    desc.iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED;
    xui_context context = NULL;
    assert(xuiCreateEx(&desc, &context) == XUI_OK);
    assert(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    xui_widget editor = NULL, other = NULL;
    assert(xuiCodeEditCreate(context, &editor, NULL) == XUI_OK);
    assert(xuiCodeEditCreate(context, &other, NULL) == XUI_OK);
    assert(xuiSetRootWidget(context, editor) == XUI_OK);
    assert(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 360, 180}) == XUI_OK);
    assert(xuiInputViewport(context, 360, 180) == XUI_OK);
    models source = {xuiCodeEditGetDocument(editor), xuiCodeEditGetSelection(editor)};
    xui_code_document other_document = xuiCodeEditGetDocument(other);
    assert(source.document && source.selection && other_document);
    assert(xuiCodeDocumentSetText(source.document, "aabcd") == XUI_OK);
    assert(xuiCodeDocumentSetText(other_document, "other") == XUI_OK);
    assert(xuiCodeDocumentDestroyChecked(source.document) == XUI_ERROR_INVALID_STATE);
    assert(xuiCodeSelectionDestroyChecked(source.selection) == XUI_ERROR_INVALID_STATE);

    xthread* thread = xrtThreadCreate(other_thread, &source, 0);
    assert(thread && xrtThreadWait(thread) == XWAIT_OK);
    xrtThreadDestroy(thread);
    xui_lifetime_link_t document = {0}, selection = {0}, detached = {0};
    int document_invalidated = 0, selection_invalidated = 0, detached_invalidated = 0;
    assert(xuiCodeDocumentEditorLifetimeWatch(source.document, &document, notified, &document_invalidated) == XUI_OK);
    assert(xuiCodeSelectionEditorLifetimeWatch(source.selection, &selection, notified, &selection_invalidated) == XUI_OK);
    assert(xuiCodeSelectionEditorLifetimeWatch(source.selection, &detached, notified, &detached_invalidated) == XUI_OK);
    xuiLifetimeUnwatch(&detached);

    uint32_t paint = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
    xuiWidgetClearDirty(editor, paint);
    assert(xuiCodeSelectionSetRange(source.selection, source.document, 1, 4) == XUI_OK);
    assert((xuiWidgetGetDirtyFlags(editor) & paint) == paint);
    assert(xuiCodeSelectionSetRange(source.selection, other_document, 0, 3) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiCodeEditingInsertText(other_document, source.selection, "reject", 0) == XUI_ERROR_INVALID_ARGUMENT);
    assert(strcmp(xuiCodeDocumentGetText(other_document), "other") == 0);

    xui_code_selection_t extra = {0};
    extra.iAnchorOffset = 100;
    extra.iCaretOffset = 101;
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    assert(xrtMemDebugFailAfter(0));
    assert(xuiCodeSelectionAdd(source.selection, &extra) == XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();
    xrtClearError();
    assert(xuiCodeSelectionGetCount(source.selection) == 1);
    assert(xuiCodeSelectionAdd(source.selection, &extra) == XUI_OK);
    assert(xuiCodeSelectionGetAt(source.selection, 1, &extra) == XUI_OK && extra.iCaretOffset == 5);
    assert(xuiCodeDocumentDelete(source.document, 1, 5) == XUI_OK);
    assert(xuiCodeSelectionGetAt(source.selection, 1, &extra) == XUI_OK && extra.iCaretOffset == 1);

    int calls = 0;
    assert(xuiCodeEditSetDocumentChangeCallbackOwned(editor, changed, &calls, NULL) == XUI_OK);
    assert(xuiCodeDocumentInsert(source.document, 1, "B") == XUI_OK && calls == 1);
    assert(xuiCodeEditSetDocumentChangeCallbackOwned(editor, NULL, NULL, NULL) == XUI_OK);
    assert(xuiCodeDocumentInsert(source.document, 2, "C") == XUI_OK && calls == 1);

    xui_call_scope_t scope = {0};
    assert(xuiCallScopeBegin(context, &scope) == XUI_OK);
    assert(xuiWidgetDestroyChecked(editor) == XUI_OK);
    assert(document_invalidated == 1 && selection_invalidated == 1 && detached_invalidated == 0);
    assert(xuiCodeSelectionSetRange(source.selection, source.document, 0, 1) == XUI_ERROR_INVALID_STATE);
    assert(xuiCallScopeEnd(&scope) == XUI_OK);
    assert(xuiCodeDocumentGetLength(other_document) == 5);
    xuiDestroy(context);
    assert(document_invalidated == 1 && selection_invalidated == 1);
    puts("code_edit_model_views passed");
    return 0;
}
