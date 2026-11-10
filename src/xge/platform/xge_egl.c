#if defined(XGE_HAS_EGL)
	#include <EGL/egl.h>
	#ifndef EGL_OPENGL_ES3_BIT
		#define EGL_OPENGL_ES3_BIT 0x00000040
	#endif
#endif

static void __xgeEGLCapsNameCopy(char* sDst, int iDstSize, const char* sSrc)
{
	if ( (sDst == NULL) || (iDstSize <= 0) ) {
		return;
	}
	if ( sSrc == NULL ) {
		sSrc = "";
	}
	snprintf(sDst, (size_t)iDstSize, "%s", sSrc);
}

static void __xgeEGLStageCopy(char* sDst, int iDstSize, const char* sStage)
{
	__xgeEGLCapsNameCopy(sDst, iDstSize, sStage);
}

int xgeEGLCapsGet(xge_egl_caps_t* pCaps)
{
	if ( pCaps == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pCaps, 0, sizeof(*pCaps));
#if defined(XGE_HAS_EGL)
	pCaps->bCompiled = 1;
#if defined(XGE_HAS_EGL_PBUFFER) || defined(XGE_HAS_EGL_SURFACELESS)
	pCaps->bOffscreen = 1;
#endif
#if defined(XGE_HAS_EGL_PBUFFER)
	pCaps->bPBuffer = 1;
#endif
#if defined(XGE_HAS_EGL_NATIVE_WINDOW)
	pCaps->bNativeWindow = 1;
#endif
#if defined(XGE_HAS_EGL_BOARD_LINUX)
	pCaps->bBoardLinux = 1;
#endif
#if defined(XGE_HAS_EGL_SURFACELESS)
	pCaps->bSurfaceless = 1;
#endif
	__xgeEGLCapsNameCopy(pCaps->sBackendName, (int)sizeof(pCaps->sBackendName), "egl");
	__xgeEGLStageCopy(pCaps->sLastStage, (int)sizeof(pCaps->sLastStage), "caps");
#else
	__xgeEGLCapsNameCopy(pCaps->sBackendName, (int)sizeof(pCaps->sBackendName), "egl-unavailable");
	__xgeEGLStageCopy(pCaps->sLastStage, (int)sizeof(pCaps->sLastStage), "not-compiled");
#endif
	return XGE_OK;
}

#if defined(XGE_HAS_EGL)
static int __xgeEGLFail(xge_egl_context_t* pContext, const char* sStage, int iResult)
{
	EGLint iError;
	char arrError[160];

	iError = eglGetError();
	if ( pContext != NULL ) {
		pContext->iLastError = (int)iError;
		__xgeEGLStageCopy(pContext->sLastStage, (int)sizeof(pContext->sLastStage), sStage);
	}
	snprintf(arrError, sizeof(arrError), "EGL failed at %s: 0x%04X", (sStage != NULL) ? sStage : "unknown", (unsigned int)iError);
	__xgeLogError("backend", arrError);
	__xgeLogFormat(XGE_LOG_ERROR, "egl", "%s", arrError);
	return iResult;
}

static int __xgeEGLHasExtension(EGLDisplay hDisplay, const char* sName)
{
	const char* sExtensions;
	const char* sMatch;
	size_t iNameLen;

	if ( sName == NULL ) {
		return 0;
	}
	sExtensions = eglQueryString(hDisplay, EGL_EXTENSIONS);
	if ( sExtensions == NULL ) {
		return 0;
	}
	iNameLen = strlen(sName);
	sMatch = sExtensions;
	while ( (sMatch = strstr(sMatch, sName)) != NULL ) {
		if ( ((sMatch == sExtensions) || (sMatch[-1] == ' ')) && ((sMatch[iNameLen] == 0) || (sMatch[iNameLen] == ' ')) ) {
			return 1;
		}
		sMatch += iNameLen;
	}
	return 0;
}

static int __xgeEGLRollback(xge_egl_context_t* context,const char* stage,int status)
{
    (void)__xgeEGLFail(context,stage,status);
    int error=context->iLastError;
    char firstStage[sizeof(context->sLastStage)];
    memcpy(firstStage,context->sLastStage,sizeof(firstStage));
    /* Preserve the startup error when rollback completes. A failed rollback
     * instead records its failing cleanup stage and leaves explicit duty. */
    if(xgeEGLUnitChecked(context)==XGE_OK){
        context->iLastError=error;
        memcpy(context->sLastStage,firstStage,sizeof(firstStage));
    }
    return status;
}

static int __xgeEGLCreateContext(xge_egl_context_t* pContext, const xge_egl_desc_t* pDesc)
{
	EGLDisplay hDisplay;
	EGLConfig hConfig;
	EGLSurface hSurface;
	EGLContext hContext;
	EGLint arrConfigAttribs[32];
	EGLint arrContextAttribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 3,
		EGL_NONE
	};
	EGLint arrPBufferAttribs[] = {
		EGL_WIDTH, 1,
		EGL_HEIGHT, 1,
		EGL_NONE
	};
	EGLint iConfigCount;
	EGLint iSurfaceBits;
	int i;

	hDisplay = eglGetDisplay((pDesc->pNativeDisplay != NULL) ? (EGLNativeDisplayType)pDesc->pNativeDisplay : EGL_DEFAULT_DISPLAY);
	if ( hDisplay == EGL_NO_DISPLAY ) {
		return __xgeEGLFail(pContext, "eglGetDisplay", XGE_ERROR_BACKEND_FAILED);
	}
	if ( eglInitialize(hDisplay, NULL, NULL) == EGL_FALSE ) {
		return __xgeEGLFail(pContext, "eglInitialize", XGE_ERROR_BACKEND_FAILED);
	}
	pContext->pDisplay=(void*)hDisplay;
	pContext->bCleanupPending=1;
	if ( eglBindAPI(EGL_OPENGL_ES_API) == EGL_FALSE ) {
		return __xgeEGLRollback(pContext,"eglBindAPI",XGE_ERROR_BACKEND_FAILED);
	}
	iSurfaceBits = 0;
	if ( pDesc->bPBuffer != 0 ) {
		iSurfaceBits |= EGL_PBUFFER_BIT;
	}
	if ( pDesc->pNativeWindow != NULL ) {
		iSurfaceBits |= EGL_WINDOW_BIT;
	}
	if ( iSurfaceBits == 0 ) {
		iSurfaceBits = EGL_PBUFFER_BIT;
	}
	i = 0;
	arrConfigAttribs[i++] = EGL_SURFACE_TYPE;
	arrConfigAttribs[i++] = iSurfaceBits;
	arrConfigAttribs[i++] = EGL_RENDERABLE_TYPE;
	arrConfigAttribs[i++] = EGL_OPENGL_ES3_BIT;
	arrConfigAttribs[i++] = EGL_RED_SIZE;
	arrConfigAttribs[i++] = 8;
	arrConfigAttribs[i++] = EGL_GREEN_SIZE;
	arrConfigAttribs[i++] = 8;
	arrConfigAttribs[i++] = EGL_BLUE_SIZE;
	arrConfigAttribs[i++] = 8;
	arrConfigAttribs[i++] = EGL_ALPHA_SIZE;
	arrConfigAttribs[i++] = 8;
	arrConfigAttribs[i++] = EGL_DEPTH_SIZE;
	arrConfigAttribs[i++] = 16;
	arrConfigAttribs[i++] = EGL_STENCIL_SIZE;
	arrConfigAttribs[i++] = 0;
	arrConfigAttribs[i++] = EGL_NONE;
	if ( eglChooseConfig(hDisplay, arrConfigAttribs, &hConfig, 1, &iConfigCount) == EGL_FALSE || iConfigCount <= 0 ) {
		return __xgeEGLRollback(pContext,"eglChooseConfig",XGE_ERROR_BACKEND_FAILED);
	}
	pContext->pConfig=(void*)hConfig;
	hSurface = EGL_NO_SURFACE;
	if ( pDesc->pNativeWindow != NULL ) {
		hSurface = eglCreateWindowSurface(hDisplay, hConfig, (EGLNativeWindowType)pDesc->pNativeWindow, NULL);
	} else if ( pDesc->bSurfaceless != 0 ) {
		if ( __xgeEGLHasExtension(hDisplay, "EGL_KHR_surfaceless_context") == 0 ) {
			return __xgeEGLRollback(pContext,"EGL_KHR_surfaceless_context",XGE_ERROR_UNSUPPORTED);
		}
	} else {
		arrPBufferAttribs[1] = pDesc->iWidth;
		arrPBufferAttribs[3] = pDesc->iHeight;
		hSurface = eglCreatePbufferSurface(hDisplay, hConfig, arrPBufferAttribs);
	}
	if ( (pDesc->bSurfaceless == 0) && (hSurface == EGL_NO_SURFACE) ) {
		return __xgeEGLRollback(pContext,"eglCreateSurface",XGE_ERROR_BACKEND_FAILED);
	}
	pContext->pSurface=(void*)hSurface;
	hContext = eglCreateContext(hDisplay, hConfig, EGL_NO_CONTEXT, arrContextAttribs);
	if ( hContext == EGL_NO_CONTEXT ) {
		return __xgeEGLRollback(pContext,"eglCreateContext",XGE_ERROR_BACKEND_FAILED);
	}
	pContext->pContext=(void*)hContext;
	if ( eglMakeCurrent(hDisplay, hSurface, hSurface, hContext) == EGL_FALSE ) {
		return __xgeEGLRollback(pContext,"eglMakeCurrent",XGE_ERROR_BACKEND_FAILED);
	}
	pContext->pDisplay = (void*)hDisplay;
	pContext->pConfig = (void*)hConfig;
	pContext->pSurface = (void*)hSurface;
	pContext->pContext = (void*)hContext;
	pContext->bInitialized = 1;
	pContext->iLastError = 0;
	__xgeEGLStageCopy(pContext->sLastStage, (int)sizeof(pContext->sLastStage), "ready");
	return XGE_OK;
}
#endif

int xgeEGLInit(xge_egl_context_t* pContext, const xge_egl_desc_t* pDesc)
{
	xge_egl_caps_t tCaps;

	if ( (pContext == NULL) || (pDesc == NULL) || (pDesc->iWidth <= 0) || (pDesc->iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pContext, 0, sizeof(*pContext));
	pContext->iWidth = pDesc->iWidth;
	pContext->iHeight = pDesc->iHeight;
	pContext->bPBuffer = pDesc->bPBuffer ? 1 : 0;
	pContext->bSurfaceless = pDesc->bSurfaceless ? 1 : 0;
	pContext->bBoardLinux = pDesc->bBoardLinux ? 1 : 0;
	pContext->pUser = pDesc->pUser;
	memset(&tCaps, 0, sizeof(tCaps));
	(void)xgeEGLCapsGet(&tCaps);
	if ( (pDesc->bSurfaceless != 0) && (tCaps.bSurfaceless == 0) ) {
		__xgeEGLStageCopy(pContext->sLastStage, (int)sizeof(pContext->sLastStage), "surfaceless-not-compiled");
		return XGE_ERROR_UNSUPPORTED;
	}
	if ( (pDesc->bPBuffer != 0) && (tCaps.bPBuffer == 0) ) {
		__xgeEGLStageCopy(pContext->sLastStage, (int)sizeof(pContext->sLastStage), "pbuffer-not-compiled");
		return XGE_ERROR_UNSUPPORTED;
	}
	if ( (pDesc->pNativeWindow != NULL) && (tCaps.bNativeWindow == 0) ) {
		__xgeEGLStageCopy(pContext->sLastStage, (int)sizeof(pContext->sLastStage), "native-window-not-compiled");
		return XGE_ERROR_UNSUPPORTED;
	}
	if ( (pDesc->bPBuffer == 0) && (pDesc->bSurfaceless == 0) && (pDesc->pNativeWindow == NULL) ) {
		__xgeEGLStageCopy(pContext->sLastStage, (int)sizeof(pContext->sLastStage), "no-surface");
		return XGE_ERROR_UNSUPPORTED;
	}
#if defined(XGE_HAS_EGL)
	int result = __xgeEGLCreateContext(pContext, pDesc);
	if ( result != XGE_OK ) return result;
	/* Backend selection commits only with a real initialized context. Failed
	 * capability/native attempts must not change an existing host's tables. */
	xge_platform_backend_t tPlatform = {0};
	tPlatform.iType = XGE_PLATFORM_BACKEND_EGL;
	tPlatform.sName = pContext->bBoardLinux ? "egl-board-linux" : "egl-offscreen";
	xgePlatformBackendSet(&tPlatform);
	xge_graphics_backend_t tGraphics = {0};
	tGraphics.iType = XGE_GPU_BACKEND_GLES30;
	tGraphics.sName = "gles30";
	xgeGraphicsBackendSet(&tGraphics);
	return XGE_OK;
#else
	__xgeEGLStageCopy(pContext->sLastStage, (int)sizeof(pContext->sLastStage), "not-compiled");
	return XGE_ERROR_UNSUPPORTED;
#endif
}

int xgeEGLUnitChecked(xge_egl_context_t* pContext)
{
    if(!pContext)return XGE_ERROR_INVALID_ARGUMENT;
#if defined(XGE_HAS_EGL)
    if(pContext->bCleanupPending || pContext->bInitialized){
        EGLDisplay display=(EGLDisplay)pContext->pDisplay;
        EGLContext context=(EGLContext)pContext->pContext;
        if(display==EGL_NO_DISPLAY)return XGE_ERROR_INVALID_STATE;
        pContext->bInitialized=0;pContext->bCleanupPending=1;
        /* Do not unbind an unrelated context after a failed MakeCurrent. */
        if(context!=EGL_NO_CONTEXT && eglGetCurrentContext()==context &&
            eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT)==EGL_FALSE)
            return __xgeEGLFail(pContext,"eglUnbind",XGE_ERROR_BACKEND_FAILED);
        if(context!=EGL_NO_CONTEXT){
            if(eglDestroyContext(display,context)==EGL_FALSE)
                return __xgeEGLFail(pContext,"eglDestroyContext",XGE_ERROR_BACKEND_FAILED);
            pContext->pContext=NULL;
        }
        if(pContext->pSurface){
            if(eglDestroySurface(display,(EGLSurface)pContext->pSurface)==EGL_FALSE)
                return __xgeEGLFail(pContext,"eglDestroySurface",XGE_ERROR_BACKEND_FAILED);
            pContext->pSurface=NULL;
        }
        if(eglTerminate(display)==EGL_FALSE)
            return __xgeEGLFail(pContext,"eglTerminate",XGE_ERROR_BACKEND_FAILED);
    }
#else
    if(pContext->bCleanupPending || pContext->bInitialized)return XGE_ERROR_UNSUPPORTED;
#endif
    memset(pContext,0,sizeof(*pContext));return XGE_OK;
}

void xgeEGLUnit(xge_egl_context_t* pContext)
{if(pContext)(void)xgeEGLUnitChecked(pContext);}

int xgeEGLMakeCurrent(xge_egl_context_t* pContext)
{
	if ( pContext == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pContext->bInitialized == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
#if defined(XGE_HAS_EGL)
	if ( eglMakeCurrent((EGLDisplay)pContext->pDisplay, (EGLSurface)pContext->pSurface, (EGLSurface)pContext->pSurface, (EGLContext)pContext->pContext) == EGL_FALSE ) {
		return __xgeEGLFail(pContext, "eglMakeCurrent", XGE_ERROR_BACKEND_FAILED);
	}
#endif
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
	if(iRet!=XGE_OK && pOffscreen->tEGL.bCleanupPending)return iRet;
	if ( (iRet != XGE_OK) && (iRet != XGE_ERROR_UNSUPPORTED) ) {
		return iRet;
	}
	if ( iRet == XGE_OK ) {
		pOffscreen->bEGLContext = 1;
	} else {
		pOffscreen->bFallbackRenderTarget = 1;
	}
	iRet = xgeRenderTargetCreate(&pOffscreen->tTarget, iWidth, iHeight);
	if ( iRet != XGE_OK ) {
		xgeEGLUnit(&pOffscreen->tEGL);
		if(!pOffscreen->tEGL.bCleanupPending)memset(pOffscreen, 0, sizeof(*pOffscreen));
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
	pOffscreen->bActive=0;
	xgeRenderTargetFree(&pOffscreen->tTarget);
	xgeEGLUnit(&pOffscreen->tEGL);
	if(!pOffscreen->tEGL.bCleanupPending)memset(pOffscreen, 0, sizeof(*pOffscreen));
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
