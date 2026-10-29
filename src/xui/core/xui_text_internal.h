#ifndef XUI_TEXT_INTERNAL_H
#define XUI_TEXT_INTERNAL_H

#include "xui.h"

#define XUI_TEXT_BREAK_FLAGS_DEFINED 1
enum {
    XUI_LB_GRAPHEME = 1, XUI_LB_NORMAL = 2, XUI_LB_EMERGENCY = 4,
    XUI_LB_HARD = 8, XUI_LB_INVISIBLE = 16, XUI_LB_SOFT_HYPHEN = 32, XUI_LB_HYPHEN_USED = 64,
    XUI_LB_HARD_END = 128
};
/* Caller supplies bytes+1 zeroed boundary bytes. HARD marks a control's start;
 * HARD_END marks the end of a mandatory sequence (CRLF only at LF's end).
 * Shared by plain and rich text so styles do not change break decisions. */
int xuiInternalTextBreakMap(const char* text, int bytes, unsigned char* boundaries);

/* Display deletion and source-coordinate shaping shared with Document.
 * CopyDisplay accepts a NULL output to count bytes; otherwise caller provides
 * at least bytes bytes. It does not append a terminator. */
int xuiInternalTextCopyDisplay(const char* text, int bytes, char* output);
int xuiInternalTextDisplayGraphemes(const char* text, int bytes, unsigned char* boundaries);
int xuiInternalTextShapeProjection(xui_context context, xui_font font,
    const char* text, int bytes, const char* language, xui_text_shape_t* shape);
/* Normalize ordered glyph clusters into Document caret fragments. Consumes
 * optional shaper caret stops on success; preserves the input on failure. */
int xuiInternalTextShapeCaretFragments(const char* text, int bytes, xui_text_shape_t* shape);

/* Shared geometric byte-buffer growth for mutable text and scratch storage. */
int xuiInternalTextBufferReserve(char** ppBuffer, int* pCapacity, int iRequired);

/* Long unwrapped text is retained in independently shaped chunks. Edits
 * rebuild only the touched chunks and their shaping neighbours; offsets and
 * aggregate advances for the untouched tail are updated without reshaping. */
#define XUI_INTERNAL_TEXT_CHUNK_THRESHOLD  (16 * 1024)
#define XUI_INTERNAL_TEXT_CHUNK_BYTES       (4 * 1024)
#define XUI_INTERNAL_TEXT_CHUNK_LOOKAHEAD   256

typedef struct xui_internal_text_chunk_t {
	int iStart;
	int iEnd;
	int iShapeStart;
	int iShapeEnd;
	int iCoreClusterCount;
	char* sText;
	double* pAdvances;
	xui_text_shape_t tShape;
	float fWidth;
} xui_internal_text_chunk_t;

typedef struct xui_internal_text_chunk_cache_t {
	xui_internal_text_chunk_t* pChunks;
	double* pChunkAdvances;
	int iChunkCount;
	int iChunkCapacity;
	int iTextSize;
	xui_font pFont;
	uint32_t iDpiGeneration;
	int bValid;
} xui_internal_text_chunk_cache_t;

void xuiInternalTextChunkCacheClear(xui_internal_text_chunk_cache_t* pCache);
int xuiInternalTextChunkCacheEnsure(xui_context pContext,
	xui_internal_text_chunk_cache_t* pCache, xui_font pFont,
	const char* sText, int iTextSize, uint32_t iDpiGeneration);
int xuiInternalTextChunkCachePatch(xui_context pContext,
	xui_internal_text_chunk_cache_t* pCache, const char* sText, int iTextSize,
	int iEditStart, int iOldEnd, int iNewEnd);
int xuiInternalTextChunkCacheAdvance(const xui_internal_text_chunk_cache_t* pCache,
	int iOffset, float* pAdvance);
int xuiInternalTextChunkCacheHitTest(const xui_internal_text_chunk_cache_t* pCache,
	float fAdvance, int bGlyph, int* pOffset);
float xuiInternalTextChunkCacheWidth(const xui_internal_text_chunk_cache_t* pCache);
int xuiInternalTextChunkCacheDraw(xui_proxy pProxy, xui_draw_context pDraw,
	const xui_internal_text_chunk_cache_t* pCache, xui_rect_t tViewport,
	float fOriginX, float fY, float fHeight, uint32_t iColor, uint32_t iFlags);
int xuiInternalTextChunkCacheDrawSpans(xui_proxy pProxy, xui_draw_context pDraw,
	const xui_internal_text_chunk_cache_t* pCache, xui_rect_t tViewport,
	float fOriginX, float fY, float fHeight, uint32_t iColor, uint32_t iFlags,
	const xui_text_paint_span_t* pSpans, int iSpanCount);

/* Raw GetText/GetLine offsets always refer to the original UTF-8 source.
 * This borrowed, NUL-terminated display string omits SHY/ZWSP/WJ/FEFF and
 * adds '-' for a selected WORD soft-hyphen break. Valid until the next
 * display-line/Draw/Reset call or destruction. Do not use display byte
 * offsets for source caret/selection mapping. */
int xuiInternalTextLayoutGetDisplayLine(xui_text_layout pLayout, int iIndex,
	const char** ppText, int* pSize);

/* Draw retained layout lines through an existing cache/render draw context.
 * Line geometry is snapped once from float top/bottom edges so fractional
 * metrics cannot lose the final pixel row. */
int xuiInternalTextLayoutDrawContext(xui_text_layout pLayout,
	xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor,
	uint32_t iFlags);

/* Geometry remains in original UTF-8 coordinates, including discretionary hyphens. */
int xuiInternalTextLayoutLineAdvance(xui_text_layout pLayout, int iLine,
	int iOffset, float* pAdvance);
int xuiInternalTextLayoutLineHitTest(xui_text_layout pLayout, int iLine,
	float fAdvance, int bGlyph, int* pOffset);
int xuiInternalTextLayoutNextCaret(xui_text_layout pLayout, int iOffset,
	int iLimit, int* pNext);

/* Font-independent source lines; CRLF is one mandatory break. */
int xuiInternalTextNextHardLine(const char* sText, int iSize, int iStart,
	int* pEnd, int* pNext);

#endif
