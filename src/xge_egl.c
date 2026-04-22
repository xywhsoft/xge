int xgeEGLInit(xge_egl_context_t* pContext, const xge_egl_desc_t* pDesc)
{
	xge_platform_backend_t tPlatform;
	xge_graphics_backend_t tGraphics;

	if ( (pContext == NULL) || (pDesc == NULL) || (pDesc->iWidth <= 0) || (pDesc->iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pContext, 0, sizeof(*pContext));
	pContext->iWidth = pDesc->iWidth;
	pContext->iHeight = pDesc->iHeight;
	pContext->bPBuffer = pDesc->bPBuffer ? 1 : 0;
	pContext->bSurfaceless = pDesc->bSurfaceless ? 1 : 0;
	pContext->bBoardLinux = pDesc->bBoardLinux ? 1 : 0;
	pContext->pDisplay = pDesc->pNativeDisplay;
	pContext->pSurface = pDesc->pNativeWindow;
	pContext->pUser = pDesc->pUser;
	memset(&tPlatform, 0, sizeof(tPlatform));
	tPlatform.iType = XGE_PLATFORM_BACKEND_EGL;
	tPlatform.sName = pContext->bBoardLinux ? "egl-board-linux" : "egl-offscreen";
	xgePlatformBackendSet(&tPlatform);
	memset(&tGraphics, 0, sizeof(tGraphics));
	tGraphics.iType = XGE_GPU_BACKEND_GLES30;
	tGraphics.sName = "gles30";
	xgeGraphicsBackendSet(&tGraphics);
#if defined(XGE_HAS_EGL)
	pContext->bInitialized = 1;
	return XGE_OK;
#else
	return XGE_ERROR_UNSUPPORTED;
#endif
}

void xgeEGLUnit(xge_egl_context_t* pContext)
{
	if ( pContext == NULL ) {
		return;
	}
	memset(pContext, 0, sizeof(*pContext));
}

int xgeEGLMakeCurrent(xge_egl_context_t* pContext)
{
	if ( pContext == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pContext->bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	return XGE_OK;
}

int xgeOffscreenInit(xge_offscreen pOffscreen, int iWidth, int iHeight)
{
	xge_egl_desc_t tDesc;
	int iRet;

	if ( (pOffscreen == NULL) || (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pOffscreen, 0, sizeof(*pOffscreen));
	pOffscreen->iWidth = iWidth;
	pOffscreen->iHeight = iHeight;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = iWidth;
	tDesc.iHeight = iHeight;
	tDesc.bPBuffer = 1;
	iRet = xgeEGLInit(&pOffscreen->tEGL, &tDesc);
	if ( (iRet != XGE_OK) && (iRet != XGE_ERROR_UNSUPPORTED) ) {
		memset(pOffscreen, 0, sizeof(*pOffscreen));
		return iRet;
	}
	iRet = xgeRenderTargetCreate(&pOffscreen->tTarget, iWidth, iHeight);
	if ( iRet != XGE_OK ) {
		xgeEGLUnit(&pOffscreen->tEGL);
		memset(pOffscreen, 0, sizeof(*pOffscreen));
		return iRet;
	}
	pOffscreen->bActive = 1;
	return XGE_OK;
}

void xgeOffscreenUnit(xge_offscreen pOffscreen)
{
	if ( pOffscreen == NULL ) {
		return;
	}
	xgeRenderTargetFree(&pOffscreen->tTarget);
	xgeEGLUnit(&pOffscreen->tEGL);
	memset(pOffscreen, 0, sizeof(*pOffscreen));
}

xge_render_target xgeOffscreenRenderTarget(xge_offscreen pOffscreen)
{
	if ( (pOffscreen == NULL) || (pOffscreen->bActive == 0) ) {
		return NULL;
	}
	return &pOffscreen->tTarget;
}

int xgeOffscreenReadPixels(xge_offscreen pOffscreen, void* pPixels, int iStride)
{
	if ( (pOffscreen == NULL) || (pOffscreen->bActive == 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return xgeRenderTargetReadPixels(&pOffscreen->tTarget, pPixels, iStride);
}
