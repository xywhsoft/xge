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
	iRet = xuiFindText("alpha beta Alpha alphabet", -1, "beta", 22, 0, -1, XUI_FIND_WRAP, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 6, "wrap");
	iRet = xuiFindText("alpha beta Alpha alphabet", -1, "beta", 20, 10, 25, XUI_FIND_WRAP, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED, "range excludes outside match");
	iRet = xuiFindText("line1\nline2", -1, "1\\nline", 0, 0, -1, XUI_FIND_ESCAPE, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 4 && tResult.iEnd == 10, "escape pattern");

	iRet = xuiFindCollectText("one two one two", -1, "one", 0, -1, XUI_FIND_CASE_SENSITIVE, arrResults, 8, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "collect count");
	XUI_TEST_CHECK(arrResults[0].iStart == 0 && arrResults[1].iStart == 8, "collect ranges");

	iRet = xuiFindText("foo_1 foo_2", -1, "foo_([0-9])", 0, 0, -1, XUI_FIND_REGEX | XUI_FIND_CASE_SENSITIVE, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 0 && tResult.iEnd == 5, "regex find");
	XUI_TEST_CHECK(tResult.iCaptureCount >= 2 && tResult.arrCaptures[1].iStart == 4 && tResult.arrCaptures[1].iEnd == 5, "regex capture");
	iRet = xuiFindText("foo_1 foo_2", -1, "foo_([0-9])", 11, 0, -1, XUI_FIND_REGEX | XUI_FIND_BACKWARD | XUI_FIND_CASE_SENSITIVE, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 6, "regex backward");

	iRet = xuiFindReplaceAllText("int a = 1;\nint b = 2;\n", -1, "int ([a-z]) = ([0-9]);", "long $1 = $2;", 0, -1, XUI_FIND_REGEX | XUI_FIND_CASE_SENSITIVE, &sOutput, &iOutputLength, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "regex replace count");
	XUI_TEST_CHECK(strcmp(sOutput, "long a = 1;\nlong b = 2;\n") == 0, "regex replace output");
	XUI_TEST_CHECK(iOutputLength == (int)strlen(sOutput), "regex replace length");
	xuiFindFreeText(sOutput);
	sOutput = NULL;

	iRet = xuiFindReplaceAllText("a\\nb\\n", -1, "\\\\n", "\n", 0, -1, XUI_FIND_ESCAPE | XUI_FIND_CASE_SENSITIVE, &sOutput, &iOutputLength, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "escape replace count");
	XUI_TEST_CHECK(strcmp(sOutput, "a\nb\n") == 0, "escape replace output");

cleanup:
	xuiFindFreeText(sOutput);
	if ( iFailed ) return 1;
	printf("xui_find_test passed\n");
	return 0;
}
