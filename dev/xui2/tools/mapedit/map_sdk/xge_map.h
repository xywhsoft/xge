#ifndef XGE_MAP_H
#define XGE_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xge_map_size_t {
	int iWidth;
	int iHeight;
	int iLayerCount;
	int iTileWidth;
	int iTileHeight;
} xge_map_size_t;

typedef struct xge_map_t {
	xge_map_size_t tSize;
	int iTilesetId;
	const char* sName;
} xge_map_t;

void xgeMapInit(xge_map_t* pMap);
void xgeMapSetDefault(xge_map_t* pMap);
void xgeMapUnit(xge_map_t* pMap);

#ifdef __cplusplus
}
#endif

#endif
