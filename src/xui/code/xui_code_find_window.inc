/* Native find UI shared by standalone CodeEdit and a multi-editor find scope.
 * A nonvisual composite owns the window/controls; editor identity is only an
 * operation target, so closing/replacing an editor cannot destroy the session. */
static xui_code_find_window_t* __xuiCodeFindWindowGetData(xui_widget pOwner)
{
	return (xui_code_find_window_t*)xuiWidgetGetTypeData(pOwner);
}

static int __xuiCodeFindEscapesValid(const char* s)
{
	/* C-string find APIs cannot represent decoded NUL. Validate even when no
	 * match exists, so a malformed replacement never depends on input data. */
	for (size_t i = 0; s != NULL && s[i]; ++i) if (s[i] == '\\') {
		if (!s[++i] || s[i] == '0') return 0;
		if (s[i] == 'x' || s[i] == 'u') {
			int digits = s[i] == 'x' ? 2 : 4, value = 0;
			for (int j = 0; j < digits; ++j) {
				unsigned char c = (unsigned char)s[i + 1];
				int h = c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
				if (h < 0) return 0;
				value = value * 16 + h; ++i;
			}
			if (!value || (value >= 0xdc00 && value <= 0xdfff)) return 0;
			if (value >= 0xd800 && value <= 0xdbff) {
				if (s[i + 1] != '\\' || s[i + 2] != 'u') return 0;
				i += 2; int low = 0;
				for (int j = 0; j < 4; ++j) {
					unsigned char c = (unsigned char)s[i + 1];
					int h = c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
					if (h < 0) return 0;
					low = low * 16 + h; ++i;
				}
				if (low < 0xdc00 || low > 0xdfff) return 0;
			}
		}
	}
	return 1;
}

static void __xuiCodeFindWindowEnable(xui_code_find_window_t* pUi)
{
	int writable = pUi->pEditor != NULL && !xuiCodeEditIsReadonly(pUi->pEditor);
	(void)xuiWidgetSetEnabled(pUi->pReplaceButton, writable);
	(void)xuiWidgetSetEnabled(pUi->pReplaceAllButton, writable);
	(void)xuiWidgetSetEnabled(pUi->pSelectionCheck, !(pUi->pFindScope && xuiCheckBoxGetChecked(pUi->pScopeCheck)) && pUi->pEditor != NULL);
}

static void __xuiCodeFindWindowInvalidate(xui_code_find_window_t* pUi)
{
	if (pUi == NULL) return;
	pUi->bPending = 1;
	pUi->fPendingElapsed = 0;
	if (pUi->pFindResultList != NULL) (void)xuiTableViewRefreshAdapter(pUi->pFindResultList);
}

static int __xuiCodeEditFindResultTableColumns(xui_code_find_window_t* pData, int bScope)
{
	xui_table_view_column_t arrColumns[3];
	xui_context pContext;
	int iCount;
	int i;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->bFindResultScopeMode == bScope && pData->pFindResultList != NULL && xuiTableViewGetColumnCount(pData->pFindResultList) > 0 ) return XUI_OK;
	pData->bFindResultScopeMode = bScope ? 1 : 0;
	if ( pData->pFindResultList == NULL ) return XUI_OK;
	pContext = xuiWidgetGetContext(pData->pFindResultList);
	memset(arrColumns, 0, sizeof(arrColumns));
	if ( bScope ) {
		arrColumns[0].sTitle = xuiTranslate(pContext, XUI_TR_FIND_COL_FILE);
		arrColumns[0].fWidth = 150.0f;
		arrColumns[0].fMinWidth = 80.0f;
		arrColumns[1].sTitle = xuiTranslate(pContext, XUI_TR_FIND_COL_POSITION);
		arrColumns[1].fWidth = 84.0f;
		arrColumns[1].fMinWidth = 64.0f;
		arrColumns[2].sTitle = xuiTranslate(pContext, XUI_TR_FIND_COL_CONTENT);
		arrColumns[2].fWidth = 340.0f;
		arrColumns[2].fMinWidth = 120.0f;
		iCount = 3;
	} else {
		arrColumns[0].sTitle = xuiTranslate(pContext, XUI_TR_FIND_COL_POSITION);
		arrColumns[0].fWidth = 84.0f;
		arrColumns[0].fMinWidth = 64.0f;
		arrColumns[1].sTitle = xuiTranslate(pContext, XUI_TR_FIND_COL_CONTENT);
		arrColumns[1].fWidth = 492.0f;
		arrColumns[1].fMinWidth = 160.0f;
		iCount = 2;
	}
	for ( i = 0; i < iCount; i++ ) {
		arrColumns[i].bVisibleSet = 1;
		arrColumns[i].bVisible = 1;
		arrColumns[i].bResizableSet = 1;
		arrColumns[i].bResizable = 1;
	}
	return xuiTableViewSetColumns(pData->pFindResultList, arrColumns, iCount);
}

static int __xuiCodeEditFindResultTableCount(xui_widget pWidget, void* pUser)
{
	xui_widget pCodeEdit;
	xui_code_find_window_t* pData;

	(void)pWidget;
	pCodeEdit = (xui_widget)pUser;
	pData = __xuiCodeFindWindowGetData(pCodeEdit);
	if ( pData == NULL || pData->bPending ) return 0;
	if ( pData->bFindResultScopeMode && pData->pFindScope != NULL ) {
		return pData->pFindScope->iResultCount;
	}
	return __xuiCodeEditGetData(pData->pEditor) ? __xuiCodeEditGetData(pData->pEditor)->iFindResultCount : 0;
}

static int __xuiCodeEditFindResultTableCell(xui_widget pWidget, int iRow, int iColumn, xui_table_view_cell_t* pCell, void* pUser)
{
	xui_widget pCodeEdit;
	xui_code_find_window_t* pData;
	xui_find_result_t* pFind;
	xui_code_find_result_t* pScopeResult;
	int iLine;
	int iColumnIndex;

	(void)pWidget;
	pCodeEdit = (xui_widget)pUser;
	pData = __xuiCodeFindWindowGetData(pCodeEdit);
	if ( pData == NULL || pData->bPending || pCell == NULL || iRow < 0 || iColumn < 0 ) return 0;
	if ( pData->bFindResultScopeMode ) {
		if ( pData->pFindScope == NULL || iRow >= pData->pFindScope->iResultCount ) return 0;
		pScopeResult = &pData->pFindScope->pResults[iRow];
		if ( iColumn == 0 ) {
			pCell->sText = __xuiCodeFindScopeGetEditorNameByWidget(pData->pFindScope, pScopeResult->pEditor);
			return 1;
		}
		if ( iColumn == 1 ) {
			__xuiCodeEditFormatFindPosition(pScopeResult->iLine, pScopeResult->iColumn, pData->sFindResultPosition, (int)sizeof(pData->sFindResultPosition));
			pCell->sText = pData->sFindResultPosition;
			return 1;
		}
		if ( iColumn == 2 ) {
			pCell->sText = pScopeResult->sPreview;
			return 1;
		}
		return 0;
	}
	xui_code_edit_data_t* pTarget = __xuiCodeEditGetData(pData->pEditor);
	if ( pTarget == NULL || iRow >= pTarget->iFindResultCount ) return 0;
	pFind = &pTarget->pFindResults[iRow];
	if ( iColumn == 0 ) {
		iLine = 0;
		iColumnIndex = 0;
		(void)xuiCodeDocumentOffsetToLineColumn(pTarget->pDocument, pFind->iStart, &iLine, &iColumnIndex);
		__xuiCodeEditFormatFindPosition(iLine, iColumnIndex, pData->sFindResultPosition, (int)sizeof(pData->sFindResultPosition));
		pCell->sText = pData->sFindResultPosition;
		return 1;
	}
	if ( iColumn == 1 ) {
		__xuiCodeEditBuildFindPreview(pTarget->pDocument, pFind->iStart, pData->sFindResultPreview, (int)sizeof(pData->sFindResultPreview));
		pCell->sText = pData->sFindResultPreview;
		return 1;
	}
	return 0;
}

static int __xuiCodeFindWindowRefreshTable(xui_code_find_window_t* pData, int bScope, int iActiveIndex)
{
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiCodeEditFindResultTableColumns(pData, bScope);
	if ( iRet != XUI_OK ) return iRet;
	if ( pData->pFindResultList == NULL ) return XUI_OK;
	iRet = xuiTableViewRefreshAdapter(pData->pFindResultList);
	if ( iRet != XUI_OK ) return iRet;
	if ( iActiveIndex >= 0 ) {
		iRet = xuiTableViewSetSelectedRow(pData->pFindResultList, iActiveIndex);
		if ( iRet == XUI_OK ) iRet = xuiTableViewEnsureCellVisible(pData->pFindResultList, iActiveIndex, 0);
	}
	return iRet;
}

static void __xuiCodeEditFindWindowOptions(xui_code_find_window_t* pData, xui_find_options_t* pOptions)
{
	uint32_t iFlags;

	if ( pOptions == NULL ) return;
	memset(pOptions, 0, sizeof(*pOptions));
	pOptions->iSize = sizeof(*pOptions);
	if ( pData == NULL ) return;
	pOptions->sPattern = (pData->pFindInput != NULL) ? xuiInputGetText(pData->pFindInput) : "";
	pOptions->sReplacement = (pData->pReplaceInput != NULL) ? xuiInputGetText(pData->pReplaceInput) : "";
	pOptions->iStartOffset = -1;
	iFlags = 0u;
	if ( pData->pCaseCheck != NULL && xuiCheckBoxGetChecked(pData->pCaseCheck) ) iFlags |= XUI_FIND_CASE_SENSITIVE;
	if ( pData->pWordCheck != NULL && xuiCheckBoxGetChecked(pData->pWordCheck) ) iFlags |= XUI_FIND_WHOLE_WORD;
	if ( pData->pRegexCheck != NULL && xuiCheckBoxGetChecked(pData->pRegexCheck) ) iFlags |= XUI_FIND_REGEX;
	if ( pData->pEscapeCheck != NULL && xuiCheckBoxGetChecked(pData->pEscapeCheck) ) iFlags |= XUI_FIND_ESCAPE;
	if ( pData->pSelectionCheck != NULL && xuiCheckBoxGetChecked(pData->pSelectionCheck) ) iFlags |= XUI_FIND_SELECTION;
	pOptions->iFlags = iFlags;
	pOptions->iRangeStart = pData->iFindRangeStart;
	pOptions->iRangeEnd = (iFlags & XUI_FIND_SELECTION) ? pData->iFindRangeEnd : -1;
}

static int __xuiCodeEditFindWindowUseScope(xui_code_find_window_t* pData)
{
	return pData != NULL && pData->pFindScope != NULL && pData->pScopeCheck != NULL && xuiCheckBoxGetChecked(pData->pScopeCheck);
}

static void __xuiCodeEditFindWindowSetStatus(xui_code_find_window_t* pData, const char* sText)
{
	if ( pData == NULL || pData->pFindStatus == NULL ) return;
	(void)xuiLabelSetText(pData->pFindStatus, (sText != NULL) ? sText : "");
}

static void __xuiCodeEditFindWindowApplyLanguage(xui_widget pCodeEdit, xui_code_find_window_t* pData)
{
	xui_context pContext;

	if ( pCodeEdit == NULL || pData == NULL || pData->pFindWindow == NULL ) return;
	pContext = xuiWidgetGetContext(pCodeEdit);
	pData->iFindLanguageRevision = xuiGetLanguageRevision(pContext);
	(void)xuiWindowSetTitle(pData->pFindWindow, xuiTranslate(pContext, pData->bFindWindowReplace ? XUI_TR_REPLACE_TITLE : XUI_TR_FIND_TITLE));
	if ( pData->pFindInput != NULL ) (void)xuiInputSetPlaceholder(pData->pFindInput, xuiTranslate(pContext, XUI_TR_FIND_PLACEHOLDER));
	if ( pData->pReplaceInput != NULL ) (void)xuiInputSetPlaceholder(pData->pReplaceInput, xuiTranslate(pContext, XUI_TR_REPLACE_PLACEHOLDER));
	if ( pData->pFindPrevButton != NULL ) (void)xuiButtonSetText(pData->pFindPrevButton, xuiTranslate(pContext, XUI_TR_FIND_PREVIOUS));
	if ( pData->pFindNextButton != NULL ) (void)xuiButtonSetText(pData->pFindNextButton, xuiTranslate(pContext, XUI_TR_FIND_NEXT));
	if ( pData->pFindAllButton != NULL ) (void)xuiButtonSetText(pData->pFindAllButton, xuiTranslate(pContext, XUI_TR_FIND_ALL));
	if ( pData->pReplaceButton != NULL ) (void)xuiButtonSetText(pData->pReplaceButton, xuiTranslate(pContext, XUI_TR_REPLACE_CURRENT));
	if ( pData->pReplaceAllButton != NULL ) (void)xuiButtonSetText(pData->pReplaceAllButton, xuiTranslate(pContext, XUI_TR_REPLACE_ALL));
	if ( pData->pCaseCheck != NULL ) (void)xuiCheckBoxSetText(pData->pCaseCheck, xuiTranslate(pContext, XUI_TR_FIND_CASE));
	if ( pData->pWordCheck != NULL ) (void)xuiCheckBoxSetText(pData->pWordCheck, xuiTranslate(pContext, XUI_TR_FIND_WORD));
	if ( pData->pRegexCheck != NULL ) (void)xuiCheckBoxSetText(pData->pRegexCheck, xuiTranslate(pContext, XUI_TR_FIND_REGEX));
	if ( pData->pEscapeCheck != NULL ) (void)xuiCheckBoxSetText(pData->pEscapeCheck, xuiTranslate(pContext, XUI_TR_FIND_ESCAPE));
	if ( pData->pSelectionCheck != NULL ) (void)xuiCheckBoxSetText(pData->pSelectionCheck, xuiTranslate(pContext, XUI_TR_FIND_SELECTION));
	if ( pData->pScopeCheck != NULL ) (void)xuiCheckBoxSetText(pData->pScopeCheck, xuiTranslate(pContext, XUI_TR_FIND_SCOPE));
	(void)__xuiCodeEditFindResultTableColumns(pData, pData->bFindResultScopeMode);
}

static int __xuiCodeEditSyncScopeList(xui_code_find_window_t* pData, xui_code_find_scope pScope)
{
	if ( pData == NULL || pScope == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiCodeFindWindowRefreshTable(pData, 1, pScope->iActiveIndex);
}

static void __xuiCodeEditFindWindowRefresh(xui_widget pCodeEdit)
{
	xui_code_find_window_t* pData;
	xui_find_options_t tOptions;
	char sStatus[64];
	int iCount;
	int iRet;
	int bUseScope;

	pData = __xuiCodeFindWindowGetData(pCodeEdit);
	if ( pData == NULL ) return;
	__xuiCodeEditFindWindowOptions(pData, &tOptions);
	bUseScope = __xuiCodeEditFindWindowUseScope(pData);
	(void)__xuiCodeEditFindResultTableColumns(pData,bUseScope);
	pData->bPending = 0;
	pData->fPendingElapsed = 0;
	__xuiCodeFindWindowEnable(pData);
	if ( bUseScope ) {
		pData->pFindScope->iResultCount=0;pData->pFindScope->iActiveIndex=-1;
		for(int i=0;i<pData->pFindScope->iEditorCount;++i) (void)xuiCodeEditClearFind(pData->pFindScope->pEditors[i].pEditor);
	} else if(pData->pEditor!=NULL) (void)xuiCodeEditClearFind(pData->pEditor);
	if ( tOptions.sPattern == NULL || tOptions.sPattern[0] == '\0' || (!bUseScope && pData->pEditor == NULL) ) {
		if ( __xuiCodeEditGetData(pData->pEditor) != NULL ) __xuiCodeEditClearFindResults(__xuiCodeEditGetData(pData->pEditor));
		if ( bUseScope && pData->pFindScope != NULL ) {
			pData->pFindScope->iResultCount = 0;
			pData->pFindScope->iActiveIndex = -1;
		}
		(void)__xuiCodeFindWindowRefreshTable(pData, bUseScope, -1);
		(void)xuiWidgetInvalidate(pCodeEdit, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		__xuiCodeEditFindWindowSetStatus(pData, "");
		return;
	}
	iCount = 0;
	if ( (tOptions.iFlags & XUI_FIND_ESCAPE) && !__xuiCodeFindEscapesValid(tOptions.sPattern) ) {
		__xuiCodeEditFindWindowSetStatus(pData, xuiTranslate(xuiWidgetGetContext(pCodeEdit), XUI_TR_FIND_INVALID_PATTERN));
		return;
	}
	xerror* pPreviousError = xrtTakeError();
	if ( bUseScope ) {
		tOptions.iFlags &= ~XUI_FIND_SELECTION;
		iRet = xuiCodeFindScopeFindAll(pData->pFindScope, &tOptions, &iCount);
		if ( iRet == XUI_OK ) iRet = __xuiCodeEditSyncScopeList(pData, pData->pFindScope);
	} else {
		iRet = xuiCodeEditFindAll(pData->pEditor, &tOptions, &iCount);
		if ( iRet == XUI_OK ) iRet = __xuiCodeFindWindowRefreshTable(pData, 0, -1);
	}
	xrtErrorFree(xrtTakeError());
	xrtSetErrorTake(pPreviousError);
	if ( iRet != XUI_OK ) {
		__xuiCodeEditFindWindowSetStatus(pData, xuiTranslate(xuiWidgetGetContext(pCodeEdit), XUI_TR_FIND_INVALID_PATTERN));
		return;
	}
	snprintf(sStatus, sizeof(sStatus), xuiTranslate(xuiWidgetGetContext(pCodeEdit), XUI_TR_FIND_MATCHES_FMT), iCount);
	__xuiCodeEditFindWindowSetStatus(pData, sStatus);
}

static void __xuiCodeEditFindInputChange(xui_widget pWidget, const char* sText, void* pUser)
{
	(void)pWidget;
	(void)sText;
	xui_code_find_window_t* pUi = __xuiCodeFindWindowGetData((xui_widget)pUser);
	if ( pUi != NULL && !pUi->bUpdating ) __xuiCodeFindWindowInvalidate(pUi);
}

static void __xuiCodeEditFindCheckChange(xui_widget pWidget, int bChecked, void* pUser)
{
	xui_code_find_window_t* pUi = __xuiCodeFindWindowGetData((xui_widget)pUser);
	if ( pUi != NULL && !pUi->bUpdating ) {
		if(pWidget==pUi->pSelectionCheck && bChecked && pUi->pEditor)
			(void)xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pUi->pEditor),&pUi->iFindRangeStart,&pUi->iFindRangeEnd);
		if(pWidget==pUi->pScopeCheck && bChecked) (void)xuiCheckBoxSetChecked(pUi->pSelectionCheck,0);
		__xuiCodeFindWindowInvalidate(pUi);
	}
}

static void __xuiCodeEditFindTableSelect(xui_widget pWidget, int iIndex, int iColumn, int iSelectionMode, void* pUser)
{
	xui_code_find_window_t* pUi = __xuiCodeFindWindowGetData((xui_widget)pUser);
	(void)pWidget; (void)iColumn; (void)iSelectionMode;
	if (pUi == NULL || pUi->bUpdating || pUi->bPending || iIndex < 0) return;
	if (__xuiCodeEditFindWindowUseScope(pUi)) {
		(void)xuiCodeFindScopeActivateResult(pUi->pFindScope, iIndex);
	} else {
		xui_code_edit_data_t* pTarget = __xuiCodeEditGetData(pUi->pEditor);
		if (pTarget == NULL || iIndex >= pTarget->iFindResultCount) return;
		xui_find_result_t match = pTarget->pFindResults[iIndex];
		pTarget->iFindActiveIndex = iIndex;
		(void)__xuiCodeEditApplyFindResult(pUi->pEditor, pTarget, &match);
	}
}

static void __xuiCodeEditFindButtonClick(xui_widget pButton, void* pUser)
{
	xui_code_find_window_t* pUi = __xuiCodeFindWindowGetData((xui_widget)pUser);
	xui_find_options_t options;
	int result = XUI_OK, count = 0;
	char status[96];
	if (pUi == NULL) return;
	if(pButton==pUi->pCloseButton){(void)xuiWindowSetOpen(pUi->pFindWindow,0);if(pUi->pEditor)(void)xuiSetFocusWidget(xuiWidgetGetContext(pUi->pOwner),pUi->pEditor);return;}
	if (pUi->bPending || pButton == pUi->pFindAllButton) __xuiCodeEditFindWindowRefresh(pUi->pOwner);
	__xuiCodeEditFindWindowOptions(pUi, &options);
	if (options.sPattern == NULL || !*options.sPattern) return;
	if ((options.iFlags & XUI_FIND_ESCAPE) && (!__xuiCodeFindEscapesValid(options.sPattern) ||
		((pButton == pUi->pReplaceButton || pButton == pUi->pReplaceAllButton) && !__xuiCodeFindEscapesValid(options.sReplacement)))) {
		__xuiCodeEditFindWindowSetStatus(pUi, xuiTranslate(xuiWidgetGetContext(pUi->pOwner), XUI_TR_FIND_INVALID_PATTERN));
		return;
	}
	xerror* previous = xrtTakeError();
	if ((pButton == pUi->pFindNextButton || pButton == pUi->pFindPrevButton) && __xuiCodeEditFindWindowUseScope(pUi)) {
		count = pUi->pFindScope->iResultCount;
		if (count > 0) {
			int index = pUi->pFindScope->iActiveIndex;
			if (index < 0) index = pButton == pUi->pFindPrevButton ? 0 : -1;
			index = (index + (pButton == pUi->pFindPrevButton ? count - 1 : 1)) % count;
			result = xuiCodeFindScopeActivateResult(pUi->pFindScope, index);
			if (result == XUI_OK) {
				pUi->bUpdating = 1;
				(void)__xuiCodeFindWindowRefreshTable(pUi, 1, index);
				pUi->bUpdating = 0;
			}
		}
	} else if (pUi->pEditor != NULL) {
		if (pButton == pUi->pFindNextButton) result = xuiCodeEditFindNext(pUi->pEditor, &options);
		else if (pButton == pUi->pFindPrevButton) result = xuiCodeEditFindPrevious(pUi->pEditor, &options);
		else if (pButton == pUi->pReplaceButton || pButton == pUi->pReplaceAllButton) {
			if (xuiCodeEditIsReadonly(pUi->pEditor)) result = XUI_ERROR_UNSUPPORTED;
			else if (pButton == pUi->pReplaceButton) result = xuiCodeEditReplaceCurrent(pUi->pEditor, &options);
			else result = xuiCodeEditReplaceAll(pUi->pEditor, &options, &count);
			if (result == XUI_OK) {
				__xuiCodeEditFindWindowRefresh(pUi->pOwner);
				if (pButton == pUi->pReplaceAllButton) {
					snprintf(status, sizeof(status), xuiTranslate(xuiWidgetGetContext(pUi->pOwner), XUI_TR_FIND_REPLACED_FMT), count);
					__xuiCodeEditFindWindowSetStatus(pUi, status);
				}
			}
		}
	}
	xrtErrorFree(xrtTakeError());
	xrtSetErrorTake(previous);
	if (result != XUI_OK) __xuiCodeEditFindWindowSetStatus(pUi, xuiTranslate(xuiWidgetGetContext(pUi->pOwner), XUI_TR_FIND_INVALID_PATTERN));
}

static void __xuiCodeEditFindWindowClose(xui_widget pWindow, void* pUser)
{
	xui_code_find_window_t* pUi = __xuiCodeFindWindowGetData((xui_widget)pUser);
	(void)pWindow;
	if (pUi && pUi->pEditor) (void)xuiSetFocusWidget(xuiWidgetGetContext(pUi->pOwner), pUi->pEditor);
}

static int __xuiCodeEditFindWindowKeyDown(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_widget pCodeEdit;
	xui_code_find_window_t* pData;
	int iKey;

	(void)pWidget;
	pCodeEdit = (xui_widget)pUser;
	pData = __xuiCodeFindWindowGetData(pCodeEdit);
	if ( pEvent == NULL || pData == NULL || pData->pFindWindow == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iType != XUI_EVENT_KEY_DOWN ) return XUI_OK;
	iKey = pEvent->iKey;
	if ( iKey >= 'a' && iKey <= 'z' ) iKey = iKey - 'a' + 'A';
	if ( (pEvent->iModifiers & XUI_MOD_CTRL) != 0u ) {
		if ( iKey == 'F' ) {
			(void)xuiCodeEditOpenFind(pData->pEditor);
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( iKey == 'H' ) {
			(void)xuiCodeEditOpenReplace(pData->pEditor);
			return XUI_EVENT_DISPATCH_STOP;
		}
	}
	if ( pEvent->iKey == XUI_KEY_F3 ) {
		__xuiCodeEditFindButtonClick(((pEvent->iModifiers & XUI_MOD_SHIFT) != 0u) ? pData->pFindPrevButton : pData->pFindNextButton, pCodeEdit);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iKey == XUI_KEY_ENTER || pEvent->iKey == XUI_KEY_F3 ) {
		__xuiCodeEditFindButtonClick(((pEvent->iModifiers & XUI_MOD_SHIFT) != 0u) ? pData->pFindPrevButton : pData->pFindNextButton, pCodeEdit);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iKey == XUI_KEY_ESCAPE ) {
		(void)xuiWindowSetOpen(pData->pFindWindow, 0);
		(void)xuiSetFocusWidget(xuiWidgetGetContext(pCodeEdit), pData->pEditor);
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static float __xuiCodeFindStackHeight(xui_widget widget)
{
    xui_thickness_t p=xuiWidgetGetPadding(widget);float h=p.fTop+p.fBottom;int count=0;
    for(xui_widget c=xuiWidgetGetFirstChild(widget);c;c=xuiWidgetGetNextSibling(c))if(xuiWidgetGetVisible(c)){h+=xuiWidgetGetPreferredSize(c).fY;if(count++)h+=xuiWidgetGetGap(widget);}
    return h;
}
static int __xuiCodeEditFindWindowLayout(xui_widget owner,int replace)
{
    xui_code_find_window_t* ui=__xuiCodeFindWindowGetData(owner);
    if(!ui || !ui->pFindWindow || !ui->pBody)return XUI_ERROR_INVALID_ARGUMENT;
    xui_rect_t root=xuiWidgetGetWorldRect(xuiGetRootWidget(xuiWidgetGetContext(owner))),r=xuiWidgetGetRect(ui->pFindWindow);
    float maxW=fmaxf(1,root.fW-16),maxH=fmaxf(1,root.fH-16);
    if(r.fW>maxW)r.fW=maxW;
    if(r.fH>maxH)r.fH=maxH;
    if(r.fX+r.fW>root.fW)r.fX=root.fW-r.fW;
    if(r.fY+r.fH>root.fH)r.fY=root.fH-r.fH;
    if(r.fX<0)r.fX=0;
    if(r.fY<0)r.fY=0;
    xui_rect_t old=xuiWidgetGetRect(ui->pFindWindow);
    if(memcmp(&r,&old,sizeof(r))!=0)(void)xuiWidgetSetRect(ui->pFindWindow,r);
    xui_font font=xuiWindowGetFont(ui->pFindWindow);
    if(ui->fLayoutWidth==r.fW && ui->fLayoutHeight==r.fH && ui->pLayoutFont==font && ui->iLayoutReplace==replace)return XUI_OK;
    ui->fLayoutWidth=r.fW;ui->fLayoutHeight=r.fH;ui->pLayoutFont=font;ui->iLayoutReplace=replace;
    float height=28,width=112;
    xui_widget buttons[]={ui->pFindPrevButton,ui->pFindNextButton,ui->pFindAllButton,ui->pReplaceButton,ui->pReplaceAllButton,ui->pCloseButton};
    for(size_t i=0;i<sizeof(buttons)/sizeof(buttons[0]);++i){xui_vec2_t m={0};if(xuiWidgetMeasureContent(buttons[i],(xui_vec2_t){XUI_LAYOUT_UNBOUNDED,XUI_LAYOUT_UNBOUNDED},&m)==XUI_OK){height=fmaxf(height,m.fY+4);width=fmaxf(width,m.fX+12);}}
    (void)xuiWindowSetMinSize(ui->pFindWindow,fminf(maxW,500),fminf(maxH,330));
    (void)xuiWidgetSetVisible(ui->pReplaceInput,replace);(void)xuiWidgetSetVisible(ui->pReplaceButton,replace);(void)xuiWidgetSetVisible(ui->pReplaceAllButton,replace);
    xui_widget items[]={ui->pFindInput,ui->pReplaceInput,ui->pScopeCheck,ui->pFindStatus};
    for(size_t i=0;i<sizeof(items)/sizeof(items[0]);++i){(void)xuiWidgetSetSizeMode(items[i],XUI_SIZE_FILL,XUI_SIZE_FIXED);(void)xuiWidgetSetPreferredSize(items[i],(xui_vec2_t){0,height});}
    for(int i=0;i<5;++i){(void)xuiWidgetSetSizeMode(ui->pRows[i],XUI_SIZE_FILL,XUI_SIZE_FIXED);(void)xuiWidgetSetPreferredSize(ui->pRows[i],(xui_vec2_t){0,height});}
    xui_widget checks[]={ui->pCaseCheck,ui->pWordCheck,ui->pRegexCheck,ui->pEscapeCheck,ui->pSelectionCheck};
    for(size_t i=0;i<sizeof(checks)/sizeof(checks[0]);++i){(void)xuiWidgetSetSizeMode(checks[i],XUI_SIZE_FILL,XUI_SIZE_FILL);(void)xuiWidgetSetFlex(checks[i],1,1);}
    int columns=r.fW-44>=3*width ? 3:2,visible=0;
    for(size_t i=0;i<sizeof(buttons)/sizeof(buttons[0]);++i){int row=xuiWidgetGetVisible(buttons[i]) ? 2+visible++/columns:4;if(xuiWidgetGetParent(buttons[i])!=ui->pRows[row]){(void)xuiInternalWidgetDetachPart(owner,buttons[i]);(void)xuiInternalWidgetInsertPart(owner,ui->pRows[row],buttons[i],NULL);}(void)xuiWidgetSetSizeMode(buttons[i],XUI_SIZE_FILL,XUI_SIZE_FILL);(void)xuiWidgetSetFlex(buttons[i],1,1);}
    for(int i=2;i<5;++i)(void)xuiWidgetSetVisible(ui->pRows[i],i<2+(visible+columns-1)/columns);
    (void)xuiWidgetSetSizeMode(ui->pFooter,XUI_SIZE_FILL,XUI_SIZE_FIXED);(void)xuiWidgetSetPreferredSize(ui->pFooter,(xui_vec2_t){0,__xuiCodeFindStackHeight(ui->pFooter)});
    (void)xuiWidgetSetSizeMode(ui->pFindResultList,XUI_SIZE_FILL,XUI_SIZE_FIXED);(void)xuiWidgetSetPreferredSize(ui->pFindResultList,(xui_vec2_t){0,fmaxf(160,height*6)});
    return xuiScrollViewSetContentSize(ui->pScroll,fmaxf(1,r.fW-4),__xuiCodeFindStackHeight(ui->pBody));
}

static int __xuiCodeEditCreateFindButton(xui_context pContext, xui_widget* ppButton, xui_font pFont, const char* sText, xui_widget pCodeEdit)
{
	xui_button_desc_t tButton;
	int iRet;

	memset(&tButton, 0, sizeof(tButton));
	tButton.iSize = sizeof(tButton);
	tButton.pFont = pFont;
	tButton.sText = sText;
	tButton.fBorderWidth = 1.0f;
	iRet = xuiInternalButtonCreatePart(pCodeEdit,ppButton,&tButton);
	if ( iRet == XUI_OK ) iRet = xuiButtonSetClick(*ppButton, __xuiCodeEditFindButtonClick, pCodeEdit);
	return iRet;
}

static int __xuiCodeEditCreateFindCheck(xui_context pContext, xui_widget* ppCheck, xui_font pFont, const char* sText, int bChecked, xui_widget pCodeEdit)
{
	xui_checkbox_desc_t tCheck;
	int iRet;

	memset(&tCheck, 0, sizeof(tCheck));
	tCheck.iSize = sizeof(tCheck);
	tCheck.pFont = pFont;
	tCheck.sText = sText;
	tCheck.bChecked = bChecked;
	tCheck.fIndicatorSize = 14.0f;
	tCheck.fGap = 4.0f;
	iRet = xuiInternalCheckBoxCreatePart(pCodeEdit,ppCheck,&tCheck);
	if ( iRet == XUI_OK ) iRet = xuiCheckBoxSetChange(*ppCheck, __xuiCodeEditFindCheckChange, pCodeEdit);
	return iRet;
}

static int __xuiCodeEditCreateFindWindow(xui_widget pCodeEdit, xui_code_find_window_t* pData)
{
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pClient;
	xui_window_desc_t tWindow;
	xui_input_desc_t tInput;
	xui_label_desc_t tLabel;
	xui_table_view_desc_t tTable;
	xui_font pFont;
	int iRet;

	if ( pCodeEdit == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pCodeEdit);
	pRoot = xuiGetRootWidget(pContext);
	if ( pRoot == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	pFont = xuiGetDefaultFont(pContext);
	if ( pFont == NULL && pData->pEditor != NULL ) pFont = xuiCodeEditGetFont(pData->pEditor);
	memset(&tWindow, 0, sizeof(tWindow));
	tWindow.iSize = sizeof(tWindow);
	tWindow.sTitle = xuiTranslate(pContext, XUI_TR_FIND_TITLE);
	tWindow.pFont = pFont;
	tWindow.bClosed = 1;
	tWindow.bTopMost = 1;
	tWindow.bHideCollapse = 1;
	tWindow.bHideMaximize = 1;
	tWindow.bNotResizable = 0;
	tWindow.fTitleBarHeight = 28.0f;
	tWindow.fBorderWidth = 1.0f;
	tWindow.fButtonSize = 18.0f;
	iRet = xuiInternalWindowCreatePart(pCodeEdit,&pData->pFindWindow,&tWindow);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWindowSetClose(pData->pFindWindow, __xuiCodeEditFindWindowClose, pCodeEdit);
	pClient = xuiWindowGetClientWidget(pData->pFindWindow);
	(void)xuiWidgetSetLayoutType(pClient, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetFlowMode(pClient, XUI_FLOW_BLOCK);
	(void)xuiWidgetSetPadding(pClient, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
	(void)xuiWidgetSetGap(pClient, 0.0f);
	(void)xuiWidgetSetEventHandler(pClient, XUI_EVENT_KEY_DOWN, __xuiCodeEditFindWindowKeyDown, pCodeEdit);
	memset(&tInput, 0, sizeof(tInput));
	tInput.iSize = sizeof(tInput);
	tInput.pFont = pFont;
	tInput.sPlaceholder = xuiTranslate(pContext, XUI_TR_FIND_PLACEHOLDER);
	tInput.fBorderWidth = 1.0f;
	iRet = xuiInternalInputCreatePart(pCodeEdit,&pData->pFindInput,&tInput);
	tInput.sPlaceholder = xuiTranslate(pContext, XUI_TR_REPLACE_PLACEHOLDER);
	if ( iRet == XUI_OK ) iRet = xuiInternalInputCreatePart(pCodeEdit,&pData->pReplaceInput,&tInput);
	if ( iRet == XUI_OK ) iRet = __xuiCodeEditCreateFindButton(pContext, &pData->pFindPrevButton, pFont, xuiTranslate(pContext, XUI_TR_FIND_PREVIOUS), pCodeEdit);
	if ( iRet == XUI_OK ) iRet = __xuiCodeEditCreateFindButton(pContext, &pData->pFindNextButton, pFont, xuiTranslate(pContext, XUI_TR_FIND_NEXT), pCodeEdit);
	if ( iRet == XUI_OK ) iRet = __xuiCodeEditCreateFindButton(pContext, &pData->pFindAllButton, pFont, xuiTranslate(pContext, XUI_TR_FIND_ALL), pCodeEdit);
	if ( iRet == XUI_OK ) iRet = __xuiCodeEditCreateFindButton(pContext, &pData->pReplaceButton, pFont, xuiTranslate(pContext, XUI_TR_REPLACE_CURRENT), pCodeEdit);
	if ( iRet == XUI_OK ) iRet = __xuiCodeEditCreateFindButton(pContext, &pData->pReplaceAllButton, pFont, xuiTranslate(pContext, XUI_TR_REPLACE_ALL), pCodeEdit);
	if ( iRet == XUI_OK ) iRet = __xuiCodeEditCreateFindCheck(pContext, &pData->pCaseCheck, pFont, xuiTranslate(pContext, XUI_TR_FIND_CASE), ((pData->pEditor ? __xuiCodeEditGetData(pData->pEditor)->iFindFlags : 0u) & XUI_FIND_CASE_SENSITIVE) != 0, pCodeEdit);
	if ( iRet == XUI_OK ) iRet = __xuiCodeEditCreateFindCheck(pContext, &pData->pWordCheck, pFont, xuiTranslate(pContext, XUI_TR_FIND_WORD), ((pData->pEditor ? __xuiCodeEditGetData(pData->pEditor)->iFindFlags : 0u) & XUI_FIND_WHOLE_WORD) != 0, pCodeEdit);
	if ( iRet == XUI_OK ) iRet = __xuiCodeEditCreateFindCheck(pContext, &pData->pRegexCheck, pFont, xuiTranslate(pContext, XUI_TR_FIND_REGEX), ((pData->pEditor ? __xuiCodeEditGetData(pData->pEditor)->iFindFlags : 0u) & XUI_FIND_REGEX) != 0, pCodeEdit);
	if ( iRet == XUI_OK ) iRet = __xuiCodeEditCreateFindCheck(pContext, &pData->pEscapeCheck, pFont, xuiTranslate(pContext, XUI_TR_FIND_ESCAPE), ((pData->pEditor ? __xuiCodeEditGetData(pData->pEditor)->iFindFlags : 0u) & XUI_FIND_ESCAPE) != 0, pCodeEdit);
	if ( iRet == XUI_OK ) iRet = __xuiCodeEditCreateFindCheck(pContext, &pData->pSelectionCheck, pFont, xuiTranslate(pContext, XUI_TR_FIND_SELECTION), ((pData->pEditor ? __xuiCodeEditGetData(pData->pEditor)->iFindFlags : 0u) & XUI_FIND_SELECTION) != 0, pCodeEdit);
	if ( iRet == XUI_OK ) iRet = __xuiCodeEditCreateFindCheck(pContext, &pData->pScopeCheck, pFont, xuiTranslate(pContext, XUI_TR_FIND_SCOPE), 0, pCodeEdit);
	memset(&tTable, 0, sizeof(tTable));
	tTable.iSize = sizeof(tTable);
	tTable.pFont = pFont;
	tTable.fDefaultRowHeight = 22.0f;
	tTable.fHeaderHeight = 24.0f;
	tTable.iSelectionMode = XUI_TABLE_VIEW_SELECTION_ROW;
	tTable.onCount = __xuiCodeEditFindResultTableCount;
	tTable.onCell = __xuiCodeEditFindResultTableCell;
	tTable.pAdapterUser = pCodeEdit;
	if ( iRet == XUI_OK ) iRet = xuiInternalTableViewCreatePart(pCodeEdit,&pData->pFindResultList,&tTable);
	if ( iRet == XUI_OK ) iRet = xuiTableViewSetSelect(pData->pFindResultList, __xuiCodeEditFindTableSelect, pCodeEdit);
	if ( iRet == XUI_OK ) iRet = __xuiCodeEditFindResultTableColumns(pData, 0);
	memset(&tLabel, 0, sizeof(tLabel));
	tLabel.iSize = sizeof(tLabel);
	tLabel.pFont = pFont;
	tLabel.sText = "";
	tLabel.iTextColor = XUI_COLOR_RGBA(90, 105, 124, 255);
	tLabel.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	if ( iRet == XUI_OK ) iRet = xuiInternalLabelCreatePart(pCodeEdit,&pData->pFindStatus,&tLabel);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiInputSetChange(pData->pFindInput, __xuiCodeEditFindInputChange, pCodeEdit);
	(void)xuiInputSetChange(pData->pReplaceInput, __xuiCodeEditFindInputChange, pCodeEdit);
    xui_scroll_view_desc_t scroll={0};scroll.iSize=sizeof(scroll);
    iRet=xuiInternalScrollViewCreatePart(pCodeEdit,&pData->pScroll,&scroll);
    if(iRet==XUI_OK)pData->pBody=xuiScrollViewGetContentWidget(pData->pScroll);
    if(iRet==XUI_OK)iRet=xuiInternalWidgetCreatePart(pCodeEdit,xuiWidgetGetBaseType(),&pData->pFooter,NULL);
    for(int i=0;i<5 && iRet==XUI_OK;++i)iRet=xuiInternalWidgetCreatePart(pCodeEdit,xuiWidgetGetBaseType(),&pData->pRows[i],NULL);
    if(iRet==XUI_OK)iRet=__xuiCodeEditCreateFindButton(pContext,&pData->pCloseButton,pFont,xuiTranslate(pContext,XUI_TR_DOCK_CLOSE),pCodeEdit);
    if(iRet!=XUI_OK)return iRet;
    (void)xuiWidgetSetLayoutType(pData->pBody,XUI_LAYOUT_COLUMN);(void)xuiWidgetSetLayoutType(pData->pFooter,XUI_LAYOUT_COLUMN);
    (void)xuiWidgetSetPadding(pData->pBody,(xui_thickness_t){12,12,12,12});(void)xuiWidgetSetGap(pData->pBody,8);
    (void)xuiWidgetSetPadding(pData->pFooter,(xui_thickness_t){12,8,12,8});(void)xuiWidgetSetGap(pData->pFooter,6);
    for(int i=0;i<5;++i){(void)xuiWidgetSetLayoutType(pData->pRows[i],XUI_LAYOUT_ROW);(void)xuiWidgetSetGap(pData->pRows[i],8);}
    (void)xuiWidgetSetSizeMode(pData->pScroll,XUI_SIZE_FILL,XUI_SIZE_FILL);(void)xuiWidgetSetFlex(pData->pScroll,1,1);
    iRet=xuiScrollViewSetScrollbarPolicy(pData->pScroll,XUI_SCROLLBAR_POLICY_HIDDEN,XUI_SCROLLBAR_POLICY_AUTO);
    if(iRet==XUI_OK)iRet=xuiInternalWidgetInsertPart(pCodeEdit,pClient,pData->pScroll,NULL);
    xui_widget separator=NULL;xui_separator_desc_t line={0};line.iSize=sizeof(line);line.iOrientation=XUI_ORIENTATION_HORIZONTAL;
    if(iRet==XUI_OK)iRet=xuiInternalWidgetCreatePart(pCodeEdit,xuiSeparatorGetType(pContext),&separator,&line);
    if(iRet==XUI_OK){(void)xuiWidgetSetSizeMode(separator,XUI_SIZE_FILL,XUI_SIZE_FIXED);(void)xuiWidgetSetPreferredSize(separator,(xui_vec2_t){0,1});iRet=xuiInternalWidgetInsertPart(pCodeEdit,pClient,separator,NULL);}
    if(iRet==XUI_OK)iRet=xuiInternalWidgetInsertPart(pCodeEdit,pClient,pData->pFooter,NULL);
    xui_widget body[]={pData->pFindInput,pData->pReplaceInput,pData->pRows[0],pData->pRows[1],pData->pScopeCheck,pData->pFindStatus,pData->pFindResultList};
    for(size_t i=0;i<sizeof(body)/sizeof(body[0]) && iRet==XUI_OK;++i)iRet=xuiInternalWidgetInsertPart(pCodeEdit,pData->pBody,body[i],NULL);
    xui_widget checks[]={pData->pCaseCheck,pData->pWordCheck,pData->pRegexCheck,pData->pEscapeCheck,pData->pSelectionCheck};
    for(size_t i=0;i<sizeof(checks)/sizeof(checks[0]) && iRet==XUI_OK;++i)iRet=xuiInternalWidgetInsertPart(pCodeEdit,pData->pRows[i/3],checks[i],NULL);
    for(int i=2;i<5 && iRet==XUI_OK;++i)iRet=xuiInternalWidgetInsertPart(pCodeEdit,pData->pFooter,pData->pRows[i],NULL);
    if(iRet==XUI_OK)iRet=xuiInternalWidgetInsertPart(pCodeEdit,pRoot,pData->pFindWindow,NULL);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pData->pFindWindow, (xui_rect_t){32.0f, 32.0f, 720.0f, 384.0f});
	__xuiCodeEditFindWindowApplyLanguage(pCodeEdit, pData);
	return __xuiCodeEditFindWindowLayout(pCodeEdit, 0);
}

static void __xuiCodeFindWindowSetFont(xui_code_find_window_t* pUi, xui_font pFont)
{
	if (pUi == NULL || pUi->pFindWindow == NULL) return;
	(void)xuiWindowSetFont(pUi->pFindWindow, pFont);
	(void)xuiInputSetFont(pUi->pFindInput, pFont);
	(void)xuiInputSetFont(pUi->pReplaceInput, pFont);
	xui_widget buttons[] = {pUi->pFindPrevButton,pUi->pFindNextButton,pUi->pFindAllButton,pUi->pReplaceButton,pUi->pReplaceAllButton,pUi->pCloseButton};
	xui_widget checks[] = {pUi->pCaseCheck,pUi->pWordCheck,pUi->pRegexCheck,pUi->pEscapeCheck,pUi->pSelectionCheck,pUi->pScopeCheck};
	for (size_t i=0;i<sizeof(buttons)/sizeof(buttons[0]);++i) (void)xuiButtonSetFont(buttons[i],pFont);
	for (size_t i=0;i<sizeof(checks)/sizeof(checks[0]);++i) (void)xuiCheckBoxSetFont(checks[i],pFont);
	(void)xuiTableViewSetFont(pUi->pFindResultList,pFont);
	(void)xuiLabelSetFont(pUi->pFindStatus,pFont);
	pUi->pLayoutFont=NULL;
}

static void __xuiCodeFindWindowOwnerDestroy(xui_widget pOwner, void* pTypeData, void* pUser)
{
	xui_code_find_window_t* pUi=pTypeData;
	(void)pUser;
	if (pUi->pFindScope != NULL && pUi->pFindScope->pFindUi==pUi) pUi->pFindScope->pFindUi=NULL;
	xui_code_edit_data_t* pTarget=__xuiCodeEditGetData(pUi->pEditor);
	if (pTarget != NULL && pTarget->pFindUi==pUi) pTarget->pFindUi=NULL;
	if (pUi->pFindWindow != NULL) (void)xuiInternalWidgetDestroyPart(pOwner,pUi->pFindWindow);
	pUi->pFindWindow=NULL;
}

static void __xuiCodeFindWindowDestroy(xui_code_find_window_t* pUi)
{
	if (pUi != NULL) (void)xuiWidgetDestroyChecked(pUi->pOwner);
}

static int __xuiCodeFindWindowOwnerUpdate(xui_widget pOwner, float fDelta, void* pUser)
{
	xui_code_find_window_t* pUi=__xuiCodeFindWindowGetData(pOwner);
	(void)pUser;
	if (pUi==NULL || pUi->pFindWindow==NULL || !xuiWindowIsOpen(pUi->pFindWindow)) return XUI_OK;
	if (pUi->iFindLanguageRevision!=xuiGetLanguageRevision(xuiWidgetGetContext(pOwner))) __xuiCodeEditFindWindowApplyLanguage(pOwner,pUi);
	if (pUi->pEditor != NULL) {
		uint32_t version=xuiCodeDocumentGetVersion(xuiCodeEditGetDocument(pUi->pEditor));
		if (version!=pUi->iTargetVersion) {
			pUi->iTargetVersion=version;
			pUi->iFindRangeStart=pUi->iFindRangeEnd=0;
			(void)xuiCheckBoxSetChecked(pUi->pSelectionCheck,0);
			__xuiCodeFindWindowInvalidate(pUi);
		}
	}
	__xuiCodeFindWindowEnable(pUi);
	(void)__xuiCodeEditFindWindowLayout(pOwner,pUi->bFindWindowReplace);
	if (pUi->bPending) {
		if (fDelta>0) pUi->fPendingElapsed+=fDelta;
		if (pUi->fPendingElapsed>=0.08f) __xuiCodeEditFindWindowRefresh(pOwner);
	}
	return XUI_OK;
}

static int __xuiCodeFindWindowCreate(xui_widget pEditor, xui_code_find_scope pScope, xui_code_find_window_t** ppUi)
{
	xui_context pContext=xuiWidgetGetContext(pEditor);
	xui_widget_type pType=xuiWidgetFindType(pContext,"code-find-controller");
	xui_widget pOwner=NULL;
	int result;
	*ppUi=NULL;
	if (pType==NULL) {
		xui_widget_type_desc_t desc={0};
		desc.iSize=sizeof(desc);desc.sName="code-find-controller";
		desc.pParent=xuiWidgetGetBaseType();desc.iTypeDataSize=sizeof(xui_code_find_window_t);
		desc.onDestroy=__xuiCodeFindWindowOwnerDestroy;desc.onUpdate=__xuiCodeFindWindowOwnerUpdate;
		result=xuiWidgetRegisterType(pContext,&pType,&desc);
		if(result!=XUI_OK)return result;
	}
	result=xuiWidgetCreateTyped(pContext,pType,&pOwner,NULL);
	if(result!=XUI_OK)return result;
	xui_code_find_window_t* pUi=__xuiCodeFindWindowGetData(pOwner);
	pUi->pOwner=pOwner;pUi->pEditor=pEditor;pUi->pFindScope=pScope;
	pUi->bFindResultScopeMode=-1;
	(void)xuiWidgetSetFlowMode(pOwner,XUI_FLOW_ABSOLUTE);
	(void)xuiWidgetSetRect(pOwner,(xui_rect_t){0,0,0,0});
	result=xuiWidgetAddChild(xuiGetRootWidget(pContext),pOwner);
	if(result==XUI_OK)result=__xuiCodeEditCreateFindWindow(pOwner,pUi);
	if(result!=XUI_OK){__xuiCodeFindWindowDestroy(pUi);return result;}
	*ppUi=pUi;
	return XUI_OK;
}

XUI_API int xuiCodeFindScopeSetActiveEditor(xui_code_find_scope pScope, xui_widget pEditor)
{
	if(pScope==NULL)return XUI_ERROR_INVALID_ARGUMENT;
	if(pEditor!=NULL) {
		int found=0;
		for(int i=0;i<pScope->iEditorCount;++i)if(pScope->pEditors[i].pEditor==pEditor){found=1;break;}
		if(!found)return XUI_ERROR_INVALID_ARGUMENT;
	}
	if(pScope->pActiveEditor==pEditor)return XUI_OK;
	pScope->pActiveEditor=pEditor;
	xui_code_find_window_t* pUi=pScope->pFindUi;
	if(pUi!=NULL) {
		pUi->pEditor=pEditor;pUi->iFindRangeStart=pUi->iFindRangeEnd=0;
		pUi->iTargetVersion=pEditor ? xuiCodeDocumentGetVersion(xuiCodeEditGetDocument(pEditor)):0;
		if(pEditor) (void)xuiCodeSelectionGetRange(xuiCodeEditGetSelection(pEditor),&pUi->iFindRangeStart,&pUi->iFindRangeEnd);
		if(!__xuiCodeEditFindWindowUseScope(pUi)) __xuiCodeFindWindowInvalidate(pUi);
		__xuiCodeFindWindowEnable(pUi);
	}
	return XUI_OK;
}

XUI_API xui_widget xuiCodeFindScopeGetActiveEditor(xui_code_find_scope pScope)
{
	return pScope ? pScope->pActiveEditor:NULL;
}

XUI_API xui_widget xuiCodeFindScopeGetFindWindow(xui_code_find_scope pScope)
{
	return pScope && pScope->pFindUi ? pScope->pFindUi->pFindWindow:NULL;
}

static int __xuiCodeEditOpenFindWindow(xui_widget pEditor, int bReplace)
{
	xui_code_edit_data_t* pData=__xuiCodeEditGetData(pEditor);
	if(pData==NULL)return XUI_ERROR_INVALID_ARGUMENT;
	xui_code_find_scope pScope=pData->pFindScope;
	xui_code_find_window_t** ppUi=pScope ? &pScope->pFindUi:&pData->pFindUi;
	int result;
	if(pScope!=NULL){result=xuiCodeFindScopeSetActiveEditor(pScope,pEditor);if(result!=XUI_OK)return result;}
	if(*ppUi==NULL){result=__xuiCodeFindWindowCreate(pEditor,pScope,ppUi);if(result!=XUI_OK)return result;}
	xui_code_find_window_t* pUi=*ppUi;
	pUi->pEditor=pEditor;pUi->bFindWindowReplace=bReplace!=0;pUi->bUpdating=1;
	pUi->iTargetVersion=xuiCodeDocumentGetVersion(pData->pDocument);
	pUi->iFindRangeStart=pUi->iFindRangeEnd=0;
	(void)xuiCodeSelectionGetRange(pData->pSelection,&pUi->iFindRangeStart,&pUi->iFindRangeEnd);
	int size=pUi->iFindRangeEnd-pUi->iFindRangeStart;
	if(!xuiWindowIsOpen(pUi->pFindWindow) && size>0 && size<256) {
		char selected[256];
		if(xuiCodeDocumentCopyRange(pData->pDocument,pUi->iFindRangeStart,pUi->iFindRangeEnd,selected,sizeof(selected),NULL)==XUI_OK)
			(void)xuiInputSetText(pUi->pFindInput,selected);
	}
	pUi->bUpdating=0;
	(void)xuiWidgetSetVisible(pUi->pScopeCheck,pScope!=NULL);
	__xuiCodeEditFindWindowApplyLanguage(pUi->pOwner,pUi);
	(void)__xuiCodeEditFindWindowLayout(pUi->pOwner,bReplace);
	if(!xuiWindowIsOpen(pUi->pFindWindow)) {
		xui_rect_t owner=xuiWidgetGetWorldRect(pEditor),window=xuiWidgetGetRect(pUi->pFindWindow);
		window.fX=owner.fX+16;window.fY=owner.fY+16;
		(void)xuiWidgetSetRect(pUi->pFindWindow,window);
	}
	(void)xuiWindowSetOpen(pUi->pFindWindow,1);
	(void)xuiWindowBringToFront(pUi->pFindWindow);
	__xuiCodeEditFindWindowRefresh(pUi->pOwner);
	return xuiSetFocusWidget(xuiWidgetGetContext(pEditor),pUi->pFindInput);
}

XUI_API int xuiCodeEditOpenFind(xui_widget pEditor){return __xuiCodeEditOpenFindWindow(pEditor,0);}
XUI_API int xuiCodeEditOpenReplace(xui_widget pEditor){return __xuiCodeEditOpenFindWindow(pEditor,1);}
XUI_API xui_widget xuiCodeEditGetFindWindow(xui_widget pEditor)
{
	xui_code_edit_data_t* pData=__xuiCodeEditGetData(pEditor);
	xui_code_find_window_t* pUi=pData ? (pData->pFindScope ? pData->pFindScope->pFindUi:pData->pFindUi):NULL;
	return pUi ? pUi->pFindWindow:NULL;
}

XUI_API int xuiCodeFindScopeFindNext(xui_code_find_scope pScope, int bPrevious)
{
	if(pScope==NULL || pScope->pFindUi==NULL || !xuiWindowIsOpen(pScope->pFindUi->pFindWindow)) return XUI_ERROR_UNSUPPORTED;
	xui_code_find_window_t* pUi=pScope->pFindUi;
	xui_call_scope_t guard={0};int result=xuiCallScopeBegin(xuiWidgetGetContext(pUi->pOwner),&guard);
	if(result!=XUI_OK)return result;
	__xuiCodeEditFindButtonClick(bPrevious ? pUi->pFindPrevButton:pUi->pFindNextButton,pUi->pOwner);
	return xuiCallScopeEnd(&guard);
}

XUI_API int xuiCodeFindScopeRefresh(xui_code_find_scope pScope)
{
	if(pScope==NULL)return XUI_ERROR_INVALID_ARGUMENT;
	if(pScope->pFindUi!=NULL)__xuiCodeEditFindWindowRefresh(pScope->pFindUi->pOwner);
	return XUI_OK;
}

