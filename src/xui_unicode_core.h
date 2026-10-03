#ifndef XUI_UNICODE_CORE_H
#define XUI_UNICODE_CORE_H
#include <stdint.h>

/* Window-independent UTF-8 navigation shared by Document and XUI widgets. */
typedef int (*xui_internal_text_read_proc)(void* user, int offset, unsigned char* byte);

typedef enum xui_internal_word_policy_t {
    XUI_INTERNAL_WORD_NATURAL = 0,
    XUI_INTERNAL_WORD_IDENTIFIER,
    XUI_INTERNAL_WORD_TERMINAL
} xui_internal_word_policy_t;

typedef enum xui_internal_word_kind_t {
    XUI_INTERNAL_WORD_SPACE = 0,
    XUI_INTERNAL_WORD_TEXT,
    XUI_INTERNAL_WORD_SYMBOL
} xui_internal_word_kind_t;

int xuiInternalTextGraphemeNextRead(xui_internal_text_read_proc onRead, void* user, int length, int offset);
int xuiInternalTextGraphemePrevRead(xui_internal_text_read_proc onRead, void* user, int length, int offset);
int xuiInternalTextGraphemeClampRead(xui_internal_text_read_proc onRead, void* user, int length, int offset);
int xuiInternalTextGraphemeNext(const char* text, int length, int offset);
int xuiInternalTextGraphemePrev(const char* text, int length, int offset);
int xuiInternalTextGraphemeClamp(const char* text, int length, int offset);
int xuiInternalTextWordBoundaryRead(xui_internal_text_read_proc onRead, void* user,
    int length, int offset, xui_internal_word_policy_t policy);
xui_internal_word_kind_t xuiInternalTextWordRangeRead(xui_internal_text_read_proc onRead,
    void* user, int length, int offset, xui_internal_word_policy_t policy,
    int* start, int* end);
int xuiInternalTextWordPrevRead(xui_internal_text_read_proc onRead, void* user,
    int length, int offset, xui_internal_word_policy_t policy);
int xuiInternalTextWordNextRead(xui_internal_text_read_proc onRead, void* user,
    int length, int offset, xui_internal_word_policy_t policy);
int xuiInternalTextWordBoundary(const char* text, int length, int offset,
    xui_internal_word_policy_t policy);
xui_internal_word_kind_t xuiInternalTextWordRange(const char* text, int length,
    int offset, xui_internal_word_policy_t policy, int* start, int* end);
int xuiInternalTextWordPrev(const char* text, int length, int offset,
    xui_internal_word_policy_t policy);
int xuiInternalTextWordNext(const char* text, int length, int offset,
    xui_internal_word_policy_t policy);

#endif
