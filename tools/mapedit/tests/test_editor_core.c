#include "xge.h"
#include "src/mapedit_history.h"
#include "src/mapedit_document.h"
#include "src/mapedit_files.h"
#include "src/mapedit_view.h"
#include "src/mapedit_xrt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); return 0; } } while (0)

static int test_history(void)
{
	mapedit_history_t h = {0};
	mapedit_history_cmd_t* c;
	clock_t start = clock();
	int i;
	for (i = 0; i < 1048576; ++i) CHECK(mapeditHistoryRecord(&h, i, 0, 1));
	for (i = 0; i < 1048576; ++i) CHECK(mapeditHistoryRecord(&h, i, 1, 2));
	CHECK(h.tCurrentCommand.iChangeCount == 1048576);
	CHECK(mapeditHistoryCommit(&h));
	CHECK(mapeditHistoryDirty(&h));
	CHECK(mapeditHistoryBytes(&h) <= MAPEDIT_HISTORY_BUDGET);
	c = mapeditHistoryPeek(&h, 0);
	CHECK(c && c->arrChanges[1048575].iOldTile == 0 && c->arrChanges[1048575].iNewTile == 2);
	mapeditHistorySaved(&h);
	CHECK(!mapeditHistoryDirty(&h));
	mapeditHistoryMoved(&h, 0);
	CHECK(mapeditHistoryDirty(&h) && h.iRedoCount == 1);
	mapeditHistoryMoved(&h, 1);
	CHECK(!mapeditHistoryDirty(&h));
	mapeditHistoryMoved(&h, 0);
	CHECK(mapeditHistoryRecord(&h, 3, 0, 4));
	CHECK(mapeditHistoryCommit(&h));
	CHECK(h.iRedoCount == 0 && mapeditHistoryDirty(&h));
	mapeditHistoryBegin(&h);
	CHECK(mapeditHistoryRecord(&h, 3, 4, 5));
	CHECK(mapeditHistoryRecord(&h, 3, 5, 4));
	CHECK(!mapeditHistoryCommit(&h));
	for (i = 0; i < 200; ++i) {
		CHECK(mapeditHistoryRecord(&h, i, 0, 1));
		CHECK(mapeditHistoryCommit(&h));
	}
	CHECK(h.iUndoCount == MAPEDIT_HISTORY_MAX);
	mapeditHistoryClear(&h);
	printf("history: 1048576 cells, duplicate updates, savepoint/branch/no-op, limits OK (%.3fs)\n", (double)(clock() - start) / CLOCKS_PER_SEC);
	return 1;
}

static int test_documents(void)
{
	xge_map_t map;
	mapedit_tileset_t* set = (mapedit_tileset_t*)calloc(1, sizeof(*set));
	mapedit_history_t h = {0};
	xvalue* entry;
	int64 cell = -1;
	CHECK(set);
	xgeMapInit(&map);
	map.iWidth = 3; map.iHeight = 3; map.iLayers = 2;
	CHECK(xgeMapAllocate(&map) == XGE_MAP_OK);
	strcpy(map.sName, "before");
	strcpy(map.sPath, "original.xson");
	CHECK(mapeditDocumentCaptureMap(&h, &map, MAPEDIT_SNAPSHOT_META));
	CHECK(mapeditDocumentUnchanged(&h.tCurrentCommand, &map, set));
	CHECK(mapeditHistoryBytes(&h) < 4096); /* Metadata edit never copies tile planes. */
	strcpy(map.sName, "after");
	CHECK(!mapeditDocumentUnchanged(&h.tCurrentCommand, &map, set));
	CHECK(mapeditHistoryCommit(&h));
	strcpy(map.sPath, "save-as.xson");
	mapeditDocumentSwap(mapeditHistoryPeek(&h, 0), &map, set);
	mapeditHistoryMoved(&h, 0);
	CHECK(strcmp(map.sName, "before") == 0 && strcmp(map.sPath, "save-as.xson") == 0);
	mapeditDocumentSwap(mapeditHistoryPeek(&h, 1), &map, set);
	mapeditHistoryMoved(&h, 1);
	CHECK(strcmp(map.sName, "after") == 0);
	mapeditHistoryClear(&h);
	map.pTiles[4] = 77; map.pTiles[13] = 88;
	CHECK(xgeMapSetPassageOverride(&map, 4, 15) == XGE_MAP_OK);
	map.pCellDataRaw = xrtXsonParse(mapeditXrtText("{\"tag\":[{\"cell\":4,\"value\":\"hello\"},{\"cell\":8,\"value\":\"cropped\"}]}", 0));
	CHECK(map.pCellDataRaw);
	CHECK(mapeditDocumentCaptureMap(&h, &map, MAPEDIT_SNAPSHOT_SIZE | MAPEDIT_SNAPSHOT_PASSAGE | MAPEDIT_SNAPSHOT_CUSTOM));
	CHECK(mapeditDocumentResize(&map, 2, 2) == XGE_MAP_OK);
	CHECK(map.iTileCount == 8 && map.pTiles[3] == 77 && map.pTiles[7] == 88);
	CHECK(map.arrPassageOverrides[0].iCellId == 3);
	entry = xrtValueArrayGet(xuiXrtValueObjectGet(map.pCellDataRaw, "tag", 0), 0);
	CHECK(xrtValueGetInt(xuiXrtValueObjectGet(entry, "cell", 0), &cell) && cell == 3);
	CHECK(xrtValueCount(xuiXrtValueObjectGet(map.pCellDataRaw, "tag", 0)) == 1);
	CHECK(mapeditHistoryCommit(&h));
	mapeditDocumentSwap(mapeditHistoryPeek(&h, 0), &map, set);
	mapeditHistoryMoved(&h, 0);
	CHECK(map.iWidth == 3 && map.pTiles[4] == 77 && map.arrPassageOverrides[0].iCellId == 4);
	CHECK(xrtValueCount(xuiXrtValueObjectGet(map.pCellDataRaw, "tag", 0)) == 2);
	mapeditDocumentSwap(mapeditHistoryPeek(&h, 1), &map, set);
	mapeditHistoryMoved(&h, 1);
	CHECK(map.iWidth == 2 && map.pTiles[3] == 77);
	CHECK(mapeditDocumentResize(&map, 0, 500) != XGE_MAP_OK && map.iWidth == 2);
	mapeditHistoryClear(&h);
	strcpy(set->sName, "before"); set->arrPassage[5] = 255;
	CHECK(mapeditDocumentCaptureTileset(&h, set));
	set->arrPassage[5] = 15; set->arrActorOverlay[5] = 1; strcpy(set->sName, "after");
	CHECK(mapeditHistoryCommit(&h));
	mapeditHistorySaved(&h);
	mapeditDocumentSwap(mapeditHistoryPeek(&h, 0), &map, set);
	mapeditHistoryMoved(&h, 0);
	CHECK(set->arrPassage[5] == 255 && set->arrActorOverlay[5] == 0 && !strcmp(set->sName, "before"));
	mapeditDocumentSwap(mapeditHistoryPeek(&h, 1), &map, set);
	mapeditHistoryMoved(&h, 1);
	CHECK(set->arrPassage[5] == 15 && !mapeditHistoryDirty(&h));
	mapeditHistoryClear(&h); xgeMapUnit(&map); free(set);
	puts("documents: metadata, tileset, resize/remap, allocation-free undo/redo OK");
	return 1;
}

static int test_view_and_files(void)
{
	float zooms[] = {0.25f, 0.5f, 1.0f, 1.25f, 1.5f, 2.0f, 8.0f};
	int i, j;
	for (i = 0; i < 7; ++i) {
		int cell = mapeditViewCell(17, zooms[i]);
		for (j = 0; j < 50; ++j) {
			CHECK(mapeditViewHit(j * cell, 0, cell, 50) == j);
			CHECK(mapeditViewHit(j * cell + cell - 1, 0, cell, 50) == j);
		}
		CHECK(mapeditViewHit(50 * cell, 0, cell, 50) == -1);
	}
	CHECK(mapeditViewHit(-1, 0, 16, 10) == -1);
	CHECK(fabsf(mapeditViewAnchor(100, 60, 16, 32) - 260.0f) < 0.001f);
	CHECK(mapeditViewZoom(NAN) == 1.0f && mapeditViewZoom(99) == 8.0f);
	CHECK(mapeditPathKey("C:/A/../b.xson") == mapeditPathKey("c:\\b.xson"));
	CHECK(mapeditPathKey("C:/p1/a.xson") != mapeditPathKey("C:/p2/a.xson"));
	CHECK(!mapeditFileStamp("map_sdk_test_file_does_not_exist.xson").exists);
	puts("view/files: pixel edges, zoom anchor, canonical identity OK");
	return 1;
}

static int quit_calls;
static int quit_allowed;
static int on_quit(void* user)
{
	(void)user;
	++quit_calls;
	if (xgeRequestQuit() != 0) quit_calls += 100; /* Reentrancy must be rejected. */
	return quit_allowed;
}

static int test_quit(void)
{
	xge_desc_t desc = {0};
	desc.iRunMode = XGE_RUN_MANUAL;
	CHECK(xgeInit(&desc) == XGE_OK);
	xgeSetQuitRequestCallback(on_quit, NULL);
	CHECK(!xgeRequestQuit() && quit_calls == 1);
	CHECK(!xgeRequestQuit() && quit_calls == 2);
	quit_allowed = 1;
	CHECK(xgeRequestQuit() && quit_calls == 3);
	xgeQuit();
	CHECK(quit_calls == 3);
	xgeSetQuitRequestCallback(NULL, NULL);
	xgeUnit();
	puts("quit: veto, retry, reentrancy, unconditional quit OK");
	return 1;
}

int main(void)
{
	return test_history() && test_documents() && test_view_and_files() && test_quit() ? 0 : 1;
}
