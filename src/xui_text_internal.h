#ifndef XUI_TEXT_INTERNAL_H
#define XUI_TEXT_INTERNAL_H

#include "../xui.h"

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

/* Raw GetText/GetLine offsets always refer to the original UTF-8 source.
 * This borrowed, NUL-terminated display string omits SHY/ZWSP/WJ/FEFF and
 * adds '-' for a selected WORD soft-hyphen break. Valid until the next
 * display-line/Draw/Reset call or destruction. Do not use display byte
 * offsets for source caret/selection mapping. */
int xuiInternalTextLayoutGetDisplayLine(xui_text_layout pLayout, int iIndex,
	const char** ppText, int* pSize);

/* Geometry remains in original UTF-8 coordinates, including discretionary hyphens. */
int xuiInternalTextLayoutLineAdvance(xui_text_layout pLayout, int iLine,
	int iOffset, float* pAdvance);
int xuiInternalTextLayoutNextCaret(xui_text_layout pLayout, int iOffset,
	int iLimit, int* pNext);

/* Font-independent source lines; CRLF is one mandatory break. */
int xuiInternalTextNextHardLine(const char* sText, int iSize, int iStart,
	int* pEnd, int* pNext);

#endif
