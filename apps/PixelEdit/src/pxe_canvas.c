#include "pxe_app.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ─── Set pixel ─── */
void PixelEditSetPixel(pxe_app_t* pApp, int iX, int iY, uint32_t iColor)
{
	pxe_layer_t* pL;
	if (!pApp || iX < 0 || iY < 0 || iX >= pApp->iCanvasW || iY >= pApp->iCanvasH) return;
	if (pApp->iActiveLayer < 0 || pApp->iActiveLayer >= pApp->iLayerCount) return;
	pL = &pApp->arrLayers[pApp->iActiveLayer];
	pL->arrPixels[iY * PXE_CANVAS_MAX + iX] = iColor;
}

/* ─── Bresenham line ─── */
void PixelEditDrawLine(pxe_app_t* pApp, int iX0, int iY0, int iX1, int iY1, uint32_t iColor)
{
	int dx = abs(iX1 - iX0), dy = abs(iY1 - iY0);
	int sx = iX0 < iX1 ? 1 : -1, sy = iY0 < iY1 ? 1 : -1;
	int err = dx - dy;
	for (;;) {
		PixelEditSetPixel(pApp, iX0, iY0, iColor);
		if (iX0 == iX1 && iY0 == iY1) break;
		if (2 * err > -dy) { err -= dy; iX0 += sx; }
		if (2 * err < dx) { err += dx; iY0 += sy; }
	}
}

/* ─── Flood fill ─── */
void PixelEditFloodFill(pxe_app_t* pApp, int iX, int iY, uint32_t iColor)
{
	pxe_layer_t* pL;
	uint32_t iTarget;
	int arrX[PXE_CANVAS_MAX * PXE_CANVAS_MAX];
	int arrY[PXE_CANVAS_MAX * PXE_CANVAS_MAX];
	int iHead = 0, iTail = 0;
	if (!pApp || iX < 0 || iY < 0 || iX >= pApp->iCanvasW || iY >= pApp->iCanvasH) return;
	pL = &pApp->arrLayers[pApp->iActiveLayer];
	iTarget = pL->arrPixels[iY * PXE_CANVAS_MAX + iX];
	if (iTarget == iColor) return;
	arrX[iTail] = iX; arrY[iTail] = iY; iTail++;
	while (iHead < iTail) {
		int cx = arrX[iHead], cy = arrY[iHead]; iHead++;
		if (cx < 0 || cy < 0 || cx >= pApp->iCanvasW || cy >= pApp->iCanvasH) continue;
		if (pL->arrPixels[cy * PXE_CANVAS_MAX + cx] != iTarget) continue;
		pL->arrPixels[cy * PXE_CANVAS_MAX + cx] = iColor;
		arrX[iTail] = cx+1; arrY[iTail] = cy; iTail++;
		arrX[iTail] = cx-1; arrY[iTail] = cy; iTail++;
		arrX[iTail] = cx; arrY[iTail] = cy+1; iTail++;
		arrX[iTail] = cx; arrY[iTail] = cy-1; iTail++;
	}
}

/* ─── Canvas render ─── */
int PixelEditCanvasRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	pxe_app_t* pApp = (pxe_app_t*)pUser;
	xui_proxy_t proxy;
	xui_rect_t r;
	float fCellW, fCellH, fOffX, fOffY;
	int iX, iY, iL;
	(void)iStateId;
	if (!pApp || !pDraw) return XUI_ERROR_INVALID_ARGUMENT;
	proxy = xuiProxyXge();
	r = xuiWidgetGetContentRect(pWidget);

	/* Background */
	(void)proxy.drawRectFill(&proxy, pDraw, r, XUI_COLOR_RGBA(40,40,50,255));

	/* Calculate cell size to fit canvas in widget */
	fCellW = (r.fW - 20.0f) / (float)pApp->iCanvasW;
	fCellH = (r.fH - 20.0f) / (float)pApp->iCanvasH;
	if (fCellW > fCellH) fCellW = fCellH;
	else fCellH = fCellW;
	if (fCellW < 1.0f) fCellW = 1.0f;
	if (fCellH < 1.0f) fCellH = 1.0f;
	fOffX = r.fX + (r.fW - fCellW * pApp->iCanvasW) * 0.5f;
	fOffY = r.fY + (r.fH - fCellH * pApp->iCanvasH) * 0.5f;

	/* Checkerboard background */
	for (iY = 0; iY < pApp->iCanvasH; iY++) {
		for (iX = 0; iX < pApp->iCanvasW; iX++) {
			uint32_t iBg = ((iX + iY) & 1) ? XUI_COLOR_RGBA(60,60,70,255) : XUI_COLOR_RGBA(50,50,60,255);
			(void)proxy.drawRectFill(&proxy, pDraw, (xui_rect_t){fOffX + iX*fCellW, fOffY + iY*fCellH, fCellW, fCellH}, iBg);
		}
	}

	/* Draw layers (bottom to top) */
	for (iL = 0; iL < pApp->iLayerCount; iL++) {
		pxe_layer_t* pL = &pApp->arrLayers[iL];
		if (!pL->bVisible) continue;
		for (iY = 0; iY < pApp->iCanvasH; iY++) {
			for (iX = 0; iX < pApp->iCanvasW; iX++) {
				uint32_t px = pL->arrPixels[iY * PXE_CANVAS_MAX + iX];
				if ((px & 0xFF) == 0) continue; /* skip transparent */
				(void)proxy.drawRectFill(&proxy, pDraw, (xui_rect_t){fOffX + iX*fCellW, fOffY + iY*fCellH, fCellW, fCellH}, px);
			}
		}
	}

	/* Grid lines */
	if (fCellW >= 4.0f) {
		for (iX = 0; iX <= pApp->iCanvasW; iX++)
			(void)proxy.drawRectFill(&proxy, pDraw, (xui_rect_t){fOffX + iX*fCellW, fOffY, 1.0f, fCellH * pApp->iCanvasH}, XUI_COLOR_RGBA(80,80,100,100));
		for (iY = 0; iY <= pApp->iCanvasH; iY++)
			(void)proxy.drawRectFill(&proxy, pDraw, (xui_rect_t){fOffX, fOffY + iY*fCellH, fCellW * pApp->iCanvasW, 1.0f}, XUI_COLOR_RGBA(80,80,100,100));
	}

	/* Border */
	(void)proxy.drawRectStroke(&proxy, pDraw, (xui_rect_t){fOffX, fOffY, fCellW*pApp->iCanvasW, fCellH*pApp->iCanvasH}, 1.0f, XUI_COLOR_RGBA(120,140,180,255));
	return XUI_OK;
}

/* ─── Canvas event ─── */
int PixelEditCanvasEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	pxe_app_t* pApp = (pxe_app_t*)pUser;
	xui_rect_t r;
	float fCellW, fCellH, fOffX, fOffY;
	int iPX, iPY;
	if (!pApp || !pEvent) return XUI_OK;
	r = xuiWidgetGetContentRect(pWidget);
	fCellW = (r.fW - 20.0f) / (float)pApp->iCanvasW;
	fCellH = (r.fH - 20.0f) / (float)pApp->iCanvasH;
	if (fCellW > fCellH) fCellW = fCellH; else fCellH = fCellW;
	if (fCellW < 1.0f) fCellW = 1.0f;
	if (fCellH < 1.0f) fCellH = 1.0f;
	fOffX = r.fX + (r.fW - fCellW * pApp->iCanvasW) * 0.5f;
	fOffY = r.fY + (r.fH - fCellH * pApp->iCanvasH) * 0.5f;
	iPX = (int)((pEvent->fX - fOffX) / fCellW);
	iPY = (int)((pEvent->fY - fOffY) / fCellH);

	switch (pEvent->iType) {
	case XUI_EVENT_POINTER_DOWN:
		if (pEvent->iButton == XUI_POINTER_BUTTON_LEFT) {
			pApp->bDrawing = 1;
			pApp->iLastX = iPX; pApp->iLastY = iPY;
			if (pApp->iTool == PXE_TOOL_PENCIL) PixelEditSetPixel(pApp, iPX, iPY, pApp->iColor);
			else if (pApp->iTool == PXE_TOOL_ERASER) PixelEditSetPixel(pApp, iPX, iPY, 0);
			else if (pApp->iTool == PXE_TOOL_FILL) PixelEditFloodFill(pApp, iPX, iPY, pApp->iColor);
			else if (pApp->iTool == PXE_TOOL_PICKER) {
				if (iPX >= 0 && iPY >= 0 && iPX < pApp->iCanvasW && iPY < pApp->iCanvasH)
					pApp->iColor = pApp->arrLayers[pApp->iActiveLayer].arrPixels[iPY * PXE_CANVAS_MAX + iPX];
			}
			(void)xuiWidgetInvalidate(pApp->pCanvasWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		break;
	case XUI_EVENT_POINTER_MOVE:
		if (pApp->bDrawing) {
			if (pApp->iTool == PXE_TOOL_PENCIL || pApp->iTool == PXE_TOOL_ERASER) {
				uint32_t c = (pApp->iTool == PXE_TOOL_PENCIL) ? pApp->iColor : 0;
				PixelEditDrawLine(pApp, pApp->iLastX, pApp->iLastY, iPX, iPY, c);
				pApp->iLastX = iPX; pApp->iLastY = iPY;
			}
			(void)xuiWidgetInvalidate(pApp->pCanvasWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		break;
	case XUI_EVENT_POINTER_UP:
		if (pApp->bDrawing) {
			pApp->bDrawing = 0;
			if (pApp->iTool == PXE_TOOL_LINE) PixelEditDrawLine(pApp, pApp->iLastX, pApp->iLastY, iPX, iPY, pApp->iColor);
			else if (pApp->iTool == PXE_TOOL_RECT) {
				int x0 = pApp->iLastX < iPX ? pApp->iLastX : iPX;
				int y0 = pApp->iLastY < iPY ? pApp->iLastY : iPY;
				int x1 = pApp->iLastX > iPX ? pApp->iLastX : iPX;
				int y1 = pApp->iLastY > iPY ? pApp->iLastY : iPY;
				PixelEditDrawLine(pApp, x0, y0, x1, y0, pApp->iColor);
				PixelEditDrawLine(pApp, x1, y0, x1, y1, pApp->iColor);
				PixelEditDrawLine(pApp, x1, y1, x0, y1, pApp->iColor);
				PixelEditDrawLine(pApp, x0, y1, x0, y0, pApp->iColor);
			}
			(void)xuiWidgetInvalidate(pApp->pCanvasWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		break;
	case XUI_EVENT_POINTER_WHEEL:
		if (pEvent->fWheelY > 0) pApp->iZoom += 2;
		else if (pEvent->fWheelY < 0) pApp->iZoom -= 2;
		if (pApp->iZoom < 4) pApp->iZoom = 4;
		if (pApp->iZoom > 48) pApp->iZoom = 48;
		(void)xuiWidgetInvalidate(pApp->pCanvasWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		break;
	default: break;
	}
	return XUI_OK;
}

/* ─── Palette render ─── */
int PixelEditPaletteRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	pxe_app_t* pApp = (pxe_app_t*)pUser;
	xui_proxy_t proxy;
	xui_rect_t r;
	float fCellSize, fCols, fY;
	int i;
	(void)iStateId;
	if (!pApp || !pDraw) return XUI_ERROR_INVALID_ARGUMENT;
	proxy = xuiProxyXge();
	r = xuiWidgetGetContentRect(pWidget);
	(void)proxy.drawRectFill(&proxy, pDraw, r, XUI_COLOR_RGBA(245,248,252,255));

	fCols = 4.0f;
	fCellSize = (r.fW - 16.0f) / fCols;
	if (fCellSize > 32.0f) fCellSize = 32.0f;
	for (i = 0; i < pApp->iPaletteCount; i++) {
		float cx = r.fX + 8.0f + (float)(i % (int)fCols) * (fCellSize + 2.0f);
		float cy = r.fY + 8.0f + (float)(i / (int)fCols) * (fCellSize + 2.0f);
		(void)proxy.drawRectFill(&proxy, pDraw, (xui_rect_t){cx, cy, fCellSize, fCellSize}, pApp->arrPalette[i]);
		if (pApp->arrPalette[i] == pApp->iColor)
			(void)proxy.drawRectStroke(&proxy, pDraw, (xui_rect_t){cx-1, cy-1, fCellSize+2, fCellSize+2}, 2.0f, XUI_COLOR_RGBA(255,255,255,255));
	}
	/* Current color indicator */
	fY = r.fY + r.fH - 40.0f;
	(void)proxy.drawRectFill(&proxy, pDraw, (xui_rect_t){r.fX+8, fY, 24, 24}, pApp->iColor);
	(void)proxy.drawRectStroke(&proxy, pDraw, (xui_rect_t){r.fX+8, fY, 24, 24}, 1.0f, XUI_COLOR_RGBA(100,100,120,255));
	return XUI_OK;
}

/* ─── Palette event ─── */
int PixelEditPaletteEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	pxe_app_t* pApp = (pxe_app_t*)pUser;
	xui_rect_t r;
	float fCellSize, fCols;
	int iCol, iRow, iIdx;
	if (!pApp || !pEvent || pEvent->iType != XUI_EVENT_POINTER_DOWN) return XUI_OK;
	r = xuiWidgetGetContentRect(pWidget);
	fCols = 4.0f;
	fCellSize = (r.fW - 16.0f) / fCols;
	if (fCellSize > 32.0f) fCellSize = 32.0f;
	iCol = (int)((pEvent->fX - r.fX - 8.0f) / (fCellSize + 2.0f));
	iRow = (int)((pEvent->fY - r.fY - 8.0f) / (fCellSize + 2.0f));
	iIdx = iRow * (int)fCols + iCol;
	if (iIdx >= 0 && iIdx < pApp->iPaletteCount) {
		pApp->iColor = pApp->arrPalette[iIdx];
		(void)xuiWidgetInvalidate(pApp->pPaletteWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return XUI_OK;
}

/* ─── Export PNG ─── */
int PixelEditExportPNG(pxe_app_t* pApp, const char* sPath)
{
	uint32_t arrOut[PXE_CANVAS_MAX * PXE_CANVAS_MAX];
	int iX, iY, iL;
	if (!pApp || !sPath) return XUI_ERROR;
	memset(arrOut, 0, sizeof(arrOut));
	for (iL = 0; iL < pApp->iLayerCount; iL++) {
		pxe_layer_t* pL = &pApp->arrLayers[iL];
		if (!pL->bVisible) continue;
		for (iY = 0; iY < pApp->iCanvasH; iY++)
			for (iX = 0; iX < pApp->iCanvasW; iX++) {
				uint32_t px = pL->arrPixels[iY * PXE_CANVAS_MAX + iX];
				if ((px & 0xFF) != 0) arrOut[iY * PXE_CANVAS_MAX + iX] = px;
			}
	}
	return xgeImageSavePNG(sPath, pApp->iCanvasW, pApp->iCanvasH, arrOut, pApp->iCanvasW * 4);
}
