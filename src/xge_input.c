int xgeKeyDown(int iKey)
{
	if ( (iKey < 0) || (iKey >= XGE_KEY_COUNT) ) {
		return 0;
	}
	return g_xge.arrKeyDown[iKey] ? 1 : 0;
}

int xgeKeyPressed(int iKey)
{
	if ( (iKey < 0) || (iKey >= XGE_KEY_COUNT) ) {
		return 0;
	}
	return g_xge.arrKeyPressed[iKey] ? 1 : 0;
}

int xgeKeyReleased(int iKey)
{
	if ( (iKey < 0) || (iKey >= XGE_KEY_COUNT) ) {
		return 0;
	}
	return g_xge.arrKeyReleased[iKey] ? 1 : 0;
}

void xgeMouseGet(float* pX, float* pY)
{
	if ( pX != NULL ) {
		*pX = g_xge.fMouseX;
	}
	if ( pY != NULL ) {
		*pY = g_xge.fMouseY;
	}
}

void xgeMouseGetDelta(float* pDX, float* pDY)
{
	if ( pDX != NULL ) {
		*pDX = g_xge.fMouseDX;
	}
	if ( pDY != NULL ) {
		*pDY = g_xge.fMouseDY;
	}
}

void xgeMouseGetWheel(float* pX, float* pY)
{
	if ( pX != NULL ) {
		*pX = g_xge.fMouseWheelX;
	}
	if ( pY != NULL ) {
		*pY = g_xge.fMouseWheelY;
	}
}

int xgeMouseDown(int iButton)
{
	return (g_xge.iMouseButtons & (unsigned int)iButton) ? 1 : 0;
}

uint32_t xgeTextGet(void)
{
	return g_xge.iTextCodepoint;
}

int xgeTouchGetCount(void)
{
	return g_xge.iTouchCount;
}

int xgeTouchGet(int iIndex, xge_touch_point_t* pPoint)
{
	if ( (iIndex < 0) || (iIndex >= g_xge.iTouchCount) || (pPoint == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pPoint = g_xge.arrTouches[iIndex];
	return XGE_OK;
}

int xgeTouchFind(uint64_t iId, xge_touch_point_t* pPoint)
{
	int iIndex;

	if ( pPoint == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xgeTouchFindIndex(iId);
	if ( iIndex < 0 ) {
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	*pPoint = g_xge.arrTouches[iIndex];
	return XGE_OK;
}

int xgeGamepadConnected(int iGamepad)
{
	if ( (iGamepad < 0) || (iGamepad >= XGE_GAMEPAD_MAX) ) {
		return 0;
	}
	return g_xge.arrGamepads[iGamepad].bConnected ? 1 : 0;
}

int xgeGamepadGetState(int iGamepad, xge_gamepad_state_t* pState)
{
	if ( (iGamepad < 0) || (iGamepad >= XGE_GAMEPAD_MAX) || (pState == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pState = g_xge.arrGamepads[iGamepad];
	return XGE_OK;
}

int xgeGamepadButtonDown(int iGamepad, uint32_t iButton)
{
	if ( (iGamepad < 0) || (iGamepad >= XGE_GAMEPAD_MAX) ) {
		return 0;
	}
	return (g_xge.arrGamepads[iGamepad].iButtons & iButton) ? 1 : 0;
}

int xgeGamepadButtonPressed(int iGamepad, uint32_t iButton)
{
	if ( (iGamepad < 0) || (iGamepad >= XGE_GAMEPAD_MAX) ) {
		return 0;
	}
	return (g_xge.arrGamepads[iGamepad].iButtonsPressed & iButton) ? 1 : 0;
}

int xgeGamepadButtonReleased(int iGamepad, uint32_t iButton)
{
	if ( (iGamepad < 0) || (iGamepad >= XGE_GAMEPAD_MAX) ) {
		return 0;
	}
	return (g_xge.arrGamepads[iGamepad].iButtonsReleased & iButton) ? 1 : 0;
}

float xgeGamepadAxis(int iGamepad, int iAxis)
{
	if ( (iGamepad < 0) || (iGamepad >= XGE_GAMEPAD_MAX) || (iAxis < 0) || (iAxis >= XGE_GAMEPAD_AXIS_COUNT) ) {
		return 0.0f;
	}
	return g_xge.arrGamepads[iGamepad].arrAxes[iAxis];
}

int xgeGamepadSetConnected(int iGamepad, int bConnected)
{
	xge_event_t tEvent;
	int bWasConnected;

	if ( (iGamepad < 0) || (iGamepad >= XGE_GAMEPAD_MAX) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	bConnected = bConnected ? 1 : 0;
	bWasConnected = g_xge.arrGamepads[iGamepad].bConnected ? 1 : 0;
	if ( bWasConnected == bConnected ) {
		return XGE_OK;
	}
	memset(&g_xge.arrGamepads[iGamepad], 0, sizeof(g_xge.arrGamepads[iGamepad]));
	g_xge.arrGamepads[iGamepad].bConnected = bConnected;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = bConnected ? XGE_EVENT_GAMEPAD_CONNECTED : XGE_EVENT_GAMEPAD_DISCONNECTED;
	tEvent.iParam1 = iGamepad;
	tEvent.pData = &g_xge.arrGamepads[iGamepad];
	xgeSceneDispatchEvent(&tEvent);
	return XGE_OK;
}

int xgeGamepadSetState(int iGamepad, const xge_gamepad_state_t* pState)
{
	xge_gamepad_state_t* pDst;
	uint32_t iOldButtons;
	uint32_t iNewButtons;

	if ( (iGamepad < 0) || (iGamepad >= XGE_GAMEPAD_MAX) || (pState == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pState->bConnected == 0 ) {
		return xgeGamepadSetConnected(iGamepad, 0);
	}
	if ( g_xge.arrGamepads[iGamepad].bConnected == 0 ) {
		xgeGamepadSetConnected(iGamepad, 1);
	}
	pDst = &g_xge.arrGamepads[iGamepad];
	iOldButtons = pDst->iButtons;
	iNewButtons = pState->iButtons;
	pDst->bConnected = 1;
	pDst->iButtons = iNewButtons;
	pDst->iButtonsPressed |= (iNewButtons & ~iOldButtons);
	pDst->iButtonsReleased |= (iOldButtons & ~iNewButtons);
	memcpy(pDst->arrAxes, pState->arrAxes, sizeof(pDst->arrAxes));
	return XGE_OK;
}
