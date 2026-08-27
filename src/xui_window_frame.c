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
	pLayout->tTitleBarRect = xuiInternalSnapRect((xui_rect_t){
		tFrameRect.fX + border,
		tFrameRect.fY + border,
		__xuiWindowFrameMax(0.0f, tFrameRect.fW - border * 2.0f),
		title
	});
	pLayout->tClientRect = xuiInternalSnapRect((xui_rect_t){
		tFrameRect.fX + border,
		tFrameRect.fY + border + title,
		__xuiWindowFrameMax(0.0f, tFrameRect.fW - border * 2.0f),
		__xuiWindowFrameMax(0.0f, tFrameRect.fH - border * 2.0f - title)
	});
}

xui_rect_t xuiInternalWindowFrameTrailingButton(const xui_internal_window_frame_layout_t* pLayout,
	const xui_internal_window_frame_metrics_t* pMetrics, int iTrailingIndex)
{
	xui_rect_t r = {0.0f, 0.0f, 0.0f, 0.0f};
	float size;
	float gap;
	float inset;
	if ( pLayout == NULL || pMetrics == NULL || !pMetrics->bShowTitleBar || iTrailingIndex < 0 ) return r;
	size = __xuiWindowFrameMax(0.0f, pMetrics->fButtonSize);
	size = __xuiWindowFrameMin(size, pLayout->tTitleBarRect.fH);
	gap = __xuiWindowFrameMax(0.0f, pMetrics->fButtonGap);
	inset = __xuiWindowFrameMax(0.0f, pMetrics->fButtonInset);
	r.fW = size;
	r.fH = size;
	r.fX = pLayout->tTitleBarRect.fX + pLayout->tTitleBarRect.fW - inset - size - (float)iTrailingIndex * (size + gap);
	r.fY = pLayout->tTitleBarRect.fY + (pLayout->tTitleBarRect.fH - size) * 0.5f;
	if ( r.fX < pLayout->tTitleBarRect.fX ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	return xuiInternalSnapRect(r);
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

xui_rect_t xuiInternalWindowFrameClamp(xui_rect_t r, xui_rect_t bounds, float fMinWidth, float fMinHeight)
{
	float right;
	float bottom;
	fMinWidth = __xuiWindowFrameMax(0.0f, fMinWidth);
	fMinHeight = __xuiWindowFrameMax(0.0f, fMinHeight);
	r.fW = __xuiWindowFrameMax(r.fW, fMinWidth);
	r.fH = __xuiWindowFrameMax(r.fH, fMinHeight);
	if ( bounds.fW > 0.0f && r.fW > bounds.fW && bounds.fW >= fMinWidth ) r.fW = bounds.fW;
	if ( bounds.fH > 0.0f && r.fH > bounds.fH && bounds.fH >= fMinHeight ) r.fH = bounds.fH;
	if ( r.fX < bounds.fX ) r.fX = bounds.fX;
	if ( r.fY < bounds.fY ) r.fY = bounds.fY;
	right = bounds.fX + bounds.fW;
	bottom = bounds.fY + bounds.fH;
	if ( bounds.fW > 0.0f && r.fX + r.fW > right ) r.fX = __xuiWindowFrameMax(bounds.fX, right - r.fW);
	if ( bounds.fH > 0.0f && r.fY + r.fH > bottom ) r.fY = __xuiWindowFrameMax(bounds.fY, bottom - r.fH);
	return xuiInternalSnapRect(r);
}

xui_rect_t xuiInternalWindowFrameMove(xui_rect_t r, xui_rect_t bounds, float fDX, float fDY)
{
	r.fX += fDX;
	r.fY += fDY;
	if ( bounds.fW > 0.0f ) {
		if ( r.fW >= bounds.fW ) r.fX = bounds.fX;
		else {
			r.fX = __xuiWindowFrameMax(bounds.fX, r.fX);
			r.fX = __xuiWindowFrameMin(bounds.fX + bounds.fW - r.fW, r.fX);
		}
	}
	if ( bounds.fH > 0.0f ) {
		if ( r.fH >= bounds.fH ) r.fY = bounds.fY;
		else {
			r.fY = __xuiWindowFrameMax(bounds.fY, r.fY);
			r.fY = __xuiWindowFrameMin(bounds.fY + bounds.fH - r.fH, r.fY);
		}
	}
	return xuiInternalSnapRect(r);
}

xui_rect_t xuiInternalWindowFrameResize(xui_rect_t r, xui_rect_t bounds, uint32_t iEdges,
	float fDX, float fDY, float fMinWidth, float fMinHeight)
{
	float right = r.fX + r.fW;
	float bottom = r.fY + r.fH;
	if ( (iEdges & XUI_WINDOW_EDGE_LEFT) != 0u ) {
		r.fX += fDX;
		r.fW -= fDX;
		if ( r.fW < fMinWidth ) { r.fW = fMinWidth; r.fX = right - fMinWidth; }
	}
	if ( (iEdges & XUI_WINDOW_EDGE_RIGHT) != 0u ) {
		r.fW += fDX;
		if ( r.fW < fMinWidth ) r.fW = fMinWidth;
	}
	if ( (iEdges & XUI_WINDOW_EDGE_TOP) != 0u ) {
		r.fY += fDY;
		r.fH -= fDY;
		if ( r.fH < fMinHeight ) { r.fH = fMinHeight; r.fY = bottom - fMinHeight; }
	}
	if ( (iEdges & XUI_WINDOW_EDGE_BOTTOM) != 0u ) {
		r.fH += fDY;
		if ( r.fH < fMinHeight ) r.fH = fMinHeight;
	}
	return xuiInternalWindowFrameClamp(r, bounds, fMinWidth, fMinHeight);
}
