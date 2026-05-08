static void __xgeXuiMessageBoxUpdateButtons(xge_xui_message_box pBox)
{
	if ( pBox == NULL ) {
		return;
	}
	pBox->iButtonCount = 1;
	pBox->arrButtonText[0] = "OK";
	pBox->arrButtonResult[0] = XGE_XUI_MESSAGE_BOX_RESULT_OK;
	if ( pBox->iButtons == XGE_XUI_MESSAGE_BOX_OK_CANCEL ) {
		pBox->iButtonCount = 2;
		pBox->arrButtonText[0] = "OK";
		pBox->arrButtonResult[0] = XGE_XUI_MESSAGE_BOX_RESULT_OK;
		pBox->arrButtonText[1] = "Cancel";
		pBox->arrButtonResult[1] = XGE_XUI_MESSAGE_BOX_RESULT_CANCEL;
	} else if ( pBox->iButtons == XGE_XUI_MESSAGE_BOX_YES_NO ) {
		pBox->iButtonCount = 2;
		pBox->arrButtonText[0] = "Yes";
		pBox->arrButtonResult[0] = XGE_XUI_MESSAGE_BOX_RESULT_YES;
		pBox->arrButtonText[1] = "No";
		pBox->arrButtonResult[1] = XGE_XUI_MESSAGE_BOX_RESULT_NO;
	} else if ( pBox->iButtons == XGE_XUI_MESSAGE_BOX_YES_NO_CANCEL ) {
		pBox->iButtonCount = 3;
		pBox->arrButtonText[0] = "Yes";
		pBox->arrButtonResult[0] = XGE_XUI_MESSAGE_BOX_RESULT_YES;
		pBox->arrButtonText[1] = "No";
		pBox->arrButtonResult[1] = XGE_XUI_MESSAGE_BOX_RESULT_NO;
		pBox->arrButtonText[2] = "Cancel";
		pBox->arrButtonResult[2] = XGE_XUI_MESSAGE_BOX_RESULT_CANCEL;
	}
}

static void __xgeXuiMessageBoxCloseResult(xge_xui_message_box pBox, int iResult)
{
	if ( pBox == NULL ) {
		return;
	}
	if ( !xgeXuiMessageBoxIsOpen(pBox) ) {
		return;
	}
	pBox->iResult = iResult;
	xgeXuiDialogSetOpen(&pBox->tDialog, 0);
	if ( pBox->procResult != NULL ) {
		pBox->procResult(pBox->pWidget, iResult, pBox->pUser);
	}
}

static int __xgeXuiMessageBoxButtonAt(xge_xui_message_box pBox, float fX, float fY)
{
	int i;

	if ( pBox == NULL ) {
		return -1;
	}
	for ( i = 0; i < pBox->iButtonCount; i++ ) {
		if ( __xgeXuiRectContains(pBox->arrButtonRect[i], fX, fY) ) {
			return i;
		}
	}
	return -1;
}

int xgeXuiMessageBoxInit(xge_xui_message_box pBox, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pBox == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pBox, 0, sizeof(*pBox));
	pTheme = xgeXuiGetTheme(pContext);
	pBox->pContext = pContext;
	pBox->pWidget = pWidget;
	pBox->pFont = pTheme->pFont;
	pBox->sMessage = "";
	pBox->iType = XGE_XUI_MESSAGE_BOX_INFO;
	pBox->iButtons = XGE_XUI_MESSAGE_BOX_OK;
	pBox->iResult = XGE_XUI_MESSAGE_BOX_RESULT_NONE;
	pBox->iMessageColor = pTheme->iTextColor;
	pBox->iButtonColor = pTheme->iStateNormal;
	pBox->iButtonHoverColor = pTheme->iStateHover;
	pBox->iButtonTextColor = pTheme->iTextColor;
	pBox->iHoverButton = -1;
	if ( xgeXuiDialogInit(&pBox->tDialog, pContext, pWidget) != XGE_OK ) {
		memset(pBox, 0, sizeof(*pBox));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget->procEvent = xgeXuiMessageBoxEventProc;
	pWidget->procPaint = xgeXuiMessageBoxPaintProc;
	pWidget->pUser = pBox;
	__xgeXuiMessageBoxUpdateButtons(pBox);
	return XGE_OK;
}

void xgeXuiMessageBoxUnit(xge_xui_message_box pBox)
{
	if ( pBox == NULL ) {
		return;
	}
	if ( pBox->pWidget != NULL && pBox->pWidget->pUser == pBox ) {
		pBox->pWidget->pUser = NULL;
		pBox->pWidget->procEvent = NULL;
		pBox->pWidget->procPaintBefore = NULL;
		pBox->pWidget->procPaint = NULL;
		pBox->pWidget->pPaintBeforeUser = NULL;
		pBox->pWidget->iCallbackFlags &= ~XGE_XUI_WIDGET_CALLBACK_PAINT_BEFORE;
	}
	memset(pBox, 0, sizeof(*pBox));
}

void xgeXuiMessageBoxSetText(xge_xui_message_box pBox, xge_font pFont, const char* sTitle, const char* sMessage)
{
	if ( pBox == NULL ) {
		return;
	}
	pBox->pFont = pFont;
	pBox->sMessage = (sMessage != NULL) ? sMessage : "";
	xgeXuiDialogSetTitle(&pBox->tDialog, pFont, (sTitle != NULL) ? sTitle : "");
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

void xgeXuiMessageBoxSetType(xge_xui_message_box pBox, int iType)
{
	if ( pBox == NULL ) {
		return;
	}
	if ( (iType < XGE_XUI_MESSAGE_BOX_INFO) || (iType > XGE_XUI_MESSAGE_BOX_QUESTION) ) {
		iType = XGE_XUI_MESSAGE_BOX_INFO;
	}
	pBox->iType = iType;
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

void xgeXuiMessageBoxSetButtons(xge_xui_message_box pBox, int iButtons)
{
	if ( pBox == NULL ) {
		return;
	}
	if ( (iButtons < XGE_XUI_MESSAGE_BOX_OK) || (iButtons > XGE_XUI_MESSAGE_BOX_YES_NO_CANCEL) ) {
		iButtons = XGE_XUI_MESSAGE_BOX_OK;
	}
	pBox->iButtons = iButtons;
	__xgeXuiMessageBoxUpdateButtons(pBox);
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

void xgeXuiMessageBoxSetResult(xge_xui_message_box pBox, xge_xui_select_proc procResult, void* pUser)
{
	if ( pBox == NULL ) {
		return;
	}
	pBox->procResult = procResult;
	pBox->pUser = pUser;
}

void xgeXuiMessageBoxSetOpen(xge_xui_message_box pBox, int bOpen)
{
	if ( pBox == NULL ) {
		return;
	}
	if ( bOpen ) {
		pBox->iResult = XGE_XUI_MESSAGE_BOX_RESULT_NONE;
	}
	xgeXuiDialogSetOpen(&pBox->tDialog, bOpen);
}

int xgeXuiMessageBoxIsOpen(xge_xui_message_box pBox)
{
	if ( pBox == NULL ) {
		return 0;
	}
	return xgeXuiDialogIsOpen(&pBox->tDialog);
}

int xgeXuiMessageBoxGetResult(xge_xui_message_box pBox)
{
	if ( pBox == NULL ) {
		return XGE_XUI_MESSAGE_BOX_RESULT_NONE;
	}
	return pBox->iResult;
}

void xgeXuiMessageBoxSetColors(xge_xui_message_box pBox, uint32_t iBackdrop, uint32_t iBackground, uint32_t iTitle, uint32_t iClose, uint32_t iMessage, uint32_t iButton, uint32_t iButtonHover, uint32_t iButtonText)
{
	if ( pBox == NULL ) {
		return;
	}
	xgeXuiDialogSetColors(&pBox->tDialog, iBackdrop, iBackground, iTitle, iClose);
	pBox->iMessageColor = iMessage;
	pBox->iButtonColor = iButton;
	pBox->iButtonHoverColor = iButtonHover;
	pBox->iButtonTextColor = iButtonText;
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

int xgeXuiMessageBoxEvent(xge_xui_message_box pBox, const xge_event_t* pEvent)
{
	int iButton;

	if ( (pBox == NULL) || (pEvent == NULL) || !xgeXuiMessageBoxIsOpen(pBox) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_MOUSE_MOVE ) {
		pBox->iHoverButton = __xgeXuiMessageBoxButtonAt(pBox, pEvent->fX, pEvent->fY);
		xgeXuiWidgetMarkPaint(pBox->pWidget);
		return pBox->tDialog.bModal ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_MOUSE_DOWN ) {
		iButton = __xgeXuiMessageBoxButtonAt(pBox, pEvent->fX, pEvent->fY);
		if ( iButton >= 0 ) {
			__xgeXuiMessageBoxCloseResult(pBox, pBox->arrButtonResult[iButton]);
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN ) {
		if ( pEvent->iParam1 == XGE_KEY_ENTER ) {
			__xgeXuiMessageBoxCloseResult(pBox, pBox->arrButtonResult[0]);
			return XGE_XUI_EVENT_CONSUMED;
		}
		if ( pEvent->iParam1 == XGE_KEY_ESCAPE ) {
			__xgeXuiMessageBoxCloseResult(pBox, (pBox->iButtons == XGE_XUI_MESSAGE_BOX_OK) ? XGE_XUI_MESSAGE_BOX_RESULT_OK : XGE_XUI_MESSAGE_BOX_RESULT_CANCEL);
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	return xgeXuiDialogEvent(&pBox->tDialog, pEvent);
}

int xgeXuiMessageBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiMessageBoxEvent((xge_xui_message_box)pUser, pEvent);
}

void xgeXuiMessageBoxPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_message_box pBox;
	xge_rect_t tContent;
	xge_rect_t tMessage;
	xge_rect_t tButton;
	xge_rect_t tIcon;
	uint32_t iIconColor;
	float fButtonW;
	float fGap;
	float fTotalW;
	float fStartX;
	int i;

	pBox = (xge_xui_message_box)pUser;
	if ( (pWidget == NULL) || (pBox == NULL) || !xgeXuiMessageBoxIsOpen(pBox) ) {
		return;
	}
	xgeXuiDialogPaintProc(pWidget, &pBox->tDialog);
	tContent = pWidget->tContentRect;
	tIcon = (xge_rect_t){ tContent.fX + 14.0f, tContent.fY + 38.0f, 18.0f, 18.0f };
	iIconColor = XGE_COLOR_RGBA(22, 138, 194, 255);
	if ( pBox->iType == XGE_XUI_MESSAGE_BOX_WARNING ) {
		iIconColor = XGE_COLOR_RGBA(214, 154, 42, 255);
	} else if ( pBox->iType == XGE_XUI_MESSAGE_BOX_ERROR ) {
		iIconColor = XGE_COLOR_RGBA(204, 84, 84, 255);
	} else if ( pBox->iType == XGE_XUI_MESSAGE_BOX_QUESTION ) {
		iIconColor = XGE_COLOR_RGBA(71, 130, 190, 255);
	}
	__xgeXuiHostDrawRect(tIcon, iIconColor);
	__xgeXuiHostDrawBorderRect(tIcon, 1.0f, XGE_COLOR_RGBA(127, 196, 229, 255));
	tMessage = (xge_rect_t){ tIcon.fX + tIcon.fW + 10.0f, tContent.fY + 32.0f, tContent.fW - 56.0f, tContent.fH - 72.0f };
	if ( pBox->pFont != NULL ) {
		__xgeXuiHostDrawTextRect(pBox->pFont, pBox->sMessage, tMessage, pBox->iMessageColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	fButtonW = 68.0f;
	fGap = 8.0f;
	fTotalW = (float)pBox->iButtonCount * fButtonW + (float)(pBox->iButtonCount - 1) * fGap;
	fStartX = tContent.fX + tContent.fW - fTotalW - 12.0f;
	for ( i = 0; i < pBox->iButtonCount; i++ ) {
		tButton = (xge_rect_t){ fStartX + (float)i * (fButtonW + fGap), tContent.fY + tContent.fH - 34.0f, fButtonW, 24.0f };
		pBox->arrButtonRect[i] = tButton;
		__xgeXuiHostDrawRect(tButton, (i == pBox->iHoverButton) ? pBox->iButtonHoverColor : pBox->iButtonColor);
		__xgeXuiHostDrawBorderRect(tButton, 1.0f, XGE_COLOR_RGBA(127, 196, 229, 255));
		if ( pBox->pFont != NULL ) {
			__xgeXuiHostDrawTextRect(pBox->pFont, pBox->arrButtonText[i], tButton, pBox->iButtonTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
}
