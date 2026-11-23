static GLuint __xgeRenderTargetFramebuffer(const xge_render_target_t* target)
{
    return (GLuint)((target->iFlags&XGE_RENDER_TARGET_WINDOW) ? g_xge.iWindowFramebufferId:target->iFramebufferId);
}

static int __xgeRenderTargetEnsureFramebuffer(xge_render_target pTarget)
{
	GLuint iFramebuffer;
	GLuint iStencilRenderbuffer;

	if ( (pTarget == NULL) || (pTarget->iWidth <= 0) || (pTarget->iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pTarget->iFlags & XGE_RENDER_TARGET_WINDOW) != 0 ) {
		return pTarget->iWidth<=g_xge.iFramebufferWidth && pTarget->iHeight<=g_xge.iFramebufferHeight
			? XGE_OK:XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xge.bGraphicsRunning == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	if ( pTarget->tTexture.iBackendId == 0 ) {
		if ( __xgeTextureUploadNow(&pTarget->tTexture) != XGE_OK ) {
			return XGE_ERROR_GPU_FAILED;
		}
	}
	if ( pTarget->iFramebufferId != 0 ) {
		return XGE_OK;
	}
	if ( (xge_glGenFramebuffers == NULL) || (xge_glDeleteFramebuffers == NULL) || (xge_glBindFramebuffer == NULL) || (xge_glFramebufferTexture2D == NULL) || (xge_glCheckFramebufferStatus == NULL) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	iStencilRenderbuffer = 0;
	xge_glGenFramebuffers(1, &iFramebuffer);
	xge_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, iFramebuffer);
	xge_glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (GLuint)pTarget->tTexture.iBackendId, 0);
	if ( (xge_glGenRenderbuffers != NULL) && (xge_glDeleteRenderbuffers != NULL) && (xge_glBindRenderbuffer != NULL) && (xge_glRenderbufferStorage != NULL) && (xge_glFramebufferRenderbuffer != NULL) ) {
		xge_glGenRenderbuffers(1, &iStencilRenderbuffer);
		xge_glBindRenderbuffer(GL_RENDERBUFFER, iStencilRenderbuffer);
		xge_glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, pTarget->iWidth, pTarget->iHeight);
		xge_glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, iStencilRenderbuffer);
		xge_glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	if ( xge_glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ) {
		xge_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint)g_xge.iCurrentFramebufferId);
		if ( iStencilRenderbuffer != 0 ) {
			xge_glDeleteRenderbuffers(1, &iStencilRenderbuffer);
		}
		xge_glDeleteFramebuffers(1, &iFramebuffer);
		return XGE_ERROR_GPU_FAILED;
	}
	xge_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint)g_xge.iCurrentFramebufferId);
	pTarget->iFramebufferId = iFramebuffer;
	pTarget->iStencilRenderbufferId = iStencilRenderbuffer;
	return XGE_OK;
}

static void __xgeRenderTargetUnpremultiply(unsigned char* pPixels, int iWidth, int iHeight, int iStride)
{
	int x;
	int y;
	if ( pPixels == NULL ) return;
	for ( y = 0; y < iHeight; y++ ) {
		unsigned char* pRow = pPixels + ((size_t)y * (size_t)iStride);
		for ( x = 0; x < iWidth; x++ ) {
			unsigned char* pPixel = pRow + ((size_t)x * 4u);
			unsigned int iAlpha = pPixel[3];
			if ( iAlpha == 0u ) {
				pPixel[0] = pPixel[1] = pPixel[2] = 0u;
			} else if ( iAlpha < 255u ) {
				pPixel[0] = (unsigned char)(((unsigned int)pPixel[0] * 255u + (iAlpha / 2u)) / iAlpha);
				pPixel[1] = (unsigned char)(((unsigned int)pPixel[1] * 255u + (iAlpha / 2u)) / iAlpha);
				pPixel[2] = (unsigned char)(((unsigned int)pPixel[2] * 255u + (iAlpha / 2u)) / iAlpha);
			}
		}
	}
}

int xgeRenderTargetWindow(xge_render_target pTarget)
{
	if ( pTarget == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pTarget, 0, sizeof(*pTarget));
	pTarget->iWidth = g_xge.iFramebufferWidth;
	pTarget->iHeight = g_xge.iFramebufferHeight;
	pTarget->iFlags = XGE_RENDER_TARGET_WINDOW;
	return XGE_OK;
}

int xgeRenderTargetCreate(xge_render_target pTarget, int iWidth, int iHeight)
{
	int iRet;

	if ( (pTarget == NULL) || (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pTarget, 0, sizeof(*pTarget));
	iRet = xgeTextureCreateRGBAEx(&pTarget->tTexture, iWidth, iHeight, NULL, XGE_TEXTURE_COMPRESS_NONE);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pTarget->tTexture.iFlags |= XGE_TEXTURE_ORIGIN_BOTTOM_LEFT | XGE_TEXTURE_RENDER_TARGET;
	pTarget->iWidth = iWidth;
	pTarget->iHeight = iHeight;
	pTarget->iFlags = XGE_RENDER_TARGET_TEXTURE;
	if ( g_xge.bGraphicsRunning != 0 ) {
		iRet = __xgeRenderTargetEnsureFramebuffer(pTarget);
		if ( iRet != XGE_OK ) {
			xgeRenderTargetFree(pTarget);
			return iRet;
		}
	}
	return XGE_OK;
}

static void __xgeRenderTargetDispose(xge_render_target pTarget);
int xgeRenderTargetResize(xge_render_target pTarget, int iWidth, int iHeight)
{
	if ( (pTarget == NULL) || (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
    if(pTarget->_iActivePasses || pTarget->tTexture.iRefCount>1)return XGE_ERROR_BUSY;
	if ( (pTarget->iFlags & XGE_RENDER_TARGET_WINDOW) != 0 ) {
		pTarget->iWidth = iWidth;
		pTarget->iHeight = iHeight;
		return XGE_OK;
	}
	if ( (pTarget->iFlags & XGE_RENDER_TARGET_TEXTURE) == 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
    xge_render_target_t next={0};int result=xgeRenderTargetCreate(&next,iWidth,iHeight);
    if(result!=XGE_OK)return result;
    /* The upload queue stores texture addresses. Relocate its prepared node
     * before publishing the candidate; never leave a pointer to this stack
     * record or detach the old node through a copied texture address. */
    __xgeTextureUploadQueueRemove(&pTarget->tTexture);
    __xgeTextureUploadQueueRebind(&next.tTexture,&pTarget->tTexture);
    int owners=pTarget->_iOwnerReferences;
    xge_render_target_t previous=*pTarget;*pTarget=next;pTarget->_iOwnerReferences=owners;
    if(owners && pTarget->tTexture.pBackend)
        ((xge_texture_shadow_t*)pTarget->tTexture.pBackend)->pTargetOwner=pTarget;
    __xgeRenderTargetDispose(&previous);return XGE_OK;
}

int xgeRenderTargetCaptureCurrent(xge_render_target pTarget, int iSrcX, int iSrcY)
{
	int iGLY;
	int iRet;

	if ( (pTarget == NULL) || ((pTarget->iFlags & XGE_RENDER_TARGET_TEXTURE) == 0) ||
	     (pTarget->iWidth <= 0) || (pTarget->iHeight <= 0) || (iSrcX < 0) || (iSrcY < 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iSrcX > (g_xge.iWidth - pTarget->iWidth)) ||
	     (iSrcY > (g_xge.iHeight - pTarget->iHeight)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (g_xge.bGraphicsRunning == 0) || (xge_glCopyTexSubImage2D == NULL) || (xge_glActiveTexture == NULL) || (xge_glBindTexture == NULL) ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	iRet = xgeFlush();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeRenderTargetEnsureFramebuffer(pTarget);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iGLY = g_xge.iHeight - iSrcY - pTarget->iHeight;
	xge_glActiveTexture(GL_TEXTURE0);
	xge_glBindTexture(GL_TEXTURE_2D, (GLuint)pTarget->tTexture.iBackendId);
	xge_glCopyTexSubImage2D(
		GL_TEXTURE_2D, 0, 0, 0, iSrcX, iGLY,
		pTarget->iWidth, pTarget->iHeight
	);
	xge_glBindTexture(GL_TEXTURE_2D, 0);
	return XGE_OK;
}

static int __xgeRenderTargetPixelSize(xge_render_target target, int* row, size_t* size)
{
	if ( !target || target->iWidth <= 0 || target->iHeight <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( target->iWidth > INT32_MAX / 4 || target->iHeight > INT32_MAX / (target->iWidth * 4) )
		return XGE_ERROR_OUT_OF_MEMORY;
	*row = target->iWidth * 4;
	*size = (size_t)*row * (size_t)target->iHeight;
	return XGE_OK;
}

static int __xgeRenderTargetReadGpu(xge_render_target pTarget, size_t size, unsigned char** pixels)
{
	GLint iPreviousReadFramebuffer, iPackBuffer, arrPack[4];
	const GLenum arrPackNames[] = {0x0D05,0x0D02,0x0D03,0x0D04}; /* ALIGNMENT, ROW_LENGTH, SKIP_ROWS, SKIP_PIXELS */
	const GLenum iReadFramebuffer = 0x8CA8, iPixelPackBuffer = 0x88EB;
	if ( !xge_glFinish || !xge_glReadPixels || !xge_glGetIntegerv || !xge_glBindFramebuffer ||
	     !xge_glGetError || !xge_glPixelStorei || !xge_glBindBuffer ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	int iRet = xgeFlush();
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeRenderTargetEnsureFramebuffer(pTarget);
	if ( iRet != XGE_OK ) return iRet;
	unsigned char* pTemp = xrtMalloc(size);
	if ( pTemp == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xge_glGetIntegerv(0x8CAA, &iPreviousReadFramebuffer); /* READ_FRAMEBUFFER_BINDING */
	xge_glGetIntegerv(0x88ED, &iPackBuffer); /* PIXEL_PACK_BUFFER_BINDING */
	xge_glBindBuffer(iPixelPackBuffer, 0);
	for ( int iRow = 0; iRow < 4; iRow++ ) {
		xge_glGetIntegerv(arrPackNames[iRow], &arrPack[iRow]);
		xge_glPixelStorei(arrPackNames[iRow], iRow == 0 ? 1 : 0);
	}
	xge_glBindFramebuffer(iReadFramebuffer, __xgeRenderTargetFramebuffer(pTarget));
	/* Complete the queued draws before this synchronous CPU snapshot. Single-
	 * worker llvmpipe can otherwise return stale tiles from a reused target. */
	xge_glFinish();
	xge_glReadPixels(0, 0, pTarget->iWidth, pTarget->iHeight, GL_RGBA, GL_UNSIGNED_BYTE, pTemp);
	iRet = xge_glGetError() == GL_NO_ERROR ? XGE_OK : XGE_ERROR_GPU_FAILED;
	xge_glBindFramebuffer(iReadFramebuffer, (GLuint)iPreviousReadFramebuffer);
	for ( int iRow = 0; iRow < 4; iRow++ ) xge_glPixelStorei(arrPackNames[iRow], arrPack[iRow]);
	xge_glBindBuffer(iPixelPackBuffer, (GLuint)iPackBuffer);
	if ( iRet != XGE_OK ) {
		xrtFree(pTemp);
		return iRet;
	}
	*pixels = pTemp;
	return XGE_OK;
}

int xgeRenderTargetReadPixels(xge_render_target pTarget, void* pPixels, int iStride)
{
	if ( !pPixels ) return XGE_ERROR_INVALID_ARGUMENT;
	int row;
	size_t size;
	int status = __xgeRenderTargetPixelSize(pTarget, &row, &size);
	if ( status != XGE_OK ) return status;
	if ( iStride <= 0 ) iStride = row;
	if ( iStride < row ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( (size_t)(pTarget->iHeight - 1) > (SIZE_MAX - (size_t)row) / (size_t)iStride )
		return XGE_ERROR_OUT_OF_MEMORY;
	if ( !g_xge.bGraphicsRunning ) {
		if ( !(pTarget->iFlags & XGE_RENDER_TARGET_TEXTURE) ) return XGE_ERROR_NOT_INITIALIZED;
		status = xgeTextureReadPixels(&pTarget->tTexture, pPixels, iStride);
		if ( status == XGE_OK ) __xgeRenderTargetUnpremultiply(pPixels, pTarget->iWidth, pTarget->iHeight, iStride);
		return status;
	}
	unsigned char* gpu = NULL;
	status = __xgeRenderTargetReadGpu(pTarget, size, &gpu);
	if ( status != XGE_OK ) return status;
	for ( int y = 0; y < pTarget->iHeight; ++y ) {
		const unsigned char* src = gpu + (size_t)(pTarget->iHeight - 1 - y) * (size_t)row;
		memcpy((unsigned char*)pPixels + (size_t)y * (size_t)iStride, src, (size_t)row);
	}
	xrtFree(gpu);
	__xgeRenderTargetUnpremultiply(pPixels, pTarget->iWidth, pTarget->iHeight, iStride);
	return XGE_OK;
}

int xgeRenderTargetReadPixelsOwned(xge_render_target target, void** output, size_t* length)
{
	if ( output ) *output = NULL;
	if ( length ) *length = 0;
	if ( !output || !length ) return XGE_ERROR_INVALID_ARGUMENT;
	int row;
	size_t size;
	int status = __xgeRenderTargetPixelSize(target, &row, &size);
	if ( status != XGE_OK ) return status;
	unsigned char* data = NULL;
	if ( g_xge.bGraphicsRunning ) {
		status = __xgeRenderTargetReadGpu(target, size, &data);
		if ( status != XGE_OK ) return status;
		unsigned char scratch[256];
		for ( int y = 0; y < target->iHeight / 2; ++y ) {
			unsigned char* top = data + (size_t)y * (size_t)row;
			unsigned char* bottom = data + (size_t)(target->iHeight - 1 - y) * (size_t)row;
			for ( size_t offset = 0; offset < (size_t)row; ) {
				size_t chunk = (size_t)row - offset;
				if ( chunk > sizeof(scratch) ) chunk = sizeof(scratch);
				memcpy(scratch, top + offset, chunk);
				memcpy(top + offset, bottom + offset, chunk);
				memcpy(bottom + offset, scratch, chunk);
				offset += chunk;
			}
		}
	} else {
		if ( !(target->iFlags & XGE_RENDER_TARGET_TEXTURE) ) return XGE_ERROR_NOT_INITIALIZED;
		data = xrtMalloc(size);
		if ( !data ) return XGE_ERROR_OUT_OF_MEMORY;
		status = xgeTextureReadPixels(&target->tTexture, data, row);
		if ( status != XGE_OK ) {
			xrtFree(data);
			return status;
		}
	}
	__xgeRenderTargetUnpremultiply(data, target->iWidth, target->iHeight, row);
	*output = data;
	*length = size;
	return XGE_OK;
}

xge_texture xgeRenderTargetTexture(xge_render_target pTarget)
{
	if ( (pTarget == NULL) || ((pTarget->iFlags & XGE_RENDER_TARGET_TEXTURE) == 0) ) {
		return NULL;
	}
	return &pTarget->tTexture;
}

static void __xgeRenderTargetDispose(xge_render_target pTarget)
{
	GLuint iFramebuffer;
	GLuint iStencilRenderbuffer;

	if ( pTarget == NULL ) {
		return;
	}
	iFramebuffer = (GLuint)pTarget->iFramebufferId;
	iStencilRenderbuffer = (GLuint)pTarget->iStencilRenderbufferId;
	if ( (iFramebuffer != 0) && (g_xge.bGraphicsRunning != 0) && (xge_glDeleteFramebuffers != NULL) ) {
		if ( g_xge.iCurrentFramebufferId == pTarget->iFramebufferId ) {
			xge_glBindFramebuffer(GL_FRAMEBUFFER, 0);
			g_xge.iCurrentFramebufferId = 0;
		}
		xge_glDeleteFramebuffers(1, &iFramebuffer);
	}
	if ( (iStencilRenderbuffer != 0) && (g_xge.bGraphicsRunning != 0) && (xge_glDeleteRenderbuffers != NULL) ) {
		xge_glDeleteRenderbuffers(1, &iStencilRenderbuffer);
	}
	xgeTextureFree(&pTarget->tTexture);
	memset(pTarget, 0, sizeof(*pTarget));
}

int xgeRenderTargetAddRef(xge_render_target target)
{
    if(!target || target->_iOwnerReferences<=0 || target->_iOwnerReferences==INT32_MAX)return 0;
    return ++target->_iOwnerReferences;
}

int xgeRenderTargetShareOwned(xge_render_target pTarget, xge_render_target* ppShared)
{
    if (ppShared == NULL) return XGE_ERROR_INVALID_ARGUMENT;
    *ppShared = NULL;
    if (pTarget == NULL) return XGE_ERROR_INVALID_ARGUMENT;
    int status = __xgeGraphicsOwnerAccess(&g_xgeGraphicsLifetime);
    if (status != XGE_OK) return status;
    if (xgeRenderTargetAddRef(pTarget) <= 0) return XGE_ERROR_INVALID_STATE;
    *ppShared = pTarget;
    return XGE_OK;
}

void xgeRenderTargetFree(xge_render_target target)
{
    if(!target)return;
    if(target->_iOwnerReferences>0){
        bool consumed=false;
        if(xgeRenderTargetReleaseOwnerChecked(target,&consumed)!=XGE_OK && !xrtGetError())xrtSetErrorKind(XERR_STATE);
        return;
    }
    if(target->_iActivePasses || target->tTexture.iRefCount>1){xrtSetErrorKind(XERR_STATE);return;}
    __xgeRenderTargetDispose(target);
}
int xgeRenderTargetReleaseOwnerChecked(xge_render_target target,bool* consumed)
{
    if(!consumed)return XGE_ERROR_INVALID_ARGUMENT;
    *consumed=false;
    if(!target || target->_iOwnerReferences<=0)return XGE_ERROR_INVALID_ARGUMENT;
    if(target->_iOwnerReferences>1){--target->_iOwnerReferences;*consumed=true;return XGE_OK;}
    if(target->_iActivePasses || target->tTexture.iRefCount>1)return XGE_ERROR_BUSY;
    __xgeRenderTargetDispose(target);xrtFree(target);*consumed=true;
    return __xgeGraphicsOwnerRelease(&g_xgeGraphicsLifetime);
}
int xgeRenderTargetReadPixelsSized(xge_render_target target,void* pixels,size_t capacity,int stride)
{
    if(!target || !pixels || target->iWidth<=0 || target->iHeight<=0 || target->iWidth>INT32_MAX/4)return XGE_ERROR_INVALID_ARGUMENT;
    size_t row=(size_t)target->iWidth*4;
    if(stride<=0)stride=(int)row;
    if((size_t)stride<row || (size_t)(target->iHeight-1)>(SIZE_MAX-row)/(size_t)stride ||
        capacity<(size_t)(target->iHeight-1)*(size_t)stride+row)return XGE_ERROR_INVALID_ARGUMENT;
    return xgeRenderTargetReadPixels(target,pixels,stride);
}

#include "xge_pass.inl"
