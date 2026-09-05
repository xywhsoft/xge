#ifndef XUI_TEXT_INTERNAL_H
#define XUI_TEXT_INTERNAL_H

#include "../xui.h"

/* Raw GetText/GetLine offsets always refer to the original UTF-8 source.
 * This borrowed, NUL-terminated display string omits SHY/ZWSP/WJ/FEFF and
 * adds '-' for a selected WORD soft-hyphen break. Valid until the next
 * display-line/Draw/Reset call or destruction. Do not use display byte
 * offsets for source caret/selection mapping. */
int xuiInternalTextLayoutGetDisplayLine(xui_text_layout pLayout, int iIndex,
	const char** ppText, int* pSize);

#endif
