/**
 * @file xge_surface.h
 * @brief XGE 画布管理模块
 */

#ifndef XGE_SURFACE_H
#define XGE_SURFACE_H

/* ============================================================================
 * 画布结构定义
 * ============================================================================ */

struct XgeSurface {
	int iWidth;
	int iHeight;
	GLuint hFramebuffer;
	GLuint hTexture;
	int bIsScreen;
};

/* ============================================================================
 * 画布 API 实现
 * ============================================================================ */

XgeSurface* xgeSurfaceCreate(int iWidth, int iHeight)
{
	XgeSurface* pSurface = (XgeSurface*)malloc(sizeof(XgeSurface));
	if ( !pSurface ) {
		return NULL;
	}
	
	pSurface->iWidth = iWidth;
	pSurface->iHeight = iHeight;
	pSurface->bIsScreen = 0;
	
	/* 创建帧缓冲 */
	glGenFramebuffers(1, &pSurface->hFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, pSurface->hFramebuffer);
	
	/* 创建纹理 */
	glGenTextures(1, &pSurface->hTexture);
	glBindTexture(GL_TEXTURE_2D, pSurface->hTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, iWidth, iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	/* 绑定纹理到帧缓冲 */
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pSurface->hTexture, 0);
	
	/* 检查帧缓冲完整性 */
	if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ) {
		XGE_ERROR("Failed to create framebuffer");
		glDeleteFramebuffers(1, &pSurface->hFramebuffer);
		glDeleteTextures(1, &pSurface->hTexture);
		free(pSurface);
		return NULL;
	}
	
	/* 恢复默认帧缓冲 */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	return pSurface;
}

void xgeSurfaceFree(XgeSurface* pSurface)
{
	if ( !pSurface ) {
		return;
	}
	
	if ( !pSurface->bIsScreen ) {
		if ( pSurface->hFramebuffer ) {
			glDeleteFramebuffers(1, &pSurface->hFramebuffer);
		}
		if ( pSurface->hTexture ) {
			glDeleteTextures(1, &pSurface->hTexture);
		}
	}
	
	free(pSurface);
}

void xgeSetTarget(XgeSurface* pTarget)
{
	g_xge.pCurrentTarget = pTarget;
	
	if ( pTarget && !pTarget->bIsScreen ) {
		glBindFramebuffer(GL_FRAMEBUFFER, pTarget->hFramebuffer);
		glViewport(0, 0, pTarget->iWidth, pTarget->iHeight);
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, g_xge.iWidth, g_xge.iHeight);
	}
}

XgeSurface* xgeGetTarget(void)
{
	return g_xge.pCurrentTarget;
}

void xgeClear(unsigned int uColor)
{
	float fR = XGE_GETR(uColor) / 255.0f;
	float fG = XGE_GETG(uColor) / 255.0f;
	float fB = XGE_GETB(uColor) / 255.0f;
	float fA = XGE_GETA(uColor) / 255.0f;
	
	glClearColor(fR, fG, fB, fA);
	glClear(GL_COLOR_BUFFER_BIT);
}

#endif /* XGE_SURFACE_H */
