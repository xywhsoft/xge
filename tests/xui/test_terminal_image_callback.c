#include "xui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    xui_widget terminal;
    unsigned calls;
    size_t refs;
    bool replace;
    bool fail;
    int protocol, column, row, size;
    char options[64];
    char payload[64];
} image_capture;

static bool hold(const void* raw)
{ ++((image_capture*)raw)->refs; return true; }
static bool drop(const void* raw)
{ --((image_capture*)raw)->refs; return true; }
static bool count(const void* raw, size_t* out)
{ *out = ((const image_capture*)raw)->refs; return true; }
static bool trace(const void* raw, xrtownershipvisitor visit, void* user)
{ (void)raw; (void)visit; (void)user; return true; }
static bool match(xrtownershipref edge, void* raw)
{
    void** query = raw;
    if (edge.Data == query[0]) ++*(int*)query[1];
    return true;
}

static void on_image(xui_widget terminal, const xui_terminal_image_t* image, void* raw)
{
    image_capture* capture = raw;
    assert(terminal == capture->terminal);
    assert(image->iSize == sizeof(*image));
    assert(image->iPayloadSize == (int)strlen(image->sPayload));
    if (capture->replace) {
        assert(capture->refs == 3);
        assert(xuiTerminalSetImageCallback(terminal, NULL, NULL) == XUI_OK);
        assert(capture->refs == 2);
        capture->replace = false;
    }
    ++capture->calls;
    capture->protocol = image->iProtocol;
    capture->column = image->iColumn;
    capture->row = image->iRow;
    capture->size = image->iPayloadSize;
    snprintf(capture->options, sizeof(capture->options), "%s", image->sOptions);
    snprintf(capture->payload, sizeof(capture->payload), "%s", image->sPayload);
    if (capture->fail) xrtSetErrorKind(XERR_IO);
}

static void write_text(xui_widget terminal, const char* text)
{
    assert(xuiTerminalWriteText(terminal, text) == XUI_OK);
    assert(xuiTerminalFlush(terminal) == XUI_OK);
}

int main(void)
{
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);

    xui_context context = NULL;
    xui_proxy proxy = NULL;
    const xui_context_desc_t desc = {.iSize = sizeof(desc), .iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED};
    assert(xuiProxyXgeCreate(&proxy) == XUI_OK);
    assert(xuiCreateOwnedEx(proxy, &desc, &context) == XUI_OK);
    xui_widget terminal = NULL;
    assert(xuiTerminalCreate(context, &terminal, NULL) == XUI_OK);
    image_capture capture = {.terminal = terminal, .refs = 1};
    const xrtownershipops ops = {count, trace};
    const xui_callback_owner_t owner = {{&capture, &ops}, hold, drop};
    assert(xuiTerminalSetImageCallbackOwned(terminal, on_image, &capture, &owner) == XUI_OK);
    assert(capture.refs == 2);
    int edges = 0;
    void* query[] = {&capture, &edges};
    assert(xuiContextTraceOwnership(context, match, query) && edges == 1);

    write_text(terminal, "\033[2;4H\033]1337;File=width=2;inline=1:QU");
    assert(capture.calls == 0);
    write_text(terminal, "JD\033");
    assert(capture.calls == 0);
    write_text(terminal, "\\");
    assert(capture.calls == 1 && capture.protocol == XUI_TERMINAL_IMAGE_ITERM2);
    assert(capture.column == 3 && capture.row == 1 && capture.size == 4);
    assert(!strcmp(capture.options, "width=2;inline=1") && !strcmp(capture.payload, "QUJD"));
    write_text(terminal, "\033]1337;File=inline=0:QUJD\a");
    assert(capture.calls == 1);

    write_text(terminal, "\033P1;2;3q~");
    assert(capture.calls == 1);
    write_text(terminal, "!3~\033\\");
    assert(capture.calls == 2 && capture.protocol == XUI_TERMINAL_IMAGE_SIXEL);
    assert(!strcmp(capture.options, "1;2;3") && !strcmp(capture.payload, "~!3~"));
    write_text(terminal, "\220q~\234");
    assert(capture.calls == 3 && !strcmp(capture.options, "") && !strcmp(capture.payload, "~"));
    write_text(terminal, "\033P$q~\033\\\033Pq~\030");
    assert(capture.calls == 3);

    write_text(terminal, "\033]1337;MultipartFile=inline=1\a");
    write_text(terminal, "\033]1337;FilePart=QU\a\033]1337;FilePart=JD\a");
    assert(capture.calls == 3);
    capture.replace = true;
    write_text(terminal, "\033]1337;FileEnd\a");
    assert(capture.calls == 4 && capture.refs == 1);
    assert(capture.protocol == XUI_TERMINAL_IMAGE_ITERM2 && !strcmp(capture.payload, "QUJD"));
    write_text(terminal, "\033Pq~\033\\");
    assert(capture.calls == 4);

    assert(xuiTerminalSetImageCallback(terminal, on_image, &capture) == XUI_OK);
    char long_image[300];
    memset(long_image, '~', sizeof(long_image));
    memcpy(long_image, "\033Pq", 3);
    long_image[sizeof(long_image) - 2] = 27;
    long_image[sizeof(long_image) - 1] = '\\';
    assert(xuiTerminalWrite(terminal, long_image, sizeof(long_image)) == XUI_OK);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiTerminalFlush(terminal) == XUI_ERROR_OUT_OF_MEMORY && capture.calls == 4);
    xrtMemDebugFailClear();
    xrtClearError();
    write_text(terminal, "\033Pq!2~\033\\");
    assert(capture.calls == 5 && !strcmp(capture.payload, "!2~"));

    capture.fail = true;
    assert(xuiTerminalWriteText(terminal, "\033Pq~\033\\") == XUI_OK);
    assert(xuiTerminalFlush(terminal) == XUI_ERROR_CALLBACK_FAILED);
    assert(xrtGetError());
    xrtClearError();
    assert(capture.calls == 6);
    assert(xuiDestroyChecked(context) == XUI_OK);
    assert(xuiProxyReleaseChecked(proxy) == XUI_OK);

    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount == after.LiveCount && before.LiveBytes == after.LiveBytes);
    puts("XUI_TERMINAL_IMAGE_CALLBACK_NATIVE_OK");
    return 0;
}
