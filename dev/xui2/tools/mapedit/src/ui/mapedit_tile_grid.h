#ifndef MAPEDIT_TILE_GRID_H
#define MAPEDIT_TILE_GRID_H

#include "xge.h"

typedef struct mapedit_tile_grid_t {
	xge_xui_widget pWidget;
	xge_font pFont;
	xui_texture pTexture;
	int iTextureWidth;
	int iTextureHeight;
	int iCellWidth;
	int iCellHeight;
	int iColumnCount;
	int iRowCount;
	int iMaxCells;
	int bExpandWidth;
	int bExpandHeight;
	int iHoverCol;
	int iHoverRow;
	int iSelectCol;
	int iSelectRow;
	int iSelectCols;
	int iSelectRows;
	int bHasSelection;
	int bDragging;
	int iDragAnchorCol;
	int iDragAnchorRow;
	const char* sEmptyText;
	uint32_t iBackgroundColor;
	uint32_t iBorderColor;
	uint32_t iGridColor;
	uint32_t iHoverColor;
	uint32_t iSelectColor;
	uint32_t iTextColor;
	int bDrawHover;
	int bDrawSelection;
	void (*procContentPaint)(struct mapedit_tile_grid_t* pGrid, xge_rect_t tRect, void* pUser);
	void* pContentPaintUser;
	void (*procCellPaint)(struct mapedit_tile_grid_t* pGrid, int iCol, int iRow, xge_rect_t tCell, void* pUser);
	void* pCellPaintUser;
	void (*procCellClick)(struct mapedit_tile_grid_t* pGrid, int iCol, int iRow, void* pUser);
	void* pCellClickUser;
	void (*procSelectionChange)(struct mapedit_tile_grid_t* pGrid, int iCol, int iRow, int iCols, int iRows, void* pUser);
	void* pSelectionChangeUser;
} mapedit_tile_grid_t;

int MapEditTileGridInit(mapedit_tile_grid_t* pGrid, xge_xui_widget pWidget, xge_font pFont);
void MapEditTileGridUnit(mapedit_tile_grid_t* pGrid);
void MapEditTileGridSetCellSize(mapedit_tile_grid_t* pGrid, int iCellWidth, int iCellHeight);
void MapEditTileGridSetGridSize(mapedit_tile_grid_t* pGrid, int iColumnCount, int iRowCount);
void MapEditTileGridSetMaxCells(mapedit_tile_grid_t* pGrid, int iMaxCells);
void MapEditTileGridSetExpand(mapedit_tile_grid_t* pGrid, int bExpandWidth, int bExpandHeight);
void MapEditTileGridSetEmptyText(mapedit_tile_grid_t* pGrid, const char* sText);
void MapEditTileGridSetTexture(mapedit_tile_grid_t* pGrid, xui_texture pTexture, int iWidth, int iHeight);
void MapEditTileGridSetContentPaint(mapedit_tile_grid_t* pGrid, void (*procContentPaint)(mapedit_tile_grid_t* pGrid, xge_rect_t tRect, void* pUser), void* pUser);
void MapEditTileGridSetCellPaint(mapedit_tile_grid_t* pGrid, void (*procCellPaint)(mapedit_tile_grid_t* pGrid, int iCol, int iRow, xge_rect_t tCell, void* pUser), void* pUser);
void MapEditTileGridSetCellClick(mapedit_tile_grid_t* pGrid, void (*procCellClick)(mapedit_tile_grid_t* pGrid, int iCol, int iRow, void* pUser), void* pUser);
void MapEditTileGridSetSelectionChange(mapedit_tile_grid_t* pGrid, void (*procSelectionChange)(mapedit_tile_grid_t* pGrid, int iCol, int iRow, int iCols, int iRows, void* pUser), void* pUser);
void MapEditTileGridSetColors(mapedit_tile_grid_t* pGrid, uint32_t iBackground, uint32_t iBorder, uint32_t iGrid, uint32_t iHover, uint32_t iSelect, uint32_t iText);
void MapEditTileGridSetInteractionPaint(mapedit_tile_grid_t* pGrid, int bDrawHover, int bDrawSelection);
int MapEditTileGridEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
void MapEditTileGridPaintProc(xge_xui_widget pWidget, void* pUser);

#endif
