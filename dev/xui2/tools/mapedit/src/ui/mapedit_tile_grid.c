#include "mapedit_tile_grid.h"
#include <string.h>

static int MapEditTileGridClampCellSize(int iValue)
{
	return (iValue > 0) ? iValue : 16;
}

static int MapEditTileGridVisibleCols(mapedit_tile_grid_t* pGrid, xge_rect_t tRect)
{
	int iCols;

	if ( pGrid == NULL || pGrid->iCellWidth <= 0 ) {
		return 0;
	}
	iCols = (int)(tRect.fW / (float)pGrid->iCellWidth);
	if ( iCols < 1 ) {
		iCols = 1;
	}
	if ( !pGrid->bExpandWidth && pGrid->iColumnCount > 0 && iCols > pGrid->iColumnCount ) {
		iCols = pGrid->iColumnCount;
	}
	return iCols;
}

static int MapEditTileGridVisibleRows(mapedit_tile_grid_t* pGrid, xge_rect_t tRect)
{
	int iRows;

	if ( pGrid == NULL || pGrid->iCellHeight <= 0 ) {
		return 0;
	}
	iRows = (int)(tRect.fH / (float)pGrid->iCellHeight);
	if ( iRows < 1 ) {
		iRows = 1;
	}
	if ( !pGrid->bExpandHeight && pGrid->iRowCount > 0 && iRows > pGrid->iRowCount ) {
		iRows = pGrid->iRowCount;
	}
	return iRows;
}

static int MapEditTileGridCellAllowed(mapedit_tile_grid_t* pGrid, int iCol, int iRow)
{
	int iIndex;

	if ( pGrid == NULL || iCol < 0 || iRow < 0 ) {
		return 0;
	}
	if ( !pGrid->bExpandWidth && pGrid->iColumnCount > 0 && iCol >= pGrid->iColumnCount ) {
		return 0;
	}
	if ( !pGrid->bExpandHeight && pGrid->iRowCount > 0 && iRow >= pGrid->iRowCount ) {
		return 0;
	}
	if ( pGrid->iColumnCount > 0 ) {
		iIndex = iRow * pGrid->iColumnCount + iCol;
	} else {
		iIndex = iRow + iCol;
	}
	if ( pGrid->iMaxCells > 0 && iIndex >= pGrid->iMaxCells ) {
		return 0;
	}
	return 1;
}

static int MapEditTileGridHitTest(mapedit_tile_grid_t* pGrid, float fX, float fY, int* pCol, int* pRow)
{
	xge_rect_t tRect;
	float fGridW;
	float fGridH;
	int iCol;
	int iRow;

	if ( pGrid == NULL || pGrid->pWidget == NULL ) {
		return 0;
	}
	tRect = pGrid->pWidget->tRect;
	if ( !pGrid->bExpandWidth && pGrid->iColumnCount > 0 ) {
		fGridW = (float)(pGrid->iColumnCount * pGrid->iCellWidth);
		if ( tRect.fW > fGridW ) {
			tRect.fW = fGridW;
		}
	}
	if ( !pGrid->bExpandHeight && pGrid->iRowCount > 0 ) {
		fGridH = (float)(pGrid->iRowCount * pGrid->iCellHeight);
		if ( tRect.fH > fGridH ) {
			tRect.fH = fGridH;
		}
	}
	if ( fX < tRect.fX || fY < tRect.fY || fX >= tRect.fX + tRect.fW || fY >= tRect.fY + tRect.fH ) {
		return 0;
	}
	iCol = (int)((fX - tRect.fX) / (float)pGrid->iCellWidth);
	iRow = (int)((fY - tRect.fY) / (float)pGrid->iCellHeight);
	if ( !MapEditTileGridCellAllowed(pGrid, iCol, iRow) ) {
		return 0;
	}
	if ( pCol != NULL ) {
		*pCol = iCol;
	}
	if ( pRow != NULL ) {
		*pRow = iRow;
	}
	return 1;
}

static void MapEditTileGridSetSelectionRect(mapedit_tile_grid_t* pGrid, int iColA, int iRowA, int iColB, int iRowB)
{
	int iMinCol;
	int iMaxCol;
	int iMinRow;
	int iMaxRow;

	if ( pGrid == NULL ) {
		return;
	}
	iMinCol = (iColA < iColB) ? iColA : iColB;
	iMaxCol = (iColA > iColB) ? iColA : iColB;
	iMinRow = (iRowA < iRowB) ? iRowA : iRowB;
	iMaxRow = (iRowA > iRowB) ? iRowA : iRowB;
	pGrid->iSelectCol = iMinCol;
	pGrid->iSelectRow = iMinRow;
	pGrid->iSelectCols = iMaxCol - iMinCol + 1;
	pGrid->iSelectRows = iMaxRow - iMinRow + 1;
	pGrid->bHasSelection = 1;
	if ( pGrid->procSelectionChange != NULL ) {
		pGrid->procSelectionChange(pGrid, pGrid->iSelectCol, pGrid->iSelectRow, pGrid->iSelectCols, pGrid->iSelectRows, pGrid->pSelectionChangeUser);
	}
}

int MapEditTileGridInit(mapedit_tile_grid_t* pGrid, xge_xui_widget pWidget, xge_font pFont)
{
	if ( pGrid == NULL || pWidget == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pGrid, 0, sizeof(*pGrid));
	pGrid->pWidget = pWidget;
	pGrid->pFont = pFont;
	pGrid->iCellWidth = 16;
	pGrid->iCellHeight = 16;
	pGrid->iColumnCount = 20;
	pGrid->iRowCount = 16;
	pGrid->iMaxCells = 0;
	pGrid->iHoverCol = -1;
	pGrid->iHoverRow = -1;
	pGrid->iSelectCol = -1;
	pGrid->iSelectRow = -1;
	pGrid->iSelectCols = 1;
	pGrid->iSelectRows = 1;
	pGrid->iDragAnchorCol = -1;
	pGrid->iDragAnchorRow = -1;
	pGrid->bDrawHover = 1;
	pGrid->bDrawSelection = 1;
	pGrid->sEmptyText = "";
	MapEditTileGridSetColors(pGrid,
		XGE_COLOR_RGBA(236, 246, 252, 255),
		XGE_COLOR_RGBA(124, 181, 219, 255),
		XGE_COLOR_RGBA(188, 224, 244, 130),
		XGE_COLOR_RGBA(102, 181, 230, 8),
		XGE_COLOR_RGBA(42, 140, 210, 100),
		XGE_COLOR_RGBA(104, 128, 148, 255));
	xgeXuiWidgetSetClip(pWidget, 1);
	xgeXuiWidgetSetPaint(pWidget, MapEditTileGridPaintProc, pGrid);
	xgeXuiWidgetSetEvent(pWidget, MapEditTileGridEventProc, pGrid);
	xgeXuiWidgetSetEventInterest(pWidget, XGE_XUI_EVENT_MASK_MOUSE_MOVE | XGE_XUI_EVENT_MASK_MOUSE_LEAVE | XGE_XUI_EVENT_MASK_MOUSE_DOWN | XGE_XUI_EVENT_MASK_MOUSE_UP, 1);
	return XGE_OK;
}

void MapEditTileGridUnit(mapedit_tile_grid_t* pGrid)
{
	if ( pGrid == NULL ) {
		return;
	}
	memset(pGrid, 0, sizeof(*pGrid));
}

void MapEditTileGridSetCellSize(mapedit_tile_grid_t* pGrid, int iCellWidth, int iCellHeight)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->iCellWidth = MapEditTileGridClampCellSize(iCellWidth);
	pGrid->iCellHeight = MapEditTileGridClampCellSize(iCellHeight);
	if ( pGrid->pWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pGrid->pWidget);
	}
}

void MapEditTileGridSetGridSize(mapedit_tile_grid_t* pGrid, int iColumnCount, int iRowCount)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->iColumnCount = (iColumnCount > 0) ? iColumnCount : 1;
	pGrid->iRowCount = (iRowCount > 0) ? iRowCount : 1;
	if ( pGrid->pWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pGrid->pWidget);
	}
}

void MapEditTileGridSetMaxCells(mapedit_tile_grid_t* pGrid, int iMaxCells)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->iMaxCells = (iMaxCells > 0) ? iMaxCells : 0;
	if ( pGrid->pWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pGrid->pWidget);
	}
}

void MapEditTileGridSetExpand(mapedit_tile_grid_t* pGrid, int bExpandWidth, int bExpandHeight)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->bExpandWidth = bExpandWidth ? 1 : 0;
	pGrid->bExpandHeight = bExpandHeight ? 1 : 0;
	if ( pGrid->pWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pGrid->pWidget);
	}
}

void MapEditTileGridSetEmptyText(mapedit_tile_grid_t* pGrid, const char* sText)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->sEmptyText = (sText != NULL) ? sText : "";
	if ( pGrid->pWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pGrid->pWidget);
	}
}

void MapEditTileGridSetTexture(mapedit_tile_grid_t* pGrid, xui_texture pTexture, int iWidth, int iHeight)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->pTexture = pTexture;
	pGrid->iTextureWidth = (iWidth > 0) ? iWidth : 0;
	pGrid->iTextureHeight = (iHeight > 0) ? iHeight : 0;
	if ( pGrid->pWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pGrid->pWidget);
	}
}

void MapEditTileGridSetContentPaint(mapedit_tile_grid_t* pGrid, void (*procContentPaint)(mapedit_tile_grid_t* pGrid, xge_rect_t tRect, void* pUser), void* pUser)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->procContentPaint = procContentPaint;
	pGrid->pContentPaintUser = pUser;
	if ( pGrid->pWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pGrid->pWidget);
	}
}

void MapEditTileGridSetCellPaint(mapedit_tile_grid_t* pGrid, void (*procCellPaint)(mapedit_tile_grid_t* pGrid, int iCol, int iRow, xge_rect_t tCell, void* pUser), void* pUser)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->procCellPaint = procCellPaint;
	pGrid->pCellPaintUser = pUser;
	if ( pGrid->pWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pGrid->pWidget);
	}
}

void MapEditTileGridSetCellClick(mapedit_tile_grid_t* pGrid, void (*procCellClick)(mapedit_tile_grid_t* pGrid, int iCol, int iRow, void* pUser), void* pUser)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->procCellClick = procCellClick;
	pGrid->pCellClickUser = pUser;
}

void MapEditTileGridSetSelectionChange(mapedit_tile_grid_t* pGrid, void (*procSelectionChange)(mapedit_tile_grid_t* pGrid, int iCol, int iRow, int iCols, int iRows, void* pUser), void* pUser)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->procSelectionChange = procSelectionChange;
	pGrid->pSelectionChangeUser = pUser;
}

void MapEditTileGridSetColors(mapedit_tile_grid_t* pGrid, uint32_t iBackground, uint32_t iBorder, uint32_t iGrid, uint32_t iHover, uint32_t iSelect, uint32_t iText)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->iBackgroundColor = iBackground;
	pGrid->iBorderColor = iBorder;
	pGrid->iGridColor = iGrid;
	pGrid->iHoverColor = iHover;
	pGrid->iSelectColor = iSelect;
	pGrid->iTextColor = iText;
}

void MapEditTileGridSetInteractionPaint(mapedit_tile_grid_t* pGrid, int bDrawHover, int bDrawSelection)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->bDrawHover = bDrawHover ? 1 : 0;
	pGrid->bDrawSelection = bDrawSelection ? 1 : 0;
	if ( pGrid->pWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pGrid->pWidget);
	}
}

int MapEditTileGridEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	mapedit_tile_grid_t* pGrid;
	int iCol;
	int iRow;

	(void)pWidget;
	pGrid = (mapedit_tile_grid_t*)pUser;
	if ( pGrid == NULL || pEvent == NULL ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_XUI_MOUSE_LEAVE ) {
		if ( pGrid->bDragging == 0 ) {
			if ( pGrid->iHoverCol != -1 || pGrid->iHoverRow != -1 ) {
				pGrid->iHoverCol = -1;
				pGrid->iHoverRow = -1;
				xgeXuiWidgetMarkPaint(pGrid->pWidget);
			}
		}
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_MOUSE_MOVE ) {
		if ( MapEditTileGridHitTest(pGrid, pEvent->fX, pEvent->fY, &iCol, &iRow) ) {
			if ( pGrid->iHoverCol != iCol || pGrid->iHoverRow != iRow ) {
				pGrid->iHoverCol = iCol;
				pGrid->iHoverRow = iRow;
				xgeXuiWidgetMarkPaint(pGrid->pWidget);
			}
			if ( pGrid->bDragging != 0 ) {
				MapEditTileGridSetSelectionRect(pGrid, pGrid->iDragAnchorCol, pGrid->iDragAnchorRow, iCol, iRow);
				xgeXuiWidgetMarkPaint(pGrid->pWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
		} else if ( pGrid->iHoverCol != -1 || pGrid->iHoverRow != -1 ) {
			pGrid->iHoverCol = -1;
			pGrid->iHoverRow = -1;
			xgeXuiWidgetMarkPaint(pGrid->pWidget);
		}
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_MOUSE_DOWN && pEvent->iParam1 == XGE_MOUSE_LEFT ) {
		if ( MapEditTileGridHitTest(pGrid, pEvent->fX, pEvent->fY, &iCol, &iRow) ) {
			pGrid->bDragging = 1;
			pGrid->iDragAnchorCol = iCol;
			pGrid->iDragAnchorRow = iRow;
			MapEditTileGridSetSelectionRect(pGrid, iCol, iRow, iCol, iRow);
			xgeXuiWidgetMarkPaint(pGrid->pWidget);
			if ( pGrid->procCellClick != NULL ) {
				pGrid->procCellClick(pGrid, iCol, iRow, pGrid->pCellClickUser);
			}
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	if ( pEvent->iType == XGE_EVENT_MOUSE_UP && pEvent->iParam1 == XGE_MOUSE_LEFT ) {
		if ( pGrid->bDragging != 0 ) {
			pGrid->bDragging = 0;
			pGrid->iDragAnchorCol = -1;
			pGrid->iDragAnchorRow = -1;
			xgeXuiWidgetMarkPaint(pGrid->pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	return XGE_XUI_EVENT_CONTINUE;
}

void MapEditTileGridPaintProc(xge_xui_widget pWidget, void* pUser)
{
	mapedit_tile_grid_t* pGrid;
	xge_rect_t tRect;
	xge_rect_t tCell;
	float fX;
	float fY;
	int iCols;
	int iRows;
	int i;

	pGrid = (mapedit_tile_grid_t*)pUser;
	if ( pGrid == NULL || pWidget == NULL ) {
		return;
	}
	tRect = pWidget->tContentRect;
	if ( !pGrid->bExpandWidth && pGrid->iColumnCount > 0 ) {
		float fGridW = (float)(pGrid->iColumnCount * pGrid->iCellWidth);
		if ( tRect.fW > fGridW ) {
			tRect.fW = fGridW;
		}
	}
	if ( !pGrid->bExpandHeight && pGrid->iRowCount > 0 ) {
		float fGridH = (float)(pGrid->iRowCount * pGrid->iCellHeight);
		if ( tRect.fH > fGridH ) {
			tRect.fH = fGridH;
		}
	}
	xgeShapeRectFillPx(tRect, pGrid->iBackgroundColor);
	if ( pGrid->pTexture != NULL && pGrid->iTextureWidth > 0 && pGrid->iTextureHeight > 0 ) {
		xge_draw_t tDraw;
		memset(&tDraw, 0, sizeof(tDraw));
		tDraw.pTexture = (xge_texture)pGrid->pTexture;
		tDraw.tSrc = (xge_rect_t){ 0.0f, 0.0f, (float)pGrid->iTextureWidth, (float)pGrid->iTextureHeight };
		tDraw.tDst = (xge_rect_t){ tRect.fX, tRect.fY, (float)pGrid->iTextureWidth, (float)pGrid->iTextureHeight };
		tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
		xgeDrawEx(&tDraw);
		xgeFlush();
	}
	iCols = MapEditTileGridVisibleCols(pGrid, tRect);
	iRows = MapEditTileGridVisibleRows(pGrid, tRect);
	if ( pGrid->procContentPaint != NULL ) {
		pGrid->procContentPaint(pGrid, tRect, pGrid->pContentPaintUser);
	}
	if ( pGrid->procCellPaint != NULL ) {
		int iCol;
		int iRow;

		for ( iRow = 0; iRow < iRows; iRow++ ) {
			for ( iCol = 0; iCol < iCols; iCol++ ) {
				if ( !MapEditTileGridCellAllowed(pGrid, iCol, iRow) ) {
					continue;
				}
				tCell = (xge_rect_t){ tRect.fX + (float)(iCol * pGrid->iCellWidth), tRect.fY + (float)(iRow * pGrid->iCellHeight), (float)pGrid->iCellWidth, (float)pGrid->iCellHeight };
				pGrid->procCellPaint(pGrid, iCol, iRow, tCell, pGrid->pCellPaintUser);
			}
		}
	}
	for ( i = 0; i <= iCols; i++ ) {
		fX = tRect.fX + (float)(i * pGrid->iCellWidth);
		xgeShapeRectFillPx((xge_rect_t){ fX, tRect.fY, 1.0f, (float)(iRows * pGrid->iCellHeight) }, pGrid->iGridColor);
	}
	for ( i = 0; i <= iRows; i++ ) {
		fY = tRect.fY + (float)(i * pGrid->iCellHeight);
		xgeShapeRectFillPx((xge_rect_t){ tRect.fX, fY, (float)(iCols * pGrid->iCellWidth), 1.0f }, pGrid->iGridColor);
	}
	if ( pGrid->bDrawHover && pGrid->iHoverCol >= 0 && pGrid->iHoverRow >= 0 ) {
		tCell = (xge_rect_t){ tRect.fX + (float)(pGrid->iHoverCol * pGrid->iCellWidth), tRect.fY + (float)(pGrid->iHoverRow * pGrid->iCellHeight), (float)pGrid->iCellWidth, (float)pGrid->iCellHeight };
		tCell.fW += 1.0f;
		tCell.fH += 1.0f;
		xgeShapeRectStrokePx(tCell, 1.0f, XGE_COLOR_RGBA(42, 140, 210, 180));
	}
	if ( pGrid->bDrawSelection && pGrid->bHasSelection ) {
		tCell = (xge_rect_t){ tRect.fX + (float)(pGrid->iSelectCol * pGrid->iCellWidth), tRect.fY + (float)(pGrid->iSelectRow * pGrid->iCellHeight), (float)(pGrid->iSelectCols * pGrid->iCellWidth), (float)(pGrid->iSelectRows * pGrid->iCellHeight) };
		tCell.fW += 1.0f;
		tCell.fH += 1.0f;
		xgeShapeRectStrokePx(tCell, 1.0f, XGE_COLOR_RGBA(28, 122, 196, 230));
	}
	xgeShapeRectStrokePx(tRect, 1.0f, pGrid->iBorderColor);
	if ( pGrid->pTexture == NULL && pGrid->sEmptyText != NULL && pGrid->sEmptyText[0] != 0 ) {
		xgeTextDrawRect(pGrid->pFont, pGrid->sEmptyText, tRect, pGrid->iTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
	}
}
