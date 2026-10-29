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

enum {
	XUI_FIND_TEST_FIND = 0,
	XUI_FIND_TEST_COLLECT,
	XUI_FIND_TEST_REPLACE_ALL,
	XUI_FIND_TEST_BUILD_REPLACEMENT,
	XUI_FIND_TEST_OPERATION_COUNT
};

static int __xuiFindTestBudgetOperation(size_t iBudget, uint32_t iFlags,
	int iOperation, int* pStatus, uint64_t* pRejections)
{
	xallocationbudget* pBudget;
	xallocationscope tScope;
	xallocationbudgetinfo tInfo;
	xui_find_result_t tResult;
	xui_find_result_t arrResults[4];
	const char* sPattern;
	const char* sReplacement;
	char* sOutput;
	char sError[80];
	int iOutputLength;
	int iCount;
	int iRet;

	pBudget = NULL;
	memset(&tScope, 0, sizeof(tScope));
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.Size = sizeof(tInfo);
	if ( !xrtAllocationBudgetCreate(iBudget, &pBudget) ) return 0;
	if ( !xrtAllocationBudgetEnter(pBudget, &tScope) ) {
		xrtAllocationBudgetFree(pBudget);
		return 0;
	}
	memset(&tResult, 0x7F, sizeof(tResult));
	memset(arrResults, 0x7F, sizeof(arrResults));
	sPattern = ((iFlags & XUI_FIND_REGEX) != 0u) ? "(alpha)" : "alpha";
	sReplacement = ((iFlags & XUI_FIND_REGEX) != 0u) ? "<$1>" : "X";
	sOutput = (char*)1;
	iOutputLength = 77;
	iCount = 77;
	switch ( iOperation ) {
	case XUI_FIND_TEST_FIND:
		iRet = xuiFindText("alpha beta alpha", -1, sPattern, 0, 0, -1,
			iFlags, &tResult, sError, sizeof(sError));
		if ( iRet != XUI_OK && (tResult.iStart != 0 || tResult.iEnd != 0) ) {
			xrtAllocationBudgetLeave(&tScope);
			xrtAllocationBudgetFree(pBudget);
			return 0;
		}
		break;
	case XUI_FIND_TEST_COLLECT:
		iRet = xuiFindCollectText("alpha beta alpha", -1, sPattern, 0, -1,
			iFlags, arrResults, 4, &iCount, sError, sizeof(sError));
		if ( iRet != XUI_OK && iCount != 0 ) {
			xrtAllocationBudgetLeave(&tScope);
			xrtAllocationBudgetFree(pBudget);
			return 0;
		}
		break;
	case XUI_FIND_TEST_REPLACE_ALL:
		iRet = xuiFindReplaceAllText("alpha beta alpha", -1, sPattern,
			sReplacement, 0, -1, iFlags, &sOutput, &iOutputLength, &iCount,
			sError, sizeof(sError));
		if ( iRet != XUI_OK &&
		     (sOutput != NULL || iOutputLength != 0 || iCount != 0) ) {
			xrtAllocationBudgetLeave(&tScope);
			xrtAllocationBudgetFree(pBudget);
			return 0;
		}
		break;
	case XUI_FIND_TEST_BUILD_REPLACEMENT:
		memset(&tResult, 0, sizeof(tResult));
		tResult.iSize = sizeof(tResult);
		tResult.iStart = 0;
		tResult.iEnd = 5;
		tResult.iCaptureCount = 2;
		tResult.arrCaptures[0].iStart = 0;
		tResult.arrCaptures[0].iEnd = 5;
		tResult.arrCaptures[1] = tResult.arrCaptures[0];
		iRet = xuiFindBuildReplacement("alpha", 5, sPattern, sReplacement,
			iFlags, &tResult, &sOutput, &iOutputLength, sError, sizeof(sError));
		if ( iRet != XUI_OK && (sOutput != NULL || iOutputLength != 0) ) {
			xrtAllocationBudgetLeave(&tScope);
			xrtAllocationBudgetFree(pBudget);
			return 0;
		}
		break;
	default:
		xrtAllocationBudgetLeave(&tScope);
		xrtAllocationBudgetFree(pBudget);
		return 0;
	}
	if ( iRet == XUI_OK ) {
		if ( iOperation == XUI_FIND_TEST_FIND &&
		     (tResult.iStart != 0 || tResult.iEnd != 5) ) iRet = XUI_ERROR_INVALID_STATE;
		if ( iOperation == XUI_FIND_TEST_COLLECT && iCount != 2 )
			iRet = XUI_ERROR_INVALID_STATE;
		if ( (iOperation == XUI_FIND_TEST_REPLACE_ALL ||
		      iOperation == XUI_FIND_TEST_BUILD_REPLACEMENT) &&
		     (sOutput == NULL || iOutputLength <= 0) ) iRet = XUI_ERROR_INVALID_STATE;
	}
	if ( sOutput != NULL && sOutput != (char*)1 ) xuiFindFreeText(sOutput);
	if ( !xrtAllocationBudgetLeave(&tScope) ||
	     !xrtAllocationBudgetInfo(pBudget, &tInfo) ) {
		xrtAllocationBudgetFree(pBudget);
		return 0;
	}
	xrtAllocationBudgetFree(pBudget);
	if ( tInfo.LiveBytes != 0u || tInfo.LiveCount != 0u ) return 0;
	*pStatus = iRet;
	*pRejections = tInfo.Rejections;
	return 1;
}

static int __xuiFindTestAllocationFailures(uint32_t iFlags)
{
	static const size_t arrBudgets[] = {
		1u, 2u, 4u, 8u, 16u, 32u, 64u, 128u, 256u, 512u,
		1024u, 2048u, 4096u, 8192u, 16384u, 32768u, 65536u
	};
	uint64_t iRejections;
	int bSawFailure;
	int bSawSuccess;
	int iStatus;
	int iOperation;
	size_t i;

	for ( iOperation = 0; iOperation < XUI_FIND_TEST_OPERATION_COUNT;
		iOperation++ ) {
		bSawFailure = 0;
		bSawSuccess = 0;
		for ( i = 0u; i < sizeof(arrBudgets) / sizeof(arrBudgets[0]); i++ ) {
			if ( !__xuiFindTestBudgetOperation(arrBudgets[i], iFlags, iOperation,
				&iStatus, &iRejections) ) return 0;
			if ( iStatus == XUI_OK ) {
				bSawSuccess = 1;
				break;
			}
			if ( iRejections == 0u || iStatus != XUI_ERROR_OUT_OF_MEMORY ) {
				printf("find budget diagnostic: flags=%u operation=%d budget=%zu status=%d rejections=%llu\n",
					(unsigned int)iFlags, iOperation, arrBudgets[i], iStatus,
					(unsigned long long)iRejections);
				return 0;
			}
			bSawFailure = 1;
		}
		if ( !bSawFailure || !bSawSuccess ) {
			printf("find budget diagnostic: flags=%u operation=%d sawFailure=%d sawSuccess=%d\n",
				(unsigned int)iFlags, iOperation, bSawFailure, bSawSuccess);
			return 0;
		}
	}
	return 1;
}

static int __xuiFindTestClamp(int iValue, int iMin, int iMax)
{
	if ( iValue < iMin ) return iMin;
	if ( iValue > iMax ) return iMax;
	return iValue;
}

static void __xuiFindTestNormalize(int iLength, int* pRangeStart,
	int* pRangeEnd, int* pStart)
{
	int iSwap;

	if ( *pRangeEnd < 0 ) *pRangeEnd = iLength;
	*pRangeStart = __xuiFindTestClamp(*pRangeStart, 0, iLength);
	*pRangeEnd = __xuiFindTestClamp(*pRangeEnd, 0, iLength);
	if ( *pRangeEnd < *pRangeStart ) {
		iSwap = *pRangeStart;
		*pRangeStart = *pRangeEnd;
		*pRangeEnd = iSwap;
	}
	if ( pStart != NULL )
		*pStart = __xuiFindTestClamp(*pStart, *pRangeStart, *pRangeEnd);
}

static int __xuiFindTestForward(const char* sText, int iStart, int iEnd,
	const char* sPattern, int iPatternLength)
{
	int i;

	for ( i = iStart; i + iPatternLength <= iEnd; i++ ) {
		if ( memcmp(sText + i, sPattern, (size_t)iPatternLength) == 0 ) return i;
	}
	return -1;
}

static int __xuiFindTestBackward(const char* sText, int iStart, int iEnd,
	const char* sPattern, int iPatternLength)
{
	int i;

	for ( i = iEnd - iPatternLength; i >= iStart; i-- ) {
		if ( memcmp(sText + i, sPattern, (size_t)iPatternLength) == 0 ) return i;
	}
	return -1;
}

static int __xuiFindTestPlainProperties(void)
{
	static const char* arrPatterns[] = { "a", "b", "aa", "ab", "ba" };
	static const uint32_t arrFlags[] = {
		XUI_FIND_CASE_SENSITIVE,
		XUI_FIND_CASE_SENSITIVE | XUI_FIND_WRAP,
		XUI_FIND_CASE_SENSITIVE | XUI_FIND_BACKWARD,
		XUI_FIND_CASE_SENSITIVE | XUI_FIND_BACKWARD | XUI_FIND_WRAP
	};
	xui_find_result_t tResult;
	char sText[6];
	char sError[80];
	int iExpected;
	int iRangeStart;
	int iRangeEnd;
	int iNormalizedStart;
	int iNormalizedEnd;
	int iNormalizedOffset;
	int iPatternLength;
	int iRet;
	int iLength;
	int iMask;
	int iTextIndex;
	int iPattern;
	int iFlags;
	int iStartOffset;

	for ( iLength = 0; iLength <= 4; iLength++ ) {
		for ( iMask = 0; iMask < (1 << iLength); iMask++ ) {
			for ( iTextIndex = 0; iTextIndex < iLength; iTextIndex++ )
				sText[iTextIndex] = ((iMask >> iTextIndex) & 1) ? 'b' : 'a';
			sText[iLength] = '\0';
			for ( iPattern = 0;
				iPattern < (int)(sizeof(arrPatterns) / sizeof(arrPatterns[0]));
				iPattern++ ) {
				iPatternLength = (int)strlen(arrPatterns[iPattern]);
				for ( iRangeStart = -2; iRangeStart <= iLength + 2; iRangeStart++ ) {
					for ( iRangeEnd = -2; iRangeEnd <= iLength + 2; iRangeEnd++ ) {
						for ( iStartOffset = -2; iStartOffset <= iLength + 2;
							iStartOffset++ ) {
							for ( iFlags = 0;
								iFlags < (int)(sizeof(arrFlags) / sizeof(arrFlags[0]));
								iFlags++ ) {
								iNormalizedStart = iRangeStart;
								iNormalizedEnd = iRangeEnd;
								iNormalizedOffset = iStartOffset;
								__xuiFindTestNormalize(iLength, &iNormalizedStart,
									&iNormalizedEnd, &iNormalizedOffset);
								if ( (arrFlags[iFlags] & XUI_FIND_BACKWARD) != 0u ) {
									iExpected = __xuiFindTestBackward(sText,
										iNormalizedStart, iNormalizedOffset,
										arrPatterns[iPattern], iPatternLength);
									if ( iExpected < 0 &&
									     (arrFlags[iFlags] & XUI_FIND_WRAP) != 0u )
										iExpected = __xuiFindTestBackward(sText,
											iNormalizedOffset, iNormalizedEnd,
											arrPatterns[iPattern], iPatternLength);
								} else {
									iExpected = __xuiFindTestForward(sText,
										iNormalizedOffset, iNormalizedEnd,
										arrPatterns[iPattern], iPatternLength);
									if ( iExpected < 0 &&
									     (arrFlags[iFlags] & XUI_FIND_WRAP) != 0u )
										iExpected = __xuiFindTestForward(sText,
											iNormalizedStart, iNormalizedOffset,
											arrPatterns[iPattern], iPatternLength);
								}
								memset(&tResult, 0x7F, sizeof(tResult));
								iRet = xuiFindText(sText, iLength,
									arrPatterns[iPattern], iStartOffset,
									iRangeStart, iRangeEnd, arrFlags[iFlags],
									&tResult, sError, sizeof(sError));
								if ( iExpected < 0 ) {
									if ( iRet != XUI_ERROR_UNSUPPORTED ||
									     tResult.iStart != 0 || tResult.iEnd != 0 )
										goto fail;
								} else if ( iRet != XUI_OK ||
								            tResult.iStart != iExpected ||
								            tResult.iEnd != iExpected + iPatternLength ||
								            tResult.iSize != sizeof(tResult) ) {
									goto fail;
								}
							}
						}
					}
				}
			}
		}
	}
	return 1;

fail:
	printf("find property diagnostic: text='%s' pattern='%s' start=%d range=%d..%d flags=%u expected=%d status=%d actual=%d..%d\n",
		sText, arrPatterns[iPattern], iStartOffset, iRangeStart, iRangeEnd,
		(unsigned int)arrFlags[iFlags], iExpected, iRet,
		tResult.iStart, tResult.iEnd);
	return 0;
}

static int __xuiFindTestCollectReplaceProperties(void)
{
	static const char* arrPatterns[] = { "a", "b", "aa", "ab", "ba" };
	xui_find_result_t arrResults[8];
	char sExpected[32];
	char sText[7];
	char sError[80];
	char* sOutput;
	int arrStarts[8];
	int iExpectedLength;
	int iExpectedCount;
	int iOutputLength;
	int iReplaceCount;
	int iRangeStart;
	int iRangeEnd;
	int iNormalizedStart;
	int iNormalizedEnd;
	int iPatternLength;
	int iResultCount;
	int iRet;
	int iLength;
	int iMask;
	int iTextIndex;
	int iPattern;
	int iCursor;
	int i;

	for ( iLength = 0; iLength <= 5; iLength++ ) {
		for ( iMask = 0; iMask < (1 << iLength); iMask++ ) {
			for ( iTextIndex = 0; iTextIndex < iLength; iTextIndex++ )
				sText[iTextIndex] = ((iMask >> iTextIndex) & 1) ? 'b' : 'a';
			sText[iLength] = '\0';
			for ( iPattern = 0;
				iPattern < (int)(sizeof(arrPatterns) / sizeof(arrPatterns[0]));
				iPattern++ ) {
				iPatternLength = (int)strlen(arrPatterns[iPattern]);
				for ( iRangeStart = -2; iRangeStart <= iLength + 2; iRangeStart++ ) {
					for ( iRangeEnd = -2; iRangeEnd <= iLength + 2; iRangeEnd++ ) {
						iNormalizedStart = iRangeStart;
						iNormalizedEnd = iRangeEnd;
						__xuiFindTestNormalize(iLength, &iNormalizedStart,
							&iNormalizedEnd, NULL);
						iExpectedCount = 0;
						iCursor = iNormalizedStart;
						while ( iCursor + iPatternLength <= iNormalizedEnd ) {
							if ( memcmp(sText + iCursor, arrPatterns[iPattern],
								(size_t)iPatternLength) == 0 ) {
								arrStarts[iExpectedCount++] = iCursor;
								iCursor += iPatternLength;
							} else {
								iCursor++;
							}
						}
						memset(arrResults, 0x7F, sizeof(arrResults));
						iResultCount = -1;
						iRet = xuiFindCollectText(sText, iLength,
							arrPatterns[iPattern], iRangeStart, iRangeEnd,
							XUI_FIND_CASE_SENSITIVE | XUI_FIND_BACKWARD |
							XUI_FIND_WRAP, arrResults, 8, &iResultCount,
							sError, sizeof(sError));
						if ( iRet != XUI_OK || iResultCount != iExpectedCount )
							goto fail;
						for ( i = 0; i < iExpectedCount; i++ ) {
							if ( arrResults[i].iStart != arrStarts[i] ||
							     arrResults[i].iEnd != arrStarts[i] + iPatternLength ||
							     arrResults[i].iSize != sizeof(arrResults[i]) )
								goto fail;
						}
						iExpectedLength = 0;
						memcpy(sExpected, sText, (size_t)iNormalizedStart);
						iExpectedLength += iNormalizedStart;
						iCursor = iNormalizedStart;
						for ( i = 0; i < iExpectedCount; i++ ) {
							memcpy(sExpected + iExpectedLength, sText + iCursor,
								(size_t)(arrStarts[i] - iCursor));
							iExpectedLength += arrStarts[i] - iCursor;
							memcpy(sExpected + iExpectedLength, "XY", 2u);
							iExpectedLength += 2;
							iCursor = arrStarts[i] + iPatternLength;
						}
						memcpy(sExpected + iExpectedLength, sText + iCursor,
							(size_t)(iLength - iCursor));
						iExpectedLength += iLength - iCursor;
						sOutput = NULL;
						iOutputLength = -1;
						iReplaceCount = -1;
						iRet = xuiFindReplaceAllText(sText, iLength,
							arrPatterns[iPattern], "XY", iRangeStart, iRangeEnd,
							XUI_FIND_CASE_SENSITIVE | XUI_FIND_BACKWARD |
							XUI_FIND_WRAP, &sOutput, &iOutputLength, &iReplaceCount,
							sError, sizeof(sError));
						if ( iRet != XUI_OK || sOutput == NULL ||
						     iReplaceCount != iExpectedCount ||
						     iOutputLength != iExpectedLength ||
						     memcmp(sOutput, sExpected, (size_t)iExpectedLength) != 0 ||
						     sOutput[iOutputLength] != '\0' ) {
							xuiFindFreeText(sOutput);
							goto fail;
						}
						xuiFindFreeText(sOutput);
					}
				}
			}
		}
	}
	return 1;

fail:
	printf("find collect/replace property diagnostic: text='%s' pattern='%s' range=%d..%d expectedCount=%d status=%d\n",
		sText, arrPatterns[iPattern], iRangeStart, iRangeEnd,
		iExpectedCount, iRet);
	return 0;
}

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
	iRet = xuiFindText("0123456789", -1, "34", 9, 7, 2,
		XUI_FIND_CASE_SENSITIVE | XUI_FIND_WRAP, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 3 && tResult.iEnd == 5,
		"reversed range is normalized before wrapping");
	{
		const char sEmbedded[] = { 'a', '\0', 'b', 'a', '\0', 'b' };
		iRet = xuiFindText(sEmbedded, (int)sizeof(sEmbedded), "b", 0, 0, -1,
			XUI_FIND_CASE_SENSITIVE, &tResult, sError, sizeof(sError));
		XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 2 && tResult.iEnd == 3,
			"explicit text length supports embedded nul");
		iRet = xuiFindReplaceAllText(sEmbedded, (int)sizeof(sEmbedded), "b", "B",
			0, -1, XUI_FIND_CASE_SENSITIVE, &sOutput, &iOutputLength, &iCount,
			sError, sizeof(sError));
		XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2 &&
			iOutputLength == (int)sizeof(sEmbedded) &&
			memcmp(sOutput, "a\0Ba\0B", sizeof(sEmbedded)) == 0,
			"replace preserves embedded nul and explicit length");
		xuiFindFreeText(sOutput);
		sOutput = NULL;
	}
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

	XUI_TEST_CHECK(__xuiFindTestAllocationFailures(XUI_FIND_CASE_SENSITIVE),
		"plain find reports allocation failures transactionally");
	XUI_TEST_CHECK(__xuiFindTestAllocationFailures(
		XUI_FIND_REGEX | XUI_FIND_CASE_SENSITIVE),
		"regex find distinguishes allocation failure from invalid pattern");
	XUI_TEST_CHECK(__xuiFindTestPlainProperties(),
		"plain find matches exhaustive range and direction reference");
	XUI_TEST_CHECK(__xuiFindTestCollectReplaceProperties(),
		"plain collect and replace match exhaustive range reference");

cleanup:
	xuiFindFreeText(sOutput);
	if ( iFailed ) return 1;
	printf("xui_find_test passed\n");
	return 0;
}
