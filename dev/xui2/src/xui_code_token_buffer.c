#include "../xui.h"

#include <string.h>

struct xui_code_token_buffer_t {
	xui_code_token_t* pTokens;
	int iTokenCount;
	int iTokenCapacity;
	uint32_t iTextVersion;
};

static int __xuiCodeTokenBufferReserve(xui_code_token_buffer pBuffer, int iCapacity)
{
	xui_code_token_t* pNew;

	if ( iCapacity <= pBuffer->iTokenCapacity ) return XUI_OK;
	pNew = (xui_code_token_t*)xrtRealloc(pBuffer->pTokens, sizeof(*pNew) * (size_t)iCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pBuffer->pTokens = pNew;
	pBuffer->iTokenCapacity = iCapacity;
	return XUI_OK;
}

static void __xuiCodeTokenBufferCopyOut(xui_code_token_t* pOut, const xui_code_token_t* pToken)
{
	*pOut = *pToken;
}

XUI_API int xuiCodeTokenBufferCreate(xui_code_token_buffer* ppBuffer)
{
	xui_code_token_buffer pBuffer;

	if ( ppBuffer == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppBuffer = NULL;
	pBuffer = (xui_code_token_buffer)xrtMalloc(sizeof(*pBuffer));
	if ( pBuffer == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pBuffer, 0, sizeof(*pBuffer));
	*ppBuffer = pBuffer;
	return XUI_OK;
}

XUI_API void xuiCodeTokenBufferDestroy(xui_code_token_buffer pBuffer)
{
	if ( pBuffer == NULL ) return;
	xrtFree(pBuffer->pTokens);
	xrtFree(pBuffer);
}

XUI_API void xuiCodeTokenBufferClear(xui_code_token_buffer pBuffer)
{
	if ( pBuffer == NULL ) return;
	pBuffer->iTokenCount = 0;
	pBuffer->iTextVersion = 0;
}

XUI_API int xuiCodeTokenBufferSet(xui_code_token_buffer pBuffer, const xui_code_token_t* pTokens, int iTokenCount, uint32_t iTextVersion)
{
	int i;
	int iRet;

	if ( (pBuffer == NULL) || (iTokenCount < 0) || (iTokenCount > 0 && pTokens == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < iTokenCount; i++ ) {
		if ( pTokens[i].iStartOffset < 0 || pTokens[i].iEndOffset <= pTokens[i].iStartOffset ) return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiCodeTokenBufferReserve(pBuffer, iTokenCount);
	if ( iRet != XUI_OK ) return iRet;
	if ( iTokenCount > 0 ) memcpy(pBuffer->pTokens, pTokens, sizeof(*pTokens) * (size_t)iTokenCount);
	pBuffer->iTokenCount = iTokenCount;
	pBuffer->iTextVersion = iTextVersion;
	return XUI_OK;
}

XUI_API int xuiCodeTokenBufferGetVersion(xui_code_token_buffer pBuffer, uint32_t* pTextVersion)
{
	if ( (pBuffer == NULL) || (pTextVersion == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*pTextVersion = pBuffer->iTextVersion;
	return XUI_OK;
}

XUI_API int xuiCodeTokenBufferGetCount(xui_code_token_buffer pBuffer)
{
	return (pBuffer != NULL) ? pBuffer->iTokenCount : 0;
}

XUI_API int xuiCodeTokenBufferGetTokens(xui_code_token_buffer pBuffer, uint32_t iTextVersion, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount)
{
	int i;

	if ( (pBuffer == NULL) || (pTokenCount == NULL) || (iTokenCapacity < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iTextVersion != pBuffer->iTextVersion ) {
		*pTokenCount = 0;
		return XUI_ERROR_UNSUPPORTED;
	}
	for ( i = 0; i < pBuffer->iTokenCount; i++ ) {
		if ( pTokens != NULL && i < iTokenCapacity ) __xuiCodeTokenBufferCopyOut(&pTokens[i], &pBuffer->pTokens[i]);
	}
	*pTokenCount = pBuffer->iTokenCount;
	return XUI_OK;
}

XUI_API int xuiCodeTokenBufferGetTokensInRange(xui_code_token_buffer pBuffer, uint32_t iTextVersion, int iStart, int iEnd, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount)
{
	int i;
	int iCount;

	if ( (pBuffer == NULL) || (pTokenCount == NULL) || (iStart < 0) || (iEnd < iStart) || (iTokenCapacity < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iTextVersion != pBuffer->iTextVersion ) {
		*pTokenCount = 0;
		return XUI_ERROR_UNSUPPORTED;
	}
	iCount = 0;
	for ( i = 0; i < pBuffer->iTokenCount; i++ ) {
		if ( pBuffer->pTokens[i].iEndOffset <= iStart || pBuffer->pTokens[i].iStartOffset >= iEnd ) continue;
		if ( pTokens != NULL && iCount < iTokenCapacity ) __xuiCodeTokenBufferCopyOut(&pTokens[iCount], &pBuffer->pTokens[i]);
		iCount++;
	}
	*pTokenCount = iCount;
	return XUI_OK;
}
