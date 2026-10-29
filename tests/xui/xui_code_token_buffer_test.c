#include "xui.h"

#include <stdio.h>
#include <string.h>

static int g_iTokenFailAllocation = -1;
static int g_bTokenAllocationFailed = 0;

static void* xuiTestTokenMalloc(size_t iSize)
{
	if ( g_iTokenFailAllocation >= 0 && g_iTokenFailAllocation-- == 0 ) {
		g_bTokenAllocationFailed = 1;
		return NULL;
	}
	return xrtMalloc(iSize);
}

static void* xuiTestTokenRealloc(void* pMemory, size_t iSize)
{
	if ( g_iTokenFailAllocation >= 0 && g_iTokenFailAllocation-- == 0 ) {
		g_bTokenAllocationFailed = 1;
		return NULL;
	}
	return xrtRealloc(pMemory, iSize);
}

#define xrtMalloc xuiTestTokenMalloc
#define xrtRealloc xuiTestTokenRealloc
#include "../../src/xui/code/xui_code_token_buffer.c"
#undef xrtMalloc
#undef xrtRealloc

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_token_buffer_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

int main(void)
{
	xui_code_token_buffer pBuffer;
	xui_code_token_t arrIn[3];
	xui_code_token_t arrInvalid[3];
	xui_code_token_t arrLarge[17];
	xui_code_token_t arrOut[3];
	uint32_t iVersion;
	int iRangeStart;
	int iRangeEnd;
	int iCount;
	int i;
	int iFailed;
	int iRet;

	pBuffer = NULL;
	iFailed = 0;
	memset(arrIn, 0, sizeof(arrIn));
	memset(arrInvalid, 0, sizeof(arrInvalid));
	memset(arrLarge, 0, sizeof(arrLarge));
	memset(arrOut, 0, sizeof(arrOut));

	iRet = xuiCodeTokenBufferCreate(&pBuffer);
	XUI_TEST_CHECK(iRet == XUI_OK && pBuffer != NULL, "buffer create");
	XUI_TEST_CHECK(pBuffer->iTokenCapacity == 0, "buffer starts without token storage");
	arrIn[0].iStartOffset = 0;
	arrIn[0].iEndOffset = 3;
	arrIn[0].iKind = XUI_CODE_TOKEN_KEYWORD;
	arrIn[0].iStyle = XUI_CODE_STYLE_KEYWORD;
	arrIn[1].iStartOffset = 4;
	arrIn[1].iEndOffset = 8;
	arrIn[1].iKind = XUI_CODE_TOKEN_IDENTIFIER;
	arrIn[1].iStyle = XUI_CODE_STYLE_IDENTIFIER;
	arrIn[2].iStartOffset = 9;
	arrIn[2].iEndOffset = 10;
	arrIn[2].iKind = XUI_CODE_TOKEN_NUMBER;
	arrIn[2].iStyle = XUI_CODE_STYLE_NUMBER;
	iRet = xuiCodeTokenBufferSet(pBuffer, arrIn, 3, 42u);
	XUI_TEST_CHECK(iRet == XUI_OK, "buffer set");
	XUI_TEST_CHECK(xuiCodeTokenBufferGetCount(pBuffer) == 3, "buffer count");
	XUI_TEST_CHECK(pBuffer->iTokenCapacity == 16, "buffer uses amortized initial capacity");
	iRet = xuiCodeTokenBufferGetVersion(pBuffer, &iVersion);
	XUI_TEST_CHECK(iRet == XUI_OK && iVersion == 42u, "buffer version");
	iRet = xuiCodeTokenBufferGetTokens(pBuffer, 41u, arrOut, 3, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED && iCount == 0, "stale version rejected");
	iRet = xuiCodeTokenBufferGetTokens(pBuffer, 42u, arrOut, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 3, "get all count");
	XUI_TEST_CHECK(arrOut[0].iKind == XUI_CODE_TOKEN_KEYWORD && arrOut[1].iKind == XUI_CODE_TOKEN_IDENTIFIER, "get all contents");
	iRet = xuiCodeTokenBufferGetTokens(pBuffer, 42u, NULL, 0, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 3, "get all count only");
	iRet = xuiCodeTokenBufferGetTokens(pBuffer, 42u, NULL, 1, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "get all rejects missing output storage");
	iRet = xuiCodeTokenBufferGetTokensInRange(pBuffer, 42u, 2, 9, arrOut, 3, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "range count");
	XUI_TEST_CHECK(arrOut[0].iKind == XUI_CODE_TOKEN_KEYWORD && arrOut[1].iKind == XUI_CODE_TOKEN_IDENTIFIER, "range contents");
	iRet = xuiCodeTokenBufferGetTokensInRange(pBuffer, 42u, 2, 9, NULL, 0, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "range count only");
	iRet = xuiCodeTokenBufferGetTokensInRange(pBuffer, 42u, 2, 9, NULL, 1, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "range rejects missing output storage");
	iRet = xuiCodeTokenBufferGetTokensInRange(pBuffer, 42u, 5, 5, arrOut, 3, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 0, "empty range has no token hits");
	iRet = xuiCodeTokenBufferSetRange(pBuffer, arrIn + 1, 2, 43u, 4, 10);
	XUI_TEST_CHECK(iRet == XUI_OK, "range buffer set");
	iRet = xuiCodeTokenBufferGetRange(pBuffer, &iVersion, &iRangeStart, &iRangeEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iVersion == 43u && iRangeStart == 4 && iRangeEnd == 10, "range metadata");
	iRet = xuiCodeTokenBufferGetTokensInRange(pBuffer, 43u, 4, 10, arrOut, 3, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "covered range fetch");
	iRet = xuiCodeTokenBufferGetTokensInRange(pBuffer, 43u, 0, 10, arrOut, 3, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED && iCount == 0, "uncovered range rejected");
	arrInvalid[0] = arrIn[1];
	arrInvalid[1] = arrIn[0];
	arrInvalid[2] = arrIn[2];
	iRet = xuiCodeTokenBufferSetRange(pBuffer, arrInvalid, 3, 44u, 0, 10);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		xuiCodeTokenBufferGetCount(pBuffer) == 2,
		"unsorted tokens rejected without replacing cache");
	iRet = xuiCodeTokenBufferGetRange(pBuffer, &iVersion, &iRangeStart, &iRangeEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iVersion == 43u &&
		iRangeStart == 4 && iRangeEnd == 10,
		"invalid token order preserves metadata");
	arrInvalid[0] = arrIn[0];
	arrInvalid[0].iStartOffset = 3;
	arrInvalid[0].iEndOffset = 5;
	iRet = xuiCodeTokenBufferSetRange(pBuffer, arrInvalid, 1, 44u, 4, 10);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		xuiCodeTokenBufferGetCount(pBuffer) == 2,
		"token outside declared range rejected");
	for ( i = 0; i < (int)(sizeof(arrLarge) / sizeof(arrLarge[0])); i++ ) {
		arrLarge[i].iStartOffset = i * 2;
		arrLarge[i].iEndOffset = i * 2 + 1;
		arrLarge[i].iKind = XUI_CODE_TOKEN_IDENTIFIER;
		arrLarge[i].iStyle = XUI_CODE_STYLE_IDENTIFIER;
	}
	g_iTokenFailAllocation = 0;
	g_bTokenAllocationFailed = 0;
	iRet = xuiCodeTokenBufferSetRange(pBuffer, arrLarge,
		(int)(sizeof(arrLarge) / sizeof(arrLarge[0])), 44u, 0, 34);
	g_iTokenFailAllocation = -1;
	XUI_TEST_CHECK(iRet == XUI_ERROR_OUT_OF_MEMORY && g_bTokenAllocationFailed,
		"growth allocation failure reported");
	iRet = xuiCodeTokenBufferGetRange(pBuffer, &iVersion, &iRangeStart, &iRangeEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeTokenBufferGetCount(pBuffer) == 2 &&
		iVersion == 43u && iRangeStart == 4 && iRangeEnd == 10,
		"growth allocation failure preserves cache");
	iRet = xuiCodeTokenBufferSetRange(pBuffer, arrLarge,
		(int)(sizeof(arrLarge) / sizeof(arrLarge[0])), 44u, 0, 34);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeTokenBufferGetCount(pBuffer) == 17,
		"buffer grows after allocation recovers");
	XUI_TEST_CHECK(pBuffer->iTokenCapacity == 32,
		"buffer growth remains amortized");
	iRet = xuiCodeTokenBufferGetTokensInRange(pBuffer, 44u, 7, 29,
		arrOut, 3, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 11 &&
		arrOut[0].iStartOffset == 8 && arrOut[2].iStartOffset == 12,
		"range query returns total count and bounded prefix");
	xuiCodeTokenBufferClear(pBuffer);
	XUI_TEST_CHECK(xuiCodeTokenBufferGetCount(pBuffer) == 0, "buffer clear");
	iRet = xuiCodeTokenBufferGetRange(pBuffer, &iVersion, &iRangeStart, &iRangeEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iVersion == 0u &&
		iRangeStart == 0 && iRangeEnd == 0,
		"buffer clear resets metadata");

cleanup:
	xuiCodeTokenBufferDestroy(pBuffer);
	if ( iFailed ) return 1;
	printf("xui_code_token_buffer_test passed\n");
	return 0;
}
