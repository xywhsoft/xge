// XUI 孵化期基础模块：context、widget tree、dirty layout 和 paint 统计

static const xge_xui_host_t* g_xgeXuiActiveHost;
static const xge_xui_host_v2_t* g_xgeXuiActiveHostV2;
static xge_xui_context g_xgeXuiActiveContext;
static xge_xui_widget g_pXgeXuiActivePaintWidget;
static float g_fXgeXuiActiveDipScale = 1.0f;

typedef struct xge_xui_host_v2_xge_render_target_t {
	xge_render_target_t tTarget;
	xge_pass_t tPass;
	int bPassActive;
} xge_xui_host_v2_xge_render_target_t;

static int __xgeXuiPaintClipPush(xge_xui_context pContext, xge_rect_t tRect);
static void __xgeXuiPaintClipPop(xge_xui_context pContext);
static xge_rect_t __xgeXuiRectIntersection(xge_rect_t tA, xge_rect_t tB);
static int __xgeXuiRectSame(xge_rect_t tA, xge_rect_t tB);
static int __xgeXuiPaintClipIntersects(xge_xui_context pContext, xge_rect_t tRect);

static void __xgeXuiHostV2XgeDrawRect(xge_rect_t tRect, uint32_t iColor, void* pUser)
{
	(void)pUser;
	xgeShapeRectFillPx(tRect, iColor);
}

static void __xgeXuiHostV2XgeDrawLine(float fX1, float fY1, float fX2, float fY2, float fWidth, uint32_t iColor, void* pUser)
{
	(void)pUser;
	xgeShapeLinePx(fX1, fY1, fX2, fY2, fWidth, iColor);
}

static void __xgeXuiHostV2XgeDrawTriangle(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor, void* pUser)
{
	(void)pUser;
	xgeShapeTriangleFillPx(tA, tB, tC, iColor);
}

static void __xgeXuiHostV2XgeDrawCircle(float fX, float fY, float fRadius, uint32_t iColor, void* pUser)
{
	(void)pUser;
	xgeShapeCircleFillPx(fX, fY, fRadius, iColor);
}

static void __xgeXuiHostV2XgeDrawCircleStroke(float fX, float fY, float fRadius, float fWidth, uint32_t iColor, void* pUser)
{
	(void)pUser;
	xgeShapeCircleStrokePx(fX, fY, fRadius, fWidth, iColor);
}

static void __xgeXuiHostV2XgeDrawTexture(xui_texture pTexture, xge_rect_t tSrc, xge_rect_t tDst, uint32_t iColor, uint32_t iFlags, void* pUser)
{
	xge_draw_t tDraw;

	(void)pUser;
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = (xge_texture)pTexture;
	tDraw.tSrc = tSrc;
	tDraw.tDst = tDst;
	tDraw.iColor = iColor;
	tDraw.iFlags = iFlags;
	xgeDrawEx(&tDraw);
}

static void __xgeXuiHostV2XgeDrawText(xui_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags, void* pUser)
{
	(void)pUser;
	xgeTextDrawRect((xge_font)pFont, sText, tRect, iColor, iFlags);
}

static xge_vec2_t __xgeXuiHostV2XgeMeasureText(xui_font pFont, const char* sText, uint32_t iFlags, void* pUser)
{
	(void)iFlags;
	(void)pUser;
	return xgeTextMeasure((xge_font)pFont, sText);
}

static int __xgeXuiHostV2XgeFontMetrics(xui_font pFont, xui_font_metrics_t* pMetrics, void* pUser)
{
	xge_font pXgeFont;

	(void)pUser;
	if ( (pFont == NULL) || (pMetrics == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pXgeFont = (xge_font)pFont;
	pMetrics->fSize = pXgeFont->fSize;
	pMetrics->fAscent = pXgeFont->fAscent;
	pMetrics->fDescent = pXgeFont->fDescent;
	pMetrics->fLineGap = pXgeFont->fLineGap;
	pMetrics->fLineHeight = pXgeFont->fLineHeight;
	return XGE_OK;
}

static void __xgeXuiHostV2XgeClipPush(xge_rect_t tRect, void* pUser)
{
	(void)pUser;
	xgeClipSet(tRect);
}

static void __xgeXuiHostV2XgeClipClear(void* pUser)
{
	(void)pUser;
	xgeClipClear();
}

static int __xgeXuiHostV2XgeClipGet(xge_rect_t* pRect, int* pEnabled, void* pUser)
{
	xge_rect_t tRect;

	(void)pUser;
	tRect = xgeClipGet();
	if ( pRect != NULL ) {
		*pRect = tRect;
	}
	if ( pEnabled != NULL ) {
		*pEnabled = (tRect.fW > 0.0f) && (tRect.fH > 0.0f);
	}
	return XGE_OK;
}

static int __xgeXuiHostV2XgeTextureCreateRGBA(xui_texture* pTexture, int iWidth, int iHeight, const void* pPixels, int iStride, uint32_t iFlags, void* pUser)
{
	xge_texture pXgeTexture;
	int iRet;

	(void)iFlags;
	(void)pUser;
	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pXgeTexture = (xge_texture)xrtMalloc(sizeof(*pXgeTexture));
	if ( pXgeTexture == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pXgeTexture, 0, sizeof(*pXgeTexture));
	iRet = xgeTextureCreateRGBA(pXgeTexture, iWidth, iHeight, pPixels);
	(void)iStride;
	if ( iRet != XGE_OK ) {
		xrtFree(pXgeTexture);
		return iRet;
	}
	*pTexture = (xui_texture)pXgeTexture;
	return XGE_OK;
}

static int __xgeXuiHostV2XgeTextureCreateMemory(xui_texture* pTexture, const void* pData, int iSize, uint32_t iFlags, void* pUser)
{
	xge_texture pXgeTexture;
	int iRet;

	(void)pUser;
	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pXgeTexture = (xge_texture)xrtMalloc(sizeof(*pXgeTexture));
	if ( pXgeTexture == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pXgeTexture, 0, sizeof(*pXgeTexture));
	iRet = xgeTextureLoadMemoryEx(pXgeTexture, pData, iSize, iFlags);
	if ( iRet != XGE_OK ) {
		xrtFree(pXgeTexture);
		return iRet;
	}
	*pTexture = (xui_texture)pXgeTexture;
	return XGE_OK;
}

static int __xgeXuiHostV2XgeTextureCreateFile(xui_texture* pTexture, const char* sPath, uint32_t iFlags, void* pUser)
{
	xge_texture pXgeTexture;
	int iRet;

	(void)pUser;
	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pXgeTexture = (xge_texture)xrtMalloc(sizeof(*pXgeTexture));
	if ( pXgeTexture == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pXgeTexture, 0, sizeof(*pXgeTexture));
	iRet = xgeTextureLoadEx(pXgeTexture, sPath, iFlags);
	if ( iRet != XGE_OK ) {
		xrtFree(pXgeTexture);
		return iRet;
	}
	*pTexture = (xui_texture)pXgeTexture;
	return XGE_OK;
}

static int __xgeXuiHostV2XgeTextureUpdateRGBA(xui_texture pTexture, int iX, int iY, int iWidth, int iHeight, const void* pPixels, int iStride, void* pUser)
{
	(void)pUser;
	return xgeTextureUpdateRGBA((xge_texture)pTexture, iX, iY, iWidth, iHeight, pPixels, iStride);
}

static int __xgeXuiHostV2XgeTextureGetDesc(xui_texture pTexture, xui_texture_desc_t* pDesc, void* pUser)
{
	xge_texture pXgeTexture;

	(void)pUser;
	if ( (pTexture == NULL) || (pDesc == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pXgeTexture = (xge_texture)pTexture;
	pDesc->iWidth = pXgeTexture->iWidth;
	pDesc->iHeight = pXgeTexture->iHeight;
	pDesc->iFormat = pXgeTexture->iFormat;
	pDesc->iFlags = 0;
	return XGE_OK;
}

static void __xgeXuiHostV2XgeTextureDestroy(xui_texture pTexture, void* pUser)
{
	(void)pUser;
	if ( pTexture != NULL ) {
		xgeTextureFree((xge_texture)pTexture);
		xrtFree(pTexture);
	}
}

static int __xgeXuiHostV2XgeRenderTargetCreate(xui_render_target* pTarget, int iWidth, int iHeight, uint32_t iFlags, void* pUser)
{
	xge_xui_host_v2_xge_render_target_t* pXgeTarget;
	int iRet;

	(void)iFlags;
	(void)pUser;
	if ( pTarget == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pTarget = NULL;
	pXgeTarget = (xge_xui_host_v2_xge_render_target_t*)xrtMalloc(sizeof(*pXgeTarget));
	if ( pXgeTarget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pXgeTarget, 0, sizeof(*pXgeTarget));
	iRet = xgeRenderTargetCreate(&pXgeTarget->tTarget, iWidth, iHeight);
	if ( iRet != XGE_OK ) {
		xrtFree(pXgeTarget);
		return iRet;
	}
	*pTarget = (xui_render_target)pXgeTarget;
	return XGE_OK;
}

static int __xgeXuiHostV2XgeRenderTargetResize(xui_render_target pTarget, int iWidth, int iHeight, void* pUser)
{
	xge_xui_host_v2_xge_render_target_t* pXgeTarget;

	(void)pUser;
	if ( pTarget == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pXgeTarget = (xge_xui_host_v2_xge_render_target_t*)pTarget;
	if ( (pXgeTarget->tTarget.iWidth == iWidth) && (pXgeTarget->tTarget.iHeight == iHeight) ) {
		return XGE_OK;
	}
	return xgeRenderTargetResize(&pXgeTarget->tTarget, iWidth, iHeight);
}

static int __xgeXuiHostV2XgeRenderTargetBegin(xui_render_target pTarget, void* pUser)
{
	xge_xui_host_v2_xge_render_target_t* pXgeTarget;
	int iRet;

	(void)pUser;
	if ( pTarget == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pXgeTarget = (xge_xui_host_v2_xge_render_target_t*)pTarget;
	if ( pXgeTarget->bPassActive != 0 ) {
		return XGE_ERROR_ALREADY_INITIALIZED;
	}
	xgePassInit(&pXgeTarget->tPass, &pXgeTarget->tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(0, 0, 0, 0));
	iRet = xgePassBegin(&pXgeTarget->tPass);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pXgeTarget->bPassActive = 1;
	return XGE_OK;
}

static void __xgeXuiHostV2XgeRenderTargetEnd(xui_render_target pTarget, void* pUser)
{
	xge_xui_host_v2_xge_render_target_t* pXgeTarget;

	(void)pUser;
	if ( pTarget == NULL ) {
		return;
	}
	pXgeTarget = (xge_xui_host_v2_xge_render_target_t*)pTarget;
	if ( pXgeTarget->bPassActive != 0 ) {
		(void)xgePassEnd(&pXgeTarget->tPass);
		pXgeTarget->bPassActive = 0;
	}
}

static xui_texture __xgeXuiHostV2XgeRenderTargetTexture(xui_render_target pTarget, void* pUser)
{
	xge_xui_host_v2_xge_render_target_t* pXgeTarget;

	(void)pUser;
	if ( pTarget == NULL ) {
		return NULL;
	}
	pXgeTarget = (xge_xui_host_v2_xge_render_target_t*)pTarget;
	return (xui_texture)xgeRenderTargetTexture(&pXgeTarget->tTarget);
}

static void __xgeXuiHostV2XgeRenderTargetDestroy(xui_render_target pTarget, void* pUser)
{
	xge_xui_host_v2_xge_render_target_t* pXgeTarget;

	(void)pUser;
	if ( pTarget == NULL ) {
		return;
	}
	pXgeTarget = (xge_xui_host_v2_xge_render_target_t*)pTarget;
	if ( pXgeTarget->bPassActive != 0 ) {
		(void)xgePassEnd(&pXgeTarget->tPass);
	}
	xgeRenderTargetFree(&pXgeTarget->tTarget);
	xrtFree(pXgeTarget);
}

static int __xgeXuiHostV2XgeFontCreateFile(xui_font* pFont, const char* sPath, float fSize, uint32_t iFlags, void* pUser)
{
	xge_font pXgeFont;
	int iRet;

	(void)iFlags;
	(void)pUser;
	if ( pFont == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pXgeFont = (xge_font)xrtMalloc(sizeof(*pXgeFont));
	if ( pXgeFont == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pXgeFont, 0, sizeof(*pXgeFont));
	iRet = xgeFontLoad(pXgeFont, sPath, fSize);
	if ( iRet != XGE_OK ) {
		xrtFree(pXgeFont);
		return iRet;
	}
	*pFont = (xui_font)pXgeFont;
	return XGE_OK;
}

static int __xgeXuiHostV2XgeFontCreateMemory(xui_font* pFont, const void* pData, int iSize, float fSize, uint32_t iFlags, void* pUser)
{
	xge_font pXgeFont;
	int iRet;

	(void)iFlags;
	(void)pUser;
	if ( pFont == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pXgeFont = (xge_font)xrtMalloc(sizeof(*pXgeFont));
	if ( pXgeFont == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pXgeFont, 0, sizeof(*pXgeFont));
	iRet = xgeFontLoadMemory(pXgeFont, pData, iSize, fSize);
	if ( iRet != XGE_OK ) {
		xrtFree(pXgeFont);
		return iRet;
	}
	*pFont = (xui_font)pXgeFont;
	return XGE_OK;
}

static void __xgeXuiHostV2XgeFontDestroy(xui_font pFont, void* pUser)
{
	(void)pUser;
	if ( pFont != NULL ) {
		xgeFontFree((xge_font)pFont);
		xrtFree(pFont);
	}
}

static void __xgeXuiHostV2XgeClipboardSetText(const char* sText, void* pUser)
{
	(void)pUser;
	xgeClipboardSetText(sText);
}

static const char* __xgeXuiHostV2XgeClipboardGetText(void* pUser)
{
	(void)pUser;
	return xgeClipboardGetText();
}

static double __xgeXuiHostV2XgeGetTime(void* pUser)
{
	(void)pUser;
	return xgeTimer();
}

static void __xgeXuiHostV2XgeGetViewportSize(float* pWidth, float* pHeight, void* pUser)
{
	(void)pUser;
	if ( pWidth != NULL ) {
		*pWidth = (float)xgeGetWidth();
	}
	if ( pHeight != NULL ) {
		*pHeight = (float)xgeGetHeight();
	}
}

static void __xgeXuiHostV2XgeFlush(void* pUser)
{
	(void)pUser;
	(void)xgeFlush();
}

static const xge_xui_host_v2_t g_xgeXuiHostV2Null = {
	.iSize = sizeof(xge_xui_host_v2_t),
	.iVersion = 2u
};

static const xge_xui_host_v2_t g_xgeXuiHostV2Xge = {
	.iSize = sizeof(xge_xui_host_v2_t),
	.iVersion = 2u,
	.draw_rect = __xgeXuiHostV2XgeDrawRect,
	.draw_line = __xgeXuiHostV2XgeDrawLine,
	.draw_triangle = __xgeXuiHostV2XgeDrawTriangle,
	.draw_circle = __xgeXuiHostV2XgeDrawCircle,
	.draw_circle_stroke = __xgeXuiHostV2XgeDrawCircleStroke,
	.draw_texture = __xgeXuiHostV2XgeDrawTexture,
	.draw_text = __xgeXuiHostV2XgeDrawText,
	.measure_text = __xgeXuiHostV2XgeMeasureText,
	.font_metrics = __xgeXuiHostV2XgeFontMetrics,
	.clip_push = __xgeXuiHostV2XgeClipPush,
	.clip_clear = __xgeXuiHostV2XgeClipClear,
	.clip_get = __xgeXuiHostV2XgeClipGet,
	.texture_create_rgba = __xgeXuiHostV2XgeTextureCreateRGBA,
	.texture_create_memory = __xgeXuiHostV2XgeTextureCreateMemory,
	.texture_create_file = __xgeXuiHostV2XgeTextureCreateFile,
	.texture_update_rgba = __xgeXuiHostV2XgeTextureUpdateRGBA,
	.texture_get_desc = __xgeXuiHostV2XgeTextureGetDesc,
	.texture_destroy = __xgeXuiHostV2XgeTextureDestroy,
	.render_target_create = __xgeXuiHostV2XgeRenderTargetCreate,
	.render_target_resize = __xgeXuiHostV2XgeRenderTargetResize,
	.render_target_begin = __xgeXuiHostV2XgeRenderTargetBegin,
	.render_target_end = __xgeXuiHostV2XgeRenderTargetEnd,
	.render_target_texture = __xgeXuiHostV2XgeRenderTargetTexture,
	.render_target_destroy = __xgeXuiHostV2XgeRenderTargetDestroy,
	.font_create_file = __xgeXuiHostV2XgeFontCreateFile,
	.font_create_memory = __xgeXuiHostV2XgeFontCreateMemory,
	.font_destroy = __xgeXuiHostV2XgeFontDestroy,
	.clipboard_set_text = __xgeXuiHostV2XgeClipboardSetText,
	.clipboard_get_text = __xgeXuiHostV2XgeClipboardGetText,
	.get_time = __xgeXuiHostV2XgeGetTime,
	.get_viewport_size = __xgeXuiHostV2XgeGetViewportSize,
	.flush = __xgeXuiHostV2XgeFlush
};

const xge_xui_host_v2_t* xgeXuiHostV2Null(void)
{
	return &g_xgeXuiHostV2Null;
}

const xge_xui_host_v2_t* xgeXuiHostV2Xge(void)
{
	return &g_xgeXuiHostV2Xge;
}

static void __xgeXuiDefaultDrawRect(xge_rect_t tRect, uint32_t iColor, void* pUser)
{
	(void)pUser;
	xgeShapeRectFillPx(tRect, iColor);
}

static void __xgeXuiDefaultDrawImage(const xge_draw_t* pDraw, void* pUser)
{
	(void)pUser;
	xgeDrawEx(pDraw);
}

static void __xgeXuiDefaultDrawTextRect(xui_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags, void* pUser)
{
	(void)pUser;
	xgeTextDrawRect(pFont, sText, tRect, iColor, iFlags);
}

static xge_vec2_t __xgeXuiDefaultMeasureText(xui_font pFont, const char* sText, void* pUser)
{
	(void)pUser;
	return xgeTextMeasure(pFont, sText);
}

static void __xgeXuiDefaultClipSet(xge_rect_t tRect, void* pUser)
{
	(void)pUser;
	xgeClipSet(tRect);
}

static void __xgeXuiDefaultClipClear(void* pUser)
{
	(void)pUser;
	xgeClipClear();
}

static const xge_xui_host_t g_xgeXuiDefaultHost = {
	__xgeXuiDefaultDrawRect,
	__xgeXuiDefaultDrawImage,
	__xgeXuiDefaultDrawTextRect,
	__xgeXuiDefaultMeasureText,
	__xgeXuiDefaultClipSet,
	__xgeXuiDefaultClipClear,
	NULL,
	NULL
};

static const xge_xui_host_t* __xgeXuiHostGet(void)
{
	return (g_xgeXuiActiveHost != NULL) ? g_xgeXuiActiveHost : &g_xgeXuiDefaultHost;
}

static const xge_xui_host_v2_t* __xgeXuiHostV2Get(void)
{
	return g_xgeXuiActiveHostV2;
}

static const xge_xui_host_v2_t* __xgeXuiHostV2ForContext(xge_xui_context pContext)
{
	if ( pContext != NULL && pContext->bInitialized != 0 && pContext->pHostV2 != NULL ) {
		return pContext->pHostV2;
	}
	if ( g_xgeXuiActiveHostV2 != NULL ) {
		return g_xgeXuiActiveHostV2;
	}
	return &g_xgeXuiHostV2Xge;
}

static int __xgeXuiHostFontCreateFile(xge_xui_context pContext, const char* sPath, float fSize, xui_font* pFont)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( pFont == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pFont = NULL;
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 == NULL || pHostV2->font_create_file == NULL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	return pHostV2->font_create_file(pFont, sPath, fSize, 0, pHostV2->pUser);
}

static int __xgeXuiHostFontCreateMemory(xge_xui_context pContext, const void* pData, int iSize, float fSize, xui_font* pFont)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( pFont == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pFont = NULL;
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 == NULL || pHostV2->font_create_memory == NULL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	return pHostV2->font_create_memory(pFont, pData, iSize, fSize, 0, pHostV2->pUser);
}

static void __xgeXuiHostFontDestroy(xge_xui_context pContext, xui_font pFont)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( pFont == NULL ) {
		return;
	}
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 != NULL && pHostV2->font_destroy != NULL ) {
		pHostV2->font_destroy(pFont, pHostV2->pUser);
	}
}

static int __xgeXuiHostFontMetrics(xui_font pFont, xui_font_metrics_t* pMetrics)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( (pFont == NULL) || (pMetrics == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pHostV2 = __xgeXuiHostV2Get();
	if ( pHostV2 == NULL ) {
		pHostV2 = &g_xgeXuiHostV2Xge;
	}
	if ( pHostV2->font_metrics == NULL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	return pHostV2->font_metrics(pFont, pMetrics, pHostV2->pUser);
}

static int __xgeXuiHostTextureCreateRGBA(xge_xui_context pContext, int iWidth, int iHeight, const void* pPixels, int iStride, uint32_t iFlags, xui_texture* pTexture)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pTexture = NULL;
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 == NULL || pHostV2->texture_create_rgba == NULL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	return pHostV2->texture_create_rgba(pTexture, iWidth, iHeight, pPixels, iStride, iFlags, pHostV2->pUser);
}

static int __xgeXuiHostTextureCreateMemory(xge_xui_context pContext, const void* pData, int iSize, uint32_t iFlags, xui_texture* pTexture)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pTexture = NULL;
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 == NULL || pHostV2->texture_create_memory == NULL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	return pHostV2->texture_create_memory(pTexture, pData, iSize, iFlags, pHostV2->pUser);
}

static int __xgeXuiHostTextureCreateFile(xge_xui_context pContext, const char* sPath, uint32_t iFlags, xui_texture* pTexture)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pTexture = NULL;
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 == NULL || pHostV2->texture_create_file == NULL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	return pHostV2->texture_create_file(pTexture, sPath, iFlags, pHostV2->pUser);
}

static int __xgeXuiHostTextureUpdateRGBA(xge_xui_context pContext, xui_texture pTexture, int iX, int iY, int iWidth, int iHeight, const void* pPixels, int iStride)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( pTexture == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 == NULL || pHostV2->texture_update_rgba == NULL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	return pHostV2->texture_update_rgba(pTexture, iX, iY, iWidth, iHeight, pPixels, iStride, pHostV2->pUser);
}

static int __xgeXuiHostTextureGetDesc(xge_xui_context pContext, xui_texture pTexture, xui_texture_desc_t* pDesc)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( (pTexture == NULL) || (pDesc == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 == NULL || pHostV2->texture_get_desc == NULL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	return pHostV2->texture_get_desc(pTexture, pDesc, pHostV2->pUser);
}

static int __xgeXuiHostRenderTargetCreate(xge_xui_context pContext, int iWidth, int iHeight, uint32_t iFlags, xui_render_target* pTarget)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( pTarget == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pTarget = NULL;
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 == NULL || pHostV2->render_target_create == NULL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	return pHostV2->render_target_create(pTarget, iWidth, iHeight, iFlags, pHostV2->pUser);
}

static int __xgeXuiHostRenderTargetResize(xge_xui_context pContext, xui_render_target pTarget, int iWidth, int iHeight)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( pTarget == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 == NULL || pHostV2->render_target_resize == NULL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	return pHostV2->render_target_resize(pTarget, iWidth, iHeight, pHostV2->pUser);
}

static int __xgeXuiHostRenderTargetBegin(xge_xui_context pContext, xui_render_target pTarget)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( pTarget == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 == NULL || pHostV2->render_target_begin == NULL ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	return pHostV2->render_target_begin(pTarget, pHostV2->pUser);
}

static void __xgeXuiHostRenderTargetEnd(xge_xui_context pContext, xui_render_target pTarget)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( pTarget == NULL ) {
		return;
	}
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 != NULL && pHostV2->render_target_end != NULL ) {
		pHostV2->render_target_end(pTarget, pHostV2->pUser);
	}
}

static xui_texture __xgeXuiHostRenderTargetTexture(xge_xui_context pContext, xui_render_target pTarget)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( pTarget == NULL ) {
		return NULL;
	}
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 != NULL && pHostV2->render_target_texture != NULL ) {
		return pHostV2->render_target_texture(pTarget, pHostV2->pUser);
	}
	return NULL;
}

static void __xgeXuiHostRenderTargetDestroy(xge_xui_context pContext, xui_render_target pTarget)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( pTarget == NULL ) {
		return;
	}
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 != NULL && pHostV2->render_target_destroy != NULL ) {
		pHostV2->render_target_destroy(pTarget, pHostV2->pUser);
	}
}

static void __xgeXuiHostTextureDestroy(xge_xui_context pContext, xui_texture pTexture)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( pTexture == NULL ) {
		return;
	}
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 != NULL && pHostV2->texture_destroy != NULL ) {
		pHostV2->texture_destroy(pTexture, pHostV2->pUser);
	}
}

static int __xgeXuiHostClipGet(xge_rect_t* pRect, int* pEnabled)
{
	const xge_xui_host_v2_t* pHostV2;

	pHostV2 = __xgeXuiHostV2Get();
	if ( pHostV2 == NULL ) {
		pHostV2 = &g_xgeXuiHostV2Xge;
	}
	if ( pHostV2->clip_get == NULL ) {
		if ( pRect != NULL ) {
			memset(pRect, 0, sizeof(*pRect));
		}
		if ( pEnabled != NULL ) {
			*pEnabled = 0;
		}
		return XGE_ERROR_UNSUPPORTED;
	}
	return pHostV2->clip_get(pRect, pEnabled, pHostV2->pUser);
}

static double __xgeXuiHostGetTime(xge_xui_context pContext)
{
	const xge_xui_host_v2_t* pHostV2;

	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 != NULL && pHostV2->get_time != NULL ) {
		return pHostV2->get_time(pHostV2->pUser);
	}
	return 0.0;
}

static xge_rect_t __xgeXuiHostGetViewportRect(xge_xui_context pContext)
{
	const xge_xui_host_v2_t* pHostV2;
	xge_rect_t tRect;

	tRect = (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 != NULL && pHostV2->get_viewport_size != NULL ) {
		pHostV2->get_viewport_size(&tRect.fW, &tRect.fH, pHostV2->pUser);
	}
	return tRect;
}

static void __xgeXuiHostFlush(xge_xui_context pContext)
{
	const xge_xui_host_v2_t* pHostV2;

	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 != NULL && pHostV2->flush != NULL ) {
		pHostV2->flush(pHostV2->pUser);
	}
}

static void __xgeXuiHostClipboardSetText(xge_xui_context pContext, const char* sText)
{
	const xge_xui_host_v2_t* pHostV2;

	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 != NULL && pHostV2->clipboard_set_text != NULL ) {
		pHostV2->clipboard_set_text(sText, pHostV2->pUser);
	}
}

static const char* __xgeXuiHostClipboardGetText(xge_xui_context pContext)
{
	const xge_xui_host_v2_t* pHostV2;

	pHostV2 = __xgeXuiHostV2ForContext(pContext);
	if ( pHostV2 != NULL && pHostV2->clipboard_get_text != NULL ) {
		return pHostV2->clipboard_get_text(pHostV2->pUser);
	}
	return NULL;
}

static void __xgeXuiPaintCommandFlush(const xge_xui_paint_command_t* pCommand)
{
	(void)pCommand;
	if ( g_xgeXuiActiveContext != NULL ) {
		g_xgeXuiActiveContext->iPaintFlushCount++;
	}
}

static void __xgeXuiHostDrawRect(xge_rect_t tRect, uint32_t iColor)
{
	const xge_xui_host_t* pHost;
	const xge_xui_host_v2_t* pHostV2;
	xge_xui_paint_command_t tCommand;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	if ( __xgeXuiPaintClipIntersects(g_xgeXuiActiveContext, tRect) == 0 ) {
		return;
	}
	memset(&tCommand, 0, sizeof(tCommand));
	tCommand.iType = XGE_XUI_PAINT_RECT;
	tCommand.tRect = tRect;
	tCommand.iColor = iColor;
	__xgeXuiPaintCommandFlush(&tCommand);
	pHostV2 = __xgeXuiHostV2Get();
	if ( pHostV2 != NULL ) {
		if ( pHostV2->draw_rect != NULL ) {
			pHostV2->draw_rect(tRect, iColor, pHostV2->pUser);
		}
		return;
	}
	pHost = __xgeXuiHostGet();
	if ( pHost->draw_rect != NULL ) {
		pHost->draw_rect(tRect, iColor, pHost->pUser);
	}
}

static void __xgeXuiHostDrawBorderRect(xge_rect_t tRect, float fWidth, uint32_t iColor)
{
	const xge_xui_host_v2_t* pHostV2;
	xge_rect_t tEdge;

	if ( (fWidth <= 0.0f) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	if ( fWidth > tRect.fW * 0.5f ) {
		fWidth = tRect.fW * 0.5f;
	}
	if ( fWidth > tRect.fH * 0.5f ) {
		fWidth = tRect.fH * 0.5f;
	}
	pHostV2 = __xgeXuiHostV2Get();
	if ( pHostV2 != NULL && pHostV2->draw_border_rect != NULL ) {
		pHostV2->draw_border_rect(tRect, fWidth, iColor, pHostV2->pUser);
		return;
	}
	tEdge = tRect;
	tEdge.fH = fWidth;
	__xgeXuiHostDrawRect(tEdge, iColor);
	tEdge.fY = tRect.fY + tRect.fH - fWidth;
	__xgeXuiHostDrawRect(tEdge, iColor);
	tEdge = tRect;
	tEdge.fW = fWidth;
	__xgeXuiHostDrawRect(tEdge, iColor);
	tEdge.fX = tRect.fX + tRect.fW - fWidth;
	__xgeXuiHostDrawRect(tEdge, iColor);
}

static void __xgeXuiHostDrawLine(float fX1, float fY1, float fX2, float fY2, float fWidth, uint32_t iColor)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( (fWidth <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	pHostV2 = __xgeXuiHostV2Get();
	if ( pHostV2 != NULL ) {
		if ( pHostV2->draw_line != NULL ) {
			pHostV2->draw_line(fX1, fY1, fX2, fY2, fWidth, iColor, pHostV2->pUser);
		}
		return;
	}
	xgeShapeLinePx(fX1, fY1, fX2, fY2, fWidth, iColor);
}

static void __xgeXuiHostDrawTriangle(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return;
	}
	pHostV2 = __xgeXuiHostV2Get();
	if ( pHostV2 != NULL ) {
		if ( pHostV2->draw_triangle != NULL ) {
			pHostV2->draw_triangle(tA, tB, tC, iColor, pHostV2->pUser);
		}
		return;
	}
	xgeShapeTriangleFillPx(tA, tB, tC, iColor);
}

static void __xgeXuiHostDrawCircle(float fX, float fY, float fRadius, uint32_t iColor)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( (fRadius <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	pHostV2 = __xgeXuiHostV2Get();
	if ( pHostV2 != NULL ) {
		if ( pHostV2->draw_circle != NULL ) {
			pHostV2->draw_circle(fX, fY, fRadius, iColor, pHostV2->pUser);
		}
		return;
	}
	xgeShapeCircleFillPx(fX, fY, fRadius, iColor);
}

static void __xgeXuiHostDrawCircleStroke(float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	const xge_xui_host_v2_t* pHostV2;

	if ( (fRadius <= 0.0f) || (fWidth <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	pHostV2 = __xgeXuiHostV2Get();
	if ( pHostV2 != NULL ) {
		if ( pHostV2->draw_circle_stroke != NULL ) {
			pHostV2->draw_circle_stroke(fX, fY, fRadius, fWidth, iColor, pHostV2->pUser);
		}
		return;
	}
	xgeShapeCircleStrokePx(fX, fY, fRadius, fWidth, iColor);
}

static void __xgeXuiHostDrawBitmapMask(xge_rect_t tRect, const uint16_t* arrRows, int iWidth, int iHeight, uint32_t iColor)
{
	xge_rect_t tPixel;
	float fCellW;
	float fCellH;
	int x;
	int y;
	uint16_t iMask;

	if ( (arrRows == NULL) || (iWidth <= 0) || (iHeight <= 0) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	fCellW = tRect.fW / (float)iWidth;
	fCellH = tRect.fH / (float)iHeight;
	if ( (fCellW <= 0.0f) || (fCellH <= 0.0f) ) {
		return;
	}
	for ( y = 0; y < iHeight; y++ ) {
		iMask = arrRows[y];
		for ( x = 0; x < iWidth; x++ ) {
			if ( (iMask & (uint16_t)(1u << (iWidth - 1 - x))) == 0 ) {
				continue;
			}
			tPixel.fX = tRect.fX + (float)x * fCellW;
			tPixel.fY = tRect.fY + (float)y * fCellH;
			tPixel.fW = fCellW + 0.25f;
			tPixel.fH = fCellH + 0.25f;
			__xgeXuiHostDrawRect(tPixel, iColor);
		}
	}
}

static void __xgeXuiHostDrawRoundedRect(xge_rect_t tRect, uint32_t iColor, float fRadius)
{
	const xge_xui_host_t* pHost;
	const xge_xui_host_v2_t* pHostV2;
	xge_xui_paint_command_t tCommand;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	if ( __xgeXuiPaintClipIntersects(g_xgeXuiActiveContext, tRect) == 0 ) {
		return;
	}
	memset(&tCommand, 0, sizeof(tCommand));
	tCommand.iType = XGE_XUI_PAINT_ROUNDED_RECT;
	tCommand.tRect = tRect;
	tCommand.iColor = iColor;
	tCommand.fRadius = fRadius;
	__xgeXuiPaintCommandFlush(&tCommand);
	pHostV2 = __xgeXuiHostV2Get();
	if ( pHostV2 != NULL ) {
		if ( pHostV2->draw_rounded_rect != NULL ) {
			pHostV2->draw_rounded_rect(tRect, iColor, fRadius, pHostV2->pUser);
		} else if ( pHostV2->draw_rect != NULL ) {
			pHostV2->draw_rect(tRect, iColor, pHostV2->pUser);
		}
		return;
	}
	pHost = __xgeXuiHostGet();
	if ( pHost->draw_rect != NULL ) {
		pHost->draw_rect(tRect, iColor, pHost->pUser);
	}
}

static void __xgeXuiHostDrawSurface(xge_rect_t tRect, const xge_xui_style_t* pStyle)
{
	if ( (pStyle == NULL) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pStyle->iBackgroundColor) != 0 ) {
		if ( pStyle->fRadius > 0.0f ) {
			__xgeXuiHostDrawRoundedRect(tRect, pStyle->iBackgroundColor, pStyle->fRadius);
		} else {
			__xgeXuiHostDrawRect(tRect, pStyle->iBackgroundColor);
		}
	}
	if ( (pStyle->fBorderWidth > 0.0f) && (XGE_COLOR_GET_A(pStyle->iBorderColor) != 0) ) {
		__xgeXuiHostDrawBorderRect(tRect, pStyle->fBorderWidth, pStyle->iBorderColor);
	}
}

static int __xgeXuiActiveControlClips(void)
{
	return (g_pXgeXuiActivePaintWidget != NULL) && (g_pXgeXuiActivePaintWidget->iRole == XGE_XUI_WIDGET_ROLE_CONTROL) && ((g_pXgeXuiActivePaintWidget->iFlags & XGE_XUI_WIDGET_CLIP) != 0);
}

static int __xgeXuiPaintClipAlreadyContains(xge_xui_context pContext, xge_rect_t tRect)
{
	xge_rect_t tCurrent;
	xge_rect_t tNext;

	if ( pContext == NULL ) {
		return 0;
	}
	if ( pContext->iPaintClipStackCount > 0 ) {
		tCurrent = pContext->arrPaintClipStack[pContext->iPaintClipStackCount - 1];
		tNext = __xgeXuiRectIntersection(tCurrent, tRect);
		return __xgeXuiRectSame(tCurrent, tNext);
	}
	if ( pContext->bPaintClipBaseEnabled ) {
		tCurrent = pContext->tPaintClipBaseRect;
		tNext = __xgeXuiRectIntersection(tCurrent, tRect);
		return __xgeXuiRectSame(tCurrent, tNext);
	}
	return 0;
}

static int __xgeXuiPaintClipIntersects(xge_xui_context pContext, xge_rect_t tRect)
{
	xge_rect_t tCurrent;
	xge_rect_t tNext;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return 0;
	}
	if ( pContext == NULL ) {
		return 1;
	}
	if ( pContext->iPaintClipStackCount > 0 ) {
		tCurrent = pContext->arrPaintClipStack[pContext->iPaintClipStackCount - 1];
		tNext = __xgeXuiRectIntersection(tCurrent, tRect);
		return (tNext.fW > 0.0f) && (tNext.fH > 0.0f);
	}
	if ( pContext->bPaintClipBaseEnabled ) {
		tCurrent = pContext->tPaintClipBaseRect;
		tNext = __xgeXuiRectIntersection(tCurrent, tRect);
		return (tNext.fW > 0.0f) && (tNext.fH > 0.0f);
	}
	return 1;
}

static int __xgeXuiActiveControlClipPush(void)
{
	if ( __xgeXuiActiveControlClips() == 0 ) {
		return 0;
	}
	if ( __xgeXuiPaintClipAlreadyContains(g_xgeXuiActiveContext, g_pXgeXuiActivePaintWidget->tContentRect) ) {
		return 0;
	}
	return __xgeXuiPaintClipPush(g_xgeXuiActiveContext, g_pXgeXuiActivePaintWidget->tContentRect);
}

static void __xgeXuiHostDrawImage(const xge_draw_t* pDraw)
{
	const xge_xui_host_t* pHost;
	const xge_xui_host_v2_t* pHostV2;
	xge_xui_paint_command_t tCommand;
	int bImageClipPushed;

	if ( (pDraw == NULL) || (pDraw->tDst.fW <= 0.0f) || (pDraw->tDst.fH <= 0.0f) ) {
		return;
	}
	if ( __xgeXuiPaintClipIntersects(g_xgeXuiActiveContext, pDraw->tDst) == 0 ) {
		return;
	}
	bImageClipPushed = __xgeXuiActiveControlClipPush();
	memset(&tCommand, 0, sizeof(tCommand));
	tCommand.iType = XGE_XUI_PAINT_IMAGE;
	tCommand.tDraw = *pDraw;
	__xgeXuiPaintCommandFlush(&tCommand);
	pHostV2 = __xgeXuiHostV2Get();
	if ( pHostV2 != NULL ) {
		if ( pHostV2->draw_texture != NULL ) {
			pHostV2->draw_texture((xui_texture)pDraw->pTexture, pDraw->tSrc, pDraw->tDst, pDraw->iColor, pDraw->iFlags, pHostV2->pUser);
		}
		if ( bImageClipPushed ) {
			__xgeXuiPaintClipPop(g_xgeXuiActiveContext);
		}
		return;
	}
	pHost = __xgeXuiHostGet();
	if ( pHost->draw_image != NULL ) {
		pHost->draw_image(pDraw, pHost->pUser);
	}
	if ( bImageClipPushed ) {
		__xgeXuiPaintClipPop(g_xgeXuiActiveContext);
	}
}

static void __xgeXuiHostDrawImageClipOnly(const xge_draw_t* pDraw)
{
	const xge_xui_host_t* pHost;
	const xge_xui_host_v2_t* pHostV2;
	xge_xui_paint_command_t tCommand;

	if ( (pDraw == NULL) || (pDraw->tDst.fW <= 0.0f) || (pDraw->tDst.fH <= 0.0f) ) {
		return;
	}
	if ( __xgeXuiPaintClipIntersects(g_xgeXuiActiveContext, pDraw->tDst) == 0 ) {
		return;
	}
	memset(&tCommand, 0, sizeof(tCommand));
	tCommand.iType = XGE_XUI_PAINT_IMAGE;
	tCommand.tDraw = *pDraw;
	__xgeXuiPaintCommandFlush(&tCommand);
	pHostV2 = __xgeXuiHostV2Get();
	if ( pHostV2 != NULL ) {
		if ( pHostV2->draw_texture != NULL ) {
			pHostV2->draw_texture((xui_texture)pDraw->pTexture, pDraw->tSrc, pDraw->tDst, pDraw->iColor, pDraw->iFlags, pHostV2->pUser);
		}
		return;
	}
	pHost = __xgeXuiHostGet();
	if ( pHost->draw_image != NULL ) {
		pHost->draw_image(pDraw, pHost->pUser);
	}
}

static void __xgeXuiHostDrawTextRect(xui_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	const xge_xui_host_t* pHost;
	const xge_xui_host_v2_t* pHostV2;
	xge_xui_paint_command_t tCommand;
	int bTextClipPushed;
	int bNeedsTextClip;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return;
	}
	if ( __xgeXuiPaintClipIntersects(g_xgeXuiActiveContext, tRect) == 0 ) {
		return;
	}
	iFlags |= XGE_TEXT_SCREEN_SPACE;
	bTextClipPushed = 0;
	bNeedsTextClip = (g_xgeXuiActiveContext != NULL) || ((iFlags & XGE_TEXT_CLIP) != 0);
	if ( bNeedsTextClip ) {
		iFlags |= XGE_TEXT_CLIP;
	}
	if ( __xgeXuiActiveControlClips() ) {
		iFlags |= XGE_TEXT_CLIP;
		bNeedsTextClip = 1;
	}
	if ( bNeedsTextClip ) {
		if ( !__xgeXuiPaintClipAlreadyContains(g_xgeXuiActiveContext, tRect) ) {
			bTextClipPushed = __xgeXuiPaintClipPush(g_xgeXuiActiveContext, tRect);
		}
		if ( bTextClipPushed ) {
			iFlags &= ~XGE_TEXT_CLIP;
		}
	}
	memset(&tCommand, 0, sizeof(tCommand));
	tCommand.iType = XGE_XUI_PAINT_TEXT;
	tCommand.pFont = pFont;
	tCommand.sText = sText;
	tCommand.tRect = tRect;
	tCommand.iColor = iColor;
	tCommand.iFlags = iFlags;
	__xgeXuiPaintCommandFlush(&tCommand);
	pHostV2 = __xgeXuiHostV2Get();
	if ( pHostV2 != NULL ) {
		if ( pHostV2->draw_text != NULL ) {
			pHostV2->draw_text((xui_font)pFont, sText, tRect, iColor, iFlags, pHostV2->pUser);
		}
		if ( bTextClipPushed ) {
			__xgeXuiPaintClipPop(g_xgeXuiActiveContext);
		}
		return;
	}
	pHost = __xgeXuiHostGet();
	if ( pHost->draw_text_rect != NULL ) {
		pHost->draw_text_rect(pFont, sText, tRect, iColor, iFlags, pHost->pUser);
	}
	if ( bTextClipPushed ) {
		__xgeXuiPaintClipPop(g_xgeXuiActiveContext);
	}
}

static xge_vec2_t __xgeXuiHostMeasureText(xui_font pFont, const char* sText)
{
	const xge_xui_host_t* pHost;
	const xge_xui_host_v2_t* pHostV2;
	xge_vec2_t tSize;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	pHostV2 = __xgeXuiHostV2Get();
	if ( pHostV2 != NULL ) {
		if ( pHostV2->measure_text != NULL ) {
			return pHostV2->measure_text((xui_font)pFont, sText, 0, pHostV2->pUser);
		}
		return tSize;
	}
	pHost = __xgeXuiHostGet();
	if ( pHost->measure_text != NULL ) {
		return pHost->measure_text(pFont, sText, pHost->pUser);
	}
	return tSize;
}

static void __xgeXuiHostClipSet(xge_rect_t tRect)
{
	const xge_xui_host_t* pHost;
	const xge_xui_host_v2_t* pHostV2;
	xge_xui_paint_command_t tCommand;

	memset(&tCommand, 0, sizeof(tCommand));
	tCommand.iType = XGE_XUI_PAINT_CLIP_SET;
	tCommand.tRect = tRect;
	__xgeXuiPaintCommandFlush(&tCommand);
	pHostV2 = __xgeXuiHostV2Get();
	if ( pHostV2 != NULL ) {
		if ( pHostV2->clip_push != NULL ) {
			pHostV2->clip_push(tRect, pHostV2->pUser);
		}
		return;
	}
	pHost = __xgeXuiHostGet();
	if ( pHost->clip_set != NULL ) {
		pHost->clip_set(tRect, pHost->pUser);
	}
}

static void __xgeXuiHostClipClear(void)
{
	const xge_xui_host_t* pHost;
	const xge_xui_host_v2_t* pHostV2;
	xge_xui_paint_command_t tCommand;

	memset(&tCommand, 0, sizeof(tCommand));
	tCommand.iType = XGE_XUI_PAINT_CLIP_CLEAR;
	__xgeXuiPaintCommandFlush(&tCommand);
	pHostV2 = __xgeXuiHostV2Get();
	if ( pHostV2 != NULL ) {
		if ( pHostV2->clip_clear != NULL ) {
			pHostV2->clip_clear(pHostV2->pUser);
		}
		return;
	}
	pHost = __xgeXuiHostGet();
	if ( pHost->clip_clear != NULL ) {
		pHost->clip_clear(pHost->pUser);
	}
}
