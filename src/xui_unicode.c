#include "../xui_config.h"
#if XGE_ENABLE_XUI
#include "xui_unicode_core.h"

#include <string.h>
#include <limits.h>

#include "xge_unicode_grapheme.h"

typedef enum xui_word_break_property_t {
	XUI_WB_OTHER = 0,
	XUI_WB_ALETTER,
	XUI_WB_CR,
	XUI_WB_DOUBLE_QUOTE,
	XUI_WB_EXTEND,
	XUI_WB_EXTEND_NUM_LET,
	XUI_WB_FORMAT,
	XUI_WB_HEBREW_LETTER,
	XUI_WB_KATAKANA,
	XUI_WB_LF,
	XUI_WB_MID_LETTER,
	XUI_WB_MID_NUM,
	XUI_WB_MID_NUM_LET,
	XUI_WB_NEWLINE,
	XUI_WB_NUMERIC,
	XUI_WB_REGIONAL,
	XUI_WB_SINGLE_QUOTE,
	XUI_WB_WSEG_SPACE,
	XUI_WB_ZWJ
} xui_word_break_property_t;

typedef struct xui_unicode_word_range_t {
	uint32_t iFirst;
	uint32_t iLast;
	uint8_t iProperty;
} xui_unicode_word_range_t;

typedef struct xui_unicode_binary_range_t {
	uint32_t iFirst;
	uint32_t iLast;
} xui_unicode_binary_range_t;

typedef struct xui_unicode_word_unit_t {
	int64_t iStart;
	int64_t iEnd;
	uint32_t iCodepoint;
	xui_word_break_property_t iProperty;
	int bXidContinue;
} xui_unicode_word_unit_t;

#include "xui_unicode_word_data.inc"

static int __xuiUnicodeRange(uint32_t iCodepoint, uint32_t iFirst, uint32_t iLast)
{
	return iCodepoint >= iFirst && iCodepoint <= iLast;
}

static int __xuiUnicodePointerRead(void* pUser, int64_t iOffset, unsigned char* pByte)
{
	const unsigned char* pText;

	if ( pUser == NULL || pByte == NULL || iOffset < 0 ) return 0;
	pText = (const unsigned char*)pUser;
	*pByte = pText[iOffset];
	return 1;
}

static int __xuiUnicodeDecode(xui_internal_text_read64_proc onRead, void* pUser,
	int64_t iLength, int64_t iOffset, uint32_t* pCodepoint, int64_t* pNext)
{
	unsigned char c0;
	unsigned char c1;
	unsigned char c2;
	unsigned char c3;
	uint32_t iCodepoint;
	int iStep;

	if ( pCodepoint != NULL ) *pCodepoint = 0xFFFDu;
	if ( pNext != NULL ) *pNext = iOffset;
	if ( onRead == NULL || iOffset < 0 || iOffset >= iLength ||
	     !onRead(pUser, iOffset, &c0) ) return 0;
	iCodepoint = 0xFFFDu;
	iStep = 1;
	if ( c0 < 0x80u ) {
		iCodepoint = c0;
	} else if ( (c0 & 0xE0u) == 0xC0u && iLength - iOffset > 1 &&
	            onRead(pUser, iOffset + 1, &c1) && (c1 & 0xC0u) == 0x80u ) {
		iCodepoint = ((uint32_t)(c0 & 0x1Fu) << 6) | (uint32_t)(c1 & 0x3Fu);
		if ( iCodepoint >= 0x80u ) iStep = 2;
		else iCodepoint = 0xFFFDu;
	} else if ( (c0 & 0xF0u) == 0xE0u && iLength - iOffset > 2 &&
	            onRead(pUser, iOffset + 1, &c1) && onRead(pUser, iOffset + 2, &c2) &&
	            (c1 & 0xC0u) == 0x80u && (c2 & 0xC0u) == 0x80u ) {
		iCodepoint = ((uint32_t)(c0 & 0x0Fu) << 12) |
			((uint32_t)(c1 & 0x3Fu) << 6) | (uint32_t)(c2 & 0x3Fu);
		if ( iCodepoint >= 0x800u && !__xuiUnicodeRange(iCodepoint, 0xD800u, 0xDFFFu) ) iStep = 3;
		else iCodepoint = 0xFFFDu;
	} else if ( (c0 & 0xF8u) == 0xF0u && iLength - iOffset > 3 &&
	            onRead(pUser, iOffset + 1, &c1) && onRead(pUser, iOffset + 2, &c2) &&
	            onRead(pUser, iOffset + 3, &c3) &&
	            (c1 & 0xC0u) == 0x80u && (c2 & 0xC0u) == 0x80u && (c3 & 0xC0u) == 0x80u ) {
		iCodepoint = ((uint32_t)(c0 & 0x07u) << 18) |
			((uint32_t)(c1 & 0x3Fu) << 12) |
			((uint32_t)(c2 & 0x3Fu) << 6) | (uint32_t)(c3 & 0x3Fu);
		if ( iCodepoint >= 0x10000u && iCodepoint <= 0x10FFFFu ) iStep = 4;
		else iCodepoint = 0xFFFDu;
	}
	if ( pCodepoint != NULL ) *pCodepoint = iCodepoint;
	if ( pNext != NULL ) *pNext = iOffset + iStep;
	return 1;
}

static int64_t __xuiUnicodePrevCodepoint(xui_internal_text_read64_proc onRead, void* pUser,
	int64_t iLength, int64_t iOffset, uint32_t* pCodepoint)
{
	unsigned char c;
	int64_t iStart;
	int64_t iNext;

	if ( iOffset <= 0 ) return 0;
	if ( iOffset > iLength ) iOffset = iLength;
	iStart = iOffset - 1;
	while ( iStart > 0 && iOffset - iStart < 4 && onRead(pUser, iStart, &c) && (c & 0xC0u) == 0x80u ) iStart--;
	if ( !__xuiUnicodeDecode(onRead, pUser, iLength, iStart, pCodepoint, &iNext) ) return iOffset - 1;
	if ( iNext < iOffset ) {
		if ( pCodepoint != NULL ) *pCodepoint = 0xFFFDu;
		return iOffset - 1;
	}
	return iStart;
}

/* Recover only the prefix context required by GB9c/GB11/GB12-13, then
 * use the same state transition as the forward map. Ordinary text queries
 * never rescan the paragraph. No allocation or contiguous input is needed. */
static int __xuiUnicodeGraphemeBoundary(xui_internal_text_read64_proc read,
    void* user, int64_t length, int64_t right_at, uint8_t right)
{
    xge_grapheme_state_t state = {0};
    uint32_t cp; uint8_t left; int64_t at, scan;
    if (right_at <= 0) return 1;
    at = __xuiUnicodePrevCodepoint(read, user, length, right_at, &cp);
    left = __xgeGraphemeProperty(cp);
    __xgeGraphemePush(&state, left);
    if ((left & 15u) == XGE_GB_RI && (right & 15u) == XGE_GB_RI) {
        scan = at;
        while (scan > 0) {
            scan = __xuiUnicodePrevCodepoint(read, user, length, scan, &cp);
            if ((__xgeGraphemeProperty(cp) & 15u) != XGE_GB_RI) break;
            state.regional = (uint8_t)!state.regional;
        }
    }
    if ((left & 15u) == XGE_GB_ZWJ && (right & 64u)) {
        scan = at;
        while (scan > 0) {
            uint8_t property;
            scan = __xuiUnicodePrevCodepoint(read, user, length, scan, &cp);
            property = __xgeGraphemeProperty(cp);
            if ((property & 15u) == XGE_GB_EXTEND) continue;
            state.pictograph = (property & 64u) ? 2 : 0;
            break;
        }
    }
    if (((right >> 4) & 3u) == XGE_INCB_CONSONANT &&
        (((left >> 4) & 3u) == XGE_INCB_EXTEND || ((left >> 4) & 3u) == XGE_INCB_LINKER)) {
        unsigned incb = (left >> 4) & 3u; int linker = 0;
        scan = at;
        for (;;) {
            if (incb == XGE_INCB_LINKER) linker = 1;
            else if (incb != XGE_INCB_EXTEND) {
                state.indic = incb == XGE_INCB_CONSONANT ? (uint8_t)(linker ? 2 : 1) : 0;
                break;
            }
            if (scan <= 0) { state.indic = 0; break; }
            scan = __xuiUnicodePrevCodepoint(read, user, length, scan, &cp);
            incb = (__xgeGraphemeProperty(cp) >> 4) & 3u;
        }
    }
    return __xgeGraphemePush(&state, right);
}

int64_t xuiInternalTextGraphemePrev64Read(xui_internal_text_read64_proc read,
    void* user, int64_t length, int64_t offset)
{
    uint32_t cp; int64_t start;
    if (!read || length <= 0 || offset <= 0) return 0;
    if (offset > length) offset = length;
    start = __xuiUnicodePrevCodepoint(read, user, length, offset, &cp);
    while (start > 0 && !__xuiUnicodeGraphemeBoundary(read, user, length,
        start, __xgeGraphemeProperty(cp)))
        start = __xuiUnicodePrevCodepoint(read, user, length, start, &cp);
    return start;
}

int64_t xuiInternalTextGraphemeNext64Read(xui_internal_text_read64_proc read,
    void* user, int64_t length, int64_t offset)
{
    xge_grapheme_state_t state = {0};
    uint32_t cp; int64_t at, end;
    if (!read || length <= 0) return 0;
    if (offset < 0) offset = 0;
    if (offset >= length) return length;
    /* Seed from the actual cluster head, including offsets inside UTF-8 or
     * inside a conjunct. Starting a fresh state at the queried byte is wrong. */
    at = __xuiUnicodePrevCodepoint(read, user, length, offset + 1, &cp);
    if (!__xuiUnicodeDecode(read, user, length, at, &cp, &end)) return offset;
    at = xuiInternalTextGraphemePrev64Read(read, user, length, end);
    while (at < length) {
        if (!__xuiUnicodeDecode(read, user, length, at, &cp, &end)) return at;
        if (__xgeGraphemePush(&state, __xgeGraphemeProperty(cp)) &&
            at > offset) return at;
        at = end;
    }
    return length;
}

int64_t xuiInternalTextGraphemeClamp64Read(xui_internal_text_read64_proc read,
    void* user, int64_t length, int64_t offset)
{
    int64_t previous, next;
    if (!read || length <= 0 || offset <= 0) return 0;
    if (offset >= length) return length;
    previous = xuiInternalTextGraphemePrev64Read(read, user, length, offset);
    next = xuiInternalTextGraphemeNext64Read(read, user, length, previous);
    return next == offset ? offset : previous;
}







static xui_word_break_property_t __xuiUnicodeWordProperty(uint32_t iCodepoint)
{
	int iLow;
	int iHigh;

	iLow = 0;
	iHigh = (int)(sizeof(__xuiUnicodeWordRanges) / sizeof(__xuiUnicodeWordRanges[0])) - 1;
	while ( iLow <= iHigh ) {
		int iMiddle = iLow + (iHigh - iLow) / 2;
		const xui_unicode_word_range_t* pRange = &__xuiUnicodeWordRanges[iMiddle];
		if ( iCodepoint < pRange->iFirst ) {
			iHigh = iMiddle - 1;
		} else if ( iCodepoint > pRange->iLast ) {
			iLow = iMiddle + 1;
		} else {
			return (xui_word_break_property_t)pRange->iProperty;
		}
	}
	return XUI_WB_OTHER;
}

static int __xuiUnicodeXidContinue(uint32_t iCodepoint)
{
	int iLow;
	int iHigh;

	iLow = 0;
	iHigh = (int)(sizeof(__xuiUnicodeXidContinueRanges) /
		sizeof(__xuiUnicodeXidContinueRanges[0])) - 1;
	while ( iLow <= iHigh ) {
		int iMiddle = iLow + (iHigh - iLow) / 2;
		const xui_unicode_binary_range_t* pRange = &__xuiUnicodeXidContinueRanges[iMiddle];
		if ( iCodepoint < pRange->iFirst ) {
			iHigh = iMiddle - 1;
		} else if ( iCodepoint > pRange->iLast ) {
			iLow = iMiddle + 1;
		} else {
			return 1;
		}
	}
	return 0;
}

static int __xuiUnicodeWordUnitRead(xui_internal_text_read64_proc onRead, void* pUser,
	int64_t iLength, int64_t iOffset, xui_unicode_word_unit_t* pUnit)
{
	uint32_t iCodepoint;
	int64_t iScalarEnd;

	if ( onRead == NULL || pUnit == NULL || iOffset < 0 || iOffset >= iLength ) return 0;
	if ( !__xuiUnicodeDecode(onRead, pUser, iLength, iOffset, &iCodepoint, &iScalarEnd) ) return 0;
	pUnit->iStart = iOffset;
	pUnit->iEnd = xuiInternalTextGraphemeNext64Read(onRead, pUser, iLength, iOffset);
	if ( pUnit->iEnd <= iOffset ) pUnit->iEnd = iScalarEnd;
	pUnit->iCodepoint = iCodepoint;
	pUnit->iProperty = __xuiUnicodeWordProperty(iCodepoint);
	pUnit->bXidContinue = __xuiUnicodeXidContinue(iCodepoint);
	return 1;
}

static int __xuiUnicodeWordUnitBefore(xui_internal_text_read64_proc onRead, void* pUser,
	int64_t iLength, int64_t iOffset, xui_unicode_word_unit_t* pUnit)
{
	int64_t iStart;
	if ( iOffset <= 0 ) return 0;
	iStart = xuiInternalTextGraphemePrev64Read(onRead, pUser, iLength, iOffset);
	return __xuiUnicodeWordUnitRead(onRead, pUser, iLength, iStart, pUnit);
}

static int __xuiUnicodeWordIgnored(xui_word_break_property_t iProperty)
{
	return iProperty == XUI_WB_EXTEND || iProperty == XUI_WB_FORMAT || iProperty == XUI_WB_ZWJ;
}

static int __xuiUnicodeWordNewline(xui_word_break_property_t iProperty)
{
	return iProperty == XUI_WB_CR || iProperty == XUI_WB_LF || iProperty == XUI_WB_NEWLINE;
}

static int __xuiUnicodeWordAhLetter(xui_word_break_property_t iProperty)
{
	return iProperty == XUI_WB_ALETTER || iProperty == XUI_WB_HEBREW_LETTER;
}

static int __xuiUnicodeWordMidLetter(xui_word_break_property_t iProperty)
{
	return iProperty == XUI_WB_MID_LETTER || iProperty == XUI_WB_MID_NUM_LET ||
		iProperty == XUI_WB_SINGLE_QUOTE;
}

static int __xuiUnicodeWordMidNumeric(xui_word_break_property_t iProperty)
{
	return iProperty == XUI_WB_MID_NUM || iProperty == XUI_WB_MID_NUM_LET ||
		iProperty == XUI_WB_SINGLE_QUOTE;
}

static int __xuiUnicodeWordWhitespace(uint32_t iCodepoint)
{
	return (iCodepoint >= 0x0009u && iCodepoint <= 0x000Du) || iCodepoint == 0x0020u ||
		iCodepoint == 0x0085u || iCodepoint == 0x00A0u || iCodepoint == 0x1680u ||
		(iCodepoint >= 0x2000u && iCodepoint <= 0x200Au) || iCodepoint == 0x2028u ||
		iCodepoint == 0x2029u || iCodepoint == 0x202Fu || iCodepoint == 0x205Fu ||
		iCodepoint == 0x3000u;
}

static int __xuiUnicodeWordTerminalExtra(uint32_t iCodepoint)
{
	return iCodepoint == '-' || iCodepoint == '.' || iCodepoint == '/' || iCodepoint == '\\' ||
		iCodepoint == ':' || iCodepoint == '@';
}

static int __xuiUnicodeWordPolicyUnit(const xui_unicode_word_unit_t* pUnit,
	xui_internal_word_policy_t iPolicy)
{
	if ( pUnit == NULL ) return 0;
	if ( iPolicy == XUI_INTERNAL_WORD_IDENTIFIER ) return pUnit->bXidContinue;
	if ( iPolicy == XUI_INTERNAL_WORD_TERMINAL ) {
		return pUnit->bXidContinue || __xuiUnicodeWordTerminalExtra(pUnit->iCodepoint);
	}
	return __xuiUnicodeWordAhLetter(pUnit->iProperty) ||
		pUnit->iProperty == XUI_WB_NUMERIC || pUnit->iProperty == XUI_WB_KATAKANA ||
		pUnit->iProperty == XUI_WB_EXTEND_NUM_LET || pUnit->bXidContinue;
}

static int __xuiUnicodeWordPreviousSignificant(xui_internal_text_read64_proc onRead,
	void* pUser, int64_t iLength, int64_t iOffset, xui_unicode_word_unit_t* pUnit)
{
	xui_unicode_word_unit_t tUnit;
	while ( __xuiUnicodeWordUnitBefore(onRead, pUser, iLength, iOffset, &tUnit) ) {
		if ( !__xuiUnicodeWordIgnored(tUnit.iProperty) ) {
			if ( pUnit != NULL ) *pUnit = tUnit;
			return 1;
		}
		iOffset = tUnit.iStart;
	}
	return 0;
}

static int __xuiUnicodeWordNextSignificant(xui_internal_text_read64_proc onRead,
	void* pUser, int64_t iLength, int64_t iOffset, xui_unicode_word_unit_t* pUnit)
{
	xui_unicode_word_unit_t tUnit;
	while ( iOffset < iLength && __xuiUnicodeWordUnitRead(onRead, pUser, iLength, iOffset, &tUnit) ) {
		if ( !__xuiUnicodeWordIgnored(tUnit.iProperty) ) {
			if ( pUnit != NULL ) *pUnit = tUnit;
			return 1;
		}
		if ( tUnit.iEnd <= iOffset ) break;
		iOffset = tUnit.iEnd;
	}
	return 0;
}

static int __xuiUnicodeWordNaturalBoundary(xui_internal_text_read64_proc onRead,
	void* pUser, int64_t iLength, int64_t iOffset)
{
	xui_unicode_word_unit_t tImmediateLeft;
	xui_unicode_word_unit_t tImmediateRight;
	xui_unicode_word_unit_t tLeft;
	xui_unicode_word_unit_t tRight;
	xui_unicode_word_unit_t tOuter;
	int64_t iRegionalCount;
	int64_t iAt;

	if ( !__xuiUnicodeWordUnitBefore(onRead, pUser, iLength, iOffset, &tImmediateLeft) ||
	     !__xuiUnicodeWordUnitRead(onRead, pUser, iLength, iOffset, &tImmediateRight) ) return 1;
	if ( tImmediateLeft.iProperty == XUI_WB_CR && tImmediateRight.iProperty == XUI_WB_LF ) return 0;
	if ( __xuiUnicodeWordNewline(tImmediateLeft.iProperty) ||
	     __xuiUnicodeWordNewline(tImmediateRight.iProperty) ) return 1;
	if ( tImmediateLeft.iProperty == XUI_WB_WSEG_SPACE &&
	     tImmediateRight.iProperty == XUI_WB_WSEG_SPACE ) return 0;
	if ( __xuiUnicodeWordIgnored(tImmediateRight.iProperty) ) return 0;
	if ( !__xuiUnicodeWordPreviousSignificant(onRead, pUser, iLength, iOffset, &tLeft) ||
	     !__xuiUnicodeWordNextSignificant(onRead, pUser, iLength, iOffset, &tRight) ) return 1;
	if ( __xuiUnicodeWordNewline(tLeft.iProperty) || __xuiUnicodeWordNewline(tRight.iProperty) ) return 1;
	if ( __xuiUnicodeWordAhLetter(tLeft.iProperty) && __xuiUnicodeWordAhLetter(tRight.iProperty) ) return 0;
	if ( __xuiUnicodeWordAhLetter(tLeft.iProperty) && __xuiUnicodeWordMidLetter(tRight.iProperty) &&
	     __xuiUnicodeWordNextSignificant(onRead, pUser, iLength, tRight.iEnd, &tOuter) &&
	     __xuiUnicodeWordAhLetter(tOuter.iProperty) ) return 0;
	if ( __xuiUnicodeWordMidLetter(tLeft.iProperty) && __xuiUnicodeWordAhLetter(tRight.iProperty) &&
	     __xuiUnicodeWordPreviousSignificant(onRead, pUser, iLength, tLeft.iStart, &tOuter) &&
	     __xuiUnicodeWordAhLetter(tOuter.iProperty) ) return 0;
	if ( tLeft.iProperty == XUI_WB_HEBREW_LETTER && tRight.iProperty == XUI_WB_SINGLE_QUOTE ) return 0;
	if ( tLeft.iProperty == XUI_WB_HEBREW_LETTER && tRight.iProperty == XUI_WB_DOUBLE_QUOTE &&
	     __xuiUnicodeWordNextSignificant(onRead, pUser, iLength, tRight.iEnd, &tOuter) &&
	     tOuter.iProperty == XUI_WB_HEBREW_LETTER ) return 0;
	if ( tLeft.iProperty == XUI_WB_DOUBLE_QUOTE && tRight.iProperty == XUI_WB_HEBREW_LETTER &&
	     __xuiUnicodeWordPreviousSignificant(onRead, pUser, iLength, tLeft.iStart, &tOuter) &&
	     tOuter.iProperty == XUI_WB_HEBREW_LETTER ) return 0;
	if ( tLeft.iProperty == XUI_WB_NUMERIC && tRight.iProperty == XUI_WB_NUMERIC ) return 0;
	if ( __xuiUnicodeWordAhLetter(tLeft.iProperty) && tRight.iProperty == XUI_WB_NUMERIC ) return 0;
	if ( tLeft.iProperty == XUI_WB_NUMERIC && __xuiUnicodeWordAhLetter(tRight.iProperty) ) return 0;
	if ( tLeft.iProperty == XUI_WB_NUMERIC && __xuiUnicodeWordMidNumeric(tRight.iProperty) &&
	     __xuiUnicodeWordNextSignificant(onRead, pUser, iLength, tRight.iEnd, &tOuter) &&
	     tOuter.iProperty == XUI_WB_NUMERIC ) return 0;
	if ( __xuiUnicodeWordMidNumeric(tLeft.iProperty) && tRight.iProperty == XUI_WB_NUMERIC &&
	     __xuiUnicodeWordPreviousSignificant(onRead, pUser, iLength, tLeft.iStart, &tOuter) &&
	     tOuter.iProperty == XUI_WB_NUMERIC ) return 0;
	if ( tLeft.iProperty == XUI_WB_KATAKANA && tRight.iProperty == XUI_WB_KATAKANA ) return 0;
	if ( (__xuiUnicodeWordAhLetter(tLeft.iProperty) || tLeft.iProperty == XUI_WB_NUMERIC ||
	      tLeft.iProperty == XUI_WB_KATAKANA || tLeft.iProperty == XUI_WB_EXTEND_NUM_LET) &&
	     tRight.iProperty == XUI_WB_EXTEND_NUM_LET ) return 0;
	if ( tLeft.iProperty == XUI_WB_EXTEND_NUM_LET &&
	     (__xuiUnicodeWordAhLetter(tRight.iProperty) || tRight.iProperty == XUI_WB_NUMERIC ||
	      tRight.iProperty == XUI_WB_KATAKANA) ) return 0;
	if ( tLeft.iProperty != XUI_WB_REGIONAL || tRight.iProperty != XUI_WB_REGIONAL ) return 1;
	iRegionalCount = 0;
	iAt = iOffset;
	while ( __xuiUnicodeWordPreviousSignificant(onRead, pUser, iLength, iAt, &tOuter) &&
	        tOuter.iProperty == XUI_WB_REGIONAL ) {
		iRegionalCount++;
		iAt = tOuter.iStart;
	}
	return (iRegionalCount & 1) == 0;
}

int xuiInternalTextWordBoundary64Read(xui_internal_text_read64_proc onRead, void* pUser,
	int64_t iLength, int64_t iOffset, xui_internal_word_policy_t iPolicy)
{
	xui_unicode_word_unit_t tImmediateLeft;
	xui_unicode_word_unit_t tImmediateRight;
	xui_unicode_word_unit_t tLeft;
	xui_unicode_word_unit_t tRight;

	if ( onRead == NULL || iLength <= 0 || iOffset <= 0 || iOffset >= iLength ) return 1;
	iOffset = xuiInternalTextGraphemeClamp64Read(onRead, pUser, iLength, iOffset);
	if ( iOffset <= 0 || iOffset >= iLength ) return 1;
	if ( iPolicy == XUI_INTERNAL_WORD_NATURAL ) {
		return __xuiUnicodeWordNaturalBoundary(onRead, pUser, iLength, iOffset);
	}
	if ( !__xuiUnicodeWordUnitBefore(onRead, pUser, iLength, iOffset, &tImmediateLeft) ||
	     !__xuiUnicodeWordUnitRead(onRead, pUser, iLength, iOffset, &tImmediateRight) ) return 1;
	if ( tImmediateLeft.iProperty == XUI_WB_CR && tImmediateRight.iProperty == XUI_WB_LF ) return 0;
	if ( __xuiUnicodeWordNewline(tImmediateLeft.iProperty) ||
	     __xuiUnicodeWordNewline(tImmediateRight.iProperty) ||
	     __xuiUnicodeWordWhitespace(tImmediateLeft.iCodepoint) ||
	     __xuiUnicodeWordWhitespace(tImmediateRight.iCodepoint) ) return 1;
	if ( __xuiUnicodeWordIgnored(tImmediateRight.iProperty) ) return 0;
	if ( !__xuiUnicodeWordPreviousSignificant(onRead, pUser, iLength, iOffset, &tLeft) ||
	     !__xuiUnicodeWordNextSignificant(onRead, pUser, iLength, iOffset, &tRight) ) return 1;
	return !(__xuiUnicodeWordPolicyUnit(&tLeft, iPolicy) &&
		__xuiUnicodeWordPolicyUnit(&tRight, iPolicy));
}

xui_internal_word_kind_t xuiInternalTextWordRange64Read(xui_internal_text_read64_proc onRead,
	void* pUser, int64_t iLength, int64_t iOffset, xui_internal_word_policy_t iPolicy,
	int64_t* pStart, int64_t* pEnd)
{
	xui_unicode_word_unit_t tUnit;
	xui_unicode_word_unit_t tRangeUnit;
	int64_t iStart;
	int64_t iEnd;
	int64_t iAt;

	if ( pStart != NULL ) *pStart = 0;
	if ( pEnd != NULL ) *pEnd = 0;
	if ( onRead == NULL || iLength <= 0 ) return XUI_INTERNAL_WORD_SPACE;
	iOffset = xuiInternalTextGraphemeClamp64Read(onRead, pUser, iLength, iOffset);
	if ( iOffset >= iLength ) iOffset = xuiInternalTextGraphemePrev64Read(onRead, pUser, iLength, iLength);
	if ( !__xuiUnicodeWordUnitRead(onRead, pUser, iLength, iOffset, &tUnit) ) return XUI_INTERNAL_WORD_SPACE;
	if ( __xuiUnicodeWordWhitespace(tUnit.iCodepoint) ) {
		if ( pStart != NULL ) *pStart = iOffset;
		if ( pEnd != NULL ) *pEnd = iOffset;
		return XUI_INTERNAL_WORD_SPACE;
	}
	iStart = tUnit.iStart;
	iEnd = tUnit.iEnd;
	while ( iStart > 0 && !xuiInternalTextWordBoundary64Read(onRead, pUser, iLength, iStart, iPolicy) ) {
		int64_t iPrevious = xuiInternalTextGraphemePrev64Read(onRead, pUser, iLength, iStart);
		if ( iPrevious >= iStart ) break;
		iStart = iPrevious;
	}
	while ( iEnd < iLength && !xuiInternalTextWordBoundary64Read(onRead, pUser, iLength, iEnd, iPolicy) ) {
		int64_t iNext = xuiInternalTextGraphemeNext64Read(onRead, pUser, iLength, iEnd);
		if ( iNext <= iEnd ) break;
		iEnd = iNext;
	}
	if ( pStart != NULL ) *pStart = iStart;
	if ( pEnd != NULL ) *pEnd = iEnd;
	if ( __xuiUnicodeWordPolicyUnit(&tUnit, iPolicy) ) return XUI_INTERNAL_WORD_TEXT;
	for ( iAt = iStart; iAt < iEnd; iAt = tRangeUnit.iEnd ) {
		if ( !__xuiUnicodeWordUnitRead(onRead, pUser, iLength, iAt, &tRangeUnit) ||
		     tRangeUnit.iEnd <= iAt ) break;
		if ( __xuiUnicodeWordPolicyUnit(&tRangeUnit, iPolicy) ) return XUI_INTERNAL_WORD_TEXT;
	}
	return XUI_INTERNAL_WORD_SYMBOL;
}

int64_t xuiInternalTextWordPrev64Read(xui_internal_text_read64_proc onRead, void* pUser,
	int64_t iLength, int64_t iOffset, xui_internal_word_policy_t iPolicy)
{
	int64_t iStart;
	int64_t iEnd;
	int64_t iProbe;
	xui_internal_word_kind_t iKind;

	if ( onRead == NULL || iLength <= 0 ) return 0;
	iOffset = xuiInternalTextGraphemeClamp64Read(onRead, pUser, iLength, iOffset);
	while ( iOffset > 0 ) {
		iProbe = xuiInternalTextGraphemePrev64Read(onRead, pUser, iLength, iOffset);
		iKind = xuiInternalTextWordRange64Read(onRead, pUser, iLength, iProbe,
			iPolicy, &iStart, &iEnd);
		if ( iKind == XUI_INTERNAL_WORD_TEXT ) return iStart;
		if ( iKind == XUI_INTERNAL_WORD_SYMBOL && iStart < iOffset ) iOffset = iStart;
		else iOffset = iProbe;
	}
	return 0;
}

int64_t xuiInternalTextWordNext64Read(xui_internal_text_read64_proc onRead, void* pUser,
	int64_t iLength, int64_t iOffset, xui_internal_word_policy_t iPolicy)
{
	int64_t iStart;
	int64_t iEnd;
	int64_t iNext;
	xui_internal_word_kind_t iKind;

	if ( onRead == NULL || iLength <= 0 ) return 0;
	iOffset = xuiInternalTextGraphemeClamp64Read(onRead, pUser, iLength, iOffset);
	if ( iOffset >= iLength ) return iLength;
	iKind = xuiInternalTextWordRange64Read(onRead, pUser, iLength, iOffset,
		iPolicy, &iStart, &iEnd);
	if ( iKind == XUI_INTERNAL_WORD_SPACE || iEnd <= iOffset ) {
		iOffset = xuiInternalTextGraphemeNext64Read(onRead, pUser, iLength, iOffset);
	} else {
		iOffset = iEnd;
	}
	while ( iOffset < iLength ) {
		iKind = xuiInternalTextWordRange64Read(onRead, pUser, iLength, iOffset,
			iPolicy, &iStart, &iEnd);
		if ( iKind == XUI_INTERNAL_WORD_TEXT ) return iStart;
		if ( iKind == XUI_INTERNAL_WORD_SYMBOL && iEnd > iOffset ) {
			iOffset = iEnd;
		} else {
			iNext = xuiInternalTextGraphemeNext64Read(onRead, pUser, iLength, iOffset);
			if ( iNext <= iOffset ) break;
			iOffset = iNext;
		}
	}
	return iLength;
}










typedef struct xui_unicode_read32_bridge {
    xui_internal_text_read_proc read;
    void* user;
} xui_unicode_read32_bridge;
static int __xuiUnicodeRead32(void* user, int64_t offset, unsigned char* byte)
{
    xui_unicode_read32_bridge* bridge = user;
    return offset >= 0 && offset <= INT_MAX && bridge->read(bridge->user, (int)offset, byte);
}
static int __xuiUnicodePointerRead32(void* user, int offset, unsigned char* byte)
{ return __xuiUnicodePointerRead(user, offset, byte); }

int xuiInternalTextGraphemeNextRead(xui_internal_text_read_proc read, void* user, int length, int offset)
{
    xui_unicode_read32_bridge bridge = {read, user};
    return (int)xuiInternalTextGraphemeNext64Read(read ? __xuiUnicodeRead32 : NULL, &bridge, length, offset);
}

int xuiInternalTextGraphemePrevRead(xui_internal_text_read_proc read, void* user, int length, int offset)
{
    xui_unicode_read32_bridge bridge = {read, user};
    return (int)xuiInternalTextGraphemePrev64Read(read ? __xuiUnicodeRead32 : NULL, &bridge, length, offset);
}

int xuiInternalTextGraphemeClampRead(xui_internal_text_read_proc read, void* user, int length, int offset)
{
    xui_unicode_read32_bridge bridge = {read, user};
    return (int)xuiInternalTextGraphemeClamp64Read(read ? __xuiUnicodeRead32 : NULL, &bridge, length, offset);
}

int xuiInternalTextWordBoundaryRead(xui_internal_text_read_proc read, void* user,
    int length, int offset, xui_internal_word_policy_t policy)
{
    xui_unicode_read32_bridge bridge = {read, user};
    return (int)xuiInternalTextWordBoundary64Read(read ? __xuiUnicodeRead32 : NULL, &bridge, length, offset, policy);
}

int xuiInternalTextWordPrevRead(xui_internal_text_read_proc read, void* user,
    int length, int offset, xui_internal_word_policy_t policy)
{
    xui_unicode_read32_bridge bridge = {read, user};
    return (int)xuiInternalTextWordPrev64Read(read ? __xuiUnicodeRead32 : NULL, &bridge, length, offset, policy);
}

int xuiInternalTextWordNextRead(xui_internal_text_read_proc read, void* user,
    int length, int offset, xui_internal_word_policy_t policy)
{
    xui_unicode_read32_bridge bridge = {read, user};
    return (int)xuiInternalTextWordNext64Read(read ? __xuiUnicodeRead32 : NULL, &bridge, length, offset, policy);
}

xui_internal_word_kind_t xuiInternalTextWordRangeRead(xui_internal_text_read_proc read,
    void* user, int length, int offset, xui_internal_word_policy_t policy, int* start, int* end)
{
    xui_unicode_read32_bridge bridge = {read, user};
    int64_t a, b;
    xui_internal_word_kind_t kind = xuiInternalTextWordRange64Read(
        read ? __xuiUnicodeRead32 : NULL, &bridge, length, offset, policy, &a, &b);
    if (start) *start = (int)a;
    if (end) *end = (int)b;
    return kind;
}
int xuiInternalTextGraphemeNext(const char* sText, int iLength, int iOffset)
{
	if ( sText == NULL ) return 0;
	if ( iLength < 0 ) iLength = (int)strlen(sText);
	return xuiInternalTextGraphemeNextRead(__xuiUnicodePointerRead32,
		(void*)sText, iLength, iOffset);
}

int xuiInternalTextGraphemePrev(const char* sText, int iLength, int iOffset)
{
	if ( sText == NULL ) return 0;
	if ( iLength < 0 ) iLength = (int)strlen(sText);
	return xuiInternalTextGraphemePrevRead(__xuiUnicodePointerRead32,
		(void*)sText, iLength, iOffset);
}

int xuiInternalTextGraphemeClamp(const char* sText, int iLength, int iOffset)
{
	if ( sText == NULL ) return 0;
	if ( iLength < 0 ) iLength = (int)strlen(sText);
	return xuiInternalTextGraphemeClampRead(__xuiUnicodePointerRead32,
		(void*)sText, iLength, iOffset);
}

int xuiInternalTextWordBoundary(const char* sText, int iLength, int iOffset,
	xui_internal_word_policy_t iPolicy)
{
	if ( sText == NULL ) return 1;
	if ( iLength < 0 ) iLength = (int)strlen(sText);
	return xuiInternalTextWordBoundaryRead(__xuiUnicodePointerRead32, (void*)sText,
		iLength, iOffset, iPolicy);
}

xui_internal_word_kind_t xuiInternalTextWordRange(const char* sText, int iLength,
	int iOffset, xui_internal_word_policy_t iPolicy, int* pStart, int* pEnd)
{
	if ( sText == NULL ) {
		if ( pStart != NULL ) *pStart = 0;
		if ( pEnd != NULL ) *pEnd = 0;
		return XUI_INTERNAL_WORD_SPACE;
	}
	if ( iLength < 0 ) iLength = (int)strlen(sText);
	return xuiInternalTextWordRangeRead(__xuiUnicodePointerRead32, (void*)sText,
		iLength, iOffset, iPolicy, pStart, pEnd);
}

int xuiInternalTextWordPrev(const char* sText, int iLength, int iOffset,
	xui_internal_word_policy_t iPolicy)
{
	if ( sText == NULL ) return 0;
	if ( iLength < 0 ) iLength = (int)strlen(sText);
	return xuiInternalTextWordPrevRead(__xuiUnicodePointerRead32, (void*)sText,
		iLength, iOffset, iPolicy);
}

int xuiInternalTextWordNext(const char* sText, int iLength, int iOffset,
	xui_internal_word_policy_t iPolicy)
{
	if ( sText == NULL ) return 0;
	if ( iLength < 0 ) iLength = (int)strlen(sText);
	return xuiInternalTextWordNextRead(__xuiUnicodePointerRead32, (void*)sText,
		iLength, iOffset, iPolicy);
}

#endif
