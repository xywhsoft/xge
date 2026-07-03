#include "xge_map.h"
#include <string.h>

void xgeMapInit(xge_map_t* pMap)
{
	if ( pMap == 0 ) {
		return;
	}
	memset(pMap, 0, sizeof(*pMap));
}

void xgeMapSetDefault(xge_map_t* pMap)
{
	if ( pMap == 0 ) {
		return;
	}
	xgeMapInit(pMap);
	pMap->sName = "default";
	pMap->tSize.iWidth = 100;
	pMap->tSize.iHeight = 100;
	pMap->tSize.iLayerCount = 4;
	pMap->tSize.iTileWidth = 32;
	pMap->tSize.iTileHeight = 32;
	pMap->iTilesetId = 0;
}

void xgeMapUnit(xge_map_t* pMap)
{
	if ( pMap == 0 ) {
		return;
	}
	memset(pMap, 0, sizeof(*pMap));
}
