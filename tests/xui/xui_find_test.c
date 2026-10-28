#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_find_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

int main(void)
{
	xui_find_result_t tResult;
	xui_find_result_t arrResults[8];
	char* sOutput;
	char sError[160];
	int iOutputLength;
	int iCount;
	int iFailed;
	int iRet;

	sOutput = NULL;
	iOutputLength = 0;
	iCount = 0;
	iFailed = 0;
	memset(sError, 0, sizeof(sError));

	iRet = xuiFindText("alpha beta Alpha alphabet", -1, "ALPHA", 0, 0, -1, 0, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 0 && tResult.iEnd == 5, "plain case insensitive");
	iRet = xuiFindText("alpha beta Alpha alphabet", -1, "Alpha", 0, 0, -1, XUI_FIND_CASE_SENSITIVE, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 11, "plain case sensitive");
	iRet = xuiFindText("alpha beta Alpha alphabet", -1, "alpha", 1, 0, -1, XUI_FIND_WHOLE_WORD, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 11, "whole word");
	iRet = xuiFindText("alpha beta Alpha alphabet", -1, "alpha", 8, 0, -1, XUI_FIND_BACKWARD, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 0, "backward");
	iRet = xuiFindText("cat scatter catapult", -1, "cat", 19, 0, -1,
		XUI_FIND_BACKWARD | XUI_FIND_WHOLE_WORD | XUI_FIND_CASE_SENSITIVE,
		&tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 0 && tResult.iEnd == 3,
		"backward whole word skips invalid suffix matches");
	iRet = xuiFindText("alpha beta Alpha alphabet", -1, "beta", 22, 0, -1, XUI_FIND_WRAP, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 6, "wrap");
	iRet = xuiFindText("alpha beta Alpha alphabet", -1, "beta", 20, 10, 25, XUI_FIND_WRAP, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED, "range excludes outside match");
	iRet = xuiFindText("line1\nline2", -1, "1\\nline", 0, 0, -1, XUI_FIND_ESCAPE, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 4 && tResult.iEnd == 10, "escape pattern");
	iRet = xuiFindText(
		"\xD0\xBC\xD0\xB8\xD1\x80 \xD0\xBC\xD0\xB8\xD1\x80x \xD0\xBC\xD0\xB8\xD1\x80",
		-1, "\xD0\xBC\xD0\xB8\xD1\x80", 1, 0, -1,
		XUI_FIND_CASE_SENSITIVE | XUI_FIND_WHOLE_WORD, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 15,
		"Unicode natural whole word skips longer word");

	iRet = xuiFindCollectText("one two one two", -1, "one", 0, -1, XUI_FIND_CASE_SENSITIVE, arrResults, 8, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "collect count");
	XUI_TEST_CHECK(arrResults[0].iStart == 0 && arrResults[1].iStart == 8, "collect ranges");
	iRet = xuiFindCollectText("one two one", -1, "one", 0, -1,
		XUI_FIND_CASE_SENSITIVE, NULL, 0, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "zero-capacity collect returns total count");

	iRet = xuiFindText("foo_1 foo_2", -1, "foo_([0-9])", 0, 0, -1, XUI_FIND_REGEX | XUI_FIND_CASE_SENSITIVE, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 0 && tResult.iEnd == 5, "regex find");
	XUI_TEST_CHECK(tResult.iCaptureCount >= 2 && tResult.arrCaptures[1].iStart == 4 && tResult.arrCaptures[1].iEnd == 5, "regex capture");
	iRet = xuiFindText("foo_1 foo_2", -1, "foo_([0-9])", 11, 0, -1, XUI_FIND_REGEX | XUI_FIND_BACKWARD | XUI_FIND_CASE_SENSITIVE, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 6, "regex backward");
	iRet = xuiFindText("aaa", -1, "aa", 3, 0, -1,
		XUI_FIND_REGEX | XUI_FIND_BACKWARD | XUI_FIND_CASE_SENSITIVE,
		&tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 1 && tResult.iEnd == 3,
		"regex backward keeps latest overlapping match");
	iRet = xuiFindText("aba", -1, "a", 1, 0, -1,
		XUI_FIND_REGEX | XUI_FIND_CASE_SENSITIVE, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 2 && tResult.iEnd == 3,
		"regex without captures honors nonzero start");
	iRet = xuiFindText("aaa", -1, "a+", 0, 0, 2,
		XUI_FIND_REGEX | XUI_FIND_CASE_SENSITIVE, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 0 && tResult.iEnd == 2,
		"regex range constrains greedy match");

	iRet = xuiFindReplaceAllText("int a = 1;\nint b = 2;\n", -1, "int ([a-z]) = ([0-9]);", "long $1 = $2;", 0, -1, XUI_FIND_REGEX | XUI_FIND_CASE_SENSITIVE, &sOutput, &iOutputLength, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "regex replace count");
	XUI_TEST_CHECK(strcmp(sOutput, "long a = 1;\nlong b = 2;\n") == 0, "regex replace output");
	XUI_TEST_CHECK(iOutputLength == (int)strlen(sOutput), "regex replace length");
	xuiFindFreeText(sOutput);
	sOutput = NULL;

	iRet = xuiFindReplaceAllText("a\\nb\\n", -1, "\\\\n", "\n", 0, -1, XUI_FIND_ESCAPE | XUI_FIND_CASE_SENSITIVE, &sOutput, &iOutputLength, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "escape replace count");
	XUI_TEST_CHECK(strcmp(sOutput, "a\nb\n") == 0, "escape replace output");
	xuiFindFreeText(sOutput);
	sOutput = NULL;

	iRet = xuiFindText("A\xF0\x9F\x98\x80" "B", -1, "\\uD83D\\uDE00", 0, 0, -1,
		XUI_FIND_ESCAPE | XUI_FIND_CASE_SENSITIVE, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 1 && tResult.iEnd == 5,
		"unicode escape surrogate pair");
	iRet = xuiFindText("text", -1, "\\uD83D", 0, 0, -1, XUI_FIND_ESCAPE,
		&tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && strstr(sError, "surrogate") != NULL,
		"lone unicode surrogate rejected");
	iRet = xuiFindText("text", -1, "\\x", 0, 0, -1, XUI_FIND_ESCAPE,
		&tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && strcmp(sError, "invalid hex escape") == 0,
		"truncated hex escape rejected");
	iRet = xuiFindText("text", -1, "\\u12", 0, 0, -1, XUI_FIND_ESCAPE,
		&tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && strcmp(sError, "invalid unicode escape") == 0,
		"truncated unicode escape rejected");
	iRet = xuiFindText("text", -1, "\\x00", 0, 0, -1, XUI_FIND_ESCAPE,
		&tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && strcmp(sError, "nul escape is unsupported") == 0,
		"escaped nul rejected");
	iRet = xuiFindText("text", -1, "(", 0, 0, -1, XUI_FIND_REGEX,
		&tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && strcmp(sError, "regex compile failed") == 0,
		"regex compile error preserved");

	iRet = xuiFindCollectText("\xC3\xA9", 2, "a*", 0, -1,
		XUI_FIND_REGEX | XUI_FIND_CASE_SENSITIVE, arrResults, 8, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "zero-length regex Unicode count");
	XUI_TEST_CHECK(arrResults[0].iStart == 0 && arrResults[1].iStart == 2,
		"zero-length regex advances by Unicode boundary");
	iRet = xuiFindText("\xC3\xA9", 2, "a*", 2, 0, -1,
		XUI_FIND_REGEX | XUI_FIND_BACKWARD | XUI_FIND_CASE_SENSITIVE,
		&tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 2 && tResult.iEnd == 2,
		"backward regex keeps terminal empty match");
	iRet = xuiFindReplaceAllText("\xC3\xA9", 2, "a*", "_", 0, -1,
		XUI_FIND_REGEX | XUI_FIND_CASE_SENSITIVE, &sOutput, &iOutputLength, &iCount,
		sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "zero-length regex replace count");
	XUI_TEST_CHECK(iOutputLength == 4 && memcmp(sOutput, "_\xC3\xA9_", 4) == 0,
		"zero-length regex replace preserves UTF-8");
	xuiFindFreeText(sOutput);
	sOutput = NULL;
	iRet = xuiFindReplaceAllText("\xC3\xA9", 2, "a*", "_", 0, 1,
		XUI_FIND_REGEX | XUI_FIND_CASE_SENSITIVE, &sOutput, &iOutputLength, &iCount,
		sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1,
		"zero-length regex partial UTF-8 range count");
	XUI_TEST_CHECK(iOutputLength == 3 && memcmp(sOutput, "_\xC3\xA9", 3) == 0,
		"zero-length regex does not consume beyond range");
	xuiFindFreeText(sOutput);
	sOutput = NULL;
	iRet = xuiFindReplaceAllText("a", 1, "a", "\\x", 0, -1,
		XUI_FIND_REGEX | XUI_FIND_ESCAPE | XUI_FIND_CASE_SENSITIVE,
		&sOutput, &iOutputLength, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && sOutput == NULL &&
		strcmp(sError, "invalid hex escape") == 0, "truncated replacement escape rejected");

	memset(&tResult, 0, sizeof(tResult));
	tResult.iStart = 0;
	tResult.iEnd = 1;
	tResult.iCaptureCount = 1;
	tResult.arrCaptures[0].iStart = 0;
	tResult.arrCaptures[0].iEnd = 99;
	iRet = xuiFindBuildReplacement("a", 1, "(a)", "$0", XUI_FIND_REGEX,
		&tResult, &sOutput, &iOutputLength, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && sOutput == NULL && iOutputLength == 0,
		"invalid capture range rejected");

	sOutput = (char*)1;
	iOutputLength = 17;
	iCount = 23;
	iRet = xuiFindReplaceAllText(NULL, -1, "a", "b", 0, -1, 0,
		&sOutput, &iOutputLength, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && sOutput == NULL &&
		iOutputLength == 0 && iCount == 0, "invalid replace clears outputs");

	memset(&tResult, 0x7F, sizeof(tResult));
	iRet = xuiFindText("a", 1, "a", 0, 0, -1, 0x80000000u,
		&tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && tResult.iStart == 0 &&
		tResult.iEnd == 0, "find rejects unknown flags and clears result");
	iCount = 9;
	iRet = xuiFindCollectText("a", 1, "a", 0, -1, 0x80000000u,
		arrResults, 8, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iCount == 0,
		"collect rejects unknown flags and clears count");
	sOutput = (char*)1;
	iOutputLength = 9;
	iCount = 9;
	iRet = xuiFindReplaceAllText("a", 1, "a", "b", 0, -1, 0x80000000u,
		&sOutput, &iOutputLength, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && sOutput == NULL &&
		iOutputLength == 0 && iCount == 0, "replace rejects unknown flags and clears outputs");
	memset(&tResult, 0, sizeof(tResult));
	tResult.iStart = 0;
	tResult.iEnd = 1;
	sOutput = (char*)1;
	iOutputLength = 9;
	iRet = xuiFindBuildReplacement("a", 1, "a", "b", 0x80000000u,
		&tResult, &sOutput, &iOutputLength, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && sOutput == NULL &&
		iOutputLength == 0, "build replacement rejects unknown flags and clears output");

cleanup:
	xuiFindFreeText(sOutput);
	if ( iFailed ) return 1;
	printf("xui_find_test passed\n");
	return 0;
}
