static int __xgeXuiTextEditLineIndex(xge_xui_text_edit pEdit, int iCursor);
static int __xgeXuiTextEditLineCount(xge_xui_text_edit pEdit);
static int __xgeXuiTextEditLineStartByIndex(xge_xui_text_edit pEdit, int iLine);
static int __xgeXuiTextEditVisualLineIndex(xge_xui_text_edit pEdit, int iCursor);
static int __xgeXuiTextEditVisualLineCount(xge_xui_text_edit pEdit);
static int __xgeXuiTextEditVisualLineStart(xge_xui_text_edit pEdit, int iLine);
static int __xgeXuiTextEditVisualLineEnd(xge_xui_text_edit pEdit, int iLine);
static void __xgeXuiTextEditEnsureCursorVisible(xge_xui_text_edit pEdit);
static void __xgeXuiTextEditResetBlink(xge_xui_text_edit pEdit);
static float __xgeXuiTextEditLineNumberWidth(xge_xui_text_edit pEdit);

enum {
	XGE_XUI_TEXT_EDIT_MENU_SELECT_ALL = 0,
	XGE_XUI_TEXT_EDIT_MENU_CUT,
	XGE_XUI_TEXT_EDIT_MENU_COPY,
	XGE_XUI_TEXT_EDIT_MENU_PASTE,
	XGE_XUI_TEXT_EDIT_MENU_DELETE,
	XGE_XUI_TEXT_EDIT_MENU_COUNT
};

static const char* g_arrXgeXuiTextEditMenuItems[XGE_XUI_TEXT_EDIT_MENU_COUNT] = {
	"Select All",
	"Cut",
	"Copy",
	"Paste",
	"Delete"
};

static void __xgeXuiTextEditMenuSelect(xge_xui_widget pWidget, int iIndex, void* pUser);
static void __xgeXuiTextEditOpenDefaultMenu(xge_xui_text_edit pEdit, float fX, float fY);

static xge_rect_t __xgeXuiTextEditImeCandidateRect(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	return xgeXuiTextEditGetCandidateRect((xge_xui_text_edit)pUser);
}

static float __xgeXuiTextEditLineHeight(xge_xui_text_edit pEdit)
{
	xge_vec2_t tSize;

	if ( pEdit == NULL ) {
		return 20.0f;
	}
	if ( pEdit->fLineHeight > 0.0f ) {
		return pEdit->fLineHeight;
	}
	if ( (pEdit->pFont != NULL) && (pEdit->pFont->fLineHeight > 1.0f) ) {
		pEdit->fLineHeight = pEdit->pFont->fLineHeight;
		return pEdit->fLineHeight;
	}
	tSize = __xgeXuiHostMeasureText(pEdit->pFont, "Mg");
	pEdit->fLineHeight = (tSize.fY > 1.0f) ? tSize.fY : 20.0f;
	return pEdit->fLineHeight;
}

static void __xgeXuiTextEditMarkLineCache(xge_xui_text_edit pEdit)
{
	if ( pEdit != NULL ) {
		pEdit->bLineCacheDirty = 1;
		pEdit->bVisualCacheDirty = 1;
	}
}

static int __xgeXuiTextEditEnsureLineCache(xge_xui_text_edit pEdit)
{
	int i;
	int iCount;
	int iCapacity;
	int* arrNew;

	if ( pEdit == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pEdit->bLineCacheDirty == 0 && pEdit->arrLineStarts != NULL && pEdit->iLineCount > 0 ) {
		return XGE_OK;
	}
	iCount = 1;
	if ( pEdit->tText.sText != NULL ) {
		for ( i = 0; i < pEdit->tText.iSize; i++ ) {
			if ( pEdit->tText.sText[i] == '\n' ) {
				iCount++;
			}
		}
	}
	if ( iCount > pEdit->iLineCapacity ) {
		iCapacity = (pEdit->iLineCapacity > 0) ? pEdit->iLineCapacity : 8;
		while ( iCapacity < iCount ) {
			iCapacity *= 2;
		}
		arrNew = (int*)xrtRealloc(pEdit->arrLineStarts, sizeof(int) * (size_t)iCapacity);
		if ( arrNew == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pEdit->arrLineStarts = arrNew;
		pEdit->iLineCapacity = iCapacity;
	}
	pEdit->arrLineStarts[0] = 0;
	iCount = 1;
	if ( pEdit->tText.sText != NULL ) {
		for ( i = 0; i < pEdit->tText.iSize; i++ ) {
			if ( pEdit->tText.sText[i] == '\n' ) {
				pEdit->arrLineStarts[iCount++] = i + 1;
			}
		}
	}
	pEdit->iLineCount = iCount;
	pEdit->bLineCacheDirty = 0;
	return XGE_OK;
}

static void __xgeXuiTextEditStateFree(xge_xui_text_edit_state_t* pState)
{
	if ( pState == NULL ) {
		return;
	}
	if ( pState->sText != NULL ) {
		xrtFree(pState->sText);
	}
	memset(pState, 0, sizeof(*pState));
}

static void __xgeXuiTextEditStackClear(xge_xui_text_edit_state_t* arrStack, int iCount)
{
	int i;

	if ( arrStack == NULL ) {
		return;
	}
	for ( i = 0; i < iCount; i++ ) {
		__xgeXuiTextEditStateFree(&arrStack[i]);
	}
}

static int __xgeXuiTextEditStateCapture(xge_xui_text_edit pEdit, xge_xui_text_edit_state_t* pState)
{
	const char* sText;
	int iSize;

	if ( pState == NULL || pEdit == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pState, 0, sizeof(*pState));
	sText = (pEdit->tText.sText != NULL) ? pEdit->tText.sText : "";
	iSize = (int)strlen(sText);
	pState->sText = (char*)xrtMalloc((size_t)iSize + 1);
	if ( pState->sText == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(pState->sText, sText, (size_t)iSize + 1);
	pState->iCursor = pEdit->tText.iCursor;
	pState->iSelectStart = pEdit->tText.iSelectStart;
	pState->iSelectEnd = pEdit->tText.iSelectEnd;
	return XGE_OK;
}

static int __xgeXuiTextEditStackPush(xge_xui_text_edit_state_t** ppStack, int* pCount, int* pCapacity, int iLimit, xge_xui_text_edit_state_t* pState)
{
	xge_xui_text_edit_state_t* arrNew;
	int iCapacity;

	if ( ppStack == NULL || pCount == NULL || pCapacity == NULL || pState == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iLimit <= 0 ) {
		__xgeXuiTextEditStateFree(pState);
		return XGE_OK;
	}
	if ( *pCount >= iLimit ) {
		__xgeXuiTextEditStateFree(&(*ppStack)[0]);
		memmove(*ppStack, *ppStack + 1, sizeof(**ppStack) * (size_t)(*pCount - 1));
		(*pCount)--;
	}
	if ( *pCount >= *pCapacity ) {
		iCapacity = (*pCapacity > 0) ? (*pCapacity * 2) : 16;
		if ( iCapacity < *pCount + 1 ) {
			iCapacity = *pCount + 1;
		}
		arrNew = (xge_xui_text_edit_state_t*)xrtRealloc(*ppStack, sizeof(**ppStack) * (size_t)iCapacity);
		if ( arrNew == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		*ppStack = arrNew;
		*pCapacity = iCapacity;
	}
	(*ppStack)[*pCount] = *pState;
	(*pCount)++;
	memset(pState, 0, sizeof(*pState));
	return XGE_OK;
}

static void __xgeXuiTextEditRedoClear(xge_xui_text_edit pEdit)
{
	if ( pEdit == NULL ) {
		return;
	}
	__xgeXuiTextEditStackClear(pEdit->arrRedo, pEdit->iRedoCount);
	pEdit->iRedoCount = 0;
}

static int __xgeXuiTextEditRecordUndo(xge_xui_text_edit pEdit)
{
	xge_xui_text_edit_state_t tState;
	int iRet;

	if ( pEdit == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeXuiTextEditStateCapture(pEdit, &tState);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeXuiTextEditStackPush(&pEdit->arrUndo, &pEdit->iUndoCount, &pEdit->iUndoCapacity, pEdit->iUndoLimit, &tState);
	if ( iRet != XGE_OK ) {
		__xgeXuiTextEditStateFree(&tState);
		return iRet;
	}
	__xgeXuiTextEditRedoClear(pEdit);
	return XGE_OK;
}

static int __xgeXuiTextEditRestoreState(xge_xui_text_edit pEdit, const xge_xui_text_edit_state_t* pState)
{
	if ( pEdit == NULL || pState == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiTextSet(&pEdit->tText, (pState->sText != NULL) ? pState->sText : "") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pEdit->tText.iCursor = __xgeXuiTextClampCursor(&pEdit->tText, pState->iCursor);
	pEdit->tText.iSelectStart = __xgeXuiTextClampCursor(&pEdit->tText, pState->iSelectStart);
	pEdit->tText.iSelectEnd = __xgeXuiTextClampCursor(&pEdit->tText, pState->iSelectEnd);
	__xgeXuiTextEditMarkLineCache(pEdit);
	pEdit->iSelectionAnchor = pEdit->tText.iSelectStart;
	__xgeXuiTextEditEnsureCursorVisible(pEdit);
	__xgeXuiTextEditResetBlink(pEdit);
	if ( pEdit->pWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pEdit->pWidget);
	}
	return XGE_OK;
}

static int __xgeXuiTextEditLineStart(xge_xui_text_edit pEdit, int iCursor)
{
	if ( (pEdit == NULL) || (pEdit->tText.sText == NULL) ) {
		return 0;
	}
	iCursor = __xgeXuiTextClampCursor(&pEdit->tText, iCursor);
	return __xgeXuiTextEditLineStartByIndex(pEdit, __xgeXuiTextEditLineIndex(pEdit, iCursor));
}

static int __xgeXuiTextEditLineEnd(xge_xui_text_edit pEdit, int iCursor)
{
	int iLine;
	int iLineCount;

	if ( (pEdit == NULL) || (pEdit->tText.sText == NULL) ) {
		return 0;
	}
	iCursor = __xgeXuiTextClampCursor(&pEdit->tText, iCursor);
	iLine = __xgeXuiTextEditLineIndex(pEdit, iCursor);
	iLineCount = __xgeXuiTextEditLineCount(pEdit);
	if ( iLine + 1 < iLineCount ) {
		return __xgeXuiTextEditLineStartByIndex(pEdit, iLine + 1) - 1;
	}
	return pEdit->tText.iSize;
}

static int __xgeXuiTextEditLineIndex(xge_xui_text_edit pEdit, int iCursor)
{
	int iLow;
	int iHigh;
	int iMid;

	if ( (pEdit == NULL) || (pEdit->tText.sText == NULL) ) {
		return 0;
	}
	iCursor = __xgeXuiTextClampCursor(&pEdit->tText, iCursor);
	if ( __xgeXuiTextEditEnsureLineCache(pEdit) != XGE_OK ) {
		return 0;
	}
	iLow = 0;
	iHigh = pEdit->iLineCount - 1;
	while ( iLow <= iHigh ) {
		iMid = (iLow + iHigh) / 2;
		if ( pEdit->arrLineStarts[iMid] <= iCursor ) {
			iLow = iMid + 1;
		} else {
			iHigh = iMid - 1;
		}
	}
	if ( iHigh < 0 ) {
		return 0;
	}
	return iHigh;
}

static int __xgeXuiTextEditLineCount(xge_xui_text_edit pEdit)
{
	if ( pEdit == NULL ) {
		return 1;
	}
	if ( __xgeXuiTextEditEnsureLineCache(pEdit) != XGE_OK ) {
		return 1;
	}
	return (pEdit->iLineCount > 0) ? pEdit->iLineCount : 1;
}

static int __xgeXuiTextEditLineStartByIndex(xge_xui_text_edit pEdit, int iLine)
{
	if ( pEdit == NULL || iLine <= 0 ) {
		return 0;
	}
	if ( __xgeXuiTextEditEnsureLineCache(pEdit) != XGE_OK ) {
		return 0;
	}
	if ( iLine < pEdit->iLineCount ) {
		return pEdit->arrLineStarts[iLine];
	}
	return pEdit->tText.iSize;
}

static int __xgeXuiTextEditVisualLinePush(xge_xui_text_edit pEdit, int iStart, int iEnd)
{
	xge_xui_text_edit_visual_line_t* arrNew;
	int iCapacity;

	if ( pEdit == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pEdit->iVisualLineCount >= pEdit->iVisualLineCapacity ) {
		iCapacity = (pEdit->iVisualLineCapacity > 0) ? (pEdit->iVisualLineCapacity * 2) : 16;
		arrNew = (xge_xui_text_edit_visual_line_t*)xrtRealloc(pEdit->arrVisualLines, sizeof(*pEdit->arrVisualLines) * (size_t)iCapacity);
		if ( arrNew == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pEdit->arrVisualLines = arrNew;
		pEdit->iVisualLineCapacity = iCapacity;
	}
	pEdit->arrVisualLines[pEdit->iVisualLineCount].iStart = iStart;
	pEdit->arrVisualLines[pEdit->iVisualLineCount].iEnd = iEnd;
	pEdit->iVisualLineCount++;
	return XGE_OK;
}

static int __xgeXuiTextEditVisualCacheRebuild(xge_xui_text_edit pEdit)
{
	float fWrapWidth;
	float fWidth;
	int iLine;
	int iLineCount;
	int iLineStart;
	int iLineEnd;
	int iSegmentStart;
	int iCursor;
	int iNext;
	int iSplit;

	if ( pEdit == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiTextEditEnsureLineCache(pEdit) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pEdit->iVisualLineCount = 0;
	iLineCount = __xgeXuiTextEditLineCount(pEdit);
	fWrapWidth = (pEdit->pWidget != NULL) ? pEdit->pWidget->tContentRect.fW : 0.0f;
	if ( pEdit->bWordWrap == 0 || fWrapWidth <= 8.0f ) {
		for ( iLine = 0; iLine < iLineCount; iLine++ ) {
			iLineStart = __xgeXuiTextEditLineStartByIndex(pEdit, iLine);
			iLineEnd = __xgeXuiTextEditLineEnd(pEdit, iLineStart);
			if ( __xgeXuiTextEditVisualLinePush(pEdit, iLineStart, iLineEnd) != XGE_OK ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
		}
		pEdit->bVisualCacheDirty = 0;
		return XGE_OK;
	}
	for ( iLine = 0; iLine < iLineCount; iLine++ ) {
		iLineStart = __xgeXuiTextEditLineStartByIndex(pEdit, iLine);
		iLineEnd = __xgeXuiTextEditLineEnd(pEdit, iLineStart);
		iSegmentStart = iLineStart;
		iCursor = iLineStart;
		while ( iCursor < iLineEnd ) {
			iNext = __xgeXuiTextNextCursor(&pEdit->tText, iCursor);
			if ( iNext <= iCursor ) {
				break;
			}
			fWidth = __xgeXuiTextPrefixWidth(pEdit->pFont, pEdit->tText.sText + iSegmentStart, iNext - iSegmentStart);
			if ( fWidth > fWrapWidth && iCursor > iSegmentStart ) {
				iSplit = iCursor;
				if ( __xgeXuiTextEditVisualLinePush(pEdit, iSegmentStart, iSplit) != XGE_OK ) {
					return XGE_ERROR_OUT_OF_MEMORY;
				}
				iSegmentStart = iSplit;
			} else {
				iCursor = iNext;
			}
		}
		if ( __xgeXuiTextEditVisualLinePush(pEdit, iSegmentStart, iLineEnd) != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	pEdit->bVisualCacheDirty = 0;
	return XGE_OK;
}

static int __xgeXuiTextEditEnsureVisualCache(xge_xui_text_edit pEdit)
{
	float fWidth;

	if ( pEdit == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	fWidth = (pEdit->pWidget != NULL) ? pEdit->pWidget->tContentRect.fW : 0.0f;
	if ( pEdit->bVisualCacheDirty == 0 && pEdit->arrVisualLines != NULL && pEdit->iVisualLineCount > 0 && pEdit->fVisualCacheWidth == fWidth ) {
		return XGE_OK;
	}
	if ( __xgeXuiTextEditVisualCacheRebuild(pEdit) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pEdit->fVisualCacheWidth = fWidth;
	return XGE_OK;
}

static int __xgeXuiTextEditVisualLineCount(xge_xui_text_edit pEdit)
{
	if ( pEdit == NULL ) {
		return 1;
	}
	if ( __xgeXuiTextEditEnsureVisualCache(pEdit) != XGE_OK ) {
		return 1;
	}
	return (pEdit->iVisualLineCount > 0) ? pEdit->iVisualLineCount : 1;
}

static int __xgeXuiTextEditVisualLineStart(xge_xui_text_edit pEdit, int iLine)
{
	if ( pEdit == NULL || iLine <= 0 ) {
		return 0;
	}
	if ( __xgeXuiTextEditEnsureVisualCache(pEdit) != XGE_OK ) {
		return 0;
	}
	if ( iLine < pEdit->iVisualLineCount ) {
		return pEdit->arrVisualLines[iLine].iStart;
	}
	return pEdit->tText.iSize;
}

static int __xgeXuiTextEditVisualLineEnd(xge_xui_text_edit pEdit, int iLine)
{
	if ( pEdit == NULL ) {
		return 0;
	}
	if ( __xgeXuiTextEditEnsureVisualCache(pEdit) != XGE_OK ) {
		return 0;
	}
	if ( iLine < 0 ) {
		iLine = 0;
	}
	if ( iLine < pEdit->iVisualLineCount ) {
		return pEdit->arrVisualLines[iLine].iEnd;
	}
	return pEdit->tText.iSize;
}

static int __xgeXuiTextEditVisualLineIndex(xge_xui_text_edit pEdit, int iCursor)
{
	int iLow;
	int iHigh;
	int iMid;

	if ( (pEdit == NULL) || (pEdit->tText.sText == NULL) ) {
		return 0;
	}
	iCursor = __xgeXuiTextClampCursor(&pEdit->tText, iCursor);
	if ( __xgeXuiTextEditEnsureVisualCache(pEdit) != XGE_OK ) {
		return 0;
	}
	iLow = 0;
	iHigh = pEdit->iVisualLineCount - 1;
	while ( iLow <= iHigh ) {
		iMid = (iLow + iHigh) / 2;
		if ( pEdit->arrVisualLines[iMid].iStart <= iCursor ) {
			iLow = iMid + 1;
		} else {
			iHigh = iMid - 1;
		}
	}
	if ( iHigh < 0 ) {
		return 0;
	}
	return iHigh;
}

static float __xgeXuiTextEditCursorX(xge_xui_text_edit pEdit, int iCursor)
{
	int iStart;

	if ( pEdit == NULL ) {
		return 0.0f;
	}
	iCursor = __xgeXuiTextClampCursor(&pEdit->tText, iCursor);
	iStart = __xgeXuiTextEditVisualLineStart(pEdit, __xgeXuiTextEditVisualLineIndex(pEdit, iCursor));
	return __xgeXuiTextPrefixWidth(pEdit->pFont, pEdit->tText.sText + iStart, iCursor - iStart);
}

static int __xgeXuiTextEditCursorFromPoint(xge_xui_text_edit pEdit, float fX, float fY)
{
	const char* sText;
	float fLocalX;
	float fLocalY;
	float fPrevWidth;
	float fWidth;
	float fLineHeight;
	int iLine;
	int iLineCount;
	int iStart;
	int iEnd;
	int iPrev;
	int iCursor;
	int iNext;

	if ( (pEdit == NULL) || (pEdit->pWidget == NULL) || (pEdit->tText.sText == NULL) ) {
		return 0;
	}
	sText = pEdit->tText.sText;
	fLineHeight = __xgeXuiTextEditLineHeight(pEdit);
	fLocalX = fX - (pEdit->pWidget->tContentRect.fX + __xgeXuiTextEditLineNumberWidth(pEdit)) + pEdit->fScrollX;
	fLocalY = fY - pEdit->pWidget->tContentRect.fY + pEdit->fScrollY;
	iLine = (int)(fLocalY / fLineHeight);
	iLineCount = __xgeXuiTextEditVisualLineCount(pEdit);
	if ( iLine < 0 ) {
		iLine = 0;
	}
	if ( iLine >= iLineCount ) {
		iLine = iLineCount - 1;
	}
	iStart = __xgeXuiTextEditVisualLineStart(pEdit, iLine);
	iEnd = __xgeXuiTextEditVisualLineEnd(pEdit, iLine);
	if ( fLocalX <= 0.0f ) {
		return iStart;
	}
	iPrev = iStart;
	fPrevWidth = 0.0f;
	iCursor = iStart;
	while ( iCursor < iEnd ) {
		iNext = __xgeXuiTextNextCursor(&pEdit->tText, iCursor);
		if ( iNext <= iCursor || iNext > iEnd ) {
			break;
		}
		fWidth = __xgeXuiTextPrefixWidth(pEdit->pFont, sText + iStart, iNext - iStart);
		if ( fLocalX < ((fPrevWidth + fWidth) * 0.5f) ) {
			return iPrev;
		}
		iPrev = iNext;
		fPrevWidth = fWidth;
		iCursor = iNext;
	}
	return iEnd;
}

static int __xgeXuiTextEditCursorFromLineX(xge_xui_text_edit pEdit, int iLine, float fX)
{
	xge_rect_t tRect;

	if ( pEdit == NULL || pEdit->pWidget == NULL ) {
		return 0;
	}
	tRect = pEdit->pWidget->tContentRect;
	return __xgeXuiTextEditCursorFromPoint(pEdit, tRect.fX + __xgeXuiTextEditLineNumberWidth(pEdit) + fX - pEdit->fScrollX, tRect.fY + ((float)iLine * __xgeXuiTextEditLineHeight(pEdit)) + 1.0f - pEdit->fScrollY);
}

static void __xgeXuiTextEditEnsureCursorVisible(xge_xui_text_edit pEdit)
{
	float fLineHeight;
	float fCursorX;
	float fCursorY;
	float fContentW;
	float fContentH;

	if ( (pEdit == NULL) || (pEdit->pWidget == NULL) ) {
		return;
	}
	fLineHeight = __xgeXuiTextEditLineHeight(pEdit);
	fContentW = pEdit->pWidget->tContentRect.fW - __xgeXuiTextEditLineNumberWidth(pEdit);
	if ( fContentW < 0.0f ) {
		fContentW = 0.0f;
	}
	fContentH = pEdit->pWidget->tContentRect.fH;
	fCursorX = __xgeXuiTextEditCursorX(pEdit, xgeXuiTextGetCursor(&pEdit->tText));
	fCursorY = (float)__xgeXuiTextEditVisualLineIndex(pEdit, xgeXuiTextGetCursor(&pEdit->tText)) * fLineHeight;
	if ( fContentW <= 0.0f || fContentH <= 0.0f ) {
		pEdit->fScrollX = 0.0f;
		pEdit->fScrollY = 0.0f;
		return;
	}
	if ( fCursorX - pEdit->fScrollX > fContentW - 2.0f ) {
		pEdit->fScrollX = fCursorX - fContentW + 2.0f;
	}
	if ( fCursorX - pEdit->fScrollX < 0.0f ) {
		pEdit->fScrollX = fCursorX;
	}
	if ( fCursorY - pEdit->fScrollY > fContentH - fLineHeight ) {
		pEdit->fScrollY = fCursorY - fContentH + fLineHeight;
	}
	if ( fCursorY - pEdit->fScrollY < 0.0f ) {
		pEdit->fScrollY = fCursorY;
	}
	if ( pEdit->fScrollX < 0.0f ) {
		pEdit->fScrollX = 0.0f;
	}
	if ( pEdit->fScrollY < 0.0f ) {
		pEdit->fScrollY = 0.0f;
	}
}

static void __xgeXuiTextEditResetBlink(xge_xui_text_edit pEdit)
{
	if ( pEdit == NULL ) {
		return;
	}
	pEdit->fCursorBlinkTime = 0.0f;
	pEdit->bCursorVisible = 1;
	xgeXuiWidgetMarkPaint(pEdit->pWidget);
}

static int __xgeXuiTextEditIsShift(const xge_event_t* pEvent)
{
	return (pEvent != NULL) && ((pEvent->iParam2 & XGE_KEY_MOD_SHIFT) != 0);
}

static int __xgeXuiTextEditIsCtrl(const xge_event_t* pEvent)
{
	return (pEvent != NULL) && ((pEvent->iParam2 & XGE_KEY_MOD_CTRL) != 0);
}

static int __xgeXuiTextEditCharClass(const char* sText, int iPos)
{
	unsigned char c;

	if ( (sText == NULL) || (iPos < 0) || (sText[iPos] == 0) ) {
		return 0;
	}
	c = (unsigned char)sText[iPos];
	if ( c <= 32 ) {
		return 0;
	}
	if ( c >= 128 ) {
		return 1;
	}
	if ( ((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || (c == '_') ) {
		return 1;
	}
	return 2;
}

static void __xgeXuiTextEditSelectWordAt(xge_xui_text_edit pEdit, int iCursor)
{
	const char* sText;
	int iStart;
	int iEnd;
	int iPrev;
	int iNext;
	int iClass;

	if ( pEdit == NULL || pEdit->tText.iSize <= 0 ) {
		return;
	}
	sText = xgeXuiTextEditGetText(pEdit);
	iStart = __xgeXuiTextClampCursor(&pEdit->tText, iCursor);
	if ( iStart >= pEdit->tText.iSize && iStart > 0 ) {
		iStart = __xgeXuiTextPrevCursor(&pEdit->tText, iStart);
	}
	iClass = __xgeXuiTextEditCharClass(sText, iStart);
	if ( iClass == 0 ) {
		xgeXuiTextSetCursor(&pEdit->tText, iCursor);
		pEdit->iSelectionAnchor = pEdit->tText.iCursor;
		return;
	}
	while ( iStart > 0 ) {
		iPrev = __xgeXuiTextPrevCursor(&pEdit->tText, iStart);
		if ( __xgeXuiTextEditCharClass(sText, iPrev) != iClass ) {
			break;
		}
		iStart = iPrev;
	}
	iEnd = iStart;
	while ( iEnd < pEdit->tText.iSize && __xgeXuiTextEditCharClass(sText, iEnd) == iClass ) {
		iNext = __xgeXuiTextNextCursor(&pEdit->tText, iEnd);
		if ( iNext <= iEnd ) {
			break;
		}
		iEnd = iNext;
	}
	xgeXuiTextSetSelection(&pEdit->tText, iStart, iEnd);
	pEdit->iSelectionAnchor = iStart;
}

static void __xgeXuiTextEditMoveCursor(xge_xui_text_edit pEdit, int iCursor, int bExtend)
{
	int iStart;
	int iEnd;

	if ( pEdit == NULL ) {
		return;
	}
	iCursor = __xgeXuiTextClampCursor(&pEdit->tText, iCursor);
	if ( bExtend != 0 ) {
		xgeXuiTextGetSelection(&pEdit->tText, &iStart, &iEnd);
		if ( iStart == iEnd ) {
			pEdit->iSelectionAnchor = pEdit->tText.iCursor;
		}
		xgeXuiTextSetSelection(&pEdit->tText, pEdit->iSelectionAnchor, iCursor);
	} else {
		xgeXuiTextSetCursor(&pEdit->tText, iCursor);
		pEdit->iSelectionAnchor = iCursor;
	}
}

static char* __xgeXuiTextEditSelectionText(xge_xui_text_edit pEdit)
{
	char* sOut;
	int iStart;
	int iEnd;
	int iSize;

	if ( pEdit == NULL ) {
		return NULL;
	}
	xgeXuiTextGetSelection(&pEdit->tText, &iStart, &iEnd);
	if ( iStart == iEnd ) {
		return NULL;
	}
	iSize = iEnd - iStart;
	sOut = (char*)xrtMalloc((size_t)iSize + 1);
	if ( sOut == NULL ) {
		return NULL;
	}
	memcpy(sOut, xgeXuiTextEditGetText(pEdit) + iStart, (size_t)iSize);
	sOut[iSize] = 0;
	return sOut;
}

static void __xgeXuiTextEditCopySelection(xge_xui_text_edit pEdit)
{
	char* sSelection;

	if ( pEdit == NULL ) {
		return;
	}
	sSelection = __xgeXuiTextEditSelectionText(pEdit);
	if ( sSelection != NULL ) {
		xgeClipboardSetText(sSelection);
		xrtFree(sSelection);
	}
}

static void __xgeXuiTextEditCutSelection(xge_xui_text_edit pEdit)
{
	int iStart;
	int iEnd;

	if ( (pEdit == NULL) || (pEdit->bReadonly != 0) ) {
		return;
	}
	xgeXuiTextGetSelection(&pEdit->tText, &iStart, &iEnd);
	if ( iStart == iEnd ) {
		return;
	}
	__xgeXuiTextEditCopySelection(pEdit);
	__xgeXuiTextEditRecordUndo(pEdit);
	__xgeXuiTextSelectionDelete(&pEdit->tText);
	__xgeXuiTextEditMarkLineCache(pEdit);
	pEdit->iSelectionAnchor = pEdit->tText.iCursor;
	__xgeXuiTextEditEnsureCursorVisible(pEdit);
	__xgeXuiTextEditResetBlink(pEdit);
	xgeXuiWidgetMarkPaint(pEdit->pWidget);
}

static void __xgeXuiTextEditPasteClipboard(xge_xui_text_edit pEdit)
{
	const char* sClipboard;

	if ( (pEdit == NULL) || (pEdit->bReadonly != 0) ) {
		return;
	}
	sClipboard = xgeClipboardGetText();
	if ( (sClipboard == NULL) || (sClipboard[0] == 0) ) {
		return;
	}
	__xgeXuiTextEditRecordUndo(pEdit);
	if ( xgeXuiTextInsert(&pEdit->tText, sClipboard) == XGE_OK ) {
		__xgeXuiTextEditMarkLineCache(pEdit);
		pEdit->iSelectionAnchor = pEdit->tText.iCursor;
		__xgeXuiTextEditEnsureCursorVisible(pEdit);
		__xgeXuiTextEditResetBlink(pEdit);
		xgeXuiWidgetMarkPaint(pEdit->pWidget);
	}
}

static void __xgeXuiTextEditDeleteSelection(xge_xui_text_edit pEdit)
{
	int iStart;
	int iEnd;

	if ( (pEdit == NULL) || (pEdit->bReadonly != 0) ) {
		return;
	}
	xgeXuiTextGetSelection(&pEdit->tText, &iStart, &iEnd);
	if ( iStart == iEnd ) {
		return;
	}
	__xgeXuiTextEditRecordUndo(pEdit);
	__xgeXuiTextSelectionDelete(&pEdit->tText);
	__xgeXuiTextEditMarkLineCache(pEdit);
	pEdit->iSelectionAnchor = pEdit->tText.iCursor;
	__xgeXuiTextEditEnsureCursorVisible(pEdit);
	__xgeXuiTextEditResetBlink(pEdit);
	xgeXuiWidgetMarkPaint(pEdit->pWidget);
}

static void __xgeXuiTextEditMenuSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	xge_xui_text_edit pEdit;

	(void)pWidget;
	pEdit = (xge_xui_text_edit)pUser;
	if ( (pEdit == NULL) || (pEdit->bInitialized == 0) ) {
		return;
	}
	switch ( iIndex ) {
		case XGE_XUI_TEXT_EDIT_MENU_SELECT_ALL:
			xgeXuiTextSetSelection(&pEdit->tText, 0, pEdit->tText.iSize);
			pEdit->iSelectionAnchor = 0;
			__xgeXuiTextEditResetBlink(pEdit);
			xgeXuiWidgetMarkPaint(pEdit->pWidget);
			break;
		case XGE_XUI_TEXT_EDIT_MENU_CUT:
			__xgeXuiTextEditCutSelection(pEdit);
			break;
		case XGE_XUI_TEXT_EDIT_MENU_COPY:
			__xgeXuiTextEditCopySelection(pEdit);
			break;
		case XGE_XUI_TEXT_EDIT_MENU_PASTE:
			__xgeXuiTextEditPasteClipboard(pEdit);
			break;
		case XGE_XUI_TEXT_EDIT_MENU_DELETE:
			__xgeXuiTextEditDeleteSelection(pEdit);
			break;
		default:
			break;
	}
}

static void __xgeXuiTextEditOpenDefaultMenu(xge_xui_text_edit pEdit, float fX, float fY)
{
	const char* sClipboard;
	int iStart;
	int iEnd;
	int bSelection;
	int bClipboard;

	if ( (pEdit == NULL) || (pEdit->pDefaultMenu == NULL) ) {
		return;
	}
	xgeXuiTextGetSelection(&pEdit->tText, &iStart, &iEnd);
	sClipboard = xgeClipboardGetText();
	bSelection = (iStart != iEnd);
	bClipboard = (sClipboard != NULL) && (sClipboard[0] != 0);
	pEdit->arrDefaultMenuEnabled[XGE_XUI_TEXT_EDIT_MENU_SELECT_ALL] = (pEdit->tText.iSize > 0);
	pEdit->arrDefaultMenuEnabled[XGE_XUI_TEXT_EDIT_MENU_CUT] = bSelection && (pEdit->bReadonly == 0);
	pEdit->arrDefaultMenuEnabled[XGE_XUI_TEXT_EDIT_MENU_COPY] = bSelection;
	pEdit->arrDefaultMenuEnabled[XGE_XUI_TEXT_EDIT_MENU_PASTE] = bClipboard && (pEdit->bReadonly == 0);
	pEdit->arrDefaultMenuEnabled[XGE_XUI_TEXT_EDIT_MENU_DELETE] = bSelection && (pEdit->bReadonly == 0);
	xgeXuiMenuSetEnabledItems(pEdit->pDefaultMenu, pEdit->arrDefaultMenuEnabled, XGE_XUI_TEXT_EDIT_MENU_COUNT);
	xgeXuiMenuOpen(pEdit->pDefaultMenu, fX, fY);
}

int xgeXuiTextEditInit(xge_xui_text_edit pEdit, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont)
{
	const xge_xui_theme_t* pTheme;
	int iRet;

	if ( (pEdit == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pEdit, 0, sizeof(*pEdit));
	__xgeXuiTextControlWidgetInit(pWidget);
	iRet = xgeXuiTextInit(&pEdit->tText);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pTheme = xgeXuiGetTheme(pContext);
	pEdit->pContext = pContext;
	pEdit->pWidget = pWidget;
	pEdit->pFont = (pFont != NULL) ? pFont : pTheme->pFont;
	pEdit->iTextColor = pTheme->iTextColor;
	xgeXuiWidgetSetBackground(pWidget, pTheme->iBackgroundColor);
	pEdit->iFocusColor = pTheme->iBackgroundColor;
	pEdit->iCursorColor = pTheme->iTextColor;
	pEdit->iSelectionColor = pTheme->iSelectionColor;
	pEdit->iFindHighlightColor = XGE_COLOR_RGBA(255, 222, 92, 96);
	pEdit->iLineNumberTextColor = XGE_COLOR_RGBA(104, 126, 148, 255);
	pEdit->iLineNumberBackgroundColor = XGE_COLOR_RGBA(232, 242, 250, 255);
	pEdit->iScrollbarMode = XGE_XUI_SCROLLBAR_MODE_COMPACT;
	pEdit->bLineCacheDirty = 1;
	pEdit->bCursorVisible = 1;
	pEdit->iUndoLimit = 128;
	pEdit->iSelectionAnchor = 0;
	pEdit->bInitialized = 1;
	pEdit->pDefaultMenu = (xge_xui_menu)xrtMalloc(sizeof(xge_xui_menu_t));
	if ( pEdit->pDefaultMenu == NULL ) {
		xgeXuiTextUnit(&pEdit->tText);
		memset(pEdit, 0, sizeof(*pEdit));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeXuiMenuInit(pEdit->pDefaultMenu, pContext, pWidget);
	if ( iRet != XGE_OK ) {
		xrtFree(pEdit->pDefaultMenu);
		xgeXuiTextUnit(&pEdit->tText);
		memset(pEdit, 0, sizeof(*pEdit));
		return iRet;
	}
	xgeXuiMenuSetFont(pEdit->pDefaultMenu, pEdit->pFont);
	xgeXuiMenuSetItems(pEdit->pDefaultMenu, g_arrXgeXuiTextEditMenuItems, XGE_XUI_TEXT_EDIT_MENU_COUNT);
	xgeXuiMenuSetSelect(pEdit->pDefaultMenu, __xgeXuiTextEditMenuSelect, pEdit);
	pWidget->tStyle.fRadius = pTheme->fRadius;
	xgeXuiWidgetSetPaddingPx(pWidget, 1.0f, 1.0f, 1.0f, 1.0f);
	xgeXuiWidgetSetImeCandidateRect(pWidget, __xgeXuiTextEditImeCandidateRect, pEdit);
	pWidget->procEvent = xgeXuiTextEditEventProc;
	pWidget->procUpdate = xgeXuiTextEditUpdateProc;
	pWidget->procPaint = xgeXuiTextEditPaintProc;
	pWidget->pUser = pEdit;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiTextEditUnit(xge_xui_text_edit pEdit)
{
	if ( pEdit == NULL ) {
		return;
	}
	if ( pEdit->pWidget != NULL && pEdit->pWidget->pUser == pEdit ) {
		pEdit->pWidget->pUser = NULL;
		pEdit->pWidget->procEvent = NULL;
		pEdit->pWidget->procUpdate = NULL;
		pEdit->pWidget->procPaint = NULL;
	}
	if ( (pEdit->pWidget != NULL) && (pEdit->pWidget->procImeCandidateRect == __xgeXuiTextEditImeCandidateRect) && (pEdit->pWidget->pImeCandidateRectUser == pEdit) ) {
		xgeXuiWidgetSetImeCandidateRect(pEdit->pWidget, NULL, NULL);
	}
	if ( pEdit->pDefaultMenu != NULL ) {
		xgeXuiMenuUnit(pEdit->pDefaultMenu);
		xrtFree(pEdit->pDefaultMenu);
	}
	xgeXuiTextUnit(&pEdit->tText);
	if ( pEdit->arrLineStarts != NULL ) {
		xrtFree(pEdit->arrLineStarts);
	}
	if ( pEdit->arrVisualLines != NULL ) {
		xrtFree(pEdit->arrVisualLines);
	}
	__xgeXuiTextEditStackClear(pEdit->arrUndo, pEdit->iUndoCount);
	__xgeXuiTextEditStackClear(pEdit->arrRedo, pEdit->iRedoCount);
	if ( pEdit->arrUndo != NULL ) {
		xrtFree(pEdit->arrUndo);
	}
	if ( pEdit->arrRedo != NULL ) {
		xrtFree(pEdit->arrRedo);
	}
	memset(pEdit, 0, sizeof(*pEdit));
}

void xgeXuiTextEditSetText(xge_xui_text_edit pEdit, const char* sText)
{
	if ( (pEdit == NULL) || (pEdit->bInitialized == 0) ) {
		return;
	}
	if ( xgeXuiTextSet(&pEdit->tText, sText) == XGE_OK ) {
		__xgeXuiTextEditStackClear(pEdit->arrUndo, pEdit->iUndoCount);
		__xgeXuiTextEditStackClear(pEdit->arrRedo, pEdit->iRedoCount);
		pEdit->iUndoCount = 0;
		pEdit->iRedoCount = 0;
		__xgeXuiTextEditMarkLineCache(pEdit);
		pEdit->fScrollX = 0.0f;
		pEdit->fScrollY = 0.0f;
		__xgeXuiTextEditEnsureCursorVisible(pEdit);
		xgeXuiWidgetMarkPaint(pEdit->pWidget);
	}
}

const char* xgeXuiTextEditGetText(xge_xui_text_edit pEdit)
{
	if ( (pEdit == NULL) || (pEdit->tText.sText == NULL) ) {
		return "";
	}
	return pEdit->tText.sText;
}

void xgeXuiTextEditSetFont(xge_xui_text_edit pEdit, xge_font pFont)
{
	if ( pEdit == NULL ) {
		return;
	}
	pEdit->pFont = pFont;
	if ( pEdit->pDefaultMenu != NULL ) {
		xgeXuiMenuSetFont(pEdit->pDefaultMenu, pFont);
	}
	pEdit->fLineHeight = 0.0f;
	pEdit->bVisualCacheDirty = 1;
	__xgeXuiTextEditEnsureCursorVisible(pEdit);
	xgeXuiWidgetMarkPaint(pEdit->pWidget);
}

void xgeXuiTextEditSetColors(xge_xui_text_edit pEdit, uint32_t iText, uint32_t iBackground, uint32_t iFocus, uint32_t iCursor)
{
	if ( pEdit == NULL ) {
		return;
	}
	pEdit->iTextColor = iText;
	xgeXuiWidgetSetBackground(pEdit->pWidget, iBackground);
	pEdit->iFocusColor = iFocus;
	pEdit->iCursorColor = iCursor;
	xgeXuiWidgetMarkPaint(pEdit->pWidget);
}

void xgeXuiTextEditSetReadonly(xge_xui_text_edit pEdit, int bReadonly)
{
	if ( pEdit == NULL ) {
		return;
	}
	pEdit->bReadonly = (bReadonly != 0);
	xgeXuiWidgetSetImeMode(pEdit->pWidget, pEdit->bReadonly ? XGE_XUI_IME_DISABLED : XGE_XUI_IME_ENABLED);
	if ( pEdit->bReadonly != 0 ) {
		xgeXuiTextClearComposition(&pEdit->tText);
	}
	__xgeXuiTextEditResetBlink(pEdit);
	xgeXuiWidgetMarkPaint(pEdit->pWidget);
}

void xgeXuiTextEditSetWordWrap(xge_xui_text_edit pEdit, int bWordWrap)
{
	if ( pEdit == NULL ) {
		return;
	}
	pEdit->bWordWrap = (bWordWrap != 0);
	pEdit->bVisualCacheDirty = 1;
	if ( pEdit->bWordWrap != 0 ) {
		pEdit->fScrollX = 0.0f;
	}
	__xgeXuiTextEditEnsureCursorVisible(pEdit);
	xgeXuiWidgetMarkPaint(pEdit->pWidget);
}

void xgeXuiTextEditSetFindHighlights(xge_xui_text_edit pEdit, const xge_xui_text_edit_highlight_t* arrHighlights, int iCount)
{
	if ( pEdit == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	pEdit->arrFindHighlights = arrHighlights;
	pEdit->iFindHighlightCount = (arrHighlights != NULL) ? iCount : 0;
	xgeXuiWidgetMarkPaint(pEdit->pWidget);
}

void xgeXuiTextEditSetLineNumbers(xge_xui_text_edit pEdit, int bEnabled, float fWidth)
{
	if ( pEdit == NULL ) {
		return;
	}
	pEdit->bLineNumbers = (bEnabled != 0);
	if ( fWidth < 0.0f ) {
		fWidth = 0.0f;
	}
	pEdit->fLineNumberWidth = fWidth;
	xgeXuiWidgetMarkPaint(pEdit->pWidget);
}

void xgeXuiTextEditSetReserveColors(xge_xui_text_edit pEdit, uint32_t iFindHighlight, uint32_t iLineNumberText, uint32_t iLineNumberBackground)
{
	if ( pEdit == NULL ) {
		return;
	}
	pEdit->iFindHighlightColor = iFindHighlight;
	pEdit->iLineNumberTextColor = iLineNumberText;
	pEdit->iLineNumberBackgroundColor = iLineNumberBackground;
	xgeXuiWidgetMarkPaint(pEdit->pWidget);
}

void xgeXuiTextEditSetScroll(xge_xui_text_edit pEdit, float fX, float fY)
{
	if ( pEdit == NULL ) {
		return;
	}
	pEdit->fScrollX = (fX > 0.0f) ? fX : 0.0f;
	pEdit->fScrollY = (fY > 0.0f) ? fY : 0.0f;
	xgeXuiWidgetMarkPaint(pEdit->pWidget);
}

void xgeXuiTextEditSetScrollbarMode(xge_xui_text_edit pEdit, int iMode)
{
	if ( pEdit == NULL ) {
		return;
	}
	pEdit->iScrollbarMode = (iMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? XGE_XUI_SCROLLBAR_MODE_FULL : XGE_XUI_SCROLLBAR_MODE_COMPACT;
	xgeXuiWidgetMarkPaint(pEdit->pWidget);
}

int xgeXuiTextEditGetScrollbarMode(xge_xui_text_edit pEdit)
{
	return (pEdit != NULL) ? pEdit->iScrollbarMode : XGE_XUI_SCROLLBAR_MODE_COMPACT;
}

int xgeXuiTextEditUndo(xge_xui_text_edit pEdit)
{
	xge_xui_text_edit_state_t tCurrent;
	xge_xui_text_edit_state_t tState;
	int iRet;

	if ( pEdit == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pEdit->iUndoCount <= 0 ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	iRet = __xgeXuiTextEditStateCapture(pEdit, &tCurrent);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tState = pEdit->arrUndo[pEdit->iUndoCount - 1];
	memset(&pEdit->arrUndo[pEdit->iUndoCount - 1], 0, sizeof(pEdit->arrUndo[pEdit->iUndoCount - 1]));
	pEdit->iUndoCount--;
	iRet = __xgeXuiTextEditStackPush(&pEdit->arrRedo, &pEdit->iRedoCount, &pEdit->iRedoCapacity, pEdit->iUndoLimit, &tCurrent);
	if ( iRet != XGE_OK ) {
		__xgeXuiTextEditStateFree(&tCurrent);
		__xgeXuiTextEditStateFree(&tState);
		return iRet;
	}
	iRet = __xgeXuiTextEditRestoreState(pEdit, &tState);
	__xgeXuiTextEditStateFree(&tState);
	return iRet;
}

int xgeXuiTextEditRedo(xge_xui_text_edit pEdit)
{
	xge_xui_text_edit_state_t tCurrent;
	xge_xui_text_edit_state_t tState;
	int iRet;

	if ( pEdit == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pEdit->iRedoCount <= 0 ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	iRet = __xgeXuiTextEditStateCapture(pEdit, &tCurrent);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tState = pEdit->arrRedo[pEdit->iRedoCount - 1];
	memset(&pEdit->arrRedo[pEdit->iRedoCount - 1], 0, sizeof(pEdit->arrRedo[pEdit->iRedoCount - 1]));
	pEdit->iRedoCount--;
	iRet = __xgeXuiTextEditStackPush(&pEdit->arrUndo, &pEdit->iUndoCount, &pEdit->iUndoCapacity, pEdit->iUndoLimit, &tCurrent);
	if ( iRet != XGE_OK ) {
		__xgeXuiTextEditStateFree(&tCurrent);
		__xgeXuiTextEditStateFree(&tState);
		return iRet;
	}
	iRet = __xgeXuiTextEditRestoreState(pEdit, &tState);
	__xgeXuiTextEditStateFree(&tState);
	return iRet;
}

xge_rect_t xgeXuiTextEditGetCandidateRect(xge_xui_text_edit pEdit)
{
	xge_rect_t tRect;
	float fLineNumberWidth;

	memset(&tRect, 0, sizeof(tRect));
	if ( (pEdit == NULL) || (pEdit->pWidget == NULL) ) {
		return tRect;
	}
	fLineNumberWidth = __xgeXuiTextEditLineNumberWidth(pEdit);
	if ( (fLineNumberWidth > 0.0f) && (pEdit->pWidget->tContentRect.fX >= (pEdit->pWidget->tRect.fX + fLineNumberWidth)) ) {
		fLineNumberWidth = 0.0f;
	}
	tRect.fX = pEdit->pWidget->tContentRect.fX + fLineNumberWidth + __xgeXuiTextEditCursorX(pEdit, xgeXuiTextGetCursor(&pEdit->tText)) - pEdit->fScrollX;
	tRect.fY = pEdit->pWidget->tContentRect.fY + ((float)__xgeXuiTextEditVisualLineIndex(pEdit, xgeXuiTextGetCursor(&pEdit->tText)) * __xgeXuiTextEditLineHeight(pEdit)) - pEdit->fScrollY;
	tRect.fW = 1.0f;
	tRect.fH = __xgeXuiTextEditLineHeight(pEdit);
	return tRect;
}

int xgeXuiTextEditEvent(xge_xui_text_edit pEdit, const xge_event_t* pEvent)
{
	int iCursor;
	int iLine;
	int iLineCount;
	int iStart;
	int iEnd;

	if ( (pEdit == NULL) || (pEdit->bInitialized == 0) || (pEdit->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( ((pEdit->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) || ((pEdit->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_XUI_FOCUS_IN ) {
		__xgeXuiTextEditResetBlink(pEdit);
		xgeXuiWidgetMarkPaint(pEdit->pWidget);
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_XUI_FOCUS_OUT ) {
		xgeXuiTextClearComposition(&pEdit->tText);
		__xgeXuiTextEditResetBlink(pEdit);
		xgeXuiWidgetMarkPaint(pEdit->pWidget);
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_MOUSE_WHEEL ) {
		float fMaxScroll;
		float fContentH;

		if ( __xgeXuiRectContains(pEdit->pWidget->tRect, pEvent->fX, pEvent->fY) == 0 ) {
			return XGE_XUI_EVENT_CONTINUE;
		}
		fContentH = (float)__xgeXuiTextEditVisualLineCount(pEdit) * __xgeXuiTextEditLineHeight(pEdit);
		fMaxScroll = fContentH - pEdit->pWidget->tContentRect.fH;
		if ( fMaxScroll < 0.0f ) {
			fMaxScroll = 0.0f;
		}
		pEdit->fScrollY -= pEvent->fDY * __xgeXuiTextEditLineHeight(pEdit);
		if ( pEdit->fScrollY < 0.0f ) {
			pEdit->fScrollY = 0.0f;
		}
		if ( pEdit->fScrollY > fMaxScroll ) {
			pEdit->fScrollY = fMaxScroll;
		}
		xgeXuiWidgetMarkPaint(pEdit->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_DOWN) || (pEvent->iType == XGE_EVENT_TOUCH_BEGIN)) && __xgeXuiRectContains(pEdit->pWidget->tRect, pEvent->fX, pEvent->fY) ) {
		double fNow;
		int bDoubleClick;

		xgeXuiSetFocus(pEdit->pContext, pEdit->pWidget);
		if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 != XGE_MOUSE_LEFT) ) {
			__xgeXuiTextEditResetBlink(pEdit);
			xgeXuiWidgetMarkPaint(pEdit->pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		}
		fNow = xgeTimer();
		bDoubleClick = ((fNow - pEdit->fLastClickTime) <= 0.35) && ((pEvent->fX - pEdit->fLastClickX) < 4.0f) && ((pEdit->fLastClickX - pEvent->fX) < 4.0f) && ((pEvent->fY - pEdit->fLastClickY) < 4.0f) && ((pEdit->fLastClickY - pEvent->fY) < 4.0f);
		iCursor = __xgeXuiTextEditCursorFromPoint(pEdit, pEvent->fX, pEvent->fY);
		if ( bDoubleClick != 0 ) {
			pEdit->fLastClickTime = fNow;
			pEdit->fLastClickX = pEvent->fX;
			pEdit->fLastClickY = pEvent->fY;
			__xgeXuiTextEditSelectWordAt(pEdit, iCursor);
			pEdit->bPressPending = 0;
			pEdit->bSelecting = 0;
		} else {
			xgeXuiTextGetSelection(&pEdit->tText, &iStart, &iEnd);
			pEdit->iPressCursor = iCursor;
			pEdit->fPressX = pEvent->fX;
			pEdit->fPressY = pEvent->fY;
			pEdit->bPressInsideSelection = (iStart != iEnd && iCursor >= iStart && iCursor <= iEnd) ? 1 : 0;
			pEdit->bPressPending = 1;
			pEdit->bSelecting = 0;
			xgeXuiSetPointerCapture(pEdit->pContext, pEvent->iPointerId, pEdit->pWidget);
		}
		__xgeXuiTextEditResetBlink(pEdit);
		xgeXuiWidgetMarkPaint(pEdit->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_MOVE) || (pEvent->iType == XGE_EVENT_TOUCH_MOVE)) && (pEdit->bPressPending != 0) ) {
		float fDX = pEvent->fX - pEdit->fPressX;
		float fDY = pEvent->fY - pEdit->fPressY;
		if ( xgeXuiGetPointerCapture(pEdit->pContext, pEvent->iPointerId) != pEdit->pWidget ) {
			return XGE_XUI_EVENT_CONTINUE;
		}
		if ( (fDX * fDX + fDY * fDY) <= 36.0f ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		pEdit->bPressPending = 0;
		if ( pEdit->bPressInsideSelection != 0 ) {
			pEdit->bSelecting = 2;
			return XGE_XUI_EVENT_CONSUMED;
		}
		pEdit->iSelectionAnchor = pEdit->iPressCursor;
		pEdit->bSelecting = 1;
		iCursor = __xgeXuiTextEditCursorFromPoint(pEdit, pEvent->fX, pEvent->fY);
		xgeXuiTextSetSelection(&pEdit->tText, pEdit->iSelectionAnchor, iCursor);
		__xgeXuiTextEditEnsureCursorVisible(pEdit);
		xgeXuiWidgetMarkPaint(pEdit->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_MOVE) || (pEvent->iType == XGE_EVENT_TOUCH_MOVE)) && (pEdit->bSelecting == 1) ) {
		if ( xgeXuiGetPointerCapture(pEdit->pContext, pEvent->iPointerId) != pEdit->pWidget ) {
			return XGE_XUI_EVENT_CONTINUE;
		}
		iCursor = __xgeXuiTextEditCursorFromPoint(pEdit, pEvent->fX, pEvent->fY);
		xgeXuiTextSetSelection(&pEdit->tText, pEdit->iSelectionAnchor, iCursor);
		__xgeXuiTextEditEnsureCursorVisible(pEdit);
		xgeXuiWidgetMarkPaint(pEdit->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_UP) || (pEvent->iType == XGE_EVENT_TOUCH_END) || (pEvent->iType == XGE_EVENT_TOUCH_CANCEL)) && ((pEdit->bSelecting != 0) || (pEdit->bPressPending != 0)) ) {
		if ( xgeXuiGetPointerCapture(pEdit->pContext, pEvent->iPointerId) != pEdit->pWidget ) {
			return XGE_XUI_EVENT_CONTINUE;
		}
		if ( pEdit->bPressPending != 0 && pEvent->iType != XGE_EVENT_TOUCH_CANCEL ) {
			pEdit->fLastClickTime = xgeTimer();
			pEdit->fLastClickX = pEvent->fX;
			pEdit->fLastClickY = pEvent->fY;
			iCursor = __xgeXuiTextEditCursorFromPoint(pEdit, pEvent->fX, pEvent->fY);
			xgeXuiTextSetCursor(&pEdit->tText, iCursor);
			pEdit->iSelectionAnchor = iCursor;
			pEdit->fPreferredX = __xgeXuiTextEditCursorX(pEdit, iCursor);
			__xgeXuiTextEditEnsureCursorVisible(pEdit);
			__xgeXuiTextEditResetBlink(pEdit);
			xgeXuiWidgetMarkPaint(pEdit->pWidget);
		}
		pEdit->bPressPending = 0;
		pEdit->bPressInsideSelection = 0;
		pEdit->bSelecting = 0;
		if ( pEdit->pContext != NULL && xgeXuiGetPointerCapture(pEdit->pContext, pEvent->iPointerId) == pEdit->pWidget ) {
			xgeXuiSetPointerCapture(pEdit->pContext, pEvent->iPointerId, NULL);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pEvent->iType == XGE_EVENT_XUI_CAPTURE_LOST || pEvent->iType == XGE_EVENT_XUI_CAPTURE_CANCEL ) {
		pEdit->bPressPending = 0;
		pEdit->bPressInsideSelection = 0;
		pEdit->bSelecting = 0;
		__xgeXuiTextEditResetBlink(pEdit);
		xgeXuiWidgetMarkPaint(pEdit->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_XUI_CONTEXT_BEGIN) || (pEvent->iType == XGE_EVENT_XUI_CONTEXT_UPDATE) || (pEvent->iType == XGE_EVENT_XUI_CONTEXT_END) || (pEvent->iType == XGE_EVENT_XUI_CONTEXT_CANCEL) ) {
		pEdit->bPressPending = 0;
		pEdit->bPressInsideSelection = 0;
		pEdit->bSelecting = 0;
		if ( pEdit->pContext != NULL && xgeXuiGetPointerCapture(pEdit->pContext, pEvent->iPointerId) == pEdit->pWidget ) {
			xgeXuiSetPointerCapture(pEdit->pContext, pEvent->iPointerId, NULL);
		}
		__xgeXuiTextEditResetBlink(pEdit);
		xgeXuiWidgetMarkPaint(pEdit->pWidget);
		if ( pEvent->iType == XGE_EVENT_XUI_CONTEXT_BEGIN ) {
			__xgeXuiTextEditOpenDefaultMenu(pEdit, pEvent->fX, pEvent->fY);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEdit->pContext != NULL) && (pEdit->pContext->pFocus != pEdit->pWidget) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pEvent->iType == XGE_EVENT_TEXT) || (pEvent->iType == XGE_EVENT_IME_START) || (pEvent->iType == XGE_EVENT_IME_UPDATE) || (pEvent->iType == XGE_EVENT_IME_END) ) {
		if ( pEdit->bReadonly != 0 ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		if ( pEvent->iType == XGE_EVENT_TEXT && pEvent->iCodepoint != 0 ) {
			__xgeXuiTextEditRecordUndo(pEdit);
		}
		if ( xgeXuiTextInputEvent(&pEdit->tText, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
			__xgeXuiTextEditMarkLineCache(pEdit);
			__xgeXuiTextEditEnsureCursorVisible(pEdit);
			__xgeXuiTextEditResetBlink(pEdit);
			xgeXuiWidgetMarkPaint(pEdit->pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		}
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType != XGE_EVENT_KEY_DOWN ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( __xgeXuiTextEditIsCtrl(pEvent) && (pEvent->iParam1 == 'Z' || pEvent->iParam1 == 'z') ) {
		if ( pEdit->bReadonly == 0 ) {
			xgeXuiTextEditUndo(pEdit);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( __xgeXuiTextEditIsCtrl(pEvent) && (pEvent->iParam1 == 'Y' || pEvent->iParam1 == 'y') ) {
		if ( pEdit->bReadonly == 0 ) {
			xgeXuiTextEditRedo(pEdit);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( __xgeXuiTextEditIsCtrl(pEvent) && (pEvent->iParam1 == 'A' || pEvent->iParam1 == 'a') ) {
		xgeXuiTextSetSelection(&pEdit->tText, 0, pEdit->tText.iSize);
		pEdit->iSelectionAnchor = 0;
		__xgeXuiTextEditResetBlink(pEdit);
		xgeXuiWidgetMarkPaint(pEdit->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( __xgeXuiTextEditIsCtrl(pEvent) && (pEvent->iParam1 == 'C' || pEvent->iParam1 == 'c' || pEvent->iParam1 == 'X' || pEvent->iParam1 == 'x') ) {
		char* sSelection = __xgeXuiTextEditSelectionText(pEdit);
		if ( sSelection != NULL ) {
			xgeClipboardSetText(sSelection);
			xrtFree(sSelection);
			if ( (pEvent->iParam1 == 'X' || pEvent->iParam1 == 'x') && (pEdit->bReadonly == 0) ) {
				__xgeXuiTextEditRecordUndo(pEdit);
				__xgeXuiTextSelectionDelete(&pEdit->tText);
				__xgeXuiTextEditMarkLineCache(pEdit);
				pEdit->iSelectionAnchor = pEdit->tText.iCursor;
				__xgeXuiTextEditEnsureCursorVisible(pEdit);
				__xgeXuiTextEditResetBlink(pEdit);
				xgeXuiWidgetMarkPaint(pEdit->pWidget);
			}
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( __xgeXuiTextEditIsCtrl(pEvent) && (pEvent->iParam1 == 'V' || pEvent->iParam1 == 'v') ) {
		const char* sClipboard;
		if ( pEdit->bReadonly != 0 ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		sClipboard = xgeClipboardGetText();
		if ( (sClipboard != NULL) && (sClipboard[0] != 0) ) {
			__xgeXuiTextEditRecordUndo(pEdit);
			if ( xgeXuiTextInsert(&pEdit->tText, sClipboard) == XGE_OK ) {
				__xgeXuiTextEditMarkLineCache(pEdit);
				pEdit->iSelectionAnchor = pEdit->tText.iCursor;
				__xgeXuiTextEditEnsureCursorVisible(pEdit);
				__xgeXuiTextEditResetBlink(pEdit);
				xgeXuiWidgetMarkPaint(pEdit->pWidget);
			}
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iParam1 == XGE_KEY_ENTER) && (pEdit->bReadonly == 0) ) {
		__xgeXuiTextEditRecordUndo(pEdit);
		if ( xgeXuiTextInsert(&pEdit->tText, "\n") == XGE_OK ) {
			__xgeXuiTextEditMarkLineCache(pEdit);
			pEdit->iSelectionAnchor = pEdit->tText.iCursor;
			__xgeXuiTextEditEnsureCursorVisible(pEdit);
			__xgeXuiTextEditResetBlink(pEdit);
			xgeXuiWidgetMarkPaint(pEdit->pWidget);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iParam1 == XGE_KEY_TAB) && (pEdit->bReadonly == 0) ) {
		__xgeXuiTextEditRecordUndo(pEdit);
		if ( xgeXuiTextInsert(&pEdit->tText, "\t") == XGE_OK ) {
			__xgeXuiTextEditMarkLineCache(pEdit);
			pEdit->iSelectionAnchor = pEdit->tText.iCursor;
			__xgeXuiTextEditEnsureCursorVisible(pEdit);
			__xgeXuiTextEditResetBlink(pEdit);
			xgeXuiWidgetMarkPaint(pEdit->pWidget);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iParam1 == XGE_KEY_BACKSPACE) && (pEdit->bReadonly == 0) ) {
		__xgeXuiTextEditRecordUndo(pEdit);
		if ( xgeXuiTextDeleteBack(&pEdit->tText) == XGE_OK ) {
			__xgeXuiTextEditMarkLineCache(pEdit);
			pEdit->iSelectionAnchor = pEdit->tText.iCursor;
			__xgeXuiTextEditEnsureCursorVisible(pEdit);
			__xgeXuiTextEditResetBlink(pEdit);
			xgeXuiWidgetMarkPaint(pEdit->pWidget);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iParam1 == XGE_KEY_DELETE) && (pEdit->bReadonly == 0) ) {
		__xgeXuiTextEditRecordUndo(pEdit);
		if ( xgeXuiTextDeleteForward(&pEdit->tText) == XGE_OK ) {
			__xgeXuiTextEditMarkLineCache(pEdit);
			pEdit->iSelectionAnchor = pEdit->tText.iCursor;
			__xgeXuiTextEditEnsureCursorVisible(pEdit);
			__xgeXuiTextEditResetBlink(pEdit);
			xgeXuiWidgetMarkPaint(pEdit->pWidget);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	xgeXuiTextGetSelection(&pEdit->tText, &iStart, &iEnd);
	if ( pEvent->iParam1 == XGE_KEY_LEFT ) {
		__xgeXuiTextEditMoveCursor(pEdit, (iStart != iEnd && !__xgeXuiTextEditIsShift(pEvent)) ? iStart : __xgeXuiTextPrevCursor(&pEdit->tText, pEdit->tText.iCursor), __xgeXuiTextEditIsShift(pEvent));
		pEdit->fPreferredX = __xgeXuiTextEditCursorX(pEdit, pEdit->tText.iCursor);
	} else if ( pEvent->iParam1 == XGE_KEY_RIGHT ) {
		__xgeXuiTextEditMoveCursor(pEdit, (iStart != iEnd && !__xgeXuiTextEditIsShift(pEvent)) ? iEnd : __xgeXuiTextNextCursor(&pEdit->tText, pEdit->tText.iCursor), __xgeXuiTextEditIsShift(pEvent));
		pEdit->fPreferredX = __xgeXuiTextEditCursorX(pEdit, pEdit->tText.iCursor);
	} else if ( pEvent->iParam1 == XGE_KEY_HOME ) {
		__xgeXuiTextEditMoveCursor(pEdit, __xgeXuiTextEditIsCtrl(pEvent) ? 0 : (pEdit->bWordWrap ? __xgeXuiTextEditVisualLineStart(pEdit, __xgeXuiTextEditVisualLineIndex(pEdit, pEdit->tText.iCursor)) : __xgeXuiTextEditLineStart(pEdit, pEdit->tText.iCursor)), __xgeXuiTextEditIsShift(pEvent));
		pEdit->fPreferredX = 0.0f;
	} else if ( pEvent->iParam1 == XGE_KEY_END ) {
		__xgeXuiTextEditMoveCursor(pEdit, __xgeXuiTextEditIsCtrl(pEvent) ? pEdit->tText.iSize : (pEdit->bWordWrap ? __xgeXuiTextEditVisualLineEnd(pEdit, __xgeXuiTextEditVisualLineIndex(pEdit, pEdit->tText.iCursor)) : __xgeXuiTextEditLineEnd(pEdit, pEdit->tText.iCursor)), __xgeXuiTextEditIsShift(pEvent));
		pEdit->fPreferredX = __xgeXuiTextEditCursorX(pEdit, pEdit->tText.iCursor);
	} else if ( (pEvent->iParam1 == XGE_KEY_UP) || (pEvent->iParam1 == XGE_KEY_DOWN) ) {
		iLine = __xgeXuiTextEditVisualLineIndex(pEdit, pEdit->tText.iCursor);
		iLineCount = __xgeXuiTextEditVisualLineCount(pEdit);
		iLine += (pEvent->iParam1 == XGE_KEY_UP) ? -1 : 1;
		if ( iLine < 0 ) {
			iLine = 0;
		}
		if ( iLine >= iLineCount ) {
			iLine = iLineCount - 1;
		}
		__xgeXuiTextEditMoveCursor(pEdit, __xgeXuiTextEditCursorFromLineX(pEdit, iLine, pEdit->fPreferredX), __xgeXuiTextEditIsShift(pEvent));
	} else if ( (pEvent->iParam1 == XGE_KEY_PAGE_UP) || (pEvent->iParam1 == XGE_KEY_PAGE_DOWN) ) {
		int iPageRows;
		iLine = __xgeXuiTextEditVisualLineIndex(pEdit, pEdit->tText.iCursor);
		iLineCount = __xgeXuiTextEditVisualLineCount(pEdit);
		iPageRows = (int)(pEdit->pWidget->tContentRect.fH / __xgeXuiTextEditLineHeight(pEdit));
		if ( iPageRows < 1 ) {
			iPageRows = 1;
		}
		iLine += (pEvent->iParam1 == XGE_KEY_PAGE_UP) ? -iPageRows : iPageRows;
		if ( iLine < 0 ) {
			iLine = 0;
		}
		if ( iLine >= iLineCount ) {
			iLine = iLineCount - 1;
		}
		__xgeXuiTextEditMoveCursor(pEdit, __xgeXuiTextEditCursorFromLineX(pEdit, iLine, pEdit->fPreferredX), __xgeXuiTextEditIsShift(pEvent));
	} else {
		return XGE_XUI_EVENT_CONTINUE;
	}
	__xgeXuiTextEditEnsureCursorVisible(pEdit);
	__xgeXuiTextEditResetBlink(pEdit);
	xgeXuiWidgetMarkPaint(pEdit->pWidget);
	return XGE_XUI_EVENT_CONSUMED;
}

int xgeXuiTextEditEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiTextEditEvent((xge_xui_text_edit)pUser, pEvent);
}

static void __xgeXuiTextEditPaintSelection(xge_xui_text_edit pEdit)
{
	xge_rect_t tRect;
	float fLineHeight;
	float fStartX;
	float fEndX;
	int iStart;
	int iEnd;
	int iLine;
	int iLineStart;
	int iLineEnd;
	int iSelStart;
	int iSelEnd;
	int iLineCount;

	if ( (pEdit == NULL) || (pEdit->pWidget == NULL) || (XGE_COLOR_GET_A(pEdit->iSelectionColor) == 0) ) {
		return;
	}
	xgeXuiTextGetSelection(&pEdit->tText, &iStart, &iEnd);
	if ( iStart == iEnd ) {
		return;
	}
	fLineHeight = __xgeXuiTextEditLineHeight(pEdit);
	iLineCount = __xgeXuiTextEditVisualLineCount(pEdit);
	for ( iLine = 0; iLine < iLineCount; iLine++ ) {
		iLineStart = __xgeXuiTextEditVisualLineStart(pEdit, iLine);
		iLineEnd = __xgeXuiTextEditVisualLineEnd(pEdit, iLine);
		iSelStart = (iStart > iLineStart) ? iStart : iLineStart;
		iSelEnd = (iEnd < iLineEnd) ? iEnd : iLineEnd;
		if ( iSelStart >= iSelEnd ) {
			continue;
		}
		fStartX = __xgeXuiTextPrefixWidth(pEdit->pFont, pEdit->tText.sText + iLineStart, iSelStart - iLineStart);
		fEndX = __xgeXuiTextPrefixWidth(pEdit->pFont, pEdit->tText.sText + iLineStart, iSelEnd - iLineStart);
		tRect.fX = pEdit->pWidget->tContentRect.fX + fStartX - pEdit->fScrollX;
		tRect.fY = pEdit->pWidget->tContentRect.fY + ((float)iLine * fLineHeight) - pEdit->fScrollY + 1.0f;
		tRect.fW = fEndX - fStartX;
		tRect.fH = fLineHeight - 2.0f;
		if ( tRect.fW <= 0.0f ) {
			continue;
		}
		if ( tRect.fH < 1.0f ) {
			tRect.fH = 1.0f;
		}
		if ( (tRect.fY + tRect.fH) < pEdit->pWidget->tContentRect.fY || tRect.fY > (pEdit->pWidget->tContentRect.fY + pEdit->pWidget->tContentRect.fH) ) {
			continue;
		}
		__xgeXuiHostDrawRect(tRect, pEdit->iSelectionColor);
	}
}

static void __xgeXuiTextEditPaintRange(xge_xui_text_edit pEdit, int iStart, int iEnd, uint32_t iColor)
{
	xge_rect_t tRect;
	float fLineHeight;
	float fStartX;
	float fEndX;
	int iLine;
	int iLineStart;
	int iLineEnd;
	int iRangeStart;
	int iRangeEnd;
	int iLineCount;

	if ( (pEdit == NULL) || (pEdit->pWidget == NULL) || (iStart >= iEnd) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	iStart = __xgeXuiTextClampCursor(&pEdit->tText, iStart);
	iEnd = __xgeXuiTextClampCursor(&pEdit->tText, iEnd);
	if ( iStart >= iEnd ) {
		return;
	}
	fLineHeight = __xgeXuiTextEditLineHeight(pEdit);
	iLineCount = __xgeXuiTextEditVisualLineCount(pEdit);
	for ( iLine = 0; iLine < iLineCount; iLine++ ) {
		iLineStart = __xgeXuiTextEditVisualLineStart(pEdit, iLine);
		iLineEnd = __xgeXuiTextEditVisualLineEnd(pEdit, iLine);
		iRangeStart = (iStart > iLineStart) ? iStart : iLineStart;
		iRangeEnd = (iEnd < iLineEnd) ? iEnd : iLineEnd;
		if ( iRangeStart >= iRangeEnd ) {
			continue;
		}
		fStartX = __xgeXuiTextPrefixWidth(pEdit->pFont, pEdit->tText.sText + iLineStart, iRangeStart - iLineStart);
		fEndX = __xgeXuiTextPrefixWidth(pEdit->pFont, pEdit->tText.sText + iLineStart, iRangeEnd - iLineStart);
		tRect.fX = pEdit->pWidget->tContentRect.fX + fStartX - pEdit->fScrollX;
		tRect.fY = pEdit->pWidget->tContentRect.fY + ((float)iLine * fLineHeight) - pEdit->fScrollY + 1.0f;
		tRect.fW = fEndX - fStartX;
		tRect.fH = fLineHeight - 2.0f;
		if ( (tRect.fW <= 0.0f) || ((tRect.fY + tRect.fH) < pEdit->pWidget->tContentRect.fY) || (tRect.fY > (pEdit->pWidget->tContentRect.fY + pEdit->pWidget->tContentRect.fH)) ) {
			continue;
		}
		if ( tRect.fH < 1.0f ) {
			tRect.fH = 1.0f;
		}
		__xgeXuiHostDrawRect(tRect, iColor);
	}
}

static void __xgeXuiTextEditPaintFindHighlights(xge_xui_text_edit pEdit)
{
	int i;

	if ( (pEdit == NULL) || (pEdit->arrFindHighlights == NULL) || (pEdit->iFindHighlightCount <= 0) ) {
		return;
	}
	for ( i = 0; i < pEdit->iFindHighlightCount; i++ ) {
		__xgeXuiTextEditPaintRange(pEdit, pEdit->arrFindHighlights[i].iStart, pEdit->arrFindHighlights[i].iEnd, pEdit->iFindHighlightColor);
	}
}

static float __xgeXuiTextEditLineNumberWidth(xge_xui_text_edit pEdit)
{
	if ( (pEdit == NULL) || (pEdit->bLineNumbers == 0) ) {
		return 0.0f;
	}
	return (pEdit->fLineNumberWidth > 0.0f) ? pEdit->fLineNumberWidth : 48.0f;
}

static void __xgeXuiTextEditPaintLineNumbers(xge_xui_text_edit pEdit)
{
	xge_rect_t tRect;
	char sNumber[32];
	float fWidth;
	float fLineHeight;
	int iLine;
	int iLineCount;

	if ( (pEdit == NULL) || (pEdit->pWidget == NULL) || (pEdit->bLineNumbers == 0) ) {
		return;
	}
	fWidth = __xgeXuiTextEditLineNumberWidth(pEdit);
	if ( fWidth <= 0.0f ) {
		return;
	}
	tRect = pEdit->pWidget->tContentRect;
	tRect.fW = fWidth;
	if ( XGE_COLOR_GET_A(pEdit->iLineNumberBackgroundColor) != 0 ) {
		__xgeXuiHostDrawRect(tRect, pEdit->iLineNumberBackgroundColor);
	}
	if ( (pEdit->pFont == NULL) || (XGE_COLOR_GET_A(pEdit->iLineNumberTextColor) == 0) ) {
		return;
	}
	fLineHeight = __xgeXuiTextEditLineHeight(pEdit);
	iLineCount = __xgeXuiTextEditVisualLineCount(pEdit);
	for ( iLine = 0; iLine < iLineCount; iLine++ ) {
		tRect.fY = pEdit->pWidget->tContentRect.fY + ((float)iLine * fLineHeight) - pEdit->fScrollY;
		tRect.fH = fLineHeight;
		if ( (tRect.fY + tRect.fH) < pEdit->pWidget->tContentRect.fY || tRect.fY > (pEdit->pWidget->tContentRect.fY + pEdit->pWidget->tContentRect.fH) ) {
			continue;
		}
		snprintf(sNumber, sizeof(sNumber), "%d", iLine + 1);
		tRect.fW = fWidth - 6.0f;
		__xgeXuiHostDrawTextRect(pEdit->pFont, sNumber, tRect, pEdit->iLineNumberTextColor, XGE_TEXT_ALIGN_RIGHT | XGE_TEXT_ALIGN_TOP | XGE_TEXT_CLIP);
		tRect.fW = fWidth;
	}
}

static void __xgeXuiTextEditPaintScrollbar(xge_xui_text_edit pEdit)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fVisibleH;
	float fContentH;
	float fSize;
	float fButton;
	float fTrackH;
	float fMaxScroll;

	if ( (pEdit == NULL) || (pEdit->pWidget == NULL) ) {
		return;
	}
	fVisibleH = pEdit->pWidget->tContentRect.fH;
	fContentH = (float)__xgeXuiTextEditVisualLineCount(pEdit) * __xgeXuiTextEditLineHeight(pEdit);
	if ( (fVisibleH <= 0.0f) || (fContentH <= fVisibleH) ) {
		return;
	}
	fSize = (pEdit->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? 16.0f : 5.0f;
	fButton = (pEdit->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? fSize : 0.0f;
	tBar.fX = pEdit->pWidget->tContentRect.fX + pEdit->pWidget->tContentRect.fW - fSize;
	tBar.fY = pEdit->pWidget->tContentRect.fY;
	tBar.fW = fSize;
	tBar.fH = pEdit->pWidget->tContentRect.fH;
	tThumb = tBar;
	tThumb.fY += fButton;
	tThumb.fH -= fButton * 2.0f;
	fTrackH = tThumb.fH;
	if ( fTrackH < 1.0f ) {
		fTrackH = 1.0f;
		tThumb.fH = 1.0f;
	}
	tThumb.fH = fTrackH * (fVisibleH / fContentH);
	if ( tThumb.fH < 8.0f ) {
		tThumb.fH = 8.0f;
	}
	if ( tThumb.fH > fTrackH ) {
		tThumb.fH = fTrackH;
	}
	fMaxScroll = fContentH - fVisibleH;
	if ( fMaxScroll > 0.0f && fTrackH > tThumb.fH ) {
		tThumb.fY += (fTrackH - tThumb.fH) * (pEdit->fScrollY / fMaxScroll);
	}
	if ( pEdit->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_COMPACT ) {
		tThumb.fX += (tThumb.fW - 4.0f) * 0.5f;
		tThumb.fW = 4.0f;
		__xgeXuiHostDrawRoundedRect(tThumb, XGE_COLOR_RGBA(104, 142, 178, 245), 2.0f);
	} else {
		__xgeXuiHostDrawRect(tBar, XGE_COLOR_RGBA(255, 255, 255, 255));
		__xgeXuiHostDrawBorderRect(tBar, 1.0f, XGE_COLOR_RGBA(184, 223, 245, 255));
		__xgeXuiHostDrawRect(tThumb, XGE_COLOR_RGBA(104, 142, 178, 245));
	}
}

void xgeXuiTextEditUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser)
{
	xge_xui_text_edit pEdit;

	pEdit = (xge_xui_text_edit)pUser;
	if ( (pWidget == NULL) || (pEdit == NULL) ) {
		return;
	}
	if ( (pEdit->pContext == NULL) || (pEdit->pContext->pFocus != pWidget) || (pEdit->bSelecting != 0) ) {
		__xgeXuiTextEditResetBlink(pEdit);
		return;
	}
	pEdit->fCursorBlinkTime += (fDelta > 0.0f) ? fDelta : 0.0f;
	if ( pEdit->fCursorBlinkTime >= 0.5f ) {
		pEdit->fCursorBlinkTime = 0.0f;
		pEdit->bCursorVisible = (pEdit->bCursorVisible == 0);
		xgeXuiWidgetMarkPaint(pWidget);
	}
}

void xgeXuiTextEditPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_text_edit pEdit;
	xge_rect_t tTextRect;
	xge_rect_t tCursor;
	xge_rect_t tContentSaved;
	char* sLine;
	float fNumberWidth;
	int iLine;
	int iLineCount;
	int iStart;
	int iEnd;
	int iSize;
	int iCursorLine;

	pEdit = (xge_xui_text_edit)pUser;
	if ( (pWidget == NULL) || (pEdit == NULL) ) {
		return;
	}
	tContentSaved = pWidget->tContentRect;
	fNumberWidth = __xgeXuiTextEditLineNumberWidth(pEdit);
	__xgeXuiTextEditPaintLineNumbers(pEdit);
	if ( fNumberWidth > 0.0f ) {
		pWidget->tContentRect.fX += fNumberWidth;
		pWidget->tContentRect.fW -= fNumberWidth;
		if ( pWidget->tContentRect.fW < 0.0f ) {
			pWidget->tContentRect.fW = 0.0f;
		}
	}
	__xgeXuiTextEditPaintFindHighlights(pEdit);
	if ( (pEdit->pContext != NULL) && (pEdit->pContext->pFocus == pWidget) ) {
		iCursorLine = __xgeXuiTextEditVisualLineIndex(pEdit, xgeXuiTextGetCursor(&pEdit->tText));
		tTextRect = pWidget->tContentRect;
		tTextRect.fY += ((float)iCursorLine * __xgeXuiTextEditLineHeight(pEdit)) - pEdit->fScrollY;
		tTextRect.fH = __xgeXuiTextEditLineHeight(pEdit);
		if ( (tTextRect.fY + tTextRect.fH) >= pWidget->tContentRect.fY && tTextRect.fY <= (pWidget->tContentRect.fY + pWidget->tContentRect.fH) ) {
			__xgeXuiHostDrawRect(tTextRect, XGE_COLOR_RGBA(255, 246, 194, 120));
		}
	}
	__xgeXuiTextEditPaintSelection(pEdit);
	if ( (pEdit->pFont != NULL) && (pEdit->tText.sText != NULL) ) {
		if ( pEdit->bWordWrap == 0 ) {
			tTextRect = pWidget->tContentRect;
			tTextRect.fX -= pEdit->fScrollX;
			tTextRect.fY -= pEdit->fScrollY;
			tTextRect.fW += pEdit->fScrollX;
			tTextRect.fH += pEdit->fScrollY + ((float)__xgeXuiTextEditLineCount(pEdit) * __xgeXuiTextEditLineHeight(pEdit));
			__xgeXuiHostDrawTextRect(pEdit->pFont, pEdit->tText.sText, tTextRect, pEdit->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP);
		} else {
			iLineCount = __xgeXuiTextEditVisualLineCount(pEdit);
			for ( iLine = 0; iLine < iLineCount; iLine++ ) {
				iStart = __xgeXuiTextEditVisualLineStart(pEdit, iLine);
				iEnd = __xgeXuiTextEditVisualLineEnd(pEdit, iLine);
				iSize = iEnd - iStart;
				if ( iSize < 0 ) {
					iSize = 0;
				}
				sLine = (char*)xrtMalloc((size_t)iSize + 1);
				if ( sLine == NULL ) {
					continue;
				}
				memcpy(sLine, pEdit->tText.sText + iStart, (size_t)iSize);
				sLine[iSize] = 0;
				tTextRect = pWidget->tContentRect;
				tTextRect.fY += ((float)iLine * __xgeXuiTextEditLineHeight(pEdit)) - pEdit->fScrollY;
				tTextRect.fH = __xgeXuiTextEditLineHeight(pEdit);
				if ( (tTextRect.fY + tTextRect.fH) >= pWidget->tContentRect.fY && tTextRect.fY <= (pWidget->tContentRect.fY + pWidget->tContentRect.fH) ) {
					__xgeXuiHostDrawTextRect(pEdit->pFont, sLine, tTextRect, pEdit->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP);
				}
				xrtFree(sLine);
			}
		}
	}
	if ( (pEdit->pContext != NULL) && (pEdit->pContext->pFocus == pWidget) && (pEdit->bCursorVisible != 0) && (XGE_COLOR_GET_A(pEdit->iCursorColor) != 0) ) {
		tCursor = xgeXuiTextEditGetCandidateRect(pEdit);
		tCursor.fW = 1.0f;
		tCursor.fH -= 2.0f;
		if ( tCursor.fH < 1.0f ) {
			tCursor.fH = 1.0f;
		}
		tCursor.fY += 1.0f;
		__xgeXuiHostDrawRect(tCursor, pEdit->iCursorColor);
	}
	pWidget->tContentRect = tContentSaved;
	__xgeXuiTextEditPaintScrollbar(pEdit);
	__xgeXuiHostDrawBorderRect(pWidget->tRect, 1.0f, XGE_COLOR_RGBA(184, 223, 245, 255));
}
