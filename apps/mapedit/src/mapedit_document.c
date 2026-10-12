#include "mapedit_document.h"
#include "xge.h"
#include "mapedit_xrt.h"
#include <stdlib.h>
#include <string.h>

typedef struct document_snapshot {
	int flags;
	xge_map_t map;
	mapedit_tileset_t* tileset;
} document_snapshot;

static size_t value_bytes(const xvalue* value)
{
	xvaluetype type;
	size_t i, bytes = 64;
	xstrview text = {0};
	if (!value) return 0;
	type = xrtValueType(value);
	if (xrtValueGetString(value, &text)) return bytes + text.Size;
	for (i = 0; i < xrtValueCount(value); ++i) {
		if (type == XVALUE_OBJECT) bytes += value_bytes(xrtValueObjectAt(value, i, &text)) + text.Size + 64;
		else if (type == XVALUE_ARRAY) bytes += value_bytes(xrtValueArrayGet(value, i)) + 32;
	}
	return bytes;
}

static void destroy_snapshot(void* payload)
{
	document_snapshot* s = (document_snapshot*)payload;
	if (s->tileset) {
		if (s->tileset->pTileCustomRaw) xrtValueRelease(s->tileset->pTileCustomRaw);
		free(s->tileset);
	}
	xgeMapUnit(&s->map);
	free(s);
}

static int attach(mapedit_history_t* h, document_snapshot* s, size_t bytes)
{
	if (bytes > MAPEDIT_HISTORY_BUDGET) { destroy_snapshot(s); return 0; }
	mapeditHistoryBegin(h);
	h->tCurrentCommand.pPayload = s;
	h->tCurrentCommand.destroyPayload = destroy_snapshot;
	h->tCurrentCommand.iPayloadBytes = bytes;
	return 1;
}

int mapeditDocumentCaptureMap(mapedit_history_t* h, const xge_map_t* map, int flags)
{
	document_snapshot* s = (document_snapshot*)calloc(1, sizeof(*s));
	size_t bytes = sizeof(*s);
	if (!s) return 0;
	s->flags = flags;
	s->map = *map;
	s->map.pTiles = NULL;
	s->map.pPassageRaw = NULL;
	s->map.pCellDataRaw = NULL;
	s->map.arrPassageOverrides = NULL;
	if (flags & MAPEDIT_SNAPSHOT_SIZE) {
		size_t size = (size_t)map->iTileCount * sizeof(int);
		s->map.pTiles = (int*)malloc(size);
		if (!s->map.pTiles) goto fail;
		memcpy(s->map.pTiles, map->pTiles, size);
		bytes += size;
	}
	if (flags & MAPEDIT_SNAPSHOT_PASSAGE) {
		size_t size = (size_t)map->iPassageOverrideCount * sizeof(*map->arrPassageOverrides);
		s->map.iPassageOverrideCapacity = map->iPassageOverrideCount;
		if (size) {
			s->map.arrPassageOverrides = (xge_map_passage_override_t*)malloc(size);
			if (!s->map.arrPassageOverrides) goto fail;
			memcpy(s->map.arrPassageOverrides, map->arrPassageOverrides, size);
		}
		if (map->pPassageRaw && !(s->map.pPassageRaw = xrtValueDeepClone(map->pPassageRaw))) goto fail;
		bytes += size + value_bytes(map->pPassageRaw);
	}
	if (flags & MAPEDIT_SNAPSHOT_CUSTOM) {
		if (map->pCellDataRaw && !(s->map.pCellDataRaw = xrtValueDeepClone(map->pCellDataRaw))) goto fail;
		bytes += value_bytes(map->pCellDataRaw);
	}
	return attach(h, s, bytes);
fail:
	destroy_snapshot(s);
	return 0;
}

int mapeditDocumentCaptureTileset(mapedit_history_t* h, const mapedit_tileset_t* set)
{
	document_snapshot* s = (document_snapshot*)calloc(1, sizeof(*s));
	int i;
	if (!s) return 0;
	s->flags = MAPEDIT_SNAPSHOT_TILESET;
	s->tileset = (mapedit_tileset_t*)malloc(sizeof(*set));
	if (!s->tileset) { free(s); return 0; }
	*s->tileset = *set;
	s->tileset->pTileCustomRaw = NULL;
	s->tileset->pStaticSurface = NULL;
	for (i = 0; i < MAPEDIT_SPECIAL_MAX; ++i) s->tileset->arrSpecial[i].pSurface = NULL;
	if (set->pTileCustomRaw && !(s->tileset->pTileCustomRaw = xrtValueDeepClone(set->pTileCustomRaw))) { destroy_snapshot(s); return 0; }
	return attach(h, s, sizeof(*s) + sizeof(*set) + value_bytes(set->pTileCustomRaw));
}

#define SWAP_FIELD(type, a, b) do { type swapValue = (a); (a) = (b); (b) = swapValue; } while (0)
#define SWAP_TEXT(a, b) do { char swapText[sizeof(a)]; memcpy(swapText, a, sizeof(a)); memcpy(a, b, sizeof(a)); memcpy(b, swapText, sizeof(a)); } while (0)

static int equal_data(const xvalue* a, const xvalue* b)
{
	if ((!a || xrtValueCount(a) == 0) && (!b || xrtValueCount(b) == 0)) return 1;
	return xrtValueEqual(a, b);
}

int mapeditDocumentUnchanged(const mapedit_history_cmd_t* command, const xge_map_t* map, const mapedit_tileset_t* set)
{
	const document_snapshot* s = (const document_snapshot*)command->pPayload;
	int i;
	if (!s) return 1;
	if (s->flags & MAPEDIT_SNAPSHOT_TILESET) {
		const mapedit_tileset_t* old = s->tileset;
		if (strcmp(old->sName, set->sName) || strcmp(old->sStaticFile, set->sStaticFile) || strcmp(old->sCustomData, set->sCustomData) ||
			old->iSpecialCount != set->iSpecialCount || memcmp(old->arrPassage, set->arrPassage, sizeof(set->arrPassage)) ||
			memcmp(old->arrActorOverlay, set->arrActorOverlay, sizeof(set->arrActorOverlay)) || !equal_data(old->pTileCustomRaw, set->pTileCustomRaw)) return 0;
		for (i = 0; i < MAPEDIT_SPECIAL_MAX; ++i)
			if (strcmp(old->arrSpecial[i].sType, set->arrSpecial[i].sType) || strcmp(old->arrSpecial[i].sFile, set->arrSpecial[i].sFile)) return 0;
		return 1;
	}
	if ((s->flags & MAPEDIT_SNAPSHOT_META) && (strcmp(s->map.sName, map->sName) || strcmp(s->map.sTileset, map->sTileset) ||
		strcmp(s->map.sCustomData, map->sCustomData) || s->map.iState != map->iState)) return 0;
	if ((s->flags & MAPEDIT_SNAPSHOT_SIZE) && (s->map.iWidth != map->iWidth || s->map.iHeight != map->iHeight ||
		s->map.iTileCount != map->iTileCount || memcmp(s->map.pTiles, map->pTiles, (size_t)map->iTileCount * sizeof(int)))) return 0;
	if ((s->flags & MAPEDIT_SNAPSHOT_CUSTOM) && !equal_data(s->map.pCellDataRaw, map->pCellDataRaw)) return 0;
	if (s->flags & MAPEDIT_SNAPSHOT_PASSAGE) {
		if (s->map.iPassageOverrideCount != map->iPassageOverrideCount) return 0;
		for (i = 0; i < map->iPassageOverrideCount; ++i)
			if (s->map.arrPassageOverrides[i].iCellId != map->arrPassageOverrides[i].iCellId || s->map.arrPassageOverrides[i].iValue != map->arrPassageOverrides[i].iValue) return 0;
	}
	return 1;
}

void mapeditDocumentSwap(mapedit_history_cmd_t* command, xge_map_t* map, mapedit_tileset_t* set)
{
	document_snapshot* s = (document_snapshot*)command->pPayload;
	if (!s) return;
	if (s->flags & MAPEDIT_SNAPSHOT_TILESET) {
		/* Texture ownership stays with the live view and is refreshed by the UI. */
		int i;
		mapedit_tileset_t* old = s->tileset;
		SWAP_TEXT(set->sName, old->sName);
		SWAP_TEXT(set->sStaticFile, old->sStaticFile);
		SWAP_TEXT(set->sCustomData, old->sCustomData);
		SWAP_FIELD(int, set->iSpecialCount, old->iSpecialCount);
		SWAP_FIELD(int, set->iPassageCount, old->iPassageCount);
		SWAP_FIELD(int, set->iActorOverlayCount, old->iActorOverlayCount);
		for (i = 0; i < MAPEDIT_SPECIAL_MAX; ++i) {
			SWAP_TEXT(set->arrSpecial[i].sType, old->arrSpecial[i].sType);
			SWAP_TEXT(set->arrSpecial[i].sFile, old->arrSpecial[i].sFile);
		}
		for (i = 0; i < 65536; ++i) {
			SWAP_FIELD(unsigned char, set->arrPassage[i], old->arrPassage[i]);
			SWAP_FIELD(unsigned char, set->arrActorOverlay[i], old->arrActorOverlay[i]);
		}
		SWAP_FIELD(xvalue*, set->pTileCustomRaw, old->pTileCustomRaw);
		return;
	}
	if (s->flags & MAPEDIT_SNAPSHOT_META) {
		SWAP_TEXT(map->sName, s->map.sName);
		SWAP_TEXT(map->sTileset, s->map.sTileset);
		SWAP_TEXT(map->sCustomData, s->map.sCustomData);
		SWAP_FIELD(int, map->iState, s->map.iState);
	}
	if (s->flags & MAPEDIT_SNAPSHOT_SIZE) {
		SWAP_FIELD(int, map->iWidth, s->map.iWidth);
		SWAP_FIELD(int, map->iHeight, s->map.iHeight);
		SWAP_FIELD(int, map->iTileCount, s->map.iTileCount);
		SWAP_FIELD(int*, map->pTiles, s->map.pTiles);
	}
	if (s->flags & MAPEDIT_SNAPSHOT_CUSTOM) SWAP_FIELD(xvalue*, map->pCellDataRaw, s->map.pCellDataRaw);
	if (s->flags & MAPEDIT_SNAPSHOT_PASSAGE) {
		SWAP_FIELD(xvalue*, map->pPassageRaw, s->map.pPassageRaw);
		SWAP_FIELD(xge_map_passage_override_t*, map->arrPassageOverrides, s->map.arrPassageOverrides);
		SWAP_FIELD(int, map->iPassageOverrideCount, s->map.iPassageOverrideCount);
		SWAP_FIELD(int, map->iPassageOverrideCapacity, s->map.iPassageOverrideCapacity);
	}
}

int mapeditDocumentResize(xge_map_t* map, int width, int height)
{
	xge_map_t resized = *map;
	size_t tiles, i, j;
	int x, y, layer;
	if (!map->pTiles || xgeMapValidateSize(width, height, map->iLayers, NULL, &tiles) != XGE_MAP_OK) return XGE_MAP_ERROR_INVALID_ARGUMENT;
	if (width == map->iWidth && height == map->iHeight) return XGE_MAP_OK;
	resized.pTiles = (int*)calloc(tiles, sizeof(int));
	resized.arrPassageOverrides = NULL;
	resized.iPassageOverrideCount = resized.iPassageOverrideCapacity = 0;
	resized.pPassageRaw = NULL;
	resized.pCellDataRaw = map->pCellDataRaw ? xrtValueDeepClone(map->pCellDataRaw) : xrtValueObject();
	if (!resized.pTiles || !resized.pCellDataRaw) goto fail;
	resized.iWidth = width; resized.iHeight = height; resized.iTileCount = (int)tiles;
	for (layer = 0; layer < map->iLayers; ++layer)
		for (y = 0; y < height && y < map->iHeight; ++y)
			for (x = 0; x < width && x < map->iWidth; ++x)
				resized.pTiles[(layer * height + y) * width + x] = map->pTiles[(layer * map->iHeight + y) * map->iWidth + x];
	for (i = 0; i < (size_t)map->iPassageOverrideCount; ++i) {
		int cell = map->arrPassageOverrides[i].iCellId;
		x = cell % map->iWidth; y = cell / map->iWidth;
		if (cell >= 0 && x < width && y < height &&
			xgeMapSetPassageOverride(&resized, y * width + x, map->arrPassageOverrides[i].iValue) != XGE_MAP_OK) goto fail;
	}
	for (i = 0; i < xrtValueCount(resized.pCellDataRaw); ++i) {
		xvalue* channel = xrtValueObjectAt(resized.pCellDataRaw, i, NULL);
		if (!channel || xrtValueType(channel) != XVALUE_ARRAY) continue;
		j = xrtValueCount(channel);
		while (j) {
			xvalue* entry = xrtValueArrayGet(channel, --j);
			xvalue* id = xuiXrtValueObjectGet(entry, "cell", 0);
			int64 cell = -1;
			(void)xrtValueGetInt(id, &cell);
			if (cell < 0 || cell >= (int64)map->iWidth * map->iHeight ||
				cell % map->iWidth >= width || cell / map->iWidth >= height) {
				(void)xrtValueArrayRemove(channel, j, 1);
			} else if (!xuiXrtValueObjectSetInt(entry, "cell", 0, (cell / map->iWidth) * width + cell % map->iWidth)) goto fail;
		}
	}
	xgeMapUnit(map);
	*map = resized;
	map->bDirty = 1;
	return XGE_MAP_OK;
fail:
	xgeMapUnit(&resized);
	return XGE_MAP_ERROR_OUT_OF_MEMORY;
}
