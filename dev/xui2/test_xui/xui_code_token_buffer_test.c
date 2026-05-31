#include "xui.h"

#include <stdio.h>

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
	xui_code_token_t arrOut[3];
	uint32_t iVersion;
	int iCount;
	int iFailed;
	int iRet;

	pBuffer = NULL;
	iFailed = 0;

	iRet = xuiCodeTokenBufferCreate(&pBuffer);
	XUI_TEST_CHECK(iRet == XUI_OK && pBuffer != NULL, "buffer create");
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
	iRet = xuiCodeTokenBufferGetVersion(pBuffer, &iVersion);
	XUI_TEST_CHECK(iRet == XUI_OK && iVersion == 42u, "buffer version");
	iRet = xuiCodeTokenBufferGetTokens(pBuffer, 41u, arrOut, 3, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED && iCount == 0, "stale version rejected");
	iRet = xuiCodeTokenBufferGetTokens(pBuffer, 42u, arrOut, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 3, "get all count");
	XUI_TEST_CHECK(arrOut[0].iKind == XUI_CODE_TOKEN_KEYWORD && arrOut[1].iKind == XUI_CODE_TOKEN_IDENTIFIER, "get all contents");
	iRet = xuiCodeTokenBufferGetTokensInRange(pBuffer, 42u, 2, 9, arrOut, 3, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "range count");
	XUI_TEST_CHECK(arrOut[0].iKind == XUI_CODE_TOKEN_KEYWORD && arrOut[1].iKind == XUI_CODE_TOKEN_IDENTIFIER, "range contents");
	xuiCodeTokenBufferClear(pBuffer);
	XUI_TEST_CHECK(xuiCodeTokenBufferGetCount(pBuffer) == 0, "buffer clear");

cleanup:
	xuiCodeTokenBufferDestroy(pBuffer);
	if ( iFailed ) return 1;
	printf("xui_code_token_buffer_test passed\n");
	return 0;
}
