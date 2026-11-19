typedef struct xge_mini_bridge_state_t {
	xge_miniprogram_bridge_t raw;
	xge_miniprogram_owned_bridge_t owned;
	xge_callback_node_t* owner;
	uint64_t creatorThread;
} xge_mini_bridge_state_t;

/* Registered before or after engine Init, like the resource registry.
 * The registry lock publishes the provider and bridge together. */
static xge_mini_bridge_state_t g_xgeMiniBridge;
static bool g_xgeMiniConfiguring;

static int __xgeMiniRawLoad(const char* uri, void** data, int* size, void* user)
{
	const xge_miniprogram_bridge_t* bridge = user;
	int status = bridge->load_resource(uri, data, size, bridge->pUser);
	if ( status != XGE_OK ) {
		/* The legacy bridge retains responsibility for a failed output. */
		*data = NULL;
		*size = 0;
	} else if ( *data == NULL ) {
		status = XGE_ERROR_RESOURCE_FAILED;
	}
	return status;
}

static void __xgeMiniRawFree(void* data, void* user)
{
	const xge_miniprogram_bridge_t* bridge = user;
	if ( bridge->free_resource ) bridge->free_resource(data, bridge->pUser);
	else xrtFree(data);
}

static void __xgeMiniRawDrop(void* bridge)
{
	xrtFree(bridge);
}

static int __xgeMiniPrepareProvider(const xge_mini_bridge_state_t* bridge,
	xge_resource_registration_t** output)
{
	if ( !bridge->raw.load_resource && !bridge->owned.load_resource ) return XGE_OK;
	xge_resource_registration_t* entry = xrtCalloc(1, sizeof(*entry));
	if ( !entry ) return XGE_ERROR_OUT_OF_MEMORY;
	xrtAtomic64Init(&entry->refs, 1);
	entry->miniProgram = true;
	*output = entry;
	entry->scheme = __xgeStrDup("res");
	if ( !entry->scheme ) return XGE_ERROR_OUT_OF_MEMORY;
	if ( bridge->owned.load_resource ) {
		if ( !__xgeCallbackNodePin(bridge->owner) ) return XGE_ERROR_INVALID_STATE;
		entry->owner = bridge->owner;
		entry->read = bridge->owned.load_resource;
		entry->raw.pUser = bridge->owned.pUser;
		entry->readerThread = bridge->creatorThread;
	} else {
		xge_miniprogram_bridge_t* raw = xrtMalloc(sizeof(*raw));
		if ( !raw ) return XGE_ERROR_OUT_OF_MEMORY;
		*raw = bridge->raw;
		entry->raw.load = __xgeMiniRawLoad;
		entry->raw.free = __xgeMiniRawFree;
		entry->raw.pUser = raw;
		entry->releaseUser = __xgeMiniRawDrop;
	}
	return XGE_OK;
}

static int __xgeMiniConfigBegin(bool allowShutdown)
{
	if ( !__xgeCallbackActivityEnter(allowShutdown) ) return XGE_ERROR_INVALID_STATE;
	(void)xrtSpinLock(&g_xgeResourceSpin);
	bool busy = g_xgeMiniConfiguring;
	if ( !busy ) g_xgeMiniConfiguring = true;
	(void)xrtSpinUnlock(&g_xgeResourceSpin);
	if ( busy ) __xgeCallbackActivityLeave();
	return busy ? XGE_ERROR_BUSY : XGE_OK;
}

static int __xgeMiniConfigFinish(xge_resource_registration_t* releaseProvider,
	xge_callback_node_t* releaseOwner, int status, xerror* ambient)
{
	(void)xrtSpinLock(&g_xgeResourceSpin);
	g_xgeMiniConfiguring = false;
	(void)xrtSpinUnlock(&g_xgeResourceSpin);
	xerror* first = status != XGE_OK ? xrtTakeError() : NULL;
	__xgeCallbackOutcome(XGE_OK, &status, &first);
	__xgeCallbackOutcome(__xgeResourceRegistrationRelease(releaseProvider), &status, &first);
	__xgeCallbackOutcome(__xgeCallbackNodeRelease(releaseOwner), &status, &first);
	__xgeCallbackActivityLeave();
	xrtSetErrorTake(first);
	return __xgeResourceOperationFinish(status, ambient);
}

static int __xgeMiniPublish(xge_mini_bridge_state_t* next,
	xge_resource_registration_t** provider)
{
	xge_resource_registration_t* previous = NULL;
	(void)xrtSpinLock(&g_xgeResourceSpin);
	int status = __xgeResourceRegistryReplaceMiniLocked(*provider, &previous);
	if ( status == XGE_OK ) {
		if ( next ) {
			xge_callback_node_t* oldOwner = g_xgeMiniBridge.owner;
			g_xgeMiniBridge = *next;
			next->owner = oldOwner;
		}
		*provider = previous;
	}
	(void)xrtSpinUnlock(&g_xgeResourceSpin);
	return status;
}

static int __xgeMiniSetBridge(xge_mini_bridge_state_t* next,
	const xge_callback_owner_t* owner, bool hasCallback)
{
	int status = __xgeMiniConfigBegin(false);
	if ( status != XGE_OK ) return status;
	xerror* ambient = xrtTakeError();
	xge_resource_registration_t* provider = NULL;
	status = __xgeCallbackNodePrepare(hasCallback, false, owner, &next->owner);
	if ( status == XGE_OK && g_xge.bMiniProgramInitialized )
		status = __xgeMiniPrepareProvider(next, &provider);
	if ( status == XGE_OK ) status = __xgeMiniPublish(next, &provider);
	return __xgeMiniConfigFinish(provider, next->owner, status, ambient);
}

static int __xgeMiniProgramDimensionsValid(int iWidth, int iHeight, float fDevicePixelRatio)
{
	double fFramebufferWidth;
	double fFramebufferHeight;

	if ( (iWidth <= 0) || (iHeight <= 0) || !isfinite(fDevicePixelRatio) ||
	     (fDevicePixelRatio <= 0.0f) ) return 0;
	fFramebufferWidth = (double)iWidth * (double)fDevicePixelRatio;
	fFramebufferHeight = (double)iHeight * (double)fDevicePixelRatio;
	return isfinite(fFramebufferWidth) && isfinite(fFramebufferHeight) &&
		fFramebufferWidth >= 1.0 && fFramebufferHeight >= 1.0 &&
		fFramebufferWidth <= (double)INT_MAX && fFramebufferHeight <= (double)INT_MAX;
}

static void __xgeMiniProgramSetDimensions(int iWidth, int iHeight, float fDevicePixelRatio)
{
	int iFramebufferWidth;
	int iFramebufferHeight;

	iFramebufferWidth = (int)((double)iWidth * fDevicePixelRatio);
	iFramebufferHeight = (int)((double)iHeight * fDevicePixelRatio);
	if ( iFramebufferWidth < 1 ) iFramebufferWidth = 1;
	if ( iFramebufferHeight < 1 ) iFramebufferHeight = 1;
	g_xge.iWindowWidth = iWidth;
	g_xge.iWindowHeight = iHeight;
	g_xge.iFramebufferWidth = iFramebufferWidth;
	g_xge.iFramebufferHeight = iFramebufferHeight;
	g_xge.fDpiScale = fDevicePixelRatio;
	g_xge.iWidth = iFramebufferWidth;
	g_xge.iHeight = iFramebufferHeight;
	g_xge.tPlatformRuntime.iWindowWidth = iWidth;
	g_xge.tPlatformRuntime.iWindowHeight = iHeight;
	g_xge.tPlatformRuntime.iFramebufferWidth = iFramebufferWidth;
	g_xge.tPlatformRuntime.iFramebufferHeight = iFramebufferHeight;
	g_xge.tPlatformRuntime.fDpiScale = fDevicePixelRatio;
	g_xge.tCamera.tViewport.fW = (float)iFramebufferWidth;
	g_xge.tCamera.tViewport.fH = (float)iFramebufferHeight;
}

int xgeMiniProgramInit(const xge_miniprogram_desc_t* pDesc)
{
	xge_platform_backend_t tPlatform;
	xge_graphics_backend_t tGraphics;

	if ( pDesc == NULL || !isfinite(pDesc->fDevicePixelRatio) ) return XGE_ERROR_INVALID_ARGUMENT;
	xge_miniprogram_desc_t next = *pDesc;
	if ( next.fDevicePixelRatio <= 0.0f ) next.fDevicePixelRatio = 1.0f;
	bool sized = next.iWidth > 0 && next.iHeight > 0;
	if ( sized && !__xgeMiniProgramDimensionsValid(next.iWidth, next.iHeight, next.fDevicePixelRatio) )
		return XGE_ERROR_INVALID_ARGUMENT;

	int status = __xgeMiniConfigBegin(false);
	if ( status != XGE_OK ) return status;
	xerror* ambient = xrtTakeError();
	xge_resource_registration_t* provider = NULL;
	status = __xgeMiniPrepareProvider(&g_xgeMiniBridge, &provider);
	if ( status == XGE_OK ) status = __xgeMiniPublish(NULL, &provider);
	if ( status != XGE_OK ) return __xgeMiniConfigFinish(provider, NULL, status, ambient);

	g_xge.tMiniProgramDesc = next;
	if ( sized ) __xgeMiniProgramSetDimensions(next.iWidth, next.iHeight, next.fDevicePixelRatio);
	memset(&tPlatform, 0, sizeof(tPlatform));
	tPlatform.iType = XGE_PLATFORM_BACKEND_MINIPROGRAM;
	tPlatform.sName = "miniprogram";
	xgePlatformBackendSet(&tPlatform);
	memset(&tGraphics, 0, sizeof(tGraphics));
	tGraphics.iType = XGE_GPU_BACKEND_WEBGL2;
	tGraphics.sName = "webgl2";
	xgeGraphicsBackendSet(&tGraphics);
	g_xge.bMiniProgramFrameTimeValid = 0;
	g_xge.bMiniProgramInitialized = 1;
	return __xgeMiniConfigFinish(provider, NULL, XGE_OK, ambient);
}

int xgeMiniProgramInitSimple(int iWidth, int iHeight, float fDevicePixelRatio)
{
	xge_miniprogram_desc_t tDesc;

	if ( (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = iWidth;
	tDesc.iHeight = iHeight;
	tDesc.fDevicePixelRatio = fDevicePixelRatio;
	return xgeMiniProgramInit(&tDesc);
}

static int __xgeMiniProgramUnit(bool allowShutdown)
{
	int status = __xgeMiniConfigBegin(allowShutdown);
	if ( status != XGE_OK ) return status;
	xerror* ambient = xrtTakeError();
	xge_resource_registration_t* provider = NULL;
	xge_mini_bridge_state_t next = {0};
	status = __xgeMiniPublish(&next, &provider);
	memset(&g_xge.tMiniProgramDesc, 0, sizeof(g_xge.tMiniProgramDesc));
	g_xge.bMiniProgramInitialized = 0;
	g_xge.bMiniProgramFrameTimeValid = 0;
	return __xgeMiniConfigFinish(provider, next.owner, status, ambient);
}

int xgeMiniProgramUnitChecked(void)
{
	return __xgeMiniProgramUnit(false);
}

void xgeMiniProgramUnit(void)
{
	if ( xgeMiniProgramUnitChecked() != XGE_OK && !xrtGetError() ) xrtSetErrorKind(XERR_STATE);
}

int xgeMiniProgramSetBridge(const xge_miniprogram_bridge_t* pBridge)
{
	if ( pBridge == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	xge_mini_bridge_state_t next = {.raw = *pBridge};
	return __xgeMiniSetBridge(&next, NULL, false);
}

int xgeMiniProgramSetBridgeOwned(const xge_miniprogram_owned_bridge_t* pBridge,
	const xge_callback_owner_t* pOwner)
{
	xge_mini_bridge_state_t next = {0};
	if ( pBridge ) next.owned = *pBridge;
	bool hasCallback = next.owned.request_frame || next.owned.load_resource || next.owned.audio_command;
	if ( !__xgeCallbackOwnerValid(pOwner) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( hasCallback && (!pOwner || !pOwner->onHold) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( hasCallback ) next.creatorThread = xrtThreadCurrentId();
	return __xgeMiniSetBridge(&next, hasCallback ? pOwner : NULL, hasCallback);
}

int xgeMiniProgramFrame(double fTimeSeconds)
{
	double fDelta;
	if ( g_xge.bMiniProgramInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	if ( !isfinite(fTimeSeconds) ) return XGE_ERROR_INVALID_ARGUMENT;
	fDelta = g_xge.bMiniProgramFrameTimeValid ?
		fTimeSeconds - g_xge.fMiniProgramLastFrameTime :
		1.0 / (double)g_xge.objDesc.iTargetFPS;
	g_xge.fMiniProgramLastFrameTime = fTimeSeconds;
	g_xge.bMiniProgramFrameTimeValid = 1;
	return __xgeFrameWithDelta((float)fDelta);
}

int xgeMiniProgramResize(int iWidth, int iHeight, float fDevicePixelRatio)
{
	if ( (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !isfinite(fDevicePixelRatio) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( fDevicePixelRatio <= 0.0f ) {
		fDevicePixelRatio = 1.0f;
	}
	if ( !__xgeMiniProgramDimensionsValid(iWidth, iHeight, fDevicePixelRatio) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	g_xge.tMiniProgramDesc.iWidth = iWidth;
	g_xge.tMiniProgramDesc.iHeight = iHeight;
	g_xge.tMiniProgramDesc.fDevicePixelRatio = fDevicePixelRatio;
	__xgeMiniProgramSetDimensions(iWidth, iHeight, fDevicePixelRatio);
	return XGE_OK;
}

int xgeMiniProgramTouch(int iPhase, const xge_miniprogram_touch_t* pTouches, int iCount)
{
	xge_event_t tEvent;
	xge_touch_event_t tTouch;
	xge_touch_point_t* pPoint;
	float fDevicePixelRatio;
	float fX;
	float fY;
	int i;
	int iIndex;
	int iEventType;

	if ( (iCount < 0) || (iCount > XGE_TOUCH_MAX) || (iCount != 0 && pTouches == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	switch ( iPhase ) {
		case XGE_TOUCH_BEGIN: iEventType = XGE_EVENT_TOUCH_BEGIN; break;
		case XGE_TOUCH_MOVE: iEventType = XGE_EVENT_TOUCH_MOVE; break;
		case XGE_TOUCH_END: iEventType = XGE_EVENT_TOUCH_END; break;
		case XGE_TOUCH_CANCEL: iEventType = XGE_EVENT_TOUCH_CANCEL; break;
		default: return XGE_ERROR_INVALID_ARGUMENT;
	}
	fDevicePixelRatio = (g_xge.tMiniProgramDesc.fDevicePixelRatio > 0.0f) ?
		g_xge.tMiniProgramDesc.fDevicePixelRatio : 1.0f;
	__xgeTouchResetStationary();
	for ( i = 0; i < iCount; i++ ) {
		iIndex = __xgeTouchFindIndex(pTouches[i].iId);
		if ( iIndex < 0 ) {
			if ( g_xge.iTouchCount >= XGE_TOUCH_MAX ) {
				continue;
			}
			iIndex = g_xge.iTouchCount++;
			memset(&g_xge.arrTouches[iIndex], 0, sizeof(g_xge.arrTouches[iIndex]));
			g_xge.arrTouches[iIndex].iId = pTouches[i].iId;
		}
		pPoint = &g_xge.arrTouches[iIndex];
		fX = pTouches[i].fX * fDevicePixelRatio;
		fY = pTouches[i].fY * fDevicePixelRatio;
		pPoint->fDX = fX - pPoint->fX;
		pPoint->fDY = fY - pPoint->fY;
		pPoint->fX = fX;
		pPoint->fY = fY;
		pPoint->iPhase = iPhase;
		pPoint->bChanged = 1;
		pPoint->bDown = ((iPhase == XGE_TOUCH_END) || (iPhase == XGE_TOUCH_CANCEL)) ? 0 : 1;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	memset(&tTouch, 0, sizeof(tTouch));
	tEvent.iType = iEventType;
	tTouch.iPhase = iPhase;
	tTouch.iCount = g_xge.iTouchCount;
	if ( tTouch.iCount > XGE_TOUCH_MAX ) {
		tTouch.iCount = XGE_TOUCH_MAX;
	}
	for ( i = 0; i < tTouch.iCount; i++ ) {
		tTouch.arrPoints[i] = g_xge.arrTouches[i];
	}
	tEvent.pData = &tTouch;
	for ( i = 0; i < tTouch.iCount; i++ ) {
		if ( tTouch.arrPoints[i].bChanged != 0 ) {
			tEvent.iPointerId = tTouch.arrPoints[i].iId;
			tEvent.fX = tTouch.arrPoints[i].fX;
			tEvent.fY = tTouch.arrPoints[i].fY;
			tEvent.fDX = tTouch.arrPoints[i].fDX;
			tEvent.fDY = tTouch.arrPoints[i].fDY;
			break;
		}
	}
	if ( (i >= tTouch.iCount) && (tTouch.iCount > 0) ) {
		tEvent.iPointerId = tTouch.arrPoints[0].iId;
		tEvent.fX = tTouch.arrPoints[0].fX;
		tEvent.fY = tTouch.arrPoints[0].fY;
		tEvent.fDX = tTouch.arrPoints[0].fDX;
		tEvent.fDY = tTouch.arrPoints[0].fDY;
	}
	for ( i = 0; i < tTouch.iCount; i++ ) {
		if ( tTouch.arrPoints[i].bChanged == 0 ) continue;
		__xgeInputQueuePointerEvent(tEvent.iType, tTouch.arrPoints[i].iId,
			XGE_MOUSE_LEFT, tTouch.arrPoints[i].bDown ? XGE_MOUSE_LEFT : 0u,
			0u, tTouch.arrPoints[i].fX, tTouch.arrPoints[i].fY,
			tTouch.arrPoints[i].fDX, tTouch.arrPoints[i].fDY);
	}
	xgeSceneDispatchEvent(&tEvent);
	return XGE_OK;
}

int xgeMiniProgramTouchOne(int iPhase, int iId, float fX, float fY, float fForce)
{
	xge_miniprogram_touch_t tTouch;

	if ( iId < 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tTouch, 0, sizeof(tTouch));
	tTouch.iId = (uint64_t)iId;
	tTouch.fX = fX;
	tTouch.fY = fY;
	tTouch.fForce = fForce;
	return xgeMiniProgramTouch(iPhase, &tTouch, 1);
}

int xgeMiniProgramText(uint32_t iCodepoint)
{
	xge_event_t tEvent;
	xge_input_event_t tInput;

	if ( iCodepoint == 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeTextPush(iCodepoint);
	memset(&tInput, 0, sizeof(tInput));
	tInput.iSize = sizeof(tInput);
	tInput.iType = XGE_EVENT_TEXT;
	tInput.iCodepoint = iCodepoint;
	if ( !__xgeInputEventQueuePush(&tInput) ) return XGE_ERROR_OUT_OF_MEMORY;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_TEXT;
	tEvent.iCodepoint = iCodepoint;
	xgeSceneDispatchEvent(&tEvent);
	return XGE_OK;
}

static int __xgeMiniDispatchBegin(xge_mini_bridge_state_t* bridge, xge_callback_snapshot_t* snapshot)
{
	(void)xrtSpinLock(&g_xgeResourceSpin);
	*bridge = g_xgeMiniBridge;
	int status = bridge->creatorThread && bridge->creatorThread != xrtThreadCurrentId() ? XGE_ERROR_BUSY : XGE_OK;
	if ( status == XGE_OK && !__xgeCallbackNodePin(bridge->owner) ) status = XGE_ERROR_INVALID_STATE;
	(void)xrtSpinUnlock(&g_xgeResourceSpin);
	if ( status == XGE_OK ) status = __xgeCallbackSnapshotBegin(bridge->owner, false, snapshot);
	return status;
}

static int __xgeMiniDispatchFinish(xge_callback_snapshot_t* snapshot, int status, int value, xerror* ambient)
{
	xerror* first = status != XGE_OK ? xrtTakeError() : NULL;
	__xgeCallbackOutcome(XGE_OK, &status, &first);
	__xgeCallbackOutcome(__xgeCallbackSnapshotEnd(snapshot), &status, &first);
	__xgeCallbackActivityLeave();
	xrtSetErrorTake(first);
	status = __xgeResourceOperationFinish(status, ambient);
	return status == XGE_OK ? value : status;
}

int xgeMiniProgramRequestFrame(void)
{
	if ( !__xgeCallbackActivityEnter(false) ) return XGE_ERROR_INVALID_STATE;
	xerror* ambient = xrtTakeError();
	xge_mini_bridge_state_t bridge;
	xge_callback_snapshot_t snapshot = {0};
	int status = __xgeMiniDispatchBegin(&bridge, &snapshot);
	int value = 0;
	if ( status == XGE_OK ) {
		if ( bridge.owned.request_frame ) value = bridge.owned.request_frame(bridge.owned.pUser);
		else if ( bridge.raw.request_frame ) value = bridge.raw.request_frame(bridge.raw.pUser);
		else status = XGE_ERROR_UNSUPPORTED;
		if ( value < 0 ) status = value;
	}
	return __xgeMiniDispatchFinish(&snapshot, status, value, ambient);
}

int xgeMiniProgramAudioCommand(int iCommand, int iHandle, const void* pData, int iSize)
{
	if ( iSize < 0 || (iSize > 0 && pData == NULL) ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( !__xgeCallbackActivityEnter(false) ) return XGE_ERROR_INVALID_STATE;
	xerror* ambient = xrtTakeError();
	xge_mini_bridge_state_t bridge;
	xge_callback_snapshot_t snapshot = {0};
	int status = __xgeMiniDispatchBegin(&bridge, &snapshot);
	int value = 0;
	if ( status == XGE_OK ) {
		if ( bridge.owned.audio_command ) {
			xbytesview data = {pData, (size_t)iSize};
			value = bridge.owned.audio_command(iCommand, iHandle, &data, bridge.owned.pUser);
		} else if ( bridge.raw.audio_command ) {
			value = bridge.raw.audio_command(iCommand, iHandle, pData, iSize, bridge.raw.pUser);
		} else status = XGE_ERROR_UNSUPPORTED;
		if ( value < 0 ) status = value;
	}
	return __xgeMiniDispatchFinish(&snapshot, status, value, ambient);
}
