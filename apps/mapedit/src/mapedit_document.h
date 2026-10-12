#ifndef MAPEDIT_DOCUMENT_H
#define MAPEDIT_DOCUMENT_H
#include "xui.h"
#include "map_sdk/xge_map.h"
#include "mapedit_history.h"
#define MAPEDIT_PATH_MAX 512
#define MAPEDIT_NAME_MAX 128
#define MAPEDIT_FILE_MAX 256
#define MAPEDIT_SPECIAL_MAX 512

typedef struct mapedit_special_tile_t {
	char sType[64];
	char sFile[MAPEDIT_FILE_MAX];
	xui_surface pSurface;
	xui_surface_desc_t tDesc;
} mapedit_special_tile_t;

typedef struct mapedit_tileset_t {
	char sPath[MAPEDIT_PATH_MAX];
	char sFile[MAPEDIT_FILE_MAX];
	char sName[MAPEDIT_NAME_MAX];
	char sStaticFile[MAPEDIT_FILE_MAX];
	char sCustomData[MAPEDIT_NAME_MAX];
	int iSpecialCount;
	int iPassageCount;
	int iActorOverlayCount;
	mapedit_special_tile_t arrSpecial[MAPEDIT_SPECIAL_MAX];
	unsigned char arrPassage[65536];
	unsigned char arrActorOverlay[65536];
	xui_surface pStaticSurface;
	xui_surface_desc_t tStaticDesc;
	xvalue* pTileCustomRaw;
	int iSchemaVersion;
	int bDirty;
	int bLoaded;
} mapedit_tileset_t;


#define MAPEDIT_SNAPSHOT_META 1
#define MAPEDIT_SNAPSHOT_PASSAGE 2
#define MAPEDIT_SNAPSHOT_CUSTOM 4
#define MAPEDIT_SNAPSHOT_SIZE 8
#define MAPEDIT_SNAPSHOT_TILESET 16

int mapeditDocumentCaptureMap(mapedit_history_t* history, const xge_map_t* map, int flags);
int mapeditDocumentCaptureTileset(mapedit_history_t* history, const mapedit_tileset_t* tileset);
void mapeditDocumentSwap(mapedit_history_cmd_t* command, xge_map_t* map, mapedit_tileset_t* tileset);
int mapeditDocumentUnchanged(const mapedit_history_cmd_t* command, const xge_map_t* map, const mapedit_tileset_t* tileset);
int mapeditDocumentResize(xge_map_t* map, int width, int height);
#endif
