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

