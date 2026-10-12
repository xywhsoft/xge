#ifndef XGE_UNICODE_SCRIPT_H
#define XGE_UNICODE_SCRIPT_H
#include "xge_unicode_grapheme.h"

/* IDs are private to the pinned Unicode data; only ISO 15924 tags cross
 * module boundaries. Empty sets denote unconstrained Common/Inherited/
 * Unknown. Graphemes remain indivisible even for contradictory scripts. */
typedef struct xge_script_set_t { uint64_t words[3]; } xge_script_set_t;
typedef struct xge_script_property_t { uint8_t primary; uint16_t extensions; } xge_script_property_t;
#include "xge_unicode_script_data.inc"

static inline xge_script_property_t __xgeScriptProperty(uint32_t cp)
{
    size_t low = 0, high = sizeof(__xgeScriptRanges) / sizeof(*__xgeScriptRanges);
    while (low < high) {
        size_t mid = low + (high - low) / 2;
        if (cp < __xgeScriptRanges[mid].first) high = mid;
        else if (cp > __xgeScriptRanges[mid].last) low = mid + 1;
        else return (xge_script_property_t){__xgeScriptRanges[mid].primary,
            __xgeScriptRanges[mid].extensions};
    }
    return (xge_script_property_t){2, 0};
}
static inline uint32_t __xgeScriptTag(uint8_t id)
{
    return id < sizeof(__xgeScriptTags) / sizeof(*__xgeScriptTags) ? __xgeScriptTags[id] : __xgeScriptTags[2];
}
static inline int __xgeScriptSetEmpty(xge_script_set_t set)
{ return !(set.words[0] | set.words[1] | set.words[2]); }
static inline int __xgeScriptSetHas(xge_script_set_t set, uint8_t id)
{ return id < 192 && (set.words[id / 64] & (UINT64_C(1) << (id % 64))) != 0; }
static inline xge_script_set_t __xgeScriptSingleton(uint8_t id)
{
    xge_script_set_t set = {{0,0,0}};
    if (id >= 3 && id < 192) set.words[id / 64] = UINT64_C(1) << (id % 64);
    return set;
}
static inline xge_script_set_t __xgeScriptIntersect(xge_script_set_t a, xge_script_set_t b)
{
    unsigned i;
    if (__xgeScriptSetEmpty(a)) return b;
    if (__xgeScriptSetEmpty(b)) return a;
    for (i = 0; i < 3; i++) a.words[i] &= b.words[i];
    return a;
}
static inline uint8_t __xgeScriptChoose(xge_script_set_t set, uint8_t preferred)
{
    unsigned i;
    if (__xgeScriptSetHas(set, preferred)) return preferred;
    for (i = 3; i < sizeof(__xgeScriptTags) / sizeof(*__xgeScriptTags); i++)
        if (__xgeScriptSetHas(set, (uint8_t)i)) return (uint8_t)i;
    return 0;
}
static inline int __xgeScriptBracket(uint32_t cp, uint32_t* pair)
{
    size_t low = 0, high = sizeof(__xgeScriptBrackets) / sizeof(*__xgeScriptBrackets);
    /* Canonically equivalent angle brackets match one another. */
    if (cp == 0x2329u) cp = 0x3008u;
    if (cp == 0x232au) cp = 0x3009u;
    while (low < high) {
        size_t mid = low + (high - low) / 2;
        if (cp < __xgeScriptBrackets[mid].cp) high = mid;
        else if (cp > __xgeScriptBrackets[mid].cp) low = mid + 1;
        else { *pair = __xgeScriptBrackets[mid].pair; return __xgeScriptBrackets[mid].open ? 1 : -1; }
    }
    return 0;
}
typedef uint32_t (*xge_script_decode_proc)(const void*, size_t, size_t*);

/* One byte per UTF-8 byte, filled with the resolved script ID. Allocation is
 * the caller's responsibility. Linear scans; bracket memory is bounded at
 * 128 entries and drops the oldest opening on overflow. Language-sensitive
 * quotes are deliberately not inferred from their Unicode code point. */
static inline int __xgeScriptMap(const unsigned char* text, size_t length,
    unsigned char* map, xge_script_decode_proc decode)
{
    struct { uint32_t closing; size_t offset; } stack[128];
    size_t count = 0, at = 0, run_start = 0;
    uint8_t preferred = 0, previous = 0;
    xge_script_set_t run = {{0,0,0}};
    xge_grapheme_state_t grapheme = {0};
    if (!length) return 1;
    if (!text || !map || !decode) return 0;
    while (at < length) {
        size_t start = at, scan = at;
        uint32_t head = 0, pair = 0;
        uint8_t cluster_preferred = 0;
        xge_script_set_t cluster = {{0,0,0}}, intersection;
        int bracket = 0;
        while (scan < length) {
            size_t next = scan;
            uint32_t cp = decode(text, length, &next);
            xge_grapheme_state_t trial = grapheme;
            xge_script_property_t property;
            xge_script_set_t actual, reduced;
            if (next <= scan || next > length || cp > 0x10ffffu) return 0;
            if (__xgeGraphemePush(&trial, __xgeGraphemeProperty(cp)) && scan > start) break;
            grapheme = trial;
            if (scan == start) { head = cp; bracket = __xgeScriptBracket(cp, &pair); }
            property = __xgeScriptProperty(cp);
            actual = __xgeScriptSets[property.extensions];
            /* Common paired punctuation follows its enclosing text even
             * when its raw SCX set is limited (e.g. CJK angle brackets).
             * Attached marks still constrain this entire grapheme. */
            if (scan == start && bracket && property.primary == 0)
                actual = (xge_script_set_t){{0,0,0}};
            reduced = __xgeScriptIntersect(cluster, actual);
            /* The first constrained scalar owns an incompatible grapheme;
             * compatible attached marks can narrow its extension set. */
            if (!__xgeScriptSetEmpty(reduced)) {
                cluster = reduced;
                if (!__xgeScriptSetHas(cluster, cluster_preferred))
                    cluster_preferred = __xgeScriptChoose(cluster, property.primary);
            }
            scan = next;
        }
        at = scan;
        if (head == '\r' || head == '\n' || head == 0x85u || head == 0x2028u || head == 0x2029u) {
            memset(map + run_start, __xgeScriptChoose(run, preferred), start - run_start);
            memset(map + start, 0, at - start);
            run_start = at; run = (xge_script_set_t){{0,0,0}};
            preferred = previous = 0; count = 0; continue;
        }
        if (bracket < 0) {
            uint32_t closing = head == 0x232au ? 0x3009u : head;
            size_t match = count;
            while (match && stack[match - 1].closing != closing) match--;
            if (match) {
                size_t opening = stack[match - 1].offset;
                if (opening < run_start && map[opening] >= 3) {
                    cluster = __xgeScriptSingleton(map[opening]);
                    cluster_preferred = map[opening];
                }
                count = match - 1;
            }
        }
        intersection = __xgeScriptIntersect(run, cluster);
        if (!__xgeScriptSetEmpty(run) && !__xgeScriptSetEmpty(cluster) && __xgeScriptSetEmpty(intersection)) {
            previous = __xgeScriptChoose(run, preferred);
            memset(map + run_start, previous, start - run_start);
            run_start = start; run = cluster;
            preferred = __xgeScriptChoose(cluster, previous);
            if (!__xgeScriptSetHas(cluster, previous)) preferred = cluster_preferred;
        } else {
            run = intersection;
            if (!__xgeScriptSetHas(run, preferred)) preferred = cluster_preferred;
        }
        if (bracket > 0) {
            if (count == 128) { memmove(stack, stack + 1, 127 * sizeof(*stack)); count--; }
            stack[count].closing = pair; stack[count++].offset = start;
        }
    }
    memset(map + run_start, __xgeScriptChoose(run, preferred), length - run_start);
    return 1;
}
#endif
