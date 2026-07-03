#include "tileset_passage.h"
#include "tileset_sets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAPEDIT_TILESET_PASSAGE_SCROLLBAR_SIZE 12.0f
#define MAPEDIT_TILESET_PASSAGE_PLACEHOLDER_ROWS 8
#define MAPEDIT_TILESET_PASSAGE_EXPERT_MIN_CELL 32

static float MapEditTilesetPassageMinFloat(float a, float b);
static unsigned char MapEditTilesetPassageBitForRegion(int iRegionCol, int iRegionRow);

static float MapEditTilesetPassageAbsFloat(float fValue)
{
	return (fValue < 0.0f) ? -fValue : fValue;
}

static int MapEditTilesetPassageRectAlmostEqual(xge_rect_t a, xge_rect_t b)
{
	return MapEditTilesetPassageAbsFloat(a.fX - b.fX) < 0.01f &&
		MapEditTilesetPassageAbsFloat(a.fY - b.fY) < 0.01f &&
		MapEditTilesetPassageAbsFloat(a.fW - b.fW) < 0.01f &&
		MapEditTilesetPassageAbsFloat(a.fH - b.fH) < 0.01f;
}

static int MapEditTilesetPassageRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static int MapEditTilesetPassageWidgetInteractive(xge_xui_widget pWidget)
{
	xge_xui_widget pIt;

	if ( pWidget == NULL ) {
		return 0;
	}
	for ( pIt = pWidget; pIt != NULL; pIt = pIt->pParent ) {
		if ( !xgeXuiWidgetIsVisible(pIt) || !xgeXuiWidgetIsEnabled(pIt) ) {
			return 0;
		}
	}
	return 1;
}

static int MapEditTilesetPassageMouseGridEvent(const xge_event_t* pEvent)
{
	if ( pEvent == NULL ) {
		return 0;
	}
	return (pEvent->iType == XGE_EVENT_MOUSE_DOWN && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_UP && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_MOVE);
}

static void MapEditTilesetPassageClearOverlayTextures(mapedit_app_t* pApp)
{
	int i;

	if ( pApp == NULL ) {
		return;
	}
	for ( i = 0; i < 256; i++ ) {
		if ( pApp->arrTilesetPassageOverlayTexture[i] != NULL ) {
			xgeXuiTextureDestroy(&pApp->tXui, pApp->arrTilesetPassageOverlayTexture[i]);
			pApp->arrTilesetPassageOverlayTexture[i] = NULL;
		}
	}
	pApp->iTilesetPassageOverlayCellWidth = 0;
	pApp->iTilesetPassageOverlayCellHeight = 0;
}

static int MapEditTilesetPassageClampInt(int iValue, int iMin, int iMax)
{
	if ( iValue < iMin ) {
		return iMin;
	}
	if ( iValue > iMax ) {
		return iMax;
	}
	return iValue;
}

static void MapEditTilesetPassagePutPixel(uint32_t* pPixels, int iWidth, int iHeight, int x, int y, uint32_t iColor)
{
	if ( pPixels == NULL || x < 0 || y < 0 || x >= iWidth || y >= iHeight ) {
		return;
	}
	pPixels[y * iWidth + x] = iColor;
}

static void MapEditTilesetPassageDrawPixelLine(uint32_t* pPixels, int iWidth, int iHeight, float fX0, float fY0, float fX1, float fY1, float fLineWidth, uint32_t iColor)
{
	float fDX;
	float fDY;
	float fLen2;
	float fHalf;
	float fLimit2;
	float fMinX;
	float fMaxX;
	float fMinY;
	float fMaxY;
	int iStartX;
	int iEndX;
	int iStartY;
	int iEndY;
	int x;
	int y;

	if ( pPixels == NULL || iWidth <= 0 || iHeight <= 0 ) {
		return;
	}
	fDX = fX1 - fX0;
	fDY = fY1 - fY0;
	fLen2 = fDX * fDX + fDY * fDY;
	if ( fLen2 <= 0.0001f ) {
		return;
	}
	fHalf = fLineWidth * 0.5f;
	fLimit2 = fHalf * fHalf;
	fMinX = (fX0 < fX1) ? fX0 : fX1;
	fMaxX = (fX0 > fX1) ? fX0 : fX1;
	fMinY = (fY0 < fY1) ? fY0 : fY1;
	fMaxY = (fY0 > fY1) ? fY0 : fY1;
	iStartX = MapEditTilesetPassageClampInt((int)(fMinX - fLineWidth - 1.0f), 0, iWidth - 1);
	iEndX = MapEditTilesetPassageClampInt((int)(fMaxX + fLineWidth + 1.0f), 0, iWidth - 1);
	iStartY = MapEditTilesetPassageClampInt((int)(fMinY - fLineWidth - 1.0f), 0, iHeight - 1);
	iEndY = MapEditTilesetPassageClampInt((int)(fMaxY + fLineWidth + 1.0f), 0, iHeight - 1);
	for ( y = iStartY; y <= iEndY; y++ ) {
		for ( x = iStartX; x <= iEndX; x++ ) {
			float fPX;
			float fPY;
			float fT;
			float fCX;
			float fCY;
			float fDDX;
			float fDDY;

			fPX = (float)x + 0.5f;
			fPY = (float)y + 0.5f;
			fT = ((fPX - fX0) * fDX + (fPY - fY0) * fDY) / fLen2;
			if ( fT < 0.0f ) {
				fT = 0.0f;
			} else if ( fT > 1.0f ) {
				fT = 1.0f;
			}
			fCX = fX0 + fDX * fT;
			fCY = fY0 + fDY * fT;
			fDDX = fPX - fCX;
			fDDY = fPY - fCY;
			if ( fDDX * fDDX + fDDY * fDDY <= fLimit2 ) {
				MapEditTilesetPassagePutPixel(pPixels, iWidth, iHeight, x, y, iColor);
			}
		}
	}
}

static void MapEditTilesetPassageDrawPixelCircle(uint32_t* pPixels, int iWidth, int iHeight, float fCX, float fCY, float fRadius, uint32_t iColor)
{
	float fR2;
	int iStartX;
	int iEndX;
	int iStartY;
	int iEndY;
	int x;
	int y;

	if ( pPixels == NULL || iWidth <= 0 || iHeight <= 0 ) {
		return;
	}
	fR2 = fRadius * fRadius;
	iStartX = MapEditTilesetPassageClampInt((int)(fCX - fRadius - 1.0f), 0, iWidth - 1);
	iEndX = MapEditTilesetPassageClampInt((int)(fCX + fRadius + 1.0f), 0, iWidth - 1);
	iStartY = MapEditTilesetPassageClampInt((int)(fCY - fRadius - 1.0f), 0, iHeight - 1);
	iEndY = MapEditTilesetPassageClampInt((int)(fCY + fRadius + 1.0f), 0, iHeight - 1);
	for ( y = iStartY; y <= iEndY; y++ ) {
		for ( x = iStartX; x <= iEndX; x++ ) {
			float fDX = ((float)x + 0.5f) - fCX;
			float fDY = ((float)y + 0.5f) - fCY;
			if ( fDX * fDX + fDY * fDY <= fR2 ) {
				MapEditTilesetPassagePutPixel(pPixels, iWidth, iHeight, x, y, iColor);
			}
		}
	}
}

static void MapEditTilesetPassageDrawPixelRectStroke(uint32_t* pPixels, int iWidth, int iHeight, float fX, float fY, float fW, float fH, float fLineWidth, uint32_t iColor)
{
	MapEditTilesetPassageDrawPixelLine(pPixels, iWidth, iHeight, fX, fY, fX + fW, fY, fLineWidth, iColor);
	MapEditTilesetPassageDrawPixelLine(pPixels, iWidth, iHeight, fX + fW, fY, fX + fW, fY + fH, fLineWidth, iColor);
	MapEditTilesetPassageDrawPixelLine(pPixels, iWidth, iHeight, fX + fW, fY + fH, fX, fY + fH, fLineWidth, iColor);
	MapEditTilesetPassageDrawPixelLine(pPixels, iWidth, iHeight, fX, fY + fH, fX, fY, fLineWidth, iColor);
}

static void MapEditTilesetPassageDrawPixelTriangleStroke(uint32_t* pPixels, int iWidth, int iHeight, xge_vec2_t a, xge_vec2_t b, xge_vec2_t c, float fLineWidth, uint32_t iColor)
{
	MapEditTilesetPassageDrawPixelLine(pPixels, iWidth, iHeight, a.fX, a.fY, b.fX, b.fY, fLineWidth, iColor);
	MapEditTilesetPassageDrawPixelLine(pPixels, iWidth, iHeight, b.fX, b.fY, c.fX, c.fY, fLineWidth, iColor);
	MapEditTilesetPassageDrawPixelLine(pPixels, iWidth, iHeight, c.fX, c.fY, a.fX, a.fY, fLineWidth, iColor);
}

static void MapEditTilesetPassageDrawPixelForbiddenTriangle(uint32_t* pPixels, int iWidth, int iHeight, int iRegionCol, int iRegionRow)
{
	float fSize;
	float fPad;
	float fTri;
	float fW;
	float fH;
	uint32_t iRed;
	xge_vec2_t a;
	xge_vec2_t b;
	xge_vec2_t c;

	fW = (float)iWidth;
	fH = (float)iHeight;
	fSize = MapEditTilesetPassageMinFloat(fW, fH);
	fPad = (fSize <= 16.0f) ? 2.0f : 3.0f;
	fTri = fSize * 0.36f;
	if ( fTri < 5.0f ) {
		fTri = 5.0f;
	}
	iRed = XGE_COLOR_RGBA(232, 26, 36, 225);
	if ( iRegionRow == 0 && iRegionCol == 0 ) {
		a = (xge_vec2_t){ fPad, fPad };
		b = (xge_vec2_t){ fPad + fTri, fPad };
		c = (xge_vec2_t){ fPad, fPad + fTri };
	} else if ( iRegionRow == 0 && iRegionCol == 1 ) {
		a = (xge_vec2_t){ fW * 0.5f, fPad + fTri };
		b = (xge_vec2_t){ fW * 0.5f - fTri * 0.5f, fPad };
		c = (xge_vec2_t){ fW * 0.5f + fTri * 0.5f, fPad };
	} else if ( iRegionRow == 0 && iRegionCol == 2 ) {
		a = (xge_vec2_t){ fW - fPad, fPad };
		b = (xge_vec2_t){ fW - fPad - fTri, fPad };
		c = (xge_vec2_t){ fW - fPad, fPad + fTri };
	} else if ( iRegionRow == 1 && iRegionCol == 0 ) {
		a = (xge_vec2_t){ fPad + fTri, fH * 0.5f };
		b = (xge_vec2_t){ fPad, fH * 0.5f - fTri * 0.5f };
		c = (xge_vec2_t){ fPad, fH * 0.5f + fTri * 0.5f };
	} else if ( iRegionRow == 1 && iRegionCol == 2 ) {
		a = (xge_vec2_t){ fW - fPad - fTri, fH * 0.5f };
		b = (xge_vec2_t){ fW - fPad, fH * 0.5f - fTri * 0.5f };
		c = (xge_vec2_t){ fW - fPad, fH * 0.5f + fTri * 0.5f };
	} else if ( iRegionRow == 2 && iRegionCol == 0 ) {
		a = (xge_vec2_t){ fPad, fH - fPad };
		b = (xge_vec2_t){ fPad + fTri, fH - fPad };
		c = (xge_vec2_t){ fPad, fH - fPad - fTri };
	} else if ( iRegionRow == 2 && iRegionCol == 1 ) {
		a = (xge_vec2_t){ fW * 0.5f, fH - fPad - fTri };
		b = (xge_vec2_t){ fW * 0.5f - fTri * 0.5f, fH - fPad };
		c = (xge_vec2_t){ fW * 0.5f + fTri * 0.5f, fH - fPad };
	} else if ( iRegionRow == 2 && iRegionCol == 2 ) {
		a = (xge_vec2_t){ fW - fPad, fH - fPad };
		b = (xge_vec2_t){ fW - fPad - fTri, fH - fPad };
		c = (xge_vec2_t){ fW - fPad, fH - fPad - fTri };
	} else {
		return;
	}
	MapEditTilesetPassageDrawPixelTriangleStroke(pPixels, iWidth, iHeight, a, b, c, 2.0f, iRed);
}

static void MapEditTilesetPassageBuildOverlayPixels(uint32_t* pPixels, int iWidth, int iHeight, unsigned char iValue)
{
	float fSize;
	float fDotRadius;
	int iRegionCol;
	int iRegionRow;
	unsigned char iBit;

	if ( pPixels == NULL || iWidth <= 0 || iHeight <= 0 ) {
		return;
	}
	memset(pPixels, 0, (size_t)iWidth * (size_t)iHeight * sizeof(uint32_t));
	fSize = MapEditTilesetPassageMinFloat((float)iWidth, (float)iHeight);
	if ( iValue == 255 ) {
		fDotRadius = fSize * 0.12f;
		if ( fDotRadius < 2.0f ) {
			fDotRadius = 2.0f;
		} else if ( fDotRadius > 4.0f ) {
			fDotRadius = 4.0f;
		}
		MapEditTilesetPassageDrawPixelCircle(pPixels, iWidth, iHeight, (float)iWidth * 0.5f, (float)iHeight * 0.5f, fDotRadius + 1.0f, XGE_COLOR_RGBA(245, 252, 246, 210));
		MapEditTilesetPassageDrawPixelCircle(pPixels, iWidth, iHeight, (float)iWidth * 0.5f, (float)iHeight * 0.5f, fDotRadius, XGE_COLOR_RGBA(26, 166, 62, 230));
		return;
	}
	if ( iValue == 0 ) {
		MapEditTilesetPassageDrawPixelRectStroke(pPixels, iWidth, iHeight, 1.5f, 1.5f, (float)iWidth - 3.0f, (float)iHeight - 3.0f, 2.0f, XGE_COLOR_RGBA(232, 26, 36, 225));
		MapEditTilesetPassageDrawPixelLine(pPixels, iWidth, iHeight, 3.0f, (float)iHeight - 3.0f, (float)iWidth - 3.0f, 3.0f, 2.0f, XGE_COLOR_RGBA(232, 26, 36, 225));
		return;
	}
	for ( iRegionRow = 0; iRegionRow < 3; iRegionRow++ ) {
		for ( iRegionCol = 0; iRegionCol < 3; iRegionCol++ ) {
			if ( iRegionRow == 1 && iRegionCol == 1 ) {
				continue;
			}
			iBit = MapEditTilesetPassageBitForRegion(iRegionCol, iRegionRow);
			if ( (iValue & iBit) == 0 ) {
				MapEditTilesetPassageDrawPixelForbiddenTriangle(pPixels, iWidth, iHeight, iRegionCol, iRegionRow);
			}
		}
	}
}

static int MapEditTilesetPassageLoadOverlayTextureSet(mapedit_app_t* pApp, const char* sDir, int iCellWidth, int iCellHeight)
{
	char sFile[64];
	str sPath;
	xui_texture pTexture;
	xui_texture_desc_t tDesc;
	int i;
	int j;

	if ( pApp == NULL || sDir == NULL || iCellWidth <= 0 || iCellHeight <= 0 ) {
		return 0;
	}
	for ( i = 0; i < 256; i++ ) {
		snprintf(sFile, sizeof(sFile), "passage_%02X.png", i);
		sPath = xrtPathJoin(4, xCore.AppPath, (str)"res", (str)sDir, (str)sFile);
		if ( sPath == NULL ) {
			goto fail;
		}
		if ( !xrtFileExists(sPath) ) {
			xrtFree(sPath);
			goto fail;
		}
		pTexture = NULL;
		memset(&tDesc, 0, sizeof(tDesc));
		if ( xgeXuiTextureCreateFile(&pApp->tXui, (const char*)sPath, XGE_IMAGE_STRAIGHT_ALPHA, &pTexture) != XGE_OK ||
		     xgeXuiTextureGetDesc(&pApp->tXui, pTexture, &tDesc) != XGE_OK ||
		     tDesc.iWidth != iCellWidth || tDesc.iHeight != iCellHeight ) {
			if ( pTexture != NULL ) {
				xgeXuiTextureDestroy(&pApp->tXui, pTexture);
			}
			xrtFree(sPath);
			goto fail;
		}
		xrtFree(sPath);
		pApp->arrTilesetPassageOverlayTexture[i] = pTexture;
	}
	pApp->iTilesetPassageOverlayCellWidth = iCellWidth;
	pApp->iTilesetPassageOverlayCellHeight = iCellHeight;
	return 1;

fail:
	for ( j = 0; j < 256; j++ ) {
		if ( pApp->arrTilesetPassageOverlayTexture[j] != NULL ) {
			xgeXuiTextureDestroy(&pApp->tXui, pApp->arrTilesetPassageOverlayTexture[j]);
			pApp->arrTilesetPassageOverlayTexture[j] = NULL;
		}
	}
	pApp->iTilesetPassageOverlayCellWidth = 0;
	pApp->iTilesetPassageOverlayCellHeight = 0;
	return 0;
}

static int MapEditTilesetPassageLoadOverlayTextures(mapedit_app_t* pApp, int iCellWidth, int iCellHeight)
{
	char sDir[64];

	if ( pApp == NULL ) {
		return 0;
	}
	snprintf(sDir, sizeof(sDir), "passage_%dx%d", iCellWidth, iCellHeight);
	if ( MapEditTilesetPassageLoadOverlayTextureSet(pApp, sDir, iCellWidth, iCellHeight) ) {
		return 1;
	}
	if ( iCellWidth == 32 && iCellHeight == 32 &&
	     MapEditTilesetPassageLoadOverlayTextureSet(pApp, "passage_16x16_32x32", iCellWidth, iCellHeight) ) {
		return 1;
	}
	return 0;
}

static void MapEditTilesetPassageEnsureOverlayTextures(mapedit_app_t* pApp, int iCellWidth, int iCellHeight)
{
	uint32_t* pPixels;
	int i;

	if ( pApp == NULL || iCellWidth <= 0 || iCellHeight <= 0 ) {
		return;
	}
	if ( pApp->iTilesetPassageOverlayCellWidth == iCellWidth &&
	     pApp->iTilesetPassageOverlayCellHeight == iCellHeight &&
	     pApp->arrTilesetPassageOverlayTexture[0] != NULL &&
	     pApp->arrTilesetPassageOverlayTexture[255] != NULL ) {
		return;
	}
	MapEditTilesetPassageClearOverlayTextures(pApp);
	if ( MapEditTilesetPassageLoadOverlayTextures(pApp, iCellWidth, iCellHeight) ) {
		return;
	}
	pPixels = (uint32_t*)malloc((size_t)iCellWidth * (size_t)iCellHeight * sizeof(uint32_t));
	if ( pPixels == NULL ) {
		return;
	}
	for ( i = 0; i < 256; i++ ) {
		MapEditTilesetPassageBuildOverlayPixels(pPixels, iCellWidth, iCellHeight, (unsigned char)i);
		if ( xgeXuiTextureCreateRGBA(&pApp->tXui, iCellWidth, iCellHeight, pPixels, iCellWidth * 4, 0, &pApp->arrTilesetPassageOverlayTexture[i]) != XGE_OK ) {
			pApp->arrTilesetPassageOverlayTexture[i] = NULL;
		}
	}
	free(pPixels);
	pApp->iTilesetPassageOverlayCellWidth = iCellWidth;
	pApp->iTilesetPassageOverlayCellHeight = iCellHeight;
}

static void MapEditTilesetPassageDrawOverlayTexture(mapedit_app_t* pApp, xge_rect_t tCell, unsigned char iValue)
{
	xui_texture pTexture;
	xge_draw_t tDraw;

	if ( pApp == NULL ) {
		return;
	}
	pTexture = pApp->arrTilesetPassageOverlayTexture[(int)iValue];
	if ( pTexture == NULL ) {
		return;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = (xge_texture)pTexture;
	tDraw.tSrc = (xge_rect_t){ 0.0f, 0.0f, (float)pApp->iTilesetPassageOverlayCellWidth, (float)pApp->iTilesetPassageOverlayCellHeight };
	tDraw.tDst = tCell;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
	xgeFlush();
}

static int MapEditTilesetPassageTilesPerRow(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTilesPerRow <= 0 ) {
		return 20;
	}
	return pApp->iSetupTilesPerRow;
}

static int MapEditTilesetPassageRealCellWidth(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTileWidth <= 0 ) {
		return 16;
	}
	return pApp->iSetupTileWidth;
}

static int MapEditTilesetPassageRealCellHeight(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTileHeight <= 0 ) {
		return 16;
	}
	return pApp->iSetupTileHeight;
}

static int MapEditTilesetPassageDisplayCellWidth(mapedit_app_t* pApp)
{
	int iCellW;

	iCellW = MapEditTilesetPassageRealCellWidth(pApp);
	if ( iCellW < MAPEDIT_TILESET_PASSAGE_EXPERT_MIN_CELL ) {
		iCellW = MAPEDIT_TILESET_PASSAGE_EXPERT_MIN_CELL;
	}
	return iCellW;
}

static int MapEditTilesetPassageDisplayCellHeight(mapedit_app_t* pApp)
{
	int iCellH;

	iCellH = MapEditTilesetPassageRealCellHeight(pApp);
	if ( iCellH < MAPEDIT_TILESET_PASSAGE_EXPERT_MIN_CELL ) {
		iCellH = MAPEDIT_TILESET_PASSAGE_EXPERT_MIN_CELL;
	}
	return iCellH;
}

static int MapEditTilesetPassageStaticStart(mapedit_app_t* pApp)
{
	mapedit_tileset_set_item_t* pItem;
	int iPerRow;
	int iReserved;

	pItem = MapEditTilesetSetSelectedItem(pApp);
	iPerRow = MapEditTilesetPassageTilesPerRow(pApp);
	iReserved = 1 + ((pItem != NULL) ? pItem->iSpecialTileCount : 0);
	if ( iReserved < 1 ) {
		iReserved = 1;
	}
	return ((iReserved + iPerRow - 1) / iPerRow) * iPerRow;
}

static int MapEditTilesetPassageStaticRows(mapedit_app_t* pApp)
{
	int iRealH;
	int iRows;

	if ( pApp == NULL ) {
		return MAPEDIT_TILESET_PASSAGE_PLACEHOLDER_ROWS;
	}
	iRealH = MapEditTilesetPassageRealCellHeight(pApp);
	if ( pApp->tTilesetPassageStaticDesc.iHeight > 0 ) {
		iRows = (pApp->tTilesetPassageStaticDesc.iHeight + iRealH - 1) / iRealH;
		return (iRows > 0) ? iRows : 1;
	}
	return MAPEDIT_TILESET_PASSAGE_PLACEHOLDER_ROWS;
}

static int MapEditTilesetPassageStaticCols(mapedit_app_t* pApp)
{
	int iRealW;
	int iCols;
	int iPerRow;

	if ( pApp == NULL || pApp->tTilesetPassageStaticDesc.iWidth <= 0 ) {
		return 0;
	}
	iRealW = MapEditTilesetPassageRealCellWidth(pApp);
	iPerRow = MapEditTilesetPassageTilesPerRow(pApp);
	iCols = (pApp->tTilesetPassageStaticDesc.iWidth + iRealW - 1) / iRealW;
	if ( iCols > iPerRow ) {
		iCols = iPerRow;
	}
	return (iCols > 0) ? iCols : 0;
}

static float MapEditTilesetPassageGridContentWidth(mapedit_app_t* pApp)
{
	return (float)(MapEditTilesetPassageTilesPerRow(pApp) * MapEditTilesetPassageDisplayCellWidth(pApp));
}

static float MapEditTilesetPassageGridContentHeight(mapedit_app_t* pApp)
{
	int iRows;

	if ( pApp == NULL ) {
		return 1.0f;
	}
	iRows = pApp->tTilesetPassageGrid.iRowCount;
	if ( iRows <= 0 ) {
		iRows = 1;
	}
	return (float)(iRows * MapEditTilesetPassageDisplayCellHeight(pApp));
}

static void MapEditTilesetPassageSetScrollContentRect(xge_xui_widget pWidget, xge_rect_t tLocal, xge_rect_t tRect)
{
	int bSizeChanged;

	if ( pWidget == NULL ) {
		return;
	}
	if ( MapEditTilesetPassageRectAlmostEqual(pWidget->tLocalRect, tLocal) && MapEditTilesetPassageRectAlmostEqual(pWidget->tRect, tRect) ) {
		return;
	}
	bSizeChanged = pWidget->tStyle.tWidth.iUnit != XGE_XUI_SIZE_PX ||
		pWidget->tStyle.tHeight.iUnit != XGE_XUI_SIZE_PX ||
		MapEditTilesetPassageAbsFloat(pWidget->tStyle.tWidth.fValue - tLocal.fW) >= 0.01f ||
		MapEditTilesetPassageAbsFloat(pWidget->tStyle.tHeight.fValue - tLocal.fH) >= 0.01f;
	if ( bSizeChanged ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(tLocal.fW), xgeXuiSizePx(tLocal.fH));
	}
	xgeXuiWidgetSetRect(pWidget, tRect);
	pWidget->tLocalRect = tLocal;
}

static void MapEditTilesetPassageClearStaticTexture(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->pTilesetPassageStaticTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pApp->pTilesetPassageStaticTexture);
		pApp->pTilesetPassageStaticTexture = NULL;
	}
	memset(&pApp->tTilesetPassageStaticDesc, 0, sizeof(pApp->tTilesetPassageStaticDesc));
}

static void MapEditTilesetPassageLoadStaticTexture(mapedit_app_t* pApp, mapedit_tileset_set_item_t* pItem)
{
	str sPath;
	xui_texture pTexture;
	xui_texture_desc_t tDesc;

	MapEditTilesetPassageClearStaticTexture(pApp);
	if ( pApp == NULL || pItem == NULL || pItem->sStaticTiles[0] == 0 ) {
		return;
	}
	sPath = xrtPathJoin(4, xCore.AppPath, (str)"assets", (str)"tilesheets", (str)pItem->sStaticTiles);
	if ( sPath == NULL ) {
		return;
	}
	if ( !xrtFileExists(sPath) ) {
		xrtFree(sPath);
		return;
	}
	pTexture = NULL;
	memset(&tDesc, 0, sizeof(tDesc));
	if ( xgeXuiTextureCreateFile(&pApp->tXui, (const char*)sPath, XGE_IMAGE_STRAIGHT_ALPHA, &pTexture) == XGE_OK &&
	     xgeXuiTextureGetDesc(&pApp->tXui, pTexture, &tDesc) == XGE_OK &&
	     tDesc.iWidth > 0 && tDesc.iHeight > 0 ) {
		pApp->pTilesetPassageStaticTexture = pTexture;
		pApp->tTilesetPassageStaticDesc = tDesc;
	} else if ( pTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pTexture);
	}
	xrtFree(sPath);
}

static void MapEditTilesetPassageUpdateScrollContentSize(mapedit_app_t* pApp)
{
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tTilesetPassageScrollFrame.pWidget == NULL ) {
		return;
	}
	fContentW = MapEditTilesetPassageGridContentWidth(pApp);
	fContentH = MapEditTilesetPassageGridContentHeight(pApp);
	if ( MapEditTilesetPassageAbsFloat(pApp->tTilesetPassageScrollModel.fContentW - fContentW) >= 0.01f ||
	     MapEditTilesetPassageAbsFloat(pApp->tTilesetPassageScrollModel.fContentH - fContentH) >= 0.01f ) {
		xgeXuiScrollFrameSetContentSize(&pApp->tTilesetPassageScrollFrame, fContentW, fContentH);
	}
}

static void MapEditTilesetPassageSyncScrollContent(mapedit_app_t* pApp)
{
	xge_rect_t tViewport;
	xge_rect_t tLocal;
	xge_rect_t tRect;
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tTilesetPassageScrollFrame.pWidget == NULL || pApp->pTilesetPassageGridWidget == NULL ) {
		return;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tTilesetPassageScrollFrame);
	fContentW = (pApp->tTilesetPassageScrollModel.fContentW > 1.0f) ? pApp->tTilesetPassageScrollModel.fContentW : MapEditTilesetPassageGridContentWidth(pApp);
	fContentH = (pApp->tTilesetPassageScrollModel.fContentH > 1.0f) ? pApp->tTilesetPassageScrollModel.fContentH : MapEditTilesetPassageGridContentHeight(pApp);
	tLocal.fX = -pApp->tTilesetPassageScrollModel.fScrollX;
	tLocal.fY = -pApp->tTilesetPassageScrollModel.fScrollY;
	tLocal.fW = (fContentW > tViewport.fW) ? fContentW : tViewport.fW;
	tLocal.fH = (fContentH > tViewport.fH) ? fContentH : tViewport.fH;
	tRect.fX = tViewport.fX + tLocal.fX;
	tRect.fY = tViewport.fY + tLocal.fY;
	tRect.fW = tLocal.fW;
	tRect.fH = tLocal.fH;
	MapEditTilesetPassageSetScrollContentRect(pApp->pTilesetPassageGridWidget, tLocal, tRect);
}

static void MapEditTilesetPassageUpdateAndSyncScrollContent(mapedit_app_t* pApp)
{
	MapEditTilesetPassageUpdateScrollContentSize(pApp);
	MapEditTilesetPassageSyncScrollContent(pApp);
}

static void MapEditTilesetPassageScrollChanged(xge_xui_scroll_frame pFrame, float fScrollX, float fScrollY, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pFrame;
	(void)fScrollX;
	(void)fScrollY;
	pApp = (mapedit_app_t*)pUser;
	MapEditTilesetPassageSyncScrollContent(pApp);
}

static void MapEditTilesetPassageScrollLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	xgeXuiScrollFrameLayout(&pApp->tTilesetPassageScrollFrame);
	MapEditTilesetPassageSyncScrollContent(pApp);
}

static int MapEditTilesetPassageTileEditable(mapedit_app_t* pApp, mapedit_tileset_set_item_t* pItem, int iTileId)
{
	int iStaticStart;
	int iLocal;
	int iRow;
	int iCol;

	if ( pApp == NULL || pItem == NULL || iTileId <= 0 ) {
		return 0;
	}
	if ( iTileId >= 1 && iTileId <= pItem->iSpecialTileCount ) {
		return 1;
	}
	iStaticStart = MapEditTilesetPassageStaticStart(pApp);
	if ( iTileId < iStaticStart || pApp->pTilesetPassageStaticTexture == NULL ) {
		return 0;
	}
	iLocal = iTileId - iStaticStart;
	iRow = iLocal / MapEditTilesetPassageTilesPerRow(pApp);
	iCol = iLocal % MapEditTilesetPassageTilesPerRow(pApp);
	if ( iRow < 0 || iRow >= MapEditTilesetPassageStaticRows(pApp) ) {
		return 0;
	}
	return iCol < MapEditTilesetPassageStaticCols(pApp);
}

static unsigned char MapEditTilesetPassageValue(mapedit_tileset_set_item_t* pItem, int iTileId)
{
	if ( pItem == NULL || iTileId < 0 || iTileId >= MAPEDIT_TILESET_PASSAGE_MAX ) {
		return (unsigned char)MAPEDIT_TILESET_PASSAGE_DEFAULT;
	}
	if ( iTileId >= pItem->iPassageCount ) {
		return (unsigned char)MAPEDIT_TILESET_PASSAGE_DEFAULT;
	}
	return pItem->arrPassage[iTileId];
}

static unsigned char MapEditTilesetPassageBitForRegion(int iRegionCol, int iRegionRow)
{
	if ( iRegionRow == 0 && iRegionCol == 0 ) {
		return 0x01;
	}
	if ( iRegionRow == 0 && iRegionCol == 1 ) {
		return 0x02;
	}
	if ( iRegionRow == 0 && iRegionCol == 2 ) {
		return 0x04;
	}
	if ( iRegionRow == 1 && iRegionCol == 0 ) {
		return 0x08;
	}
	if ( iRegionRow == 1 && iRegionCol == 2 ) {
		return 0x10;
	}
	if ( iRegionRow == 2 && iRegionCol == 0 ) {
		return 0x20;
	}
	if ( iRegionRow == 2 && iRegionCol == 1 ) {
		return 0x40;
	}
	if ( iRegionRow == 2 && iRegionCol == 2 ) {
		return 0x80;
	}
	return 0;
}

static void MapEditTilesetPassageDrawStaticTexture(mapedit_app_t* pApp, xge_rect_t tRect)
{
	xge_draw_t tDraw;
	float fScaleX;
	float fScaleY;
	int iStaticStartRow;

	if ( pApp == NULL || pApp->pTilesetPassageStaticTexture == NULL ||
	     pApp->tTilesetPassageStaticDesc.iWidth <= 0 || pApp->tTilesetPassageStaticDesc.iHeight <= 0 ) {
		return;
	}
	fScaleX = (float)MapEditTilesetPassageDisplayCellWidth(pApp) / (float)MapEditTilesetPassageRealCellWidth(pApp);
	fScaleY = (float)MapEditTilesetPassageDisplayCellHeight(pApp) / (float)MapEditTilesetPassageRealCellHeight(pApp);
	iStaticStartRow = MapEditTilesetPassageStaticStart(pApp) / MapEditTilesetPassageTilesPerRow(pApp);
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = (xge_texture)pApp->pTilesetPassageStaticTexture;
	tDraw.tSrc = (xge_rect_t){ 0.0f, 0.0f, (float)pApp->tTilesetPassageStaticDesc.iWidth, (float)pApp->tTilesetPassageStaticDesc.iHeight };
	tDraw.tDst = (xge_rect_t){
		tRect.fX,
		tRect.fY + (float)(iStaticStartRow * MapEditTilesetPassageDisplayCellHeight(pApp)),
		(float)pApp->tTilesetPassageStaticDesc.iWidth * fScaleX,
		(float)pApp->tTilesetPassageStaticDesc.iHeight * fScaleY
	};
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
	xgeFlush();
}

static void MapEditTilesetPassageContentPaint(mapedit_tile_grid_t* pGrid, xge_rect_t tRect, void* pUser)
{
	mapedit_app_t* pApp;
	mapedit_tileset_set_item_t* pItem;
	xge_rect_t tStaticRect;
	int iStaticStartRow;
	int iCellH;
	const char* sText;

	(void)pGrid;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pItem = MapEditTilesetSetSelectedItem(pApp);
	if ( pApp->pTilesetPassageStaticTexture != NULL ) {
		MapEditTilesetPassageDrawStaticTexture(pApp, tRect);
		return;
	}
	iCellH = MapEditTilesetPassageDisplayCellHeight(pApp);
	iStaticStartRow = MapEditTilesetPassageStaticStart(pApp) / MapEditTilesetPassageTilesPerRow(pApp);
	tStaticRect = (xge_rect_t){
		tRect.fX,
		tRect.fY + (float)(iStaticStartRow * iCellH),
		(float)(MapEditTilesetPassageTilesPerRow(pApp) * MapEditTilesetPassageDisplayCellWidth(pApp)),
		(float)(MapEditTilesetPassageStaticRows(pApp) * iCellH)
	};
	xgeShapeRectFillPx(tStaticRect, XGE_COLOR_RGBA(235, 244, 250, 255));
	sText = (pItem == NULL) ? "未选择图集" : "静态图块集未设置或图片不可用";
	xgeTextDrawRect(pApp->bFontReady ? &pApp->tFont : NULL, sText, tStaticRect, XGE_COLOR_RGBA(104, 128, 148, 255),
		XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
}

static float MapEditTilesetPassageMinFloat(float a, float b)
{
	return (a < b) ? a : b;
}

static void MapEditTilesetPassageCellPaint(mapedit_tile_grid_t* pGrid, int iCol, int iRow, xge_rect_t tCell, void* pUser)
{
	mapedit_app_t* pApp;
	mapedit_tileset_set_item_t* pItem;
	int iPerRow;
	int iTileId;
	int iStaticStart;
	int iSpecialCount;
	unsigned char iValue;

	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pItem = MapEditTilesetSetSelectedItem(pApp);
	iPerRow = MapEditTilesetPassageTilesPerRow(pApp);
	iTileId = iRow * iPerRow + iCol;
	iSpecialCount = (pItem != NULL) ? pItem->iSpecialTileCount : 0;
	iStaticStart = MapEditTilesetPassageStaticStart(pApp);
	if ( iTileId == 0 ) {
		xgeShapeRectFillPx(tCell, XGE_COLOR_RGBA(242, 248, 252, 235));
		xgeShapeRectStrokePx(tCell, 1.0f, XGE_COLOR_RGBA(98, 158, 108, 220));
		xgeTextDrawRect(pGrid->pFont, "0", tCell, XGE_COLOR_RGBA(48, 112, 62, 255),
			XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
		return;
	}
	if ( iTileId >= 1 && iTileId <= iSpecialCount ) {
		xgeShapeRectFillPx(tCell, XGE_COLOR_RGBA(204, 232, 250, 168));
		xgeShapeRectStrokePx(tCell, 1.0f, XGE_COLOR_RGBA(72, 150, 208, 210));
	}
	if ( iTileId > iSpecialCount && iTileId < iStaticStart ) {
		xgeShapeRectFillPx(tCell, XGE_COLOR_RGBA(224, 232, 238, 180));
		if ( pGrid->iCellWidth >= 20 && pGrid->iCellHeight >= 16 ) {
			xgeTextDrawRect(pGrid->pFont, "-", tCell, XGE_COLOR_RGBA(120, 136, 148, 255),
				XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
		}
		return;
	}
	if ( !MapEditTilesetPassageTileEditable(pApp, pItem, iTileId) ) {
		return;
	}
	iValue = MapEditTilesetPassageValue(pItem, iTileId);
	MapEditTilesetPassageDrawOverlayTexture(pApp, tCell, iValue);
}

static void MapEditTilesetPassageCellClick(mapedit_tile_grid_t* pGrid, int iCol, int iRow, void* pUser)
{
	mapedit_app_t* pApp;
	mapedit_tileset_set_item_t* pItem;
	xge_rect_t tCell;
	char sStatus[128];
	int iPerRow;
	int iTileId;
	int iRegionCol;
	int iRegionRow;
	float fRelX;
	float fRelY;
	unsigned char iValue;
	unsigned char iBit;

	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL || pGrid == NULL ) {
		return;
	}
	pItem = MapEditTilesetSetSelectedItem(pApp);
	iPerRow = MapEditTilesetPassageTilesPerRow(pApp);
	iTileId = iRow * iPerRow + iCol;
	pApp->iTilesetPassageSelectedTile = iTileId;
	if ( !MapEditTilesetPassageTileEditable(pApp, pItem, iTileId) ) {
		snprintf(sStatus, sizeof(sStatus), "ID %d 不是可编辑通行图块", iTileId);
		MapEditAppSetStatus(pApp, sStatus);
		return;
	}
	MapEditTilesetSetEnsurePassageCount(pItem, iTileId + 1);
	iValue = pItem->arrPassage[iTileId];
	tCell = (xge_rect_t){
		pGrid->pWidget->tContentRect.fX + (float)(iCol * pGrid->iCellWidth),
		pGrid->pWidget->tContentRect.fY + (float)(iRow * pGrid->iCellHeight),
		(float)pGrid->iCellWidth,
		(float)pGrid->iCellHeight
	};
	fRelX = pApp->fTilesetPassageLastMouseX - tCell.fX;
	fRelY = pApp->fTilesetPassageLastMouseY - tCell.fY;
	iRegionCol = (int)(fRelX / (tCell.fW / 3.0f));
	iRegionRow = (int)(fRelY / (tCell.fH / 3.0f));
	if ( iRegionCol < 0 ) {
		iRegionCol = 0;
	} else if ( iRegionCol > 2 ) {
		iRegionCol = 2;
	}
	if ( iRegionRow < 0 ) {
		iRegionRow = 0;
	} else if ( iRegionRow > 2 ) {
		iRegionRow = 2;
	}
	iBit = MapEditTilesetPassageBitForRegion(iRegionCol, iRegionRow);
	if ( iBit == 0 ) {
		pItem->arrPassage[iTileId] = (iValue == 0) ? 255 : 0;
	} else {
		pItem->arrPassage[iTileId] = (unsigned char)(iValue ^ iBit);
	}
	if ( MapEditTilesetSetSaveSelected(pApp) == XGE_OK ) {
		snprintf(sStatus, sizeof(sStatus), "通行数据已保存: ID %d = %u", iTileId, (unsigned int)pItem->arrPassage[iTileId]);
	} else {
		snprintf(sStatus, sizeof(sStatus), "通行数据已修改，但保存失败: ID %d", iTileId);
	}
	MapEditAppSetStatus(pApp, sStatus);
	if ( pApp->pTilesetPassageGridWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pTilesetPassageGridWidget);
	}
}

void MapEditTilesetPassageRefresh(mapedit_app_t* pApp)
{
	mapedit_tileset_set_item_t* pItem;
	int iPerRow;
	int iCellW;
	int iCellH;
	int iStaticStart;
	int iStaticRows;
	int iRows;

	if ( pApp == NULL || pApp->pTilesetPassageGridWidget == NULL ) {
		return;
	}
	pItem = MapEditTilesetSetSelectedItem(pApp);
	iPerRow = MapEditTilesetPassageTilesPerRow(pApp);
	iCellW = MapEditTilesetPassageDisplayCellWidth(pApp);
	iCellH = MapEditTilesetPassageDisplayCellHeight(pApp);
	MapEditTilesetPassageLoadStaticTexture(pApp, pItem);
	iStaticStart = MapEditTilesetPassageStaticStart(pApp);
	iStaticRows = MapEditTilesetPassageStaticRows(pApp);
	iRows = (iStaticStart / iPerRow) + iStaticRows;
	if ( iRows < MAPEDIT_TILESET_PASSAGE_PLACEHOLDER_ROWS ) {
		iRows = MAPEDIT_TILESET_PASSAGE_PLACEHOLDER_ROWS;
	}
	MapEditTileGridSetCellSize(&pApp->tTilesetPassageGrid, iCellW, iCellH);
	MapEditTileGridSetGridSize(&pApp->tTilesetPassageGrid, iPerRow, iRows);
	MapEditTileGridSetMaxCells(&pApp->tTilesetPassageGrid, iPerRow * iRows);
	MapEditTilesetPassageEnsureOverlayTextures(pApp, iCellW, iCellH);
	pApp->tTilesetPassageGrid.bHasSelection = 0;
	pApp->tTilesetPassageGrid.iHoverCol = -1;
	pApp->tTilesetPassageGrid.iHoverRow = -1;
	pApp->iTilesetPassageSelectedTile = -1;
	if ( pApp->tTilesetPassageScrollFrame.pWidget != NULL ) {
		xgeXuiScrollFrameSetOffset(&pApp->tTilesetPassageScrollFrame, 0.0f, 0.0f);
		MapEditTilesetPassageUpdateAndSyncScrollContent(pApp);
	}
	xgeXuiWidgetMarkLayout(pApp->pTilesetPassageCanvasWidget);
	xgeXuiWidgetMarkPaint(pApp->pTilesetPassageGridWidget);
}

int MapEditTilesetPassageHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent)
{
	xge_rect_t tViewport;

	if ( pApp == NULL || pEvent == NULL || pApp->pTilesetPassageGridWidget == NULL || !MapEditTilesetPassageMouseGridEvent(pEvent) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( !MapEditTilesetPassageWidgetInteractive(pApp->pTilesetPassageGridWidget) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tTilesetPassageScrollFrame);
	if ( pApp->tTilesetPassageGrid.bDragging ||
	     (MapEditTilesetPassageRectContains(tViewport, pEvent->fX, pEvent->fY) &&
	      MapEditTilesetPassageRectContains(pApp->pTilesetPassageGridWidget->tRect, pEvent->fX, pEvent->fY)) ) {
		pApp->fTilesetPassageLastMouseX = pEvent->fX;
		pApp->fTilesetPassageLastMouseY = pEvent->fY;
		return MapEditTileGridEventProc(pApp->pTilesetPassageGridWidget, pEvent, &pApp->tTilesetPassageGrid);
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int MapEditTilesetPassageCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow)
{
	xge_xui_widget pContent;
	xge_xui_widget pCanvasWidget;
	xge_xui_widget pGridWidget;

	if ( pApp == NULL || pWindow == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pContent = MapEditWorkspaceNewContent(pApp);
	pCanvasWidget = xgeXuiWidgetCreate();
	pGridWidget = xgeXuiWidgetCreate();
	if ( pContent == NULL || pCanvasWidget == NULL || pGridWidget == NULL ) {
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetSize(pCanvasWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetBackground(pCanvasWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetBorder(pCanvasWidget, 1.0f, XGE_COLOR_RGBA(164, 206, 236, 255));
	xgeXuiWidgetSetSize(pGridWidget, xgeXuiSizePx(1.0f), xgeXuiSizePx(1.0f));
	xgeXuiScrollModelInit(&pApp->tTilesetPassageScrollModel);
	if ( xgeXuiScrollFrameInit(&pApp->tTilesetPassageScrollFrame, &pApp->tXui, pCanvasWidget, &pApp->tTilesetPassageScrollModel) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetEvent(pCanvasWidget, xgeXuiScrollFrameEventProc, &pApp->tTilesetPassageScrollFrame);
	xgeXuiWidgetSetLayoutProc(pCanvasWidget, MapEditTilesetPassageScrollLayoutProc, pApp);
	xgeXuiWidgetSetLayout(xgeXuiScrollFrameGetViewportWidget(&pApp->tTilesetPassageScrollFrame), XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiScrollFrameSetChange(&pApp->tTilesetPassageScrollFrame, MapEditTilesetPassageScrollChanged, pApp);
	xgeXuiScrollFrameSetScrollbarMode(&pApp->tTilesetPassageScrollFrame, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiScrollFrameSetWheelAxis(&pApp->tTilesetPassageScrollFrame, XGE_XUI_WHEEL_AXIS_BOTH);
	xgeXuiScrollFrameSetContentDragEnabled(&pApp->tTilesetPassageScrollFrame, 0);
	xgeXuiScrollFrameSetMetrics(&pApp->tTilesetPassageScrollFrame, MAPEDIT_TILESET_PASSAGE_SCROLLBAR_SIZE, 28.0f, 0.0f, 0.0f);
	xgeXuiScrollFrameSetColors(&pApp->tTilesetPassageScrollFrame, XGE_COLOR_RGBA(226, 240, 249, 255), XGE_COLOR_RGBA(90, 164, 214, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(68, 142, 204, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(180, 204, 218, 255));
	if ( xgeXuiWidgetAdd(xgeXuiScrollFrameGetViewportWidget(&pApp->tTilesetPassageScrollFrame), pGridWidget) != XGE_OK ||
	     MapEditTileGridInit(&pApp->tTilesetPassageGrid, pGridWidget, pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	MapEditTileGridSetExpand(&pApp->tTilesetPassageGrid, 0, 0);
	MapEditTileGridSetColors(&pApp->tTilesetPassageGrid,
		XGE_COLOR_RGBA(236, 246, 252, 255),
		XGE_COLOR_RGBA(124, 181, 219, 255),
		XGE_COLOR_RGBA(188, 224, 244, 150),
		XGE_COLOR_RGBA(102, 181, 230, 8),
		XGE_COLOR_RGBA(42, 140, 210, 105),
		XGE_COLOR_RGBA(104, 128, 148, 255));
	MapEditTileGridSetInteractionPaint(&pApp->tTilesetPassageGrid, 0, 0);
	MapEditTileGridSetEmptyText(&pApp->tTilesetPassageGrid, "");
	MapEditTileGridSetContentPaint(&pApp->tTilesetPassageGrid, MapEditTilesetPassageContentPaint, pApp);
	MapEditTileGridSetCellPaint(&pApp->tTilesetPassageGrid, MapEditTilesetPassageCellPaint, pApp);
	MapEditTileGridSetCellClick(&pApp->tTilesetPassageGrid, MapEditTilesetPassageCellClick, pApp);
	pApp->pTilesetPassageCanvasWidget = pCanvasWidget;
	pApp->pTilesetPassageGridWidget = pGridWidget;
	if ( xgeXuiWidgetAdd(pContent, pCanvasWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiDockWindowSetClientWidget(pWindow, pContent);
	MapEditTilesetPassageRefresh(pApp);
	return XGE_OK;
}

void MapEditTilesetPassageUnit(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	MapEditTilesetPassageClearStaticTexture(pApp);
	MapEditTilesetPassageClearOverlayTextures(pApp);
	if ( pApp->pTilesetPassageGridWidget != NULL ) {
		MapEditTileGridUnit(&pApp->tTilesetPassageGrid);
		pApp->pTilesetPassageGridWidget = NULL;
	}
	if ( pApp->pTilesetPassageCanvasWidget != NULL ) {
		xgeXuiScrollFrameUnit(&pApp->tTilesetPassageScrollFrame);
		pApp->pTilesetPassageCanvasWidget = NULL;
	}
}
