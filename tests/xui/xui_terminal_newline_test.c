#include "xui.h"
#include <stdio.h>
#include <string.h>

#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr, "line %d: %s\n", __LINE__, #x); failed = 1; goto cleanup; } } while (0)

static int at(xui_widget terminal, int col, int row, uint32_t codepoint)
{
    xui_terminal_cell_t cell;
    memset(&cell, 0, sizeof(cell));
    return xuiTerminalGetCell(terminal, col, row, &cell) == XUI_OK && cell.iCodepoint == codepoint;
}

static int reentryResult;
static void switchOnResponse(xui_widget widget, const uint8_t* data, int size, void* user)
{
    (void)data; (void)size; (void)user;
    reentryResult = xuiTerminalSetNewlineMode(widget, XUI_TERMINAL_NEWLINE_VT);
}

static void destroyOnResponse(xui_widget widget, const uint8_t* data, int size, void* user)
{
    (void)data; (void)size; (void)user;
    xuiWidgetDestroy(widget);
}

int main(void)
{
    xui_context context = NULL;
    xui_widget terminal = NULL;
    xui_terminal_desc_t desc;
    int checks = 0, failed = 0, x = 0, y = 0;
    memset(&desc, 0, sizeof(desc));
    desc.iSize = sizeof(desc);
    desc.iColumns = 12;
    desc.iRows = 5;
    desc.fCellWidth = 8;
    desc.fCellHeight = 16;
    CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiTerminalCreate(context, &terminal, &desc) == XUI_OK);
    CHECK(xuiTerminalGetNewlineMode(terminal) == XUI_TERMINAL_NEWLINE_TEXT);
    CHECK(xuiTerminalWriteText(terminal, "abc\ndef") == XUI_OK);
    CHECK(xuiTerminalFlush(terminal) == XUI_OK);
    CHECK(at(terminal, 0, 1, 'd'));
    CHECK(xuiTerminalGetCursor(terminal, &x, &y) == XUI_OK && x == 3 && y == 1);

    CHECK(xuiTerminalClear(terminal) == XUI_OK);
    CHECK(xuiTerminalWriteText(terminal, "abc\r") == XUI_OK);
    CHECK(xuiTerminalFlush(terminal) == XUI_OK);
    CHECK(xuiTerminalWriteText(terminal, "\ndef\n\ng") == XUI_OK);
    CHECK(xuiTerminalFlush(terminal) == XUI_OK);
    CHECK(at(terminal, 0, 1, 'd') && at(terminal, 0, 3, 'g'));

    CHECK(xuiTerminalClear(terminal) == XUI_OK);
    CHECK(xuiTerminalWriteText(terminal, "abc\rX") == XUI_OK);
    CHECK(xuiTerminalFlush(terminal) == XUI_OK);
    CHECK(at(terminal, 0, 0, 'X') && at(terminal, 1, 0, 'b'));
    CHECK(xuiTerminalGetCursor(terminal, &x, &y) == XUI_OK && x == 1 && y == 0);

    /* Queue ordering: the pending LF must still use TEXT when switching to VT. */
    CHECK(xuiTerminalClear(terminal) == XUI_OK);
    CHECK(xuiTerminalWriteText(terminal, "abc\nd") == XUI_OK);
    CHECK(xuiTerminalSetNewlineMode(terminal, XUI_TERMINAL_NEWLINE_VT) == XUI_OK);
    CHECK(at(terminal, 0, 1, 'd'));
    CHECK(xuiTerminalGetNewlineMode(terminal) == XUI_TERMINAL_NEWLINE_VT);
    CHECK(xuiTerminalWriteText(terminal, "\nE") == XUI_OK);
    CHECK(xuiTerminalFlush(terminal) == XUI_OK);
    CHECK(at(terminal, 1, 2, 'E'));
    CHECK(xuiTerminalClear(terminal) == XUI_OK);
    CHECK(xuiTerminalGetNewlineMode(terminal) == XUI_TERMINAL_NEWLINE_VT);
    CHECK(xuiTerminalWriteText(terminal, "abc\ndef") == XUI_OK);
    CHECK(xuiTerminalFlush(terminal) == XUI_OK);
    CHECK(at(terminal, 3, 1, 'd'));
    CHECK(xuiTerminalGetCursor(terminal, &x, &y) == XUI_OK && x == 6 && y == 1);
    CHECK(xuiTerminalWriteText(terminal, "\033cabc\nx") == XUI_OK);
    CHECK(xuiTerminalFlush(terminal) == XUI_OK);
    CHECK(xuiTerminalGetNewlineMode(terminal) == XUI_TERMINAL_NEWLINE_VT && at(terminal, 3, 1, 'x'));
    CHECK(xuiTerminalResize(terminal, 13, 5) == XUI_OK);
    CHECK(xuiTerminalGetNewlineMode(terminal) == XUI_TERMINAL_NEWLINE_VT);

    CHECK(xuiTerminalWriteText(terminal, "\033[?1049h") == XUI_OK);
    CHECK(xuiTerminalFlush(terminal) == XUI_OK);
    CHECK(xuiTerminalGetNewlineMode(terminal) == XUI_TERMINAL_NEWLINE_VT);
    CHECK(xuiTerminalWriteText(terminal, "\033[?1049l") == XUI_OK);
    CHECK(xuiTerminalFlush(terminal) == XUI_OK);
    CHECK(xuiTerminalGetNewlineMode(terminal) == XUI_TERMINAL_NEWLINE_VT);
    CHECK(xuiTerminalSetNewlineMode(terminal, 42) == XUI_ERROR_INVALID_ARGUMENT);
    CHECK(xuiTerminalGetNewlineMode(terminal) == XUI_TERMINAL_NEWLINE_VT);
    CHECK(xuiTerminalSetNewlineMode(NULL, XUI_TERMINAL_NEWLINE_TEXT) == XUI_ERROR_INVALID_ARGUMENT);
    CHECK(xuiTerminalGetNewlineMode(NULL) == XUI_ERROR_INVALID_ARGUMENT);
    CHECK(xuiTerminalSetNewlineMode(terminal, XUI_TERMINAL_NEWLINE_TEXT) == XUI_OK);
    CHECK(xuiTerminalClear(terminal) == XUI_OK);

    /* Split UTF-8, SGR and LF are processed without rewriting the byte stream. */
    CHECK(xuiTerminalWriteText(terminal, "abc\n\033[31m\xe4") == XUI_OK);
    CHECK(xuiTerminalFlush(terminal) == XUI_OK);
    CHECK(xuiTerminalWriteText(terminal, "\xbd\xa0\033[0m\nZ") == XUI_OK);
    CHECK(xuiTerminalFlush(terminal) == XUI_OK);
    CHECK(at(terminal, 0, 1, 0x4f60) && at(terminal, 0, 2, 'Z'));
    CHECK(xuiTerminalClear(terminal) == XUI_OK);
    CHECK(xuiTerminalWriteText(terminal, "abc\033DX") == XUI_OK);
    CHECK(xuiTerminalFlush(terminal) == XUI_OK);
    CHECK(at(terminal, 3, 1, 'X')); /* IND remains VT even in TEXT mode. */
    CHECK(xuiTerminalResize(terminal, 3, 5) == XUI_OK);
    CHECK(xuiTerminalClear(terminal) == XUI_OK);
    CHECK(xuiTerminalWriteText(terminal, "abcdef") == XUI_OK);
    CHECK(xuiTerminalFlush(terminal) == XUI_OK);
    CHECK(at(terminal, 0, 1, 'd')); /* Auto-wrap still starts at column zero. */
    CHECK(xuiTerminalWriteText(terminal, "\nZ") == XUI_OK);
    CHECK(xuiTerminalFlush(terminal) == XUI_OK);
    CHECK(at(terminal, 0, 2, 'Z')); /* Pending wrap plus LF advances once. */
    CHECK(xuiTerminalWriteText(terminal, "\nA\nB\nC\nD\nE") == XUI_OK);
    CHECK(xuiTerminalFlush(terminal) == XUI_OK);
    CHECK(xuiTerminalGetCursor(terminal, &x, &y) == XUI_OK && x == 1 && y == 4);
    CHECK(at(terminal, 0, 4, 'E'));
    CHECK(xuiTerminalSetInputCallback(terminal, switchOnResponse, NULL) == XUI_OK);
    CHECK(xuiTerminalWriteText(terminal, "\033[5n") == XUI_OK);
    CHECK(xuiTerminalSetNewlineMode(terminal, XUI_TERMINAL_NEWLINE_VT) == XUI_OK);
    CHECK(reentryResult == XUI_ERROR_INVALID_STATE);
    CHECK(xuiTerminalGetNewlineMode(terminal) == XUI_TERMINAL_NEWLINE_VT);
    CHECK(xuiTerminalSetNewlineMode(terminal, XUI_TERMINAL_NEWLINE_TEXT) == XUI_OK);
    CHECK(xuiTerminalSetInputCallback(terminal, destroyOnResponse, NULL) == XUI_OK);
    CHECK(xuiTerminalWriteText(terminal, "\033[5n") == XUI_OK);
    CHECK(xuiTerminalSetNewlineMode(terminal, XUI_TERMINAL_NEWLINE_VT) == XUI_ERROR_INVALID_STATE);
cleanup:
    if (context != NULL) xuiDestroy(context);
    if (!failed) printf("xui_terminal_newline_test passed: %d checks\n", checks);
    return failed;
}
