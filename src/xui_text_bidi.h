#ifndef XUI_TEXT_BIDI_H
#define XUI_TEXT_BIDI_H
#include "../xui.h"
/* Shared paragraph analysis; byte positions always refer to immutable logical
 * UTF-8. Visual runs are a separate per-line projection, never stored text. */
typedef struct xui_text_bidi_t* xui_text_bidi;
enum { XUI_BIDI_LTR=0, XUI_BIDI_RTL=1, XUI_BIDI_AUTO_LTR=254, XUI_BIDI_AUTO_RTL=253 };
typedef struct xui_bidi_paragraph_t { size_t start,end; uint8_t base; } xui_bidi_paragraph_t;
typedef struct xui_bidi_run_t { size_t start,end; uint8_t level; } xui_bidi_run_t;
typedef struct xui_bidi_mirror_t { size_t offset; uint32_t original,mirrored; } xui_bidi_mirror_t;
typedef struct xui_bidi_line_t {
    size_t start,end,count,mirror_count;
    uint8_t base;
    xui_bidi_run_t* runs; /* Physical left-to-right order; reverse codepoints in odd runs. */
    xui_bidi_mirror_t* mirrors;
    uint8_t* levels; /* L1 levels by logical UTF-8 byte, end-start entries. */
} xui_bidi_line_t;
/* Copies valid UTF-8 of at most INT_MAX bytes. All output owners must start
 * empty; free a previous value before reuse. Failed creation publishes NULL,
 * and failed line creation publishes an empty line. */
int xuiInternalTextBidiCreate(const char*,size_t,unsigned,xui_text_bidi*);
/* Valid UTF-8 with no RTL characters or explicit direction controls can keep
 * the existing LTR fast path without allocating a paragraph analysis. */
int xuiInternalTextBidiNeedsAnalysis(const char*,size_t);
void xuiInternalTextBidiFree(xui_text_bidi);
size_t xuiInternalTextBidiRetainedBytes(xui_text_bidi);
size_t xuiInternalTextBidiTextBytes(xui_text_bidi);
const char* xuiInternalTextBidiText(xui_text_bidi);
size_t xuiInternalTextBidiParagraphCount(xui_text_bidi);
int xuiInternalTextBidiParagraph(xui_text_bidi,size_t,xui_bidi_paragraph_t*);
int xuiInternalTextBidiLevel(xui_text_bidi,size_t,uint8_t*);
int xuiInternalTextBidiLine(xui_text_bidi,size_t,size_t,xui_bidi_line_t*);
/* A displayed terminal hyphen makes the preceding WS/BN/format tail no
 * longer trailing. Restore only those bytes in the caller's level array;
 * segment/paragraph separators retain L1. Does not alter the line owner. */
int xuiInternalTextBidiRestoreHyphenLevels(xui_text_bidi,size_t,size_t,uint8_t*);
void xuiInternalTextBidiLineFree(xui_bidi_line_t*);
#endif
