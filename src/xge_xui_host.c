// XUI 孵化期基础模块：context、widget tree、dirty layout 和 paint 统计

static const xge_xui_host_t* g_xgeXuiActiveHost;
static xge_xui_context g_xgeXuiActiveContext;
static float g_fXgeXuiActiveDipScale = 1.0f;

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

static void __xgeXuiDefaultDrawTextRect(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags, void* pUser)
{
	(void)pUser;
	xgeTextDrawRect(pFont, sText, tRect, iColor, iFlags);
}

static xge_vec2_t __xgeXuiDefaultMeasureText(xge_font pFont, const char* sText, void* pUser)
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
	xge_xui_paint_command_t tCommand;

	memset(&tCommand, 0, sizeof(tCommand));
	tCommand.iType = XGE_XUI_PAINT_RECT;
	tCommand.tRect = tRect;
	tCommand.iColor = iColor;
	__xgeXuiPaintCommandFlush(&tCommand);
	pHost = __xgeXuiHostGet();
	if ( pHost->draw_rect != NULL ) {
		pHost->draw_rect(tRect, iColor, pHost->pUser);
	}
}

static void __xgeXuiHostDrawBorderRect(xge_rect_t tRect, float fWidth, uint32_t iColor)
{
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
	xge_xui_paint_command_t tCommand;

	memset(&tCommand, 0, sizeof(tCommand));
	tCommand.iType = XGE_XUI_PAINT_ROUNDED_RECT;
	tCommand.tRect = tRect;
	tCommand.iColor = iColor;
	tCommand.fRadius = fRadius;
	__xgeXuiPaintCommandFlush(&tCommand);
	pHost = __xgeXuiHostGet();
	if ( pHost->draw_rect != NULL ) {
		pHost->draw_rect(tRect, iColor, pHost->pUser);
	}
}

static void __xgeXuiHostDrawImage(const xge_draw_t* pDraw)
{
	const xge_xui_host_t* pHost;
	xge_xui_paint_command_t tCommand;

	memset(&tCommand, 0, sizeof(tCommand));
	tCommand.iType = XGE_XUI_PAINT_IMAGE;
	if ( pDraw != NULL ) {
		tCommand.tDraw = *pDraw;
	}
	__xgeXuiPaintCommandFlush(&tCommand);
	pHost = __xgeXuiHostGet();
	if ( pHost->draw_image != NULL ) {
		pHost->draw_image(pDraw, pHost->pUser);
	}
}

static void __xgeXuiHostDrawTextRect(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	const xge_xui_host_t* pHost;
	xge_xui_paint_command_t tCommand;

	memset(&tCommand, 0, sizeof(tCommand));
	tCommand.iType = XGE_XUI_PAINT_TEXT;
	tCommand.pFont = pFont;
	tCommand.sText = sText;
	tCommand.tRect = tRect;
	tCommand.iColor = iColor;
	tCommand.iFlags = iFlags;
	__xgeXuiPaintCommandFlush(&tCommand);
	pHost = __xgeXuiHostGet();
	if ( pHost->draw_text_rect != NULL ) {
		pHost->draw_text_rect(pFont, sText, tRect, iColor, iFlags, pHost->pUser);
	}
}

static xge_vec2_t __xgeXuiHostMeasureText(xge_font pFont, const char* sText)
{
	const xge_xui_host_t* pHost;
	xge_vec2_t tSize;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	pHost = __xgeXuiHostGet();
	if ( pHost->measure_text != NULL ) {
		return pHost->measure_text(pFont, sText, pHost->pUser);
	}
	return tSize;
}

static void __xgeXuiHostClipSet(xge_rect_t tRect)
{
	const xge_xui_host_t* pHost;
	xge_xui_paint_command_t tCommand;

	memset(&tCommand, 0, sizeof(tCommand));
	tCommand.iType = XGE_XUI_PAINT_CLIP_SET;
	tCommand.tRect = tRect;
	__xgeXuiPaintCommandFlush(&tCommand);
	pHost = __xgeXuiHostGet();
	if ( pHost->clip_set != NULL ) {
		pHost->clip_set(tRect, pHost->pUser);
	}
}

static void __xgeXuiHostClipClear(void)
{
	const xge_xui_host_t* pHost;
	xge_xui_paint_command_t tCommand;

	memset(&tCommand, 0, sizeof(tCommand));
	tCommand.iType = XGE_XUI_PAINT_CLIP_CLEAR;
	__xgeXuiPaintCommandFlush(&tCommand);
	pHost = __xgeXuiHostGet();
	if ( pHost->clip_clear != NULL ) {
		pHost->clip_clear(pHost->pUser);
	}
}
