typedef struct __xge_async_payload_t {
	xge_image_t tImage;
	int iResult;
} __xge_async_payload_t;

static void __xgeAsyncPayloadFree(__xge_async_payload_t* pPayload)
{
	if ( pPayload == NULL ) return;
	xgeImageFree(&pPayload->tImage);
	xrtFree(pPayload);
}

static int __xgeAsyncBegin(xge_async_request pRequest, int iType, void* pTarget, const char* sPath, xge_async_proc onComplete, void* pUser)
{
	if ( (pRequest == NULL) || (pTarget == NULL) || (sPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeAsyncRequestFree(pRequest);
	memset(pRequest, 0, sizeof(*pRequest));
	pRequest->sURI = __xgeStrDup(sPath);
	if ( pRequest->sURI == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pRequest->iType = iType;
	pRequest->iStatus = XGE_ASYNC_LOADING;
	pRequest->iResult = XGE_OK;
	pRequest->fProgress = 0.0f;
	pRequest->pTarget = pTarget;
	pRequest->onComplete = onComplete;
	pRequest->pUser = pUser;
	return XGE_OK;
}

static int __xgeAsyncFinishEx(xge_async_request pRequest, int iResult, int bInvokeCallback)
{
	int iFinalResult;

	pRequest->iResult = iResult;
	pRequest->fProgress = 1.0f;
	if ( pRequest->bCancel != 0 ) {
		pRequest->iStatus = XGE_ASYNC_CANCELLED;
		pRequest->iResult = XGE_ERROR;
	} else if ( iResult == XGE_OK ) {
		pRequest->iStatus = XGE_ASYNC_READY;
	} else {
		pRequest->iStatus = XGE_ASYNC_FAILED;
	}
	/* Completion callbacks may release the request. Do not inspect it after this
	 * point; every public caller receives the result captured here. */
	iFinalResult = pRequest->iResult;
	if ( bInvokeCallback && (pRequest->onComplete != NULL) ) {
		pRequest->onComplete(pRequest, pRequest->pUser);
	} else if ( !bInvokeCallback ) {
		pRequest->bCallbackPending = 1;
	}
	return iFinalResult;
}

static int __xgeAsyncFinish(xge_async_request pRequest, int iResult)
{
	return __xgeAsyncFinishEx(pRequest, iResult, 1);
}

static uint32 __xgeAsyncThreadProc(ptr pParam)
{
	xge_async_request pRequest;
	__xge_async_payload_t* pPayload;
	const char* sPath;
	void* pData;
	size_t iSize;

	pRequest = (xge_async_request)pParam;
	if ( (pRequest == NULL) || (pRequest->pPayload == NULL) ) {
		return 1;
	}
	pPayload = (__xge_async_payload_t*)pRequest->pPayload;
	pPayload->iResult = XGE_ERROR_INVALID_ARGUMENT;
	sPath = pRequest->sURI;
	if ( strncmp(sPath, "file://", 7) == 0 ) sPath += 7;
	else if ( strncmp(sPath, "res://", 6) == 0 ) sPath += 6;
	pData = __xgeFileGetAll(sPath, &iSize);
	if ( pData == NULL ) {
		pPayload->iResult = XGE_ERROR_FILE_NOT_FOUND;
	} else if ( iSize > (size_t)INT32_MAX ) {
		xrtFree(pData);
		pPayload->iResult = XGE_ERROR_RESOURCE_FAILED;
	} else {
		pPayload->iResult = xgeImageLoadMemoryEx(&pPayload->tImage, pData, (int)iSize, pRequest->iFlags);
		xrtFree(pData);
	}
	/* Only this private payload is touched by the worker. Public request state,
	 * target objects and callbacks are committed by xgeAsyncPoll on the caller thread. */
	return (uint32)(pPayload->iResult == XGE_OK ? 0 : 1);
}

static int __xgeAsyncStartThread(xge_async_request pRequest)
{
	xthread* pThread;
	__xge_async_payload_t* pPayload;
	int i;

	if ( (pRequest->iType != XGE_ASYNC_IMAGE) && (pRequest->iType != XGE_ASYNC_TEXTURE) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	/* A registered resource provider owns its own threading contract. Do not
	 * invoke arbitrary provider callbacks from XGE's worker. */
	for ( i = 0; i < g_xge.iResourceProviderCount; i++ ) {
		const char* sScheme = g_xge.arrResourceProviders[i].sScheme;
		if ( (sScheme != NULL) &&
			 ((strncmp(pRequest->sURI, sScheme, strlen(sScheme)) == 0) &&
			  (strncmp(pRequest->sURI + strlen(sScheme), "://", 3) == 0)) ) {
			return XGE_ERROR_UNSUPPORTED;
		}
	}
	if ( (strstr(pRequest->sURI, "://") != NULL) &&
		 (strncmp(pRequest->sURI, "file://", 7) != 0) &&
		 (strncmp(pRequest->sURI, "res://", 6) != 0) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	pPayload = (__xge_async_payload_t*)xrtCalloc(1, sizeof(*pPayload));
	if ( pPayload == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	pRequest->pPayload = pPayload;

	pThread = xrtThreadCreate((ptr)__xgeAsyncThreadProc, pRequest, 0);
	if ( pThread == NULL ) {
		__xgeAsyncPayloadFree(pPayload);
		pRequest->pPayload = NULL;
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pRequest->pThread = pThread;
	pRequest->bThreaded = 1;
	return XGE_OK;
}

void xgeAsyncRequestInit(xge_async_request pRequest)
{
	if ( pRequest == NULL ) {
		return;
	}
	memset(pRequest, 0, sizeof(*pRequest));
	pRequest->iStatus = XGE_ASYNC_PENDING;
}

void xgeAsyncRequestFree(xge_async_request pRequest)
{
	if ( pRequest == NULL ) {
		return;
	}
	if ( pRequest->pThread != NULL ) {
		xrtThreadWait((xthread*)pRequest->pThread);
		xrtThreadDestroy((xthread*)pRequest->pThread);
		pRequest->pThread = NULL;
	}
	if ( pRequest->sURI != NULL ) {
		xrtFree(pRequest->sURI);
	}
	__xgeAsyncPayloadFree((__xge_async_payload_t*)pRequest->pPayload);
	memset(pRequest, 0, sizeof(*pRequest));
	pRequest->iStatus = XGE_ASYNC_PENDING;
}

int xgeAsyncRequestCancel(xge_async_request pRequest)
{
	if ( pRequest == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pRequest->iStatus == XGE_ASYNC_READY) || (pRequest->iStatus == XGE_ASYNC_FAILED) || (pRequest->iStatus == XGE_ASYNC_CANCELLED) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	pRequest->bCancel = 1;
	pRequest->iStatus = XGE_ASYNC_CANCELLED;
	pRequest->iResult = XGE_ERROR;
	return XGE_OK;
}

int xgeAsyncThreadingSet(int bEnabled)
{
	g_xge.bAsyncThreadingEnabled = bEnabled ? 1 : 0;
	return XGE_OK;
}

int xgeAsyncThreadingGet(void)
{
	return g_xge.bAsyncThreadingEnabled;
}

int xgeAsyncPoll(xge_async_request pRequest)
{
	if ( pRequest == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pRequest->pThread != NULL ) {
		if ( xrtThreadWaitFor((xthread*)pRequest->pThread, 0) == XWAIT_TIMEOUT ) {
			return XGE_ASYNC_LOADING;
		}
		xrtThreadDestroy((xthread*)pRequest->pThread);
		pRequest->pThread = NULL;
	}
	if ( pRequest->bThreaded != 0 ) {
		__xge_async_payload_t* pPayload = (__xge_async_payload_t*)pRequest->pPayload;
		int iResult = (pPayload != NULL) ? pPayload->iResult : XGE_ERROR;
		int iStatus;
		if ( (pRequest->bCancel == 0) && (iResult == XGE_OK) && (pPayload != NULL) ) {
			if ( pRequest->iType == XGE_ASYNC_IMAGE ) {
				*(xge_image)pRequest->pTarget = pPayload->tImage;
				memset(&pPayload->tImage, 0, sizeof(pPayload->tImage));
			} else if ( pRequest->iType == XGE_ASYNC_TEXTURE ) {
				iResult = xgeTextureCreateFromImage((xge_texture)pRequest->pTarget, &pPayload->tImage);
			}
		}
		__xgeAsyncPayloadFree(pPayload);
		pRequest->pPayload = NULL;
		pRequest->bThreaded = 0;
		iStatus = (pRequest->bCancel != 0) ? XGE_ASYNC_CANCELLED :
			((iResult == XGE_OK) ? XGE_ASYNC_READY : XGE_ASYNC_FAILED);
		(void)__xgeAsyncFinish(pRequest, iResult);
		return iStatus;
	}
	return pRequest->iStatus;
}

int xgeAsyncImageLoad(xge_async_request pRequest, xge_image pImage, const char* sPath, uint32_t iFlags, xge_async_proc onComplete, void* pUser)
{
	int iRet;

	iRet = __xgeAsyncBegin(pRequest, XGE_ASYNC_IMAGE, pImage, sPath, onComplete, pUser);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pRequest->iFlags = iFlags;
	if ( g_xge.bAsyncThreadingEnabled != 0 ) {
		iRet = __xgeAsyncStartThread(pRequest);
		if ( iRet == XGE_OK ) {
			return XGE_OK;
		}
	}
	iRet = xgeImageLoadEx(pImage, sPath, iFlags);
	return __xgeAsyncFinish(pRequest, iRet);
}

int xgeAsyncTextureLoad(xge_async_request pRequest, xge_texture pTexture, const char* sPath, uint32_t iFlags, xge_async_proc onComplete, void* pUser)
{
	int iRet;

	iRet = __xgeAsyncBegin(pRequest, XGE_ASYNC_TEXTURE, pTexture, sPath, onComplete, pUser);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pRequest->iFlags = iFlags;
	if ( g_xge.bAsyncThreadingEnabled != 0 ) {
		iRet = __xgeAsyncStartThread(pRequest);
		if ( iRet == XGE_OK ) {
			return XGE_OK;
		}
	}
	iRet = xgeTextureLoadEx(pTexture, sPath, iFlags);
	return __xgeAsyncFinish(pRequest, iRet);
}

int xgeAsyncFontLoad(xge_async_request pRequest, xge_font pFont, const char* sPath, float fSize, xge_async_proc onComplete, void* pUser)
{
	int iRet;

	iRet = __xgeAsyncBegin(pRequest, XGE_ASYNC_FONT, pFont, sPath, onComplete, pUser);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pRequest->fSize = fSize;
	if ( g_xge.bAsyncThreadingEnabled != 0 ) {
		iRet = __xgeAsyncStartThread(pRequest);
		if ( iRet == XGE_OK ) {
			return XGE_OK;
		}
	}
	iRet = xgeFontLoad(pFont, sPath, fSize);
	return __xgeAsyncFinish(pRequest, iRet);
}

int xgeAsyncSoundLoad(xge_async_request pRequest, xge_sound pSound, const char* sPath, xge_async_proc onComplete, void* pUser)
{
	int iRet;

	iRet = __xgeAsyncBegin(pRequest, XGE_ASYNC_SOUND, pSound, sPath, onComplete, pUser);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( g_xge.bAsyncThreadingEnabled != 0 ) {
		iRet = __xgeAsyncStartThread(pRequest);
		if ( iRet == XGE_OK ) {
			return XGE_OK;
		}
	}
	iRet = xgeSoundLoad(pSound, sPath);
	return __xgeAsyncFinish(pRequest, iRet);
}
