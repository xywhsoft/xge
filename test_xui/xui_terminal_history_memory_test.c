#include "src/xui_internal.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t gDecodeSteps;
static size_t gCellReads;
static int gFailAfter = -1;
static void* history_realloc(void* p, size_t size)
{
	if (gFailAfter == 0) return NULL;
	if (gFailAfter > 0) --gFailAfter;
	return xrtRealloc(p, size);
}
#define XUI_TERMINAL_HISTORY_STEP(kind) (++g##kind)
#define xrtRealloc history_realloc
#include "../src/xui_terminal.c"
#undef xrtRealloc

static int gFailures;
#define CHECK(expr) do { if (!(expr)) { \
	printf("FAIL line %d: %s\n", __LINE__, #expr); ++gFailures; } } while (0)

typedef struct fixture_t {
	xui_test_proxy_state_t proxy;
	xui_context context;
	xui_widget terminal;
} fixture_t;

static void fixture_init(fixture_t* f, int columns, int rows, int limit)
{
	xui_terminal_desc_t desc;
	memset(f, 0, sizeof(*f));
	xuiTestProxyInit(&f->proxy);
	CHECK(xuiCreate(&f->context) == XUI_OK);
	CHECK(xuiSetProxy(f->context, &f->proxy.tProxy) == XUI_OK);
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.iColumns = columns;
	desc.iRows = rows;
	desc.iScrollbackLimit = limit;
	desc.fCellWidth = 8.0f;
	desc.fCellHeight = 16.0f;
	CHECK(xuiTerminalCreate(f->context, &f->terminal, &desc) == XUI_OK);
}

static void fixture_unit(fixture_t* f)
{
	xuiWidgetDestroy(f->terminal);
	xuiDestroy(f->context);
}

static void write_text(fixture_t* f, const char* text)
{
	CHECK(xuiTerminalWriteText(f->terminal, text) == XUI_OK);
	CHECK(xuiTerminalFlush(f->terminal) == XUI_OK);
}

static int same_cell(const xui_terminal_cell_t* a, const xui_terminal_cell_t* b)
{
	return a->iCodepoint == b->iCodepoint && a->iFgColor == b->iFgColor &&
		a->iBgColor == b->iBgColor && a->iFlags == b->iFlags && a->iStyle == b->iStyle &&
		a->iLinkId == b->iLinkId && a->iWidth == b->iWidth &&
		a->iCombiningCount == b->iCombiningCount &&
		memcmp(a->arrCombining, b->arrCombining, a->iCombiningCount * sizeof(uint32_t)) == 0;
}

static void codec_roundtrip(void)
{
	xui_terminal_cell_t row[257], reference[771], decoded;
	xui_terminal_history_line_t* line;
	xui_terminal_history_reader_t reader;
	int i, j, failure, count;
	memset(row, 0, sizeof(row));
	for (i = 0; i < 257; ++i) {
		row[i].iCodepoint = (uint32_t)('A' + i % 26);
		row[i].iWidth = 1;
		row[i].iFgColor = XUI_COLOR_RGBA(i, 34, 56, 255);
		row[i].iBgColor = XUI_COLOR_RGBA(98, i, 76, 255);
		row[i].iFlags = (uint32_t)(i % 8) | XUI_TERMINAL_CELL_ITALIC | XUI_TERMINAL_CELL_STRIKE;
		row[i].iStyle = (uint16_t)i;
		row[i].iLinkId = (uint16_t)(i / 7);
	}
	for (i = 0; i < 254; i += 11) {
		row[i].iCodepoint = i % 2 ? 0x4e2du : 0x1f600u;
		row[i].iWidth = 2;
		row[i].iFlags |= XUI_TERMINAL_CELL_WIDE;
		row[i + 1].iCodepoint = 0;
		row[i + 1].iWidth = 0;
		row[i + 1].iFlags |= XUI_TERMINAL_CELL_WIDE_CONT;
	}
	for (i = 3; i < 257; i += 13) {
		row[i].iCombiningCount = XUI_TERMINAL_MAX_COMBINING;
		row[i].iFlags |= XUI_TERMINAL_CELL_COMBINING | XUI_TERMINAL_CELL_COMBINING_OVERFLOW;
		for (j = 0; j < XUI_TERMINAL_MAX_COMBINING; ++j) row[i].arrCombining[j] = 0x301u + (uint32_t)j;
	}
	for (failure = 0; failure < 5; ++failure) {
		line = (xui_terminal_history_line_t*)xrtCalloc(1, sizeof(*line));
		gFailAfter = failure;
		CHECK(__xuiTerminalHistoryAppendRow(line, row, 257, 1) == XUI_ERROR_OUT_OF_MEMORY);
		CHECK(line->iCellCount == 0 && line->iRunCount == 0 && line->iTextSize == 0 && !line->bOpen);
		gFailAfter = -1;
		CHECK(__xuiTerminalHistoryAppendRow(line, row, 257, 1) == XUI_OK);
		for (i = 0; i < 257; ++i) {
			__xuiTerminalHistoryCellToPublic(line, i, &decoded);
			CHECK(same_cell(&decoded, &row[i]));
		}
		__xuiTerminalHistoryLineDestroy(line);
	}
	line = (xui_terminal_history_line_t*)xrtCalloc(1, sizeof(*line));
	for (j = 0; j < 3; ++j) {
		CHECK(__xuiTerminalHistoryAppendRow(line, row, 257, j < 2) == XUI_OK);
		memcpy(reference + j * 257, row, sizeof(row));
	}
	CHECK(line->iCellCount == 771 && !line->bOpen);
	__xuiTerminalHistoryReaderInit(&reader, line, 0);
	for (i = 0; i < 771; ++i) {
		__xuiTerminalHistoryReaderNext(&reader, &decoded);
		CHECK(same_cell(&decoded, &reference[i]));
	}
	{
		xui_terminal_data_t data;
		int columns, r, cell;
		memset(&data, 0, sizeof(data));
		data.ppHistory = &line;
		data.iHistoryCount = data.iScrollbackLimit = 1;
		for (columns = 1; columns <= 19; ++columns) {
			data.iColumns = columns;
			CHECK(__xuiTerminalRebuildHistoryView(&data) == XUI_OK);
			cell = 0;
			for (r = 0; r < data.iScrollbackCount; ++r) {
				xui_terminal_history_view_row_t* view = &data.pHistoryView[r];
				CHECK(view->iStart == cell && view->iEnd > cell && view->iEnd - cell <= columns);
				if (columns > 1 && view->iEnd < line->iCellCount)
					CHECK((reference[view->iEnd - 1].iFlags & XUI_TERMINAL_CELL_WIDE) == 0u ||
						(reference[view->iEnd].iFlags & XUI_TERMINAL_CELL_WIDE_CONT) == 0u);
				__xuiTerminalHistoryReaderInit(&reader, line, cell);
				for (; cell < view->iEnd; ++cell) {
					__xuiTerminalHistoryReaderNext(&reader, &decoded);
					CHECK(same_cell(&decoded, &reference[cell]));
				}
			}
			CHECK(cell == 771);
		}
		__xuiTerminalHistoryViewClear(&data);
		xrtFree(data.pHistoryView);
	}
	for (count = 513; count >= 0; count -= 19) {
		__xuiTerminalHistoryTruncate(line, count);
		CHECK(__xuiTerminalHistoryAppendRow(line, row, 257, 1) == XUI_OK);
		memcpy(reference + count, row, sizeof(row));
		for (i = 0; i < line->iCellCount; ++i) {
			__xuiTerminalHistoryCellToPublic(line, i, &decoded);
			CHECK(same_cell(&decoded, &reference[i]));
		}
	}
	__xuiTerminalHistoryTruncate(line, 0);
	CHECK(line->iRunCount == 0 && line->iGlyphCount == 0 && line->iCombiningCount == 0);
	CHECK(__xuiTerminalHistoryAppendRow(line, row, 257, 0) == XUI_OK);
	__xuiTerminalHistoryLineDestroy(line);
	printf("codec: truecolor/style/link, wide/continuation, 16 combining marks, truncation/reappend, 5 allocation rollback points\n");
}

static void indexed_reads(int cells)
{
	fixture_t f;
	xui_terminal_data_t* data;
	xui_terminal_history_line_t* line;
	xui_terminal_cell_t row[256], cell;
	xui_terminal_history_reader_t reader;
	xui_terminal_history_view_row_t* view;
	xui_draw_context draw = NULL;
	xui_surface surface = NULL;
	xui_font font = NULL;
	size_t seekSteps;
	int i, j;
	fixture_init(&f, 80, 4, 4);
	data = __xuiTerminalGetData(f.terminal);
	memset(row, 0, sizeof(row));
	for (i = 0; i < 256; ++i) { row[i].iCodepoint = 'A' + i % 26; row[i].iWidth = 1; }
	for (i = 0; i < cells; i += 256)
		CHECK(__xuiTerminalPushScrollback(data, row, 256, 1) == XUI_OK);
	line = __xuiTerminalHistoryLastLine(data);
	CHECK(line->iCellCount == cells && line->iRunCount == 1 && line->iGlyphCount == 0);
	CHECK(line->iTextSize == cells);
	gDecodeSteps = gCellReads = 0;
	for (i = 0; i < 32; ++i) {
		__xuiTerminalHistoryCellToPublic(line, cells - 100 + i, &cell);
		CHECK(cell.iCodepoint == (uint32_t)('A' + ((cells - 100 + i) % 256) % 26));
	}
	seekSteps = gDecodeSteps;
	CHECK(gCellReads == 32 && gDecodeSteps <= 32u * XUI_TERMINAL_HISTORY_TEXT_STRIDE);
	gDecodeSteps = gCellReads = 0;
	CHECK(__xuiTerminalRebuildHistoryView(data) == XUI_OK);
	CHECK(gDecodeSteps == 0 && gCellReads == 0);
	CHECK(f.proxy.tProxy.fontLoadMemory(&f.proxy.tProxy, &font, "history", 7, 13.0f, XUI_FONT_FORMAT_TTF) == XUI_OK);
	data->pFont = font;
	CHECK(xuiTestSurfaceCreate(&f.proxy, &surface, 640, 64, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
	CHECK(f.proxy.tProxy.drawBegin(&f.proxy.tProxy, &draw, surface) == XUI_OK);
	for (j = 0; j < 32; ++j) {
		i = data->iScrollbackCount - 3 + j % 3;
		view = __xuiTerminalGetHistoryViewRow(data, i);
		gDecodeSteps = gCellReads = 0;
		CHECK(__xuiTerminalRenderHistoryRow(&f.proxy.tProxy, draw, data, i, 0.0f, 0.0f) == XUI_OK);
		CHECK(gCellReads <= 80 && gDecodeSteps <= 80 + XUI_TERMINAL_HISTORY_TEXT_STRIDE);
		__xuiTerminalHistoryReaderInit(&reader, line, view->iStart);
		for (i = 0; i < view->iEnd - view->iStart; ++i) {
			__xuiTerminalHistoryReaderNext(&reader, &cell);
			CHECK(same_cell(&cell, &data->pHistoryRenderCells[i]));
		}
	}
	gDecodeSteps = gCellReads = 0;
	CHECK(strlen(__xuiTerminalGetScrollbackLine(data, data->iScrollbackCount - 2)) == 80);
	CHECK(gCellReads == 80 && gDecodeSteps <= 80 + 2 * XUI_TERMINAL_HISTORY_TEXT_STRIDE);
	printf("indexed cells=%d: 32 random reads=%zu decodes; deep 80-column copy=%zu; render <=111 per row\n",
		cells, seekSteps, gDecodeSteps);
	CHECK(f.proxy.tProxy.drawEnd(&f.proxy.tProxy, draw) == XUI_OK);
	f.proxy.tProxy.surfaceDestroy(&f.proxy.tProxy, surface);
	fixture_unit(&f);
}

static void resize_and_public_semantics(void)
{
	fixture_t f;
	xui_terminal_data_t* data;
	xui_terminal_history_line_t* line;
	xui_terminal_cell_t expected[8], cell;
	char text[4096];
	char* url = NULL;
	int i, j, start, length, historyCount;
	fixture_init(&f, 8, 2, 64);
	data = __xuiTerminalGetData(f.terminal);
	write_text(&f, "\x1b[38;2;12;34;56;48;2;98;76;54;1;3;4mABe\xcc\x81\xe4\xb8\xad" "C\xf0\x9f\x98\x80");
	for (i = 0; i < 8; ++i) CHECK(xuiTerminalGetCell(f.terminal, i, 0, &expected[i]) == XUI_OK);
	write_text(&f, "\x1b[0m\r\nnext\r\nlast");
	line = data->ppHistory[data->iScrollbackStart];
	CHECK(line->iCellCount == 8 && !line->bOpen);
	for (i = 0; i < 8; ++i) {
		__xuiTerminalHistoryCellToPublic(line, i, &cell);
		CHECK(same_cell(&cell, &expected[i]));
	}
	for (j = 0; j < 4; ++j) {
		CHECK(xuiTerminalResize(f.terminal, 3, 2) == XUI_OK);
		CHECK(xuiTerminalResize(f.terminal, 8, 4) == XUI_OK);
		for (i = 0; i < 8; ++i) {
			CHECK(xuiTerminalGetCell(f.terminal, i, 0, &cell) == XUI_OK);
			CHECK(same_cell(&cell, &expected[i]));
		}
	}
	CHECK(xuiTerminalResize(f.terminal, 8, 2) == XUI_OK);
	CHECK(xuiTerminalClearScrollback(f.terminal) == XUI_OK);
	CHECK(xuiTerminalClear(f.terminal) == XUI_OK);
	write_text(&f, "ABCDEFGHijklmnopQRSTUVWXyz\r\nHARD\r\nTAIL");
	CHECK(data->iHistoryCount == 1 && !data->ppHistory[data->iScrollbackStart]->bOpen);
	for (j = 0; j < 3; ++j) {
		CHECK(xuiTerminalResize(f.terminal, 4, 2) == XUI_OK);
		CHECK(xuiTerminalResize(f.terminal, 8, 8) == XUI_OK);
		CHECK(xuiTerminalResize(f.terminal, 32, 4) == XUI_OK);
		CHECK(xuiTerminalSerializeText(f.terminal, text, sizeof(text)) > 0);
		CHECK(strstr(text, "ABCDEFGHijklmnopQRSTUVWXyz\nHARD\nTAIL") != NULL);
	}
	CHECK(xuiTerminalClearScrollback(f.terminal) == XUI_OK);
	CHECK(xuiTerminalClear(f.terminal) == XUI_OK);
	CHECK(xuiTerminalResize(f.terminal, 32, 2) == XUI_OK);
	write_text(&f, "\x1b]8;;https://example.test/history\x1b\\A\x1b[31mBe\xcc\x81\xe4\xb8\xad\x1b]8;;\x1b\\\r\nnext\r\nlast");
	CHECK(xuiTerminalFindText(f.terminal, "Be\xcc\x81\xe4\xb8\xad", 0, &start, &length) == 1);
	CHECK(start == 0 && length == 1);
	CHECK(xuiTerminalSetSelectionRange(f.terminal, 0, 0, 0, 5) == XUI_OK);
	CHECK(xuiTerminalGetSelectionText(f.terminal, text, sizeof(text)) > 0);
	CHECK(strcmp(text, "ABe\xcc\x81\xe4\xb8\xad") == 0);
	CHECK(xuiTerminalCopySelection(f.terminal) == XUI_OK);
	CHECK(__xuiTerminalOscLinkAt(data, 0, 3, &url, &start, &length) == (int)strlen("https://example.test/history"));
	CHECK(url != NULL && strcmp(url, "https://example.test/history") == 0 && start == 0 && length == 5);
	xrtFree(url);
	historyCount = data->iHistoryCount;
	write_text(&f, "\x1b[?1049h\x1b[?1002h\x1b[?1006hTUI\x1b[2;1Hbottom\r\nalt-scroll");
	CHECK(data->bAltScreen && data->iMouseTracking == 1002 && data->iMouseEncoding == 1006);
	CHECK(data->iHistoryCount == historyCount && data->pMain != data->pAlt);
	write_text(&f, "\x1b[?1049l\x1b[?1002l\x1b[?1006l");
	CHECK(!data->bAltScreen && data->iMouseTracking == 0 && data->iHistoryCount == historyCount);
	CHECK(xuiTerminalClearScrollback(f.terminal) == XUI_OK);
	CHECK(data->iHistoryCount == 0);
	fixture_unit(&f);
	printf("public: resize round trips, soft/hard wrap, wide/combining/color, history find/selection/copy/link, alternate/TUI/mouse modes\n");
}

static void memory_budget(void)
{
	/* Capacity-based model: live grids, history streams/views/caches, render scratch and queue.
	 * Excludes allocator overhead, proxy surfaces/fonts and caller-owned input/copy buffers.
	 * Before compaction (92230b0): 51,982,576 bytes; after resize/search/copy: 53,649,816.
	 * All four budget assertions below failed on that baseline. */
	static const char line[] =
		"\x1b[32m00000000\x1b[0m generated terminal output with colors and cells 0123456789\r\n";
	const int inputSize = 4 * 1024 * 1024;
	xui_test_proxy_state_t proxy;
	xui_context context = NULL;
	xui_widget terminal = NULL;
	xui_terminal_desc_t desc;
	xui_terminal_stats_t stats;
	char* input = (char*)malloc((size_t)inputSize);
	char* text;
	int i, size;
	CHECK(input != NULL);
	if (input == NULL) return;
	for (i = 0; i < inputSize; ++i) input[i] = line[i % (sizeof(line) - 1)];
	memset(&proxy, 0, sizeof(proxy));
	xuiTestProxyInit(&proxy);
	CHECK(xuiCreate(&context) == XUI_OK);
	CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
	memset(&desc, 0, sizeof(desc));
	desc.iSize = sizeof(desc);
	desc.iColumns = 120;
	desc.iRows = 40;
	desc.iScrollbackLimit = 20000;
	desc.iParseBudgetBytes = 65536;
	desc.fCellWidth = 8.0f;
	desc.fCellHeight = 16.0f;
	CHECK(xuiTerminalCreate(context, &terminal, &desc) == XUI_OK);
	for (i = 0; i < inputSize; i += 65536)
		CHECK(xuiTerminalWrite(terminal, input + i, 65536) == XUI_OK);
	CHECK(xuiTerminalFlush(terminal) == XUI_OK);
	CHECK(xuiTerminalGetStats(terminal, &stats) == XUI_OK);
	printf("4MiB retained=%d history=%zu screen=%zu queue=%zu total=%zu bytes\n",
		stats.iHistoryLogicalLines, stats.iHistoryMemoryBytes, stats.iScreenMemoryBytes,
		stats.iQueueMemoryBytes,
		stats.iHistoryMemoryBytes + stats.iScreenMemoryBytes + stats.iQueueMemoryBytes);
	CHECK(stats.iHistoryLogicalLines == 20000);
	CHECK(stats.iOutputBytesParsed == (uint64_t)inputSize);
	CHECK(stats.iHistoryMemoryBytes <= 8u * 1024u * 1024u);
	CHECK(stats.iHistoryMemoryBytes + stats.iScreenMemoryBytes + stats.iQueueMemoryBytes <= 14u * 1024u * 1024u);
	CHECK(xuiTerminalResize(terminal, 72, 40) == XUI_OK);
	CHECK(xuiTerminalResize(terminal, 160, 40) == XUI_OK);
	CHECK(xuiTerminalFindText(terminal, "generated terminal output", 0, NULL, NULL) == 1);
	size = xuiTerminalSerializeText(terminal, NULL, 0);
	text = (char*)malloc((size_t)size + 1u);
	CHECK(text != NULL);
	if (text != NULL) {
		CHECK(xuiTerminalSerializeText(terminal, text, size + 1) == size);
		CHECK(strstr(text, "generated terminal output") != NULL);
		free(text);
	}
	CHECK(xuiTerminalGetStats(terminal, &stats) == XUI_OK);
	printf("after resize/search/full-copy history=%zu total=%zu bytes\n",
		stats.iHistoryMemoryBytes,
		stats.iHistoryMemoryBytes + stats.iScreenMemoryBytes + stats.iQueueMemoryBytes);
	CHECK(stats.iHistoryMemoryBytes <= 10u * 1024u * 1024u);
	CHECK(stats.iHistoryMemoryBytes + stats.iScreenMemoryBytes + stats.iQueueMemoryBytes <= 16u * 1024u * 1024u);
	xuiWidgetDestroy(terminal);
	xuiDestroy(context);
	free(input);
}

int main(void)
{
	codec_roundtrip();
	indexed_reads(4096);
	indexed_reads(65536);
	indexed_reads(1048576);
	resize_and_public_semantics();
	memory_budget();
	printf("xui_terminal_history_memory_test: %s (%d failures)\n",
		gFailures ? "FAILED" : "passed", gFailures);
	return gFailures ? 1 : 0;
}
