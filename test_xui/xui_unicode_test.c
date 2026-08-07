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

	printf("xui_unicode_test passed\n");
	return 0;
}
