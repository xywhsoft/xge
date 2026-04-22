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

static int __xgeAsyncFinish(xge_async_request pRequest, int iResult)
{
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
	if ( pRequest->onComplete != NULL ) {
		pRequest->onComplete(pRequest, pRequest->pUser);
	}
	return pRequest->iResult;
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
	if ( pRequest->sURI != NULL ) {
		xrtFree(pRequest->sURI);
	}
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

int xgeAsyncImageLoad(xge_async_request pRequest, xge_image pImage, const char* sPath, uint32_t iFlags, xge_async_proc onComplete, void* pUser)
{
	int iRet;

	iRet = __xgeAsyncBegin(pRequest, XGE_ASYNC_IMAGE, pImage, sPath, onComplete, pUser);
	if ( iRet != XGE_OK ) {
		return iRet;
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
	iRet = xgeSoundLoad(pSound, sPath);
	return __xgeAsyncFinish(pRequest, iRet);
}
