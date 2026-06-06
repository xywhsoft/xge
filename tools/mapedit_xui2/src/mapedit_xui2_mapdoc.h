#ifndef MAPEDIT_XUI2_MAPDOC_H
#define MAPEDIT_XUI2_MAPDOC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mapedit_xui2_mapdoc_t {
	char sPath[512];
	char sName[128];
	char sTileset[128];
	int iWidth;
	int iHeight;
	int iLayerCount;
	int iState;
	int* pTiles;
	int iTileCount;
	int iNonZeroTileCount;
} mapedit_xui2_mapdoc_t;

void mapeditXui2MapDocInit(mapedit_xui2_mapdoc_t* pDoc);
void mapeditXui2MapDocUnit(mapedit_xui2_mapdoc_t* pDoc);
int mapeditXui2MapDocLoad(mapedit_xui2_mapdoc_t* pDoc, const char* sPath);
int mapeditXui2MapDocSaveSmoke(const mapedit_xui2_mapdoc_t* pDoc, const char* sPath);
int mapeditXui2MapDocTileIndex(const mapedit_xui2_mapdoc_t* pDoc, int iLayer, int x, int y);
int mapeditXui2MapDocSetTile(mapedit_xui2_mapdoc_t* pDoc, int iLayer, int x, int y, int iTile);

#ifdef __cplusplus
}
#endif

#endif
