static int __xgeXuiTextEnsure(xge_xui_text pText, int iNeed)
{
	char* sNew;
	int iCapacity;

	if ( pText == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeed <= pText->iCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pText->iCapacity > 0) ? pText->iCapacity : 32;
	while ( iCapacity < iNeed ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	sNew = (char*)xrtRealloc(pText->sText, (size_t)iCapacity);
	if ( sNew == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pText->sText = sNew;
	pText->iCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeXuiTextCompositionEnsure(xge_xui_text pText, int iNeed)
{
	char* sNew;
	int iCapacity;

	if ( pText == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeed <= pText->iCompositionCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pText->iCompositionCapacity > 0) ? pText->iCompositionCapacity : 32;
	while ( iCapacity < iNeed ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	sNew = (char*)xrtRealloc(pText->sComposition, (size_t)iCapacity);
	if ( sNew == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pText->sComposition = sNew;
	pText->iCompositionCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeXuiTextUTF8Encode(uint32_t iCodepoint, char* sOut)
{
	if ( sOut == NULL ) {
		return 0;
	}
	if ( iCodepoint < 0x80 ) {
		sOut[0] = (char)iCodepoint;
		return 1;
	}
	if ( iCodepoint < 0x800 ) {
		sOut[0] = (char)(0xC0 | ((iCodepoint >> 6) & 0x1F));
		sOut[1] = (char)(0x80 | (iCodepoint & 0x3F));
		return 2;
	}
	if ( (iCodepoint < 0x10000) && ((iCodepoint < 0xD800) || (iCodepoint > 0xDFFF)) ) {
		sOut[0] = (char)(0xE0 | ((iCodepoint >> 12) & 0x0F));
		sOut[1] = (char)(0x80 | ((iCodepoint >> 6) & 0x3F));
		sOut[2] = (char)(0x80 | (iCodepoint & 0x3F));
		return 3;
	}
	return 0;
}

static int __xgeXuiTextNormalizedSize(const char* sText)
{
	int iSize;

	if ( sText == NULL ) {
		return 0;
	}
	iSize = 0;
	while ( *sText != 0 ) {
		if ( *sText == '\r' ) {
			iSize++;
			if ( sText[1] == '\n' ) {
				sText++;
			}
		}
		else {
			iSize++;
		}
		sText++;
	}
	return iSize;
}

static void __xgeXuiTextCopyNormalized(char* sOut, const char* sText)
{
	if ( (sOut == NULL) || (sText == NULL) ) {
		return;
	}
	while ( *sText != 0 ) {
		if ( *sText == '\r' ) {
			*sOut++ = '\n';
			if ( sText[1] == '\n' ) {
				sText++;
			}
		}
		else {
			*sOut++ = *sText;
		}
		sText++;
	}
	*sOut = 0;
}

static void __xgeXuiTextCopyNormalizedInsert(char* sOut, const char* sText)
{
	if ( (sOut == NULL) || (sText == NULL) ) {
		return;
	}
	while ( *sText != 0 ) {
		if ( *sText == '\r' ) {
			*sOut++ = '\n';
			if ( sText[1] == '\n' ) {
				sText++;
			}
		}
		else {
			*sOut++ = *sText;
		}
		sText++;
	}
}

static int __xgeXuiTextClampCursor(xge_xui_text pText, int iCursor)
{
	if ( (pText == NULL) || (pText->sText == NULL) ) {
		return 0;
	}
	if ( iCursor < 0 ) {
		return 0;
	}
	if ( iCursor > pText->iSize ) {
		return pText->iSize;
	}
	while ( (iCursor > 0) && (((unsigned char)pText->sText[iCursor] & 0xC0) == 0x80) ) {
		iCursor--;
	}
	return iCursor;
}

static int __xgeXuiTextPrevCursor(xge_xui_text pText, int iCursor)
{
	iCursor = __xgeXuiTextClampCursor(pText, iCursor);
	if ( iCursor <= 0 ) {
		return 0;
	}
	iCursor--;
	while ( (iCursor > 0) && (((unsigned char)pText->sText[iCursor] & 0xC0) == 0x80) ) {
		iCursor--;
	}
	return iCursor;
}

static int __xgeXuiTextNextCursor(xge_xui_text pText, int iCursor)
{
	iCursor = __xgeXuiTextClampCursor(pText, iCursor);
	if ( (pText == NULL) || (pText->sText == NULL) || (iCursor >= pText->iSize) ) {
		return iCursor;
	}
	iCursor++;
	while ( (iCursor < pText->iSize) && (((unsigned char)pText->sText[iCursor] & 0xC0) == 0x80) ) {
		iCursor++;
	}
	return iCursor;
}

static void __xgeXuiTextSelectionRange(xge_xui_text pText, int* pStart, int* pEnd)
{
	int iStart;
	int iEnd;

	iStart = 0;
	iEnd = 0;
	if ( pText != NULL ) {
		iStart = __xgeXuiTextClampCursor(pText, pText->iSelectStart);
		iEnd = __xgeXuiTextClampCursor(pText, pText->iSelectEnd);
	}
	if ( iStart > iEnd ) {
		int iTmp = iStart;
		iStart = iEnd;
		iEnd = iTmp;
	}
	if ( pStart != NULL ) {
		*pStart = iStart;
	}
	if ( pEnd != NULL ) {
		*pEnd = iEnd;
	}
}

static float __xgeXuiTextPrefixWidth(xge_font pFont, const char* sText, int iBytes)
{
	char* sCopy;
	xge_vec2_t tSize;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( (pFont == NULL) || (sText == NULL) || (iBytes <= 0) ) {
		return 0.0f;
	}
	sCopy = (char*)xrtMalloc((size_t)iBytes + 1);
	if ( sCopy == NULL ) {
		return 0.0f;
	}
	memcpy(sCopy, sText, (size_t)iBytes);
	sCopy[iBytes] = 0;
	tSize = __xgeXuiHostMeasureText(pFont, sCopy);
	xrtFree(sCopy);
	return tSize.fX;
}

static float __xgeXuiInputPasswordDisplayWidth(xge_xui_input pInput, int iLimitBytes)
{
	const char* sText;
	char* sOut;
	xge_vec2_t tSize;
	int i;
	int iCount;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( (pInput == NULL) || (pInput->pFont == NULL) ) {
		return 0.0f;
	}
	sText = xgeXuiInputGetText(pInput);
	if ( sText == NULL ) {
		return 0.0f;
	}
	if ( iLimitBytes < 0 || iLimitBytes > pInput->tText.iSize ) {
		iLimitBytes = pInput->tText.iSize;
	}
	iCount = 0;
	for ( i = 0; sText[i] != 0 && i < iLimitBytes; i++ ) {
		if ( ((unsigned char)sText[i] & 0xC0) != 0x80 ) {
			iCount++;
		}
	}
	if ( iCount <= 0 ) {
		return 0.0f;
	}
	sOut = (char*)xrtMalloc((size_t)iCount + 1);
	if ( sOut == NULL ) {
		return 0.0f;
	}
	for ( i = 0; i < iCount; i++ ) {
		sOut[i] = '*';
	}
	sOut[i] = 0;
	tSize = __xgeXuiHostMeasureText(pInput->pFont, sOut);
	xrtFree(sOut);
	return tSize.fX;
}

static float __xgeXuiInputDisplayTextWidthForAlign(xge_xui_input pInput)
{
	if ( pInput == NULL ) {
		return 0.0f;
	}
	if ( pInput->bPassword != 0 ) {
		return __xgeXuiInputPasswordDisplayWidth(pInput, -1);
	}
	return __xgeXuiTextPrefixWidth(pInput->pFont, xgeXuiInputGetText(pInput), pInput->tText.iSize);
}

static float __xgeXuiInputTextAlignOffset(xge_xui_input pInput, float fTextW)
{
	float fRemain;

	if ( (pInput == NULL) || (pInput->pWidget == NULL) || (pInput->fScrollX > 0.0f) ) {
		return 0.0f;
	}
	fRemain = pInput->pWidget->tContentRect.fW - fTextW;
	if ( fRemain <= 0.0f ) {
		return 0.0f;
	}
	if ( pInput->iTextAlign == XGE_XUI_INPUT_TEXT_ALIGN_RIGHT ) {
		return fRemain;
	}
	if ( pInput->iTextAlign == XGE_XUI_INPUT_TEXT_ALIGN_CENTER ) {
		return fRemain * 0.5f;
	}
	return 0.0f;
}

static int __xgeXuiInputCursorFromX(xge_xui_input pInput, float fX)
{
	const char* sText;
	float fLocalX;
	float fTextW;
	float fPrevWidth;
	float fWidth;
	int iPrev;
	int iCursor;
	int iNext;

	if ( (pInput == NULL) || (pInput->tText.sText == NULL) || (pInput->pWidget == NULL) ) {
		return 0;
	}
	sText = pInput->tText.sText;
	fTextW = __xgeXuiInputDisplayTextWidthForAlign(pInput);
	fLocalX = fX - pInput->pWidget->tContentRect.fX - __xgeXuiInputTextAlignOffset(pInput, fTextW) + pInput->fScrollX;
	if ( fLocalX <= 0.0f ) {
		return 0;
	}
	iPrev = 0;
	fPrevWidth = 0.0f;
	iCursor = 0;
	while ( iCursor < pInput->tText.iSize ) {
		iNext = __xgeXuiTextNextCursor(&pInput->tText, iCursor);
		if ( iNext <= iCursor ) {
			break;
		}
		fWidth = (pInput->bPassword != 0) ? __xgeXuiInputPasswordDisplayWidth(pInput, iNext) : __xgeXuiTextPrefixWidth(pInput->pFont, sText, iNext);
		if ( fLocalX < ((fPrevWidth + fWidth) * 0.5f) ) {
			return iPrev;
		}
		iPrev = iNext;
		fPrevWidth = fWidth;
		iCursor = iNext;
	}
	return pInput->tText.iSize;
}

static int __xgeXuiTextSelectionDelete(xge_xui_text pText)
{
	int iStart;
	int iEnd;
	int iRemain;

	if ( pText == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeXuiTextSelectionRange(pText, &iStart, &iEnd);
	if ( (iStart < 0) || (iEnd > pText->iSize) || (iStart == iEnd) ) {
		pText->iSelectStart = pText->iCursor;
		pText->iSelectEnd = pText->iCursor;
		return XGE_OK;
	}
	iRemain = pText->iSize - iEnd;
	memmove(pText->sText + iStart, pText->sText + iEnd, (size_t)iRemain + 1);
	pText->iSize -= iEnd - iStart;
	pText->iCursor = iStart;
	pText->iSelectStart = iStart;
	pText->iSelectEnd = iStart;
	return XGE_OK;
}

int xgeXuiTextInit(xge_xui_text pText)
{
	int iRet;

	if ( pText == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pText, 0, sizeof(*pText));
	iRet = __xgeXuiTextEnsure(pText, 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pText->sText[0] = 0;
	iRet = __xgeXuiTextCompositionEnsure(pText, 1);
	if ( iRet != XGE_OK ) {
		xrtFree(pText->sText);
		memset(pText, 0, sizeof(*pText));
		return iRet;
	}
	pText->sComposition[0] = 0;
	return XGE_OK;
}

void xgeXuiTextUnit(xge_xui_text pText)
{
	if ( pText == NULL ) {
		return;
	}
	if ( pText->sText != NULL ) {
		xrtFree(pText->sText);
	}
	if ( pText->sComposition != NULL ) {
		xrtFree(pText->sComposition);
	}
	memset(pText, 0, sizeof(*pText));
}

int xgeXuiTextSet(xge_xui_text pText, const char* sText)
{
	int iSize;
	int iRet;

	if ( pText == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( sText == NULL ) {
		sText = "";
	}
	iSize = __xgeXuiTextNormalizedSize(sText);
	iRet = __xgeXuiTextEnsure(pText, iSize + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	__xgeXuiTextCopyNormalized(pText->sText, sText);
	pText->iSize = iSize;
	pText->iCursor = iSize;
	pText->iSelectStart = iSize;
	pText->iSelectEnd = iSize;
	xgeXuiTextClearComposition(pText);
	return XGE_OK;
}

int xgeXuiTextInsert(xge_xui_text pText, const char* sText)
{
	int iSize;
	int iRet;
	char* sInsert;

	if ( (pText == NULL) || (sText == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pText->sText == NULL ) {
		iRet = xgeXuiTextInit(pText);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	iRet = __xgeXuiTextSelectionDelete(pText);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iSize = __xgeXuiTextNormalizedSize(sText);
	iRet = __xgeXuiTextEnsure(pText, pText->iSize + iSize + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( pText->iCursor < 0 ) {
		pText->iCursor = 0;
	}
	if ( pText->iCursor > pText->iSize ) {
		pText->iCursor = pText->iSize;
	}
	memmove(pText->sText + pText->iCursor + iSize, pText->sText + pText->iCursor, (size_t)(pText->iSize - pText->iCursor) + 1);
	sInsert = pText->sText + pText->iCursor;
	__xgeXuiTextCopyNormalizedInsert(sInsert, sText);
	pText->iCursor += iSize;
	pText->iSize += iSize;
	pText->iSelectStart = pText->iCursor;
	pText->iSelectEnd = pText->iCursor;
	xgeXuiTextClearComposition(pText);
	return XGE_OK;
}

int xgeXuiTextInsertCodepoint(xge_xui_text pText, uint32_t iCodepoint)
{
	char arrText[4];
	int iSize;

	iSize = __xgeXuiTextUTF8Encode(iCodepoint, arrText);
	if ( iSize <= 0 ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	arrText[iSize] = 0;
	return xgeXuiTextInsert(pText, arrText);
}

int xgeXuiTextDeleteBack(xge_xui_text pText)
{
	int iStart;

	if ( (pText == NULL) || (pText->sText == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pText->iSelectStart != pText->iSelectEnd ) {
		return __xgeXuiTextSelectionDelete(pText);
	}
	if ( pText->iCursor <= 0 ) {
		return XGE_OK;
	}
	if ( pText->iCursor > pText->iSize ) {
		pText->iCursor = pText->iSize;
	}
	iStart = pText->iCursor - 1;
	while ( (iStart > 0) && (((unsigned char)pText->sText[iStart] & 0xC0) == 0x80) ) {
		iStart--;
	}
	memmove(pText->sText + iStart, pText->sText + pText->iCursor, (size_t)(pText->iSize - pText->iCursor) + 1);
	pText->iSize -= pText->iCursor - iStart;
	pText->iCursor = iStart;
	pText->iSelectStart = iStart;
	pText->iSelectEnd = iStart;
	xgeXuiTextClearComposition(pText);
	return XGE_OK;
}

int xgeXuiTextDeleteForward(xge_xui_text pText)
{
	int iEnd;

	if ( (pText == NULL) || (pText->sText == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pText->iSelectStart != pText->iSelectEnd ) {
		return __xgeXuiTextSelectionDelete(pText);
	}
	pText->iCursor = __xgeXuiTextClampCursor(pText, pText->iCursor);
	if ( pText->iCursor >= pText->iSize ) {
		return XGE_OK;
	}
	iEnd = __xgeXuiTextNextCursor(pText, pText->iCursor);
	if ( iEnd <= pText->iCursor ) {
		return XGE_OK;
	}
	memmove(pText->sText + pText->iCursor, pText->sText + iEnd, (size_t)(pText->iSize - iEnd) + 1);
	pText->iSize -= iEnd - pText->iCursor;
	pText->iSelectStart = pText->iCursor;
	pText->iSelectEnd = pText->iCursor;
	xgeXuiTextClearComposition(pText);
	return XGE_OK;
}

void xgeXuiTextSetCursor(xge_xui_text pText, int iCursor)
{
	if ( pText == NULL ) {
		return;
	}
	pText->iCursor = __xgeXuiTextClampCursor(pText, iCursor);
	pText->iSelectStart = pText->iCursor;
	pText->iSelectEnd = pText->iCursor;
}

int xgeXuiTextGetCursor(xge_xui_text pText)
{
	if ( pText == NULL ) {
		return 0;
	}
	pText->iCursor = __xgeXuiTextClampCursor(pText, pText->iCursor);
	return pText->iCursor;
}

void xgeXuiTextSetSelection(xge_xui_text pText, int iStart, int iEnd)
{
	if ( pText == NULL ) {
		return;
	}
	pText->iSelectStart = __xgeXuiTextClampCursor(pText, iStart);
	pText->iSelectEnd = __xgeXuiTextClampCursor(pText, iEnd);
	pText->iCursor = pText->iSelectEnd;
}

void xgeXuiTextGetSelection(xge_xui_text pText, int* pStart, int* pEnd)
{
	__xgeXuiTextSelectionRange(pText, pStart, pEnd);
}

int xgeXuiTextSetComposition(xge_xui_text pText, const char* sText)
{
	int iSize;
	int iRet;

	if ( pText == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( sText == NULL ) {
		sText = "";
	}
	iSize = (int)strlen(sText);
	iRet = __xgeXuiTextCompositionEnsure(pText, iSize + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	memcpy(pText->sComposition, sText, (size_t)iSize + 1);
	pText->iCompositionSize = iSize;
	return XGE_OK;
}

void xgeXuiTextClearComposition(xge_xui_text pText)
{
	if ( pText == NULL ) {
		return;
	}
	if ( pText->sComposition != NULL ) {
		pText->sComposition[0] = 0;
	}
	pText->iCompositionSize = 0;
}

const char* xgeXuiTextGetComposition(xge_xui_text pText)
{
	if ( (pText == NULL) || (pText->sComposition == NULL) ) {
		return "";
	}
	return pText->sComposition;
}

int xgeXuiTextInputEvent(xge_xui_text pText, const xge_event_t* pEvent)
{
	const xge_ime_event_t* pIme;

	if ( (pText == NULL) || (pEvent == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pEvent->iType == XGE_EVENT_IME_START ) {
		xgeXuiTextClearComposition(pText);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pEvent->iType == XGE_EVENT_IME_UPDATE ) {
		pIme = (const xge_ime_event_t*)pEvent->pData;
		if ( pIme != NULL ) {
			return (xgeXuiTextSetComposition(pText, pIme->sText) == XGE_OK) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		}
		return (xgeXuiTextSetComposition(pText, "") == XGE_OK) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_IME_END ) {
		xgeXuiTextClearComposition(pText);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pEvent->iType != XGE_EVENT_TEXT ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iCodepoint == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( xgeXuiTextInsertCodepoint(pText, pEvent->iCodepoint) != XGE_OK ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	return XGE_XUI_EVENT_CONSUMED;
}
