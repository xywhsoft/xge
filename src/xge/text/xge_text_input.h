#ifndef XGE_TEXT_INPUT_H
#define XGE_TEXT_INPUT_H
#include <stdint.h>
#include <limits.h>
#include <string.h>

static inline unsigned char __xgeLanguageLower(unsigned char c)
{ return c >= 'A' && c <= 'Z' ? (unsigned char)(c + 32) : c; }
static inline int __xgeLanguageAlpha(unsigned char c)
{ c = __xgeLanguageLower(c); return c >= 'a' && c <= 'z'; }
static inline int __xgeLanguageDigit(unsigned char c)
{ return c >= '0' && c <= '9'; }
static inline int __xgeLanguageEqual(const char* a, const char* b, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) if (__xgeLanguageLower((unsigned char)a[i]) != __xgeLanguageLower((unsigned char)b[i])) return 0;
    return 1;
}
/* RFC 5646 grammar plus duplicate variant/extension rejection; registry
 * membership is deliberately not inferred. Bounded stack, no allocations. */
static inline int __xgeTextLanguageValid(const char* text)
{
    static const char* const grandfathered[] = {
        "en-GB-oed","i-ami","i-bnn","i-default","i-enochian","i-hak","i-klingon","i-lux","i-mingo",
        "i-navajo","i-pwn","i-tao","i-tay","i-tsu","sgn-BE-FR","sgn-BE-NL","sgn-CH-DE",
        "art-lojban","cel-gaulish","no-bok","no-nyn","zh-guoyu","zh-hakka","zh-min","zh-min-nan","zh-xiang"
    };
    struct { const char* text; unsigned length; int alpha, digit; } parts[128];
    size_t bytes = 0, count = 0, at = 0, i, variants;
    uint64_t extensions = 0;
    if (!text) return 1;
    while (text[bytes] && bytes < 255) bytes++;
    if (!bytes || text[bytes]) return 0;
    for (i = 0; i < sizeof(grandfathered) / sizeof(*grandfathered); i++)
        if (strlen(grandfathered[i]) == bytes && __xgeLanguageEqual(text, grandfathered[i], bytes)) return 1;
    while (at < bytes) {
        size_t start = at; int alpha = 1, digit = 1;
        while (at < bytes && text[at] != '-') {
            unsigned char c = (unsigned char)text[at++];
            if (!__xgeLanguageAlpha(c)) alpha = 0;
            if (!__xgeLanguageDigit(c)) digit = 0;
            if (!__xgeLanguageAlpha(c) && !__xgeLanguageDigit(c)) return 0;
        }
        if (at == start || at - start > 8 || count == 128) return 0;
        parts[count].text = text + start; parts[count].length = (unsigned)(at - start);
        parts[count].alpha = alpha; parts[count++].digit = digit;
        if (at < bytes && ++at == bytes) return 0;
    }
    if (!count) return 0;
    if (parts[0].length == 1 && __xgeLanguageLower((unsigned char)parts[0].text[0]) == 'x') return count > 1;
    if (!parts[0].alpha || parts[0].length < 2) return 0;
    at = 1;
    if (parts[0].length <= 3) {
        unsigned extlang = 0;
        while (at < count && extlang < 3 && parts[at].length == 3 && parts[at].alpha) { at++; extlang++; }
    }
    if (at < count && parts[at].length == 4 && parts[at].alpha) at++;
    if (at < count && ((parts[at].length == 2 && parts[at].alpha) || (parts[at].length == 3 && parts[at].digit))) at++;
    variants = at;
    while (at < count && (parts[at].length >= 5 || (parts[at].length == 4 && __xgeLanguageDigit((unsigned char)parts[at].text[0])))) {
        for (i = variants; i < at; i++) if (parts[i].length == parts[at].length && __xgeLanguageEqual(parts[i].text, parts[at].text, parts[i].length)) return 0;
        at++;
    }
    while (at < count && parts[at].length == 1 && __xgeLanguageLower((unsigned char)parts[at].text[0]) != 'x') {
        unsigned char c = __xgeLanguageLower((unsigned char)parts[at].text[0]);
        unsigned index = __xgeLanguageDigit(c) ? c - '0' : c - 'a' + 10;
        size_t start;
        if (extensions & (UINT64_C(1) << index)) return 0;
        extensions |= UINT64_C(1) << index; start = ++at;
        while (at < count && parts[at].length >= 2) at++;
        if (start == at) return 0;
    }
    if (at < count && parts[at].length == 1 && __xgeLanguageLower((unsigned char)parts[at].text[0]) == 'x') return at + 1 < count;
    return at == count;
}
static inline int __xgeTextScriptValid(uint32_t script)
{
    unsigned i;
    if (script) for (i=0;i<4;i++) if (!__xgeLanguageAlpha((unsigned char)(script >> (i*8)))) return 0;
    return 1;
}
/* Caller supplies valid UTF-8. Validate bounded item/context byte equality
 * and scalar boundaries without decoding the full paragraph per item. */
static inline int __xgeTextInputRange(const char* text, int bytes, const char* context,
    int length, int offset, int* context_bytes)
{
    if (!text || bytes < 0) return 0;
    if (!context) {
        if (length || offset) return 0;
        *context_bytes=bytes;return 1;
    }
    if (length < -1) return 0;
    if (length == -1) { size_t n=strlen(context);if(n>INT_MAX)return 0;length=(int)n; }
    if (offset < 0 || bytes > length || offset > length-bytes) return 0;
    if ((offset < length && ((unsigned char)context[offset] & 0xc0)==0x80) ||
        (offset+bytes < length && ((unsigned char)context[offset+bytes] & 0xc0)==0x80) ||
        memcmp(context+offset,text,(size_t)bytes)) return 0;
    *context_bytes=length;return 1;
}
#endif
