#include "xui_internal.h"

static float __xuiWindowFrameMin(float a, float b)
{
	return (a < b) ? a : b;
}

static float __xuiWindowFrameMax(float a, float b)
{
	return (a > b) ? a : b;
}

static int __xuiWindowFrameContains(xui_rect_t r, float x, float y)
{
	return x >= r.fX && y >= r.fY && x < r.fX + r.fW && y < r.fY + r.fH;
}

void xuiInternalWindowFrameLayout(xui_rect_t tFrameRect,
	const xui_internal_window_frame_metrics_t* pMetrics,
	xui_internal_window_frame_layout_t* pLayout)
{
	float border;
	float title;
	if ( pLayout == NULL ) return;
	pLayout->tFrameRect = xuiInternalSnapRect(tFrameRect);
	pLayout->tTitleBarRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	pLayout->tClientRect = pLayout->tFrameRect;
	if ( pMetrics == NULL ) return;
	border = __xuiWindowFrameMax(0.0f, pMetrics->fBorderWidth);
	title = pMetrics->bShowTitleBar ? __xuiWindowFrameMax(0.0f, pMetrics->fTitleBarHeight) : 0.0f;
	title = __xuiWindowFrameMin(title, __xuiWindowFrameMax(0.0f, tFrameRect.fH - border * 2.0f));
	pLayout->tTitleBarRect = xuiInternalRectFromFloatNearest(
		tFrameRect.fX + border,
		tFrameRect.fY + border,
		__xuiWindowFrameMax(0.0f, tFrameRect.fW - border * 2.0f),
		title
	);
	pLayout->tClientRect = xuiInternalRectFromFloatNearest(
		tFrameRect.fX + border,
		tFrameRect.fY + border + title,
		__xuiWindowFrameMax(0.0f, tFrameRect.fW - border * 2.0f),
		__xuiWindowFrameMax(0.0f, tFrameRect.fH - border * 2.0f - title)
	);
}

xui_rect_t xuiInternalWindowFrameTrailingButton(const xui_internal_window_frame_layout_t* pLayout,
	const xui_internal_window_frame_metrics_t* pMetrics, int iTrailingIndex)
{
	xui_rect_t r = {0.0f, 0.0f, 0.0f, 0.0f};
	float size;
	float gap;
	float inset;
	float x;
	float y;
	if ( pLayout == NULL || pMetrics == NULL || !pMetrics->bShowTitleBar || iTrailingIndex < 0 ) return r;
	size = __xuiWindowFrameMax(0.0f, pMetrics->fButtonSize);
	size = __xuiWindowFrameMin(size, pLayout->tTitleBarRect.fH);
	gap = __xuiWindowFrameMax(0.0f, pMetrics->fButtonGap);
	inset = __xuiWindowFrameMax(0.0f, pMetrics->fButtonInset);
	x = pLayout->tTitleBarRect.fX + pLayout->tTitleBarRect.fW - inset - size - (float)iTrailingIndex * (size + gap);
	y = pLayout->tTitleBarRect.fY + (pLayout->tTitleBarRect.fH - size) * 0.5f;
	if ( x < pLayout->tTitleBarRect.fX ) return r;
	return xuiInternalRectFromFloatNearest(x, y, size, size);
}

uint32_t xuiInternalWindowFrameResizeEdgesAt(xui_rect_t tFrameRect,
	const xui_internal_window_frame_metrics_t* pMetrics, float fX, float fY)
{
	float grip;
	float topGrip;
	uint32_t edges;
	if ( pMetrics == NULL || !pMetrics->bResizable || !__xuiWindowFrameContains(tFrameRect, fX, fY) ) return 0u;
	grip = __xuiWindowFrameMax(2.0f, pMetrics->fResizeGrip);
	topGrip = pMetrics->bShowTitleBar ? __xuiWindowFrameMin(4.0f, grip) : grip;
	edges = 0u;
	if ( fX - tFrameRect.fX <= grip ) edges |= XUI_WINDOW_EDGE_LEFT;
	if ( tFrameRect.fX + tFrameRect.fW - fX <= grip ) edges |= XUI_WINDOW_EDGE_RIGHT;
	if ( fY - tFrameRect.fY <= topGrip ) edges |= XUI_WINDOW_EDGE_TOP;
	if ( tFrameRect.fY + tFrameRect.fH - fY <= grip ) edges |= XUI_WINDOW_EDGE_BOTTOM;
	return edges & pMetrics->iResizeEdges;
}

int xuiInternalWindowFrameResizeCursor(uint32_t iEdges)
{
	if ( (iEdges & (XUI_WINDOW_EDGE_LEFT | XUI_WINDOW_EDGE_RIGHT)) != 0u &&
	     (iEdges & (XUI_WINDOW_EDGE_TOP | XUI_WINDOW_EDGE_BOTTOM)) != 0u ) {
		return (((iEdges & XUI_WINDOW_EDGE_LEFT) != 0u) == ((iEdges & XUI_WINDOW_EDGE_TOP) != 0u)) ?
			XUI_CURSOR_RESIZE_NWSE : XUI_CURSOR_RESIZE_NESW;
	}
	if ( (iEdges & (XUI_WINDOW_EDGE_LEFT | XUI_WINDOW_EDGE_RIGHT)) != 0u ) return XUI_CURSOR_RESIZE_EW;
	if ( (iEdges & (XUI_WINDOW_EDGE_TOP | XUI_WINDOW_EDGE_BOTTOM)) != 0u ) return XUI_CURSOR_RESIZE_NS;
	return XUI_CURSOR_INHERIT;
}

static xui_rect_t __xuiWindowFrameClampFloat(float x, float y, float w, float h,
	xui_rect_t bounds, float fMinWidth, float fMinHeight)
{
	float right;
	float bottom;
	fMinWidth = __xuiWindowFrameMax(0.0f, fMinWidth);
	fMinHeight = __xuiWindowFrameMax(0.0f, fMinHeight);
	w = __xuiWindowFrameMax(w, fMinWidth);
	h = __xuiWindowFrameMax(h, fMinHeight);
	if ( bounds.fW > 0.0f && w > bounds.fW && bounds.fW >= fMinWidth ) w = bounds.fW;
	if ( bounds.fH > 0.0f && h > bounds.fH && bounds.fH >= fMinHeight ) h = bounds.fH;
	if ( x < bounds.fX ) x = bounds.fX;
	if ( y < bounds.fY ) y = bounds.fY;
	right = bounds.fX + bounds.fW;
	bottom = bounds.fY + bounds.fH;
	if ( bounds.fW > 0.0f && x + w > right ) x = __xuiWindowFrameMax(bounds.fX, right - w);
	if ( bounds.fH > 0.0f && y + h > bottom ) y = __xuiWindowFrameMax(bounds.fY, bottom - h);
	return xuiInternalRectFromFloatNearest(x, y, w, h);
}

xui_rect_t xuiInternalWindowFrameClamp(xui_rect_t r, xui_rect_t bounds, float fMinWidth, float fMinHeight)
{
	return __xuiWindowFrameClampFloat(r.fX, r.fY, r.fW, r.fH, bounds, fMinWidth, fMinHeight);
}

xui_rect_t xuiInternalWindowFrameMove(xui_rect_t r, xui_rect_t bounds, float fDX, float fDY)
{
	float x = r.fX + fDX;
	float y = r.fY + fDY;
	if ( bounds.fW > 0.0f ) {
		if ( r.fW >= bounds.fW ) x = bounds.fX;
		else {
			x = __xuiWindowFrameMax(bounds.fX, x);
			x = __xuiWindowFrameMin(bounds.fX + bounds.fW - r.fW, x);
		}
	}
	if ( bounds.fH > 0.0f ) {
		if ( r.fH >= bounds.fH ) y = bounds.fY;
		else {
			y = __xuiWindowFrameMax(bounds.fY, y);
			y = __xuiWindowFrameMin(bounds.fY + bounds.fH - r.fH, y);
		}
	}
	return xuiInternalRectFromFloatNearest(x, y, r.fW, r.fH);
}

xui_rect_t xuiInternalWindowFrameResize(xui_rect_t r, xui_rect_t bounds, uint32_t iEdges,
	float fDX, float fDY, float fMinWidth, float fMinHeight)
{
	float right = r.fX + r.fW;
	float bottom = r.fY + r.fH;
	float x = r.fX, y = r.fY, w = r.fW, h = r.fH;
	if ( (iEdges & XUI_WINDOW_EDGE_LEFT) != 0u ) {
		x += fDX;
		w -= fDX;
		if ( w < fMinWidth ) { w = fMinWidth; x = right - fMinWidth; }
	}
	if ( (iEdges & XUI_WINDOW_EDGE_RIGHT) != 0u ) {
		w += fDX;
		if ( w < fMinWidth ) w = fMinWidth;
	}
	if ( (iEdges & XUI_WINDOW_EDGE_TOP) != 0u ) {
		y += fDY;
		h -= fDY;
		if ( h < fMinHeight ) { h = fMinHeight; y = bottom - fMinHeight; }
	}
	if ( (iEdges & XUI_WINDOW_EDGE_BOTTOM) != 0u ) {
		h += fDY;
		if ( h < fMinHeight ) h = fMinHeight;
	}
	return __xuiWindowFrameClampFloat(x, y, w, h, bounds, fMinWidth, fMinHeight);
}
