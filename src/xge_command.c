static int __xgeRenderCommandAtomicCompareExchange(volatile int* pValue, int iExpected, int iValue)
{
	#if defined(_MSC_VER)
		return (int)_InterlockedCompareExchange((volatile long*)pValue, (long)iValue, (long)iExpected);
	#elif defined(__GNUC__) || defined(__clang__)
		return __sync_val_compare_and_swap(pValue, iExpected, iValue);
	#else
		int iOld;

		iOld = *pValue;
		if ( iOld == iExpected ) {
			*pValue = iValue;
		}
		return iOld;
	#endif
}

static void __xgeRenderCommandLock(void)
{
	while ( __xgeRenderCommandAtomicCompareExchange(&g_xge.iRenderCommandLock, 0, 1) != 0 ) {
	}
}

static void __xgeRenderCommandUnlock(void)
{
	#if defined(_MSC_VER)
		_InterlockedExchange((volatile long*)&g_xge.iRenderCommandLock, 0);
	#elif defined(__GNUC__) || defined(__clang__)
		__sync_lock_release(&g_xge.iRenderCommandLock);
	#else
		g_xge.iRenderCommandLock = 0;
	#endif
}

static void __xgeRenderCommandRelease(xge_render_command_t* pCommand)
{
	if ( pCommand == NULL ) {
		return;
	}
	switch ( pCommand->iType ) {
		case XGE_RENDER_COMMAND_DRAW:
			if ( pCommand->u.tDraw.pTexture != NULL ) {
				xgeTextureFree(pCommand->u.tDraw.pTexture);
				pCommand->u.tDraw.pTexture = NULL;
			}
			break;

		default:
			break;
	}
	memset(pCommand, 0, sizeof(*pCommand));
}

static void __xgeRenderCommandUnit(void)
{
	xge_render_command_t* pCommands;
	int iCount;
	int i;

	__xgeRenderCommandLock();
	pCommands = g_xge.pRenderCommands;
	iCount = g_xge.iRenderCommandCount;
	g_xge.pRenderCommands = NULL;
	g_xge.iRenderCommandCount = 0;
	g_xge.iRenderCommandCapacity = 0;
	__xgeRenderCommandUnlock();

	for ( i = 0; i < iCount; i++ ) {
		__xgeRenderCommandRelease(&pCommands[i]);
	}
	if ( pCommands != NULL ) {
		xrtFree(pCommands);
	}
}

static void __xgeRenderCommandReset(void)
{
	__xgeRenderCommandUnit();
	g_xge.bRenderActive = 0;
}

static int __xgeRenderCommandReserve(int iNeeded)
{
	xge_render_command_t* pCommands;
	int iCapacity;

	if ( iNeeded <= g_xge.iRenderCommandCapacity ) {
		return XGE_OK;
	}
	iCapacity = (g_xge.iRenderCommandCapacity > 0) ? g_xge.iRenderCommandCapacity : 64;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	pCommands = (xge_render_command_t*)xrtRealloc(g_xge.pRenderCommands, sizeof(*pCommands) * (size_t)iCapacity);
	if ( pCommands == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	g_xge.pRenderCommands = pCommands;
	g_xge.iRenderCommandCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeRenderCommandPush(const xge_render_command_t* pCommand)
{
	int iRet;

	if ( pCommand == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeRenderCommandReserve(g_xge.iRenderCommandCount + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	g_xge.pRenderCommands[g_xge.iRenderCommandCount] = *pCommand;
	g_xge.iRenderCommandCount++;
	return XGE_OK;
}

static int __xgeRenderCommandDraw(const xge_draw_t* pDraw)
{
	xge_render_command_t tCommand;
	int iRet;

	if ( (pDraw == NULL) || (pDraw->pTexture == NULL) || (pDraw->pTexture->iRefCount <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tCommand, 0, sizeof(tCommand));
	tCommand.iType = XGE_RENDER_COMMAND_DRAW;
	tCommand.u.tDraw = *pDraw;

	__xgeRenderCommandLock();
	iRet = xgeTextureAddRef(tCommand.u.tDraw.pTexture);
	if ( iRet <= 0 ) {
		__xgeRenderCommandUnlock();
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeRenderCommandPush(&tCommand);
	if ( iRet != XGE_OK ) {
		xgeTextureFree(tCommand.u.tDraw.pTexture);
	}
	__xgeRenderCommandUnlock();
	return iRet;
}

static int __xgeRenderCommandFlush(void)
{
	xge_render_command_t* pCommands;
	int iCount;
	int i;

	__xgeRenderCommandLock();
	pCommands = g_xge.pRenderCommands;
	iCount = g_xge.iRenderCommandCount;
	g_xge.pRenderCommands = NULL;
	g_xge.iRenderCommandCount = 0;
	g_xge.iRenderCommandCapacity = 0;
	__xgeRenderCommandUnlock();

	for ( i = 0; i < iCount; i++ ) {
		switch ( pCommands[i].iType ) {
			case XGE_RENDER_COMMAND_DRAW:
				__xgeDrawExImmediate(&pCommands[i].u.tDraw);
				break;

			default:
				break;
		}
		__xgeRenderCommandRelease(&pCommands[i]);
	}
	if ( pCommands != NULL ) {
		xrtFree(pCommands);
	}
	return XGE_OK;
}
