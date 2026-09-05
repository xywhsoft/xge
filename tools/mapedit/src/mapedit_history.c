#include "mapedit_history.h"
#include <stdlib.h>
#include <string.h>

static size_t command_bytes(const mapedit_history_cmd_t* c)
{
	return (size_t)c->iChangeCapacity * sizeof(*c->arrChanges) +
		c->iLookupCapacity * sizeof(*c->pLookup) + c->iPayloadBytes;
}

void mapeditHistoryFreeCommand(mapedit_history_cmd_t* c)
{
	if (!c) return;
	free(c->arrChanges);
	free(c->pLookup);
	if (c->pPayload && c->destroyPayload) c->destroyPayload(c->pPayload);
	memset(c, 0, sizeof(*c));
}

static void clear_stack(mapedit_history_cmd_t* stack, int* count)
{
	while (*count > 0) mapeditHistoryFreeCommand(&stack[--*count]);
}

void mapeditHistoryClear(mapedit_history_t* h)
{
	if (!h) return;
	mapeditHistoryFreeCommand(&h->tCurrentCommand);
	clear_stack(h->arrUndo, &h->iUndoCount);
	clear_stack(h->arrRedo, &h->iRedoCount);
	memset(h, 0, sizeof(*h));
}

void mapeditHistoryBegin(mapedit_history_t* h)
{
	if (h) mapeditHistoryFreeCommand(&h->tCurrentCommand);
}

size_t mapeditHistoryBytes(const mapedit_history_t* h)
{
	size_t bytes = command_bytes(&h->tCurrentCommand);
	int i;
	for (i = 0; i < h->iUndoCount; ++i) bytes += command_bytes(&h->arrUndo[i]);
	for (i = 0; i < h->iRedoCount; ++i) bytes += command_bytes(&h->arrRedo[i]);
	return bytes;
}

static size_t hash_index(int index, size_t capacity)
{
	return (size_t)((uint32_t)index * UINT32_C(2654435761)) & (capacity - 1);
}

static int rehash(mapedit_history_cmd_t* c, size_t capacity)
{
	int* slots = (int*)calloc(capacity, sizeof(*slots));
	int i;
	if (!slots) return 0;
	for (i = 0; i < c->iChangeCount; ++i) {
		size_t slot = hash_index(c->arrChanges[i].iIndex, capacity);
		while (slots[slot]) slot = (slot + 1) & (capacity - 1);
		slots[slot] = i + 1;
	}
	free(c->pLookup);
	c->pLookup = slots;
	c->iLookupCapacity = capacity;
	return 1;
}

int mapeditHistoryRecord(mapedit_history_t* h, int index, int oldValue, int newValue)
{
	mapedit_history_cmd_t* c;
	size_t slot;
	if (!h || index < 0) return 0;
	if (oldValue == newValue) return 1;
	c = &h->tCurrentCommand;
	if (!c->iLookupCapacity || (size_t)(c->iChangeCount + 1) * 2 >= c->iLookupCapacity) {
		size_t cap = c->iLookupCapacity ? c->iLookupCapacity * 2 : 128;
		if (cap * sizeof(int) + (size_t)(c->iChangeCount + 1) * sizeof(*c->arrChanges) > MAPEDIT_HISTORY_BUDGET || !rehash(c, cap)) return 0;
	}
	slot = hash_index(index, c->iLookupCapacity);
	while (c->pLookup[slot]) {
		mapedit_history_change_t* change = &c->arrChanges[c->pLookup[slot] - 1];
		if (change->iIndex == index) {
			/* Keep tombstones until commit: repainting a cell remains O(1). */
			change->iNewTile = newValue;
			return 1;
		}
		slot = (slot + 1) & (c->iLookupCapacity - 1);
	}
	if (c->iChangeCount == c->iChangeCapacity) {
		int cap = c->iChangeCapacity ? c->iChangeCapacity * 2 : 64;
		mapedit_history_change_t* changes;
		if ((size_t)cap * sizeof(*changes) + c->iLookupCapacity * sizeof(int) > MAPEDIT_HISTORY_BUDGET) return 0;
		changes = (mapedit_history_change_t*)realloc(c->arrChanges, (size_t)cap * sizeof(*changes));
		if (!changes) return 0;
		c->arrChanges = changes;
		c->iChangeCapacity = cap;
	}
	c->arrChanges[c->iChangeCount] = (mapedit_history_change_t){ index, oldValue, newValue };
	c->pLookup[slot] = ++c->iChangeCount;
	return 1;
}

static void drop_first(mapedit_history_cmd_t* stack, int* count)
{
	mapeditHistoryFreeCommand(stack);
	--*count;
	memmove(stack, stack + 1, (size_t)*count * sizeof(*stack));
	memset(&stack[*count], 0, sizeof(*stack));
}

int mapeditHistoryCommit(mapedit_history_t* h)
{
	mapedit_history_cmd_t* c = &h->tCurrentCommand;
	int i, count = 0;
	for (i = 0; i < c->iChangeCount; ++i) {
		if (c->arrChanges[i].iOldTile != c->arrChanges[i].iNewTile) c->arrChanges[count++] = c->arrChanges[i];
	}
	c->iChangeCount = count;
	free(c->pLookup);
	c->pLookup = NULL;
	c->iLookupCapacity = 0;
	if (!count && !c->pPayload) { mapeditHistoryFreeCommand(c); return 0; }
	clear_stack(h->arrRedo, &h->iRedoCount);
	while (h->iUndoCount && (h->iUndoCount == MAPEDIT_HISTORY_MAX || mapeditHistoryBytes(h) > MAPEDIT_HISTORY_BUDGET)) drop_first(h->arrUndo, &h->iUndoCount);
	c->iBefore = h->iRevision;
	c->iAfter = ++h->iNextRevision;
	h->iRevision = c->iAfter;
	h->arrUndo[h->iUndoCount++] = *c;
	memset(c, 0, sizeof(*c));
	return 1;
}

mapedit_history_cmd_t* mapeditHistoryPeek(mapedit_history_t* h, int redo)
{
	int count = redo ? h->iRedoCount : h->iUndoCount;
	return count ? &(redo ? h->arrRedo : h->arrUndo)[count - 1] : NULL;
}

void mapeditHistoryMoved(mapedit_history_t* h, int redo)
{
	mapedit_history_cmd_t* c = mapeditHistoryPeek(h, redo);
	if (!c) return;
	h->iRevision = redo ? c->iAfter : c->iBefore;
	if (redo) { h->arrUndo[h->iUndoCount++] = *c; --h->iRedoCount; }
	else { h->arrRedo[h->iRedoCount++] = *c; --h->iUndoCount; }
	memset(c, 0, sizeof(*c));
}

void mapeditHistorySaved(mapedit_history_t* h) { h->iSavedRevision = h->iRevision; }
int mapeditHistoryDirty(const mapedit_history_t* h) { return h->iRevision != h->iSavedRevision; }
