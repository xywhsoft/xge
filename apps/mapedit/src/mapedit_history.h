#ifndef MAPEDIT_HISTORY_H
#define MAPEDIT_HISTORY_H

#include <stddef.h>
#include <stdint.h>

#define MAPEDIT_HISTORY_MAX 128
#define MAPEDIT_HISTORY_BUDGET ((size_t)128 * 1024 * 1024)

typedef struct mapedit_history_change_t {
	int iIndex, iOldTile, iNewTile;
} mapedit_history_change_t;

/* CPU document data only. Payloads are swapped on undo/redo, never GPU snapshots. */
typedef struct mapedit_history_cmd_t {
	mapedit_history_change_t* arrChanges;
	int iChangeCount, iChangeCapacity;
	int* pLookup;
	size_t iLookupCapacity;
	void* pPayload;
	void (*destroyPayload)(void*);
	size_t iPayloadBytes;
	uint64_t iBefore, iAfter;
} mapedit_history_cmd_t;

typedef struct mapedit_history_t {
	mapedit_history_cmd_t tCurrentCommand;
	mapedit_history_cmd_t arrUndo[MAPEDIT_HISTORY_MAX], arrRedo[MAPEDIT_HISTORY_MAX];
	int iUndoCount, iRedoCount;
	uint64_t iRevision, iSavedRevision, iNextRevision;
} mapedit_history_t;

void mapeditHistoryFreeCommand(mapedit_history_cmd_t* pCommand);
void mapeditHistoryClear(mapedit_history_t* pHistory);
void mapeditHistoryBegin(mapedit_history_t* pHistory);
/* Returns zero on allocation/budget failure; caller must not change the document. */
int mapeditHistoryRecord(mapedit_history_t* pHistory, int iIndex, int iOld, int iNew);
int mapeditHistoryCommit(mapedit_history_t* pHistory);
mapedit_history_cmd_t* mapeditHistoryPeek(mapedit_history_t* pHistory, int bRedo);
void mapeditHistoryMoved(mapedit_history_t* pHistory, int bRedo);
void mapeditHistorySaved(mapedit_history_t* pHistory);
int mapeditHistoryDirty(const mapedit_history_t* pHistory);
size_t mapeditHistoryBytes(const mapedit_history_t* pHistory);

#endif
