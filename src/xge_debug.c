static xge_debug_stats_t __xgeDebugStatsGetValue(void)
{
	xge_debug_stats_t tStats;

	memset(&tStats, 0, sizeof(tStats));
	if ( g_xge.bInitialized == 0 ) {
		return tStats;
	}
	tStats.tFrame = xgeFrameStatsGet();
	tStats.iTextureCount = g_xge.iTextureCount;
	tStats.iTextureMemoryBytes = g_xge.iTextureMemoryBytes;
	tStats.iFontCount = g_xge.iFontCount;
	tStats.iAudioCount = g_xge.iAudioCount;
	if ( (g_xge.bSokolRunning != 0) && (glGetError != NULL) ) {
		g_xge.iLastGLError = (int)glGetError();
	}
	tStats.iLastGLError = g_xge.iLastGLError;
	return tStats;
}

int xgeDebugGetStats(xge_debug_stats_t* pStats)
{
	if ( pStats == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pStats = __xgeDebugStatsGetValue();
	return XGE_OK;
}

int xgeDebugDumpCaps(char* sBuffer, int iSize)
{
	xge_gpu_caps_t tCaps;
	xge_platform_caps_t tPlatformCaps;
	const char* sPlatform;
	const char* sGraphics;
	const char* sVendor;
	const char* sRenderer;
	const char* sVersion;
	const char* sShading;
	int iNeed;

	if ( (sBuffer == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(&tCaps, 0, sizeof(tCaps));
	memset(&tPlatformCaps, 0, sizeof(tPlatformCaps));
	(void)xgeGpuCapsGet(&tCaps);
	(void)xgePlatformCapsGet(&tPlatformCaps);
	sPlatform = tPlatformCaps.sPlatformName;
	sGraphics = tPlatformCaps.sGraphicsName;
	sVendor = tCaps.sVendor;
	sRenderer = tCaps.sRenderer;
	sVersion = tCaps.sVersion;
	sShading = tCaps.sShadingLanguage;
	iNeed = snprintf(sBuffer, (size_t)iSize,
		"XGE %d.%d.%d\n"
		"Platform Backend: %s\n"
		"Sokol Target: %s\n"
		"Graphics Backend: %s\n"
		"Window: %d\n"
		"Offscreen: %d\n"
		"RenderTarget: %d\n"
		"ThreadSafeSubmit: %d\n"
		"RenderThread: %d\n"
		"Platform Running: %d\n"
		"Window Size: %dx%d\n"
		"Framebuffer Size: %dx%d\n"
		"DPI Scale: %.2f\n"
		"HighDPI: %d\n"
		"Touch: %d\n"
		"Mouse: %d\n"
		"TextInput: %d\n"
		"Gamepad: %d\n"
		"Sokol GLCore: %d\n"
		"Sokol GLES3: %d\n"
		"Sokol D3D11: %d\n"
		"Sokol Metal: %d\n"
		"Sokol Dummy: %d\n"
		"Audio: %d\n"
		"OpenGL Vendor: %s\n"
		"OpenGL Renderer: %s\n"
		"OpenGL Version: %s\n"
		"GLSL Version: %s\n"
		"Max Texture Size: %d\n",
		XGE_VERSION_MAJOR, XGE_VERSION_MINOR, XGE_VERSION_PATCH,
		sPlatform, tPlatformCaps.sSokolTargetName, sGraphics,
		tPlatformCaps.bWindow, tPlatformCaps.bOffscreen, tPlatformCaps.bRenderTarget,
		tPlatformCaps.bThreadSafeSubmit, xgeRenderThreadGet(),
		g_xge.tPlatformRuntime.bRunning,
		g_xge.tPlatformRuntime.iWindowWidth, g_xge.tPlatformRuntime.iWindowHeight,
		g_xge.tPlatformRuntime.iFramebufferWidth, g_xge.tPlatformRuntime.iFramebufferHeight,
		g_xge.tPlatformRuntime.fDpiScale,
		tPlatformCaps.bHighDPI, tPlatformCaps.bTouch, tPlatformCaps.bMouse,
		tPlatformCaps.bTextInput, tPlatformCaps.bGamepad,
		tPlatformCaps.bSokolGLCore, tPlatformCaps.bSokolGLES3, tPlatformCaps.bSokolD3D11,
		tPlatformCaps.bSokolMetal, tPlatformCaps.bSokolDummy, tPlatformCaps.bAudio,
		sVendor, sRenderer, sVersion, sShading, tCaps.iMaxTextureSize);
	if ( iNeed < 0 ) {
		sBuffer[0] = 0;
		return XGE_ERROR;
	}
	return iNeed;
}

int xgedbgDirtyRectCount(void)
{
	if ( g_xge.bInitialized == 0 ) {
		return 0;
	}
	return g_xge.iDirtyRectCount;
}

int xgedbgDirtyRectGet(int iIndex, xge_rect_t* pRect)
{
	if ( (g_xge.bInitialized == 0) || (pRect == NULL) || (iIndex < 0) || (iIndex >= g_xge.iDirtyRectCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*pRect = g_xge.arrDirtyRects[iIndex];
	return XGE_OK;
}

void xgedbgDirtyRectClear(void)
{
	__xgeDirtyRectClear();
}

static const char* __xgedbgXuiLayoutName(int iLayout)
{
	switch ( iLayout ) {
	case XGE_XUI_LAYOUT_ABSOLUTE: return "absolute";
	case XGE_XUI_LAYOUT_ROW: return "row";
	case XGE_XUI_LAYOUT_COLUMN: return "column";
	case XGE_XUI_LAYOUT_STACK: return "stack";
	case XGE_XUI_LAYOUT_GRID: return "grid";
	case XGE_XUI_LAYOUT_DOCK: return "dock";
	default: return "unknown";
	}
}

static const char* __xgedbgXuiOverflowName(int iOverflow)
{
	switch ( iOverflow ) {
	case XGE_XUI_OVERFLOW_VISIBLE: return "visible";
	case XGE_XUI_OVERFLOW_CLIP: return "clip";
	case XGE_XUI_OVERFLOW_HIDDEN: return "hidden";
	case XGE_XUI_OVERFLOW_SCROLL: return "scroll";
	default: return "unknown";
	}
}

static const char* __xgedbgXuiLayerName(int iLayer)
{
	switch ( iLayer ) {
	case XGE_XUI_LAYER_NORMAL: return "normal";
	case XGE_XUI_LAYER_FLOATING: return "floating";
	case XGE_XUI_LAYER_POPUP: return "popup";
	case XGE_XUI_LAYER_MODAL: return "modal";
	case XGE_XUI_LAYER_TOOLTIP: return "tooltip";
	case XGE_XUI_LAYER_DRAG_ADORNER: return "dragAdorner";
	case XGE_XUI_LAYER_DEBUG: return "debug";
	default: return "unknown";
	}
}

static float __xgedbgXuiSizeDebugValue(xge_xui_size_t tSize)
{
	return tSize.fValue;
}

static xge_rect_t __xgedbgXuiMarginRect(xge_xui_widget pWidget)
{
	xge_rect_t tRect;

	tRect = pWidget->tOuterRect;
	if ( (tRect.fW <= 0.0f) && (tRect.fH <= 0.0f) ) {
		tRect = pWidget->tRect;
		tRect.fX -= __xgedbgXuiSizeDebugValue(pWidget->tStyle.tMargin.tLeft);
		tRect.fY -= __xgedbgXuiSizeDebugValue(pWidget->tStyle.tMargin.tTop);
		tRect.fW += __xgedbgXuiSizeDebugValue(pWidget->tStyle.tMargin.tLeft) + __xgedbgXuiSizeDebugValue(pWidget->tStyle.tMargin.tRight);
		tRect.fH += __xgedbgXuiSizeDebugValue(pWidget->tStyle.tMargin.tTop) + __xgedbgXuiSizeDebugValue(pWidget->tStyle.tMargin.tBottom);
	}
	return tRect;
}

static int __xgedbgXuiChildCount(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	int iCount;

	iCount = 0;
	if ( pWidget == NULL ) {
		return 0;
	}
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		iCount++;
	}
	return iCount;
}

static int __xgedbgXuiWidgetDepth(xge_xui_widget pWidget)
{
	int iDepth;

	iDepth = 0;
	if ( pWidget == NULL ) {
		return 0;
	}
	while ( pWidget->pParent != NULL ) {
		iDepth++;
		pWidget = pWidget->pParent;
	}
	return iDepth;
}

static void __xgedbgAppend(char* sBuffer, int iSize, int* pOffset, const char* sFormat, ...)
{
	char arrLine[1024];
	va_list Args;
	int iLen;
	int iCopy;

	if ( (pOffset == NULL) || (sFormat == NULL) ) {
		return;
	}
	va_start(Args, sFormat);
	iLen = vsnprintf(arrLine, sizeof(arrLine), sFormat, Args);
	va_end(Args);
	if ( iLen < 0 ) {
		return;
	}
	if ( iLen >= (int)sizeof(arrLine) ) {
		iLen = (int)sizeof(arrLine) - 1;
	}
	if ( (sBuffer != NULL) && (iSize > 0) && (*pOffset < (iSize - 1)) ) {
		iCopy = iLen;
		if ( iCopy > (iSize - 1 - *pOffset) ) {
			iCopy = iSize - 1 - *pOffset;
		}
		if ( iCopy > 0 ) {
			memcpy(sBuffer + *pOffset, arrLine, (size_t)iCopy);
			sBuffer[*pOffset + iCopy] = 0;
		}
	}
	*pOffset += iLen;
}

static void __xgedbgXuiWidgetTreeDump(xge_xui_widget pWidget, int iDepth, char* sBuffer, int iSize, int* pOffset)
{
	xge_xui_widget pChild;
	const char* sName;
	int i;

	if ( (pWidget == NULL) || (pOffset == NULL) ) {
		return;
	}
	if ( iDepth > 64 ) {
		__xgedbgAppend(sBuffer, iSize, pOffset, "%*s... depth-limit\n", 64 * 2, "");
		return;
	}
	sName = (pWidget->sName != NULL) ? pWidget->sName : "";
	__xgedbgAppend(sBuffer, iSize, pOffset,
		"%*swidget ptr=%p id=%d name=\"%s\" layout=%s overflow=%s layer=%s tree=%u tabStop=%d focusScope=%d tabIndex=%d ime=%d hit=%d transparent=%d flags=0x%04X dirty=(style:%d layout:%d paint:%d) z=%d children=%d bg=0x%08X borderColor=0x%08X borderWidth=%.1f focusRingColor=0x%08X focusRingWidth=%.1f disabledOverlay=0x%08X debugOutlineColor=0x%08X debugOutlineWidth=%.1f radius=%.1f rect=(%.1f,%.1f,%.1f,%.1f) outer=(%.1f,%.1f,%.1f,%.1f) border=(%.1f,%.1f,%.1f,%.1f) paddingRect=(%.1f,%.1f,%.1f,%.1f) content=(%.1f,%.1f,%.1f,%.1f) margin=(%.1f,%.1f,%.1f,%.1f) padding=(%.1f,%.1f,%.1f,%.1f) desired=(%.1f,%.1f)\n",
		iDepth * 2,
		"",
		(void*)pWidget,
		pWidget->iId,
		sName,
		__xgedbgXuiLayoutName(pWidget->tStyle.iLayout),
		__xgedbgXuiOverflowName(pWidget->tStyle.iOverflow),
		__xgedbgXuiLayerName(xgeXuiWidgetGetLayer(pWidget)),
		pWidget->iTreeOrder,
		xgeXuiWidgetIsTabStop(pWidget),
		xgeXuiWidgetIsFocusScope(pWidget),
		pWidget->iTabIndex,
		xgeXuiWidgetGetImeMode(pWidget),
		xgeXuiWidgetIsHitTestVisible(pWidget),
		xgeXuiWidgetIsInputTransparent(pWidget),
		pWidget->iFlags,
		(pWidget->iFlags & XGE_XUI_WIDGET_DIRTY_STYLE) != 0,
		(pWidget->iFlags & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0,
		(pWidget->iFlags & XGE_XUI_WIDGET_DIRTY_PAINT) != 0,
		pWidget->tStyle.iZ,
		__xgedbgXuiChildCount(pWidget),
		pWidget->tStyle.iBackgroundColor,
		pWidget->tStyle.iBorderColor,
		pWidget->tStyle.fBorderWidth,
		pWidget->tStyle.iFocusRingColor,
		pWidget->tStyle.fFocusRingWidth,
		pWidget->tStyle.iDisabledOverlayColor,
		pWidget->tStyle.iDebugOutlineColor,
		pWidget->tStyle.fDebugOutlineWidth,
		pWidget->tStyle.fRadius,
		pWidget->tRect.fX,
		pWidget->tRect.fY,
		pWidget->tRect.fW,
		pWidget->tRect.fH,
		pWidget->tOuterRect.fX,
		pWidget->tOuterRect.fY,
		pWidget->tOuterRect.fW,
		pWidget->tOuterRect.fH,
		pWidget->tBorderRect.fX,
		pWidget->tBorderRect.fY,
		pWidget->tBorderRect.fW,
		pWidget->tBorderRect.fH,
		pWidget->tPaddingRect.fX,
		pWidget->tPaddingRect.fY,
		pWidget->tPaddingRect.fW,
		pWidget->tPaddingRect.fH,
		pWidget->tContentRect.fX,
		pWidget->tContentRect.fY,
		pWidget->tContentRect.fW,
		pWidget->tContentRect.fH,
		__xgedbgXuiSizeDebugValue(pWidget->tStyle.tMargin.tLeft),
		__xgedbgXuiSizeDebugValue(pWidget->tStyle.tMargin.tTop),
		__xgedbgXuiSizeDebugValue(pWidget->tStyle.tMargin.tRight),
		__xgedbgXuiSizeDebugValue(pWidget->tStyle.tMargin.tBottom),
		__xgedbgXuiSizeDebugValue(pWidget->tStyle.tPadding.tLeft),
		__xgedbgXuiSizeDebugValue(pWidget->tStyle.tPadding.tTop),
		__xgedbgXuiSizeDebugValue(pWidget->tStyle.tPadding.tRight),
		__xgedbgXuiSizeDebugValue(pWidget->tStyle.tPadding.tBottom),
		pWidget->tDesiredSize.fX,
		pWidget->tDesiredSize.fY);
	i = 0;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		(void)i;
		__xgedbgXuiWidgetTreeDump(pChild, iDepth + 1, sBuffer, iSize, pOffset);
		i++;
	}
}

int xgedbgXuiWidgetTreeDump(xge_xui_widget pRoot, char* sBuffer, int iSize)
{
	int iOffset;

	if ( (pRoot == NULL) || (sBuffer == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sBuffer[0] = 0;
	iOffset = 0;
	__xgedbgXuiWidgetTreeDump(pRoot, 0, sBuffer, iSize, &iOffset);
	return iOffset;
}

int xgedbgXuiWidgetInspect(xge_xui_context pContext, xge_xui_widget pWidget, xgedbg_xui_widget_info_t* pInfo)
{
	if ( (pWidget == NULL) || (pInfo == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pInfo, 0, sizeof(*pInfo));
	pInfo->pWidget = pWidget;
	pInfo->pParent = pWidget->pParent;
	pInfo->iId = pWidget->iId;
	pInfo->sName = (pWidget->sName != NULL) ? pWidget->sName : "";
	pInfo->iFlags = pWidget->iFlags;
	pInfo->iLayout = pWidget->tStyle.iLayout;
	pInfo->iLayer = xgeXuiWidgetGetLayer(pWidget);
	pInfo->iZ = pWidget->tStyle.iZ;
	pInfo->iTreeOrder = pWidget->iTreeOrder;
	pInfo->iDepth = __xgedbgXuiWidgetDepth(pWidget);
	pInfo->iChildCount = __xgedbgXuiChildCount(pWidget);
	pInfo->bVisible = (pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) != 0;
	pInfo->bEnabled = (pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) != 0;
	pInfo->bFocusable = (pWidget->iFlags & XGE_XUI_WIDGET_FOCUSABLE) != 0;
	pInfo->bTabStop = xgeXuiWidgetIsTabStop(pWidget);
	pInfo->bFocusScope = xgeXuiWidgetIsFocusScope(pWidget);
	pInfo->iTabIndex = pWidget->iTabIndex;
	pInfo->iImeMode = xgeXuiWidgetGetImeMode(pWidget);
	pInfo->bClipped = (pWidget->iFlags & XGE_XUI_WIDGET_CLIP) != 0;
	pInfo->bHitTestVisible = xgeXuiWidgetIsHitTestVisible(pWidget);
	pInfo->bInputTransparent = xgeXuiWidgetIsInputTransparent(pWidget);
	pInfo->bDirtyStyle = (pWidget->iFlags & XGE_XUI_WIDGET_DIRTY_STYLE) != 0;
	pInfo->bDirtyLayout = (pWidget->iFlags & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0;
	pInfo->bDirtyPaint = (pWidget->iFlags & XGE_XUI_WIDGET_DIRTY_PAINT) != 0;
	if ( pContext != NULL ) {
		pInfo->bFocus = pContext->pFocus == pWidget;
		pInfo->bHover = pContext->pHover == pWidget;
		pInfo->bCapture = xgeXuiWidgetHasCapture(pContext, pWidget);
	}
	pInfo->tRect = pWidget->tRect;
	pInfo->tLocalRect = pWidget->tLocalRect;
	pInfo->tOuterRect = pWidget->tOuterRect;
	pInfo->tBorderRect = pWidget->tBorderRect;
	pInfo->tPaddingRect = pWidget->tPaddingRect;
	pInfo->tContentRect = pWidget->tContentRect;
	pInfo->tDesiredSize = pWidget->tDesiredSize;
	pInfo->tMargin = pWidget->tStyle.tMargin;
	pInfo->tPadding = pWidget->tStyle.tPadding;
	pInfo->iBackgroundColor = pWidget->tStyle.iBackgroundColor;
	pInfo->iBorderColor = pWidget->tStyle.iBorderColor;
	pInfo->iFocusRingColor = pWidget->tStyle.iFocusRingColor;
	pInfo->iDisabledOverlayColor = pWidget->tStyle.iDisabledOverlayColor;
	pInfo->iDebugOutlineColor = pWidget->tStyle.iDebugOutlineColor;
	pInfo->fRadius = pWidget->tStyle.fRadius;
	pInfo->fBorderWidth = pWidget->tStyle.fBorderWidth;
	pInfo->fFocusRingWidth = pWidget->tStyle.fFocusRingWidth;
	pInfo->fDebugOutlineWidth = pWidget->tStyle.fDebugOutlineWidth;
	return XGE_OK;
}

int xgedbgXuiWidgetInspectAt(xge_xui_context pContext, float fX, float fY, xgedbg_xui_widget_info_t* pInfo)
{
	xge_xui_widget pWidget;

	if ( (pContext == NULL) || (pInfo == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pWidget = xgeXuiHitTest(pContext, fX, fY);
	if ( pWidget == NULL ) {
		memset(pInfo, 0, sizeof(*pInfo));
		return XGE_ERROR;
	}
	return xgedbgXuiWidgetInspect(pContext, pWidget, pInfo);
}

static uint32_t __xgedbgXuiOverlayDepthColor(int iDepth)
{
	static const uint32_t arrColors[] = {
		XGE_COLOR_RGBA(72, 178, 255, 210),
		XGE_COLOR_RGBA(255, 190, 72, 210),
		XGE_COLOR_RGBA(120, 232, 150, 210),
		XGE_COLOR_RGBA(255, 112, 150, 210),
		XGE_COLOR_RGBA(190, 140, 255, 210)
	};

	if ( iDepth < 0 ) {
		iDepth = 0;
	}
	return arrColors[iDepth % (int)(sizeof(arrColors) / sizeof(arrColors[0]))];
}

static void __xgedbgXuiDebugOverlayPaintWidget(xge_xui_widget pWidget, uint32_t iFlags, xge_font pFont, int iDepth)
{
	xge_xui_widget pChild;
	uint32_t iColor;
	char arrLabel[128];

	if ( pWidget == NULL ) {
		return;
	}
	if ( (pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 ) {
		return;
	}
	iColor = __xgedbgXuiOverlayDepthColor(iDepth);
	if ( (iFlags & XGEDBG_XUI_OVERLAY_RECTS) != 0 ) {
		xgeShapeRectStrokePx(pWidget->tBorderRect, 1.0f, iColor);
	}
	if ( (iFlags & XGEDBG_XUI_OVERLAY_CONTENT_RECTS) != 0 ) {
		xgeShapeRectStrokePx(pWidget->tContentRect, 1.0f, XGE_COLOR_RGBA(255, 255, 255, 90));
	}
	if ( (iFlags & XGEDBG_XUI_OVERLAY_MARGIN) != 0 ) {
		xgeShapeRectStrokePx(__xgedbgXuiMarginRect(pWidget), 1.0f, XGE_COLOR_RGBA(255, 150, 50, 150));
	}
	if ( (iFlags & XGEDBG_XUI_OVERLAY_PADDING) != 0 ) {
		xgeShapeRectStrokePx(pWidget->tPaddingRect, 2.0f, XGE_COLOR_RGBA(120, 255, 170, 150));
	}
	if ( ((iFlags & XGEDBG_XUI_OVERLAY_DIRTY) != 0) && ((pWidget->iFlags & (XGE_XUI_WIDGET_DIRTY_STYLE | XGE_XUI_WIDGET_DIRTY_LAYOUT | XGE_XUI_WIDGET_DIRTY_PAINT)) != 0) ) {
		xgeShapeRectStrokePx(pWidget->tBorderRect, 2.0f, XGE_COLOR_RGBA(255, 70, 70, 230));
	}
	if ( ((iFlags & XGEDBG_XUI_OVERLAY_LABELS) != 0) && (pFont != NULL) && (pWidget->tRect.fW > 12.0f) && (pWidget->tRect.fH > 12.0f) ) {
		snprintf(arrLabel, sizeof(arrLabel), "#%d %s %s %.0fx%.0f%s%s%s", pWidget->iId, (pWidget->sName != NULL) ? pWidget->sName : "", __xgedbgXuiLayoutName(pWidget->tStyle.iLayout), pWidget->tRect.fW, pWidget->tRect.fH, (pWidget->iFlags & XGE_XUI_WIDGET_DIRTY_STYLE) ? " S" : "", (pWidget->iFlags & XGE_XUI_WIDGET_DIRTY_LAYOUT) ? " L" : "", (pWidget->iFlags & XGE_XUI_WIDGET_DIRTY_PAINT) ? " P" : "");
		xgeTextDraw(pFont, arrLabel, pWidget->tRect.fX + 3.0f, pWidget->tRect.fY + 3.0f, XGE_COLOR_RGBA(245, 248, 255, 230));
	}
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xgedbgXuiDebugOverlayPaintWidget(pChild, iFlags, pFont, iDepth + 1);
	}
}

int xgedbgXuiDebugOverlayPaint(xge_xui_context pContext, uint32_t iFlags, xge_font pFont)
{
	xge_xui_widget pCapture;
	int i;

	if ( pContext == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iFlags == 0 ) {
		iFlags = XGEDBG_XUI_OVERLAY_DEFAULT;
	}
	__xgedbgXuiDebugOverlayPaintWidget(pContext->pRoot, iFlags, pFont, 0);
	__xgedbgXuiDebugOverlayPaintWidget(pContext->pOverlayRoot, iFlags, pFont, 0);
	if ( ((iFlags & XGEDBG_XUI_OVERLAY_FOCUS) != 0) && (pContext->pFocus != NULL) ) {
		xgeShapeRectStrokePx(pContext->pFocus->tBorderRect, 2.0f, XGE_COLOR_RGBA(255, 230, 64, 245));
	}
	if ( ((iFlags & XGEDBG_XUI_OVERLAY_HOVER) != 0) && (pContext->pHover != NULL) ) {
		xgeShapeRectStrokePx(pContext->pHover->tBorderRect, 2.0f, XGE_COLOR_RGBA(80, 220, 255, 245));
	}
	if ( ((iFlags & XGEDBG_XUI_OVERLAY_CAPTURE) != 0) && (pContext->pCapture != NULL) ) {
		xgeShapeRectStrokePx(pContext->pCapture->tBorderRect, 2.0f, XGE_COLOR_RGBA(255, 100, 100, 245));
	}
	if ( (iFlags & XGEDBG_XUI_OVERLAY_CAPTURE) != 0 ) {
		for ( i = 1; i < XGE_XUI_POINTER_CAPTURE_CAPACITY; i++ ) {
			pCapture = pContext->arrPointerCaptureWidget[i];
			if ( (pCapture != NULL) && (pCapture != pContext->pCapture) ) {
				xgeShapeRectStrokePx(pCapture->tBorderRect, 2.0f, XGE_COLOR_RGBA(255, 100, 100, 245));
			}
		}
	}
	return XGE_OK;
}

int xgedbgXuiLayoutSnapshot(xge_xui_context pContext, char* sBuffer, int iSize)
{
	int iOffset;

	if ( (pContext == NULL) || (sBuffer == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sBuffer[0] = 0;
	iOffset = 0;
	__xgedbgAppend(sBuffer, iSize, &iOffset,
		"xui context=%p initialized=%d refresh=%d events=%d focus=%p capture=%p hover=%p dirty_layout=%d dirty_paint=%d paint_commands=%d paint_flush=%d\n",
		(void*)pContext,
		pContext->bInitialized,
		pContext->bRefreshRequested,
		pContext->iEventCount,
		(void*)pContext->pFocus,
		(void*)pContext->pCapture,
		(void*)pContext->pHover,
		pContext->iDirtyLayoutCount,
		pContext->iDirtyPaintCount,
		pContext->iPaintCommandCount,
		pContext->iPaintFlushCount);
	if ( pContext->pRoot != NULL ) {
		__xgedbgAppend(sBuffer, iSize, &iOffset, "root:\n");
		__xgedbgXuiWidgetTreeDump(pContext->pRoot, 1, sBuffer, iSize, &iOffset);
	}
	if ( pContext->pOverlayRoot != NULL ) {
		__xgedbgAppend(sBuffer, iSize, &iOffset, "overlay:\n");
		__xgedbgXuiWidgetTreeDump(pContext->pOverlayRoot, 1, sBuffer, iSize, &iOffset);
	}
	return iOffset;
}

int xgedbgXuiPageTrace(xge_xui_page_t* pPage, char* sBuffer, int iSize)
{
	xvalue pDoc;
	int iOffset;
	int i;

	if ( (pPage == NULL) || (sBuffer == NULL) || (iSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sBuffer[0] = 0;
	iOffset = 0;
	pDoc = (xvalue)pPage->pDocument;
	__xgedbgAppend(sBuffer, iSize, &iOffset,
		"xui page=%p uri=%s loaded=%d root=%p style_version=%u theme_version=%u model_version=%u error=\"%s\"\n",
		(void*)pPage,
		(pPage->tLoader.sURI != NULL) ? pPage->tLoader.sURI : "<memory>",
		(pPage->pRoot != NULL),
		(void*)pPage->pRoot,
		pPage->iStyleVersion,
		pPage->iThemeVersion,
		pPage->iModelVersion,
		pPage->sError);
	__xgedbgAppend(sBuffer, iSize, &iOffset,
		"document=%p type=%d resource_bytes=%d imports=%d indexes=%d click_bindings=%d model_bindings=%d controls={button:%d image:%d input:%d label:%d separator:%d scrollView:%d virtualList:%d}\n",
		(void*)pDoc,
		(pDoc != NULL) ? xvoType(pDoc) : -1,
		pPage->tLoader.tResource.iSize,
		pPage->iImportCount,
		pPage->iIndexCount,
		pPage->iClickBindingCount,
		pPage->iModelBindingCount,
		pPage->iButtonCount,
		pPage->iImageCount,
		pPage->iInputCount,
		pPage->iLabelCount,
		pPage->iSeparatorCount,
		pPage->iScrollViewCount,
		pPage->iVirtualListCount);
	__xgedbgAppend(sBuffer, iSize, &iOffset,
		"sections={styles:%d tokens:%d templates:%d} safe_area_applied=%d index_overflow=%d\n",
		(pPage->pMergedStyles != NULL),
		(pPage->pMergedTokens != NULL),
		(pPage->pMergedTemplates != NULL),
		pPage->bSafeAreaApplied,
		pPage->bIndexOverflow);
	if ( pPage->iImportCount > 0 ) {
		__xgedbgAppend(sBuffer, iSize, &iOffset, "imports:\n");
		for ( i = 0; i < pPage->iImportCount; i++ ) {
			__xgedbgAppend(sBuffer, iSize, &iOffset, "  [%d] uri=%s doc=%p bytes=%d\n", i, pPage->arrImportURI[i], pPage->arrImportDocument[i], pPage->arrImportResource[i].iSize);
		}
	}
	if ( pPage->iIndexCount > 0 ) {
		__xgedbgAppend(sBuffer, iSize, &iOffset, "indexes:\n");
		for ( i = 0; i < pPage->iIndexCount; i++ ) {
			__xgedbgAppend(sBuffer, iSize, &iOffset, "  [%d] key=%s id=%d numeric=%d widget=%p\n", i, (pPage->arrIndex[i].sKey != NULL) ? pPage->arrIndex[i].sKey : "", pPage->arrIndex[i].iNumericId, pPage->arrIndex[i].bNumeric, (void*)pPage->arrIndex[i].pWidget);
		}
	}
	if ( pPage->iModelBindingCount > 0 ) {
		__xgedbgAppend(sBuffer, iSize, &iOffset, "model_bindings:\n");
		for ( i = 0; i < pPage->iModelBindingCount; i++ ) {
			__xgedbgAppend(sBuffer, iSize, &iOffset, "  [%d] kind=%d key=%s widget=%p control=%p index=%d\n", i, pPage->arrModelBinding[i].iKind, pPage->arrModelBinding[i].sKey, (void*)pPage->arrModelBinding[i].pWidget, pPage->arrModelBinding[i].pControl, pPage->arrModelBinding[i].iControlIndex);
		}
	}
	return iOffset;
}
