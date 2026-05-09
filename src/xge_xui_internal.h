#ifndef XGE_XUI_INTERNAL_H
#define XGE_XUI_INTERNAL_H

/*
 * Private XUI implementation header.
 *
 * XGE currently builds as a single translation unit, so private helpers with
 * internal linkage are shared by include order. This header marks the internal
 * boundary and is the home for small shared private definitions as XUI controls
 * are split into per-control source files.
 */

#define XGE_XUI_INTERNAL 1

typedef void (*xge_xui_render_cache_proc)(xge_rect_t tRect, void* pUser);

static int __xgeXuiRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < (tRect.fX + tRect.fW)) && (fY < (tRect.fY + tRect.fH));
}

static float __xgeXuiClampFloat(float fValue, float fMin, float fMax)
{
	if ( fValue < fMin ) {
		return fMin;
	}
	if ( fValue > fMax ) {
		return fMax;
	}
	return fValue;
}

static void __xgeXuiWidgetApplyRolePolicy(xge_xui_widget pWidget, int iRole, int bFocusable, int iImeMode)
{
	if ( pWidget == NULL ) {
		return;
	}
	xgeXuiWidgetSetRole(pWidget, iRole);
	xgeXuiWidgetSetFocusable(pWidget, bFocusable);
	xgeXuiWidgetSetImeMode(pWidget, iImeMode);
}

static void __xgeXuiControlWidgetInit(xge_xui_widget pWidget, int bFocusable)
{
	__xgeXuiWidgetApplyRolePolicy(pWidget, XGE_XUI_WIDGET_ROLE_CONTROL, bFocusable, XGE_XUI_IME_DISABLED);
}

static void __xgeXuiTextControlWidgetInit(xge_xui_widget pWidget)
{
	__xgeXuiWidgetApplyRolePolicy(pWidget, XGE_XUI_WIDGET_ROLE_CONTROL, 1, XGE_XUI_IME_ENABLED);
}

static void __xgeXuiViewportWidgetInit(xge_xui_widget pWidget, int bFocusable)
{
	__xgeXuiWidgetApplyRolePolicy(pWidget, XGE_XUI_WIDGET_ROLE_VIEWPORT, bFocusable, XGE_XUI_IME_DISABLED);
}

static void __xgeXuiOverlayWidgetInit(xge_xui_widget pWidget, int bFocusable)
{
	__xgeXuiWidgetApplyRolePolicy(pWidget, XGE_XUI_WIDGET_ROLE_OVERLAY, bFocusable, XGE_XUI_IME_DISABLED);
}

static int __xgeXuiRoundCacheSize(float fValue)
{
	int iValue;

	if ( fValue <= 0.0f ) {
		return 0;
	}
	if ( fValue > 32767.0f ) {
		return 0;
	}
	iValue = (int)(fValue + 0.999f);
	return (iValue > 0) ? iValue : 1;
}

static void __xgeXuiRenderCacheInit(xge_xui_render_cache pCache)
{
	if ( pCache == NULL ) {
		return;
	}
	memset(pCache, 0, sizeof(*pCache));
	pCache->bDirty = 1;
}

static void __xgeXuiRenderCacheUnit(xge_xui_render_cache pCache)
{
	if ( pCache == NULL ) {
		return;
	}
	xgeRenderTargetFree(&pCache->tTarget);
	memset(pCache, 0, sizeof(*pCache));
}

static void __xgeXuiRenderCacheInvalidate(xge_xui_render_cache pCache)
{
	if ( pCache == NULL ) {
		return;
	}
	pCache->bDirty = 1;
}

static xge_texture __xgeXuiRenderCacheTexture(xge_xui_render_cache pCache)
{
	if ( (pCache == NULL) || (pCache->bValid == 0) || (pCache->bDirty != 0) ) {
		return NULL;
	}
	return xgeRenderTargetTexture(&pCache->tTarget);
}

static int __xgeXuiRenderCacheRebuild(xge_xui_render_cache pCache, xge_rect_t tRect, float fDipScale, xge_xui_render_cache_proc procPaint, void* pUser)
{
	xge_pass_t tPass;
	xge_rect_t tOldClip;
	int iWidth;
	int iHeight;
	int iRet;
	int bOldClip;

	if ( (pCache == NULL) || (procPaint == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iWidth = __xgeXuiRoundCacheSize(tRect.fW);
	iHeight = __xgeXuiRoundCacheSize(tRect.fH);
	if ( (iWidth <= 0) || (iHeight <= 0) ) {
		pCache->iLastError = XGE_ERROR_INVALID_ARGUMENT;
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( g_xge.bSokolRunning == 0 ) {
		pCache->bValid = 0;
		pCache->iLastError = XGE_ERROR_NOT_INITIALIZED;
		return XGE_ERROR_NOT_INITIALIZED;
	}
	iRet = xgeFlush();
	if ( iRet != XGE_OK ) {
		pCache->bValid = 0;
		pCache->iLastError = iRet;
		return iRet;
	}
	tOldClip = xgeClipGet();
	bOldClip = (tOldClip.fW > 0.0f) && (tOldClip.fH > 0.0f);
	if ( (pCache->iWidth != iWidth) || (pCache->iHeight != iHeight) || (pCache->tTarget.iWidth != iWidth) || (pCache->tTarget.iHeight != iHeight) ) {
		xgeRenderTargetFree(&pCache->tTarget);
		iRet = xgeRenderTargetCreate(&pCache->tTarget, iWidth, iHeight);
		if ( iRet != XGE_OK ) {
			if ( bOldClip ) {
				xgeClipSet(tOldClip);
			} else {
				xgeClipClear();
			}
			pCache->bValid = 0;
			pCache->iLastError = iRet;
			return iRet;
		}
		pCache->iWidth = iWidth;
		pCache->iHeight = iHeight;
	}
	xgePassInit(&tPass, &pCache->tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(0, 0, 0, 0));
	iRet = xgePassBegin(&tPass);
	if ( iRet != XGE_OK ) {
		if ( bOldClip ) {
			xgeClipSet(tOldClip);
		} else {
			xgeClipClear();
		}
		pCache->bValid = 0;
		pCache->iLastError = iRet;
		return iRet;
	}
	xgeClipClear();
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = (float)iWidth;
	tRect.fH = (float)iHeight;
	procPaint(tRect, pUser);
	xgeFlush();
	iRet = xgePassEnd(&tPass);
	if ( bOldClip ) {
		xgeClipSet(tOldClip);
	} else {
		xgeClipClear();
	}
	if ( iRet != XGE_OK ) {
		pCache->bValid = 0;
		pCache->iLastError = iRet;
		return iRet;
	}
	pCache->bValid = 1;
	pCache->bDirty = 0;
	pCache->fDipScale = fDipScale;
	pCache->iLastError = XGE_OK;
	return XGE_OK;
}

static xge_texture __xgeXuiRenderCacheEnsure(xge_xui_render_cache pCache, xge_rect_t tRect, float fDipScale, xge_xui_render_cache_proc procPaint, void* pUser)
{
	int iWidth;
	int iHeight;

	if ( pCache == NULL ) {
		return NULL;
	}
	iWidth = __xgeXuiRoundCacheSize(tRect.fW);
	iHeight = __xgeXuiRoundCacheSize(tRect.fH);
	if ( (pCache->bValid != 0) && (pCache->bDirty == 0) && (pCache->iWidth == iWidth) && (pCache->iHeight == iHeight) && (pCache->fDipScale == fDipScale) ) {
		return xgeRenderTargetTexture(&pCache->tTarget);
	}
	if ( __xgeXuiRenderCacheRebuild(pCache, tRect, fDipScale, procPaint, pUser) != XGE_OK ) {
		return NULL;
	}
	return __xgeXuiRenderCacheTexture(pCache);
}

#endif
