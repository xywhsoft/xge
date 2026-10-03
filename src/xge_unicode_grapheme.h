#ifndef XGE_UNICODE_GRAPHEME_H
#define XGE_UNICODE_GRAPHEME_H
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* One allocation-free Unicode 17 extended-grapheme state machine shared by
 * editing, layout, font fallback and GDEF stops. Rules: UAX #29 revision 47. */
enum {
    XGE_GB_OTHER, XGE_GB_CR, XGE_GB_LF, XGE_GB_CONTROL, XGE_GB_EXTEND,
    XGE_GB_ZWJ, XGE_GB_RI, XGE_GB_PREPEND, XGE_GB_SPACING,
    XGE_GB_L, XGE_GB_V, XGE_GB_T, XGE_GB_LV, XGE_GB_LVT
};
enum { XGE_INCB_NONE, XGE_INCB_CONSONANT, XGE_INCB_EXTEND, XGE_INCB_LINKER };
enum { XGE_GRAPHEME_BREAK = 0, XGE_GRAPHEME_NOBREAK = 1, XGE_GRAPHEME_INSIDE = 2 };
typedef struct xge_grapheme_state_t {
    uint8_t previous, initialized, indic, pictograph, regional;
} xge_grapheme_state_t;
#include "xge_unicode_grapheme_data.inc"

static inline uint8_t __xgeGraphemeProperty(uint32_t cp)
{
    size_t low = 0, high = sizeof(__xgeGraphemeRanges) / sizeof(*__xgeGraphemeRanges);
    while (low < high) {
        size_t mid = low + (high - low) / 2;
        if (cp < __xgeGraphemeRanges[mid].first) high = mid;
        else if (cp > __xgeGraphemeRanges[mid].last) low = mid + 1;
        else return __xgeGraphemeRanges[mid].property;
    }
    return 0;
}
static inline int __xgeGraphemeControl(unsigned gb)
{ return gb == XGE_GB_CR || gb == XGE_GB_LF || gb == XGE_GB_CONTROL; }

/* Returns a boundary before right, then advances context by exactly one scalar. */
static inline int __xgeGraphemePush(xge_grapheme_state_t* state, uint8_t property)
{
    unsigned left = state->previous, right = property & 15u, incb = (property >> 4) & 3u;
    int boundary = 1;
    if (!state->initialized) boundary = 1; /* GB1 */
    else if (left == XGE_GB_CR && right == XGE_GB_LF) boundary = 0; /* GB3 */
    else if (__xgeGraphemeControl(left) || __xgeGraphemeControl(right)) boundary = 1; /* GB4/5 */
    else if (left == XGE_GB_L && (right == XGE_GB_L || right == XGE_GB_V || right == XGE_GB_LV || right == XGE_GB_LVT)) boundary = 0; /* GB6 */
    else if ((left == XGE_GB_LV || left == XGE_GB_V) && (right == XGE_GB_V || right == XGE_GB_T)) boundary = 0; /* GB7 */
    else if ((left == XGE_GB_LVT || left == XGE_GB_T) && right == XGE_GB_T) boundary = 0; /* GB8 */
    else if (right == XGE_GB_EXTEND || right == XGE_GB_ZWJ || right == XGE_GB_SPACING || left == XGE_GB_PREPEND) boundary = 0; /* GB9/9a/9b */
    else if (incb == XGE_INCB_CONSONANT && state->indic == 2) boundary = 0; /* GB9c */
    else if ((property & 64u) && state->pictograph == 2) boundary = 0; /* GB11 */
    else if (left == XGE_GB_RI && right == XGE_GB_RI && state->regional) boundary = 0; /* GB12/13 */
    state->regional = right == XGE_GB_RI ? (uint8_t)(left == XGE_GB_RI ? !state->regional : 1) : 0;
    if (incb == XGE_INCB_CONSONANT) state->indic = 1;
    else if (incb == XGE_INCB_LINKER) state->indic = state->indic ? 2 : 0;
    else if (incb != XGE_INCB_EXTEND) state->indic = 0;
    if (right == XGE_GB_ZWJ) state->pictograph = state->pictograph == 1 ? 2 : 0;
    else if (right == XGE_GB_EXTEND) state->pictograph = state->pictograph == 1 ? 1 : 0;
    else state->pictograph = (property & 64u) ? 1 : 0;
    state->previous = (uint8_t)right; state->initialized = 1;
    return boundary;
}
typedef uint32_t (*xge_grapheme_decode_proc)(const void*, size_t, size_t*);

/* Boundaries after code units, matching the existing private break-map format. */
static inline void __xgeGraphemeMap(const void* text, size_t length, char* map,
    xge_grapheme_decode_proc decode)
{
    size_t at = 0, previous_end = 0;
    xge_grapheme_state_t state = {0};
    if (!length) return;
    memset(map, XGE_GRAPHEME_INSIDE, length);
    while (at < length) {
        uint32_t cp = decode(text, length, &at);
        int boundary;
        if (at <= previous_end || at > length) return;
        boundary = __xgeGraphemePush(&state, __xgeGraphemeProperty(cp));
        if (previous_end) map[previous_end - 1] = boundary ? XGE_GRAPHEME_BREAK : XGE_GRAPHEME_NOBREAK;
        previous_end = at;
    }
    map[previous_end - 1] = XGE_GRAPHEME_BREAK; /* GB2 */
}
#endif
