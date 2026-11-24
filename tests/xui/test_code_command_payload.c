#include "xui.h"
#include <assert.h>
#include <stdio.h>

typedef struct {
    xui_code_provider_set providers;
    xvalue* expected;
    int nativePayload;
    size_t owners;
    unsigned valueCalls;
    unsigned nativeCalls;
    unsigned scalarCalls;
    bool clearOnCall;
} command_state;

static bool hold(const void* raw)
{
    ++((command_state*)raw)->owners;
    return true;
}

static bool drop(const void* raw)
{
    command_state* state = (command_state*)raw;
    assert(state->owners > 1);
    --state->owners;
    return true;
}

static bool count(const void* raw, size_t* count)
{
    *count = ((const command_state*)raw)->owners;
    return true;
}

static bool trace(const void* raw, xrtownershipvisitor visit, void* user)
{
    (void)raw; (void)visit; (void)user;
    return true;
}

static int value_command(xui_widget widget, int command, const xui_code_command_input_t* input,
    int* handled, void* user)
{
    (void)widget;
    command_state* state = user;
    assert(command == 1 && state->owners == 2);
    assert(xuiCodeCommandInputGetValue(input) == state->expected);
    ++state->valueCalls;
    if (state->clearOnCall) {
        assert(xuiCodeProviderSetClearChecked(state->providers) == XUI_OK);
        assert(state->owners == 2);
        assert(xuiCodeCommandInputGetValue(input) == state->expected);
    }
    *handled = 1;
    return XUI_OK;
}

static int raw_command(xui_widget widget, int command, const void* data, int* handled, void* user)
{
    (void)widget;
    command_state* state = user;
    assert(command == 1 && data == &state->nativePayload);
    ++state->nativeCalls;
    *handled = 1;
    return XUI_OK;
}

static int scalar_command(xui_widget widget, int command, int* handled, void* user)
{
    (void)widget;
    assert(command == 1);
    ++((command_state*)user)->scalarCalls;
    *handled = 1;
    return XUI_OK;
}

int main(void)
{
    assert(xrtMemDebugEnable(true));
    command_state state = {.owners = 1};
    xui_code_provider_set* providers = &state.providers;
    assert(xuiCodeProviderSetCreate(providers) == XUI_OK);
    xvalue* payload = xrtValueInt(42);
    assert(payload);
    const xrtownershipops ops = {count, trace};
    const xui_callback_owner_t owner = {{&state, &ops}, hold, drop};
    int handled = 0;

    assert(xuiCodeProviderSetCommand(*providers, raw_command, &state) == XUI_OK);
    assert(xuiCodeProviderExecuteCommand(*providers, NULL, 1, &state.nativePayload, &handled) == XUI_OK);
    assert(handled == 1 && state.nativeCalls == 1);
    assert(xuiCodeProviderExecuteCommandValue(*providers, NULL, 1, payload, &handled) == XUI_ERROR_UNSUPPORTED);
    assert(handled == 0 && state.nativeCalls == 1);
    assert(xuiCodeProviderSetCommandOwned(*providers, raw_command, &state, &owner) == XUI_OK);
    assert(state.owners == 2);
    assert(xuiCodeProviderExecuteCommand(*providers, NULL, 1, &state.nativePayload, &handled) == XUI_OK);
    assert(handled == 1 && state.nativeCalls == 2);

    assert(xuiCodeProviderSetCommandHandlerOwned(*providers, scalar_command, &state, &owner) == XUI_OK);
    assert(xuiCodeProviderExecuteCommand(*providers, NULL, 1, &state.nativePayload, &handled) == XUI_OK);
    assert(handled == 1 && state.scalarCalls == 1);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiCodeProviderSetValueCommandOwned(*providers, value_command, &state, &owner) == XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();
    xrtClearError();
    assert(xuiCodeProviderExecuteCommand(*providers, NULL, 1, NULL, &handled) == XUI_OK && state.scalarCalls == 2);

    assert(xuiCodeProviderSetValueCommandOwned(*providers, value_command, &state, &owner) == XUI_OK);
    assert(xuiCodeProviderExecuteCommand(*providers, NULL, 1, &state.nativePayload, &handled) == XUI_ERROR_UNSUPPORTED);
    assert(handled == 0 && state.valueCalls == 0);
    assert(xuiCodeProviderExecuteCommand(*providers, NULL, 1, NULL, &handled) == XUI_OK && handled == 1);
    state.expected = payload;
    assert(xrtMemDebugFailAfter(0));
    for (unsigned i = 0; i < 1000; ++i)
        assert(xuiCodeProviderExecuteCommandValue(*providers, NULL, 1, payload, &handled) == XUI_OK && handled == 1);
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    assert(state.valueCalls == 1001);
    state.clearOnCall = true;
    assert(xuiCodeProviderExecuteCommandValue(*providers, NULL, 1, payload, &handled) == XUI_OK);
    assert(handled == 1 && state.valueCalls == 1002 && state.owners == 1);
    assert(xuiCodeProviderSetDestroyChecked(*providers) == XUI_OK);
    xrtValueRelease(payload);
    puts("XUI_CODE_COMMAND_PAYLOAD_NATIVE_OK");
    return 0;
}
