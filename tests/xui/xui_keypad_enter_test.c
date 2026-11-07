#include "xge.h"
#include "xui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>

/* Exercise the production XGE queue -> XUI adapter. Direct xuiInputKeyDown
 * calls would bypass the mapping defect this test must detect. */
static int checks;
#define CHECK(e) do { ++checks; if (!(e)) { \
    fprintf(stderr, "keypad_enter:%d: %s\n", __LINE__, #e); failed = 1; goto cleanup; \
} } while (0)

typedef struct key_log {
    int down, up, key, commits;
    uint32_t modifiers;
} key_log;

static int observe(xui_widget widget, const xui_event_t* event, void* user)
{
    key_log* log = user;
    (void)widget;
    if (event->iPhase == XUI_EVENT_PHASE_TARGET) {
        if (event->iType == XUI_EVENT_KEY_DOWN) ++log->down;
        if (event->iType == XUI_EVENT_KEY_UP) ++log->up;
        if (event->iType == XUI_EVENT_KEY_DOWN || event->iType == XUI_EVENT_KEY_UP) {
            log->key = event->iKey;
            log->modifiers = event->iModifiers;
        }
    }
    return XUI_OK;
}

static int editEvent(xui_widget widget, const xui_edit_event_t* event, void* user)
{
    (void)widget;
    if (event->iType == XUI_EDIT_EVENT_COMMIT) ++((key_log*)user)->commits;
    return XUI_OK;
}

static int key(xui_context context, int code, uint32_t modifiers, int repeat)
{
    xge_input_event_t event = {0};
    event.iSize = sizeof(event);
    event.iFlags = XGE_INPUT_EVENT_FLAG_SYNTHETIC |
                   (repeat ? XGE_INPUT_EVENT_FLAG_REPEAT : 0u);
    event.iType = XGE_EVENT_KEY_DOWN;
    event.iKey = code;
    event.iModifiers = modifiers;
    int status = xgeInputEventPost(&event);
    event.iType = XGE_EVENT_KEY_UP;
    event.iFlags = XGE_INPUT_EVENT_FLAG_SYNTHETIC;
    if (status == XGE_OK) status = xgeInputEventPost(&event);
    if (status == XGE_OK)
        status = xuiProxyXgePumpInputRect(context, (xui_rect_t){0, 0, 800, 600});
    return status;
}

int main(void)
{
    int failed = 0, initialized = 0;
    xui_context context = NULL;
    xui_widget root = NULL, probe = NULL, input = NULL, editor = NULL;
    xui_test_proxy_state_t proxy;
    key_log log = {0};
    xge_desc_t engine = {0};
    engine.iRunMode = XGE_RUN_MANUAL;
    CHECK(xgeInit(&engine) == XGE_OK);
    initialized = 1;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(xuiSetViewportSize(context, 800, 600) == XUI_OK);
    CHECK(xuiWidgetCreate(context, &root) == XUI_OK);
    CHECK(xuiSetRootWidget(context, root) == XUI_OK);
    CHECK(xuiWidgetCreate(context, &probe) == XUI_OK);
    CHECK(xuiWidgetAddChild(root, probe) == XUI_OK);
    CHECK(xuiWidgetSetFocusable(probe, 1) == XUI_OK);
    CHECK(xuiWidgetSetEventCallback(probe, observe, &log) == XUI_OK);
    xui_input_desc_t desc = {0};
    desc.iSize = sizeof(desc);
    desc.sText = "value";
    CHECK(xuiInputCreate(context, &input, &desc) == XUI_OK);
    CHECK(xuiWidgetAddChild(root, input) == XUI_OK);
    CHECK(xuiWidgetSetRect(input, (xui_rect_t){0, 0, 400, 40}) == XUI_OK);
    CHECK(xuiEditSetEvent(input, editEvent, &log) == XUI_OK);
    xui_edit_behavior_t behavior = {0};
    behavior.iSize = sizeof(behavior);
    behavior.iEnterBehavior = XUI_EDIT_ENTER_COMMIT;
    CHECK(xuiEditSetBehavior(input, &behavior) == XUI_OK);
    xui_code_edit_desc_t code = {0};
    code.iSize = sizeof(code);
    CHECK(xuiCodeEditCreate(context, &editor, &code) == XUI_OK);
    CHECK(xuiWidgetAddChild(root, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 50, 700, 500}) == XUI_OK);
    CHECK(xuiLayout(context) == XUI_OK);

    const int keys[] = {XGE_KEY_ENTER, XGE_KEY_KP_ENTER};
    for (unsigned i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
        memset(&log, 0, sizeof(log));
        CHECK(xuiSetFocusWidget(context, probe) == XUI_OK);
        CHECK(key(context, keys[i], 0, 0) == XUI_OK);
        printf("ENTER,%d,down=%d,up=%d,mapped=%d,commits=%d\n",
               keys[i], log.down, log.up, log.key, log.commits);
        CHECK(log.down == 1 && log.up == 1 && log.key == XUI_KEY_ENTER);
        memset(&log, 0, sizeof(log));
        CHECK(key(context, keys[i], XGE_KEY_MOD_SHIFT, 0) == XUI_OK);
        CHECK(log.down == 1 && log.up == 1 && log.key == XUI_KEY_ENTER);
        CHECK(log.modifiers == XUI_MOD_SHIFT);

        CHECK(xuiSetFocusWidget(context, input) == XUI_OK);
        CHECK(key(context, keys[i], 0, 0) == XUI_OK);
        CHECK(log.commits == 1 && !strcmp(xuiInputGetText(input), "value"));
        CHECK(xgeInputKeyConsumed(keys[i]));

        CHECK(xuiSetFocusWidget(context, editor) == XUI_OK);
        CHECK(xuiCodeEditSetText(editor, "") == XUI_OK);
        CHECK(key(context, keys[i], 0, 0) == XUI_OK);
        CHECK(!strcmp(xuiCodeEditGetText(editor), "\n"));
        CHECK(key(context, keys[i], 0, 1) == XUI_OK);
        CHECK(!strcmp(xuiCodeEditGetText(editor), "\n\n"));
        CHECK(xgeInputKeyConsumed(keys[i]));
        CHECK(xuiCodeEditSetReadonly(editor, 1) == XUI_OK);
        CHECK(key(context, keys[i], 0, 0) == XUI_OK);
        CHECK(!strcmp(xuiCodeEditGetText(editor), "\n\n"));
        CHECK(xuiCodeEditSetReadonly(editor, 0) == XUI_OK);
    }
    CHECK(xgeInputEventPendingCount() == 0);
cleanup:
    if (context) xuiDestroy(context);
    if (initialized) xgeUnit();
    if (!failed) printf("keypad_enter passed: %d checks; main/keypad Enter, down/up, modifiers, "
                        "commit, newline, repeat and original-key consumption\n", checks);
    return failed;
}
