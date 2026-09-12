typedef struct xge_texture_shadow_t {
	int iWidth;
	int iHeight;
	int iFormat;
	int iStride;
	unsigned char* pPixels;
	int iStorage;
	int iSwizzle;
	int bPackingDisabled;
} xge_texture_shadow_t;

typedef struct xge_texture_yuv420p_t {
	int iWidth;
	int iHeight;
	int iPlaneWidth[3];
	int iPlaneHeight[3];
	int iPlaneStride[3];
	unsigned char* pPlane[3];
	GLuint iPlaneTexture[3];
} xge_texture_yuv420p_t;

static uint64_t __xgeTextureMemoryBytes(xge_texture pTexture)
{
	int iUVWidth;
	int iUVHeight;

	if ( (pTexture == NULL) || (pTexture->iWidth <= 0) || (pTexture->iHeight <= 0) ) {
		return 0;
	}
	if ( pTexture->iFormat == XGE_PIXEL_YUV420P ) {
		iUVWidth = (pTexture->iWidth + 1) / 2;
		iUVHeight = (pTexture->iHeight + 1) / 2;
		return (uint64_t)pTexture->iWidth * (uint64_t)pTexture->iHeight + (uint64_t)iUVWidth * (uint64_t)iUVHeight * 2u;
	}
	if ( pTexture->iFormat == XGE_PIXEL_RGBA8 ) {
		const xge_texture_shadow_t* pShadow = (const xge_texture_shadow_t*)pTexture->pBackend;
		unsigned int iBytes = 4u;
		if ( pShadow != NULL && pShadow->iStorage == XGE_TEXTURE_STORAGE_R8 ) iBytes = 1u;
		else if ( pShadow != NULL && pShadow->iStorage >= XGE_TEXTURE_STORAGE_RG8 ) iBytes = 2u;
		return (uint64_t)pTexture->iWidth * (uint64_t)pTexture->iHeight * iBytes;
	}
	return 0;
}

static int __xgeTextureShadowSet(xge_texture pTexture, int iWidth, int iHeight, const void* pPixels)
{
	xge_texture_shadow_t* pShadow;
	int iSize;

	if ( (pTexture == NULL) || (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iWidth > (INT32_MAX / 4) || iHeight > (INT32_MAX / (iWidth * 4)) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iSize = iWidth * iHeight * 4;
	pShadow = (xge_texture_shadow_t*)xrtMalloc(sizeof(*pShadow));
	if ( pShadow == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pShadow, 0, sizeof(*pShadow));
	pShadow->pPixels = (unsigned char*)xrtMalloc((size_t)iSize);
	if ( pShadow->pPixels == NULL ) {
		xrtFree(pShadow);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( pPixels != NULL ) {
		memcpy(pShadow->pPixels, pPixels, (size_t)iSize);
	} else {
		memset(pShadow->pPixels, 0, (size_t)iSize);
	}
	pShadow->iWidth = iWidth;
	pShadow->iHeight = iHeight;
	pShadow->iFormat = XGE_PIXEL_RGBA8;
	pShadow->iStride = iWidth * 4;
	pShadow->iStorage = XGE_TEXTURE_STORAGE_RGBA8;
	pTexture->pBackend = pShadow;
	return XGE_OK;
}

static void __xgeTextureShadowFree(xge_texture pTexture)
{
	xge_texture_shadow_t* pShadow;

	if ( (pTexture == NULL) || (pTexture->pBackend == NULL) ) {
		return;
	}
	pShadow = (xge_texture_shadow_t*)pTexture->pBackend;
	if ( pShadow->pPixels != NULL ) {
		xrtFree(pShadow->pPixels);
	}
	xrtFree(pShadow);
	pTexture->pBackend = NULL;
}

static int __xgeTextureHasShadow(xge_texture pTexture)
{
	xge_texture_shadow_t* pShadow;

	if ( (pTexture == NULL) || (pTexture->pBackend == NULL) ) {
		return 0;
	}
	pShadow = (xge_texture_shadow_t*)pTexture->pBackend;
	if ( (pShadow->pPixels == NULL) || (pShadow->iWidth != pTexture->iWidth) || (pShadow->iHeight != pTexture->iHeight) || (pShadow->iFormat != XGE_PIXEL_RGBA8) ) {
		return 0;
	}
	return 1;
}

static int __xgeTextureYUV420PAlloc(xge_texture pTexture, int iWidth, int iHeight)
{
	xge_texture_yuv420p_t* pYUV;
	int i;

	if ( (pTexture == NULL) || (iWidth <= 0) || (iHeight <= 0) ||
		 (iWidth >= INT_MAX) || (iHeight >= INT_MAX) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pYUV = (xge_texture_yuv420p_t*)xrtMalloc(sizeof(*pYUV));
	if ( pYUV == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pYUV, 0, sizeof(*pYUV));
	pYUV->iWidth = iWidth;
	pYUV->iHeight = iHeight;
	pYUV->iPlaneWidth[0] = iWidth;
	pYUV->iPlaneHeight[0] = iHeight;
	pYUV->iPlaneWidth[1] = (iWidth + 1) / 2;
	pYUV->iPlaneHeight[1] = (iHeight + 1) / 2;
	pYUV->iPlaneWidth[2] = pYUV->iPlaneWidth[1];
	pYUV->iPlaneHeight[2] = pYUV->iPlaneHeight[1];
	for ( i = 0; i < 3; i++ ) {
		size_t iSize;
		pYUV->iPlaneStride[i] = pYUV->iPlaneWidth[i];
		iSize = (size_t)pYUV->iPlaneStride[i] * (size_t)pYUV->iPlaneHeight[i];
		pYUV->pPlane[i] = (unsigned char*)xrtMalloc(iSize);
		if ( pYUV->pPlane[i] == NULL ) {
			int j;
			for ( j = 0; j < i; j++ ) {
				xrtFree(pYUV->pPlane[j]);
			}
			xrtFree(pYUV);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pYUV->pPlane[i], i == 0 ? 0 : 128, iSize);
	}
	pTexture->pBackend = pYUV;
	return XGE_OK;
}

static int __xgeTextureHasYUV420P(xge_texture pTexture)
{
	xge_texture_yuv420p_t* pYUV;

	if ( (pTexture == NULL) || (pTexture->pBackend == NULL) || (pTexture->iFormat != XGE_PIXEL_YUV420P) ) {
		return 0;
	}
	pYUV = (xge_texture_yuv420p_t*)pTexture->pBackend;
	if ( (pYUV->iWidth != pTexture->iWidth) || (pYUV->iHeight != pTexture->iHeight) ) {
		return 0;
	}
	return (pYUV->pPlane[0] != NULL) && (pYUV->pPlane[1] != NULL) && (pYUV->pPlane[2] != NULL);
}

static void __xgeTextureYUV420PFree(xge_texture pTexture)
{
	xge_texture_yuv420p_t* pYUV;
	int i;

	if ( (pTexture == NULL) || (pTexture->pBackend == NULL) ) {
		return;
	}
	pYUV = (xge_texture_yuv420p_t*)pTexture->pBackend;
	if ( glDeleteTextures != NULL ) {
		for ( i = 0; i < 3; i++ ) {
			if ( pYUV->iPlaneTexture[i] != 0 ) {
				glDeleteTextures(1, &pYUV->iPlaneTexture[i]);
				pYUV->iPlaneTexture[i] = 0;
			}
		}
	}
	for ( i = 0; i < 3; i++ ) {
		if ( pYUV->pPlane[i] != NULL ) {
			xrtFree(pYUV->pPlane[i]);
		}
	}
	xrtFree(pYUV);
	pTexture->pBackend = NULL;
}

static GLint __xgeSamplerFilterToGL(int iFilter)
{
	if ( iFilter == XGE_FILTER_NEAREST ) {
		return GL_NEAREST;
	}
	return GL_LINEAR;
}

static GLint __xgeSamplerWrapToGL(int iWrap)
{
	if ( iWrap == XGE_WRAP_REPEAT ) {
		return GL_REPEAT;
	}
	return GL_CLAMP_TO_EDGE;
}

xge_sampler_t xgeSamplerDefault(void)
{
	xge_sampler_t tSampler;

	tSampler.iMinFilter = XGE_FILTER_LINEAR;
	tSampler.iMagFilter = XGE_FILTER_LINEAR;
	tSampler.iWrapS = XGE_WRAP_CLAMP;
	tSampler.iWrapT = XGE_WRAP_CLAMP;
	return tSampler;
}

static int __xgeSamplerNormalize(xge_sampler_t* pSampler)
{
	if ( pSampler == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pSampler->iMinFilter == 0 ) {
		pSampler->iMinFilter = XGE_FILTER_LINEAR;
	}
	if ( pSampler->iMagFilter == 0 ) {
		pSampler->iMagFilter = XGE_FILTER_LINEAR;
	}
	if ( pSampler->iWrapS == 0 ) {
		pSampler->iWrapS = XGE_WRAP_CLAMP;
	}
	if ( pSampler->iWrapT == 0 ) {
		pSampler->iWrapT = XGE_WRAP_CLAMP;
	}
	if ( ((pSampler->iMinFilter != XGE_FILTER_NEAREST) && (pSampler->iMinFilter != XGE_FILTER_LINEAR)) || ((pSampler->iMagFilter != XGE_FILTER_NEAREST) && (pSampler->iMagFilter != XGE_FILTER_LINEAR)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( ((pSampler->iWrapS != XGE_WRAP_CLAMP) && (pSampler->iWrapS != XGE_WRAP_REPEAT)) || ((pSampler->iWrapT != XGE_WRAP_CLAMP) && (pSampler->iWrapT != XGE_WRAP_REPEAT)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return XGE_OK;
}

static int __xgeTextureApplySampler(xge_texture pTexture)
{
	xge_sampler_t tSampler;
	xge_texture_yuv420p_t* pYUV;
	int i;

	if ( (pTexture == NULL) || (pTexture->iBackendId == 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (g_xge.bSokolRunning == 0) && (g_xge.bRenderThreadGLCurrent == 0) ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	if ( (glBindTexture == NULL) || (glTexParameteri == NULL) ) {
		return XGE_ERROR_GPU_FAILED;
	}
	tSampler = pTexture->tSampler;
	if ( __xgeSamplerNormalize(&tSampler) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pTexture->iFormat == XGE_PIXEL_YUV420P ) {
		if ( !__xgeTextureHasYUV420P(pTexture) ) {
			return XGE_ERROR_RESOURCE_FAILED;
		}
		pYUV = (xge_texture_yuv420p_t*)pTexture->pBackend;
		for ( i = 0; i < 3; i++ ) {
			if ( pYUV->iPlaneTexture[i] == 0 ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
			glBindTexture(GL_TEXTURE_2D, pYUV->iPlaneTexture[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, __xgeSamplerFilterToGL(tSampler.iMinFilter));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, __xgeSamplerFilterToGL(tSampler.iMagFilter));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, __xgeSamplerWrapToGL(tSampler.iWrapS));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, __xgeSamplerWrapToGL(tSampler.iWrapT));
		}
	} else {
		glBindTexture(GL_TEXTURE_2D, (GLuint)pTexture->iBackendId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, __xgeSamplerFilterToGL(tSampler.iMinFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, __xgeSamplerFilterToGL(tSampler.iMagFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, __xgeSamplerWrapToGL(tSampler.iWrapS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, __xgeSamplerWrapToGL(tSampler.iWrapT));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	pTexture->tSampler = tSampler;
	return XGE_OK;
}

static int __xgeTextureUploadYUV420PNow(xge_texture pTexture)
{
	xge_texture_yuv420p_t* pYUV;
	int i;
	int iRet;

	if ( (pTexture == NULL) || (pTexture->iWidth <= 0) || (pTexture->iHeight <= 0) || (pTexture->iFormat != XGE_PIXEL_YUV420P) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pTexture->iBackendId != 0 ) {
		return XGE_OK;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	if ( !__xgeTextureHasYUV420P(pTexture) ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( (glGenTextures == NULL) || (glBindTexture == NULL) || (glTexImage2D == NULL) ) {
		return XGE_ERROR_GPU_FAILED;
	}
	pYUV = (xge_texture_yuv420p_t*)pTexture->pBackend;
	glGenTextures(3, pYUV->iPlaneTexture);
	if ( glPixelStorei != NULL ) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}
	for ( i = 0; i < 3; i++ ) {
		glBindTexture(GL_TEXTURE_2D, pYUV->iPlaneTexture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, pYUV->iPlaneWidth[i], pYUV->iPlaneHeight[i], 0, GL_RED, GL_UNSIGNED_BYTE, pYUV->pPlane[i]);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	pTexture->iBackendId = pYUV->iPlaneTexture[0];
	iRet = __xgeTextureApplySampler(pTexture);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	return XGE_OK;
}

#include "xge_texture_storage.h"

static int __xgeTextureUploadNow(xge_texture pTexture)
{
	xge_texture_shadow_t* pShadow;
	xge_texture_choice_t arrChoices[6];
	GLuint iTexture;
	int iRet;
	int i, iCount;

	if ( (pTexture == NULL) || (pTexture->iWidth <= 0) || (pTexture->iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pTexture->iFormat == XGE_PIXEL_YUV420P ) {
		return __xgeTextureUploadYUV420PNow(pTexture);
	}
	if ( pTexture->iFormat != XGE_PIXEL_RGBA8 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pTexture->iBackendId != 0 ) {
		return XGE_OK;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		return XGE_ERROR_NOT_INITIALIZED;
	}
	if ( !__xgeTextureHasShadow(pTexture) ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( (glGenTextures == NULL) || (glBindTexture == NULL) ||
		 (glTexImage2D == NULL) ) {
		return XGE_ERROR_GPU_FAILED;
	}
	pShadow = (xge_texture_shadow_t*)pTexture->pBackend;
	iCount = __xgeTextureChoices(pTexture, arrChoices);
	for ( i = 0; i < iCount; i++ ) {
		iRet = __xgeTextureNewGPU(pTexture, pShadow->pPixels, arrChoices[i], &iTexture);
		if ( iRet == XGE_OK ) {
			pTexture->iBackendId = iTexture;
			__xgeTextureStorageCommit(pTexture, arrChoices[i]);
			return XGE_OK;
		}
		if ( iRet != XGE_ERROR_UNSUPPORTED ) return iRet;
	}
	return XGE_ERROR_GPU_FAILED;
}

static int __xgeTextureFallbackUse(xge_texture pTexture, int iOriginalError, uint32_t iFlags)
{
	xge_texture_shadow_t* pShadow;
	int iRet;

	if ( (pTexture == NULL) || (g_xge.tFallbackTexture.iRefCount <= 0) ) {
		return iOriginalError;
	}
	pShadow = (xge_texture_shadow_t*)g_xge.tFallbackTexture.pBackend;
	if ( pShadow == NULL || pShadow->pPixels == NULL ) return iOriginalError;
	iRet = xgeTextureCreateRGBAEx(pTexture, g_xge.tFallbackTexture.iWidth, g_xge.tFallbackTexture.iHeight, pShadow->pPixels, iFlags);
	if ( iRet != XGE_OK ) {
		return iOriginalError;
	}
	pTexture->iFlags |= XGE_TEXTURE_FALLBACK;
	return XGE_OK;
}

int xgeTextureCreateRGBA(xge_texture pTexture, int iWidth, int iHeight, const void* pPixels)
{
	return xgeTextureCreateRGBAEx(pTexture, iWidth, iHeight, pPixels, XGE_TEXTURE_COMPRESS_LOSSLESS);
}

int xgeTextureCreateRGBAEx(xge_texture pTexture, int iWidth, int iHeight, const void* pPixels, uint32_t iFlags)
{
	int iRet;

	if ( (pTexture == NULL) || (iWidth <= 0) || (iHeight <= 0) ||
	     ((iFlags & XGE_TEXTURE_COMPRESS_MASK) == XGE_TEXTURE_COMPRESS_MASK) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pTexture->iRefCount != 0) || (pTexture->iBackendId != 0) ||
		 (pTexture->pBackend != NULL) ) {
		return XGE_ERROR_INVALID_STATE;
	}

	memset(pTexture, 0, sizeof(*pTexture));
	iRet = __xgeTextureShadowSet(pTexture, iWidth, iHeight, pPixels);
	if ( iRet != XGE_OK ) {
		return iRet;
	}

	pTexture->iWidth = iWidth;
	pTexture->iHeight = iHeight;
	pTexture->iFormat = XGE_PIXEL_RGBA8;
	pTexture->iFlags = iFlags & XGE_TEXTURE_COMPRESS_MASK;
	pTexture->iRefCount = 1;
	pTexture->tSampler = xgeSamplerDefault();
	g_xge.iTextureCount++;
	g_xge.iTextureMemoryBytes += (uint64_t)iWidth * (uint64_t)iHeight * 4u;
	if ( g_xge.bSokolRunning != 0 ) {
		iRet = __xgeTextureUploadNow(pTexture);
		if ( iRet != XGE_OK ) {
			xgeTextureFree(pTexture);
			return iRet;
		}
	} else {
		(void)xgeTextureUploadQueue(pTexture);
	}
	return XGE_OK;
}

int xgeTextureCreateYUV420P(xge_texture pTexture, int iWidth, int iHeight)
{
	int iRet;

	if ( (pTexture == NULL) || (iWidth <= 0) || (iHeight <= 0) ||
		 (iWidth >= INT_MAX) || (iHeight >= INT_MAX) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pTexture->iRefCount != 0) || (pTexture->iBackendId != 0) ||
		 (pTexture->pBackend != NULL) ) {
		return XGE_ERROR_INVALID_STATE;
	}
	memset(pTexture, 0, sizeof(*pTexture));
	iRet = __xgeTextureYUV420PAlloc(pTexture, iWidth, iHeight);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pTexture->iWidth = iWidth;
	pTexture->iHeight = iHeight;
	pTexture->iFormat = XGE_PIXEL_YUV420P;
	pTexture->iRefCount = 1;
	pTexture->tSampler = xgeSamplerDefault();
	g_xge.iTextureCount++;
	g_xge.iTextureMemoryBytes += __xgeTextureMemoryBytes(pTexture);
	if ( g_xge.bSokolRunning != 0 ) {
		iRet = __xgeTextureUploadNow(pTexture);
		if ( iRet != XGE_OK ) {
			xgeTextureFree(pTexture);
			return iRet;
		}
	} else {
		(void)xgeTextureUploadQueue(pTexture);
	}
	return XGE_OK;
}

int xgeImageLoad(xge_image pImage, const char* sPath)
{
	return xgeImageLoadEx(pImage, sPath, XGE_IMAGE_PREMULTIPLIED);
}

int xgeImageLoadEx(xge_image pImage, const char* sPath, uint32_t iFlags)
{
	xge_resource_t tResource;
	int iRet;

	if ( (pImage == NULL) || (sPath == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}

	iRet = xgeResourceLoad(sPath, &tResource);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeImageLoadMemoryEx(pImage, tResource.pData, tResource.iSize, iFlags);
	xgeResourceFree(&tResource);
	return iRet;
}

int xgeImageLoadMemory(xge_image pImage, const void* pData, int iSize)
{
	return xgeImageLoadMemoryEx(pImage, pData, iSize, XGE_IMAGE_PREMULTIPLIED);
}

int xgeImageLoadMemoryEx(xge_image pImage, const void* pData, int iSize, uint32_t iFlags)
{
	int iWidth;
	int iHeight;
	int iChannels;
	unsigned char* pPixels;

	if ( (pImage == NULL) || (pData == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}

	pPixels = stbi_load_from_memory((const stbi_uc*)pData, iSize, &iWidth, &iHeight, &iChannels, 4);
	if ( pPixels == NULL ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	memset(pImage, 0, sizeof(*pImage));
	pImage->iWidth = iWidth;
	pImage->iHeight = iHeight;
	pImage->iFormat = XGE_PIXEL_RGBA8;
	pImage->iStride = iWidth * 4;
	pImage->pPixels = pPixels;
	pImage->iFlags = ((iFlags & XGE_IMAGE_STRAIGHT_ALPHA) != 0) ? XGE_IMAGE_STRAIGHT_ALPHA : XGE_IMAGE_PREMULTIPLIED;
	if ( (pImage->iFlags & XGE_IMAGE_PREMULTIPLIED) != 0 ) {
		xgeImagePremultiply(pImage);
	}
	return XGE_OK;
}

void* xgeImageGetPixels(xge_image pImage)
{
	if ( pImage == NULL ) {
		return NULL;
	}
	return pImage->pPixels;
}

void xgeImagePremultiply(xge_image pImage)
{
	unsigned char* pPixels;
	int i;
	int iCount;
	unsigned int iA;

	if ( (pImage == NULL) || (pImage->pPixels == NULL) || (pImage->iFormat != XGE_PIXEL_RGBA8) ) {
		return;
	}
	pPixels = (unsigned char*)pImage->pPixels;
	iCount = pImage->iWidth * pImage->iHeight;
	for ( i = 0; i < iCount; i++ ) {
		iA = (unsigned int)pPixels[(i * 4) + 3];
		pPixels[(i * 4) + 0] = (unsigned char)(((unsigned int)pPixels[(i * 4) + 0] * iA + 127u) / 255u);
		pPixels[(i * 4) + 1] = (unsigned char)(((unsigned int)pPixels[(i * 4) + 1] * iA + 127u) / 255u);
		pPixels[(i * 4) + 2] = (unsigned char)(((unsigned int)pPixels[(i * 4) + 2] * iA + 127u) / 255u);
	}
	pImage->iFlags &= ~XGE_IMAGE_STRAIGHT_ALPHA;
	pImage->iFlags |= XGE_IMAGE_PREMULTIPLIED;
}

static void __xgeImageWriteCallback(void* pContext, void* pData, int iSize)
{
	FILE* pFile;

	pFile = (FILE*)pContext;
	if ( (pFile != NULL) && (pData != NULL) && (iSize > 0) ) {
		(void)fwrite(pData, 1, (size_t)iSize, pFile);
	}
}

int xgeImageSavePNGEx(const char* sPath, int iWidth, int iHeight, const void* pPixels, int iStride, uint32_t iFlags)
{
	FILE* pFile;
	const void* pSavePixels;
	unsigned char* pStraightPixels;
	int iRet;
	int x;
	int y;

	if ( (sPath == NULL) || (iWidth <= 0) || (iHeight <= 0) || (pPixels == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iWidth > INT_MAX / 4 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iStride <= 0 ) {
		iStride = iWidth * 4;
	}
	if ( (iStride < iWidth * 4) ||
		 ((iFlags & ~(XGE_IMAGE_PREMULTIPLIED | XGE_IMAGE_STRAIGHT_ALPHA)) != 0u) ||
		 ((iFlags & (XGE_IMAGE_PREMULTIPLIED | XGE_IMAGE_STRAIGHT_ALPHA)) ==
		  (XGE_IMAGE_PREMULTIPLIED | XGE_IMAGE_STRAIGHT_ALPHA)) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pSavePixels = pPixels;
	pStraightPixels = NULL;
	if ( (iFlags & XGE_IMAGE_PREMULTIPLIED) != 0u ) {
		size_t iBytes = (size_t)iStride * (size_t)iHeight;
		if ( (size_t)iStride != 0u && (iBytes / (size_t)iStride) != (size_t)iHeight ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		pStraightPixels = (unsigned char*)xrtMalloc(iBytes);
		if ( pStraightPixels == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
		memcpy(pStraightPixels, pPixels, iBytes);
		for ( y = 0; y < iHeight; y++ ) {
			unsigned char* pRow = pStraightPixels + (size_t)y * (size_t)iStride;
			for ( x = 0; x < iWidth; x++ ) {
				unsigned char* pPixel = pRow + x * 4;
				unsigned int iAlpha = pPixel[3];
				if ( iAlpha == 0u ) {
					pPixel[0] = pPixel[1] = pPixel[2] = 0u;
				} else if ( iAlpha < 255u ) {
					pPixel[0] = (unsigned char)(((unsigned int)pPixel[0] * 255u + iAlpha / 2u) / iAlpha);
					pPixel[1] = (unsigned char)(((unsigned int)pPixel[1] * 255u + iAlpha / 2u) / iAlpha);
					pPixel[2] = (unsigned char)(((unsigned int)pPixel[2] * 255u + iAlpha / 2u) / iAlpha);
				}
			}
		}
		pSavePixels = pStraightPixels;
	}
#if defined(_WIN32) || defined(_WIN64)
	{
		wchar_t arrPathW[1024];
		int iPathLen;

		iPathLen = MultiByteToWideChar(CP_UTF8, 0, sPath, -1, arrPathW, (int)(sizeof(arrPathW) / sizeof(arrPathW[0])));
		if ( iPathLen > 0 ) {
			pFile = _wfopen(arrPathW, L"wb");
		} else {
			pFile = fopen(sPath, "wb");
		}
	}
#else
	pFile = fopen(sPath, "wb");
#endif
	if ( pFile == NULL ) {
		if ( pStraightPixels != NULL ) xrtFree(pStraightPixels);
		return XGE_ERROR_FILE_NOT_FOUND;
	}
	iRet = stbi_write_png_to_func(__xgeImageWriteCallback, pFile, iWidth, iHeight, 4, pSavePixels, iStride) ? XGE_OK : XGE_ERROR;
	fclose(pFile);
	if ( pStraightPixels != NULL ) xrtFree(pStraightPixels);
	return iRet;
}

int xgeImageSavePNG(const char* sPath, int iWidth, int iHeight, const void* pPixels, int iStride)
{
	return xgeImageSavePNGEx(sPath, iWidth, iHeight, pPixels, iStride, XGE_IMAGE_STRAIGHT_ALPHA);
}

void xgeImageFree(xge_image pImage)
{
	if ( pImage == NULL ) {
		return;
	}
	if ( pImage->pPixels != NULL ) {
		stbi_image_free(pImage->pPixels);
	}
	memset(pImage, 0, sizeof(*pImage));
}

int xgeTextureCreateFromImage(xge_texture pTexture, const xge_image_t* pImage)
{
	return xgeTextureCreateFromImageEx(pTexture, pImage, XGE_TEXTURE_COMPRESS_LOSSLESS);
}

int xgeTextureCreateFromImageEx(xge_texture pTexture, const xge_image_t* pImage, uint32_t iFlags)
{
	if ( (pTexture == NULL) || (pImage == NULL) || (pImage->pPixels == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return xgeTextureCreateRGBAEx(pTexture, pImage->iWidth, pImage->iHeight, pImage->pPixels, iFlags);
}

int xgeTextureLoad(xge_texture pTexture, const char* sPath)
{
	return xgeTextureLoadEx(pTexture, sPath, XGE_IMAGE_PREMULTIPLIED);
}

int xgeTextureLoadEx(xge_texture pTexture, const char* sPath, uint32_t iFlags)
{
	xge_image_t objImage;
	int iRet;

	if ( (iFlags & XGE_TEXTURE_COMPRESS_MASK) == XGE_TEXTURE_COMPRESS_MASK ) return XGE_ERROR_INVALID_ARGUMENT;
	iRet = xgeImageLoadEx(&objImage, sPath, iFlags);
	if ( iRet != XGE_OK ) {
		return __xgeTextureFallbackUse(pTexture, iRet, iFlags);
	}
	iRet = xgeTextureCreateFromImageEx(pTexture, &objImage, iFlags);
	xgeImageFree(&objImage);
	if ( iRet != XGE_OK ) {
		return __xgeTextureFallbackUse(pTexture, iRet, iFlags);
	}
	return iRet;
}

int xgeTextureLoadMemory(xge_texture pTexture, const void* pData, int iSize)
{
	return xgeTextureLoadMemoryEx(pTexture, pData, iSize, XGE_IMAGE_PREMULTIPLIED);
}

int xgeTextureLoadMemoryEx(xge_texture pTexture, const void* pData, int iSize, uint32_t iFlags)
{
	xge_image_t objImage;
	int iRet;

	if ( (iFlags & XGE_TEXTURE_COMPRESS_MASK) == XGE_TEXTURE_COMPRESS_MASK ) return XGE_ERROR_INVALID_ARGUMENT;
	iRet = xgeImageLoadMemoryEx(&objImage, pData, iSize, iFlags);
	if ( iRet != XGE_OK ) {
		return __xgeTextureFallbackUse(pTexture, iRet, iFlags);
	}
	iRet = xgeTextureCreateFromImageEx(pTexture, &objImage, iFlags);
	xgeImageFree(&objImage);
	if ( iRet != XGE_OK ) {
		return __xgeTextureFallbackUse(pTexture, iRet, iFlags);
	}
	return iRet;
}

int xgeTextureAddRef(xge_texture pTexture)
{
	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pTexture->iRefCount < INT32_MAX ) {
		pTexture->iRefCount++;
	}
	return pTexture->iRefCount;
}

int xgeTextureUpdateRGBA(xge_texture pTexture, int iX, int iY, int iWidth, int iHeight, const void* pPixels, int iStride)
{
	xge_texture_shadow_t* pShadow;
	xge_texture_choice_t tChoice;
	const unsigned char* pSrc = (const unsigned char*)pPixels;
	const void* pUpload;
	void* pPacked = NULL;
	int i, iCopyStride, iRet;
	GLint iBinding = 0, iAlignment = 4, iInternal;
	GLenum iFormat, iType;

	if ( pTexture == NULL || pPixels == NULL || iX < 0 || iY < 0 || iWidth <= 0 || iHeight <= 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pTexture->iFormat != XGE_PIXEL_RGBA8 || !__xgeTextureHasShadow(pTexture) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iWidth > pTexture->iWidth || iHeight > pTexture->iHeight ||
	     iX > pTexture->iWidth - iWidth || iY > pTexture->iHeight - iHeight ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCopyStride = iWidth * 4;
	if ( iStride <= 0 ) iStride = iCopyStride;
	if ( iStride < iCopyStride || (size_t)(iHeight - 1) > (SIZE_MAX - (size_t)iCopyStride) / (size_t)iStride ) return XGE_ERROR_INVALID_ARGUMENT;
	pShadow = (xge_texture_shadow_t*)pTexture->pBackend;
	tChoice = (xge_texture_choice_t){pShadow->iStorage, pShadow->iSwizzle};
	if ( (pTexture->iBackendId != 0) && (g_xge.bSokolRunning != 0) ) {
		if ( glBindTexture == NULL || glTexSubImage2D == NULL ) return XGE_ERROR_GPU_FAILED;
		if ( !__xgeTexturePatchFits(tChoice, pSrc, iWidth, iHeight, iStride) ) goto promote;
		pUpload = pSrc;
		if ( tChoice.iStorage != XGE_TEXTURE_STORAGE_RGBA8 || iStride != iCopyStride ) {
			pPacked = __xgeTexturePack(tChoice, pSrc, iWidth, iHeight, iStride);
			if ( pPacked == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
			pUpload = pPacked;
		}
		__xgeTextureGLFormat(tChoice.iStorage, &iInternal, &iFormat, &iType);
		if ( glGetIntegerv != NULL ) {
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &iBinding);
			glGetIntegerv(GL_UNPACK_ALIGNMENT, &iAlignment);
		}
		__xgeTextureClearGLErrors();
		glBindTexture(GL_TEXTURE_2D, (GLuint)pTexture->iBackendId);
		if ( glPixelStorei != NULL ) glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexSubImage2D(GL_TEXTURE_2D, 0, iX, iY, iWidth, iHeight, iFormat, iType, pUpload);
		iRet = __xgeTextureGLError(glGetError != NULL ? glGetError() : GL_NO_ERROR);
		if ( glPixelStorei != NULL ) glPixelStorei(GL_UNPACK_ALIGNMENT, iAlignment);
		glBindTexture(GL_TEXTURE_2D, (GLuint)iBinding);
		xrtFree(pPacked);
		if ( iRet == XGE_ERROR_UNSUPPORTED && tChoice.iStorage != XGE_TEXTURE_STORAGE_RGBA8 ) goto promote;
		if ( iRet != XGE_OK ) return iRet;
	}
	for ( i = 0; i < iHeight; i++ )
		memcpy(pShadow->pPixels + (size_t)(iY + i) * (size_t)pShadow->iStride + (size_t)iX * 4u,
		       pSrc + (size_t)i * (size_t)iStride, (size_t)iCopyStride);
	return XGE_OK;

promote:
	{
		/* One-way promotion preserves untouched pixels and the previous GPU/CPU
		 * contents if allocation/upload fails. Never recompress on later updates. */
		size_t iBytes = (size_t)pShadow->iStride * (size_t)pShadow->iHeight;
		unsigned char* pMerged = (unsigned char*)xrtMalloc(iBytes);
		GLuint iNew, iOld = (GLuint)pTexture->iBackendId;
		if ( pMerged == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
		memcpy(pMerged, pShadow->pPixels, iBytes);
		for ( i = 0; i < iHeight; i++ )
			memcpy(pMerged + (size_t)(iY + i) * (size_t)pShadow->iStride + (size_t)iX * 4u,
			       pSrc + (size_t)i * (size_t)iStride, (size_t)iCopyStride);
		tChoice = (xge_texture_choice_t){XGE_TEXTURE_STORAGE_RGBA8, 0};
		iRet = __xgeTextureNewGPU(pTexture, pMerged, tChoice, &iNew);
		if ( iRet == XGE_OK ) {
			pTexture->iBackendId = iNew;
			glDeleteTextures(1, &iOld);
			__xgeTextureStorageCommit(pTexture, tChoice);
			pShadow->bPackingDisabled = 1;
			xrtFree(pShadow->pPixels);
			pShadow->pPixels = pMerged;
		} else xrtFree(pMerged);
		return iRet;
	}
}

static int __xgeTextureCopyPlane(unsigned char* pDst, int iDstStride, int iWidth, int iHeight, const void* pSrcData, int iSrcStride)
{
	const unsigned char* pSrc;
	int i;

	if ( (pDst == NULL) || (pSrcData == NULL) || (iDstStride < iWidth) || (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iSrcStride <= 0 ) {
		iSrcStride = iWidth;
	}
	if ( iSrcStride < iWidth ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pSrc = (const unsigned char*)pSrcData;
	for ( i = 0; i < iHeight; i++ ) {
		memcpy(pDst + (i * iDstStride), pSrc + (i * iSrcStride), (size_t)iWidth);
	}
	return XGE_OK;
}

int xgeTextureUpdateYUV420P(xge_texture pTexture, const void* pY, int iStrideY, const void* pU, int iStrideU, const void* pV, int iStrideV)
{
	xge_texture_yuv420p_t* pYUV;
	int iRet;
	int i;

	if ( (pTexture == NULL) || (pY == NULL) || (pU == NULL) || (pV == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xgeTextureHasYUV420P(pTexture) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pYUV = (xge_texture_yuv420p_t*)pTexture->pBackend;
	iRet = __xgeTextureCopyPlane(pYUV->pPlane[0], pYUV->iPlaneStride[0], pYUV->iPlaneWidth[0], pYUV->iPlaneHeight[0], pY, iStrideY);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeTextureCopyPlane(pYUV->pPlane[1], pYUV->iPlaneStride[1], pYUV->iPlaneWidth[1], pYUV->iPlaneHeight[1], pU, iStrideU);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = __xgeTextureCopyPlane(pYUV->pPlane[2], pYUV->iPlaneStride[2], pYUV->iPlaneWidth[2], pYUV->iPlaneHeight[2], pV, iStrideV);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( (pTexture->iBackendId != 0) && (g_xge.bSokolRunning != 0) ) {
		if ( (glBindTexture == NULL) || (glTexSubImage2D == NULL) ) {
			return XGE_ERROR_GPU_FAILED;
		}
		if ( glPixelStorei != NULL ) {
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}
		for ( i = 0; i < 3; i++ ) {
			glBindTexture(GL_TEXTURE_2D, pYUV->iPlaneTexture[i]);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pYUV->iPlaneWidth[i], pYUV->iPlaneHeight[i], GL_RED, GL_UNSIGNED_BYTE, pYUV->pPlane[i]);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	return XGE_OK;
}

xge_sampler_t xgeTextureGetSampler(xge_texture pTexture)
{
	xge_sampler_t tSampler;

	if ( pTexture == NULL ) {
		return xgeSamplerDefault();
	}
	tSampler = pTexture->tSampler;
	if ( __xgeSamplerNormalize(&tSampler) != XGE_OK ) {
		return xgeSamplerDefault();
	}
	return tSampler;
}

int xgeTextureSetSampler(xge_texture pTexture, const xge_sampler_t* pSampler)
{
	xge_sampler_t tSampler;
	int iRet;

	if ( (pTexture == NULL) || (pSampler == NULL) || (pTexture->iRefCount <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tSampler = *pSampler;
	iRet = __xgeSamplerNormalize(&tSampler);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pTexture->tSampler = tSampler;
	if ( (pTexture->iBackendId != 0) && (g_xge.bSokolRunning != 0) ) {
		return __xgeTextureApplySampler(pTexture);
	}
	return XGE_OK;
}

int xgeTextureFallbackSetRGBA(int iWidth, int iHeight, const void* pPixels)
{
	int iRet;

	if ( (iWidth <= 0) || (iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeTextureFallbackClear();
	iRet = xgeTextureCreateRGBA(&g_xge.tFallbackTexture, iWidth, iHeight, pPixels);
	if ( iRet != XGE_OK ) {
		memset(&g_xge.tFallbackTexture, 0, sizeof(g_xge.tFallbackTexture));
	}
	return iRet;
}

int xgeTextureFallbackGet(xge_texture pTexture)
{
	xge_texture_shadow_t* pShadow;
	int iRet;

	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xge.tFallbackTexture.iRefCount <= 0 ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pShadow = (xge_texture_shadow_t*)g_xge.tFallbackTexture.pBackend;
	if ( (pShadow == NULL) || (pShadow->pPixels == NULL) ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	iRet = xgeTextureCreateRGBA(pTexture, g_xge.tFallbackTexture.iWidth, g_xge.tFallbackTexture.iHeight, pShadow->pPixels);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pTexture->iFlags |= XGE_TEXTURE_FALLBACK;
	return XGE_OK;
}

void xgeTextureFallbackClear(void)
{
	if ( g_xge.tFallbackTexture.iRefCount > 0 ) {
		xgeTextureFree(&g_xge.tFallbackTexture);
	}
	memset(&g_xge.tFallbackTexture, 0, sizeof(g_xge.tFallbackTexture));
}

static int __xgeTextureUploadQueueContains(xge_texture pTexture)
{
	xge_texture_upload_node_t* pNode;

	if ( pTexture == NULL ) {
		return 0;
	}
	pNode = g_xge.pTextureUploadHead;
	while ( pNode != NULL ) {
		if ( pNode->pTexture == pTexture ) {
			return 1;
		}
		pNode = pNode->pNext;
	}
	return 0;
}

int xgeTextureUploadQueue(xge_texture pTexture)
{
	xge_texture_upload_node_t* pNode;

	if ( (pTexture == NULL) || (pTexture->iRefCount <= 0) || (pTexture->iWidth <= 0) || (pTexture->iHeight <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pTexture->iFormat != XGE_PIXEL_RGBA8) && (pTexture->iFormat != XGE_PIXEL_YUV420P) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pTexture->iBackendId != 0 ) {
		return XGE_OK;
	}
	if ( (pTexture->iFormat == XGE_PIXEL_RGBA8) && !__xgeTextureHasShadow(pTexture) ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( (pTexture->iFormat == XGE_PIXEL_YUV420P) && !__xgeTextureHasYUV420P(pTexture) ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if ( (pTexture->iFlags & XGE_TEXTURE_UPLOAD_QUEUED) != 0 ) {
		if ( __xgeTextureUploadQueueContains(pTexture) ) {
			return XGE_OK;
		}
		pTexture->iFlags &= ~XGE_TEXTURE_UPLOAD_QUEUED;
	}
	pNode = (xge_texture_upload_node_t*)xrtMalloc(sizeof(*pNode));
	if ( pNode == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pNode, 0, sizeof(*pNode));
	pNode->pTexture = pTexture;
	if ( g_xge.pTextureUploadTail != NULL ) {
		g_xge.pTextureUploadTail->pNext = pNode;
	} else {
		g_xge.pTextureUploadHead = pNode;
	}
	g_xge.pTextureUploadTail = pNode;
	pTexture->iFlags |= XGE_TEXTURE_UPLOAD_QUEUED;
	return XGE_OK;
}

int xgeTextureUploadFlush(void)
{
	xge_texture_upload_node_t* pNode;
	xge_texture_upload_node_t* pNext;
	int iCount;

	if ( (g_xge.bSokolRunning == 0) && (g_xge.bRenderThreadGLCurrent == 0) ) {
		return 0;
	}
	iCount = 0;
	pNode = g_xge.pTextureUploadHead;
	g_xge.pTextureUploadHead = NULL;
	g_xge.pTextureUploadTail = NULL;
	while ( pNode != NULL ) {
		pNext = pNode->pNext;
		if ( pNode->pTexture != NULL ) {
			pNode->pTexture->iFlags &= ~XGE_TEXTURE_UPLOAD_QUEUED;
			if ( __xgeTextureUploadNow(pNode->pTexture) == XGE_OK ) {
				iCount++;
			}
		}
		xrtFree(pNode);
		pNode = pNext;
	}
	return iCount;
}

static void __xgeTextureUploadQueueFree(void)
{
	xge_texture_upload_node_t* pNode;
	xge_texture_upload_node_t* pNext;

	pNode = g_xge.pTextureUploadHead;
	g_xge.pTextureUploadHead = NULL;
	g_xge.pTextureUploadTail = NULL;
	while ( pNode != NULL ) {
		pNext = pNode->pNext;
		if ( pNode->pTexture != NULL ) {
			pNode->pTexture->iFlags &= ~XGE_TEXTURE_UPLOAD_QUEUED;
		}
		xrtFree(pNode);
		pNode = pNext;
	}
}

static void __xgeTextureUploadQueueRemove(xge_texture pTexture)
{
	xge_texture_upload_node_t* pNode;
	xge_texture_upload_node_t* pPrev;
	xge_texture_upload_node_t* pNext;

	if ( pTexture == NULL ) {
		return;
	}
	pPrev = NULL;
	pNode = g_xge.pTextureUploadHead;
	while ( pNode != NULL ) {
		pNext = pNode->pNext;
		if ( pNode->pTexture == pTexture ) {
			if ( pPrev != NULL ) {
				pPrev->pNext = pNext;
			} else {
				g_xge.pTextureUploadHead = pNext;
			}
			if ( g_xge.pTextureUploadTail == pNode ) {
				g_xge.pTextureUploadTail = pPrev;
			}
			xrtFree(pNode);
			pTexture->iFlags &= ~XGE_TEXTURE_UPLOAD_QUEUED;
			return;
		}
		pPrev = pNode;
		pNode = pNext;
	}
	pTexture->iFlags &= ~XGE_TEXTURE_UPLOAD_QUEUED;
}

int xgeTextureReadPixels(xge_texture pTexture, void* pPixels, int iStride)
{
	xge_texture_shadow_t* pShadow;
	unsigned char* pDst;
	unsigned char* pSrc;
	int i;
	int iCopyStride;

	if ( (pTexture == NULL) || (pPixels == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pTexture->iWidth <= 0) || (pTexture->iHeight <= 0) || (pTexture->iFormat != XGE_PIXEL_RGBA8) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iStride <= 0 ) {
		iStride = pTexture->iWidth * 4;
	}
	if ( iStride < (pTexture->iWidth * 4) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pTexture->pBackend == NULL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	pShadow = (xge_texture_shadow_t*)pTexture->pBackend;
	if ( (pShadow->pPixels == NULL) || (pShadow->iWidth != pTexture->iWidth) || (pShadow->iHeight != pTexture->iHeight) ) {
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pDst = (unsigned char*)pPixels;
	pSrc = pShadow->pPixels;
	iCopyStride = pTexture->iWidth * 4;
	for ( i = 0; i < pTexture->iHeight; i++ ) {
		memcpy(pDst + (i * iStride), pSrc + (i * pShadow->iStride), (size_t)iCopyStride);
	}
	return XGE_OK;
}

void xgeTextureFree(xge_texture pTexture)
{
	GLuint iTexture;
	uint64_t iBytes;

	if ( pTexture == NULL ) {
		return;
	}
	if ( pTexture->iRefCount > 1 ) {
		pTexture->iRefCount--;
		return;
	}
	iBytes = __xgeTextureMemoryBytes(pTexture);
	if ( iBytes > 0 ) {
		if ( g_xge.iTextureMemoryBytes >= iBytes ) {
			g_xge.iTextureMemoryBytes -= iBytes;
		} else {
			g_xge.iTextureMemoryBytes = 0;
		}
		if ( g_xge.iTextureCount > 0 ) {
			g_xge.iTextureCount--;
		}
	}
	__xgeTextureUploadQueueRemove(pTexture);
	if ( pTexture->iFormat == XGE_PIXEL_YUV420P ) {
		__xgeTextureYUV420PFree(pTexture);
	} else {
		iTexture = (GLuint)pTexture->iBackendId;
		if ( (iTexture != 0) && (glDeleteTextures != NULL) ) {
			glDeleteTextures(1, &iTexture);
		}
		__xgeTextureShadowFree(pTexture);
	}
	pTexture->iFlags &= ~XGE_TEXTURE_UPLOAD_QUEUED;
	memset(pTexture, 0, sizeof(*pTexture));
}
