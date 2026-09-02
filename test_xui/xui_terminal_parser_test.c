#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_terminal_parser_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_terminal_parser_test_state_t {
	char sTitle[128];
	char sInput[256];
	int iInputSize;
} xui_terminal_parser_test_state_t;

static void __xuiTerminalParserTestInput(xui_widget pWidget, const uint8_t* pData,
	int iSize, void* pUser)
{
	xui_terminal_parser_test_state_t* pState = (xui_terminal_parser_test_state_t*)pUser;
	int iCopy;
	(void)pWidget;
	if ( pState == NULL || pData == NULL || iSize <= 0 ) return;
	iCopy = iSize;
	if ( pState->iInputSize + iCopy >= (int)sizeof(pState->sInput) ) {
		iCopy = (int)sizeof(pState->sInput) - 1 - pState->iInputSize;
	}
	if ( iCopy > 0 ) {
		memcpy(pState->sInput + pState->iInputSize, pData, (size_t)iCopy);
		pState->iInputSize += iCopy;
		pState->sInput[pState->iInputSize] = '\0';
	}
}

static void __xuiTerminalParserTestTitle(xui_widget pWidget, const char* sTitle, void* pUser)
{
	xui_terminal_parser_test_state_t* pState = (xui_terminal_parser_test_state_t*)pUser;

	(void)pWidget;
	if ( pState != NULL && sTitle != NULL ) {
		strncpy(pState->sTitle, sTitle, sizeof(pState->sTitle) - 1u);
		pState->sTitle[sizeof(pState->sTitle) - 1u] = '\0';
	}
}

int main(void)
{
	xui_test_proxy_state_t tProxyState;
	xui_terminal_parser_test_state_t tState;
	xui_terminal_desc_t tDesc;
	xui_context pContext;
	xui_widget pTerminal;
	xui_font pFont;
	xui_terminal_cell_t tCell;
	char sBuffer[1024];
	int iFailed;
	int iRet;
	int iCursorX;
	int iCursorY;
	int iLine;
	int iColumn;
	int iLength;

	memset(&tProxyState, 0, sizeof(tProxyState));
	memset(&tState, 0, sizeof(tState));
	memset(&tDesc, 0, sizeof(tDesc));
	pContext = NULL;
	pTerminal = NULL;
	pFont = NULL;
	iFailed = 0;

	xuiTestProxyInit(&tProxyState);
	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tProxyState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tProxyState.tProxy.fontLoadMemory(&tProxyState.tProxy, &pFont, "terminal-parser", 15, 13.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "font set");

	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.iColumns = 12;
	tDesc.iRows = 5;
	tDesc.iScrollbackLimit = 8;
	tDesc.fCellWidth = 8.0f;
	tDesc.fCellHeight = 16.0f;
	tDesc.fPadding = 0.0f;
	iRet = xuiTerminalCreate(pContext, &pTerminal, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTerminal != NULL, "terminal create");
	(void)xuiTerminalSetTitleCallback(pTerminal, __xuiTerminalParserTestTitle, &tState);
	(void)xuiTerminalSetInputCallback(pTerminal, __xuiTerminalParserTestInput, &tState);

	iRet = xuiTerminalWriteText(pTerminal, "ab\r\ncd");
	XUI_TEST_CHECK(iRet == XUI_OK, "write crlf text");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush crlf text");
	iRet = xuiTerminalGetCell(pTerminal, 0, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'c', "line feed stores second row");
	iRet = xuiTerminalGetCursor(pTerminal, &iCursorX, &iCursorY);
	XUI_TEST_CHECK(iRet == XUI_OK && iCursorX == 2 && iCursorY == 1, "cursor after crlf");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear for cursor movement");
	iRet = xuiTerminalWriteText(pTerminal, "\x1b[3;4HX\x1b[AY\x1b[2D!");
	XUI_TEST_CHECK(iRet == XUI_OK, "write cursor movement");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush cursor movement");
	iRet = xuiTerminalGetCell(pTerminal, 3, 2, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'X', "CUP writes X");
	iRet = xuiTerminalGetCell(pTerminal, 4, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'Y', "CUU writes Y");
	iRet = xuiTerminalGetCell(pTerminal, 3, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == '!', "CUB writes exclamation");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear for erase");
	iRet = xuiTerminalWriteText(pTerminal, "erase-line\x1b[1;4H\x1b[KZ\x1b[2J");
	XUI_TEST_CHECK(iRet == XUI_OK, "write erase");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush erase");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && (tCell.iCodepoint == 0u || tCell.iCodepoint == ' '), "ED clears start cell");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear for sgr");
	iRet = xuiTerminalWriteText(pTerminal, "\x1b[38;2;12;34;56mT\x1b[7mI\x1b[0m");
	XUI_TEST_CHECK(iRet == XUI_OK, "write sgr");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush sgr");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'T' && tCell.iFgColor == XUI_COLOR_RGBA(12, 34, 56, 255), "true color fg");
	iRet = xuiTerminalGetCell(pTerminal, 1, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'I' && (tCell.iFlags & XUI_TERMINAL_CELL_INVERSE) != 0u, "inverse flag");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear for extended sgr");
	iRet = xuiTerminalWriteText(pTerminal, "\x1b[1;3;5;9mX\x1b[0m");
	XUI_TEST_CHECK(iRet == XUI_OK, "write extended sgr");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush extended sgr");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(tCell.iFlags & (XUI_TERMINAL_CELL_BOLD | XUI_TERMINAL_CELL_ITALIC |
		 XUI_TERMINAL_CELL_BLINK | XUI_TERMINAL_CELL_STRIKE)) ==
		(XUI_TERMINAL_CELL_BOLD | XUI_TERMINAL_CELL_ITALIC |
		 XUI_TERMINAL_CELL_BLINK | XUI_TERMINAL_CELL_STRIKE), "extended sgr flags");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear for character editing");
	iRet = xuiTerminalWriteText(pTerminal, "ABCDE\x1b[1;3H\x1b[2@Z");
	XUI_TEST_CHECK(iRet == XUI_OK, "write insert characters");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush insert characters");
	iRet = xuiTerminalGetCell(pTerminal, 2, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'Z', "ICH inserted character");
	iRet = xuiTerminalGetCell(pTerminal, 4, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'C', "ICH shifted tail");
	iRet = xuiTerminalWriteText(pTerminal, "\x1b[1;3H\x1b[2P");
	XUI_TEST_CHECK(iRet == XUI_OK, "write delete characters");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush delete characters");
	iRet = xuiTerminalGetCell(pTerminal, 2, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'C', "DCH compacts tail");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear for insert mode");
	iRet = xuiTerminalWriteText(pTerminal, "ABCDE\x1b[1;3H\x1b[4hZ\x1b[4l");
	XUI_TEST_CHECK(iRet == XUI_OK, "write insert mode");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush insert mode");
	iRet = xuiTerminalSerializeText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet > 0 && strstr(sBuffer, "ABZCDE") != NULL, "insert mode preserves tail");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear for control strings");
	iRet = xuiTerminalWriteText(pTerminal, "A\x1bPignored payload\x1b\\B");
	XUI_TEST_CHECK(iRet == XUI_OK, "write ignored DCS");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush ignored DCS");
	iRet = xuiTerminalSerializeText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet > 0 && strstr(sBuffer, "AB") != NULL && strstr(sBuffer, "ignored") == NULL,
		"DCS payload is not rendered");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear for DEC graphics");
	iRet = xuiTerminalWriteText(pTerminal, "\x1b(0lqk\x1b(B");
	XUI_TEST_CHECK(iRet == XUI_OK, "write DEC graphics");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush DEC graphics");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 0x250cu, "DEC graphics corner");
	iRet = xuiTerminalGetCell(pTerminal, 1, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 0x2500u, "DEC graphics line");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear for tab stops");
	iRet = xuiTerminalWriteText(pTerminal, "ab\x1bH\r\tX\r\t\x1b[0g\r\tY");
	XUI_TEST_CHECK(iRet == XUI_OK, "write tab stops");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush tab stops");
	iRet = xuiTerminalGetCell(pTerminal, 2, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'X', "custom tab stop");
	iRet = xuiTerminalGetCell(pTerminal, 8, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'Y', "cleared tab falls back");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear for utf8");
	iRet = xuiTerminalWriteText(pTerminal, "wide \xe4\xb8\xad");
	XUI_TEST_CHECK(iRet == XUI_OK, "write utf8");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush utf8");
	iRet = xuiTerminalGetCell(pTerminal, 5, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 0x4e2du && tCell.iWidth == 2, "utf8 cjk wide cell");
	iRet = xuiTerminalFindText(pTerminal, "\xe4\xb8\xad", 0, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == 1 && iLine == 0 && iColumn == 5, "find utf8");
	iRet = xuiTerminalGetFindMatch(pTerminal, &iLine, &iColumn, &iLength);
	XUI_TEST_CHECK(iRet == 1 && iLength == 2, "find utf8 width");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear for alt screen");
	iRet = xuiTerminalWriteText(pTerminal, "MAIN\x1b[?1049hALT\x1b[?1049l");
	XUI_TEST_CHECK(iRet == XUI_OK, "write alternate screen");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush alternate screen");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'M', "main screen restored after alt");

	iRet = xuiTerminalWriteText(pTerminal, "\x1b]0;Parser title\a");
	XUI_TEST_CHECK(iRet == XUI_OK, "write osc title");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tState.sTitle, "Parser title") == 0, "osc title callback");

	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	iRet = xuiTerminalWriteText(pTerminal, "\x1b[2;3H\x1b[6n\x1b[c");
	XUI_TEST_CHECK(iRet == XUI_OK, "write terminal reports");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK && strstr(tState.sInput, "\x1b[2;3R") != NULL &&
		strstr(tState.sInput, "\x1b[?1;2c") != NULL, "DSR and DA responses");

	iRet = xuiTerminalResize(pTerminal, 8, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "resize for horizontal reflow");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear history for horizontal reflow");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear for horizontal reflow");
	iRet = xuiTerminalWriteText(pTerminal, "\x1b[31mABCDEFGHIJ\x1b[0m");
	XUI_TEST_CHECK(iRet == XUI_OK, "write horizontal reflow text");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush horizontal reflow text");
	iRet = xuiTerminalResize(pTerminal, 4, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "shrink horizontal reflow");
	iRet = xuiTerminalResize(pTerminal, 8, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "grow horizontal reflow");
	iRet = xuiTerminalSerializeText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet > 0 && strstr(sBuffer, "ABCD") != NULL && strstr(sBuffer, "EFGHIJ") != NULL,
		"horizontal reflow retains all text");
	iRet = xuiTerminalGetCell(pTerminal, 0, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'E' && tCell.iFgColor != 0u,
		"horizontal reflow retains styled cells");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear for advanced find");
	iRet = xuiTerminalWriteText(pTerminal, "foo food FOO 123");
	XUI_TEST_CHECK(iRet == XUI_OK, "write advanced find text");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush advanced find text");
	iRet = xuiTerminalFindText(pTerminal, "foo", XUI_TERMINAL_SEARCH_WHOLE_WORD,
		&iLine, &iColumn);
	XUI_TEST_CHECK(iRet == 1 && iColumn == 0, "whole-word terminal find");
	iRet = xuiTerminalFindText(pTerminal, "[0-9]+", XUI_TERMINAL_SEARCH_REGEX,
		&iLine, &iColumn);
	XUI_TEST_CHECK(iRet == 1, "regex terminal find");
	iRet = xuiTerminalGetFindMatch(pTerminal, &iLine, &iColumn, &iLength);
	XUI_TEST_CHECK(iRet == 1 && iLength == 3, "regex terminal match width");

	iRet = xuiTerminalResize(pTerminal, 8, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "resize for stable history search");
	(void)xuiTerminalClearScrollback(pTerminal);
	(void)xuiTerminalClear(pTerminal);
	iRet = xuiTerminalWriteText(pTerminal, "xxneed\r\nsecond\r\nthird");
	if ( iRet == XUI_OK ) iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "write stable history search text");
	iRet = xuiTerminalFindText(pTerminal, "need", 0, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == 1 && iLine == 0 && iColumn == 2, "find stable history match");
	iRet = xuiTerminalResize(pTerminal, 4, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "reflow stable history match");
	iRet = xuiTerminalGetFindMatch(pTerminal, &iLine, &iColumn, &iLength);
	XUI_TEST_CHECK(iRet == 1 && iLine == 0 && iColumn == 2 && iLength == 4,
		"history search anchor survives reflow");

	iRet = xuiTerminalSerializeText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet > 0 && strstr(sBuffer, "xxne") != NULL &&
		strstr(sBuffer, "thir") != NULL, "serialize parser text");

cleanup:
	if ( pContext != NULL ) xuiDestroy(pContext);
	if ( iFailed ) return 1;
	printf("xui_terminal_parser_test passed\n");
	return 0;
}
