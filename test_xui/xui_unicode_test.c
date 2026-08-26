#include "src/xui_internal.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_unicode_test failed: %s\n", (msg)); \
			return 1; \
		} \
	} while ( 0 )

static int check_single_cluster(const char* sText, const char* sName)
{
	int iLength;
	int i;

	iLength = (int)strlen(sText);
	if ( xuiInternalTextGraphemeNext(sText, iLength, 0) != iLength ) {
		printf("xui_unicode_test failed: %s next\n", sName);
		return 0;
	}
	if ( xuiInternalTextGraphemePrev(sText, iLength, iLength) != 0 ) {
		printf("xui_unicode_test failed: %s prev\n", sName);
		return 0;
	}
	for ( i = 1; i < iLength; i++ ) {
		if ( xuiInternalTextGraphemeClamp(sText, iLength, i) != 0 ) {
			printf("xui_unicode_test failed: %s clamp\n", sName);
			return 0;
		}
	}
	return 1;
}

static int check_word_range(const char* sText, int iOffset,
	xui_internal_word_policy_t iPolicy, xui_internal_word_kind_t iExpectedKind,
	int iExpectedStart, int iExpectedEnd, const char* sName)
{
	xui_internal_word_kind_t iKind;
	int iStart;
	int iEnd;

	iStart = -1;
	iEnd = -1;
	iKind = xuiInternalTextWordRange(sText, -1, iOffset, iPolicy, &iStart, &iEnd);
	if ( iKind != iExpectedKind || iStart != iExpectedStart || iEnd != iExpectedEnd ) {
		printf("xui_unicode_test failed: %s range kind=%d [%d,%d)\n",
			sName, (int)iKind, iStart, iEnd);
		return 0;
	}
	return 1;
}

int main(void)
{
	static const char sFamily[] =
		"\xF0\x9F\x91\xA8\xE2\x80\x8D"
		"\xF0\x9F\x91\xA9\xE2\x80\x8D"
		"\xF0\x9F\x91\xA7\xE2\x80\x8D"
		"\xF0\x9F\x91\xA6";
	static const char sSkinTone[] =
		"\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBD";
	static const char sFlag[] =
		"\xF0\x9F\x87\xA8\xF0\x9F\x87\xB3";
	static const char sHeart[] =
		"\xE2\x9D\xA4\xEF\xB8\x8F";
	static const char sKeycap[] =
		"1\xEF\xB8\x8F\xE2\x83\xA3";
	static const char sTagFlag[] =
		"\xF0\x9F\x8F\xB4"
		"\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xA5"
		"\xF3\xA0\x81\xAE\xF3\xA0\x81\xA7\xF3\xA0\x81\xBF";
	static const char sCombining[] = "e\xCC\x81";
	static const char sHangul[] = "\xE1\x84\x80\xE1\x85\xA1\xE1\x86\xA8";
	static const char sNaturalWords[] =
		"can't 3.14 \xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82 "
		"\xE4\xBD\xA0\xE5\xA5\xBD";
	static const char sIdentifier[] =
		"\xE5\x8F\x98\xE9\x87\x8F_name";
	static const char sCombiningWord[] = "e\xCC\x81" "lan";
	static const char sWordMovement[] =
		"hello, \xD0\xBC\xD0\xB8\xD1\x80";
	static const char sNbsp[] = "a\xC2\xA0" "b";
	char sMixed[96];
	char sRegionalRun[32];
	int iFamilyLength;
	int iMixedLength;
	int iFlagLength;

	XUI_TEST_CHECK(check_single_cluster(sFamily, "family ZWJ"), "family cluster");
	XUI_TEST_CHECK(check_single_cluster(sSkinTone, "skin tone"), "skin tone cluster");
	XUI_TEST_CHECK(check_single_cluster(sFlag, "regional flag"), "flag cluster");
	XUI_TEST_CHECK(check_single_cluster(sHeart, "variation selector"), "heart cluster");
	XUI_TEST_CHECK(check_single_cluster(sKeycap, "keycap"), "keycap cluster");
	XUI_TEST_CHECK(check_single_cluster(sTagFlag, "tag flag"), "tag flag cluster");
	XUI_TEST_CHECK(check_single_cluster(sCombining, "combining mark"), "combining cluster");
	XUI_TEST_CHECK(check_single_cluster(sHangul, "Hangul syllable"), "Hangul cluster");
	XUI_TEST_CHECK(check_single_cluster("\r\n", "CRLF"), "CRLF cluster");

	iFamilyLength = (int)strlen(sFamily);
	sMixed[0] = 'A';
	memcpy(sMixed + 1, sFamily, (size_t)iFamilyLength);
	sMixed[1 + iFamilyLength] = 'B';
	sMixed[2 + iFamilyLength] = '\0';
	iMixedLength = iFamilyLength + 2;
	XUI_TEST_CHECK(xuiInternalTextGraphemeNext(sMixed, iMixedLength, 0) == 1, "mixed first");
	XUI_TEST_CHECK(xuiInternalTextGraphemeNext(sMixed, iMixedLength, 1) == 1 + iFamilyLength, "mixed family next");
	XUI_TEST_CHECK(xuiInternalTextGraphemePrev(sMixed, iMixedLength, 1 + iFamilyLength) == 1, "mixed family prev");
	XUI_TEST_CHECK(xuiInternalTextGraphemePrev(sMixed, iMixedLength, iMixedLength) == 1 + iFamilyLength, "mixed last");

	iFlagLength = (int)strlen(sFlag);
	memcpy(sRegionalRun, sFlag, (size_t)iFlagLength);
	memcpy(sRegionalRun + iFlagLength, sFlag, 4u);
	sRegionalRun[iFlagLength + 4] = '\0';
	XUI_TEST_CHECK(xuiInternalTextGraphemeNext(sRegionalRun, iFlagLength + 4, 0) == iFlagLength, "regional pair");
	XUI_TEST_CHECK(xuiInternalTextGraphemeNext(sRegionalRun, iFlagLength + 4, iFlagLength) == iFlagLength + 4, "regional odd tail");
	XUI_TEST_CHECK(xuiInternalTextGraphemePrev(sRegionalRun, iFlagLength + 4, iFlagLength + 4) == iFlagLength, "regional tail prev");
	XUI_TEST_CHECK(xuiInternalTextGraphemePrev(sRegionalRun, iFlagLength + 4, iFlagLength) == 0, "regional pair prev");

	XUI_TEST_CHECK(check_word_range(sNaturalWords, 2, XUI_INTERNAL_WORD_NATURAL,
		XUI_INTERNAL_WORD_TEXT, 0, 5, "apostrophe word"), "apostrophe word range");
	XUI_TEST_CHECK(check_word_range(sNaturalWords, 7, XUI_INTERNAL_WORD_NATURAL,
		XUI_INTERNAL_WORD_TEXT, 6, 10, "decimal word"), "decimal word range");
	XUI_TEST_CHECK(check_word_range(sNaturalWords, 15, XUI_INTERNAL_WORD_NATURAL,
		XUI_INTERNAL_WORD_TEXT, 11, 23, "Cyrillic word"), "Cyrillic word range");
	XUI_TEST_CHECK(check_word_range(sNaturalWords, 24, XUI_INTERNAL_WORD_NATURAL,
		XUI_INTERNAL_WORD_TEXT, 24, 27, "first Han word"), "first Han word range");
	XUI_TEST_CHECK(check_word_range(sNaturalWords, 27, XUI_INTERNAL_WORD_NATURAL,
		XUI_INTERNAL_WORD_TEXT, 27, 30, "second Han word"), "second Han word range");
	XUI_TEST_CHECK(check_word_range(sIdentifier, 3, XUI_INTERNAL_WORD_IDENTIFIER,
		XUI_INTERNAL_WORD_TEXT, 0, 11, "Unicode identifier"), "Unicode identifier range");
	XUI_TEST_CHECK(check_word_range(sCombiningWord, 1, XUI_INTERNAL_WORD_NATURAL,
		XUI_INTERNAL_WORD_TEXT, 0, 6, "combining word"), "combining word range");
	XUI_TEST_CHECK(check_word_range(sNbsp, 1, XUI_INTERNAL_WORD_NATURAL,
		XUI_INTERNAL_WORD_SPACE, 1, 1, "Unicode whitespace"), "Unicode whitespace range");

	XUI_TEST_CHECK(check_word_range(sMixed, 4, XUI_INTERNAL_WORD_NATURAL,
		XUI_INTERNAL_WORD_SYMBOL, 1, 1 + iFamilyLength, "emoji symbol"), "emoji symbol range");
	XUI_TEST_CHECK(xuiInternalTextWordNext(sWordMovement, -1, 0,
		XUI_INTERNAL_WORD_NATURAL) == 7, "word next skips punctuation and space");
	XUI_TEST_CHECK(xuiInternalTextWordPrev(sWordMovement, -1, (int)strlen(sWordMovement),
		XUI_INTERNAL_WORD_NATURAL) == 7, "word prev enters Cyrillic word");
	XUI_TEST_CHECK(xuiInternalTextWordPrev(sWordMovement, -1, 7,
		XUI_INTERNAL_WORD_NATURAL) == 0, "word prev skips punctuation and space");
	XUI_TEST_CHECK(xuiInternalTextWordBoundary(sIdentifier, -1, 6,
		XUI_INTERNAL_WORD_IDENTIFIER) == 0, "identifier joins underscore");
	XUI_TEST_CHECK(xuiInternalTextWordBoundary(sNaturalWords, -1, 27,
		XUI_INTERNAL_WORD_NATURAL) != 0, "natural Han boundary");

	printf("xui_unicode_test passed\n");
	return 0;
}
